#ifndef __SCALER_VFEDEV_DPRX_H__
#define __SCALER_VFEDEV_DPRX_H__

#ifdef CONFIG_SUPPORT_SRC_DPRX

extern long vfe_dprx_ioctl(struct file *file, unsigned int cmd,  unsigned long arg);

#define DPRX_VFE_DBG(fmt, args ...)         pr_debug("[VFE][DPRX][DBG]" fmt, ## args)
#define DPRX_VFE_INFO(fmt, args ...)        pr_info("[VFE][DPRX][INFO]" fmt, ## args)
#define DPRX_VFE_WARN(fmt, args ...)        pr_warn("[VFE][DPRX][WARN]" fmt, ## args)
#define DPRX_VFE_ERR(fmt, args ...)         pr_err("[VFE][DPRX][ERR]" fmt, ## args)

#else

#define vfe_dprx_ioctl(file, cmd, arg)     (-ENOTTY)
#endif

#endif // __SCALER_VFEDEV_DPRX_H__
