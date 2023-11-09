#include <generated/autoconf.h>
#include <linux/kernel.h>
#include <linux/cpu_pm.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/spinlock.h>
#include <linux/syscore_ops.h>
#include <io.h>
#include <rbus/iso_misc_reg.h>
//#include <rtd_log/rtd_module_log.h>

#ifdef CONFIG_RTK_KDRV_QOS_CONTROL
#include <rtk_kdriver/rtk_qos_export.h>
#endif

#ifdef CONFIG_RTK_KDRV_SB2
extern void store_sb2_status(void);
extern void restore_sb2_status(void);
#endif
#ifdef CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER
extern void dc_mt_resume(void);
#endif

#ifdef CONFIG_SUPPORT_SCALER
extern void vbe_disp_first_resume(void);
#endif

#ifdef CONFIG_REALTEK_ARM_WRAPPER
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

// pinmux - main
#define PINMUX_MAIN_1_BEGIN 0xb8000800
#define PINMUX_MAIN_1_END 0xb80008f4

// pinmux - pad_lvdsphy
#define PINMUX_MAIN_2_BEGIN 0xb8000ca0
#define PINMUX_MAIN_2_END 0xb8000cc0

// pinmux - pmm
#define PINMUX_ISO_BEGIN 0xb8060200
#define PINMUX_ISO_END 0xb8060244

static unsigned int pinmux_buffer[((PINMUX_MAIN_1_END - PINMUX_MAIN_1_BEGIN) +
									(PINMUX_MAIN_2_END - PINMUX_MAIN_2_BEGIN) +
									(PINMUX_ISO_END - PINMUX_ISO_BEGIN))/4];
static unsigned int uart2rbus_status = 0;
static int pinmux_suspend_store(unsigned int *idx, unsigned int register_begin, unsigned int register_end)
{
    while(register_begin < register_end)
    {
        pinmux_buffer[(*idx)] = rtd_inl(register_begin);
        register_begin += 4;
        (*idx)++;
    }

	return 0;
}

static int pinmux_resume_reload(unsigned int *idx, unsigned int register_begin, unsigned int register_end)
{
    while(register_begin < register_end)
    {
        rtd_outl(register_begin, pinmux_buffer[(*idx)]);
        register_begin += 4;
        (*idx)++;
    }

	return 0;
}

static int pinmux_suspend(void)
{
    unsigned int idx = 0;

//    rtd_pr_misc_notice("[%s] do %s\n", PINMUX_MODULE_NAME, __func__);

	pinmux_suspend_store(&idx, PINMUX_MAIN_1_BEGIN, PINMUX_MAIN_1_END); // don't change this sequence
	pinmux_suspend_store(&idx, PINMUX_MAIN_2_BEGIN, PINMUX_MAIN_2_END); // don't change this sequence
	pinmux_suspend_store(&idx, PINMUX_ISO_BEGIN, PINMUX_ISO_END); // don't change this sequence

#ifdef CONFIG_RTK_KDRV_SB2
	store_sb2_status();
#endif
    arm_wrapper_suspend();

#ifdef CONFIG_RTK_KDRV_QOS_CONTROL
	rtk_qos_suspend();
#endif

	// uart2rbus status store.
	uart2rbus_status = rtd_inl(ISO_MISC_DDC_CTRL_reg);
//    rtd_pr_misc_notice("[%s] do %s finish\n", PINMUX_MODULE_NAME, __func__);

    return 0;
}

static void pinmux_resume (void)
{
    unsigned int idx = 0;

//    rtd_pr_misc_notice("[%s] do %s\n", PINMUX_MODULE_NAME, __func__);

	pinmux_resume_reload(&idx, PINMUX_MAIN_1_BEGIN, PINMUX_MAIN_1_END); // don't change this sequence
	pinmux_resume_reload(&idx, PINMUX_MAIN_2_BEGIN, PINMUX_MAIN_2_END); // don't change this sequence
	pinmux_resume_reload(&idx, PINMUX_ISO_BEGIN, PINMUX_ISO_END); // don't change this sequence

	// uart2rbus status reload.
	rtd_outl(ISO_MISC_DDC_CTRL_reg, uart2rbus_status);

#ifdef CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER
	dc_mt_resume();
#endif

#ifdef CONFIG_RTK_KDRV_SB2
	restore_sb2_status();
#endif

#ifdef CONFIG_REALTEK_ARM_WRAPPER
	arm_wrapper_resume();
#endif

#ifdef CONFIG_RTK_KDRV_QOS_CONTROL
	rtk_qos_resume();
#endif

#ifdef CONFIG_SUPPORT_SCALER
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
