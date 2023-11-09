#ifndef __XHCI_PLAT_RTD287X_CONFIG_H__
#define __XHCI_PLAT_RTD287X_CONFIG_H__
#include <rtk_kdriver/rtk_gpio.h>

#define MAX_XHCI_USB3_PHY_NUM 2

/*---------------------------------------------------------
 * Refer to DesignWare Cores SuperSpeed USB 3.0 xHCI Host Contoller Databook,Version 2.70a
 * 6.1.3 Global Registers Map
 *-------------------------------------------------------- */
struct gevnt {
	__le64 addr;
	__le32 size;
	__le32 count;
};
struct xhci_global_regs {
	__le32 gsbuscfg0; /* offset 0xc100 */
	__le32 gsbuscfg1;
	__le32 gtxthrcgs;
	__le32 grxthrcgs;

	__le32 gctl; //0xC110
	__le32 gpmsts;
	__le32 gsts;
	__le32 guctl1;

	__le32 gsnpsid; //0xC120
	__le32 ggpio;
	__le32 guid;
	__le32 guctl;

	__le64 gbuserraddr; //0xC130
	__le64 gprtbimap;

	__le32 ghw_params0; //0xC140
	__le32 ghw_params1;
	__le32 ghw_params2;
	__le32 ghw_params3;

	__le32 ghw_params4; //0xC150
	__le32 ghw_params5;
	__le32 ghw_params6;
	__le32 ghw_params7;

	__le32 gdbgfifospace; //0xC160
	__le32 gdbgltssm;
	__le32 gdbglnmcc;
	__le32 gdbgbmu;

	__le32 gdbglspmux; //0xC170
	__le32 gdbglsp;
	__le32 gdbgepinfo0;
	__le32 gdbgepinfo1;

	__le64 gprtbimap_hs; //0xC180
	__le64 gprtbimap_fs;

	__le32 rsvd1; //0xC190
	__le32 gerrinjctl_1; //0xC194
	__le32 gerrinjctl_2;

	__le32 rsvd2[9];

	__le32 gusb3rmmictl[16]; //0xc1c0

	__le32 gusb2phycfg[16]; //0xc200

	__le32 gusb2i2cctl[16]; //0xc240

	__le32 gusb2phyacc[16]; //0xc280
#define VSTS_DONE    (1 << 24)
#define IS_VSTS_DONE(p)           (p & VSTS_DONE)

	__le32 gusb3pipectl[16]; //0xc2c0
	__le32 gtxfifosiz[32]; //0xc300
	__le32 grxfifosiz[32]; //0xc380
	struct gevnt genvt_regs[32]; //0xc400
	__le32 ghw_params8; //0xc600
	__le32 rsvd3[5];
	__le32 gtxfifoprihst; //0xc618
	__le32 grxfifoprihst; //0xc61c
	__le32 gfifopridbc; //0xc620
	__le32 gdmahlratio; //0xc624
	__le32 rsvd4[2];

	__le32 gfladj; //0xc630
};

struct XHCI_WRAPPER_REGS
{
	//xhci some global regs
	unsigned int XHCI_USB3_TOP_GSBUSCFG0_REG;
	unsigned int XHCI_USB3_TOP_GSBUSCFG1_REG;
	unsigned int XHCI_USB3_TOP_GCTL_REG;
	unsigned int XHCI_USB3_TOP_GUSB2PHYCFG0_REG;
	unsigned int XHCI_USB3_TOP_GUSB2PHYCFG1_REG;

	//usb3 wrapper regs
	unsigned int XHCI_USB3_TOP_WRAP_CTR_REG;
	unsigned int XHCI_USB3_TOP_GNR_INT_REG;
	unsigned int XHCI_USB3_TOP_USB2_PHY_UTMI_REG;
	unsigned int XHCI_USB3_TOP_USB3_PHY_PIPE_REG;
	unsigned int XHCI_USB3_TOP_MDIO_CTR_REG;
	unsigned int XHCI_USB3_TOP_VSTATUS0_OUT_REG;
	unsigned int XHCI_USB3_TOP_SLP_BACK0_EN_REG;
	unsigned int XHCI_USB3_TOP_SLP_BACK0_CTR_REG;
	unsigned int XHCI_USB3_TOP_SLP_BACK0_ST_REG;
	unsigned int XHCI_USB3_TOP_PHY2_SLB0_EN_REG;
	unsigned int XHCI_USB3_TOP_PHY2_SLB0_ST_REG;
	unsigned int XHCI_USB3_TOP_SLP_BACK1_CTR_REG;
	unsigned int XHCI_USB3_TOP_SLP_BACK1_ST_REG;
	unsigned int XHCI_USB3_TOP_PHY2_SLB1_EN_REG;
	unsigned int XHCI_USB3_TOP_PHY2_SLB1_ST_REG;
	unsigned int XHCI_USB3_TOP_MDIO_CTR_PORT1_REG;
	unsigned int XHCI_USB3_TOP_USB2_SPD_CTR_REG;
	unsigned int XHCI_USB3_TOP_PHY3_SLB_EN_REG;
	unsigned int XHCI_USB3_TOP_PHY3_SLB_CT_REG;
	unsigned int XHCI_USB3_TOP_PHY3_SLB_ST_REG;
	unsigned int XHCI_USB3_TOP_USB_DBG_REG;
	unsigned int XHCI_USB3_TOP_USB_SCTCH_REG;
	unsigned int XHCI_USB3_TOP_USB_TMP_SP_0_REG;
	unsigned int XHCI_USB3_TOP_USB_TMP_0_REG;
	unsigned int XHCI_USB3_TOP_USB_TMP_1_REG;
	unsigned int XHCI_USB3_TOP_USB_TMP_2_REG;
	unsigned int XHCI_USB3_TOP_USB_TMP_3_REG;
	unsigned int XHCI_USB3_TOP_HMAC_CTR0_REG;
	unsigned int XHCI_USB3_TOP_HMAC_CTR1_REG;
	unsigned int XHCI_USB3_TOP_MAC3_HST_ST_REG;
	unsigned int XHCI_USB3_TOP_USB2_PHY0_REG;
	unsigned int XHCI_USB3_TOP_USB2_PHY1_REG;
	unsigned int XHCI_USB3_TOP_RAM_CTR_REG;
	unsigned int XHCI_USB3_TOP_RAM_ADDR_REG;
	unsigned int XHCI_USB3_TOP_RAM_WDATA_REG;
	unsigned int XHCI_USB3_TOP_RAM_RDATA_REG;
	unsigned int XHCI_USB3_TOP_USB3_OVR_CT_REG;
	unsigned int XHCI_USB3_TOP_ANA_PHY0_REG;
	unsigned int XHCI_USB2_TOP_ANA_PHY1_REG;
	unsigned int XHCI_USB3_TOP_GBL_USB_CT_REG;
	unsigned int XHCI_USB3_TOP_CRC_CNT_1_REG;
	unsigned int XHCI_USB3_TOP_CRC_CNT_2_REG;
	
	
};

typedef struct {
    unsigned char port;
    unsigned char  page;
    unsigned char  reg;
    unsigned short val;
} U3_PHY_REGISTER;


typedef struct {
    int           port1;
    unsigned char addr;
    unsigned char val;
} U2_PHY_REGISTER;


#define Z0_GEAR(val)   (3+((val&0xf)*4))
//[GEAR]->[Value]:
//0x0->0x03;0x1->0x07;0x2->0x0b;0x3->0x0f;0x4->0x13;0x5->0x17;0x6->0x1b;0x7->0x1f
//0x8->0x23;0x9->0x27;0xa->0x2b;0xb->0x2f;0xc->0x33;0xd->0x37;0xe->0x3b;0xf->0x3f

#define Z0_XHCI_PHY_NAME        "Z0_XHCI_VALUE"

#endif // __XHCI_PLAT_RTD287X_CONFIG_H__
