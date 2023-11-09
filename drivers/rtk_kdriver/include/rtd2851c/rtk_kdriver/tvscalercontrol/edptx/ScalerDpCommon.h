#ifndef __SCALER_DP_COMMON_H__
#define __SCALER_DP_COMMON_H__
/********************************************************************************/
/*   The  Software  is  proprietary,  confidential,  and  valuable to Realtek   */
/*   Semiconductor  Corporation  ("Realtek").  All  rights, including but not   */
/*   limited  to  copyrights,  patents,  trademarks, trade secrets, mask work   */
/*   rights, and other similar rights and interests, are reserved to Realtek.   */
/*   Without  prior  written  consent  from  Realtek,  copying, reproduction,   */
/*   modification,  distribution,  or  otherwise  is strictly prohibited. The   */
/*   Software  shall  be  kept  strictly  in  confidence,  and  shall  not be   */
/*   disclosed to or otherwise accessed by any third party.                     */
/*   c<2003> - <2012>                                                           */
/*   The Software is provided "AS IS" without any warranty of any kind,         */
/*   express, implied, statutory or otherwise.                                  */
/********************************************************************************/

//----------------------------------------------------------------------------------------------------
// ID Code      : ScalerFunctionInclude.h No.0000
// Update Note  :
//----------------------------------------------------------------------------------------------------

//#if(CONFIG_ENABLE_EDPTX_DRV == _ON)
//****************************************************************************
// DEFINITIONS / MACROS
//****************************************************************************
#define code

#define _OFF 0
#define _ON 1
#define _FALSE 0
#define _TRUE 1
#define _LOW 0
#define _HIGH 1
#define _DISABLE 0
#define _ENABLE 1
#define _UNSTABLE 0
#define _STABLE 1
#define _FAIL 0
#define _SUCCESS 1

#define _BIT0                       0x01
#define _BIT1                       0x02
#define _BIT2                       0x04
#define _BIT3                       0x08
#define _BIT4                       0x10
#define _BIT5                       0x20
#define _BIT6                       0x40
#define _BIT7                       0x80
#define _BIT8                       0x0100
#define _BIT9                       0x0200
#define _BIT10                      0x0400
#define _BIT11                      0x0800
#define _BIT12                      0x1000
#define _BIT13                      0x2000
#define _BIT14                      0x4000
#define _BIT15                      0x8000
#define _BIT16                      0x10000
#define _BIT17                      0x20000
#define _BIT18                      0x40000
#define _BIT19                      0x80000
#define _BIT20                      0x100000
#define _BIT21                      0x200000
#define _BIT22                      0x400000
#define _BIT23                      0x800000
#define _BIT24                      0x1000000
#define _BIT25                      0x2000000
#define _BIT26                      0x4000000
#define _BIT27                      0x8000000
#define _BIT28                      0x10000000
#define _BIT29                      0x20000000
#define _BIT30                      0x40000000
#define _BIT31                      0x80000000

#define _DP_ONE_LANE    1
#define _DP_TWO_LANE   2
#define _DP_FOUR_LANE  4

#define DP_TX_PORT0_SUPPORT _ON
#define DP_TX_PORT1_SUPPORT _ON
#define DP_TX_PORT2_SUPPORT _OFF
#define DP_TX_PORT3_SUPPORT _OFF

#define GET_PANEL_DISPLAY_DP_TX_PORT_0()    (DP_TX_PORT0_SUPPORT && (eDpTx_get_outPort() & _DISPLAY_DP_TX_PORT_0))
#define GET_PANEL_DISPLAY_DP_TX_PORT_1()    (DP_TX_PORT1_SUPPORT && (eDpTx_get_outPort() & _DISPLAY_DP_TX_PORT_1))
#define GET_PANEL_DISPLAY_DP_TX_PORT_2()    (DP_TX_PORT2_SUPPORT && (eDpTx_get_outPort() & _DISPLAY_DP_TX_PORT_2))
#define GET_PANEL_DISPLAY_DP_TX_PORT_3()    (DP_TX_PORT3_SUPPORT && (eDpTx_get_outPort() & _DISPLAY_DP_TX_PORT_3))


//------------------------------------------
// Definitions of data type
//------------------------------------------
//typedef unsigned long   DWORD;
//typedef unsigned short  WORD;
//typedef unsigned char   BYTE;
typedef unsigned char   bit;


//****************************************************************************
// STRUCT / TYPE / ENUM DEFINITTIONS
//****************************************************************************
enum{
    _PANEL_VBO=0,
    _PANEL_LVDS,
    _PANEL_DPTX,
}EnumPanelStype;

enum{
    _PANEL_DPTX_LANE_NUMBER_1=0,
    _PANEL_DPTX_LANE_NUMBER_2,
    _PANEL_DPTX_LANE_NUMBER_4,
    _PANEL_DPTX_LANE_NUMBER_8,
    _PANEL_DPTX_LANE_NUMBER_16
}EnumPanelDptxLaneNumber;

enum{
    _FRAME_SYNC_MODE,
    _FRAME_FRC_MODE,
}EnumFrameSyncModeType;


enum{
    _DPTX_PANEL_1_SECTION=0,
    _DPTX_PANEL_2_SECTION,
    _DPTX_PANEL_4_SECTION,
    _DPTX_PANEL_8_SECTION,
}EnumDptxPanelSectionType;

enum{
    _PANEL_DPTX_VENDOR_SPECIFIC_NONE=0,
    _PANEL_DPTX_VENDOR_SPECIFIC_004F1H,
}EnumPanelDptcVendorSpecificType;


enum{
    _DPTX_TX0_PORT=_BIT0,
    _DPTX_TX1_PORT=_BIT1,
    _DPTX_TX2_PORT=_BIT2,
    _DPTX_TX3_PORT=_BIT3,
}EnumPanelDptxPortNumberSel;

typedef enum _DP_PORT_NUM{
    _TX0 = 0,
    _TX1,
    _TX2,
    _TX3,
    _TX_MAP_NONE=255,
}DP_PORT_NUM;

typedef enum {
    _PANEL_POWER_OFF_STATE=0,
    _PANEL_POWER_ON,
    _PANEL_BACKLIGHT_OFF,
    _WAITING_FOR_BACKLIGHT,
    _BACKLIGHT_ON_ALREADY,
    _WAITING_FOR_PANEL_OFF
} EnumPanelPowerStatus;

typedef enum {
    _PANEL_OFF=0,
    _PANEL_ON,
    _BACKLIGHT_OFF,
    _BACKLIGHT_ON,
    _PANEL_POWER_SEQ_CMD_NUM,
} EnumPanelPowerSeq;

typedef enum {
    _LIGHT_CONTROL_OFF,
    _LIGHT_CONTROL_ON,
} EnumPanelBackLightControlType;


typedef enum{
    _SYSTEM_TIMER_EVENT_PANEL_POWER_OFF=0,
    _SYSTEM_TIMER_EVENT_PANEL_BACKLIGHT_ON,
    _SCALER_TIMER_EVENT_NUM
}EnumScalerTimerEventDPType;

//typedef enum{
#define _PANEL_POWER_ON_T3  LVDS_TO_LIGHT_ON_ms
#define _PANEL_POWER_OFF_T6 LVDS_TO_PANEL_OFF_ms
//}EnumPanelPowerEventType;

typedef enum{
    _PANEL_CONTROL_OFF=0,
    _PANEL_CONTROL_ON
}EnumPanelControlType;


typedef enum{
    _DPTX_POWER_SEQUENCE_PANEL_OFF,
    _DPTX_POWER_SEQUENCE_PANEL_ON,
    _DPTX_POWER_SEQUENCE_BACKLIGHT_OFF,
    _DPTX_POWER_SEQUENCE_BACKLIGHT_ON,
    _DPTX_POWER_SEQUENCE_IDLE_PATTERN,
    _DPTX_POWER_SEQUENCE_LINK_TRAINING,
    _DPTX_POWER_SEQUENCE_VIDEO_PATTERN,
}EnumDptxPowerSequenceLevel;

typedef enum{
    _PANEL_DPTX_LINK_RBR=0x06,
    _PANEL_DPTX_LINK_HBR=0x0a,
    _PANEL_DPTX_LINK_HBR2=0x14,
    _PANEL_DPTX_LINK_HBR3=0x1e,
    _PANEL_DPTX_LINK_HBR3_9G
}EnumPanelDptxLinkRateType;

//typedef enum{
#define _PANEL_DPTX_SCRAMBLE_SEED_AUTO 0
#define _PANEL_DPTX_SCRAMBLE_SEED_0XFFFE 1
#define _PANEL_DPTX_SCRAMBLE_SEED_0XFFFF 2
//}EnumPanelDptxScrambleSeedType;

//typedef enum{
#define _PANEL_DPTX_LT_MANUAL_MODE 0
#define _PANEL_DPTX_LT_NORMAL_MODE 1
//}EnumPanelDptxLtType;

typedef enum{
    _PANEL_DISP_18_BIT=0,
    _PANEL_DISP_24_BIT,
    _PANEL_DISP_30_BIT,
    _PANEL_DISP_36_BIT,
    _PANEL_DISP_48_BIT,
}EnumPanelDispBitMode;


typedef enum {
    _COLOR_SPACE_RGB   = 0x00,
    _COLOR_SPACE_YUV422,
    _COLOR_SPACE_YUV444,
    _COLOR_SPACE_UNKNOW
} EnumDisplayDPTxColorSpaceType;


typedef enum{
    _EVENT_IVS=0,
    _EVENT_DVS,
    _EVENT_NUMBER,
} EnumScalerDDomainEventType;

typedef enum{
    _DISPLAY_BACKGROUND_H_START=0,
    _DISPLAY_BACKGROUND_H_WIDTH,
    _DISPLAY_BACKGROUND_V_START,
    _DISPLAY_BACKGROUND_V_HEIGHT,
    _DISPLAY_BACKGROUND_INFO_NUM_MAX
} EnumScalerDisplayBackgroundInfoType;


typedef enum{
    _DISPLAY_FORMAT_HS_WIDTH=0,
    _DISPLAY_FORMAT_VS_WIDTH,
    _DISPLAY_FORMAT_INFO_MAX
} EnumScalerDisplayFormatInfoType;

typedef enum{
    EDPTX_TIMING_4K60_4lane_hbr2=0,
    EDPTX_TIMING_4K60_4lane_hbr2_NB,
    EDPTX_TIMING_4K60_4lane_hbr2_TEST,
    EDPTX_TIMING_QHD165_4lane_hbr2,
    EDPTX_TIMING_FHQ120_4lane_hbr,
    EDPTX_TIMING_FHQ60_4lane_rbr,
    EDPTX_TIMING_UWQHD60_4lane_hbr,
    EDPTX_TIMING_WQXGA60_4lane_hbr,
    EDPTX_TIMING_UWQHD144_4lane_hbr2,
    EDPTX_TIMING_NUM,
} edptx_timing_type;

typedef enum
{
    _DISPLAY_DP_TX_PORT_0 = _BIT0,
    _DISPLAY_DP_TX_PORT_1 = _BIT1,
    _DISPLAY_DP_TX_PORT_2 = _BIT2,
    _DISPLAY_DP_TX_PORT_3 = _BIT3,
    _DISPLAY_DP_TX_PORT_MAX = 0xF0,
    _DISPLAY_DP_TX_NO_PORT = 0xFF,
} EnumDisplayDpTxOutputPort;

//typedef enum{
#define _TRAIN_PATTERN_END_BEFORE_IDEL_PATTERN 0
#define _TRAIN_PATTERN_END_AFTER_IDEL_PATTERN 1
//}EnumTrainPatternEndType;


// Macro/Definition
#define DP_TX_PORT_MAX_NUM  4
#define DP_TX_MAC_MAX_NUM  2
#define _DISPLAY_DP_TX_PORT_VALID 2
#define _HW_DISPLAY_DPTX_MULTI_SST_OPERATION _OFF
#define _PANEL_POWER_SEQUENCE_CONTROL_BY_USER _OFF

#define _PANEL_DPTX_LT_TPS3_SUPPORT_BY_USER _ON
#define _PANEL_DPTX_LT_TPS4_SUPPORT_BY_USER _OFF
#define _PANEL_DPTX_LINK_SEQUENCE_SWAP _OFF

#define _PANEL_POW_SEQ_T6_TIMER_EVENT_MODE _OFF
#define _PANEL_DPTX_HPD_DETECT _ON
#define _PANEL_DPTX_HPD_DETECT_UNPLUG _ON
#define _PANEL_DPTX_MSA_TIMING_PAR_IGNORE_EN _DISABLE
#define _PANEL_DPTX_ML_CH_8_10B _ON

#define _PANEL_DPTX_PORT_SWAP _DISABLE

#define _PANEL_STYLE _PANEL_DPTX
#define _PANEL_DPTX_PORT_NUMBER_SEL _DPTX_TX0_PORT
#define _PANEL_DISP_BIT_MODE _PANEL_DISP_24_BIT
#define _PANEL_DPTX_LINK_RATE _PANEL_DPTX_LINK_HBR2
#define _PANEL_DPTX_LANE_NUMBER _PANEL_DPTX_LANE_NUMBER_16 // _PANEL_DPTX_LANE_NUMBER_4 // 1port=4Lane, 4port=16Lane
#define _PANEL_DPTX_SCRAMBLE_SEED_TYPE _PANEL_DPTX_SCRAMBLE_SEED_AUTO
#define _PANEL_DPTX_SET_LT_SIGNAL_LEVEL_MODE _PANEL_DPTX_LT_MANUAL_MODE
#define _PANEL_DPTX_PANEL_SECTION _DPTX_PANEL_1_SECTION
#define _PANEL_DPTX_VENDOR_SPECIFIC_TYPE _PANEL_DPTX_VENDOR_SPECIFIC_NONE
#define _PANEL_DPTX_AUX_SET_TP_END_SEQUENCE _TRAIN_PATTERN_END_BEFORE_IDEL_PATTERN // _TRAIN_PATTERN_END_AFTER_IDEL_PATTERN

#define _PANEL_DPTX_FORCE_OUTPUT_SUPPORT _ON // should enable force output after complete panel power sequence verify and before MP
#define _PANEL_DPTX_FORCE_MSA_ENHANCE_FRAMING _ON
#define _PANEL_DPTX_FORCE_MSA_DOWN_SPREAD _ON
#define _PANEL_DPTX_FORCE_FRAMING_CHANGE_CAP _OFF
#define _PANEL_DPTX_FORCE_ALTERNATE_SR_CAP _OFF

#define _FRC_SUPPORT _OFF

#define _PANEL_DPTX_PN_SWAP                                 _ENABLE
#define _PANEL_DPTX_LANE_SWAP                             _DISABLE

#if (_PANEL_DPTX_PN_SWAP == _ENABLE)
#define _DPTX_PN_SWAP                                       0xffff    // board PN swap setting for H5X
#else
#define _DPTX_PN_SWAP                                       0x0
#endif // #if (_PANEL_DPTX_PN_SWAP == _ENABLE)

#if (_PANEL_DPTX_LANE_SWAP == _ENABLE)
#define _DPTX_LANE_SOURCE_7_0                                         0x76543210      // [TBD] DEFINE BY BOARD/CABLE PIN MUX SETTING
#define _DPTX_LANE_SOURCE_15_8                                       0xfedcba98      // [TBD] DEFINE BY BOARD/CABLE PIN MUX SETTING
#else
#define _DPTX_LANE_SOURCE_7_0                                         0x10325476
#define _DPTX_LANE_SOURCE_15_8                                       0x98badcfe      // [TBD] DEFINE BY BOARD/CABLE PIN MUX SETTING
#endif // #if (_PANEL_DPTX_LANE_SWAP == _ENABLE)

//#define EDPTX_DETEC_TASKID ROS_TASK_EDPTX_STATE_TASK


#define _SFG_TX_PHY_CLK_SEL  2  // edp tx mode only phy2 clk is enabled
#define _SFG_TX_PIF_PIXEL_MODE 1 // 00:1 pixel mode, 01:2 pixel mode, 2:4 pixel mode, 3:8 pixel mode

#define _HPD_CLK_DIV    1 // 00:Divide by 8, 01:Divide by 16, 2:Divide by 32, 3:Divide by 64
#define _HPD_DEB_LEVEL  1 // HPD Debounce by HPD_CLK_DIV. (default=1)
#define _HPD_DEB 0 // 1: High level, 0: Low level

#define _HPD_T5          0x34b // 0.5 Ms Target Timer (default: 0x34b)
#define _HPD_T10        0x696 // 1 Ms Target Timer (default: 0x0696)
#define _HPD_T20        0x0d2c// 2 Ms Target Timer (default: 0x0d2c)
#define _HPD_T100     0x41dc // 10 Ms Target Timer (default: 0x41dc)
#define _HPD_LONG    0x0// HPD Low Duration. Counted by HPD Detection Timer.(default: 0x0)

#define _DPHY_TXBIST_MODE   0 // Tx1 Bist Mode Select: 'b00: Normal (From MAC), 'b01: 10 bits mode (TP1/TP2/PRBS7/PRBS31), 'b10: 8 bits mode (TPS3/bist pattern gen.)
#define _DPHY_ENHANCE_MODE  _PANEL_DPTX_FORCE_MSA_ENHANCE_FRAMING
#define _DPHY_SCRAMBLE_DISABLE 0
#define _DPHY_SCRAMBLE_DEFAULT (_PANEL_DPTX_SCRAMBLE_SEED_TYPE == _PANEL_DPTX_SCRAMBLE_SEED_0XFFFF? 1: 0) // 1: Scramble default value = 16'hffff, 0: Scramble default value = 16'hfffe
#define _DPHY_RD_START_POS  4   // Read Start Depth Position. fifo depth=8 (default=4)


// system API
#define Scaler32SetDWord(pulAddr, ulValue)              rtd_outl(pulAddr, ulValue)
#define Scaler32SetByte(pulAddr, ucIndex, ucValue)  rtd_maskl(pulAddr, ~(MASK_BYTE << (SHIFT_CNT_BYTE * ucIndex)), ucValue << (SHIFT_CNT_BYTE * ucIndex))
#define Scaler32SetBit(pulAddr, ulAnd, ulOr)               rtd_maskl(pulAddr, ~(ulAnd), ulOr)
#define ScalerSetBit(pulAddr, ulAnd, ulOr)                    rtd_maskl(pulAddr, ~(ulAnd & 0xff), ulOr & 0xff)
#define Scaler32GetByte(pulAddr, ucIndex)                  ((rtd_inl(pulAddr) >> (SHIFT_CNT_BYTE * ucIndex)) & MASK_BYTE)
#define Scaler32GetBit(pulAddr, ulAnd)                         (rtd_inl(pulAddr) & ulAnd)

#if 1 // [eDPTX][TBD] bypass NON-eDP TX HW register access in current state
#define _Scaler32SetDWord(x,y) //rtd_outl(x,y)
#define _Scaler32SetByte(x,y,z) //rtd_maskl(x,y,z)
#define _Scaler32SetBit(x,y,z) //rtd_maskl(x,y,z)
#define _ScalerSetBit(x,y,z) //rtd_maskl(x,y,z)
#define _Scaler32GetByte(x,y) (0) //
#define _Scaler32GetBit(x,y) (0) //
#endif // #if 1 // [FIX-ME] bypass NON-eDP TX HW register access

#define SHIFT_CNT_WORD    16
#define SHIFT_CNT_BYTE      8
#define MASK_WORD 0xffff
#define MASK_BYTE 0xff

#define TO_DWORD(a,b,c,d) (((a & 0xff)<<24)|((b & 0xff)<<16)|((c & 0xff)<<8)|(d & 0xff))

#define SCALER32_GET_OFFSET_DWORD(ulOffset, pulAddr)                        rtd_inl(pulAddr|ulOffset)
#define SCALER32_SET_OFFSET_DWORD(ulOffset, pulAddr, ulValue)          rtd_outl(pulAddr|ulOffset, ulValue)
#define SCALER32_GET_OFFSET_WORD(ulOffset, pulAddr, ucWordIndex)  ((rtd_inl(pulAddr|ulOffset) >> (SHIFT_CNT_WORD * ucWordIndex)) & MASK_WORD)
#define SCALER32_SET_OFFSET_WORD(ulOffset, pulAddr, ucWordIndex, ulValue)      rtd_maskl(pulAddr|ulOffset, ~(MASK_WORD << (SHIFT_CNT_WORD * ucWordIndex)), ulValue << (SHIFT_CNT_WORD * ucWordIndex))
#define SCALER32_GET_OFFSET_BYTE(ulOffset, pulAddr, ucIndex)             ((rtd_inl(pulAddr|ulOffset) >> (SHIFT_CNT_BYTE * ucIndex)) & MASK_BYTE)
#define SCALER32_SET_OFFSET_BYTE(ulOffset, pulAddr, ucIndex, ucValue)        rtd_maskl(pulAddr|ulOffset, ~(MASK_BYTE << (SHIFT_CNT_BYTE * ucIndex)), ucValue << (SHIFT_CNT_BYTE * ucIndex))
#define SCALER32_GET_OFFSET_BIT(ulOffset, pulAddr, ulAnd)                   (rtd_inl(pulAddr|ulOffset) & ulAnd)
#define SCALER32_SET_OFFSET_BIT(ulOffset, pulAddr, ulAnd, ulOr)           rtd_maskl(pulAddr|ulOffset, ~(ulAnd), ulOr)


#define ScalerTimerDelayXms(x) ROSTimeDly(x)
#define DELAY_5US() udelay(5)
#define DELAY_XUS(x) udelay(x)
#define DELAY_AUX_CMD_DATA() udelay(g_ui_auxCmdDelayUs)   // AUX cmd and cmd data for eDP AUX data bus latency, delay 5us for MNT eDP RX

#define DELAY_WAIT_T1 10 // 10ms in T1

#define ABSDWORD(x,y) ((x > y) ? (x-y) : (y-x))


#define GET_PANEL_DPTX_MSO_MODE() (_OFF)

// [eDPTX][TBD] set panel power action
#define SET_PANELPOWERACTION(enumEvent)

// [eDPTX][TBD] Support PCB GPIO control for system debug (L/H signal debug)
#define PCB_DPTX0_POWER_SEQUENCE(bLevel)
#define PCB_DPTX1_POWER_SEQUENCE(bLevel)
#define PCB_DPTX2_POWER_SEQUENCE(bLevel)
#define PCB_DPTX3_POWER_SEQUENCE(bLevel)

// pre-assign
unsigned char  pData[256];

// [eDPTX][TBD]
#define _EXT_XTAL (1)
#define _PANEL_DPTX_DCLK_DIV (1)
#define _PANEL_DPTX_MSO_DIV (1)

#define GET_DWORD_MUL_DIV(x,y,z) (((unsigned long long)x*y)/z)

#define GET_D_CLK_FREQ() eDpTx_get_edp_tx_timing_info(eDpTx_get_currentTimingMode(), EDPTX_TG_PIXELFREQ)
#define GET_MAC_CNT() eDpTx_get_edp_tx_timing_info(eDpTx_get_currentTimingMode(), EDPTX_TG_MAC_CNT)
#define GET_ACT_SIZE_H() eDpTx_get_edp_tx_timing_info(eDpTx_get_currentTimingMode(), EDPTX_TG_HWIDTH)
#define GET_ACT_SIZE_V() eDpTx_get_edp_tx_timing_info(eDpTx_get_currentTimingMode(), EDPTX_TG_VHEIGHT)
#define GET_HS_WIDTH() eDpTx_get_edp_tx_timing_info(eDpTx_get_currentTimingMode(), EDPTX_TG_HSWIDTH)
#define GET_VS_WIDTH() eDpTx_get_edp_tx_timing_info(eDpTx_get_currentTimingMode(), EDPTX_TG_VSWIDTH)
#define GET_LINK_RATE() eDpTx_get_edp_tx_timing_info(eDpTx_get_currentTimingMode(), EDPTX_TG_LINK_RATE)

#define GET_DIS_TIMING_GEN_H_TOTAL() (PPOVERLAY_DH_Total_Last_Line_Length_get_dh_total(rtd_inl(PPOVERLAY_DH_Total_Last_Line_Length_reg)) +1)
#define GET_FREE_RUN_DVTOTAL() (CONFIG_DISP_VERTICAL_TOTAL)

// [eDPTX][TBD]
#define _SPEED_30K (2)
#define _SPEED_33K (3)
#define GET_PANEL_DPTX_SPREAD_RANGE() (0)
#define GET_PANEL_DPTX_SPREAD_SPEED() (_SPEED_30K)
#define GET_PANEL_DPTX_PORT_SWAP() (0)
#define GET_PANEL_DPTX_SWING_LEVEL(x) (0)//GET_DISPLAY_DP_TXX_LANE3_CURRENT_VOL_SWING(x)
#define GET_PANEL_DPTX_PREEMPHASIS_LEVEL(x) (0)//GET_DISPLAY_DP_TXX_LANE3_CURRENT_PRE_EMPHASIS(x)
#define GET_PANEL_DPTX_LANE_PN_SWAP() (_PANEL_DPTX_PN_SWAP)
#define GET_PANEL_DPTX_LANE_SWAP() (_PANEL_DPTX_LANE_SWAP)

#define ScalerGlobalGetFrontBackStatus(x) (1) // [eDPTX][TBD]

#define _ENABLE_DEBUG_CMD _OFF
#if (_ENABLE_DEBUG_CMD == _ON)
#define FW_BREAK_CHECK(){if(rtd_inl(0xB802D3F4) & _BIT16) ErrorMessageEDPTx("[EDPTX] BREAK@%s.%d\n", __FUNCTION__, __LINE__); while(rtd_inl(0xB802D3F4) & _BIT16){ScalerTimerDelayXms(500);} if(rtd_inl(0xB802D3F4) & _BIT17) {rtd_maskl(0xB802D3F4, ~(_BIT16|_BIT17), _BIT16|_BIT17); ErrorMessageEDPTx("[EDPTX] CHECK PASS@%s.%d\n", __FUNCTION__, __LINE__);}}
#define BYPASS_AUX_CHECK() {if(rtd_inl(0xB802D3F4) & _BIT23){ ErrorMessageEDPTx("[EDPTX] BYPASS AUX\n"); return;}}

#endif // #if (_ENABLE_DEBUG_CMD == _ON)


//****************************************************************************
// VARIABLE EXTERN
//****************************************************************************


//****************************************************************************
// FUNCTION EXTERN
//****************************************************************************
//int ScalerTimerWaitForActiveTimerEvent(EnumScalerTimerEventDPType eventType);
//int ScalerTimerActiveTimerEvent(unsigned int time_ms, EnumScalerTimerEventDPType dp_event);
//int ScalerTimerCancelTimerEvent(EnumScalerTimerEventDPType dp_event);
//int ScalerTimerWaitForDDomainEvent(EnumScalerDDomainEventType event);
//unsigned int ScalerMDomainGetDVtotalLastline(void);

//unsigned int ScalerDDomainGetDisplayBackground_R1(EnumScalerDisplayBackgroundInfoType info);
//unsigned int ScalerDDomainGetDisplayFormat_R1a1(EnumScalerDisplayFormatInfoType info);
//unsigned char ScalerMDomainGetFrameSyncMode(void);

//int UserCommonPowerPanel_Init(EnumPanelPowerStatus pwrStatus);

//#endif // End of #if(_PANEL_STYLE == _PANEL_DPTX)

#endif // __SCALER_DP_COMMON_H__
