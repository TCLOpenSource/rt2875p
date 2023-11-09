/******************************************************************************
 *
 *   Copyright(c) 2015. Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author kevin_wang@realtek.com
 *
 *****************************************************************************/
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/kthread.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/pageremap.h>
#include <linux/string.h>
#include <linux/freezer.h>
#ifdef CONFIG_LG_SNAPSHOT_BOOT
#include <linux/suspend.h>
#endif
#include <linux/version.h>

#include <linux/uaccess.h>
#include <asm/cacheflush.h>

#include <mach/pcbMgr.h>
#include <mach/platform.h>

#include <base_types.h>
#include <rbus/tp_reg.h>
#include <rtk_cw_util.h>
#include <rtk_crt.h>

#include <rbus/sys_reg_reg.h>
#include <rbus/timer_reg.h>

#include "tp_soc.h"
#include <tp/tp_dbg.h>
#include <tp/tp_def.h>
#include <tp/tp_drv_global.h>
#include <tp/tp_drv.h>
#include <tp/tp_ioctl.h>
#include <tp/tp_reg_ctrl.h>
#include <tp/tp_ta.h>

/*****************************************

 inner function

******************************************/

#define TSP_PID(tsp)                          (((tsp[1] & 0x1F) <<8) +  (tsp[2]))

#define TPO_OUTPUT_EN(x)                      ((x>>21) & 0x3)
#define TPO_SRC(x)                            ((x>>16) & 0x3)

#define TP_SRC_INTERNAL_DEMOD_GET(x)          (x&0xf)
#define TP_SRC_EXTERNAL_TUNER0_GET(x)         ((x>>4)&0xf)
#define TP_SRC_EXTERNAL_TUNER1_GET(x)         ((x>>8)&0xf)
#define TP_SRC_EXTERNAL_CAM0_GET(x)           ((x>>12)&0xf)
#define TP_SRC_EXTERNAL_CAM1_GET(x)           ((x>>16)&0xf)

#ifdef TP_SUPPORT_CI
static unsigned long buff_check_timeout = 0;
#endif
static unsigned long tp_status_timeout = 0;

#ifdef SUPPORT_TP_CP_SAVING_MODE
extern bool mcp_enable_cp_power_saving(bool on);
#endif

extern void rtk_inv_range(const void *virt_start, const void *virt_end);
//extern UINT8 tp_tpo_dbg_en;
extern struct file *rec_bbp_fd;
extern struct file *rec_bbp_fd1;

int save_tp_reigster_value_ex(void);
int restore_tp_reigster_value_ex(void);
UINT8 GetTSPLen(TPK_TSP_LEN_T tsp_len);

void _dump_tsp_buffer_data(TPK_TP_ENGINE_T tp_id,
                           UINT8* p_virt_start,
                           UINT8* p_virt_end,
                           UINT8 block_size,
                           UINT16 pid_filter)
{

        while(p_virt_start < p_virt_end) {
                int i;
                UINT8* pTSP = (block_size == 192) ? &p_virt_start[4] : p_virt_start;

                if ((pTSP[1] & 0x40) && (pid_filter == 0xFFFF || TSP_PID(pTSP) == pid_filter)) {
                        if (block_size == 192) {
                                TP_DBG_SIMPLE("[TP%d] TSP = [ %02x %02x %02x %02x][",
                                              tp_id, p_virt_start[0], p_virt_start[1], p_virt_start[2], p_virt_start[3]);
                        } else {
                                TP_DBG_SIMPLE("[TP%d] TSP = [", tp_id);
                        }

                        for (i = 0; i < 32; i++)
                                TP_DBG_SIMPLE("%02x", pTSP[i]);


                        TP_DBG_SIMPLE(" ... ]\n");
                }

                p_virt_start +=  block_size;

        }
}

static INT32 _dump_tp_ring_buffer(void *arg)
{
        UINT8 buf_idx[MAX_TP_COUNT] = {TP0_BUFFER_MASS_DATA,
                                       TP1_BUFFER_MASS_DATA,
                                       TP2_BUFFER_MASS_DATA,
                                       TP3_BUFFER_MASS_DATA
                                      };

        UINT8 start_code_buf_idx[MAX_TP_COUNT] = { TP0_START_CODE_PACK,
                                                   TP1_START_CODE_PACK,
                                                   0x00,
                                                   0x00
                                                 };
        UINT8 *wp_temp_phy;
        UINT8 *wp_temp_virtual;
        UADDRESS wp_update;
        struct __kernel_old_timeval saved_time[MAX_TP_COUNT] = {0}, new_time = {0};
        UINT32 time_cnt;


        UINT32 i;
        INT32 result;
        TP_BUFFER_PARAM_T buf_param;
        void *virtBase, *virtLimit, *virtWP, *virtLastWP;
        wait_queue_head_t timeout_wq;
        init_waitqueue_head(&timeout_wq);

        current->flags &= ~PF_NOFREEZE;

        pTp_drv->dumpdata_start  = 1;

        for( i = 0; i < MAX_TP_COUNT; i++) {
                pTp_drv->tp[i].lastWP = 0;
                pTp_drv->tp[i].old_wp = 0;
                pTp_drv->tp[i].now_startcode_parsing_address = 0;
                pTp_drv->tp[i].wp_update_by_startcode = 0;
                pTp_drv->tp[i].timeout_wp = 0;
                pTp_drv->tp[i].find_new_start_code = 0;

                Tp_get_curtime(&pTp_drv->tp[i].saved_time);
                Tp_get_curtime(&saved_time[i]);
        }

        while(!kthread_should_stop()) {
                for (i = 0; i < MAX_TP_COUNT; i++) {
                        _tp_lock();
                        if (pTp_drv->tp[i].Shared_RWPtr.inited == 1) {
                                if(pTp_drv->useECP[i] == 1) {
                                        _tp_unlock();
                                        continue;
                                }

                                pTp_drv->tp[i].monitor_pvr_rp = *(pTp_drv->tp[i].Shared_RWPtr.pRP);
                                pTp_drv->tp[i].monitor_pvr_wp = *(pTp_drv->tp[i].Shared_RWPtr.pWP);

                                if(pTp_drv->tp[i].wp_update_by_startcode == 0) {
                                        pTp_drv->tp[i].wp_update_by_startcode = *pTp_drv->tp[i].Shared_RWPtr.pWP;
                                        pTp_drv->tp[i].timeout_wp = *pTp_drv->tp[i].Shared_RWPtr.pWP;

                                        Tp_get_curtime(&saved_time[i]);
                                }

                                wp_update = 0;
                                if( pTp_drv->tp[i].useTPStartCodeSearch == 1 ) {
                                        result = Tp_Get_Buffer(start_code_buf_idx[i], &buf_param);
                                        if (result != TPK_SUCCESS) {
                                                TP_WARNING("get buffer status fail (tp_id=%u)\n", i);
                                        } else {
                                                if( (pTp_drv->tp[i].old_wp < buf_param.Base) || (pTp_drv->tp[i].old_wp > buf_param.Limit) ) {
                                                        TP_DBG("old_wp from "PT_FMT_UADDR" to "PT_FMT_UADDR"\n", pTp_drv->tp[i].old_wp, buf_param.Base);
                                                        pTp_drv->tp[i].old_wp = buf_param.Base;
                                                }

                                                pTp_drv->tp[i].find_new_start_code = 0;

                                                while( pTp_drv->tp[i].old_wp != buf_param.WP) {
                                                        UINT8 *mass_temp_phy;
                                                        UINT8 *mass_temp_virtual;
                                                        TP_BUFFER_PARAM_T buf_param_tpbuf;
                                                        UADDRESS mass_temp_phy_temp;

                                                        Tp_Get_Buffer(buf_idx[i], &buf_param_tpbuf);
                                                        wp_temp_virtual = (UINT8 *)(pTp_drv->tp[i].start_code_search_buffer.BaseVir + (pTp_drv->tp[i].old_wp - pTp_drv->tp[i].start_code_search_buffer.BasePhy));
                                                        mass_temp_phy = (UINT8*)(unsigned long)(wp_temp_virtual[0] << 24 | wp_temp_virtual[1] << 16 | wp_temp_virtual[2] << 8 | wp_temp_virtual[3]);
                                                        mass_temp_virtual = (UINT8*)(pTp_drv->tp[i].mass_buffer.BaseVir + (mass_temp_phy - pTp_drv->tp[i].mass_buffer.BasePhy));
                                                        mass_temp_virtual = mass_temp_virtual + wp_temp_virtual[4] + 4;

                                                        if(pTp_drv->tp[i].show_startcode_info_cnt < 10) {
                                                                pTp_drv->tp[i].show_startcode_info_cnt++;
                                                                TP_DBG_SIMPLE("[%d]stco_p="PT_FMT_UADDR" stco_v=%p mass_p=%p sRP="PT_FMT_UADDR" sWP="PT_FMT_UADDR" WP="PT_FMT_UADDR"\n",
                                                                              pTp_drv->tp[i].show_startcode_info_cnt, pTp_drv->tp[i].old_wp, (void*)wp_temp_virtual, mass_temp_phy, *pTp_drv->tp[i].Shared_RWPtr.pRP, *pTp_drv->tp[i].Shared_RWPtr.pWP, buf_param_tpbuf.WP);
                                                        }

                                                        mass_temp_phy_temp = (UADDRESS )mass_temp_phy;
                                                        if(buf_param_tpbuf.WP >= *pTp_drv->tp[i].Shared_RWPtr.pRP) {
                                                                if(!((mass_temp_phy_temp >= *pTp_drv->tp[i].Shared_RWPtr.pRP) && (mass_temp_phy_temp <= buf_param_tpbuf.WP) )) {
                                                                        TP_ERROR("ERROR!! mass_temp_phy="PT_FMT_UADDR" is not in the range RP="PT_FMT_UADDR" WP="PT_FMT_UADDR"\n", mass_temp_phy_temp, *pTp_drv->tp[i].Shared_RWPtr.pRP, buf_param_tpbuf.WP);
                                                                        wp_temp_virtual[0] = 0;
                                                                        wp_temp_virtual[1] = 0;
                                                                        wp_temp_virtual[2] = 0;
                                                                        wp_temp_virtual[3] = 0;
                                                                }
                                                        } else {
                                                                if(((mass_temp_phy_temp < *pTp_drv->tp[i].Shared_RWPtr.pRP) && (mass_temp_phy_temp > buf_param_tpbuf.WP) )) {
                                                                        TP_ERROR("ERROR!! mass_temp_phy="PT_FMT_UADDR" is not in the range RP="PT_FMT_UADDR"WP="PT_FMT_UADDR"\n", mass_temp_phy_temp, *pTp_drv->tp[i].Shared_RWPtr.pRP, buf_param_tpbuf.WP);
                                                                        wp_temp_virtual[0] = 0;
                                                                        wp_temp_virtual[1] = 0;
                                                                        wp_temp_virtual[2] = 0;
                                                                        wp_temp_virtual[3] = 0;
                                                                }
                                                        }

                                                        pTp_drv->tp[i].old_wp = pTp_drv->tp[i].old_wp + START_CODE_SEARCH_INFO_PACK_LENGTH;
                                                        if(pTp_drv->tp[i].old_wp == buf_param.Limit)
                                                                pTp_drv->tp[i].old_wp = buf_param.Base;

                                                        pTp_drv->tp[i].find_new_start_code = 1;
                                                }

                                                if(*pTp_drv->tp[i].Shared_RWPtr_start_code_search.pWP != buf_param.WP) {
                                                        *pTp_drv->tp[i].Shared_RWPtr_start_code_search.pWP = buf_param.WP;
                                                }
                                        }

                                        pTp_drv->tp[i].start_code_search_buffer.RP_Connect[START_CODE_PACK_BUFFER_RP] = *pTp_drv->tp[i].Shared_RWPtr_start_code_search.pRP;

                                        Tp_Update_Buffer_RP(i , start_code_buf_idx[i], &pTp_drv->tp[i].start_code_search_buffer);

                                        if(pTp_drv->tp[i].find_new_start_code == 1) {
                                                if(pTp_drv->tp[i].old_wp == buf_param.Base)
                                                        wp_temp_phy = (UINT8*)(buf_param.Limit - START_CODE_SEARCH_INFO_PACK_LENGTH);
                                                else
                                                        wp_temp_phy = (UINT8*)(pTp_drv->tp[i].old_wp - START_CODE_SEARCH_INFO_PACK_LENGTH);

                                                wp_temp_virtual = pTp_drv->tp[i].start_code_search_buffer.BaseVir + (wp_temp_phy - pTp_drv->tp[i].start_code_search_buffer.BasePhy);
                                                wp_update = (UADDRESS)(wp_temp_virtual[0] << 24 | wp_temp_virtual[1] << 16 | wp_temp_virtual[2] << 8 | wp_temp_virtual[3]);

                                                if(pTp_drv->tp[i].now_startcode_parsing_address == 0)
                                                        pTp_drv->tp[i].now_startcode_parsing_address = wp_update;

                                                if(pTp_drv->tp[i].now_startcode_parsing_address != wp_update) {
                                                        if( CHECK_LOG_BLOCK_MASK(TP_LOG_BLOCK_STARTCODE) ) {
                                                                TP_BUFFER_PARAM_T buf_param_tpbuf;
                                                                Tp_Get_Buffer(buf_idx[i], &buf_param_tpbuf);
                                                                TP_DBG("TP%u: "PT_FMT_UADDR" pvr_wp="PT_FMT_UADDR" pvr_rp="PT_FMT_UADDR" tp_wp="PT_FMT_UADDR" tp_rp="PT_FMT_UADDR"\n", i, wp_update, *pTp_drv->tp[i].Shared_RWPtr.pWP, *pTp_drv->tp[i].Shared_RWPtr.pRP, buf_param_tpbuf.WP, buf_param_tpbuf.RP);
                                                                if( buf_param_tpbuf.WP > *(pTp_drv->tp[i].Shared_RWPtr.pWP) ) {
                                                                        if( (wp_update > *(pTp_drv->tp[i].Shared_RWPtr.pWP)) && wp_update < buf_param_tpbuf.WP) {
                                                                                //TP_DBG("update\n");
                                                                        } else {
                                                                                //TP_WARNING("no update\n");
                                                                        }
                                                                } else {
                                                                        if( ( (wp_update >  *(pTp_drv->tp[i].Shared_RWPtr.pWP)) && (wp_update < buf_param_tpbuf.Limit)) ||
                                                                            ( (wp_update > buf_param_tpbuf.Base) &&  (wp_update < buf_param_tpbuf.WP))) {
                                                                                ;
                                                                                //TP_DBG("update\n");
                                                                        } else {
                                                                                //TP_DBG("no update\n");
                                                                        }
                                                                }
                                                        }
                                                        pTp_drv->tp[i].wp_update_by_startcode = pTp_drv->tp[i].now_startcode_parsing_address;
                                                        pTp_drv->tp[i].timeout_wp = pTp_drv->tp[i].now_startcode_parsing_address;
                                                        if( CHECK_LOG_BLOCK_MASK(TP_LOG_BLOCK_STARTCODE) ) {
                                                                TP_DBG("TP%u: new wp "PT_FMT_UADDR"\n", i, pTp_drv->tp[i].wp_update_by_startcode);
                                                        }
                                                }
                                                pTp_drv->tp[i].now_startcode_parsing_address = wp_update;
                                                Tp_get_curtime(&saved_time[i]);
                                        }
                                }
                                /* update status */
                                result = Tp_Get_Buffer(buf_idx[i], &buf_param);
                                if (result != TPK_SUCCESS) {
                                        TP_WARNING("get buffer status fail (tp_id=%u)\n", i);
                                } else {
                                        if( pTp_drv->tp[i].useTPStartCodeSearch == 1 ) {
                                                if( CHECK_LOG_BLOCK_MASK(TP_LOG_BLOCK_STARTCODE) ) {
                                                        //TP_DBG_SIMPLE("HW WP= 0x%x RP= 0x%x\n", buf_param.WP, buf_param.RP);
                                                }

                                                Tp_get_curtime(&new_time);

                                                time_cnt = (new_time.tv_sec >= saved_time[i].tv_sec) ?
                                                           ((new_time.tv_sec - saved_time[i].tv_sec) * 1000) :
                                                           ((new_time.tv_sec + (0xffffffff - saved_time[i].tv_sec)) * 1000);
                                                time_cnt += ((new_time.tv_usec - saved_time[i].tv_usec) / 1000);

                                                if(time_cnt > 500) {
                                                        pTp_drv->tp[i].wp_update_by_startcode = pTp_drv->tp[i].timeout_wp;
                                                        saved_time[i] = new_time;
                                                        pTp_drv->tp[i].timeout_wp = buf_param.WP;
                                                        pTp_drv->tp[i].now_startcode_parsing_address = buf_param.WP;
                                                        TP_DBG("TP%u: timeout pvr update "PT_FMT_UADDR" "PT_FMT_UADDR"\n", i, pTp_drv->tp[i].wp_update_by_startcode, pTp_drv->tp[i].timeout_wp);
                                                }

                                                if( (pTp_drv->tp[i].wp_update_by_startcode >= buf_param.Base) && (pTp_drv->tp[i].wp_update_by_startcode <= buf_param.Limit))
                                                        buf_param.WP = pTp_drv->tp[i].wp_update_by_startcode;

                                                if( CHECK_LOG_BLOCK_MASK(TP_LOG_BLOCK_STARTCODE)) {
                                                        //TP_DBG_SIMPLE("UpdateWP= 0x%x\n", buf_param.WP);
                                                }
                                                Tp_FlushPatternSearch(i);
                                        }

                                        *pTp_drv->tp[i].Shared_RWPtr.pWP = buf_param.WP;
                                }

                                pTp_drv->tp[i].mass_buffer.RP_Connect[DUMP_BUFFER_RP] = *pTp_drv->tp[i].Shared_RWPtr.pRP;

                                Tp_Update_Buffer_RP(i , buf_idx[i], &pTp_drv->tp[i].mass_buffer);


                                /* flush data */
                                if ((pTp_drv->tp[i].lastWP < buf_param.Base) || (pTp_drv->tp[i].lastWP > buf_param.Limit)) {
                                        pTp_drv->tp[i].lastWP = buf_param.Base;
                                }

                                if (buf_param.WP >= pTp_drv->tp[i].lastWP) {
                                        virtLastWP = Tp_Buf_Phy_to_Vir(i, TP_BUFFER_MASS_DATA, (UINT8 *)pTp_drv->tp[i].lastWP);
                                        virtWP     = Tp_Buf_Phy_to_Vir(i, TP_BUFFER_MASS_DATA, (UINT8 *)buf_param.WP);

                                        if ( CHECK_LOG_BLOCK_MASK(TP_LOG_BLOCK_PVR) ) {
                                                _dump_tsp_buffer_data(i, virtLastWP, virtWP, 192, dump_tp_pvr_buff_pid[i]);
                                        }
                                        rtk_inv_range(virtLastWP, virtWP);
                                } else {
                                        virtLastWP = Tp_Buf_Phy_to_Vir(i, TP_BUFFER_MASS_DATA, (UINT8 *)pTp_drv->tp[i].lastWP);
                                        virtLimit  = Tp_Buf_Phy_to_Vir(i, TP_BUFFER_MASS_DATA, (UINT8 *)buf_param.Limit);
                                        virtBase   = Tp_Buf_Phy_to_Vir(i, TP_BUFFER_MASS_DATA, (UINT8 *)buf_param.Base);
                                        virtWP     = Tp_Buf_Phy_to_Vir(i, TP_BUFFER_MASS_DATA, (UINT8 *)buf_param.WP);

                                        if ( CHECK_LOG_BLOCK_MASK(TP_LOG_BLOCK_PVR) ) {
                                                _dump_tsp_buffer_data(i, virtLastWP, virtLimit, 192, dump_tp_pvr_buff_pid[i]);
                                        }
                                        rtk_inv_range(virtLastWP, virtLimit);

                                        if ( CHECK_LOG_BLOCK_MASK(TP_LOG_BLOCK_PVR) ) {
                                                _dump_tsp_buffer_data(i, virtLastWP, virtLimit, 192, dump_tp_pvr_buff_pid[i]);
                                        }
                                        rtk_inv_range(virtBase, virtWP);
                                }

                                pTp_drv->tp[i].lastWP = buf_param.WP;
                        }

                        _tp_unlock();
                }

                if (freezing(current)) {
                        try_to_freeze();
                }

                /* interruptible_sleep_on_timeout(&timeout_wq, 10); */
                wait_event_interruptible_timeout(timeout_wq, 0, 10);
        }

        return 0;
}

char* GetTPOSrcStr(unsigned char src)
{
        switch(src) {
                case 0:
                        return "MEM";
                case 1:
                        return "TP0";
                case 2:
                        return "TP1";
                case 3:
                        return "TP2";
                default :
                        return "Unknown";
        }
}

/* this function is used to monitor tpo and tpp ring buffer status */
void check_tpp_ring_buffer(void)
{
#ifdef TP_SUPPORT_CI
        UADDRESS Base;
        UADDRESS Limit;
        UADDRESS RP;
        UADDRESS WP;
        unsigned long Cnt;
        unsigned long Drp;
        unsigned long tpo_frmcfg = READ_REG32(TP_TP_TF_OUT_FRMCFG_reg);

        if (buff_check_timeout == 0)
                buff_check_timeout = jiffies + HZ;

        if (jiffies < buff_check_timeout)
                return;

        if ( CHECK_LOG_BLOCK_MASK( TP_LOG_BLOCK_TPOUT) ) {
                TP_INFO("TPO FRMCFG=%08lx (OTU=%lx, SRC=%lx (%s))\n",
                        tpo_frmcfg,
                        TPO_OUTPUT_EN(tpo_frmcfg),
                        TPO_SRC(tpo_frmcfg),
                        GetTPOSrcStr(TPO_SRC(tpo_frmcfg)));
        }

        if (TPO_OUTPUT_EN(tpo_frmcfg) && TPO_SRC(tpo_frmcfg) == 0) {
                int i;

                for ( i = 0 ; i < 3; i++) {
                        if ( CHECK_LOG_BLOCK_MASK(TP_LOG_BLOCK_TPOUT) ) {

                                Cnt = READ_REG32(TP_TP_TF0_P_CNT_reg + (i * 0x30));
                                Drp = READ_REG32(TP_TP_TF0_P_DRP_CNT_reg + (i * 0x30));

                                if (Cnt >= 0xFFFFFF || Drp >= 0xFFFF) {
                                        WRITE_REG32(TP_TP_TF0_P_CNT_reg + (i * 0x30), 0);
                                        WRITE_REG32(TP_TP_TF0_P_DRP_CNT_reg + (i * 0x30), 0);
                                }

                                WRITE_REG32(TP_TP_P_RING_CTRL_reg, i);

                                Base  = READ_REG32(TP_TP_P_RING_BASE_reg);
                                RP    = READ_REG32(TP_TP_P_RING_RP_reg);
                                WP    = READ_REG32(TP_TP_P_RING_WP_reg);
                                Limit = READ_REG32(TP_TP_P_RING_LIMIT_reg);

                                TP_INFO("TPP_RING[%d]={"PT_FMT_UADDR", "PT_FMT_UADDR", "PT_FMT_UADDR", "PT_FMT_UADDR"}, CNT=%06lu, Drp=%04lu, (Err=%llu)\n",
                                        i,
                                        Base,
                                        RP,
                                        WP,
                                        Limit,
                                        Cnt,
                                        Drp,
                                        pTp_drv->tpp[i].mass_buffer.buffer_err_cnt);
                        }

                        Tp_TPP_CheckRingBuffer();
                }
        }

        buff_check_timeout = jiffies + HZ;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return;
#endif
}

/*****************************************

 function for driver base to init RHAL_TP

******************************************/
/*------------------------------------------------------------------
 * Func : RHAL_TPInit
 *
 * Desc : init TP MODULE
 *
 * Parm : tp_id : TP Index
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPInit(IN TPK_TP_ENGINE_T tp_id)
{
        INT32 ret;
        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        ret = Tp_init(tp_id);
        if (ret != TPK_SUCCESS) {
                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                goto TP_ERROR;
        }
        Tp_Init_PID_Filter(tp_id);
#ifdef TP_SUPPORT_CI
        /*TPOut clk & tpout mode set ,init once*/
        Tp_TPO_Init(0);
#endif

        /* set tpi pol - mac5p internal demod only support falling */
        pTp_drv->tpi[TP_TPI_demod].clk_pol = 1;
        pTp_drv->tpi[TP_TPI_demod].sync_pol = 0;
        pTp_drv->tpi[TP_TPI_demod].val_pol = 0;
        Tp_SetInputPol(TP_TPI_demod, pTp_drv->tpi[TP_TPI_demod]);

#if defined(CONFIG_RTK_KDRV_ATSC3_DMX)
        if(TP_Get_MCM_ATSC3_Exist()==1) {
                pTp_drv->tpi[TP_TPI0].clk_pol = 1;
                pTp_drv->tpi[TP_TPI0].sync_pol = 0;
                pTp_drv->tpi[TP_TPI0].val_pol = 0;
                Tp_SetInputPol(TP_TPI0, pTp_drv->tpi[TP_TPI0]);
        }
#endif
        pTp_drv->tp[tp_id].tp_param.sync_byte_cip14 = 0x47;

        WRITE_REG32(TP_TP0_ERR_CNT_EN_reg, 0x000006ff);
        WRITE_REG32(TP_TP1_ERR_CNT_EN_reg, 0x000006ff);
        WRITE_REG32(TP_TP2_ERR_CNT_EN_reg, 0x000006ff);

        _tp_unlock();

        TP_TRACE("exit, id = %d\n", tp_id);

        return TPK_SUCCESS;

TP_ERROR:
        _tp_unlock();
        return ret;
}

EXPORT_SYMBOL(RHAL_TPInit);



/*------------------------------------------------------------------
 * Func : RHAL_TPUninit
 *
 * Desc : uninit TP MODULE
 *
 * Parm : id : TP Index
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPUninit(IN TPK_TP_ENGINE_T tp_id)
{
        INT32  ret;

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        ret = Tp_uninit(tp_id);

        _tp_unlock();

        return ret;
}
EXPORT_SYMBOL(RHAL_TPUninit);

/*------------------------------------------------------------------
 * Func : RHAL_SetTPMode
 *
 * Desc : Set TS Input mode of TP MODULE
 *
 * Parm : id : TP Index
 *        param : TS Parameter
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_SetTPMode(
        IN TPK_TP_ENGINE_T          tp_id,
        IN TPK_TP_TS_PARAM_T        Param
)
{
        INT32 ret;
        TP_INPUT_INTERFACE_PARAM_T tp_input_if_param;

        TP_TRACE("Enter, tp_id=%d, serial=%u, pid_filter_en=%u\n", tp_id, Param.serial, Param.pid_filter_en);

        if (Param.ts_in_tsp_len > byte_204) {
                TP_WARNING("bad parameter, ts_in_tsp_len=%u\n", Param.ts_in_tsp_len);
                return TPK_BAD_PARAMETER;
        }

        _tp_lock();
        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        ret = Tp_init(tp_id);
        if (ret != TPK_SUCCESS) {
                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                goto TP_ERROR;
        }

#if 0
        /* check if stream is started?? */
        if (pTp_drv->tp[tp_id].tp_stream_status == TP_STREAM_START) {
                TP_WARNING("not allow to change tp mode if stream is running, id=%u\n", tp_id);
                ret = TPK_RESOURCE_CONFLICT;
                goto TP_ERROR;
        }
#endif

        /* 1. tp input from from pad */
        /* 1.1 set TP input interface */
        tp_input_if_param.serial      = Param.serial;
        tp_input_if_param.data_order  = Param.data_order;
        tp_input_if_param.datapin     = Param.datapin;
        tp_input_if_param.sync_byte   = Param.sync_byte;
        tp_input_if_param.packet_size = Param.ts_in_tsp_len;
        Tp_ENG_SetInputInterface(tp_id, tp_input_if_param);

        /* 1.2 Set TP input interface as two-bits mode */
        /* TP HW ignore serial/parallel, data_order, datapin when set two-bits mode */
        Tp_ENG_TwoBitsModeEnable(tp_id, Param.two_bits);

        /* 2. tp input from memory */
        /* add time stamp */
        /* TP will add 4 bytes debug info in the front of TSP */
#ifdef TP_TIMESTAMP_USE_27M
        Tp_FRC_SetPrefix(tp_id, (Param.keep ? TP_FRC_KEPP : (Param.prefix_en ? TP_FRC_DEBUG_02 : TP_FRC_NO_PREFIX)), 0);
#else
        Tp_FRC_SetPrefix(tp_id, (Param.keep ? TP_FRC_KEPP : (Param.prefix_en ? TP_FRC_DEBUG_01 : TP_FRC_NO_PREFIX)), 0);
#endif

        /* frame rate control */
        /* TP will check 4 bytes debug info in the front of TSP when file playback */
        Tp_FRC_Enable(tp_id, (Param.frc_en ? 1 : 0));

        Tp_ENG_SetSyncReplace( tp_id, Param.sync_byte);

        if (Param.prefix_en) {
                pTp_drv->tp[tp_id].IsPktWithTimeStamp    = 1;
                pTp_drv->tp[tp_id].mass_buffer.bock_size = MASS_DATA_BUFFER_BLOCK_SIZE_WITH_TIME_STAMP;
        } else {
                pTp_drv->tp[tp_id].IsPktWithTimeStamp    = 0;
                pTp_drv->tp[tp_id].mass_buffer.bock_size = MASS_DATA_BUFFER_BLOCK_SIZE;
        }

        /* clr_tsp_scrmbl_bit should be used if scramble enabled */
        memcpy(&(pTp_drv->tp[tp_id].tp_param), &Param, sizeof(Param));
        pTp_drv->tp[tp_id].tp_param.inited = 1;

        _tp_unlock();

        TP_TRACE("exit, tp_id=%d, serial=%u, pid_filter_en=%u\n",
                 tp_id, Param.serial, Param.pid_filter_en);

        return TPK_SUCCESS;

TP_ERROR:
        _tp_unlock();
        return ret;

}

EXPORT_SYMBOL(RHAL_SetTPMode);

/*------------------------------------------------------------------
 * Func : RHAL_SetInputSourcePol
 *
 * Desc : Set TP Source Pol
 *
 * Parm : [I] tpi : TP input source
 *                [I] pol : pol param
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/

INT32 RHAL_SetInputSourcePol(
        IN TPK_TPI_PIN_T        tpi,
        IN TPK_TP_INPUT_PARAM_T pol
)
{
        CHECK_MAX_TPI_COUNT(tpi);

        _tp_lock();

        Tp_SetInputPol(tpi, pol);

        _tp_unlock();

        TP_TRACE("Exit, tpi=%d, clk_pol=%d, sync_pol=%d, val_pol=%d\n",
                 tpi, pol.clk_pol, pol.sync_pol, pol.val_pol);

        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_SetInputSourcePol);

/*------------------------------------------------------------------
 * Func : RHAL_SetTPRingBuffer
 *
 * Desc : Set TP Ring Buffer
 *
 * Parm : [I] id : TP Index
 *        [I] pPhyAddr : physical address
 *        [I] pVirAddr : virtual address
 *        [I] size     : size of ring buffer
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_SetTPRingBuffer(
        IN TPK_TP_ENGINE_T      tp_id,
        IN UINT8*               pPhyAddr,
        IN UINT8*               pVirAddr,
        IN UINT32               size
)
{
        tp_tp_threshold_RBUS threshold_reg;
        tp_tp_fullness_RBUS fullness_reg;
        UINT32  allignment_size;
        UINT8*  pPhyAddrLimit;
        UINT8*  pVirAddrLimit;
        UINT8   buf_idx = TP0_BUFFER_MASS_DATA;
        UADDRESS  info_pack_basePhy, info_pack_limitPhy;
        TP_BUFFER_PARAM_T buf_param;
        INT32 ret;

        TP_DBG("RHAL_SetTPRingBuffer, tp_id= %d, size=%u, pPhyAddr=%p, pVirAddr=%p\n", tp_id, size, pPhyAddr, pVirAddr);

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        if ((pPhyAddr == NULL) ||
            (pVirAddr == NULL) ||
            (size < MINIMUM_TP_BUFFER_SIZE)) {
                TP_WARNING("RHAL_SetTPRingBuffer bad parameter, pPhyAddr=%p, size=%u\n", pPhyAddr, size);
                pTp_drv->tp[tp_id].mass_buffer.inited = 0;
                return TPK_BAD_PARAMETER;
        }


        _tp_lock();

        ret = Tp_init(tp_id);

        if (ret != TPK_SUCCESS) {
                _tp_unlock();
                TP_WARNING("RHAL_SetTPRingBuffer Call Tp_init() fail (%d)\n", ret);
                return ret;
        }
        _tp_unlock();

        info_pack_basePhy  = pTp_drv->tp[tp_id].info_pack_buffer.BasePhy;
        info_pack_limitPhy = pTp_drv->tp[tp_id].info_pack_buffer.LimitPhy;

        if ((pTp_drv->tp[tp_id].info_pack_buffer.inited != 1) ||
            (info_pack_basePhy == 0) ||
            (info_pack_limitPhy == 0) ) {
                TP_WARNING("RHAL_SetTPRingBuffer TP%u bad parameter for info pack buffer (inited=%u,basePhy="PT_FMT_UADDR",limitPhy="PT_FMT_UADDR")\n",
                           tp_id, pTp_drv->tp[tp_id].info_pack_buffer.inited,
                           info_pack_basePhy, info_pack_limitPhy);
                return TPK_BAD_PARAMETER;
        }

        switch(tp_id) {
                case TP_TP0:
                        buf_idx = TP0_BUFFER_MASS_DATA;
                        break;
                case TP_TP1:
                        buf_idx = TP1_BUFFER_MASS_DATA;
                        break;
                case TP_TP2:
                        buf_idx = TP2_BUFFER_MASS_DATA;
                        break;
                case TP_TP3:
                        buf_idx = TP3_BUFFER_MASS_DATA;
                        break;
                default:
                        TP_WARNING("RHAL_SetTPRingBuffer unsupported id=%d\n", tp_id);
                        return TPK_BAD_PARAMETER;
        }

        /* /////////////////// */
        //
        /*   Note. we could also remap phycial address to uncashed memory for section using */
        /*      void *dvr_remap_cached_memory(unsigned long phys_addr, size_t size, const void *caller); */
        /*      void *dvr_remap_uncached_memory(unsigned long phys_addr, size_t size, const void *caller); */
        /*      void dvr_unmap_memory(void *cpu_addr, size_t size); */
        //
        /*      *uaddr = dvr_remap_uncached_memory(phys_addr, size, __builtin_return_address(0)); */
        //
        /* /////////////////// */

        /*  start to set buffer register */
        if (pTp_drv->tp[tp_id].IsPktWithTimeStamp == 1) {
                /* Allignment 376 bytes for length */
                allignment_size = (size / TP_BUFFER_WITH_FLOW_CTRL_ALIGNMENT) * TP_BUFFER_WITH_FLOW_CTRL_ALIGNMENT;
        } else {
                allignment_size = (size / TP_BUFFER_ALIGNMENT) * TP_BUFFER_ALIGNMENT;
        }

        pPhyAddrLimit = (UINT8*)((UADDRESS)pPhyAddr + allignment_size);
        pVirAddrLimit = (UINT8*)((UADDRESS)pVirAddr + allignment_size);

        /* set mass data buffer */
        buf_param.Base  = (UADDRESS)pPhyAddr;
        buf_param.Limit = (UADDRESS)pPhyAddrLimit;
        buf_param.RP    = (UADDRESS)pPhyAddr;
        buf_param.WP    = (UADDRESS)pPhyAddr;

        _tp_lock();

        /* check if stream is started?? */
        if (pTp_drv->tp[tp_id].tp_stream_status == TP_STREAM_START) {
                _tp_unlock();
                TP_WARNING("RHAL_SetTPRingBuffer TP%u stream is running, pPhyAddr=%p, size=%u\n", tp_id, pPhyAddr, size);
                return TPK_RESOURCE_CONFLICT;
        }

        ret = Tp_Set_Buffer(buf_idx, buf_param, BUFFER_WRITE_MASK_NONE);

        if (ret != TPK_SUCCESS) {
                _tp_unlock();
                return ret;
        }

        /* save to data structure */
        pTp_drv->tp[tp_id].mass_buffer.OriBufSize = size;
        pTp_drv->tp[tp_id].mass_buffer.BasePhy    = (UADDRESS)pPhyAddr;
        pTp_drv->tp[tp_id].mass_buffer.LimitPhy   = (UADDRESS)pPhyAddrLimit;
        pTp_drv->tp[tp_id].mass_buffer.BufSize    = allignment_size;
        pTp_drv->tp[tp_id].mass_buffer.BaseVir    = (UADDRESS)pVirAddr;
        pTp_drv->tp[tp_id].mass_buffer.LimitVir   = (UADDRESS)pVirAddrLimit;
        pTp_drv->tp[tp_id].mass_buffer.RP         = (UADDRESS)pPhyAddr;
        pTp_drv->tp[tp_id].mass_buffer.WP         = (UADDRESS)pPhyAddr;
        pTp_drv->tp[tp_id].mass_buffer.RP_Connect[MASS_DATA_BUFFER_RP] = (UADDRESS)pPhyAddr;
        pTp_drv->tp[tp_id].mass_buffer.RP_Connect[SECTION_BUFFER_RP]   = (UADDRESS)pPhyAddr;
        pTp_drv->tp[tp_id].mass_buffer.RP_Connect_cnt = MASS_DATA_BUFFER_RP_CONNECT;

        /* set CI_BUFFER_RP to be invalid */
        pTp_drv->tp[tp_id].mass_buffer.RP_Connect[CI_BUFFER_RP] = INVALID_BUFFER;

        if (pTp_drv->tp[tp_id].IsPktWithTimeStamp == 0) {
                pTp_drv->tp[tp_id].mass_buffer.bock_size = MASS_DATA_BUFFER_BLOCK_SIZE;
        } else {
                pTp_drv->tp[tp_id].mass_buffer.bock_size = MASS_DATA_BUFFER_BLOCK_SIZE_WITH_TIME_STAMP;
        }

        pTp_drv->tp[tp_id].mass_buffer.inited = 1;

        /* set info pack buffer */
        switch(tp_id) {
                case TP_TP0:
                        buf_idx = TP0_BUFFER_INFO_PACK;
                        break;
                case TP_TP1:
                        buf_idx = TP1_BUFFER_INFO_PACK;
                        break;
                case TP_TP2:
                        buf_idx = TP2_BUFFER_INFO_PACK;
                        break;
                case TP_TP3:
                        buf_idx = TP3_BUFFER_INFO_PACK;
                        break;
                default:
                        _tp_unlock();
                        TP_WARNING("RHAL_SetTPRingBuffer unsupported tp_id=%d\n", tp_id);
                        return TPK_BAD_PARAMETER;
        }

        buf_param.Base  = info_pack_basePhy;
        buf_param.Limit = info_pack_limitPhy;
        buf_param.RP    = info_pack_basePhy;
        buf_param.WP    = info_pack_basePhy;

        ret = Tp_Set_Buffer(buf_idx, buf_param, BUFFER_WRITE_MASK_NONE);
        if (ret != TPK_SUCCESS) {
                _tp_unlock();
                return ret;
        }

        /* set threshold and fullness again */
        threshold_reg.regValue  = 0;
        threshold_reg.threshold = TP_BUFFER_WITH_FLOW_CTRL_THRESHOLD;
        WRITE_REG32(TP_TP_THRESHOLD_reg, threshold_reg.regValue);

        fullness_reg.regValue = 0;
        fullness_reg.fullness = size - TP_BUFFER_WITH_FLOW_CTRL_THRESHOLD;
        WRITE_REG32(TP_TP_FULLNESS_reg, fullness_reg.regValue);

        pTp_drv->tp[tp_id].threshold = threshold_reg.regValue;
        pTp_drv->tp[tp_id].fullness  = fullness_reg.regValue;

        _tp_unlock();

        TP_TRACE("RHAL_SetTPRingBuffer Exit, tp_id=%d, size=%u, pPhyAddr=%p, pVirAddr=%p\n",
                 tp_id, size, pPhyAddr, pVirAddr);

        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_SetTPRingBuffer);

/*------------------------------------------------------------------
 * Func : RHAL_DumpTPRingBuffer
 *
 * Desc : Set TP Ring Buffer & address of rw pointer shared with the caller for dumping data
 *
 * Parm : [I] id : TP Index
 *        [I] enable     : enable or disable
 *        [I] pSharedRP : address of RP shared with the caller
 *        [I] pSharedWP : address of WP shared with the caller
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/

INT32 RHAL_DumpTPRingBuffer(IN TPK_TP_ENGINE_T tp_id,
                            IN UINT8 enable,
                            IN UINT8* pSharedRP,
                            IN UINT8* pSharedWP)
{
        struct task_struct *thread_dumpdata;
        UINT8 stopthread;
        UINT8 i;
        INT32 result;
        TP_BUFFER_PARAM_T buf_param;
        UINT8 buf_idx[MAX_TP_COUNT] = { TP0_BUFFER_MASS_DATA,
                                        TP1_BUFFER_MASS_DATA,
                                        TP2_BUFFER_MASS_DATA,
                                        TP3_BUFFER_MASS_DATA
                                      };

        UINT8 start_code_buf_idx[MAX_TP_COUNT] = { TP0_START_CODE_PACK,
                                                   TP1_START_CODE_PACK,
                                                   0x00,
                                                   0x00
                                                 };

        if((pSharedRP == NULL) || (pSharedWP == NULL)) {
                TP_WARNING("bad parameter,  pShareRP=%p,pShareWP=%p\n", pSharedRP, pSharedWP);
                return TPK_BAD_PARAMETER;
        }

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();
        result = Tp_init(tp_id);
        if(result != TPK_SUCCESS) {
                _tp_unlock();
                TP_WARNING("Call Tp_init() fail (%d)\n", result);
                return result;
        }
        _tp_unlock();


        _tp_lock();
        if (enable) {
                /* create thread to udpate status of buffer pointer */
                if(pTp_drv->thread_dumpdata != NULL) {
                        TP_INFO("_dump_tp_ring_buffer thread is already existed\n");
                } else {
                        /* no need to pass parameter into thread function */
                        thread_dumpdata = kthread_run(_dump_tp_ring_buffer, NULL, "_dump_tp_ring_buffer");
                        if(!IS_ERR(thread_dumpdata)) {
                                pTp_drv->thread_dumpdata = thread_dumpdata;

                        } else {
                                _tp_unlock();
                                TP_WARNING("create _dump_tp_ring_buffer thread failed, error=%ld\n", PTR_ERR(thread_dumpdata));
                                return TPK_NOT_ENOUGH_RESOURCE;
                        }
                }

                /* set shared RW pointer of mass data buffer */
                pTp_drv->tp[tp_id].Shared_RWPtr.pRP = (UADDRESS *)pSharedRP;
                pTp_drv->tp[tp_id].Shared_RWPtr.pWP = (UADDRESS *)pSharedWP;

                result = Tp_Get_Buffer(buf_idx[tp_id], &buf_param);
                if(result != TPK_SUCCESS) {
                        TP_WARNING("get buffer status fail (tp_id=%u)\n", tp_id);
                } else {
                        *pTp_drv->tp[tp_id].Shared_RWPtr.pWP = buf_param.WP;
                        *pTp_drv->tp[tp_id].Shared_RWPtr.pRP = buf_param.WP;
                        TP_DBG("Shared_RWPtr.pWP="PT_FMT_UADDR" RP="PT_FMT_UADDR"\n", buf_param.WP, buf_param.RP);
                }

                if( pTp_drv->tp[tp_id].Shared_RWPtr_start_code_search.inited == 1 ) {
                        if(Tp_is_SupportPatternSearch(tp_id) == TPK_SUCCESS) {
                                result = Tp_Get_Buffer(start_code_buf_idx[tp_id], &buf_param);
                                if(result != TPK_SUCCESS) {
                                        TP_WARNING("get buffer status fail (tp_id=%u)\n", tp_id);
                                } else {
                                        *pTp_drv->tp[tp_id].Shared_RWPtr_start_code_search.pWP = buf_param.WP;
                                        *pTp_drv->tp[tp_id].Shared_RWPtr_start_code_search.pRP = buf_param.WP;
                                        TP_DBG("TP%d: Shared_RWPtr_start_code_search.pWP ="PT_FMT_UADDR" RP="PT_FMT_UADDR"\n", tp_id, buf_param.WP, buf_param.RP);
                                        if(pTp_drv->tp[tp_id].useTPStartCodeSearch == 1)
                                                Tp_SetPatternSearchEnable( tp_id, 1);
                                }
                        }
                }
                pTp_drv->tp[tp_id].Shared_RWPtr.inited = 1;
        } else {
                /* remove shared RW pointer of mass data buffer */
                pTp_drv->tp[tp_id].Shared_RWPtr.pRP    = NULL;
                pTp_drv->tp[tp_id].Shared_RWPtr.pWP    = NULL;
                pTp_drv->tp[tp_id].Shared_RWPtr.inited = 0;

                if(Tp_is_SupportPatternSearch(tp_id) == TPK_SUCCESS) {
                        pTp_drv->tp[tp_id].Shared_RWPtr_start_code_search.pWP = NULL;
                        pTp_drv->tp[tp_id].Shared_RWPtr_start_code_search.pRP = NULL;
                        Tp_SetPatternSearchEnable( tp_id, 0);
                        pTp_drv->tp[tp_id].useTPStartCodeSearch = 0;
                }

                /* set DUMP_BUFFER_RP to be invalid */
                pTp_drv->tp[tp_id].mass_buffer.RP_Connect[DUMP_BUFFER_RP] = INVALID_BUFFER;

                /* check if need to stop thread */
                stopthread = 1;
                for (i = 0; i < MAX_TP_COUNT; i++) {
                        if (pTp_drv->tp[i].Shared_RWPtr.inited == 1) {
                                stopthread = 0;
                                break;
                        }
                }

                /* stop thread */
                if (stopthread == 1) {
                        if(pTp_drv->thread_dumpdata == NULL) {
                                TP_INFO("_dump_tp_ring_buffer thread does not exist, just exit\n");
                        } else {

                                _tp_unlock();
                                if(pTp_drv->thread_dumpdata) {
                                        i = 0;
                                        while(!pTp_drv->dumpdata_start && i < 10) {
                                                TP_WARNING("[%s %d] @@@@@@ wait  pTp_drv->dumpdata_start=%d ", __FUNCTION__, __LINE__, pTp_drv->dumpdata_start);
                                                msleep(1);
                                                i++;
                                        }
                                        /* it will wait until thread function exit */
                                        result = kthread_stop(pTp_drv->thread_dumpdata);
                                        if(result < 0) {
                                                TP_WARNING("call kthread_stop return error = %d", result);
                                                //return TPK_FAIL;
                                        }
                                }
                                _tp_lock();
                                pTp_drv->thread_dumpdata = NULL;
                                pTp_drv->dumpdata_start = 0;
                        }
                }
        }
        _tp_unlock();

        TP_INFO("exit, TP add RP for dumpdata, tp_id=%u, enable=%u, pSharedRP=%p, pSharedWP=%p \n",
                tp_id, enable, pSharedRP, pSharedWP);

        return TPK_SUCCESS;

}

EXPORT_SYMBOL(RHAL_DumpTPRingBuffer);

/*------------------------------------------------------------------
 * Func : RHAL_TPFlushBuffer
 *
 * Desc : Flush TP Ring Buffer
 *
 * Parm : [I] id : TP Index
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPFlushBuffer(TPK_TP_ENGINE_T tp_id)
{
        INT32 ret;

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        ret = Tp_init(tp_id);
        if (ret != TPK_SUCCESS) {
                _tp_unlock();

                TP_WARNING("Call Tp_init() fail (%d)\n", ret);

                return ret;
        }

        ret = Tp_flush_Buf(tp_id, 1, (BUF_SEL_MASS_DATA | BUF_SEL_INFO_PACK));

        _tp_unlock();

        TP_TRACE("exit, tp_id=%d\n", tp_id);

        return ret;
}

EXPORT_SYMBOL(RHAL_TPFlushBuffer);



/*------------------------------------------------------------------
 * Func : RHAL_GetTPBufferStatus
 *
 * Desc : Get TP Ring Buffer Status
 *
 * Parm : id       : TP Index
 *        pBufferSize : buffer size
 *        pDataSize   : data size
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_GetTPBufferStatus(
        IN TPK_TP_ENGINE_T        tp_id,
        OUT UINT32*               pBufferSize,
        OUT UINT32*               pDataSize
)
{
        TP_BUFFER_PARAM_T buf_param;
        UINT8 buf_idx;
        INT32 ret;

        /* check parameter */
        if ((pBufferSize == NULL) || (pDataSize == NULL)) {
                TP_WARNING("bad parameter, pBufferSize=%p, pDataSize=%p\n", pBufferSize, pDataSize);
                return TPK_BAD_PARAMETER;
        }

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        if(pTp_drv->tp[tp_id].mass_buffer.inited == 0) {
                *pBufferSize = 0;
                *pDataSize   = 0;
                return TPK_SUCCESS;
        }

        switch(tp_id) {
                case TP_TP0:
                        buf_idx = TP0_BUFFER_MASS_DATA;
                        break;
                case TP_TP1:
                        buf_idx = TP1_BUFFER_MASS_DATA;
                        break;
                case TP_TP2:
                        buf_idx = TP2_BUFFER_MASS_DATA;
                        break;
                case TP_TP3:
                        buf_idx = TP3_BUFFER_MASS_DATA;
                        break;
                default:
                        TP_WARNING("unsupported tp_id=%d\n", tp_id);
                        return TPK_BAD_PARAMETER;
        }

        _tp_lock();

        ret = Tp_init(tp_id);
        if (ret != TPK_SUCCESS) {
                _tp_unlock();
                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                return ret;
        }

        /* get buffer status from reg */
        Tp_Get_Buffer(buf_idx, &buf_param);

        if ((buf_param.Base == _ZERO) ||
            (buf_param.Limit == _ZERO) ||
            (buf_param.Base >= buf_param.Limit)) {
                _tp_unlock();

                TP_WARNING("buffer status error, base="PT_FMT_UADDR", limit="PT_FMT_UADDR"\n", buf_param.Base, buf_param.Limit);
                return TPK_NOT_INIT;
        }

        *pBufferSize = buf_param.Limit - buf_param.Base;
        *pDataSize   = (buf_param.WP >= buf_param.RP) ?
                       (buf_param.WP - buf_param.RP) :
                       (*pBufferSize - (buf_param.RP - buf_param.WP));

        _tp_unlock();

        TP_TRACE("exit, TP%u, BufferSize=%u, DataSize=%u\n", tp_id, *pBufferSize, *pDataSize);

        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_GetTPBufferStatus);


/*------------------------------------------------------------------
 * Func : RHAL_SetCIDLBufferRP
 *
 * Desc : Set CI download Buffer read point
 *
 * Parm : id       : TP Index
 *
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_CIP_SetBufferRP(
        IN TPK_TP_ENGINE_T        tp_id,
        IN UINT32               u32PhyRP
)
{
        TP_BUFFER_PARAM_T buf_param;
        UINT8 buf_idx;
        INT32 ret;

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        switch(tp_id) {
                case TP_TP0:
                        buf_idx = TP0_BUFFER_MASS_DATA;
                        break;
                case TP_TP1:
                        buf_idx = TP1_BUFFER_MASS_DATA;
                        break;
                case TP_TP2:
                        buf_idx = TP2_BUFFER_MASS_DATA;
                        break;
                case TP_TP3:
                        buf_idx = TP3_BUFFER_MASS_DATA;
                        break;
                default:
                        TP_WARNING("unsupported tp_id=%d\n", tp_id);
                        return TPK_BAD_PARAMETER;
        }

        _tp_lock();

        ret = Tp_init(tp_id);
        if (ret != TPK_SUCCESS) {
                _tp_unlock();
                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                return ret;
        }

        /* get buffer status from reg */
        Tp_Get_Buffer(buf_idx, &buf_param);

        if ((buf_param.Base == _ZERO) ||
            (buf_param.Limit == _ZERO) ||
            (buf_param.Base >= buf_param.Limit)) {
                _tp_unlock();

                TP_WARNING("buffer status error, base="PT_FMT_UADDR", limit="PT_FMT_UADDR"\n", buf_param.Base, buf_param.Limit);
                return TPK_NOT_INIT;
        }


        ret = Tp_Buf_Set_Mass_RR_Conect(tp_id, buf_idx, CI_BUFFER_RP, u32PhyRP);

        _tp_unlock();

        TP_TRACE("exit, TP%u, u32RP=%u\n", tp_id, u32PhyRP);

        return ret;
}



/*------------------------------------------------------------------
 * Func : RHAL_GetCIDLBufferStatus
 *
 * Desc : Get CI download Buffer Status
 *
 * Parm : id       : TP Index
 *
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_CIP_GetBufferStatus(
        IN TPK_TP_ENGINE_T        tp_id,
        UINT32               *pPhyBase,
        UINT32               *pPhyLimit,
        UINT32               *pPhyRead,
        UINT32              *pPhyWrite
)
{
        TP_BUFFER_PARAM_T buf_param;
        UINT8 buf_idx;
        INT32 ret;

        /* check parameter */
        if ((pPhyBase == NULL) || (pPhyLimit == NULL) || (pPhyRead == NULL) || (pPhyWrite == NULL)) {
                TP_WARNING("bad parameter, pPhyBase=%p, pPhyLimit=%p ,pPhyRead=%p ,pPhyWrite=%p\n",
                           pPhyBase, pPhyLimit, pPhyRead, pPhyWrite);
                return TPK_BAD_PARAMETER;
        }

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        switch(tp_id) {
                case TP_TP0:
                        buf_idx = TP0_BUFFER_MASS_DATA;
                        break;
                case TP_TP1:
                        buf_idx = TP1_BUFFER_MASS_DATA;
                        break;
                case TP_TP2:
                        buf_idx = TP2_BUFFER_MASS_DATA;
                        break;
                case TP_TP3:
                        buf_idx = TP3_BUFFER_MASS_DATA;
                        break;
                default:
                        TP_WARNING("unsupported tp_id=%d\n", tp_id);
                        return TPK_BAD_PARAMETER;
        }

        _tp_lock();

        ret = Tp_init(tp_id);
        if (ret != TPK_SUCCESS) {
                _tp_unlock();
                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                return ret;
        }

        /* get buffer status from reg */
        Tp_Get_Buffer(buf_idx, &buf_param);

        if ((buf_param.Base == _ZERO) ||
            (buf_param.Limit == _ZERO) ||
            (buf_param.Base >= buf_param.Limit)) {
                _tp_unlock();

                TP_WARNING("buffer status error, base="PT_FMT_UADDR", limit="PT_FMT_UADDR"\n", buf_param.Base, buf_param.Limit);
                return TPK_NOT_INIT;
        }
        *pPhyBase = buf_param.Base;
        *pPhyLimit = buf_param.Limit;
        *pPhyRead = Tp_Buf_Get_Mass_RR_Conect(tp_id, CI_BUFFER_RP);
        *pPhyWrite = buf_param.WP;

        _tp_unlock();

        TP_TRACE("exit, TP%u, pVirRead=%u, pVirWrite=%u\n", tp_id, *pPhyRead, *pPhyWrite);

        return TPK_SUCCESS;
}


/*------------------------------------------------------------------
 * Func : RHAL_GetCAMBitRate(Bps)
 *
 * Desc : Get the input/output flow of CAM
 *
 * Parm : [O] inputFlow  : the bit rate of CAM input(Bps)
 *        [O] outputFlow : the bit rate of CAM output(Bps)
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_CIP_GetCAMBitRate(OUT UINT32 * inputBitRate, OUT UINT32 * outputBitRate)
{
#ifdef TP_SUPPORT_CI
        UINT8 i, packet_size;

        /* init */
        *inputBitRate = 0;
        *outputBitRate = 0;

        _tp_lock();
        /* input */
        for(i = TP_TPP0; i < MAX_TP_P_COUNT; i++) {
                packet_size = GetTSPLen(pTp_drv->tp[i].tp_param.ts_in_tsp_len);
                if ( pTp_drv->tpp[i].tpp_stream_status == TPP_STREAM_START ) {
                        *inputBitRate += pTp_drv->tpp[i].recPkt_per_second * packet_size;
                        //TP_WARNING("CAM tpp%d=%llu, %d, %llu\n",i,  pTp_drv->tpp[i].recPkt_per_second,packet_size,pTp_drv->tpp[i].recPkt_per_second * packet_size);
                }
        }

        /* output */
        for(i = TP_TP0; i < MAX_TP_COUNT; i++) {
                packet_size = GetTSPLen(pTp_drv->tp[i].tp_param.ts_in_tsp_len);
                if ( pTp_drv->tp[i].tp_stream_status == TP_STREAM_START &&
                     pTp_drv->tp[i].tp_param.casType == TPK_WITH_CIP_CAS )
                        //pTp_drv->tp[i].tp_param.casType == TPK_WITH_CAS )
                {
                        *outputBitRate += pTp_drv->tp[i].pkt_per_second * packet_size;
                        //TP_WARNING("CAM tp%d=%llu, %d, %llu\n",i,  pTp_drv->tp[i].pkt_per_second,packet_size,pTp_drv->tp[i].pkt_per_second * packet_size);
                        break;
                }
        }

        _tp_unlock();
        return TPK_SUCCESS;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif
}

INT32 RHAL_CIP_GetData(TPK_TP_ENGINE_T tp_id, char *buffer, UINT32 *buffer_size)
{
#ifdef TP_SUPPORT_CI
        UINT8 *pRingBufPhy;
        UINT8 *p_buffer_vir;
        UINT32 RingBufCnt = 0;
        INT32 ret = TPK_SUCCESS;

        _tp_lock();

        TP_ReadData(tp_id, &pRingBufPhy, &RingBufCnt, pTp_drv->tp[tp_id].mass_buffer.bock_size, TP_BUFFER_CI);

        p_buffer_vir = Tp_Buf_Phy_to_Vir(tp_id, TP_BUFFER_CI, pRingBufPhy);

        if(RingBufCnt > *buffer_size) {
                RingBufCnt = *buffer_size;
        }

        RingBufCnt = (RingBufCnt / pTp_drv->tp[tp_id].mass_buffer.bock_size) * pTp_drv->tp[tp_id].mass_buffer.bock_size;

        *buffer_size = RingBufCnt;

        if(RingBufCnt>0){
                if (copy_to_user(to_user_ptr(buffer), p_buffer_vir,RingBufCnt)){
                        TP_ERROR("%s copy_to_user Fail\n", __func__);
                        ret = TPK_FAIL;
                } else {
                        ret = TPK_SUCCESS;
                }

                if(ret==TPK_SUCCESS){
                        TP_ReleaseData(tp_id, pRingBufPhy, RingBufCnt, TP_BUFFER_CI);
                }
        }
        _tp_unlock();

        return ret;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif

}

/*------------------------------------------------------------------
 * Func : RHAL_CIP_ReadData
 *
 * Desc : Read Data form TP
 *
 * Parm : [I] id : TP Index
 *        [O] ppReadPhyPtr : start address of Data
 *        [O] pContinueReadSize : data size
 *        [I] Threshold : threshold of read data
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_CIP_ReadData(
        TPK_TP_ENGINE_T       tp_id,
        UINT8**               ppReadPhyPtr,
        UINT32*               pContinueReadSize,
        UINT32                Threshold,
        long*                 phyAddrOffset)
{
#ifdef TP_SUPPORT_CI

        INT32 ret;

        UINT8 *pReadVirPtr;

        CHECK_MAX_TP_COUNT(tp_id);

        _tp_lock();

        ret = Tp_init(tp_id);

        check_tpp_ring_buffer();   /* here is used to monitor tpo and tpp ring buffer status */

        if (ret != TPK_SUCCESS) {
                _tp_unlock();

                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                return ret;
        }

        ret = TP_ReadData(tp_id, ppReadPhyPtr, pContinueReadSize, Threshold, TP_BUFFER_CI);

        pReadVirPtr = Tp_Buf_Phy_to_Vir(tp_id, TP_BUFFER_CI, *ppReadPhyPtr);

        *phyAddrOffset = *ppReadPhyPtr-pReadVirPtr;

        _tp_unlock();

        if ((ret == TPK_SUCCESS) || (ret == TPK_BUFFER_FULL) || (ret == TPK_BUFFER_UNDER_THRESHOLD)) {
                if (*pContinueReadSize) {
                        TP_VERBOSE("%s Get TP%u data return=%d (len=%u, ptr=%p)\n",
                                   __func__,tp_id, ret, *pContinueReadSize, *ppReadPhyPtr);
                }
        }
        return ret;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif

}

/*------------------------------------------------------------------
 * Func : RHAL_CIP_ReleaseData
 *
 * Desc : Release Data
 *
 * Parm : [I] id : TP Index
 *        [I] pReadPhyPtr : Start address to be released
 *        [I] Size : number of bytes to be released
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_CIP_ReleaseData(
        TPK_TP_ENGINE_T        tp_id,
        UINT8*                 pReadPhyPtr,
        UINT32                 Size
)
{
#ifdef TP_SUPPORT_CI
        INT32 ret;

        CHECK_MAX_TP_COUNT(tp_id);

        _tp_lock();

        ret = Tp_init(tp_id);

        if (ret != TPK_SUCCESS) {
                _tp_unlock();

                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                return ret;
        }


        ret = TP_ReleaseData(tp_id, pReadPhyPtr, Size, TP_BUFFER_CI);

        if (ret == TPK_SUCCESS) {
                pTp_drv->tp_pid[tp_id].tp_mass_data_release_cnt += Size;
                TP_VERBOSE("release TP%u data success (len=%u, ptr=%p)\n", tp_id, Size, pReadPhyPtr);
        }

        _tp_unlock();

        return ret;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif

}


/*------------------------------------------------------------------
 * Func : RHAL_TPStreamControl
 *
 * Desc : Get TP Ring Buffer Status
 *
 * Parm : [I] id   : TP Index
 *        [I] ctrl : buffer size
 *
 *            TP_STREAM_START: Start Steaming
 *            TP_STREAM_STOP : Stop Streaming
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPStreamControl(
        TPK_TP_ENGINE_T                tp_id,
        TPK_TP_STREAM_CTRL_T            ctrl
)
{
        tp_tp_tf0_cntl_RBUS tp_cntl;
        tp_tp_tf0_frmcfg_RBUS tp_frm;
        TPK_PID_REG_PARAM_T pid_reg;
        UINT16 pid_idx, i, start_stream_cnt = 0;
        UINT8 ddr_buf_idx;
        //UINT8 info_buf_idx;
        INT32 ret;
        REGADDR fram_reg_addr;

        tp_mask_print_emerg( TP_LOG_BLOCK_ALL, TP_LOG_LEVEL_WARNING, "RHAL_TPStreamControl Enter, tp_id=%d, ctrl=%u\n", tp_id, ctrl);

        if ((ctrl != TP_STREAM_START) && (ctrl != TP_STREAM_STOP)) {
                TP_WARNING("bad parameter, ctrl=%u\n", ctrl);
                return TPK_BAD_PARAMETER;
        }

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        switch(tp_id) {
                case TP_TP0:
                        pid_idx       = TP0_FIRST_PID_FILTER_IDX;
                        ddr_buf_idx   = TP0_BUFFER_MASS_DATA;
                        //info_buf_idx  = TP0_BUFFER_INFO_PACK;
                        fram_reg_addr = TP_TP_TF0_FRMCFG_reg;
                        break;
                case TP_TP1:
                        pid_idx       = TP1_FIRST_PID_FILTER_IDX;
                        ddr_buf_idx   = TP1_BUFFER_MASS_DATA;
                        //info_buf_idx  = TP1_BUFFER_INFO_PACK;
                        fram_reg_addr = TP_TP_TF1_FRMCFG_reg;
                        break;
                case TP_TP2:
                        pid_idx       = TP2_FIRST_PID_FILTER_IDX;
                        ddr_buf_idx   = TP2_BUFFER_MASS_DATA;
                        //info_buf_idx  = TP2_BUFFER_INFO_PACK;
                        fram_reg_addr = TP_TP_TF2_FRMCFG_reg;
                        break;
#ifdef TP_SUPPORT_TP3
                case TP_TP3:
                        pid_idx       = TP3_FIRST_PID_FILTER_IDX;
                        ddr_buf_idx   = TP3_BUFFER_MASS_DATA;
                        //info_buf_idx  = TP3_BUFFER_INFO_PACK;
                        fram_reg_addr = TP_TP_TF3_FRMCFG_reg;
                        break;
#endif
                default:
                        TP_WARNING("unsupported tp_id=%d\n", tp_id);
                        return TPK_BAD_PARAMETER;
        }

        _tp_lock();

        ret = Tp_init(tp_id);
        if (ret != TPK_SUCCESS) {
                _tp_unlock();
                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                return ret;
        }

        if ((pTp_drv->tp[tp_id].mass_buffer.inited == 0) ||
            (pTp_drv->tp[tp_id].info_pack_buffer.inited == 0)) {
                _tp_unlock();

                TP_WARNING("buffer not init, tp_id=%u, mass_buffer.inited=%u, info_pack_buffer.inited=%u\n",
                           tp_id,
                           pTp_drv->tp[tp_id].mass_buffer.inited,
                           pTp_drv->tp[tp_id].info_pack_buffer.inited);

                return TPK_NOT_INIT;
        }

        TP_DBG("TP%d: change stream status from %d to %d\n\n", tp_id, pTp_drv->tp[tp_id].tp_stream_status, ctrl);
        switch(ctrl) {
                case TP_STREAM_START:
                        if (pTp_drv->tp[tp_id].tp_stream_status == TP_STREAM_STOP) {
                                /* start stream sequence */
                                /* check if PID filter enabled, if not, force to set PID filter -> ring buffer mapping */
                                tp_cntl.regValue = 0;

                                if (pTp_drv->tp[tp_id].tp_param.pid_filter_en == 0) {
                                        /* disable pid filter */
                                        tp_cntl.pid_en = 1;   /* set select bit =1 */
                                        Tp_SetTpCtrlReg(tp_id, tp_cntl.regValue, 0);

                                        /* set ddr buffer to mass data buf_idx, other parameter won't change */
                                        Tp_GetPidParam(pid_idx, &pid_reg);
                                        pid_reg.ddr_q = ddr_buf_idx;
                                        Tp_SetPidParam(pid_idx, pid_reg);
                                } else {
                                        /* enable pid filter */
                                        tp_cntl.pid_en = 1;
                                        Tp_SetTpCtrlReg(tp_id, tp_cntl.regValue, 1);
                                }

                                TP_INFO("TP%u, pid_filter_en=%u\n", tp_id, pTp_drv->tp[tp_id].tp_param.pid_filter_en);

                                /* flush ring buffer */
                                ret = Tp_flush_Buf(tp_id, 1, (BUF_SEL_MASS_DATA | BUF_SEL_INFO_PACK));
                                if (ret != TPK_SUCCESS) {
                                        _tp_unlock();
                                        return ret;
                                }

                                /* frame enable =0 */
                                tp_frm.regValue = READ_REG32(fram_reg_addr);
                                tp_frm.frm_en   = 0;
                                WRITE_REG32(fram_reg_addr, tp_frm.regValue);

                                /* clean counter */
                                ret = Tp_ResetCounter(tp_id);
                                if (ret != TPK_SUCCESS) {
                                        _tp_unlock();
                                        return ret;
                                }

                                /* reset framer */
                                /* ret=Tp_rst(id); */
                                if (ret != TPK_SUCCESS) {
                                        _tp_unlock();
                                        return ret;
                                }

                                pTp_drv->tp[tp_id].IsMassBufferFull             = 0;
                                pTp_drv->tp_pid[tp_id].tp_mass_data_release_cnt = 0;
                                /* framer enble =1 */
                                tp_frm.frm_en = 1;
                                WRITE_REG32(fram_reg_addr, tp_frm.regValue);
                                pTp_drv->tp[tp_id].tp_stream_status = TP_STREAM_START;

                                if(pTp_drv->deliveryMode[tp_id] == TP_DELIVERY_MODE_TS || pTp_drv->deliveryMode[tp_id] == TP_DELIVERY_MODE_CI20){
                                    ret = section_assemble_thread_start_stop(1);
                                    if( ret != TPK_SUCCESS )
                                        TP_WARNING("section assemble thread start fail.\n");
                                }
#if 1
                                ret = tp_dbg_thread_start_stop(1);
                                if( ret != TPK_SUCCESS )
                                        TP_WARNING("TP debug thread start fail.\n");
#endif

                        }

                        break;
                case TP_STREAM_STOP:
                        if (pTp_drv->tp[tp_id].tp_stream_status == TP_STREAM_START) {
                                pTp_drv->tp[tp_id].tp_stream_status = TP_STREAM_STOP;

                                pTp_drv->tp[tp_id].tp_pkt_cnt.pkt = 0;
                                pTp_drv->tp[tp_id].tp_pkt_cnt.droppedPkt = 0;
                                pTp_drv->tp[tp_id].tp_pkt_cnt.errorPkt = 0;
                                for( i = 0; i < MAX_TP_COUNT; i++) {
                                        if (pTp_drv->tp[i].tp_stream_status == TP_STREAM_START)
                                                start_stream_cnt++;
                                }

                                if (start_stream_cnt == 0) {
                                        if(pTp_drv->deliveryMode[tp_id] == TP_DELIVERY_MODE_TS || pTp_drv->deliveryMode[tp_id] == TP_DELIVERY_MODE_CI20){
                                            ret = section_assemble_thread_start_stop(0);
                                            if( ret != TPK_SUCCESS )
                                                TP_WARNING("section assemble thread stop fail.\n");
                                        }
#if 1
                                        ret = tp_dbg_thread_start_stop(0);
                                        if( ret != TPK_SUCCESS )
                                                TP_WARNING("TP debug thread stop fail.\n");
#endif
                                }

                                /* frame enable =0 */
                                tp_frm.regValue = READ_REG32(fram_reg_addr);
                                tp_frm.frm_en   = 0;
                                WRITE_REG32(fram_reg_addr, tp_frm.regValue);

                                /* reset framer */
                                /* ret=Tp_rst(id); */

                                if (ret != TPK_SUCCESS) {
                                        _tp_unlock();
                                        return ret;
                                }

                                /* flush ring buffer */
                                ret = Tp_flush_Buf(tp_id, 1, (BUF_SEL_MASS_DATA | BUF_SEL_INFO_PACK));
                                if (ret != TPK_SUCCESS) {
                                        _tp_unlock();
                                        return ret;
                                }
                        }

                        break;
        }

        _tp_unlock();

        TP_TRACE("exit, tp_id=%d, ctrl=%u\n", tp_id, ctrl);
        tp_mask_print_emerg( TP_LOG_BLOCK_ALL, TP_LOG_LEVEL_WARNING, "RHAL_TPStreamControl exit, tp_id=%d, ctrl=%u\n", tp_id, ctrl);
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_TPStreamControl);


/*------------------------------------------------------------------
 * Func : RHAL_TPReadData
 *
 * Desc : Read Data form TP
 *
 * Parm : [I] id : TP Index
 *        [O] ppReadPhyPtr : start address of Data
 *        [O] pContinueReadSize : data size
 *        [I] Threshold : threshold of read data
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPReadData(
        TPK_TP_ENGINE_T       tp_id,
        UINT8**               ppReadPhyPtr,
        UINT32*               pContinueReadSize,
        UINT32                Threshold)
{
        INT32 ret;

        CHECK_MAX_TP_COUNT(tp_id);

        _tp_lock();

        ret = Tp_init(tp_id);

#ifdef TP_SUPPORT_CI
        check_tpp_ring_buffer();   /* here is used to monitor tpo and tpp ring buffer status */
#endif

        if (ret != TPK_SUCCESS) {
                _tp_unlock();

                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                return ret;
        }

        ret = TP_ReadData(tp_id, ppReadPhyPtr, pContinueReadSize, Threshold, TP_BUFFER_MASS_DATA);

        _tp_unlock();

        if ((ret == TPK_SUCCESS) || (ret == TPK_BUFFER_FULL) || (ret == TPK_BUFFER_UNDER_THRESHOLD)) {
                if (*pContinueReadSize) {
                        TP_VERBOSE("Get TP%u data return=%d (len=%u, ptr=%p)\n",
                                   tp_id, ret, *pContinueReadSize, *ppReadPhyPtr);
                }
        }

        return ret;
}
EXPORT_SYMBOL(RHAL_TPReadData);

/*------------------------------------------------------------------
 * Func : RHAL_TPReleaseData
 *
 * Desc : Release Data
 *
 * Parm : [I] id : TP Index
 *        [I] pReadPhyPtr : Start address to be released
 *        [I] Size : number of bytes to be released
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPReleaseData(
        TPK_TP_ENGINE_T        tp_id,
        UINT8*                 pReadPhyPtr,
        UINT32                 Size
)
{
        INT32 ret;

        CHECK_MAX_TP_COUNT(tp_id);

        _tp_lock();

        ret = Tp_init(tp_id);

        if (ret != TPK_SUCCESS) {
                _tp_unlock();

                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                return ret;
        }

        /* record ts */
        _tp_dump_lock();
        if (rec_fd[tp_id] && Size) {
#if IS_ENABLED(CONFIG_RTK_FEATURE_FOR_GKI)
                unsigned char* pucReadPtr;

                /* int write_count; */
                pucReadPtr = Tp_Buf_Phy_to_Vir(tp_id, TP_BUFFER_MASS_DATA, pReadPhyPtr);
                /* write_count = kernel_write(rec_fd,pucReadPtr,Size,&rec_fd->f_pos); */
                kernel_write(rec_fd[tp_id], pucReadPtr, Size, &rec_fd[tp_id]->f_pos);
#endif
        }
        _tp_dump_unlock();

        ret = TP_ReleaseData(tp_id, pReadPhyPtr, Size, TP_BUFFER_MASS_DATA);

        if (ret == TPK_SUCCESS) {
                pTp_drv->tp_pid[tp_id].tp_mass_data_release_cnt += Size;

                TP_VERBOSE("release TP%u data success (len=%u, ptr=%p)\n", tp_id, Size, pReadPhyPtr);
        }

        _tp_unlock();

        return ret;
}

EXPORT_SYMBOL(RHAL_TPReleaseData);


/*------------------------------------------------------------------
 * Func : RHAL_SetTPSource
 *
 * Desc : Set Data Source
 *
 * Parm : [I] id : TP Index
 *        [I] tp_src : TP src
 *        [I] casType : ci
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_SetTPSource(TPK_TP_ENGINE_T   tp_id,
                       TPK_TP_SOURCE_T   tp_src,
                       TPK_CAS_TYPE_T    casType)
{
        INT32 ret;
        REGADDR tp_cntl_reg;
        tp_tp_tf0_cntl_RBUS tp_cntl;
        BOOL bHaveCAM = false;
        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        if((casType == TPK_WITH_CAS) || (casType == TPK_WITH_CIP_CAS))
                bHaveCAM = true;

        _tp_lock();

        ret = Tp_init(tp_id);
        if (ret != TPK_SUCCESS) {
                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                goto TP_ERROR;
        }

#if 0
        /* check if stream is started?? */
        if (pTp_drv->tp[tp_id].tp_stream_status == TP_STREAM_START) {
                TP_WARNING("not allow to change source if TP stream is running, id=%u\n", tp_id);
                ret = TPK_RESOURCE_CONFLICT;
                goto TP_ERROR;
        }
#endif

        if ((tp_src != MTP) && (pTp_drv->mtp[tp_id].mtp_connect_type == MTP_TO_TP) &&
            (pTp_drv->mtp[tp_id].mtp_stream_status == MTP_STREAM_START)) {
                TP_WARNING("not allow to change source if MTP stream is running, mtp_id=%u\n", tp_id);
                ret = TPK_RESOURCE_CONFLICT;
                goto TP_ERROR;
        }

        switch(tp_id) {
                case TP_TP0:
                        tp_cntl_reg = TP_TP_TF0_CNTL_reg;
                        break;
                case TP_TP1:
                        tp_cntl_reg = TP_TP_TF1_CNTL_reg;
                        break;
                case TP_TP2:
                        tp_cntl_reg = TP_TP_TF2_CNTL_reg;
                        break;
#ifdef TP_SUPPORT_TP3
                case TP_TP3:
                        tp_cntl_reg = TP_TP_TF3_CNTL_reg;
                        break;
#endif
                default:
                        TP_WARNING("unsupported id=%d\n", tp_id);
                        ret = TPK_BAD_PARAMETER;
                        goto TP_ERROR;
        }

        tp_cntl.regValue = 0;
        tp_cntl.regValue = READ_REG32(tp_cntl_reg);

#if defined(CONFIG_RTK_KDRV_ATSC3_DMX)
        if(TP_Get_MCM_ATSC3_Exist()==1) {
                TP_INFO("RHAL_SetTPSource Ori src=%d\n", tp_src);
                tp_src = pTp_drv->tp[tp_id].tp_src;;
                if(tp_src == TS_UNMAPPING) {
                        tp_src = Internal_Demod;
                }
                TP_INFO("RHAL_SetTPSource change to src=%d\n", tp_src);
        }
#endif

        switch(tp_src) {
                case TS_SRC_0:
                        Tp_SetMemoryMode(tp_id, 0);

                        tp_cntl.regValue = 0;
                        tp_cntl.regValue = READ_REG32(tp_cntl_reg);

#ifdef TP_SUPPORT_CI
                        if(bHaveCAM == true) {
#ifdef CONFIG_RTK_TP_WITH_MIO
                                tp_cntl.src_sel_mio = 1;   //for mark2/mio
#endif
                                tp_cntl.ci_mode   = 1;
                                tp_cntl.src_sel_h = 0;
                                tp_cntl.src_sel_l = 1;
                        } else {
#ifdef CONFIG_RTK_TP_WITH_MIO
                                tp_cntl.src_sel_mio = 0;   //for mark2/mio
#endif
                                tp_cntl.ci_mode   = 0;
                                tp_cntl.src_sel_h = 0;
                                tp_cntl.src_sel_l = 0;
                        }
#else
                        tp_cntl.src_sel_h = 0;
                        tp_cntl.src_sel_l = 0;
#endif

                        Tp_Write_Data_Reg_Write(tp_cntl_reg, tp_cntl.regValue);
                        break;

                case TS_SRC_1:
                        Tp_SetMemoryMode(tp_id, 0);
                        tp_cntl.regValue = 0;
                        tp_cntl.regValue = READ_REG32(tp_cntl_reg);

#ifdef TP_SUPPORT_CI
                        if(bHaveCAM == true) {
#ifdef CONFIG_RTK_TP_WITH_MIO
                                tp_cntl.src_sel_mio = 1;   //for mark2/mio
#endif
                                tp_cntl.ci_mode   = 1;
                                tp_cntl.src_sel_h = 0;
                                tp_cntl.src_sel_l = 1;
                        } else {
#ifdef CONFIG_RTK_TP_WITH_MIO
                                tp_cntl.src_sel_mio = 0;   //for mark2/mio
#endif
                                tp_cntl.ci_mode   = 0;
                                tp_cntl.src_sel_h = 0;
                                tp_cntl.src_sel_l = 1;
                        }
#else
                        tp_cntl.src_sel_h = 0;
                        tp_cntl.src_sel_l = 1;
#endif

                        Tp_Write_Data_Reg_Write(tp_cntl_reg, tp_cntl.regValue);
                        break;

                case TS_SRC_2:
                        Tp_SetMemoryMode(tp_id, 0);
                        tp_cntl.regValue = 0;
                        tp_cntl.regValue = READ_REG32(tp_cntl_reg);

#ifdef TP_SUPPORT_CI
                        if(bHaveCAM == true) {
#ifdef CONFIG_RTK_TP_WITH_MIO
                                tp_cntl.src_sel_mio = 1;   //for mark2/mio
#endif
                                tp_cntl.ci_mode   = 1;
                                tp_cntl.src_sel_h = 0;
                                tp_cntl.src_sel_l = 1;
                        } else {
#ifdef CONFIG_RTK_TP_WITH_MIO
                                tp_cntl.src_sel_mio = 0;   //for mark2/mio
#endif
                                tp_cntl.ci_mode   = 0;
                                tp_cntl.src_sel_h = 1;
                                tp_cntl.src_sel_l = 0;
                        }
#else
                        tp_cntl.src_sel_h = 1;
                        tp_cntl.src_sel_l = 0;
#endif

                        Tp_Write_Data_Reg_Write(tp_cntl_reg, tp_cntl.regValue);

                        break;


                case Internal_Demod:
                        Tp_SetMemoryMode(tp_id, 0);
                        tp_cntl.regValue = 0;
                        tp_cntl.regValue = READ_REG32(tp_cntl_reg);

#ifdef TP_SUPPORT_CI
                        if(bHaveCAM == true) {
#ifdef CAM_SRC_CHANGE_BASE_ON_TP_INPUT_COUNT
                                pTp_drv->need_to_change_input_source[tp_id] = 1;
                                tp_cntl.ci_mode   = 0;
                                tp_cntl.src_sel_h = 1;
                                tp_cntl.src_sel_l = 1;
#else
#ifdef CONFIG_RTK_TP_WITH_MIO
                                tp_cntl.src_sel_mio = 1;   //for mark2/mio
#endif
                                tp_cntl.ci_mode   = 1;
                                tp_cntl.src_sel_h = 0;
                                tp_cntl.src_sel_l = 1;
#endif
                        } else {
#ifdef CONFIG_RTK_TP_WITH_MIO
                                tp_cntl.src_sel_mio = 0;   //for mark2/mio
#endif
                                tp_cntl.ci_mode   = 0;
                                tp_cntl.src_sel_h = 1;
                                tp_cntl.src_sel_l = 1;
#ifdef CAM_SRC_CHANGE_BASE_ON_TP_INPUT_COUNT
                                pTp_drv->need_to_change_input_source[tp_id] = 0;
#endif
                        }
#else
                        tp_cntl.src_sel_h = 1;
                        tp_cntl.src_sel_l = 1;
#endif

                        Tp_Write_Data_Reg_Write(tp_cntl_reg, tp_cntl.regValue);
                        break;

                case MTP:
                        /* set to memory to memory mode */
                        if(bHaveCAM == true) {
                                Tp_SetMemoryMode(tp_id, 0);
                        } else {
                                Tp_SetMemoryMode(tp_id, 1);
                        }

                        tp_cntl.regValue = 0;
                        tp_cntl.regValue = READ_REG32(tp_cntl_reg);

#ifdef TP_SUPPORT_CI
                        if(bHaveCAM == true) {
#ifdef CONFIG_RTK_TP_WITH_MIO
                                tp_cntl.src_sel_mio = 1;   //for mark2/mio
#endif
                                tp_cntl.ci_mode   = 1;
                                tp_cntl.src_sel_h = 0;
                                tp_cntl.src_sel_l = 1;
                        } else {
                                tp_cntl.ci_mode = 0;
#ifdef CONFIG_RTK_TP_WITH_MIO
                                tp_cntl.src_sel_mio = 0;   //for mark2/mio
#endif
                        }
#endif

                        Tp_Write_Data_Reg_Write(tp_cntl_reg, tp_cntl.regValue);

                        break;

                default:
                        TP_WARNING("unsupported tp_src=%d\n", tp_src);
                        ret = TPK_BAD_PARAMETER;
                        goto TP_ERROR;
        }

        pTp_drv->tp[tp_id].tp_src = tp_src;
        if(tp_src == MTP)
                pTp_drv->mtp[tp_id].mtp_connect_type = MTP_TO_TP;
        else if(pTp_drv->mtp[tp_id].mtp_connect_type == MTP_TO_TP)
                pTp_drv->mtp[tp_id].mtp_connect_type = MTP_NOT_CONNECT;

        pTp_drv->tp[tp_id].tp_param.casType = casType;

        /* tpi log print, RF-channel(mapping tp ch to tpp ch */
        if( tp_id < TP_TP3 && (tp_src == Internal_Demod || tp_src == TS_SRC_0) ) {
                pTp_drv->tpi_log_print[tp_id] = TPI_LOG_RF;
        }
        _tp_unlock();

        TP_TRACE("exit, tp_id=%d, casType=%u, tp_src=%u\n",
                 tp_id, casType, pTp_drv->tp[tp_id].tp_src);

        return TPK_SUCCESS;

TP_ERROR:
        _tp_unlock();
        return ret;

}
EXPORT_SYMBOL(RHAL_SetTPSource);

/*------------------------------------------------------------------
 * Func : RHAL_SetTPSource_By_Demod
 *
 * Desc : Set Data Source
 *
 * Parm : [I] id : TP Index
 *        [I] tp_src : TP src
 *        [I] casType : ci
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_SetTPSource_By_Demod(TPK_TP_ENGINE_T   tp_id,
                       TPK_TP_SOURCE_T   tp_src,
                       TPK_CAS_TYPE_T    casType)
{
        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        TP_INFO("RHAL_SetTPSource_By_Demod tp_src=%d\n", tp_src);
        pTp_drv->tp[tp_id].tp_src = tp_src;

        _tp_unlock();
        return TPK_SUCCESS;

}
EXPORT_SYMBOL(RHAL_SetTPSource_By_Demod);

/************************************************************

				      TPO related APIs

************************************************************/

/*------------------------------------------------------------------
 * Func : RHAL_TPOUT_SetMode
 *
 * Desc : Set TP Out mode
 *
 * Parm : [I] tpout_id : TP out Index
 *        [I] Param : TS Output Parameter
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPOUT_SetMode(
        UINT8                  tpout_id,
        TPK_TPOUT_TS_PARAM_T   Param)
{
#ifdef TP_SUPPORT_CI
        INT32 ret = TPK_SUCCESS;

        /* check parameter */
        CHECK_MAX_TPOUT_COUNT(tpout_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        ret = Tp_TPO_Init(tpout_id);
        if (ret != TPK_SUCCESS) {
                TP_WARNING("Call Tp_TPO_Init() fail (%d)\n", ret);
                _tp_unlock();
                return ret;
        }

        /* if tpo enabled ,stop first */
        if (pTp_drv->tpout[tpout_id].tpout_stream_status == TPOUT_STREAM_START) {
                Tp_TPO_Enable(tpout_id, TPOUT_STREAM_STOP);
        }

        Tp_TPO_SetMode(tpout_id, Param);

        memcpy(&(pTp_drv->tpout[tpout_id].tpout_param), &Param, sizeof(Param));

        /* if tpo enabled ,enable TPO after change the TPO source */
        if (pTp_drv->tpout[tpout_id].tpout_stream_status == TPOUT_STREAM_START) {
               Tp_TPO_Enable(tpout_id, TPOUT_STREAM_START);
        }

        _tp_unlock();

        TP_TRACE("exit, RHAL_TPOUT_SetMode TPOUT%u in_serial=%u, in_clk_pol=%u, out_serial=%u, out_clk_pol=%u\n",
                 tpout_id, Param.in_serial, Param.in_clk_pol,
                 Param.out_serial, Param.out_clk_pol);

        return TPK_SUCCESS;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif
}
EXPORT_SYMBOL(RHAL_TPOUT_SetMode);

/*------------------------------------------------------------------
 * Func : RHAL_TPOUT_GetMode
 *
 * Desc : Get TP Out mode
 *
 * Parm : [I] tpout_id : TP out Index
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPOUT_GetMode(
        UINT8                  tpout_id,
        TPK_TPOUT_TS_PARAM_T   *pParam)
{
#ifdef TP_SUPPORT_CI
        /* check parameter */
        CHECK_MAX_TPOUT_COUNT(tpout_id);
        CHECK_TP_STRUCTURE();

        if (pParam == NULL) {
                TP_WARNING("bad parameter, pParam=%p\n", pParam);
                return TPK_BAD_PARAMETER;
        }

        _tp_lock();

        Tp_TPO_GetMode(tpout_id, pParam);

        _tp_unlock();

        TP_TRACE("%s exit, TPOUT%u in_serial=%u, in_clk_pol=%u, out_serial=%u, out_clk_pol=%u\n",
                 __func__,tpout_id, pParam->in_serial, pParam->in_clk_pol,
                 pParam->out_serial, pParam->out_clk_pol);

        return TPK_SUCCESS;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif
}
EXPORT_SYMBOL(RHAL_TPOUT_GetMode);
/*------------------------------------------------------------------
 * Func : RHAL_GetTpoutStatus
 *
 * Desc : Get Status of TP Out module
 *
 * Parm : [I] tpout_id : Index of TP Out module
 *        [O] pTpoutStatus : TP output status output buffer
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPOUT_GetStatus(
        UINT8                tpout_id,
        TPK_TPOUT_STATUS_T*  pTpoutStatus
)
{
        TP_TRACE("Enter, id = %u\n", tpout_id);

        if ((tpout_id >= MAX_TPOUT_COUNT) || (pTpoutStatus == NULL)) {
                TP_WARNING("bad parameter, tpout_id=%u, pTpoutStatus=%p\n",
                           tpout_id, pTpoutStatus);

                return TPK_BAD_PARAMETER;
        }

        CHECK_TP_STRUCTURE();

        _tp_lock();

        memcpy(pTpoutStatus, &pTp_drv->tpout[tpout_id], sizeof(TPK_TPOUT_STATUS_T));

        _tp_unlock();

        TP_TRACE("%s exit, tpout_id=%u\n", __func__, tpout_id);

        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_TPOUT_GetStatus);
/*------------------------------------------------------------------
 * Func : RHAL_TPOUT_Enable
 *
 * Desc : TP Out control
 *
 * Parm : [I] tpout_id : TP out Index
 *        [I] ctrl     : TP out countrl
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPOUT_Enable(
        UINT8                   tpout_id,
        TPK_TPOUT_STREAM_CTRL_T ctrl
)
{
#ifdef TP_SUPPORT_CI
        INT32 ret = TPK_SUCCESS;

        CHECK_MAX_TPOUT_COUNT(tpout_id);
        CHECK_TP_STRUCTURE();

        tp_mask_print_emerg( TP_LOG_BLOCK_ALL, TP_LOG_LEVEL_WARNING, "%s Enter, tpout_id=%d, ctrl=%u\n", __func__,tpout_id, ctrl);

        if ((ctrl != TPOUT_STREAM_START) && (ctrl != TPOUT_STREAM_STOP)) {
                TP_WARNING("ctrl=%u\n", ctrl);
                return TPK_BAD_PARAMETER;
        }

        _tp_lock();

        ret = Tp_TPO_Init(tpout_id);
        if (ret != TPK_SUCCESS) {
                TP_WARNING("Call Tp_TPO_Init() fail (%d)\n", ret);
                _tp_unlock();
                return ret;
        }

        if (pTp_drv->tpout[tpout_id].tpout_src == TPK_TPOUT_UNMAPPING) {
                TP_WARNING("%s Ln=%d tpout_id=%d tpout_src==%d return fail\n", __func__, __LINE__, tpout_id, pTp_drv->tpout[tpout_id].tpout_src);
                _tp_unlock();
                return TPK_FAIL;
        }
#ifdef CAM_SRC_CHANGE_BASE_ON_TP_INPUT_COUNT
        if (pTp_drv->tpout[tpout_id].tpout_stream_status == TPOUT_STREAM_START) {
                TP_TPInput_Monitor_Init(TPINPUT_MONITOR_TPID);
        }else{
                TP_TPInput_Monitor_Uninit(TPINPUT_MONITOR_TPID);
        }
#endif
        if (pTp_drv->tpout[tpout_id].tpout_stream_status == ctrl) {
                _tp_unlock();
                return TPK_SUCCESS;
        }

        Tp_TPO_Enable(tpout_id, ctrl);

        pTp_drv->tpout[tpout_id].tpout_stream_status = ctrl;

        _tp_unlock();

        tp_mask_print_emerg( TP_LOG_BLOCK_ALL, TP_LOG_LEVEL_WARNING, "%s exit, tpout_id=%d, ctrl=%u\n",__func__, tpout_id, ctrl);
        TP_TRACE("exit, TPOUT%u ctrl=%u\n", tpout_id, ctrl);

        return TPK_SUCCESS;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif
}
EXPORT_SYMBOL(RHAL_TPOUT_Enable);


/*------------------------------------------------------------------
 * Func : RHAL_TPOUT_SetDataSource
 *
 * Desc : Set TP Out Data source
 *
 * Parm : [I] tpout_id : TP out Index
 *        [I] tpout_src : tp output source
 *                [I] remux_param : tpp remux parameter
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/

INT32 RHAL_TPOUT_SetDataSource(
        UINT8              tpout_id,
        TPK_TPOUT_SOURCE_T tpout_src
)
{
#ifdef TP_SUPPORT_CI
        TPK_TP_SOURCE_T src_type;
        INT32 ret = TPK_SUCCESS;

        CHECK_MAX_TPOUT_COUNT(tpout_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        /* check tpo init status */
        ret = Tp_TPO_Init(tpout_id);
        if (ret != TPK_SUCCESS) {
                TP_WARNING("Call Tp_TPO_Init() fail (%d)\n", ret);
                _tp_unlock();
                return ret;
        }

        /* check if the same source */
        if (pTp_drv->tpout[tpout_id].tpout_src == tpout_src ) {
                TP_WARNING("%s the same TPOut source: %d (0:MTP,1:TPP,2:INDEMOD,3:TPI1,4:TPI2,5:TPI3)!! exit directly", __func__,tpout_src);
                _tp_unlock();
                return ret;
        }

        /* if tpo enabled ,stop first */
        if (pTp_drv->tpout[tpout_id].tpout_stream_status == TPOUT_STREAM_START) {
                Tp_TPO_Enable(tpout_id, TPOUT_STREAM_STOP);
        }

        /* set MTP to "not connect mode" */
        if ((tpout_src != TPK_TP_MTP_TO_TPOUT) && (pTp_drv->mtp[0].mtp_connect_type == MTP_TO_TPOUT)) {
                pTp_drv->mtp[0].mtp_connect_type = MTP_NOT_CONNECT;
        }

        switch(tpout_src) {
                case TPK_IN_DEMOD_TO_TPOUT:
                		src_type = Internal_Demod;
                		break;
                case TPK_TPP_TO_TPOUT:
                case TPK_TP_MTP_TO_TPOUT:
                		src_type = MTP;
                		break;
                case TPK_TPI0_TO_TPOUT:
                		src_type = TS_SRC_0;
                		break;
                case TPK_TPI1_TO_TPOUT:
                		src_type = TS_SRC_1;
                		break;
                case TPK_TPI2_TO_TPOUT:
                		src_type = TS_SRC_2;
                		break;
                case TPK_MIO_TO_TPOUT:
                        src_type = MIO;
                        break;
                default:
                		src_type = TS_UNMAPPING;
               			break;
        }

        Tp_TPO_SetDataSource(src_type);

        /* tpo bypass internal demod, set tpo input param,same as demod output param */
        /* please move me out */
        if (tpout_src == TPK_IN_DEMOD_TO_TPOUT) {
                TPK_TPOUT_TS_PARAM_T tpo_param;
                Tp_TPO_GetMode(tpout_id, &tpo_param);
                tpo_param.in_clk_pol = pTp_drv->tpi[TP_TPI_demod].clk_pol;
                tpo_param.in_sync_pol = pTp_drv->tpi[TP_TPI_demod].sync_pol;
                tpo_param.in_val_pol = pTp_drv->tpi[TP_TPI_demod].val_pol;
                Tp_TPO_SetMode(tpout_id, tpo_param);
        }

        //save tpout source
        pTp_drv->tpout[tpout_id].tpout_src = tpout_src;

        /* if tpo enabled ,enable TPO after change the TPO source */
        if (pTp_drv->tpout[tpout_id].tpout_stream_status == TPOUT_STREAM_START) {
                Tp_TPO_Enable(tpout_id, TPOUT_STREAM_START);
        }

        _tp_unlock();

        TP_TRACE("%s exit, TPOUT%u set to tpout_src(%u)(0:MTP,1:TPP,2:INDEMOD,3:TPI1,4:TPI2,5:TPI3) \n", __func__,tpout_id, tpout_src);
        return TPK_SUCCESS;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif
}

INT32 RHAL_TPOUT_SetBypassClk(
        UINT8                   tpout_id,
        UINT32                  bypass_out_clk_h,
        UINT32                  bypass_out_clk_l)
{
#ifdef TP_SUPPORT_CI
    CHECK_MAX_TPOUT_COUNT(tpout_id);
    CHECK_TP_STRUCTURE();

    _tp_lock();
    Tp_TPO_SetBypassClk(bypass_out_clk_h, bypass_out_clk_l);
    _tp_unlock();

	return TPK_SUCCESS;
#else
	TP_ERROR("%s Not Support!!", __func__);
	return TPK_FAIL;
#endif
}
/*------------------------------------------------------------------
 * Func : RHAL_TPOUT_SetClockSpeed
 *
 * Desc : Set TP Out Clock Speed
 *
 * Parm : [I] tpout_id : TP out Index
 *        [I] pre_div     : TP clock pre div
 *        [I] post_div    : TP clock post div
 *        [I] div16       : TP clock div16
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPOUT_SetClockSpeed(
        UINT8                   tpout_id,
        UINT32                  pre_div,
        UINT32                  post_div,
        UINT32                  div16
)
{
#ifdef TP_SUPPORT_CI
        CHECK_MAX_TPOUT_COUNT(tpout_id);

        CHECK_TP_STRUCTURE();

        _tp_lock();
#ifdef CONFIG_RTK_TP_WITH_MIO
        {
            unsigned int div_result;
#ifdef TP_VDEC_SPEED_MHZ
            unsigned int tpo_clk;
            tpo_clk = (TP_SPEED_MHZ * 1000)/(pre_div + post_div + 2);
            div_result = (TP_VDEC_SPEED_MHZ * 1000)/tpo_clk;
#else
            div_result = (pre_div + post_div + 2)/4;
#endif
            if(div_result<4){
                div_result = 5;
                TP_ERROR("%s mio tpo clock div setting wrong, use default value!!\n", __func__);
            }
            div_result = div_result - 2;

            pre_div = (div_result + 1) / 2;
            if (pre_div > 511)
                pre_div = 511;

            post_div = div_result - pre_div;
            if (post_div > 511)
                post_div = 511;
        }
#endif
        Tp_TPO_SetClkDiv(pre_div, post_div, div16);
        _tp_unlock();

        TP_WARNING("exit, TPOUT%u pre_div=%u post_div=%u div16=%u\n", tpout_id, pre_div, post_div, div16);

        return TPK_SUCCESS;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif
}
EXPORT_SYMBOL(RHAL_TPOUT_SetClockSpeed);

INT32 RHAL_TPOUT_GetClockSpeed(
        UINT8                   tpout_id,
        UINT32                  *ppre_div,
        UINT32                  *ppost_div,
        UINT32                  *pdiv16
)
{
#ifdef TP_SUPPORT_CI
        CHECK_MAX_TPOUT_COUNT(tpout_id);

        CHECK_TP_STRUCTURE();

        _tp_lock();

        Tp_TPO_GetClkDiv(ppre_div, ppost_div, pdiv16);

        _tp_unlock();

        return TPK_SUCCESS;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif
}
EXPORT_SYMBOL(RHAL_TPP_Init);

/************************************************************

				      TPP related APIs

************************************************************/
/*------------------------------------------------------------------
 * Func : RHAL_TPP_Init
 *
 * Desc : init TPP MODULE
 *
 * Parm : tpp_id : TPP Index
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPP_Init(IN TPK_TPP_ENGINE_T tpp_id)
{
#ifdef TP_SUPPORT_CI

        INT32 ret;
        CHECK_MAX_TP_P_COUNT(tpp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        ret = Tp_TPP_Init(tpp_id);
        if(ret != TPK_SUCCESS) {
                TP_ERROR("Call Tp_TPP_Init() fail tpp_id=%u ret=%d\n", tpp_id,ret);
                _tp_unlock();
                return ret;
        }

        /* init mass buffer */
        ret = Tp_TPP_MassBuffInit(tpp_id);
        if( ret != TPK_SUCCESS ) {
                TP_ERROR("TP%u init buffer buffer error (%d)\n", tpp_id, ret);
                _tp_unlock();
                return ret;
        }

        /* reset  Counter*/
        Tp_TPP_ResetCounter(tpp_id);

        /* update status */
        pTp_drv->tpp[tpp_id].tpp_src				= Internal_Demod;
        pTp_drv->tpp[tpp_id].tpp_stream_status 		= TPP_STREAM_STOP;
        pTp_drv->tpp[tpp_id].tpp_param.sync_byte 	= 0x47;
        pTp_drv->tpp[tpp_id].Inited					= 1;

        _tp_unlock();

        TP_TRACE("%s exit, id = %d\n",__func__, tpp_id);

        return TPK_SUCCESS;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif

}

/*------------------------------------------------------------------
 * Func : RHAL_TPP_Uninit
 *
 * Desc : uninit TPP MODULE
 *
 * Parm : tpp_id : TPP Index
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPP_Uninit(IN TPK_TPP_ENGINE_T tpp_id)
{
#ifdef TP_SUPPORT_CI
        tp_tp_tf0_p_cntl_RBUS tp_p_cntl;

        CHECK_MAX_TP_P_COUNT(tpp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        if(pTp_drv->tpp[tpp_id].Inited == 0) {
                TP_WARNING("TPP%d did not init yet\n", tpp_id);
                _tp_unlock();
                return TPK_SUCCESS;
        }

        /* reset TPP framer state machine */
        tp_p_cntl.regValue   = 0;
        tp_p_cntl.rst_en     = 1;
        Tp_TPP_SetCtrlReg(tpp_id, tp_p_cntl.regValue, 1);
        ndelay(110);
        Tp_TPP_SetCtrlReg(tpp_id, tp_p_cntl.regValue, 0);
        ndelay(110);

        //disable output data to tpo
        Tp_TPP_OutEnable(tpp_id,0);

        //stream stop
        TP_TPP_StreamControl(tpp_id,TPP_STREAM_STOP);

        /* release tpp mass buffer */
        if(pTp_drv->tpp[tpp_id].mass_buffer.inited) {
                dvr_free((UINT8 *)pTp_drv->tpp[tpp_id].mass_buffer.BaseVir) ;
                pTp_drv->tpp[tpp_id].mass_buffer.BaseVir = 0;
                pTp_drv->tpp[tpp_id].mass_buffer.inited = 0;
        }

        pTp_drv->tpp[tpp_id].Inited				= 0;
        pTp_drv->tpp[tpp_id].tpp_src			= TS_UNMAPPING;
        pTp_drv->tpp[tpp_id].tpp_param.sync_byte = 0x47;
        pTp_drv->tpp[tpp_id].tpp_stream_status 	= TPP_STREAM_STOP;
        pTp_drv->tpp[tpp_id].is_connected		= 0;

        _tp_unlock();

        TP_TRACE("%s exit, id = %d\n",__func__, tpp_id);

        return TPK_SUCCESS;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif

}
EXPORT_SYMBOL(RHAL_TPP_Uninit);

/*------------------------------------------------------------------
 * Func : RHAL_TPP_SetMode
 *
 * Desc : Set TS Input mode of TP MODULE
 *
 * Parm : id : TPP Index
 *    param : TPK_TPP_TS_PARAM_T
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPP_SetMode(
        IN TPK_TPP_ENGINE_T    tpp_id,
        IN TPK_TPP_TS_PARAM_T  Param
)
{
#ifdef TP_SUPPORT_CI
        INT32 ret = TPK_SUCCESS;

        tp_tp_tf0_p_frmcfg_RBUS tp_p_frmcfg_reg;
        tp_tp_tf0_p_cntl_RBUS tp_p_cntl;

        TP_TRACE("%s enter, tpp_id=%d, serial=%u, pid_filter_en=%u\n",__func__, tpp_id, Param.serial, Param.pid_filter_en);

        if (Param.ts_in_tsp_len > byte_204) {
                TP_WARNING("%s bad parameter, ts_in_tsp_len=%u\n", __func__,Param.ts_in_tsp_len);
                return TPK_BAD_PARAMETER;
        }

        CHECK_MAX_TP_P_COUNT(tpp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        Tp_TPP_GetFrmReg(tpp_id, &tp_p_frmcfg_reg.regValue);
        tp_p_frmcfg_reg.serial = Param.serial;
        tp_p_frmcfg_reg.data_order = Param.data_order;
        tp_p_frmcfg_reg.datapin = Param.datapin;
        tp_p_frmcfg_reg.packet_size = Param.ts_in_tsp_len;
        tp_p_frmcfg_reg.sync_byte = Param.sync_byte;
        Tp_TPP_SetFrmReg(tpp_id, tp_p_frmcfg_reg.regValue);

        tp_p_cntl.regValue   = 0;
        tp_p_cntl.pid_en   = 1;
        Tp_TPP_SetCtrlReg(tpp_id, tp_p_cntl.regValue, Param.pid_filter_en);

        pTp_drv->tpp[tpp_id].tpp_param = Param;

        _tp_unlock();

        TP_TRACE("%s exit, tp_id=%d, serial=%u, pid_filter_en=%u\n",
                 __func__,tpp_id, Param.serial, Param.pid_filter_en);

        return ret;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif
}
/*------------------------------------------------------------------
 * Func : RHAL_TPP_GetMode
 *
 * Desc : Set TS Input mode of TP MODULE
 *
 * Parm : id : TPP Index
 * pParam : TPK_TPP_TS_PARAM_T
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPP_GetMode(
        IN TPK_TPP_ENGINE_T    tpp_id,
        OUT TPK_TPP_TS_PARAM_T  *pParam
)
{
#ifdef TP_SUPPORT_CI
        INT32 ret = TPK_SUCCESS;
        tp_tp_tf0_p_frmcfg_RBUS tp_p_frmcfg_reg;
        tp_tp_tf0_p_cntl_RBUS tpp_tf_cntl_data;

        TP_TRACE("%s enter, tpp_id=%d\n",__func__, tpp_id);

        if (pParam == NULL) {
                TP_WARNING("%s bad parameter pParam=%p\n",__func__, pParam);
                return TPK_BAD_PARAMETER;
        }
        CHECK_MAX_TP_P_COUNT(tpp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        Tp_TPP_GetFrmReg(tpp_id, &tp_p_frmcfg_reg.regValue);

        pParam->serial = tp_p_frmcfg_reg.serial;
        pParam->data_order = tp_p_frmcfg_reg.data_order;
        pParam->datapin = tp_p_frmcfg_reg.datapin;
        pParam->ts_in_tsp_len = tp_p_frmcfg_reg.packet_size;
        pParam->sync_byte = tp_p_frmcfg_reg.sync_byte;

        Tp_TPP_GetCtrlReg(tpp_id, (UINT32*)&tpp_tf_cntl_data);
        pParam->pid_filter_en = tpp_tf_cntl_data.pid_en;

        _tp_unlock();

        TP_TRACE("%s exit, tp_id=%d, serial=%u, ts_in_tsp_len=%u\n",
                 __func__,tpp_id, pParam->serial, pParam->ts_in_tsp_len);


        return ret;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif
}
EXPORT_SYMBOL(RHAL_TPOUT_SetDataSource);

/*------------------------------------------------------------------
 * Func : RHAL_TPP_EnablePIDFilter
 *
 * Desc : Set TPP PID filter On/Off
 *
 * Parm : [I] id : TPP ID
 *        [I] on : Enable TPP PID filter or not
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/

INT32 RHAL_TPP_EnablePIDFilter(
        IN TPK_TPP_ENGINE_T   tpp_id,
        IN UINT8             on
)
{
#ifdef TP_SUPPORT_CI
        INT32 ret = TPK_SUCCESS;
        tp_tp_tf0_p_cntl_RBUS tpp_cntl;

        TP_TRACE("%s enter, tpp_id=%u, on=%u\n",__func__, tpp_id, on);

        CHECK_MAX_TP_P_COUNT(tpp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        tpp_cntl.regValue = 0;
        tpp_cntl.pid_en = 1; /* set select bit = 1 */

        Tp_TPP_SetCtrlReg(tpp_id, tpp_cntl.regValue, on);

        pTp_drv->tpp[tpp_id].tpp_param.pid_filter_en = on;

        _tp_unlock();


        TP_TRACE("%s exit, tpp_id= %u, on = %u\n",__func__, tpp_id, on);

        return ret;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif
}

/*------------------------------------------------------------------
 * Func : RHAL_TPP_StreamControl
 *
 * Desc : set tpp stream control
 *
 * Parm : [I] tpp_id   : TPP Index
 *        [I] ctrl :  control
 *
 *            TPP_STREAM_START: Start Steaming
 *            TPP_STREAM_STOP : Stop Streaming
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPP_StreamControl(IN TPK_TPP_ENGINE_T tpp_id, IN TPK_TPP_STREAM_CTRL_T ctrl)
{
#ifdef TP_SUPPORT_CI
        INT32 ret = TPK_SUCCESS;

        TP_TRACE("fun:%s enter, tpp_id= %u, tpp_src = %u\n", __func__,tpp_id,ctrl);

        CHECK_MAX_TP_P_COUNT(tpp_id);
        CHECK_TP_STRUCTURE();

        if ((ctrl != TPP_STREAM_START) && (ctrl != TPP_STREAM_STOP)) {
                TP_ERROR("%s bad parameter ctrl=%d!!", __func__,ctrl);
                return TPK_BAD_PARAMETER;
        }

        _tp_lock();

        if (pTp_drv->tpp[tpp_id].tpp_stream_status == ctrl) {
                _tp_unlock();
                return TPK_SUCCESS;
        }

        ret = TP_TPP_StreamControl(tpp_id, ctrl);

        Tp_TPP_CheckRingBuffer();

        pTp_drv->tpp[tpp_id].tpp_stream_status = ctrl;

        _tp_unlock();

        TP_TRACE("fun:%s exit, tpp_id= %u, ctrl = %u\n", __func__,tpp_id,ctrl);
        tp_mask_print_emerg( TP_LOG_BLOCK_ALL, TP_LOG_LEVEL_WARNING, "fun:%s exit, tpp_id= %u, ctrl = %u\n", __func__,tpp_id,ctrl);

        return ret;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif
}
EXPORT_SYMBOL(RHAL_TPP_StreamControl);

/*------------------------------------------------------------------
 * Func : RHAL_TPP_OutEnable
 *
 * Desc : TPP Out data to TPO
 *
 * Parm : [I] tpout_id : TP out Index
 *        [I] ctrl     : TP out countrl
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPP_OutEnable(
        IN TPK_TPP_ENGINE_T tpp_id,
        IN UINT8            enable
)
{
#ifdef TP_SUPPORT_CI
        INT32 ret = TPK_SUCCESS;

        CHECK_MAX_TP_P_COUNT(tpp_id);
        CHECK_TP_STRUCTURE();

        tp_mask_print_emerg( TP_LOG_BLOCK_ALL, TP_LOG_LEVEL_WARNING, "%s Enter, tpp_id=%d, enable=%u\n", __func__,tpp_id, enable);

        _tp_lock();

        if(!pTp_drv->tpp[tpp_id].mass_buffer.inited) {
                TP_WARNING("%s TPP%d did not init yet or tpp mass buffer has not inited yet\n", __func__,tpp_id);
                _tp_unlock();
                return TPK_FAIL;
        }

        if (!pTp_drv->tpp[tpp_id].is_connected) {
                TP_WARNING("%s TPP%d has not connect TPO yet\n", __func__,tpp_id);
                _tp_unlock();
                return TPK_FAIL;
        }
#if 0
        if (pTp_drv->tpp[tpp_id].ddr_out_enable == enable) {
                _tp_unlock();
                return TPK_SUCCESS;
        }
#endif
        ret = Tp_TPP_OutEnable(tpp_id, enable);
        if(ret == TPK_SUCCESS){
                pTp_drv->tpp[tpp_id].ddr_out_enable = enable;
        }

        _tp_unlock();

        tp_mask_print_emerg( TP_LOG_BLOCK_ALL, TP_LOG_LEVEL_WARNING, "%s exit, tpp_id=%d, enable=%u\n",__func__, tpp_id, enable);

        return ret;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif
}
EXPORT_SYMBOL(RHAL_TPP_OutEnable);

/*------------------------------------------------------------------
 * Func : RHAL_TPP_SetSyncReplace
 *
 * Desc : set tpp sync replace
 *
 * Parm : [I] tpp_id   : TPP Index
 *		  [I] sync_byte : 0x47/0x48/0x49
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPP_SetSyncReplace(TPK_TPP_ENGINE_T tpp_id, UINT8 sync_byte )
{
#ifdef TP_SUPPORT_CI
        INT32 ret = TPK_SUCCESS;

        TP_TRACE("fun:%s enter, tpp_id= %u, sync_byte = %u\n", __func__,tpp_id,sync_byte);

        CHECK_MAX_TP_P_COUNT(tpp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        ret = Tp_TPP_SetSyncReplace(tpp_id, sync_byte);

        pTp_drv->tpp[tpp_id].tpp_param.sync_byte = sync_byte;

        _tp_unlock();

        TP_TRACE("fun:%s exit, tpp_id= %u, sync_byte = %u\n", __func__,tpp_id,sync_byte);

        return ret;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif

}
EXPORT_SYMBOL(RHAL_TPP_SetSyncReplace);

/*------------------------------------------------------------------
 * Func : RHAL_TPP_SetDataSource
 *
 * Desc : Set TPP soruce
 *
 * Parm : [I] tpp_id : TPP ID
 *        [I] tpp_src : tpp source
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPP_SetDataSource(IN TPK_TPP_ENGINE_T tpp_id, IN TPK_TP_SOURCE_T tpp_src)
{
#ifdef TP_SUPPORT_CI
        INT32 ret = TPK_SUCCESS;

        TP_TRACE("fun:%s enter, tpp_id= %u, tpp_src = %u\n", __func__,tpp_id,tpp_src);

        CHECK_MAX_TP_P_COUNT(tpp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        ret = Tp_TPP_Init(tpp_id);
        if(ret != TPK_SUCCESS) {
                _tp_unlock();
                TP_INFO("exit, tpp_id=%u, Tp_TPP_Init	fail\n", tpp_id);
                return ret;
        }

        Tp_TPP_CheckRingBuffer();

        /* disable tp' */
        if (tpp_src == TS_UNMAPPING) {
                pTp_drv->tpp[tpp_id].is_connected 	= 0;
                pTp_drv->tpp[tpp_id].tpp_src	 	= TS_UNMAPPING;
                pTp_drv->tpp[tpp_id].tpp_param.sync_byte = 0x47;
                Tp_TPP_SetSyncReplace(tpp_id, 0x47);
                _tp_unlock();

                TP_INFO("%s exit, set to tpp_id=%u, remux_src=%u \n", __func__, tpp_id,tpp_src);
                return TPK_SUCCESS;
        }
        /* set tpp source */
        ret = Tp_TPP_SetDataSource(tpp_id, tpp_src);

        Tp_TPP_ResetCounter(tpp_id);

        if(pTp_drv->tpp[tpp_id].tpp_src != tpp_src)
                Tp_TPP_FlushBuf(tpp_id);

        pTp_drv->tpp[tpp_id].is_connected 	= 1;
        pTp_drv->tpp[tpp_id].tpp_src		= tpp_src;

        pTp_drv->tpi_log_print[tpp_id] = TPI_LOG_CAM;

        _tp_unlock();

        TP_TRACE("fun:%s exit, tpp_id= %u, tpp_src = %u\n", __func__,tpp_id,tpp_src);

        return ret;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif
}
EXPORT_SYMBOL(RHAL_TPP_SetDataSource);

INT32 RHAL_TPP_GetDataSource(IN TPK_TPP_ENGINE_T tpp_id){
        CHECK_MAX_TP_P_COUNT(tpp_id);
        CHECK_TP_STRUCTURE();

        return pTp_drv->tpp[tpp_id].tpp_src;
}
EXPORT_SYMBOL(RHAL_TPP_GetDataSource);

/*------------------------------------------------------------------
 * Func : RHAL_TPP_FlushBuffer
 *
 * Desc :
 *
 * Parm : [I] id : TPP Index
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPP_FlushBuffer(IN TPK_TPP_ENGINE_T tpp_id)
{
#ifdef TP_SUPPORT_CI
        tp_tp_p_ring_ctrl_RBUS tp_p_ring_ctrl_reg;
        UADDRESS Base, Limit;


        CHECK_MAX_TPP_COUNT(tpp_id);

        _tp_lock();

        /*stop tpp frame first*/
        if (pTp_drv->tpp[tpp_id].tpp_stream_status == TPP_STREAM_START) {
                TP_TPP_StreamControl(tpp_id,TPP_STREAM_STOP);
        }

        /*read ring buffer*/
        tp_p_ring_ctrl_reg.regValue = 0;
        tp_p_ring_ctrl_reg.idx = tpp_id;
        tp_p_ring_ctrl_reg.r_w = 0; /* read */
        WRITE_REG32(TP_TP_P_RING_CTRL_reg, tp_p_ring_ctrl_reg.regValue);

        Base  = READ_REG32(TP_TP_P_RING_BASE_reg);
        Limit = READ_REG32(TP_TP_P_RING_LIMIT_reg);

        /*write ring buffer*/
        WRITE_REG32(TP_TP_P_RING_LIMIT_reg, Limit);
        WRITE_REG32(TP_TP_P_RING_BASE_reg, Base);
        WRITE_REG32(TP_TP_P_RING_RP_reg, Base);
        WRITE_REG32(TP_TP_P_RING_WP_reg, Base);

        tp_p_ring_ctrl_reg.regValue = 0;
        tp_p_ring_ctrl_reg.idx = tpp_id;
        tp_p_ring_ctrl_reg.r_w = 1; /* write */
        WRITE_REG32(TP_TP_P_RING_CTRL_reg, tp_p_ring_ctrl_reg.regValue);

        /*if start,set start last*/
        if (pTp_drv->tpp[tpp_id].tpp_stream_status == TPP_STREAM_START) {
                TP_TPP_StreamControl(tpp_id,TPP_STREAM_START);
        }

        _tp_unlock();

        TP_TRACE("%s tpp_id[%u]  mode = flush \n", __func__,tpp_id);

        return TPK_SUCCESS;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif
}

/*------------------------------------------------------------------
 * Func : RHAL_TPP_GetBufferStatus
 *
 * Desc : Get TPP Buffer Status
 *
 * Parm :
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPP_GetBufferStatus(
        IN  TPK_TPP_ENGINE_T  tpp_id,
        OUT UINT32*           pBufferSize,
        OUT UINT32*           pDataSize)
{
#ifdef TP_SUPPORT_CI
        UADDRESS Base;
        UADDRESS Limit;
        UADDRESS RP;
        UADDRESS WP;
        tp_tp_p_ring_ctrl_RBUS tp_p_ring_ctrl_reg;

        CHECK_MAX_TPP_COUNT(tpp_id);


        if ((pBufferSize == NULL) || (pDataSize == NULL) ) {
                TP_WARNING("%s BAD_PARAMETER tpp_id[%u] pBufferSize = %p, pDataSize = %p\n",__func__,
                           tpp_id, pBufferSize, pDataSize);
                return TPK_BAD_PARAMETER;
        }

        _tp_lock();

        tp_p_ring_ctrl_reg.regValue = 0;
        tp_p_ring_ctrl_reg.idx = tpp_id;
        tp_p_ring_ctrl_reg.r_w = 0; /* read */
        WRITE_REG32(TP_TP_P_RING_CTRL_reg, tp_p_ring_ctrl_reg.regValue);

        Base  = READ_REG32(TP_TP_P_RING_BASE_reg);
        RP    = READ_REG32(TP_TP_P_RING_RP_reg);
        WP    = READ_REG32(TP_TP_P_RING_WP_reg);
        Limit = READ_REG32(TP_TP_P_RING_LIMIT_reg);

        *pBufferSize = Limit - Base;
        *pDataSize = (WP >= RP) ?
                     (WP - RP) :
                     (*pBufferSize - (RP - WP));
        _tp_unlock();

        TP_TRACE("%s exit tpp_id[%u] pBufferSize=%u, pDataSize=%u\n",
                 __func__,tpp_id, *pBufferSize, *pDataSize);

        return TPK_SUCCESS;
#else
        TP_ERROR("%s Not Support!!", __func__);
        return TPK_FAIL;
#endif
}

INT32  RHAL_TPP_Delivery_Data(UINT8 id, char *buffer, UINT32 *buffer_size)
{
	int result=0;
	UINT32 len = 0;
	TPK_TPP_ENGINE_T tpp_id = id;
	unsigned char *p_rp = NULL;

	CHECK_MAX_TP_P_COUNT(tpp_id);

	result = Tp_TPP_ReadData(tpp_id, &p_rp, &len, 1024);
	if(result < 0 && result != TPK_BUFFER_FULL){
		TP_VERBOSE("%s Tp_TPP_ReadData data failed, result=%d\n", __func__, result);
		*buffer_size = 0;
		return result;
	}else{
		TP_VERBOSE("Get TPP%u data return=%d (len=%u, ptr=%p)\n", tpp_id, result, len, p_rp);
	}

	p_rp = Tp_TPP_Buf_Phy_to_Vir(tpp_id, (unsigned char*)p_rp);

	if(len > *buffer_size)
		len = *buffer_size;

	if(copy_to_user((void *)buffer, (const void __user *)p_rp, len)) {
		TP_ERROR("%s copy_to_user Fail\n", __func__);
		return -1;
	} else {
		*buffer_size = len;
	}

	p_rp = Tp_TPP_Buf_Vir_to_Phy(tpp_id, (unsigned char*)p_rp);

	Tp_TPP_ReleaseData(tpp_id, p_rp, len);
	return result;
}

INT32  RHAL_Delivery_TPP_SetInputConfig(UINT8 id, TPK_INPUT_PORT_T inputPort, TPK_INPUT_TYPE_T inputType)
{
	TPK_TPP_ENGINE_T tpp_id;
	TPK_TP_SOURCE_T tpp_src, tp_src;
	TPK_TP_STATUS_T tpStatus;

	TP_WARNING("[%s, line %d] ch:%d portType:%d inputType:%d\n",  __func__, __LINE__, id, inputPort, inputType);

	CHECK_MAX_TP_COUNT(id)

	tpp_id = id;
	CHECK_MAX_TPP_COUNT(tpp_id);

	tp_src = RHAL_GetTPSource(inputPort, 0);
	if (RHAL_GetTpStatus(id, &tpStatus) != TPK_SUCCESS)
		return TPK_FAIL;

	if(inputType!=TPK_INPUT_DISABLE)
	{
		tpp_src = tp_src;
		if(inputType == TPK_INPUT_MEM)
			tpp_src = MTP ;

		//init tpp ,allocate tpp ring buffer
		RHAL_TPP_Init(tpp_id);

		//set tpp source
		if (RHAL_TPP_SetDataSource(tpp_id, tpp_src) != TPK_SUCCESS)
			return TPK_FAIL;

	    if (RHAL_TPP_SetSyncReplace(tpp_id, 0x47) != TPK_SUCCESS)
			return TPK_FAIL;

		//set tpp stream start
		if (RHAL_TPP_StreamControl(tpp_id, TPP_STREAM_START) != TPK_SUCCESS)
			return TPK_FAIL;

		if(pTp_drv->deliveryMode[id] == TP_DELIVERY_MODE_CI20 && tpStatus.tp_param.prefix_en){
			tpStatus.tp_param.ts_in_tsp_len = byte_188;
			tpStatus.tp_param.frc_en = 0;
			tpStatus.tp_param.keep = 0;
			TP_WARNING("[CI2.0]tpp read set in tsp len to 188\n");
			if (RHAL_SetTPMode(id, tpStatus.tp_param) != TPK_SUCCESS)
			{
				TP_ERROR("[%s %d] - RHAL_SetTPMode fail!\n",  __func__, __LINE__);
				return -1;
			}
		}

		TP_WARNING("%s Set CI channel to TPP%d! tpp_src=%d\n", __func__, tpp_id, tpp_src);
	}else{
		//set tpp stream stop
		if (RHAL_TPP_StreamControl(tpp_id, TPP_STREAM_STOP) != TPK_SUCCESS)
			return TPK_FAIL;

		if (RHAL_TPP_OutEnable(tpp_id, 0) != TPK_SUCCESS)
			return TPK_FAIL;

		if(pTp_drv->deliveryMode[id] == TP_DELIVERY_MODE_CI20 && tpStatus.tp_param.prefix_en){
			tpStatus.tp_param.frc_en = tpStatus.tp_param.prefix_en;
			tpStatus.tp_param.ts_in_tsp_len = (tpStatus.tp_param.prefix_en) ? byte_192 : byte_188;
			tpStatus.tp_param.keep = (tpStatus.tp_param.prefix_en)? 1:0;
			TP_WARNING("[CI2.0]tpp read set in tsp len to default value\n");
		}

	}
	return TPK_SUCCESS;
}

/*------------------------------------------------------------------
 * Func : RHAL_SetMTPBuffer
 *
 * Desc : Set MTP Buffer
 *
 * Parm : [I] mtp_id   : MTP Index
 *        [I] pPhyAddr : start address of MTP buffer
 *        [I] size     : size of MTP burrer
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_SetMTPBuffer(
        TPK_TP_MTP_T      mtp_id,
        UINT8*            pPhyAddr,
        UINT8*            pVirAddr,
        UINT32            size)
{
        TPK_MTP_BUF_T *pMtpBufParam;

        REGADDR m2m_limit_reg;
        REGADDR m2m_base_reg;
        REGADDR m2m_rp_reg;
        REGADDR m2m_wp_reg;

        UINT8 is_mtp_running = 0;

        switch (mtp_id) {
                case TP_TP0_MTP:
                        m2m_limit_reg = TP_TP0_M2M_RING_LIMIT_reg;
                        m2m_base_reg  = TP_TP0_M2M_RING_BASE_reg;
                        m2m_rp_reg    = TP_TP0_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP0_M2M_RING_WP_reg;
                        break;
                case TP_TP1_MTP:
                        m2m_limit_reg = TP_TP1_M2M_RING_LIMIT_reg;
                        m2m_base_reg  = TP_TP1_M2M_RING_BASE_reg;
                        m2m_rp_reg    = TP_TP1_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP1_M2M_RING_WP_reg;
                        break;
                case TP_TP2_MTP:
                        m2m_limit_reg = TP_TP2_M2M_RING_LIMIT_reg;
                        m2m_base_reg  = TP_TP2_M2M_RING_BASE_reg;
                        m2m_rp_reg    = TP_TP2_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP2_M2M_RING_WP_reg;
                        break;
#ifdef TP_SUPPORT_TP3
                case TP_TP3_MTP:
                        m2m_limit_reg = TP_TP3_M2M_RING_LIMIT_reg;
                        m2m_base_reg  = TP_TP3_M2M_RING_BASE_reg;
                        m2m_rp_reg    = TP_TP3_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP3_M2M_RING_WP_reg;
                        break;
#endif
#ifdef TP_SUPPORT_CI
                case TP_TPP0_MTP:
                        m2m_limit_reg = TP_TP0_P_M2M_RING_LIMIT_reg;
                        m2m_base_reg  = TP_TP0_P_M2M_RING_BASE_reg;
                        m2m_rp_reg    = TP_TP0_P_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP0_P_M2M_RING_WP_reg;
                        break;
                case TP_TPP1_MTP:
                        m2m_limit_reg = TP_TP1_P_M2M_RING_LIMIT_reg;
                        m2m_base_reg  = TP_TP1_P_M2M_RING_BASE_reg;
                        m2m_rp_reg    = TP_TP1_P_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP1_P_M2M_RING_WP_reg;
                        break;
                case TP_TPP2_MTP:
                        m2m_limit_reg = TP_TP2_P_M2M_RING_LIMIT_reg;
                        m2m_base_reg  = TP_TP2_P_M2M_RING_BASE_reg;
                        m2m_rp_reg    = TP_TP2_P_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP2_P_M2M_RING_WP_reg;
                        break;
#endif
                default:
                        TP_WARNING("bad parameter (mtp_id=%u)\n", mtp_id);
                        return TPK_BAD_PARAMETER;
                        break;
        }

        CHECK_MTP_ID(mtp_id);
        CHECK_TP_STRUCTURE();

        if ((pPhyAddr == NULL) && (pVirAddr == NULL) && (size == 0)) {
                TP_WARNING("MTP%d: Bad parameter to set MTP buffer!(pPhyAddr=%p, size=%u\n", mtp_id, pPhyAddr, size);
                pTp_drv->mtp[mtp_id].valid = 0;
                return TPK_SUCCESS;
        }

        if(pPhyAddr == (UINT8*)pTp_drv->mtp[mtp_id].mtp_buf_param.BasePhy && (UINT8*)pTp_drv->mtp[mtp_id].mtp_buf_param.BaseVir == pVirAddr && pTp_drv->mtp[mtp_id].mtp_buf_param.BufSize == size && pTp_drv->mtp[mtp_id].valid == 1) {
                TP_WARNING("MTP:%d: buffer is the same, not need to set again pPhyAddr=0x%p pVirAddr=0x%p size=0x%x\n", mtp_id, pPhyAddr, pVirAddr, size);
                return TPK_SUCCESS;
        }

        TP_DBG("MTP%d: pPhyAddr=0x%p pVirAddr=0x%p size=0x%x\n", mtp_id, pPhyAddr, pVirAddr, size);

        _tp_lock();

        if (pTp_drv->mtp[mtp_id].mtp_stream_status == MTP_STREAM_START) {
                TP_WARNING("MTP%d: stream is running now, set mtp buffer\n", mtp_id);
                is_mtp_running = 1;
        }

        if(is_mtp_running == 1)
                TP_MTPStreamControl(mtp_id, MTP_STREAM_STOP);

        /* save data into structure */
        pTp_drv->mtp[mtp_id].mtp_buf_param.BasePhy  = (UADDRESS)pPhyAddr;
        pTp_drv->mtp[mtp_id].mtp_buf_param.BaseVir  = (UADDRESS)pVirAddr;
        pTp_drv->mtp[mtp_id].mtp_buf_param.BufSize  = size;
        pTp_drv->mtp[mtp_id].mtp_buf_param.LimitPhy = (UADDRESS)pPhyAddr + size;
        pTp_drv->mtp[mtp_id].mtp_buf_param.RP       = (UADDRESS)pPhyAddr;
        pTp_drv->mtp[mtp_id].mtp_buf_param.WP       = (UADDRESS)pPhyAddr;
        pTp_drv->mtp[mtp_id].mtp_buf_param.inited   = 1;
        pTp_drv->mtp[mtp_id].valid = 1;

        /* set m2m memory dma parameter */
        pMtpBufParam = &pTp_drv->mtp[mtp_id].mtp_buf_param;
        WRITE_REG32(m2m_limit_reg, pMtpBufParam->LimitPhy);
        WRITE_REG32(m2m_base_reg, pMtpBufParam->BasePhy);

        pMtpBufParam->RP = pMtpBufParam->WP = pMtpBufParam->BasePhy;
        WRITE_REG32(m2m_rp_reg, pMtpBufParam->RP);
        WRITE_REG32(m2m_wp_reg, pMtpBufParam->WP);

        /* set a reasonable value to MTP0 to avoid MTP0 invalid read while in_go=1 -> in_go=0 */
        if (pTp_drv->mtp[0].valid == 0) {
                WRITE_REG32(TP_TP0_M2M_RING_LIMIT_reg, pMtpBufParam->LimitPhy);
                WRITE_REG32(TP_TP0_M2M_RING_BASE_reg, pMtpBufParam->BasePhy);
                WRITE_REG32(TP_TP0_M2M_RING_RP_reg, pMtpBufParam->RP);
                WRITE_REG32(TP_TP0_M2M_RING_WP_reg, pMtpBufParam->WP);
        }

        if(is_mtp_running == 1)
                TP_MTPStreamControl(mtp_id, MTP_STREAM_START);

        _tp_unlock();

        TP_TRACE("exit, MTP%d: pPhyAddr=%p, size=%u\n", mtp_id, pPhyAddr, size);

        return TPK_SUCCESS;
}

EXPORT_SYMBOL(RHAL_SetMTPBuffer);

/*------------------------------------------------------------------
 * Func : RHAL_MTP_TPOutEnable
 *
 * Desc : MTP0 ring buffer output to TPO  Control
 *
 * Parm : [I] mtp_id : MTP Index
 *        [I] on   :  onoff
 *        only MTP0 support output to TPO directly
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_MTP_TPOutEnable(TPK_TP_MTP_T mtp_id,UINT8 enable)
{
        tp_tp0_m2m_ring_ctrl_RBUS m2m_ctrl;
        INT32 ret = TPK_SUCCESS;

        CHECK_MTP_ID(mtp_id);
        CHECK_TP_STRUCTURE();

        if (mtp_id != TP_TP0_MTP) {
                TP_WARNING("%s bad parameter only MTP0 can support output to TPO directly, mtp_id = %u,enable=%d\n", __func__, mtp_id, enable);
                return TPK_BAD_PARAMETER;
        }

        _tp_lock();

        if (pTp_drv->tpout[0].tpout_src == TPK_TP_MTP_TO_TPOUT) { /* TPO from MTP0 */
                m2m_ctrl.regValue = 0;
                if(enable){
                        m2m_ctrl.go   = 1;
                        m2m_ctrl.in_go   = 0;
                }else{
                        m2m_ctrl.go   = 0;
                        m2m_ctrl.in_go   = 0;
                }
                Tp_Write_Data_Reg_Write(TP_TP0_M2M_RING_CTRL_reg, m2m_ctrl.regValue);
        }else{
                ret =	TPK_FAIL;
                TP_WARNING("%s fail, TPO data source is not TPK_TP_MTP_TO_TPOUT, mtp_id = %u, tpout_src=%d\n", __func__, mtp_id, pTp_drv->tpout[0].tpout_src);
        }

        _tp_unlock();

        TP_TRACE("fun:%s exit, mtp_id= %u, mtp_id = %u\n", __func__,mtp_id,enable);
        return ret;

}


/*------------------------------------------------------------------
 * Func : RHAL_MTPStreamControl
 *
 * Desc : MTP Streaming Control
 *
 * Parm : [I] mtp_id : MTP Index
 *        [I] ctrl   : MTP control
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_MTPStreamControl(
        TPK_TP_MTP_T          mtp_id,
        TPK_MTP_STREAM_CTRL_T ctrl)
{

#if 0
        UINT8 i;

        for (i = 0; i < MAX_TP_COUNT; i++) {
                if (pTp_drv->tp[i].tp_src == MTP) {
                        break;
                }
        }
#endif

        INT32 ret;

        CHECK_MTP_ID(mtp_id);
        CHECK_TP_STRUCTURE();

        if ((ctrl != MTP_STREAM_START) && (ctrl != MTP_STREAM_STOP)) {
                TP_WARNING("bad parameter of %s mtp_id = %u, ctrl = %u\n", __func__, mtp_id, ctrl);
                return TPK_BAD_PARAMETER;
        }

        _tp_lock();

        CHECK_MTP_BUF_VALID(mtp_id);

        if (pTp_drv->mtp[mtp_id].mtp_stream_status == ctrl) {
                _tp_unlock();

                TP_WARNING("MTP%d: stream status is the same,no change.mtp_stream_status: %d\n", mtp_id,ctrl);

                return TPK_SUCCESS;
        }

        ret = TP_MTPStreamControl(mtp_id, ctrl);

        _tp_unlock();

        TP_DBG("MTP%d: stream status = %u ret=%d\n", mtp_id, ctrl, ret);
        if( CHECK_LOG_BLOCK_MASK(TP_LOG_BLOCK_MTP) ) {
                TP_DBG("MTP%d: stream status = %u ret=%d\n", mtp_id, ctrl, ret);
        }
        return ret;
}

EXPORT_SYMBOL(RHAL_MTPStreamControl);



/*------------------------------------------------------------------
 * Func : RHAL_WriteMTPBuffer
 *
 * Desc : Write MTP Buffer
 *
 * Parm : [I] mtp_id : MTP Index
 *        [I] pPhyWP : Write pointer
 *        [I] ContinueWrite : Continuout Write pointer
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_WriteMTPBuffer(
        TPK_TP_MTP_T        mtp_id,
        UINT8*              pPhyWP,
        UINT32              ContinueWrite)
{
        TPK_MTP_BUF_T *pBuf_param;
        UADDRESS new_wp;
        UADDRESS writeableSize;
#ifndef TP_VERIFY_ENABLE
#ifndef TP_TIMESTAMP_USE_27M
        int i;
        UINT32 now_timestamp;
        UINT8 *p_now_timestamp;
#endif
#endif
        TP_BUFFER_PARAM_T ring_buf;
        unsigned char* pucWritePtr1;

        REGADDR m2m_rp_reg;
        REGADDR m2m_wp_reg;
#if 0
        UINT8 i;

        for (i = 0; i < MAX_TP_COUNT; i++) {
                if (pTp_drv->tp[i].tp_src == MTP) {
                        break;
                }
        }
#endif

        switch (mtp_id) {
                case TP_TP0_MTP:
                        m2m_rp_reg    = TP_TP0_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP0_M2M_RING_WP_reg;
                        break;
                case TP_TP1_MTP:
                        m2m_rp_reg    = TP_TP1_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP1_M2M_RING_WP_reg;
                        break;
                case TP_TP2_MTP:
                        m2m_rp_reg    = TP_TP2_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP2_M2M_RING_WP_reg;
                        break;
#ifdef TP_SUPPORT_TP3
                case TP_TP3_MTP:
                        m2m_rp_reg    = TP_TP3_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP3_M2M_RING_WP_reg;
                        break;
#endif
#ifdef TP_SUPPORT_CI
                case TP_TPP0_MTP:
                        m2m_rp_reg    = TP_TP0_P_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP0_P_M2M_RING_WP_reg;
                        break;
                case TP_TPP1_MTP:
                        m2m_rp_reg    = TP_TP1_P_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP1_P_M2M_RING_WP_reg;
                        break;
                case TP_TPP2_MTP:
                        m2m_rp_reg    = TP_TP2_P_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP2_P_M2M_RING_WP_reg;
                        break;
#endif
                default:
                        TP_WARNING("bad parameter (mtp_id=%u)\n", mtp_id);
                        return TPK_BAD_PARAMETER;
                        break;
        }

        CHECK_MTP_ID(mtp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        new_wp = (UADDRESS)pPhyWP + ContinueWrite;
        pBuf_param = &pTp_drv->mtp[mtp_id].mtp_buf_param;

        if( CHECK_LOG_BLOCK_MASK(TP_LOG_BLOCK_MTP) ) {
                Tp_Get_MTP_Buffer(mtp_id, &ring_buf);
                pucWritePtr1 = (unsigned char*)(pBuf_param->BaseVir + ((UADDRESS)ring_buf.RP - pBuf_param->BasePhy));
                TP_DBG_SIMPLE("MTP%d: %02x%02x%02x%02x pPhyWP=%p v=%p p="PT_FMT_UADDR"\n", mtp_id, pucWritePtr1[0], pucWritePtr1[1], pucWritePtr1[2], pucWritePtr1[3], (void*)pPhyWP, (void*)pucWritePtr1, ring_buf.RP);
        }

        CHECK_MTP_BUF_VALID(mtp_id);

        if(pTp_drv->mtp[mtp_id].mtp_stream_status != MTP_STREAM_START) {
                _tp_unlock();
                TP_WARNING("MTP%d: stream did not start yet\n", mtp_id);
                return TPK_NOT_SET;
        }

        pBuf_param->RP = READ_REG32(m2m_rp_reg);

        if (((UADDRESS)pPhyWP < pBuf_param->BasePhy) ||
            ((UADDRESS)pPhyWP >= pBuf_param->LimitPhy)) {
                _tp_unlock();

                TP_WARNING("MTP%d: invalid wp %p - out of boundrary ("PT_FMT_UADDR" ~ "PT_FMT_UADDR") \n",
                           mtp_id, pPhyWP, pBuf_param->BasePhy, pBuf_param->LimitPhy);

                return TPK_DATA_OUT_OF_RANGE;
        }

        if ((UADDRESS)pPhyWP != pBuf_param->WP) {
                _tp_unlock();

                TP_WARNING("MTP%d: wp not sync (input wp=%p / wp rec="PT_FMT_UADDR")\n",
                           mtp_id, pPhyWP, pBuf_param->WP);

                return TPK_RESOURCE_CONFLICT;
        }
        writeableSize = (pBuf_param->LimitPhy - (UADDRESS)pPhyWP);

        //if ((new_wp < pBuf_param->BasePhy) ||
         //   (new_wp > pBuf_param->LimitPhy)) {
        if (ContinueWrite > writeableSize) {
                _tp_unlock();

                TP_WARNING("MTP%d: new wp "PT_FMT_UADDR" - out of boundrary ("PT_FMT_UADDR" ~ "PT_FMT_UADDR") \n",
                           mtp_id, new_wp, pBuf_param->BasePhy, pBuf_param->LimitPhy);

                return TPK_DATA_OUT_OF_RANGE;
        }

        if (new_wp == pBuf_param->LimitPhy)
                new_wp = pBuf_param->BasePhy;

        /* MTP for TP */
        if ((mtp_id >= TP_TP0_MTP) && (mtp_id <= TP_TP3_MTP) && (pTp_drv->tp[mtp_id].tp_param.ts_in_tsp_len == byte_192)) {
                unsigned char* pucWritePtr;

                pucWritePtr = (unsigned char*)(pBuf_param->BaseVir + ((UADDRESS)pPhyWP - pBuf_param->BasePhy));
#ifndef TP_VERIFY_ENABLE
#ifndef TP_TIMESTAMP_USE_27M
                for(i = 0; i < ContinueWrite-4; i = i + 192) {
                        p_now_timestamp = (UINT8*)&now_timestamp;
                        p_now_timestamp[0] = pucWritePtr[3 + i];
                        p_now_timestamp[1] = pucWritePtr[2 + i];
                        p_now_timestamp[2] = pucWritePtr[1 + i];
                        p_now_timestamp[3] = pucWritePtr[0 + i];

                        if(pTp_drv->mtp[mtp_id].first_tsp == 1) {
                                pTp_drv->mtp[mtp_id].last_timestamp = now_timestamp;
                                pTp_drv->mtp[mtp_id].first_tsp = 0;
                        }

                        if( now_timestamp < pTp_drv->mtp[mtp_id].last_timestamp) {
                                // timestamp max value => 4294967100(0xFFFFFF3C)    timestamp max value div 300 => 14316557(0xDA740D)
                                if( (pTp_drv->mtp[mtp_id].last_timestamp - now_timestamp) >= 0xDA0000) {
                                        pTp_drv->mtp[mtp_id].timestmp_offset = pTp_drv->mtp[mtp_id].timestmp_offset + 14316557;
                                        TP_DBG("MTP%d: timestamp wrap around new=0x%x old=0x%x\n", mtp_id, now_timestamp, pTp_drv->mtp[mtp_id].last_timestamp);
                                } else {
                                        if( CHECK_LOG_BLOCK_MASK(TP_LOG_BLOCK_MTP) ) {
                                                TP_DBG("MTP%d: timestamp wrap around new=0x%x old=0x%x, diff value is small, not need to add offset\n", mtp_id, now_timestamp, pTp_drv->mtp[mtp_id].last_timestamp);
                                        }
                                }
                        }
                        pTp_drv->mtp[mtp_id].last_timestamp = now_timestamp;
                        now_timestamp = now_timestamp + pTp_drv->mtp[mtp_id].timestmp_offset;
                        pucWritePtr[3 + i] = p_now_timestamp[0];
                        pucWritePtr[2 + i] = p_now_timestamp[1];
                        pucWritePtr[1 + i] = p_now_timestamp[2];
                        pucWritePtr[0 + i] = p_now_timestamp[3];
                }
#endif
#endif
                dmac_flush_range(pucWritePtr, pucWritePtr + ContinueWrite);
                outer_flush_range((UADDRESS)pPhyWP, (UADDRESS)pPhyWP + ContinueWrite);

#if 0
                for(i = 0; i < ContinueWrite; i = i + 192) {
                        if(pucWritePtr[5 + i] == 0x40 && pucWritePtr[6 + i] == 0x65) {
                                now_timestamp = pucWritePtr[0 + i] << 24 | pucWritePtr[1 + i] << 16 | pucWritePtr[2 + i] << 8 | pucWritePtr[3 + i];
                                TP_DBG("%02x%02x%02x%02x\n", pucWritePtr[0 + i], pucWritePtr[1 + i], pucWritePtr[2 + i], pucWritePtr[3 + i]);
                        }
                }
#endif
        }/* MTP for TPP */
        else {
                unsigned char* pucWritePtr;
                pucWritePtr = (unsigned char*)(pBuf_param->BaseVir + ((UADDRESS)pPhyWP - pBuf_param->BasePhy));
                dmac_flush_range(pucWritePtr, pucWritePtr + ContinueWrite);
                outer_flush_range((UADDRESS)pPhyWP, (UADDRESS)(pPhyWP + ContinueWrite));
        }

        /* record ts */
        _tp_dump_lock();
        if (rec_mtp_fd && ContinueWrite) {
                unsigned char* pucWritePtr;
                pucWritePtr = (unsigned char*)(pBuf_param->BaseVir + ((UADDRESS)pPhyWP - pBuf_param->BasePhy));
                rtk_inv_range((void *)pucWritePtr, (void *)((UADDRESS)pucWritePtr + ContinueWrite));
#if IS_ENABLED(CONFIG_RTK_FEATURE_FOR_GKI)
                kernel_write(rec_mtp_fd, pucWritePtr, ContinueWrite, &rec_mtp_fd->f_pos);
#endif
        }
        _tp_dump_unlock();

        if( CHECK_LOG_BLOCK_MASK(TP_LOG_BLOCK_MTP) ) {
                TP_DBG_SIMPLE("MTP%d: ->%02x%02x%02x%02x pPhyWP=%p v=%p p="PT_FMT_UADDR"\n", mtp_id, pucWritePtr1[0], pucWritePtr1[1], pucWritePtr1[2], pucWritePtr1[3], (void*)pPhyWP, pucWritePtr1, ring_buf.RP);
        }
        WRITE_REG32(m2m_wp_reg, new_wp);

        pBuf_param->WP = new_wp;

        pTp_drv->mtp[mtp_id].mtp_write_data_cnt += ContinueWrite;

        _tp_unlock();

        TP_TRACE("MTP%d: pPhyWP = %p, Len=%u\n", mtp_id, pPhyWP, ContinueWrite);

        return TPK_SUCCESS;
}

EXPORT_SYMBOL(RHAL_WriteMTPBuffer);



/*------------------------------------------------------------------
 * Func : RHAL_GetMTPBufferStatus
 *
 * Desc : Get MTP Buffer Status
 *
 * Parm : [I] mtp_id : MTP Index
 *        [O] ppWritePhyPtr : write able physical address
 *        [O] pContinueWriteSize : size of writeable region
 *        [O] pWritableSize : size of total writeable region
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_GetMTPBufferStatus(
        TPK_TP_MTP_T      mtp_id,
        UINT8**           ppWritePhyPtr,
        UINT32*           pContinueWriteSize,
        UINT32*           pWritableSize)
{
        UINT32 block_size;
        TPK_MTP_BUF_T *pBuf_param;

        REGADDR m2m_rp_reg;
#if 0
        UINT8 i;

        for (i = 0; i < MAX_TP_COUNT; i++) {
                if (pTp_drv->tp[i].tp_src == MTP) {
                        break;
                }
        }
#endif
        if (pTp_drv->tp[mtp_id].tp_param.ts_in_tsp_len == byte_192 )
                block_size = 192*2;
        else
                block_size = 188*2;

        switch (mtp_id) {
                case TP_TP0_MTP:
                        m2m_rp_reg = TP_TP0_M2M_RING_RP_reg;
                        break;
                case TP_TP1_MTP:
                        m2m_rp_reg = TP_TP1_M2M_RING_RP_reg;
                        break;
                case TP_TP2_MTP:
                        m2m_rp_reg = TP_TP2_M2M_RING_RP_reg;
                        break;
#ifdef TP_SUPPORT_TP3
                case TP_TP3_MTP:
                        m2m_rp_reg = TP_TP3_M2M_RING_RP_reg;
                        break;
#endif
#ifdef TP_SUPPORT_CI
                case TP_TPP0_MTP:
                        m2m_rp_reg = TP_TP0_P_M2M_RING_RP_reg;
                        break;
                case TP_TPP1_MTP:
                        m2m_rp_reg = TP_TP1_P_M2M_RING_RP_reg;
                        break;
                case TP_TPP2_MTP:
                        m2m_rp_reg = TP_TP2_P_M2M_RING_RP_reg;
                        break;
#endif
                default:
                        TP_WARNING("bad parameter of %s (mtp_id=%d)\n", __func__, mtp_id);
                        return TPK_BAD_PARAMETER;
                        break;
        }

        CHECK_MTP_ID(mtp_id);
        CHECK_TP_STRUCTURE();

        if ((ppWritePhyPtr == NULL) || (pContinueWriteSize == NULL) || (pWritableSize == NULL)) {
                TP_WARNING("MTP%d: ppWritePhyPtr = %p, pContinueWriteSize = %p, pWritableSize = %p\n",
                           mtp_id, ppWritePhyPtr, pContinueWriteSize, pWritableSize);
                return TPK_BAD_PARAMETER;
        }

        _tp_lock();

        pBuf_param = &pTp_drv->mtp[mtp_id].mtp_buf_param;

        CHECK_MTP_BUF_VALID(mtp_id);

        if(pTp_drv->mtp[mtp_id].mtp_stream_status != MTP_STREAM_START) {
                *pContinueWriteSize = 0;
                *pWritableSize      = 0;
                _tp_unlock();
                return TPK_SUCCESS;
        }

        pBuf_param->RP = READ_REG32(m2m_rp_reg);

        *ppWritePhyPtr = (UINT8*)pBuf_param->WP;

        if (pBuf_param->WP >= pBuf_param->RP) {
                *pContinueWriteSize = pBuf_param->LimitPhy - pBuf_param->WP;

                /* make sure that more than 376 bytes left */
                if ((pBuf_param->RP - pBuf_param->BasePhy) < block_size)
                        *pContinueWriteSize = (*pContinueWriteSize > block_size) ? *pContinueWriteSize - block_size : 0;

                if ((pBuf_param->RP - pBuf_param->BasePhy) < block_size) {
                        *pWritableSize = *pContinueWriteSize;
                } else {
                        *pWritableSize = *pContinueWriteSize + (pBuf_param->RP - pBuf_param->BasePhy - block_size);
                }

        } else {
                *pContinueWriteSize = pBuf_param->RP - pBuf_param->WP;

                /* make sure that more than 376 bytes left */
                *pContinueWriteSize = (*pContinueWriteSize > block_size) ? *pContinueWriteSize - block_size : 0;

                *pWritableSize = *pContinueWriteSize;
        }
        _tp_unlock();

        TP_TRACE("MTP%d: pWritePhyPtr = %p, ContinueWriteSize=%u\n",
                 mtp_id, *ppWritePhyPtr, *pContinueWriteSize);

        return TPK_SUCCESS;
}

EXPORT_SYMBOL(RHAL_GetMTPBufferStatus);



/*------------------------------------------------------------------
 * Func : RHAL_MTPFlushBuffer
 *
 * Desc :
 *
 * Parm : [I] id : TP Index
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_MTPFlushBuffer(IN TPK_TP_MTP_T mtp_id, IN MTP_BUFF_FLUSH_MODE_T mode)
{
        TPK_MTP_BUF_T *pBuf_param;
        tp_tp0_m2m_ring_ctrl_RBUS m2m_ctrl;

        REGADDR m2m_ctrl_reg;
        REGADDR m2m_rp_reg;
        REGADDR m2m_wp_reg;
#if 0
        UINT8 i;

        for (i = 0; i < MAX_TP_COUNT; i++) {
                if (pTp_drv->tp[i].tp_src == MTP) {
                        break;
                }
        }
#endif

        switch (mtp_id) {
                case TP_TP0_MTP:
                        m2m_ctrl_reg  = TP_TP0_M2M_RING_CTRL_reg;
                        m2m_rp_reg    = TP_TP0_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP0_M2M_RING_WP_reg;
                        break;
                case TP_TP1_MTP:
                        m2m_ctrl_reg  = TP_TP1_M2M_RING_CTRL_reg;
                        m2m_rp_reg    = TP_TP1_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP1_M2M_RING_WP_reg;
                        break;
                case TP_TP2_MTP:
                        m2m_ctrl_reg  = TP_TP2_M2M_RING_CTRL_reg;
                        m2m_rp_reg    = TP_TP2_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP2_M2M_RING_WP_reg;
                        break;
#ifdef TP_SUPPORT_TP3
                case TP_TP3_MTP:
                        m2m_ctrl_reg  = TP_TP3_M2M_RING_CTRL_reg;
                        m2m_rp_reg    = TP_TP3_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP3_M2M_RING_WP_reg;
                        break;
#endif
#ifdef TP_SUPPORT_CI
                case TP_TPP0_MTP:
                        m2m_ctrl_reg  = TP_TP0_P_M2M_RING_CTRL_reg;
                        m2m_rp_reg    = TP_TP0_P_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP0_P_M2M_RING_WP_reg;
                        break;
                case TP_TPP1_MTP:
                        m2m_ctrl_reg  = TP_TP1_P_M2M_RING_CTRL_reg;
                        m2m_rp_reg    = TP_TP1_P_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP1_P_M2M_RING_WP_reg;
                        break;
                case TP_TPP2_MTP:
                        m2m_ctrl_reg  = TP_TP2_P_M2M_RING_CTRL_reg;
                        m2m_rp_reg    = TP_TP2_P_M2M_RING_RP_reg;
                        m2m_wp_reg    = TP_TP2_P_M2M_RING_WP_reg;
                        break;
#endif
                default:
                        TP_WARNING("bad parameter (mtp_id=%u)\n", mtp_id);
                        return TPK_BAD_PARAMETER;
                        break;
        }

        CHECK_MTP_ID(mtp_id);
        CHECK_TP_STRUCTURE();

        if ((mode != MTP_BUFF_FLUSH_TO_BASE) &&
            (mode != MTP_BUFF_FLUSH_TO_RP) &&
            (mode != MTP_BUFF_FLUSH_TO_WP)) {
                TP_WARNING("MTP%d: mode=%u\n", mtp_id, mode);
                return TPK_BAD_PARAMETER;
        }

        _tp_lock();

        pBuf_param     = &pTp_drv->mtp[mtp_id].mtp_buf_param;
        pBuf_param->RP = READ_REG32(m2m_rp_reg);

        CHECK_MTP_BUF_VALID(mtp_id);

        if (pTp_drv->mtp[mtp_id].mtp_stream_status == MTP_STREAM_START) {
                /* stop MM buffer first */
                m2m_ctrl.regValue   = 0;
                m2m_ctrl.in_go      = 1;
                m2m_ctrl.write_data = 0;
                WRITE_REG32(m2m_ctrl_reg, m2m_ctrl.regValue);
        }

        udelay(100); // must wait over 1us then write rp ,wp because we set in_go in mtp.

        switch(mode) {
                case MTP_BUFF_FLUSH_TO_BASE:
                        pBuf_param->RP = pBuf_param->BasePhy;
                        WRITE_REG32(m2m_rp_reg, pBuf_param->RP);

                        pBuf_param->WP = pBuf_param->BasePhy;
                        WRITE_REG32(m2m_wp_reg, pBuf_param->WP);
                        break;

                case MTP_BUFF_FLUSH_TO_RP:
                        pBuf_param->WP = pBuf_param->RP;
                        WRITE_REG32(m2m_wp_reg, pBuf_param->WP);
                        break;

                case MTP_BUFF_FLUSH_TO_WP:
                        pBuf_param->RP = pBuf_param->WP;
                        WRITE_REG32(m2m_rp_reg, pBuf_param->RP);
                        break;
        }

        if (pTp_drv->mtp[mtp_id].mtp_stream_status == MTP_STREAM_START) {
                /* start MM buffer again */
                m2m_ctrl.regValue   = 0;
                m2m_ctrl.in_go      = 1;
                m2m_ctrl.write_data = 1;
                WRITE_REG32(m2m_ctrl_reg, m2m_ctrl.regValue);
        }

        udelay(10); // must wait over 1us then write rp ,wp because we set in_go in mtp.

        _tp_unlock();

        TP_TRACE("MTP%d: mode = %u\n", mtp_id, mode);
        if( CHECK_LOG_BLOCK_MASK(TP_LOG_BLOCK_MTP) ) {
                TP_DBG("MTP_ID[%u] mode = %u\n", mtp_id, mode);
        }

        return TPK_SUCCESS;
}

EXPORT_SYMBOL(RHAL_MTPFlushBuffer);

/*------------------------------------------------------------------
 * Func : RHAL_MTPFrameControl
 *
 * Desc :
 *
 * Parm : [I] id : TP Index
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_MTPFrameControl(IN TPK_TP_MTP_T mtp_id, IN UINT8 enable)
{
        INT32 ret = TPK_SUCCESS;

        UINT8  bock_size;

        REGADDR m2m_ctrl_reg;
        REGADDR tp_m2m_ring_rp_reg;

        tp_fr_ctrl_RBUS fr_ctrl;
        tp_tp0_m2m_ring_ctrl_RBUS m2m_ctrl;

        TP_BUFFER_PARAM_T mtp_buff;


        TP_TRACE("Enter, mtp_id=%d, enable=%u\n", mtp_id, enable);
        //tp_mask_print_emerg( TP_LOG_BLOCK_ALL, TP_LOG_LEVEL_WARNING, "RHAL_MTPFrameControl Enter, mtp_id=%d, enable=%u\n", mtp_id, enable);

        CHECK_MTP_ID(mtp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        ret = Tp_init((TPK_TP_ENGINE_T)mtp_id);

        if (ret != TPK_SUCCESS) {
                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                _tp_unlock();
                return ret;
        }

#if 0
        if(pTp_drv->mtp[mtp_id].mtp_stream_status == MTP_STREAM_START && enable == 1) {
                _tp_unlock();
                return TPK_SUCCESS;
        }
#endif  //  #if 0

        switch (mtp_id) {
                case TP_TP0_MTP:
                        m2m_ctrl_reg  = TP_TP0_M2M_RING_CTRL_reg;
                        tp_m2m_ring_rp_reg = TP_TP0_M2M_RING_RP_reg;
                        break;
                case TP_TP1_MTP:
                        m2m_ctrl_reg  = TP_TP1_M2M_RING_CTRL_reg;
                        tp_m2m_ring_rp_reg = TP_TP1_M2M_RING_RP_reg;
                        break;
                case TP_TP2_MTP:
                        m2m_ctrl_reg  = TP_TP2_M2M_RING_CTRL_reg;
                        tp_m2m_ring_rp_reg = TP_TP2_M2M_RING_RP_reg;
                        break;
#ifdef TP_SUPPORT_TP3
                case TP_TP3_MTP:
                        m2m_ctrl_reg  = TP_TP3_M2M_RING_CTRL_reg;
                        tp_m2m_ring_rp_reg = TP_TP3_M2M_RING_RP_reg;
                        break;
#endif
                default:
                        TP_WARNING("bad parameter (mtp_id=%u)\n", mtp_id);
                        return TPK_BAD_PARAMETER;
                        break;
        }

        if(pTp_drv->mtp[mtp_id].mtp_stream_status == MTP_STREAM_START && enable == 1) {
                m2m_ctrl.regValue = 0;
                m2m_ctrl.in_go    = 0;
                Tp_Write_Data_Reg_Write(m2m_ctrl_reg, m2m_ctrl.regValue);
        }

        /* frame rate control */
        /* TP will check 4 bytes debug info in the front of TSP when file playback */
        switch(mtp_id) {
                case TP_TP0_MTP:
                        fr_ctrl.regValue = READ_REG32(TP_FR_CTRL_reg);
                        fr_ctrl.tp0_en   = enable ? 1 : 0;
                        fr_ctrl.tp0_rate = 3; /*increase rate control from 1x to 2x since some special stream audio underflow*/
                        WRITE_REG32(TP_FR_CTRL_reg, fr_ctrl.regValue);
                        break;

                case TP_TP1_MTP:
                        fr_ctrl.regValue = READ_REG32(TP_FR_CTRL_reg);
                        fr_ctrl.tp1_en   = enable ? 1 : 0;
                        fr_ctrl.tp1_rate = 3; /*increase rate control from 1x to 2x since some special stream audio underflow*/
                        WRITE_REG32(TP_FR_CTRL_reg, fr_ctrl.regValue);
                        break;

                case TP_TP2_MTP:
                        fr_ctrl.regValue = READ_REG32(TP_FR_CTRL_reg);
                        fr_ctrl.tp2_en   = enable ? 1 : 0;
                        WRITE_REG32(TP_FR_CTRL_reg, fr_ctrl.regValue);
                        break;
#ifdef TP_SUPPORT_TP3
                case TP_TP3_MTP:
                        fr_ctrl.regValue = READ_REG32(TP_FR_CTRL_reg);
                        fr_ctrl.tp3_en   = enable ? 1 : 0;
                        WRITE_REG32(TP_FR_CTRL_reg, fr_ctrl.regValue);
                        break;
#endif
                default:
                        TP_WARNING("unsupported mtp_id=%d\n", mtp_id);
                        ret = TPK_BAD_PARAMETER;
        }

        if(pTp_drv->mtp[mtp_id].mtp_stream_status == MTP_STREAM_START && enable == 1) {
                /* check mtp buffer alignment */
                /* RP will shift from timestamp to sync byte, need to realign, otherwise in_go fail */
                if( CHECK_LOG_BLOCK_MASK(TP_LOG_BLOCK_MTP) ) {
                        TPK_MTP_BUF_T *pBuf_param;
                        unsigned char* pucWritePtr1;
                        TP_BUFFER_PARAM_T ring_buf;
                        pBuf_param = &pTp_drv->mtp[mtp_id].mtp_buf_param;
                        Tp_Get_MTP_Buffer(mtp_id, &ring_buf);
                        pucWritePtr1 = (unsigned char*)(pBuf_param->BaseVir + ((UADDRESS)ring_buf.RP - pBuf_param->BasePhy));
                        TP_DBG_SIMPLE("MTP%d: start v=%p p_rp="PT_FMT_UADDR" p_wp="PT_FMT_UADDR"\n", mtp_id, (void*)pucWritePtr1, ring_buf.RP, ring_buf.WP);
                }
                bock_size = pTp_drv->tp[mtp_id].mass_buffer.bock_size;
                Tp_Get_MTP_Buffer(mtp_id, &mtp_buff);
                if ( (mtp_buff.RP - mtp_buff.Base) % bock_size ) {
                        TP_WARNING("MTP%d: RP is not alignem(RP=%p, Base=%p), realigned to RP=%p, block_size=%d\n",
                                   mtp_id, (void*)mtp_buff.RP, (void*)mtp_buff.Base, (void*)(mtp_buff.RP - ((mtp_buff.RP - mtp_buff.Base) % bock_size)), bock_size);
                        WRITE_REG32(tp_m2m_ring_rp_reg, (mtp_buff.RP - ((mtp_buff.RP - mtp_buff.Base) % bock_size)));
                }

                m2m_ctrl.regValue = 0;
                m2m_ctrl.in_go    = 1;
                Tp_Write_Data_Reg_Write(m2m_ctrl_reg, m2m_ctrl.regValue);
        }
        _tp_unlock();

        TP_TRACE("exit, MTP%d: enable=%u\n", mtp_id, enable);
        //tp_mask_print_emerg( TP_LOG_BLOCK_ALL, TP_LOG_LEVEL_WARNING, "RHAL_MTPFrameControl Exit, mtp_id=%d, enable=%u\n", mtp_id, enable);
        if( CHECK_LOG_BLOCK_MASK(TP_LOG_BLOCK_MTP) ) {
                TP_DBG("exit, MTP%d: enable=%u\n", mtp_id, enable);
        }

        return ret;
}

EXPORT_SYMBOL(RHAL_MTPFrameControl);


/******************************

         PID filter

*******************************/
/*------------------------------------------------------------------
 * Func : RHAL_AddPIDFilter
 *
 * Desc : Add a PID Filter
 *
 * Parm : [I] id  : TP Index
 *        [I] PIDParam : which PID to be removed
 *        [I] file_handle : handle of PID
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_AddPIDFilter(
        TPK_TP_ENGINE_T         tp_id,
        TPK_PID_FILTER_PARAM_T  PIDParm,
        void                   *file_handle)
{
        INT32 ret;

        TP_TRACE("enter, tp_id= %d, PID=0x%x, descram_en=%u, keysel=%u, file_handle =%p\n",
                 tp_id, PIDParm.PID, PIDParm.DescrambleEn, PIDParm.valid, file_handle);

        CHECK_MAX_TP_COUNT(tp_id);

        _tp_lock();

        ret = Tp_init(tp_id);
        if (ret != TPK_SUCCESS) {
                _tp_unlock();
                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                return ret;
        }


        ret = Tp_AddPIDFilter(tp_id, PIDParm, file_handle);

        _tp_unlock();

        TP_TRACE("exit, tp_id= %d, PID=0x%x, descram_en=%u, keysel=%u, file_handle =%p, ret=%d\n",
                 tp_id, PIDParm.PID, PIDParm.DescrambleEn, PIDParm.valid, file_handle, ret);

        return ret;
}

EXPORT_SYMBOL(RHAL_AddPIDFilter);



/*------------------------------------------------------------------
 * Func : RHAL_RemovePIDFilter
 *
 * Desc : Remove a PID Filter
 *
 * Parm : [I] id  : TP Index
 *        [I] PID : which PID to be removed
 *        [I] file_handle : handle of PID
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_RemovePIDFilter(
        TPK_TP_ENGINE_T    tp_id,
        UINT16             PID,
        void*              file_handle)
{
        INT32 ret;

        CHECK_MAX_TP_COUNT(tp_id);

        _tp_lock();

        ret = Tp_init(tp_id);
        if (ret != TPK_SUCCESS) {
                _tp_unlock();

                TP_WARNING("Call Tp_init() fail (%d)\n", ret);

                return ret;
        }

        ret = Tp_RemovePIDFilter(tp_id, PID, file_handle);

        _tp_unlock();

        TP_TRACE("exit, tp_id= %d, PID=0x%x, file_handle=%p\n",
                 tp_id, PID, file_handle);

        return ret;
}

EXPORT_SYMBOL(RHAL_RemovePIDFilter);



/*------------------------------------------------------------------
 * Func : RHAL_RemoveAllPIDFilters
 *
 * Desc : Remove all PID Filters
 *
 * Parm : [I] id : TP Index
 *        [I] file_handle : file handle of PID filter
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_RemoveAllPIDFilters(
        TPK_TP_ENGINE_T     tp_id,
        void*               file_handle
)
{
        UINT16 i;
        UINT16 j;
        UINT16 max_pid_cnt;
        INT32 ret;

        TP_TRACE("enter, tp_id=%d, file_handle =%p\n", tp_id, file_handle);

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        ret = Tp_Get_MaxPID_Cnt(tp_id, &max_pid_cnt);
        if (ret != TPK_SUCCESS) {
                TP_WARNING("call Tp_Get_MaxPID_Cnt return error\n");
                return ret;
        }

        _tp_lock();

        ret = Tp_init(tp_id);
        if (ret != TPK_SUCCESS) {
                _tp_unlock();
                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                return ret;
        }

        /* force to remove all pid filters without any condition */
        if (file_handle == REGARDLESS_OF_FILE_HANDLE) {
                _tp_unlock();

                /* remove all section filter first */
                RHAL_RemoveAllSectionFilters(tp_id, REGARDLESS_OF_FILE_HANDLE);

                _tp_lock();

                for(i = 0; i < max_pid_cnt; i++) {
                        ret = Tp_Remove_PID(tp_id, i);
                        if (ret != TPK_SUCCESS) {
                                TP_WARNING("remove PID register setting error\n");
                                goto PID_ERROR;
                        }

                        pTp_drv->tp_pid[tp_id].pid_file_handle_cnt[i] = 0;
                }

                memset(pTp_drv->tp_pid[tp_id].file_handle, 0, sizeof(void*)*MAX_PID_PER_TP * MAX_FILE_HANDLE_REC);

                /* it will also disable PCR tracking */
                pTp_drv->tp_pid[tp_id].pcr.enable = 0;

                TP_INFO("TP%u force to remove all pid filter recardless of file_handle\n", tp_id);
                goto PID_SUCCESS;
        }

        /* remove pid filter depends on file handle */
        for( i = 0; i < max_pid_cnt; i++) {
                /* check file handle on each pid filter */
                for( j = 0; j < MAX_FILE_HANDLE_REC; j++) {
                        if (pTp_drv->tp_pid[tp_id].file_handle[i][j] == file_handle)
                                break;
                }

                if (j < MAX_FILE_HANDLE_REC) {
                        pTp_drv->tp_pid[tp_id].file_handle[i][j] = NULL;
                        pTp_drv->tp_pid[tp_id].pid_file_handle_cnt[i]--;

                        /* remove PID register */
                        if (pTp_drv->tp_pid[tp_id].pid_file_handle_cnt[i] == 0) {
                                ret = Tp_Remove_PID(tp_id, i);
                                if (ret != TPK_SUCCESS) {
                                        TP_WARNING("remove PID register setting error\n");
                                        goto PID_ERROR;
                                }
                        }
                }
        }

PID_SUCCESS:

        _tp_unlock();

        TP_TRACE("exit, id = %d, file_handle =%p\n", tp_id, file_handle);

        return TPK_SUCCESS;

PID_ERROR:

        _tp_unlock();

        return ret;
}

EXPORT_SYMBOL(RHAL_RemoveAllPIDFilters);



/*------------------------------------------------------------------
 * Func : RHAL_GetPIDFilterStatus
 *
 * Desc : PID Filter status
 *
 * Parm : [I] id : TP Index
 *        [I] PID_Buf_Cnt : PID Buffer Count
 *        [O] pPIDParm  : PID Filter output
 *        [O] pPIDLen   : number of PID
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_GetPIDFilterStatus(
        TPK_TP_ENGINE_T         tp_id,
        UINT16                  PID_Buf_Cnt,
        TPK_PID_FILTER_PARAM_T *pPIDParm,
        UINT16                 *pPIDLen
)
{
        UINT16 i;
        UINT16 max_pid_cnt;
        UINT16 PIDParm_idx = 0;
        INT32 ret;

        if ((PID_Buf_Cnt == 0) ||
            (pPIDParm == NULL) ||
            (pPIDLen == NULL)) {
                TP_WARNING("bad parameter, id=%u, PID_Buf_Cnt=%u, pPIDParm=%p, pPIDLen=%p\n",
                           tp_id, PID_Buf_Cnt, pPIDParm, pPIDLen);

                return TPK_BAD_PARAMETER;
        }

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        ret = Tp_Get_MaxPID_Cnt(tp_id, &max_pid_cnt);
        if (ret != TPK_SUCCESS) {
                TP_WARNING("call Tp_Get_MaxPID_Cnt return error\n");
                return ret;
        }

        _tp_lock();

        ret = Tp_init(tp_id);
        if (ret != TPK_SUCCESS) {
                _tp_unlock();
                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                return ret;
        }

        /* only collect valid PID filter info */
        for(i = 0; i < max_pid_cnt; i++) {
                if ((pTp_drv->tp_pid[tp_id].pid_api_setting[i].valid) &&
                    (PIDParm_idx < PID_Buf_Cnt)) {
                        pPIDParm[PIDParm_idx] = pTp_drv->tp_pid[tp_id].pid_api_setting[i];
                        PIDParm_idx++;
                }
        }

        _tp_unlock();

        *pPIDLen = PIDParm_idx;

        TP_TRACE("return %u valid PID filter\n", PIDParm_idx);

        return TPK_SUCCESS;
}

EXPORT_SYMBOL(RHAL_GetPIDFilterStatus);

/*------------------------------------------------------------------
 * Func : RHAL_EnablePIDFilter
 *
 * Desc : Set TP PID filter On/Off
 *
 * Parm : [I] id : TP ID
 *              [I] on : Enable PID filter or not
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/

INT32 RHAL_EnablePIDFilter(
        IN TPK_TP_ENGINE_T   tp_id,
        IN UINT8             on
)
{
        INT32 ret;
        tp_tp_tf0_cntl_RBUS tp_cntl;

        TP_TRACE("enter, tp_id=%u, on=%u\n", tp_id, on);

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        ret = Tp_init(tp_id);
        if (ret != TPK_SUCCESS) {
                _tp_unlock();
                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                return ret;
        }

        if (on) {
                tp_cntl.pid_en = 1; /* set select bit = 1 */
                Tp_SetTpCtrlReg(tp_id, tp_cntl.regValue, 1);
        } else {
                tp_cntl.pid_en = 1; /* set select bit =1 */
                Tp_SetTpCtrlReg(tp_id, tp_cntl.regValue, 0);

        }
        pTp_drv->tp[tp_id].tp_param.pid_filter_en = on;
        _tp_unlock();


        TP_TRACE("exit, tp_id= %u, on = %u\n", tp_id, on);

        return ret;

}

EXPORT_SYMBOL(RHAL_EnablePIDFilter);


/******************************

         Section filter

*******************************/
/*------------------------------------------------------------------
 * Func : RHAL_AddSectionFilter
 *
 * Desc : Add a section filter
 *
 * Parm : [I] id : TP Index
 *        [I] Param : Section filter parameter
 *        [I] pFileHandle : File handle
 *        [O] ppSecHandle : Section handle output
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_AddSectionFilter(
        TPK_TP_ENGINE_T          tp_id,
        TPK_SEC_FILTER_PARAM_T   Param,
        void                    *pFileHandle,
        void                   **ppSecHandle)
{
        INT32 ret;

        CHECK_MAX_TP_COUNT(tp_id);

        _tp_lock();

        ret = Tp_init(tp_id);
        if (ret != TPK_SUCCESS) {
                _tp_unlock();

                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                return ret;
        }

        /* _Dump_SectionFilter_Status(&Param); */

        ret = Tp_AddSecFilter(tp_id, Param, pFileHandle, ppSecHandle);

#if 0
        /*  hack for test */
        ret = Tp_PCRTrackingEnable(id, 1, 0x7d1, TP_LOCAL_90KHz);
#endif

        _tp_unlock();

        TP_TRACE("exit, TP%u, PID=0x%x, pSecHandle=%p, ret=%d\n",
                 tp_id, Param.PID, *ppSecHandle, ret);

        return ret;
}

EXPORT_SYMBOL(RHAL_AddSectionFilter);



/*------------------------------------------------------------------
 * Func : RHAL_RemoveSectionFilter
 *
 * Desc : Remove Section Filter
 *
 * Parm : [I] id          : TP Index
 *        [I] pSecHandle : Section handle
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_RemoveSectionFilter(
        TPK_TP_ENGINE_T     tp_id,
        void*               pSecHandle
)
{
        INT32 ret;

        CHECK_MAX_TP_COUNT(tp_id);

        _tp_lock();

        ret = Tp_init(tp_id);
        if (ret != TPK_SUCCESS) {
                _tp_unlock();

                TP_WARNING("Call Tp_init() fail (%d)\n", ret);

                return ret;
        }

        ret = Tp_RemoveSectionFilter(tp_id, pSecHandle);

        _tp_unlock();

        TP_TRACE("exit, TP%u, pSecHandle=%p, ret=%d\n", tp_id, pSecHandle, ret);

        return ret;
}

EXPORT_SYMBOL(RHAL_RemoveSectionFilter);



/*------------------------------------------------------------------
 * Func : RHAL_RemoveAllSectionFilters
 *
 * Desc :
 *
 * Parm : [I] id : TP Index
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32  RHAL_RemoveAllSectionFilters(IN TPK_TP_ENGINE_T tp_id, IN void* file_handle)
{
        UINT16 i;
        INT32 ret;

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        ret = Tp_init(tp_id);
        if (ret != TPK_SUCCESS) {
                _tp_unlock();

                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                return ret;
        }

        for( i = 0; i < MAX_SEC_COUNT; i++) {
                if (file_handle == REGARDLESS_OF_FILE_HANDLE) {
                        if ((pTp_drv->sec[i].valid) &&
                            (pTp_drv->sec[i].tp_id == tp_id)) {
                                ret = Tp_RemoveSectionFilter(tp_id, (void*)(&pTp_drv->sec[i]));
                                if (ret != TPK_SUCCESS) {
                                        TP_WARNING("remove section filter failed, id=%u, sec_idx=%u, file_handle=%p\n",
                                                   tp_id, i, file_handle);
                                        goto SEC_ERROR;
                                }
                        }
                } else {
                        if ((pTp_drv->sec[i].valid) &&
                            (pTp_drv->sec[i].tp_id == tp_id) &&
                            (pTp_drv->sec[i].file_handle == file_handle)) {
                                ret = Tp_RemoveSectionFilter(tp_id, (void*)(&pTp_drv->sec[i]));
                                if (ret != TPK_SUCCESS) {
                                        TP_WARNING("remove section filter failed, id=%u, sec_idx=%u, file_handle=%p\n",
                                                   tp_id, i, file_handle);
                                        goto SEC_ERROR;
                                }
                        }
                }
        }

        _tp_unlock();

        TP_TRACE("exit, TP%u, file_handle=%p\n", tp_id, file_handle);

        return TPK_SUCCESS;

SEC_ERROR:
        _tp_unlock();

        return ret;
}

EXPORT_SYMBOL(RHAL_RemoveAllSectionFilters);



/*------------------------------------------------------------------
 * Func : RHAL_GetSectionFilterStatus
 *
 * Desc : Get Section Filter Status
 *
 * Parm : [I] id : TP Index
 *        [I] Sec_Buf_cnt :
 *        [O] pSecStatus
 *        [O] usSecLen
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_GetSectionFilterStatus(
        TPK_TP_ENGINE_T              tp_id,
        UINT16                       Sec_Buf_cnt,
        TPK_SEC_FILTER_STATUS_T     *pSecStatus,
        UINT16                      *usSecLen
)
{
        UINT16 SecParm_idx = 0;
        UINT16 i;
        INT32 ret;

        if ((pSecStatus == NULL) || (Sec_Buf_cnt == 0)) {
                TP_WARNING("bad parameter, Sec_Buf_cnt=%u, pSecStatus=%p,usSecLen=%p\n",
                           Sec_Buf_cnt, pSecStatus, usSecLen);

                return TPK_BAD_PARAMETER;
        }

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        ret = Tp_init(tp_id);
        if (ret != TPK_SUCCESS) {
                _tp_unlock();
                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                return ret;
        }

        /* only collect valid section filter info */
        for( i = 0; i < MAX_SEC_COUNT; i++) {
                if ((pTp_drv->sec[i].valid) &&
                    (pTp_drv->sec[i].tp_id == tp_id) &&
                    (SecParm_idx < Sec_Buf_cnt)) {
                        pSecStatus[SecParm_idx] = pTp_drv->sec[i];
                        SecParm_idx++;
                }
        }

        _tp_unlock();

        *usSecLen = SecParm_idx;

        TP_TRACE("return %u valid Section filter\n", SecParm_idx);

        return TPK_SUCCESS;
}

EXPORT_SYMBOL(RHAL_GetSectionFilterStatus);



/******************************

        PCR tracking

*******************************/

/*------------------------------------------------------------------
 * Func : RHAL_PCRTrackingEnable
 *
 * Desc : Enable PCR tracking
 *
 * Parm : [I] id      : TP Index
 *        [I] on      : 0 : disable, others : enable
 *        [I] PID     : value of PCR PID
 *        [I] clk_src : clock source of STC
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_PCRTrackingEnable(
        TPK_TP_ENGINE_T   tp_id,
        UINT8             on,
        UINT16            PID,
        TPK_PCR_CLK_SRC_T clk_src
)
{
        INT32 ret;

        TP_TRACE("enter, tp_id=%d, On=%u, PID=0x%x, clk_src=%u\n",
                 tp_id, on, PID, clk_src);

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        ret = Tp_init(tp_id);
        if (ret != TPK_SUCCESS) {
                _tp_unlock();
                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                return ret;
        }

        ret = Tp_PCRTrackingEnable(tp_id, on, PID, clk_src);

        _tp_unlock();

        TP_TRACE("exit, id = %d, on=%u, PID=0x%x, clk_src=%u, ret=%d\n",
                 tp_id, on, PID, clk_src, ret);

        return ret;
}

EXPORT_SYMBOL(RHAL_PCRTrackingEnable);



/*------------------------------------------------------------------
 * Func : RHAL_GetPCRTrackingStatus
 *
 * Desc : Get PCR Tracking Status
 *
 * Parm : [I] id       : TP Index
 *        [O] pPCR     : last PCR
 *        [O] pSTC     : last STC
 *        [O] pPCRBase : PCR base
 *        [O] pSTCBase : STC base
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_GetPCRTrackingStatus(
        TPK_TP_ENGINE_T     tp_id,
        UINT64*                 pPCR,
        OUT  UINT64*            pSTC,
        OUT  UINT64*            pPCRBase,
        OUT  UINT64*            pSTCBase
)
{
        INT32 ret;

        if ((pPCR == NULL) ||
            (pSTC == NULL) ||
            (pPCRBase == NULL) ||
            (pSTCBase == NULL)) {
                TP_WARNING("bad parameter, pPCR=%p, pSTC=%p,pPCRBase=%p,pSTCBase=%p\n",
                           pPCR , pSTC, pPCRBase, pSTCBase);
                return TPK_BAD_PARAMETER;
        }

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        ret = Tp_init(tp_id);
        if (ret != TPK_SUCCESS) {
                _tp_unlock();
                TP_WARNING("Call Tp_init() fail (%d)\n", ret);
                return ret;
        }

        ret = Tp_GetPCRTrackingStatus(tp_id, pPCR, pSTC, pPCRBase, pSTCBase);

        _tp_unlock();

        return ret;
}

EXPORT_SYMBOL(RHAL_GetPCRTrackingStatus);



/*------------------------------------------------------------------
 * Func : RHAL_ResetPCRTrackingStatus
 *
 * Desc : Reset PCR Tracking Status
 *
 * Parm : [I] id : TP Index
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_ResetPCRTrackingStatus(TPK_TP_ENGINE_T tp_id)
{
        INT32 ret;

        CHECK_MAX_TP_COUNT(tp_id);

        _tp_lock();

        ret = Tp_ResetPCRStatus(tp_id);

        _tp_unlock();

        return ret;
}

EXPORT_SYMBOL(RHAL_ResetPCRTrackingStatus);


/*------------------------------------------------------------------
 * Func : RHAL_GetPCRAdaptionField
 *
 * Desc : Get PCR Adaption Field
 *
 * Parm : [I] id       : TP Index
 *        [I] PID      : PCR PID
 *        [O] pData    : PCR Adaption Field
 *        [O] Size    : PCR Adaption Field
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_GetPCRAdaptionField(TPK_TP_ENGINE_T tp_id,UINT16 PID,UINT8 *pData,UINT8 *pDataSize)
{
        INT32 ret = TPK_SUCCESS;
        CHECK_MAX_TP_COUNT(tp_id);

        _tp_lock();

        *pDataSize = pTp_drv->tp_pid[tp_id].pcr.adaption_field[0];

        if(pTp_drv->tp_pid[tp_id].pcr.PID == PID && *pDataSize>0 && *pDataSize <= 183) {
            memcpy(pData,&pTp_drv->tp_pid[tp_id].pcr.adaption_field[1],*pDataSize);
        }else{
            *pDataSize = 0;
            ret = TPK_NOT_SET;
        }
        _tp_unlock();

        return ret;
}
EXPORT_SYMBOL(RHAL_GetPCRAdaptionField);


/******************************

         Descrambler APIs

*******************************/
#ifdef CONFIG_RTK_KDRV_SUPPORT_NAGNA
/*------------------------------------------------------------------
 * Func : RHAL_SetNagxx_PID_update_bit
 *
 * Desc : Enable/disable PID update bit function
 *
 * Parm : [I] id   : TP Index
 *        [I] enable : Enable/disable PID update bit function
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_SetNagxx_PID_update_bit(TPK_TP_ENGINE_T tp_id, BOOL enable)
{
        INT32 ret = TPK_SUCCESS;
        CHECK_MAX_TP_COUNT(tp_id);
        TP_TRACE("[%s] enter tp_id=%d enable=%d!", __func__, tp_id, enable);
        ret = Tp_SetNagxx_PID_update_bit(tp_id, enable);
        return ret;
}
EXPORT_SYMBOL(RHAL_SetNagxx_PID_update_bit);

/*------------------------------------------------------------------
 * Func : RHAL_Tp_GetNagxx_param
 *
 * Desc : get nagxx control status
 *
 * Parm : [O] nag_ctl_data   : nagra cotrol data
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_Tp_GetNagxx_param(TPK_NAGXX_Conrol_Data *nag_ctl_data)
{
        INT32 ret = TPK_SUCCESS;

        TP_TRACE("[%s] enter!", __func__);
        ret = Tp_GetNagxx_param(nag_ctl_data);
        return ret;
}
EXPORT_SYMBOL(RHAL_Tp_GetNagxx_param);
#endif
/*------------------------------------------------------------------
 * Func : RHAL_SetDescrambleAlgorithm
 *
 * Desc : Set TP's Descramble Algorithm
 *
 * Parm : [I] id   : TP Index
 *        [I] algo : Descamble algorithm
 *
 *            TPK_DESCRAMBLE_ALGO_DES_ECB_CLEAR : DES_ECB
 *            TPK_DESCRAMBLE_ALGO_TDES_ECB_CLEAR : TDES ECB
 *            ...
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_SetDescrambleAlgorithm(
        TPK_TP_ENGINE_T                tp_id,
        TPK_DESCRAMBLE_ALGORITHM_T     algo
)
{
        tp_tp_tp0_des_cntl_RBUS des_ctrl;
        REGADDR tp_des_cntl_reg;
        static bool cw_init = false;

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        if (algo.round > 256) {
                TP_WARNING("TP%u multi2 round(%u) over 256, not support\n",
                           tp_id, algo.round);
                return TPK_BAD_PARAMETER;
        }

        /* TP3 do not support descramble */
        switch(tp_id) {
                case TP_TP0:
                        tp_des_cntl_reg = TP_TP_TP0_DES_CNTL_reg;
                        break;
                case TP_TP1:
                        tp_des_cntl_reg = TP_TP_TP1_DES_CNTL_reg;
                        break;
                case TP_TP2:
                        tp_des_cntl_reg = TP_TP_TP2_DES_CNTL_reg;
                        break;
                default:
                        TP_WARNING("unsupported descrambler tp_id=%d\n", tp_id);
                        return TPK_BAD_PARAMETER;
        }


        des_ctrl.regValue = 0;
        des_ctrl.map_11   = DES_CNTL_MAP_ODD;
        des_ctrl.map_10   = DES_CNTL_MAP_EVEN;
        des_ctrl.map_01   = DES_CNTL_MAP_ODD;

        switch(algo.algo) {
                case TPK_DESCRAMBLE_ALGO_DES_ECB_CLEAR:
                        TP_INFO("TP%u Descramble Algorithm = DES ECB KEEP CLEAR\n", tp_id);
                        des_ctrl.mode     = DES_CNTL_MODE_DES;
                        des_ctrl.des_mode = DES_CNTL_DESMODE_ECB;
                        des_ctrl.ofb_mode = DES_CNTL_OFB_KEEP_CLEAR;

                        break;
                case TPK_DESCRAMBLE_ALGO_TDES_ECB_CLEAR:
                        TP_INFO("TP%u Descramble Algorithm = TDES ECB KEEP CLEAR\n", tp_id);
                        des_ctrl.mode     = DES_CNTL_MODE_TDES;
                        des_ctrl.des_mode = DES_CNTL_DESMODE_ECB;
                        des_ctrl.ofb_mode = DES_CNTL_OFB_DEFAULT;

                        break;
                case TPK_DESCRAMBLE_ALGO_TDES_CBC_OFB:
                        TP_INFO("TP%u Descramble Algorithm = TDES CBC OFB\n", tp_id);
                        des_ctrl.mode     = DES_CNTL_MODE_TDES;
                        des_ctrl.des_mode = DES_CNTL_DESMODE_CBC;
                        des_ctrl.ofb_mode = DES_CNTL_OFB_OFB;

                        break;
                case TPK_DESCRAMBLE_ALGO_CSA:
                        TP_INFO("TP%u Descramble Algorithm = CSA\n", tp_id);
                        des_ctrl.mode     = DES_CNTL_MODE_CSA;
                        des_ctrl.csa_mode = DES_CNTL_CSA_MODE_1;

                        break;
                case TPK_DESCRAMBLE_ALGO_CSA2:
                        TP_INFO("TP%u Descramble Algorithm = CSA2\n", tp_id);
                        des_ctrl.mode     = DES_CNTL_MODE_CSA;
                        des_ctrl.csa_mode = DES_CNTL_CSA_MODE_2;

                        break;
                case TPK_DESCRAMBLE_ALGO_CSA_RAW:
                        TP_INFO("TP%u Descramble Algorithm = CSA RAW\n", tp_id);
                        des_ctrl.mode     = DES_CNTL_MODE_CSA;
                        des_ctrl.csa_mode = DES_CNTL_CSA_MODE_ORIGINAL;

                        break;
                case TPK_DESCRAMBLE_ALGO_MULTI2:
                        TP_INFO("TP%u Descramble Algorithm = MULTI2 , Round=%u\n", tp_id, algo.round);
                        des_ctrl.ofb_mode	 = DES_CNTL_OFB_KEEP_CLEAR;
                        des_ctrl.mode		 = DES_CNTL_MODE_MULTI2;
                        des_ctrl.des_mode	 = DES_CNTL_DESMODE_CBC;
                        des_ctrl.multi2_mode = DES_CNTL_MULTI2_ALIGN63;
                        des_ctrl.round		 = algo.round - 1;

                        break;
                case TPK_DESCRAMBLE_ALGO_MULTI2_OFB:
                        TP_INFO("TP%u Descramble Algorithm = MULTI2 OFB, Round=%u\n", tp_id, algo.round);
                        des_ctrl.ofb_mode    = DES_CNTL_OFB_DEFAULT;
                        des_ctrl.mode        = DES_CNTL_MODE_MULTI2;
                        des_ctrl.des_mode    = DES_CNTL_DESMODE_CBC;
                        des_ctrl.multi2_mode = DES_CNTL_MULTI2_ALIGN63;
                        des_ctrl.round       = algo.round - 1;

                        break;
                case TPK_DESCRAMBLE_ALGO_MULTI2_CBC:
                        TP_INFO("TP%u Descramble Algorithm = MULTI2 CBC, Round=%u\n", tp_id, algo.round);
                        des_ctrl.ofb_mode	 = DES_CNTL_OFB_DEFAULT;
                        des_ctrl.mode		 = DES_CNTL_MODE_MULTI2;
                        des_ctrl.des_mode	 = DES_CNTL_DESMODE_CBC;
                        des_ctrl.multi2_mode = DES_CNTL_MULTI2_ALIGN63;
                        des_ctrl.round		 = algo.round - 1;

                        break;
                case TPK_DESCRAMBLE_ALGO_AES_128_CBC_CLEAR:
                        TP_INFO("TP%u Descramble Algorithm = AES 128 CBC KEEP CLEAR\n", tp_id);
                        des_ctrl.ofb_mode = DES_CNTL_OFB_KEEP_CLEAR;
                        des_ctrl.mode     = DES_CNTL_MODE_AES128_CBC;
                        des_ctrl.des_mode = DES_CNTL_DESMODE_CBC;

                        break;
                case TPK_DESCRAMBLE_ALGO_AES_128_ECB_CLEAR:
                        TP_INFO("TP%u Descramble Algorithm = AES 128 ECB KEEP CLEAR\n", tp_id);
                        des_ctrl.ofb_mode = DES_CNTL_OFB_KEEP_CLEAR;
                        des_ctrl.mode     = DES_CNTL_MODE_AES128_CBC;
                        des_ctrl.des_mode = DES_CNTL_DESMODE_ECB;

                        break;
                case TPK_DESCRAMBLE_ALGO_AES_RCBC_CS_CLEAR:
                        TP_INFO("TP%u Descramble Algorithm = AES RCBC CS KEEP CLEAR\n", tp_id);
                        des_ctrl.mode = DES_CNTL_MODE_AES128_RCBC_CS;
                        break;

                case TPK_DESCRAMBLE_ALGO_AES_128_CBC_OFB:
                case TPK_DESCRAMBLE_ALGO_AES_128_CBC_OFB_ATIS:
                        TP_INFO("TP%u Descramble Algorithm = AES 128 CBC OFB\n", tp_id);
                        des_ctrl.ofb_mode = DES_CNTL_OFB_OFB;
                        des_ctrl.mode     = DES_CNTL_MODE_AES128_CBC;
                        des_ctrl.des_mode = DES_CNTL_DESMODE_CBC;

                        break;

                case TPK_DESCRAMBLE_ALGO_CAM_ECB_CLEAR:
                        TP_INFO("TP%u Descramble Algorithm = CAM ECB KEEP CLEAR\n", tp_id);
                        des_ctrl.ofb_mode = DES_CNTL_OFB_KEEP_CLEAR;
                        des_ctrl.mode     = DES_CNTL_MODE_CAM;
                        des_ctrl.des_mode = DES_CNTL_DESMODE_ECB;
                        break;

                case TPK_DESCRAMBLE_ALGO_CAM_CBC_CLEAR:
                        TP_INFO("TP%u Descramble Algorithm = CAM ECB KEEP CLEAR\n", tp_id);
                        des_ctrl.ofb_mode = DES_CNTL_OFB_KEEP_CLEAR;
                        des_ctrl.mode     = DES_CNTL_MODE_CAM;
                        des_ctrl.des_mode = DES_CNTL_DESMODE_CBC;
                        break;

                case TPK_DESCRAMBLE_ALGO_CAM_CBC_OFB:
                        TP_INFO("TP%u Descramble Algorithm = CAM ECB KEEP CLEAR\n", tp_id);
                        des_ctrl.ofb_mode = DES_CNTL_OFB_OFB;
                        des_ctrl.mode     = DES_CNTL_MODE_CAM;
                        des_ctrl.des_mode = DES_CNTL_DESMODE_CBC;
                        break;
                default:
                        TP_WARNING("TP%u Set Descrambler Failed - Unsupported Descramble Algorithm %u\n",
                                   tp_id, algo.algo);
                        return TPK_BAD_PARAMETER;
        }

        _tp_lock();

        WRITE_REG32(tp_des_cntl_reg, des_ctrl.regValue);

        pTp_drv->tp[tp_id].desc = algo;

        _tp_unlock();


        /* static bool cw_init = false; */
        if(!cw_init) {
                TP_DBG(" Request CW for TP !!! \n");
                rtk_cw_request_multi_cws("TP", 0, 48);
                cw_init = true;
        } else {
                TP_DBG("  CW has already requested!!! \n");
        }

        TP_TRACE("exit, TP%u, algo=%u, round=%u\n", tp_id, algo.algo, algo.round);

        return TPK_SUCCESS;
}

EXPORT_SYMBOL(RHAL_SetDescrambleAlgorithm);


INT32  RHAL_SetPIDDescrambleAlgorithm(UINT8 key_sel, TPK_DESCRAMBLE_ALGORITHM_T     desc)
{
        pTp_drv->pid_desc[key_sel].desc = desc;
        return TPK_SUCCESS;

}
EXPORT_SYMBOL(RHAL_SetPIDDescrambleAlgorithm);

INT32 RHAL_FreePIDDescrambleAlgorithm(void)
{
        int i;
        for(i = 0; i < MAX_KEY_COUNT; i++) {
                pTp_drv->pid_desc[i].desc.algo = TPK_DESCRAMBLE_ALGO_NONE;
                pTp_drv->pid_desc[i].desc.round = 0;
        }
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_FreePIDDescrambleAlgorithm);


/*------------------------------------------------------------------
 * Func : RHAL_SetCW
 *
 * Desc : Set CW (Control Word)
 *
 * Parm : [I] id      : TP Index
 *        [I] key_set : key set index
 *            [I] key_id  : which key index
 *
 *                TP_SETCW_KEYID_EVEN_KEY
 *                TP_SETCW_KEYID_ODD_KEY
 *                TP_SETCW_KEYID_MULTI2_SYS_KEY : System Key (Multi2)
 *
 *            [I] Key     : key  value
 *            [I] IV      : Initial vector
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_SetCW(
        TPK_TP_ENGINE_T tp_id,
        UINT8           key_set,
        UINT8           key_id,
        UINT8           Key[TPK_MAX_KEY_LENGTH],
        UINT8           IV[TPK_MAX_IV_LENGTH])
{
        UINT8 ofst;
        UINT8 IV_clr[TPK_MAX_IV_LENGTH];
        TPK_DESCRAMBLE_ALGORITHM_T realalgo;

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_MAX_KEY_COUNT(key_set);
        CHECK_TP_STRUCTURE();

        /* init algo */
        realalgo.algo = TPK_DESCRAMBLE_ALGO_NONE;
        realalgo.round = 0;

        if( pTp_drv->tp[tp_id].desc.algo != 0)
                realalgo = pTp_drv->tp[tp_id].desc;
        else if(pTp_drv->pid_desc[key_set].desc.algo != 0)
                realalgo = pTp_drv->pid_desc[key_set].desc;

        if (key_id > TP_SETCW_KEYID_ODD_KEY) {
                if ((realalgo.algo != TPK_DESCRAMBLE_ALGO_MULTI2_OFB) ||
                    (key_id != TP_SETCW_KEYID_MULTI2_SYS_KEY)) {
                        TP_WARNING("TP%u key_id(%u) not support (desc.algo = %u)\n",
                                   tp_id, key_id, realalgo.algo);

                        return TPK_BAD_PARAMETER;
                }
        }

        if ((Key == NULL) || (IV == NULL)) {
                TP_WARNING("key=%p, IV=%p\n", Key, IV);
                return TPK_BAD_PARAMETER;
        }

        _tp_lock();

        switch (realalgo.algo) {
                case TPK_DESCRAMBLE_ALGO_DES_ECB_CLEAR:
                        TP_SetCW(tp_id, key_set, (key_id) ? 2 : 0, BYTES_TO_LONG(&Key[0], 3), BYTES_TO_LONG(&Key[3], 4));

                        break;
                case TPK_DESCRAMBLE_ALGO_TDES_ECB_CLEAR:
                case TPK_DESCRAMBLE_ALGO_TDES_CBC_OFB:
                        ofst = (key_id) ? 4 : 0;
                        TP_SetCW(tp_id, key_set, ofst++, BYTES_TO_LONG(&Key[14], 3), BYTES_TO_LONG(&Key[17], 4));
                        TP_SetCW(tp_id, key_set, ofst++, BYTES_TO_LONG(&Key[ 7], 3), BYTES_TO_LONG(&Key[10], 4));
                        TP_SetCW(tp_id, key_set, ofst++, BYTES_TO_LONG(&Key[ 0], 3), BYTES_TO_LONG(&Key[ 3], 4));

                        if (realalgo.algo == TPK_DESCRAMBLE_ALGO_TDES_CBC_OFB)
                                TP_SetCW(tp_id, key_set, ofst, BYTES_TO_LONG(&IV[0], 4), BYTES_TO_LONG(&IV[4], 4));
                        else
                                TP_SetCW(tp_id, key_set, ofst, 0, 0);

                        break;
                case TPK_DESCRAMBLE_ALGO_CSA:           /* CSA Mode : used by DVB-C CA */
                case TPK_DESCRAMBLE_ALGO_CSA2:          /* CSA Mode : used by DVB-C CA */
                case TPK_DESCRAMBLE_ALGO_CSA_RAW:
                        TP_SetCW(tp_id, key_set, (key_id) ? 1 : 0, BYTES_TO_LONG(&Key[0], 4), BYTES_TO_LONG(&Key[4], 4));

                        break;
                case TPK_DESCRAMBLE_ALGO_MULTI2:
                case TPK_DESCRAMBLE_ALGO_MULTI2_CBC:
                case TPK_DESCRAMBLE_ALGO_MULTI2_OFB:
                        if (key_id == TP_SETCW_KEYID_MULTI2_SYS_KEY) {
                                /* system key (256 bits) */
                                TP_SetCW(tp_id, key_set, 4, BYTES_TO_LONG(&Key[0], 4),  BYTES_TO_LONG(&Key[ 4], 4));
                                TP_SetCW(tp_id, key_set, 5, BYTES_TO_LONG(&Key[8], 4),  BYTES_TO_LONG(&Key[12], 4));
                                TP_SetCW(tp_id, key_set, 6, BYTES_TO_LONG(&Key[16], 4), BYTES_TO_LONG(&Key[20], 4));
                                TP_SetCW(tp_id, key_set, 7, BYTES_TO_LONG(&Key[24], 4), BYTES_TO_LONG(&Key[28], 4));
                        } else {
                                /* even/odd key (64 bits) */
                                ofst = (key_id) ? 2 : 0;
                                TP_SetCW(tp_id, key_set, ofst++, BYTES_TO_LONG(&Key[0], 4), BYTES_TO_LONG(&Key[ 4], 4));
                                TP_SetCW(tp_id, key_set, ofst,   BYTES_TO_LONG(&IV[0], 4), BYTES_TO_LONG(&IV[4], 4));
                        }

                        break;
                case TPK_DESCRAMBLE_ALGO_AES_128_CBC_OFB_ATIS:
                        ofst = (key_id) ? 4 : 0;

                        memset(IV_clr, 0, TPK_MAX_IV_LENGTH);
                        TP_SetCW(tp_id, key_set, ofst++, BYTES_TO_LONG(&Key[0], 4), BYTES_TO_LONG(&Key[ 4], 4));
                        TP_SetCW(tp_id, key_set, ofst++, BYTES_TO_LONG(&Key[8], 4), BYTES_TO_LONG(&Key[12], 4));
                        TP_SetCW(tp_id, key_set, ofst++, BYTES_TO_LONG(&IV_clr[0], 4), BYTES_TO_LONG(&IV_clr[ 4], 4));
                        TP_SetCW(tp_id, key_set, ofst,   BYTES_TO_LONG(&IV_clr[8], 4), BYTES_TO_LONG(&IV_clr[12], 4));

                        break;
                case TPK_DESCRAMBLE_ALGO_AES_128_CBC_CLEAR:
                case TPK_DESCRAMBLE_ALGO_AES_128_CBC_OFB:
                case TPK_DESCRAMBLE_ALGO_AES_RCBC_CS_CLEAR:
                case TPK_DESCRAMBLE_ALGO_AES_128_ECB_CLEAR:
                case TPK_DESCRAMBLE_ALGO_CAM_ECB_CLEAR:
                case TPK_DESCRAMBLE_ALGO_CAM_CBC_CLEAR:
                case TPK_DESCRAMBLE_ALGO_CAM_CBC_OFB:
                        ofst = (key_id) ? 4 : 0;
                        TP_SetCW(tp_id, key_set, ofst++, BYTES_TO_LONG(&Key[0], 4), BYTES_TO_LONG(&Key[ 4], 4));
                        TP_SetCW(tp_id, key_set, ofst++, BYTES_TO_LONG(&Key[8], 4), BYTES_TO_LONG(&Key[12], 4));
                        TP_SetCW(tp_id, key_set, ofst++, BYTES_TO_LONG(&IV[0], 4), BYTES_TO_LONG(&IV[ 4], 4));
                        TP_SetCW(tp_id, key_set, ofst,   BYTES_TO_LONG(&IV[8], 4), BYTES_TO_LONG(&IV[12], 4));

                        break;
                default:
                        _tp_unlock();

                        TP_WARNING("TP%u descrambler algorithm (%u) not support\n",
                                   tp_id, realalgo.algo);
                        return TPK_NOT_SET;
        }

        _tp_unlock();

        TP_TRACE("exit, TP%u, key_set=%u, key_id=%u\n", tp_id, key_set, key_id);

        return TPK_SUCCESS;
}

EXPORT_SYMBOL(RHAL_SetCW);



/*------------------------------------------------------------------
 * Func : RHAL_DescrambleControl
 *
 * Desc : Set Descramble Mode
 *
 * Parm : [I] id : TP Index
 *        [I] mode : Descramble mode
 *
 *            TPK_NO_DESCRAMBLE
 *            TPK_TSP_LEVEL_DESCRAMBLE
 *            TPK_PES_LEVEL_DESCRAMBLE
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_DescrambleControl(
        IN TPK_TP_ENGINE_T       tp_id,
        IN TPK_DESCRAMBLE_MODE_T mode
)
{
        tp_tp_tf0_cntl_RBUS tp_cntl;
        REGADDR tp_cntl_reg;
#ifdef SUPPORT_TP_CP_SAVING_MODE
        UINT8 i;
        UINT8 cp_engine_status = 0;
#endif

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        /* TP3 do not support descramble */
        switch(tp_id) {
                case TP_TP0:
                        tp_cntl_reg = TP_TP_TF0_CNTL_reg;
                        break;

                case TP_TP1:
                        tp_cntl_reg = TP_TP_TF1_CNTL_reg;
                        break;

                case TP_TP2:
                        tp_cntl_reg = TP_TP_TF2_CNTL_reg;
                        break;

                default:
                        TP_WARNING("unsupported descrambler id=%d\n", tp_id);
                        return TPK_BAD_PARAMETER;
        }

        _tp_lock();

        tp_cntl.regValue   = 0;
        tp_cntl.regValue   = READ_REG32(tp_cntl_reg);
        tp_cntl.write_data = 0;

        switch(mode) {
                case TPK_TSP_LEVEL_DESCRAMBLE:
                case TPK_PES_LEVEL_DESCRAMBLE:
                        /* if (pTp_drv->tp[tp_id].desc.algo == TPK_DESCRAMBLE_ALGO_NONE) {
                                 _tp_unlock();
                                 TP_WARNING("TP%u Start descrambler failed - no descramble algorithm specified\n", tp_id);
                                 return TPK_NOT_INIT;
                         }
                         */

                        if (pTp_drv->tp[tp_id].tp_param.pid_filter_en == 0) {
                                TP_WARNING("TP%u descrambler can not work without PID filter, please enable PID filter\n", tp_id);
                        }

                        if (mode == TPK_TSP_LEVEL_DESCRAMBLE) {
                                tp_cntl.pes_en = 0;
                                tp_cntl.psc_en = 0;
                                tp_cntl.de_en  = 1;
                                tp_cntl.tsc_en = 1;
                        } else {
                                tp_cntl.pes_en = 1;
                                tp_cntl.psc_en = 1;
                                tp_cntl.de_en  = 0;
                                tp_cntl.tsc_en = 0;
                        }
                        break;

                case TPK_NO_DESCRAMBLE:
                        tp_cntl.pes_en = 0;
                        tp_cntl.psc_en = 0;
                        tp_cntl.de_en  = 0;
                        tp_cntl.tsc_en = 0;

                        break;

                default:
                        _tp_unlock();
                        TP_WARNING("TP%u unsupport descramble mode (%u)\n", tp_id,
                                   mode);
                        return TPK_BAD_PARAMETER;
        }

        if(tp_cntl.de_en == 0 ) {
                Tp_Update_PIDFilter_predes( tp_id,      0);
        }

        Tp_Write_Data_Reg_Write(tp_cntl_reg, tp_cntl.regValue);

        if(tp_cntl.de_en == 1 ) {
                Tp_Update_PIDFilter_predes( tp_id,      1);
        }

        pTp_drv->tp[tp_id].desc_mode = mode;

#ifdef SUPPORT_TP_CP_SAVING_MODE
        //just for mac8p and after
        for(i=0; i<3; i++){
            switch (i)
            {
                case TP_TP0:
                    tp_cntl_reg = TP_TP_TF0_CNTL_reg;
                    break;
                case TP_TP1:
                    tp_cntl_reg = TP_TP_TF1_CNTL_reg;
                    break;
                case TP_TP2:
                    tp_cntl_reg = TP_TP_TF2_CNTL_reg;
                    break;
                default:
                    TP_WARNING("unsupported descrambler id=%d\n", tp_id);
                    break;
            }
            tp_cntl.regValue = READ_REG32(tp_cntl_reg);
            if(tp_cntl.de_en || tp_cntl.tsc_en || tp_cntl.pes_en || tp_cntl.psc_en){
                cp_engine_status=1;
            }
        }
        if(cp_engine_status){
            mcp_enable_cp_power_saving(false); //exit cp power saving mode, open CP engine
            TP_WARNING("tp%d open CP engine\n", tp_id);
        }else{
            mcp_enable_cp_power_saving(true); //enter cp power saving mode, close CP engine
            TP_WARNING("tp%d close CP engine\n", tp_id);
        }
#endif
        _tp_unlock();

        TP_TRACE("exit, TP%u, mode = %u\n", tp_id, mode);
        return TPK_SUCCESS;
}

EXPORT_SYMBOL(RHAL_DescrambleControl);



/***********************************

         debug message

************************************/

/*------------------------------------------------------------------
 * Func : RHAL_TrackErrorPacket
 *
 * Desc : Get TP's packet status
 *
 * Parm : [I] id         : TP index
 *        [O] p_err_pkt  : number of error packet
 *        [O] p_rcv_pkt  : number of received packet
 *        [O] p_drop_pkt : number of dropped packet
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TrackErrorPacket(
        TPK_TP_ENGINE_T    tp_id,
        UINT32            *p_err_pkt,
        UINT32            *p_rcv_pkt,
        UINT32            *p_drop_pkt
)
{
        INT32 ret;
        TPK_TP_PKT_CNT_T tp_pkt_status;

        CHECK_MAX_TP_COUNT(tp_id);

        _tp_lock();

        ret = Tp_TrackErrorPacket(tp_id, &tp_pkt_status.pkt, &tp_pkt_status.droppedPkt, &tp_pkt_status.errorPkt);

        _tp_unlock();

        /* only copy sizeof(UINT32) to demux, because the difference type of packet, demux is UINT32, TP is UINT64 */
        memcpy(p_rcv_pkt, &tp_pkt_status.pkt, sizeof(UINT32));
        memcpy(p_drop_pkt, &tp_pkt_status.droppedPkt, sizeof(UINT32));
        memcpy(p_err_pkt, &tp_pkt_status.errorPkt, sizeof(UINT32));

        return ret;
}

EXPORT_SYMBOL(RHAL_TrackErrorPacket);



/***********************************

 hardware info and status for AP

************************************/

/*------------------------------------------------------------------
 * Func : RHAL_GetHardwareInfo
 *
 * Desc : Get hardware info
 *
 * Parm : [O] pTpHardwareInfo : output buffer of TP hardware status
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_GetHardwareInfo(TPK_HARDWARE_INFO_T *pTpHardwareInfo)
{
        UINT32 tp_hardware_info_max_size = MAX_TP_MODULE_SUPPORT;
        TPK_TP_ENGINE_T i;

        TP_TRACE("Enter\n");

        if (pTpHardwareInfo == NULL) {
                TP_WARNING("bad parameter, pTpHardwareInfo=%p\n", pTpHardwareInfo);
                return TPK_BAD_PARAMETER;
        }

        if (tp_hardware_info_max_size < MAX_TP_COUNT) {
                TP_WARNING("tp_hardware_info_max_size (%u) is smaller than MAX_TP_COUNT (%u)\n",
                           tp_hardware_info_max_size, MAX_TP_COUNT);

                return TPK_RESOURCE_CONFLICT;
        }

        pTpHardwareInfo->chip_version    = CHIP_VERSION;
        pTpHardwareInfo->tp_num          = MAX_TP_COUNT;
        pTpHardwareInfo->tpout_num       = MAX_TPOUT_COUNT;
        pTpHardwareInfo->mtp_num         = MAX_MTP_COUNT;
        pTpHardwareInfo->descrambler_num = MAX_KEY_COUNT;

        for( i = 0; i < MAX_TP_COUNT; i++) {
                if (i == TP_TP0) {
                        pTpHardwareInfo->tp_hareware_info[i].pid_filter_num = TP0_PID_FILTER_COUNT;
                        pTpHardwareInfo->tp_hareware_info[i].sec_filter_num = TP0_SEC_FILTER_COUNT;
                } else if (i == TP_TP1) {
                        pTpHardwareInfo->tp_hareware_info[i].pid_filter_num = TP1_PID_FILTER_COUNT;
                        pTpHardwareInfo->tp_hareware_info[i].sec_filter_num = TP1_SEC_FILTER_COUNT;
                } else if (i == TP_TP2) {
                        pTpHardwareInfo->tp_hareware_info[i].pid_filter_num = TP2_PID_FILTER_COUNT;
                        pTpHardwareInfo->tp_hareware_info[i].sec_filter_num = TP2_SEC_FILTER_COUNT;
                } else if (i == TP_TP3) {
                        pTpHardwareInfo->tp_hareware_info[i].pid_filter_num = TP3_PID_FILTER_COUNT;
                        pTpHardwareInfo->tp_hareware_info[i].sec_filter_num = TP3_SEC_FILTER_COUNT;
                }
        }

        TP_TRACE("exit, chip_version=%u, tp_num=%u, tpout_num=%u, mtp_num=%u\n",
                 pTpHardwareInfo->chip_version,
                 pTpHardwareInfo->tp_num,
                 pTpHardwareInfo->tpout_num,
                 pTpHardwareInfo->mtp_num);

        return TPK_SUCCESS;
}

EXPORT_SYMBOL(RHAL_GetHardwareInfo);


/*------------------------------------------------------------------
 * Func : RHAL_GetTpStatus
 *
 * Desc :
 *
 * Parm : [I] id : TP Index
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_GetTpStatus(IN TPK_TP_ENGINE_T tp_id, OUT TPK_TP_STATUS_T *pTpStatus)
{

        if (jiffies >= tp_status_timeout) {
                TP_TRACE("Enter, id = %u\n", tp_id);
        }

        if (pTpStatus == NULL) {
                TP_WARNING("bad parameter, pTpStatus=%p\n", pTpStatus);
                return TPK_BAD_PARAMETER;
        }

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        memcpy(pTpStatus, &pTp_drv->tp[tp_id], sizeof(TPK_TP_STATUS_T));

        _tp_unlock();

        if (jiffies >= tp_status_timeout) {
                TP_TRACE("exit, tp_id=%u\n", tp_id);
                tp_status_timeout = jiffies + HZ;
        }

        return TPK_SUCCESS;
}

EXPORT_SYMBOL(RHAL_GetTpStatus);


/*------------------------------------------------------------------
 * Func : RHAL_GetMtpStatus
 *
 * Desc : Get Status of MTP engine
 *
 * Parm : [I] mtp_id     : Index of MTP engine
 *        [O] pMtpStatus : status of MTP
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_GetMtpStatus(
        UINT8                     mtp_id,
        TPK_MTP_STATUS_T         *pMtpStatus
)
{
        TP_TRACE("Enter, mtp_id = %d\n", mtp_id);

        if (pMtpStatus == NULL) {
                TP_WARNING("bad parameter pMtpStatus=%p\n", pMtpStatus);

                return TPK_BAD_PARAMETER;
        }

        CHECK_MTP_ID(mtp_id);
        CHECK_TP_STRUCTURE();

        _tp_lock();

        memcpy(pMtpStatus, &pTp_drv->mtp[mtp_id], sizeof(TPK_MTP_STATUS_T));

        _tp_unlock();

        TP_TRACE("exit, mtp_id = %u\n", mtp_id);
        return TPK_SUCCESS;
}

EXPORT_SYMBOL(RHAL_GetMtpStatus);


TPK_TP_SOURCE_T TpSrc_MapFromPCBToTP(TP_SRC_TYPE_T tpSrcPcb)
{
        TPK_TP_SOURCE_T tpSrc;

        switch(tpSrcPcb) {
                case TP_SRC_0:
                        tpSrc = TS_SRC_0;
                        break;
                case TP_SRC_1:
                        tpSrc = TS_SRC_1;
                        break;
                case TP_SRC_2:
                        tpSrc = TS_SRC_2;
                        break;
                case TP_SRC_3:
                        tpSrc = TS_SRC_3;
                        break;
                case TP_SRC_INTERNAL:
                        tpSrc = Internal_Demod;
                        break;
                case TP_SRC_UNUSED:
                        tpSrc = TS_UNMAPPING;
                        break;
                default:
                        tpSrc = TS_UNMAPPING;
                        break;
        }
        return tpSrc;
}
EXPORT_SYMBOL(TpSrc_MapFromPCBToTP);

INT32 RHAL_GetTPSource(TPK_INPUT_PORT_T portType, TPK_CAS_TYPE_T caseType)
{

        unsigned long long info = 0;
        TPK_TP_SOURCE_T tp_src;
        int tmp_src;

        switch(portType) {
                case TPK_PORT_IN_DEMOD0:
                case TPK_PORT_IN_DEMOD1:
                        tp_src = Internal_Demod;
                        break;
                case TPK_PORT_EXT_INPUT0: {
                        if (pcb_mgr_get_enum_info_byname("TP_SRC", &info) == 0) {
                                if(TPK_WITH_CAS == caseType){
                                        tmp_src = TP_SRC_EXTERNAL_CAM0_GET(info);
                                } else {
                                        tmp_src = TP_SRC_EXTERNAL_TUNER0_GET(info);
                                }

                                tp_src = TpSrc_MapFromPCBToTP((TP_SRC_TYPE_T)tmp_src);
                        } else {
                                tp_src = TS_SRC_0;
                        }
                        break;
                }
                case TPK_PORT_EXT_INPUT1: {
                        if (pcb_mgr_get_enum_info_byname("TP_SRC", &info) == 0) {
                                if(TPK_WITH_CAS == caseType){
                                        tmp_src = TP_SRC_EXTERNAL_CAM1_GET(info);
                                } else {
                                        tmp_src = TP_SRC_EXTERNAL_TUNER1_GET(info);
                                }

                                tp_src = TpSrc_MapFromPCBToTP((TP_SRC_TYPE_T)tmp_src);
                        } else {
                                tp_src = TS_SRC_2;
                        }
                        break;
                }
                case TPK_PORT_FROM_MEM:
                case TPK_PORT_IP0:
                case TPK_PORT_IP1:
                case TPK_PORT_IP2:
                case TPK_PORT_IP3:
                case TPK_PORT_CI_DEMUX_OUT_CI_CH0:
                case TPK_PORT_CI_DEMUX_OUT_CI_CH1:
                case TPK_PORT_CI_DEMUX_OUT_CI_CH2:
                case TPK_PORT_CI_DEMUX_OUT_CI_CH3: {
                        tp_src = MTP;
                        break;
                }
                default: {
                        tp_src = TS_UNMAPPING;
                        break;
                }
        }

        return tp_src;
}
EXPORT_SYMBOL(RHAL_GetTPSource);

INT32 RHAL_TPDbgRecTSData(IN TPK_TP_ENGINE_T tp_id, IN UINT8 enable, IN UINT8* path)
{
#if IS_ENABLED(CONFIG_RTK_FEATURE_FOR_GKI)
        struct  file *tmp_fd = NULL;
        UINT8 n;

        if ( ((enable) && (path == NULL)) ) {
                TP_WARNING("bad parameter, path=%s\n", path);
                return TPK_BAD_PARAMETER;
        }

        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        if(enable) {
                if(rec_fd[tp_id]) {
                        for ( n = 0; n < MAX_TP_COUNT; n++) {
                                if (0 == strncmp(path, tp_file_path[n], strlen(tp_file_path[n])))
                                        break;
                        }

                        if(n < MAX_TP_COUNT) {
                                TP_WARNING("File path is same, %s\n", path);
                                return TPK_SUCCESS;
                        }

                        TP_INFO("TS Record Stopped\n");
                        filp_close(rec_fd[tp_id], NULL);
                        rec_fd[tp_id] = NULL;
                        memset(tp_file_path[tp_id], 0, sizeof(tp_file_path[tp_id]));
                }

                tmp_fd = filp_open(path, O_RDWR | O_CREAT | O_TRUNC, 0);
                if(IS_ERR(tmp_fd)) {
                        TP_WARNING("Start Rec TS failed, open %s failed\n", path);
                        return TPK_BAD_PARAMETER;
                } else {

                        for ( n = 0; n < MAX_TP_COUNT; n++) {
                                if (0 == strncmp(path, tp_file_path[n], strlen(path)))
                                        break;
                        }

                        if(n < MAX_TP_COUNT) {
                                TP_WARNING("File path is same, %s\n", path);
                                return TPK_BAD_PARAMETER;
                        } else {
                                TP_INFO("Start Rec TS to - %s,rec_fd=%p\n", path, tmp_fd);
                                strncpy(tp_file_path[tp_id], path, (sizeof(tp_file_path) / MAX_TP_COUNT) - 1);
                                rec_fd[tp_id] = tmp_fd;
                        }
                }
        } else {
                if(rec_fd[tp_id]) {
                        TP_INFO("TS Record Stopped\n");
                        filp_close(rec_fd[tp_id], NULL);
                        rec_fd[tp_id] = NULL;
                        memset(tp_file_path[tp_id], 0, sizeof(tp_file_path[tp_id]));
                }
        }
#endif
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_TPDbgRecTSData);


void RHAL_SetTPRefClockMode(REFERENCE_CLOCK_CALLBACK callback)
{
	pTp_drv->set_get_referenc_clock  = callback;
	return;
}
EXPORT_SYMBOL(RHAL_SetTPRefClockMode);


/*------------------------------------------------------------------
 * Func : RHAL_SetTPHardwareReset
 *
 * Desc : Set TP Hardware Reset
 *
 * Parm : [I] restore_value : restore value or not
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_SetTPHardwareReset(
        UINT8              restore_value
)
{
        tp_tp_tf0_frmcfg_RBUS tp_frm;
        long clk_src_ch_a;
        long clk_src_ch_b;

        CHECK_TP_STRUCTURE();

        _tp_lock();

        TP_DBG_SIMPLE( "TPHardwareReset\n");
        if(pTp_drv->set_get_referenc_clock!=NULL){
                TP_DBG_SIMPLE( "TPHardwareReset: set reference clock:[0:%d][1:%d]\n", MISC_90KHz, MISC_90KHz);
                /*0:get 1:set*/
                clk_src_ch_a = pTp_drv->set_get_referenc_clock(0, 0, 0);
                clk_src_ch_b = pTp_drv->set_get_referenc_clock(1, 0, 0);
                pTp_drv->set_get_referenc_clock(0, MISC_90KHz, 1);
                pTp_drv->set_get_referenc_clock(1, MISC_90KHz, 1);
        }
        msleep(10);

        pTp_drv->hw_reset = 1;

        tp_frm.regValue    = READ_REG32(TP_TP_TF0_FRMCFG_reg);
        tp_frm.frm_en      = 0;
        WRITE_REG32(TP_TP_TF0_FRMCFG_reg, tp_frm.regValue);

        save_tp_reigster_value_ex();

        CRT_CLK_OnOff(TP, CLK_OFF, NULL);
#ifdef CONFIG_RTK_TP_WITH_MIO
        CRT_CLK_OnOff(TP_MIO, CLK_OFF, NULL);
#endif
        udelay(10);

        CRT_CLK_OnOff(TP, CLK_ON, NULL);
#ifdef CONFIG_RTK_TP_WITH_MIO
        CRT_CLK_OnOff(TP_MIO, CLK_ON, NULL);
#endif
        restore_tp_reigster_value_ex();

        tp_frm.regValue    = READ_REG32(TP_TP_TF0_FRMCFG_reg);
        tp_frm.frm_en      = 1;
        WRITE_REG32(TP_TP_TF0_FRMCFG_reg, tp_frm.regValue);

        pTp_drv->hw_reset = 0;

        if(pTp_drv->set_get_referenc_clock!=NULL){
                TP_DBG_SIMPLE( "TPHardwareReset: set reference clock:[0:%ld][1:%ld]\n", clk_src_ch_a, clk_src_ch_b);
                pTp_drv->set_get_referenc_clock(0, clk_src_ch_a, 1);
                pTp_drv->set_get_referenc_clock(1, clk_src_ch_b, 1);
        }

        _tp_unlock();

        TP_TRACE("exit, restore_value %u\n", restore_value);
        return TPK_SUCCESS;
}

EXPORT_SYMBOL(RHAL_SetTPHardwareReset);


/*------------------------------------------------------------------
 * Func : RHAL_SetTPPatternSearchInfoBuffer
 *
 * Desc : Set Start Code Search Buffer
 *
 * Parm : [I] tp_id : TP Index
 *        [I] pPhyAddr : physical address
 *        [I] pVirAddr : virtual address
 *        [I] size     : size of ring buffer
 *        [I] pSharedRP : address of RP shared with the caller
 *        [I] pSharedWP : address of WP shared with the caller
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_SetTPPatternSearchInfoBuffer(UINT8 tp_id, UINT8* pPhyAddr, UINT8* pVirAddr, UINT32 size, UINT8* pSharedRP, UINT8* pSharedWP)
{
        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        if( Tp_is_SupportPatternSearch(tp_id) == TPK_NOT_SUPPORT )
                return TPK_BAD_PARAMETER;

        _tp_lock();

        Tp_SetPatternSearchInfoBuffer(tp_id, pPhyAddr, pVirAddr, size, pSharedRP, pSharedWP);

        _tp_unlock();

        TP_TRACE("exit, tp_id=%d pPhyAddr=0x%p pVirAddr=0x%p size=%d pSharedRP=0x%p pSharedWP=0x%p\n", tp_id, (void*)pPhyAddr, (void*)pVirAddr, size, (void*)pSharedRP, (void*)pSharedWP);
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_SetTPPatternSearchInfoBuffer);


/*------------------------------------------------------------------
 * Func : RHAL_SetTPPatternSearchPID
 *
 * Desc : Set Start Code PID Number
 *
 * Parm : [I] tp_id : TP Index
 *        [I] PID     : value of Start Code Search PID
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_SetTPPatternSearchPID(UINT8 tp_id, UINT16 PID)
{
        CHECK_MAX_TP_COUNT(tp_id);
        CHECK_TP_STRUCTURE();

        if( Tp_is_SupportPatternSearch(tp_id) == TPK_NOT_SUPPORT )
                return TPK_BAD_PARAMETER;

        _tp_lock();

        Tp_SetPatternSearchPID(tp_id, PID);

        _tp_unlock();

        TP_TRACE("exit, tp_id=%d PID=0x%x\n", tp_id, PID);
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_SetTPPatternSearchPID);

/*------------------------------------------------------------------
 * Func : RHAL_DumpTPRegister
 *
 * Desc : Dump the TP Register to file
 *
 * Parm : None
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_DumpTPRegister(char *str)
{
        _tp_lock();

        Tp_DumpTPRegister(str);

        _tp_unlock();
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_DumpTPRegister);








UINT8 GetTSPLen(TPK_TSP_LEN_T tsp_len)
{
        switch(tsp_len) {
                case byte_188:
                        return 188;
                case byte_192:
                        return 192;
                case byte_204:
                        return 204;
                default:
                        TP_WARNING("Unknow tsp_len\n");
                        return 188;
        }
}


/*------------------------------------------------------------------
 * Func : RHAL_DisplayTPFramerInformation
 *
 * Desc :
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_DisplayTPFramerInformation(TPK_TP_ENGINE_T tp_id)
{
        _tp_lock();
        TP_DisplayTPFramerInformation(tp_id);
        _tp_unlock();
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_DisplayTPFramerInformation);

/*------------------------------------------------------------------
 * Func : RHAL_TPRawModeEnable
 *
 * Desc :
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_TPRawModeEnable(TPK_TP_ENGINE_T tp_id, TPK_TP_RAW_CTRL_T enable)
{
        _tp_lock();
        Tp_ENG_RawModeEnable(tp_id, enable);
        _tp_unlock();
        TP_WARNING("RHAL_TPRawModeEnable tp_id=%d enable = %d \n", tp_id, enable);
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_TPRawModeEnable);

#if defined(CONFIG_RTK_KDRV_ATSC3_DMX)
/*------------------------------------------------------------------
 * Func : RHAL_ATSC30_Init
 *
 * Desc :
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_ATSC30_Init(TPK_TP_ENGINE_T tp_id)
{
        _tp_lock();
        pTp_drv->tp_atsc[tp_id].clock_recovery = 1;
        pTp_drv->tp_atsc[tp_id].wall_clock     = 0;
        memset(pTp_drv->tp_atsc[tp_id].pTimeInfo, 0x00, 9);
        _tp_unlock();
        TP_INFO("RHAL_ATSC30_Init tp_id=%d \n", tp_id);
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_ATSC30_Init);

/*------------------------------------------------------------------
 * Func : RHAL_ATSC30_UnInit
 *
 * Desc :
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_ATSC30_UnInit(TPK_TP_ENGINE_T tp_id)
{
        _tp_lock();
        pTp_drv->tp_atsc[tp_id].clock_recovery = 1;
        pTp_drv->tp_atsc[tp_id].wall_clock     = 0;
        memset(pTp_drv->tp_atsc[tp_id].pTimeInfo, 0x00, 9);
        _tp_unlock();
        TP_INFO("RHAL_ATSC30_UnInit tp_id=%d\n", tp_id);
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_ATSC30_UnInit);

/*------------------------------------------------------------------
 * Func : RHAL_ATSC30_Open
 *
 * Desc :
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_ATSC30_Open(TPK_TP_ENGINE_T tp_id)
{
        unsigned int   BbfBufferZoneSize         = 0;
        unsigned int   BbfBufferZoneStartPhyAddr = 0;
        unsigned char *BbfBufferZoneStartVirAddr = 0;
        int i;

        _tp_lock();

        if(pTp_drv->tp_atsc[tp_id].open_count != 0) {
                TP_ERROR("[%s %d] Already Start , just return \n", __func__ , __LINE__ );
                _tp_unlock();
                return TPK_FAIL;
        }

        pTp_drv->tp_atsc[tp_id].clock_recovery = 1;
        pTp_drv->tp_atsc[tp_id].wall_clock     = 0;
        memset(pTp_drv->tp_atsc[tp_id].pTimeInfo, 0x00, 9);

        //pTp_drv->tp_atsc[tp_id].dmx_in = alloc_file_ring( 4000 * 188 * 4 );
        //if (pTp_drv->tp_atsc[tp_id].dmx_in == NULL) {
        //        TP_ERROR("[%s %d] dmx_in==NULL \n", __func__ , __LINE__ );
        //        _tp_unlock();
        //        return TPK_FAIL;
        //}
        pTp_drv->tp_atsc[tp_id].dmx_in = NULL;

        pTp_drv->tp_atsc[tp_id].cb.callback = Tp_atsc3_dmx_callback;

        pTp_drv->tp_atsc[tp_id].dmx = create_atsc3_dmx(&pTp_drv->tp_atsc[tp_id].cb, pTp_drv->tp_atsc[tp_id].dmx_in);

        if (pTp_drv->tp_atsc[tp_id].dmx == NULL) {
                TP_ERROR("[%s %d] dmx==NULL \n", __func__ , __LINE__ );
                _tp_unlock();
                return TPK_FAIL;
        }

        pTp_drv->tp_atsc[tp_id].p_in = pTp_drv->tp_atsc[tp_id].dmx->p_in;

        pTp_drv->tp_atsc[tp_id].bbp_array_write_index       = 0;
        pTp_drv->tp_atsc[tp_id].bbp_array_read_index        = 0;
        pTp_drv->tp_atsc[tp_id].bbp_array_read_index_count  = 0;

        pTp_drv->tp_atsc[tp_id].bbp_array_size   = sizeof(DELIVERY_BB_FRAME_T) * BBP_ARRAY_COUNT + BBP_ARRAY_SIZE * BBP_ARRAY_COUNT;
        pTp_drv->tp_atsc[tp_id].bbp_array_cached = dvr_malloc_specific( pTp_drv->tp_atsc[tp_id].bbp_array_size, GFP_DCU2 );
        pTp_drv->tp_atsc[tp_id].bbp_array_phy    = dvr_to_phys( pTp_drv->tp_atsc[tp_id].bbp_array_cached );

        pTp_drv->tp_atsc[tp_id].bbp_array = (DELIVERY_BB_FRAME_T *)(pTp_drv->tp_atsc[tp_id].bbp_array_cached);

        for(i = 0; i < BBP_ARRAY_COUNT; i++) {
                memset(&pTp_drv->tp_atsc[tp_id].bbp_array[i], 0x00, sizeof(DELIVERY_BB_FRAME_T));
                pTp_drv->tp_atsc[tp_id].bbp_array[i].data = (UINT8*)(&(pTp_drv->tp_atsc[tp_id].bbp_array[BBP_ARRAY_COUNT])) + BBP_ARRAY_SIZE * i;
        }

        BbfBufferZoneSize = 3670016 + 524288;
        BbfBufferZoneStartVirAddr = dvr_malloc_specific( BbfBufferZoneSize, GFP_DCU2 );
        BbfBufferZoneStartPhyAddr = dvr_to_phys( BbfBufferZoneStartVirAddr );
        TP_WARNING("BbfBuffer=> startPhyAdd:0x%x, totalSize:%d, tp_id:%d\n", BbfBufferZoneStartPhyAddr, BbfBufferZoneSize, tp_id);
        if (BbfBufferZoneStartVirAddr != 0) {
                // TP CARVEDOUT_TP Buffer size is 32MB TP0 and TP1 use 28MB, other size is 4MB, 3.5M for D Ring, 0.5M for I Ring
                pTp_drv->tp_atsc[tp_id].dring_phyAddr = BbfBufferZoneStartPhyAddr;
                pTp_drv->tp_atsc[tp_id].dring_cachedaddr = BbfBufferZoneStartVirAddr;
                pTp_drv->tp_atsc[tp_id].dring_nonCachedaddr = NULL;
                pTp_drv->tp_atsc[tp_id].dring_virAddr =  BbfBufferZoneStartVirAddr;
                pTp_drv->tp_atsc[tp_id].dring_size = 3668256 ;  // 188 Byte alignment
                //TP_WARNING("[%s]TP[%d] dring cachedAddr 0x%lx, uncachedAddr 0x%lx, phyAddr 0x%llx, Size 0x%x\n", __func__, TP_TP1,(unsigned long)(uintptr_t)pTp_drv->tp_atsc[tp_id].dring_cachedaddr, (unsigned long)(uintptr_t)pTp_drv->tp_atsc[tp_id].dring_nonCachedaddr,(long long)pTp_drv->tp_atsc[tp_id].dring_phyAddr, pTp_drv->tp_atsc[tp_id].dring_size);

                pTp_drv->tp_atsc[tp_id].iring_phyAddr = BbfBufferZoneStartPhyAddr + 3670016;
                pTp_drv->tp_atsc[tp_id].iring_cachedaddr = BbfBufferZoneStartVirAddr + 3670016;
                pTp_drv->tp_atsc[tp_id].iring_nonCachedaddr = NULL;
                pTp_drv->tp_atsc[tp_id].iring_virAddr =  BbfBufferZoneStartVirAddr + 3670016;
                pTp_drv->tp_atsc[tp_id].iring_size = 524280 ;    // 12 Byte alignment

                //TP_WARNING("[%s]TP[%d] iring cachedAddr 0x%lx, uncachedAddr 0x%lx, phyAddr 0x%llx, Size 0x%x\n", __func__, TP_TP1,(unsigned long)(uintptr_t)pTp_drv->tp_atsc[tp_id].iring_cachedaddr, (unsigned long)(uintptr_t)pTp_drv->tp_atsc[tp_id].iring_nonCachedaddr,(long long)pTp_drv->tp_atsc[tp_id].iring_phyAddr, pTp_drv->tp_atsc[tp_id].iring_size);
                Tp_ATSC30_Setting(tp_id);
        }

        TP_WARNING("dring_phyAddr 0x%llx dring_nonCachedaddr=0x%p\n", (long long)pTp_drv->tp_atsc[0].dring_phyAddr, pTp_drv->tp_atsc[0].dring_nonCachedaddr);
        pTp_drv->tp_atsc[tp_id].open_count++;
        _tp_unlock();

        TP_DBG("RHAL_ATSC30_Open tp_id=%d bbp_array_phy="PT_FMT_UADDR"\n", tp_id, pTp_drv->tp_atsc[tp_id].bbp_array_phy);
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_ATSC30_Open);

/*------------------------------------------------------------------
 * Func : RHAL_ATSC30_Close
 *
 * Desc :
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_ATSC30_Close(TPK_TP_ENGINE_T tp_id)
{
        _tp_lock();

        if(pTp_drv->tp_atsc[tp_id].open_count == 0) {
                TP_ERROR("[%s %d] already close , just return \n", __func__ , __LINE__);
                _tp_unlock();
                return TPK_RESOURCE_CONFLICT;
        }

        pTp_drv->tp_atsc[tp_id].open_count--;

        dvr_free(pTp_drv->tp_atsc[tp_id].bbp_array_cached) ;
        pTp_drv->tp_atsc[tp_id].bbp_array_cached = 0;

        dvr_free(pTp_drv->tp_atsc[tp_id].dring_cachedaddr) ;
        pTp_drv->tp_atsc[tp_id].dring_cachedaddr = NULL;
        pTp_drv->tp_atsc[tp_id].dring_nonCachedaddr = NULL;
        pTp_drv->tp_atsc[tp_id].dring_size = 0 ;


        pTp_drv->tp_atsc[tp_id].iring_cachedaddr = NULL;
        pTp_drv->tp_atsc[tp_id].iring_nonCachedaddr = NULL;
        pTp_drv->tp_atsc[tp_id].iring_size = 0 ;

        pTp_drv->tp_atsc[tp_id].clock_recovery = 1;
        pTp_drv->tp_atsc[tp_id].wall_clock     = 0;
        memset(pTp_drv->tp_atsc[tp_id].pTimeInfo, 0x00, 9);

        destroy_atsc3_dmx(pTp_drv->tp_atsc[tp_id].dmx);

        //if(pTp_drv->tp_atsc[tp_id].dmx_in != NULL) {
        //        destroy_file_ring(pTp_drv->tp_atsc[tp_id].dmx_in);
        //        pTp_drv->tp_atsc[tp_id].dmx_in = NULL;
        //}

        _tp_unlock();
        TP_DBG("RHAL_ATSC30_Close tp_id=%d\n", tp_id);
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_ATSC30_Close);

/*------------------------------------------------------------------
 * Func : RHAL_ATSC30_RequestBBFrame
 *
 * Desc :
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_ATSC30_RequestBBFrame(TPK_TP_ENGINE_T tp_id)
{
        UINT8 plp_index = 0;
        UINT16 i = 0;
        _tp_lock();

        //pTp_drv->tp[tp_id].mass_buffer.RP_Connect[CI_BUFFER_RP] = pTp_drv->tp[tp_id].mass_buffer.RP_Connect[SECTION_BUFFER_RP];
        if(pTp_drv->tp_atsc[tp_id].isATSC30 == 1) {
                _tp_unlock();
                return TPK_RESOURCE_CONFLICT;
        }

        pTp_drv->tp_atsc[tp_id].for_debug_atsc_request_bbf_time = READ_REG32(TIMER_SCPU_CLK90K_LO_reg);

        pTp_drv->tp_atsc[tp_id].bbp_array_write_index       = 0;
        pTp_drv->tp_atsc[tp_id].bbp_array_read_index        = 0;
        pTp_drv->tp_atsc[tp_id].bbp_array_read_index_count  = 0;

        pTp_drv->tp_atsc[tp_id].for_debug_first_bbf_in_time       = 0;
        pTp_drv->tp_atsc[tp_id].for_debug_first_valid_bbf_in_time = 0;
        pTp_drv->tp_atsc[tp_id].for_debug_first_get_bbf_time      = 0;
        pTp_drv->tp_atsc[tp_id].for_debug_first_release_bbf_time  = 0;
        pTp_drv->tp_atsc[tp_id].for_debug_bbf_total_size          = 0;
        pTp_drv->tp_atsc[tp_id].for_debug_ring_buffer_full_cnt    = 0;

        pTp_drv->tp_atsc[tp_id].wallclock_stc_offset              = 0;

        pTp_drv->tp_atsc[tp_id].error_cnt = 0;

        for(plp_index = 0; plp_index < ATSC30_PLP_MAX; plp_index++) {
                pTp_drv->tp_atsc[tp_id].last_bbp_cc[plp_index] = 0;
                pTp_drv->tp_atsc[tp_id].plp_cnt[plp_index]     = 0;
        }

        pTp_drv->tp_atsc[tp_id].stc_monitor_index = 0;

        for(i = 0; i < ATSC30_STC_MONITOR_CNT; i++) {
                pTp_drv->tp_atsc[tp_id].stc_monitor[i] = 0;
        }

        Tp_ATSC30_StreamControl(tp_id, TP_ATSC_STREAM_START);

        pTp_drv->tp_atsc[tp_id].dmx->ops.start(pTp_drv->tp_atsc[tp_id].dmx);

        pTp_drv->tp_atsc[tp_id].isATSC30 = 1;

        tp_dbg_thread_start_stop(1);

        _tp_unlock();
        TP_DBG("#DeliveryPerf# [%d] RHAL_ATSC30_RequestBBFrame tp_id=%d\n", READ_REG32(TIMER_SCPU_CLK90K_LO_reg) / 90, tp_id);
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_ATSC30_RequestBBFrame);

/*------------------------------------------------------------------
 * Func : RHAL_ATSC30_CancelBBFrame
 *
 * Desc :
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_ATSC30_CancelBBFrame(TPK_TP_ENGINE_T tp_id)
{
        _tp_lock();

        pTp_drv->tp_atsc[tp_id].for_debug_atsc_cancel_bbf_time = READ_REG32(TIMER_SCPU_CLK90K_LO_reg);

        if(pTp_drv->tp_atsc[tp_id].isATSC30 != 0) {
                pTp_drv->tp[tp_id].mass_buffer.RP_Connect[CI_BUFFER_RP] = INVALID_BUFFER;
                pTp_drv->tp_atsc[tp_id].dmx->ops.stop(pTp_drv->tp_atsc[tp_id].dmx);
                Tp_ATSC30_StreamControl(tp_id, TP_ATSC_STREAM_STOP);
                pTp_drv->tp_atsc[tp_id].isATSC30 = 0;
        } else {
                TP_DBG("RHAL_ATSC30_CancelBBFrame tp_id=%d BBFrame not requested, just return\n", tp_id);
        }

        tp_dbg_thread_start_stop(0);

        _tp_unlock();
        TP_DBG("RHAL_ATSC30_CancelBBFrame tp_id=%d\n", tp_id);
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_ATSC30_CancelBBFrame);

/*------------------------------------------------------------------
 * Func : RHAL_ATSC30_GetBBFrame
 *
 * Desc :
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_ATSC30_GetBBFrame(TPK_TP_ENGINE_T tp_id, UINT32 *bbframe_address, UINT32 *count)
{
        _tp_lock();

        if(pTp_drv->tp_atsc[tp_id].bbp_array_write_index >= pTp_drv->tp_atsc[tp_id].bbp_array_read_index) {
                *count = pTp_drv->tp_atsc[tp_id].bbp_array_write_index - pTp_drv->tp_atsc[tp_id].bbp_array_read_index;
        } else {
                *count = BBP_ARRAY_COUNT - pTp_drv->tp_atsc[tp_id].bbp_array_read_index;
        }
        *bbframe_address = (&pTp_drv->tp_atsc[tp_id].bbp_array[pTp_drv->tp_atsc[tp_id].bbp_array_read_index]) - pTp_drv->tp_atsc[tp_id].bbp_array;
        pTp_drv->tp_atsc[tp_id].bbp_array_read_index_count = *count;

        if(pTp_drv->tp_atsc[tp_id].for_debug_first_get_bbf_time == 0)
                pTp_drv->tp_atsc[tp_id].for_debug_first_get_bbf_time = READ_REG32(TIMER_SCPU_CLK90K_LO_reg);

        _tp_unlock();
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_ATSC30_GetBBFrame);

/*------------------------------------------------------------------
 * Func : RHAL_ATSC30_ReturnBBFrame
 *
 * Desc :
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_ATSC30_ReturnBBFrame(TPK_TP_ENGINE_T tp_id, UINT32 bbframe_address)
{
        int i;
        _tp_lock();

        if(pTp_drv->tp_atsc[tp_id].for_debug_first_release_bbf_time == 0)
                pTp_drv->tp_atsc[tp_id].for_debug_first_release_bbf_time = READ_REG32(TIMER_SCPU_CLK90K_LO_reg);

        for(i = 0; i < pTp_drv->tp_atsc[tp_id].bbp_array_read_index_count; i++) {
                unsigned int real_read_index = pTp_drv->tp_atsc[tp_id].bbp_array_read_index + i;
                if(real_read_index >= BBP_ARRAY_COUNT)
                        real_read_index = real_read_index - BBP_ARRAY_COUNT;

                if( ((pTp_drv->tp_atsc[tp_id].last_bbp_cc[pTp_drv->tp_atsc[tp_id].bbp_array[real_read_index].plp_id] + 1) & 0xff) != pTp_drv->tp_atsc[tp_id].bbp_array[real_read_index].cc) {
                        TP_WARNING("BBP CC is not continue want=0x%02x real=0x%02x PLP=0x%x\n", (pTp_drv->tp_atsc[tp_id].last_bbp_cc[pTp_drv->tp_atsc[tp_id].bbp_array[real_read_index].plp_id] + 1), pTp_drv->tp_atsc[tp_id].bbp_array[real_read_index].cc, pTp_drv->tp_atsc[tp_id].bbp_array[real_read_index].plp_id);
                }
                pTp_drv->tp_atsc[tp_id].last_bbp_cc[pTp_drv->tp_atsc[tp_id].bbp_array[real_read_index].plp_id] = pTp_drv->tp_atsc[tp_id].bbp_array[real_read_index].cc;

                if(rec_bbp_fd != NULL) {
#if IS_ENABLED(CONFIG_RTK_FEATURE_FOR_GKI)
                        kernel_write(rec_bbp_fd, pTp_drv->tp_atsc[tp_id].bbp_array[real_read_index].data, pTp_drv->tp_atsc[tp_id].bbp_array[real_read_index].len, &rec_bbp_fd->f_pos);
#endif
                }
        }

        pTp_drv->tp_atsc[tp_id].bbp_array_read_index = pTp_drv->tp_atsc[tp_id].bbp_array_read_index + pTp_drv->tp_atsc[tp_id].bbp_array_read_index_count;
        if(pTp_drv->tp_atsc[tp_id].bbp_array_read_index >= BBP_ARRAY_COUNT)
                pTp_drv->tp_atsc[tp_id].bbp_array_read_index = 0;

        _tp_unlock();
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_ATSC30_ReturnBBFrame);

/*------------------------------------------------------------------
 * Func : RHAL_ATSC30_GetBBFrameInfo
 *
 * Desc :
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_ATSC30_GetBBFrameInfo(TPK_TP_ENGINE_T tp_id, UINT32 *buff_phy, UINT32 *buff_size)
{
        _tp_lock();

        *buff_phy  = pTp_drv->tp_atsc[tp_id].bbp_array_phy;
        *buff_size = pTp_drv->tp_atsc[tp_id].bbp_array_size;

        _tp_unlock();
        TP_INFO("RHAL_ATSC30_GetBBFrameInfo tp_id=%d *buff_phy=0x%x *buff_size=0x%x\n", tp_id, *buff_phy, *buff_size);
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_ATSC30_GetBBFrameInfo);

/*------------------------------------------------------------------
 * Func : RHAL_ATSC30_GetTimeInfo
 *
 * Desc :
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_ATSC30_GetTimeInfo(TPK_TP_ENGINE_T tp_id, UINT32 *second, UINT32 *n_second, UINT8 *wall_clock)
{
        UINT32 value_hi, value_low;
        UINT64 pcr_value;
        UINT32 ori_second;
        UINT64 temp;

        _tp_lock();

        Tp_Get_PCR(tp_id, &value_hi, &value_low);

        pcr_value = (((UINT64)value_hi) << 32) + value_low;
        pcr_value = pcr_value + pTp_drv->tp_atsc[tp_id].wallclock_stc_offset;
        ori_second = div64_u64(pcr_value, (90 * 1000));

        if(pTp_drv->tp_atsc[tp_id].wall_clock == 1) {
            *second = ori_second - pTp_drv->tp_atsc[tp_id].current_utc_offset - pTp_drv->tp_atsc[tp_id].leap59 + pTp_drv->tp_atsc[tp_id].leap61;
        } else {
            *second = ori_second;
        }

        temp = mul_u64_u32_div(pcr_value, 1000000, 90);
        div_u64_rem(temp, 1000000000, n_second);
        *wall_clock = pTp_drv->tp_atsc[tp_id].wall_clock;

        _tp_unlock();
        TP_INFO("RHAL_ATSC30_GetTimeInfo tp_id=%u pcr_value=%llu ori_second=%u sec=%u nsec=%10u wall_clock=%u\n", tp_id, pcr_value, ori_second, *second, *n_second, *wall_clock);
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_ATSC30_GetTimeInfo);

/*------------------------------------------------------------------
 * Func : RHAL_ATSC30_SetTimeInfo
 *
 * Desc :
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_ATSC30_SetTimeInfo(TPK_TP_ENGINE_T tp_id, UINT32 second, UINT32 n_second, UINT8 wall_clock)
{
        UINT32 value_hi, value_low;
        UINT64 pcr_value, stc_local;

        _tp_lock();

        pcr_value = (second * (UINT64)1000 + n_second / 1000000) * (UINT64)90;

        Tp_Get_PCR(tp_id, &value_hi, &value_low);
        stc_local = (((UINT64)value_hi) << 32) + value_low;
        pTp_drv->tp_atsc[tp_id].wallclock_stc_offset = pcr_value - stc_local;

        _tp_unlock();
        TP_DBG("RHAL_ATSC30_SetTimeInfo tp_id=%d sec=%d nsec=%d wall_clock=%d wallclock_stc_offset=%lld\n", tp_id, second, n_second, wall_clock, pTp_drv->tp_atsc[tp_id].wallclock_stc_offset);
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_ATSC30_SetTimeInfo);

/*------------------------------------------------------------------
 * Func : RHAL_ATSC30_SetSystemTimeInfo
 *
 * Desc :
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_ATSC30_SetSystemTimeInfo(TPK_TP_ENGINE_T tp_id, UINT8 current_utc_offset, UINT16 ptp_prepend, UINT8 leap59, UINT8 leap61)
{
        _tp_lock();

        Tp_Delivery_Set_SystemTime(tp_id, current_utc_offset, ptp_prepend, leap59, leap61);

        _tp_unlock();
        TP_DBG("RHAL_ATSC30_SetSystemTimeInfo tp_id=%d current_utc_offset=%d ptp_prepend=%d leap59=%d leap61=%d\n", tp_id, current_utc_offset, ptp_prepend, leap59, leap61);
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_ATSC30_SetSystemTimeInfo);

/*------------------------------------------------------------------
 * Func : RHAL_ATSC30_SetClockRecovery
 *
 * Desc :
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_ATSC30_SetClockRecovery(TPK_TP_ENGINE_T tp_id, UINT8 isNeedClockRecovery)
{
        _tp_lock();

        pTp_drv->tp_atsc[tp_id].clock_recovery = isNeedClockRecovery;
        pTp_drv->tp_japan[tp_id].clock_recovery = isNeedClockRecovery;

        if(isNeedClockRecovery == 0) {
                pTp_drv->tp_atsc[tp_id].wall_clock = 0;
                pTp_drv->tp_japan[tp_id].wall_clock = 0;
        }

        _tp_unlock();
        TP_DBG("RHAL_ATSC30_SetClockRecovery tp_id=%d isNeedClockRecovery=%d\n", tp_id, isNeedClockRecovery);
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_ATSC30_SetClockRecovery);

/*------------------------------------------------------------------
 * Func : RHAL_Delivery_ATSC30_SetInputConfig
 *
 * Desc :
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_Delivery_ATSC30_SetInputConfig(TPK_TP_ENGINE_T tp_id, TPK_INPUT_PORT_T  inputPort, TPK_INPUT_TYPE_T  inputType)
{
        INT32 ret = TPK_SUCCESS;

        _tp_lock();

        if(TP_Get_MCM_ATSC3_Exist()==1) {
                inputPort = TPK_PORT_EXT_INPUT0;
        }

        pTp_drv->tp_atsc[tp_id].input_port = inputPort;

        _tp_unlock();

        TP_DBG("RHAL_Delivery_ATSC30_SetInputConfig tp_id=%d portType=%d\n", tp_id, inputPort);
        return ret;
}
EXPORT_SYMBOL(RHAL_Delivery_ATSC30_SetInputConfig);
#endif

/*------------------------------------------------------------------
 * Func : RHAL_Delivery_SetMode
 * Desc :
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
void RHAL_Delivery_SetMode(TPK_TP_ENGINE_T tp_id, UINT8 mode)
{
    pTp_drv->deliveryMode[tp_id] = mode;
    TP_DBG("RHAL_Delivery_SetMode tp:%d mode=%d\n",tp_id, mode);
}
EXPORT_SYMBOL(RHAL_Delivery_SetMode);

/*------------------------------------------------------------------
 * Func : RHAL_Delivery_GetMode
 * Desc :
 * Retn : mode
 *------------------------------------------------------------------*/
UINT8 RHAL_Delivery_GetMode(TPK_TP_ENGINE_T tp_id)
{
        return pTp_drv->deliveryMode[tp_id];
}
EXPORT_SYMBOL(RHAL_Delivery_GetMode);

/*------------------------------------------------------------------
 * Func : RHAL_Delivery_GetSourceType
 * Desc :
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
UINT16 RHAL_Delivery_GetSourceType(TPK_TP_ENGINE_T tp_id)
{
    UINT16 srcType = -1;
    srcType = pTp_drv->tp_japan[tp_id].src_type;
    TP_DBG("RHAL_Delivery_GetSourceType  TP:%d srcType=%d\n", tp_id, srcType);
    return srcType;
}
EXPORT_SYMBOL(RHAL_Delivery_GetSourceType);

#if defined(CONFIG_RTK_KDRV_JP4K)
extern RTK_TLV_REG_T tlv_reg_tlv0;
#ifdef CONFIG_RTK_SUPPORT_TWO_HWTLV
extern RTK_TLV_REG_T tlv_reg_tlv1;
#endif
UINT8 RHAL_Delivery_GetValidFilterTableIdx(UINT32 tp_id)
{
    UINT8 i;

    for(i = 0; i < TLV_PACKET_FILTER_MAX_NUM; i++){
        if(!pTp_drv->tp_japan[tp_id].filter_table_idx[i]){
            pTp_drv->tp_japan[tp_id].filter_table_idx[i] = 1;
            return i;
        }
    }
    return -1;
}

void RHAL_Delivery_ReturnFilterTableIdx(UINT32 tp_id, UINT8 idx)
{
    pTp_drv->tp_japan[tp_id].filter_table_idx[idx]  = 0;
}
/*------------------------------------------------------------------
 * Func : RHAL_Delivery_RequestData
 * Desc :
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_Delivery_RequestData(TPK_TP_ENGINE_T tp_id)
{
        TP_BUFFER_PARAM_T param;
        unsigned char *tp_buffer;
        UINT8    *tlv_buffer;
        UADDRESS tlv_buffer_phy;

        _tp_lock();

        if(pTp_drv->tp_japan[tp_id].isJAPAN4K == 1) {
            _tp_unlock();
            return TPK_RESOURCE_CONFLICT;
        }

        if ( pTp_drv->tp_japan[tp_id].src_type == TPK_DELI_SRC_TYPE_TLV_SW0
            || pTp_drv->tp_japan[tp_id].src_type == TPK_DELI_SRC_TYPE_TS ) {

            UINT8 buf_idx = 0;
            tp_buffer = dvr_malloc_specific( TLV_RINGBUFFER_SIZE, GFP_DCU2);
            if(NULL == tp_buffer){
                _tp_unlock();
                TP_ERROR("tp_id:%d, malloc failed,not enough L:%d!!", tp_id, __LINE__);
                return TPK_NOT_ENOUGH_RESOURCE;
            }

            param.Base = dvr_to_phys( tp_buffer );
            param.RP = param.WP = param.Base;
            param.Limit = param.Base + (TLV_RINGBUFFER_SIZE / (188 * 192)) * (188 * 192);
            buf_idx = Tp_Get_RingBuffer_Index(tp_id);
            Tp_Set_Buffer(buf_idx, param, BUFFER_WRITE_MASK_NONE);

            /* save to data structure */
            pTp_drv->tp[tp_id].mass_buffer.OriBufSize = (TLV_RINGBUFFER_SIZE / (188 * 192)) * (188 * 192);
            pTp_drv->tp[tp_id].mass_buffer.BasePhy    = param.Base;
            pTp_drv->tp[tp_id].mass_buffer.LimitPhy   = param.Base + (TLV_RINGBUFFER_SIZE / (188 * 192)) * (188 * 192);
            pTp_drv->tp[tp_id].mass_buffer.BufSize    = param.Limit - param.Base;
            pTp_drv->tp[tp_id].mass_buffer.BaseVir    = (UADDRESS)tp_buffer;
            pTp_drv->tp[tp_id].mass_buffer.LimitVir   = (UADDRESS)tp_buffer + (TLV_RINGBUFFER_SIZE / (188 * 192)) * (188 * 192);
            pTp_drv->tp[tp_id].mass_buffer.RP         = param.Base;
            pTp_drv->tp[tp_id].mass_buffer.WP         = param.Base;
            pTp_drv->tp[tp_id].mass_buffer.RP_Connect[MASS_DATA_BUFFER_RP] = param.Base;
            pTp_drv->tp[tp_id].mass_buffer.RP_Connect[SECTION_BUFFER_RP]   = 0;
            pTp_drv->tp[tp_id].mass_buffer.RP_Connect[CI_BUFFER_RP]        = 0;
            pTp_drv->tp[tp_id].mass_buffer.now_rp = param.Base;
            pTp_drv->tp[tp_id].mass_buffer.RP_Connect_cnt = 1;
            pTp_drv->tp[tp_id].mass_buffer.inited     = 1;
            pTp_drv->tp[tp_id].mass_buffer.bock_size = 188;
            Tp_FRC_SetPrefix( tp_id, TP_FRC_NO_PREFIX, 0x00);
        }else{
            UINT32 i;
            for(i=0;i<TLV_DESCRIPTOR_CNT;i++){
               tlv_buffer = (UINT8 *)dvr_malloc_specific(TLV_DESCRIPTOR_BUFFER_SIZE, GFP_DCU2);
               if(NULL == tlv_buffer){
    	           TP_ERROR("tp_id:%d, malloc failed,not enough L:%d!!", tp_id, __LINE__);
    	           _tp_unlock();
    	           return TPK_NOT_ENOUGH_RESOURCE;
               }
               tlv_buffer_phy = dvr_to_phys( tlv_buffer );

               pTp_drv->tp_japan[tp_id].tlv_descripor_buffer[i].OriBufSize = TLV_DESCRIPTOR_BUFFER_SIZE;
               pTp_drv->tp_japan[tp_id].tlv_descripor_buffer[i].BasePhy	= tlv_buffer_phy;
               pTp_drv->tp_japan[tp_id].tlv_descripor_buffer[i].LimitPhy	= pTp_drv->tp_japan[tp_id].tlv_descripor_buffer[i].BasePhy + TLV_DESCRIPTOR_BUFFER_SIZE;
               pTp_drv->tp_japan[tp_id].tlv_descripor_buffer[i].BufSize	= TLV_DESCRIPTOR_BUFFER_SIZE;
               pTp_drv->tp_japan[tp_id].tlv_descripor_buffer[i].BaseVir	= (UADDRESS)tlv_buffer;
               pTp_drv->tp_japan[tp_id].tlv_descripor_buffer[i].LimitVir	= pTp_drv->tp_japan[tp_id].tlv_descripor_buffer[i].BaseVir + TLV_DESCRIPTOR_BUFFER_SIZE;
           }
#ifdef CONFIG_RTK_SUPPORT_HWTLV_FILTER
           TP_TLV_EnableHWParseNTP((RTK_TLV_REG_T *)pTp_drv->tp_japan[tp_id].dmx->ptlv_regs);
           TP_TLV_FilterInit((RTK_TLV_REG_T *)pTp_drv->tp_japan[tp_id].dmx->ptlv_regs);
#endif
           TP_TLV_Config(tp_id, (RTK_TLV_REG_T *)pTp_drv->tp_japan[tp_id].dmx->ptlv_regs);
        }

        if( pTp_drv->tp_japan[tp_id].src_type != TPK_DELI_SRC_TYPE_TS ){
           pTp_drv->tp_japan[tp_id].cb.callback = Tp_japan4k_dmx_callback;
           pTp_drv->tp_japan[tp_id].dmx->cb     = pTp_drv->tp_japan[tp_id].cb;
        }
        pTp_drv->tp_japan[tp_id].isJAPAN4K = 1;
        pTp_drv->tp_japan[tp_id].for_debug_tlv_total_size       = 0;
        pTp_drv->tp_japan[tp_id].for_debug_ring_buffer_full_cnt = 0;

        tp_dbg_thread_start_stop(1);
        _tp_unlock();
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_Delivery_RequestData);

/*------------------------------------------------------------------
 * Func : RHAL_Delivery_CancelData
 * Desc :
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_Delivery_CancelData(TPK_TP_ENGINE_T tp_id)
{
    _tp_lock();

    if ( pTp_drv->tp_japan[tp_id].src_type == TPK_DELI_SRC_TYPE_TLV_SW0
        || pTp_drv->tp_japan[tp_id].src_type == TPK_DELI_SRC_TYPE_TS ) {

        pTp_drv->tp[tp_id].mass_buffer.RP_Connect[CI_BUFFER_RP] = INVALID_BUFFER;
        pTp_drv->tp[tp_id].mass_buffer.RP_Connect[MASS_DATA_BUFFER_RP] = INVALID_BUFFER;

        if (pTp_drv->tp[tp_id].mass_buffer.BaseVir != 0x00) {
            TP_DBG("tp_id:%d, RHAL_Delivery_CancelData. free:%lu, L:%d!!", tp_id, pTp_drv->tp[tp_id].mass_buffer.BaseVir, __LINE__);
            dvr_free( (UINT8*)(pTp_drv->tp[tp_id].mass_buffer.BaseVir));
            pTp_drv->tp[tp_id].mass_buffer.BaseVir = 0x00;
        }
    } else {
        UINT32 i;
#ifdef CONFIG_RTK_SUPPORT_HWTLV_FILTER
        TP_TLV_DisableHWParseNTP((RTK_TLV_REG_T *)pTp_drv->tp_japan[tp_id].dmx->ptlv_regs);
        TP_TLV_FilterUnInit((RTK_TLV_REG_T *)pTp_drv->tp_japan[tp_id].dmx->ptlv_regs);
#endif
        TP_TLV_Release((RTK_TLV_REG_T *)pTp_drv->tp_japan[tp_id].dmx->ptlv_regs);

        for (i = 0; i < TLV_DESCRIPTOR_CNT; i++){
            if(pTp_drv->tp_japan[tp_id].tlv_descripor_buffer[i].BaseVir != 0x00) {
                dvr_free( (UINT8*)(pTp_drv->tp_japan[tp_id].tlv_descripor_buffer[i].BaseVir));
                pTp_drv->tp_japan[tp_id].tlv_descripor_buffer[i].BaseVir = 0x00;
            }
        }
        TP_DBG("tp_id:%d, RHAL_Delivery_CancelData. L:%d!!", tp_id, __LINE__);
    }

    pTp_drv->tp_japan[tp_id].isJAPAN4K = 0;

    tp_dbg_thread_start_stop(0);

    _tp_unlock();
    return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_Delivery_CancelData);

int RHAL_Delivery_GetTsData(TPK_TP_ENGINE_T tp_id, char *buffer, UINT32 *buffer_size)
{
    int result;
    UINT32 len = 0;
    unsigned char *p_rp = NULL;

    result = TP_ReadData_NoNeedAligment(tp_id, &p_rp, &len, 1024, TP_BUFFER_MASS_DATA);
    p_rp = Tp_Buf_Phy_to_Vir(tp_id, TP_BUFFER_MASS_DATA, (unsigned char*)p_rp);
    if(result < 0) {
        TP_VERBOSE("%s read data failed, result=%d\n", __func__, result);
        *buffer_size = 0;
        return -1;
    }

    if( len > *buffer_size )
        len = *buffer_size;

    if( copy_to_user( (void __user *)to_user_ptr(buffer), (const void __user *)p_rp, len) ) {
        TP_ERROR("%s copy_to_user Fail\n", __func__);
        return -1;
    } else {
        *buffer_size = len;
    }

    p_rp = Tp_Buf_Vir_to_Phy(tp_id, TP_BUFFER_MASS_DATA, (unsigned char*)p_rp);
    TP_ReleaseData_NoNeedAligment(tp_id, p_rp, len, TP_BUFFER_MASS_DATA);
    return 0;
}
EXPORT_SYMBOL(RHAL_Delivery_GetTsData);
/*------------------------------------------------------------------
 * Func : RHAL_Delivery_GetData
 * Desc :
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_Delivery_GetData(TPK_TP_ENGINE_T tp_id, char *buffer, UINT32 *buffer_size)
{
    INT32 ret = TPK_SUCCESS;

    _tp_lock();

    if(pTp_drv->deliveryMode[tp_id] == TP_DELIVERY_MODE_CI20 ){
        RHAL_TPP_Delivery_Data(tp_id, buffer, buffer_size);
    }else if(pTp_drv->deliveryMode[tp_id] == TP_DELIVERY_MODE_TS ){
        RHAL_Delivery_GetTsData(tp_id, buffer, buffer_size);
    }else{
        pTp_drv->tp_japan[tp_id].dmx->rhal_tlv_buffer              = buffer;
        pTp_drv->tp_japan[tp_id].dmx->rhal_tlv_buffer_max_size = *buffer_size;
        pTp_drv->tp_japan[tp_id].dmx->rhal_tlv_buffer_size         = 0;

        japan4k_dmx_run(pTp_drv->tp_japan[tp_id].dmx);

        *buffer_size = pTp_drv->tp_japan[tp_id].dmx->rhal_tlv_buffer_size;
	}
    _tp_unlock();

    //TP_DBG("RHAL_Delivery_GetData tp_id=%d buffer_size=%d\n", tp_id, *buffer_size);
    return ret;
}
EXPORT_SYMBOL(RHAL_Delivery_GetData);

/*------------------------------------------------------------------
 * Func : RHAL_JAPAN4K_Open
 * Desc :
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_JAPAN4K_Open(TPK_TP_ENGINE_T tp_id)
{
        _tp_lock();

        if(pTp_drv->tp_japan[tp_id].open_count != 0) {
                TP_ERROR("[%s %d] Already Start , just return \n", __func__ , __LINE__ );
                _tp_unlock();
                return TPK_FAIL;
        }
        pTp_drv->tp_japan[tp_id].clock_recovery = 1;
        pTp_drv->tp_japan[tp_id].wall_clock     = 0;

        pTp_drv->tp_japan[tp_id].dmx = create_japan4k_dmx(tp_id);
        if (pTp_drv->tp_japan[tp_id].dmx == NULL) {
            TP_ERROR("[%s %d] Error!! malloc japan4k_dmx fail \n", __func__ , __LINE__ );
            _tp_unlock();
            return TPK_FAIL;
        }
        pTp_drv->tp_japan[tp_id].dmx->tp_id = tp_id;
        pTp_drv->tp_japan[tp_id].src_type = TPK_DELI_SRC_TYPE_NONE;
        pTp_drv->tp_japan[tp_id].open_count++;

        _tp_unlock();
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_JAPAN4K_Open);

/*------------------------------------------------------------------
 * Func : RHAL_JAPAN4K_Close
 * Desc :
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_JAPAN4K_Close(TPK_TP_ENGINE_T tp_id)
{
        _tp_lock();

        if(pTp_drv->tp_japan[tp_id].open_count == 0) {
                TP_ERROR("[%s %d] already close , just return \n", __func__ , __LINE__);
                _tp_unlock();
                return TPK_RESOURCE_CONFLICT;
        }

        pTp_drv->tp_japan[tp_id].open_count--;
        pTp_drv->tp_japan[tp_id].clock_recovery = 1;
        pTp_drv->tp_japan[tp_id].wall_clock     = 0;
        pTp_drv->tp_japan[tp_id].dmx->tp_id = MAX_TP_COUNT;
        pTp_drv->tp_japan[tp_id].src_type = TPK_DELI_SRC_TYPE_NONE;
        pTp_drv->tp_japan[tp_id].dmx = NULL;
        //destroy_japan4k_dmx(pTp_drv->tp_japan[tp_id].dmx);

        _tp_unlock();
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_JAPAN4K_Close);

/*------------------------------------------------------------------
 * Func : RHAL_Delivery_JAPAN4K_SetInputConfig
 *
 * Desc :
 *
 *
 * Retn : TPK_SUCCESS
 *------------------------------------------------------------------*/
INT32 RHAL_Delivery_JAPAN4K_SetInputConfig(TPK_TP_ENGINE_T tp_id, TPK_INPUT_PORT_T  inputPort, TPK_INPUT_TYPE_T  inputType, TPK_DELIVERY_SRC_TYPE_T srcType)
{
    INT32 ret = TPK_SUCCESS;
    INT32 i;

    _tp_lock();
    pTp_drv->tp_japan[tp_id].input_port = inputPort;
    pTp_drv->tp_japan[tp_id].inputType  = inputType;

    for(i = 0; i < MAX_TP_COUNT; i++){
        if(pTp_drv->tp_japan[i].src_type == srcType ){
            break;
        }
    }
    if(i == MAX_TP_COUNT){
        pTp_drv->tp_japan[tp_id].src_type = srcType;
    } else {
        TP_ERROR("TP%d set the same TLV source type with TP:%d src:%d \n", i, tp_id, srcType);
        _tp_unlock();
        return TPK_FAIL;
    }

    if(pTp_drv->tp_japan[tp_id].src_type == TPK_DELI_SRC_TYPE_TLV_HW0)
        pTp_drv->tp_japan[tp_id].dmx->ptlv_regs = (void*)(&tlv_reg_tlv0);
#ifdef CONFIG_RTK_SUPPORT_TWO_HWTLV
    else if(pTp_drv->tp_japan[tp_id].src_type == TPK_DELI_SRC_TYPE_TLV_HW1)
        pTp_drv->tp_japan[tp_id].dmx->ptlv_regs = (void*)(&tlv_reg_tlv1);
#endif
    else
        pTp_drv->tp_japan[tp_id].dmx->ptlv_regs = NULL;

    _tp_unlock();

    if ( pTp_drv->tp_japan[tp_id].src_type == TPK_DELI_SRC_TYPE_TLV_SW0 ) {
        TPK_TP_SOURCE_T tp_src;
        TPK_TP_STATUS_T tpStatus;

        if (RHAL_GetTpStatus(tp_id, &tpStatus) != TPK_SUCCESS){
            TP_ERROR("[%s %d] RHAL_GetTpStatus failed \n", __func__ , __LINE__);
            ret = TPK_FAIL;
            goto end;
        }

        if(inputType == TPK_INPUT_SERIAL)
            tpStatus.tp_param.serial = 1;
        else
            tpStatus.tp_param.serial = 0;

        if (RHAL_SetTPMode(tp_id, tpStatus.tp_param) != TPK_SUCCESS){
            TP_ERROR("[%s %d] RHAL_SetTPMode failed \n", __func__ , __LINE__);
            ret = TPK_FAIL;
            goto end;
        }

        if(TPK_PORT_EXT_INPUT0 == inputPort)
            tp_src = TS_SRC_0;
        else if(TPK_PORT_EXT_INPUT1 == inputPort)
            tp_src = TS_SRC_1;
        else
            tp_src = TS_SRC_2;

        if(RHAL_SetTPSource(tp_id, tp_src, TPK_WITHOUT_CAS) != TPK_SUCCESS){
            TP_ERROR("[%s %d] RHAL_SetTPSource failed \n", __func__ , __LINE__);
            ret = TPK_FAIL;
            goto end;
        }
    }
end:
    TP_DBG("RHAL_Delivery_JAPAN4K_SetInputConfig tp_id=%d inputPort=%d\n", tp_id, inputPort);
    return ret;
}
EXPORT_SYMBOL(RHAL_Delivery_JAPAN4K_SetInputConfig);


INT32 RHAL_Delivery_TS_SetInputConfig(TPK_TP_ENGINE_T tp_id, TPK_INPUT_PORT_T  inputPort, TPK_INPUT_TYPE_T  inputType, TPK_DELIVERY_SRC_TYPE_T srcType)
{
    INT32 ret = TPK_SUCCESS;
    TPK_TP_SOURCE_T tp_src;
    TPK_TP_STATUS_T tpStatus;

    pTp_drv->tp_japan[tp_id].input_port = inputPort;
    pTp_drv->tp_japan[tp_id].inputType  = inputType;
    pTp_drv->tp_japan[tp_id].src_type = TPK_DELI_SRC_TYPE_TS;

    if (RHAL_GetTpStatus(tp_id, &tpStatus) != TPK_SUCCESS){
        TP_ERROR("[%s %d] RHAL_GetTpStatus failed \n", __func__ , __LINE__);
        ret = TPK_FAIL;
        goto end;
    }

    if(inputType == TPK_INPUT_SERIAL)
        tpStatus.tp_param.serial = 1;
    else
        tpStatus.tp_param.serial = 0;
    tpStatus.tp_param.sync_byte = 0x47;

    if (RHAL_SetTPMode(tp_id, tpStatus.tp_param) != TPK_SUCCESS){
        TP_ERROR("[%s %d] RHAL_SetTPMode failed \n", __func__ , __LINE__);
        ret = TPK_FAIL;
        goto end;
    }

    if(TPK_PORT_EXT_INPUT0 == inputPort)
        tp_src = TS_SRC_0;
    else if(TPK_PORT_EXT_INPUT1 == inputPort)
        tp_src = TS_SRC_1;
    else if(TPK_PORT_EXT_INPUT2 == inputPort)
        tp_src = TS_SRC_2;
    else
        tp_src = Internal_Demod;

    if(RHAL_SetTPSource(tp_id, tp_src, TPK_WITHOUT_CAS) != TPK_SUCCESS){
        TP_ERROR("[%s %d] RHAL_SetTPSource failed \n", __func__ , __LINE__);
        ret = TPK_FAIL;
    }
end:
    TP_DBG("RHAL_Delivery_TS_SetInputConfig tp_id=%d inputPort=%d\n", tp_id, inputPort);
    return ret;
}
EXPORT_SYMBOL(RHAL_Delivery_TS_SetInputConfig);
/*======================================================
 * Func  : RHAL_Delivery_ReadDataFromHwtlv
 * Desc  :
 * Param : get tlv data from tp by hw tlv parse
 * Retn  : 0 : successed, others failed
 *======================================================*/
int RHAL_Delivery_ReadDataFromHWTLV(struct japan4k_dmx* p_this)
{
    unsigned long length_temp, i, descriptor_idx, tlv_packet_cnt;
    //unsigned long flag_temp, data0_temp, dbg_cnt_temp, buffer_flag_temp;
    unsigned long data1_temp, wp_temp, rp_temp;
    unsigned char *pData;
    UINT32        tlv_buffer_max_size, tlv_buffer_now_size;
    RTK_TLV_REG_T *ptlv_regs = NULL;

    JAPAN4K_DMX_TRACE();

    ptlv_regs = (RTK_TLV_REG_T *)p_this->ptlv_regs;

    while(READ_REG32(ptlv_regs->TLV_DESCRIPTOR_WRITE) != READ_REG32(ptlv_regs->TLV_DESCRIPTOR_READ))
    {
        TP_Get_TLVBufferInformation(p_this->tp_id, &tlv_buffer_max_size, &tlv_buffer_now_size);
        if( (tlv_buffer_max_size - tlv_buffer_now_size) <= TLV_DESCRIPTOR_BUFFER_SIZE )
            break;

        wp_temp = READ_REG32(ptlv_regs->TLV_DESCRIPTOR_WRITE);
        rp_temp = READ_REG32(ptlv_regs->TLV_DESCRIPTOR_READ);
        descriptor_idx = rp_temp&0x7f;
        WRITE_REG32( ptlv_regs->TLV_DESCRIPTOR, 0x00001000| descriptor_idx );

        //flag_temp    = READ_REG32(ptlv_regs->TLV_DESCRIPTOR_FLAG);
        //data0_temp   = READ_REG32(ptlv_regs->TLV_DESCRIPTOR_DATA0);
        data1_temp   = READ_REG32(ptlv_regs->TLV_DESCRIPTOR_DATA1);
        //dbg_cnt_temp = READ_REG32(ptlv_regs->TLV_DESCRIPTOR_PKT_NUM);
        //buffer_flag_temp = READ_REG32(ptlv_regs->TLV_BUFFER_FLAG);
        //pData        = (char*)((data0_temp - pTp_drv->tp_japan[p_this->tp_id].mass_buffer.BasePhy) + pTp_drv->tp_japan[p_this->tp_id].mass_buffer.BaseVir);
        pData        = (char*)(pTp_drv->tp_japan[p_this->tp_id].tlv_descripor_buffer[descriptor_idx].BaseVir);

        dmac_flush_range( pData, pData + TLV_DESCRIPTOR_BUFFER_SIZE);
        outer_flush_range((UADDRESS)data0_temp, (UADDRESS)data0_temp + TLV_DESCRIPTOR_BUFFER_SIZE);
        length_temp = data1_temp & 0x1FFFF;

        //JAPAN4K_DMX_ERROR("%lx c=%3ld l=0x%4lx w=0x%3lx r=0x%3lx f=0x%6lx bf=0x%2lx    [%02X %02X %02X %02X %02X %02X %02X %02X] %02X %02X %02X %02X %02X %02X\n", data0_temp, (data1_temp & 0x01FE0000)>>17, data1_temp & 0x1FFFF, wp_temp, rp_temp, flag_temp, buffer_flag_temp, pData[0], pData[1], pData[2], pData[3], pData[4], pData[5], pData[6], pData[7], pData[8], pData[9], pData[10], pData[11], pData[12], pData[13]);

        i = 0;
        tlv_packet_cnt = 0;
        while(i < length_temp) {
            unsigned long tlv_size;
            //unsigned long long timestamp;

            if(pData[i + TLV_STUFF_DATA_SIZE] != TLV_SYNC_BYTE) {
                JAPAN4K_DMX_ERROR("ERROR!! Data is not sync byte [%02X %02X %02X %02X %02X %02X %02X %02X] %02X %02X %02X %02X %02X %02X\n", pData[i+0], pData[i+1], pData[i+2], pData[i+3], pData[i+4], pData[i+5], pData[i+6], pData[i+7], pData[i+8], pData[i+9], pData[i+10], pData[i+11], pData[i+12], pData[i+13]);
                JAPAN4K_DMX_ERROR("wp=0x%x(%d) rp=0x%x(%d) i=%ld length_temp=%ld\n", READ_REG32(ptlv_regs->TLV_DESCRIPTOR_WRITE), READ_REG32(ptlv_regs->TLV_DESCRIPTOR_WRITE), READ_REG32(ptlv_regs->TLV_DESCRIPTOR_READ), READ_REG32(ptlv_regs->TLV_DESCRIPTOR_READ), i, length_temp);
                pTp_drv->tp_japan[p_this->tp_id].not_sync_count++;
                if(pTp_drv->tp_japan[p_this->tp_id].not_sync_count > 5) {
                    pTp_drv->tp_japan[p_this->tp_id].not_sync_count = 0;
                    JAPAN4K_DMX_ERROR("TP_TLV_BUFFER_FLAG_reg=0x%x TP_TLV_BUFFER_reg=0x%x\n", READ_REG32(ptlv_regs->TLV_BUFFER_FLAG), READ_REG32(ptlv_regs->TLV_BUFFER));
                    TP_TLV_Reset(p_this->tp_id, ptlv_regs);
                }
                break;
            }
            tlv_size = ((pData[i + TLV_STUFF_DATA_SIZE + 2]<<8)&0xff00) + pData[i + TLV_STUFF_DATA_SIZE + 3];
            if(pData[i + TLV_STUFF_DATA_SIZE + 1] != 0xff ){
                unsigned long bitream_timestamp;
                unsigned long tp_pcra = READ_REG32(TP_TP_INTERNAL_CNT_LOW_reg);
                //timestamp = ((unsigned long long)pData[i+0]<<56) + ((unsigned long long)pData[i+1]<<48) + ((unsigned long long)pData[i+2]<<40) + ((unsigned long long)pData[i+3]<<32) + ((unsigned long long)pData[i+4]<<24) + ((unsigned long long)pData[i+5]<<16) + ((unsigned long long)pData[i+6]<<8) + (unsigned long long)pData[i+7];
                /*if(japan4k_dmx_tlv_packet_filter(p_this, &(pData[i + TLV_STUFF_DATA_SIZE])) ) */{
                    p_this->cb.callback(p_this, &pData[i + TLV_STUFF_DATA_SIZE], tlv_size + 4);
                }
                bitream_timestamp = pData[i + 4]<<24 | pData[i + 5]<<16 | pData[i + 6]<< 8 |  pData[i + 7];
                if( (tp_pcra-bitream_timestamp) < 25000) {
                    pTp_drv->tp_japan[p_this->tp_id].timestamp_range1++;
                } else {
                    pTp_drv->tp_japan[p_this->tp_id].timestamp_range2++;
                }
                if( (tp_pcra-bitream_timestamp) > pTp_drv->tp_japan[p_this->tp_id].timestamp_max)
                    pTp_drv->tp_japan[p_this->tp_id].timestamp_max = (tp_pcra-bitream_timestamp);
                }

                i = i + TLV_STUFF_DATA_SIZE + 4 + tlv_size;
                tlv_packet_cnt++;
        }
        //JAPAN4K_DMX_ERROR("tlv_packet_cnt = %ld\n", tlv_packet_cnt);

        WRITE_REG32(ptlv_regs->TLV_DESCRIPTOR_DATA1 , 0x00000000 );
        WRITE_REG32(ptlv_regs->TLV_DESCRIPTOR , 0x00001100| descriptor_idx );

        if( (READ_REG32(ptlv_regs->TLV_DESCRIPTOR_FLAG)&0x10000) != 0x00)  {
            wp_temp = READ_REG32(ptlv_regs->TLV_DESCRIPTOR_WRITE);
            rp_temp = READ_REG32(ptlv_regs->TLV_DESCRIPTOR_READ);
            JAPAN4K_DMX_ERROR("full wp=0x%lx rp=0x%lx\n", wp_temp, rp_temp);
            TP_TLV_Reset(p_this->tp_id, ptlv_regs);
        }

        if( (READ_REG32(ptlv_regs->TLV_BUFFER_FLAG)&TP_TLV_BUFFER_INT_overflow_en_mask) == TP_TLV_BUFFER_INT_overflow_en_mask)  {
            JAPAN4K_DMX_ERROR("ERROR overflow TP_TLV_BUFFER_FLAG_reg=0x%x TP_TLV_BUFFER_reg=0x%x\n", READ_REG32(ptlv_regs->TLV_BUFFER_FLAG), READ_REG32(ptlv_regs->TLV_BUFFER));
        }

        rp_temp++;
        if( (rp_temp&0xff)==0x00000080 ) {
            rp_temp = ~rp_temp;
            rp_temp = rp_temp & 0x00000100;
        }
        WRITE_REG32(ptlv_regs->TLV_DESCRIPTOR_READ, rp_temp);

        TP_Get_TLVBufferInformation(p_this->tp_id, &tlv_buffer_max_size, &tlv_buffer_now_size);
        //JAPAN4K_DMX_ERROR("[%d] %ld %ld\n", p_this->tp_id, tlv_buffer_max_size, tlv_buffer_now_size);
        if( (tlv_buffer_max_size - tlv_buffer_now_size) <= 64 * 1024 )
            break;
    }
    return 0;
}
EXPORT_SYMBOL(RHAL_Delivery_ReadDataFromHWTLV);

#ifdef CONFIG_RTK_SUPPORT_HWTLV_FILTER
INT32 RHAL_Delivery_HWTLV_EnableFilter(RTK_TLV_REG_T *ptlv_regs)
{
    _tp_lock();
    /*enable filter  */
    TP_TLV_FilterEnable(ptlv_regs);
    _tp_unlock();
    return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_Delivery_HWTLV_EnableFilter);

INT32 RHAL_Delivery_HWTLV_DisableFilter(RTK_TLV_REG_T *ptlv_regs)
{
    _tp_lock();
    /*disable filter*/
    TP_TLV_FilterDisable(ptlv_regs);
    _tp_unlock();
    return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_Delivery_HWTLV_DisableFilter);

INT32 RHAL_Delivery_HWTLV_AddFilter(RTK_TLV_REG_T *ptlv_regs, UINT8 filter_idx, JP4K_DELIVERY_FILTER_T *p_filter_param)
{
    RTK_TLV_FILTER_PARAMS filter_params;
    _tp_lock();
    memcpy(&filter_params, p_filter_param, sizeof(RTK_TLV_FILTER_PARAMS));
    TP_TLV_SetFilterParams(ptlv_regs, filter_idx, &filter_params);
    _tp_unlock();
    return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_Delivery_HWTLV_AddFilter);

INT32 RHAL_Delivery_HWTLV_RemoveFilter(RTK_TLV_REG_T *ptlv_regs, UINT8 filter_idx)
{
    _tp_lock();
    TP_TLV_SetFilterParams(ptlv_regs, filter_idx, NULL);
    _tp_unlock();
    return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_Delivery_HWTLV_RemoveFilter);

INT32 RHAL_Delivery_HWTLV_GetNTP(RTK_TLV_REG_T *ptlv_regs, UINT32 *ntp_high, UINT32 *ntp_low)
{
    INT32 ret = 0;
    tp_tlv_ntp_stc_RBUS tlv_ntp_stc_reg;
    tlv_ntp_stc_reg.regValue = READ_REG32(ptlv_regs->TLV_STC);
    if(0 == tlv_ntp_stc_reg.st){
        *ntp_high = READ_REG32(ptlv_regs->TLV_NTP_HIGH);
        *ntp_low = READ_REG32(ptlv_regs->TLV_NTP_LOW);
    }else{
        TP_ERROR("TLV hw not have NTP yet");
        ret = -1;
    }
    return ret;
}
EXPORT_SYMBOL(RHAL_Delivery_HWTLV_GetNTP);

INT32 RHAL_Delivery_HWTLV_GetSTC(RTK_TLV_REG_T *ptlv_regs, UINT32 *stc_high, UINT32 *stc_low)
{
    INT32 ret = 0;
    tp_tlv_ntp_stc_RBUS tlv_ntp_stc_reg;
    tlv_ntp_stc_reg.regValue = READ_REG32(ptlv_regs->TLV_STC);
    if(0 == tlv_ntp_stc_reg.st){
        *stc_high = READ_REG32(ptlv_regs->TLV_STC_HIGH);
        *stc_low = READ_REG32(ptlv_regs->TLV_STC_LOW);
    }else{
        TP_ERROR("TLV hw not have STC yet");
        ret = -1;
    }
    return ret;
}
EXPORT_SYMBOL(RHAL_Delivery_HWTLV_GetSTC);
#endif
#endif

INT32 RHAL_TP_CheckProtectStatus(TPK_TP_ENGINE_T tp_id)
{
        UINT8 isNeedProtect;

        _tp_lock();

        Tp_Get_ProtectStatus( tp_id, &isNeedProtect);
        _tp_unlock();
        return isNeedProtect;
}
EXPORT_SYMBOL(RHAL_TP_CheckProtectStatus);

INT32 RHAL_TP_EnableMemProtect(TPK_TP_ENGINE_T tp_id, TP_TA_MEMORY_PRISON_TYPE_T mem_type, UINT32 start_address, UINT32 stop_address)
{
        _tp_ecp_lock();

        optee_tp_init();
        Tp_Ta_set_mem_prison(start_address, stop_address, mem_type, 1);

        if(mem_type == MEMORY_PRISON_TP){
            pTp_drv->useECP[tp_id] = 1;
            pTp_drv->tp[tp_id].tp_mc_start_address = start_address;
            pTp_drv->tp[tp_id].tp_mc_end_address   = stop_address;
        }else if(mem_type == MEMORY_PRISON_VIDEO_ES){
            pTp_drv->tp[tp_id].video_es_mc_start_address = start_address;
            pTp_drv->tp[tp_id].video_es_mc_end_address   = stop_address;
        }else if(mem_type == MEMORY_PRISON_AUDIO_ES){
            pTp_drv->tp[tp_id].audio_es_mc_start_address = start_address;
            pTp_drv->tp[tp_id].audio_es_mc_end_address   = stop_address;
        }
        _tp_ecp_unlock();
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_TP_EnableMemProtect);

INT32 RHAL_TP_DisableMemProtect(TPK_TP_ENGINE_T tp_id, TP_TA_MEMORY_PRISON_TYPE_T mem_type)
{
        _tp_ecp_lock();
        Tp_Ta_set_mem_prison(pTp_drv->tp[tp_id].tp_mc_start_address, pTp_drv->tp[tp_id].tp_mc_end_address, mem_type, 0);

        if(mem_type == MEMORY_PRISON_TP){
            pTp_drv->useECP[tp_id] = 0;
            pTp_drv->tp[tp_id].tp_mc_start_address = 0;
            pTp_drv->tp[tp_id].tp_mc_end_address   = 0;
        }else if(mem_type == MEMORY_PRISON_VIDEO_ES){
            pTp_drv->tp[tp_id].video_es_mc_start_address = 0;
            pTp_drv->tp[tp_id].video_es_mc_end_address   = 0;
        }else if(mem_type == MEMORY_PRISON_AUDIO_ES){
            pTp_drv->tp[tp_id].audio_es_mc_start_address = 0;
            pTp_drv->tp[tp_id].audio_es_mc_end_address   = 0;
        }
        optee_tp_deinit();
        _tp_ecp_unlock();

        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_TP_DisableMemProtect);


INT32 RHAL_TP_IsJapan4K(void)
{
        if(pTp_drv->tp_japan[TP_TP0].isJAPAN4K == 1)
                return 1;
        else
                return 0;
}

EXPORT_SYMBOL(RHAL_TP_IsJapan4K);

INT32 RHAL_TP_IsATSC30(void)
{
        if(pTp_drv->tp_atsc[TP_TP0].isATSC30 == 1)
                return 1;
        else
                return 0;
}

EXPORT_SYMBOL(RHAL_TP_IsATSC30);

INT32 RHAL_setGetTPPcr90KCnt(UINT32 pcr_type,
                UINT32 rOrw_flag, UINT64 *cnt)
{
        UINT32 ptrlo = 0;
        UINT32 ptrhi = 0;

        _tp_lock();

        if(pcr_type > 1)
        {
                TP_ERROR("%s pcr_type [%d] Not Support!!", __func__, pcr_type);
                return TPK_FAIL;
        }

        if(rOrw_flag == 0) //READ
        {
                Tp_Get_PCR(pcr_type, &ptrhi, &ptrlo);
                *cnt = (((UINT64)ptrhi) << 32) | ptrlo;
        }
        else if(rOrw_flag == 1) //WRITE
        {
                ptrlo = *cnt & 0xFFFFFFFF;
                ptrhi = *cnt  >> 32;
                Tp_Set_PCR(pcr_type, ptrhi, ptrlo);
        }
        else
        {
                TP_ERROR("%s rOrw_flag [%d] Not Support!!", __func__, rOrw_flag);
                return TPK_FAIL;
        }

        _tp_unlock();
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_setGetTPPcr90KCnt);

INT32 RHAL_adjustStcClock(UINT32 stc_type,
                UINT32 fcode, UINT32 ncode)
{
        _tp_lock();

        if(stc_type > 1)
        {
                TP_ERROR("%s stc_type [%d] Not Support!!", __func__, stc_type);
                return TPK_FAIL;
        }

        Tp_atsc30_adj_f_n_code(stc_type, ncode, fcode);

        _tp_unlock();
        return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_adjustStcClock);

#ifdef CONFIG_RTK_TP_WITH_MIO
/*MIO TP*/
INT32 RHAL_MIO_SetIOMode(UINT32 mode)
{
    TPK_MIO_T2P_PARAM_T T2P_param;
    TPK_MIO_P2T_PARAM_T P2T_param;

    _tp_lock();
    TP_MIO_Get_T2PMode(&T2P_param);
    T2P_param.T2P_ctrl_param.mode = mode;
    TP_MIO_Set_T2PMode(T2P_param);

    TP_MIO_Get_P2TMode(&P2T_param);
    P2T_param.P2T_ctrl_param.mode = mode;
    TP_MIO_Set_P2TMode_(P2T_param);
    _tp_unlock();
    TP_WARNING("RHAL_MIO_TP_IOMode_Set mode=%d (0x0:2bit 0x1:serial mode)\n", mode);

    return TPK_SUCCESS;
}
EXPORT_SYMBOL(RHAL_MIO_SetIOMode);
#endif

