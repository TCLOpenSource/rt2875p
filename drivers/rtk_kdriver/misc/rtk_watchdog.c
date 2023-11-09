
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
#include <linux/panic_notifier.h>
#endif

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/ioctl.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/kthread.h>              /* kernel thread */
#include <linux/suspend.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <linux/sched/types.h>
#endif
#include <uapi/linux/sched/types.h>
#include <linux/sched/rt.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <rbus/iso_misc_reg.h>
#include <rbus/wdog_reg.h>
#include <rbus/stb_reg.h>
#include <rbus/vde_reg.h>
#if defined(CONFIG_ARCH_RTK2885P)
#include <rbus/iso_gpio_reg.h>
#else
#include <rbus/gpio_reg.h>
#endif
#include <linux/freezer.h>
#include <rtk_kdriver/rtd_logger.h>
#include <rbus/sb2_reg.h>
#include <rbus/dc_mc_reg.h>
#include <mach/platform.h>
#include <rtk_kdriver/rtk_watchdog.h>
#include <rtd_types.h>
#include <generated/autoconf.h>
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rmm/rmm.h>
#include <linux/interrupt.h>
#include <rtd_log/rtd_module_log.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#define WDOG_DEVICE_NAME			"rtk_watchdog"
#else
#define WDOG_DEVICE_NAME			"watchdog"
#endif

#ifdef CONFIG_ARM64
#include <linux/mm.h>
#include <asm/uaccess.h>
#include <asm/ptrace.h>
#include <linux/kdebug.h>
#include <linux/notifier.h>

static void dump_mem2(const char *lvl, const char *str, unsigned long bottom,
		      unsigned long top, unsigned long base, int user);
static int rtk_dump_mem(struct notifier_block *nb, unsigned long val,
			    void *regs);
static int rtk_panic(struct notifier_block *self, unsigned long v, void *p);
static struct notifier_block rtk_dump_mem_nb = {
	       .notifier_call = rtk_dump_mem,
	};
static struct notifier_block rtk_panic_nb = {
	       .notifier_call = rtk_panic,
	};
#endif

#define WDOG_FREQUENCY 27000000   //27Mhz
#define WDOG_SECS_TO_TICKS(secs)		((secs) * WDOG_FREQUENCY)
#define WDOG_TICKS_TO_SECS(ticks)		((ticks) / WDOG_FREQUENCY)
#define WDOG_TIMEOUT_MAX            60
#define WDOG_DEFAULT_PRETIMEOUT     2
#define WDOG_STATUS_MAGIC_NUM       0x12348765

#define LAST_BOOT_REASON_SOFTWARE BIT(2)       //last boot reason is software-induced reset
#define LAST_BOOT_REASON_BUTTION  BIT(3)       //last boot reason is reset button press
#define LAST_BOOT_REASON_POWER    BIT(4)       //last boot reason is power cycle  
#define LAST_BOOT_REASON_WATCHDOG BIT(5)       //last boot reason is watchdog expiration  
#define WDIOS_ENABLE_WITH_KERNEL_AUTO_KICK     0x0008          /* Turn on the watchdog timer,and kernel will kick watchdog by itself */

#define	WDIOC_REBOOT_IMMEDIATELY	_IOR(WATCHDOG_IOCTL_BASE, 11, int)
#define WDIOS_HW_RESET                   		0x0001          /* paltform HW reset Immediately*/
#define WDIOS_SW_RESET                    	0x0002          /* paltform SW reset Immediately*/

typedef enum{
	RHAL_WDOG_IS_ON_NEED_AP_KICK = 0,
	RHAL_WDOG_IS_OFF,
	RHAL_WDOG_IS_ON_WITH_KERNEL_AUTO_KICK,
	RHAL_WDOG_STATUS_MAX	
}RHAL_WDOG_STATUS_T;
unsigned int wdog_major = 0;
unsigned int wdog_minor = 0;
static struct wdog_dev *wdog_drv_dev;
static struct class *wdog_class;
static unsigned int rtk_wdog_kick_by_ap = 0;
static DEFINE_MUTEX(rtk_wdog_mutex);

static unsigned long    watchdog_task = 0;

__maybe_unused int is_in_misc_isr = 0;
EXPORT_SYMBOL(is_in_misc_isr);


#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
extern int is_in_dcmt_isr;
#else
static int is_in_dcmt_isr = 0;
#endif

static bool wdog_pm_nosig_freezing = false;

/* ML7BU-1050
 * Command register:
 * 180C2748[24] => 0: off; 1: on
 *
 * Status register:
 * 180C274C[15:0] for error status
 */
#define CMD_REG (DC_MC_MC_DMY_WDG_RST_FW2_reg)   //(0xB80C2748)
#define STA_REG (DC_MC_MC_DMY_WDG_RST_FW3_reg)   //(0xB80C274C)


extern EnumPanicCpuType g_rtdlog_panic_cpu_type;
static __maybe_unused bool rtd_show_log[NORMAL] = {1,1,1,1,1,1,1};

#ifdef CONFIG_REALTEK_UART2RBUS_CONTROL
extern void enable_uart2rbus(unsigned int value);
#endif

extern void handle_sysrq(int key);
void DDR_scan_set_error(int cpu)
{
	unsigned int reg_base = STA_REG;
	unsigned int reg_mask;
	unsigned int reg_shift;
	unsigned int ctrl_reg_base = CMD_REG;
	unsigned int i = 0;

	if (cpu >= DDR_SCAN_STATUS_NUM)
		return;

	if ((rtd_inl(ctrl_reg_base) & _BIT24) == 0x0)
		return;

	switch(cpu) {
		case DDR_SCAN_STATUS_SCPU: // SCPU
			reg_mask = 0x00000001;
			reg_shift = 0;
			break;
		case DDR_SCAN_STATUS_ACPU1: // ACPU1
			reg_mask = 0x00000002;
			reg_shift = 1;
			break;
		case DDR_SCAN_STATUS_ACPU2: // ACPU2
			reg_mask = 0x00000004;
			reg_shift = 2;
			break;
		case DDR_SCAN_STATUS_VCPU1: // VCPU1
			reg_mask = 0x00000008;
			reg_shift = 3;
			break;
		case DDR_SCAN_STATUS_VCPU2: // VCPU2
			reg_mask = 0x00000010;
			reg_shift = 4;
			break;
		case DDR_SCAN_STATUS_KCPU: // KCPU
			reg_mask = 0x00000020;
			reg_shift = 5;
			break;
		case DDR_SCAN_STATUS_GPU: // GPU
			reg_mask = 0x00000040;
			reg_shift = 6;
			break;
		default:
			return;
	}

	// wait and get hw semaphore 2
	while(1) {
		if ((rtd_inl(SB2_HD_SEM_NEW_2_reg) & _BIT0) == 0x1)
			break;

		for(i = 0;i < 1024;i++); //reduce rbus traffic
	}

	rtd_maskl(reg_base, ~(reg_mask), 1 << reg_shift);

	// release hw semaphore 2
	rtd_outl(SB2_HD_SEM_NEW_2_reg, 0x0);

}

extern unsigned int debugMode; // declaire in mach-rtdxxxx/init.c
static int my_panic(const char *fmt, ...)
{
#ifdef CONFIG_REALTEK_UART2RBUS_CONTROL
	//enable_uart2rbus(1);
#endif
	if (fmt) {
		struct va_format vaf;
		va_list args;

		va_start(args, fmt);

		vaf.fmt = fmt;
		vaf.va = &args;

		if (debugMode == RELEASE_LEVEL)
			panic("%pV", &vaf);
		else {
#ifdef CONFIG_CUSTOMER_TV010
			panic("%pV", &vaf);
#else

			if((is_in_dcmt_isr)||(is_in_misc_isr))
			{
				va_end(args);
                                return 0;
			}
			rtd_pr_misc_err("%pV", &vaf);
#if IS_ENABLED(CONFIG_REALTEK_LOGBUF)
			if(rtd_show_log[g_rtdlog_panic_cpu_type]){
			        //DumpWork.param = g_rtdlog_panic_cpu_type;
			        //queue_work(p_dumpqueue, &DumpWork.work);
			        rtd_show_log[g_rtdlog_panic_cpu_type] = 0;
			        if(rtdlog_watchdog_dump_work((EnumPanicCpuType)g_rtdlog_panic_cpu_type))
                                        rtd_show_log[g_rtdlog_panic_cpu_type] = 1;
			}
#endif //   #if defined (CONFIG_REALTEK_LOGBUF)
#endif
		}

		va_end(args);
	}

	return 0;
}

static int flag_watchdog=0;
static void rtk_watchdog_set_flag(int flag)
{
	flag_watchdog = flag;
}

int rtk_watchdog_get_flag(void)
{
	return flag_watchdog;
}
EXPORT_SYMBOL(rtk_watchdog_get_flag);

/* return value: 1 => success, 0 => failure */
int kill_watchdog (void)
{
    watchdog_task = 0;
    return 1;
}

int judge_watchdog (void)
{
    return watchdog_task;
}

/*If hw watchdog enable, return 1, otherwise return 0.*/
int is_watchdog_enable(void)
{
	if((rtd_inl(WDOG_TCWCR_reg) & WDOG_TCWCR_wden_mask) == 0xa5)
		return 0;
	else
		return 1;
}
EXPORT_SYMBOL(is_watchdog_enable);

#ifdef CONFIG_CUSTOMER_TV043
static void rtk_wdog_judge_software_reboot(void)
{
    if(rtd_inl(STB_WDOG_DATA9_reg) == WDOG_STATUS_MAGIC_NUM){
        rtd_outl(STB_WDOG_DATA9_reg, 0);
    }            

}
#endif

void SetBootStatus(unsigned int bootmap)
{
    unsigned int bootreason = 0;
    bootreason = rtd_inl(STB_WDOG_DATA4);
    bootreason |= bootmap;
    rtd_outl(STB_WDOG_DATA4, bootreason);
}


extern int panic_timeout;
static int wdt_status=1;

#ifdef MODULE
static int wdt_setup (char *str)
#else
static int __init wdt_setup (char *str)
#endif
{
    if (!strcmp(str,"off")) //if wdt=off, means disable wdog.
    {
        wdt_status=0;
        panic_timeout = 0;
        watchdog_enable(0);  //disable wdog
    }
    return 0;
}

#ifndef MODULE
__setup("wdt=", wdt_setup);
#else
extern bool rtk_parse_commandline_equal(const char *string, char *output_string, int string_size);
void get_bootparam_wdt_set (void)
{
    char para_buff[20]={0};

    memset(para_buff,0,sizeof(para_buff));
    if(rtk_parse_commandline_equal("wdt", para_buff,sizeof(para_buff)) == 0)
    {
        rtd_pr_misc_err("Error : can't get wdt_set reclaim from bootargs\n");
        return;
    }
    wdt_setup(para_buff);
    
    return;
}
#endif

//WDOG_TCWOV_reg 0xB8062210
//WDOG_TCWCR_reg 0xB8062204
int watchdog_enable (unsigned char On)
{
    if (!wdt_status)
    {
        rtd_pr_misc_err("\e[1;31mwarning\e[0m! watchdog NOT enabled by bootcode setting wdt=skip\n");
    }

    if (On && wdt_status)
    {
        if(rtd_inl(WDOG_TCWOV_reg) == 0x10237980 || rtd_inl(WDOG_TCWOV_reg) == 0x10237880){
            rtd_maskl(WDOG_TCWCR_reg, ~WDOG_TCWCR_wden_mask, 0xa5);
            rtd_outl(WDOG_TCWOV_reg, 0x10237880);
        }else {
        /* enable watchdog */
        rtd_outl(WDOG_TCWTR_reg, 0x01);
        rtd_maskl(WDOG_TCWCR_reg, ~WDOG_TCWCR_wden_mask, 0xa5);
        rtd_clearbits(WDOG_TCWCR_reg, WDOG_TCWCR_wden_mask);
        rtk_watchdog_set_flag(1);
        }
    }
    else
    {
        rtd_outl(WDOG_TCWTR_reg, 0x01); //clear wdog timer.
        /* disable watchdog */
        rtd_maskl(WDOG_TCWCR_reg, ~WDOG_TCWCR_wden_mask, 0xa5);
        rtk_watchdog_set_flag(2);
    }
    return 1;
}
EXPORT_SYMBOL(watchdog_enable);

//used to adjust watchodg max timeout value from 10s to 60s
void rtk_wdog_adjust_timeout_upper_bound(void)
{
	if(!(rtd_inl(WDOG_TCWCR_reg) & WDOG_TCWCR_upper_bound_sel_mask)){
		rtd_setbits(WDOG_TCWCR_reg, WDOG_TCWCR_upper_bound_sel_mask);
	}
}


void rtk_wdog_dump_vcpu_status (void)
{
	static unsigned int cnt = 0;
	if(cnt % 10 == 0) {
		rtd_pr_misc_err("=== dump VCPU1 status (0xB8001200 ~ 0XB800122C) ===\n");
		rtd_pr_misc_err("I0    :%08x I1    :%08x EPC    :%08x CAUSE :%08x\n", rtd_inl(VDE_VC_TRACE_0_reg), rtd_inl(VDE_VC_TRACE_1_reg), rtd_inl(VDE_VC_TRACE_2_reg), rtd_inl(VDE_VC_TRACE_3_reg));
		rtd_pr_misc_err("STATUS:%08x ECAUSE:%08x ESTATUS:%08x INTVEC:%08x\n", rtd_inl(VDE_VC_TRACE_4_reg), rtd_inl(VDE_VC_TRACE_5_reg), rtd_inl(VDE_VC_TRACE_6_reg), rtd_inl(VDE_VC_TRACE_7_reg));
		rtd_pr_misc_err("SP    :%08x RA    :%08x RETIRE :%08x DEBUG :%08x\n", rtd_inl(VDE_VC_TRACE_8_reg), rtd_inl(VDE_VC_TRACE_9_reg), rtd_inl(VDE_VC_TRACE_10_reg), rtd_inl(VDE_VC_DBG_reg));
		rtd_pr_misc_err("===================================================\n");
	}
	cnt++;
}

#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
#define VDE2_OFFSET(reg) reg + 0x36000 //vde2_reg.h
void rtk_wdog_dump_vcpu2_status (void)
{
	static unsigned int cnt = 0;
	if(cnt % 10 == 0) {
		rtd_pr_misc_err("=== dump VCPU2 status (0xB8037200 ~ 0XB803722C) ===\n");
		rtd_pr_misc_err("I0    :%08x I1    :%08x EPC    :%08x CAUSE :%08x\n", rtd_inl(VDE2_OFFSET(VDE_VC_TRACE_0_reg)), rtd_inl(VDE2_OFFSET(VDE_VC_TRACE_1_reg)), rtd_inl(VDE2_OFFSET(VDE_VC_TRACE_2_reg)), rtd_inl(VDE2_OFFSET(VDE_VC_TRACE_3_reg)));
		rtd_pr_misc_err("STATUS:%08x ECAUSE:%08x ESTATUS:%08x INTVEC:%08x\n", rtd_inl(VDE2_OFFSET(VDE_VC_TRACE_4_reg)), rtd_inl(VDE2_OFFSET(VDE_VC_TRACE_5_reg)), rtd_inl(VDE2_OFFSET(VDE_VC_TRACE_6_reg)), rtd_inl(VDE2_OFFSET(VDE_VC_TRACE_7_reg)));
		rtd_pr_misc_err("SP    :%08x RA    :%08x RETIRE :%08x DEBUG :%08x\n", rtd_inl(VDE2_OFFSET(VDE_VC_TRACE_8_reg)), rtd_inl(VDE2_OFFSET(VDE_VC_TRACE_9_reg)), rtd_inl(VDE2_OFFSET(VDE_VC_TRACE_10_reg)), rtd_inl(VDE2_OFFSET(VDE_VC_DBG_reg)));
		rtd_pr_misc_err("===================================================\n");
	}
	cnt++;
}
#endif

// SB2_DUMMY_2_reg:
//		[31:24] : audio1
//		[23:16] : audio2
//		[15: 0] : video1
// SB2_DUMMY_3_reg:
//		[31:16] : kcpu
//		[15: 0] : video2

#define ACPU1_WDOG_MAGIC_VALUE	(0x23000000)
#define ACPU1_WDOG_MAGIC_MASK	(0xFF000000)

#ifdef CONFIG_RTK_KDRV_SUPPORT_ACPU2
#define ACPU2_WDOG_MAGIC_VALUE	(0x00790000)
#define ACPU2_WDOG_MAGIC_MASK	(0x00FF0000)
#endif

#define VCPU1_WDOG_MAGIC_VALUE	(0x00002379)
#define VCPU1_WDOG_MAGIC_MASK	(0x0000FFFF)

#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
#define VCPU2_WDOG_MAGIC_VALUE	(0x00002379)
#define VCPU2_WDOG_MAGIC_MASK	(0x0000FFFF)
#endif

#define KCPU_WDOG_MAGIC_VALUE	(0x23790000)
#define KCPU_WDOG_MAGIC_MASK	(0xFFFF0000)

int hw_watchdog (void *p)
{
#define TO_MSEC 2000
//	unsigned long timeout;
	bool enable_check_av_watchgod = true;
	unsigned long to_a, to_v, to_k, to_v2, to_a2;
	unsigned int val_dummy_2, val_dummy_3;
	unsigned int not_tick_cnt_v=0, not_tick_cnt_a =0;
	struct sched_param param = { .sched_priority = MAX_RT_PRIO-1 };
	watchdog_task = (unsigned long)current;
#if 0 // fix me - jinn - kernel porting
	daemonize("hw_watchdog");
#endif
	sched_setscheduler(current, SCHED_FIFO, &param);
//#if 0 /*no need to turn on watchdog, watchdog on/off control by bootcode.*/
#ifdef CONFIG_CUSTOMER_TV010
	watchdog_enable(1);
	rtd_pr_misc_err("enter hw_watchdog!!!    %x\n",current->flags);
#endif

#ifdef CONFIG_CUSTOMER_TV043
	rtk_wdog_judge_software_reboot();
#endif
	rtk_wdog_adjust_timeout_upper_bound();
	to_a = to_v = to_k = to_v2 = to_a2 = jiffies + msecs_to_jiffies(TO_MSEC);

        strncpy(current->comm,"hw_watchdog",sizeof(current->comm));
	while (1)
	{
		if(rtk_wdog_kick_by_ap == 0)
		{
			//rtd_pr_misc_err("now kernel control kick!!!\n");
			rtd_outl(WDOG_TCWTR_reg, 0x01);
		}
		else
		{
			//rtd_pr_misc_err("now ap control kick!!!\n");
		}
	if(enable_check_av_watchgod)
	{
		val_dummy_2 = rtd_inl(SB2_DUMMY_2_reg);
		val_dummy_3 = rtd_inl(SB2_DUMMY_3_reg);

		if ( (val_dummy_2 & VCPU1_WDOG_MAGIC_MASK) == VCPU1_WDOG_MAGIC_VALUE ) // video1
		{
			local_irq_disable();
			to_v = jiffies + msecs_to_jiffies(TO_MSEC);
			rtd_maskl(SB2_DUMMY_2_reg, ~VCPU1_WDOG_MAGIC_MASK, 0x0000);
			local_irq_enable();
			not_tick_cnt_v = 0;
		}
		else
			not_tick_cnt_v++;
		if ( (val_dummy_2 & ACPU1_WDOG_MAGIC_MASK) == ACPU1_WDOG_MAGIC_VALUE ) // audio1
		{
			local_irq_disable();
			to_a = jiffies + msecs_to_jiffies(TO_MSEC);
			rtd_maskl(SB2_DUMMY_2_reg, ~ACPU1_WDOG_MAGIC_MASK, 0x00);
			local_irq_enable();
			not_tick_cnt_a = 0;
		}
		else
			not_tick_cnt_a++;
#ifdef CONFIG_RTK_KDRV_SUPPORT_ACPU2
		if ( (val_dummy_2 & ACPU2_WDOG_MAGIC_MASK) == ACPU2_WDOG_MAGIC_VALUE ) // audio2
		{
			to_a2 = jiffies + msecs_to_jiffies(TO_MSEC);
			rtd_maskl(SB2_DUMMY_2_reg, ~ACPU2_WDOG_MAGIC_MASK, 0x00);
		}
#endif
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
		if ( (val_dummy_3 & VCPU2_WDOG_MAGIC_MASK) == VCPU2_WDOG_MAGIC_VALUE ) // video2
		{
			to_v2 = jiffies + msecs_to_jiffies(TO_MSEC);
			rtd_maskl(SB2_DUMMY_3_reg, ~VCPU2_WDOG_MAGIC_MASK, 0x0000);
		}
#endif

#if !IS_ENABLED(CONFIG_RTK_KDRV_TEE)
		if ( (val_dummy_3 & KCPU_WDOG_MAGIC_MASK) == KCPU_WDOG_MAGIC_VALUE ) // kcpu
		{
			to_k = jiffies + msecs_to_jiffies(TO_MSEC);
			rtd_maskl(SB2_DUMMY_3_reg, ~KCPU_WDOG_MAGIC_MASK, 0x0000);
		}
#endif

		if (time_is_before_jiffies(to_a)) {
			set_rtdlog_panic_cpu_type(ACPU1);
			DDR_scan_set_error(DDR_SCAN_STATUS_ACPU1);
			my_panic("[A1DSP] WatchDog does not receive signal for %d seconds, value: %08x, Tick(0x%x) \n", TO_MSEC/1000, val_dummy_2, not_tick_cnt_a);
		}
#ifdef CONFIG_RTK_KDRV_SUPPORT_ACPU2
		if (time_is_before_jiffies(to_a2)) {
			set_rtdlog_panic_cpu_type(ACPU2);
			DDR_scan_set_error(DDR_SCAN_STATUS_ACPU2);
			my_panic("[A2DSP] WatchDog does not receive signal for %d seconds, value: %08x \n", TO_MSEC/1000, val_dummy_2);
		}
#endif
		if (time_is_before_jiffies(to_v)) {
			set_rtdlog_panic_cpu_type(VCPU1);
			DDR_scan_set_error(DDR_SCAN_STATUS_VCPU1);
			rtk_wdog_dump_vcpu_status();
			my_panic("[V1DSP] WatchDog does not receive signal for %d seconds, value: %08x, Tick(0x%x) \n", TO_MSEC/1000, val_dummy_2, not_tick_cnt_v);
		}
#if !IS_ENABLED(CONFIG_RTK_KDRV_TEE)
		if (time_is_before_jiffies(to_k)) {
			/*ravi_li dump crt info when KCPU not kick watchdog*/
			set_rtdlog_panic_cpu_type(KCPU);
			DDR_scan_set_error(DDR_SCAN_STATUS_KCPU);
			rtd_pr_misc_err("read SYS_REG_SYS_SRST0_reg val == %#x read SYS_REG_SYS_CLKEN0_reg val == %#x \n", rtd_inl(SYS_REG_SYS_SRST0_reg), rtd_inl(SYS_REG_SYS_CLKEN0_reg));
			my_panic("[KDSP] WatchDog does not receive signal for %d seconds, value: %08x \n", TO_MSEC/1000, val_dummy_3);
		}
#endif
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
		if (time_is_before_jiffies(to_v2)) {
			set_rtdlog_panic_cpu_type(VCPU2);
			DDR_scan_set_error(DDR_SCAN_STATUS_VCPU2);
			rtk_wdog_dump_vcpu2_status();
			my_panic("[V2DSP] WatchDog does not receive signal for %d seconds, value: %08x \n", TO_MSEC/1000, val_dummy_3);
		}
#endif
	}
		do
		{
			msleep(100);
			/* MA6PBU-1437, when system entering freeze state, disable check a/v watchdog.
			 * after system resume, enable check a/v watchdog
			 * Besides, tick HW watchdog register, alyways tick for prevent any system crash during STR suspend/resume flow.
			 */
			if(wdog_pm_nosig_freezing && enable_check_av_watchgod)
			{
				rtd_pr_misc_notice("Disable check A/V watchdog!!\n");
				enable_check_av_watchgod = false;
			}
			else
			{
				if(!wdog_pm_nosig_freezing && !enable_check_av_watchgod)
				{
					rtd_pr_misc_notice("Enable check A/V watchdog!!\n");
					enable_check_av_watchgod = true;
					to_a = to_v = to_k = jiffies + msecs_to_jiffies(TO_MSEC);
					not_tick_cnt_v = not_tick_cnt_a = 0;
				}
			}

		}
                while(0);

		if (watchdog_task == 0)
		{
			watchdog_enable(0);
			break;
		}

	}

	rtd_pr_misc_err("hw_watchdog: exit...\n");
	do_exit(0);
	return 0;
}

static int watchdog_pm_notifier_event(struct notifier_block *this, unsigned long event, void *ptr)
{
    switch (event) {
    case PM_SUSPEND_PREPARE:
        wdog_pm_nosig_freezing = true;
        break;
    case PM_POST_SUSPEND:
        wdog_pm_nosig_freezing = false;
        break;
    default:
        break;
    }
    return NOTIFY_DONE;
}

struct notifier_block watchdog_pm_notifier = {
    .notifier_call = watchdog_pm_notifier_event,
};

#if 0
int hw_watchdog(void *p)
{
    struct sched_param param = { .sched_priority = MAX_RT_PRIO-1 };
    //static int tmp=0;
#if 0 // fix me - jinn - kernel porting
    daemonize("hw_watchdog");
#endif
    sched_setscheduler(current, SCHED_FIFO, &param);
    current->flags &= ~PF_NOFREEZE;

    //  rtd_pr_misc_err("************************************\n");
    //  rtd_pr_misc_err("*********watchdog mechanism*********\n");
    //  rtd_pr_misc_err("************************************\n");

    /* enable watchdog */
    #if 0
    rtd_outl(WDOG_TCWTR_reg, 0x01);//outl(0x01, VENUS_MIS_TCWTR);//WDOG_TCWTR_reg
    rtd_outl(WDOG_TCWCR_reg, 0xa5);//outl(0xa5, VENUS_MIS_TCWCR);
    rtd_outl(WDOG_TCWOV_reg, 0xc000000);//outl(0xc000000, VENUS_MIS_TCWOV);
    #endif

    //rtd_outl(WDOG_TCWCR_reg, 0xc000000);//outl(0x00, VENUS_MIS_TCWCR);
    watchdog_task = (unsigned long)current;
    rtd_pr_misc_err("enter hw_watchdog!!!\n");

    while (1)
    {
        msleep_interruptible(1000);
        rtd_outl(WDOG_TCWTR_reg, 0x01);//outl(0x01, VENUS_MIS_TCWTR);

        if (watchdog_task == 0)
        {
            rtd_pr_misc_err("hw_watchdog: exit...\n");
            break;
        }

        /* check if we need to freeze */
#if 0
        if (freezing(current))
        {
            /* disable watchdog */
            outl(0xa5, VENUS_MIS_TCWCR);

            refrigerator();

            /* enable watchdog */
            outl(0xc000000, VENUS_MIS_TCWOV);
            outl(0x00, VENUS_MIS_TCWCR);
        }
#endif

    }
    do_exit(0);
    return 0;
}
#endif

static void rtk_wdog_ping (void)
{
	rtd_outl(WDOG_TCWTR_reg, 0x01);
}

static void rtk_wdog_set_timeout(unsigned int timeout)
{
	rtd_outl(WDOG_TCWOV_reg, WDOG_SECS_TO_TICKS(timeout));
}

static unsigned int rtk_wdog_get_timeout(void)
{
	//For The LSB 8 bit is fixed to 0x80 in hardware.(TCWOV_sel 32¡¯hxxxx_xxxx will map to 32¡¯hxxxx_xx80.)
	//so need add additional timeout Ticks offset to get the accurate timeout seconds. 
	return WDOG_TICKS_TO_SECS(rtd_inl(WDOG_TCWOV_reg) + 0xff);
}
static int rtk_wdog_set_pretimeout(unsigned int pretimeout)
{
	unsigned int tmp_time;
	if(pretimeout == 0){ //this case means disable watchdog NMI function.
		rtd_clearbits(WDOG_TCWCR_reg, WDOG_TCWCR_wdie_mask);                                     
	}
	tmp_time = rtk_wdog_get_timeout() - pretimeout;
	if(tmp_time <= 0 ){
		rtd_pr_misc_err("[%d][%s]ERR:the pretimout value should not larger than the watchdog timeout value. \n",__LINE__ ,__func__);
		return -1;
	}
	rtd_outl(WDOG_TCWNMI_reg, WDOG_SECS_TO_TICKS(tmp_time));
	rtd_setbits(WDOG_TCWCR_reg, WDOG_TCWCR_wdie_mask);   
	return 0;
}
static int rtk_wdog_get_pretimeout(void)
{
	unsigned int tmp_time = WDOG_TICKS_TO_SECS(rtd_inl(WDOG_TCWNMI_reg));
	int pretimeout = rtk_wdog_get_timeout() - tmp_time;
	if(pretimeout < 0){
		pretimeout = 0;
	}
	return pretimeout;
}

static int rtk_wdog_getbootstatus(void)
{
	int tmp_bootstatus = 0;
	int tmp_wdc = 0;

        tmp_wdc = WDOG_TCWCR_get_wd_rst_src(rtd_inl(WDOG_TCWCR_reg));
        if(tmp_wdc == 0x1){
            tmp_bootstatus = LAST_BOOT_REASON_SOFTWARE;
        }else if(tmp_wdc == 0x2){
            tmp_bootstatus = LAST_BOOT_REASON_WATCHDOG;
        }else{
            tmp_bootstatus = LAST_BOOT_REASON_POWER;
        }  
        
	return tmp_bootstatus;
}

static void rtk_wdog_reset(int reset_type)
{
    volatile unsigned int iRegVal;
    
    if(reset_type & WDIOS_HW_RESET)//HW reset
    {
#if defined(CONFIG_ARCH_RTK2885P)
        iRegVal = rtd_inl(ISO_GPIO_GPDATO_1_reg);
        iRegVal |= (1<<ISO_GPIO_GPDATO_1_gpdato44_shift);//pull low to POWER_EN pin
        rtd_outl(ISO_GPIO_GPDATO_1_reg, iRegVal);
#else
        iRegVal = rtd_inl(GPIO_GPDATO_1_reg);
        iRegVal |= (1<<GPIO_GPDATO_1_gpdato44_shift);//pull low to POWER_EN pin
        rtd_outl(GPIO_GPDATO_1_reg, iRegVal);
#endif
        while(1)
        {
            rtd_pr_misc_err("%s HW RESET...\n",__FUNCTION__);
        }
        
    }else if(reset_type & WDIOS_SW_RESET){//watchdog reset
        // disable all interrupt, ?
        rtd_clearbits(WDOG_TCWCR_reg, 0xff);
        // retart system
        // enable the dog
        iRegVal = rtd_inl(WDOG_TCWCR_reg);
        iRegVal |= (1<<WDOG_TCWCR_im_wdog_rst_shift); // bit 31
        rtd_outl(WDOG_TCWCR_reg, iRegVal);
        while(1)
        {
            rtd_pr_misc_err("%s SW RESET...\n",__FUNCTION__);
        }
        
    }
    return;
}

int rtk_wdog_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int timeout = 0;
	int pretimeout = 0;
	int timeout_left = 0;
	int boot_status = 0;
	int option =0;
        int timeout_tick = 0;
	RHAL_WDOG_STATUS_T wdog_status = RHAL_WDOG_IS_OFF;
	switch(cmd) {
			case WDIOC_KEEPALIVE:
					rtk_wdog_ping();
					break;

			case WDIOC_SETTIMEOUT:
					if (get_user(timeout, (int __user *)arg))
							return -EFAULT;
					if (timeout > WDOG_TIMEOUT_MAX)
					{
							rtd_pr_misc_err("[%d][%s]ERR:Set timeout value %d Fail,For timeout value should between: 0 ~ %d seconds!!!\n", __LINE__ ,__func__, timeout, WDOG_TIMEOUT_MAX);
							return -EFAULT;
					}

					if(timeout > 10)
					{
							rtk_wdog_adjust_timeout_upper_bound();
					}
					rtk_wdog_set_timeout(timeout);
					rtk_wdog_ping();
					break;	

			case WDIOC_SETOPTIONS: 
					if (copy_from_user(&option, (int __user *)arg, sizeof(int)))
							return -EFAULT;

					if (option & WDIOS_DISABLECARD) {
							rtk_wdog_kick_by_ap = 0;
							watchdog_enable(0);
					}

					if (option & WDIOS_ENABLECARD) {
							rtk_wdog_kick_by_ap = 1;
							watchdog_enable(1);
							rtk_wdog_ping();
					}

					if (option & WDIOS_ENABLE_WITH_KERNEL_AUTO_KICK) {
							rtk_wdog_kick_by_ap = 0;
							watchdog_enable(1); //use fault watchdog value.
							rtk_wdog_ping();
					}
					break;

			case WDIOC_GETTIMEOUT:
					timeout = rtk_wdog_get_timeout();
					return put_user(timeout, (int __user *)arg);	

			case WDIOC_GETTIMELEFT: 
					//get the time that's left before rebooting
					timeout_tick = rtd_inl(WDOG_TCWOV_reg) - rtd_inl(WDOG_TCWSTS_reg);
                                        if(timeout_tick < 0){
							timeout_tick = 0;
					}
					timeout_left = WDOG_TICKS_TO_SECS(timeout_tick);
					return put_user(timeout_left, (int __user *)arg);

			case WDIOC_SETPRETIMEOUT:
					if (get_user(pretimeout, (int __user *)arg))
							return -EFAULT;
					if (pretimeout > WDOG_TIMEOUT_MAX)
							return -EFAULT;	
					rtk_wdog_set_pretimeout(pretimeout);
					break;

			case WDIOC_GETPRETIMEOUT: 
					pretimeout = rtk_wdog_get_pretimeout();
					return put_user(pretimeout, (int __user *)arg);

			case WDIOC_GETBOOTSTATUS:
					boot_status =  rtk_wdog_getbootstatus();
					return put_user(boot_status, (int __user *)arg);

			case WDIOC_GETSTATUS:
					if(is_watchdog_enable()){
							if(rtk_wdog_kick_by_ap == 0){
									wdog_status = RHAL_WDOG_IS_ON_WITH_KERNEL_AUTO_KICK;
							}
							else{
									wdog_status = RHAL_WDOG_IS_ON_NEED_AP_KICK;      
							}
					}
					else{
							wdog_status = RHAL_WDOG_IS_OFF;
					}
					return put_user(wdog_status, (int __user *)arg);
			case WDIOC_REBOOT_IMMEDIATELY:
					if (copy_from_user(&option, (int __user *)arg, sizeof(int)))
							return -EFAULT;
                                        rtk_wdog_reset(option);
                                        break;	
			default:  /* redundant, as cmd was checked against MAXNR */
					rtd_pr_misc_err("wdog ioctl code not supported\n");
				        return -ENOTTY;
	}
	return 0;
       
}

static long rtk_wdog_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret;

	mutex_lock(&rtk_wdog_mutex);
	ret = rtk_wdog_ioctl(file, cmd, arg);
	mutex_unlock(&rtk_wdog_mutex);

	return ret;
}

struct file_operations wdog_fops = {
        .owner                 =    THIS_MODULE,
        .unlocked_ioctl        =    rtk_wdog_unlocked_ioctl,
};
#if 1//def CONFIG_CUSTOMER_TV043
irqreturn_t wdog_NMI_intr (int irq, void *dev_id)
{
    	int value = rtd_inl(WDOG_MSK_ISR_reg);

    	//struct task_struct *p;
    	if(!(value & WDOG_MSK_ISR_wdog_int_mask)) { //TODO:ALL CPU INTR BIT
        	return IRQ_NONE;
    	}
    	//clear wdog interrupt pending bit.Write ¡°1¡± to clear. 
    	rtd_setbits(WDOG_MSK_ISR_reg, WDOG_MSK_ISR_wdog_int_mask);
        //disable wdog interrupt function to avoid multiple print pretimeout log.
        rtd_clearbits(WDOG_TCWCR_reg, WDOG_TCWCR_wdie_mask);

        dump_stack();
        handle_sysrq('m'); /* dump memory stats */

#if 0 //following flow need wait for is_roku_application() ready.        
        rcu_read_lock();
        for_each_process(p) {
            task_lock(p);
            if (is_roku_application(p))
            { 
                send_sig(SIGKILL, p, 1); 
                task_unlock(p); 
                break;
            }
            task_unlock(p);
        }
        rcu_read_unlock();        
#endif        
        handle_sysrq('t'); /* dump scheduler state */
        return IRQ_HANDLED;
}
#endif

static int irq_init =0;
static int watchdog_probe(struct platform_device *pdev)
{
        int virq = -1,ret=0;
    
        if (irq_init==0)
        {
            virq = platform_get_irq(pdev, 0);
            if (virq < 0) 
            {
                rtd_pr_misc_err("watchdog_probe : platform_get_irq. ret=%d\n",virq);
                return virq;
            }

            ret = request_irq(virq, wdog_NMI_intr, IRQF_SHARED, "wdog", wdog_NMI_intr);
            if (ret)
            {
                rtd_pr_misc_err("!!!!!!!!!!!!!!!!! Register watchdog_probe Interrupt handler failed - IRQ %d !!!!!!!!!!!!!!!!\n", virq);
            }
            else
            {
                rtd_pr_misc_err("!!!!!!!!!!!!!!!!! Register watchdog_probe Interrupt handler successed - IRQ %d !!!!!!!!!!!!!!!!\n", virq);
                irq_init=1;
            }
	}
	return ret;
}

static int watchdog_remove(struct platform_device *pdev)
{
	int irq=0;
	if (irq_init==1)
	{
		irq = platform_get_irq(pdev, 0);
		if (irq >= 0) 
		{
			free_irq(irq, (void *)wdog_NMI_intr);
			irq_init=0;
		}
	}
	return 0;
}

static const struct of_device_id watchdog_match[] = {
	{
		.compatible = "realtek,watchdog",
	},
	{},
};

MODULE_DEVICE_TABLE(of, watchdog_match);

static struct platform_driver watchdog_driver =
{
	.probe = watchdog_probe,
	.remove = watchdog_remove,

	.driver   = {
		.name = "watchdog",
		.of_match_table = of_match_ptr(watchdog_match),
	}
};

#if 0
int __init rtk_wdog_init(void)
{
	int result;
	result = platform_driver_register(&watchdog_driver);
	if (result) 
	{
		rtd_pr_misc_err("rtk_wdog_init : can not register platform driver, ret=%d\n", result);
	}
	return result;
}

void __exit rtk_wdog_exit(void)
{
	platform_driver_unregister(&watchdog_driver);
}
#endif

#ifdef CONFIG_ARM64
static bool region_dumpable(unsigned long *base, unsigned long *bottom, unsigned long *top,
		int user, unsigned long forbidden_start, unsigned long forbidden_end)
{
	unsigned long par;

	par = PAR(*base, user);
	if (((par & SYS_PAR_EL1_PA_MASK) >= forbidden_start) && ((par & SYS_PAR_EL1_PA_MASK) < forbidden_end))
		return false;

	/* check bottom, top */
	par = PAR(*bottom, user);
	if ((par & SYS_PAR_EL1_F) ||
			((par & SYS_PAR_EL1_PA_MASK) >= forbidden_start &&
			(par & SYS_PAR_EL1_PA_MASK) < forbidden_end))
		*bottom = round_down(*base, PAGE_SIZE);  /* within the page as same as @base */

	par = PAR(*top, user);
	if ((par & SYS_PAR_EL1_F) ||
			((par & SYS_PAR_EL1_PA_MASK) >= forbidden_start &&
			(par & SYS_PAR_EL1_PA_MASK) < forbidden_end))
		*top = round_up(*base, PAGE_SIZE);      /* within the page as smae as @base */

	return true;
}


static void dump_mem2(const char *lvl, const char *istr, unsigned long bottom,
		      unsigned long top, unsigned long base, int user)
{
	unsigned long first, par;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	mm_segment_t fs;
#endif
	int i;
	unsigned int layout_idx = 0;
	unsigned long gic_start, gic_end, rbus_start, rbus_end;

	// shape the underflow and overflow
	if ((base < PAGE_SIZE) && ((long)bottom < 0))
		bottom = 0;
	if ((PAGE_ALIGN(base) == 0) && ((long)top >= 0))
		top = (unsigned long)(0-1);
	if ((base < bottom) || (base > top) || (top-bottom) > PAGE_SIZE ) {
		rtd_pr_misc_err("%s%s 0x%08lx (NA) - par(0x%lx)\n",  lvl, istr, base, PAR(base, user));
		return;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	uaccess_enable();
#endif
	par = PAR(base, user);
	if (par & SYS_PAR_EL1_F) {
		rtd_pr_misc_err("%s%s 0x%08lx (no mapping) - par(0x%lx)\n",
		       lvl, istr, base, par);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
		uaccess_disable();
#endif
		return;
	}

	layout_idx = carvedout_buf_get_layout_idx();
	/* skip GIC */
	gic_start = carvedout_buf[layout_idx][CARVEDOUT_MAP_GIC][0];
	gic_end = gic_start + carvedout_buf[layout_idx][CARVEDOUT_MAP_GIC][1];
	if (!region_dumpable(&base, &bottom, &top, user, gic_start, gic_end)) {
		rtd_pr_misc_err("%s%s 0x%08lx (GIC) - par(0x%lx)\n",
		       lvl, istr, base, par);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
		uaccess_disable();
#endif
		return;
	}

	/* skip RBUS */
	rbus_start = carvedout_buf[layout_idx][CARVEDOUT_MAP_RBUS][0];
	rbus_end = rbus_start + carvedout_buf[layout_idx][CARVEDOUT_MAP_RBUS][1];
	if (!region_dumpable(&base, &bottom, &top, user, rbus_start, rbus_end)) {
		rtd_pr_misc_err("%s%s 0x%08lx (RBUS) - par(0x%lx)\n",
		       lvl, istr, base, par);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
		uaccess_disable();
#endif
		return;
	}

	/*
	 * We need to switch to kernel mode so that we can use __get_user
	 * to safely read from kernel space.  Note that we now dump the
	 * code first, just in case the backtrace kills us.
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	fs = get_fs();
	set_fs(KERNEL_DS);
#endif

	rtd_pr_misc_err("%s%s 0x%016lx (0x%016lx to 0x%16lx) - par(0x%lx)\n", lvl, istr, base, bottom, top, par);

	for (first = bottom & ~31; first < top; first += 32) {
		unsigned long p;
		char _str[sizeof(" 12345678") * 8 + 1];

		memset(_str, ' ', sizeof(_str));
		_str[sizeof(_str) - 1] = '\0';

		for (p = first, i = 0; i < 4 && p < top; i++, p += 8) {
			if (p >= bottom && p < top) {
				unsigned long val;
				if (__get_user(val, (unsigned long *)p) == 0)
					sprintf(_str + i * 17, " %016lx", val);
				else { // the address (p) in kernel space
					val = *((unsigned long *)p);
					sprintf(_str + i * 17, " %016lx", val);
				}
			}
		}
		rtd_pr_misc_err("%s%04lx:%s\n", lvl, first & 0xffff, _str);
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
	set_fs(fs);
	uaccess_disable();
#endif
}

void dump_regs_refs(const char *lvl, struct pt_regs *reg)
{
	int i = 0;
	char s1[32];

	for(i = 0; i < 31; i++) {
		sprintf(s1, "x%d:", i);
		dump_mem2(KERN_EMERG, s1, reg->regs[i] - 256,
			reg->regs[i] + 256, reg->regs[i], user_mode(reg));
	}
}

static int rtk_dump_mem(struct notifier_block *nb, unsigned long val,
			    void *args)
{
	struct pt_regs *regs = (struct pt_regs *)((struct die_args *)args)->regs;

	DDR_scan_set_error(DDR_SCAN_STATUS_SCPU);

	if(console_loglevel < 2)
		console_loglevel = 2;

	if (!user_mode(regs) || in_interrupt()) {
		dump_regs_refs(KERN_EMERG, regs);
		dump_mem2(KERN_EMERG, "Stack: ", regs->sp - 512,
			regs->sp + 512, regs->sp, user_mode(regs));
		dump_mem2(KERN_EMERG, "pc: ", regs->pc-256,
			regs->pc + 256, regs->pc, user_mode(regs));

	}
	return NOTIFY_DONE;
}

static int rtk_panic(struct notifier_block *self, unsigned long v, void *p)
{
	DDR_scan_set_error(DDR_SCAN_STATUS_SCPU);

	if(console_loglevel == 0) {
		console_loglevel = 7;

		pr_emerg("Kernel panic - not syncing: %s\n", p);
#ifdef CONFIG_DEBUG_BUGVERBOSE
		/*
	 	* Avoid nested stack-dumping if a panic occurs during oops processing
	 	*/
		if (oops_in_progress <= 1)
			dump_stack();
#endif
		console_loglevel = 0;
	}

	return 0;
}

#endif

#define WDOG_NMI_IRQ_NUM    31
static __init int rtk_wdog_init_module(void)
{
	int result = -1;
	dev_t dev = 0;

	rtd_pr_misc_info( " ***************** rtk_wdog_init_module init module ********************* \n");

#ifdef MODULE
	get_bootparam_wdt_set();
#endif     

	if (wdog_major) {
		dev = MKDEV(wdog_major, wdog_minor);
		result = register_chrdev_region(dev, 1, WDOG_DEVICE_NAME);
	} else {
		result = alloc_chrdev_region(&dev, wdog_minor, 1, WDOG_DEVICE_NAME);
		wdog_major = MAJOR(dev);
	}
	if (result < 0) {
		rtd_pr_misc_err("wdog: can't get major %d\n", wdog_major);
		return result;
	}
	rtd_pr_misc_info("wdog:get result:%d,get dev:0x%08x,major:%d\n", result, dev, wdog_major);

	wdog_class = class_create(THIS_MODULE, WDOG_DEVICE_NAME);
	if (IS_ERR(wdog_class))
		return PTR_ERR(wdog_class);

	wdog_drv_dev = kmalloc(sizeof(struct wdog_dev), GFP_KERNEL);
	if (!wdog_drv_dev) {
		device_destroy(wdog_class, dev);
		result = -ENOMEM;
		rtd_pr_misc_err("wdog: alloc memory Failed, result = %d\n", result);
		return result;
	}
	memset(wdog_drv_dev, 0, sizeof(struct wdog_dev));

	cdev_init(&wdog_drv_dev->cdev, &wdog_fops);
	wdog_drv_dev->cdev.owner = THIS_MODULE;
	wdog_drv_dev->cdev.ops = &wdog_fops;

	result = cdev_add(&wdog_drv_dev->cdev, dev, 1);
	if (result) {
		device_destroy(wdog_class, dev);
		rtd_pr_misc_err("wdog: add cdev Failed, result = %d\n", result);
		goto fail;
	}

	device_create(wdog_class, NULL, dev, NULL, WDOG_DEVICE_NAME);

	result = platform_driver_register(&watchdog_driver);
	if (result) 
	{
		device_destroy(wdog_class, dev);
		rtd_pr_misc_err("rtk_wdog_init_module : can not register platform driver, ret=%d\n", result);
		goto fail;
	}

#ifdef CONFIG_ARM64
	register_die_notifier(&rtk_dump_mem_nb);
	atomic_notifier_chain_register(&panic_notifier_list,
				       &rtk_panic_nb);
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	kthread_run (hw_watchdog, NULL, "hw_watchdog_thread%d", 0);
#endif
	register_pm_notifier(&watchdog_pm_notifier);

	return 0;
fail:
	kfree(wdog_drv_dev);
	wdog_drv_dev = NULL;
	return result;
}

static __exit void rtk_wdog_cleanup_module(void)
{
	dev_t devno = MKDEV(wdog_major, wdog_minor);
	if (wdog_drv_dev) {
		cdev_del(&(wdog_drv_dev->cdev));
		device_destroy(wdog_class, MKDEV(wdog_major, wdog_minor));
		kfree(wdog_drv_dev);
	}
	class_destroy(wdog_class);
	unregister_chrdev_region(devno, 1);	
	platform_driver_unregister(&watchdog_driver);
	unregister_pm_notifier(&watchdog_pm_notifier);
#ifdef CONFIG_ARM64
	unregister_die_notifier(&rtk_dump_mem_nb);
	atomic_notifier_chain_unregister(&panic_notifier_list,
				       &rtk_panic_nb);
#endif
}

module_init(rtk_wdog_init_module);
module_exit(rtk_wdog_cleanup_module);
MODULE_AUTHOR("Realtek.com");
MODULE_LICENSE("GPL");

