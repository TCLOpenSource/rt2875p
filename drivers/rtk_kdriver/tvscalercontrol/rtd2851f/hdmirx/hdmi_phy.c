/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2016
 *
 * All rights reserved.
 *
 *============================================================*/

/*======================= Description ============================
 *
 * file: 		hdmiPhy.c
 *
 * author: 	Iron Man, Tony Stark
 * date:
 * version: 	3.0
 *
 *============================================================*/

/*========================Header Files============================*/
#ifndef BUILD_QUICK_SHOW
#include <mach/platform.h>
#endif
#include "hdmi_common.h"
#include "hdmi_phy.h"
#include "hdmi_phy_dfe.h"
#include "hdmi_hdcp.h"
#include "hdmi_scdc.h"
#include "hdmi_reg.h"
#include "hdmi_vfe_config.h"
#include "hdmi_debug.h"

#include "hdmi_reg_phy.h"   // for HDMI PHY
#include "hdmi_reg_phy_dfe.h"   //for DFE PHY


extern HDMI_PORT_INFO_T hdmi_rx[HDMI_PORT_TOTAL_NUM];

/**********************************************************************************************
*
*	Marco or Definitions
*
**********************************************************************************************/

#ifndef IRQ_HANDLED
#define IRQ_HANDLED  1
#define IRQ_NONE     0
#endif

#define CLOCK_TOLERANCE(port)		(phy_st[port].clk_tolerance)
//#define CLK_DEBOUNCE_CNT_THD		7       // Set 2, it will easy to set phy in background and influence others.
#define CLK_DEBOUNCE_SHORT_CNT_THD		1       // to prevent Tx HDCP handshake too fast
//#define NOCLK_DEBOUNCE_CNT		8       // count for confirm it's no clock, (QD980>5, has sudden no clock and clear scdc)
#define NOCLK_DEBOUNCE_CNT		13	// count for confirm it's no clock, (Panasonic DMR-UBZ1 >13, has sudden no clock and clear scdc)
#define CLK_UNSTABLE_CNT		2       // count for clock unstable change

#define TMDS_CHAR_ERR_PERIOD		4
#define TMDS_CHAR_ERR_THREAD		8
#define TMDS_CHAR_ERR_MAX_LOOP 15  //(4*1/23hz * 1000) / 15ms (one checkmode time) = 11.5 --> 15

#define TMDS_BIT_ERR_PERIOD		16
#define TMDS_BIT_ERR_THREAD		32
#define TMDS_BIT_ERR_MAX_LOOP          50      //(16 * 1/23hz *1000) / 15ms (one check mode time) =  46.3 --> 50

#define MAX_ERR_DETECT_TIMES	10
#define TMDS_BIT_ERR_SIGNAL_BAD_THREAD	1000

#define PHY_P0_Z0 		0xe //Soc Target Z0 90 ohm
#define PHY_P1_Z0		0xa//Soc Target Z0 90 ohm
#define PHY_P2_Z0		0xa//Soc Target Z0 90 ohm
#define HDMI_EQ_LE_NOT_ZERO_ENABLE
#define CLK_OVER_300M			1

#define HPD_R1K_SEL	0x1 //HPD_R1K_SEL = 0x0~0x7 (0.85Kohm~1.0Kohm)



#ifndef MAX
    #define MAX(a,b)                (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
    #define MIN(a,b)                (((a) < (b)) ? (a) : (b))
#endif
/////////////////////////////////

u32 phy_reg_mapping(u32 nport, u32 addr)
{
	if (addr >= 0x1800DE00 && addr <= 0x1800DE4C)//DFE-port0
	{
		addr -= 0x1800DE00;
		switch(nport) {
		case 0:  addr += 0xB800DE00; break;
		//case 1:  addr += 0xB80B9C00; break;
		//case 2:  addr += 0xB80BAC00; break;
		//case 3:  addr += 0xB80BCC00; break;
		default: addr += 0xB800DE00; break;
        	}
	}
	else if (addr >= 0x1803BE00 && addr <= 0x1803BEFF)//DFE-port1 & port2
	{
		addr -= 0x1803BE00;
		switch(nport) {
		//case 0:  addr += 0xB80B7C00; break;
		case 1:  addr += 0xB803BE00; break;
		case 2:  addr += 0xB803CE00; break;
		//case 3:  addr += 0xB80BCC00; break;
		default: addr += 0xB803BE00; break;
        	}
	}
	else if (addr >= 0x1800DA00 && addr <= 0x1800DDFF)//PHY-port0
	{
		addr -= 0x1800DA00;
		switch(nport) {
		case 0:  addr += 0xB800DA00; break;
		//case 1:  addr += 0xB803BA00; break;
		//case 2:  addr += 0xB803CA00; break;
		//case 3:  addr += 0xB80B2A00; break;
		default: addr += 0xB800DA00; break;
		}
	}
	else if (addr >= 0x183BA00 && addr <= 0x1803BDFF)//PHY-port1 & port2
	{
		addr -= 0x1803BA00;
		switch(nport) {
		//case 0:  addr += 0xB800DA00; break;
		case 1:  addr += 0xB803BA00; break;
		case 2:  addr += 0xB803CA00; break;
		//case 3:  addr += 0xB80B2A00; break;
		default: addr += 0xB803BA00; break;
		}
	}
	else
	{
		HDMI_WARN("%s unknown address = %08x\n", __func__, addr);
	}

	//HDMI_WARN("p[%d] addr=%08x\n", nport, addr);
	if (RBUS_BASE_PHYS <= addr && addr < (RBUS_BASE_PHYS + RBUS_BASE_SIZE)) {
		addr = addr - RBUS_BASE_PHYS + RBUS_BASE_VIRT_OLD; // translate to legacy rbus VA
	}

	return addr;
}

#define ScalerTimer_DelayXms(x)                 udelay((x<10) ? (x*100) : 5000)
#define _phy_rtd_outl(addr, val)                    rtd_outl(phy_reg_mapping(nport, addr), val)
#define _phy_rtd_part_outl(addr, msb, lsb, val)     rtd_part_outl(phy_reg_mapping(nport, addr), msb, lsb, val)
#define _phy_rtd_inl(addr, val)                     rtd_inl(phy_reg_mapping(nport, addr))
#define _phy_rtd_part_inl(addr, msb, lsb)           rtd_part_inl(phy_reg_mapping(nport, addr), msb, lsb)
#define _phy_rtd_maskl(x,y,z)  		rtd_maskl(phy_reg_mapping(nport, x), y, z)


///////////////////////////


/**********************************************************************************************
*
*	Const Declarations
*
**********************************************************************************************/

//typedef unsigned char UINT8;
//typedef char INT8;
//typedef unsigned int UINT32;
//typedef int INT32;
//typedef unsigned short UINT16;
//typedef short INT16;

/////////////////////////////////////////////////////
//pure HDMI port

#if 1
APHY_Fix_1_T APHY_Fix_1[] =
{
	{TMDS_5p94G,0x1,0x1},
	{TMDS_2p97G,0x1,0x1},
	{TMDS_1p485G,0x1,0x1},
	{TMDS_0p742G,0x1,0x1},
	{TMDS_0p25G,0x1,0x1},
	{TMDS_4p445G,0x1,0x1},
	{TMDS_3p7125G,0x1,0x1},
	{TMDS_2p2275G,0x1,0x1},
	{TMDS_1p85625G,0x1,0x1},
	{TMDS_1p11375G,0x1,0x1},
	{TMDS_0p928125G,0x1,0x1},
	{TMDS_0p54G,0x1,0x1},
	{TMDS_0p405G,0x1,0x1},
	{TMDS_0p3375G,0x1,0x1},
	{TMDS_0p27G,0x1,0x1},
	{TMDS_OLT_5p94G,0x0,0x0},
};

APHY_Param_1_T APHY_Param_1[] =
{
	{TMDS_5p94G,0x3,0x1,0x9,0x2,0x24,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,
0xf,0xf,0xf,0xf,0x3,0x3,0x3,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x1,0x2,0x2,0x2,0x7,0x7,0x7,0x7,0x7,0x7,0x3,0x3,0x3,0x1,0x1,0x1},
	{TMDS_2p97G,0x3,0x0,0xe,0x2,0x10,0x0,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x2,0x2,0x2,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0xf,0xf,0xf,0x0,0x0,0x0,0x0,0x0,
0x0,0x6,0x6,0x6,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6,0x6,0x6,0x6,0x6,0x6,0x1,0x1,0x1,0x0,0x0,0x0},
	{TMDS_1p485G,0x3,0x1,0x9,0x2,0x24,0x0,0x0,0x0,0x2,0x2,0x2,0x1,0x1,0x1,0x3,0x3,0x3,0x1,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x5,0x5,0x5,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0p742G,0x3,0x1,0xd,0x2,0x4c,0x0,0x0,0x0,0x3,0x3,0x3,0x1,0x1,0x1,0x3,0x3,0x3,0x1,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x5,0x5,0x5,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0p25G,0x0,0x1,0xf,0x3,0x9c,0x6,0x6,0x6,0x4,0x4,0x4,0x1,0x1,0x1,0x3,0x3,0x3,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x5,0x5,0x5,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_4p445G,0x2,0x1,0x9,0x2,0x24,0x3,0x3,0x3,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,
0xf,0xf,0xf,0xf,0x3,0x3,0x3,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x1,0x3,0x3,0x3,0x5,0x5,0x5,0x7,0x7,0x7,0x3,0x3,0x3,0x1,0x1,0x1},
	{TMDS_3p7125G,0x0,0x1,0x9,0x2,0x24,0x6,0x6,0x6,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,
0xf,0xf,0xf,0xf,0x3,0x3,0x3,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x1,0x3,0x3,0x3,0x5,0x5,0x5,0x7,0x7,0x7,0x3,0x3,0x3,0x1,0x1,0x1},
	{TMDS_2p2275G,0x2,0x0,0xe,0x2,0x10,0x3,0x3,0x3,0x1,0x1,0x1,0x1,0x1,0x1,0x2,0x2,0x2,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0xf,0xf,0xf,0x0,0x0,0x0,0x0,0x0,
0x0,0x6,0x6,0x6,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x2,0x2,0x2,0x3,0x3,0x3,0x1,0x1,0x1,0x0,0x0,0x0},
	{TMDS_1p85625G,0x0,0x0,0xe,0x2,0x10,0x6,0x6,0x6,0x1,0x1,0x1,0x1,0x1,0x1,0x2,0x2,0x2,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0xf,0xf,0xf,0x0,0x0,0x0,0x0,0x0,
0x0,0x6,0x6,0x6,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x2,0x2,0x2,0x3,0x3,0x3,0x1,0x1,0x1,0x0,0x0,0x0},
	{TMDS_1p11375G,0x2,0x1,0x9,0x2,0x24,0x3,0x3,0x3,0x2,0x2,0x2,0x1,0x1,0x1,0x3,0x3,0x3,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x5,0x5,0x5,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0p928125G,0x0,0x1,0x9,0x2,0x24,0x6,0x6,0x6,0x2,0x2,0x2,0x1,0x1,0x1,0x3,0x3,0x3,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x5,0x5,0x5,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0p54G,0x2,0x1,0xd,0x2,0x4c,0x3,0x3,0x3,0x3,0x3,0x3,0x1,0x1,0x1,0x3,0x3,0x3,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x5,0x5,0x5,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0p405G,0x0,0x1,0xd,0x2,0x4c,0x6,0x6,0x6,0x3,0x3,0x3,0x1,0x1,0x1,0x3,0x3,0x3,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x5,0x5,0x5,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0p3375G,0x3,0x1,0xf,0x3,0x9c,0x0,0x0,0x0,0x4,0x4,0x4,0x1,0x1,0x1,0x3,0x3,0x3,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x5,0x5,0x5,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0p27G,0x2,0x1,0xf,0x3,0x9c,0x3,0x3,0x3,0x4,0x4,0x4,0x1,0x1,0x1,0x3,0x3,0x3,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
0x0,0x5,0x5,0x5,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_OLT_5p94G,0x3,0x1,0x9,0x2,0xda,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0xf,0xf,0xf,0xf,0xf,0xf,0xf,0xf,
0xf,0xf,0xf,0xf,0x3,0x3,0x3,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x1,0x2,0x2,0x2,0x7,0x7,0x7,0x7,0x7,0x7,0x3,0x3,0x3,0x1,0x1,0x1},
};

DFE_ini_1_T DFE_ini_1[] =
{
	{TMDS_5p94G,0x37,0x14,0x14,0x14,0x19,0x19,0x19,0xa,0xa,0xa,0xa,0xa,0xa,0x3},
	{TMDS_2p97G,0x37,0x14,0x14,0x14,0x19,0x19,0x19,0xa,0xa,0xa,0xa,0xa,0xa,0x3},
	{TMDS_1p485G,0x3c,0x14,0x14,0x14,0x1e,0x1e,0x1e,0xa,0xa,0xa,0xf,0xf,0xf,0x3},
	{TMDS_0p742G,0x37,0xf,0xf,0xf,0x19,0x19,0x19,0x1,0x1,0x1,0xa,0xa,0xa,0x2},
	{TMDS_0p25G,0x37,0x19,0x19,0x19,0x19,0x19,0x19,0x0,0x0,0x0,0x0,0x0,0x0,0x3},
	{TMDS_4p445G,0x37,0x14,0x14,0x14,0x19,0x19,0x19,0xa,0xa,0xa,0xa,0xa,0xa,0x3},
	{TMDS_3p7125G,0x37,0x14,0x14,0x14,0x19,0x19,0x19,0xa,0xa,0xa,0xa,0xa,0xa,0x3},
	{TMDS_2p2275G,0x37,0x14,0x14,0x14,0x19,0x19,0x19,0xa,0xa,0xa,0xa,0xa,0xa,0x3},
	{TMDS_1p85625G,0x37,0x14,0x14,0x14,0x19,0x19,0x19,0xa,0xa,0xa,0xa,0xa,0xa,0x3},
	{TMDS_1p11375G,0x37,0x14,0x14,0x14,0x19,0x19,0x19,0xa,0xa,0xa,0xa,0xa,0xa,0x3},
	{TMDS_0p928125G,0x37,0x14,0x14,0x14,0x19,0x19,0x19,0xa,0xa,0xa,0xa,0xa,0xa,0x3},
	{TMDS_0p54G,0x37,0xf,0xf,0xf,0x19,0x19,0x19,0x1,0x1,0x1,0xa,0xa,0xa,0x2},
	{TMDS_0p405G,0x37,0xf,0xf,0xf,0x19,0x19,0x19,0x1,0x1,0x1,0xa,0xa,0xa,0x2},
	{TMDS_0p3375G,0x37,0x14,0x14,0x14,0x19,0x19,0x19,0xa,0xa,0xa,0xa,0xa,0xa,0x3},
	{TMDS_0p27G,0x37,0x14,0x14,0x14,0x19,0x19,0x19,0xa,0xa,0xa,0xa,0xa,0xa,0x3},
	{TMDS_OLT_5p94G,0x37,0x14,0x14,0x14,0x19,0x19,0x19,0xa,0xa,0xa,0xa,0xa,0xa,0x3},
};

DPHY_Param_1_T DPHY_Param_1[] =
{
	{TMDS_5p94G,0x0,0x2,0x1,0x0},
	{TMDS_2p97G,0x0,0x6,0x1,0x1},
	{TMDS_1p485G,0x0,0x6,0x0,0x2},
	{TMDS_0p742G,0x0,0x8,0x0,0x2},
	{TMDS_0p25G,0x0,0x8,0x0,0x2},
	{TMDS_4p445G,0x3,0x4,0x1,0x0},
	{TMDS_3p7125G,0x3,0x4,0x1,0x0},
	{TMDS_2p2275G,0x3,0x3,0x1,0x1},
	{TMDS_1p85625G,0x3,0x3,0x1,0x1},
	{TMDS_1p11375G,0x3,0x3,0x0,0x2},
	{TMDS_0p928125G,0x3,0x3,0x0,0x2},
	{TMDS_0p54G,0x3,0x3,0x0,0x2},
	{TMDS_0p405G,0x3,0x3,0x0,0x2},
	{TMDS_0p3375G,0x3,0x3,0x0,0x2},
	{TMDS_0p27G,0x3,0x3,0x0,0x2},
	{TMDS_OLT_5p94G,0x0,0x2,0x1,0x0},
};

LEQ_TAP0_Adapt_1_T LEQ_TAP0_Adapt_1[] =
{
	{TMDS_5p94G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_2p97G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_1p485G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p742G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p25G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_4p445G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_3p7125G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_2p2275G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_1p85625G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_1p11375G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p928125G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_0p54G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p405G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p3375G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0p27G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_OLT_5p94G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
};

Manual_DFE_1_T Manual_DFE_1[] =
{
	{TMDS_5p94G,0x1f,0x1f,0x5,0x5,0x5},
	{TMDS_2p97G,0x1f,0x1f,0x5,0x5,0x5},
	{TMDS_1p485G,0x1f,0x1f,0x5,0x5,0x5},
	{TMDS_0p742G,0x1f,0x1f,0x5,0x5,0x5},
	{TMDS_0p25G,0x1f,0x1f,0x5,0x5,0x5},
	{TMDS_4p445G,0x1f,0x1f,0x5,0x5,0x5},
	{TMDS_3p7125G,0x1f,0x1f,0x5,0x5,0x5},
	{TMDS_2p2275G,0x1f,0x1f,0x5,0x5,0x5},
	{TMDS_1p85625G,0x1f,0x1f,0x5,0x5,0x5},
	{TMDS_1p11375G,0x1f,0x1f,0x5,0x5,0x5},
	{TMDS_0p928125G,0x1f,0x1f,0x5,0x5,0x5},
	{TMDS_0p54G,0x1f,0x1f,0x5,0x5,0x5},
	{TMDS_0p405G,0x1f,0x1f,0x5,0x5,0x5},
	{TMDS_0p3375G,0x1f,0x1f,0x5,0x5,0x5},
	{TMDS_0p27G,0x1f,0x1f,0x5,0x5,0x5},
	{TMDS_OLT_5p94G,0x3f,0x0,0x0,0x0,0x0},
};

TAP0_2_Adapt_1_T TAP0_2_Adapt_1[] =
{
	{TMDS_5p94G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_2p97G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_1p485G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p742G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p25G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_4p445G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_3p7125G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_2p2275G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_1p85625G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_1p11375G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p928125G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p54G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p405G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p3375G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0p27G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_OLT_5p94G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
};

void Mac8p_Pure_HDMI_TMDS_Main_Seq(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	HDMI_WARN("Pure_HDMI_TMDS_2023_04_06\n");
	if(value_param==TMDS_5p94G || value_param==TMDS_2p97G || value_param==TMDS_1p485G || value_param==TMDS_0p742G || value_param==TMDS_4p445G || value_param==TMDS_3p7125G || value_param==TMDS_2p2275G || value_param==TMDS_1p85625G || value_param==TMDS_1p11375G || value_param==TMDS_0p928125G || value_param==TMDS_0p54G || value_param==TMDS_0p405G )
	{
		DPHY_Fix_1_func(nport, value_param);
		DPHY_Param_1_func(nport, value_param);
		DPHY_Init_Flow_1_func(nport, value_param);
		APHY_Fix_1_func(nport, value_param);
		APHY_Param_1_func(nport, value_param);
		APHY_Init_Flow_1_func(nport, value_param);
		DCDR_settings_1_func(nport, value_param);
		DFE_ini_1_func(nport, value_param);
		Koffset_1_func(nport, value_param);
		DCDR_settings_2_func(nport, value_param);
		LEQ_TAP0_Adapt_1_func(nport, value_param);
		Koffset_1_func(nport, value_param);
		DCDR_settings_2_func(nport, value_param);
		TAP0_2_Adapt_1_func(nport, value_param);
		Koffset_1_func(nport, value_param);
		DCDR_settings_2_func(nport, value_param);
	}
	else if(value_param==TMDS_0p25G || value_param==TMDS_0p3375G || value_param==TMDS_0p27G )
	{
		DPHY_Fix_1_func(nport, value_param);
		DPHY_Param_1_func(nport, value_param);
		DPHY_Init_Flow_1_func(nport, value_param);
		APHY_Fix_1_func(nport, value_param);
		APHY_Param_1_func(nport, value_param);
		APHY_Init_Flow_1_func(nport, value_param);
		DCDR_settings_1_func(nport, value_param);
		DFE_ini_1_func(nport, value_param);
		Koffset_1_func(nport, value_param);
		DCDR_settings_2_func(nport, value_param);
		Manual_DFE_1_func(nport, value_param);
		Koffset_1_func(nport, value_param);
		DCDR_settings_2_func(nport, value_param);
	}
	else if(value_param==TMDS_OLT_5p94G )
	{
		DPHY_Fix_1_func(nport, value_param);
		DPHY_Param_1_func(nport, value_param);
		DPHY_Init_Flow_1_func(nport, value_param);
		APHY_Fix_1_func(nport, value_param);
		APHY_Param_1_func(nport, value_param);
		APHY_Init_Flow_1_func(nport, value_param);
		DCDR_settings_1_func(nport, value_param);
		Manual_DFE_1_func(nport, value_param);
	}
}

void DPHY_Fix_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	//VCO band manual
	_phy_rtd_part_outl(0x1800db68, 27, 24, 8); //reg_p0_ck_vco_coarse_manual
	_phy_rtd_part_outl(0x1800db68, 23, 20, 0); //reg_p0_ck_vco_fine_manual
}

void DPHY_Param_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	//DPHY DCDR KP&KI
	_phy_rtd_part_outl(0x1800db00, 9, 8, DPHY_Param_1[value_param].KP_2_1); //DCDR KP format 4.2 (.2)
	_phy_rtd_part_outl(0x1800db00, 13, 10, DPHY_Param_1[value_param].KP_3_1); //DCDR KP format 4.2 (4)
	_phy_rtd_part_outl(0x1800db00, 7, 5, DPHY_Param_1[value_param].KI_4_1); //DCDR KI
	_phy_rtd_part_outl(0x1800db00, 19, 17, DPHY_Param_1[value_param].RATE_SEL_5_1); //REG_P0_RATE_SEL
}

void DPHY_Init_Flow_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	//VCO band manual
	_phy_rtd_part_outl(0x1800db68, 1, 1, 1); //reg_p0_ck_fld_rstb
	_phy_rtd_part_outl(0x1800db58, 31, 31, 0); //reg_p0_ck_auto_mode
}

void APHY_Fix_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	//TOP 
	_phy_rtd_part_outl(0x1800da14, 0, 0, 1); //REG_P0_Z0_Z0POW_FIX
	_phy_rtd_part_outl(0x1800da14, 27, 27, 0); //REG_Z0_N_OFF
	_phy_rtd_part_outl(0x1800da14, 28, 28, 0); //REG_Z0_P_OFF
	_phy_rtd_part_outl(0x1800da14, 26, 26, 0); //REG_Z0_FT_PN_SHORT_EN
	//_phy_rtd_part_outl(0x1800da18, 4, 0, 0xb); //REG_P0_Z0_ADJR Set by OTP Trim value
	_phy_rtd_part_outl(0x1800da04, 14, 14, 0); //REG_FAST_SW_DIV_EN
	_phy_rtd_part_outl(0x1800da04, 17, 15, 0); //REG_FAST_SW_DIV_SEL
	_phy_rtd_part_outl(0x1800da04, 0, 0, 1); //REG_HDMIRX_BIAS_EN
	_phy_rtd_part_outl(0x1800da04, 1, 1, 1); //REG_CK_LATCH
	_phy_rtd_part_outl(0x1800da04, 3, 3, 1); //REG_P0_CK_TX_EN
	//CK lane
	_phy_rtd_part_outl(0x1800da20, 9, 8, 2); //REG_P0_CK_SEL_VCO_VDAC
	_phy_rtd_part_outl(0x1800da20, 21, 19, 2); //REG_P0_CK_SEL_LPF_IDAC
	_phy_rtd_part_outl(0x1800da20, 25, 25, 0); //REG_P0_CK_RES_SHORT
	_phy_rtd_part_outl(0x1800da20, 27, 27, 0); //REG_P0_CK_SHORT_VCTRLS
	_phy_rtd_part_outl(0x1800da20, 28, 28, 1); //REG_P0_CK_SEL_CS
	_phy_rtd_part_outl(0x1800da20, 29, 29, 1); //REG_P0_CK_SEL_CP
	_phy_rtd_part_outl(0x1800da24, 11, 11, 1); //REG_P0_CK_SEL_HS_CLK
	_phy_rtd_part_outl(0x1800da24, 12, 12, 0); //REG_P0_CK_SEL_DIV_BAND_2OR4
	_phy_rtd_part_outl(0x1800da24, 14, 13, 0); //REG_P0_CK_SEL_DIV_PLL
	_phy_rtd_part_outl(0x1800da24, 16, 15, 0); //REG_P0_CK_SEL_DIV_REF
	_phy_rtd_part_outl(0x1800da24, 18, 17, 1); //REG_P0_CK_SEL_DIV_FLD
	_phy_rtd_part_outl(0x1800da28, 0, 0, APHY_Fix_1[value_param].CK_CKXTAL_SEL_24_1); //REG_P0_CK_CKXTAL_SEL
	_phy_rtd_part_outl(0x1800da28, 1, 1, APHY_Fix_1[value_param].CK_SEL_CKIN_25_1); //REG_P0_CK_SEL_CKIN
	_phy_rtd_part_outl(0x1800da28, 2, 2, 0); //REG_P0_CK_MD_HST_SEL
	_phy_rtd_part_outl(0x1800da28, 3, 3, 1); //REG_P0_CK_EN_NON_HST_AMP
	_phy_rtd_part_outl(0x1800da28, 4, 4, 1); //REG_P0_CK_EN_HST_AMP
	_phy_rtd_part_outl(0x1800da28, 6, 5, 3); //REG_P0_CK_EN_ENHANCE_BIAS
	_phy_rtd_part_outl(0x1800da28, 13, 13, 1); //REG_P0_CK_RESERV
	_phy_rtd_part_outl(0x1800da20, 0, 0, 1); //REG_P0_CK_POW_AC_COUPLE
	_phy_rtd_part_outl(0x1800da20, 2, 1, 1); //REG_P0_CK_ACC_RXVCM_SEL
	_phy_rtd_part_outl(0x1800da20, 3, 3, 0); //REG_P0_CK_ACC_INOFF_EN
	_phy_rtd_part_outl(0x1800da20, 4, 4, 0); //REG_P0_CK_ACC_INPOFF_SINGLE_EN
	_phy_rtd_part_outl(0x1800da20, 5, 5, 0); //REG_P0_CK_ACC_INNOFF_SINGLE_EN
	_phy_rtd_part_outl(0x1800da20, 6, 6, 1); //REG_P0_CK_POW_VCO
	_phy_rtd_part_outl(0x1800da20, 7, 7, 1); //REG_P0_CK_POW_VCO_VDAC
	_phy_rtd_part_outl(0x1800da20, 18, 18, 1); //REG_P0_CK_POW_LPF_IDAC
	_phy_rtd_part_outl(0x1800da20, 12, 12, 1); //REG_P0_CK_POW_CP
	_phy_rtd_part_outl(0x1800da24, 28, 28, 0); //REG_P0_CK_WDPOW
	//BGR lane
	_phy_rtd_part_outl(0x1800da30, 8, 6, 3); //REG_P0_B_DA_LANE_ISEL
	_phy_rtd_part_outl(0x1800da40, 8, 6, 3); //REG_P0_B_DA_LANE_ISEL
	_phy_rtd_part_outl(0x1800da50, 8, 6, 3); //REG_P0_B_DA_LANE_ISEL
	_phy_rtd_part_outl(0x1800da30, 11, 10, 3); //REG_P0_B_LE_EQ_CUR_ADJ
	_phy_rtd_part_outl(0x1800da40, 11, 10, 3); //REG_P0_B_LE_EQ_CUR_ADJ
	_phy_rtd_part_outl(0x1800da50, 11, 10, 3); //REG_P0_B_LE_EQ_CUR_ADJ
	_phy_rtd_part_outl(0x1800da38, 19, 19, 0); //REG_P0_B_PI_CUR_ADJ
	_phy_rtd_part_outl(0x1800da48, 19, 19, 0); //REG_P0_B_PI_CUR_ADJ
	_phy_rtd_part_outl(0x1800da58, 19, 19, 0); //REG_P0_B_PI_CUR_ADJ
	_phy_rtd_part_outl(0x1800da38, 3, 1, 0); //REG_P0_B_DFE_PREAMP_ISEL
	_phy_rtd_part_outl(0x1800da48, 3, 1, 0); //REG_P0_B_DFE_PREAMP_ISEL
	_phy_rtd_part_outl(0x1800da58, 3, 1, 0); //REG_P0_B_DFE_PREAMP_ISEL
	_phy_rtd_part_outl(0x1800da38, 7, 5, 6); //REG_P0_B_DFE_SUMAMP_ISEL
	_phy_rtd_part_outl(0x1800da48, 7, 5, 6); //REG_P0_B_DFE_SUMAMP_ISEL
	_phy_rtd_part_outl(0x1800da58, 7, 5, 6); //REG_P0_B_DFE_SUMAMP_ISEL
	_phy_rtd_part_outl(0x1800da38, 10, 8, 0); //REG_P0_B_DFE_TAP_DELAY
	_phy_rtd_part_outl(0x1800da48, 10, 8, 0); //REG_P0_B_DFE_TAP_DELAY
	_phy_rtd_part_outl(0x1800da58, 10, 8, 0); //REG_P0_B_DFE_TAP_DELAY
	_phy_rtd_part_outl(0x1800da30, 0, 0, 1); //REG_P0_B_POW_AC_COUPLE
	_phy_rtd_part_outl(0x1800da40, 0, 0, 1); //REG_P0_B_POW_AC_COUPLE
	_phy_rtd_part_outl(0x1800da50, 0, 0, 1); //REG_P0_B_POW_AC_COUPLE
	_phy_rtd_part_outl(0x1800da30, 2, 1, 1); //REG_P0_B_ACC_RXVCM_SEL
	_phy_rtd_part_outl(0x1800da40, 2, 1, 1); //REG_P0_B_ACC_RXVCM_SEL
	_phy_rtd_part_outl(0x1800da50, 2, 1, 1); //REG_P0_B_ACC_RXVCM_SEL
	_phy_rtd_part_outl(0x1800da30, 3, 3, 0); //REG_P0_B_ACC_INOFF_EN
	_phy_rtd_part_outl(0x1800da40, 3, 3, 0); //REG_P0_B_ACC_INOFF_EN
	_phy_rtd_part_outl(0x1800da50, 3, 3, 0); //REG_P0_B_ACC_INOFF_EN
	_phy_rtd_part_outl(0x1800da30, 4, 4, 0); //REG_P0_B_ACC_INPOFF_SINGLE_EN
	_phy_rtd_part_outl(0x1800da40, 4, 4, 0); //REG_P0_B_ACC_INPOFF_SINGLE_EN
	_phy_rtd_part_outl(0x1800da50, 4, 4, 0); //REG_P0_B_ACC_INPOFF_SINGLE_EN
	_phy_rtd_part_outl(0x1800da30, 5, 5, 0); //REG_P0_B_ACC_INNOFF_SINGLE_EN
	_phy_rtd_part_outl(0x1800da40, 5, 5, 0); //REG_P0_B_ACC_INNOFF_SINGLE_EN
	_phy_rtd_part_outl(0x1800da50, 5, 5, 0); //REG_P0_B_ACC_INNOFF_SINGLE_EN
	_phy_rtd_part_outl(0x1800da30, 9, 9, 1); //REG_P0_B_POW_EQ_DFE
	_phy_rtd_part_outl(0x1800da40, 9, 9, 1); //REG_P0_B_POW_EQ_DFE
	_phy_rtd_part_outl(0x1800da50, 9, 9, 1); //REG_P0_B_POW_EQ_DFE
	_phy_rtd_part_outl(0x1800da38, 28, 28, 0); //REG_P0_B_KOFF_SEL_1MANUAL_0AUTO
	_phy_rtd_part_outl(0x1800da48, 28, 28, 0); //REG_P0_B_KOFF_SEL_1MANUAL_0AUTO
	_phy_rtd_part_outl(0x1800da58, 28, 28, 0); //REG_P0_B_KOFF_SEL_1MANUAL_0AUTO
	_phy_rtd_part_outl(0x1800da38, 27, 27, 0); //REG_P0_B_POW_SAVING_SWITCH_ENABLE
	_phy_rtd_part_outl(0x1800da48, 27, 27, 0); //REG_P0_B_POW_SAVING_SWITCH_ENABLE
	_phy_rtd_part_outl(0x1800da58, 27, 27, 0); //REG_P0_B_POW_SAVING_SWITCH_ENABLE
}

void APHY_Param_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	//CK lane
	_phy_rtd_part_outl(0x1800da20, 11, 10, APHY_Param_1[value_param].CK_SEL_BAND_CAP_2_1); //REG_P0_CK_SEL_BAND_CAP
	_phy_rtd_part_outl(0x1800da20, 13, 13, APHY_Param_1[value_param].CK_POW_CP2_3_1); //REG_P0_CK_POW_CP2
	_phy_rtd_part_outl(0x1800da20, 17, 14, APHY_Param_1[value_param].CK_SEL_ICP_4_1); //REG_P0_CK_SEL_ICP
	_phy_rtd_part_outl(0x1800da20, 24, 22, APHY_Param_1[value_param].CK_SEL_LPF_RES_5_1); //REG_P0_CK_SEL_LPF_RES
	_phy_rtd_part_outl(0x1800da24, 8, 0, APHY_Param_1[value_param].CK_SEL_M_DIV_6_1); //REG_P0_CK_SEL_M_DIV
	_phy_rtd_part_outl(0x1800da24, 9, 9, 0); //REG_P0_CK_SEL_PRE_M_DIV
	_phy_rtd_part_outl(0x1800da24, 10, 10, 0); //REG_P0_CK_SEL_POST_M_DIV
	//BGR lane
	_phy_rtd_part_outl(0x1800da38, 18, 18, 0); //REG_P0_B_PI_CSEL
	_phy_rtd_part_outl(0x1800da48, 18, 18, 0); //REG_P0_B_PI_CSEL
	_phy_rtd_part_outl(0x1800da58, 18, 18, 0); //REG_P0_B_PI_CSEL
	_phy_rtd_part_outl(0x1800da38, 22, 20, APHY_Param_1[value_param].B_PI_ISEL_11_1); //REG_P0_B_PI_ISEL
	_phy_rtd_part_outl(0x1800da48, 22, 20, APHY_Param_1[value_param].B_PI_ISEL_11_1_16); //REG_P0_B_PI_ISEL
	_phy_rtd_part_outl(0x1800da58, 22, 20, APHY_Param_1[value_param].B_PI_ISEL_11_1_32); //REG_P0_B_PI_ISEL
	_phy_rtd_part_outl(0x1800da38, 25, 23, APHY_Param_1[value_param].B_PI_SEL_DATARATE_12_1); //REG_P0_B_PI_SEL_DATARATE
	_phy_rtd_part_outl(0x1800da48, 25, 23, APHY_Param_1[value_param].B_PI_SEL_DATARATE_12_1_16); //REG_P0_B_PI_SEL_DATARATE
	_phy_rtd_part_outl(0x1800da58, 25, 23, APHY_Param_1[value_param].B_PI_SEL_DATARATE_12_1_32); //REG_P0_B_PI_SEL_DATARATE
	_phy_rtd_part_outl(0x1800da38, 15, 15, APHY_Param_1[value_param].B_DEMUX_DIV5_10_SEL_13_1); //REG_P0_B_DEMUX_DIV5_10_SEL
	_phy_rtd_part_outl(0x1800da48, 15, 15, APHY_Param_1[value_param].B_DEMUX_DIV5_10_SEL_13_1_16); //REG_P0_B_DEMUX_DIV5_10_SEL
	_phy_rtd_part_outl(0x1800da58, 15, 15, APHY_Param_1[value_param].B_DEMUX_DIV5_10_SEL_13_1_32); //REG_P0_B_DEMUX_DIV5_10_SEL
	_phy_rtd_part_outl(0x1800da38, 17, 16, APHY_Param_1[value_param].B_DEMUX_RATE_SEL_14_1); //REG_P0_B_DEMUX_RATE_SEL
	_phy_rtd_part_outl(0x1800da48, 17, 16, APHY_Param_1[value_param].B_DEMUX_RATE_SEL_14_1_16); //REG_P0_B_DEMUX_RATE_SEL
	_phy_rtd_part_outl(0x1800da58, 17, 16, APHY_Param_1[value_param].B_DEMUX_RATE_SEL_14_1_32); //REG_P0_B_DEMUX_RATE_SEL
	_phy_rtd_part_outl(0x1800da38, 13, 13, APHY_Param_1[value_param].B_DFE_TAP_EN_15_1); //REG_P0_B_DFE_TAP_EN, Tap1 enable
	_phy_rtd_part_outl(0x1800da48, 13, 13, APHY_Param_1[value_param].B_DFE_TAP_EN_15_1_16); //REG_P0_B_DFE_TAP_EN, Tap1 enable
	_phy_rtd_part_outl(0x1800da58, 13, 13, APHY_Param_1[value_param].B_DFE_TAP_EN_15_1_32); //REG_P0_B_DFE_TAP_EN, Tap1 enable
	_phy_rtd_part_outl(0x1800da38, 14, 14, APHY_Param_1[value_param].B_DFE_TAP_EN_16_1); //REG_P0_B_DFE_TAP_EN, Tap2 enable
	_phy_rtd_part_outl(0x1800da48, 14, 14, APHY_Param_1[value_param].B_DFE_TAP_EN_16_1_16); //REG_P0_B_DFE_TAP_EN, Tap2 enable
	_phy_rtd_part_outl(0x1800da58, 14, 14, APHY_Param_1[value_param].B_DFE_TAP_EN_16_1_32); //REG_P0_B_DFE_TAP_EN, Tap2 enable
	_phy_rtd_part_outl(0x1800da38, 11, 11, 0); //REG_P0_B_DFE_SUMAMP_EN_CS
	_phy_rtd_part_outl(0x1800da48, 11, 11, 0); //REG_P0_B_DFE_SUMAMP_EN_CS
	_phy_rtd_part_outl(0x1800da58, 11, 11, 0); //REG_P0_B_DFE_SUMAMP_EN_CS
	_phy_rtd_part_outl(0x1800da30, 13, 12, APHY_Param_1[value_param].B_LE_IHALF_LE_18_1); //REG_P0_B_LE_IHALF_LE
	_phy_rtd_part_outl(0x1800da40, 13, 12, APHY_Param_1[value_param].B_LE_IHALF_LE_18_1_16); //REG_P0_B_LE_IHALF_LE
	_phy_rtd_part_outl(0x1800da50, 13, 12, APHY_Param_1[value_param].B_LE_IHALF_LE_18_1_32); //REG_P0_B_LE_IHALF_LE
	_phy_rtd_part_outl(0x1800da30, 14, 14, 0); //REG_P0_B_LE_IHALF_NC
	_phy_rtd_part_outl(0x1800da40, 14, 14, 0); //REG_P0_B_LE_IHALF_NC
	_phy_rtd_part_outl(0x1800da50, 14, 14, 0); //REG_P0_B_LE_IHALF_NC
	_phy_rtd_part_outl(0x1800da30, 15, 15, 0); //REG_P0_B_LE_IHALF_TAP0
	_phy_rtd_part_outl(0x1800da40, 15, 15, 0); //REG_P0_B_LE_IHALF_TAP0
	_phy_rtd_part_outl(0x1800da50, 15, 15, 0); //REG_P0_B_LE_IHALF_TAP0
	_phy_rtd_part_outl(0x1800da30, 19, 16, APHY_Param_1[value_param].B_LE_ISEL_LE1_21_1); //REG_P0_B_LE_ISEL_LE1
	_phy_rtd_part_outl(0x1800da40, 19, 16, APHY_Param_1[value_param].B_LE_ISEL_LE1_21_1_16); //REG_P0_B_LE_ISEL_LE1
	_phy_rtd_part_outl(0x1800da50, 19, 16, APHY_Param_1[value_param].B_LE_ISEL_LE1_21_1_32); //REG_P0_B_LE_ISEL_LE1
	_phy_rtd_part_outl(0x1800da30, 23, 20, APHY_Param_1[value_param].B_LE_ISEL_LE2_22_1); //REG_P0_B_LE_ISEL_LE2
	_phy_rtd_part_outl(0x1800da40, 23, 20, APHY_Param_1[value_param].B_LE_ISEL_LE2_22_1_16); //REG_P0_B_LE_ISEL_LE2
	_phy_rtd_part_outl(0x1800da50, 23, 20, APHY_Param_1[value_param].B_LE_ISEL_LE2_22_1_32); //REG_P0_B_LE_ISEL_LE2
	_phy_rtd_part_outl(0x1800da30, 27, 24, APHY_Param_1[value_param].B_LE_ISEL_NC_23_1); //REG_P0_B_LE_ISEL_NC
	_phy_rtd_part_outl(0x1800da40, 27, 24, APHY_Param_1[value_param].B_LE_ISEL_NC_23_1_16); //REG_P0_B_LE_ISEL_NC
	_phy_rtd_part_outl(0x1800da50, 27, 24, APHY_Param_1[value_param].B_LE_ISEL_NC_23_1_32); //REG_P0_B_LE_ISEL_NC
	_phy_rtd_part_outl(0x1800da30, 31, 28, APHY_Param_1[value_param].B_LE_ISEL_TAP0_24_1); //REG_P0_B_LE_ISEL_TAP0
	_phy_rtd_part_outl(0x1800da40, 31, 28, APHY_Param_1[value_param].B_LE_ISEL_TAP0_24_1_16); //REG_P0_B_LE_ISEL_TAP0
	_phy_rtd_part_outl(0x1800da50, 31, 28, APHY_Param_1[value_param].B_LE_ISEL_TAP0_24_1_32); //REG_P0_B_LE_ISEL_TAP0
	_phy_rtd_part_outl(0x1800da34, 1, 0, APHY_Param_1[value_param].B_LE_RLSEL_LE2_25_1); //REG_P0_B_LE_RLSEL_LE2
	_phy_rtd_part_outl(0x1800da44, 1, 0, APHY_Param_1[value_param].B_LE_RLSEL_LE2_25_1_16); //REG_P0_B_LE_RLSEL_LE2
	_phy_rtd_part_outl(0x1800da54, 1, 0, APHY_Param_1[value_param].B_LE_RLSEL_LE2_25_1_32); //REG_P0_B_LE_RLSEL_LE2
	_phy_rtd_part_outl(0x1800da34, 3, 2, APHY_Param_1[value_param].B_LE_RLSEL_LE11_26_1); //REG_P0_B_LE_RLSEL_LE11
	_phy_rtd_part_outl(0x1800da44, 3, 2, APHY_Param_1[value_param].B_LE_RLSEL_LE11_26_1_16); //REG_P0_B_LE_RLSEL_LE11
	_phy_rtd_part_outl(0x1800da54, 3, 2, APHY_Param_1[value_param].B_LE_RLSEL_LE11_26_1_32); //REG_P0_B_LE_RLSEL_LE11
	_phy_rtd_part_outl(0x1800da34, 5, 4, APHY_Param_1[value_param].B_LE_RLSEL_LE12_27_1); //REG_P0_B_LE_RLSEL_LE12
	_phy_rtd_part_outl(0x1800da44, 5, 4, APHY_Param_1[value_param].B_LE_RLSEL_LE12_27_1_16); //REG_P0_B_LE_RLSEL_LE12
	_phy_rtd_part_outl(0x1800da54, 5, 4, APHY_Param_1[value_param].B_LE_RLSEL_LE12_27_1_32); //REG_P0_B_LE_RLSEL_LE12
	_phy_rtd_part_outl(0x1800da34, 7, 6, APHY_Param_1[value_param].B_LE_RLSEL_NC1_28_1); //REG_P0_B_LE_RLSEL_NC1
	_phy_rtd_part_outl(0x1800da44, 7, 6, APHY_Param_1[value_param].B_LE_RLSEL_NC1_28_1_16); //REG_P0_B_LE_RLSEL_NC1
	_phy_rtd_part_outl(0x1800da54, 7, 6, APHY_Param_1[value_param].B_LE_RLSEL_NC1_28_1_32); //REG_P0_B_LE_RLSEL_NC1
	_phy_rtd_part_outl(0x1800da34, 9, 8, APHY_Param_1[value_param].B_LE_RLSEL_NC2_29_1); //REG_P0_B_LE_RLSEL_NC2
	_phy_rtd_part_outl(0x1800da44, 9, 8, APHY_Param_1[value_param].B_LE_RLSEL_NC2_29_1_16); //REG_P0_B_LE_RLSEL_NC2
	_phy_rtd_part_outl(0x1800da54, 9, 8, APHY_Param_1[value_param].B_LE_RLSEL_NC2_29_1_32); //REG_P0_B_LE_RLSEL_NC2
	_phy_rtd_part_outl(0x1800da34, 11, 10, APHY_Param_1[value_param].B_LE_RLSEL_TAP0_30_1); //REG_P0_B_LE_RLSEL_TAP0
	_phy_rtd_part_outl(0x1800da44, 11, 10, APHY_Param_1[value_param].B_LE_RLSEL_TAP0_30_1_16); //REG_P0_B_LE_RLSEL_TAP0
	_phy_rtd_part_outl(0x1800da54, 11, 10, APHY_Param_1[value_param].B_LE_RLSEL_TAP0_30_1_32); //REG_P0_B_LE_RLSEL_TAP0
	_phy_rtd_part_outl(0x1800da34, 14, 12, APHY_Param_1[value_param].B_LE_RSSEL_LE2_31_1); //REG_P0_B_LE_RSSEL_LE2
	_phy_rtd_part_outl(0x1800da44, 14, 12, APHY_Param_1[value_param].B_LE_RSSEL_LE2_31_1_16); //REG_P0_B_LE_RSSEL_LE2
	_phy_rtd_part_outl(0x1800da54, 14, 12, APHY_Param_1[value_param].B_LE_RSSEL_LE2_31_1_32); //REG_P0_B_LE_RSSEL_LE2
	_phy_rtd_part_outl(0x1800da34, 17, 15, APHY_Param_1[value_param].B_LE_RSSEL_LE11_32_1); //REG_P0_B_LE_RSSEL_LE11
	_phy_rtd_part_outl(0x1800da44, 17, 15, APHY_Param_1[value_param].B_LE_RSSEL_LE11_32_1_16); //REG_P0_B_LE_RSSEL_LE11
	_phy_rtd_part_outl(0x1800da54, 17, 15, APHY_Param_1[value_param].B_LE_RSSEL_LE11_32_1_32); //REG_P0_B_LE_RSSEL_LE11
	_phy_rtd_part_outl(0x1800da34, 20, 18, APHY_Param_1[value_param].B_LE_RSSEL_LE12_33_1); //REG_P0_B_LE_RSSEL_LE12
	_phy_rtd_part_outl(0x1800da44, 20, 18, APHY_Param_1[value_param].B_LE_RSSEL_LE12_33_1_16); //REG_P0_B_LE_RSSEL_LE12
	_phy_rtd_part_outl(0x1800da54, 20, 18, APHY_Param_1[value_param].B_LE_RSSEL_LE12_33_1_32); //REG_P0_B_LE_RSSEL_LE12
	_phy_rtd_part_outl(0x1800da34, 22, 21, APHY_Param_1[value_param].B_LE_RSSEL_TAP0_34_1); //REG_P0_B_LE_RSSEL_TAP0
	_phy_rtd_part_outl(0x1800da44, 22, 21, APHY_Param_1[value_param].B_LE_RSSEL_TAP0_34_1_16); //REG_P0_B_LE_RSSEL_TAP0
	_phy_rtd_part_outl(0x1800da54, 22, 21, APHY_Param_1[value_param].B_LE_RSSEL_TAP0_34_1_32); //REG_P0_B_LE_RSSEL_TAP0
	_phy_rtd_part_outl(0x1800da34, 24, 23, 0); //REG_P0_B_LE_KOFF_RANGE
	_phy_rtd_part_outl(0x1800da44, 24, 23, 0); //REG_P0_B_LE_KOFF_RANGE
	_phy_rtd_part_outl(0x1800da54, 24, 23, 0); //REG_P0_B_LE_KOFF_RANGE
	_phy_rtd_part_outl(0x1800da34, 25, 25, 1); //REG_P0_B_LE_VGEN_RS_CAL_EN
	_phy_rtd_part_outl(0x1800da44, 25, 25, 1); //REG_P0_B_LE_VGEN_RS_CAL_EN
	_phy_rtd_part_outl(0x1800da54, 25, 25, 1); //REG_P0_B_LE_VGEN_RS_CAL_EN
	_phy_rtd_part_outl(0x1800da34, 26, 26, 1); //REG_P0_B_POW_LEQ_KOFF
	_phy_rtd_part_outl(0x1800da44, 26, 26, 1); //REG_P0_B_POW_LEQ_KOFF
	_phy_rtd_part_outl(0x1800da54, 26, 26, 1); //REG_P0_B_POW_LEQ_KOFF
	_phy_rtd_part_outl(0x1800da34, 27, 27, APHY_Param_1[value_param].B_POW_NC1_LEQ_38_1); //REG_P0_B_POW_NC1_LEQ
	_phy_rtd_part_outl(0x1800da44, 27, 27, APHY_Param_1[value_param].B_POW_NC1_LEQ_38_1_16); //REG_P0_B_POW_NC1_LEQ
	_phy_rtd_part_outl(0x1800da54, 27, 27, APHY_Param_1[value_param].B_POW_NC1_LEQ_38_1_32); //REG_P0_B_POW_NC1_LEQ
	_phy_rtd_part_outl(0x1800da34, 28, 28, 0); //REG_P0_B_POW_NC2_LEQ
	_phy_rtd_part_outl(0x1800da44, 28, 28, 0); //REG_P0_B_POW_NC2_LEQ
	_phy_rtd_part_outl(0x1800da54, 28, 28, 0); //REG_P0_B_POW_NC2_LEQ
	_phy_rtd_part_outl(0x1800da34, 29, 29, 1); //REG_P0_B_POW_RL_LEQ
	_phy_rtd_part_outl(0x1800da44, 29, 29, 1); //REG_P0_B_POW_RL_LEQ
	_phy_rtd_part_outl(0x1800da54, 29, 29, 1); //REG_P0_B_POW_RL_LEQ
	_phy_rtd_part_outl(0x1800da38, 4, 4, 1); //REG_P0_B_DFE_SUMAMP_DCGAIN_MAX
	_phy_rtd_part_outl(0x1800da48, 4, 4, 1); //REG_P0_B_DFE_SUMAMP_DCGAIN_MAX
	_phy_rtd_part_outl(0x1800da58, 4, 4, 1); //REG_P0_B_DFE_SUMAMP_DCGAIN_MAX
}

void APHY_Init_Flow_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	//CK lane
	_phy_rtd_part_outl(0x1800da20, 26, 26, 1); //REG_P0_CK_TIE_VCTRLS
	_phy_rtd_part_outl(0x1800da24, 19, 19, 1); //REG_P0_CK_RSTB_CLK_FLD
	_phy_rtd_part_outl(0x1800da24, 20, 20, 1); //REG_P0_CK_RSTB_REF_FB_DIV
	_phy_rtd_part_outl(0x1800da24, 21, 21, 0); //REG_P0_CK_RSTB_DIV_BAND_2OR4
	_phy_rtd_part_outl(0x1800da24, 22, 22, 1); //REG_P0_CK_RSTB_DIV_IQ
	_phy_rtd_part_outl(0x1800da24, 24, 24, 1); //REG_P0_CK_RSTB_UPDN_CP1
	_phy_rtd_part_outl(0x1800da24, 25, 25, 1); //REG_P0_CK_RSTB_UPDN_CP2
	_phy_rtd_part_outl(0x1800da24, 26, 26, 1); //REG_P0_CK_RSTB_M_DIV
	_phy_rtd_part_outl(0x1800da24, 27, 27, 1); //REG_P0_CK_RSTB_PRESCALER
	_phy_rtd_part_outl(0x1800da24, 23, 23, 1); //REG_P0_CK_RSTB_PFD
	_phy_rtd_part_outl(0x1800da20, 26, 26, 0); //REG_P0_CK_TIE_VCTRLS 1 -> 0
	//BGR lane
	_phy_rtd_maskl(0x1800dad0, 0xfffffffa, 0x00000007); //P0_RSTB_DEMUX_B
}

void DCDR_settings_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	_phy_rtd_maskl(0x1800db00, 0x5fffffff, 0xe0000000); //P0_B_DIG_RST_N
	_phy_rtd_maskl(0x1800db00, 0xebffffff, 0x1c000000); //P0_B_CDR_RST_N
	_phy_rtd_part_outl(0x1800db14, 23, 23, 1); //reg_p0_en_clkout_manual
	_phy_rtd_part_outl(0x1800db14, 31, 31, 1); //reg_p0_bypass_clk_rdy
	_phy_rtd_part_outl(0x1800db18, 31, 31, 1); //reg_p0_wd_sdm_en
	_phy_rtd_part_outl(0x1800db14, 0, 0, 1); //p0_wdog_rst_n
}

void DFE_ini_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	//=========================================Adapt initial =================================================
	_phy_rtd_part_outl(0x1800de00, 23, 16, 0); //B-Lane, dfe_adapt_en disable
	_phy_rtd_part_outl(0x1800de10, 23, 16, 0); //B-Lane, dfe_adapt_en disable
	_phy_rtd_part_outl(0x1800de20, 23, 16, 0); //B-Lane, dfe_adapt_en disable
	_phy_rtd_part_outl(0x1800de3c, 21, 19, 0); //vth_min<2:0>
	_phy_rtd_part_outl(0x1800de3c, 18, 15, 7); //vth_max<3:0>
	_phy_rtd_part_outl(0x1800de40, 15, 9, 36); //tap0_max<6:0>
	_phy_rtd_part_outl(0x1800de38, 25, 20, DFE_ini_1[value_param].tap1_min_p2_6_1); //tap1_min<5:0>
	_phy_rtd_part_outl(0x1800de38, 19, 15, 31); //tap2_max<4:0>
	_phy_rtd_part_outl(0x1800de38, 14, 10, 31); //tap2_min<4:0>
	//BGR lane
	_phy_rtd_part_outl(0x1800de04, 3, 0, 5); //B-Lane, vth_init<3:0>
	_phy_rtd_part_outl(0x1800de14, 3, 0, 5); //B-Lane, vth_init<3:0>
	_phy_rtd_part_outl(0x1800de24, 3, 0, 5); //B-Lane, vth_init<3:0>
	_phy_rtd_part_outl(0x1800de04, 13, 7, DFE_ini_1[value_param].tap0_init_lane0_p2_11_1); //B-Lane, tap0_init<6:0>
	_phy_rtd_part_outl(0x1800de14, 13, 7, DFE_ini_1[value_param].tap0_init_lane0_p2_11_1_16); //B-Lane, tap0_init<6:0>
	_phy_rtd_part_outl(0x1800de24, 13, 7, DFE_ini_1[value_param].tap0_init_lane0_p2_11_1_32); //B-Lane, tap0_init<6:0>
	_phy_rtd_part_outl(0x1800de00, 15, 11, DFE_ini_1[value_param].le_min_lane0_p2_12_1); //B-Lane, le_min<4:0>
	_phy_rtd_part_outl(0x1800de10, 15, 11, DFE_ini_1[value_param].le_min_lane0_p2_12_1_16); //B-Lane, le_min<4:0>
	_phy_rtd_part_outl(0x1800de20, 15, 11, DFE_ini_1[value_param].le_min_lane0_p2_12_1_32); //B-Lane, le_min<4:0>
	_phy_rtd_part_outl(0x1800de00, 10, 6, DFE_ini_1[value_param].le_init_lane0_p2_13_1); //B-Lane, le_init<4:0>
	_phy_rtd_part_outl(0x1800de10, 10, 6, DFE_ini_1[value_param].le_init_lane0_p2_13_1_16); //B-Lane, le_init<4:0>
	_phy_rtd_part_outl(0x1800de20, 10, 6, DFE_ini_1[value_param].le_init_lane0_p2_13_1_32); //B-Lane, le_init<4:0>
	_phy_rtd_part_outl(0x1800de04, 19, 14, DFE_ini_1[value_param].tap1_init_lane0_p2_14_1); //B-Lane, tap1_init<5:0>
	_phy_rtd_part_outl(0x1800de14, 19, 14, DFE_ini_1[value_param].tap1_init_lane0_p2_14_1_16); //B-Lane, tap1_init<5:0>
	_phy_rtd_part_outl(0x1800de24, 19, 14, DFE_ini_1[value_param].tap1_init_lane0_p2_14_1_32); //B-Lane, tap1_init<5:0>
	_phy_rtd_part_outl(0x1800de04, 25, 20, 0); //B-Lane, tap2_init<5:0>
	_phy_rtd_part_outl(0x1800de14, 25, 20, 0); //B-Lane, tap2_init<5:0>
	_phy_rtd_part_outl(0x1800de24, 25, 20, 0); //B-Lane, tap2_init<5:0>
	_phy_rtd_part_outl(0x1800de08, 15, 15, 0); //B-Lane, de_video_en
	_phy_rtd_part_outl(0x1800de18, 15, 15, 0); //B-Lane, de_video_en
	_phy_rtd_part_outl(0x1800de28, 15, 15, 0); //B-Lane, de_video_en
	_phy_rtd_part_outl(0x1800de08, 14, 14, 0); //B-Lane, de_packet_en
	_phy_rtd_part_outl(0x1800de18, 14, 14, 0); //B-Lane, de_packet_en
	_phy_rtd_part_outl(0x1800de28, 14, 14, 0); //B-Lane, de_packet_en
	_phy_rtd_part_outl(0x1800de00, 24, 24, 0); //B-Lane, rl_threshold
	_phy_rtd_part_outl(0x1800de10, 24, 24, 0); //B-Lane, rl_threshold
	_phy_rtd_part_outl(0x1800de20, 24, 24, 0); //B-Lane, rl_threshold
	//===================== Adapt mode setting =====================
	_phy_rtd_part_outl(0x1800de30, 31, 30, DFE_ini_1[value_param].adapt_mode_p2_20_1); //adapt_mode_p2<1:0>
	_phy_rtd_part_outl(0x1800de30, 3, 3, 0); //tap0_trans_p2
	_phy_rtd_part_outl(0x1800de30, 2, 2, 1); //tap0_notrans_p2
	_phy_rtd_part_outl(0x1800de34, 31, 29, 2); //tap0_gain_p2<2:0>
	_phy_rtd_part_outl(0x1800de3c, 11, 8, 1); //tap0_threshold_p2<3:0>
	_phy_rtd_part_outl(0x1800de3c, 7, 6, 1); //tap0_adjust_p2<1:0>
	_phy_rtd_part_outl(0x1800de30, 26, 24, 0); //tap1_delay_p2<2:0>
	_phy_rtd_part_outl(0x1800de34, 28, 26, 0); //tap1_gain_p2<2:0>
	_phy_rtd_part_outl(0x1800de3c, 5, 5, 1); //tap1_trans_p2
	_phy_rtd_part_outl(0x1800de34, 25, 23, 0); //tap2_gain_p2<2:0>
	_phy_rtd_part_outl(0x1800de30, 23, 21, 0); //tap24_delay_p2<2:0>
	_phy_rtd_part_outl(0x1800de40, 25, 20, 2); //tap_divisor_p2<5:0>
	_phy_rtd_part_outl(0x1800de30, 20, 18, 0); //le_delay_p2<2:0>
	_phy_rtd_part_outl(0x1800de34, 13, 11, 1); //le_gain1_p2<2:0>, h1.5=0, gain1/gain2=2
	_phy_rtd_part_outl(0x1800de34, 10, 8, 6); //le_gain2_p2<2:0>, h1.5=0, gain1/gain2=2
	_phy_rtd_part_outl(0x1800de34, 6, 4, 6); //le_gain_prec_p2<2:0>
	_phy_rtd_part_outl(0x1800de34, 3, 3, 0); //le_prec_sign_p2<0>
	_phy_rtd_part_outl(0x1800de34, 7, 7, 0); //le2_sign_p2
	_phy_rtd_part_outl(0x1800de3c, 14, 12, 7); //vth_threshold_p2<2:0>
	_phy_rtd_part_outl(0x1800de40, 19, 16, 15); //vth_divisor_p2<3:0>
	_phy_rtd_part_outl(0x1800de30, 29, 29, 0); //edge_det_mode_p2
	_phy_rtd_part_outl(0x1800de3c, 4, 4, 0); //edge_last_notrans_p2
	_phy_rtd_part_outl(0x1800de30, 28, 28, 0); //transition_only_p2
	_phy_rtd_part_outl(0x1800de34, 2, 0, 2); //trans_rlength_p2<2:0>
	_phy_rtd_part_outl(0x1800de30, 15, 8, 191); //gray_en_p2<7:0>
	_phy_rtd_part_outl(0x1800de30, 17, 16, 1); //servo_delay_p2<1:0>
	//===================== Load ini =====================
	_phy_rtd_part_outl(0x1800de30, 7, 4, 15); //dfe_adapt_rstb_p2<3:0>
	_phy_rtd_part_outl(0x1800de08, 31, 24, 255); //B-Lane, load_in_init<7:0>
	_phy_rtd_part_outl(0x1800de18, 31, 24, 255); //B-Lane, load_in_init<7:0>
	_phy_rtd_part_outl(0x1800de28, 31, 24, 255); //B-Lane, load_in_init<7:0>
	_phy_rtd_part_outl(0x1800de08, 31, 24, 0); //B-Lane, load_in_init<7:0>
	_phy_rtd_part_outl(0x1800de18, 31, 24, 0); //B-Lane, load_in_init<7:0>
	_phy_rtd_part_outl(0x1800de28, 31, 24, 0); //B-Lane, load_in_init<7:0>
	_phy_rtd_part_outl(0x1800de30, 27, 27, 0); //le_auto_reload_p2
}

void Koffset_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{

	 int i, RX_Data,KOff_Range=0;
        int RX_Data_Result_0=0, RX_Data_Result_1=0;
        int RX_Data_Toggle_Code_P1=64, RX_Data_Toggle_Code_P2=64, RX_Data_Toggle_Code_N1=64, RX_Data_Toggle_Code_N2=64;
        int Flag_1=0, Flag_2=0;                                                                                        
        int SW_Cal_Result=0;                                                                                           
        int j,k;                                                                                                       
	int RX_Data_No_Toggle_Code_P1=64, RX_Data_No_Toggle_Code_N1=64;                                                      
		                                                                                                                   
	//#define Read_Cnt 20                                                                                                
	#define RX_Data_times 20	                                                                                                 
      #define FORE_KOFF_RANGE  0  // range maybe 0  1  2   3                                                           
		                                                                                                               
//B lane                                                                                                               
	RX_Data_Result_0=0, RX_Data_Result_1=0;                                                                        
	RX_Data_Toggle_Code_P1=64, RX_Data_Toggle_Code_P2=64, RX_Data_Toggle_Code_N1=64, RX_Data_Toggle_Code_N2=64;    
	Flag_1=0, Flag_2=0;                                                                                            
	SW_Cal_Result=0;                                                                                               
	RX_Data_No_Toggle_Code_P1=64, RX_Data_No_Toggle_Code_N1=64;                                                      
                                                                                                                       
///////////////////////////////////////////////////////////////////////////////////////                                
for (KOff_Range=0; KOff_Range<FORE_KOFF_RANGE+1; KOff_Range++) 
{                                                                                                                      
                                                                                                                       
	rtd_part_outl(0xB800DA38, 28, 28, 0x1);	//REG_P0_B_KOFF_SEL_1MANUAL_0AUTO                                    
	rtd_part_outl(0xB800DA30, 3, 3, 0x1);	//REG_P0_B_ACC_INOFF_EN                                                
	rtd_part_outl(0xB800DA38, 14, 12, 0x0);	//REG_P0_B_DFE_TAP_EN                                                
	                                                                                                           
	rtd_part_outl(0xB800DA34, 24, 23, KOff_Range);//REG_P0_B_LE_KOFF_RANGE                                       
	rtd_part_outl(0xB800DB78, 30, 30, 0x1);//reg_p0_b_manual_offset_en , input open                              
	                                                                                                           
	rtd_part_outl(0xB80b4010, 8, 8, 0x0);	//                                                                     
	rtd_part_outl(0xB80b4010, 7, 4, 0x5);	//                                                                     
	rtd_part_outl(0xB800d618, 9, 5, 0x12);	//                                                                   
	                                                                                                           
	rtd_part_outl(0xB800DB2C, 15, 11, 0x2);	//Debug Selection, G==0x1, R=0x0, B=0x2                              
	rtd_part_outl(0xB800DB04, 5, 4, 0x2);	//REG_P0_TESTOUT_SEL                                                   
	                                                                                                           
	rtd_part_outl(0xB800DB7C, 28, 28, 0x1);//reg_p0_B_offset_manual_eq                                           
                                                                                                                       
                                                                                                                       
	/////////////////////////////////////////////////////////////////////////////////////////                            
	//2comp 0~16  Bin 0~16                                                                                       
	//for(B_Offset_Code=15; B_Offset_Code<=0; B_Offset_Code--)    //                                             
	for(j=15; j>=0 ;j--)    //     Offset_Code=+15 -> 0; bin_Code=15->0                                          
	{                                                                                                            
		rtd_part_outl(0xB800DB7C, 26, 22,j);//reg_p0_b_offset_ini_eqB_Offset_Code                            
		                                                                                       
		for(i=0;i<RX_Data_times;i++)                                                                              
		{                                                                                                    
			RX_Data= rtd_part_inl(0xB800d614, 12, 12);	//MAC RX Data Debug Port                         
			                                                                                       
			if(RX_Data==1)                                                                               
			    RX_Data_Result_1++;                                                                  
			else                                                                                         
			    RX_Data_Result_0++;                                                                  
		}                                                                                                    
                                                                                                                       
		if(RX_Data_Result_1>1 && RX_Data_Result_1<RX_Data_times)                                                  
		{                                                                                                    
		        RX_Data_Toggle_Code_P1= j;                                                                   
		}                                                                                                    
		else if(RX_Data_Result_1==20 && RX_Data_Result_0==0)	//Add No toggle status                                     
		{                                                                                                                
			RX_Data_No_Toggle_Code_P1= j;                                                                              
		}                                                                                                                
							                                                                                                   
		RX_Data_Result_1=0;                                                                                  
		RX_Data_Result_0=0;                                                                                  
                                                                                                                       
	}                                                                                                            
	if(RX_Data_Toggle_Code_P1==64)                                                                             
	{                                                                                                          
		Flag_1=1;                                                                                            
	}                                                                                                          
                                                                                                                       
	for(k=17; k<32; k++)    //     Offset_Code=-15 -> 0; bin_Code=17->31                                         
	{                                                                                                            
		rtd_part_outl( 0xB800DB7C, 26, 22, k);//reg_p0_b_offset_ini_eqB_Offset_Code                          
		for(i=0;i<RX_Data_times;i++)                                                                              
		{                                                                                                    
		        RX_Data= rtd_part_inl(0xB800d614, 12, 12);	//MAC RX Data Debug Port                         
		        if(RX_Data==1)                                                                               
		                RX_Data_Result_1++;                                                                  
		        else                                                                                         
		                RX_Data_Result_0++;                                                                  
		}                                                                                                    
	                                                                                                   
		if(RX_Data_Result_1>1 && RX_Data_Result_1<RX_Data_times)                                                  
		{                                                                                                    
		         RX_Data_Toggle_Code_N1= k;                                                                  
		}                                                                                                    
		else if(RX_Data_Result_0==20 && RX_Data_Result_1==0)	//Add No toggle status                                     
		{                                                                                                                
		                                                                                                   
			RX_Data_No_Toggle_Code_N1= k;                                                                              
		}					                                                                                                       
		RX_Data_Result_1=0;                                                                                  
		RX_Data_Result_0=0;                                                                                  
	                                                                                                   
	}                                                                                                            
	if(RX_Data_Toggle_Code_N1==64)                                                                             
	{                                                                                                          
		Flag_2=1;                                                                                            
	}                                                                                                          
	        //L0_Offset_Toggle_Code->Caption  ="L0_Offset_Toggle_Code(2S_Comp)= 0x"+ IntToStr(RX_Data_Toggle_Code);      
                                                                                                                       
	//=============================================================================================================      
                                                                                                                       
	if(Flag_1==0 && Flag_2==0)  // Bin 0~31 Toogle                                                               
	{                                                                                                            
		SW_Cal_Result= ( RX_Data_Toggle_Code_P1+   (RX_Data_Toggle_Code_N1-31))/2;                           
		                                                                                    
		//L0_Offset_Toggle_Code->Caption  ="L0_Offset_Toggle_Code(2S_Comp)= 0x"+ IntToStr(SW_Cal_Result);    
		HDMI_EMG("Case1:L0_Offset_Toggle_Code(2S_Comp)= 0x%x\n",SW_Cal_Result);   
		break;
	}                                                                                                            
	else if(Flag_1==1 && Flag_2==1)                                                                              
	{			                                                                                                       
		if(RX_Data_No_Toggle_Code_P1==0)                                                                            
		SW_Cal_Result= RX_Data_No_Toggle_Code_N1;                                                                  
		else if(RX_Data_No_Toggle_Code_N1==31)                                                                       
		SW_Cal_Result= RX_Data_No_Toggle_Code_P1;   
		HDMI_EMG("Case No_Toggle:L0_Offset_Toggle_Code(2S_Comp)= 0x%x\n",SW_Cal_Result);   
		
	}			                                                                                                           
	else if(Flag_1==0 && Flag_2==1)                                                                              
	{                                                                                                            
		for(j=0; j<=15 ;j++)     //Offset_Code=0   -> +15; bin_Code=0->15                                    
		{                                                                                                    
		    	rtd_part_outl(0xB800DB7C, 26, 22,j);//reg_p0_b_offset_ini_eqB_Offset_Code                    
			                                                                                           
			for(i=0;i<RX_Data_times;i++)                                                                      
			{                                                                                            
			        RX_Data= rtd_part_inl(0xB800d614, 12, 12);	//MAC RX Data Debug Port                 
			        if(RX_Data==1)                                                                       
			        RX_Data_Result_1++;                                                                  
				else                                                                                         
			        RX_Data_Result_0++;                                                                  
			}                                                                                                   
			                                                                                           
			if(RX_Data_Result_1>1 && RX_Data_Result_1<RX_Data_times)                                                
			{                                                                                                  
			    RX_Data_Toggle_Code_P2= j;                                                                 
			}                                                                                                  
			RX_Data_Result_1=0;                                                                            
			RX_Data_Result_0=0;                                                                                
		}                                                                                                                
		if((RX_Data_Toggle_Code_P1!=64)&&(RX_Data_Toggle_Code_P2!=64))
		{
			SW_Cal_Result= (RX_Data_Toggle_Code_P1+RX_Data_Toggle_Code_P2)/2;
		}
		else if(RX_Data_Toggle_Code_P1!=64)
		{
			SW_Cal_Result=RX_Data_Toggle_Code_P1;
		}
		else if(RX_Data_Toggle_Code_P2!=64)
		{
	        	SW_Cal_Result=RX_Data_Toggle_Code_P2;
		}
		//L0_Offset_Toggle_Code->Caption  ="L0_Offset_Toggle_Code(2S_Comp)= 0x"+ IntToStr(SW_Cal_Result);
		HDMI_EMG("Case2:L0_Offset_Toggle_Code(2S_Comp)=0x%x\n",SW_Cal_Result);
		break;

	}
	else if(Flag_1==1 && Flag_2==0)  //Bin 17~31 Toggle RX_Data_N    // Offset_Code=->0 -> -15                   
	{                                                                                                            
		for(k=31; k>=17; k--)    //Offset_Code=0   -> 115; bin_Code=31->17                                         
		{                                                                                                          
			rtd_part_outl(0xB800DB7C, 26, 22, k);//reg_p0_b_offset_ini_eqB_Offset_Code                         
	                                                                                               
			for(i=0;i<RX_Data_times;i++)                                                                            
			{                                                                                                  
				RX_Data= rtd_part_inl(0xB800d614, 12, 12);	//MAC RX Data Debug Port                       
				                                                                               
				if(RX_Data==1)                                                                             
				RX_Data_Result_1++;                                                                
				else                                                                                       
				RX_Data_Result_0++;                                                                
			}                                                                                                  
			if(RX_Data_Result_1>1 && RX_Data_Result_1<RX_Data_times)                                                
			{                                                                                                  
			         RX_Data_Toggle_Code_N2= k;                                                                
			}                                                                                                  
			RX_Data_Result_1=0;                                                                                
			RX_Data_Result_0=0;                                                                                
		}                                                                                                          
	                                                                                     
		if((RX_Data_Toggle_Code_N1!=64)&&(RX_Data_Toggle_Code_N2!=64))
		{
	    		SW_Cal_Result= (RX_Data_Toggle_Code_N1+RX_Data_Toggle_Code_N2)/2;
		}
		else if(RX_Data_Toggle_Code_N1!=64)
		{
		  	SW_Cal_Result=RX_Data_Toggle_Code_N1;
		}
		else if(RX_Data_Toggle_Code_N2!=64)
		{
			SW_Cal_Result=RX_Data_Toggle_Code_N2;
		}
 		//L0_Offset_Toggle_Code->Caption  ="L0_Offset_Toggle_Code(2S_Comp)= 0x"+ IntToStr(SW_Cal_Result);
        	HDMI_EMG("Case3:L0_Offset_Toggle_Code(2S_Comp)= 0x%x\n",SW_Cal_Result);

		break;
	}    

}

	rtd_part_outl(0xB800DB7C, 26, 22, SW_Cal_Result);//Manual offset code                              
	rtd_part_outl(0xB800DB78, 30, 30, 0x0);//reg_p0_b_manual_offset_en , input open => ON	                         
	rtd_part_outl(0xB800DA30, 3, 3, 0x0);	//REG_P0_B_ACC_INOFF_EN => ON                                            
	rtd_part_outl(0xB800DA38, 14, 12, 0x6);	//REG_P0_B_DFE_TAP_EN => ON                                            

	
//////////////////
//G lane                                                                                                               
	RX_Data_Result_0=0, RX_Data_Result_1=0;                                                                        
	RX_Data_Toggle_Code_P1=64, RX_Data_Toggle_Code_P2=64, RX_Data_Toggle_Code_N1=64, RX_Data_Toggle_Code_N2=64;    
	Flag_1=0, Flag_2=0;                                                                                            
	SW_Cal_Result=0;                                                                                               
	RX_Data_No_Toggle_Code_P1=64, RX_Data_No_Toggle_Code_N1=64;                                                      
                                                                                                                       
///////////////////////////////////////////////////////////////////////////////////////                                
 for (KOff_Range=0; KOff_Range<FORE_KOFF_RANGE+1; KOff_Range++) 
{                                                                                                                     
                                                                                                                       
	rtd_part_outl(0xB800DA48, 28, 28, 0x1);	//REG_P0_G_KOFF_SEL_1MANUAL_0AUTO                                    
	rtd_part_outl(0xB800DA40, 3, 3, 0x1);	//REG_P0_G_ACC_INOFF_EN                                                
	rtd_part_outl(0xB800DA48, 14, 12, 0x0);	//REG_P0_G_DFE_TAP_EN                                                
	                                                                                                           
	rtd_part_outl(0xB800DA44, 24, 23, KOff_Range);//REG_P0_G_LE_KOFF_RANGE                                       
	rtd_part_outl(0xB800DB80, 30, 30, 0x1);//reg_p0_g_manual_offset_en , input open                              
	                                                                                                           
	rtd_part_outl(0xB80b4010, 8, 8, 0x0);	//                                                                     
	rtd_part_outl(0xB80b4010, 7, 4, 0x5);	//                                                                     
	rtd_part_outl(0xB800d618, 9, 5, 0x12);	//                                                                   
	                                                                                                           
	rtd_part_outl(0xB800DB2C, 15, 11, 0x1);	//Debug Selection, G==0x1, R=0x0, B=0x2                              
	rtd_part_outl(0xB800DB04, 5, 4, 0x2);	//REG_P0_TESTOUT_SEL                                                   
	                                                                                                           
	rtd_part_outl(0xB800DB84, 28, 28, 0x1);//reg_p0_G_offset_manual_eq                                           
                                                                                                                       
                                                                                                                       
	/////////////////////////////////////////////////////////////////////////////////////////                            
	//2comp 0~16  Bin 0~16                                                                                       
	//for(G_Offset_Code=15; G_Offset_Code<=0; G_Offset_Code--)    //                                             
	for(j=15; j>=0 ;j--)    //     Offset_Code=+15 -> 0; bin_Code=15->0                                          
	{                                                                                                            
		rtd_part_outl(0xB800DB84, 26, 22,j);//reg_p0_g_offset_ini_eqG_Offset_Code                            
		                                                                                       
		for(i=0;i<RX_Data_times;i++)                                                                              
		{                                                                                                    
			RX_Data= rtd_part_inl(0xB800d614, 12, 12);	//MAC RX Data Debug Port                         
			                                                                                       
			if(RX_Data==1)                                                                               
			    RX_Data_Result_1++;                                                                  
			else                                                                                         
			    RX_Data_Result_0++;                                                                  
		}                                                                                                    
                                                                                                                       
		if(RX_Data_Result_1>1 && RX_Data_Result_1<RX_Data_times)                                                  
		{                                                                                                    
		        RX_Data_Toggle_Code_P1= j;                                                                   
		}                                                                                                    
		else if(RX_Data_Result_1==20 && RX_Data_Result_0==0)	//Add No toggle status                                     
		{                                                                                                                
			RX_Data_No_Toggle_Code_P1= j;                                                                              
		}                                                                                                                
							                                                                                                   
		RX_Data_Result_1=0;                                                                                  
		RX_Data_Result_0=0;                                                                                  
                                                                                                                       
	}                                                                                                            
	if(RX_Data_Toggle_Code_P1==64)                                                                             
	{                                                                                                          
		Flag_1=1;                                                                                            
	}                                                                                                          
                                                                                                                       
	for(k=17; k<32; k++)    //     Offset_Code=-15 -> 0; bin_Code=17->31                                         
	{                                                                                                            
		rtd_part_outl( 0xB800DB84, 26, 22, k);//reg_p0_b_offset_ini_eqB_Offset_Code                          
		for(i=0;i<RX_Data_times;i++)                                                                              
		{                                                                                                    
		        RX_Data= rtd_part_inl(0xB800d614, 12, 12);	//MAC RX Data Debug Port                         
		        if(RX_Data==1)                                                                               
		                RX_Data_Result_1++;                                                                  
		        else                                                                                         
		                RX_Data_Result_0++;                                                                  
		}                                                                                                    
	                                                                                                   
		if(RX_Data_Result_1>1 && RX_Data_Result_1<RX_Data_times)                                                  
		{                                                                                                    
		         RX_Data_Toggle_Code_N1= k;                                                                  
		}                                                                                                    
		else if(RX_Data_Result_0==20 && RX_Data_Result_1==0)	//Add No toggle status                                     
		{                                                                                                                
		                                                                                                   
			RX_Data_No_Toggle_Code_N1= k;                                                                              
		}					                                                                                                       
		RX_Data_Result_1=0;                                                                                  
		RX_Data_Result_0=0;                                                                                  
	                                                                                                   
	}                                                                                                            
	if(RX_Data_Toggle_Code_N1==64)                                                                             
	{                                                                                                          
		Flag_2=1;                                                                                            
	}                                                                                                          
	        //L1_Offset_Toggle_Code->Caption  ="L1_Offset_Toggle_Code(2S_Comp)= 0x"+ IntToStr(RX_Data_Toggle_Code);      
                                                                                                                       
	//=============================================================================================================      
                                                                                                                       
	if(Flag_1==0 && Flag_2==0)  // Bin 0~31 Toogle                                                               
	{                                                                                                            
		SW_Cal_Result= ( RX_Data_Toggle_Code_P1+   (RX_Data_Toggle_Code_N1-31))/2;                           
		                                                                                    
		//L1_Offset_Toggle_Code->Caption  ="L1_Offset_Toggle_Code(2S_Comp)= 0x"+ IntToStr(SW_Cal_Result);    
		HDMI_EMG("Case1:L1_Offset_Toggle_Code(2S_Comp)= 0x%x\n",SW_Cal_Result);                                
		break;

	}                                                                                                            
	else if(Flag_1==1 && Flag_2==1)                                                                              
	{			                                                                                                       
		if(RX_Data_No_Toggle_Code_P1==0)                                                                            
		SW_Cal_Result= RX_Data_No_Toggle_Code_N1;                                                                  
		else if(RX_Data_No_Toggle_Code_N1==31)                                                                       
		SW_Cal_Result= RX_Data_No_Toggle_Code_P1;   
		HDMI_EMG("Case No_Toggle:L1_Offset_Toggle_Code(2S_Comp)= 0x%x\n",SW_Cal_Result);   
		
	}			                                                                                                           
	else if(Flag_1==0 && Flag_2==1)                                                                              
	{                                                                                                            
		for(j=0; j<=15 ;j++)     //Offset_Code=0   -> +15; bin_Code=0->15                                    
		{                                                                                                    
		    	rtd_part_outl(0xB800DB84, 26, 22,j);//reg_p0_g_offset_ini_eqG_Offset_Code                    
			                                                                                           
			for(i=0;i<RX_Data_times;i++)                                                                      
			{                                                                                            
			        RX_Data= rtd_part_inl(0xB800d614, 12, 12);	//MAC RX Data Debug Port                 
			        if(RX_Data==1)                                                                       
			        RX_Data_Result_1++;                                                                  
				else                                                                                         
			        RX_Data_Result_0++;                                                                  
			}                                                                                                   
			                                                                                           
			if(RX_Data_Result_1>1 && RX_Data_Result_1<RX_Data_times)                                                
			{                                                                                                  
			    RX_Data_Toggle_Code_P2= j;                                                                 
			}                                                                                                  
			RX_Data_Result_1=0;                                                                            
			RX_Data_Result_0=0;                                                                                
		}                                                                                                                
		if((RX_Data_Toggle_Code_P1!=64)&&(RX_Data_Toggle_Code_P2!=64))
		{
			SW_Cal_Result= (RX_Data_Toggle_Code_P1+RX_Data_Toggle_Code_P2)/2;
		}
		else if(RX_Data_Toggle_Code_P1!=64)
		{
			SW_Cal_Result=RX_Data_Toggle_Code_P1;
		}
		else if(RX_Data_Toggle_Code_P2!=64)
		{
	        	SW_Cal_Result=RX_Data_Toggle_Code_P2;
		}
		//L1_Offset_Toggle_Code->Caption  ="L1_Offset_Toggle_Code(2S_Comp)= 0x"+ IntToStr(SW_Cal_Result);
        	HDMI_EMG("Case2:L1_Offset_Toggle_Code(2S_Comp)= 0x%x\n",SW_Cal_Result);

		break;

	}
	else if(Flag_1==1 && Flag_2==0)  //Bin 17~31 Toggle RX_Data_N    // Offset_Code=->0 -> -15                   
	{                                                                                                            
		for(k=31; k>=17; k--)    //Offset_Code=0   -> 115; bin_Code=31->17                                         
		{                                                                                                          
			rtd_part_outl(0xB800DB84, 26, 22, k);//reg_p0_g_offset_ini_eqG_Offset_Code                         
	                                                                                               
			for(i=0;i<RX_Data_times;i++)                                                                            
			{                                                                                                  
				RX_Data= rtd_part_inl(0xB800d614, 12, 12);	//MAC RX Data Debug Port                       
				                                                                               
				if(RX_Data==1)                                                                             
				RX_Data_Result_1++;                                                                
				else                                                                                       
				RX_Data_Result_0++;                                                                
			} 
			
			if(RX_Data_Result_1>1 && RX_Data_Result_1<RX_Data_times)                                                
			{                                                                                                  
				RX_Data_Toggle_Code_N2= k;                                                                
			}

			RX_Data_Result_1=0;                                                                                
			RX_Data_Result_0=0;                                                                                
		}                                                                                                          

		if((RX_Data_Toggle_Code_N1!=64)&&(RX_Data_Toggle_Code_N2!=64))
		{
	    		SW_Cal_Result= (RX_Data_Toggle_Code_N1+RX_Data_Toggle_Code_N2)/2;
		}
		else if(RX_Data_Toggle_Code_N1!=64)
		{
		  	SW_Cal_Result=RX_Data_Toggle_Code_N1;
		}
		else if(RX_Data_Toggle_Code_N2!=64)
		{
			SW_Cal_Result=RX_Data_Toggle_Code_N2;
		}

 		//L1_Offset_Toggle_Code->Caption  ="L1_Offset_Toggle_Code(2S_Comp)= 0x"+ IntToStr(SW_Cal_Result);
		HDMI_EMG("Case3:L1_Offset_Toggle_Code(2S_Comp)= 0x%x\n",SW_Cal_Result);
		break;
	}       

}

	rtd_part_outl(0xB800DB84, 26, 22, SW_Cal_Result);//Manual offset code                              
	rtd_part_outl(0xB800DB80, 30, 30, 0x0);//reg_p0_g_manual_offset_en , input open => ON	                         
	rtd_part_outl(0xB800DA40, 3, 3, 0x0);	//REG_P0_G_ACC_INOFF_EN => ON                                            
	rtd_part_outl(0xB800DA48, 14, 12, 0x6);	//REG_P0_G_DFE_TAP_EN => ON                                            

	
//////////////////
//R lane                                                                                                               
	RX_Data_Result_0=0, RX_Data_Result_1=0;                                                                        
	RX_Data_Toggle_Code_P1=64, RX_Data_Toggle_Code_P2=64, RX_Data_Toggle_Code_N1=64, RX_Data_Toggle_Code_N2=64;    
	Flag_1=0, Flag_2=0;                                                                                            
	SW_Cal_Result=0;                                                                                               
	RX_Data_No_Toggle_Code_P1=64, RX_Data_No_Toggle_Code_N1=64;                                                      
                                                                                                                       
///////////////////////////////////////////////////////////////////////////////////////                                
                                                                                                                      
 for (KOff_Range=0; KOff_Range<FORE_KOFF_RANGE+1; KOff_Range++) 
{                                                                                                                     
                                                                                                                      
	rtd_part_outl(0xB800DA58, 28, 28, 0x1);	//REG_P0_R_KOFF_SEL_1MANUAL_0AUTO                                    
	rtd_part_outl(0xB800DA50, 3, 3, 0x1);	//REG_P0_R_ACC_INOFF_EN                                                
	rtd_part_outl(0xB800DA58, 14, 12, 0x0);	//REG_P0_R_DFE_TAP_EN                                                
	                                                                                                           
	rtd_part_outl(0xB800DA54, 24, 23, KOff_Range);//REG_P0_R_LE_KOFF_RANGE                                       
	rtd_part_outl(0xB800DB88, 30, 30, 0x1);//reg_p0_r_manual_offset_en , input open                              
	                                                                                                           
	rtd_part_outl(0xB80b4010, 8, 8, 0x0);	//                                                                     
	rtd_part_outl(0xB80b4010, 7, 4, 0x5);	//                                                                     
	rtd_part_outl(0xB800d618, 9, 5, 0x12);	//                                                                   
	                                                                                                           
	rtd_part_outl(0xB800DB2C, 15, 11, 0x0);	//Debug Selection, G==0x1, R=0x0, B=0x2                              
	rtd_part_outl(0xB800DB04, 5, 4, 0x2);	//REG_P0_TESTOUT_SEL                                                   
	                                                                                                           
	rtd_part_outl(0xB800DB8C, 28, 28, 0x1);//reg_p0_R_offset_manual_eq                                           
                                                                                                                       
                                                                                                                       
	/////////////////////////////////////////////////////////////////////////////////////////                            
	//2comp 0~16  Bin 0~16                                                                                       
	//for(B_Offset_Code=15; B_Offset_Code<=0; B_Offset_Code--)    //                                             
	for(j=15; j>=0 ;j--)    //     Offset_Code=+15 -> 0; bin_Code=15->0                                          
	{                                                                                                            
		rtd_part_outl(0xB800DB8C, 26, 22,j);//reg_p0_b_offset_ini_eqB_Offset_Code                            
		                                                                                       
		for(i=0;i<RX_Data_times;i++)                                                                              
		{                                                                                                    
			RX_Data= rtd_part_inl(0xB800d614, 12, 12);	//MAC RX Data Debug Port                         
			                                                                                       
			if(RX_Data==1)                                                                               
			    RX_Data_Result_1++;                                                                  
			else                                                                                         
			    RX_Data_Result_0++;                                                                  
		}                                                                                                    
                                                                                                                       
		if(RX_Data_Result_1>1 && RX_Data_Result_1<RX_Data_times)                                                  
		{                                                                                                    
		        RX_Data_Toggle_Code_P1= j;                                                                   
		}                                                                                                    
		else if(RX_Data_Result_1==20 && RX_Data_Result_0==0)	//Add No toggle status                                     
		{                                                                                                                
			RX_Data_No_Toggle_Code_P1= j;                                                                              
		}                                                                                                                
							                                                                                                   
		RX_Data_Result_1=0;                                                                                  
		RX_Data_Result_0=0;                                                                                  
                                                                                                                       
	}                                                                                                            
	if(RX_Data_Toggle_Code_P1==64)                                                                             
	{                                                                                                          
		Flag_1=1;                                                                                            
	}                                                                                                          
                                                                                                                       
	for(k=17; k<32; k++)    //     Offset_Code=-15 -> 0; bin_Code=17->31                                         
	{                                                                                                            
		rtd_part_outl( 0xB800DB8C, 26, 22, k);//reg_p0_b_offset_ini_eqB_Offset_Code                          
		for(i=0;i<RX_Data_times;i++)                                                                              
		{                                                                                                    
		        RX_Data= rtd_part_inl(0xB800d614, 12, 12);	//MAC RX Data Debug Port                         
		        if(RX_Data==1)                                                                               
		                RX_Data_Result_1++;                                                                  
		        else                                                                                         
		                RX_Data_Result_0++;                                                                  
		}                                                                                                    
	                                                                                                   
		if(RX_Data_Result_1>1 && RX_Data_Result_1<RX_Data_times)                                                  
		{                                                                                                    
		         RX_Data_Toggle_Code_N1= k;                                                                  
		}                                                                                                    
		else if(RX_Data_Result_0==20 && RX_Data_Result_1==0)	//Add No toggle status                                     
		{                                                                                                                
		                                                                                                   
			RX_Data_No_Toggle_Code_N1= k;                                                                              
		}					                                                                                                       
		RX_Data_Result_1=0;                                                                                  
		RX_Data_Result_0=0;                                                                                  
	                                                                                                   
	}                                                                                                            
	if(RX_Data_Toggle_Code_N1==64)                                                                             
	{                                                                                                          
		Flag_2=1;                                                                                            
	}                                                                                                          
	        //L2_Offset_Toggle_Code->Caption  ="L2_Offset_Toggle_Code(2S_Comp)= 0x"+ IntToStr(RX_Data_Toggle_Code);      
                                                                                                                       
	//=============================================================================================================      
                                                                                                                       
	if(Flag_1==0 && Flag_2==0)  // Bin 0~31 Toogle                                                               
	{                                                                                                            
		SW_Cal_Result= ( RX_Data_Toggle_Code_P1+   (RX_Data_Toggle_Code_N1-31))/2;                           
		                                                                                    
		//L2_Offset_Toggle_Code->Caption  ="L0_Offset_Toggle_Code(2S_Comp)= 0x"+ IntToStr(SW_Cal_Result);    
		HDMI_EMG("Case1:L2_Offset_Toggle_Code(2S_Comp)= 0x%x\n",SW_Cal_Result); 
		break;

	}                                                                                                            
	else if(Flag_1==1 && Flag_2==1)                                                                              
	{			                                                                                                       
		if(RX_Data_No_Toggle_Code_P1==0)                                                                            
		SW_Cal_Result= RX_Data_No_Toggle_Code_N1;                                                                  
		else if(RX_Data_No_Toggle_Code_N1==31)                                                                       
		SW_Cal_Result= RX_Data_No_Toggle_Code_P1;                                                                  
		HDMI_EMG("Case No_Toggle:L2_Offset_Toggle_Code(2S_Comp)= 0x%x\n",SW_Cal_Result);   
	}			                                                                                                           
	else if(Flag_1==0 && Flag_2==1)                                                                              
	{                                                                                                            
		for(j=0; j<=15 ;j++)     //Offset_Code=0   -> +15; bin_Code=0->15                                    
		{                                                                                                    
		    	rtd_part_outl(0xB800DB8C, 26, 22,j);//reg_p0_b_offset_ini_eqR_Offset_Code                    
			                                                                                           
			for(i=0;i<RX_Data_times;i++)                                                                      
			{                                                                                            
			        RX_Data= rtd_part_inl(0xB800d614, 12, 12);	//MAC RX Data Debug Port                 
			        if(RX_Data==1)                                                                       
			        RX_Data_Result_1++;                                                                  
				else                                                                                         
			        RX_Data_Result_0++;                                                                  
			}                                                                                                   
			                                                                                           
			if(RX_Data_Result_1>1 && RX_Data_Result_1<RX_Data_times)                                                
			{                                                                                                  
			    RX_Data_Toggle_Code_P2= j;                                                                 
			}                                                                                                  
			RX_Data_Result_1=0;                                                                            
			RX_Data_Result_0=0;                                                                                
		}                                                                                                                
		if((RX_Data_Toggle_Code_P1!=64)&&(RX_Data_Toggle_Code_P2!=64))
		{
			SW_Cal_Result= (RX_Data_Toggle_Code_P1+RX_Data_Toggle_Code_P2)/2;
		}
		else if(RX_Data_Toggle_Code_P1!=64)
		{
			SW_Cal_Result=RX_Data_Toggle_Code_P1;
		}
		else if(RX_Data_Toggle_Code_P2!=64)
		{
	        	SW_Cal_Result=RX_Data_Toggle_Code_P2;
		}	        
		//L0_Offset_Toggle_Code->Caption  ="L0_Offset_Toggle_Code(2S_Comp)= 0x"+ IntToStr(SW_Cal_Result);
		HDMI_EMG("Case2:L2_Offset_Toggle_Code(2S_Comp)= 0x%x\n",SW_Cal_Result);
		break;

	}
	else if(Flag_1==1 && Flag_2==0)  //Bin 17~31 Toggle RX_Data_N    // Offset_Code=->0 -> -15                   
	{                                                                                                            
		for(k=31; k>=17; k--)    //Offset_Code=0   -> 115; bin_Code=31->17                                         
		{                                                                                                          
			rtd_part_outl(0xB800DB8C, 26, 22, k);//reg_p0_r_offset_ini_eqR_Offset_Code                         
	                                                                                               
			for(i=0;i<RX_Data_times;i++)                                                                            
			{                                                                                                  
				RX_Data= rtd_part_inl(0xB800d614, 12, 12);	//MAC RX Data Debug Port                       
				                                                                               
				if(RX_Data==1)                                                                             
				RX_Data_Result_1++;                                                                
				else                                                                                       
				RX_Data_Result_0++;                                                                
			}                                                                                                  
			if(RX_Data_Result_1>1 && RX_Data_Result_1<RX_Data_times)                                                
			{                                                                                                  
			         RX_Data_Toggle_Code_N2= k;                                                                
			}                                                                                                  
			RX_Data_Result_1=0;                                                                                
			RX_Data_Result_0=0;                                                                                
		}                                                                                                          
	                                                                                     
		if((RX_Data_Toggle_Code_N1!=64)&&(RX_Data_Toggle_Code_N2!=64))
		{
	    		SW_Cal_Result= (RX_Data_Toggle_Code_N1+RX_Data_Toggle_Code_N2)/2;
		}
		else if(RX_Data_Toggle_Code_N1!=64)
		{
		  	SW_Cal_Result=RX_Data_Toggle_Code_N1;
		}
		else if(RX_Data_Toggle_Code_N2!=64)
		{
			SW_Cal_Result=RX_Data_Toggle_Code_N2;
		}

 		//L2_Offset_Toggle_Code->Caption  ="L2_Offset_Toggle_Code(2S_Comp)= 0x"+ IntToStr(SW_Cal_Result);
		HDMI_EMG("Case3:L2_Offset_Toggle_Code(2S_Comp)= 0x%x\n",SW_Cal_Result);
		break;
	                                               
	}   
}
	rtd_part_outl(0xB800DB8C, 26, 22, SW_Cal_Result);//Manual offset code                              
	rtd_part_outl(0xB800DB88, 30, 30, 0x0);//reg_p0_r_manual_offset_en , input open => ON	                         
	rtd_part_outl(0xB800DA50, 3, 3, 0x0);	//REG_P0_R_ACC_INOFF_EN => ON                                            
	rtd_part_outl(0xB800DA58, 14, 12, 0x6);	//REG_P0_R_DFE_TAP_EN => ON                                            
	
//////////////////


 ////Restore to ori                                                                                                    
        rtd_part_outl(0xB80b4010, 7, 4, 0x0);	//                                                                       
        rtd_part_outl(0xB800d618, 9, 5, 0x1F);	
}

void DCDR_settings_2_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	//DEMUX_Reset
	_phy_rtd_maskl(0x1800dad0, 0xfffffffa, 0x00000000); //P0_RSTB_DEMUX_B
	_phy_rtd_maskl(0x1800dad0, 0xfffffffa, 0x00000007);
	//DCDR_Reset
	_phy_rtd_maskl(0x1800db00, 0x5fffffff, 0x00000000); //P0_B_DIG_RST_N
	_phy_rtd_maskl(0x1800db00, 0xebffffff, 0x00000000); //P0_B_CDR_RST_N
	_phy_rtd_maskl(0x1800db00, 0x5fffffff, 0xe0000000); //P0_B_DIG_RST_N
	_phy_rtd_maskl(0x1800db00, 0xebffffff, 0x1c000000); //P0_B_CDR_RST_N
}

void LEQ_TAP0_Adapt_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	//===================== Adapt EN =====================
	_phy_rtd_part_outl(0x1800da38, 0, 0, LEQ_TAP0_Adapt_1[value_param].B_DFE_ADAPT_EN_2_1); //B-Lane, DFE_ADAPT_EN (APHY)
	_phy_rtd_part_outl(0x1800da48, 0, 0, LEQ_TAP0_Adapt_1[value_param].B_DFE_ADAPT_EN_2_1_16); //B-Lane, DFE_ADAPT_EN (APHY)
	_phy_rtd_part_outl(0x1800da58, 0, 0, LEQ_TAP0_Adapt_1[value_param].B_DFE_ADAPT_EN_2_1_32); //B-Lane, DFE_ADAPT_EN (APHY)
	_phy_rtd_part_outl(0x1800de00, 30, 30, LEQ_TAP0_Adapt_1[value_param].timer_ctrl_en_lane0_p2_3_1); //B-Lane, timer_ctrl_en
	_phy_rtd_part_outl(0x1800de10, 30, 30, LEQ_TAP0_Adapt_1[value_param].timer_ctrl_en_lane0_p2_3_1_16); //B-Lane, timer_ctrl_en
	_phy_rtd_part_outl(0x1800de20, 30, 30, LEQ_TAP0_Adapt_1[value_param].timer_ctrl_en_lane0_p2_3_1_32); //B-Lane, timer_ctrl_en
	_phy_rtd_part_outl(0x1800de00, 22, 22, LEQ_TAP0_Adapt_1[value_param].dfe_adapt_en_lane0_p2_4_1); //B-Lane, dfe_adapt_en enable Vth adapt
	_phy_rtd_part_outl(0x1800de10, 22, 22, LEQ_TAP0_Adapt_1[value_param].dfe_adapt_en_lane0_p2_4_1_16); //B-Lane, dfe_adapt_en enable Vth adapt
	_phy_rtd_part_outl(0x1800de20, 22, 22, LEQ_TAP0_Adapt_1[value_param].dfe_adapt_en_lane0_p2_4_1_32); //B-Lane, dfe_adapt_en enable Vth adapt
	_phy_rtd_part_outl(0x1800de00, 16, 16, LEQ_TAP0_Adapt_1[value_param].dfe_adapt_en_lane0_p2_5_1); //B-Lane, dfe_adapt_en enable Tap0 adapt
	_phy_rtd_part_outl(0x1800de10, 16, 16, LEQ_TAP0_Adapt_1[value_param].dfe_adapt_en_lane0_p2_5_1_16); //B-Lane, dfe_adapt_en enable Tap0 adapt
	_phy_rtd_part_outl(0x1800de20, 16, 16, LEQ_TAP0_Adapt_1[value_param].dfe_adapt_en_lane0_p2_5_1_32); //B-Lane, dfe_adapt_en enable Tap0 adapt
	udelay(500);
	_phy_rtd_part_outl(0x1800de00, 23, 23, LEQ_TAP0_Adapt_1[value_param].dfe_adapt_en_lane0_p2_6_1); //B-Lane, dfe_adapt_en enable Le adapt
	_phy_rtd_part_outl(0x1800de10, 23, 23, LEQ_TAP0_Adapt_1[value_param].dfe_adapt_en_lane0_p2_6_1_16); //B-Lane, dfe_adapt_en enable Le adapt
	_phy_rtd_part_outl(0x1800de20, 23, 23, LEQ_TAP0_Adapt_1[value_param].dfe_adapt_en_lane0_p2_6_1_32); //B-Lane, dfe_adapt_en enable Le adapt
	_phy_rtd_part_outl(0x1800de00, 17, 17, LEQ_TAP0_Adapt_1[value_param].dfe_adapt_en_lane0_p2_7_1); //B-Lane, dfe_adapt_en enable Tap1 adapt
	_phy_rtd_part_outl(0x1800de10, 17, 17, LEQ_TAP0_Adapt_1[value_param].dfe_adapt_en_lane0_p2_7_1_16); //B-Lane, dfe_adapt_en enable Tap1 adapt
	_phy_rtd_part_outl(0x1800de20, 17, 17, LEQ_TAP0_Adapt_1[value_param].dfe_adapt_en_lane0_p2_7_1_32); //B-Lane, dfe_adapt_en enable Tap1 adapt
	udelay(500);
	_phy_rtd_part_outl(0x1800de00, 18, 18, LEQ_TAP0_Adapt_1[value_param].dfe_adapt_en_lane0_p2_8_1); //B-Lane, dfe_adapt_en enable Tap2 adapt
	_phy_rtd_part_outl(0x1800de10, 18, 18, LEQ_TAP0_Adapt_1[value_param].dfe_adapt_en_lane0_p2_8_1_16); //B-Lane, dfe_adapt_en enable Tap2 adapt
	_phy_rtd_part_outl(0x1800de20, 18, 18, LEQ_TAP0_Adapt_1[value_param].dfe_adapt_en_lane0_p2_8_1_32); //B-Lane, dfe_adapt_en enable Tap2 adapt
	udelay(500);
	//===================== Adapt disable =====================
	_phy_rtd_part_outl(0x1800de00, 23, 16, 0); //B-Lane, dfe_adapt_en disable
	_phy_rtd_part_outl(0x1800de10, 23, 16, 0); //B-Lane, dfe_adapt_en disable
	_phy_rtd_part_outl(0x1800de20, 23, 16, 0); //B-Lane, dfe_adapt_en disable
}

void TAP0_2_Adapt_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	//===================== Adapt EN =====================
	_phy_rtd_part_outl(0x1800da38, 0, 0, TAP0_2_Adapt_1[value_param].B_DFE_ADAPT_EN_2_1); //B-Lane, DFE_ADAPT_EN (APHY)
	_phy_rtd_part_outl(0x1800da48, 0, 0, TAP0_2_Adapt_1[value_param].B_DFE_ADAPT_EN_2_1_16); //B-Lane, DFE_ADAPT_EN (APHY)
	_phy_rtd_part_outl(0x1800da58, 0, 0, TAP0_2_Adapt_1[value_param].B_DFE_ADAPT_EN_2_1_32); //B-Lane, DFE_ADAPT_EN (APHY)
	_phy_rtd_part_outl(0x1800de00, 30, 30, TAP0_2_Adapt_1[value_param].timer_ctrl_en_lane0_p2_3_1); //B-Lane, timer_ctrl_en
	_phy_rtd_part_outl(0x1800de10, 30, 30, TAP0_2_Adapt_1[value_param].timer_ctrl_en_lane0_p2_3_1_16); //B-Lane, timer_ctrl_en
	_phy_rtd_part_outl(0x1800de20, 30, 30, TAP0_2_Adapt_1[value_param].timer_ctrl_en_lane0_p2_3_1_32); //B-Lane, timer_ctrl_en
	_phy_rtd_part_outl(0x1800de00, 22, 22, TAP0_2_Adapt_1[value_param].dfe_adapt_en_lane0_p2_4_1); //B-Lane, dfe_adapt_en enable Vth adapt
	_phy_rtd_part_outl(0x1800de10, 22, 22, TAP0_2_Adapt_1[value_param].dfe_adapt_en_lane0_p2_4_1_16); //B-Lane, dfe_adapt_en enable Vth adapt
	_phy_rtd_part_outl(0x1800de20, 22, 22, TAP0_2_Adapt_1[value_param].dfe_adapt_en_lane0_p2_4_1_32); //B-Lane, dfe_adapt_en enable Vth adapt
	_phy_rtd_part_outl(0x1800de00, 16, 16, TAP0_2_Adapt_1[value_param].dfe_adapt_en_lane0_p2_5_1); //B-Lane, dfe_adapt_en enable Tap0 adapt
	_phy_rtd_part_outl(0x1800de10, 16, 16, TAP0_2_Adapt_1[value_param].dfe_adapt_en_lane0_p2_5_1_16); //B-Lane, dfe_adapt_en enable Tap0 adapt
	_phy_rtd_part_outl(0x1800de20, 16, 16, TAP0_2_Adapt_1[value_param].dfe_adapt_en_lane0_p2_5_1_32); //B-Lane, dfe_adapt_en enable Tap0 adapt
	udelay(500);
	_phy_rtd_part_outl(0x1800de00, 23, 23, TAP0_2_Adapt_1[value_param].dfe_adapt_en_lane0_p2_6_1); //B-Lane, dfe_adapt_en enable Le adapt
	_phy_rtd_part_outl(0x1800de10, 23, 23, TAP0_2_Adapt_1[value_param].dfe_adapt_en_lane0_p2_6_1_16); //B-Lane, dfe_adapt_en enable Le adapt
	_phy_rtd_part_outl(0x1800de20, 23, 23, TAP0_2_Adapt_1[value_param].dfe_adapt_en_lane0_p2_6_1_32); //B-Lane, dfe_adapt_en enable Le adapt
	_phy_rtd_part_outl(0x1800de00, 17, 17, TAP0_2_Adapt_1[value_param].dfe_adapt_en_lane0_p2_7_1); //B-Lane, dfe_adapt_en enable Tap1 adapt
	_phy_rtd_part_outl(0x1800de10, 17, 17, TAP0_2_Adapt_1[value_param].dfe_adapt_en_lane0_p2_7_1_16); //B-Lane, dfe_adapt_en enable Tap1 adapt
	_phy_rtd_part_outl(0x1800de20, 17, 17, TAP0_2_Adapt_1[value_param].dfe_adapt_en_lane0_p2_7_1_32); //B-Lane, dfe_adapt_en enable Tap1 adapt
	udelay(500);
	_phy_rtd_part_outl(0x1800de00, 18, 18, TAP0_2_Adapt_1[value_param].dfe_adapt_en_lane0_p2_8_1); //B-Lane, dfe_adapt_en enable Tap2 adapt
	_phy_rtd_part_outl(0x1800de10, 18, 18, TAP0_2_Adapt_1[value_param].dfe_adapt_en_lane0_p2_8_1_16); //B-Lane, dfe_adapt_en enable Tap2 adapt
	_phy_rtd_part_outl(0x1800de20, 18, 18, TAP0_2_Adapt_1[value_param].dfe_adapt_en_lane0_p2_8_1_32); //B-Lane, dfe_adapt_en enable Tap2 adapt
	udelay(500);
	//===================== Adapt disable =====================
	_phy_rtd_part_outl(0x1800de00, 23, 16, 0); //B-Lane, dfe_adapt_en disable
	_phy_rtd_part_outl(0x1800de10, 23, 16, 0); //B-Lane, dfe_adapt_en disable
	_phy_rtd_part_outl(0x1800de20, 23, 16, 0); //B-Lane, dfe_adapt_en disable
}

void Manual_DFE_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	_phy_rtd_part_outl(0x1800de00, 23, 16, 0); //B-Lane, dfe_adapt_en disable
	_phy_rtd_part_outl(0x1800de10, 23, 16, 0); //B-Lane, dfe_adapt_en disable
	_phy_rtd_part_outl(0x1800de20, 23, 16, 0); //B-Lane, dfe_adapt_en disable
	_phy_rtd_part_outl(0x1800de30, 7, 4, 15); //dfe_adapt_rstb_p2<3:0>
	//BGR lane
	_phy_rtd_part_outl(0x1800de3c, 21, 19, 0); //vth_min<2:0>
	_phy_rtd_part_outl(0x1800de3c, 18, 15, 7); //vth_max<3:0>
	_phy_rtd_part_outl(0x1800de40, 15, 9, 36); //tap0_max<6:0>
	_phy_rtd_part_outl(0x1800de38, 25, 20, Manual_DFE_1[value_param].tap1_min_p2_7_1); //tap1_min<5:0>
	_phy_rtd_part_outl(0x1800de38, 19, 15, 31); //tap2_max<4:0>
	_phy_rtd_part_outl(0x1800de38, 14, 10, Manual_DFE_1[value_param].tap2_min_p2_9_1); //tap2_min<4:0>
	_phy_rtd_part_outl(0x1800de04, 3, 0, Manual_DFE_1[value_param].vth_init_lane0_p2_10_1); //B-Lane, vth_init<3:0>
	_phy_rtd_part_outl(0x1800de14, 3, 0, Manual_DFE_1[value_param].vth_init_lane0_p2_10_1_16); //B-Lane, vth_init<3:0>
	_phy_rtd_part_outl(0x1800de24, 3, 0, Manual_DFE_1[value_param].vth_init_lane0_p2_10_1_32); //B-Lane, vth_init<3:0>
	_phy_rtd_part_outl(0x1800de04, 13, 7, 25); //B-Lane, tap0_init<6:0>
	_phy_rtd_part_outl(0x1800de14, 13, 7, 25); //B-Lane, tap0_init<6:0>
	_phy_rtd_part_outl(0x1800de24, 13, 7, 25); //B-Lane, tap0_init<6:0>
	_phy_rtd_part_outl(0x1800de00, 10, 6, 0); //B-Lane, le_init<4:0>
	_phy_rtd_part_outl(0x1800de10, 10, 6, 0); //B-Lane, le_init<4:0>
	_phy_rtd_part_outl(0x1800de20, 10, 6, 0); //B-Lane, le_init<4:0>
	_phy_rtd_part_outl(0x1800de04, 19, 14, 0); //B-Lane, tap1_init<5:0>
	_phy_rtd_part_outl(0x1800de14, 19, 14, 0); //B-Lane, tap1_init<5:0>
	_phy_rtd_part_outl(0x1800de24, 19, 14, 0); //B-Lane, tap1_init<5:0>
	_phy_rtd_part_outl(0x1800de04, 25, 20, 0); //B-Lane, tap2_init<5:0>
	_phy_rtd_part_outl(0x1800de14, 25, 20, 0); //B-Lane, tap2_init<5:0>
	_phy_rtd_part_outl(0x1800de24, 25, 20, 0); //B-Lane, tap2_init<5:0>
	_phy_rtd_part_outl(0x1800de08, 31, 24, 255); //B-Lane, load_in_init<7:0>
	_phy_rtd_part_outl(0x1800de18, 31, 24, 255); //B-Lane, load_in_init<7:0>
	_phy_rtd_part_outl(0x1800de28, 31, 24, 255); //B-Lane, load_in_init<7:0>
	_phy_rtd_part_outl(0x1800de08, 31, 24, 0); //B-Lane, load_in_init<7:0>
	_phy_rtd_part_outl(0x1800de18, 31, 24, 0); //B-Lane, load_in_init<7:0>
	_phy_rtd_part_outl(0x1800de28, 31, 24, 0); //B-Lane, load_in_init<7:0>
}


#endif
////////////////////////////////////////////
//HDMI_combo_DP_Port//////
#if 1
APHY_Fix_tmds_1_T APHY_Fix_tmds_1[] =
{
	{TMDS_5p94G,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x0,0x1,0x1},
	{TMDS_2p97G,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x0,0x1,0x1},
	{TMDS_1p485G,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x0,0x1,0x0,0x0,0x1},
	{TMDS_0p742G,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x0,0x1,0x0,0x0,0x1},
	{TMDS_0p25G,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0,0x1},
	{TMDS_4p445G,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x0,0x1,0x1},
	{TMDS_3p7125G,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x0,0x1,0x1},
	{TMDS_2p2275G,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x0,0x1,0x1},
	{TMDS_1p85625G,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x0,0x1,0x1},
	{TMDS_1p11375G,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x0,0x1,0x1},
	{TMDS_0p928125G,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x0,0x1,0x1},
	{TMDS_0p54G,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x0,0x1,0x1},
	{TMDS_0p405G,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x0,0x1,0x1},
	{TMDS_0p3375G,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x0,0x1,0x1},
	{TMDS_0p27G,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x0,0x1,0x1},
	{TMDS_OLT_5p94G,0x0,0x0,0x1,0x0,0x1,0x0,0x0,0x0,0x1,0x0,0x0,0x1,0x0},
	{TMDS_CP_5p94G,0x3,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x0},
};

APHY_Init_Flow_tmds_1_T APHY_Init_Flow_tmds_1[] =
{
	{TMDS_5p94G,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_2p97G,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_1p485G,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0p742G,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0p25G,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_4p445G,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_3p7125G,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_2p2275G,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_1p85625G,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_1p11375G,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0p928125G,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0p54G,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0p405G,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0p3375G,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0p27G,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_OLT_5p94G,0x1,0x1,0x0,0x0,0x0,0x0,0x1},
	{TMDS_CP_5p94G,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
};

APHY_Para_tmds_1_T APHY_Para_tmds_1[] =
{
	{TMDS_5p94G,0x2,0x1,0x0,0x0,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x3,0x3,0x3,0x24,0x0,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x0,0x2,0x1,0x1,0xf,0x0,0x0,0x10,0x0,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x0,0x2,0x1,0x1,0xf,0x0,0x0,0x10,0x0,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x0,0x2,0x1,0x1,0xf,0x0,0x0,0x10,0x0,0x2},
	{TMDS_2p97G,0x2,0x1,0x0,0x0,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x3,0x3,0x3,0x6,0x1,0x1,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x3,0x2,0x1,0xf,0x0,0x1,0x6,0x0,0x1,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x3,0x2,0x1,0xf,0x0,0x1,0x6,0x0,0x1,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x3,0x2,0x1,0xf,0x0,0x1,0x6,0x0,0x1},
	{TMDS_1p485G,0x2,0x1,0x0,0x0,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x3,0x3,0x3,0x24,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x1,0xf,0x0,0x2,0x6,0x1,0x0,0x0,0x0,0x0,
0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x1,0xf,0x0,0x2,0x6,0x1,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x1,0xf,0x0,0x2,0x6,0x1,0x0},
	{TMDS_0p742G,0x2,0x1,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x3,0x3,0x3,0x24,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x1,0xf,0x0,0x3,0x10,0x1,0x0,0x0,0x0,0x0,
0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x1,0xf,0x0,0x3,0x10,0x1,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x1,0xf,0x0,0x3,0x10,0x1,0x0},
	{TMDS_0p25G,0x2,0x1,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x4c,0x1,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0x7,0x6,0x4,0x4c,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0x7,0x6,0x4,0x4c,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0x7,0x6,0x4,0x4c,0x0,0x0},
	{TMDS_4p445G,0x2,0x1,0x0,0x0,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x2,0x2,0x2,0x24,0x0,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x1,0x2,0x1,0x0,0xc,0x3,0x0,0x10,0x0,0x2,0x2,0x3,0x1,
0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x1,0x2,0x1,0x0,0xc,0x3,0x0,0x10,0x0,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x1,0x2,0x1,0x0,0xc,0x3,0x0,0x10,0x0,0x2},
	{TMDS_3p7125G,0x2,0x1,0x0,0x0,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x24,0x0,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x0,0x2,0x1,0x0,0x7,0x6,0x0,0x10,0x0,0x2,0x2,0x3,0x1,
0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x0,0x2,0x1,0x0,0x7,0x6,0x0,0x10,0x0,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x0,0x2,0x1,0x0,0x7,0x6,0x0,0x10,0x0,0x2},
	{TMDS_2p2275G,0x2,0x1,0x0,0x0,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x2,0x2,0x2,0x24,0x0,0x1,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x3,0x2,0x0,0xc,0x3,0x1,0x6,0x0,0x1,0x1,0x1,0x0,
0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x3,0x2,0x0,0xc,0x3,0x1,0x6,0x0,0x1,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x3,0x2,0x0,0xc,0x3,0x1,0x6,0x0,0x1},
	{TMDS_1p85625G,0x2,0x1,0x0,0x0,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x24,0x0,0x0,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x3,0x2,0x0,0x7,0x6,0x1,0x6,0x0,0x0,0x1,0x1,0x0,
0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x3,0x2,0x0,0x7,0x6,0x1,0x6,0x0,0x0,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x3,0x2,0x0,0x7,0x6,0x1,0x6,0x0,0x0},
	{TMDS_1p11375G,0x2,0x1,0x0,0x0,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x2,0x2,0x2,0x24,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0xc,0x3,0x2,0x6,0x1,0x0,0x0,0x0,0x0,
0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0xc,0x3,0x2,0x6,0x1,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0xc,0x3,0x2,0x6,0x1,0x0},
	{TMDS_0p928125G,0x2,0x1,0x0,0x0,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x24,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0x7,0x6,0x2,0x6,0x1,0x0,0x0,0x0,0x0,
0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0x7,0x6,0x2,0x6,0x1,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0x7,0x6,0x2,0x6,0x1,0x0},
	{TMDS_0p54G,0x2,0x1,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x2,0x2,0x2,0x24,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0xc,0x3,0x3,0x10,0x1,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0xc,0x3,0x3,0x10,0x1,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0xc,0x3,0x3,0x10,0x1,0x0},
	{TMDS_0p405G,0x2,0x1,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x24,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0x7,0x6,0x3,0x10,0x1,0x0,0x0,0x0,0x0,
0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0x7,0x6,0x3,0x10,0x1,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0x7,0x6,0x3,0x10,0x1,0x0},
	{TMDS_0p3375G,0x2,0x1,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x3,0x3,0x3,0x24,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x1,0xf,0x0,0x4,0x4c,0x0,0x0,0x0,0x0,0x0,
0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x1,0xf,0x0,0x4,0x4c,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x1,0xf,0x0,0x4,0x4c,0x0,0x0},
	{TMDS_0p27G,0x2,0x1,0x0,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x2,0x2,0x2,0x24,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0xc,0x3,0x4,0x4c,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0xc,0x3,0x4,0x4c,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0xc,0x3,0x4,0x4c,0x0,0x0},
	{TMDS_OLT_5p94G,0x2,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x3,0x3,0x3,0x24,0x0,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x0,0x2,0x1,0x1,0xf,0x0,0x0,0x6b,0x0,0x2,0x2,0x3,0x1,
0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x0,0x2,0x1,0x1,0xf,0x0,0x0,0x6b,0x0,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x0,0x2,0x1,0x1,0xf,0x0,0x0,0x6b,0x0,0x2},
	{TMDS_CP_5p94G,0x3,0x3,0x1,0x3,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x3,0x3,0x3,0x24,0x0,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x0,0x3,0x3,0x1,0xf,0x6,0x3,0x6b,0x0,0x2,0x2,0x3,0x1,
0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x0,0x3,0x3,0x1,0xf,0x6,0x3,0x6b,0x0,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x0,0x3,0x3,0x1,0xf,0x6,0x3,0x6b,0x0,0x2},
};

DFE_ini_tmds_1_T DFE_ini_tmds_1[] =
{
	{TMDS_5p94G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_2p97G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_1p485G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_0p742G,0x5,0x10,0x5,0x10,0x5,0x10,0xf,0x10},
	{TMDS_0p25G,0x0,0x19,0x0,0x19,0x0,0x19,0x0,0x19},
	{TMDS_4p445G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_3p7125G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_2p2275G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_1p85625G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_1p11375G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_0p928125G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_0p54G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_0p405G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_0p3375G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_0p27G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_OLT_5p94G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_CP_5p94G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
};

DPHY_Fix_tmds_1_T DPHY_Fix_tmds_1[] =
{
	{TMDS_5p94G,0x0,0x0,0x0,0x0},
	{TMDS_2p97G,0x0,0x0,0x0,0x0},
	{TMDS_1p485G,0x0,0x0,0x0,0x0},
	{TMDS_0p742G,0x0,0x0,0x0,0x0},
	{TMDS_0p25G,0x0,0x0,0x0,0x0},
	{TMDS_4p445G,0x0,0x0,0x0,0x0},
	{TMDS_3p7125G,0x0,0x0,0x0,0x0},
	{TMDS_2p2275G,0x0,0x0,0x0,0x0},
	{TMDS_1p85625G,0x0,0x0,0x0,0x0},
	{TMDS_1p11375G,0x0,0x0,0x0,0x0},
	{TMDS_0p928125G,0x0,0x0,0x0,0x0},
	{TMDS_0p54G,0x0,0x0,0x0,0x0},
	{TMDS_0p405G,0x0,0x0,0x0,0x0},
	{TMDS_0p3375G,0x0,0x0,0x0,0x0},
	{TMDS_0p27G,0x0,0x0,0x0,0x0},
	{TMDS_OLT_5p94G,0x1,0x1,0x1,0x1},
	{TMDS_CP_5p94G,0x1,0x1,0x1,0x1},
};

DPHY_Para_tmds_1_T DPHY_Para_tmds_1[] =
{
	{TMDS_5p94G,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,
0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf},
	{TMDS_2p97G,0x12,0x12,0x12,0x12,0x8,0x8,0x8,0x8,0x1,0x1,0x1,0x1,0x2,0x2,0x2,0x1,0x1,0x1,0x2,0x98,0x7,0x2,0x98,0x7,0x2,0x98,
0x7,0x2,0x98,0x7,0x2,0x98,0x7,0x2,0x98,0x7,0x2,0x98,0x7,0x2,0x98,0x7,0x2,0x98,0x7,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf},
	{TMDS_1p485G,0x14,0x14,0x14,0x14,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x0,0x3,0x3,0x3,0x1,0x1,0x1,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,
0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf},
	{TMDS_0p742G,0x14,0x14,0x14,0x14,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x0,0x3,0x3,0x3,0x1,0x1,0x1,0x6,0x98,0xd,0x6,0x98,0xd,0x6,0x98,
0xd,0x6,0x98,0xd,0x6,0x98,0xd,0x6,0x98,0xd,0x6,0x98,0xd,0x6,0x98,0xd,0x6,0x98,0xd,0x6,0x98,0xd,0x6,0x98,0xd,0x6,0x98,0xd},
	{TMDS_0p25G,0x14,0x14,0x14,0x14,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x0,0x3,0x3,0x3,0x1,0x1,0x1,0x6,0xa0,0xf,0x6,0xa0,0xf,0x6,0xa0,
0xf,0x6,0xa0,0xf,0x6,0xa0,0xf,0x6,0xa0,0xf,0x6,0xa0,0xf,0x6,0xa0,0xf,0x6,0xa0,0xf,0x6,0xa0,0xf,0x6,0xa0,0xf,0x0,0x0,0x0},
	{TMDS_4p445G,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf},
	{TMDS_3p7125G,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf},
	{TMDS_2p2275G,0x12,0x12,0x12,0x12,0x4,0x4,0x4,0x4,0x1,0x1,0x1,0x1,0x2,0x2,0x2,0x1,0x1,0x1,0x2,0x98,0x7,0x2,0x98,0x7,0x2,0x98,
0x7,0x2,0x98,0x7,0x2,0x98,0x7,0x2,0x98,0x7,0x2,0x98,0x7,0x2,0x98,0x7,0x2,0x98,0x7,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf},
	{TMDS_1p85625G,0x12,0x12,0x12,0x12,0x4,0x4,0x4,0x4,0x1,0x1,0x1,0x1,0x2,0x2,0x2,0x1,0x1,0x1,0x2,0x98,0x7,0x2,0x98,0x7,0x2,0x98,
0x7,0x2,0x98,0x7,0x2,0x98,0x7,0x2,0x98,0x7,0x2,0x98,0x7,0x2,0x98,0x7,0x2,0x98,0x7,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf},
	{TMDS_1p11375G,0x14,0x14,0x14,0x14,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x0,0x3,0x3,0x3,0x1,0x1,0x1,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,
0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf},
	{TMDS_0p928125G,0x14,0x14,0x14,0x14,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x0,0x3,0x3,0x3,0x1,0x1,0x1,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,
0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf},
	{TMDS_0p54G,0x14,0x14,0x14,0x14,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x0,0x3,0x3,0x3,0x1,0x1,0x1,0x6,0x98,0xd,0x6,0x98,0xd,0x6,0x98,
0xd,0x6,0x98,0xd,0x6,0x98,0xd,0x6,0x98,0xd,0x6,0x98,0xd,0x6,0x98,0xd,0x6,0x98,0xd,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf},
	{TMDS_0p405G,0x14,0x14,0x14,0x14,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x0,0x3,0x3,0x3,0x1,0x1,0x1,0x6,0x98,0xd,0x6,0x98,0xd,0x6,0x98,
0xd,0x6,0x98,0xd,0x6,0x98,0xd,0x6,0x98,0xd,0x6,0x98,0xd,0x6,0x98,0xd,0x6,0x98,0xd,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf},
	{TMDS_0p3375G,0x14,0x14,0x14,0x14,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x0,0x3,0x3,0x3,0x1,0x1,0x1,0x6,0xa0,0xf,0x6,0xa0,0xf,0x6,0xa0,
0xf,0x6,0xa0,0xf,0x6,0xa0,0xf,0x6,0xa0,0xf,0x6,0xa0,0xf,0x6,0xa0,0xf,0x6,0xa0,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf},
	{TMDS_0p27G,0x14,0x14,0x14,0x14,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x0,0x3,0x3,0x3,0x1,0x1,0x1,0x6,0xa0,0xf,0x6,0xa0,0xf,0x6,0xa0,
0xf,0x6,0xa0,0xf,0x6,0xa0,0xf,0x6,0xa0,0xf,0x6,0xa0,0xf,0x6,0xa0,0xf,0x6,0xa0,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf},
	{TMDS_OLT_5p94G,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x6,0xb8,0xf,0x2,0x98,0xf,0x2,0x98,
0xf,0x6,0xb8,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x6,0xb8,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf},
	{TMDS_CP_5p94G,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x6,0xb8,0xf,0x2,0x98,0xf,0x2,0x98,
0xf,0x6,0xb8,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x6,0xb8,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf,0x2,0x98,0xf},
};

Koffset_tmds_1_T Koffset_tmds_1[] =
{
	{TMDS_5p94G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,
0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0xf,0xf,0xf},
	{TMDS_2p97G,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,
0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x3,0x3,0x3},
	{TMDS_1p485G,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,
0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_0p742G,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,
0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_0p25G,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,
0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_4p445G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,
0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0xf,0xf,0xf},
	{TMDS_3p7125G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,
0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0xf,0xf,0xf},
	{TMDS_2p2275G,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,
0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x3,0x3,0x3},
	{TMDS_1p85625G,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x3,0x3,0x3},
	{TMDS_1p11375G,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_0p928125G,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_0p54G,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,
0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_0p405G,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,
0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_0p3375G,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,
0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_0p27G,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x1,
0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_OLT_5p94G,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xf,0xf,0xf},
	{TMDS_CP_5p94G,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xf,0xf,0xf},
};

LEQ_VTH_Tap0_3_4_Adapt_tmds_1_T LEQ_VTH_Tap0_3_4_Adapt_tmds_1[] =
{
	{TMDS_5p94G,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x1,0x1},
	{TMDS_2p97G,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0},
	{TMDS_1p485G,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p742G,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0},
	{TMDS_0p25G,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x1,0x1},
	{TMDS_4p445G,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x1,0x1},
	{TMDS_3p7125G,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x1,0x1},
	{TMDS_2p2275G,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0},
	{TMDS_1p85625G,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0},
	{TMDS_1p11375G,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p928125G,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p54G,0x0,0x1,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0},
	{TMDS_0p405G,0x0,0x1,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x0},
	{TMDS_0p3375G,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x1,0x1},
	{TMDS_0p27G,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x1,0x0,0x1,0x1},
	{TMDS_OLT_5p94G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_CP_5p94G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
};

Tap0_to_Tap4_Adapt_tmds_1_T Tap0_to_Tap4_Adapt_tmds_1[] =
{
	{TMDS_5p94G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_2p97G,0x1,0x1,0x1,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x1,0x1,0x1,0x0,0x0},
	{TMDS_1p485G,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p742G,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p25G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_4p445G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_3p7125G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_2p2275G,0x1,0x1,0x1,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x1,0x1,0x1,0x0,0x0},
	{TMDS_1p85625G,0x1,0x1,0x1,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x1,0x1,0x1,0x0,0x0,0x1,0x1,0x1,0x0,0x0},
	{TMDS_1p11375G,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p928125G,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p54G,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p405G,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0},
	{TMDS_0p3375G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_0p27G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_OLT_5p94G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_CP_5p94G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
};

void Mac8p_TMDS_setting_Main_Seq(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	HDMI_WARN(" TMDS_setting_Main_Seq Ver.2022_02_02\n");
	if(value_param==TMDS_5p94G || value_param==TMDS_2p97G || value_param==TMDS_1p485G || value_param==TMDS_0p742G || value_param==TMDS_4p445G || value_param==TMDS_3p7125G || value_param==TMDS_2p2275G || value_param==TMDS_1p85625G || value_param==TMDS_1p11375G || value_param==TMDS_0p928125G || value_param==TMDS_0p54G || value_param==TMDS_0p405G || value_param==TMDS_OLT_5p94G || value_param==TMDS_CP_5p94G )
	{
		DFE_ini_tmds_1_func(nport, value_param);
		DPHY_Fix_tmds_1_func(nport, value_param);
		DPHY_Para_tmds_1_func(nport, value_param);
		APHY_Fix_tmds_1_func(nport, value_param);
		APHY_Para_tmds_1_func(nport, value_param);
		DPHY_Init_Flow_tmds_1_func(nport, value_param);
		APHY_Init_Flow_tmds_1_func(nport, value_param);
		DCDR_settings_tmds_1_func(nport, value_param);
		DCDR_settings_tmds_2_func(nport, value_param);
		Koffset_tmds_1_func(nport, value_param);
		LEQ_VTH_Tap0_3_4_Adapt_tmds_1_func(nport, value_param);
		Koffset_tmds_1_func(nport, value_param);
		Tap0_to_Tap4_Adapt_tmds_1_func(nport, value_param);
		Koffset_tmds_1_func(nport, value_param);
		DCDR_settings_tmds_3_func(nport, value_param);
	}
	else if(value_param==TMDS_0p25G || value_param==TMDS_0p3375G || value_param==TMDS_0p27G )
	{
		DFE_ini_tmds_1_func(nport, value_param);
		DPHY_Fix_tmds_1_func(nport, value_param);
		DPHY_Para_tmds_1_func(nport, value_param);
		APHY_Fix_tmds_1_func(nport, value_param);
		APHY_Para_tmds_1_func(nport, value_param);
		DPHY_Init_Flow_tmds_1_func(nport, value_param);
		APHY_Init_Flow_tmds_1_func(nport, value_param);
		DCDR_settings_tmds_1_func(nport, value_param);
		DCDR_settings_tmds_2_func(nport, value_param);
		Koffset_tmds_1_func(nport, value_param);
		Manual_DFE_tmds_1_func(nport, value_param);
		Koffset_tmds_1_func(nport, value_param);
		DCDR_settings_tmds_3_func(nport, value_param);
	}
}

void DFE_ini_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	_phy_rtd_outl(0x1803bc70,0x00000000);
	//R lane
	_phy_rtd_outl(0x1803be00,0x00000000);
	_phy_rtd_outl(0x1803be04,0x00000000);
	_phy_rtd_outl(0x1803be08,0x20C00000);
	_phy_rtd_outl(0x1803be0c,0x05b45c00);
	_phy_rtd_outl(0x1803be10,0xc0004000);
	_phy_rtd_outl(0x1803be14,0x00280000);
	_phy_rtd_outl(0x1803be18,0x1038ffe0);
	_phy_rtd_outl(0x1803be1c,0x7FF07FE0);
	_phy_rtd_outl(0x1803be20,0x3FFF83E0);
	_phy_rtd_outl(0x1803be24,0x00000000);
	_phy_rtd_outl(0x1803be28,0x009D00E7);
	_phy_rtd_outl(0x1803be2c,0xFF00FFC0);
	_phy_rtd_outl(0x1803be30,0xff005400);
	//G lane
	_phy_rtd_outl(0x1803be40,0x00000000);
	_phy_rtd_outl(0x1803be44,0x00000000);
	_phy_rtd_outl(0x1803be48,0x20C00000);
	_phy_rtd_outl(0x1803be4c,0x05b45c00);
	_phy_rtd_outl(0x1803be50,0xc0004000);
	_phy_rtd_outl(0x1803be54,0x00280000);
	_phy_rtd_outl(0x1803be58,0x1038ffe0);
	_phy_rtd_outl(0x1803be5c,0x7FF07FE0);
	_phy_rtd_outl(0x1803be60,0x3FFF83E0);
	_phy_rtd_outl(0x1803be64,0x00000000);
	_phy_rtd_outl(0x1803be68,0x009D00E7);
	_phy_rtd_outl(0x1803be6c,0xFF00FFC0);
	_phy_rtd_outl(0x1803be70,0xff005400);
	//B lane
	_phy_rtd_outl(0x1803be80,0x00000000);
	_phy_rtd_outl(0x1803be84,0x00000000);
	_phy_rtd_outl(0x1803be88,0x20C00000);
	_phy_rtd_outl(0x1803be8c,0x05b45c00);
	_phy_rtd_outl(0x1803be90,0xc0004000);
	_phy_rtd_outl(0x1803be94,0x00280000);
	_phy_rtd_outl(0x1803be98,0x1038ffe0);
	_phy_rtd_outl(0x1803be9c,0x7FF07FE0);
	_phy_rtd_outl(0x1803bea0,0x3FFF83E0);
	_phy_rtd_outl(0x1803bea4,0x00000000);
	_phy_rtd_outl(0x1803bea8,0x009D00E7);
	_phy_rtd_outl(0x1803beac,0xFF00FFC0);
	_phy_rtd_outl(0x1803beb0,0xff005400);
	//CK lane
	_phy_rtd_outl(0x1803bec0,0x00000000);
	_phy_rtd_outl(0x1803bec4,0x00000000);
	_phy_rtd_outl(0x1803bec8,0x20C00000);
	_phy_rtd_outl(0x1803becc,0x05b45c00);
	_phy_rtd_outl(0x1803bed0,0xc0004000);
	_phy_rtd_outl(0x1803bed4,0x00280000);
	_phy_rtd_outl(0x1803bed8,0x1038ffe0);
	_phy_rtd_outl(0x1803bedc,0x7FF07FE0);
	_phy_rtd_outl(0x1803bee0,0x3e0383e0);
	_phy_rtd_outl(0x1803bee4,0x00000000);
	_phy_rtd_outl(0x1803bee8,0x009D00E7);
	_phy_rtd_outl(0x1803beec,0xFF00FFC0);
	_phy_rtd_outl(0x1803bef0,0xff005400);
	// *************************************************************************************************************************************
	//B-Lane_initial DFE Value_Start
	_phy_rtd_part_outl(0x1803bea8, 9, 5, 14); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_phy_rtd_part_outl(0x1803bea8, 4, 0, 14); //VTHN_INIT_B[4:0]	4:0	R/W	0x0F		VTHN INIT
	_phy_rtd_part_outl(0x1803bea0, 4, 0, DFE_ini_tmds_1[value_param].LEQ_INIT_B_4_0_62); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_phy_rtd_part_outl(0x1803bea4, 31, 24, DFE_ini_tmds_1[value_param].TAP0_INIT_B_7_0_63); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_phy_rtd_part_outl(0x1803bea4, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_phy_rtd_part_outl(0x1803bea4, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_phy_rtd_part_outl(0x1803bea4, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_phy_rtd_part_outl(0x1803bea4, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//B-Lane DFE COEF LOAD IN
	_phy_rtd_part_outl(0x1803be8c, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_phy_rtd_part_outl(0x1803be8c, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_phy_rtd_part_outl(0x1803be8c, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_phy_rtd_part_outl(0x1803be8c, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_phy_rtd_part_outl(0x1803be8c, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_phy_rtd_part_outl(0x1803be8c, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_phy_rtd_part_outl(0x1803be88, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_phy_rtd_part_outl(0x1803be8c, 4, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_phy_rtd_part_outl(0x1803be8c, 5, 5, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_phy_rtd_part_outl(0x1803be88, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	//B-Lane_initial DFE Value_Stop
	// *************************************************************************************************************************************
	//G-Lane_initial DFE Value_Start
	_phy_rtd_part_outl(0x1803be68, 9, 5, 14); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_phy_rtd_part_outl(0x1803be68, 4, 0, 14); //VTHN_INIT_B[4:0]	4:0	R/W	0x0F		VTHN INIT
	_phy_rtd_part_outl(0x1803be60, 4, 0, DFE_ini_tmds_1[value_param].LEQ_INIT_G_4_0_84); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_phy_rtd_part_outl(0x1803be64, 31, 24, DFE_ini_tmds_1[value_param].TAP0_INIT_G_7_0_85); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_phy_rtd_part_outl(0x1803be64, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_phy_rtd_part_outl(0x1803be64, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_phy_rtd_part_outl(0x1803be64, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_phy_rtd_part_outl(0x1803be64, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//G-Lane DFE COEF LOAD IN
	_phy_rtd_part_outl(0x1803be4c, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_phy_rtd_part_outl(0x1803be4c, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_phy_rtd_part_outl(0x1803be4c, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_phy_rtd_part_outl(0x1803be4c, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_phy_rtd_part_outl(0x1803be4c, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_phy_rtd_part_outl(0x1803be4c, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_phy_rtd_part_outl(0x1803be48, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_phy_rtd_part_outl(0x1803be4c, 4, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_phy_rtd_part_outl(0x1803be4c, 5, 5, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_phy_rtd_part_outl(0x1803be48, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	//G-Lane_initial DFE Value_Stop
	// *************************************************************************************************************************************
	//R-Lane_initial DFE Value_Start
	_phy_rtd_part_outl(0x1803be28, 9, 5, 14); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_phy_rtd_part_outl(0x1803be28, 4, 0, 14); //VTHN_INIT_B[4:0]	4:0	R/W	0x0F		VTHN INIT
	_phy_rtd_part_outl(0x1803be20, 4, 0, DFE_ini_tmds_1[value_param].LEQ_INIT_R_4_0_106); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_phy_rtd_part_outl(0x1803be24, 31, 24, DFE_ini_tmds_1[value_param].TAP0_INIT_R_7_0_107); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_phy_rtd_part_outl(0x1803be24, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_phy_rtd_part_outl(0x1803be24, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_phy_rtd_part_outl(0x1803be24, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_phy_rtd_part_outl(0x1803be24, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//R-Lane DFE COEF LOAD IN
	_phy_rtd_part_outl(0x1803be0c, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_phy_rtd_part_outl(0x1803be0c, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_phy_rtd_part_outl(0x1803be0c, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_phy_rtd_part_outl(0x1803be0c, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_phy_rtd_part_outl(0x1803be0c, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_phy_rtd_part_outl(0x1803be0c, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_phy_rtd_part_outl(0x1803be08, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_phy_rtd_part_outl(0x1803be0c, 4, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_phy_rtd_part_outl(0x1803be0c, 5, 5, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_phy_rtd_part_outl(0x1803be08, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	//R-Lane_initial DFE Value_Stop
	// *************************************************************************************************************************************
	//CK-Lane_initial DFE Value_Start
	_phy_rtd_part_outl(0x1803bee8, 9, 5, 14); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_phy_rtd_part_outl(0x1803bee8, 4, 0, 14); //VTHN_INIT_B`[4:0]	4:0	R/W	0x0F		VTHN INIT
	_phy_rtd_part_outl(0x1803bee0, 4, 0, DFE_ini_tmds_1[value_param].LEQ_INIT_CK_4_0_128); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_phy_rtd_part_outl(0x1803bee4, 31, 24, DFE_ini_tmds_1[value_param].TAP0_INIT_CK_7_0_129); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_phy_rtd_part_outl(0x1803bee4, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_phy_rtd_part_outl(0x1803bee4, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_phy_rtd_part_outl(0x1803bee4, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_phy_rtd_part_outl(0x1803bee4, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//CK-Lane DFE COEF LOAD IN
	_phy_rtd_part_outl(0x1803becc, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_phy_rtd_part_outl(0x1803becc, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_phy_rtd_part_outl(0x1803becc, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_phy_rtd_part_outl(0x1803becc, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_phy_rtd_part_outl(0x1803becc, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_phy_rtd_part_outl(0x1803becc, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_phy_rtd_part_outl(0x1803bec8, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_phy_rtd_part_outl(0x1803becc, 4, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_phy_rtd_part_outl(0x1803becc, 5, 5, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_phy_rtd_part_outl(0x1803bec8, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	//CK-Lane_initial DFE Value_Stop
	//**************************************************************************************************************************************
	_phy_rtd_part_outl(0x1803bef4, 17, 15, 6); //rotation_mode Adaption flow control mode
	_phy_rtd_part_outl(0x1803beb4, 17, 15, 6); //rotation_mode Adaption flow control mode
	_phy_rtd_part_outl(0x1803be74, 17, 15, 6); //rotation_mode Adaption flow control mode
	_phy_rtd_part_outl(0x1803be34, 17, 15, 6); //rotation_mode Adaption flow control mode
}

void DPHY_Fix_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	//DPHY fix start
	_phy_rtd_part_outl(0x1803bb00, 0, 0, 0x1); //[DPHY fix]  reg_p0_b_dig_rst_n
	_phy_rtd_part_outl(0x1803bb00, 1, 1, 0x1); //[DPHY fix]  reg_p0_g_dig_rst_n
	_phy_rtd_part_outl(0x1803bb00, 2, 2, 0x1); //[DPHY fix]  reg_p0_r_dig_rst_n
	_phy_rtd_part_outl(0x1803bb00, 3, 3, 0x1); //[DPHY fix]  reg_p0_ck_dig_rst_n
	_phy_rtd_part_outl(0x1803bb00, 4, 4, 0x1); //[DPHY fix]  reg_p0_b_cdr_rst_n
	_phy_rtd_part_outl(0x1803bb00, 5, 5, 0x1); //[DPHY fix]  reg_p0_g_cdr_rst_n
	_phy_rtd_part_outl(0x1803bb00, 6, 6, 0x1); //[DPHY fix]  reg_p0_r_cdr_rst_n
	_phy_rtd_part_outl(0x1803bb00, 7, 7, 0x1); //[DPHY fix]  reg_p0_ck_cdr_rst_n
	_phy_rtd_part_outl(0x1803bb00, 8, 8, 0x0); //[DPHY fix]  reg_p0_b_clk_inv
	_phy_rtd_part_outl(0x1803bb00, 9, 9, 0x0); //[DPHY fix]  reg_p0_g_clk_inv
	_phy_rtd_part_outl(0x1803bb00, 10, 10, 0x0); //[DPHY fix]  reg_p0_r_clk_inv
	_phy_rtd_part_outl(0x1803bb00, 11, 11, 0x0); //[DPHY fix]  reg_p0_ck_clk_inv
	_phy_rtd_part_outl(0x1803bb00, 12, 12, 0x0); //[DPHY fix]  reg_p0_b_shift_inv
	_phy_rtd_part_outl(0x1803bb00, 13, 13, 0x0); //[DPHY fix]  reg_p0_g_shift_inv
	_phy_rtd_part_outl(0x1803bb00, 14, 14, 0x0); //[DPHY fix]  reg_p0_r_shift_inv
	_phy_rtd_part_outl(0x1803bb00, 15, 15, 0x0); //[DPHY fix]  reg_p0_ck_shift_inv
	_phy_rtd_part_outl(0x1803bb00, 24, 24, 0x0); //[DPHY fix]  reg_p0_b_data_order
	_phy_rtd_part_outl(0x1803bb00, 25, 25, 0x0); //[DPHY fix]  reg_p0_g_data_order
	_phy_rtd_part_outl(0x1803bb00, 26, 26, 0x0); //[DPHY fix]  reg_p0_r_data_order
	_phy_rtd_part_outl(0x1803bb00, 27, 27, 0x0); //[DPHY fix]  reg_p0_ck_data_order
	_phy_rtd_part_outl(0x1803bb00, 28, 28, 0x0); //[DPHY fix]  reg_p0_b_dyn_kp_en
	_phy_rtd_part_outl(0x1803bb00, 29, 29, 0x0); //[DPHY fix]  reg_p0_g_dyn_kp_en
	_phy_rtd_part_outl(0x1803bb00, 30, 30, 0x0); //[DPHY fix]  reg_p0_r_dyn_kp_en
	_phy_rtd_part_outl(0x1803bb00, 31, 31, 0x0); //[DPHY fix]  reg_p0_ck_dyn_kp_en
	_phy_rtd_part_outl(0x1803bb04, 24, 24, 0x0); //[]  reg_p0_b_dfe_edge_out
	_phy_rtd_part_outl(0x1803bb04, 25, 25, 0x0); //[]  reg_p0_b_dfe_edge_out
	_phy_rtd_part_outl(0x1803bb04, 26, 26, 0x0); //[]  reg_p0_r_dfe_edge_out
	_phy_rtd_part_outl(0x1803bb04, 27, 27, 0x0); //[]  reg_p0_ck_dfe_edge_out
	_phy_rtd_part_outl(0x1803bb10, 0, 0, 0x0); //[]  reg_p0_b_bypass_sdm_int
	_phy_rtd_part_outl(0x1803bb10, 11, 1, 0x000); //[]  reg_p0_b_int_init
	_phy_rtd_part_outl(0x1803bb10, 21, 12, 0x000); //[]  reg_p0_b_acc2_period
	_phy_rtd_part_outl(0x1803bb10, 22, 22, 0x0); //[]  reg_p0_b_acc2_manual
	_phy_rtd_part_outl(0x1803bb10, 23, 23, 0x0); //[]  reg_p0_b_squ_tri
	_phy_rtd_part_outl(0x1803bb14, 0, 0, 0x0); //[]  reg_p0_g_bypass_sdm_int
	_phy_rtd_part_outl(0x1803bb14, 11, 1, 0x000); //[]  reg_p0_g_int_init
	_phy_rtd_part_outl(0x1803bb14, 21, 12, 0x000); //[]  reg_p0_g_acc2_period
	_phy_rtd_part_outl(0x1803bb14, 22, 22, 0x0); //[]  reg_p0_g_acc2_manual
	_phy_rtd_part_outl(0x1803bb14, 23, 23, 0x0); //[]  reg_p0_g_squ_tri
	_phy_rtd_part_outl(0x1803bb18, 0, 0, 0x0); //[]  reg_p0_r_bypass_sdm_int
	_phy_rtd_part_outl(0x1803bb18, 11, 1, 0x000); //[]  reg_p0_r_int_init
	_phy_rtd_part_outl(0x1803bb18, 21, 12, 0x000); //[]  reg_p0_r_acc2_period
	_phy_rtd_part_outl(0x1803bb18, 22, 22, 0x0); //[]  reg_p0_r_acc2_manual
	_phy_rtd_part_outl(0x1803bb18, 23, 23, 0x0); //[]  reg_p0_r_squ_tri
	_phy_rtd_part_outl(0x1803bb1c, 0, 0, 0x0); //[]  reg_p0_ck_bypass_sdm_int
	_phy_rtd_part_outl(0x1803bb1c, 11, 1, 0x000); //[]  reg_p0_ck_int_init
	_phy_rtd_part_outl(0x1803bb1c, 21, 12, 0x000); //[]  reg_p0_ck_acc2_period
	_phy_rtd_part_outl(0x1803bb1c, 22, 22, 0x0); //[]  reg_p0_ck_acc2_manual
	_phy_rtd_part_outl(0x1803bb1c, 23, 23, 0x0); //[]  reg_p0_ck_squ_tri
	_phy_rtd_part_outl(0x1803bb20, 0, 0, DPHY_Fix_tmds_1[value_param].b_pi_m_mode_50); //[]  reg_p0_b_pi_m_mode
	_phy_rtd_part_outl(0x1803bb20, 3, 1, 0x0); //[]  reg_p0_b_testout_sel
	_phy_rtd_part_outl(0x1803bb20, 8, 4, 0x00); //[]  reg_p0_b_timer_lpf
	_phy_rtd_part_outl(0x1803bb20, 13, 9, 0x00); //[]  reg_p0_b_timer_eq
	_phy_rtd_part_outl(0x1803bb20, 18, 14, 0x00); //[]  reg_p0_b_timer_ber
	_phy_rtd_part_outl(0x1803bb24, 0, 0, DPHY_Fix_tmds_1[value_param].g_pi_m_mode_55); //[]  reg_p0_g_pi_m_mode
	_phy_rtd_part_outl(0x1803bb24, 3, 1, 0x0); //[]  reg_p0_g_testout_sel
	_phy_rtd_part_outl(0x1803bb24, 8, 4, 0x00); //[]  reg_p0_g_timer_lpf
	_phy_rtd_part_outl(0x1803bb24, 13, 9, 0x00); //[]  reg_p0_g_timer_eq
	_phy_rtd_part_outl(0x1803bb24, 18, 14, 0x00); //[]  reg_p0_g_timer_ber
	_phy_rtd_part_outl(0x1803bb28, 0, 0, DPHY_Fix_tmds_1[value_param].r_pi_m_mode_60); //[]  reg_p0_r_pi_m_mode
	_phy_rtd_part_outl(0x1803bb28, 3, 1, 0x0); //[]  reg_p0_r_testout_sel
	_phy_rtd_part_outl(0x1803bb28, 8, 4, 0x00); //[]  reg_p0_r_timer_lpf
	_phy_rtd_part_outl(0x1803bb28, 13, 9, 0x00); //[]  reg_p0_r_timer_eq
	_phy_rtd_part_outl(0x1803bb28, 18, 14, 0x00); //[]  reg_p0_r_timer_ber
	_phy_rtd_part_outl(0x1803bb2c, 0, 0, DPHY_Fix_tmds_1[value_param].ck_pi_m_mode_65); //[]  reg_p0_ck_pi_m_mode
	_phy_rtd_part_outl(0x1803bb2c, 3, 1, 0x0); //[]  reg_p0_ck_testout_sel
	_phy_rtd_part_outl(0x1803bb2c, 8, 4, 0x00); //[]  reg_p0_ck_timer_lpf
	_phy_rtd_part_outl(0x1803bb2c, 13, 9, 0x00); //[]  reg_p0_ck_timer_eq
	_phy_rtd_part_outl(0x1803bb2c, 18, 14, 0x00); //[]  reg_p0_ck_timer_ber
	_phy_rtd_outl(0x1803bb30,0x000000ff); //[]  reg_p0_b_st_m_value
	_phy_rtd_outl(0x1803bb34,0x000000ff); //[]  reg_p0_g_st_m_value
	_phy_rtd_outl(0x1803bb38,0x000000ff); //[]  reg_p0_r_st_m_value
	_phy_rtd_outl(0x1803bb3c,0x000000ff); //[]  reg_p0_ck_st_m_value
	//Clock Ready
	_phy_rtd_part_outl(0x1803bb40, 0, 0, 0x0); //[]  reg_p0_b_force_clkrdy
	_phy_rtd_part_outl(0x1803bb40, 1, 1, 0x0); //[]  reg_p0_g_force_clkrdy
	_phy_rtd_part_outl(0x1803bb40, 2, 2, 0x0); //[]  reg_p0_r_force_clkrdy
	_phy_rtd_part_outl(0x1803bb40, 3, 3, 0x0); //[]  reg_p0_ck_force_clkrdy
	_phy_rtd_part_outl(0x1803bb40, 7, 4, 0x0); //[]  reg_p0_b_timer_clk
	_phy_rtd_part_outl(0x1803bb40, 11, 8, 0x0); //[]  reg_p0_g_timer_clk
	_phy_rtd_part_outl(0x1803bb40, 15, 12, 0x0); //[]  reg_p0_r_timer_clk
	_phy_rtd_part_outl(0x1803bb40, 19, 16, 0x0); //[]  reg_p0_ck_timer_clk
	_phy_rtd_part_outl(0x1803bb44, 11, 8, 0xf); //[DPHY fix]  reg_p0_b_offset_delay_cnt
	_phy_rtd_part_outl(0x1803bb44, 13, 12, 0x0); //[DPHY fix]  reg_p0_b_offset_stable_cnt
	_phy_rtd_part_outl(0x1803bb44, 19, 14, 0x0b); //[DPHY fix]  reg_p0_b_offset_divisor
	_phy_rtd_part_outl(0x1803bb44, 26, 20, 0x28); //[DPHY fix]  reg_p0_b_offset_timeout
	_phy_rtd_part_outl(0x1803bb48, 0, 0, 0x0); //[]  reg_p0_b_offset_en_ope
	_phy_rtd_part_outl(0x1803bb48, 1, 1, 0x0); //[]  reg_p0_b_offset_pc_ope
	_phy_rtd_part_outl(0x1803bb48, 6, 2, 0x0); //[]  reg_p0_b_offset_ini_ope
	_phy_rtd_part_outl(0x1803bb48, 7, 7, 0x0); //[]  reg_p0_b_offset_gray_en_ope
	_phy_rtd_part_outl(0x1803bb48, 8, 8, 0x0); //[]  reg_p0_b_offset_manual_ope
	_phy_rtd_part_outl(0x1803bb48, 9, 9, 0x0); //[]  reg_p0_b_offset_z0_ok_ope
	_phy_rtd_part_outl(0x1803bb48, 10, 10, 0x0); //[]  reg_p0_b_offset_en_opo
	_phy_rtd_part_outl(0x1803bb48, 11, 11, 0x0); //[]  reg_p0_b_offset_pc_opo
	_phy_rtd_part_outl(0x1803bb48, 16, 12, 0x0); //[]  reg_p0_b_offset_ini_opo
	_phy_rtd_part_outl(0x1803bb48, 17, 17, 0x0); //[]  reg_p0_b_offset_gray_en_opo
	_phy_rtd_part_outl(0x1803bb48, 18, 18, 0x0); //[]  reg_p0_b_offset_manual_opo
	_phy_rtd_part_outl(0x1803bb48, 19, 19, 0x0); //[]  reg_p0_b_offset_z0_ok_opo
	_phy_rtd_part_outl(0x1803bb48, 20, 20, 0x0); //[]  reg_p0_b_offset_en_one
	_phy_rtd_part_outl(0x1803bb48, 21, 21, 0x0); //[]  reg_p0_b_offset_pc_one
	_phy_rtd_part_outl(0x1803bb48, 26, 22, 0x0); //[]  reg_p0_b_offset_ini_one
	_phy_rtd_part_outl(0x1803bb48, 27, 27, 0x0); //[]  reg_p0_b_offset_gray_en_one
	_phy_rtd_part_outl(0x1803bb48, 28, 28, 0x0); //[]  reg_p0_b_offset_manual_one
	_phy_rtd_part_outl(0x1803bb48, 29, 29, 0x0); //[]  reg_p0_b_offset_z0_ok_one
	_phy_rtd_part_outl(0x1803bb4c, 0, 0, 0x0); //[DPHY fix]  reg_p0_b_offset_en_ono
	_phy_rtd_part_outl(0x1803bb4c, 1, 1, 0x0); //[DPHY fix]  reg_p0_b_offset_pc_ono
	_phy_rtd_part_outl(0x1803bb4c, 6, 2, 0x0); //[DPHY fix]  reg_p0_b_offset_ini_ono
	_phy_rtd_part_outl(0x1803bb4c, 7, 7, 0x0); //[DPHY fix]  reg_p0_b_offset_gray_en_ono
	_phy_rtd_part_outl(0x1803bb4c, 8, 8, 0x0); //[DPHY fix]  reg_p0_b_offset_manual_ono
	_phy_rtd_part_outl(0x1803bb4c, 9, 9, 0x0); //[DPHY fix]  reg_p0_b_offset_z0_ok_ono
	_phy_rtd_part_outl(0x1803bb4c, 16, 12, 0x0); //[DPHY fix]  reg_p0_b_offset_ini_de
	_phy_rtd_part_outl(0x1803bb4c, 17, 17, 0x0); //[DPHY fix]  reg_p0_b_offset_gray_en_de
	_phy_rtd_part_outl(0x1803bb4c, 26, 22, 0x0); //[DPHY fix]  reg_p0_b_offset_ini_do
	_phy_rtd_part_outl(0x1803bb4c, 27, 27, 0x0); //[DPHY fix]  reg_p0_b_offset_gray_en_do
	_phy_rtd_part_outl(0x1803bb50, 6, 2, 0x0); //[DPHY fix]  reg_p0_b_offset_ini_ee
	_phy_rtd_part_outl(0x1803bb50, 7, 7, 0x0); //[DPHY fix]  reg_p0_b_offset_gray_en_ee
	_phy_rtd_part_outl(0x1803bb50, 16, 12, 0x0); //[DPHY fix]  reg_p0_b_offset_ini_eo
	_phy_rtd_part_outl(0x1803bb50, 17, 17, 0x0); //[DPHY fix]  reg_p0_b_offset_gray_en_eo
	_phy_rtd_part_outl(0x1803bb50, 26, 22, 0x0); //[DPHY fix]  reg_p0_b_offset_ini_eq
	_phy_rtd_part_outl(0x1803bb50, 27, 27, 0x0); //[DPHY fix]  reg_p0_b_offset_gray_en_eq
	_phy_rtd_part_outl(0x1803bb50, 29, 29, 0x0); //[DPHY fix]  reg_p0_b_offset_z0_ok_eq
	_phy_rtd_part_outl(0x1803bb54, 11, 8, 0xf); //[DPHY fix]  reg_p0_g_offset_delay_cnt
	_phy_rtd_part_outl(0x1803bb54, 13, 12, 0x0); //[DPHY fix]  reg_p0_g_offset_stable_cnt
	_phy_rtd_part_outl(0x1803bb54, 19, 14, 0x0b); //[DPHY fix]  reg_p0_g_offset_divisor
	_phy_rtd_part_outl(0x1803bb54, 26, 20, 0x28); //[DPHY fix]  reg_p0_g_offset_timeout
	_phy_rtd_part_outl(0x1803bb58, 0, 0, 0x0); //[]  reg_p0_g_offset_en_ope
	_phy_rtd_part_outl(0x1803bb58, 1, 1, 0x0); //[]  reg_p0_g_offset_pc_ope
	_phy_rtd_part_outl(0x1803bb58, 6, 2, 0x0); //[]  reg_p0_g_offset_ini_ope
	_phy_rtd_part_outl(0x1803bb58, 7, 7, 0x0); //[]  reg_p0_g_offset_gray_en_ope
	_phy_rtd_part_outl(0x1803bb58, 8, 8, 0x0); //[]  reg_p0_g_offset_manual_ope
	_phy_rtd_part_outl(0x1803bb58, 9, 9, 0x0); //[]  reg_p0_g_offset_z0_ok_ope
	_phy_rtd_part_outl(0x1803bb58, 10, 10, 0x0); //[]  reg_p0_g_offset_en_opo
	_phy_rtd_part_outl(0x1803bb58, 11, 11, 0x0); //[]  reg_p0_g_offset_pc_opo
	_phy_rtd_part_outl(0x1803bb58, 16, 12, 0x0); //[]  reg_p0_g_offset_ini_opo
	_phy_rtd_part_outl(0x1803bb58, 17, 17, 0x0); //[]  reg_p0_g_offset_gray_en_opo
	_phy_rtd_part_outl(0x1803bb58, 18, 18, 0x0); //[]  reg_p0_g_offset_manual_opo
	_phy_rtd_part_outl(0x1803bb58, 19, 19, 0x0); //[]  reg_p0_g_offset_z0_ok_opo
	_phy_rtd_part_outl(0x1803bb58, 20, 20, 0x0); //[]  reg_p0_g_offset_en_one
	_phy_rtd_part_outl(0x1803bb58, 21, 21, 0x0); //[]  reg_p0_g_offset_pc_one
	_phy_rtd_part_outl(0x1803bb58, 26, 22, 0x0); //[]  reg_p0_g_offset_ini_one
	_phy_rtd_part_outl(0x1803bb58, 27, 27, 0x0); //[]  reg_p0_g_offset_gray_en_one
	_phy_rtd_part_outl(0x1803bb58, 28, 28, 0x0); //[]  reg_p0_g_offset_manual_one
	_phy_rtd_part_outl(0x1803bb58, 29, 29, 0x0); //[]  reg_p0_g_offset_z0_ok_one
	_phy_rtd_part_outl(0x1803bb5c, 0, 0, 0x0); //[DPHY fix]  reg_p0_g_offset_en_ono
	_phy_rtd_part_outl(0x1803bb5c, 6, 2, 0x0); //[DPHY fix]  reg_p0_g_offset_ini_ono
	_phy_rtd_part_outl(0x1803bb5c, 7, 7, 0x0); //[DPHY fix]  reg_p0_g_offset_gray_en_ono
	_phy_rtd_part_outl(0x1803bb5c, 1, 1, 0x0); //[DPHY fix]  reg_p0_g_offset_pc_ono
	_phy_rtd_part_outl(0x1803bb5c, 8, 8, 0x0); //[DPHY fix]  reg_p0_g_offset_manual_ono
	_phy_rtd_part_outl(0x1803bb5c, 9, 9, 0x0); //[DPHY fix]  reg_p0_g_offset_z0_ok_ono
	_phy_rtd_part_outl(0x1803bb5c, 16, 12, 0x0); //[DPHY fix]  reg_p0_g_offset_ini_de
	_phy_rtd_part_outl(0x1803bb5c, 17, 17, 0x0); //[DPHY fix]  reg_p0_g_offset_gray_en_de
	_phy_rtd_part_outl(0x1803bb5c, 26, 22, 0x0); //[DPHY fix]  reg_p0_g_offset_ini_do
	_phy_rtd_part_outl(0x1803bb5c, 27, 27, 0x0); //[DPHY fix]  reg_p0_g_offset_gray_en_do
	_phy_rtd_part_outl(0x1803bb60, 6, 2, 0x0); //[DPHY fix]  reg_p0_g_offset_ini_ee
	_phy_rtd_part_outl(0x1803bb60, 7, 7, 0x0); //[DPHY fix]  reg_p0_g_offset_gray_en_ee
	_phy_rtd_part_outl(0x1803bb60, 16, 12, 0x0); //[DPHY fix]  reg_p0_g_offset_ini_eo
	_phy_rtd_part_outl(0x1803bb60, 17, 17, 0x0); //[DPHY fix]  reg_p0_g_offset_gray_en_eo
	_phy_rtd_part_outl(0x1803bb60, 26, 22, 0x0); //[DPHY fix]  reg_p0_g_offset_ini_eq
	_phy_rtd_part_outl(0x1803bb60, 27, 27, 0x0); //[DPHY fix]  reg_p0_g_offset_gray_en_eq
	_phy_rtd_part_outl(0x1803bb60, 29, 29, 0x0); //[DPHY fix]  reg_p0_g_offset_z0_ok_eq
	_phy_rtd_part_outl(0x1803bb64, 11, 8, 0xf); //[DPHY fix]  reg_p0_r_offset_delay_cnt
	_phy_rtd_part_outl(0x1803bb64, 13, 12, 0x0); //[DPHY fix]  reg_p0_r_offset_stable_cnt
	_phy_rtd_part_outl(0x1803bb64, 19, 14, 0x0b); //[DPHY fix]  reg_p0_r_offset_divisor
	_phy_rtd_part_outl(0x1803bb64, 26, 20, 0x28); //[DPHY fix]  reg_p0_r_offset_timeout
	_phy_rtd_part_outl(0x1803bb68, 0, 0, 0x0); //[]  reg_p0_r_offset_en_ope
	_phy_rtd_part_outl(0x1803bb68, 1, 1, 0x0); //[]  reg_p0_r_offset_pc_ope
	_phy_rtd_part_outl(0x1803bb68, 6, 2, 0x0); //[]  reg_p0_r_offset_ini_ope
	_phy_rtd_part_outl(0x1803bb68, 7, 7, 0x0); //[]  reg_p0_r_offset_gray_en_ope
	_phy_rtd_part_outl(0x1803bb68, 8, 8, 0x0); //[]  reg_p0_r_offset_manual_ope
	_phy_rtd_part_outl(0x1803bb68, 9, 9, 0x0); //[]  reg_p0_r_offset_z0_ok_ope
	_phy_rtd_part_outl(0x1803bb68, 10, 10, 0x0); //[]  reg_p0_r_offset_en_opo
	_phy_rtd_part_outl(0x1803bb68, 11, 11, 0x0); //[]  reg_p0_r_offset_pc_opo
	_phy_rtd_part_outl(0x1803bb68, 16, 12, 0x0); //[]  reg_p0_r_offset_ini_opo
	_phy_rtd_part_outl(0x1803bb68, 17, 17, 0x0); //[]  reg_p0_r_offset_gray_en_opo
	_phy_rtd_part_outl(0x1803bb68, 18, 18, 0x0); //[]  reg_p0_r_offset_manual_opo
	_phy_rtd_part_outl(0x1803bb68, 19, 19, 0x0); //[]  reg_p0_r_offset_z0_ok_opo
	_phy_rtd_part_outl(0x1803bb68, 20, 20, 0x0); //[]  reg_p0_r_offset_en_one
	_phy_rtd_part_outl(0x1803bb68, 21, 21, 0x0); //[]  reg_p0_r_offset_pc_one
	_phy_rtd_part_outl(0x1803bb68, 26, 22, 0x0); //[]  reg_p0_r_offset_ini_one
	_phy_rtd_part_outl(0x1803bb68, 27, 27, 0x0); //[]  reg_p0_r_offset_gray_en_one
	_phy_rtd_part_outl(0x1803bb68, 28, 28, 0x0); //[]  reg_p0_r_offset_manual_one
	_phy_rtd_part_outl(0x1803bb68, 29, 29, 0x0); //[]  reg_p0_r_offset_z0_ok_one
	_phy_rtd_part_outl(0x1803bb6c, 0, 0, 0x0); //[DPHY fix]  reg_p0_r_offset_en_ono
	_phy_rtd_part_outl(0x1803bb6c, 1, 1, 0x0); //[DPHY fix]  reg_p0_r_offset_pc_ono
	_phy_rtd_part_outl(0x1803bb6c, 6, 2, 0x0); //[DPHY fix]  reg_p0_r_offset_ini_ono
	_phy_rtd_part_outl(0x1803bb6c, 7, 7, 0x0); //[DPHY fix]  reg_p0_r_offset_gray_en_ono
	_phy_rtd_part_outl(0x1803bb6c, 8, 8, 0x0); //[DPHY fix]  reg_p0_r_offset_manual_ono
	_phy_rtd_part_outl(0x1803bb6c, 9, 9, 0x0); //[DPHY fix]  reg_p0_r_offset_z0_ok_ono
	_phy_rtd_part_outl(0x1803bb6c, 16, 12, 0x0); //[DPHY fix]  reg_p0_r_offset_ini_de
	_phy_rtd_part_outl(0x1803bb6c, 17, 17, 0x0); //[DPHY fix]  reg_p0_r_offset_gray_en_de
	_phy_rtd_part_outl(0x1803bb6c, 26, 22, 0x0); //[DPHY fix]  reg_p0_r_offset_ini_do
	_phy_rtd_part_outl(0x1803bb6c, 27, 27, 0x0); //[DPHY fix]  reg_p0_r_offset_gray_en_do
	_phy_rtd_part_outl(0x1803bb70, 6, 2, 0x0); //[DPHY fix]  reg_p0_r_offset_ini_ee
	_phy_rtd_part_outl(0x1803bb70, 7, 7, 0x0); //[DPHY fix]  reg_p0_r_offset_gray_en_ee
	_phy_rtd_part_outl(0x1803bb70, 16, 12, 0x0); //[DPHY fix]  reg_p0_r_offset_ini_eo
	_phy_rtd_part_outl(0x1803bb70, 17, 17, 0x0); //[DPHY fix]  reg_p0_r_offset_gray_en_eo
	_phy_rtd_part_outl(0x1803bb70, 26, 22, 0x0); //[DPHY fix]  reg_p0_r_offset_ini_eq
	_phy_rtd_part_outl(0x1803bb70, 27, 27, 0x0); //[DPHY fix]  reg_p0_r_offset_gray_en_eq
	_phy_rtd_part_outl(0x1803bb70, 29, 29, 0x0); //[DPHY fix]  reg_p0_r_offset_z0_ok_eq
	_phy_rtd_part_outl(0x1803bb74, 11, 8, 0xf); //[DPHY fix]  reg_p0_ck_offset_delay_cnt
	_phy_rtd_part_outl(0x1803bb74, 13, 12, 0x0); //[DPHY fix]  reg_p0_ck_offset_stable_cnt
	_phy_rtd_part_outl(0x1803bb74, 19, 14, 0x0b); //[DPHY fix]  reg_p0_ck_offset_divisor
	_phy_rtd_part_outl(0x1803bb74, 26, 20, 0x28); //[DPHY fix]  reg_p0_ck_offset_timeout
	_phy_rtd_part_outl(0x1803bb78, 0, 0, 0x0); //[]  reg_p0_ck_offset_en_ope
	_phy_rtd_part_outl(0x1803bb78, 1, 1, 0x0); //[]  reg_p0_ck_offset_pc_ope
	_phy_rtd_part_outl(0x1803bb78, 6, 2, 0x0); //[]  reg_p0_ck_offset_ini_ope
	_phy_rtd_part_outl(0x1803bb78, 7, 7, 0x0); //[]  reg_p0_ck_offset_gray_en_ope
	_phy_rtd_part_outl(0x1803bb78, 8, 8, 0x0); //[]  reg_p0_ck_offset_manual_ope
	_phy_rtd_part_outl(0x1803bb78, 9, 9, 0x0); //[]  reg_p0_ck_offset_z0_ok_ope
	_phy_rtd_part_outl(0x1803bb78, 10, 10, 0x0); //[]  reg_p0_ck_offset_en_opo
	_phy_rtd_part_outl(0x1803bb78, 11, 11, 0x0); //[]  reg_p0_ck_offset_pc_opo
	_phy_rtd_part_outl(0x1803bb78, 16, 12, 0x0); //[]  reg_p0_ck_offset_ini_opo
	_phy_rtd_part_outl(0x1803bb78, 17, 17, 0x0); //[]  reg_p0_ck_offset_gray_en_opo
	_phy_rtd_part_outl(0x1803bb78, 18, 18, 0x0); //[]  reg_p0_ck_offset_manual_opo
	_phy_rtd_part_outl(0x1803bb78, 19, 19, 0x0); //[]  reg_p0_ck_offset_z0_ok_opo
	_phy_rtd_part_outl(0x1803bb78, 20, 20, 0x0); //[]  reg_p0_ck_offset_en_one
	_phy_rtd_part_outl(0x1803bb78, 21, 21, 0x0); //[]  reg_p0_ck_offset_pc_one
	_phy_rtd_part_outl(0x1803bb78, 26, 22, 0x0); //[]  reg_p0_ck_offset_ini_one
	_phy_rtd_part_outl(0x1803bb78, 27, 27, 0x0); //[]  reg_p0_ck_offset_gray_en_one
	_phy_rtd_part_outl(0x1803bb78, 28, 28, 0x0); //[]  reg_p0_ck_offset_manual_one
	_phy_rtd_part_outl(0x1803bb78, 29, 29, 0x0); //[]  reg_p0_ck_offset_z0_ok_one
	_phy_rtd_part_outl(0x1803bb7c, 0, 0, 0x0); //[DPHY fix]  reg_p0_ck_offset_en_ono
	_phy_rtd_part_outl(0x1803bb7c, 1, 1, 0x0); //[DPHY fix]  reg_p0_ck_offset_pc_ono
	_phy_rtd_part_outl(0x1803bb7c, 6, 2, 0x0); //[DPHY fix]  reg_p0_ck_offset_ini_ono
	_phy_rtd_part_outl(0x1803bb7c, 7, 7, 0x0); //[DPHY fix]  reg_p0_ck_offset_gray_en_ono
	_phy_rtd_part_outl(0x1803bb7c, 8, 8, 0x0); //[DPHY fix]  reg_p0_ck_offset_manual_ono
	_phy_rtd_part_outl(0x1803bb7c, 9, 9, 0x0); //[DPHY fix]  reg_p0_ck_offset_z0_ok_ono
	_phy_rtd_part_outl(0x1803bb7c, 16, 12, 0x0); //[DPHY fix]  reg_p0_ck_offset_ini_de
	_phy_rtd_part_outl(0x1803bb7c, 17, 17, 0x0); //[DPHY fix]  reg_p0_ck_offset_gray_en_de
	_phy_rtd_part_outl(0x1803bb7c, 26, 22, 0x0); //[DPHY fix]  reg_p0_ck_offset_ini_do
	_phy_rtd_part_outl(0x1803bb7c, 27, 27, 0x0); //[DPHY fix]  reg_p0_ck_offset_gray_en_do
	_phy_rtd_part_outl(0x1803bb80, 6, 2, 0x0); //[DPHY fix]  reg_p0_ck_offset_ini_ee
	_phy_rtd_part_outl(0x1803bb80, 7, 7, 0x0); //[DPHY fix]  reg_p0_ck_offset_gray_en_ee
	_phy_rtd_part_outl(0x1803bb80, 16, 12, 0x0); //[DPHY fix]  reg_p0_ck_offset_ini_eo
	_phy_rtd_part_outl(0x1803bb80, 17, 17, 0x0); //[DPHY fix]  reg_p0_ck_offset_gray_en_eo
	_phy_rtd_part_outl(0x1803bb80, 26, 22, 0x0); //[DPHY fix]  reg_p0_ck_offset_ini_eq
	_phy_rtd_part_outl(0x1803bb80, 27, 27, 0x0); //[DPHY fix]  reg_p0_ck_offset_gray_en_eq
	_phy_rtd_part_outl(0x1803bb80, 29, 29, 0x0); //[DPHY fix]  reg_p0_ck_offset_z0_ok_eq
	_phy_rtd_part_outl(0x1803bb88, 2, 2, 0x0); //[DPHY fix]  reg_p0_b_pn_swap_en
	_phy_rtd_part_outl(0x1803bb88, 3, 3, 0x1); //[DPHY fix]  reg_p0_b_dfe_data_en
	_phy_rtd_part_outl(0x1803bb88, 7, 5, 0x3); //[DPHY fix]  reg_p0_b_inbuf_num
	_phy_rtd_part_outl(0x1803bb88, 10, 10, 0x0); //[DPHY fix]  reg_p0_g_pn_swap_en
	_phy_rtd_part_outl(0x1803bb88, 11, 11, 0x1); //[DPHY fix]  reg_p0_g_dfe_data_en
	_phy_rtd_part_outl(0x1803bb88, 15, 13, 0x3); //[DPHY fix]  reg_p0_g_inbuf_num
	_phy_rtd_part_outl(0x1803bb88, 18, 18, 0x0); //[DPHY fix]  reg_p0_r_pn_swap_en
	_phy_rtd_part_outl(0x1803bb88, 19, 19, 0x1); //[DPHY fix]  reg_p0_r_dfe_data_en
	_phy_rtd_part_outl(0x1803bb88, 23, 21, 0x3); //[DPHY fix]  reg_p0_r_fifo_en
	_phy_rtd_part_outl(0x1803bb88, 26, 26, 0x0); //[DPHY fix]  reg_p0_ck_pn_swap_en
	_phy_rtd_part_outl(0x1803bb88, 27, 27, 0x1); //[DPHY fix]  reg_p0_ck_dfe_data_en
	_phy_rtd_part_outl(0x1803bb88, 31, 29, 0x3); //[DPHY fix]  reg_p0_ck_inbuf_num
	_phy_rtd_part_outl(0x1803bb8c, 0, 0, 0x0); //[]  reg_p0_b_001_enable
	_phy_rtd_part_outl(0x1803bb8c, 1, 1, 0x0); //[]  reg_p0_b_101_enable
	_phy_rtd_part_outl(0x1803bb8c, 2, 2, 0x0); //[]  reg_p0_b_en_bec_acc
	_phy_rtd_part_outl(0x1803bb8c, 3, 3, 0x0); //[]  reg_p0_b_en_bec_read
	_phy_rtd_part_outl(0x1803bb8c, 12, 12, 0x0); //[]  reg_p0_g_001_enable
	_phy_rtd_part_outl(0x1803bb8c, 13, 13, 0x0); //[]  reg_p0_g_101_enable
	_phy_rtd_part_outl(0x1803bb8c, 14, 14, 0x0); //[]  reg_p0_g_en_bec_acc
	_phy_rtd_part_outl(0x1803bb8c, 15, 15, 0x0); //[]  reg_p0_g_en_bec_read
	_phy_rtd_part_outl(0x1803bb90, 0, 0, 0x0); //[]  reg_p0_r_001_enable
	_phy_rtd_part_outl(0x1803bb90, 1, 1, 0x0); //[]  reg_p0_r_101_enable
	_phy_rtd_part_outl(0x1803bb90, 2, 2, 0x0); //[]  reg_p0_r_en_bec_acc
	_phy_rtd_part_outl(0x1803bb90, 3, 3, 0x0); //[]  reg_p0_r_en_bec_read
	_phy_rtd_part_outl(0x1803bb90, 12, 12, 0x0); //[]  reg_p0_ck_001_enable
	_phy_rtd_part_outl(0x1803bb90, 13, 13, 0x0); //[]  reg_p0_ck_101_enable
	_phy_rtd_part_outl(0x1803bb90, 14, 14, 0x0); //[]  reg_p0_ck_en_bec_acc
	_phy_rtd_part_outl(0x1803bb90, 15, 15, 0x0); //[]  reg_p0_ck_en_bec_read
	_phy_rtd_part_outl(0x1803bbb8, 0, 0, 0x0); //[DPHY fix]  reg_p0_b_cp2adp_en
	_phy_rtd_part_outl(0x1803bbb8, 1, 1, 0x0); //[DPHY fix]  reg_p0_b_stable_time_mode
	_phy_rtd_part_outl(0x1803bbb8, 2, 2, 0x0); //[DPHY fix]  reg_p0_b_bypass_k_band_mode
	_phy_rtd_part_outl(0x1803bbb8, 18, 18, 0x0); //[DPHY fix]  reg_p0_b_calib_late
	_phy_rtd_part_outl(0x1803bbb8, 19, 19, 0x0); //[DPHY fix]  reg_p0_b_calib_manual
	_phy_rtd_part_outl(0x1803bbb8, 22, 20, 0x2); //[DPHY fix]  reg_p0_b_calib_time
	_phy_rtd_part_outl(0x1803bbb8, 27, 23, 0x0c); //[DPHY fix]  reg_p0_b_adp_time
	_phy_rtd_part_outl(0x1803bbb8, 28, 28, 0x0); //[DPHY fix]  reg_p0_b_adap_eq_off
	_phy_rtd_part_outl(0x1803bbb8, 29, 29, 0x0); //[DPHY fix]  reg_p0_b_cp_en_manual
	_phy_rtd_part_outl(0x1803bbb8, 30, 30, 0x0); //[DPHY fix]  reg_p0_b_adp_en_manual
	_phy_rtd_part_outl(0x1803bbb8, 31, 31, 0x1); //[DPHY fix]  reg_p0_b_auto_mode
	_phy_rtd_part_outl(0x1803bbbc, 31, 28, 0x8); //[DPHY fix] reg_p0_b_cp2adp_time
	_phy_rtd_part_outl(0x1803bbc0, 1, 1, 0x1); //[] reg_p0_b_calib_reset_sel
	_phy_rtd_part_outl(0x1803bbc0, 3, 2, 0x0); //[] reg_p0_b_vc_sel
	_phy_rtd_part_outl(0x1803bbc0, 5, 4, 0x0); //[] reg_p0_b_cdr_c
	_phy_rtd_part_outl(0x1803bbc0, 11, 6, 0x00); //[] reg_p0_b_cdr_r
	_phy_rtd_part_outl(0x1803bbc0, 31, 12, 0xa9878); //[] reg_p0_b_cdr_cp
	_phy_rtd_part_outl(0x1803bbc4, 2, 0, 0x0); //[DPHY fix] reg_p0_b_init_time
	_phy_rtd_part_outl(0x1803bbc4, 7, 3, 0x04); //[DPHY fix] reg_p0_b_cp_time
	_phy_rtd_part_outl(0x1803bbc8, 6, 2, 0x01); //[DPHY fix]  reg_p0_b_timer_6
	_phy_rtd_part_outl(0x1803bbc8, 7, 7, 0x0); //[DPHY fix]  reg_p0_b_timer_5
	_phy_rtd_part_outl(0x1803bbc8, 11, 8, 0x7); //[DPHY fix]  reg_p0_b_vco_fine_init
	_phy_rtd_part_outl(0x1803bbc8, 15, 12, 0x7); //[DPHY fix]  reg_p0_b_vco_coarse_init
	_phy_rtd_part_outl(0x1803bbc8, 16, 16, 0x0); //[DPHY fix]  reg_p0_b_bypass_coarse_k_mode
	_phy_rtd_part_outl(0x1803bbc8, 18, 17, 0x0); //[DPHY fix]  reg_p0_b_vco_fine_time_sel
	_phy_rtd_part_outl(0x1803bbc8, 19, 19, 0x0); //[DPHY fix]  reg_p0_b_xtal_24m_en
	_phy_rtd_part_outl(0x1803bbc8, 23, 20, 0x0); //[DPHY fix]  reg_p0_b_vco_fine_manual
	_phy_rtd_part_outl(0x1803bbc8, 27, 24, 0x8); //[DPHY fix]  reg_p0_b_vco_coarse_manual
	_phy_rtd_part_outl(0x1803bbc8, 28, 28, 0x0); //[DPHY fix]  reg_p0_b_coarse_calib_manual
	_phy_rtd_part_outl(0x1803bbc8, 29, 29, 0x0); //[DPHY fix]  reg_p0_b_acdr_ckfld_en
	_phy_rtd_part_outl(0x1803bbc8, 30, 30, 0x0); //[DPHY fix]  reg_p0_b_pll_to_acdr_manual_en
	_phy_rtd_part_outl(0x1803bbc8, 31, 31, 0x0); //[DPHY fix]  reg_p0_b_pll_to_acdr_rdy_manual
	_phy_rtd_part_outl(0x1803bbcc, 4, 0, 0x01); //[]  reg_p0_b_timer_4
	_phy_rtd_part_outl(0x1803bbcc, 7, 5, 0x0); //[]  reg_p0_b_vc_chg_time
	_phy_rtd_part_outl(0x1803bbcc, 16, 16, 0x1); //[]  reg_p0_b_old_mode
	_phy_rtd_part_outl(0x1803bbcc, 21, 17, 0x10); //[]  reg_p0_b_slope_band
	_phy_rtd_part_outl(0x1803bbcc, 22, 22, 0x0); //[]  reg_p0_b_slope_manual
	_phy_rtd_part_outl(0x1803bbcc, 23, 23, 0x0); //[]  reg_p0_b_rxidle_bypass
	_phy_rtd_part_outl(0x1803bbcc, 24, 24, 0x0); //[]  reg_p0_b_pfd_bypass
	_phy_rtd_part_outl(0x1803bbcc, 29, 25, 0x00); //[]  reg_p0_b_pfd_time
	_phy_rtd_part_outl(0x1803bbcc, 30, 30, 0x0); //[]  reg_p0_b_pfd_en_manual
	_phy_rtd_part_outl(0x1803bbcc, 31, 31, 0x0); //[]  reg_p0_b_start_en_manual
	_phy_rtd_part_outl(0x1803bbd0, 9, 5, 0x08); //[]  reg_p0_b_cp_time_2
	_phy_rtd_part_outl(0x1803bbd8, 0, 0, 0x0); //[DPHY fix]  reg_p0_g_cp2adp_en
	_phy_rtd_part_outl(0x1803bbd8, 1, 1, 0x0); //[DPHY fix]  reg_p0_g_stable_time_mode
	_phy_rtd_part_outl(0x1803bbd8, 2, 2, 0x0); //[DPHY fix]  reg_p0_g_bypass_k_band_mode
	_phy_rtd_part_outl(0x1803bbd8, 18, 18, 0x0); //[DPHY fix]  reg_p0_g_calib_late
	_phy_rtd_part_outl(0x1803bbd8, 19, 19, 0x0); //[DPHY fix]  reg_p0_g_calib_manual
	_phy_rtd_part_outl(0x1803bbd8, 22, 20, 0x2); //[DPHY fix]  reg_p0_g_calib_time
	_phy_rtd_part_outl(0x1803bbd8, 27, 23, 0x0c); //[DPHY fix]  reg_p0_g_adp_time
	_phy_rtd_part_outl(0x1803bbd8, 28, 28, 0x0); //[DPHY fix]  reg_p0_g_adap_eq_off
	_phy_rtd_part_outl(0x1803bbd8, 29, 29, 0x0); //[DPHY fix]  reg_p0_g_cp_en_manual
	_phy_rtd_part_outl(0x1803bbd8, 30, 30, 0x0); //[DPHY fix]  reg_p0_g_adp_en_manual
	_phy_rtd_part_outl(0x1803bbd8, 31, 31, 0x1); //[DPHY fix]  reg_p0_g_auto_mode
	_phy_rtd_part_outl(0x1803bbdc, 31, 28, 0x8); //[DPHY fix] reg_p0_g_cp2adp_time
	_phy_rtd_part_outl(0x1803bbe0, 1, 1, 0x1); //[] reg_p0_g_calib_reset_sel
	_phy_rtd_part_outl(0x1803bbe0, 3, 2, 0x0); //[] reg_p0_g_vc_sel
	_phy_rtd_part_outl(0x1803bbe0, 5, 4, 0x0); //[] reg_p0_g_cdr_c
	_phy_rtd_part_outl(0x1803bbe0, 11, 6, 0x00); //[] reg_p0_g_cdr_r
	_phy_rtd_part_outl(0x1803bbe0, 31, 12, 0xa9878); //[] reg_p0_g_cdr_cp
	_phy_rtd_part_outl(0x1803bbe4, 2, 0, 0x0); //[DPHY fix] reg_p0_g_init_time
	_phy_rtd_part_outl(0x1803bbe4, 7, 3, 0x04); //[DPHY fix] reg_p0_g_cp_time
	_phy_rtd_part_outl(0x1803bbe8, 6, 2, 0x01); //[DPHY fix]  reg_p0_g_timer_6
	_phy_rtd_part_outl(0x1803bbe8, 7, 7, 0x0); //[DPHY fix]  reg_p0_g_timer_5
	_phy_rtd_part_outl(0x1803bbe8, 11, 8, 0x7); //[DPHY fix]  reg_p0_g_vco_fine_init
	_phy_rtd_part_outl(0x1803bbe8, 15, 12, 0x7); //[DPHY fix]  reg_p0_g_vco_coarse_init
	_phy_rtd_part_outl(0x1803bbe8, 16, 16, 0x0); //[DPHY fix]  reg_p0_g_bypass_coarse_k_mode
	_phy_rtd_part_outl(0x1803bbe8, 18, 17, 0x0); //[DPHY fix]  reg_p0_g_vco_fine_time_sel
	_phy_rtd_part_outl(0x1803bbe8, 19, 19, 0x0); //[DPHY fix]  reg_p0_g_xtal_24m_en
	_phy_rtd_part_outl(0x1803bbe8, 23, 20, 0x0); //[DPHY fix]  reg_p0_g_vco_fine_manual
	_phy_rtd_part_outl(0x1803bbe8, 27, 24, 0x8); //[DPHY fix]  reg_p0_g_vco_coarse_manual
	_phy_rtd_part_outl(0x1803bbe8, 28, 28, 0x0); //[DPHY fix]  reg_p0_g_coarse_calib_manual
	_phy_rtd_part_outl(0x1803bbe8, 29, 29, 0x0); //[DPHY fix]  reg_p0_g_acdr_ckfld_en
	_phy_rtd_part_outl(0x1803bbe8, 30, 30, 0x0); //[DPHY fix]  reg_p0_g_pll_to_acdr_manual_en
	_phy_rtd_part_outl(0x1803bbe8, 31, 31, 0x0); //[DPHY fix]  reg_p0_g_pll_to_acdr_rdy_manual
	_phy_rtd_part_outl(0x1803bbec, 4, 0, 0x01); //[]  reg_p0_g_timer_4
	_phy_rtd_part_outl(0x1803bbec, 7, 5, 0x0); //[]  reg_p0_g_vc_chg_time
	_phy_rtd_part_outl(0x1803bbec, 16, 16, 0x1); //[]  reg_p0_g_old_mode
	_phy_rtd_part_outl(0x1803bbec, 21, 17, 0x10); //[]  reg_p0_g_slope_band
	_phy_rtd_part_outl(0x1803bbec, 22, 22, 0x0); //[]  reg_p0_g_slope_manual
	_phy_rtd_part_outl(0x1803bbec, 23, 23, 0x0); //[]  reg_p0_g_rxidle_bypass
	_phy_rtd_part_outl(0x1803bbec, 24, 24, 0x0); //[]  reg_p0_g_pfd_bypass
	_phy_rtd_part_outl(0x1803bbec, 29, 25, 0x00); //[]  reg_p0_g_pfd_time
	_phy_rtd_part_outl(0x1803bbec, 30, 30, 0x0); //[]  reg_p0_g_pfd_en_manual
	_phy_rtd_part_outl(0x1803bbec, 31, 31, 0x0); //[]  reg_p0_g_start_en_manual
	_phy_rtd_part_outl(0x1803bbf0, 9, 5, 0x08); //[]  reg_p0_g_cp_time_2
	_phy_rtd_part_outl(0x1803bbf4, 0, 0, 0x0); //[DPHY fix]  reg_p0_r_cp2adp_en
	_phy_rtd_part_outl(0x1803bbf4, 1, 1, 0x0); //[DPHY fix]  reg_p0_r_stable_time_mode
	_phy_rtd_part_outl(0x1803bbf4, 2, 2, 0x0); //[DPHY fix]  reg_p0_r_bypass_k_band_mode
	_phy_rtd_part_outl(0x1803bbf4, 18, 18, 0x0); //[DPHY fix]  reg_p0_r_calib_late
	_phy_rtd_part_outl(0x1803bbf4, 19, 19, 0x0); //[DPHY fix]  reg_p0_r_calib_manual
	_phy_rtd_part_outl(0x1803bbf4, 22, 20, 0x2); //[DPHY fix]  reg_p0_r_calib_time
	_phy_rtd_part_outl(0x1803bbf4, 27, 23, 0x0c); //[DPHY fix]  reg_p0_r_adp_time
	_phy_rtd_part_outl(0x1803bbf4, 28, 28, 0x0); //[DPHY fix]  reg_p0_r_adap_eq_off
	_phy_rtd_part_outl(0x1803bbf4, 29, 29, 0x0); //[DPHY fix]  reg_p0_r_cp_en_manual
	_phy_rtd_part_outl(0x1803bbf4, 30, 30, 0x0); //[DPHY fix]  reg_p0_r_adp_en_manual
	_phy_rtd_part_outl(0x1803bbf4, 31, 31, 0x1); //[DPHY fix]  reg_p0_r_auto_mode
	_phy_rtd_part_outl(0x1803bbf8, 31, 28, 0x8); //[DPHY fix] reg_p0_r_cp2adp_time
	_phy_rtd_part_outl(0x1803bbfc, 1, 1, 0x1); //[] reg_p0_r_calib_reset_sel
	_phy_rtd_part_outl(0x1803bbfc, 3, 2, 0x0); //[] reg_p0_r_vc_sel
	_phy_rtd_part_outl(0x1803bbfc, 5, 4, 0x0); //[] reg_p0_r_cdr_c
	_phy_rtd_part_outl(0x1803bbfc, 11, 6, 0x00); //[] reg_p0_r_cdr_r
	_phy_rtd_part_outl(0x1803bbfc, 31, 12, 0xa9878); //[] reg_p0_r_cdr_cp
	_phy_rtd_part_outl(0x1803bc00, 2, 0, 0x0); //[DPHY fix] reg_p0_r_init_time
	_phy_rtd_part_outl(0x1803bc00, 7, 3, 0x04); //[DPHY fix] reg_p0_r_cp_time
	_phy_rtd_part_outl(0x1803bc04, 6, 2, 0x01); //[DPHY fix]  reg_p0_r_timer_6
	_phy_rtd_part_outl(0x1803bc04, 7, 7, 0x0); //[DPHY fix]  reg_p0_r_timer_5
	_phy_rtd_part_outl(0x1803bc04, 11, 8, 0x7); //[DPHY fix]  reg_p0_r_vco_fine_init
	_phy_rtd_part_outl(0x1803bc04, 15, 12, 0x7); //[DPHY fix]  reg_p0_r_vco_coarse_init
	_phy_rtd_part_outl(0x1803bc04, 16, 16, 0x0); //[DPHY fix]  reg_p0_r_bypass_coarse_k_mode
	_phy_rtd_part_outl(0x1803bc04, 18, 17, 0x0); //[DPHY fix]  reg_p0_r_vco_fine_time_sel
	_phy_rtd_part_outl(0x1803bc04, 19, 19, 0x0); //[DPHY fix]  reg_p0_r_xtal_24m_en
	_phy_rtd_part_outl(0x1803bc04, 23, 20, 0x0); //[DPHY fix]  reg_p0_r_vco_fine_manual
	_phy_rtd_part_outl(0x1803bc04, 27, 24, 0x8); //[DPHY fix]  reg_p0_r_vco_coarse_manual
	_phy_rtd_part_outl(0x1803bc04, 28, 28, 0x0); //[DPHY fix]  reg_p0_r_coarse_calib_manual
	_phy_rtd_part_outl(0x1803bc04, 29, 29, 0x0); //[DPHY fix]  reg_p0_r_acdr_ckfld_en
	_phy_rtd_part_outl(0x1803bc04, 30, 30, 0x0); //[DPHY fix]  reg_p0_r_pll_to_acdr_manual_en
	_phy_rtd_part_outl(0x1803bc04, 31, 31, 0x0); //[DPHY fix]  reg_p0_r_pll_to_acdr_rdy_manual
	_phy_rtd_part_outl(0x1803bc08, 4, 0, 0x01); //[]  reg_p0_r_timer_4
	_phy_rtd_part_outl(0x1803bc08, 7, 5, 0x0); //[]  reg_p0_r_vc_chg_time
	_phy_rtd_part_outl(0x1803bc08, 16, 16, 0x1); //[]  reg_p0_r_old_mode
	_phy_rtd_part_outl(0x1803bc08, 21, 17, 0x10); //[]  reg_p0_r_slope_band
	_phy_rtd_part_outl(0x1803bc08, 22, 22, 0x0); //[]  reg_p0_r_slope_manual
	_phy_rtd_part_outl(0x1803bc08, 23, 23, 0x0); //[]  reg_p0_r_rxidle_bypass
	_phy_rtd_part_outl(0x1803bc08, 24, 24, 0x0); //[]  reg_p0_r_pfd_bypass
	_phy_rtd_part_outl(0x1803bc08, 29, 25, 0x00); //[]  reg_p0_r_pfd_time
	_phy_rtd_part_outl(0x1803bc08, 30, 30, 0x0); //[]  reg_p0_r_pfd_en_manual
	_phy_rtd_part_outl(0x1803bc08, 31, 31, 0x0); //[]  reg_p0_r_start_en_manual
	_phy_rtd_part_outl(0x1803bc0c, 9, 5, 0x08); //[]  reg_p0_r_cp_time_2
	_phy_rtd_part_outl(0x1803bc10, 0, 0, 0x0); //[DPHY fix]  reg_p0_ck_cp2adp_en
	_phy_rtd_part_outl(0x1803bc10, 1, 1, 0x0); //[DPHY fix]  reg_p0_ck_stable_time_mode
	_phy_rtd_part_outl(0x1803bc10, 2, 2, 0x0); //[DPHY fix]  reg_p0_ck_bypass_k_band_mode
	_phy_rtd_part_outl(0x1803bc10, 18, 18, 0x0); //[DPHY fix]  reg_p0_ck_calib_late
	_phy_rtd_part_outl(0x1803bc10, 19, 19, 0x0); //[DPHY fix]  reg_p0_ck_calib_manual
	_phy_rtd_part_outl(0x1803bc10, 22, 20, 0x2); //[DPHY fix]  reg_p0_ck_calib_time
	_phy_rtd_part_outl(0x1803bc10, 27, 23, 0x0c); //[DPHY fix]  reg_p0_ck_adp_time
	_phy_rtd_part_outl(0x1803bc10, 28, 28, 0x0); //[DPHY fix]  reg_p0_ck_adap_eq_off
	_phy_rtd_part_outl(0x1803bc10, 29, 29, 0x0); //[DPHY fix]  reg_p0_ck_cp_en_manual
	_phy_rtd_part_outl(0x1803bc10, 30, 30, 0x0); //[DPHY fix]  reg_p0_ck_adp_en_manual
	_phy_rtd_part_outl(0x1803bc10, 31, 31, 0x1); //[DPHY fix]  reg_p0_ck_auto_mode
	_phy_rtd_part_outl(0x1803bc14, 31, 28, 0x8); //[DPHY fix] reg_p0_ck_cp2adp_time
	_phy_rtd_part_outl(0x1803bc18, 1, 1, 0x1); //[] reg_p0_ck_calib_reset_sel
	_phy_rtd_part_outl(0x1803bc18, 3, 2, 0x0); //[] reg_p0_ck_vc_sel
	_phy_rtd_part_outl(0x1803bc18, 5, 4, 0x0); //[] reg_p0_ck_cdr_c
	_phy_rtd_part_outl(0x1803bc18, 11, 6, 0x00); //[] reg_p0_ck_cdr_r
	_phy_rtd_part_outl(0x1803bc18, 31, 12, 0xa9878); //[] reg_p0_ck_cdr_cp
	_phy_rtd_part_outl(0x1803bc1c, 2, 0, 0x0); //[DPHY fix] reg_p0_ck_init_time
	_phy_rtd_part_outl(0x1803bc1c, 7, 3, 0x04); //[DPHY fix] reg_p0_ck_cp_time
	_phy_rtd_part_outl(0x1803bc20, 6, 2, 0x01); //[DPHY fix]  reg_p0_ck_timer_6
	_phy_rtd_part_outl(0x1803bc20, 7, 7, 0x0); //[DPHY fix]  reg_p0_ck_timer_5
	_phy_rtd_part_outl(0x1803bc20, 11, 8, 0x7); //[DPHY fix]  reg_p0_ck_vco_fine_init
	_phy_rtd_part_outl(0x1803bc20, 15, 12, 0x7); //[DPHY fix]  reg_p0_ck_vco_coarse_init
	_phy_rtd_part_outl(0x1803bc20, 16, 16, 0x0); //[DPHY fix]  reg_p0_ck_bypass_coarse_k_mode
	_phy_rtd_part_outl(0x1803bc20, 18, 17, 0x0); //[DPHY fix]  reg_p0_ck_vco_fine_time_sel
	_phy_rtd_part_outl(0x1803bc20, 19, 19, 0x0); //[DPHY fix]  reg_p0_ck_xtal_24m_en
	_phy_rtd_part_outl(0x1803bc20, 23, 20, 0x0); //[DPHY fix]  reg_p0_ck_vco_fine_manual
	_phy_rtd_part_outl(0x1803bc20, 27, 24, 0x8); //[DPHY fix]  reg_p0_ck_vco_coarse_manual
	_phy_rtd_part_outl(0x1803bc20, 28, 28, 0x0); //[DPHY fix]  reg_p0_ck_coarse_calib_manual
	_phy_rtd_part_outl(0x1803bc20, 29, 29, 0x0); //[DPHY fix]  reg_p0_ck_acdr_ckfld_en
	_phy_rtd_part_outl(0x1803bc20, 30, 30, 0x0); //[DPHY fix]  reg_p0_ck_pll_to_acdr_manual_en
	_phy_rtd_part_outl(0x1803bc20, 31, 31, 0x0); //[DPHY fix]  reg_p0_ck_pll_to_acdr_rdy_manual
	_phy_rtd_part_outl(0x1803bc24, 4, 0, 0x01); //[]  reg_p0_ck_timer_4
	_phy_rtd_part_outl(0x1803bc24, 7, 5, 0x0); //[]  reg_p0_ck_vc_chg_time
	_phy_rtd_part_outl(0x1803bc24, 16, 16, 0x1); //[]  reg_p0_ck_old_mode
	_phy_rtd_part_outl(0x1803bc24, 21, 17, 0x10); //[]  reg_p0_ck_slope_band
	_phy_rtd_part_outl(0x1803bc24, 22, 22, 0x0); //[]  reg_p0_ck_slope_manual
	_phy_rtd_part_outl(0x1803bc24, 23, 23, 0x0); //[]  reg_p0_ck_rxidle_bypass
	_phy_rtd_part_outl(0x1803bc24, 24, 24, 0x0); //[]  reg_p0_ck_pfd_bypass
	_phy_rtd_part_outl(0x1803bc24, 29, 25, 0x00); //[]  reg_p0_ck_pfd_time
	_phy_rtd_part_outl(0x1803bc24, 30, 30, 0x0); //[]  reg_p0_ck_pfd_en_manual
	_phy_rtd_part_outl(0x1803bc24, 31, 31, 0x0); //[]  reg_p0_ck_start_en_manual
	_phy_rtd_part_outl(0x1803bc28, 9, 5, 0x08); //[]  reg_p0_ck_cp_time_2
	//MOD
	_phy_rtd_part_outl(0x1803bc2c, 0, 0, 0x1); //[]  reg_p0_ck_md_rstb
	_phy_rtd_part_outl(0x1803bc2c, 1, 1, 0x1); //[]  reg_p0_ck_md_auto
	_phy_rtd_part_outl(0x1803bc2c, 3, 2, 0x0); //[]  reg_p0_ck_md_sel
	_phy_rtd_part_outl(0x1803bc2c, 7, 4, 0x0); //[]  reg_p0_ck_md_adj
	_phy_rtd_part_outl(0x1803bc2c, 13, 8, 0x03); //[]  reg_p0_ck_md_threshold
	_phy_rtd_part_outl(0x1803bc2c, 20, 14, 0x00); //[]  reg_p0_ck_md_debounce
	_phy_rtd_part_outl(0x1803bc2c, 25, 21, 0x00); //[]  reg_p0_ck_error_limit
	_phy_rtd_part_outl(0x1803bc2c, 29, 26, 0x0); //[]  reg_p0_ck_md_reserved
	_phy_rtd_part_outl(0x1803bc30, 2, 0, 0x0); //[]  reg_p0_mod_sel
	//PLLCDR
	_phy_rtd_part_outl(0x1803bc34, 0, 0, 0x0); //[]  reg_p0_b_pllcdr_manual_enable
	_phy_rtd_part_outl(0x1803bc34, 1, 1, 0x0); //[]  reg_p0_g_pllcdr_manual_enable
	_phy_rtd_part_outl(0x1803bc34, 2, 2, 0x0); //[]  reg_p0_r_pllcdr_manual_enable
	_phy_rtd_part_outl(0x1803bc34, 3, 3, 0x0); //[]  reg_p0_ck_pllcdr_manual_enable
	_phy_rtd_part_outl(0x1803bc34, 4, 4, 0x0); //[]  reg_p0_b_pllcdr_vcoband_manual_enable
	_phy_rtd_part_outl(0x1803bc34, 5, 5, 0x0); //[]  reg_p0_g_pllcdr_vcoband_manual_enable
	_phy_rtd_part_outl(0x1803bc34, 6, 6, 0x0); //[]  reg_p0_r_pllcdr_vcoband_manual_enable
	_phy_rtd_part_outl(0x1803bc34, 7, 7, 0x0); //[]  reg_p0_ck_pllcdr_vcoband_manual_enable
	_phy_rtd_part_outl(0x1803bc34, 19, 16, 0x0); //[]  reg_p0_b_acdr_icp_sel_manual
	_phy_rtd_part_outl(0x1803bc34, 23, 20, 0x0); //[]  reg_p0_g_acdr_icp_sel_manual
	_phy_rtd_part_outl(0x1803bc34, 27, 24, 0x0); //[]  reg_p0_r_acdr_icp_sel_manual
	_phy_rtd_part_outl(0x1803bc34, 31, 28, 0x3); //[]  reg_p0_ck_acdr_icp_sel_manual
	_phy_rtd_part_outl(0x1803bc38, 3, 0, 0x0); //[]  reg_p0_b_acdr_vco_coarse_i_manual
	_phy_rtd_part_outl(0x1803bc38, 7, 4, 0x0); //[]  reg_p0_g_acdr_vco_coarse_i_manual
	_phy_rtd_part_outl(0x1803bc38, 11, 8, 0x0); //[]  reg_p0_r_acdr_vco_coarse_i_manual
	_phy_rtd_part_outl(0x1803bc38, 15, 12, 0x0); //[]  reg_p0_ck_acdr_vco_coarse_i_manual
	_phy_rtd_part_outl(0x1803bc38, 19, 16, 0x0); //[]  reg_p0_b_acdr_vco_fine_i_manual
	_phy_rtd_part_outl(0x1803bc38, 23, 20, 0x0); //[]  reg_p0_g_acdr_vco_fine_i_manual
	_phy_rtd_part_outl(0x1803bc38, 27, 24, 0x0); //[]  reg_p0_r_acdr_vco_fine_i_manual
	_phy_rtd_part_outl(0x1803bc38, 31, 28, 0x0); //[]  reg_p0_ck_acdr_vco_fine_i_manual
	_phy_rtd_part_outl(0x1803bc6c, 0, 0, 0x0); //[DPHY fix]  reg_p0_acdr_en
	_phy_rtd_part_outl(0x1803bc6c, 2, 2, 0x0); //[DPHY fix]  reg_p0_pow_on_manual
	_phy_rtd_part_outl(0x1803bc6c, 3, 3, 0x0); //[DPHY fix]  reg_p0_pow_on_manual_aphy_en
	_phy_rtd_part_outl(0x1803bc6c, 4, 4, 0x0); //[DPHY fix]  reg_p0_pow_on_manual_b_en
	_phy_rtd_part_outl(0x1803bc6c, 5, 5, 0x0); //[DPHY fix]  reg_p0_pow_on_manual_g_en
	_phy_rtd_part_outl(0x1803bc6c, 6, 6, 0x0); //[DPHY fix]  reg_p0_pow_on_manual_r_en
	_phy_rtd_part_outl(0x1803bc6c, 7, 7, 0x0); //[DPHY fix]  reg_p0_pow_on_manual_ck_en
	_phy_rtd_part_outl(0x1803bc6c, 8, 8, 0x0); //[DPHY fix]  reg_p0_pow_save_bypass_b
	_phy_rtd_part_outl(0x1803bc6c, 9, 9, 0x0); //[DPHY fix]  reg_p0_pow_save_bypass_g
	_phy_rtd_part_outl(0x1803bc6c, 10, 10, 0x0); //[DPHY fix]  reg_p0_pow_save_bypass_r
	_phy_rtd_part_outl(0x1803bc6c, 11, 11, 0x0); //[DPHY fix]  reg_p0_pow_save_bypass_ck
	_phy_rtd_part_outl(0x1803bc6c, 12, 12, 0x0); //[DPHY fix]  reg_p0_pow_save_xtal_24m_enable
	_phy_rtd_part_outl(0x1803bc6c, 14, 13, 0x3); //[DPHY fix]  reg_p0_pow_save_rst_dly_sel
	_phy_rtd_part_outl(0x1803bc70, 17, 16, 0x0); //[DPHY fix]  reg_tap1_mask
	_phy_rtd_part_outl(0x1803bc7c, 31, 31, 0x0); //[DPHY fix]  reg_port_out_sel
	_phy_rtd_part_outl(0x1803bc80, 7, 0, 0x00); //[]  reg_p0_dig_reserved_3
	_phy_rtd_part_outl(0x1803bc80, 15, 8, 0x00); //[]  reg_p0_dig_reserved_2
	_phy_rtd_part_outl(0x1803bc80, 23, 16, 0x00); //[]  reg_p0_dig_reserved_1
	_phy_rtd_part_outl(0x1803bc80, 31, 24, 0x00); //[]  reg_p0_dig_reserved_0
}

void DPHY_Para_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	// DPHY para start
	_phy_rtd_part_outl(0x1803bb04, 5, 0, DPHY_Para_tmds_1[value_param].b_kp_2); //[]  reg_p0_b_kp
	_phy_rtd_part_outl(0x1803bb04, 11, 6, DPHY_Para_tmds_1[value_param].g_kp_3); //[]  reg_p0_g_kp
	_phy_rtd_part_outl(0x1803bb04, 17, 12, DPHY_Para_tmds_1[value_param].r_kp_4); //[]  reg_p0_r_kp
	_phy_rtd_part_outl(0x1803bb04, 23, 18, DPHY_Para_tmds_1[value_param].ck_kp_5); //[]  reg_p0_ck_kp
	_phy_rtd_part_outl(0x1803bb08, 5, 0, DPHY_Para_tmds_1[value_param].b_bbw_kp_6); //[]  reg_p0_b_bbw_kp
	_phy_rtd_part_outl(0x1803bb08, 11, 6, DPHY_Para_tmds_1[value_param].g_bbw_kp_7); //[]  reg_p0_g_bbw_kp
	_phy_rtd_part_outl(0x1803bb08, 17, 12, DPHY_Para_tmds_1[value_param].r_bbw_kp_8); //[]  reg_p0_r_bbw_kp
	_phy_rtd_part_outl(0x1803bb08, 23, 18, DPHY_Para_tmds_1[value_param].ck_bbw_kp_9); //[]  reg_p0_ck_bbw_kp
	_phy_rtd_part_outl(0x1803bb0c, 2, 0, DPHY_Para_tmds_1[value_param].b_ki_10); //[]  reg_p0_b_ki
	_phy_rtd_part_outl(0x1803bb0c, 5, 3, DPHY_Para_tmds_1[value_param].g_ki_11); //[]  reg_p0_g_ki
	_phy_rtd_part_outl(0x1803bb0c, 8, 6, DPHY_Para_tmds_1[value_param].r_ki_12); //[]  reg_p0_r_ki
	_phy_rtd_part_outl(0x1803bb0c, 11, 9, DPHY_Para_tmds_1[value_param].ck_ki_13); //[]  reg_p0_ck_ki
	_phy_rtd_part_outl(0x1803bb0c, 18, 16, 0x0); //[]  reg_p0_b_bbw_ki
	_phy_rtd_part_outl(0x1803bb0c, 21, 19, 0x0); //[]  reg_p0_g_bbw_ki
	_phy_rtd_part_outl(0x1803bb0c, 24, 22, 0x0); //[]  reg_p0_r_bbw_ki
	_phy_rtd_part_outl(0x1803bb0c, 27, 25, 0x0); //[]  reg_p0_ck_bbw_ki
	_phy_rtd_part_outl(0x1803bb0c, 12, 12, 0x0); //[]  reg_p0_b_st_mode
	_phy_rtd_part_outl(0x1803bb0c, 13, 13, 0x0); //[]  reg_p0_g_st_mode
	_phy_rtd_part_outl(0x1803bb0c, 14, 14, 0x0); //[]  reg_p0_r_st_mode
	_phy_rtd_part_outl(0x1803bb0c, 15, 15, 0x0); //[]  reg_p0_ck_st_mode
	_phy_rtd_part_outl(0x1803bb0c, 28, 28, 0x0); //[]  reg_p0_b_bbw_st_mode
	_phy_rtd_part_outl(0x1803bb0c, 29, 29, 0x0); //[]  reg_p0_g_bbw_st_mode
	_phy_rtd_part_outl(0x1803bb0c, 30, 30, 0x0); //[]  reg_p0_r_bbw_st_mode
	_phy_rtd_part_outl(0x1803bb0c, 31, 31, 0x0); //[]  reg_p0_ck_bbw_st_mode
	_phy_rtd_part_outl(0x1803bb00, 17, 16, DPHY_Para_tmds_1[value_param].b_rate_sel_26); //[DPHY para]  reg_p0_b_rate_sel
	_phy_rtd_part_outl(0x1803bb00, 19, 18, DPHY_Para_tmds_1[value_param].g_rate_sel_27); //[DPHY para]  reg_p0_g_rate_sel
	_phy_rtd_part_outl(0x1803bb00, 21, 20, DPHY_Para_tmds_1[value_param].r_rate_sel_28); //[DPHY para]  reg_p0_r_rate_sel
	_phy_rtd_part_outl(0x1803bb88, 1, 1, DPHY_Para_tmds_1[value_param].b_clk_div2_en_29); //[DPHY para]  reg_p0_b_clk_div2_en
	_phy_rtd_part_outl(0x1803bb88, 9, 9, DPHY_Para_tmds_1[value_param].g_clk_div2_en_30); //[DPHY para]  reg_p0_g_clk_div2_en
	_phy_rtd_part_outl(0x1803bb88, 17, 17, DPHY_Para_tmds_1[value_param].r_clk_div2_en_31); //[DPHY para]  reg_p0_r_clk_div2_en
	_phy_rtd_part_outl(0x1803bb8c, 25, 24, 0x0); //[DPHY para]  reg_p0_b_format_sel
	_phy_rtd_part_outl(0x1803bb8c, 27, 26, 0x0); //[DPHY para]  reg_p0_g_format_sel
	_phy_rtd_part_outl(0x1803bb8c, 29, 28, 0x0); //[DPHY para]  reg_p0_r_format_sel
	_phy_rtd_part_outl(0x1803bb8c, 31, 30, 0x0); //[DPHY para]  reg_p0_ck_format_sel
	_phy_rtd_part_outl(0x1803bc3c, 7, 0, DPHY_Para_tmds_1[value_param].b_acdr_pll_config_1_36); //[]  reg_p0_b_acdr_pll_config_1
	_phy_rtd_part_outl(0x1803bc3c, 15, 8, DPHY_Para_tmds_1[value_param].b_acdr_pll_config_2_37); //[]  reg_p0_b_acdr_pll_config_2
	_phy_rtd_part_outl(0x1803bc3c, 23, 16, DPHY_Para_tmds_1[value_param].b_acdr_pll_config_3_38); //[]  reg_p0_b_acdr_pll_config_3
	_phy_rtd_part_outl(0x1803bc3c, 31, 24, 0x00); //[]  reg_p0_b_acdr_pll_config_4
	_phy_rtd_part_outl(0x1803bc40, 7, 0, DPHY_Para_tmds_1[value_param].b_acdr_cdr_config_1_40); //[]  reg_p0_b_acdr_cdr_config_1
	_phy_rtd_part_outl(0x1803bc40, 15, 8, DPHY_Para_tmds_1[value_param].b_acdr_cdr_config_2_41); //[]  reg_p0_b_acdr_cdr_config_2
	_phy_rtd_part_outl(0x1803bc40, 23, 16, DPHY_Para_tmds_1[value_param].b_acdr_cdr_config_3_42); //[]  reg_p0_b_acdr_cdr_config_3
	_phy_rtd_part_outl(0x1803bc40, 31, 24, 0x00); //[]  reg_p0_b_acdr_cdr_config_4
	_phy_rtd_part_outl(0x1803bc44, 7, 0, DPHY_Para_tmds_1[value_param].b_acdr_manual_config_1_44); //[]  reg_p0_b_acdr_manual_config_1
	_phy_rtd_part_outl(0x1803bc44, 15, 8, DPHY_Para_tmds_1[value_param].b_acdr_manual_config_2_45); //[]  reg_p0_b_acdr_manual_config_2
	_phy_rtd_part_outl(0x1803bc44, 23, 16, DPHY_Para_tmds_1[value_param].b_acdr_manual_config_3_46); //[]  reg_p0_b_acdr_manual_config_3
	_phy_rtd_part_outl(0x1803bc44, 31, 24, 0x00); //[]  reg_p0_b_acdr_manual_config_4
	_phy_rtd_part_outl(0x1803bc48, 7, 0, DPHY_Para_tmds_1[value_param].g_acdr_pll_config_1_48); //[]  reg_p0_g_acdr_pll_config_1
	_phy_rtd_part_outl(0x1803bc48, 15, 8, DPHY_Para_tmds_1[value_param].g_acdr_pll_config_2_49); //[]  reg_p0_g_acdr_pll_config_2
	_phy_rtd_part_outl(0x1803bc48, 23, 16, DPHY_Para_tmds_1[value_param].g_acdr_pll_config_3_50); //[]  reg_p0_g_acdr_pll_config_3
	_phy_rtd_part_outl(0x1803bc48, 31, 24, 0x00); //[]  reg_p0_g_acdr_pll_config_4
	_phy_rtd_part_outl(0x1803bc4c, 7, 0, DPHY_Para_tmds_1[value_param].g_acdr_cdr_config_1_52); //[]  reg_p0_g_acdr_cdr_config_1
	_phy_rtd_part_outl(0x1803bc4c, 15, 8, DPHY_Para_tmds_1[value_param].g_acdr_cdr_config_2_53); //[]  reg_p0_g_acdr_cdr_config_2
	_phy_rtd_part_outl(0x1803bc4c, 23, 16, DPHY_Para_tmds_1[value_param].g_acdr_cdr_config_3_54); //[]  reg_p0_g_acdr_cdr_config_3
	_phy_rtd_part_outl(0x1803bc4c, 31, 24, 0x00); //[]  reg_p0_g_acdr_cdr_config_4
	_phy_rtd_part_outl(0x1803bc50, 7, 0, DPHY_Para_tmds_1[value_param].g_acdr_manual_config_1_56); //[]  reg_p0_g_acdr_manual_config_1
	_phy_rtd_part_outl(0x1803bc50, 15, 8, DPHY_Para_tmds_1[value_param].g_acdr_manual_config_2_57); //[]  reg_p0_g_acdr_manual_config_2
	_phy_rtd_part_outl(0x1803bc50, 23, 16, DPHY_Para_tmds_1[value_param].g_acdr_manual_config_3_58); //[]  reg_p0_g_acdr_manual_config_3
	_phy_rtd_part_outl(0x1803bc50, 31, 24, 0x00); //[]  reg_p0_g_acdr_manual_config_4
	_phy_rtd_part_outl(0x1803bc54, 7, 0, DPHY_Para_tmds_1[value_param].r_acdr_pll_config_1_60); //[]  reg_p0_r_acdr_pll_config_1
	_phy_rtd_part_outl(0x1803bc54, 15, 8, DPHY_Para_tmds_1[value_param].r_acdr_pll_config_2_61); //[]  reg_p0_r_acdr_pll_config_2
	_phy_rtd_part_outl(0x1803bc54, 23, 16, DPHY_Para_tmds_1[value_param].r_acdr_pll_config_3_62); //[]  reg_p0_r_acdr_pll_config_3
	_phy_rtd_part_outl(0x1803bc54, 31, 24, 0x00); //[]  reg_p0_r_acdr_pll_config_4
	_phy_rtd_part_outl(0x1803bc58, 7, 0, DPHY_Para_tmds_1[value_param].r_acdr_cdr_config_1_64); //[]  reg_p0_r_acdr_cdr_config_1
	_phy_rtd_part_outl(0x1803bc58, 15, 8, DPHY_Para_tmds_1[value_param].r_acdr_cdr_config_2_65); //[]  reg_p0_r_acdr_cdr_config_2
	_phy_rtd_part_outl(0x1803bc58, 23, 16, DPHY_Para_tmds_1[value_param].r_acdr_cdr_config_3_66); //[]  reg_p0_r_acdr_cdr_config_3
	_phy_rtd_part_outl(0x1803bc58, 31, 24, 0x00); //[]  reg_p0_r_acdr_cdr_config_4
	_phy_rtd_part_outl(0x1803bc5c, 7, 0, DPHY_Para_tmds_1[value_param].r_acdr_manual_config_1_68); //[]  reg_p0_r_acdr_manual_config_1
	_phy_rtd_part_outl(0x1803bc5c, 15, 8, DPHY_Para_tmds_1[value_param].r_acdr_manual_config_2_69); //[]  reg_p0_r_acdr_manual_config_2
	_phy_rtd_part_outl(0x1803bc5c, 23, 16, DPHY_Para_tmds_1[value_param].r_acdr_manual_config_3_70); //[]  reg_p0_r_acdr_manual_config_3
	_phy_rtd_part_outl(0x1803bc5c, 31, 24, 0x00); //[]  reg_p0_r_acdr_manual_config_4
	_phy_rtd_part_outl(0x1803bc60, 7, 0, DPHY_Para_tmds_1[value_param].ck_acdr_pll_config_1_72); //[]  reg_p0_ck_acdr_pll_config_1
	_phy_rtd_part_outl(0x1803bc60, 15, 8, DPHY_Para_tmds_1[value_param].ck_acdr_pll_config_2_73); //[]  reg_p0_ck_acdr_pll_config_2
	_phy_rtd_part_outl(0x1803bc60, 23, 16, DPHY_Para_tmds_1[value_param].ck_acdr_pll_config_3_74); //[]  reg_p0_ck_acdr_pll_config_3
	_phy_rtd_part_outl(0x1803bc60, 31, 24, 0x00); //[]  reg_p0_ck_acdr_pll_config_4
	_phy_rtd_part_outl(0x1803bc64, 7, 0, DPHY_Para_tmds_1[value_param].ck_acdr_cdr_config_1_76); //[]  reg_p0_ck_acdr_cdr_config_1
	_phy_rtd_part_outl(0x1803bc64, 15, 8, DPHY_Para_tmds_1[value_param].ck_acdr_cdr_config_2_77); //[]  reg_p0_ck_acdr_cdr_config_2
	_phy_rtd_part_outl(0x1803bc64, 23, 16, DPHY_Para_tmds_1[value_param].ck_acdr_cdr_config_3_78); //[]  reg_p0_ck_acdr_cdr_config_3
	_phy_rtd_part_outl(0x1803bc64, 31, 24, 0x00); //[]  reg_p0_ck_acdr_cdr_config_4
	_phy_rtd_part_outl(0x1803bc68, 7, 0, DPHY_Para_tmds_1[value_param].ck_acdr_manual_config_1_80); //[]  reg_p0_ck_acdr_manual_config_1
	_phy_rtd_part_outl(0x1803bc68, 15, 8, DPHY_Para_tmds_1[value_param].ck_acdr_manual_config_2_81); //[]  reg_p0_ck_acdr_manual_config_2
	_phy_rtd_part_outl(0x1803bc68, 23, 16, DPHY_Para_tmds_1[value_param].ck_acdr_manual_config_3_82); //[]  reg_p0_ck_acdr_manual_config_3
	_phy_rtd_part_outl(0x1803bc68, 31, 24, 0x00); //[]  reg_p0_ck_acdr_manual_config_4
}

void APHY_Fix_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	//APHY fix start
	//_phy_rtd_part_outl(0x1803ba04, 4, 0, 0x08); //[]  REG_Z0_ADJR_0 Set by OTP Trim value
	_phy_rtd_part_outl(0x1803ba04, 5, 5, 0x01); //[]  REG_P0_Z0_Z0POW_FIX
	_phy_rtd_part_outl(0x1803ba04, 6, 6, 0x0); //[]  Z0_P_OFF
	_phy_rtd_part_outl(0x1803ba04, 7, 7, 0x0); //[]  Z0_N_OFF
	_phy_rtd_part_outl(0x1803ba04, 9, 9, 0x0); //[]  REG_Z0_FT_PN_SHORT_EN
	_phy_rtd_part_outl(0x1803ba10, 0, 0, 0x0); //[]  TOP_IN_1, FAST_SW_SEL
	_phy_rtd_part_outl(0x1803ba10, 1, 1, 0x0); //[]  TOP_IN_1, DIV_EN
	_phy_rtd_part_outl(0x1803ba10, 4, 2, 0x0); //[]  TOP_IN_1, DIV_SEL
	_phy_rtd_part_outl(0x1803ba10, 5, 5, 0x0); //[]  TOP_IN_1, V_STROBE_EN
	_phy_rtd_part_outl(0x1803ba10, 6, 6, 0x1); //[]  TOP_IN_1, CK_TX to B/G/R EN
	_phy_rtd_part_outl(0x1803ba10, 7, 7, 0x0); //[]  TOP_IN_1, dummy
	_phy_rtd_part_outl(0x1803ba10, 11, 8, 0x0); //[] TOP_IN_2, test signal sel
	_phy_rtd_part_outl(0x1803ba10, 13, 12, APHY_Fix_tmds_1[value_param].REG_TOP_IN_2_14); //[] TOP_IN_2, test mode sel
	_phy_rtd_part_outl(0x1803ba10, 14, 14, 0x1); //[] TOP_IN_2, CK_TX EN
	_phy_rtd_part_outl(0x1803ba10, 15, 15, APHY_Fix_tmds_1[value_param].REG_TOP_IN_2_16); //[] TOP_IN_2, CK_MD SEL
	_phy_rtd_part_outl(0x1803ba10, 23, 16, 0x1); //[]  TOP_IN_3, dummy
	_phy_rtd_part_outl(0x1803ba10, 31, 24, 0x0); //[]  TOP_IN_4, dummy
	_phy_rtd_part_outl(0x1803ba14, 7, 0, 0x10); //[]  REG_TOP_IN_5 dummy
	_phy_rtd_part_outl(0x1803ba18, 0, 0, 0x1); //[APHY fix]  REG_HDMIRX_BIAS_EN bandgap reference power
	_phy_rtd_part_outl(0x1803ba18, 10, 8, 0x0); //[APHY fix]  REG_BG_RBGLOOP2 dummy
	_phy_rtd_part_outl(0x1803ba18, 13, 11, 0x0); //[APHY fix]  REG_BG_RBG dummy
	_phy_rtd_part_outl(0x1803ba18, 15, 14, 0x0); //[APHY fix]  REG_BG_RBG2 dummy
	_phy_rtd_part_outl(0x1803ba18, 16, 16, 0x0); //[APHY fix]  REG_BG_POW dummy
	_phy_rtd_part_outl(0x1803ba18, 17, 17, 0x0); //[APHY fix]  REG_BG_ENVBGUP
	_phy_rtd_part_outl(0x1803ba20, 7, 0, 0x00); //[]  dummy
	_phy_rtd_part_outl(0x1803ba20, 15, 8, 0x00); //[]  dummy
	_phy_rtd_part_outl(0x1803ba20, 23, 16, 0x00); //[]  dummy
	_phy_rtd_part_outl(0x1803ba20, 31, 24, 0x00); //[]  dummy
	_phy_rtd_part_outl(0x1803ba24, 7, 0, 0x00); //[]  dummy
	_phy_rtd_part_outl(0x1803ba24, 15, 8, 0x00); //[]  dummy
	_phy_rtd_part_outl(0x1803ba24, 23, 16, 0x00); //[]  dummy
	_phy_rtd_part_outl(0x1803ba24, 31, 24, 0x00); //[]  dummy
	_phy_rtd_part_outl(0x1803ba30, 1, 1, 0x0); //[APHY fix]  CK-Lane INPOFF_SINGLE_EN
	_phy_rtd_part_outl(0x1803ba30, 2, 2, 0x0); //[APHY fix]  CK-Lane INNOFF_SINGLE_EN
	_phy_rtd_part_outl(0x1803ba30, 3, 3, 0x1); //[APHY fix]  CK-Lane POW_AC_COUPLE
	_phy_rtd_part_outl(0x1803ba30, 5, 4, 0x0); //[APHY fix]  CK-Lane RXVCM_SEL[1:0]
	_phy_rtd_part_outl(0x1803ba34, 21, 21, 0x1); //[APHY fix]   	CK-Lane RS_CAL_EN
	_phy_rtd_part_outl(0x1803ba34, 25, 25, 0x1); //[APHY fix]   	CK-Lane POW_DATALANE_BIAS
	_phy_rtd_part_outl(0x1803ba34, 26, 26, 0x0); //[APHY fix]   	CK-Lane REG_FORCE_STARTUP(Const-GM)
	_phy_rtd_part_outl(0x1803ba34, 27, 27, 0x0); //[APHY fix]   	CK-Lane REG_POWB_STARTUP
	_phy_rtd_part_outl(0x1803ba38, 8, 8, APHY_Fix_tmds_1[value_param].CK_10_42); //[APHY fix]  		CK-Lane POW_DFE
	_phy_rtd_part_outl(0x1803ba38, 14, 12, 0x6); //[APHY fix] 		CK-Lane DFE_SUMAMP_ ISEL
	_phy_rtd_part_outl(0x1803ba38, 15, 15, 0x0); //[APHY fix] 		CK-Lane DFE_SUMAMP_DCGAIN_MAX
	_phy_rtd_part_outl(0x1803ba38, 19, 18, 0x0); //[APHY fix] 		CK-Lane Dummy
	_phy_rtd_part_outl(0x1803ba38, 20, 20, 0x0); //[APHY fix] 		CK-Lane DFE CKI_DELAY_EN
	_phy_rtd_part_outl(0x1803ba38, 21, 21, 0x0); //[APHY fix] 		CK-Lane DFE CKIB_DELAY_EN
	_phy_rtd_part_outl(0x1803ba38, 22, 22, 0x0); //[APHY fix] 		CK-Lane DFE CKQ_DELAY_EN
	_phy_rtd_part_outl(0x1803ba38, 23, 23, 0x0); //[APHY fix] 		CK-Lane DFE CKQB_DELAY_EN
	_phy_rtd_part_outl(0x1803ba38, 24, 24, 0x0); //[APHY fix] 		CK-Lane EN_EYE_MNT
	_phy_rtd_part_outl(0x1803ba38, 25, 25, 0x0); //[APHY fix] 		CK-Lane DEMUX input clock phase select:(for eye mnt)
	_phy_rtd_part_outl(0x1803ba38, 26, 26, 0x1); //[APHY fix] 		CK-Lane VTH+OFFSET select. 0:VTH, 1:VTH+OFFSET
	_phy_rtd_part_outl(0x1803ba38, 27, 27, 0x0); //[APHY fix] 		CK-Lane DA_EG_VOS_PULLLOW(koffset disable)
	_phy_rtd_part_outl(0x1803ba38, 31, 29, 0x0); //[APHY fix] 			CK-Lane dummy
	_phy_rtd_part_outl(0x1803ba3c, 2, 0, 0x0); //[APHY fix]  	CK-Lane DFE_TAP_DELAY
	_phy_rtd_part_outl(0x1803ba3c, 8, 8, APHY_Fix_tmds_1[value_param].CK_14_56); //[APHY fix]   	CK-Lane rstb of dtata-lane test div8
	_phy_rtd_part_outl(0x1803ba3c, 16, 16, 0x0); //[APHY fix]  		CK-Lane TRANSITION CNT enable.
	_phy_rtd_part_outl(0x1803ba3c, 17, 17, 0x0); //[APHY fix]  		CK-Lane QCG1 injection-locked EN
	_phy_rtd_part_outl(0x1803ba3c, 18, 18, 0x0); //[APHY fix]  		CK-Lane QCG2_EN
	_phy_rtd_part_outl(0x1803ba3c, 19, 19, 0x0); //[APHY fix]  		CK-Lane QCG2 injection-locked EN
	_phy_rtd_part_outl(0x1803ba3c, 20, 20, 0x0); //[APHY fix]  		CK-Lane QCG FLD divider(/8) rstb
	_phy_rtd_part_outl(0x1803ba3c, 21, 21, 0x0); //[APHY fix]  		CK-Lane QCG FLD SEL
	_phy_rtd_part_outl(0x1803ba3c, 22, 22, 0x1); //[APHY fix]  		CK-Lane CK_FLD SEL.
	_phy_rtd_part_outl(0x1803ba3c, 23, 23, 0x1); //[APHY fix]  		CK-Lane CK-lane clk to B/G/R EN.
	_phy_rtd_part_outl(0x1803ba40, 8, 8, 0x1); //[APHY fix]  CK-Lane PI_EN
	_phy_rtd_part_outl(0x1803ba40, 12, 12, 0x0); //[APHY fix]  CK-Lane QCG1 Cload contrl SEL
	_phy_rtd_part_outl(0x1803ba40, 13, 13, 0x0); //[APHY fix]  CK-Lane QCG2 Cload contrl SEL
	_phy_rtd_part_outl(0x1803ba40, 15, 15, 0x0); //[APHY fix]  CK-Lane BIAS_PI_CUR_SEL
	_phy_rtd_part_outl(0x1803ba40, 21, 21, 0x0); //[APHY fix]  CK-Lane PI_EYE_EN
	_phy_rtd_part_outl(0x1803ba40, 23, 23, 0x0); //[APHY fix]  CK-Lane KI SEL
	_phy_rtd_part_outl(0x1803ba40, 25, 25, 0x0); //[APHY fix]  CK-Lane FKP_RSTB_SEL
	_phy_rtd_part_outl(0x1803ba40, 27, 26, 0x0); //[APHY fix]  CK-Lane KI_CK_SEL
	_phy_rtd_part_outl(0x1803ba40, 29, 29, 0x0); //[APHY fix]  CK-Lane ST_CODE SEL
	_phy_rtd_part_outl(0x1803ba40, 30, 30, 0x0); //[APHY fix]  CK-Lane QCG Ckin SEL.
	_phy_rtd_part_outl(0x1803ba44, 3, 2, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_DIV_FLD[1:0]
	_phy_rtd_part_outl(0x1803ba44, 5, 4, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_DIV_REF[1:0]
	_phy_rtd_part_outl(0x1803ba44, 7, 6, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_DIV_PLL[1:0]
	_phy_rtd_part_outl(0x1803ba44, 9, 8, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_DIV_TRAINING[1:0]
	_phy_rtd_part_outl(0x1803ba44, 12, 12, 0x0); //[APHY fix]  CK-Lane dummy
	_phy_rtd_part_outl(0x1803ba44, 13, 13, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_FLD_0CKFB_1CKREF
	_phy_rtd_part_outl(0x1803ba44, 15, 14, 0x0); //[APHY fix]  CK-Lane no use
	_phy_rtd_part_outl(0x1803ba44, 18, 18, 0x1); //[APHY fix]  CK-Lane ACDR_RSTB_DIV_PLL ... removed
	_phy_rtd_part_outl(0x1803ba44, 24, 24, 0x0); //[APHY fix]  CK-Lane ACDR_EN_UPDN_PULSE_FILTER
	_phy_rtd_part_outl(0x1803ba44, 25, 25, 0x1); //[APHY fix]  CK-Lane ACDR_RSTB_UPDN ... dummy
	_phy_rtd_part_outl(0x1803ba44, 27, 26, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_UPDN_WIDTH[1:0]
	_phy_rtd_part_outl(0x1803ba44, 28, 28, 0x1); //[APHY fix]  CK-Lane ACDR_POW_LPF_IDAC
	_phy_rtd_part_outl(0x1803ba44, 31, 29, 0x3); //[APHY fix]  CK-Lane ACDR_SEL_LPF_IDAC[2:0]
	_phy_rtd_part_outl(0x1803ba48, 0, 0, APHY_Fix_tmds_1[value_param].ACDR_CK_5_88); //[APHY fix]  CK-Lane ACDR_POW_CP
	_phy_rtd_part_outl(0x1803ba48, 2, 2, 0x0); //[APHY fix]  CK-Lane ACDR_POW_IDN_BBPD
	_phy_rtd_part_outl(0x1803ba48, 3, 3, 0x0); //[APHY fix]  CK-Lane no use
	_phy_rtd_part_outl(0x1803ba48, 6, 4, 0x7); //[APHY fix]  CK-Lane ACDR_SEL_TIE_IDN_BBPD
	_phy_rtd_part_outl(0x1803ba48, 7, 7, 0x1); //[APHY fix]  CK-Lane ACDR_POW_IBIAS_IDN_HV
	_phy_rtd_part_outl(0x1803ba48, 8, 8, 0x1); //[APHY fix]  CK-Lane ACDR_POW_VCO
	_phy_rtd_part_outl(0x1803ba48, 9, 9, 0x1); //[APHY fix]  CK-Lane ACDR_POW_VCO_VDAC
	_phy_rtd_part_outl(0x1803ba48, 11, 10, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_V15_VDAC
	_phy_rtd_part_outl(0x1803ba48, 14, 14, 0x0); //[APHY fix]  CK-Lane no use
	_phy_rtd_part_outl(0x1803ba48, 15, 15, 0x0); //[APHY fix]  CK-Lane no use
	_phy_rtd_part_outl(0x1803ba48, 16, 16, 0x0); //[APHY fix]  CK-Lane ACDR_POW_TEST_MODE
	_phy_rtd_part_outl(0x1803ba48, 18, 17, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_TEST_MODE
	_phy_rtd_part_outl(0x1803ba48, 23, 19, 0x00); //[APHY fix]  CK-Lane no use
	_phy_rtd_part_outl(0x1803ba48, 31, 31, 0x0); //[APHY fix]  CK-Lane no use
	_phy_rtd_part_outl(0x1803ba4c, 15, 13, 0x1); //[APHY fix]  CK-Lane no use
	_phy_rtd_part_outl(0x1803ba4c, 19, 16, 0x0); //[APHY fix]  CK-Lane LE1_ISEL_IN_2 -> dummy
	_phy_rtd_part_outl(0x1803ba4c, 31, 25, 0x44); //[APHY fix]  CK-Lane no use for LEQ
	_phy_rtd_part_outl(0x1803ba50, 1, 1, 0x0); //[APHY fix]  		B-Lane INPOFF_SINGLE_EN
	_phy_rtd_part_outl(0x1803ba50, 2, 2, 0x0); //[APHY fix]  		B-Lane INNOFF_SINGLE_EN
	_phy_rtd_part_outl(0x1803ba50, 3, 3, 0x1); //[APHY fix]  		B-Lane POW_AC_COUPLE
	_phy_rtd_part_outl(0x1803ba50, 5, 4, 0x1); //[APHY fix]  		B-Lane RXVCM_SEL[1:0]
	_phy_rtd_part_outl(0x1803ba54, 21, 21, 0x1); //[APHY fix]   		B-Lane RS_CAL_EN
	_phy_rtd_part_outl(0x1803ba54, 25, 25, 0x1); //[APHY fix]   		B-Lane POW_DATALANE_BIAS
	_phy_rtd_part_outl(0x1803ba54, 26, 26, 0x0); //[APHY fix]   		B-Lane REG_FORCE_STARTUP(Const-GM)
	_phy_rtd_part_outl(0x1803ba54, 27, 27, 0x0); //[APHY fix]   		B-Lane REG_POWB_STARTUP
	_phy_rtd_part_outl(0x1803ba58, 8, 8, 0x1); //[APHY fix]  		B-Lane POW_DFE
	_phy_rtd_part_outl(0x1803ba58, 14, 12, 0x6); //[APHY fix] 		B-Lane DFE_SUMAMP_ ISEL
	_phy_rtd_part_outl(0x1803ba58, 15, 15, 0x0); //[APHY fix] 		B-Lane DFE_SUMAMP_DCGAIN_MAX
	_phy_rtd_part_outl(0x1803ba58, 19, 18, 0x0); //[APHY fix] 		B-Lane Dummy
	_phy_rtd_part_outl(0x1803ba58, 20, 20, 0x0); //[APHY fix] 		B-Lane DFE CKI_DELAY_EN
	_phy_rtd_part_outl(0x1803ba58, 21, 21, 0x0); //[APHY fix] 		B-Lane DFE CKIB_DELAY_EN
	_phy_rtd_part_outl(0x1803ba58, 22, 22, 0x0); //[APHY fix] 		B-Lane DFE CKQ_DELAY_EN
	_phy_rtd_part_outl(0x1803ba58, 23, 23, 0x0); //[APHY fix] 		B-Lane DFE CKQB_DELAY_EN
	_phy_rtd_part_outl(0x1803ba58, 24, 24, 0x0); //[APHY fix] 		B-Lane EN_EYE_MNT
	_phy_rtd_part_outl(0x1803ba58, 25, 25, 0x0); //[APHY fix] 		B-Lane DEMUX input clock phase select:(for eye mnt)
	_phy_rtd_part_outl(0x1803ba58, 26, 26, 0x1); //[APHY fix] 		B-Lane VTH+OFFSET select. 0:VTH, 1:VTH+OFFSET
	_phy_rtd_part_outl(0x1803ba58, 27, 27, 0x0); //[APHY fix] 		B-Lane DA_EG_VOS_PULLLOW(koffset disable)
	_phy_rtd_part_outl(0x1803ba58, 31, 29, 0x0); //[APHY fix] 			B-Lane dummy
	_phy_rtd_part_outl(0x1803ba5c, 2, 0, 0x0); //[APHY fix]  	B-Lane DFE_TAP_DELAY
	_phy_rtd_part_outl(0x1803ba5c, 8, 8, APHY_Fix_tmds_1[value_param].B_14_127); //[APHY fix]   	B-Lane rstb of dtata-lane test div8
	_phy_rtd_part_outl(0x1803ba5c, 16, 16, 0x0); //[APHY fix]  		B-Lane TRANSITION CNT enable.
	_phy_rtd_part_outl(0x1803ba5c, 17, 17, 0x1); //[APHY fix]  		B-Lane QCG1 injection-locked EN
	_phy_rtd_part_outl(0x1803ba5c, 18, 18, 0x0); //[APHY fix]  		B-Lane QCG2_EN
	_phy_rtd_part_outl(0x1803ba5c, 19, 19, 0x0); //[APHY fix]  		B-Lane QCG2 injection-locked EN
	_phy_rtd_part_outl(0x1803ba5c, 20, 20, 0x0); //[APHY fix]  		B-Lane QCG FLD divider(/8) rstb
	_phy_rtd_part_outl(0x1803ba5c, 21, 21, 0x0); //[APHY fix]  		B-Lane QCG FLD SEL
	_phy_rtd_part_outl(0x1803ba5c, 22, 22, 0x1); //[APHY fix]  		B-Lane CK_FLD SEL.
	_phy_rtd_part_outl(0x1803ba5c, 23, 23, 0x0); //[APHY fix]  		B-Lane CK-lane clk to B/G/R EN.
	_phy_rtd_part_outl(0x1803ba60, 8, 8, 0x1); //[APHY fix]  B-Lane PI_EN
	_phy_rtd_part_outl(0x1803ba60, 12, 12, 0x0); //[APHY fix]  B-Lane QCG1 Cload contrl SEL
	_phy_rtd_part_outl(0x1803ba60, 13, 13, 0x0); //[APHY fix]  B-Lane QCG2 Cload contrl SEL
	_phy_rtd_part_outl(0x1803ba60, 15, 15, 0x0); //[APHY fix]  B-Lane BIAS_PI_CUR_SEL
	_phy_rtd_part_outl(0x1803ba60, 21, 21, 0x0); //[APHY fix]  B-Lane PI_EYE_EN
	_phy_rtd_part_outl(0x1803ba60, 23, 23, 0x0); //[APHY fix]  B-Lane KI SEL
	_phy_rtd_part_outl(0x1803ba60, 25, 25, 0x0); //[APHY fix]  B-Lane FKP_RSTB_SEL
	_phy_rtd_part_outl(0x1803ba60, 27, 26, 0x0); //[APHY fix]  B-Lane KI_CK_SEL
	_phy_rtd_part_outl(0x1803ba60, 29, 29, 0x0); //[APHY fix]  B-Lane ST_CODE SEL
	_phy_rtd_part_outl(0x1803ba60, 30, 30, 0x0); //[APHY fix]  B-Lane QCG Ckin SEL.
	_phy_rtd_part_outl(0x1803ba64, 3, 2, 0x0); //[APHY fix]  B-Lane ACDR_SEL_DIV_FLD[1:0]
	_phy_rtd_part_outl(0x1803ba64, 5, 4, 0x0); //[APHY fix]  B-Lane ACDR_SEL_DIV_REF[1:0]
	_phy_rtd_part_outl(0x1803ba64, 7, 6, 0x0); //[APHY fix]  B-Lane ACDR_SEL_DIV_PLL[1:0]
	_phy_rtd_part_outl(0x1803ba64, 9, 8, 0x0); //[APHY fix]  B-Lane ACDR_SEL_DIV_TRAINING[1:0]
	_phy_rtd_part_outl(0x1803ba64, 12, 12, 0x0); //[APHY fix]  B-Lane dummy
	_phy_rtd_part_outl(0x1803ba64, 13, 13, 0x0); //[APHY fix]  B-Lane ACDR_SEL_FLD_0CKFB_1CKREF
	_phy_rtd_part_outl(0x1803ba64, 15, 14, 0x0); //[APHY fix]  B-Lane no use
	_phy_rtd_part_outl(0x1803ba64, 18, 18, 0x0); //[APHY fix]  B-Lane ACDR_RSTB_DIV_PLL ... removed
	_phy_rtd_part_outl(0x1803ba64, 24, 24, 0x1); //[APHY fix]  B-Lane ACDR_EN_UPDN_PULSE_FILTER
	_phy_rtd_part_outl(0x1803ba64, 25, 25, 0x1); //[APHY fix]  B-Lane CMU_SEL_CP
	_phy_rtd_part_outl(0x1803ba64, 27, 26, 0x0); //[APHY fix]  B-Lane ACDR_SEL_UPDN_WIDTH[1:0]
	_phy_rtd_part_outl(0x1803ba64, 28, 28, 0x0); //[APHY fix]  B-Lane ACDR_POW_LPF_IDAC
	_phy_rtd_part_outl(0x1803ba64, 31, 29, 0x0); //[APHY fix]  B-Lane ACDR_SEL_LPF_IDAC[2:0]
	_phy_rtd_part_outl(0x1803ba68, 0, 0, 0x1); //[APHY fix]  B-Lane ACDR_POW_CP
	_phy_rtd_part_outl(0x1803ba68, 2, 2, 0x0); //[APHY fix]  B-Lane ACDR_POW_IDN_BBPD
	_phy_rtd_part_outl(0x1803ba68, 3, 3, 0x0); //[APHY fix]  B-Lane no use
	_phy_rtd_part_outl(0x1803ba68, 6, 4, 0x2); //[APHY fix]  B-Lane ACDR_SEL_TIE_IDN_BBPD
	_phy_rtd_part_outl(0x1803ba68, 7, 7, 0x1); //[APHY fix]  B-Lane ACDR_POW_IBIAS_IDN_HV
	_phy_rtd_part_outl(0x1803ba68, 8, 8, 0x1); //[APHY fix]  B-Lane ACDR_POW_VCO
	_phy_rtd_part_outl(0x1803ba68, 9, 9, 0x1); //[APHY fix]  B-Lane ACDR_POW_VCO_VDAC
	_phy_rtd_part_outl(0x1803ba68, 11, 10, 0x0); //[APHY fix]  B-Lane ACDR_SEL_V15_VDAC
	_phy_rtd_part_outl(0x1803ba68, 14, 14, 0x0); //[APHY fix]  B-Lane no use
	_phy_rtd_part_outl(0x1803ba68, 15, 15, 0x0); //[APHY fix]  B-Lane no use
	_phy_rtd_part_outl(0x1803ba68, 16, 16, 0x0); //[APHY fix]  B-Lane ACDR_POW_TEST_MODE
	_phy_rtd_part_outl(0x1803ba68, 18, 17, 0x0); //[APHY fix]  B-Lane ACDR_SEL_TEST_MODE
	_phy_rtd_part_outl(0x1803ba68, 23, 19, 0x00); //[APHY fix]  B-Lane no use
	_phy_rtd_part_outl(0x1803ba68, 31, 31, 0x1); //[APHY fix]  B-Lane no use
	_phy_rtd_part_outl(0x1803ba6c, 13, 13, APHY_Fix_tmds_1[value_param].ACDR_B_10_173); //[APHY fix]  B-Lane ckref sel. 0:xtal, 1:tx or train clk
	_phy_rtd_part_outl(0x1803ba6c, 14, 14, 0x1); //[APHY fix]  B-Lane ckref sel. 0:train_clk, 1:tx_clk
	_phy_rtd_part_outl(0x1803ba6c, 15, 15, 0x0); //[APHY fix]  B-Lane ckbk sel. 0:M-DIV, 1:PRESCALER
	_phy_rtd_part_outl(0x1803ba6c, 19, 16, 0x0); //[APHY fix]  B-Lane LE1_ISEL_IN_2 -> dummy
	_phy_rtd_part_outl(0x1803ba6c, 31, 25, 0x00); //[APHY fix]  B-Lane no use for LEQ
	_phy_rtd_part_outl(0x1803ba70, 1, 1, 0x0); //[APHY fix]  		G-Lane INPOFF_SINGLE_EN
	_phy_rtd_part_outl(0x1803ba70, 2, 2, 0x0); //[APHY fix]  		G-Lane INNOFF_SINGLE_EN
	_phy_rtd_part_outl(0x1803ba70, 3, 3, 0x1); //[APHY fix]  		G-Lane POW_AC_COUPLE
	_phy_rtd_part_outl(0x1803ba70, 5, 4, 0x1); //[APHY fix]  		G-Lane RXVCM_SEL[1:0]
	_phy_rtd_part_outl(0x1803ba74, 21, 21, 0x1); //[APHY fix]   		G-Lane RS_CAL_EN
	_phy_rtd_part_outl(0x1803ba74, 25, 25, 0x1); //[APHY fix]   		G-Lane POW_DATALANE_BIAS
	_phy_rtd_part_outl(0x1803ba74, 26, 26, 0x0); //[APHY fix]   		G-Lane REG_FORCE_STARTUP(Const-GM)
	_phy_rtd_part_outl(0x1803ba74, 27, 27, 0x0); //[APHY fix]   		G-Lane REG_POWB_STARTUP
	_phy_rtd_part_outl(0x1803ba78, 8, 8, 0x1); //[APHY fix]  		G-Lane POW_DFE
	_phy_rtd_part_outl(0x1803ba78, 14, 12, 0x6); //[APHY fix] 		G-Lane DFE_SUMAMP_ ISEL
	_phy_rtd_part_outl(0x1803ba78, 15, 15, 0x0); //[APHY fix] 		G-Lane DFE_SUMAMP_DCGAIN_MAX
	_phy_rtd_part_outl(0x1803ba78, 19, 18, 0x0); //[APHY fix] 		G-Lane Dummy
	_phy_rtd_part_outl(0x1803ba78, 20, 20, 0x0); //[APHY fix] 		G-Lane DFE CKI_DELAY_EN
	_phy_rtd_part_outl(0x1803ba78, 21, 21, 0x0); //[APHY fix] 		G-Lane DFE CKIB_DELAY_EN
	_phy_rtd_part_outl(0x1803ba78, 22, 22, 0x0); //[APHY fix] 		G-Lane DFE CKQ_DELAY_EN
	_phy_rtd_part_outl(0x1803ba78, 23, 23, 0x0); //[APHY fix] 		G-Lane DFE CKQB_DELAY_EN
	_phy_rtd_part_outl(0x1803ba78, 24, 24, 0x0); //[APHY fix] 		G-Lane EN_EYE_MNT
	_phy_rtd_part_outl(0x1803ba78, 25, 25, 0x0); //[APHY fix] 		G-Lane DEMUX input clock phase select:(for eye mnt)
	_phy_rtd_part_outl(0x1803ba78, 26, 26, 0x1); //[APHY fix] G-Lane VTH+OFFSET select. 0:VTH, 1:VTH+OFFSET
	_phy_rtd_part_outl(0x1803ba78, 27, 27, 0x0); //[APHY fix] 		G-Lane DA_EG_VOS_PULLLOW(koffset disable)
	_phy_rtd_part_outl(0x1803ba78, 31, 29, 0x0); //[APHY fix] 			G-Lane dummy
	_phy_rtd_part_outl(0x1803ba7c, 2, 0, 0x0); //[APHY fix]  	G-Lane DFE_TAP_DELAY
	_phy_rtd_part_outl(0x1803ba7c, 8, 8, APHY_Fix_tmds_1[value_param].G_14_200); //[APHY fix]   	G-Lane rstb of dtata-lane test div8
	_phy_rtd_part_outl(0x1803ba7c, 16, 16, APHY_Fix_tmds_1[value_param].G_15_201); //[APHY fix]  		G-Lane TRANSITION CNT enable.
	_phy_rtd_part_outl(0x1803ba7c, 17, 17, 0x1); //[APHY fix]  		G-Lane QCG1 injection-locked EN
	_phy_rtd_part_outl(0x1803ba7c, 18, 18, 0x0); //[APHY fix]  		G-Lane QCG2_EN
	_phy_rtd_part_outl(0x1803ba7c, 19, 19, 0x0); //[APHY fix]  		G-Lane QCG2 injection-locked EN
	_phy_rtd_part_outl(0x1803ba7c, 20, 20, 0x0); //[APHY fix]  		G-Lane QCG FLD divider(/8) rstb
	_phy_rtd_part_outl(0x1803ba7c, 21, 21, 0x0); //[APHY fix]  		G-Lane QCG FLD SEL
	_phy_rtd_part_outl(0x1803ba7c, 22, 22, 0x1); //[APHY fix]  		G-Lane CK_FLD SEL.
	_phy_rtd_part_outl(0x1803ba7c, 23, 23, 0x0); //[APHY fix]  		G-Lane CK-lane clk to B/G/R EN.
	_phy_rtd_part_outl(0x1803ba80, 8, 8, 0x1); //[APHY fix]  G-Lane PI_EN
	_phy_rtd_part_outl(0x1803ba80, 12, 12, 0x0); //[APHY fix]  G-Lane QCG1 Cload contrl SEL
	_phy_rtd_part_outl(0x1803ba80, 13, 13, 0x0); //[APHY fix]  G-Lane QCG2 Cload contrl SEL
	_phy_rtd_part_outl(0x1803ba80, 15, 15, 0x0); //[APHY fix]  G-Lane BIAS_PI_CUR_SEL
	_phy_rtd_part_outl(0x1803ba80, 21, 21, 0x0); //[APHY fix]  G-Lane PI_EYE_EN
	_phy_rtd_part_outl(0x1803ba80, 23, 23, 0x0); //[APHY fix]  G-Lane KI SEL
	_phy_rtd_part_outl(0x1803ba80, 25, 25, 0x0); //[APHY fix]  G-Lane FKP_RSTB_SEL
	_phy_rtd_part_outl(0x1803ba80, 27, 26, 0x0); //[APHY fix]  G-Lane KI_CK_SEL
	_phy_rtd_part_outl(0x1803ba80, 29, 29, 0x0); //[APHY fix]  G-Lane ST_CODE SEL
	_phy_rtd_part_outl(0x1803ba80, 30, 30, 0x0); //[APHY fix]  G-Lane QCG Ckin SEL.
	_phy_rtd_part_outl(0x1803ba84, 3, 2, 0x0); //[APHY fix]  G-Lane ACDR_SEL_DIV_FLD[1:0]
	_phy_rtd_part_outl(0x1803ba84, 5, 4, 0x0); //[APHY fix]  G-Lane ACDR_SEL_DIV_REF[1:0]
	_phy_rtd_part_outl(0x1803ba84, 7, 6, 0x0); //[APHY fix]  G-Lane ACDR_SEL_DIV_PLL[1:0]
	_phy_rtd_part_outl(0x1803ba84, 9, 8, 0x0); //[APHY fix]  G-Lane ACDR_SEL_DIV_TRAINING[1:0]
	_phy_rtd_part_outl(0x1803ba84, 12, 12, 0x0); //[APHY fix]  G-Lane dummy
	_phy_rtd_part_outl(0x1803ba84, 13, 13, 0x0); //[APHY fix]  G-Lane ACDR_SEL_FLD_0CKFB_1CKREF
	_phy_rtd_part_outl(0x1803ba84, 15, 14, 0x0); //[APHY fix]  G-Lane no use
	_phy_rtd_part_outl(0x1803ba84, 18, 18, 0x0); //[APHY fix]  G-Lane ACDR_RSTB_DIV_PLL ... removed
	_phy_rtd_part_outl(0x1803ba84, 24, 24, 0x1); //[APHY fix]  G-Lane ACDR_EN_UPDN_PULSE_FILTER
	_phy_rtd_part_outl(0x1803ba84, 25, 25, 0x1); //[APHY fix]  G-Lane CMU_SEL_CP
	_phy_rtd_part_outl(0x1803ba84, 27, 26, 0x0); //[APHY fix]  G-Lane ACDR_SEL_UPDN_WIDTH[1:0]
	_phy_rtd_part_outl(0x1803ba84, 28, 28, 0x0); //[APHY fix]  G-Lane ACDR_POW_LPF_IDAC
	_phy_rtd_part_outl(0x1803ba84, 31, 29, 0x0); //[APHY fix]  G-Lane ACDR_SEL_LPF_IDAC[2:0]
	_phy_rtd_part_outl(0x1803ba88, 0, 0, 0x1); //[APHY fix]  G-Lane ACDR_POW_CP
	_phy_rtd_part_outl(0x1803ba88, 2, 2, 0x0); //[APHY fix]  G-Lane ACDR_POW_IDN_BBPD
	_phy_rtd_part_outl(0x1803ba88, 3, 3, 0x0); //[APHY fix]  G-Lane no use
	_phy_rtd_part_outl(0x1803ba88, 6, 4, 0x2); //[APHY fix]  G-Lane ACDR_SEL_TIE_IDN_BBPD
	_phy_rtd_part_outl(0x1803ba88, 7, 7, 0x1); //[APHY fix]  G-Lane ACDR_POW_IBIAS_IDN_HV
	_phy_rtd_part_outl(0x1803ba88, 8, 8, 0x1); //[APHY fix]  G-Lane ACDR_POW_VCO
	_phy_rtd_part_outl(0x1803ba88, 9, 9, 0x1); //[APHY fix]  G-Lane ACDR_POW_VCO_VDAC
	_phy_rtd_part_outl(0x1803ba88, 11, 10, 0x0); //[APHY fix]  G-Lane ACDR_SEL_V15_VDAC
	_phy_rtd_part_outl(0x1803ba88, 14, 14, 0x0); //[APHY fix]  G-Lane no use
	_phy_rtd_part_outl(0x1803ba88, 15, 15, 0x0); //[APHY fix]  G-Lane no use
	_phy_rtd_part_outl(0x1803ba88, 16, 16, 0x0); //[APHY fix]  G-Lane ACDR_POW_TEST_MODE
	_phy_rtd_part_outl(0x1803ba88, 18, 17, 0x0); //[APHY fix]  G-Lane ACDR_SEL_TEST_MODE
	_phy_rtd_part_outl(0x1803ba88, 23, 19, 0x00); //[APHY fix]  G-Lane no use
	_phy_rtd_part_outl(0x1803ba88, 31, 31, 0x1); //[APHY fix]  G-Lane no use
	_phy_rtd_part_outl(0x1803ba8c, 13, 13, APHY_Fix_tmds_1[value_param].ACDR_G_10_246); //[APHY fix]  G-Lane ckref sel. 0:xtal, 1:tx or train clk
	_phy_rtd_part_outl(0x1803ba8c, 14, 14, 0x1); //[APHY fix]  G-Lane ckref sel. 0:train_clk, 1:tx_clk
	_phy_rtd_part_outl(0x1803ba8c, 15, 15, 0x0); //[APHY fix]  G-Lane ckbk sel. 0:M-DIV, 1:PRESCALER
	_phy_rtd_part_outl(0x1803ba8c, 19, 16, 0x0); //[APHY fix]  G-Lane LE1_ISEL_IN_2 -> dummy
	_phy_rtd_part_outl(0x1803ba8c, 31, 25, 0x00); //[APHY fix]  G-Lane no use for LEQ
	_phy_rtd_part_outl(0x1803ba90, 1, 1, 0x0); //[APHY fix]  		R-Lane INPOFF_SINGLE_EN
	_phy_rtd_part_outl(0x1803ba90, 2, 2, 0x0); //[APHY fix]  		R-Lane INNOFF_SINGLE_EN
	_phy_rtd_part_outl(0x1803ba90, 3, 3, 0x1); //[APHY fix]  		R-Lane POW_AC_COUPLE
	_phy_rtd_part_outl(0x1803ba90, 5, 4, 0x1); //[APHY fix]  		R-Lane RXVCM_SEL[1:0]
	_phy_rtd_part_outl(0x1803ba94, 21, 21, 0x1); //[APHY fix]   		R-Lane RS_CAL_EN
	_phy_rtd_part_outl(0x1803ba94, 25, 25, 0x1); //[APHY fix]   		R-Lane POW_DATALANE_BIAS
	_phy_rtd_part_outl(0x1803ba94, 26, 26, 0x0); //[APHY fix]   		R-Lane REG_FORCE_STARTUP(Const-GM)
	_phy_rtd_part_outl(0x1803ba94, 27, 27, 0x0); //[APHY fix]   		R-Lane REG_POWB_STARTUP
	_phy_rtd_part_outl(0x1803ba98, 8, 8, 0x1); //[APHY fix]  		R-Lane POW_DFE
	_phy_rtd_part_outl(0x1803ba98, 14, 12, 0x6); //[APHY fix] 		R-Lane DFE_SUMAMP_ ISEL
	_phy_rtd_part_outl(0x1803ba98, 15, 15, 0x0); //[APHY fix] 		R-Lane DFE_SUMAMP_DCGAIN_MAX
	_phy_rtd_part_outl(0x1803ba98, 19, 18, 0x0); //[APHY fix] 		R-Lane Dummy
	_phy_rtd_part_outl(0x1803ba98, 20, 20, 0x0); //[APHY fix] 		R-Lane DFE CKI_DELAY_EN
	_phy_rtd_part_outl(0x1803ba98, 21, 21, 0x0); //[APHY fix] 		R-Lane DFE CKIB_DELAY_EN
	_phy_rtd_part_outl(0x1803ba98, 22, 22, 0x0); //[APHY fix] 		R-Lane DFE CKQ_DELAY_EN
	_phy_rtd_part_outl(0x1803ba98, 23, 23, 0x0); //[APHY fix] 		R-Lane DFE CKQB_DELAY_EN
	_phy_rtd_part_outl(0x1803ba98, 24, 24, 0x0); //[APHY fix] 		R-Lane EN_EYE_MNT
	_phy_rtd_part_outl(0x1803ba98, 25, 25, 0x0); //[APHY fix] 		R-Lane DEMUX input clock phase select:(for eye mnt)
	_phy_rtd_part_outl(0x1803ba98, 26, 26, 0x1); //[APHY fix] R-Lane VTH+OFFSET select. 0:VTH, 1:VTH+OFFSET
	_phy_rtd_part_outl(0x1803ba98, 27, 27, 0x0); //[APHY fix] 		R-Lane DA_EG_VOS_PULLLOW(koffset disable)
	_phy_rtd_part_outl(0x1803ba98, 31, 29, 0x0); //[APHY fix] 			R-Lane dummy
	_phy_rtd_part_outl(0x1803ba9c, 2, 0, 0x0); //[APHY fix]  	R-Lane DFE_TAP_DELAY
	_phy_rtd_part_outl(0x1803ba9c, 8, 8, APHY_Fix_tmds_1[value_param].R_14_273); //[APHY fix]   	R-Lane rstb of dtata-lane test div8
	_phy_rtd_part_outl(0x1803ba9c, 16, 16, APHY_Fix_tmds_1[value_param].R_15_274); //[APHY fix]  		R-Lane TRANSITION CNT enable.
	_phy_rtd_part_outl(0x1803ba9c, 17, 17, 0x1); //[APHY fix]  		R-Lane QCG1 injection-locked EN
	_phy_rtd_part_outl(0x1803ba9c, 18, 18, 0x0); //[APHY fix]  		R-Lane QCG2_EN
	_phy_rtd_part_outl(0x1803ba9c, 19, 19, 0x0); //[APHY fix]  		R-Lane QCG2 injection-locked EN
	_phy_rtd_part_outl(0x1803ba9c, 20, 20, 0x0); //[APHY fix]  		R-Lane QCG FLD divider(/8) rstb
	_phy_rtd_part_outl(0x1803ba9c, 21, 21, 0x0); //[APHY fix]  		R-Lane QCG FLD SEL
	_phy_rtd_part_outl(0x1803ba9c, 22, 22, 0x1); //[APHY fix]  		R-Lane CK_FLD SEL.
	_phy_rtd_part_outl(0x1803ba9c, 23, 23, 0x0); //[APHY fix]  		R-Lane CK-lane clk to B/G/R EN.
	_phy_rtd_part_outl(0x1803baa0, 8, 8, 0x1); //[APHY fix]  R-Lane PI_EN
	_phy_rtd_part_outl(0x1803baa0, 12, 12, 0x0); //[APHY fix]  R-Lane QCG1 Cload contrl SEL
	_phy_rtd_part_outl(0x1803baa0, 13, 13, 0x0); //[APHY fix]  R-Lane QCG2 Cload contrl SEL
	_phy_rtd_part_outl(0x1803baa0, 15, 15, 0x0); //[APHY fix]  R-Lane BIAS_PI_CUR_SEL
	_phy_rtd_part_outl(0x1803baa0, 21, 21, 0x0); //[APHY fix]  R-Lane PI_EYE_EN
	_phy_rtd_part_outl(0x1803baa0, 23, 23, 0x0); //[APHY fix]  R-Lane KI SEL
	_phy_rtd_part_outl(0x1803baa0, 25, 25, 0x0); //[APHY fix]  R-Lane FKP_RSTB_SEL
	_phy_rtd_part_outl(0x1803baa0, 27, 26, 0x0); //[APHY fix]  R-Lane KI_CK_SEL
	_phy_rtd_part_outl(0x1803baa0, 29, 29, 0x0); //[APHY fix]  R-Lane ST_CODE SEL
	_phy_rtd_part_outl(0x1803baa0, 30, 30, 0x0); //[APHY fix]  R-Lane QCG Ckin SEL.
	_phy_rtd_part_outl(0x1803baa4, 3, 2, 0x0); //[APHY fix]  R-Lane ACDR_SEL_DIV_FLD[1:0]
	_phy_rtd_part_outl(0x1803baa4, 5, 4, 0x0); //[APHY fix]  R-Lane ACDR_SEL_DIV_REF[1:0]
	_phy_rtd_part_outl(0x1803baa4, 7, 6, 0x0); //[APHY fix]  R-Lane ACDR_SEL_DIV_PLL[1:0]
	_phy_rtd_part_outl(0x1803baa4, 9, 8, 0x0); //[APHY fix]  R-Lane ACDR_SEL_DIV_TRAINING[1:0]
	_phy_rtd_part_outl(0x1803baa4, 12, 12, 0x0); //[APHY fix]  R-Lane dummy
	_phy_rtd_part_outl(0x1803baa4, 13, 13, 0x0); //[APHY fix]  R-Lane ACDR_SEL_FLD_0CKFB_1CKREF
	_phy_rtd_part_outl(0x1803baa4, 15, 14, 0x0); //[APHY fix]  R-Lane no use
	_phy_rtd_part_outl(0x1803baa4, 18, 18, 0x0); //[APHY fix]  R-Lane ACDR_RSTB_DIV_PLL ... removed
	_phy_rtd_part_outl(0x1803baa4, 24, 24, 0x1); //[APHY fix]  R-Lane ACDR_EN_UPDN_PULSE_FILTER
	_phy_rtd_part_outl(0x1803baa4, 25, 25, 0x1); //[APHY fix]  R-Lane CMU_SEL_CP
	_phy_rtd_part_outl(0x1803baa4, 27, 26, 0x0); //[APHY fix]  R-Lane ACDR_SEL_UPDN_WIDTH[1:0]
	_phy_rtd_part_outl(0x1803baa4, 28, 28, 0x0); //[APHY fix]  R-Lane ACDR_POW_LPF_IDAC
	_phy_rtd_part_outl(0x1803baa4, 31, 29, 0x0); //[APHY fix]  R-Lane ACDR_SEL_LPF_IDAC[2:0]
	_phy_rtd_part_outl(0x1803baa8, 0, 0, 0x1); //[APHY fix]  R-Lane ACDR_POW_CP
	_phy_rtd_part_outl(0x1803baa8, 2, 2, 0x0); //[APHY fix]  R-Lane ACDR_POW_IDN_BBPD
	_phy_rtd_part_outl(0x1803baa8, 3, 3, 0x0); //[APHY fix]  R-Lane no use
	_phy_rtd_part_outl(0x1803baa8, 6, 4, 0x2); //[APHY fix]  R-Lane ACDR_SEL_TIE_IDN_BBPD
	_phy_rtd_part_outl(0x1803baa8, 7, 7, 0x1); //[APHY fix]  R-Lane ACDR_POW_IBIAS_IDN_HV
	_phy_rtd_part_outl(0x1803baa8, 8, 8, 0x1); //[APHY fix]  R-Lane ACDR_POW_VCO
	_phy_rtd_part_outl(0x1803baa8, 9, 9, 0x1); //[APHY fix]  R-Lane ACDR_POW_VCO_VDAC
	_phy_rtd_part_outl(0x1803baa8, 11, 10, 0x0); //[APHY fix]  R-Lane ACDR_SEL_V15_VDAC
	_phy_rtd_part_outl(0x1803baa8, 14, 14, 0x0); //[APHY fix]  R-Lane no use
	_phy_rtd_part_outl(0x1803baa8, 15, 15, 0x0); //[APHY fix]  R-Lane no use
	_phy_rtd_part_outl(0x1803baa8, 16, 16, 0x0); //[APHY fix]  R-Lane ACDR_POW_TEST_MODE
	_phy_rtd_part_outl(0x1803baa8, 18, 17, 0x0); //[APHY fix]  R-Lane ACDR_SEL_TEST_MODE
	_phy_rtd_part_outl(0x1803baa8, 23, 19, 0x00); //[APHY fix]  R-Lane no use
	_phy_rtd_part_outl(0x1803baa8, 31, 31, 0x1); //[APHY fix]  R-Lane no use
	_phy_rtd_part_outl(0x1803baac, 13, 13, APHY_Fix_tmds_1[value_param].ACDR_R_10_319); //[APHY fix]  R-Lane ckref sel. 0:xtal, 1:tx or train clk
	_phy_rtd_part_outl(0x1803baac, 14, 14, 0x1); //[APHY fix]  R-Lane ckref sel. 0:train_clk, 1:tx_clk
	_phy_rtd_part_outl(0x1803baac, 15, 15, 0x0); //[APHY fix]  R-Lane ckbk sel. 0:M-DIV, 1:PRESCALER
	_phy_rtd_part_outl(0x1803baac, 19, 16, 0x0); //[APHY fix]  R-Lane LE1_ISEL_IN_2 -> dummy
	_phy_rtd_part_outl(0x1803baac, 31, 25, 0x00); //[APHY fix]  R-Lane no use for LEQ
	_phy_rtd_part_outl(0x1803bab4, 0, 0, 0x1); //[APHY fix]  Non-Hysteresis Amp Power
	_phy_rtd_part_outl(0x1803bab4, 3, 1, 0x5); //[APHY fix]  CK Mode hysteresis amp Hysteresis adjustment[2:0]
	_phy_rtd_part_outl(0x1803bab4, 5, 5, 0x0); //[APHY fix]  CK Mode detect source
	_phy_rtd_part_outl(0x1803bab4, 6, 6, 0x0); //[APHY fix]  Hysteresis amp power
	_phy_rtd_part_outl(0x1803bab4, 8, 8, 0x0); //[APHY fix]  CMU_WDPOW
	_phy_rtd_part_outl(0x1803bab4, 9, 9, 0x0); //[APHY fix]  CMU_WDSET
	_phy_rtd_part_outl(0x1803bab4, 10, 10, 0x0); //[APHY fix]  CMU_WDRST
	_phy_rtd_part_outl(0x1803bab4, 11, 11, 0x1); //[APHY fix]  no use
	_phy_rtd_part_outl(0x1803bab4, 12, 12, 0x0); //[APHY fix]  EN_TST_CKBK
	_phy_rtd_part_outl(0x1803bab4, 13, 13, 0x0); //[APHY fix]  EN_TST_CKREF
	_phy_rtd_part_outl(0x1803bab4, 15, 14, 0x0); //[APHY fix]  no use
	_phy_rtd_part_outl(0x1803bab4, 23, 16, 0x00); //[APHY fix]  dummy
	_phy_rtd_part_outl(0x1803bab4, 31, 24, 0x00); //[APHY fix]  dummy
	_phy_rtd_part_outl(0x1803bab8, 7, 0, 0xf4); //[APHY fix]  dummy
	_phy_rtd_part_outl(0x1803bab8, 9, 9, 0x1); //[APHY fix]  no use
	_phy_rtd_part_outl(0x1803bab8, 10, 10, 0x1); //[APHY fix]  CMU_SEL_CS
	_phy_rtd_part_outl(0x1803bab8, 15, 11, 0x09); //[APHY fix]  no use
	_phy_rtd_part_outl(0x1803bab8, 23, 16, 0x06); //[APHY fix]  dummy
	_phy_rtd_part_outl(0x1803bab8, 31, 24, 0x07); //[APHY fix]  dummy
	_phy_rtd_part_outl(0x1803babc, 7, 0, 0x44); //[APHY fix]  dummy
	_phy_rtd_part_outl(0x1803babc, 15, 8, 0xef); //[APHY fix]  dummy
	_phy_rtd_part_outl(0x1803babc, 23, 16, 0x0f); //[APHY fix]  dummy
	_phy_rtd_part_outl(0x1803babc, 31, 24, 0x00); //[APHY fix]  dummy
}

void APHY_Para_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	// APHY para start
	_phy_rtd_part_outl(0x1803ba3c, 11, 9, APHY_Para_tmds_1[value_param].CK_14_2); //[APHY para]   	CK-Lane DEMUX_PIX_RATE_SEL<2:0>
	_phy_rtd_part_outl(0x1803ba3c, 15, 13, APHY_Para_tmds_1[value_param].CK_14_3); //[APHY para]   CK-Lane DEMUX_RATE_SEL
	_phy_rtd_part_outl(0x1803ba40, 11, 9, 0x0); //[APHY para]  CK-Lane PI_ISEL<2:0>
	_phy_rtd_part_outl(0x1803ba40, 14, 14, APHY_Para_tmds_1[value_param].CK_18_5); //[APHY para]  CK-Lane PI_CSEL
	_phy_rtd_part_outl(0x1803ba40, 20, 18, APHY_Para_tmds_1[value_param].CK_19_6); //[APHY para]  CK-Lane PI_DIV_SEL
	_phy_rtd_part_outl(0x1803ba40, 31, 31, 0x1); //[APHY para]  CK-Lane DFE Ckin SEL.
	_phy_rtd_part_outl(0x1803ba64, 19, 19, APHY_Para_tmds_1[value_param].ACDR_B_3_8); //[APHY para]  B-Lane  ACDR_SEL_HS_CLK
	_phy_rtd_part_outl(0x1803ba84, 19, 19, APHY_Para_tmds_1[value_param].ACDR_G_3_9); //[APHY para]  G-Lane  ACDR_SEL_HS_CLK
	_phy_rtd_part_outl(0x1803baa4, 19, 19, APHY_Para_tmds_1[value_param].ACDR_R_3_10); //[APHY para]  R-Lane  ACDR_SEL_HS_CLK
	_phy_rtd_part_outl(0x1803ba44, 19, 19, APHY_Para_tmds_1[value_param].ACDR_CK_3_11); //[APHY para]  CK-Lane  ACDR_SEL_HS_CLK
	_phy_rtd_part_outl(0x1803ba48, 1, 1, 0x0); //[APHY para]  CK-Lane ACDR_POW_CP_INTG2_CORE
	_phy_rtd_part_outl(0x1803ba68, 1, 1, APHY_Para_tmds_1[value_param].ACDR_B_5_13); //[APHY para]  B-Lane ACDR_POW_CP_INTG2_CORE
	_phy_rtd_part_outl(0x1803ba88, 1, 1, APHY_Para_tmds_1[value_param].ACDR_G_5_14); //[APHY para]  G-Lane ACDR_POW_CP_INTG2_CORE
	_phy_rtd_part_outl(0x1803baa8, 1, 1, APHY_Para_tmds_1[value_param].ACDR_R_5_15); //[APHY para]  R-Lane ACDR_POW_CP_INTG2_CORE
	_phy_rtd_part_outl(0x1803ba48, 13, 12, 0x3); //[APHY para]  CK-Lane ACDR_SEL_BAND_CAP
	_phy_rtd_part_outl(0x1803ba68, 13, 12, APHY_Para_tmds_1[value_param].ACDR_B_6_17); //[APHY para]  B-Lane ACDR_SEL_BAND_CAP
	_phy_rtd_part_outl(0x1803ba88, 13, 12, APHY_Para_tmds_1[value_param].ACDR_G_6_18); //[APHY para]  G-Lane ACDR_SEL_BAND_CAP
	_phy_rtd_part_outl(0x1803baa8, 13, 12, APHY_Para_tmds_1[value_param].ACDR_R_6_19); //[APHY para]  R-Lane ACDR_SEL_BAND_CAP
	_phy_rtd_part_outl(0x1803ba4c, 7, 0, APHY_Para_tmds_1[value_param].ACDR_CK_9_20); //[APHY para]  CK-Lane CMU_SEL_M_DIV[7:0]
	_phy_rtd_part_outl(0x1803ba4c, 8, 8, 0x0); //[APHY para]  CK-Lane CMU_SEL_M_DIV[8]
	_phy_rtd_part_outl(0x1803ba4c, 9, 9, 0x0); //[APHY para]  CK-Lane CMU_SEL_PRE_M_DIV
	_phy_rtd_part_outl(0x1803ba4c, 10, 10, APHY_Para_tmds_1[value_param].ACDR_CK_10_23); //[APHY para]  CK-Lane CMU_SEL_POST_M_DIV
	_phy_rtd_part_outl(0x1803ba4c, 21, 20, APHY_Para_tmds_1[value_param].ACDR_CK_11_24); //[APHY para]  CK-Lane RLSEL_LE1_2
	_phy_rtd_part_outl(0x1803ba50, 9, 8, APHY_Para_tmds_1[value_param].B_2_25); //[APHY para]  		B-Lane RLSEL_LE1
	_phy_rtd_part_outl(0x1803ba50, 11, 10, APHY_Para_tmds_1[value_param].B_2_26); //[APHY para]  		B-Lane RLSEL_LE2
	_phy_rtd_part_outl(0x1803ba50, 13, 12, APHY_Para_tmds_1[value_param].B_2_27); //[APHY para]  		B-Lane RLSEL_NC
	_phy_rtd_part_outl(0x1803ba50, 15, 14, APHY_Para_tmds_1[value_param].B_2_28); //[APHY para]  		B-Lane RLSEL_TAP0
	_phy_rtd_part_outl(0x1803ba50, 19, 16, APHY_Para_tmds_1[value_param].B_3_29); //[APHY para]  		B-Lane RSSEL_LE1_1
	_phy_rtd_part_outl(0x1803ba50, 23, 20, APHY_Para_tmds_1[value_param].B_3_30); //[APHY para]  		B-Lane RSSEL_LE1_2
	_phy_rtd_part_outl(0x1803ba50, 27, 24, APHY_Para_tmds_1[value_param].B_4_31); //[APHY para]  		B-Lane RSSEL_LE2
	_phy_rtd_part_outl(0x1803ba50, 29, 28, APHY_Para_tmds_1[value_param].B_4_32); //[APHY para]  		B-Lane RSSEL_TAP0
	_phy_rtd_part_outl(0x1803ba50, 31, 30, APHY_Para_tmds_1[value_param].B_4_33); //[APHY para]  	B-Lane KOFF_RANGE
	_phy_rtd_part_outl(0x1803ba54, 3, 0, APHY_Para_tmds_1[value_param].B_5_34); //[APHY para]    	B-Lane LE1_ISEL_IN
	_phy_rtd_part_outl(0x1803ba54, 7, 4, APHY_Para_tmds_1[value_param].B_5_35); //[APHY para]    	B-Lane LE2_ISEL_IN
	_phy_rtd_part_outl(0x1803ba54, 11, 8, APHY_Para_tmds_1[value_param].B_6_36); //[APHY para]   		B-Lane LE_NC_ISEL_IN
	_phy_rtd_part_outl(0x1803ba54, 15, 12, 0xf); //[APHY para]  		B-Lane TAP0_ISEL_IN
	_phy_rtd_part_outl(0x1803ba54, 17, 16, APHY_Para_tmds_1[value_param].B_7_38); //[APHY para]   		B-Lane LE_IHALF[1:0]
	_phy_rtd_part_outl(0x1803ba54, 18, 18, APHY_Para_tmds_1[value_param].B_7_39); //[APHY para]   		B-Lane NC_IHALF
	_phy_rtd_part_outl(0x1803ba54, 19, 19, APHY_Para_tmds_1[value_param].B_7_40); //[APHY para]   		B-Lane TAP0_IHALF
	_phy_rtd_part_outl(0x1803ba54, 20, 20, 0x0); //[APHY para]   		B-Lane EN_LEQ6G
	_phy_rtd_part_outl(0x1803ba54, 22, 22, APHY_Para_tmds_1[value_param].B_7_42); //[APHY para]   	B-Lane POW_NC
	_phy_rtd_part_outl(0x1803ba54, 30, 28, 0x3); //[APHY para]   		B-Lane DATALANE_BIAS_ISEL[2:0]
	_phy_rtd_part_outl(0x1803ba54, 31, 31, 0x1); //[APHY para]   		B-Lane POW_LEQ_RL
	_phy_rtd_part_outl(0x1803ba58, 1, 1, APHY_Para_tmds_1[value_param].B_9_45); //[APHY para]  		B-Lane SEL_CMFB_LS
	_phy_rtd_part_outl(0x1803ba58, 3, 2, 0x3); //[APHY para]  		B-Lane LEQ_CUR_ADJ
	_phy_rtd_part_outl(0x1803ba58, 5, 4, 0x0); //[APHY para]  		B-Lane PTAT_CUR_ADJ
	_phy_rtd_part_outl(0x1803ba58, 6, 6, 0x0); //[APHY para]  		B-Lane REG_BIAS_POW_CON_GM
	_phy_rtd_part_outl(0x1803ba58, 11, 9, 0x0); //[APHY para] 		B-Lane PTAT_CUR_ADJ_FINE
	_phy_rtd_part_outl(0x1803ba58, 16, 16, 0x1); //[APHY para] 		B-Lane FR_CK_SEL
	_phy_rtd_part_outl(0x1803ba5c, 11, 9, APHY_Para_tmds_1[value_param].B_14_51); //[APHY para]   	B-Lane DEMUX_PIX_RATE_SEL<2:0>
	_phy_rtd_part_outl(0x1803ba5c, 12, 12, 0x1); //[APHY para]   	B-Lane DEMUX_FR_CK_SEL
	_phy_rtd_part_outl(0x1803ba5c, 15, 13, APHY_Para_tmds_1[value_param].B_14_53); //[APHY para]   	B-Lane DEMUX_RATE_SEL
	_phy_rtd_part_outl(0x1803ba5c, 24, 24, 0x0); //[APHY para]  		B-Lane QCG1_SEL_BIAS_ST
	_phy_rtd_part_outl(0x1803ba5c, 25, 25, 0x0); //[APHY para]  		B-Lane QCG1_SEL_BIAS_RG
	_phy_rtd_part_outl(0x1803ba5c, 27, 26, APHY_Para_tmds_1[value_param].B_16_56); //[APHY para]  		B-Lane QCG1 Rload SEL
	_phy_rtd_part_outl(0x1803ba5c, 31, 28, APHY_Para_tmds_1[value_param].B_16_57); //[APHY para]  		B-Lane QCG1 Cload SEL
	_phy_rtd_part_outl(0x1803ba60, 11, 9, APHY_Para_tmds_1[value_param].B_18_58); //[APHY para]  B-Lane PI_ISEL<2:0>
	_phy_rtd_part_outl(0x1803ba60, 14, 14, 0x1); //[APHY para]  B-Lane PI_CSEL
	_phy_rtd_part_outl(0x1803ba60, 20, 18, APHY_Para_tmds_1[value_param].B_19_60); //[APHY para]  B-Lane PI_DIV_SEL
	_phy_rtd_part_outl(0x1803ba60, 22, 22, 0x0); //[APHY para]  B-Lane DCDR ckout SEL
	_phy_rtd_part_outl(0x1803ba60, 31, 31, 0x1); //[APHY para]  B-Lane DFE Ckin SEL.
	_phy_rtd_part_outl(0x1803ba6c, 7, 0, APHY_Para_tmds_1[value_param].ACDR_B_9_63); //[APHY para]  B-Lane CMU_SEL_M_DIV[7:0]
	_phy_rtd_part_outl(0x1803ba6c, 8, 8, 0x0); //[APHY para]  B-Lane CMU_SEL_M_DIV[8]
	_phy_rtd_part_outl(0x1803ba6c, 9, 9, APHY_Para_tmds_1[value_param].ACDR_B_10_65); //[APHY para]  B-Lane CMU_SEL_PRE_M_DIV
	_phy_rtd_part_outl(0x1803ba6c, 10, 10, 0x1); //[APHY para]  B-Lane CMU_SEL_POST_M_DIV
	_phy_rtd_part_outl(0x1803ba6c, 21, 20, APHY_Para_tmds_1[value_param].ACDR_B_11_67); //[APHY para]  B-Lane RLSEL_LE1_2
	_phy_rtd_part_outl(0x1803ba6c, 23, 22, 0x0); //[APHY para]  B-Lane RLSEL_NC_2
	_phy_rtd_part_outl(0x1803ba6c, 24, 24, 0x0); //[APHY para]  B-Lane POW_NC
	_phy_rtd_part_outl(0x1803ba70, 9, 8, APHY_Para_tmds_1[value_param].G_2_70); //[APHY para]  		G-Lane RLSEL_LE1
	_phy_rtd_part_outl(0x1803ba70, 11, 10, APHY_Para_tmds_1[value_param].G_2_71); //[APHY para]  		G-Lane RLSEL_LE2
	_phy_rtd_part_outl(0x1803ba70, 13, 12, APHY_Para_tmds_1[value_param].G_2_72); //[APHY para]  		G-Lane RLSEL_NC
	_phy_rtd_part_outl(0x1803ba70, 15, 14, APHY_Para_tmds_1[value_param].G_2_73); //[APHY para]  		G-Lane RLSEL_TAP0
	_phy_rtd_part_outl(0x1803ba70, 19, 16, APHY_Para_tmds_1[value_param].G_3_74); //[APHY para]  		G-Lane RSSEL_LE1_1
	_phy_rtd_part_outl(0x1803ba70, 23, 20, APHY_Para_tmds_1[value_param].G_3_75); //[APHY para]  		G-Lane RSSEL_LE1_2
	_phy_rtd_part_outl(0x1803ba70, 27, 24, APHY_Para_tmds_1[value_param].G_4_76); //[APHY para]  		G-Lane RSSEL_LE2
	_phy_rtd_part_outl(0x1803ba70, 29, 28, APHY_Para_tmds_1[value_param].G_4_77); //[APHY para]  		G-Lane RSSEL_TAP0
	_phy_rtd_part_outl(0x1803ba70, 31, 30, APHY_Para_tmds_1[value_param].G_4_78); //[APHY para]  	G-Lane KOFF_RANGE
	_phy_rtd_part_outl(0x1803ba74, 3, 0, APHY_Para_tmds_1[value_param].G_5_79); //[APHY para]    	G-Lane LE1_ISEL_IN
	_phy_rtd_part_outl(0x1803ba74, 7, 4, APHY_Para_tmds_1[value_param].G_5_80); //[APHY para]    	G-Lane LE2_ISEL_IN
	_phy_rtd_part_outl(0x1803ba74, 11, 8, APHY_Para_tmds_1[value_param].G_6_81); //[APHY para]   		G-Lane LE_NC_ISEL_IN
	_phy_rtd_part_outl(0x1803ba74, 15, 12, 0xf); //[APHY para]  		G-Lane TAP0_ISEL_IN
	_phy_rtd_part_outl(0x1803ba74, 17, 16, APHY_Para_tmds_1[value_param].G_7_83); //[APHY para]   		G-Lane LE_IHALF[1:0]
	_phy_rtd_part_outl(0x1803ba74, 18, 18, APHY_Para_tmds_1[value_param].G_7_84); //[APHY para]   		G-Lane NC_IHALF
	_phy_rtd_part_outl(0x1803ba74, 19, 19, APHY_Para_tmds_1[value_param].G_7_85); //[APHY para]   		G-Lane TAP0_IHALF
	_phy_rtd_part_outl(0x1803ba74, 20, 20, 0x0); //[APHY para]   		G-Lane EN_LEQ6G
	_phy_rtd_part_outl(0x1803ba74, 22, 22, APHY_Para_tmds_1[value_param].G_7_87); //[APHY para]   	G-Lane POW_NC
	_phy_rtd_part_outl(0x1803ba74, 30, 28, 0x3); //[APHY para]   		G-Lane DATALANE_BIAS_ISEL[2:0]
	_phy_rtd_part_outl(0x1803ba74, 31, 31, 0x1); //[APHY para]   		G-Lane POW_LEQ_RL
	_phy_rtd_part_outl(0x1803ba78, 1, 1, APHY_Para_tmds_1[value_param].G_9_90); //[APHY para]  		G-Lane SEL_CMFB_LS
	_phy_rtd_part_outl(0x1803ba78, 3, 2, 0x3); //[APHY para]  		G-Lane LEQ_CUR_ADJ
	_phy_rtd_part_outl(0x1803ba78, 5, 4, 0x0); //[APHY para]  		G-Lane PTAT_CUR_ADJ
	_phy_rtd_part_outl(0x1803ba78, 6, 6, 0x0); //[APHY para]  		G-Lane REG_BIAS_POW_CON_GM
	_phy_rtd_part_outl(0x1803ba78, 11, 9, 0x0); //[APHY para] 		G-Lane PTAT_CUR_ADJ_FINE
	_phy_rtd_part_outl(0x1803ba78, 16, 16, 0x1); //[APHY para] 		G-Lane FR_CK_SEL
	_phy_rtd_part_outl(0x1803ba7c, 11, 9, APHY_Para_tmds_1[value_param].G_14_96); //[APHY para]   	G-Lane DEMUX_PIX_RATE_SEL<2:0>
	_phy_rtd_part_outl(0x1803ba7c, 12, 12, 0x1); //[APHY para]   	G-Lane DEMUX_FR_CK_SEL
	_phy_rtd_part_outl(0x1803ba7c, 15, 13, APHY_Para_tmds_1[value_param].G_14_98); //[APHY para]   	G-Lane DEMUX_RATE_SEL
	_phy_rtd_part_outl(0x1803ba7c, 24, 24, 0x0); //[APHY para]  		G-Lane QCG1_SEL_BIAS_ST
	_phy_rtd_part_outl(0x1803ba7c, 25, 25, 0x0); //[APHY para]  		G-Lane QCG1_SEL_BIAS_RG
	_phy_rtd_part_outl(0x1803ba7c, 27, 26, APHY_Para_tmds_1[value_param].G_16_101); //[APHY para]  		G-Lane QCG1 Rload SEL
	_phy_rtd_part_outl(0x1803ba7c, 31, 28, APHY_Para_tmds_1[value_param].G_16_102); //[APHY para]  		G-Lane QCG1 Cload SEL
	_phy_rtd_part_outl(0x1803ba80, 11, 9, APHY_Para_tmds_1[value_param].G_18_103); //[APHY para]  G-Lane PI_ISEL<2:0>
	_phy_rtd_part_outl(0x1803ba80, 14, 14, 0x1); //[APHY para]  G-Lane PI_CSEL
	_phy_rtd_part_outl(0x1803ba80, 20, 18, APHY_Para_tmds_1[value_param].G_19_105); //[APHY para]  G-Lane PI_DIV_SEL
	_phy_rtd_part_outl(0x1803ba80, 22, 22, 0x0); //[APHY para]  G-Lane DCDR ckout SEL
	_phy_rtd_part_outl(0x1803ba80, 31, 31, 0x1); //[APHY para]  G-Lane DFE Ckin SEL.
	_phy_rtd_part_outl(0x1803ba8c, 7, 0, APHY_Para_tmds_1[value_param].ACDR_G_9_108); //[APHY para]  G-Lane CMU_SEL_M_DIV[7:0]
	_phy_rtd_part_outl(0x1803ba8c, 8, 8, 0x0); //[APHY para]  G-Lane CMU_SEL_M_DIV[8]
	_phy_rtd_part_outl(0x1803ba8c, 9, 9, APHY_Para_tmds_1[value_param].ACDR_G_10_110); //[APHY para]  G-Lane CMU_SEL_PRE_M_DIV
	_phy_rtd_part_outl(0x1803ba8c, 10, 10, 0x1); //[APHY para]  G-Lane CMU_SEL_POST_M_DIV
	_phy_rtd_part_outl(0x1803ba8c, 21, 20, APHY_Para_tmds_1[value_param].ACDR_G_11_112); //[APHY para]  G-Lane RLSEL_LE1_2
	_phy_rtd_part_outl(0x1803ba8c, 23, 22, 0x0); //[APHY para]  G-Lane RLSEL_NC_2
	_phy_rtd_part_outl(0x1803ba8c, 24, 24, 0x0); //[APHY para]  G-Lane POW_NC
	_phy_rtd_part_outl(0x1803ba90, 9, 8, APHY_Para_tmds_1[value_param].R_2_115); //[APHY para]  		R-Lane RLSEL_LE1
	_phy_rtd_part_outl(0x1803ba90, 11, 10, APHY_Para_tmds_1[value_param].R_2_116); //[APHY para]  		R-Lane RLSEL_LE2
	_phy_rtd_part_outl(0x1803ba90, 13, 12, APHY_Para_tmds_1[value_param].R_2_117); //[APHY para]  		R-Lane RLSEL_NC
	_phy_rtd_part_outl(0x1803ba90, 15, 14, APHY_Para_tmds_1[value_param].R_2_118); //[APHY para]  		R-Lane RLSEL_TAP0
	_phy_rtd_part_outl(0x1803ba90, 19, 16, APHY_Para_tmds_1[value_param].R_3_119); //[APHY para]  		R-Lane RSSEL_LE1_1
	_phy_rtd_part_outl(0x1803ba90, 23, 20, APHY_Para_tmds_1[value_param].R_3_120); //[APHY para]  		R-Lane RSSEL_LE1_2
	_phy_rtd_part_outl(0x1803ba90, 27, 24, APHY_Para_tmds_1[value_param].R_4_121); //[APHY para]  		R-Lane RSSEL_LE2
	_phy_rtd_part_outl(0x1803ba90, 29, 28, APHY_Para_tmds_1[value_param].R_4_122); //[APHY para]  		R-Lane RSSEL_TAP0
	_phy_rtd_part_outl(0x1803ba90, 31, 30, APHY_Para_tmds_1[value_param].R_4_123); //[APHY para]  		R-Lane KOFF_RANGE
	_phy_rtd_part_outl(0x1803ba94, 3, 0, APHY_Para_tmds_1[value_param].R_5_124); //[APHY para]    	R-Lane LE1_ISEL_IN
	_phy_rtd_part_outl(0x1803ba94, 7, 4, APHY_Para_tmds_1[value_param].R_5_125); //[APHY para]    	R-Lane LE2_ISEL_IN
	_phy_rtd_part_outl(0x1803ba94, 11, 8, APHY_Para_tmds_1[value_param].R_6_126); //[APHY para]   		R-Lane LE_NC_ISEL_IN
	_phy_rtd_part_outl(0x1803ba94, 15, 12, 0xf); //[APHY para]  		R-Lane TAP0_ISEL_IN
	_phy_rtd_part_outl(0x1803ba94, 17, 16, APHY_Para_tmds_1[value_param].R_7_128); //[APHY para]   		R-Lane LE_IHALF[1:0]
	_phy_rtd_part_outl(0x1803ba94, 18, 18, APHY_Para_tmds_1[value_param].R_7_129); //[APHY para]   		R-Lane NC_IHALF
	_phy_rtd_part_outl(0x1803ba94, 19, 19, APHY_Para_tmds_1[value_param].R_7_130); //[APHY para]   		R-Lane TAP0_IHALF
	_phy_rtd_part_outl(0x1803ba94, 20, 20, 0x0); //[APHY para]   		R-Lane EN_LEQ6G
	_phy_rtd_part_outl(0x1803ba94, 22, 22, APHY_Para_tmds_1[value_param].R_7_132); //[APHY para]   	R-Lane POW_NC
	_phy_rtd_part_outl(0x1803ba94, 30, 28, 0x3); //[APHY para]   		R-Lane DATALANE_BIAS_ISEL[2:0]
	_phy_rtd_part_outl(0x1803ba94, 31, 31, 0x1); //[APHY para]   		R-Lane POW_LEQ_RL
	_phy_rtd_part_outl(0x1803ba98, 1, 1, APHY_Para_tmds_1[value_param].R_9_135); //[APHY para]  		R-Lane SEL_CMFB_LS
	_phy_rtd_part_outl(0x1803ba98, 3, 2, 0x3); //[APHY para]  		R-Lane LEQ_CUR_ADJ
	_phy_rtd_part_outl(0x1803ba98, 5, 4, 0x0); //[APHY para]  		R-Lane PTAT_CUR_ADJ
	_phy_rtd_part_outl(0x1803ba98, 6, 6, 0x0); //[APHY para]  		R-Lane REG_BIAS_POW_CON_GM
	_phy_rtd_part_outl(0x1803ba98, 11, 9, 0x0); //[APHY para] 		R-Lane PTAT_CUR_ADJ_FINE
	_phy_rtd_part_outl(0x1803ba98, 16, 16, 0x1); //[APHY para] 		R-Lane FR_CK_SEL
	_phy_rtd_part_outl(0x1803ba9c, 11, 9, APHY_Para_tmds_1[value_param].R_14_141); //[APHY para]   	R-Lane DEMUX_PIX_RATE_SEL<2:0>
	_phy_rtd_part_outl(0x1803ba9c, 12, 12, 0x1); //[APHY para]   	R-Lane DEMUX_FR_CK_SEL
	_phy_rtd_part_outl(0x1803ba9c, 15, 13, APHY_Para_tmds_1[value_param].R_14_143); //[APHY para]   	R-Lane DEMUX_RATE_SEL
	_phy_rtd_part_outl(0x1803ba9c, 24, 24, 0x0); //[APHY para]  		R-Lane QCG1_SEL_BIAS_ST
	_phy_rtd_part_outl(0x1803ba9c, 25, 25, 0x0); //[APHY para]  		R-Lane QCG1_SEL_BIAS_RG
	_phy_rtd_part_outl(0x1803ba9c, 27, 26, APHY_Para_tmds_1[value_param].R_16_146); //[APHY para]  		R-Lane QCG1 Rload SEL
	_phy_rtd_part_outl(0x1803ba9c, 31, 28, APHY_Para_tmds_1[value_param].R_16_147); //[APHY para]  		R-Lane QCG1 Cload SEL
	_phy_rtd_part_outl(0x1803baa0, 11, 9, APHY_Para_tmds_1[value_param].R_18_148); //[APHY para]  R-Lane PI_ISEL<2:0>
	_phy_rtd_part_outl(0x1803baa0, 14, 14, 0x1); //[APHY para]  R-Lane PI_CSEL
	_phy_rtd_part_outl(0x1803baa0, 20, 18, APHY_Para_tmds_1[value_param].R_19_150); //[APHY para]  R-Lane PI_DIV_SEL
	_phy_rtd_part_outl(0x1803baa0, 22, 22, 0x0); //[APHY para]  R-Lane DCDR ckout SEL
	_phy_rtd_part_outl(0x1803baa0, 31, 31, 0x1); //[APHY para]  R-Lane DFE Ckin SEL.
	_phy_rtd_part_outl(0x1803baac, 7, 0, APHY_Para_tmds_1[value_param].ACDR_R_9_153); //[APHY para]  R-Lane CMU_SEL_M_DIV[7:0]
	_phy_rtd_part_outl(0x1803baac, 8, 8, 0x0); //[APHY para]  R-Lane CMU_SEL_M_DIV[8]
	_phy_rtd_part_outl(0x1803baac, 9, 9, APHY_Para_tmds_1[value_param].ACDR_R_10_155); //[APHY para]  R-Lane CMU_SEL_PRE_M_DIV
	_phy_rtd_part_outl(0x1803baac, 10, 10, 0x1); //[APHY para]  R-Lane CMU_SEL_POST_M_DIV
	_phy_rtd_part_outl(0x1803baac, 21, 20, APHY_Para_tmds_1[value_param].ACDR_R_11_157); //[APHY para]  R-Lane RLSEL_LE1_2
	_phy_rtd_part_outl(0x1803baac, 23, 22, 0x0); //[APHY para]  R-Lane RLSEL_NC_2
	_phy_rtd_part_outl(0x1803baac, 24, 24, 0x0); //[APHY para]  R-Lane POW_NC
	_phy_rtd_part_outl(0x1803bab4, 4, 4, 0x0); //[APHY para]  CMU input clock source select:
	_phy_rtd_part_outl(0x1803bab4, 7, 7, 0x1); //[APHY para]  CMU_CKXTAL_SEL
	_phy_rtd_part_outl(0x1803bab8, 8, 8, 0x1); //[APHY para  CMU_SEL_CP
}

void DPHY_Init_Flow_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	// DPHY init flow start
	_phy_rtd_part_outl(0x1803bb88, 4, 4, 0x1); //[DPHY init flow]  reg_p0_b_fifo_en
	_phy_rtd_part_outl(0x1803bb88, 12, 12, 0x1); //[DPHY init flow]  reg_p0_g_fifo_en
	_phy_rtd_part_outl(0x1803bb88, 20, 20, 0x1); //[DPHY init flow]  reg_p0_r_fifo_en
	_phy_rtd_part_outl(0x1803bb88, 28, 28, 0x1); //[DPHY init flow]  reg_p0_ck_fifo_en
	_phy_rtd_part_outl(0x1803bc6c, 1, 1, 0x0); //[DPHY init flow]  reg_p0_pow_save_10b18b_sel
	_phy_rtd_part_outl(0x1803bc7c, 5, 0, 0x0e); //[DPHY init flow]  reg_p0_dig_debug_sel
	//DPHY fld flow start
	_phy_rtd_part_outl(0x1803bbb8, 10, 3, 0x08); //[DPHY fld flow]  reg_p0_b_divide_num
	_phy_rtd_part_outl(0x1803bbbc, 11, 0, 0x0db); //[DPHY fld flow] reg_p0_b_lock_dn_limit
	_phy_rtd_part_outl(0x1803bbbc, 27, 16, 0x0e1); //[DPHY fld flow] reg_p0_b_lock_up_limit
	_phy_rtd_part_outl(0x1803bbc4, 19, 8, 0x0db); //[DPHY fld flow] reg_p0_b_coarse_lock_dn_limit
	_phy_rtd_part_outl(0x1803bbc4, 31, 20, 0x0e1); //[DPHY fld flow] reg_p0_b_coarse_lock_up_limit
	_phy_rtd_part_outl(0x1803bbc8, 1, 1, 0x1); //[DPHY fld flow]  reg_p0_b_fld_rstb
	_phy_rtd_part_outl(0x1803bbd8, 10, 3, 0x08); //[DPHY fld flow]  reg_p0_g_divide_num
	_phy_rtd_part_outl(0x1803bbdc, 11, 0, 0x0db); //[DPHY fld flow] reg_p0_g_lock_dn_limit
	_phy_rtd_part_outl(0x1803bbdc, 27, 16, 0x0e1); //[DPHY fld flow] reg_p0_g_lock_up_limit
	_phy_rtd_part_outl(0x1803bbe4, 19, 8, 0x0db); //[DPHY fld flow] reg_p0_g_coarse_lock_dn_limit
	_phy_rtd_part_outl(0x1803bbe4, 31, 20, 0x0e1); //[DPHY fld flow] reg_p0_g_coarse_lock_up_limit
	_phy_rtd_part_outl(0x1803bbe8, 1, 1, 0x1); //[DPHY fld flow]  reg_p0_g_fld_rstb
	_phy_rtd_part_outl(0x1803bbf4, 10, 3, 0x08); //[DPHY fld flow]  reg_p0_r_divide_num
	_phy_rtd_part_outl(0x1803bbf8, 11, 0, 0x0db); //[DPHY fld flow] reg_p0_r_lock_dn_limit
	_phy_rtd_part_outl(0x1803bbf8, 27, 16, 0x0e1); //[DPHY fld flow] reg_p0_r_lock_up_limit
	_phy_rtd_part_outl(0x1803bc00, 19, 8, 0x0db); //[DPHY fld flow] reg_p0_r_coarse_lock_dn_limit
	_phy_rtd_part_outl(0x1803bc00, 31, 20, 0x0e1); //[DPHY fld flow] reg_p0_r_coarse_lock_up_limit
	_phy_rtd_part_outl(0x1803bc04, 1, 1, 0x1); //[DPHY fld flow]  reg_p0_r_fld_rstb
	_phy_rtd_part_outl(0x1803bc10, 10, 3, 0x08); //[DPHY fld flow]  reg_p0_ck_divide_num
	_phy_rtd_part_outl(0x1803bc14, 11, 0, 0x0db); //[DPHY fld flow] reg_p0_ck_lock_dn_limit
	_phy_rtd_part_outl(0x1803bc14, 27, 16, 0x0e1); //[DPHY fld flow] reg_p0_ck_lock_up_limit
	_phy_rtd_part_outl(0x1803bc1c, 19, 8, 0x0db); //[DPHY fld flow] reg_p0_ck_coarse_lock_dn_limit
	_phy_rtd_part_outl(0x1803bc1c, 31, 20, 0x0e1); //[DPHY fld flow] reg_p0_ck_coarse_lock_up_limit
	_phy_rtd_part_outl(0x1803bc20, 1, 1, 0x1); //[DPHY fld flow]  reg_p0_ck_fld_rstb
}


void APHY_Init_Flow_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	_phy_rtd_part_outl(0x1803ba30, 0, 0, 1); //INOFF_EN =0 ->1 ck off
	ScalerTimer_DelayXms(1);
	_phy_rtd_part_outl(0x1803ba30, 0, 0, 0); //INOFF_EN =0 ->1 ck on
	// APHY init flow start
	_phy_rtd_part_outl(0x1803bc3c, 9, 9, 0x1); //B //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_phy_rtd_part_outl(0x1803bc48, 9, 9, 0x1); //G //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_phy_rtd_part_outl(0x1803bc54, 9, 9, 0x1); //R //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_phy_rtd_part_outl(0x1803bc60, 9, 9, 0x1); //K //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	ScalerTimer_DelayXms(1);
	_phy_rtd_part_outl(0x1803ba18, 1, 1, 0x0); //[APHY init flow]  REG_CK_LATCH Before PSM, latch the register value of PHY. 0->1: Latch
	_phy_rtd_part_outl(0x1803ba30, 6, 6, 0x0); //[APHY init flow]  CK-Lane FAST_SW_EN
	_phy_rtd_part_outl(0x1803ba30, 7, 7, 0x0); //[APHY init flow]  CK-Lane FAST_SW_DELAY_EN
	_phy_rtd_part_outl(0x1803ba38, 17, 17, 0x1); //[APHY init flow] 	CK-Lane BBPD_RSTB
	_phy_rtd_part_outl(0x1803ba40, 16, 16, APHY_Init_Flow_tmds_1[value_param].CK_19_10); //[APHY init flow]  CK-Lane DCDR_RSTB
	_phy_rtd_part_outl(0x1803ba40, 17, 17, APHY_Init_Flow_tmds_1[value_param].CK_19_11); //[APHY init flow]  CK-Lane PI_DIV rstb
	_phy_rtd_part_outl(0x1803ba40, 24, 24, 0x0); //[APHY init flow]  CK-Lane FKP_RSTB (FKP:Fast KP)
	_phy_rtd_part_outl(0x1803ba40, 28, 28, 0x0); //[APHY init flow]  CK-Lane FKT EN
	_phy_rtd_part_outl(0x1803ba44, 0, 0, 0x1); //[APHY init flow]  CK-Lane ACDR_RSTB_PFD
	_phy_rtd_part_outl(0x1803ba44, 1, 1, 0x0); //[APHY init flow]  CK-Lane ACDR_RSTB_CLK_FLD
	_phy_rtd_part_outl(0x1803ba44, 16, 16, 0x0); //[APHY init flow]  CK-Lane ACDR_RSTB_DIV_IQ
	_phy_rtd_part_outl(0x1803ba44, 16, 16, 0x1); //[APHY init flow]  CK-Lane ACDR_RSTB_DIV_IQ
	_phy_rtd_part_outl(0x1803ba44, 17, 17, 0x0); //[APHY init flow]  CK-Lane ACDR_RSTB_DIV_IQ
	_phy_rtd_part_outl(0x1803ba44, 17, 17, APHY_Init_Flow_tmds_1[value_param].ACDR_CK_3_19); //[APHY init flow]  CK-Lane ACDR_RSTB_DIV_IQ
	_phy_rtd_part_outl(0x1803ba44, 20, 20, 0x1); //[APHY para]  CK-Lane ACDR_SEL_0FR_1HR_DIV_IQ
	_phy_rtd_part_outl(0x1803ba48, 24, 24, 0x0); //[APHY init flow]  CK-Lane RSTB_BBPD_KP_KI
	_phy_rtd_part_outl(0x1803ba4c, 11, 11, 0x1); //[APHY init flow]  CK-Lane RSTB_PRESCALER
	_phy_rtd_part_outl(0x1803ba4c, 12, 12, 0x1); //[APHY init flow]  CK-Lane RSTB_M_DIV
	_phy_rtd_part_outl(0x1803ba50, 6, 6, 0x0); //[APHY init flow]  	B-Lane FAST_SW_EN
	_phy_rtd_part_outl(0x1803ba50, 7, 7, 0x0); //[APHY init flow]  	B-Lane FAST_SW_DELAY_EN
	_phy_rtd_part_outl(0x1803ba58, 17, 17, 0x1); //[APHY init flow] 	B-Lane BBPD_RSTB
	_phy_rtd_part_outl(0x1803ba60, 16, 16, 0x1); //[APHY init flow]  B-Lane DCDR_RSTB
	_phy_rtd_part_outl(0x1803ba60, 17, 17, 0x1); //[APHY init flow]  B-Lane PI_DIV rstb
	_phy_rtd_part_outl(0x1803ba60, 24, 24, 0x0); //[APHY init flow]  B-Lane FKP_RSTB (FKP:Fast KP)
	_phy_rtd_part_outl(0x1803ba60, 28, 28, 0x0); //[APHY init flow]  B-Lane FKT EN
	_phy_rtd_part_outl(0x1803ba64, 0, 0, 0x0); //[APHY init flow]  B-Lane ACDR_RSTB_PFD
	ScalerTimer_DelayXms(1);	
	_phy_rtd_part_outl(0x1803ba64, 0, 0, 0x1); //[APHY init flow]  B-Lane ACDR_RSTB_PFD
	_phy_rtd_part_outl(0x1803ba64, 1, 1, 0x0); //[APHY init flow]  B-Lane ACDR_RSTB_CLK_FLD
	_phy_rtd_part_outl(0x1803ba64, 16, 16, 0x0); //[APHY init flow]  B-Lane ACDR_RSTB_DIV_IQ
	_phy_rtd_part_outl(0x1803ba64, 16, 16, 0x1); //[APHY init flow]  B-Lane ACDR_RSTB_DIV_IQ
	_phy_rtd_part_outl(0x1803ba64, 17, 17, 0x0); //[APHY init flow]  B-Lane ACDR_RSTB_DIV_IQ
	_phy_rtd_part_outl(0x1803ba64, 17, 17, APHY_Init_Flow_tmds_1[value_param].ACDR_B_3_36); //[APHY init flow]  B-Lane ACDR_RSTB_DIV_IQ
	_phy_rtd_part_outl(0x1803ba64, 20, 20, 0x0); //[APHY init flow]  B-Lane RSTB_REF_FB_DIV
	_phy_rtd_part_outl(0x1803ba64, 20, 20, 0x1); //[APHY init flow]  B-Lane RSTB_REF_FB_DIV
	_phy_rtd_part_outl(0x1803ba68, 24, 24, 0x1); //[APHY init flow]  B-Lane RSTB_BBPD_KP_KI
	_phy_rtd_part_outl(0x1803ba6c, 11, 11, 0x0); //[APHY init flow]  B-Lane RSTB_PRESCALER
	_phy_rtd_part_outl(0x1803ba6c, 11, 11, 0x1); //[APHY init flow]  B-Lane RSTB_PRESCALER
	_phy_rtd_part_outl(0x1803ba6c, 12, 12, 0x0); //[APHY init flow]  B-Lane RSTB_M_DIV
	_phy_rtd_part_outl(0x1803ba6c, 12, 12, 0x1); //[APHY init flow]  B-Lane RSTB_M_DIV
	_phy_rtd_part_outl(0x1803ba70, 6, 6, 0x0); //[APHY init flow]  	G-Lane FAST_SW_EN
	_phy_rtd_part_outl(0x1803ba70, 7, 7, 0x0); //[APHY init flow]  	G-Lane FAST_SW_DELAY_EN
	_phy_rtd_part_outl(0x1803ba78, 17, 17, 0x1); //[APHY init flow] 	G-Lane BBPD_RSTB
	_phy_rtd_part_outl(0x1803ba80, 16, 16, 0x1); //[APHY init flow]  G-Lane DCDR_RSTB
	_phy_rtd_part_outl(0x1803ba80, 17, 17, 0x1); //[APHY init flow]  G-Lane PI_DIV rstb
	_phy_rtd_part_outl(0x1803ba80, 24, 24, 0x0); //[APHY init flow]  G-Lane FKP_RSTB (FKP:Fast KP)
	_phy_rtd_part_outl(0x1803ba80, 28, 28, 0x0); //[APHY init flow]  G-Lane FKT EN
	_phy_rtd_part_outl(0x1803ba84, 0, 0, 0x0); //[APHY init flow]  G-Lane ACDR_RSTB_PFD
	ScalerTimer_DelayXms(1);	
	_phy_rtd_part_outl(0x1803ba84, 0, 0, 0x1); //[APHY init flow]  G-Lane ACDR_RSTB_PFD
	_phy_rtd_part_outl(0x1803ba84, 1, 1, 0x0); //[APHY init flow]  G-Lane ACDR_RSTB_CLK_FLD
	_phy_rtd_part_outl(0x1803ba84, 16, 16, 0x0); //[APHY init flow]  G-Lane ACDR_RSTB_DIV_IQ
	_phy_rtd_part_outl(0x1803ba84, 16, 16, 0x1); //[APHY init flow]  G-Lane ACDR_RSTB_DIV_IQ
	_phy_rtd_part_outl(0x1803ba84, 17, 17, 0x0); //[APHY init flow]  G-Lane ACDR_RSTB_DIV_IQ
	_phy_rtd_part_outl(0x1803ba84, 17, 17, APHY_Init_Flow_tmds_1[value_param].ACDR_G_3_56); //[APHY init flow]  G-Lane ACDR_RSTB_DIV_IQ
	_phy_rtd_part_outl(0x1803ba84, 20, 20, 0x0); //[APHY init flow]  G-Lane RSTB_REF_FB_DIV
	_phy_rtd_part_outl(0x1803ba84, 20, 20, 0x1); //[APHY init flow]  G-Lane RSTB_REF_FB_DIV
	_phy_rtd_part_outl(0x1803ba88, 24, 24, 0x1); //[APHY init flow]  G-Lane RSTB_BBPD_KP_KI
	_phy_rtd_part_outl(0x1803ba8c, 11, 11, 0x0); //[APHY init flow]  G-Lane RSTB_PRESCALER
	_phy_rtd_part_outl(0x1803ba8c, 11, 11, 0x1); //[APHY init flow]  G-Lane RSTB_PRESCALER
	_phy_rtd_part_outl(0x1803ba8c, 12, 12, 0x0); //[APHY init flow]  G-Lane RSTB_M_DIV
	_phy_rtd_part_outl(0x1803ba8c, 12, 12, 0x1); //[APHY init flow]  G-Lane RSTB_M_DIV
	_phy_rtd_part_outl(0x1803ba90, 6, 6, 0x0); //[APHY init flow]  	R-Lane FAST_SW_EN
	_phy_rtd_part_outl(0x1803ba90, 7, 7, 0x0); //[APHY init flow]  	R-Lane FAST_SW_DELAY_EN
	_phy_rtd_part_outl(0x1803ba98, 17, 17, 0x1); //[APHY init flow] 	R-Lane BBPD_RSTB
	_phy_rtd_part_outl(0x1803baa0, 16, 16, 0x1); //[APHY init flow]  R-Lane DCDR_RSTB
	_phy_rtd_part_outl(0x1803baa0, 17, 17, 0x1); //[APHY init flow]  R-Lane PI_DIV rstb
	_phy_rtd_part_outl(0x1803baa0, 24, 24, 0x0); //[APHY init flow]  R-Lane FKP_RSTB (FKP:Fast KP)
	_phy_rtd_part_outl(0x1803baa0, 28, 28, 0x0); //[APHY init flow]  R-Lane FKT EN
	_phy_rtd_part_outl(0x1803baa4, 0, 0, 0x0); //[APHY init flow]  R-Lane ACDR_RSTB_PFD
	ScalerTimer_DelayXms(1);	
	_phy_rtd_part_outl(0x1803baa4, 0, 0, 0x1); //[APHY init flow]  R-Lane ACDR_RSTB_PFD
	_phy_rtd_part_outl(0x1803baa4, 1, 1, 0x0); //[APHY init flow]  R-Lane ACDR_RSTB_CLK_FLD
	_phy_rtd_part_outl(0x1803baa4, 16, 16, 0x0); //[APHY init flow]  R-Lane ACDR_RSTB_DIV_IQ
	_phy_rtd_part_outl(0x1803baa4, 16, 16, 0x1); //[APHY init flow]  R-Lane ACDR_RSTB_DIV_IQ
	_phy_rtd_part_outl(0x1803baa4, 17, 17, 0x0); //[APHY init flow]  R-Lane ACDR_RSTB_DIV_IQ
	_phy_rtd_part_outl(0x1803baa4, 17, 17, APHY_Init_Flow_tmds_1[value_param].ACDR_R_3_76); //[APHY init flow]  R-Lane ACDR_RSTB_DIV_IQ
	_phy_rtd_part_outl(0x1803baa4, 20, 20, 0x0); //[APHY init flow]  R-Lane RSTB_REF_FB_DIV
	_phy_rtd_part_outl(0x1803baa4, 20, 20, 0x1); //[APHY init flow]  R-Lane RSTB_REF_FB_DIV
	_phy_rtd_part_outl(0x1803baa8, 24, 24, 0x1); //[APHY init flow]  R-Lane RSTB_BBPD_KP_KI
	_phy_rtd_part_outl(0x1803baac, 11, 11, 0x0); //[APHY init flow]  R-Lane RSTB_PRESCALER
	_phy_rtd_part_outl(0x1803baac, 11, 11, 0x1); //[APHY init flow]  R-Lane RSTB_PRESCALER
	_phy_rtd_part_outl(0x1803baac, 12, 12, 0x0); //[APHY init flow]  R-Lane RSTB_M_DIV
	_phy_rtd_part_outl(0x1803baac, 12, 12, 0x1); //[APHY init flow]  R-Lane RSTB_M_DIV
	_phy_rtd_part_outl(0x1803bc3c, 9, 9, 0x0); //B //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_phy_rtd_part_outl(0x1803bc48, 9, 9, 0x0); //G //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_phy_rtd_part_outl(0x1803bc54, 9, 9, 0x0); //R //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_phy_rtd_part_outl(0x1803bc60, 9, 9, 0x0); //K //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	//APHY acdr flow start
	_phy_rtd_part_outl(0x1803bafc, 0, 0, 0x0); //[APHY acdr flow]  P0_ACDR_B_ACDR_EN_AFN
	_phy_rtd_part_outl(0x1803bafc, 1, 1, 0x0); //[APHY acdr flow]  P0_ACDR_G_ACDR_EN_AFN
	_phy_rtd_part_outl(0x1803bafc, 2, 2, 0x0); //[APHY acdr flow]  P0_ACDR_R_ACDR_EN_AFN
	_phy_rtd_part_outl(0x1803bafc, 3, 3, 0x0); //[APHY acdr flow]  P0_ACDR_CK_ACDR_EN_AFN
	_phy_rtd_part_outl(0x1803bafc, 4, 4, 0x1); //[APHY acdr flow]  P0_B_DEMUX_RSTB
	_phy_rtd_part_outl(0x1803bafc, 5, 5, 0x1); //[APHY acdr flow]  P0_G_DEMUX_RSTB
	_phy_rtd_part_outl(0x1803bafc, 6, 6, 0x1); //[APHY acdr flow]  P0_R_DEMUX_RSTB
	_phy_rtd_part_outl(0x1803bafc, 7, 7, APHY_Init_Flow_tmds_1[value_param].P0_CK_DEMUX_RSTB_96); //[APHY acdr flow]  P0_CK_DEMUX_RSTB
	_phy_rtd_part_outl(0x1803ba3c, 7, 7, 0x0); //[APHY acdr flow]  		CK-Lane DFE_ADAPT_EN
	_phy_rtd_part_outl(0x1803ba5c, 7, 7, 0x1); //[APHY acdr flow]  		B-Lane DFE_ADAPT_EN
	_phy_rtd_part_outl(0x1803ba7c, 7, 7, 0x1); //[APHY acdr flow]  		G-Lane DFE_ADAPT_EN
	_phy_rtd_part_outl(0x1803ba9c, 7, 7, 0x1); //[APHY acdr flow]  R-Lane DFE_ADAPT_EN
}

void DCDR_settings_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	_phy_rtd_part_outl(0x1803ba58, 17, 17, 0); //BBPD_rstb
	_phy_rtd_part_outl(0x1803ba78, 17, 17, 0); //BBPD_rstb
	_phy_rtd_part_outl(0x1803ba98, 17, 17, 0); //BBPD_rstb
	_phy_rtd_part_outl(0x1803ba58, 17, 17, 1); //BBPD_rstb
	_phy_rtd_part_outl(0x1803ba78, 17, 17, 1); //BBPD_rstb
	_phy_rtd_part_outl(0x1803ba98, 17, 17, 1); //BBPD_rstb
	//#====================================== CMU RESET
	_phy_rtd_part_outl(0x1803bafc, 4, 4, 0x0);
	_phy_rtd_part_outl(0x1803bafc, 5, 5, 0x0);
	_phy_rtd_part_outl(0x1803bafc, 6, 6, 0x0);
	_phy_rtd_part_outl(0x1803bafc, 4, 4, 0x1);
	_phy_rtd_part_outl(0x1803bafc, 5, 5, 0x1);
	_phy_rtd_part_outl(0x1803bafc, 6, 6, 0x1);
	//#====================================== phase I
	_phy_rtd_part_outl(0x1803bc20, 27, 25, 4); //Mark2_Modify; CK-Lane SEL_BAND_V2I
	//ScalerTimer_DelayXms(5);000
	//#===================================== phase IV : set pow_save_bypass == 0
	//#====================================== cdr reset release
}

void DCDR_settings_tmds_2_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	_phy_rtd_part_outl(0x1803bb88, 4, 4, 0x0); //DCDR 5.94G, reg_p0_b/g/r_clk_div2_en
	_phy_rtd_part_outl(0x1803bb88, 12, 12, 0x0); //DCDR 5.94G, reg_p0_b/g/r_clk_div2_en
	_phy_rtd_part_outl(0x1803bb88, 20, 20, 0x0); //DCDR 5.94G, reg_p0_b/g/r_clk_div2_en
	_phy_rtd_part_outl(0x1803bb88, 28, 28, 0x0); //DCDR 5.94G, reg_p0_b/g/r_clk_div2_en
	_phy_rtd_part_outl(0x1803bb88, 4, 4, 0x1); //DCDR 5.94G, reg_p0_b/g/r_clk_div2_en
	_phy_rtd_part_outl(0x1803bb88, 12, 12, 0x1); //DCDR 5.94G, reg_p0_b/g/r_clk_div2_en
	_phy_rtd_part_outl(0x1803bb88, 20, 20, 0x1); //DCDR 5.94G, reg_p0_b/g/r_clk_div2_en
	_phy_rtd_part_outl(0x1803bb88, 28, 28, 0x1); //DCDR 5.94G, reg_p0_b/g/r_clk_div2_en
	_phy_rtd_part_outl(0x1803bb00, 0, 0, 0x0);
	_phy_rtd_part_outl(0x1803bb00, 1, 1, 0x0);
	_phy_rtd_part_outl(0x1803bb00, 2, 2, 0x0);
	_phy_rtd_part_outl(0x1803bb00, 3, 3, 0x0);
	_phy_rtd_part_outl(0x1803bb00, 4, 4, 0x0);
	_phy_rtd_part_outl(0x1803bb00, 5, 5, 0x0);
	_phy_rtd_part_outl(0x1803bb00, 6, 6, 0x0);
	_phy_rtd_part_outl(0x1803bb00, 7, 7, 0x0);
	_phy_rtd_part_outl(0x1803bb00, 0, 0, 0x1);
	_phy_rtd_part_outl(0x1803bb00, 1, 1, 0x1);
	_phy_rtd_part_outl(0x1803bb00, 2, 2, 0x1);
	_phy_rtd_part_outl(0x1803bb00, 3, 3, 0x1);
	_phy_rtd_part_outl(0x1803bb00, 4, 4, 0x1);
	_phy_rtd_part_outl(0x1803bb00, 5, 5, 0x1);
	_phy_rtd_part_outl(0x1803bb00, 6, 6, 0x1);
	_phy_rtd_part_outl(0x1803bb00, 7, 7, 0x1);
}

void Koffset_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	_phy_rtd_part_outl(0x1803bcb0, 15, 15, 0); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 14, 14, 0); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 13, 13, 0); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 12, 12, 0); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 11, 11, 0); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 10, 10, 0); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 9, 9, 0); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 8, 8, 0); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bc04, 0, 0, 0); //R finetunstart off
	_phy_rtd_part_outl(0x1803bbe8, 0, 0, 0); //G finetunstart off
	_phy_rtd_part_outl(0x1803bbc8, 0, 0, 0); //B finetunstart off
	_phy_rtd_part_outl(0x1803bc20, 0, 0, 0); //CK finetunstart off
	//========================================= STEP1 Data_Even KOFF =================================================
	_phy_rtd_part_outl(0x1803ba58, 28, 28, 1); //#B  DFE_TAP0_ICOM_EN=1
	_phy_rtd_part_outl(0x1803ba5c, 6, 3, 0); //#B  DFE_TAP_EN[4:1] = 0
	_phy_rtd_part_outl(0x1803ba78, 28, 28, 1); //#G  DFE_TAP0_ICOM_EN=1
	_phy_rtd_part_outl(0x1803ba7c, 6, 3, 0); //#G  DFE_TAP_EN[4:1] = 0
	_phy_rtd_part_outl(0x1803ba98, 28, 28, 1); //#R  DFE_TAP0_ICOM_EN=1
	_phy_rtd_part_outl(0x1803ba9c, 6, 3, 0); //#R  DFE_TAP_EN[4:1] = 0
	_phy_rtd_outl(0x1803bb44,0x0282cf00); //B_KOFF rstn,  //[26:20] timout = 0101000, [19:14] divisor = 001000, [11:8] delay count = 11
	_phy_rtd_outl(0x1803bb54,0x0282cf00); //G_KOFF rstn
	_phy_rtd_outl(0x1803bb64, 0x0282cf00); //R_KOFF rstn
	_phy_rtd_part_outl(0x1803bb44, 0, 0, Koffset_tmds_1[value_param].b_offset_rst_n_23); //Release B_KOFF rstn
	_phy_rtd_part_outl(0x1803bb54, 0, 0, Koffset_tmds_1[value_param].g_offset_rst_n_24); //Release G_KOFF rstn
	_phy_rtd_part_outl(0x1803bb64, 0, 0, Koffset_tmds_1[value_param].r_offset_rst_n_25); //Release R_KOFF rstn
	//===B LANE KOFF
	_phy_rtd_part_outl(0x1803bb4c, 28, 28, 0); //manaul_do=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803bb4c, 18, 18, 0); //manaul_de=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803bb50, 28, 28, 0); //manaul_eq=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803bb50, 18, 18, 0); //manaul_eo=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803bb50, 8, 8, 0); //manaul_ee=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803bb48, 18, 18, 0); //manaul_opo=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803bb48, 8, 8, 0); //manaul_ope=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803ba50, 0, 0, 1); //INOFF_EN =0 ->1
	_phy_rtd_part_outl(0x1803ba58, 0, 0, 0); //REG_B_9[0] POW_CMFB_LEQ 1->0
	_phy_rtd_part_outl(0x1803ba54, 22, 22, Koffset_tmds_1[value_param].B_7_36); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  1->0
	_phy_rtd_part_outl(0x1803ba54, 23, 23, 0); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  1->0
	_phy_rtd_part_outl(0x1803ba54, 24, 24, 0); //REG_B_8[0] POW_LEQ:  1->0
	//B-Lane z0_ok
	_phy_rtd_part_outl(0x1803bb50, 19, 19, 1); //[19]z0_ok_eo=1
	_phy_rtd_part_outl(0x1803bb50, 9, 9, 1); //[9] z0_ok_ee=1
	_phy_rtd_part_outl(0x1803bb4c, 29, 29, 1); //[29]z0_ok_do=1
	_phy_rtd_part_outl(0x1803bb4c, 19, 19, 1); //[19]z0_ok_de=1
	_phy_rtd_part_outl(0x1803bb48, 19, 19, 1); //[19] z0_ok_opo=1
	_phy_rtd_part_outl(0x1803bb48, 9, 9, 1); //[9] z0_ok_ope=1
	_phy_rtd_part_outl(0x1803bb4c, 21, 21, 0); //[20]offset_pc_do=1
	_phy_rtd_part_outl(0x1803bb4c, 11, 11, 0); //[20]offset_pc_de=1
	_phy_rtd_part_outl(0x1803bb50, 1, 1, 0); //[20]offset_pc_ee=1
	_phy_rtd_part_outl(0x1803bb50, 11, 11, 0); //[20]offset_pc_eo=1
	_phy_rtd_part_outl(0x1803bb50, 21, 21, 0); //[21]offfset_pc_eq=1
	_phy_rtd_part_outl(0x1803bb48, 1, 1, 0); //[1]offset_pc_ope=0
	_phy_rtd_part_outl(0x1803bb48, 11, 11, 0); //[11]offset_pc_opo=0
	//Enable B-Lane DCVS KOFF
	_phy_rtd_part_outl(0x1803bb4c, 10, 10, Koffset_tmds_1[value_param].b_offset_en_de_54); //[10]offset_en_de=1
	_phy_rtd_part_outl(0x1803bb4c, 20, 20, Koffset_tmds_1[value_param].b_offset_en_do_55); //[20]offset_en_do=1
	_phy_rtd_part_outl(0x1803bb50, 0, 0, Koffset_tmds_1[value_param].b_offset_en_ee_56); //[20]offset_en_ee=1
	_phy_rtd_part_outl(0x1803bb50, 10, 10, Koffset_tmds_1[value_param].b_offset_en_eo_57); //[20]offset_en_eo=1
	_phy_rtd_part_outl(0x1803bb48, 10, 10, Koffset_tmds_1[value_param].b_offset_en_opo_58); //[10]offset_en_opo=1
	_phy_rtd_part_outl(0x1803bb48, 0, 0, Koffset_tmds_1[value_param].b_offset_en_ope_59); //[0]offset_en_ope=1
	ScalerTimer_DelayXms(1);
	//Check Offset cal. OK Check
	_phy_rtd_part_outl(0x1803bc7c, 5, 0, 0x8); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_phy_rtd_part_outl(0x1803bb44, 7, 4, 0x9); //reg_p0_b_offset_coef_sel
	HDMI_EMG("DE Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 27, 27));
	HDMI_EMG("DO Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 26, 26));
	HDMI_EMG("EE Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 25, 25));
	HDMI_EMG("EOOffset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 24, 24));
	HDMI_EMG("DATA OFFSET OK = %d\n",_phy_rtd_part_inl(0x1803bb44, 28, 28));
	HDMI_EMG("DATA OFFSET OK = %d\n",_phy_rtd_part_inl(0x1803bb44, 29, 29));
	//Disable B-Lane DCVS KOFF
	_phy_rtd_part_outl(0x1803bb4c, 10, 10, 0); //[10]offset_en_de=1->0
	_phy_rtd_part_outl(0x1803bb4c, 20, 20, 0); //[20]offset_en_do=1->0
	_phy_rtd_part_outl(0x1803bb50, 0, 0, 0); //[20]offset_en_ee=1->0
	_phy_rtd_part_outl(0x1803bb50, 10, 10, 0); //[20]offset_en_eo=1->0
	_phy_rtd_part_outl(0x1803bb48, 10, 10, 0); //[10]offset_en_opo=1->0
	_phy_rtd_part_outl(0x1803bb48, 0, 0, 0); //[0]offset_en_ope=1->0
	//B-Lane Koffset result READ  *******************************************************************
	_phy_rtd_part_outl(0x1803bb44, 7, 4, 0); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("Data_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 4, 0));
	_phy_rtd_part_outl(0x1803bb44, 7, 4, 1); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("Data_Odd_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 4, 0));
	_phy_rtd_part_outl(0x1803bb44, 7, 4, 2); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("Edge_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 4, 0));
	_phy_rtd_part_outl(0x1803bb44, 7, 4, 3); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("Edge_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 4, 0));
	_phy_rtd_part_outl(0x1803bb44, 7, 4, 4); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("OFFP_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 4, 0));
	_phy_rtd_part_outl(0x1803bb44, 7, 4, 5); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("OFFP_Odd_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 4, 0));
	//===G LANE KOFF
	_phy_rtd_part_outl(0x1803bb5c, 28, 28, 0); //manaul_do=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803bb5c, 18, 18, 0); //manaul_de=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803bb60, 28, 28, 0); //manaul_eq=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803bb60, 18, 18, 0); //manaul_eo=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803bb60, 8, 8, 0); //manaul_ee=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803bb58, 18, 18, 0); //manaul_opo=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803bb58, 8, 8, 0); //manaul_ope=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803ba70, 0, 0, 1); //INOFF_EN =0 ->1
	_phy_rtd_part_outl(0x1803ba78, 0, 0, 0); //REG_B_9[0] POW_CMFB_LEQ 1->0
	_phy_rtd_part_outl(0x1803ba74, 22, 22, Koffset_tmds_1[value_param].G_7_99); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  1->0
	_phy_rtd_part_outl(0x1803ba74, 23, 23, 0); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  1->0
	_phy_rtd_part_outl(0x1803ba74, 24, 24, 0); //REG_B_8[0] POW_LEQ:  1->0
	//G-Lane z0_ok
	_phy_rtd_part_outl(0x1803bb60, 19, 19, 1); //[19]z0_ok_eo=1
	_phy_rtd_part_outl(0x1803bb60, 9, 9, 1); //[9] z0_ok_ee=1
	_phy_rtd_part_outl(0x1803bb5c, 29, 29, 1); //[29]z0_ok_do=1
	_phy_rtd_part_outl(0x1803bb5c, 19, 19, 1); //[19]z0_ok_de=1
	_phy_rtd_part_outl(0x1803bb58, 19, 19, 1); //[19] z0_ok_opo=1
	_phy_rtd_part_outl(0x1803bb58, 9, 9, 1); //[9] z0_ok_ope=1
	_phy_rtd_part_outl(0x1803bb5c, 21, 21, 0); //[20]offset_pc_do=1
	_phy_rtd_part_outl(0x1803bb5c, 11, 11, 0); //[20]offset_pc_de=1
	_phy_rtd_part_outl(0x1803bb60, 1, 1, 0); //[20]offset_pc_ee=1
	_phy_rtd_part_outl(0x1803bb60, 11, 11, 0); //[20]offset_pc_eo=1
	_phy_rtd_part_outl(0x1803bb60, 21, 21, 0); //[21]offfset_pc_eq=1
	_phy_rtd_part_outl(0x1803bb58, 1, 1, 0); //[1]offset_pc_ope=0
	_phy_rtd_part_outl(0x1803bb58, 11, 11, 0); //[11]offset_pc_opo=0
	//Enable G-Lane DCVS KOFF
	_phy_rtd_part_outl(0x1803bb5c, 10, 10, Koffset_tmds_1[value_param].g_offset_en_de_117); //[10]offset_en_de=1
	_phy_rtd_part_outl(0x1803bb5c, 20, 20, Koffset_tmds_1[value_param].g_offset_en_do_118); //[20]offset_en_do=1
	_phy_rtd_part_outl(0x1803bb60, 0, 0, Koffset_tmds_1[value_param].g_offset_en_ee_119); //[20]offset_en_ee=1
	_phy_rtd_part_outl(0x1803bb60, 10, 10, Koffset_tmds_1[value_param].g_offset_en_eo_120); //[20]offset_en_eo=1
	_phy_rtd_part_outl(0x1803bb58, 10, 10, Koffset_tmds_1[value_param].g_offset_en_opo_121); //[10]offset_en_opo=1
	_phy_rtd_part_outl(0x1803bb58, 0, 0, Koffset_tmds_1[value_param].g_offset_en_ope_122); //[0]offset_en_ope=1
	ScalerTimer_DelayXms(1);
	//Check Offset cal. OK Check
	_phy_rtd_part_outl(0x1803bc7c, 5, 0, 0x8); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_phy_rtd_part_outl(0x1803bb54, 7, 4, 0x9); //reg_p0_g_offset_coef_sel
	HDMI_EMG("DE Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 27, 27));
	HDMI_EMG("DO Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 26, 26));
	HDMI_EMG("EE Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 25, 25));
	HDMI_EMG("EOOffset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 24, 24));
	HDMI_EMG("DATA OFFSET OK = %d\n",_phy_rtd_part_inl(0x1803bb54, 28, 28));
	HDMI_EMG("DATA OFFSET OK = %d\n",_phy_rtd_part_inl(0x1803bb54, 29, 29));
	//Disable G-Lane DCVS KOFF
	_phy_rtd_part_outl(0x1803bb5c, 10, 10, 0); //[10]offset_en_de=1->0
	_phy_rtd_part_outl(0x1803bb5c, 20, 20, 0); //[20]offset_en_do=1->0
	_phy_rtd_part_outl(0x1803bb60, 0, 0, 0); //[20]offset_en_ee=1->0
	_phy_rtd_part_outl(0x1803bb60, 10, 10, 0); //[20]offset_en_eo=1->0
	_phy_rtd_part_outl(0x1803bb58, 10, 10, 0); //[10]offset_en_opo=1->0
	_phy_rtd_part_outl(0x1803bb58, 0, 0, 0); //[0]offset_en_ope=1->0
	//G-Lane Koffset result READ  *******************************************************************
	_phy_rtd_part_outl(0x1803bb54, 7, 4, 0); //reg_p0_g_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("Data_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 12, 8));
	_phy_rtd_part_outl(0x1803bb54, 7, 4, 1); //reg_p0_g_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("Data_Odd_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 12, 8));
	_phy_rtd_part_outl(0x1803bb54, 7, 4, 2); //reg_p0_g_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("Edge_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 12, 8));
	_phy_rtd_part_outl(0x1803bb54, 7, 4, 3); //reg_p0_g_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("Edge_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 12, 8));
	_phy_rtd_part_outl(0x1803bb54, 7, 4, 4); //reg_p0_g_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("OFFP_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 12, 8));
	_phy_rtd_part_outl(0x1803bb54, 7, 4, 5); //reg_p0_g_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("OFFP_Odd_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 12, 8));
	//===R LANE KOFF
	_phy_rtd_part_outl(0x1803bb6c, 28, 28, 0); //manaul_do=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803bb6c, 18, 18, 0); //manaul_de=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803bb70, 28, 28, 0); //manaul_eq=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803bb70, 18, 18, 0); //manaul_eo=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803bb70, 8, 8, 0); //manaul_ee=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803bb68, 18, 18, 0); //manaul_opo=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803bb68, 8, 8, 0); //manaul_ope=0, 0:Manual; 1:Auto
	_phy_rtd_part_outl(0x1803ba90, 0, 0, 1); //INOFF_EN =0 ->1
	_phy_rtd_part_outl(0x1803ba98, 0, 0, 0); //REG_B_9[0] POW_CMFB_LEQ 1->0
	_phy_rtd_part_outl(0x1803ba94, 22, 22, Koffset_tmds_1[value_param].R_7_162); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  1->0
	_phy_rtd_part_outl(0x1803ba94, 23, 23, 0); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  1->0
	_phy_rtd_part_outl(0x1803ba94, 24, 24, 0); //REG_B_8[0] POW_LEQ:  1->0
	//R-Lane z0_ok
	_phy_rtd_part_outl(0x1803bb70, 19, 19, 1); //[19]z0_ok_eo=1
	_phy_rtd_part_outl(0x1803bb70, 9, 9, 1); //[9] z0_ok_ee=1
	_phy_rtd_part_outl(0x1803bb6c, 29, 29, 1); //[29]z0_ok_do=1
	_phy_rtd_part_outl(0x1803bb6c, 19, 19, 1); //[19]z0_ok_de=1
	_phy_rtd_part_outl(0x1803bb68, 19, 19, 1); //[19] z0_ok_opo=1
	_phy_rtd_part_outl(0x1803bb68, 9, 9, 1); //[9] z0_ok_ope=1
	_phy_rtd_part_outl(0x1803bb6c, 21, 21, 0); //[20]offset_pc_do=1
	_phy_rtd_part_outl(0x1803bb6c, 11, 11, 0); //[20]offset_pc_de=1
	_phy_rtd_part_outl(0x1803bb70, 1, 1, 0); //[20]offset_pc_ee=1
	_phy_rtd_part_outl(0x1803bb70, 11, 11, 0); //[20]offset_pc_eo=1
	_phy_rtd_part_outl(0x1803bb70, 21, 21, 0); //[21]offfset_pc_eq=1
	_phy_rtd_part_outl(0x1803bb68, 1, 1, 0); //[1]offset_pc_ope=0
	_phy_rtd_part_outl(0x1803bb68, 11, 11, 0); //[11]offset_pc_opo=0
	//Enable R-Lane DCVS KOFF
	_phy_rtd_part_outl(0x1803bb6c, 10, 10, Koffset_tmds_1[value_param].r_offset_en_de_180); //[10]offset_en_de=1
	_phy_rtd_part_outl(0x1803bb6c, 20, 20, Koffset_tmds_1[value_param].r_offset_en_do_181); //[20]offset_en_do=1
	_phy_rtd_part_outl(0x1803bb70, 0, 0, Koffset_tmds_1[value_param].r_offset_en_ee_182); //[20]offset_en_ee=1
	_phy_rtd_part_outl(0x1803bb70, 10, 10, Koffset_tmds_1[value_param].r_offset_en_eo_183); //[20]offset_en_eo=1
	_phy_rtd_part_outl(0x1803bb68, 10, 10, Koffset_tmds_1[value_param].r_offset_en_opo_184); //[10]offset_en_opo=1
	_phy_rtd_part_outl(0x1803bb68, 0, 0, Koffset_tmds_1[value_param].r_offset_en_ope_185); //[0]offset_en_ope=1
	ScalerTimer_DelayXms(1);
	//Check Offset cal. OK Check
	_phy_rtd_part_outl(0x1803bc7c, 5, 0, 0x8); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_phy_rtd_part_outl(0x1803bb64, 7, 4, 0x9); //reg_p0_r_offset_coef_sel
	HDMI_EMG("DE Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 27, 27));
	HDMI_EMG("DO Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 26, 26));
	HDMI_EMG("EE Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 25, 25));
	HDMI_EMG("EOOffset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 24, 24));
	HDMI_EMG("DATA OFFSET OK = %d\n",_phy_rtd_part_inl(0x1803bb64, 28, 28));
	HDMI_EMG("DATA OFFSET OK = %d\n",_phy_rtd_part_inl(0x1803bb64, 29, 29));
	//Disable R-Lane DCVS KOFF
	_phy_rtd_part_outl(0x1803bb6c, 10, 10, 0); //[10]offset_en_de=1->0
	_phy_rtd_part_outl(0x1803bb6c, 20, 20, 0); //[20]offset_en_do=1->0
	_phy_rtd_part_outl(0x1803bb70, 0, 0, 0); //[20]offset_en_ee=1->0
	_phy_rtd_part_outl(0x1803bb70, 10, 10, 0); //[20]offset_en_eo=1->0
	_phy_rtd_part_outl(0x1803bb68, 10, 10, 0); //[10]offset_en_opo=1->0
	_phy_rtd_part_outl(0x1803bb68, 0, 0, 0); //[0]offset_en_ope=1->0
	//R-Lane Koffset result READ  *******************************************************************
	_phy_rtd_part_outl(0x1803bb64, 7, 4, 0); //reg_p0_r_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("Data_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 20, 16));
	_phy_rtd_part_outl(0x1803bb64, 7, 4, 1); //reg_p0_r_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("Data_Odd_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 20, 16));
	_phy_rtd_part_outl(0x1803bb64, 7, 4, 2); //reg_p0_r_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("Edge_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 20, 16));
	_phy_rtd_part_outl(0x1803bb64, 7, 4, 3); //reg_p0_r_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("Edge_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 20, 16));
	_phy_rtd_part_outl(0x1803bb64, 7, 4, 4); //reg_p0_r_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("OFFP_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 20, 16));
	_phy_rtd_part_outl(0x1803bb64, 7, 4, 5); //reg_p0_r_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("OFFP_Odd_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 20, 16));
	//========================================= STEP2 LEQ KOFF =================================================
	_phy_rtd_part_outl(0x1803ba54, 22, 22, Koffset_tmds_1[value_param].B_7_216); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  0->1
	_phy_rtd_part_outl(0x1803ba54, 23, 23, 1); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  0->1
	_phy_rtd_part_outl(0x1803ba54, 24, 24, Koffset_tmds_1[value_param].B_8_218); //REG_B_8[0] POW_LEQ:  0->1
	_phy_rtd_part_outl(0x1803ba58, 0, 0, 1); //REG_B_9[0] POW_CMFB_LEQ 0->1
	_phy_rtd_part_outl(0x1803bb50, 19, 19, 0); //[19]z0_ok_eo=0
	_phy_rtd_part_outl(0x1803bb50, 9, 9, 0); //[9] z0_ok_ee=0
	_phy_rtd_part_outl(0x1803bb4c, 29, 29, 0); //[19]z0_ok_do=0
	_phy_rtd_part_outl(0x1803bb50, 20, 20, Koffset_tmds_1[value_param].b_offset_en_eq_223); //[21]offset_en_eq=0->1
	ScalerTimer_DelayXms(1);
	HDMI_EMG("p0_b_offset_eq_ok_ro	EQ OFFSET OK = %d\n",_phy_rtd_part_inl(0x1803bb44, 30, 30));
	_phy_rtd_part_outl(0x1803bb50, 20, 20, 0); //[20]offset_en_eq=0
	//B-Lane LEQ Offset Result
	_phy_rtd_part_outl(0x1803bb44, 7, 4, 8); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_EMG("EQ_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 4, 0));
	//===G LANE KOFF LEQ
	_phy_rtd_part_outl(0x1803ba74, 22, 22, Koffset_tmds_1[value_param].G_7_230); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  0->1
	_phy_rtd_part_outl(0x1803ba74, 23, 23, 1); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  0->1
	_phy_rtd_part_outl(0x1803ba74, 24, 24, Koffset_tmds_1[value_param].G_8_232); //REG_B_8[0] POW_LEQ:  0->1
	_phy_rtd_part_outl(0x1803ba78, 0, 0, 1); //REG_B_9[0] POW_CMFB_LEQ 0->1
	_phy_rtd_part_outl(0x1803bb60, 19, 19, 0); //[19]z0_ok_eo=0
	_phy_rtd_part_outl(0x1803bb60, 9, 9, 0); //[9] z0_ok_ee=0
	_phy_rtd_part_outl(0x1803bb5c, 29, 29, 0); //[19]z0_ok_do=0
	_phy_rtd_part_outl(0x1803bb60, 20, 20, Koffset_tmds_1[value_param].g_offset_en_eq_237); //[21]offset_en_eq=0->1
	ScalerTimer_DelayXms(1);
	HDMI_EMG("p0_g_offset_eq_ok_ro EQ OFFSET OK = %d\n",_phy_rtd_part_inl(0x1803bb54, 30, 30));
	_phy_rtd_part_outl(0x1803bb60, 20, 20, 0); //[20]offset_en_eq=0
	//G-Lane LEQ Offset Result
	_phy_rtd_part_outl(0x1803bb54, 7, 4, 8); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_EMG("EQ_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 12, 8));
	//===R LANE KOFF LEQ
	_phy_rtd_part_outl(0x1803ba94, 22, 22, Koffset_tmds_1[value_param].R_7_244); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  0->1
	_phy_rtd_part_outl(0x1803ba94, 23, 23, 1); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  0->1
	_phy_rtd_part_outl(0x1803ba94, 24, 24, Koffset_tmds_1[value_param].R_8_246); //REG_B_8[0] POW_LEQ:  0->1
	_phy_rtd_part_outl(0x1803ba98, 0, 0, 1); //REG_B_9[0] POW_CMFB_LEQ 0->1
	_phy_rtd_part_outl(0x1803bb70, 19, 19, 0); //[19]z0_ok_eo=0
	_phy_rtd_part_outl(0x1803bb70, 9, 9, 0); //[9] z0_ok_ee=0
	_phy_rtd_part_outl(0x1803bb6c, 29, 29, 0); //[19]z0_ok_do=0
	_phy_rtd_part_outl(0x1803bb70, 20, 20, Koffset_tmds_1[value_param].r_offset_en_eq_251); //[21]offset_en_eq=0->1
	ScalerTimer_DelayXms(1);
	HDMI_EMG("p0_r_offset_eq_ok_ro EQ OFFSET OK = %d\n",_phy_rtd_part_inl(0x1803bb64, 30, 30));
	_phy_rtd_part_outl(0x1803bb70, 20, 20, 0); //[20]offset_en_eq=0
	//R-Lane LEQ Offset Result
	_phy_rtd_part_outl(0x1803bb64, 7, 4, 8); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_EMG("EQ_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 20, 16));
	//========================================= STEP3 All DCVS KOFF ==
	//B-LANE
	_phy_rtd_part_outl(0x1803bb4c, 10, 10, Koffset_tmds_1[value_param].b_offset_en_de_259); //[10]offset_en_de=1
	_phy_rtd_part_outl(0x1803bb4c, 20, 20, Koffset_tmds_1[value_param].b_offset_en_do_260); //[20]offset_en_do=1
	_phy_rtd_part_outl(0x1803bb50, 0, 0, Koffset_tmds_1[value_param].b_offset_en_ee_261); //[20]offset_en_ee=1
	_phy_rtd_part_outl(0x1803bb50, 10, 10, Koffset_tmds_1[value_param].b_offset_en_eo_262); //[20]offset_en_eo=1
	_phy_rtd_part_outl(0x1803bb48, 10, 10, Koffset_tmds_1[value_param].b_offset_en_opo_263); //[10]offset_en_opo=1
	_phy_rtd_part_outl(0x1803bb48, 0, 0, Koffset_tmds_1[value_param].b_offset_en_ope_264); //[0]offset_en_ope=1
	ScalerTimer_DelayXms(1);
	//Check Offset cal. OK Check
	_phy_rtd_part_outl(0x1803bc7c, 5, 0, 0x8); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_phy_rtd_part_outl(0x1803bb44, 7, 4, 0x9); //reg_p0_b_offset_coef_sel
	HDMI_EMG("DE Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 27, 27));
	HDMI_EMG("DO Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 26, 26));
	HDMI_EMG("EE Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 25, 25));
	HDMI_EMG("EOOffset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 24, 24));
	HDMI_EMG("DATA OFFSET OK = %d\n",_phy_rtd_part_inl(0x1803bb44, 28, 28));
	HDMI_EMG("DATA OFFSET OK = %d\n",_phy_rtd_part_inl(0x1803bb44, 29, 29));
	//Disable B-Lane DCVS KOFF
	_phy_rtd_part_outl(0x1803bb4c, 10, 10, 0); //[10]offset_en_de=1->0
	_phy_rtd_part_outl(0x1803bb4c, 20, 20, 0); //[20]offset_en_do=1->0
	_phy_rtd_part_outl(0x1803bb50, 0, 0, 0); //[20]offset_en_ee=1->0
	_phy_rtd_part_outl(0x1803bb50, 10, 10, 0); //[20]offset_en_eo=1->0
	_phy_rtd_part_outl(0x1803bb48, 10, 10, 0); //[10]offset_en_opo=1->0
	_phy_rtd_part_outl(0x1803bb48, 0, 0, 0); //[0]offset_en_ope=1->0
	//===G LANE KOFF Data/Edge
	_phy_rtd_part_outl(0x1803bb5c, 10, 10, Koffset_tmds_1[value_param].g_offset_en_de_282); //[10]offset_en_de=1
	_phy_rtd_part_outl(0x1803bb5c, 20, 20, Koffset_tmds_1[value_param].g_offset_en_do_283); //[20]offset_en_do=1
	_phy_rtd_part_outl(0x1803bb60, 0, 0, Koffset_tmds_1[value_param].g_offset_en_ee_284); //[20]offset_en_ee=1
	_phy_rtd_part_outl(0x1803bb60, 10, 10, Koffset_tmds_1[value_param].g_offset_en_eo_285); //[20]offset_en_eo=1
	_phy_rtd_part_outl(0x1803bb58, 10, 10, Koffset_tmds_1[value_param].g_offset_en_opo_286); //[10]offset_en_opo=1
	_phy_rtd_part_outl(0x1803bb58, 0, 0, Koffset_tmds_1[value_param].g_offset_en_ope_287); //[0]offset_en_ope=1
	ScalerTimer_DelayXms(1);
	//G-Lane DCVS Offset cal. OK Check
	_phy_rtd_part_outl(0x1803bc7c, 5, 0, 0xA); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_phy_rtd_part_outl(0x1803bb54, 7, 4, 0x9); //reg_p0_g_offset_coef_sel
	HDMI_EMG("DE Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 27, 27));
	HDMI_EMG("DO Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 26, 26));
	HDMI_EMG("EE Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 25, 25));
	HDMI_EMG("EO Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 24, 24));
	HDMI_EMG("DATA OFFSET OK = %d\n",_phy_rtd_part_inl(0x1803bb54, 28, 28));
	HDMI_EMG("DATA OFFSET OK = %d\n",_phy_rtd_part_inl(0x1803bb54, 29, 29));
	//G-LANE
	_phy_rtd_part_outl(0x1803bb5c, 10, 10, 0); //[10]offset_en_de=0
	_phy_rtd_part_outl(0x1803bb5c, 20, 20, 0); //[20]offset_en_do=0
	_phy_rtd_part_outl(0x1803bb60, 0, 0, 0); //[20]offset_en_ee=0
	_phy_rtd_part_outl(0x1803bb60, 10, 10, 0); //[20]offset_en_eo=0
	_phy_rtd_part_outl(0x1803bb58, 10, 10, 0); //[10]offset_en_opo=1->0
	_phy_rtd_part_outl(0x1803bb58, 0, 0, 0); //[0]offset_en_ope=1->0
	//===R LANE KOFF Data/Edge
	_phy_rtd_part_outl(0x1803bb6c, 10, 10, Koffset_tmds_1[value_param].r_offset_en_de_305); //[10]offset_en_de=1
	_phy_rtd_part_outl(0x1803bb6c, 20, 20, Koffset_tmds_1[value_param].r_offset_en_do_306); //[20]offset_en_do=1
	_phy_rtd_part_outl(0x1803bb70, 0, 0, Koffset_tmds_1[value_param].r_offset_en_ee_307); //[20]offset_en_ee=1
	_phy_rtd_part_outl(0x1803bb70, 10, 10, Koffset_tmds_1[value_param].r_offset_en_eo_308); //[20]offset_en_eo=1
	_phy_rtd_part_outl(0x1803bb68, 10, 10, Koffset_tmds_1[value_param].r_offset_en_opo_309); //[10]offset_en_opo=1
	_phy_rtd_part_outl(0x1803bb68, 0, 0, Koffset_tmds_1[value_param].r_offset_en_ope_310); //[0]offset_en_ope=1
	ScalerTimer_DelayXms(1);
	//R-Lane DCVS Offset cal. OK Check
	_phy_rtd_part_outl(0x1803bc7c, 5, 0, 0xC); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_phy_rtd_part_outl(0x1803bb64, 7, 4, 0x9); //reg_p0_r_offset_coef_sel
	HDMI_EMG("DE Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 27, 27));
	HDMI_EMG("DO Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 26, 26));
	HDMI_EMG("EE Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 25, 25));
	HDMI_EMG("EO Offset cal. OK Check = %d\n",_phy_rtd_part_inl(0x1803bc84, 24, 24));
	HDMI_EMG("DATA OFFSET OK = %d\n",_phy_rtd_part_inl(0x1803bb64, 28, 28));
	HDMI_EMG("DATA OFFSET OK = %d\n",_phy_rtd_part_inl(0x1803bb64, 29, 29));
	//Disable R-Lane DCVS KOFF
	_phy_rtd_part_outl(0x1803bb6c, 10, 10, 0); //[10]offset_en_de=0
	_phy_rtd_part_outl(0x1803bb6c, 20, 20, 0); //[20]offset_en_do=0
	_phy_rtd_part_outl(0x1803bb70, 0, 0, 0); //[20]offset_en_ee=0
	_phy_rtd_part_outl(0x1803bb70, 10, 10, 0); //[20]offset_en_eo=0
	_phy_rtd_part_outl(0x1803bb68, 10, 10, 0); //[10]offset_en_opo=1->0
	_phy_rtd_part_outl(0x1803bb68, 0, 0, 0); //[0]offset_en_ope=1->0
	//B-Lane LEQ & DCVS Offset Result
	_phy_rtd_part_outl(0x1803bb44, 7, 4, 0); //reg_p0_b_offset_coef_sel[4:0]
	HDMI_EMG("Data_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 4, 0));
	_phy_rtd_part_outl(0x1803bb44, 7, 4, 1); //reg_p0_b_offset_coef_sel[4:0]
	HDMI_EMG("Data_Odd_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 4, 0));
	_phy_rtd_part_outl(0x1803bb44, 7, 4, 2); //reg_p0_b_offset_coef_sel[4:0]
	HDMI_EMG("Edge_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 4, 0));
	_phy_rtd_part_outl(0x1803bb44, 7, 4, 3); //reg_p0_b_offset_coef_sel[4:0]
	HDMI_EMG("Edge_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 4, 0));
	_phy_rtd_part_outl(0x1803bb44, 7, 4, 8); //reg_p0_b_offset_coef_sel[4:0]
	HDMI_EMG("EQ_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 4, 0));
	_phy_rtd_part_outl(0x1803bb44, 7, 4, 4); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("OFFP_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 4, 0));
	_phy_rtd_part_outl(0x1803bb44, 7, 4, 5); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("OFFP_Odd_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 4, 0));
	//G-Lane LEQ & DCVS Offset Result
	_phy_rtd_part_outl(0x1803bb54, 7, 4, 0); //reg_p0_g_offset_coef_sel[4:0]
	HDMI_EMG("Data_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 12, 8));
	_phy_rtd_part_outl(0x1803bb54, 7, 4, 1); //reg_p0_g_offset_coef_sel[4:0]
	HDMI_EMG("Data_Odd_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 12, 8));
	_phy_rtd_part_outl(0x1803bb54, 7, 4, 2); //reg_p0_g_offset_coef_sel[4:0]
	HDMI_EMG("Edge_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 12, 8));
	_phy_rtd_part_outl(0x1803bb54, 7, 4, 3); //reg_p0_g_offset_coef_sel[4:0]
	HDMI_EMG("Edge_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 12, 8));
	_phy_rtd_part_outl(0x1803bb54, 7, 4, 8); //reg_p0_g_offset_coef_sel[4:0]
	HDMI_EMG("EQ_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 12, 8));
	_phy_rtd_part_outl(0x1803bb54, 7, 4, 4); //reg_p0_g_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("OFFP_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 12, 8));
	_phy_rtd_part_outl(0x1803bb54, 7, 4, 5); //reg_p0_g_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("OFFP_Odd_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 12, 8));
	//R-Lane LEQ & DCVS Offset Result
	_phy_rtd_part_outl(0x1803bb64, 7, 4, 0); //reg_p0_r_offset_coef_sel[4:0]
	HDMI_EMG("Data_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 20, 16));
	_phy_rtd_part_outl(0x1803bb64, 7, 4, 1); //reg_p0_r_offset_coef_sel[4:0]
	HDMI_EMG("Data_Odd_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 20, 16));
	_phy_rtd_part_outl(0x1803bb64, 7, 4, 2); //reg_p0_r_offset_coef_sel[4:0]
	HDMI_EMG("Edge_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 20, 16));
	_phy_rtd_part_outl(0x1803bb64, 7, 4, 3); //reg_p0_r_offset_coef_sel[4:0]
	HDMI_EMG("Edge_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 20, 16));
	_phy_rtd_part_outl(0x1803bb64, 7, 4, 8); //reg_p0_r_offset_coef_sel[4:0]
	HDMI_EMG("EQ_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 20, 16));
	_phy_rtd_part_outl(0x1803bb64, 7, 4, 4); //reg_p0_r_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("OFFP_Even_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 20, 16));
	_phy_rtd_part_outl(0x1803bb64, 7, 4, 5); //reg_p0_r_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 4:OFFP_Even, 5:OFFP_Odd, 8:EQ
	HDMI_EMG("OFFP_Odd_Koffset = %d\n",_phy_rtd_part_inl(0x1803bb84, 20, 16));
	//Input on
	_phy_rtd_part_outl(0x1803ba50, 0, 0, 0); //INOFF_EN =0 Input_on
	_phy_rtd_part_outl(0x1803ba70, 0, 0, 0); //INOFF_EN =0 Input_on
	_phy_rtd_part_outl(0x1803ba90, 0, 0, 0); //INOFF_EN =0 Input_on
	_phy_rtd_part_outl(0x1803ba58, 28, 28, 0); //#B  DFE_TAP0_ICOM_EN=0
	_phy_rtd_part_outl(0x1803ba5c, 6, 3, Koffset_tmds_1[value_param].B_13_377); //#B  DFE_TAP_EN[4:1] = 0xf
	_phy_rtd_part_outl(0x1803ba78, 28, 28, 0); //#G  DFE_TAP0_ICOM_EN=0
	_phy_rtd_part_outl(0x1803ba7c, 6, 3, Koffset_tmds_1[value_param].G_13_379); //#G  DFE_TAP_EN[4:1] = 0xf
	_phy_rtd_part_outl(0x1803ba98, 28, 28, 0); //#R  DFE_TAP0_ICOM_EN=0
	_phy_rtd_part_outl(0x1803ba9c, 6, 3, Koffset_tmds_1[value_param].R_13_381); //#R  DFE_TAP_EN[4:1] = 0xf
	_phy_rtd_part_outl(0x1803bc04, 0, 0, 0); //R finetunstart on
	_phy_rtd_part_outl(0x1803bbe8, 0, 0, 0); //G finetunstart on
	_phy_rtd_part_outl(0x1803bbc8, 0, 0, 0); //B finetunstart on
}

void LEQ_VTH_Tap0_3_4_Adapt_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	_phy_rtd_part_outl(0x1803bcb0, 15, 15, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 14, 14, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 13, 13, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 12, 12, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 11, 11, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 10, 10, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 9, 9, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 8, 8, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bc70, 0, 0, 1); //r,g,b rstb_eq=1
	_phy_rtd_part_outl(0x1803bc70, 1, 1, 1); //r,g,b rstb_eq=1
	_phy_rtd_part_outl(0x1803bc70, 2, 2, 1); //r,g,b rstb_eq=1
	_phy_rtd_part_outl(0x1803bc70, 4, 4, 1); //r,g,b leq_en=1
	_phy_rtd_part_outl(0x1803bc70, 5, 5, 1); //r,g,b leq_en=1
	_phy_rtd_part_outl(0x1803bc70, 6, 6, 1); //r,g,b leq_en=1
	_phy_rtd_part_outl(0x1803bc70, 12, 12, 1); //r,g,b vth_en=1
	_phy_rtd_part_outl(0x1803bc70, 13, 13, 1); //r,g,b vth_en=1
	_phy_rtd_part_outl(0x1803bc70, 14, 14, 1); //r,g,b vth_en=1
	_phy_rtd_part_outl(0x1803bc70, 8, 8, 1); //r,g,b dfe_en=1
	_phy_rtd_part_outl(0x1803bc70, 9, 9, 1); //r,g,b dfe_en=1
	_phy_rtd_part_outl(0x1803bc70, 10, 10, 1); //r,g,b dfe_en=1
	//DFE_Adaptation
	//===CK LANE TAP0 & LEQ
	_phy_rtd_outl(0x1803becc,0x00005400); //Timer, set eqfe_en,servo_en,timer_en=1
	_phy_rtd_part_outl(0x1803bef8, 23, 23, 1); //Inverse vth sign polarity
	_phy_rtd_part_outl(0x1803bed0, 31, 31, 1); //leq1_inv=1
	_phy_rtd_part_outl(0x1803bed0, 30, 30, 1); //leq2_inv=0
	_phy_rtd_part_outl(0x1803bed4, 17, 15, 0); //LEQ_GAIN1=0
	_phy_rtd_part_outl(0x1803bed4, 14, 12, 0); //LEQ_GAIN2=0
	_phy_rtd_part_outl(0x1803bed8, 22, 21, 1); //LEQ1_trans_mode=1
	_phy_rtd_part_outl(0x1803bed8, 20, 19, 3); //LEQ2_trans_mode=3
	_phy_rtd_part_outl(0x1803bed8, 28, 26, 4); //tap0_gain=4
	_phy_rtd_part_outl(0x1803bed8, 25, 23, 4); //tap0_gain=4
	_phy_rtd_part_outl(0x1803becc, 26, 24, 5); //20200824_tap_timer=5
	_phy_rtd_part_outl(0x1803becc, 20, 18, 5); //20200825_LEQ_timer=5
	_phy_rtd_part_outl(0x1803becc, 23, 21, 5); //20200825_VTH_timer=5
	_phy_rtd_part_outl(0x1803becc, 13, 13, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].EQFE_EN_CK_36); //20210807_DFE ADAP LE=0
	_phy_rtd_part_outl(0x1803bee8, 25, 25, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_CK_6_0_37); //20210807_DFE_ADAP_EN_tap0=0
	_phy_rtd_part_outl(0x1803bee8, 26, 26, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_CK_6_0_38); //20200824_DFE_ADAP_EN_tap1=0
	_phy_rtd_part_outl(0x1803bee8, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=0
	_phy_rtd_part_outl(0x1803bee8, 28, 28, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_CK_6_0_40); //20200824_DFE_ADAP_EN_tap3=0
	_phy_rtd_part_outl(0x1803bee8, 29, 29, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_CK_6_0_41); //20200824_DFE_ADAP_EN_tap4=0
	_phy_rtd_part_outl(0x1803bec8, 22, 22, 0); //VTH_EN_B=1
	_phy_rtd_part_outl(0x1803bec8, 23, 23, 0); //VTH_DFE_EN_B=1
	_phy_rtd_part_outl(0x1803bedc, 31, 30, 0x1); //tap0_adjust_b =1
	udelay(500);
	//===B LANE TAP0 & LEQ
	_phy_rtd_outl(0x1803be8c,0x00005400); //Timer, set eqfe_en,servo_en,timer_en=1
	_phy_rtd_part_outl(0x1803beb8, 23, 23, 1); //Inverse vth sign polarity
	_phy_rtd_part_outl(0x1803be90, 31, 31, 1); //leq1_inv=1
	_phy_rtd_part_outl(0x1803be90, 30, 30, 1); //leq2_inv=0
	_phy_rtd_part_outl(0x1803be94, 17, 15, 0); //LEQ_GAIN1=0
	_phy_rtd_part_outl(0x1803be94, 14, 12, 0); //LEQ_GAIN2=0
	_phy_rtd_part_outl(0x1803be98, 22, 21, 1); //LEQ1_trans_mode=1
	_phy_rtd_part_outl(0x1803be98, 20, 19, 3); //LEQ2_trans_mode=3
	_phy_rtd_part_outl(0x1803be98, 28, 26, 4); //tap0_gain=4
	_phy_rtd_part_outl(0x1803be98, 25, 23, 4); //tap0_gain=4
	_phy_rtd_part_outl(0x1803be8c, 26, 24, 5); //20200824_tap_timer=5
	_phy_rtd_part_outl(0x1803be8c, 20, 18, 5); //20200825_LEQ_timer=5
	_phy_rtd_part_outl(0x1803be8c, 23, 21, 5); //20200825_VTH_timer=5
	_phy_rtd_part_outl(0x1803be8c, 13, 13, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].EQFE_EN_B_59); //20210807_DFE ADAP LE=0
	_phy_rtd_part_outl(0x1803bea8, 25, 25, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_B_6_0_60); //20210807_DFE_ADAP_EN_tap0=0
	_phy_rtd_part_outl(0x1803bea8, 26, 26, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_B_6_0_61); //20200824_DFE_ADAP_EN_tap1=0
	_phy_rtd_part_outl(0x1803bea8, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=0
	_phy_rtd_part_outl(0x1803bea8, 28, 28, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_B_6_0_63); //20200824_DFE_ADAP_EN_tap3=0
	_phy_rtd_part_outl(0x1803bea8, 29, 29, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_B_6_0_64); //20200824_DFE_ADAP_EN_tap4=0
	_phy_rtd_part_outl(0x1803be88, 22, 22, 1); //VTH_EN_B=1
	_phy_rtd_part_outl(0x1803be88, 23, 23, 1); //VTH_DFE_EN_B=1
	_phy_rtd_part_outl(0x1803be9c, 31, 30, 0x1); //tap0_adjust_b =1
	udelay(500);
	//===G LANE TAP0 & LEQ
	_phy_rtd_outl(0x1803be4c,0x00005400); //Timer, set eqfe_en,servo_en,timer_en=1
	_phy_rtd_part_outl(0x1803be78, 23, 23, 1); //Inverse vth sign polarity
	_phy_rtd_part_outl(0x1803be50, 31, 31, 1); //leq1_inv=1
	_phy_rtd_part_outl(0x1803be50, 30, 30, 1); //leq2_inv=0
	_phy_rtd_part_outl(0x1803be54, 17, 15, 0); //LEQ_GAIN1=0
	_phy_rtd_part_outl(0x1803be54, 14, 12, 0); //LEQ_GAIN2=0
	_phy_rtd_part_outl(0x1803be58, 22, 21, 1); //LEQ1_trans_mode=1
	_phy_rtd_part_outl(0x1803be58, 20, 19, 3); //LEQ2_trans_mode=3
	_phy_rtd_part_outl(0x1803be58, 28, 26, 4); //tap0_gain=4
	_phy_rtd_part_outl(0x1803be58, 25, 23, 4); //tap0_gain=4
	_phy_rtd_part_outl(0x1803be4c, 26, 24, 5); //20200824_tap_timer=5
	_phy_rtd_part_outl(0x1803be4c, 20, 18, 5); //20200825_LEQ_timer=5
	_phy_rtd_part_outl(0x1803be4c, 23, 21, 5); //20200825_VTH_timer=5
	_phy_rtd_part_outl(0x1803be4c, 13, 13, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].EQFE_EN_G_82); //20210807_DFE ADAP LE=0
	_phy_rtd_part_outl(0x1803be68, 25, 25, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_G_6_0_83); //20210807_DFE_ADAP_EN_tap0=0
	_phy_rtd_part_outl(0x1803be68, 26, 26, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_G_6_0_84); //20200824_DFE_ADAP_EN_tap1=0
	_phy_rtd_part_outl(0x1803be68, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=0
	_phy_rtd_part_outl(0x1803be68, 28, 28, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_G_6_0_86); //20200824_DFE_ADAP_EN_tap3=0
	_phy_rtd_part_outl(0x1803be68, 29, 29, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_G_6_0_87); //20200824_DFE_ADAP_EN_tap4=0
	_phy_rtd_part_outl(0x1803be48, 22, 22, 1); //VTH_EN_B=1
	_phy_rtd_part_outl(0x1803be48, 23, 23, 1); //VTH_DFE_EN_B=1
	_phy_rtd_part_outl(0x1803be5c, 31, 30, 0x1); //tap0_adjust_b =1
	udelay(500);
	//===R LANE LEQ_DFE_adapat
	_phy_rtd_outl(0x1803be0c, 0x00005400); //Timer, set eqfe_en,servo_en,timer_en=1
	_phy_rtd_part_outl(0x1803be38, 23, 23, 1); //Inverse vth sign polarity
	_phy_rtd_part_outl(0x1803be10, 31, 31, 1); //leq1_inv=1
	_phy_rtd_part_outl(0x1803be10, 30, 30, 1); //leq2_inv=0
	_phy_rtd_part_outl(0x1803be14, 17, 15, 0); //LEQ_GAIN1=0
	_phy_rtd_part_outl(0x1803be14, 14, 12, 0); //LEQ_GAIN2=0
	_phy_rtd_part_outl(0x1803be18, 22, 21, 1); //LEQ1_trans_mode=1
	_phy_rtd_part_outl(0x1803be18, 20, 19, 3); //LEQ2_trans_mode=3
	_phy_rtd_part_outl(0x1803be18, 28, 26, 4); //tap0_gain=4
	_phy_rtd_part_outl(0x1803be18, 25, 23, 4); //tap0_gain=4
	_phy_rtd_part_outl(0x1803be0c, 26, 24, 5); //20200824_tap_timer=5
	_phy_rtd_part_outl(0x1803be0c, 20, 18, 5); //20200825_LEQ_timer=5
	_phy_rtd_part_outl(0x1803be0c, 23, 21, 5); //20200825_VTH_timer=5
	_phy_rtd_part_outl(0x1803be0c, 13, 13, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].EQFE_EN_R_105); //20210807_DFE ADAP LE=0
	_phy_rtd_part_outl(0x1803be28, 25, 25, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_R_6_0_106); //20210807_DFE_ADAP_EN_tap0=0
	_phy_rtd_part_outl(0x1803be28, 26, 26, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_R_6_0_107); //20200824_DFE_ADAP_EN_tap1=0
	_phy_rtd_part_outl(0x1803be28, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=0
	_phy_rtd_part_outl(0x1803be28, 28, 28, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_R_6_0_109); //20200824_DFE_ADAP_EN_tap3=0
	_phy_rtd_part_outl(0x1803be28, 29, 29, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_R_6_0_110); //20200824_DFE_ADAP_EN_tap4=0
	_phy_rtd_part_outl(0x1803be08, 22, 22, 1); //VTH_EN_B=1
	_phy_rtd_part_outl(0x1803be08, 23, 23, 1); //VTH_DFE_EN_B=1
	_phy_rtd_part_outl(0x1803be1c, 31, 30, 0x1); //tap0_adjust_b =1
	udelay(500);
	//LEQ_DFE adapt
	_phy_rtd_part_outl(0x1803becc, 13, 13, 0); //20210807_DFE ADAP LE=0
	_phy_rtd_part_outl(0x1803bee8, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_phy_rtd_part_outl(0x1803bee8, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=1
	_phy_rtd_part_outl(0x1803bee8, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=1
	_phy_rtd_part_outl(0x1803bee8, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=1
	_phy_rtd_part_outl(0x1803bee8, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=1
	_phy_rtd_part_outl(0x1803bec8, 22, 22, 0); //VTH_EN_B=0
	_phy_rtd_part_outl(0x1803bec8, 23, 23, 0); //VTH_DFE_EN_B=0
	_phy_rtd_part_outl(0x1803be8c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_phy_rtd_part_outl(0x1803bea8, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_phy_rtd_part_outl(0x1803bea8, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=0
	_phy_rtd_part_outl(0x1803bea8, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=0
	_phy_rtd_part_outl(0x1803bea8, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=0
	_phy_rtd_part_outl(0x1803bea8, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=0
	_phy_rtd_part_outl(0x1803be88, 22, 22, 0); //VTH_EN_B=0
	_phy_rtd_part_outl(0x1803be88, 23, 23, 0); //VTH_DFE_EN_B=0
	_phy_rtd_part_outl(0x1803be4c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_phy_rtd_part_outl(0x1803be68, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_phy_rtd_part_outl(0x1803be68, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=0
	_phy_rtd_part_outl(0x1803be68, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=0
	_phy_rtd_part_outl(0x1803be68, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=0
	_phy_rtd_part_outl(0x1803be68, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=0
	_phy_rtd_part_outl(0x1803be48, 22, 22, 0); //VTH_EN_B=0
	_phy_rtd_part_outl(0x1803be48, 23, 23, 0); //VTH_DFE_EN_B=0
	_phy_rtd_part_outl(0x1803be0c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_phy_rtd_part_outl(0x1803be28, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=0
	_phy_rtd_part_outl(0x1803be28, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=0
	_phy_rtd_part_outl(0x1803be28, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=0
	_phy_rtd_part_outl(0x1803be28, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=0
	_phy_rtd_part_outl(0x1803be08, 22, 22, 0); //VTH_EN_B=0
	_phy_rtd_part_outl(0x1803be08, 23, 23, 0); //VTH_DFE_EN_B=0
	_phy_rtd_part_outl(0x1803be28, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
}

void Tap0_to_Tap4_Adapt_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	_phy_rtd_part_outl(0x1803bcb0, 15, 15, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 14, 14, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 13, 13, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 12, 12, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 11, 11, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 10, 10, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 9, 9, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 8, 8, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bc70, 0, 0, 1); //r,g,b rstb_eq=1
	_phy_rtd_part_outl(0x1803bc70, 1, 1, 1); //r,g,b rstb_eq=1
	_phy_rtd_part_outl(0x1803bc70, 2, 2, 1); //r,g,b rstb_eq=1
	_phy_rtd_part_outl(0x1803bc70, 4, 4, 1); //r,g,b leq_en=1
	_phy_rtd_part_outl(0x1803bc70, 5, 5, 1); //r,g,b leq_en=1
	_phy_rtd_part_outl(0x1803bc70, 6, 6, 1); //r,g,b leq_en=1
	_phy_rtd_part_outl(0x1803bc70, 12, 12, 1); //r,g,b vth_en=1
	_phy_rtd_part_outl(0x1803bc70, 13, 13, 1); //r,g,b vth_en=1
	_phy_rtd_part_outl(0x1803bc70, 14, 14, 1); //r,g,b vth_en=1
	_phy_rtd_part_outl(0x1803bc70, 8, 8, 1); //r,g,b dfe_en=1
	_phy_rtd_part_outl(0x1803bc70, 9, 9, 1); //r,g,b dfe_en=1
	_phy_rtd_part_outl(0x1803bc70, 10, 10, 1); //r,g,b dfe_en=1
	//CK-Lane Tap0-4 Adapt
	_phy_rtd_part_outl(0x1803becc, 13, 13, 0); //20210807_DFE ADAP LE=0
	_phy_rtd_part_outl(0x1803bee8, 25, 25, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_CK_6_0_23); //20210807_DFE_ADAP_EN_tap0=0
	_phy_rtd_part_outl(0x1803bee8, 26, 26, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_CK_6_0_24); //20200824_DFE_ADAP_EN_tap1=0
	_phy_rtd_part_outl(0x1803bee8, 27, 27, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_CK_6_0_25); //20200824_DFE_ADAP_EN_tap2=0
	_phy_rtd_part_outl(0x1803bee8, 28, 28, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_CK_6_0_26); //20200824_DFE_ADAP_EN_tap3=0
	_phy_rtd_part_outl(0x1803bee8, 29, 29, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_CK_6_0_27); //20200824_DFE_ADAP_EN_tap4=0
	_phy_rtd_part_outl(0x1803bec8, 22, 22, 0); //VTH_EN_B=0
	_phy_rtd_part_outl(0x1803bec8, 23, 23, 0); //VTH_DFE_EN_B=0
	//B-Lane Tap0-4 Adapt
	_phy_rtd_part_outl(0x1803be8c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_phy_rtd_part_outl(0x1803bea8, 25, 25, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_B_6_0_32); //20210807_DFE_ADAP_EN_tap0=0
	_phy_rtd_part_outl(0x1803bea8, 26, 26, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_B_6_0_33); //20200824_DFE_ADAP_EN_tap1=0
	_phy_rtd_part_outl(0x1803bea8, 27, 27, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_B_6_0_34); //20200824_DFE_ADAP_EN_tap2=0
	_phy_rtd_part_outl(0x1803bea8, 28, 28, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_B_6_0_35); //20200824_DFE_ADAP_EN_tap3=0
	_phy_rtd_part_outl(0x1803bea8, 29, 29, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_B_6_0_36); //20200824_DFE_ADAP_EN_tap4=0
	_phy_rtd_part_outl(0x1803be88, 22, 22, 1); //VTH_EN_B=1
	_phy_rtd_part_outl(0x1803be88, 23, 23, 1); //VTH_DFE_EN_B=1
	//G-Lane Tap0-4 Adapt
	_phy_rtd_part_outl(0x1803be4c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_phy_rtd_part_outl(0x1803be68, 25, 25, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_G_6_0_41); //20210807_DFE_ADAP_EN_tap0=0
	_phy_rtd_part_outl(0x1803be68, 26, 26, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_G_6_0_42); //20200824_DFE_ADAP_EN_tap1=0
	_phy_rtd_part_outl(0x1803be68, 27, 27, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_G_6_0_43); //20200824_DFE_ADAP_EN_tap2=0
	_phy_rtd_part_outl(0x1803be68, 28, 28, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_G_6_0_44); //20200824_DFE_ADAP_EN_tap3=0
	_phy_rtd_part_outl(0x1803be68, 29, 29, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_G_6_0_45); //20200824_DFE_ADAP_EN_tap4=0
	_phy_rtd_part_outl(0x1803be48, 22, 22, 1); //VTH_EN_B=1
	_phy_rtd_part_outl(0x1803be48, 23, 23, 1); //VTH_DFE_EN_B=1
	//R-Lane Tap0-4 Adapt
	_phy_rtd_part_outl(0x1803be0c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_phy_rtd_part_outl(0x1803be28, 25, 25, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_R_6_0_50); //20210807_DFE_ADAP_EN_tap0=0
	_phy_rtd_part_outl(0x1803be28, 26, 26, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_R_6_0_51); //20200824_DFE_ADAP_EN_tap1=0
	_phy_rtd_part_outl(0x1803be28, 27, 27, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_R_6_0_52); //20200824_DFE_ADAP_EN_tap2=0
	_phy_rtd_part_outl(0x1803be28, 28, 28, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_R_6_0_53); //20200824_DFE_ADAP_EN_tap3=0
	_phy_rtd_part_outl(0x1803be28, 29, 29, Tap0_to_Tap4_Adapt_tmds_1[value_param].DFE_ADAPT_EN_R_6_0_54); //20200824_DFE_ADAP_EN_tap4=0
	_phy_rtd_part_outl(0x1803be08, 22, 22, 1); //VTH_EN_B=1
	_phy_rtd_part_outl(0x1803be08, 23, 23, 1); //VTH_DFE_EN_B=1
	udelay(500);
	//CK-Lane Tap0-4 Adapt
	_phy_rtd_part_outl(0x1803becc, 13, 13, 0); //20210807_DFE ADAP LE=0
	_phy_rtd_part_outl(0x1803bee8, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_phy_rtd_part_outl(0x1803bee8, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=1
	_phy_rtd_part_outl(0x1803bee8, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=1
	_phy_rtd_part_outl(0x1803bee8, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=1
	_phy_rtd_part_outl(0x1803bee8, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=1
	_phy_rtd_part_outl(0x1803bec8, 22, 22, 0); //VTH_EN_B=0
	_phy_rtd_part_outl(0x1803bec8, 23, 23, 0); //VTH_DFE_EN_B=0
	//B-Lane Tap0-4 Adapt
	_phy_rtd_part_outl(0x1803be8c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_phy_rtd_part_outl(0x1803bea8, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_phy_rtd_part_outl(0x1803bea8, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=1
	_phy_rtd_part_outl(0x1803bea8, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=1
	_phy_rtd_part_outl(0x1803bea8, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=1
	_phy_rtd_part_outl(0x1803bea8, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=1
	_phy_rtd_part_outl(0x1803be88, 22, 22, 0); //VTH_EN_B=0
	_phy_rtd_part_outl(0x1803be88, 23, 23, 0); //VTH_DFE_EN_B=0
	//G-Lane Tap0-4 Adapt
	_phy_rtd_part_outl(0x1803be4c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_phy_rtd_part_outl(0x1803be68, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_phy_rtd_part_outl(0x1803be68, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=1
	_phy_rtd_part_outl(0x1803be68, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=1
	_phy_rtd_part_outl(0x1803be68, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=1
	_phy_rtd_part_outl(0x1803be68, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=1
	_phy_rtd_part_outl(0x1803be48, 22, 22, 0); //VTH_EN_B=0
	_phy_rtd_part_outl(0x1803be48, 23, 23, 0); //VTH_DFE_EN_B=0
	//R-Lane Tap0-4 Adapt
	_phy_rtd_part_outl(0x1803be0c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_phy_rtd_part_outl(0x1803be28, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_phy_rtd_part_outl(0x1803be28, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=1
	_phy_rtd_part_outl(0x1803be28, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=1
	_phy_rtd_part_outl(0x1803be28, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=1
	_phy_rtd_part_outl(0x1803be28, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=1
	_phy_rtd_part_outl(0x1803be08, 22, 22, 0); //VTH_EN_B=0
	_phy_rtd_part_outl(0x1803be48, 23, 23, 0); //VTH_DFE_EN_B=0
}

void DCDR_settings_tmds_3_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	_phy_rtd_part_outl(0x1803bb00, 0, 0, 0x0); //Toggle Dphy reset at the Phy script ending
	_phy_rtd_part_outl(0x1803bb00, 1, 1, 0x0); //Toggle Dphy reset at the Phy script ending
	_phy_rtd_part_outl(0x1803bb00, 2, 2, 0x0); //Toggle Dphy reset at the Phy script ending
	_phy_rtd_part_outl(0x1803bb00, 3, 3, 0x0); //Toggle Dphy reset at the Phy script ending
	_phy_rtd_part_outl(0x1803bb00, 4, 4, 0x0); //Toggle Dphy reset at the Phy script ending
	_phy_rtd_part_outl(0x1803bb00, 5, 5, 0x0); //Toggle Dphy reset at the Phy script ending
	_phy_rtd_part_outl(0x1803bb00, 6, 6, 0x0); //Toggle Dphy reset at the Phy script ending
	_phy_rtd_part_outl(0x1803bb00, 7, 7, 0x0); //Toggle Dphy reset at the Phy script ending
	_phy_rtd_part_outl(0x1803bb00, 0, 0, 0x1); //Toggle Dphy reset at the Phy script ending
	_phy_rtd_part_outl(0x1803bb00, 1, 1, 0x1); //Toggle Dphy reset at the Phy script ending
	_phy_rtd_part_outl(0x1803bb00, 2, 2, 0x1); //Toggle Dphy reset at the Phy script ending
	_phy_rtd_part_outl(0x1803bb00, 3, 3, 0x1); //Toggle Dphy reset at the Phy script ending
	_phy_rtd_part_outl(0x1803bb00, 4, 4, 0x1); //Toggle Dphy reset at the Phy script ending
	_phy_rtd_part_outl(0x1803bb00, 5, 5, 0x1); //Toggle Dphy reset at the Phy script ending
	_phy_rtd_part_outl(0x1803bb00, 6, 6, 0x1); //Toggle Dphy reset at the Phy script ending
	_phy_rtd_part_outl(0x1803bb00, 7, 7, 0x1); //Toggle Dphy reset at the Phy script ending
	//DFE Adaptation power off
	_phy_rtd_part_outl(0x1803ba3c, 7, 7, 0); //Mark2_Modify; CK-Lane DFE_ADAPT_EN
	_phy_rtd_part_outl(0x1803ba5c, 7, 7, 0); //Mark2_Modify; B-Lane DFE_ADAPT_EN
	_phy_rtd_part_outl(0x1803ba7c, 7, 7, 0); //Mark2_Modify; G-Lane DFE_ADAPT_EN
	_phy_rtd_part_outl(0x1803ba9c, 7, 7, 0); //Mark2_Modify; R-Lane DFE_ADAPT_EN
}

void Manual_DFE_tmds_1_func(unsigned char nport, TMDS_VALUE_PARAM_T value_param)
{
	// *************************************************************************************************************************************
	//B-Lane_initial DFE Value_Start
	_phy_rtd_part_outl(0x1803bcb0, 15, 15, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 14, 14, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 13, 13, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 12, 12, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 11, 11, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 10, 10, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 9, 9, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bcb0, 8, 8, 1); //offset_code/vth select
	_phy_rtd_part_outl(0x1803bea8, 9, 5, 5); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_phy_rtd_part_outl(0x1803bea8, 4, 0, 5); //VTHN_INIT_B[4:0]	4:0	R/W	0x0F		VTHN INIT
	_phy_rtd_part_outl(0x1803bea0, 4, 0, 0); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_phy_rtd_part_outl(0x1803bea4, 31, 24, 25); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_phy_rtd_part_outl(0x1803bea4, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_phy_rtd_part_outl(0x1803bea4, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_phy_rtd_part_outl(0x1803bea4, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_phy_rtd_part_outl(0x1803bea4, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//B-Lane DFE COEF LOAD IN
	_phy_rtd_part_outl(0x1803be8c, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_phy_rtd_part_outl(0x1803be8c, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_phy_rtd_part_outl(0x1803be8c, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_phy_rtd_part_outl(0x1803be8c, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_phy_rtd_part_outl(0x1803be8c, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_phy_rtd_part_outl(0x1803be8c, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_phy_rtd_part_outl(0x1803be88, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_phy_rtd_part_outl(0x1803be8c, 4, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_phy_rtd_part_outl(0x1803be8c, 5, 5, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_phy_rtd_part_outl(0x1803be88, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	//B-Lane_initial DFE Value_Stop
	//**************************************************************************************************************************************
	// *************************************************************************************************************************************
	//G-Lane_initial DFE Value_Start
	_phy_rtd_part_outl(0x1803be68, 9, 5, 5); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_phy_rtd_part_outl(0x1803be68, 4, 0, 5); //VTHN_INIT_B[4:0]	4:0	R/W	0x0F		VTHN INIT
	_phy_rtd_part_outl(0x1803be60, 4, 0, 0); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_phy_rtd_part_outl(0x1803be64, 31, 24, 25); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_phy_rtd_part_outl(0x1803be64, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_phy_rtd_part_outl(0x1803be64, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_phy_rtd_part_outl(0x1803be64, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_phy_rtd_part_outl(0x1803be64, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//G-Lane DFE COEF LOAD IN
	_phy_rtd_part_outl(0x1803be4c, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_phy_rtd_part_outl(0x1803be4c, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_phy_rtd_part_outl(0x1803be4c, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_phy_rtd_part_outl(0x1803be4c, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_phy_rtd_part_outl(0x1803be4c, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_phy_rtd_part_outl(0x1803be4c, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_phy_rtd_part_outl(0x1803be48, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_phy_rtd_part_outl(0x1803be4c, 4, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_phy_rtd_part_outl(0x1803be4c, 5, 5, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_phy_rtd_part_outl(0x1803be48, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	//G-Lane_initial DFE Value_Stop
	//**************************************************************************************************************************************
	// *************************************************************************************************************************************
	//R-Lane_initial DFE Value_Start
	_phy_rtd_part_outl(0x1803be28, 9, 5, 5); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_phy_rtd_part_outl(0x1803be28, 4, 0, 5); //VTHN_INIT_B[4:0]	4:0	R/W	0x0F		VTHN INIT
	_phy_rtd_part_outl(0x1803be20, 4, 0, 0); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_phy_rtd_part_outl(0x1803be24, 31, 24, 25); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_phy_rtd_part_outl(0x1803be24, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_phy_rtd_part_outl(0x1803be24, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_phy_rtd_part_outl(0x1803be24, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_phy_rtd_part_outl(0x1803be24, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//R-Lane DFE COEF LOAD IN
	_phy_rtd_part_outl(0x1803be0c, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_phy_rtd_part_outl(0x1803be0c, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_phy_rtd_part_outl(0x1803be0c, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_phy_rtd_part_outl(0x1803be0c, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_phy_rtd_part_outl(0x1803be0c, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_phy_rtd_part_outl(0x1803be0c, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_phy_rtd_part_outl(0x1803be08, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_phy_rtd_part_outl(0x1803be0c, 4, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_phy_rtd_part_outl(0x1803be0c, 5, 5, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_phy_rtd_part_outl(0x1803be08, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	//R-Lane_initial DFE Value_Stop
	//**************************************************************************************************************************************
	// *************************************************************************************************************************************
	//CK-Lane_initial DFE Value_Start
	_phy_rtd_part_outl(0x1803bee8, 9, 5, 5); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_phy_rtd_part_outl(0x1803bee8, 4, 0, 5); //VTHN_INIT_B[4:0]	4:0	R/W	0x0F		VTHN INIT
	_phy_rtd_part_outl(0x1803bee0, 4, 0, 0); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_phy_rtd_part_outl(0x1803bee4, 31, 24, 25); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_phy_rtd_part_outl(0x1803bee4, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_phy_rtd_part_outl(0x1803bee4, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_phy_rtd_part_outl(0x1803bee4, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_phy_rtd_part_outl(0x1803bee4, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//CK-Lane DFE COEF LOAD IN
	_phy_rtd_part_outl(0x1803becc, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_phy_rtd_part_outl(0x1803becc, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_phy_rtd_part_outl(0x1803becc, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_phy_rtd_part_outl(0x1803becc, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_phy_rtd_part_outl(0x1803becc, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_phy_rtd_part_outl(0x1803becc, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_phy_rtd_part_outl(0x1803bec8, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_phy_rtd_part_outl(0x1803becc, 4, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_phy_rtd_part_outl(0x1803becc, 5, 5, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_phy_rtd_part_outl(0x1803bec8, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_phy_rtd_part_outl(0x1803bef4, 17, 15, 3); //Adaption flow control mode
	_phy_rtd_part_outl(0x1803beb4, 17, 15, 3); //Adaption flow control mode
	_phy_rtd_part_outl(0x1803be74, 17, 15, 3); //Adaption flow control mode
	_phy_rtd_part_outl(0x1803be34, 17, 15, 3); //Adaption flow control mode
	//CK-Lane_initial DFE Value_Stop
	//**************************************************************************************************************************************
}


#endif

////////////////////////////////////////////

typedef struct {
	// factory mode
	unsigned char eq_mode;  // 0: auto adaptive  1: manual mode
	unsigned char manual_eq[4];
}HDMIRX_PHY_FACTORY_T;


#if BIST_PHY_SCAN
typedef enum {
	PHY_INIT_SCAN,
	PHY_ICP_SCAN,
	PHY_KP_SCAN,
	PHY_END_SCAN,
}HDMIRX_PHY_SCAN_T;
#endif


typedef enum {
	EQ_MANUAL_OFF,
	EQ_MANUAL_ON,
}HDMIRX_PHY_EQ_MODE;

/**********************************************************************************************
*
*	Variables
*
**********************************************************************************************/
unsigned int clock_bound_3g;
unsigned int clock_bound_1p5g;
unsigned int clock_bound_110m;
unsigned int clock_bound_45m;

HDMIRX_PHY_STRUCT_T phy_st[HDMI_PORT_TOTAL_NUM];

unsigned int phy_isr_en[HDMI_PORT_TOTAL_NUM];

HDMIRX_PHY_FACTORY_T phy_fac_eq_st[HDMI_PORT_TOTAL_NUM];

#if BIST_PHY_SCAN
HDMIRX_PHY_SCAN_T  phy_scan_state = PHY_INIT_SCAN;
#endif


/**********************************************************************************************
*
*	Funtion Declarations
*
**********************************************************************************************/

unsigned char newbase_rxphy_tmds_measure(unsigned char port);
unsigned char newbase_rxphy_is_clock_stable(unsigned char port);
void newbase_rxphy_job(unsigned char p);
unsigned char newbase_hdmi_set_phy(unsigned char port, unsigned int b, unsigned char frl_mode,unsigned char lane);

//dfe
void newbase_hdmi_dump_dfe_para(unsigned char nport, unsigned char lane_mode);
unsigned char newbase_hdmi_dfe_record(unsigned char nport, unsigned int clk);
unsigned char newbase_hdmi_dfe_close(unsigned char nport, unsigned int clk, unsigned char lane_mode, unsigned char phy_state);
void newbase_hdmi_dfe_hi_speed(unsigned char nport, unsigned int b_clk, unsigned char lane_mode);
void newbase_hdmi_dfe_6g_long_cable_patch(unsigned char nport);
//extern unsigned char newbase_hdmi_dfe_hi_speed_close(unsigned char nport);
void newbase_hdmi_dfe_mi_speed(unsigned char nport, unsigned int b_clk, unsigned char lane_mode);
void newbasse_hdmi_dfe_mid_adapthve(unsigned char nport, unsigned int b_clk);

//Z0
void lib_hdmi_z0_calibration(void);

//eq setting
#ifdef CONFIG_POWER_SAVING_MODE
void lib_hdmi_eq_pi_power_en(unsigned char bport,unsigned char en);
#endif
void lib_hdmi_eq_bias_band_setting(unsigned char bport, unsigned int b);


//dfe setting
void lib_hdmi_dfe_power(unsigned char port, unsigned char en, unsigned char lane_mode);
void lib_hdmi_set_dfe(unsigned char nport, unsigned int b, unsigned char dfe_mode, unsigned char lane_mode);
void lib_hdmi_set_dfe_mid_band(unsigned char nport, unsigned int b_clk, int dfe_mode, unsigned char lane_mode);
void lib_hdmi_dfe_manual_set(unsigned char nport, unsigned char lane_mode);
unsigned char lib_hdmi_dfe_tap0_fix(unsigned char nport,unsigned char lane_mode);
void lib_hdmi_tap0_fix_small_swing(unsigned char nport, unsigned char lane_mode);

void lib_hdmi_dfe_second_flow(unsigned char nport, unsigned int b_clk,unsigned char lane_mode);



void lib_hdmi_dfe_config_setting(unsigned char nport,unsigned char lane_mode);



//PLL/DEMUX/CDR
void lib_hdmi_cco_band_config(unsigned char nport, unsigned int bclk, HDMI_PHY_PARAM_T *phy_param);
void lib_hdmi_cmu_pll_en(unsigned char nport, unsigned char enable);
void lib_hdmi_demux_ck_vcopll_rst(unsigned char nport, unsigned char rst);
void lib_hdmi_cdr_rst(unsigned char nport, unsigned char rst);

//Watch Dog
void lib_hdmi_wdog(unsigned char nport);
void lib_hdmi_wdg_rst(unsigned char port, unsigned char rst);

//PHY TABLE
//void lib_hdmi_set_phy_table(unsigned char bport, HDMI_PHY_PARAM_T *phy_param);
//unsigned char lib_hdmi_get_phy_table(unsigned char nport,unsigned int b,HDMI_PHY_PARAM_T **p_phy_table);


//foreground K offset
//void lib_hdmi_fg_koffset_manual_adjust(unsigned char port, unsigned int r, unsigned int g, unsigned int b, unsigned int ck);
//void lib_hdmi_fg_koffset_manual(unsigned char port);
//void lib_hdmi_fg_koffset_auto(unsigned char port);
//void lib_hdmi_fg_koffset_proc(unsigned char nport);
//void lib_hdmi_fg_koffset_disable(unsigned char port);
void lib_hdmi_single_end(unsigned char nport, unsigned char channel, unsigned char p_off, unsigned char n_off);



//char & bit error detect

unsigned char lib_hdmi_char_err_get_error(unsigned char nport, unsigned int *r_err, unsigned int *g_err, unsigned int *b_err);
unsigned char lib_hdmi_bit_err_det_done(unsigned char nport);


//clock detect
void lib_hdmi_trigger_measurement(unsigned char nport);
void lib_hdmi_trigger_measure_start(unsigned char nport);
 void lib_hdmi_trigger_measure_stop(unsigned char nport);
unsigned char lib_hdmi_is_clk_ready(unsigned char nport);


unsigned char lib_hdmi_clock40x_sync_status(unsigned char nport);

/**********************************************************************************************
*
*	Function Body
*
**********************************************************************************************/



void newbase_rxphy_isr_set(unsigned char port, unsigned char en)
{
	phy_isr_en[port] = en;
}

void newbase_rxphy_init_struct(unsigned char port)
{
	memset(&phy_st[port], 0, sizeof(HDMIRX_PHY_STRUCT_T));
}

unsigned char newbase_rxphy_get_frl_mode(unsigned char port)
{
       return phy_st[port].frl_mode;
}

unsigned char newbase_rxphy_get_phy_proc_state(unsigned char port)
{
       return phy_st[port].phy_proc_state;
}

void newbase_hdmi_rxphy_handler(unsigned char port)
{
	if (lib_hdmi_get_fw_debug_bit(DEBUG_27_BYPASS_PHY_ISR)) {
		return ;
	}

	//--------------------------------------------
	// for HDMI 2.0
	// check TMDS lock status
	//--------------------------------------------
	if ( newbase_rxphy_tmds_measure(port)) {
		newbase_hdmi_hdcp14_set_aksv_intr(port, 0);

		newbase_rxphy_job(port);  //hd20 phy

		newbase_hdmi_hdcp14_set_aksv_intr(port, 1);
	}
	else
	{
		newbase_hdmi_reset_video_state(port);
		//SET_HDMI_DETECT_EVENT(port, HDMI_DETECT_PHY_RESET);
	}
}



int newbase_rxphy_isr(void)
{
	unsigned char i;

	for (i=0; i<HDMI_PORT_TOTAL_NUM; i++)
	{
		if (!phy_isr_en[i])
			continue;

		if((GET_FLOW_CFG(HDMI_FLOW_CFG_HPD, HDMI_FLOW_CFG1_DISABLE_PORT_FAST_SWITCH) == HDMI_PCB_FORCE_ENABLE) && (i != newbase_hdmi_get_current_display_port()))
			continue;

		newbase_hdmi_rxphy_handler(i);

		newbase_hdcp_handler(i);

		newbase_hdmi_video_monitor(i);

		newbase_hdmi_error_handler(i);

		newbase_hdmi_check_connection_state(i);

		newbase_hdmi_scdc_handler(i);  // handle SCDC event

		newbase_hdmi_power_saving_handler(i);  // handle Power Saving
	}

	return IRQ_HANDLED;

}
void newbase_set_qs_rxphy_status(unsigned char port)
{
	unsigned int clk_tmp = 0;
	if (lib_hdmi_is_clk_ready(port)) {
		clk_tmp = lib_hdmi_get_clock(port);
		newbase_hdmi2p0_check_tmds_config(port); //check scdc flag
	}

	lib_hdmi_trigger_measure_start(port);

	if (clk_tmp > VALID_MIN_CLOCK) {
		//if ((newbase_hdmi2p0_get_clock40x_flag(port) || newbase_hdmi2p0_get_scramble_flag(port)) &&
		if ((newbase_hdmi2p0_get_clock40x_flag(port)) &&
			(clk_tmp < 1422 && clk_tmp > 699)) {	//It's possible for 40x only when clock=(3G~6G)/4 //WOSQRTK-9795
			clk_tmp <<= 2;				 //HDMI2.0, change clock to clock*4
			phy_st[port].rxphy_40x = 1;
		}
		phy_st[port].clk_pre = clk_tmp;
		phy_st[port].clk_stable = 1;
		HDMI_EMG("clk = %d\n", phy_st[port].clk_pre);
		phy_st[port].clk = clk_tmp;
		phy_st[port].frl_mode = MODE_TMDS;
		phy_st[port].lane_num= HDMI_3LANE;
		phy_st[port].clk_tolerance = MAX(clk_tmp>>7, 5);
		phy_st[port].clk_debounce_count = 0;
		phy_st[port].clk_unstable_count = 0;
		hdmi_rx[port].timing_t.tmds_clk_b = phy_st[port].clk;
	}

	SET_HDMI_DETECT_EVENT(port, HDMI_DETECT_PHY_CLOCK_STABLE);
	phy_st[port].phy_proc_state = PHY_PROC_DONE;
}

unsigned char newbase_rxphy_set_phy_for_aksv(unsigned char port)
{
	unsigned char dest_state = PHY_PROC_CLOSE_ADP; //PHY_PROC_DONE;
	unsigned char monitor_cnt = 8;

	if (phy_st[port].phy_proc_state >= dest_state) {
		HDCP_FLOW_PRINTF("[p%d] phy already init (%d)\n", port, phy_st[port].phy_proc_state);
		return TRUE;
	}

	if (newbase_rxphy_is_clk_stable(port))
	{
		HDCP_FLOW_PRINTF("[p%d] clk %d\n", port, phy_st[port].clk);
	}
	else if (newbase_rxphy_get_clk_pre(port) > VALID_MIN_CLOCK)
	{
		phy_st[port].clk = newbase_rxphy_get_clk_pre(port);
		HDCP_FLOW_PRINTF("[p%d] use pre clk %d\n", port,  phy_st[port].clk);
	}
	else
	{
		HDCP_FLOW_PRINTF("[p%d] no clk\n", port);
		return FALSE;
	}

	//newbase_rxphy_set_frl_mode(port, newbase_hdmi_scdc_get_frl_mode(port));
	if (newbase_rxphy_get_frl_mode(port) != MODE_TMDS) {
		HDCP_FLOW_PRINTF("[p%d] frl mode = %s,  must be TMDS\n", port, _hdmi_hd21_frl_mode_str(newbase_rxphy_get_frl_mode(port)));
		return FALSE;
	}

	while (phy_st[port].phy_proc_state < dest_state && monitor_cnt) {
		HDCP_FLOW_PRINTF("[p%d] current phy state = %d, target state = %d\n", port, phy_st[port].phy_proc_state, dest_state);
		newbase_rxphy_job(port);  //hd20 phy

		if (--monitor_cnt == 0)
			HDMI_EMG("[p%d] set phy fail for HDCP14 aksv intr. (%d/%d)\n", port, phy_st[port].phy_proc_state, dest_state);
	}

	return TRUE;
}

unsigned char newbase_rxphy_tmds_measure(unsigned char port)
{
unsigned int clk_tmp = 0;
unsigned char clk_stable_prev = phy_st[port].clk_stable;

	if (lib_hdmi_is_clk_ready(port)) {
		clk_tmp = lib_hdmi_get_clock(port);
		newbase_hdmi2p0_check_tmds_config(port); //check scdc flag
		hdmi_rx[port].raw_tmds_phy_md_clk = clk_tmp;
	}
	
	lib_hdmi_trigger_measure_start(port);

	if (clk_tmp > VALID_MIN_CLOCK) {

		//if ((newbase_hdmi2p0_get_clock40x_flag(port) || newbase_hdmi2p0_get_scramble_flag(port)) &&
		if ((newbase_hdmi2p0_get_clock40x_flag(port)) &&
			(clk_tmp < 1422 && clk_tmp > 699)) {	//It's possible for 40x only when clock=(3G~6G)/4 //WOSQRTK-9795
			clk_tmp <<= 2; 				 //HDMI2.0, change clock to clock*4
			phy_st[port].rxphy_40x = 1;
		}

		if (HDMI_ABS(clk_tmp, phy_st[port].clk_pre) < CLOCK_TOLERANCE(port)) {
			// ------------------------------------------
			// Get valid clock and close to previous clock
			// ------------------------------------------
			if (newbase_rxphy_is_clock_stable(port)) {
				phy_st[port].clk_stable = 1;
				phy_st[port].clk = clk_tmp;
				phy_st[port].frl_mode = MODE_TMDS;
				phy_st[port].lane_num= HDMI_3LANE;
			}
			phy_st[port].clk_unstable_count = 0;
		}
		else if (phy_st[port].clk_unstable_count < CLK_UNSTABLE_CNT) {
			// ------------------------------------------
			// Get valid clock but different with previous clock, count for debouncing
			// ------------------------------------------
			phy_st[port].clk_unstable_count++;
		} else {
			// ------------------------------------------
			// Get valid clock but different with previous clock
			// ------------------------------------------
			phy_st[port].clk = 0;
			phy_st[port].clk_pre = clk_tmp;
			phy_st[port].clk_tolerance = MAX(clk_tmp>>7, 5);
			phy_st[port].clk_debounce_count = 0;
			phy_st[port].clk_unstable_count = 0;
			phy_st[port].clk_stable = 0;
			phy_st[port].phy_proc_state = PHY_PROC_INIT;
		}

		phy_st[port].noclk_debounce_count = 0;

	} else {

		if (phy_st[port].noclk_debounce_count < NOCLK_DEBOUNCE_CNT) {
			// ------------------------------------------
			// NO clock, and start debouncing
			// ------------------------------------------

			phy_st[port].noclk_debounce_count++;

		} else {
			// ------------------------------------------
			// NO clock, really!!  Then reset clk_info
			// ------------------------------------------

			if (phy_st[port].clk_pre) {
				phy_st[port].clk_pre = 0;
				phy_st[port].clk= 0;
				phy_st[port].clk_debounce_count= 0;
				phy_st[port].clk_unstable_count = 0;
				phy_st[port].clk_stable = 0;
				phy_st[port].phy_proc_state = PHY_PROC_INIT;
			}

			newbase_hdmi2p0_reset_scdc_toggle(port);
		}
	}

	lib_hdmi_trigger_measure_stop(port);

	if (clk_stable_prev != phy_st[port].clk_stable)
	{
#if HDMI_PHY_CLOCK_DYNAMIC_CNT_ENABLE
		HDMI_HDCP_E current_hdcp_mode = newbase_hdcp_get_auth_mode(port);
		if(HDCP14  == current_hdcp_mode || HDCP22 == current_hdcp_mode)
			HDCP_WARN("HDMI[p%d] clock stable change. HDCP mode (%s), clk_debounce (%x)\n", port,  _hdmi_hdcp_mode_str(current_hdcp_mode), phy_st[port].clk_debounce_count);
#endif //#if HDMI_PHY_CLOCK_DYNAMIC_CNT_ENABLE
		HDMI_EMG("clk_stable[p%d] from %d to %d clk=%d \n", port, clk_stable_prev, phy_st[port].clk_stable,phy_st[port].clk);

		lib_hdmi_misc_variable_initial(port);
		if(phy_st[port].clk_stable ==1)
		{
			SET_HDMI_DETECT_EVENT(port, HDMI_DETECT_PHY_CLOCK_STABLE);
		}

#if HDMI_ABORT_OFMS_WHEN_CLK_NOT_STABLE
		if (!phy_st[port].clk_stable && GET_H_VIDEO_FSM(port)==MAIN_FSM_HDMI_MEASURE)
			newbase_hdmi_dp_measure_abort();
#endif
	}

	if (phy_st[port].clk_stable) {
		return TRUE;
	} else {
		return FALSE;
	}

}



/*------------------------------------------------------------------
 * Func : newbase_rxphy_is_clock_stable
 *
 * Desc : When HDCP1.4/2.2 auth from tx quickly, rxphy should quick stable refer to CLK_DEBOUNCE_SHORT_CNT_THD.
 *           NO_HDCP or HDCP_OFF are need to wait rxphy stable depend on  CLK_DEBOUNCE_CNT_THD.
 *
 * Para : [IN] port  : physical port number
 *
 * Retn : TRUE: stable; FALSE: unstable, then clk_debounce_count++.
 *------------------------------------------------------------------*/
unsigned char newbase_rxphy_is_clock_stable(unsigned char port)
{
	unsigned int phy_stable_cnt = GET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_PHY_STABLE_CNT_THD);

	if(newbase_hdmi_get_device_phy_stable_count(port)!=0)
	{// special device use specified stable count
		phy_stable_cnt = newbase_hdmi_get_device_phy_stable_count(port);
	}

	//HDMI_EMG("[newbase_rxphy_is_clock_stable] port=%d, phy_stable_cnt=%d, b_clk=%d\n", port, phy_stable_cnt, phy_st[port].clk_pre);
	if (phy_st[port].clk_debounce_count >= CLK_DEBOUNCE_SHORT_CNT_THD)
	{
	       HDMI_HDCP_E hdcp_auth_mode = newbase_hdcp_get_auth_mode(port);
		if(NO_HDCP == hdcp_auth_mode || HDCP_OFF == hdcp_auth_mode)
		{
			if (phy_st[port].clk_debounce_count >=  phy_stable_cnt)
			{
				return TRUE;//Normal case: HDCP auth from TX is after rxphy_clock_stable
			}
			else
			{
				phy_st[port].clk_debounce_count++;
			}
		}
		else
		{
			return TRUE;//For special device case: HDCP auth from TX is near.
		}
	}
	else
	{
		phy_st[port].clk_debounce_count++;
	}

	return FALSE;
}

/*---------------------------------------------------
 * Func : newbase_hdmi_dfe_fw_le_scan
 *
 * Desc : calibrate le with CED, to find out the best
 *        le with the minimum character error rate
 *
 * Para : nport : HDMI port number
 *
 * Retn : port : target HDMI port
 *
 * Note : 0 : le parameter doesn't change
 *        1 : le parameter changed
 *--------------------------------------------------*/
int newbase_hdmi_dfe_fw_le_scan(unsigned char port)
{
	int i;
	int le[3] = {0, 0, 0};
	int tap0[3] = {0, 0, 0};
	unsigned int ced[3] = {0, 0, 0};
	unsigned int try_ofst[6] = {-1, 1, -2, 2, -3, 3};
	int best_ced = 0xFFFFFFFF;
	int best_le = le[1];
	unsigned int ced_threshold = GET_FLOW_CFG(HDMI_FLOW_CFG_DFE, HDMI_FLOW_CFG5_FW_LE_SCAN_CED_THRESHOLD);
	unsigned int period = 60;

	for (i=0; i<3; i++)
	{
		le[i] = lib_hdmi_dfe_get_le(port,i);
		tap0[i] = lib_hdmi_dfe_get_tap0(port,i);
	}

	DFE_WARN("HDMI[p%d] newbase_hdmi_dfe_fw_le_scan. init le=%d/%d/%d, tap0=%d/%d/%d ced_threshold=%d (ced_chk_cnt=%d)\n",
		port, le[0], le[1], le[2], tap0[0], tap0[1], tap0[2], ced_threshold, phy_st[port].ced_thr_chk_cnt);

	newbase_hdmi_fw_ced_measure(port, ced, period);



	if( (ced[0] ==32767 || ced[1]  ==32767  || ced[2] ==32767 )&&((phy_st[port].ced_thr_chk_cnt<15)))
	{
		DFE_WARN("HDMI[p%d] newbase_hdmi_dfe_fw_le_scan, Abnormal CED=%d/%d/%d, Need Reset PHY\n", port, ced[0], ced[1], ced[2]);
		phy_st[port].ced_thr_chk_cnt++;

		if (newbase_hdmi_tmds_scramble_off_sync(port) && phy_st[port].ced_thr_chk_cnt>=3)
		{
			DFE_WARN("HDMI[p%d] newbase_hdmi_dfe_fw_le_scan, Abnormal CED=%d/%d/%d with TMDS off sync, skip le scan\n", port, ced[0], ced[1], ced[2]);
			return LE_NO_CHANGE;
		}

		return NEED_RESET_PHY;
	}

	if (ced[0] < ced_threshold &&
		ced[1] < ced_threshold &&
		ced[2] < ced_threshold)
	{
		DFE_WARN("HDMI[p%d] newbase_hdmi_dfe_fw_le_scan, good CED=%d/%d/%d, skip le scan\n", port, ced[0], ced[1], ced[2]);
		return LE_NO_CHANGE;
	}

	DFE_WARN("HDMI[p%d] newbase_hdmi_dfe_fw_le_scan, bad CED=%d/%d/%d, do le scan \n", port, ced[0], ced[1], ced[2]);
	newbase_hdmi_set_badcable_flag(port,1);

	for (i=0 ; i <6 ; i++)
	{
		int nle = le[1] + try_ofst[i];

		if (IS_VALID_LE(nle)==0)   // invalid LE, skip it
			continue;

		lib_hdmi_dfe_update_le(port, le[0], nle, le[2]);  // update LE paramete

		newbase_hdmi_fw_ced_measure(port, ced, period);

		DFE_WARN("HDMI[p%d] newbase_hdmi_dfe_fw_le_scan. round=%d, LE=%d/%d(%d+(%d))/%d, ced=%d/%d/%d \n",
			port, i,
			lib_hdmi_dfe_get_le(port,0),
			lib_hdmi_dfe_get_le(port,1), le[1],try_ofst[i],
			lib_hdmi_dfe_get_le(port,2),
			ced[0], ced[1], ced[2]);

		if (ced[1] < best_ced)  // Note: here we only handled the Ch1, for R/B Channel , need to take care of BR swap function
		{
			best_ced = ced[1];
			best_le = nle;
		}

		if (best_ced==0)
			break;
	}

	lib_hdmi_dfe_update_le(port, le[0], best_le, le[2]);

	DFE_WARN("HDMI[p%d] newbase_hdmi_dfe_fw_le_scan Best LE=%d->%d (CED=%d)\n", port, le[1], best_le, best_ced);

	if (best_le != le[1])
	{
		phy_st[port].dfe_t[1].le = lib_hdmi_dfe_get_le(port, 1);  // update ch1 le parameter
		newbase_hdmi_dump_dfe_para(port, phy_st[port].lane_num);
		return LE_CHANGED;
	}

	return LE_NO_CHANGE;
}

void newbase_rxphy_job(unsigned char p)
{
	switch (phy_st[p].phy_proc_state) {
	case PHY_PROC_INIT:

		if (newbase_hdmi_set_phy(p, phy_st[p].clk,phy_st[p].frl_mode,phy_st[p].lane_num))
			newbase_rxphy_reset_setphy_proc(p);

		newbase_hdmi_reset_meas_counter(p);
		newbase_rxphy_set_apply_clock(p,phy_st[p].clk);
		newbase_hdmi_open_err_detect(p);

		if (phy_st[p].clk <= clock_bound_45m) {
			phy_st[p].phy_proc_state= PHY_PROC_CLOSE_ADP;
		} else {
			phy_st[p].phy_proc_state= PHY_PROC_RECORD;
		}
		break;
	case PHY_PROC_RECORD:
		if (newbase_hdmi_dfe_record(p, phy_st[p].clk)) {
			phy_st[p].phy_proc_state = PHY_PROC_CLOSE_ADP;
			// go to PHY_PROC_CLOSE_ADP without taking a break;
		} else {
			break;
		}
	case PHY_PROC_CLOSE_ADP:
		if (newbase_hdmi_dfe_close(p, phy_st[p].clk,phy_st[p].lane_num,PHY_PROC_CLOSE_ADP)) {

			newbase_hdmi2p0_detect_config(p);

			if (GET_FLOW_CFG(HDMI_FLOW_CFG_DFE, HDMI_FLOW_CFG5_FW_LE_SCAN_ENABLE))
				phy_st[p].phy_proc_state = PHY_PROC_FW_LE_SCAN;
			else
				phy_st[p].phy_proc_state = PHY_PROC_DONE;

#if BIST_DFE_SCAN
			if (lib_hdmi_get_fw_debug_bit(DEBUG_30_DFE_SCAN_TEST) && (phy_st[p].clk > clock_bound_45m)) {
				debug_hdmi_dfe_scan(p,phy_st[p].lane_num);
			}
#endif
#if BIST_PHY_SCAN
		if (lib_hdmi_get_fw_debug_bit(DEBUG_22_PHY_SCAN_TEST))
			debug_hdmi_phy_scan(p);
#endif

		} else {
			newbase_rxphy_reset_setphy_proc(p);
		}
		break;

	case PHY_PROC_FW_LE_SCAN:
		if (GET_FLOW_CFG(HDMI_FLOW_CFG_DFE, HDMI_FLOW_CFG5_FW_LE_SCAN_ENABLE))
		{
			if(newbase_hdmi_dfe_fw_le_scan(p)==NEED_RESET_PHY)
			{
				newbase_rxphy_reset_setphy_proc(p);
			}
			else
			{
				phy_st[p].phy_proc_state = PHY_PROC_DONE;
			}

		}
		else
		{
			phy_st[p].phy_proc_state = PHY_PROC_DONE;
		}
		break;

	case PHY_PROC_DONE:
		phy_st[p].recovery = (phy_st[p].le_re_adaptive) ? 1 : 0;  // setup recovery flag is le_readaptive is done
		if (lib_hdmi_get_fw_debug_bit(DEBUG_18_PHY_DBG_MSG))
			debug_hdmi_dump_msg(p);
		break;




	/**********************************************************************/
	//
	//	Belowing are for recovery flow in check_mode stage.
	//
	/**********************************************************************/
	case PHY_PROC_RECOVER_6G_LONGCABLE:
		newbase_hdmi_dfe_hi_speed(p, phy_st[p].clk,phy_st[p].lane_num);
		newbase_hdmi_err_detect_add(p);
		phy_st[p].phy_proc_state = PHY_PROC_RECOVER_6G_LONGCABLE_RECORD;
		break;
	case PHY_PROC_RECOVER_6G_LONGCABLE_RECORD:
		if (newbase_hdmi_dfe_record(p, phy_st[p].clk)) {
			phy_st[p].phy_proc_state = PHY_PROC_RECOVER_6G_LONGCABLE_CLOSE_ADP;
			// go to PHY_PROC_RECOVER_HI_CLOSE_ADP;
		} else {
			break;
		}
	case PHY_PROC_RECOVER_6G_LONGCABLE_CLOSE_ADP:
		newbase_hdmi_dfe_close(p, phy_st[p].clk, phy_st[p].lane_num,PHY_PROC_RECOVER_6G_LONGCABLE_CLOSE_ADP);
		if (GET_FLOW_CFG(HDMI_FLOW_CFG_DFE, HDMI_FLOW_CFG5_FW_LE_SCAN_ENABLE))
			phy_st[p].phy_proc_state = PHY_PROC_FW_LE_SCAN;
		else
			phy_st[p].phy_proc_state = PHY_PROC_DONE;
		break;

	case PHY_PROC_RECOVER_HI:
		newbase_hdmi_dfe_hi_speed(p, phy_st[p].clk,phy_st[p].lane_num);
		newbase_hdmi_err_detect_add(p);
		phy_st[p].phy_proc_state = PHY_PROC_RECORD;
		break;

	case PHY_PROC_RECOVER_MI:
		newbase_hdmi_dfe_mi_speed(p, phy_st[p].clk,phy_st[p].lane_num);
		newbase_hdmi_err_detect_add(p);
		phy_st[p].phy_proc_state = PHY_PROC_RECORD;
		break;

	case PHY_PROC_RECOVER_MID_BAND_LONGCABLE:
		newbasse_hdmi_dfe_mid_adapthve(p, phy_st[p].clk);
		newbase_hdmi_err_detect_add(p);	
		if (GET_FLOW_CFG(HDMI_FLOW_CFG_DFE, HDMI_FLOW_CFG5_FW_LE_SCAN_ENABLE))
			phy_st[p].phy_proc_state = PHY_PROC_FW_LE_SCAN;
		else
			phy_st[p].phy_proc_state = PHY_PROC_DONE;
		break;

	default:
		phy_st[p].phy_proc_state = PHY_PROC_INIT;
		break;
	}

}

void newbase_rxphy_force_clear_clk_st(unsigned char port)
{
	// case 1: clk=1080p50->1080p60, timing changed, but clock is the same, then force to re-set phy
	// case 2: clk=4k2kp60->1080p60, timing changed, but clock is the same, then force to re-set phy (also need to clear SCDC)
	//
#if BIST_DFE_SCAN
	if (lib_hdmi_get_fw_debug_bit(DEBUG_30_DFE_SCAN_TEST)) return;
#endif
	newbase_rxphy_init_struct(port);
}

unsigned int newbase_rxphy_get_clk(unsigned char port)
{
	return phy_st[port].clk;
}
unsigned int newbase_rxphy_get_clk_pre(unsigned char port)
{
	return phy_st[port].clk_pre;
}

unsigned char newbase_rxphy_is_clk_stable(unsigned char port)
{
	//if frl mode no check clk, fix me
	if (hdmi_rx[port].hdmi_2p1_en) {
		return 1;
	}
	return phy_st[port].clk_stable;
}

void newbase_rxphy_set_apply_clock(unsigned char port,unsigned int b)
{
	phy_st[port].apply_clk = b;
}

unsigned int newbase_rxphy_get_apply_clock(unsigned char port)
{
	return phy_st[port].apply_clk;
}

unsigned char newbase_rxphy_is_tmds_mode(unsigned char port)
{
	if (phy_st[port].frl_mode == MODE_TMDS)
		return 1;
	else
		return 0;

}

unsigned char newbase_rxphy_get_setphy_done(unsigned char port)
{

	return (phy_st[port].phy_proc_state == PHY_PROC_DONE);
}

void newbase_rxphy_reset_setphy_proc(unsigned char port)
{
	phy_st[port].clk_pre = 0;
	phy_st[port].clk= 0;
	phy_st[port].clk_debounce_count= 0;
	phy_st[port].clk_unstable_count = 0;
	phy_st[port].clk_stable = 0;

	//for frl mode
	phy_st[port].frl_mode = 0;
	phy_st[port].pre_frl_mode = 0;
	phy_st[port].phy_proc_state = PHY_PROC_INIT;
	HDMI_PRINTF("%s(%d)\n", __func__, port);
}

void newbase_hdmi_dfe_recovery_6g_long_cable(unsigned char port)
{
	phy_st[port].phy_proc_state = PHY_PROC_RECOVER_6G_LONGCABLE;
	phy_st[port].recovery = 1;
}

void newbase_hdmi_dfe_recovery_mi_speed(unsigned char port)
{
	phy_st[port].phy_proc_state = PHY_PROC_RECOVER_MI;
	phy_st[port].recovery = 1;
}


void newbase_hdmi_dfe_recovery_hi_speed(unsigned char port)
{
	phy_st[port].phy_proc_state = PHY_PROC_RECOVER_HI;
	phy_st[port].recovery = 1;
}


void newbase_hdmi_dfe_recovery_midband_long_cable(unsigned char port)
{
	phy_st[port].phy_proc_state = PHY_PROC_RECOVER_MID_BAND_LONGCABLE;
	phy_st[port].recovery = 1;

}

void newbase_hdmi_set_eq_mode(unsigned char nport, unsigned char mode)
{
	phy_fac_eq_st[nport].eq_mode = mode;

	if (phy_fac_eq_st[nport].eq_mode == EQ_MANUAL_OFF) {
		phy_st[nport].clk_pre = 0;
		phy_st[nport].clk= 0;
		phy_st[nport].clk_debounce_count= 0;
		phy_st[nport].clk_unstable_count = 0;
		phy_st[nport].clk_stable = 0;
		phy_st[nport].phy_proc_state = PHY_PROC_INIT;
	}

}


void newbase_hdmi_manual_eq_ch(unsigned char nport,unsigned char ch,unsigned char eq_ch)
{
//	unsigned char bgrc;
//	unsigned char mode;

#if HDMI_DFE_BRSWAP	
	if (ch == 0) {
		phy_fac_eq_st[nport].manual_eq[0] = eq_ch;
	}
	else if (ch == 1) {
		phy_fac_eq_st[nport].manual_eq[1] = eq_ch;
	}
	else if (ch == 2) {
		phy_fac_eq_st[nport].manual_eq[2] = eq_ch;
	} else {
		return;
	}
#else
	if (ch == 0) {
		phy_fac_eq_st[nport].manual_eq[0] = eq_ch;
	}
	else if (ch == 1) {
		phy_fac_eq_st[nport].manual_eq[1] = eq_ch;
	}
	else if (ch == 2) {
		phy_fac_eq_st[nport].manual_eq[2] = eq_ch;
	} else {
		return;
	}
#endif

	phy_st[nport].clk_pre = 0;
	phy_st[nport].clk= 0;
	phy_st[nport].clk_debounce_count= 0;
	phy_st[nport].clk_unstable_count = 0;
	phy_st[nport].clk_stable = 0;
	phy_st[nport].phy_proc_state = PHY_PROC_INIT;
	

}



void newbase_hdmi_manual_eq(unsigned char nport, unsigned char eq_ch0, unsigned char eq_ch1, unsigned char eq_ch2)
{
 #if 0
	if (phy_fac_eq_st[nport].eq_mode == EQ_MANUAL_OFF)
		return;

	phy_fac_eq_st[nport].manual_eq[0] = eq_ch0;
	phy_fac_eq_st[nport].manual_eq[1] = eq_ch1;
	phy_fac_eq_st[nport].manual_eq[2] = eq_ch2;
	
	lib_hdmi_dfe_update_le(nport, eq_ch0, eq_ch1, eq_ch2);
#endif
}


unsigned char newbase_hdmi_set_phy(unsigned char port, unsigned int b, unsigned char frl_mode,unsigned char lane)
{  
	//unsigned char mode;
	//bool b_mode_found = 0;
	//HDMI_PHY_PARAM_T *p_phy_table = hdmi_phy_param;
	//unsigned char tb_size = 0;
	TMDS_VALUE_PARAM_T TMDS_Timing = TMDS_5p94G;
	unsigned int Delta_b=30;//30 ~=3MHz TMDS_CLK = (b x 27)/256	
	
	HDMI_PRINTF("%s start[p%d]=%d frl=%d lane=%d\n", __func__, port, b,frl_mode,lane);


	if (b == 0) return 1;

#ifdef HDMI_FIX_HDMI_POWER_SAVING
	newbase_hdmi_reset_power_saving_state(port);
#endif

	lib_hdmi_mac_reset(port);

	lib_hdmi_cdr_rst(port, 1);
	lib_hdmi_demux_ck_vcopll_rst(port, 1);
	lib_hdmi_wdg_rst(port, 1);
	lib_hdmi_cmu_pll_en(port, 0);

////////////////
//TMDS clock define
//#define TMDS_6G		5688
//#define TMDS_5G		4740
//#define TMDS_4G		3792
//#define TMDS_3G		2844
//#define TMDS_1p5G	1422
//#define TMDS_742M    704
//#define TMDS_270M    255
//// TMDS_CLK = (b x 27)/256 ,  b= TMDS_CLK*256/27

	if (frl_mode == 0)
	{
		HDMI_WARN("HDMI Port[%d] frl_mode == MODE_TMDS!! (b=%d)\n", port, b);

		if (b > (4214+Delta_b)) 
		{
			TMDS_Timing= TMDS_5p94G;
			HDMI_WARN("HDMI Port[%d]  TMDS_5_94G! (b=%d)\n", port, b);
		} 
		else if ((b > (3520+Delta_b)) && (b <= (4214+Delta_b))) 
		{
			TMDS_Timing= TMDS_4p445G;
			HDMI_WARN("HDMI Port[%d]  TMDS_4_445G! (b=%d)\n", port, b);
		} 

		else if ((b > (3168+Delta_b)) && (b <= (3520+Delta_b))) 
		{
			TMDS_Timing= TMDS_3p7125G;
			HDMI_WARN("HDMI Port[%d]  TMDS_3_7125G! (b=%d)\n", port, b);
		} 
		else if ((b > (2122+Delta_b)) && (b <= (3816+Delta_b))) 
		{
			TMDS_Timing= TMDS_2p97G;
			HDMI_WARN("HDMI Port[%d]  TMDS_2_97G! (b=%d)\n", port, b);
		} 
		else if ((b > (1760+Delta_b)) && (b <= (2122+Delta_b))) 
		{
			TMDS_Timing= TMDS_2p2275G;
			HDMI_WARN("HDMI Port[%d]  TMDS_2_2275G! (b=%d)\n", port, b);
		} 

		else if ((b > (1408+Delta_b)) && (b <= (1760+Delta_b))) 
		{
			TMDS_Timing= TMDS_1p85625G;
			HDMI_WARN("HDMI Port[%d]  TMDS_1_85625G! (b=%d)\n", port, b);
		} 
		else if ((b > (1056+Delta_b)) && (b <= (1408+Delta_b))) 
		{
			TMDS_Timing= TMDS_1p485G;
			HDMI_WARN("HDMI Port[%d]  TMDS_1_485G! (b=%d)\n", port, b);
		} 
		else if ((b > (968+Delta_b)) && (b <=(1056+Delta_b))) 
		{
			TMDS_Timing= TMDS_1p11375G;
			HDMI_WARN("HDMI Port[%d]  TMDS_1_11375G! (b=%d)\n", port, b);
		} 
		else if ((b > (792+Delta_b)) && (b <=(968+Delta_b)))
		{
			TMDS_Timing= TMDS_0p928125G;
			HDMI_WARN("HDMI Port[%d]  TMDS_0_928125G! (b=%d)\n", port, b);
		}
		else if ((b > (512+Delta_b)) && (b <=(792+Delta_b)))
		{
			TMDS_Timing= TMDS_0p742G;
			HDMI_WARN("HDMI Port[%d]  TMDS_0_742G! (b=%d)\n", port, b);
		}
		else if ((b > (384+Delta_b)) && (b <=(512+Delta_b))) 
		{
			TMDS_Timing= TMDS_0p54G;
			HDMI_WARN("HDMI Port[%d]  TMDS_0_54G! (b=%d)\n", port, b);
		} 
		else if ((b > (352+Delta_b)) && (b <=(384+Delta_b)))
		{
			TMDS_Timing= TMDS_0p405G;
			HDMI_WARN("HDMI Port[%d]  TMDS_0_405G! (b=%d)\n", port, b);
		}

		else if ((b > (252+Delta_b)) && (b <=(352+Delta_b)))
		{
			TMDS_Timing= TMDS_0p3375G;
			HDMI_WARN("HDMI Port[%d]  TMDS_0_3375G! (b=%d)\n", port, b);
		}
		else if ((b > (238+Delta_b)) && (b <=(252+Delta_b)))
		{
			TMDS_Timing= TMDS_0p27G;
			HDMI_WARN("HDMI Port[%d]  TMDS_0_27G! (b=%d)\n", port, b);
		}
		else		
		{
			TMDS_Timing= TMDS_0p25G;
			HDMI_WARN("HDMI Port[%d]  TMDS_0_25G! (b=%d)\n", port, b);
		}

	}

#ifdef CONFIG_POWER_SAVING_MODE
	lib_hdmi_cdr_rst(port, 0);
	lib_hdmi_demux_ck_vcopll_rst(port, 0);
	//lib_hdmi_wdg_rst(port, 1);
	lib_hdmi_cmu_pll_en(port, 1);
#endif


	if (port == HDMI_PORT0) {
		Mac8p_Pure_HDMI_TMDS_Main_Seq(port,TMDS_Timing);
	}
	else
	{
		Mac8p_TMDS_setting_Main_Seq(port,TMDS_Timing);
	}	

	udelay(1);
	
	if(newbase_hdcp_get_auth_mode(port) == HDCP_OFF || newbase_hdcp_get_auth_mode(port) == NO_HDCP )
	{// No AKE_INIT of HDCP22, open HDCP14 and HDCP22 at the same time
		HDMI_WARN("HDMI Port[%d] HDCP Enable after set phy!! (b=%d)\n", port, b);
		newbase_hdmi_switch_hdcp_mode(port, NO_HDCP);
		newbase_hdmi_reset_hdcp_once(port, FALSE);
	}
	else
	{
		HDMI_WARN("HDMI Port[%d] HDCP already start before set phy!! (b=%d)\n", port, b);
	}

	lib_hdmi_mac_release(port, frl_mode);
	
	return 0;

}

unsigned char newbase_hdmi_dfe_record(unsigned char nport, unsigned int clk)
{
	if (lib_hdmi_dfe_is_record_enable(nport)) {
		lib_hdmi_dfe_record_enable(nport, 0);
		return TRUE; // stop recocrding
	} else {
		lib_hdmi_dfe_record_enable(nport, 1);
		return FALSE; // start recoding min/max
	}
}

unsigned char newbase_hdmi_dfe_close(unsigned char nport, unsigned int clk, unsigned char lane_mode,unsigned char phy_state)
{
	unsigned char lane;
	
	lib_hdmi_set_dfe_close(nport,lane_mode);

	 if (!lib_hdmi_clock40x_sync_status(nport)) {
                HDMI_EMG("%s: scdc sync status=0\n", __func__);
        }

	switch (phy_state) {
	case PHY_PROC_CLOSE_ADP:
	#if	0
		if (clk > 2900) { // > 3G
			lib_hdmi_load_le_max(nport,lane_mode);
		} else {
			lib_hdmi_dfe_examine_le_coef(nport, clk,lane_mode);
		}

		newbase_hdmi_tap2_adjust_with_le(nport,clk,lane_mode);
			
		if (newbase_hdmi_dfe_threshold_check(nport,clk,lane_mode) && (phy_st[nport].dfe_thr_chk_cnt < 15)) {
			phy_st[nport].dfe_thr_chk_cnt++;
			return FALSE;
		}
    #endif
		break;
	case PHY_PROC_RECOVER_6G_LONGCABLE_CLOSE_ADP:
		newbase_hdmi_dfe_6g_long_cable_patch(nport); //check FRL mode need this patch or not ?
		break;
	default:
		break;
	}

	
	if (phy_fac_eq_st[nport].eq_mode == EQ_MANUAL_ON) {		
		newbase_hdmi_manual_eq(nport,	phy_fac_eq_st[nport].manual_eq[0] ,phy_fac_eq_st[nport].manual_eq[1] ,phy_fac_eq_st[nport].manual_eq[2]); 
	}

#if	1
	for (lane=0; lane<3; lane++) {
		phy_st[nport].dfe_t[lane].tap0max = lib_hdmi_dfe_get_tap0max(nport, lane);
		phy_st[nport].dfe_t[lane].tap0min = lib_hdmi_dfe_get_tap0min(nport, lane);
		phy_st[nport].dfe_t[lane].vth = lib_hdmi_dfe_get_vth(nport, lane);
		phy_st[nport].dfe_t[lane].tap0 = lib_hdmi_dfe_get_tap0(nport, lane);
		phy_st[nport].dfe_t[lane].tap1 = lib_hdmi_dfe_get_tap1(nport, lane);
		phy_st[nport].dfe_t[lane].tap2 = lib_hdmi_dfe_get_tap2(nport, lane);
		phy_st[nport].dfe_t[lane].le = lib_hdmi_dfe_get_le(nport, lane);
		phy_st[nport].dfe_t[lane].lemax = lib_hdmi_dfe_get_lemax(nport, lane);
		phy_st[nport].dfe_t[lane].lemin = lib_hdmi_dfe_get_lemin(nport, lane);
	}
#endif	
	newbase_hdmi_dump_dfe_para(nport,lane_mode);


	return TRUE;
}



void newbase_hdmi_dfe_hi_speed(unsigned char nport, unsigned int b_clk,unsigned char lane_mode)
{
#if 0
 	if (nport == HDMI_PORT0) {
	unsigned char bgrc;
	HDMI_PRINTF("%s\n", __func__);

	lib_hdmi_dfe_power(nport, 1,lane_mode);

	// ENABLE  VTH & TAP0
	if (lane_mode == HDMI_4LANE)
		bgrc = LN_ALL;
	else
		bgrc = (LN_R|LN_G|LN_B);
	
	lib_hdmi_dfe_init_vth(nport, bgrc, 0x05);
	lib_hdmi_dfe_init_tap0(nport, bgrc, 0x14);

	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth),(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth));
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth),(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth));
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth),(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth));

	lib_hdmi_dfe_second_flow(nport, b_clk,lane_mode);

	lib_hdmi_dfe_init_tap1(nport, bgrc, 0x0c);
	lib_hdmi_dfe_init_tap2(nport, bgrc, 0x00);
	lib_hdmi_dfe_init_le(nport, bgrc, 0x08);


	//set tap tap1 tap2 max min value
	if (b_clk < 2844 && b_clk > 1420) {   //  3G ~1.5G
		hdmi_mask(DFE_REG_DFE_LIMIT0_reg,
			~(DFE_P0_REG_DFE_LIMIT0_P0_tap1_min_p0_mask|DFE_P0_REG_DFE_LIMIT0_P0_tap2_max_p0_mask|DFE_P0_REG_DFE_LIMIT0_P0_tap2_min_p0_mask),
			(DFE_P0_REG_DFE_LIMIT0_P0_tap1_min_p0(0x10)|DFE_P0_REG_DFE_LIMIT0_P0_tap2_max_p0(0x14)|DFE_P0_REG_DFE_LIMIT0_P0_tap2_min_p0(0xa)));
		hdmi_mask(DFE_REG_DFE_LIMIT2_reg, ~(DFE_P0_REG_DFE_LIMIT2_P0_tap0_max_p0_mask), (DFE_P0_REG_DFE_LIMIT2_P0_tap0_max_p0(0x24)));
	} else {
		hdmi_mask(DFE_REG_DFE_LIMIT0_reg,
			~(DFE_P0_REG_DFE_LIMIT0_P0_tap1_min_p0_mask|DFE_P0_REG_DFE_LIMIT0_P0_tap2_max_p0_mask|DFE_P0_REG_DFE_LIMIT0_P0_tap2_min_p0_mask),
// LCB6			(DFE_REG_DFE_LIMIT0_tap1_min(0x35)|DFE_REG_DFE_LIMIT0_tap2_max(0x0a)|DFE_REG_DFE_LIMIT0_tap2_min(0x5)));
//			(DFE_P0_REG_DFE_LIMIT0_P0_tap1_min_p0(0x35)|DFE_P0_REG_DFE_LIMIT0_P0_tap2_max_p0(0x14)|DFE_P0_REG_DFE_LIMIT0_P0_tap2_min_p0(0x0)));
			(DFE_P0_REG_DFE_LIMIT0_P0_tap1_min_p0(0x35)|DFE_P0_REG_DFE_LIMIT0_P0_tap2_max_p0(0x14)|DFE_P0_REG_DFE_LIMIT0_P0_tap2_min_p0(0x5)));
		hdmi_mask(DFE_REG_DFE_LIMIT2_reg,~(DFE_P0_REG_DFE_LIMIT2_P0_tap0_max_p0_mask),(DFE_P0_REG_DFE_LIMIT2_P0_tap0_max_p0(0x24)));
	}



	if (b_clk > 2700) {  // 3G need tap2 but below not need
		hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_LE|REG_dfe_adapt_en_lane0_TAP2),(REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_LE|REG_dfe_adapt_en_lane0_TAP2));
		hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_LE|REG_dfe_adapt_en_lane1_TAP2),(REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_LE|REG_dfe_adapt_en_lane1_TAP2));
		hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_LE|REG_dfe_adapt_en_lane2_TAP2),(REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_LE|REG_dfe_adapt_en_lane2_TAP2));
	} else {
		hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_LE|REG_dfe_adapt_en_lane0_TAP2),(REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_LE));
		hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_LE|REG_dfe_adapt_en_lane1_TAP2),(REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_LE));
		hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_LE|REG_dfe_adapt_en_lane2_TAP2),(REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_LE));

	}

 	}
#endif
}

void newbase_hdmi_dfe_6g_long_cable_patch(unsigned char nport)
{
 #if 0
	signed char tap2;
	unsigned char lemax;

	tap2 = lib_hdmi_dfe_get_tap2(nport, 1);
	lib_hdmi_dfe_init_tap2(nport, LN_G, MIN(tap2+6, 10));

	lemax = lib_hdmi_dfe_get_lemax(nport, 1);
	lemax += lib_hdmi_dfe_get_tap1max(nport, 1);
	lib_hdmi_dfe_init_tap1(nport, LN_G, MIN(lemax+4, 63));

	HDMI_EMG("%s end\n", __func__);
#endif	
}


void newbase_hdmi_dfe_mi_speed(unsigned char nport, unsigned int b_clk,unsigned char lane_mode)
{
	HDMI_PRINTF("%s\n", __func__);
#if	0
	if (nport == HDMI_PORT0) {
		lib_hdmi_dfe_power(nport, 1,lane_mode);

		// ENABLE  VTH & TAP0
		hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth),(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth));
		hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth),(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth));
		hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth),(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth));

		udelay(1);

		lib_hdmi_dfe_second_flow(nport, b_clk,lane_mode);
	//	lib_hdmi_fg_koffset_proc(nport, lane_mode);
		
	//	lib_hdmi_tap0_fix_small_swing(nport,lane_mode);
	//	lib_hdmi_fg_koffset_proc(nport, lane_mode);

		if (b_clk > 2700) {  // 3G need tap2 but below not need
			hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_LE|REG_dfe_adapt_en_lane0_TAP2),(REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_LE|REG_dfe_adapt_en_lane0_TAP2));
			hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_LE|REG_dfe_adapt_en_lane1_TAP2),(REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_LE|REG_dfe_adapt_en_lane1_TAP2));
			hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_LE|REG_dfe_adapt_en_lane2_TAP2),(REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_LE|REG_dfe_adapt_en_lane2_TAP2));

		} else {
			hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_LE|REG_dfe_adapt_en_lane0_TAP2),(REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_LE));
			hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_LE|REG_dfe_adapt_en_lane1_TAP2),(REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_LE));
			hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_LE|REG_dfe_adapt_en_lane2_TAP2),(REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_LE));
		}

	}
#endif	
}

void newbasse_hdmi_dfe_mid_adapthve(unsigned char nport, unsigned int b_clk)
{
#if 0
	unsigned char le0;

//	HDMI_PRINTF("%s\n", __func__);
	//le0 = lib_hdmi_dfe_get_le(nport,0);
	le0 = phy_st[nport].dfe_t[0].le ;


	if (le0 >= 8)
		return;
//	le1 = lib_hdmi_dfe_get_le(nport,1);
//	le2 = lib_hdmi_dfe_get_le(nport,2);

	lib_hdmi_dfe_init_le(nport, (LN_R|LN_G|LN_B), (le0 + 2));
 	phy_st[nport].dfe_t[0].le=le0+2;
 	phy_st[nport].dfe_t[1].le=le0+2;
 	phy_st[nport].dfe_t[1].le=le0+2;
	HDMI_PRINTF("le=%x\n", (le0 + 2));
#endif
}


void newbase_hdmi_reset_thr_cnt(unsigned char port)
{
	//le  abnormal counter
	phy_st[port].dfe_thr_chk_cnt = 0;
	phy_st[port].ced_thr_chk_cnt=0;
}

void newbase_hdmi_open_err_detect(unsigned char port)
{
	//re-adaptive counter
	phy_st[port].error_detect_count = 0;
	
}


void newbase_hdmi_err_detect_add(unsigned char port)
{
		if (phy_st[port].error_detect_count == 0)
			HDMI_EMG("eq adjust count start\n");
		
		if (phy_st[port].error_detect_count < MAX_ERR_DETECT_TIMES) {
			++phy_st[port].error_detect_count;
			HDMI_EMG("cnt = %d\n",phy_st[port].error_detect_count);
		}

		if (phy_st[port].error_detect_count >= MAX_ERR_DETECT_TIMES)
			HDMI_EMG("eq adjust count end\n");
}

unsigned char newbase_hdmi_err_detect_stop(unsigned char port)
{
	if (phy_st[port].error_detect_count >= MAX_ERR_DETECT_TIMES)
		return 1;
	else
		return 0;
}

unsigned char newbase_hdmi_get_err_recovery(unsigned char port)
{
	return phy_st[port].recovery;
}


unsigned char newbase_hdmi_char_err_detection(unsigned char port,unsigned int *r_err, unsigned int *g_err, unsigned int *b_err)
{

	if (phy_st[port].char_err_loop == 0) {
		lib_hdmi_char_err_start(port, 0, TMDS_CHAR_ERR_PERIOD);
		phy_st[port].char_err_loop++;

	} else {
		if (lib_hdmi_char_err_get_error(port, r_err, g_err, b_err)) {
			HDMI_PORT_INFO_T* p_hdmi_rx_st = newbase_hdmi_get_rx_port_info(port);

			phy_st[port].char_err_loop = 0;
			newbase_hdmi_ced_error_cnt_accumulated(port, b_err, g_err, r_err);

			if(p_hdmi_rx_st != NULL)
			{
				p_hdmi_rx_st->fw_char_error[0] = *r_err;
				p_hdmi_rx_st->fw_char_error[1] = *g_err;
				p_hdmi_rx_st->fw_char_error[2] = *b_err;
			}
			else
			{
				HDMI_EMG("[newbase_hdmi_char_err_detection] NULL hdmi_rx, port = %d\n", port);
			}
			if ((*r_err > TMDS_CHAR_ERR_THREAD) || (*g_err > TMDS_CHAR_ERR_THREAD) || (*b_err > TMDS_CHAR_ERR_THREAD)) {
				// Do something to save the world
				HDMI_PRINTF("[CHAR_ERR](%d, %d, %d)\n", *b_err, *g_err, *r_err);
				return 1;
			}
			return 0;
		}

		if (phy_st[port].char_err_loop > TMDS_CHAR_ERR_MAX_LOOP) {
			phy_st[port].char_err_loop = 0;
			newbase_hdmi_set_6G_long_cable_enable(port , 0);//disable 6g long cable detect
			HDMI_EMG("[CHAR_ERR] Life is too short.\n");
		} else {
			phy_st[port].char_err_loop++;
		}

	}
#if 0
	if (phy_st[port].char_err_loop == 0) {
		lib_hdmi_char_err_start(port, 0, TMDS_CHAR_ERR_MAX_LOOP);
		phy_st[port].char_err_loop++;

	} else if (phy_st[port].char_err_loop > TMDS_CHAR_ERR_MAX_LOOP) {
		phy_st[port].char_err_loop = 0;
		phy_st[port].detect_6g_longcable = 0;

		if (lib_hdmi_char_err_get_error(port, &r_err, &g_err, &b_err)) {

			if ((g_err > TMDS_CHAR_ERR_THREAD) && (r_err < 10) && (b_err < 10) && ((phy_st[port].dfe_t[1].le+phy_st[port].dfe_t[1].tap1)>10)) {
				phy_st[port].detect_6g_longcable = 1;
			}

			if ((r_err > TMDS_CHAR_ERR_THREAD) || (g_err > TMDS_CHAR_ERR_THREAD) || (b_err > TMDS_CHAR_ERR_THREAD)) {
				// Do something to save the world
				HDMI_PRINTF("[CHAR_ERR](%d, %d, %d)\n", r_err, g_err, b_err);
			}
			return 1;
		}

	} else {
		phy_st[port].char_err_loop++;
	}
#endif
	return 0;

}
unsigned char newbase_hdmi_detect_6g_longcable(unsigned char port,unsigned int r_err,unsigned int g_err,unsigned int b_err)
{
	if(phy_st[port].detect_6g_longcable_enable == 0) //char_err_loop > TMDS_CHAR_ERR_MAX_LOOP will disable 6g long cable detect
		return 0;

	if ((g_err > TMDS_CHAR_ERR_THREAD) && (r_err < 10) && (b_err < 10) && ((phy_st[port].dfe_t[1].le+phy_st[port].dfe_t[1].tap1)>10)) {
		return 1;
	}

	return 0;
}


unsigned char newbase_hdmi_get_longcable_flag(unsigned char port)
{
	return phy_st[port].longcable_flag;
}

void newbase_hdmi_set_longcable_flag(unsigned char port, unsigned char enable)
{
	 phy_st[port].longcable_flag = enable;
}

unsigned char newbase_hdmi_get_badcable_flag(unsigned char port)
{
	return phy_st[port].badcable_flag;
}

void newbase_hdmi_set_badcable_flag(unsigned char port, unsigned char enable)
{
	 phy_st[port].badcable_flag = enable;
}


unsigned char newbase_hdmi_bit_err_detection(unsigned char port, unsigned int *r_err, unsigned int *g_err, unsigned int *b_err)
{
        unsigned char bit_err_det = 0;

	if (phy_st[port].bit_err_loop == 0)
	{
		lib_hdmi_bit_err_start(port, 0, TMDS_BIT_ERR_PERIOD);
		phy_st[port].bit_err_loop++;
	}
	else
	{
		if (lib_hdmi_bit_err_get_error(port, r_err, g_err, b_err))
		{
			HDMI_PORT_INFO_T* p_hdmi_rx_st = newbase_hdmi_get_rx_port_info(port);

			phy_st[port].bit_err_loop = 0;
			phy_st[port].bit_err_occurred = 0;
			if(p_hdmi_rx_st != NULL)
			{
				p_hdmi_rx_st->fw_bit_error[0] = *r_err;
				p_hdmi_rx_st->fw_bit_error[1] = *g_err;
				p_hdmi_rx_st->fw_bit_error[2] = *b_err;
			}
			else
			{
				HDMI_EMG("[newbase_hdmi_bit_err_detection] NULL hdmi_rx, port = %d\n", port);
			}

			if ((*r_err > TMDS_BIT_ERR_THREAD) || (*g_err > TMDS_BIT_ERR_THREAD) || (*b_err > TMDS_BIT_ERR_THREAD)) {
				// Do something to save the world
				HDMI_PRINTF("[BIT_ERR](%d, %d, %d)\n", *r_err, *g_err, *b_err);
				bit_err_det = 1;
			}


                         //USER:Lewislee DATE:2017/03/20
			//for KTASKWBS-6722, TATASKY 4K STB + HDMI Cable : Maxicom (Non standard)
			//The g_err bit error very large issue
			if((*g_err > TMDS_BIT_ERR_SIGNAL_BAD_THREAD) && (*r_err < 10) && (*b_err < 10))
			{
					phy_st[port].bit_err_occurred = 1;
					// Do something to save the world
					HDMI_PRINTF("[BIT_ERR] Signal BAD Re-Auto EQ\n");
					bit_err_det = 1;
			}

				if (bit_err_det)
					return 1;
				else
					return 0;

		}

		if (phy_st[port].bit_err_loop > TMDS_BIT_ERR_MAX_LOOP) {
			phy_st[port].bit_err_loop = 0;
			phy_st[port].bit_err_occurred = 0;
			HDMI_PRINTF("[BIT_ERR] Life is too short.\n");
		} else {
			phy_st[port].bit_err_loop++;
		}

	}

	return 0;

}

unsigned char newbase_hdmi_set_6G_long_cable_enable(unsigned char port, unsigned char enable)
{
	return phy_st[port].detect_6g_longcable_enable = enable;
}

/*---------------------------------------------------
 * Func : newbase_hdmi_ced_error_cnt_accumulated
 *
 * Desc : scdc read will clear zero between offset 0x50 ~ 0x55
 *
 * Para : nport : HDMI port number
 *        err    : ced error of detet one times mode
 *
 * Retn : N/A
 *--------------------------------------------------*/
void newbase_hdmi_ced_error_cnt_accumulated(unsigned char port, unsigned int *b_err, unsigned int *g_err, unsigned int *r_err)
{
	if(0x7FFF - phy_st[port].char_error_cnt[0] < *b_err)
		phy_st[port].char_error_cnt[0] = 0x7FFF;
	else
		phy_st[port].char_error_cnt[0] += *b_err ;

	if(0x7FFF - phy_st[port].char_error_cnt[1] < *g_err)
		phy_st[port].char_error_cnt[1] = 0x7FFF;
	else
		phy_st[port].char_error_cnt[1] += *g_err ;

	if(0x7FFF - phy_st[port].char_error_cnt[2] < *r_err)
		phy_st[port].char_error_cnt[2] = 0x7FFF;
	else
		phy_st[port].char_error_cnt[2] += *r_err ;

}
void newbase_hdmi_ced_error_cnt_reset(unsigned char port)
{
	phy_st[port].char_error_cnt[0] = 0;
	phy_st[port].char_error_cnt[1] = 0;
	phy_st[port].char_error_cnt[2] = 0;
	phy_st[port].char_error_cnt[3] = 0;
}
unsigned short newbase_hdmi_get_ced_error_cnt(unsigned char port , unsigned char ch)
{
	return phy_st[port].char_error_cnt[ch];
}


unsigned char newbase_hdmi_is_bit_error_occured(unsigned char port)
{
	return phy_st[port].bit_err_occurred;
}

void newbase_hdmi_dump_dfe_para(unsigned char nport, unsigned char lane_mode)
{
	unsigned char lane;

#if HDMI_DFE_BRSWAP
	unsigned char j;
	if (nport >= HD20_PORT) {
		for (j=0; j<lane_mode; j++) {
			
			if (j == 0)
				lane = 2;
			else if (j == 2)
				lane = 0;
			else
				lane = j;
			
			DFE_WARN("HDMI[p%d] ********************Lane%d******************\n", nport, j);
			DFE_WARN("HDMI[p%d] Lane%d Vth=0x%x\n", nport, j, phy_st[nport].dfe_t[lane].vth);
			DFE_WARN("HDMI[p%d] Lane%d Tap0=0x%x, max=0x%02x, min=0x%02x\n", nport,
				j, phy_st[nport].dfe_t[lane].tap0, phy_st[nport].dfe_t[lane].tap0max, phy_st[nport].dfe_t[lane].tap0min);

			DFE_WARN("HDMI[p%d] Lane%d Tap1=0x%02x, Tap2=%+d\n",  nport,
				j, phy_st[nport].dfe_t[lane].tap1, phy_st[nport].dfe_t[lane].tap2);

			DFE_WARN("HDMI[p%d] Lane%d LE=0x%02x, max=0x%02x, min=0x%02x\n", nport,
				j, phy_st[nport].dfe_t[lane].le, phy_st[nport].dfe_t[lane].lemax, phy_st[nport].dfe_t[lane].lemin);
		}
	} else {

		for (lane=0; lane<lane_mode; lane++) {
			DFE_WARN("HDMI[p%d] ********************Lane%d******************\n", nport,lane);
			DFE_WARN("HDMI[p%d] Lane%d Vth=0x%x\n", nport, lane, phy_st[nport].dfe_t[lane].vth);
			DFE_WARN("HDMI[p%d] Lane%d Tap0=0x%x, max=0x%02x, min=0x%02x\n", nport,
				lane, phy_st[nport].dfe_t[lane].tap0, phy_st[nport].dfe_t[lane].tap0max, phy_st[nport].dfe_t[lane].tap0min);

			DFE_WARN("HDMI[p%d] Lane%d Tap1=0x%02x, Tap2=%+d\n", nport,
				lane, phy_st[nport].dfe_t[lane].tap1, phy_st[nport].dfe_t[lane].tap2);

			DFE_WARN("HDMI[p%d] Lane%d LE=0x%02x, max=0x%02x, min=0x%02x\n", nport,
				lane, phy_st[nport].dfe_t[lane].le, phy_st[nport].dfe_t[lane].lemax, phy_st[nport].dfe_t[lane].lemin);
		}
	}

#else

	for (lane=0; lane<lane_mode; lane++) {
		DFE_WARN("HDMI[p%d] Bryan 0411********************Lane%d******************\n", nport, lane);
		DFE_WARN("HDMI[p%d] Lane%d Vth=0x%x\n", nport, lane, phy_st[nport].dfe_t[lane].vth);
		DFE_WARN("HDMI[p%d] Lane%d Tap0=0x%x, max=0x%02x, min=0x%02x\n", nport,
			lane, phy_st[nport].dfe_t[lane].tap0, phy_st[nport].dfe_t[lane].tap0max, phy_st[nport].dfe_t[lane].tap0min);

		DFE_WARN("HDMI[p%d] Lane%d Tap1=0x%02x, Tap2=%+d\n", nport,
			lane, phy_st[nport].dfe_t[lane].tap1, phy_st[nport].dfe_t[lane].tap2);

		DFE_WARN("HDMI[p%d] Lane%d LE=0x%02x, max=0x%02x, min=0x%02x\n", nport,
			lane, phy_st[nport].dfe_t[lane].le, phy_st[nport].dfe_t[lane].lemax, phy_st[nport].dfe_t[lane].lemin);
	}
#endif

}


#ifdef CONFIG_POWER_SAVING_MODE
void newbase_hdmi_phy_pw_saving(unsigned char port)
{
	lib_hdmi_cdr_rst(port, 1);
	lib_hdmi_demux_ck_vcopll_rst(port, 1);
	lib_hdmi_eq_pi_power_en(port,0);
//	lib_hdmi_wdg_rst(port, 1);
	lib_hdmi_cmu_pll_en(port, 0);
}

#endif

unsigned char lib_hdmi_clock40x_sync_status(unsigned char nport)
{
#if 1
        /* skip sync check. will run in interrupt context */
        DFE_WARN("HDMI[p%d] var_40x=%d toggle_count=%d\n", nport, newbase_hdmi2p0_get_clock40x_flag(nport) ,newbase_hdmi2p0_get_tmds_toggle_flag(nport));
        return 1;
#else
        unsigned char reg_40x = 0, var_40x = 0;

        reg_40x =  (lib_hdmi_scdc_get_tmds_config(nport) & _BIT1) >> 1;
	 var_40x =  newbase_hdmi2p0_get_clock40x_flag(nport);

	DFE_WARN("HDMI[p%d] reg_40x =%d  var_40x=%d toggle_count=%d\n", nport, reg_40x,var_40x,newbase_hdmi2p0_get_tmds_toggle_flag(nport));	
        if (reg_40x == var_40x)
                return 1;
        else {
                return 0;
        }
#endif
}

void lib_hdmi_trigger_measurement(unsigned char nport)
{
	if (nport == HDMI_PORT0) {
		//hdmi_mask(HDMIRX_2P0_PHY_REGD56_reg+((nport)<<4), ~HDMIRX_2P0_PHY_REGD56_p0_ck_md_rstb_mask, 0);
		//hdmi_mask(HDMIRX_2P0_PHY_REGD56_reg+((nport)<<4), ~HDMIRX_2P0_PHY_REGD56_p0_ck_md_rstb_mask, HDMIRX_2P0_PHY_REGD56_p0_ck_md_rstb_mask);
		//hdmi_out(HDMIRX_2P0_PHY_REGD57_reg+((nport)<<4), HDMIRX_2P0_PHY_REGD57_p0_ck_md_ok_mask);
		hdmi_mask(GDIRX_HDMI20_PHY_P0_MOD_REG0_reg, ~GDIRX_HDMI20_PHY_P0_MOD_REG0_p0_ck_md_rstb_mask, 0);
		hdmi_mask(GDIRX_HDMI20_PHY_P0_MOD_REG0_reg, ~GDIRX_HDMI20_PHY_P0_MOD_REG0_p0_ck_md_rstb_mask, GDIRX_HDMI20_PHY_P0_MOD_REG0_p0_ck_md_rstb_mask);
		hdmi_out(GDIRX_HDMI20_PHY_P0_MOD_REG1_reg, GDIRX_HDMI20_PHY_P0_MOD_REG1_p0_ck_md_ok_mask);		
	}
	else
	{
		hdmi_mask(HDMIRX_PHY_mod_regd00,~HDMIRX_PHY_HD21_P0_MOD_REGD00_reg_p0_ck_md_rstb_mask,0);
		hdmi_mask(HDMIRX_PHY_mod_regd00,~HDMIRX_PHY_HD21_P0_MOD_REGD00_reg_p0_ck_md_rstb_mask,HDMIRX_PHY_HD21_P0_MOD_REGD00_reg_p0_ck_md_rstb_mask);
		hdmi_out(HDMIRX_PHY_mod_regd01, HDMIRX_PHY_HD21_P0_MOD_REGD01_p0_ck_md_ok_ro_mask);
	}
}

void lib_hdmi_trigger_measure_start(unsigned char nport)
{
	if (nport == HDMI_PORT0) {
		//hdmi_mask(HDMIRX_2P0_PHY_REGD56_reg+((nport)<<4), ~HDMIRX_2P0_PHY_REGD56_p0_ck_md_rstb_mask, 0);
		hdmi_mask(GDIRX_HDMI20_PHY_P0_MOD_REG0_reg, ~GDIRX_HDMI20_PHY_P0_MOD_REG0_p0_ck_md_rstb_mask, 0);
	}
	else
	{
		hdmi_mask(HDMIRX_PHY_mod_regd00,~HDMIRX_PHY_HD21_P0_MOD_REGD00_reg_p0_ck_md_rstb_mask,0);
	
	}
}

void lib_hdmi_trigger_measure_stop(unsigned char nport)
{
	if (nport == HDMI_PORT0) {
	//hdmi_mask(HDMIRX_2P0_PHY_REGD56_reg+((nport)<<4),
	//		~HDMIRX_2P0_PHY_REGD56_p0_ck_md_rstb_mask,
	//		HDMIRX_2P0_PHY_REGD56_p0_ck_md_rstb_mask);

	hdmi_mask(GDIRX_HDMI20_PHY_P0_MOD_REG0_reg, ~GDIRX_HDMI20_PHY_P0_MOD_REG0_p0_ck_md_rstb_mask, GDIRX_HDMI20_PHY_P0_MOD_REG0_p0_ck_md_rstb_mask);
	}
	else
	{
		hdmi_mask(HDMIRX_PHY_mod_regd00,~HDMIRX_PHY_HD21_P0_MOD_REGD00_reg_p0_ck_md_rstb_mask,HDMIRX_PHY_HD21_P0_MOD_REGD00_reg_p0_ck_md_rstb_mask);
	
	}
}

unsigned char lib_hdmi_is_clk_ready(unsigned char nport)
{
	if (nport == HDMI_PORT0) {
		//hdmi_out(HDMIRX_2P0_PHY_REGD57_reg+((nport)<<4), HDMIRX_2P0_PHY_REGD57_p0_ck_md_ok_mask);
		hdmi_out(GDIRX_HDMI20_PHY_P0_MOD_REG1_reg, GDIRX_HDMI20_PHY_P0_MOD_REG1_p0_ck_md_ok_mask);		
		
		udelay(10);
		//if (HDMIRX_2P0_PHY_REGD57_get_p0_ck_md_ok(hdmi_in(HDMIRX_2P0_PHY_REGD57_reg+((nport)<<4)))) {
		if (GDIRX_HDMI20_PHY_P0_MOD_REG1_get_p0_ck_md_ok(hdmi_in(GDIRX_HDMI20_PHY_P0_MOD_REG1_reg))) {
			return TRUE;
		} else {
			return FALSE;
		}
	}
	else
	{
		hdmi_out(HDMIRX_PHY_mod_regd01, HDMIRX_PHY_HD21_P0_MOD_REGD01_p0_ck_md_ok_ro_mask);
		udelay(40);
		if (HDMIRX_PHY_HD21_P0_MOD_REGD01_get_p0_ck_md_ok_ro(hdmi_in(HDMIRX_PHY_mod_regd01))) {
			return TRUE;
		} else {
			return FALSE;
		}
	    return 0;
	}
}

unsigned int lib_hdmi_get_clock(unsigned char nport)
{   

	if (nport == HDMI_PORT0) {
		//return HDMIRX_2P0_PHY_REGD57_get_p0_ck_md_count(hdmi_in(HDMIRX_2P0_PHY_REGD57_reg+((nport)<<4)));
		return GDIRX_HDMI20_PHY_P0_MOD_REG1_get_p0_ck_md_count(hdmi_in(GDIRX_HDMI20_PHY_P0_MOD_REG1_reg));

	}
	else
	{
		return HDMIRX_PHY_HD21_P0_MOD_REGD01_get_p0_ck_md_count_ro(hdmi_in(HDMIRX_PHY_mod_regd01));
	}

}

void lib_hdmi_hysteresis_en(unsigned char en)
{

	unsigned char nport;
	if (en) 
	{
		for (nport=0; nport<HDMI_PORT_TOTAL_NUM; nport++)
		{
			if (newbase_hdmi_phy_port_status(nport) == HDMI_PHY_PORT_NOTUSED)
				continue;
			if(nport==0)
			{
				hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_CK2_reg,
				~(GDIRX_HDMI20_PHY_P0_P0_CK2_reg_p0_ck_sel_ckin_mask|  GDIRX_HDMI20_PHY_P0_P0_CK2_reg_p0_ck_en_non_hst_amp_mask| GDIRX_HDMI20_PHY_P0_P0_CK2_reg_p0_ck_en_enhance_bias_mask),
				(GDIRX_HDMI20_PHY_P0_P0_CK2_reg_p0_ck_sel_ckin_mask|GDIRX_HDMI20_PHY_P0_P0_CK2_reg_p0_ck_en_non_hst_amp_mask|GDIRX_HDMI20_PHY_P0_P0_CK2_reg_p0_ck_en_enhance_bias(3)));
			}
			else
			{
				hdmi_mask(HDMIRX_PHY_ck_cmu_1_reg,~(cmu_ck_1_HYS_AMP_EN|cmu_ck_1_MD_DET_SRC|cmu_ck_1_HYS_WIN_SEL|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN),
				(cmu_ck_1_HYS_AMP_EN|cmu_ck_1_NON_HYS_AMP_EN|cmu_ck_1_MD_DET_SRC|cmu_ck_1_ENHANCE_BIAS_10|cmu_ck_1_CKIN_SEL));
			}
		}
	}
	else 
	{
		for (nport=0; nport<HDMI_PORT_TOTAL_NUM; nport++)
		{
			if (newbase_hdmi_phy_port_status(nport) == HDMI_PHY_PORT_NOTUSED)
				continue;
			if(nport==0)
			{
				hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_CK2_reg,
				~(GDIRX_HDMI20_PHY_P0_P0_CK2_reg_p0_ck_sel_ckin_mask|  GDIRX_HDMI20_PHY_P0_P0_CK2_reg_p0_ck_en_non_hst_amp_mask| GDIRX_HDMI20_PHY_P0_P0_CK2_reg_p0_ck_en_enhance_bias_mask),
				(GDIRX_HDMI20_PHY_P0_P0_CK2_reg_p0_ck_en_enhance_bias(1)));
			
			}
			else
			{			
				hdmi_mask(HDMIRX_PHY_ck_cmu_1_reg,~(cmu_ck_1_HYS_AMP_EN|cmu_ck_1_MD_DET_SRC|cmu_ck_1_HYS_WIN_SEL|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN),
				(cmu_ck_1_ENHANCE_BIAS_01|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN));
			}
		}
	}




}

void lib_hdmi_phy_init(void)
{
	unsigned char nport;

	HDMI_PRINTF("%s\n", __func__);
	lib_hdmi_z0_calibration();
	
	for (nport=0; nport<HDMI_PORT_TOTAL_NUM; nport++)
	{
		if (newbase_hdmi_phy_port_status(nport) == HDMI_PHY_PORT_NOTUSED)
			continue;
		//newbase_hdmi_set_phy_script(nport, 0);
		if (nport == HDMI_PORT0) {
			DPHY_Fix_1_func(nport, TMDS_5p94G);
			DPHY_Param_1_func(nport, TMDS_5p94G);
			DPHY_Init_Flow_1_func(nport, TMDS_5p94G);
			APHY_Fix_1_func(nport, TMDS_5p94G);
			APHY_Param_1_func(nport, TMDS_5p94G);
			APHY_Init_Flow_1_func(nport, TMDS_5p94G);
		}
		else
		{
			DFE_ini_tmds_1_func(nport, TMDS_5p94G);
			DPHY_Fix_tmds_1_func(nport, TMDS_5p94G);
			DPHY_Para_tmds_1_func(nport, TMDS_5p94G);
			APHY_Fix_tmds_1_func(nport, TMDS_5p94G);
			APHY_Para_tmds_1_func(nport, TMDS_5p94G);
			DPHY_Init_Flow_tmds_1_func(nport, TMDS_5p94G);
			APHY_Init_Flow_tmds_1_func(nport, TMDS_5p94G);
		}
	}
	for (nport=HDMI_PORT0; nport<HDMI_PORT_TOTAL_NUM; nport++) {

		if(newbase_hdmi_phy_port_status(nport) == HDMI_PHY_PORT_NOTUSED)
			continue;

		lib_hdmi_z300_sel(nport, 0);
		
	}

}


void lib_hdmi_z0_set(unsigned char nport, unsigned char lane, unsigned char enable)
{

	HDMI_PRINTF("Port(%d)%s(%d)\n",nport, __func__,enable);

#if 1
	//unsigned int bmask = 0;
	if (nport == HDMI_PORT0) {
		
		if (enable == 0)
			hdmi_mask(GDIRX_HDMI20_PHY_P0_TOP_IN_0_reg, ~(REG_CK_LATCH), 0);  //latch bit clear
			
		hdmi_mask(GDIRX_HDMI20_PHY_P0_Z0POW_reg,~(GDIRX_HDMI20_PHY_P0_Z0POW_reg_z0_p_off_mask) ,GDIRX_HDMI20_PHY_P0_Z0POW_reg_z0_p_off(!enable));
		hdmi_mask(GDIRX_HDMI20_PHY_P0_Z0POW_reg,~(GDIRX_HDMI20_PHY_P0_Z0POW_reg_z0_n_off_mask) ,GDIRX_HDMI20_PHY_P0_Z0POW_reg_z0_n_off(!enable));
		hdmi_mask(GDIRX_HDMI20_PHY_P0_Z0POW_reg,~(GDIRX_HDMI20_PHY_P0_Z0POW_reg_p0_z0_z0pow_fix_mask) ,GDIRX_HDMI20_PHY_P0_Z0POW_reg_p0_z0_z0pow_fix(enable));
		
		if (enable == 0)
			hdmi_mask(GDIRX_HDMI20_PHY_P0_TOP_IN_0_reg, ~(REG_CK_LATCH), REG_CK_LATCH);  //latch bit set
	}
	else
	{


		if (enable == 0)
			hdmi_mask(HDMIRX_PHY_top_in_2_reg, ~(CK_LATCH), 0);  //latch bit clear
		hdmi_mask(HDMIRX_PHY_z0k_reg, ~(HDMIRX_PHY_Z0K_reg_z0_p_off_mask), HDMIRX_PHY_Z0K_reg_z0_p_off(!enable));
		hdmi_mask(HDMIRX_PHY_z0k_reg, ~(HDMIRX_PHY_Z0K_reg_z0_n_off_mask), HDMIRX_PHY_Z0K_reg_z0_n_off(!enable));
		hdmi_mask(HDMIRX_PHY_z0k_reg, ~(HDMIRX_PHY_Z0K_reg_p0_z0_z0pow_fix_mask), HDMIRX_PHY_Z0K_reg_p0_z0_z0pow_fix(enable));
		//hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~HD21_TOP_IN_Z0_FIX_SELECT_0, enable?(HD21_TOP_IN_Z0_FIX_SELECT_0):0 );//Remove from Mac8p

		if (enable == 0)
			hdmi_mask(HDMIRX_PHY_top_in_2_reg, ~(CK_LATCH), CK_LATCH);  //latch bit set
	
	}
#endif		
}


void lib_hdmi_z300_sel(unsigned char nport, unsigned char mode)  // 0: active mode 1:vcm mode
{
#if 1
	if (nport == HDMI_PORT0) {

	if (mode) {
		hdmi_mask(GDIRX_HDMI20_PHY_P0_Z0POW_reg, ~(GDIRX_HDMI20_PHY_P0_Z0POW_reg_p0_z0_z0pow_fix_mask),GDIRX_HDMI20_PHY_P0_Z0POW_reg_p0_z0_z0pow_fix(mode));
	} else {
		hdmi_mask(GDIRX_HDMI20_PHY_P0_Z0POW_reg, ~(GDIRX_HDMI20_PHY_P0_Z0POW_reg_p0_z0_z0pow_fix_mask), GDIRX_HDMI20_PHY_P0_Z0POW_reg_p0_z0_z0pow_fix(mode));
	}
	}
	else
	{
		if (mode) {
			hdmi_mask(HDMIRX_PHY_z0k_reg, ~(HDMIRX_PHY_Z0K_reg_p0_z0_z0pow_fix_mask), HDMIRX_PHY_Z0K_reg_p0_z0_z0pow_fix(mode));
		} else {
			hdmi_mask(HDMIRX_PHY_z0k_reg, ~(HDMIRX_PHY_Z0K_reg_p0_z0_z0pow_fix_mask), HDMIRX_PHY_Z0K_reg_p0_z0_z0pow_fix(mode));
		}


	}
#endif
}


void lib_hdmi_wdg_rst(unsigned char port, unsigned char rst)
{
 #if 0
	unsigned int reg_ck3;

	if (port == HDMI_PORT0) {
		reg_ck3 = HDMIRX_2P0_PHY_P0_CK3_reg;
	} else if (port == HDMI_PORT1) {
		reg_ck3 = HDMIRX_2P0_PHY_P1_CK3_reg;
	} else if (port == HDMI_PORT2) {
		reg_ck3 = HDMIRX_2P0_PHY_P2_CK3_reg;
	} else {
        	return;
        }

	if (rst) {
		hdmi_mask(reg_ck3, ~(P0_ck_9_CMU_PFD_RSTB|_BIT3|P0_ck_9_CMU_WDRST|_BIT1), (_BIT3|P0_ck_9_CMU_WDRST));	//PFD_RSTB  reset
	} else {
		hdmi_mask(reg_ck3, ~(P0_ck_9_CMU_PFD_RSTB|_BIT3|P0_ck_9_CMU_WDRST|_BIT1), (P0_ck_9_CMU_PFD_RSTB));  //release, normal
	}
#endif	
}


void lib_hdmi_dfe_power(unsigned char port, unsigned char en, unsigned char lane_mode)
{
#if 0
	unsigned int R3_addr,G3_addr,B3_addr;

	if (port == HDMI_PORT0) {
		R3_addr = HDMIRX_2P0_PHY_P0_R3_reg;
		G3_addr = HDMIRX_2P0_PHY_P0_G3_reg;
		B3_addr = HDMIRX_2P0_PHY_P0_B3_reg;
	} else if (port == HDMI_PORT1) {
		R3_addr = HDMIRX_2P0_PHY_P1_R3_reg;
		G3_addr = HDMIRX_2P0_PHY_P1_G3_reg;
		B3_addr = HDMIRX_2P0_PHY_P1_B3_reg;
	} else if (port == HDMI_PORT2) {
		R3_addr = HDMIRX_2P0_PHY_P2_R3_reg;
		G3_addr = HDMIRX_2P0_PHY_P2_G3_reg;
		B3_addr = HDMIRX_2P0_PHY_P2_B3_reg;
	} else {
		return;
	}

	hdmi_mask(R3_addr, ~(P0_b_9_DFE_ADAPTION_POW_EN|P0_b_9_DFE_TAP_EN), (DFE_HDMI_RX_PHY_P0_DFE_ADAPTION_EN((en!=0))|DFE_HDMI_RX_PHY_P0_b_tap_en(0x3)));
	hdmi_mask(G3_addr, ~(P0_b_9_DFE_ADAPTION_POW_EN|P0_b_9_DFE_TAP_EN), (DFE_HDMI_RX_PHY_P0_DFE_ADAPTION_EN((en!=0))|DFE_HDMI_RX_PHY_P0_b_tap_en(0x3)));
	hdmi_mask(B3_addr, ~(P0_b_9_DFE_ADAPTION_POW_EN|P0_b_9_DFE_TAP_EN), (DFE_HDMI_RX_PHY_P0_DFE_ADAPTION_EN((en!=0))|DFE_HDMI_RX_PHY_P0_b_tap_en(0x3)));
#endif
}

void lib_hdmi_eq_set_clock_boundary(unsigned char port)
{
	clock_bound_3g = TMDS_3G;
	clock_bound_1p5g = TMDS_1p5G;
	clock_bound_45m = 430;
	clock_bound_110m = 1042;
	//HDMI_PRINTF("clock_bound_3g=%d\n", clock_bound_3g);
}

#ifdef CONFIG_POWER_SAVING_MODE
void lib_hdmi_eq_pi_power_en(unsigned char nport,unsigned char en)
{

        if (nport == HDMI_PORT0) {
			
		if (en) {

			hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_B0_reg, ~(GDIRX_HDMI20_PHY_P0_P0_B0_reg_p0_b_pow_eq_dfe_mask),  GDIRX_HDMI20_PHY_P0_P0_B0_reg_p0_b_pow_eq_dfe_mask);
			hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_G0_reg, ~(GDIRX_HDMI20_PHY_P0_P0_G0_reg_p0_g_pow_eq_dfe_mask) , GDIRX_HDMI20_PHY_P0_P0_G0_reg_p0_g_pow_eq_dfe_mask);
			hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_R0_reg, ~(GDIRX_HDMI20_PHY_P0_P0_R0_reg_p0_r_pow_eq_dfe_mask), GDIRX_HDMI20_PHY_P0_P0_R0_reg_p0_r_pow_eq_dfe_mask);
			
		} 
		else 
		{
			hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_B0_reg, ~(GDIRX_HDMI20_PHY_P0_P0_B0_reg_p0_b_pow_eq_dfe_mask),  0);
			hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_G0_reg, ~(GDIRX_HDMI20_PHY_P0_P0_G0_reg_p0_g_pow_eq_dfe_mask) , 0);
			hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_R0_reg, ~(GDIRX_HDMI20_PHY_P0_P0_R0_reg_p0_r_pow_eq_dfe_mask), 0);

		}
			
        }
	else
	{
		if (en) {

			hdmi_mask(HDMIRX_PHY_b1_reg, ~(POW_LEQ|RS_CAL_EN),  POW_LEQ|RS_CAL_EN);
			hdmi_mask(HDMIRX_PHY_g1_reg, ~(POW_LEQ|RS_CAL_EN) , POW_LEQ|RS_CAL_EN);
			hdmi_mask(HDMIRX_PHY_r1_reg, ~(POW_LEQ|RS_CAL_EN), POW_LEQ|RS_CAL_EN);
			
		} 
		else 
		{
			hdmi_mask(HDMIRX_PHY_b1_reg, ~(POW_LEQ|RS_CAL_EN),  0x0);
			hdmi_mask(HDMIRX_PHY_g1_reg, ~(POW_LEQ|RS_CAL_EN) , 0x0);
			hdmi_mask(HDMIRX_PHY_r1_reg, ~(POW_LEQ|RS_CAL_EN),  0x0);
		}
	}
}	
#endif

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
unsigned char lib_hdmi_dfe_tap0_fix(unsigned char nport,unsigned char lane_mode)
{
#if 0
	unsigned char i, tap0max[4], tap0[4];
	unsigned char tap0_max_of_all, tap0_min_of_all;
	unsigned char bgrc;

	//read Tap0 max
	for (i=0; i<lane_mode; i++) {
		tap0max[i] = lib_hdmi_dfe_get_tap0max(nport, i);
		tap0[i] = lib_hdmi_dfe_get_tap0(nport, i);
	}


	if (lane_mode == HDMI_4LANE) {
		bgrc = LN_ALL;
		if ((tap0max[0] > 2) && (tap0max[1] > 2) && (tap0max[2] > 2) && (tap0max[3] > 2)) {
			return FALSE;
		}
		tap0_max_of_all = MAX(tap0[0],MAX(tap0[1], MAX(tap0[2], tap0[3])));
		tap0_min_of_all = MIN(tap0[0],MIN(tap0[1], MIN(tap0[2], tap0[3])));

		if ((tap0_max_of_all - tap0_min_of_all) < 8) {
			return FALSE;
		}
	} else {
		bgrc = (LN_R|LN_G|LN_B);
		if ((tap0max[0] > 2) && (tap0max[1] > 2) && (tap0max[2] > 2)) {
			return FALSE;
		}
		tap0_max_of_all = MAX(tap0[0], MAX(tap0[1], tap0[2]));
		tap0_min_of_all = MIN(tap0[0], MIN(tap0[1], tap0[2]));

		if ((tap0_max_of_all - tap0_min_of_all) < 8) {
			return FALSE;
		}
	}

	HDMI_EMG("%s\n", __func__);
	
	lib_hdmi_dfe_init_vth(nport, bgrc, 0x05);
	lib_hdmi_dfe_init_tap0(nport, bgrc, 0x14);

#if 1
//#ifdef PLAN_B
	udelay(100);
	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth), (REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth));
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth), (REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth));
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth), (REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth));

	udelay(500);
	hdmi_mask(DFE_REG_DFE_READBACK_reg,~DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0_mask,DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0(1));
	udelay(500);
	hdmi_mask(DFE_REG_DFE_READBACK_reg,~DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0_mask,DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0(0));
	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth),0);
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth),0);
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth),0);


#endif

#endif
	return TRUE;
}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


void lib_hdmi_tap0_fix_small_swing(unsigned char nport, unsigned char lane_mode)
{
#if 0
	unsigned char bgrc;
	HDMI_EMG("%s\n", __func__);
	if (lane_mode == HDMI_4LANE)
		bgrc = LN_ALL;
	else
		bgrc = (LN_R|LN_G|LN_B);
	
	lib_hdmi_dfe_init_vth(nport, bgrc, 0x05);
	lib_hdmi_dfe_init_tap0(nport, bgrc, 0x14);
	udelay(100);
	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth), (REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth));
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth), (REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth));
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth), (REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth));
	udelay(500);
	hdmi_mask(DFE_REG_DFE_READBACK_reg,~DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0_mask,DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0(1));
	udelay(500);
	hdmi_mask(DFE_REG_DFE_READBACK_reg,~DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0_mask,DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0(0));
	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth),0);
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth),0);
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth),0);
#endif	
}


void lib_hdmi_dfe_second_flow(unsigned char nport, unsigned int b_clk, unsigned char lane_mode)
{
#if 0
	unsigned char i, tap0max[4];

	udelay(100);
	hdmi_mask(DFE_REG_DFE_READBACK_reg,~DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0_mask,DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0(1));
	udelay(500);
	hdmi_mask(DFE_REG_DFE_READBACK_reg,~DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0_mask,DFE_P0_REG_DFE_READBACK_P0_record_limit_en_p0(0));
	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth),0);
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth),0);
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth),0);


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
	if (b_clk > 5000) {
		lib_hdmi_dfe_tap0_fix(nport,lane_mode);
	}
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

	//read Tap0 max
	for (i=0; i<lane_mode; i++) {
		tap0max[i] = lib_hdmi_dfe_get_tap0max(nport, i);
		tap0max[i] = (tap0max[i]>2)?(tap0max[i]-2):0;
	}

	// LOAD TAP0 max -2  & Vth 5
	hdmi_mask(DFE_REG_DFE_INIT0_L0_reg,~(DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0_mask),(DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(tap0max[0]))|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0(0x5));
	hdmi_mask(DFE_REG_DFE_INIT0_L1_reg,~(DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0_mask),(DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(tap0max[1]))|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0(0x5));
	hdmi_mask(DFE_REG_DFE_INIT0_L2_reg,~(DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0_mask),(DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(tap0max[2]))|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0(0x5));


	//load initial data
	hdmi_mask(DFE_REG_DFE_INIT1_L0_reg,~(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH),(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH));
	hdmi_mask(DFE_REG_DFE_INIT1_L1_reg,~(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH),(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH));
	hdmi_mask(DFE_REG_DFE_INIT1_L2_reg,~(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH),(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH));
	udelay(1);

	hdmi_mask(DFE_REG_DFE_INIT1_L0_reg,~(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH),0);
	hdmi_mask(DFE_REG_DFE_INIT1_L1_reg,~(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH),0);
	hdmi_mask(DFE_REG_DFE_INIT1_L2_reg,~(LOAD_IN_INIT_TAP0|LOAD_IN_INIT_VTH),0);
#endif
}


void lib_hdmi_cmu_pll_en(unsigned char nport, unsigned char enable)
{  
#if 1
	if (enable)
	{	
		if (nport == HDMI_PORT0)
		{
				hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_CK2_reg,
				~(GDIRX_HDMI20_PHY_P0_P0_CK2_reg_p0_ck_sel_ckin_mask),
				GDIRX_HDMI20_PHY_P0_P0_CK2_reg_p0_ck_sel_ckin_mask);
				hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_CK0_reg,
				~(GDIRX_HDMI20_PHY_P0_P0_CK0_reg_p0_ck_pow_vco_mask|GDIRX_HDMI20_PHY_P0_P0_CK0_reg_p0_ck_pow_vco_vdac_mask|GDIRX_HDMI20_PHY_P0_P0_CK0_reg_p0_ck_pow_cp_mask|GDIRX_HDMI20_PHY_P0_P0_CK0_reg_p0_ck_pow_lpf_idac_mask),
				(GDIRX_HDMI20_PHY_P0_P0_CK2_reg_p0_ck_sel_ckin_mask|GDIRX_HDMI20_PHY_P0_P0_CK0_reg_p0_ck_pow_vco_vdac_mask|GDIRX_HDMI20_PHY_P0_P0_CK0_reg_p0_ck_pow_cp_mask|GDIRX_HDMI20_PHY_P0_P0_CK0_reg_p0_ck_pow_lpf_idac_mask));

				hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_CK1_reg,
				~(GDIRX_HDMI20_PHY_P0_P0_CK1_reg_p0_ck_rstb_clk_fld_mask|GDIRX_HDMI20_PHY_P0_P0_CK1_reg_p0_ck_rstb_ref_fb_div_mask|GDIRX_HDMI20_PHY_P0_P0_CK1_reg_p0_ck_rstb_div_band_2or4_mask|GDIRX_HDMI20_PHY_P0_P0_CK1_reg_p0_ck_rstb_div_iq_mask),
				(GDIRX_HDMI20_PHY_P0_P0_CK1_reg_p0_ck_rstb_clk_fld_mask|GDIRX_HDMI20_PHY_P0_P0_CK1_reg_p0_ck_rstb_ref_fb_div_mask|GDIRX_HDMI20_PHY_P0_P0_CK1_reg_p0_ck_rstb_div_band_2or4_mask|GDIRX_HDMI20_PHY_P0_P0_CK1_reg_p0_ck_rstb_div_iq_mask));

		}
		else
		{
			hdmi_mask(HDMIRX_PHY_ck_cmu_1_reg,~(cmu_ck_1_CKIN_SEL),cmu_ck_1_CKIN_SEL);

			hdmi_mask(HDMIRX_PHY_fld_regd18,~(acdr_manual_RDY|acdr_manual_CMU_SEL),0x0);//R
			hdmi_mask(HDMIRX_PHY_fld_regd11,~(acdr_manual_RDY|acdr_manual_CMU_SEL),0x0);//G
			hdmi_mask(HDMIRX_PHY_fld_regd04,~(acdr_manual_RDY|acdr_manual_CMU_SEL), 0x0);//B
			hdmi_mask(HDMIRX_PHY_fld_regd25,~(acdr_manual_RDY|acdr_manual_CMU_SEL), 0x0);//CK
			//r lane on
			hdmi_mask(HDMIRX_PHY_acdr_r0_reg,~(acdr_PFD_RSTB|acdr_DIV_IQ_RSTB), acdr_PFD_RSTB|acdr_DIV_IQ_RSTB);
			hdmi_mask(HDMIRX_PHY_acdr_r1_reg,~(acdr_VCO_EN), acdr_VCO_EN);//1: on
			//g lane on
			hdmi_mask(HDMIRX_PHY_acdr_g0_reg,~(acdr_PFD_RSTB|acdr_DIV_IQ_RSTB), acdr_PFD_RSTB|acdr_DIV_IQ_RSTB);
			hdmi_mask(HDMIRX_PHY_acdr_g1_reg,~(acdr_VCO_EN), acdr_VCO_EN);//1: on
			//b lane on
			hdmi_mask(HDMIRX_PHY_acdr_b0_reg,~(acdr_PFD_RSTB|acdr_DIV_IQ_RSTB), acdr_PFD_RSTB|acdr_DIV_IQ_RSTB);
			hdmi_mask(HDMIRX_PHY_acdr_b1_reg,~(acdr_VCO_EN), acdr_VCO_EN);//1: on
			//ck lane on
			hdmi_mask(HDMIRX_PHY_acdr_ck0_reg,~(acdr_PFD_RSTB|acdr_DIV_IQ_RSTB), acdr_PFD_RSTB|acdr_DIV_IQ_RSTB);
			hdmi_mask(HDMIRX_PHY_acdr_ck1_reg,~(acdr_VCO_EN), acdr_VCO_EN);//1: on			
		}

	} 
	else 
	{ //disable default tmds clock
		if (nport == HDMI_PORT0)
		{
				hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_CK2_reg,
				~(GDIRX_HDMI20_PHY_P0_P0_CK2_reg_p0_ck_sel_ckin_mask),
				GDIRX_HDMI20_PHY_P0_P0_CK2_reg_p0_ck_sel_ckin_mask);
				hdmi_mask(GDIRX_HDMI20_PHY_P0_P0_CK0_reg,
				~(GDIRX_HDMI20_PHY_P0_P0_CK0_reg_p0_ck_pow_vco_mask|GDIRX_HDMI20_PHY_P0_P0_CK0_reg_p0_ck_pow_vco_vdac_mask|GDIRX_HDMI20_PHY_P0_P0_CK0_reg_p0_ck_pow_cp_mask|GDIRX_HDMI20_PHY_P0_P0_CK0_reg_p0_ck_pow_lpf_idac_mask),
				0x0);
		}
		else
		{
			hdmi_mask(HDMIRX_PHY_ck_cmu_1_reg,~(cmu_ck_1_CKIN_SEL),cmu_ck_1_CKIN_SEL);
			//default from register
			hdmi_mask(HDMIRX_PHY_fld_regd18,~(acdr_manual_RDY|acdr_manual_CMU_SEL),(acdr_manual_RDY_FROM_REG));//R
			hdmi_mask(HDMIRX_PHY_fld_regd11,~(acdr_manual_RDY|acdr_manual_CMU_SEL),(acdr_manual_RDY_FROM_REG));//G
			hdmi_mask(HDMIRX_PHY_fld_regd04,~(acdr_manual_RDY|acdr_manual_CMU_SEL), (acdr_manual_RDY_FROM_REG));//B
			hdmi_mask(HDMIRX_PHY_fld_regd25,~(acdr_manual_RDY|acdr_manual_CMU_SEL), (acdr_manual_RDY_FROM_REG));//CK

			//r lane off
			hdmi_mask(HDMIRX_PHY_acdr_r0_reg,~(acdr_PFD_RSTB|acdr_DIV_IQ_RSTB), 0x0);
			hdmi_mask(HDMIRX_PHY_acdr_r1_reg,~(acdr_VCO_EN), 0x0);//0: 0ff
			//g lane off
			hdmi_mask(HDMIRX_PHY_acdr_g0_reg,~(acdr_PFD_RSTB|acdr_DIV_IQ_RSTB), 0x0);
			hdmi_mask(HDMIRX_PHY_acdr_g1_reg,~(acdr_VCO_EN), 0x0);//0: 0ff
			//b lane off
			hdmi_mask(HDMIRX_PHY_acdr_b0_reg,~(acdr_PFD_RSTB|acdr_DIV_IQ_RSTB), 0x0);
			hdmi_mask(HDMIRX_PHY_acdr_b1_reg,~(acdr_VCO_EN), 0x0);//0: 0ff
			//ck lane off
			hdmi_mask(HDMIRX_PHY_acdr_ck0_reg,~(acdr_PFD_RSTB|acdr_DIV_IQ_RSTB), 0x0);
			hdmi_mask(HDMIRX_PHY_acdr_ck1_reg,~(acdr_VCO_EN), 0x0);//0: 0ff		}
		}

	}
#endif	
	
}

void lib_hdmi_demux_ck_vcopll_rst(unsigned char nport, unsigned char rst)
{

// rst = 1: reset
// rst = 0: normal working operation
	unsigned int rgb_demux;
	if (nport == HDMI_PORT0)
	{

		rgb_demux = GDIRX_HDMI20_PHY_P0_APHY_REG0_p0_rstb_demux_r_mask|GDIRX_HDMI20_PHY_P0_APHY_REG0_p0_rstb_demux_g_mask|GDIRX_HDMI20_PHY_P0_APHY_REG0_p0_rstb_demux_b_mask;

		if (rst) {		
			hdmi_mask(GDIRX_HDMI20_PHY_P0_APHY_REG0_reg, ~rgb_demux, 0);
		} else {
			hdmi_mask(GDIRX_HDMI20_PHY_P0_APHY_REG0_reg, ~rgb_demux, rgb_demux);
		}
		
		}
	else
	{
		rgb_demux=HDMIRX_PHY_HD21_REGA00_p0_r_demux_rstb_mask|HDMIRX_PHY_HD21_REGA00_p0_g_demux_rstb_mask|HDMIRX_PHY_HD21_REGA00_p0_b_demux_rstb_mask;

		if (rst) {
			hdmi_mask(HDMIRX_PHY_rega00, ~rgb_demux, 0);
			
		} else {

			hdmi_mask(HDMIRX_PHY_rega00, ~rgb_demux,rgb_demux );
	
		}

	}
}



void lib_hdmi_cdr_rst(unsigned char nport, unsigned char rst)
{
#if 1
// rst = 1: reset
// rst = 0: normal working operation
	unsigned int rst_reg,dig_bit,cdr_bit;
        // release reset

	if (nport == HDMI_PORT0) {
		 rst_reg = GDIRX_HDMI20_PHY_P0_DCDR_REG0_reg;
		 dig_bit = GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_r_dig_rst_n_mask|GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_g_dig_rst_n_mask|GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_b_dig_rst_n_mask;
		 cdr_bit = GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_r_cdr_rst_n_mask|GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_g_cdr_rst_n_mask|GDIRX_HDMI20_PHY_P0_DCDR_REG0_p0_b_cdr_rst_n_mask;
	} else if (nport == HDMI_PORT1) {
		 rst_reg = HDMIRX_PHY_cdr_regd00;
		 dig_bit = HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_r_dig_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_g_dig_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_b_dig_rst_n_mask;
		 cdr_bit =HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_r_cdr_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_g_cdr_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_b_cdr_rst_n_mask;
	} else if (nport == HDMI_PORT2) {
		 rst_reg = HDMIRX_PHY_cdr_regd00;
		 dig_bit = HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_r_dig_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_g_dig_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_b_dig_rst_n_mask;
		 cdr_bit =HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_r_cdr_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_g_cdr_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_b_cdr_rst_n_mask;

	} else {
		return;
	}
	

	if (rst) {
		hdmi_mask(rst_reg, ~dig_bit, 0);
		hdmi_mask(rst_reg, ~cdr_bit, 0);

	} else {
		hdmi_mask(rst_reg, ~(dig_bit|cdr_bit), (dig_bit|cdr_bit));
	}

#endif
}

void lib_hdmi_wdog(unsigned char nport)
{
#if 0
	unsigned int reg_cmu0;
	unsigned int reg_cmu1;
	
	if (nport == HDMI_PORT0) {
		 reg_cmu0 = HDMIRX_2P0_PHY_REGD20_reg;
		 reg_cmu1 = HDMIRX_2P0_PHY_REGD21_reg;
	} else if (nport == HDMI_PORT1) {
		reg_cmu0 = HDMIRX_2P0_PHY_REGD22_reg;
		 reg_cmu1 = HDMIRX_2P0_PHY_REGD23_reg;
	} else if (nport == HDMI_PORT2) {
		reg_cmu0 = HDMIRX_2P0_PHY_REGD24_reg;
		reg_cmu1 = HDMIRX_2P0_PHY_REGD25_reg;
	} else {
		return;
	}

	hdmi_mask(reg_cmu0, ~(HDMIRX_2P0_PHY_REGD20_reg_p0_bypass_eqen_rdy_mask|HDMIRX_2P0_PHY_REGD20_reg_p0_bypass_data_rdy_mask|HDMIRX_2P0_PHY_REGD20_reg_p0_bypass_pi_mask|
		                                               HDMIRX_2P0_PHY_REGD20_reg_p0_data_rdy_time_mask|HDMIRX_2P0_PHY_REGD20_reg_p0_en_clkout_manual_mask|HDMIRX_2P0_PHY_REGD20_reg_p0_en_eqen_manual_mask|
		                                                HDMIRX_2P0_PHY_REGD20_reg_p0_en_data_manual_mask|HDMIRX_2P0_PHY_REGD20_reg_p0_eqen_rdy_time_mask|HDMIRX_2P0_PHY_REGD20_reg_p0_pll_wd_base_time_mask|
		                                                HDMIRX_2P0_PHY_REGD20_reg_p0_pll_wd_rst_wid_mask|HDMIRX_2P0_PHY_REGD20_reg_p0_pll_wd_time_rdy_mask|HDMIRX_2P0_PHY_REGD20_p0_pll_wd_ckrdy_ro_mask|
		                                                HDMIRX_2P0_PHY_REGD20_p0_wdog_rst_n_mask),
		                                                	HDMIRX_2P0_PHY_REGD20_reg_p0_bypass_clk_rdy(1)| HDMIRX_2P0_PHY_REGD20_reg_p0_bypass_eqen_rdy(1)|HDMIRX_2P0_PHY_REGD20_reg_p0_bypass_data_rdy(1)|
		                                                 HDMIRX_2P0_PHY_REGD20_reg_p0_data_rdy_time(4)| HDMIRX_2P0_PHY_REGD20_reg_p0_en_clkout_manual(1)|HDMIRX_2P0_PHY_REGD20_reg_p0_en_eqen_manual(1)|
									HDMIRX_2P0_PHY_REGD20_reg_p0_en_data_manual(1)|HDMIRX_2P0_PHY_REGD20_reg_p0_eqen_rdy_time(4)|HDMIRX_2P0_PHY_REGD20_reg_p0_pll_wd_base_time(2)|
									HDMIRX_2P0_PHY_REGD20_reg_p0_pll_wd_rst_wid(1)|HDMIRX_2P0_PHY_REGD20_reg_p0_pll_wd_time_rdy(1) |HDMIRX_2P0_PHY_REGD20_p0_pll_wd_ckrdy_ro(1)|
									HDMIRX_2P0_PHY_REGD20_p0_wdog_rst_n(1));


	hdmi_mask(reg_cmu1, ~(HDMIRX_2P0_PHY_REGD21_reg_p0_wd_sdm_en_mask),0);
	udelay(5);
	hdmi_mask(reg_cmu1, ~(HDMIRX_2P0_PHY_REGD21_reg_p0_wd_sdm_en_mask), HDMIRX_2P0_PHY_REGD21_reg_p0_wd_sdm_en_mask);	//adams modify, 20160130	//Load CMU M code

#endif
}

void lib_hdmi_dfe_config_setting(unsigned char nport,unsigned char lane_mode)
{
        lib_hdmi_mac_afifo_enable(nport,lane_mode);
}



//------------------------------------------------------------------------
// Tool: OTP_Parser
// Date: 22/07/15 11:22:20

//------------------------------------------------------------------------
// [Field 0]
// OTP field name : HDMI Z0
// OTP bit : [1600:1617]: HDMI Z0 18 bits
// Rbus addr : 0x180380c8[0] ~ 0x180380c8[17]
//B80380C8[17:0]: z0 value 3 port
//port0_z0=B80380C8[4:0] port0_done=B80380C8[5]
//port1_z0=B80380C8[10:6] port1_done=B80380C8[11]
//port2_z0=B80380C8[16:12] port2_done=B80380C8[17]

void lib_hdmi_z0_calibration(void)
{
	 #define Z0_DIFF_ERR		0x0

	//------------------------------------------------------------------------
	// Tool: OTP_Parser
	// Date: 22/07/15 11:22:20

	//------------------------------------------------------------------------
	// [Field 0]
	// OTP field name : HDMI Z0
	// OTP bit : [1600:1617]: HDMI Z0 18 bits
	// Rbus addr : 0x180380c8[0] ~ 0x180380c8[17]
	unsigned int HDMI_Z0_Para1 = rtd_part_inl(0xb80380c8,17,0);
	 
	 unsigned char nport;
	 unsigned char z0_otp_done;
        unsigned char z0_0, z0_1,z0_2;    //z0 for 4 port
	 unsigned char ext_trace_z0_0,ext_trace_z0_1,ext_trace_z0_2; //external z0 for 3 port
	 unsigned long long val = 0;  
	//------------------------------------------------------------------------
	// [Field 0]
	// OTP field name : HDMI Z0
	// OTP bit : [1600:1617]: HDMI Z0 18 bits
	// Rbus addr : 0x180380c8[0] ~ 0x180380c8[17]
	 
	 ext_trace_z0_0 = ext_trace_z0_1 = ext_trace_z0_2 = (0x5 - Z0_DIFF_ERR);  
	 z0_otp_done = ((HDMI_Z0_Para1 & 0x00000020) >> 5) | ((HDMI_Z0_Para1 & 0x00000800) >> 10) | ((HDMI_Z0_Para1 & 0x00020000) >> 15);
	 
	  if (pcb_mgr_get_enum_info_byname("HDMI_EXT_Z0_CFG", &val)==0) {
		ext_trace_z0_0 = (val & 0x000000FF)- Z0_DIFF_ERR;
		ext_trace_z0_1 = ((val & 0x0000FF00) >> 8) - Z0_DIFF_ERR;
		ext_trace_z0_2 = ((val & 0x00FF0000) >> 16) - Z0_DIFF_ERR;
		
	  } else {
			HDMI_PRINTF("NOT GET  EXTERNAL PCB TRACE Z0\n");
	  }

	  
	HDMI_PRINTF("p0_ext_trace_z0=%x  p1_ext_trace_z0=%x p2_ext_trace_z0=%x\n",ext_trace_z0_0,ext_trace_z0_1,ext_trace_z0_2);


	HDMI_PRINTF("Z0 OTP done_bit=%x value=%x\n",z0_otp_done,HDMI_Z0_Para1);


        if (z0_otp_done & 0x1) {
                z0_0 = HDMI_Z0_Para1 & 0x0000001F;
        }
	else
		z0_0=PHY_P0_Z0;

	if(ext_trace_z0_0==0)
		z0_0 = z0_0 - 3;// For pure port Target 100 ohm diff to Target 90 ohm (Need -3)


        if (z0_otp_done & 0x2) {
                z0_1 = (HDMI_Z0_Para1 & 0x000007C0) >> 6;
        }
	else
		z0_1=PHY_P1_Z0;
		

	if(ext_trace_z0_1==0)
		z0_1 = z0_1 - 2;// For combo port Target 100 ohm diff to Target 90 ohm (Need -2)

	
        if (z0_otp_done & 0x4) {
                z0_2 = (HDMI_Z0_Para1 & 0x0001F000) >> 12;
        }
	else
		z0_2=PHY_P2_Z0;

	if(ext_trace_z0_2==0)
		z0_2 = z0_2 - 2;// For combo port Target 100 ohm diff to Target 90 ohm (Need -2)


	HDMI_PRINTF("z0_0=%x z0_1=%x z0_2=%x\n",z0_0,z0_1,z0_2);

///
	for (nport=0; nport<HDMI_PORT_TOTAL_NUM; nport++)
	{
		if (newbase_hdmi_phy_port_status(nport) == HDMI_PHY_PORT_NOTUSED)
		{
			HDMI_PRINTF("lib_hdmi_z0_calibration, skip noot used port %d\n", nport );
			continue;
		}

		if (HDMI_PORT0 == nport)
		{
			hdmi_mask(GDIRX_HDMI20_PHY_P0_Z0K_reg, ~(GDIRX_HDMI20_PHY_P0_Z0K_reg_p0_z0_adjr_mask),
				(GDIRX_HDMI20_PHY_P0_Z0K_reg_p0_z0_adjr(z0_0)));
		}
		else if (HDMI_PORT1 == nport)
		{
			hdmi_mask(HDMIRX_PHY_z0k_reg, ~(HDMIRX_PHY_Z0K_reg_z0_adjr_0_mask),
				(HDMIRX_PHY_Z0K_reg_z0_adjr_0(z0_1)));
		}
		else if (HDMI_PORT2 == nport)
		{
			hdmi_mask(HDMIRX_PHY_z0k_reg, ~(HDMIRX_PHY_Z0K_reg_z0_adjr_0_mask),
				(HDMIRX_PHY_Z0K_reg_z0_adjr_0(z0_2)));
		}
	}
}

#if 0
void lib_hdmi_single_end(unsigned char nport, unsigned char channel, unsigned char p_off, unsigned char n_off)
{
// channel = 0(R), 1(G), 2(B) 3(CK)
	hdmi_mask(reg_rgb1[nport][channel], ~(P0_b_1_inputoff_p), p_off<<2);
	hdmi_mask(reg_rgb1[nport][channel], ~(P0_b_1_inputoff_n), n_off<<2);

	if (nport == HDMI_PORT3) {
		if (channel == 3) {
			hdmi_mask(DFE_HDMI_RX_PHY_P0_HD21_P0_CK1_reg, ~(P0_b_1_inputoff_p), p_off<<2);
			hdmi_mask(DFE_HDMI_RX_PHY_P0_HD21_P0_CK1_reg, ~(P0_b_1_inputoff_n), n_off<<2);
		}
	}
}
#endif



#if BIST_DFE_SCAN
//--------------------------------------------------------------------------
//
//	Debug Functions, not release.
//
//--------------------------------------------------------------------------
void debug_hdmi_dfe_scan(unsigned char nport,unsigned char lane_mode)
{
   
	#define ERR_DET_ONETIME_MODE 0
	signed char dfe_arg;
	unsigned int bit_r_err, bit_g_err, bit_b_err, ch_r_err, ch_g_err, ch_b_err;
	unsigned char mode;
	unsigned int crc = 0;
	unsigned char bgrc;
	unsigned char tmds_ch;

	lib_hdmi_set_fw_debug_bit(DEBUG_27_BYPASS_PHY_ISR, 1);
	lib_hdmi_set_fw_debug_bit(DEBUG_26_BYPASS_DETECT, 1);
	lib_hdmi_set_fw_debug_bit(DEBUG_25_BYPASS_CHECK, 1);
	HDMI_EMG("================SCAN START===================\n");

	if (lane_mode == HDMI_4LANE)
		bgrc = LN_ALL;
	else
		bgrc = (LN_R|LN_G|LN_B);

	//tap0 0~36
	for (dfe_arg=0; dfe_arg<0x25; dfe_arg++) {
		lib_hdmi_dfe_init_tap0(nport, bgrc, dfe_arg);
		//lib_hdmi_dfe_init_tap1(nport, (LN_R|LN_G|LN_B), dfe_arg);
		//lib_hdmi_dfe_init_tap2(nport, (LN_R|LN_G|LN_B), dfe_arg);
		udelay(100);

#if ERR_DET_ONETIME_MODE
		lib_hdmi_bit_err_start(nport, 0, 63);
		lib_hdmi_char_err_start(nport, 0, 63);
		lib_hdmi_clear_rgb_hv_status(nport);

		while (!lib_hdmi_bit_err_get_error(nport, &bit_r_err, &bit_g_err, &bit_b_err)) {
			msleep(20);
		}
		while (!lib_hdmi_char_err_get_error(nport, &ch_r_err, &ch_g_err, &ch_b_err)) {
			msleep(20);
		}
#else
		lib_hdmi_bit_err_start(nport, 1, 10);
		lib_hdmi_char_err_start(nport, 1, 10);
		lib_hdmi_clear_rgb_hv_status(nport);
		msleep(1500);
		lib_hdmi_bit_err_get_error(nport, &bit_r_err, &bit_g_err, &bit_b_err);
		lib_hdmi_char_err_get_error(nport, &ch_r_err, &ch_g_err, &ch_b_err);
#endif
		crc = lib_hdmi_read_crc(nport);
		tmds_ch = (lib_hdmi_read_rgb_hv_status(nport) & 0x000000F8) >> 3;

		HDMI_EMG("tap0=%02d, [CH](%05d, %05d, %05d); [BIT](%05d, %05d, %05d) PP_CRC=%x tmds_ch=%x\n",
			dfe_arg, ch_r_err, ch_g_err, ch_b_err, bit_r_err, bit_g_err, bit_b_err,crc,tmds_ch);

	}

	lib_hdmi_dfe_init_tap0(nport, (LN_B), phy_st[nport].dfe_t[0].tap0);
	lib_hdmi_dfe_init_tap0(nport, (LN_G), phy_st[nport].dfe_t[1].tap0);
	lib_hdmi_dfe_init_tap0(nport, (LN_R), phy_st[nport].dfe_t[2].tap0);

	if (lane_mode == HDMI_4LANE)
		lib_hdmi_dfe_init_tap0(nport, (LN_CK), phy_st[nport].dfe_t[3].tap0);


	mode = DFE_P0_REG_DFE_MODE_P0_get_adapt_mode_p0(hdmi_in(DFE_REG_DFE_MODE_reg));


	if (mode == 2) {
			//le 0~24
			for (dfe_arg=0; dfe_arg<25; dfe_arg++) {
			//lib_hdmi_dfe_init_tap0(nport, (LN_R|LN_G|LN_B), dfe_arg);
			//lib_hdmi_dfe_init_tap1(nport, (LN_R|LN_G|LN_B), dfe_arg);
			lib_hdmi_dfe_init_le(nport, (LN_R|LN_G|LN_B), dfe_arg);
			//lib_hdmi_dfe_init_tap2(nport, (LN_R|LN_G|LN_B), dfe_arg);
			udelay(100);

#if ERR_DET_ONETIME_MODE
			lib_hdmi_bit_err_start(nport, 0, 63);
			lib_hdmi_char_err_start(nport, 0, 63);
			lib_hdmi_clear_rgb_hv_status(nport);

			while (!lib_hdmi_bit_err_get_error(nport, &bit_r_err, &bit_g_err, &bit_b_err)) {
				msleep(20);
			}
			
			while (!lib_hdmi_char_err_get_error(nport, &ch_r_err, &ch_g_err, &ch_b_err)) {
				msleep(20);
			}
#else
			lib_hdmi_bit_err_start(nport, 1, 10);
			lib_hdmi_char_err_start(nport, 1, 10);
			lib_hdmi_clear_rgb_hv_status(nport);
			msleep(1500);
			lib_hdmi_bit_err_get_error(nport, &bit_r_err, &bit_g_err, &bit_b_err);
			lib_hdmi_char_err_get_error(nport, &ch_r_err, &ch_g_err, &ch_b_err);
#endif
			crc = lib_hdmi_read_crc(nport);
			tmds_ch = (lib_hdmi_read_rgb_hv_status(nport) & 0x000000F8) >> 3;
			
			HDMI_EMG("le=%02d, [CH](%05d, %05d, %05d); [BIT](%05d, %05d, %05d) PP_CRC=%x tmds_ch=%x\n",
				dfe_arg, ch_r_err, ch_g_err, ch_b_err, bit_r_err, bit_g_err, bit_b_err,crc,tmds_ch);

		}

		lib_hdmi_dfe_init_le(nport, (LN_B), phy_st[nport].dfe_t[0].le);
		lib_hdmi_dfe_init_le(nport, (LN_G), phy_st[nport].dfe_t[1].le);
		lib_hdmi_dfe_init_le(nport, (LN_R), phy_st[nport].dfe_t[2].le);
	} else {
		
		//mode 3
			for (dfe_arg=0; dfe_arg<25; dfe_arg++) {
				//lib_hdmi_dfe_init_tap0(nport, (LN_R|LN_G|LN_B), dfe_arg);
				lib_hdmi_dfe_init_tap1(nport, bgrc, dfe_arg);
				//lib_hdmi_dfe_init_tap2(nport, (LN_R|LN_G|LN_B), dfe_arg);
				udelay(100);

#if ERR_DET_ONETIME_MODE
				lib_hdmi_bit_err_start(nport, 0, 63);
				lib_hdmi_char_err_start(nport, 0, 63);
				lib_hdmi_clear_rgb_hv_status(nport);

				while (!lib_hdmi_bit_err_get_error(nport, &bit_r_err, &bit_g_err, &bit_b_err)) {
					msleep(20);
				}
				while (!lib_hdmi_char_err_get_error(nport, &ch_r_err, &ch_g_err, &ch_b_err)) {
					msleep(20);
				}
#else
				lib_hdmi_bit_err_start(nport, 1, 10);
				lib_hdmi_char_err_start(nport, 1, 10);
				lib_hdmi_clear_rgb_hv_status(nport);
				msleep(1500);
				lib_hdmi_bit_err_get_error(nport, &bit_r_err, &bit_g_err, &bit_b_err);
				lib_hdmi_char_err_get_error(nport, &ch_r_err, &ch_g_err, &ch_b_err);
#endif
				crc = lib_hdmi_read_crc(nport);
				tmds_ch = (lib_hdmi_read_rgb_hv_status(nport) & 0x000000F8) >> 3;
				HDMI_EMG("le=%02d, [CH](%05d, %05d, %05d); [BIT](%05d, %05d, %05d) PP_CRC=%x tmds_ch=%x\n",
					dfe_arg, ch_r_err, ch_g_err, ch_b_err, bit_r_err, bit_g_err, bit_b_err,crc, tmds_ch);
			}

			lib_hdmi_dfe_init_tap1(nport, (LN_B), phy_st[nport].dfe_t[0].tap1+phy_st[nport].dfe_t[0].le);
			lib_hdmi_dfe_init_tap1(nport, (LN_G), phy_st[nport].dfe_t[1].tap1+phy_st[nport].dfe_t[1].le);
			lib_hdmi_dfe_init_tap1(nport, (LN_R), phy_st[nport].dfe_t[2].tap1+phy_st[nport].dfe_t[2].le);
	}

	

	//tap2 -20~20
	for (dfe_arg=-20; dfe_arg<21; dfe_arg++) {
		//lib_hdmi_dfe_init_tap0(nport, (LN_R|LN_G|LN_B), dfe_arg);
		//lib_hdmi_dfe_init_tap1(nport, (LN_R|LN_G|LN_B), dfe_arg);
		lib_hdmi_dfe_init_tap2(nport, bgrc, dfe_arg);
		udelay(100);
#if ERR_DET_ONETIME_MODE
		lib_hdmi_bit_err_start(nport, 0, 63);
		lib_hdmi_char_err_start(nport, 0, 63);
		lib_hdmi_clear_rgb_hv_status(nport);
		while (!lib_hdmi_bit_err_get_error(nport, &bit_r_err, &bit_g_err, &bit_b_err)) {
			msleep(20);
		}
		while (!lib_hdmi_char_err_get_error(nport, &ch_r_err, &ch_g_err, &ch_b_err)) {
			msleep(20);
		}
#else
		lib_hdmi_bit_err_start(nport, 1, 10);
		lib_hdmi_char_err_start(nport, 1, 10);
		lib_hdmi_clear_rgb_hv_status(nport);
		msleep(1500);
		lib_hdmi_bit_err_get_error(nport, &bit_r_err, &bit_g_err, &bit_b_err);
		lib_hdmi_char_err_get_error(nport, &ch_r_err, &ch_g_err, &ch_b_err);
#endif

		crc = lib_hdmi_read_crc(nport);
		tmds_ch = (lib_hdmi_read_rgb_hv_status(nport) & 0x000000F8) >> 3;
		HDMI_EMG("tap2=%02d, [CH](%05d, %05d, %05d); [BIT](%05d, %05d, %05d) PP_CRC=%x tmds_ch=%x\n",
			dfe_arg, ch_r_err, ch_g_err, ch_b_err, bit_r_err, bit_g_err, bit_b_err,crc,tmds_ch);

	}

	lib_hdmi_dfe_init_tap2(nport, (LN_B), phy_st[nport].dfe_t[0].tap2);
	lib_hdmi_dfe_init_tap2(nport, (LN_G), phy_st[nport].dfe_t[1].tap2);
	lib_hdmi_dfe_init_tap2(nport, (LN_R), phy_st[nport].dfe_t[2].tap2);

	//disable bit error/character detect
	lib_hdmi_bit_err_start(nport, 0, 0);
	lib_hdmi_char_err_start(nport, 0, 0);

	
	lib_hdmi_set_fw_debug_bit(DEBUG_27_BYPASS_PHY_ISR, 0);
	lib_hdmi_set_fw_debug_bit(DEBUG_26_BYPASS_DETECT, 0);
	lib_hdmi_set_fw_debug_bit(DEBUG_25_BYPASS_CHECK, 0);

	HDMI_EMG("================SCAN END===================\n");


}
#endif

#if BIST_PHY_SCAN

int newbase_get_phy_scan_done(unsigned char nport)
{
	if (phy_scan_state == PHY_END_SCAN)
		return 1;
	else {
		phy_st[nport].clk_pre = 0;
		phy_st[nport].clk= 0;
		phy_st[nport].clk_debounce_count= 0;
		phy_st[nport].clk_unstable_count = 0;
		phy_st[nport].clk_stable = 0;
		phy_st[nport].phy_proc_state = PHY_PROC_INIT;
		return 0;
	}
}

void debug_hdmi_dump_msg(unsigned char nport)
{
	unsigned int bit_r_err, bit_g_err, bit_b_err, ch_r_err, ch_g_err, ch_b_err;
	//unsigned int i = 0;
	
	lib_hdmi_set_fw_debug_bit(DEBUG_26_BYPASS_DETECT, 1);
	lib_hdmi_set_fw_debug_bit(DEBUG_25_BYPASS_CHECK, 1);

	lib_hdmi_bch_1bit_error_clr(nport);
	lib_hdmi_bch_2bit_error_clr(nport);
	lib_hdmi_clear_rgb_hv_status(nport);
	lib_hdmi_bit_err_start(nport, 1, 10);
	lib_hdmi_char_err_start(nport, 1, 10);
	msleep(50);
	lib_hdmi_bit_err_get_error(nport, &bit_r_err, &bit_g_err, &bit_b_err);
	lib_hdmi_char_err_get_error(nport, &ch_r_err, &ch_g_err, &ch_b_err);
	HDMI_EMG("bch1=%d bch2=%d\n",lib_hdmi_get_bch_1bit_error(nport),lib_hdmi_get_bch_2bit_error(nport));
	HDMI_EMG("BGRHV = %x\n",(lib_hdmi_read_rgb_hv_status(nport) & 0x000000F8) >> 3);
	HDMI_EMG("[CH B G R](%05d, %05d, %05d); [BIT B G R](%05d, %05d, %05d)\n",  ch_b_err, ch_g_err, ch_r_err , bit_b_err, bit_g_err, bit_r_err);
	
	lib_hdmi_set_fw_debug_bit(DEBUG_26_BYPASS_DETECT, 0);
	lib_hdmi_set_fw_debug_bit(DEBUG_25_BYPASS_CHECK, 0);
}

void debug_hdmi_phy_scan(unsigned char nport)
{
#if 0  // FIXME : phy register has been changed...
/*

CK_ICP  b800da54[7:4] = 0x8 ~ 0xe
KP=6	b800da90[15:8] = 0x18
KP=7    b800da90[15:8] = 0x1c
KP=8	b800da90[15:8] = 0x20
KP=9    b800da90[15:8] = 0x24
KP=10	b800da90[15:8] = 0x28
*/

	#define MAX_ICP 0xE
	#define MIN_ICP 0x8
	#define MAX_KP 0xC
	#define MIN_KP 0x4
	
	static unsigned int original_icp, original_kp;
	static unsigned int modify_icp, modify_kp;
	unsigned int wReg2,wCk2Reg;


	if (nport == HDMI_PORT0) {
		wReg2 = DFE_HDMI_RX_PHY_REGD0_reg;
		wCk2Reg = DFE_HDMI_RX_PHY_P0_CK2_reg;
	} else if (nport == HDMI_PORT1) {
		wReg2 = DFE_HDMI_RX_PHY_REGD5_reg;
		wCk2Reg = DFE_HDMI_RX_PHY_P1_CK2_reg;
	} else if (nport == HDMI_PORT2) {
		wReg2 = DFE_HDMI_RX_PHY_REGD10_reg;
		wCk2Reg = DFE_HDMI_RX_PHY_P2_CK2_reg;
	} else if (nport == HDMI_PORT3) {
		wReg2 = DFE_HDMI_RX_PHY_P0_HD21_Y1_REGD00_reg;
		wCk2Reg = DFE_HDMI_RX_PHY_P0_HD21_P0_CMU2_reg;
	} else {
		return;
	}

	msleep(5000);
	
	switch(phy_scan_state) {
		case PHY_INIT_SCAN:
			HDMI_EMG("PHY_INIT_SCAN");
			original_icp = (((hdmi_in(wCk2Reg) & P0_ck_5_CMU_SEL_PUMP_I_mask)) >> 4);
			original_kp = 	DFE_HDMI_RX_PHY_REGD0_get_reg_p0_kp(hdmi_in(wReg2));
			modify_icp = MIN_ICP;
			modify_kp = MIN_KP;
			phy_scan_state = PHY_ICP_SCAN;
			phy_st[nport].clk_pre = 0;
			phy_st[nport].clk= 0;
			phy_st[nport].clk_debounce_count= 0;
			phy_st[nport].clk_unstable_count = 0;
			phy_st[nport].clk_stable = 0;
			phy_st[nport].phy_proc_state = PHY_PROC_INIT;
			break;
		
		case PHY_ICP_SCAN:
			if ((modify_icp) > MAX_ICP) {
				hdmi_mask(wCk2Reg, P0_ck_5_CMU_SEL_PUMP_I_mask,  (original_icp << 4));
				phy_scan_state = PHY_KP_SCAN;
				
			} else {

				HDMI_EMG("PHY_ICP_SCAN");
				HDMI_EMG("ICP=%d\n",modify_icp);
				hdmi_mask(wCk2Reg, P0_ck_5_CMU_SEL_PUMP_I_mask,  (modify_icp << 4));
				modify_icp++;
			}
			break;

		case PHY_KP_SCAN:
			if ((modify_kp) > MAX_KP) {
				hdmi_mask(wReg2, DFE_HDMI_RX_PHY_REGD0_reg_p0_kp_mask,  ((original_kp << 2) << DFE_HDMI_RX_PHY_REGD0_reg_p0_kp_shift));
				phy_scan_state = PHY_END_SCAN;
				
			} else {

				HDMI_EMG("PHY_KP_SCAN");
				HDMI_EMG("KP=%d\n",modify_kp);
				hdmi_mask(wReg2, DFE_HDMI_RX_PHY_REGD0_reg_p0_kp_mask, ((modify_kp << 2) << DFE_HDMI_RX_PHY_REGD0_reg_p0_kp_shift));
				modify_kp++;
			}
			break;

		case PHY_END_SCAN:
			HDMI_EMG("PHY_END_SCAN");
			lib_hdmi_set_fw_debug_bit(DEBUG_22_PHY_SCAN_TEST,0x0);
			break;

	}
#endif
}

#endif

#ifndef BUILD_QUICK_SHOW
//------------------------------------------------------------------------------
// HDMI PHY ISR 
//------------------------------------------------------------------------------

static bool hdmiPhyEnable = false;
static struct task_struct *hdmi_task;


static int _hdmi_rxphy_thread(void* arg)
{
	while (!kthread_should_stop() && hdmiPhyEnable == true) 
	{
		newbase_rxphy_isr();
		msleep(10); // sleep 10 ms
	}

	return 0;
}

void newbase_rxphy_isr_enable(char enable)
{
	int err = 0;

	if (enable)
	{
		if (hdmiPhyEnable != true)
		{
			hdmi_task = kthread_create(_hdmi_rxphy_thread, NULL, "hdmi_task");

			if (IS_ERR(hdmi_task))
			{
				err = PTR_ERR(hdmi_task);
				hdmi_task = NULL;
				hdmiPhyEnable = false;
				HDMI_EMG("%s , Unable to start kernel thread (err_id = %d),HDMI_phy\n", __func__,err);
				return ;
			}

			hdmiPhyEnable = true;
			wake_up_process(hdmi_task);
			HDMI_PRINTF("%s hdmi phy thread started\n" , __func__ );
		}
	}
	else
	{
		if (hdmiPhyEnable == true)
		{
			int ret = kthread_stop(hdmi_task);
			hdmiPhyEnable = false;

			if (ret==0)
			{
				HDMI_INFO("hdmi phy thread stopped\n");
			}
		}
	}
}
#endif

HDMIRX_PHY_STRUCT_T* newbase_rxphy_get_status(unsigned char port)
{
	return (port < HDMI_PORT_TOTAL_NUM) ? &phy_st[port] : NULL;
}


