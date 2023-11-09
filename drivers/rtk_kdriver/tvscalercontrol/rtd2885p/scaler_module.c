//Kernel Header file
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/fs.h>       /* everything... */
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/sched/rt.h>
#include <linux/kthread.h>
#include <linux/string.h>

// HDCP
extern int hdcp2_init_module(void);
extern void __exit hdcp2_cleanup_module(void);

// HDMI
#if IS_ENABLED(CONFIG_RTK_HDMI_RX)
extern int __init hdmi_module_init(void);
extern void __exit hdmi_module_exit(void);
extern int __init  hdmi_hw_init(void);
extern void __exit hdmi_hw_exit(void);
#else
#define hdmi_hw_init()
#define hdmi_hw_exit()
#define hdmi_module_init()
#define hdmi_module_exit()
#endif

// VFE
extern int vfe_module_init(void);
extern void __exit vfe_module_exit(void);

// VSC
extern int vsc_init_module(void);
extern void __exit vsc_cleanup_module(void);

// VO
extern int vo_module_init(void);
extern void vo_module_exit(void);

// VDC
extern int __init video_module_init(void);
extern void __exit video_module_exit(void);

// VT
extern int vivt_init_module(void);
extern void __exit vivt_cleanup_module(void);

// VBE
extern int vbe_module_init(void);
extern void __exit vbe_module_exit(void);

// VPQ
extern int vpq_module_init(void);
extern void __exit vpq_module_exit(void);


extern int vpqex_module_init(void);
extern void __exit vpqex_module_exit(void);


#ifndef CONFIG_MEMC_NOTSUPPORT 
extern int __init memc_isr_init_module(void);
extern void memc_isr_exit_module(void);
#endif

#ifndef CONFIG_ARM64 //ARM32 compatible
extern int vpqut_module_init(void);
extern void __exit vpqut_module_exit(void);
#else
#define vpqut_module_init()
#define vpqut_module_exit()
#endif

extern int vpq_memc_module_init(void);
extern void __exit vpq_memc_module_exit(void);

extern int vpq_led_module_init(void);
extern void __exit vpq_led_module_exit(void);

// DDomain
extern int __init ddomain_isr_init_module(void);
extern void __exit ddomain_isr_exit_module(void);

// VGIP
extern int __init vgip_isr_init_module(void);
extern void __exit vgip_isr_exit_module(void);

// Dolby Vision
extern int dolby_device_init(void);
extern void dolby_device_exit(void);

//vivr
extern int vivr_init_module(void);
extern void __exit vivr_cleanup_module(void);


/////////////////////////////////////////////////////////////////////////
// Module Function
/////////////////////////////////////////////////////////////////////////

int __init scaler_module_init(void)
{
    // Init all submodule over here
    video_module_init(); // vdc
    hdmi_hw_init();  // HDMI
    hdmi_module_init();//HDMI

    vfe_module_init();
    vsc_init_module();
    vo_module_init();
    vivt_init_module();
    vbe_module_init();

    // VPQ
    vpq_module_init();
    vpq_memc_module_init();
    vpqex_module_init();
    vpqut_module_init();
    vpq_led_module_init();

#ifndef CONFIG_MEMC_NOTSUPPORT
    memc_isr_init_module();
#endif

#ifdef CONFIG_RTK_KDRV_DV
    dolby_device_init();
#endif
    // HDCP2
    hdcp2_init_module();

    // DDomain
    ddomain_isr_init_module();

    // VGIP
    vgip_isr_init_module();

    //vivr
    vivr_init_module();
    return 0;
}

// exit point of scaler module

void __exit scaler_module_exit(void)
{
    // deinit all submodule over here
    video_module_exit();  // vdc
    hdmi_hw_exit();  // HDMI
    hdmi_module_exit();// HDMI
    vfe_module_exit();
    vsc_cleanup_module();
    vo_module_exit();
    vivt_cleanup_module();
    vbe_module_exit();

    // VPQ
    vpqut_module_exit();
    vpq_memc_module_exit();
    vpqex_module_exit();
    vpq_module_exit();
    vpq_led_module_exit();

#ifndef CONFIG_MEMC_NOTSUPPORT
    memc_isr_exit_module();
#endif

#ifdef CONFIG_RTK_KDRV_DV
    dolby_device_exit();
#endif

    // HDCP2
    hdcp2_cleanup_module();

    // DDomain
    ddomain_isr_exit_module();

    // VGIP
    vgip_isr_exit_module();

    //vivr
    vivr_cleanup_module();
}

module_init(scaler_module_init);
module_exit(scaler_module_exit);
MODULE_LICENSE("GPL");
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);


