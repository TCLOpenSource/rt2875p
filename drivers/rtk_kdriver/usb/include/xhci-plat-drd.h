#ifndef __XCHI_PLAT_DRD_H__
#define __XCHI_PLAT_DRD_H__
#include <rtd_log/rtd_module_log.h>
#include "xhci-plat-config.h"
struct xhci_drd_priv
{
        unsigned int usb2_port_cnt;
        unsigned int usb3_port_cnt;
        struct xhci_global_regs *global_regs;
        struct XHCI_WRAPPER_REGS wrapper_regs;
        /********* RTK defined *********/
};

struct rtk_xhci_drd
{
        struct device           *dev;
        struct xhci_drd_priv  priv;
};

#define drd_to_xhci_drd_priv(dwc)  (&(((struct rtk_xhci_drd *)(dwc))->priv))

#define xhci_drd_dbg(fmt, args...)          rtd_pr_xhci_drd_debug(fmt, ## args)
#define xhci_drd_info(fmt, args...)         rtd_pr_xhci_drd_info(fmt, ## args)
#define xhci_drd_warn(fmt, args...)         rtd_pr_xhci_drd_warn(fmt, ## args)
#define xhci_drd_err(fmt, args...)          rtd_pr_xhci_drd_err(fmt, ## args)

#endif /* __DRIVERS_USB_DWC3_CORE_H */
