// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * PCIe host controller driver for Realtek TV SoC
 *
 * Copyright (C) 2021 Realtek Corporation.
 *
 */

#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/of_pci.h>
#include <linux/pci.h>
#include <linux/phy/phy.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/random.h>
#include <linux/reset.h>
#include <linux/resource.h>
#include <linux/types.h>

#include <rtk_kdriver/io.h>
#include <rbus/pcie_wrap_reg.h>
#include <rbus/gpio_reg.h>
#include <rbus/sb2_reg.h>
#include <rtk_kdriver/rtk_gpio.h>
#include <pcbMgr.h>
#include "../../pci/controller/dwc/pcie-designware.h"
#include "pcie-rtk_tv.h"
#include <mach/rtk_platform.h>
#include <mach/platform.h>

/*
for Version B fix
    1. config byte access
    2. memory byte access
    3. EP memory 64MB access limitation
*/

//#define REASSIGN_ID

#define PCIE_RTK_VID        (0x10ECUL )
#define PCIE_RTK_PID        (0x2001UL )

#define PCIE_MEMORY_SPACE   (0x78000000 )
#define RTK_PCIE_DMA
#define RTK_PCIE_DMA_ADDR   (PCIE_MEMORY_SPACE+0x600000)
#define RTK_PCIE_DMA_SIZE   (2*1024*1024)

struct device *rtk_pcie_dev = NULL;
struct dw_pcie *pci_local = NULL;
u32 pcie_enable;
static u32 pcie_link_speed = GEN2;
u32 pcie_rw_req_dealy = 0;
u32 pcie_disable_rbus_timeout = 1;
static DEFINE_SPINLOCK(rtk_pcie_send_req_lock);

static int rtk_pcie_set_link_speed(struct dw_pcie *pci, e_pciegen speed, u32 initiate_speed_change);
static void rtk_pcie_hold_perst(void);
static void rtk_pcie_release_perst(void);
static void rtk_pcie_toggle_perst(void);

struct rtk_pcie_dw {
    struct device *dev;

    struct resource *xali_res;
    struct resource *dbi_res;
    struct resource *wrap_res;
    void __iomem *xali_base;
    void __iomem *wrap_base;

    struct clk *core_clk;
    struct reset_control *core_apb_rst;
    struct reset_control *core_rst;
    struct dw_pcie pci;
    struct rtk_bpmp *bpmp;

    bool supports_clkreq;
    bool enable_cdm_check;
    bool link_state;
    bool update_fc_fixup;
    u8 init_link_width;
    u32 msi_ctrl_int;
    u32 num_lanes;
    u32 max_speed;
    u32 cid;
    u32 cfg_link_cap_l1sub;
    u32 pcie_cap_base;
    u32 aspm_cmrt;
    u32 aspm_pwr_on_t;
    u32 aspm_l0s_enter_lat;

    struct regulator *pex_ctl_supply;
    struct regulator *slot_ctl_3v3;
    struct regulator *slot_ctl_12v;

    unsigned int phy_count;
    struct phy **phys;

    u32 ic_ver;
    struct dentry *debugfs;
};


static inline struct rtk_pcie_dw *to_rtk_pcie(struct dw_pcie *pci)
{
    return container_of(pci, struct rtk_pcie_dw, pci);
}


static __maybe_unused
u32 pci_bit_mask(u8 byte_mask)
{
	int i;
	u32 mask = 0;

	for (i = 0; i < 4; i++) {
		if ((byte_mask >> i) & 0x1)
			mask |= (0xFF << (i << 3));
	}

	return mask;
}

static __maybe_unused
u32 pci_bit_shift(u32 addr)
{
	return ((addr & 0x3) << 3);
}

static __maybe_unused
u32 pci_byte_mask(u32 addr, u8 size)
{
	u8 offset = (addr & 0x03);

	switch (size) {
	case 0x01:
		return 0x1 << offset;
	case 0x02:
		if (offset <= 2)
			return 0x3 << offset;

		rtd_pr_pcie_alert("compute config mask - data cross dword boundrary (addr=0x%x, length=2)\n", addr);
		break;

	case 0x03:
		if (offset <= 1)
			return 0x7 << offset;

		rtd_pr_pcie_alert("compute config mask - data cross dword boundrary (addr=0x%x, length=3)\n",  addr);
		break;

	case 0x04:
		if (offset == 0)
			return 0xF;

		rtd_pr_pcie_alert("compute config mask - data cross dword boundrary (addr=0x%x, length=4)\n", addr);
		break;

	default:
		rtd_pr_pcie_alert("compute config mask failed - size %d should between 1~4\n", size);
	}
	return 0;
}

static __maybe_unused
u32 _pci_address_conversion(struct pci_bus *bus,
						unsigned int devfn, int reg)
{
	int busno = bus->number;
	int dev = PCI_SLOT(devfn);
	int func = PCI_FUNC(devfn);

	return (busno << 24) | (dev << 19) | (func << 16) | reg;
}

static int rtk_pcie_int_crt_enable(u32 enable)
{
    u32 en_val;

    en_val = (enable)?1:0;

    rtd_outl(SYS_REG_INT_CTRL_SCPU_2_reg,
        SYS_REG_INT_CTRL_SCPU_2_pcie_int_scpu_routing_en(1)|
        SYS_REG_INT_CTRL_SCPU_2_write_data(en_val));

    return 0;
}


static __maybe_unused
int rtk_pcie_int_wrap_enable(u32 enable)
{
    u32 en_val;

    en_val = (enable)?1:0;

    rtd_outl(PCIE_WRAP_PCIE_SII_INT_CTRL1_reg,
        PCIE_WRAP_PCIE_SII_INT_CTRL1_cfg_bw_mgt_msi_en(en_val) |
        PCIE_WRAP_PCIE_SII_INT_CTRL1_cfg_bw_mgt_int_en(en_val) |
        PCIE_WRAP_PCIE_SII_INT_CTRL1_cfg_link_auto_bw_msi_en(en_val) |
        PCIE_WRAP_PCIE_SII_INT_CTRL1_cfg_link_auto_bw_int_en(en_val) |
        PCIE_WRAP_PCIE_SII_INT_CTRL1_hp_msi_en(en_val) |
        PCIE_WRAP_PCIE_SII_INT_CTRL1_hp_int_en(en_val) |
        PCIE_WRAP_PCIE_SII_INT_CTRL1_hp_pme_en(en_val) |
        PCIE_WRAP_PCIE_SII_INT_CTRL1_radm_intd_deasserted_en(en_val) |
        PCIE_WRAP_PCIE_SII_INT_CTRL1_radm_intc_deasserted_en(en_val) |
        PCIE_WRAP_PCIE_SII_INT_CTRL1_radm_intb_deasserted_en(en_val) |
        PCIE_WRAP_PCIE_SII_INT_CTRL1_radm_inta_deasserted_en(en_val) |
        PCIE_WRAP_PCIE_SII_INT_CTRL1_radm_intd_asserted_en(en_val) |
        PCIE_WRAP_PCIE_SII_INT_CTRL1_radm_intc_asserted_en(en_val) |
        PCIE_WRAP_PCIE_SII_INT_CTRL1_radm_intb_asserted_en(en_val) |
        PCIE_WRAP_PCIE_SII_INT_CTRL1_radm_inta_asserted_en(en_val) |
        PCIE_WRAP_PCIE_SII_INT_CTRL1_cfg_pme_msi_en(en_val) |
        PCIE_WRAP_PCIE_SII_INT_CTRL1_cfg_pme_int_en(en_val) |
        PCIE_WRAP_PCIE_SII_INT_CTRL1_cfg_aer_rc_err_msi_en(en_val) |
        PCIE_WRAP_PCIE_SII_INT_CTRL1_cfg_aer_rc_err_int_en(en_val));

    return 0;
}


static void rtk_pcie_int_msi_sta_clr(void)
{
    rtd_outl(PCIE_WRAP_PCIE_INT_PENDING_STATUS1_reg,
        PCIE_WRAP_PCIE_INT_PENDING_STATUS1_int_pending(1));
    rtd_outl(PCIE_WRAP_PCIE_MSI_STATUS2_reg,
        PCIE_WRAP_PCIE_MSI_STATUS2_msi_ctrl_int(1));
}


void rtk_pcie_int_msi_sta_clr_exp(void)
{
    rtk_pcie_int_msi_sta_clr();
}
EXPORT_SYMBOL(rtk_pcie_int_msi_sta_clr_exp);

static void rtk_pcie_int_msi_enable(struct pcie_port* pp, u32 enable)
{
    u32 en_val;

    en_val = (enable)?1:0;
    if(en_val)
        dw_pcie_msi_init(pp);

    rtd_outl(PCIE_WRAP_PCIE_MSI_CTRL1_reg,
        PCIE_WRAP_PCIE_MSI_CTRL1_msi_ctrl_int_en(en_val));
}

static irqreturn_t rtk_pcie_irq_handler(int irq, void *arg)
{
    u32 reg_info;
    u32 msi_vec;

    reg_info = rtd_inl(PCIE_WRAP_PCIE_MSI_STATUS2_reg);
    if(PCIE_WRAP_PCIE_MSI_STATUS2_get_msi_ctrl_int(reg_info)){
        rtk_pcie_int_msi_sta_clr();
        msi_vec = PCIE_WRAP_PCIE_MSI_STATUS2_get_msi_ctrl_int_vec(reg_info);
        /* it's no idea how to handle msi verter now. */
        return IRQ_HANDLED;
    }

    return IRQ_NONE;
}

static void rtk_pcie_enable_interrupts(struct pcie_port *pp)
{
    struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
    __maybe_unused struct rtk_pcie_dw *pcie = to_rtk_pcie(pci);

    rtk_pcie_int_crt_enable(1);

    if (IS_ENABLED(CONFIG_PCI_MSI))
        rtk_pcie_int_msi_enable(pp,1);

    return;
}

#define PCIE_WAIT_PHY_TIME (500)
static void rtk_pcie_phy_setting(struct pcie_port *pp)
{
#define PHY_SETTING_1112
#ifdef PHY_SETTING_1112
    //// Mark2_PCIE PHY Parameter_fine tune Gen1_11/12 /////
    rtd_outl(0xb807e280,0xa852010d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x6042020d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x001d060d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xa2700a0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x0b66230d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x4f10240d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xf802280d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x1d602a0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xa8012b0d);
    udelay(PCIE_WAIT_PHY_TIME);
    //// Mark2_PCIE PHY Parameter_fine tune Gen2_11/12 /////
    rtd_outl(0xb807e280,0xa848410d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x6042420d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x001d460d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xa2704a0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x0b66630d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x4f20640d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xf802680d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x1d606a0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xa8016b0d);
    udelay(PCIE_WAIT_PHY_TIME);
#else
    //// Mark2_PCIE PHY Parameter /////
    rtd_outl(0xb807e280,0x4008000d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xa808010d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x6042020d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x2771030d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x52f5040d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xead3050d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x0003060d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x2e20070d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x3591080d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x521c090d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xb6480a0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xa9050b0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xc0000c0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xf71e0d0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x20000e0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x0d500f0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x000c100d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x4C00110d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xfc00120d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x0c81130d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xde01140d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x0000150d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x0000160d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x0000170d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x0000180d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x7c43190d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x22a21a0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x0ea01b0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x00001c0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x32001d0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x66eb1e0d);
    udelay(PCIE_WAIT_PHY_TIME);

    rtd_outl(0xb807e280,0x00001f0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xc4aa200d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x88aa210d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x0053220d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xab66230d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x4f00240d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x1260250d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xc40a260d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x41d6270d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xf882280d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xff00290d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x1d402a0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x88012b0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xffff2c0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0xffff2d0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x78002e0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x80082f0d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x0000300d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x0000310d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x521c090d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x501c090d);
    udelay(PCIE_WAIT_PHY_TIME);
    rtd_outl(0xb807e280,0x521c090d);
    udelay(PCIE_WAIT_PHY_TIME);
#endif
}

#define MDIO_WR     (0x0DUL)
#define MDIO_RD     (0x0CUL)

#define  get_mdio_data(data)    ((0xFFFF0000&(data))>>16)
#define  mdio_data(data)        (0xFFFF0000&((data)<<16))

//#define CALIBRA_DEBUG
#ifdef CALIBRA_DEBUG
#define CAL_DEBUG(fmt, args...)      \
    rtd_pr_pcie_alert(fmt , ## args)
#else
#define CAL_DEBUG(fmt, args...)
#endif

u16 read_pcie_mdio(u16 mdio_addr)
{
    u32 reg_info = 0;
    u32 data;

    reg_info = ((u32)mdio_addr << 8) | MDIO_RD;
    rtd_outl(PCIE_WRAP_PCIE_MDIO_CTRL1_reg,reg_info);
    udelay(100);

    data = rtd_inl(PCIE_WRAP_PCIE_MDIO_CTRL1_reg);
    CAL_DEBUG("R mdio [0x%08x] 0x%08x=0x%08x\n",reg_info, (u32)mdio_addr,data);

    return (u16)get_mdio_data(data);
}

void write_pcie_mdio(u16 mdio_addr, u16 data)
{
    u32 reg_info;

    reg_info = mdio_data((u32) data) | ((u32) mdio_addr << 8) | MDIO_WR;
    CAL_DEBUG("W mdio [0x%08x] 0x%08x=0x%08x\n",reg_info, (u32)mdio_addr,(u32)data);
    rtd_outl(PCIE_WRAP_PCIE_MDIO_CTRL1_reg,reg_info);
    udelay(100);
#ifdef CALIBRA_DEBUG
    read_pcie_mdio(u16 mdio_addr);  //for deug
#endif
    return;
}

void write_pcie_mdio_mask(u16 mdio_addr, u16 mask, u16 sht, u16 val)
{
    u16 reg_info;

    reg_info = read_pcie_mdio(mdio_addr);
    CAL_DEBUG("W_M read mdio 0x%08x=0x%08x\n",(u32)mdio_addr,(u32)reg_info);
    reg_info &= ~(mask << sht);
    reg_info |= (val << sht);
    CAL_DEBUG("W_M mdio 0x%08x mask=0x%08x sht=0x%08x val=0x%08x info=0x%08x\n",
        (u32)mdio_addr,(u32)mask,(u32)sht,(u32)val,(u32)reg_info);
    write_pcie_mdio(mdio_addr,reg_info);

    return;
}

u16 read_pcie_mdio_mask(u16 mdio_addr, u16 mask, u16 sht)
{
    u16 reg_info;

    reg_info = read_pcie_mdio(mdio_addr);
    CAL_DEBUG("R_M read mdio 0x%08x=0x%08x\n",(u32)mdio_addr,(u32)reg_info);
    reg_info = (reg_info >> sht) & mask;
    CAL_DEBUG("R_M mdio 0x%08x mask=0x%08x sht=0x%08x info=0x%08x\n",
        (u32)mdio_addr,(u32)mask,(u32)sht,(u32)reg_info);

    return (u16)reg_info;
}

static __maybe_unused
u32 wait_pcie_mdio_mask(u16 mdio_addr, u16 mask, u16 sht, u16 val, u32 x100us)
{
    u32 loop = x100us;
    u16 reg1;
    u16 ret = 0;

    while(1){   //waiting 0x1F/0x5F Bit[7] ==0
        reg1 = read_pcie_mdio_mask(mdio_addr, mask, sht);
        CAL_DEBUG("%s(%d)reg1=0x%x val=0x%x\n",__func__,__LINE__,reg1,val);
        if(reg1 == val ){
            ret = 1;
            break;
        }
        udelay(50);
        loop--;
        if(loop ==0 ){  //(50+50)us * loop
            CAL_DEBUG("%s(%d)wait 0x%04x mask=0x%x sht=%d val=0x%x timeout!!!\n",__func__,__LINE__);
            break;
        }
    }
    return ret;
}

#define SIMPLE_CAL_FLOW //skip OOBS refine
static void rtk_pcie_phy_calibration(struct pcie_port *pp,e_pciegen gen)
{
    u32 reg1;
    u32 offset = 0;
#ifndef SIMPLE_CAL_FLOW
    u32 oobs1;
#endif
    PCIE_ALERT("%s(%d)calibrate Gen%d\n",__func__,__LINE__,(u32)gen);
    if((gen != GEN1) && (gen != GEN2)){
        PCIE_ALERT("unsupported GEN%d, skip calibration.\n",(u32)gen);
        return;
    }

    rtk_pcie_hold_perst();   // holding device

    if(gen == GEN2)
        offset = 0x40;

    //OOBS_SEL=0x0(OOBS Auto);OOBS_Forcecal=[1]
    write_pcie_mdio(0x09 + offset,0x720c);   //0x09 = 0x720c
    //OOBS_Forcecal=[0]
    write_pcie_mdio(0x09 + offset,0x700c);   //0x09 = 0x700c
    //OOBS_Forcecal=[1]
    write_pcie_mdio(0x09 + offset,0x720c);   //0x09 = 0x720c
    //RX_TEST_EN=[1]
    write_pcie_mdio_mask(0x0d + offset, 1, 6, 1);    //0x0D[6]=[1]
    //0x19/0x59 Bit[2]=[1] ; 0x10/0x50=0x03C4
    write_pcie_mdio_mask(0x19 + offset, 1, 2, 1);   //0x19[2]=[1]
    write_pcie_mdio(0x10 + offset,0x03C4);          //0x10 = 0x03C4

#ifndef SIMPLE_CAL_FLOW
    //RX_Debug port[7] = RXIDLE; 0x1F/0x5F Bit[6]
    reg1 = wait_pcie_mdio_mask(0x1f + offset, 1, 6, 0, 20);
    if(reg1 == 0 ){
        PCIE_ALERT("%s(%d)wait RXIDLE fail\n",__func__,__LINE__);
        goto CAL_FAIL;
    }
    mdelay(1);

    //0x19/x049 Bit[2]=[1] ; 0x10/0x50=0x03C4
    write_pcie_mdio_mask(0x19 + offset, 1, 2, 1);   //0x19/0x49 Bit[2]=[1]
    write_pcie_mdio(0x10 + offset,0x03c4);          //0x10/0x50 = 0x03c4
    //get OOBS Debug port 0x1F/0x5F Bit[12:8]
    oobs1 = read_pcie_mdio_mask(0x1f + offset, 0x1f, 8);
    PCIE_ALERT("oobs1=0x%08x\n",oobs1);
    //fill OOBS to 0x03/0x43[5:1]
    write_pcie_mdio_mask(0x03 + offset, 0x1f, 1, oobs1);
    //0x09/0x49 OOBS_SEL=0x1(OOBS Manual)
    write_pcie_mdio(0x09 + offset,0x721C);   //0x09 = 0x721C
    //wait 0x1F/0x5F Bit[15] =[1]
#endif

    if(gen == GEN1){
        reg1 = wait_pcie_mdio_mask(0x1f + offset, 1, 15, 1, 40);
        if(reg1 == 0 ){
            PCIE_ALERT("%s(%d)wait FILTER_OUT_EQ fail\n",__func__,__LINE__);
            goto CAL_FAIL;
        }
    }else{
        PCIE_ALERT("Gen%d skip check Front End Offset\n",gen);
    }

    mdelay(1);

    //0x19/0x59 Bit[2]=[0] ; 0x10/0x50=0x0C¡F0x0D/0x4D[6]=[0]
    write_pcie_mdio_mask(0x19 + offset, 1, 2, 0);   //0x19/0x59 Bit[2]=[0]
    write_pcie_mdio(0x10 + offset,0x0C);            //0x10/0x50 = 0x0C
    write_pcie_mdio_mask(0x0D + offset, 1, 6, 0);   //0x0D/0x4D[6]=[0]
    //0x1F/0x5F Bit[4:1] RX_OFFSET_CODE
    reg1 = read_pcie_mdio_mask(0x1f + offset, 0xF, 1);
    PCIE_ALERT("RX_OFFSET_CODE=0x%08x\n",reg1);

    return;

CAL_FAIL:
    PCIE_ALERT("phy calibrattion fail\n");
    return;

}


#define PCIE_WRAP_PCIE_XALI_BASE_VDDR   (0xb807C000)
#define PCIE_WRAP_PCIE_DBI_BASE_VADDR   (0xB807D000)

static __maybe_unused
u32 rtk_pcie_read_dbi(struct dw_pcie *pci, void __iomem *base,
				u32 reg, size_t size)
{
    u32 dbase, offset;
    u32 reginfo;

    dbase = reg & 0xfffff000;
    offset = reg & 0x00000fff;

    rtd_outl(PCIE_WRAP_PCIE_HOST_SYS_BUS_CTRL2_reg, dbase);
    if (size == 4){
		reginfo = readl(base+offset);
	}else if (size == 2){
		reginfo = readw(base+offset);
	}else if (size == 1){
		reginfo = readb(base+offset);
	}else{
		reginfo = 0;
	}

    return reginfo;
}

static __maybe_unused
void rtk_pcie_write_dbi(struct dw_pcie *pci, void __iomem *base,
				  u32 reg, size_t size, u32 val)
{
    u32 dbase, offset;
    u32 val_new = 0;

    if(size != 4){
        u32 val_tmp = 0;
        u32 byte_mask = pci_byte_mask(reg, size);
        u32 bit_mask = pci_bit_mask(byte_mask);
        u32 bit_shift = pci_bit_shift(reg);

        val_tmp = rtk_pcie_read_dbi(pci,base,(reg & ~0x3),4);
        val_new = (val_tmp & ~(bit_mask)) | (val << bit_shift);
    }else{
        val_new = val;
    }

    dbase = reg & 0xfffff000;
    offset = reg & 0x00000ffc;  // for write must align 4bytes. need to DIC fix it. //reg & 0x00000fff;
    rtd_outl(PCIE_WRAP_PCIE_HOST_SYS_BUS_CTRL2_reg, dbase);
	writel(val_new, base+offset);

    return;
}


static __maybe_unused
int send_rd_requset(struct dw_pcie *pci,PCIE_REQ_CTX *req_ctx,int size)
{
    u32 slv_sb_ctrl5 = 0;
    u32 xali_addr_base;
    u32 xali_addr;
    u32 resp;
    unsigned long flags;
    u32 sb2_reg;

    spin_lock_irqsave(&rtk_pcie_send_req_lock, flags);

    if (pcie_disable_rbus_timeout) {
        sb2_reg = (rtd_inl(SB2_INV_INTEN_reg) & (SB2_INV_INTEN_to_en_mask | SB2_INV_INTEN_to_en1_mask | SB2_INV_INTEN_to_end_mask));
        rtd_outl(SB2_INV_INTEN_reg, sb2_reg);  // disable rbus timeout
    }

    //set sideband signals
    slv_sb_ctrl5 =
        ((req_ctx->tlp_type     & 0x1f)<< 0) |
        ((req_ctx->np           & 0x01)<< 5) |
        ((req_ctx->a_ep         & 0x01)<< 6) |
        ((req_ctx->ns           & 0x01)<< 8) |
        ((req_ctx->ro           & 0x01)<< 9) |
        ((req_ctx->tc           & 0x01)<<10) |
        ((req_ctx->msg_code     & 0x01)<<13) |
        ((req_ctx->dbi_access   & 0x01)<<21) |
        ((req_ctx->atu_bypass   & 0x01)<<22);
    rtd_outl(PCIE_WRAP_PCIE_AXI_SLAVE_SB_CTRL5_reg, slv_sb_ctrl5);

    //set xali address
    xali_addr_base = req_ctx->addr & 0xfffff000;
    rtd_outl(PCIE_WRAP_PCIE_HOST_SYS_BUS_CTRL1_reg, xali_addr_base);

    udelay(pcie_rw_req_dealy);

    //initialte read request
    xali_addr = PCIE_WRAP_PCIE_XALI_BASE_VDDR | (req_ctx->addr & 0xfff);

    if (size == 4){
		req_ctx->data = rtd_inl(xali_addr);
	}else if (size == 2){
		req_ctx->data = rtd_inw(xali_addr);
	}else if (size == 1){
		req_ctx->data = rtd_inb(xali_addr);
	}else{
	    rtd_pr_pcie_alert("%s(%d)assigned size not match\n",__func__,__LINE__);
	}

    resp= rtd_inl(PCIE_WRAP_PCIE_AXI_SLAVE_SB_STATUS1_reg);
    req_ctx->resp = (resp>>16) & 0x7ff;
    //fixme: check if TLP ok!!

    if (pcie_disable_rbus_timeout)
        rtd_outl(SB2_INV_INTEN_reg, sb2_reg | SB2_INV_INTEN_write_data_mask);  // restore rbus timeout setting

    spin_unlock_irqrestore(&rtk_pcie_send_req_lock, flags);
    return 0;
}



static __maybe_unused
int send_wr_requset(struct dw_pcie *pci,PCIE_REQ_CTX *req_ctx,int size)
{
    struct rtk_pcie_dw *pcie = NULL;
    u32 slv_sb_ctrl1 = 0;
    u32 slv_sb_ctrl2 = 0;
    u32 slv_sb_ctrl3 = 0;
    u32 slv_sb_ctrl4 = 0;
    u32 xali_addr_base;
    u32 xali_addr;
    u32 resp;
    unsigned long flags;

    pcie = to_rtk_pcie(pci);
    if(pcie->ic_ver == VERSION_A ){
        if(size != 4){
            PCIE_ALERT("%s(%d)can't support non-4byte access!!!\n",__func__,__LINE__);
            WARN_ON(1);
        }
    }
    slv_sb_ctrl1 =
        ((req_ctx->tlp_type     & 0x1f)<< 0) |
        ((req_ctx->np           & 0x01)<< 5) |
        ((req_ctx->a_ep         & 0x01)<< 6) |
        ((req_ctx->ns           & 0x01)<< 8) |
        ((req_ctx->ro           & 0x01)<< 9) |
        ((req_ctx->tc           & 0x01)<<10) |
        ((req_ctx->msg_code     & 0x01)<<13) |
        ((req_ctx->dbi_access   & 0x01)<<21);

    slv_sb_ctrl2 = req_ctx->hdr_3rd_dw;
    slv_sb_ctrl3 = req_ctx->hdr_4th_dw;

    slv_sb_ctrl4 =
        ((req_ctx->p_tag        & 0xff)<< 0) |
        ((req_ctx->atu_bypass   & 0x01)<< 8) |
        ((req_ctx->d_ep         & 0x01)<< 9) |
        ((req_ctx->silentDrop   & 0x01)<<10);

    spin_lock_irqsave(&rtk_pcie_send_req_lock, flags);

    rtd_outl(PCIE_WRAP_PCIE_AXI_SLAVE_SB_CTRL1_reg, slv_sb_ctrl1);
    rtd_outl(PCIE_WRAP_PCIE_AXI_SLAVE_SB_CTRL2_reg, slv_sb_ctrl2);
    rtd_outl(PCIE_WRAP_PCIE_AXI_SLAVE_SB_CTRL3_reg, slv_sb_ctrl3);
    rtd_outl(PCIE_WRAP_PCIE_AXI_SLAVE_SB_CTRL4_reg, slv_sb_ctrl4);

    //set xali address
    xali_addr_base = req_ctx->addr & 0xfffff000;
    //initialte write request
    xali_addr = PCIE_WRAP_PCIE_XALI_BASE_VDDR | (req_ctx->addr & 0xfff);
    //rtd_outl(xali_addr, req_ctx->data);
    rtd_outl(PCIE_WRAP_PCIE_HOST_SYS_BUS_CTRL1_reg, xali_addr_base);

    udelay(pcie_rw_req_dealy);

    if (size == 4){
		rtd_outl(xali_addr, req_ctx->data);
	}else if (size == 2){
		rtd_outw(xali_addr, req_ctx->data);
	}else if (size == 1){
		rtd_outb(xali_addr, req_ctx->data);
	}else{
	    rtd_pr_pcie_alert("%s(%d)assigned size not match\n",__func__,__LINE__);
	}

    resp= rtd_inl(PCIE_WRAP_PCIE_AXI_SLAVE_SB_STATUS1_reg);
    req_ctx->resp = resp & 0x7ff;

    //fixme: check if TLP ok!!
    spin_unlock_irqrestore(&rtk_pcie_send_req_lock, flags);
    return 0;
}

static __maybe_unused
int rtk_pcie_send_cfg_rd(struct pcie_port *pp, struct pci_bus *bus,
    unsigned int devfn, int addr, int size, u32 *data)
{
    int ret;
    u32 type;
    struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
    PCIE_REQ_CTX ctx = {0};

    if (bus->parent->number == pp->root_bus_nr) {
		type = PCIE_ATU_TYPE_CFG0;
	} else {
		type = PCIE_ATU_TYPE_CFG1;
	}

    ctx.addr = ((bus->number << 24) |
                (PCI_SLOT(devfn)<<19) |
                (PCI_FUNC(devfn)<<16) |
                addr);
    ctx.is_read = 1;
    ctx.tlp_type = type;
    ret = send_rd_requset(pci, &ctx,size);
    *data = ctx.data;

    return ret;
}


static __maybe_unused
int rtk_pcie_send_cfg_rd_2(int addr, int size, u32 *data)
{
    int ret;
    u32 type;

    PCIE_REQ_CTX ctx = {0};

    type = PCIE_ATU_TYPE_CFG0;

    ctx.addr = ((0 << 24) |
                (1<<19) |
                (0<<16) |
                addr);
    ctx.is_read = 1;
    ctx.tlp_type = type;
    ret = send_rd_requset(pci_local, &ctx,size);
    *data = ctx.data;

    return ret;
}


static __maybe_unused
int rtk_pcie_send_cfg_wr(struct pcie_port *pp, struct pci_bus *bus,
    unsigned int devfn, int addr, int size, u32 data)
{
    int ret;
    u32 type;
    u32 data_new = 0;
    u32 addr_new = 0;
    struct dw_pcie *pci = NULL;
    struct rtk_pcie_dw *pcie = NULL;
    __maybe_unused u32 reginfo = 0;
    PCIE_REQ_CTX ctx = {0};

    pci = to_dw_pcie_from_pp(pp);
    pcie = to_rtk_pcie(pci);

    if (bus->parent->number == pp->root_bus_nr) {
		type = PCIE_ATU_TYPE_CFG0;
	} else {
		type = PCIE_ATU_TYPE_CFG1;
	}
    if(pcie->ic_ver > VERSION_A ){
        addr_new = addr;
        data_new = data;
    }else{
        addr_new = (addr & ~0x3);
        if(size != 4){
            u32 byte_mask = pci_byte_mask(addr, size);
            u32 bit_mask = pci_bit_mask(byte_mask);
            u32 bit_shift = pci_bit_shift(addr);
            PCIE_REQ_CTX ctxr = {0};

            ctxr.is_read = 1;
            ctxr.tlp_type = type;
            ctxr.addr = ((bus->number << 24) |
                        (PCI_SLOT(devfn)<<19) |
                        (PCI_FUNC(devfn)<<16) |
                        addr_new);

            ret = send_rd_requset(pci,&ctxr,4);
            data_new = (ctxr.data & ~(bit_mask)) |  (data << bit_shift);
            size = 4;
        }else{
            data_new = data;
        }
    }
    ctx.data = data_new;
    ctx.is_read = 0;
    ctx.tlp_type = type;
    ctx.addr = ((bus->number << 24) |
                (PCI_SLOT(devfn)<<19) |
                (PCI_FUNC(devfn)<<16) |
                addr_new);
    ret = send_wr_requset(pci,&ctx,size);

    return ret;
}

int rtk_pcie_send_mem_rd(u32 addr, int size, u32 *data)
{
    int ret;
    u32 type;

    PCIE_REQ_CTX ctx = {0};

    type = PCIE_ATU_TYPE_MEM;
    ctx.addr = addr;

    ctx.is_read = 1;
    ctx.tlp_type = type;
    ret = send_rd_requset(pci_local,&ctx,size);
    *data = ctx.data;
	wmb();
    return 0;
}
EXPORT_SYMBOL(rtk_pcie_send_mem_rd);

int rtk_pcie_send_mem_wr(u32 addr, int size, u32 data)
{
    struct rtk_pcie_dw *pcie = NULL;
    int ret;
    u32 type;
    u32 data_new = 0;
    u32 addr_new = 0;
    __maybe_unused u32 reginfo = 0;
    PCIE_REQ_CTX ctx = {0};

    pcie = to_rtk_pcie(pci_local);
    type = PCIE_ATU_TYPE_MEM;
    if(pcie->ic_ver > VERSION_A ){
        addr_new = addr;
        data_new = data;
    }else{
        addr_new = (addr & ~0x3);
        if(size != 4){
            u32 byte_mask = pci_byte_mask(addr, size);
            u32 bit_mask = pci_bit_mask(byte_mask);
            u32 bit_shift = pci_bit_shift(addr);
            PCIE_REQ_CTX ctxr = {0};

            ctxr.is_read = 1;
            ctxr.tlp_type = type;
            ctxr.addr = addr_new;

            ret = send_rd_requset(pci_local,&ctxr,4);
            data_new = (ctxr.data & ~(bit_mask)) |  (data << bit_shift);
            size = 4;
        }else{
            data_new = data;
        }
    }

    ctx.data = data_new;
    ctx.is_read = 0;
    ctx.tlp_type = type;
    ctx.addr = addr_new;
    ret = send_wr_requset(pci_local,&ctx,size);
	wmb();

    return ret;
}
EXPORT_SYMBOL(rtk_pcie_send_mem_wr);


void rtk_pcie_dump_status(void){
#if 1
    /* for debug to dump status of RC */
#else
    u32 reginfo;
    u32 pci_reg;
    rtd_pr_pcie_alert("%s(%d) reg(%08x)=%08x\n",__func__,__LINE__,0xb807e230,rtd_inl(0xb807e230));
    rtd_pr_pcie_alert("%s(%d) reg(%08x)=%08x\n",__func__,__LINE__,0xb807E260,rtd_inl(0xb807E260));
    rtd_pr_pcie_alert("%s(%d) reg(%08x)=%08x\n",__func__,__LINE__,0xb807e020,rtd_inl(0xb807e020));

    pci_reg = 0x18;
    reginfo = rtk_pcie_read_dbi(pci_local, pci_local->dbi_base, pci_reg, 4);
    rtd_pr_pcie_alert("%s(%d)own pci reg(0x%x) = %08x\n",__func__,__LINE__,pci_reg,reginfo);
    pci_reg = 0x1c;
    reginfo = rtk_pcie_read_dbi(pci_local, pci_local->dbi_base, pci_reg, 4);
    rtd_pr_pcie_alert("%s(%d)own pci reg(0x%x) = %08x\n",__func__,__LINE__,pci_reg,reginfo);
    pci_reg = 0x20;
    reginfo = rtk_pcie_read_dbi(pci_local, pci_local->dbi_base, pci_reg, 4);
    rtd_pr_pcie_alert("%s(%d)own pci reg(0x%x) = %08x\n",__func__,__LINE__,pci_reg,reginfo);
    pci_reg = 0x24;
    reginfo = rtk_pcie_read_dbi(pci_local, pci_local->dbi_base, pci_reg, 4);
    rtd_pr_pcie_alert("%s(%d)own pci reg(0x%x) = %08x\n",__func__,__LINE__,pci_reg,reginfo);
    pci_reg = 0x28;
    reginfo = rtk_pcie_read_dbi(pci_local, pci_local->dbi_base, pci_reg, 4);
    rtd_pr_pcie_alert("%s(%d)own pci reg(0x%x) = %08x\n",__func__,__LINE__,pci_reg,reginfo);
    pci_reg = 0x30;
    reginfo = rtk_pcie_read_dbi(pci_local, pci_local->dbi_base, pci_reg, 4);
    rtd_pr_pcie_alert("%s(%d)own pci reg(0x%x) = %08x\n",__func__,__LINE__,pci_reg,reginfo);

    pci_reg = 0x130;
    reginfo = rtk_pcie_read_dbi(pci_local, pci_local->dbi_base, pci_reg, 4);
    rtd_pr_pcie_alert("%s(%d)own pci reg(0x%x) = %08x\n",__func__,__LINE__,pci_reg,reginfo);

    pci_reg = 4;
    rtk_pcie_send_cfg_rd_2(pci_reg, 4, &reginfo);
    rtd_pr_pcie_alert("%s(%d)ext pci reg(0x%x) = %08x\n",__func__,__LINE__,pci_reg,reginfo);

    pci_reg = 0x10; //BAR0
    rtk_pcie_send_cfg_rd_2(pci_reg, 4, &reginfo);
    rtd_pr_pcie_alert("%s(%d)ext pci reg(0x%x) = %08x\n",__func__,__LINE__,pci_reg,reginfo);

    pci_reg = 0x14; //BAR1
    rtk_pcie_send_cfg_rd_2(pci_reg, 4, &reginfo);
    rtd_pr_pcie_alert("%s(%d)ext pci reg(0x%x) = %08x\n",__func__,__LINE__,pci_reg,reginfo);

    pci_reg = 0x18; //BAR2
    rtk_pcie_send_cfg_rd_2(pci_reg, 4, &reginfo);
    rtd_pr_pcie_alert("%s(%d)ext pci reg(0x%x) = %08x\n",__func__,__LINE__,pci_reg,reginfo);

    pci_reg = 0x1c; //BAR3
    rtk_pcie_send_cfg_rd_2(pci_reg, 4, &reginfo);
    rtd_pr_pcie_alert("%s(%d)ext pci reg(0x%x) = %08x\n",__func__,__LINE__,pci_reg,reginfo);

    pci_reg = 0x20; //BAR4
    rtk_pcie_send_cfg_rd_2(pci_reg, 4, &reginfo);
    rtd_pr_pcie_alert("%s(%d)ext pci reg(0x%x) = %08x\n",__func__,__LINE__,pci_reg,reginfo);

    pci_reg = 0x24; //BAR5
    rtk_pcie_send_cfg_rd_2(pci_reg, 4, &reginfo);
    rtd_pr_pcie_alert("%s(%d)ext pci reg(0x%x) = %08x\n",__func__,__LINE__,pci_reg,reginfo);
#endif
}
EXPORT_SYMBOL(rtk_pcie_dump_status);

void rtk_pcie_rc_write(u32 reg,u32 val)
{
    rtd_outl(reg, val);
    rtd_pr_pcie_alert("%s(%d)set 0x%x=0x%x\n",__func__,__LINE__,reg,val);
}
EXPORT_SYMBOL(rtk_pcie_rc_write);

u32 rtk_pcie_rc_read(u32 reg)
{
    u32 reginfo;
    reginfo = rtd_inl(reg);
    rtd_pr_pcie_alert("%s(%d)get 0x%x=0x%x\n",__func__,__LINE__,reg,reginfo);
    return reginfo;
}
EXPORT_SYMBOL(rtk_pcie_rc_read);


static __maybe_unused
int rtk_pcie_rd_other_conf(struct pcie_port *pp, struct pci_bus *bus,
    unsigned int devfn, int where, int size, u32 *val)
{
    int ret = 0;

    ret = rtk_pcie_send_cfg_rd(pp, bus, devfn, where, size, val);

    return ret;
}

static __maybe_unused
int rtk_pcie_wr_other_conf(struct pcie_port *pp, struct pci_bus *bus,
    unsigned int devfn, int where, int size, u32 val)
{
    int ret = 0;

    ret = rtk_pcie_send_cfg_wr(pp, bus, devfn, where, size, val);

    return ret;
}

static __maybe_unused
int rtk_pcie_rd_own_conf(struct pcie_port *pp, int where, int size,
			       u32 *val)
{
	struct dw_pcie *pci;

	pci = to_dw_pcie_from_pp(pp);

	*val = rtk_pcie_read_dbi(pci, pci->dbi_base, where, size);

	return 0;
}

static __maybe_unused
int rtk_pcie_wr_own_conf(struct pcie_port *pp, int where, int size,
			       u32 val)
{
	struct dw_pcie *pci;

	pci = to_dw_pcie_from_pp(pp);

	rtk_pcie_write_dbi(pci, pci->dbi_base, where, size, val);

	return 0;
}


static int rtk_pci_set_max_payload_size(struct dw_pcie *pci){
    int ret = 0;
    u32 max_payload_size;
    u32 reg_info;

    /*
     PCIE_CX_MAX_MTU_SEL:
            0 : 128, 1: 256, 2: 512, 3:1024, 4: 2048, 5: 4096
     HW fixed at 256. So, select 1.
    */
            max_payload_size = PCIE_CX_MAX_MTU_SEL;

    reg_info  = dw_pcie_readl_dbi(pci,PCIE_DEVICE_CONTROL_DEVICE_STATUS);
    reg_info &= ~(PCIE_CAP_MAX_PAYLOAD_SIZE_CS_MASK|PCIE_CAP_MAX_READ_REQ_SIZE_MASK);
    reg_info |= (max_payload_size << PCIE_CAP_MAX_PAYLOAD_SIZE_CS_POSITION);
    reg_info |= (max_payload_size << PCIE_CAP_MAX_READ_REQ_SIZE_POSITION);
    dw_pcie_writel_dbi(pci,PCIE_DEVICE_CONTROL_DEVICE_STATUS,reg_info);

    rtd_pr_pcie_alert("%s(%d)max_payload_size = %d, (reg_info=%08lx)\n",__func__,__LINE__, max_payload_size, reg_info);

    return ret;

}

static int rtk_pcie_chg_ID(struct pcie_port *pp)
{
#ifdef REASSIGN_ID
    u32 val;

    pci = to_dw_pcie_from_pp(pp);
    /* re-assign PID/VID */
    dw_pcie_dbi_ro_wr_en(pci);
    val = dw_pcie_readl_dbi(pci, PCI_VENDOR_ID);
    val = (PCIE_RTK_PID << 16) | PCIE_RTK_VID;
    dw_pcie_writel_dbi(pci, PCI_VENDOR_ID, val);
    dw_pcie_dbi_ro_wr_dis(pci);
#endif
    return 0;
}

static int rtk_pcie_dw_host_init(struct pcie_port *pp)
{
    struct dw_pcie *pci = NULL;
    struct rtk_pcie_dw *pcie = NULL;
    int ret = 0;

    pci = to_dw_pcie_from_pp(pp);
    pcie = to_rtk_pcie(pci);

    rtk_pcie_set_link_speed(pci, pcie_link_speed, 0);

    /* all setting should be ready before */
    if (dw_pcie_wait_for_link(pci)) {
        /* have anything need to do?? */
        rtd_pr_pcie_alert("%s(%d)link fail\n",__func__,__LINE__);
        ret = -1;
        goto OUT;
    }

    rtk_pci_set_max_payload_size(pci);
    dw_pcie_setup_rc(pp);
    rtk_pcie_chg_ID(pp);
    rtk_pcie_enable_interrupts(pp);

OUT:
    return ret;
}

static int rtk_pcie_set_link_speed(	struct dw_pcie *pci,
        e_pciegen speed, u32 initiate_speed_change)
{
    int ret = 0;
    u32 val;

    val = dw_pcie_readl_dbi(pci, PCIE_LINK_CONTROL2_LINK_STATUS2_REG);
    val &= ~PCIE_CAP_TARGET_LINK_SPEED_MASK;
    val |= (speed << PCIE_CAP_TARGET_LINK_SPEED_POSITION);
    dw_pcie_writel_dbi(pci, PCIE_LINK_CONTROL2_LINK_STATUS2_REG, val);

#if 0
    if(initiate_speed_change) {
         rt_pci_reg_rmw(PCIE_GEN2_CTRL_OFF,
                (0 << PCIE_PL_DIRECT_SPEED_CHANGE_POSITION),
                PCIE_PL_DIRECT_SPEED_CHANGE_MASK);
         rt_pci_reg_rmw(PCIE_GEN2_CTRL_OFF,
                (1 << PCIE_PL_DIRECT_SPEED_CHANGE_POSITION),
                PCIE_PL_DIRECT_SPEED_CHANGE_MASK);
    }

#endif
    return ret;

}


static int rtk_pcie_link_up(struct dw_pcie *pci)
{
    __maybe_unused struct rtk_pcie_dw *pcie = to_rtk_pcie(pci);

    int ret = 0;
    __maybe_unused u32 temp;
    u32 current_link_rate;
    uint32_t reg_info;

    // 1. Wait SMLH link is up
    // 2. Wait RDLH link is up
    // 3. Wait Expected Data Rate
    // 4. Wait LTSSM to L0 for 50ms

    temp = dw_pcie_readl_dbi(pci, PCIE_LINK_CONTROL_LINK_STATUS_REG);
    current_link_rate =  (temp &  PCIE_CAP_LINK_SPEED_MASK) >> PCIE_CAP_LINK_SPEED_POSITION;
    reg_info = rtd_inl(PCIE_WRAP_PCIE_SII_PM_STATUS1_reg);
    reg_info = PCIE_WRAP_PCIE_SII_PM_STATUS1_get_smlh_ltssm_state(reg_info);

    if(reg_info == S_L0){   //link success
        ret = 1;
        if( (current_link_rate == 0) || (current_link_rate > GEN5)){
            rtd_pr_pcie_alert("%s(%d)currentlink rate unknow(%x)\n",__func__,__LINE__,current_link_rate);
        }
    }

    return ret;
}



static const struct dw_pcie_ops rtk_dw_pcie_ops = {
    .link_up   = rtk_pcie_link_up,
    .read_dbi  = rtk_pcie_read_dbi,
    .write_dbi = rtk_pcie_write_dbi,
};

static struct dw_pcie_host_ops rtk_pcie_dw_host_ops = {
    .rd_other_conf  = rtk_pcie_rd_other_conf,
    .wr_other_conf  = rtk_pcie_wr_other_conf,
    .rd_own_conf    = rtk_pcie_rd_own_conf,
    .wr_own_conf    = rtk_pcie_wr_own_conf,
    .host_init      = rtk_pcie_dw_host_init,
};


static int rtk_pcie_dw_parse_dt(struct rtk_pcie_dw *pcie)
{
    /* Parse keyword from dts
       linux-5.4\arch\arm64\boot\dts\rtk_tv\rtd6748.dts */
    rtd_pr_pcie_alert("%s(%d)No any indormation need parse now.\n",__func__,__LINE__);
    return 0;

}


static int rtk_pci_clk_sel(u32 enable)
{
    u32 reg_info;
    u32 en_val;

    // Note: 0 for PCIE enable; 1 for USB3
    en_val = (enable)? 0:1;
    rtd_pr_pcie_alert("%s(%d)PCIE %s\n",__func__,__LINE__,(en_val)?"dis-select":"selected");

    reg_info = rtd_inl(SYS_REG_SYS_CLKSEL_reg);
    reg_info &= ~SYS_REG_SYS_CLKSEL_pcie_usb3_sel_mask;
    reg_info |= SYS_REG_SYS_CLKSEL_pcie_usb3_sel(en_val);
    rtd_outl(SYS_REG_SYS_CLKSEL_reg,reg_info);

    return 0;
}

/*
The SW decides the address bit [31:26] such that all the memory bus will access the address space
Reserved	25:0	-	-	Reserved
    ==> need discuss with memory manage owner
*/
void rtk_pcie_set_mem_space(struct rtk_pcie_dw *pcie,u32 mem_space)
{
    rtd_outl(PCIE_WRAP_PCIE_MEM_BUS_CTRL4_reg,mem_space);
}


int rtk_pcie_set_mem_space_exp(u32 mem_space)
{
    rtk_pcie_set_mem_space(NULL,mem_space);
    return 0;
}
EXPORT_SYMBOL(rtk_pcie_set_mem_space_exp);

void* rtk_pcie_dma_alloc_coherent(struct device *dev, size_t size,
		dma_addr_t *dma_handle, gfp_t gfp)
{
    return dma_alloc_coherent(rtk_pcie_dev, size,dma_handle,gfp);
}
EXPORT_SYMBOL(rtk_pcie_dma_alloc_coherent);

void rtk_pcie_dma_free_coherent(struct device *dev, size_t size,
		void *cpu_addr, dma_addr_t dma_handle)
{
	dma_free_coherent(rtk_pcie_dev, size, cpu_addr, dma_handle);
}
EXPORT_SYMBOL(rtk_pcie_dma_free_coherent);

/* ==================================================================
    async-reset
    rst 0 ==> clk 1 ==> delay ==> clk 0 ==> rst 1 ==> clk 1
================================================================== */
static int rtk_pci_crt_enable(u32 enable)
{
    u32 en_val;

    en_val = (enable)? 1:0;

    ///////////PCIe CRT /////
    if(en_val){
        rtd_outl( SYS_REG_SYS_SRST3_reg,
            SYS_REG_SYS_SRST3_rstn_pcie(1) | SYS_REG_SYS_SRST3_write_data(0));
        rtd_outl( SYS_REG_SYS_CLKEN3_reg,
            SYS_REG_SYS_CLKEN3_clken_pcie(1) | SYS_REG_SYS_CLKEN3_write_data(1));
        udelay(50);
        rtd_outl( SYS_REG_SYS_CLKEN3_reg,
            SYS_REG_SYS_CLKEN3_clken_pcie(1) | SYS_REG_SYS_CLKEN3_write_data(0));
        rtd_outl( SYS_REG_SYS_SRST3_reg,
            SYS_REG_SYS_SRST3_rstn_pcie(1) | SYS_REG_SYS_SRST3_write_data(1));
        rtd_outl( SYS_REG_SYS_CLKEN3_reg,
            SYS_REG_SYS_CLKEN3_clken_pcie(1) | SYS_REG_SYS_CLKEN3_write_data(1));
    }else{
        rtd_outl( SYS_REG_SYS_SRST3_reg,
            SYS_REG_SYS_SRST3_rstn_pcie(1) | SYS_REG_SYS_SRST3_write_data(0));
        rtd_outl( SYS_REG_SYS_CLKEN3_reg,
            SYS_REG_SYS_CLKEN3_clken_pcie(1) | SYS_REG_SYS_CLKEN3_write_data(0));
    }

    return 0;
}

static int rtk_pci_set_app_ltssm_enable(u32 enable)
{
    u32 reg_info;
    u32 en_val;

    en_val = (enable)? 1:0;

    reg_info = rtd_inl(PCIE_WRAP_PCIE_SII_GEN_CORE_CTRL1_reg);
    reg_info &= ~PCIE_WRAP_PCIE_SII_GEN_CORE_CTRL1_app_ltssm_enable_mask;
    reg_info |= PCIE_WRAP_PCIE_SII_GEN_CORE_CTRL1_app_ltssm_enable(en_val);
    rtd_outl(PCIE_WRAP_PCIE_SII_GEN_CORE_CTRL1_reg,reg_info);

    return 0;
}

static int rtk_pci_mdio_enable(u32 enable)
{
    u32 en_val;

    en_val = (enable)? 1:0;

    //pcie_mdrst_n bit1=1
    rtd_outl( PCIE_WRAP_PCIE_CLKRST_CTRL1_reg,
            PCIE_WRAP_PCIE_CLKRST_CTRL1_pcie_mdrst_n(1) |
            PCIE_WRAP_PCIE_CLKRST_CTRL1_write_data(en_val));

    return 0;
}

static void rtk_pci_set_bus_endian(e_bus_endian endian)
{
    u32 reg_info;

    reg_info = rtd_inl( PCIE_WRAP_PCIE_MEM_BUS_CTRL1_reg);
    reg_info &= ~PCIE_WRAP_PCIE_MEM_BUS_CTRL1_dw_endian_swap_mask;
    reg_info |= PCIE_WRAP_PCIE_MEM_BUS_CTRL1_dw_endian_swap(endian);
    rtd_outl( PCIE_WRAP_PCIE_MEM_BUS_CTRL1_reg,reg_info);
}

static int rtk_pci_wrap_enable(u32 enable)
{
    int ret = 0;
    u32 reg_info;
    u32 en_val;
    u32 wloop;

    en_val = (enable)? 1:0;

    ///////////PCIe WRAP /////
    // pow_pcie bit0=1
    reg_info = rtd_inl( PCIE_WRAP_PCIE_PIPE_CTRL1_reg);
    reg_info &= ~PCIE_WRAP_PCIE_PIPE_CTRL1_pow_pcie_mask;
    reg_info |= PCIE_WRAP_PCIE_PIPE_CTRL1_pow_pcie(en_val);
    rtd_outl( PCIE_WRAP_PCIE_PIPE_CTRL1_reg,reg_info);

    // sys_aux_pwr_det bit0=1
    reg_info = rtd_inl( PCIE_WRAP_PCIE_SII_PM_CTRL1_reg);
    reg_info &= ~PCIE_WRAP_PCIE_SII_PM_CTRL1_sys_aux_pwr_det_mask;
    reg_info |= PCIE_WRAP_PCIE_SII_PM_CTRL1_sys_aux_pwr_det(en_val);
    rtd_outl( PCIE_WRAP_PCIE_SII_PM_CTRL1_reg,reg_info);

    // aux bit8=1,mstr bit11=1,slv bit12=1,dbi bit13=1
    rtd_outl(PCIE_WRAP_PCIE_CLKRST_CTRL1_reg,
            PCIE_WRAP_PCIE_CLKRST_CTRL1_dbi_aclken(1) |
            PCIE_WRAP_PCIE_CLKRST_CTRL1_slv_aclken(1) |
            PCIE_WRAP_PCIE_CLKRST_CTRL1_mstr_aclken(1) |
            PCIE_WRAP_PCIE_CLKRST_CTRL1_aux_clken(1) |
            PCIE_WRAP_PCIE_CLKRST_CTRL1_write_data(en_val));
    udelay(50);

    // perst_n bit2=1
    rtd_outl(PCIE_WRAP_PCIE_CLKRST_CTRL1_reg,
            PCIE_WRAP_PCIE_CLKRST_CTRL1_perst_n(1) |
            PCIE_WRAP_PCIE_CLKRST_CTRL1_write_data(en_val));
    udelay(50);

    // power_up_rst_n bit3=1,button_rst_n bit4=1
    rtd_outl(PCIE_WRAP_PCIE_CLKRST_CTRL1_reg,
            PCIE_WRAP_PCIE_CLKRST_CTRL1_power_up_rst_n(1) |
            PCIE_WRAP_PCIE_CLKRST_CTRL1_button_rst_n(1) |
            PCIE_WRAP_PCIE_CLKRST_CTRL1_write_data(en_val));
    udelay(50);

    // core_clken bit9=1
    rtd_outl(PCIE_WRAP_PCIE_CLKRST_CTRL1_reg,
            PCIE_WRAP_PCIE_CLKRST_CTRL1_core_clken(1) |
            PCIE_WRAP_PCIE_CLKRST_CTRL1_write_data(en_val));
    udelay(50);

    //change DDR bus little endian
    rtk_pci_set_bus_endian(BUS_LITTLE_EDN);

    if(en_val){
        u32 rdy_info;

        wloop = 0;    //wait 5ms for clk ready
        do {
            rdy_info = rtd_inl(PCIE_WRAP_PCIE_PIPE_STATUS1_reg) & 0x01;
            if(wloop > 100){
                rtd_pr_pcie_alert("%s(%d)Pcie clock can't ready(over %s us)\n",__func__,__LINE__,(50*wloop));
                ret = -1;
                goto OUT;
            }
            wloop ++;
            udelay(50);

        }while(!rdy_info);
    }

OUT:
    return ret;
}


static unsigned char g_pice_reset_pin_present = 0;
static RTK_GPIO_ID   g_gid_pcie_reset;
static unsigned int  g_gid_pcie_reset_invert = 0;

static void rtk_pcie_hold_perst(void)
{
    if (g_pice_reset_pin_present)
    {
        rtd_pr_pcie_warn("hold PCIe reset pin!!!! (gpio=%s_%d, val=%d)\n",
            gpio_type(gpio_group(g_gid_pcie_reset)), gpio_idx(g_gid_pcie_reset), (g_gid_pcie_reset_invert) ? 0 : 1);

        rtk_gpio_output(g_gid_pcie_reset, (g_gid_pcie_reset_invert) ? 0 : 1);
        rtk_gpio_set_dir(g_gid_pcie_reset,1);
    }
    else
    {
        rtd_pr_pcie_warn("hold PCIe reset failed, pcie reset gpio absent !!!!!\n");
    }
}

static void rtk_pcie_release_perst(void)
{
    if (g_pice_reset_pin_present)
    {
        rtd_pr_pcie_warn("release PCIe reset pin!!!! (gpio=%s_%d, val=%d)\n",
            gpio_type(gpio_group(g_gid_pcie_reset)), gpio_idx(g_gid_pcie_reset), (g_gid_pcie_reset_invert) ? 1 : 0);

        rtk_gpio_output(g_gid_pcie_reset, (g_gid_pcie_reset_invert) ? 1 : 0);
        rtk_gpio_set_dir(g_gid_pcie_reset,1);
    }
    else
    {
        rtd_pr_pcie_warn("release PCIe reset failed, pcie reset gpio absent !!!!!\n");
    }
}

static void rtk_pcie_toggle_perst(void)
{
    /* To use formal GPIO AP later */
    rtk_pcie_hold_perst();
    udelay(100);
    rtk_pcie_release_perst();
}


static int rtk_pcie_load_pcb_cfg(void)
{
    unsigned long long param = 0;

    if (pcb_mgr_get_enum_info_byname("PIN_PCIE_RST", &param) == 0)
    {
        int pinindex = 0;
        int invert = 0;

        if (GET_PIN_TYPE(param) == PCB_PIN_TYPE_ISO_GPIO)
        {
            pinindex = GET_PIN_INDEX(param);
            invert = GET_GPIO_INVERT(param);
            g_pice_reset_pin_present = 1;
            g_gid_pcie_reset = rtk_gpio_id(ISO_GPIO, pinindex);
            g_gid_pcie_reset_invert = (invert) ? 1 : 0;
        }
        else if (GET_PIN_TYPE(param) == PCB_PIN_TYPE_GPIO)
        {
            pinindex = GET_PIN_INDEX(param);
            invert = GET_GPIO_INVERT(param);
            g_pice_reset_pin_present = 1;
            g_gid_pcie_reset = rtk_gpio_id(MIS_GPIO, pinindex);
            g_gid_pcie_reset_invert = (invert) ? 1 : 0;
        }
        else
        {
            rtd_pr_pcie_warn("find PIN_PCIE_RST fail, unknown pin type %d\n", GET_PIN_TYPE(param));
            return 0;
        }
    }
    else
    {
        rtd_pr_pcie_warn("Can't find PIN_PCIE_RST, use default setting\n");
        g_pice_reset_pin_present = 1;
        g_gid_pcie_reset = rtk_gpio_id(ISO_GPIO, 63);
        g_gid_pcie_reset_invert = 1;
    }

    rtd_pr_pcie_warn("PCIe reset pin = %s_%d, invert=%d\n",
        gpio_type(gpio_group(g_gid_pcie_reset)),
        gpio_idx(g_gid_pcie_reset),
        g_gid_pcie_reset_invert);
    return 1;
}

static int rtk_pcie_config_controller(struct rtk_pcie_dw *pcie)
{
    struct dw_pcie *pci = &pcie->pci;
    struct pcie_port *pp = &pci->pp;
    int ret = 0;

    /* Set PHY MUX for PCIE */
    rtk_pci_clk_sel(1);

    /* PCIe CRT */
    rtk_pci_crt_enable(1);

    /* enable pcie_mdrst_n */
    rtk_pci_mdio_enable(1);

    /* set PCIE PHY */
    rtk_pcie_phy_setting(pp);

    /* PCIe WRAP */
    ret = rtk_pci_wrap_enable(1);
    if(ret){
        goto fail_wrap;
    }

    /* calibrate PCIE PHY */
    rtk_pcie_phy_calibration(pp,GEN1);
    rtk_pcie_phy_calibration(pp,GEN2);

    // toggle perst#
    rtk_pcie_toggle_perst();

    // app_ltssm_enable bit0=1
    rtk_pci_set_app_ltssm_enable(1);

    //To setting mem mapping base range, should git this from dtb information later.
    if(pcie->ic_ver == VERSION_A ){
        rtk_pcie_set_mem_space(pcie,PCIE_MEMORY_SPACE);
    }else{
        //0:Range from 0~2G; 1:Range from 2~4G
        rtk_pcie_set_mem_space(pcie,0);
    }

    return ret;

fail_wrap:
    //disable wrap
    rtk_pci_wrap_enable(0);
    //disable mdio
    rtk_pci_mdio_enable(0);
    //disable CRT
    rtk_pci_crt_enable(0);

    return ret;
}

static int __deinit_controller(struct rtk_pcie_dw *pcie)
{
    /* PCIe WRAP */
    rtk_pci_wrap_enable(0);

    /* PCIe CRT */
    rtk_pci_crt_enable(0);

    return 0;
}


static int rtk_pcie_init_controller(struct rtk_pcie_dw *pcie)
{
    struct dw_pcie *pci = &pcie->pci;
    struct pcie_port *pp = &pci->pp;
    int ret;

    ret = rtk_pcie_config_controller(pcie);
    if (ret < 0){
        return ret;
    }

    pp->ops = &rtk_pcie_dw_host_ops;

    ret = dw_pcie_host_init(pp);
    if (ret < 0) {
        dev_err(pcie->dev, "Failed to add PCIe port: %d\n", ret);
        goto fail_host_init;
    }

    return 0;

fail_host_init:
    return __deinit_controller(pcie);

}


static int rtk_pcie_deinit_controller(struct rtk_pcie_dw *pcie)
{
    dw_pcie_host_deinit(&pcie->pci.pp);

    return __deinit_controller(pcie);

}


static int rtk_pcie_config_rp(struct rtk_pcie_dw *pcie)
{
    struct pcie_port *pp = &pcie->pci.pp;
    struct device *dev = pcie->dev;
    char *name;
    int ret = 0;

    if (IS_ENABLED(CONFIG_PCI_MSI)) {
            pp->msi_irq = of_irq_get_byname(dev->of_node, "msi");
            if (!pp->msi_irq) {
                    dev_err(dev, "Failed to get MSI interrupt\n");
                    return -ENODEV;
            }
    }

    rtk_pcie_init_controller(pcie);

    name = devm_kasprintf(dev, GFP_KERNEL, "%pOFP", dev->of_node);
    if (!name) {
            ret = -ENOMEM;
            goto fail_host_init;
    }

    return ret;

fail_host_init:
    rtk_pcie_deinit_controller(pcie);

    return ret;

}


//===========================================================================
// Func : pcie_atr_show_debug_ctrl
//===========================================================================

static ssize_t pcie_atr_show_debug_ctrl(
    struct device*              dev,
    struct device_attribute*    attr,
    char*                       buf
    )
{
    int  n;
    int  count = PAGE_SIZE;
    char *ptr = buf;

    if (strcmp(attr->attr.name, "rw_req_delay")==0)
    {
        n = scnprintf(ptr, count, "%d\n", pcie_rw_req_dealy);
        ptr+=n; count-=n;
    }
    else if (strcmp(attr->attr.name, "disable_rbus_timeout")==0)
    {
        n = scnprintf(ptr, count, "%d\n", pcie_disable_rbus_timeout);
        ptr+=n; count-=n;
    }
    else if (strcmp(attr->attr.name, "pcie_reset_gpio")==0)
    {
        if (g_pice_reset_pin_present)
        {
            n = scnprintf(ptr, count, "no pcie_reset_gpio exist\n");
            ptr+=n; count-=n;
        }
        else
        {
            n = scnprintf(ptr, count, "GPIO=%s_%d, invert=%d, val=%d\n",
                gpio_type(gpio_group(g_gid_pcie_reset)),
                gpio_idx(g_gid_pcie_reset),
                g_gid_pcie_reset_invert,
                rtk_gpio_output_get(g_gid_pcie_reset));
            ptr+=n; count-=n;
        }
    }

    return ptr - buf;
}


static ssize_t pcie_attr_store_debug_ctrl(struct device *dev,
                   struct device_attribute *attr,
                   const char *buf, size_t count)
{
    unsigned int val = 0;

    if (strcmp(attr->attr.name, "rw_req_delay")==0)
    {
        if (sscanf(buf, "%5d", &val)==1)
            pcie_rw_req_dealy = (val > 100) ? 100 : val;
    }
    else if (strcmp(attr->attr.name, "disable_rbus_timeout")==0)
    {
        if (sscanf(buf, "%d", &val)==1)
            pcie_disable_rbus_timeout = (val) ? 1 : 0;
    }
    else
    {
        return -1;
    }

    return count;
}

static DEVICE_ATTR(rw_req_delay, 0644, pcie_atr_show_debug_ctrl, pcie_attr_store_debug_ctrl);
static DEVICE_ATTR(pcie_reset_gpio, 0444, pcie_atr_show_debug_ctrl, NULL);
static DEVICE_ATTR(disable_rbus_timeout, 0444, pcie_atr_show_debug_ctrl, NULL);


static int rtk_pcie_dw_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct rtk_pcie_dw *pcie;
    struct resource *dbi_res;
    struct pcie_port *pp;
    struct dw_pcie *pci;
    u32 ic_ver;
    int ret;
#ifdef RTK_PCIE_DMA
    unsigned long pcie_mem_addr = RTK_PCIE_DMA_ADDR;
    unsigned long pcie_mem_size = RTK_PCIE_DMA_SIZE;
    int dma_ret;
#endif

    ic_ver = get_ic_version();
    PCIE_ALERT("RTK TV PCIE (Ver. %d) 2022/08/24\n",ic_ver);
    if(!pcie_enable){
        rtd_pr_pcie_alert("need bootcode paramter \"pcie_enable\" to enable PCIE\n",__func__,__LINE__);
        return -ENODEV;
    }

    rtk_pcie_load_pcb_cfg();

    rtk_pcie_hold_perst();
    rtk_pcie_dev = dev;

    pcie = devm_kzalloc(dev, sizeof(*pcie), GFP_KERNEL);
    if (!pcie){
        return -ENOMEM;
    }

    pcie->ic_ver = ic_ver;
    pci = &pcie->pci;
    pci_local = pci;
    pci->dev = &pdev->dev;
    pci->ops = &rtk_dw_pcie_ops;
    pp = &pci->pp;
    pcie->dev = &pdev->dev;

    ret = rtk_pcie_dw_parse_dt(pcie);
    if (ret < 0) {
        dev_err(dev, "Failed to parse device tree: %d\n", ret);
        return ret;
    }

#ifdef RTK_PCIE_DMA
    dma_ret = dma_declare_coherent_memory(dev, pcie_mem_addr,
                        pcie_mem_addr, pcie_mem_size);

    dev_err(dev, "Declare coherent memory for RTK PCIE %s, ret = 0x%x, mem_addr = 0x%lx\n, size = %ld\n",
                        (dma_ret)?"fail":"success", dma_ret,pcie_mem_addr, pcie_mem_size);
    if (dma_ret) {
        ret = -1;
        goto fail;
    }
#endif

    pcie->phy_count = 1;

    dbi_res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dbi");
    if (!dbi_res) {
        dev_err(dev, "Failed to find \"dbi\" region\n");
        return -ENODEV;
    }
    pcie->dbi_res = dbi_res;

    pci->dbi_base = devm_ioremap_resource(dev, dbi_res);
    if (IS_ERR(pci->dbi_base)){
        return PTR_ERR(pci->dbi_base);
    }

    pcie->wrap_res = platform_get_resource_byname(pdev, IORESOURCE_MEM,"wrap");
    if (!pcie->wrap_res) {
        dev_err(dev, "Failed to find \"wrap\" region\n");
        return -ENODEV;
    }
    pcie->wrap_base = devm_ioremap_resource(dev, pcie->wrap_res);

    if (IS_ERR(pcie->wrap_base)){
        return PTR_ERR(pcie->wrap_base);
    }

	pp->irq = platform_get_irq_byname(pdev, "intr");
	if (pp->irq < 0) {
		dev_err(dev, "failed to get irq\n");
		return pp->irq;
	}
	ret = devm_request_irq(dev, pp->irq, rtk_pcie_irq_handler,
				IRQF_SHARED, "rtk-pcie-intr", pcie);
	if (ret) {
		dev_err(dev, "failed to request irq\n");
		return ret;
	}

	if (IS_ENABLED(CONFIG_PCI_MSI)) {
		pp->msi_irq = platform_get_irq_byname(pdev, "msi");
		if (pp->msi_irq < 0) {
			dev_err(dev, "failed to get msi irq\n");
			return pp->msi_irq;
		}
	}

    platform_set_drvdata(pdev, pcie);

    ret = rtk_pcie_config_rp(pcie);
    if (ret && ret != -ENOMEDIUM){
        goto fail;
    }

    device_create_file(&pdev->dev, &dev_attr_rw_req_delay);
    device_create_file(&pdev->dev, &dev_attr_pcie_reset_gpio);
    device_create_file(&pdev->dev, &dev_attr_disable_rbus_timeout);

    return 0;

fail:
    /* anything need to disable ?? */
    return ret;
}

static int __exit rtk_pcie_dw_remove(struct platform_device *pdev)
{
    struct rtk_pcie_dw *pcie = platform_get_drvdata(pdev);

    device_remove_file(&pdev->dev, &dev_attr_rw_req_delay);
    device_remove_file(&pdev->dev, &dev_attr_pcie_reset_gpio);
    device_remove_file(&pdev->dev, &dev_attr_disable_rbus_timeout);

    rtk_pcie_deinit_controller(pcie);
    pm_runtime_put_sync(pcie->dev);
    pm_runtime_disable(pcie->dev);

    return 0;
}

static int rtk_pcie_dw_suspend_late(struct device *dev)
{
    /* reset PCIE CRT and hold clock?? */
    rtk_pci_crt_enable(0);

    return 0;

}

static int rtk_pcie_dw_suspend_noirq(struct device *dev)
{
    struct rtk_pcie_dw *pcie = dev_get_drvdata(dev);

    /* save any information that needed */
    /* disable MAC ?? */
    /* disable phy */
    return __deinit_controller(pcie);

}

static int rtk_pcie_dw_resume_noirq(struct device *dev)
{
    struct rtk_pcie_dw *pcie = dev_get_drvdata(dev);
    int ret;

    ret = rtk_pcie_config_controller(pcie);
    if (ret < 0)
            return ret;

    ret = rtk_pcie_dw_host_init(&pcie->pci.pp);
    if (ret < 0) {
            dev_err(dev, "Failed to init host: %d\n", ret);
            goto fail_host_init;
    }

    /* Restore MSI interrupt vector */
    /* or anything ?? */

    return 0;

fail_host_init:
    return __deinit_controller(pcie);

}

static int rtk_pcie_dw_resume_early(struct device *dev)
{
    /* Disable HW_HOT_RST mode */
    return 0;
}

static void rtk_pcie_dw_shutdown(struct platform_device *pdev)
{
    struct rtk_pcie_dw *pcie = platform_get_drvdata(pdev);

    disable_irq(pcie->pci.pp.irq);
    if (IS_ENABLED(CONFIG_PCI_MSI))
            disable_irq(pcie->pci.pp.msi_irq);

    __deinit_controller(pcie);

}

static const struct of_device_id rtk_pcie_dw_of_match[] = {
    { .compatible = "realtek,rtk-pcie" },
    {},
};

static const struct dev_pm_ops rtk_pcie_dw_pm_ops = {
    .suspend_late   = rtk_pcie_dw_suspend_late,
    .suspend_noirq  = rtk_pcie_dw_suspend_noirq,
    .resume_noirq   = rtk_pcie_dw_resume_noirq,
    .resume_early   = rtk_pcie_dw_resume_early,
};

static struct platform_driver rtk_pcie_dw_driver = {
    .probe = rtk_pcie_dw_probe,
    .remove = __exit_p(rtk_pcie_dw_remove),
    .shutdown = rtk_pcie_dw_shutdown,
    .driver = {
        .name   = "rtk-pcie",
        .pm = &rtk_pcie_dw_pm_ops,
        .of_match_table = rtk_pcie_dw_of_match,
    },
};

module_platform_driver(rtk_pcie_dw_driver);
MODULE_DEVICE_TABLE(of, rtk_pcie_dw_of_match);

MODULE_AUTHOR("Alexkh <alexkh@realtek.com>");
MODULE_DESCRIPTION("Realtek PCIe host controller driver");
MODULE_LICENSE("GPL v2");

#ifndef CONFIG_RTK_KDRV_PCIE_MODULE
u32 pcie_enable = 0;
static int rtk_pcie_enable_chk(char *buf)
{
    PCIE_ALERT("\n=== bootcode enable PCIE ===\n");
    pcie_enable  = 1;

    return 0;
}

early_param("pcie_enable",rtk_pcie_enable_chk);

static int rtk_pcie_link_speed_chk(char *buf)
{
    int val;

    PCIE_ALERT("\n=== bootcode force PCIE link speed (%s)===\n", buf);

    if (sscanf(buf, "%d", &val)==1)
    {
        switch (val)
        {
        case GEN1:
        case GEN2:
            pcie_link_speed  = val;
            break;
        default:
            rtd_pr_pcie_alert("%s(%d) invalid link speed - %d, use default %d\n",__func__,__LINE__, val, pcie_link_speed);
        }
    }

    return 0;
}

early_param("pcie_link_speed",rtk_pcie_link_speed_chk);

#else
u32 pcie_enable = 1;
module_param_named(pcie_link_speed, pcie_link_speed, uint, S_IRUGO | S_IWUSR);
#endif