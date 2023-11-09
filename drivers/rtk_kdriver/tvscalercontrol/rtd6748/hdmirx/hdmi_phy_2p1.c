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

/**********************************************************************************************
*
*	Marco or Definitions
*
**********************************************************************************************/
#define HDMI_FRL_TAP0234_ADP_EN		1
#define HD21_PHY_Z0        			0x11

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
unsigned char Vth_value=3;// Vth = 3 for long Cable

#ifndef MAX
    #define MAX(a,b)                (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
    #define MIN(a,b)                (((a) < (b)) ? (a) : (b))
#endif


/**********************************************************************************************
*
*	Const Declarations
*
**********************************************************************************************/


#ifdef HD20_FLD_MODE


#else
HDMI21_PHY_TMDS_PARAM_T hdmi21_phy_tmds_param[] =
{                        
       //                                                                               cco setting                                          charge pump                             LPF                                    CDR
       //b_up  b_down     M/N/preM/postM      mansel/ck_src   kvco/encap/bandsel/cellsel/cap_en/           halfrate/cp_op/cp_cap/icp    VC/RS/CS/CP               pi_div/demux/rate_sel/pixratesel/pi_isel/pi_cur_adj/pi_csel/kp/ki/full&half rate      
       //                                                                           res_en/fine_I/coarse_I/DM_en/DM_I
	{5860,2844,       0x10,0x0,0x0,0x0,        0x1,0x0,           0x0,0x1,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x6,    0x1,0x1,0x1,0xf,                 0x3,0x2,0x3,0x3,         0x0,0x1,0x0,0x2,0x2,0x0,0x0,(0x3<<2),0x1,0x1, "T1"  },                                                                                                                                                                                                                                                                              
	{2844,1422,       0x10,0x1,0x0,0x0,        0x1,0x0,           0x0,0x1,0x0,0x0,0x1,0x1,0x0,0x0,0x0,0x6,    0x1,0x1,0x1,0xf,                0x3,0x2,0x3,0x3,          0x1,0x2,0x1,0x3,0x2,0x0,0x0,(0x8<<2),0x1,0x1, "T2"   },                                                                                                                                                                                                                                                                                                                              
	{1422,1185,       0x10,0x1,0x0,0x0,        0x1,0x0,           0x0,0x1,0x0,0x0,0x1,0x1,0x7,0x7,0x0,0x6,    0x1,0x1,0x1,0xf,                0x3,0x2,0x3,0x3,          0x1,0x2,0x1,0x3,0x2,0x0,0x0,(0x8<<2),0x0,0x1, "T3"   },                                                                                                                                                                                                                                                                                                                              
	{1185,711,         0x10,0x0,0x0,0x0,        0x1,0x0,           0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6,    0x0,0x1,0x1,0x4,                0x3,0x1,0x3,0x3,          0x2,0x3,0x2,0x3,0x2,0x0,0x0,(0x8<<2),0x0,0x1, "T4"   },                                                                                                                                                                                                                                                                                                                  
	{711,593,         0x10,0x0,0x0,0x0,        0x1,0x0,           0x0,0x1,0x0,0x0,0x0,0x0,0x7,0x7,0x0,0x6,    0x0,0x1,0x1,0x8,                0x3,0x1,0x3,0x3,          0x2,0x3,0x2,0x3,0x2,0x0,0x0,(0x8<<2),0x0,0x1, "T5"   },                                                                                                                                                                                                                                                                                                                  
	{593, 356,         0x24,0x0,0x0,0x0,        0x1,0x0,           0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6,    0x0,0x1,0x1,0xa,                 0x3,0x1,0x3,0x3,          0x3,0x3,0x2,0x3,0x2,0x0,0x0,(0x8<<2),0x0,0x1, "T6"   },                                                                                                                                                                                                                                                                                                                                
	{356, 120,          0x24,0x0,0x0,0x1,        0x1,0x0,           0x0,0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x6,    0x0,0x1,0x1,0xe,                0x3,0x1,0x3,0x3,          0x4,0x3,0x2,0x3,0x2,0x0,0x0,(0x8<<2),0x0,0x1, "T7"  },                             																																													        
};
#endif

//for hdmi2.0 phy parameter according Fvco
HDMI21_PHY_TMDS_PARAM2_T hdmi21_phy_tmds_param2[] =
{
	//
	{6000,5500,  0x1,0x1,0x3,0x2,0x0,0x0,0xB,0xB},
	{5500,5000,  0x1,0x1,0x3,0x2,0x0,0x0,0x8,0x8},
	{5000,4500,  0x1,0x1,0x2,0x2,0x0,0x1,0x8,0x8},
	{4500,4000,  0x1,0x1,0x2,0x2,0x0,0x1,0x4,0x4},
	{4000,3500,  0x1,0x1,0x1,0x2,0x0,0x2,0x7,0x7},
	{3500,3000,  0x1,0x1,0x1,0x2,0x0,0x2,0x5,0x5},
	{3000,2500,  0x1,0x1,0x0,0x4,0x0,0x3,0x8,0x8},
	{2500,2000,  0x0,0x1,0x0,0x4,0x0,0x3,0x8,0x8},
};

#if FRL_DCDR_3G_6G_MODE

HDMI21_PHY_FRL_PARAM_T hdmi21_phy_frl_param[] =
{                        
       //                                                                                    cco setting                                                                                                                                 charge pump                              LPF                                                                CDR
       //b_up  b_down     M/N/preM/postM        mansel/ck_src   kvco/encap/bandsel/cellsel/cap_en(x3)/res_en(x3)/fine_I(x3)/coarse_I(x3)/DM_en(x3)/DM_I(x3)  halfrate/cp_op/cp_cap/icp(x2)/cpt(x2)     VC/RS(x3)/CS(x3)/CP(x3)                           pi_div/demux/rate_sel/pixratesel/pi_isel/pi_csel/kp/ki (useless for acdr) /full&half rate    
	{12000,12000,       0x6b,0x0,0x0,0x0,        0x0,0x1,           0x1,0x0,0x3,0x1,0x1,0x1,0x1,0x0,0x1,0x0,0x4,0x4,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x6,0x6,0x6,       0x1,0x1,0x1,0x8,0x6,0x2,0x4,                0x4,0x0,0x5,0x0,0x3,0x3,0x3,0x2,0x0,0x2,\
	                                                                                                                                                                                                                                                                                                                                 				      0x0,0x1,0x0,0x1,0x0,0x0,0x0,0x0,0x0, "12G FRL RATE"  },
	{10000,10000,       0xb5,0x0,0x1,0x0,        0x0,0x1,           0x0,0x0,0x3,0x1,0x1,0x1,0x1,0x0,0x1,0x0,0x4,0x4,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x6,0x6,0x6,        0x1,0x1,0x1,0x8,0x6,0x2,0x4,                0x4,0x0,0x5,0x0,0x3,0x3,0x3,0x2,0x0,0x2,\
	                                                                                                                                                                                                                                                                                                                                 				       0x0,0x1,0x0,0x1,0x0,0x0,0x0,0x0,0x0, "10G  FRL RATE"   },
	{8000,8000,          0x46,0x0,0x0,0x0,        0x0,0x1,           0x1,0x0,0x1,0x0,0x1,0x1,0x1,0x0,0x1,0x0,0x4,0x4,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x6,0x6,0x6,         0x1,0x1,0x1,0x8,0x6,0x2,0x4,                0x4,0x0,0x5,0x0,0x3,0x3,0x3,0x2,0x0,0x2,\
	                                                                                                                                                                                                                                                                                                                                 				              0x0,0x1,0x0,0x1,0x0,0x0,0x0,0x0,0x0, "8G  FRL RATE"   },
};

#else
HDMI21_PHY_FRL_PARAM_T hdmi21_phy_frl_param[] =
{                        
       //                                                                                    cco setting                                                                                                                                 charge pump                              LPF                                                                CDR
       //b_up  b_down     M/N/preM/postM        mansel/ck_src   kvco/encap/bandsel/cellsel/cap_en(x3)/res_en(x3)/fine_I(x3)/coarse_I(x3)/DM_en(x3)/DM_I(x3)  halfrate/cp_op/cp_cap/icp(x2)/cpt(x2)     VC/RS(x3)/CS(x3)/CP(x3)                           pi_div/demux/rate_sel/pixratesel/pi_isel/pi_csel/kp/ki (useless for acdr) /full&half rate    
	{12000,12000,       0x6b,0x0,0x0,0x0,        0x0,0x1,           0x1,0x0,0x3,0x1,0x1,0x1,0x1,0x0,0x1,0x0,0x4,0x4,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x6,0x6,0x6,       0x1,0x1,0x1,0x8,0x2,0x2,0x4,                0x4,0x0,0x5,0x0,0x3,0x3,0x3,0x0,0x0,0x2,\
	                                                                                                                                                                                                                                                                                                                                 				      0x0,0x1,0x0,0x1,0x0,0x0,0x0,0x0,0x0, "12G FRL RATE"  },
	{10000,10000,       0xb5,0x0,0x1,0x0,        0x0,0x1,           0x0,0x0,0x3,0x1,0x1,0x1,0x1,0x0,0x1,0x0,0x4,0x4,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x6,0x6,0x6,        0x1,0x1,0x1,0x8,0x6,0x2,0x4,                0x4,0x0,0x5,0x0,0x3,0x3,0x3,0x2,0x0,0x2,\
	                                                                                                                                                                                                                                                                                                                                 				       0x0,0x1,0x0,0x1,0x0,0x0,0x0,0x0,0x0, "10G  FRL RATE"   },
	{8000,8000,          0x46,0x0,0x0,0x0,        0x0,0x1,           0x1,0x1,0x1,0x0,0x1,0x1,0x1,0x0,0x1,0x0,0x4,0x4,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x6,0x6,0x6,         0x1,0x1,0x1,0x8,0x6,0x2,0x4,                0x4,0x0,0x5,0x0,0x3,0x3,0x3,0x2,0x0,0x2,\
	                                                                                                                                                                                                                                                                                                                                 				              0x0,0x1,0x0,0x1,0x0,0x0,0x0,0x0,0x0, "8G  FRL RATE"   },
	{6000,6000,          0x6b,0x0,0x0,0x0,        0x0,0x1,           0x1,0x0,0x3,0x1,0x1,0x1,0x1,0x0,0x1,0x0,0x4,0x4,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x6,0x6,0x6,         0x0,0x1,0x1,0x8,0x6,0x2,0x4,                0x4,0x0,0x5,0x0,0x3,0x3,0x3,0x2,0x0,0x2,\
	                                                                                                                                                                                                                                                                                                                                 					0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1, "6G  FRL RATE"   },  
	{3000,3000,          0x6b,0x0,0x1,0x0,        0x0,0x1,           0x1,0x1,0x0,0x0,0x1,0x1,0x1,0x0,0x1,0x0,0x4,0x4,0x4,0x4,0x4,0x4,0x0,0x0,0x0,0x6,0x6,0x6,         0x0,0x1,0x1,0x8,0x6,0x2,0x4,                0x4,0x0,0x5,0x0,0x3,0x3,0x3,0x2,0x0,0x2,\
	                             																																													       0x1,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x1, "3G  FRL RATE"  },

};

#endif


HDMI21_EQ_PARAM_T hdmi21_eq_param[] =
{
	{5860,2844,    0x5,0x2,0x2,0x1,0x5,0xb,0xb,0x1,0x3,0x5,0x0,0x6,0x1,0x1,0x0},
	{2844,1422,    0x3,0x0,0x0,0x0,0x3,0x3,0x3,0x0,0x2,0x4,0x0,0x4,0x0,0x0,0x0},
	{1422,120,      0x1,0x0,0x0,0x0,0x1,0x1,0x1,0x0,0x2,0x0,0x0,0x4,0x0,0x0,0x0}
};



HDMI21_EQ_PARAM_T hdmi21_eq_frl_param[] =
{
	//12G modify for CTS--EQ BW larger (LEQ_CURRENT_ADJ = 0x2 -> 0x1, LEQ_ISEL = 0x6 -> 0x7, RLSEL_LEQ = 0x7 -> 0xf) & RSSEL_LEQ2 = 0xc -> 0x8
	{12000,12000,    0xf,0x1,0x1,0x1,0x8,0xd,0xd,0x1,0x1,0x7,0x4,0x6,0x1,0x1,0x0},
	{10000,10000,    0x7,0x6,0x6,0x1,0x8,0xd,0xd,0x1,0x2,0x6,0x3,0x6,0x1,0x1,0x0},
	{8000,8000,       0x7,0x6,0x6,0x1,0x8,0xd,0xd,0x1,0x2,0x6,0x3,0x6,0x1,0x1,0x0},
	{6000,6000,       0x5,0x2,0x2,0x1,0x5,0xb,0xb,0x1,0x3,0x5,0x0,0x6,0x1,0x1,0x0},
	{3000,3000,	    0x3,0x0,0x0,0x0,0x3,0x3,0x3,0x0,0x2,0x4,0x0,0x4,0x0,0x0,0x0}
};	

HDMI21_EQ_PARAM_T hdmi21_eq_frl_paramNew[] =
{
	//12G modify for CTS--EQ BW larger (LEQ_CURRENT_ADJ = 0x2 -> 0x1, LEQ_ISEL = 0x6 -> 0x7, RLSEL_LEQ = 0x7 -> 0xf) & RSSEL_LEQ2 = 0xc -> 0x8
#if 0 //ori
	{12000,12000,    0xf,0x1,0x1,0x1,0x8,0xd,0xd,0x1,0x1,0x7,0x4,0x6,0x1,0x1,0x0},
#else
//RDC  for LG 7M Cable
	{12000,12000,    0xf,0x5,0x5,0x1,0x8,0xd,0xd,0x1,0x1,0x7,0x5,0x6,0x1,0x1,0x0},
#endif
	{10000,10000,    0x7,0x6,0x6,0x1,0x8,0xd,0xd,0x1,0x2,0x6,0x3,0x6,0x1,0x1,0x0},
	{8000,8000,       0x7,0x6,0x6,0x1,0x8,0xd,0xd,0x1,0x2,0x6,0x3,0x6,0x1,0x1,0x0},
	{6000,6000,       0x5,0x2,0x2,0x1,0x5,0xb,0xb,0x1,0x3,0x5,0x0,0x6,0x1,0x1,0x0},
	{3000,3000,	    0x3,0x0,0x0,0x0,0x3,0x3,0x3,0x0,0x2,0x4,0x0,0x4,0x0,0x0,0x0}
};	

#if defined(HD21_CLK_OVER_300M)


#ifdef HD20_FLD_MODE


#else
HDMI21_PHY_TMDS_PARAM_T hdmi21_phy_over3G_tmds_param[] =
{
       //                                                                               cco setting                                          charge pump                             LPF                                                         Others
       //b_up  b_down     M/N/preM/postM      mansel/ck_src   kvco/encap/bandsel/cellsel/cap_en/           halfrate/cp_op/cp_cap/icp    VC/RS/CS/CP             pi_div/demux/rate_sel/pixratesel/pi_isel/pi_cur_adj/pi_csel/kp/ki/full&half rate      
       //                                                                           res_en/fine_I/coarse_I/DM_en/DM_I
	{5860,4740,       0x6,0x1,0x0,0x0,        0x1,0x0,           0x1,0x1,0x1,0x0,0x0,0x0,0x5,0x5,0x0,0x6,    0x0,0x1,0x1,0x4,                 0x3,0x0,0x3,0x3,         0x0,0x1,0x0,0x2,0x2,0x1,0x2,(0x3<<2),0x1,0x1, "500M~600M half rate"  }, 
	{4740,3792,       0x6,0x1,0x0,0x0,        0x1,0x0,           0x1,0x1,0x1,0x0,0x0,0x0,0x5,0x5,0x0,0x6,    0x0,0x1,0x1,0x4,                 0x3,0x0,0x3,0x3,         0x0,0x1,0x0,0x2,0x2,0x1,0x2,(0x3<<2),0x1,0x1, "400M~500M half rate"  }, 
	{3792,2844,       0x6,0x1,0x0,0x0,        0x1,0x0,           0x1,0x1,0x1,0x0,0x0,0x0,0x5,0x5,0x0,0x6,    0x0,0x1,0x1,0x4,                 0x3,0x0,0x3,0x3,         0x0,0x1,0x0,0x2,0x2,0x1,0x2,(0x3<<2),0x1,0x1, "300M~400M half rate"  }, 
};
#endif

#endif




HDMI21_FLD_PARAM_T hdmi21_fld_param[] = 
{
	{12000,12000,    0x4,0x4,30,0x356,0x32c,0x356,0x32c},
	{10000,10000,    0x4,0x4,30,0x2cc,0x2a2,0x2cc,0x2a2},
	{8000,8000,        0x4,0x4,30,0x243,0x215,0x241,0x217},
	{6000,6000,       0x4,0x4,30,0x356,0x32c,0x356,0x32c},
	{3000,3000,	    0x4,0x4,30,0x1BB,0x187,0x1B6,0x18c}
};


const unsigned int reg_ck_cmu[HD21_PORT_TOTAL_NUM] = {
	HDMIRX_2P1_PHY_P0_CK_CMU_reg,
	HDMIRX_2P1_PHY_P1_CK_CMU_reg
};

const unsigned int reg_acdr_rgbc0[HD21_PORT_TOTAL_NUM][4] = {
	{HDMIRX_2P1_PHY_P0_ACDR_R0_reg,HDMIRX_2P1_PHY_P0_ACDR_G0_reg,HDMIRX_2P1_PHY_P0_ACDR_B0_reg,HDMIRX_2P1_PHY_P0_ACDR_CK0_reg},
	{HDMIRX_2P1_PHY_P1_ACDR_R0_reg,HDMIRX_2P1_PHY_P1_ACDR_G0_reg,HDMIRX_2P1_PHY_P1_ACDR_B0_reg,HDMIRX_2P1_PHY_P1_ACDR_CK0_reg}};

const unsigned int reg_acdr_rgbc1[HD21_PORT_TOTAL_NUM][4] = {
	{HDMIRX_2P1_PHY_P0_ACDR_R1_reg,HDMIRX_2P1_PHY_P0_ACDR_G1_reg,HDMIRX_2P1_PHY_P0_ACDR_B1_reg,HDMIRX_2P1_PHY_P0_ACDR_CK1_reg},
	{HDMIRX_2P1_PHY_P1_ACDR_R1_reg,HDMIRX_2P1_PHY_P1_ACDR_G1_reg,HDMIRX_2P1_PHY_P1_ACDR_B1_reg,HDMIRX_2P1_PHY_P1_ACDR_CK1_reg}};

const unsigned int reg_rgbc0[HD21_PORT_TOTAL_NUM][4] ={
	{HDMIRX_2P1_PHY_P0_R0_reg, HDMIRX_2P1_PHY_P0_G0_reg, HDMIRX_2P1_PHY_P0_B0_reg,HDMIRX_2P1_PHY_P0_CK0_reg},
	{HDMIRX_2P1_PHY_P1_R0_reg, HDMIRX_2P1_PHY_P1_G0_reg, HDMIRX_2P1_PHY_P1_B0_reg,HDMIRX_2P1_PHY_P1_CK0_reg}};

const unsigned int reg_rgbc1[HD21_PORT_TOTAL_NUM][4] ={
	{HDMIRX_2P1_PHY_P0_R1_reg, HDMIRX_2P1_PHY_P0_G1_reg, HDMIRX_2P1_PHY_P0_B1_reg,HDMIRX_2P1_PHY_P0_CK1_reg},
	{HDMIRX_2P1_PHY_P1_R1_reg, HDMIRX_2P1_PHY_P1_G1_reg, HDMIRX_2P1_PHY_P1_B1_reg,HDMIRX_2P1_PHY_P1_CK1_reg}};


const unsigned int reg_rgbc2[HD21_PORT_TOTAL_NUM][4] ={
	{HDMIRX_2P1_PHY_P0_R2_reg, HDMIRX_2P1_PHY_P0_G2_reg, HDMIRX_2P1_PHY_P0_B2_reg,HDMIRX_2P1_PHY_P0_CK2_reg},
	{HDMIRX_2P1_PHY_P1_R2_reg, HDMIRX_2P1_PHY_P1_G2_reg, HDMIRX_2P1_PHY_P1_B2_reg,HDMIRX_2P1_PHY_P1_CK2_reg}};



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

//bias power

void lib_hdmi21_bias_init(unsigned char nport);

//dfe
void newbase_hdmi21_dump_dfe_para(unsigned char nport, unsigned char lane_mode);
void lib_hdmi21_dump_dfe_tap0_le(unsigned char nport,unsigned char lane_mode);
unsigned char newbase_hdmi21_tmds_dfe_record(unsigned char nport, unsigned int clk);
unsigned char newbase_hdmi21_tmds_dfe_close(unsigned char nport, unsigned int clk, unsigned char lane_mode, unsigned char phy_state);
unsigned char newbase_hdmi21_frl_dfe_close(unsigned char nport, unsigned int clk, unsigned char lane_mode,unsigned char phy_state);
void newbase_hdmi21_tmds_dfe_hi_speed(unsigned char nport, unsigned int b_clk, unsigned char lane_mode);
void newbase_hdmi21_tmds_dfe_6g_long_cable_patch(unsigned char nport);
//extern unsigned char newbase_hdmi_dfe_hi_speed_close(unsigned char nport);
void newbase_hdmi21_tmds_dfe_mi_speed(unsigned char nport, unsigned int b_clk, unsigned char lane_mode);
void newbase_hdmi21_tmds_dfe_mid_adapthve(unsigned char nport, unsigned int b_clk);

//Z0
void lib_hdmi21_z0_calibration(void);
void lib_hdmi21_z300_sel(unsigned char port, unsigned char mode);  // 0: active mode 1:vcm mode

//LDO
void lib_hdmi21_ldo_calibration(void);

//eq setting
#ifdef CONFIG_POWER_SAVING_MODE
void lib_hdmi21_ac_couple_power_en(unsigned char nport,unsigned char en);
void lib_hdmi21_eq_pi_power_en(unsigned char nport,unsigned char en, unsigned char frl_mode,unsigned char lane_mode);
#endif
void lib_hdmi21_eq_bias_band_setting(unsigned char nport, unsigned int b,unsigned char frl_mode,unsigned char lane_mode);

		
//dfe setting
void lib_hdmi21_dfe_init(unsigned char nport);
void lib_hdmi21_dfe_power(unsigned char nport, unsigned char en, unsigned char lane_mode);
void lib_hdmi21_set_dfe(unsigned char nport, unsigned int b, unsigned char dfe_mode, unsigned char lane_mode);
void lib_hdmi21_set_dfe_mid_band(unsigned char nport, unsigned int b_clk, int dfe_mode, unsigned char lane_mode);
void lib_hdmi21_dfe_manual_set(unsigned char nport, unsigned char lane_mode);


void lib_hdmi21_dfe_config_setting(unsigned char nport,unsigned char lane_mode);



//PLL/DEMUX/CDR
void lib_hdmi21_cco_band_config(unsigned char nport, unsigned int bclk, HDMI21_PHY_TMDS_PARAM_T *phy_param);
void lib_hdmi21_cmu_pll_en(unsigned char nport, unsigned char enable,unsigned char frl_mode);
void lib_hdmi21_demux_ck_vcopll_rst(unsigned char nport, unsigned char rst, unsigned char lane_mode);
void lib_hdmi21_cdr_rst(unsigned char nport, unsigned char rst,unsigned char frl_mode,unsigned char lane_mode);
void lib_hdmi21_8b18b_fifo_rst(unsigned char nport,unsigned char rst,unsigned char frl_mode,unsigned char lane_mode);

//Watch Dog
void lib_hdmi21_wdog(unsigned char nport,unsigned char frl_mode,unsigned char lane_mode);
void lib_hdmi21_cmu_rst(unsigned char port, unsigned char rst);

//PHY TABLE
void lib_hdmi21_set_phy_table(unsigned char nport, HDMI21_PHY_TMDS_PARAM_T *phy_param);
void lib_hdmi21_set_frl_phy_table(unsigned char nport, HDMI21_PHY_FRL_PARAM_T *phy_param);
void lib_hdmi21_set_fld_table(unsigned char nport,HDMI21_FLD_PARAM_T * fld_param);
unsigned char lib_hdmi21_get_phy_table(unsigned char nport,unsigned int b,HDMI21_PHY_TMDS_PARAM_T **p_phy_table, HDMI21_PHY_FRL_PARAM_T **p_frl_phy_table,unsigned char frl_mode);


//foreground K offset
void lib_hdmi21_fg_koffset_manual_adjust(unsigned char nport, unsigned int r, unsigned int g, unsigned int b, unsigned int ck);
void lib_hdmi21_fg_koffset_manual(unsigned char nport);
void lib_hdmi21_fg_koffset_auto(unsigned char nport, unsigned char lane_mode);
void lib_hdmi21_fg_koffset_proc(unsigned char nport, unsigned char lane_mode);
void lib_hdmi21_fg_koffset_disable(unsigned char nport);
void lib_hdmi21_single_end(unsigned char nport, unsigned char channel, unsigned char p_off, unsigned char n_off);


//acdr
void lib_hdmi21_fld_ck_en(unsigned char nport, unsigned char en,unsigned char frl_mode,unsigned char lane_mode);
void lib_hdmi21_fld_reset(unsigned char nport, unsigned char rst, unsigned char frl_mode,unsigned char lane_mode);
void lib_hdmi21_acdr_power_en(unsigned char nport , unsigned char en, unsigned char frl_mode,unsigned char lane_mode);
void lib_hdmi21_acdr_finetune_en(unsigned char nport , unsigned char en, unsigned char lane_mode);
void lib_hdmi21_bbpd_reset(unsigned char nport, unsigned char en, unsigned char lane_mode);
void lib_hdmi21_cdr_mode_config(unsigned char nport,unsigned char mode);
void lib_hdmi21_cdr_init(unsigned char nport, unsigned frl_mode,unsigned char lane_mode);
int lib_hdmi21_is_acdr_state(unsigned char nport, unsigned char lane);
int lib_hdmi21_acdr_calib_status(unsigned char nport, unsigned char lane, unsigned char value);
void lib_hdmi21_dfe_param_init(unsigned char nport, unsigned int b, unsigned char dfe_mode, unsigned char lane_mode);

int lib_hdmi21_acdr_is_unlock(unsigned char nport,unsigned char lane_mode);
int lib_hdmi21_coarse_calib_is_ok(unsigned char nport,unsigned char lane_mode);

void lib_fast_swtch_mode(unsigned char nport,unsigned char mode);
//high measure clk detect
int lib_clk_lane_transition_detect(unsigned char nport);


int newbase_hdmi21_rxphy_get_frl_info(unsigned char port, unsigned char frl_mode);
void newbase_hdmi21_rxphy_set_frl_info(unsigned char port,unsigned char frl_mode);


/**********************************************************************************************
*
*	Function Body
*
**********************************************************************************************/


unsigned char newbase_rxphy_frl_measure(unsigned char port)
{
	if (port >= HD20_PORT)
		return 0;

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
		phy_st[port].clk = FRL_3G;
		break;
	case 2:
		phy_st[port].lane_num = HDMI_3LANE;
		phy_st[port].frl_mode = MODE_FRL_6G_3_LANE;
		phy_st[port].clk = FRL_6G;
		break;
	case 3:   // FRL:6G/4Ln
		phy_st[port].lane_num = HDMI_4LANE;
		phy_st[port].frl_mode = MODE_FRL_6G_4_LANE;
		phy_st[port].clk = FRL_6G;
		break;
	case 4:   // FRL:8G/4Ln
		phy_st[port].lane_num = HDMI_4LANE;
		phy_st[port].frl_mode = MODE_FRL_8G_4_LANE;
		phy_st[port].clk = FRL_8G;
		break;
	case 5:   // FRL:10G/4Ln
		phy_st[port].lane_num = HDMI_4LANE;
		phy_st[port].frl_mode = MODE_FRL_10G_4_LANE;
		phy_st[port].clk = FRL_10G;
		break;
	case 6:   // FRL:12G/4Ln
		phy_st[port].lane_num = HDMI_4LANE;
		phy_st[port].frl_mode = MODE_FRL_12G_4_LANE;
		phy_st[port].clk = FRL_12G;
		break;
	default:   // FRL:6G/4Ln
		phy_st[port].lane_num = HDMI_4LANE;
		phy_st[port].frl_mode = MODE_FRL_6G_4_LANE;
		phy_st[port].clk = FRL_6G;
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


#if 0
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
	if (phy_st[port].clk_debounce_count >= CLK_DEBOUNCE_SHORT_CNT_THD)
	{
	       HDMI_HDCP_E hdcp_auth_mode = newbase_hdcp_get_auth_mode(port);
		if(NO_HDCP == hdcp_auth_mode || HDCP_OFF == hdcp_auth_mode)
		{
			if (phy_st[port].clk_debounce_count >=  newbase_hdmi_get_pcbinfo_entry(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_PHY_STABLE_CNT_THD))
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
#endif

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
#if !HDMI_FRL_TAP0234_ADP_EN
										phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];
#else 

#endif
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
#if !HDMI_FRL_TAP0234_ADP_EN										
										phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];
#else 

#endif										
										HDMI_EMG(" check le hit boundary, lane = %d, le = %d\n", lane, phy_st[nport].dfe_t[lane].le);			
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
									}
									else    //Change Tap2 value only when LE not in boundary condition
									{
#if !HDMI_FRL_TAP0234_ADP_EN
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
#else 

#endif
									}

									if(adj_le_done[nport][lane]==FALSE)
									{
										lib_hdmi_dfe_init_le(nport,(1<<lane),phy_st[nport].dfe_t[lane].le);
#if !HDMI_FRL_TAP0234_ADP_EN
										lib_hdmi_dfe_init_tap2(nport,(1<<lane),phy_st[nport].dfe_t[lane].tap2);										
#else 

#endif								
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
#if !HDMI_FRL_TAP0234_ADP_EN									
									phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];
#else 

#endif								
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
#if !HDMI_FRL_TAP0234_ADP_EN
										phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];
#else

#endif								
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
#if !HDMI_FRL_TAP0234_ADP_EN
										phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];
#else

#endif								
										HDMI_EMG("check le hit boundary, lane = %d, le = %d\n", lane, phy_st[nport].dfe_t[lane].le);			
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
									}
									else     //Change Tap2 value only when LE not in boundary condition
									{
#if !HDMI_FRL_TAP0234_ADP_EN
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
#else 

#endif								
									}

									if(adj_le_done[nport][lane]==FALSE)
									{
										lib_hdmi_dfe_init_le(nport,(1<<lane),phy_st[nport].dfe_t[lane].le);
#if !HDMI_FRL_TAP0234_ADP_EN
										lib_hdmi_dfe_init_tap2(nport,(1<<lane),phy_st[nport].dfe_t[lane].tap2);										
#else 

#endif								
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
	unsigned int current_time_ms = hdmi_get_system_time_ms();
	#define TAP1_MAX_VALUE	31
	#define TAP2_MAX_VALUE	31
	#define LE_MAX_VALUE	30
	int lane = 0;
	unsigned char mode;
	mode = DFE_P0_REG_DFE_MODE_P0_get_adapt_mode_p0(hdmi_in(DFE_REG_DFE_MODE_reg));
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
						if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
						{
							HDMI_PRINTF("[disp adp stage1] lane=%d, disp_err_zero_cnt %d over(tap1=%d)\n",lane,phy_st[nport].disp_err_zero_cnt[lane] , phy_st[nport].dfe_t[lane].tap1);
						}
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
						if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_HD20_PHY_LOG_ENABLE )) //for phy log debugging
						{
							HDMI_PRINTF("[disp adp stage1] lane=%d, disp_err_zero_cnt %d over(tap1=%d)\n",lane,phy_st[nport].disp_err_zero_cnt[lane] , phy_st[nport].dfe_t[lane].tap1);
						}
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
									else if(phy_st[nport].dfe_t[lane].le_add_tap1>(TAP1_MAX_VALUE+(DFE_P0_REG_DFE_EN_L0_P0_get_le_min_lane0_p0(hdmi_in(DFE_REG_DFE_EN_L0_reg)))))
									{
										phy_st[nport].dfe_t[lane].le_add_tap1 = (TAP1_MAX_VALUE+(DFE_P0_REG_DFE_EN_L0_P0_get_le_min_lane0_p0(hdmi_in(DFE_REG_DFE_EN_L0_reg))));
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].tap1_adp_step[lane] = - 1 *phy_st[nport].tap1_adp_step[lane];
										phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];

										HDMI_EMG("check tap1 hit boundary, lane = %d, le_add_tap1 = %d\n", lane, phy_st[nport].dfe_t[lane].le_add_tap1);			
									}

									lib_hdmi_dfe_init_tap1(nport,(1<<lane),phy_st[nport].dfe_t[lane].le_add_tap1);

									if(phy_st[nport].dfe_t[lane].le_add_tap1 <= (DFE_P0_REG_DFE_EN_L0_P0_get_le_min_lane0_p0(hdmi_in(DFE_REG_DFE_EN_L0_reg))))
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
									else if(phy_st[nport].dfe_t[lane].le_add_tap1>(TAP1_MAX_VALUE+(DFE_P0_REG_DFE_EN_L0_P0_get_le_min_lane0_p0(hdmi_in(DFE_REG_DFE_EN_L0_reg)))))
									{
										phy_st[nport].dfe_t[lane].le_add_tap1 = (TAP1_MAX_VALUE+(DFE_P0_REG_DFE_EN_L0_P0_get_le_min_lane0_p0(hdmi_in(DFE_REG_DFE_EN_L0_reg))));
										//phy_st[nport].disp_adp_stage[lane] = 3;  // end
										m_wrong_direction_cnt[nport][lane]=0;
										phy_st[nport].tap1_adp_step[lane] = - 1 *phy_st[nport].tap1_adp_step[lane];
										phy_st[nport].tap2_adp_step[lane] = - 1 *phy_st[nport].tap2_adp_step[lane];

										HDMI_EMG("check tap1 hit boundary, lane = %d, le_add_tap1 = %d\n", lane, phy_st[nport].dfe_t[lane].le_add_tap1);			
									}
									lib_hdmi_dfe_init_tap1(nport,(1<<lane),phy_st[nport].dfe_t[lane].le_add_tap1);

									if(phy_st[nport].dfe_t[lane].le_add_tap1 <= (DFE_P0_REG_DFE_EN_L0_P0_get_le_min_lane0_p0(hdmi_in(DFE_REG_DFE_EN_L0_reg))))
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

}




#if PHY_ADAPTIVE_BY_LTP_PAT_AFTER_LTP_PASS
//check ltp err cnt after finishing link training pass (for TEK CTS 20s ltp pattern still output)
void lib_hdmi_hd21_ltp_phy_adaptive_check(unsigned char nport, unsigned char lane_num)
{

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

   		
}
#endif

//check ltp err in link training state
void lib_hdmi_hd21_ltp_phy_adaptive(unsigned char nport, unsigned char lane_num)
{

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
   	

}




void newbase_rxphy_frl_job(unsigned char p)
{
//#if LINK_TRAINING_TX_INFO_DEBUG
	unsigned int err[4];
	unsigned char nport = 0;
//#endif
	if (p >= HD20_PORT) {
		HDMI_EMG("port=%d error %s need to HD21 port\n",p,__func__);
		return;
	}

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
			#if HDMI_FRL_TAP0234_ADP_EN
			lib_dfe_tap0234_adap_en(p, phy_st[p].lane_num, ON);
			#endif
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
				lib_hdmi21_phy_error_handling(p);
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
	if (p >= HD20_PORT) {
		HDMI_EMG("port error=%d %s need to HD20 port\n",p,__func__);
		return;
	}
	
	switch (phy_st[p].phy_proc_state) {
	case PHY_PROC_INIT:
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
		lib_hdmi21_fg_koffset_proc(p, phy_st[p].lane_num); //do koffset after tap0 finish
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
			//phy_st[p].phy_proc_state = PHY_PROC_INIT;
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


#if 0

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
#endif


#if 0
unsigned char newbase_rxphy_is_clk_stable(unsigned char port)
{
	//if frl mode no check clk, fix me
	if (hdmi_rx[port].hdmi_2p1_en) {
		return 1;
	}
	return phy_st[port].clk_stable;
}

unsigned char newbase_rxphy_is_tmds_mode(unsigned port)
{
	if (phy_st[port].frl_mode == MODE_TMDS)
		return 1;
	else 
		return 0;

}
#endif


#if 0
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
}

void newbase_hdmi_dfe_recovery_mi_speed(unsigned char port)
{
	phy_st[port].phy_proc_state = PHY_PROC_RECOVER_MI;
}

void newbase_hdmi_dfe_recovery_midband_long_cable(unsigned char port)
{
	phy_st[port].phy_proc_state = PHY_PROC_RECOVER_MID_BAND_LONGCABLE;

}

#endif

#if 0
void newbase_hdmi_set_frl_info(unsigned p, unsigned char frl_mode)
{
	switch(frl_mode)
	{
	case 0:
		phy_st[p].lane_num = HDMI_3LANE;
		phy_st[p].frl_mode = MODE_TMDS;
		break;
	case 1:
		phy_st[p].lane_num = HDMI_3LANE;
		phy_st[p].frl_mode = MODE_FRL_3G_3_LANE;
		phy_st[p].clk = FRL_3G;
		break;
	case 2:
		phy_st[p].lane_num = HDMI_3LANE;
		phy_st[p].frl_mode = MODE_FRL_6G_3_LANE;
		phy_st[p].clk = FRL_6G;
		break;
	case 3:   // FRL:6G/4Ln
		phy_st[p].lane_num = HDMI_4LANE;
		phy_st[p].frl_mode = MODE_FRL_6G_4_LANE;
		phy_st[p].clk = FRL_6G;
		break;
	case 4:   // FRL:6G/4Ln
		phy_st[p].lane_num = HDMI_4LANE;
		phy_st[p].frl_mode = MODE_FRL_8G_4_LANE;
		phy_st[p].clk = FRL_8G;
		break;
	case 5:   // FRL:6G/4Ln
		phy_st[p].lane_num = HDMI_4LANE;
		phy_st[p].frl_mode = MODE_FRL_10G_4_LANE;
		phy_st[p].clk = FRL_10G;
		break;
	case 6:   // FRL:6G/4Ln
		phy_st[p].lane_num = HDMI_4LANE;
		phy_st[p].frl_mode = MODE_FRL_12G_4_LANE;
		phy_st[p].clk = FRL_12G;
		break;
	default:
		phy_st[p].lane_num = HDMI_3LANE;
		phy_st[p].frl_mode = MODE_TMDS;
		break;
	}



}
#endif

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
			b = FRL_3G;
			break;
		case MODE_FRL_6G_3_LANE:
			b = FRL_6G;
			break;
		case MODE_FRL_6G_4_LANE:
			b = FRL_6G;
			break;
		case MODE_FRL_8G_4_LANE:
			b = FRL_8G;
			break;
		case MODE_FRL_10G_4_LANE:
			b = FRL_10G;
			break;
		case MODE_FRL_12G_4_LANE:
			b = FRL_12G;
			break;	
		default:
			b = 0;
			break;		
	}

	return b;

}

#if 0
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
	if (nport == HDMI_PORT3)  {
		if (ch == 0) {
			phy_fac_eq_st[nport].manual_eq[2] = eq_ch;
		}
		else if (ch == 1) {
			phy_fac_eq_st[nport].manual_eq[1] = eq_ch;
		}
		else if (ch == 2) {
			phy_fac_eq_st[nport].manual_eq[0] = eq_ch;

		} else {
			return;
		}

	} else {

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
	unsigned char bgrc;
	unsigned char mode;

	//phy_st[nport].dfe_t[0].le ~ phy_st[nport].dfe_t[3].le
	/*
	if (lane_mode == HDMI_4LANE)
		bgrc = LN_ALL;
	else
	*/
	if (phy_fac_eq_st[nport].eq_mode == EQ_MANUAL_OFF)
		return;
	

	phy_fac_eq_st[nport].manual_eq[0] = eq_ch0;
	phy_fac_eq_st[nport].manual_eq[1] = eq_ch1;
	phy_fac_eq_st[nport].manual_eq[2] = eq_ch2;
	
	bgrc = (LN_R|LN_G|LN_B);
	mode = DFE_P0_REG_DFE_MODE_P0_get_adapt_mode_p0(hdmi_in(DFE_REG_DFE_MODE_reg));


	if (mode == 2) {
/*
		if (bgrc&LN_CK) {
			hdmi_mask(DFE_REG_DFE_EN_L3_reg,
				~(DFE_HD21_REG_DFE_EN_L3_P0_le_init_lane3_p0_mask), (DFE_HD21_REG_DFE_EN_L3_P0_le_init_lane3_p0(value)));
		}
*/
		
		//le 0~24
		if (bgrc&LN_R) {
			hdmi_mask(DFE_REG_DFE_EN_L2_reg,
				~(DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0_mask), (DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0(eq_ch2)));
		}
		if (bgrc&LN_G) {
			hdmi_mask(DFE_REG_DFE_EN_L1_reg,
				~(DFE_P0_REG_DFE_EN_L1_P0_le_init_lane1_p0_mask), (DFE_P0_REG_DFE_EN_L1_P0_le_init_lane1_p0(eq_ch1)));
		}
		if (bgrc&LN_B) {
			hdmi_mask(DFE_REG_DFE_EN_L0_reg,
				~(DFE_P0_REG_DFE_EN_L0_P0_le_init_lane0_p0_mask), (DFE_P0_REG_DFE_EN_L0_P0_le_init_lane0_p0(eq_ch0)));
		}

		lib_hdmi_dfe_load_in_adp(nport, bgrc, LOAD_IN_INIT_LE);

	}
	 else if (mode == 3) {
		
		//mode 3
			//0~24
		/*	
		if (bgrc&LN_CK) {
			hdmi_mask(DFE_REG_DFE_INIT0_L3_reg,
				~(DFE_HD21_REG_DFE_INIT0_L3_P0_tap1_init_lane3_p0_mask), (DFE_HD21_REG_DFE_INIT0_L3_P0_tap1_init_lane3_p0(value)));
		}
		*/
		
		if (bgrc&LN_R) {
			hdmi_mask(DFE_REG_DFE_INIT0_L2_reg,
				~(DFE_P0_REG_DFE_INIT0_L2_P0_tap1_init_lane2_p0_mask), (DFE_P0_REG_DFE_INIT0_L2_P0_tap1_init_lane2_p0(eq_ch2)));
		}
		if (bgrc&LN_G) {
			hdmi_mask(DFE_REG_DFE_INIT0_L1_reg,
				~(DFE_P0_REG_DFE_INIT0_L1_P0_tap1_init_lane1_p0_mask), (DFE_P0_REG_DFE_INIT0_L1_P0_tap1_init_lane1_p0(eq_ch1)));
		}
		if (bgrc&LN_B) {
			hdmi_mask(DFE_REG_DFE_INIT0_L0_reg,
				~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask), (DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(eq_ch0)));
		}

		lib_hdmi_dfe_load_in_adp(nport, bgrc, LOAD_IN_INIT_TAP1);
	}	
	else {
		//do nothing
	}


}

#endif


unsigned char newbase_hdmi21_set_phy(unsigned char port, unsigned int b, unsigned char frl_mode,unsigned char lane)
{

	unsigned char mode;
	bool b_mode_found = 0;
	HDMI21_PHY_TMDS_PARAM_T *p_phy_table = hdmi21_phy_tmds_param;
	HDMI21_PHY_FRL_PARAM_T *p_frl_phy_table = hdmi21_phy_frl_param;
	HDMI21_FLD_PARAM_T *p_frl_fld_table = hdmi21_fld_param;
	unsigned char tb_size = 0;
//	unsigned char unlock_cnt = 0;
	unsigned char max_le = 0;
	unsigned char acdr_unlock = 0;
    unsigned char tx_flt_no_retrain = (lib_hdmi_scdc_read(port, SCDC_CONFIGURATION)&SCDC_FLT_NO_RETRAIN);
	
	HDMI_WARN("%s start[p%d]=%d frl=%d lane=%d\n", __func__, port, b,frl_mode,lane);

	if (b == 0) return 1;

#ifdef HDMI_FIX_HDMI_POWER_SAVING
	newbase_hdmi_reset_power_saving_state(port);    // reset power saving state
#endif

	lib_hdmi_mac_reset(port);
#if 1 
hdmi21_set_phy:
#endif
	lib_hdmi21_cdr_rst(port, 1,frl_mode, lane);
	lib_hdmi21_8b18b_fifo_rst(port,1,frl_mode, lane);
	lib_hdmi21_demux_ck_vcopll_rst(port, 1, lane);
//	lib_hdmi21_cmu_rst(port, 1);
	lib_hdmi21_cmu_pll_en(port, 0,frl_mode);

	lib_hdmi21_eq_bias_band_setting(port, b,frl_mode,lane);
	tb_size = lib_hdmi21_get_phy_table(port,b,&p_phy_table,&p_frl_phy_table,frl_mode);

	if (frl_mode == MODE_TMDS) {
		for (mode=0; mode<tb_size; mode++) {
			if ((p_phy_table[mode].b_upper > b) && (p_phy_table[mode].b_lower <= b)) {
				lib_hdmi21_cco_band_config(port, b, &p_phy_table[mode]); 
				lib_hdmi21_set_phy_table(port, &p_phy_table[mode]);
#ifdef HD20_FLD_MODE
				lib_hdmi_fld_config(port,&p_phy_table[mode]);
#endif
		              b_mode_found = 1;
		              break;
			}
		}
	} else {
		for (mode=0; mode<tb_size; mode++) {
			if ((p_frl_phy_table[mode].b_upper == b) && (p_frl_phy_table[mode].b_lower == b)) {
				lib_hdmi21_set_frl_phy_table(port, &p_frl_phy_table[mode]);
				lib_hdmi21_set_fld_table(port,&p_frl_fld_table[mode]);
		              b_mode_found = 1;
		              break;
			}
		}
	}

	if (!b_mode_found) {
	        HDMI_EMG("Not Found in PhyMode (b=%d)\n", b);
	        return 1;
	}
	
#ifdef CONFIG_POWER_SAVING_MODE
	lib_hdmi21_ac_couple_power_en(port,1);
	lib_hdmi21_eq_pi_power_en(port,1,frl_mode,lane);
#endif

//	lib_hdmi21_cmu_rst(port, 0);

	lib_hdmi21_wdog(port,frl_mode,lane);
	udelay(5);

	lib_hdmi21_cmu_pll_en(port, 1,frl_mode);
	udelay(1);
	
//acdr_start:
	
	lib_hdmi_set_dfe_close(port,lane);

	lib_hdmi21_acdr_finetune_en(port,0,lane);  //dcdr mode
	
	lib_hdmi21_acdr_power_en(port,0, frl_mode,lane);
	
	lib_hdmi21_fld_reset(port,0,frl_mode,lane);
	
	lib_hdmi21_fld_ck_en(port,0,frl_mode,lane);

	lib_hdmi21_cdr_init(port,frl_mode,lane);  //dcdr or acdr mode init


	lib_hdmi21_dfe_param_init(port,b,2,lane);//dfe_mode 2

	lib_hdmi21_fg_koffset_disable(port);//Garran 0903
		
	/********************** phase I  (check vco current)*******************/

	//enable fld clk
	lib_hdmi21_fld_ck_en(port,1,frl_mode,lane);
	
	//release fld rst  freq check and adjust I
#ifdef HD20_FLD_MODE
	lib_hdmi21_fld_reset(port,1,frl_mode,lane);
	//check coarse_calib_ok
	lib_hdmi21_coarse_calib_is_ok(port,lane);
#else
	if (frl_mode) {  //acdr
		lib_hdmi21_fld_reset(port,1,frl_mode,lane);
                //check coarse_calib_ok
	        lib_hdmi21_coarse_calib_is_ok(port,lane);
        } else {
        	//dcdr for icp setting apply
		lib_hdmi21_fld_reset(port,1,frl_mode,lane);
        }
#endif

	/**********************phase II   (PLL Mode)  ********************/

	lib_hdmi21_acdr_power_en(port,1, frl_mode,lane);
	udelay(40);

#if 1
	lib_hdmi21_demux_ck_vcopll_rst(port, 0, lane);
	udelay(1);

	lib_hdmi21_8b18b_fifo_rst(port,0,frl_mode, lane);
	udelay(1);
	
	lib_hdmi21_cdr_rst(port, 0, frl_mode,lane);
	udelay(1);
	//check after conard
	//lib_hdmi21_dfe_config_setting(port,lane);

	lib_hdmi21_fg_koffset_disable(port);
#endif	

	if (frl_mode == MODE_TMDS)
	{
		lib_hdmi21_acdr_finetune_en(port,0,lane);  //dcdr mode
		udelay(50);

		lib_hdmi21_fg_koffset_proc(port, lane); //do koffset after tap0 finish
		//DCDR mode dfe
		if (b > clock_bound_110m) {
			lib_hdmi21_set_dfe(port, b, 2, lane);  //use mode 2
		} else if ((b > clock_bound_45m) && (b < clock_bound_110m)) {
			lib_hdmi21_set_dfe_mid_band(port, b, 2, lane);
		} else {
			lib_hdmi21_dfe_manual_set(port,lane);
		}
		udelay(50);
//		if (newbase_hdmi21_get_koffset_flag(port))
//			lib_hdmi21_fg_koffset_proc(port, lane); //do koffset after tap0 finish
	}
	else {
		//Add Koffset flow
		if (lib_hdmi21_acdr_calib_status(port,lane,0))	
			lib_hdmi21_fg_koffset_auto(port,lane);
		
		lib_dfe_tap0_le_adp_en(port,lane,1); //DFE adaption enable for acdr mode
		lib_hdmi21_acdr_finetune_en(port,1,lane);
	}


	/**********************phase III   (ACDR  Mode)  ********************/


	// step1 dephy set acdr parameter and control acdr signal
	// step2 BBPD (reduce bandwidth of ACDR)
	// step3
	if (frl_mode) {
		if (lib_hdmi21_is_acdr_state(port,lane)) {
                        lib_hdmi_dfe_record_enable(port, 1);		//record enable (need to record after finuetune start)
			udelay((ADP_TIME+CALIB_TIME));  //check after
		}
		//else
		//	goto phase3;


		if (lib_hdmi21_acdr_calib_status(port,lane,1)) {
			lib_hdmi_dfe_record_enable(port, 0);	//record disable
			lib_dfe_tap0_disable(port,lane);
			//lib_hdmi_load_tap0_max(port,lane);
			udelay(20);
			lib_dfe_le_disable(port,lane);

			lib_hdmi21_dump_dfe_tap0_le(port,lane);
			//Add Koffset flow
			lib_hdmi21_acdr_finetune_en(port,0,lane);  //dcdr mode
			if (lib_hdmi21_acdr_calib_status(port,lane,0)) {			
				lib_hdmi21_fg_koffset_auto(port,lane);
				lib_hdmi21_acdr_finetune_en(port,1,lane);  //acdr mode
				if (lib_hdmi21_is_acdr_state(port,lane)) {
					udelay((ADP_TIME+CALIB_TIME)); //check after
				}
				if (lib_hdmi21_acdr_is_unlock(port,lane)) {
					HDMI_EMG("acdr unlock fail 1\n");
					if(acdr_unlock<10)
					{
						acdr_unlock++;
						HDMI_EMG("========1 acdr_unlock=%d ========\n",acdr_unlock);
						goto hdmi21_set_phy;
					}
					else
					{
					return 1;
					}
				}
			}
#if 0 //ori
			lib_hdmi_vth_set(port,lane,0x4);
#else

		if (!newbase_hdmi_get_longcable_flag(port))
		{
        		max_le =MAX(MAX(lib_hdmi_dfe_get_le(port, 0), lib_hdmi_dfe_get_le(port, 1)),MAX(lib_hdmi_dfe_get_le(port, 2), lib_hdmi_dfe_get_le(port, 3)));
#if 1
			if(( max_le>25)&&(!tx_flt_no_retrain))
			{
				HDMI_EMG("newbase_hdmi_set_longcable_flag\n");
				newbase_hdmi_set_longcable_flag(port,1);
				goto hdmi21_set_phy;
			}
			else
#endif
			{
				lib_hdmi_vth_set(port,lane,6); ////Vth = 6 for CTS
			}	
		}		
		else
		{
				HDMI_EMG("Set Vth = %d for long Cable\n",Vth_value);
				lib_hdmi_vth_set(port,lane,Vth_value); //0222RDC SD Allan Vth = 3 for long Cable
		}		
#endif
			udelay(20);
			lib_acdr_dfe_tap_all_adp_en(port,frl_mode,lane,1);
			udelay(50);
			lib_acdr_dfe_tap_all_adp_en(port,frl_mode,lane,0); //DFE adaption disable for acdr mode
			lib_hdmi_load_tap1_add(port,lane,6);  //HDMI2.1 CTS
			lib_hdmi_load_le_add(port,lane,2);    //HDMI2.1 CTS
		}
		//else
		//	goto phase3;


		if (lib_hdmi21_acdr_is_unlock(port,lane)) {
			HDMI_EMG("acdr unlock fail 2\n");
			if(acdr_unlock<10)
			{
				acdr_unlock++;
				HDMI_EMG("========2 acdr_unlock=%d ========\n",acdr_unlock);
				goto hdmi21_set_phy;
			}
			else
			{
			return 1;
			}
		}
		lib_hdmi21_acdr_finetune_en(port,0,lane);  //dcdr mode
		if (lib_hdmi21_acdr_calib_status(port,lane,0)) {			
			lib_hdmi21_fg_koffset_auto(port,lane);  //acdr mode witchout cdr reset

			lib_hdmi21_acdr_finetune_en(port,1,lane);  //acdr mode
			if (lib_hdmi21_is_acdr_state(port,lane)) {
				udelay((ADP_TIME+CALIB_TIME)); //check after
			}
			if (lib_hdmi21_acdr_is_unlock(port,lane)) {
				HDMI_EMG("acdr unlock fail 3\n");
				if(acdr_unlock<10)
				{
					acdr_unlock++;
					HDMI_EMG("========3 acdr_unlock=%d ========\n",acdr_unlock);
					goto hdmi21_set_phy;
				}
				else
				{
				return 1;
				}
			}
		}
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
		phy_st[nport].dfe_t[lane].vth = lib_hdmi_dfe_get_vth(nport, lane);
		phy_st[nport].dfe_t[lane].tap0 = lib_hdmi_dfe_get_tap0(nport, lane);
		phy_st[nport].dfe_t[lane].tap1 = lib_hdmi_dfe_get_tap1(nport, lane);
		phy_st[nport].dfe_t[lane].tap2 = lib_hdmi_dfe_get_tap2(nport, lane);
		phy_st[nport].dfe_t[lane].tap3 = lib_hdmi_dfe_get_tap3(nport, lane);
		phy_st[nport].dfe_t[lane].tap4 = lib_hdmi_dfe_get_tap4(nport, lane);
		phy_st[nport].dfe_t[lane].le = lib_hdmi_dfe_get_le(nport, lane);
		phy_st[nport].dfe_t[lane].lemax = lib_hdmi_dfe_get_lemax(nport, lane);
		phy_st[nport].dfe_t[lane].lemin = lib_hdmi_dfe_get_lemin(nport, lane);
	}

	newbase_hdmi21_dump_dfe_para(nport,lane_mode);
#if HDMI_FRL_TAP0234_ADP_EN
	lib_hdmi21_dfe_power(nport, 1,lane_mode); //tap0234 adapt for CTS
#else
	lib_hdmi21_dfe_power(nport, 0,lane_mode);
#endif
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

#if 0
unsigned char newbase_tmds_dfe_hi_speed_close(unsigned char nport)
{
	unsigned char lane;

	lib_hdmi_set_dfe_close(nport);

	newbase_hdmi_dfe_6g_long_cable_patch(nport);

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

	newbase_hdmi21_dump_dfe_para(nport);

	lib_hdmi21_dfe_power(nport, 0);

	return TRUE;
}
#endif

void newbase_hdmi21_tmds_dfe_mi_speed(unsigned char nport, unsigned int b_clk,unsigned char lane_mode)
{
	HDMI_PRINTF("%s\n", __func__);

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

#if 0

void newbase_hdmi_open_err_detect(unsigned char port)
{
	//init detect cnt
	phy_st[port].error_detect_count = 0;
	phy_st[port].dfe_thr_chk_cnt = 0;

}

#if 0

void newbase_hdmi_err_detect_add(unsigned char port)
{
		if (phy_st[port].error_detect_count <= MAX_ERR_DETECT_TIMES)
			++phy_st[port].error_detect_count;
		
}

unsigned char newbase_hdmi_err_detect_stop(unsigned char port)
{
	if (phy_st[port].error_detect_count > MAX_ERR_DETECT_TIMES)
		return 1;
	else
		return 0;
}

#endif

unsigned char newbase_hdmi_char_err_detection(unsigned char port,unsigned int *r_err, unsigned int *g_err, unsigned int *b_err)
{

	if (phy_st[port].char_err_loop == 0) {
		lib_hdmi_char_err_start(port, 0, TMDS_CHAR_ERR_PERIOD);
		phy_st[port].char_err_loop++;

	} else {
		if (lib_hdmi_char_err_get_error(port, r_err, g_err, b_err)) {
			phy_st[port].char_err_loop = 0;
			newbase_hdmi_ced_error_cnt_accumulated(port, b_err, g_err, r_err);
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

	if ((g_err > TMDS_CHAR_ERR_THREAD) && (r_err < 10) && (b_err < 10) && ((phy_st[port].dfe_t[1].le+phy_st[port].dfe_t[1].tap1)>10))
		return 1;

	return 0;
}

unsigned char newbase_hdmi_bit_err_detection(unsigned char port, unsigned int *r_err, unsigned int *g_err, unsigned int *b_err)
{
        unsigned char bit_err_det = 0;

	if (phy_st[port].bit_err_loop == 0) {
		lib_hdmi_bit_err_start(port, 0, TMDS_BIT_ERR_PERIOD);
		phy_st[port].bit_err_loop++;

	} else {
		if (lib_hdmi_bit_err_get_error(port, r_err, g_err, b_err)) {
			phy_st[port].bit_err_loop = 0;
			phy_st[port].bit_err_occurred = 0;

			if ((*r_err > TMDS_BIT_ERR_THREAD) || (*g_err > TMDS_BIT_ERR_THREAD) || (*b_err > TMDS_BIT_ERR_THREAD)) {
				// Do something to save the world
				HDMI_PRINTF("[BIT_ERR](%d, %d, %d)\n", *r_err, *g_err, *b_err);
				bit_err_det = 1;
			}


                         //USER:Lewislee DATE:2017/03/20
			//for KTASKWBS-6722, TATASKY 4K STB + HDMI Cable : Maxicom (Non standard)
			//The g_err bit error very large issue
			if((*g_err > TMDS_BIT_ERR_SIGNAL_BAD_THREAD) && (*r_err < 10) && (*b_err < 10)) {
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
#if 0
	if (phy_st[port].bit_err_loop == 0) {
		lib_hdmi_bit_err_start(port, 0, TMDS_BIT_ERR_MAX_LOOP);
		phy_st[port].bit_err_loop++;

	} else if (phy_st[port].bit_err_loop > TMDS_BIT_ERR_MAX_LOOP) {
		phy_st[port].bit_err_loop = 0;

		if (lib_hdmi_bit_err_get_error(port, &r_err, &g_err, &b_err)) {

			if ((r_err > TMDS_BIT_ERR_THREAD) || (g_err > TMDS_BIT_ERR_THREAD) || (b_err > TMDS_BIT_ERR_THREAD)) {
				// Do something to save the world
				HDMI_PRINTF("[BIT_ERR](%d, %d, %d)\n", r_err, g_err, b_err);
			}
			return 1;
		}

	} else {
		phy_st[port].bit_err_loop++;
	}
#endif
	return 0;

}


unsigned char newbase_hdmi_set_6G_long_cable_enable(unsigned char port, unsigned char enable)
{
	return phy_st[port].detect_6g_longcable_enable = enable;
}

#endif


#if 0
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

#endif

void newbase_hdmi21_dump_dfe_para(unsigned char nport, unsigned char lane_mode)
{
	unsigned char lane;

	for (lane=0; lane<lane_mode; lane++) {
		DFE_WARN("HDMI[p%d] ********************Lane%d******************\n", nport,lane);
		DFE_WARN("HDMI[p%d]  Lane%d Vth=0x%x\n", nport,lane, phy_st[nport].dfe_t[lane].vth);
		DFE_WARN("HDMI[p%d] Lane%d Tap0=0x%x, max=0x%02x, min=0x%02x\n",nport,

			lane, phy_st[nport].dfe_t[lane].tap0, phy_st[nport].dfe_t[lane].tap0max, phy_st[nport].dfe_t[lane].tap0min);

		DFE_WARN("HDMI[p%d]  Lane%d Tap1=0x%02x, Tap2=%+d Tap3=%+d Tap4=%+d\n",
			nport,lane, phy_st[nport].dfe_t[lane].tap1, phy_st[nport].dfe_t[lane].tap2,phy_st[nport].dfe_t[lane].tap3,phy_st[nport].dfe_t[lane].tap4);

		DFE_WARN("HDMI[p%d]  Lane%d LE=0x%02x, max=0x%02x, min=0x%02x\n",
			nport,lane, phy_st[nport].dfe_t[lane].le, phy_st[nport].dfe_t[lane].lemax, phy_st[nport].dfe_t[lane].lemin);
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
		DFE_WARN("HDMI[p%d]  Lane%d Tap0=0x%x, max=0x%02x, min=0x%02x\n",nport,lane, phy_st[nport].dfe_t[lane].tap0, phy_st[nport].dfe_t[lane].tap0max, phy_st[nport].dfe_t[lane].tap0min);
		DFE_WARN("HDMI[p%d] Lane%d LE=0x%02x, max=0x%02x, min=0x%02x\n",nport,lane, phy_st[nport].dfe_t[lane].le, phy_st[nport].dfe_t[lane].lemax, phy_st[nport].dfe_t[lane].lemin);
	}
	
}


#ifdef CONFIG_POWER_SAVING_MODE
void newbase_hdmi21_phy_pw_saving(unsigned char port)
{	
	if (port >= HD20_PORT) {
		return;
	}
	lib_hdmi21_cdr_rst(port, 1, MODE_TMDS,HDMI_4LANE);
	lib_hdmi21_demux_ck_vcopll_rst(port, 1, HDMI_4LANE);

	lib_hdmi21_ac_couple_power_en(port,0);
	lib_hdmi21_eq_pi_power_en(port,0,MODE_TMDS,HDMI_4LANE);
//	lib_hdmi21_cmu_rst(port, 1);
	lib_hdmi21_cmu_pll_en(port, 0,MODE_TMDS);
	
}

#endif



#if 0 //HDMI_FRL_TRANS_DET

void lib_hdmi_transition_cnt_en(unsigned char nport, unsigned char enable)
{

}

void lib_hdmi_eq_init(unsigned char nport)
{
	lib_hdmi21_eq_pi_power_en(nport,1,MODE_TMDS,HDMI_4LANE);

	lib_hdmi21_dfe_power(nport, 1,HDMI_4LANE);
	
	hdmi_mask(DFE_REG_DFE_MODE_reg,
		~(DFE_P0_REG_DFE_MODE_P0_adapt_mode_p0_mask|DFE_P0_REG_DFE_MODE_P0_edge_det_mode_p0_mask|
		DFE_P0_REG_DFE_MODE_P0_transition_only_p0_mask|DFE_P0_REG_DFE_MODE_P0_tap1_delay_p0_mask|
		DFE_P0_REG_DFE_MODE_P0_tap24_delay_p0_mask|DFE_P0_REG_DFE_MODE_P0_le_delay_p0_mask|
		DFE_P0_REG_DFE_MODE_P0_servo_delay_p0_mask|DFE_P0_REG_DFE_MODE_P0_gray_en_p0_mask|
		DFE_P0_REG_DFE_MODE_P0_dfe_adapt_rstb_p0_mask|
		DFE_P0_REG_DFE_MODE_P0_tap0_notrans_p0_mask),
		(DFE_P0_REG_DFE_MODE_P0_adapt_mode_p0(3)|DFE_P0_REG_DFE_MODE_P0_edge_det_mode_p0(0)|
		DFE_P0_REG_DFE_MODE_P0_transition_only_p0_mask|DFE_P0_REG_DFE_MODE_P0_tap1_delay_p0(1)|
		DFE_P0_REG_DFE_MODE_P0_tap24_delay_p0(1)|DFE_P0_REG_DFE_MODE_P0_le_delay_p0(1)|
		DFE_P0_REG_DFE_MODE_P0_servo_delay_p0(1)|DFE_P0_REG_DFE_MODE_P0_gray_en_p0_mask|
		DFE_P0_REG_DFE_MODE_P0_tap0_notrans_p0_mask));

	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(DFE_P0_REG_DFE_EN_L0_P0_clk_inv_lane0_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_timer_ctrl_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_reduce_adapt_gain_lane2_p0_mask|DFE_REG_DFE_EN_L2_rl_det_en_lane2_mask|DFE_REG_DFE_EN_L2_rl_threshold_lane2_mask|DFE_REG_DFE_EN_L2_dfe_adapt_en_lane2_mask|DFE_REG_DFE_EN_L2_le_min_lane2_mask|DFE_REG_DFE_EN_L2_le_init_lane2_mask|DFE_REG_DFE_EN_L2_tap4_init_lane2_mask),(DFE_REG_DFE_EN_L2_timer_ctrl_en_lane2_mask|DFE_REG_DFE_EN_L2_reduce_adapt_gain_lane2(2))|DFE_REG_DFE_EN_L2_rl_threshold_lane2(1)|DFE_REG_DFE_EN_L2_le_min_lane2(0x18)|DFE_REG_DFE_EN_L2_le_init_lane2(0x8));
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(DFE_P0_REG_DFE_EN_L1_P0_clk_inv_lane1_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_timer_ctrl_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_reduce_adapt_gain_lane2_p0_mask|DFE_REG_DFE_EN_L2_rl_det_en_lane2_mask|DFE_REG_DFE_EN_L2_rl_threshold_lane2_mask|DFE_REG_DFE_EN_L2_dfe_adapt_en_lane2_mask|DFE_REG_DFE_EN_L2_le_min_lane2_mask|DFE_REG_DFE_EN_L2_le_init_lane2_mask|DFE_REG_DFE_EN_L2_tap4_init_lane2_mask),(DFE_REG_DFE_EN_L2_timer_ctrl_en_lane2_mask|DFE_REG_DFE_EN_L2_reduce_adapt_gain_lane2(2))|DFE_REG_DFE_EN_L2_rl_threshold_lane2(1)|DFE_REG_DFE_EN_L2_le_min_lane2(0x18)|DFE_REG_DFE_EN_L2_le_init_lane2(0x8));
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(DFE_P0_REG_DFE_EN_L2_P0_clk_inv_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_timer_ctrl_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_reduce_adapt_gain_lane2_p0_mask|DFE_REG_DFE_EN_L2_rl_det_en_lane2_mask|DFE_REG_DFE_EN_L2_rl_threshold_lane2_mask|DFE_REG_DFE_EN_L2_dfe_adapt_en_lane2_mask|DFE_REG_DFE_EN_L2_le_min_lane2_mask|DFE_REG_DFE_EN_L2_le_init_lane2_mask|DFE_REG_DFE_EN_L2_tap4_init_lane2_mask),(DFE_REG_DFE_EN_L2_timer_ctrl_en_lane2_mask|DFE_REG_DFE_EN_L2_reduce_adapt_gain_lane2(2))|DFE_REG_DFE_EN_L2_rl_threshold_lane2(1)|DFE_REG_DFE_EN_L2_le_min_lane2(0x18)|DFE_REG_DFE_EN_L2_le_init_lane2(0x8));
	hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(DFE_P0_REG_DFE_EN_L3_P0_clk_inv_lane3_p0_mask|DFE_P0_REG_DFE_EN_L3_P0_timer_ctrl_en_lane3_p0_mask|DFE_HD21_REG_DFE_EN_L3_P0_reduce_adapt_gain_lane3_p0_mask|DFE_HD21_REG_DFE_EN_L3_P0_rl_det_en_lane3_p0_mask|DFE_HD21_REG_DFE_EN_L3_P0_rl_threshold_lane3_p0_mask|DFE_HD21_REG_DFE_EN_L3_P0_dfe_adapt_en_lane3_p0_mask|DFE_HD21_REG_DFE_EN_L3_P0_le_min_lane3_p0_mask|DFE_HD21_REG_DFE_EN_L3_P0_le_init_lane3_p0_mask|DFE_HD21_REG_DFE_EN_L3_P0_tap4_init_lane3_p0_mask),(DFE_HD21_REG_DFE_EN_L3_P0_timer_ctrl_en_lane3_p0_mask|DFE_REG_DFE_EN_L2_reduce_adapt_gain_lane2(2))|DFE_REG_DFE_EN_L2_rl_threshold_lane2(1)|DFE_REG_DFE_EN_L2_le_min_lane2(0x18)|DFE_REG_DFE_EN_L2_le_init_lane2(0x8));


	hdmi_mask(DFE_REG_DFE_INIT0_L0_reg,
		~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_REG_DFE_INIT0_L0_vth_init_lane0_mask),
		(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(0xc)|DFE_REG_DFE_INIT0_L0_tap0_init_lane0(0x14)|DFE_REG_DFE_INIT0_L0_vth_init_lane0(0x3)));
	hdmi_mask(DFE_REG_DFE_INIT0_L1_reg,
		~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_REG_DFE_INIT0_L0_vth_init_lane0_mask),
		(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(0xc)|DFE_REG_DFE_INIT0_L0_tap0_init_lane0(0x14)|DFE_REG_DFE_INIT0_L0_vth_init_lane0(0x3)));
	hdmi_mask(DFE_REG_DFE_INIT0_L2_reg,
		~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_REG_DFE_INIT0_L0_vth_init_lane0_mask),
		(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(0xc)|DFE_REG_DFE_INIT0_L0_tap0_init_lane0(0x14)|DFE_REG_DFE_INIT0_L0_vth_init_lane0(0x3)));

	hdmi_mask(DFE_REG_DFE_INIT0_L3_reg,
			~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_REG_DFE_INIT0_L0_vth_init_lane0_mask),
			(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(0xc)|DFE_REG_DFE_INIT0_L0_tap0_init_lane0(0x14)|DFE_REG_DFE_INIT0_L0_vth_init_lane0(0x3)));
		

	//cloud teset
	hdmi_mask(DFE_REG_DFE_INIT1_L0_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_init1_rev1_lane0_p0_mask),(_BIT19|_BIT21|_BIT23));
	hdmi_mask(DFE_REG_DFE_INIT1_L1_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_init1_rev1_lane0_p0_mask),(_BIT19|_BIT21|_BIT23));
	hdmi_mask(DFE_REG_DFE_INIT1_L2_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_init1_rev1_lane0_p0_mask),(_BIT19|_BIT21|_BIT23));
	hdmi_mask(DFE_REG_DFE_INIT1_L3_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_init1_rev1_lane0_p0_mask),(_BIT19|_BIT21|_BIT23));


	//load initial data
	hdmi_mask(DFE_REG_DFE_INIT1_L0_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask);
	hdmi_mask(DFE_REG_DFE_INIT1_L1_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask);
	hdmi_mask(DFE_REG_DFE_INIT1_L2_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask);
	hdmi_mask(DFE_REG_DFE_INIT1_L3_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask);

	udelay(1);

	hdmi_mask(DFE_REG_DFE_INIT1_L0_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),0);
	hdmi_mask(DFE_REG_DFE_INIT1_L1_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),0);
	hdmi_mask(DFE_REG_DFE_INIT1_L2_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),0);
	hdmi_mask(DFE_REG_DFE_INIT1_L3_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),0);


}

//only continue mode
void lib_hdmi_lane_rate_detect_start(unsigned char nport)
{


	lib_hdmi_transition_cnt_en(nport,1); //power consumption need to check
	lib_hdmi_eq_init(nport);
	
	hdmi_mask(HD21_CLKDETSR_reg,~(HDMI21_P0_HD21_CLKDETSR_reset_counter_mask),0x0);
	hdmi_mask(HD21_CLKDET2SR_reg,~(HDMI21_P0_HD21_CLKDET2SR_reset_counter_mask),0x0);

	//set input clock counter for 1st detect module
	hdmi_mask(HD21_CLK_SETTING_00_reg,
			    ~(HDMI21_P0_HD21_CLK_SETTING_00_dclk_cnt_start_mask|HDMI21_P0_HD21_CLK_SETTING_00_dclk_cnt_end_mask),
			    HDMI21_P0_HD21_CLK_SETTING_00_dclk_cnt_start(1)|HDMI21_P0_HD21_CLK_SETTING_00_dclk_cnt_end(0x400));

	//set input clock counter for 2nd detect module
	hdmi_mask(HD21_CLK2_SETTING_00_reg,
			    ~(HDMI21_P0_HD21_CLK2_SETTING_00_dclk_cnt_start_mask|HDMI21_P0_HD21_CLK2_SETTING_00_dclk_cnt_end_mask),
			    HDMI21_P0_HD21_CLK2_SETTING_00_dclk_cnt_start(1)|HDMI21_P0_HD21_CLK2_SETTING_00_dclk_cnt_end(0x400));
	
	//CK lane transition detect
	hdmi_mask(HD21_CLKDETCR_reg,~(HDMI21_P0_HD21_CLKDETCR_det_sel_mask),HDMI21_P0_HD21_CLKDETCR_det_sel(1));
	
	//measure ck lane and r lane at the same time
	hdmi_mask(HD21_CLKDETCR_reg,~(HDMI21_P0_HD21_CLKDETCR_trans_meas_sel_mask), HDMI21_P0_HD21_CLKDETCR_trans_meas_sel(3));	
	#if HDMI_DFE_BRSWAP  //connect blane & measure r lane (br swap), so it will get b freq
	hdmi_mask(HD21_CLKDET2CR_reg,~(HDMI21_P0_HD21_CLKDET2CR_trans_meas_sel_mask), HDMI21_P0_HD21_CLKDETCR_trans_meas_sel(2));
	#else
	hdmi_mask(HD21_CLKDET2CR_reg,~(HDMI21_P0_HD21_CLKDET2CR_trans_meas_sel_mask), HDMI21_P0_HD21_CLKDETCR_trans_meas_sel(0));
	#endif
	
	
	hdmi_mask(HD21_CLKDETCR_reg,~(HDMI21_P0_HD21_CLKDETCR_clock_det_en_mask), HDMI21_P0_HD21_CLKDETCR_clock_det_en_mask);
	hdmi_mask(HD21_CLKDET2CR_reg,~(HDMI21_P0_HD21_CLKDET2CR_clock_det_en_mask), HDMI21_P0_HD21_CLKDET2CR_clock_det_en_mask);
	msleep(5);

}



void lib_hdmi_lane_rate_detect_stop(unsigned char nport)
{
	
	hdmi_mask(HD21_CLKDETSR_reg,~(HDMI21_P0_HD21_CLKDETSR_pop_out_mask),HDMI21_P0_HD21_CLKDETSR_pop_out_mask);
	hdmi_mask(HD21_CLKDET2SR_reg,~(HDMI21_P0_HD21_CLKDET2SR_pop_out_mask),HDMI21_P0_HD21_CLKDET2SR_pop_out_mask);

}

int lib_hdmi_lane_rate_is_ready(unsigned char nport)
{
	unsigned char cnt = 255;
	

	if (!HDMI21_P0_HD21_CLKDETCR_get_clock_det_en(hdmi_in(HD21_CLKDETCR_reg)))
		return 0;
	
	while (((hdmi_in(HD21_CLKDETSR_reg))&HDMI21_P0_HD21_CLKDETSR_pop_out_mask) && --cnt) {
			udelay(10);
	}

	if (cnt == 0) {
			HDMI_EMG("CLKDETSR pop timeout\n");
			return 0;
	}

	cnt = 255;
	while (((hdmi_in(HD21_CLKDET2SR_reg))&HDMI21_P0_HD21_CLKDET2SR_pop_out_mask) && --cnt) {
			udelay(10);
	}

	if (cnt == 0) {
			HDMI_EMG("CLKDET2SR pop timeout\n");
			return 0;
	}

	return 1;
	
}


int lib_hdmi_get_lane_rate(unsigned char nport)
{
	unsigned int ck_xtal_cnt = 0, b_xtal_cnt = 0;
	unsigned int tc;
	unsigned int bfreq,ckfreq;

	
	if (!lib_tap0_signal_detect(nport))  //no signal
		return MODE_TMDS;

	ck_xtal_cnt = HDMI21_P0_HD21_CLKDETSR_get_clk_counter(hdmi_in(HD21_CLKDETSR_reg));
	b_xtal_cnt = HDMI21_P0_HD21_CLKDET2SR_get_clk_counter(hdmi_in(HD21_CLKDET2SR_reg));
	tc = HDMI21_P0_HD21_CLK_SETTING_00_get_dclk_cnt_end(hdmi_in(HD21_CLK_SETTING_00_reg));

	// 12G clk_trans_div = 12G/2*0.55/64 = 51.5624  (536)
	// 10G clk_trans_div = 10G/2*0.55/64 = 42.968M  (643)
	// 8G clk_trans_div = 8G/2*0.55/64 = 34.375M  (804)
	// 6G clk_trans_div = 6G/2*0.55/64 = 25.75M  (23~ 28)  (1069)
	// 3G clk_trans_div = 3G/2*0.55/64 = 12.89M  (10~15) (2144)


	HDMI_EMG("ck_xtal_cnt=%d  b_xtal_cnt=%d tc=%d\n",ck_xtal_cnt,b_xtal_cnt,tc);

	if ((ck_xtal_cnt == 0) || (b_xtal_cnt == 0))
		return MODE_TMDS;

	bfreq= tc * 27 / b_xtal_cnt ;
	ckfreq = tc * 27/ ck_xtal_cnt;

	//connect blane & measure r lane (br swap), so it will get b freq
	HDMI_EMG("bfreq=%d ckfreq=%d\n",bfreq,ckfreq);


        if (((ckfreq > 23) && (ckfreq < 28)) && ((bfreq > 23) && (bfreq < 28)))
		return MODE_FRL_6G_4_LANE;
	else {
		if ((bfreq > 23) && ( bfreq < 28)) {
			return MODE_FRL_6G_3_LANE;
		}else if ((bfreq > 10) && (bfreq < 15)) {
			return MODE_FRL_3G_3_LANE;
		}
		else
			return MODE_TMDS;
	}
	
	
}
#endif


void lib_hdmi21_hysteresis_en(unsigned char en)
{
	if (en) {
		hdmi_mask(reg_ck_cmu[0],~(cmu_ck_1_HYS_AMP_EN|cmu_ck_1_MD_DET_SRC|cmu_ck_1_HYS_WIN_SEL|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN),
		(cmu_ck_1_HYS_AMP_EN|cmu_ck_1_NON_HYS_AMP_EN|cmu_ck_1_MD_DET_SRC|cmu_ck_1_ENHANCE_BIAS_10|cmu_ck_1_CKIN_SEL));

		hdmi_mask(reg_ck_cmu[1],~(cmu_ck_1_HYS_AMP_EN|cmu_ck_1_MD_DET_SRC|cmu_ck_1_HYS_WIN_SEL|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN),
		(cmu_ck_1_HYS_AMP_EN|cmu_ck_1_NON_HYS_AMP_EN|cmu_ck_1_MD_DET_SRC|cmu_ck_1_ENHANCE_BIAS_10|cmu_ck_1_CKIN_SEL));


	}else {
		hdmi_mask(reg_ck_cmu[0],~(cmu_ck_1_HYS_AMP_EN|cmu_ck_1_MD_DET_SRC|cmu_ck_1_HYS_WIN_SEL|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN),
		(cmu_ck_1_ENHANCE_BIAS_01|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN));
		hdmi_mask(reg_ck_cmu[1],~(cmu_ck_1_HYS_AMP_EN|cmu_ck_1_MD_DET_SRC|cmu_ck_1_HYS_WIN_SEL|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN),
		(cmu_ck_1_ENHANCE_BIAS_01|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN));

	}
}

void lib_hdmi21_stb_tmds_clk_wakeup_en(unsigned char en)
{
	unsigned int stb_hysteresis_window_otp = rtd_inl(EFUSE_DATAO_122_reg)>>23;
	
	unsigned int otp_done = (stb_hysteresis_window_otp&0x100)>>8;
	unsigned int port2_window = (otp_done==0x1)?(stb_hysteresis_window_otp&0x30)>>4 : 0;
	unsigned int port3_window = (otp_done==0x1)?(stb_hysteresis_window_otp&0xc0)>>6 : 0;

	if (en) {
		//HDMI Z0 set (CK Lane)
		hdmi_mask(HDMIRX_2P1_PHY_Z0POW_reg, ~(HDMIRX_2P1_PHY_Z0POW_reg_z0_z0pow_ck_mask),HDMIRX_2P1_PHY_Z0POW_reg_z0_z0pow_ck_mask); //Z0 power on
		hdmi_mask(HDMIRX_2P1_PHY_TOP_IN_0_reg, ~(Z0_P_OFF|Z0_N_OFF), 0); //[1] Noff [0]poff :0 =>normal
		hdmi_mask(HDMIRX_2P1_PHY_TOP_IN_2_reg, ~(BIAS_EN), BIAS_EN); //MBIAS setting
		
		//Enable switch and 3.3V hysteresis amp
		hdmi_mask(HDMIRX_2P1_PHY_TOP_IN_0_reg,~(CK_detect_DIV8_enable),CK_detect_DIV8_enable);	//TOP_IN_1[6]=1, CK-detect and DIV8 enable; 
		//TOP_IN_4[1:0], port-2 CK-detect(standby mode use) hysteresis window selec
		//TOP_IN_4[3:2], port-3 CK-detect(standby mode use) hysteresis window select
		hdmi_mask(HDMIRX_2P1_PHY_TOP_IN_0_reg,~(Port1_Hysteresis_Window|Port0_Hysteresis_Window),((port3_window<<26)|(port2_window<<24)));
		hdmi_mask(HDMIRX_2P1_PHY_TOP_IN_2_reg,~(CK_LATCH),0);	// REG_CK_LATCH=0, CK_LATCH=0
		hdmi_mask(HDMIRX_2P1_PHY_TOP_IN_2_reg,~(CK_LATCH),CK_LATCH);	// REG_CK_LATCH=1, CK_LATCH=1
	}else {
		hdmi_mask(HDMIRX_2P1_PHY_TOP_IN_0_reg,~(CK_detect_DIV8_enable),0);	//TOP_IN_1[6]=1, CK-detect and DIV8 enable; 
	}
}

void lib_hdmi21_phy_init(void)
{
	unsigned char i;

	//MBias
	hdmi_mask(HDMIRX_2P1_PHY_TOP_IN_0_reg, ~(IB_LEQ_ADJ_L|IPTAT_LEQ_ADJ_L|CK_CMP|Z0_N_OFF|Z0_P_OFF), IB_LEQ_ADJ_L_001|IPTAT_LEQ_ADJ_L_101|CK_CMP);
	hdmi_mask(HDMIRX_2P1_PHY_TOP_IN_2_reg,
			     ~(BG_ENVBGUP|BG_RBG2|BG_RBG|BG_RBGLOOP2|BIAS_EN),
				BG_RBG2_DEFAULT|BG_RBG_DEFAULT|BG_RBGLOOP2_DEFAULT|BIAS_EN);  //bandgap reference power

	//CK lane AC couple  (for hystersis amp work)
	for (i=0; i<HD20_PORT; i++) 
	{
		unsigned char nport = i;
		
		HDMI_EMG("[%s][%d][nport = %d]\n", __FUNCTION__, __LINE__, nport);
		if(newbase_hdmi_phy_port_status(i) == HDMI_PHY_PORT_NOTUSED) 
			continue;

		hdmi_mask(HDMIRX_PHY_CK1_reg, ~ck_5_AC_COPULE_POW, ck_5_AC_COPULE_POW);
	}

	hdmi_mask(HDMIRX_2P1_PHY_P0_CK0_reg, ~ck_2_RXVCM_SEL, ck_2_RXVCM_10);
	hdmi_mask(HDMIRX_2P1_PHY_P1_CK0_reg, ~ck_2_RXVCM_SEL, ck_2_RXVCM_10);


	//BIAS & VTH dac setting
	for (i=0; i<HD20_PORT; i++) {
		lib_hdmi21_dfe_init(i);
		lib_hdmi21_bias_init(i);
	}


	//Z0 & Koffset init
	for (i=0; i<HD20_PORT; i++) {
		if(newbase_hdmi_phy_port_status(i) == HDMI_PHY_PORT_NOTUSED) 
			continue;
			
		hdmi_mask(HDMIRX_2P1_PHY_TOP_IN_1_reg, ~(HD21_TOP_IN_CK_TX_0<< (i << 3)), (HD21_TOP_IN_CK_TX_0<<(i<<3)));
		lib_hdmi21_fg_koffset_manual_adjust(i, 0x10, 0x10, 0x10,0x10);
	}

	lib_hdmi21_z0_calibration();
	lib_hdmi21_ldo_calibration();


	for (i=0; i<HD20_PORT; i++) {

		if(newbase_hdmi_phy_port_status(i) == HDMI_PHY_PORT_NOTUSED)
		{	//if nport not used, turn off some power setting		
			hdmi_mask(reg_ck_cmu[i],~(cmu_ck_1_HYS_AMP_EN|cmu_ck_1_MD_DET_SRC|cmu_ck_1_HYS_WIN_SEL|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN), 0);
			
			HDMI_EMG("[%s][%d][0xb800DB2C = %x]\n", __FUNCTION__, __LINE__, rtd_inl(0xb800DB2C));
			continue;
		}
		lib_hdmi21_z300_sel(i, 0);

#if !HDMI_HYSTERESIS_EN  //hysteresis off
		//ACDR & CMU & non-hys-path
		hdmi_mask(reg_ck_cmu[i],~(cmu_ck_1_HYS_AMP_EN|cmu_ck_1_MD_DET_SRC|cmu_ck_1_HYS_WIN_SEL|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN),
		(cmu_ck_1_ENHANCE_BIAS_01|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN));
#else

		hdmi_mask(reg_ck_cmu[i],~(cmu_ck_1_HYS_AMP_EN|cmu_ck_1_MD_DET_SRC|cmu_ck_1_HYS_WIN_SEL|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN),
		(cmu_ck_1_HYS_AMP_EN|cmu_ck_1_MD_DET_SRC|cmu_ck_1_ENHANCE_BIAS_10|cmu_ck_1_CKIN_SEL|cmu_ck_1_NON_HYS_AMP_EN));
#endif

		//for data ldo
		hdmi_mask(reg_acdr_rgbc0[i][0],~(acdr_r_3_VC_TUNE|acdr_r_3_ICP_TUNE|acdr_r_2_CKFB|acdr_r_1_CCO_POW|acdr_r_1_LDO_EN),acdr_r_3_VC_DEFAULT|acdr_r_3_ICP_DEFAULT|acdr_r_1_CCO_POW|acdr_r_1_LDO_EN);
		hdmi_mask(reg_acdr_rgbc0[i][1],~(acdr_g_3_VC_TUNE|acdr_g_3_ICP_TUNE|acdr_g_2_CKFB|acdr_g_1_CCO_POW|acdr_g_1_LDO_EN),acdr_g_3_VC_DEFAULT|acdr_g_3_ICP_DEFAULT|acdr_g_1_CCO_POW|acdr_g_1_LDO_EN);
		hdmi_mask(reg_acdr_rgbc0[i][2],~(acdr_b_3_VC_TUNE|acdr_b_3_ICP_TUNE|acdr_b_2_CKFB|acdr_b_1_CCO_POW|acdr_b_1_LDO_EN),acdr_b_3_VC_DEFAULT|acdr_b_3_ICP_DEFAULT|acdr_b_1_CCO_POW|acdr_b_1_LDO_EN);
		hdmi_mask(reg_acdr_rgbc0[i][3],~(acdr_ck_3_VC_TUNE|acdr_ck_3_ICP_TUNE|acdr_ck_2_CKFB|acdr_ck_1_CCO_POW|acdr_ck_1_LDO_EN),acdr_ck_3_VC_DEFAULT|acdr_ck_3_ICP_DEFAULT|acdr_ck_1_CCO_POW|acdr_ck_1_LDO_EN);

		hdmi_mask(reg_acdr_rgbc1[i][0],~(acdr_r_7_CMU_PREDIVN|acdr_r_5_VC_TEST),0);
		hdmi_mask(reg_acdr_rgbc1[i][1],~(acdr_g_7_CMU_PREDIVN|acdr_g_5_VC_TEST),0);
		hdmi_mask(reg_acdr_rgbc1[i][2],~(acdr_b_7_CMU_PREDIVN|acdr_b_5_VC_TEST),0);
		hdmi_mask(reg_acdr_rgbc1[i][3],~(acdr_ck_7_CMU_PREDIVN|acdr_ck_5_VC_TEST),0);

		//hdmi_mask(DFE_HDMI_RX_PHY_LDO_reg, ~(DFE_HDMI_RX_PHY_LDO_reg_p0_ldo_pow_mask), (0));

		hdmi_mask(reg_rgbc1[i][0], ~b_8_POW_PR, b_8_POW_PR);
		hdmi_mask(reg_rgbc1[i][1], ~b_8_POW_PR, b_8_POW_PR);
		hdmi_mask(reg_rgbc1[i][2], ~b_8_POW_PR, b_8_POW_PR);
		hdmi_mask(reg_rgbc1[i][3], ~b_8_POW_PR, b_8_POW_PR);

		lib_fast_swtch_mode(i,PS_NORMAL_MODE);
	
	}

}


void lib_hdmi21_z0_set(unsigned char port, unsigned char lane, unsigned char enable)
{
	unsigned int bmask = 0;
	unsigned int Z0bmask = HDMIRX_2P1_PHY_Z0POW_reg_z0_z0pow_ck(1<<(port))|HDMIRX_2P1_PHY_Z0POW_reg_z0_z0pow_b(1<<(port))|
									HDMIRX_2P1_PHY_Z0POW_reg_z0_z0pow_g(1<<(port))|HDMIRX_2P1_PHY_Z0POW_reg_z0_z0pow_r(1<<(port));

	HDMI_INFO("Set_21_Z0_Port=%d, Z0bmask=%x, En=%d\n", port, Z0bmask, enable);

	//if (enable == 0)
		hdmi_mask(HDMIRX_2P1_PHY_TOP_IN_2_reg, ~(CK_LATCH), 0);  //latch bit clear

	if (lane&LN_CK) {
		bmask |= HDMIRX_2P1_PHY_Z0POW_reg_z0_z0pow_ck(1<<(port));
	}
	if (lane&LN_B) {
		bmask |= HDMIRX_2P1_PHY_Z0POW_reg_z0_z0pow_b(1<<(port));
	}
	if (lane&LN_G) {
		bmask |= HDMIRX_2P1_PHY_Z0POW_reg_z0_z0pow_g(1<<(port));
	}
	if (lane&LN_R) {
		bmask |= HDMIRX_2P1_PHY_Z0POW_reg_z0_z0pow_r(1<<(port));
	}
	
	if (!bmask) {
		return;
	}

	
	hdmi_mask(HDMIRX_2P1_PHY_Z0POW_reg, ~Z0bmask, enable?bmask:0);
	hdmi_mask(HDMIRX_2P1_PHY_TOP_IN_0_reg, ~(HD21_TOP_IN_Z0_FIX_SELECT_0 << (2*port)), enable?(HD21_TOP_IN_Z0_FIX_SELECT_10 << (2*port)):0 );
	HDMI_INFO("Z0POW_reg=%x, TOP_IN_reg=%x, bmask=%x\n", hdmi_in(HDMIRX_2P1_PHY_Z0POW_reg),hdmi_in(HDMIRX_2P1_PHY_TOP_IN_0_reg),bmask);

	//if (enable == 0)
		hdmi_mask(HDMIRX_2P1_PHY_TOP_IN_2_reg, ~(CK_LATCH), CK_LATCH);  //latch bit set
		
}


void lib_hdmi21_z300_sel(unsigned char port, unsigned char mode)  // 0: active mode 1:vcm mode
{
	
	if (mode) {
		hdmi_mask(HDMIRX_2P1_PHY_Z0POW_reg, ~(HDMIRX_2P1_PHY_Z0POW_reg_z0_z300_sel((1<<(port)))), HDMIRX_2P1_PHY_Z0POW_reg_z0_z300_sel((1<<(port))));
	} else {
		hdmi_mask(HDMIRX_2P1_PHY_Z0POW_reg, ~(HDMIRX_2P1_PHY_Z0POW_reg_z0_z300_sel((1<<(port)))), 0);
	}
}


#if 0

void lib_hdmi_ck_md_ref_sel(unsigned char port)
{

	const unsigned int ck_addr[HDMI_PORT_TOTAL_NUM] = {
	        DFE_HDMI_RX_PHY_P0_CK1_reg,  /* p0_ck_2_CK_MD_REF_SEL */
	        DFE_HDMI_RX_PHY_P1_CK1_reg,  /* p1_ck_2_CK_MD_REF_SEL */
	        DFE_HDMI_RX_PHY_P2_CK1_reg,   /* p2_ck_2_CK_MD_REF_SEL */
	        DFE_HDMI_RX_PHY_P0_HD21_P0_CMU1_reg   /* p2_ck_2_CK_MD_REF_SEL */
	};

	if (port >= HDMI_PORT_TOTAL_NUM) {
		return;
	}

	hdmi_mask(ck_addr[port], ~p0_ck_2_CK_MD_REF_SEL, p0_ck_2_CK_MD_REF_SEL);
}

#endif


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


void lib_hdmi21_dfe_power(unsigned char nport, unsigned char en, unsigned char lane_mode)
{
	if (nport >= HD21_PORT_TOTAL_NUM) {
		return;
	}

	hdmi_mask(HDMIRX_PHY_R2_reg, ~(b_9_DFE_ADAPTION_POW_EN|b_9_DFE_TAP_EN), (HD21_RX_PHY_P0_DFE_ADAPTION_EN(en)|HD21_RX_PHY_P0_b_tap_en(0xf)));
	hdmi_mask(HDMIRX_PHY_G2_reg, ~(b_9_DFE_ADAPTION_POW_EN|b_9_DFE_TAP_EN), (HD21_RX_PHY_P0_DFE_ADAPTION_EN(en)|HD21_RX_PHY_P0_b_tap_en(0xf)));
	hdmi_mask(HDMIRX_PHY_B2_reg, ~(b_9_DFE_ADAPTION_POW_EN|b_9_DFE_TAP_EN), (HD21_RX_PHY_P0_DFE_ADAPTION_EN(en)|HD21_RX_PHY_P0_b_tap_en(0xf)));

	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK2_reg, ~(b_9_DFE_ADAPTION_POW_EN|b_9_DFE_TAP_EN), (HD21_RX_PHY_P0_DFE_ADAPTION_EN(en)|HD21_RX_PHY_P0_b_tap_en(0xf)));

		
}

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
	if (en) {
		hdmi_mask(HDMIRX_PHY_B0_reg, ~b_2_RXVCM_SEL, b_2_RXVCM_10);
		hdmi_mask(HDMIRX_PHY_G0_reg, ~g_2_RXVCM_SEL, g_2_RXVCM_10);
		hdmi_mask(HDMIRX_PHY_R0_reg, ~r_2_RXVCM_SEL, r_2_RXVCM_10);
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~ck_2_RXVCM_SEL, ck_2_RXVCM_10);		
		
		hdmi_mask(HDMIRX_PHY_B1_reg, ~b_5_AC_COPULE_POW, b_5_AC_COPULE_POW);
		hdmi_mask(HDMIRX_PHY_G1_reg, ~g_5_AC_COPULE_POW, g_5_AC_COPULE_POW);
		hdmi_mask(HDMIRX_PHY_R1_reg, ~r_5_AC_COPULE_POW, r_5_AC_COPULE_POW);
		hdmi_mask(HDMIRX_PHY_CK1_reg, ~ck_5_AC_COPULE_POW, ck_5_AC_COPULE_POW);  //check power consumption for 3 lane frl mode
	} else {
		hdmi_mask(HDMIRX_PHY_B1_reg, ~b_5_AC_COPULE_POW, 0x0);
		hdmi_mask(HDMIRX_PHY_G1_reg, ~g_5_AC_COPULE_POW, 0x0);
		hdmi_mask(HDMIRX_PHY_R1_reg, ~r_5_AC_COPULE_POW, 0x0);
		hdmi_mask(HDMIRX_PHY_CK1_reg, ~ck_5_AC_COPULE_POW, ck_5_AC_COPULE_POW);   //for hysteresis amp always open

	}

}


void lib_hdmi21_eq_pi_power_en(unsigned char nport,unsigned char en, unsigned char frl_mode,unsigned char lane_mode)
{
	//add ac couple power
	if (en) {

		hdmi_mask(HDMIRX_PHY_B1_reg, ~(b_6_EQ_POW|b_6_RS_CAL_EN), b_6_EQ_POW|b_6_RS_CAL_EN);
		hdmi_mask(HDMIRX_PHY_G1_reg, ~(b_6_EQ_POW|b_6_RS_CAL_EN) , b_6_EQ_POW|b_6_RS_CAL_EN);
		hdmi_mask(HDMIRX_PHY_R1_reg, ~(b_6_EQ_POW|b_6_RS_CAL_EN), b_6_EQ_POW|b_6_RS_CAL_EN);

		
		if (lane_mode == HDMI_4LANE) {
			hdmi_mask(HDMIRX_PHY_CK1_reg, ~(b_6_EQ_POW|b_6_RS_CAL_EN), b_6_EQ_POW|b_6_RS_CAL_EN);	
		} else {
			hdmi_mask(HDMIRX_PHY_CK1_reg, ~(b_6_EQ_POW|b_6_RS_CAL_EN), 0x0);	
		}
		
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
		
	} else {
		// 4 lane all close
		hdmi_mask(HDMIRX_PHY_B1_reg, ~(b_8_POW_PR|b_6_EQ_POW|b_6_RS_CAL_EN), 0x0);
		hdmi_mask(HDMIRX_PHY_G1_reg, ~(b_8_POW_PR|b_6_EQ_POW|b_6_RS_CAL_EN), 0x0);
		hdmi_mask(HDMIRX_PHY_R1_reg, ~(b_8_POW_PR|b_6_EQ_POW|b_6_RS_CAL_EN), 0x0);
		hdmi_mask(HDMIRX_PHY_CK1_reg, ~(b_8_POW_PR|b_6_EQ_POW|b_6_RS_CAL_EN), 0x0);
	}
	
}
#endif

void lib_hdmi21_eq_bias_band_setting(unsigned char nport, unsigned int b,unsigned char frl_mode,unsigned char lane_mode)
{

	HDMI21_EQ_PARAM_T *p_eq_table;
	unsigned char tb_size = 0; 
	unsigned char mode = 0;
	unsigned char b_mode_found = 0;

	if (nport >= HD21_PORT_TOTAL_NUM)
		return;
		
	lib_hdmi21_eq_set_clock_boundary(nport);
		
	if (frl_mode == MODE_TMDS) {
		p_eq_table = hdmi21_eq_param;
		tb_size =  sizeof(hdmi21_eq_param)/sizeof(HDMI21_EQ_PARAM_T);
		for(mode = 0; mode < tb_size; mode++) {
			if ((p_eq_table[mode].b_upper > b) && (p_eq_table[mode].b_lower <= b)) {
				p_eq_table =  &p_eq_table[mode]; 
				b_mode_found = 1;
				break;
			}
		}
#if HD21_CLK_OVER_300M
		if (b_mode_found) {
			if ((b > clock_bound_3g) && !newbase_hdmi2p0_get_clock40x_flag(nport)) {	
				p_eq_table =  &hdmi21_eq_param[1];  //change to 3G mode
				if ((p_eq_table->b_lower != 1422) || (p_eq_table->b_upper != 2844)) {
					HDMI_EMG("ERROR not 3G mode\n");
					return ;
				}
			}
		}
#endif
	} else {  //frl mode
	if (newbase_hdmi_get_longcable_flag(nport))
	{
		HDMI_EMG("hdmi21_eq_frl_paramNew\n");

		p_eq_table = hdmi21_eq_frl_paramNew;
	}
	else
	{
		HDMI_EMG("hdmi21_eq_frl_param\n");
		p_eq_table = hdmi21_eq_frl_param;
	}	
		tb_size =  sizeof(hdmi21_eq_frl_param)/sizeof(HDMI21_EQ_PARAM_T);
		for(mode = 0; mode < tb_size; mode++) {
			if ((p_eq_table[mode].b_upper == b) && (p_eq_table[mode].b_lower == b)) {
					p_eq_table =  &p_eq_table[mode]; 
			              b_mode_found = 1;
			              break;
			}
		}
	}

	if (b_mode_found) {
	//		hdmi_mask(HDMIRX_2P1_PHY_TOP_IN_0_reg,(TOP_IN_2_IB_LEQ_ADJ|TOP_IN_2_IPTAT_LEQ_ADJ),(p_eq_table->IB_LEQ_ADJ<< 11)|(p_eq_table->IPTAT_LEQ_ADJ<< 8));
		
			hdmi_mask(HDMIRX_PHY_B0_reg,~(b_2_LE_PTAT_ISEL|b_2_RLSEL_LEQ1_NC2|b_1_DFE_BLEED),(p_eq_table->LE_PTAT_ISEL<< 13)|(p_eq_table->RLSEL_LEQ1_NC2 << 8)|(p_eq_table->DFE_Bleed << 3));
			hdmi_mask(HDMIRX_PHY_G0_reg,~(b_2_LE_PTAT_ISEL|b_2_RLSEL_LEQ1_NC2|b_1_DFE_BLEED),(p_eq_table->LE_PTAT_ISEL<< 13)|(p_eq_table->RLSEL_LEQ1_NC2 << 8)|(p_eq_table->DFE_Bleed << 3));
			hdmi_mask(HDMIRX_PHY_R0_reg,~(b_2_LE_PTAT_ISEL|b_2_RLSEL_LEQ1_NC2|b_1_DFE_BLEED),(p_eq_table->LE_PTAT_ISEL<< 13)|(p_eq_table->RLSEL_LEQ1_NC2 << 8)|(p_eq_table->DFE_Bleed << 3));
			if (lane_mode == HDMI_4LANE)
				hdmi_mask(HDMIRX_PHY_CK0_reg,~(b_2_LE_PTAT_ISEL|b_2_RLSEL_LEQ1_NC2|b_1_DFE_BLEED),(p_eq_table->LE_PTAT_ISEL<< 13)|(p_eq_table->RLSEL_LEQ1_NC2 << 8)|(p_eq_table->DFE_Bleed << 3));

		
			hdmi_mask(HDMIRX_PHY_B1_reg,~(b_7_TAP0_ISEL|b_7_LEQ_ISEL|b_6_RLSEL_LEQ|b_6_RLSEL_TAP0|b_6_RSSEL_TAP0|b_5_LEQ_CURRENT_ADJ),(p_eq_table->TAP0_ISEL << 21)| (p_eq_table->LEQ_ISEL << 18)|(p_eq_table->RLSEL_LEQ << 12) | (p_eq_table->RLSEL_TAP0 << 11) | (p_eq_table->RSSEL_TAP0 << 10)|(p_eq_table->LEQ_CURRETN_ADJ<< 3));
			hdmi_mask(HDMIRX_PHY_G1_reg,~(b_7_TAP0_ISEL|b_7_LEQ_ISEL|b_6_RLSEL_LEQ|b_6_RLSEL_TAP0|b_6_RSSEL_TAP0|b_5_LEQ_CURRENT_ADJ), (p_eq_table->TAP0_ISEL << 21)|(p_eq_table->LEQ_ISEL << 18)|(p_eq_table->RLSEL_LEQ << 12) | (p_eq_table->RLSEL_TAP0 << 11) | (p_eq_table->RSSEL_TAP0 << 10)|(p_eq_table->LEQ_CURRETN_ADJ<< 3));
			hdmi_mask(HDMIRX_PHY_R1_reg,~(b_7_TAP0_ISEL|b_7_LEQ_ISEL|b_6_RLSEL_LEQ|b_6_RLSEL_TAP0|b_6_RSSEL_TAP0|b_5_LEQ_CURRENT_ADJ), (p_eq_table->TAP0_ISEL << 21)|(p_eq_table->LEQ_ISEL << 18)|(p_eq_table->RLSEL_LEQ << 12) | (p_eq_table->RLSEL_TAP0 << 11) | (p_eq_table->RSSEL_TAP0 << 10)|(p_eq_table->LEQ_CURRETN_ADJ<< 3));
			if (lane_mode == HDMI_4LANE)
				hdmi_mask(HDMIRX_PHY_CK1_reg,~(b_7_TAP0_ISEL|b_7_LEQ_ISEL|b_6_RLSEL_LEQ|b_6_RLSEL_TAP0|b_6_RSSEL_TAP0|b_5_LEQ_CURRENT_ADJ), (p_eq_table->TAP0_ISEL << 21)|(p_eq_table->LEQ_ISEL << 18)|(p_eq_table->RLSEL_LEQ << 12) | (p_eq_table->RLSEL_TAP0 << 11) | (p_eq_table->RSSEL_TAP0 << 10)|(p_eq_table->LEQ_CURRETN_ADJ<< 3));


			if(( b>2700)&&(factory_or_power_only_mode_en==1) )//// for LGE-110HAM Protobal Genearator 4K30 timing (TID=3)
			{
				int eq_calib = factory_or_power_only_mode_hd21_eq_ofst;
				HDMI_EMG("Apply setting for  LGE-110HAM Protobal Genearator (EQ offset = %d)\n", eq_calib);
				hdmi_mask(HDMIRX_PHY_B2_reg,~(b_11_RSSEL_LEQ1_2|b_11_RSSEL_LEQ1_1),((p_eq_table->RSSEL_LEQ1_2+eq_calib)<< 20)|((p_eq_table->RSSEL_LEQ1_1+eq_calib)<< 16));
				hdmi_mask(HDMIRX_PHY_G2_reg,~(b_11_RSSEL_LEQ1_2|b_11_RSSEL_LEQ1_1),((p_eq_table->RSSEL_LEQ1_2+eq_calib)<< 20)|((p_eq_table->RSSEL_LEQ1_1+eq_calib)<< 16));
				hdmi_mask(HDMIRX_PHY_R2_reg,~(b_11_RSSEL_LEQ1_2|b_11_RSSEL_LEQ1_1),((p_eq_table->RSSEL_LEQ1_2+eq_calib)<< 20)|((p_eq_table->RSSEL_LEQ1_1+eq_calib)<< 16));
				if (lane_mode == HDMI_4LANE)
					hdmi_mask(HDMIRX_PHY_CK2_reg,~(b_11_RSSEL_LEQ1_2|b_11_RSSEL_LEQ1_1),((p_eq_table->RSSEL_LEQ1_2+eq_calib)<< 20)|((p_eq_table->RSSEL_LEQ1_1+eq_calib)<< 16));
			}
			else
			{
				hdmi_mask(HDMIRX_PHY_B2_reg,~(b_11_RSSEL_LEQ1_2|b_11_RSSEL_LEQ1_1),(p_eq_table->RSSEL_LEQ1_2<< 20)|(p_eq_table->RSSEL_LEQ1_1<< 16));
				hdmi_mask(HDMIRX_PHY_G2_reg,~(b_11_RSSEL_LEQ1_2|b_11_RSSEL_LEQ1_1),(p_eq_table->RSSEL_LEQ1_2<< 20)|(p_eq_table->RSSEL_LEQ1_1<< 16));
				hdmi_mask(HDMIRX_PHY_R2_reg,~(b_11_RSSEL_LEQ1_2|b_11_RSSEL_LEQ1_1),(p_eq_table->RSSEL_LEQ1_2<< 20)|(p_eq_table->RSSEL_LEQ1_1<< 16));
				if (lane_mode == HDMI_4LANE)
					hdmi_mask(HDMIRX_PHY_CK2_reg,~(b_11_RSSEL_LEQ1_2|b_11_RSSEL_LEQ1_1),(p_eq_table->RSSEL_LEQ1_2<< 20)|(p_eq_table->RSSEL_LEQ1_1<< 16));
			}


			hdmi_mask(HDMIRX_PHY_B3_reg,~(b_16_RSSEL_LEQ2|b_14_RLSEL_LEQ1_NC1), ((p_eq_table->RSSEL_LEQ2 & 0x7)<< 29)|((p_eq_table->RSSEL_LEQ2 & 0x8) << 20)|(p_eq_table->RLSEL_LEQ1_NC1 << 13));
			hdmi_mask(HDMIRX_PHY_G3_reg,~(b_16_RSSEL_LEQ2|b_14_RLSEL_LEQ1_NC1), ((p_eq_table->RSSEL_LEQ2 & 0x7)<< 29)|((p_eq_table->RSSEL_LEQ2 & 0x8) << 20)|(p_eq_table->RLSEL_LEQ1_NC1 << 13));
			hdmi_mask(HDMIRX_PHY_R3_reg,~(b_16_RSSEL_LEQ2|b_14_RLSEL_LEQ1_NC1), ((p_eq_table->RSSEL_LEQ2 & 0x7)<< 29)|((p_eq_table->RSSEL_LEQ2 & 0x8) << 20)|(p_eq_table->RLSEL_LEQ1_NC1 << 13));
			if (lane_mode == HDMI_4LANE)
				hdmi_mask(HDMIRX_PHY_CK3_reg,~(b_16_RSSEL_LEQ2|b_14_RLSEL_LEQ1_NC1), ((p_eq_table->RSSEL_LEQ2 & 0x7)<< 29)|((p_eq_table->RSSEL_LEQ2 & 0x8) << 20)|(p_eq_table->RLSEL_LEQ1_NC1 << 13));

			hdmi_mask(HDMIRX_PHY_B4_reg,~(b_17_POW_PTAT_STAGE1|b_17_POW_PTAT), (p_eq_table->POW_PTAT_STAGE1<< 7)|(p_eq_table->POW_PTAT<< 6));
			hdmi_mask(HDMIRX_PHY_G4_reg,~(b_17_POW_PTAT_STAGE1|b_17_POW_PTAT), (p_eq_table->POW_PTAT_STAGE1<< 7)|(p_eq_table->POW_PTAT<< 6));
			hdmi_mask(HDMIRX_PHY_R4_reg,~(b_17_POW_PTAT_STAGE1|b_17_POW_PTAT), (p_eq_table->POW_PTAT_STAGE1<< 7)|(p_eq_table->POW_PTAT<< 6));
			if (lane_mode == HDMI_4LANE)
				hdmi_mask(HDMIRX_PHY_CK4_reg,~(b_17_POW_PTAT_STAGE1|b_17_POW_PTAT), (p_eq_table->POW_PTAT_STAGE1<< 7)|(p_eq_table->POW_PTAT<< 6));
		
	}else {

		HDMI_EMG("not find EQ table\n");
	}

}


unsigned char lib_hdmi21_get_phy_table(unsigned char nport,unsigned int b,HDMI21_PHY_TMDS_PARAM_T **p_phy_table,HDMI21_PHY_FRL_PARAM_T **p_frl_phy_table, unsigned char frl_mode)
{
	unsigned char tb_size = 0;
		
	if (frl_mode == MODE_TMDS) {
		*p_phy_table = hdmi21_phy_tmds_param;
		tb_size = sizeof(hdmi21_phy_tmds_param)/sizeof(HDMI21_PHY_TMDS_PARAM_T);

		#if (HD21_CLK_OVER_300M)
		if (b > 2844 && !newbase_hdmi2p0_get_clock40x_flag(nport)) {
			*p_phy_table = hdmi21_phy_over3G_tmds_param;
			tb_size = sizeof(hdmi21_phy_over3G_tmds_param)/sizeof(HDMI21_PHY_TMDS_PARAM_T);
			HDMI_EMG("WTF..CLK_OVER_300M\n");
		}
		#endif
		
	} else {
		*p_frl_phy_table = hdmi21_phy_frl_param;
		tb_size = sizeof(hdmi21_phy_frl_param)/sizeof(HDMI21_PHY_FRL_PARAM_T);
	}

	return tb_size;
}


void lib_hdmi21_dfe_init(unsigned char nport)
{
	hdmi_mask(HDMIRX_PHY_B4_reg,~(VTH_MANUAL|KOFF_VTH_MANUAL_3|KOFF_VTH_MANUAL_2|KOFF_VTH_MANUAL_1|KOFF_VTH_MANUAL_0),(KOFF_VTH_MANUAL_3|KOFF_VTH_MANUAL_2|KOFF_VTH_MANUAL_1|KOFF_VTH_MANUAL_0));
	hdmi_mask(HDMIRX_PHY_G4_reg,~(VTH_MANUAL|KOFF_VTH_MANUAL_3|KOFF_VTH_MANUAL_2|KOFF_VTH_MANUAL_1|KOFF_VTH_MANUAL_0),(KOFF_VTH_MANUAL_3|KOFF_VTH_MANUAL_2|KOFF_VTH_MANUAL_1|KOFF_VTH_MANUAL_0));
	hdmi_mask(HDMIRX_PHY_R4_reg,~(VTH_MANUAL|KOFF_VTH_MANUAL_3|KOFF_VTH_MANUAL_2|KOFF_VTH_MANUAL_1|KOFF_VTH_MANUAL_0),(KOFF_VTH_MANUAL_3|KOFF_VTH_MANUAL_2|KOFF_VTH_MANUAL_1|KOFF_VTH_MANUAL_0));
	hdmi_mask(HDMIRX_PHY_CK4_reg,~(VTH_MANUAL|KOFF_VTH_MANUAL_3|KOFF_VTH_MANUAL_2|KOFF_VTH_MANUAL_1|KOFF_VTH_MANUAL_0),(KOFF_VTH_MANUAL_3|KOFF_VTH_MANUAL_2|KOFF_VTH_MANUAL_1|KOFF_VTH_MANUAL_0));

	hdmi_mask(HDMIRX_PHY_B2_reg,~(b_10_DFE_SUPAMP_DCGAIN_MAX|b_10_DFE_SUMAMP_ISEL),ck_10_DFE_SUPAMP_DCGAIN_MAX|b_10_DFE_SUMAMP_ISEL_101);
	hdmi_mask(HDMIRX_PHY_G2_reg,~(g_10_DFE_SUPAMP_DCGAIN_MAX|g_10_DFE_SUMAMP_ISEL),g_10_DFE_SUPAMP_DCGAIN_MAX|g_10_DFE_SUMAMP_ISEL_101);
	hdmi_mask(HDMIRX_PHY_R2_reg,~(r_10_DFE_SUPAMP_DCGAIN_MAX|r_10_DFE_SUMAMP_ISEL),r_10_DFE_SUPAMP_DCGAIN_MAX|r_10_DFE_SUMAMP_ISEL_101);
	hdmi_mask(HDMIRX_PHY_CK2_reg,~(ck_10_DFE_SUPAMP_DCGAIN_MAX|ck_10_DFE_SUMAMP_ISEL),ck_10_DFE_SUPAMP_DCGAIN_MAX|ck_10_DFE_SUMAMP_ISEL_101);
}


void lib_hdmi21_bias_init(unsigned char nport)
{

	hdmi_mask(HDMIRX_PHY_B0_reg,~(b_3_PTAT_CURRENT_ADJ),b_3_PTAT_CURRENT_ADJ);
	hdmi_mask(HDMIRX_PHY_G0_reg,~(g_3_PTAT_CURRENT_ADJ),g_3_PTAT_CURRENT_ADJ);
	hdmi_mask(HDMIRX_PHY_R0_reg,~(r_3_PTAT_CURRENT_ADJ),r_3_PTAT_CURRENT_ADJ);	
	hdmi_mask(HDMIRX_PHY_CK0_reg,~(ck_3_PTAT_CURRENT_ADJ),ck_3_PTAT_CURRENT_ADJ);

	hdmi_mask(HDMIRX_PHY_B1_reg,~(b_5_TAP0_CURRENT_ADJ),b_5_TAP0_CURRENT_ADJ);
	hdmi_mask(HDMIRX_PHY_G1_reg,~(g_5_TAP0_CURRENT_ADJ),g_5_TAP0_CURRENT_ADJ);
	hdmi_mask(HDMIRX_PHY_R1_reg,~(r_5_TAP0_CURRENT_ADJ),r_5_TAP0_CURRENT_ADJ);	
	hdmi_mask(HDMIRX_PHY_CK1_reg,~(ck_5_TAP0_CURRENT_ADJ),ck_5_TAP0_CURRENT_ADJ);

	hdmi_mask(HDMIRX_PHY_B2_reg,~(b_12_PI_CURRENT_ADJ),0x0);
	hdmi_mask(HDMIRX_PHY_G2_reg,~(g_12_PI_CURRENT_ADJ),0x0);
	hdmi_mask(HDMIRX_PHY_R2_reg,~(r_12_PI_CURRENT_ADJ),0x0);
	hdmi_mask(HDMIRX_PHY_CK2_reg,~(ck_12_PI_CURRENT_ADJ),0x0);	
}


void lib_hdmi21_set_dfe(unsigned char nport, unsigned int b, unsigned char dfe_mode, unsigned char lane_mode)
{
	HDMI_PRINTF("%s\n", __func__);

	// 1.1G~6G LE/TAP0 init setting
	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(DFE_P0_REG_DFE_EN_L0_P0_le_init_lane0_p0_mask), (DFE_P0_REG_DFE_EN_L0_P0_le_init_lane0_p0(0xf)));
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(DFE_P0_REG_DFE_EN_L1_P0_le_init_lane1_p0_mask), (DFE_P0_REG_DFE_EN_L1_P0_le_init_lane1_p0(0xf)));
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0_mask), (DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0(0xf)));
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(DFE_P0_REG_DFE_EN_L3_P0_le_init_lane3_p0_mask), (DFE_P0_REG_DFE_EN_L3_P0_le_init_lane3_p0(0xf)));

	hdmi_mask(DFE_REG_DFE_INIT0_L0_reg,~DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask,DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(0x14));	//lane0 tap0 initial	//0xc
	hdmi_mask(DFE_REG_DFE_INIT0_L1_reg,~DFE_P0_REG_DFE_INIT0_L1_P0_tap0_init_lane1_p0_mask,DFE_P0_REG_DFE_INIT0_L1_P0_tap0_init_lane1_p0(0x14));	//lane1 tap0 initial	//0xc
	hdmi_mask(DFE_REG_DFE_INIT0_L2_reg,~DFE_P0_REG_DFE_INIT0_L2_P0_tap0_init_lane2_p0_mask,DFE_P0_REG_DFE_INIT0_L2_P0_tap0_init_lane2_p0(0x14));	//lane2 tap0 initial	//0xc
	if (lane_mode == HDMI_4LANE)
			hdmi_mask(DFE_REG_DFE_INIT0_L3_reg,~DFE_P0_REG_DFE_INIT0_L3_P0_tap0_init_lane3_p0_mask,DFE_P0_REG_DFE_INIT0_L3_P0_tap0_init_lane3_p0(0x14));	//lane3 tap0 initial	//0xc

	// load initial data
	hdmi_mask(DFE_REG_DFE_INIT1_L0_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask);
	hdmi_mask(DFE_REG_DFE_INIT1_L1_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask);
	hdmi_mask(DFE_REG_DFE_INIT1_L2_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_INIT1_L3_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask);

	udelay(1);

	hdmi_mask(DFE_REG_DFE_INIT1_L0_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),0);
	hdmi_mask(DFE_REG_DFE_INIT1_L1_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),0);
	hdmi_mask(DFE_REG_DFE_INIT1_L2_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_INIT1_L3_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),0);

	lib_hdmi21_dump_dfe_tap0_le(nport,lane_mode);

	// TAP0/LE adaptation & max/min record & Tap0_max-2 flow
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

	lib_hdmi21_fg_koffset_proc(nport, lane_mode); //do koffset after tap0 finish

	if (b > 2700) {  // 3G need tap2 but below not need
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

	newbase_hdmi21_set_koffset_flag(nport,1);

}

void lib_hdmi21_set_dfe_mid_band(unsigned char nport, unsigned int b_clk, int dfe_mode, unsigned char lane_mode)
{
	HDMI_PRINTF("%s\n", __func__);
	
	// 430M~1.1G LE/TAP0 init setting
	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(DFE_P0_REG_DFE_EN_L0_P0_le_init_lane0_p0_mask), (DFE_P0_REG_DFE_EN_L0_P0_le_init_lane0_p0(0x5)));
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(DFE_P0_REG_DFE_EN_L1_P0_le_init_lane1_p0_mask), (DFE_P0_REG_DFE_EN_L1_P0_le_init_lane1_p0(0x5)));
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0_mask), (DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0(0x5)));
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(DFE_P0_REG_DFE_EN_L3_P0_le_init_lane3_p0_mask), (DFE_P0_REG_DFE_EN_L3_P0_le_init_lane3_p0(0x5)));

	hdmi_mask(DFE_REG_DFE_INIT0_L0_reg,~DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask,DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(0x14));	//lane0 tap0 initial	//0xc
	hdmi_mask(DFE_REG_DFE_INIT0_L1_reg,~DFE_P0_REG_DFE_INIT0_L1_P0_tap0_init_lane1_p0_mask,DFE_P0_REG_DFE_INIT0_L1_P0_tap0_init_lane1_p0(0x14));	//lane1 tap0 initial	//0xc
	hdmi_mask(DFE_REG_DFE_INIT0_L2_reg,~DFE_P0_REG_DFE_INIT0_L2_P0_tap0_init_lane2_p0_mask,DFE_P0_REG_DFE_INIT0_L2_P0_tap0_init_lane2_p0(0x14));	//lane2 tap0 initial	//0xc
	if (lane_mode == HDMI_4LANE)
			hdmi_mask(DFE_REG_DFE_INIT0_L3_reg,~DFE_P0_REG_DFE_INIT0_L3_P0_tap0_init_lane3_p0_mask,DFE_P0_REG_DFE_INIT0_L3_P0_tap0_init_lane3_p0(0x14));	//lane3 tap0 initial	//0xc

	// load initial data
	hdmi_mask(DFE_REG_DFE_INIT1_L0_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask);
	hdmi_mask(DFE_REG_DFE_INIT1_L1_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask);
	hdmi_mask(DFE_REG_DFE_INIT1_L2_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_INIT1_L3_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask);

	udelay(1);

	hdmi_mask(DFE_REG_DFE_INIT1_L0_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),0);
	hdmi_mask(DFE_REG_DFE_INIT1_L1_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),0);
	hdmi_mask(DFE_REG_DFE_INIT1_L2_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_INIT1_L3_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),0);

	lib_hdmi21_dump_dfe_tap0_le(nport,lane_mode);

	//750M TAP0/TAP1

	// ENABLE  VTH/TAP0
	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth),(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth));
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth),(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth));
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth),(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth));
	if (lane_mode == HDMI_4LANE)
			hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_Vth),(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_Vth));
	udelay(1);
	lib_hdmi_dfe_record_enable(nport, 1);
	udelay(500);
	lib_hdmi_dfe_record_enable(nport, 0);
	// DISABLE VTH/TAP0
	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_Vth),0x0);
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_Vth),0x0);
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_Vth),0x0);
	if (lane_mode == HDMI_4LANE)
			hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_Vth),0x0);

	lib_hdmi_load_tap0_max(nport,lane_mode);
	lib_hdmi21_fg_koffset_proc(nport, lane_mode); //do koffset after tap0 finish	

	// ENABLE  TAP0~TAP1
	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_TAP1),(REG_dfe_adapt_en_lane0_TAP0|REG_dfe_adapt_en_lane0_TAP1));
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_TAP1),(REG_dfe_adapt_en_lane1_TAP0|REG_dfe_adapt_en_lane1_TAP1));
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_TAP1),(REG_dfe_adapt_en_lane2_TAP0|REG_dfe_adapt_en_lane2_TAP1));
	if (lane_mode == HDMI_4LANE)
			hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_TAP1),(REG_dfe_adapt_en_lane3_TAP0|REG_dfe_adapt_en_lane3_TAP1));
	
	newbase_hdmi21_set_koffset_flag(nport,1);
}

void lib_hdmi21_dfe_manual_set(unsigned char nport, unsigned char lane_mode)
{
	HDMI_PRINTF("%s\n", __func__);

	//disable dfe
	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~DFE_P0_REG_DFE_EN_L0_P0_dfe_adapt_en_lane0_p0_mask,0);
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~DFE_P0_REG_DFE_EN_L1_P0_dfe_adapt_en_lane1_p0_mask,0);
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~DFE_P0_REG_DFE_EN_L2_P0_dfe_adapt_en_lane2_p0_mask,0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_EN_L3_reg,~DFE_P0_REG_DFE_EN_L3_P0_dfe_adapt_en_lane3_p0_mask,0);


	hdmi_mask(DFE_REG_DFE_MODE_reg,~DFE_P0_REG_DFE_MODE_P0_adapt_mode_p0_mask,DFE_P0_REG_DFE_MODE_P0_adapt_mode_p0(2));	//adaptive mode sel
	
	hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(DFE_P0_REG_DFE_EN_L0_P0_le_init_lane0_p0_mask), (DFE_P0_REG_DFE_EN_L0_P0_le_init_lane0_p0(0x5)));
	hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(DFE_P0_REG_DFE_EN_L1_P0_le_init_lane1_p0_mask), (DFE_P0_REG_DFE_EN_L1_P0_le_init_lane1_p0(0x5)));
	hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0_mask), (DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0(0x5)));
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(DFE_P0_REG_DFE_EN_L3_P0_le_init_lane3_p0_mask), (DFE_P0_REG_DFE_EN_L3_P0_le_init_lane3_p0(0x5)));


	
	hdmi_mask(DFE_REG_DFE_INIT0_L0_reg,~DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask,DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(0));	//lane0 tap1 initial
	hdmi_mask(DFE_REG_DFE_INIT0_L1_reg,~DFE_P0_REG_DFE_INIT0_L1_P0_tap1_init_lane1_p0_mask,DFE_P0_REG_DFE_INIT0_L1_P0_tap1_init_lane1_p0(0));	//lane1 tap1 initial
	hdmi_mask(DFE_REG_DFE_INIT0_L2_reg,~DFE_P0_REG_DFE_INIT0_L2_P0_tap1_init_lane2_p0_mask,DFE_P0_REG_DFE_INIT0_L2_P0_tap1_init_lane2_p0(0));	//lane2 tap1 initial
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_INIT0_L3_reg,~DFE_P0_REG_DFE_INIT0_L3_P0_tap1_init_lane3_p0_mask,DFE_P0_REG_DFE_INIT0_L3_P0_tap1_init_lane3_p0(0));	//lane3 tap1 initial


	hdmi_mask(DFE_REG_DFE_INIT0_L0_reg,~DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask,DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(0x14));	//lane0 tap0 initial	//0xc
	hdmi_mask(DFE_REG_DFE_INIT0_L1_reg,~DFE_P0_REG_DFE_INIT0_L1_P0_tap0_init_lane1_p0_mask,DFE_P0_REG_DFE_INIT0_L1_P0_tap0_init_lane1_p0(0x14));	//lane1 tap0 initial	//0xc
	hdmi_mask(DFE_REG_DFE_INIT0_L2_reg,~DFE_P0_REG_DFE_INIT0_L2_P0_tap0_init_lane2_p0_mask,DFE_P0_REG_DFE_INIT0_L2_P0_tap0_init_lane2_p0(0x14));	//lane2 tap0 initial	//0xc
	if (lane_mode == HDMI_4LANE)
			hdmi_mask(DFE_REG_DFE_INIT0_L3_reg,~DFE_P0_REG_DFE_INIT0_L3_P0_tap0_init_lane3_p0_mask,DFE_P0_REG_DFE_INIT0_L3_P0_tap0_init_lane3_p0(0x14));	//lane3 tap0 initial	//0xc



	hdmi_mask(DFE_REG_DFE_INIT1_L0_reg,~DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask,DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask);		//lane0  initial load
	hdmi_mask(DFE_REG_DFE_INIT1_L1_reg,~DFE_P0_REG_DFE_INIT1_L1_P0_load_in_init_lane1_p0_mask,DFE_P0_REG_DFE_INIT1_L1_P0_load_in_init_lane1_p0_mask);		//lane1  initial load
	hdmi_mask(DFE_REG_DFE_INIT1_L2_reg,~DFE_P0_REG_DFE_INIT1_L2_P0_load_in_init_lane2_p0_mask,DFE_P0_REG_DFE_INIT1_L2_P0_load_in_init_lane2_p0_mask);		//lane2  initial load
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_INIT1_L3_reg,~DFE_P0_REG_DFE_INIT1_L3_P0_load_in_init_lane3_p0_mask,DFE_P0_REG_DFE_INIT1_L3_P0_load_in_init_lane3_p0_mask);		//lane3  initial load


	udelay(1);	//10000
	hdmi_mask(DFE_REG_DFE_INIT1_L0_reg,~DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask,0);		//lane0  initial load
	hdmi_mask(DFE_REG_DFE_INIT1_L1_reg,~DFE_P0_REG_DFE_INIT1_L1_P0_load_in_init_lane1_p0_mask,0);		//lane1  initial load
	hdmi_mask(DFE_REG_DFE_INIT1_L2_reg,~DFE_P0_REG_DFE_INIT1_L2_P0_load_in_init_lane2_p0_mask,0);		//lane2  initial load
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_INIT1_L3_reg,~DFE_P0_REG_DFE_INIT1_L3_P0_load_in_init_lane3_p0_mask,0);		//lane3  initial load

	newbase_hdmi21_set_koffset_flag(nport,1);
}



//cdr & dcdr mode ck lane & data lane enable
void lib_hdmi21_cmu_pll_en(unsigned char nport, unsigned char enable, unsigned char frl_mode)
{

	if (nport >= HD21_PORT_TOTAL_NUM)
		return;

	if (enable) {	
		if (frl_mode == MODE_TMDS)	
			hdmi_mask(HDMIRX_PHY_CK_CMU_reg,~(cmu_ck_1_CKIN_SEL),cmu_ck_1_CKIN_SEL);
		else		//frl mode chage to crystal clock
			hdmi_mask(HDMIRX_PHY_CK_CMU_reg,~(cmu_ck_1_CKIN_SEL),0x0);
	} else { //disable default tmds clock
		hdmi_mask(HDMIRX_PHY_CK_CMU_reg,~(cmu_ck_1_CKIN_SEL),cmu_ck_1_CKIN_SEL);
	}

	if (enable) {
		if (frl_mode == MODE_TMDS) {
			//only setting ck lane
			hdmi_mask(HDMIRX_PHY_ACDR_R_MANUAL_CONFIG_reg,~(acdr_r_manual_RDY_FROM|acdr_r_manual_CMU_SEL),0x0);
			hdmi_mask(HDMIRX_PHY_ACDR_G_MANUAL_CONFIG_reg,~(acdr_g_manual_RDY_FROM|acdr_g_manual_CMU_SEL),0x0);
			hdmi_mask(HDMIRX_PHY_ACDR_B_MANUAL_CONFIG_reg,~(acdr_b_manual_RDY_FROM|acdr_b_manual_CMU_SEL), 0x0);
			hdmi_mask(HDMIRX_PHY_ACDR_CK_MANUAL_CONFIG_reg,~(acdr_ck_manual_RDY_FROM|acdr_ck_manual_CMU_SEL), (acdr_ck_manual_RDY_FROM_REG));
			//r lane off
			hdmi_mask(HDMIRX_PHY_ACDR_R0_reg,~(acdr_r_2_PFD_RSTB|acdr_r_2_CK_ENABLE), 0x0);
			//g lane off
			hdmi_mask(HDMIRX_PHY_ACDR_G0_reg,~(acdr_g_2_PFD_RSTB|acdr_g_2_CK_ENABLE), 0x0);
			//b lane off
			hdmi_mask(HDMIRX_PHY_ACDR_B0_reg,~(acdr_b_2_PFD_RSTB|acdr_b_2_CK_ENABLE), 0x0);
			//ck lane on (pll enable)
			hdmi_mask(HDMIRX_PHY_ACDR_CK0_reg,~(acdr_ck_2_PFD_RSTB|acdr_ck_2_CK_ENABLE), (acdr_ck_2_PFD_RSTB|acdr_ck_2_CK_ENABLE));
			
		} else {
			
			hdmi_mask(HDMIRX_PHY_ACDR_R_MANUAL_CONFIG_reg,~(acdr_r_manual_RDY_FROM|acdr_r_manual_CMU_SEL), 0x0);
			hdmi_mask(HDMIRX_PHY_ACDR_G_MANUAL_CONFIG_reg,~(acdr_g_manual_RDY_FROM|acdr_g_manual_CMU_SEL), 0x0);
			hdmi_mask(HDMIRX_PHY_ACDR_B_MANUAL_CONFIG_reg,~(acdr_b_manual_RDY_FROM|acdr_b_manual_CMU_SEL), 0x0);
			hdmi_mask(HDMIRX_PHY_ACDR_CK_MANUAL_CONFIG_reg,~(acdr_ck_manual_RDY_FROM|acdr_ck_manual_CMU_SEL), 0x0);  //need to check 3lane or 4 lane mode

			//r lane on
			hdmi_mask(HDMIRX_PHY_ACDR_R0_reg,~(acdr_r_2_PFD_RSTB|acdr_r_2_CK_ENABLE), (acdr_r_2_PFD_RSTB|acdr_r_2_CK_ENABLE));
			//g lane on
			hdmi_mask(HDMIRX_PHY_ACDR_G0_reg,~(acdr_g_2_PFD_RSTB|acdr_g_2_CK_ENABLE), (acdr_g_2_PFD_RSTB|acdr_g_2_CK_ENABLE));
			//b lane on
			hdmi_mask(HDMIRX_PHY_ACDR_B0_reg,~(acdr_b_2_PFD_RSTB|acdr_b_2_CK_ENABLE), (acdr_b_2_PFD_RSTB|acdr_b_2_CK_ENABLE));
			//ck lane on
			hdmi_mask(HDMIRX_PHY_ACDR_CK0_reg,~(acdr_ck_2_PFD_RSTB|acdr_ck_2_CK_ENABLE), (acdr_ck_2_PFD_RSTB|acdr_ck_2_CK_ENABLE));
			
		}
	}
	else {
		
		//default from register
		hdmi_mask(HDMIRX_PHY_ACDR_R_MANUAL_CONFIG_reg,~(acdr_r_manual_RDY_FROM|acdr_r_manual_CMU_SEL), acdr_r_manual_RDY_FROM_REG);
		hdmi_mask(HDMIRX_PHY_ACDR_G_MANUAL_CONFIG_reg,~(acdr_g_manual_RDY_FROM|acdr_g_manual_CMU_SEL), acdr_g_manual_RDY_FROM_REG);
		hdmi_mask(HDMIRX_PHY_ACDR_B_MANUAL_CONFIG_reg,~(acdr_b_manual_RDY_FROM|acdr_b_manual_CMU_SEL), acdr_b_manual_RDY_FROM_REG);
		hdmi_mask(HDMIRX_PHY_ACDR_CK_MANUAL_CONFIG_reg,~(acdr_ck_manual_RDY_FROM|acdr_ck_manual_CMU_SEL), acdr_ck_manual_RDY_FROM_REG);

		//r lane off
		hdmi_mask(HDMIRX_PHY_ACDR_R0_reg,~(acdr_r_2_PFD_RSTB|acdr_r_2_CK_ENABLE), 0x0);
		//g lane off
		hdmi_mask(HDMIRX_PHY_ACDR_G0_reg,~(acdr_g_2_PFD_RSTB|acdr_g_2_CK_ENABLE), 0x0);
		//b lane off
		hdmi_mask(HDMIRX_PHY_ACDR_B0_reg,~(acdr_b_2_PFD_RSTB|acdr_b_2_CK_ENABLE), 0x0);
		//ck lane off
		hdmi_mask(HDMIRX_PHY_ACDR_CK0_reg,~(acdr_ck_2_PFD_RSTB|acdr_ck_2_CK_ENABLE), (0x0));

	}
	

}


void lib_hdmi21_demux_ck_vcopll_rst(unsigned char nport, unsigned char rst, unsigned char lane_mode)
{
// rst = 1: reset
// rst = 0: normal working operation

	if (nport >= HD21_PORT_TOTAL_NUM)
		return;


	if (rst) {
		lib_hdmi_dfe_rst(nport, 1);
		hdmi_mask(HDMIRX_PHY_Y1_RST_reg, ~(HDMIRX_2P1_PHY_P0_HD21_Y1_RST_p0_r_demux_rstb_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_RST_p0_g_demux_rstb_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_RST_p0_b_demux_rstb_mask), 0);
		if (lane_mode == HDMI_4LANE) 
			hdmi_mask(HDMIRX_PHY_Y1_RST_reg, ~HDMIRX_2P1_PHY_P0_HD21_Y1_RST_p0_ck_demux_rstb_mask, 0);
		
	} else {

		hdmi_mask(HDMIRX_PHY_Y1_RST_reg, ~(HDMIRX_2P1_PHY_P0_HD21_Y1_RST_p0_r_demux_rstb_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_RST_p0_g_demux_rstb_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_RST_p0_b_demux_rstb_mask), (HDMIRX_2P1_PHY_P0_HD21_Y1_RST_p0_r_demux_rstb_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_RST_p0_g_demux_rstb_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_RST_p0_b_demux_rstb_mask));
		
		if (lane_mode == HDMI_4LANE)
			hdmi_mask(HDMIRX_PHY_Y1_RST_reg, ~HDMIRX_2P1_PHY_P0_HD21_Y1_RST_p0_ck_demux_rstb_mask, HDMIRX_2P1_PHY_P0_HD21_Y1_RST_p0_ck_demux_rstb_mask);
		
		lib_hdmi_dfe_rst(nport, 0);
	}
}


void lib_hdmi21_8b18b_fifo_rst(unsigned char nport, unsigned char rst,unsigned char frl_mode,unsigned char lane_mode)
{
	const unsigned int fifo_mask = (HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_p0_r_fifo_rst_n_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_p0_g_fifo_rst_n_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_p0_b_fifo_rst_n_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_p0_ck_fifo_rst_n_mask);

	unsigned int fifo_rst;
	HDMI_PRINTF("%s\n", __func__);

	if (lane_mode == HDMI_4LANE)
		fifo_rst = fifo_mask;
	else
		fifo_rst = HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_p0_r_fifo_rst_n_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_p0_g_fifo_rst_n_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD21_p0_b_fifo_rst_n_mask;


        PWR_EMG("HDMI[p%d] HDMIRX_PHY_REGD21_reg (%08lx=%08x, b4=%d, b3=%d , b2=%d, b1=%d))\n",
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
}




void lib_hdmi21_cdr_rst(unsigned char nport, unsigned char rst, unsigned char frl_mode,unsigned char lane_mode)
{

// rst = 1: reset
// rst = 0: normal working operation


	const unsigned int dig_bit[HD21_PORT_TOTAL_NUM] = {
	(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_p0_r_dig_rst_n_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_p0_g_dig_rst_n_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_p0_b_dig_rst_n_mask),
	(HDMIRX_2P1_PHY_P1_HD21_Y1_REGD00_p1_r_dig_rst_n_mask|HDMIRX_2P1_PHY_P1_HD21_Y1_REGD00_p1_g_dig_rst_n_mask|HDMIRX_2P1_PHY_P1_HD21_Y1_REGD00_p1_b_dig_rst_n_mask)};
	
	const unsigned int cdr_bit[HD21_PORT_TOTAL_NUM] = {
	(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_p0_r_cdr_rst_n_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_p0_g_cdr_rst_n_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_p0_b_cdr_rst_n_mask),
	(HDMIRX_2P1_PHY_P1_HD21_Y1_REGD00_p1_r_cdr_rst_n_mask|HDMIRX_2P1_PHY_P1_HD21_Y1_REGD00_p1_g_cdr_rst_n_mask|HDMIRX_2P1_PHY_P1_HD21_Y1_REGD00_p1_b_cdr_rst_n_mask)};
	
	if (nport >= HD21_PORT_TOTAL_NUM)
		return;
	
	if (rst) {
		hdmi_mask(HDMIRX_PHY_REGD00_reg, ~dig_bit[nport], 0);
		hdmi_mask(HDMIRX_PHY_REGD00_reg, ~cdr_bit[nport], 0);
		hdmi_mask(HDMIRX_PHY_REGD03_reg,~(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD03_p0_ck_dig_rst_n_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD03_p0_ck_cdr_rst_n_mask),0);

	} else {

		if (frl_mode == MODE_TMDS) {  //only 3 lane
			hdmi_mask(HDMIRX_PHY_REGD00_reg, ~(dig_bit[nport]|cdr_bit[nport]), (dig_bit[nport]|cdr_bit[nport]));
		} else {
			//only enable dig rst
			hdmi_mask(HDMIRX_PHY_REGD00_reg, ~(dig_bit[nport]|cdr_bit[nport]), dig_bit[nport]);
			if (lane_mode == HDMI_4LANE) {
				hdmi_mask(HDMIRX_PHY_REGD03_reg,~(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD03_p0_ck_dig_rst_n_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD03_p0_ck_cdr_rst_n_mask),HDMIRX_2P1_PHY_P0_HD21_Y1_REGD03_p0_ck_dig_rst_n_mask);
			}
		}
	}
}

void lib_hdmi21_wdog(unsigned char nport,unsigned char frl_mode,unsigned char lane_mode)
{

	if (nport >= HD21_PORT_TOTAL_NUM)
		return;

	hdmi_mask(HDMIRX_PHY_REGD07_reg, ~(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_bypass_eqen_rdy_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_bypass_data_rdy_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_bypass_pi_mask|
		                                               HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_data_rdy_time_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_en_clkout_manual_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_en_eqen_manual_mask|
		                                                HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_en_data_manual_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_eqen_rdy_time_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_pll_wd_base_time_mask|
		                                                HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_pll_wd_rst_wid_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_pll_wd_time_rdy_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_p0_pll_wd_ckrdy_ro_mask|
		                                                HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_p0_wdog_rst_n_mask),
		                                                	HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_bypass_clk_rdy(1)| HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_bypass_eqen_rdy(1)|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_bypass_data_rdy(1)|
		                                                 HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_data_rdy_time(4)| HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_en_clkout_manual(1)|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_en_eqen_manual(1)|
									HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_en_data_manual(1)|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_eqen_rdy_time(4)|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_pll_wd_base_time(2)|
									HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_pll_wd_rst_wid(1)|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_pll_wd_time_rdy(1) |HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_p0_pll_wd_ckrdy_ro(1)|
									HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_p0_wdog_rst_n(1));


	hdmi_mask(HDMIRX_PHY_REGD08_reg, ~(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD08_reg_p0_wd_sdm_en_mask),0);
	udelay(5);
	hdmi_mask(HDMIRX_PHY_REGD08_reg, ~(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD08_reg_p0_wd_sdm_en_mask), HDMIRX_2P1_PHY_P0_HD21_Y1_REGD08_reg_p0_wd_sdm_en_mask);	//adams modify, 20160130	//Load CMU M code


	if (frl_mode == MODE_TMDS) {
		hdmi_mask(HDMIRX_PHY_REGD07_reg,~HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_acdr_l0_en_mask,0x0);
		hdmi_mask(HDMIRX_PHY_REGD09_reg,~(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD09_reg_p0_acdr_l3_en_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD09_reg_p0_acdr_l2_en_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD09_reg_p0_acdr_l1_en_mask),0x0);
	} else {
		hdmi_mask(HDMIRX_PHY_REGD07_reg,~(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_acdr_l0_en_mask),HDMIRX_2P1_PHY_P0_HD21_Y1_REGD07_reg_p0_acdr_l0_en_mask);
		hdmi_mask(HDMIRX_PHY_REGD09_reg,~(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD09_reg_p0_acdr_l2_en_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD09_reg_p0_acdr_l1_en_mask),(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD09_reg_p0_acdr_l2_en_mask|HDMIRX_2P1_PHY_P0_HD21_Y1_REGD09_reg_p0_acdr_l1_en_mask));
		if (lane_mode == HDMI_4LANE) {
			hdmi_mask(HDMIRX_PHY_REGD09_reg,~(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD09_reg_p0_acdr_l3_en_mask),HDMIRX_2P1_PHY_P0_HD21_Y1_REGD09_reg_p0_acdr_l3_en_mask);
		} else {
			hdmi_mask(HDMIRX_PHY_REGD09_reg,~(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD09_reg_p0_acdr_l3_en_mask),0x0);
		}
	}

}

void lib_hdmi21_dfe_config_setting(unsigned char nport,unsigned char lane_mode)
{
        lib_hdmi_mac_afifo_enable(nport,lane_mode);
}


void lib_hdmi21_set_fld_table(unsigned char nport,HDMI21_FLD_PARAM_T * fld_param)
{

	hdmi_mask(HDMIRX_PHY_FLD_R_REGD04_reg,~(HDMIRX_2P1_PHY_P0_FLD_R_REGD04_reg_p0_r_vco_coarse_init_mask|HDMIRX_2P1_PHY_P0_FLD_R_REGD04_reg_p0_r_vco_fine_init_mask),HDMIRX_2P1_PHY_P0_FLD_R_REGD04_reg_p0_r_vco_coarse_init(fld_param->vco_coarse_init)|HDMIRX_2P1_PHY_P0_FLD_R_REGD04_reg_p0_r_vco_fine_init(fld_param->vco_fine_init));
	hdmi_mask(HDMIRX_PHY_FLD_G_REGD04_reg,~(HDMIRX_2P1_PHY_P0_FLD_G_REGD04_reg_p0_g_vco_coarse_init_mask|HDMIRX_2P1_PHY_P0_FLD_G_REGD04_reg_p0_g_vco_fine_init_mask),HDMIRX_2P1_PHY_P0_FLD_G_REGD04_reg_p0_g_vco_coarse_init(fld_param->vco_coarse_init)|HDMIRX_2P1_PHY_P0_FLD_G_REGD04_reg_p0_g_vco_fine_init(fld_param->vco_fine_init));
	hdmi_mask(HDMIRX_PHY_FLD_B_REGD04_reg,~(HDMIRX_2P1_PHY_P0_FLD_B_REGD04_reg_p0_b_vco_coarse_init_mask|HDMIRX_2P1_PHY_P0_FLD_B_REGD04_reg_p0_b_vco_fine_init_mask),HDMIRX_2P1_PHY_P0_FLD_B_REGD04_reg_p0_b_vco_coarse_init(fld_param->vco_coarse_init)|HDMIRX_2P1_PHY_P0_FLD_B_REGD04_reg_p0_b_vco_fine_init(fld_param->vco_fine_init));
	hdmi_mask(HDMIRX_PHY_FLD_CK_REGD04_reg,~(HDMIRX_2P1_PHY_P0_FLD_CK_REGD04_reg_p0_ck_vco_coarse_init_mask|HDMIRX_2P1_PHY_P0_FLD_CK_REGD04_reg_p0_ck_vco_fine_init_mask),HDMIRX_2P1_PHY_P0_FLD_CK_REGD04_reg_p0_ck_vco_coarse_init(fld_param->vco_coarse_init)|HDMIRX_2P1_PHY_P0_FLD_CK_REGD04_reg_p0_ck_vco_fine_init(fld_param->vco_fine_init));

	hdmi_mask(HDMIRX_PHY_FLD_R_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_divide_num_mask),HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_divide_num(fld_param->divide_num));
	hdmi_mask(HDMIRX_PHY_FLD_G_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_G_REGD00_reg_p0_g_divide_num_mask),HDMIRX_2P1_PHY_P0_FLD_G_REGD00_reg_p0_g_divide_num(fld_param->divide_num));
	hdmi_mask(HDMIRX_PHY_FLD_B_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_B_REGD00_reg_p0_b_divide_num_mask),HDMIRX_2P1_PHY_P0_FLD_B_REGD00_reg_p0_b_divide_num(fld_param->divide_num));
	hdmi_mask(HDMIRX_PHY_FLD_CK_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_CK_REGD00_reg_p0_ck_divide_num_mask),HDMIRX_2P1_PHY_P0_FLD_CK_REGD00_reg_p0_ck_divide_num(fld_param->divide_num));

	hdmi_mask(HDMIRX_PHY_FLD_R_REGD01_reg,~(HDMIRX_2P1_PHY_P0_FLD_R_REGD01_reg_p0_r_lock_up_limit_mask|HDMIRX_2P1_PHY_P0_FLD_R_REGD01_reg_p0_r_lock_dn_limit_mask),HDMIRX_2P1_PHY_P0_FLD_R_REGD01_reg_p0_r_lock_up_limit(fld_param->lock_up_limit)|HDMIRX_2P1_PHY_P0_FLD_R_REGD01_reg_p0_r_lock_dn_limit(fld_param->lock_dn_limit));
	hdmi_mask(HDMIRX_PHY_FLD_G_REGD01_reg,~(HDMIRX_2P1_PHY_P0_FLD_G_REGD01_reg_p0_g_lock_up_limit_mask|HDMIRX_2P1_PHY_P0_FLD_G_REGD01_reg_p0_g_lock_dn_limit_mask),HDMIRX_2P1_PHY_P0_FLD_G_REGD01_reg_p0_g_lock_up_limit(fld_param->lock_up_limit)|HDMIRX_2P1_PHY_P0_FLD_G_REGD01_reg_p0_g_lock_dn_limit(fld_param->lock_dn_limit));
	hdmi_mask(HDMIRX_PHY_FLD_B_REGD01_reg,~(HDMIRX_2P1_PHY_P0_FLD_B_REGD01_reg_p0_b_lock_up_limit_mask|HDMIRX_2P1_PHY_P0_FLD_B_REGD01_reg_p0_b_lock_dn_limit_mask),HDMIRX_2P1_PHY_P0_FLD_B_REGD01_reg_p0_b_lock_up_limit(fld_param->lock_up_limit)|HDMIRX_2P1_PHY_P0_FLD_B_REGD01_reg_p0_b_lock_dn_limit(fld_param->lock_dn_limit));
	hdmi_mask(HDMIRX_PHY_FLD_CK_REGD01_reg,~(HDMIRX_2P1_PHY_P0_FLD_CK_REGD01_reg_p0_ck_lock_up_limit_mask|HDMIRX_2P1_PHY_P0_FLD_CK_REGD01_reg_p0_ck_lock_dn_limit_mask),HDMIRX_2P1_PHY_P0_FLD_CK_REGD01_reg_p0_ck_lock_up_limit(fld_param->lock_up_limit)|HDMIRX_2P1_PHY_P0_FLD_CK_REGD01_reg_p0_ck_lock_dn_limit(fld_param->lock_dn_limit));

	hdmi_mask(HDMIRX_PHY_FLD_R_REGD03_reg,~(HDMIRX_2P1_PHY_P0_FLD_R_REGD03_reg_p0_r_coarse_lock_up_limit_mask|HDMIRX_2P1_PHY_P0_FLD_R_REGD03_reg_p0_r_coarse_lock_dn_limit_mask),HDMIRX_2P1_PHY_P0_FLD_R_REGD03_reg_p0_r_coarse_lock_up_limit(fld_param->coarse_lock_up_limit)|HDMIRX_2P1_PHY_P0_FLD_R_REGD03_reg_p0_r_coarse_lock_dn_limit(fld_param->coarse_lock_dn_limit));
	hdmi_mask(HDMIRX_PHY_FLD_G_REGD03_reg,~(HDMIRX_2P1_PHY_P0_FLD_G_REGD03_reg_p0_g_coarse_lock_up_limit_mask|HDMIRX_2P1_PHY_P0_FLD_G_REGD03_reg_p0_g_coarse_lock_dn_limit_mask),HDMIRX_2P1_PHY_P0_FLD_R_REGD03_reg_p0_r_coarse_lock_up_limit(fld_param->coarse_lock_up_limit)|HDMIRX_2P1_PHY_P0_FLD_R_REGD03_reg_p0_r_coarse_lock_dn_limit(fld_param->coarse_lock_dn_limit));
	hdmi_mask(HDMIRX_PHY_FLD_B_REGD03_reg,~(HDMIRX_2P1_PHY_P0_FLD_B_REGD03_reg_p0_b_coarse_lock_up_limit_mask|HDMIRX_2P1_PHY_P0_FLD_B_REGD03_reg_p0_b_coarse_lock_dn_limit_mask),HDMIRX_2P1_PHY_P0_FLD_R_REGD03_reg_p0_r_coarse_lock_up_limit(fld_param->coarse_lock_up_limit)|HDMIRX_2P1_PHY_P0_FLD_R_REGD03_reg_p0_r_coarse_lock_dn_limit(fld_param->coarse_lock_dn_limit));
	hdmi_mask(HDMIRX_PHY_FLD_CK_REGD03_reg,~(HDMIRX_2P1_PHY_P0_FLD_CK_REGD03_reg_p0_ck_coarse_lock_up_limit_mask|HDMIRX_2P1_PHY_P0_FLD_CK_REGD03_reg_p0_ck_coarse_lock_dn_limit_mask),HDMIRX_2P1_PHY_P0_FLD_R_REGD03_reg_p0_r_coarse_lock_up_limit(fld_param->coarse_lock_up_limit)|HDMIRX_2P1_PHY_P0_FLD_R_REGD03_reg_p0_r_coarse_lock_dn_limit(fld_param->coarse_lock_dn_limit));
	
}



void lib_hdmi21_set_frl_phy_table(unsigned char nport, HDMI21_PHY_FRL_PARAM_T *phy_param)
{

	//enable ldo & cco for clock lane and disable for other lanes (put first setting)
	hdmi_mask(HDMIRX_PHY_ACDR_B0_reg,~(acdr_b_1_CCO_POW|acdr_b_1_LDO_EN),(acdr_b_1_CCO_POW|acdr_b_1_LDO_EN));
	hdmi_mask(HDMIRX_PHY_ACDR_G0_reg,~(acdr_g_1_CCO_POW|acdr_g_1_LDO_EN),(acdr_g_1_CCO_POW|acdr_g_1_LDO_EN));
	hdmi_mask(HDMIRX_PHY_ACDR_R0_reg,~(acdr_r_1_CCO_POW|acdr_r_1_LDO_EN),(acdr_r_1_CCO_POW|acdr_r_1_LDO_EN));
	hdmi_mask(HDMIRX_PHY_ACDR_CK0_reg,~(acdr_ck_1_CCO_POW|acdr_ck_1_LDO_EN),(acdr_ck_1_CCO_POW|acdr_ck_1_LDO_EN));


	//MANUAL SEL
	if (nport == HDMI_PORT0)
		hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg,
					~(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg_p0_r_pllcdr_manual_enable_mask|HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg_p0_g_pllcdr_manual_enable_mask|
					HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg_p0_b_pllcdr_manual_enable_mask|HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg_p0_ck_pllcdr_manual_enable_mask),
					(phy_param->MANUAL_SEL << 30 | phy_param->MANUAL_SEL << 29 |  phy_param->MANUAL_SEL << 28 |  phy_param->MANUAL_SEL << 27));
	else
		hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg,
					~(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg_p1_r_pllcdr_manual_enable_mask|HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg_p1_g_pllcdr_manual_enable_mask|
					HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg_p1_b_pllcdr_manual_enable_mask|HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg_p1_ck_pllcdr_manual_enable_mask),
					(phy_param->MANUAL_SEL << 26 | phy_param->MANUAL_SEL << 25 |  phy_param->MANUAL_SEL << 24 |  phy_param->MANUAL_SEL << 23));

	//M code
	hdmi_mask(HDMIRX_PHY_REGD08_reg, ~HDMIRX_2P1_PHY_P0_HD21_Y1_REGD08_reg_p0_n_code_mask, (phy_param->M_code)<<HDMIRX_2P1_PHY_P0_HD21_Y1_REGD08_reg_p0_n_code_shift);		

	//N code, data lane ck src
	hdmi_mask(HDMIRX_PHY_ACDR_B1_reg, ~(acdr_b_7_CMU_N_code|acdr_b_7_CMU_PREDIVN|acdr_b_6_CK_SRC_SEL|acdr_b_5_VC_INIT), (phy_param->N_code<<16)|(phy_param->DATA_LANE_CK_SRC << 14)|phy_param->VC_VOL_SEL);		//N code set
	hdmi_mask(HDMIRX_PHY_ACDR_G1_reg, ~(acdr_g_7_CMU_N_code|acdr_g_7_CMU_PREDIVN|acdr_g_6_CK_SRC_SEL|acdr_g_5_VC_INIT), (phy_param->N_code<<16)|(phy_param->DATA_LANE_CK_SRC << 14)|phy_param->VC_VOL_SEL);		//N code set
	hdmi_mask(HDMIRX_PHY_ACDR_R1_reg, ~(acdr_r_7_CMU_N_code|acdr_r_7_CMU_PREDIVN|acdr_r_6_CK_SRC_SEL|acdr_r_5_VC_INIT), (phy_param->N_code<<16)|(phy_param->DATA_LANE_CK_SRC << 14)|phy_param->VC_VOL_SEL);	;		//N code set
	hdmi_mask(HDMIRX_PHY_ACDR_CK1_reg, ~(acdr_ck_7_CMU_N_code|acdr_ck_7_CMU_PREDIVN|acdr_ck_6_CK_SRC_SEL|acdr_ck_5_VC_INIT), (phy_param->N_code<<16)|(phy_param->DATA_LANE_CK_SRC << 14)|phy_param->VC_VOL_SEL);		//N code set

	//bandsel , postM 
	hdmi_mask(HDMIRX_PHY_ACDR_B0_reg,~(acdr_b_4_CP_OP|acdr_b_4_CP_CAP|acdr_b_3_BAND_SEL|acdr_b_2_POST_M),((phy_param->CP_OP_EN<<25)|(phy_param->CP_CAP_EN<<24)|(phy_param->BAND_SEL<<22)|(phy_param->POST_M<<10)));
	hdmi_mask(HDMIRX_PHY_ACDR_G0_reg,~(acdr_g_4_CP_OP|acdr_g_4_CP_CAP|acdr_g_3_BAND_SEL|acdr_g_2_POST_M),((phy_param->CP_OP_EN<<25)|(phy_param->CP_CAP_EN<<24)|(phy_param->BAND_SEL<<22)|(phy_param->POST_M<<10)));
	hdmi_mask(HDMIRX_PHY_ACDR_R0_reg,~(acdr_r_4_CP_OP|acdr_r_4_CP_CAP|acdr_r_3_BAND_SEL|acdr_r_2_POST_M),((phy_param->CP_OP_EN<<25)|(phy_param->CP_CAP_EN<<24)|(phy_param->BAND_SEL<<22)|(phy_param->POST_M<<10)));
	hdmi_mask(HDMIRX_PHY_ACDR_CK0_reg,~(acdr_ck_4_CP_OP|acdr_ck_4_CP_CAP|acdr_ck_3_BAND_SEL|acdr_ck_2_POST_M),((phy_param->CP_OP_EN<<25)|(phy_param->CP_CAP_EN<<24)|(phy_param->BAND_SEL<<22)|(phy_param->POST_M<<10)));
		

	//DEMUX/ PI_DIV  (PI_ISEL set 0 useless for acdr)
	hdmi_mask(HDMIRX_PHY_B1_reg, ~(b_8_PR_RATE_SEL|b_8_DEMUX|b_7_PI_ISEL), (phy_param->DEMUX_RATE_SEL<<29)|(phy_param->PI_DIV<<26) |(((phy_param->PI_ISEL & 0x4)>>2 )<<24)| (phy_param->PI_ISEL<<16));
	hdmi_mask(HDMIRX_PHY_G1_reg, ~(g_8_PR_RATE_SEL|g_8_DEMUX|g_7_PI_ISEL), (phy_param->DEMUX_RATE_SEL<<29)|(phy_param->PI_DIV<<26)|(((phy_param->PI_ISEL & 0x4)>>2 )<<24)|(phy_param->PI_ISEL<<16));
	hdmi_mask(HDMIRX_PHY_R1_reg, ~(r_8_PR_RATE_SEL|r_8_DEMUX|r_7_PI_ISEL), (phy_param->DEMUX_RATE_SEL<<29)|(phy_param->PI_DIV<<26)|(((phy_param->PI_ISEL & 0x4)>>2 )<<24)|(phy_param->PI_ISEL<<16));
	hdmi_mask(HDMIRX_PHY_CK1_reg, ~(ck_8_PR_RATE_SEL|ck_8_DEMUX|ck_7_PI_ISEL), (phy_param->DEMUX_RATE_SEL<<29)|(phy_param->PI_DIV<<26)|(((phy_param->PI_ISEL & 0x4)>>2 )<<24)|(phy_param->PI_ISEL<<16));


	//FULL&HALF_RATE_SEL/PIX_RATE_SEL (PI_CSEL set 0  useless for acdr)
	hdmi_mask(HDMIRX_PHY_B2_reg, ~(b_12_FR_CK_SEL|b_12_PIX_RATE_SEL|b_12_PI_CSEL), (phy_param->FULL_RATE_SEL<<31)| (phy_param->PIX_RATE_SEL<<29)|(phy_param->PI_CSEL<<24));
	hdmi_mask(HDMIRX_PHY_G2_reg, ~(g_12_FR_CK_SEL|g_12_PIX_RATE_SEL|g_12_PI_CSEL), (phy_param->FULL_RATE_SEL<<31)| (phy_param->PIX_RATE_SEL<<29)|(phy_param->PI_CSEL<<24));
	hdmi_mask(HDMIRX_PHY_R2_reg, ~(r_12_FR_CK_SEL|r_12_PIX_RATE_SEL|r_12_PI_CSEL),   (phy_param->FULL_RATE_SEL<<31)|(phy_param->PIX_RATE_SEL<<29)|(phy_param->PI_CSEL<<24));
	hdmi_mask(HDMIRX_PHY_CK2_reg, ~(ck_12_FR_CK_SEL|ck_12_PIX_RATE_SEL|ck_12_PI_CSEL), (phy_param->FULL_RATE_SEL<<31)|(phy_param->PIX_RATE_SEL<<29)|(phy_param->PI_CSEL<<24));


	//icp double , pre-M
	hdmi_mask(HDMIRX_PHY_ACDR_B0_reg,~(acdr_b_2_PRE_M|acdr_b_1_HALF_RATE_SEL), (phy_param->PRE_M << 9) | (phy_param->HALF_RATE_SEL<<4));
	hdmi_mask(HDMIRX_PHY_ACDR_G0_reg,~(acdr_g_2_PRE_M|acdr_g_1_HALF_RATE_SEL), (phy_param->PRE_M << 9) | (phy_param->HALF_RATE_SEL<<4));
	hdmi_mask(HDMIRX_PHY_ACDR_R0_reg,~(acdr_r_2_PRE_M|acdr_r_1_HALF_RATE_SEL), (phy_param->PRE_M << 9) | (phy_param->HALF_RATE_SEL<<4));
	hdmi_mask(HDMIRX_PHY_ACDR_CK0_reg,~(acdr_ck_2_PRE_M|acdr_ck_1_HALF_RATE_SEL), (phy_param->PRE_M << 9) | (phy_param->HALF_RATE_SEL<<4));

	//DELAY_CEL LDO_VSEL  KVCO
	hdmi_mask(HDMIRX_PHY_ACDR_B1_reg,~(acdr_b_8_DELAY_CELL_SEL|acdr_b_6_LDO2V|acdr_b_5_KVCO_SEL), phy_param->DELAY_CEL_SEL<<24|HD21_RX_PHY_P0_VSEL_LDO(0) | phy_param->CMU_KVCO<<5);
	hdmi_mask(HDMIRX_PHY_ACDR_G1_reg,~(acdr_g_8_DELAY_CELL_SEL|acdr_g_6_LDO2V|acdr_g_5_KVCO_SEL), phy_param->DELAY_CEL_SEL<<24|HD21_RX_PHY_P0_VSEL_LDO(0) | phy_param->CMU_KVCO<<5);
	hdmi_mask(HDMIRX_PHY_ACDR_R1_reg,~(acdr_r_8_DELAY_CELL_SEL|acdr_r_6_LDO2V|acdr_r_5_KVCO_SEL), phy_param->DELAY_CEL_SEL<<24|HD21_RX_PHY_P0_VSEL_LDO(0) | phy_param->CMU_KVCO<<5);
	hdmi_mask(HDMIRX_PHY_ACDR_CK1_reg,~(acdr_ck_8_DELAY_CELL_SEL|acdr_ck_6_LDO2V|acdr_ck_5_KVCO_SEL), phy_param->DELAY_CEL_SEL<<24|HD21_RX_PHY_P0_VSEL_LDO(0) | phy_param->CMU_KVCO<<5);

	//ENCAP
	hdmi_mask(HDMIRX_PHY_MISC_0_reg,
				~(HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_r_vco_encap_mask|HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_g_vco_encap_mask|
				HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_b_vco_encap_mask|HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_ck_vco_encap_mask),
				(phy_param->EN_CAP<<30|phy_param->EN_CAP<<22| phy_param->EN_CAP<<14|phy_param->EN_CAP<<6));


#if 0
//icp for merlin5
	hdmi_mask(HDMIRX_PHY_FLD_B_REGD02_reg,~(ICP_PLL_MODE_mask|ICP_ACDR_MODE_STEP1_mask|ICP_ACDR_MODE_STEP2_mask|ICP_ACDR_MODE_STEP3_mask), 
				(phy_param->ICP_PLL_MODE<<28)|(phy_param->ICP_ACDR_MODE<<24)|(phy_param->ICP_ACDR_MODE<<20)|(phy_param->ICP_ACDR_MODE<<16));
	hdmi_mask(HDMIRX_PHY_FLD_G_REGD02_reg,~(ICP_PLL_MODE_mask|ICP_ACDR_MODE_STEP1_mask|ICP_ACDR_MODE_STEP2_mask|ICP_ACDR_MODE_STEP3_mask),
				(phy_param->ICP_PLL_MODE<<28)|(phy_param->ICP_ACDR_MODE<<24)|(phy_param->ICP_ACDR_MODE<<20)|(phy_param->ICP_ACDR_MODE<<16));
	hdmi_mask(HDMIRX_PHY_FLD_R_REGD02_reg,~(ICP_PLL_MODE_mask|ICP_ACDR_MODE_STEP1_mask|ICP_ACDR_MODE_STEP2_mask|ICP_ACDR_MODE_STEP3_mask),
				(phy_param->ICP_PLL_MODE<<28)|(phy_param->ICP_ACDR_MODE<<24)|(phy_param->ICP_ACDR_MODE<<20)|(phy_param->ICP_ACDR_MODE<<16));
	hdmi_mask(HDMIRX_PHY_FLD_CK_REGD02_reg,~(ICP_PLL_MODE_mask|ICP_ACDR_MODE_STEP1_mask|ICP_ACDR_MODE_STEP2_mask|ICP_ACDR_MODE_STEP3_mask),
				(phy_param->ICP_PLL_MODE<<28)|(phy_param->ICP_ACDR_MODE<<24)|(phy_param->ICP_ACDR_MODE<<20)|(phy_param->ICP_ACDR_MODE<<16));
#else

	hdmi_mask(HDMIRX_PHY_FLD_R_REGD03_reg,~(HDMIRX_2P1_PHY_P0_FLD_R_REGD03_reg_p0_r_cp_time_mask),phy_param->CP_TIME<<3);
	hdmi_mask(HDMIRX_PHY_FLD_G_REGD03_reg,~(HDMIRX_2P1_PHY_P0_FLD_G_REGD03_reg_p0_g_cp_time_mask),phy_param->CP_TIME<<3);
	hdmi_mask(HDMIRX_PHY_FLD_B_REGD03_reg,~(HDMIRX_2P1_PHY_P0_FLD_B_REGD03_reg_p0_b_cp_time_mask),phy_param->CP_TIME<<3);
	hdmi_mask(HDMIRX_PHY_FLD_CK_REGD03_reg,~(HDMIRX_2P1_PHY_P0_FLD_CK_REGD03_reg_p0_ck_cp_time_mask),phy_param->CP_TIME<<3);

	hdmi_mask(HDMIRX_PHY_FLD_R_REGD06_reg,~(HDMIRX_2P1_PHY_P0_FLD_R_REGD06_reg_p0_r_cp_time_2_mask),phy_param->CP_TIME_2<<5);
	hdmi_mask(HDMIRX_PHY_FLD_G_REGD06_reg,~(HDMIRX_2P1_PHY_P0_FLD_G_REGD06_reg_p0_g_cp_time_2_mask),phy_param->CP_TIME_2<<5);
	hdmi_mask(HDMIRX_PHY_FLD_B_REGD06_reg,~(HDMIRX_2P1_PHY_P0_FLD_B_REGD06_reg_p0_b_cp_time_2_mask),phy_param->CP_TIME_2<<5);
	hdmi_mask(HDMIRX_PHY_FLD_CK_REGD06_reg,~(HDMIRX_2P1_PHY_P0_FLD_CK_REGD06_reg_p0_ck_cp_time_2_mask),phy_param->CP_TIME_2<<5);

	
	//icp for merlin5
	hdmi_mask(HDMIRX_PHY_FLD_B_REGD02_reg,~(ICP_PLL_MODE_mask|ICP_ACDR_MODE_STEP1_mask|ICP_ACDR_MODE_STEP2_mask|ICP_ACDR_MODE_STEP3_mask), 
				(phy_param->ICP_PLL_MODE<<28)|((phy_param->ICP_ACDR_MODE + 8)<<24)|((phy_param->ICP_ACDR_MODE + 4)<<20)|(phy_param->ICP_ACDR_MODE<<16));
	hdmi_mask(HDMIRX_PHY_FLD_G_REGD02_reg,~(ICP_PLL_MODE_mask|ICP_ACDR_MODE_STEP1_mask|ICP_ACDR_MODE_STEP2_mask|ICP_ACDR_MODE_STEP3_mask),
				(phy_param->ICP_PLL_MODE<<28)|((phy_param->ICP_ACDR_MODE + 8)<<24)|((phy_param->ICP_ACDR_MODE + 4)<<20)|(phy_param->ICP_ACDR_MODE<<16));
	hdmi_mask(HDMIRX_PHY_FLD_R_REGD02_reg,~(ICP_PLL_MODE_mask|ICP_ACDR_MODE_STEP1_mask|ICP_ACDR_MODE_STEP2_mask|ICP_ACDR_MODE_STEP3_mask),
				(phy_param->ICP_PLL_MODE<<28)|((phy_param->ICP_ACDR_MODE + 8)<<24)|((phy_param->ICP_ACDR_MODE + 4)<<20)|(phy_param->ICP_ACDR_MODE<<16));
	hdmi_mask(HDMIRX_PHY_FLD_CK_REGD02_reg,~(ICP_PLL_MODE_mask|ICP_ACDR_MODE_STEP1_mask|ICP_ACDR_MODE_STEP2_mask|ICP_ACDR_MODE_STEP3_mask),
				(phy_param->ICP_PLL_MODE<<28)|((phy_param->ICP_ACDR_MODE + 8)<<24)|((phy_param->ICP_ACDR_MODE + 4)<<20)|(phy_param->ICP_ACDR_MODE<<16));
#endif

	//rs/cs/cp
	hdmi_mask(HDMIRX_PHY_ACDR_B_PLL_CONFIG_reg, 
		~(HDMIRX_2P1_PHY_P0_ACDR_B_PLL_CONFIG_reg_pll_p0_acdr_b_lfrs_sel_mask|
	       HDMIRX_2P1_PHY_P0_ACDR_B_PLL_CONFIG_reg_pll_p0_acdr_b_cs_cap_sel_mask |HDMIRX_2P1_PHY_P0_ACDR_B_PLL_CONFIG_reg_pll_p0_acdr_b_cp_cap_sel_mask),
	        (phy_param->RS_PLL_MODE<<25) | (phy_param->CS_PLL_MODE<<23) |(phy_param->CP_PLL_MODE<<21));
	hdmi_mask(HDMIRX_PHY_ACDR_G_PLL_CONFIG_reg, 
		~(HDMIRX_2P1_PHY_P0_ACDR_G_PLL_CONFIG_reg_pll_p0_acdr_g_lfrs_sel_mask|
	       HDMIRX_2P1_PHY_P0_ACDR_G_PLL_CONFIG_reg_pll_p0_acdr_g_cs_cap_sel_mask |HDMIRX_2P1_PHY_P0_ACDR_G_PLL_CONFIG_reg_pll_p0_acdr_g_cp_cap_sel_mask),
	        (phy_param->RS_PLL_MODE<<25) | (phy_param->CS_PLL_MODE<<23) |(phy_param->CP_PLL_MODE<<21));
	hdmi_mask(HDMIRX_PHY_ACDR_R_PLL_CONFIG_reg, 
		~(HDMIRX_2P1_PHY_P0_ACDR_R_PLL_CONFIG_reg_pll_p0_acdr_r_lfrs_sel_mask|
	       HDMIRX_2P1_PHY_P0_ACDR_R_PLL_CONFIG_reg_pll_p0_acdr_r_cs_cap_sel_mask |HDMIRX_2P1_PHY_P0_ACDR_R_PLL_CONFIG_reg_pll_p0_acdr_r_cp_cap_sel_mask),
	        (phy_param->RS_PLL_MODE<<25) | (phy_param->CS_PLL_MODE<<23) |(phy_param->CP_PLL_MODE<<21));
	hdmi_mask(HDMIRX_PHY_ACDR_CK_PLL_CONFIG_reg, 
		~(HDMIRX_2P1_PHY_P0_ACDR_CK_PLL_CONFIG_reg_pll_p0_acdr_ck_lfrs_sel_mask|
	       HDMIRX_2P1_PHY_P0_ACDR_CK_PLL_CONFIG_reg_pll_p0_acdr_ck_cs_cap_sel_mask |HDMIRX_2P1_PHY_P0_ACDR_CK_PLL_CONFIG_reg_pll_p0_acdr_ck_cp_cap_sel_mask),
	        (phy_param->RS_PLL_MODE<<25) | (phy_param->CS_PLL_MODE<<23) |(phy_param->CP_PLL_MODE<<21));


	hdmi_mask(HDMIRX_PHY_ACDR_B_CDR_CONFIG_reg, 
		~(HDMIRX_2P1_PHY_P0_ACDR_B_CDR_CONFIG_reg_cdr_p0_acdr_b_lfrs_sel_mask|
	       HDMIRX_2P1_PHY_P0_ACDR_B_CDR_CONFIG_reg_cdr_p0_acdr_b_cs_cap_sel_mask |HDMIRX_2P1_PHY_P0_ACDR_B_CDR_CONFIG_reg_cdr_p0_acdr_b_cp_cap_sel_mask),
	       (phy_param->RS_ACDR_MODE<<25) | (phy_param->CS_ACDR_MODE<<23) |(phy_param->CP_ACDR_MODE<<21));
	hdmi_mask(HDMIRX_PHY_ACDR_G_CDR_CONFIG_reg, 
		~(HDMIRX_2P1_PHY_P0_ACDR_G_CDR_CONFIG_reg_cdr_p0_acdr_g_lfrs_sel_mask|
	       HDMIRX_2P1_PHY_P0_ACDR_G_CDR_CONFIG_reg_cdr_p0_acdr_g_cs_cap_sel_mask |HDMIRX_2P1_PHY_P0_ACDR_G_CDR_CONFIG_reg_cdr_p0_acdr_g_cp_cap_sel_mask),
	       (phy_param->RS_ACDR_MODE<<25) | (phy_param->CS_ACDR_MODE<<23) |(phy_param->CP_ACDR_MODE<<21));
	hdmi_mask(HDMIRX_PHY_ACDR_R_CDR_CONFIG_reg, 
		~(HDMIRX_2P1_PHY_P0_ACDR_R_CDR_CONFIG_reg_cdr_p0_acdr_r_lfrs_sel_mask|
	       HDMIRX_2P1_PHY_P0_ACDR_R_CDR_CONFIG_reg_cdr_p0_acdr_r_cs_cap_sel_mask |HDMIRX_2P1_PHY_P0_ACDR_R_CDR_CONFIG_reg_cdr_p0_acdr_r_cp_cap_sel_mask),
	       (phy_param->RS_ACDR_MODE<<25) | (phy_param->CS_ACDR_MODE<<23) |(phy_param->CP_ACDR_MODE<<21));
	hdmi_mask(HDMIRX_PHY_ACDR_CK_CDR_CONFIG_reg, 
		~(HDMIRX_2P1_PHY_P0_ACDR_CK_CDR_CONFIG_reg_cdr_p0_acdr_ck_lfrs_sel_mask|
	       HDMIRX_2P1_PHY_P0_ACDR_CK_CDR_CONFIG_reg_cdr_p0_acdr_ck_cs_cap_sel_mask |HDMIRX_2P1_PHY_P0_ACDR_CK_CDR_CONFIG_reg_cdr_p0_acdr_ck_cp_cap_sel_mask),
	       (phy_param->RS_ACDR_MODE<<25) | (phy_param->CS_ACDR_MODE<<23) |(phy_param->CP_ACDR_MODE<<21));


	hdmi_mask(HDMIRX_PHY_ACDR_B_MANUAL_CONFIG_reg, 
		~(HDMIRX_2P1_PHY_P0_ACDR_B_MANUAL_CONFIG_reg_manual_p0_acdr_b_icp_sel_mask|HDMIRX_2P1_PHY_P0_ACDR_B_MANUAL_CONFIG_reg_manual_p0_acdr_b_lfrs_sel_mask|
	       HDMIRX_2P1_PHY_P0_ACDR_B_MANUAL_CONFIG_reg_manual_p0_acdr_b_cs_cap_sel_mask |HDMIRX_2P1_PHY_P0_ACDR_B_MANUAL_CONFIG_reg_manual_p0_acdr_b_cp_cap_sel_mask),
	       (phy_param->RS_MANUAL_MODE<<25) | (phy_param->CS_MANUAL_MODE<<23) |(phy_param->CP_MANUAL_MODE<<21));
	hdmi_mask(HDMIRX_PHY_ACDR_G_MANUAL_CONFIG_reg, 
		~(HDMIRX_2P1_PHY_P0_ACDR_G_MANUAL_CONFIG_reg_manual_p0_acdr_g_icp_sel_mask|HDMIRX_2P1_PHY_P0_ACDR_G_MANUAL_CONFIG_reg_manual_p0_acdr_g_lfrs_sel_mask|
	       HDMIRX_2P1_PHY_P0_ACDR_G_MANUAL_CONFIG_reg_manual_p0_acdr_g_cs_cap_sel_mask |HDMIRX_2P1_PHY_P0_ACDR_G_MANUAL_CONFIG_reg_manual_p0_acdr_g_cp_cap_sel_mask),
	       (phy_param->RS_MANUAL_MODE<<25) | (phy_param->CS_MANUAL_MODE<<23) |(phy_param->CP_MANUAL_MODE<<21));
	hdmi_mask(HDMIRX_PHY_ACDR_R_MANUAL_CONFIG_reg, 
		~(HDMIRX_2P1_PHY_P0_ACDR_R_MANUAL_CONFIG_reg_manual_p0_acdr_r_icp_sel_mask|HDMIRX_2P1_PHY_P0_ACDR_R_MANUAL_CONFIG_reg_manual_p0_acdr_r_lfrs_sel_mask|
	       HDMIRX_2P1_PHY_P0_ACDR_R_MANUAL_CONFIG_reg_manual_p0_acdr_r_cs_cap_sel_mask |HDMIRX_2P1_PHY_P0_ACDR_R_MANUAL_CONFIG_reg_manual_p0_acdr_r_cp_cap_sel_mask),
	       (phy_param->RS_MANUAL_MODE<<25) | (phy_param->CS_MANUAL_MODE<<23) |(phy_param->CP_MANUAL_MODE<<21));
	hdmi_mask(HDMIRX_PHY_ACDR_CK_MANUAL_CONFIG_reg, 
		~(HDMIRX_2P1_PHY_P0_ACDR_CK_MANUAL_CONFIG_reg_manual_p0_acdr_ck_icp_sel_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_MANUAL_CONFIG_reg_manual_p0_acdr_ck_lfrs_sel_mask|
	       HDMIRX_2P1_PHY_P0_ACDR_CK_MANUAL_CONFIG_reg_manual_p0_acdr_ck_cs_cap_sel_mask |HDMIRX_2P1_PHY_P0_ACDR_CK_MANUAL_CONFIG_reg_manual_p0_acdr_ck_cp_cap_sel_mask),
	       (phy_param->RS_MANUAL_MODE<<25) | (phy_param->CS_MANUAL_MODE<<23) |(phy_param->CP_MANUAL_MODE<<21));


	hdmi_mask(HDMIRX_PHY_ACDR_R_PLL_CONFIG_reg,~(HDMIRX_2P1_PHY_P0_ACDR_R_PLL_CONFIG_reg_pll_p0_acdr_r_vco_fine_i_mask|HDMIRX_2P1_PHY_P0_ACDR_R_PLL_CONFIG_reg_pll_p0_acdr_r_vco_coarse_i_mask|HDMIRX_2P1_PHY_P0_ACDR_R_PLL_CONFIG_reg_pll_p0_acdr_r_en_lpf_r_mask|HDMIRX_2P1_PHY_P0_ACDR_R_PLL_CONFIG_reg_pll_p0_acdr_r_en_lpf_c_mask|HDMIRX_2P1_PHY_P0_ACDR_R_PLL_CONFIG_reg_pll_p0_acdr_r_en_idn_mask|HDMIRX_2P1_PHY_P0_ACDR_R_PLL_CONFIG_reg_pll_p0_acdr_r_sel_idn_mask),phy_param->FINE_I_PLL_MODE <<17|phy_param->COURSE_I_PLL_MODE <<13|phy_param->RES_EN_PLL_MODE <<12|phy_param->CAP_EN_PLL_MODE <<11|phy_param->DM_EN_PLL_MODE<<10|phy_param->DM_I_PLL_MODE<<6);
	hdmi_mask(HDMIRX_PHY_ACDR_G_PLL_CONFIG_reg,~(HDMIRX_2P1_PHY_P0_ACDR_G_PLL_CONFIG_reg_pll_p0_acdr_g_vco_fine_i_mask|HDMIRX_2P1_PHY_P0_ACDR_G_PLL_CONFIG_reg_pll_p0_acdr_g_vco_coarse_i_mask|HDMIRX_2P1_PHY_P0_ACDR_G_PLL_CONFIG_reg_pll_p0_acdr_g_en_lpf_r_mask|HDMIRX_2P1_PHY_P0_ACDR_G_PLL_CONFIG_reg_pll_p0_acdr_g_en_lpf_c_mask|HDMIRX_2P1_PHY_P0_ACDR_G_PLL_CONFIG_reg_pll_p0_acdr_g_en_idn_mask|HDMIRX_2P1_PHY_P0_ACDR_G_PLL_CONFIG_reg_pll_p0_acdr_g_sel_idn_mask),phy_param->FINE_I_PLL_MODE <<17|phy_param->COURSE_I_PLL_MODE <<13|phy_param->RES_EN_PLL_MODE <<12|phy_param->CAP_EN_PLL_MODE <<11|phy_param->DM_EN_PLL_MODE<<10|phy_param->DM_I_PLL_MODE<<6);
	hdmi_mask(HDMIRX_PHY_ACDR_B_PLL_CONFIG_reg,~(HDMIRX_2P1_PHY_P0_ACDR_B_PLL_CONFIG_reg_pll_p0_acdr_b_vco_fine_i_mask|HDMIRX_2P1_PHY_P0_ACDR_B_PLL_CONFIG_reg_pll_p0_acdr_b_vco_coarse_i_mask|HDMIRX_2P1_PHY_P0_ACDR_B_PLL_CONFIG_reg_pll_p0_acdr_b_en_lpf_r_mask|HDMIRX_2P1_PHY_P0_ACDR_B_PLL_CONFIG_reg_pll_p0_acdr_b_en_lpf_c_mask|HDMIRX_2P1_PHY_P0_ACDR_B_PLL_CONFIG_reg_pll_p0_acdr_b_en_idn_mask|HDMIRX_2P1_PHY_P0_ACDR_B_PLL_CONFIG_reg_pll_p0_acdr_b_sel_idn_mask),phy_param->FINE_I_PLL_MODE <<17|phy_param->COURSE_I_PLL_MODE <<13|phy_param->RES_EN_PLL_MODE <<12|phy_param->CAP_EN_PLL_MODE <<11|phy_param->DM_EN_PLL_MODE<<10|phy_param->DM_I_PLL_MODE<<6);
	hdmi_mask(HDMIRX_PHY_ACDR_CK_PLL_CONFIG_reg,~(HDMIRX_2P1_PHY_P0_ACDR_CK_PLL_CONFIG_reg_pll_p0_acdr_ck_vco_fine_i_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_PLL_CONFIG_reg_pll_p0_acdr_ck_vco_coarse_i_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_PLL_CONFIG_reg_pll_p0_acdr_ck_en_lpf_r_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_PLL_CONFIG_reg_pll_p0_acdr_ck_en_lpf_c_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_PLL_CONFIG_reg_pll_p0_acdr_ck_en_idn_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_PLL_CONFIG_reg_pll_p0_acdr_ck_sel_idn_mask),phy_param->FINE_I_PLL_MODE <<17|phy_param->COURSE_I_PLL_MODE <<13|phy_param->RES_EN_PLL_MODE <<12|phy_param->CAP_EN_PLL_MODE <<11|phy_param->DM_EN_PLL_MODE<<10|phy_param->DM_I_PLL_MODE<<6);

	hdmi_mask(HDMIRX_PHY_ACDR_R_CDR_CONFIG_reg,~(HDMIRX_2P1_PHY_P0_ACDR_R_CDR_CONFIG_reg_cdr_p0_acdr_r_vco_fine_i_mask|HDMIRX_2P1_PHY_P0_ACDR_R_CDR_CONFIG_reg_cdr_p0_acdr_r_vco_coarse_i_mask|HDMIRX_2P1_PHY_P0_ACDR_R_CDR_CONFIG_reg_cdr_p0_acdr_r_en_lpf_r_mask|HDMIRX_2P1_PHY_P0_ACDR_R_CDR_CONFIG_reg_cdr_p0_acdr_r_en_lpf_c_mask|HDMIRX_2P1_PHY_P0_ACDR_R_CDR_CONFIG_reg_cdr_p0_acdr_r_en_idn_mask|HDMIRX_2P1_PHY_P0_ACDR_R_CDR_CONFIG_reg_cdr_p0_acdr_r_sel_idn_mask),phy_param->FINE_I_ACDR_MODE <<17|phy_param->COURSE_I_ACDR_MODE <<13|phy_param->RES_EN_ACDR_MODE <<12|phy_param->CAP_EN_ACDR_MODE <<11|phy_param->DM_EN_ACDR_MODE<<10|phy_param->DM_I_ACDR_MODE<<6);
	hdmi_mask(HDMIRX_PHY_ACDR_G_CDR_CONFIG_reg,~(HDMIRX_2P1_PHY_P0_ACDR_G_CDR_CONFIG_reg_cdr_p0_acdr_g_vco_fine_i_mask|HDMIRX_2P1_PHY_P0_ACDR_G_CDR_CONFIG_reg_cdr_p0_acdr_g_vco_coarse_i_mask|HDMIRX_2P1_PHY_P0_ACDR_G_CDR_CONFIG_reg_cdr_p0_acdr_g_en_lpf_r_mask|HDMIRX_2P1_PHY_P0_ACDR_G_CDR_CONFIG_reg_cdr_p0_acdr_g_en_lpf_c_mask|HDMIRX_2P1_PHY_P0_ACDR_G_CDR_CONFIG_reg_cdr_p0_acdr_g_en_idn_mask|HDMIRX_2P1_PHY_P0_ACDR_G_CDR_CONFIG_reg_cdr_p0_acdr_g_sel_idn_mask),phy_param->FINE_I_ACDR_MODE <<17|phy_param->COURSE_I_ACDR_MODE <<13|phy_param->RES_EN_ACDR_MODE <<12|phy_param->CAP_EN_ACDR_MODE <<11|phy_param->DM_EN_ACDR_MODE<<10|phy_param->DM_I_ACDR_MODE<<6);
	hdmi_mask(HDMIRX_PHY_ACDR_B_CDR_CONFIG_reg,~(HDMIRX_2P1_PHY_P0_ACDR_B_CDR_CONFIG_reg_cdr_p0_acdr_b_vco_fine_i_mask|HDMIRX_2P1_PHY_P0_ACDR_B_CDR_CONFIG_reg_cdr_p0_acdr_b_vco_coarse_i_mask|HDMIRX_2P1_PHY_P0_ACDR_B_CDR_CONFIG_reg_cdr_p0_acdr_b_en_lpf_r_mask|HDMIRX_2P1_PHY_P0_ACDR_B_CDR_CONFIG_reg_cdr_p0_acdr_b_en_lpf_c_mask|HDMIRX_2P1_PHY_P0_ACDR_B_CDR_CONFIG_reg_cdr_p0_acdr_b_en_idn_mask|HDMIRX_2P1_PHY_P0_ACDR_B_CDR_CONFIG_reg_cdr_p0_acdr_b_sel_idn_mask),phy_param->FINE_I_ACDR_MODE <<17|phy_param->COURSE_I_ACDR_MODE <<13|phy_param->RES_EN_ACDR_MODE <<12|phy_param->CAP_EN_ACDR_MODE <<11|phy_param->DM_EN_ACDR_MODE<<10|phy_param->DM_I_ACDR_MODE<<6);
	hdmi_mask(HDMIRX_PHY_ACDR_CK_CDR_CONFIG_reg,~(HDMIRX_2P1_PHY_P0_ACDR_CK_CDR_CONFIG_reg_cdr_p0_acdr_ck_vco_fine_i_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_CDR_CONFIG_reg_cdr_p0_acdr_ck_vco_coarse_i_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_CDR_CONFIG_reg_cdr_p0_acdr_ck_en_lpf_r_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_CDR_CONFIG_reg_cdr_p0_acdr_ck_en_lpf_c_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_CDR_CONFIG_reg_cdr_p0_acdr_ck_en_idn_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_CDR_CONFIG_reg_cdr_p0_acdr_ck_sel_idn_mask),phy_param->FINE_I_ACDR_MODE <<17|phy_param->COURSE_I_ACDR_MODE <<13|phy_param->RES_EN_ACDR_MODE <<12|phy_param->CAP_EN_ACDR_MODE <<11|phy_param->DM_EN_ACDR_MODE<<10|phy_param->DM_I_ACDR_MODE<<6);

	hdmi_mask(HDMIRX_PHY_ACDR_R_MANUAL_CONFIG_reg,~(HDMIRX_2P1_PHY_P0_ACDR_R_MANUAL_CONFIG_reg_manual_p0_acdr_r_vco_fine_i_mask|HDMIRX_2P1_PHY_P0_ACDR_R_MANUAL_CONFIG_reg_manual_p0_acdr_r_vco_coarse_i_mask|HDMIRX_2P1_PHY_P0_ACDR_R_MANUAL_CONFIG_reg_manual_p0_acdr_r_en_lpf_r_mask|HDMIRX_2P1_PHY_P0_ACDR_R_MANUAL_CONFIG_reg_manual_p0_acdr_r_en_lpf_c_mask|HDMIRX_2P1_PHY_P0_ACDR_R_MANUAL_CONFIG_reg_manual_p0_acdr_r_en_idn_mask|HDMIRX_2P1_PHY_P0_ACDR_R_MANUAL_CONFIG_reg_manual_p0_acdr_r_sel_idn_mask),phy_param->FINE_I_MANUAL_MODE <<17|phy_param->COURSE_I_MANUAL_MODE <<13|phy_param->RES_EN_MANUAL_MODE <<12|phy_param->CAP_EN_MANUAL_MODE <<11|phy_param->DM_EN_MANUAL_MODE<<10|phy_param->DM_I_MANUAL_MODE<<6);
	hdmi_mask(HDMIRX_PHY_ACDR_G_MANUAL_CONFIG_reg,~(HDMIRX_2P1_PHY_P0_ACDR_G_MANUAL_CONFIG_reg_manual_p0_acdr_g_vco_fine_i_mask|HDMIRX_2P1_PHY_P0_ACDR_G_MANUAL_CONFIG_reg_manual_p0_acdr_g_vco_coarse_i_mask|HDMIRX_2P1_PHY_P0_ACDR_G_MANUAL_CONFIG_reg_manual_p0_acdr_g_en_lpf_r_mask|HDMIRX_2P1_PHY_P0_ACDR_G_MANUAL_CONFIG_reg_manual_p0_acdr_g_en_lpf_c_mask|HDMIRX_2P1_PHY_P0_ACDR_G_MANUAL_CONFIG_reg_manual_p0_acdr_g_en_idn_mask|HDMIRX_2P1_PHY_P0_ACDR_G_MANUAL_CONFIG_reg_manual_p0_acdr_g_sel_idn_mask),phy_param->FINE_I_MANUAL_MODE <<17|phy_param->COURSE_I_MANUAL_MODE <<13|phy_param->RES_EN_MANUAL_MODE <<12|phy_param->CAP_EN_MANUAL_MODE <<11|phy_param->DM_EN_MANUAL_MODE<<10|phy_param->DM_I_MANUAL_MODE<<6);
	hdmi_mask(HDMIRX_PHY_ACDR_B_MANUAL_CONFIG_reg,~(HDMIRX_2P1_PHY_P0_ACDR_B_MANUAL_CONFIG_reg_manual_p0_acdr_b_vco_fine_i_mask|HDMIRX_2P1_PHY_P0_ACDR_B_MANUAL_CONFIG_reg_manual_p0_acdr_b_vco_coarse_i_mask|HDMIRX_2P1_PHY_P0_ACDR_B_MANUAL_CONFIG_reg_manual_p0_acdr_b_en_lpf_r_mask|HDMIRX_2P1_PHY_P0_ACDR_B_MANUAL_CONFIG_reg_manual_p0_acdr_b_en_lpf_c_mask|HDMIRX_2P1_PHY_P0_ACDR_B_MANUAL_CONFIG_reg_manual_p0_acdr_b_en_idn_mask|HDMIRX_2P1_PHY_P0_ACDR_B_MANUAL_CONFIG_reg_manual_p0_acdr_b_sel_idn_mask),phy_param->FINE_I_MANUAL_MODE <<17|phy_param->COURSE_I_MANUAL_MODE <<13|phy_param->RES_EN_MANUAL_MODE <<12|phy_param->CAP_EN_MANUAL_MODE <<11|phy_param->DM_EN_MANUAL_MODE<<10|phy_param->DM_I_MANUAL_MODE<<6);
	hdmi_mask(HDMIRX_PHY_ACDR_CK_MANUAL_CONFIG_reg,~(HDMIRX_2P1_PHY_P0_ACDR_CK_MANUAL_CONFIG_reg_manual_p0_acdr_ck_vco_fine_i_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_MANUAL_CONFIG_reg_manual_p0_acdr_ck_vco_coarse_i_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_MANUAL_CONFIG_reg_manual_p0_acdr_ck_en_lpf_r_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_MANUAL_CONFIG_reg_manual_p0_acdr_ck_en_lpf_c_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_MANUAL_CONFIG_reg_manual_p0_acdr_ck_en_idn_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_MANUAL_CONFIG_reg_manual_p0_acdr_ck_sel_idn_mask),phy_param->FINE_I_MANUAL_MODE <<17|phy_param->COURSE_I_MANUAL_MODE <<13|phy_param->RES_EN_MANUAL_MODE <<12|phy_param->CAP_EN_MANUAL_MODE <<11|phy_param->DM_EN_MANUAL_MODE<<10|phy_param->DM_I_MANUAL_MODE<<6);
										  

	//kp ki not useless so setting default value
	hdmi_mask(HDMIRX_PHY_REGD00_reg,
		~(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_reg_p0_rate_sel_mask|
		HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_reg_p0_kp_mask|
		HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_reg_p0_ki_mask|
		HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_reg_p0_bypass_sdm_int_mask),
		(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_reg_p0_rate_sel(phy_param->RATE_SEL)|
		(phy_param->KP<<HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_reg_p0_kp_shift)|
		(phy_param->KI<<HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_reg_p0_ki_shift)|
		HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_reg_p0_bypass_sdm_int_mask));

	hdmi_mask(HDMIRX_PHY_REGD10_reg, ~HDMIRX_2P1_PHY_P0_HD21_Y1_REGD10_reg_p0_ck_md_sel_mask, HDMIRX_2P1_PHY_P0_HD21_Y1_REGD10_reg_p0_ck_md_sel(0));
	


}


//clock lane
void lib_hdmi21_set_phy_table(unsigned char nport, HDMI21_PHY_TMDS_PARAM_T *phy_param)
{
	
	//enable ldo & cco for clock lane and disable for other lanes (put first setting)
	hdmi_mask(HDMIRX_PHY_ACDR_B0_reg,~(acdr_b_1_CCO_POW|acdr_b_1_LDO_EN),0x0);
	hdmi_mask(HDMIRX_PHY_ACDR_G0_reg,~(acdr_g_1_CCO_POW|acdr_g_1_LDO_EN),0x0);
	hdmi_mask(HDMIRX_PHY_ACDR_R0_reg,~(acdr_r_1_CCO_POW|acdr_r_1_LDO_EN),0x0);
	hdmi_mask(HDMIRX_PHY_ACDR_CK0_reg,~(acdr_ck_1_CCO_POW|acdr_ck_1_LDO_EN),(acdr_r_1_CCO_POW|acdr_r_1_LDO_EN));


	//MANUAL SEL
	if (nport == HDMI_PORT0)
		hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg,
					~(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg_p0_r_pllcdr_manual_enable_mask|HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg_p0_g_pllcdr_manual_enable_mask|
					HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg_p0_b_pllcdr_manual_enable_mask|HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg_p0_ck_pllcdr_manual_enable_mask),
					(phy_param->MANUAL_SEL << 30 | phy_param->MANUAL_SEL << 29 |  phy_param->MANUAL_SEL << 28 |  phy_param->MANUAL_SEL << 27));
	else
		hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg,
					~(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg_p1_r_pllcdr_manual_enable_mask|HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg_p1_g_pllcdr_manual_enable_mask|
					HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg_p1_b_pllcdr_manual_enable_mask|HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg_p1_ck_pllcdr_manual_enable_mask),
					(phy_param->MANUAL_SEL << 26 | phy_param->MANUAL_SEL << 25 |  phy_param->MANUAL_SEL << 24 |  phy_param->MANUAL_SEL << 23));


	//M code
	hdmi_mask(HDMIRX_PHY_REGD08_reg, ~HDMIRX_2P1_PHY_P0_HD21_Y1_REGD08_reg_p0_n_code_mask, (phy_param->M_code)<<HDMIRX_2P1_PHY_P0_HD21_Y1_REGD08_reg_p0_n_code_shift);		

	//N code
	hdmi_mask(HDMIRX_PHY_ACDR_CK1_reg, ~(acdr_ck_7_CMU_N_code|acdr_ck_7_CMU_PREDIVN|acdr_ck_5_VC_INIT), (phy_param->N_code<<16)|phy_param->VC_VOL_SEL);		//N code set

	//data lane ck src
	hdmi_mask(HDMIRX_PHY_ACDR_B1_reg, ~(acdr_b_6_CK_SRC_SEL),(phy_param->DATA_LANE_CK_SRC << 14));
	hdmi_mask(HDMIRX_PHY_ACDR_G1_reg, ~(acdr_g_6_CK_SRC_SEL), (phy_param->DATA_LANE_CK_SRC << 14));
	hdmi_mask(HDMIRX_PHY_ACDR_R1_reg, ~(acdr_r_6_CK_SRC_SEL), (phy_param->DATA_LANE_CK_SRC << 14));
	//hdmi_mask(HDMIRX_PHY_ACDR_CK1_reg, ~(acdr_ck_6_CK_SRC_SEL), (phy_param->DATA_LANE_CK_SRC << 14));



	//charge pump cap & op enable  , postM 
	hdmi_mask(HDMIRX_PHY_ACDR_CK0_reg,~(acdr_ck_4_CP_OP|acdr_ck_4_CP_CAP|acdr_ck_2_POST_M),((phy_param->CP_OP_EN<<25)|(phy_param->CP_CAP_EN<<24)|(phy_param->POST_M<<10)));


	//demux/pi_div/pi_csel
	hdmi_mask(HDMIRX_PHY_B1_reg, ~(b_8_DEMUX|b_8_PR_RATE_SEL|b_7_PI_ISEL), (phy_param->DEMUX_RATE_SEL<<29)| (phy_param->PI_DIV<<26)|(((phy_param->PI_ISEL & 0x4)>>2 )<<24)|(phy_param->PI_ISEL<<16));
	hdmi_mask(HDMIRX_PHY_G1_reg, ~(g_8_DEMUX|b_8_PR_RATE_SEL|g_7_PI_ISEL), (phy_param->DEMUX_RATE_SEL<<29)|(phy_param->PI_DIV<<26)|(((phy_param->PI_ISEL & 0x4)>>2 )<<24)|(phy_param->PI_ISEL<<16));
	hdmi_mask(HDMIRX_PHY_R1_reg, ~(r_8_DEMUX|b_8_PR_RATE_SEL|r_7_PI_ISEL), (phy_param->DEMUX_RATE_SEL<<29)|(phy_param->PI_DIV<<26)|(((phy_param->PI_ISEL & 0x4)>>2 )<<24)|(phy_param->PI_ISEL<<16));


	//HALF&FULL RATE SEL/PIX_RATE_SEL/PI_CSEL
	hdmi_mask(HDMIRX_PHY_B2_reg, ~(b_12_FR_CK_SEL|b_12_PIX_RATE_SEL|b_12_PI_CURRENT_ADJ|b_12_PI_CSEL), (phy_param->FULL_RATE_SEL<<31)|(phy_param->PIX_RATE_SEL<<29)|(phy_param->PI_CURRENT_ADJ<< 28)|(phy_param->PI_CSEL<<24));
	hdmi_mask(HDMIRX_PHY_G2_reg, ~(g_12_FR_CK_SEL|g_12_PIX_RATE_SEL|g_12_PI_CURRENT_ADJ|g_12_PI_CSEL), (phy_param->FULL_RATE_SEL<<31)|(phy_param->PIX_RATE_SEL<<29)|(phy_param->PI_CURRENT_ADJ << 28)|(phy_param->PI_CSEL<<24));
	hdmi_mask(HDMIRX_PHY_R2_reg, ~(r_12_FR_CK_SEL|r_12_PIX_RATE_SEL|r_12_PI_CURRENT_ADJ|r_12_PI_CSEL), (phy_param->FULL_RATE_SEL<<31)|(phy_param->PIX_RATE_SEL<<29)|(phy_param->PI_CURRENT_ADJ << 28)|(phy_param->PI_CSEL<<24));
	
	
	//icp double , pre-M
	
	hdmi_mask(HDMIRX_PHY_ACDR_CK0_reg,~(acdr_ck_2_PRE_M|acdr_ck_1_HALF_RATE_SEL), (phy_param->PRE_M << 9) | (phy_param->HALF_RATE_SEL<<4));

	//DELAY_CEL LDO_VSEL  KVCO
	hdmi_mask(HDMIRX_PHY_ACDR_CK1_reg,~(acdr_ck_8_DELAY_CELL_SEL|acdr_ck_6_LDO2V|acdr_ck_5_KVCO_SEL), phy_param->DELAY_CEL_SEL<<24|HD21_RX_PHY_P0_VSEL_LDO(0) | phy_param->CMU_KVCO<<5);

#if 0  //check after
	//ENCAP
	hdmi_mask(HDMIRX_PHY_MISC_0_reg,
				~(HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_r_vco_encap_mask|HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_g_vco_encap_mask|
				HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_b_vco_encap_mask|HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_ck_vco_encap_mask),
				(phy_param->EN_CAP<<30|phy_param->EN_CAP<<22| phy_param->EN_CAP<<14|phy_param->EN_CAP<<6));
	hdmi_mask(HDMIRX_PHY_MISC_0_reg, ~(HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_ck_vco_encap_mask),phy_param->EN_CAP<<6);
#endif	

	//icp for merlin5
	hdmi_mask(HDMIRX_PHY_FLD_CK_REGD02_reg,~(ICP_PLL_MODE_mask),(phy_param->ICP_PLL_MODE<<28));
	

	//rs/cs/cp
	hdmi_mask(HDMIRX_PHY_ACDR_CK_MANUAL_CONFIG_reg, 
		~(HDMIRX_2P1_PHY_P0_ACDR_CK_MANUAL_CONFIG_reg_manual_p0_acdr_ck_lfrs_sel_mask|
	       HDMIRX_2P1_PHY_P0_ACDR_CK_MANUAL_CONFIG_reg_manual_p0_acdr_ck_cs_cap_sel_mask |HDMIRX_2P1_PHY_P0_ACDR_CK_MANUAL_CONFIG_reg_manual_p0_acdr_ck_cp_cap_sel_mask),
	       (phy_param->RS_MANUAL_MODE<<25) | (phy_param->CS_MANUAL_MODE<<23) |(phy_param->CP_MANUAL_MODE<<21));
											  

	hdmi_mask(HDMIRX_PHY_ACDR_CK_MANUAL_CONFIG_reg,~(HDMIRX_2P1_PHY_P0_ACDR_CK_MANUAL_CONFIG_reg_manual_p0_acdr_ck_vco_fine_i_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_MANUAL_CONFIG_reg_manual_p0_acdr_ck_vco_coarse_i_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_MANUAL_CONFIG_reg_manual_p0_acdr_ck_en_lpf_r_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_MANUAL_CONFIG_reg_manual_p0_acdr_ck_en_lpf_c_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_MANUAL_CONFIG_reg_manual_p0_acdr_ck_en_idn_mask|HDMIRX_2P1_PHY_P0_ACDR_CK_MANUAL_CONFIG_reg_manual_p0_acdr_ck_sel_idn_mask),phy_param->FINE_I_MANUAL_MODE <<17|phy_param->COURSE_I_MANUAL_MODE <<13|phy_param->RES_EN_MANUAL_MODE <<12|phy_param->CAP_EN_MANUAL_MODE <<11|phy_param->DM_EN_MANUAL_MODE<<10|phy_param->DM_I_MANUAL_MODE<<6);

	hdmi_mask(HDMIRX_PHY_REGD00_reg,
		~(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_reg_p0_rate_sel_mask|
		HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_reg_p0_kp_mask|
		HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_reg_p0_ki_mask|
		HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_reg_p0_bypass_sdm_int_mask),
		(HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_reg_p0_rate_sel(phy_param->RATE_SEL)|
		(phy_param->KP<<HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_reg_p0_kp_shift)|
		(phy_param->KI<<HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_reg_p0_ki_shift)|
		HDMIRX_2P1_PHY_P0_HD21_Y1_REGD00_reg_p0_bypass_sdm_int_mask));

	hdmi_mask(HDMIRX_PHY_REGD10_reg, ~HDMIRX_2P1_PHY_P0_HD21_Y1_REGD10_reg_p0_ck_md_sel_mask, HDMIRX_2P1_PHY_P0_HD21_Y1_REGD10_reg_p0_ck_md_sel(0));
	

}

void lib_hdmi_fld_config(unsigned char nport, unsigned int bclk, HDMI21_PHY_TMDS_PARAM_T *phy_param)
{
	unsigned int Fvco = 0;
	unsigned int tmds_b = bclk;
	unsigned int fld_cnt_up,fld_cnt_dn;
	unsigned int Fvco_up, Fvco_dn;
	unsigned int divide_num = 30;
	//Fvco=[(Fin x PRE_M x POST_M x M_Code) ¡Ò N_Code ]
	//Fin=[(b+1)x27¡Ò256]¡A

	
	if (newbase_hdmi2p0_get_clock40x_flag(nport)) {
		if (((bclk >> 2) < 1422) && ((bclk >> 2) > 699))
			tmds_b = (bclk >> 2);
	}

	//cco band
	Fvco = (tmds_b+1) * 27 *  (phy_param->PRE_M? 1:2) * (1<< phy_param->POST_M) * (phy_param->M_code + 4);	
	Fvco = (Fvco / (1<< phy_param->N_code) ) >> 8;


	if ((Fvco > 5000) && (Fvco <= 6000)) {
		Fvco_up = Fvco + 150;
		Fvco_dn = Fvco - 150;
	} else if ((Fvco > 2000) && (Fvco <= 4999)) {
		Fvco_up = Fvco + 190;
		Fvco_dn = Fvco - 190;
	} else {
		HDMI_EMG("Fvco can not found \n");
		return;
	}

	
	//fld_cnt = (divide num * ck_fld * (fvco/16) * 2)/27
	
	fld_cnt_up = (divide_num * (Fvco_up >> 3)) / 27;
	fld_cnt_dn = (divide_num * (Fvco_dn >> 3)) / 27;
	
	
	hdmi_mask(HDMIRX_PHY_FLD_CK_REGD04_reg,~(HDMIRX_2P1_PHY_P0_FLD_CK_REGD04_reg_p0_ck_vco_coarse_init_mask|HDMIRX_2P1_PHY_P0_FLD_CK_REGD04_reg_p0_ck_vco_fine_init_mask),HDMIRX_2P1_PHY_P0_FLD_CK_REGD04_reg_p0_ck_vco_coarse_init(0x4)|HDMIRX_2P1_PHY_P0_FLD_CK_REGD04_reg_p0_ck_vco_fine_init(0x4));
	hdmi_mask(HDMIRX_PHY_FLD_CK_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_CK_REGD00_reg_p0_ck_divide_num_mask),HDMIRX_2P1_PHY_P0_FLD_CK_REGD00_reg_p0_ck_divide_num(30));
	hdmi_mask(HDMIRX_PHY_FLD_CK_REGD03_reg,~(HDMIRX_2P1_PHY_P0_FLD_CK_REGD03_reg_p0_ck_coarse_lock_up_limit_mask|HDMIRX_2P1_PHY_P0_FLD_CK_REGD03_reg_p0_ck_coarse_lock_dn_limit_mask),HDMIRX_2P1_PHY_P0_FLD_R_REGD03_reg_p0_r_coarse_lock_up_limit(fld_cnt_up)|HDMIRX_2P1_PHY_P0_FLD_R_REGD03_reg_p0_r_coarse_lock_dn_limit(fld_cnt_dn));

	//dcdr mode: no wait 20us to check freq and change to acdr , so no use lock up/dn limit
	hdmi_mask(HDMIRX_PHY_FLD_CK_REGD01_reg,~(HDMIRX_2P1_PHY_P0_FLD_CK_REGD01_reg_p0_ck_lock_up_limit_mask|HDMIRX_2P1_PHY_P0_FLD_CK_REGD01_reg_p0_ck_lock_dn_limit_mask),HDMIRX_2P1_PHY_P0_FLD_CK_REGD01_reg_p0_ck_lock_up_limit(0x0)|HDMIRX_2P1_PHY_P0_FLD_CK_REGD01_reg_p0_ck_lock_dn_limit(0x0));
	
}

void lib_hdmi21_cco_band_config(unsigned char nport, unsigned int bclk, HDMI21_PHY_TMDS_PARAM_T *phy_param)
{
	HDMI21_PHY_TMDS_PARAM2_T *phy_param2 = &hdmi21_phy_tmds_param2[0];
	unsigned char i = 0, tabsize = 0;
	unsigned int Fvco = 0;
	unsigned int tmds_b = bclk;
	
	//Fvco=[(Fin x PRE_M x POST_M x M_Code) ¡Ò N_Code ]
	//Fin=[(b+1)x27¡Ò256]¡A

	
	if (newbase_hdmi2p0_get_clock40x_flag(nport)) {
		if (((bclk >> 2) < 1422) && ((bclk >> 2) > 699))
			tmds_b = (bclk >> 2);
	}

	//cco band
	Fvco = (tmds_b+1) * 27 *  (phy_param->PRE_M? 1:2) * (1<< phy_param->POST_M) * (phy_param->M_code + 4);	
	Fvco = (Fvco / (1<< phy_param->N_code) ) >> 8;


	tabsize = sizeof(hdmi21_phy_tmds_param2)/sizeof(HDMI21_PHY_TMDS_PARAM2_T);

	for (i = 0 ; i < tabsize ; ++i) {
		if ( (Fvco >= hdmi21_phy_tmds_param2[i].fvco_lower)  &&  (Fvco < phy_param2[i].fvco_upper)) {
				break;
		}
	}

	if (i >= tabsize) {
	        HDMI_EMG("Not Found in PHY_PARAM2 (Fvco=%d)\n", Fvco);
		 return;
	}

	phy_param2 = &hdmi21_phy_tmds_param2[i];

	//setting register in lib_hdmi_set_phy_table
	phy_param->CMU_KVCO = phy_param2 ->CMU_KVCO;
	phy_param->EN_CAP = phy_param2->EN_CAP;
	phy_param->PI_ISEL = phy_param2->PI_ISEL;
	phy_param->PI_CURRENT_ADJ = phy_param2->PI_CURRENT_ADJ;
	phy_param->PI_CSEL = phy_param2->PI_CSEL;
	phy_param->FINE_I_MANUAL_MODE = phy_param2->FINE_I;
	phy_param->COURSE_I_MANUAL_MODE = phy_param2->COURSE_I;

	hdmi_mask(HDMIRX_PHY_ACDR_CK0_reg, ~(acdr_ck_3_BAND_SEL), HD21_RX_PHY_CCO_BAND_SEL(phy_param2->CCO_BAND_SEL));

}


void lib_hdmi21_fg_koffset_manual_adjust(unsigned char nport, unsigned int r, unsigned int g, unsigned int b, unsigned int ck)
{
	if (nport >= HD21_PORT_TOTAL_NUM) {
		return;
	}

	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_3_FORE_KOFF_ADJR), r<<16);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_3_FORE_KOFF_ADJR), g<<16);
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_3_FORE_KOFF_ADJR), b<<16);
	hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_3_FORE_KOFF_ADJR), ck<<16);	
}

void lib_hdmi21_fg_koffset_manual(unsigned char nport)
{
	if (nport >= HD21_PORT_TOTAL_NUM) {
		return;
	}

  	hdmi_mask(HDMIRX_PHY_R0_reg, ~(r_4_FORE_KOFF_AUTOK), 0);
  	hdmi_mask(HDMIRX_PHY_G0_reg, ~(g_4_FORE_KOFF_AUTOK), 0);
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_KOFF_AUTOK), 0);
	hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_KOFF_AUTOK), 0);

}


#if 1

void lib_hdmi_do_koffset_dcvs_cal(unsigned char nport,unsigned char lane_mode)
{
	volatile u_int32_t koff_r_result;
	volatile u_int32_t  koff_g_result;
	volatile u_int32_t  koff_b_result;
	volatile u_int32_t koff_ck_result;
	UINT8 k_cnt;
	//DCVS no need to setting koffset range
	
	//for b lane
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);

	// start cal
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF),b_4_FORE_POW_FORE_KOFF);

	//wait finish cal
	k_cnt = 100;
	while (!((hdmi_in(HDMIRX_PHY_KOFFSET_reg))&HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_ok_b_mask) && --k_cnt) {
		udelay(10);
	}

	if (!k_cnt) {
		HDMI_ERR("%s: func err(B)\n", __func__);
	}

	koff_b_result = HDMIRX_2P1_PHY_P0_KOFFSET_get_reg_p0_fore_off_tst_b(hdmi_in(HDMIRX_PHY_KOFFSET_reg));

	if (!(hdmi_in(HDMIRX_PHY_KOFFSET_reg) & HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_bound_b_mask)) {
		//HDMI_PRINTF("[HDMI] B channel foreground K Off result   %x  range =%x \n",koff_b_result,i);
	} else {
		HDMI_INFO("%s: out of B range \n", __func__);
	}


	//for g lane
	
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);

	// start cal
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF),b_4_FORE_POW_FORE_KOFF);

	//wait finish cal
	k_cnt = 100;
	while (!((hdmi_in(HDMIRX_PHY_KOFFSET_reg))&HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_ok_g_mask) && --k_cnt) {
		udelay(10);
	}

	if (!k_cnt) {
		HDMI_ERR("%s: func err(G)\n", __func__);
	}

	koff_g_result = HDMIRX_2P1_PHY_P0_KOFFSET_get_reg_p0_fore_off_tst_g(hdmi_in(HDMIRX_PHY_KOFFSET_reg));

	if (!(hdmi_in(HDMIRX_PHY_KOFFSET_reg) & HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_bound_g_mask)) {
		//HDMI_PRINTF("[HDMI] G channel foreground K Off result   %x  range =%x \n",koff_g_result,i);
	} else {
		HDMI_INFO("%s: out of G range \n", __func__);
	}
	
	//for r lane

	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);

	// start cal
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF),b_4_FORE_POW_FORE_KOFF);

	//wait finish cal
	k_cnt = 100;
	while(!((hdmi_in(HDMIRX_PHY_KOFFSET_reg))&HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_ok_r_mask) && --k_cnt) {
		udelay(10);
	}

	if (!k_cnt) {
		HDMI_ERR("%s: func err(R)\n", __func__);
	}

	koff_r_result = HDMIRX_2P1_PHY_P0_KOFFSET_get_reg_p0_fore_off_tst_r(hdmi_in(HDMIRX_PHY_KOFFSET_reg));

	if (!(hdmi_in(HDMIRX_PHY_KOFFSET_reg) & HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_bound_r_mask)) {
		//HDMI_PRINTF("[HDMI] R channel foreground K Off result   %x  range =%x \n",koff_r_result,i);
	} else {
		HDMI_INFO("%s: out of R range \n", __func__);
	}
	

	//for ck lane
	if (lane_mode == HDMI_4LANE) {
			hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);
			// start cal
			hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF),b_4_FORE_POW_FORE_KOFF);

			//wait finish cal
			k_cnt = 100;
			while(!((hdmi_in(HDMIRX_PHY_KOFFSET_reg))&HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_ok_ck_mask) && --k_cnt) {
				udelay(10);
			}

			if (!k_cnt) {
				HDMI_ERR("%s: func err(CK)\n", __func__);
			}

			koff_ck_result = HDMIRX_2P1_PHY_P0_KOFFSET_get_reg_p0_fore_off_tst_ck(hdmi_in(HDMIRX_PHY_KOFFSET_reg));

			if (!(hdmi_in(HDMIRX_PHY_KOFFSET_reg) & HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_bound_ck_mask)) {
				//HDMI_PRINTF("[HDMI] CK channel foreground K Off result   %x  range =%x \n",koff_ck_result,i);
			} else {
				HDMI_INFO("%s: out of CK range \n", __func__);
			}
	}

	if (lane_mode == HDMI_4LANE)
		HDMI_PRINTF("FG KOFFSET=(b=%d,g=%d,r=%d,ck=%d)\n", koff_b_result,koff_g_result,koff_r_result,koff_ck_result);
	else
		HDMI_PRINTF("FG KOFFSET=(b=%d,g=%d,r=%d)\n", koff_b_result,koff_g_result,koff_r_result);




}



void lib_hdmi_do_koffset_eq_cal(unsigned char nport,unsigned char lane_mode)
{
	#define FORE_KOFF_RANGE  3  // range maybe 0  1  2   3

	volatile u_int32_t koff_r_result;
	volatile u_int32_t  koff_g_result;
	volatile u_int32_t  koff_b_result;
	volatile u_int32_t koff_ck_result;
	UINT8 i, k_cnt;
	unsigned char range_r,range_g,range_b,range_ck;
	
	//for b lane
	for (i=0; i<FORE_KOFF_RANGE+1; i++) {
		hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);

		//set k off range
		hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_KOFF_RANGE),i<<29);

		// start cal
		hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF),b_4_FORE_POW_FORE_KOFF);

		//wait finish cal
		k_cnt = 100;
		while (!((hdmi_in(HDMIRX_PHY_KOFFSET_reg))&HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_ok_b_mask) && --k_cnt) {
			udelay(10);
		}

		if (!k_cnt) {
			HDMI_EMG("[ERROR] %s: func err(B)\n", __func__);
		}

		koff_b_result = HDMIRX_2P1_PHY_P0_KOFFSET_get_reg_p0_fore_off_tst_b(hdmi_in(HDMIRX_PHY_KOFFSET_reg));
		range_b = i;
		if (!(hdmi_in(HDMIRX_PHY_KOFFSET_reg) & HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_bound_b_mask)) {
			//HDMI_PRINTF("[HDMI] B channel foreground K Off result   %x  range =%x \n",koff_b_result,i);
			break;
		} else {
			if (i == FORE_KOFF_RANGE)
				HDMI_EMG("%s: out of B range (value=%d)\n", __func__,i);

		}
	}


	//for g lane
	for (i=0; i<FORE_KOFF_RANGE+1; i++) {
		hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);

		//set k off range
		hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_KOFF_RANGE),i<<29);

		// start cal
		hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF),b_4_FORE_POW_FORE_KOFF);

		//wait finish cal
		k_cnt = 100;
		while (!((hdmi_in(HDMIRX_PHY_KOFFSET_reg))&HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_ok_g_mask) && --k_cnt) {
			udelay(10);
		}

		if (!k_cnt) {
			HDMI_EMG("[ERROR] %s: func err(G)\n", __func__);
		}

		koff_g_result = HDMIRX_2P1_PHY_P0_KOFFSET_get_reg_p0_fore_off_tst_g(hdmi_in(HDMIRX_PHY_KOFFSET_reg));
		range_g = i;
		if (!(hdmi_in(HDMIRX_PHY_KOFFSET_reg) & HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_bound_g_mask)) {
		//	HDMI_PRINTF("[HDMI] G channel foreground K Off result   %x  range =%x \n",koff_g_result,i);
			break;
		} else {
			if (i == FORE_KOFF_RANGE)
				HDMI_EMG("%s: out of G range (value=%d)\n", __func__,i);

		}
	}


	//for r lane
	for (i=0; i<FORE_KOFF_RANGE+1; i++) {
		hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);

		//set k off range
		hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_KOFF_RANGE),i<<29);

		// start cal
		hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF),b_4_FORE_POW_FORE_KOFF);

		//wait finish cal
		k_cnt = 100;
		while(!((hdmi_in(HDMIRX_PHY_KOFFSET_reg))&HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_ok_r_mask) && --k_cnt) {
			udelay(10);
		}

		if (!k_cnt) {
			HDMI_EMG("[ERROR] %s: func err(R)\n", __func__);
		}

		koff_r_result = HDMIRX_2P1_PHY_P0_KOFFSET_get_reg_p0_fore_off_tst_r(hdmi_in(HDMIRX_PHY_KOFFSET_reg));
		range_r = i;
		if (!(hdmi_in(HDMIRX_PHY_KOFFSET_reg) & HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_bound_r_mask)) {
		//	HDMI_PRINTF("[HDMI] R channel foreground K Off result   %x  range =%x \n",koff_r_result,i);
			break;
		} else {
			if (i == FORE_KOFF_RANGE)
				HDMI_EMG("%s: out of R range (value=%d)\n", __func__,i);

		}
	}

	//for ck lane
	if (lane_mode == HDMI_4LANE) {
		for (i=0; i<FORE_KOFF_RANGE+1; i++) {
			hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);

			//set k off range
			hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_KOFF_RANGE),i<<29);

			// start cal
			hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF),b_4_FORE_POW_FORE_KOFF);

			//wait finish cal
			k_cnt = 100;
			while(!((hdmi_in(HDMIRX_PHY_KOFFSET_reg))&HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_ok_ck_mask) && --k_cnt) {
				udelay(10);
			}

			if (!k_cnt) {
				HDMI_EMG("[ERROR] %s: func err(CK)\n", __func__);
			}

			koff_ck_result = HDMIRX_2P1_PHY_P0_KOFFSET_get_reg_p0_fore_off_tst_ck(hdmi_in(HDMIRX_PHY_KOFFSET_reg));
			range_ck = i;
			if (!(hdmi_in(HDMIRX_PHY_KOFFSET_reg) & HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_bound_ck_mask)) {
			//	HDMI_PRINTF("[HDMI] CK channel foreground K Off result   %x  range =%x \n",koff_ck_result,i);
				break;
			} else {
				if (i == FORE_KOFF_RANGE)
					HDMI_EMG("%s: out of CK range (value=%d)\n", __func__,i);
			}
		}

	}

	if (lane_mode == HDMI_4LANE)
		HDMI_PRINTF("FG KOFFSET=(b=%d,g=%d,r=%d,ck=%d) range=(b=%d,g=%d,r=%d,ck=%d),\n", koff_b_result,koff_g_result,koff_r_result,koff_ck_result,range_b,range_g,range_r,range_ck);
	else
		HDMI_PRINTF("FG KOFFSET=(b=%d,g=%d,r=%d) range=(b=%d,g=%d,r=%d)\n", koff_b_result,koff_g_result,koff_r_result,range_b,range_g,range_r);



}



void lib_hdmi21_fg_koffset_disable(unsigned char nport)
{

	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_3_FORE_EN_FORE_KOFF|b_1_OFFCAL_SEL), 0);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_3_FORE_EN_FORE_KOFF|b_1_OFFCAL_SEL), 0);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_3_FORE_EN_FORE_KOFF|b_1_OFFCAL_SEL), 0);
	hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_3_FORE_EN_FORE_KOFF|b_1_OFFCAL_SEL), 0);


	hdmi_mask(HDMIRX_PHY_B2_reg, ~(b_10_DCVS_EN), 0);
	hdmi_mask(HDMIRX_PHY_G2_reg, ~(b_10_DCVS_EN), 0);
	hdmi_mask(HDMIRX_PHY_R2_reg, ~(b_10_DCVS_EN), 0);
	hdmi_mask(HDMIRX_PHY_CK2_reg, ~(b_10_DCVS_EN), 0);
	
}


#if 0//Case3

void lib_hdmi21_fg_koffset_auto(unsigned char nport,unsigned char lane_mode)
{
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_3_FORE_EN_FORE_KOFF|b_1_OFFCAL_SEL), 0);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_3_FORE_EN_FORE_KOFF|b_1_OFFCAL_SEL), 0);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_3_FORE_EN_FORE_KOFF|b_1_OFFCAL_SEL), 0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_3_FORE_EN_FORE_KOFF|b_1_OFFCAL_SEL), 0);

	//EQ power disable
	hdmi_mask(HDMIRX_PHY_B1_reg, ~(b_6_EQ_POW), 0x0);
	hdmi_mask(HDMIRX_PHY_G1_reg, ~(g_6_EQ_POW) , 0x0);
	hdmi_mask(HDMIRX_PHY_R1_reg, ~(r_6_EQ_POW), 0x0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK1_reg, ~(ck_6_EQ_POW), 0x0);

	//DCVS disable
	hdmi_mask(HDMIRX_PHY_B2_reg, ~(b_10_DCVS_EN), 0);
	hdmi_mask(HDMIRX_PHY_G2_reg, ~(b_10_DCVS_EN), 0);
	hdmi_mask(HDMIRX_PHY_R2_reg, ~(b_10_DCVS_EN), 0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK2_reg, ~(b_10_DCVS_EN), 0);

	// cut input off
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_1_inputoff),b_1_inputoff);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_1_inputoff),b_1_inputoff);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_1_inputoff),b_1_inputoff);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_1_inputoff),b_1_inputoff);


	// initial circuit power
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_3_FORE_EN_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_1_OFFCAL_SEL),(b_3_FORE_EN_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_1_OFFCAL_DA_E));
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_3_FORE_EN_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_1_OFFCAL_SEL),(b_3_FORE_EN_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_1_OFFCAL_DA_E));
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_3_FORE_EN_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_1_OFFCAL_SEL),(b_3_FORE_EN_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_1_OFFCAL_DA_E));
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_3_FORE_EN_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_1_OFFCAL_SEL),(b_3_FORE_EN_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_1_OFFCAL_DA_E));

        // init koffset range
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_KOFF_RANGE),0x0);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(g_4_FORE_KOFF_RANGE),0x0);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(r_4_FORE_KOFF_RANGE),0x0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(ck_4_FORE_KOFF_RANGE),0x0);

/////////////////////////		
#if 0
	hdmi_mask(HDMIRX_PHY_B2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN); //0xb800DA38[15]
	hdmi_mask(HDMIRX_PHY_G2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN);
	hdmi_mask(HDMIRX_PHY_R2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN);

	//K DCVS DATA_EVEN
	lib_hdmi_do_koffset_dcvs_cal(nport,lane_mode);
	

	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);

#endif	

//////////////////do_koffset_eq_cal: K EQ Offset   /////////////////////////
//EQ power enable
		HDMI_PRINTF("[ K EQ Offset] \n");

	hdmi_mask(HDMIRX_PHY_B1_reg, ~(b_6_EQ_POW), b_6_EQ_POW);
	hdmi_mask(HDMIRX_PHY_G1_reg, ~(g_6_EQ_POW) ,g_6_EQ_POW);
	hdmi_mask(HDMIRX_PHY_R1_reg, ~(r_6_EQ_POW), r_6_EQ_POW);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK1_reg, ~(ck_6_EQ_POW),ck_6_EQ_POW);


	hdmi_mask(HDMIRX_PHY_B2_reg, ~(b_10_DCVS_EN), 0);
	hdmi_mask(HDMIRX_PHY_G2_reg, ~(b_10_DCVS_EN), 0);
	hdmi_mask(HDMIRX_PHY_R2_reg, ~(b_10_DCVS_EN), 0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK2_reg, ~(b_10_DCVS_EN), 0);


	//KEQ
	lib_hdmi_do_koffset_eq_cal(nport,lane_mode);

//////////////   do_koffset_dcvs_cal: DCVS DATA_EVEN    ////////////////////
		HDMI_PRINTF("[ K DCVS DATA_EVEN Offset]\n");

	hdmi_mask(HDMIRX_PHY_B2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN); //0xb800DA38[15]
	hdmi_mask(HDMIRX_PHY_G2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN);
	hdmi_mask(HDMIRX_PHY_R2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN);

	//OFFCAL_SEL, 000: Data_Even
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_1_OFFCAL_SEL),(b_1_OFFCAL_DA_E));
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_1_OFFCAL_SEL),(b_1_OFFCAL_DA_E));
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_1_OFFCAL_SEL),(b_1_OFFCAL_DA_E));
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_1_OFFCAL_SEL),(b_1_OFFCAL_DA_E));


	//K DCVS DATA_EVEN
	lib_hdmi_do_koffset_dcvs_cal(nport,lane_mode);
	

	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);


//////////////   do_koffset_dcvs_cal: DCVS DATA_ODD    ////////////////////
		HDMI_PRINTF("[ K DCVS DATA_ODD Offset]\n");

	//OFFCAL_SEL, 001: Data_Odd
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_DA_O);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_DA_O);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_DA_O);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_DA_O);
		
	
	hdmi_mask(HDMIRX_PHY_B2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN);
	hdmi_mask(HDMIRX_PHY_G2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN);
	hdmi_mask(HDMIRX_PHY_R2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN);

	//K DCVS DATA_ODD
	lib_hdmi_do_koffset_dcvs_cal(nport,lane_mode);

//////////////   do_koffset_dcvs_cal: DCVS EDGE_EVEN    ////////////////////
		HDMI_PRINTF("[ K DCVS EDGE_EVEN Offset]\n");
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_EG_E);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_EG_E);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_EG_E);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_EG_E);


	//K DCVS EDGE_EVEN
	lib_hdmi_do_koffset_dcvs_cal(nport,lane_mode);

//////////////   do_koffset_dcvs_cal: DCVS EDGE_ODD    ////////////////////
		HDMI_PRINTF("[ K DCVS EDGE_ODD Offset]\n");
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_EG_O);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_EG_O);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_EG_O);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_EG_O);
	

	//K DCVS EDGE_ODD
	lib_hdmi_do_koffset_dcvs_cal(nport,lane_mode);

//////////////////
#if 0
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_DA_E);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_DA_E);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_DA_E);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_DA_E);
	

	//K DCVS DATA_EVEN
	lib_hdmi_do_koffset_dcvs_cal(nport,lane_mode);
#endif
/////////////////////////


//	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);
//	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);
//	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);
//	if (lane_mode == HDMI_4LANE)
//		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);

	// cut input on
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_1_inputoff),0);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_1_inputoff),0);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_1_inputoff),0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_1_inputoff),0);

}
#else


void lib_hdmi21_fg_koffset_auto(unsigned char nport,unsigned char lane_mode)
{
	hdmi_mask(HDMIRX_PHY_R2_reg, ~(b_9_DFE_TAP_EN), 0);//Disable TAP_EN
	hdmi_mask(HDMIRX_PHY_G2_reg, ~(b_9_DFE_TAP_EN), 0);//Disable TAP_EN
	hdmi_mask(HDMIRX_PHY_B2_reg, ~(b_9_DFE_TAP_EN), 0);//Disable TAP_EN
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK2_reg, ~(b_9_DFE_TAP_EN), 0);//Disable TAP_EN
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_3_FORE_EN_FORE_KOFF|b_1_OFFCAL_SEL), 0);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_3_FORE_EN_FORE_KOFF|b_1_OFFCAL_SEL), 0);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_3_FORE_EN_FORE_KOFF|b_1_OFFCAL_SEL), 0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_3_FORE_EN_FORE_KOFF|b_1_OFFCAL_SEL), 0);

	//EQ power disable
	hdmi_mask(HDMIRX_PHY_B4_reg, ~(EQ1_2PTAT_pow|EQ1_1_and_EQ1_2PTAT_pow), 0x0);
	hdmi_mask(HDMIRX_PHY_G4_reg, ~(EQ1_2PTAT_pow|EQ1_1_and_EQ1_2PTAT_pow) , 0x0);
	hdmi_mask(HDMIRX_PHY_R4_reg, ~(EQ1_2PTAT_pow|EQ1_1_and_EQ1_2PTAT_pow), 0x0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK4_reg, ~(EQ1_2PTAT_pow|EQ1_1_and_EQ1_2PTAT_pow), 0x0);
	hdmi_mask(HDMIRX_PHY_B1_reg, ~(b_6_EQ_POW), 0x0);
	hdmi_mask(HDMIRX_PHY_G1_reg, ~(g_6_EQ_POW) , 0x0);
	hdmi_mask(HDMIRX_PHY_R1_reg, ~(r_6_EQ_POW), 0x0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK1_reg, ~(ck_6_EQ_POW), 0x0);

	//DCVS disable
	hdmi_mask(HDMIRX_PHY_B2_reg, ~(b_10_DCVS_EN), 0);
	hdmi_mask(HDMIRX_PHY_G2_reg, ~(b_10_DCVS_EN), 0);
	hdmi_mask(HDMIRX_PHY_R2_reg, ~(b_10_DCVS_EN), 0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK2_reg, ~(b_10_DCVS_EN), 0);

	// cut input off
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_1_inputoff),b_1_inputoff);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_1_inputoff),b_1_inputoff);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_1_inputoff),b_1_inputoff);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_1_inputoff),b_1_inputoff);

	// initial circuit power
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_3_FORE_EN_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_1_OFFCAL_SEL),(b_3_FORE_EN_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_1_OFFCAL_DA_E));
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_3_FORE_EN_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_1_OFFCAL_SEL),(b_3_FORE_EN_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_1_OFFCAL_DA_E));
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_3_FORE_EN_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_1_OFFCAL_SEL),(b_3_FORE_EN_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_1_OFFCAL_DA_E));
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_3_FORE_EN_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_1_OFFCAL_SEL),(b_3_FORE_EN_FORE_KOFF|b_4_FORE_KOFF_AUTOK|b_1_OFFCAL_DA_E));


//////////////   do_koffset_dcvs_cal: DCVS DATA_EVEN    ////////////////////
		HDMI_PRINTF("[ K DCVS DATA_EVEN Offset]\n");
	hdmi_mask(HDMIRX_PHY_B2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN);
	hdmi_mask(HDMIRX_PHY_G2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN);
	hdmi_mask(HDMIRX_PHY_R2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN);

        // init koffset range
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_KOFF_RANGE),0x0);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(g_4_FORE_KOFF_RANGE),0x0);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(r_4_FORE_KOFF_RANGE),0x0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(ck_4_FORE_KOFF_RANGE),0x0);

	
	//K DCVS DATA_EVEN
	lib_hdmi_do_koffset_dcvs_cal(nport,lane_mode);
	

	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);

//////////////////do_koffset_eq_cal: K EQ Offset   /////////////////////////
//EQ power enable
		HDMI_PRINTF("[ K EQ Offset] \n");
	if (phy_st[nport].clk > 3000)//>3G
	{
		hdmi_mask(HDMIRX_PHY_B4_reg, ~(EQ1_2PTAT_pow|EQ1_1_and_EQ1_2PTAT_pow),(EQ1_2PTAT_pow|EQ1_1_and_EQ1_2PTAT_pow));
		hdmi_mask(HDMIRX_PHY_G4_reg, ~(EQ1_2PTAT_pow|EQ1_1_and_EQ1_2PTAT_pow) ,(EQ1_2PTAT_pow|EQ1_1_and_EQ1_2PTAT_pow));
		hdmi_mask(HDMIRX_PHY_R4_reg, ~(EQ1_2PTAT_pow|EQ1_1_and_EQ1_2PTAT_pow), (EQ1_2PTAT_pow|EQ1_1_and_EQ1_2PTAT_pow));
		if (lane_mode == HDMI_4LANE)
			hdmi_mask(HDMIRX_PHY_CK4_reg, ~(EQ1_2PTAT_pow|EQ1_1_and_EQ1_2PTAT_pow), (EQ1_2PTAT_pow|EQ1_1_and_EQ1_2PTAT_pow));
	}	
//EQ power enable
	hdmi_mask(HDMIRX_PHY_B1_reg, ~(b_6_EQ_POW), b_6_EQ_POW);
	hdmi_mask(HDMIRX_PHY_G1_reg, ~(g_6_EQ_POW) ,g_6_EQ_POW);
	hdmi_mask(HDMIRX_PHY_R1_reg, ~(r_6_EQ_POW), r_6_EQ_POW);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK1_reg, ~(ck_6_EQ_POW),ck_6_EQ_POW);


	hdmi_mask(HDMIRX_PHY_B2_reg, ~(b_10_DCVS_EN), 0);
	hdmi_mask(HDMIRX_PHY_G2_reg, ~(b_10_DCVS_EN), 0);
	hdmi_mask(HDMIRX_PHY_R2_reg, ~(b_10_DCVS_EN), 0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK2_reg, ~(b_10_DCVS_EN), 0);


	//KEQ
	lib_hdmi_do_koffset_eq_cal(nport,lane_mode);


//////////////   do_koffset_dcvs_cal: DCVS DATA_ODD    ////////////////////
		HDMI_PRINTF("[ K DCVS DATA_ODD Offset]\n");
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_DA_O);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_DA_O);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_DA_O);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_DA_O);
		
	
	hdmi_mask(HDMIRX_PHY_B2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN);
	hdmi_mask(HDMIRX_PHY_G2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN);
	hdmi_mask(HDMIRX_PHY_R2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK2_reg, ~(b_10_DCVS_EN), b_10_DCVS_EN);

	//K DCVS DATA_ODD
	lib_hdmi_do_koffset_dcvs_cal(nport,lane_mode);

//////////////   do_koffset_dcvs_cal: DCVS EDGE_EVEN    ////////////////////
		HDMI_PRINTF("[ K DCVS EDGE_EVEN Offset]\n");
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_EG_E);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_EG_E);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_EG_E);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_EG_E);


	//K DCVS EDGE_EVEN
	lib_hdmi_do_koffset_dcvs_cal(nport,lane_mode);

//////////////   do_koffset_dcvs_cal: DCVS EDGE_ODD    ////////////////////
		HDMI_PRINTF("[ K DCVS EDGE_ODD Offset]\n");
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_EG_O);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_EG_O);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_EG_O);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_EG_O);
	

	//K DCVS EDGE_ODD
	lib_hdmi_do_koffset_dcvs_cal(nport,lane_mode);

//////////////   do_koffset_dcvs_cal: DCVS DATA_EVEN    ////////////////////
		HDMI_PRINTF("[ K DCVS DATA_EVEN Offset]\n");
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_DA_E);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_DA_E);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_DA_E);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF|b_1_OFFCAL_SEL),b_1_OFFCAL_DA_E);
	

	//K DCVS DATA_EVEN
	lib_hdmi_do_koffset_dcvs_cal(nport,lane_mode);


//	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);
//	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);
//	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);
//	if (lane_mode == HDMI_4LANE)
//		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_4_FORE_POW_FORE_KOFF),0);

	// cut input on
	hdmi_mask(HDMIRX_PHY_B0_reg, ~(b_1_inputoff),0);
	hdmi_mask(HDMIRX_PHY_G0_reg, ~(b_1_inputoff),0);
	hdmi_mask(HDMIRX_PHY_R0_reg, ~(b_1_inputoff),0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK0_reg, ~(b_1_inputoff),0);

	hdmi_mask(HDMIRX_PHY_R2_reg, ~(b_9_DFE_TAP_EN), HD21_RX_PHY_P0_b_tap_en(0xf));//Enable TAP_EN TAP4~TAP1
	hdmi_mask(HDMIRX_PHY_G2_reg, ~(b_9_DFE_TAP_EN), HD21_RX_PHY_P0_b_tap_en(0xf));//Enable TAP_EN TAP4~TAP1
	hdmi_mask(HDMIRX_PHY_B2_reg, ~(b_9_DFE_TAP_EN), HD21_RX_PHY_P0_b_tap_en(0xf));//Enable TAP_EN TAP4~TAP1
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(HDMIRX_PHY_CK2_reg, ~(b_9_DFE_TAP_EN),  HD21_RX_PHY_P0_b_tap_en(0xf));//Enable TAP_EN TAP4~TAP1
}

#endif
#else
void lib_hdmi21_fg_koffset_auto(unsigned char port,unsigned char lane_mode)
{
#define FORE_KOFF_RANGE  1  // range maybe 0  1  2   3
	UINT8 i, k_cnt;
	UINT32 w_b_addr_control,w_g_addr_control,w_r_addr_control,w_ck_addr_control;
	UINT32 w_koffset_addr_result;
	volatile u_int32_t koff_r_result;
	volatile u_int32_t  koff_g_result;
	volatile u_int32_t  koff_b_result;
	volatile u_int32_t koff_ck_result;

	if (port == HDMI_PORT0) {
		w_b_addr_control = HDMIRX_2P1_PHY_P0_B0_reg;
		w_g_addr_control = HDMIRX_2P1_PHY_P0_G0_reg;
		w_r_addr_control = HDMIRX_2P1_PHY_P0_R0_reg;
		w_ck_addr_control = HDMIRX_2P1_PHY_P0_CK0_reg;
		w_koffset_addr_result = HDMIRX_2P1_PHY_P0_KOFFSET_reg;

	} else if (port == HDMI_PORT1) {
		w_b_addr_control = HDMIRX_2P1_PHY_P1_B0_reg;
		w_g_addr_control = HDMIRX_2P1_PHY_P1_G0_reg;
		w_r_addr_control = HDMIRX_2P1_PHY_P1_R0_reg;
		w_r_addr_control = HDMIRX_2P1_PHY_P1_CK0_reg;
		w_koffset_addr_result = HDMIRX_2P1_PHY_P1_KOFFSET_reg;

	}  else {
		return;
	}


	// initial circuit power
	hdmi_mask(w_b_addr_control, ~(P0_b_3_FORE_EN_FORE_KOFF|P0_b_4_FORE_KOFF_AUTOK|P0_b_4_FORE_POW_FORE_KOFF),(P0_b_3_FORE_EN_FORE_KOFF|P0_b_4_FORE_KOFF_AUTOK));
	hdmi_mask(w_g_addr_control, ~(P0_b_3_FORE_EN_FORE_KOFF|P0_b_4_FORE_KOFF_AUTOK|P0_b_4_FORE_POW_FORE_KOFF),(P0_b_3_FORE_EN_FORE_KOFF|P0_b_4_FORE_KOFF_AUTOK));
	hdmi_mask(w_r_addr_control, ~(P0_b_3_FORE_EN_FORE_KOFF|P0_b_4_FORE_KOFF_AUTOK|P0_b_4_FORE_POW_FORE_KOFF),(P0_b_3_FORE_EN_FORE_KOFF|P0_b_4_FORE_KOFF_AUTOK));
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(w_ck_addr_control, ~(P0_b_3_FORE_EN_FORE_KOFF|P0_b_4_FORE_KOFF_AUTOK|P0_b_4_FORE_POW_FORE_KOFF),(P0_b_3_FORE_EN_FORE_KOFF|P0_b_4_FORE_KOFF_AUTOK));
		
	// cut input off
	hdmi_mask(w_b_addr_control, ~(P0_b_1_inputoff),P0_b_1_inputoff);
	hdmi_mask(w_g_addr_control, ~(P0_b_1_inputoff),P0_b_1_inputoff);
	hdmi_mask(w_r_addr_control, ~(P0_b_1_inputoff),P0_b_1_inputoff);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(w_ck_addr_control, ~(P0_b_1_inputoff),P0_b_1_inputoff);

	//for b lane
	for (i=0; i<FORE_KOFF_RANGE+1; i++) {
		hdmi_mask(w_b_addr_control, ~(P0_b_4_FORE_POW_FORE_KOFF),0);

		//set k off range
		hdmi_mask(w_b_addr_control, ~(P0_b_4_FORE_KOFF_RANGE),i<<29);

		// start cal
		hdmi_mask(w_b_addr_control, ~(P0_b_4_FORE_POW_FORE_KOFF),P0_b_4_FORE_POW_FORE_KOFF);

		//wait finish cal
		k_cnt = 100;
		while (!((hdmi_in(w_koffset_addr_result))&HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_ok_b_mask) && --k_cnt) {
			udelay(10);
		}

		if (!k_cnt) {
			HDMI_EMG("[ERROR] %s: func err(B)\n", __func__);
		}

		koff_b_result = HDMIRX_2P1_PHY_P0_KOFFSET_get_reg_p0_fore_off_tst_b(hdmi_in(w_koffset_addr_result));

		if (!(hdmi_in(w_koffset_addr_result) & HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_bound_b_mask)) {
			//HDMI_PRINTF("[HDMI] R channel foreground K Off result   %x  range =%x \n",koff_b_result,i);
			break;
		} else {

			if (i == FORE_KOFF_RANGE)
				HDMI_EMG("%s: out of B range (value=%d)\n", __func__,i);

		}
	}


	//for g lane
	for (i=0; i<FORE_KOFF_RANGE+1; i++) {
		hdmi_mask(w_g_addr_control, ~(P0_b_4_FORE_POW_FORE_KOFF),0);

		//set k off range
		hdmi_mask(w_g_addr_control, ~(P0_b_4_FORE_KOFF_RANGE),i<<29);

		// start cal
		hdmi_mask(w_g_addr_control, ~(P0_b_4_FORE_POW_FORE_KOFF),P0_b_4_FORE_POW_FORE_KOFF);

		//wait finish cal
		k_cnt = 100;
		while (!((hdmi_in(w_koffset_addr_result))&HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_ok_g_mask) && --k_cnt) {
			udelay(10);
		}

		if (!k_cnt) {
			HDMI_EMG("[ERROR] %s: func err(G)\n", __func__);
		}

		koff_g_result = HDMIRX_2P1_PHY_P0_KOFFSET_get_reg_p0_fore_off_tst_g(hdmi_in(w_koffset_addr_result));

		if (!(hdmi_in(w_koffset_addr_result) & HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_bound_g_mask)) {
			break;
		} else {
			if (i == FORE_KOFF_RANGE)
				HDMI_EMG("%s: out of G range (value=%d)\n", __func__,i);

		}
	}


	//for r lane
	for (i=0; i<FORE_KOFF_RANGE+1; i++) {
		hdmi_mask(w_r_addr_control, ~(P0_b_4_FORE_POW_FORE_KOFF),0);

		//set k off range
		hdmi_mask(w_r_addr_control, ~(P0_b_4_FORE_KOFF_RANGE),i<<29);

		// start cal
		hdmi_mask(w_r_addr_control, ~(P0_b_4_FORE_POW_FORE_KOFF),P0_b_4_FORE_POW_FORE_KOFF);

		//wait finish cal
		k_cnt = 100;
		while(!((hdmi_in(w_koffset_addr_result))&HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_ok_r_mask) && --k_cnt) {
			udelay(10);
		}

		if (!k_cnt) {
			HDMI_EMG("[ERROR] %s: func err(R)\n", __func__);
		}

		koff_r_result = HDMIRX_2P1_PHY_P0_KOFFSET_get_reg_p0_fore_off_tst_r(hdmi_in(w_koffset_addr_result));

		if (!(hdmi_in(w_koffset_addr_result) & HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_bound_r_mask)) {
			break;
		} else {
			if (i == FORE_KOFF_RANGE)
				HDMI_EMG("%s: out of R range (value=%d)\n", __func__,i);

		}
	}

	//for ck lane
	if (lane_mode == HDMI_4LANE) {
		for (i=0; i<FORE_KOFF_RANGE+1; i++) {
			hdmi_mask(w_ck_addr_control, ~(P0_b_4_FORE_POW_FORE_KOFF),0);

			//set k off range
			hdmi_mask(w_ck_addr_control, ~(P0_b_4_FORE_KOFF_RANGE),i<<29);

			// start cal
			hdmi_mask(w_ck_addr_control, ~(P0_b_4_FORE_POW_FORE_KOFF),P0_b_4_FORE_POW_FORE_KOFF);

			//wait finish cal
			k_cnt = 100;
			while(!((hdmi_in(w_koffset_addr_result))&HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_ok_ck_mask) && --k_cnt) {
				udelay(10);
			}

			if (!k_cnt) {
				HDMI_EMG("[ERROR] %s: func err(CK)\n", __func__);
			}

			koff_ck_result = HDMIRX_2P1_PHY_P0_KOFFSET_get_reg_p0_fore_off_tst_ck(hdmi_in(w_koffset_addr_result));

			if (!(hdmi_in(w_koffset_addr_result) & HDMIRX_2P1_PHY_P0_KOFFSET_reg_p0_fore_off_bound_ck_mask)) {
				break;
			} else {
				if (i == FORE_KOFF_RANGE)
					HDMI_EMG("%s: out of CK range (value=%d)\n", __func__,i);
			}
		}

	}


	// cut input on
	hdmi_mask(w_b_addr_control, ~(P0_b_1_inputoff),0);
	hdmi_mask(w_g_addr_control, ~(P0_b_1_inputoff),0);
	hdmi_mask(w_r_addr_control, ~(P0_b_1_inputoff),0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(w_ck_addr_control, ~(P0_b_1_inputoff),0);

	if (lane_mode == HDMI_4LANE)
		HDMI_PRINTF("FG KOFFSET=(b=%d,g=%d,r=%d,ck=%d)\n", koff_b_result,koff_g_result,koff_r_result,koff_ck_result);
	else
		HDMI_PRINTF("FG KOFFSET=(b=%d,g=%d,r=%d)\n", koff_b_result,koff_g_result,koff_r_result);

}
#endif


void lib_hdmi21_fg_koffset_proc(unsigned char nport, unsigned char lane_mode)
{

	lib_hdmi21_fg_koffset_auto(nport,lane_mode);
	lib_hdmi21_cdr_rst(nport, 1, MODE_TMDS,lane_mode);
	udelay(5);
	lib_hdmi21_cdr_rst(nport, 0, MODE_TMDS,lane_mode);
	udelay(50);
}

//0xB80381E0
//B80381E0[31:0]: z0 value 4 port
//port0_z0=B80381E0[4:0] port0_done=B80381E0[5]
//port1_z0=B80381E0[10:6] port1_done=B80381E0[11]
//port2_z0=B80381E0[16:12] port2_done=B80381E0[17]
//port3_z0=B80381E0[22:18] port3_done=B80381E0[23]

extern bool AP_SET_HDMI_ext_trace_z0; 
extern  unsigned char ext_trace_z0; 

void lib_hdmi21_z0_calibration(void)
{
	 #define Z0_DIFF_ERR		0x0
        unsigned int z0_otp_value = rtd_inl(EFUSE_DATAO_120_reg);
	 
	 unsigned char z0_otp_done;
        unsigned char z0_0, z0_1;    //z0 for 2 port
	 unsigned char z0_low_bound_0,z0_low_bound_1;     //  85 -- 100 -- 115  +-15%
	 unsigned char z0_high_bound_0,z0_high_bound_1;    //high boundary z0 for all port
	 unsigned char ext_trace_z0_0,ext_trace_z0_1; //external z0 for 3 port
	 unsigned char soc_z0 = HD21_PHY_Z0;
	 unsigned char target_z0_0,target_z0_1;     // target_z0 = soc_z0 + pcb_trace_z0  (target z0 for all port)
	 unsigned long long val = 0;    

	 ext_trace_z0_0 = ext_trace_z0_1 = (0x5 - Z0_DIFF_ERR);  // 1 step 1.5 ohm  5.4 ohm = 4 step
	 z0_otp_done = ((z0_otp_value & 0x00000020) >> 5) | ((z0_otp_value & 0x00000800) >> 10) | ((z0_otp_value & 0x00020000) >> 15) | ((z0_otp_value & 0x00800000) >> 20);

	 
	  if (AP_SET_HDMI_ext_trace_z0==TRUE) {
		ext_trace_z0_0 = ext_trace_z0- Z0_DIFF_ERR;
		ext_trace_z0_1 = ext_trace_z0- Z0_DIFF_ERR;
	  }else   if (pcb_mgr_get_enum_info_byname("HDMI_EXT_Z0_CFG", &val)==0) {
		ext_trace_z0_0 = (val & 0x000000FF)- Z0_DIFF_ERR;
		ext_trace_z0_1 = ((val & 0x0000FF00) >> 8) - Z0_DIFF_ERR;
	  } else {
			HDMI_PRINTF("NOT GET  EXTERNAL PCB TRACE Z0\n");
	  }
	  
	HDMI_PRINTF("soc_z0=%x p0_ext_trace_z0=%x  p1_ext_trace_z0=%x\n",soc_z0, ext_trace_z0_0,ext_trace_z0_1);
	
	 target_z0_0 = soc_z0 + ext_trace_z0_0;
	 target_z0_1 = soc_z0 + ext_trace_z0_1;

	 
	 z0_high_bound_0 = target_z0_0 + 10;
	 z0_high_bound_1 = target_z0_1 + 10;
	 

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

     

	HDMI_PRINTF("z0_0=%x z0_1=%x\n",z0_0,z0_1);

        hdmi_mask(HDMIRX_2P1_PHY_Z0K_reg, ~(HDMIRX_2P1_PHY_Z0K_reg_z0_adjr_0_mask|HDMIRX_2P1_PHY_Z0K_reg_z0_adjr_1_mask),
                (HDMIRX_2P1_PHY_Z0K_reg_z0_adjr_0(z0_0)|HDMIRX_2P1_PHY_Z0K_reg_z0_adjr_1(z0_1)));

}

//[pad no. port2], B lane ldo sel: [2:0], G lane ldo sel: [5:3], R lane ldo sel: [8:6], CK lane ldo sel: [11:9]
//[pad no. port3], B lane ldo sel: [14:12], G lane ldo sel: [17:15], R lane ldo sel: [20:18], CK lane ldo sel: [23:21]
//[done bit]: [24]
void lib_hdmi21_ldo_calibration(void)
{
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
}



#if 0
void lib_hdmi21_single_end(unsigned char nport, unsigned char channel, unsigned char p_off, unsigned char n_off)
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


// 1: clk lane  0: not clk lane
int lib_clk_lane_high_width_measure(unsigned char nport)
{
	unsigned char  i, cnt;
	unsigned int num_0[3],num_1[3];
	int max_num_1, min_num_1;
	int max_num_0;
	unsigned int x = 0,offset_n = 0;
	unsigned timeout_cnt = 256;
	static unsigned int test = 0;


	HDMI_EMG("lib_clk_lane_high_width_measure\n");

	if (test == 0) {
	//force FRL 6G to open CDR CLK and EQ  before use transition detect
		//newbase_hdmi21_set_phy_eq(HD21_PORT, FRL_6G,MODE_FRL_6G_4_LANE,HDMI_4LANE);
		newbase_hdmi21_set_phy(HD21_PORT, FRL_6G,MODE_FRL_6G_4_LANE,HDMI_4LANE);
		lib_hdmi_set_dfe_close(HD21_PORT,HDMI_4LANE);
		//test = test+1;
	}
	HDMI_EMG("test=%d\n",test);
	
	
	num_0[0] = num_0[1] = num_0[2] = 0;
	num_1[0] = num_1[1] = num_1[2] = 0;
	
	hdmi_mask(HD21_CLK_HWM_SETTING_00_reg,~(HDMI21_P0_HD21_CLK_HWM_SETTING_00_one_time_n_mask),HDMI21_P0_HD21_CLK_HWM_SETTING_00_one_time_n(0x3FF));
	hdmi_mask(HD21_CLK_HWM_SETTING_02_reg,~(HDMI21_P0_HD21_CLK_HWM_SETTING_02_tr_weight_mask),HDMI21_P0_HD21_CLK_HWM_SETTING_02_tr_weight(0x2));


	//do one time mode 3 times
	for ( i = 0; i < 3 ; ++i) {
		hdmi_mask(HD21_CLK_HWM_SETTING_00_reg,~(HDMI21_P0_HD21_CLK_HWM_SETTING_00_one_time_en_mask),HDMI21_P0_HD21_CLK_HWM_SETTING_00_one_time_en_mask);	
		cnt = 255;
		while (!((hdmi_in(HD21_CLK_HWN_ONE_ST_00_reg))&HDMI21_P0_HD21_CLK_HWN_ONE_ST_00_one_time_done_flag_mask) && --cnt) {
				udelay(10);
		}
		if (cnt == 0) {
			HDMI_EMG("clk measure timeout\n");
			break;
		}
		num_0[i] = HDMI21_P0_HD21_CLK_HWN_ONE_ST_00_get_one_time_0_num(hdmi_in(HD21_CLK_HWN_ONE_ST_00_reg)); //max number 0
		num_1[i] = HDMI21_P0_HD21_CLK_HWN_ONE_ST_00_get_one_time_1_num(hdmi_in(HD21_CLK_HWN_ONE_ST_00_reg)); //max number 1
		hdmi_mask(HD21_CLK_HWN_ONE_ST_00_reg,~(HDMI21_P0_HD21_CLK_HWN_ONE_ST_00_one_time_done_flag_mask),HDMI21_P0_HD21_CLK_HWN_ONE_ST_00_one_time_done_flag_mask); //clear done flag
	}

	max_num_1 = MAX(num_1[0],MAX(num_1[1], num_1[2]));
	min_num_1 = MIN(num_1[0],MIN(num_1[1], num_1[2]));
	max_num_0 = MAX(num_0[0],MAX(num_0[1], num_0[2]));

	HDMI_EMG("num_0 %d %d %d\n",num_0[0],num_0[1],num_0[2]);
	HDMI_EMG("num_1 %d %d %d\n",num_1[0],num_1[1],num_1[2]);
	HDMI_EMG("max_num_1=%d min_num_1=%d %d\n",max_num_1,min_num_1,0<<1);
	x = (max_num_1 + min_num_1) / 2;

	offset_n = 5;

	if ((x == 0) || (x <= offset_n))
		return 0;
	if (HDMI_ABS(max_num_0 , max_num_1) >= 5) {
		HDMI_EMG("max_num_1 and max_num_0 over range...\n");
		return 0;
	}

	HDMI_EMG("x=%d offset_n=%d\n",x,offset_n);

	hdmi_mask(HD21_CLK_HWM_SETTING_01_reg,~(HDMI21_P0_HD21_CLK_HWM_SETTING_01_cont_x_mask), HDMI21_P0_HD21_CLK_HWM_SETTING_01_cont_x(x));
	hdmi_mask(HD21_CLK_HWM_SETTING_01_reg,~(HDMI21_P0_HD21_CLK_HWM_SETTING_01_cont_offset_n_mask), HDMI21_P0_HD21_CLK_HWM_SETTING_01_cont_offset_n(offset_n));
	hdmi_mask(HD21_CLK_HWM_SETTING_02_reg,~(HDMI21_P0_HD21_CLK_HWM_SETTING_02_cont_err_th_mask),HDMI21_P0_HD21_CLK_HWM_SETTING_02_cont_err_th(0xFF));	

	//enable continue mode
	hdmi_mask(HD21_CLK_HWM_SETTING_01_reg,~(HDMI21_P0_HD21_CLK_HWM_SETTING_01_cont_en_mask),HDMI21_P0_HD21_CLK_HWM_SETTING_01_cont_en_mask);

	while(--timeout_cnt) {
		if (HDMI21_P0_HD21_CLK_HWN_CONT_ST_01_get_cont_err_ov_flag(hdmi_in(HD21_CLK_HWN_CONT_ST_01_reg))) {
		HDMI_EMG("err cnt =%d\n",HDMI21_P0_HD21_CLK_HWN_CONT_ST_01_get_cont_mode_err_cnt(HD21_CLK_HWN_CONT_ST_01_reg));
			hdmi_mask(HD21_CLK_HWM_SETTING_01_reg,~(HDMI21_P0_HD21_CLK_HWM_SETTING_01_cont_en_mask),0x0);
			hdmi_mask(HD21_CLK_HWN_CONT_ST_01_reg,~(HDMI21_P0_HD21_CLK_HWN_CONT_ST_01_cont_err_ov_flag_mask),HDMI21_P0_HD21_CLK_HWN_CONT_ST_01_cont_err_ov_flag_mask);
			HDMI_EMG("timeout_cnt=%d\n",timeout_cnt);
		return 0;
	}
		msleep(20);
	}
	
	//disable continue mode
	hdmi_mask(HD21_CLK_HWM_SETTING_01_reg,~(HDMI21_P0_HD21_CLK_HWM_SETTING_01_cont_en_mask),0x0);
	hdmi_mask(HD21_CLK_HWN_CONT_ST_01_reg,~(HDMI21_P0_HD21_CLK_HWN_CONT_ST_01_cont_err_ov_flag_mask),HDMI21_P0_HD21_CLK_HWN_CONT_ST_01_cont_err_ov_flag_mask);

	return 1;
}


#if 0
//dphy output sel to mac
void lib_hdmi_out_port_sel(unsigned char nport)
{

	if (nport == 0) {
		hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg,~HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg_port_out_sel_mask,0x0);
	} else {
		hdmi_mask(HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg,~HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg_port_out_sel_mask,HDMIRX_2P1_PHY_GLOBAL_HD21_Y1_REGD30_reg_port_out_sel_mask);
	}

}
#endif


void lib_hdmi21_fld_ck_en(unsigned char nport, unsigned char en,unsigned char frl_mode,unsigned char lane_mode)
{	
	if (en) {
		if (frl_mode == MODE_TMDS) {
			hdmi_mask(HDMIRX_PHY_ACDR_CK0_reg,~acdr_b_1_FLD_CK_EN,acdr_b_1_FLD_CK_EN);
		} else {
		
		hdmi_mask(HDMIRX_PHY_ACDR_B0_reg,~acdr_b_1_FLD_CK_EN,acdr_b_1_FLD_CK_EN);
		hdmi_mask(HDMIRX_PHY_ACDR_G0_reg,~acdr_b_1_FLD_CK_EN,acdr_b_1_FLD_CK_EN);
		hdmi_mask(HDMIRX_PHY_ACDR_R0_reg,~acdr_b_1_FLD_CK_EN,acdr_b_1_FLD_CK_EN);

		if (lane_mode == HDMI_4LANE)
			hdmi_mask(HDMIRX_PHY_ACDR_CK0_reg,~acdr_b_1_FLD_CK_EN,acdr_b_1_FLD_CK_EN);
		else
			hdmi_mask(HDMIRX_PHY_ACDR_CK0_reg,~acdr_b_1_FLD_CK_EN,0x0);
		}
	} else {
		hdmi_mask(HDMIRX_PHY_ACDR_B0_reg,~acdr_b_1_FLD_CK_EN,0x0);
		hdmi_mask(HDMIRX_PHY_ACDR_G0_reg,~acdr_b_1_FLD_CK_EN,0x0);
		hdmi_mask(HDMIRX_PHY_ACDR_R0_reg,~acdr_b_1_FLD_CK_EN,0x0);
		hdmi_mask(HDMIRX_PHY_ACDR_CK0_reg,~acdr_b_1_FLD_CK_EN,0x0);
	}


}

void lib_hdmi21_fld_reset(unsigned char nport, unsigned char rst, unsigned char frl_mode,unsigned char lane_mode)
{
	if (rst) {
		if (frl_mode == MODE_TMDS) {
			hdmi_mask(HDMIRX_PHY_FLD_CK_REGD04_reg,~HDMIRX_2P1_PHY_P0_FLD_CK_REGD04_p0_ck_fld_rstb_mask,HDMIRX_2P1_PHY_P0_FLD_CK_REGD04_p0_ck_fld_rstb_mask);
		} else {
			hdmi_mask(HDMIRX_PHY_FLD_R_REGD04_reg,~HDMIRX_2P1_PHY_P0_FLD_R_REGD04_p0_r_fld_rstb_mask,HDMIRX_2P1_PHY_P0_FLD_R_REGD04_p0_r_fld_rstb_mask);
			hdmi_mask(HDMIRX_PHY_FLD_G_REGD04_reg,~HDMIRX_2P1_PHY_P0_FLD_G_REGD04_p0_g_fld_rstb_mask,HDMIRX_2P1_PHY_P0_FLD_G_REGD04_p0_g_fld_rstb_mask);
			hdmi_mask(HDMIRX_PHY_FLD_B_REGD04_reg,~HDMIRX_2P1_PHY_P0_FLD_B_REGD04_p0_b_fld_rstb_mask,HDMIRX_2P1_PHY_P0_FLD_B_REGD04_p0_b_fld_rstb_mask);
			if (lane_mode == HDMI_4LANE)
				hdmi_mask(HDMIRX_PHY_FLD_CK_REGD04_reg,~HDMIRX_2P1_PHY_P0_FLD_CK_REGD04_p0_ck_fld_rstb_mask,HDMIRX_2P1_PHY_P0_FLD_CK_REGD04_p0_ck_fld_rstb_mask);
			else
				hdmi_mask(HDMIRX_PHY_FLD_CK_REGD04_reg,~HDMIRX_2P1_PHY_P0_FLD_CK_REGD04_p0_ck_fld_rstb_mask,0x0);
		}
	} else {
		hdmi_mask(HDMIRX_PHY_FLD_R_REGD04_reg,~HDMIRX_2P1_PHY_P0_FLD_R_REGD04_p0_r_fld_rstb_mask,0x0);
		hdmi_mask(HDMIRX_PHY_FLD_G_REGD04_reg,~HDMIRX_2P1_PHY_P0_FLD_G_REGD04_p0_g_fld_rstb_mask,0x0);
		hdmi_mask(HDMIRX_PHY_FLD_B_REGD04_reg,~HDMIRX_2P1_PHY_P0_FLD_B_REGD04_p0_b_fld_rstb_mask,0x0);
		hdmi_mask(HDMIRX_PHY_FLD_CK_REGD04_reg,~HDMIRX_2P1_PHY_P0_FLD_CK_REGD04_p0_ck_fld_rstb_mask,0x0);
	}
}

void lib_hdmi21_acdr_power_en(unsigned char nport , unsigned char en, unsigned char frl_mode,unsigned char lane_mode)
{
	if (en) {
		if (frl_mode == MODE_TMDS) {
			hdmi_mask(HDMIRX_PHY_MISC_0_reg,~HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_b_acdr_en_afn_mask,0x0);
			hdmi_mask(HDMIRX_PHY_MISC_0_reg,~HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_g_acdr_en_afn_mask,0x0);
			hdmi_mask(HDMIRX_PHY_MISC_0_reg,~HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_r_acdr_en_afn_mask,0x0);
			hdmi_mask(HDMIRX_PHY_MISC_0_reg,~HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_ck_acdr_en_afn_mask,HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_ck_acdr_en_afn_mask);
		}else {
		
		hdmi_mask(HDMIRX_PHY_MISC_0_reg,~HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_b_acdr_en_afn_mask,HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_b_acdr_en_afn_mask);
		hdmi_mask(HDMIRX_PHY_MISC_0_reg,~HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_g_acdr_en_afn_mask,HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_g_acdr_en_afn_mask);
		hdmi_mask(HDMIRX_PHY_MISC_0_reg,~HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_r_acdr_en_afn_mask,HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_r_acdr_en_afn_mask);
		
		if (lane_mode == HDMI_4LANE) {
			hdmi_mask(HDMIRX_PHY_MISC_0_reg,~HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_ck_acdr_en_afn_mask,HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_ck_acdr_en_afn_mask);
		} else {
			hdmi_mask(HDMIRX_PHY_MISC_0_reg,~HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_ck_acdr_en_afn_mask,0x0);
		}
		}
	} else {
		hdmi_mask(HDMIRX_PHY_MISC_0_reg,~HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_b_acdr_en_afn_mask,0x0);
		hdmi_mask(HDMIRX_PHY_MISC_0_reg,~HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_g_acdr_en_afn_mask,0x0);
		hdmi_mask(HDMIRX_PHY_MISC_0_reg,~HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_r_acdr_en_afn_mask,0x0);
		hdmi_mask(HDMIRX_PHY_MISC_0_reg,~HDMIRX_2P1_PHY_P0_MISC_0_p0_acdr_ck_acdr_en_afn_mask,0x0);

	}

}


void lib_hdmi21_acdr_finetune_en(unsigned char nport , unsigned char en, unsigned char lane_mode)
{
	if (en) {
		hdmi_mask(HDMIRX_PHY_FLD_R_REGD04_reg,~HDMIRX_2P1_PHY_P0_FLD_R_REGD04_p0_r_acdr_fine_tune_start_mask,HDMIRX_2P1_PHY_P0_FLD_R_REGD04_p0_r_acdr_fine_tune_start_mask);
		hdmi_mask(HDMIRX_PHY_FLD_G_REGD04_reg,~HDMIRX_2P1_PHY_P0_FLD_G_REGD04_p0_g_acdr_fine_tune_start_mask,HDMIRX_2P1_PHY_P0_FLD_G_REGD04_p0_g_acdr_fine_tune_start_mask);
		hdmi_mask(HDMIRX_PHY_FLD_B_REGD04_reg,~HDMIRX_2P1_PHY_P0_FLD_B_REGD04_p0_b_acdr_fine_tune_start_mask,HDMIRX_2P1_PHY_P0_FLD_B_REGD04_p0_b_acdr_fine_tune_start_mask);
		if (lane_mode == HDMI_4LANE)
			hdmi_mask(HDMIRX_PHY_FLD_CK_REGD04_reg,~HDMIRX_2P1_PHY_P0_FLD_CK_REGD04_p0_ck_acdr_fine_tune_start_mask,HDMIRX_2P1_PHY_P0_FLD_CK_REGD04_p0_ck_acdr_fine_tune_start_mask);
		else
			hdmi_mask(HDMIRX_PHY_FLD_CK_REGD04_reg,~HDMIRX_2P1_PHY_P0_FLD_CK_REGD04_p0_ck_acdr_fine_tune_start_mask,0x0);
	} else {
		hdmi_mask(HDMIRX_PHY_FLD_R_REGD04_reg,~HDMIRX_2P1_PHY_P0_FLD_R_REGD04_p0_r_acdr_fine_tune_start_mask,0x0);
		hdmi_mask(HDMIRX_PHY_FLD_G_REGD04_reg,~HDMIRX_2P1_PHY_P0_FLD_G_REGD04_p0_g_acdr_fine_tune_start_mask,0x0);
		hdmi_mask(HDMIRX_PHY_FLD_B_REGD04_reg,~HDMIRX_2P1_PHY_P0_FLD_B_REGD04_p0_b_acdr_fine_tune_start_mask,0x0);
		hdmi_mask(HDMIRX_PHY_FLD_CK_REGD04_reg,~HDMIRX_2P1_PHY_P0_FLD_CK_REGD04_p0_ck_acdr_fine_tune_start_mask,0x0);
	}

}


void lib_hdmi21_bbpd_reset(unsigned char nport, unsigned char en, unsigned char lane_mode)
{

	if (en) {
		hdmi_mask(HDMIRX_PHY_R0_reg,~r_4_BBPD_RSTB,r_4_BBPD_RSTB);
		hdmi_mask(HDMIRX_PHY_G0_reg,~g_4_BBPD_RSTB,g_4_BBPD_RSTB);
		hdmi_mask(HDMIRX_PHY_B0_reg,~b_4_BBPD_RSTB,b_4_BBPD_RSTB);
		if (lane_mode == HDMI_4LANE)
			hdmi_mask(HDMIRX_PHY_CK0_reg,~ck_4_BBPD_RSTB,ck_4_BBPD_RSTB);
		else
			hdmi_mask(HDMIRX_PHY_CK0_reg,~ck_4_BBPD_RSTB,0x0);
	} else {
		hdmi_mask(HDMIRX_PHY_R0_reg,~r_4_BBPD_RSTB,0x0);
		hdmi_mask(HDMIRX_PHY_G0_reg,~g_4_BBPD_RSTB,0x0);
		hdmi_mask(HDMIRX_PHY_B0_reg,~b_4_BBPD_RSTB,0x0);
		hdmi_mask(HDMIRX_PHY_CK0_reg,~ck_4_BBPD_RSTB,0x0);
	}

}


void lib_hdmi21_cdr_mode_config(unsigned char nport,unsigned char mode)
{
	if (mode == ACDR_MODE) {
		hdmi_mask(HDMIRX_PHY_FLD_R_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_auto_mode_mask|HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_adap_eq_off_mask),HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_auto_mode(1)|HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_adap_eq_off(0));
		hdmi_mask(HDMIRX_PHY_FLD_G_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_G_REGD00_reg_p0_g_auto_mode_mask|HDMIRX_2P1_PHY_P0_FLD_G_REGD00_reg_p0_g_adap_eq_off_mask),HDMIRX_2P1_PHY_P0_FLD_G_REGD00_reg_p0_g_auto_mode(1)|HDMIRX_2P1_PHY_P0_FLD_G_REGD00_reg_p0_g_adap_eq_off(0));
		hdmi_mask(HDMIRX_PHY_FLD_B_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_B_REGD00_reg_p0_b_auto_mode_mask|HDMIRX_2P1_PHY_P0_FLD_B_REGD00_reg_p0_b_adap_eq_off_mask),HDMIRX_2P1_PHY_P0_FLD_B_REGD00_reg_p0_b_auto_mode(1)|HDMIRX_2P1_PHY_P0_FLD_B_REGD00_reg_p0_b_adap_eq_off(0));
		hdmi_mask(HDMIRX_PHY_FLD_CK_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_CK_REGD00_reg_p0_ck_auto_mode_mask|HDMIRX_2P1_PHY_P0_FLD_CK_REGD00_reg_p0_ck_adap_eq_off_mask),HDMIRX_2P1_PHY_P0_FLD_CK_REGD00_reg_p0_ck_auto_mode(1)|HDMIRX_2P1_PHY_P0_FLD_CK_REGD00_reg_p0_ck_adap_eq_off(0));

	} else {
		hdmi_mask(HDMIRX_PHY_FLD_R_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_auto_mode_mask|HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_cp2adp_en_mask),HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_auto_mode(0)|HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_cp2adp_en(0));
		hdmi_mask(HDMIRX_PHY_FLD_G_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_G_REGD00_reg_p0_g_auto_mode_mask|HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_cp2adp_en_mask),HDMIRX_2P1_PHY_P0_FLD_G_REGD00_reg_p0_g_auto_mode(0)|HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_cp2adp_en(0));
		hdmi_mask(HDMIRX_PHY_FLD_B_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_B_REGD00_reg_p0_b_auto_mode_mask|HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_cp2adp_en_mask),HDMIRX_2P1_PHY_P0_FLD_B_REGD00_reg_p0_b_auto_mode(0)|HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_cp2adp_en(0));
		hdmi_mask(HDMIRX_PHY_FLD_CK_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_CK_REGD00_reg_p0_ck_auto_mode_mask|HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_cp2adp_en_mask),HDMIRX_2P1_PHY_P0_FLD_CK_REGD00_reg_p0_ck_auto_mode(0)|HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_cp2adp_en(0));

		hdmi_mask(HDMIRX_PHY_FLD_R_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_adp_en_manual_mask),HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_adp_en_manual(1));
		hdmi_mask(HDMIRX_PHY_FLD_G_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_G_REGD00_reg_p0_g_adp_en_manual_mask),HDMIRX_2P1_PHY_P0_FLD_G_REGD00_reg_p0_g_adp_en_manual(1));
		hdmi_mask(HDMIRX_PHY_FLD_B_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_B_REGD00_reg_p0_b_adp_en_manual_mask),HDMIRX_2P1_PHY_P0_FLD_B_REGD00_reg_p0_b_adp_en_manual(1));
		hdmi_mask(HDMIRX_PHY_FLD_CK_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_CK_REGD00_reg_p0_ck_adp_en_manual_mask),HDMIRX_2P1_PHY_P0_FLD_CK_REGD00_reg_p0_ck_adp_en_manual(1));

	}

}


void lib_hdmi21_cdr_init(unsigned char nport, unsigned frl_mode,unsigned char lane_mode)
{
	//adp time 6us  calib time 5us  (rdivider num check after)
	hdmi_mask(HDMIRX_PHY_FLD_R_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_adp_time_mask|HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_calib_time_mask),HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_adp_time(ADP_CNT)|HDMIRX_2P1_PHY_P0_FLD_R_REGD00_reg_p0_r_calib_time(CALIB_CNT));
	hdmi_mask(HDMIRX_PHY_FLD_G_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_G_REGD00_reg_p0_g_adp_time_mask|HDMIRX_2P1_PHY_P0_FLD_G_REGD00_reg_p0_g_calib_time_mask),HDMIRX_2P1_PHY_P0_FLD_G_REGD00_reg_p0_g_adp_time(ADP_CNT)|HDMIRX_2P1_PHY_P0_FLD_G_REGD00_reg_p0_g_calib_time(CALIB_CNT));
	hdmi_mask(HDMIRX_PHY_FLD_B_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_B_REGD00_reg_p0_b_adp_time_mask|HDMIRX_2P1_PHY_P0_FLD_B_REGD00_reg_p0_b_calib_time_mask),HDMIRX_2P1_PHY_P0_FLD_B_REGD00_reg_p0_b_adp_time(ADP_CNT)|HDMIRX_2P1_PHY_P0_FLD_B_REGD00_reg_p0_b_calib_time(CALIB_CNT));
	hdmi_mask(HDMIRX_PHY_FLD_CK_REGD00_reg,~(HDMIRX_2P1_PHY_P0_FLD_CK_REGD00_reg_p0_ck_adp_time_mask|HDMIRX_2P1_PHY_P0_FLD_CK_REGD00_reg_p0_ck_calib_time_mask),HDMIRX_2P1_PHY_P0_FLD_CK_REGD00_reg_p0_ck_adp_time(0x9)|HDMIRX_2P1_PHY_P0_FLD_CK_REGD00_reg_p0_ck_calib_time(CALIB_CNT));

	if (frl_mode == MODE_TMDS)
		lib_hdmi21_bbpd_reset(nport,0,lane_mode);  //dcdr mode
	else
		lib_hdmi21_bbpd_reset(nport,1,lane_mode);


	if (frl_mode == MODE_TMDS)
		lib_hdmi21_cdr_mode_config(nport,DCDR_MODE);
	else {
		lib_hdmi21_cdr_mode_config(nport,ACDR_MODE);
	}

}



//fld_st
//init state:0x0  check freq:0x1  freq_coarse_tune:0x13  coarse_tune_done:0x15 acdr_fine_tune:0x16 
//adp_enable:0x4
//calib_ok & track_freq 0x5

//0x16->0x5

int lib_hdmi21_is_acdr_state(unsigned char nport, unsigned char lane)
{
	unsigned char cnt = 0;
	unsigned char st_r,st_g,st_b,st_c;
	unsigned char timeout = 25;
	
	if (lane == HDMI_3LANE) {
		for (cnt = 0 ; cnt < timeout ; ++cnt) { 
			st_r = HDMIRX_2P1_PHY_P0_FLD_R_REGD06_get_p0_r_fld_st_reg(hdmi_in(HDMIRX_PHY_FLD_R_REGD06_reg));
			st_g = HDMIRX_2P1_PHY_P0_FLD_G_REGD06_get_p0_g_fld_st_reg(hdmi_in(HDMIRX_PHY_FLD_G_REGD06_reg));
			st_b = HDMIRX_2P1_PHY_P0_FLD_B_REGD06_get_p0_b_fld_st_reg(hdmi_in(HDMIRX_PHY_FLD_B_REGD06_reg));
			if ((st_r == 5) && (st_g == 5) && (st_b == 5))
				break;
			udelay(10);
		}
	} else {
		for (cnt = 0 ; cnt < timeout ; ++cnt)  {
			st_r = HDMIRX_2P1_PHY_P0_FLD_R_REGD06_get_p0_r_fld_st_reg(hdmi_in(HDMIRX_PHY_FLD_R_REGD06_reg));
			st_g = HDMIRX_2P1_PHY_P0_FLD_G_REGD06_get_p0_g_fld_st_reg(hdmi_in(HDMIRX_PHY_FLD_G_REGD06_reg));
			st_b = HDMIRX_2P1_PHY_P0_FLD_B_REGD06_get_p0_b_fld_st_reg(hdmi_in(HDMIRX_PHY_FLD_B_REGD06_reg));
			st_c = HDMIRX_2P1_PHY_P0_FLD_CK_REGD06_get_p0_ck_fld_st_reg(hdmi_in(HDMIRX_PHY_FLD_CK_REGD06_reg));
			if ((st_r == 5) && (st_g == 5) && (st_b == 5) && (st_c == 5))
				break;
			udelay(10);
		}
	}

	if (cnt == timeout) {
		HDMI_EMG("track_freq fail , acdr state timeout\n");
		return 0;
	} else {
		return 1;
	}
		
}


int lib_hdmi21_acdr_calib_status(unsigned char nport, unsigned char lane, unsigned char value)
{
	unsigned char cnt = 0;
	unsigned char st_r,st_g,st_b,st_c;
	unsigned char timeout = 20;
	
	if (lane == HDMI_3LANE) {
		for (cnt = 0 ; cnt < timeout ; ++cnt) { 
			st_r = HDMIRX_2P1_PHY_P0_FLD_R_REGD06_get_p0_r_calib_ok(hdmi_in(HDMIRX_PHY_FLD_R_REGD06_reg));
			st_g = HDMIRX_2P1_PHY_P0_FLD_G_REGD06_get_p0_g_calib_ok(hdmi_in(HDMIRX_PHY_FLD_G_REGD06_reg));
			st_b = HDMIRX_2P1_PHY_P0_FLD_B_REGD06_get_p0_b_calib_ok(hdmi_in(HDMIRX_PHY_FLD_B_REGD06_reg));
			if ((st_r == value) && (st_g == value) && (st_b == value))
				break;
			udelay(5);
		}
	} else {
		for (cnt = 0 ; cnt < timeout ; ++cnt)  {
			st_r = HDMIRX_2P1_PHY_P0_FLD_R_REGD06_get_p0_r_calib_ok(hdmi_in(HDMIRX_PHY_FLD_R_REGD06_reg));
			st_g = HDMIRX_2P1_PHY_P0_FLD_G_REGD06_get_p0_g_calib_ok(hdmi_in(HDMIRX_PHY_FLD_G_REGD06_reg));
			st_b = HDMIRX_2P1_PHY_P0_FLD_B_REGD06_get_p0_b_calib_ok(hdmi_in(HDMIRX_PHY_FLD_B_REGD06_reg));
			st_c = HDMIRX_2P1_PHY_P0_FLD_CK_REGD06_get_p0_ck_calib_ok(hdmi_in(HDMIRX_PHY_FLD_CK_REGD06_reg));
			if ((st_r == value) && (st_g == value) && (st_b == value) && (st_c == value))
				break;
			udelay(5);
		}
	}

	if (cnt == timeout) {
		HDMI_EMG("calib timeout\n");
		return 0;
	} else {
		return 1;
	}

}




void lib_hdmi21_dfe_param_init(unsigned char nport, unsigned int b, unsigned char dfe_mode, unsigned char lane_mode)
{
	HDMI_PRINTF("%s\n", __func__);

	lib_hdmi21_dfe_power(nport, 1,lane_mode);
	
	hdmi_mask(DFE_REG_DFE_MODE_reg,
		~(DFE_P0_REG_DFE_MODE_P0_adapt_mode_p0_mask|DFE_P0_REG_DFE_MODE_P0_edge_det_mode_p0_mask|
		DFE_P0_REG_DFE_MODE_P0_transition_only_p0_mask|DFE_P0_REG_DFE_MODE_P0_tap1_delay_p0_mask|
		DFE_P0_REG_DFE_MODE_P0_tap24_delay_p0_mask|DFE_P0_REG_DFE_MODE_P0_le_delay_p0_mask|
		DFE_P0_REG_DFE_MODE_P0_servo_delay_p0_mask|DFE_P0_REG_DFE_MODE_P0_gray_en_p0_mask|
		DFE_P0_REG_DFE_MODE_P0_dfe_adapt_rstb_p0_mask|
		DFE_P0_REG_DFE_MODE_P0_tap0_notrans_p0_mask),
		(DFE_P0_REG_DFE_MODE_P0_adapt_mode_p0(dfe_mode)|DFE_P0_REG_DFE_MODE_P0_edge_det_mode_p0(0)|
		DFE_P0_REG_DFE_MODE_P0_transition_only_p0(0)|DFE_P0_REG_DFE_MODE_P0_tap1_delay_p0(0)|
		DFE_P0_REG_DFE_MODE_P0_tap24_delay_p0(0)|DFE_P0_REG_DFE_MODE_P0_le_delay_p0(1)|
		DFE_P0_REG_DFE_MODE_P0_servo_delay_p0(0)|DFE_P0_REG_DFE_MODE_P0_gray_en_p0(0xBF)|
		DFE_P0_REG_DFE_MODE_P0_tap0_notrans_p0_mask));

	if ((10000 == b) || ( 8000 == b)) 
	{
		hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(DFE_P0_REG_DFE_EN_L0_P0_clk_inv_lane0_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_timer_ctrl_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_reduce_adapt_gain_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_rl_det_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_rl_threshold_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_dfe_adapt_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_le_min_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_tap4_init_lane2_p0_mask),(DFE_P0_REG_DFE_EN_L2_P0_timer_ctrl_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_reduce_adapt_gain_lane2_p0(0))|DFE_P0_REG_DFE_EN_L2_P0_rl_threshold_lane2_p0(1)|DFE_P0_REG_DFE_EN_L2_P0_le_min_lane2_p0(0x1e)|DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0(0xf));
		hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(DFE_P0_REG_DFE_EN_L1_P0_clk_inv_lane1_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_timer_ctrl_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_reduce_adapt_gain_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_rl_det_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_rl_threshold_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_dfe_adapt_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_le_min_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_tap4_init_lane2_p0_mask),(DFE_P0_REG_DFE_EN_L2_P0_timer_ctrl_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_reduce_adapt_gain_lane2_p0(0))|DFE_P0_REG_DFE_EN_L2_P0_rl_threshold_lane2_p0(1)|DFE_P0_REG_DFE_EN_L2_P0_le_min_lane2_p0(0x1e)|DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0(0xf));
		hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(DFE_P0_REG_DFE_EN_L2_P0_clk_inv_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_timer_ctrl_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_reduce_adapt_gain_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_rl_det_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_rl_threshold_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_dfe_adapt_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_le_min_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_tap4_init_lane2_p0_mask),(DFE_P0_REG_DFE_EN_L2_P0_timer_ctrl_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_reduce_adapt_gain_lane2_p0(0))|DFE_P0_REG_DFE_EN_L2_P0_rl_threshold_lane2_p0(1)|DFE_P0_REG_DFE_EN_L2_P0_le_min_lane2_p0(0x1e)|DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0(0xf));
		if (lane_mode == HDMI_4LANE)
			hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(DFE_P0_REG_DFE_EN_L3_P0_clk_inv_lane3_p0_mask|DFE_P0_REG_DFE_EN_L3_P0_timer_ctrl_en_lane3_p0_mask|DFE_P0_REG_DFE_EN_L3_P0_reduce_adapt_gain_lane3_p0_mask|DFE_P0_REG_DFE_EN_L3_P0_rl_det_en_lane3_p0_mask|DFE_P0_REG_DFE_EN_L3_P0_rl_threshold_lane3_p0_mask|DFE_P0_REG_DFE_EN_L3_P0_dfe_adapt_en_lane3_p0_mask|DFE_P0_REG_DFE_EN_L3_P0_le_min_lane3_p0_mask|DFE_P0_REG_DFE_EN_L3_P0_le_init_lane3_p0_mask|DFE_P0_REG_DFE_EN_L3_P0_tap4_init_lane3_p0_mask),(DFE_P0_REG_DFE_EN_L3_P0_timer_ctrl_en_lane3_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_reduce_adapt_gain_lane2_p0(0))|DFE_P0_REG_DFE_EN_L2_P0_rl_threshold_lane2_p0(1)|DFE_P0_REG_DFE_EN_L2_P0_le_min_lane2_p0(0x1e)|DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0(0xf));
	}
	else
	{
		hdmi_mask(DFE_REG_DFE_EN_L0_reg,~(DFE_P0_REG_DFE_EN_L0_P0_clk_inv_lane0_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_timer_ctrl_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_reduce_adapt_gain_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_rl_det_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_rl_threshold_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_dfe_adapt_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_le_min_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_tap4_init_lane2_p0_mask),(DFE_P0_REG_DFE_EN_L2_P0_timer_ctrl_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_reduce_adapt_gain_lane2_p0(0))|DFE_P0_REG_DFE_EN_L2_P0_rl_threshold_lane2_p0(1)|DFE_P0_REG_DFE_EN_L2_P0_le_min_lane2_p0(0x1e)|DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0(0x14));
		hdmi_mask(DFE_REG_DFE_EN_L1_reg,~(DFE_P0_REG_DFE_EN_L1_P0_clk_inv_lane1_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_timer_ctrl_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_reduce_adapt_gain_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_rl_det_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_rl_threshold_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_dfe_adapt_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_le_min_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_tap4_init_lane2_p0_mask),(DFE_P0_REG_DFE_EN_L2_P0_timer_ctrl_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_reduce_adapt_gain_lane2_p0(0))|DFE_P0_REG_DFE_EN_L2_P0_rl_threshold_lane2_p0(1)|DFE_P0_REG_DFE_EN_L2_P0_le_min_lane2_p0(0x1e)|DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0(0x14));
		hdmi_mask(DFE_REG_DFE_EN_L2_reg,~(DFE_P0_REG_DFE_EN_L2_P0_clk_inv_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_timer_ctrl_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_reduce_adapt_gain_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_rl_det_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_rl_threshold_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_dfe_adapt_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_le_min_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_tap4_init_lane2_p0_mask),(DFE_P0_REG_DFE_EN_L2_P0_timer_ctrl_en_lane2_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_reduce_adapt_gain_lane2_p0(0))|DFE_P0_REG_DFE_EN_L2_P0_rl_threshold_lane2_p0(1)|DFE_P0_REG_DFE_EN_L2_P0_le_min_lane2_p0(0x1e)|DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0(0x14));
		if (lane_mode == HDMI_4LANE)
			hdmi_mask(DFE_REG_DFE_EN_L3_reg,~(DFE_P0_REG_DFE_EN_L3_P0_clk_inv_lane3_p0_mask|DFE_P0_REG_DFE_EN_L3_P0_timer_ctrl_en_lane3_p0_mask|DFE_P0_REG_DFE_EN_L3_P0_reduce_adapt_gain_lane3_p0_mask|DFE_P0_REG_DFE_EN_L3_P0_rl_det_en_lane3_p0_mask|DFE_P0_REG_DFE_EN_L3_P0_rl_threshold_lane3_p0_mask|DFE_P0_REG_DFE_EN_L3_P0_dfe_adapt_en_lane3_p0_mask|DFE_P0_REG_DFE_EN_L3_P0_le_min_lane3_p0_mask|DFE_P0_REG_DFE_EN_L3_P0_le_init_lane3_p0_mask|DFE_P0_REG_DFE_EN_L3_P0_tap4_init_lane3_p0_mask),(DFE_P0_REG_DFE_EN_L3_P0_timer_ctrl_en_lane3_p0_mask|DFE_P0_REG_DFE_EN_L2_P0_reduce_adapt_gain_lane2_p0(0))|DFE_P0_REG_DFE_EN_L2_P0_rl_threshold_lane2_p0(1)|DFE_P0_REG_DFE_EN_L2_P0_le_min_lane2_p0(0x1e)|DFE_P0_REG_DFE_EN_L2_P0_le_init_lane2_p0(0x14));
	}

#if 0//ori
	hdmi_mask(DFE_REG_DFE_INIT0_L0_reg,
		~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0_mask),
		(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(0x0)|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(0xf)|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0(0x5)));
	hdmi_mask(DFE_REG_DFE_INIT0_L1_reg,
		~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0_mask),
		(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(0x0)|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(0xf)|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0(0x5)));
	hdmi_mask(DFE_REG_DFE_INIT0_L2_reg,
		~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0_mask),
		(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(0x0)|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(0xf)|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0(0x5)));
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_INIT0_L3_reg,
			~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0_mask),
			(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(0x0)|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(0xf)|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0(0x5)));

#else

	if (newbase_hdmi_get_longcable_flag(nport))//Set Vth 
	{
		hdmi_mask(DFE_REG_DFE_INIT0_L0_reg,
			~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0_mask),
			(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(0x0)|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(0xf)|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0(Vth_value)));
		hdmi_mask(DFE_REG_DFE_INIT0_L1_reg,
			~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0_mask),
			(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(0x0)|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(0xf)|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0(Vth_value)));
		hdmi_mask(DFE_REG_DFE_INIT0_L2_reg,
			~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0_mask),
			(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(0x0)|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(0xf)|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0(Vth_value)));
		if (lane_mode == HDMI_4LANE)
			hdmi_mask(DFE_REG_DFE_INIT0_L3_reg,
				~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0_mask),
				(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(0x0)|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(0xf)|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0(Vth_value)));
	}
	else
	{
		hdmi_mask(DFE_REG_DFE_INIT0_L0_reg,
			~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0_mask),
			(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(0x0)|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(0xf)|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0(0x5)));
		hdmi_mask(DFE_REG_DFE_INIT0_L1_reg,
			~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0_mask),
			(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(0x0)|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(0xf)|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0(0x5)));
		hdmi_mask(DFE_REG_DFE_INIT0_L2_reg,
			~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0_mask),
			(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(0x0)|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(0xf)|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0(0x5)));
		if (lane_mode == HDMI_4LANE)
			hdmi_mask(DFE_REG_DFE_INIT0_L3_reg,
				~(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0_mask|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0_mask),
				(DFE_P0_REG_DFE_INIT0_L0_P0_tap1_init_lane0_p0(0x0)|DFE_P0_REG_DFE_INIT0_L0_P0_tap0_init_lane0_p0(0xf)|DFE_P0_REG_DFE_INIT0_L0_P0_vth_init_lane0_p0(0x5)));
	}
#endif
	//cloud teset
	hdmi_mask(DFE_REG_DFE_INIT1_L0_reg,
	~(DFE_P0_REG_DFE_INIT1_L0_P0_leq2_trans_mode_lane0_p0_mask|DFE_P0_REG_DFE_INIT1_L0_P0_leq1_trans_mode_lane0_p0_mask),
	DFE_P0_REG_DFE_INIT1_L0_P0_leq2_trans_mode_lane0_p0(0x1)|DFE_P0_REG_DFE_INIT1_L0_P0_leq1_trans_mode_lane0_p0(0x1));
	hdmi_mask(DFE_REG_DFE_INIT1_L1_reg,
		~(DFE_P0_REG_DFE_INIT1_L0_P0_leq2_trans_mode_lane0_p0_mask|DFE_P0_REG_DFE_INIT1_L0_P0_leq1_trans_mode_lane0_p0_mask),
		DFE_P0_REG_DFE_INIT1_L0_P0_leq2_trans_mode_lane0_p0(0x1)|DFE_P0_REG_DFE_INIT1_L0_P0_leq1_trans_mode_lane0_p0(0x1));
	hdmi_mask(DFE_REG_DFE_INIT1_L2_reg,
		~(DFE_P0_REG_DFE_INIT1_L0_P0_leq2_trans_mode_lane0_p0_mask|DFE_P0_REG_DFE_INIT1_L0_P0_leq1_trans_mode_lane0_p0_mask),
		DFE_P0_REG_DFE_INIT1_L0_P0_leq2_trans_mode_lane0_p0(0x1)|DFE_P0_REG_DFE_INIT1_L0_P0_leq1_trans_mode_lane0_p0(0x1));
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_INIT1_L3_reg,
		~(DFE_P0_REG_DFE_INIT1_L0_P0_leq2_trans_mode_lane0_p0_mask|DFE_P0_REG_DFE_INIT1_L0_P0_leq1_trans_mode_lane0_p0_mask),
		DFE_P0_REG_DFE_INIT1_L0_P0_leq2_trans_mode_lane0_p0(0x1)|DFE_P0_REG_DFE_INIT1_L0_P0_leq1_trans_mode_lane0_p0(0x1));


	//load initial data
	hdmi_mask(DFE_REG_DFE_INIT1_L0_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask);
	hdmi_mask(DFE_REG_DFE_INIT1_L1_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask);
	hdmi_mask(DFE_REG_DFE_INIT1_L2_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_INIT1_L3_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask);

	udelay(1);

	hdmi_mask(DFE_REG_DFE_INIT1_L0_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),0);
	hdmi_mask(DFE_REG_DFE_INIT1_L1_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),0);
	hdmi_mask(DFE_REG_DFE_INIT1_L2_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),0);
	if (lane_mode == HDMI_4LANE)
		hdmi_mask(DFE_REG_DFE_INIT1_L3_reg,~(DFE_P0_REG_DFE_INIT1_L0_P0_load_in_init_lane0_p0_mask),0);


	//four lane adapt reset release
	hdmi_mask(DFE_REG_DFE_MODE_reg,~(DFE_P0_REG_DFE_MODE_P0_dfe_adapt_rstb_p0_mask),DFE_P0_REG_DFE_MODE_P0_dfe_adapt_rstb_p0_mask);
	//set run length = 4
	#if 0
	hdmi_mask(DFE_REG_DFE_GAIN_reg,
	~(DFE_P0_REG_DFE_GAIN_P0_le_gain1_p0_mask|DFE_P0_REG_DFE_GAIN_P0_le_gain2_p0_mask|DFE_P0_REG_DFE_GAIN_P0_le2_sign_p0_mask|DFE_P0_REG_DFE_GAIN_P0_le_gain_prec_p0_mask|DFE_P0_REG_DFE_GAIN_P0_trans_rlength_p0_mask),
	DFE_P0_REG_DFE_GAIN_P0_le_gain1_p0(0x0)|DFE_P0_REG_DFE_GAIN_P0_le_gain2_p0(0x0)|DFE_P0_REG_DFE_GAIN_P0_le2_sign_p0(0x1)|DFE_P0_REG_DFE_GAIN_P0_le_gain_prec_p0(0x6)|DFE_P0_REG_DFE_GAIN_P0_trans_rlength_p0(0x2));
	#else   //tap1~tap4 gain = 0
	hdmi_mask(DFE_REG_DFE_GAIN_reg,
	~(DFE_P0_REG_DFE_GAIN_P0_tap1_gain_p0_mask|DFE_P0_REG_DFE_GAIN_P0_tap2_gain_p0_mask|DFE_P0_REG_DFE_GAIN_P0_tap3_gain_p0_mask|DFE_P0_REG_DFE_GAIN_P0_tap4_gain_p0_mask|DFE_P0_REG_DFE_GAIN_P0_le_gain1_p0_mask|DFE_P0_REG_DFE_GAIN_P0_le_gain2_p0_mask|DFE_P0_REG_DFE_GAIN_P0_le2_sign_p0_mask|DFE_P0_REG_DFE_GAIN_P0_le_gain_prec_p0_mask|DFE_P0_REG_DFE_GAIN_P0_trans_rlength_p0_mask),
	DFE_P0_REG_DFE_GAIN_P0_le_gain1_p0(0x0)|DFE_P0_REG_DFE_GAIN_P0_le_gain2_p0(0x0)|DFE_P0_REG_DFE_GAIN_P0_le2_sign_p0(0x1)|DFE_P0_REG_DFE_GAIN_P0_le_gain_prec_p0(0x6)|DFE_P0_REG_DFE_GAIN_P0_trans_rlength_p0(0x2));

	#endif

	hdmi_mask(DFE_REG_DFE_LIMIT0_reg,
			~(DFE_P0_REG_DFE_LIMIT0_P0_tap1_min_p0_mask|DFE_P0_REG_DFE_LIMIT0_P0_tap2_max_p0_mask|DFE_P0_REG_DFE_LIMIT0_P0_tap2_min_p0_mask|DFE_P0_REG_DFE_LIMIT0_P0_tap3_max_p0_mask|DFE_P0_REG_DFE_LIMIT0_P0_tap3_min_p0_mask),
			DFE_P0_REG_DFE_LIMIT0_P0_tap1_min_p0(0x1f)|DFE_P0_REG_DFE_LIMIT0_P0_tap2_max_p0(0x1f)|DFE_P0_REG_DFE_LIMIT0_P0_tap2_min_p0(0x1f)|DFE_P0_REG_DFE_LIMIT0_P0_tap3_max_p0(0x1f)|DFE_P0_REG_DFE_LIMIT0_P0_tap3_min_p0(0x1f));

	//Vth_min=0,Vth_max=0x7,Vth_th=0,Tap0_th=1
#if 0
	hdmi_mask(DFE_REG_DFE_LIMIT1_reg,
			~(DFE_P0_REG_DFE_LIMIT1_P0_tap4_max_p0_mask|DFE_P0_REG_DFE_LIMIT1_P0_tap4_min_p0_mask|DFE_P0_REG_DFE_LIMIT1_P0_vth_min_p0_mask|DFE_P0_REG_DFE_LIMIT1_P0_vth_max_p0_mask|
			DFE_P0_REG_DFE_LIMIT1_P0_tap0_adjust_p0_mask|DFE_P0_REG_DFE_LIMIT1_P0_tap1_trans_p0_mask),
			DFE_P0_REG_DFE_LIMIT1_P0_tap4_max_p0(0x1f)|DFE_P0_REG_DFE_LIMIT1_P0_tap4_min_p0(0x1f)|DFE_P0_REG_DFE_LIMIT1_P0_vth_min_p0(1)|DFE_P0_REG_DFE_LIMIT1_P0_vth_max_p0(0x7)|DFE_P0_REG_DFE_LIMIT1_P0_tap0_adjust_p0(0x1)|DFE_P0_REG_DFE_LIMIT1_P0_tap1_trans_p0(0x0));
#else // tap1_trans_p0 0 ->1  For Test only by Garran 0722 
	hdmi_mask(DFE_REG_DFE_LIMIT1_reg,
			~(DFE_P0_REG_DFE_LIMIT1_P0_tap4_max_p0_mask|DFE_P0_REG_DFE_LIMIT1_P0_tap4_min_p0_mask|DFE_P0_REG_DFE_LIMIT1_P0_vth_min_p0_mask|DFE_P0_REG_DFE_LIMIT1_P0_vth_max_p0_mask|
			DFE_P0_REG_DFE_LIMIT1_P0_vth_threshold_p0_mask|DFE_P0_REG_DFE_LIMIT1_P0_tap0_threshold_p0_mask|DFE_P0_REG_DFE_LIMIT1_P0_tap0_adjust_p0_mask|DFE_P0_REG_DFE_LIMIT1_P0_tap1_trans_p0_mask),
			DFE_P0_REG_DFE_LIMIT1_P0_tap4_max_p0(0x1f)|DFE_P0_REG_DFE_LIMIT1_P0_tap4_min_p0(0x1f)|DFE_P0_REG_DFE_LIMIT1_P0_vth_min_p0(1)|DFE_P0_REG_DFE_LIMIT1_P0_vth_max_p0(0x7)|
			DFE_P0_REG_DFE_LIMIT1_P0_vth_threshold_p0(0x7)|DFE_P0_REG_DFE_LIMIT1_P0_tap0_threshold_p0(0x1)|DFE_P0_REG_DFE_LIMIT1_P0_tap0_adjust_p0(0x1)|DFE_P0_REG_DFE_LIMIT1_P0_tap1_trans_p0(0x1));

#endif


	hdmi_mask(DFE_REG_DFE_LIMIT2_reg, ~(DFE_P0_REG_DFE_LIMIT2_P0_tap0_max_p0_mask), DFE_P0_REG_DFE_LIMIT2_P0_tap0_max_p0(0x24));

}




int lib_hdmi21_acdr_is_unlock(unsigned char nport,unsigned char lane_mode)
{
	unsigned char unlock = 0;
	
	unlock =  unlock | HDMIRX_2P1_PHY_P0_FLD_R_REGD02_get_reg_p0_r_unstable_flg(hdmi_in(HDMIRX_PHY_FLD_R_REGD02_reg));
	unlock= unlock | (HDMIRX_2P1_PHY_P0_FLD_G_REGD02_get_reg_p0_g_unstable_flg(hdmi_in(HDMIRX_PHY_FLD_G_REGD02_reg)) << 1);
	unlock = unlock | (HDMIRX_2P1_PHY_P0_FLD_B_REGD02_get_reg_p0_b_unstable_flg(hdmi_in(HDMIRX_PHY_FLD_B_REGD02_reg)) << 2);
	if (lane_mode == HDMI_4LANE)
		unlock = unlock | (HDMIRX_2P1_PHY_P0_FLD_CK_REGD02_get_reg_p0_ck_unstable_flg(hdmi_in(HDMIRX_PHY_FLD_CK_REGD02_reg)) << 3);	

	if (unlock)
		return 1;
	else
		return 0;

}

int lib_hdmi21_coarse_calib_is_ok(unsigned char nport,unsigned char lane_mode)
{
	unsigned char st_r,st_g,st_b,st_c;
	unsigned char timeout = 30;
	
	if (lane_mode == HDMI_4LANE) {
		while(timeout --) {
	st_r = HDMIRX_2P1_PHY_P0_FLD_R_REGD06_get_p0_r_coarse_calib_ok(hdmi_in(HDMIRX_PHY_FLD_R_REGD06_reg));
	st_g = HDMIRX_2P1_PHY_P0_FLD_G_REGD06_get_p0_g_coarse_calib_ok(hdmi_in(HDMIRX_PHY_FLD_G_REGD06_reg));
	st_b = HDMIRX_2P1_PHY_P0_FLD_B_REGD06_get_p0_b_coarse_calib_ok(hdmi_in(HDMIRX_PHY_FLD_B_REGD06_reg));
	st_c = HDMIRX_2P1_PHY_P0_FLD_CK_REGD06_get_p0_ck_coarse_calib_ok(hdmi_in(HDMIRX_PHY_FLD_CK_REGD06_reg));

			if (st_r & st_g & st_b & st_c)
				break;
			udelay(10);
		}
	} else {
		while(timeout --) {
			st_r = HDMIRX_2P1_PHY_P0_FLD_R_REGD06_get_p0_r_coarse_calib_ok(hdmi_in(HDMIRX_PHY_FLD_R_REGD06_reg));
			st_g = HDMIRX_2P1_PHY_P0_FLD_G_REGD06_get_p0_g_coarse_calib_ok(hdmi_in(HDMIRX_PHY_FLD_G_REGD06_reg));
			st_b = HDMIRX_2P1_PHY_P0_FLD_B_REGD06_get_p0_b_coarse_calib_ok(hdmi_in(HDMIRX_PHY_FLD_B_REGD06_reg));
			
			if (st_r & st_g & st_b)
				break;
			udelay(10);
		}
	}

	if (timeout == 0) {
		HDMI_EMG("coarse calib timeout\n");
		return 0;
	} 
	
	if (lane_mode == HDMI_4LANE) {
		HDMI_EMG("coarse calbration (b,g,r,ck) = (%d %d %d %d)\n", st_b,st_g,st_r,st_c);
	} else {
		HDMI_EMG("coarse calbration (b,g,r) = (%d %d %d)\n", st_b,st_g,st_r);
	}


	return 1;
}


void lib_fast_swtch_mode(unsigned char nport,unsigned char mode)
{
	if (mode == PS_NORMAL_MODE) {
			//DELAY_CEL LDO_VSEL  KVCO
		if (nport == HDMI_PORT0) {
			hdmi_mask(HDMIRX_2P1_PHY_TOP_IN_1_reg, ~(TOP_IN_8_P0_FS_SIGNAL_DLY_TIME|TOP_IN_8_P0_FS_SIGNAL_DLY|TOP_IN_8_P0_FS_SIGNAL_SEL),TOP_IN_8_P0_FS_SIGNAL_DLY_TIME_100);
		}else if (nport == HDMI_PORT1) {
			hdmi_mask(HDMIRX_2P1_PHY_TOP_IN_1_reg, ~(TOP_IN_7_P1_FS_SIGNAL_DLY_TIME|TOP_IN_7_P1_FS_SIGNAL_DLY|TOP_IN_7_P1_FS_SIGNAL_SEL),TOP_IN_7_P1_FS_SIGNAL_DLY_TIME_100);
		} else {
			return;
		}
			
		hdmi_mask(HDMIRX_PHY_ACDR_B1_reg,~(acdr_b_8_FS_SIGNAL_DLY_EN|acdr_b_8_FS_EN), 0x0);
		hdmi_mask(HDMIRX_PHY_ACDR_G1_reg,~(acdr_g_8_FS_SIGNAL_DLY_EN|acdr_g_8_FS_EN), 0x0);
		hdmi_mask(HDMIRX_PHY_ACDR_R1_reg,~(acdr_r_8_FS_SIGNAL_DLY_EN|acdr_r_8_FS_EN), 0x0);
		hdmi_mask(HDMIRX_PHY_ACDR_CK1_reg,~(acdr_ck_8_FS_SIGNAL_DLY_EN|acdr_ck_8_FS_EN), 0x0);		
	} else {


	}


}


