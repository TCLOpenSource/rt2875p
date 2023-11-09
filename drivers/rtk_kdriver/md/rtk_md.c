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
#include <rbus/md_reg.h>
#include <rbus/pll27x_reg_reg.h>
#include <rbus/pll_reg_reg.h>
#include <linux/kthread.h>
#include <linux/err.h>
#include <linux/freezer.h>
#include <linux/irq.h>
#include <rtk_kdriver/rtk_crt.h>
#include <asm/cacheflush.h>
#include "rtk_md_debug.h"
#include "rtk_md_hw.h"


struct md_dev *md_drv_dev;
extern bool md_dbg_onoff;
static struct completion md_check_thread_exited;
static struct task_struct * g_md_check_task_struct = NULL;
static int g_md_check_thread_exited_flag = 0;
bool g_enable_multi_req = 0;
int g_rtk_md_irq_num = -1;

static void rtk_md_self_checking(bool do_self_check);

static void rtk_md_hw_init(struct md_dev *pdev)
{
    if (down_interruptible(&md_drv_dev->sem))
        return;
    rtk_md_init_reg(pdev, g_enable_multi_req);
    pdev->CmdWptr = 0;
    pdev->wrptr = 0;
    pdev->u64InstCnt = 0;
    pdev->u64IssuedInstCnt = 0;
    
    up(&md_drv_dev->sem);

}

int rtk_md_get_issued_inst_count(uint64_t *p_u64IssuedInstCnt)
{
    if(!p_u64IssuedInstCnt)
        return -1;
    *p_u64IssuedInstCnt = md_drv_dev->u64IssuedInstCnt;
    return MD_SUCCESS;
}

#ifndef CONFIG_RTK_MD_USE_SW
static void rtk_md_hw_reset(void)
{
    if (down_interruptible(&md_drv_dev->sem))
        return;

    rtk_md_dump_reg(NULL);
    rtk_md_store_kv_reg();
    rtk_md_hw_init(md_drv_dev);

    up(&md_drv_dev->sem);
}

static void rtk_md_reset_and_restore(struct md_dev *pdev)
{
    if (down_interruptible(&md_drv_dev->sem))
        return;
    rtk_md_dump_reg(NULL);
    rtk_md_store_kv_reg();
    rtk_md_reset_reg(pdev, g_enable_multi_req);
    rtk_md_dump_reg(NULL);
    up(&md_drv_dev->sem);

}


static irqreturn_t rtk_md_irq_handler(int irq, void *dev_id)
{
    struct md_dev *pdev = (struct md_dev *)dev_id;
    
    if(rtk_md_check_empty_event()) {
        rtk_md_on_off_empty_int(0);
        rtk_md_clear_empty_int_status();
        pdev->CmdDone = 1;

        return IRQ_HANDLED;
    }
    return IRQ_NONE;
}


static int _rtk_md_wait_hw_stopped(struct md_dev *pdev, uint64_t timeout)
{
    /*clear cmd empty interrupt */
    rtk_md_clear_empty_int_status();
    if(rtk_md_check_hw_run_state()) {
        rtk_md_clear_empty_int_status();
        PDEBUG("\nwait for complete before down\n");
        wait_event_interruptible_timeout(pdev->wait_queue,
                                         (pdev->CmdDone == 1), timeout);
        PDEBUG("\nwait for complete after down\n");
        if(pdev->CmdDone == 1) {
            return 0;
        }
        return -1;
    }
    return 0;
}

int rtk_md_wait_hw_stopped(uint64_t timeout)
{
    int retval = 0;
    if (down_interruptible(&md_drv_dev->sem))
        return -ERESTARTSYS;
    retval = _rtk_md_wait_hw_stopped(md_drv_dev, timeout);
    up(&md_drv_dev->sem);
    return retval;
}

int rtk_md_get_inst_count(uint64_t *p_u64InstCnt)
{
    if(!p_u64InstCnt)
        return -1;
    if (down_interruptible(&md_drv_dev->sem))
        return -ERESTARTSYS;
    rtk_md_update_inst_count(md_drv_dev);
    *p_u64InstCnt = md_drv_dev->u64InstCnt;
    up(&md_drv_dev->sem);
    return MD_SUCCESS;
}

static int _rtk_md_memcpy(struct md_dev   *pdev, uint32_t        addrdst,
                      uint32_t addrsrc, int32_t len, uint32_t flags)
{
    uint32_t len_tmp = MD_SEQ_LEN_LIMIT;
    uint32_t dwcmdword[4] = {0};
    int32_t wret = 0;
    if(len <= 0)
        return wret;
    dwcmdword[0] = (MD_SS | MD_1B | ((flags & BIT(0))  ? MD_CONST : 0) |  ((flags & BIT(1))  ? MD_MOVE_DIR : 0));
    do {
        len_tmp = (len > MD_SEQ_LEN_LIMIT) ? MD_SEQ_LEN_LIMIT : len;

        dwcmdword[1] = addrdst;
        dwcmdword[2] = addrsrc;
        dwcmdword[3] = (len_tmp & MD_SEQ_LEN_LIMIT_MASK);

        wret = rtk_write_cmd_wrapper(pdev, (uint8_t *)dwcmdword, sizeof(dwcmdword));
        len -= len_tmp;
        addrdst += len_tmp;
        addrsrc += len_tmp;
    } while(len);
    return wret;
}
#endif

int rtk_md_memcpy(uint32_t dst_phys_addr, uint32_t src_phys_addr,
              uint32_t length, int32_t op, uint32_t is_wait)
{
#ifdef CONFIG_RTK_MD_USE_SW
    void *src = rtk_md_phys_to_virt(src_phys_addr);
    void *dst = rtk_md_phys_to_virt(dst_phys_addr);
    if(!src || !dst)
		return -EINVAL;
    if(length == 0) {
        PDEWAR("md_copy_pages with zero length = %d, ignore it\n", length);
        return 0;
    }
    if(op & BIT(1))
	memmove(dst, src, length);
    else
	memcpy(dst, src, length);
    dmac_flush_range(dst, dst + length);
    md_drv_dev->u64IssuedInstCnt++;
    return 0;
#else
    PDEBUG("%s,%d, dst_phys_addr=0x%08x, src_phys_addr=0x%08x, length=%d\n", __func__, __LINE__, dst_phys_addr, src_phys_addr, length);
    if(length == 0) {
        PDEWAR("md_copy_pages with zero length = %d, ignore it\n", length);
        return 0;
    }
    if (down_interruptible(&md_drv_dev->sem))
        return -ERESTARTSYS;
    md_drv_dev->CmdDone = 0;
    _rtk_md_memcpy(md_drv_dev, dst_phys_addr, src_phys_addr, length, op);
    if(is_wait) {
        /*Enable empty interrupt */
        rtk_md_on_off_empty_int(1);
        wait_event_interruptible_timeout(md_drv_dev->wait_queue, (md_drv_dev->CmdDone == 1), (2 * HZ));
        if(md_drv_dev->CmdDone == 0) {
            PDEBUG("MD : no empty interrupt.\n");
            up(&md_drv_dev->sem);
            return -1;
        }
    }
    up(&md_drv_dev->sem);
    return 0;
#endif	
}

int rtk_md_do_cmds(uint8_t *pbyCommandBuffer, uint32_t lCommandLength)
{
#ifdef CONFIG_RTK_MD_USE_SW
    int i;
    int cmd_cnt;
    if(lCommandLength & (MD_ONE_CMD_LEN - 1))
		return -1;
    cmd_cnt = lCommandLength / MD_ONE_CMD_LEN;
    for(i = 0; i < cmd_cnt; i++) {
		unsigned int *cmd = (unsigned int *)(pbyCommandBuffer + i * MD_ONE_CMD_LEN);
		if(cmd[0] & BIT(6)) {
			if(rtk_md_memfill(cmd[1], cmd[3], 1, 1, cmd[2]) !=0) {
				return -1;
			}
		} else {
			if(rtk_md_memcpy(cmd[1], cmd[2], cmd[3], 
				(cmd[0] & BIT(7)) ? BIT(1) : 0, true) !=0) {
				return -1;
			}
		}
    }
    return 0;
#else
    int retval = 0;
    if(pbyCommandBuffer == NULL || lCommandLength == 0)
        return -1;
    if (down_interruptible(&md_drv_dev->sem)) {
        return -ERESTARTSYS;
    }
    md_drv_dev->CmdDone = 0;
    retval = rtk_write_cmd_wrapper(md_drv_dev, pbyCommandBuffer, lCommandLength);
    up(&md_drv_dev->sem);
    return retval;
#endif	
}

#ifndef CONFIG_RTK_MD_USE_SW
/*****************************HW freeze check and restore*************************************/

static int rtk_md_status_check(void *arg)
{
    uint64_t last_sw_inst_count = 0;
    uint64_t last_hw_inst_count = 0;
    uint64_t check_counter = 0;
    while (!g_md_check_thread_exited_flag) {
        uint64_t cur_hw_inst_count = 0;
        set_freezable();
        if(!rtk_md_is_clk_on()) {
            msleep(10);
            continue;
        }  
        rtk_md_get_inst_count(&cur_hw_inst_count);
        rtk_md_get_issued_inst_count(&last_sw_inst_count);
        if(last_hw_inst_count == cur_hw_inst_count &&
                (last_hw_inst_count != last_sw_inst_count)) {
            check_counter++;
        } else {
            last_hw_inst_count = cur_hw_inst_count;
            check_counter = 0;
        }

        if(check_counter > 2000) {
            if(last_sw_inst_count > last_hw_inst_count) {
                PDEERR("[MD]md seems to freeze now, ready to reset\n");
                rtk_md_reset_and_restore(md_drv_dev);
            } 
            check_counter = 0;
        }
        msleep(10);
    }
    complete_and_exit(&md_check_thread_exited, 1);
}
#endif

__init int rtk_md_init(struct device *md_device, int irq_num)
{
    int ret = 0;
    dma_addr_t dwPhysicalAddress = 0;

    md_drv_dev = kzalloc(sizeof(struct md_dev), GFP_KERNEL);
    if (!md_drv_dev) {
        ret = -ENOMEM;
        goto FAIL_ALLOCATE_MD_DEV;
    }
    sema_init(&md_drv_dev->sem, 1);
    init_waitqueue_head(&md_drv_dev->wait_queue);
    md_drv_dev->size = MD_CMD_BUF;

    md_drv_dev->CmdBuf = dma_alloc_coherent(md_device, md_drv_dev->size, &
                                            dwPhysicalAddress, GFP_ATOMIC);
    if(!md_drv_dev->CmdBuf) {
        ret = -ENOMEM;
        goto FAIL_ALLOCATE_CMD_BUFFER;
    }
    PDEIFO("Command Buffer Address = %lx, Physical Address = %x, Size = %08x\n",
           (unsigned long) md_drv_dev->CmdBuf, (uint32_t)
           dwPhysicalAddress, md_drv_dev->size);
    md_drv_dev->wrptr = 0;
    md_drv_dev->CmdBase = (void *) dwPhysicalAddress;
    md_drv_dev->CmdLimit = (void *) (dwPhysicalAddress + md_drv_dev->size);
    md_drv_dev->md_device = md_device;
#ifndef CONFIG_RTK_MD_USE_SW
    ret = request_irq(irq_num, rtk_md_irq_handler,
                      IRQF_SHARED, "md", (void *)md_drv_dev);
    if(ret < 0) {
        PDEERR("[MD] Request irq failed, ret=%d\n", ret);
        goto FAIL_REQUEST_IRQ;
    }

    rtk_md_store_kv_reg();
    rtk_md_hw_init(md_drv_dev);

    g_md_check_thread_exited_flag = 0;
    g_md_check_task_struct = kthread_create(rtk_md_status_check, NULL, "md_check_thread");
    if(IS_ERR(g_md_check_task_struct)) {
        PDEERR("[MD] Create check thread failed\n");
        ret = -EFAULT;
        goto FAIL_CREATE_KERNEL_THREAD;
    }
    init_completion(&md_check_thread_exited);
    wake_up_process(g_md_check_task_struct);    

    rtk_md_self_checking(true);
#endif
    return 0;
#ifndef CONFIG_RTK_MD_USE_SW    
FAIL_CREATE_KERNEL_THREAD:	
    free_irq(irq_num, (void *)md_drv_dev);
FAIL_REQUEST_IRQ:	
    dma_free_coherent(md_drv_dev->md_device, md_drv_dev->size,
                      (void*)md_drv_dev->CmdBuf, (dma_addr_t)md_drv_dev->CmdBase);
#endif	
FAIL_ALLOCATE_CMD_BUFFER:
    kfree(md_drv_dev);
    md_drv_dev = NULL;
FAIL_ALLOCATE_MD_DEV:
    return ret;
}


void rtk_md_uninit(int irq_num)
{
    if(g_md_check_task_struct) {
        g_md_check_thread_exited_flag = 1;
        wait_for_completion(&md_check_thread_exited);
        g_md_check_task_struct = NULL;
    }
#ifndef CONFIG_RTK_MD_USE_SW
    free_irq(irq_num, (void *)md_drv_dev);
#endif
    dma_free_coherent(md_drv_dev->md_device, md_drv_dev->size, (void*)md_drv_dev->
                      CmdBuf, (dma_addr_t)md_drv_dev->CmdBase);
    kfree(md_drv_dev);
    md_drv_dev = NULL;
}


/**********************************Global APIS for other modules to use*****************************************/
uint64_t smd_memcpy(
    uint32_t        addrDst,
    uint32_t        addrSrc,
    int32_t         len)
{
#ifdef CONFIG_RTK_MD_USE_SW
    rtk_md_memcpy(addrDst, addrSrc, len, 0, 0);
    return md_drv_dev->u64IssuedInstCnt;
#else
    int wret = rtk_md_memcpy(addrDst, addrSrc, len, 0, 0);
    if(wret != MD_SUCCESS) {
        PDEERR( "[MD]%s fail(%d).\n", __func__, wret);
    }
    return md_drv_dev->u64IssuedInstCnt;
#endif	
}
EXPORT_SYMBOL(smd_memcpy);

bool smd_waitComplete(
    uint64_t        handle,
    uint64_t        timeout,
    uint64_t        *hwCnt)
{
#ifdef CONFIG_RTK_MD_USE_SW
   *hwCnt = md_drv_dev->u64IssuedInstCnt;
   return true;
#else
    if (down_interruptible(&md_drv_dev->sem))
        return false;
    /* Enable empty interrupt */
    rtk_md_on_off_empty_int(1);
    
    _rtk_md_wait_hw_stopped(md_drv_dev, timeout);
    if(md_drv_dev->CmdDone == 0) {
        PDEALT("MD : no empty interrupt.\n");
        up(&md_drv_dev->sem);
        return false;
    }
    rtk_md_update_inst_count(md_drv_dev);
    hwCnt = (uint64_t*) &md_drv_dev->u64InstCnt;
    up(&md_drv_dev->sem);
    return true;
#endif	
}
EXPORT_SYMBOL(smd_waitComplete);

bool smd_checkFinish(
    uint64_t        handle)
{
#ifdef CONFIG_RTK_MD_USE_SW
    return true;
#else
    uint64_t sw_counter = (uint64_t)handle;
    if(down_timeout(&md_drv_dev->sem, HZ / 10)) {
        PDEERR( "[MD]%s down_tiemput!\n", __func__);
        return false;
    }
    rtk_md_update_inst_count(md_drv_dev);
    up(&md_drv_dev->sem);

    if(md_drv_dev->u64InstCnt >= sw_counter) {
        return true;
    }
    return false;
#endif	
}
EXPORT_SYMBOL(smd_checkFinish);


void smd_checkComplete(void)
{
#ifdef CONFIG_RTK_MD_USE_SW
    return ;
#else
    uint32_t u32_err_cnt = 0;
    while(1) {
        if(smd_checkFinish(md_drv_dev->u64IssuedInstCnt))
            break;
        udelay(200);
        u32_err_cnt++;
        /*add 200ms timeout*/
        if(u32_err_cnt > 1000) {
            PDEERR("%s timeout\n", __FUNCTION__);
            rtk_md_hw_reset();
            break;
        }
    }
#endif	
}
EXPORT_SYMBOL(smd_checkComplete);

int smd_memfill(uint32_t addrdst, int32_t len, uint32_t val)
{
#ifdef CONFIG_RTK_MD_USE_SW
    return rtk_md_memfill(addrdst, len, MD_1B, MD_SS, val);
#else
    int ret = -1;
    if(down_interruptible(&md_drv_dev->sem))
        return -1;    
    ret = rtk_md_memfill(addrdst, len, MD_1B, MD_SS, val);
    up(&md_drv_dev->sem);
    return ret;
#endif	
}
EXPORT_SYMBOL(smd_memfill);

int smd_fdma_copy(uint32_t flash_address, unsigned char *buf,
              uint32_t len, uint32_t fdma_max_xfer, uint32_t fdma_dir)
{
    int ret = 0;
    
    if (buf == NULL)
        return -EFAULT;
	
    if (down_interruptible(&md_drv_dev->sem))
        return -EBUSY;
    ret = rtk_md_hw_fdma_copy(flash_address, buf, len, fdma_max_xfer, fdma_dir);
    up(&md_drv_dev->sem);
    return ret;
}
EXPORT_SYMBOL(smd_fdma_copy);

static void rtk_md_self_checking(bool do_self_check)
{
#ifndef CONFIG_RTK_MD_USE_SW
    if(do_self_check) {
        /*self-checking*/ 
        int retry_cnt = 0;
        for(retry_cnt = 0; retry_cnt <= 2; retry_cnt++) {
            smd_memcpy((unsigned long)md_drv_dev->CmdBase + 0x1000,
                       (unsigned long)(md_drv_dev->CmdBase + 0x2000), 0x40);
            smd_checkComplete();
            if(rtk_md_get_hw_instcnt() >= 1) {
                PDEIFO("[MD]%s success.instcnt=%d\n", __func__, rtk_md_get_hw_instcnt());
                break;
            }
        }
    }
#endif	
    rtk_md_set_dummy_2_reg(SMQ_SELF_CHECK, FINISH);
    if(do_self_check)
        rtk_md_print_sk_md_status(__FUNCTION__);
}



/*****************************MD driver STR and STD functions*************************************/

#ifdef CONFIG_PM
int rtk_md_suspend_noirq(void)
{
    PDEIFO("[MD]%s\n", __func__);
    rtk_md_hw_power_down();
    PDEIFO("[MD]%s success.\n", __func__);
    return 0;
}

int rtk_md_resume_noirq(void)
{
    PDEIFO("[MD]%s\n", __func__);
    rtk_md_hw_init(md_drv_dev);
    return 0;
}

int rtk_md_resume_early(void)
{
    PDEIFO("[MD]%s\n", __func__);
    rtk_md_self_checking(true);
    return 0;
}

int rtk_md_restore(void)
{
    PDEIFO("[MD]%s\n", __func__);
    rtk_md_hw_init(md_drv_dev);
    rtk_md_self_checking(true);
    return 0;
}

int rtk_md_freeze_noirq(void)
{
    PDEIFO("[MD]%s\n", __func__);
    rtk_md_self_checking(false);
    return 0;
}

int rtk_md_poweroff(void)
{
    PDEIFO("[MD]%s\n", __func__);
    rtk_md_hw_power_down();
    PDEIFO("[MD]%s success.\n", __func__);
    return 0;
}

#endif

