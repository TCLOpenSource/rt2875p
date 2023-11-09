/*
 * xhci-plat.h - xHCI host controller driver platform Bus Glue.
 *
 * Copyright (C) 2015 Renesas Electronics Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

/*
 * Copy from drivers/usb/host/xhci-plat.h
 */

#ifndef _XHCI_PLAT_H
#define _XHCI_PLAT_H
#include <rtd_log/rtd_module_log.h>
#include "xhci.h"
#include "xhci-plat-config.h"
#include "xhci-plat-hw-wrapper.h"
/*------------------------------ Realtek defined -------------------------------------------*/


#define HOST_GLOBAL_REGS_OFFSET         (0x8100)

#define NO_RESTORE_U3_PHY           (0x1 << 0)
#define NO_RESTORE_U2_PHY           (0x1 << 1)
#define NO_U3_PORT_AUTO_RECOVER     (0x1 << 2)
#define NO_U3_SUPER_SPEED_PORT      (0x1 << 3)
#define NO_HUB_POLLING_CHECK        (0x1 << 4)
extern unsigned int xhci_control_flag;

/* hub polling check */
#define USB_HUB_CHECK_TIME           500
extern struct timer_list xhci_hub_check_timer;

/* CRC.
 * Merlin3_DesignSpec-USB3
 * REGISTER:: USB_CRC_CNT_1
 * */
#define CRC_SRC_NONE                0x0
#define CRC_SRC_U3_PORT             0x1
#define CRC_SRC_U2_PORT0            0x4
#define CRC_SRC_U2_PORT1            0x5
#define CRC_SRC_U2_PORT2            0x6


/* Functions */
int _xhci_get_port_status_str(struct xhci_hcd *xhci, char* ptr, size_t count);
const char* xhci_crc_error_src(unsigned char id);

void xhci_usb3_mdio_write(struct xhci_hcd *xhci, unsigned char port, unsigned char phy_addr, unsigned char reg_addr, unsigned short val);
unsigned short xhci_usb3_mdio_read(struct xhci_hcd *xhci, unsigned char port, unsigned char phy_addr, unsigned char reg_addr);
void xhci_init_wrapper_regs(struct usb_hcd *hcd, unsigned int reg_start);
int xhci_usb2_phy_init(struct platform_device *pdev);
int xhci_usb3_phy_init(struct platform_device *pdev);
int xhci_usb2_set_phy_from_otp(struct platform_device *pdev);
int set_xhci_usb2_phy_reg(struct xhci_hcd *xhci, unsigned char phy, unsigned char page, unsigned char addr,unsigned char val);
int get_xhci_usb2_phy_reg(struct xhci_hcd *xhci, unsigned char phy, unsigned char page, unsigned char addr);
int xhci_usb2_set_swing(struct platform_device *pdev, unsigned int swing_val);


#define xhci_plat_dbg(fmt, args...)          rtd_pr_xhci_plat_debug(fmt, ## args)
#define xhci_plat_info(fmt, args...)         rtd_pr_xhci_plat_info(fmt, ## args)
#define xhci_plat_warn(fmt, args...)        rtd_pr_xhci_plat_warn(fmt, ## args)
#define xhci_plat_err(fmt, args...)          rtd_pr_xhci_plat_err(fmt, ## args)
/*------------------------------ Realtek defined -------------------------------------------*/

struct xhci_plat_priv
{
        const char *firmware_name;
        void (*plat_start)(struct usb_hcd *);
        int (*init_quirk)(struct usb_hcd *);
        int (*resume_quirk)(struct usb_hcd *);
        /********* RTK defined *********/
        unsigned int usb2_port_cnt;
        unsigned int usb3_port_cnt;
        struct xhci_global_regs __iomem *global_regs;
        struct timer_list polling_check_timer;
        bool is_timer_on;
        bool polling_check_running;
        bool polling_check_suspend;
        bool disable_polling_check;
        unsigned int u2_port_reset_hang;
        unsigned int u2_port_reset_hang_count;
        unsigned int u3_port_reset_hang;
        unsigned int u3_port_reset_hang_count;
        unsigned int port_check_count;
        unsigned int black_device_count;
        unsigned int u3_port_count;
        bool u3_delayed_vbus_on;
        unsigned int debug_ctrl_flag;
        struct delayed_work power_saving_dwork;
        struct XHCI_WRAPPER_REGS wrapper_regs;
        /********* RTK defined *********/
        /* For KINGMAX storage device
	 * or some device that are out of spec,
	 * and need us to adjust DSM */
        //void (*down_shift_DSM)(struct usb_device *udev);

        /* for port test mode used (rtk-hack) */
        struct usb_device *udev_in_test;	
};


#define hcd_to_xhci_priv(h)  ((struct xhci_plat_priv *)hcd_to_xhci(h)->priv)
#define xhci_to_xhci_priv(h) ((struct xhci_plat_priv *)xhci->priv)
#endif  /* _XHCI_PLAT_H */
