#ifndef __RTK_EHCI_PLAT_H__
#define __RTK_EHCI_PLAT_H__
#include <linux/io.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <rtd_log/rtd_module_log.h>
#include "ehci.h"
#include "ehci-plat-hw-wrapper.h"

/* platform ID */
#define ID_EHCI_TOP      0
#define ID_EHCI_EX_TOP   1

#define ehci_plat_dbg(fmt, args...)           rtd_pr_ehci_plat_debug(fmt, ## args)
#define ehci_plat_info(fmt, args...)          rtd_pr_ehci_plat_info(fmt, ## args)
#define ehci_plat_warn(fmt, args...)          rtd_pr_ehci_plat_warn(fmt, ## args)
#define ehci_plat_err(fmt, args...)          rtd_pr_ehci_plat_err(fmt, ## args)


/* debug control flag */
#define NO_EHCI_POLLING_CHECK  (0x1)


typedef struct
{
        unsigned char page;
        unsigned char addr;
} U2_PHY_REG;


typedef struct
{
        int port1;
        unsigned char addr;
        unsigned char val;
} U2_PHY_REGISTER;


struct vstatus_reg
{
        int id;
        unsigned long regs[EHCI_MAX_ROOT_PORTS];
};


struct ehci_completion
{
        int id;
        struct completion phy_mac_completion;
};


#define EHCI_MAX_CLKS 3

struct ehci_platform_priv
{
        struct clk *clks[EHCI_MAX_CLKS];
        struct reset_control *rst;
        struct phy **phys;
        int num_phys;
        bool reset_on_resume;
        /********* RTK defined *********/
        unsigned int usb2_port_cnt;
        int id;
        unsigned long *vstatus_regs;
        struct ehci_synopsys_regs __iomem *synop_spec_regs;
        struct timer_list polling_check_timer;
        bool is_timer_on;
        bool polling_check_running;
        unsigned debug_ctrl_flag;
        struct completion *phy_mac_completion;
        struct list_head ehci_list;
        struct EHCI_WRAPPER_REGS wrapper_regs;
        struct ehci_hcd *ehci;
        /********* RTK defined *********/
            /********* RTK defined *********/
        /* For KINGMAX storage device
	 * or some device that are out of spec,
	 * and need us to adjust DSM */
        //void (*down_shift_DSM)(struct usb_device *udev);

        /* for port test mode used (rtk-hack) */
        struct usb_device *udev_in_test;	
};

#define hcd_to_ehci_priv(h) ((struct ehci_platform_priv *)hcd_to_ehci(h)->priv)
#define ehci_to_ehci_priv(ehci)  ((struct ehci_platform_priv *)ehci->priv)

static inline struct ehci_hcd *ehci_priv_to_ehci(struct ehci_platform_priv *priv)
{
        return container_of((void *)priv, struct ehci_hcd, priv);
}


#define EHCI_POLLING_CHECK_TIME     500
extern void start_ehci_polling_check_timer(struct ehci_hcd *ehci);
extern void stop_ehci_polling_check_timer(struct ehci_hcd *ehci);

extern ssize_t _get_ehci_portsc_str(struct ehci_hcd *ehci, char *buf, ssize_t size);

extern const U2_PHY_REG u2_phy_reg_table[];
extern unsigned long *id_get_ehci_vstatus_regs(struct ehci_platform_priv *priv);
extern struct completion *id_get_ehci_completion(struct ehci_platform_priv *priv);
int ehci_usb2_phy_init(struct platform_device *pdev);
extern int get_ehci_usb2_phy_reg(struct ehci_hcd *ehci, unsigned char port1, unsigned char page, unsigned char addr);
extern int set_ehci_usb2_phy_reg(struct ehci_hcd *ehci, unsigned char port1, unsigned char page, unsigned char addr, unsigned char val);

extern void ehci_init_wrapper_regs(struct usb_hcd *hcd, unsigned int reg_start);

/* CRC related func */
extern int enble_usb2_wrap_crc(struct ehci_hcd *ehci, bool on);
extern int reset_usb2_wrap_crc(struct ehci_hcd *ehci);
extern int get_usb2_wrap_crc_err_cnt(struct ehci_hcd *ehci, int port1);
extern int get_usb2_wrap_crc_en(struct ehci_hcd *ehci) ;
extern int get_usb2_wrap_crc_rst(struct ehci_hcd *ehci) ;

#define Z0_GEAR(val)   (3+((val&0xf)*4))
#define Z0_EHCI_PHY_NAME        "Z0_EHCI_VALUE"

#endif
