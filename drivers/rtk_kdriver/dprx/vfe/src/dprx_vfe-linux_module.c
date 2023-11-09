/*=============================================================
 * File:    dprx_vfe_sysfs.c
 *
 * Desc:    DPRX VFE API
 *
 * AUTHOR:  kevin_wang@realtek.com
 *
 * Vresion: 0.0.1
 *
 *------------------------------------------------------------
 * Copyright (c) Realtek Semiconductor Corporation, 2021
 *
 * All rights reserved.
 *
 *============================================================*/
#include <linux/syscalls.h>
#include <linux/miscdevice.h>
#include <linux/module.h>


// Module RTK DPRX Adapter
#ifdef CONFIG_DPRX_DRV_ENABLE_RTK_DPRX_ADAPTER
extern int  rtk_dprx_module_init(void);
extern void rtk_dprx_module_exit(void);
#else
#define rtk_dprx_module_init()
#define rtk_dprx_module_exit()
#endif

// Module function for sysfs
#ifdef CONFIG_RTK_DPRX_SYSFS
extern int  dprx_sysfs_init(void);
extern void dprx_syfss_exit(void);
#else
#define dprx_sysfs_init()
#define dprx_syfss_exit()
#endif

 /*------------------------------------------------
 * Func : dprx_module_exit
 *
 * Desc : dprx module init function
 *
 * Para : N/A
 *
 * Retn : N/A
 *-----------------------------------------------*/
int __init dprx_module_init(void)
{
    rtk_dprx_module_init();
    dprx_sysfs_init();
    return 0;
}

/*------------------------------------------------
 * Func : dprx_module_exit
 *
 * Desc : dprx module exit function
 *
 * Para : N/A
 *
 * Retn : N/A
 *-----------------------------------------------*/
static void __exit dprx_module_exit(void)
{
    dprx_syfss_exit();
    rtk_dprx_module_exit();
}

module_init(dprx_module_init);
module_exit(dprx_module_exit);
MODULE_LICENSE("GPL");
