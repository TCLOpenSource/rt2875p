/******************************************************************************
*
*   Copyright(c) 2016 Realtek Semiconductor Corp. All rights reserved.
*
*   @author realtek.com
*
******************************************************************************/
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <linux/device.h>
#include <rtk_kdriver/io.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/md_reg.h>
#include <rbus/pll27x_reg_reg.h>
#include <rbus/pll_reg_reg.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/freezer.h>
#include <asm/cacheflush.h>
#include <rtk_kdriver/rtk_crt.h>
#include "rtk_md_debug.h"
#include <rtk_kdriver/md/rtk_md.h>
#include "rtk_md_hw.h"
#ifdef CONFIG_RTK_MD_USE_SW
#include "rtk_md_reg.h"
#else
#ifdef MD_MD_SMQ_CNTL
#include "rtk_md_reg.h"
#endif
#endif

extern bool md_dbg_onoff;
extern struct md_dev *md_drv_dev;
#ifdef CONFIG_RTK_MD_USE_SW
#undef MD_RECOVERY_VCPU
#undef MD_RECOVERY_KCPU
#else
#define MD_RECOVERY_VCPU
//#define MD_RECOVERY_KCPU
#undef MD_RECOVERY_KCPU
#endif
/*****************************VCPU-MD and KCPU-MD***********************************/

#if defined(MD_RECOVERY_VCPU)
static unsigned int md_v_store = 0;
static unsigned int md_v_ctl = 0;
static unsigned int md_v_ine = 0;
static unsigned int md_v_base = 0;
static unsigned int md_v_limit = 0;
static unsigned int md_v_rp = 0;
static unsigned int md_v_wp = 0;
static unsigned int md_v_icnt = 0;


static void rtk_md_init_v_reg(void)
{
    if(md_v_store == 0)
        return;

    if(md_v_base)
        rtd_outl(MD_VMQBASE_reg, md_v_base);
    if(md_v_limit)
        rtd_outl(MD_VMQLIMIT_reg, md_v_limit);
    if(md_v_rp)
        rtd_outl(MD_VMQRDPTR_reg, md_v_rp);
    if(md_v_wp)
        rtd_outl(MD_VMQWRPTR_reg, md_v_wp);

    if(md_v_icnt)
        rtd_outl(MD_VMQ_INSTCNT_reg, md_v_icnt);


    if(md_v_ine) {
        rtd_outl(MD_VMQ_INT_STATUS_reg, 0x1);
        rtd_outl(MD_VMQ_INT_ENABLE_reg, md_v_ine | 0x1);
    }

    if(md_v_base && md_v_limit && md_v_rp && md_v_wp)
        rtd_outl(MD_VMQ_CNTL_reg, md_v_ctl | 0x3);

    md_v_store = 0;
}

static void rtk_md_store_v_reg(void)
{
    if(rtd_inl(MD_CLK_ENABLE_REG) & MD_CLK_ENABLE_MASK) {
        md_v_ctl = rtd_inl(MD_VMQ_CNTL_reg);
        md_v_ine = rtd_inl(MD_VMQ_INT_ENABLE_reg);
        md_v_base = rtd_inl(MD_VMQBASE_reg);
        md_v_limit = rtd_inl(MD_VMQLIMIT_reg);
        md_v_rp = rtd_inl(MD_VMQRDPTR_reg);
        md_v_wp = rtd_inl(MD_VMQWRPTR_reg);
        md_v_icnt = rtd_inl(MD_VMQ_INSTCNT_reg);
        md_v_store = 1;
    }
}

#endif

#if defined(MD_RECOVERY_KCPU)
static unsigned int md_k_ctl = 0;
static unsigned int md_k_ins = 0;
static unsigned int md_k_ine = 0;
static unsigned int md_k_base = 0;
static unsigned int md_k_limit = 0;
static unsigned int md_k_rp = 0;
static unsigned int md_k_wp = 0;

static void rtk_md_init_k_reg(void)
{
    if(md_k_base)
        rtd_outl(MD_KMQBASE_reg, md_k_base);
    if(md_k_limit)
        rtd_outl(MD_KMQLIMIT_reg, md_k_limit);
    if(md_k_rp)
        rtd_outl(MD_KMQRDPTR_reg, md_k_rp);
    if(md_k_wp)
        rtd_outl(MD_KMQWRPTR_reg, md_k_wp);
    if(md_k_ine) {
        rtd_outl(MD_KMQ_INT_STATUS_reg, 0x1);
        rtd_outl(MD_KMQ_INT_ENABLE_reg, md_k_ine | 0x1);
    }

    if(md_k_base && md_k_limit && md_k_rp && md_k_wp)
        rtd_outl(MD_KMQ_CNTL_reg, md_k_ctl | 0x3);
}

static void rtk_md_store_k_reg(void)
{
    if(rtd_inl(MD_CLK_ENABLE_REG)&MD_CLK_ENABLE_MASK) {
        md_k_ctl = rtd_inl(MD_KMQ_CNTL_reg);
        md_k_ine = rtd_inl(MD_KMQ_INT_ENABLE_reg);
        md_k_base = rtd_inl(MD_KMQBASE_reg);
        md_k_limit = rtd_inl(MD_KMQLIMIT_reg);
        md_k_rp = rtd_inl(MD_KMQRDPTR_reg);
        md_k_wp = rtd_inl(MD_KMQWRPTR_reg);
    }
}

#endif

void rtk_md_init_kv_reg(void)
{
#if defined(MD_RECOVERY_VCPU)
    rtk_md_init_v_reg();
#endif

#if defined(MD_RECOVERY_KCPU)
    rtk_md_init_k_reg();
#endif
}

void rtk_md_store_kv_reg(void)
{
#if defined(MD_RECOVERY_VCPU)
    rtk_md_store_v_reg();
#endif

#if defined(MD_RECOVERY_KCPU)
    rtk_md_store_k_reg();
#endif

}

/*****************************HW Debug Interface functions***********************************/
#ifndef CONFIG_RTK_MD_USE_SW
typedef struct {
    uint32_t data[4];
} MD_CMD;

void rtk_md_dump_reg(char *buf)
{
    uint32_t* rp = NULL;
    if(buf == NULL) {

        PDEERR( " ======================================== \n");
        PDEERR( "MdCtrl         = %08x\n", (unsigned int)rtd_inl(MD_SMQ_CNTL_reg));
        PDEERR( "MdInts         = %08x\n", (unsigned int)rtd_inl(MD_SMQ_INT_STATUS_reg));
        PDEERR( "MdInte         = %08x\n", (unsigned int)rtd_inl(MD_SMQ_INT_ENABLE_reg));
        PDEERR( "MdCmdBase      = %08x\n", (unsigned int)rtd_inl(MD_SMQBASE_reg));
        PDEERR( "MdCmdLimit     = %08x\n", (unsigned int)rtd_inl(MD_SMQLIMIT_reg));
        PDEERR( "MdCmdReadPtr   = %08x\n", (unsigned int)rtd_inl(MD_SMQRDPTR_reg));
        PDEERR( "MdCmdWritePtr  = %08x\n", (unsigned int)rtd_inl(MD_SMQWRPTR_reg));
        PDEERR( "MdInsRemainder = %08x\n", (unsigned int)rtd_inl(MD_SMQFIFOSTATE_reg));
        PDEERR( "MdInstCnt      = %08x\n", (unsigned int)rtd_inl(MD_SMQ_INSTCNT_reg));
        if(rtd_inl(MD_SMQRDPTR_reg)) {
            /*This is a case for GDMA resume flow.K2L/p only. each cmd from GDMA is 4 words*/
            rp =  (uint32_t*)phys_to_virt((unsigned long)rtd_inl(MD_SMQRDPTR_reg));
            PDEERR( "CMD            = (%08x, %08x, %08x, %08x)\n", *(rp - 8), *(rp - 7), *(rp - 6), *(rp - 5));
            PDEERR( "CMD            = (%08x, %08x, %08x, %08x)\n", *(rp - 4), *(rp - 3), *(rp - 2), *(rp - 1));
            PDEERR( "CMD            = (%08x, %08x, %08x, %08x)\n", *(rp), *(rp + 1), *(rp + 2), *(rp + 3));
        }
        PDEERR( " ======================================== \n");
    }else {
        buf[0] = 0;
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), " ======================================== \n");
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "MdCtrl         = %08x\n", (unsigned int)rtd_inl(MD_SMQ_CNTL_reg));
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "MdInts         = %08x\n", (unsigned int)rtd_inl(MD_SMQ_INT_STATUS_reg));

        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "MdInte         = %08x\n", (unsigned int)rtd_inl(MD_SMQ_INT_ENABLE_reg));

        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "MdCmdBase      = %08x\n", (unsigned int)rtd_inl(MD_SMQBASE_reg));

        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "MdCmdLimit     = %08x\n", (unsigned int)rtd_inl(MD_SMQLIMIT_reg));

        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "MdCmdReadPtr   = %08x\n", (unsigned int)rtd_inl(MD_SMQRDPTR_reg));

        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "MdCmdWritePtr  = %08x\n", (unsigned int)rtd_inl(MD_SMQWRPTR_reg));

        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "MdInsRemainder = %08x\n", (unsigned int)rtd_inl(MD_SMQFIFOSTATE_reg));

        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "MdInstCnt      = %08x\n", (unsigned int)rtd_inl(MD_SMQ_INSTCNT_reg));
        if(rtd_inl(MD_SMQRDPTR_reg)) {
            /*This is a case for GDMA resume flow.K2L/p only. each cmd from GDMA is 4 words*/
            rp =  (uint32_t*)phys_to_virt((unsigned long)rtd_inl(MD_SMQRDPTR_reg));
            snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "CMD            = (%08x, %08x, %08x, %08x)\n", *(rp - 8), *(rp - 7), *(rp - 6), *(rp - 5));
            snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "CMD            = (%08x, %08x, %08x, %08x)\n", *(rp - 4), *(rp - 3), *(rp - 2), *(rp - 1));
            snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "CMD            = (%08x, %08x, %08x, %08x)\n", *(rp), *(rp + 1), *(rp + 2), *(rp + 3));
        }
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), " ======================================== \n");
    }

}

void rtk_md_dump_last_hw_status(void)
{
    if(md_dbg_onoff) {
        rtk_md_dump_reg(NULL);
    }
}

void md_dump_cmd_ring(struct md_dev *pdev, unsigned int count, char *buf)
{
    MD_CMD* p_md_cmd = (MD_CMD*) pdev->CmdBuf;
    int cmd_ring_size = MD_CMD_BUF / sizeof(MD_CMD);
    int cmd_count = pdev->cmd_cnt / sizeof(MD_CMD);
    int i;

    if (count >= cmd_ring_size)
        count = cmd_ring_size;

    i = (cmd_count > count) ? cmd_count - count : 0;

    if(buf == NULL) {
        while (i < cmd_count) {
            int idx = i % cmd_ring_size;

            PDEWAR("[MD][WARN] CMD=%d, DESC[%d](%p/%08lx)={op:%08x, daddr:%08x, saddr:%08x, len=%08x}\n",
                   i,
                   idx,
                   &p_md_cmd[idx],                               // Virtual Address
                   ((unsigned long)pdev->CmdBase) + (idx * sizeof(MD_CMD)),         // Physical Address
                   p_md_cmd[idx].data[0],
                   p_md_cmd[idx].data[1],
                   p_md_cmd[idx].data[2],
                   p_md_cmd[idx].data[3]);
            i++;
        }
    } else {
        while (i < cmd_count) {
            int idx = i % cmd_ring_size;
            snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf),"[MD][WARN] CMD=%d, DESC[%d](%p/%08lx)={op:%08x, saddr:%08x, daddr:%08x, len=%08x}\n",
                   i,
                   idx,
                   &p_md_cmd[idx],                               // Virtual Address
                   ((unsigned long)pdev->CmdBase) + (idx * sizeof(MD_CMD)),         // Physical Address
                   p_md_cmd[idx].data[0],
                   p_md_cmd[idx].data[1],
                   p_md_cmd[idx].data[2],
                   p_md_cmd[idx].data[3]);
            i++;
        }
    }
}

void md_dump_cmd_ring_status(void)
{
    md_dump_cmd_ring(md_drv_dev, 30, NULL);   // dump latest 30 descriptors
}


void md_dump_last_cmd(void)
{
    if(md_dbg_onoff) {
        md_dump_cmd_ring(md_drv_dev, 1, NULL); // dump the latest  descriptor
    }
}


static int rtk_md_write_cmd(struct md_dev *pdev, uint8_t *pbyCommandBuffer,
                    uint32_t lCommandLength, int32_t go)
{
    uint32_t        dwDataCounter = 0;
    uint8_t         *pbyWritePointer = NULL;
    uint8_t         *pWptr = NULL;
    uint8_t         *pWptrPhy = NULL;
    uint8_t         *pWptrLimit = NULL;
    uint32_t        u32_err_cnt = 0;

    pWptrLimit      = (uint8_t *)pdev->CmdLimit;
    pbyWritePointer = (uint8_t *)((unsigned long)rtd_inl(MD_SMQWRPTR_reg));
    pWptr           = (uint8_t *)pdev->CmdBuf + pdev->wrptr;
    pWptrPhy        = (uint8_t *)pdev->CmdBase + pdev->wrptr;

    if(lCommandLength == 0) {
        if(go) {
            rtd_outl(MD_SMQ_INT_STATUS_reg, MdIntCommandEmpty | MdWriteData);   /*clear com_empty interrupt status */
            rtd_outl(MD_SMQ_CNTL_reg, MdGo | MdEndianSwap | MdWriteData);
        }
        rtk_md_update_inst_count(pdev);
        PDEERR( "[%s][MD] get 0 cmd from caller.\n", __func__);
        goto WRITEFAIL;
    }

    if ((pdev->wrptr + lCommandLength) >= pdev->size) {
        while(1) {
            unsigned long rp = (unsigned long)rtd_inl(MD_SMQRDPTR_reg);
            if((unsigned long)pbyWritePointer == rp) {
                break;
            }
            udelay(1);
            u32_err_cnt++;
            /*add 200ms timeout*/
            if(u32_err_cnt > 200000) {
                PDEERR( "[%s][MD] checking available cmd queue time out, w=%lx, r=%lx, pwptr=%08x, cmdl=%d, psize=%08x\n",
                        __func__, (unsigned long)pbyWritePointer, rp, (uint32_t)pdev->wrptr, lCommandLength, (uint32_t)pdev->size);
                goto WRITEFAIL;
            }
        }
    }
    u32_err_cnt = 0;
    while(1) {
        uint8_t *pbyReadPointer = (uint8_t *)((unsigned long)rtd_inl(MD_SMQRDPTR_reg));
        if(pbyReadPointer <= pbyWritePointer) {
            pbyReadPointer += pdev->size;
        }

        if((pbyWritePointer + lCommandLength) < pbyReadPointer) {
            break;
        }
        udelay(1);
        u32_err_cnt++;
        /*add 200ms timeout*/
        if(u32_err_cnt > 200000) {
            PDEERR( "[%s][MD] getting available cmd queue time out, w=%lx, r=%lx, cmdl=%d\n",
                    __func__, (unsigned long)pbyWritePointer, (unsigned long)pbyReadPointer, lCommandLength);

            goto WRITEFAIL;
        }
    }

    DSB;
    for(dwDataCounter = 0; dwDataCounter < lCommandLength; dwDataCounter += sizeof(uint32_t)) {
        *((uint32_t *)(pWptr)) = (*(uint32_t *)(pbyCommandBuffer + dwDataCounter));

        pWptr += sizeof(uint32_t);
        pWptrPhy += sizeof(uint32_t);
        if((unsigned long)pWptrPhy >= (unsigned long)pWptrLimit) {
            pWptr = pWptr - pdev->size;
            pWptrPhy = pWptrPhy - pdev->size;
        }
    }
    pdev->u64IssuedInstCnt += lCommandLength / MD_ONE_CMD_LEN;

    DSB;

    pdev->wrptr += lCommandLength;
    pdev->cmd_cnt += lCommandLength;
    if (pdev->wrptr >= pdev->size)
        pdev->wrptr -= pdev->size;

    if(go) {
        /*toggle.... */
        rtd_outl(MD_SMQWRPTR_reg, (unsigned long)pWptrPhy);
        /* for merlin5 multi-slave feature */
        DSB;
        rtd_outl(MD_SMQ_CNTL_reg, MdGo | MdEndianSwap | MdWriteData);

        rtk_md_update_inst_count(pdev);
    } else {
        pdev->CmdWptr = (void *)pWptrPhy;
    }

    return MD_SUCCESS;
WRITEFAIL:
    return MD_ERR_WRITE_FAIL;
}


int32_t rtk_write_cmd_wrapper(struct md_dev *dev,
                                uint8_t *pbyCommandBuffer, uint32_t lCommandLength)
{
    int32_t wret = -1;
    int32_t timeoutcnt = 0;
    int32_t errcnt = 3;
    int32_t icnt = 3;
    uint32_t ori_rptr = 0;
    /*ToDo : add INT status conidtion.
     *      if err,then print cmd*/
    if(!rtk_md_is_clk_on())
       return -ENXIO;
    do {

        ori_rptr = rtd_inl(MD_SMQRDPTR_reg);
        md_dump_last_cmd();   // just for debug
        /*must be enable go bit!*/
        wret = rtk_md_write_cmd(dev, pbyCommandBuffer, lCommandLength, 1);
        rtk_md_dump_last_hw_status();   // just for debug
        if(wret == MD_ERR_NO_SEM) {
            msleep(1);
            /*need this? or break*/
            timeoutcnt++;
        } else if(wret == MD_ERR_WRITE_FAIL) {
            msleep(1);
            timeoutcnt += 3;
        } else {
            /*if md is busy, should we need more delay?*/
            for(icnt = 0; icnt < errcnt; icnt++) {
                udelay(50);
                if(ori_rptr == rtd_inl(MD_SMQRDPTR_reg)) {
                    timeoutcnt += 10;
                } else {
                    goto WSUCCESS;
                }
            }
        }

        if(timeoutcnt >= 10) {
            PDEERR("%s timeout\n", __FUNCTION__);
            //md_reset();
            errcnt--;
            timeoutcnt = 0;
        }
    } while(wret != 0 || errcnt < 0);
WSUCCESS:
    return wret;
}
#endif

int rtk_md_memfill(uint32_t addrdst, int32_t len,
            uint32_t bytemode, uint32_t movemode, uint32_t val)
{
#ifdef CONFIG_RTK_MD_USE_SW
    void *dst = rtk_md_phys_to_virt(addrdst);
    if(!dst)
		return -EINVAL;
    if(len <= 0)
        return 0;
    memset(dst, val & 0xFF, len);
    dmac_flush_range(dst, dst + len);
    md_drv_dev->u64IssuedInstCnt++;
    return 0;
#else
    uint8_t const_data = (val & 0xff);
    uint32_t len_tmp = 0;
    int32_t timeout = 0;
    if(len <= 0)
        return 0;
    if(!rtk_md_is_clk_on())
        return -ENXIO;
    do {
        rtd_outl(MD_REG_CMD1_reg, (movemode | bytemode | MD_CONST));
        rtd_outl(MD_REG_CMD2_reg, addrdst);
        rtd_outl(MD_REG_CMD5_reg, const_data | (const_data << 8) |
                            (const_data << 16) | (const_data << 24));
        timeout = 0;
        len_tmp = (len > MD_SEQ_LEN_LIMIT) ? MD_SEQ_LEN_LIMIT : len;

        rtd_outl(MD_REG_CMD4_reg, len_tmp & MD_SEQ_LEN_LIMIT_MASK);

        rtd_outl(MD_REG_CTRL_reg, 0x1);
        rtd_outl(MD_REG_MODE_reg, 0x1);

        while(rtd_inl(MD_REG_MODE_reg) & 0x1) {
            msleep(1);
            timeout++;
            if(timeout > 100) {
                PDEERR("[%s]timeout,size=%d", __func__, timeout);
                rtd_outl(MD_REG_MODE_reg, 0);
                return -1;
            }
        }
        len -= len_tmp;
        addrdst += len_tmp;
    } while(len);
    rtd_outl(MD_REG_MODE_reg, 0);

    return 0;
#endif	
}

#ifndef CONFIG_RTK_MD_USE_SW
bool rtk_md_check_hw_run_state(void)
{
     bool ret = false;
     if(!rtk_md_is_clk_on())
        return false;
     if(rtd_inl(MD_SMQ_CNTL_reg) & MdGo)
        ret = true;
     PDEBUG("\nrtk_md_check_hw_run_state, %08x\n", rtd_inl(MD_SMQ_CNTL_reg));
     return ret;
}


void rtk_md_clear_empty_int_status(void)
{
     rtd_outl(MD_SMQ_INT_STATUS_reg, MdIntCommandEmpty | MdWriteData);
}


bool rtk_md_check_empty_event(void)
{
     if((rtd_inl(MD_SMQ_INT_ENABLE_reg) & MdIntCommandEmpty)
            && (rtd_inl(MD_SMQ_INT_STATUS_reg) & MdIntCommandEmpty))
        return true;
     return false;
}

uint16_t rtk_md_get_hw_instcnt(void)
{
     return (rtd_inl(MD_SMQ_INSTCNT_reg) & MD_SMQ_INSTCNT_smq_inst_cnt_mask);
}

void rtk_md_update_inst_count(struct md_dev *pdev)
{
    uint16_t u16CmdExecuted;
    uint16_t u16InsCntLow;
    
    if(!rtk_md_is_clk_on()) {
        pdev->u64InstCnt = pdev->u64IssuedInstCnt;
        return;
    }
    
    u16CmdExecuted = rtk_md_get_hw_instcnt();
    u16InsCntLow = pdev->u64InstCnt & 0xFFFFLL;
    if(u16InsCntLow > u16CmdExecuted) {
        pdev->u64InstCnt = (pdev->u64InstCnt & ~0xFFFFLL) + 0x10000LL + u16CmdExecuted;
    } else {
        pdev->u64InstCnt = (pdev->u64InstCnt & ~0xFFFFLL) + u16CmdExecuted;
    }
}



void rtk_md_on_off_empty_int(uint32_t on_off)
{
     if(on_off == 1)
        rtd_outl(MD_SMQ_INT_ENABLE_reg, MdIntCommandEmpty | MdIntSCPUInt | MdWriteData);
    else
        rtd_outl(MD_SMQ_INT_ENABLE_reg, MdIntCommandEmpty | MdIntSCPUInt);
}

void rtk_md_enable_mult_reg(uint32_t on_off)
{
    if(on_off == 1)
        rtd_outl(MD_MULTI_REQ_reg, rtd_inl(MD_MULTI_REQ_reg) | BIT(MD_MULTI_REQ_multi_req_en_shift));  //Multi-req default value = 0x00000021
    else
        rtd_outl(MD_MULTI_REQ_reg, rtd_inl(MD_MULTI_REQ_reg) & (~ BIT(MD_MULTI_REQ_multi_req_en_shift)));
}
#endif

void rtk_md_print_sk_md_status(const char *function)
{
    PDEIFO( "%s kcpu md status(%s), scpu md status(%s)\n", function, GET_KMQ_STATUS() == STOP ? "stop" : "run", GET_DUMMY1_REG(SMQ_STATUS) == STOP ? "stop" : "run");
}

void rtk_md_set_dummy_1_reg(uint32_t event, uint32_t value)
{
    SET_DUMMY1_REG(event, value);
}

void rtk_md_set_dummy_2_reg(uint32_t event, uint32_t value)
{
    SET_DUMMY2_REG(event, value);
}


void rtk_md_init_reg(struct md_dev *pdev, uint32_t set_multi_req)
{
    if(!(rtd_inl(MD_CLK_ENABLE_REG) & MD_CLK_ENABLE_MASK)) {
        CRT_CLK_OnOff(MD, CLK_ON, NULL);
        udelay(10);
    }
#ifndef CONFIG_RTK_MD_USE_SW
    rtd_outl(MD_SMQ_CNTL_reg, MdEndianSwap | MdWriteData);
    rtd_outl(MD_SMQBASE_reg, (unsigned long) pdev->CmdBase);
    rtd_outl(MD_SMQLIMIT_reg, (unsigned long) pdev->CmdLimit);
    rtd_outl(MD_SMQRDPTR_reg, (unsigned long) pdev->CmdBase);
    rtd_outl(MD_SMQWRPTR_reg, (unsigned long) pdev->CmdBase);
    rtd_outl(MD_SMQ_INSTCNT_reg, 0);
#endif
    rtk_md_set_dummy_1_reg(SMQ_STATUS, START);
#ifndef CONFIG_RTK_MD_USE_SW
    rtk_md_init_kv_reg();

    rtk_md_enable_mult_reg(set_multi_req);
#endif

}
#ifndef CONFIG_RTK_MD_USE_SW
void rtk_md_reset_reg(struct md_dev *pdev, uint32_t set_multi_req)
{
    rtd_outl(MD_CLK_SRST_REG, BIT(MD_CLK_SRST_SHIFT));
    rtd_outl(MD_CLK_SRST_REG, BIT(MD_CLK_SRST_SHIFT) | MD_CLK_SRST_WRITE_DATA_MASK);
    udelay(10);
    rtd_outl(MD_SMQ_CNTL_reg, MdEndianSwap | MdWriteData);
    rtd_outl(MD_SMQBASE_reg, (unsigned long) pdev->CmdBase);
    rtd_outl(MD_SMQLIMIT_reg, (unsigned long) pdev->CmdLimit);
    rtd_outl(MD_SMQRDPTR_reg, (unsigned long) (pdev->CmdBase + pdev->wrptr));
    rtd_outl(MD_SMQWRPTR_reg, (unsigned long) (pdev->CmdBase + pdev->wrptr));
    rtd_outl(MD_SMQ_INSTCNT_reg, pdev->u64IssuedInstCnt & 0xFFFF);

    rtk_md_set_dummy_1_reg(SMQ_STATUS, START);
    rtk_md_init_kv_reg();

    rtk_md_enable_mult_reg(set_multi_req);
}
#endif

bool rtk_md_is_clk_on(void)
{
    if(rtd_inl(MD_CLK_ENABLE_REG) & MD_CLK_ENABLE_MASK)
        return true;
    return false;
}


int rtk_md_hw_power_down(void)
{
    int retrycnt = 0;
    PDEIFO("[MD]%s\n", __func__);
#ifdef CONFIG_RTK_MD_USE_SW
    while((rtd_inl(MD_MD_FDMA_CTRL1_reg) & MD_MD_FDMA_CTRL1_fdma_go_mask)) {
        if(retrycnt >= 6000) {
            PDEERR("[MD]wait finish timeout.\n");
            break;
        }

        if ( retrycnt == 0 ) {
            if (rtd_inl(MD_MD_FDMA_CTRL1_reg) & MD_MD_FDMA_CTRL1_fdma_go_mask)
                PDEIFO("fdma is not idle... will retry again.\n");
        }
        retrycnt++;
        udelay(100);
    }
#else
    while(!(rtd_inl(MD_SMQ_CNTL_reg) & MD_SMQ_CNTL_smq_idle_mask)
          || (rtd_inl(MD_MD_FDMA_CTRL1_reg) & MD_MD_FDMA_CTRL1_fdma_go_mask)) {
        if(retrycnt >= 6000) {
            PDEERR("[MD]wait finish timeout.\n");
            break;
        }

        if ( retrycnt == 0 ) {
            if (!(rtd_inl(MD_SMQ_CNTL_reg) & MD_SMQ_CNTL_smq_idle_mask))
                PDEIFO("scpu md is not idle... will retry again.\n");
            if (rtd_inl(MD_MD_FDMA_CTRL1_reg) & MD_MD_FDMA_CTRL1_fdma_go_mask)
                PDEIFO("fdma is not idle... will retry again.\n");
        }
        retrycnt++;
        udelay(100);
    }

    rtk_md_print_sk_md_status(__FUNCTION__);

    rtk_md_store_kv_reg();
#endif
    /* clock disable */
    if (GET_KMQ_STATUS() == STOP) {
        CRT_CLK_OnOff(MD, CLK_OFF, NULL);
    }
    else {
        rtk_md_set_dummy_1_reg(SMQ_STATUS, STOP);
        rtk_md_set_dummy_2_reg(SMQ_SELF_CHECK, RESET);
    }
    return 0;
}
#ifndef CONFIG_RTK_MD_USE_SW
void rtk_md_hw_set_burst_len(int byte)
{
    if (byte < 8)
        byte = 8;
    if (byte > 8 * 1008)
        byte = 8 * 1008;
    rtd_outl(MD_MD_BSTLEN,MD_MD_BSTLEN_max_bstlen(byte/8));
}

int rtk_md_hw_get_burst_len(void)
{
    return 8 * MD_MD_BSTLEN_get_max_bstlen(rtd_inl(MD_MD_BSTLEN));
}
#endif

int rtk_md_hw_fdma_copy(uint32_t flash_address, unsigned char *buf,
              uint32_t len, uint32_t fdma_max_xfer, uint32_t fdma_dir)
{
    // add clock check
    if(!rtk_md_is_clk_on())
        return -ENXIO;
	
    // set DDR and flash addr
    rtd_outl(MD_MD_FDMA_DDR_SADDR_reg, (unsigned int)virt_to_phys(buf));
    rtd_outl(MD_MD_FDMA_FL_SADDR_reg, MD_MD_FDMA_FL_SADDR_fdma_fl_saddr((unsigned int)flash_address));
    rtd_outl(MD_MD_FDMA_CTRL2_reg, MD_MD_FDMA_CTRL2_fdma_length(len) | MD_MD_FDMA_CTRL2_fdma_dir(fdma_dir) | \
        MD_MD_FDMA_CTRL2_fdma_max_xfer(fdma_max_xfer) | MD_MD_FDMA_CTRL2_fdma_swap(0) | \
        MD_MD_FDMA_CTRL2_fl_mapping_mode(1)); // forward map, no_swap, DMA LEN, FLASH to DDR

    dmac_flush_range(buf, buf + len);
    outer_flush_range(virt_to_phys(buf), virt_to_phys(buf + len));

    DSB;
    rtd_outl(MD_MD_FDMA_CTRL1_reg, MD_MD_FDMA_CTRL1_write_en1(1) | MD_MD_FDMA_CTRL1_fdma_go(1));
    while(rtd_inl(MD_MD_FDMA_CTRL1_reg) & MD_MD_FDMA_CTRL1_fdma_go_mask); // wait for MD done its operation
    rtd_outl(MD_MD_SMQ_INT_STATUS_reg, MD_MD_SMQ_INT_STATUS_fdma_done(1));  // clear flash done bit	
    return 0;
    
}

void * rtk_md_phys_to_virt(unsigned long phy_addr)
{
	return phys_to_virt(phy_addr);
}