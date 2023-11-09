#include <generated/autoconf.h>
#include <linux/kernel.h>
#include <linux/cpu_pm.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/spinlock.h>
#include <linux/syscore_ops.h>
#include <rtk_kdriver/io.h>
#include <rbus/iso_misc_reg.h>
#include <rbus/pinmux_reg.h>
#include <rbus/pinmux_lvdsphy_reg.h>
//#include <rtd_log/rtd_module_log.h>

#if IS_ENABLED(CONFIG_RTK_KDRV_QOS_CONTROL)
#include <rtk_kdriver/rtk_qos_export.h>
#endif

#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
extern void store_sb2_status(void);
extern void restore_sb2_status(void);
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
extern void dc_mt_resume(void);
#endif

#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
extern void vbe_disp_first_resume(void);
#endif

#if IS_ENABLED(CONFIG_REALTEK_ARM_WRAPPER)
void arm_wrapper_suspend (void);
void arm_wrapper_resume (void);
#else
static inline void arm_wrapper_suspend (void){}
static inline void arm_wrapper_resume (void){}
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_COMMON)
extern void rtk_timer_init(void);
#endif

#define PINMUX_MODULE_NAME "Pinmux"

static unsigned int uart2rbus_status = 0;

extern void pinmux_suspend_store_range(void);
extern void pinmux_resume_reload_range(void);

static int pinmux_suspend(void)
{

	pinmux_suspend_store_range();

#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
	store_sb2_status();
#endif
    arm_wrapper_suspend();

#if IS_ENABLED(CONFIG_RTK_KDRV_QOS_CONTROL)
	rtk_qos_suspend();
#endif

	// uart2rbus status store.
	uart2rbus_status = rtd_inl(ISO_MISC_DDC_CTRL_reg);
//    rtd_pr_misc_notice("[%s] do %s finish\n", PINMUX_MODULE_NAME, __func__);

    return 0;
}

static void pinmux_resume (void)
{
	pinmux_resume_reload_range();

	// uart2rbus status reload.
	rtd_outl(ISO_MISC_DDC_CTRL_reg, uart2rbus_status);

#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
	dc_mt_resume();
#endif

#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
	restore_sb2_status();
#endif

#if IS_ENABLED(CONFIG_REALTEK_ARM_WRAPPER)
	arm_wrapper_resume();
#endif

#if IS_ENABLED(CONFIG_RTK_KDRV_QOS_CONTROL)
	rtk_qos_resume();
#endif

#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
	vbe_disp_first_resume();
#endif

#if IS_ENABLED(CONFIG_RTK_KDRV_COMMON)
	rtk_timer_init();
#endif

//    rtd_pr_misc_notice("[%s] do %s finish\n", PINMUX_MODULE_NAME, __func__);
}


static struct syscore_ops pinmux_syscore_ops =
{
    .suspend = pinmux_suspend,
    .resume = pinmux_resume,
};

static int pinmux_init(void)
{
    register_syscore_ops(&pinmux_syscore_ops);
    return 0;
}
core_initcall(pinmux_init);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Realtek.com");
