#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/console.h>
#include <linux/printk.h>
#include <linux/version.h>
#include <linux/suspend.h>

#include "rtk_avcpu.h"
#include <rtk_kdriver/rtk_avcpu_export.h>
#include <rtk_kdriver/RPCDriver.h>
#include <rtk_kdriver/io.h>
#include <rbus/stb_reg.h>

#include <base_types.h>
#include <linux/delay.h>
#include <rtd_log/rtd_module_log.h>

#define AVCPU_SUSPEND_RESUME_TIMEOUT_SEC 2

#if IS_ENABLED(CONFIG_RTK_KDRV_SERIAL_8250)
extern void rtk_serial8250_emergency_resume(void);
#endif

extern void rtk_inv_range(const void *virt_start, const void *virt_end);
static int invalidate_status_flag(void)
{
#if 0
	dmac_inv_range(phys_to_virt(ACPU_STATUS_FLAG_ADDRESS),phys_to_virt(ACPU_STATUS_FLAG_ADDRESS+0x4));
	outer_inv_range(ACPU_STATUS_FLAG_ADDRESS, ACPU_STATUS_FLAG_ADDRESS+0x4);

#ifdef CONFIG_RTK_KDRV_SUPPORT_ACPU2
	dmac_inv_range(phys_to_virt(ACPU2_STATUS_FLAG_ADDRESS),phys_to_virt(ACPU2_STATUS_FLAG_ADDRESS+0x4));
	outer_inv_range(ACPU2_STATUS_FLAG_ADDRESS, ACPU2_STATUS_FLAG_ADDRESS+0x4);
#endif

	dmac_inv_range(phys_to_virt(VCPU_STATUS_FLAG_ADDRESS),phys_to_virt(VCPU_STATUS_FLAG_ADDRESS+0x4));
	outer_inv_range(VCPU_STATUS_FLAG_ADDRESS, VCPU_STATUS_FLAG_ADDRESS+0x4);

#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	dmac_inv_range(phys_to_virt(VCPU2_STATUS_FLAG_ADDRESS),phys_to_virt(VCPU2_STATUS_FLAG_ADDRESS+0x4));
	outer_inv_range(VCPU2_STATUS_FLAG_ADDRESS, VCPU2_STATUS_FLAG_ADDRESS+0x4);
#endif
#endif
	rtk_inv_range(phys_to_virt(ACPU_STATUS_FLAG_ADDRESS),phys_to_virt(ACPU_STATUS_FLAG_ADDRESS+0x4));
	rtk_inv_range(phys_to_virt(VCPU_STATUS_FLAG_ADDRESS),phys_to_virt(VCPU_STATUS_FLAG_ADDRESS+0x4));
#ifdef CONFIG_RTK_KDRV_SUPPORT_ACPU2
	rtk_inv_range(phys_to_virt(ACPU2_STATUS_FLAG_ADDRESS),phys_to_virt(ACPU2_STATUS_FLAG_ADDRESS+0x4));
#endif
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	rtk_inv_range(phys_to_virt(VCPU2_STATUS_FLAG_ADDRESS),phys_to_virt(VCPU2_STATUS_FLAG_ADDRESS+0x4));
#endif

	return 0;
}
static int flush_status_flag(void)
{
	dmac_flush_range(phys_to_virt(ACPU_STATUS_FLAG_ADDRESS),phys_to_virt(ACPU_STATUS_FLAG_ADDRESS+0x4));
	outer_flush_range(ACPU_STATUS_FLAG_ADDRESS, ACPU_STATUS_FLAG_ADDRESS+0x4);

#ifdef CONFIG_RTK_KDRV_SUPPORT_ACPU2
	dmac_flush_range(phys_to_virt(ACPU2_STATUS_FLAG_ADDRESS),phys_to_virt(ACPU2_STATUS_FLAG_ADDRESS+0x4));
	outer_flush_range(ACPU2_STATUS_FLAG_ADDRESS, ACPU2_STATUS_FLAG_ADDRESS+0x4);
#endif

	dmac_flush_range(phys_to_virt(VCPU_STATUS_FLAG_ADDRESS),phys_to_virt(VCPU_STATUS_FLAG_ADDRESS+0x4));
	outer_flush_range(VCPU_STATUS_FLAG_ADDRESS, VCPU_STATUS_FLAG_ADDRESS+0x4);

#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	dmac_flush_range(phys_to_virt(VCPU2_STATUS_FLAG_ADDRESS),phys_to_virt(VCPU2_STATUS_FLAG_ADDRESS+0x4));
	outer_flush_range(VCPU2_STATUS_FLAG_ADDRESS, VCPU2_STATUS_FLAG_ADDRESS+0x4);
#endif

	return 0;
}

void force_console_enable(void)
{
	console_loglevel = 7;
#if IS_ENABLED(CONFIG_RTK_KDRV_SERIAL_8250)
	rtk_serial8250_emergency_resume();
#endif
#if 0//CONFIG_RTK_KDRV_AVCPU /* not export on native linux code*/
	resume_console();
#endif
}

static int avcpu_notify_suspend(void)
{
	int ret;
	unsigned long context;
	unsigned long timeout;
	int wait_flag = 0, avcpu_flag;


	/* Clear AV CPU resume status*/
	*((volatile int *)phys_to_virt(AVCPU_STATUS_FLAG_STD_ADDR)) = 0x0;
	dmac_flush_range(phys_to_virt(AVCPU_STATUS_FLAG_STD_ADDR),phys_to_virt(AVCPU_STATUS_FLAG_STD_ADDR+0x4));
	outer_flush_range(AVCPU_STATUS_FLAG_STD_ADDR, AVCPU_STATUS_FLAG_STD_ADDR+0x4);

	/*Clear status flag and set complete flag*/
	avcpu_flag = 0;
	*((volatile int *)phys_to_virt(ACPU_STATUS_FLAG_ADDRESS)) |= 0x03000000;
	avcpu_flag |= 1;
#ifdef CONFIG_RTK_KDRV_SUPPORT_ACPU2
	*((volatile int *)phys_to_virt(ACPU2_STATUS_FLAG_ADDRESS)) |= 0x0C000000;
	avcpu_flag |= 8;
#endif
	*((volatile int *)phys_to_virt(VCPU_STATUS_FLAG_ADDRESS)) |= 0x03000000;
	avcpu_flag |= 2;
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	*((volatile int *)phys_to_virt(VCPU2_STATUS_FLAG_ADDRESS)) |= 0x03000000;
	avcpu_flag |= 4;
#endif
	flush_status_flag();


	//reset the highest temperature value
	rtd_maskl(STB_WDOG_DATA14_reg,0xFF00FFFF,0x0);

	rtd_outl(STB_WDOG_DATA14_reg, rtd_inl(STB_WDOG_DATA14_reg) | 0xF0000000); //set bit31-28;

	rtd_pr_misc_notice( "[AVCPU] avcpu_notify_suspend\n");

	ret = send_krpc(RPC_AUDIO, KRPC_PROCEDUREID_ROS, 0, ENUM_KERNEL_RPC_SUSPEND, 0, 0, &context);
	if( ret != RPC_OK)
	{
		rtd_pr_misc_err( "send rpc suspend AUDIO Error\n");
	}

	ret = send_krpc(RPC_VIDEO, KRPC_PROCEDUREID_ROS, 0, ENUM_KERNEL_RPC_SUSPEND, 0, 0, &context);
	if( ret != RPC_OK)
	{
		rtd_pr_misc_err( "send rpc suspend  VIDEO Error\n");
	}
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	ret = send_krpc(RPC_VIDEO2, KRPC_PROCEDUREID_ROS, 0, ENUM_KERNEL_RPC_SUSPEND, 0, 0, &context);
	if( ret != RPC_OK)
	{
		rtd_pr_misc_err( "send rpc suspend  VIDEO2 Error\n");
	}
#endif

	timeout = jiffies + HZ*AVCPU_SUSPEND_RESUME_TIMEOUT_SEC;
	do
	{

		invalidate_status_flag();

		if (((*((volatile int *)phys_to_virt(ACPU_STATUS_FLAG_ADDRESS))) & 0x03000000) == 0)
		{
			wait_flag |= 1;
		}else {
		}
#ifdef CONFIG_RTK_KDRV_SUPPORT_ACPU2
		if (((*(volatile int *)phys_to_virt(ACPU2_STATUS_FLAG_ADDRESS)) & 0x0C000000) == 0)
		{
			wait_flag |= 8;
		}
		else {
		}
#endif
		if ((((*((volatile int *)phys_to_virt(VCPU_STATUS_FLAG_ADDRESS))) & 0x03000000) == 0)
			|| ((rtd_inl(STB_WDOG_DATA14_reg)&0xC0000000) ==0x0))
		{
			wait_flag |= 2;
		}
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
		if ((((*((volatile int *)phys_to_virt(VCPU2_STATUS_FLAG_ADDRESS))) & 0x03000000) == 0)
			|| ((rtd_inl(STB_WDOG_DATA14_reg)&0x30000000) ==0x0))
		{
			wait_flag |= 4;
		}
#endif
		if((wait_flag & avcpu_flag) == avcpu_flag)
			break;
	}while(time_before(jiffies, timeout));


	if(!(wait_flag & 0x1)) {
		rtd_pr_misc_err( "AUDIO 1 SUSPEND FAIL!! %x\n", *((volatile int *)phys_to_virt(ACPU_STATUS_FLAG_ADDRESS)));
	}

#ifdef CONFIG_RTK_KDRV_SUPPORT_ACPU2
	if(!(wait_flag & 0x8)) {
		rtd_pr_misc_err( "AUDIO 2 SUSPEND FAIL!! %x\n", *((volatile int *)phys_to_virt(ACPU2_STATUS_FLAG_ADDRESS)));
	}
#endif

	if(!(wait_flag & 0x2)) {
		rtd_pr_misc_err( "VIDEO SUSPEND FAIL!! %x\n", *((volatile int *)phys_to_virt(VCPU_STATUS_FLAG_ADDRESS)));
	}

#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	if(!(wait_flag & 0x4)) {
		rtd_pr_misc_err( "VIDEO2 SUSPEND FAIL!! %x\n", *((volatile int *)phys_to_virt(VCPU2_STATUS_FLAG_ADDRESS)));
	}
#endif

	if(wait_flag != avcpu_flag)
	{
		force_console_enable();
		panic("[AVCPU] Suspend fail(%x/%x)\n", wait_flag, rtd_inl(STB_WDOG_DATA14_reg));

	}
	return ((wait_flag == avcpu_flag) ? 0: -1);
}


#define _RETRY_  (5)
static int avcpu_notify_resume(void)
{
	int ret = 0;
	unsigned long timeout;
	int wait_flag = 0, avcpu_flag;

	avcpu_flag = 0;
	avcpu_flag |= 1;
#ifdef CONFIG_RTK_KDRV_SUPPORT_ACPU2
	avcpu_flag |= 8;
#endif
	avcpu_flag |= 2;
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	avcpu_flag |= 4;
#endif
	rtd_pr_misc_notice( "[AVCPU] avcpu_notify_resume\n");

	/* notify AV CPU resume, for STD make image then resume case*/
	*((volatile int *)phys_to_virt(AVCPU_STATUS_FLAG_STD_ADDR)) = 0x03000000;
	dmac_flush_range(phys_to_virt(AVCPU_STATUS_FLAG_STD_ADDR),phys_to_virt(AVCPU_STATUS_FLAG_STD_ADDR+0x4));
	outer_flush_range(AVCPU_STATUS_FLAG_STD_ADDR, AVCPU_STATUS_FLAG_STD_ADDR+0x4);

	/*Clear SCPU STR status flag for AVCPU reference*/
	*((volatile int *)phys_to_virt(SCPU_STR_STATUS_FLAG_ADDR)) = SCPU_STR_STATUS_SUSPEND_CLEAR;
	dmac_flush_range(phys_to_virt(SCPU_STR_STATUS_FLAG_ADDR),phys_to_virt(SCPU_STR_STATUS_FLAG_ADDR+0x4));
	outer_flush_range(SCPU_STR_STATUS_FLAG_ADDR, SCPU_STR_STATUS_FLAG_ADDR+0x4);

	wait_flag = 0;
	timeout = jiffies + HZ*AVCPU_SUSPEND_RESUME_TIMEOUT_SEC;
	do
	{
		invalidate_status_flag();

		if ((*((volatile int *)phys_to_virt(ACPU_STATUS_FLAG_ADDRESS)) & 0x03000000) == 0x03000000)
			wait_flag |= 1;

#ifdef CONFIG_RTK_KDRV_SUPPORT_ACPU2
		if ((*((volatile int *)phys_to_virt(ACPU2_STATUS_FLAG_ADDRESS)) & 0x0C000000) == 0x0C000000)
			wait_flag |= 8;
#endif

		if(((*((volatile int *)phys_to_virt(VCPU_STATUS_FLAG_ADDRESS)) & 0x03000000) == 0x03000000)
			||((rtd_inl(STB_WDOG_DATA14_reg)&0xC0000000) ==0xC0000000))
		{
			wait_flag |= 2;
		}

#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
		if(((*((volatile int *)phys_to_virt(VCPU2_STATUS_FLAG_ADDRESS)) & 0x03000000) == 0x03000000)
			||((rtd_inl(STB_WDOG_DATA14_reg)&0x30000000) ==0x30000000))
		{
			wait_flag |= 4;
		}
#endif

		if((wait_flag & avcpu_flag) == avcpu_flag)
			break;
	}while(time_before(jiffies, timeout));

	if(!(wait_flag & 0x1))
	{

		rtd_pr_misc_err( "AUDIO 1 RESUME FAIL!! %x", *((volatile int *)phys_to_virt(ACPU_STATUS_FLAG_ADDRESS)));
		ret = -1;
	}

#ifdef CONFIG_RTK_KDRV_SUPPORT_ACPU2
	if(!(wait_flag & 0x8))
	{
		rtd_pr_misc_err( "AUDIO 2 RESUME FAIL!! %x", *((volatile int *)phys_to_virt(ACPU2_STATUS_FLAG_ADDRESS)));
		ret = -1;
	}
#endif

	if(!(wait_flag & 0x2))
	{
		rtd_pr_misc_err( "VIDEO RESUME FAIL!! %x", *((volatile int *)phys_to_virt(VCPU_STATUS_FLAG_ADDRESS)));
		ret = -1;
	}

#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	if(!(wait_flag & 0x4))
	{
		rtd_pr_misc_err( "VIDEO2 RESUME FAIL!! %x", *((volatile int *)phys_to_virt(VCPU2_STATUS_FLAG_ADDRESS)));
		ret = -1;
	}
#endif

#if 0 //test for futhure  
	if ((wait_flag != 0xf)  &&
	    (acpufail_check_flag < _RETRY_))
		goto again;
#endif		
	/* Clear AV CPU resume status*/
	*((volatile int *)phys_to_virt(AVCPU_STATUS_FLAG_STD_ADDR)) = 0x0;
	dmac_flush_range(phys_to_virt(AVCPU_STATUS_FLAG_STD_ADDR),phys_to_virt(AVCPU_STATUS_FLAG_STD_ADDR+0x4));
	outer_flush_range(AVCPU_STATUS_FLAG_STD_ADDR, AVCPU_STATUS_FLAG_STD_ADDR+0x4);

	if(wait_flag != avcpu_flag)
	{
		force_console_enable();
		panic("[AVCPU] Resume fail(%x/%x)\n", wait_flag, rtd_inl(STB_WDOG_DATA14_reg));

	}
	return ((wait_flag == avcpu_flag) ? 0: -1);
}

int avcpu_suspend(void)
{
	rtd_pr_misc_notice( "Before [AVCPU] suspend\n");
	return avcpu_notify_suspend();
}

int avcpu_resume(void)
{
	return avcpu_notify_resume();
}

static int avcpu_pm_notifier_event(struct notifier_block *this, unsigned long event, void *ptr)
{
        switch (event) {
	case PM_SUSPEND_PREPARE:
		/*Notify SCPU STR suspend flow begin*/
		*((volatile int *)phys_to_virt(SCPU_STR_STATUS_FLAG_ADDR)) = SCPU_STR_STATUS_SUSPEND_BEGIN;
		dmac_flush_range(phys_to_virt(SCPU_STR_STATUS_FLAG_ADDR),phys_to_virt(SCPU_STR_STATUS_FLAG_ADDR+0x4));
		outer_flush_range(SCPU_STR_STATUS_FLAG_ADDR, SCPU_STR_STATUS_FLAG_ADDR+0x4);
		break;
	case PM_POST_SUSPEND:
		break;
        default:
                break;
        }
	return NOTIFY_DONE;
}


struct notifier_block avcpu_pm_notifier = {
        .notifier_call = avcpu_pm_notifier_event,
};
