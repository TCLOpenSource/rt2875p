#ifndef __SCALER_VFEDEV_DPRX_H__
#define __SCALER_VFEDEV_DPRX_H__

#include <linux/fs.h>
#include <linux/printk.h>
#include <rtd_log/rtd_module_log.h>

#ifdef CONFIG_SUPPORT_SRC_DPRX

extern unsigned char    DPRX_Global_Status;
extern unsigned short   DPRX_Input_Source;
extern struct semaphore DPRX_DetectSemaphore;

#define set_DPRX_Global_Status(status)      (DPRX_Global_Status = status)
#define get_DPRX_Global_Status(status)      (DPRX_Global_Status)

extern int  vfe_dprx_init(void);
extern long vfe_dprx_ioctl(struct file *file, unsigned int cmd,  unsigned long arg);
extern void vfe_dprx_set_global_status(SOURCE_STATUS status);

extern void Set_Dprx_Reply_Zero_Timing_Flag(unsigned char flag);

#define DPRX_VFE_DBG(fmt, args ...)         rtd_pr_vfe_debug("[DPRX]" fmt, ## args)
#define DPRX_VFE_INFO(fmt, args ...)        rtd_pr_vfe_info("[DPRX]" fmt, ## args)
#define DPRX_VFE_WARN(fmt, args ...)        rtd_pr_vfe_warn("[DPRX]" fmt, ## args)
#define DPRX_VFE_ERR(fmt, args ...)         rtd_pr_vfe_err("[DPRX]" fmt, ## args)

#else
#define vfe_dprx_init()                    (-1)
#define vfe_dprx_ioctl(file, cmd, arg)     (-ENOTTY)
#define vfe_dprx_set_global_status(status)
#endif

#endif // __SCALER_VFEDEV_DPRX_H__
