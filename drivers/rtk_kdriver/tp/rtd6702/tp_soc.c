#include <rtk_kdriver/io.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/freezer.h>
#include <linux/pageremap.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#ifdef CONFIG_LG_SNAPSHOT_BOOT
#include <linux/suspend.h>
#endif
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#include <asm/cacheflush.h>
#else
#include <asm/outercache.h>
#include <asm/cacheflush.h>
#endif
#include <rbus/sys_reg_reg.h>
#include <rbus/timer_reg.h>
#include <rbus/tp_reg.h>
#include <rbus/pll_reg_reg.h>
#include <rbus/efuse_reg.h>
#ifdef CONFIG_RTK_TP_WITH_MIO
#include <rbus/MIO/mio_spi_synchronizer_reg.h>
#endif

#include <base_types.h>
#include <rtk_crt.h>
#include <rtk_cw_util.h>

#include <tp/tp_dbg.h>
#include <tp/tp_drv_global.h>
#include <tp/tp_reg_ctrl.h>
#include <tp/tp_ta.h>

#include <rtd_log/rtd_module_log.h>
#include <rtk_kdriver/rtk_otp_region_api.h>
#include "tp_soc.h"

INT32 Tp_ENG_RawModeEnable( TPK_TP_ENGINE_T tp_id, BOOL enable)
{
        tp_tp_tf0_cntl_RBUS tp_cntl;


        tp_cntl.regValue = 0;
        tp_cntl.dir_dma = 1;
        Tp_SetTpCtrlReg(tp_id, tp_cntl.regValue, enable ? 1 : 0);

        /*clear buf_rdy_ctl & pid_en in raw mode*/

        tp_cntl.regValue = 0;
        tp_cntl.pid_en = 1;
        tp_cntl.buf_rdy_ctl = 1;

        tp_cntl.tp_err_fix_en = 1;
        tp_cntl.null_en = 1;
        tp_cntl.trerr_en = 1;
        tp_cntl.sync_en = 1;
        Tp_SetTpCtrlReg(tp_id, tp_cntl.regValue, enable ? 0 : 1);

        /* set raw buffer idx */
        if(enable){
                Tp_Raw_Buffer_Set(tp_id);
        }
        return TPK_SUCCESS;
}

INT32 Tp_Raw_Buffer_Set(TPK_TP_ENGINE_T tp_id)
{
        tp_tp_raw_ring_buf_idx_RBUS tp_raw_buf_idx;

        CHECK_MAX_TP_COUNT(tp_id);

        /* set raw buffer idx */
        tp_raw_buf_idx.regValue = READ_REG32(TP_TP_RAW_RING_BUF_IDX_reg);	
        switch(tp_id) {
                case TP_TP0:
                        tp_raw_buf_idx.tp0 = TP0_BUFFER_MASS_DATA;
                        break;
                case TP_TP1:
                        tp_raw_buf_idx.tp1 = TP1_BUFFER_MASS_DATA;
                        break;
                case TP_TP2:
                        tp_raw_buf_idx.tp2 = TP2_BUFFER_MASS_DATA;
                        break;
#ifdef TP_SUPPORT_TP3
                case TP_TP3:
                        tp_raw_buf_idx.tp3 = TP3_BUFFER_MASS_DATA;
                        break;
#endif
                default:
                        TP_WARNING("tp_id=%u not support\n", tp_id);
                        return TPK_BAD_PARAMETER;
        }
        WRITE_REG32(TP_TP_RAW_RING_BUF_IDX_reg, tp_raw_buf_idx.regValue);
        return TPK_SUCCESS;
}


INT32 TP_Get_MCM_ATSC3_Exist(void)
{
        int mcm_atsc3 = rtk_otp_field_read_int_by_name("hw_mcm_atsc3");
        if (mcm_atsc3 != -1) {
                if(mcm_atsc3 == 1) {
                        TP_DBG("TP_Get_MCM_ATSC3_Exist ATSC3.0 MCM Exist\n");
                        return 1;
                } else {
                        TP_DBG("TP_Get_MCM_ATSC3_Exist ATSC3.0 MCM Not Exist\n");
                        return 0;
                }
        } else {
                TP_DBG("TP_Get_MCM_ATSC3_Exist OTP not define hw_mcm_atsc3\n");
                return 0;
        }
}
