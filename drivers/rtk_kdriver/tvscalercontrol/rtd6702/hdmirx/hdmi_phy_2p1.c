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

#include <linux/kthread.h> //kthread_create()¡Bkthread_run()
#include <linux/err.h> //IS_ERR()¡BPTR_ERR()
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <mach/platform.h>
#include "hdmi_common.h"
//#include "hdmi_phy.h"
#include "hdmi_phy_dfe.h"
#include "hdmi_phy_2p1.h"
#include "hdmi_hdcp.h"
#include "hdmi_scdc.h"
#include "hdmi_reg.h"
#include "hdmi_vfe_config.h"

extern HDMI_PORT_INFO_T hdmi_rx[HDMI_PORT_TOTAL_NUM];
int factory_or_power_only_mode_hd21_eq_ofst = 2;
unsigned char PHY_DBG_MSG=0;
/**********************************************************************************************
*
*	Marco or Definitions
*
**********************************************************************************************/
#define HDMI_PHY_DBG(fmt, args...)  if(PHY_DBG_MSG) rtd_pr_hdmi_debug("[HDMI][PHY][L7]" fmt, ##args)
#define HDMI_PHY_INFO(fmt, args...)  if(PHY_DBG_MSG) rtd_pr_hdmi_info("[HDMI][PHY][L6]" fmt, ##args)
#define HDMI_PHY_WARN(fmt, args...)  if(PHY_DBG_MSG) rtd_pr_hdmi_warn("[HDMI][PHY][L4]" fmt, ##args)



#define HD21_PHY_Z0        			0x6

#define HD21_PHY_LDO        			0x4	//LDO default value

#define HDMI_EQ_LE_NOT_ZERO_ENABLE
#define HD21_CLK_OVER_300M			1
#define FRL_DCDR_3G_6G_MODE		0

#define HD21_MAX_ERR_CHECK_CNT	200
#define HD21_DISP_ERR_CNT_THRESHOLD	4
#define LINK_TRAINING_TX_INFO_DEBUG	0

#define ADP_CNT			9
#define CALIB_CNT		7
#define ADP_TIME		6     //0.64 * 9
#define CALIB_TIME		5    //0.64 * 7

//#define FRL_DBG(fmt, args...)  if(FRL_FLOW_PRINT_FLAG) rtd_pr_hdmi_debug("[HDMI][HD21-FRL][L7]" fmt, ##args)

#ifndef MAX
    #define MAX(a,b)                (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
    #define MIN(a,b)                (((a) < (b)) ? (a) : (b))

	
#endif
u32 phy_reg_mapping(u32 nport, u32 addr)
{
	if (addr >= 0x180b7C00 && addr <= 0x180b7DFF)
	{
		addr -= 0x180b7C00;
		switch(nport) {
		case 0:  addr += 0xB80B7C00; break;
		case 1:  addr += 0xB80B9C00; break;
		case 2:  addr += 0xB80BAC00; break;
		case 3:  addr += 0xB80BCC00; break;
		default: addr += 0xB80B7C00; break;
        	}
	}
	else if (addr >= 0x1800DA00 && addr <= 0x1800DDFF)
	{
		addr -= 0x1800DA00;
		switch(nport) {
		case 0:  addr += 0xB800DA00; break;
		case 1:  addr += 0xB803BA00; break;
		case 2:  addr += 0xB803CA00; break;
		case 3:  addr += 0xB80B2A00; break;
		default: addr += 0xB800DA00; break;
		}
	}
	else
	{
		HDMI_WARN("%s unknown address = %08x\n", __func__, addr);
	}
	//HDMI_WARN("p[%d] addr=%08x\n", nport, addr);
	return addr;
}

#define ScalerTimer_DelayXms(x)                 udelay((x<10) ? (x*100) : 5000)
#define _rtd_outl(addr, val)                    rtd_outl(phy_reg_mapping(nport, addr), val)
#define _rtd_part_outl(addr, msb, lsb, val)     rtd_part_outl(phy_reg_mapping(nport, addr), msb, lsb, val)
#define _rtd_inl(addr, val)                     rtd_inl(phy_reg_mapping(nport, addr))
#define _rtd_part_inl(addr, msb, lsb)           rtd_part_inl(phy_reg_mapping(nport, addr), msb, lsb)

//Driver Code  Copy from Golden setting generator TMDS mode
APHY_Para_tmds_1_T APHY_Para_tmds_1[] =
{
	{TMDS_5_94G,0x0,0x3,0x24,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x0,0x2,0x1,0x1,0xf,0x0,0x0,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,
0xf,0x0,0x1,0x0,0x1,0x0,0x3,0x0,0x2,0x1,0x1,0xf,0x0,0x0,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x0,0x3,0x0,0x2,0x1,0x1,0xf,0x0,0x0,0x2},
	{TMDS_2_97G,0x0,0x3,0x10,0x1,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x3,0x2,0x1,0xf,0x0,0x1,0x1,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,
0x0,0x0,0x0,0x1,0x0,0x0,0x3,0x0,0x3,0x2,0x1,0xf,0x0,0x1,0x1,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x3,0x0,0x3,0x2,0x1,0xf,0x0,0x1,0x1},
	{TMDS_1_485G,0x0,0x3,0x24,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x1,0xf,0x0,0x2,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,
0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x0,0x3,0x3,0x1,0xf,0x0,0x2,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x0,0x3,0x3,0x1,0xf,0x0,0x2,0x0},
	{TMDS_0_742G,0x1,0x3,0x4c,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x1,0xf,0x0,0x3,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,
0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x0,0x3,0x3,0x1,0xf,0x0,0x3,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x0,0x3,0x3,0x1,0xf,0x0,0x3,0x0},
	{TMDS_0_25G,0x1,0x0,0x9c,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0x7,0x6,0x4,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,
0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x0,0x3,0x3,0x0,0x7,0x6,0x4,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x0,0x3,0x3,0x0,0x7,0x6,0x4,0x0},
	{TMDS_4_445G,0x0,0x2,0x24,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x1,0x2,0x1,0x0,0xc,0x3,0x0,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,
0xf,0x0,0x1,0x0,0x1,0x0,0x3,0x0,0x2,0x1,0x0,0xc,0x0,0x0,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x0,0x3,0x0,0x2,0x1,0x0,0xc,0x3,0x0,0x2},
	{TMDS_3_7125G,0x0,0x0,0x24,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x0,0x2,0x1,0x0,0x7,0x6,0x0,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x0,0x3,0x0,0x2,0x1,0x0,0x7,0x0,0x0,0x2,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x0,0x1,0x0,0x1,0x0,0x3,0x0,0x2,0x1,0x0,0x7,0x6,0x0,0x2},
	{TMDS_2_2275G,0x0,0x2,0x10,0x1,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x3,0x2,0x0,0xc,0x3,0x1,0x1,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x3,0x0,0x3,0x2,0x0,0xc,0x3,0x1,0x1,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x3,0x0,0x3,0x2,0x0,0xc,0x3,0x1,0x1},
	{TMDS_1_85625G,0x0,0x0,0x10,0x0,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x3,0x2,0x0,0x7,0x6,0x1,0x0,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x3,0x0,0x3,0x2,0x0,0x7,0x6,0x1,0x0,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x0,0x3,0x0,0x3,0x2,0x0,0x7,0x6,0x1,0x0},
	{TMDS_1_11375G,0x0,0x2,0x24,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0xc,0x3,0x2,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x0,0x3,0x3,0x0,0xc,0x3,0x2,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x0,0x3,0x3,0x0,0xc,0x3,0x2,0x0},
	{TMDS_0_928125G,0x0,0x0,0x24,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0x7,0x6,0x2,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,
0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x0,0x3,0x3,0x0,0x7,0x6,0x2,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x0,0x3,0x3,0x0,0x7,0x6,0x2,0x0},
	{TMDS_0_54G,0x1,0x2,0x4c,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0xc,0x3,0x3,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,
0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x0,0x3,0x3,0x0,0xc,0x3,0x3,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x0,0x3,0x3,0x0,0xc,0x3,0x3,0x0},
	{TMDS_0_405G,0x1,0x0,0x4c,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0x7,0x6,0x3,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,
0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x0,0x3,0x3,0x0,0x7,0x6,0x3,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x0,0x3,0x3,0x0,0x7,0x6,0x3,0x0},
	{TMDS_0_3375G,0x1,0x3,0x9c,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x1,0xf,0x0,0x4,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x0,0x0,0x2,0x3,0x3,0x1,0xf,0x0,0x4,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x0,0x0,0x2,0x3,0x3,0x1,0xf,0x0,0x4,0x0},
	{TMDS_0_27G,0x1,0x2,0x9c,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x3,0x3,0x0,0xc,0x3,0x4,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,
0x0,0x1,0x0,0x1,0x0,0x0,0x0,0x2,0x3,0x3,0x0,0xc,0x3,0x4,0x0,0x0,0x0,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x0,0x0,0x2,0x3,0x3,0x0,0xc,0x3,0x4,0x0},
};

DFE_ini_tmds_1_T DFE_ini_tmds_1[] =
{
	{TMDS_5_94G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_2_97G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_1_485G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_0_742G,0x5,0x10,0x5,0x10,0x5,0x10,0xf,0x10},
	{TMDS_0_25G,0x0,0x19,0x0,0x19,0x0,0x19,0x0,0x19},
	{TMDS_4_445G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_3_7125G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_2_2275G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_1_85625G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_1_11375G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_0_928125G,0xf,0x10,0xf,0x10,0xf,0x10,0xf,0x10},
	{TMDS_0_54G,0x5,0x10,0x5,0x10,0x5,0x10,0xf,0x10},
	{TMDS_0_405G,0x5,0x10,0x5,0x10,0x5,0x10,0xf,0x10},
	{TMDS_0_3375G,0x0,0x19,0x0,0x19,0x0,0x19,0x0,0x19},
	{TMDS_0_27G,0x0,0x19,0x0,0x19,0x0,0x19,0x0,0x19},
};

DPHY_Para_tmds_1_T DPHY_Para_tmds_1[] =
{
	{TMDS_5_94G,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x2,0x98,0x2f,0x2,0x98,0x2f},
	{TMDS_2_97G,0x12,0x12,0x12,0x12,0x8,0x8,0x8,0x8,0x1,0x1,0x1,0x1,0x2,0x2,0x2,0x1,0x1,0x1,0x2,0x98,0x27,0x2,0x98,0x27},
	{TMDS_1_485G,0x14,0x14,0x14,0x14,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x0,0x3,0x3,0x3,0x1,0x1,0x1,0x2,0x98,0x2f,0x2,0x98,0x2f},
	{TMDS_0_742G,0x14,0x14,0x14,0x14,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x0,0x3,0x3,0x3,0x1,0x1,0x1,0x6,0x98,0x2d,0x6,0x98,0x2d},
	{TMDS_0_25G,0x14,0x14,0x14,0x14,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x0,0x3,0x3,0x3,0x1,0x1,0x1,0x6,0xa0,0x2f,0x6,0xa0,0x2f},
	{TMDS_4_445G,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x2,0x98,0x2f,0x2,0x98,0x2f},
	{TMDS_3_7125G,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x0,0x0,0x0,0x2,0x98,0x2f,0x2,0x98,0x2f},
	{TMDS_2_2275G,0x12,0x12,0x12,0x12,0x4,0x4,0x4,0x4,0x1,0x1,0x1,0x1,0x2,0x2,0x2,0x1,0x1,0x1,0x2,0x98,0x27,0x2,0x98,0x27},
	{TMDS_1_85625G,0x12,0x12,0x12,0x12,0x4,0x4,0x4,0x4,0x1,0x1,0x1,0x1,0x2,0x2,0x2,0x1,0x1,0x1,0x2,0x98,0x27,0x2,0x98,0x27},
	{TMDS_1_11375G,0x14,0x14,0x14,0x14,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x0,0x3,0x3,0x3,0x1,0x1,0x1,0x2,0x98,0x2f,0x2,0x98,0x2f},
	{TMDS_0_928125G,0x14,0x14,0x14,0x14,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x0,0x3,0x3,0x3,0x1,0x1,0x1,0x2,0x98,0x2f,0x2,0x98,0x2f},
	{TMDS_0_54G,0x14,0x14,0x14,0x14,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x0,0x3,0x3,0x3,0x1,0x1,0x1,0x6,0x98,0x2d,0x6,0x98,0x2d},
	{TMDS_0_405G,0x14,0x14,0x14,0x14,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x0,0x3,0x3,0x3,0x1,0x1,0x1,0x6,0x98,0x2d,0x6,0x98,0x27},
	{TMDS_0_3375G,0x14,0x14,0x14,0x14,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x0,0x3,0x3,0x3,0x1,0x1,0x1,0x6,0xa0,0x2f,0x6,0xa0,0x2f},
	{TMDS_0_27G,0x14,0x14,0x14,0x14,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x0,0x3,0x3,0x3,0x1,0x1,0x1,0x6,0xa0,0x2f,0x6,0xa0,0x2f},
};

Koffset_tmds_1_T Koffset_tmds_1[] =
{
	{TMDS_5_94G,0x1,0x1,0x1,0xf,0xf,0xf},
	{TMDS_2_97G,0x0,0x0,0x0,0x3,0x3,0x3},
	{TMDS_1_485G,0x0,0x0,0x0,0x1,0x1,0x1},
	{TMDS_0_742G,0x0,0x0,0x0,0x1,0x1,0x1},
	{TMDS_0_25G,0x0,0x0,0x0,0x1,0x1,0x1},
	{TMDS_4_445G,0x0,0x0,0x0,0xf,0xf,0xf},
	{TMDS_3_7125G,0x0,0x0,0x0,0xf,0xf,0xf},
	{TMDS_2_2275G,0x0,0x0,0x0,0x3,0x3,0x3},
	{TMDS_1_85625G,0x0,0x0,0x0,0x3,0x3,0x3},
	{TMDS_1_11375G,0x0,0x0,0x0,0x1,0x1,0x1},
	{TMDS_0_928125G,0x0,0x0,0x0,0x1,0x1,0x1},
	{TMDS_0_54G,0x0,0x0,0x0,0x1,0x1,0x1},
	{TMDS_0_405G,0x0,0x0,0x0,0x1,0x1,0x1},
	{TMDS_0_3375G,0x0,0x0,0x0,0x1,0x1,0x1},
	{TMDS_0_27G,0x0,0x0,0x0,0x1,0x1,0x1},
};

LEQ_VTH_Tap0_3_4_Adapt_tmds_1_T LEQ_VTH_Tap0_3_4_Adapt_tmds_1[] =
{
	{TMDS_5_94G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_2_97G,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0},
	{TMDS_1_485G,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0},
	{TMDS_0_742G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0_25G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_4_445G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_3_7125G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_2_2275G,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0},
	{TMDS_1_85625G,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0},
	{TMDS_1_11375G,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0},
	{TMDS_0_928125G,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0},
	{TMDS_0_54G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0_405G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0_3375G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_0_27G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
};

Tap0_to_Tap4_Adapt_tmds_1_T Tap0_to_Tap4_Adapt_tmds_1[] =
{
	{TMDS_5_94G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_2_97G,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0},
	{TMDS_1_485G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0_742G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0_25G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_4_445G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_3_7125G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_2_2275G,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0},
	{TMDS_1_85625G,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0},
	{TMDS_1_11375G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0_928125G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0_54G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0_405G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
	{TMDS_0_3375G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_0_27G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
};

void TMDS_Merged_V1p2_20220420_Main_Seq(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing)
{
	if((tmds_timing==TMDS_5_94G) || (tmds_timing==TMDS_2_97G) || (tmds_timing==TMDS_1_485G) || (tmds_timing==TMDS_0_742G) || (tmds_timing==TMDS_4_445G) || (tmds_timing==TMDS_3_7125G) || (tmds_timing==TMDS_2_2275G) || (tmds_timing==TMDS_1_85625G) || (tmds_timing==TMDS_1_11375G) || (tmds_timing==TMDS_0_928125G) || (tmds_timing==TMDS_0_54G) || (tmds_timing==TMDS_0_405G) || (tmds_timing==TMDS_0_3375G) || (tmds_timing==TMDS_0_27G) )
	{
		DFE_ini_tmds_1_func(nport, tmds_timing);
		DPHY_Fix_tmds_1_func(nport, tmds_timing);
		DPHY_Para_tmds_1_func(nport, tmds_timing);
		APHY_Fix_tmds_1_func(nport, tmds_timing);
		APHY_Para_tmds_1_func(nport, tmds_timing);
		DPHY_Init_Flow_tmds_1_func(nport, tmds_timing);
		APHY_Init_Flow_tmds_1_func(nport, tmds_timing);
		DCDR_settings_tmds_1_func(nport, tmds_timing);
		DCDR_settings_tmds_2_func(nport, tmds_timing);
		Koffset_tmds_1_func(nport, tmds_timing);
		LEQ_VTH_Tap0_3_4_Adapt_tmds_1_func(nport, tmds_timing);
		Koffset_tmds_1_func(nport, tmds_timing);
		Tap0_to_Tap4_Adapt_tmds_1_func(nport, tmds_timing);
		Koffset_tmds_1_func(nport, tmds_timing);
		DCDR_settings_tmds_3_func(nport, tmds_timing);
	}
	else if(tmds_timing==TMDS_0_25G)
	{
		DFE_ini_tmds_1_func(nport, tmds_timing);
		DPHY_Fix_tmds_1_func(nport, tmds_timing);
		DPHY_Para_tmds_1_func(nport, tmds_timing);
		APHY_Fix_tmds_1_func(nport, tmds_timing);
		APHY_Para_tmds_1_func(nport, tmds_timing);
		DPHY_Init_Flow_tmds_1_func(nport, tmds_timing);
		APHY_Init_Flow_tmds_1_func(nport, tmds_timing);
		DCDR_settings_tmds_1_func(nport, tmds_timing);
		DCDR_settings_tmds_2_func(nport, tmds_timing);
		Koffset_tmds_1_func(nport, tmds_timing);
		Manual_DFE_tmds_1_func(nport, tmds_timing);
		Koffset_tmds_1_func(nport, tmds_timing);
		DCDR_settings_tmds_3_func(nport, tmds_timing);
	}
}

void DFE_ini_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing)
{
	_rtd_part_outl(0x1800dd70, 31, 0, 0x00000000);
	//R lane
	_rtd_part_outl(0x180b7c00, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7c04, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7c08, 31, 0, 0x20C00000);
	_rtd_part_outl(0x180b7c0c, 31, 0, 0x05b47400);
	_rtd_part_outl(0x180b7c10, 31, 0, 0xc0004000);
	_rtd_part_outl(0x180b7c14, 31, 0, 0x00280000);
	_rtd_part_outl(0x180b7c18, 31, 0, 0x1038ffe0);
	_rtd_part_outl(0x180b7c1c, 31, 0, 0x7FF07FE0);
	_rtd_part_outl(0x180b7c20, 31, 0, 0x3e0383e0);
	_rtd_part_outl(0x180b7c24, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7c28, 31, 0, 0x009D00E7);
	_rtd_part_outl(0x180b7c2c, 31, 0, 0xFF00FFC0);
	_rtd_part_outl(0x180b7c30, 31, 0, 0xff005400);
	//G lane
	_rtd_part_outl(0x180b7c40, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7c44, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7c48, 31, 0, 0x20C00000);
	_rtd_part_outl(0x180b7c4c, 31, 0, 0x05b47400);
	_rtd_part_outl(0x180b7c50, 31, 0, 0xc0004000);
	_rtd_part_outl(0x180b7c54, 31, 0, 0x00280000);
	_rtd_part_outl(0x180b7c58, 31, 0, 0x1038ffe0);
	_rtd_part_outl(0x180b7c5c, 31, 0, 0x7FF07FE0);
	_rtd_part_outl(0x180b7c60, 31, 0, 0x3e0383e0);
	_rtd_part_outl(0x180b7c64, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7c68, 31, 0, 0x009D00E7);
	_rtd_part_outl(0x180b7c6c, 31, 0, 0xFF00FFC0);
	_rtd_part_outl(0x180b7c70, 31, 0, 0xff005400);
	//B lane
	_rtd_part_outl(0x180b7c80, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7c84, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7c88, 31, 0, 0x20C00000);
	_rtd_part_outl(0x180b7c8c, 31, 0, 0x05b47400);
	_rtd_part_outl(0x180b7c90, 31, 0, 0xc0004000);
	_rtd_part_outl(0x180b7c94, 31, 0, 0x00280000);
	_rtd_part_outl(0x180b7c98, 31, 0, 0x1038ffe0);
	_rtd_part_outl(0x180b7c9c, 31, 0, 0x7FF07FE0);
	_rtd_part_outl(0x180b7ca0, 31, 0, 0x3e0383e0);
	_rtd_part_outl(0x180b7ca4, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7ca8, 31, 0, 0x009D00E7);
	_rtd_part_outl(0x180b7cac, 31, 0, 0xFF00FFC0);
	_rtd_part_outl(0x180b7cb0, 31, 0, 0xff005400);
	//CK lane
	_rtd_part_outl(0x180b7cc0, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7cc4, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7cc8, 31, 0, 0x20C00000);
	_rtd_part_outl(0x180b7ccc, 31, 0, 0x05b47400);
	_rtd_part_outl(0x180b7cd0, 31, 0, 0xc0004000);
	_rtd_part_outl(0x180b7cd4, 31, 0, 0x00280000);
	_rtd_part_outl(0x180b7cd8, 31, 0, 0x1038ffe0);
	_rtd_part_outl(0x180b7cdc, 31, 0, 0x7FF07FE0);
	_rtd_part_outl(0x180b7ce0, 31, 0, 0x3e0383e0);
	_rtd_part_outl(0x180b7ce4, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7ce8, 31, 0, 0x009D00E7);
	_rtd_part_outl(0x180b7cec, 31, 0, 0xFF00FFC0);
	_rtd_part_outl(0x180b7cf0, 31, 0, 0xff005400);
	// *************************************************************************************************************************************
	//B-Lane_initial DFE Value_Start
	_rtd_part_outl(0x180b7ca8, 9, 5, 5); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_rtd_part_outl(0x180b7ca8, 4, 0, 5); //VTHN_INIT_B[4:0]	4:0	R/W	0x0F		VTHN INIT
	_rtd_part_outl(0x180b7ca0, 4, 0, DFE_ini_tmds_1[tmds_timing].LEQ_INIT_B_4_0_62); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_rtd_part_outl(0x180b7ca4, 31, 24, DFE_ini_tmds_1[tmds_timing].TAP0_INIT_B_7_0_63); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_rtd_part_outl(0x180b7ca4, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_rtd_part_outl(0x180b7ca4, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_rtd_part_outl(0x180b7ca4, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_rtd_part_outl(0x180b7ca4, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//B-Lane DFE COEF LOAD IN
	_rtd_part_outl(0x180b7c8c, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_rtd_part_outl(0x180b7c8c, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_rtd_part_outl(0x180b7c8c, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_rtd_part_outl(0x180b7c8c, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_rtd_part_outl(0x180b7c8c, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_rtd_part_outl(0x180b7c8c, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_rtd_part_outl(0x180b7c88, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_rtd_part_outl(0x180b7c8c, 5, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_rtd_part_outl(0x180b7c88, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	//B-Lane_initial DFE Value_Stop
	// *************************************************************************************************************************************
	//G-Lane_initial DFE Value_Start
	_rtd_part_outl(0x180b7c68, 9, 5, 5); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_rtd_part_outl(0x180b7c68, 4, 0, 5); //VTHN_INIT_B[4:0]	4:0	R/W	0x0F		VTHN INIT
	_rtd_part_outl(0x180b7c60, 4, 0, DFE_ini_tmds_1[tmds_timing].LEQ_INIT_G_4_0_83); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_rtd_part_outl(0x180b7c64, 31, 24, DFE_ini_tmds_1[tmds_timing].TAP0_INIT_G_7_0_84); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_rtd_part_outl(0x180b7c64, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_rtd_part_outl(0x180b7c64, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_rtd_part_outl(0x180b7c64, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_rtd_part_outl(0x180b7c64, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//G-Lane DFE COEF LOAD IN
	_rtd_part_outl(0x180b7c4c, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_rtd_part_outl(0x180b7c4c, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_rtd_part_outl(0x180b7c4c, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_rtd_part_outl(0x180b7c4c, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_rtd_part_outl(0x180b7c4c, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_rtd_part_outl(0x180b7c4c, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_rtd_part_outl(0x180b7c48, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_rtd_part_outl(0x180b7c4c, 5, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_rtd_part_outl(0x180b7c48, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	//G-Lane_initial DFE Value_Stop
	// *************************************************************************************************************************************
	//R-Lane_initial DFE Value_Start
	_rtd_part_outl(0x180b7c28, 9, 5, 5); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_rtd_part_outl(0x180b7c28, 4, 0, 5); //VTHN_INIT_B[4:0]	4:0	R/W	0x0F		VTHN INIT
	_rtd_part_outl(0x180b7c20, 4, 0, DFE_ini_tmds_1[tmds_timing].LEQ_INIT_R_4_0_104); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_rtd_part_outl(0x180b7c24, 31, 24, DFE_ini_tmds_1[tmds_timing].TAP0_INIT_R_7_0_105); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_rtd_part_outl(0x180b7c24, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_rtd_part_outl(0x180b7c24, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_rtd_part_outl(0x180b7c24, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_rtd_part_outl(0x180b7c24, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//R-Lane DFE COEF LOAD IN
	_rtd_part_outl(0x180b7c0c, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_rtd_part_outl(0x180b7c0c, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_rtd_part_outl(0x180b7c0c, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_rtd_part_outl(0x180b7c0c, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_rtd_part_outl(0x180b7c0c, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_rtd_part_outl(0x180b7c0c, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_rtd_part_outl(0x180b7c08, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_rtd_part_outl(0x180b7c0c, 5, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_rtd_part_outl(0x180b7c08, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	//R-Lane_initial DFE Value_Stop
	// *************************************************************************************************************************************
	//CK-Lane_initial DFE Value_Start
	_rtd_part_outl(0x180b7ce8, 9, 5, 5); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_rtd_part_outl(0x180b7ce8, 4, 0, 5); //VTHN_INIT_B`[4:0]	4:0	R/W	0x0F		VTHN INIT
	_rtd_part_outl(0x180b7ce0, 4, 0, DFE_ini_tmds_1[tmds_timing].LEQ_INIT_CK_4_0_125); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_rtd_part_outl(0x180b7ce4, 31, 24, DFE_ini_tmds_1[tmds_timing].TAP0_INIT_CK_7_0_126); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_rtd_part_outl(0x180b7ce4, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_rtd_part_outl(0x180b7ce4, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_rtd_part_outl(0x180b7ce4, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_rtd_part_outl(0x180b7ce4, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//CK-Lane DFE COEF LOAD IN
	_rtd_part_outl(0x180b7ccc, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_rtd_part_outl(0x180b7ccc, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_rtd_part_outl(0x180b7ccc, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_rtd_part_outl(0x180b7ccc, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_rtd_part_outl(0x180b7ccc, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_rtd_part_outl(0x180b7ccc, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_rtd_part_outl(0x180b7cc8, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_rtd_part_outl(0x180b7ccc, 5, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_rtd_part_outl(0x180b7cc8, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	//CK-Lane_initial DFE Value_Stop
	//**************************************************************************************************************************************
	_rtd_part_outl(0x180b7cf4, 17, 15, 3); //rotation_mode Adaption flow control mode
	_rtd_part_outl(0x180b7cb4, 17, 15, 3); //rotation_mode Adaption flow control mode
	_rtd_part_outl(0x180b7c74, 17, 15, 3); //rotation_mode Adaption flow control mode
	_rtd_part_outl(0x180b7c34, 17, 15, 3); //rotation_mode Adaption flow control mode
}

void DPHY_Fix_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing)
{
	//DPHY fix start
	_rtd_part_outl(0x1800dc00, 0, 0, 0x1); //[DPHY fix]  reg_p0_b_dig_rst_n
	_rtd_part_outl(0x1800dc00, 1, 1, 0x1); //[DPHY fix]  reg_p0_g_dig_rst_n
	_rtd_part_outl(0x1800dc00, 2, 2, 0x1); //[DPHY fix]  reg_p0_r_dig_rst_n
	_rtd_part_outl(0x1800dc00, 3, 3, 0x1); //[DPHY fix]  reg_p0_ck_dig_rst_n
	_rtd_part_outl(0x1800dc00, 4, 4, 0x1); //[DPHY fix]  reg_p0_b_cdr_rst_n
	_rtd_part_outl(0x1800dc00, 5, 5, 0x1); //[DPHY fix]  reg_p0_g_cdr_rst_n
	_rtd_part_outl(0x1800dc00, 6, 6, 0x1); //[DPHY fix]  reg_p0_r_cdr_rst_n
	_rtd_part_outl(0x1800dc00, 7, 7, 0x1); //[DPHY fix]  reg_p0_ck_cdr_rst_n
	_rtd_part_outl(0x1800dc00, 8, 8, 0x0); //[DPHY fix]  reg_p0_b_clk_inv
	_rtd_part_outl(0x1800dc00, 9, 9, 0x0); //[DPHY fix]  reg_p0_g_clk_inv
	_rtd_part_outl(0x1800dc00, 10, 10, 0x0); //[DPHY fix]  reg_p0_r_clk_inv
	_rtd_part_outl(0x1800dc00, 11, 11, 0x0); //[DPHY fix]  reg_p0_ck_clk_inv
	_rtd_part_outl(0x1800dc00, 12, 12, 0x0); //[DPHY fix]  reg_p0_b_shift_inv
	_rtd_part_outl(0x1800dc00, 13, 13, 0x0); //[DPHY fix]  reg_p0_g_shift_inv
	_rtd_part_outl(0x1800dc00, 14, 14, 0x0); //[DPHY fix]  reg_p0_r_shift_inv
	_rtd_part_outl(0x1800dc00, 15, 15, 0x0); //[DPHY fix]  reg_p0_ck_shift_inv
	_rtd_part_outl(0x1800dc00, 24, 24, 0x0); //[DPHY fix]  reg_p0_b_data_order
	_rtd_part_outl(0x1800dc00, 25, 25, 0x0); //[DPHY fix]  reg_p0_g_data_order
	_rtd_part_outl(0x1800dc00, 26, 26, 0x0); //[DPHY fix]  reg_p0_r_data_order
	_rtd_part_outl(0x1800dc00, 27, 27, 0x0); //[DPHY fix]  reg_p0_ck_data_order
	_rtd_part_outl(0x1800dc00, 28, 28, 0x0); //[DPHY fix]  reg_p0_b_dyn_kp_en
	_rtd_part_outl(0x1800dc00, 29, 29, 0x0); //[DPHY fix]  reg_p0_g_dyn_kp_en
	_rtd_part_outl(0x1800dc00, 30, 30, 0x0); //[DPHY fix]  reg_p0_r_dyn_kp_en
	_rtd_part_outl(0x1800dc00, 31, 31, 0x0); //[DPHY fix]  reg_p0_ck_dyn_kp_en
	_rtd_part_outl(0x1800dc04, 24, 24, 0x0); //[]  reg_p0_b_dfe_edge_out
	_rtd_part_outl(0x1800dc04, 25, 25, 0x0); //[]  reg_p0_b_dfe_edge_out
	_rtd_part_outl(0x1800dc04, 26, 26, 0x0); //[]  reg_p0_r_dfe_edge_out
	_rtd_part_outl(0x1800dc04, 27, 27, 0x0); //[]  reg_p0_ck_dfe_edge_out
	_rtd_part_outl(0x1800dc10, 0, 0, 0x0); //[]  reg_p0_b_bypass_sdm_int
	_rtd_part_outl(0x1800dc10, 11, 1, 0x000); //[]  reg_p0_b_int_init
	_rtd_part_outl(0x1800dc10, 21, 12, 0x000); //[]  reg_p0_b_acc2_period
	_rtd_part_outl(0x1800dc10, 22, 22, 0x0); //[]  reg_p0_b_acc2_manual
	_rtd_part_outl(0x1800dc10, 23, 23, 0x0); //[]  reg_p0_b_squ_tri
	_rtd_part_outl(0x1800dc14, 0, 0, 0x0); //[]  reg_p0_g_bypass_sdm_int
	_rtd_part_outl(0x1800dc14, 11, 1, 0x000); //[]  reg_p0_g_int_init
	_rtd_part_outl(0x1800dc14, 21, 12, 0x000); //[]  reg_p0_g_acc2_period
	_rtd_part_outl(0x1800dc14, 22, 22, 0x0); //[]  reg_p0_g_acc2_manual
	_rtd_part_outl(0x1800dc14, 23, 23, 0x0); //[]  reg_p0_g_squ_tri
	_rtd_part_outl(0x1800dc18, 0, 0, 0x0); //[]  reg_p0_r_bypass_sdm_int
	_rtd_part_outl(0x1800dc18, 11, 1, 0x000); //[]  reg_p0_r_int_init
	_rtd_part_outl(0x1800dc18, 21, 12, 0x000); //[]  reg_p0_r_acc2_period
	_rtd_part_outl(0x1800dc18, 22, 22, 0x0); //[]  reg_p0_r_acc2_manual
	_rtd_part_outl(0x1800dc18, 23, 23, 0x0); //[]  reg_p0_r_squ_tri
	_rtd_part_outl(0x1800dc1c, 0, 0, 0x0); //[]  reg_p0_ck_bypass_sdm_int
	_rtd_part_outl(0x1800dc1c, 11, 1, 0x000); //[]  reg_p0_ck_int_init
	_rtd_part_outl(0x1800dc1c, 21, 12, 0x000); //[]  reg_p0_ck_acc2_period
	_rtd_part_outl(0x1800dc1c, 22, 22, 0x0); //[]  reg_p0_ck_acc2_manual
	_rtd_part_outl(0x1800dc1c, 23, 23, 0x0); //[]  reg_p0_ck_squ_tri
	_rtd_part_outl(0x1800dc20, 0, 0, 0x0); //[]  reg_p0_b_pi_m_mode
	_rtd_part_outl(0x1800dc20, 3, 1, 0x0); //[]  reg_p0_b_testout_sel
	_rtd_part_outl(0x1800dc20, 8, 4, 0x00); //[]  reg_p0_b_timer_lpf
	_rtd_part_outl(0x1800dc20, 13, 9, 0x00); //[]  reg_p0_b_timer_eq
	_rtd_part_outl(0x1800dc20, 18, 14, 0x00); //[]  reg_p0_b_timer_ber
	_rtd_part_outl(0x1800dc24, 0, 0, 0x0); //[]  reg_p0_g_pi_m_mode
	_rtd_part_outl(0x1800dc24, 3, 1, 0x0); //[]  reg_p0_g_testout_sel
	_rtd_part_outl(0x1800dc24, 8, 4, 0x00); //[]  reg_p0_g_timer_lpf
	_rtd_part_outl(0x1800dc24, 13, 9, 0x00); //[]  reg_p0_g_timer_eq
	_rtd_part_outl(0x1800dc24, 18, 14, 0x00); //[]  reg_p0_g_timer_ber
	_rtd_part_outl(0x1800dc28, 0, 0, 0x0); //[]  reg_p0_r_pi_m_mode
	_rtd_part_outl(0x1800dc28, 3, 1, 0x0); //[]  reg_p0_r_testout_sel
	_rtd_part_outl(0x1800dc28, 8, 4, 0x00); //[]  reg_p0_r_timer_lpf
	_rtd_part_outl(0x1800dc28, 13, 9, 0x00); //[]  reg_p0_r_timer_eq
	_rtd_part_outl(0x1800dc28, 18, 14, 0x00); //[]  reg_p0_r_timer_ber
	_rtd_part_outl(0x1800dc2c, 0, 0, 0x0); //[]  reg_p0_ck_pi_m_mode
	_rtd_part_outl(0x1800dc2c, 3, 1, 0x0); //[]  reg_p0_ck_testout_sel
	_rtd_part_outl(0x1800dc2c, 8, 4, 0x00); //[]  reg_p0_ck_timer_lpf
	_rtd_part_outl(0x1800dc2c, 13, 9, 0x00); //[]  reg_p0_ck_timer_eq
	_rtd_part_outl(0x1800dc2c, 18, 14, 0x00); //[]  reg_p0_ck_timer_ber
	_rtd_part_outl(0x1800dc30, 31, 0, 0x000000ff); //[]  reg_p0_b_st_m_value
	_rtd_part_outl(0x1800dc34, 31, 0, 0x000000ff); //[]  reg_p0_g_st_m_value
	_rtd_part_outl(0x1800dc38, 31, 0, 0x000000ff); //[]  reg_p0_r_st_m_value
	_rtd_part_outl(0x1800dc3c, 31, 0, 0x000000ff); //[]  reg_p0_ck_st_m_value
	//Clock Ready
	_rtd_part_outl(0x1800dc40, 0, 0, 0x0); //[]  reg_p0_b_force_clkrdy
	_rtd_part_outl(0x1800dc40, 1, 1, 0x0); //[]  reg_p0_g_force_clkrdy
	_rtd_part_outl(0x1800dc40, 2, 2, 0x0); //[]  reg_p0_r_force_clkrdy
	_rtd_part_outl(0x1800dc40, 3, 3, 0x0); //[]  reg_p0_ck_force_clkrdy
	_rtd_part_outl(0x1800dc40, 7, 4, 0x0); //[]  reg_p0_b_timer_clk
	_rtd_part_outl(0x1800dc40, 11, 8, 0x0); //[]  reg_p0_g_timer_clk
	_rtd_part_outl(0x1800dc40, 15, 12, 0x0); //[]  reg_p0_r_timer_clk
	_rtd_part_outl(0x1800dc40, 19, 16, 0x0); //[]  reg_p0_ck_timer_clk
	_rtd_part_outl(0x1800dc44, 11, 8, 0xf); //[DPHY fix]  reg_p0_b_offset_delay_cnt
	_rtd_part_outl(0x1800dc44, 13, 12, 0x0); //[DPHY fix]  reg_p0_b_offset_stable_cnt
	_rtd_part_outl(0x1800dc44, 19, 14, 0x0b); //[DPHY fix]  reg_p0_b_offset_divisor
	_rtd_part_outl(0x1800dc44, 26, 20, 0x28); //[DPHY fix]  reg_p0_b_offset_timeout
	_rtd_part_outl(0x1800dc48, 0, 0, 0x0); //[]  reg_p0_b_offset_en_ope
	_rtd_part_outl(0x1800dc48, 1, 1, 0x0); //[]  reg_p0_b_offset_pc_ope
	_rtd_part_outl(0x1800dc48, 6, 2, 0x10); //[]  reg_p0_b_offset_ini_ope
	_rtd_part_outl(0x1800dc48, 7, 7, 0x0); //[]  reg_p0_b_offset_gray_en_ope
	_rtd_part_outl(0x1800dc48, 8, 8, 0x0); //[]  reg_p0_b_offset_manual_ope
	_rtd_part_outl(0x1800dc48, 9, 9, 0x0); //[]  reg_p0_b_offset_z0_ok_ope
	_rtd_part_outl(0x1800dc48, 10, 10, 0x0); //[]  reg_p0_b_offset_en_opo
	_rtd_part_outl(0x1800dc48, 11, 11, 0x0); //[]  reg_p0_b_offset_pc_opo
	_rtd_part_outl(0x1800dc48, 16, 12, 0x10); //[]  reg_p0_b_offset_ini_opo
	_rtd_part_outl(0x1800dc48, 17, 17, 0x0); //[]  reg_p0_b_offset_gray_en_opo
	_rtd_part_outl(0x1800dc48, 18, 18, 0x0); //[]  reg_p0_b_offset_manual_opo
	_rtd_part_outl(0x1800dc48, 19, 19, 0x0); //[]  reg_p0_b_offset_z0_ok_opo
	_rtd_part_outl(0x1800dc48, 20, 20, 0x0); //[]  reg_p0_b_offset_en_one
	_rtd_part_outl(0x1800dc48, 21, 21, 0x0); //[]  reg_p0_b_offset_pc_one
	_rtd_part_outl(0x1800dc48, 26, 22, 0x10); //[]  reg_p0_b_offset_ini_one
	_rtd_part_outl(0x1800dc48, 27, 27, 0x0); //[]  reg_p0_b_offset_gray_en_one
	_rtd_part_outl(0x1800dc48, 28, 28, 0x0); //[]  reg_p0_b_offset_manual_one
	_rtd_part_outl(0x1800dc48, 29, 29, 0x0); //[]  reg_p0_b_offset_z0_ok_one
	_rtd_part_outl(0x1800dc4c, 0, 0, 0x0); //[DPHY fix]  reg_p0_b_offset_en_ono
	_rtd_part_outl(0x1800dc4c, 1, 1, 0x0); //[DPHY fix]  reg_p0_b_offset_pc_ono
	_rtd_part_outl(0x1800dc4c, 6, 2, 0x10); //[DPHY fix]  reg_p0_b_offset_ini_ono
	_rtd_part_outl(0x1800dc4c, 7, 7, 0x0); //[DPHY fix]  reg_p0_b_offset_gray_en_ono
	_rtd_part_outl(0x1800dc4c, 8, 8, 0x0); //[DPHY fix]  reg_p0_b_offset_manual_ono
	_rtd_part_outl(0x1800dc4c, 9, 9, 0x0); //[DPHY fix]  reg_p0_b_offset_z0_ok_ono
	_rtd_part_outl(0x1800dc4c, 16, 12, 0x10); //[DPHY fix]  reg_p0_b_offset_ini_de
	_rtd_part_outl(0x1800dc4c, 17, 17, 0x0); //[DPHY fix]  reg_p0_b_offset_gray_en_de
	_rtd_part_outl(0x1800dc4c, 26, 22, 0x10); //[DPHY fix]  reg_p0_b_offset_ini_do
	_rtd_part_outl(0x1800dc4c, 27, 27, 0x0); //[DPHY fix]  reg_p0_b_offset_gray_en_do
	_rtd_part_outl(0x1800dc50, 6, 2, 0x10); //[DPHY fix]  reg_p0_b_offset_ini_ee
	_rtd_part_outl(0x1800dc50, 7, 7, 0x0); //[DPHY fix]  reg_p0_b_offset_gray_en_ee
	_rtd_part_outl(0x1800dc50, 16, 12, 0x10); //[DPHY fix]  reg_p0_b_offset_ini_eo
	_rtd_part_outl(0x1800dc50, 17, 17, 0x0); //[DPHY fix]  reg_p0_b_offset_gray_en_eo
	_rtd_part_outl(0x1800dc50, 26, 22, 0x10); //[DPHY fix]  reg_p0_b_offset_ini_eq
	_rtd_part_outl(0x1800dc50, 27, 27, 0x0); //[DPHY fix]  reg_p0_b_offset_gray_en_eq
	_rtd_part_outl(0x1800dc50, 29, 29, 0x0); //[DPHY fix]  reg_p0_b_offset_z0_ok_eq
	_rtd_part_outl(0x1800dc54, 11, 8, 0xf); //[DPHY fix]  reg_p0_g_offset_delay_cnt
	_rtd_part_outl(0x1800dc54, 13, 12, 0x0); //[DPHY fix]  reg_p0_g_offset_stable_cnt
	_rtd_part_outl(0x1800dc54, 19, 14, 0x0b); //[DPHY fix]  reg_p0_g_offset_divisor
	_rtd_part_outl(0x1800dc54, 26, 20, 0x28); //[DPHY fix]  reg_p0_g_offset_timeout
	_rtd_part_outl(0x1800dc58, 0, 0, 0x0); //[]  reg_p0_g_offset_en_ope
	_rtd_part_outl(0x1800dc58, 1, 1, 0x0); //[]  reg_p0_g_offset_pc_ope
	_rtd_part_outl(0x1800dc58, 6, 2, 0x10); //[]  reg_p0_g_offset_ini_ope
	_rtd_part_outl(0x1800dc58, 7, 7, 0x0); //[]  reg_p0_g_offset_gray_en_ope
	_rtd_part_outl(0x1800dc58, 8, 8, 0x0); //[]  reg_p0_g_offset_manual_ope
	_rtd_part_outl(0x1800dc58, 9, 9, 0x0); //[]  reg_p0_g_offset_z0_ok_ope
	_rtd_part_outl(0x1800dc58, 10, 10, 0x0); //[]  reg_p0_g_offset_en_opo
	_rtd_part_outl(0x1800dc58, 11, 11, 0x0); //[]  reg_p0_g_offset_pc_opo
	_rtd_part_outl(0x1800dc58, 16, 12, 0x10); //[]  reg_p0_g_offset_ini_opo
	_rtd_part_outl(0x1800dc58, 17, 17, 0x0); //[]  reg_p0_g_offset_gray_en_opo
	_rtd_part_outl(0x1800dc58, 18, 18, 0x0); //[]  reg_p0_g_offset_manual_opo
	_rtd_part_outl(0x1800dc58, 19, 19, 0x0); //[]  reg_p0_g_offset_z0_ok_opo
	_rtd_part_outl(0x1800dc58, 20, 20, 0x0); //[]  reg_p0_g_offset_en_one
	_rtd_part_outl(0x1800dc58, 21, 21, 0x0); //[]  reg_p0_g_offset_pc_one
	_rtd_part_outl(0x1800dc58, 26, 22, 0x10); //[]  reg_p0_g_offset_ini_one
	_rtd_part_outl(0x1800dc58, 27, 27, 0x0); //[]  reg_p0_g_offset_gray_en_one
	_rtd_part_outl(0x1800dc58, 28, 28, 0x0); //[]  reg_p0_g_offset_manual_one
	_rtd_part_outl(0x1800dc58, 29, 29, 0x0); //[]  reg_p0_g_offset_z0_ok_one
	_rtd_part_outl(0x1800dc5c, 0, 0, 0x0); //[DPHY fix]  reg_p0_g_offset_en_ono
	_rtd_part_outl(0x1800dc5c, 6, 2, 0x10); //[DPHY fix]  reg_p0_g_offset_ini_ono
	_rtd_part_outl(0x1800dc5c, 7, 7, 0x0); //[DPHY fix]  reg_p0_g_offset_gray_en_ono
	_rtd_part_outl(0x1800dc5c, 1, 1, 0x0); //[DPHY fix]  reg_p0_g_offset_pc_ono
	_rtd_part_outl(0x1800dc5c, 8, 8, 0x0); //[DPHY fix]  reg_p0_g_offset_manual_ono
	_rtd_part_outl(0x1800dc5c, 9, 9, 0x0); //[DPHY fix]  reg_p0_g_offset_z0_ok_ono
	_rtd_part_outl(0x1800dc5c, 16, 12, 0x10); //[DPHY fix]  reg_p0_g_offset_ini_de
	_rtd_part_outl(0x1800dc5c, 17, 17, 0x0); //[DPHY fix]  reg_p0_g_offset_gray_en_de
	_rtd_part_outl(0x1800dc5c, 26, 22, 0x10); //[DPHY fix]  reg_p0_g_offset_ini_do
	_rtd_part_outl(0x1800dc5c, 27, 27, 0x0); //[DPHY fix]  reg_p0_g_offset_gray_en_do
	_rtd_part_outl(0x1800dc60, 6, 2, 0x10); //[DPHY fix]  reg_p0_g_offset_ini_ee
	_rtd_part_outl(0x1800dc60, 7, 7, 0x0); //[DPHY fix]  reg_p0_g_offset_gray_en_ee
	_rtd_part_outl(0x1800dc60, 16, 12, 0x10); //[DPHY fix]  reg_p0_g_offset_ini_eo
	_rtd_part_outl(0x1800dc60, 17, 17, 0x0); //[DPHY fix]  reg_p0_g_offset_gray_en_eo
	_rtd_part_outl(0x1800dc60, 26, 22, 0x10); //[DPHY fix]  reg_p0_g_offset_ini_eq
	_rtd_part_outl(0x1800dc60, 27, 27, 0x0); //[DPHY fix]  reg_p0_g_offset_gray_en_eq
	_rtd_part_outl(0x1800dc60, 29, 29, 0x0); //[DPHY fix]  reg_p0_g_offset_z0_ok_eq
	_rtd_part_outl(0x1800dc64, 11, 8, 0xf); //[DPHY fix]  reg_p0_r_offset_delay_cnt
	_rtd_part_outl(0x1800dc64, 13, 12, 0x0); //[DPHY fix]  reg_p0_r_offset_stable_cnt
	_rtd_part_outl(0x1800dc64, 19, 14, 0x0b); //[DPHY fix]  reg_p0_r_offset_divisor
	_rtd_part_outl(0x1800dc64, 26, 20, 0x28); //[DPHY fix]  reg_p0_r_offset_timeout
	_rtd_part_outl(0x1800dc68, 0, 0, 0x0); //[]  reg_p0_r_offset_en_ope
	_rtd_part_outl(0x1800dc68, 1, 1, 0x0); //[]  reg_p0_r_offset_pc_ope
	_rtd_part_outl(0x1800dc68, 6, 2, 0x10); //[]  reg_p0_r_offset_ini_ope
	_rtd_part_outl(0x1800dc68, 7, 7, 0x0); //[]  reg_p0_r_offset_gray_en_ope
	_rtd_part_outl(0x1800dc68, 8, 8, 0x0); //[]  reg_p0_r_offset_manual_ope
	_rtd_part_outl(0x1800dc68, 9, 9, 0x0); //[]  reg_p0_r_offset_z0_ok_ope
	_rtd_part_outl(0x1800dc68, 10, 10, 0x0); //[]  reg_p0_r_offset_en_opo
	_rtd_part_outl(0x1800dc68, 11, 11, 0x0); //[]  reg_p0_r_offset_pc_opo
	_rtd_part_outl(0x1800dc68, 16, 12, 0x10); //[]  reg_p0_r_offset_ini_opo
	_rtd_part_outl(0x1800dc68, 17, 17, 0x0); //[]  reg_p0_r_offset_gray_en_opo
	_rtd_part_outl(0x1800dc68, 18, 18, 0x0); //[]  reg_p0_r_offset_manual_opo
	_rtd_part_outl(0x1800dc68, 19, 19, 0x0); //[]  reg_p0_r_offset_z0_ok_opo
	_rtd_part_outl(0x1800dc68, 20, 20, 0x0); //[]  reg_p0_r_offset_en_one
	_rtd_part_outl(0x1800dc68, 21, 21, 0x0); //[]  reg_p0_r_offset_pc_one
	_rtd_part_outl(0x1800dc68, 26, 22, 0x10); //[]  reg_p0_r_offset_ini_one
	_rtd_part_outl(0x1800dc68, 27, 27, 0x0); //[]  reg_p0_r_offset_gray_en_one
	_rtd_part_outl(0x1800dc68, 28, 28, 0x0); //[]  reg_p0_r_offset_manual_one
	_rtd_part_outl(0x1800dc68, 29, 29, 0x0); //[]  reg_p0_r_offset_z0_ok_one
	_rtd_part_outl(0x1800dc6c, 0, 0, 0x0); //[DPHY fix]  reg_p0_r_offset_en_ono
	_rtd_part_outl(0x1800dc6c, 1, 1, 0x0); //[DPHY fix]  reg_p0_r_offset_pc_ono
	_rtd_part_outl(0x1800dc6c, 6, 2, 0x10); //[DPHY fix]  reg_p0_r_offset_ini_ono
	_rtd_part_outl(0x1800dc6c, 7, 7, 0x0); //[DPHY fix]  reg_p0_r_offset_gray_en_ono
	_rtd_part_outl(0x1800dc6c, 8, 8, 0x0); //[DPHY fix]  reg_p0_r_offset_manual_ono
	_rtd_part_outl(0x1800dc6c, 9, 9, 0x0); //[DPHY fix]  reg_p0_r_offset_z0_ok_ono
	_rtd_part_outl(0x1800dc6c, 16, 12, 0x10); //[DPHY fix]  reg_p0_r_offset_ini_de
	_rtd_part_outl(0x1800dc6c, 17, 17, 0x0); //[DPHY fix]  reg_p0_r_offset_gray_en_de
	_rtd_part_outl(0x1800dc6c, 26, 22, 0x10); //[DPHY fix]  reg_p0_r_offset_ini_do
	_rtd_part_outl(0x1800dc6c, 27, 27, 0x0); //[DPHY fix]  reg_p0_r_offset_gray_en_do
	_rtd_part_outl(0x1800dc70, 6, 2, 0x10); //[DPHY fix]  reg_p0_r_offset_ini_ee
	_rtd_part_outl(0x1800dc70, 7, 7, 0x0); //[DPHY fix]  reg_p0_r_offset_gray_en_ee
	_rtd_part_outl(0x1800dc70, 16, 12, 0x10); //[DPHY fix]  reg_p0_r_offset_ini_eo
	_rtd_part_outl(0x1800dc70, 17, 17, 0x0); //[DPHY fix]  reg_p0_r_offset_gray_en_eo
	_rtd_part_outl(0x1800dc70, 26, 22, 0x10); //[DPHY fix]  reg_p0_r_offset_ini_eq
	_rtd_part_outl(0x1800dc70, 27, 27, 0x0); //[DPHY fix]  reg_p0_r_offset_gray_en_eq
	_rtd_part_outl(0x1800dc70, 29, 29, 0x0); //[DPHY fix]  reg_p0_r_offset_z0_ok_eq
	_rtd_part_outl(0x1800dc74, 11, 8, 0xf); //[DPHY fix]  reg_p0_ck_offset_delay_cnt
	_rtd_part_outl(0x1800dc74, 13, 12, 0x0); //[DPHY fix]  reg_p0_ck_offset_stable_cnt
	_rtd_part_outl(0x1800dc74, 19, 14, 0x0b); //[DPHY fix]  reg_p0_ck_offset_divisor
	_rtd_part_outl(0x1800dc74, 26, 20, 0x28); //[DPHY fix]  reg_p0_ck_offset_timeout
	_rtd_part_outl(0x1800dc78, 0, 0, 0x0); //[]  reg_p0_ck_offset_en_ope
	_rtd_part_outl(0x1800dc78, 1, 1, 0x0); //[]  reg_p0_ck_offset_pc_ope
	_rtd_part_outl(0x1800dc78, 6, 2, 0x10); //[]  reg_p0_ck_offset_ini_ope
	_rtd_part_outl(0x1800dc78, 7, 7, 0x0); //[]  reg_p0_ck_offset_gray_en_ope
	_rtd_part_outl(0x1800dc78, 8, 8, 0x0); //[]  reg_p0_ck_offset_manual_ope
	_rtd_part_outl(0x1800dc78, 9, 9, 0x0); //[]  reg_p0_ck_offset_z0_ok_ope
	_rtd_part_outl(0x1800dc78, 10, 10, 0x0); //[]  reg_p0_ck_offset_en_opo
	_rtd_part_outl(0x1800dc78, 11, 11, 0x0); //[]  reg_p0_ck_offset_pc_opo
	_rtd_part_outl(0x1800dc78, 16, 12, 0x10); //[]  reg_p0_ck_offset_ini_opo
	_rtd_part_outl(0x1800dc78, 17, 17, 0x0); //[]  reg_p0_ck_offset_gray_en_opo
	_rtd_part_outl(0x1800dc78, 18, 18, 0x0); //[]  reg_p0_ck_offset_manual_opo
	_rtd_part_outl(0x1800dc78, 19, 19, 0x0); //[]  reg_p0_ck_offset_z0_ok_opo
	_rtd_part_outl(0x1800dc78, 20, 20, 0x0); //[]  reg_p0_ck_offset_en_one
	_rtd_part_outl(0x1800dc78, 21, 21, 0x0); //[]  reg_p0_ck_offset_pc_one
	_rtd_part_outl(0x1800dc78, 26, 22, 0x10); //[]  reg_p0_ck_offset_ini_one
	_rtd_part_outl(0x1800dc78, 27, 27, 0x0); //[]  reg_p0_ck_offset_gray_en_one
	_rtd_part_outl(0x1800dc78, 28, 28, 0x0); //[]  reg_p0_ck_offset_manual_one
	_rtd_part_outl(0x1800dc78, 29, 29, 0x0); //[]  reg_p0_ck_offset_z0_ok_one
	_rtd_part_outl(0x1800dc7c, 0, 0, 0x0); //[DPHY fix]  reg_p0_ck_offset_en_ono
	_rtd_part_outl(0x1800dc7c, 1, 1, 0x0); //[DPHY fix]  reg_p0_ck_offset_pc_ono
	_rtd_part_outl(0x1800dc7c, 6, 2, 0x10); //[DPHY fix]  reg_p0_ck_offset_ini_ono
	_rtd_part_outl(0x1800dc7c, 7, 7, 0x0); //[DPHY fix]  reg_p0_ck_offset_gray_en_ono
	_rtd_part_outl(0x1800dc7c, 8, 8, 0x0); //[DPHY fix]  reg_p0_ck_offset_manual_ono
	_rtd_part_outl(0x1800dc7c, 9, 9, 0x0); //[DPHY fix]  reg_p0_ck_offset_z0_ok_ono
	_rtd_part_outl(0x1800dc7c, 16, 12, 0x10); //[DPHY fix]  reg_p0_ck_offset_ini_de
	_rtd_part_outl(0x1800dc7c, 17, 17, 0x0); //[DPHY fix]  reg_p0_ck_offset_gray_en_de
	_rtd_part_outl(0x1800dc7c, 26, 22, 0x10); //[DPHY fix]  reg_p0_ck_offset_ini_do
	_rtd_part_outl(0x1800dc7c, 27, 27, 0x0); //[DPHY fix]  reg_p0_ck_offset_gray_en_do
	_rtd_part_outl(0x1800dc80, 6, 2, 0x10); //[DPHY fix]  reg_p0_ck_offset_ini_ee
	_rtd_part_outl(0x1800dc80, 7, 7, 0x0); //[DPHY fix]  reg_p0_ck_offset_gray_en_ee
	_rtd_part_outl(0x1800dc80, 16, 12, 0x10); //[DPHY fix]  reg_p0_ck_offset_ini_eo
	_rtd_part_outl(0x1800dc80, 17, 17, 0x0); //[DPHY fix]  reg_p0_ck_offset_gray_en_eo
	_rtd_part_outl(0x1800dc80, 26, 22, 0x10); //[DPHY fix]  reg_p0_ck_offset_ini_eq
	_rtd_part_outl(0x1800dc80, 27, 27, 0x0); //[DPHY fix]  reg_p0_ck_offset_gray_en_eq
	_rtd_part_outl(0x1800dc80, 29, 29, 0x0); //[DPHY fix]  reg_p0_ck_offset_z0_ok_eq
	_rtd_part_outl(0x1800dc88, 2, 2, 0x0); //[DPHY fix]  reg_p0_b_pn_swap_en
	_rtd_part_outl(0x1800dc88, 3, 3, 0x1); //[DPHY fix]  reg_p0_b_dfe_data_en
	_rtd_part_outl(0x1800dc88, 7, 5, 0x3); //[DPHY fix]  reg_p0_b_inbuf_num
	_rtd_part_outl(0x1800dc88, 10, 10, 0x0); //[DPHY fix]  reg_p0_g_pn_swap_en
	_rtd_part_outl(0x1800dc88, 11, 11, 0x1); //[DPHY fix]  reg_p0_g_dfe_data_en
	_rtd_part_outl(0x1800dc88, 15, 13, 0x3); //[DPHY fix]  reg_p0_g_inbuf_num
	_rtd_part_outl(0x1800dc88, 18, 18, 0x0); //[DPHY fix]  reg_p0_r_pn_swap_en
	_rtd_part_outl(0x1800dc88, 19, 19, 0x1); //[DPHY fix]  reg_p0_r_dfe_data_en
	_rtd_part_outl(0x1800dc88, 23, 21, 0x3); //[DPHY fix]  reg_p0_r_fifo_en
	_rtd_part_outl(0x1800dc88, 26, 26, 0x0); //[DPHY fix]  reg_p0_ck_pn_swap_en
	_rtd_part_outl(0x1800dc88, 27, 27, 0x1); //[DPHY fix]  reg_p0_ck_dfe_data_en
	_rtd_part_outl(0x1800dc88, 31, 29, 0x3); //[DPHY fix]  reg_p0_ck_inbuf_num
	_rtd_part_outl(0x1800dc8c, 0, 0, 0x0); //[]  reg_p0_b_001_enable
	_rtd_part_outl(0x1800dc8c, 1, 1, 0x0); //[]  reg_p0_b_101_enable
	_rtd_part_outl(0x1800dc8c, 2, 2, 0x0); //[]  reg_p0_b_en_bec_acc
	_rtd_part_outl(0x1800dc8c, 3, 3, 0x0); //[]  reg_p0_b_en_bec_read
	_rtd_part_outl(0x1800dc8c, 12, 12, 0x0); //[]  reg_p0_g_001_enable
	_rtd_part_outl(0x1800dc8c, 13, 13, 0x0); //[]  reg_p0_g_101_enable
	_rtd_part_outl(0x1800dc8c, 14, 14, 0x0); //[]  reg_p0_g_en_bec_acc
	_rtd_part_outl(0x1800dc8c, 15, 15, 0x0); //[]  reg_p0_g_en_bec_read
	_rtd_part_outl(0x1800dc90, 0, 0, 0x0); //[]  reg_p0_r_001_enable
	_rtd_part_outl(0x1800dc90, 1, 1, 0x0); //[]  reg_p0_r_101_enable
	_rtd_part_outl(0x1800dc90, 2, 2, 0x0); //[]  reg_p0_r_en_bec_acc
	_rtd_part_outl(0x1800dc90, 3, 3, 0x0); //[]  reg_p0_r_en_bec_read
	_rtd_part_outl(0x1800dc90, 12, 12, 0x0); //[]  reg_p0_ck_001_enable
	_rtd_part_outl(0x1800dc90, 13, 13, 0x0); //[]  reg_p0_ck_101_enable
	_rtd_part_outl(0x1800dc90, 14, 14, 0x0); //[]  reg_p0_ck_en_bec_acc
	_rtd_part_outl(0x1800dc90, 15, 15, 0x0); //[]  reg_p0_ck_en_bec_read
	_rtd_part_outl(0x1800dcb8, 0, 0, 0x0); //[DPHY fix]  reg_p0_b_cp2adp_en
	_rtd_part_outl(0x1800dcb8, 1, 1, 0x0); //[DPHY fix]  reg_p0_b_stable_time_mode
	_rtd_part_outl(0x1800dcb8, 2, 2, 0x0); //[DPHY fix]  reg_p0_b_bypass_k_band_mode
	_rtd_part_outl(0x1800dcb8, 18, 18, 0x0); //[DPHY fix]  reg_p0_b_calib_late
	_rtd_part_outl(0x1800dcb8, 19, 19, 0x0); //[DPHY fix]  reg_p0_b_calib_manual
	_rtd_part_outl(0x1800dcb8, 22, 20, 0x2); //[DPHY fix]  reg_p0_b_calib_time
	_rtd_part_outl(0x1800dcb8, 27, 23, 0x0c); //[DPHY fix]  reg_p0_b_adp_time
	_rtd_part_outl(0x1800dcb8, 28, 28, 0x0); //[DPHY fix]  reg_p0_b_adap_eq_off
	_rtd_part_outl(0x1800dcb8, 29, 29, 0x0); //[DPHY fix]  reg_p0_b_cp_en_manual
	_rtd_part_outl(0x1800dcb8, 30, 30, 0x0); //[DPHY fix]  reg_p0_b_adp_en_manual
	_rtd_part_outl(0x1800dcb8, 31, 31, 0x1); //[DPHY fix]  reg_p0_b_auto_mode
	_rtd_part_outl(0x1800dcbc, 31, 28, 0x8); //[DPHY fix] reg_p0_b_cp2adp_time
	_rtd_part_outl(0x1800dcc0, 1, 1, 0x1); //[] reg_p0_b_calib_reset_sel
	_rtd_part_outl(0x1800dcc0, 3, 2, 0x0); //[] reg_p0_b_vc_sel
	_rtd_part_outl(0x1800dcc0, 5, 4, 0x0); //[] reg_p0_b_cdr_c
	_rtd_part_outl(0x1800dcc0, 11, 6, 0x00); //[] reg_p0_b_cdr_r
	_rtd_part_outl(0x1800dcc0, 31, 12, 0xa9878); //[] reg_p0_b_cdr_cp
	_rtd_part_outl(0x1800dcc4, 2, 0, 0x0); //[DPHY fix] reg_p0_b_init_time
	_rtd_part_outl(0x1800dcc4, 7, 3, 0x04); //[DPHY fix] reg_p0_b_cp_time
	_rtd_part_outl(0x1800dcc8, 6, 2, 0x01); //[DPHY fix]  reg_p0_b_timer_6
	_rtd_part_outl(0x1800dcc8, 7, 7, 0x0); //[DPHY fix]  reg_p0_b_timer_5
	_rtd_part_outl(0x1800dcc8, 11, 8, 0x7); //[DPHY fix]  reg_p0_b_vco_fine_init
	_rtd_part_outl(0x1800dcc8, 15, 12, 0x7); //[DPHY fix]  reg_p0_b_vco_coarse_init
	_rtd_part_outl(0x1800dcc8, 16, 16, 0x0); //[DPHY fix]  reg_p0_b_bypass_coarse_k_mode
	_rtd_part_outl(0x1800dcc8, 18, 17, 0x0); //[DPHY fix]  reg_p0_b_vco_fine_time_sel
	_rtd_part_outl(0x1800dcc8, 19, 19, 0x0); //[DPHY fix]  reg_p0_b_xtal_24m_en
	_rtd_part_outl(0x1800dcc8, 23, 20, 0x0); //[DPHY fix]  reg_p0_b_vco_fine_manual
	_rtd_part_outl(0x1800dcc8, 27, 24, 0x0); //[DPHY fix]  reg_p0_b_vco_coarse_manual
	_rtd_part_outl(0x1800dcc8, 28, 28, 0x0); //[DPHY fix]  reg_p0_b_coarse_calib_manual
	_rtd_part_outl(0x1800dcc8, 29, 29, 0x0); //[DPHY fix]  reg_p0_b_acdr_ckfld_en
	_rtd_part_outl(0x1800dcc8, 30, 30, 0x0); //[DPHY fix]  reg_p0_b_pll_to_acdr_manual_en
	_rtd_part_outl(0x1800dcc8, 31, 31, 0x0); //[DPHY fix]  reg_p0_b_pll_to_acdr_rdy_manual
	_rtd_part_outl(0x1800dccc, 4, 0, 0x01); //[]  reg_p0_b_timer_4
	_rtd_part_outl(0x1800dccc, 7, 5, 0x0); //[]  reg_p0_b_vc_chg_time
	_rtd_part_outl(0x1800dccc, 16, 16, 0x1); //[]  reg_p0_b_old_mode
	_rtd_part_outl(0x1800dccc, 21, 17, 0x10); //[]  reg_p0_b_slope_band
	_rtd_part_outl(0x1800dccc, 22, 22, 0x0); //[]  reg_p0_b_slope_manual
	_rtd_part_outl(0x1800dccc, 23, 23, 0x0); //[]  reg_p0_b_rxidle_bypass
	_rtd_part_outl(0x1800dccc, 24, 24, 0x0); //[]  reg_p0_b_pfd_bypass
	_rtd_part_outl(0x1800dccc, 29, 25, 0x00); //[]  reg_p0_b_pfd_time
	_rtd_part_outl(0x1800dccc, 30, 30, 0x0); //[]  reg_p0_b_pfd_en_manual
	_rtd_part_outl(0x1800dccc, 31, 31, 0x0); //[]  reg_p0_b_start_en_manual
	_rtd_part_outl(0x1800dcd0, 9, 5, 0x08); //[]  reg_p0_b_cp_time_2
	_rtd_part_outl(0x1800dcd8, 0, 0, 0x0); //[DPHY fix]  reg_p0_g_cp2adp_en
	_rtd_part_outl(0x1800dcd8, 1, 1, 0x0); //[DPHY fix]  reg_p0_g_stable_time_mode
	_rtd_part_outl(0x1800dcd8, 2, 2, 0x0); //[DPHY fix]  reg_p0_g_bypass_k_band_mode
	_rtd_part_outl(0x1800dcd8, 18, 18, 0x0); //[DPHY fix]  reg_p0_g_calib_late
	_rtd_part_outl(0x1800dcd8, 19, 19, 0x0); //[DPHY fix]  reg_p0_g_calib_manual
	_rtd_part_outl(0x1800dcd8, 22, 20, 0x2); //[DPHY fix]  reg_p0_g_calib_time
	_rtd_part_outl(0x1800dcd8, 27, 23, 0x0c); //[DPHY fix]  reg_p0_g_adp_time
	_rtd_part_outl(0x1800dcd8, 28, 28, 0x0); //[DPHY fix]  reg_p0_g_adap_eq_off
	_rtd_part_outl(0x1800dcd8, 29, 29, 0x0); //[DPHY fix]  reg_p0_g_cp_en_manual
	_rtd_part_outl(0x1800dcd8, 30, 30, 0x0); //[DPHY fix]  reg_p0_g_adp_en_manual
	_rtd_part_outl(0x1800dcd8, 31, 31, 0x1); //[DPHY fix]  reg_p0_g_auto_mode
	_rtd_part_outl(0x1800dcdc, 31, 28, 0x8); //[DPHY fix] reg_p0_g_cp2adp_time
	_rtd_part_outl(0x1800dce0, 1, 1, 0x1); //[] reg_p0_g_calib_reset_sel
	_rtd_part_outl(0x1800dce0, 3, 2, 0x0); //[] reg_p0_g_vc_sel
	_rtd_part_outl(0x1800dce0, 5, 4, 0x0); //[] reg_p0_g_cdr_c
	_rtd_part_outl(0x1800dce0, 11, 6, 0x00); //[] reg_p0_g_cdr_r
	_rtd_part_outl(0x1800dce0, 31, 12, 0xa9878); //[] reg_p0_g_cdr_cp
	_rtd_part_outl(0x1800dce4, 2, 0, 0x0); //[DPHY fix] reg_p0_g_init_time
	_rtd_part_outl(0x1800dce4, 7, 3, 0x04); //[DPHY fix] reg_p0_g_cp_time
	_rtd_part_outl(0x1800dce8, 6, 2, 0x01); //[DPHY fix]  reg_p0_g_timer_6
	_rtd_part_outl(0x1800dce8, 7, 7, 0x0); //[DPHY fix]  reg_p0_g_timer_5
	_rtd_part_outl(0x1800dce8, 11, 8, 0x7); //[DPHY fix]  reg_p0_g_vco_fine_init
	_rtd_part_outl(0x1800dce8, 15, 12, 0x7); //[DPHY fix]  reg_p0_g_vco_coarse_init
	_rtd_part_outl(0x1800dce8, 16, 16, 0x0); //[DPHY fix]  reg_p0_g_bypass_coarse_k_mode
	_rtd_part_outl(0x1800dce8, 18, 17, 0x0); //[DPHY fix]  reg_p0_g_vco_fine_time_sel
	_rtd_part_outl(0x1800dce8, 19, 19, 0x0); //[DPHY fix]  reg_p0_g_xtal_24m_en
	_rtd_part_outl(0x1800dce8, 23, 20, 0x0); //[DPHY fix]  reg_p0_g_vco_fine_manual
	_rtd_part_outl(0x1800dce8, 27, 24, 0x0); //[DPHY fix]  reg_p0_g_vco_coarse_manual
	_rtd_part_outl(0x1800dce8, 28, 28, 0x0); //[DPHY fix]  reg_p0_g_coarse_calib_manual
	_rtd_part_outl(0x1800dce8, 29, 29, 0x0); //[DPHY fix]  reg_p0_g_acdr_ckfld_en
	_rtd_part_outl(0x1800dce8, 30, 30, 0x0); //[DPHY fix]  reg_p0_g_pll_to_acdr_manual_en
	_rtd_part_outl(0x1800dce8, 31, 31, 0x0); //[DPHY fix]  reg_p0_g_pll_to_acdr_rdy_manual
	_rtd_part_outl(0x1800dcec, 4, 0, 0x01); //[]  reg_p0_g_timer_4
	_rtd_part_outl(0x1800dcec, 7, 5, 0x0); //[]  reg_p0_g_vc_chg_time
	_rtd_part_outl(0x1800dcec, 16, 16, 0x1); //[]  reg_p0_g_old_mode
	_rtd_part_outl(0x1800dcec, 21, 17, 0x10); //[]  reg_p0_g_slope_band
	_rtd_part_outl(0x1800dcec, 22, 22, 0x0); //[]  reg_p0_g_slope_manual
	_rtd_part_outl(0x1800dcec, 23, 23, 0x0); //[]  reg_p0_g_rxidle_bypass
	_rtd_part_outl(0x1800dcec, 24, 24, 0x0); //[]  reg_p0_g_pfd_bypass
	_rtd_part_outl(0x1800dcec, 29, 25, 0x00); //[]  reg_p0_g_pfd_time
	_rtd_part_outl(0x1800dcec, 30, 30, 0x0); //[]  reg_p0_g_pfd_en_manual
	_rtd_part_outl(0x1800dcec, 31, 31, 0x0); //[]  reg_p0_g_start_en_manual
	_rtd_part_outl(0x1800dcf0, 9, 5, 0x08); //[]  reg_p0_g_cp_time_2
	_rtd_part_outl(0x1800dcf4, 0, 0, 0x0); //[DPHY fix]  reg_p0_r_cp2adp_en
	_rtd_part_outl(0x1800dcf4, 1, 1, 0x0); //[DPHY fix]  reg_p0_r_stable_time_mode
	_rtd_part_outl(0x1800dcf4, 2, 2, 0x0); //[DPHY fix]  reg_p0_r_bypass_k_band_mode
	_rtd_part_outl(0x1800dcf4, 18, 18, 0x0); //[DPHY fix]  reg_p0_r_calib_late
	_rtd_part_outl(0x1800dcf4, 19, 19, 0x0); //[DPHY fix]  reg_p0_r_calib_manual
	_rtd_part_outl(0x1800dcf4, 22, 20, 0x2); //[DPHY fix]  reg_p0_r_calib_time
	_rtd_part_outl(0x1800dcf4, 27, 23, 0x0c); //[DPHY fix]  reg_p0_r_adp_time
	_rtd_part_outl(0x1800dcf4, 28, 28, 0x0); //[DPHY fix]  reg_p0_r_adap_eq_off
	_rtd_part_outl(0x1800dcf4, 29, 29, 0x0); //[DPHY fix]  reg_p0_r_cp_en_manual
	_rtd_part_outl(0x1800dcf4, 30, 30, 0x0); //[DPHY fix]  reg_p0_r_adp_en_manual
	_rtd_part_outl(0x1800dcf4, 31, 31, 0x1); //[DPHY fix]  reg_p0_r_auto_mode
	_rtd_part_outl(0x1800dcf8, 31, 28, 0x8); //[DPHY fix] reg_p0_r_cp2adp_time
	_rtd_part_outl(0x1800dcfc, 1, 1, 0x1); //[] reg_p0_r_calib_reset_sel
	_rtd_part_outl(0x1800dcfc, 3, 2, 0x0); //[] reg_p0_r_vc_sel
	_rtd_part_outl(0x1800dcfc, 5, 4, 0x0); //[] reg_p0_r_cdr_c
	_rtd_part_outl(0x1800dcfc, 11, 6, 0x00); //[] reg_p0_r_cdr_r
	_rtd_part_outl(0x1800dcfc, 31, 12, 0xa9878); //[] reg_p0_r_cdr_cp
	_rtd_part_outl(0x1800dd00, 2, 0, 0x0); //[DPHY fix] reg_p0_r_init_time
	_rtd_part_outl(0x1800dd00, 7, 3, 0x04); //[DPHY fix] reg_p0_r_cp_time
	_rtd_part_outl(0x1800dd04, 6, 2, 0x01); //[DPHY fix]  reg_p0_r_timer_6
	_rtd_part_outl(0x1800dd04, 7, 7, 0x0); //[DPHY fix]  reg_p0_r_timer_5
	_rtd_part_outl(0x1800dd04, 11, 8, 0x7); //[DPHY fix]  reg_p0_r_vco_fine_init
	_rtd_part_outl(0x1800dd04, 15, 12, 0x7); //[DPHY fix]  reg_p0_r_vco_coarse_init
	_rtd_part_outl(0x1800dd04, 16, 16, 0x0); //[DPHY fix]  reg_p0_r_bypass_coarse_k_mode
	_rtd_part_outl(0x1800dd04, 18, 17, 0x0); //[DPHY fix]  reg_p0_r_vco_fine_time_sel
	_rtd_part_outl(0x1800dd04, 19, 19, 0x0); //[DPHY fix]  reg_p0_r_xtal_24m_en
	_rtd_part_outl(0x1800dd04, 23, 20, 0x0); //[DPHY fix]  reg_p0_r_vco_fine_manual
	_rtd_part_outl(0x1800dd04, 27, 24, 0x0); //[DPHY fix]  reg_p0_r_vco_coarse_manual
	_rtd_part_outl(0x1800dd04, 28, 28, 0x0); //[DPHY fix]  reg_p0_r_coarse_calib_manual
	_rtd_part_outl(0x1800dd04, 29, 29, 0x0); //[DPHY fix]  reg_p0_r_acdr_ckfld_en
	_rtd_part_outl(0x1800dd04, 30, 30, 0x0); //[DPHY fix]  reg_p0_r_pll_to_acdr_manual_en
	_rtd_part_outl(0x1800dd04, 31, 31, 0x0); //[DPHY fix]  reg_p0_r_pll_to_acdr_rdy_manual
	_rtd_part_outl(0x1800dd08, 4, 0, 0x01); //[]  reg_p0_r_timer_4
	_rtd_part_outl(0x1800dd08, 7, 5, 0x0); //[]  reg_p0_r_vc_chg_time
	_rtd_part_outl(0x1800dd08, 16, 16, 0x1); //[]  reg_p0_r_old_mode
	_rtd_part_outl(0x1800dd08, 21, 17, 0x10); //[]  reg_p0_r_slope_band
	_rtd_part_outl(0x1800dd08, 22, 22, 0x0); //[]  reg_p0_r_slope_manual
	_rtd_part_outl(0x1800dd08, 23, 23, 0x0); //[]  reg_p0_r_rxidle_bypass
	_rtd_part_outl(0x1800dd08, 24, 24, 0x0); //[]  reg_p0_r_pfd_bypass
	_rtd_part_outl(0x1800dd08, 29, 25, 0x00); //[]  reg_p0_r_pfd_time
	_rtd_part_outl(0x1800dd08, 30, 30, 0x0); //[]  reg_p0_r_pfd_en_manual
	_rtd_part_outl(0x1800dd08, 31, 31, 0x0); //[]  reg_p0_r_start_en_manual
	_rtd_part_outl(0x1800dd0c, 9, 5, 0x08); //[]  reg_p0_r_cp_time_2
	_rtd_part_outl(0x1800dd10, 0, 0, 0x0); //[DPHY fix]  reg_p0_ck_cp2adp_en
	_rtd_part_outl(0x1800dd10, 1, 1, 0x0); //[DPHY fix]  reg_p0_ck_stable_time_mode
	_rtd_part_outl(0x1800dd10, 2, 2, 0x0); //[DPHY fix]  reg_p0_ck_bypass_k_band_mode
	_rtd_part_outl(0x1800dd10, 18, 18, 0x0); //[DPHY fix]  reg_p0_ck_calib_late
	_rtd_part_outl(0x1800dd10, 19, 19, 0x0); //[DPHY fix]  reg_p0_ck_calib_manual
	_rtd_part_outl(0x1800dd10, 22, 20, 0x2); //[DPHY fix]  reg_p0_ck_calib_time
	_rtd_part_outl(0x1800dd10, 27, 23, 0x0c); //[DPHY fix]  reg_p0_ck_adp_time
	_rtd_part_outl(0x1800dd10, 28, 28, 0x0); //[DPHY fix]  reg_p0_ck_adap_eq_off
	_rtd_part_outl(0x1800dd10, 29, 29, 0x0); //[DPHY fix]  reg_p0_ck_cp_en_manual
	_rtd_part_outl(0x1800dd10, 30, 30, 0x0); //[DPHY fix]  reg_p0_ck_adp_en_manual
	_rtd_part_outl(0x1800dd10, 31, 31, 0x1); //[DPHY fix]  reg_p0_ck_auto_mode
	_rtd_part_outl(0x1800dd14, 31, 28, 0x8); //[DPHY fix] reg_p0_ck_cp2adp_time
	_rtd_part_outl(0x1800dd18, 1, 1, 0x1); //[] reg_p0_ck_calib_reset_sel
	_rtd_part_outl(0x1800dd18, 3, 2, 0x0); //[] reg_p0_ck_vc_sel
	_rtd_part_outl(0x1800dd18, 5, 4, 0x0); //[] reg_p0_ck_cdr_c
	_rtd_part_outl(0x1800dd18, 11, 6, 0x00); //[] reg_p0_ck_cdr_r
	_rtd_part_outl(0x1800dd18, 31, 12, 0xa9878); //[] reg_p0_ck_cdr_cp
	_rtd_part_outl(0x1800dd1c, 2, 0, 0x0); //[DPHY fix] reg_p0_ck_init_time
	_rtd_part_outl(0x1800dd1c, 7, 3, 0x04); //[DPHY fix] reg_p0_ck_cp_time
	_rtd_part_outl(0x1800dd20, 6, 2, 0x01); //[DPHY fix]  reg_p0_ck_timer_6
	_rtd_part_outl(0x1800dd20, 7, 7, 0x0); //[DPHY fix]  reg_p0_ck_timer_5
	_rtd_part_outl(0x1800dd20, 11, 8, 0x7); //[DPHY fix]  reg_p0_ck_vco_fine_init
	_rtd_part_outl(0x1800dd20, 15, 12, 0x7); //[DPHY fix]  reg_p0_ck_vco_coarse_init
	_rtd_part_outl(0x1800dd20, 16, 16, 0x0); //[DPHY fix]  reg_p0_ck_bypass_coarse_k_mode
	_rtd_part_outl(0x1800dd20, 18, 17, 0x0); //[DPHY fix]  reg_p0_ck_vco_fine_time_sel
	_rtd_part_outl(0x1800dd20, 19, 19, 0x0); //[DPHY fix]  reg_p0_ck_xtal_24m_en
	_rtd_part_outl(0x1800dd20, 23, 20, 0x0); //[DPHY fix]  reg_p0_ck_vco_fine_manual
	_rtd_part_outl(0x1800dd20, 27, 24, 0x8); //[DPHY fix]  reg_p0_ck_vco_coarse_manual
	_rtd_part_outl(0x1800dd20, 28, 28, 0x0); //[DPHY fix]  reg_p0_ck_coarse_calib_manual
	_rtd_part_outl(0x1800dd20, 29, 29, 0x0); //[DPHY fix]  reg_p0_ck_acdr_ckfld_en
	_rtd_part_outl(0x1800dd20, 30, 30, 0x0); //[DPHY fix]  reg_p0_ck_pll_to_acdr_manual_en
	_rtd_part_outl(0x1800dd20, 31, 31, 0x0); //[DPHY fix]  reg_p0_ck_pll_to_acdr_rdy_manual
	_rtd_part_outl(0x1800dd24, 4, 0, 0x01); //[]  reg_p0_ck_timer_4
	_rtd_part_outl(0x1800dd24, 7, 5, 0x0); //[]  reg_p0_ck_vc_chg_time
	_rtd_part_outl(0x1800dd24, 16, 16, 0x1); //[]  reg_p0_ck_old_mode
	_rtd_part_outl(0x1800dd24, 21, 17, 0x10); //[]  reg_p0_ck_slope_band
	_rtd_part_outl(0x1800dd24, 22, 22, 0x0); //[]  reg_p0_ck_slope_manual
	_rtd_part_outl(0x1800dd24, 23, 23, 0x0); //[]  reg_p0_ck_rxidle_bypass
	_rtd_part_outl(0x1800dd24, 24, 24, 0x0); //[]  reg_p0_ck_pfd_bypass
	_rtd_part_outl(0x1800dd24, 29, 25, 0x00); //[]  reg_p0_ck_pfd_time
	_rtd_part_outl(0x1800dd24, 30, 30, 0x0); //[]  reg_p0_ck_pfd_en_manual
	_rtd_part_outl(0x1800dd24, 31, 31, 0x0); //[]  reg_p0_ck_start_en_manual
	_rtd_part_outl(0x1800dd28, 9, 5, 0x08); //[]  reg_p0_ck_cp_time_2
	//MOD
	_rtd_part_outl(0x1800dd2c, 0, 0, 0x1); //[]  reg_p0_ck_md_rstb
	_rtd_part_outl(0x1800dd2c, 1, 1, 0x1); //[]  reg_p0_ck_md_auto
	_rtd_part_outl(0x1800dd2c, 3, 2, 0x0); //[]  reg_p0_ck_md_sel
	_rtd_part_outl(0x1800dd2c, 7, 4, 0x0); //[]  reg_p0_ck_md_adj
	_rtd_part_outl(0x1800dd2c, 13, 8, 0x03); //[]  reg_p0_ck_md_threshold
	_rtd_part_outl(0x1800dd2c, 20, 14, 0x00); //[]  reg_p0_ck_md_debounce
	_rtd_part_outl(0x1800dd2c, 25, 21, 0x00); //[]  reg_p0_ck_error_limit
	_rtd_part_outl(0x1800dd2c, 29, 26, 0x0); //[]  reg_p0_ck_md_reserved
	_rtd_part_outl(0x1800dd30, 2, 0, 0x0); //[]  reg_p0_mod_sel
	//PLLCDR
	_rtd_part_outl(0x1800dd34, 0, 0, 0x0); //[]  reg_p0_b_pllcdr_manual_enable
	_rtd_part_outl(0x1800dd34, 1, 1, 0x0); //[]  reg_p0_g_pllcdr_manual_enable
	_rtd_part_outl(0x1800dd34, 2, 2, 0x0); //[]  reg_p0_r_pllcdr_manual_enable
	_rtd_part_outl(0x1800dd34, 3, 3, 0x0); //[]  reg_p0_ck_pllcdr_manual_enable
	_rtd_part_outl(0x1800dd34, 4, 4, 0x0); //[]  reg_p0_b_pllcdr_vcoband_manual_enable
	_rtd_part_outl(0x1800dd34, 5, 5, 0x0); //[]  reg_p0_g_pllcdr_vcoband_manual_enable
	_rtd_part_outl(0x1800dd34, 6, 6, 0x0); //[]  reg_p0_r_pllcdr_vcoband_manual_enable
	_rtd_part_outl(0x1800dd34, 7, 7, 0x0); //[]  reg_p0_ck_pllcdr_vcoband_manual_enable
	_rtd_part_outl(0x1800dd34, 19, 16, 0x0); //[]  reg_p0_b_acdr_icp_sel_manual
	_rtd_part_outl(0x1800dd34, 23, 20, 0x0); //[]  reg_p0_g_acdr_icp_sel_manual
	_rtd_part_outl(0x1800dd34, 27, 24, 0x0); //[]  reg_p0_r_acdr_icp_sel_manual
	_rtd_part_outl(0x1800dd34, 31, 28, 0x3); //[]  reg_p0_ck_acdr_icp_sel_manual
	_rtd_part_outl(0x1800dd38, 3, 0, 0x0); //[]  reg_p0_b_acdr_vco_coarse_i_manual
	_rtd_part_outl(0x1800dd38, 7, 4, 0x0); //[]  reg_p0_g_acdr_vco_coarse_i_manual
	_rtd_part_outl(0x1800dd38, 11, 8, 0x0); //[]  reg_p0_r_acdr_vco_coarse_i_manual
	_rtd_part_outl(0x1800dd38, 15, 12, 0x0); //[]  reg_p0_ck_acdr_vco_coarse_i_manual
	_rtd_part_outl(0x1800dd38, 19, 16, 0x0); //[]  reg_p0_b_acdr_vco_fine_i_manual
	_rtd_part_outl(0x1800dd38, 23, 20, 0x0); //[]  reg_p0_g_acdr_vco_fine_i_manual
	_rtd_part_outl(0x1800dd38, 27, 24, 0x0); //[]  reg_p0_r_acdr_vco_fine_i_manual
	_rtd_part_outl(0x1800dd38, 31, 28, 0x0); //[]  reg_p0_ck_acdr_vco_fine_i_manual
	_rtd_part_outl(0x1800dd6c, 0, 0, 0x0); //[DPHY fix]  reg_p0_acdr_en
	_rtd_part_outl(0x1800dd6c, 2, 2, 0x0); //[DPHY fix]  reg_p0_pow_on_manual
	_rtd_part_outl(0x1800dd6c, 3, 3, 0x0); //[DPHY fix]  reg_p0_pow_on_manual_aphy_en
	_rtd_part_outl(0x1800dd6c, 4, 4, 0x0); //[DPHY fix]  reg_p0_pow_on_manual_b_en
	_rtd_part_outl(0x1800dd6c, 5, 5, 0x0); //[DPHY fix]  reg_p0_pow_on_manual_g_en
	_rtd_part_outl(0x1800dd6c, 6, 6, 0x0); //[DPHY fix]  reg_p0_pow_on_manual_r_en
	_rtd_part_outl(0x1800dd6c, 7, 7, 0x0); //[DPHY fix]  reg_p0_pow_on_manual_ck_en
	_rtd_part_outl(0x1800dd6c, 8, 8, 0x0); //[DPHY fix]  reg_p0_pow_save_bypass_b
	_rtd_part_outl(0x1800dd6c, 9, 9, 0x0); //[DPHY fix]  reg_p0_pow_save_bypass_g
	_rtd_part_outl(0x1800dd6c, 10, 10, 0x0); //[DPHY fix]  reg_p0_pow_save_bypass_r
	_rtd_part_outl(0x1800dd6c, 11, 11, 0x0); //[DPHY fix]  reg_p0_pow_save_bypass_ck
	_rtd_part_outl(0x1800dd6c, 12, 12, 0x0); //[DPHY fix]  reg_p0_pow_save_xtal_24m_enable
	_rtd_part_outl(0x1800dd6c, 14, 13, 0x3); //[DPHY fix]  reg_p0_pow_save_rst_dly_sel
	_rtd_part_outl(0x1800dd70, 17, 16, 0x0); //[DPHY fix]  reg_tap1_mask
	_rtd_part_outl(0x1800dd7c, 31, 31, 0x0); //[DPHY fix]  reg_port_out_sel
	_rtd_part_outl(0x1800dd80, 7, 0, 0x00); //[]  reg_p0_dig_reserved_3
	_rtd_part_outl(0x1800dd80, 15, 8, 0x00); //[]  reg_p0_dig_reserved_2
	_rtd_part_outl(0x1800dd80, 23, 16, 0x00); //[]  reg_p0_dig_reserved_1
	_rtd_part_outl(0x1800dd80, 31, 24, 0x00); //[]  reg_p0_dig_reserved_0
}

void DPHY_Para_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing)
{
	// DPHY para start
	_rtd_part_outl(0x1800dc04, 5, 0, DPHY_Para_tmds_1[tmds_timing].b_kp_2); //[]  reg_p0_b_kp
	_rtd_part_outl(0x1800dc04, 11, 6, DPHY_Para_tmds_1[tmds_timing].g_kp_3); //[]  reg_p0_g_kp
	_rtd_part_outl(0x1800dc04, 17, 12, DPHY_Para_tmds_1[tmds_timing].r_kp_4); //[]  reg_p0_r_kp
	_rtd_part_outl(0x1800dc04, 23, 18, DPHY_Para_tmds_1[tmds_timing].ck_kp_5); //[]  reg_p0_ck_kp
	_rtd_part_outl(0x1800dc08, 5, 0, DPHY_Para_tmds_1[tmds_timing].b_bbw_kp_6); //[]  reg_p0_b_bbw_kp
	_rtd_part_outl(0x1800dc08, 11, 6, DPHY_Para_tmds_1[tmds_timing].g_bbw_kp_7); //[]  reg_p0_g_bbw_kp
	_rtd_part_outl(0x1800dc08, 17, 12, DPHY_Para_tmds_1[tmds_timing].r_bbw_kp_8); //[]  reg_p0_r_bbw_kp
	_rtd_part_outl(0x1800dc08, 23, 18, DPHY_Para_tmds_1[tmds_timing].ck_bbw_kp_9); //[]  reg_p0_ck_bbw_kp
	_rtd_part_outl(0x1800dc0c, 2, 0, DPHY_Para_tmds_1[tmds_timing].b_ki_10); //[]  reg_p0_b_ki
	_rtd_part_outl(0x1800dc0c, 5, 3, DPHY_Para_tmds_1[tmds_timing].g_ki_11); //[]  reg_p0_g_ki
	_rtd_part_outl(0x1800dc0c, 8, 6, DPHY_Para_tmds_1[tmds_timing].r_ki_12); //[]  reg_p0_r_ki
	_rtd_part_outl(0x1800dc0c, 11, 9, DPHY_Para_tmds_1[tmds_timing].ck_ki_13); //[]  reg_p0_ck_ki
	_rtd_part_outl(0x1800dc0c, 18, 16, 0x0); //[]  reg_p0_b_bbw_ki
	_rtd_part_outl(0x1800dc0c, 21, 19, 0x0); //[]  reg_p0_g_bbw_ki
	_rtd_part_outl(0x1800dc0c, 24, 22, 0x0); //[]  reg_p0_r_bbw_ki
	_rtd_part_outl(0x1800dc0c, 27, 25, 0x0); //[]  reg_p0_ck_bbw_ki
	_rtd_part_outl(0x1800dc0c, 12, 12, 0x0); //[]  reg_p0_b_st_mode
	_rtd_part_outl(0x1800dc0c, 13, 13, 0x0); //[]  reg_p0_g_st_mode
	_rtd_part_outl(0x1800dc0c, 14, 14, 0x0); //[]  reg_p0_r_st_mode
	_rtd_part_outl(0x1800dc0c, 15, 15, 0x0); //[]  reg_p0_ck_st_mode
	_rtd_part_outl(0x1800dc0c, 28, 28, 0x0); //[]  reg_p0_b_bbw_st_mode
	_rtd_part_outl(0x1800dc0c, 29, 29, 0x0); //[]  reg_p0_g_bbw_st_mode
	_rtd_part_outl(0x1800dc0c, 30, 30, 0x0); //[]  reg_p0_r_bbw_st_mode
	_rtd_part_outl(0x1800dc0c, 31, 31, 0x0); //[]  reg_p0_ck_bbw_st_mode
	_rtd_part_outl(0x1800dc00, 17, 16, DPHY_Para_tmds_1[tmds_timing].b_rate_sel_26); //[DPHY para]  reg_p0_b_rate_sel
	_rtd_part_outl(0x1800dc00, 19, 18, DPHY_Para_tmds_1[tmds_timing].g_rate_sel_27); //[DPHY para]  reg_p0_g_rate_sel
	_rtd_part_outl(0x1800dc00, 21, 20, DPHY_Para_tmds_1[tmds_timing].r_rate_sel_28); //[DPHY para]  reg_p0_r_rate_sel
	_rtd_part_outl(0x1800dc88, 0, 0, 0x0); //[DPHY para]  reg_p0_b_10b18b_sel
	_rtd_part_outl(0x1800dc88, 1, 1, DPHY_Para_tmds_1[tmds_timing].b_clk_div2_en_30); //[DPHY para]  reg_p0_b_clk_div2_en
	_rtd_part_outl(0x1800dc88, 8, 8, 0x0); //[DPHY para]  reg_p0_g_10b18b_sel
	_rtd_part_outl(0x1800dc88, 9, 9, DPHY_Para_tmds_1[tmds_timing].g_clk_div2_en_32); //[DPHY para]  reg_p0_g_clk_div2_en
	_rtd_part_outl(0x1800dc88, 16, 16, 0x0); //[DPHY para]  reg_p0_r_10b18b_sel
	_rtd_part_outl(0x1800dc88, 17, 17, DPHY_Para_tmds_1[tmds_timing].r_clk_div2_en_34); //[DPHY para]  reg_p0_r_clk_div2_en
	_rtd_part_outl(0x1800dc88, 24, 24, 0x0); //[DPHY para]  reg_p0_ck_10b18b_sel
	_rtd_part_outl(0x1800dd60, 7, 0, DPHY_Para_tmds_1[tmds_timing].ck_acdr_pll_config_1_36); //[]  reg_p0_ck_acdr_pll_config_1
	_rtd_part_outl(0x1800dd60, 15, 8, DPHY_Para_tmds_1[tmds_timing].ck_acdr_pll_config_2_37); //[]  reg_p0_ck_acdr_pll_config_2
	_rtd_part_outl(0x1800dd60, 23, 16, DPHY_Para_tmds_1[tmds_timing].ck_acdr_pll_config_3_38); //[]  reg_p0_ck_acdr_pll_config_3
	_rtd_part_outl(0x1800dd60, 31, 24, 0x00); //[]  reg_p0_ck_acdr_pll_config_4
	_rtd_part_outl(0x1800dd64, 7, 0, DPHY_Para_tmds_1[tmds_timing].ck_acdr_cdr_config_1_40); //[]  reg_p0_ck_acdr_cdr_config_1
	_rtd_part_outl(0x1800dd64, 15, 8, DPHY_Para_tmds_1[tmds_timing].ck_acdr_cdr_config_2_41); //[]  reg_p0_ck_acdr_cdr_config_2
	_rtd_part_outl(0x1800dd64, 23, 16, DPHY_Para_tmds_1[tmds_timing].ck_acdr_cdr_config_3_42); //[]  reg_p0_ck_acdr_cdr_config_3
	_rtd_part_outl(0x1800dd64, 31, 24, 0x00); //[]  reg_p0_ck_acdr_cdr_config_4
	_rtd_part_outl(0x1800dd68, 7, 0, 0x00); //[]  reg_p0_ck_acdr_manual_config_1
	_rtd_part_outl(0x1800dd68, 15, 8, 0x00); //[]  reg_p0_ck_acdr_manual_config_2
	_rtd_part_outl(0x1800dd68, 23, 16, 0x00); //[]  reg_p0_ck_acdr_manual_config_3
	_rtd_part_outl(0x1800dd68, 31, 24, 0x00); //[]  reg_p0_ck_acdr_manual_config_4
}

void APHY_Fix_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing)
{
	//APHY fix start
	//_rtd_part_outl(0x1800da04, 4, 0, 0x0a); //[]  REG_Z0_ADJR_0
	_rtd_part_outl(0x1800da08, 0, 0, 0x1); //[]  REG_Z0_Z0POW_B
	_rtd_part_outl(0x1800da08, 4, 4, 0x1); //[]  REG_Z0_Z0POW_CK
	_rtd_part_outl(0x1800da08, 8, 8, 0x1); //[]  REG_Z0_Z0POW_G
	_rtd_part_outl(0x1800da08, 12, 12, 0x1); //[]  REG_Z0_Z0POW_R
	_rtd_part_outl(0x1800da08, 20, 20, 0x1); //[]  REG_Z0_Z300POW
	_rtd_part_outl(0x1800da08, 24, 24, 0x0); //[]  REG_Z0_Z300_SEL
	_rtd_part_outl(0x1800da10, 0, 0, 0x0); //[]  Z0_P_OFF
	_rtd_part_outl(0x1800da10, 1, 1, 0x0); //[]  Z0_N_OFF
	_rtd_part_outl(0x1800da10, 3, 2, 0x3); //[]  Z0_RFIX_SEL
	_rtd_part_outl(0x1800da10, 7, 4, 0x0); //[]  dummy.
	_rtd_part_outl(0x1800da10, 8, 8, 0x0); //[]  fast sw sel
	_rtd_part_outl(0x1800da10, 9, 9, 0x0); //[]  fast sw clock div enable
	_rtd_part_outl(0x1800da10, 12, 10, 0x0); //[]  fast sw clock div select (delay time=
	_rtd_part_outl(0x1800da10, 13, 13, 0x0); //[]  Enable AVDD10 voltage probe.
	_rtd_part_outl(0x1800da10, 15, 14, 0x0); //[]  IO overvoltage detect bit select
	_rtd_part_outl(0x1800da10, 16, 16, 0x1); //[]  CK_TX EN
	_rtd_part_outl(0x1800da10, 17, 17, 0x0); //[]  CK_MD selection.
	_rtd_part_outl(0x1800da10, 19, 18, 0x3); //[]  1.8V CK-detect hysteresis window select.
	_rtd_part_outl(0x1800da10, 20, 20, 0x0); //[]  CK-PAD to CK-detect switch enable.
	_rtd_part_outl(0x1800da10, 21, 21, 0x0); //[]  Standby mode CK-detect & AC-couple
	_rtd_part_outl(0x1800da10, 22, 22, 0x0); //[]  dummy
	_rtd_part_outl(0x1800da10, 23, 23, 0x0); //[]  div32 rstb (CK-detect use).
	_rtd_part_outl(0x1800da10, 27, 24, 0x0); //[]  test voltage/test current/test clock select  @[6:5]
	_rtd_part_outl(0x1800da10, 28, 28, 0x0); //[]  dummy
	_rtd_part_outl(0x1800da10, 30, 29, 0x0); //[]  tst pad mode sel
	_rtd_part_outl(0x1800da10, 31, 31, 0x0); //[]  dummy
	_rtd_part_outl(0x1800da14, 7, 0, 0x10); //[]  REG_TOP_IN_5 dummy
	_rtd_part_outl(0x1800da18, 0, 0, 0x1); //[APHY fix]  REG_HDMIRX_BIAS_EN bandgap reference power
	_rtd_part_outl(0x1800da18, 10, 8, 0x0); //[APHY fix]  REG_BG_RBGLOOP2 dummy
	_rtd_part_outl(0x1800da18, 13, 11, 0x0); //[APHY fix]  REG_BG_RBG dummy
	_rtd_part_outl(0x1800da18, 15, 14, 0x0); //[APHY fix]  REG_BG_RBG2 dummy
	_rtd_part_outl(0x1800da18, 16, 16, 0x0); //[APHY fix]  REG_BG_POW dummy
	_rtd_part_outl(0x1800da18, 17, 17, 0x0); //[APHY fix]  REG_BG_ENVBGUP
	_rtd_part_outl(0x1800da20, 7, 0, 0x00); //[]  dummy
	_rtd_part_outl(0x1800da20, 15, 8, 0x00); //[]  dummy
	_rtd_part_outl(0x1800da20, 23, 16, 0x00); //[]  dummy
	_rtd_part_outl(0x1800da20, 31, 24, 0x00); //[]  dummy
	_rtd_part_outl(0x1800da24, 7, 0, 0x00); //[]  dummy
	_rtd_part_outl(0x1800da24, 15, 8, 0x00); //[]  dummy
	_rtd_part_outl(0x1800da24, 23, 16, 0x00); //[]  dummy
	_rtd_part_outl(0x1800da24, 31, 24, 0x00); //[]  dummy
	_rtd_part_outl(0x1800da30, 1, 1, 0x0); //[APHY fix]  CK-Lane INPOFF_SINGLE_EN
	_rtd_part_outl(0x1800da30, 2, 2, 0x0); //[APHY fix]  CK-Lane INNOFF_SINGLE_EN
	_rtd_part_outl(0x1800da30, 3, 3, 0x1); //[APHY fix]  CK-Lane POW_AC_COUPLE
	_rtd_part_outl(0x1800da30, 5, 4, 0x0); //[APHY fix]  CK-Lane RXVCM_SEL[1:0]
	_rtd_part_outl(0x1800da34, 21, 21, 0x1); //[APHY fix]   	CK-Lane RS_CAL_EN
	_rtd_part_outl(0x1800da34, 25, 25, 0x1); //[APHY fix]   	CK-Lane POW_DATALANE_BIAS
	_rtd_part_outl(0x1800da34, 26, 26, 0x0); //[APHY fix]   	CK-Lane REG_FORCE_STARTUP(Const-GM)
	_rtd_part_outl(0x1800da34, 27, 27, 0x0); //[APHY fix]   	CK-Lane REG_POWB_STARTUP
	_rtd_part_outl(0x1800da38, 8, 8, 0x1); //[APHY fix]  		CK-Lane POW_DFE
	_rtd_part_outl(0x1800da38, 14, 12, 0x6); //[APHY fix] 		CK-Lane DFE_SUMAMP_ ISEL
	_rtd_part_outl(0x1800da38, 15, 15, 0x0); //[APHY fix] 		CK-Lane DFE_SUMAMP_DCGAIN_MAX
	_rtd_part_outl(0x1800da38, 19, 18, 0x0); //[APHY fix] 		CK-Lane Dummy
	_rtd_part_outl(0x1800da38, 20, 20, 0x0); //[APHY fix] 		CK-Lane DFE CKI_DELAY_EN
	_rtd_part_outl(0x1800da38, 21, 21, 0x0); //[APHY fix] 		CK-Lane DFE CKIB_DELAY_EN
	_rtd_part_outl(0x1800da38, 22, 22, 0x0); //[APHY fix] 		CK-Lane DFE CKQ_DELAY_EN
	_rtd_part_outl(0x1800da38, 23, 23, 0x0); //[APHY fix] 		CK-Lane DFE CKQB_DELAY_EN
	_rtd_part_outl(0x1800da38, 24, 24, 0x0); //[APHY fix] 		CK-Lane EN_EYE_MNT
	_rtd_part_outl(0x1800da38, 25, 25, 0x0); //[APHY fix] 		CK-Lane DEMUX input clock phase select:(for eye mnt)
	_rtd_part_outl(0x1800da38, 26, 26, 0x0); //[APHY fix] 		CK-Lane VTH_MANUAL
	_rtd_part_outl(0x1800da38, 27, 27, 0x0); //[APHY fix] 		CK-Lane DA_EG_VOS_PULLLOW(koffset disable)
	_rtd_part_outl(0x1800da38, 31, 29, 0x0); //[APHY fix] 			CK-Lane dummy
	_rtd_part_outl(0x1800da3c, 2, 0, 0x0); //[APHY fix]  	CK-Lane DFE_TAP_DELAY
	_rtd_part_outl(0x1800da3c, 8, 8, 0x0); //[APHY fix]   	CK-Lane rstb of dtata-lane test div8
	_rtd_part_outl(0x1800da3c, 9, 9, 0x0); //[APHY fix]   	CK-Lane TRANSITION CNT enable.
	_rtd_part_outl(0x1800da3c, 16, 16, 0x0); //[APHY fix]  		CK-Lane QCG1_EN
	_rtd_part_outl(0x1800da3c, 17, 17, 0x0); //[APHY fix]  		CK-Lane QCG1 injection-locked EN
	_rtd_part_outl(0x1800da3c, 18, 18, 0x0); //[APHY fix]  		CK-Lane QCG2_EN
	_rtd_part_outl(0x1800da3c, 19, 19, 0x0); //[APHY fix]  		CK-Lane QCG2 injection-locked EN
	_rtd_part_outl(0x1800da3c, 20, 20, 0x0); //[APHY fix]  		CK-Lane QCG FLD divider(/8) rstb
	_rtd_part_outl(0x1800da3c, 21, 21, 0x0); //[APHY fix]  		CK-Lane QCG FLD SEL
	_rtd_part_outl(0x1800da3c, 22, 22, 0x1); //[APHY fix]  		CK-Lane CK_FLD SEL.
	_rtd_part_outl(0x1800da3c, 23, 23, 0x1); //[APHY fix]  		CK-Lane CK-lane clk to B/G/R EN.
	_rtd_part_outl(0x1800da40, 8, 8, 0x1); //[APHY fix]  CK-Lane PI_EN
	_rtd_part_outl(0x1800da40, 12, 12, 0x0); //[APHY fix]  CK-Lane QCG1 Cload contrl SEL
	_rtd_part_outl(0x1800da40, 13, 13, 0x0); //[APHY fix]  CK-Lane QCG2 Cload contrl SEL
	_rtd_part_outl(0x1800da40, 15, 15, 0x0); //[APHY fix]  CK-Lane BIAS_PI_CUR_SEL
	_rtd_part_outl(0x1800da40, 21, 21, 0x0); //[APHY fix]  CK-Lane PI_EYE_EN
	_rtd_part_outl(0x1800da40, 23, 23, 0x0); //[APHY fix]  CK-Lane KI SEL
	_rtd_part_outl(0x1800da40, 25, 25, 0x0); //[APHY fix]  CK-Lane FKP_RSTB_SEL
	_rtd_part_outl(0x1800da40, 27, 26, 0x0); //[APHY fix]  CK-Lane KI_CK_SEL
	_rtd_part_outl(0x1800da40, 29, 29, 0x0); //[APHY fix]  CK-Lane ST_CODE SEL
	_rtd_part_outl(0x1800da40, 30, 30, 0x0); //[APHY fix]  CK-Lane QCG Ckin SEL.
	_rtd_part_outl(0x1800da44, 3, 2, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_DIV_FLD[1:0]
	_rtd_part_outl(0x1800da44, 5, 4, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_DIV_REF[1:0]
	_rtd_part_outl(0x1800da44, 7, 6, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_DIV_PLL[1:0]
	_rtd_part_outl(0x1800da44, 9, 8, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_DIV_TRAINING[1:0]
	_rtd_part_outl(0x1800da44, 12, 12, 0x0); //[APHY fix]  CK-Lane dummy
	_rtd_part_outl(0x1800da44, 13, 13, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_FLD_0CKFB_1CKREF
	_rtd_part_outl(0x1800da44, 15, 14, 0x0); //[APHY fix]  CK-Lane no use
	_rtd_part_outl(0x1800da44, 18, 18, 0x1); //[APHY fix]  CK-Lane ACDR_RSTB_DIV_PLL ... removed
	_rtd_part_outl(0x1800da44, 24, 24, 0x0); //[APHY fix]  CK-Lane ACDR_EN_UPDN_PULSE_FILTER
	_rtd_part_outl(0x1800da44, 25, 25, 0x1); //[APHY fix]  CK-Lane ACDR_RSTB_UPDN ... dummy
	_rtd_part_outl(0x1800da44, 27, 26, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_UPDN_WIDTH[1:0]
	_rtd_part_outl(0x1800da44, 28, 28, 0x1); //[APHY fix]  CK-Lane ACDR_POW_LPF_IDAC
	_rtd_part_outl(0x1800da44, 31, 29, 0x3); //[APHY fix]  CK-Lane ACDR_SEL_LPF_IDAC[2:0]
	_rtd_part_outl(0x1800da48, 0, 0, 0x1); //[APHY fix]  CK-Lane ACDR_POW_CP
	_rtd_part_outl(0x1800da48, 2, 2, 0x0); //[APHY fix]  CK-Lane ACDR_POW_IDN_BBPD
	_rtd_part_outl(0x1800da48, 3, 3, 0x0); //[APHY fix]  CK-Lane no use
	_rtd_part_outl(0x1800da48, 6, 4, 0x7); //[APHY fix]  CK-Lane ACDR_SEL_TIE_IDN_BBPD
	_rtd_part_outl(0x1800da48, 7, 7, 0x1); //[APHY fix]  CK-Lane ACDR_POW_IBIAS_IDN_HV
	_rtd_part_outl(0x1800da48, 8, 8, 0x1); //[APHY fix]  CK-Lane ACDR_POW_VCO
	_rtd_part_outl(0x1800da48, 9, 9, 0x1); //[APHY fix]  CK-Lane ACDR_POW_VCO_VDAC
	_rtd_part_outl(0x1800da48, 11, 10, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_V15_VDAC
	_rtd_part_outl(0x1800da48, 14, 14, 0x0); //[APHY fix]  CK-Lane no use
	_rtd_part_outl(0x1800da48, 15, 15, 0x0); //[APHY fix]  CK-Lane no use
	_rtd_part_outl(0x1800da48, 16, 16, 0x0); //[APHY fix]  CK-Lane ACDR_POW_TEST_MODE
	_rtd_part_outl(0x1800da48, 18, 17, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_TEST_MODE
	_rtd_part_outl(0x1800da48, 23, 19, 0x00); //[APHY fix]  CK-Lane no use
	_rtd_part_outl(0x1800da48, 31, 31, 0x0); //[APHY fix]  CK-Lane no use
	_rtd_part_outl(0x1800da4c, 15, 13, 0x1); //[APHY fix]  CK-Lane no use
	_rtd_part_outl(0x1800da4c, 19, 16, 0x0); //[APHY fix]  CK-Lane LE1_ISEL_IN_2 -> dummy
	_rtd_part_outl(0x1800da4c, 31, 25, 0x44); //[APHY fix]  CK-Lane no use for LEQ
	_rtd_part_outl(0x1800da50, 1, 1, 0x0); //[APHY fix]  		B-Lane INPOFF_SINGLE_EN
	_rtd_part_outl(0x1800da50, 2, 2, 0x0); //[APHY fix]  		B-Lane INNOFF_SINGLE_EN
	_rtd_part_outl(0x1800da50, 3, 3, 0x1); //[APHY fix]  		B-Lane POW_AC_COUPLE
	_rtd_part_outl(0x1800da50, 5, 4, 0x1); //[APHY fix]  		B-Lane RXVCM_SEL[1:0]
	_rtd_part_outl(0x1800da54, 21, 21, 0x1); //[APHY fix]   		B-Lane RS_CAL_EN
	_rtd_part_outl(0x1800da54, 25, 25, 0x1); //[APHY fix]   		B-Lane POW_DATALANE_BIAS
	_rtd_part_outl(0x1800da54, 26, 26, 0x0); //[APHY fix]   		B-Lane REG_FORCE_STARTUP(Const-GM)
	_rtd_part_outl(0x1800da54, 27, 27, 0x0); //[APHY fix]   		B-Lane REG_POWB_STARTUP
	_rtd_part_outl(0x1800da58, 8, 8, 0x1); //[APHY fix]  		B-Lane POW_DFE
	_rtd_part_outl(0x1800da58, 14, 12, 0x6); //[APHY fix] 		B-Lane DFE_SUMAMP_ ISEL
	_rtd_part_outl(0x1800da58, 15, 15, 0x0); //[APHY fix] 		B-Lane DFE_SUMAMP_DCGAIN_MAX
	_rtd_part_outl(0x1800da58, 19, 18, 0x0); //[APHY fix] 		B-Lane Dummy
	_rtd_part_outl(0x1800da58, 20, 20, 0x0); //[APHY fix] 		B-Lane DFE CKI_DELAY_EN
	_rtd_part_outl(0x1800da58, 21, 21, 0x0); //[APHY fix] 		B-Lane DFE CKIB_DELAY_EN
	_rtd_part_outl(0x1800da58, 22, 22, 0x0); //[APHY fix] 		B-Lane DFE CKQ_DELAY_EN
	_rtd_part_outl(0x1800da58, 23, 23, 0x0); //[APHY fix] 		B-Lane DFE CKQB_DELAY_EN
	_rtd_part_outl(0x1800da58, 24, 24, 0x0); //[APHY fix] 		B-Lane EN_EYE_MNT
	_rtd_part_outl(0x1800da58, 25, 25, 0x0); //[APHY fix] 		B-Lane DEMUX input clock phase select:(for eye mnt)
	_rtd_part_outl(0x1800da58, 26, 26, 0x0); //[APHY fix] 		B-Lane VTH_MANUAL
	_rtd_part_outl(0x1800da58, 27, 27, 0x0); //[APHY fix] 		B-Lane DA_EG_VOS_PULLLOW(koffset disable)
	_rtd_part_outl(0x1800da58, 31, 29, 0x0); //[APHY fix] 			B-Lane dummy
	_rtd_part_outl(0x1800da5c, 2, 0, 0x0); //[APHY fix]  	B-Lane DFE_TAP_DELAY
	_rtd_part_outl(0x1800da5c, 8, 8, 0x0); //[APHY fix]   	B-Lane rstb of dtata-lane test div8
	_rtd_part_outl(0x1800da5c, 9, 9, 0x0); //[APHY fix]   	B-Lane TRANSITION CNT enable.
	_rtd_part_outl(0x1800da5c, 16, 16, 0x1); //[APHY fix]  		B-Lane QCG1_EN
	_rtd_part_outl(0x1800da5c, 17, 17, 0x1); //[APHY fix]  		B-Lane QCG1 injection-locked EN
	_rtd_part_outl(0x1800da5c, 18, 18, 0x0); //[APHY fix]  		B-Lane QCG2_EN
	_rtd_part_outl(0x1800da5c, 19, 19, 0x0); //[APHY fix]  		B-Lane QCG2 injection-locked EN
	_rtd_part_outl(0x1800da5c, 20, 20, 0x0); //[APHY fix]  		B-Lane QCG FLD divider(/8) rstb
	_rtd_part_outl(0x1800da5c, 21, 21, 0x0); //[APHY fix]  		B-Lane QCG FLD SEL
	_rtd_part_outl(0x1800da5c, 22, 22, 0x1); //[APHY fix]  		B-Lane CK_FLD SEL.
	_rtd_part_outl(0x1800da5c, 23, 23, 0x0); //[APHY fix]  		B-Lane CK-lane clk to B/G/R EN.
	_rtd_part_outl(0x1800da60, 8, 8, 0x1); //[APHY fix]  B-Lane PI_EN
	_rtd_part_outl(0x1800da60, 12, 12, 0x0); //[APHY fix]  B-Lane QCG1 Cload contrl SEL
	_rtd_part_outl(0x1800da60, 13, 13, 0x0); //[APHY fix]  B-Lane QCG2 Cload contrl SEL
	_rtd_part_outl(0x1800da60, 15, 15, 0x0); //[APHY fix]  B-Lane BIAS_PI_CUR_SEL
	_rtd_part_outl(0x1800da60, 21, 21, 0x0); //[APHY fix]  B-Lane PI_EYE_EN
	_rtd_part_outl(0x1800da60, 23, 23, 0x0); //[APHY fix]  B-Lane KI SEL
	_rtd_part_outl(0x1800da60, 25, 25, 0x0); //[APHY fix]  B-Lane FKP_RSTB_SEL
	_rtd_part_outl(0x1800da60, 27, 26, 0x0); //[APHY fix]  B-Lane KI_CK_SEL
	_rtd_part_outl(0x1800da60, 29, 29, 0x0); //[APHY fix]  B-Lane ST_CODE SEL
	_rtd_part_outl(0x1800da60, 30, 30, 0x0); //[APHY fix]  B-Lane QCG Ckin SEL.
	_rtd_part_outl(0x1800da64, 3, 2, 0x0); //[APHY fix]  B-Lane ACDR_SEL_DIV_FLD[1:0]
	_rtd_part_outl(0x1800da64, 5, 4, 0x1); //[APHY fix]  B-Lane ACDR_SEL_DIV_REF[1:0]
	_rtd_part_outl(0x1800da64, 7, 6, 0x0); //[APHY fix]  B-Lane ACDR_SEL_DIV_PLL[1:0]
	_rtd_part_outl(0x1800da64, 9, 8, 0x0); //[APHY fix]  B-Lane ACDR_SEL_DIV_TRAINING[1:0]
	_rtd_part_outl(0x1800da64, 12, 12, 0x0); //[APHY fix]  B-Lane dummy
	_rtd_part_outl(0x1800da64, 13, 13, 0x0); //[APHY fix]  B-Lane ACDR_SEL_FLD_0CKFB_1CKREF
	_rtd_part_outl(0x1800da64, 15, 14, 0x0); //[APHY fix]  B-Lane no use
	_rtd_part_outl(0x1800da64, 18, 18, 0x0); //[APHY fix]  B-Lane ACDR_RSTB_DIV_PLL ... removed
	_rtd_part_outl(0x1800da64, 24, 24, 0x1); //[APHY fix]  B-Lane ACDR_EN_UPDN_PULSE_FILTER
	_rtd_part_outl(0x1800da64, 25, 25, 0x0); //[APHY fix]  B-Lane ACDR_RSTB_UPDN ... dummy
	_rtd_part_outl(0x1800da64, 27, 26, 0x0); //[APHY fix]  B-Lane ACDR_SEL_UPDN_WIDTH[1:0]
	_rtd_part_outl(0x1800da64, 28, 28, 0x0); //[APHY fix]  B-Lane ACDR_POW_LPF_IDAC
	_rtd_part_outl(0x1800da64, 31, 29, 0x0); //[APHY fix]  B-Lane ACDR_SEL_LPF_IDAC[2:0]
	_rtd_part_outl(0x1800da68, 0, 0, 0x0); //[APHY fix]  B-Lane ACDR_POW_CP
	_rtd_part_outl(0x1800da68, 2, 2, 0x0); //[APHY fix]  B-Lane ACDR_POW_IDN_BBPD
	_rtd_part_outl(0x1800da68, 3, 3, 0x0); //[APHY fix]  B-Lane no use
	_rtd_part_outl(0x1800da68, 6, 4, 0x2); //[APHY fix]  B-Lane ACDR_SEL_TIE_IDN_BBPD
	_rtd_part_outl(0x1800da68, 7, 7, 0x1); //[APHY fix]  B-Lane ACDR_POW_IBIAS_IDN_HV
	_rtd_part_outl(0x1800da68, 8, 8, 0x0); //[APHY fix]  B-Lane ACDR_POW_VCO
	_rtd_part_outl(0x1800da68, 9, 9, 0x0); //[APHY fix]  B-Lane ACDR_POW_VCO_VDAC
	_rtd_part_outl(0x1800da68, 11, 10, 0x0); //[APHY fix]  B-Lane ACDR_SEL_V15_VDAC
	_rtd_part_outl(0x1800da68, 14, 14, 0x0); //[APHY fix]  B-Lane no use
	_rtd_part_outl(0x1800da68, 15, 15, 0x0); //[APHY fix]  B-Lane no use
	_rtd_part_outl(0x1800da68, 16, 16, 0x0); //[APHY fix]  B-Lane ACDR_POW_TEST_MODE
	_rtd_part_outl(0x1800da68, 18, 17, 0x0); //[APHY fix]  B-Lane ACDR_SEL_TEST_MODE
	_rtd_part_outl(0x1800da68, 23, 19, 0x00); //[APHY fix]  B-Lane no use
	_rtd_part_outl(0x1800da68, 31, 31, 0x1); //[APHY fix]  B-Lane no use
	_rtd_part_outl(0x1800da6c, 15, 13, 0x0); //[APHY fix]  B-Lane no use
	_rtd_part_outl(0x1800da6c, 19, 16, 0x0); //[APHY fix]  B-Lane LE1_ISEL_IN_2 -> dummy
	_rtd_part_outl(0x1800da6c, 31, 25, 0x00); //[APHY fix]  B-Lane no use for LEQ
	_rtd_part_outl(0x1800da70, 1, 1, 0x0); //[APHY fix]  		G-Lane INPOFF_SINGLE_EN
	_rtd_part_outl(0x1800da70, 2, 2, 0x0); //[APHY fix]  		G-Lane INNOFF_SINGLE_EN
	_rtd_part_outl(0x1800da70, 3, 3, 0x1); //[APHY fix]  		G-Lane POW_AC_COUPLE
	_rtd_part_outl(0x1800da70, 5, 4, 0x1); //[APHY fix]  		G-Lane RXVCM_SEL[1:0]
	_rtd_part_outl(0x1800da74, 21, 21, 0x1); //[APHY fix]   		G-Lane RS_CAL_EN
	_rtd_part_outl(0x1800da74, 25, 25, 0x1); //[APHY fix]   		G-Lane POW_DATALANE_BIAS
	_rtd_part_outl(0x1800da74, 26, 26, 0x0); //[APHY fix]   		G-Lane REG_FORCE_STARTUP(Const-GM)
	_rtd_part_outl(0x1800da74, 27, 27, 0x0); //[APHY fix]   		G-Lane REG_POWB_STARTUP
	_rtd_part_outl(0x1800da78, 8, 8, 0x1); //[APHY fix]  		G-Lane POW_DFE
	_rtd_part_outl(0x1800da78, 14, 12, 0x6); //[APHY fix] 		G-Lane DFE_SUMAMP_ ISEL
	_rtd_part_outl(0x1800da78, 15, 15, 0x0); //[APHY fix] 		G-Lane DFE_SUMAMP_DCGAIN_MAX
	_rtd_part_outl(0x1800da78, 19, 18, 0x0); //[APHY fix] 		G-Lane Dummy
	_rtd_part_outl(0x1800da78, 20, 20, 0x0); //[APHY fix] 		G-Lane DFE CKI_DELAY_EN
	_rtd_part_outl(0x1800da78, 21, 21, 0x0); //[APHY fix] 		G-Lane DFE CKIB_DELAY_EN
	_rtd_part_outl(0x1800da78, 22, 22, 0x0); //[APHY fix] 		G-Lane DFE CKQ_DELAY_EN
	_rtd_part_outl(0x1800da78, 23, 23, 0x0); //[APHY fix] 		G-Lane DFE CKQB_DELAY_EN
	_rtd_part_outl(0x1800da78, 24, 24, 0x0); //[APHY fix] 		G-Lane EN_EYE_MNT
	_rtd_part_outl(0x1800da78, 25, 25, 0x0); //[APHY fix] 		G-Lane DEMUX input clock phase select:(for eye mnt)
	_rtd_part_outl(0x1800da78, 26, 26, 0x0); //[APHY fix] 		G-Lane VTH_MANUAL
	_rtd_part_outl(0x1800da78, 27, 27, 0x0); //[APHY fix] 		G-Lane DA_EG_VOS_PULLLOW(koffset disable)
	_rtd_part_outl(0x1800da78, 31, 29, 0x0); //[APHY fix] 			G-Lane dummy
	_rtd_part_outl(0x1800da7c, 2, 0, 0x0); //[APHY fix]  	G-Lane DFE_TAP_DELAY
	_rtd_part_outl(0x1800da7c, 8, 8, 0x0); //[APHY fix]   	G-Lane rstb of dtata-lane test div8
	_rtd_part_outl(0x1800da7c, 9, 9, 0x0); //[APHY fix]   	G-Lane TRANSITION CNT enable.
	_rtd_part_outl(0x1800da7c, 16, 16, 0x1); //[APHY fix]  		G-Lane QCG1_EN
	_rtd_part_outl(0x1800da7c, 17, 17, 0x1); //[APHY fix]  		G-Lane QCG1 injection-locked EN
	_rtd_part_outl(0x1800da7c, 18, 18, 0x0); //[APHY fix]  		G-Lane QCG2_EN
	_rtd_part_outl(0x1800da7c, 19, 19, 0x0); //[APHY fix]  		G-Lane QCG2 injection-locked EN
	_rtd_part_outl(0x1800da7c, 20, 20, 0x0); //[APHY fix]  		G-Lane QCG FLD divider(/8) rstb
	_rtd_part_outl(0x1800da7c, 21, 21, 0x0); //[APHY fix]  		G-Lane QCG FLD SEL
	_rtd_part_outl(0x1800da7c, 22, 22, 0x1); //[APHY fix]  		G-Lane CK_FLD SEL.
	_rtd_part_outl(0x1800da7c, 23, 23, 0x0); //[APHY fix]  		G-Lane CK-lane clk to B/G/R EN.
	_rtd_part_outl(0x1800da80, 8, 8, 0x1); //[APHY fix]  G-Lane PI_EN
	_rtd_part_outl(0x1800da80, 12, 12, 0x0); //[APHY fix]  G-Lane QCG1 Cload contrl SEL
	_rtd_part_outl(0x1800da80, 13, 13, 0x0); //[APHY fix]  G-Lane QCG2 Cload contrl SEL
	_rtd_part_outl(0x1800da80, 15, 15, 0x0); //[APHY fix]  G-Lane BIAS_PI_CUR_SEL
	_rtd_part_outl(0x1800da80, 21, 21, 0x0); //[APHY fix]  G-Lane PI_EYE_EN
	_rtd_part_outl(0x1800da80, 23, 23, 0x0); //[APHY fix]  G-Lane KI SEL
	_rtd_part_outl(0x1800da80, 25, 25, 0x0); //[APHY fix]  G-Lane FKP_RSTB_SEL
	_rtd_part_outl(0x1800da80, 27, 26, 0x0); //[APHY fix]  G-Lane KI_CK_SEL
	_rtd_part_outl(0x1800da80, 29, 29, 0x0); //[APHY fix]  G-Lane ST_CODE SEL
	_rtd_part_outl(0x1800da80, 30, 30, 0x0); //[APHY fix]  G-Lane QCG Ckin SEL.
	_rtd_part_outl(0x1800da84, 3, 2, 0x0); //[APHY fix]  G-Lane ACDR_SEL_DIV_FLD[1:0]
	_rtd_part_outl(0x1800da84, 5, 4, 0x1); //[APHY fix]  G-Lane ACDR_SEL_DIV_REF[1:0]
	_rtd_part_outl(0x1800da84, 7, 6, 0x0); //[APHY fix]  G-Lane ACDR_SEL_DIV_PLL[1:0]
	_rtd_part_outl(0x1800da84, 9, 8, 0x0); //[APHY fix]  G-Lane ACDR_SEL_DIV_TRAINING[1:0]
	_rtd_part_outl(0x1800da84, 12, 12, 0x0); //[APHY fix]  G-Lane dummy
	_rtd_part_outl(0x1800da84, 13, 13, 0x0); //[APHY fix]  G-Lane ACDR_SEL_FLD_0CKFB_1CKREF
	_rtd_part_outl(0x1800da84, 15, 14, 0x0); //[APHY fix]  G-Lane no use
	_rtd_part_outl(0x1800da84, 18, 18, 0x0); //[APHY fix]  G-Lane ACDR_RSTB_DIV_PLL ... removed
	_rtd_part_outl(0x1800da84, 24, 24, 0x1); //[APHY fix]  G-Lane ACDR_EN_UPDN_PULSE_FILTER
	_rtd_part_outl(0x1800da84, 25, 25, 0x0); //[APHY fix]  G-Lane ACDR_RSTB_UPDN ... dummy
	_rtd_part_outl(0x1800da84, 27, 26, 0x0); //[APHY fix]  G-Lane ACDR_SEL_UPDN_WIDTH[1:0]
	_rtd_part_outl(0x1800da84, 28, 28, 0x0); //[APHY fix]  G-Lane ACDR_POW_LPF_IDAC
	_rtd_part_outl(0x1800da84, 31, 29, 0x0); //[APHY fix]  G-Lane ACDR_SEL_LPF_IDAC[2:0]
	_rtd_part_outl(0x1800da88, 0, 0, 0x0); //[APHY fix]  G-Lane ACDR_POW_CP
	_rtd_part_outl(0x1800da88, 2, 2, 0x0); //[APHY fix]  G-Lane ACDR_POW_IDN_BBPD
	_rtd_part_outl(0x1800da88, 3, 3, 0x0); //[APHY fix]  G-Lane no use
	_rtd_part_outl(0x1800da88, 6, 4, 0x2); //[APHY fix]  G-Lane ACDR_SEL_TIE_IDN_BBPD
	_rtd_part_outl(0x1800da88, 7, 7, 0x1); //[APHY fix]  G-Lane ACDR_POW_IBIAS_IDN_HV
	_rtd_part_outl(0x1800da88, 8, 8, 0x0); //[APHY fix]  G-Lane ACDR_POW_VCO
	_rtd_part_outl(0x1800da88, 9, 9, 0x0); //[APHY fix]  G-Lane ACDR_POW_VCO_VDAC
	_rtd_part_outl(0x1800da88, 11, 10, 0x0); //[APHY fix]  G-Lane ACDR_SEL_V15_VDAC
	_rtd_part_outl(0x1800da88, 14, 14, 0x0); //[APHY fix]  G-Lane no use
	_rtd_part_outl(0x1800da88, 15, 15, 0x0); //[APHY fix]  G-Lane no use
	_rtd_part_outl(0x1800da88, 16, 16, 0x0); //[APHY fix]  G-Lane ACDR_POW_TEST_MODE
	_rtd_part_outl(0x1800da88, 18, 17, 0x0); //[APHY fix]  G-Lane ACDR_SEL_TEST_MODE
	_rtd_part_outl(0x1800da88, 23, 19, 0x00); //[APHY fix]  G-Lane no use
	_rtd_part_outl(0x1800da88, 31, 31, 0x1); //[APHY fix]  G-Lane no use
	_rtd_part_outl(0x1800da8c, 15, 13, 0x0); //[APHY fix]  G-Lane no use
	_rtd_part_outl(0x1800da8c, 19, 16, 0x0); //[APHY fix]  G-Lane LE1_ISEL_IN_2 -> dummy
	_rtd_part_outl(0x1800da8c, 31, 25, 0x00); //[APHY fix]  G-Lane no use for LEQ
	_rtd_part_outl(0x1800da90, 1, 1, 0x0); //[APHY fix]  		R-Lane INPOFF_SINGLE_EN
	_rtd_part_outl(0x1800da90, 2, 2, 0x0); //[APHY fix]  		R-Lane INNOFF_SINGLE_EN
	_rtd_part_outl(0x1800da90, 3, 3, 0x1); //[APHY fix]  		R-Lane POW_AC_COUPLE
	_rtd_part_outl(0x1800da90, 5, 4, 0x1); //[APHY fix]  		R-Lane RXVCM_SEL[1:0]
	_rtd_part_outl(0x1800da94, 21, 21, 0x1); //[APHY fix]   		R-Lane RS_CAL_EN
	_rtd_part_outl(0x1800da94, 25, 25, 0x1); //[APHY fix]   		R-Lane POW_DATALANE_BIAS
	_rtd_part_outl(0x1800da94, 26, 26, 0x0); //[APHY fix]   		R-Lane REG_FORCE_STARTUP(Const-GM)
	_rtd_part_outl(0x1800da94, 27, 27, 0x0); //[APHY fix]   		R-Lane REG_POWB_STARTUP
	_rtd_part_outl(0x1800da98, 8, 8, 0x1); //[APHY fix]  		R-Lane POW_DFE
	_rtd_part_outl(0x1800da98, 14, 12, 0x6); //[APHY fix] 		R-Lane DFE_SUMAMP_ ISEL
	_rtd_part_outl(0x1800da98, 15, 15, 0x0); //[APHY fix] 		R-Lane DFE_SUMAMP_DCGAIN_MAX
	_rtd_part_outl(0x1800da98, 19, 18, 0x0); //[APHY fix] 		R-Lane Dummy
	_rtd_part_outl(0x1800da98, 20, 20, 0x0); //[APHY fix] 		R-Lane DFE CKI_DELAY_EN
	_rtd_part_outl(0x1800da98, 21, 21, 0x0); //[APHY fix] 		R-Lane DFE CKIB_DELAY_EN
	_rtd_part_outl(0x1800da98, 22, 22, 0x0); //[APHY fix] 		R-Lane DFE CKQ_DELAY_EN
	_rtd_part_outl(0x1800da98, 23, 23, 0x0); //[APHY fix] 		R-Lane DFE CKQB_DELAY_EN
	_rtd_part_outl(0x1800da98, 24, 24, 0x0); //[APHY fix] 		R-Lane EN_EYE_MNT
	_rtd_part_outl(0x1800da98, 25, 25, 0x0); //[APHY fix] 		R-Lane DEMUX input clock phase select:(for eye mnt)
	_rtd_part_outl(0x1800da98, 26, 26, 0x0); //[APHY fix] 		R-Lane VTH_MANUAL
	_rtd_part_outl(0x1800da98, 27, 27, 0x0); //[APHY fix] 		R-Lane DA_EG_VOS_PULLLOW(koffset disable)
	_rtd_part_outl(0x1800da98, 31, 29, 0x0); //[APHY fix] 			R-Lane dummy
	_rtd_part_outl(0x1800da9c, 2, 0, 0x0); //[APHY fix]  	R-Lane DFE_TAP_DELAY
	_rtd_part_outl(0x1800da9c, 8, 8, 0x0); //[APHY fix]   	R-Lane rstb of dtata-lane test div8
	_rtd_part_outl(0x1800da9c, 9, 9, 0x0); //[APHY fix]   	R-Lane TRANSITION CNT enable.
	_rtd_part_outl(0x1800da9c, 16, 16, 0x1); //[APHY fix]  		R-Lane QCG1_EN
	_rtd_part_outl(0x1800da9c, 17, 17, 0x1); //[APHY fix]  		R-Lane QCG1 injection-locked EN
	_rtd_part_outl(0x1800da9c, 18, 18, 0x0); //[APHY fix]  		R-Lane QCG2_EN
	_rtd_part_outl(0x1800da9c, 19, 19, 0x0); //[APHY fix]  		R-Lane QCG2 injection-locked EN
	_rtd_part_outl(0x1800da9c, 20, 20, 0x0); //[APHY fix]  		R-Lane QCG FLD divider(/8) rstb
	_rtd_part_outl(0x1800da9c, 21, 21, 0x0); //[APHY fix]  		R-Lane QCG FLD SEL
	_rtd_part_outl(0x1800da9c, 22, 22, 0x1); //[APHY fix]  		R-Lane CK_FLD SEL.
	_rtd_part_outl(0x1800da9c, 23, 23, 0x0); //[APHY fix]  		R-Lane CK-lane clk to B/G/R EN.
	_rtd_part_outl(0x1800daa0, 8, 8, 0x1); //[APHY fix]  R-Lane PI_EN
	_rtd_part_outl(0x1800daa0, 12, 12, 0x0); //[APHY fix]  R-Lane QCG1 Cload contrl SEL
	_rtd_part_outl(0x1800daa0, 13, 13, 0x0); //[APHY fix]  R-Lane QCG2 Cload contrl SEL
	_rtd_part_outl(0x1800daa0, 15, 15, 0x0); //[APHY fix]  R-Lane BIAS_PI_CUR_SEL
	_rtd_part_outl(0x1800daa0, 21, 21, 0x0); //[APHY fix]  R-Lane PI_EYE_EN
	_rtd_part_outl(0x1800daa0, 23, 23, 0x0); //[APHY fix]  R-Lane KI SEL
	_rtd_part_outl(0x1800daa0, 25, 25, 0x0); //[APHY fix]  R-Lane FKP_RSTB_SEL
	_rtd_part_outl(0x1800daa0, 27, 26, 0x0); //[APHY fix]  R-Lane KI_CK_SEL
	_rtd_part_outl(0x1800daa0, 29, 29, 0x0); //[APHY fix]  R-Lane ST_CODE SEL
	_rtd_part_outl(0x1800daa0, 30, 30, 0x0); //[APHY fix]  R-Lane QCG Ckin SEL.
	_rtd_part_outl(0x1800daa4, 3, 2, 0x0); //[APHY fix]  R-Lane ACDR_SEL_DIV_FLD[1:0]
	_rtd_part_outl(0x1800daa4, 5, 4, 0x1); //[APHY fix]  R-Lane ACDR_SEL_DIV_REF[1:0]
	_rtd_part_outl(0x1800daa4, 7, 6, 0x0); //[APHY fix]  R-Lane ACDR_SEL_DIV_PLL[1:0]
	_rtd_part_outl(0x1800daa4, 9, 8, 0x0); //[APHY fix]  R-Lane ACDR_SEL_DIV_TRAINING[1:0]
	_rtd_part_outl(0x1800daa4, 12, 12, 0x0); //[APHY fix]  R-Lane dummy
	_rtd_part_outl(0x1800daa4, 13, 13, 0x0); //[APHY fix]  R-Lane ACDR_SEL_FLD_0CKFB_1CKREF
	_rtd_part_outl(0x1800daa4, 15, 14, 0x0); //[APHY fix]  R-Lane no use
	_rtd_part_outl(0x1800daa4, 18, 18, 0x0); //[APHY fix]  R-Lane ACDR_RSTB_DIV_PLL ... removed
	_rtd_part_outl(0x1800daa4, 24, 24, 0x1); //[APHY fix]  R-Lane ACDR_EN_UPDN_PULSE_FILTER
	_rtd_part_outl(0x1800daa4, 25, 25, 0x0); //[APHY fix]  R-Lane ACDR_RSTB_UPDN ... dummy
	_rtd_part_outl(0x1800daa4, 27, 26, 0x0); //[APHY fix]  R-Lane ACDR_SEL_UPDN_WIDTH[1:0]
	_rtd_part_outl(0x1800daa4, 28, 28, 0x0); //[APHY fix]  R-Lane ACDR_POW_LPF_IDAC
	_rtd_part_outl(0x1800daa4, 31, 29, 0x0); //[APHY fix]  R-Lane ACDR_SEL_LPF_IDAC[2:0]
	_rtd_part_outl(0x1800daa8, 0, 0, 0x0); //[APHY fix]  R-Lane ACDR_POW_CP
	_rtd_part_outl(0x1800daa8, 2, 2, 0x0); //[APHY fix]  R-Lane ACDR_POW_IDN_BBPD
	_rtd_part_outl(0x1800daa8, 3, 3, 0x0); //[APHY fix]  R-Lane no use
	_rtd_part_outl(0x1800daa8, 6, 4, 0x2); //[APHY fix]  R-Lane ACDR_SEL_TIE_IDN_BBPD
	_rtd_part_outl(0x1800daa8, 7, 7, 0x1); //[APHY fix]  R-Lane ACDR_POW_IBIAS_IDN_HV
	_rtd_part_outl(0x1800daa8, 8, 8, 0x0); //[APHY fix]  R-Lane ACDR_POW_VCO
	_rtd_part_outl(0x1800daa8, 9, 9, 0x0); //[APHY fix]  R-Lane ACDR_POW_VCO_VDAC
	_rtd_part_outl(0x1800daa8, 11, 10, 0x0); //[APHY fix]  R-Lane ACDR_SEL_V15_VDAC
	_rtd_part_outl(0x1800daa8, 14, 14, 0x0); //[APHY fix]  R-Lane no use
	_rtd_part_outl(0x1800daa8, 15, 15, 0x0); //[APHY fix]  R-Lane no use
	_rtd_part_outl(0x1800daa8, 16, 16, 0x0); //[APHY fix]  R-Lane ACDR_POW_TEST_MODE
	_rtd_part_outl(0x1800daa8, 18, 17, 0x0); //[APHY fix]  R-Lane ACDR_SEL_TEST_MODE
	_rtd_part_outl(0x1800daa8, 23, 19, 0x00); //[APHY fix]  R-Lane no use
	_rtd_part_outl(0x1800daa8, 31, 31, 0x1); //[APHY fix]  R-Lane no use
	_rtd_part_outl(0x1800daac, 15, 13, 0x0); //[APHY fix]  R-Lane no use
	_rtd_part_outl(0x1800daac, 19, 16, 0x0); //[APHY fix]  R-Lane LE1_ISEL_IN_2 -> dummy
	_rtd_part_outl(0x1800daac, 31, 25, 0x00); //[APHY fix]  R-Lane no use for LEQ
	_rtd_part_outl(0x1800dab4, 0, 0, 0x1); //[APHY fix]  Non-Hysteresis Amp Power
	_rtd_part_outl(0x1800dab4, 2, 1, 0x3); //[APHY fix]  CK Mode hysteresis amp Hysteresis adjustment[1:0]
	_rtd_part_outl(0x1800dab4, 3, 3, 0x1); //[APHY fix]  CK Mode detect source
	_rtd_part_outl(0x1800dab4, 5, 5, 0x0); //[APHY fix]  no use
	_rtd_part_outl(0x1800dab4, 6, 6, 0x1); //[APHY fix]  Hysteresis amp power
	_rtd_part_outl(0x1800dab4, 8, 8, 0x0); //[APHY fix]  CMU_WDPOW
	_rtd_part_outl(0x1800dab4, 9, 9, 0x0); //[APHY fix]  CMU_WDSET
	_rtd_part_outl(0x1800dab4, 10, 10, 0x0); //[APHY fix]  CMU_WDRST
	_rtd_part_outl(0x1800dab4, 11, 11, 0x1); //[APHY fix]  no use
	_rtd_part_outl(0x1800dab4, 12, 12, 0x0); //[APHY fix]  EN_TST_CKBK
	_rtd_part_outl(0x1800dab4, 13, 13, 0x0); //[APHY fix]  EN_TST_CKREF
	_rtd_part_outl(0x1800dab4, 15, 14, 0x0); //[APHY fix]  no use
	_rtd_part_outl(0x1800dab4, 23, 16, 0x00); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dab4, 31, 24, 0x00); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dab8, 7, 0, 0xf4); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dab8, 9, 9, 0x1); //[APHY fix]  no use
	_rtd_part_outl(0x1800dab8, 10, 10, 0x1); //[APHY fix]  CMU_SEL_CS
	_rtd_part_outl(0x1800dab8, 15, 11, 0x09); //[APHY fix]  no use
	_rtd_part_outl(0x1800dab8, 23, 16, 0x06); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dab8, 31, 24, 0x07); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dabc, 7, 0, 0x44); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dabc, 15, 8, 0xef); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dabc, 23, 16, 0x0f); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dabc, 31, 24, 0x00); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dac0, 7, 0, 0x00); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dac0, 15, 8, 0x00); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dac0, 23, 16, 0x00); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dac0, 31, 24, 0x01); //[APHY fix]  dummy
}

void APHY_Para_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing)
{
	// APHY para start
	_rtd_part_outl(0x1800da44, 19, 19, 0x1); //[APHY para]  CK-Lane  ACDR_SEL_HS_CLK
	_rtd_part_outl(0x1800da44, 20, 20, 0x1); //[APHY para]  CK-Lane ACDR_SEL_0FR_1HR_DIV_IQ
	_rtd_part_outl(0x1800da48, 1, 1, APHY_Para_tmds_1[tmds_timing].ACDR_CK_5_4); //[APHY para]  CK-Lane ACDR_POW_CP_INTG2_CORE
	_rtd_part_outl(0x1800da48, 13, 12, APHY_Para_tmds_1[tmds_timing].ACDR_CK_6_5); //[APHY para]  CK-Lane ACDR_SEL_BAND_CAP
	_rtd_part_outl(0x1800da4c, 7, 0, APHY_Para_tmds_1[tmds_timing].ACDR_CK_9_6); //[APHY para]  CK-Lane CMU_SEL_M_DIV[7:0]
	_rtd_part_outl(0x1800da4c, 8, 8, 0x0); //[APHY para]  CK-Lane CMU_SEL_M_DIV[8]
	_rtd_part_outl(0x1800da4c, 9, 9, 0x0); //[APHY para]  CK-Lane CMU_SEL_PRE_M_DIV
	_rtd_part_outl(0x1800da4c, 10, 10, 0x0); //[APHY para]  CK-Lane CMU_SEL_POST_M_DIV
	_rtd_part_outl(0x1800da4c, 21, 20, APHY_Para_tmds_1[tmds_timing].ACDR_CK_11_10); //[APHY para]  CK-Lane RLSEL_LE1_2
	_rtd_part_outl(0x1800da50, 9, 8, APHY_Para_tmds_1[tmds_timing].B_2_11); //[APHY para]  		B-Lane RLSEL_LE1
	_rtd_part_outl(0x1800da50, 11, 10, APHY_Para_tmds_1[tmds_timing].B_2_12); //[APHY para]  		B-Lane RLSEL_LE2
	_rtd_part_outl(0x1800da50, 13, 12, APHY_Para_tmds_1[tmds_timing].B_2_13); //[APHY para]  		B-Lane RLSEL_NC
	_rtd_part_outl(0x1800da50, 15, 14, APHY_Para_tmds_1[tmds_timing].B_2_14); //[APHY para]  		B-Lane RLSEL_TAP0
	_rtd_part_outl(0x1800da50, 19, 16, APHY_Para_tmds_1[tmds_timing].B_3_15); //[APHY para]  		B-Lane RSSEL_LE1_1
	_rtd_part_outl(0x1800da50, 23, 20, APHY_Para_tmds_1[tmds_timing].B_3_16); //[APHY para]  		B-Lane RSSEL_LE1_2
	_rtd_part_outl(0x1800da50, 27, 24, APHY_Para_tmds_1[tmds_timing].B_4_17); //[APHY para]  		B-Lane RSSEL_LE2
	_rtd_part_outl(0x1800da50, 29, 28, APHY_Para_tmds_1[tmds_timing].B_4_18); //[APHY para]  		B-Lane RSSEL_TAP0
	_rtd_part_outl(0x1800da50, 31, 30, APHY_Para_tmds_1[tmds_timing].B_4_19); //[APHY para]  	B-Lane KOFF_RANGE
	_rtd_part_outl(0x1800da54, 3, 0, APHY_Para_tmds_1[tmds_timing].B_5_20); //[APHY para]    	B-Lane LE1_ISEL_IN
	_rtd_part_outl(0x1800da54, 7, 4, APHY_Para_tmds_1[tmds_timing].B_5_21); //[APHY para]    	B-Lane LE2_ISEL_IN
	_rtd_part_outl(0x1800da54, 11, 8, APHY_Para_tmds_1[tmds_timing].B_6_22); //[APHY para]   		B-Lane LE_NC_ISEL_IN
	_rtd_part_outl(0x1800da54, 15, 12, 0xf); //[APHY para]  		B-Lane TAP0_ISEL_IN
	_rtd_part_outl(0x1800da54, 17, 16, APHY_Para_tmds_1[tmds_timing].B_7_24); //[APHY para]   		B-Lane LE_IHALF[1:0]
	_rtd_part_outl(0x1800da54, 18, 18, APHY_Para_tmds_1[tmds_timing].B_7_25); //[APHY para]   		B-Lane NC_IHALF
	_rtd_part_outl(0x1800da54, 19, 19, APHY_Para_tmds_1[tmds_timing].B_7_26); //[APHY para]   		B-Lane TAP0_IHALF
	_rtd_part_outl(0x1800da54, 20, 20, 0x0); //[APHY para]   		B-Lane EN_LEQ6G
	_rtd_part_outl(0x1800da54, 22, 22, APHY_Para_tmds_1[tmds_timing].B_7_28); //[APHY para]   	B-Lane POW_NC
	_rtd_part_outl(0x1800da54, 30, 28, 0x3); //[APHY para]   		B-Lane DATALANE_BIAS_ISEL[2:0]
	_rtd_part_outl(0x1800da54, 31, 31, 0x1); //[APHY para]   		B-Lane POW_LEQ_RL
	_rtd_part_outl(0x1800da58, 1, 1, APHY_Para_tmds_1[tmds_timing].B_9_31); //[APHY para]  		B-Lane SEL_CMFB_LS
	_rtd_part_outl(0x1800da58, 3, 2, 0x3); //[APHY para]  		B-Lane LEQ_CUR_ADJ
	_rtd_part_outl(0x1800da58, 5, 4, 0x0); //[APHY para]  		B-Lane PTAT_CUR_ADJ
	_rtd_part_outl(0x1800da58, 6, 6, 0x0); //[APHY para]  		B-Lane REG_BIAS_POW_CON_GM
	_rtd_part_outl(0x1800da58, 11, 9, 0x0); //[APHY para] 		B-Lane PTAT_CUR_ADJ_FINE
	_rtd_part_outl(0x1800da58, 16, 16, 0x1); //[APHY para] 		B-Lane FR_CK_SEL
	_rtd_part_outl(0x1800da5c, 11, 10, APHY_Para_tmds_1[tmds_timing].B_14_37); //[APHY para]   	B-Lane DEMUX_PIX_RATE_SEL<1:0>
	_rtd_part_outl(0x1800da5c, 12, 12, 0x1); //[APHY para]   	B-Lane DEMUX_FR_CK_SEL
	_rtd_part_outl(0x1800da5c, 15, 13, APHY_Para_tmds_1[tmds_timing].B_14_39); //[APHY para]   	B-Lane DEMUX_RATE_SEL
	_rtd_part_outl(0x1800da5c, 24, 24, 0x0); //[APHY para]  		B-Lane QCG1_SEL_BIAS_ST
	_rtd_part_outl(0x1800da5c, 25, 25, 0x0); //[APHY para]  		B-Lane QCG1_SEL_BIAS_RG
	_rtd_part_outl(0x1800da5c, 27, 26, APHY_Para_tmds_1[tmds_timing].B_16_42); //[APHY para]  		B-Lane QCG1 Rload SEL
	_rtd_part_outl(0x1800da5c, 31, 28, APHY_Para_tmds_1[tmds_timing].B_16_43); //[APHY para]  		B-Lane QCG1 Cload SEL
	_rtd_part_outl(0x1800da60, 11, 9, APHY_Para_tmds_1[tmds_timing].B_18_44); //[APHY para]  B-Lane PI_ISEL<2:0>
	_rtd_part_outl(0x1800da60, 14, 14, 0x1); //[APHY para]  B-Lane PI_CSEL
	_rtd_part_outl(0x1800da60, 20, 18, APHY_Para_tmds_1[tmds_timing].B_19_46); //[APHY para]  B-Lane PI_DIV_SEL
	_rtd_part_outl(0x1800da60, 22, 22, 0x0); //[APHY para]  B-Lane DCDR ckout SEL
	_rtd_part_outl(0x1800da60, 31, 31, 0x1); //[APHY para]  B-Lane DFE Ckin SEL.
	_rtd_part_outl(0x1800da6c, 21, 20, APHY_Para_tmds_1[tmds_timing].ACDR_B_11_49); //[APHY para]  B-Lane RLSEL_LE1_2
	_rtd_part_outl(0x1800da6c, 23, 22, 0x0); //[APHY para]  B-Lane RLSEL_NC_2
	_rtd_part_outl(0x1800da6c, 24, 24, 0x0); //[APHY para]  B-Lane POW_NC
	_rtd_part_outl(0x1800da70, 9, 8, APHY_Para_tmds_1[tmds_timing].G_2_52); //[APHY para]  		G-Lane RLSEL_LE1
	_rtd_part_outl(0x1800da70, 11, 10, APHY_Para_tmds_1[tmds_timing].G_2_53); //[APHY para]  		G-Lane RLSEL_LE2
	_rtd_part_outl(0x1800da70, 13, 12, APHY_Para_tmds_1[tmds_timing].G_2_54); //[APHY para]  		G-Lane RLSEL_NC
	_rtd_part_outl(0x1800da70, 15, 14, APHY_Para_tmds_1[tmds_timing].G_2_55); //[APHY para]  		G-Lane RLSEL_TAP0
	_rtd_part_outl(0x1800da70, 19, 16, APHY_Para_tmds_1[tmds_timing].G_3_56); //[APHY para]  		G-Lane RSSEL_LE1_1
	_rtd_part_outl(0x1800da70, 23, 20, APHY_Para_tmds_1[tmds_timing].G_3_57); //[APHY para]  		G-Lane RSSEL_LE1_2
	_rtd_part_outl(0x1800da70, 27, 24, APHY_Para_tmds_1[tmds_timing].G_4_58); //[APHY para]  		G-Lane RSSEL_LE2
	_rtd_part_outl(0x1800da70, 29, 28, APHY_Para_tmds_1[tmds_timing].G_4_59); //[APHY para]  		G-Lane RSSEL_TAP0
	_rtd_part_outl(0x1800da70, 31, 30, APHY_Para_tmds_1[tmds_timing].G_4_60); //[APHY para]  	G-Lane KOFF_RANGE
	_rtd_part_outl(0x1800da74, 3, 0, APHY_Para_tmds_1[tmds_timing].G_5_61); //[APHY para]    	G-Lane LE1_ISEL_IN
	_rtd_part_outl(0x1800da74, 7, 4, APHY_Para_tmds_1[tmds_timing].G_5_62); //[APHY para]    	G-Lane LE2_ISEL_IN
	_rtd_part_outl(0x1800da74, 11, 8, APHY_Para_tmds_1[tmds_timing].G_6_63); //[APHY para]   		G-Lane LE_NC_ISEL_IN
	_rtd_part_outl(0x1800da74, 15, 12, 0xf); //[APHY para]  		G-Lane TAP0_ISEL_IN
	_rtd_part_outl(0x1800da74, 17, 16, APHY_Para_tmds_1[tmds_timing].G_7_65); //[APHY para]   		G-Lane LE_IHALF[1:0]
	_rtd_part_outl(0x1800da74, 18, 18, APHY_Para_tmds_1[tmds_timing].G_7_66); //[APHY para]   		G-Lane NC_IHALF
	_rtd_part_outl(0x1800da74, 19, 19, APHY_Para_tmds_1[tmds_timing].G_7_67); //[APHY para]   		G-Lane TAP0_IHALF
	_rtd_part_outl(0x1800da74, 20, 20, 0x0); //[APHY para]   		G-Lane EN_LEQ6G
	_rtd_part_outl(0x1800da74, 22, 22, APHY_Para_tmds_1[tmds_timing].G_7_69); //[APHY para]   	G-Lane POW_NC
	_rtd_part_outl(0x1800da74, 30, 28, 0x3); //[APHY para]   		G-Lane DATALANE_BIAS_ISEL[2:0]
	_rtd_part_outl(0x1800da74, 31, 31, 0x1); //[APHY para]   		G-Lane POW_LEQ_RL
	_rtd_part_outl(0x1800da78, 1, 1, APHY_Para_tmds_1[tmds_timing].G_9_72); //[APHY para]  		G-Lane SEL_CMFB_LS
	_rtd_part_outl(0x1800da78, 3, 2, APHY_Para_tmds_1[tmds_timing].G_9_73); //[APHY para]  		G-Lane LEQ_CUR_ADJ
	_rtd_part_outl(0x1800da78, 5, 4, APHY_Para_tmds_1[tmds_timing].G_9_74); //[APHY para]  		G-Lane PTAT_CUR_ADJ
	_rtd_part_outl(0x1800da78, 6, 6, 0x0); //[APHY para]  		G-Lane REG_BIAS_POW_CON_GM
	_rtd_part_outl(0x1800da78, 11, 9, 0x0); //[APHY para] 		G-Lane PTAT_CUR_ADJ_FINE
	_rtd_part_outl(0x1800da78, 16, 16, 0x1); //[APHY para] 		G-Lane FR_CK_SEL
	_rtd_part_outl(0x1800da7c, 11, 10, APHY_Para_tmds_1[tmds_timing].G_14_78); //[APHY para]   	G-Lane DEMUX_PIX_RATE_SEL<1:0>
	_rtd_part_outl(0x1800da7c, 12, 12, 0x1); //[APHY para]   	G-Lane DEMUX_FR_CK_SEL
	_rtd_part_outl(0x1800da7c, 15, 13, APHY_Para_tmds_1[tmds_timing].G_14_80); //[APHY para]   	G-Lane DEMUX_RATE_SEL
	_rtd_part_outl(0x1800da7c, 24, 24, 0x0); //[APHY para]  		G-Lane QCG1_SEL_BIAS_ST
	_rtd_part_outl(0x1800da7c, 25, 25, 0x0); //[APHY para]  		G-Lane QCG1_SEL_BIAS_RG
	_rtd_part_outl(0x1800da7c, 27, 26, APHY_Para_tmds_1[tmds_timing].G_16_83); //[APHY para]  		G-Lane QCG1 Rload SEL
	_rtd_part_outl(0x1800da7c, 31, 28, APHY_Para_tmds_1[tmds_timing].G_16_84); //[APHY para]  		G-Lane QCG1 Cload SEL
	_rtd_part_outl(0x1800da80, 11, 9, APHY_Para_tmds_1[tmds_timing].G_18_85); //[APHY para]  G-Lane PI_ISEL<2:0>
	_rtd_part_outl(0x1800da80, 14, 14, 0x1); //[APHY para]  G-Lane PI_CSEL
	_rtd_part_outl(0x1800da80, 20, 18, APHY_Para_tmds_1[tmds_timing].G_19_87); //[APHY para]  G-Lane PI_DIV_SEL
	_rtd_part_outl(0x1800da80, 22, 22, 0x0); //[APHY para]  G-Lane DCDR ckout SEL
	_rtd_part_outl(0x1800da80, 31, 31, 0x1); //[APHY para]  G-Lane DFE Ckin SEL.
	_rtd_part_outl(0x1800da8c, 21, 20, APHY_Para_tmds_1[tmds_timing].ACDR_G_11_90); //[APHY para]  G-Lane RLSEL_LE1_2
	_rtd_part_outl(0x1800da8c, 23, 22, 0x0); //[APHY para]  G-Lane RLSEL_NC_2
	_rtd_part_outl(0x1800da8c, 24, 24, 0x0); //[APHY para]  G-Lane POW_NC
	_rtd_part_outl(0x1800da90, 9, 8, APHY_Para_tmds_1[tmds_timing].R_2_93); //[APHY para]  		R-Lane RLSEL_LE1
	_rtd_part_outl(0x1800da90, 11, 10, APHY_Para_tmds_1[tmds_timing].R_2_94); //[APHY para]  		R-Lane RLSEL_LE2
	_rtd_part_outl(0x1800da90, 13, 12, APHY_Para_tmds_1[tmds_timing].R_2_95); //[APHY para]  		R-Lane RLSEL_NC
	_rtd_part_outl(0x1800da90, 15, 14, APHY_Para_tmds_1[tmds_timing].R_2_96); //[APHY para]  		R-Lane RLSEL_TAP0
	_rtd_part_outl(0x1800da90, 19, 16, APHY_Para_tmds_1[tmds_timing].R_3_97); //[APHY para]  		R-Lane RSSEL_LE1_1
	_rtd_part_outl(0x1800da90, 23, 20, APHY_Para_tmds_1[tmds_timing].R_3_98); //[APHY para]  		R-Lane RSSEL_LE1_2
	_rtd_part_outl(0x1800da90, 27, 24, APHY_Para_tmds_1[tmds_timing].R_4_99); //[APHY para]  		R-Lane RSSEL_LE2
	_rtd_part_outl(0x1800da90, 29, 28, APHY_Para_tmds_1[tmds_timing].R_4_100); //[APHY para]  		R-Lane RSSEL_TAP0
	_rtd_part_outl(0x1800da90, 31, 30, APHY_Para_tmds_1[tmds_timing].R_4_101); //[APHY para]  		R-Lane KOFF_RANGE
	_rtd_part_outl(0x1800da94, 3, 0, APHY_Para_tmds_1[tmds_timing].R_5_102); //[APHY para]    	R-Lane LE1_ISEL_IN
	_rtd_part_outl(0x1800da94, 7, 4, APHY_Para_tmds_1[tmds_timing].R_5_103); //[APHY para]    	R-Lane LE2_ISEL_IN
	_rtd_part_outl(0x1800da94, 11, 8, APHY_Para_tmds_1[tmds_timing].R_6_104); //[APHY para]   		R-Lane LE_NC_ISEL_IN
	_rtd_part_outl(0x1800da94, 15, 12, 0xf); //[APHY para]  		R-Lane TAP0_ISEL_IN
	_rtd_part_outl(0x1800da94, 17, 16, APHY_Para_tmds_1[tmds_timing].R_7_106); //[APHY para]   		R-Lane LE_IHALF[1:0]
	_rtd_part_outl(0x1800da94, 18, 18, APHY_Para_tmds_1[tmds_timing].R_7_107); //[APHY para]   		R-Lane NC_IHALF
	_rtd_part_outl(0x1800da94, 19, 19, APHY_Para_tmds_1[tmds_timing].R_7_108); //[APHY para]   		R-Lane TAP0_IHALF
	_rtd_part_outl(0x1800da94, 20, 20, 0x0); //[APHY para]   		R-Lane EN_LEQ6G
	_rtd_part_outl(0x1800da94, 22, 22, APHY_Para_tmds_1[tmds_timing].R_7_110); //[APHY para]   	R-Lane POW_NC
	_rtd_part_outl(0x1800da94, 30, 28, 0x3); //[APHY para]   		R-Lane DATALANE_BIAS_ISEL[2:0]
	_rtd_part_outl(0x1800da94, 31, 31, 0x1); //[APHY para]   		R-Lane POW_LEQ_RL
	_rtd_part_outl(0x1800da98, 1, 1, APHY_Para_tmds_1[tmds_timing].R_9_113); //[APHY para]  		R-Lane SEL_CMFB_LS
	_rtd_part_outl(0x1800da98, 3, 2, APHY_Para_tmds_1[tmds_timing].R_9_114); //[APHY para]  		R-Lane LEQ_CUR_ADJ
	_rtd_part_outl(0x1800da98, 5, 4, APHY_Para_tmds_1[tmds_timing].R_9_115); //[APHY para]  		R-Lane PTAT_CUR_ADJ
	_rtd_part_outl(0x1800da98, 6, 6, 0x0); //[APHY para]  		R-Lane REG_BIAS_POW_CON_GM
	_rtd_part_outl(0x1800da98, 11, 9, 0x0); //[APHY para] 		R-Lane PTAT_CUR_ADJ_FINE
	_rtd_part_outl(0x1800da98, 16, 16, 0x1); //[APHY para] 		R-Lane FR_CK_SEL
	_rtd_part_outl(0x1800da9c, 11, 10, APHY_Para_tmds_1[tmds_timing].R_14_119); //[APHY para]   	R-Lane DEMUX_PIX_RATE_SEL<1:0>
	_rtd_part_outl(0x1800da9c, 12, 12, 0x1); //[APHY para]   	R-Lane DEMUX_FR_CK_SEL
	_rtd_part_outl(0x1800da9c, 15, 13, APHY_Para_tmds_1[tmds_timing].R_14_121); //[APHY para]   	R-Lane DEMUX_RATE_SEL
	_rtd_part_outl(0x1800da9c, 24, 24, 0x0); //[APHY para]  		R-Lane QCG1_SEL_BIAS_ST
	_rtd_part_outl(0x1800da9c, 25, 25, 0x0); //[APHY para]  		R-Lane QCG1_SEL_BIAS_RG
	_rtd_part_outl(0x1800da9c, 27, 26, APHY_Para_tmds_1[tmds_timing].R_16_124); //[APHY para]  		R-Lane QCG1 Rload SEL
	_rtd_part_outl(0x1800da9c, 31, 28, APHY_Para_tmds_1[tmds_timing].R_16_125); //[APHY para]  		R-Lane QCG1 Cload SEL
	_rtd_part_outl(0x1800daa0, 11, 9, APHY_Para_tmds_1[tmds_timing].R_18_126); //[APHY para]  R-Lane PI_ISEL<2:0>
	_rtd_part_outl(0x1800daa0, 14, 14, 0x1); //[APHY para]  R-Lane PI_CSEL
	_rtd_part_outl(0x1800daa0, 20, 18, APHY_Para_tmds_1[tmds_timing].R_19_128); //[APHY para]  R-Lane PI_DIV_SEL
	_rtd_part_outl(0x1800daa0, 22, 22, 0x0); //[APHY para]  R-Lane DCDR ckout SEL
	_rtd_part_outl(0x1800daa0, 31, 31, 0x1); //[APHY para]  R-Lane DFE Ckin SEL.
	_rtd_part_outl(0x1800daac, 21, 20, APHY_Para_tmds_1[tmds_timing].ACDR_R_11_131); //[APHY para]  R-Lane RLSEL_LE1_2
	_rtd_part_outl(0x1800daac, 23, 22, 0x0); //[APHY para]  R-Lane RLSEL_NC_2
	_rtd_part_outl(0x1800daac, 24, 24, 0x0); //[APHY para]  R-Lane POW_NC
	_rtd_part_outl(0x1800dab4, 4, 4, 0x1); //[APHY para]  CMU input clock source select:
	_rtd_part_outl(0x1800dab4, 7, 7, 0x1); //[APHY para]  CMU_CKXTAL_SEL
	_rtd_part_outl(0x1800dab8, 8, 8, 0x1); //[APHY para  CMU_SEL_CP
}

void DPHY_Init_Flow_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing)
{
	// DPHY init flow start
	_rtd_part_outl(0x1800dc88, 4, 4, 0x1); //[DPHY init flow]  reg_p0_b_fifo_en
	_rtd_part_outl(0x1800dc88, 12, 12, 0x1); //[DPHY init flow]  reg_p0_g_fifo_en
	_rtd_part_outl(0x1800dc88, 20, 20, 0x1); //[DPHY init flow]  reg_p0_r_fifo_en
	_rtd_part_outl(0x1800dc88, 28, 28, 0x1); //[DPHY init flow]  reg_p0_ck_fifo_en
	_rtd_part_outl(0x1800dd6c, 1, 1, 0x0); //[DPHY init flow]  reg_p0_pow_save_10b18b_sel
	_rtd_part_outl(0x1800dd7c, 5, 0, 0x0e); //[DPHY init flow]  reg_p0_dig_debug_sel
	//DPHY fld flow start
	_rtd_part_outl(0x1800dcb8, 10, 3, 0x08); //[DPHY fld flow]  reg_p0_b_divide_num
	_rtd_part_outl(0x1800dcbc, 11, 0, 0x0db); //[DPHY fld flow] reg_p0_b_lock_dn_limit
	_rtd_part_outl(0x1800dcbc, 27, 16, 0x0e1); //[DPHY fld flow] reg_p0_b_lock_up_limit
	_rtd_part_outl(0x1800dcc4, 19, 8, 0x0db); //[DPHY fld flow] reg_p0_b_coarse_lock_dn_limit
	_rtd_part_outl(0x1800dcc4, 31, 20, 0x0e1); //[DPHY fld flow] reg_p0_b_coarse_lock_up_limit
	_rtd_part_outl(0x1800dcc8, 1, 1, 0x1); //[DPHY fld flow]  reg_p0_b_fld_rstb
	_rtd_part_outl(0x1800dcd8, 10, 3, 0x08); //[DPHY fld flow]  reg_p0_g_divide_num
	_rtd_part_outl(0x1800dcdc, 11, 0, 0x0db); //[DPHY fld flow] reg_p0_g_lock_dn_limit
	_rtd_part_outl(0x1800dcdc, 27, 16, 0x0e1); //[DPHY fld flow] reg_p0_g_lock_up_limit
	_rtd_part_outl(0x1800dce4, 19, 8, 0x0db); //[DPHY fld flow] reg_p0_g_coarse_lock_dn_limit
	_rtd_part_outl(0x1800dce4, 31, 20, 0x0e1); //[DPHY fld flow] reg_p0_g_coarse_lock_up_limit
	_rtd_part_outl(0x1800dce8, 1, 1, 0x1); //[DPHY fld flow]  reg_p0_g_fld_rstb
	_rtd_part_outl(0x1800dcf4, 10, 3, 0x08); //[DPHY fld flow]  reg_p0_r_divide_num
	_rtd_part_outl(0x1800dcf8, 11, 0, 0x0db); //[DPHY fld flow] reg_p0_r_lock_dn_limit
	_rtd_part_outl(0x1800dcf8, 27, 16, 0x0e1); //[DPHY fld flow] reg_p0_r_lock_up_limit
	_rtd_part_outl(0x1800dd00, 19, 8, 0x0db); //[DPHY fld flow] reg_p0_r_coarse_lock_dn_limit
	_rtd_part_outl(0x1800dd00, 31, 20, 0x0e1); //[DPHY fld flow] reg_p0_r_coarse_lock_up_limit
	_rtd_part_outl(0x1800dd04, 1, 1, 0x1); //[DPHY fld flow]  reg_p0_r_fld_rstb
	_rtd_part_outl(0x1800dd10, 10, 3, 0x08); //[DPHY fld flow]  reg_p0_ck_divide_num
	_rtd_part_outl(0x1800dd14, 11, 0, 0x0db); //[DPHY fld flow] reg_p0_ck_lock_dn_limit
	_rtd_part_outl(0x1800dd14, 27, 16, 0x0e1); //[DPHY fld flow] reg_p0_ck_lock_up_limit
	_rtd_part_outl(0x1800dd1c, 19, 8, 0x0db); //[DPHY fld flow] reg_p0_ck_coarse_lock_dn_limit
	_rtd_part_outl(0x1800dd1c, 31, 20, 0x0e1); //[DPHY fld flow] reg_p0_ck_coarse_lock_up_limit
	_rtd_part_outl(0x1800dd20, 1, 1, 0x1); //[DPHY fld flow]  reg_p0_ck_fld_rstb
}

void APHY_Init_Flow_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing)
{
	// APHY init flow start
	_rtd_part_outl(0x1800dd3c, 9, 9, 0x1); //B //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_rtd_part_outl(0x1800dd48, 9, 9, 0x1); //G //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_rtd_part_outl(0x1800dd54, 9, 9, 0x1); //R //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_rtd_part_outl(0x1800dd60, 9, 9, 0x1); //K //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_rtd_part_outl(0x1800da18, 1, 1, 0x0); //[APHY init flow]  REG_CK_LATCH Before PSM, latch the register value of PHY. 0->1: Latch
	_rtd_part_outl(0x1800da30, 6, 6, 0x0); //[APHY init flow]  CK-Lane FAST_SW_EN
	_rtd_part_outl(0x1800da30, 7, 7, 0x0); //[APHY init flow]  CK-Lane FAST_SW_DELAY_EN
	_rtd_part_outl(0x1800da38, 17, 17, 0x1); //[APHY init flow] 	CK-Lane BBPD_RSTB
	_rtd_part_outl(0x1800da40, 16, 16, 0x0); //[APHY init flow]  CK-Lane DCDR_RSTB
	_rtd_part_outl(0x1800da40, 17, 17, 0x0); //[APHY init flow]  CK-Lane PI_DIV rstb
	_rtd_part_outl(0x1800da40, 24, 24, 0x0); //[APHY init flow]  CK-Lane FKP_RSTB (FKP:Fast KP)
	_rtd_part_outl(0x1800da40, 28, 28, 0x0); //[APHY init flow]  CK-Lane FKT EN
	_rtd_part_outl(0x1800da44, 0, 0, 0x1); //[APHY init flow]  CK-Lane ACDR_RSTB_PFD
	_rtd_part_outl(0x1800da44, 1, 1, 0x0); //[APHY init flow]  CK-Lane ACDR_RSTB_CLK_FLD
	_rtd_part_outl(0x1800da44, 16, 16, 0x1); //[APHY init flow]  CK-Lane ACDR_RSTB_DIV_IQ
	_rtd_part_outl(0x1800da48, 24, 24, 0x0); //[APHY init flow]  CK-Lane RSTB_BBPD_KP_KI
	_rtd_part_outl(0x1800da4c, 11, 11, 0x1); //[APHY init flow]  CK-Lane RSTB_PRESCALER
	_rtd_part_outl(0x1800da4c, 12, 12, 0x1); //[APHY init flow]  CK-Lane RSTB_M_DIV
	_rtd_part_outl(0x1800da50, 6, 6, 0x0); //[APHY init flow]  	B-Lane FAST_SW_EN
	_rtd_part_outl(0x1800da50, 7, 7, 0x0); //[APHY init flow]  	B-Lane FAST_SW_DELAY_EN
	_rtd_part_outl(0x1800da58, 17, 17, 0x1); //[APHY init flow] 	B-Lane BBPD_RSTB
	_rtd_part_outl(0x1800da60, 16, 16, 0x1); //[APHY init flow]  B-Lane DCDR_RSTB
	_rtd_part_outl(0x1800da60, 17, 17, 0x1); //[APHY init flow]  B-Lane PI_DIV rstb
	_rtd_part_outl(0x1800da60, 24, 24, 0x0); //[APHY init flow]  B-Lane FKP_RSTB (FKP:Fast KP)
	_rtd_part_outl(0x1800da60, 28, 28, 0x0); //[APHY init flow]  B-Lane FKT EN
	_rtd_part_outl(0x1800da64, 0, 0, 0x0); //[APHY init flow]  B-Lane ACDR_RSTB_PFD
	_rtd_part_outl(0x1800da64, 1, 1, 0x0); //[APHY init flow]  B-Lane ACDR_RSTB_CLK_FLD
	_rtd_part_outl(0x1800da64, 16, 16, 0x1); //[APHY init flow]  B-Lane ACDR_RSTB_DIV_IQ
	_rtd_part_outl(0x1800da68, 24, 24, 0x1); //[APHY init flow]  B-Lane RSTB_BBPD_KP_KI
	_rtd_part_outl(0x1800da6c, 11, 11, 0x0); //[APHY init flow]  B-Lane RSTB_PRESCALER
	_rtd_part_outl(0x1800da6c, 12, 12, 0x0); //[APHY init flow]  B-Lane RSTB_M_DIV
	_rtd_part_outl(0x1800da70, 6, 6, 0x0); //[APHY init flow]  	G-Lane FAST_SW_EN
	_rtd_part_outl(0x1800da70, 7, 7, 0x0); //[APHY init flow]  	G-Lane FAST_SW_DELAY_EN
	_rtd_part_outl(0x1800da78, 17, 17, 0x1); //[APHY init flow] 	G-Lane BBPD_RSTB
	_rtd_part_outl(0x1800da80, 16, 16, 0x1); //[APHY init flow]  G-Lane DCDR_RSTB
	_rtd_part_outl(0x1800da80, 17, 17, 0x1); //[APHY init flow]  G-Lane PI_DIV rstb
	_rtd_part_outl(0x1800da80, 24, 24, 0x0); //[APHY init flow]  G-Lane FKP_RSTB (FKP:Fast KP)
	_rtd_part_outl(0x1800da80, 28, 28, 0x0); //[APHY init flow]  G-Lane FKT EN
	_rtd_part_outl(0x1800da84, 0, 0, 0x0); //[APHY init flow]  G-Lane ACDR_RSTB_PFD
	_rtd_part_outl(0x1800da84, 1, 1, 0x0); //[APHY init flow]  G-Lane ACDR_RSTB_CLK_FLD
	_rtd_part_outl(0x1800da84, 16, 16, 0x1); //[APHY init flow]  G-Lane ACDR_RSTB_DIV_IQ
	_rtd_part_outl(0x1800da88, 24, 24, 0x1); //[APHY init flow]  G-Lane RSTB_BBPD_KP_KI
	_rtd_part_outl(0x1800da8c, 11, 11, 0x0); //[APHY init flow]  G-Lane RSTB_PRESCALER
	_rtd_part_outl(0x1800da8c, 12, 12, 0x0); //[APHY init flow]  G-Lane RSTB_M_DIV
	_rtd_part_outl(0x1800da90, 6, 6, 0x0); //[APHY init flow]  	R-Lane FAST_SW_EN
	_rtd_part_outl(0x1800da90, 7, 7, 0x0); //[APHY init flow]  	R-Lane FAST_SW_DELAY_EN
	_rtd_part_outl(0x1800da98, 17, 17, 0x1); //[APHY init flow] 	R-Lane BBPD_RSTB
	_rtd_part_outl(0x1800daa0, 16, 16, 0x1); //[APHY init flow]  R-Lane DCDR_RSTB
	_rtd_part_outl(0x1800daa0, 17, 17, 0x1); //[APHY init flow]  R-Lane PI_DIV rstb
	_rtd_part_outl(0x1800daa0, 24, 24, 0x0); //[APHY init flow]  R-Lane FKP_RSTB (FKP:Fast KP)
	_rtd_part_outl(0x1800daa0, 28, 28, 0x0); //[APHY init flow]  R-Lane FKT EN
	_rtd_part_outl(0x1800daa4, 0, 0, 0x0); //[APHY init flow]  R-Lane ACDR_RSTB_PFD
	_rtd_part_outl(0x1800daa4, 1, 1, 0x0); //[APHY init flow]  R-Lane ACDR_RSTB_CLK_FLD
	_rtd_part_outl(0x1800daa4, 16, 16, 0x1); //[APHY init flow]  R-Lane ACDR_RSTB_DIV_IQ
	_rtd_part_outl(0x1800daa8, 24, 24, 0x1); //[APHY init flow]  R-Lane RSTB_BBPD_KP_KI
	_rtd_part_outl(0x1800daac, 11, 11, 0x0); //[APHY init flow]  R-Lane RSTB_PRESCALER
	_rtd_part_outl(0x1800daac, 12, 12, 0x0); //[APHY init flow]  R-Lane RSTB_M_DIV
	_rtd_part_outl(0x1800dd3c, 9, 9, 0x0); //B //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_rtd_part_outl(0x1800dd48, 9, 9, 0x0); //G //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_rtd_part_outl(0x1800dd54, 9, 9, 0x0); //R //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_rtd_part_outl(0x1800dd60, 9, 9, 0x0); //K //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	//APHY acdr flow start
	_rtd_part_outl(0x1800dafc, 0, 0, 0x0); //[APHY acdr flow]  P0_ACDR_B_ACDR_EN_AFN
	_rtd_part_outl(0x1800dafc, 1, 1, 0x0); //[APHY acdr flow]  P0_ACDR_G_ACDR_EN_AFN
	_rtd_part_outl(0x1800dafc, 2, 2, 0x0); //[APHY acdr flow]  P0_ACDR_R_ACDR_EN_AFN
	_rtd_part_outl(0x1800dafc, 3, 3, 0x0); //[APHY acdr flow]  P0_ACDR_CK_ACDR_EN_AFN
	_rtd_part_outl(0x1800dafc, 4, 4, 0x1); //[APHY acdr flow]  P0_B_DEMUX_RSTB
	_rtd_part_outl(0x1800dafc, 5, 5, 0x1); //[APHY acdr flow]  P0_G_DEMUX_RSTB
	_rtd_part_outl(0x1800dafc, 6, 6, 0x1); //[APHY acdr flow]  P0_R_DEMUX_RSTB
	_rtd_part_outl(0x1800dafc, 7, 7, 0x1); //[APHY acdr flow]  P0_CK_DEMUX_RSTB
	_rtd_part_outl(0x1800da3c, 7, 7, 0x0); //[APHY acdr flow]  		CK-Lane DFE_ADAPT_EN
	_rtd_part_outl(0x1800da5c, 7, 7, 0x1); //[APHY acdr flow]  		B-Lane DFE_ADAPT_EN
	_rtd_part_outl(0x1800da7c, 7, 7, 0x1); //[APHY acdr flow]  		G-Lane DFE_ADAPT_EN
	_rtd_part_outl(0x1800da9c, 7, 7, 0x1); //[APHY acdr flow]  R-Lane DFE_ADAPT_EN
}

void DCDR_settings_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing)
{
	_rtd_part_outl(0x1800da58, 17, 17, 0); //BBPD_rstb
	_rtd_part_outl(0x1800da78, 17, 17, 0); //BBPD_rstb
	_rtd_part_outl(0x1800da98, 17, 17, 0); //BBPD_rstb
	_rtd_part_outl(0x1800da58, 17, 17, 1); //BBPD_rstb
	_rtd_part_outl(0x1800da78, 17, 17, 1); //BBPD_rstb
	_rtd_part_outl(0x1800da98, 17, 17, 1); //BBPD_rstb
	//#====================================== CMU RESET
	_rtd_part_outl(0x1800dafc, 7, 4, 0x0);
	_rtd_part_outl(0x1800dafc, 7, 4, 0xf);
	//#====================================== phase I
	_rtd_part_outl(0x1800dd20, 27, 25, 4); //Mark2_Modify; CK-Lane SEL_BAND_V2I
	//#5000
	//#===================================== phase IV : set pow_save_bypass == 0
	//#====================================== cdr reset release
}

void DCDR_settings_tmds_2_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing)
{
	_rtd_part_outl(0x1800dc88, 4, 4, 0x0); //DCDR 5.94G, reg_p0_b/g/r_clk_div2_en
	_rtd_part_outl(0x1800dc88, 12, 12, 0x0); //DCDR 5.94G, reg_p0_b/g/r_clk_div2_en
	_rtd_part_outl(0x1800dc88, 20, 20, 0x0); //DCDR 5.94G, reg_p0_b/g/r_clk_div2_en
	_rtd_part_outl(0x1800dc88, 28, 28, 0x0); //DCDR 5.94G, reg_p0_b/g/r_clk_div2_en
	_rtd_part_outl(0x1800dc88, 4, 4, 0x1); //DCDR 5.94G, reg_p0_b/g/r_clk_div2_en
	_rtd_part_outl(0x1800dc88, 12, 12, 0x1); //DCDR 5.94G, reg_p0_b/g/r_clk_div2_en
	_rtd_part_outl(0x1800dc88, 20, 20, 0x1); //DCDR 5.94G, reg_p0_b/g/r_clk_div2_en
	_rtd_part_outl(0x1800dc88, 28, 28, 0x1); //DCDR 5.94G, reg_p0_b/g/r_clk_div2_en
	_rtd_part_outl(0x1800dc00, 7, 0, 0x0);
	_rtd_part_outl(0x1800dc00, 7, 0, 0xff);
}

void Koffset_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing)
{
	_rtd_part_outl(0x1800dd04, 0, 0, 0); //R finetunstart off
	_rtd_part_outl(0x1800dce8, 0, 0, 0); //G finetunstart off
	_rtd_part_outl(0x1800dcc8, 0, 0, 0); //B finetunstart off
	_rtd_part_outl(0x1800dd20, 0, 0, 0); //CK finetunstart off
	//========================================= STEP1 Data_Even KOFF =================================================
	_rtd_part_outl(0x1800da58, 28, 28, 1); //#B  DFE_TAP0_ICOM_EN=1
	_rtd_part_outl(0x1800da5c, 6, 3, 0); //#B  DFE_TAP_EN[4:1] = 0
	_rtd_part_outl(0x1800da78, 28, 28, 1); //#G  DFE_TAP0_ICOM_EN=1
	_rtd_part_outl(0x1800da7c, 6, 3, 0); //#G  DFE_TAP_EN[4:1] = 0
	_rtd_part_outl(0x1800da98, 28, 28, 1); //#R  DFE_TAP0_ICOM_EN=1
	_rtd_part_outl(0x1800da9c, 6, 3, 0); //#R  DFE_TAP_EN[4:1] = 0
	_rtd_part_outl(0x1800dc44, 31, 0, 0x0282cf00); //B_KOFF rstn,  //[26:20] timout = 0101000, [19:14] divisor = 001000, [11:8] delay count = 11
	_rtd_part_outl(0x1800dc54, 31, 0, 0x0282cf00); //G_KOFF rstn
	_rtd_part_outl(0x1800dc64, 31, 0, 0x0282cf00); //R_KOFF rstn
	_rtd_part_outl(0x1800dc44, 0, 0, 1); //Release B_KOFF rstn
	_rtd_part_outl(0x1800dc54, 0, 0, 1); //Release G_KOFF rstn
	_rtd_part_outl(0x1800dc64, 0, 0, 1); //Release R_KOFF rstn
	//===BLANE KOFF, check p0_b_offset_da_ok_ro ,0x180bcc44[28]
	_rtd_part_outl(0x1800dc4c, 28, 28, 0); //manaul_do=0, 0:Manual; 1:Auto
	_rtd_part_outl(0x1800dc4c, 18, 18, 0); //manaul_de=0, 0:Manual; 1:Auto
	_rtd_part_outl(0x1800dc50, 28, 28, 0); //manaul_eq=0, 0:Manual; 1:Auto
	_rtd_part_outl(0x1800dc50, 18, 18, 0); //manaul_eo=0, 0:Manual; 1:Auto
	_rtd_part_outl(0x1800dc50, 8, 8, 0); //manaul_ee=0, 0:Manual; 1:Auto
	_rtd_part_outl(0x1800da50, 0, 0, 1); //INOFF_EN =0 ->1
	_rtd_part_outl(0x1800da58, 0, 0, 0); //REG_B_9[0] POW_CMFB_LEQ 1->0
	_rtd_part_outl(0x1800da54, 22, 22, 0); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  1->0
	_rtd_part_outl(0x1800da54, 23, 23, 0); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  1->0
	_rtd_part_outl(0x1800da54, 24, 24, 0); //REG_B_8[0] POW_LEQ:  1->0
	//B-Lane z0_ok
	_rtd_part_outl(0x1800dc50, 19, 19, 1); //[19]z0_ok_eo=1
	_rtd_part_outl(0x1800dc50, 9, 9, 1); //[9] z0_ok_ee=1
	_rtd_part_outl(0x1800dc4c, 29, 29, 1); //[29]z0_ok_do=1
	_rtd_part_outl(0x1800dc4c, 19, 19, 1); //[19]z0_ok_de=1
	_rtd_part_outl(0x1800dc4c, 21, 21, 1); //[20]offset_pc_do=1
	_rtd_part_outl(0x1800dc4c, 11, 11, 1); //[20]offset_pc_de=1
	_rtd_part_outl(0x1800dc50, 1, 1, 1); //[20]offset_pc_ee=1
	_rtd_part_outl(0x1800dc50, 11, 11, 1); //[20]offset_pc_eo=1
	_rtd_part_outl(0x1800dc50, 21, 21, 1); //[21]offfset_pc_eq=1
	//Enable B-Lane DCVS KOFF
	_rtd_part_outl(0x1800dc4c, 10, 10, 1); //[10]offset_en_de=1
	_rtd_part_outl(0x1800dc4c, 20, 20, 1); //[20]offset_en_do=1
	_rtd_part_outl(0x1800dc50, 0, 0, 1); //[20]offset_en_ee=1
	_rtd_part_outl(0x1800dc50, 10, 10, 1); //[20]offset_en_eo=1
	//Check Offset cal. OK Check
	_rtd_part_outl(0x1800dd7c, 5, 0, 0x8); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_rtd_part_outl(0x1800dc44, 7, 4, 0x9); //reg_p0_b_offset_coef_sel
	HDMI_PHY_INFO("DE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 27, 27));
	HDMI_PHY_INFO("DO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 26, 26));
	HDMI_PHY_INFO("EE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 25, 25));
	HDMI_PHY_INFO("EOOffset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 24, 24));
	HDMI_PHY_INFO("DATA OFFSET OK = %d\n",_rtd_part_inl(0x1800dc44, 28, 28));
	//Disable B-Lane DCVS KOFF
	_rtd_part_outl(0x1800dc4c, 10, 10, 0); //[10]offset_en_de=1->0
	_rtd_part_outl(0x1800dc4c, 20, 20, 0); //[20]offset_en_do=1->0
	_rtd_part_outl(0x1800dc50, 0, 0, 0); //[20]offset_en_ee=1->0
	_rtd_part_outl(0x1800dc50, 10, 10, 0); //[20]offset_en_eo=1->0
	//B-Lane Koffset result READ  *******************************************************************
	_rtd_part_outl(0x1800dc44, 7, 4, 0); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Data_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	_rtd_part_outl(0x1800dc44, 7, 4, 1); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Data_Odd_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	_rtd_part_outl(0x1800dc44, 7, 4, 2); //reg_p0_b_offset_coef_sel[4:0]//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	_rtd_part_outl(0x1800dc44, 7, 4, 3); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	//===G LANE KOFF, check p0_b_offset_da_ok_ro ,0x180bcc54[28]
	_rtd_part_outl(0x1800dc5c, 28, 28, 0); //manaul_do=0
	_rtd_part_outl(0x1800dc5c, 18, 18, 0); //manaul_de=0
	_rtd_part_outl(0x1800dc60, 28, 28, 0); //manaul_eq=0
	_rtd_part_outl(0x1800dc60, 18, 18, 0); //manaul_eo=0
	_rtd_part_outl(0x1800dc60, 8, 8, 0); //manaul_ee=0
	_rtd_part_outl(0x1800da70, 0, 0, 1); //INOFF_EN =0 ->1
	_rtd_part_outl(0x1800da78, 0, 0, 0); //REG_B_9[0] POW_CMFB_LEQ 1->0
	_rtd_part_outl(0x1800da74, 22, 22, 0); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  1->0
	_rtd_part_outl(0x1800da74, 23, 23, 0); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  1->0
	_rtd_part_outl(0x1800da74, 24, 24, 0); //REG_B_8[0] POW_LEQ:  1->0
	//G-Lane z0_ok
	_rtd_part_outl(0x1800dc60, 19, 19, 1); //[19]z0_ok_eo=1
	_rtd_part_outl(0x1800dc60, 9, 9, 1); //[9] z0_ok_ee=1
	_rtd_part_outl(0x1800dc5c, 29, 29, 1); //[29]z0_ok_do=1
	_rtd_part_outl(0x1800dc5c, 19, 19, 1); //[19]z0_ok_de=1
	_rtd_part_outl(0x1800dc5c, 21, 21, 1); //[20]offset_pc_do=1
	_rtd_part_outl(0x1800dc5c, 11, 11, 1); //[20]offset_pc_de=1
	_rtd_part_outl(0x1800dc60, 1, 1, 1); //[20]offset_pc_ee=1
	_rtd_part_outl(0x1800dc60, 11, 11, 1); //[20]offset_pc_eo=1
	_rtd_part_outl(0x1800dc60, 21, 21, 1); //[21]offfset_pc_eq=1
	//Enable G-Lane DCVS KOFF
	_rtd_part_outl(0x1800dc5c, 10, 10, 1); //[10]offset_en_de=1
	_rtd_part_outl(0x1800dc5c, 20, 20, 1); //[20]offset_en_do=1
	_rtd_part_outl(0x1800dc60, 0, 0, 1); //[20]offset_en_ee=1
	_rtd_part_outl(0x1800dc60, 10, 10, 1); //[20]offset_en_eo=1
	//G-Lane DCVS Offset cal. OK Check
	_rtd_part_outl(0x1800dd7c, 5, 0, 0xA); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_rtd_part_outl(0x1800dc54, 7, 4, 0x9); //reg_p0_g_offset_coef_sel
	HDMI_PHY_INFO("DE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 27, 27));
	HDMI_PHY_INFO("DO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 26, 26));
	HDMI_PHY_INFO("EE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 25, 25));
	HDMI_PHY_INFO("EO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 24, 24));
	HDMI_PHY_INFO("DATA OFFSET OK = %d\n",_rtd_part_inl(0x1800dc54, 28, 28));
	//Disable G-Lane DCVS KOFF
	_rtd_part_outl(0x1800dc5c, 10, 10, 0); //[10]offset_en_de=0
	_rtd_part_outl(0x1800dc5c, 20, 20, 0); //[20]offset_en_do=0
	_rtd_part_outl(0x1800dc60, 0, 0, 0); //[20]offset_en_ee=0
	_rtd_part_outl(0x1800dc60, 10, 10, 0); //[20]offset_en_eo=0
	//G-Lane DCVS Offset Result
	_rtd_part_outl(0x1800dc54, 7, 4, 0); //reg_p0_g_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Data_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	_rtd_part_outl(0x1800dc54, 7, 4, 1); //reg_p0_g_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Data_Odd_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	_rtd_part_outl(0x1800dc54, 7, 4, 2); //reg_p0_g_offset_coef_sel[4:0]//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	_rtd_part_outl(0x1800dc54, 7, 4, 3); //reg_p0_g_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	//===R LANE KOFF
	_rtd_part_outl(0x1800dc6c, 28, 28, 0); //manaul_do=0
	_rtd_part_outl(0x1800dc6c, 18, 18, 0); //manaul_de=0
	_rtd_part_outl(0x1800dc70, 28, 28, 0); //manaul_eq=0
	_rtd_part_outl(0x1800dc70, 18, 18, 0); //manaul_eo=0
	_rtd_part_outl(0x1800dc70, 8, 8, 0); //manaul_ee=0
	_rtd_part_outl(0x1800da90, 0, 0, 1); //INOFF_EN =0 ->1
	_rtd_part_outl(0x1800da98, 0, 0, 0); //REG_B_9[0] POW_CMFB_LEQ 1->0
	_rtd_part_outl(0x1800da94, 22, 22, 0); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  1->0
	_rtd_part_outl(0x1800da94, 23, 23, 0); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  1->0
	_rtd_part_outl(0x1800da94, 24, 24, 0); //REG_B_8[0] POW_LEQ:  1->0
	//R-Lane z0_ok
	_rtd_part_outl(0x1800dc70, 19, 19, 1); //[19]z0_ok_eo=1
	_rtd_part_outl(0x1800dc70, 9, 9, 1); //[9] z0_ok_ee=1
	_rtd_part_outl(0x1800dc6c, 29, 29, 1); //[29]z0_ok_do=1
	_rtd_part_outl(0x1800dc6c, 19, 19, 1); //[19]z0_ok_de=1
	_rtd_part_outl(0x1800dc6c, 21, 21, 1); //[20]offset_pc_do=1
	_rtd_part_outl(0x1800dc6c, 11, 11, 1); //[20]offset_pc_de=1
	_rtd_part_outl(0x1800dc70, 1, 1, 1); //[20]offset_pc_ee=1
	_rtd_part_outl(0x1800dc70, 11, 11, 1); //[20]offset_pc_eo=1
	_rtd_part_outl(0x1800dc70, 21, 21, 1); //[21]offfset_pc_eq=1
	//Enable R-Lane DCVS KOFF
	_rtd_part_outl(0x1800dc6c, 10, 10, 1); //[10]offset_en_de=1
	_rtd_part_outl(0x1800dc6c, 20, 20, 1); //[20]offset_en_do=1
	_rtd_part_outl(0x1800dc70, 0, 0, 1); //[20]offset_en_ee=1
	_rtd_part_outl(0x1800dc70, 10, 10, 1); //[20]offset_en_eo=1
	//R-Lane DCVS Offset cal. OK Check
	_rtd_part_outl(0x1800dd7c, 5, 0, 0xC); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_rtd_part_outl(0x1800dc64, 7, 4, 0x9); //reg_p0_b_offset_coef_sel
	HDMI_PHY_INFO("DE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 27, 27));
	HDMI_PHY_INFO("DO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 26, 26));
	HDMI_PHY_INFO("EE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 25, 25));
	HDMI_PHY_INFO("EO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 24, 24));
	HDMI_PHY_INFO("DATA OFFSET OK = %d\n",_rtd_part_inl(0x1800dc64, 28, 28));
	//Disable R-Lane DCVS KOFF
	_rtd_part_outl(0x1800dc6c, 10, 10, 0); //[10]offset_en_de=0
	_rtd_part_outl(0x1800dc6c, 20, 20, 0); //[20]offset_en_do=0
	_rtd_part_outl(0x1800dc70, 0, 0, 0); //[20]offset_en_ee=0
	_rtd_part_outl(0x1800dc70, 10, 10, 0); //[20]offset_en_eo=0
	//R-Lane LEQ & DCVS Offset Result
	_rtd_part_outl(0x1800dc64, 7, 4, 0); //reg_p0_r_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Data_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	_rtd_part_outl(0x1800dc64, 7, 4, 1); //reg_p0_r_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Data_Odd_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	_rtd_part_outl(0x1800dc64, 7, 4, 2); //reg_p0_r_offset_coef_sel[4:0]//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	_rtd_part_outl(0x1800dc64, 7, 4, 3); //reg_p0_r_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	//========================================= STEP2 LEQ KOFF =================================================
	_rtd_part_outl(0x1800da54, 22, 22, Koffset_tmds_1[tmds_timing].B_7_163); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  0->1
	_rtd_part_outl(0x1800da54, 23, 23, 1); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  0->1
	_rtd_part_outl(0x1800da54, 24, 24, 1); //REG_B_8[0] POW_LEQ:  0->1
	_rtd_part_outl(0x1800da58, 0, 0, 1); //REG_B_9[0] POW_CMFB_LEQ 0->1
	_rtd_part_outl(0x1800dc50, 19, 19, 0); //[19]z0_ok_eo=0
	_rtd_part_outl(0x1800dc50, 9, 9, 0); //[9] z0_ok_ee=0
	_rtd_part_outl(0x1800dc4c, 29, 29, 0); //[19]z0_ok_do=0
	_rtd_part_outl(0x1800dc50, 20, 20, 1); //[21]offset_en_eq=0->1
	HDMI_PHY_INFO("p0_b_offset_eq_ok_ro	EQ OFFSET OK = %d\n",_rtd_part_inl(0x1800dc44, 30, 30));
	_rtd_part_outl(0x1800dc50, 20, 20, 0); //[20]offset_en_eq=0
	//B-Lane LEQ Offset Result
	_rtd_part_outl(0x1800dc44, 7, 4, 8); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("EQ_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	//===G LANE KOFF LEQ
	_rtd_part_outl(0x1800da74, 22, 22, Koffset_tmds_1[tmds_timing].G_7_177); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  0->1
	_rtd_part_outl(0x1800da74, 23, 23, 1); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  0->1
	_rtd_part_outl(0x1800da74, 24, 24, 1); //REG_B_8[0] POW_LEQ:  0->1
	_rtd_part_outl(0x1800da78, 0, 0, 1); //REG_B_9[0] POW_CMFB_LEQ 0->1
	_rtd_part_outl(0x1800dc60, 19, 19, 0); //[19]z0_ok_eo=0
	_rtd_part_outl(0x1800dc60, 9, 9, 0); //[9] z0_ok_ee=0
	_rtd_part_outl(0x1800dc5c, 29, 29, 0); //[19]z0_ok_do=0
	_rtd_part_outl(0x1800dc60, 20, 20, 1); //[21]offset_en_eq=0->1
	HDMI_PHY_INFO("p0_g_offset_eq_ok_ro EQ OFFSET OK = %d\n",_rtd_part_inl(0x1800dc54, 30, 30));
	_rtd_part_outl(0x1800dc60, 20, 20, 0); //[20]offset_en_eq=0
	//G-Lane LEQ Offset Result
	_rtd_part_outl(0x1800dc54, 7, 4, 8); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("EQ_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	//===R LANE KOFF LEQ
	_rtd_part_outl(0x1800da94, 22, 22, Koffset_tmds_1[tmds_timing].R_7_191); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  0->1
	_rtd_part_outl(0x1800da94, 23, 23, 1); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  0->1
	_rtd_part_outl(0x1800da94, 24, 24, 1); //REG_B_8[0] POW_LEQ:  0->1
	_rtd_part_outl(0x1800da98, 0, 0, 1); //REG_B_9[0] POW_CMFB_LEQ 0->1
	_rtd_part_outl(0x1800dc70, 19, 19, 0); //[19]z0_ok_eo=0
	_rtd_part_outl(0x1800dc70, 9, 9, 0); //[9] z0_ok_ee=0
	_rtd_part_outl(0x1800dc6c, 29, 29, 0); //[19]z0_ok_do=0
	_rtd_part_outl(0x1800dc70, 20, 20, 1); //[21]offset_en_eq=0->1
	HDMI_PHY_INFO("p0_r_offset_eq_ok_ro EQ OFFSET OK = %d\n",_rtd_part_inl(0x1800dc64, 30, 30));
	_rtd_part_outl(0x1800dc70, 20, 20, 0); //[20]offset_en_eq=0
	//R-Lane LEQ Offset Result
	_rtd_part_outl(0x1800dc64, 7, 4, 8); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("EQ_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	//========================================= STEP3 All DCVS KOFF ==
	//B-LANE
	_rtd_part_outl(0x1800dc4c, 10, 10, 1); //[10]offset_en_de=1
	_rtd_part_outl(0x1800dc4c, 20, 20, 1); //[20]offset_en_do=1
	_rtd_part_outl(0x1800dc50, 0, 0, 1); //[20]offset_en_ee=1
	_rtd_part_outl(0x1800dc50, 10, 10, 1); //[20]offset_en_eo=1
	ScalerTimer_DelayXms(5);
	//Check Offset cal. OK Check
	_rtd_part_outl(0x1800dd7c, 5, 0, 0x8); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_rtd_part_outl(0x1800dc44, 7, 4, 0x9); //reg_p0_b_offset_coef_sel
	HDMI_PHY_INFO("DE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 27, 27));
	HDMI_PHY_INFO("DO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 26, 26));
	HDMI_PHY_INFO("EE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 25, 25));
	HDMI_PHY_INFO("EOOffset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 24, 24));
	HDMI_PHY_INFO("DATA OFFSET OK = %d\n",_rtd_part_inl(0x1800dc44, 28, 28));
	//Disable B-Lane DCVS KOFF
	_rtd_part_outl(0x1800dc4c, 10, 10, 0); //[10]offset_en_de=1->0
	_rtd_part_outl(0x1800dc4c, 20, 20, 0); //[20]offset_en_do=1->0
	_rtd_part_outl(0x1800dc50, 0, 0, 0); //[20]offset_en_ee=1->0
	_rtd_part_outl(0x1800dc50, 10, 10, 0); //[20]offset_en_eo=1->0
	//===G LANE KOFF Data/Edge
	_rtd_part_outl(0x1800dc5c, 10, 10, 1); //[10]offset_en_de=1
	_rtd_part_outl(0x1800dc5c, 20, 20, 1); //[20]offset_en_do=1
	_rtd_part_outl(0x1800dc60, 0, 0, 1); //[20]offset_en_ee=1
	_rtd_part_outl(0x1800dc60, 10, 10, 1); //[20]offset_en_eo=1
	ScalerTimer_DelayXms(5);
	//G-Lane DCVS Offset cal. OK Check
	_rtd_part_outl(0x1800dd7c, 5, 0, 0xA); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_rtd_part_outl(0x1800dc54, 7, 4, 0x9); //reg_p0_g_offset_coef_sel
	HDMI_PHY_INFO("DE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 27, 27));
	HDMI_PHY_INFO("DO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 26, 26));
	HDMI_PHY_INFO("EE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 25, 25));
	HDMI_PHY_INFO("EO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 24, 24));
	HDMI_PHY_INFO("DATA OFFSET OK = %d\n",_rtd_part_inl(0x1800dc54, 28, 28));
	//G-LANE
	_rtd_part_outl(0x1800dc5c, 10, 10, 0); //[10]offset_en_de=0
	_rtd_part_outl(0x1800dc5c, 20, 20, 0); //[20]offset_en_do=0
	_rtd_part_outl(0x1800dc60, 0, 0, 0); //[20]offset_en_ee=0
	_rtd_part_outl(0x1800dc60, 10, 10, 0); //[20]offset_en_eo=0
	//===R LANE KOFF Data/Edge
	_rtd_part_outl(0x1800dc6c, 10, 10, 1); //[10]offset_en_de=1
	_rtd_part_outl(0x1800dc6c, 20, 20, 1); //[20]offset_en_do=1
	_rtd_part_outl(0x1800dc70, 0, 0, 1); //[20]offset_en_ee=1
	_rtd_part_outl(0x1800dc70, 10, 10, 1); //[20]offset_en_eo=1
	ScalerTimer_DelayXms(5);
	//R-Lane DCVS Offset cal. OK Check
	_rtd_part_outl(0x1800dd7c, 5, 0, 0xC); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_rtd_part_outl(0x1800dc64, 7, 4, 0x9); //reg_p0_r_offset_coef_sel
	HDMI_PHY_INFO("DE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 27, 27));
	HDMI_PHY_INFO("DO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 26, 26));
	HDMI_PHY_INFO("EE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 25, 25));
	HDMI_PHY_INFO("EO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 24, 24));
	HDMI_PHY_INFO("DATA OFFSET OK = %d\n",_rtd_part_inl(0x1800dc64, 28, 28));
	//Disable R-Lane DCVS KOFF
	_rtd_part_outl(0x1800dc6c, 10, 10, 0); //[10]offset_en_de=0
	_rtd_part_outl(0x1800dc6c, 20, 20, 0); //[20]offset_en_do=0
	_rtd_part_outl(0x1800dc70, 0, 0, 0); //[20]offset_en_ee=0
	_rtd_part_outl(0x1800dc70, 10, 10, 0); //[20]offset_en_eo=0
	//B-Lane LEQ & DCVS Offset Result
	_rtd_part_outl(0x1800dc44, 7, 4, 0); //reg_p0_b_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Data_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	_rtd_part_outl(0x1800dc44, 7, 4, 1); //reg_p0_b_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Data_Odd_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	_rtd_part_outl(0x1800dc44, 7, 4, 2); //reg_p0_b_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	_rtd_part_outl(0x1800dc44, 7, 4, 3); //reg_p0_b_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	_rtd_part_outl(0x1800dc44, 7, 4, 8); //reg_p0_b_offset_coef_sel[4:0]
	HDMI_PHY_INFO("EQ_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	//G-Lane LEQ & DCVS Offset Result
	_rtd_part_outl(0x1800dc54, 7, 4, 0); //reg_p0_g_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Data_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	_rtd_part_outl(0x1800dc54, 7, 4, 1); //reg_p0_g_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Data_Odd_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	_rtd_part_outl(0x1800dc54, 7, 4, 2); //reg_p0_g_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	_rtd_part_outl(0x1800dc54, 7, 4, 3); //reg_p0_g_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	_rtd_part_outl(0x1800dc54, 7, 4, 8); //reg_p0_g_offset_coef_sel[4:0]
	HDMI_PHY_INFO("EQ_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	//R-Lane LEQ & DCVS Offset Result
	_rtd_part_outl(0x1800dc64, 7, 4, 0); //reg_p0_r_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Data_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	_rtd_part_outl(0x1800dc64, 7, 4, 1); //reg_p0_r_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Data_Odd_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	_rtd_part_outl(0x1800dc64, 7, 4, 2); //reg_p0_r_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	_rtd_part_outl(0x1800dc64, 7, 4, 3); //reg_p0_r_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	_rtd_part_outl(0x1800dc64, 7, 4, 8); //reg_p0_r_offset_coef_sel[4:0]
	HDMI_PHY_INFO("EQ_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	//Input on
	_rtd_part_outl(0x1800da50, 0, 0, 0); //INOFF_EN =0 Input_on
	_rtd_part_outl(0x1800da70, 0, 0, 0); //INOFF_EN =0 Input_on
	_rtd_part_outl(0x1800da90, 0, 0, 0); //INOFF_EN =0 Input_on
	_rtd_part_outl(0x1800da58, 28, 28, 0); //#B  DFE_TAP0_ICOM_EN=0
	_rtd_part_outl(0x1800da5c, 6, 3, Koffset_tmds_1[tmds_timing].B_13_297); //#B  DFE_TAP_EN[4:1] = 0xf
	_rtd_part_outl(0x1800da78, 28, 28, 0); //#G  DFE_TAP0_ICOM_EN=0
	_rtd_part_outl(0x1800da7c, 6, 3, Koffset_tmds_1[tmds_timing].G_13_299); //#G  DFE_TAP_EN[4:1] = 0xf
	_rtd_part_outl(0x1800da98, 28, 28, 0); //#R  DFE_TAP0_ICOM_EN=0
	_rtd_part_outl(0x1800da9c, 6, 3, Koffset_tmds_1[tmds_timing].R_13_301); //#R  DFE_TAP_EN[4:1] = 0xf
	_rtd_part_outl(0x1800dd04, 0, 0, 0); //R finetunstart on
	_rtd_part_outl(0x1800dce8, 0, 0, 0); //G finetunstart on
	_rtd_part_outl(0x1800dcc8, 0, 0, 0); //B finetunstart on
}

void LEQ_VTH_Tap0_3_4_Adapt_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing)
{
	_rtd_part_outl(0x1800dd70, 3, 0, 0xf); //ck,r,g,b rstb_eq=1
	_rtd_part_outl(0x1800dd70, 7, 4, 0xf); //ck,r,g,b leq_en=1
	_rtd_part_outl(0x1800dd70, 15, 12, 0xf); //ck,r,g,b vth_en=1
	_rtd_part_outl(0x1800dd70, 11, 8, 0xf); //dfe_en=1
	_rtd_part_outl(0x1800dd70, 3, 0, 0x0); //ck,r,g,b rstb_eq=0
	_rtd_part_outl(0x1800dd70, 7, 4, 0x0); //ck,r,g,b leq_en=0
	_rtd_part_outl(0x1800dd70, 15, 12, 0x0); //ck,r,g,b vth_en=0
	_rtd_part_outl(0x1800dd70, 11, 8, 0x0); //dfe_en=0
	_rtd_part_outl(0x1800dd70, 3, 0, 0xf); //ck,r,g,b rstb_eq=1
	_rtd_part_outl(0x1800dd70, 7, 4, 0xf); //ck,r,g,b leq_en=1
	_rtd_part_outl(0x1800dd70, 15, 12, 0xf); //ck,r,g,b vth_en=1
	_rtd_part_outl(0x1800dd70, 11, 8, 0xf); //dfe_en=1
	//DFE_Adaptation
	//===CK LANE TAP0 & LEQ
	_rtd_part_outl(0x180b7ccc, 31, 0, 0x00007400); //Timer, set eqfe_en,servo_en,timer_en=1
	_rtd_part_outl(0x180b7cd0, 31, 31, 1); //leq1_inv=1
	_rtd_part_outl(0x180b7cd0, 30, 30, 1); //leq2_inv=0
	_rtd_part_outl(0x180b7cd4, 17, 15, 0); //LEQ_GAIN1=0
	_rtd_part_outl(0x180b7cd4, 14, 12, 0); //LEQ_GAIN2=0
	_rtd_part_outl(0x180b7cd8, 22, 21, 1); //LEQ1_trans_mode=1
	_rtd_part_outl(0x180b7cd8, 20, 19, 3); //LEQ2_trans_mode=3
	_rtd_part_outl(0x180b7cd8, 28, 26, 4); //tap0_gain=4
	_rtd_part_outl(0x180b7ccc, 26, 24, 5); //20200824_tap_timer=5
	_rtd_part_outl(0x180b7ccc, 20, 18, 5); //20200825_LEQ_timer=5
	_rtd_part_outl(0x180b7ccc, 23, 21, 5); //20200825_VTH_timer=5
	_rtd_part_outl(0x180b7ccc, 13, 13, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[tmds_timing].EQFE_EN_CK_26); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7ce8, 25, 25, 1); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7ce8, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=0
	_rtd_part_outl(0x180b7ce8, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=0
	_rtd_part_outl(0x180b7ce8, 28, 28, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_CK_6_0_30); //20200824_DFE_ADAP_EN_tap3=0
	_rtd_part_outl(0x180b7ce8, 29, 29, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_CK_6_0_31); //20200824_DFE_ADAP_EN_tap4=0
	_rtd_part_outl(0x180b7cc8, 23, 22, 0x3); //VTH_DFE_EN_B=1, VTH_EN_B=1
	_rtd_part_outl(0x180b7cdc, 31, 30, 0x1); //tap0_adjust_b =1
	//===B LANE TAP0 & LEQ
	_rtd_part_outl(0x180b7c8c, 31, 0, 0x00007400); //Timer, set eqfe_en,servo_en,timer_en=1
	_rtd_part_outl(0x180b7c90, 31, 31, 1); //leq1_inv=1
	_rtd_part_outl(0x180b7c90, 30, 30, 1); //leq2_inv=0
	_rtd_part_outl(0x180b7c94, 17, 15, 0); //LEQ_GAIN1=0
	_rtd_part_outl(0x180b7c94, 14, 12, 0); //LEQ_GAIN2=0
	_rtd_part_outl(0x180b7c98, 22, 21, 1); //LEQ1_trans_mode=1
	_rtd_part_outl(0x180b7c98, 20, 19, 3); //LEQ2_trans_mode=3
	_rtd_part_outl(0x180b7c98, 28, 26, 4); //tap0_gain=4
	_rtd_part_outl(0x180b7c8c, 26, 24, 5); //20200824_tap_timer=5
	_rtd_part_outl(0x180b7c8c, 20, 18, 5); //20200825_LEQ_timer=5
	_rtd_part_outl(0x180b7c8c, 23, 21, 5); //20200825_VTH_timer=5
	_rtd_part_outl(0x180b7c8c, 13, 13, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[tmds_timing].EQFE_EN_B_46); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7ca8, 25, 25, 1); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7ca8, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=0
	_rtd_part_outl(0x180b7ca8, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=0
	_rtd_part_outl(0x180b7ca8, 28, 28, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_B_6_0_50); //20200824_DFE_ADAP_EN_tap3=0
	_rtd_part_outl(0x180b7ca8, 29, 29, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_B_6_0_51); //20200824_DFE_ADAP_EN_tap4=0
	_rtd_part_outl(0x180b7c88, 23, 22, 0x3); //VTH_DFE_EN_B=1, VTH_EN_B=1
	_rtd_part_outl(0x180b7c9c, 31, 30, 0x1); //tap0_adjust_b =1
	//===G LANE TAP0 & LEQ
	_rtd_part_outl(0x180b7c4c, 31, 0, 0x00007400); //Timer, set eqfe_en,servo_en,timer_en=1
	_rtd_part_outl(0x180b7c50, 31, 31, 1); //leq1_inv=1
	_rtd_part_outl(0x180b7c50, 30, 30, 1); //leq2_inv=0
	_rtd_part_outl(0x180b7c54, 17, 15, 0); //LEQ_GAIN1=0
	_rtd_part_outl(0x180b7c54, 14, 12, 0); //LEQ_GAIN2=0
	_rtd_part_outl(0x180b7c58, 22, 21, 1); //LEQ1_trans_mode=1
	_rtd_part_outl(0x180b7c58, 20, 19, 3); //LEQ2_trans_mode=3
	_rtd_part_outl(0x180b7c58, 28, 26, 4); //tap0_gain=4
	_rtd_part_outl(0x180b7c4c, 26, 24, 5); //20200824_tap_timer=5
	_rtd_part_outl(0x180b7c4c, 20, 18, 5); //20200825_LEQ_timer=5
	_rtd_part_outl(0x180b7c4c, 23, 21, 5); //20200825_VTH_timer=5
	_rtd_part_outl(0x180b7c4c, 13, 13, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[tmds_timing].EQFE_EN_G_66); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7c68, 25, 25, 1); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7c68, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=0
	_rtd_part_outl(0x180b7c68, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=0
	_rtd_part_outl(0x180b7c68, 28, 28, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_G_6_0_70); //20200824_DFE_ADAP_EN_tap3=0
	_rtd_part_outl(0x180b7c68, 29, 29, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_G_6_0_71); //20200824_DFE_ADAP_EN_tap4=0
	_rtd_part_outl(0x180b7c48, 23, 22, 0x3); //VTH_DFE_EN_B=1, VTH_EN_B=1
	_rtd_part_outl(0x180b7c5c, 31, 30, 0x1); //tap0_adjust_b =1
	//===R LANE LEQ_DFE_adapat
	_rtd_part_outl(0x180b7c0c, 31, 0, 0x00007400); //Timer, set eqfe_en,servo_en,timer_en=1
	_rtd_part_outl(0x180b7c10, 31, 31, 1); //leq1_inv=1
	_rtd_part_outl(0x180b7c10, 30, 30, 1); //leq2_inv=0
	_rtd_part_outl(0x180b7c14, 17, 15, 0); //LEQ_GAIN1=0
	_rtd_part_outl(0x180b7c14, 14, 12, 0); //LEQ_GAIN2=0
	_rtd_part_outl(0x180b7c18, 22, 21, 1); //LEQ1_trans_mode=1
	_rtd_part_outl(0x180b7c18, 20, 19, 3); //LEQ2_trans_mode=3
	_rtd_part_outl(0x180b7c18, 28, 26, 4); //tap0_gain=4
	_rtd_part_outl(0x180b7c0c, 26, 24, 5); //20200824_tap_timer=5
	_rtd_part_outl(0x180b7c0c, 20, 18, 5); //20200825_LEQ_timer=5
	_rtd_part_outl(0x180b7c0c, 23, 21, 5); //20200825_VTH_timer=5
	_rtd_part_outl(0x180b7c0c, 13, 13, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[tmds_timing].EQFE_EN_R_86); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7c28, 25, 25, 1); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7c28, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=0
	_rtd_part_outl(0x180b7c28, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=0
	_rtd_part_outl(0x180b7c28, 28, 28, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_R_6_0_90); //20200824_DFE_ADAP_EN_tap3=0
	_rtd_part_outl(0x180b7c28, 29, 29, LEQ_VTH_Tap0_3_4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_R_6_0_91); //20200824_DFE_ADAP_EN_tap4=0
	_rtd_part_outl(0x180b7c08, 23, 22, 0x3); //VTH_DFE_EN_B=1, VTH_EN_B=1
	_rtd_part_outl(0x180b7c1c, 31, 30, 0x1); //tap0_adjust_b =1
	//LEQ_DFE adapt
	_rtd_part_outl(0x180b7ccc, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7ce8, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7ce8, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=1
	_rtd_part_outl(0x180b7ce8, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=1
	_rtd_part_outl(0x180b7ce8, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=1
	_rtd_part_outl(0x180b7ce8, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=1
	_rtd_part_outl(0x180b7cc8, 23, 22, 0); //VTH_DFE_EN_B=1, VTH_EN_B=1
	_rtd_part_outl(0x180b7c8c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7ca8, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7ca8, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=0
	_rtd_part_outl(0x180b7ca8, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=0
	_rtd_part_outl(0x180b7ca8, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=0
	_rtd_part_outl(0x180b7ca8, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=0
	_rtd_part_outl(0x180b7c88, 23, 22, 0); //VTH_DFE_EN_B=0, VTH_EN_B=0
	_rtd_part_outl(0x180b7c4c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7c68, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7c68, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=0
	_rtd_part_outl(0x180b7c68, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=0
	_rtd_part_outl(0x180b7c68, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=0
	_rtd_part_outl(0x180b7c68, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=0
	_rtd_part_outl(0x180b7c48, 23, 22, 0); //VTH_DFE_EN_B=1, VTH_EN_B=0
	_rtd_part_outl(0x180b7c0c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7c28, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=0
	_rtd_part_outl(0x180b7c28, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=0
	_rtd_part_outl(0x180b7c28, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=0
	_rtd_part_outl(0x180b7c28, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=0
	_rtd_part_outl(0x180b7c08, 23, 22, 0); //VTH_DFE_EN_B=1, VTH_EN_B=0
	_rtd_part_outl(0x180b7c28, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
}

void Tap0_to_Tap4_Adapt_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing)
{
	_rtd_part_outl(0x1800dd70, 3, 0, 0xf); //ck,r,g,b rstb_eq=1
	_rtd_part_outl(0x1800dd70, 7, 4, 0xf); //ck,r,g,b leq_en=1
	_rtd_part_outl(0x1800dd70, 15, 12, 0xf); //ck,r,g,b vth_en=1
	_rtd_part_outl(0x1800dd70, 11, 8, 0xf); //dfe_en=1
	//CK-Lane Tap0-4 Adapt
	_rtd_part_outl(0x180b7ccc, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7ce8, 25, 25, 1); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7ce8, 26, 26, 1); //20200824_DFE_ADAP_EN_tap1=0
	_rtd_part_outl(0x180b7ce8, 27, 27, Tap0_to_Tap4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_CK_6_0_9); //20200824_DFE_ADAP_EN_tap2=0
	_rtd_part_outl(0x180b7ce8, 28, 28, Tap0_to_Tap4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_CK_6_0_10); //20200824_DFE_ADAP_EN_tap3=0
	_rtd_part_outl(0x180b7ce8, 29, 29, Tap0_to_Tap4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_CK_6_0_11); //20200824_DFE_ADAP_EN_tap4=0
	_rtd_part_outl(0x180b7cc8, 23, 22, 0x3); //VTH_DFE_EN_B=1, VTH_EN_B=1
	//B-Lane Tap0-4 Adapt
	_rtd_part_outl(0x180b7c8c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7ca8, 25, 25, 1); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7ca8, 26, 26, 1); //20200824_DFE_ADAP_EN_tap1=0
	_rtd_part_outl(0x180b7ca8, 27, 27, Tap0_to_Tap4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_B_6_0_17); //20200824_DFE_ADAP_EN_tap2=0
	_rtd_part_outl(0x180b7ca8, 28, 28, Tap0_to_Tap4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_B_6_0_18); //20200824_DFE_ADAP_EN_tap3=0
	_rtd_part_outl(0x180b7ca8, 29, 29, Tap0_to_Tap4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_B_6_0_19); //20200824_DFE_ADAP_EN_tap4=0
	_rtd_part_outl(0x180b7c88, 23, 22, 0x3); //VTH_DFE_EN_B=1, VTH_EN_B=1
	//G-Lane Tap0-4 Adapt
	_rtd_part_outl(0x180b7c4c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7c68, 25, 25, 1); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7c68, 26, 26, 1); //20200824_DFE_ADAP_EN_tap1=0
	_rtd_part_outl(0x180b7c68, 27, 27, Tap0_to_Tap4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_G_6_0_25); //20200824_DFE_ADAP_EN_tap2=0
	_rtd_part_outl(0x180b7c68, 28, 28, Tap0_to_Tap4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_G_6_0_26); //20200824_DFE_ADAP_EN_tap3=0
	_rtd_part_outl(0x180b7c68, 29, 29, Tap0_to_Tap4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_G_6_0_27); //20200824_DFE_ADAP_EN_tap4=0
	_rtd_part_outl(0x180b7c48, 23, 22, 0x3); //VTH_DFE_EN_B=1, VTH_EN_B=1
	//R-Lane Tap0-4 Adapt
	_rtd_part_outl(0x180b7c0c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7c28, 25, 25, 1); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7c28, 26, 26, 1); //20200824_DFE_ADAP_EN_tap1=0
	_rtd_part_outl(0x180b7c28, 27, 27, Tap0_to_Tap4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_R_6_0_33); //20200824_DFE_ADAP_EN_tap2=0
	_rtd_part_outl(0x180b7c28, 28, 28, Tap0_to_Tap4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_R_6_0_34); //20200824_DFE_ADAP_EN_tap3=0
	_rtd_part_outl(0x180b7c28, 29, 29, Tap0_to_Tap4_Adapt_tmds_1[tmds_timing].DFE_ADAPT_EN_R_6_0_35); //20200824_DFE_ADAP_EN_tap4=0
	_rtd_part_outl(0x180b7c08, 23, 22, 0x3); //VTH_DFE_EN_B=1, VTH_EN_B=1
	//CK-Lane Tap0-4 Adapt
	_rtd_part_outl(0x180b7ccc, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7ce8, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7ce8, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=1
	_rtd_part_outl(0x180b7ce8, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=1
	_rtd_part_outl(0x180b7ce8, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=1
	_rtd_part_outl(0x180b7ce8, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=1
	_rtd_part_outl(0x180b7cc8, 23, 22, 0); //VTH_DFE_EN_B=1, VTH_EN_B=1
	//B-Lane Tap0-4 Adapt
	_rtd_part_outl(0x180b7c8c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7ca8, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7ca8, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=1
	_rtd_part_outl(0x180b7ca8, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=1
	_rtd_part_outl(0x180b7ca8, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=1
	_rtd_part_outl(0x180b7ca8, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=1
	_rtd_part_outl(0x180b7c88, 23, 22, 0); //VTH_DFE_EN_B=1, VTH_EN_B=1
	//G-Lane Tap0-4 Adapt
	_rtd_part_outl(0x180b7c4c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7c68, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7c68, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=1
	_rtd_part_outl(0x180b7c68, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=1
	_rtd_part_outl(0x180b7c68, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=1
	_rtd_part_outl(0x180b7c68, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=1
	_rtd_part_outl(0x180b7c48, 23, 22, 0); //VTH_DFE_EN_B=1, VTH_EN_B=1
	//R-Lane Tap0-4 Adapt
	_rtd_part_outl(0x180b7c0c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7c28, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7c28, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=1
	_rtd_part_outl(0x180b7c28, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=1
	_rtd_part_outl(0x180b7c28, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=1
	_rtd_part_outl(0x180b7c28, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=1
	_rtd_part_outl(0x180b7c08, 23, 22, 0); //VTH_DFE_EN_B=1, VTH_EN_B=1
}

void DCDR_settings_tmds_3_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing)
{
	_rtd_part_outl(0x1800dc00, 7, 0, 0x0); //Toggle Dphy reset at the Phy script ending
	ScalerTimer_DelayXms(5);
	_rtd_part_outl(0x1800dc00, 7, 0, 0xff); //Toggle Dphy reset at the Phy script ending
	//DFE Adaptation power off
	_rtd_part_outl(0x1800da3c, 7, 7, 0); //Mark2_Modify; CK-Lane DFE_ADAPT_EN
	_rtd_part_outl(0x1800da5c, 7, 7, 0); //Mark2_Modify; B-Lane DFE_ADAPT_EN
	_rtd_part_outl(0x1800da7c, 7, 7, 0); //Mark2_Modify; G-Lane DFE_ADAPT_EN
	_rtd_part_outl(0x1800da9c, 7, 7, 0); //Mark2_Modify; R-Lane DFE_ADAPT_EN
}

void Manual_DFE_tmds_1_func(unsigned char nport, TMDS_TIMING_PARAM_T tmds_timing)
{
	// *************************************************************************************************************************************
	//B-Lane_initial DFE Value_Start
	_rtd_part_outl(0x180b7ca8, 9, 5, 5); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_rtd_part_outl(0x180b7ca8, 4, 0, 5); //VTHN_INIT_B[4:0]	4:0	R/W	0x0F		VTHN INIT
	_rtd_part_outl(0x180b7ca0, 4, 0, 0); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_rtd_part_outl(0x180b7ca4, 31, 24, 25); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_rtd_part_outl(0x180b7ca4, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_rtd_part_outl(0x180b7ca4, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_rtd_part_outl(0x180b7ca4, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_rtd_part_outl(0x180b7ca4, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//B-Lane DFE COEF LOAD IN
	_rtd_part_outl(0x180b7c8c, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_rtd_part_outl(0x180b7c8c, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_rtd_part_outl(0x180b7c8c, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_rtd_part_outl(0x180b7c8c, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_rtd_part_outl(0x180b7c8c, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_rtd_part_outl(0x180b7c8c, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_rtd_part_outl(0x180b7c88, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_rtd_part_outl(0x180b7c8c, 5, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_rtd_part_outl(0x180b7c88, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	//B-Lane_initial DFE Value_Stop
	//**************************************************************************************************************************************
	// *************************************************************************************************************************************
	//G-Lane_initial DFE Value_Start
	_rtd_part_outl(0x180b7c68, 9, 5, 5); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_rtd_part_outl(0x180b7c68, 4, 0, 5); //VTHN_INIT_B[4:0]	4:0	R/W	0x0F		VTHN INIT
	_rtd_part_outl(0x180b7c60, 4, 0, 0); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_rtd_part_outl(0x180b7c64, 31, 24, 25); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_rtd_part_outl(0x180b7c64, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_rtd_part_outl(0x180b7c64, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_rtd_part_outl(0x180b7c64, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_rtd_part_outl(0x180b7c64, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//G-Lane DFE COEF LOAD IN
	_rtd_part_outl(0x180b7c4c, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_rtd_part_outl(0x180b7c4c, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_rtd_part_outl(0x180b7c4c, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_rtd_part_outl(0x180b7c4c, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_rtd_part_outl(0x180b7c4c, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_rtd_part_outl(0x180b7c4c, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_rtd_part_outl(0x180b7c48, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_rtd_part_outl(0x180b7c4c, 5, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_rtd_part_outl(0x180b7c48, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	//G-Lane_initial DFE Value_Stop
	//**************************************************************************************************************************************
	// *************************************************************************************************************************************
	//R-Lane_initial DFE Value_Start
	_rtd_part_outl(0x180b7c28, 9, 5, 5); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_rtd_part_outl(0x180b7c28, 4, 0, 5); //VTHN_INIT_B[4:0]	4:0	R/W	0x0F		VTHN INIT
	_rtd_part_outl(0x180b7c20, 4, 0, 0); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_rtd_part_outl(0x180b7c24, 31, 24, 25); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_rtd_part_outl(0x180b7c24, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_rtd_part_outl(0x180b7c24, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_rtd_part_outl(0x180b7c24, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_rtd_part_outl(0x180b7c24, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//R-Lane DFE COEF LOAD IN
	_rtd_part_outl(0x180b7c0c, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_rtd_part_outl(0x180b7c0c, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_rtd_part_outl(0x180b7c0c, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_rtd_part_outl(0x180b7c0c, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_rtd_part_outl(0x180b7c0c, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_rtd_part_outl(0x180b7c0c, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_rtd_part_outl(0x180b7c08, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_rtd_part_outl(0x180b7c0c, 5, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_rtd_part_outl(0x180b7c08, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	//R-Lane_initial DFE Value_Stop
	//**************************************************************************************************************************************
	// *************************************************************************************************************************************
	//CK-Lane_initial DFE Value_Start
	_rtd_part_outl(0x180b7ce8, 9, 5, 5); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_rtd_part_outl(0x180b7ce8, 4, 0, 5); //VTHN_INIT_B[4:0]	4:0	R/W	0x0F		VTHN INIT
	_rtd_part_outl(0x180b7ce0, 4, 0, 0); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_rtd_part_outl(0x180b7ce4, 31, 24, 25); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_rtd_part_outl(0x180b7ce4, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_rtd_part_outl(0x180b7ce4, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_rtd_part_outl(0x180b7ce4, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_rtd_part_outl(0x180b7ce4, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//CK-Lane DFE COEF LOAD IN
	_rtd_part_outl(0x180b7ccc, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_rtd_part_outl(0x180b7ccc, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_rtd_part_outl(0x180b7ccc, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_rtd_part_outl(0x180b7ccc, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_rtd_part_outl(0x180b7ccc, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_rtd_part_outl(0x180b7ccc, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_rtd_part_outl(0x180b7cc8, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_rtd_part_outl(0x180b7ccc, 5, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_rtd_part_outl(0x180b7cc8, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_rtd_part_outl(0x180b7cf4, 17, 15, 3); //Adaption flow control mode
	_rtd_part_outl(0x180b7cb4, 17, 15, 3); //Adaption flow control mode
	_rtd_part_outl(0x180b7c74, 17, 15, 3); //Adaption flow control mode
	_rtd_part_outl(0x180b7c34, 17, 15, 3); //Adaption flow control mode
	//CK-Lane_initial DFE Value_Stop
	//**************************************************************************************************************************************
}





//FROM Golden setting generator FRL mode
finetunestart_on_T finetunestart_on[] =
{
	{FRL_12G,0x1},
	{FRL_10G,0x1},
	{FRL_8G,0x1},
	{FRL_6G,0x1},
	{FRL_3G,0x0},
};

APHY_Para_frl_1_T APHY_Para_frl_1[] =
{
	{FRL_12G,0x3,0x3,0x3,0x1,0xc,0xc,0x4,0x3,0x2,0xf,0xf,0xc,0x0,0x0,0x1,0x0,0x1,0x0,0x0,0x1,0x3,0x2,0x6b,0x1,0x3,0x3,0x1,0x3,0x3,0x3,0x1,0xc,0xc,0x4,0x3,0x2,0xf,0xf,0xc,0x0,0x0,0x1,0x0,0x1,0x0,0x0,0x1,0x3,0x2,0x6b,0x1,0x3,0x3,0x1,0x3,0x3,0x3,0x1,0xc,0xc,0x4,0x3,0x2,0xf,0xf,0xc,0x0,0x0,0x1,0x0,0x1,0x0,0x0,0x1,0x3,0x2,0x6b,0x1,0x3,0x3,0x1,0x3,0x3,0x3,0x1,0xc,0xc,0x4,0x3,0x2,0xf,0xf,0xc,0x0,0x0,0x1,0x0,0x1,0x0,0x0,0x1,0x3,0x2,0x6b,0x1,0x3,0x3,0x1},
	{FRL_10G,0x2,0x3,0x3,0x1,0xc,0xc,0x6,0x3,0x2,0xf,0xf,0xd,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x2,0x1,0xb5,0x0,0x2,0x3,0x1,0x2,0x3,0x3,0x1,0xc,0xc,0x6,0x3,0x2,0xf,0xf,0xd,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x2,0x1,0xb5,0x0,0x2,0x3,0x1,0x2,0x3,0x3,0x1,0xc,0xc,0x6,0x3,0x2,0xf,0xf,0xd,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x2,0x1,0xb5,0x0,0x2,0x3,0x1,0x2,0x3,0x3,0x1,0xc,0xc,0x6,0x3,0x2,0xf,0xf,0xd,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x2,0x1,0xb5,0x0,0x2,0x3,0x1},
	{FRL_8G,0x2,0x3,0x3,0x1,0xc,0xc,0x6,0x3,0x2,0xf,0xf,0xd,0x0,0x0,0x1,0x0,0x1,0x0,0x0,0x1,0x0,0x1,0x90,0x0,0x2,0x3,0x1,0x2,0x3,0x3,0x1,0xc,0xc,0x6,0x3,0x2,0xf,0xf,0xd,0x0,0x0,0x1,0x0,0x1,0x0,0x0,0x1,0x0,0x1,0x90,0x0,0x2,0x3,0x1,0x2,0x3,0x3,0x1,0xc,0xc,0x6,0x3,0x2,0xf,0xf,0xd,0x0,0x0,0x1,0x0,0x1,0x0,0x0,0x1,0x0,0x1,0x90,0x0,0x2,0x3,0x1,0x2,0x3,0x3,0x1,0xc,0xc,0x6,0x3,0x2,0xf,0xf,0xd,0x0,0x0,0x1,0x0,0x1,0x0,0x0,0x1,0x0,0x1,0x90,0x0,0x2,0x3,0x1},
	{FRL_6G,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x1,0x3,0x1,0xda,0x0,0x2,0x0,0x0,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x1,0x3,0x1,0xda,0x0,0x2,0x0,0x0,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x1,0x3,0x1,0xda,0x0,0x2,0x0,0x0,0x2,0x3,0x1,0x1,0x7,0x5,0x4,0x3,0x1,0xd,0xf,0xf,0x1,0x0,0x1,0x1,0x0,0x1,0x1,0x1,0x3,0x1,0xda,0x0,0x2,0x0,0x0},
	{FRL_3G,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x0,0x3,0x2,0xda,0x0,0x1,0x0,0x0,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x0,0x3,0x2,0xda,0x0,0x1,0x0,0x0,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x0,0x3,0x2,0xda,0x0,0x1,0x0,0x0,0x1,0x1,0x0,0x0,0x3,0x3,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x0,0x1,0x0,0x1,0x1,0x0,0x3,0x2,0xda,0x0,0x1,0x0,0x0},
};

DFE_ini_frl_1_T DFE_ini_frl_1[] =
{
	{FRL_12G,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3},
	{FRL_10G,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5},
	{FRL_8G,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5},
	{FRL_6G,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5},
	{FRL_3G,0x5,0x5,0x5,0x5,0x5,0x5,0x5,0x5},
};

DPHY_Fix_frl_1_T DPHY_Fix_frl_1[] =
{
	{FRL_12G,0x77750,0x77750,0x77750,0x77750},
	{FRL_10G,0x77750,0x77750,0x77750,0x77750},
	{FRL_8G,0x77770,0x77770,0x77770,0x77770},
	{FRL_6G,0x77770,0x77770,0x77770,0x77770},
	{FRL_3G,0x77750,0x77750,0x77750,0x77750},
};

DPHY_Init_Flow_frl_1_T DPHY_Init_Flow_frl_1[] =
{
	{FRL_12G,0x32c,0x356,0x32c,0x356,0x32c,0x356,0x32c,0x356,0x32c,0x356,0x32c,0x356,0x32c,0x356,0x32c,0x356},
	{FRL_10G,0x2a2,0x2cc,0x2a2,0x2cc,0x2a2,0x2cc,0x2a2,0x2cc,0x2a2,0x2cc,0x2a2,0x2cc,0x2a2,0x2cc,0x2a2,0x2cc},
	{FRL_8G,0x215,0x243,0x215,0x243,0x215,0x243,0x215,0x243,0x215,0x243,0x215,0x243,0x215,0x243,0x215,0x243},
	{FRL_6G,0x32c,0x356,0x32c,0x356,0x32c,0x356,0x32c,0x356,0x32c,0x356,0x32c,0x356,0x32c,0x356,0x32c,0x356},
	{FRL_3G,0x32c,0x356,0x32c,0x356,0x32c,0x356,0x32c,0x356,0x32c,0x356,0x32c,0x356,0x32c,0x356,0x32c,0x356},
};

DPHY_Para_frl_1_T DPHY_Para_frl_1[] =
{
	{FRL_12G,0x0,0x0,0x0,0x0},
	{FRL_10G,0x0,0x0,0x0,0x0},
	{FRL_8G,0x0,0x0,0x0,0x0},
	{FRL_6G,0x1,0x1,0x1,0x1},
	{FRL_3G,0x1,0x1,0x1,0x1},
};

Koffset_frl_1_T Koffset_frl_1[] =
{
	{FRL_12G,0x3,0x3,0x3,0x3,0xf,0xf,0xf,0xf,0x1},
	{FRL_10G,0x3,0x3,0x3,0x3,0xf,0xf,0xf,0xf,0x1},
	{FRL_8G,0x3,0x3,0x3,0x3,0xf,0xf,0xf,0xf,0x1},
	{FRL_6G,0x3,0x3,0x3,0x3,0xf,0xf,0xf,0xf,0x1},
	{FRL_3G,0x2,0x2,0x2,0x2,0x3,0x3,0x3,0x3,0x0},
};

LEQ_VTH_Tap0_3_4_Adapt_frl_1_T LEQ_VTH_Tap0_3_4_Adapt_frl_1[] =
{
	{FRL_12G,0x2,0x1,0x1,0x2,0x1,0x1,0x2,0x1,0x1,0x2,0x1,0x1,0x1},
	{FRL_10G,0x2,0x1,0x1,0x2,0x1,0x1,0x2,0x1,0x1,0x2,0x1,0x1,0x1},
	{FRL_8G,0x2,0x1,0x1,0x2,0x1,0x1,0x2,0x1,0x1,0x2,0x1,0x1,0x1},
	{FRL_6G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{FRL_3G,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0,0x1,0x0,0x0,0x0},
};

Tap0_to_Tap4_Adapt_frl_1_T Tap0_to_Tap4_Adapt_frl_1[] =
{
	{FRL_12G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{FRL_10G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{FRL_8G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{FRL_6G,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{FRL_3G,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
};

void FRL_Merged_V1_20220315_Main_Seq(unsigned char nport, FRL_TIMING_PARAM_T frl_timing)
{
	DFE_ini_frl_1_func(nport, frl_timing);
	DPHY_Fix_frl_1_func(nport, frl_timing);
	DPHY_Para_frl_1_func(nport, frl_timing);
	APHY_Fix_frl_1_func(nport, frl_timing);
	APHY_Para_frl_1_func(nport, frl_timing);
	DPHY_Init_Flow_frl_1_func(nport, frl_timing);
	APHY_Init_Flow_frl_1_func(nport, frl_timing);
	ACDR_settings_frl_1_func(nport, frl_timing);
	ACDR_settings_frl_2_func(nport, frl_timing);
	Koffset_frl_1_func(nport, frl_timing);
	LEQ_VTH_Tap0_3_4_Adapt_frl_1_func(nport, frl_timing);
	Koffset_frl_1_func(nport, frl_timing);
	Tap0_to_Tap4_Adapt_frl_1_func(nport, frl_timing);
	Koffset_frl_1_func(nport, frl_timing);
	ACDR_settings_frl_3_func(nport, frl_timing);
	ACDR_settings_frl_4_func(nport, frl_timing);
}


void DFE_ini_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing)
{
	_rtd_part_outl(0x1800dd70, 31, 0, 0x00000000);
	//R lane
	_rtd_part_outl(0x180b7c00, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7c04, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7c08, 31, 0, 0x20C00000);
	_rtd_part_outl(0x180b7c0c, 31, 0, 0x05b47400);
	_rtd_part_outl(0x180b7c10, 31, 0, 0xc0004000);
	_rtd_part_outl(0x180b7c14, 31, 0, 0x00280000);
	_rtd_part_outl(0x180b7c18, 31, 0, 0x1038ffe0);
	_rtd_part_outl(0x180b7c1c, 31, 0, 0x7FF07FE0);
	_rtd_part_outl(0x180b7c20, 31, 0, 0x3e0383e0);
	_rtd_part_outl(0x180b7c24, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7c28, 31, 0, 0x009D00E7);
	_rtd_part_outl(0x180b7c2c, 31, 0, 0xFF00FFC0);
	_rtd_part_outl(0x180b7c30, 31, 0, 0xff005400);
	//G lane
	_rtd_part_outl(0x180b7c40, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7c44, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7c48, 31, 0, 0x20C00000);
	_rtd_part_outl(0x180b7c4c, 31, 0, 0x05b47400);
	_rtd_part_outl(0x180b7c50, 31, 0, 0xc0004000);
	_rtd_part_outl(0x180b7c54, 31, 0, 0x00280000);
	_rtd_part_outl(0x180b7c58, 31, 0, 0x1038ffe0);
	_rtd_part_outl(0x180b7c5c, 31, 0, 0x7FF07FE0);
	_rtd_part_outl(0x180b7c60, 31, 0, 0x3e0383e0);
	_rtd_part_outl(0x180b7c64, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7c68, 31, 0, 0x009D00E7);
	_rtd_part_outl(0x180b7c6c, 31, 0, 0xFF00FFC0);
	_rtd_part_outl(0x180b7c70, 31, 0, 0xff005400);
	//B lane
	_rtd_part_outl(0x180b7c80, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7c84, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7c88, 31, 0, 0x20C00000);
	_rtd_part_outl(0x180b7c8c, 31, 0, 0x05b47400);
	_rtd_part_outl(0x180b7c90, 31, 0, 0xc0004000);
	_rtd_part_outl(0x180b7c94, 31, 0, 0x00280000);
	_rtd_part_outl(0x180b7c98, 31, 0, 0x1038ffe0);
	_rtd_part_outl(0x180b7c9c, 31, 0, 0x7FF07FE0);
	_rtd_part_outl(0x180b7ca0, 31, 0, 0x3e0383e0);
	_rtd_part_outl(0x180b7ca4, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7ca8, 31, 0, 0x009D00E7);
	_rtd_part_outl(0x180b7cac, 31, 0, 0xFF00FFC0);
	_rtd_part_outl(0x180b7cb0, 31, 0, 0xff005400);
	//CK lane
	_rtd_part_outl(0x180b7cc0, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7cc4, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7cc8, 31, 0, 0x20C00000);
	_rtd_part_outl(0x180b7ccc, 31, 0, 0x05b47400);
	_rtd_part_outl(0x180b7cd0, 31, 0, 0xc0004000);
	_rtd_part_outl(0x180b7cd4, 31, 0, 0x00280000);
	_rtd_part_outl(0x180b7cd8, 31, 0, 0x1038ffe0);
	_rtd_part_outl(0x180b7cdc, 31, 0, 0x7FF07FE0);
	_rtd_part_outl(0x180b7ce0, 31, 0, 0x3e0383e0);
	_rtd_part_outl(0x180b7ce4, 31, 0, 0x00000000);
	_rtd_part_outl(0x180b7ce8, 31, 0, 0x009D00E7);
	_rtd_part_outl(0x180b7cec, 31, 0, 0xFF00FFC0);
	_rtd_part_outl(0x180b7cf0, 31, 0, 0xff005400);
	// *************************************************************************************************************************************
	//B-Lane_initial DFE Value_Start
	_rtd_part_outl(0x180b7ca8, 9, 5, DFE_ini_frl_1[frl_timing].VTHP_INIT_B_4_0_60); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_rtd_part_outl(0x180b7ca8, 4, 0, DFE_ini_frl_1[frl_timing].VTHN_INIT_B_4_0_61); //VTHN_INIT_B[4:0]	4:0	R/W	0x0F		VTHN INIT
	_rtd_part_outl(0x180b7ca0, 4, 0, 15); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_rtd_part_outl(0x180b7ca4, 31, 24, 16); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_rtd_part_outl(0x180b7ca4, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_rtd_part_outl(0x180b7ca4, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_rtd_part_outl(0x180b7ca4, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_rtd_part_outl(0x180b7ca4, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//B-Lane DFE COEF LOAD IN
	_rtd_part_outl(0x180b7c8c, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_rtd_part_outl(0x180b7c8c, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_rtd_part_outl(0x180b7c8c, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_rtd_part_outl(0x180b7c8c, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_rtd_part_outl(0x180b7c8c, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_rtd_part_outl(0x180b7c8c, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_rtd_part_outl(0x180b7c88, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_rtd_part_outl(0x180b7c8c, 5, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_rtd_part_outl(0x180b7c88, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	//B-Lane_initial DFE Value_Stop
	// *************************************************************************************************************************************
	//G-Lane_initial DFE Value_Start
	_rtd_part_outl(0x180b7c68, 9, 5, DFE_ini_frl_1[frl_timing].VTHP_INIT_G_4_0_81); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_rtd_part_outl(0x180b7c68, 4, 0, DFE_ini_frl_1[frl_timing].VTHN_INIT_G_4_0_82); //VTHN_INIT_B[4:0]	4:0	R/W	0x0F		VTHN INIT
	_rtd_part_outl(0x180b7c60, 4, 0, 15); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_rtd_part_outl(0x180b7c64, 31, 24, 16); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_rtd_part_outl(0x180b7c64, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_rtd_part_outl(0x180b7c64, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_rtd_part_outl(0x180b7c64, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_rtd_part_outl(0x180b7c64, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//G-Lane DFE COEF LOAD IN
	_rtd_part_outl(0x180b7c4c, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_rtd_part_outl(0x180b7c4c, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_rtd_part_outl(0x180b7c4c, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_rtd_part_outl(0x180b7c4c, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_rtd_part_outl(0x180b7c4c, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_rtd_part_outl(0x180b7c4c, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_rtd_part_outl(0x180b7c48, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_rtd_part_outl(0x180b7c4c, 5, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_rtd_part_outl(0x180b7c48, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	//G-Lane_initial DFE Value_Stop
	// *************************************************************************************************************************************
	//R-Lane_initial DFE Value_Start
	_rtd_part_outl(0x180b7c28, 9, 5, DFE_ini_frl_1[frl_timing].VTHP_INIT_R_4_0_102); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_rtd_part_outl(0x180b7c28, 4, 0, DFE_ini_frl_1[frl_timing].VTHN_INIT_R_4_0_103); //VTHN_INIT_B[4:0]	4:0	R/W	0x0F		VTHN INIT
	_rtd_part_outl(0x180b7c20, 4, 0, 15); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_rtd_part_outl(0x180b7c24, 31, 24, 16); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_rtd_part_outl(0x180b7c24, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_rtd_part_outl(0x180b7c24, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_rtd_part_outl(0x180b7c24, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_rtd_part_outl(0x180b7c24, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//R-Lane DFE COEF LOAD IN
	_rtd_part_outl(0x180b7c0c, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_rtd_part_outl(0x180b7c0c, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_rtd_part_outl(0x180b7c0c, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_rtd_part_outl(0x180b7c0c, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_rtd_part_outl(0x180b7c0c, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_rtd_part_outl(0x180b7c0c, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_rtd_part_outl(0x180b7c08, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_rtd_part_outl(0x180b7c0c, 5, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_rtd_part_outl(0x180b7c08, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	//R-Lane_initial DFE Value_Stop
	// *************************************************************************************************************************************
	//CK-Lane_initial DFE Value_Start
	_rtd_part_outl(0x180b7ce8, 9, 5, DFE_ini_frl_1[frl_timing].VTHP_INIT_CK_4_0_123); //VTHP_INIT_B[4:0]	9:5	R/W	0x0F		VTHP INIT
	_rtd_part_outl(0x180b7ce8, 4, 0, DFE_ini_frl_1[frl_timing].VTHN_INIT_CK_4_0_124); //VTHN_INIT_B`[4:0]	4:0	R/W	0x0F		VTHN INIT
	_rtd_part_outl(0x180b7ce0, 4, 0, 15); //LEQ_INIT_B[4:0]	4:0		R/W	0x00	LEQ INIT
	_rtd_part_outl(0x180b7ce4, 31, 24, 16); //TAP0_INIT_B[7:0]	31:24	R/W	0x00	TAP0 INIT
	_rtd_part_outl(0x180b7ce4, 23, 18, 0); //TAP1_INIT_B[5:0]	23:18	R/W	0x00	TAP1 INIT
	_rtd_part_outl(0x180b7ce4, 17, 12, 0); //TAP2_INIT_B[5:0]	17:12	R/W	0x00	TAP2 INIT
	_rtd_part_outl(0x180b7ce4, 11, 6, 0); //TAP3_INIT_B[5:0]	11:6	R/W	0x00	TAP3 INIT
	_rtd_part_outl(0x180b7ce4, 5, 0, 0); //TAP4_INIT_B[5:0]	5:0		R/W	0x00	TAP4 INIT
	//CK-Lane DFE COEF LOAD IN
	_rtd_part_outl(0x180b7ccc, 0, 0, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP0 LOAD IN
	_rtd_part_outl(0x180b7ccc, 1, 1, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP1 LOAD IN
	_rtd_part_outl(0x180b7ccc, 2, 2, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP2 LOAD IN
	_rtd_part_outl(0x180b7ccc, 3, 3, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP3 LOAD IN
	_rtd_part_outl(0x180b7ccc, 4, 4, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP4 LOAD IN
	_rtd_part_outl(0x180b7ccc, 5, 5, 1); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	Vth LOAD IN
	_rtd_part_outl(0x180b7cc8, 31, 31, 1); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	_rtd_part_outl(0x180b7ccc, 5, 0, 0); //TAP_LOAD_IN_B[4:0]	4:0	R/W	0x00	TAP LOAD IN
	_rtd_part_outl(0x180b7cc8, 31, 31, 0x0); //EQ_SELREG_B	31	R/W	0x0	EQ SELREG	LEQ LOAD IN
	//CK-Lane_initial DFE Value_Stop
	//**************************************************************************************************************************************
	_rtd_part_outl(0x180b7cf4, 17, 15, 3); //rotation_mode Adaption flow control mode
	_rtd_part_outl(0x180b7cb4, 17, 15, 3); //rotation_mode Adaption flow control mode
	_rtd_part_outl(0x180b7c74, 17, 15, 3); //rotation_mode Adaption flow control mode
	_rtd_part_outl(0x180b7c34, 17, 15, 3); //rotation_mode Adaption flow control mode
}
void DPHY_Fix_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing)
{
	//DPHY fix start
	_rtd_part_outl(0x1800dc00, 0, 0, 0x1); //[DPHY fix]  reg_p0_b_dig_rst_n
	_rtd_part_outl(0x1800dc00, 1, 1, 0x1); //[DPHY fix]  reg_p0_g_dig_rst_n
	_rtd_part_outl(0x1800dc00, 2, 2, 0x1); //[DPHY fix]  reg_p0_r_dig_rst_n
	_rtd_part_outl(0x1800dc00, 3, 3, 0x1); //[DPHY fix]  reg_p0_ck_dig_rst_n
	_rtd_part_outl(0x1800dc00, 4, 4, 0x0); //[DPHY fix]  reg_p0_b_cdr_rst_n
	_rtd_part_outl(0x1800dc00, 5, 5, 0x0); //[DPHY fix]  reg_p0_g_cdr_rst_n
	_rtd_part_outl(0x1800dc00, 6, 6, 0x0); //[DPHY fix]  reg_p0_r_cdr_rst_n
	_rtd_part_outl(0x1800dc00, 7, 7, 0x0); //[DPHY fix]  reg_p0_ck_cdr_rst_n
	_rtd_part_outl(0x1800dc00, 8, 8, 0x0); //[DPHY fix]  reg_p0_b_clk_inv
	_rtd_part_outl(0x1800dc00, 9, 9, 0x0); //[DPHY fix]  reg_p0_g_clk_inv
	_rtd_part_outl(0x1800dc00, 10, 10, 0x0); //[DPHY fix]  reg_p0_r_clk_inv
	_rtd_part_outl(0x1800dc00, 11, 11, 0x0); //[DPHY fix]  reg_p0_ck_clk_inv
	_rtd_part_outl(0x1800dc00, 12, 12, 0x0); //[DPHY fix]  reg_p0_b_shift_inv
	_rtd_part_outl(0x1800dc00, 13, 13, 0x0); //[DPHY fix]  reg_p0_g_shift_inv
	_rtd_part_outl(0x1800dc00, 14, 14, 0x0); //[DPHY fix]  reg_p0_r_shift_inv
	_rtd_part_outl(0x1800dc00, 15, 15, 0x0); //[DPHY fix]  reg_p0_ck_shift_inv
	_rtd_part_outl(0x1800dc00, 24, 24, 0x0); //[DPHY fix]  reg_p0_b_data_order
	_rtd_part_outl(0x1800dc00, 25, 25, 0x0); //[DPHY fix]  reg_p0_g_data_order
	_rtd_part_outl(0x1800dc00, 26, 26, 0x0); //[DPHY fix]  reg_p0_r_data_order
	_rtd_part_outl(0x1800dc00, 27, 27, 0x0); //[DPHY fix]  reg_p0_ck_data_order
	_rtd_part_outl(0x1800dc00, 28, 28, 0x0); //[DPHY fix]  reg_p0_b_dyn_kp_en
	_rtd_part_outl(0x1800dc00, 29, 29, 0x0); //[DPHY fix]  reg_p0_g_dyn_kp_en
	_rtd_part_outl(0x1800dc00, 30, 30, 0x0); //[DPHY fix]  reg_p0_r_dyn_kp_en
	_rtd_part_outl(0x1800dc00, 31, 31, 0x0); //[DPHY fix]  reg_p0_ck_dyn_kp_en
	_rtd_part_outl(0x1800dc04, 5, 0, 0x00); //[]  reg_p0_b_kp
	_rtd_part_outl(0x1800dc04, 11, 6, 0x00); //[]  reg_p0_g_kp
	_rtd_part_outl(0x1800dc04, 17, 12, 0x00); //[]  reg_p0_r_kp
	_rtd_part_outl(0x1800dc04, 23, 18, 0x00); //[]  reg_p0_ck_kp
	_rtd_part_outl(0x1800dc04, 24, 24, 0x0); //[]  reg_p0_b_dfe_edge_out
	_rtd_part_outl(0x1800dc04, 25, 25, 0x0); //[]  reg_p0_b_dfe_edge_out
	_rtd_part_outl(0x1800dc04, 26, 26, 0x0); //[]  reg_p0_r_dfe_edge_out
	_rtd_part_outl(0x1800dc04, 27, 27, 0x0); //[]  reg_p0_ck_dfe_edge_out
	_rtd_part_outl(0x1800dc08, 5, 0, 0x0c); //[]  reg_p0_b_bbw_kp
	_rtd_part_outl(0x1800dc08, 11, 6, 0x0c); //[]  reg_p0_g_bbw_kp
	_rtd_part_outl(0x1800dc08, 17, 12, 0x0c); //[]  reg_p0_r_bbw_kp
	_rtd_part_outl(0x1800dc08, 23, 18, 0x0c); //[]  reg_p0_ck_bbw_kp
	_rtd_part_outl(0x1800dc0c, 2, 0, 0x1); //[]  reg_p0_b_ki
	_rtd_part_outl(0x1800dc0c, 5, 3, 0x1); //[]  reg_p0_g_ki
	_rtd_part_outl(0x1800dc0c, 8, 6, 0x1); //[]  reg_p0_r_ki
	_rtd_part_outl(0x1800dc0c, 11, 9, 0x1); //[]  reg_p0_ck_ki
	_rtd_part_outl(0x1800dc0c, 12, 12, 0x0); //[]  reg_p0_b_st_mode
	_rtd_part_outl(0x1800dc0c, 13, 13, 0x0); //[]  reg_p0_g_st_mode
	_rtd_part_outl(0x1800dc0c, 14, 14, 0x0); //[]  reg_p0_r_st_mode
	_rtd_part_outl(0x1800dc0c, 15, 15, 0x0); //[]  reg_p0_ck_st_mode
	_rtd_part_outl(0x1800dc0c, 18, 16, 0x1); //[]  reg_p0_b_bbw_ki
	_rtd_part_outl(0x1800dc0c, 21, 19, 0x1); //[]  reg_p0_g_bbw_ki
	_rtd_part_outl(0x1800dc0c, 24, 22, 0x1); //[]  reg_p0_r_bbw_ki
	_rtd_part_outl(0x1800dc0c, 27, 25, 0x1); //[]  reg_p0_ck_bbw_ki
	_rtd_part_outl(0x1800dc0c, 28, 28, 0x0); //[]  reg_p0_b_bbw_st_mode
	_rtd_part_outl(0x1800dc0c, 29, 29, 0x0); //[]  reg_p0_g_bbw_st_mode
	_rtd_part_outl(0x1800dc0c, 30, 30, 0x0); //[]  reg_p0_r_bbw_st_mode
	_rtd_part_outl(0x1800dc0c, 31, 31, 0x0); //[]  reg_p0_ck_bbw_st_mode
	_rtd_part_outl(0x1800dc10, 0, 0, 0x0); //[]  reg_p0_b_bypass_sdm_int
	_rtd_part_outl(0x1800dc10, 11, 1, 0x000); //[]  reg_p0_b_int_init
	_rtd_part_outl(0x1800dc10, 21, 12, 0x000); //[]  reg_p0_b_acc2_period
	_rtd_part_outl(0x1800dc10, 22, 22, 0x0); //[]  reg_p0_b_acc2_manual
	_rtd_part_outl(0x1800dc10, 23, 23, 0x0); //[]  reg_p0_b_squ_tri
	_rtd_part_outl(0x1800dc14, 0, 0, 0x0); //[]  reg_p0_g_bypass_sdm_int
	_rtd_part_outl(0x1800dc14, 11, 1, 0x000); //[]  reg_p0_g_int_init
	_rtd_part_outl(0x1800dc14, 21, 12, 0x000); //[]  reg_p0_g_acc2_period
	_rtd_part_outl(0x1800dc14, 22, 22, 0x0); //[]  reg_p0_g_acc2_manual
	_rtd_part_outl(0x1800dc14, 23, 23, 0x0); //[]  reg_p0_g_squ_tri
	_rtd_part_outl(0x1800dc18, 0, 0, 0x0); //[]  reg_p0_r_bypass_sdm_int
	_rtd_part_outl(0x1800dc18, 11, 1, 0x000); //[]  reg_p0_r_int_init
	_rtd_part_outl(0x1800dc18, 21, 12, 0x000); //[]  reg_p0_r_acc2_period
	_rtd_part_outl(0x1800dc18, 22, 22, 0x0); //[]  reg_p0_r_acc2_manual
	_rtd_part_outl(0x1800dc18, 23, 23, 0x0); //[]  reg_p0_r_squ_tri
	_rtd_part_outl(0x1800dc1c, 0, 0, 0x0); //[]  reg_p0_ck_bypass_sdm_int
	_rtd_part_outl(0x1800dc1c, 11, 1, 0x000); //[]  reg_p0_ck_int_init
	_rtd_part_outl(0x1800dc1c, 21, 12, 0x000); //[]  reg_p0_ck_acc2_period
	_rtd_part_outl(0x1800dc1c, 22, 22, 0x0); //[]  reg_p0_ck_acc2_manual
	_rtd_part_outl(0x1800dc1c, 23, 23, 0x0); //[]  reg_p0_ck_squ_tri
	_rtd_part_outl(0x1800dc20, 0, 0, 0x0); //[]  reg_p0_b_pi_m_mode
	_rtd_part_outl(0x1800dc20, 3, 1, 0x0); //[]  reg_p0_b_testout_sel
	_rtd_part_outl(0x1800dc20, 8, 4, 0x00); //[]  reg_p0_b_timer_lpf
	_rtd_part_outl(0x1800dc20, 13, 9, 0x00); //[]  reg_p0_b_timer_eq
	_rtd_part_outl(0x1800dc20, 18, 14, 0x00); //[]  reg_p0_b_timer_ber
	_rtd_part_outl(0x1800dc24, 0, 0, 0x0); //[]  reg_p0_g_pi_m_mode
	_rtd_part_outl(0x1800dc24, 3, 1, 0x0); //[]  reg_p0_g_testout_sel
	_rtd_part_outl(0x1800dc24, 8, 4, 0x00); //[]  reg_p0_g_timer_lpf
	_rtd_part_outl(0x1800dc24, 13, 9, 0x00); //[]  reg_p0_g_timer_eq
	_rtd_part_outl(0x1800dc24, 18, 14, 0x00); //[]  reg_p0_g_timer_ber
	_rtd_part_outl(0x1800dc28, 0, 0, 0x0); //[]  reg_p0_r_pi_m_mode
	_rtd_part_outl(0x1800dc28, 3, 1, 0x0); //[]  reg_p0_r_testout_sel
	_rtd_part_outl(0x1800dc28, 8, 4, 0x00); //[]  reg_p0_r_timer_lpf
	_rtd_part_outl(0x1800dc28, 13, 9, 0x00); //[]  reg_p0_r_timer_eq
	_rtd_part_outl(0x1800dc28, 18, 14, 0x00); //[]  reg_p0_r_timer_ber
	_rtd_part_outl(0x1800dc2c, 0, 0, 0x0); //[]  reg_p0_ck_pi_m_mode
	_rtd_part_outl(0x1800dc2c, 3, 1, 0x0); //[]  reg_p0_ck_testout_sel
	_rtd_part_outl(0x1800dc2c, 8, 4, 0x00); //[]  reg_p0_ck_timer_lpf
	_rtd_part_outl(0x1800dc2c, 13, 9, 0x00); //[]  reg_p0_ck_timer_eq
	_rtd_part_outl(0x1800dc2c, 18, 14, 0x00); //[]  reg_p0_ck_timer_ber
	_rtd_part_outl(0x1800dc30, 31, 0, 0x000000ff); //[]  reg_p0_b_st_m_value
	_rtd_part_outl(0x1800dc34, 31, 0, 0x000000ff); //[]  reg_p0_g_st_m_value
	_rtd_part_outl(0x1800dc38, 31, 0, 0x000000ff); //[]  reg_p0_r_st_m_value
	_rtd_part_outl(0x1800dc3c, 31, 0, 0x000000ff); //[]  reg_p0_ck_st_m_value
	//Clock Ready
	_rtd_part_outl(0x1800dc40, 0, 0, 0x0); //[]  reg_p0_b_force_clkrdy
	_rtd_part_outl(0x1800dc40, 1, 1, 0x0); //[]  reg_p0_g_force_clkrdy
	_rtd_part_outl(0x1800dc40, 2, 2, 0x0); //[]  reg_p0_r_force_clkrdy
	_rtd_part_outl(0x1800dc40, 3, 3, 0x0); //[]  reg_p0_ck_force_clkrdy
	_rtd_part_outl(0x1800dc40, 7, 4, 0x0); //[]  reg_p0_b_timer_clk
	_rtd_part_outl(0x1800dc40, 11, 8, 0x0); //[]  reg_p0_g_timer_clk
	_rtd_part_outl(0x1800dc40, 15, 12, 0x0); //[]  reg_p0_r_timer_clk
	_rtd_part_outl(0x1800dc40, 19, 16, 0x0); //[]  reg_p0_ck_timer_clk
	_rtd_part_outl(0x1800dc44, 11, 8, 0xf); //[DPHY fix]  reg_p0_b_offset_delay_cnt
	_rtd_part_outl(0x1800dc44, 13, 12, 0x0); //[DPHY fix]  reg_p0_b_offset_stable_cnt
	_rtd_part_outl(0x1800dc44, 19, 14, 0x0b); //[DPHY fix]  reg_p0_b_offset_divisor
	_rtd_part_outl(0x1800dc44, 26, 20, 0x28); //[DPHY fix]  reg_p0_b_offset_timeout
	_rtd_part_outl(0x1800dc48, 0, 0, 0x0); //[]  reg_p0_b_offset_en_ope
	_rtd_part_outl(0x1800dc48, 1, 1, 0x0); //[]  reg_p0_b_offset_pc_ope
	_rtd_part_outl(0x1800dc48, 6, 2, 0x10); //[]  reg_p0_b_offset_ini_ope
	_rtd_part_outl(0x1800dc48, 7, 7, 0x0); //[]  reg_p0_b_offset_gray_en_ope
	_rtd_part_outl(0x1800dc48, 8, 8, 0x0); //[]  reg_p0_b_offset_manual_ope
	_rtd_part_outl(0x1800dc48, 9, 9, 0x0); //[]  reg_p0_b_offset_z0_ok_ope
	_rtd_part_outl(0x1800dc48, 10, 10, 0x0); //[]  reg_p0_b_offset_en_opo
	_rtd_part_outl(0x1800dc48, 11, 11, 0x0); //[]  reg_p0_b_offset_pc_opo
	_rtd_part_outl(0x1800dc48, 16, 12, 0x10); //[]  reg_p0_b_offset_ini_opo
	_rtd_part_outl(0x1800dc48, 17, 17, 0x0); //[]  reg_p0_b_offset_gray_en_opo
	_rtd_part_outl(0x1800dc48, 18, 18, 0x0); //[]  reg_p0_b_offset_manual_opo
	_rtd_part_outl(0x1800dc48, 19, 19, 0x0); //[]  reg_p0_b_offset_z0_ok_opo
	_rtd_part_outl(0x1800dc48, 20, 20, 0x0); //[]  reg_p0_b_offset_en_one
	_rtd_part_outl(0x1800dc48, 21, 21, 0x0); //[]  reg_p0_b_offset_pc_one
	_rtd_part_outl(0x1800dc48, 26, 22, 0x10); //[]  reg_p0_b_offset_ini_one
	_rtd_part_outl(0x1800dc48, 27, 27, 0x0); //[]  reg_p0_b_offset_gray_en_one
	_rtd_part_outl(0x1800dc48, 28, 28, 0x0); //[]  reg_p0_b_offset_manual_one
	_rtd_part_outl(0x1800dc48, 29, 29, 0x0); //[]  reg_p0_b_offset_z0_ok_one
	_rtd_part_outl(0x1800dc4c, 0, 0, 0x0); //[DPHY fix]  reg_p0_b_offset_en_ono
	_rtd_part_outl(0x1800dc4c, 1, 1, 0x0); //[DPHY fix]  reg_p0_b_offset_pc_ono
	_rtd_part_outl(0x1800dc4c, 6, 2, 0x10); //[DPHY fix]  reg_p0_b_offset_ini_ono
	_rtd_part_outl(0x1800dc4c, 7, 7, 0x0); //[DPHY fix]  reg_p0_b_offset_gray_en_ono
	_rtd_part_outl(0x1800dc4c, 8, 8, 0x0); //[DPHY fix]  reg_p0_b_offset_manual_ono
	_rtd_part_outl(0x1800dc4c, 9, 9, 0x0); //[DPHY fix]  reg_p0_b_offset_z0_ok_ono
	_rtd_part_outl(0x1800dc4c, 16, 12, 0x10); //[DPHY fix]  reg_p0_b_offset_ini_de
	_rtd_part_outl(0x1800dc4c, 17, 17, 0x0); //[DPHY fix]  reg_p0_b_offset_gray_en_de
	_rtd_part_outl(0x1800dc4c, 26, 22, 0x10); //[DPHY fix]  reg_p0_b_offset_ini_do
	_rtd_part_outl(0x1800dc4c, 27, 27, 0x0); //[DPHY fix]  reg_p0_b_offset_gray_en_do
	_rtd_part_outl(0x1800dc50, 6, 2, 0x10); //[DPHY fix]  reg_p0_b_offset_ini_ee
	_rtd_part_outl(0x1800dc50, 7, 7, 0x0); //[DPHY fix]  reg_p0_b_offset_gray_en_ee
	_rtd_part_outl(0x1800dc50, 16, 12, 0x10); //[DPHY fix]  reg_p0_b_offset_ini_eo
	_rtd_part_outl(0x1800dc50, 17, 17, 0x0); //[DPHY fix]  reg_p0_b_offset_gray_en_eo
	_rtd_part_outl(0x1800dc50, 26, 22, 0x10); //[DPHY fix]  reg_p0_b_offset_ini_eq
	_rtd_part_outl(0x1800dc50, 27, 27, 0x0); //[DPHY fix]  reg_p0_b_offset_gray_en_eq
	_rtd_part_outl(0x1800dc50, 29, 29, 0x0); //[DPHY fix]  reg_p0_b_offset_z0_ok_eq
	_rtd_part_outl(0x1800dc54, 11, 8, 0xf); //[DPHY fix]  reg_p0_g_offset_delay_cnt
	_rtd_part_outl(0x1800dc54, 13, 12, 0x0); //[DPHY fix]  reg_p0_g_offset_stable_cnt
	_rtd_part_outl(0x1800dc54, 19, 14, 0x0b); //[DPHY fix]  reg_p0_g_offset_divisor
	_rtd_part_outl(0x1800dc54, 26, 20, 0x28); //[DPHY fix]  reg_p0_g_offset_timeout
	_rtd_part_outl(0x1800dc58, 0, 0, 0x0); //[]  reg_p0_g_offset_en_ope
	_rtd_part_outl(0x1800dc58, 1, 1, 0x0); //[]  reg_p0_g_offset_pc_ope
	_rtd_part_outl(0x1800dc58, 6, 2, 0x10); //[]  reg_p0_g_offset_ini_ope
	_rtd_part_outl(0x1800dc58, 7, 7, 0x0); //[]  reg_p0_g_offset_gray_en_ope
	_rtd_part_outl(0x1800dc58, 8, 8, 0x0); //[]  reg_p0_g_offset_manual_ope
	_rtd_part_outl(0x1800dc58, 9, 9, 0x0); //[]  reg_p0_g_offset_z0_ok_ope
	_rtd_part_outl(0x1800dc58, 10, 10, 0x0); //[]  reg_p0_g_offset_en_opo
	_rtd_part_outl(0x1800dc58, 11, 11, 0x0); //[]  reg_p0_g_offset_pc_opo
	_rtd_part_outl(0x1800dc58, 16, 12, 0x10); //[]  reg_p0_g_offset_ini_opo
	_rtd_part_outl(0x1800dc58, 17, 17, 0x0); //[]  reg_p0_g_offset_gray_en_opo
	_rtd_part_outl(0x1800dc58, 18, 18, 0x0); //[]  reg_p0_g_offset_manual_opo
	_rtd_part_outl(0x1800dc58, 19, 19, 0x0); //[]  reg_p0_g_offset_z0_ok_opo
	_rtd_part_outl(0x1800dc58, 20, 20, 0x0); //[]  reg_p0_g_offset_en_one
	_rtd_part_outl(0x1800dc58, 21, 21, 0x0); //[]  reg_p0_g_offset_pc_one
	_rtd_part_outl(0x1800dc58, 26, 22, 0x10); //[]  reg_p0_g_offset_ini_one
	_rtd_part_outl(0x1800dc58, 27, 27, 0x0); //[]  reg_p0_g_offset_gray_en_one
	_rtd_part_outl(0x1800dc58, 28, 28, 0x0); //[]  reg_p0_g_offset_manual_one
	_rtd_part_outl(0x1800dc58, 29, 29, 0x0); //[]  reg_p0_g_offset_z0_ok_one
	_rtd_part_outl(0x1800dc5c, 0, 0, 0x0); //[DPHY fix]  reg_p0_g_offset_en_ono
	_rtd_part_outl(0x1800dc5c, 6, 2, 0x10); //[DPHY fix]  reg_p0_g_offset_ini_ono
	_rtd_part_outl(0x1800dc5c, 7, 7, 0x0); //[DPHY fix]  reg_p0_g_offset_gray_en_ono
	_rtd_part_outl(0x1800dc5c, 1, 1, 0x0); //[DPHY fix]  reg_p0_g_offset_pc_ono
	_rtd_part_outl(0x1800dc5c, 8, 8, 0x0); //[DPHY fix]  reg_p0_g_offset_manual_ono
	_rtd_part_outl(0x1800dc5c, 9, 9, 0x0); //[DPHY fix]  reg_p0_g_offset_z0_ok_ono
	_rtd_part_outl(0x1800dc5c, 16, 12, 0x10); //[DPHY fix]  reg_p0_g_offset_ini_de
	_rtd_part_outl(0x1800dc5c, 17, 17, 0x0); //[DPHY fix]  reg_p0_g_offset_gray_en_de
	_rtd_part_outl(0x1800dc5c, 26, 22, 0x10); //[DPHY fix]  reg_p0_g_offset_ini_do
	_rtd_part_outl(0x1800dc5c, 27, 27, 0x0); //[DPHY fix]  reg_p0_g_offset_gray_en_do
	_rtd_part_outl(0x1800dc60, 6, 2, 0x10); //[DPHY fix]  reg_p0_g_offset_ini_ee
	_rtd_part_outl(0x1800dc60, 7, 7, 0x0); //[DPHY fix]  reg_p0_g_offset_gray_en_ee
	_rtd_part_outl(0x1800dc60, 16, 12, 0x10); //[DPHY fix]  reg_p0_g_offset_ini_eo
	_rtd_part_outl(0x1800dc60, 17, 17, 0x0); //[DPHY fix]  reg_p0_g_offset_gray_en_eo
	_rtd_part_outl(0x1800dc60, 26, 22, 0x10); //[DPHY fix]  reg_p0_g_offset_ini_eq
	_rtd_part_outl(0x1800dc60, 27, 27, 0x0); //[DPHY fix]  reg_p0_g_offset_gray_en_eq
	_rtd_part_outl(0x1800dc60, 29, 29, 0x0); //[DPHY fix]  reg_p0_g_offset_z0_ok_eq
	_rtd_part_outl(0x1800dc64, 11, 8, 0xf); //[DPHY fix]  reg_p0_r_offset_delay_cnt
	_rtd_part_outl(0x1800dc64, 13, 12, 0x0); //[DPHY fix]  reg_p0_r_offset_stable_cnt
	_rtd_part_outl(0x1800dc64, 19, 14, 0x0b); //[DPHY fix]  reg_p0_r_offset_divisor
	_rtd_part_outl(0x1800dc64, 26, 20, 0x28); //[DPHY fix]  reg_p0_r_offset_timeout
	_rtd_part_outl(0x1800dc68, 0, 0, 0x0); //[]  reg_p0_r_offset_en_ope
	_rtd_part_outl(0x1800dc68, 1, 1, 0x0); //[]  reg_p0_r_offset_pc_ope
	_rtd_part_outl(0x1800dc68, 6, 2, 0x10); //[]  reg_p0_r_offset_ini_ope
	_rtd_part_outl(0x1800dc68, 7, 7, 0x0); //[]  reg_p0_r_offset_gray_en_ope
	_rtd_part_outl(0x1800dc68, 8, 8, 0x0); //[]  reg_p0_r_offset_manual_ope
	_rtd_part_outl(0x1800dc68, 9, 9, 0x0); //[]  reg_p0_r_offset_z0_ok_ope
	_rtd_part_outl(0x1800dc68, 10, 10, 0x0); //[]  reg_p0_r_offset_en_opo
	_rtd_part_outl(0x1800dc68, 11, 11, 0x0); //[]  reg_p0_r_offset_pc_opo
	_rtd_part_outl(0x1800dc68, 16, 12, 0x10); //[]  reg_p0_r_offset_ini_opo
	_rtd_part_outl(0x1800dc68, 17, 17, 0x0); //[]  reg_p0_r_offset_gray_en_opo
	_rtd_part_outl(0x1800dc68, 18, 18, 0x0); //[]  reg_p0_r_offset_manual_opo
	_rtd_part_outl(0x1800dc68, 19, 19, 0x0); //[]  reg_p0_r_offset_z0_ok_opo
	_rtd_part_outl(0x1800dc68, 20, 20, 0x0); //[]  reg_p0_r_offset_en_one
	_rtd_part_outl(0x1800dc68, 21, 21, 0x0); //[]  reg_p0_r_offset_pc_one
	_rtd_part_outl(0x1800dc68, 26, 22, 0x10); //[]  reg_p0_r_offset_ini_one
	_rtd_part_outl(0x1800dc68, 27, 27, 0x0); //[]  reg_p0_r_offset_gray_en_one
	_rtd_part_outl(0x1800dc68, 28, 28, 0x0); //[]  reg_p0_r_offset_manual_one
	_rtd_part_outl(0x1800dc68, 29, 29, 0x0); //[]  reg_p0_r_offset_z0_ok_one
	_rtd_part_outl(0x1800dc6c, 0, 0, 0x0); //[DPHY fix]  reg_p0_r_offset_en_ono
	_rtd_part_outl(0x1800dc6c, 1, 1, 0x0); //[DPHY fix]  reg_p0_r_offset_pc_ono
	_rtd_part_outl(0x1800dc6c, 6, 2, 0x10); //[DPHY fix]  reg_p0_r_offset_ini_ono
	_rtd_part_outl(0x1800dc6c, 7, 7, 0x0); //[DPHY fix]  reg_p0_r_offset_gray_en_ono
	_rtd_part_outl(0x1800dc6c, 8, 8, 0x0); //[DPHY fix]  reg_p0_r_offset_manual_ono
	_rtd_part_outl(0x1800dc6c, 9, 9, 0x0); //[DPHY fix]  reg_p0_r_offset_z0_ok_ono
	_rtd_part_outl(0x1800dc6c, 16, 12, 0x10); //[DPHY fix]  reg_p0_r_offset_ini_de
	_rtd_part_outl(0x1800dc6c, 17, 17, 0x0); //[DPHY fix]  reg_p0_r_offset_gray_en_de
	_rtd_part_outl(0x1800dc6c, 26, 22, 0x10); //[DPHY fix]  reg_p0_r_offset_ini_do
	_rtd_part_outl(0x1800dc6c, 27, 27, 0x0); //[DPHY fix]  reg_p0_r_offset_gray_en_do
	_rtd_part_outl(0x1800dc70, 6, 2, 0x10); //[DPHY fix]  reg_p0_r_offset_ini_ee
	_rtd_part_outl(0x1800dc70, 7, 7, 0x0); //[DPHY fix]  reg_p0_r_offset_gray_en_ee
	_rtd_part_outl(0x1800dc70, 16, 12, 0x10); //[DPHY fix]  reg_p0_r_offset_ini_eo
	_rtd_part_outl(0x1800dc70, 17, 17, 0x0); //[DPHY fix]  reg_p0_r_offset_gray_en_eo
	_rtd_part_outl(0x1800dc70, 26, 22, 0x10); //[DPHY fix]  reg_p0_r_offset_ini_eq
	_rtd_part_outl(0x1800dc70, 27, 27, 0x0); //[DPHY fix]  reg_p0_r_offset_gray_en_eq
	_rtd_part_outl(0x1800dc70, 29, 29, 0x0); //[DPHY fix]  reg_p0_r_offset_z0_ok_eq
	_rtd_part_outl(0x1800dc74, 11, 8, 0xf); //[DPHY fix]  reg_p0_ck_offset_delay_cnt
	_rtd_part_outl(0x1800dc74, 13, 12, 0x0); //[DPHY fix]  reg_p0_ck_offset_stable_cnt
	_rtd_part_outl(0x1800dc74, 19, 14, 0x0b); //[DPHY fix]  reg_p0_ck_offset_divisor
	_rtd_part_outl(0x1800dc74, 26, 20, 0x28); //[DPHY fix]  reg_p0_ck_offset_timeout
	_rtd_part_outl(0x1800dc78, 0, 0, 0x0); //[]  reg_p0_ck_offset_en_ope
	_rtd_part_outl(0x1800dc78, 1, 1, 0x0); //[]  reg_p0_ck_offset_pc_ope
	_rtd_part_outl(0x1800dc78, 6, 2, 0x10); //[]  reg_p0_ck_offset_ini_ope
	_rtd_part_outl(0x1800dc78, 7, 7, 0x0); //[]  reg_p0_ck_offset_gray_en_ope
	_rtd_part_outl(0x1800dc78, 8, 8, 0x0); //[]  reg_p0_ck_offset_manual_ope
	_rtd_part_outl(0x1800dc78, 9, 9, 0x0); //[]  reg_p0_ck_offset_z0_ok_ope
	_rtd_part_outl(0x1800dc78, 10, 10, 0x0); //[]  reg_p0_ck_offset_en_opo
	_rtd_part_outl(0x1800dc78, 11, 11, 0x0); //[]  reg_p0_ck_offset_pc_opo
	_rtd_part_outl(0x1800dc78, 16, 12, 0x10); //[]  reg_p0_ck_offset_ini_opo
	_rtd_part_outl(0x1800dc78, 17, 17, 0x0); //[]  reg_p0_ck_offset_gray_en_opo
	_rtd_part_outl(0x1800dc78, 18, 18, 0x0); //[]  reg_p0_ck_offset_manual_opo
	_rtd_part_outl(0x1800dc78, 19, 19, 0x0); //[]  reg_p0_ck_offset_z0_ok_opo
	_rtd_part_outl(0x1800dc78, 20, 20, 0x0); //[]  reg_p0_ck_offset_en_one
	_rtd_part_outl(0x1800dc78, 21, 21, 0x0); //[]  reg_p0_ck_offset_pc_one
	_rtd_part_outl(0x1800dc78, 26, 22, 0x10); //[]  reg_p0_ck_offset_ini_one
	_rtd_part_outl(0x1800dc78, 27, 27, 0x0); //[]  reg_p0_ck_offset_gray_en_one
	_rtd_part_outl(0x1800dc78, 28, 28, 0x0); //[]  reg_p0_ck_offset_manual_one
	_rtd_part_outl(0x1800dc78, 29, 29, 0x0); //[]  reg_p0_ck_offset_z0_ok_one
	_rtd_part_outl(0x1800dc7c, 0, 0, 0x0); //[DPHY fix]  reg_p0_ck_offset_en_ono
	_rtd_part_outl(0x1800dc7c, 1, 1, 0x0); //[DPHY fix]  reg_p0_ck_offset_pc_ono
	_rtd_part_outl(0x1800dc7c, 6, 2, 0x10); //[DPHY fix]  reg_p0_ck_offset_ini_ono
	_rtd_part_outl(0x1800dc7c, 7, 7, 0x0); //[DPHY fix]  reg_p0_ck_offset_gray_en_ono
	_rtd_part_outl(0x1800dc7c, 8, 8, 0x0); //[DPHY fix]  reg_p0_ck_offset_manual_ono
	_rtd_part_outl(0x1800dc7c, 9, 9, 0x0); //[DPHY fix]  reg_p0_ck_offset_z0_ok_ono
	_rtd_part_outl(0x1800dc7c, 16, 12, 0x10); //[DPHY fix]  reg_p0_ck_offset_ini_de
	_rtd_part_outl(0x1800dc7c, 17, 17, 0x0); //[DPHY fix]  reg_p0_ck_offset_gray_en_de
	_rtd_part_outl(0x1800dc7c, 26, 22, 0x10); //[DPHY fix]  reg_p0_ck_offset_ini_do
	_rtd_part_outl(0x1800dc7c, 27, 27, 0x0); //[DPHY fix]  reg_p0_ck_offset_gray_en_do
	_rtd_part_outl(0x1800dc80, 6, 2, 0x10); //[DPHY fix]  reg_p0_ck_offset_ini_ee
	_rtd_part_outl(0x1800dc80, 7, 7, 0x0); //[DPHY fix]  reg_p0_ck_offset_gray_en_ee
	_rtd_part_outl(0x1800dc80, 16, 12, 0x10); //[DPHY fix]  reg_p0_ck_offset_ini_eo
	_rtd_part_outl(0x1800dc80, 17, 17, 0x0); //[DPHY fix]  reg_p0_ck_offset_gray_en_eo
	_rtd_part_outl(0x1800dc80, 26, 22, 0x10); //[DPHY fix]  reg_p0_ck_offset_ini_eq
	_rtd_part_outl(0x1800dc80, 27, 27, 0x0); //[DPHY fix]  reg_p0_ck_offset_gray_en_eq
	_rtd_part_outl(0x1800dc80, 29, 29, 0x0); //[DPHY fix]  reg_p0_ck_offset_z0_ok_eq
	_rtd_part_outl(0x1800dc88, 2, 2, 0x0); //[DPHY fix]  reg_p0_b_pn_swap_en
	_rtd_part_outl(0x1800dc88, 3, 3, 0x1); //[DPHY fix]  reg_p0_b_dfe_data_en
	_rtd_part_outl(0x1800dc88, 7, 5, 0x3); //[DPHY fix]  reg_p0_b_inbuf_num
	_rtd_part_outl(0x1800dc88, 10, 10, 0x0); //[DPHY fix]  reg_p0_g_pn_swap_en
	_rtd_part_outl(0x1800dc88, 11, 11, 0x1); //[DPHY fix]  reg_p0_g_dfe_data_en
	_rtd_part_outl(0x1800dc88, 15, 13, 0x3); //[DPHY fix]  reg_p0_g_inbuf_num
	_rtd_part_outl(0x1800dc88, 18, 18, 0x0); //[DPHY fix]  reg_p0_r_pn_swap_en
	_rtd_part_outl(0x1800dc88, 19, 19, 0x1); //[DPHY fix]  reg_p0_r_dfe_data_en
	_rtd_part_outl(0x1800dc88, 23, 21, 0x3); //[DPHY fix]  reg_p0_r_fifo_en
	_rtd_part_outl(0x1800dc88, 26, 26, 0x0); //[DPHY fix]  reg_p0_ck_pn_swap_en
	_rtd_part_outl(0x1800dc88, 27, 27, 0x1); //[DPHY fix]  reg_p0_ck_dfe_data_en
	_rtd_part_outl(0x1800dc88, 31, 29, 0x3); //[DPHY fix]  reg_p0_ck_inbuf_num
	_rtd_part_outl(0x1800dc8c, 0, 0, 0x0); //[]  reg_p0_b_001_enable
	_rtd_part_outl(0x1800dc8c, 1, 1, 0x0); //[]  reg_p0_b_101_enable
	_rtd_part_outl(0x1800dc8c, 2, 2, 0x0); //[]  reg_p0_b_en_bec_acc
	_rtd_part_outl(0x1800dc8c, 3, 3, 0x0); //[]  reg_p0_b_en_bec_read
	_rtd_part_outl(0x1800dc8c, 12, 12, 0x0); //[]  reg_p0_g_001_enable
	_rtd_part_outl(0x1800dc8c, 13, 13, 0x0); //[]  reg_p0_g_101_enable
	_rtd_part_outl(0x1800dc8c, 14, 14, 0x0); //[]  reg_p0_g_en_bec_acc
	_rtd_part_outl(0x1800dc8c, 15, 15, 0x0); //[]  reg_p0_g_en_bec_read
	_rtd_part_outl(0x1800dc90, 0, 0, 0x0); //[]  reg_p0_r_001_enable
	_rtd_part_outl(0x1800dc90, 1, 1, 0x0); //[]  reg_p0_r_101_enable
	_rtd_part_outl(0x1800dc90, 2, 2, 0x0); //[]  reg_p0_r_en_bec_acc
	_rtd_part_outl(0x1800dc90, 3, 3, 0x0); //[]  reg_p0_r_en_bec_read
	_rtd_part_outl(0x1800dc90, 12, 12, 0x0); //[]  reg_p0_ck_001_enable
	_rtd_part_outl(0x1800dc90, 13, 13, 0x0); //[]  reg_p0_ck_101_enable
	_rtd_part_outl(0x1800dc90, 14, 14, 0x0); //[]  reg_p0_ck_en_bec_acc
	_rtd_part_outl(0x1800dc90, 15, 15, 0x0); //[]  reg_p0_ck_en_bec_read
	_rtd_part_outl(0x1800dcb8, 0, 0, 0x0); //[DPHY fix]  reg_p0_b_cp2adp_en
	_rtd_part_outl(0x1800dcb8, 1, 1, 0x0); //[DPHY fix]  reg_p0_b_stable_time_mode
	_rtd_part_outl(0x1800dcb8, 2, 2, 0x0); //[DPHY fix]  reg_p0_b_bypass_k_band_mode
	_rtd_part_outl(0x1800dcb8, 18, 18, 0x0); //[DPHY fix]  reg_p0_b_calib_late
	_rtd_part_outl(0x1800dcb8, 19, 19, 0x0); //[DPHY fix]  reg_p0_b_calib_manual
	_rtd_part_outl(0x1800dcb8, 22, 20, 0x2); //[DPHY fix]  reg_p0_b_calib_time
	_rtd_part_outl(0x1800dcb8, 27, 23, 0x0c); //[DPHY fix]  reg_p0_b_adp_time
	_rtd_part_outl(0x1800dcb8, 28, 28, 0x0); //[DPHY fix]  reg_p0_b_adap_eq_off
	_rtd_part_outl(0x1800dcb8, 29, 29, 0x0); //[DPHY fix]  reg_p0_b_cp_en_manual
	_rtd_part_outl(0x1800dcb8, 30, 30, 0x0); //[DPHY fix]  reg_p0_b_adp_en_manual
	_rtd_part_outl(0x1800dcb8, 31, 31, 0x1); //[DPHY fix]  reg_p0_b_auto_mode
	_rtd_part_outl(0x1800dcbc, 31, 28, 0x8); //[DPHY fix] reg_p0_b_cp2adp_time
	_rtd_part_outl(0x1800dcc0, 1, 1, 0x1); //[] reg_p0_b_calib_reset_sel
	_rtd_part_outl(0x1800dcc0, 3, 2, 0x0); //[] reg_p0_b_vc_sel
	_rtd_part_outl(0x1800dcc0, 5, 4, 0x0); //[] reg_p0_b_cdr_c
	_rtd_part_outl(0x1800dcc0, 11, 6, 0x00); //[] reg_p0_b_cdr_r
	_rtd_part_outl(0x1800dcc0, 31, 12, DPHY_Fix_frl_1[frl_timing].b_cdr_cp_307); //[] reg_p0_b_cdr_cp
	_rtd_part_outl(0x1800dcc4, 2, 0, 0x0); //[DPHY fix] reg_p0_b_init_time
	_rtd_part_outl(0x1800dcc4, 7, 3, 0x04); //[DPHY fix] reg_p0_b_cp_time
	_rtd_part_outl(0x1800dcc8, 6, 2, 0x01); //[DPHY fix]  reg_p0_b_timer_6
	_rtd_part_outl(0x1800dcc8, 7, 7, 0x0); //[DPHY fix]  reg_p0_b_timer_5
	_rtd_part_outl(0x1800dcc8, 11, 8, 0x7); //[DPHY fix]  reg_p0_b_vco_fine_init
	_rtd_part_outl(0x1800dcc8, 15, 12, 0x7); //[DPHY fix]  reg_p0_b_vco_coarse_init
	_rtd_part_outl(0x1800dcc8, 16, 16, 0x0); //[DPHY fix]  reg_p0_b_bypass_coarse_k_mode
	_rtd_part_outl(0x1800dcc8, 18, 17, 0x0); //[DPHY fix]  reg_p0_b_vco_fine_time_sel
	_rtd_part_outl(0x1800dcc8, 19, 19, 0x0); //[DPHY fix]  reg_p0_b_xtal_24m_en
	_rtd_part_outl(0x1800dcc8, 23, 20, 0x0); //[DPHY fix]  reg_p0_b_vco_fine_manual
	_rtd_part_outl(0x1800dcc8, 27, 24, 0x0); //[DPHY fix]  reg_p0_b_vco_coarse_manual
	_rtd_part_outl(0x1800dcc8, 28, 28, 0x0); //[DPHY fix]  reg_p0_b_coarse_calib_manual
	_rtd_part_outl(0x1800dcc8, 29, 29, 0x0); //[DPHY fix]  reg_p0_b_acdr_ckfld_en
	_rtd_part_outl(0x1800dcc8, 30, 30, 0x0); //[DPHY fix]  reg_p0_b_pll_to_acdr_manual_en
	_rtd_part_outl(0x1800dcc8, 31, 31, 0x0); //[DPHY fix]  reg_p0_b_pll_to_acdr_rdy_manual
	_rtd_part_outl(0x1800dccc, 4, 0, 0x01); //[]  reg_p0_b_timer_4
	_rtd_part_outl(0x1800dccc, 7, 5, 0x0); //[]  reg_p0_b_vc_chg_time
	_rtd_part_outl(0x1800dccc, 16, 16, 0x1); //[]  reg_p0_b_old_mode
	_rtd_part_outl(0x1800dccc, 21, 17, 0x10); //[]  reg_p0_b_slope_band
	_rtd_part_outl(0x1800dccc, 22, 22, 0x0); //[]  reg_p0_b_slope_manual
	_rtd_part_outl(0x1800dccc, 23, 23, 0x0); //[]  reg_p0_b_rxidle_bypass
	_rtd_part_outl(0x1800dccc, 24, 24, 0x0); //[]  reg_p0_b_pfd_bypass
	_rtd_part_outl(0x1800dccc, 29, 25, 0x00); //[]  reg_p0_b_pfd_time
	_rtd_part_outl(0x1800dccc, 30, 30, 0x0); //[]  reg_p0_b_pfd_en_manual
	_rtd_part_outl(0x1800dccc, 31, 31, 0x0); //[]  reg_p0_b_start_en_manual
	_rtd_part_outl(0x1800dcd0, 9, 5, 0x08); //[]  reg_p0_b_cp_time_2
	_rtd_part_outl(0x1800dcd8, 0, 0, 0x0); //[DPHY fix]  reg_p0_g_cp2adp_en
	_rtd_part_outl(0x1800dcd8, 1, 1, 0x0); //[DPHY fix]  reg_p0_g_stable_time_mode
	_rtd_part_outl(0x1800dcd8, 2, 2, 0x0); //[DPHY fix]  reg_p0_g_bypass_k_band_mode
	_rtd_part_outl(0x1800dcd8, 18, 18, 0x0); //[DPHY fix]  reg_p0_g_calib_late
	_rtd_part_outl(0x1800dcd8, 19, 19, 0x0); //[DPHY fix]  reg_p0_g_calib_manual
	_rtd_part_outl(0x1800dcd8, 22, 20, 0x2); //[DPHY fix]  reg_p0_g_calib_time
	_rtd_part_outl(0x1800dcd8, 27, 23, 0x0c); //[DPHY fix]  reg_p0_g_adp_time
	_rtd_part_outl(0x1800dcd8, 28, 28, 0x0); //[DPHY fix]  reg_p0_g_adap_eq_off
	_rtd_part_outl(0x1800dcd8, 29, 29, 0x0); //[DPHY fix]  reg_p0_g_cp_en_manual
	_rtd_part_outl(0x1800dcd8, 30, 30, 0x0); //[DPHY fix]  reg_p0_g_adp_en_manual
	_rtd_part_outl(0x1800dcd8, 31, 31, 0x1); //[DPHY fix]  reg_p0_g_auto_mode
	_rtd_part_outl(0x1800dcdc, 31, 28, 0x8); //[DPHY fix] reg_p0_g_cp2adp_time
	_rtd_part_outl(0x1800dce0, 1, 1, 0x1); //[] reg_p0_g_calib_reset_sel
	_rtd_part_outl(0x1800dce0, 3, 2, 0x0); //[] reg_p0_g_vc_sel
	_rtd_part_outl(0x1800dce0, 5, 4, 0x0); //[] reg_p0_g_cdr_c
	_rtd_part_outl(0x1800dce0, 11, 6, 0x00); //[] reg_p0_g_cdr_r
	_rtd_part_outl(0x1800dce0, 31, 12, DPHY_Fix_frl_1[frl_timing].g_cdr_cp_350); //[] reg_p0_g_cdr_cp
	_rtd_part_outl(0x1800dce4, 2, 0, 0x0); //[DPHY fix] reg_p0_g_init_time
	_rtd_part_outl(0x1800dce4, 7, 3, 0x04); //[DPHY fix] reg_p0_g_cp_time
	_rtd_part_outl(0x1800dce8, 6, 2, 0x01); //[DPHY fix]  reg_p0_g_timer_6
	_rtd_part_outl(0x1800dce8, 7, 7, 0x0); //[DPHY fix]  reg_p0_g_timer_5
	_rtd_part_outl(0x1800dce8, 11, 8, 0x7); //[DPHY fix]  reg_p0_g_vco_fine_init
	_rtd_part_outl(0x1800dce8, 15, 12, 0x7); //[DPHY fix]  reg_p0_g_vco_coarse_init
	_rtd_part_outl(0x1800dce8, 16, 16, 0x0); //[DPHY fix]  reg_p0_g_bypass_coarse_k_mode
	_rtd_part_outl(0x1800dce8, 18, 17, 0x0); //[DPHY fix]  reg_p0_g_vco_fine_time_sel
	_rtd_part_outl(0x1800dce8, 19, 19, 0x0); //[DPHY fix]  reg_p0_g_xtal_24m_en
	_rtd_part_outl(0x1800dce8, 23, 20, 0x0); //[DPHY fix]  reg_p0_g_vco_fine_manual
	_rtd_part_outl(0x1800dce8, 27, 24, 0x0); //[DPHY fix]  reg_p0_g_vco_coarse_manual
	_rtd_part_outl(0x1800dce8, 28, 28, 0x0); //[DPHY fix]  reg_p0_g_coarse_calib_manual
	_rtd_part_outl(0x1800dce8, 29, 29, 0x0); //[DPHY fix]  reg_p0_g_acdr_ckfld_en
	_rtd_part_outl(0x1800dce8, 30, 30, 0x0); //[DPHY fix]  reg_p0_g_pll_to_acdr_manual_en
	_rtd_part_outl(0x1800dce8, 31, 31, 0x0); //[DPHY fix]  reg_p0_g_pll_to_acdr_rdy_manual
	_rtd_part_outl(0x1800dcec, 4, 0, 0x01); //[]  reg_p0_g_timer_4
	_rtd_part_outl(0x1800dcec, 7, 5, 0x0); //[]  reg_p0_g_vc_chg_time
	_rtd_part_outl(0x1800dcec, 16, 16, 0x1); //[]  reg_p0_g_old_mode
	_rtd_part_outl(0x1800dcec, 21, 17, 0x10); //[]  reg_p0_g_slope_band
	_rtd_part_outl(0x1800dcec, 22, 22, 0x0); //[]  reg_p0_g_slope_manual
	_rtd_part_outl(0x1800dcec, 23, 23, 0x0); //[]  reg_p0_g_rxidle_bypass
	_rtd_part_outl(0x1800dcec, 24, 24, 0x0); //[]  reg_p0_g_pfd_bypass
	_rtd_part_outl(0x1800dcec, 29, 25, 0x00); //[]  reg_p0_g_pfd_time
	_rtd_part_outl(0x1800dcec, 30, 30, 0x0); //[]  reg_p0_g_pfd_en_manual
	_rtd_part_outl(0x1800dcec, 31, 31, 0x0); //[]  reg_p0_g_start_en_manual
	_rtd_part_outl(0x1800dcf0, 9, 5, 0x08); //[]  reg_p0_g_cp_time_2
	_rtd_part_outl(0x1800dcf4, 0, 0, 0x0); //[DPHY fix]  reg_p0_r_cp2adp_en
	_rtd_part_outl(0x1800dcf4, 1, 1, 0x0); //[DPHY fix]  reg_p0_r_stable_time_mode
	_rtd_part_outl(0x1800dcf4, 2, 2, 0x0); //[DPHY fix]  reg_p0_r_bypass_k_band_mode
	_rtd_part_outl(0x1800dcf4, 18, 18, 0x0); //[DPHY fix]  reg_p0_r_calib_late
	_rtd_part_outl(0x1800dcf4, 19, 19, 0x0); //[DPHY fix]  reg_p0_r_calib_manual
	_rtd_part_outl(0x1800dcf4, 22, 20, 0x2); //[DPHY fix]  reg_p0_r_calib_time
	_rtd_part_outl(0x1800dcf4, 27, 23, 0x0c); //[DPHY fix]  reg_p0_r_adp_time
	_rtd_part_outl(0x1800dcf4, 28, 28, 0x0); //[DPHY fix]  reg_p0_r_adap_eq_off
	_rtd_part_outl(0x1800dcf4, 29, 29, 0x0); //[DPHY fix]  reg_p0_r_cp_en_manual
	_rtd_part_outl(0x1800dcf4, 30, 30, 0x0); //[DPHY fix]  reg_p0_r_adp_en_manual
	_rtd_part_outl(0x1800dcf4, 31, 31, 0x1); //[DPHY fix]  reg_p0_r_auto_mode
	_rtd_part_outl(0x1800dcf8, 31, 28, 0x8); //[DPHY fix] reg_p0_r_cp2adp_time
	_rtd_part_outl(0x1800dcfc, 1, 1, 0x1); //[] reg_p0_r_calib_reset_sel
	_rtd_part_outl(0x1800dcfc, 3, 2, 0x0); //[] reg_p0_r_vc_sel
	_rtd_part_outl(0x1800dcfc, 5, 4, 0x0); //[] reg_p0_r_cdr_c
	_rtd_part_outl(0x1800dcfc, 11, 6, 0x00); //[] reg_p0_r_cdr_r
	_rtd_part_outl(0x1800dcfc, 31, 12, DPHY_Fix_frl_1[frl_timing].r_cdr_cp_393); //[] reg_p0_r_cdr_cp
	_rtd_part_outl(0x1800dd00, 2, 0, 0x0); //[DPHY fix] reg_p0_r_init_time
	_rtd_part_outl(0x1800dd00, 7, 3, 0x04); //[DPHY fix] reg_p0_r_cp_time
	_rtd_part_outl(0x1800dd04, 6, 2, 0x01); //[DPHY fix]  reg_p0_r_timer_6
	_rtd_part_outl(0x1800dd04, 7, 7, 0x0); //[DPHY fix]  reg_p0_r_timer_5
	_rtd_part_outl(0x1800dd04, 11, 8, 0x7); //[DPHY fix]  reg_p0_r_vco_fine_init
	_rtd_part_outl(0x1800dd04, 15, 12, 0x7); //[DPHY fix]  reg_p0_r_vco_coarse_init
	_rtd_part_outl(0x1800dd04, 16, 16, 0x0); //[DPHY fix]  reg_p0_r_bypass_coarse_k_mode
	_rtd_part_outl(0x1800dd04, 18, 17, 0x0); //[DPHY fix]  reg_p0_r_vco_fine_time_sel
	_rtd_part_outl(0x1800dd04, 19, 19, 0x0); //[DPHY fix]  reg_p0_r_xtal_24m_en
	_rtd_part_outl(0x1800dd04, 23, 20, 0x0); //[DPHY fix]  reg_p0_r_vco_fine_manual
	_rtd_part_outl(0x1800dd04, 27, 24, 0x0); //[DPHY fix]  reg_p0_r_vco_coarse_manual
	_rtd_part_outl(0x1800dd04, 28, 28, 0x0); //[DPHY fix]  reg_p0_r_coarse_calib_manual
	_rtd_part_outl(0x1800dd04, 29, 29, 0x0); //[DPHY fix]  reg_p0_r_acdr_ckfld_en
	_rtd_part_outl(0x1800dd04, 30, 30, 0x0); //[DPHY fix]  reg_p0_r_pll_to_acdr_manual_en
	_rtd_part_outl(0x1800dd04, 31, 31, 0x0); //[DPHY fix]  reg_p0_r_pll_to_acdr_rdy_manual
	_rtd_part_outl(0x1800dd08, 4, 0, 0x01); //[]  reg_p0_r_timer_4
	_rtd_part_outl(0x1800dd08, 7, 5, 0x0); //[]  reg_p0_r_vc_chg_time
	_rtd_part_outl(0x1800dd08, 16, 16, 0x1); //[]  reg_p0_r_old_mode
	_rtd_part_outl(0x1800dd08, 21, 17, 0x10); //[]  reg_p0_r_slope_band
	_rtd_part_outl(0x1800dd08, 22, 22, 0x0); //[]  reg_p0_r_slope_manual
	_rtd_part_outl(0x1800dd08, 23, 23, 0x0); //[]  reg_p0_r_rxidle_bypass
	_rtd_part_outl(0x1800dd08, 24, 24, 0x0); //[]  reg_p0_r_pfd_bypass
	_rtd_part_outl(0x1800dd08, 29, 25, 0x00); //[]  reg_p0_r_pfd_time
	_rtd_part_outl(0x1800dd08, 30, 30, 0x0); //[]  reg_p0_r_pfd_en_manual
	_rtd_part_outl(0x1800dd08, 31, 31, 0x0); //[]  reg_p0_r_start_en_manual
	_rtd_part_outl(0x1800dd0c, 9, 5, 0x08); //[]  reg_p0_r_cp_time_2
	_rtd_part_outl(0x1800dd10, 0, 0, 0x0); //[DPHY fix]  reg_p0_ck_cp2adp_en
	_rtd_part_outl(0x1800dd10, 1, 1, 0x0); //[DPHY fix]  reg_p0_ck_stable_time_mode
	_rtd_part_outl(0x1800dd10, 2, 2, 0x0); //[DPHY fix]  reg_p0_ck_bypass_k_band_mode
	_rtd_part_outl(0x1800dd10, 18, 18, 0x0); //[DPHY fix]  reg_p0_ck_calib_late
	_rtd_part_outl(0x1800dd10, 19, 19, 0x0); //[DPHY fix]  reg_p0_ck_calib_manual
	_rtd_part_outl(0x1800dd10, 22, 20, 0x2); //[DPHY fix]  reg_p0_ck_calib_time
	_rtd_part_outl(0x1800dd10, 27, 23, 0x0c); //[DPHY fix]  reg_p0_ck_adp_time
	_rtd_part_outl(0x1800dd10, 28, 28, 0x0); //[DPHY fix]  reg_p0_ck_adap_eq_off
	_rtd_part_outl(0x1800dd10, 29, 29, 0x0); //[DPHY fix]  reg_p0_ck_cp_en_manual
	_rtd_part_outl(0x1800dd10, 30, 30, 0x0); //[DPHY fix]  reg_p0_ck_adp_en_manual
	_rtd_part_outl(0x1800dd10, 31, 31, 0x1); //[DPHY fix]  reg_p0_ck_auto_mode
	_rtd_part_outl(0x1800dd14, 31, 28, 0x8); //[DPHY fix] reg_p0_ck_cp2adp_time
	_rtd_part_outl(0x1800dd18, 1, 1, 0x1); //[] reg_p0_ck_calib_reset_sel
	_rtd_part_outl(0x1800dd18, 3, 2, 0x0); //[] reg_p0_ck_vc_sel
	_rtd_part_outl(0x1800dd18, 5, 4, 0x0); //[] reg_p0_ck_cdr_c
	_rtd_part_outl(0x1800dd18, 11, 6, 0x00); //[] reg_p0_ck_cdr_r
	_rtd_part_outl(0x1800dd18, 31, 12, DPHY_Fix_frl_1[frl_timing].ck_cdr_cp_436); //[] reg_p0_ck_cdr_cp
	_rtd_part_outl(0x1800dd1c, 2, 0, 0x0); //[DPHY fix] reg_p0_ck_init_time
	_rtd_part_outl(0x1800dd1c, 7, 3, 0x04); //[DPHY fix] reg_p0_ck_cp_time
	_rtd_part_outl(0x1800dd20, 6, 2, 0x01); //[DPHY fix]  reg_p0_ck_timer_6
	_rtd_part_outl(0x1800dd20, 7, 7, 0x0); //[DPHY fix]  reg_p0_ck_timer_5
	_rtd_part_outl(0x1800dd20, 11, 8, 0x7); //[DPHY fix]  reg_p0_ck_vco_fine_init
	_rtd_part_outl(0x1800dd20, 15, 12, 0x7); //[DPHY fix]  reg_p0_ck_vco_coarse_init
	_rtd_part_outl(0x1800dd20, 16, 16, 0x0); //[DPHY fix]  reg_p0_ck_bypass_coarse_k_mode
	_rtd_part_outl(0x1800dd20, 18, 17, 0x0); //[DPHY fix]  reg_p0_ck_vco_fine_time_sel
	_rtd_part_outl(0x1800dd20, 19, 19, 0x0); //[DPHY fix]  reg_p0_ck_xtal_24m_en
	_rtd_part_outl(0x1800dd20, 23, 20, 0x0); //[DPHY fix]  reg_p0_ck_vco_fine_manual
	_rtd_part_outl(0x1800dd20, 27, 24, 0x0); //[DPHY fix]  reg_p0_ck_vco_coarse_manual
	_rtd_part_outl(0x1800dd20, 28, 28, 0x0); //[DPHY fix]  reg_p0_ck_coarse_calib_manual
	_rtd_part_outl(0x1800dd20, 29, 29, 0x0); //[DPHY fix]  reg_p0_ck_acdr_ckfld_en
	_rtd_part_outl(0x1800dd20, 30, 30, 0x0); //[DPHY fix]  reg_p0_ck_pll_to_acdr_manual_en
	_rtd_part_outl(0x1800dd20, 31, 31, 0x0); //[DPHY fix]  reg_p0_ck_pll_to_acdr_rdy_manual
	_rtd_part_outl(0x1800dd24, 4, 0, 0x01); //[]  reg_p0_ck_timer_4
	_rtd_part_outl(0x1800dd24, 7, 5, 0x0); //[]  reg_p0_ck_vc_chg_time
	_rtd_part_outl(0x1800dd24, 16, 16, 0x1); //[]  reg_p0_ck_old_mode
	_rtd_part_outl(0x1800dd24, 21, 17, 0x10); //[]  reg_p0_ck_slope_band
	_rtd_part_outl(0x1800dd24, 22, 22, 0x0); //[]  reg_p0_ck_slope_manual
	_rtd_part_outl(0x1800dd24, 23, 23, 0x0); //[]  reg_p0_ck_rxidle_bypass
	_rtd_part_outl(0x1800dd24, 24, 24, 0x0); //[]  reg_p0_ck_pfd_bypass
	_rtd_part_outl(0x1800dd24, 29, 25, 0x00); //[]  reg_p0_ck_pfd_time
	_rtd_part_outl(0x1800dd24, 30, 30, 0x0); //[]  reg_p0_ck_pfd_en_manual
	_rtd_part_outl(0x1800dd24, 31, 31, 0x0); //[]  reg_p0_ck_start_en_manual
	_rtd_part_outl(0x1800dd28, 9, 5, 0x08); //[]  reg_p0_ck_cp_time_2
	//MOD
	_rtd_part_outl(0x1800dd2c, 0, 0, 0x1); //[]  reg_p0_ck_md_rstb
	_rtd_part_outl(0x1800dd2c, 1, 1, 0x1); //[]  reg_p0_ck_md_auto
	_rtd_part_outl(0x1800dd2c, 3, 2, 0x0); //[]  reg_p0_ck_md_sel
	_rtd_part_outl(0x1800dd2c, 7, 4, 0x0); //[]  reg_p0_ck_md_adj
	_rtd_part_outl(0x1800dd2c, 13, 8, 0x03); //[]  reg_p0_ck_md_threshold
	_rtd_part_outl(0x1800dd2c, 20, 14, 0x00); //[]  reg_p0_ck_md_debounce
	_rtd_part_outl(0x1800dd2c, 25, 21, 0x00); //[]  reg_p0_ck_error_limit
	_rtd_part_outl(0x1800dd2c, 29, 26, 0x0); //[]  reg_p0_ck_md_reserved
	_rtd_part_outl(0x1800dd30, 2, 0, 0x0); //[]  reg_p0_mod_sel
	//PLLCDR
	_rtd_part_outl(0x1800dd34, 0, 0, 0x0); //[]  reg_p0_b_pllcdr_manual_enable
	_rtd_part_outl(0x1800dd34, 1, 1, 0x0); //[]  reg_p0_g_pllcdr_manual_enable
	_rtd_part_outl(0x1800dd34, 2, 2, 0x0); //[]  reg_p0_r_pllcdr_manual_enable
	_rtd_part_outl(0x1800dd34, 3, 3, 0x0); //[]  reg_p0_ck_pllcdr_manual_enable
	_rtd_part_outl(0x1800dd34, 4, 4, 0x1); //[]  reg_p0_b_pllcdr_vcoband_manual_enable
	_rtd_part_outl(0x1800dd34, 5, 5, 0x1); //[]  reg_p0_g_pllcdr_vcoband_manual_enable
	_rtd_part_outl(0x1800dd34, 6, 6, 0x1); //[]  reg_p0_r_pllcdr_vcoband_manual_enable
	_rtd_part_outl(0x1800dd34, 7, 7, 0x1); //[]  reg_p0_ck_pllcdr_vcoband_manual_enable
	_rtd_part_outl(0x1800dd34, 19, 16, 0x0); //[]  reg_p0_b_acdr_icp_sel_manual
	_rtd_part_outl(0x1800dd34, 23, 20, 0x0); //[]  reg_p0_g_acdr_icp_sel_manual
	_rtd_part_outl(0x1800dd34, 27, 24, 0x0); //[]  reg_p0_r_acdr_icp_sel_manual
	_rtd_part_outl(0x1800dd34, 31, 28, 0x0); //[]  reg_p0_ck_acdr_icp_sel_manual
	_rtd_part_outl(0x1800dd38, 3, 0, 0x8); //[]  reg_p0_b_acdr_vco_coarse_i_manual
	_rtd_part_outl(0x1800dd38, 7, 4, 0x8); //[]  reg_p0_g_acdr_vco_coarse_i_manual
	_rtd_part_outl(0x1800dd38, 11, 8, 0x8); //[]  reg_p0_r_acdr_vco_coarse_i_manual
	_rtd_part_outl(0x1800dd38, 15, 12, 0x8); //[]  reg_p0_ck_acdr_vco_coarse_i_manual
	_rtd_part_outl(0x1800dd38, 19, 16, 0x0); //[]  reg_p0_b_acdr_vco_fine_i_manual
	_rtd_part_outl(0x1800dd38, 23, 20, 0x0); //[]  reg_p0_g_acdr_vco_fine_i_manual
	_rtd_part_outl(0x1800dd38, 27, 24, 0x0); //[]  reg_p0_r_acdr_vco_fine_i_manual
	_rtd_part_outl(0x1800dd38, 31, 28, 0x0); //[]  reg_p0_ck_acdr_vco_fine_i_manual
	_rtd_part_outl(0x1800dd6c, 0, 0, 0x1); //[DPHY fix]  reg_p0_acdr_en
	_rtd_part_outl(0x1800dd6c, 2, 2, 0x1); //[DPHY fix]  reg_p0_pow_on_manual
	_rtd_part_outl(0x1800dd6c, 3, 3, 0x1); //[DPHY fix]  reg_p0_pow_on_manual_aphy_en
	_rtd_part_outl(0x1800dd6c, 4, 4, 0x0); //[DPHY fix]  reg_p0_pow_on_manual_b_en
	_rtd_part_outl(0x1800dd6c, 5, 5, 0x0); //[DPHY fix]  reg_p0_pow_on_manual_g_en
	_rtd_part_outl(0x1800dd6c, 6, 6, 0x0); //[DPHY fix]  reg_p0_pow_on_manual_r_en
	_rtd_part_outl(0x1800dd6c, 7, 7, 0x0); //[DPHY fix]  reg_p0_pow_on_manual_ck_en
	_rtd_part_outl(0x1800dd6c, 8, 8, 0x1); //[DPHY fix]  reg_p0_pow_save_bypass_b
	_rtd_part_outl(0x1800dd6c, 9, 9, 0x1); //[DPHY fix]  reg_p0_pow_save_bypass_g
	_rtd_part_outl(0x1800dd6c, 10, 10, 0x1); //[DPHY fix]  reg_p0_pow_save_bypass_r
	_rtd_part_outl(0x1800dd6c, 11, 11, 0x1); //[DPHY fix]  reg_p0_pow_save_bypass_ck
	_rtd_part_outl(0x1800dd6c, 12, 12, 0x0); //[DPHY fix]  reg_p0_pow_save_xtal_24m_enable
	_rtd_part_outl(0x1800dd6c, 14, 13, 0x3); //[DPHY fix]  reg_p0_pow_save_rst_dly_sel
	_rtd_part_outl(0x1800dd70, 17, 16, 0x0); //[DPHY fix]  reg_tap1_mask
	_rtd_part_outl(0x1800dd7c, 31, 31, 0x0); //[DPHY fix]  reg_port_out_sel
	_rtd_part_outl(0x1800dd80, 7, 0, 0x00); //[]  reg_p0_dig_reserved_3
	_rtd_part_outl(0x1800dd80, 15, 8, 0x00); //[]  reg_p0_dig_reserved_2
	_rtd_part_outl(0x1800dd80, 23, 16, 0x00); //[]  reg_p0_dig_reserved_1
	_rtd_part_outl(0x1800dd80, 31, 24, 0x00); //[]  reg_p0_dig_reserved_0
}
void DPHY_Para_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing)
{
	// DPHY para start
	_rtd_part_outl(0x1800dc00, 17, 16, DPHY_Para_frl_1[frl_timing].b_rate_sel_2); //[DPHY para]  reg_p0_b_rate_sel
	_rtd_part_outl(0x1800dc00, 19, 18, DPHY_Para_frl_1[frl_timing].g_rate_sel_3); //[DPHY para]  reg_p0_g_rate_sel
	_rtd_part_outl(0x1800dc00, 21, 20, DPHY_Para_frl_1[frl_timing].r_rate_sel_4); //[DPHY para]  reg_p0_r_rate_sel
	_rtd_part_outl(0x1800dc00, 23, 22, DPHY_Para_frl_1[frl_timing].ck_rate_sel_5); //[DPHY para]  reg_p0_ck_rate_sel
	_rtd_part_outl(0x1800dc88, 0, 0, 0x1); //[DPHY para]  reg_p0_b_10b18b_sel
	_rtd_part_outl(0x1800dc88, 1, 1, 0x0); //[DPHY para]  reg_p0_b_clk_div2_en
	_rtd_part_outl(0x1800dc88, 8, 8, 0x1); //[DPHY para]  reg_p0_g_10b18b_sel
	_rtd_part_outl(0x1800dc88, 9, 9, 0x0); //[DPHY para]  reg_p0_g_clk_div2_en
	_rtd_part_outl(0x1800dc88, 16, 16, 0x1); //[DPHY para]  reg_p0_r_10b18b_sel
	_rtd_part_outl(0x1800dc88, 17, 17, 0x0); //[DPHY para]  reg_p0_r_clk_div2_en
	_rtd_part_outl(0x1800dc88, 24, 24, 0x1); //[DPHY para]  reg_p0_ck_10b18b_sel
	_rtd_part_outl(0x1800dc88, 25, 25, 0x0); //[DPHY para]  reg_p0_ck_clk_div2_en
	_rtd_part_outl(0x1800dd3c, 7, 0, 0x0a); //[]  reg_p0_b_acdr_pll_config_1
	_rtd_part_outl(0x1800dd3c, 15, 8, 0xac); //[]  reg_p0_b_acdr_pll_config_2
	_rtd_part_outl(0x1800dd3c, 23, 16, 0x0f); //[]  reg_p0_b_acdr_pll_config_3
	_rtd_part_outl(0x1800dd3c, 31, 24, 0x00); //[]  reg_p0_b_acdr_pll_config_4
	_rtd_part_outl(0x1800dd40, 7, 0, 0x0b); //[]  reg_p0_b_acdr_cdr_config_1
	_rtd_part_outl(0x1800dd40, 15, 8, 0x81); //[]  reg_p0_b_acdr_cdr_config_2
	_rtd_part_outl(0x1800dd40, 23, 16, 0x03); //[]  reg_p0_b_acdr_cdr_config_3
	_rtd_part_outl(0x1800dd40, 31, 24, 0x00); //[]  reg_p0_b_acdr_cdr_config_4
	_rtd_part_outl(0x1800dd44, 7, 0, 0x00); //[]  reg_p0_b_acdr_manual_config_1
	_rtd_part_outl(0x1800dd44, 15, 8, 0x00); //[]  reg_p0_b_acdr_manual_config_2
	_rtd_part_outl(0x1800dd44, 23, 16, 0x00); //[]  reg_p0_b_acdr_manual_config_3
	_rtd_part_outl(0x1800dd44, 31, 24, 0x00); //[]  reg_p0_b_acdr_manual_config_4
	_rtd_part_outl(0x1800dd48, 7, 0, 0x0a); //[]  reg_p0_g_acdr_pll_config_1
	_rtd_part_outl(0x1800dd48, 15, 8, 0xac); //[]  reg_p0_g_acdr_pll_config_2
	_rtd_part_outl(0x1800dd48, 23, 16, 0x0f); //[]  reg_p0_g_acdr_pll_config_3
	_rtd_part_outl(0x1800dd48, 31, 24, 0x00); //[]  reg_p0_g_acdr_pll_config_4
	_rtd_part_outl(0x1800dd4c, 7, 0, 0x0b); //[]  reg_p0_g_acdr_cdr_config_1
	_rtd_part_outl(0x1800dd4c, 15, 8, 0x81); //[]  reg_p0_g_acdr_cdr_config_2
	_rtd_part_outl(0x1800dd4c, 23, 16, 0x03); //[]  reg_p0_g_acdr_cdr_config_3
	_rtd_part_outl(0x1800dd4c, 31, 24, 0x00); //[]  reg_p0_g_acdr_cdr_config_4
	_rtd_part_outl(0x1800dd50, 7, 0, 0x00); //[]  reg_p0_g_acdr_manual_config_1
	_rtd_part_outl(0x1800dd50, 15, 8, 0x00); //[]  reg_p0_g_acdr_manual_config_2
	_rtd_part_outl(0x1800dd50, 23, 16, 0x00); //[]  reg_p0_g_acdr_manual_config_3
	_rtd_part_outl(0x1800dd50, 31, 24, 0x00); //[]  reg_p0_g_acdr_manual_config_4
	_rtd_part_outl(0x1800dd54, 7, 0, 0x0a); //[]  reg_p0_r_acdr_pll_config_1
	_rtd_part_outl(0x1800dd54, 15, 8, 0xac); //[]  reg_p0_r_acdr_pll_config_2
	_rtd_part_outl(0x1800dd54, 23, 16, 0x0f); //[]  reg_p0_r_acdr_pll_config_3
	_rtd_part_outl(0x1800dd54, 31, 24, 0x00); //[]  reg_p0_r_acdr_pll_config_4
	_rtd_part_outl(0x1800dd58, 7, 0, 0x0b); //[]  reg_p0_r_acdr_cdr_config_1
	_rtd_part_outl(0x1800dd58, 15, 8, 0x81); //[]  reg_p0_r_acdr_cdr_config_2
	_rtd_part_outl(0x1800dd58, 23, 16, 0x03); //[]  reg_p0_r_acdr_cdr_config_3
	_rtd_part_outl(0x1800dd58, 31, 24, 0x00); //[]  reg_p0_r_acdr_cdr_config_4
	_rtd_part_outl(0x1800dd5c, 7, 0, 0x00); //[]  reg_p0_r_acdr_manual_config_1
	_rtd_part_outl(0x1800dd5c, 15, 8, 0x00); //[]  reg_p0_r_acdr_manual_config_2
	_rtd_part_outl(0x1800dd5c, 23, 16, 0x00); //[]  reg_p0_r_acdr_manual_config_3
	_rtd_part_outl(0x1800dd5c, 31, 24, 0x00); //[]  reg_p0_r_acdr_manual_config_4
	_rtd_part_outl(0x1800dd60, 7, 0, 0x0e); //[]  reg_p0_ck_acdr_pll_config_1
	_rtd_part_outl(0x1800dd60, 15, 8, 0xbc); //[]  reg_p0_ck_acdr_pll_config_2
	_rtd_part_outl(0x1800dd60, 23, 16, 0x0f); //[]  reg_p0_ck_acdr_pll_config_3
	_rtd_part_outl(0x1800dd60, 31, 24, 0x00); //[]  reg_p0_ck_acdr_pll_config_4
	_rtd_part_outl(0x1800dd64, 7, 0, 0x0b); //[]  reg_p0_ck_acdr_cdr_config_1
	_rtd_part_outl(0x1800dd64, 15, 8, 0x81); //[]  reg_p0_ck_acdr_cdr_config_2
	_rtd_part_outl(0x1800dd64, 23, 16, 0x03); //[]  reg_p0_ck_acdr_cdr_config_3
	_rtd_part_outl(0x1800dd64, 31, 24, 0x00); //[]  reg_p0_ck_acdr_cdr_config_4
	_rtd_part_outl(0x1800dd68, 7, 0, 0x00); //[]  reg_p0_ck_acdr_manual_config_1
	_rtd_part_outl(0x1800dd68, 15, 8, 0x00); //[]  reg_p0_ck_acdr_manual_config_2
	_rtd_part_outl(0x1800dd68, 23, 16, 0x00); //[]  reg_p0_ck_acdr_manual_config_3
	_rtd_part_outl(0x1800dd68, 31, 24, 0x00); //[]  reg_p0_ck_acdr_manual_config_4
}

void APHY_Fix_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing)
{
	//APHY fix start
	//_rtd_part_outl(0x1800da04, 4, 0, 0x0b); //[]  REG_Z0_ADJR_0
	_rtd_part_outl(0x1800da08, 0, 0, 0x1); //[]  REG_Z0_Z0POW_B
	_rtd_part_outl(0x1800da08, 4, 4, 0x1); //[]  REG_Z0_Z0POW_CK
	_rtd_part_outl(0x1800da08, 8, 8, 0x1); //[]  REG_Z0_Z0POW_G
	_rtd_part_outl(0x1800da08, 12, 12, 0x1); //[]  REG_Z0_Z0POW_R
	_rtd_part_outl(0x1800da08, 24, 24, 0x0); //[]  REG_Z0_Z300_SEL
	_rtd_part_outl(0x1800da10, 0, 0, 0x0); //[]  Z0_P_OFF
	_rtd_part_outl(0x1800da10, 1, 1, 0x0); //[]  Z0_N_OFF
	_rtd_part_outl(0x1800da10, 3, 2, 0x3); //[]  Z0_RFIX_SEL
	_rtd_part_outl(0x1800da10, 7, 4, 0x0); //[]  dummy.
	_rtd_part_outl(0x1800da10, 8, 8, 0x0); //[]  fast sw sel
	_rtd_part_outl(0x1800da10, 9, 9, 0x0); //[]  fast sw clock div enable
	_rtd_part_outl(0x1800da10, 12, 10, 0x0); //[]  fast sw clock div select (delay time=
	_rtd_part_outl(0x1800da10, 13, 13, 0x0); //[]  Enable AVDD10 voltage probe.
	_rtd_part_outl(0x1800da10, 15, 14, 0x0); //[]  IO overvoltage detect bit select
	_rtd_part_outl(0x1800da10, 16, 16, 0x1); //[]  CK_TX EN
	_rtd_part_outl(0x1800da10, 17, 17, 0x0); //[]  CK_MD selection.
	_rtd_part_outl(0x1800da10, 19, 18, 0x3); //[]  1.8V CK-detect hysteresis window select.
	_rtd_part_outl(0x1800da10, 20, 20, 0x0); //[]  CK-PAD to CK-detect switch enable.
	_rtd_part_outl(0x1800da10, 21, 21, 0x0); //[]  Standby mode CK-detect & AC-couple
	_rtd_part_outl(0x1800da10, 22, 22, 0x0); //[]  dummy
	_rtd_part_outl(0x1800da10, 23, 23, 0x0); //[]  div32 rstb (CK-detect use).
	_rtd_part_outl(0x1800da10, 27, 24, 0x0); //[]  test voltage/test current/test clock select  @[6:5]
	_rtd_part_outl(0x1800da10, 28, 28, 0x0); //[]  dummy
	_rtd_part_outl(0x1800da10, 30, 29, 0x0); //[]  tst pad mode sel
	_rtd_part_outl(0x1800da10, 31, 31, 0x0); //[]  dummy
	_rtd_part_outl(0x1800da14, 7, 0, 0x10); //[]  REG_TOP_IN_5 dummy
	_rtd_part_outl(0x1800da18, 0, 0, 0x1); //[APHY fix]  REG_HDMIRX_BIAS_EN bandgap reference power
	_rtd_part_outl(0x1800da18, 10, 8, 0x0); //[APHY fix]  REG_BG_RBGLOOP2 dummy
	_rtd_part_outl(0x1800da18, 13, 11, 0x0); //[APHY fix]  REG_BG_RBG dummy
	_rtd_part_outl(0x1800da18, 15, 14, 0x0); //[APHY fix]  REG_BG_RBG2 dummy
	_rtd_part_outl(0x1800da18, 16, 16, 0x0); //[APHY fix]  REG_BG_POW dummy
	_rtd_part_outl(0x1800da18, 17, 17, 0x0); //[APHY fix]  REG_BG_ENVBGUP
	_rtd_part_outl(0x1800da20, 7, 0, 0x00); //[]  dummy
	_rtd_part_outl(0x1800da20, 15, 8, 0x00); //[]  dummy
	_rtd_part_outl(0x1800da20, 23, 16, 0x00); //[]  dummy
	_rtd_part_outl(0x1800da20, 31, 24, 0x00); //[]  dummy
	_rtd_part_outl(0x1800da24, 7, 0, 0x00); //[]  dummy
	_rtd_part_outl(0x1800da24, 15, 8, 0x00); //[]  dummy
	_rtd_part_outl(0x1800da24, 23, 16, 0x00); //[]  dummy
	_rtd_part_outl(0x1800da24, 31, 24, 0x00); //[]  dummy
	_rtd_part_outl(0x1800da30, 1, 1, 0x0); //[APHY fix]  CK-Lane INPOFF_SINGLE_EN
	_rtd_part_outl(0x1800da30, 2, 2, 0x0); //[APHY fix]  CK-Lane INNOFF_SINGLE_EN
	_rtd_part_outl(0x1800da30, 3, 3, 0x1); //[APHY fix]  CK-Lane POW_AC_COUPLE
	_rtd_part_outl(0x1800da30, 5, 4, 0x1); //[APHY fix]  CK-Lane RXVCM_SEL[1:0]
	_rtd_part_outl(0x1800da34, 21, 21, 0x1); //[APHY fix]   	CK-Lane RS_CAL_EN
	_rtd_part_outl(0x1800da34, 25, 25, 0x1); //[APHY fix]   	CK-Lane POW_DATALANE_BIAS
	_rtd_part_outl(0x1800da34, 26, 26, 0x0); //[APHY fix]   	CK-Lane REG_FORCE_STARTUP(Const-GM)
	_rtd_part_outl(0x1800da34, 27, 27, 0x0); //[APHY fix]   	CK-Lane REG_POWB_STARTUP
	_rtd_part_outl(0x1800da38, 8, 8, 0x1); //[APHY fix]  		CK-Lane POW_DFE
	_rtd_part_outl(0x1800da38, 14, 12, 0x6); //[APHY fix] 		CK-Lane DFE_SUMAMP_ ISEL
	_rtd_part_outl(0x1800da38, 15, 15, 0x0); //[APHY fix] 		CK-Lane DFE_SUMAMP_DCGAIN_MAX
	_rtd_part_outl(0x1800da38, 19, 18, 0x0); //[APHY fix] 		CK-Lane Dummy
	_rtd_part_outl(0x1800da38, 20, 20, 0x0); //[APHY fix] 		CK-Lane DFE CKI_DELAY_EN
	_rtd_part_outl(0x1800da38, 21, 21, 0x0); //[APHY fix] 		CK-Lane DFE CKIB_DELAY_EN
	_rtd_part_outl(0x1800da38, 22, 22, 0x0); //[APHY fix] 		CK-Lane DFE CKQ_DELAY_EN
	_rtd_part_outl(0x1800da38, 23, 23, 0x0); //[APHY fix] 		CK-Lane DFE CKQB_DELAY_EN
	_rtd_part_outl(0x1800da38, 24, 24, 0x0); //[APHY fix] 		CK-Lane EN_EYE_MNT
	_rtd_part_outl(0x1800da38, 25, 25, 0x0); //[APHY fix] 		CK-Lane DEMUX input clock phase select:(for eye mnt)
	_rtd_part_outl(0x1800da38, 26, 26, 0x0); //[APHY fix] 		CK-Lane VTH_MANUAL
	_rtd_part_outl(0x1800da38, 27, 27, 0x0); //[APHY fix] 		CK-Lane DA_EG_VOS_PULLLOW(koffset disable)
	_rtd_part_outl(0x1800da38, 31, 29, 0x0); //[APHY fix] 			CK-Lane dummy
	_rtd_part_outl(0x1800da3c, 2, 0, 0x0); //[APHY fix]  	CK-Lane DFE_TAP_DELAY
	_rtd_part_outl(0x1800da3c, 7, 7, 0x1); //[APHY fix]   		CK-Lane DFE_ADAPT_EN
	_rtd_part_outl(0x1800da3c, 8, 8, 0x0); //[APHY fix]   	CK-Lane rstb of dtata-lane test div8
	_rtd_part_outl(0x1800da3c, 9, 9, 0x0); //[APHY fix]   	CK-Lane TRANSITION CNT enable.
	_rtd_part_outl(0x1800da3c, 16, 16, 0x0); //[APHY fix]  		CK-Lane QCG1_EN
	_rtd_part_outl(0x1800da3c, 17, 17, 0x0); //[APHY fix]  		CK-Lane QCG1 injection-locked EN
	_rtd_part_outl(0x1800da3c, 18, 18, 0x1); //[APHY fix]  		CK-Lane QCG2_EN
	_rtd_part_outl(0x1800da3c, 19, 19, 0x0); //[APHY fix]  		CK-Lane QCG2 injection-locked EN
	_rtd_part_outl(0x1800da3c, 20, 20, 0x1); //[APHY fix]  		CK-Lane QCG FLD divider(/8) rstb
	_rtd_part_outl(0x1800da3c, 21, 21, 0x1); //[APHY fix]  		CK-Lane QCG FLD SEL
	_rtd_part_outl(0x1800da3c, 22, 22, 0x0); //[APHY fix]  		CK-Lane CK_FLD SEL.
	_rtd_part_outl(0x1800da3c, 23, 23, 0x0); //[APHY fix]  		CK-Lane CK-lane clk to B/G/R EN.
	_rtd_part_outl(0x1800da40, 8, 8, 0x0); //[APHY fix]  CK-Lane PI_EN
	_rtd_part_outl(0x1800da40, 12, 12, 0x0); //[APHY fix]  CK-Lane QCG1 Cload contrl SEL
	_rtd_part_outl(0x1800da40, 13, 13, 0x0); //[APHY fix]  CK-Lane QCG2 Cload contrl SEL
	_rtd_part_outl(0x1800da40, 15, 15, 0x0); //[APHY fix]  CK-Lane BIAS_PI_CUR_SEL
	_rtd_part_outl(0x1800da40, 21, 21, 0x0); //[APHY fix]  CK-Lane PI_EYE_EN
	_rtd_part_outl(0x1800da40, 23, 23, 0x0); //[APHY fix]  CK-Lane KI SEL
	_rtd_part_outl(0x1800da40, 25, 25, 0x0); //[APHY fix]  CK-Lane FKP_RSTB_SEL
	_rtd_part_outl(0x1800da40, 27, 26, 0x0); //[APHY fix]  CK-Lane KI_CK_SEL
	_rtd_part_outl(0x1800da40, 29, 29, 0x0); //[APHY fix]  CK-Lane ST_CODE SEL
	_rtd_part_outl(0x1800da40, 30, 30, 0x0); //[APHY fix]  CK-Lane QCG Ckin SEL.
	_rtd_part_outl(0x1800da44, 3, 2, 0x1); //[APHY fix]  CK-Lane ACDR_SEL_DIV_FLD[1:0]
	_rtd_part_outl(0x1800da44, 5, 4, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_DIV_REF[1:0]
	_rtd_part_outl(0x1800da44, 7, 6, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_DIV_PLL[1:0]
	_rtd_part_outl(0x1800da44, 9, 8, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_DIV_TRAINING[1:0]
	_rtd_part_outl(0x1800da44, 12, 12, 0x0); //[APHY fix]  CK-Lane dummy
	_rtd_part_outl(0x1800da44, 13, 13, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_FLD_0CKFB_1CKREF
	_rtd_part_outl(0x1800da44, 15, 14, 0x0); //[APHY fix]  CK-Lane no use
	_rtd_part_outl(0x1800da44, 18, 18, 0x1); //[APHY fix]  CK-Lane ACDR_RSTB_DIV_PLL ... removed
	_rtd_part_outl(0x1800da44, 24, 24, 0x0); //[APHY fix]  CK-Lane ACDR_EN_UPDN_PULSE_FILTER
	_rtd_part_outl(0x1800da44, 25, 25, 0x1); //[APHY fix]  CK-Lane ACDR_RSTB_UPDN ... dummy
	_rtd_part_outl(0x1800da44, 27, 26, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_UPDN_WIDTH[1:0]
	_rtd_part_outl(0x1800da44, 28, 28, 0x1); //[APHY fix]  CK-Lane ACDR_POW_LPF_IDAC
	_rtd_part_outl(0x1800da44, 31, 29, 0x3); //[APHY fix]  CK-Lane ACDR_SEL_LPF_IDAC[2:0]
	_rtd_part_outl(0x1800da48, 0, 0, 0x1); //[APHY fix]  CK-Lane ACDR_POW_CP
	_rtd_part_outl(0x1800da48, 2, 2, 0x1); //[APHY fix]  CK-Lane ACDR_POW_IDN_BBPD
	_rtd_part_outl(0x1800da48, 3, 3, 0x0); //[APHY fix]  CK-Lane no use
	_rtd_part_outl(0x1800da48, 6, 4, 0x7); //[APHY fix]  CK-Lane ACDR_SEL_TIE_IDN_BBPD
	_rtd_part_outl(0x1800da48, 7, 7, 0x0); //[APHY fix]  CK-Lane ACDR_POW_IBIAS_IDN_HV
	_rtd_part_outl(0x1800da48, 8, 8, 0x1); //[APHY fix]  CK-Lane ACDR_POW_VCO
	_rtd_part_outl(0x1800da48, 9, 9, 0x1); //[APHY fix]  CK-Lane ACDR_POW_VCO_VDAC
	_rtd_part_outl(0x1800da48, 11, 10, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_V15_VDAC
	_rtd_part_outl(0x1800da48, 14, 14, 0x0); //[APHY fix]  CK-Lane no use
	_rtd_part_outl(0x1800da48, 15, 15, 0x0); //[APHY fix]  CK-Lane no use
	_rtd_part_outl(0x1800da48, 16, 16, 0x0); //[APHY fix]  CK-Lane ACDR_POW_TEST_MODE
	_rtd_part_outl(0x1800da48, 18, 17, 0x0); //[APHY fix]  CK-Lane ACDR_SEL_TEST_MODE
	_rtd_part_outl(0x1800da48, 23, 19, 0x00); //[APHY fix]  CK-Lane no use
	_rtd_part_outl(0x1800da48, 31, 31, 0x0); //[APHY fix]  CK-Lane no use
	_rtd_part_outl(0x1800da4c, 15, 13, 0x0); //[APHY fix]  CK-Lane no use
	_rtd_part_outl(0x1800da4c, 19, 16, 0x0); //[APHY fix]  CK-Lane LE1_ISEL_IN_2 -> dummy
	_rtd_part_outl(0x1800da4c, 31, 25, 0x44); //[APHY fix]  CK-Lane no use for LEQ
	_rtd_part_outl(0x1800da50, 1, 1, 0x0); //[APHY fix]  		B-Lane INPOFF_SINGLE_EN
	_rtd_part_outl(0x1800da50, 2, 2, 0x0); //[APHY fix]  		B-Lane INNOFF_SINGLE_EN
	_rtd_part_outl(0x1800da50, 3, 3, 0x1); //[APHY fix]  		B-Lane POW_AC_COUPLE
	_rtd_part_outl(0x1800da50, 5, 4, 0x1); //[APHY fix]  		B-Lane RXVCM_SEL[1:0]
	_rtd_part_outl(0x1800da54, 21, 21, 0x1); //[APHY fix]   		B-Lane RS_CAL_EN
	_rtd_part_outl(0x1800da54, 25, 25, 0x1); //[APHY fix]   		B-Lane POW_DATALANE_BIAS
	_rtd_part_outl(0x1800da54, 26, 26, 0x0); //[APHY fix]   		B-Lane REG_FORCE_STARTUP(Const-GM)
	_rtd_part_outl(0x1800da54, 27, 27, 0x0); //[APHY fix]   		B-Lane REG_POWB_STARTUP
	_rtd_part_outl(0x1800da58, 8, 8, 0x1); //[APHY fix]  		B-Lane POW_DFE
	_rtd_part_outl(0x1800da58, 14, 12, 0x6); //[APHY fix] 		B-Lane DFE_SUMAMP_ ISEL
	_rtd_part_outl(0x1800da58, 15, 15, 0x0); //[APHY fix] 		B-Lane DFE_SUMAMP_DCGAIN_MAX
	_rtd_part_outl(0x1800da58, 19, 18, 0x0); //[APHY fix] 		B-Lane Dummy
	_rtd_part_outl(0x1800da58, 20, 20, 0x0); //[APHY fix] 		B-Lane DFE CKI_DELAY_EN
	_rtd_part_outl(0x1800da58, 21, 21, 0x0); //[APHY fix] 		B-Lane DFE CKIB_DELAY_EN
	_rtd_part_outl(0x1800da58, 22, 22, 0x0); //[APHY fix] 		B-Lane DFE CKQ_DELAY_EN
	_rtd_part_outl(0x1800da58, 23, 23, 0x0); //[APHY fix] 		B-Lane DFE CKQB_DELAY_EN
	_rtd_part_outl(0x1800da58, 24, 24, 0x0); //[APHY fix] 		B-Lane EN_EYE_MNT
	_rtd_part_outl(0x1800da58, 25, 25, 0x0); //[APHY fix] 		B-Lane DEMUX input clock phase select:(for eye mnt)
	_rtd_part_outl(0x1800da58, 26, 26, 0x0); //[APHY fix] 		B-Lane VTH_MANUAL
	_rtd_part_outl(0x1800da58, 27, 27, 0x0); //[APHY fix] 		B-Lane DA_EG_VOS_PULLLOW(koffset disable)
	_rtd_part_outl(0x1800da58, 31, 29, 0x0); //[APHY fix] 			B-Lane dummy
	_rtd_part_outl(0x1800da5c, 2, 0, 0x0); //[APHY fix]  	B-Lane DFE_TAP_DELAY
	_rtd_part_outl(0x1800da5c, 7, 7, 0x1); //[APHY fix]   		B-Lane DFE_ADAPT_EN
	_rtd_part_outl(0x1800da5c, 8, 8, 0x0); //[APHY fix]   	B-Lane rstb of dtata-lane test div8
	_rtd_part_outl(0x1800da5c, 9, 9, 0x0); //[APHY fix]   	B-Lane TRANSITION CNT enable.
	_rtd_part_outl(0x1800da5c, 16, 16, 0x0); //[APHY fix]  		B-Lane QCG1_EN
	_rtd_part_outl(0x1800da5c, 17, 17, 0x0); //[APHY fix]  		B-Lane QCG1 injection-locked EN
	_rtd_part_outl(0x1800da5c, 18, 18, 0x1); //[APHY fix]  		B-Lane QCG2_EN
	_rtd_part_outl(0x1800da5c, 19, 19, 0x0); //[APHY fix]  		B-Lane QCG2 injection-locked EN
	_rtd_part_outl(0x1800da5c, 20, 20, 0x1); //[APHY fix]  		B-Lane QCG FLD divider(/8) rstb
	_rtd_part_outl(0x1800da5c, 21, 21, 0x1); //[APHY fix]  		B-Lane QCG FLD SEL
	_rtd_part_outl(0x1800da5c, 22, 22, 0x0); //[APHY fix]  		B-Lane CK_FLD SEL.
	_rtd_part_outl(0x1800da5c, 23, 23, 0x1); //[APHY fix]  		B-Lane CK-lane clk to B/G/R EN.
	_rtd_part_outl(0x1800da60, 8, 8, 0x0); //[APHY fix]  B-Lane PI_EN
	_rtd_part_outl(0x1800da60, 12, 12, 0x0); //[APHY fix]  B-Lane QCG1 Cload contrl SEL
	_rtd_part_outl(0x1800da60, 13, 13, 0x0); //[APHY fix]  B-Lane QCG2 Cload contrl SEL
	_rtd_part_outl(0x1800da60, 15, 15, 0x0); //[APHY fix]  B-Lane BIAS_PI_CUR_SEL
	_rtd_part_outl(0x1800da60, 21, 21, 0x0); //[APHY fix]  B-Lane PI_EYE_EN
	_rtd_part_outl(0x1800da60, 23, 23, 0x0); //[APHY fix]  B-Lane KI SEL
	_rtd_part_outl(0x1800da60, 25, 25, 0x0); //[APHY fix]  B-Lane FKP_RSTB_SEL
	_rtd_part_outl(0x1800da60, 27, 26, 0x0); //[APHY fix]  B-Lane KI_CK_SEL
	_rtd_part_outl(0x1800da60, 29, 29, 0x0); //[APHY fix]  B-Lane ST_CODE SEL
	_rtd_part_outl(0x1800da60, 30, 30, 0x0); //[APHY fix]  B-Lane QCG Ckin SEL.
	_rtd_part_outl(0x1800da64, 3, 2, 0x1); //[APHY fix]  B-Lane ACDR_SEL_DIV_FLD[1:0]
	_rtd_part_outl(0x1800da64, 5, 4, 0x0); //[APHY fix]  B-Lane ACDR_SEL_DIV_REF[1:0]
	_rtd_part_outl(0x1800da64, 7, 6, 0x0); //[APHY fix]  B-Lane ACDR_SEL_DIV_PLL[1:0]
	_rtd_part_outl(0x1800da64, 9, 8, 0x0); //[APHY fix]  B-Lane ACDR_SEL_DIV_TRAINING[1:0]
	_rtd_part_outl(0x1800da64, 12, 12, 0x0); //[APHY fix]  B-Lane dummy
	_rtd_part_outl(0x1800da64, 13, 13, 0x0); //[APHY fix]  B-Lane ACDR_SEL_FLD_0CKFB_1CKREF
	_rtd_part_outl(0x1800da64, 15, 14, 0x0); //[APHY fix]  B-Lane no use
	_rtd_part_outl(0x1800da64, 18, 18, 0x1); //[APHY fix]  B-Lane ACDR_RSTB_DIV_PLL ... removed
	_rtd_part_outl(0x1800da64, 24, 24, 0x0); //[APHY fix]  B-Lane ACDR_EN_UPDN_PULSE_FILTER
	_rtd_part_outl(0x1800da64, 25, 25, 0x1); //[APHY fix]  B-Lane ACDR_RSTB_UPDN ... dummy
	_rtd_part_outl(0x1800da64, 27, 26, 0x0); //[APHY fix]  B-Lane ACDR_SEL_UPDN_WIDTH[1:0]
	_rtd_part_outl(0x1800da64, 28, 28, 0x1); //[APHY fix]  B-Lane ACDR_POW_LPF_IDAC
	_rtd_part_outl(0x1800da64, 31, 29, 0x3); //[APHY fix]  B-Lane ACDR_SEL_LPF_IDAC[2:0]
	_rtd_part_outl(0x1800da68, 0, 0, 0x1); //[APHY fix]  B-Lane ACDR_POW_CP
	_rtd_part_outl(0x1800da68, 2, 2, 0x1); //[APHY fix]  B-Lane ACDR_POW_IDN_BBPD
	_rtd_part_outl(0x1800da68, 3, 3, 0x0); //[APHY fix]  B-Lane no use
	_rtd_part_outl(0x1800da68, 6, 4, 0x7); //[APHY fix]  B-Lane ACDR_SEL_TIE_IDN_BBPD
	_rtd_part_outl(0x1800da68, 7, 7, 0x0); //[APHY fix]  B-Lane ACDR_POW_IBIAS_IDN_HV
	_rtd_part_outl(0x1800da68, 8, 8, 0x1); //[APHY fix]  B-Lane ACDR_POW_VCO
	_rtd_part_outl(0x1800da68, 9, 9, 0x1); //[APHY fix]  B-Lane ACDR_POW_VCO_VDAC
	_rtd_part_outl(0x1800da68, 11, 10, 0x0); //[APHY fix]  B-Lane ACDR_SEL_V15_VDAC
	_rtd_part_outl(0x1800da68, 14, 14, 0x0); //[APHY fix]  B-Lane no use
	_rtd_part_outl(0x1800da68, 15, 15, 0x0); //[APHY fix]  B-Lane no use
	_rtd_part_outl(0x1800da68, 16, 16, 0x0); //[APHY fix]  B-Lane ACDR_POW_TEST_MODE
	_rtd_part_outl(0x1800da68, 18, 17, 0x0); //[APHY fix]  B-Lane ACDR_SEL_TEST_MODE
	_rtd_part_outl(0x1800da68, 23, 19, 0x00); //[APHY fix]  B-Lane no use
	_rtd_part_outl(0x1800da68, 31, 31, 0x0); //[APHY fix]  B-Lane no use
	_rtd_part_outl(0x1800da6c, 15, 13, 0x1); //[APHY fix]  B-Lane no use
	_rtd_part_outl(0x1800da6c, 19, 16, 0x0); //[APHY fix]  B-Lane LE1_ISEL_IN_2 -> dummy
	_rtd_part_outl(0x1800da6c, 31, 25, 0x44); //[APHY fix]  B-Lane no use for LEQ
	_rtd_part_outl(0x1800da70, 1, 1, 0x0); //[APHY fix]  		G-Lane INPOFF_SINGLE_EN
	_rtd_part_outl(0x1800da70, 2, 2, 0x0); //[APHY fix]  		G-Lane INNOFF_SINGLE_EN
	_rtd_part_outl(0x1800da70, 3, 3, 0x1); //[APHY fix]  		G-Lane POW_AC_COUPLE
	_rtd_part_outl(0x1800da70, 5, 4, 0x1); //[APHY fix]  		G-Lane RXVCM_SEL[1:0]
	_rtd_part_outl(0x1800da74, 21, 21, 0x1); //[APHY fix]   		G-Lane RS_CAL_EN
	_rtd_part_outl(0x1800da74, 25, 25, 0x1); //[APHY fix]   		G-Lane POW_DATALANE_BIAS
	_rtd_part_outl(0x1800da74, 26, 26, 0x0); //[APHY fix]   		G-Lane REG_FORCE_STARTUP(Const-GM)
	_rtd_part_outl(0x1800da74, 27, 27, 0x0); //[APHY fix]   		G-Lane REG_POWB_STARTUP
	_rtd_part_outl(0x1800da78, 8, 8, 0x1); //[APHY fix]  		G-Lane POW_DFE
	_rtd_part_outl(0x1800da78, 14, 12, 0x6); //[APHY fix] 		G-Lane DFE_SUMAMP_ ISEL
	_rtd_part_outl(0x1800da78, 15, 15, 0x0); //[APHY fix] 		G-Lane DFE_SUMAMP_DCGAIN_MAX
	_rtd_part_outl(0x1800da78, 19, 18, 0x0); //[APHY fix] 		G-Lane Dummy
	_rtd_part_outl(0x1800da78, 20, 20, 0x0); //[APHY fix] 		G-Lane DFE CKI_DELAY_EN
	_rtd_part_outl(0x1800da78, 21, 21, 0x0); //[APHY fix] 		G-Lane DFE CKIB_DELAY_EN
	_rtd_part_outl(0x1800da78, 22, 22, 0x0); //[APHY fix] 		G-Lane DFE CKQ_DELAY_EN
	_rtd_part_outl(0x1800da78, 23, 23, 0x0); //[APHY fix] 		G-Lane DFE CKQB_DELAY_EN
	_rtd_part_outl(0x1800da78, 24, 24, 0x0); //[APHY fix] 		G-Lane EN_EYE_MNT
	_rtd_part_outl(0x1800da78, 25, 25, 0x0); //[APHY fix] 		G-Lane DEMUX input clock phase select:(for eye mnt)
	_rtd_part_outl(0x1800da78, 26, 26, 0x0); //[APHY fix] 		G-Lane VTH_MANUAL
	_rtd_part_outl(0x1800da78, 27, 27, 0x0); //[APHY fix] 		G-Lane DA_EG_VOS_PULLLOW(koffset disable)
	_rtd_part_outl(0x1800da78, 31, 29, 0x0); //[APHY fix] 			G-Lane dummy
	_rtd_part_outl(0x1800da7c, 2, 0, 0x0); //[APHY fix]  	G-Lane DFE_TAP_DELAY
	_rtd_part_outl(0x1800da7c, 7, 7, 0x1); //[APHY fix]   		G-Lane DFE_ADAPT_EN
	_rtd_part_outl(0x1800da7c, 8, 8, 0x0); //[APHY fix]   	G-Lane rstb of dtata-lane test div8
	_rtd_part_outl(0x1800da7c, 9, 9, 0x0); //[APHY fix]   	G-Lane TRANSITION CNT enable.
	_rtd_part_outl(0x1800da7c, 16, 16, 0x0); //[APHY fix]  		G-Lane QCG1_EN
	_rtd_part_outl(0x1800da7c, 17, 17, 0x0); //[APHY fix]  		G-Lane QCG1 injection-locked EN
	_rtd_part_outl(0x1800da7c, 18, 18, 0x1); //[APHY fix]  		G-Lane QCG2_EN
	_rtd_part_outl(0x1800da7c, 19, 19, 0x0); //[APHY fix]  		G-Lane QCG2 injection-locked EN
	_rtd_part_outl(0x1800da7c, 20, 20, 0x1); //[APHY fix]  		G-Lane QCG FLD divider(/8) rstb
	_rtd_part_outl(0x1800da7c, 21, 21, 0x1); //[APHY fix]  		G-Lane QCG FLD SEL
	_rtd_part_outl(0x1800da7c, 22, 22, 0x0); //[APHY fix]  		G-Lane CK_FLD SEL.
	_rtd_part_outl(0x1800da7c, 23, 23, 0x1); //[APHY fix]  		G-Lane CK-lane clk to B/G/R EN.
	_rtd_part_outl(0x1800da80, 8, 8, 0x0); //[APHY fix]  G-Lane PI_EN
	_rtd_part_outl(0x1800da80, 12, 12, 0x0); //[APHY fix]  G-Lane QCG1 Cload contrl SEL
	_rtd_part_outl(0x1800da80, 13, 13, 0x0); //[APHY fix]  G-Lane QCG2 Cload contrl SEL
	_rtd_part_outl(0x1800da80, 15, 15, 0x0); //[APHY fix]  G-Lane BIAS_PI_CUR_SEL
	_rtd_part_outl(0x1800da80, 21, 21, 0x0); //[APHY fix]  G-Lane PI_EYE_EN
	_rtd_part_outl(0x1800da80, 23, 23, 0x0); //[APHY fix]  G-Lane KI SEL
	_rtd_part_outl(0x1800da80, 25, 25, 0x0); //[APHY fix]  G-Lane FKP_RSTB_SEL
	_rtd_part_outl(0x1800da80, 27, 26, 0x0); //[APHY fix]  G-Lane KI_CK_SEL
	_rtd_part_outl(0x1800da80, 29, 29, 0x0); //[APHY fix]  G-Lane ST_CODE SEL
	_rtd_part_outl(0x1800da80, 30, 30, 0x0); //[APHY fix]  G-Lane QCG Ckin SEL.
	_rtd_part_outl(0x1800da84, 3, 2, 0x1); //[APHY fix]  G-Lane ACDR_SEL_DIV_FLD[1:0]
	_rtd_part_outl(0x1800da84, 5, 4, 0x0); //[APHY fix]  G-Lane ACDR_SEL_DIV_REF[1:0]
	_rtd_part_outl(0x1800da84, 7, 6, 0x0); //[APHY fix]  G-Lane ACDR_SEL_DIV_PLL[1:0]
	_rtd_part_outl(0x1800da84, 9, 8, 0x0); //[APHY fix]  G-Lane ACDR_SEL_DIV_TRAINING[1:0]
	_rtd_part_outl(0x1800da84, 12, 12, 0x0); //[APHY fix]  G-Lane dummy
	_rtd_part_outl(0x1800da84, 13, 13, 0x0); //[APHY fix]  G-Lane ACDR_SEL_FLD_0CKFB_1CKREF
	_rtd_part_outl(0x1800da84, 15, 14, 0x0); //[APHY fix]  G-Lane no use
	_rtd_part_outl(0x1800da84, 18, 18, 0x1); //[APHY fix]  G-Lane ACDR_RSTB_DIV_PLL ... removed
	_rtd_part_outl(0x1800da84, 24, 24, 0x0); //[APHY fix]  G-Lane ACDR_EN_UPDN_PULSE_FILTER
	_rtd_part_outl(0x1800da84, 25, 25, 0x1); //[APHY fix]  G-Lane ACDR_RSTB_UPDN ... dummy
	_rtd_part_outl(0x1800da84, 27, 26, 0x0); //[APHY fix]  G-Lane ACDR_SEL_UPDN_WIDTH[1:0]
	_rtd_part_outl(0x1800da84, 28, 28, 0x1); //[APHY fix]  G-Lane ACDR_POW_LPF_IDAC
	_rtd_part_outl(0x1800da84, 31, 29, 0x3); //[APHY fix]  G-Lane ACDR_SEL_LPF_IDAC[2:0]
	_rtd_part_outl(0x1800da88, 0, 0, 0x1); //[APHY fix]  G-Lane ACDR_POW_CP
	_rtd_part_outl(0x1800da88, 2, 2, 0x1); //[APHY fix]  G-Lane ACDR_POW_IDN_BBPD
	_rtd_part_outl(0x1800da88, 3, 3, 0x0); //[APHY fix]  G-Lane no use
	_rtd_part_outl(0x1800da88, 6, 4, 0x7); //[APHY fix]  G-Lane ACDR_SEL_TIE_IDN_BBPD
	_rtd_part_outl(0x1800da88, 7, 7, 0x0); //[APHY fix]  G-Lane ACDR_POW_IBIAS_IDN_HV
	_rtd_part_outl(0x1800da88, 8, 8, 0x1); //[APHY fix]  G-Lane ACDR_POW_VCO
	_rtd_part_outl(0x1800da88, 9, 9, 0x1); //[APHY fix]  G-Lane ACDR_POW_VCO_VDAC
	_rtd_part_outl(0x1800da88, 11, 10, 0x0); //[APHY fix]  G-Lane ACDR_SEL_V15_VDAC
	_rtd_part_outl(0x1800da88, 14, 14, 0x0); //[APHY fix]  G-Lane no use
	_rtd_part_outl(0x1800da88, 15, 15, 0x0); //[APHY fix]  G-Lane no use
	_rtd_part_outl(0x1800da88, 16, 16, 0x0); //[APHY fix]  G-Lane ACDR_POW_TEST_MODE
	_rtd_part_outl(0x1800da88, 18, 17, 0x0); //[APHY fix]  G-Lane ACDR_SEL_TEST_MODE
	_rtd_part_outl(0x1800da88, 23, 19, 0x00); //[APHY fix]  G-Lane no use
	_rtd_part_outl(0x1800da88, 31, 31, 0x0); //[APHY fix]  G-Lane no use
	_rtd_part_outl(0x1800da8c, 15, 13, 0x1); //[APHY fix]  G-Lane no use
	_rtd_part_outl(0x1800da8c, 19, 16, 0x0); //[APHY fix]  G-Lane LE1_ISEL_IN_2 -> dummy
	_rtd_part_outl(0x1800da8c, 31, 25, 0x44); //[APHY fix]  G-Lane no use for LEQ
	_rtd_part_outl(0x1800da90, 1, 1, 0x0); //[APHY fix]  		R-Lane INPOFF_SINGLE_EN
	_rtd_part_outl(0x1800da90, 2, 2, 0x0); //[APHY fix]  		R-Lane INNOFF_SINGLE_EN
	_rtd_part_outl(0x1800da90, 3, 3, 0x1); //[APHY fix]  		R-Lane POW_AC_COUPLE
	_rtd_part_outl(0x1800da90, 5, 4, 0x1); //[APHY fix]  		R-Lane RXVCM_SEL[1:0]
	_rtd_part_outl(0x1800da94, 21, 21, 0x1); //[APHY fix]   		R-Lane RS_CAL_EN
	_rtd_part_outl(0x1800da94, 25, 25, 0x1); //[APHY fix]   		R-Lane POW_DATALANE_BIAS
	_rtd_part_outl(0x1800da94, 26, 26, 0x0); //[APHY fix]   		R-Lane REG_FORCE_STARTUP(Const-GM)
	_rtd_part_outl(0x1800da94, 27, 27, 0x0); //[APHY fix]   		R-Lane REG_POWB_STARTUP
	_rtd_part_outl(0x1800da98, 8, 8, 0x1); //[APHY fix]  		R-Lane POW_DFE
	_rtd_part_outl(0x1800da98, 14, 12, 0x6); //[APHY fix] 		R-Lane DFE_SUMAMP_ ISEL
	_rtd_part_outl(0x1800da98, 15, 15, 0x0); //[APHY fix] 		R-Lane DFE_SUMAMP_DCGAIN_MAX
	_rtd_part_outl(0x1800da98, 19, 18, 0x0); //[APHY fix] 		R-Lane Dummy
	_rtd_part_outl(0x1800da98, 20, 20, 0x0); //[APHY fix] 		R-Lane DFE CKI_DELAY_EN
	_rtd_part_outl(0x1800da98, 21, 21, 0x0); //[APHY fix] 		R-Lane DFE CKIB_DELAY_EN
	_rtd_part_outl(0x1800da98, 22, 22, 0x0); //[APHY fix] 		R-Lane DFE CKQ_DELAY_EN
	_rtd_part_outl(0x1800da98, 23, 23, 0x0); //[APHY fix] 		R-Lane DFE CKQB_DELAY_EN
	_rtd_part_outl(0x1800da98, 24, 24, 0x0); //[APHY fix] 		R-Lane EN_EYE_MNT
	_rtd_part_outl(0x1800da98, 25, 25, 0x0); //[APHY fix] 		R-Lane DEMUX input clock phase select:(for eye mnt)
	_rtd_part_outl(0x1800da98, 26, 26, 0x0); //[APHY fix] 		R-Lane VTH_MANUAL
	_rtd_part_outl(0x1800da98, 27, 27, 0x0); //[APHY fix] 		R-Lane DA_EG_VOS_PULLLOW(koffset disable)
	_rtd_part_outl(0x1800da98, 31, 29, 0x0); //[APHY fix] 			R-Lane dummy
	_rtd_part_outl(0x1800da9c, 2, 0, 0x0); //[APHY fix]  	R-Lane DFE_TAP_DELAY
	_rtd_part_outl(0x1800da9c, 7, 7, 0x1); //[APHY fix]   		R-Lane DFE_ADAPT_EN
	_rtd_part_outl(0x1800da9c, 8, 8, 0x0); //[APHY fix]   	R-Lane rstb of dtata-lane test div8
	_rtd_part_outl(0x1800da9c, 9, 9, 0x0); //[APHY fix]   	R-Lane TRANSITION CNT enable.
	_rtd_part_outl(0x1800da9c, 16, 16, 0x0); //[APHY fix]  		R-Lane QCG1_EN
	_rtd_part_outl(0x1800da9c, 17, 17, 0x0); //[APHY fix]  		R-Lane QCG1 injection-locked EN
	_rtd_part_outl(0x1800da9c, 18, 18, 0x1); //[APHY fix]  		R-Lane QCG2_EN
	_rtd_part_outl(0x1800da9c, 19, 19, 0x0); //[APHY fix]  		R-Lane QCG2 injection-locked EN
	_rtd_part_outl(0x1800da9c, 20, 20, 0x1); //[APHY fix]  		R-Lane QCG FLD divider(/8) rstb
	_rtd_part_outl(0x1800da9c, 21, 21, 0x1); //[APHY fix]  		R-Lane QCG FLD SEL
	_rtd_part_outl(0x1800da9c, 22, 22, 0x0); //[APHY fix]  		R-Lane CK_FLD SEL.
	_rtd_part_outl(0x1800da9c, 23, 23, 0x1); //[APHY fix]  		R-Lane CK-lane clk to B/G/R EN.
	_rtd_part_outl(0x1800daa0, 8, 8, 0x0); //[APHY fix]  R-Lane PI_EN
	_rtd_part_outl(0x1800daa0, 12, 12, 0x0); //[APHY fix]  R-Lane QCG1 Cload contrl SEL
	_rtd_part_outl(0x1800daa0, 13, 13, 0x0); //[APHY fix]  R-Lane QCG2 Cload contrl SEL
	_rtd_part_outl(0x1800daa0, 15, 15, 0x0); //[APHY fix]  R-Lane BIAS_PI_CUR_SEL
	_rtd_part_outl(0x1800daa0, 21, 21, 0x0); //[APHY fix]  R-Lane PI_EYE_EN
	_rtd_part_outl(0x1800daa0, 23, 23, 0x0); //[APHY fix]  R-Lane KI SEL
	_rtd_part_outl(0x1800daa0, 25, 25, 0x0); //[APHY fix]  R-Lane FKP_RSTB_SEL
	_rtd_part_outl(0x1800daa0, 27, 26, 0x0); //[APHY fix]  R-Lane KI_CK_SEL
	_rtd_part_outl(0x1800daa0, 29, 29, 0x0); //[APHY fix]  R-Lane ST_CODE SEL
	_rtd_part_outl(0x1800daa0, 30, 30, 0x0); //[APHY fix]  R-Lane QCG Ckin SEL.
	_rtd_part_outl(0x1800daa4, 3, 2, 0x1); //[APHY fix]  R-Lane ACDR_SEL_DIV_FLD[1:0]
	_rtd_part_outl(0x1800daa4, 5, 4, 0x0); //[APHY fix]  R-Lane ACDR_SEL_DIV_REF[1:0]
	_rtd_part_outl(0x1800daa4, 7, 6, 0x0); //[APHY fix]  R-Lane ACDR_SEL_DIV_PLL[1:0]
	_rtd_part_outl(0x1800daa4, 9, 8, 0x0); //[APHY fix]  R-Lane ACDR_SEL_DIV_TRAINING[1:0]
	_rtd_part_outl(0x1800daa4, 12, 12, 0x0); //[APHY fix]  R-Lane dummy
	_rtd_part_outl(0x1800daa4, 13, 13, 0x0); //[APHY fix]  R-Lane ACDR_SEL_FLD_0CKFB_1CKREF
	_rtd_part_outl(0x1800daa4, 15, 14, 0x0); //[APHY fix]  R-Lane no use
	_rtd_part_outl(0x1800daa4, 18, 18, 0x1); //[APHY fix]  R-Lane ACDR_RSTB_DIV_PLL ... removed
	_rtd_part_outl(0x1800daa4, 24, 24, 0x0); //[APHY fix]  R-Lane ACDR_EN_UPDN_PULSE_FILTER
	_rtd_part_outl(0x1800daa4, 25, 25, 0x1); //[APHY fix]  R-Lane ACDR_RSTB_UPDN ... dummy
	_rtd_part_outl(0x1800daa4, 27, 26, 0x0); //[APHY fix]  R-Lane ACDR_SEL_UPDN_WIDTH[1:0]
	_rtd_part_outl(0x1800daa4, 28, 28, 0x1); //[APHY fix]  R-Lane ACDR_POW_LPF_IDAC
	_rtd_part_outl(0x1800daa4, 31, 29, 0x3); //[APHY fix]  R-Lane ACDR_SEL_LPF_IDAC[2:0]
	_rtd_part_outl(0x1800daa8, 0, 0, 0x1); //[APHY fix]  R-Lane ACDR_POW_CP
	_rtd_part_outl(0x1800daa8, 2, 2, 0x1); //[APHY fix]  R-Lane ACDR_POW_IDN_BBPD
	_rtd_part_outl(0x1800daa8, 3, 3, 0x0); //[APHY fix]  R-Lane no use
	_rtd_part_outl(0x1800daa8, 6, 4, 0x7); //[APHY fix]  R-Lane ACDR_SEL_TIE_IDN_BBPD
	_rtd_part_outl(0x1800daa8, 7, 7, 0x0); //[APHY fix]  R-Lane ACDR_POW_IBIAS_IDN_HV
	_rtd_part_outl(0x1800daa8, 8, 8, 0x1); //[APHY fix]  R-Lane ACDR_POW_VCO
	_rtd_part_outl(0x1800daa8, 9, 9, 0x1); //[APHY fix]  R-Lane ACDR_POW_VCO_VDAC
	_rtd_part_outl(0x1800daa8, 11, 10, 0x0); //[APHY fix]  R-Lane ACDR_SEL_V15_VDAC
	_rtd_part_outl(0x1800daa8, 14, 14, 0x0); //[APHY fix]  R-Lane no use
	_rtd_part_outl(0x1800daa8, 15, 15, 0x0); //[APHY fix]  R-Lane no use
	_rtd_part_outl(0x1800daa8, 16, 16, 0x0); //[APHY fix]  R-Lane ACDR_POW_TEST_MODE
	_rtd_part_outl(0x1800daa8, 18, 17, 0x0); //[APHY fix]  R-Lane ACDR_SEL_TEST_MODE
	_rtd_part_outl(0x1800daa8, 23, 19, 0x00); //[APHY fix]  R-Lane no use
	_rtd_part_outl(0x1800daa8, 31, 31, 0x0); //[APHY fix]  R-Lane no use
	_rtd_part_outl(0x1800daac, 15, 13, 0x1); //[APHY fix]  R-Lane no use
	_rtd_part_outl(0x1800daac, 19, 16, 0x0); //[APHY fix]  R-Lane LE1_ISEL_IN_2 -> dummy
	_rtd_part_outl(0x1800daac, 31, 25, 0x44); //[APHY fix]  R-Lane no use for LEQ
	_rtd_part_outl(0x1800dab4, 0, 0, 0x1); //[APHY fix]  Non-Hysteresis Amp Power
	_rtd_part_outl(0x1800dab4, 2, 1, 0x3); //[APHY fix]  CK Mode hysteresis amp Hysteresis adjustment[1:0]
	_rtd_part_outl(0x1800dab4, 3, 3, 0x0); //[APHY fix]  CK Mode detect source
	_rtd_part_outl(0x1800dab4, 5, 5, 0x0); //[APHY fix]  no use
	_rtd_part_outl(0x1800dab4, 6, 6, 0x1); //[APHY fix]  Hysteresis amp power
	_rtd_part_outl(0x1800dab4, 8, 8, 0x0); //[APHY fix]  CMU_WDPOW
	_rtd_part_outl(0x1800dab4, 9, 9, 0x0); //[APHY fix]  CMU_WDSET
	_rtd_part_outl(0x1800dab4, 10, 10, 0x0); //[APHY fix]  CMU_WDRST
	_rtd_part_outl(0x1800dab4, 11, 11, 0x1); //[APHY fix]  no use
	_rtd_part_outl(0x1800dab4, 12, 12, 0x0); //[APHY fix]  EN_TST_CKBK
	_rtd_part_outl(0x1800dab4, 13, 13, 0x0); //[APHY fix]  EN_TST_CKREF
	_rtd_part_outl(0x1800dab4, 15, 14, 0x0); //[APHY fix]  no use
	_rtd_part_outl(0x1800dab4, 23, 16, 0x6b); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dab4, 31, 24, 0x04); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dab8, 7, 0, 0x00); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dab8, 9, 9, 0x0); //[APHY fix]  no use
	_rtd_part_outl(0x1800dab8, 10, 10, 0x0); //[APHY fix]  CMU_SEL_CS
	_rtd_part_outl(0x1800dab8, 15, 11, 0x00); //[APHY fix]  no use
	_rtd_part_outl(0x1800dab8, 23, 16, 0x00); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dab8, 31, 24, 0x00); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dabc, 7, 0, 0x00); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dabc, 15, 8, 0x00); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dabc, 23, 16, 0x00); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dabc, 31, 24, 0x00); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dac0, 7, 0, 0x00); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dac0, 15, 8, 0x00); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dac0, 23, 16, 0x00); //[APHY fix]  dummy
	_rtd_part_outl(0x1800dac0, 31, 24, 0x00); //[APHY fix]  dummy
}
void APHY_Para_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing)
{
	// APHY para start
	_rtd_part_outl(0x1800da30, 9, 8, APHY_Para_frl_1[frl_timing].CK_2_2); //[APHY para]  CK-Lane RLSEL_LE1
	_rtd_part_outl(0x1800da30, 11, 10, APHY_Para_frl_1[frl_timing].CK_2_3); //[APHY para]  CK-Lane RLSEL_LE2
	_rtd_part_outl(0x1800da30, 13, 12, APHY_Para_frl_1[frl_timing].CK_2_4); //[APHY para]  CK-Lane RLSEL_NC
	_rtd_part_outl(0x1800da30, 15, 14, APHY_Para_frl_1[frl_timing].CK_2_5); //[APHY para]  CK-Lane RLSEL_TAP0
	_rtd_part_outl(0x1800da30, 19, 16, APHY_Para_frl_1[frl_timing].CK_3_6); //[APHY para]  CK-Lane RSSEL_LE1_1
	_rtd_part_outl(0x1800da30, 23, 20, APHY_Para_frl_1[frl_timing].CK_3_7); //[APHY para]  CK-Lane RSSEL_LE1_2
	_rtd_part_outl(0x1800da30, 27, 24, APHY_Para_frl_1[frl_timing].CK_4_8); //[APHY para]  CK-Lane RSSEL_LE2
	_rtd_part_outl(0x1800da30, 29, 28, APHY_Para_frl_1[frl_timing].CK_4_9); //[APHY para]  CK-Lane RSSEL_TAP0
	_rtd_part_outl(0x1800da30, 31, 30, APHY_Para_frl_1[frl_timing].CK_4_10); //[APHY para]  	CK-Lane KOFF_RANGE
	_rtd_part_outl(0x1800da34, 3, 0, APHY_Para_frl_1[frl_timing].CK_5_11); //[APHY para]    CK-Lane LE1_ISEL_IN
	_rtd_part_outl(0x1800da34, 7, 4, APHY_Para_frl_1[frl_timing].CK_5_12); //[APHY para]    CK-Lane LE2_ISEL_IN
	_rtd_part_outl(0x1800da34, 11, 8, APHY_Para_frl_1[frl_timing].CK_6_13); //[APHY para]   	CK-Lane LE_NC_ISEL_IN
	_rtd_part_outl(0x1800da34, 15, 12, 0xf); //[APHY para]  	CK-Lane TAP0_ISEL_IN
	_rtd_part_outl(0x1800da34, 17, 16, 0x0); //[APHY para]   	CK-Lane LE_IHALF[1:0]
	_rtd_part_outl(0x1800da34, 18, 18, APHY_Para_frl_1[frl_timing].CK_7_16); //[APHY para]   	CK-Lane NC_IHALF
	_rtd_part_outl(0x1800da34, 19, 19, APHY_Para_frl_1[frl_timing].CK_7_17); //[APHY para]   	CK-Lane TAP0_IHALF
	_rtd_part_outl(0x1800da34, 20, 20, 0x0); //[APHY para]   	CK-Lane EN_LEQ6G
	_rtd_part_outl(0x1800da34, 22, 22, APHY_Para_frl_1[frl_timing].CK_7_19); //[APHY para]   	CK-Lane POW_NC
	_rtd_part_outl(0x1800da34, 30, 28, 0x3); //[APHY para]   CK-Lane DATALANE_BIAS_ISEL[2:0]
	_rtd_part_outl(0x1800da34, 31, 31, 0x1); //[APHY para]   CK-Lane POW_LEQ_RL
	_rtd_part_outl(0x1800da38, 1, 1, 0x0); //[APHY para]  		CK-Lane SEL_CMFB_LS
	_rtd_part_outl(0x1800da38, 3, 2, 0x3); //[APHY para]  		CK-Lane LEQ_CUR_ADJ
	_rtd_part_outl(0x1800da38, 5, 4, 0x0); //[APHY para]  		CK-Lane PTAT_CUR_ADJ
	_rtd_part_outl(0x1800da38, 6, 6, 0x0); //[APHY para]  		CK-Lane REG_BIAS_POW_CON_GM
	_rtd_part_outl(0x1800da38, 11, 9, 0x0); //[APHY para] 		CK-Lane PTAT_CUR_ADJ_FINE
	_rtd_part_outl(0x1800da38, 16, 16, APHY_Para_frl_1[frl_timing].CK_11_27); //[APHY para] 		CK-Lane FR_CK_SEL
	_rtd_part_outl(0x1800da3c, 11, 10, APHY_Para_frl_1[frl_timing].CK_14_28); //[APHY para]   	CK-Lane DEMUX_PIX_RATE_SEL<1:0>
	_rtd_part_outl(0x1800da3c, 12, 12, APHY_Para_frl_1[frl_timing].CK_14_29); //[APHY para]   	CK-Lane DEMUX_FR_CK_SEL
	_rtd_part_outl(0x1800da3c, 15, 13, 0x1); //[APHY para]   	CK-Lane DEMUX_RATE_SEL
	_rtd_part_outl(0x1800da3c, 24, 24, 0x0); //[APHY para]  		CK-Lane QCG1_SEL_BIAS_ST
	_rtd_part_outl(0x1800da3c, 25, 25, 0x0); //[APHY para]  		CK-Lane QCG1_SEL_BIAS_RG
	_rtd_part_outl(0x1800da3c, 27, 26, 0x0); //[APHY para]  		CK-Lane QCG1 Rload SEL
	_rtd_part_outl(0x1800da3c, 31, 28, 0xd); //[APHY para]  		CK-Lane QCG1 Cload SEL
	_rtd_part_outl(0x1800da40, 0, 0, 0x0); //[APHY para]  CK-Lane QCG2_SEL_BIAS_ST
	_rtd_part_outl(0x1800da40, 1, 1, 0x0); //[APHY para]  CK-Lane QCG2_SEL_BIAS_RG
	_rtd_part_outl(0x1800da40, 3, 2, 0x0); //[APHY para]  CK-Lane QCG2 Rload SEL
	_rtd_part_outl(0x1800da40, 7, 4, 0xd); //[APHY para]  CK-Lane QCG2 Cload SEL
	_rtd_part_outl(0x1800da40, 11, 9, 0x0); //[APHY para]  CK-Lane PI_ISEL<2:0>
	_rtd_part_outl(0x1800da40, 14, 14, 0x0); //[APHY para]  CK-Lane PI_CSEL
	_rtd_part_outl(0x1800da40, 20, 18, 0x4); //[APHY para]  CK-Lane PI_DIV_SEL
	_rtd_part_outl(0x1800da40, 22, 22, 0x1); //[APHY para]  CK-Lane DCDR ckout SEL
	_rtd_part_outl(0x1800da40, 31, 31, 0x0); //[APHY para]  CK-Lane DFE Ckin SEL.
	_rtd_part_outl(0x1800da44, 11, 10, APHY_Para_frl_1[frl_timing].ACDR_CK_2_44); //[APHY para]  CK-Lane ACDR_SEL_IDNBIAS_LV[1:0]
	_rtd_part_outl(0x1800da44, 17, 17, 0x1); //[APHY para]  CK-Lane ACDR_RSTB_DIV_BAND_2OR4_LV
	_rtd_part_outl(0x1800da44, 19, 19, APHY_Para_frl_1[frl_timing].ACDR_CK_3_46); //[APHY para]  CK-Lane  ACDR_SEL_HS_CLK
	_rtd_part_outl(0x1800da44, 20, 20, 0x1); //[APHY para]  CK-Lane ACDR_SEL_0FR_1HR_DIV_IQ
	_rtd_part_outl(0x1800da44, 23, 21, 0x0); //[APHY para]  CK-Lane ACDR_SEL_DIV_BAND_2OR4_LV
	_rtd_part_outl(0x1800da48, 1, 1, 0x0); //[APHY para]  CK-Lane ACDR_POW_CP_INTG2_CORE
	_rtd_part_outl(0x1800da48, 13, 12, APHY_Para_frl_1[frl_timing].ACDR_CK_6_50); //[APHY para]  CK-Lane ACDR_SEL_BAND_CAP
	_rtd_part_outl(0x1800da48, 26, 25, APHY_Para_frl_1[frl_timing].ACDR_CK_8_51); //[APHY para]  CK-Lane BBPD_KP_SEL[1:0]
	_rtd_part_outl(0x1800da48, 28, 27, 0x1); //[APHY para]  CK-Lane BBPD_KI_SEL[1:0]
	_rtd_part_outl(0x1800da48, 29, 29, 0x0); //[APHY para]  CK-Lane BBPD_BYPASS_CTN_KP
	_rtd_part_outl(0x1800da48, 30, 30, 0x1); //[APHY para]  CK-Lane BBPD_BYPASS_CTN_KI
	_rtd_part_outl(0x1800da4c, 7, 0, APHY_Para_frl_1[frl_timing].ACDR_CK_9_55); //[APHY para]  CK-Lane CMU_SEL_M_DIV[7:0]
	_rtd_part_outl(0x1800da4c, 8, 8, 0x0); //[APHY para]  CK-Lane CMU_SEL_M_DIV[8]
	_rtd_part_outl(0x1800da4c, 9, 9, 0x0); //[APHY para]  CK-Lane CMU_SEL_PRE_M_DIV
	_rtd_part_outl(0x1800da4c, 10, 10, APHY_Para_frl_1[frl_timing].ACDR_CK_10_58); //[APHY para]  CK-Lane CMU_SEL_POST_M_DIV
	_rtd_part_outl(0x1800da4c, 21, 20, APHY_Para_frl_1[frl_timing].ACDR_CK_11_59); //[APHY para]  CK-Lane RLSEL_LE1_2
	_rtd_part_outl(0x1800da4c, 23, 22, APHY_Para_frl_1[frl_timing].ACDR_CK_11_60); //[APHY para]  CK-Lane RLSEL_NC_2
	_rtd_part_outl(0x1800da4c, 24, 24, APHY_Para_frl_1[frl_timing].ACDR_CK_12_61); //[APHY para]  CK-Lane POW_NC
	_rtd_part_outl(0x1800da50, 9, 8, APHY_Para_frl_1[frl_timing].B_2_62); //[APHY para]  		B-Lane RLSEL_LE1
	_rtd_part_outl(0x1800da50, 11, 10, APHY_Para_frl_1[frl_timing].B_2_63); //[APHY para]  		B-Lane RLSEL_LE2
	_rtd_part_outl(0x1800da50, 13, 12, APHY_Para_frl_1[frl_timing].B_2_64); //[APHY para]  		B-Lane RLSEL_NC
	_rtd_part_outl(0x1800da50, 15, 14, APHY_Para_frl_1[frl_timing].B_2_65); //[APHY para]  		B-Lane RLSEL_TAP0
	_rtd_part_outl(0x1800da50, 19, 16, APHY_Para_frl_1[frl_timing].B_3_66); //[APHY para]  		B-Lane RSSEL_LE1_1
	_rtd_part_outl(0x1800da50, 23, 20, APHY_Para_frl_1[frl_timing].B_3_67); //[APHY para]  		B-Lane RSSEL_LE1_2
	_rtd_part_outl(0x1800da50, 27, 24, APHY_Para_frl_1[frl_timing].B_4_68); //[APHY para]  		B-Lane RSSEL_LE2
	_rtd_part_outl(0x1800da50, 29, 28, APHY_Para_frl_1[frl_timing].B_4_69); //[APHY para]  		B-Lane RSSEL_TAP0
	_rtd_part_outl(0x1800da50, 31, 30, APHY_Para_frl_1[frl_timing].B_4_70); //[APHY para]  	B-Lane KOFF_RANGE
	_rtd_part_outl(0x1800da54, 3, 0, APHY_Para_frl_1[frl_timing].B_5_71); //[APHY para]    	B-Lane LE1_ISEL_IN
	_rtd_part_outl(0x1800da54, 7, 4, APHY_Para_frl_1[frl_timing].B_5_72); //[APHY para]    	B-Lane LE2_ISEL_IN
	_rtd_part_outl(0x1800da54, 11, 8, APHY_Para_frl_1[frl_timing].B_6_73); //[APHY para]   		B-Lane LE_NC_ISEL_IN
	_rtd_part_outl(0x1800da54, 15, 12, 0xf); //[APHY para]  		B-Lane TAP0_ISEL_IN
	_rtd_part_outl(0x1800da54, 17, 16, 0x0); //[APHY para]   		B-Lane LE_IHALF[1:0]
	_rtd_part_outl(0x1800da54, 18, 18, APHY_Para_frl_1[frl_timing].B_7_76); //[APHY para]   		B-Lane NC_IHALF
	_rtd_part_outl(0x1800da54, 19, 19, APHY_Para_frl_1[frl_timing].B_7_77); //[APHY para]   		B-Lane TAP0_IHALF
	_rtd_part_outl(0x1800da54, 20, 20, 0x0); //[APHY para]   		B-Lane EN_LEQ6G
	_rtd_part_outl(0x1800da54, 22, 22, APHY_Para_frl_1[frl_timing].B_7_79); //[APHY para]   	B-Lane POW_NC
	_rtd_part_outl(0x1800da54, 30, 28, 0x3); //[APHY para]   		B-Lane DATALANE_BIAS_ISEL[2:0]
	_rtd_part_outl(0x1800da54, 31, 31, 0x1); //[APHY para]   		B-Lane POW_LEQ_RL
	_rtd_part_outl(0x1800da58, 1, 1, 0x0); //[APHY para]  		B-Lane SEL_CMFB_LS
	_rtd_part_outl(0x1800da58, 3, 2, 0x3); //[APHY para]  		B-Lane LEQ_CUR_ADJ
	_rtd_part_outl(0x1800da58, 5, 4, 0x0); //[APHY para]  		B-Lane PTAT_CUR_ADJ
	_rtd_part_outl(0x1800da58, 6, 6, 0x0); //[APHY para]  		B-Lane REG_BIAS_POW_CON_GM
	_rtd_part_outl(0x1800da58, 11, 9, 0x0); //[APHY para] 		B-Lane PTAT_CUR_ADJ_FINE
	_rtd_part_outl(0x1800da58, 16, 16, APHY_Para_frl_1[frl_timing].B_11_87); //[APHY para] 		B-Lane FR_CK_SEL
	_rtd_part_outl(0x1800da5c, 11, 10, APHY_Para_frl_1[frl_timing].B_14_88); //[APHY para]   	B-Lane DEMUX_PIX_RATE_SEL<1:0>
	_rtd_part_outl(0x1800da5c, 12, 12, APHY_Para_frl_1[frl_timing].B_14_89); //[APHY para]   	B-Lane DEMUX_FR_CK_SEL
	_rtd_part_outl(0x1800da5c, 15, 13, 0x1); //[APHY para]   	B-Lane DEMUX_RATE_SEL
	_rtd_part_outl(0x1800da5c, 24, 24, 0x0); //[APHY para]  		B-Lane QCG1_SEL_BIAS_ST
	_rtd_part_outl(0x1800da5c, 25, 25, 0x0); //[APHY para]  		B-Lane QCG1_SEL_BIAS_RG
	_rtd_part_outl(0x1800da5c, 27, 26, 0x0); //[APHY para]  		B-Lane QCG1 Rload SEL
	_rtd_part_outl(0x1800da5c, 31, 28, 0xd); //[APHY para]  		B-Lane QCG1 Cload SEL
	_rtd_part_outl(0x1800da60, 0, 0, 0x0); //[APHY para]  B-Lane QCG2_SEL_BIAS_ST
	_rtd_part_outl(0x1800da60, 1, 1, 0x0); //[APHY para]  B-Lane QCG2_SEL_BIAS_RG
	_rtd_part_outl(0x1800da60, 3, 2, 0x0); //[APHY para]  B-Lane QCG2 Rload SEL
	_rtd_part_outl(0x1800da60, 7, 4, 0xd); //[APHY para]  B-Lane QCG2 Cload SEL
	_rtd_part_outl(0x1800da60, 11, 9, 0x0); //[APHY para]  B-Lane PI_ISEL<2:0>
	_rtd_part_outl(0x1800da60, 14, 14, 0x0); //[APHY para]  B-Lane PI_CSEL
	_rtd_part_outl(0x1800da60, 20, 18, 0x4); //[APHY para]  B-Lane PI_DIV_SEL
	_rtd_part_outl(0x1800da60, 22, 22, 0x1); //[APHY para]  B-Lane DCDR ckout SEL
	_rtd_part_outl(0x1800da60, 31, 31, 0x0); //[APHY para]  B-Lane DFE Ckin SEL.
	_rtd_part_outl(0x1800da64, 11, 10, APHY_Para_frl_1[frl_timing].ACDR_B_2_104); //[APHY para]  B-Lane ACDR_SEL_IDNBIAS_LV[1:0]
	_rtd_part_outl(0x1800da64, 17, 17, 0x1); //[APHY para]  B-Lane ACDR_RSTB_DIV_BAND_2OR4_LV
	_rtd_part_outl(0x1800da64, 19, 19, APHY_Para_frl_1[frl_timing].ACDR_B_3_106); //[APHY para]  B-Lane  ACDR_SEL_HS_CLK
	_rtd_part_outl(0x1800da64, 20, 20, 0x1); //[APHY para]  B-Lane ACDR_SEL_0FR_1HR_DIV_IQ
	_rtd_part_outl(0x1800da64, 23, 21, 0x0); //[APHY para]  B-Lane ACDR_SEL_DIV_BAND_2OR4_LV
	_rtd_part_outl(0x1800da68, 1, 1, 0x0); //[APHY para]  B-Lane ACDR_POW_CP_INTG2_CORE
	_rtd_part_outl(0x1800da68, 13, 12, APHY_Para_frl_1[frl_timing].ACDR_B_6_110); //[APHY para]  B-Lane ACDR_SEL_BAND_CAP
	_rtd_part_outl(0x1800da68, 26, 25, APHY_Para_frl_1[frl_timing].ACDR_B_8_111); //[APHY para]  B-Lane BBPD_KP_SEL[1:0]
	_rtd_part_outl(0x1800da68, 28, 27, 0x1); //[APHY para]  B-Lane BBPD_KI_SEL[1:0]
	_rtd_part_outl(0x1800da68, 29, 29, 0x0); //[APHY para]  B-Lane BBPD_BYPASS_CTN_KP
	_rtd_part_outl(0x1800da68, 30, 30, 0x1); //[APHY para]  B-Lane BBPD_BYPASS_CTN_KI
	_rtd_part_outl(0x1800da6c, 7, 0, APHY_Para_frl_1[frl_timing].ACDR_B_9_115); //[APHY para]  B-Lane CMU_SEL_M_DIV[7:0]
	_rtd_part_outl(0x1800da6c, 8, 8, 0x0); //[APHY para]  B-Lane CMU_SEL_M_DIV[8]
	_rtd_part_outl(0x1800da6c, 9, 9, 0x0); //[APHY para]  B-Lane CMU_SEL_PRE_M_DIV
	_rtd_part_outl(0x1800da6c, 10, 10, APHY_Para_frl_1[frl_timing].ACDR_B_10_118); //[APHY para]  B-Lane CMU_SEL_POST_M_DIV
	_rtd_part_outl(0x1800da6c, 21, 20, APHY_Para_frl_1[frl_timing].ACDR_B_11_119); //[APHY para]  B-Lane RLSEL_LE1_2
	_rtd_part_outl(0x1800da6c, 23, 22, APHY_Para_frl_1[frl_timing].ACDR_B_11_120); //[APHY para]  B-Lane RLSEL_NC_2
	_rtd_part_outl(0x1800da6c, 24, 24, APHY_Para_frl_1[frl_timing].ACDR_B_12_121); //[APHY para]  B-Lane POW_NC
	_rtd_part_outl(0x1800da70, 9, 8, APHY_Para_frl_1[frl_timing].G_2_122); //[APHY para]  		G-Lane RLSEL_LE1
	_rtd_part_outl(0x1800da70, 11, 10, APHY_Para_frl_1[frl_timing].G_2_123); //[APHY para]  		G-Lane RLSEL_LE2
	_rtd_part_outl(0x1800da70, 13, 12, APHY_Para_frl_1[frl_timing].G_2_124); //[APHY para]  		G-Lane RLSEL_NC
	_rtd_part_outl(0x1800da70, 15, 14, APHY_Para_frl_1[frl_timing].G_2_125); //[APHY para]  		G-Lane RLSEL_TAP0
	_rtd_part_outl(0x1800da70, 19, 16, APHY_Para_frl_1[frl_timing].G_3_126); //[APHY para]  		G-Lane RSSEL_LE1_1
	_rtd_part_outl(0x1800da70, 23, 20, APHY_Para_frl_1[frl_timing].G_3_127); //[APHY para]  		G-Lane RSSEL_LE1_2
	_rtd_part_outl(0x1800da70, 27, 24, APHY_Para_frl_1[frl_timing].G_4_128); //[APHY para]  		G-Lane RSSEL_LE2
	_rtd_part_outl(0x1800da70, 29, 28, APHY_Para_frl_1[frl_timing].G_4_129); //[APHY para]  		G-Lane RSSEL_TAP0
	_rtd_part_outl(0x1800da70, 31, 30, APHY_Para_frl_1[frl_timing].G_4_130); //[APHY para]  	G-Lane KOFF_RANGE
	_rtd_part_outl(0x1800da74, 3, 0, APHY_Para_frl_1[frl_timing].G_5_131); //[APHY para]    	G-Lane LE1_ISEL_IN
	_rtd_part_outl(0x1800da74, 7, 4, APHY_Para_frl_1[frl_timing].G_5_132); //[APHY para]    	G-Lane LE2_ISEL_IN
	_rtd_part_outl(0x1800da74, 11, 8, APHY_Para_frl_1[frl_timing].G_6_133); //[APHY para]   		G-Lane LE_NC_ISEL_IN
	_rtd_part_outl(0x1800da74, 15, 12, 0xf); //[APHY para]  		G-Lane TAP0_ISEL_IN
	_rtd_part_outl(0x1800da74, 17, 16, 0x0); //[APHY para]   		G-Lane LE_IHALF[1:0]
	_rtd_part_outl(0x1800da74, 18, 18, APHY_Para_frl_1[frl_timing].G_7_136); //[APHY para]   		G-Lane NC_IHALF
	_rtd_part_outl(0x1800da74, 19, 19, APHY_Para_frl_1[frl_timing].G_7_137); //[APHY para]   		G-Lane TAP0_IHALF
	_rtd_part_outl(0x1800da74, 20, 20, 0x0); //[APHY para]   		G-Lane EN_LEQ6G
	_rtd_part_outl(0x1800da74, 22, 22, APHY_Para_frl_1[frl_timing].G_7_139); //[APHY para]   	G-Lane POW_NC
	_rtd_part_outl(0x1800da74, 30, 28, 0x3); //[APHY para]   		G-Lane DATALANE_BIAS_ISEL[2:0]
	_rtd_part_outl(0x1800da74, 31, 31, 0x1); //[APHY para]   		G-Lane POW_LEQ_RL
	_rtd_part_outl(0x1800da78, 1, 1, 0x0); //[APHY para]  		G-Lane SEL_CMFB_LS
	_rtd_part_outl(0x1800da78, 3, 2, 0x3); //[APHY para]  		G-Lane LEQ_CUR_ADJ
	_rtd_part_outl(0x1800da78, 5, 4, 0x0); //[APHY para]  		G-Lane PTAT_CUR_ADJ
	_rtd_part_outl(0x1800da78, 6, 6, 0x0); //[APHY para]  		G-Lane REG_BIAS_POW_CON_GM
	_rtd_part_outl(0x1800da78, 11, 9, 0x0); //[APHY para] 		G-Lane PTAT_CUR_ADJ_FINE
	_rtd_part_outl(0x1800da78, 16, 16, APHY_Para_frl_1[frl_timing].G_11_147); //[APHY para] 		G-Lane FR_CK_SEL
	_rtd_part_outl(0x1800da7c, 11, 10, APHY_Para_frl_1[frl_timing].G_14_148); //[APHY para]   	G-Lane DEMUX_PIX_RATE_SEL<1:0>
	_rtd_part_outl(0x1800da7c, 12, 12, APHY_Para_frl_1[frl_timing].G_14_149); //[APHY para]   	G-Lane DEMUX_FR_CK_SEL
	_rtd_part_outl(0x1800da7c, 15, 13, 0x1); //[APHY para]   	G-Lane DEMUX_RATE_SEL
	_rtd_part_outl(0x1800da7c, 24, 24, 0x0); //[APHY para]  		G-Lane QCG1_SEL_BIAS_ST
	_rtd_part_outl(0x1800da7c, 25, 25, 0x0); //[APHY para]  		G-Lane QCG1_SEL_BIAS_RG
	_rtd_part_outl(0x1800da7c, 27, 26, 0x0); //[APHY para]  		G-Lane QCG1 Rload SEL
	_rtd_part_outl(0x1800da7c, 31, 28, 0xd); //[APHY para]  		G-Lane QCG1 Cload SEL
	_rtd_part_outl(0x1800da80, 0, 0, 0x0); //[APHY para]  G-Lane QCG2_SEL_BIAS_ST
	_rtd_part_outl(0x1800da80, 1, 1, 0x0); //[APHY para]  G-Lane QCG2_SEL_BIAS_RG
	_rtd_part_outl(0x1800da80, 3, 2, 0x0); //[APHY para]  G-Lane QCG2 Rload SEL
	_rtd_part_outl(0x1800da80, 7, 4, 0xd); //[APHY para]  G-Lane QCG2 Cload SEL
	_rtd_part_outl(0x1800da80, 11, 9, 0x0); //[APHY para]  G-Lane PI_ISEL<2:0>
	_rtd_part_outl(0x1800da80, 14, 14, 0x0); //[APHY para]  G-Lane PI_CSEL
	_rtd_part_outl(0x1800da80, 20, 18, 0x4); //[APHY para]  G-Lane PI_DIV_SEL
	_rtd_part_outl(0x1800da80, 22, 22, 0x1); //[APHY para]  G-Lane DCDR ckout SEL
	_rtd_part_outl(0x1800da80, 31, 31, 0x0); //[APHY para]  G-Lane DFE Ckin SEL.
	_rtd_part_outl(0x1800da84, 11, 10, APHY_Para_frl_1[frl_timing].ACDR_G_2_164); //[APHY para]  G-Lane ACDR_SEL_IDNBIAS_LV[1:0]
	_rtd_part_outl(0x1800da84, 17, 17, 0x1); //[APHY para]  G-Lane ACDR_RSTB_DIV_BAND_2OR4_LV
	_rtd_part_outl(0x1800da84, 19, 19, APHY_Para_frl_1[frl_timing].ACDR_G_3_166); //[APHY para]  G-Lane  ACDR_SEL_HS_CLK
	_rtd_part_outl(0x1800da84, 20, 20, 0x1); //[APHY para]  G-Lane ACDR_SEL_0FR_1HR_DIV_IQ
	_rtd_part_outl(0x1800da84, 23, 21, 0x0); //[APHY para]  G-Lane ACDR_SEL_DIV_BAND_2OR4_LV
	_rtd_part_outl(0x1800da88, 1, 1, 0x0); //[APHY para]  G-Lane ACDR_POW_CP_INTG2_CORE
	_rtd_part_outl(0x1800da88, 13, 12, APHY_Para_frl_1[frl_timing].ACDR_G_6_170); //[APHY para]  G-Lane ACDR_SEL_BAND_CAP
	_rtd_part_outl(0x1800da88, 26, 25, APHY_Para_frl_1[frl_timing].ACDR_G_8_171); //[APHY para]  G-Lane BBPD_KP_SEL[1:0]
	_rtd_part_outl(0x1800da88, 28, 27, 0x1); //[APHY para]  G-Lane BBPD_KI_SEL[1:0]
	_rtd_part_outl(0x1800da88, 29, 29, 0x0); //[APHY para]  G-Lane BBPD_BYPASS_CTN_KP
	_rtd_part_outl(0x1800da88, 30, 30, 0x1); //[APHY para]  G-Lane BBPD_BYPASS_CTN_KI
	_rtd_part_outl(0x1800da8c, 7, 0, APHY_Para_frl_1[frl_timing].ACDR_G_9_175); //[APHY para]  G-Lane CMU_SEL_M_DIV[7:0]
	_rtd_part_outl(0x1800da8c, 8, 8, 0x0); //[APHY para]  G-Lane CMU_SEL_M_DIV[8]
	_rtd_part_outl(0x1800da8c, 9, 9, 0x0); //[APHY para]  G-Lane CMU_SEL_PRE_M_DIV
	_rtd_part_outl(0x1800da8c, 10, 10, APHY_Para_frl_1[frl_timing].ACDR_G_10_178); //[APHY para]  G-Lane CMU_SEL_POST_M_DIV
	_rtd_part_outl(0x1800da8c, 21, 20, APHY_Para_frl_1[frl_timing].ACDR_G_11_179); //[APHY para]  G-Lane RLSEL_LE1_2
	_rtd_part_outl(0x1800da8c, 23, 22, APHY_Para_frl_1[frl_timing].ACDR_G_11_180); //[APHY para]  G-Lane RLSEL_NC_2
	_rtd_part_outl(0x1800da8c, 24, 24, APHY_Para_frl_1[frl_timing].ACDR_G_12_181); //[APHY para]  G-Lane POW_NC
	_rtd_part_outl(0x1800da90, 9, 8, APHY_Para_frl_1[frl_timing].R_2_182); //[APHY para]  		R-Lane RLSEL_LE1
	_rtd_part_outl(0x1800da90, 11, 10, APHY_Para_frl_1[frl_timing].R_2_183); //[APHY para]  		R-Lane RLSEL_LE2
	_rtd_part_outl(0x1800da90, 13, 12, APHY_Para_frl_1[frl_timing].R_2_184); //[APHY para]  		R-Lane RLSEL_NC
	_rtd_part_outl(0x1800da90, 15, 14, APHY_Para_frl_1[frl_timing].R_2_185); //[APHY para]  		R-Lane RLSEL_TAP0
	_rtd_part_outl(0x1800da90, 19, 16, APHY_Para_frl_1[frl_timing].R_3_186); //[APHY para]  		R-Lane RSSEL_LE1_1
	_rtd_part_outl(0x1800da90, 23, 20, APHY_Para_frl_1[frl_timing].R_3_187); //[APHY para]  		R-Lane RSSEL_LE1_2
	_rtd_part_outl(0x1800da90, 27, 24, APHY_Para_frl_1[frl_timing].R_4_188); //[APHY para]  		R-Lane RSSEL_LE2
	_rtd_part_outl(0x1800da90, 29, 28, APHY_Para_frl_1[frl_timing].R_4_189); //[APHY para]  		R-Lane RSSEL_TAP0
	_rtd_part_outl(0x1800da90, 31, 30, APHY_Para_frl_1[frl_timing].R_4_190); //[APHY para]  	R-Lane KOFF_RANGE
	_rtd_part_outl(0x1800da94, 3, 0, APHY_Para_frl_1[frl_timing].R_5_191); //[APHY para]    	R-Lane LE1_ISEL_IN
	_rtd_part_outl(0x1800da94, 7, 4, APHY_Para_frl_1[frl_timing].R_5_192); //[APHY para]    	R-Lane LE2_ISEL_IN
	_rtd_part_outl(0x1800da94, 11, 8, APHY_Para_frl_1[frl_timing].R_6_193); //[APHY para]   		R-Lane LE_NC_ISEL_IN
	_rtd_part_outl(0x1800da94, 15, 12, 0xf); //[APHY para]  		R-Lane TAP0_ISEL_IN
	_rtd_part_outl(0x1800da94, 17, 16, 0x0); //[APHY para]   		R-Lane LE_IHALF[1:0]
	_rtd_part_outl(0x1800da94, 18, 18, APHY_Para_frl_1[frl_timing].R_7_196); //[APHY para]   		R-Lane NC_IHALF
	_rtd_part_outl(0x1800da94, 19, 19, APHY_Para_frl_1[frl_timing].R_7_197); //[APHY para]   		R-Lane TAP0_IHALF
	_rtd_part_outl(0x1800da94, 20, 20, 0x0); //[APHY para]   		R-Lane EN_LEQ6G
	_rtd_part_outl(0x1800da94, 22, 22, APHY_Para_frl_1[frl_timing].R_7_199); //[APHY para]   	R-Lane POW_NC
	_rtd_part_outl(0x1800da94, 30, 28, 0x3); //[APHY para]   		R-Lane DATALANE_BIAS_ISEL[2:0]
	_rtd_part_outl(0x1800da94, 31, 31, 0x1); //[APHY para]   		R-Lane POW_LEQ_RL
	_rtd_part_outl(0x1800da98, 1, 1, 0x0); //[APHY para]  		R-Lane SEL_CMFB_LS
	_rtd_part_outl(0x1800da98, 3, 2, 0x3); //[APHY para]  		R-Lane LEQ_CUR_ADJ
	_rtd_part_outl(0x1800da98, 5, 4, 0x0); //[APHY para]  		R-Lane PTAT_CUR_ADJ
	_rtd_part_outl(0x1800da98, 6, 6, 0x0); //[APHY para]  		R-Lane REG_BIAS_POW_CON_GM
	_rtd_part_outl(0x1800da98, 11, 9, 0x0); //[APHY para] 		R-Lane PTAT_CUR_ADJ_FINE
	_rtd_part_outl(0x1800da98, 16, 16, APHY_Para_frl_1[frl_timing].R_11_207); //[APHY para] 		R-Lane FR_CK_SEL
	_rtd_part_outl(0x1800da9c, 11, 10, APHY_Para_frl_1[frl_timing].R_14_208); //[APHY para]   	R-Lane DEMUX_PIX_RATE_SEL<1:0>
	_rtd_part_outl(0x1800da9c, 12, 12, APHY_Para_frl_1[frl_timing].R_14_209); //[APHY para]   	R-Lane DEMUX_FR_CK_SEL
	_rtd_part_outl(0x1800da9c, 15, 13, 0x1); //[APHY para]   	R-Lane DEMUX_RATE_SEL
	_rtd_part_outl(0x1800da9c, 24, 24, 0x0); //[APHY para]  		R-Lane QCG1_SEL_BIAS_ST
	_rtd_part_outl(0x1800da9c, 25, 25, 0x0); //[APHY para]  		R-Lane QCG1_SEL_BIAS_RG
	_rtd_part_outl(0x1800da9c, 27, 26, 0x0); //[APHY para]  		R-Lane QCG1 Rload SEL
	_rtd_part_outl(0x1800da9c, 31, 28, 0xd); //[APHY para]  		R-Lane QCG1 Cload SEL
	_rtd_part_outl(0x1800daa0, 0, 0, 0x0); //[APHY para]  R-Lane QCG2_SEL_BIAS_ST
	_rtd_part_outl(0x1800daa0, 1, 1, 0x0); //[APHY para]  R-Lane QCG2_SEL_BIAS_RG
	_rtd_part_outl(0x1800daa0, 3, 2, 0x0); //[APHY para]  R-Lane QCG2 Rload SEL
	_rtd_part_outl(0x1800daa0, 7, 4, 0xd); //[APHY para]  R-Lane QCG2 Cload SEL
	_rtd_part_outl(0x1800daa0, 11, 9, 0x0); //[APHY para]  R-Lane PI_ISEL<2:0>
	_rtd_part_outl(0x1800daa0, 14, 14, 0x0); //[APHY para]  R-Lane PI_CSEL
	_rtd_part_outl(0x1800daa0, 20, 18, 0x4); //[APHY para]  R-Lane PI_DIV_SEL
	_rtd_part_outl(0x1800daa0, 22, 22, 0x1); //[APHY para]  R-Lane DCDR ckout SEL
	_rtd_part_outl(0x1800daa0, 31, 31, 0x0); //[APHY para]  R-Lane DFE Ckin SEL.
	_rtd_part_outl(0x1800daa4, 11, 10, APHY_Para_frl_1[frl_timing].ACDR_R_2_224); //[APHY para]  R-Lane ACDR_SEL_IDNBIAS_LV[1:0]
	_rtd_part_outl(0x1800daa4, 17, 17, 0x1); //[APHY para]  R-Lane ACDR_RSTB_DIV_BAND_2OR4_LV
	_rtd_part_outl(0x1800daa4, 19, 19, APHY_Para_frl_1[frl_timing].ACDR_R_3_226); //[APHY para]  R-Lane  ACDR_SEL_HS_CLK
	_rtd_part_outl(0x1800daa4, 20, 20, 0x1); //[APHY para]  R-Lane ACDR_SEL_0FR_1HR_DIV_IQ
	_rtd_part_outl(0x1800daa4, 23, 21, 0x0); //[APHY para]  R-Lane ACDR_SEL_DIV_BAND_2OR4_LV
	_rtd_part_outl(0x1800daa8, 1, 1, 0x0); //[APHY para]  R-Lane ACDR_POW_CP_INTG2_CORE
	_rtd_part_outl(0x1800daa8, 13, 12, APHY_Para_frl_1[frl_timing].ACDR_R_6_230); //[APHY para]  R-Lane ACDR_SEL_BAND_CAP
	_rtd_part_outl(0x1800daa8, 26, 25, APHY_Para_frl_1[frl_timing].ACDR_R_8_231); //[APHY para]  R-Lane BBPD_KP_SEL[1:0]
	_rtd_part_outl(0x1800daa8, 28, 27, 0x1); //[APHY para]  R-Lane BBPD_KI_SEL[1:0]
	_rtd_part_outl(0x1800daa8, 29, 29, 0x0); //[APHY para]  R-Lane BBPD_BYPASS_CTN_KP
	_rtd_part_outl(0x1800daa8, 30, 30, 0x1); //[APHY para]  R-Lane BBPD_BYPASS_CTN_KI
	_rtd_part_outl(0x1800daac, 7, 0, APHY_Para_frl_1[frl_timing].ACDR_R_9_235); //[APHY para]  R-Lane CMU_SEL_M_DIV[7:0]
	_rtd_part_outl(0x1800daac, 8, 8, 0x0); //[APHY para]  R-Lane CMU_SEL_M_DIV[8]
	_rtd_part_outl(0x1800daac, 9, 9, 0x0); //[APHY para]  R-Lane CMU_SEL_PRE_M_DIV
	_rtd_part_outl(0x1800daac, 10, 10, APHY_Para_frl_1[frl_timing].ACDR_R_10_238); //[APHY para]  R-Lane CMU_SEL_POST_M_DIV
	_rtd_part_outl(0x1800daac, 21, 20, APHY_Para_frl_1[frl_timing].ACDR_R_11_239); //[APHY para]  R-Lane RLSEL_LE1_2
	_rtd_part_outl(0x1800daac, 23, 22, APHY_Para_frl_1[frl_timing].ACDR_R_11_240); //[APHY para]  R-Lane RLSEL_NC_2
	_rtd_part_outl(0x1800daac, 24, 24, APHY_Para_frl_1[frl_timing].ACDR_R_12_241); //[APHY para]  R-Lane POW_NC
	_rtd_part_outl(0x1800dab4, 4, 4, 0x0); //[APHY para]  CMU input clock source select:
	_rtd_part_outl(0x1800dab4, 7, 7, 0x0); //[APHY para]  CMU_CKXTAL_SEL
	_rtd_part_outl(0x1800dab8, 8, 8, 0x0); //[APHY para  CMU_SEL_CP
}
void DPHY_Init_Flow_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing)
{
	// DPHY init flow start
	_rtd_part_outl(0x1800dc88, 4, 4, 0x1); //[DPHY init flow]  reg_p0_b_fifo_en
	_rtd_part_outl(0x1800dc88, 12, 12, 0x1); //[DPHY init flow]  reg_p0_g_fifo_en
	_rtd_part_outl(0x1800dc88, 20, 20, 0x1); //[DPHY init flow]  reg_p0_r_fifo_en
	_rtd_part_outl(0x1800dc88, 28, 28, 0x1); //[DPHY init flow]  reg_p0_ck_fifo_en
	_rtd_part_outl(0x1800dd6c, 1, 1, 0x1); //[DPHY init flow]  reg_p0_pow_save_10b18b_sel
	_rtd_part_outl(0x1800dd7c, 5, 0, 0x0e); //[DPHY init flow]  reg_p0_dig_debug_sel
	//DPHY fld flow start
	_rtd_part_outl(0x1800dcb8, 10, 3, 0x1e); //[DPHY fld flow]  reg_p0_b_divide_num
	_rtd_part_outl(0x1800dcbc, 11, 0, DPHY_Init_Flow_frl_1[frl_timing].b_lock_dn_limit_10); //[DPHY fld flow] reg_p0_b_lock_dn_limit
	_rtd_part_outl(0x1800dcbc, 27, 16, DPHY_Init_Flow_frl_1[frl_timing].b_lock_up_limit_11); //[DPHY fld flow] reg_p0_b_lock_up_limit
	_rtd_part_outl(0x1800dcc4, 19, 8, DPHY_Init_Flow_frl_1[frl_timing].b_coarse_lock_dn_limit_12); //[DPHY fld flow] reg_p0_b_coarse_lock_dn_limit
	_rtd_part_outl(0x1800dcc4, 31, 20, DPHY_Init_Flow_frl_1[frl_timing].b_coarse_lock_up_limit_13); //[DPHY fld flow] reg_p0_b_coarse_lock_up_limit
	_rtd_part_outl(0x1800dcc8, 1, 1, 0x1); //[DPHY fld flow]  reg_p0_b_fld_rstb
	_rtd_part_outl(0x1800dcd8, 10, 3, 0x1e); //[DPHY fld flow]  reg_p0_g_divide_num
	_rtd_part_outl(0x1800dcdc, 11, 0, DPHY_Init_Flow_frl_1[frl_timing].g_lock_dn_limit_16); //[DPHY fld flow] reg_p0_g_lock_dn_limit
	_rtd_part_outl(0x1800dcdc, 27, 16, DPHY_Init_Flow_frl_1[frl_timing].g_lock_up_limit_17); //[DPHY fld flow] reg_p0_g_lock_up_limit
	_rtd_part_outl(0x1800dce4, 19, 8, DPHY_Init_Flow_frl_1[frl_timing].g_coarse_lock_dn_limit_18); //[DPHY fld flow] reg_p0_g_coarse_lock_dn_limit
	_rtd_part_outl(0x1800dce4, 31, 20, DPHY_Init_Flow_frl_1[frl_timing].g_coarse_lock_up_limit_19); //[DPHY fld flow] reg_p0_g_coarse_lock_up_limit
	_rtd_part_outl(0x1800dce8, 1, 1, 0x1); //[DPHY fld flow]  reg_p0_g_fld_rstb
	_rtd_part_outl(0x1800dcf4, 10, 3, 0x1e); //[DPHY fld flow]  reg_p0_r_divide_num
	_rtd_part_outl(0x1800dcf8, 11, 0, DPHY_Init_Flow_frl_1[frl_timing].r_lock_dn_limit_22); //[DPHY fld flow] reg_p0_r_lock_dn_limit
	_rtd_part_outl(0x1800dcf8, 27, 16, DPHY_Init_Flow_frl_1[frl_timing].r_lock_up_limit_23); //[DPHY fld flow] reg_p0_r_lock_up_limit
	_rtd_part_outl(0x1800dd00, 19, 8, DPHY_Init_Flow_frl_1[frl_timing].r_coarse_lock_dn_limit_24); //[DPHY fld flow] reg_p0_r_coarse_lock_dn_limit
	_rtd_part_outl(0x1800dd00, 31, 20, DPHY_Init_Flow_frl_1[frl_timing].r_coarse_lock_up_limit_25); //[DPHY fld flow] reg_p0_r_coarse_lock_up_limit
	_rtd_part_outl(0x1800dd04, 1, 1, 0x1); //[DPHY fld flow]  reg_p0_r_fld_rstb
	_rtd_part_outl(0x1800dd10, 10, 3, 0x1e); //[DPHY fld flow]  reg_p0_ck_divide_num
	_rtd_part_outl(0x1800dd14, 11, 0, DPHY_Init_Flow_frl_1[frl_timing].ck_lock_dn_limit_28); //[DPHY fld flow] reg_p0_ck_lock_dn_limit
	_rtd_part_outl(0x1800dd14, 27, 16, DPHY_Init_Flow_frl_1[frl_timing].ck_lock_up_limit_29); //[DPHY fld flow] reg_p0_ck_lock_up_limit
	_rtd_part_outl(0x1800dd1c, 19, 8, DPHY_Init_Flow_frl_1[frl_timing].ck_coarse_lock_dn_limit_30); //[DPHY fld flow] reg_p0_ck_coarse_lock_dn_limit
	_rtd_part_outl(0x1800dd1c, 31, 20, DPHY_Init_Flow_frl_1[frl_timing].ck_coarse_lock_up_limit_31); //[DPHY fld flow] reg_p0_ck_coarse_lock_up_limit
	_rtd_part_outl(0x1800dd20, 1, 1, 0x1); //[DPHY fld flow]  reg_p0_ck_fld_rstb
}
void APHY_Init_Flow_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing)
{
	// APHY init flow start
	_rtd_part_outl(0x1800dd3c, 9, 9, 0x1); //B //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_rtd_part_outl(0x1800dd48, 9, 9, 0x1); //G //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_rtd_part_outl(0x1800dd54, 9, 9, 0x1); //R //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_rtd_part_outl(0x1800dd60, 9, 9, 0x1); //K //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_rtd_part_outl(0x1800da18, 1, 1, 0x0); //[APHY init flow]  REG_CK_LATCH Before PSM, latch the register value of PHY. 0->1: Latch
	_rtd_part_outl(0x1800da30, 6, 6, 0x0); //[APHY init flow]  CK-Lane FAST_SW_EN
	_rtd_part_outl(0x1800da30, 7, 7, 0x0); //[APHY init flow]  CK-Lane FAST_SW_DELAY_EN
	_rtd_part_outl(0x1800da38, 17, 17, 0x1); //[APHY init flow] 	CK-Lane BBPD_RSTB
	_rtd_part_outl(0x1800da40, 16, 16, 0x0); //[APHY init flow]  CK-Lane DCDR_RSTB
	_rtd_part_outl(0x1800da40, 17, 17, 0x0); //[APHY init flow]  CK-Lane PI_DIV rstb
	_rtd_part_outl(0x1800da40, 24, 24, 0x0); //[APHY init flow]  CK-Lane FKP_RSTB (FKP:Fast KP)
	_rtd_part_outl(0x1800da40, 28, 28, 0x1); //[APHY init flow]  CK-Lane FKT EN
	_rtd_part_outl(0x1800da44, 0, 0, 0x1); //[APHY init flow]  CK-Lane ACDR_RSTB_PFD
	_rtd_part_outl(0x1800da44, 1, 1, 0x1); //[APHY init flow]  CK-Lane ACDR_RSTB_CLK_FLD
	_rtd_part_outl(0x1800da44, 16, 16, 0x1); //[APHY init flow]  CK-Lane ACDR_RSTB_DIV_IQ
	_rtd_part_outl(0x1800da48, 24, 24, 0x1); //[APHY init flow]  CK-Lane RSTB_BBPD_KP_KI
	_rtd_part_outl(0x1800da4c, 11, 11, 0x1); //[APHY init flow]  CK-Lane RSTB_PRESCALER
	_rtd_part_outl(0x1800da4c, 12, 12, 0x1); //[APHY init flow]  CK-Lane RSTB_M_DIV
	_rtd_part_outl(0x1800da50, 6, 6, 0x0); //[APHY init flow]  	B-Lane FAST_SW_EN
	_rtd_part_outl(0x1800da50, 7, 7, 0x0); //[APHY init flow]  	B-Lane FAST_SW_DELAY_EN
	_rtd_part_outl(0x1800da58, 17, 17, 0x1); //[APHY init flow] 	B-Lane BBPD_RSTB
	_rtd_part_outl(0x1800da60, 16, 16, 0x0); //[APHY init flow]  B-Lane DCDR_RSTB
	_rtd_part_outl(0x1800da60, 17, 17, 0x0); //[APHY init flow]  B-Lane PI_DIV rstb
	_rtd_part_outl(0x1800da60, 24, 24, 0x0); //[APHY init flow]  B-Lane FKP_RSTB (FKP:Fast KP)
	_rtd_part_outl(0x1800da60, 28, 28, 0x1); //[APHY init flow]  B-Lane FKT EN
	_rtd_part_outl(0x1800da64, 0, 0, 0x1); //[APHY init flow]  B-Lane ACDR_RSTB_PFD
	_rtd_part_outl(0x1800da64, 1, 1, 0x1); //[APHY init flow]  B-Lane ACDR_RSTB_CLK_FLD
	_rtd_part_outl(0x1800da64, 16, 16, 0x1); //[APHY init flow]  B-Lane ACDR_RSTB_DIV_IQ
	_rtd_part_outl(0x1800da68, 24, 24, 0x1); //[APHY init flow]  B-Lane RSTB_BBPD_KP_KI
	_rtd_part_outl(0x1800da6c, 11, 11, 0x1); //[APHY init flow]  B-Lane RSTB_PRESCALER
	_rtd_part_outl(0x1800da6c, 12, 12, 0x1); //[APHY init flow]  B-Lane RSTB_M_DIV
	_rtd_part_outl(0x1800da70, 6, 6, 0x0); //[APHY init flow]  	G-Lane FAST_SW_EN
	_rtd_part_outl(0x1800da70, 7, 7, 0x0); //[APHY init flow]  	G-Lane FAST_SW_DELAY_EN
	_rtd_part_outl(0x1800da78, 17, 17, 0x1); //[APHY init flow] 	G-Lane BBPD_RSTB
	_rtd_part_outl(0x1800da80, 16, 16, 0x0); //[APHY init flow]  G-Lane DCDR_RSTB
	_rtd_part_outl(0x1800da80, 17, 17, 0x0); //[APHY init flow]  G-Lane PI_DIV rstb
	_rtd_part_outl(0x1800da80, 24, 24, 0x0); //[APHY init flow]  G-Lane FKP_RSTB (FKP:Fast KP)
	_rtd_part_outl(0x1800da80, 28, 28, 0x1); //[APHY init flow]  G-Lane FKT EN
	_rtd_part_outl(0x1800da84, 0, 0, 0x1); //[APHY init flow]  G-Lane ACDR_RSTB_PFD
	_rtd_part_outl(0x1800da84, 1, 1, 0x1); //[APHY init flow]  G-Lane ACDR_RSTB_CLK_FLD
	_rtd_part_outl(0x1800da84, 16, 16, 0x1); //[APHY init flow]  G-Lane ACDR_RSTB_DIV_IQ
	_rtd_part_outl(0x1800da88, 24, 24, 0x1); //[APHY init flow]  G-Lane RSTB_BBPD_KP_KI
	_rtd_part_outl(0x1800da8c, 11, 11, 0x1); //[APHY init flow]  G-Lane RSTB_PRESCALER
	_rtd_part_outl(0x1800da8c, 12, 12, 0x1); //[APHY init flow]  G-Lane RSTB_M_DIV
	_rtd_part_outl(0x1800da90, 6, 6, 0x0); //[APHY init flow]  	R-Lane FAST_SW_EN
	_rtd_part_outl(0x1800da90, 7, 7, 0x0); //[APHY init flow]  	R-Lane FAST_SW_DELAY_EN
	_rtd_part_outl(0x1800da98, 17, 17, 0x1); //[APHY init flow] 	R-Lane BBPD_RSTB
	_rtd_part_outl(0x1800daa0, 16, 16, 0x0); //[APHY init flow]  R-Lane DCDR_RSTB
	_rtd_part_outl(0x1800daa0, 17, 17, 0x0); //[APHY init flow]  R-Lane PI_DIV rstb
	_rtd_part_outl(0x1800daa0, 24, 24, 0x0); //[APHY init flow]  R-Lane FKP_RSTB (FKP:Fast KP)
	_rtd_part_outl(0x1800daa0, 28, 28, 0x1); //[APHY init flow]  R-Lane FKT EN
	_rtd_part_outl(0x1800daa4, 0, 0, 0x1); //[APHY init flow]  R-Lane ACDR_RSTB_PFD
	_rtd_part_outl(0x1800daa4, 1, 1, 0x1); //[APHY init flow]  R-Lane ACDR_RSTB_CLK_FLD
	_rtd_part_outl(0x1800daa4, 16, 16, 0x1); //[APHY init flow]  R-Lane ACDR_RSTB_DIV_IQ
	_rtd_part_outl(0x1800daa8, 24, 24, 0x1); //[APHY init flow]  R-Lane RSTB_BBPD_KP_KI
	_rtd_part_outl(0x1800daac, 11, 11, 0x1); //[APHY init flow]  R-Lane RSTB_PRESCALER
	_rtd_part_outl(0x1800daac, 12, 12, 0x1); //[APHY init flow]  R-Lane RSTB_M_DIV
	_rtd_part_outl(0x1800dd3c, 9, 9, 0x0); //B //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_rtd_part_outl(0x1800dd48, 9, 9, 0x0); //G //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_rtd_part_outl(0x1800dd54, 9, 9, 0x0); //R //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	_rtd_part_outl(0x1800dd60, 9, 9, 0x0); //K //reg_p0_b_acdr_pll_config_2_1 [9] TIE VCTRL ...0
	//APHY acdr flow start
	_rtd_part_outl(0x1800dafc, 0, 0, 0x1); //[APHY acdr flow]  P0_ACDR_B_ACDR_EN_AFN
	_rtd_part_outl(0x1800dafc, 1, 1, 0x1); //[APHY acdr flow]  P0_ACDR_G_ACDR_EN_AFN
	_rtd_part_outl(0x1800dafc, 2, 2, 0x1); //[APHY acdr flow]  P0_ACDR_R_ACDR_EN_AFN
	_rtd_part_outl(0x1800dafc, 3, 3, 0x1); //[APHY acdr flow]  P0_ACDR_CK_ACDR_EN_AFN
	_rtd_part_outl(0x1800dafc, 4, 4, 0x1); //[APHY acdr flow]  P0_B_DEMUX_RSTB
	_rtd_part_outl(0x1800dafc, 5, 5, 0x1); //[APHY acdr flow]  P0_G_DEMUX_RSTB
	_rtd_part_outl(0x1800dafc, 6, 6, 0x1); //[APHY acdr flow]  P0_R_DEMUX_RSTB
	_rtd_part_outl(0x1800dafc, 7, 7, 0x1); //[APHY acdr flow]  P0_CK_DEMUX_RSTB
}

void ACDR_settings_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing)
{
	//#====================================== CMU RESET
	_rtd_part_outl(0x1800dafc, 7, 4, 0x0);
	_rtd_part_outl(0x1800dafc, 7, 4, 0xf);
	//#====================================== phase I
	//#====================================== FLD reset
	_rtd_part_outl(0x1800dcc8, 0, 0, 0x0);
	_rtd_part_outl(0x1800dce8, 0, 0, 0x0);
	_rtd_part_outl(0x1800dd04, 0, 0, 0x0);
	_rtd_part_outl(0x1800dd20, 0, 0, 0x0);
	//#5000
	//#===================================== 30000 ns pass
	//#===================================== phase II, PLL to ACDR
}

void ACDR_settings_frl_2_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing)
{
	//#15000
	_rtd_part_outl(0x1800dafc, 3, 0, 0xf);
	//#15000
	_rtd_part_outl(0x1800da38, 17, 17, 0); //CK-Lane BBPD_RSTB
	_rtd_part_outl(0x1800da58, 17, 17, 0); //B-Lane BBPD_RSTB
	_rtd_part_outl(0x1800da78, 17, 17, 0); //G-Lane BBPD_RSTB
	_rtd_part_outl(0x1800da98, 17, 17, 0); //R-Lane BBPD_RSTB
	_rtd_part_outl(0x1800da38, 17, 17, 1); //CK-Lane BBPD_RSTB
	_rtd_part_outl(0x1800da58, 17, 17, 1); //B-Lane BBPD_RSTB
	_rtd_part_outl(0x1800da78, 17, 17, 1); //G-Lane BBPD_RSTB
	_rtd_part_outl(0x1800da98, 17, 17, 1); //R-Lane BBPD_RSTB
	_rtd_part_outl(0x1800dc88, 27, 27, 1); //DFE data enable
	_rtd_part_outl(0x1800dc88, 19, 19, 1); //DFE data enable
	_rtd_part_outl(0x1800dc88, 11, 11, 1); //DFE data enable
	_rtd_part_outl(0x1800dc88, 3, 3, 1); //DFE data enable
}

void Koffset_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing)
{
	_rtd_part_outl(0x1800dd04, 0, 0, 0); //R finetunstart off
	_rtd_part_outl(0x1800dce8, 0, 0, 0); //G finetunstart off
	_rtd_part_outl(0x1800dcc8, 0, 0, 0); //B finetunstart off
	_rtd_part_outl(0x1800dd20, 0, 0, 0); //CK finetunstart off
	//========================================= STEP1 Data_Even KOFF =================================================
	_rtd_part_outl(0x1800da58, 28, 28, 1); //#B  DFE_TAP0_ICOM_EN=1
	_rtd_part_outl(0x1800da5c, 6, 3, 0); //#B  DFE_TAP_EN[4:1] = 0
	_rtd_part_outl(0x1800da38, 28, 28, 1); //#CK DFE_TAP0_ICOM_EN=1
	_rtd_part_outl(0x1800da3c, 6, 3, 0); //#CK DFE_TAP_EN[4:1] = 0
	_rtd_part_outl(0x1800da78, 28, 28, 1); //#G  DFE_TAP0_ICOM_EN=1
	_rtd_part_outl(0x1800da7c, 6, 3, 0); //#G  DFE_TAP_EN[4:1] = 0
	_rtd_part_outl(0x1800da98, 28, 28, 1); //#R  DFE_TAP0_ICOM_EN=1
	_rtd_part_outl(0x1800da9c, 6, 3, 0); //#R  DFE_TAP_EN[4:1] = 0
	_rtd_part_outl(0x1800dc44, 31, 0, 0x0282cf00); //B_KOFF rstn,  //[26:20] timout = 0101000, [19:14] divisor = 001000, [11:8] delay count = 11
	_rtd_part_outl(0x1800dc54, 31, 0, 0x0282cf00); //G_KOFF rstn
	_rtd_part_outl(0x1800dc64, 31, 0, 0x0282cf00); //R_KOFF rstn
	_rtd_part_outl(0x1800dc74, 31, 0, 0x0282cf00); //CK_KOFF rstn
	_rtd_part_outl(0x1800dc44, 0, 0, 1); //Release B_KOFF rstn
	_rtd_part_outl(0x1800dc54, 0, 0, 1); //Release G_KOFF rstn
	_rtd_part_outl(0x1800dc64, 0, 0, 1); //Release R_KOFF rstn
	_rtd_part_outl(0x1800dc74, 0, 0, 1); //Release CK_KOFF rstn
	//===BLANE KOFF
	_rtd_part_outl(0x1800dc4c, 28, 28, 0); //manaul_do=0, 0:Manual; 1:Auto
	_rtd_part_outl(0x1800dc4c, 18, 18, 0); //manaul_de=0, 0:Manual; 1:Auto
	_rtd_part_outl(0x1800dc50, 28, 28, 0); //manaul_eq=0, 0:Manual; 1:Auto
	_rtd_part_outl(0x1800dc50, 18, 18, 0); //manaul_eo=0, 0:Manual; 1:Auto
	_rtd_part_outl(0x1800dc50, 8, 8, 0); //manaul_ee=0, 0:Manual; 1:Auto
	_rtd_part_outl(0x1800da50, 0, 0, 1); //INOFF_EN =0 ->1
	_rtd_part_outl(0x1800da58, 0, 0, 0); //REG_B_9[0] POW_CMFB_LEQ 1->0
	_rtd_part_outl(0x1800da54, 23, 22, 0); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  1->0
	_rtd_part_outl(0x1800da54, 24, 24, 0); //REG_B_8[0] POW_LEQ:  1->0
	//B-Lane z0_ok
	_rtd_part_outl(0x1800dc50, 19, 19, 1); //[19]z0_ok_eo=1
	_rtd_part_outl(0x1800dc50, 9, 9, 1); //[9] z0_ok_ee=1
	_rtd_part_outl(0x1800dc4c, 29, 29, 1); //[29]z0_ok_do=1
	_rtd_part_outl(0x1800dc4c, 19, 19, 1); //[19]z0_ok_de=1
	_rtd_part_outl(0x1800dc4c, 21, 21, 1); //[20]offset_pc_do=1
	_rtd_part_outl(0x1800dc4c, 11, 11, 1); //[20]offset_pc_de=1
	_rtd_part_outl(0x1800dc50, 1, 1, 1); //[20]offset_pc_ee=1
	_rtd_part_outl(0x1800dc50, 11, 11, 1); //[20]offset_pc_eo=1
	_rtd_part_outl(0x1800dc50, 21, 21, 1); //[21]offfset_pc_eq=1
	//Enable B-Lane DCVS KOFF
	_rtd_part_outl(0x1800dc4c, 10, 10, 1); //[10]offset_en_de=1
	_rtd_part_outl(0x1800dc4c, 20, 20, 1); //[20]offset_en_do=1
	_rtd_part_outl(0x1800dc50, 0, 0, 1); //[20]offset_en_ee=1
	_rtd_part_outl(0x1800dc50, 10, 10, 1); //[20]offset_en_eo=1
	//Check Offset cal. OK Check
	_rtd_part_outl(0x1800dd7c, 5, 0, 0x8); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_rtd_part_outl(0x1800dc44, 7, 4, 0x9); //reg_p0_b_offset_coef_sel
	HDMI_PHY_INFO("DE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 27, 27));
	HDMI_PHY_INFO("DO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 26, 26));
	HDMI_PHY_INFO("EE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 25, 25));
	HDMI_PHY_INFO("EOOffset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 24, 24));	
	HDMI_PHY_INFO("DATA OFFSET OK = %d\n",_rtd_part_inl(0x1800dc44, 28, 28));
	//Disable B-Lane DCVS KOFF
	_rtd_part_outl(0x1800dc4c, 10, 10, 0); //[10]offset_en_de=1->0
	_rtd_part_outl(0x1800dc4c, 20, 20, 0); //[20]offset_en_do=1->0
	_rtd_part_outl(0x1800dc50, 0, 0, 0); //[20]offset_en_ee=1->0
	_rtd_part_outl(0x1800dc50, 10, 10, 0); //[20]offset_en_eo=1->0
	//B-Lane DCVS Offset Result
	_rtd_part_outl(0x1800dc44, 7, 4, 0); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Data_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	_rtd_part_outl(0x1800dc44, 7, 4, 1); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Data_Odd_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	_rtd_part_outl(0x1800dc44, 7, 4, 2); //reg_p0_b_offset_coef_sel[4:0]//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	_rtd_part_outl(0x1800dc44, 7, 4, 3); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	//===G LANE KOFF
	_rtd_part_outl(0x1800dc5c, 28, 28, 0); //manaul_do=0
	_rtd_part_outl(0x1800dc5c, 18, 18, 0); //manaul_de=0
	_rtd_part_outl(0x1800dc60, 28, 28, 0); //manaul_eq=0
	_rtd_part_outl(0x1800dc60, 18, 18, 0); //manaul_eo=0
	_rtd_part_outl(0x1800dc60, 8, 8, 0); //manaul_ee=0
	_rtd_part_outl(0x1800da70, 0, 0, 1); //INOFF_EN =0 ->1
	_rtd_part_outl(0x1800da78, 0, 0, 0); //REG_B_9[0] POW_CMFB_LEQ 1->0
	_rtd_part_outl(0x1800da74, 23, 22, 0); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  1->0
	_rtd_part_outl(0x1800da74, 24, 24, 0); //REG_B_8[0] POW_LEQ:  1->0
	//G-Lane z0_ok
	_rtd_part_outl(0x1800dc60, 19, 19, 1); //[19]z0_ok_eo=1
	_rtd_part_outl(0x1800dc60, 9, 9, 1); //[9] z0_ok_ee=1
	_rtd_part_outl(0x1800dc5c, 29, 29, 1); //[29]z0_ok_do=1
	_rtd_part_outl(0x1800dc5c, 19, 19, 1); //[19]z0_ok_de=1
	_rtd_part_outl(0x1800dc5c, 21, 21, 1); //[20]offset_pc_do=1
	_rtd_part_outl(0x1800dc5c, 11, 11, 1); //[20]offset_pc_de=1
	_rtd_part_outl(0x1800dc60, 1, 1, 1); //[20]offset_pc_ee=1
	_rtd_part_outl(0x1800dc60, 11, 11, 1); //[20]offset_pc_eo=1
	_rtd_part_outl(0x1800dc60, 21, 21, 1); //[21]offfset_pc_eq=1
	//Enable G-Lane DCVS KOFF
	_rtd_part_outl(0x1800dc5c, 10, 10, 1); //[10]offset_en_de=1
	_rtd_part_outl(0x1800dc5c, 20, 20, 1); //[20]offset_en_do=1
	_rtd_part_outl(0x1800dc60, 0, 0, 1); //[20]offset_en_ee=1
	_rtd_part_outl(0x1800dc60, 10, 10, 1); //[20]offset_en_eo=1
	//G-Lane DCVS Offset cal. OK Check
	_rtd_part_outl(0x1800dd7c, 5, 0, 0xA); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_rtd_part_outl(0x1800dc54, 7, 4, 0x9); //reg_p0_g_offset_coef_sel
	HDMI_PHY_INFO("DE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 27, 27));
	HDMI_PHY_INFO("DO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 26, 26));
	HDMI_PHY_INFO("EE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 25, 25));
	HDMI_PHY_INFO("EO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 24, 24));
	HDMI_PHY_INFO("DATA OFFSET OK = %d\n",_rtd_part_inl(0x1800dc54, 28, 28));
	//Disable G-Lane DCVS KOFF
	_rtd_part_outl(0x1800dc5c, 10, 10, 0); //[10]offset_en_de=0
	_rtd_part_outl(0x1800dc5c, 20, 20, 0); //[20]offset_en_do=0
	_rtd_part_outl(0x1800dc60, 0, 0, 0); //[20]offset_en_ee=0
	_rtd_part_outl(0x1800dc60, 10, 10, 0); //[20]offset_en_eo=0
	//G-Lane DCVS Offset Result
	_rtd_part_outl(0x1800dc54, 7, 4, 0); //reg_p0_g_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Data_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	_rtd_part_outl(0x1800dc54, 7, 4, 1); //reg_p0_g_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Data_Odd_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	_rtd_part_outl(0x1800dc54, 7, 4, 2); //reg_p0_g_offset_coef_sel[4:0]//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	_rtd_part_outl(0x1800dc54, 7, 4, 3); //reg_p0_g_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	//===R LANE KOFF
	_rtd_part_outl(0x1800dc6c, 28, 28, 0); //manaul_do=0
	_rtd_part_outl(0x1800dc6c, 18, 18, 0); //manaul_de=0
	_rtd_part_outl(0x1800dc70, 28, 28, 0); //manaul_eq=0
	_rtd_part_outl(0x1800dc70, 18, 18, 0); //manaul_eo=0
	_rtd_part_outl(0x1800dc70, 8, 8, 0); //manaul_ee=0
	_rtd_part_outl(0x1800da90, 0, 0, 1); //INOFF_EN =0 ->1
	_rtd_part_outl(0x1800da98, 0, 0, 0); //REG_B_9[0] POW_CMFB_LEQ 1->0
	_rtd_part_outl(0x1800da94, 23, 22, 0); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  1->0
	_rtd_part_outl(0x1800da94, 24, 24, 0); //REG_B_8[0] POW_LEQ:  1->0
	//R-Lane z0_ok
	_rtd_part_outl(0x1800dc70, 19, 19, 1); //[19]z0_ok_eo=1
	_rtd_part_outl(0x1800dc70, 9, 9, 1); //[9] z0_ok_ee=1
	_rtd_part_outl(0x1800dc6c, 29, 29, 1); //[29]z0_ok_do=1
	_rtd_part_outl(0x1800dc6c, 19, 19, 1); //[19]z0_ok_de=1
	_rtd_part_outl(0x1800dc6c, 21, 21, 1); //[20]offset_pc_do=1
	_rtd_part_outl(0x1800dc6c, 11, 11, 1); //[20]offset_pc_de=1
	_rtd_part_outl(0x1800dc70, 1, 1, 1); //[20]offset_pc_ee=1
	_rtd_part_outl(0x1800dc70, 11, 11, 1); //[20]offset_pc_eo=1
	_rtd_part_outl(0x1800dc70, 21, 21, 1); //[21]offfset_pc_eq=1
	//Enable R-Lane DCVS KOFF
	_rtd_part_outl(0x1800dc6c, 10, 10, 1); //[10]offset_en_de=1
	_rtd_part_outl(0x1800dc6c, 20, 20, 1); //[20]offset_en_do=1
	_rtd_part_outl(0x1800dc70, 0, 0, 1); //[20]offset_en_ee=1
	_rtd_part_outl(0x1800dc70, 10, 10, 1); //[20]offset_en_eo=1
	//R-Lane DCVS Offset cal. OK Check
	_rtd_part_outl(0x1800dd7c, 5, 0, 0xC); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_rtd_part_outl(0x1800dc64, 7, 4, 0x9); //reg_p0_r_offset_coef_sel
	HDMI_PHY_INFO("DE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 27, 27));
	HDMI_PHY_INFO("DO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 26, 26));
	HDMI_PHY_INFO("EE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 25, 25));
	HDMI_PHY_INFO("EO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 24, 24));
	HDMI_PHY_INFO("DATA OFFSET OK = %d\n",_rtd_part_inl(0x1800dc64, 28, 28));
	//Disable R-Lane DCVS KOFF
	_rtd_part_outl(0x1800dc6c, 10, 10, 0); //[10]offset_en_de=0
	_rtd_part_outl(0x1800dc6c, 20, 20, 0); //[20]offset_en_do=0
	_rtd_part_outl(0x1800dc70, 0, 0, 0); //[20]offset_en_ee=0
	_rtd_part_outl(0x1800dc70, 10, 10, 0); //[20]offset_en_eo=0
	//R-Lane LEQ & DCVS Offset Result
	_rtd_part_outl(0x1800dc64, 7, 4, 0); //reg_p0_r_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Data_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	_rtd_part_outl(0x1800dc64, 7, 4, 1); //reg_p0_r_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Data_Odd_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	_rtd_part_outl(0x1800dc64, 7, 4, 2); //reg_p0_r_offset_coef_sel[4:0]//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	_rtd_part_outl(0x1800dc64, 7, 4, 3); //reg_p0_r_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	//===CK LANE KOFF
	_rtd_part_outl(0x1800dc7c, 28, 28, 0); //manaul_do=0
	_rtd_part_outl(0x1800dc7c, 18, 18, 0); //manaul_de=0
	_rtd_part_outl(0x1800dc80, 28, 28, 0); //manaul_eq=0
	_rtd_part_outl(0x1800dc80, 18, 18, 0); //manaul_eo=0
	_rtd_part_outl(0x1800dc80, 8, 8, 0); //manaul_ee=0
	_rtd_part_outl(0x1800da30, 0, 0, 1); //INOFF_EN =0 ->1
	_rtd_part_outl(0x1800da38, 0, 0, 0); //REG_B_9[0] POW_CMFB_LEQ 1->0
	_rtd_part_outl(0x1800da34, 23, 22, 0); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  1->0
	_rtd_part_outl(0x1800da34, 24, 24, 0); //REG_B_8[0] POW_LEQ:  1->0
	//CK-Lane z0_ok
	_rtd_part_outl(0x1800dc80, 19, 19, 1); //[19]z0_ok_eo=1
	_rtd_part_outl(0x1800dc80, 9, 9, 1); //[9] z0_ok_ee=1
	_rtd_part_outl(0x1800dc7c, 29, 29, 1); //[29]z0_ok_do=1
	_rtd_part_outl(0x1800dc7c, 19, 19, 1); //[19]z0_ok_de=1
	_rtd_part_outl(0x1800dc7c, 21, 21, 1); //[20]offset_pc_do=1
	_rtd_part_outl(0x1800dc7c, 11, 11, 1); //[20]offset_pc_de=1
	_rtd_part_outl(0x1800dc80, 1, 1, 1); //[20]offset_pc_ee=1
	_rtd_part_outl(0x1800dc80, 11, 11, 1); //[20]offset_pc_eo=1
	_rtd_part_outl(0x1800dc80, 21, 21, 1); //[21]offfset_pc_eq=1
	//Disable CK-Lane DCVS KOFF
	_rtd_part_outl(0x1800dc7c, 10, 10, 1); //[10]offset_en_de=1
	_rtd_part_outl(0x1800dc7c, 20, 20, 1); //[20]offset_en_do=1
	_rtd_part_outl(0x1800dc80, 0, 0, 1); //[20]offset_en_ee=1
	_rtd_part_outl(0x1800dc80, 10, 10, 1); //[20]offset_en_eo=1
	//CK-Lane DCVS Offset cal. OK Check
	_rtd_part_outl(0x1800dd7c, 5, 0, 0xE); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_rtd_part_outl(0x1800dc74, 7, 4, 0x9); //reg_p0_ck_offset_coef_sel
	HDMI_PHY_INFO("DE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 27, 27));
	HDMI_PHY_INFO("DO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 26, 26));
	HDMI_PHY_INFO("EE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 25, 25));
	HDMI_PHY_INFO("EO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 24, 24));
	HDMI_PHY_INFO("DATA OFFSET OK = %d\n",_rtd_part_inl(0x1800dc74, 28, 28));
	//Disable CK-Lane DCVS KOFF
	_rtd_part_outl(0x1800dc7c, 10, 10, 0); //[10]offset_en_de=0
	_rtd_part_outl(0x1800dc7c, 20, 20, 0); //[20]offset_en_do=0
	_rtd_part_outl(0x1800dc80, 0, 0, 0); //[20]offset_en_ee=0
	_rtd_part_outl(0x1800dc80, 10, 10, 0); //[20]offset_en_eo=0
	//CK-Lane LEQ & DCVS Offset Result
	_rtd_part_outl(0x1800dc74, 7, 4, 0); //reg_p0_ck_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Data_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 28, 24));
	_rtd_part_outl(0x1800dc74, 7, 4, 1); //reg_p0_ck_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Data_Odd_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 28, 24));
	_rtd_part_outl(0x1800dc74, 7, 4, 2); //reg_p0_ck_offset_coef_sel[4:0]//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 28, 24));
	_rtd_part_outl(0x1800dc74, 7, 4, 3); //reg_p0_ck_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 28, 24));
	//========================================= STEP2 LEQ KOFF =================================================
	//===B LANE KOFF LEQ
	_rtd_part_outl(0x1800da54, 23, 22, Koffset_frl_1[frl_timing].B_7_212); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  0->1
	_rtd_part_outl(0x1800da54, 24, 24, 1); //REG_B_8[0] POW_LEQ:  0->1
	_rtd_part_outl(0x1800da58, 0, 0, 1); //REG_B_9[0] POW_CMFB_LEQ 0->1
	_rtd_part_outl(0x1800dc50, 19, 19, 0); //[19]z0_ok_eo=0
	_rtd_part_outl(0x1800dc50, 9, 9, 0); //[9] z0_ok_ee=0
	_rtd_part_outl(0x1800dc4c, 29, 29, 0); //[19]z0_ok_do=0
	_rtd_part_outl(0x1800dc50, 20, 20, 1); //[21]offset_en_eq=0->1
	HDMI_PHY_INFO("p0_b_offset_eq_ok_ro	EQ OFFSET OK = %d\n",_rtd_part_inl(0x1800dc44, 30, 30));
	_rtd_part_outl(0x1800dc50, 20, 20, 0); //[20]offset_en_eq=0
	//B-Lane LEQ Offset Result
	_rtd_part_outl(0x1800dc44, 7, 4, 8); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("EQ_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	//===G LANE KOFF LEQ
	_rtd_part_outl(0x1800da74, 23, 22, Koffset_frl_1[frl_timing].G_7_225); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  0->1
	_rtd_part_outl(0x1800da74, 24, 24, 1); //REG_B_8[0] POW_LEQ:  0->1
	_rtd_part_outl(0x1800da78, 0, 0, 1); //REG_B_9[0] POW_CMFB_LEQ 0->1
	_rtd_part_outl(0x1800dc60, 19, 19, 0); //[19]z0_ok_eo=0
	_rtd_part_outl(0x1800dc60, 9, 9, 0); //[9] z0_ok_ee=0
	_rtd_part_outl(0x1800dc5c, 29, 29, 0); //[19]z0_ok_do=0
	_rtd_part_outl(0x1800dc60, 20, 20, 1); //[21]offset_en_eq=0->1
	HDMI_PHY_INFO("p0_g_offset_eq_ok_ro EQ OFFSET OK = %d\n",_rtd_part_inl(0x1800dc54, 30, 30));
	_rtd_part_outl(0x1800dc60, 20, 20, 0); //[20]offset_en_eq=0
	//G-Lane LEQ Offset Result
	_rtd_part_outl(0x1800dc54, 7, 4, 8); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("EQ_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	//===R LANE KOFF LEQ
	_rtd_part_outl(0x1800da94, 23, 22, Koffset_frl_1[frl_timing].R_7_238); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  0->1
	_rtd_part_outl(0x1800da94, 24, 24, 1); //REG_B_8[0] POW_LEQ:  0->1
	_rtd_part_outl(0x1800da98, 0, 0, 1); //REG_B_9[0] POW_CMFB_LEQ 0->1
	_rtd_part_outl(0x1800dc70, 19, 19, 0); //[19]z0_ok_eo=0
	_rtd_part_outl(0x1800dc70, 9, 9, 0); //[9] z0_ok_ee=0
	_rtd_part_outl(0x1800dc6c, 29, 29, 0); //[19]z0_ok_do=0
	_rtd_part_outl(0x1800dc70, 20, 20, 1); //[21]offset_en_eq=0->1
	HDMI_PHY_INFO("p0_r_offset_eq_ok_ro EQ OFFSET OK = %d\n",_rtd_part_inl(0x1800dc64, 30, 30));
	_rtd_part_outl(0x1800dc70, 20, 20, 0); //[20]offset_en_eq=0
	//R-Lane LEQ Offset Result
	_rtd_part_outl(0x1800dc64, 7, 4, 8); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("EQ_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	//===CK LANE KOFF LEQ
	_rtd_part_outl(0x1800da34, 23, 22, Koffset_frl_1[frl_timing].CK_7_251); //REG_B_7[7:6] POW_LEQ_KOFF, POW_NC:  0->1
	_rtd_part_outl(0x1800da34, 24, 24, 1); //REG_B_8[0] POW_LEQ:  0->1
	_rtd_part_outl(0x1800da38, 0, 0, 1); //REG_B_9[0] POW_CMFB_LEQ 0->1
	_rtd_part_outl(0x1800dc80, 19, 19, 0); //[19]z0_ok_eo=0
	_rtd_part_outl(0x1800dc80, 9, 9, 0); //[9] z0_ok_ee=0
	_rtd_part_outl(0x1800dc7c, 29, 29, 0); //[19]z0_ok_do=0
	_rtd_part_outl(0x1800dc80, 20, 20, 1); //[21]offset_en_eq=0->1
	HDMI_PHY_INFO("p0_ck_offset_eq_ok_ro EQ OFFSET OK = %d\n",_rtd_part_inl(0x1800dc74, 30, 30));
	_rtd_part_outl(0x1800dc80, 20, 20, 0); //[20]offset_en_eq=0
	//CK-Lane LEQ Offset Result
	_rtd_part_outl(0x1800dc74, 7, 4, 8); //reg_p0_b_offset_coef_sel[4:0]	//0:Data_Even, 1:Data_Odd, 2:Edge_Even, 3:Edge_Odd, 8:EQ
	HDMI_PHY_INFO("EQ_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 28, 24));
	//========================================= STEP3 All DCVS KOFF ==
	//B-LANE
	_rtd_part_outl(0x1800dc4c, 10, 10, 1); //[10]offset_en_de=1
	_rtd_part_outl(0x1800dc4c, 20, 20, 1); //[20]offset_en_do=1
	_rtd_part_outl(0x1800dc50, 0, 0, 1); //[20]offset_en_ee=1
	_rtd_part_outl(0x1800dc50, 10, 10, 1); //[20]offset_en_eo=1
	ScalerTimer_DelayXms(5);
	//Check Offset cal. OK Check
	_rtd_part_outl(0x1800dd7c, 5, 0, 0x8); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_rtd_part_outl(0x1800dc44, 7, 4, 0x9); //reg_p0_b_offset_coef_sel
	HDMI_PHY_INFO("DE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 27, 27));
	HDMI_PHY_INFO("DO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 26, 26));
	HDMI_PHY_INFO("EE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 25, 25));
	HDMI_PHY_INFO("EOOffset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 24, 24));
	HDMI_PHY_INFO("DATA OFFSET OK = %d\n",_rtd_part_inl(0x1800dc44, 28, 28));
	//Disable B-Lane DCVS KOFF
	_rtd_part_outl(0x1800dc4c, 10, 10, 0); //[10]offset_en_de=1->0
	_rtd_part_outl(0x1800dc4c, 20, 20, 0); //[20]offset_en_do=1->0
	_rtd_part_outl(0x1800dc50, 0, 0, 0); //[20]offset_en_ee=1->0
	_rtd_part_outl(0x1800dc50, 10, 10, 0); //[20]offset_en_eo=1->0
	//===G LANE KOFF Data/Edge
	_rtd_part_outl(0x1800dc5c, 10, 10, 1); //[10]offset_en_de=1
	_rtd_part_outl(0x1800dc5c, 20, 20, 1); //[20]offset_en_do=1
	_rtd_part_outl(0x1800dc60, 0, 0, 1); //[20]offset_en_ee=1
	_rtd_part_outl(0x1800dc60, 10, 10, 1); //[20]offset_en_eo=1
	ScalerTimer_DelayXms(5);
	//G-Lane DCVS Offset cal. OK Check
	_rtd_part_outl(0x1800dd7c, 5, 0, 0xA); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_rtd_part_outl(0x1800dc54, 7, 4, 0x9); //reg_p0_g_offset_coef_sel
	HDMI_PHY_INFO("DE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 27, 27));
	HDMI_PHY_INFO("DO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 26, 26));
	HDMI_PHY_INFO("EE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 25, 25));
	HDMI_PHY_INFO("EO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 24, 24));
	HDMI_PHY_INFO("DATA OFFSET OK = %d\n",_rtd_part_inl(0x1800dc54, 28, 28));
	//G-LANE
	_rtd_part_outl(0x1800dc5c, 10, 10, 0); //[10]offset_en_de=0
	_rtd_part_outl(0x1800dc5c, 20, 20, 0); //[20]offset_en_do=0
	_rtd_part_outl(0x1800dc60, 0, 0, 0); //[20]offset_en_ee=0
	_rtd_part_outl(0x1800dc60, 10, 10, 0); //[20]offset_en_eo=0
	//===R LANE KOFF Data/Edge
	_rtd_part_outl(0x1800dc6c, 10, 10, 1); //[10]offset_en_de=1
	_rtd_part_outl(0x1800dc6c, 20, 20, 1); //[20]offset_en_do=1
	_rtd_part_outl(0x1800dc70, 0, 0, 1); //[20]offset_en_ee=1
	_rtd_part_outl(0x1800dc70, 10, 10, 1); //[20]offset_en_eo=1
	ScalerTimer_DelayXms(5);
	//R-Lane DCVS Offset cal. OK Check
	_rtd_part_outl(0x1800dd7c, 5, 0, 0xC); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_rtd_part_outl(0x1800dc64, 7, 4, 0x9); //reg_p0_r_offset_coef_sel
	HDMI_PHY_INFO("DE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 27, 27));
	HDMI_PHY_INFO("DO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 26, 26));
	HDMI_PHY_INFO("EE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 25, 25));
	HDMI_PHY_INFO("EO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 24, 24));
	HDMI_PHY_INFO("DATA OFFSET OK = %d\n",_rtd_part_inl(0x1800dc64, 28, 28));
	//Disable R-Lane DCVS KOFF
	_rtd_part_outl(0x1800dc6c, 10, 10, 0); //[10]offset_en_de=0
	_rtd_part_outl(0x1800dc6c, 20, 20, 0); //[20]offset_en_do=0
	_rtd_part_outl(0x1800dc70, 0, 0, 0); //[20]offset_en_ee=0
	_rtd_part_outl(0x1800dc70, 10, 10, 0); //[20]offset_en_eo=0
	//===CK LANE KOFF Data/Edge
	_rtd_part_outl(0x1800dc7c, 10, 10, 1); //[10]offset_en_de=1
	_rtd_part_outl(0x1800dc7c, 20, 20, 1); //[20]offset_en_do=1
	_rtd_part_outl(0x1800dc80, 0, 0, 1); //[20]offset_en_ee=1
	_rtd_part_outl(0x1800dc80, 10, 10, 1); //[20]offset_en_eo=1
	ScalerTimer_DelayXms(5);
	//CK-Lane DCVS Offset cal. OK Check
	_rtd_part_outl(0x1800dd7c, 5, 0, 0xE); //reg_p0_dig_debug_sel; Debug port0 signal selection
	_rtd_part_outl(0x1800dc74, 7, 4, 0x9); //reg_p0_ck_offset_coef_sel
	HDMI_PHY_INFO("DE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 27, 27));
	HDMI_PHY_INFO("DO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 26, 26));
	HDMI_PHY_INFO("EE Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 25, 25));
	HDMI_PHY_INFO("EO Offset cal. OK Check = %d\n",_rtd_part_inl(0x1800dd84, 24, 24));
	HDMI_PHY_INFO("DATA OFFSET OK = %d\n",_rtd_part_inl(0x1800dc74, 28, 28));
	//Disable CK-Lane DCVS KOFF
	_rtd_part_outl(0x1800dc7c, 10, 10, 0); //[10]offset_en_de=0
	_rtd_part_outl(0x1800dc7c, 20, 20, 0); //[20]offset_en_do=0
	_rtd_part_outl(0x1800dc80, 0, 0, 0); //[20]offset_en_ee=0
	_rtd_part_outl(0x1800dc80, 10, 10, 0); //[20]offset_en_eo=0
	//B-Lane LEQ & DCVS Offset Result
	_rtd_part_outl(0x1800dc44, 7, 4, 0); //reg_p0_b_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Data_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	_rtd_part_outl(0x1800dc44, 7, 4, 1); //reg_p0_b_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Data_Odd_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	_rtd_part_outl(0x1800dc44, 7, 4, 2); //reg_p0_b_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	_rtd_part_outl(0x1800dc44, 7, 4, 3); //reg_p0_b_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	_rtd_part_outl(0x1800dc44, 7, 4, 8); //reg_p0_b_offset_coef_sel[4:0]
	HDMI_PHY_INFO("EQ_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 4, 0));
	//G-Lane LEQ & DCVS Offset Result
	_rtd_part_outl(0x1800dc54, 7, 4, 0); //reg_p0_g_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Data_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	_rtd_part_outl(0x1800dc54, 7, 4, 1); //reg_p0_g_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Data_Odd_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	_rtd_part_outl(0x1800dc54, 7, 4, 2); //reg_p0_g_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	_rtd_part_outl(0x1800dc54, 7, 4, 3); //reg_p0_g_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	_rtd_part_outl(0x1800dc54, 7, 4, 8); //reg_p0_g_offset_coef_sel[4:0]
	HDMI_PHY_INFO("EQ_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 12, 8));
	//R-Lane LEQ & DCVS Offset Result
	_rtd_part_outl(0x1800dc64, 7, 4, 0); //reg_p0_r_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Data_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	_rtd_part_outl(0x1800dc64, 7, 4, 1); //reg_p0_r_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Data_Odd_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	_rtd_part_outl(0x1800dc64, 7, 4, 2); //reg_p0_r_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	_rtd_part_outl(0x1800dc64, 7, 4, 3); //reg_p0_r_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	_rtd_part_outl(0x1800dc64, 7, 4, 8); //reg_p0_r_offset_coef_sel[4:0]
	HDMI_PHY_INFO("EQ_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 20, 16));
	//CK-Lane LEQ & DCVS Offset Result
	_rtd_part_outl(0x1800dc74, 7, 4, 0); //reg_p0_ck_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Data_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 28, 24));
	_rtd_part_outl(0x1800dc74, 7, 4, 1); //reg_p0_ck_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Data_Odd_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 28, 24));
	_rtd_part_outl(0x1800dc74, 7, 4, 2); //reg_p0_ck_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 28, 24));
	_rtd_part_outl(0x1800dc74, 7, 4, 3); //reg_p0_ck_offset_coef_sel[4:0]
	HDMI_PHY_INFO("Edge_Even_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 28, 24));
	_rtd_part_outl(0x1800dc74, 7, 4, 8); //reg_p0_ck_offset_coef_sel[4:0]
	HDMI_PHY_INFO("EQ_Koffset = %d\n",_rtd_part_inl(0x1800dc84, 28, 24));
	//Input on
	_rtd_part_outl(0x1800da50, 0, 0, 0); //INOFF_EN =0 Input_on
	_rtd_part_outl(0x1800da70, 0, 0, 0); //INOFF_EN =0 Input_on
	_rtd_part_outl(0x1800da90, 0, 0, 0); //INOFF_EN =0 Input_on
	_rtd_part_outl(0x1800da30, 0, 0, 0); //INOFF_EN =0 Input_on
	_rtd_part_outl(0x1800da58, 28, 28, 0); //#B  DFE_TAP0_ICOM_EN=0
	_rtd_part_outl(0x1800da5c, 6, 3, Koffset_frl_1[frl_timing].B_13_386); //#B  DFE_TAP_EN[4:1] = 0xf
	_rtd_part_outl(0x1800da38, 28, 28, 0); //#CK DFE_TAP0_ICOM_EN=0
	_rtd_part_outl(0x1800da3c, 6, 3, Koffset_frl_1[frl_timing].CK_13_388); //#CK DFE_TAP_EN[4:1] = 0xf
	_rtd_part_outl(0x1800da78, 28, 28, 0); //#G  DFE_TAP0_ICOM_EN=0
	_rtd_part_outl(0x1800da7c, 6, 3, Koffset_frl_1[frl_timing].G_13_390); //#G  DFE_TAP_EN[4:1] = 0xf
	_rtd_part_outl(0x1800da98, 28, 28, 0); //#R  DFE_TAP0_ICOM_EN=0
	_rtd_part_outl(0x1800da9c, 6, 3, Koffset_frl_1[frl_timing].R_13_392); //#R  DFE_TAP_EN[4:1] = 0xf
	_rtd_part_outl(0x1800dd04, 0, 0, 1); //R finetunstart on
	_rtd_part_outl(0x1800dce8, 0, 0, 1); //G finetunstart on
	_rtd_part_outl(0x1800dcc8, 0, 0, 1); //B finetunstart on
	_rtd_part_outl(0x1800dd20, 0, 0, Koffset_frl_1[frl_timing].ck_fine_tune_start_396); //CK finetunstart on
}

void LEQ_VTH_Tap0_3_4_Adapt_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing)
{
	_rtd_part_outl(0x1800dd04, 0, 0, 0); //finetunstart off
	_rtd_part_outl(0x1800dce8, 0, 0, 0); //finetunstart off
	_rtd_part_outl(0x1800dcc8, 0, 0, 0); //finetunstart off
	_rtd_part_outl(0x1800dd20, 0, 0, 0); //finetunstart off
	ScalerTimer_DelayXms(5);
	//DFE_Adaptation
	//===CK LANE TAP0 & LEQ & Tap3 & Tap4 Adapt
	_rtd_part_outl(0x180b7ccc, 31, 0, 0x00007400); //Timer, set eqfe_en,servo_en,timer_en=1
	_rtd_part_outl(0x180b7cd0, 31, 31, 1); //leq1_inv=1
	_rtd_part_outl(0x180b7cd0, 30, 30, 1); //leq2_inv=0
	_rtd_part_outl(0x180b7cd4, 17, 15, 0); //LEQ_GAIN1=0
	_rtd_part_outl(0x180b7cd4, 14, 12, 0); //LEQ_GAIN2=0
	_rtd_part_outl(0x180b7cd8, 22, 21, LEQ_VTH_Tap0_3_4_Adapt_frl_1[frl_timing].LEQ1_TRANS_MODE_CK_1_0_12); //20211122_LEQ1_trans_mode=2
	_rtd_part_outl(0x180b7cd8, 20, 19, 3); //LEQ2_trans_mode=3
	_rtd_part_outl(0x180b7cd8, 28, 26, 4); //tap0_gain=4
	_rtd_part_outl(0x180b7ccc, 26, 24, 5); //20200824_tap_timer=5
	_rtd_part_outl(0x180b7ccc, 20, 18, 5); //20200825_LEQ_timer=5
	_rtd_part_outl(0x180b7ccc, 23, 21, 5); //20200825_VTH_timer=5
	_rtd_part_outl(0x180b7ccc, 13, 13, 1); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7ce8, 25, 25, 1); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7ce8, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=0
	_rtd_part_outl(0x180b7ce8, 27, 27, 0); //20211122_DFE_ADAP_EN_tap2=0
	_rtd_part_outl(0x180b7ce8, 28, 28, LEQ_VTH_Tap0_3_4_Adapt_frl_1[frl_timing].DFE_ADAPT_EN_CK_6_0_22); //20211122_DFE_ADAP_EN_tap3=1
	_rtd_part_outl(0x180b7ce8, 29, 29, LEQ_VTH_Tap0_3_4_Adapt_frl_1[frl_timing].DFE_ADAPT_EN_CK_6_0_23); //20211122_DFE_ADAP_EN_tap4=1
	_rtd_part_outl(0x180b7cc8, 23, 22, 0x3); //VTH_DFE_EN_B=1, VTH_EN_B=1
	_rtd_part_outl(0x180b7cdc, 31, 30, 0x1); //tap0_adjust_b =1
	//===B LANE  TAP0 & LEQ & Tap3 & Tap4 Adapt
	_rtd_part_outl(0x180b7c8c, 31, 0, 0x00007400); //Timer, set eqfe_en,servo_en,timer_en=1
	_rtd_part_outl(0x180b7c90, 31, 31, 1); //leq1_inv=1
	_rtd_part_outl(0x180b7c90, 30, 30, 1); //leq2_inv=0
	_rtd_part_outl(0x180b7c94, 17, 15, 0); //LEQ_GAIN1=0
	_rtd_part_outl(0x180b7c94, 14, 12, 0); //LEQ_GAIN2=0
	_rtd_part_outl(0x180b7c98, 22, 21, LEQ_VTH_Tap0_3_4_Adapt_frl_1[frl_timing].LEQ1_TRANS_MODE_B_1_0_32); //20211122_LEQ1_trans_mode=2
	_rtd_part_outl(0x180b7c98, 20, 19, 3); //LEQ2_trans_mode=3
	_rtd_part_outl(0x180b7c98, 28, 26, 4); //tap0_gain=4
	_rtd_part_outl(0x180b7c8c, 26, 24, 5); //20200824_tap_timer=5
	_rtd_part_outl(0x180b7c8c, 20, 18, 5); //20200825_LEQ_timer=5
	_rtd_part_outl(0x180b7c8c, 23, 21, 5); //20200825_VTH_timer=5
	_rtd_part_outl(0x180b7c8c, 13, 13, 1); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7ca8, 25, 25, 1); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7ca8, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=0
	_rtd_part_outl(0x180b7ca8, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=0
	_rtd_part_outl(0x180b7ca8, 28, 28, LEQ_VTH_Tap0_3_4_Adapt_frl_1[frl_timing].DFE_ADAPT_EN_B_6_0_42); //20211122_DFE_ADAP_EN_tap3=1
	_rtd_part_outl(0x180b7ca8, 29, 29, LEQ_VTH_Tap0_3_4_Adapt_frl_1[frl_timing].DFE_ADAPT_EN_B_6_0_43); //20211122_DFE_ADAP_EN_tap4=1
	_rtd_part_outl(0x180b7c88, 23, 22, 0x3); //VTH_DFE_EN_B=1, VTH_EN_B=1
	_rtd_part_outl(0x180b7c9c, 31, 30, 0x1); //tap0_adjust_b =1
	//===G LANE  TAP0 & LEQ & Tap3 & Tap4 Adapt
	_rtd_part_outl(0x180b7c4c, 31, 0, 0x00007400); //Timer, set eqfe_en,servo_en,timer_en=1
	_rtd_part_outl(0x180b7c50, 31, 31, 1); //leq1_inv=1
	_rtd_part_outl(0x180b7c50, 30, 30, 1); //leq2_inv=0
	_rtd_part_outl(0x180b7c54, 17, 15, 0); //LEQ_GAIN1=0
	_rtd_part_outl(0x180b7c54, 14, 12, 0); //LEQ_GAIN2=0
	_rtd_part_outl(0x180b7c58, 22, 21, LEQ_VTH_Tap0_3_4_Adapt_frl_1[frl_timing].LEQ1_TRANS_MODE_G_1_0_52); //20211122_LEQ1_trans_mode=2
	_rtd_part_outl(0x180b7c58, 20, 19, 3); //LEQ2_trans_mode=3
	_rtd_part_outl(0x180b7c58, 28, 26, 4); //tap0_gain=4
	_rtd_part_outl(0x180b7c4c, 26, 24, 5); //20200824_tap_timer=5
	_rtd_part_outl(0x180b7c4c, 20, 18, 5); //20200825_LEQ_timer=5
	_rtd_part_outl(0x180b7c4c, 23, 21, 5); //20200825_VTH_timer=5
	_rtd_part_outl(0x180b7c4c, 13, 13, 1); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7c68, 25, 25, 1); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7c68, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=0
	_rtd_part_outl(0x180b7c68, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=0
	_rtd_part_outl(0x180b7c68, 28, 28, LEQ_VTH_Tap0_3_4_Adapt_frl_1[frl_timing].DFE_ADAPT_EN_G_6_0_62); //20211122_DFE_ADAP_EN_tap3=1
	_rtd_part_outl(0x180b7c68, 29, 29, LEQ_VTH_Tap0_3_4_Adapt_frl_1[frl_timing].DFE_ADAPT_EN_G_6_0_63); //20211122_DFE_ADAP_EN_tap4=1
	_rtd_part_outl(0x180b7c48, 23, 22, 0x3); //VTH_DFE_EN_B=1, VTH_EN_B=1
	_rtd_part_outl(0x180b7c5c, 31, 30, 0x1); //tap0_adjust_b =1
	//===R LANE TAP0 & LEQ & Tap3 & Tap4 Adapt
	_rtd_part_outl(0x180b7c0c, 31, 0, 0x00007400); //Timer, set eqfe_en,servo_en,timer_en=1
	_rtd_part_outl(0x180b7c10, 31, 31, 1); //leq1_inv=1
	_rtd_part_outl(0x180b7c10, 30, 30, 1); //leq2_inv=0
	_rtd_part_outl(0x180b7c14, 17, 15, 0); //LEQ_GAIN1=0
	_rtd_part_outl(0x180b7c14, 14, 12, 0); //LEQ_GAIN2=0
	_rtd_part_outl(0x180b7c18, 22, 21, LEQ_VTH_Tap0_3_4_Adapt_frl_1[frl_timing].LEQ1_TRANS_MODE_R_1_0_72); //20211122_LEQ1_trans_mode=2
	_rtd_part_outl(0x180b7c18, 20, 19, 3); //LEQ2_trans_mode=3
	_rtd_part_outl(0x180b7c18, 28, 26, 4); //tap0_gain=4
	_rtd_part_outl(0x180b7c0c, 26, 24, 5); //20200824_tap_timer=5
	_rtd_part_outl(0x180b7c0c, 20, 18, 5); //20200825_LEQ_timer=5
	_rtd_part_outl(0x180b7c0c, 23, 21, 5); //20200825_VTH_timer=5
	_rtd_part_outl(0x180b7c0c, 13, 13, 1); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7c28, 25, 25, 1); //20210807_DFE_ADAP_EN_tap0=1
	_rtd_part_outl(0x180b7c28, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=0
	_rtd_part_outl(0x180b7c28, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=0
	_rtd_part_outl(0x180b7c28, 28, 28, LEQ_VTH_Tap0_3_4_Adapt_frl_1[frl_timing].DFE_ADAPT_EN_R_6_0_82); //20211122_DFE_ADAP_EN_tap3=1
	_rtd_part_outl(0x180b7c28, 29, 29, LEQ_VTH_Tap0_3_4_Adapt_frl_1[frl_timing].DFE_ADAPT_EN_R_6_0_83); //20211122_DFE_ADAP_EN_tap4=1
	_rtd_part_outl(0x180b7c08, 23, 22, 0x3); //VTH_DFE_EN_B=1, VTH_EN_B=1
	_rtd_part_outl(0x180b7c1c, 31, 30, 0x1); //tap0_adjust_b =1
	//Disable DFE adapt
	_rtd_part_outl(0x1800dd70, 3, 0, 0xf); //ck,r,g,b rstb_eq=1
	_rtd_part_outl(0x1800dd70, 7, 4, 0xf); //ck,r,g,b leq_en=1
	_rtd_part_outl(0x1800dd70, 15, 12, 0xf); //ck,r,g,b vth_en=1
	_rtd_part_outl(0x1800dd70, 11, 8, 0xf); //dfe_en=1
	_rtd_part_outl(0x1800dd04, 0, 0, 1); //finetunstart on
	_rtd_part_outl(0x1800dce8, 0, 0, 1); //finetunstart on
	_rtd_part_outl(0x1800dcc8, 0, 0, 1); //finetunstart on
	_rtd_part_outl(0x1800dd20, 0, 0, LEQ_VTH_Tap0_3_4_Adapt_frl_1[frl_timing].ck_fine_tune_start_94); //finetunstart on
	ScalerTimer_DelayXms(5);
	//CK-Lane Tap0-4 Adapt
	_rtd_part_outl(0x180b7ccc, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7ce8, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7ce8, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=1
	_rtd_part_outl(0x180b7ce8, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=1
	_rtd_part_outl(0x180b7ce8, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=1
	_rtd_part_outl(0x180b7ce8, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=1
	_rtd_part_outl(0x180b7cc8, 23, 22, 0); //VTH_DFE_EN_B=1, VTH_EN_B=1
	//B-Lane Tap0-4 Adapt
	_rtd_part_outl(0x180b7c8c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7ca8, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7ca8, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=1
	_rtd_part_outl(0x180b7ca8, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=1
	_rtd_part_outl(0x180b7ca8, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=1
	_rtd_part_outl(0x180b7ca8, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=1
	_rtd_part_outl(0x180b7c88, 23, 22, 0); //VTH_DFE_EN_B=1, VTH_EN_B=1
	//G-Lane Tap0-4 Adapt
	_rtd_part_outl(0x180b7c4c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7c68, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7c68, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=1
	_rtd_part_outl(0x180b7c68, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=1
	_rtd_part_outl(0x180b7c68, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=1
	_rtd_part_outl(0x180b7c68, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=1
	_rtd_part_outl(0x180b7c48, 23, 22, 0); //VTH_DFE_EN_B=1, VTH_EN_B=1
	//R-Lane Tap0-4 Adapt
	_rtd_part_outl(0x180b7c0c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7c28, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7c28, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=1
	_rtd_part_outl(0x180b7c28, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=1
	_rtd_part_outl(0x180b7c28, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=1
	_rtd_part_outl(0x180b7c28, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=1
	_rtd_part_outl(0x180b7c08, 23, 22, 0); //VTH_DFE_EN_B=1, VTH_EN_B=1
	_rtd_part_outl(0x1800dd04, 0, 0, 0); //finetunstart off
	_rtd_part_outl(0x1800dce8, 0, 0, 0); //finetunstart off
	_rtd_part_outl(0x1800dcc8, 0, 0, 0); //finetunstart off
	_rtd_part_outl(0x1800dd20, 0, 0, 0); //finetunstart off
}

void Tap0_to_Tap4_Adapt_frl_1_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing)
{
	_rtd_part_outl(0x1800dd04, 0, 0, 0); //finetunstart off
	_rtd_part_outl(0x1800dce8, 0, 0, 0); //finetunstart off
	_rtd_part_outl(0x1800dcc8, 0, 0, 0); //finetunstart off
	_rtd_part_outl(0x1800dd20, 0, 0, 0); //finetunstart off
	ScalerTimer_DelayXms(5);
	_rtd_part_outl(0x1800dd70, 3, 0, 0xf); //ck,r,g,b rstb_eq=1
	_rtd_part_outl(0x1800dd70, 7, 4, 0xf); //ck,r,g,b leq_en=1
	_rtd_part_outl(0x1800dd70, 15, 12, 0xf); //ck,r,g,b vth_en=1
	_rtd_part_outl(0x1800dd70, 11, 8, 0xf); //dfe_en=1
	//CK-Lane
	_rtd_part_outl(0x180b7ccc, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7ce8, 25, 25, 1); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7ce8, 26, 26, 1); //20200824_DFE_ADAP_EN_tap1=0
	_rtd_part_outl(0x180b7ce8, 27, 27, 1); //20200824_DFE_ADAP_EN_tap2=0
	_rtd_part_outl(0x180b7ce8, 28, 28, Tap0_to_Tap4_Adapt_frl_1[frl_timing].DFE_ADAPT_EN_CK_6_0_14); //20200824_DFE_ADAP_EN_tap3=0
	_rtd_part_outl(0x180b7ce8, 29, 29, Tap0_to_Tap4_Adapt_frl_1[frl_timing].DFE_ADAPT_EN_CK_6_0_15); //20200824_DFE_ADAP_EN_tap4=0
	_rtd_part_outl(0x180b7cc8, 23, 22, 0x3); //VTH_DFE_EN_B=1, VTH_EN_B=1
	//B-Lane
	_rtd_part_outl(0x180b7c8c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7ca8, 25, 25, 1); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7ca8, 26, 26, 1); //20200824_DFE_ADAP_EN_tap1=0
	_rtd_part_outl(0x180b7ca8, 27, 27, 1); //20200824_DFE_ADAP_EN_tap2=0
	_rtd_part_outl(0x180b7ca8, 28, 28, Tap0_to_Tap4_Adapt_frl_1[frl_timing].DFE_ADAPT_EN_B_6_0_22); //20200824_DFE_ADAP_EN_tap3=0
	_rtd_part_outl(0x180b7ca8, 29, 29, Tap0_to_Tap4_Adapt_frl_1[frl_timing].DFE_ADAPT_EN_B_6_0_23); //20200824_DFE_ADAP_EN_tap4=0
	_rtd_part_outl(0x180b7c88, 23, 22, 0x3); //VTH_DFE_EN_B=1, VTH_EN_B=1
	//G-Lane
	_rtd_part_outl(0x180b7c4c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7c68, 25, 25, 1); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7c68, 26, 26, 1); //20200824_DFE_ADAP_EN_tap1=0
	_rtd_part_outl(0x180b7c68, 27, 27, 1); //20200824_DFE_ADAP_EN_tap2=0
	_rtd_part_outl(0x180b7c68, 28, 28, Tap0_to_Tap4_Adapt_frl_1[frl_timing].DFE_ADAPT_EN_G_6_0_30); //20200824_DFE_ADAP_EN_tap3=0
	_rtd_part_outl(0x180b7c68, 29, 29, Tap0_to_Tap4_Adapt_frl_1[frl_timing].DFE_ADAPT_EN_G_6_0_31); //20200824_DFE_ADAP_EN_tap4=0
	_rtd_part_outl(0x180b7c48, 23, 22, 0x3); //VTH_DFE_EN_B=1, VTH_EN_B=1
	//R-Lane
	_rtd_part_outl(0x180b7c0c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7c28, 25, 25, 1); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7c28, 26, 26, 1); //20200824_DFE_ADAP_EN_tap1=0
	_rtd_part_outl(0x180b7c28, 27, 27, 1); //20200824_DFE_ADAP_EN_tap2=0
	_rtd_part_outl(0x180b7c28, 28, 28, Tap0_to_Tap4_Adapt_frl_1[frl_timing].DFE_ADAPT_EN_R_6_0_38); //20200824_DFE_ADAP_EN_tap3=0
	_rtd_part_outl(0x180b7c28, 29, 29, Tap0_to_Tap4_Adapt_frl_1[frl_timing].DFE_ADAPT_EN_R_6_0_39); //20200824_DFE_ADAP_EN_tap4=0
	_rtd_part_outl(0x180b7c08, 23, 22, 0x3); //VTH_DFE_EN_B=1, VTH_EN_B=1
	_rtd_part_outl(0x1800dd04, 0, 0, 1); //R finetunstart on
	_rtd_part_outl(0x1800dce8, 0, 0, 1); //G finetunstart on
	_rtd_part_outl(0x1800dcc8, 0, 0, 1); //B finetunstart on
	_rtd_part_outl(0x1800dd20, 0, 0, Tap0_to_Tap4_Adapt_frl_1[frl_timing].ck_fine_tune_start_44); //CK finetunstart on
	ScalerTimer_DelayXms(5);
	//CK-Lane
	_rtd_part_outl(0x180b7ccc, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7ce8, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7ce8, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=1
	_rtd_part_outl(0x180b7ce8, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=1
	_rtd_part_outl(0x180b7ce8, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=1
	_rtd_part_outl(0x180b7ce8, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=1
	_rtd_part_outl(0x180b7cc8, 23, 22, 0); //VTH_DFE_EN_B=1, VTH_EN_B=1
	//B-Lane
	_rtd_part_outl(0x180b7c8c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7ca8, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7ca8, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=1
	_rtd_part_outl(0x180b7ca8, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=1
	_rtd_part_outl(0x180b7ca8, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=1
	_rtd_part_outl(0x180b7ca8, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=1
	_rtd_part_outl(0x180b7c88, 23, 22, 0); //VTH_DFE_EN_B=1, VTH_EN_B=1
	//G-Lane
	_rtd_part_outl(0x180b7c4c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7c68, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7c68, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=1
	_rtd_part_outl(0x180b7c68, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=1
	_rtd_part_outl(0x180b7c68, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=1
	_rtd_part_outl(0x180b7c68, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=1
	_rtd_part_outl(0x180b7c48, 23, 22, 0); //VTH_DFE_EN_B=1, VTH_EN_B=1
	//R-Lane
	_rtd_part_outl(0x180b7c0c, 13, 13, 0); //20210807_DFE ADAP LE=0
	_rtd_part_outl(0x180b7c28, 25, 25, 0); //20210807_DFE_ADAP_EN_tap0=0
	_rtd_part_outl(0x180b7c28, 26, 26, 0); //20200824_DFE_ADAP_EN_tap1=1
	_rtd_part_outl(0x180b7c28, 27, 27, 0); //20200824_DFE_ADAP_EN_tap2=1
	_rtd_part_outl(0x180b7c28, 28, 28, 0); //20200824_DFE_ADAP_EN_tap3=1
	_rtd_part_outl(0x180b7c28, 29, 29, 0); //20200824_DFE_ADAP_EN_tap4=1
	_rtd_part_outl(0x180b7c08, 23, 22, 0); //VTH_DFE_EN_B=1, VTH_EN_B=1
	_rtd_part_outl(0x1800dd04, 0, 0, 0); //R finetunstart off
	_rtd_part_outl(0x1800dce8, 0, 0, 0); //G finetunstart off
	_rtd_part_outl(0x1800dcc8, 0, 0, 0); //B finetunstart off
	_rtd_part_outl(0x1800dd20, 0, 0, 0); //CK finetunstart off
	ScalerTimer_DelayXms(5);
}

void ACDR_settings_frl_3_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing)
{
	_rtd_part_outl(0x1800dcc8, 0, 0, 0x1); //finetunestart_r
	_rtd_part_outl(0x1800dce8, 0, 0, 0x1); //finetunestart_g
	_rtd_part_outl(0x1800dd04, 0, 0, 0x1); //finetunestart_b
	_rtd_part_outl(0x1800dd20, 0, 0, finetunestart_on[frl_timing].ck_fine_tune_start_4); //finetunestart_ck
	//#===================================== phase III, ACDR to koffset mode
}

void ACDR_settings_frl_4_func(unsigned char nport, FRL_TIMING_PARAM_T frl_timing)
{
	_rtd_part_outl(0x1800dc88, 4, 4, 0x0);
	_rtd_part_outl(0x1800dc88, 12, 12, 0x0);
	_rtd_part_outl(0x1800dc88, 20, 20, 0x0);
	_rtd_part_outl(0x1800dc88, 28, 28, 0x0);
	_rtd_part_outl(0x1800dc88, 4, 4, 0x1);
	_rtd_part_outl(0x1800dc88, 12, 12, 0x1);
	_rtd_part_outl(0x1800dc88, 20, 20, 0x1);
	_rtd_part_outl(0x1800dc88, 28, 28, 0x1);
	_rtd_part_outl(0x1800dc00, 0, 0, 0x0);
	_rtd_part_outl(0x1800dc00, 1, 1, 0x0);
	_rtd_part_outl(0x1800dc00, 2, 2, 0x0);
	_rtd_part_outl(0x1800dc00, 3, 3, 0x0);
	_rtd_part_outl(0x1800dc00, 0, 0, 0x1);
	_rtd_part_outl(0x1800dc00, 1, 1, 0x1);
	_rtd_part_outl(0x1800dc00, 2, 2, 0x1);
	_rtd_part_outl(0x1800dc00, 3, 3, 0x1);
	//DFE Adaptation power off
	_rtd_part_outl(0x1800da3c, 7, 7, 0); //Mark2_Modify; CK-Lane DFE_ADAPT_EN
	_rtd_part_outl(0x1800da5c, 7, 7, 0); //Mark2_Modify; B-Lane DFE_ADAPT_EN
	_rtd_part_outl(0x1800da7c, 7, 7, 0); //Mark2_Modify; G-Lane DFE_ADAPT_EN
	_rtd_part_outl(0x1800da9c, 7, 7, 0); //Mark2_Modify; R-Lane DFE_ADAPT_EN
}


/**********************************************************************************************
*
*	Const Declarations
*
**********************************************************************************************/
#if  0
DCDR_DFE_PARAM_T DCDR_set_dfe_param[] =
{
	{TMDS_Timing_6G,     0x5,0xF,0x10,0x0,0x0,0x0,0x0,0xF,  0x1,0x1,0x1,0x1,0x1,0x1},
	{TMDS_Timing_3G,     0x5,0xF,0x10,0x0,0x0,0x0,0x0,0x3,  0x1,0x1,0x1,0x1,0x0,0x0},
	{TMDS_Timing_1P5G,     0x5,0xF,0x10,0x0,0x0,0x0,0x0,0x1,  0x1,0x1,0x0,0x0,0x0,0x0},
	{TMDS_Timing_742M,     0x5,0x5,0x10,0x0,0x0,0x0,0x0,0x1,  0x0,0x1,0x0,0x0,0x0,0x0},
	{TMDS_Timing_540M,     0x5,0x5,0x10,0x0,0x0,0x0,0x0,0x1,  0x0,0x1,0x0,0x0,0x0,0x0},
	{TMDS_Timing_250M,     0x5,0x0,0x19,0x0,0x0,0x0,0x0,0x0,  0x0,0x0,0x0,0x0,0x0,0x0},

};

ACDR_DFE_PARAM_T ACDR_set_dfe_param[] =
{
	{FRL_Timing_12G,     0x3,0xF,0x10,0x0,0x0,0x0,0x0,0xF,  0x1,0x1,0x1,0x1,0x1,0x1,0x2},
	{FRL_Timing_10G,     0x5,0xF,0x10,0x0,0x0,0x0,0x0,0xF,  0x1,0x1,0x1,0x1,0x1,0x1,0x2},
	{FRL_Timing_8G,     0x5,0xF,0x10,0x0,0x0,0x0,0x0,0xF,  0x1,0x1,0x1,0x1,0x1,0x1,0x2},
	{FRL_Timing_6G,     0x5,0xF,0x10,0x0,0x0,0x0,0x0,0xF,  0x1,0x1,0x1,0x1,0x1,0x1,0x1},
	{FRL_Timing_3G,     0x5,0xF,0x10,0x0,0x0,0x0,0x0,0x3,  0x1,0x1,0x1,0x1,0x0,0x0,0x1},

};

//DCDR_set_dfe_param[TMDS_Timing].VTH_INIT
DCDR_PHY_TMDS_PARAM_T DCDR_set_phy_param[] =
{                        
	//Timing		
	{TMDS_Timing_6G,     0x0,0x2,0x2,0x0,0x0,0x2,0x1,0x1,0x7,0x5, 0x4,0x3,   0xd,0xf,0xf,0x1,0x0,0x1,0x3,0x0,0x0, 0x0,   
	0xf,0x2,0x1,0x1,     0x1,0xf,0x0,0x1,0x0,0x1,0x0,0x0,  0x24,0x0,0x0,0x1,   0x8,0x8,0x1,0x1,0x0,0x2,0x98,0x2f},

	{TMDS_Timing_3G,     0x0,0x1,0x1,0x0,0x0,0x1,0x0,0x0, 0x3,0x3,0x1,0x0,   0x0,0x0,0x0,0x0,0x1,0x0,0x3,0x0,0x0, 0x0,    
	0x3,0x3,0x1,0x2,     0x1,0xf,0x0,0x1,0x0,0x1,0x1,0x0,  0x10,0x0,0x0,0x1,   0x12,0x8,0x1,0x2,0x1,0x2,0x98,0x27},

	{TMDS_Timing_1P5G,  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1, 0x0,0x0,   0x0,0x0,0x0,0x0,0x1,0x0,0x3,0x0,0x0,0x0,   
	0x1,0x3,0x1,0x3,     0x1,0xf,0x0,0x1,0x0,0x1,0x2,0x0,  0x24,0x0,0x0,0x1,   0x14,0x4,0x0,0x3,0x1,0x2,0x98,0x2f},

	{TMDS_Timing_742M,  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x1,0x1,0x0,0x0,   0x0,0x0,0x0,0x0,0x1,0x0,0x3,0x0,0x0,0x0,    
	0x1,0x3,0x1,0x3,     0x1,0xf,0x0,0x1,0x0,0x1,0x3,0x0,  0x4c,0x0,0x0,0x1,   0x14,0x4,0x0,0x3,0x1,0x6,0x98,0x2d},

	{TMDS_Timing_540M,  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x1,0x1,0x0,0x0,   0x0,0x0,0x0,0x0,0x1,0x0,0x3,0x0,0x0,0x0,    
	0x1,0x3,0x1,0x3,     0x0,0xc,0x0,0x1,0x3,0x1,0x3,0x0,  0x4c,0x0,0x0,0x1,   0x14,0x4,0x0,0x3,0x1,0x6,0x98,0x2d},
	
	{TMDS_Timing_250M,  0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1,0x1, 0x0,0x0,   0x0,0x0,0x0,0x0,0x1,0x0,0x3,0x0,0x0,0x0,    
	0x1,0x3,0x1,0x3,     0x0,0x7,0x0,0x1,0x6,0x1,0x4,0x0,  0x9c,0x0,0x0,0x1,   0x14,0x4,0x0,0x3,0x1,0x6,0xa0,0x2f},
};


ACDR_PHY_FRL_PARAM_T ACDR_set_phy_param[] =
{                        
	//Timing		 
	{FRL_Timing_12G,  0x356,0x32c,    0x0,0x3,0x3,0x3,0x1,0x3,0x3,0x1,0xc,0xc, 0x4,0x3,0x2,   0xf,0xf,0xc,0x0,0x1,0x3,0x3,0x0,0x0,0x0,   
	0x0,0xf,0x1,0x0,0x1,     0x0,0x0,0x0,0x4,0x3,0x2,  0x6b,0x0,0x1,0x1,0x0,   0x0,0x77750,0x81},

	{FRL_Timing_10G,  0x2cc,0x2a2,    0x0,0x2,0x2,0x3,0x1,0x3,0x3,0x1,0xc,0xc, 0x6,0x3,0x2,   0xf,0xf,0xd,0x0,0x1,0x3,0x3,0x0,0x0,0x0,    
	0x0,0xf,0x1,0x0,0x1,     0x0,0x0,0x0,0x4,0x2,0x1,  0xb5,0x0,0x0,0x1,0x1,   0x0,0x77750,0x81},

	{FRL_Timing_8G,   0x243,0x215,    0x0,0x2,0x2,0x3,0x1,0x3,0x3,0x1,0xc,0xc, 0x6,0x3,0x2,   0xf,0xf,0xd,0x0,0x1,0x3,0x3,0x0,0x0,0x0,    
	0x0,0xf,0x1,0x0,0x1,     0x0,0x0,0x0,0x4,0x0,0x1,  0x90,0x0,0x0,0x1,0x0,   0x0,0x77770,0x81},

	{FRL_Timing_6G,   0x356,0x32c,    0x0,0x2,0x2,0x0,0x0,0x3,0x1,0x1,0x7,0x5, 0x4,0x3,0x1,   0xd,0xf,0xf,0x0,0x1,0x3,0x3,0x0,0x0,0x0,    
	0x1,0xf,0x0,0x1,0x1,      0x0,0x0,0x0,0x4,0x3,0x1,  0xda,0x0,0x0,0x1,0x1,   0x1,0x77770,0x81},

	{FRL_Timing_3G,   0x356,0x32c,    0x0,0x1,0x1,0x0,0x0,0x1,0x0,0x0,0x3,0x3, 0x1,0x0,0x0,   0x0,0x0,0x0,0x1,0x0,0x3,0x3,0x0, 0x0,0x0,    
	0x1,0x3,0x0,0x1,0x1,      0x0,0x0,0x0,0x4,0x3,0x2,  0xda,0x0,0x0,0x0,0x1,   0x1,0x77750,0x81},
};
#endif

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
extern unsigned int clock_bound_3g;
extern unsigned int clock_bound_1p5g;
extern unsigned int clock_bound_110m;
extern unsigned int clock_bound_45m;


extern HDMIRX_PHY_STRUCT_T phy_st[HDMI_PORT_TOTAL_NUM];

extern unsigned int phy_isr_en[HDMI_PORT_TOTAL_NUM];

extern HDMIRX_PHY_FACTORY_T phy_fac_eq_st[HDMI_PORT_TOTAL_NUM];

extern char m_disparity_rst_handle_once;
extern char m_scdc_rst_handle_once;

#if BIST_PHY_SCAN
extern HDMIRX_PHY_SCAN_T  phy_scan_state;
#endif


/**********************************************************************************************
*
*	Funtion Declarations
*
**********************************************************************************************/


unsigned char newbase_hdmi21_set_phy(unsigned char port, unsigned int b, unsigned char frl_mode,unsigned char lane);
//unsigned char newbase_hdmi21_set_phyNew(unsigned char nport, unsigned int b, unsigned char frl_mode,unsigned char lane);


//dfe
void newbase_hdmi21_dump_dfe_para(unsigned char nport, unsigned char lane_mode);
void newbase_hdmi21_dump_dfe_paraNew(unsigned char nport, unsigned char lane_mode);

void lib_hdmi21_dump_dfe_tap0_le(unsigned char nport,unsigned char lane_mode);
unsigned char newbase_hdmi21_tmds_dfe_record(unsigned char nport, unsigned int clk);
unsigned char newbase_hdmi21_tmds_dfe_close(unsigned char nport, unsigned int clk, unsigned char lane_mode, unsigned char phy_state);
unsigned char newbase_hdmi21_frl_dfe_close(unsigned char nport, unsigned int clk, unsigned char lane_mode,unsigned char phy_state);
void newbase_hdmi21_tmds_dfe_hi_speed(unsigned char nport, unsigned int b_clk, unsigned char lane_mode);
void newbase_hdmi21_tmds_dfe_6g_long_cable_patch(unsigned char nport);
void newbase_hdmi21_tmds_dfe_mi_speed(unsigned char nport, unsigned int b_clk, unsigned char lane_mode);
void newbase_hdmi21_tmds_dfe_mid_adapthve(unsigned char nport, unsigned int b_clk);

//Z0
void lib_hdmi21_z0_calibration(void);
void lib_hdmi21_z300_sel(unsigned char nport, unsigned char mode);  // 0: active mode 1:vcm mode

//LDO
void lib_hdmi21_ldo_calibration(void);

//eq setting
#ifdef CONFIG_POWER_SAVING_MODE
void lib_hdmi21_ac_couple_power_en(unsigned char nport,unsigned char en);
void lib_hdmi21_eq_pi_power_en(unsigned char nport,unsigned char en, unsigned char frl_mode,unsigned char lane_mode);
#endif
		
//dfe setting


//PLL/DEMUX/CDR
void lib_hdmi21_cmu_pll_en(unsigned char nport, unsigned char enable,unsigned char frl_mode);
void lib_hdmi21_demux_ck_vcopll_rst(unsigned char nport, unsigned char rst, unsigned char lane_mode);
void lib_hdmi21_cdr_rst(unsigned char nport, unsigned char rst,unsigned char frl_mode,unsigned char lane_mode);

//Watch Dog
void lib_hdmi21_cmu_rst(unsigned char port, unsigned char rst);

//foreground K offset
void lib_fast_swtch_mode(unsigned char nport,unsigned char mode);


int newbase_hdmi21_rxphy_get_frl_info(unsigned char port, unsigned char frl_mode);
void newbase_hdmi21_rxphy_set_frl_info(unsigned char port,unsigned char frl_mode);


/**********************************************************************************************
*
*	Function Body
*
**********************************************************************************************/

#ifdef FORCE_SCRIPT_BASED_PHY_SETTING  // for Bringup only....

void newbase_hdmi_set_phy_script(unsigned char nport, unsigned char frl_mode)
{
	switch(frl_mode)
	{
#if 0
	case 0:
		HDMI_WARN("%s FORCE_SCRIPT_BASED_PHY_SETTING : TMDS 3L6G script -Golden_Setting_V1_Value1_TMDS-5.94G_p0.tbl\n", __func__);
		#include "script/20220315/Golden_Setting_V1_Value1_TMDS-5.94G.tbl"
		break;
	case 4:
		HDMI_WARN("%s FORCE_SCRIPT_BASED_PHY_SETTING : script - Golden_Setting_V1_FRL_Value3_FRL-8G_p0.tbl\n", __func__);
		#include "script/20220315/Golden_Setting_V1_Value3_FRL-8G_p0.tbl"
		break;

	case 5:
		HDMI_WARN("%s FORCE_SCRIPT_BASED_PHY_SETTING : script -Golden_Setting_V1_FRL_Value2_FRL-10G_p0.tbl\n", __func__);
		#include "script/20220315/Golden_Setting_V1_Value2_FRL-10G_p0.tbl"
		break;

	case 6:
		HDMI_WARN("%s FORCE_SCRIPT_BASED_PHY_SETTING : script - Golden_Setting_V1_FRL_Value1_FRL-12G_p0.tbl\n", __func__);
		#include "script/20220315/Golden_Setting_V1_Value1_FRL-12G_p0.tbl"
		break;

	case 3:
		HDMI_WARN("%s FORCE_SCRIPT_BASED_PHY_SETTING : script - Golden_Setting_V1_FRL_Value4_FRL-6G_p0.tbl\n", __func__);
		#include "script/20220315/Golden_Setting_V1_Value4_FRL-6G_p0.tbl"

		break;
	case 2:
		HDMI_WARN("%s FORCE_SCRIPT_BASED_PHY_SETTING : script - Golden_Setting_V1_FRL_Value4_FRL-6G.tbl\n", __func__);
		#include "script/20220315/Golden_Setting_V1_Value4_FRL-6G_p0.tbl"
		break;

	case 1:
		HDMI_WARN("%s FORCE_SCRIPT_BASED_PHY_SETTING : script -Golden_Setting_V1_FRL_Value5_FRL-3G.tbl\n", __func__);
		#include "script/20220315/Golden_Setting_V1_Value5_FRL-3G_p0.tbl"
		break;
#endif		
	default:
		HDMI_WARN("%s FORCE_SCRIPT_BASED_PHY_SETTING : script - Golden_Setting_V1_FRL_Value4_FRL-6G_p0.tbl\n", __func__);
		#include "script/20220315/Golden_Setting_V1_Value4_FRL-6G_p0.tbl"
		//HDMI_WARN("%s FORCE_SCRIPT_BASED_PHY_SETTING : script - rbus_cmd_acdr_6g_port0_modify_without_MAC_V3p5.tbl\n", __func__);
		//#include "script/rbus_cmd_acdr_6g_port0_modify_without_MAC_V3p5.tbl"

		break;
	}
}



void newbase_hdmi_set_tmds_phy_script(unsigned char nport, TMDS_TIMING_PARAM_T TMDS_Timing)
{
	switch(TMDS_Timing)
	{
#if 0
	case TMDS_Timing_6G:
		HDMI_WARN("%s FORCE_SCRIPT_BASED_PHY_SETTING : TMDS 3L6G script -Golden_Setting_V1_Value1_TMDS-5.94G.tbl\n", __func__);
		#include "script/20220315/Golden_Setting_V1_Value1_TMDS-5.94G.tbl"
		break;
	case TMDS_Timing_3G:
		HDMI_WARN("%s FORCE_SCRIPT_BASED_PHY_SETTING : script -Golden_Setting_V1_Value2_TMDS-2.97G.tbl\n", __func__);
		#include "script/20220315/Golden_Setting_V1_Value2_TMDS-2.97G.tbl"
		break;
	case TMDS_Timing_1P5G:
		HDMI_WARN("%s FORCE_SCRIPT_BASED_PHY_SETTING : script -Golden_Setting_V1_Value3_TMDS-1.485G.tbl\n", __func__);
		#include "script/20220315/Golden_Setting_V1_Value3_TMDS-1.485G.tbl"
		break;
	case TMDS_Timing_742M:
		HDMI_WARN("%s FORCE_SCRIPT_BASED_PHY_SETTING : script -Golden_Setting_V1_Value4_TMDS-0.742G.tbl\n", __func__);
		#include "script/20220315/Golden_Setting_V1_Value4_TMDS-0.742G.tbl"
		break;
	case TMDS_Timing_540M:
		HDMI_WARN("%s FORCE_SCRIPT_BASED_PHY_SETTING : script -Golden_Setting_V1_Value4_TMDS-0.54G.tbl\n", __func__);
		#include "script/20220315/Golden_Setting_V1_Value4_TMDS-0.54G.tbl"
		break;
	case TMDS_Timing_250M:
		HDMI_WARN("%s FORCE_SCRIPT_BASED_PHY_SETTING : script - Golden_Setting_V1_Value5_TMDS-0.25G.tbl\n", __func__);
		#include "script/20220315/Golden_Setting_V1_Value5_TMDS-0.25G.tbl"
		break;
#endif
	default:
		//HDMI_WARN("%s FORCE_SCRIPT_BASED_PHY_SETTING : TMDS 3L6G script -Golden_Setting_V1_Value1_TMDS-5.94G.tbl\n", __func__);
		//#include "script/20220315/Golden_Setting_V1_Value1_TMDS-5.94G.tbl"
		HDMI_WARN("%s FORCE_SCRIPT_BASED_PHY_SETTING : TMDS 3L6G script - rbus_cmd_dcdr_5p94g_p0_V2p8_manual_offset_fix_dfe_KI_0.tbl\n", __func__);
		#include "script/rbus_cmd_dcdr_5p94g_p0_V2p8_manual_offset_fix_dfe_KI_0.tbl"

		break;
	}
}



#endif


unsigned char newbase_rxphy_frl_measure(unsigned char port)
{

	if (phy_st[port].frl_mode != phy_st[port].pre_frl_mode) {
		HDMI_EMG("frl mdoe change %d -> %d", phy_st[port].pre_frl_mode,phy_st[port].frl_mode);
		newbase_hdmi21_rxphy_set_frl_info(port,phy_st[port].frl_mode);
		phy_st[port].pre_frl_mode = phy_st[port].frl_mode;
		phy_st[port].clk_pre = phy_st[port].clk;
		phy_st[port].clk_debounce_count= 0;
		phy_st[port].clk_unstable_count = 0;
		phy_st[port].clk_stable = 0;
		phy_st[port].phy_proc_state = PHY_PROC_INIT;
		SET_HDMI_DETECT_EVENT(port, HDMI_DETECT_FRL_MODE_CHANGE);
	}

	return 1;
	
}

void newbase_hdmi21_rxphy_set_frl_info(unsigned char port,unsigned char frl_mode)
{
	switch(frl_mode)
	{
	case 0:
		phy_st[port].lane_num = HDMI_3LANE;
		phy_st[port].frl_mode = MODE_TMDS;
		break;
	case 1:
		phy_st[port].lane_num = HDMI_3LANE;
		phy_st[port].frl_mode = MODE_FRL_3G_3_LANE;
		phy_st[port].clk = FRL_3G_B;
		break;
	case 2:
		phy_st[port].lane_num = HDMI_3LANE;
		phy_st[port].frl_mode = MODE_FRL_6G_3_LANE;
		phy_st[port].clk = FRL_6G_B;
		break;
	case 3:   // FRL:6G/4Ln
		phy_st[port].lane_num = HDMI_4LANE;
		phy_st[port].frl_mode = MODE_FRL_6G_4_LANE;
		phy_st[port].clk = FRL_6G_B;
		break;
	case 4:   // FRL:8G/4Ln
		phy_st[port].lane_num = HDMI_4LANE;
		phy_st[port].frl_mode = MODE_FRL_8G_4_LANE;
		phy_st[port].clk = FRL_8G_B;
		break;
	case 5:   // FRL:10G/4Ln
		phy_st[port].lane_num = HDMI_4LANE;
		phy_st[port].frl_mode = MODE_FRL_10G_4_LANE;
		phy_st[port].clk = FRL_10G_B;
		break;
	case 6:   // FRL:12G/4Ln
		phy_st[port].lane_num = HDMI_4LANE;
		phy_st[port].frl_mode = MODE_FRL_12G_4_LANE;
		phy_st[port].clk = FRL_12G_B;
		break;
	default:   // FRL:6G/4Ln
		phy_st[port].lane_num = HDMI_4LANE;
		phy_st[port].frl_mode = MODE_FRL_6G_4_LANE;
		phy_st[port].clk = FRL_6G_B;
		break;
	}

	phy_st[port].pre_frl_mode = phy_st[port].frl_mode;;
}

unsigned char newbase_hdmi_scdc_get_frl_mode(unsigned char port)
{
	unsigned char frl_mode = MODE_TMDS;
	if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_LINK_TRAINING_BYPASS ) > 0)
	{//bypass link training, force frl mode
	    frl_mode = (unsigned char)GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_LINK_TRAINING_BYPASS );
	}
	else
	{// TX polling FLT Ready and then send change FRL rate
		if (lib_hdmi_scdc_read(port,  SCDC_STATUS_FLAGS) & SCDC_STATUS_FLT_READY)
		{
			frl_mode = (lib_hdmi_scdc_get_sink_config_31(port) & SCDC_FRL_RATE);
		} else {
			frl_mode = MODE_TMDS;
		}
	}
	return frl_mode;
	
}

unsigned char newbase_rxphy_get_frl_mode(unsigned char port)
{
	return phy_st[port].frl_mode;
}

void newbase_rxphy_set_frl_mode(unsigned char port, unsigned char frl_mode)
{

	if (frl_mode == MODE_TMDS)
		phy_st[port].pre_frl_mode = MODE_TMDS;  //for update frl clock if change from tmds to frl
	
	phy_st[port].frl_mode = frl_mode;

//	phy_st[port].lane_num = HDMI_4LANE;
}

void newbase_hdmi_frl_set_phy(unsigned char port, unsigned char frl_mode)
{
	newbase_rxphy_set_frl_mode(port,frl_mode);
    	
       HDMI_EMG("[link training] frl mdoe change %d -> %d", phy_st[port].pre_frl_mode,phy_st[port].frl_mode);
	newbase_hdmi21_rxphy_set_frl_info(port,phy_st[port].frl_mode);
	phy_st[port].pre_frl_mode = phy_st[port].frl_mode;
	phy_st[port].clk_pre = phy_st[port].clk;
	phy_st[port].clk_debounce_count= 0;
	phy_st[port].clk_unstable_count = 0;
	phy_st[port].clk_stable = 0;
	phy_st[port].phy_proc_state = PHY_PROC_INIT;
	SET_HDMI_DETECT_EVENT(port, HDMI_DETECT_PHY_RESET);
	
    		newbase_rxphy_frl_job(port);

}



static unsigned char m_wrong_direction_cnt[4][4];
static unsigned char enable_adj_tap1[4][4];
static unsigned char adj_tap1_done[4][4];
static unsigned char adj_le_done[4][4];
static unsigned char adj_parameter_changed[4][4];
static unsigned char acdr_finetune_en_toggle_once[4][4];;
static unsigned int ber_sum[4];
static unsigned int ber_sum_cnt[4];
unsigned int scdc_extend[4];
void newbase_hdmi_init_disparity_ltp_var(unsigned char nport)
{
	//disp var
	phy_st[nport].disp_max_cnt = 0;
	phy_st[nport].disp_start = 0;
	phy_st[nport].tap1_adp_step[0] = 1;
	phy_st[nport].tap1_adp_step[1] = 1;
	phy_st[nport].tap1_adp_step[2] = 1;
	phy_st[nport].tap1_adp_step[3] = 1;

	phy_st[nport].tap2_adp_step[0] = 1;
	phy_st[nport].tap2_adp_step[1] = 1;
	phy_st[nport].tap2_adp_step[2] = 1;
	phy_st[nport].tap2_adp_step[3] = 1;
	
	phy_st[nport].le_adp_step[0] = 1;
	phy_st[nport].le_adp_step[1] = 1;
	phy_st[nport].le_adp_step[2] = 1;
	phy_st[nport].le_adp_step[3] = 1;
	//phy_st[nport].wrong_direction_cnt[0] = -3;
	//phy_st[nport].wrong_direction_cnt[1] = -3;
	//phy_st[nport].wrong_direction_cnt[2] = -3;
	//phy_st[nport].wrong_direction_cnt[3] = -3;

	m_wrong_direction_cnt[nport][0] = 0;
	m_wrong_direction_cnt[nport][1] = 0;
	m_wrong_direction_cnt[nport][2] = 0;
	m_wrong_direction_cnt[nport][3] = 0;

	adj_tap1_done[nport][0] = 0;
	adj_tap1_done[nport][1] = 0;
	adj_tap1_done[nport][2] = 0;
	adj_tap1_done[nport][3] = 0;

	adj_le_done[nport][0] = 0;
	adj_le_done[nport][1] = 0;
	adj_le_done[nport][2] = 0;
	adj_le_done[nport][3] = 0;

	adj_parameter_changed[nport][0] = 1;
	adj_parameter_changed[nport][1] = 1;
	adj_parameter_changed[nport][2] = 1;
	adj_parameter_changed[nport][3] = 1;
	
	enable_adj_tap1[nport][0] = 0;
	enable_adj_tap1[nport][1] = 0;
	enable_adj_tap1[nport][2] = 0;
	enable_adj_tap1[nport][3] = 0;
	ber_sum[nport]=0;
	ber_sum_cnt[nport]=0;
	scdc_extend[nport]=0;

	acdr_finetune_en_toggle_once[nport][0] = 0;
	acdr_finetune_en_toggle_once[nport][1] = 0;
	acdr_finetune_en_toggle_once[nport][2] = 0;
	acdr_finetune_en_toggle_once[nport][3] = 0;


	//phy_st[nport].disp_err_pre[0] = phy_st[nport].disp_err_pre[1] = phy_st[nport].disp_err_pre[2]  = phy_st[nport].disp_err_pre[3] = 0;
	//phy_st[nport].disp_err_after[0] = phy_st[nport].disp_err_after[1] = phy_st[nport].disp_err_after[2] = phy_st[nport].disp_err_after[3] = 0;
	phy_st[nport].disp_adp_stage[0] = phy_st[nport].disp_adp_stage[1] = phy_st[nport].disp_adp_stage[2] =phy_st[nport].disp_adp_stage[3] = 0;
	//phy_st[nport].disp_err_diff1[0] = phy_st[nport].disp_err_diff1[1]= phy_st[nport].disp_err_diff1[2]= phy_st[nport].disp_err_diff1[3] = 0;
	//phy_st[nport].disp_err_diff2[0] = phy_st[nport].disp_err_diff2[1]= phy_st[nport].disp_err_diff2[2]= phy_st[nport].disp_err_diff2[3] = 0;
	phy_st[nport].disp_err_zero_cnt[0] = phy_st[nport].disp_err_zero_cnt[1]= phy_st[nport].disp_err_zero_cnt[2]= phy_st[nport].disp_err_zero_cnt[3] = 0;
	phy_st[nport].disp_BER_zero_cnt[0] = phy_st[nport].disp_BER_zero_cnt[1]= phy_st[nport].disp_BER_zero_cnt[2]= phy_st[nport].disp_BER_zero_cnt[3] = 0;
	
	//phy_st[nport].disp_err_t[0] = phy_st[nport].disp_err_t[1]= phy_st[nport].disp_err_t[2]= phy_st[nport].disp_err_t[3] = 0;
	//phy_st[nport].err_occur_t1[0] = phy_st[nport].err_occur_t1[1]= phy_st[nport].err_occur_t1[2]= phy_st[nport].err_occur_t1[3] = 0;
	//phy_st[nport].err_occur_t2[0] = phy_st[nport].err_occur_t2[1]= phy_st[nport].err_occur_t2[2]= phy_st[nport].err_occur_t2[3] = 0;
	//phy_st[nport].err_occur_t3[0] = phy_st[nport].err_occur_t3[1]= phy_st[nport].err_occur_t3[2]= phy_st[nport].err_occur_t3[3] = 0;
	phy_st[nport].disp_timer_cnt[0] = phy_st[nport].disp_timer_cnt[1]= phy_st[nport].disp_timer_cnt[2]= phy_st[nport].disp_timer_cnt[3] = 5; 
	
#if PHY_ADAPTIVE_BY_LTP_PAT_AFTER_LTP_PASS
	//ltp var
	phy_st[nport].ltp_err_pre[0] = phy_st[nport].ltp_err_pre[1] = phy_st[nport].ltp_err_pre[2]  = phy_st[nport].ltp_err_pre[3] = 0;
	phy_st[nport].ltp_err_zero_cnt[0] = phy_st[nport].ltp_err_zero_cnt[1] =phy_st[nport].ltp_err_zero_cnt[2] =phy_st[nport].ltp_err_zero_cnt[3] = 0;
	phy_st[nport].ltp_err_start[0] = phy_st[nport].ltp_err_start[1] = phy_st[nport].ltp_err_start[2] =phy_st[nport].ltp_err_start[3] = 0;
	phy_st[nport].ltp_err_end [0] =  phy_st[nport].ltp_err_end [1] = phy_st[nport].ltp_err_end [2]  = phy_st[nport].ltp_err_end [3] = 0;
#endif

	//FRL_Timing_last[nport]=FRL_Timing_Changed;
	//TMDS_Timing_last[nport]=TMDS_Timing_Changed;
}

//bool enable_adj_tap1=FALSE;
//unsigned char acdr_finetune_en_toggle_once = FALSE;
//dfe adaptive according disparity error , this algorithm is by garran
static unsigned int  TimeStamp0[4][4];
static unsigned int  TimeStamp1[4][4];

extern FRL_LT_FSM_STATUS_T lt_fsm_status[HDMI_PORT_TOTAL_NUM];
void lib_hdmi21_phy_error_handling(unsigned char nport)
{
	unsigned int current_time_ms = hdmi_get_system_time_ms();
	#define TAP1_MAX_VALUE	31
	#define TAP2_MAX_VALUE	31
	#define LE_MAX_VALUE	30
	unsigned char align_status;
	int lane = 0;
	unsigned int  TimeStamp2;
	//unsigned int BER2_TH=500;	//Clayton, Add TH for change tuning direction
	//bool enable_adj_le=FALSE;

	if ((newbase_hdmi_get_power_saving_state(nport)==PS_FSM_POWER_SAVING_ON))     // do not do power saving when HDMI is power off
		return ;

	if (phy_st[nport].lane_num == HDMI_4LANE)
		align_status = (hdmi_in(HD21_channel_align_c_reg)  & 0x1) & (hdmi_in(HD21_channel_align_r_reg)  & 0x1) & ( hdmi_in(HD21_channel_align_g_reg) & 0x1) & (hdmi_in(HD21_channel_align_b_reg) & 0x1);
	else
		align_status = (hdmi_in(HD21_channel_align_r_reg)  & 0x1) & ( hdmi_in(HD21_channel_align_g_reg) & 0x1) & (hdmi_in(HD21_channel_align_b_reg) & 0x1);

	if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD21_PHY_LOG_ENABLE ))
		HDMI_PRINTF("[disp init]disp_start=%d, Current Time(ms)=%d, align_status=%d\n",phy_st[nport].disp_start, current_time_ms, align_status);			

	if (align_status) {

		if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD21_PHY_LOG_ENABLE )) //for phy log debugging
		{
			newbase_hdmi21_dump_dfe_para(nport,phy_st[nport].lane_num);
		}

		
		if (phy_st[nport].disp_start == 0) 
		{
			TimeStamp0[nport][0]=hdmi_get_system_time_ms();//Record T0
			TimeStamp0[nport][1]=hdmi_get_system_time_ms();//Record T0
			TimeStamp0[nport][2]=hdmi_get_system_time_ms();//Record T0
			TimeStamp0[nport][3]=hdmi_get_system_time_ms();//Record T0
			
			lib_hdmi_hd21_fw_disparity_error_status(nport,phy_st[nport].disp_err0);//Record error cnt
			phy_st[nport].disp_start = 1;
			phy_st[nport].disp_adp_stage[0]  = phy_st[nport].disp_adp_stage[1]  = phy_st[nport].disp_adp_stage[2] = phy_st[nport].disp_adp_stage[3]  = 1;
			phy_st[nport].disp_timer_cnt[0] = phy_st[nport].disp_timer_cnt[1]= phy_st[nport].disp_timer_cnt[2]= phy_st[nport].disp_timer_cnt[3] = 5;  //100ms
		}
		else if (phy_st[nport].disp_start == 1)
		{

			if (phy_st[nport].disp_timer_cnt[lane] == 0)
			{  //near 100ms
				TimeStamp1[nport][0]=hdmi_get_system_time_ms();//Record T0
				TimeStamp1[nport][1]=hdmi_get_system_time_ms();//Record T0
				TimeStamp1[nport][2]=hdmi_get_system_time_ms();//Record T0
				TimeStamp1[nport][3]=hdmi_get_system_time_ms();//Record T0
				lib_hdmi_hd21_fw_disparity_error_status(nport,phy_st[nport].disp_err1);
				phy_st[nport].disp_start = 2;
				phy_st[nport].disp_adp_stage[0]  = phy_st[nport].disp_adp_stage[1]  = phy_st[nport].disp_adp_stage[2] = phy_st[nport].disp_adp_stage[3]  = 1;
				phy_st[nport].disp_timer_cnt[0] = phy_st[nport].disp_timer_cnt[1]= phy_st[nport].disp_timer_cnt[2]= phy_st[nport].disp_timer_cnt[3] = 5;  //100ms
			}
			else 
			{
				phy_st[nport].disp_timer_cnt[lane]--;
			}
	
		}
		else if (phy_st[nport].disp_start == 2)
		{
			if (phy_st[nport].disp_timer_cnt[lane] == 0) 
			{  //near 100ms
				TimeStamp2=hdmi_get_system_time_ms();
				lib_hdmi_hd21_fw_disparity_error_status(nport,phy_st[nport].disp_err2);
				for (lane = 0; lane < phy_st[nport].lane_num ; lane++) 
				{
					if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD21_PHY_LOG_ENABLE ))
					{
						HDMI_PRINTF("[disp ing]lane=%d, disp_adp_stage[lane]=%d, m_wrong_direction_cnt=%d, disp_err_zero_cnt[lane]=%d \n",lane,
						phy_st[nport].disp_adp_stage[lane], m_wrong_direction_cnt[nport][lane], phy_st[nport].disp_err_zero_cnt[lane] );
					}

					//if(m_wrong_direction_cnt[nport][lane] >10)
					//{
					//	HDMI_EMG("[Disp end]wrong_direction over, nport=%d, lane=%d, m_wrong_direction_cnt[nport][lane] =%d\n", nport, lane, m_wrong_direction_cnt[nport][lane] );
					//	m_wrong_direction_cnt[nport][lane] =0;
					//	phy_st[nport].disp_adp_stage[lane] = 2;  //end
					//}

					phy_st[nport].disp_BER1[lane]= (phy_st[nport].disp_err1[lane] - phy_st[nport].disp_err0[lane])*10000/(TimeStamp1[nport][lane]-TimeStamp0[nport][lane]);
					phy_st[nport].disp_BER2_temp[lane]= (phy_st[nport].disp_err2[lane] - phy_st[nport].disp_err1[lane])*10000/(TimeStamp2-TimeStamp1[nport][lane]);
					phy_st[nport].disp_BER2[lane]=phy_st[nport].disp_BER2_temp[lane];

					if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD21_PHY_LOG_ENABLE ))
					{
						HDMI_PRINTF("[disp adp]lane=%d, disp_BER1=%d, disp_BER2_temp=%d, disp_err2=%d, disp_err1=%d, disp_err0=%d,TimeStamp2=%d, TimeStamp1=%d, TimeStamp0=%d\n", 
						lane, 
						phy_st[nport].disp_BER1[lane],
						phy_st[nport].disp_BER2_temp[lane],
						phy_st[nport].disp_err2[lane],
						phy_st[nport].disp_err1[lane],
						phy_st[nport].disp_err0[lane],
						TimeStamp2,
						TimeStamp1[nport][lane],
						TimeStamp0[nport][lane]);
					}
					//if((GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1 , HDMI_FLOW_CFG2_CTS_EXTEND_MODE) == TRUE))
					if((lt_fsm_status[nport].tx_flt_no_train == TRUE)&&(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1 , HDMI_FLOW_CFG2_CTS_EXTEND_MODE) == TRUE))
					{

						if(m_scdc_rst_handle_once && (scdc_extend[nport]==FALSE))// 850ms flag ==TRUE
						{
							if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD21_PHY_LOG_ENABLE ))
							{
								HDMI_EMG("[disp adp stage1] m_scdc_rst_handle_once= %d  disp_err_zero_cnt=%d  port[%d]lane:%d\n",m_scdc_rst_handle_once,phy_st[nport].disp_err_zero_cnt[lane],nport,lane);
							}
							if(phy_st[nport].disp_BER2[lane] > 0)
							{
								HDMI_EMG("Start Extend tuning process: @ port[%d]lane:%d  disp_BER2=%d \n",nport,lane,phy_st[nport].disp_BER2[lane]);
								// Call MAC_API1 Here;
								newbase_hdmi_restart_scdc_extend(nport);
								scdc_extend[nport]=TRUE;
							}

						}	
					}
					//Check for Stop tuning condition
					if (phy_st[nport].disp_err_zero_cnt[lane] >=10000) 
					{
						//phy_st[nport].disp_adp_stage[lane] = 2;//end Marked for keep on tuing  never stop 
						phy_st[nport].disp_err_zero_cnt[lane] = 10000;//Set to Maxmum error cnt protect
						if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD21_PHY_LOG_ENABLE ))
						{
							HDMI_PRINTF("[disp adp stage1] lane=%d, disp_err_zero_cnt %d over(tap1=%d)\n",lane,phy_st[nport].disp_err_zero_cnt[lane] , phy_st[nport].dfe_t[lane].tap1);
						}
					}


					if(phy_st[nport].disp_BER2[lane] < 300)
					{
						phy_st[nport].disp_BER_zero_cnt[lane] = phy_st[nport].disp_BER_zero_cnt[lane] +1;
						
						if(phy_st[nport].disp_BER_zero_cnt[lane]>30)
						{
							if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD21_PHY_LOG_ENABLE ))
							{
								HDMI_PRINTF("[disp stage1] le < 300 le& tap2 adj - Inverse step=%d (nport=%d, lane=%d le=%d, tap2=%d, m_wrong_direction_cnt=%d)\n",phy_st[nport].le_adp_step[lane], nport, lane,phy_st[nport].dfe_t[lane].le, phy_st[nport].dfe_t[lane].tap2,m_wrong_direction_cnt[nport][lane]);
							}
							if(adj_le_done[nport][lane]==FALSE)
							{
								adj_le_done[nport][lane]=TRUE;
								enable_adj_tap1[nport][lane]= TRUE;
								m_wrong_direction_cnt[nport][lane]=0;
							}
							phy_st[nport].disp_BER_zero_cnt[lane] = 0;

						}
					}
					else
					{
						phy_st[nport].disp_BER_zero_cnt[lane] = 0;
					}


					//Dummy now for No tap1_done case
					if((adj_le_done[nport][lane]==TRUE)&&(adj_tap1_done[nport][lane]==TRUE))
					{
						phy_st[nport].disp_adp_stage[lane] = 2;//end
						phy_st[nport].disp_err_zero_cnt[lane] = 0;
						HDMI_PRINTF("[disp End]ADP Done  lane=%d, disp_err_zero_cnt %d over(tap1=%d)\n",lane,phy_st[nport].disp_err_zero_cnt[lane] , phy_st[nport].dfe_t[lane].tap1);
					}

	
					if (phy_st[nport].disp_adp_stage[lane] == 2) 
					{  //end
						HDMI_PRINTF("[disp End] (lane=%d tap1=%d) tap2=%d)\n",lane,phy_st[nport].dfe_t[lane].tap1, phy_st[nport].dfe_t[lane].tap2);
						continue;
					}
					else if (phy_st[nport].disp_adp_stage[lane] == 1)
					{
						#if 0  //move to before stop_tuning_flow
						phy_st[nport].disp_BER1[lane]= (phy_st[nport].disp_err1[lane] - phy_st[nport].disp_err0[lane])*10000/(TimeStamp1[nport][lane]-TimeStamp0[nport][lane]);
						phy_st[nport].disp_BER2_temp[lane]= (phy_st[nport].disp_err2[lane] - phy_st[nport].disp_err1[lane])*10000/(TimeStamp2-TimeStamp1[nport][lane]);
						HDMI_PRINTF("[disp adp stage1]lane=%d, disp_BER1=%d, disp_BER2_temp=%d, disp_err2=%d, disp_err1=%d, disp_err0=%d,TimeStamp2=%d, TimeStamp1=%d, TimeStamp0=%d\n", 
							lane, 
							phy_st[nport].disp_BER1[lane],
							phy_st[nport].disp_BER2_temp[lane],
							phy_st[nport].disp_err2[lane],
							phy_st[nport].disp_err1[lane],
							phy_st[nport].disp_err0[lane],
							TimeStamp2,
							TimeStamp1[nport][lane],
							TimeStamp0[nport][lane]);
						#endif
						if(((phy_st[nport].disp_BER1[lane]>100000))&&(acdr_finetune_en_toggle_once[nport][lane]==FALSE))
						{
							HDMI_EMG("Too much error @ port[%d]lane:%d\n",nport,lane);
							HDMI_EMG("hdmi21_set_phy again \n");
							newbase_hdmi21_set_phy(PHY_PROC_INIT, phy_st[PHY_PROC_INIT].clk,phy_st[PHY_PROC_INIT].frl_mode,phy_st[PHY_PROC_INIT].lane_num);
							newbase_hdmi_reset_meas_counter(PHY_PROC_INIT);

							acdr_finetune_en_toggle_once[nport][lane]=TRUE;
						}
						else
						{
							acdr_finetune_en_toggle_once[nport][lane]=TRUE;//No need toggle in this adj

						}
						
						//if((phy_st[nport].disp_err2[lane] - phy_st[nport].disp_err1[lane]) <= 1)	//Clayton: orig: err2=err1 / new: err2-err1 <= 1, 1is acceptable
						if((phy_st[nport].disp_err2[lane] - phy_st[nport].disp_err1[lane]) == 0)	
						{
							if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD21_PHY_LOG_ENABLE ))
							{
								HDMI_PRINTF("Almost no error increased , error @ port[%d]lane:%d, err2= %d, err1= %d\n",nport,lane, phy_st[nport].disp_err2[lane], phy_st[nport].disp_err1[lane]);
							}
							phy_st[nport].disp_err_zero_cnt[lane] = phy_st[nport].disp_err_zero_cnt[lane] + 1;					


							phy_st[nport].disp_start = 2;
							//phy_st[nport].disp_err0[lane]=phy_st[nport].disp_err1[lane];
							//phy_st[nport].disp_err1[lane]=phy_st[nport].disp_err2[lane];
							//TimeStamp0[nport][lane]=TimeStamp1[nport][lane];
							//TimeStamp1[nport][lane]=TimeStamp2;

						}
						else
						{
							//phy_st[nport].disp_BER2[lane]=phy_st[nport].disp_BER2_temp[lane];

							//if ((phy_st[nport].disp_BER2[lane]<= phy_st[nport].disp_BER1[lane]) || ((phy_st[nport].disp_err_zero_cnt[lane] >=3) && (phy_st[nport].disp_BER2[lane] < 100))||(adj_parameter_changed[nport][lane]== TRUE))
							if ((phy_st[nport].disp_BER2[lane]<= phy_st[nport].disp_BER1[lane])||(adj_parameter_changed[nport][lane]== TRUE))
							{
								if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD21_PHY_LOG_ENABLE ))
								{
									HDMI_PRINTF("[disp stage1]  lane=%d, disp_err_zero_cnt %d  disp_BER2[lane]=%d)\n",lane,phy_st[nport].disp_err_zero_cnt[lane] , phy_st[nport].disp_BER2[lane]);
								}
								if (adj_parameter_changed[nport][lane]== TRUE)
								{
									adj_parameter_changed[nport][lane]= FALSE;
								}

								phy_st[nport].disp_err_zero_cnt[lane]=0; //Clayton

								if((enable_adj_tap1[nport][lane]==TRUE) || (adj_le_done[nport][lane]==TRUE)) //clayton
								{
									
									phy_st[nport].dfe_t[lane].tap1 = phy_st[nport].dfe_t[lane].tap1 +2* phy_st[nport].tap1_adp_step[lane];
									if(phy_st[nport].dfe_t[lane].tap1<0)
									{
										phy_st[nport].dfe_t[lane].tap1 = 0;
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										if(adj_le_done[nport][lane]==FALSE)
										{
											enable_adj_tap1[nport][lane]= FALSE;//change  to LE adj
											adj_parameter_changed[nport][lane]= TRUE;
										}
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].tap1_adp_step[lane] = - 1 *phy_st[nport].tap1_adp_step[lane];
										HDMI_EMG("check tap1 hit boundary, lane = %d, tap1 = %d\n", lane, phy_st[nport].dfe_t[lane].tap1);			
									}
									else if(phy_st[nport].dfe_t[lane].tap1>TAP1_MAX_VALUE)
									{
										phy_st[nport].dfe_t[lane].tap1 = TAP1_MAX_VALUE;
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										if(adj_le_done[nport][lane]==FALSE)
										{
											enable_adj_tap1[nport][lane]= FALSE;//change  to LE adj
											adj_parameter_changed[nport][lane]= TRUE;
										}
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].tap1_adp_step[lane] = - 1 *phy_st[nport].tap1_adp_step[lane];
										HDMI_EMG("check tap1 hit boundary, lane = %d, tap1 = %d\n", lane, phy_st[nport].dfe_t[lane].tap1);			
									}
									if(adj_tap1_done[nport][lane]==FALSE)
									{
										lib_hdmi_dfe_init_tap1(nport,(1<<lane),phy_st[nport].dfe_t[lane].tap1);
										HDMI_PRINTF("[disp stage1] tap1 adj (lane=%d tap1=%d)\n",lane,phy_st[nport].dfe_t[lane].tap1);

									}


								}
								else //adj le
								{
								
									phy_st[nport].dfe_t[lane].le = phy_st[nport].dfe_t[lane].le + phy_st[nport].le_adp_step[lane];

									if(phy_st[nport].dfe_t[lane].le<0)
									{
										phy_st[nport].dfe_t[lane].le = 0;
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										if(adj_tap1_done[nport][lane]==FALSE)
										{
											enable_adj_tap1[nport][lane]= TRUE;//change  to TAP1 adj
											adj_parameter_changed[nport][lane]= TRUE;
										}
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].le_adp_step[lane] = - 1 *phy_st[nport].le_adp_step[lane];
										phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];
										HDMI_EMG("check le hit boundary, lane = %d, le = %d\n", lane, phy_st[nport].dfe_t[lane].le);			
									}
									else if(phy_st[nport].dfe_t[lane].le>LE_MAX_VALUE)
									{

										phy_st[nport].dfe_t[lane].le = LE_MAX_VALUE;
										if(adj_tap1_done[nport][lane]==FALSE)
										{
											enable_adj_tap1[nport][lane]= TRUE;//change  to TAP1 adj
											adj_parameter_changed[nport][lane]= TRUE;
										}
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].le_adp_step[lane] = - 1 *phy_st[nport].le_adp_step[lane];
										phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];
										HDMI_EMG(" check le hit boundary, lane = %d, le = %d\n", lane, phy_st[nport].dfe_t[lane].le);			
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
									}
									else    //Change Tap2 value only when LE not in boundary condition
									{
										phy_st[nport].dfe_t[lane].tap2 = phy_st[nport].dfe_t[lane].tap2 +2* phy_st[nport].tap2_adp_step[lane];
										if(phy_st[nport].dfe_t[lane].tap2<-31)
										{
											phy_st[nport].dfe_t[lane].tap2 = -31;
											//phy_st[nport].disp_adp_stage[lane] = 3;  // end
											//enable_adj_tap1[nport][lane]= TRUE;//change  to TAP1 adj
											HDMI_EMG("check le hit boundary, lane = %d, tap2= %d\n", lane, phy_st[nport].dfe_t[lane].tap2);			
										}
										else if(phy_st[nport].dfe_t[lane].tap2>31)
										{
											//enable_adj_tap1[nport][lane]= TRUE;	//Clayton: fix coding wrong
											phy_st[nport].dfe_t[lane].tap2 = 31;
											HDMI_EMG("check le hit boundary, lane = %d, tap2 = %d\n", lane, phy_st[nport].dfe_t[lane].tap2);			
											//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										}

									}

									if(adj_le_done[nport][lane]==FALSE)
									{
										lib_hdmi_dfe_init_le(nport,(1<<lane),phy_st[nport].dfe_t[lane].le);
										lib_hdmi_dfe_init_tap2(nport,(1<<lane),phy_st[nport].dfe_t[lane].tap2);										
										if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD21_PHY_LOG_ENABLE ))
										{
											HDMI_PRINTF("[disp stage1] le & tap2 adj (lane=%d le=%d tap2=%d)\n",lane,phy_st[nport].dfe_t[lane].le,phy_st[nport].dfe_t[lane].tap2);
										}
									}

									// Clayton: dummy case, //Clayton: fix coding wrong
									/*

									if (phy_st[nport].dfe_t[lane].le > LE_MAX_VALUE)
									{
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										enable_adj_tap1[nport][lane]= TRUE;
										
									}
									*/

								}
								
								phy_st[nport].disp_start = 2;
								phy_st[nport].disp_err0[lane]=phy_st[nport].disp_err1[lane];
								phy_st[nport].disp_err1[lane]=phy_st[nport].disp_err2[lane];
								TimeStamp0[nport][lane]=TimeStamp1[nport][lane];
								TimeStamp1[nport][lane]=TimeStamp2;

							}
							else if (phy_st[nport].disp_BER2[lane]>phy_st[nport].disp_BER1[lane])
							{ 
								phy_st[nport].disp_err_zero_cnt[lane]=0;	//Clayton

								if((enable_adj_tap1[nport][lane]==TRUE) || (adj_le_done[nport][lane]==TRUE)) //clayton
								{
									phy_st[nport].tap1_adp_step[lane] = - 1 *phy_st[nport].tap1_adp_step[lane];
									phy_st[nport].dfe_t[lane].tap1 = phy_st[nport].dfe_t[lane].tap1 + 2*phy_st[nport].tap1_adp_step[lane];
									m_wrong_direction_cnt[nport][lane] ++;
									if(m_wrong_direction_cnt[nport][lane]>1 )
									{
										m_wrong_direction_cnt[nport][lane]=0;
										enable_adj_tap1[nport][lane]= FALSE;//change  to LE adj
										adj_parameter_changed[nport][lane]= TRUE;
									}
									
									if(phy_st[nport].dfe_t[lane].tap1<0)
									{
										phy_st[nport].dfe_t[lane].tap1 = 0;
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										if(adj_le_done[nport][lane]==FALSE)
										{
											enable_adj_tap1[nport][lane]= FALSE;//change  to LE adj
											adj_parameter_changed[nport][lane]= TRUE;
										}
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].tap1_adp_step[lane] = - 1 *phy_st[nport].tap1_adp_step[lane];
										HDMI_EMG("check tap1 hit boundary, lane = %d, tap1 = %d\n", lane, phy_st[nport].dfe_t[lane].tap1);			
									}
									else if(phy_st[nport].dfe_t[lane].tap1>TAP1_MAX_VALUE)
									{
										phy_st[nport].dfe_t[lane].tap1 = TAP1_MAX_VALUE;
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										if(adj_le_done[nport][lane]==FALSE)
										{
											enable_adj_tap1[nport][lane]= FALSE;//change  to LE adj
											adj_parameter_changed[nport][lane]= TRUE;
										}
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].tap1_adp_step[lane] = - 1 *phy_st[nport].tap1_adp_step[lane];
										HDMI_EMG("check tap1 hit boundary, lane = %d, tap1 = %d\n", lane, phy_st[nport].dfe_t[lane].tap1);			
									}
									if(adj_tap1_done[nport][lane]==FALSE)
									{
										lib_hdmi_dfe_init_tap1(nport,(1<<lane),phy_st[nport].dfe_t[lane].tap1);
										if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD21_PHY_LOG_ENABLE ))
										{
											HDMI_PRINTF("[disp stage1] tap1 adj - Inverse step=%d (nport=%d, lane=%d tap1=%d, m_wrong_direction_cnt=%d)\n",phy_st[nport].tap1_adp_step[lane], nport, lane,phy_st[nport].dfe_t[lane].tap1,  m_wrong_direction_cnt[nport][lane]);
										}

										if ((phy_st[nport].disp_BER2[lane] <25) )
										{
											//adj_tap1_done[nport][lane]=TRUE;
											HDMI_PRINTF("[disp stage1] disp_BER2<25, tap1 adj ctn (lane=%d le=%d)\n",lane,phy_st[nport].dfe_t[lane].tap1);
											//enable_adj_tap1[nport][lane]= FALSE;
											//m_wrong_direction_cnt[nport][lane]=0;
										}
									}

								}
								else //adj le
								{
									phy_st[nport].le_adp_step[lane] = - 1 *phy_st[nport].le_adp_step[lane];
									
									phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];

									phy_st[nport].dfe_t[lane].le = phy_st[nport].dfe_t[lane].le + phy_st[nport].le_adp_step[lane];

									m_wrong_direction_cnt[nport][lane] ++;
									if(m_wrong_direction_cnt[nport][lane]>1 )
									{
										m_wrong_direction_cnt[nport][lane]=0;
										enable_adj_tap1[nport][lane]= TRUE;//change  to TPA1 adj
										adj_parameter_changed[nport][lane]= TRUE;
									}

									if(phy_st[nport].dfe_t[lane].le<0)
									{
										phy_st[nport].dfe_t[lane].le = 0;
										if(adj_tap1_done[nport][lane]==FALSE)
										{
											enable_adj_tap1[nport][lane]= TRUE;//change  to TAP1 adj
											adj_parameter_changed[nport][lane]= TRUE;
										}										
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].le_adp_step[lane] = - 1 *phy_st[nport].le_adp_step[lane];
										phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];
										HDMI_EMG("check le hit boundary, lane = %d, le = %d\n", lane, phy_st[nport].dfe_t[lane].le);			
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
									}
									else if(phy_st[nport].dfe_t[lane].le>LE_MAX_VALUE)	//Clayton
									{
										phy_st[nport].dfe_t[lane].le = LE_MAX_VALUE;	//Clayton: fix coding wrong
										m_wrong_direction_cnt[nport][lane]=0;
										if(adj_tap1_done[nport][lane]==FALSE)
										{
											enable_adj_tap1[nport][lane]= TRUE;//change  to TAP1 adj
											adj_parameter_changed[nport][lane]= TRUE;
										}										
										phy_st[nport].le_adp_step[lane] = - 1 *phy_st[nport].le_adp_step[lane];
										phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];
										HDMI_EMG("check le hit boundary, lane = %d, le = %d\n", lane, phy_st[nport].dfe_t[lane].le);			
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
									}
									else     //Change Tap2 value only when LE not in boundary condition
									{
										phy_st[nport].dfe_t[lane].tap2 = phy_st[nport].dfe_t[lane].tap2 +2* phy_st[nport].tap2_adp_step[lane];
										if(phy_st[nport].dfe_t[lane].tap2<-31)
										{
											phy_st[nport].dfe_t[lane].tap2 = -31;
											//phy_st[nport].disp_adp_stage[lane] = 3;  // end
											//enable_adj_tap1[nport][lane]= TRUE;//change  to TAP1 adj
											HDMI_EMG("check le hit boundary, lane = %d, tap2 = %d\n", lane, phy_st[nport].dfe_t[lane].tap2);			
										}
										else if(phy_st[nport].dfe_t[lane].tap2>31)
										{
											//enable_adj_tap1[nport][lane]= TRUE;	//Clayton: fix coding wrong
											phy_st[nport].dfe_t[lane].tap2 = 31;
											HDMI_EMG("check le hit boundary, lane = %d, tap2 = %d\n", lane, phy_st[nport].dfe_t[lane].tap2);			
											//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										}
									}

									if(adj_le_done[nport][lane]==FALSE)
									{
										lib_hdmi_dfe_init_le(nport,(1<<lane),phy_st[nport].dfe_t[lane].le);
										lib_hdmi_dfe_init_tap2(nport,(1<<lane),phy_st[nport].dfe_t[lane].tap2);										
										if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD21_PHY_LOG_ENABLE ))
										{
											HDMI_PRINTF("[disp stage1] le& tap2 adj - Inverse step=%d (nport=%d, lane=%d le=%d, tap2=%d, m_wrong_direction_cnt=%d)\n",phy_st[nport].le_adp_step[lane], nport, lane,phy_st[nport].dfe_t[lane].le, phy_st[nport].dfe_t[lane].tap2,m_wrong_direction_cnt[nport][lane]);
										}
									}

								}

								phy_st[nport].disp_start = 2;
								phy_st[nport].disp_err0[lane]=phy_st[nport].disp_err1[lane];
								phy_st[nport].disp_err1[lane]=phy_st[nport].disp_err2[lane];
								TimeStamp0[nport][lane]=TimeStamp1[nport][lane];
								TimeStamp1[nport][lane]=TimeStamp2;
							}


						}

					}

				}

				//if((GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1 , HDMI_FLOW_CFG2_CTS_EXTEND_MODE) == TRUE))
				if((lt_fsm_status[nport].tx_flt_no_train == TRUE)&&(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1 , HDMI_FLOW_CFG2_CTS_EXTEND_MODE) == TRUE))
				{
					if(m_scdc_rst_handle_once && (scdc_extend[nport]==TRUE))// 850ms flag ==TRUE
					{

						if (phy_st[nport].lane_num == HDMI_4LANE)
						{
							ber_sum[nport]= phy_st[nport].disp_BER2[0]+phy_st[nport].disp_BER2[1]+phy_st[nport].disp_BER2[2]+phy_st[nport].disp_BER2[3];
						}
						else
						{
							ber_sum[nport]= phy_st[nport].disp_BER2[0]+phy_st[nport].disp_BER2[1]+phy_st[nport].disp_BER2[2];

						}
						if(ber_sum[nport] <1)
						{
							ber_sum_cnt[nport]=ber_sum_cnt[nport]+1;
							if(ber_sum_cnt[nport]>5)
							{
								HDMI_EMG("End Extend tuning process: @ port[%d] ber_sum_cnt=%d \n",nport, ber_sum_cnt[nport]);
								// Call MAC_API2 Here;
								newbase_hdmi_force_stop_scdc_extend(nport);
								ber_sum_cnt[nport]=0;
								scdc_extend[nport]=FALSE;
							}

						}
						else
						{
							ber_sum_cnt[nport]=0;
						}
						
					}

				}
				if (phy_st[nport].lane_num == HDMI_4LANE)
				{
					if ((phy_st[nport].disp_adp_stage[0] == 2) & (phy_st[nport].disp_adp_stage[1] == 2) & (phy_st[nport].disp_adp_stage[2] == 2) & (phy_st[nport].disp_adp_stage[3] == 2))
					{
						phy_st[nport].disp_start = 3; //ending
						HDMI_EMG("[disp stage2] port=%d, 4 lane done, dfe tap1(0,1,2,3)=(%d,%d,%d,%d)\\n",
							nport, phy_st[nport].dfe_t[0].tap1, phy_st[nport].dfe_t[1].tap1, phy_st[nport].dfe_t[2].tap1, phy_st[nport].dfe_t[3].tap1);
					}
				}
				else
				{
					if ((phy_st[nport].disp_adp_stage[0] == 2) & (phy_st[nport].disp_adp_stage[1] == 2) & (phy_st[nport].disp_adp_stage[2] == 2))  
					{
						phy_st[nport].disp_start = 3; //ending
						HDMI_EMG("[disp stage2] port=%d, 3 lane done, dfe tap1(0,1,2)=(%d,%d,%d)\\n",
							nport, phy_st[nport].dfe_t[0].tap1, phy_st[nport].dfe_t[1].tap1, phy_st[nport].dfe_t[2].tap1);
					}	
				}

			}
			else 
			{
				phy_st[nport].disp_timer_cnt[lane]--;
			}
		
		}
		else if(phy_st[nport].disp_start==3)
		{//End lib_hdmi21_phy_error_handling
			if((lt_fsm_status[nport].tx_flt_no_train == TRUE)&&(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1 , HDMI_FLOW_CFG2_CTS_EXTEND_MODE) == TRUE))
			//if((GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1 , HDMI_FLOW_CFG2_CTS_EXTEND_MODE) == TRUE))
			{//to do 
				newbase_hdmi_scdc_hw_mask_enable(nport, 0);
				HDMI_PRINTF("[disp stage3] port=%d, Adjust finished, close extend\n", nport);
				phy_st[nport].disp_start =4;
			}
		}
		else 
		{
			//phy_st[nport].disp_start == 3 do nothing

		}
	}


}

void lib_hdmi20_phy_error_handling(unsigned char nport)
{
#if 0
	unsigned int current_time_ms = hdmi_get_system_time_ms();
	#define TAP1_MAX_VALUE	31
	#define TAP2_MAX_VALUE	31
	#define LE_MAX_VALUE	30
	int lane = 0;
	unsigned char mode;

	mode = DFE_P0_REG_DFE_CTRL_GB_P0_get_dfe_adapt_mode_g_1_0(hdmi_in(REG_DFE_CTRL_GB_reg));
	if ((newbase_hdmi_get_power_saving_state(nport)==PS_FSM_POWER_SAVING_ON))     // do not do power saving when HDMI is power off
		return ;

	for (lane=0; lane< 3; lane++) {
		phy_st[nport].dfe_t[lane].tap1 = lib_hdmi_dfe_get_tap1(nport, lane);
		phy_st[nport].dfe_t[lane].tap2 = lib_hdmi_dfe_get_tap2(nport, lane);
		phy_st[nport].dfe_t[lane].le = lib_hdmi_dfe_get_le(nport, lane);
		phy_st[nport].dfe_t[lane].lemax = lib_hdmi_dfe_get_lemax(nport, lane);
		phy_st[nport].dfe_t[lane].lemin = lib_hdmi_dfe_get_lemin(nport, lane);
	}
	
	if (mode == 2) 
	{
		if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE ))
			HDMI_PRINTF("[disp init]disp_start=%d, Current Time(ms)=%d\n",phy_st[nport].disp_start, current_time_ms);			


		if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
		{
			newbase_hdmi21_dump_dfe_para(nport,phy_st[nport].lane_num);
		}
		
		if (phy_st[nport].disp_start == 0) 
		{
			
			for (lane=0; lane< 3; lane++) {
				phy_st[nport].dfe_t[lane].tap1 = lib_hdmi_dfe_get_tap1(nport, lane);
				phy_st[nport].dfe_t[lane].tap2 = lib_hdmi_dfe_get_tap2(nport, lane);
				phy_st[nport].dfe_t[lane].le = lib_hdmi_dfe_get_le(nport, lane);


				lib_hdmi_dfe_init_le(nport,(1<<lane),phy_st[nport].dfe_t[lane].le);

				phy_st[nport].dfe_t[lane].tap1 = lib_hdmi_dfe_get_tap1(nport, lane);
				phy_st[nport].dfe_t[lane].tap2 = lib_hdmi_dfe_get_tap2(nport, lane);
				phy_st[nport].dfe_t[lane].le = lib_hdmi_dfe_get_le(nport, lane);
				HDMI_PRINTF("[disp init]ADD LE tdisp_start=%d, phy_st[nport].dfe_t[lane].le=%d\n",phy_st[nport].disp_start, phy_st[nport].dfe_t[lane].le);			
			}

			newbase_hdmi_fw_ced_measure(nport, phy_st[nport].ced_err0, 60);

			phy_st[nport].disp_start = 1;
			phy_st[nport].disp_adp_stage[0]  = phy_st[nport].disp_adp_stage[1]  = phy_st[nport].disp_adp_stage[2] = phy_st[nport].disp_adp_stage[3]  = 1;
			phy_st[nport].disp_timer_cnt[0] = phy_st[nport].disp_timer_cnt[1]= phy_st[nport].disp_timer_cnt[2]= phy_st[nport].disp_timer_cnt[3] = 5;  //100ms
		}
		else if (phy_st[nport].disp_start == 1)
		{

			if (phy_st[nport].disp_timer_cnt[lane] == 0)
			{  //near 100ms
				newbase_hdmi_fw_ced_measure(nport, phy_st[nport].ced_err1, 60);

				phy_st[nport].disp_start = 2;
				phy_st[nport].disp_adp_stage[0]  = phy_st[nport].disp_adp_stage[1]  = phy_st[nport].disp_adp_stage[2] = phy_st[nport].disp_adp_stage[3]  = 1;
				phy_st[nport].disp_timer_cnt[0] = phy_st[nport].disp_timer_cnt[1]= phy_st[nport].disp_timer_cnt[2]= phy_st[nport].disp_timer_cnt[3] = 5;  //100ms
			}
			else 
			{
				phy_st[nport].disp_timer_cnt[lane]--;
			}
	
		}
		else if (phy_st[nport].disp_start == 2)
		{
			if (phy_st[nport].disp_timer_cnt[lane] == 0) 
			{  //near 100ms
				//lib_hdmi_hd21_fw_disparity_error_status(nport,phy_st[nport].disp_err2);
				newbase_hdmi_fw_ced_measure(nport, phy_st[nport].ced_err2, 60);
				for (lane = 0; lane < phy_st[nport].lane_num ; lane++) 
				{

					if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
					{
						HDMI_PRINTF("[disp ing]lane=%d, disp_adp_stage[lane]=%d, m_wrong_direction_cnt=%d, disp_err_zero_cnt[lane]=%d \n",lane,
						phy_st[nport].disp_adp_stage[lane], m_wrong_direction_cnt[nport][lane], phy_st[nport].disp_err_zero_cnt[lane] );

						//if(m_wrong_direction_cnt[nport][lane] >10)
						//{
						//	HDMI_EMG("[Disp end]wrong_direction over, nport=%d, lane=%d, m_wrong_direction_cnt[nport][lane] =%d\n", nport, lane, m_wrong_direction_cnt[nport][lane] );
						//	m_wrong_direction_cnt[nport][lane] =0;
						//	phy_st[nport].disp_adp_stage[lane] = 2;  //end
						//}

						HDMI_PRINTF("[disp adp]lane=%d,  ced_err2=%d, ced_err1=%d, ced_err0=%d\n", 
						lane, 
						phy_st[nport].ced_err2[lane],
						phy_st[nport].ced_err1[lane],
						phy_st[nport].ced_err0[lane]);
					}

					//Check for Stop tuning condition
					if (phy_st[nport].disp_err_zero_cnt[lane] >=10000) 
					{
						//phy_st[nport].disp_adp_stage[lane] = 2;//end Marked for keep on tuing  never stop 
						phy_st[nport].disp_err_zero_cnt[lane] = 10000;//Set to Maxmum error cnt protect
						HDMI_PRINTF("[disp adp stage1] lane=%d, disp_err_zero_cnt %d over(tap1=%d)\n",lane,phy_st[nport].disp_err_zero_cnt[lane] , phy_st[nport].dfe_t[lane].tap1);
					}

					//Dummy now for No tap1_done case
					if((adj_le_done[nport][lane]==TRUE)&&(adj_tap1_done[nport][lane]==TRUE))
					{
						phy_st[nport].disp_adp_stage[lane] = 2;//end
						phy_st[nport].disp_err_zero_cnt[lane] = 0;
						HDMI_PRINTF("[disp End]ADP Done  lane=%d, disp_err_zero_cnt %d over(tap1=%d)\n",lane,phy_st[nport].disp_err_zero_cnt[lane] , phy_st[nport].dfe_t[lane].tap1);
					}

	
					if (phy_st[nport].disp_adp_stage[lane] == 2) 
					{  //end
						HDMI_PRINTF("[disp End] (lane=%d tap1=%d) tap2=%d)\n",lane,phy_st[nport].dfe_t[lane].tap1, phy_st[nport].dfe_t[lane].tap2);
						continue;
					}
					else if (phy_st[nport].disp_adp_stage[lane] == 1)
					{

					#if HDMI20_PHY_ERROR_CORRECTION_EN // le tap1
						if(((phy_st[nport].ced_err2[lane] ) <= 10)||(HDMI_ABS(phy_st[nport].ced_err2[lane],phy_st[nport].ced_err1[lane])<10))	
						{
							if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
							{
								HDMI_PRINTF("Almost no error increased , error @ port[%d]lane:%d, err2= %d, err1= %d\n",nport,lane, phy_st[nport].ced_err2[lane], phy_st[nport].ced_err1[lane]);
							}
							phy_st[nport].disp_err_zero_cnt[lane] = phy_st[nport].disp_err_zero_cnt[lane] + 1;					
							phy_st[nport].disp_start = 2;
						}
						else
						{
							//if ((phy_st[nport].ced_err2[lane]<= phy_st[nport].ced_err1[lane])||(adj_parameter_changed[nport][lane]== TRUE))
							//if ((phy_st[nport].ced_err2[lane]< phy_st[nport].ced_err1[lane])&&((phy_st[nport].ced_err1[lane]- phy_st[nport].ced_err2[lane])>10))
							//if ((phy_st[nport].ced_err2[lane]<= phy_st[nport].ced_err1[lane])||(adj_parameter_changed[nport][lane]== TRUE))
							if ((phy_st[nport].ced_err2[lane]<=phy_st[nport].ced_err1[lane])&&((phy_st[nport].ced_err1[lane]- phy_st[nport].ced_err2[lane])>20))
							//if (phy_st[nport].ced_err2[lane]< phy_st[nport].ced_err1[lane])
							{

								if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
								{
									HDMI_PRINTF("[disp stage1]  lane=%d, disp_err_zero_cnt %d  ced_err2[lane]=%d)\n",lane,phy_st[nport].disp_err_zero_cnt[lane] , phy_st[nport].ced_err2[lane]);
								}
								if (adj_parameter_changed[nport][lane]== TRUE)
								{
									adj_parameter_changed[nport][lane]= FALSE;
								}

								phy_st[nport].disp_err_zero_cnt[lane]=0; //

								if((enable_adj_tap1[nport][lane]==TRUE) || (adj_le_done[nport][lane]==TRUE)) //
								{
									
									phy_st[nport].dfe_t[lane].tap1 = phy_st[nport].dfe_t[lane].tap1 +2* phy_st[nport].tap1_adp_step[lane];
									if(phy_st[nport].dfe_t[lane].tap1<0)
									{
										phy_st[nport].dfe_t[lane].tap1 = 0;
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										if(adj_le_done[nport][lane]==FALSE)
										{
											enable_adj_tap1[nport][lane]= FALSE;//change  to LE adj
											adj_parameter_changed[nport][lane]= TRUE;
										}
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].tap1_adp_step[lane] = - 1 *phy_st[nport].tap1_adp_step[lane];
										HDMI_EMG("check tap1 hit boundary, lane = %d, tap1 = %d\n", lane, phy_st[nport].dfe_t[lane].tap1);			
									}
									else if(phy_st[nport].dfe_t[lane].tap1>TAP1_MAX_VALUE)
									{
										phy_st[nport].dfe_t[lane].tap1 = TAP1_MAX_VALUE;
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										if(adj_le_done[nport][lane]==FALSE)
										{
											enable_adj_tap1[nport][lane]= FALSE;//change  to LE adj
											adj_parameter_changed[nport][lane]= TRUE;
										}
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].tap1_adp_step[lane] = - 1 *phy_st[nport].tap1_adp_step[lane];
										HDMI_EMG("check tap1 hit boundary, lane = %d, tap1 = %d\n", lane, phy_st[nport].dfe_t[lane].tap1);			
									}
									if(adj_tap1_done[nport][lane]==FALSE)
									{
										lib_hdmi_dfe_init_tap1(nport,(1<<lane),phy_st[nport].dfe_t[lane].tap1);
										if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
										{
											HDMI_PRINTF("[disp stage1] tap1 adj (lane=%d tap1=%d)\n",lane,phy_st[nport].dfe_t[lane].tap1);
										}
									}

								}
								else //adj le
								{
								
									phy_st[nport].dfe_t[lane].le = phy_st[nport].dfe_t[lane].le + phy_st[nport].le_adp_step[lane];

									if(phy_st[nport].dfe_t[lane].le<0)
									{
										phy_st[nport].dfe_t[lane].le = 0;
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										if(adj_tap1_done[nport][lane]==FALSE)
										{
											enable_adj_tap1[nport][lane]= TRUE;//change  to TAP1 adj
											adj_parameter_changed[nport][lane]= TRUE;
										}
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].le_adp_step[lane] = - 1 *phy_st[nport].le_adp_step[lane];
										phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];
										HDMI_EMG("check le hit boundary, lane = %d, le = %d\n", lane, phy_st[nport].dfe_t[lane].le);			
									}
									else if(phy_st[nport].dfe_t[lane].le>LE_MAX_VALUE)
									//else if(phy_st[nport].dfe_t[lane].le>(phy_st[nport].dfe_t[lane].lemax+5))
									{

										phy_st[nport].dfe_t[lane].le = LE_MAX_VALUE;
										//phy_st[nport].dfe_t[lane].le = phy_st[nport].dfe_t[lane].lemax+5;
										if(adj_tap1_done[nport][lane]==FALSE)
										{
											enable_adj_tap1[nport][lane]= TRUE;//change  to TAP1 adj
											adj_parameter_changed[nport][lane]= TRUE;
										}
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].le_adp_step[lane] = - 1 *phy_st[nport].le_adp_step[lane];
										phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];
										HDMI_EMG(" check le hit boundary, lane = %d, le = %d\n", lane, phy_st[nport].dfe_t[lane].le);			
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
									}
									else    //Change Tap2 value only when LE not in boundary condition
									{
										phy_st[nport].dfe_t[lane].tap2 = phy_st[nport].dfe_t[lane].tap2 +2* phy_st[nport].tap2_adp_step[lane];
										if(phy_st[nport].dfe_t[lane].tap2<-31)
										{
											phy_st[nport].dfe_t[lane].tap2 = -31;
											//phy_st[nport].disp_adp_stage[lane] = 3;  // end
											//enable_adj_tap1[nport][lane]= TRUE;//change  to TAP1 adj
											HDMI_EMG("check le hit boundary, lane = %d, tap2= %d\n", lane, phy_st[nport].dfe_t[lane].tap2);			
										}
										else if(phy_st[nport].dfe_t[lane].tap2>31)
										{
											//enable_adj_tap1[nport][lane]= TRUE;	//
											phy_st[nport].dfe_t[lane].tap2 = 31;
											HDMI_EMG("check le hit boundary, lane = %d, tap2 = %d\n", lane, phy_st[nport].dfe_t[lane].tap2);			
											//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										}
									}
									if(adj_le_done[nport][lane]==FALSE)
									{
										lib_hdmi_dfe_init_le(nport,(1<<lane),phy_st[nport].dfe_t[lane].le);
										lib_hdmi_dfe_init_tap2(nport,(1<<lane),phy_st[nport].dfe_t[lane].tap2);										
										if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
										{
											HDMI_PRINTF("[disp stage1] le & tap2 adj (lane=%d le=%d tap2=%d)\n",lane,phy_st[nport].dfe_t[lane].le,phy_st[nport].dfe_t[lane].tap2);

										}
									}
								}
							}
							//else if (phy_st[nport].ced_err2[lane]>phy_st[nport].ced_err1[lane])
							else if ((phy_st[nport].ced_err2[lane]>phy_st[nport].ced_err1[lane])&&((phy_st[nport].ced_err2[lane]- phy_st[nport].ced_err1[lane])>20))
							{ 
								phy_st[nport].disp_err_zero_cnt[lane]=0;	//

								if((enable_adj_tap1[nport][lane]==TRUE) || (adj_le_done[nport][lane]==TRUE)) //
								{
									phy_st[nport].tap1_adp_step[lane] = - 1 *phy_st[nport].tap1_adp_step[lane];
									phy_st[nport].dfe_t[lane].tap1 = phy_st[nport].dfe_t[lane].tap1 + 2*phy_st[nport].tap1_adp_step[lane];
									m_wrong_direction_cnt[nport][lane] ++;
									if(m_wrong_direction_cnt[nport][lane]>1 )
									{
										m_wrong_direction_cnt[nport][lane]=0;
										enable_adj_tap1[nport][lane]= FALSE;//change  to LE adj
										adj_parameter_changed[nport][lane]= TRUE;
									}
									
									if(phy_st[nport].dfe_t[lane].tap1<0)
									{
										phy_st[nport].dfe_t[lane].tap1 = 0;
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										if(adj_le_done[nport][lane]==FALSE)
										{
											enable_adj_tap1[nport][lane]= FALSE;//change  to LE adj
											adj_parameter_changed[nport][lane]= TRUE;
										}
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].tap1_adp_step[lane] = - 1 *phy_st[nport].tap1_adp_step[lane];
										HDMI_EMG("check tap1 hit boundary, lane = %d, tap1 = %d\n", lane, phy_st[nport].dfe_t[lane].tap1);			
									}
									else if(phy_st[nport].dfe_t[lane].tap1>TAP1_MAX_VALUE)
									{
										phy_st[nport].dfe_t[lane].tap1 = TAP1_MAX_VALUE;
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										if(adj_le_done[nport][lane]==FALSE)
										{
											enable_adj_tap1[nport][lane]= FALSE;//change  to LE adj
											adj_parameter_changed[nport][lane]= TRUE;
										}
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].tap1_adp_step[lane] = - 1 *phy_st[nport].tap1_adp_step[lane];
										HDMI_EMG("check tap1 hit boundary, lane = %d, tap1 = %d\n", lane, phy_st[nport].dfe_t[lane].tap1);			
									}
									if(adj_tap1_done[nport][lane]==FALSE)
									{
										lib_hdmi_dfe_init_tap1(nport,(1<<lane),phy_st[nport].dfe_t[lane].tap1);
										if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
										{
											HDMI_PRINTF("[disp stage1] tap1 adj - Inverse step=%d (nport=%d, lane=%d tap1=%d, m_wrong_direction_cnt=%d)\n",phy_st[nport].tap1_adp_step[lane], nport, lane,phy_st[nport].dfe_t[lane].tap1,  m_wrong_direction_cnt[nport][lane]);
										}
										/*
										if ((phy_st[nport].ced_err2[lane] <25) )
										{
											//adj_tap1_done[nport][lane]=TRUE;
											HDMI_PRINTF("[disp stage1] ced_err2<25, tap1 adj ctn (lane=%d le=%d)\n",lane,phy_st[nport].dfe_t[lane].tap1);
											//enable_adj_tap1[nport][lane]= FALSE;
											//m_wrong_direction_cnt[nport][lane]=0;
										}*/
									}

								}
								else //adj le
								{
									phy_st[nport].le_adp_step[lane] = - 1 *phy_st[nport].le_adp_step[lane];
									
									phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];

									phy_st[nport].dfe_t[lane].le = phy_st[nport].dfe_t[lane].le + phy_st[nport].le_adp_step[lane];

									m_wrong_direction_cnt[nport][lane] ++;
									if(m_wrong_direction_cnt[nport][lane]>1 )
									{
										m_wrong_direction_cnt[nport][lane]=0;
										enable_adj_tap1[nport][lane]= TRUE;//change  to TPA1 adj
										adj_parameter_changed[nport][lane]= TRUE;
									}

									if(phy_st[nport].dfe_t[lane].le<0)
									{
										phy_st[nport].dfe_t[lane].le = 0;
										if(adj_tap1_done[nport][lane]==FALSE)
										{
											enable_adj_tap1[nport][lane]= TRUE;//change  to TAP1 adj
											adj_parameter_changed[nport][lane]= TRUE;
										}										
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].le_adp_step[lane] = - 1 *phy_st[nport].le_adp_step[lane];
										phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];
										HDMI_EMG("check le hit boundary, lane = %d, le = %d\n", lane, phy_st[nport].dfe_t[lane].le);			
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
									}
									 else if(phy_st[nport].dfe_t[lane].le>LE_MAX_VALUE)
									{

										phy_st[nport].dfe_t[lane].le = LE_MAX_VALUE;
										m_wrong_direction_cnt[nport][lane]=0;
										if(adj_tap1_done[nport][lane]==FALSE)
										{
											enable_adj_tap1[nport][lane]= TRUE;//change  to TAP1 adj
											adj_parameter_changed[nport][lane]= TRUE;
										}										
										phy_st[nport].le_adp_step[lane] = - 1 *phy_st[nport].le_adp_step[lane];
										phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];
										HDMI_EMG("check le hit boundary, lane = %d, le = %d\n", lane, phy_st[nport].dfe_t[lane].le);			
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
									}
									else     //Change Tap2 value only when LE not in boundary condition
									{
										phy_st[nport].dfe_t[lane].tap2 = phy_st[nport].dfe_t[lane].tap2 +2* phy_st[nport].tap2_adp_step[lane];
										if(phy_st[nport].dfe_t[lane].tap2<-31)
										{
											phy_st[nport].dfe_t[lane].tap2 = -31;
											//phy_st[nport].disp_adp_stage[lane] = 3;  // end
											//enable_adj_tap1[nport][lane]= TRUE;//change  to TAP1 adj
											HDMI_EMG("check le hit boundary, lane = %d, tap2 = %d\n", lane, phy_st[nport].dfe_t[lane].tap2);			
										}
										else if(phy_st[nport].dfe_t[lane].tap2>31)
										{
											//enable_adj_tap1[nport][lane]= TRUE;
											phy_st[nport].dfe_t[lane].tap2 = 31;
											HDMI_EMG("check le hit boundary, lane = %d, tap2 = %d\n", lane, phy_st[nport].dfe_t[lane].tap2);			
											//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										}
									}

									if(adj_le_done[nport][lane]==FALSE)
									{
										lib_hdmi_dfe_init_le(nport,(1<<lane),phy_st[nport].dfe_t[lane].le);
										lib_hdmi_dfe_init_tap2(nport,(1<<lane),phy_st[nport].dfe_t[lane].tap2);										
										if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
										{
											HDMI_PRINTF("[disp stage1] le& tap2 adj - Inverse step=%d (nport=%d, lane=%d le=%d, tap2=%d, m_wrong_direction_cnt=%d)\n",phy_st[nport].le_adp_step[lane], nport, lane,phy_st[nport].dfe_t[lane].le, phy_st[nport].dfe_t[lane].tap2,m_wrong_direction_cnt[nport][lane]);
										}
									}
								}
							}

						}
						#endif
					#if 0//HDMI20_PHY_ERROR_CORRECTION_EN //LE only
						//if(((phy_st[nport].ced_err2[lane] ) <= 10)||(HDMI_ABS(phy_st[nport].ced_err2[lane],phy_st[nport].ced_err1[lane])<10))	
						if((phy_st[nport].ced_err2[lane] ) <= 10)	
						{
							if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
							{
								HDMI_PRINTF("Almost no error increased , error @ port[%d]lane:%d, err2= %d, err1= %d\n",nport,lane, phy_st[nport].ced_err2[lane], phy_st[nport].ced_err1[lane]);
							}
							phy_st[nport].disp_err_zero_cnt[lane] = phy_st[nport].disp_err_zero_cnt[lane] + 1;					
							phy_st[nport].disp_start = 2;
						}
						else
						{
							//if ((phy_st[nport].ced_err2[lane]<= phy_st[nport].ced_err1[lane])||(adj_parameter_changed[nport][lane]== TRUE))
							if ((phy_st[nport].ced_err2[lane]<=phy_st[nport].ced_err1[lane])&&((phy_st[nport].ced_err1[lane]- phy_st[nport].ced_err2[lane])>20))
							//if (phy_st[nport].ced_err2[lane]< phy_st[nport].ced_err1[lane])
							{

								if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
								{
									HDMI_PRINTF("[disp stage1]  lane=%d, disp_err_zero_cnt %d  ced_err2[lane]=%d)\n",lane,phy_st[nport].disp_err_zero_cnt[lane] , phy_st[nport].ced_err2[lane]);
								}
								//if (adj_parameter_changed[nport][lane]== TRUE)
								//{
								//	adj_parameter_changed[nport][lane]= FALSE;
								//}

								phy_st[nport].disp_err_zero_cnt[lane]=0; //

								
								phy_st[nport].dfe_t[lane].le = phy_st[nport].dfe_t[lane].le + phy_st[nport].le_adp_step[lane];

								if(phy_st[nport].dfe_t[lane].le<0)
								{
									phy_st[nport].dfe_t[lane].le = 0;
									//phy_st[nport].disp_adp_stage[lane] = 3;  // end
									//if(adj_tap1_done[nport][lane]==FALSE)
									//{
										//enable_adj_tap1[nport][lane]= TRUE;//change  to TAP1 adj
										//adj_parameter_changed[nport][lane]= TRUE;
									//}
									m_wrong_direction_cnt[nport][lane]=0;
									phy_st[nport].le_adp_step[lane] = - 1 *phy_st[nport].le_adp_step[lane];
									phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];
									HDMI_EMG("check le hit boundary, lane = %d, le = %d\n", lane, phy_st[nport].dfe_t[lane].le);			
								}
								else if(phy_st[nport].dfe_t[lane].le>LE_MAX_VALUE)
								//else if(phy_st[nport].dfe_t[lane].le>(phy_st[nport].dfe_t[lane].lemax+5))
								{

									phy_st[nport].dfe_t[lane].le = LE_MAX_VALUE;
									//phy_st[nport].dfe_t[lane].le = phy_st[nport].dfe_t[lane].lemax+5;
									//if(adj_tap1_done[nport][lane]==FALSE)
									//{
										//enable_adj_tap1[nport][lane]= TRUE;//change  to TAP1 adj
										//adj_parameter_changed[nport][lane]= TRUE;
									//}
									m_wrong_direction_cnt[nport][lane]=0;
									phy_st[nport].le_adp_step[lane] = - 1 *phy_st[nport].le_adp_step[lane];
									phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];
									HDMI_EMG(" check le hit boundary, lane = %d, le = %d\n", lane, phy_st[nport].dfe_t[lane].le);			
									//phy_st[nport].disp_adp_stage[lane] = 3;  // end
								}
								else    //Change Tap2 value only when LE not in boundary condition
								{
									phy_st[nport].dfe_t[lane].tap2 = phy_st[nport].dfe_t[lane].tap2 +2* phy_st[nport].tap2_adp_step[lane];
									if(phy_st[nport].dfe_t[lane].tap2<-31)
									{
										phy_st[nport].dfe_t[lane].tap2 = -31;
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										//enable_adj_tap1[nport][lane]= TRUE;//change  to TAP1 adj
										HDMI_EMG("check le hit boundary, lane = %d, tap2= %d\n", lane, phy_st[nport].dfe_t[lane].tap2);			
									}
									else if(phy_st[nport].dfe_t[lane].tap2>31)
									{
										//enable_adj_tap1[nport][lane]= TRUE;	//
										phy_st[nport].dfe_t[lane].tap2 = 31;
										HDMI_EMG("check le hit boundary, lane = %d, tap2 = %d\n", lane, phy_st[nport].dfe_t[lane].tap2);			
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
									}
								}
								if(adj_le_done[nport][lane]==FALSE)
								{
									lib_hdmi_dfe_init_le(nport,(1<<lane),phy_st[nport].dfe_t[lane].le);
									lib_hdmi_dfe_init_tap2(nport,(1<<lane),phy_st[nport].dfe_t[lane].tap2);										
									if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
									{
										HDMI_PRINTF("[disp stage1] le & tap2 adj (lane=%d le=%d tap2=%d)\n",lane,phy_st[nport].dfe_t[lane].le,phy_st[nport].dfe_t[lane].tap2);

										}
								}
							}
							//else if (phy_st[nport].ced_err2[lane]>phy_st[nport].ced_err1[lane])
							else if ((phy_st[nport].ced_err2[lane]>phy_st[nport].ced_err1[lane])&&((phy_st[nport].ced_err2[lane]- phy_st[nport].ced_err1[lane])>20))
							{ 
								phy_st[nport].disp_err_zero_cnt[lane]=0;	//
								
								//else //adj le
								phy_st[nport].le_adp_step[lane] = - 1 *phy_st[nport].le_adp_step[lane];
								
								phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];

								phy_st[nport].dfe_t[lane].le = phy_st[nport].dfe_t[lane].le + phy_st[nport].le_adp_step[lane];

								m_wrong_direction_cnt[nport][lane] ++;
								//if(m_wrong_direction_cnt[nport][lane]>1 )
								//{
								//	m_wrong_direction_cnt[nport][lane]=0;
									//enable_adj_tap1[nport][lane]= TRUE;//change  to TPA1 adj
									//adj_parameter_changed[nport][lane]= TRUE;
								//}

								if(phy_st[nport].dfe_t[lane].le<0)
								{
									phy_st[nport].dfe_t[lane].le = 0;
									//if(adj_tap1_done[nport][lane]==FALSE)
									//{
										//enable_adj_tap1[nport][lane]= TRUE;//change  to TAP1 adj
										//adj_parameter_changed[nport][lane]= TRUE;
									//}										
									//m_wrong_direction_cnt[nport][lane]=0;
									phy_st[nport].le_adp_step[lane] = - 1 *phy_st[nport].le_adp_step[lane];
									phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];
									HDMI_EMG("check le hit boundary, lane = %d, le = %d\n", lane, phy_st[nport].dfe_t[lane].le);			
									//phy_st[nport].disp_adp_stage[lane] = 3;  // end
								}
								 else if(phy_st[nport].dfe_t[lane].le>LE_MAX_VALUE)
								//else if(phy_st[nport].dfe_t[lane].le>(phy_st[nport].dfe_t[lane].lemax+5))
								{

									phy_st[nport].dfe_t[lane].le = LE_MAX_VALUE;
									//phy_st[nport].dfe_t[lane].le = phy_st[nport].dfe_t[lane].lemax+5;
									m_wrong_direction_cnt[nport][lane]=0;
									if(adj_tap1_done[nport][lane]==FALSE)
									{
										//enable_adj_tap1[nport][lane]= TRUE;//change  to TAP1 adj
										//adj_parameter_changed[nport][lane]= TRUE;
									}										
									phy_st[nport].le_adp_step[lane] = - 1 *phy_st[nport].le_adp_step[lane];
									phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];
									HDMI_EMG("check le hit boundary, lane = %d, le = %d\n", lane, phy_st[nport].dfe_t[lane].le);			
									//phy_st[nport].disp_adp_stage[lane] = 3;  // end
								}
								else     //Change Tap2 value only when LE not in boundary condition
								{
									phy_st[nport].dfe_t[lane].tap2 = phy_st[nport].dfe_t[lane].tap2 +2* phy_st[nport].tap2_adp_step[lane];
									if(phy_st[nport].dfe_t[lane].tap2<-31)
									{
										phy_st[nport].dfe_t[lane].tap2 = -31;
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										//enable_adj_tap1[nport][lane]= TRUE;//change  to TAP1 adj
										HDMI_EMG("check le hit boundary, lane = %d, tap2 = %d\n", lane, phy_st[nport].dfe_t[lane].tap2);			
									}
									else if(phy_st[nport].dfe_t[lane].tap2>31)
									{
										//enable_adj_tap1[nport][lane]= TRUE;
										phy_st[nport].dfe_t[lane].tap2 = 31;
										HDMI_EMG("check le hit boundary, lane = %d, tap2 = %d\n", lane, phy_st[nport].dfe_t[lane].tap2);			
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
									}
								}

								if(adj_le_done[nport][lane]==FALSE)
								{
									lib_hdmi_dfe_init_le(nport,(1<<lane),phy_st[nport].dfe_t[lane].le);
									lib_hdmi_dfe_init_tap2(nport,(1<<lane),phy_st[nport].dfe_t[lane].tap2);										
									if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
									{
										HDMI_PRINTF("[disp stage1] le& tap2 adj - Inverse step=%d (nport=%d, lane=%d le=%d, tap2=%d, m_wrong_direction_cnt=%d)\n",phy_st[nport].le_adp_step[lane], nport, lane,phy_st[nport].dfe_t[lane].le, phy_st[nport].dfe_t[lane].tap2,m_wrong_direction_cnt[nport][lane]);
									}
								}
							}

						}
						#endif
						
						phy_st[nport].disp_start = 2;
						phy_st[nport].ced_err0[lane]=phy_st[nport].ced_err1[lane];
						phy_st[nport].ced_err1[lane]=phy_st[nport].ced_err2[lane];

					}

				}

				if (phy_st[nport].lane_num == HDMI_4LANE)
				{
					if ((phy_st[nport].disp_adp_stage[0] == 2) & (phy_st[nport].disp_adp_stage[1] == 2) & (phy_st[nport].disp_adp_stage[2] == 2) & (phy_st[nport].disp_adp_stage[3] == 2))
					{
						phy_st[nport].disp_start = 3; //ending
						HDMI_EMG("[disp stage2] port=%d, 4 lane done, dfe tap1(0,1,2,3)=(%d,%d,%d,%d)\\n",
							nport, phy_st[nport].dfe_t[0].tap1, phy_st[nport].dfe_t[1].tap1, phy_st[nport].dfe_t[2].tap1, phy_st[nport].dfe_t[3].tap1);
					}
				}
				else
				{
					if ((phy_st[nport].disp_adp_stage[0] == 2) & (phy_st[nport].disp_adp_stage[1] == 2) & (phy_st[nport].disp_adp_stage[2] == 2))  
					{
						phy_st[nport].disp_start = 3; //ending
						HDMI_EMG("[disp stage2] port=%d, 3 lane done, dfe tap1(0,1,2)=(%d,%d,%d)\\n",
							nport, phy_st[nport].dfe_t[0].tap1, phy_st[nport].dfe_t[1].tap1, phy_st[nport].dfe_t[2].tap1);
					}	
				}

			}
			else 
			{
				phy_st[nport].disp_timer_cnt[lane]--;
			}
		
		}
		else if(phy_st[nport].disp_start==3)
		{//End lib_hdmi20_phy_error_handling
			#if 0
			if((lt_fsm_status[nport].tx_flt_no_train == TRUE)&&(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1 , HDMI_FLOW_CFG2_CTS_EXTEND_MODE) == TRUE))
			//if((GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1 , HDMI_FLOW_CFG2_CTS_EXTEND_MODE) == TRUE))
			{//to do 
				newbase_hdmi_scdc_hw_mask_enable(nport, 0);
				HDMI_PRINTF("[disp stage3] port=%d, Adjust finished, close extend\n", nport);
				phy_st[nport].disp_start =4;
			}
			#endif
		}
		else 
		{
			//phy_st[nport].disp_start == 3 do nothing

		}
	}
	else  //Mode ==3
	{

		if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
		{
			newbase_hdmi21_dump_dfe_para(nport,phy_st[nport].lane_num);
		}

		
		if (phy_st[nport].disp_start == 0) 
		{

			for (lane=0; lane< 3; lane++) {
				phy_st[nport].dfe_t[lane].tap1 = lib_hdmi_dfe_get_tap1(nport, lane);
				phy_st[nport].dfe_t[lane].tap2 = lib_hdmi_dfe_get_tap2(nport, lane);
				phy_st[nport].dfe_t[lane].le = lib_hdmi_dfe_get_le(nport, lane);

				phy_st[nport].dfe_t[lane].le_add_tap1 = phy_st[nport].dfe_t[lane].le+phy_st[nport].dfe_t[lane].tap1;

				lib_hdmi_dfe_init_tap1(nport,(1<<lane),phy_st[nport].dfe_t[lane].le_add_tap1);

				phy_st[nport].dfe_t[lane].tap1 = lib_hdmi_dfe_get_tap1(nport, lane);
				phy_st[nport].dfe_t[lane].tap2 = lib_hdmi_dfe_get_tap2(nport, lane);
				phy_st[nport].dfe_t[lane].le = lib_hdmi_dfe_get_le(nport, lane);

				HDMI_PRINTF("[disp init]ADD LEdisp_start=%d, phy_st[nport].dfe_t[lane].le_add_tap1=%d\n",phy_st[nport].disp_start, phy_st[nport].dfe_t[lane].le_add_tap1);			

			}

			
			newbase_hdmi_fw_ced_measure(nport, phy_st[nport].ced_err0, 60);

			phy_st[nport].disp_start = 1;
			phy_st[nport].disp_adp_stage[0]  = phy_st[nport].disp_adp_stage[1]  = phy_st[nport].disp_adp_stage[2] = phy_st[nport].disp_adp_stage[3]  = 1;
			phy_st[nport].disp_timer_cnt[0] = phy_st[nport].disp_timer_cnt[1]= phy_st[nport].disp_timer_cnt[2]= phy_st[nport].disp_timer_cnt[3] = 5;  //100ms
		}
		else if (phy_st[nport].disp_start == 1)
		{

			if (phy_st[nport].disp_timer_cnt[lane] == 0)
			{  //near 100ms
				newbase_hdmi_fw_ced_measure(nport, phy_st[nport].ced_err1, 60);

				phy_st[nport].disp_start = 2;
				phy_st[nport].disp_adp_stage[0]  = phy_st[nport].disp_adp_stage[1]  = phy_st[nport].disp_adp_stage[2] = phy_st[nport].disp_adp_stage[3]  = 1;
				phy_st[nport].disp_timer_cnt[0] = phy_st[nport].disp_timer_cnt[1]= phy_st[nport].disp_timer_cnt[2]= phy_st[nport].disp_timer_cnt[3] = 5;  //100ms
			}
			else 
			{
				phy_st[nport].disp_timer_cnt[lane]--;
			}
	
		}
		else if (phy_st[nport].disp_start == 2)
		{
			if (phy_st[nport].disp_timer_cnt[lane] == 0) 
			{  //near 100ms
				//lib_hdmi_hd21_fw_disparity_error_status(nport,phy_st[nport].disp_err2);
				newbase_hdmi_fw_ced_measure(nport, phy_st[nport].ced_err2, 60);
				for (lane = 0; lane < phy_st[nport].lane_num ; lane++) 
				{
					//HDMI_PRINTF("[disp ing]lane=%d, disp_adp_stage[lane]=%d, m_wrong_direction_cnt=%d, disp_err_zero_cnt[lane]=%d \n",lane,
					//phy_st[nport].disp_adp_stage[lane], m_wrong_direction_cnt[nport][lane], phy_st[nport].disp_err_zero_cnt[lane] );

					//if(m_wrong_direction_cnt[nport][lane] >10)
					//{
					//	HDMI_EMG("[Disp end]wrong_direction over, nport=%d, lane=%d, m_wrong_direction_cnt[nport][lane] =%d\n", nport, lane, m_wrong_direction_cnt[nport][lane] );
					//	m_wrong_direction_cnt[nport][lane] =0;
					//	phy_st[nport].disp_adp_stage[lane] = 2;  //end
					//}

					phy_st[nport].dfe_t[lane].le_add_tap1 = phy_st[nport].dfe_t[lane].le+phy_st[nport].dfe_t[lane].tap1;

					if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
					{
						HDMI_PRINTF("[disp adp]lane=%d,  ced_err2=%d, ced_err1=%d, ced_err0=%d\n", 
						lane, 
						phy_st[nport].ced_err2[lane],
						phy_st[nport].ced_err1[lane],
						phy_st[nport].ced_err0[lane]);
					}
					//Check for Stop tuning condition
					if (phy_st[nport].disp_err_zero_cnt[lane] >=10000) 
					{
						//phy_st[nport].disp_adp_stage[lane] = 2;//end Marked for keep on tuing  never stop 
						phy_st[nport].disp_err_zero_cnt[lane] = 10000;//Set to Maxmum error cnt protect
						HDMI_PRINTF("[disp adp stage1] lane=%d, disp_err_zero_cnt %d over(tap1=%d)\n",lane,phy_st[nport].disp_err_zero_cnt[lane] , phy_st[nport].dfe_t[lane].tap1);
					}

					//Dummy now for No tap1_done case
					if((adj_le_done[nport][lane]==TRUE)&&(adj_tap1_done[nport][lane]==TRUE))
					{
						phy_st[nport].disp_adp_stage[lane] = 2;//end
						phy_st[nport].disp_err_zero_cnt[lane] = 0;
						HDMI_PRINTF("[disp End]ADP Done  lane=%d, disp_err_zero_cnt %d over(tap1=%d)\n",lane,phy_st[nport].disp_err_zero_cnt[lane] , phy_st[nport].dfe_t[lane].tap1);
					}

	
					if (phy_st[nport].disp_adp_stage[lane] == 2) 
					{  //end
						HDMI_PRINTF("[disp End] (lane=%d tap1=%d) tap2=%d)\n",lane,phy_st[nport].dfe_t[lane].tap1, phy_st[nport].dfe_t[lane].tap2);
						continue;
					}
					else if (phy_st[nport].disp_adp_stage[lane] == 1)
					{

					 #if HDMI20_PHY_ERROR_CORRECTION_EN

						if((phy_st[nport].ced_err2[lane] ) <= 10)	
						{
							//HDMI_PRINTF("Almost no error increased , error @ port[%d]lane:%d, err2= %d, err1= %d\n",nport,lane, phy_st[nport].disp_err2[lane], phy_st[nport].disp_err1[lane]);
							if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
							{
								HDMI_PRINTF("Almost no error increased , error @ port[%d]lane:%d, err2= %d, err1= %d\n",nport,lane, phy_st[nport].ced_err2[lane], phy_st[nport].ced_err1[lane]);
							}

							phy_st[nport].disp_err_zero_cnt[lane] = phy_st[nport].disp_err_zero_cnt[lane] + 1;					
							phy_st[nport].disp_start = 2;
						}
						else
						{
							//if ((phy_st[nport].ced_err2[lane]<= phy_st[nport].ced_err1[lane])||(adj_parameter_changed[nport][lane]== TRUE))


							if ((phy_st[nport].ced_err2[lane]<=phy_st[nport].ced_err1[lane])&&((phy_st[nport].ced_err1[lane]- phy_st[nport].ced_err2[lane])>20))
							{

								if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
								{
									HDMI_PRINTF("[disp stage1]  lane=%d, disp_err_zero_cnt %d  ced_err2[lane]=%d)\n",lane,phy_st[nport].disp_err_zero_cnt[lane] , phy_st[nport].ced_err2[lane]);
								}
								if (adj_parameter_changed[nport][lane]== TRUE)
								{
									adj_parameter_changed[nport][lane]= FALSE;
								}

								phy_st[nport].disp_err_zero_cnt[lane]=0; //

								if(adj_tap1_done[nport][lane]==FALSE) //
								{
									//if(( phy_st[nport].dfe_t[lane].le)< (DFE_P0_REG_DFE_EN_L0_P0_get_le_min_lane0_p0(hdmi_in(DFE_REG_DFE_EN_L0_reg))))
									phy_st[nport].dfe_t[lane].le_add_tap1 = phy_st[nport].dfe_t[lane].le_add_tap1 +1* phy_st[nport].tap1_adp_step[lane];
		
									if(phy_st[nport].dfe_t[lane].le_add_tap1<0)
									{
										phy_st[nport].dfe_t[lane].le_add_tap1 = 0;
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].tap1_adp_step[lane] = - 1 *phy_st[nport].tap1_adp_step[lane];
										phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];

										HDMI_EMG("check tap1 hit boundary, lane = %d, tap1 = %d\n", lane, phy_st[nport].dfe_t[lane].le_add_tap1);			
									}
									else if(phy_st[nport].dfe_t[lane].le_add_tap1>(TAP1_MAX_VALUE+(DFE_P0_REG_DFE_CTRL_B8_P0_get_leq_min_b_4_0(hdmi_in(REG_DFE_CTRL_B8_reg)))))
									{
										phy_st[nport].dfe_t[lane].le_add_tap1 = (TAP1_MAX_VALUE+(DFE_P0_REG_DFE_CTRL_B8_P0_get_leq_min_b_4_0(hdmi_in(REG_DFE_CTRL_B8_reg))));
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].tap1_adp_step[lane] = - 1 *phy_st[nport].tap1_adp_step[lane];
										phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];

										HDMI_EMG("check tap1 hit boundary, lane = %d, le_add_tap1 = %d\n", lane, phy_st[nport].dfe_t[lane].le_add_tap1);			
									}

									lib_hdmi_dfe_init_tap1(nport,(1<<lane),phy_st[nport].dfe_t[lane].le_add_tap1);

									if(phy_st[nport].dfe_t[lane].le_add_tap1 <= (DFE_P0_REG_DFE_CTRL_B8_P0_get_leq_min_b_4_0(hdmi_in(REG_DFE_CTRL_B8_reg))))
									{
										phy_st[nport].dfe_t[lane].tap2 = phy_st[nport].dfe_t[lane].tap2 +2* phy_st[nport].tap2_adp_step[lane];
										lib_hdmi_dfe_init_tap2(nport,(1<<lane),phy_st[nport].dfe_t[lane].tap2);
									}

									if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
									{
										HDMI_PRINTF("[disp stage1] tap1 adj (lane=%d le_add_tap1=%d tap2=%d)\n",lane,phy_st[nport].dfe_t[lane].le_add_tap1,phy_st[nport].dfe_t[lane].tap2);
									}
								}
							
								//phy_st[nport].disp_start = 2;
								//phy_st[nport].ced_err0[lane]=phy_st[nport].ced_err1[lane];
								//phy_st[nport].ced_err1[lane]=phy_st[nport].ced_err2[lane];

							}
							//else if (phy_st[nport].ced_err2[lane]>phy_st[nport].ced_err1[lane])
							else if ((phy_st[nport].ced_err2[lane]>phy_st[nport].ced_err1[lane])&&((phy_st[nport].ced_err2[lane]- phy_st[nport].ced_err1[lane])>20))
							{ 
								phy_st[nport].disp_err_zero_cnt[lane]=0;	//

								if(adj_tap1_done[nport][lane]==FALSE) //
								{
									phy_st[nport].tap1_adp_step[lane] = - 1 *phy_st[nport].tap1_adp_step[lane];

									//if(( phy_st[nport].dfe_t[lane].le)< (DFE_P0_REG_DFE_EN_L0_P0_get_le_min_lane0_p0(hdmi_in(DFE_REG_DFE_EN_L0_reg))))

									phy_st[nport].dfe_t[lane].le_add_tap1 = phy_st[nport].dfe_t[lane].le_add_tap1 +1* phy_st[nport].tap1_adp_step[lane];
		
									if(phy_st[nport].dfe_t[lane].le_add_tap1<0)
									{
										phy_st[nport].dfe_t[lane].le_add_tap1 = 0;
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].tap1_adp_step[lane] = - 1 *phy_st[nport].tap1_adp_step[lane];
										phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];

										HDMI_EMG("check tap1 hit boundary, lane = %d, tap1 = %d\n", lane, phy_st[nport].dfe_t[lane].le_add_tap1);			
									}
									else if(phy_st[nport].dfe_t[lane].le_add_tap1>(TAP1_MAX_VALUE+(DFE_P0_REG_DFE_CTRL_B8_P0_get_leq_min_b_4_0(hdmi_in(REG_DFE_CTRL_B8_reg)))))
									{
										phy_st[nport].dfe_t[lane].le_add_tap1 = (TAP1_MAX_VALUE+(DFE_P0_REG_DFE_CTRL_B8_P0_get_leq_min_b_4_0(hdmi_in(REG_DFE_CTRL_B8_reg))));
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].tap1_adp_step[lane] = - 1 *phy_st[nport].tap1_adp_step[lane];
										phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];

										HDMI_EMG("check tap1 hit boundary, lane = %d, le_add_tap1 = %d\n", lane, phy_st[nport].dfe_t[lane].le_add_tap1);			
									}
									lib_hdmi_dfe_init_tap1(nport,(1<<lane),phy_st[nport].dfe_t[lane].le_add_tap1);

									if(phy_st[nport].dfe_t[lane].le_add_tap1 <= (DFE_P0_REG_DFE_CTRL_B8_P0_get_leq_min_b_4_0(hdmi_in(REG_DFE_CTRL_B8_reg))))
									{
										phy_st[nport].dfe_t[lane].tap2 = phy_st[nport].dfe_t[lane].tap2 +2* phy_st[nport].tap2_adp_step[lane];
										lib_hdmi_dfe_init_tap2(nport,(1<<lane),phy_st[nport].dfe_t[lane].tap2);
									}
									if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
									{
										HDMI_PRINTF("[disp stage1] tap1 adj (lane=%d le_add_tap1=%d tap2=%d)\n",lane,phy_st[nport].dfe_t[lane].le_add_tap1,phy_st[nport].dfe_t[lane].tap2);
									}
								}

								//phy_st[nport].disp_start = 2;
								//phy_st[nport].ced_err0[lane]=phy_st[nport].ced_err1[lane];
								//phy_st[nport].ced_err1[lane]=phy_st[nport].ced_err2[lane];
							}


						}

					#endif

					}

					phy_st[nport].disp_start = 2;
					phy_st[nport].ced_err0[lane]=phy_st[nport].ced_err1[lane];
					phy_st[nport].ced_err1[lane]=phy_st[nport].ced_err2[lane];

				}

				phy_st[nport].dfe_t[lane].tap1 = lib_hdmi_dfe_get_tap1(nport, lane);
				phy_st[nport].dfe_t[lane].le = lib_hdmi_dfe_get_le(nport, lane);
				if (phy_st[nport].lane_num == HDMI_4LANE)
				{
					if ((phy_st[nport].disp_adp_stage[0] == 2) & (phy_st[nport].disp_adp_stage[1] == 2) & (phy_st[nport].disp_adp_stage[2] == 2) & (phy_st[nport].disp_adp_stage[3] == 2))
					{
						phy_st[nport].disp_start = 3; //ending
						HDMI_EMG("[disp stage2] port=%d, 4 lane done, dfe tap1(0,1,2,3)=(%d,%d,%d,%d)\\n",
							nport, phy_st[nport].dfe_t[0].tap1, phy_st[nport].dfe_t[1].tap1, phy_st[nport].dfe_t[2].tap1, phy_st[nport].dfe_t[3].tap1);
					}
				}
				else
				{
					if ((phy_st[nport].disp_adp_stage[0] == 2) & (phy_st[nport].disp_adp_stage[1] == 2) & (phy_st[nport].disp_adp_stage[2] == 2))  
					{
						phy_st[nport].disp_start = 3; //ending
						HDMI_EMG("[disp stage2] port=%d, 3 lane done, dfe tap1(0,1,2)=(%d,%d,%d)\\n",
							nport, phy_st[nport].dfe_t[0].tap1, phy_st[nport].dfe_t[1].tap1, phy_st[nport].dfe_t[2].tap1);
					}	
				}

			}
			else 
			{
				phy_st[nport].disp_timer_cnt[lane]--;
			}
		
		}
		else if(phy_st[nport].disp_start==3)
		{//End lib_hdmi20_phy_error_handling
			#if 0
			if((lt_fsm_status[nport].tx_flt_no_train == TRUE)&&(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1 , HDMI_FLOW_CFG2_CTS_EXTEND_MODE) == TRUE))
			//if((GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1 , HDMI_FLOW_CFG2_CTS_EXTEND_MODE) == TRUE))
			{//to do 
				newbase_hdmi_scdc_hw_mask_enable(nport, 0);
				HDMI_PRINTF("[disp stage3] port=%d, Adjust finished, close extend\n", nport);
				phy_st[nport].disp_start =4;
			}
			#endif
		}
		else 
		{
			//phy_st[nport].disp_start == 3 do nothing

		}
	
	}
#endif
}




#if PHY_ADAPTIVE_BY_LTP_PAT_AFTER_LTP_PASS
//check ltp err cnt after finishing link training pass (for TEK CTS 20s ltp pattern still output)
void lib_hdmi_hd21_ltp_phy_adaptive_check(unsigned char nport, unsigned char lane_num)
{
#if 1
	unsigned char lock[4];
	unsigned char lock_all = 0;
	unsigned int err_cnt[4];
	unsigned char lane = 0;


	if (phy_st[nport].ltp_state) {  //video state
		  lock[3] = hdmi_in(HD21_channel_align_c_reg)  & 0x1;
       	  lock[2] = hdmi_in(HD21_channel_align_r_reg)  & 0x1;
       	  lock[1] = hdmi_in(HD21_channel_align_g_reg) & 0x1;
      	         lock[0] = hdmi_in(HD21_channel_align_b_reg) & 0x1;
				 
		  if (lane_num == HDMI_4LANE) {
		  	if (lock[0] & lock[1] & lock[2] & lock[3])
				return;
		  } else {
			if (lock[0] & lock[1] & lock[2]) 
				return;
		  }
	}
	

	lock[0] = HD21_LT_B_get_lock_valid(hdmi_in(HD21_LT_B_reg));
   	lock[1] = HD21_LT_G_get_lock_valid(hdmi_in(HD21_LT_G_reg));
       lock[2] = HD21_LT_R_get_lock_valid(hdmi_in(HD21_LT_R_reg));
       lock[3] = HD21_LT_C_get_lock_valid(hdmi_in(HD21_LT_C_reg));

	if (lane_num == HDMI_4LANE) { 
		
		HDMI_PRINTF("[ltp_chk]ltp lock= %d %d %d %d\n",lock[0],lock[1],lock[2],lock[3]);
		lock_all = lock[0] & lock[1] & lock[2] & lock[3];
	}
	else {
		HDMI_PRINTF("[ltp_chk]ltp lock= %d %d %d\n",lock[0],lock[1],lock[2]);
		lock_all = lock[0] & lock[1] & lock[2];
	}

	 if (lock_all) {
		    hdmi_mask(HD21_LT_B_reg, ~(HD21_LT_B_popup_mask|_BIT4), HD21_LT_B_popup_mask);
		    hdmi_mask(HD21_LT_G_reg, ~(HD21_LT_G_popup_mask|_BIT4), HD21_LT_G_popup_mask);
		    hdmi_mask(HD21_LT_R_reg, ~(HD21_LT_R_popup_mask|_BIT4), HD21_LT_R_popup_mask);
		    if (lane_num == HDMI_4LANE)
		    	hdmi_mask(HD21_LT_C_reg, ~(HD21_LT_C_popup_mask|_BIT4), HD21_LT_C_popup_mask);

		    udelay(10);

		    if (lane_num == HDMI_4LANE)
				HDMI_PRINTF("[ltp_chk]%x %x %x %x\n",hdmi_in(HD21_LT_B_reg),hdmi_in(HD21_LT_G_reg),hdmi_in(HD21_LT_R_reg),hdmi_in(HD21_LT_C_reg));
		    else
				HDMI_PRINTF("[ltp_chk]%x %x %x\n",hdmi_in(HD21_LT_B_reg),hdmi_in(HD21_LT_G_reg),hdmi_in(HD21_LT_R_reg));
			
		    err_cnt[0] = (lock[0]) ? HD21_LT_B_get_err_cnt(hdmi_in(HD21_LT_B_reg)) : 0xFFFFFFFF;
		    err_cnt[1] = (lock[1]) ? HD21_LT_G_get_err_cnt(hdmi_in(HD21_LT_G_reg)) : 0xFFFFFFFF;
		    err_cnt[2] = (lock[2]) ? HD21_LT_R_get_err_cnt(hdmi_in(HD21_LT_R_reg)) : 0xFFFFFFFF;
		    if (lane_num == HDMI_4LANE)
		    	err_cnt[3] = (lock[3]) ? HD21_LT_C_get_err_cnt(hdmi_in(HD21_LT_C_reg)) : 0xFFFFFFFF;

		    if (lane_num == HDMI_4LANE) {
			    if ((err_cnt[0] == 0x7FFF) ||  (err_cnt[1] == 0x7FFF)  ||  (err_cnt[2] == 0x7FFF) || (err_cnt[3] == 0x7FFF))
			    {
					hdmi_mask(HD21_LT_B_reg, ~(HDMI21_P0_HD21_LT_B_reset_mask|_BIT4), HDMI21_P0_HD21_LT_B_reset_mask);
				       hdmi_mask(HD21_LT_G_reg, ~(HDMI21_P0_HD21_LT_G_reset_mask|_BIT4), HDMI21_P0_HD21_LT_G_reset_mask);
					hdmi_mask(HD21_LT_R_reg, ~(HDMI21_P0_HD21_LT_R_reset_mask|_BIT4), HDMI21_P0_HD21_LT_R_reset_mask);
					hdmi_mask(HD21_LT_C_reg, ~(HDMI21_P0_HD21_LT_C_reset_mask|_BIT4), HDMI21_P0_HD21_LT_C_reset_mask);
					HDMI_PRINTF("[ltp_chk]change ltp to video pattern\n");
					phy_st[nport].ltp_state = 1;
					return;
			    }
		     } else {
			    if ((err_cnt[0] == 0x7FFF) ||  (err_cnt[1] == 0x7FFF)  ||  (err_cnt[2] == 0x7FFF))
			    {
					hdmi_mask(HD21_LT_B_reg, ~(HDMI21_P0_HD21_LT_B_reset_mask|_BIT4), HDMI21_P0_HD21_LT_B_reset_mask);
				       hdmi_mask(HD21_LT_G_reg, ~(HDMI21_P0_HD21_LT_G_reset_mask|_BIT4), HDMI21_P0_HD21_LT_G_reset_mask);
					hdmi_mask(HD21_LT_R_reg, ~(HDMI21_P0_HD21_LT_R_reset_mask|_BIT4), HDMI21_P0_HD21_LT_R_reset_mask);
					HDMI_PRINTF("[ltp_chk]change ltp to video pattern\n");
					phy_st[nport].ltp_state = 1;
					return;
			    }
		     }

		    if (lane_num == HDMI_4LANE)
		    		HDMI_PRINTF("[ltp_chk]err_cnt= %d %d %d %d\n",err_cnt[0],err_cnt[1],err_cnt[2],err_cnt[3]);
		    else
				HDMI_PRINTF("[ltp_chk]err_cnt= %d %d %d\n",err_cnt[0],err_cnt[1],err_cnt[2]);

		    for(lane = 0 ; lane < lane_num ; ++lane) {
				#if 0	
			    	if(err_cnt[lane] > 0) {
					if (phy_st[nport].dfe_t[lane].tap1 > 24) {
						HDMI_PRINTF("[adp_1]tap1 is up without adaptive....\n");
						return ;
			    		}
					phy_st[nport].dfe_t[lane].tap1 = phy_st[nport].dfe_t[lane].tap1 + 2;		//tap1 + 2
					lib_hdmi_dfe_init_tap1(nport,(1<<lane),phy_st[nport].dfe_t[lane].tap1);
					HDMI_PRINTF("[adp_1]  tap1 adj (lane=%d tap1=%d)\n",lane,phy_st[nport].dfe_t[lane].tap1);
					if (lane == 0)
						  hdmi_mask(HD21_LT_B_reg, ~(HDMI21_P0_HD21_LT_B_reset_mask|_BIT4), HDMI21_P0_HD21_LT_B_reset_mask);
					else if (lane == 1)
						  hdmi_mask(HD21_LT_G_reg, ~(HDMI21_P0_HD21_LT_G_reset_mask|_BIT4), HDMI21_P0_HD21_LT_G_reset_mask);
					else if (lane == 2)
						hdmi_mask(HD21_LT_R_reg, ~(HDMI21_P0_HD21_LT_R_reset_mask|_BIT4), HDMI21_P0_HD21_LT_R_reset_mask);
					else
						hdmi_mask(HD21_LT_C_reg, ~(HDMI21_P0_HD21_LT_C_reset_mask|_BIT4), HDMI21_P0_HD21_LT_C_reset_mask);
				}
				#else

				if (phy_st[nport].ltp_err_end[lane])
					continue;
				
				if(err_cnt[lane] > 0) {
					if ( phy_st[nport].ltp_err_start[lane] == 0)
					{
						phy_st[nport].ltp_err_pre[lane] = err_cnt[lane];
						phy_st[nport].dfe_t[lane].tap1 = phy_st[nport].dfe_t[lane].tap1 + 2;		//tap1 + 2
						lib_hdmi_dfe_init_tap1(nport,(1<<lane),phy_st[nport].dfe_t[lane].tap1);
						HDMI_PRINTF("[ltp_chk] tap1 adj + 2 (lane=%d tap1=%d)\n",lane,phy_st[nport].dfe_t[lane].tap1);
						phy_st[nport].ltp_err_start[lane] = 1;
						
					} else {
						if (err_cnt[lane] >  phy_st[nport].ltp_err_pre[lane] ) {
							if (phy_st[nport].ltp_err_zero_cnt[lane] > 5) {
								phy_st[nport].dfe_t[lane].tap1 = phy_st[nport].dfe_t[lane].tap1 - 2;		//tap1 - 2
								lib_hdmi_dfe_init_tap1(nport,(1<<lane),phy_st[nport].dfe_t[lane].tap1);
								HDMI_PRINTF("[ltp_chk] tap1 adj - 2 (lane=%d tap1=%d)\n",lane,phy_st[nport].dfe_t[lane].tap1);
								phy_st[nport].ltp_err_end[lane] =1;
							}
						} else if (err_cnt[lane] <=  phy_st[nport].ltp_err_pre[lane] )  {
							phy_st[nport].ltp_err_pre[lane] = err_cnt[lane];
							phy_st[nport].dfe_t[lane].tap1 = phy_st[nport].dfe_t[lane].tap1 + 2;		//tap1 + 2
							lib_hdmi_dfe_init_tap1(nport,(1<<lane),phy_st[nport].dfe_t[lane].tap1);
							HDMI_PRINTF("[ltp_chk] tap1 adj + 2 (lane=%d tap1=%d)\n",lane,phy_st[nport].dfe_t[lane].tap1);
						} 
					}

					if (lane == 0)
						  hdmi_mask(HD21_LT_B_reg, ~(HDMI21_P0_HD21_LT_B_reset_mask|_BIT4), HDMI21_P0_HD21_LT_B_reset_mask);
					else if (lane == 1)
						  hdmi_mask(HD21_LT_G_reg, ~(HDMI21_P0_HD21_LT_G_reset_mask|_BIT4), HDMI21_P0_HD21_LT_G_reset_mask);
					else if (lane == 2)
						hdmi_mask(HD21_LT_R_reg, ~(HDMI21_P0_HD21_LT_R_reset_mask|_BIT4), HDMI21_P0_HD21_LT_R_reset_mask);
					else
						hdmi_mask(HD21_LT_C_reg, ~(HDMI21_P0_HD21_LT_C_reset_mask|_BIT4), HDMI21_P0_HD21_LT_C_reset_mask);	
					
					phy_st[nport].ltp_err_zero_cnt[lane] = 0;
				}
				else {
					phy_st[nport].ltp_err_zero_cnt[lane] = phy_st[nport].ltp_err_zero_cnt[lane]+1; //accumulate zero cnt
				}
				#endif
		    }	
	    }else {
			HDMI_PRINTF("[ltp_chk]ltp no lock\n");
	    }
#endif
   		
}
#endif

//check ltp err in link training state
void lib_hdmi_hd21_ltp_phy_adaptive(unsigned char nport, unsigned char lane_num)
{
#if 1
    unsigned char lock[4];
    unsigned char lock_all;
    unsigned int err_cnt[4];
    unsigned char lane = 0;
    unsigned char timeout = GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_LTP_PHY_ADAPTIVE_EN );

 
	    while(timeout >0)
	    {
		     lock[0] = HD21_LT_B_get_lock_valid(hdmi_in(HD21_LT_B_reg));
	   	     lock[1] = HD21_LT_G_get_lock_valid(hdmi_in(HD21_LT_G_reg));
	            lock[2] = HD21_LT_R_get_lock_valid(hdmi_in(HD21_LT_R_reg));
		     if (lane_num == HDMI_4LANE)
	            		lock[3] = HD21_LT_C_get_lock_valid(hdmi_in(HD21_LT_C_reg));

		    if (lane_num == HDMI_4LANE) {
			lock_all = lock[0] & lock[1] & lock[2] & lock[3];
		    	HDMI_PRINTF("[LTP] lock= %d %d %d %d\n",lock[0],lock[1],lock[2],lock[3]);
		    }
		    else {
			lock_all = lock[0] & lock[1] & lock[2];	
			HDMI_PRINTF("[LTP] lock= %d %d %d\n",lock[0],lock[1],lock[2]);
		     }	

		    if (lock_all) {
			    hdmi_mask(HD21_LT_B_reg, ~(HD21_LT_B_popup_mask|_BIT4), HD21_LT_B_popup_mask);
			    hdmi_mask(HD21_LT_G_reg, ~(HD21_LT_G_popup_mask|_BIT4), HD21_LT_G_popup_mask);
			    hdmi_mask(HD21_LT_R_reg, ~(HD21_LT_R_popup_mask|_BIT4), HD21_LT_R_popup_mask);
			     if (lane_num == HDMI_4LANE)
			    		hdmi_mask(HD21_LT_C_reg, ~(HD21_LT_C_popup_mask|_BIT4), HD21_LT_C_popup_mask);

			     udelay(10);

			     if (lane_num == HDMI_4LANE)
			    		HDMI_PRINTF("[LTP] %x %x %x %x\n",hdmi_in(HD21_LT_B_reg),hdmi_in(HD21_LT_G_reg),hdmi_in(HD21_LT_R_reg),hdmi_in(HD21_LT_C_reg));
			     else
				 	HDMI_PRINTF("[LTP] %x %x %x\n",hdmi_in(HD21_LT_B_reg),hdmi_in(HD21_LT_G_reg),hdmi_in(HD21_LT_R_reg));

				 
			    err_cnt[0] = (lock[0]) ? HD21_LT_B_get_err_cnt(hdmi_in(HD21_LT_B_reg)) : 0xFFFFFFFF;
			    err_cnt[1] = (lock[1]) ? HD21_LT_G_get_err_cnt(hdmi_in(HD21_LT_G_reg)) : 0xFFFFFFFF;
			    err_cnt[2] = (lock[2]) ? HD21_LT_R_get_err_cnt(hdmi_in(HD21_LT_R_reg)) : 0xFFFFFFFF;
			    if (lane_num == HDMI_4LANE)
			    	err_cnt[3] = (lock[3]) ? HD21_LT_C_get_err_cnt(hdmi_in(HD21_LT_C_reg)) : 0xFFFFFFFF;

			     if (lane_num == HDMI_4LANE) {
				    HDMI_PRINTF("[LTP] err_cnt= %d %d %d %d\n",err_cnt[0],err_cnt[1],err_cnt[2],err_cnt[3]);

				    if ((err_cnt[0] == 0x7FFF) ||  (err_cnt[1] == 0x7FFF)  ||  (err_cnt[2] == 0x7FFF) || (err_cnt[3] == 0x7FFF))
				    {
						hdmi_mask(HD21_LT_B_reg, ~(HDMI21_P0_HD21_LT_B_reset_mask|_BIT4), HDMI21_P0_HD21_LT_B_reset_mask);
					       hdmi_mask(HD21_LT_G_reg, ~(HDMI21_P0_HD21_LT_G_reset_mask|_BIT4), HDMI21_P0_HD21_LT_G_reset_mask);
						hdmi_mask(HD21_LT_R_reg, ~(HDMI21_P0_HD21_LT_R_reset_mask|_BIT4), HDMI21_P0_HD21_LT_R_reset_mask);
						hdmi_mask(HD21_LT_C_reg, ~(HDMI21_P0_HD21_LT_C_reset_mask|_BIT4), HDMI21_P0_HD21_LT_C_reset_mask);
						continue;
				    }
			     } else {
				    HDMI_PRINTF("[LTP] err_cnt= %d %d %d\n",err_cnt[0],err_cnt[1],err_cnt[2]);

				    if ((err_cnt[0] == 0x7FFF) ||  (err_cnt[1] == 0x7FFF)  ||  (err_cnt[2] == 0x7FFF))
				    {
						hdmi_mask(HD21_LT_B_reg, ~(HDMI21_P0_HD21_LT_B_reset_mask|_BIT4), HDMI21_P0_HD21_LT_B_reset_mask);
					       hdmi_mask(HD21_LT_G_reg, ~(HDMI21_P0_HD21_LT_G_reset_mask|_BIT4), HDMI21_P0_HD21_LT_G_reset_mask);
						hdmi_mask(HD21_LT_R_reg, ~(HDMI21_P0_HD21_LT_R_reset_mask|_BIT4), HDMI21_P0_HD21_LT_R_reset_mask);
						continue;
				    }

			     }
				

			    for(lane = 0 ; lane < lane_num ; ++lane) {
			    	if(err_cnt[lane] > 0) {
					phy_st[nport].dfe_t[lane].tap1 = phy_st[nport].dfe_t[lane].tap1 + 2;		//tap1 + 2
					lib_hdmi_dfe_init_tap1(nport,(1<<lane),phy_st[nport].dfe_t[lane].tap1);
					HDMI_PRINTF("[LTP] tap1 adj (lane=%d tap1=%d)\n",lane,phy_st[nport].dfe_t[lane].tap1);
					if (lane == 0)
						  hdmi_mask(HD21_LT_B_reg, ~(HDMI21_P0_HD21_LT_B_reset_mask|_BIT4), HDMI21_P0_HD21_LT_B_reset_mask);
					else if (lane == 1)
						  hdmi_mask(HD21_LT_G_reg, ~(HDMI21_P0_HD21_LT_G_reset_mask|_BIT4), HDMI21_P0_HD21_LT_G_reset_mask);
					else if (lane == 2)
						hdmi_mask(HD21_LT_R_reg, ~(HDMI21_P0_HD21_LT_R_reset_mask|_BIT4), HDMI21_P0_HD21_LT_R_reset_mask);
					else
						hdmi_mask(HD21_LT_C_reg, ~(HDMI21_P0_HD21_LT_C_reset_mask|_BIT4), HDMI21_P0_HD21_LT_C_reset_mask);
				}
			    }	
		    }else {
				HDMI_PRINTF("[LTP]no lock\n");
		    }
		     mdelay(10);
		     timeout--;
			
	    }
   	
#endif
}




void newbase_rxphy_frl_job(unsigned char p)
{
//#if LINK_TRAINING_TX_INFO_DEBUG
	unsigned int err[4];
	unsigned char nport = 0;
//#endif

	if (lib_hdmi_get_fw_debug_bit(DEBUG_14_FRL_FORCE_MODE)) {
		return;
	}
	

	switch (phy_st[p].phy_proc_state) {
	case PHY_PROC_INIT:
		newbase_hdmi21_set_phy(p, phy_st[p].clk,phy_st[p].frl_mode,phy_st[p].lane_num);
		newbase_hdmi_init_disparity_ltp_var(p);
//check after		lib_hdmi_hd21_disparity_symbol_error_disable();
//		newbase_hdmi2p0_inc_scdc_toggle(p);

		phy_st[p].phy_proc_state= PHY_PROC_CLOSE_ADP;
			
	case PHY_PROC_CLOSE_ADP:

		if (newbase_hdmi21_frl_dfe_close(p, phy_st[p].clk,phy_st[p].lane_num,PHY_PROC_CLOSE_ADP)) {

			newbase_hdmi2p0_detect_config(p);

			phy_st[p].phy_proc_state = PHY_PROC_DONE;

		} else {
			phy_st[p].phy_proc_state = PHY_PROC_INIT;
			break;
		}

	case PHY_PROC_DONE:
		if (lib_hdmi_get_fw_debug_bit(DEBUG_18_PHY_DBG_MSG))
			debug_hdmi_dump_msg(p);

		if(GET_FRL_LT_FSM(p)>=LT_FSM_LTSP_PASS) {
//#if LINK_TRAINING_TX_INFO_DEBUG
			if(lt_fsm_status[p].tx_flt_no_train == TRUE)
			{
				lib_hdmi_char_err_get_scdc_ced(p, phy_st[p].frl_mode, err);

				nport = p;
				HDMI_PRINTF("[CEDF] port=%d, align(b,g,r,ck)=(%x %x %x %x)  scdc err (b,g,r,ck)=%x %x %x %x), scdc_0x10= %x,  scdc_0x40=%x,%x,%x, m_disparity_rst_handle_once=%d\n",
				p,
				hdmi_in(HD21_channel_align_b_reg)&0x1,
				hdmi_in(HD21_channel_align_g_reg)&0x1,
				hdmi_in(HD21_channel_align_r_reg)&0x1,
				hdmi_in(HD21_channel_align_c_reg)&0x1, 
				err[0],err[1],err[2],err[3],
				lib_hdmi_scdc_read(p,  SCDC_UPDATE_FLAGS),
				lib_hdmi_scdc_read(p,  SCDC_STATUS_FLAGS),
				lib_hdmi_scdc_read(p,  SCDC_STATUS_FLAGS_1),
				lib_hdmi_scdc_read(p,  SCDC_STATUS_FLAGS_2),
				m_disparity_rst_handle_once);
			}
//#endif

#if PHY_ADAPTIVE_BY_LTP_PAT_AFTER_LTP_PASS
			lib_hdmi_hd21_ltp_phy_adaptive_check(p,phy_st[p].lane_num);
#endif			

			//if (phy_st[p].disp_max_cnt <= HD21_MAX_ERR_CHECK_CNT)
			if (m_disparity_rst_handle_once)
			{
				//lib_hdmi21_phy_error_handling(p);
			}


		}

		break;

	default:
		phy_st[p].phy_proc_state = PHY_PROC_INIT;
		break;

	}
}


void newbase_rxphy_tmds_job(unsigned char p)
{

	
	switch (phy_st[p].phy_proc_state) {
	case PHY_PROC_INIT:
#if 1 //def FORCE_SCRIPT_BASED_PHY_SETTING
		if (newbase_hdmi21_set_phy(p, phy_st[p].clk,phy_st[p].frl_mode,phy_st[p].lane_num))
			newbase_rxphy_reset_setphy_proc(p);

		newbase_hdmi2p0_check_tmds_config(p);
		newbase_hdmi2p0_detect_config(p);
		newbase_hdmi_reset_meas_counter(p);
		phy_st[p].phy_proc_state= PHY_PROC_DONE;
#else
		if(newbase_hdmi21_set_phy(p, phy_st[p].clk,phy_st[p].frl_mode,phy_st[p].lane_num))
			newbase_rxphy_reset_setphy_proc(p);	
		newbase_hdmi_init_disparity_ltp_var(p);
		newbase_hdmi_reset_meas_counter(p);

		newbase_rxphy_set_apply_clock(p,phy_st[p].clk);
		newbase_hdmi_open_err_detect(p);
//		newbase_hdmi2p0_inc_scdc_toggle(p);

		if (phy_st[p].clk <= clock_bound_45m) {
			phy_st[p].phy_proc_state= PHY_PROC_CLOSE_ADP;
		} else {
			phy_st[p].phy_proc_state= PHY_PROC_RECORD;
		}
#endif
		break;
	case PHY_PROC_RECORD:
		if (newbase_hdmi21_tmds_dfe_record(p, phy_st[p].clk)) {
			phy_st[p].phy_proc_state = PHY_PROC_CLOSE_ADP;
			// go to PHY_PROC_CLOSE_ADP without taking a break;
		} else {
			break;
		}
	case PHY_PROC_CLOSE_ADP:
		if (newbase_hdmi21_tmds_dfe_close(p, phy_st[p].clk,phy_st[p].lane_num,PHY_PROC_CLOSE_ADP)) {

			newbase_hdmi2p0_detect_config(p);

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
			//phy_st[p].phy_proc_state = PHY_PROC_INIT;
			newbase_rxphy_reset_setphy_proc(p);
		}
		break;
	case PHY_PROC_DONE:
		phy_st[p].recovery = 0;

		if (lib_hdmi_get_fw_debug_bit(DEBUG_18_PHY_DBG_MSG))
			debug_hdmi_dump_msg(p);
		#if HDMI20_PHY_ERROR_CORRECTION_EN
			lib_hdmi20_phy_error_handling(p);
		#endif

		break;




	/**********************************************************************/
	//
	//	Belowing are for recovery flow in check_mode stage.
	//
	/**********************************************************************/
	case PHY_PROC_RECOVER_6G_LONGCABLE:
		newbase_hdmi21_tmds_dfe_hi_speed(p, phy_st[p].clk,phy_st[p].lane_num);
		newbase_hdmi_err_detect_add(p);
		phy_st[p].phy_proc_state = PHY_PROC_RECOVER_6G_LONGCABLE_RECORD;
		break;
	case PHY_PROC_RECOVER_6G_LONGCABLE_RECORD:
		if (newbase_hdmi21_tmds_dfe_record(p, phy_st[p].clk)) {
			phy_st[p].phy_proc_state = PHY_PROC_RECOVER_6G_LONGCABLE_CLOSE_ADP;
			// go to PHY_PROC_RECOVER_HI_CLOSE_ADP;
		} else {
			break;
		}
	case PHY_PROC_RECOVER_6G_LONGCABLE_CLOSE_ADP:
		newbase_hdmi21_tmds_dfe_close(p, phy_st[p].clk, phy_st[p].lane_num,PHY_PROC_RECOVER_6G_LONGCABLE_CLOSE_ADP);
		phy_st[p].phy_proc_state = PHY_PROC_DONE;
		break;

	case PHY_PROC_RECOVER_HI:
		newbase_hdmi21_tmds_dfe_hi_speed(p, phy_st[p].clk,phy_st[p].lane_num);
		newbase_hdmi_err_detect_add(p);
		phy_st[p].phy_proc_state = PHY_PROC_RECORD;
		break;


	case PHY_PROC_RECOVER_MI:
		newbase_hdmi21_tmds_dfe_mi_speed(p, phy_st[p].clk,phy_st[p].lane_num);
		newbase_hdmi_err_detect_add(p);
		phy_st[p].phy_proc_state = PHY_PROC_RECORD;
		break;

	case PHY_PROC_RECOVER_MID_BAND_LONGCABLE:
		newbase_hdmi21_tmds_dfe_mid_adapthve(p, phy_st[p].clk);
		newbase_hdmi_err_detect_add(p);
		phy_st[p].phy_proc_state = PHY_PROC_DONE;
		break;

	default:
		phy_st[p].phy_proc_state = PHY_PROC_INIT;
		break;
	}
}



void newbase_hdmi21_set_koffset_flag(unsigned char port,unsigned char en)
{
	phy_st[port].do_koffset = en;
}

int newbase_hdmi21_get_koffset_flag(unsigned char port)
{
	return phy_st[port].do_koffset;
}



void newbase_hdmi21_set_phy_frl_mode(unsigned port, unsigned char frl_mode)
{
	
	phy_st[port].frl_mode = frl_mode;
	
	switch (frl_mode)
	{
		case MODE_FRL_3G_3_LANE:
			phy_st[port].lane_num = HDMI_3LANE;
			break;
		case MODE_FRL_6G_3_LANE:
			phy_st[port].lane_num = HDMI_3LANE;
			break;
		case MODE_FRL_6G_4_LANE:
			phy_st[port].lane_num = HDMI_4LANE;
			break;
		case MODE_FRL_8G_4_LANE:
			phy_st[port].lane_num = HDMI_4LANE;
			break;
		case MODE_FRL_10G_4_LANE:
			phy_st[port].lane_num = HDMI_4LANE;
			break;
		case MODE_FRL_12G_4_LANE:
			phy_st[port].lane_num = HDMI_4LANE;
			break;
		default:
			phy_st[port].lane_num = HDMI_3LANE;
			break;
	}

}



int newbase_hdmi21_rxphy_get_frl_info(unsigned char port, unsigned char frl_mode)
{
	unsigned int b;
	
	switch(frl_mode)
	{
		case MODE_FRL_3G_3_LANE:
			b = FRL_3G_B;
			break;
		case MODE_FRL_6G_3_LANE:
			b = FRL_6G_B;
			break;
		case MODE_FRL_6G_4_LANE:
			b = FRL_6G_B;
			break;
		case MODE_FRL_8G_4_LANE:
			b = FRL_8G_B;
			break;
		case MODE_FRL_10G_4_LANE:
			b = FRL_10G_B;
			break;
		case MODE_FRL_12G_4_LANE:
			b = FRL_12G_B;
			break;	
		default:
			b = 0;
			break;		
	}

	return b;

}


#ifdef FORCE_SCRIPT_BASED_PHY_SETTING  // for Bringup only....
extern void newbase_hdmi_set_phy_script(unsigned char nport, unsigned char frl_mode);
#endif


unsigned char newbase_hdmi21_set_phy(unsigned char nport, unsigned int b, unsigned char frl_mode,unsigned char lane)
{
	FRL_TIMING_PARAM_T FRL_Timing;
	TMDS_TIMING_PARAM_T TMDS_Timing=0;	
	unsigned int Delta_b=30;//30 ~=3MHz TMDS_CLK = (b x 27)/256
	HDMI_WARN("%s start[p%d]b=%d frl=%d lane=%d\n", __func__, nport, b,frl_mode,lane);
	if (lib_hdmi_get_fw_debug_bit(DEBUG_18_PHY_DBG_MSG))
	{
		PHY_DBG_MSG=1;
	}
	else
	{
		PHY_DBG_MSG=0;
	}
	if (b == 0) return 1;

#ifdef HDMI_FIX_HDMI_POWER_SAVING
	newbase_hdmi_reset_power_saving_state(nport);    // reset power saving state
#endif

	lib_hdmi_mac_reset(nport);

	//Need to be modify by input timing index
	switch(b)
	{
		case FRL_12G_B: FRL_Timing = FRL_12G;break;	
		case FRL_10G_B: FRL_Timing = FRL_10G;break;	
		case FRL_8G_B:	FRL_Timing = FRL_8G;break;	
		case FRL_6G_B:	FRL_Timing = FRL_6G;break;	
		case FRL_3G_B:	FRL_Timing = FRL_3G;break;	
		default: FRL_Timing = FRL_12G;break;
	}

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
		HDMI_WARN("HDMI Port[%d] frl_mode == MODE_TMDS!! (b=%d)\n", nport, b);

		if (b > (4214+Delta_b)) 
		{
			TMDS_Timing= TMDS_5_94G;
			HDMI_WARN("HDMI Port[%d]  TMDS_5_94G! (b=%d)\n", nport, b);
		} 
		else if ((b > (3520+Delta_b)) && (b <= (4214+Delta_b))) 
		{
			TMDS_Timing= TMDS_4_445G;
			HDMI_WARN("HDMI Port[%d]  TMDS_4_445G! (b=%d)\n", nport, b);
		} 

		else if ((b > (3168+Delta_b)) && (b <= (3520+Delta_b))) 
		{
			TMDS_Timing= TMDS_3_7125G;
			HDMI_WARN("HDMI Port[%d]  TMDS_3_7125G! (b=%d)\n", nport, b);
		} 
		else if ((b > (2122+Delta_b)) && (b <= (3816+Delta_b))) 
		{
			TMDS_Timing= TMDS_2_97G;
			HDMI_WARN("HDMI Port[%d]  TMDS_2_97G! (b=%d)\n", nport, b);
		} 
		else if ((b > (1760+Delta_b)) && (b <= (2122+Delta_b))) 
		{
			TMDS_Timing= TMDS_2_2275G;
			HDMI_WARN("HDMI Port[%d]  TMDS_2_2275G! (b=%d)\n", nport, b);
		} 

		else if ((b > (1408+Delta_b)) && (b <= (1760+Delta_b))) 
		{
			TMDS_Timing= TMDS_1_85625G;
			HDMI_WARN("HDMI Port[%d]  TMDS_1_85625G! (b=%d)\n", nport, b);
		} 
		else if ((b > (1056+Delta_b)) && (b <= (1408+Delta_b))) 
		{
			TMDS_Timing= TMDS_1_485G;
			HDMI_WARN("HDMI Port[%d]  TMDS_1_485G! (b=%d)\n", nport, b);
		} 
		else if ((b > (968+Delta_b)) && (b <=(1056+Delta_b))) 
		{
			TMDS_Timing= TMDS_1_11375G;
			HDMI_WARN("HDMI Port[%d]  TMDS_1_11375G! (b=%d)\n", nport, b);
		} 
		else if ((b > (792+Delta_b)) && (b <=(880+Delta_b)))
		{
			TMDS_Timing= TMDS_0_928125G;
			HDMI_WARN("HDMI Port[%d]  TMDS_0_928125G! (b=%d)\n", nport, b);
		}
		else if ((b > (512+Delta_b)) && (b <=(792+Delta_b)))
		{
			TMDS_Timing= TMDS_0_742G;
			HDMI_WARN("HDMI Port[%d]  TMDS_0_742G! (b=%d)\n", nport, b);
		}
		else if ((b > (384+Delta_b)) && (b <=(512+Delta_b))) 
		{
			TMDS_Timing= TMDS_0_54G;
			HDMI_WARN("HDMI Port[%d]  TMDS_0_54G! (b=%d)\n", nport, b);
		} 
		else if ((b > (352+Delta_b)) && (b <=(384+Delta_b)))
		{
			TMDS_Timing= TMDS_0_405G;
			HDMI_WARN("HDMI Port[%d]  TMDS_0_405G! (b=%d)\n", nport, b);
		}
		else if ((b > (252+Delta_b)) && (b <=(352+Delta_b)))
		{
			TMDS_Timing= TMDS_0_3375G;
			HDMI_WARN("HDMI Port[%d]  TMDS_0_3375G! (b=%d)\n", nport, b);
		}
		else if ((b > (238+Delta_b)) && (b <=(252+Delta_b)))
		{
			TMDS_Timing= TMDS_0_27G;
			HDMI_WARN("HDMI Port[%d]  TMDS_0_27G! (b=%d)\n", nport, b);
		}
		else		
		{
			TMDS_Timing= TMDS_0_25G;
			HDMI_WARN("HDMI Port[%d]  TMDS_0_25G! (b=%d)\n", nport, b);
		}

	}

	udelay(1);

#ifdef FORCE_SCRIPT_BASED_PHY_SETTING  // for Bringup only....
	if (frl_mode == MODE_TMDS)
	{
		//if((TMDS_Timing== TMDS_Timing_6G)||(TMDS_Timing== TMDS_Timing_3G)||(TMDS_Timing== TMDS_Timing_1P5G)||(TMDS_Timing== TMDS_Timing_742M))
		if(0)
		{
	   		//newbase_hdmi_set_tmds_phy_script(nport, TMDS_Timing);
			//Fixed_DFE_LE_TAP0(nport,b,lane);
			newbase_hdmi21_dump_dfe_paraNew(nport,lane);		
			goto set_phy_done;
		}
	}
	else
	{
		//if((FRL_Timing== FRL_Timing_3G)||(FRL_Timing== FRL_Timing_12G)||(FRL_Timing== FRL_Timing_6G))
		if(0)
		{
	   		//newbase_hdmi_set_phy_script(nport, frl_mode);
			newbase_hdmi21_dump_dfe_paraNew(nport,lane);		
			goto set_phy_done;
		}
	}
#endif

#ifdef CONFIG_POWER_SAVING_MODE
	lib_hdmi21_cdr_rst(nport, 0, frl_mode,lane);
	lib_hdmi21_demux_ck_vcopll_rst(nport, 0, lane);
	lib_hdmi21_ac_couple_power_en(nport,1);
	lib_hdmi21_eq_pi_power_en(nport,1,frl_mode,lane);
//	lib_hdmi21_cmu_rst(port, 1);
	lib_hdmi21_cmu_pll_en(nport, 1,frl_mode);
#endif

	if (frl_mode == MODE_TMDS)
	{
		TMDS_Merged_V1p2_20220420_Main_Seq(nport,TMDS_Timing);
	}
	else
	{

		FRL_Merged_V1_20220315_Main_Seq(nport,FRL_Timing);
	}
	//Dump DFE
	newbase_hdmi21_dump_dfe_paraNew(nport,lane);	

/////////////////////////////////////////////////////////////////////////////////////////////

set_phy_done:

	if(newbase_hdcp_get_auth_mode(nport) == HDCP_OFF || newbase_hdcp_get_auth_mode(nport) == NO_HDCP )
	{// No AKE_INIT of HDCP22, open HDCP14 and HDCP22 at the same time
		HDMI_WARN("HDMI Port[%d] HDCP Enable after set phy!! (b=%d)\n", nport, b);
		newbase_hdmi_switch_hdcp_mode(nport, NO_HDCP);
		newbase_hdmi_reset_hdcp_once(nport, FALSE);
	}
	else
	{
		HDMI_WARN("HDMI Port[%d] HDCP already start before set phy!! (b=%d)\n", nport, b);
	}



	
	lib_hdmi_mac_release(nport, frl_mode);

	return 0;
}




unsigned char newbase_hdmi21_tmds_dfe_record(unsigned char nport, unsigned int clk)
{
	if (lib_hdmi_dfe_is_record_enable(nport)) {
		lib_hdmi_dfe_record_enable(nport, 0);
		return TRUE; // stop recocrding
	} else {
		lib_hdmi_dfe_record_enable(nport, 1);
		return FALSE; // start recoding min/max
	}
}


unsigned char newbase_hdmi21_frl_dfe_close(unsigned char nport, unsigned int clk, unsigned char lane_mode,unsigned char phy_state)
{
	unsigned char lane;
	
	//lib_hdmi_set_dfe_close(nport,lane_mode);
	
	for (lane=0; lane< lane_mode; lane++) {
		phy_st[nport].dfe_t[lane].tap0max = lib_hdmi_dfe_get_tap0max(nport, lane);
		phy_st[nport].dfe_t[lane].tap0min = lib_hdmi_dfe_get_tap0min(nport, lane);
		phy_st[nport].dfe_t[lane].vth = lib_hdmi_dfe_get_vthp(nport, lane);
		phy_st[nport].dfe_t[lane].tap0 = lib_hdmi_dfe_get_tap0(nport, lane);
		phy_st[nport].dfe_t[lane].tap1 = lib_hdmi_dfe_get_tap1(nport, lane);
		phy_st[nport].dfe_t[lane].tap2 = lib_hdmi_dfe_get_tap2(nport, lane);
		phy_st[nport].dfe_t[lane].tap3 = lib_hdmi_dfe_get_tap3(nport, lane);
		phy_st[nport].dfe_t[lane].tap4 = lib_hdmi_dfe_get_tap4(nport, lane);
		phy_st[nport].dfe_t[lane].le = lib_hdmi_dfe_get_le(nport, lane);
		phy_st[nport].dfe_t[lane].lemax = lib_hdmi_dfe_get_lemax(nport, lane);
		phy_st[nport].dfe_t[lane].lemin = lib_hdmi_dfe_get_lemin(nport, lane);
	}

	//newbase_hdmi21_dump_dfe_para(nport,lane_mode);

	//lib_hdmi21_dfe_power(nport, 0,lane_mode);
	return TRUE;


}



unsigned char newbase_hdmi21_tmds_dfe_close(unsigned char nport, unsigned int clk, unsigned char lane_mode,unsigned char phy_state)
{
	unsigned char lane;
	
	lib_hdmi_set_dfe_close(nport,lane_mode);

	switch (phy_state) {
	case PHY_PROC_CLOSE_ADP:
		/*
		if (clk > 2900) { // > 3G
			lib_hdmi_load_le_max(nport,lane_mode);
		} else*/ {
			lib_hdmi_dfe_examine_le_coef(nport, clk,lane_mode);
		}

		newbase_hdmi_tap2_adjust_with_le(nport,clk,lane_mode);
			
		if (newbase_hdmi_dfe_threshold_check(nport,clk,lane_mode) && (phy_st[nport].dfe_thr_chk_cnt < 30)) {
			phy_st[nport].dfe_thr_chk_cnt++;
			return FALSE;
		}

		break;
	case PHY_PROC_RECOVER_6G_LONGCABLE_CLOSE_ADP:
		newbase_hdmi21_tmds_dfe_6g_long_cable_patch(nport); //check FRL mode need this patch or not ?
		break;
	default:
		break;
	}

	
	if (phy_fac_eq_st[nport].eq_mode == EQ_MANUAL_ON) {		
		newbase_hdmi_manual_eq(nport,	phy_fac_eq_st[nport].manual_eq[0] ,phy_fac_eq_st[nport].manual_eq[1] ,phy_fac_eq_st[nport].manual_eq[2]); 
	}
	
	for (lane=0; lane< lane_mode; lane++) {
		phy_st[nport].dfe_t[lane].tap0max = lib_hdmi_dfe_get_tap0max(nport, lane);
		phy_st[nport].dfe_t[lane].tap0min = lib_hdmi_dfe_get_tap0min(nport, lane);
		phy_st[nport].dfe_t[lane].vth = lib_hdmi_dfe_get_vth(nport, lane);
		phy_st[nport].dfe_t[lane].tap0 = lib_hdmi_dfe_get_tap0(nport, lane);
		phy_st[nport].dfe_t[lane].tap1 = lib_hdmi_dfe_get_tap1(nport, lane);
		phy_st[nport].dfe_t[lane].tap2 = lib_hdmi_dfe_get_tap2(nport, lane);
		phy_st[nport].dfe_t[lane].tap3 = 0;
		phy_st[nport].dfe_t[lane].tap4 = 0;
		phy_st[nport].dfe_t[lane].le = lib_hdmi_dfe_get_le(nport, lane);
		phy_st[nport].dfe_t[lane].lemax = lib_hdmi_dfe_get_lemax(nport, lane);
		phy_st[nport].dfe_t[lane].lemin = lib_hdmi_dfe_get_lemin(nport, lane);
	}

	newbase_hdmi21_dump_dfe_para(nport,lane_mode);

	return TRUE;
}



void newbase_hdmi21_tmds_dfe_hi_speed(unsigned char nport, unsigned int b_clk,unsigned char lane_mode)
{  
	HDMI_PRINTF("%s\n", __func__);
#if 0

	lib_hdmi21_dfe_power(nport, 1,lane_mode);
	lib_hdmi21_dfe_param_init(nport,b_clk,2,lane_mode);

	lib_dfe_tap0_le_adp_en(nport,lane_mode,1);
	udelay(1);
	lib_hdmi_dfe_record_enable(nport, 1);
	udelay(500);
	lib_hdmi_dfe_record_enable(nport, 0);
	lib_dfe_tap0_disable(nport,lane_mode);
	lib_hdmi_load_tap0_max(nport,lane_mode);
	udelay(100);
	lib_dfe_le_disable(nport,lane_mode);
	lib_hdmi21_dump_dfe_tap0_le(nport,lane_mode);
	if (b_clk > 2700) {  // 3G need tap2 but below not need
		// ENABLE  TAP0~TAP2
		hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_TAP2),(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_TAP2));
		hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_TAP2),(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_TAP2));
		hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_TAP2),(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_TAP2));
		if (lane_mode == HDMI_4LANE)
				hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_TAP1|REG_dfe_adapt_en_lane3_TAP2),(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_TAP1|REG_dfe_adapt_en_lane3_TAP2));

	} else {

		// ENABLE  TAP0~TAP1
		hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_TAP1),(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_TAP1));
		hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_TAP1),(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_TAP1));
		hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_TAP1),(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_TAP1));
		if (lane_mode == HDMI_4LANE)
				hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_TAP1),(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_TAP1));

	}
#endif
	newbase_hdmi21_set_koffset_flag(nport,0); //recover not do koffset , becasue input off will mute display
}

void newbase_hdmi21_tmds_dfe_6g_long_cable_patch(unsigned char nport)
{   
	signed char tap2;
	unsigned char lemax;

	tap2 = lib_hdmi_dfe_get_tap2(nport, 1);
	lib_hdmi_dfe_init_tap2(nport, LN_G, MIN(tap2+6, 10));

	lemax = lib_hdmi_dfe_get_lemax(nport, 1);
	lemax += lib_hdmi_dfe_get_tap1max(nport, 1);
	lib_hdmi_dfe_init_tap1(nport, LN_G, MIN(lemax+4, 63));

	HDMI_EMG("%s end\n", __func__);
}


void newbase_hdmi21_tmds_dfe_mi_speed(unsigned char nport, unsigned int b_clk,unsigned char lane_mode)
{
	HDMI_PRINTF("%s\n", __func__);
#if 0
	lib_hdmi21_dfe_power(nport, 1,lane_mode);
	lib_hdmi21_dfe_param_init(nport,b_clk,2,lane_mode);

	lib_dfe_tap0_le_adp_en(nport,lane_mode,1);
	udelay(1);
	lib_hdmi_dfe_record_enable(nport, 1);
	udelay(500);
	lib_hdmi_dfe_record_enable(nport, 0);
	lib_dfe_tap0_disable(nport,lane_mode);
	lib_hdmi_load_tap0_max(nport,lane_mode);
	udelay(100);
	lib_dfe_le_disable(nport,lane_mode);
	lib_hdmi21_dump_dfe_tap0_le(nport,lane_mode);
#if 0
	if (b_clk > 2700) {  // 3G need tap2 but below not need
		// ENABLE  TAP0~TAP2
		hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_TAP2),(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_TAP1|REG_dfe_adapt_en_lane0_TAP2));
		hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_TAP2),(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_TAP1|REG_dfe_adapt_en_lane1_TAP2));
		hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_TAP2),(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_TAP1|REG_dfe_adapt_en_lane2_TAP2));
		if (lane_mode == HDMI_4LANE)
				hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_TAP1|REG_dfe_adapt_en_lane3_TAP2),(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_TAP1|REG_dfe_adapt_en_lane3_TAP2));

	} else {

		// ENABLE  TAP0~TAP1
		hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_TAP1),(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_TAP1));
		hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_TAP1),(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_TAP1));
		hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_TAP1),(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_TAP1));
		if (lane_mode == HDMI_4LANE)
				hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_TAP1),(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_TAP1));

	}	
#endif
#endif
	newbase_hdmi21_set_koffset_flag(nport,0); //recover not do koffset , becasue input off will mute display

}

void newbase_hdmi21_tmds_dfe_mid_adapthve(unsigned char nport, unsigned int b_clk)
{
	unsigned char le0;

//	HDMI_PRINTF("%s\n", __func__);
	le0 = lib_hdmi_dfe_get_le(nport,0);

	if (le0 >= 8)
		return;
//	le1 = lib_hdmi_dfe_get_le(nport,1);
//	le2 = lib_hdmi_dfe_get_le(nport,2);

	lib_hdmi_dfe_init_le(nport, (LN_R|LN_G|LN_B), (le0 + 2));

	HDMI_PRINTF("le=%x\n", (le0 + 2));

}




void newbase_hdmi21_dump_dfe_para(unsigned char nport, unsigned char lane_mode)
{
	unsigned char lane;

	for (lane=0; lane<lane_mode; lane++) {
		DFE_WARN("HDMI[p%d] ********************Lane%d******************\n", nport,lane);
		DFE_WARN("HDMI[p%d]  Lane%d Vth=0x%x\n", nport,lane, phy_st[nport].dfe_t[lane].vth);
		//DFE_WARN("HDMI[p%d] Lane%d Tap0=0x%x, max=0x%02x, min=0x%02x\n",nport,
		//	lane, phy_st[nport].dfe_t[lane].tap0, phy_st[nport].dfe_t[lane].tap0max, phy_st[nport].dfe_t[lane].tap0min);
		DFE_WARN("HDMI[p%d] Lane%d Tap0=0x%x, max=0x%02x\n",nport,
			lane, phy_st[nport].dfe_t[lane].tap0, phy_st[nport].dfe_t[lane].tap0max);
		DFE_WARN("HDMI[p%d]  Lane%d Tap1=0x%02x, Tap2=%+d Tap3=%+d Tap4=%+d\n",
			nport,lane, phy_st[nport].dfe_t[lane].tap1, phy_st[nport].dfe_t[lane].tap2,phy_st[nport].dfe_t[lane].tap3,phy_st[nport].dfe_t[lane].tap4);
		//DFE_WARN("HDMI[p%d]  Lane%d LE=0x%02x, max=0x%02x, min=0x%02x\n",
		//	nport,lane, phy_st[nport].dfe_t[lane].le, phy_st[nport].dfe_t[lane].lemax, phy_st[nport].dfe_t[lane].lemin);
		DFE_WARN("HDMI[p%d]  Lane%d LE=0x%02x\n",
			nport,lane, phy_st[nport].dfe_t[lane].le);
	}

}


void newbase_hdmi21_dump_dfe_paraNew(unsigned char nport, unsigned char lane_mode)
{
	unsigned char lane;

	for (lane=0; lane<3; lane++) {
		phy_st[nport].dfe_t[lane].tap0max = lib_hdmi_dfe_get_tap0max(nport, lane);
		phy_st[nport].dfe_t[lane].tap0min = lib_hdmi_dfe_get_tap0min(nport, lane);
		phy_st[nport].dfe_t[lane].vth = lib_hdmi_dfe_get_vthp(nport, lane);
		phy_st[nport].dfe_t[lane].tap0 = lib_hdmi_dfe_get_tap0(nport, lane);
		phy_st[nport].dfe_t[lane].tap1 = lib_hdmi_dfe_get_tap1(nport, lane);
		phy_st[nport].dfe_t[lane].tap2 = lib_hdmi_dfe_get_tap2(nport, lane);
		phy_st[nport].dfe_t[lane].le = lib_hdmi_dfe_get_le(nport, lane);
		phy_st[nport].dfe_t[lane].lemax = lib_hdmi_dfe_get_lemax(nport, lane);
		phy_st[nport].dfe_t[lane].lemin = lib_hdmi_dfe_get_lemin(nport, lane);
	}

	for (lane=0; lane<lane_mode; lane++) {
		DFE_WARN("HDMI[p%d] ********************Lane%d******************\n", nport,lane);
		DFE_WARN("HDMI[p%d]  Lane%d Vthp=0x%x\n", nport,lane, phy_st[nport].dfe_t[lane].vth);
		//DFE_WARN("HDMI[p%d] Lane%d Tap0=0x%x, max=0x%02x, min=0x%02x\n",nport,
		//	lane, phy_st[nport].dfe_t[lane].tap0, phy_st[nport].dfe_t[lane].tap0max, phy_st[nport].dfe_t[lane].tap0min);
		DFE_WARN("HDMI[p%d] Lane%d Tap0=0x%x, max=0x%02x\n",nport,
			lane, phy_st[nport].dfe_t[lane].tap0, phy_st[nport].dfe_t[lane].tap0max);
		DFE_WARN("HDMI[p%d]  Lane%d Tap1=0x%02x, Tap2=%+d Tap3=%+d Tap4=%+d\n",
			nport,lane, phy_st[nport].dfe_t[lane].tap1, phy_st[nport].dfe_t[lane].tap2,phy_st[nport].dfe_t[lane].tap3,phy_st[nport].dfe_t[lane].tap4);
		//DFE_WARN("HDMI[p%d]  Lane%d LE=0x%02x, max=0x%02x, min=0x%02x\n",
		//	nport,lane, phy_st[nport].dfe_t[lane].le, phy_st[nport].dfe_t[lane].lemax, phy_st[nport].dfe_t[lane].lemin);
		DFE_WARN("HDMI[p%d]  Lane%d LE=0x%02x\n",
			nport,lane, phy_st[nport].dfe_t[lane].le);

	}

}


void lib_hdmi21_dump_dfe_tap0_le(unsigned char nport,unsigned char lane_mode)
{

	unsigned char lane;

	for (lane=0; lane< lane_mode; lane++) {
		phy_st[nport].dfe_t[lane].tap0max = lib_hdmi_dfe_get_tap0max(nport, lane);
		phy_st[nport].dfe_t[lane].tap0min = lib_hdmi_dfe_get_tap0min(nport, lane);
		phy_st[nport].dfe_t[lane].vth = lib_hdmi_dfe_get_vth(nport, lane);
		phy_st[nport].dfe_t[lane].tap0 = lib_hdmi_dfe_get_tap0(nport, lane);
		phy_st[nport].dfe_t[lane].le = lib_hdmi_dfe_get_le(nport, lane);
		phy_st[nport].dfe_t[lane].lemax = lib_hdmi_dfe_get_lemax(nport, lane);
		phy_st[nport].dfe_t[lane].lemin = lib_hdmi_dfe_get_lemin(nport, lane);
	}

	for (lane=0; lane<lane_mode; lane++) {
		DFE_WARN("HDMI[p%d] ********************Lane%d******************\n",nport, lane);
		DFE_WARN("HDMI[p%d]  Lane%d Vth=0x%x\n",nport, lane, phy_st[nport].dfe_t[lane].vth);
		//DFE_WARN("HDMI[p%d] Lane%d Tap0=0x%x, max=0x%02x, min=0x%02x\n",nport,
		//	lane, phy_st[nport].dfe_t[lane].tap0, phy_st[nport].dfe_t[lane].tap0max, phy_st[nport].dfe_t[lane].tap0min);
		DFE_WARN("HDMI[p%d]  Lane%d Tap0=0x%x, max=0x%02x, \n",nport,lane, phy_st[nport].dfe_t[lane].tap0, phy_st[nport].dfe_t[lane].tap0max);
		//DFE_WARN("HDMI[p%d] Lane%d LE=0x%02x, max=0x%02x, min=0x%02x\n",nport,lane, phy_st[nport].dfe_t[lane].le, phy_st[nport].dfe_t[lane].lemax, phy_st[nport].dfe_t[lane].lemin);
		DFE_WARN("HDMI[p%d] Lane%d LE=0x%02x\n",nport,lane, phy_st[nport].dfe_t[lane].le);
	}
	
}


#ifdef CONFIG_POWER_SAVING_MODE
void newbase_hdmi21_phy_pw_saving(unsigned char port)
{	

	lib_hdmi21_cdr_rst(port, 1, MODE_TMDS,HDMI_4LANE);
	lib_hdmi21_demux_ck_vcopll_rst(port, 1, HDMI_4LANE);

	lib_hdmi21_ac_couple_power_en(port,0);
	lib_hdmi21_eq_pi_power_en(port,0,MODE_TMDS,HDMI_4LANE);
//	lib_hdmi21_cmu_rst(port, 1);
	lib_hdmi21_cmu_pll_en(port, 0,MODE_TMDS);
	
}

#endif





void lib_hdmi21_hysteresis_en(unsigned char en)
{
	unsigned char nport;
	if (en) 
	{
		for (nport=0; nport<HDMI_PORT_TOTAL_NUM; nport++)
		{
			if (newbase_hdmi_phy_port_status(nport) == HDMI_PHY_PORT_NOTUSED)
				continue;
			hdmi_mask(HDMIRX_PHY_ck_cmu_1_reg,~(cmu_ck_1_HYS_AMP_EN|cmu_ck_1_MD_DET_SRC|cmu_ck_1_HYS_WIN_SEL|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN),
			(cmu_ck_1_HYS_AMP_EN|cmu_ck_1_NON_HYS_AMP_EN|cmu_ck_1_MD_DET_SRC|cmu_ck_1_ENHANCE_BIAS_10|cmu_ck_1_CKIN_SEL));
		}
	}
	else 
	{
		for (nport=0; nport<HDMI_PORT_TOTAL_NUM; nport++)
		{
			if (newbase_hdmi_phy_port_status(nport) == HDMI_PHY_PORT_NOTUSED)
				continue;
			hdmi_mask(HDMIRX_PHY_ck_cmu_1_reg,~(cmu_ck_1_HYS_AMP_EN|cmu_ck_1_MD_DET_SRC|cmu_ck_1_HYS_WIN_SEL|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN),
			(cmu_ck_1_ENHANCE_BIAS_01|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN));
		}
	}
}

void lib_hdmi21_stb_tmds_clk_wakeup_en(unsigned char en)
{
	unsigned char nport;
	
	if (en) {

		for (nport=0; nport<HDMI_PORT_TOTAL_NUM; nport++)
		{
			//HDMI Z0 set
			hdmi_mask(HDMIRX_PHY_z0pow_reg, ~(Z0_Z0POW),Z0_Z0POW); //Z0 power on
			hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(Z0_P_OFF|Z0_N_OFF), 0); //[1] Noff [0]poff :0 =>normal
			hdmi_mask(HDMIRX_PHY_top_in_2_reg, ~(BIAS_EN), BIAS_EN); //MBIAS setting

			//Enable switch and 3.3V hysteresis amp
			hdmi_mask(HDMIRX_PHY_top_in_0_reg,~(AC_Couple_Enable|CK_Detect_Enable),(AC_Couple_Enable|CK_Detect_Enable));
			hdmi_mask(HDMIRX_PHY_top_in_0_reg,~(Divider_Enable),0);	//REG_TOP_IN_3[7]		"divider enable(divide-by-32). 0:off, 1:on
			hdmi_mask(HDMIRX_PHY_top_in_0_reg,~(Divider_Enable),Divider_Enable);	//(note: when turn on divider, must set 0-->1)"
			hdmi_mask(HDMIRX_PHY_top_in_0_reg,~(Hysteresis_Window_Sel),0);	//REG_TOP_IN_3[3:2] 	hysteresis window select. 00:60mV, 01:72mV, 10:85mV, 11:99mV
			hdmi_mask(HDMIRX_PHY_top_in_2_reg,~(CK_LATCH),0);	// REG_CK_LATCH=0, CK_LATCH=0
			hdmi_mask(HDMIRX_PHY_top_in_2_reg,~(CK_LATCH),CK_LATCH);	// REG_CK_LATCH=1, CK_LATCH=1
		}

	}else {
		for (nport=0; nport<HDMI_PORT_TOTAL_NUM; nport++)
		{
			hdmi_mask(HDMIRX_PHY_top_in_0_reg,~(Divider_Enable|AC_Couple_Enable|CK_Detect_Enable),0);
		}
	}
}



//static FRL_TIMING_PARAM_T_ORI FRL_Timing_last[4];
//static TMDS_TIMING_PARAM_T_ORI TMDS_Timing_last[4];
void lib_hdmi21_phy_init(void)
{
	unsigned char nport;
	
	for (nport=0; nport<HDMI_PORT_TOTAL_NUM; nport++)
	{
		if (newbase_hdmi_phy_port_status(nport) == HDMI_PHY_PORT_NOTUSED)
			continue;
		//newbase_hdmi_set_phy_script(nport, 0);
		 //=====[DPHY_Fix]=====//
		 //DPHY fix start
		DPHY_Fix_tmds_1_func(nport, TMDS_5_94G);
		//=====[APHY_Fix]=====//
		 //APHY fix start
		APHY_Fix_tmds_1_func(nport, TMDS_5_94G);

	}

	lib_hdmi21_z0_calibration();

	for (nport=0; nport<HDMI_PORT_TOTAL_NUM; nport++)
	{
		if (newbase_hdmi_phy_port_status(nport) == HDMI_PHY_PORT_NOTUSED)
			continue;
		lib_hdmi21_z300_sel(nport, 0);
		lib_fast_swtch_mode(nport,PS_NORMAL_MODE);
		DFE_ini_tmds_1_func(nport, TMDS_5_94G);;
		//FRL_Timing_last[nport]=FRL_Timing_Changed;
		//TMDS_Timing_last[nport]=TMDS_Timing_Changed;
	}

	return;
}


void lib_hdmi21_z0_set(unsigned char nport, unsigned char lane, unsigned char enable)
{
#if 1
	unsigned int bmask = 0;

	if (enable == 0)
		hdmi_mask(HDMIRX_PHY_top_in_2_reg, ~(CK_LATCH), 0);  //latch bit clear

	if (lane&LN_CK) {
		bmask |= HDMIRX_PHY_Z0POW_get_reg_z0_z0pow_ck(1);
	}
	if (lane&LN_B) {
		bmask |= HDMIRX_PHY_Z0POW_get_reg_z0_z0pow_b(1);
	}
	if (lane&LN_G) {
		bmask |= HDMIRX_PHY_Z0POW_get_reg_z0_z0pow_g(1);
	}
	if (lane&LN_R) {
		bmask |= HDMIRX_PHY_Z0POW_get_reg_z0_z0pow_r(1);
	}
	
	if (!bmask) {
		return;
	}

	
	hdmi_mask(HDMIRX_PHY_z0pow_reg, ~bmask, enable?bmask:0);
	hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~HD21_TOP_IN_Z0_FIX_SELECT_0, enable?(HD21_TOP_IN_Z0_FIX_SELECT_0):0 );

	if (enable == 0)
		hdmi_mask(HDMIRX_PHY_top_in_2_reg, ~(CK_LATCH), CK_LATCH);  //latch bit set
#endif		
}


void lib_hdmi21_z300_sel(unsigned char nport, unsigned char mode)  // 0: active mode 1:vcm mode
{
#if	1
	if (mode) {
		hdmi_mask(HDMIRX_PHY_z0pow_reg, ~HDMIRX_PHY_Z0POW_reg_z0_z300_sel_mask, HDMIRX_PHY_Z0POW_reg_z0_z300_sel(1));
	} else {
		hdmi_mask(HDMIRX_PHY_z0pow_reg, ~HDMIRX_PHY_Z0POW_reg_z0_z300_sel_mask, HDMIRX_PHY_Z0POW_reg_z0_z300_sel(0));
	}
#endif
}




void lib_hdmi_cmu_rst(unsigned char port, unsigned char rst)
{
#if 0  //h5 check after
	const unsigned int reg_ck3[HDMI_PORT_TOTAL_NUM] = {
		DFE_HDMI_RX_PHY_P0_CK3_reg,
		DFE_HDMI_RX_PHY_P1_CK3_reg,
		DFE_HDMI_RX_PHY_P2_CK3_reg,
		DFE_HDMI_RX_PHY_P0_HD21_P0_CMU3_reg
	};

	if (port >= HDMI_PORT_TOTAL_NUM) {
		return;
	}

	if (rst) {
		hdmi_mask(reg_ck3[port], ~(P0_ck_9_CMU_PFD_RSTB|_BIT3|P0_ck_9_CMU_WDRST|_BIT1), (_BIT3|P0_ck_9_CMU_WDRST));	//PFD_RSTB  reset
	} else {
		hdmi_mask(reg_ck3[port], ~(P0_ck_9_CMU_PFD_RSTB|_BIT3|P0_ck_9_CMU_WDRST|_BIT1), (P0_ck_9_CMU_PFD_RSTB|_BIT1));  //release, normal
	}
#endif
}

#if 0
void lib_hdmi21_dfe_power(unsigned char nport, unsigned char en, unsigned char lane_mode)
{
	hdmi_mask(HDMIRX_PHY_r2_reg, ~(b_9_DFE_ADAPTION_POW_EN|b_9_DFE_TAP_EN), (HD21_RX_PHY_P0_DFE_ADAPTION_EN(en)|HD21_RX_PHY_P0_b_tap_en(0xf)));
	hdmi_mask(HDMIRX_PHY_r2_reg, ~(b_9_DFE_ADAPTION_POW_EN|b_9_DFE_TAP_EN), (HD21_RX_PHY_P0_DFE_ADAPTION_EN(en)|HD21_RX_PHY_P0_b_tap_en(0xf)));
	hdmi_mask(HDMIRX_PHY_b2_reg, ~(b_9_DFE_ADAPTION_POW_EN|b_9_DFE_TAP_EN), (HD21_RX_PHY_P0_DFE_ADAPTION_EN(en)|HD21_RX_PHY_P0_b_tap_en(0xf)));

	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_ck2_reg, ~(b_9_DFE_ADAPTION_POW_EN|b_9_DFE_TAP_EN), (HD21_RX_PHY_P0_DFE_ADAPTION_EN(en)|HD21_RX_PHY_P0_b_tap_en(0xf)));
}
#endif
void lib_hdmi21_eq_set_clock_boundary(unsigned char port)
{

        
            clock_bound_3g = TMDS_3G;
            clock_bound_1p5g = TMDS_1p5G;
            clock_bound_45m = 430;
            clock_bound_110m = 1042;
        

        //HDMI_PRINTF("clock_bound_3g=%d\n", clock_bound_3g);

}

#ifdef CONFIG_POWER_SAVING_MODE

void lib_hdmi21_ac_couple_power_en(unsigned char nport,unsigned char en)
{
#if 1
	if (en) {
		hdmi_mask(HDMIRX_PHY_b0_reg, ~RXVCM_SEL, RXVCM_10);
		hdmi_mask(HDMIRX_PHY_g0_reg, ~RXVCM_SEL, RXVCM_10);
		hdmi_mask(HDMIRX_PHY_r0_reg, ~RXVCM_SEL, RXVCM_10);
		hdmi_mask(HDMIRX_PHY_ck0_reg, ~RXVCM_SEL, RXVCM_10);		
		
		hdmi_mask(HDMIRX_PHY_b0_reg, ~AC_COPULE_POW, AC_COPULE_POW);
		hdmi_mask(HDMIRX_PHY_g0_reg, ~AC_COPULE_POW, AC_COPULE_POW);
		hdmi_mask(HDMIRX_PHY_r0_reg, ~AC_COPULE_POW, AC_COPULE_POW);
		hdmi_mask(HDMIRX_PHY_ck0_reg, ~AC_COPULE_POW, AC_COPULE_POW);  //check power consumption for 3 lane frl mode
	} else {
		hdmi_mask(HDMIRX_PHY_b0_reg, ~AC_COPULE_POW, 0x0);
		hdmi_mask(HDMIRX_PHY_g0_reg, ~AC_COPULE_POW, 0x0);
		hdmi_mask(HDMIRX_PHY_r0_reg, ~AC_COPULE_POW, 0x0);
		hdmi_mask(HDMIRX_PHY_ck0_reg, ~AC_COPULE_POW, AC_COPULE_POW);   //for hysteresis amp always open

	}
#endif
}


void lib_hdmi21_eq_pi_power_en(unsigned char nport,unsigned char en, unsigned char frl_mode,unsigned char lane_mode)
{



#if 1
	//add ac couple power
	if (en) {

		hdmi_mask(HDMIRX_PHY_b1_reg, ~(POW_LEQ|RS_CAL_EN),  POW_LEQ|RS_CAL_EN);
		hdmi_mask(HDMIRX_PHY_g1_reg, ~(POW_LEQ|RS_CAL_EN) , POW_LEQ|RS_CAL_EN);
		hdmi_mask(HDMIRX_PHY_r1_reg, ~(POW_LEQ|RS_CAL_EN), POW_LEQ|RS_CAL_EN);

		
		if (lane_mode == HDMI_4LANE) {
			hdmi_mask(HDMIRX_PHY_ck1_reg, ~(POW_LEQ|RS_CAL_EN),POW_LEQ|RS_CAL_EN);	
		} else {
			hdmi_mask(HDMIRX_PHY_ck1_reg, ~(POW_LEQ|RS_CAL_EN), 0x0);	
		}
#if 0		
		udelay(1);
		
		if (frl_mode)	 {
				hdmi_mask(HDMIRX_PHY_B1_reg, ~(b_8_POW_PR), 0x0);
				hdmi_mask(HDMIRX_PHY_G1_reg, ~(b_8_POW_PR), 0x0);
				hdmi_mask(HDMIRX_PHY_R1_reg, ~(b_8_POW_PR), 0x0);
				hdmi_mask(HDMIRX_PHY_CK1_reg, ~(b_8_POW_PR), 0x0);

		} else {
		hdmi_mask(HDMIRX_PHY_B1_reg, ~(b_8_POW_PR), b_8_POW_PR);
		hdmi_mask(HDMIRX_PHY_G1_reg, ~(b_8_POW_PR), b_8_POW_PR);
		hdmi_mask(HDMIRX_PHY_R1_reg, ~(b_8_POW_PR), b_8_POW_PR);
		}
#endif		
	} else {
		// 4 lane all close
		hdmi_mask(HDMIRX_PHY_b1_reg, ~(POW_LEQ|RS_CAL_EN),  0x0);
		hdmi_mask(HDMIRX_PHY_g1_reg, ~(POW_LEQ|RS_CAL_EN) , 0x0);
		hdmi_mask(HDMIRX_PHY_r1_reg, ~(POW_LEQ|RS_CAL_EN),  0x0);
		hdmi_mask(HDMIRX_PHY_ck1_reg, ~(POW_LEQ|RS_CAL_EN), 0x0);	
	}
#endif	
}
#endif


//cdr & dcdr mode ck lane & data lane enable
void lib_hdmi21_cmu_pll_en(unsigned char nport, unsigned char enable, unsigned char frl_mode)
{

#if 1
	if (enable) {	
		if (frl_mode == MODE_TMDS)	
			hdmi_mask(HDMIRX_PHY_ck_cmu_1_reg,~(cmu_ck_1_CKIN_SEL),cmu_ck_1_CKIN_SEL);
		else		//frl mode chage to crystal clock
			hdmi_mask(HDMIRX_PHY_ck_cmu_1_reg,~(cmu_ck_1_CKIN_SEL),0x0);
	} else { //disable default tmds clock
		hdmi_mask(HDMIRX_PHY_ck_cmu_1_reg,~(cmu_ck_1_CKIN_SEL),cmu_ck_1_CKIN_SEL);
	}


	if (enable) {
		if (frl_mode == MODE_TMDS) {
			//only setting ck lane
			hdmi_mask(HDMIRX_PHY_fld_regd18,~(acdr_manual_RDY|acdr_manual_CMU_SEL),0x0);//
			hdmi_mask(HDMIRX_PHY_fld_regd11,~(acdr_manual_RDY|acdr_manual_CMU_SEL),0x0);
			hdmi_mask(HDMIRX_PHY_fld_regd04,~(acdr_manual_RDY|acdr_manual_CMU_SEL), 0x0);
			hdmi_mask(HDMIRX_PHY_fld_regd25,~(acdr_manual_RDY|acdr_manual_CMU_SEL), (acdr_manual_RDY_FROM_REG));
			//r lane off
			hdmi_mask(HDMIRX_PHY_acdr_r0_reg,~(acdr_PFD_RSTB|acdr_DIV_IQ_RSTB), 0x0);
			hdmi_mask(HDMIRX_PHY_acdr_r1_reg,~(acdr_VCO_EN), 0x0);//0: 0ff
			//g lane off
			hdmi_mask(HDMIRX_PHY_acdr_g0_reg,~(acdr_PFD_RSTB|acdr_DIV_IQ_RSTB), 0x0);
			hdmi_mask(HDMIRX_PHY_acdr_g1_reg,~(acdr_VCO_EN), 0x0);//0: 0ff
			//b lane off
			hdmi_mask(HDMIRX_PHY_acdr_b0_reg,~(acdr_PFD_RSTB|acdr_DIV_IQ_RSTB), 0x0);
			hdmi_mask(HDMIRX_PHY_acdr_b1_reg,~(acdr_VCO_EN), 0x0);//0: 0ff
			//ck lane on (pll enable)
			hdmi_mask(HDMIRX_PHY_acdr_ck0_reg,~(acdr_PFD_RSTB|acdr_DIV_IQ_RSTB), acdr_PFD_RSTB|acdr_DIV_IQ_RSTB);
			hdmi_mask(HDMIRX_PHY_acdr_ck1_reg,~(acdr_VCO_EN), acdr_VCO_EN);//1: on
		} else {
			
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
	else {
		
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
		hdmi_mask(HDMIRX_PHY_acdr_ck1_reg,~(acdr_VCO_EN), 0x0);//0: 0ff

	}
#endif	

}


void lib_hdmi21_demux_ck_vcopll_rst(unsigned char nport, unsigned char rst, unsigned char lane_mode)
{
// rst = 1: reset
// rst = 0: normal working operation
#if 1
	if (rst) {
		lib_hdmi_dfe_rst(nport, 1);
		hdmi_mask(HDMIRX_PHY_rega00, ~(HDMIRX_PHY_HD21_REGA00_p0_r_demux_rstb_mask|HDMIRX_PHY_HD21_REGA00_p0_g_demux_rstb_mask|HDMIRX_PHY_HD21_REGA00_p0_b_demux_rstb_mask), 0);
		if (lane_mode == HDMI_4LANE) 
			hdmi_mask(HDMIRX_PHY_rega00, ~HDMIRX_PHY_HD21_REGA00_p0_ck_demux_rstb_mask, 0);
		
	} else {

		hdmi_mask(HDMIRX_PHY_rega00, ~(HDMIRX_PHY_HD21_REGA00_p0_r_demux_rstb_mask|HDMIRX_PHY_HD21_REGA00_p0_g_demux_rstb_mask|HDMIRX_PHY_HD21_REGA00_p0_b_demux_rstb_mask), (HDMIRX_PHY_HD21_REGA00_p0_r_demux_rstb_mask|HDMIRX_PHY_HD21_REGA00_p0_g_demux_rstb_mask|HDMIRX_PHY_HD21_REGA00_p0_b_demux_rstb_mask));
		
		if (lane_mode == HDMI_4LANE)
			hdmi_mask(HDMIRX_PHY_rega00, ~HDMIRX_PHY_HD21_REGA00_p0_ck_demux_rstb_mask, HDMIRX_PHY_HD21_REGA00_p0_ck_demux_rstb_mask);
		
		lib_hdmi_dfe_rst(nport, 0);
	}
#endif
}


void lib_hdmi21_8b18b_fifo_rst(unsigned char nport, unsigned char rst,unsigned char frl_mode,unsigned char lane_mode)
{
#if 0
	const unsigned int fifo_mask = (HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_p0_r_fifo_rst_n_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_p0_g_fifo_rst_n_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_p0_b_fifo_rst_n_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_p0_ck_fifo_rst_n_mask);

	unsigned int fifo_rst;
	HDMI_PRINTF("%s\n", __func__);

	if (lane_mode == HDMI_4LANE)
		fifo_rst = fifo_mask;
	else
		fifo_rst = HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_p0_r_fifo_rst_n_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_p0_g_fifo_rst_n_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_p0_b_fifo_rst_n_mask;


        PWR_EMG("HDMI[p%d] HDMIRX_PHY_REGD21_reg (%08x=%08x, b4=%d, b3=%d , b2=%d, b1=%d))\n",
            nport,
            (HDMIRX_PHY_REGD21_reg),
            (hdmi_in(HDMIRX_PHY_REGD21_reg)),
            (hdmi_in(HDMIRX_PHY_REGD21_reg)>>4) & 1,
            (hdmi_in(HDMIRX_PHY_REGD21_reg)>>3) & 1,
            (hdmi_in(HDMIRX_PHY_REGD21_reg)>>2) & 1,
            (hdmi_in(HDMIRX_PHY_REGD21_reg)>>1) & 1);

	if (rst) {
		hdmi_mask(HDMIRX_PHY_REGD21_reg, ~(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_reg_p0_4b18b_mode_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_reg_p0_4b18b_data_order_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_reg_p0_hdmi21_mask),0x0);
		hdmi_mask(HDMIRX_PHY_REGD00_reg,~HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_reg_p0_data_order_mask,0x0);
              udelay(5);
		hdmi_mask(HDMIRX_PHY_REGD21_reg,~(fifo_mask),0x0);
		
	} else {
		if (frl_mode != MODE_TMDS) {
			//8bit 18bit mode  data order = [3:0]
		    	hdmi_mask(HDMIRX_PHY_REGD21_reg,
						~(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_reg_p0_4b18b_mode_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_reg_p0_4b18b_data_order_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_reg_p0_hdmi21_mask),
						HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_reg_p0_4b18b_data_order_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_reg_p0_hdmi21_mask);
		    	hdmi_mask(HDMIRX_PHY_REGD21_reg, ~(fifo_mask),fifo_rst);
                } else {
		    	hdmi_mask(HDMIRX_PHY_REGD21_reg, ~(fifo_mask),fifo_rst);
                     udelay(5);
                	//mode & data order don't care 
			hdmi_mask(HDMIRX_PHY_REGD21_reg,
						~(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_reg_p0_4b18b_mode_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_reg_p0_4b18b_data_order_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_reg_p0_hdmi21_mask),
						HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_reg_p0_4b18b_mode_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_reg_p0_4b18b_data_order_mask);
			//data order normal
			hdmi_mask(HDMIRX_PHY_REGD00_reg,~HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_reg_p0_data_order_mask,0x0);
			hdmi_mask(HDMIRX_PHY_REGD21_reg, ~(fifo_mask),0x0);
                }
	}
#endif
}




void lib_hdmi21_cdr_rst(unsigned char nport, unsigned char rst, unsigned char frl_mode,unsigned char lane_mode)
{

// rst = 1: reset
// rst = 0: normal working operation
#if 1
	if (rst) {
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_r_cdr_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_g_cdr_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_b_cdr_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_ck_cdr_rst_n_mask), 0);
		hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_r_dig_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_g_dig_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_b_dig_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_ck_dig_rst_n_mask), 0);

	} else {

		if (frl_mode == MODE_TMDS) {  //only 3 lane
			hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_r_cdr_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_g_cdr_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_b_cdr_rst_n_mask), (HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_r_cdr_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_g_cdr_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_b_cdr_rst_n_mask));
			hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_r_dig_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_g_dig_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_b_dig_rst_n_mask), (HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_r_dig_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_g_dig_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_b_dig_rst_n_mask));

		} else {
			//only enable dig rst
			hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_r_cdr_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_g_cdr_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_b_cdr_rst_n_mask), 0);
			hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_r_dig_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_g_dig_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_b_dig_rst_n_mask), (HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_r_dig_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_g_dig_rst_n_mask|HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_b_dig_rst_n_mask));
			if (lane_mode == HDMI_4LANE) {
			hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_ck_cdr_rst_n_mask), 0);
			hdmi_mask(HDMIRX_PHY_cdr_regd00, ~(HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_ck_dig_rst_n_mask), (HDMIRX_PHY_HD21_P0_CDR_REGD00_reg_p0_ck_dig_rst_n_mask));
			}
		}
	}
#endif	
}


//0xB80381E0
//B80381E0[31:0]: z0 value 4 port
//port0_z0=B80381E0[4:0] port0_done=B80381E0[5]
//port1_z0=B80381E0[10:6] port1_done=B80381E0[11]
//port2_z0=B80381E0[16:12] port2_done=B80381E0[17]
//port3_z0=B80381E0[22:18] port3_done=B80381E0[23]


void lib_hdmi21_z0_calibration(void)
{
#if 1
	 #define Z0_DIFF_ERR		0x0
        unsigned int z0_otp_value = rtd_inl(EFUSE_DATAO_120_reg);
	 
	 unsigned char z0_otp_done;
        unsigned char z0_0, z0_1,z0_2, z0_3;    //z0 for 4 port
	 unsigned char z0_low_bound_0,z0_low_bound_1,z0_low_bound_2,z0_low_bound_3;     //  85 -- 100 -- 115  +-15%
	 unsigned char z0_high_bound_0,z0_high_bound_1,z0_high_bound_2,z0_high_bound_3;    //high boundary z0 for all port
	 unsigned char ext_trace_z0_0,ext_trace_z0_1,ext_trace_z0_2,ext_trace_z0_3; //external z0 for 3 port
	 unsigned char soc_z0 = HD21_PHY_Z0;
	 unsigned char target_z0_0,target_z0_1,target_z0_2,target_z0_3;     // target_z0 = soc_z0 + pcb_trace_z0  (target z0 for all port)
	 unsigned long long val = 0;    
	unsigned char nport;

	 ext_trace_z0_0 = ext_trace_z0_1 =  ext_trace_z0_2 = ext_trace_z0_3 =(0x5 - Z0_DIFF_ERR);  // 1 step 1.5 ohm  5.4 ohm = 4 step
	 z0_otp_done = ((z0_otp_value & 0x00000020) >> 5) | ((z0_otp_value & 0x00000800) >> 10) | ((z0_otp_value & 0x00020000) >> 15) | ((z0_otp_value & 0x00800000) >> 20);
	 
	  if (pcb_mgr_get_enum_info_byname("HDMI_EXT_Z0_CFG", &val)==0) {
		ext_trace_z0_0 = (val & 0x000000FF)- Z0_DIFF_ERR;
		ext_trace_z0_1 = ((val & 0x0000FF00) >> 8) - Z0_DIFF_ERR;
		ext_trace_z0_2 = ((val & 0x00FF0000) >> 16) - Z0_DIFF_ERR;
		ext_trace_z0_3 = ((val & 0xFF000000) >> 24) - Z0_DIFF_ERR;
		
	  } else {
			HDMI_PRINTF("NOT GET  EXTERNAL PCB TRACE Z0\n");
	  }
	  
	HDMI_PRINTF("soc_z0=%x p0_ext_trace_z0=%x  p1_ext_trace_z0=%x\n",soc_z0, ext_trace_z0_0,ext_trace_z0_1);
	
	 target_z0_0 = soc_z0 + ext_trace_z0_0;
	 target_z0_1 = soc_z0 + ext_trace_z0_1;

	 target_z0_2 = soc_z0 + ext_trace_z0_2;
	 target_z0_3 = soc_z0 + ext_trace_z0_3;

	 
	 z0_high_bound_0 = target_z0_0 + 10;
	 z0_high_bound_1 = target_z0_1 + 10;

	 z0_high_bound_2 = target_z0_0 + 10;
	 z0_high_bound_3 = target_z0_1 + 10;
	 

	 if (target_z0_0 > 6)
	 	z0_low_bound_0 = target_z0_0 - 6;
	 else {
		z0_low_bound_0 = 0;
	 }

	  if (target_z0_1 > 6)
	 	z0_low_bound_1 = target_z0_1 - 6;
	 else {
		z0_low_bound_1 = 0;
	 }

	 if (target_z0_2 > 6)
	 	z0_low_bound_2 = target_z0_2 - 6;
	 else {
		z0_low_bound_0 = 0;
	 }

	  if (target_z0_3 > 6)
	 	z0_low_bound_3 = target_z0_3 - 6;
	 else {
		z0_low_bound_3 = 0;
	 }

	 
	HDMI_PRINTF("Z0 OTP done_bit=%x value=%x\n",z0_otp_done,z0_otp_value);

	
        if (z0_otp_done & 0x1) {
                z0_0 = z0_otp_value & 0x0000001F;
		  z0_0 = z0_0 + ext_trace_z0_0;
		  if ((z0_0 < z0_low_bound_0) || (z0_0 > z0_high_bound_0)) {
		        z0_0 = target_z0_0;
			 HDMI_PRINTF("port0 z0 otp over boundary\n");
		  }
        }
        else
                z0_0 = target_z0_0;

        if (z0_otp_done & 0x2) {
                z0_1 = (z0_otp_value & 0x000007C0) >> 6;
		  z0_1 = z0_1 + ext_trace_z0_1;
		  if ((z0_1 < z0_low_bound_1) || (z0_1 > z0_high_bound_1)) {
			 z0_1 = target_z0_1;
			 HDMI_PRINTF("port1 z0 otp over boundary\n");
		  }
        }
        else
                z0_1 = target_z0_1;


        if (z0_otp_done & 0x4) {
                z0_2 = (z0_otp_value & 0x0001F000) >> 12;
		  z0_2 = z0_2 + ext_trace_z0_2;
		  if ((z0_2 < z0_low_bound_2) || (z0_2 > z0_high_bound_2)) {
			  z0_2 = target_z0_2;
			   HDMI_PRINTF("port2 z0 otp over boundary\n");
		  }
        }
        else
                z0_2 = target_z0_2;

        if (z0_otp_done & 0x8) {
                z0_3 = (z0_otp_value & 0x007C0000) >> 18;
		  z0_3 = z0_3 + ext_trace_z0_3;
		 if ((z0_3 < z0_low_bound_3) || (z0_3 > z0_high_bound_3)) {
			z0_3 = target_z0_3;
			 HDMI_PRINTF("port3 z0 otp over boundary\n");
		 }
        }
        else
                z0_3 = target_z0_3;
		
	HDMI_PRINTF("z0_0=%x z0_1=%xz0_2=%x z0_3=%x\n",z0_0,z0_1,z0_2,z0_3);

	
	for (nport=0; nport<HDMI_PORT_TOTAL_NUM; nport++)
	{

		switch(nport)
		{
			case HDMI_PORT0:
			hdmi_mask(HDMIRX_PHY_z0k_reg, ~(HDMIRX_PHY_Z0K_reg_z0_adjr_0_mask),
			        (HDMIRX_PHY_Z0K_reg_z0_adjr_0(z0_0)));				
				break;	
			case HDMI_PORT1: 
			hdmi_mask(HDMIRX_PHY_z0k_reg, ~(HDMIRX_PHY_Z0K_reg_z0_adjr_0_mask),
			        (HDMIRX_PHY_Z0K_reg_z0_adjr_0(z0_1)));				
				break;	
			case HDMI_PORT2:
			hdmi_mask(HDMIRX_PHY_z0k_reg, ~(HDMIRX_PHY_Z0K_reg_z0_adjr_0_mask),
			        (HDMIRX_PHY_Z0K_reg_z0_adjr_0(z0_2)));				
				break;	
			case HDMI_PORT3:
			hdmi_mask(HDMIRX_PHY_z0k_reg, ~(HDMIRX_PHY_Z0K_reg_z0_adjr_0_mask),
			        (HDMIRX_PHY_Z0K_reg_z0_adjr_0(z0_3)));				
				break;	
			default: 
				break;
		}

	}

	
#endif
}

//[pad no. port2], B lane ldo sel: [2:0], G lane ldo sel: [5:3], R lane ldo sel: [8:6], CK lane ldo sel: [11:9]
//[pad no. port3], B lane ldo sel: [14:12], G lane ldo sel: [17:15], R lane ldo sel: [20:18], CK lane ldo sel: [23:21]
//[done bit]: [24]
void lib_hdmi21_ldo_calibration(void)
{
#if 0
	unsigned int ldo_otp_value = rtd_inl(EFUSE_DATAO_123_reg);
	unsigned char ldo_otp_done = (ldo_otp_value & 0x01000000) >> 24;
	unsigned char ldo_default = HD21_PHY_LDO;	//if no need trimming, use ldo_default

	HDMI_EMG("[%s][%d][ldo_otp_value = %d, ldo_otp_done = %d]\n", __FUNCTION__, __LINE__, ldo_otp_value, ldo_otp_done);

	if (ldo_otp_done) 
	{
		hdmi_mask(HDMIRX_2P1_PHY_P0_ACDR_B1_reg, ~(acdr_b_6_LDO2V), HD21_RX_PHY_P0_VSEL_LDO(ldo_otp_value & 0x00000007));
		hdmi_mask(HDMIRX_2P1_PHY_P0_ACDR_G1_reg, ~(acdr_g_6_LDO2V), HD21_RX_PHY_P0_VSEL_LDO((ldo_otp_value & 0x00000038) >> 3));
		hdmi_mask(HDMIRX_2P1_PHY_P0_ACDR_R1_reg, ~(acdr_r_6_LDO2V), HD21_RX_PHY_P0_VSEL_LDO((ldo_otp_value & 0x000001C0) >> 6));
		hdmi_mask(HDMIRX_2P1_PHY_P0_ACDR_CK1_reg, ~(acdr_ck_6_LDO2V), HD21_RX_PHY_P0_VSEL_LDO((ldo_otp_value & 0x00000E00) >> 9));
		hdmi_mask(HDMIRX_2P1_PHY_P1_ACDR_B1_reg, ~(acdr_b_6_LDO2V), HD21_RX_PHY_P0_VSEL_LDO((ldo_otp_value & 0x00007000) >> 12));
		hdmi_mask(HDMIRX_2P1_PHY_P1_ACDR_G1_reg, ~(acdr_g_6_LDO2V), HD21_RX_PHY_P0_VSEL_LDO((ldo_otp_value & 0x00038000) >> 15));
		hdmi_mask(HDMIRX_2P1_PHY_P1_ACDR_R1_reg, ~(acdr_r_6_LDO2V), HD21_RX_PHY_P0_VSEL_LDO((ldo_otp_value & 0x001C0000) >> 18));
		hdmi_mask(HDMIRX_2P1_PHY_P1_ACDR_CK1_reg, ~(acdr_ck_6_LDO2V), HD21_RX_PHY_P0_VSEL_LDO((ldo_otp_value & 0x00E00000) >> 21));
		
		HDMI_EMG("[%s][%d][ldo_otp_value = %x]\n", __FUNCTION__, __LINE__, ldo_otp_value);
	}
	else
	{
		hdmi_mask(HDMIRX_2P1_PHY_P0_ACDR_B1_reg, ~(acdr_b_6_LDO2V), HD21_RX_PHY_P0_VSEL_LDO(ldo_default));
		hdmi_mask(HDMIRX_2P1_PHY_P0_ACDR_G1_reg, ~(acdr_g_6_LDO2V), HD21_RX_PHY_P0_VSEL_LDO(ldo_default));
		hdmi_mask(HDMIRX_2P1_PHY_P0_ACDR_R1_reg, ~(acdr_r_6_LDO2V), HD21_RX_PHY_P0_VSEL_LDO(ldo_default));
		hdmi_mask(HDMIRX_2P1_PHY_P0_ACDR_CK1_reg, ~(acdr_ck_6_LDO2V), HD21_RX_PHY_P0_VSEL_LDO(ldo_default));
		hdmi_mask(HDMIRX_2P1_PHY_P1_ACDR_B1_reg, ~(acdr_b_6_LDO2V), HD21_RX_PHY_P0_VSEL_LDO(ldo_default));
		hdmi_mask(HDMIRX_2P1_PHY_P1_ACDR_G1_reg, ~(acdr_g_6_LDO2V), HD21_RX_PHY_P0_VSEL_LDO(ldo_default));
		hdmi_mask(HDMIRX_2P1_PHY_P1_ACDR_R1_reg, ~(acdr_r_6_LDO2V), HD21_RX_PHY_P0_VSEL_LDO(ldo_default));
		hdmi_mask(HDMIRX_2P1_PHY_P1_ACDR_CK1_reg, ~(acdr_ck_6_LDO2V), HD21_RX_PHY_P0_VSEL_LDO(ldo_default));

		HDMI_EMG("[%s][%d][ldo_value = %x, b800daa8 = %x]\n", __FUNCTION__, __LINE__, ldo_default, rtd_inl(HDMIRX_2P1_PHY_P0_ACDR_R1_reg));
	}
#endif
}





void lib_fast_swtch_mode(unsigned char nport,unsigned char mode)
{
	if (mode == PS_NORMAL_MODE) {
			//DELAY_CEL LDO_VSEL  KVCO
		hdmi_mask(HDMIRX_PHY_top_in_0_reg, ~(TOP_IN_2_P0_FS_SIGNAL_DLY_TIME|TOP_IN_2_P0_FS_SIGNAL_DLY|TOP_IN_2_P0_FS_SIGNAL_SEL),TOP_IN_2_P0_FS_SIGNAL_DLY_TIME_100);
		hdmi_mask(HDMIRX_PHY_b0_reg,~(acdr_b_7_FS_SIGNAL_DLY_EN|acdr_b_6_FS_EN), 0x0);
		hdmi_mask(HDMIRX_PHY_g0_reg,~(acdr_g_7_FS_SIGNAL_DLY_EN|acdr_g_6_FS_EN), 0x0);
		hdmi_mask(HDMIRX_PHY_r0_reg,~(acdr_r_7_FS_SIGNAL_DLY_EN|acdr_r_6_FS_EN), 0x0);
		hdmi_mask(HDMIRX_PHY_ck0_reg,~(acdr_ck_7_FS_SIGNAL_DLY_EN|acdr_ck_6_FS_EN), 0x0);		
	} else {


	}

}


