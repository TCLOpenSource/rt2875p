/*=============================================================
 * File:    dprx_platform-rtk_tv.h
 *
 * Desc:    DPRX Platform Dependet API
 *
 * AUTHOR:  kevin_wang@realtek.com
 *
 * Vresion: 0.0.1
 *
 *------------------------------------------------------------
 * Copyright (c) Realtek Semiconductor Corporation, 2021
 *
 * All rights reserved.
 *
 *============================================================*/
#include <dprx_platform.h>
#include <mach/pcbMgr.h>

#ifndef __DPRX_PLATFORM_RTK_TV_H__
#define __DPRX_PLATFORM_RTK_TV_H__

#define PLAT_DPRX_PORT_NUM   2

/*-----------------------------------------------------------------------
 *   DP related configuration
 *-----------------------------------------------------------------------*/

/****************************************************************
 * Param   : DP_PORT_ORDER
 *
 * Desc    : This parameter is used to describe relation
 *           between logical DP channel (e.g. UI channel) and
 *           Physical DP port. if this pcb parameter absents,
 *           the DP driver will use 0x0021 as default value)
 *
 * Format  : D15:12  : DP4 PORT SEL : 0 : not used, 1~4: DP Port Config id 1~4, others reserved
 *           D11:8   : DP3 PORT SEL : 0 : not used, 1~4: DP Port Config id 1~4, others reserved
 *           D7:4    : DP2 PORT SEL : 0 : not used, 1~4: DP Port Config id 1~4, others reserved
 *           D3:0    : DP1 PORT SEL : 0 : not used, 1~4: DP Port Config id 1~4, others reserved
 *
 * Example : DP_PORT_ORDER = 0x1234 means
 *           DP CH1 = DP PORT4 (DP_PORT4_CFG)
 *           DP CH2 = DP PORT3 (DP_PORT3_CFG)
 *           DP CH3 = DP PORT2 (DP_PORT2_CFG)
 *           DP CH4 = DP PORT1 (DP_PORT1_CFG)
 *****************************************************************/
#define DP1_CH_SEL(x)         (x & 0xF)
#define DP2_CH_SEL(x)         ((x & 0xF)<<4)
#define DP3_CH_SEL(x)         ((x & 0xF)<<8)
#define DP4_CH_SEL(x)         ((x & 0xF)<<12)


/**********************DP_PORT0/1/2/3_CFG format************************
 *  D63:63  : DP_PORT_VALID        (0: invalid / 1 : valid)
 *  D62:60  : DP_PORT_TYPE         (0: invalid / 1 : valid)
 *  D59:0   : DP_PORT_SPECIFIC_DATA
 *****************************************************************/
#define DP_PORT_VALID(x)                ((unsigned long long)(x & 0x1)<<63)  // [63:63]
#define DP_PORT_VALID_MASK              DP_PORT_VALID(1)
#define GET_DP_PORT_VALID(x)            (x & DP_PORT_VALID_MASK)

#define DP_PORT_TYPE(x)                 ((unsigned long long)(x & 0x7)<<60)  // [62:60]
#define DP_PORT_TYPE_DP                 DP_PORT_TYPE(0)
#define DP_PORT_TYPE_mDP                DP_PORT_TYPE(1)
#define DP_PORT_TYPE_TYPE_C             DP_PORT_TYPE(2)
#define DP_PORT_TYPE_PSEUDO_HDMI_DP     DP_PORT_TYPE(3)
#define DP_PORT_TYPE_PSEUDO_DP          DP_PORT_TYPE(4)
#define DP_PORT_TYPE_MAX                DP_PORT_TYPE(5)
#define DP_PORT_TYPE_MASK               DP_PORT_TYPE(7)
#define GET_DP_PORT_TYPE(x)             (x & DP_PORT_TYPE_MASK)

//-----------------------------------------------------------------------------------
// DP_PORT_SPECIFIC_DATA [59:0] DP_PORT_TYPE = DP_PORT_TYPE_DP / DP_PORT_TYPE_mDP / DP_PORT_TYPE_TYPE_C
//-----------------------------------------------------------------------------------
// [59] EDID Type
#define DP_EDID_TYPE(x)                 ((unsigned long long)(x & 0x1)<<59)
#define DP_EDID_TYPE_INT_DDC            DP_EDID_TYPE(0)
#define DP_EDID_TYPE_EXT_EEPROM         DP_EDID_TYPE(1)
#define DP_EDID_TYPE_MASK               DP_EDID_TYPE(1)
#define GET_DP_EDID_TYPE(x)             (x & DP_EDID_TYPE_MASK)

// [58:56] DP_DDC_CH_SEL
#define DP_DDC_CH_SEL(x)                ((unsigned long long)(x & 0x7)<<56)
#define DP_DDC_CH_SEL_DDC0              DP_DDC_CH_SEL(0)
#define DP_DDC_CH_SEL_DDC1              DP_DDC_CH_SEL(1)
#define DP_DDC_CH_SEL_DDC2              DP_DDC_CH_SEL(2)
#define DP_DDC_CH_SEL_DDC3              DP_DDC_CH_SEL(3)
#define DP_DDC_CH_SEL_MASK              DP_DDC_CH_SEL(7)
#define GET_DP_DDC_CH_SEL(x)            ((x & DP_DDC_CH_SEL_MASK) >> 56)

// [55:54] AUX Channel ID
#define DP_AUX_CH_SEL(x)                ((unsigned long long)(x & 0x3)<<54)
#define DP_AUX_CH_SEL_AUX0              DP_AUX_CH_SEL(0)
#define DP_AUX_CH_SEL_AUX1              DP_AUX_CH_SEL(1)
#define DP_AUX_CH_SEL_MASK              DP_AUX_CH_SEL(3)
#define GET_DP_AUX_CH_SEL(x)            ((x & DP_AUX_CH_SEL_MASK) >> 54)

// [53:53] AUX PN SWAP
#define DP_AUX_PN_SWAP(x)               ((unsigned long long)(x & 0x1)<<53)
#define DP_AUX_PN_SWAP_OFF              DP_AUX_PN_SWAP(0)
#define DP_AUX_PN_SWAP_ON               DP_AUX_PN_SWAP(1)
#define DP_AUX_PN_SWAP_MASK             DP_AUX_PN_SWAP(1)
#define GET_DP_AUX_PN_SWAP(x)           ((x & DP_AUX_PN_SWAP_MASK)>>53)

// [49:48] DP_PHY_ID
#define DP_PHY_ID(x)                    ((unsigned long long)(x & 0x3)<<48)
#define DP_PHY_ID_PHY0                  DP_PHY_ID(0)
#define DP_PHY_ID_PHY1                  DP_PHY_ID(1)
#define DP_PHY_ID_MASK                  DP_PHY_ID(1)
#define GET_DP_PHY_ID(x)                ((x & DP_PHY_ID_MASK) >> 48)

// [47:46] DP_MAX_LANE_CNT
#define DP_MAX_LANE_CNT(x)              ((unsigned long long)(x & 0x3)<<46)
#define DP_MAX_LANE_CNT_1               DP_MAX_LANE_CNT(0)
#define DP_MAX_LANE_CNT_2               DP_MAX_LANE_CNT(1)
#define DP_MAX_LANE_CNT_4               DP_MAX_LANE_CNT(2)
#define DP_MAX_LANE_CNT_MASK            DP_MAX_LANE_CNT(3)
#define GET_MAX_LANE_CNT(x)             ((x & DP_MAX_LANE_CNT_MASK)>>46)

// [45:44] DP_LANE0_PHY_MAPPING
#define DP_LANE0_PHY_MAPPING(x)         ((unsigned long long)(x & 0x3)<<44)
#define DP_LANE0_PHY_MAPPING_0          DP_LANE0_PHY_MAPPING(0)
#define DP_LANE0_PHY_MAPPING_1          DP_LANE0_PHY_MAPPING(1)
#define DP_LANE0_PHY_MAPPING_2          DP_LANE0_PHY_MAPPING(2)
#define DP_LANE0_PHY_MAPPING_3          DP_LANE0_PHY_MAPPING(3)
#define DP_LANE0_PHY_MAPPING_MASK       DP_LANE0_PHY_MAPPING(3)
#define GET_DP_LANE0_PHY_MAPPING(x)     ((x & DP_LANE0_PHY_MAPPING_MASK) >> 44)

// [43:42] DP_LANE1_PHY_MAPPING
#define DP_LANE1_PHY_MAPPING(x)         ((unsigned long long)(x & 0x3)<<42)
#define DP_LANE1_PHY_MAPPING_0          DP_LANE1_PHY_MAPPING(0)
#define DP_LANE1_PHY_MAPPING_1          DP_LANE1_PHY_MAPPING(1)
#define DP_LANE1_PHY_MAPPING_2          DP_LANE1_PHY_MAPPING(2)
#define DP_LANE1_PHY_MAPPING_3          DP_LANE1_PHY_MAPPING(3)
#define DP_LANE1_PHY_MAPPING_MASK       DP_LANE1_PHY_MAPPING(3)
#define GET_DP_LANE1_PHY_MAPPING(x)     ((x & DP_LANE1_PHY_MAPPING_MASK) >> 42)

// [41:40] DP_LANE2_PHY_MAPPING
#define DP_LANE2_PHY_MAPPING(x)         ((unsigned long long)(x & 0x3)<<40)
#define DP_LANE2_PHY_MAPPING_0          DP_LANE2_PHY_MAPPING(0)
#define DP_LANE2_PHY_MAPPING_1          DP_LANE2_PHY_MAPPING(1)
#define DP_LANE2_PHY_MAPPING_2          DP_LANE2_PHY_MAPPING(2)
#define DP_LANE2_PHY_MAPPING_3          DP_LANE2_PHY_MAPPING(3)
#define DP_LANE2_PHY_MAPPING_MASK       DP_LANE2_PHY_MAPPING(3)
#define GET_DP_LANE2_PHY_MAPPING(x)     ((x & DP_LANE2_PHY_MAPPING_MASK)>>40)

// [39:38] DP_LANE3_PHY_MAPPING
#define DP_LANE3_PHY_MAPPING(x)         ((unsigned long long)(x & 0x3)<<38)
#define DP_LANE3_PHY_MAPPING_0          DP_LANE3_PHY_MAPPING(0)
#define DP_LANE3_PHY_MAPPING_1          DP_LANE3_PHY_MAPPING(1)
#define DP_LANE3_PHY_MAPPING_2          DP_LANE3_PHY_MAPPING(2)
#define DP_LANE3_PHY_MAPPING_3          DP_LANE3_PHY_MAPPING(3)
#define DP_LANE3_PHY_MAPPING_MASK       DP_LANE3_PHY_MAPPING(3)
#define GET_DP_LANE3_PHY_MAPPING(x)     ((x & DP_LANE3_PHY_MAPPING_MASK)>>38)

// [37:37] DP_LANE0_PN_SWAP
#define DP_LANE0_PN_SWAP(x)             ((unsigned long long)(x & 0x1)<<37)
#define DP_LANE0_PN_SWAP_OFF            DP_LANE0_PN_SWAP(0)
#define DP_LANE0_PN_SWAP_ON             DP_LANE0_PN_SWAP(1)
#define DP_LANE0_PN_SWAP_MASK           DP_LANE0_PN_SWAP(1)
#define GET_DP_LANE0_PN_SWAP(x)         ((x & DP_LANE0_PN_SWAP_MASK)>>37)

// [36:36] DP_LANE1_PN_SWAP
#define DP_LANE1_PN_SWAP(x)             ((unsigned long long)(x & 0x1)<<36)
#define DP_LANE1_PN_SWAP_OFF            DP_LANE1_PN_SWAP(0)
#define DP_LANE1_PN_SWAP_ON             DP_LANE1_PN_SWAP(1)
#define DP_LANE1_PN_SWAP_MASK           DP_LANE1_PN_SWAP(1)
#define GET_DP_LANE1_PN_SWAP(x)         ((x & DP_LANE1_PN_SWAP_MASK)>>36)

// [35:35] DP_LANE2_PN_SWAP
#define DP_LANE2_PN_SWAP(x)             ((unsigned long long)(x & 0x1)<<35)
#define DP_LANE2_PN_SWAP_OFF            DP_LANE2_PN_SWAP(0)
#define DP_LANE2_PN_SWAP_ON             DP_LANE2_PN_SWAP(1)
#define DP_LANE2_PN_SWAP_MASK           DP_LANE2_PN_SWAP(1)
#define GET_DP_LANE2_PN_SWAP(x)         ((x & DP_LANE2_PN_SWAP_MASK)>>35)

// [34:34] DP_LANE3_PN_SWAP
#define DP_LANE3_PN_SWAP(x)             ((unsigned long long)(x & 0x1)<<34)
#define DP_LANE3_PN_SWAP_OFF            DP_LANE3_PN_SWAP(0)
#define DP_LANE3_PN_SWAP_ON             DP_LANE3_PN_SWAP(1)
#define DP_LANE3_PN_SWAP_MASK           DP_LANE3_PN_SWAP(1)
#define GET_DP_LANE3_PN_SWAP(x)         ((x & DP_LANE3_PN_SWAP_MASK)>>34)

// [33:32] Reserved

// [31:30] DP_DET_GPIO_TYPE
#define DP_DET_GPIO_TYPE(x)             ((unsigned long long)(x & 0x3)<<30)
#define DP_DET_GPIO_TYPE_NONE           DP_DET_GPIO_TYPE(0)
#define DP_DET_GPIO_TYPE_MISC           DP_DET_GPIO_TYPE(1)
#define DP_DET_GPIO_TYPE_ISO            DP_DET_GPIO_TYPE(2)
#define DP_DET_GPIO_TYPE_MASK           DP_DET_GPIO_TYPE(3)
#define GET_DP_DET_GPIO_TYPE(x)         (x & DP_DET_GPIO_TYPE_MASK)

// [29:28] DP_AUX_P_GPIO_TYPE
#define DP_AUX_P_GPIO_TYPE(x)           ((unsigned long long)(x & 0x3)<<28)
#define DP_AUX_P_GPIO_TYPE_NONE         DP_AUX_P_GPIO_TYPE(0)
#define DP_AUX_P_GPIO_TYPE_MISC         DP_AUX_P_GPIO_TYPE(1)
#define DP_AUX_P_GPIO_TYPE_ISO          DP_AUX_P_GPIO_TYPE(2)
#define DP_AUX_P_GPIO_TYPE_MASK         DP_AUX_P_GPIO_TYPE(3)
#define GET_DP_AUX_P_GPIO_TYPE(x)       (x & DP_AUX_P_GPIO_TYPE_MASK)

// [27:26] DP_AUX_N_GPIO_TYPE
#define DP_AUX_N_GPIO_TYPE(x)           ((unsigned long long)(x & 0x3)<<26)
#define DP_AUX_N_GPIO_TYPE_NONE         DP_AUX_N_GPIO_TYPE(0)
#define DP_AUX_N_GPIO_TYPE_MISC         DP_AUX_N_GPIO_TYPE(1)
#define DP_AUX_N_GPIO_TYPE_ISO          DP_AUX_N_GPIO_TYPE(2)
#define DP_AUX_N_GPIO_TYPE_MASK         DP_AUX_N_GPIO_TYPE(3)
#define GET_DP_AUX_N_GPIO_TYPE(x)       (x & DP_AUX_N_GPIO_TYPE_MASK)

// [25:24] DP_HPD_GPIO_TYPE
#define DP_HPD_GPIO_TYPE(x)             ((unsigned long long)(x & 0x3)<<24)
#define DP_HPD_GPIO_TYPE_NONE           DP_HPD_GPIO_TYPE(0)
#define DP_HPD_GPIO_TYPE_MISC           DP_HPD_GPIO_TYPE(1)
#define DP_HPD_GPIO_TYPE_ISO            DP_HPD_GPIO_TYPE(2)
#define DP_HPD_GPIO_TYPE_MASK           DP_HPD_GPIO_TYPE(3)
#define GET_DP_HPD_GPIO_TYPE(x)         (x & DP_HPD_GPIO_TYPE_MASK)

// [23:18] DP_AUX_P_GPIO_ID
#define DP_DET_GPIO_ID(x)               ((unsigned long long)(x & 0x3F)<<18)
#define GET_DP_DET_GPIO_ID(x)           ((x>>18) & 0x3F)

// [17:12] DP_AUX_P_GPIO_ID
#define DP_AUX_P_GPIO_ID(x)             ((unsigned long long)(x & 0x3F)<<12)
#define GET_DP_AUX_P_GPIO_ID(x)         ((x>>12) & 0x3F)

// [11:6] DP_AUX_N_GPIO_ID
#define DP_AUX_N_GPIO_ID(x)             ((unsigned long long)(x & 0x3F)<<6)
#define GET_DP_AUX_N_GPIO_ID(x)         ((x>>6) & 0x3F)

// [5:0] DP_HPD_GPIO_ID
#define DP_HPD_GPIO_ID(x)               ((unsigned long long)(x & 0x3F))
#define GET_DP_HPD_GPIO_ID(x)           (x & 0x3F)

//-----------------------------------------------------------------------------------
// DP_PORT_SPECIFIC_DATA [59:0] DP_PORT_TYPE = DP_PORT_TYPE_TYPE_C
//-----------------------------------------------------------------------------------
// [59]    EDID Type        (as DP_PORT_TYPE = DP_PORT_TYPE_DP)
// [58:56] DDC_CH_SEL       (as DP_PORT_TYPE = DP_PORT_TYPE_DP)
// [55:54] AUX CH SEL       (as DP_PORT_TYPE = DP_PORT_TYPE_DP)
// [53:53] AUX PN_SWAP      (as DP_PORT_TYPE = DP_PORT_TYPE_DP)
// [49:48] DP_PHY_ID        (as DP_PORT_TYPE = DP_PORT_TYPE_DP)
// [47:46] DP_MAX_LANE_CNT  (as DP_PORT_TYPE = DP_PORT_TYPE_DP)
// [45:44] DP_LANE0_PHY_MAPPING (as DP_PORT_TYPE = DP_PORT_TYPE_DP)
// [43:42] DP_LANE1_PHY_MAPPING (as DP_PORT_TYPE = DP_PORT_TYPE_DP)
// [41:40] DP_LANE2_PHY_MAPPING (as DP_PORT_TYPE = DP_PORT_TYPE_DP)
// [39:38] DP_LANE3_PHY_MAPPING (as DP_PORT_TYPE = DP_PORT_TYPE_DP)
// [37:37] DP_LANE0_PN_SWAP (as DP_PORT_TYPE = DP_PORT_TYPE_DP)
// [36:36] DP_LANE1_PN_SWAP (as DP_PORT_TYPE = DP_PORT_TYPE_DP)
// [35:35] DP_LANE2_PN_SWAP (as DP_PORT_TYPE = DP_PORT_TYPE_DP)
// [34:34] DP_LANE3_PN_SWAP (as DP_PORT_TYPE = DP_PORT_TYPE_DP)
// [33: 2] Reserved
// [ 1: 0] TYPEC DP Port number
#define DP_PORT_TYPEC_DP_SEL(x)       ((unsigned long long)(x & 0x3))
#define DP_PORT_TYPEC_DP_SEL_PD0      DP_PORT_TYPEC_DP_SEL(0)
#define DP_PORT_TYPEC_DP_SEL_MASK     DP_PORT_TYPEC_DP_SEL(3)
#define GET_DP_PORT_TYPEC_DP_SEL(x)   (x & DP_PORT_TYPEC_DP_SEL_MASK)

//-----------------------------------------------------------------------------------
// DP_PORT_SPECIFIC_DATA [59:0] DP_PORT_TYPE = DP_PORT_TYPE_PSEUDO_HDMI_DP
//-----------------------------------------------------------------------------------
// [59: 2]  Reserved
// [1:0]    HDMI Port number
#define DP_PORT_HDMI_PORT_SEL(x)       ((unsigned long long)(x & 0x3))
#define DP_PORT_HDMI_PORT_SEL_HDMI1    DP_PORT_HDMI_PORT_SEL(0)
#define DP_PORT_HDMI_PORT_SEL_HDMI2    DP_PORT_HDMI_PORT_SEL(1)
#define DP_PORT_HDMI_PORT_SEL_HDMI3    DP_PORT_HDMI_PORT_SEL(2)
#define DP_PORT_HDMI_PORT_SEL_HDMI4    DP_PORT_HDMI_PORT_SEL(3)
#define DP_PORT_HDMI_PORT_SEL_MASK     DP_PORT_HDMI_PORT_SEL(3)
#define GET_DP_PORT_HDMI_PORT_SEL(x)   (x & DP_PORT_HDMI_PORT_SEL_MASK)

//-----------------------------------------------------------------------------------
// DP_PORT_SPECIFIC_DATA [59:0] DP_PORT_TYPE = DP_PORT_TYPE_PSEUDO_DP
//-----------------------------------------------------------------------------------
 //[59:0]   Reserved

/////////////////////////////////////////////////////
#ifdef UNIT_TEST
#define static    // remove static declariation of local variable
extern unsigned char g_dprx_channel_port[PLAT_DPRX_PORT_NUM];
extern DP_PORT_CFG   g_dprx_port_cfg[PLAT_DPRX_PORT_NUM];
#endif

#endif // __DPRX_PLATFORM_RTK_TV_H__
