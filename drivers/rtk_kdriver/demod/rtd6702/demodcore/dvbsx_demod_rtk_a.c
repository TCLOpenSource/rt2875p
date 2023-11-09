/***************************************************************************** 
Copyright 2018, Realtek Inc. All rights reserved.
File Name: dvbsx_demod_rtk_a.c
Version: 1.0.9
Released 2018-03-15
Description: DVBS programming interface
******************************************************************************/
#include <rtk_kdriver/io.h>
#include "dvbsx_demod_rtk_a.h"
#include "diseqc_rtk.h"
#include "tv_osal.h"
#include "tuner.h"
#include "rbus/pll27x_reg_reg.h"
#include "rbus/tv_sb1_ana_reg.h"
#include "demod_rtk_common.h"
#include "rbus/dtv_demod_misc_reg.h"

unsigned char u8DVBSx255kSupport = 1;
unsigned int u32DefaultStandard = 0;
stBlindScanCheckedParams gScanedParams;
unsigned char gDVBSxUnicableSpectrumInverse = 0; //spectrum inverted if unicable device used.
extern struct RtkDemodDvbSxBsSpecialParams gDvbSxBsSpecialParams;
extern struct RtkDemodDvbSxBsLocalParams gDvbSxBsLocalParams;
struct LiteSymbolRateSignalListStruct gLiteymbolRateSignalList;
RTK_DVBSx_SIGNAL_PARAMS stPreSxParams;
unsigned char Pre_Plt = 0;
extern unsigned char u8stLockCnt;

/*Constellation and Code rate table of DVB-S/S2*/
static RTK_DVBS2_MODU_CR S2_Mod_CodeRate_table[] =
{
	{RTK_DVBS2_CODE_RATE_1_4},//0
	{RTK_DVBS2_CODE_RATE_1_3},
	{RTK_DVBS2_CODE_RATE_2_5},
	{RTK_DVBS2_CODE_RATE_1_2},
	{RTK_DVBS2_CODE_RATE_3_5},
	{RTK_DVBS2_CODE_RATE_2_3},//5
	{RTK_DVBS2_CODE_RATE_3_4},
	{RTK_DVBS2_CODE_RATE_4_5},
	{RTK_DVBS2_CODE_RATE_5_6},
	{RTK_DVBS2_CODE_RATE_8_9},
	{RTK_DVBS2_CODE_RATE_9_10}//10
};

/*Constellation and Code rate table of DVB-S2X*/
static RTK_DVBS2X_MODU_CR S2X_Mod_CodeRate_table[] =
{
	{RTK_DVBS2X_CODE_RATE_3_5},
	{RTK_DVBS2X_CODE_RATE_2_3},
	{RTK_DVBS2X_CODE_RATE_13_45},  //24
	{RTK_DVBS2X_CODE_RATE_9_20},
	{RTK_DVBS2X_CODE_RATE_11_20},
	{RTK_DVBS2X_CODE_RATE_26_45},
	{RTK_DVBS2X_CODE_RATE_28_45},
	{RTK_DVBS2X_CODE_RATE_23_36},  //29
	{RTK_DVBS2X_CODE_RATE_25_36},
	{RTK_DVBS2X_CODE_RATE_13_18},
	{RTK_DVBS2X_CODE_RATE_7_9},
	{RTK_DVBS2X_CODE_RATE_90_180},
	{RTK_DVBS2X_CODE_RATE_96_180}, //34
	{RTK_DVBS2X_CODE_RATE_100_180},
	{RTK_DVBS2X_CODE_RATE_104_180},
	{RTK_DVBS2X_CODE_RATE_116_180},
	{RTK_DVBS2X_CODE_RATE_124_180},
	{RTK_DVBS2X_CODE_RATE_128_180},//39
	{RTK_DVBS2X_CODE_RATE_132_180},
	{RTK_DVBS2X_CODE_RATE_135_180},
	{RTK_DVBS2X_CODE_RATE_140_180},
	{RTK_DVBS2X_CODE_RATE_144_180},
	{RTK_DVBS2X_CODE_RATE_150_180},//44
	{RTK_DVBS2X_CODE_RATE_154_180},
	{RTK_DVBS2X_CODE_RATE_18_30},
	{RTK_DVBS2X_CODE_RATE_20_30},
	{RTK_DVBS2X_CODE_RATE_22_30}   //48
};

/* for signal's symbol rate < 0.8M*/
static unsigned char DvbSxSmallSymbolRegs[] =
{
	0x2d, 0x11, 0x03, //47 sync sequence
	0x2d, 0x12, 0x05,
	0x2d, 0x13, 0x04,
	0x2d, 0x14, 0x02,
	0x2d, 0x15, 0x01,
	0x0a, 0x4d, 0x0a, //timing scan symbol number
	0x15, 0x5c, 0x03,
	0x15, 0x6a, 0x01,
	0x15, 0x6b, 0x01,
	0x2d, 0x48, 0x01,
	0x2a, 0xda, 0x01,
	0x2a, 0xd7, 0x80,
	0x2a, 0xd6, 0x80,
	0x2a, 0xd5, 0x80,
	0x2a, 0xd4, 0x80,
	0x2a, 0xd3, 0x80,
	//ts ouput from
	0x67, 0x2c, 0xbc,
	0x67, 0x2d, 0x00,
	0x67, 0x00, 0xbc,
	0x67, 0x01, 0x00,
	0x18, 0x2b, 0x01,
};

static unsigned char ExtenedDVBSxRegs[] =
{
	//Set CommonOptmized regs
	0x67, 0x0d, 0x10,
	0x67, 0x16, 0x10,
	
	0x09, 0x9F, 0x14,
	0x09, 0x93, 0xec,
	0x09, 0xA0, 0x1a,
	0x09, 0x94, 0xe6,
	0x09, 0xA1, 0x1a,
	0x09, 0x95, 0xe6,  //changed 12.23 CFO RANGE
	
	0x10, 0x09, 0x80,  //AGC kp 2016.12.15
	0x10, 0x0a, 0x20,  //AGC ki
	0x10, 0x0b, 0x04,  //AGC ki
	0x10, 0x0c, 0x01,  //AGC kp
	
	0x2a, 0x54, 0x10,
	0x2a, 0x55, 0x1a,
	0x2d, 0x56, 0x01,
	0x2a, 0xda, 0x03,  //2017.08.03 Registers default setting

	//-------------------------------------------------------
	//Ori Extened Regs
	0x29, 0x84, 0x05,  //hw default 5
	//Xiaohui
	//8©y params_dvbs_eq_mode Equalizer mode 0 -> feed forward only, 1 -> DFE only, 2 -> feed forward to DFE based on SNR
	//9©y params_dvbs_eq_mode_force
	//Params_dvbs_eq_mode	Bit0~1	RW	29H 38H	2'b01
	0x29, 0x34, 0x02,   //5©y params_feed_f_alpha the alpha value of update the forward coefs, 0-1/32, 1-1/64, 2-1/128, 3-1/256, 4-1/512, 6-1/8, 7-1/16
	0x0a, 0xf3, 0x01,   // dc remove enable
	0x0a, 0xf0, 0x04,   //dc alpha
	//0x10, 0x04, 0xcf, // agc target power, same as wagtail
						//xiaofu
	0x2D, 0x49, 0x01,   //pre_equalizer_enable_s2x  U(1,0)
	0x2D, 0x4A, 0x0b,   //pre_main_path_index_s2x   U(5,0)
	0x2D, 0x4B, 0x15,   //pre_feed_f_tap_n_s2x      U(5,0) RW

	0x2D, 0x52, 0x01,   //add_f_tap_n   U(4,0) RW

	0x2d, 0x4e, 0x2c,
	0x2d, 0x4f, 0x00,   // params_pre_equ_ratio_th
	0x2d, 0x50, 0x90,
	0x2d, 0x51, 0x00,   // params_pre_equ_power_th
	0x2d, 0x8b, 0x2c,   //80 //params_pre_equ_ratio_th_qpsk_8psk[7:0]
	0x2d, 0x8c, 0x00,   //params_pre_equ_ratio_th_qpsk_8psk[11:8]
	0x2d, 0x8d, 0x90,   //20 //params_pre_equ_power_th_qpsk_8psk[7:0]
	0x2d, 0x8e, 0x00,   //1 //params_pre_equ_power_th_qpsk_8psk[11:8]

	0x29, 0x8b, 0x00,   // params_s2x_header_snr_th_qpsk[11:8]

	0x29, 0xfc, 0xff,   // bypass equalize, scale. alpha1/alpha2/ vco, ff
	0x2d, 0xab, 0x00,   // params_qpsk_ini_vco_dpfll_lock_flag

	0x2d, 0x99, 0x02,   // bit[1] is header pn qpsk_pilot_flag

	0x29, 0x08, 0x03,   // s state2 lock number;
	0x29, 0x7A, 0x40,   // S QPSK phase noise thr

	0x2a, 0xa0, 0x5a,   //c1, U(8, 15)
	0x2a, 0xa1, 0x3d,   //c2, U(8, 27)

	0x2a, 0x7c, 0x80,   // state1 c1 U(8, 13)
	0x2a, 0x7d, 0x50,   // state1 c2 U(8, 25)

	0x2a, 0x7e, 0x80,   // state2 c1 U(8, 13)
	0x2a, 0x7f, 0x50,   // state2 c2 U(8, 25)
	
	0x2d, 0xa3, 0x23,   // pilot phase compute end idx when phase noise, 35

	0x10, 0x05, 0x03,   //Change AGC Target Level
	0x10, 0x04, 0x0f,
	0x10, 0x03, 0x0b,
};

static const short LiteCfoTable[] =
{
	240, -240, 720, 1500, -720, -1500, 1200, -1200,
	720, 240, 1200, -240, 1500, -720, -1200, -1500,
	1200, 720, 1500, 240, -240, -720, -1200, -1500,
	1500, 1200, 720, 240, -240, -720, -1200, -1500,
	-240, 240, -720, 720, -1200, 1200, -1500, 1500,
	-720, -240, -1200, 240, -1500, 720, 1200, 1500,
	-1200, -720, -1500, -240, 240, 720, 1200, 1500,
	-1500, -1200, -720, -240, 240, 720, 1200, 1500,
};

static unsigned int SrOptimalTable[] = 
{
	1024000, 3500,
	3200000, 3200,
};
#if 0
#define MAX_ITERATION_RATIO 15 //16
static const DVBSX_LDPC_ITERATION_TABLE sDvbSxLdpcIterationTable[] =
{
	//QPSK
	{RTK_DVBSx_MODU_QPSK,		RTK_DVBS2_CODE_RATE_1_4,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	34,		35},
	{RTK_DVBSx_MODU_QPSK,		RTK_DVBS2_CODE_RATE_1_3,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	34,		31},
	{RTK_DVBSx_MODU_QPSK,		RTK_DVBS2_CODE_RATE_2_5,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	34,		29},
	{RTK_DVBSx_MODU_QPSK,		RTK_DVBS2_CODE_RATE_1_2,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	34,		30},
	{RTK_DVBSx_MODU_QPSK,		RTK_DVBS2_CODE_RATE_3_5,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	55,		24},
	{RTK_DVBSx_MODU_QPSK,		RTK_DVBS2_CODE_RATE_2_3,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	62,		31},
	{RTK_DVBSx_MODU_QPSK,		RTK_DVBS2_CODE_RATE_3_4,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	72,		30},
	{RTK_DVBSx_MODU_QPSK,		RTK_DVBS2_CODE_RATE_4_5,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	78,		29},
	{RTK_DVBSx_MODU_QPSK,		RTK_DVBS2_CODE_RATE_5_6,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	83,		28},
	{RTK_DVBSx_MODU_QPSK,		RTK_DVBS2_CODE_RATE_8_9,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	94,		35},
	{RTK_DVBSx_MODU_QPSK,		RTK_DVBS2_CODE_RATE_9_10,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	96,		35},
	{RTK_DVBSx_MODU_QPSK,		RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_11_20,		48,		21},
	{RTK_DVBSx_MODU_QPSK,		RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_9_20,		37,		21},
	{RTK_DVBSx_MODU_QPSK,		RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_13_45,		20,		21},
	//8PSK
	{RTK_DVBSx_MODU_8PSK,		RTK_DVBS2_CODE_RATE_3_5,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	91,		16},
	{RTK_DVBSx_MODU_8PSK,		RTK_DVBS2_CODE_RATE_2_3,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	99,		21},
	{RTK_DVBSx_MODU_8PSK,		RTK_DVBS2_CODE_RATE_3_4,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	112,	20},
	{RTK_DVBSx_MODU_8PSK,		RTK_DVBS2_CODE_RATE_5_6,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	126,	19},
	{RTK_DVBSx_MODU_8PSK,		RTK_DVBS2_CODE_RATE_8_9,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	139,	23},
	{RTK_DVBSx_MODU_8PSK,		RTK_DVBS2_CODE_RATE_9_10,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	142,	23},
	{RTK_DVBSx_MODU_8PSK,		RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_23_36,		97,		16},
	{RTK_DVBSx_MODU_8PSK,		RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_25_36,		105,	15},
	{RTK_DVBSx_MODU_8PSK,		RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_13_18,		109,	16},
	{RTK_DVBSx_MODU_8PSKL,		RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_100_180,	83,		14},
	{RTK_DVBSx_MODU_8PSKL,		RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_104_180,	85,		14},
	//16APSK
	{RTK_DVBSx_MODU_16APSK,		RTK_DVBS2_CODE_RATE_3_5,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	120,	15},
	{RTK_DVBSx_MODU_16APSK,		RTK_DVBS2_CODE_RATE_2_3,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	128,	15},
	{RTK_DVBSx_MODU_16APSK,		RTK_DVBS2_CODE_RATE_3_4,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	136,	14},
	{RTK_DVBSx_MODU_16APSK,		RTK_DVBS2_CODE_RATE_5_6,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	144,	14},
	{RTK_DVBSx_MODU_16APSK,		RTK_DVBS2_CODE_RATE_8_9,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	152,	17},
	{RTK_DVBSx_MODU_16APSK,		RTK_DVBS2_CODE_RATE_9_10,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	160,	17},
	{RTK_DVBSx_MODU_16APSK,		RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_26_45,		117,	10},
	{RTK_DVBSx_MODU_16APSK,		RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2_CODE_RATE_3_5,		117,	12},
	{RTK_DVBSx_MODU_16APSK,		RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_28_45,		120,	11},
	{RTK_DVBSx_MODU_16APSK,		RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_23_36,		122,	12},
	{RTK_DVBSx_MODU_16APSK,		RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_25_36,		130,	11},
	{RTK_DVBSx_MODU_16APSK,		RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_13_18,		133,	12},
	{RTK_DVBSx_MODU_16APSK,		RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_140_180,	142,	11},
	{RTK_DVBSx_MODU_16APSK,		RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_154_180,	155,	11},
	{RTK_DVBSx_MODU_16APSKL,	RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_90_180,	102,	10},
	{RTK_DVBSx_MODU_16APSKL,	RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_96_180,	106,	10},
	{RTK_DVBSx_MODU_16APSKL,	RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_100_180,	108,	11},
	{RTK_DVBSx_MODU_16APSKL,	RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_20_30,		125,	10},
	{RTK_DVBSx_MODU_16APSKL,	RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_18_30,		115,	10},
	//32APSK
	{RTK_DVBSx_MODU_32APSK,		RTK_DVBS2_CODE_RATE_3_4,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	148,	12},
	{RTK_DVBSx_MODU_32APSK,		RTK_DVBS2_CODE_RATE_4_5,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	156,	11},
	{RTK_DVBSx_MODU_32APSK,		RTK_DVBS2_CODE_RATE_5_6,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	164,	11},
	{RTK_DVBSx_MODU_32APSK,		RTK_DVBS2_CODE_RATE_8_9,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	172,	14},
	{RTK_DVBSx_MODU_32APSK,		RTK_DVBS2_CODE_RATE_9_10,		RTK_DVBS2X_CODE_RATE_UNKNOWN,	180,	14},
	{RTK_DVBSx_MODU_32APSKL,	RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2_CODE_RATE_2_3,		148,	12},
	{RTK_DVBSx_MODU_32APSK,		RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_128_180,	159,	8},
	{RTK_DVBSx_MODU_32APSK,		RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_132_180,	161,	9},
	{RTK_DVBSx_MODU_32APSK,		RTK_DVBS2_CODE_RATE_UNKNOWN,	RTK_DVBS2X_CODE_RATE_140_180,	169,	8},
};
#define DVBSX_LDPC_ITERATION_TABLE_SIZE sizeof(sDvbSxLdpcIterationTable)/sizeof(DVBSX_LDPC_ITERATION_TABLE)
#endif
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvbSxTunerSpecialSettings(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS u8Status = RTK_DEMOD_SUCCESSFUL;

	if(pDemod->CurrentTunerId == TV_TUNER_AV2018)
	{
		REALTEK_A_DVBSX_INFO("SetDvbSxMode with TV_TUNER_AV2018\n");
	}
	else if(pDemod->CurrentTunerId == TV_TUNER_RT710)
	{
		REALTEK_A_DVBSX_INFO("SetDvbSxMode with TV_TUNER_RT710\n");
		//Change AGC Target Level for RT710
		u8Status |= RTK_A_DVBSX_Write(pDemod, 0x10, 0x05, 0x03);
		u8Status |= RTK_A_DVBSX_Write(pDemod, 0x10, 0x04, 0x12);
		u8Status |= RTK_A_DVBSX_Write(pDemod, 0x10, 0x03, 0xff);

		//Change AGC Speed for RT710
		u8Status |= RTK_A_DVBSX_Write(pDemod, 0x10, 0x09, 0x20);
		u8Status |= RTK_A_DVBSX_Write(pDemod, 0x10, 0x0a, 0x08);
		u8Status |= RTK_A_DVBSX_Write(pDemod, 0x10, 0x0b, 0x08);
		u8Status |= RTK_A_DVBSX_Write(pDemod, 0x10, 0x0c, 0x02);
	}
	else if(pDemod->CurrentTunerId == TV_TUNER_RDA5815M)
	{
		REALTEK_A_DVBSX_INFO("SetDvbSxMode with TV_TUNER_RDA5815M\n");
	}
	
	return u8Status;
}

RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDefaultValue(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS u8Status = RTK_DEMOD_SUCCESSFUL;
	
	u8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x16, 0x01);//0x00, 0x16 => 0x00 to 0x01
	u8Status |= RTK_A_DVBSX_Write(pDemod, 0x10, 0x3c, 0x00);//0x10, 0x3c => 0x01 to 0x00
	u8Status |= RTK_A_DVBSX_Write(pDemod, 0x17, 0xd9, 0xc1);//0x17, 0xd9 => 0xef to 0xc1
	u8Status |= RTK_A_DVBSX_Write(pDemod, 0x17, 0xdf, 0x10);//0x17, 0xdf => 0x20 to 0x10
	u8Status |= RTK_A_DVBSX_Write(pDemod, 0x17, 0xf9, 0x01);//0x17, 0xf9 => 0x02 to 0x01
	u8Status |= RTK_A_DVBSX_Write(pDemod, 0x18, 0x17, 0x01);//0x18, 0x17 => 0x04 to 0x01
	
	return u8Status;
}

/********************************************************************************
functionf   :RTK_A_DVBSX_SetDvbSxMode
parameter   :u32SymbolrateHz symbol rate unit in Hz.
return      :Operate Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal.
Description
Set DVB-S_S2_S2X auto mode with SymbolRate value, unit in symbols per second.
Call this function to set demodulator receives DVB-S and DVB-S2 signal with symbol rate set mode.
*********************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvbSxMode(DVBSX_DEMOD_MODULE *pDemod, unsigned int u32Standard, unsigned int u32SymbolrateHz, unsigned int u32FreqKhz)
{
	RTK_A_DVBSX_STATUS u8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned char u8ModeReg = 0, u8ConfigdoneReg = 0;

	gstRtkDemodDvbConfig.ui8ConfigDVBType = RTK_DEMOD_DVBS2_MODE; //Either DVB-S or DVB-S2 can be set for the compatible receiving mode.
	gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz = u32SymbolrateHz;
	gstRtkDemodDvbConfig.DvbSxTunerInput.ui32FreqKhz = u32FreqKhz;
	
	//u8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0x01); /* Reset DSP */
	u8Status |= RTK_A_DVBSX_Read(pDemod, 0x00, 0x04, &u8ConfigdoneReg);
	u8Status |= RTK_A_DVBSX_Read(pDemod, 0x00, 0x2b, &u8ModeReg);

	if(u32SymbolrateHz < RTK_DVBSX_LITE_SYMBOL_HZ)
	{
		gstRtkDemodDvbConfig.ui32DVBSAdcClkKHz = RTK_A_DVBSX_LOW_SR_ADC_CLK_KHz;
	}
	else
	{
		gstRtkDemodDvbConfig.ui32DVBSAdcClkKHz = RTK_A_DVBSX_ADC_CLK_KHz;
	}

	RTK_A_DVBSX_Write(pDemod, 0x00, 0x03, 0x01); //chip reset

	//Set default values
	u8Status |= RTK_A_DVBSX_SetDefaultValue(pDemod);

	//Set sysmtem clock registers
	u8Status |= RTK_A_DVBSX_SetSysClkClockDVBSx(pDemod);

	//Set TS interface registers
	u8Status |= RTK_A_DVBSX_SetTSMode(pDemod, gstRtkDemodDvbConfig.stMPEGTSMode);

	//Set GPIO Functions for AGC Out Pin
	RTK_A_DVBSX_Write(pDemod, 0x00, 0x23, 0x00);
	RTK_A_DVBSX_Write(pDemod, 0x00, 0x21, 0x03);
	RTK_A_DVBSX_Write(pDemod, 0x00, 0x22, 0x00);
	RTK_A_DVBSX_Write(pDemod, 0x00, 0x37, 0x00);

	RTK_A_DVBSX_SetDvBSxRegs(pDemod, FALSE);

	//new chip add the following registers
	u8Status |= RTK_A_DVBSX_WriteRegArray(pDemod, ExtenedDVBSxRegs, sizeof(ExtenedDVBSxRegs)/sizeof(unsigned char));

	RTK_A_DVBSX_SetSymbolRate(pDemod, u32SymbolrateHz/1000);
	if(gstRtkDemodDvbConfig.DvbSxTunerInput.ui8IQSwapMode)
	{
		u8Status |= RTK_A_DVBSX_SwapIQ(pDemod, 1);
	}
	else
	{
		u8Status |= RTK_A_DVBSX_SwapIQ(pDemod, 0);
	}
	
	//Special Settings for Different Tuner
	RTK_A_DVBSX_SetDvbSxTunerSpecialSettings(pDemod);
	
	//Disable LDPC 1.6X Clock
	rtd_maskl(SYS_REG_SYS_CLKDIV_reg, ~(SYS_REG_SYS_CLKDIV_dtv_demod_atb_clk_sel_mask), SYS_REG_SYS_CLKDIV_dtv_demod_atb_clk_sel(0x1));
	
	u8Status |= RTK_A_DVBSX_SetSrOptimalTable(pDemod, u32SymbolrateHz, SrOptimalTable, sizeof(SrOptimalTable)/sizeof(unsigned int));
	//clear stPreSxParams
	stPreSxParams.DVBSxType = 0;
	stPreSxParams.DVBSxModu = 0;
	stPreSxParams.DVBSCodeRate = 0;
	stPreSxParams.DVBS2CodeRate = 0;

	if(u32SymbolrateHz >= 19000000)//for high SR
	{
		RTK_A_DVBSX_Write(pDemod, 0x2a, 0xe0, 0x40);
		RTK_A_DVBSX_Write(pDemod, 0x18, 0x2b, 0x01);// LDPC parameter TS Lock with 1 BCH Correct
	}
	else
	{
		RTK_A_DVBSX_Write(pDemod, 0x18, 0x2b, 0x00);// LDPC parameter TS Lock with 0 BCH Correct
	}
	
	if(u32SymbolrateHz > 29700000 && u32SymbolrateHz < 29740000)//DAGC
	{
		REALTEK_A_DVBSX_INFO("DAGC Settings\n");
		RTK_A_DVBSX_Write(pDemod, 0x09, 0x48, 0x07);
		RTK_A_DVBSX_Write(pDemod, 0x0a, 0x4b, 0x07);
		RTK_A_DVBSX_Write(pDemod, 0x09, 0x3f, 0x02);
		RTK_A_DVBSX_Write(pDemod, 0x09, 0x49, 0x02);
		RTK_A_DVBSX_Write(pDemod, 0x09, 0x40, 0x07);
		RTK_A_DVBSX_Write(pDemod, 0x09, 0x4a, 0x07);
		
		RTK_A_DVBSX_Write(pDemod, 0x0a, 0xf6, 0x06);
		RTK_A_DVBSX_Write(pDemod, 0x0a, 0xf7, 0x06);
		
		RTK_A_DVBSX_Write(pDemod, 0x10, 0x09, 0x83);
		RTK_A_DVBSX_Write(pDemod, 0x10, 0x0a, 0x2c);
		RTK_A_DVBSX_Write(pDemod, 0x10, 0x0b, 0x06);
		RTK_A_DVBSX_Write(pDemod, 0x10, 0x0c, 0x02);
	}
	if(u32SymbolrateHz >= 40000000)
	{
		RTK_A_DVBSX_Write(pDemod, 0x2d, 0xa0, 0x52);//for SR higer than 40M
		RTK_A_DVBSX_Write(pDemod, 0x2d, 0xa2, 0x1c);
	}

	RTK_A_DVBSX_Write(pDemod, 0x2d, 0x23, 0x0e);
	RTK_A_DVBSX_Write(pDemod, 0x2a, 0xed, 0x04); //resume pilot adjust

	//RTK_A_DVBSX_Write(pDemod, 0x29, 0x12, 0x4f);
	RTK_A_DVBSX_Write(pDemod, 0x18, 0x8b, 0x01); //set 10ms TS rate counter window. 2018.5.2
	RTK_A_DVBSX_Write(pDemod, 0x00, 0x04, 0x01);
	RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0x01); /* Reset DSP */
	RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0x00); /* Start DSP */

	RTK_A_DVBSX_Write(pDemod, 0x18, 0x99, 0x01); // Reset packet error counter.
	RTK_A_DVBSX_Write(pDemod, 0x18, 0x99, 0x00);

	return u8Status;
}

RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvbSxVeryLowSrAdc(DVBSX_DEMOD_MODULE *pDemod, unsigned char VeryLowSrEn)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	if(DtvGetICVersion() == 0) { //Mark2 Version B, use PLLDif
		if(VeryLowSrEn)
			rtd_maskl(PLL27X_REG_SYS_PLL_DIF2_reg, ~(PLL27X_REG_SYS_PLL_DIF2_plldif_divadc_sel_mask), PLL27X_REG_SYS_PLL_DIF2_plldif_divadc_sel(0x0)); //ADC CLK 60M
		else
			rtd_maskl(PLL27X_REG_SYS_PLL_DIF2_reg, ~(PLL27X_REG_SYS_PLL_DIF2_plldif_divadc_sel_mask), PLL27X_REG_SYS_PLL_DIF2_plldif_divadc_sel(0x4)); //ADC CLK 120M
	}
	else { //Mark2 Version A, use CKOUT Div
		if(VeryLowSrEn)
			rtd_maskl(TV_SB1_ANA_IQADC_CTRL6_reg, ~(TV_SB1_ANA_IQADC_CTRL6_reg_qadc_ckout_div_mask|TV_SB1_ANA_IQADC_CTRL6_reg_iadc_ckout_div_mask), TV_SB1_ANA_IQADC_CTRL6_reg_qadc_ckout_div(3) | TV_SB1_ANA_IQADC_CTRL6_reg_iadc_ckout_div(3)); //IADC_CKOUT_DIV<bit9:8>=3(/2),QADC_CKOUT_DIV<bit9:8>=3(/4)
		else
			rtd_maskl(TV_SB1_ANA_IQADC_CTRL6_reg, ~(TV_SB1_ANA_IQADC_CTRL6_reg_qadc_ckout_div_mask|TV_SB1_ANA_IQADC_CTRL6_reg_iadc_ckout_div_mask), TV_SB1_ANA_IQADC_CTRL6_reg_qadc_ckout_div(1) | TV_SB1_ANA_IQADC_CTRL6_reg_iadc_ckout_div(1)); //IADC_CKOUT_DIV<bit9:8>=1(/2),QADC_CKOUT_DIV<bit9:8>=1(/2)
	}
	return status;
}

/********************************************************************************
function    :RTK_A_DVBSX_SetDvbSxModeOnly
parameter   :None
return      :Operate Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal.
Description
Just set demodulator on DVB-S_S2_S2X mode, default set 45M symbol rate.
The following is blind scan typically flow:

1, RTK_A_DVBSX_SetDvbSxModeOnly(void) or RTK_A_DVBSX_SetDvbSxMode be called once, demodulator will works on DVB-S/S2 mode.
2, Set DiSEqC for satellite band (DiSEqC command, 13V&18V, 22K on off etc)
3, Blind scan for current band setting.

*********************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvbSxModeOnly(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;

	status = RTK_A_DVBSX_SetDvbSxMode(pDemod, u32DefaultStandard, 45000*1000, DUMMY_FREQ_KHZ);

	return status;
}

/********************************************************************************
function    :RTK_A_DVBSX_SetDVBSx255KEnableDisable
parameter   :u8EnableDvbSx225KFlag 
return      :Operate Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal.
Description
Set DVB-S_S2_S2X 255K support or not
1: Enable 255K
0: Disable 255K
Blind scan do not support 255k symbol rate.
*********************************************************************************/
void RTK_A_DVBSX_SetDVBSx255kEnableDisable(DVBSX_DEMOD_MODULE *pDemod, unsigned char u8EnableDvbSx225kFlag)
{
	if(u8EnableDvbSx225kFlag)
	{
		u8DVBSx255kSupport = 1;
	}
	else
	{
		u8DVBSx255kSupport = 0;
		RTK_A_DVBSX_SetDvbSxModeOnly(pDemod); //Set DVB-S/S2 mode.
	}
}

/*Internal used function*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvBSxRegs(DVBSX_DEMOD_MODULE *pDemod, BOOL BypassAGC)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned char RegVal = 0;

	//bypass_agc
	if(BypassAGC)
	{
		RTK_A_DVBSX_Write(pDemod, 0x10, 0x2a, 0x01);
	}
	else
	{
		RTK_A_DVBSX_Write(pDemod, 0x10, 0x2a, 0x00);
	}

	//dagc_xmin
	RTK_A_DVBSX_Write(pDemod, 0x08, 0x1E, 0x06);

	//std_target_cfo
	RTK_A_DVBSX_Write(pDemod, 0x08, 0x1C, 0x0e);

	//params_min_group_spac
	RTK_A_DVBSX_Write(pDemod, 0x09, 0x6E, 0x50);

	//params_envelop_got_block_len
	RTK_A_DVBSX_Write(pDemod, 0x09, 0xb6, 0x20);

	//dvbc_bw_bypass
	RTK_A_DVBSX_Write(pDemod, 0x15, 0x3F, 0x00);

	//dvbs_mode_r
	RTK_A_DVBSX_Write(pDemod, 0x00, 0x2b, 0x01);

	//rst
	RTK_A_DVBSX_Write(pDemod, 0x00, 0x09, 0x10);
	RTK_A_DVBSX_Write(pDemod, 0x00, 0x0a, 0x00);
	RTK_A_DVBSX_Write(pDemod, 0x00, 0x14, 0x0C); // 4->c , 10.13, xwang

	//clk_force
	RTK_A_DVBSX_Write(pDemod, 0x00, 0x40, 0x00);
	RTK_A_DVBSX_Write(pDemod, 0x00, 0x41, 0x20);
	RTK_A_DVBSX_Write(pDemod, 0x00, 0x42, 0x00);
	RTK_A_DVBSX_Write(pDemod, 0x00, 0x43, 0xff);

	//params_blind_scan_state
	RTK_A_DVBSX_Write(pDemod, 0x09, 0xA6, 0x00);

	//DVB-S PER window
	RTK_A_DVBSX_Write(pDemod, 0x95, 0x21, 0x08);

	RTK_A_DVBSX_Write(pDemod, 0x16, 0xc4, 0x3c);
	
	RTK_A_DVBSX_Write(pDemod, 0x18, 0x8f, 0x72); //0x72 1s. for FPGA set 0.2s.
	RTK_A_DVBSX_Write(pDemod, 0x95, 0x60, 0x00);
	RTK_A_DVBSX_Write(pDemod, 0x95, 0x61, 0x08);

	//11.3 mac use previous value.
	RTK_A_DVBSX_Write(pDemod, 0x17, 0x44, 0x04);
	RTK_A_DVBSX_Write(pDemod, 0x17, 0x46, 0x10);
	RTK_A_DVBSX_Write(pDemod, 0x17, 0x4c, 0x10);
	RTK_A_DVBSX_Write(pDemod, 0x17, 0x4e, 0x40);
	RTK_A_DVBSX_Write(pDemod, 0x17, 0x53, 0xbe);
	RTK_A_DVBSX_Write(pDemod, 0x17, 0x58, 0x20);
	RTK_A_DVBSX_Write(pDemod, 0x17, 0xc3, 0x80);
	RTK_A_DVBSX_Write(pDemod, 0x17, 0xc4, 0x00);
	RTK_A_DVBSX_Write(pDemod, 0x17, 0xfd, 0x00);

	if(gstRtkDemodDvbConfig.DvbSxTunerInput.ui8SxIFAgcPolarity == RTK_DEMOD_IF_AGC_POLARITY_INVERTED)
	{
		RTK_A_DVBSX_Read(pDemod, 0x00, 0x20, &RegVal);
		RegVal |= 0x02;
		RTK_A_DVBSX_Write(pDemod, 0x00, 0x20, RegVal); //5815m/av2018 AGC need to reverse
	}
	//RTK_A_DVBSX_Write(pDemod, 0x00, 0x04, 0x01);
	
	return status;
}

/*Internal used function*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvBSxSymbolAuto(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;

	RTK_A_DVBSX_Write(pDemod, 0x09, 0xa6, 0x00); // params_blind_scan_state

	RTK_A_DVBSX_Write(pDemod, 0x15, 0x59, 0x00); // symbol_rate_force_en
	RTK_A_DVBSX_Write(pDemod, 0x15, 0x95, 0x00); // symbol_rate_point_force_en symbol_rate_point_use_reg // not used
	RTK_A_DVBSX_Write(pDemod, 0x15, 0x92, 0xa0); // symbol_rate_point_force u(19,5) = 20*32768/120*2^5
	RTK_A_DVBSX_Write(pDemod, 0x15, 0x93, 0xaa);
	RTK_A_DVBSX_Write(pDemod, 0x15, 0x94, 0x02);
	RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0x01); // Reset DSP
	RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0x00);

	return status;
}

/********************************************************************************
function    :RTK_A_DVBSX_GetLockingStatus
parameter   :stRtkDemodLockingStatus .
return      :Operate Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal.
Description
Get different locked flags for DVB-S/S2.
The locked flag in sequency is
1, Timing lock.
2, Sync lock
3, FEC lock
4, TS lock
*********************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetLockingStatus(DVBSX_DEMOD_MODULE *pDemod, stRTK_LOCKING_STATUS *stRtkDemodLockingStatus)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned char u8tmp = 0;
	unsigned char u8FecLockFlag = 0;
	unsigned char u8TSLockFlag = 0;

	/*Locking status init as default value.**/
	stRtkDemodLockingStatus->u8TimingLockFlag = 0;
	stRtkDemodLockingStatus->u8SyncLockFlag = 0;
	stRtkDemodLockingStatus->u8FecLockFlag = 0;
	stRtkDemodLockingStatus->u8TSLockFlag = 0;
	stRtkDemodLockingStatus->u8PLLLockFlag = 0; //add PLL lock flag.

	//Check Timing lock status
	status |= RTK_A_DVBSX_Read(pDemod, 0x2b, 0x0d, &u8tmp);
	if((u8tmp&0x03) == 0x02)
	{
		stRtkDemodLockingStatus->u8TimingLockFlag = 1;
	}

	//Check Sync lock status
	status |= RTK_A_DVBSX_Read(pDemod, 0x2c, 0x17, &u8tmp);
	if((u8tmp&0x03) == 0x01)
	{ 
		stRtkDemodLockingStatus->u8SyncLockFlag = 1; //S sync
	}
	status |= RTK_A_DVBSX_Read(pDemod, 0x2c, 0x18, &u8tmp);
	if((u8tmp&0x07) == 0x04)
	{
		stRtkDemodLockingStatus->u8SyncLockFlag = 1; //S2 sync
	}

	//Check FEC lock status
	RTK_A_DVBSX_FECLockFlag(pDemod, &u8FecLockFlag);
	if(u8FecLockFlag)
	{
		stRtkDemodLockingStatus->u8FecLockFlag = 1;
	}

	//Check TS lock status
	RTK_A_DVBSX_TSLockFlag(pDemod, &u8TSLockFlag);
	if(u8TSLockFlag)
	{
		stRtkDemodLockingStatus->u8TSLockFlag = 1;
	}

	return status;
}

RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindScanCheckSymbolRate(DVBSX_DEMOD_MODULE *pDemod, unsigned int *u32symbolRateHz) 
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned int u32symbolRate = 0;
	unsigned int u32timeout = 200; //200ms
	unsigned long stime = 0;

	*u32symbolRateHz = gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz;
	if(gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz < 6000000)
	{
		u32timeout = 400; //400ms
	}
	if(gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz < 3500000)
	{
		u32timeout = 500; //500ms
	}

	stime = tv_osal_time();
	while((tv_osal_time() - stime) < u32timeout)
	{
		status |= RTK_A_DVBSX_GetDVBSxSymbolRate(pDemod, &u32symbolRate);
		if(RTK_A_DVBSX_AbsSub(gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz, u32symbolRate) < gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz/100)
		{
			*u32symbolRateHz = u32symbolRate;
			break;
		}	
		tv_osal_msleep(10);
	}
	*u32symbolRateHz = u32symbolRate;

	return status;
}

/********************************************************************************
Function    :RTK_A_DVBSX_ChannelLockCheck
Parameter   :pu8DvbSxLockStatus
Return      :Operate Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal.
Description
Repeat checking demodulator's lock status until timeout. 
this function can be used in DVB-S, DVB-S2 channel locking or channel scanning.
if *pu8DvbSxLockStatus == 1, signal locked. else signal not locked.
*********************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_ChannelLockCheck(DVBSX_DEMOD_MODULE *pDemod, unsigned char *pu8DvbSxLockStatus) /*Check DVB-S_S2 lock status.This function checks lock status within a time window related with symbol rate detected*/
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned char u8TimingSyncFlag = 0, u8FrameSyncFlag = 0, LiteCfoCnt = 0, ScanCnt = 0, CfoTableIdx = 0, FoundCfoIdx = 0;
	stRTK_LOCKING_STATUS sRTKLockingStatus;
	unsigned int u32AcqTimeOut = 2000; /*default check times = 2s*/
	unsigned int u32LowSrAcqTimeOut = 2200; /*Maximum check times = 2.2s*/
	unsigned long stime = 0;
	unsigned int u32TimingUnsyncTimeOut = 300;
	unsigned int u32FrameUnsyncTimeOut = 1000;
	unsigned int u32LiteSrTimingUnsyncTimeOut = 800;
	short FoundCfo = 0;
	unsigned char u8TimingLocked = 0, u8LockCnt = 0;

	if((gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz < 198000)||(gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz > 45100000))
	{
		REALTEK_A_DVBSX_INFO("Error! symbol rate not in valid range\n");
		gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz = 5000000;
	}

	//very small symbol rate.
	if(gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz < RTK_DVBSX_LITE_SYMBOL_HZ)
	{
		stRTK_LOCKING_STATUS sRTKLockingStatus;
		unsigned int u32symbolRateHz = 0;

		//New Lite Table start===
		for( ScanCnt = 0; ScanCnt < MAX_LITE_SR_SIGNAL_NUM; ScanCnt++)
		{
			if(gLiteymbolRateSignalList.LiteSignalList[ScanCnt].SymbolRateHz == gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz && gLiteymbolRateSignalList.LiteSignalList[ScanCnt].FreqKhz == gstRtkDemodDvbConfig.DvbSxTunerInput.ui32FreqKhz)
			{
				REALTEK_A_DVBSX_INFO("Found [DigMixer %u] %d\n", ScanCnt, gLiteymbolRateSignalList.LiteSignalList[ScanCnt].Cfo);
				FoundCfoIdx = ScanCnt;
				FoundCfo = gLiteymbolRateSignalList.LiteSignalList[ScanCnt].Cfo;

				RTK_A_DVBSX_ForceSymbolRateandCFO(pDemod, gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz/1000, gLiteymbolRateSignalList.LiteSignalList[ScanCnt].Cfo);
				RTK_A_DVBSX_BlindScanCheckSymbolRate(pDemod, &u32symbolRateHz);
				if(u32symbolRateHz)
				{
					stime = tv_osal_time();
					while((tv_osal_time() - stime) < u32LowSrAcqTimeOut)
					{
						RTK_A_DVBSX_GetLockingStatus(pDemod, &sRTKLockingStatus);
						if(((tv_osal_time() - stime) > u32LiteSrTimingUnsyncTimeOut) && (sRTKLockingStatus.u8TimingLockFlag == 0)) //Timing lock time out
						{
							REALTEK_A_DVBSX_INFO("Timing lock time = %u\n ", u32LiteSrTimingUnsyncTimeOut);
							break;
						}
						if(sRTKLockingStatus.u8TSLockFlag)
						{
							*pu8DvbSxLockStatus = sRTKLockingStatus.u8TSLockFlag;
							return status;
						}
						tv_osal_msleep(10);
					}
				}

				REALTEK_A_DVBSX_INFO("Retry [DigMixer] %d\n", gLiteymbolRateSignalList.LiteSignalList[ScanCnt].Cfo);
				RTK_A_DVBSX_ForceSymbolRateandCFO(pDemod, gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz/1000, gLiteymbolRateSignalList.LiteSignalList[ScanCnt].Cfo);
				RTK_A_DVBSX_BlindScanCheckSymbolRate(pDemod, &u32symbolRateHz);
				if(u32symbolRateHz)
				{
					stime = tv_osal_time();
					while((tv_osal_time() - stime) < u32LowSrAcqTimeOut)
					{
						RTK_A_DVBSX_GetLockingStatus(pDemod, &sRTKLockingStatus);
						if(((tv_osal_time() - stime) > u32LiteSrTimingUnsyncTimeOut) && (sRTKLockingStatus.u8TimingLockFlag == 0)) //Timing lock time out
						{
							REALTEK_A_DVBSX_INFO("Timing lock time = %u\n ", u32LiteSrTimingUnsyncTimeOut);
							break;
						}
						if(sRTKLockingStatus.u8TSLockFlag)
						{
							*pu8DvbSxLockStatus = sRTKLockingStatus.u8TSLockFlag;
							return status;
						}
						tv_osal_msleep(10);
					}
				}

				//Find CFO index
				if(FoundCfo == 0)
				{
					LiteCfoCnt = CfoTableIdx = 0;
				}
				else
				{
					while(CfoTableIdx < 64)
					{
						if (FoundCfo == LiteCfoTable[CfoTableIdx])
						{
							REALTEK_A_DVBSX_INFO("Mapping [DigMixer %u] %d \n", CfoTableIdx, LiteCfoTable[CfoTableIdx]);
							LiteCfoCnt = CfoTableIdx;
							break;
						}
						CfoTableIdx = CfoTableIdx + 8;
					}
				}
			}
		}

		REALTEK_A_DVBSX_INFO("LiteCfoCnt=%u  CfoTableIdx=%u \n", LiteCfoCnt, CfoTableIdx);
		while(LiteCfoCnt < (CfoTableIdx+8))
		{
			REALTEK_A_DVBSX_INFO("New [DigMixer %u] %d \n", LiteCfoCnt, LiteCfoTable[LiteCfoCnt]);
			RTK_A_DVBSX_ForceSymbolRateandCFO(pDemod, gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz/1000, LiteCfoTable[LiteCfoCnt]);
			RTK_A_DVBSX_BlindScanCheckSymbolRate(pDemod, &u32symbolRateHz);
			if(u32symbolRateHz)
			{
				stime = tv_osal_time();
				while((tv_osal_time() - stime) < u32LowSrAcqTimeOut)
				{
					RTK_A_DVBSX_GetLockingStatus(pDemod, &sRTKLockingStatus);
					if(((tv_osal_time() - stime) > u32LiteSrTimingUnsyncTimeOut) && (sRTKLockingStatus.u8TimingLockFlag == 0)) //Timing lock time out
					{
						REALTEK_A_DVBSX_INFO("Timing lock time = %u\n ", u32LiteSrTimingUnsyncTimeOut);
						break;
					}
					if(sRTKLockingStatus.u8TSLockFlag)
					{
						*pu8DvbSxLockStatus = sRTKLockingStatus.u8TSLockFlag;
						if(FoundCfo == 0)
						{
							gLiteymbolRateSignalList.LiteSignalList[gLiteymbolRateSignalList.u8LiteSignalNum].FreqKhz = gstRtkDemodDvbConfig.DvbSxTunerInput.ui32FreqKhz;
							gLiteymbolRateSignalList.LiteSignalList[gLiteymbolRateSignalList.u8LiteSignalNum].SymbolRateHz = gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz;
							gLiteymbolRateSignalList.LiteSignalList[gLiteymbolRateSignalList.u8LiteSignalNum].Cfo = LiteCfoTable[LiteCfoCnt];
							REALTEK_A_DVBSX_INFO("Record[%u] Freq=%u SR=%u CFO=%d\n", gLiteymbolRateSignalList.u8LiteSignalNum, gLiteymbolRateSignalList.LiteSignalList[gLiteymbolRateSignalList.u8LiteSignalNum].FreqKhz, gLiteymbolRateSignalList.LiteSignalList[gLiteymbolRateSignalList.u8LiteSignalNum].SymbolRateHz, gLiteymbolRateSignalList.LiteSignalList[gLiteymbolRateSignalList.u8LiteSignalNum].Cfo);
							gLiteymbolRateSignalList.u8LiteSignalNum++;
						}
						else
						{
							//update CFO only
							gLiteymbolRateSignalList.LiteSignalList[FoundCfoIdx].Cfo = LiteCfoTable[LiteCfoCnt];
							REALTEK_A_DVBSX_INFO("Update CFO only[%u] CFO=%d\n", FoundCfoIdx, gLiteymbolRateSignalList.LiteSignalList[FoundCfoIdx].Cfo);
						}

						if(gLiteymbolRateSignalList.u8LiteSignalNum > (MAX_LITE_SR_SIGNAL_NUM - 1)) 
							gLiteymbolRateSignalList.u8LiteSignalNum = 0;

						return status;
					}
					tv_osal_msleep(10);
				}
			}
			LiteCfoCnt++;
		}
		//New Lite Table end===

		*pu8DvbSxLockStatus = sRTKLockingStatus.u8TSLockFlag;
		return status;
	}

	/*Typically the TimeOut time set as Double of the real locking time. */ 
	if(gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz >= 20000000) // 20M -- 45M symbol rate
	{
		u32AcqTimeOut = 1000;//2200;
		u32TimingUnsyncTimeOut = 200;//180 + 20; //60->300->180
		u32FrameUnsyncTimeOut = 240;//90->340->240
	}
	else if(gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz >= 10000000) // 10M -- 20M symbol rate
	{
		u32AcqTimeOut = 1500;//2500;
		u32TimingUnsyncTimeOut = 230;//200 + 30; //80->200
		u32FrameUnsyncTimeOut = 240;//120->240
	}
	else if(gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz >= 5000000) // 5M -- 10M symbol rate
	{
		u32AcqTimeOut = 1800;//2800;
		u32TimingUnsyncTimeOut = 240;//((60*1000000)/gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz)*10 + 180;
		u32FrameUnsyncTimeOut = 300;//((120*1000000)/gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz)*10 + 180;
	}
	else if(gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz >= 2000000) // 2M -- 5M symbol rate
	{
		u32AcqTimeOut = 2300;//3300;
		u32TimingUnsyncTimeOut = 380;//((60*1000000)/gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz)*10 + 260;
		u32FrameUnsyncTimeOut = 500;//((120*1000000)/gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz)*10 + 260;
	}
	else //<2M symbol rate
	{
		u32AcqTimeOut = 3000;//4000;
		u32TimingUnsyncTimeOut = 600;//((60*1000000)/gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz)*10 + 300;
		u32FrameUnsyncTimeOut = 900;//((120*1000000)/gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz)*10 + 300;
	}

	*pu8DvbSxLockStatus = 0; /*default assuming not locked*/
	if(gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz > 29700000 && gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz < 29740000)
	{
		u32AcqTimeOut = 5000;
		u32TimingUnsyncTimeOut = 500;
		REALTEK_A_DVBSX_INFO("Lock Time Out = %u, No Signal Time Out = %u\n", u32AcqTimeOut, u32TimingUnsyncTimeOut);
		
		stime = tv_osal_time();
		while((tv_osal_time() - stime) < u32AcqTimeOut)
		{
			status |= RTK_A_DVBSX_GetLockingStatus(pDemod, &sRTKLockingStatus);

			if(sRTKLockingStatus.u8TimingLockFlag == 1 && u8TimingLocked == 0) //If Timing locked at least one time, set u8TimingSyncFlag locked
			{
				u8TimingSyncFlag = 1;
				u8TimingLocked = 1;
				REALTEK_A_DVBSX_INFO("TIMING lock!!! %lu ms\n ", tv_osal_time() - stime);
			}
			if(sRTKLockingStatus.u8SyncLockFlag == 1) //If Frame sync locked at least one time, set u8FrameSyncFlag locked
			{
				u8FrameSyncFlag = 1;
			}

			if(((tv_osal_time() - stime) > u32TimingUnsyncTimeOut) && (u8TimingLocked == 0)) //Timing lock time out.
			{
				REALTEK_A_DVBSX_INFO("Timing UnSync time out = %u\n ", u32TimingUnsyncTimeOut);
				*pu8DvbSxLockStatus = 0;
				return status;
			}

			//if(((tv_osal_time() - stime) > u32FrameUnsyncTimeOut) && (u8FrameSyncFlag == 0)) //Frame sync lock time out.
			//{
			//	REALTEK_A_DVBSX_INFO("Frame sync time out = %u\n ", u32FrameUnsyncTimeOut);
			//	*pu8DvbSxLockStatus = 0; //not check frame sync
			//	return status;
			//}

			if(sRTKLockingStatus.u8FecLockFlag == 1 || sRTKLockingStatus.u8TSLockFlag == 1) //FEC locked, quit the checking process.
			{
				u8LockCnt++;
				//REALTEK_A_DVBSX_INFO("FEC/TS lock (%u) !!!\n", u8LockCnt);
				if(u8LockCnt >= 4)
				{
					*pu8DvbSxLockStatus = 1;
					REALTEK_A_DVBSX_INFO("FEC/TS lock finished !!!\n");
					return status;
				}
				tv_osal_msleep(40);
			}
			else
			{
				u8LockCnt = 0;
			}
			tv_osal_msleep(10);
		}
	}
	else
	{
		REALTEK_A_DVBSX_INFO("Lock Time Out = %u, No Signal Time Out = %u\n", u32AcqTimeOut, u32TimingUnsyncTimeOut);
		stime = tv_osal_time();
		while((tv_osal_time() - stime) < u32AcqTimeOut)
		{
			status |= RTK_A_DVBSX_GetLockingStatus(pDemod, &sRTKLockingStatus);

			if(sRTKLockingStatus.u8TimingLockFlag == 1 && u8TimingLocked == 0) //If Timing locked at least one time, set u8TimingSyncFlag locked
			{
				u8TimingSyncFlag = 1;
				u8TimingLocked = 1;
				REALTEK_A_DVBSX_INFO("TIMING lock!!! %lu ms\n ", tv_osal_time() - stime);
			}
			if(sRTKLockingStatus.u8SyncLockFlag == 1) //If Frame sync locked at least one time, set u8FrameSyncFlag locked
			{
				u8FrameSyncFlag = 1;
			}

			if(((tv_osal_time() - stime) > u32TimingUnsyncTimeOut) && (u8TimingSyncFlag == 0)) //Timing lock time out.
			{
				REALTEK_A_DVBSX_INFO("Timing UnSync time out = %u\n ", u32TimingUnsyncTimeOut);
				*pu8DvbSxLockStatus = 0;
				return status;
			}

			//if(((tv_osal_time() - stime) > u32FrameUnsyncTimeOut) && (u8FrameSyncFlag == 0)) //Frame sync lock time out.
			//{
			//	REALTEK_A_DVBSX_INFO("Frame sync time out = %u\n ", u32FrameUnsyncTimeOut);
			//	*pu8DvbSxLockStatus = 0; //not check frame sync
			//	return status;
			//}

			if(sRTKLockingStatus.u8FecLockFlag == 1) //FEC locked, quit the checking process.
			{
				*pu8DvbSxLockStatus = 1;
				REALTEK_A_DVBSX_INFO("FEC lock!!!\n");
				return status;
			}
			if(sRTKLockingStatus.u8TSLockFlag == 1) //TS locked, quit the checking process.
			{
				*pu8DvbSxLockStatus = 1;
				REALTEK_A_DVBSX_INFO("TS lock!!!\n");
				return status;
			}
			tv_osal_msleep(10);
		}
	}

	return status;
}

/********************************************************************************
Function    :RTK_A_DVBSX_BlindscanChannelLockCheck
Parameter   :pu8DvbSxLockStatus
Return      :Operate Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal.
Description
Repeat checking demodulator's lock status until timeout. 
this function can be used in DVB-S, DVB-S2 channel locking or channel scanning.
if *pu8DvbSxLockStatus == 1, signal locked. else signal not locked.
*********************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_BlindscanChannelLockCheck(DVBSX_DEMOD_MODULE *pDemod, unsigned char *pu8DvbSxLockStatus) /*Check DVB-S_S2 lock status.This function checks lock status within a time window related with symbol rate detected*/
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned char u8TimingSyncFlag = 0;
	stRTK_LOCKING_STATUS sRTKLockingStatus;
	unsigned int u32AcqTimeOut = 2000; /*default check times = 2s*/
	unsigned long stime = 0;
	unsigned int u32TimingUnsyncTimeOut = 300;
	unsigned char u8TimingLocked = 0;

	/*Typically the TimeOut time set as Double of the real locking time. */ 
	if(gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz >= 20000000) // 20M--45M symbol rate
	{
		u32AcqTimeOut = 1000;
		u32TimingUnsyncTimeOut = 300;//60->300->180
	}
	else if(gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz >= 10000000) // 10M--20M symbol rate
	{
		u32AcqTimeOut = 1500;
		u32TimingUnsyncTimeOut = 330;//80->200
	}
	else if(gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz >= 5000000) // 5M--10M symbol rate
	{
		u32AcqTimeOut = 1800;
		u32TimingUnsyncTimeOut = 340;//((60*1000000)/gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz)*10 + 180;
	}
	else if(gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz >= 2000000) // 2M--5M symbol rate
	{
		u32AcqTimeOut = 2300;
		u32TimingUnsyncTimeOut = 480;//((60*1000000)/gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz)*10 + 260;
	}
	else //<2M symbol rate
	{
		u32AcqTimeOut = 3000;
		u32TimingUnsyncTimeOut = 700;//((60*1000000)/gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz)*10 + 300;
	}
	REALTEK_A_DVBSX_INFO("Demod Lock Time Out = %u, Timing Time Out = %u\n", u32AcqTimeOut, u32TimingUnsyncTimeOut);

	*pu8DvbSxLockStatus = 0; /*default assuming not locked*/

	stime = tv_osal_time();
	while((tv_osal_time() - stime) < u32AcqTimeOut)
	{
		status |= RTK_A_DVBSX_GetLockingStatus(pDemod, &sRTKLockingStatus);

		if(sRTKLockingStatus.u8TimingLockFlag == 1 && u8TimingLocked == 0) //If Timing locked at least one time, set u8TimingSyncFlag locked
		{
			u8TimingSyncFlag = 1;
			u8TimingLocked = 1;
			REALTEK_A_DVBSX_INFO("TIMING lock!!! %lu ms\n ", tv_osal_time() - stime);
			if(gDvbSxBsSpecialParams.u8UnicableEnabled)
			{
				if(gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz < 20000000 && gDvbSxBsSpecialParams.u8IsSpectrumCheckDone)
				{
					*pu8DvbSxLockStatus = 1;
					REALTEK_A_DVBSX_INFO("Unicable Mode SR<20M, Only Check Timing Lock\n");
					return status;
				}
				else
				{
					REALTEK_A_DVBSX_INFO("Unicable Mode SR>=20M or Not Check Spectrum, Need Check FEC Lock\n");
				}
			}
			else
			{
				if(gstRtkDemodDvbConfig.DvbSxTunerInput.ui32SymbolRateHz < 20000000)
				{
					*pu8DvbSxLockStatus = 1;
					REALTEK_A_DVBSX_INFO("Normal Mode SR<20M, Only Check Timing Lock\n");
					return status;
				}
				else
				{
					REALTEK_A_DVBSX_INFO("Normal Mode SR>=20M, Need Check FEC Lock\n");
				}
			}
		}

		if(((tv_osal_time() - stime) > u32TimingUnsyncTimeOut) && (u8TimingSyncFlag == 0)) //Timing lock time out.
		{
			REALTEK_A_DVBSX_INFO("Timing UnSync time out = %u\n", u32TimingUnsyncTimeOut);
			*pu8DvbSxLockStatus = 0;
			return status;
		}

		if(sRTKLockingStatus.u8FecLockFlag == 1) //FEC locked, quit the checking process.
		{
			*pu8DvbSxLockStatus = 1;
			REALTEK_A_DVBSX_INFO("FEC LOCK!!!\n");
			RTK_A_DVBSX_GetDvBSxSpectrum(pDemod, &gDvbSxBsSpecialParams.pSignalSpectrum);
			gDvbSxBsSpecialParams.u8IsSpectrumCheckDone = 1;
			return status;
		}
		if(sRTKLockingStatus.u8TSLockFlag == 1) //TS locked, quit the checking process.
		{
			*pu8DvbSxLockStatus = 1;
			REALTEK_A_DVBSX_INFO("TS LOCK!!!\n");
			RTK_A_DVBSX_GetDvBSxSpectrum(pDemod, &gDvbSxBsSpecialParams.pSignalSpectrum);
			gDvbSxBsSpecialParams.u8IsSpectrumCheckDone = 1;
			return status;
		}
		tv_osal_msleep(10);
	}

	return status;
}

RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxPER(DVBSX_DEMOD_MODULE *pDemod, unsigned int *pui32PER) /*Get DVB-S RS PER*E9 or DVB-S2 BCH Error rate, PER is error rate after FEC bit corrected.*/
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;

	*pui32PER = 1000000000;

	status = RTK_A_DVBSX_GetPER(pDemod, pui32PER);

	return status;
}

RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxBER(DVBSX_DEMOD_MODULE *pDemod, unsigned int *pui32BER)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned char u8DVBSxMode = 0;

	*pui32BER = 1000000000; //init as 0.

	status = RTK_A_DVBSX_DVBSxModeDetected(pDemod, &u8DVBSxMode);

	if(u8DVBSxMode == RTK_DEMOD_DVBS2_MODE) /*DVBS2 mode*/
	{
		status |= RTK_A_DVBSX_GetDVBS2BER(pDemod, pui32BER);
	}
	if(u8DVBSxMode == RTK_DEMOD_DVBS_MODE) /*DVBS mode*/
	{
		status |= RTK_A_DVBSX_GetDVBSBER(pDemod, pui32BER);
	}

	return status;
}

RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxBEC(DVBSX_DEMOD_MODULE *pDemod, unsigned int *pui32BEC_NUM, unsigned int *pui32BEC_DEN)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned char u8DVBSxMode = 0;

	*pui32BEC_NUM = -1; //init as 0.
	*pui32BEC_DEN = -1;

	status = RTK_A_DVBSX_DVBSxModeDetected(pDemod, &u8DVBSxMode);

	if(u8DVBSxMode == RTK_DEMOD_DVBS2_MODE) /*DVBS2 mode*/
	{
		status |= RTK_A_DVBSX_GetDVBS2BEC(pDemod, pui32BEC_NUM, pui32BEC_DEN);
	}
	if(u8DVBSxMode == RTK_DEMOD_DVBS_MODE) /*DVBS mode*/
	{
		status |= RTK_A_DVBSX_GetDVBSBEC(pDemod, pui32BEC_NUM, pui32BEC_DEN);
	}

	return status;
}

/********************************************************************************
Function    :RTK_A_DVBSX_GetDVBSxSignalQuality
Parameter   :pu8SQI
Return      :Operate Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal.
Description
Get DVB-S/S2/S2x evaluated signal quality, the returned signal quality is calculated from SNR.
signal quality range: 0 -> 100
*********************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxSignalQuality(DVBSX_DEMOD_MODULE *pDemod, unsigned char *pu8SQI)
{
	unsigned int u32Reg = 0, ui32PER = 0;
	int i32SNRx10 = 0;
	unsigned char ui8HeaderValue;
	int i32SNRRefer = 0, i32SNRx10Plus;
	unsigned char u8QualityValue = 0, u8fecLockStatus = 0, u8TsLockStatus = 0;
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned char u8DVBSMode = 0, u8DVBS2Mode = 0, u8DVBSxMode, u8Rxheader;
	unsigned char DvbsSNR[5] = {33, 51, 61, 71, 77};

	/*DVBS2*/
	//unsigned char Dvbs2QpskSNR[8] = {11, 24, 33, 41, 47, 52, 63, 65};
	//unsigned char Dvbs28pskSNR[6] = {55, 65, 81, 94, 100, 110};

	/*DVBS2_S2x*/
	int DVBS2_S2xMode[54] =
	{
		-21,	-12,	-3,		11,		24,		33,		41,		47,		52,		63,		65,					/*QPSK*/
		55,		65,		81,		94,		100,	110,														/*8PSK*/
		93,		108,	116,	122,	134,	139,	135,	148,	152,	172,	174,				/*16APSK and 32APSK*/
		0,		6,		18,		49,		55,		67,		74,		78,		63,		71,		72,		79,		84,	/*DVBS2x*/
		88,		87,		89,		76,		98,		103,	112,	125,	117,	125,	125,	129,	138	/*DVBS2x*/
	};

	RTK_A_DVBSX_DVBSxModeDetected(pDemod, &u8DVBSxMode);

	if(u8DVBSxMode == RTK_DEMOD_DVBS2_MODE) /*DVBS2 mode*/
	{
		u8DVBS2Mode = 1;
	}
	if(u8DVBSxMode == RTK_DEMOD_DVBS_MODE) /*DVBS mode*/
	{
		u8DVBSMode = 1;
	}
	if((u8DVBSxMode!= RTK_DEMOD_DVBS2_MODE) && (u8DVBSxMode != RTK_DEMOD_DVBS_MODE))
	{
		*pu8SQI = 0;
		return status;
	}

	*pu8SQI = 0;
	/*Set TS lock checking timeout value.*/
	if((u8DVBS2Mode == 1) && (status == RTK_DEMOD_SUCCESSFUL)) /*S2*/
	{
		status |= RTK_A_DVBSX_Read(pDemod, 0x2c, 0x28, &u8Rxheader);
		ui8HeaderValue = (unsigned char)(u8Rxheader&0xFF);
		if(u8Rxheader <= 113 && u8Rxheader >= 4) //4~113 step size is 4
		{
			i32SNRRefer = DVBS2_S2xMode[ui8HeaderValue/4 -1];
		}
		else if(ui8HeaderValue <= 183 && ui8HeaderValue >= 132)//132~183 step size is 2
		{
			i32SNRRefer = DVBS2_S2xMode[ui8HeaderValue/2 -38];
		}
		else
		{
			i32SNRRefer = 0;
		}
	}
	if(u8DVBSMode == 1) /*DVB-S, QPSK*/
	{
		status |= RTK_A_DVBSX_Read(pDemod, 0x2c, 0x15, &u8Rxheader);
		u8Rxheader &= 0x3f;/**/
		if(u8Rxheader <= 1)//1/2
		{
			u32Reg = 0;
		}
		else if(u8Rxheader <= 7)//2/3
		{
			u32Reg = 1;
		}
		else if(u8Rxheader <= 11)//3/4
		{
			u32Reg = 2;
		}
		else if(u8Rxheader <= 17)//5/6
		{
			u32Reg = 3;
		}
		else if(u8Rxheader <= 25)//7/8
		{
			u32Reg = 4;
		}
		else
		{
			u32Reg = 5;
		}
		if(u32Reg <= 4)
		{
			i32SNRRefer = DvbsSNR[u32Reg];
			/*i32DvbsCRmx100 = DvbsCRmx100[u32Reg];*/
		}
		/*logmx100 = 301;*/
	}
	status |= RTK_A_DVBSX_GetDVBSxSNR(pDemod, &i32SNRx10);
	//REALTEK_A_DVBSX_INFO("RTK SNRx10 = %d!\n", i32SNRx10);
	RTK_A_DVBSX_GetDVBSxPER(pDemod, &ui32PER);
	RTK_A_DVBSX_FECLockFlag(pDemod, &u8fecLockStatus);
	RTK_A_DVBSX_TSLockFlag(pDemod, &u8TsLockStatus);
	//if((i32SNRx10 < 100) && (i32SNRx10 > 50))
	//{
	//	i32SNRx10 = i32SNRx10 - (100 - i32SNRx10)/7; /*Calibrate CN value.*/
	//}

	/* Eb_NoX10 = u32SNRx10*10 ¨C 10log(188/204) ¨C 10log(m) ¨C 10log(code_rate) + imp_margin_X10*/
	if(u8DVBSMode == 0x00) /*DVB-S, QPSK*/
	{
		/* Eb_NoX10 = (i32DvbsCRmx100 + u32SNRx10*10 + imp_margin_X10 * 10 - 35 - logmx100)/10;*/
		/* u32SNRx10 = Eb_NoX10;*/
	}
	/*Evaluation of signal quality*/
	if((i32SNRx10 + 5) >= i32SNRRefer) //Signal can be received.
	{
		i32SNRx10Plus = i32SNRx10 - i32SNRRefer;

		if(i32SNRx10Plus >= 50) /* > 5dB _ refCn*/
			u8QualityValue = 100;
		else if(i32SNRx10Plus >= 0) /*5dB*/
			u8QualityValue = (unsigned char)(50 + i32SNRx10Plus);
		else
			u8QualityValue = (unsigned char)(45);

		/*Calibrate SQI 2016.12.16 */
		if(ui32PER == 0)
		{
			u8QualityValue = u8QualityValue + 15;
		}
		else if(ui32PER < 10000000)
		{
			u8QualityValue = u8QualityValue + 10;
		}
		else
		{
			u8QualityValue = u8QualityValue + 5;
		}
		//Check TS status
		if(u8TsLockStatus) 
		{
			u8QualityValue = u8QualityValue + 2;
		}
		else //TS error.
		{
			u8QualityValue = 40;
		}
		if(!u8fecLockStatus)
		{
			 u8QualityValue = 20;
		}

		if(u8QualityValue > 100)
		{
			u8QualityValue = 100;
		}
	}
	else
	{
		if((u8fecLockStatus) && (u8TsLockStatus)) //FEC locked.
		{
			u8QualityValue = 40;
		}
		else //signal can not be locked, only mode can be detected.
		{
			u8QualityValue = 10;
		}
	}

	if((u8QualityValue < 25) && (u8QualityValue >= 2))
	{
		u8QualityValue = u8QualityValue - 2;
	}
	if((u8QualityValue < 15) && (u8QualityValue > 3))
	{
		u8QualityValue = u8QualityValue - 3;
	}
	*pu8SQI = u8QualityValue;

	return status;
}

RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSBER(DVBSX_DEMOD_MODULE *pDemod, unsigned int *ui32BER)
{
	unsigned char ui8Tmp = 0;
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned short ui16WinLen = 1;
	RtkDemod64Data i64Data, i64Data1;
	unsigned int ui32Tmp = 0;

	ui8Status = RTK_A_DVBSX_LatchOn(pDemod);

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x95, 0x21, &ui8Tmp);
	ui16WinLen = ui8Tmp<<8;
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x95, 0x20, &ui8Tmp);
	ui16WinLen = ui16WinLen + ui8Tmp;

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x95, 0x69, &ui8Tmp);
	ui32Tmp = ui8Tmp<<16;
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x95, 0x23, &ui8Tmp);
	ui32Tmp = ui32Tmp + (ui8Tmp<<8);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x95, 0x22, &ui8Tmp);
	ui32Tmp = ui32Tmp + ui8Tmp;

	ui8Status |= RTK_A_DVBSX_LatchOff(pDemod);
	RTK_A_DVBSX_64Mult(&i64Data, ui32Tmp, 1000000000);
	RTK_A_DVBSX_64Mult(&i64Data1, ui16WinLen, 204*8);
	*ui32BER = RTK_A_DVBSX_64Div(i64Data, i64Data1);

	return ui8Status;
}


RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSBEC(DVBSX_DEMOD_MODULE *pDemod, unsigned int *ui32BEC_NUM, unsigned int *ui32BEC_DEN)
{
	unsigned char ui8Tmp = 0;
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned short ui16WinLen = 1;
	unsigned int ui32Tmp = 0;

	ui8Status = RTK_A_DVBSX_LatchOn(pDemod);

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x95, 0x21, &ui8Tmp);
	ui16WinLen = ui8Tmp<<8;
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x95, 0x20, &ui8Tmp);
	ui16WinLen = ui16WinLen + ui8Tmp;

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x95, 0x69, &ui8Tmp);
	ui32Tmp = ui8Tmp<<16;
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x95, 0x23, &ui8Tmp);
	ui32Tmp = ui32Tmp + (ui8Tmp<<8);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x95, 0x22, &ui8Tmp);
	ui32Tmp = ui32Tmp + ui8Tmp;

	ui8Status |= RTK_A_DVBSX_LatchOff(pDemod);
	*ui32BEC_NUM = ui32Tmp;
	*ui32BEC_DEN = ui16WinLen*204*8;

	return ui8Status;
}

/****************************************************************************
Function   :RTK_A_DVBSX_GetDVBS2BER
Parameters :ui32S2BER: Error rate real number*1e9.
Return     :Operate Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal.

Description:
Get DVB-S2 BER value,
The returned BER value is scaled by *1.0E9. This BER is not the TS output error
rate, some bit error may be corrected by BCH FEC decoder. 
******************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBS2BER(DVBSX_DEMOD_MODULE *pDemod, unsigned int *ui32S2BER)
{
	unsigned char u8Rxheader = 0, ui8RXHeader = 0, ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned char ui8PWindow[3], ui8PBERCount[3];
	static unsigned short ui16PBCHLenTable[54] = {
		16200,	21600,	25920,	32400,	38880,	43200,	48600,	51840,	54000,	57600,	58320,					//S2 QPSK
		38800,	43200,	48600,	54000,	57600,	58320,															//S2 8PSK
		43200,	48600,	51840,	54000,	57600,	58320,															//S2 16APSK
		48600,	51840,	54000,	57600,	58320,																	//S2 32APSK
		18720,	29160,	35640,																					//S2x QPSK
		36000,	37440,	41400,	45000,	46800,																	//S2x 8PSK
		32400,	35460,	36000,	37440,	38880,	38880,	40320,	41400,	43200,	45000,	46800,	50400,	55440,	//S2x 16APSK
		43200,	46080,	46080,	47520,	50400																	//S2x 32APSK
	};
	unsigned short ui16NBCHLen = 1;
	unsigned int ui32BerCalculated = 0;
	unsigned int ui32BlockWindow = 0, ui32BERCount = 0;
	RtkDemod64Data i64Data, i64Data1;
	
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x2c, 0x28, &u8Rxheader);
	ui8RXHeader = (unsigned char)(u8Rxheader&0xFF);
	if(ui8RXHeader <= 113 && ui8RXHeader >= 4) //4~113 increase by 4 steps
	{
		ui16NBCHLen = ui16PBCHLenTable[ui8RXHeader/4 -1];
	}
	else if(ui8RXHeader <= 183 && ui8RXHeader >= 132)//132~183 increase by 2 steps
	{
		ui16NBCHLen = ui16PBCHLenTable[ui8RXHeader/2 -38];
	}
	else
	{
		ui16NBCHLen = 1;
	}

	ui8Status |= RTK_A_DVBSX_LatchOn(pDemod);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x12, &ui8PWindow[2]);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x11, &ui8PWindow[1]);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x10, &ui8PWindow[0]);

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x1B, &ui8PBERCount[2]);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x1A, &ui8PBERCount[1]);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x19, &ui8PBERCount[0]);
	ui8Status |= RTK_A_DVBSX_LatchOff(pDemod);

	if(ui8Status)
	{
		*ui32S2BER = 0;
		return ui8Status;
	}

	ui32BlockWindow = (ui8PWindow[2]&0x0f)*256*256 + ui8PWindow[1]*256 + ui8PWindow[0];
	ui32BERCount = ui8PBERCount[2]*256*256 + ui8PBERCount[1]*256 + ui8PBERCount[0];

	if(ui32BlockWindow!= 0)
	{
		RTK_A_DVBSX_64Mult(&i64Data, ui32BERCount, 1000000000);
		RTK_A_DVBSX_64Mult(&i64Data1, ui32BlockWindow, ui16NBCHLen);

		/* ui32BerCalculated = ui32BERCount*1000000000/(ui32BlockWindow*ui16NBCHLen);*/
		ui32BerCalculated = RTK_A_DVBSX_64Div(i64Data, i64Data1);

	}
	*ui32S2BER = ui32BerCalculated;

	return ui8Status;
}

RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBS2BEC(DVBSX_DEMOD_MODULE *pDemod, unsigned int *ui32S2BEC_NUM, unsigned int *ui32S2BEC_DEN)
{
	unsigned char u8Rxheader=0, ui8RXHeader = 0, ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned char ui8PWindow[3], ui8PBERCount[3];
	static unsigned short ui16PBCHLenTable[54] = {
		16200,	21600,	25920,	32400,	38880,	43200,	48600,	51840,	54000,	57600,	58320,					//S2 QPSK
		38800,	43200,	48600,	54000,	57600,	58320,															//S2 8PSK
		43200,	48600,	51840,	54000,	57600,	58320,															//S2 16APSK
		48600,	51840,	54000,	57600,	58320,																	//S2 32APSK
		18720,	29160,	35640,																					//S2x QPSK
		36000,	37440,	41400,	45000,	46800,																	//S2x 8PSK
		32400,	35460,	36000,	37440,	38880,	38880,	40320,	41400,	43200,	45000,	46800,	50400,	55440,	//S2x 16APSK
		43200,	46080,	46080,	47520,	50400																	//S2x 32APSK
	};
	unsigned short ui16NBCHLen = 1;
	unsigned int ui32BlockWindow = 0, ui32BERCount = 0;
	
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x2c, 0x28, &u8Rxheader);
	ui8RXHeader = (unsigned char)(u8Rxheader&0xFF);
	if(ui8RXHeader <= 113 && ui8RXHeader >= 4) //4~113 increase by 4 steps
	{
		ui16NBCHLen = ui16PBCHLenTable[ui8RXHeader/4 -1];
	}
	else if(ui8RXHeader <= 183 && ui8RXHeader >= 132)//132~183 increase by 2 steps
	{
		ui16NBCHLen = ui16PBCHLenTable[ui8RXHeader/2 -38];
	}
	else
	{
		ui16NBCHLen = 1;
	}

	ui8Status |= RTK_A_DVBSX_LatchOn(pDemod);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x12, &ui8PWindow[2]);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x11, &ui8PWindow[1]);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x10, &ui8PWindow[0]);

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x1B, &ui8PBERCount[2]);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x1A, &ui8PBERCount[1]);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x19, &ui8PBERCount[0]);
	ui8Status |= RTK_A_DVBSX_LatchOff(pDemod);

	if(ui8Status)
	{
		*ui32S2BEC_NUM = 0;
		*ui32S2BEC_DEN = 0;
		return ui8Status;
	}

	ui32BlockWindow = (ui8PWindow[2]&0x0f)*256*256 + ui8PWindow[1]*256 + ui8PWindow[0];
	ui32BERCount = ui8PBERCount[2]*256*256 + ui8PBERCount[1]*256 + ui8PBERCount[0];

	if(ui32BlockWindow!= 0)
	{
		*ui32S2BEC_NUM = ui32BERCount;
		*ui32S2BEC_DEN = ui32BlockWindow*ui16NBCHLen;
	}

	return ui8Status;
}

//The symbol rate function may be zero when Force Fine symbol rate.
//should be changed as manual set value when timing locked && BW_NUM = 0.
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxSymbolRate(DVBSX_DEMOD_MODULE *pDemod, unsigned int *pui32SymbolRateHz) /*Get DVB-S_S2 SymbolRate.*/
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned char ui8Tmp = 0, ui8BWNum = 0, u8TimingLockFlag = 0, u8tmp = 0;
	unsigned int ui32Reg = 0, u32SymbolRate = 0;
	unsigned char data[4];
	unsigned char u8SymbolMode = 0;
	unsigned char u8SymbolIndex = 1;

	*pui32SymbolRateHz = 0;

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x59, &u8SymbolMode);

	if(u8SymbolMode == 0) //No Manual set symbol rate. auto or force fine symbol rate
	{
		ui8Status = RTK_A_DVBSX_LatchOn(pDemod);
		ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x4a, &ui8BWNum);
		ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x4d, &u8SymbolIndex);

		if((u8SymbolIndex == 2) && (ui8BWNum > 1))
		{
			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x45, &ui8Tmp);
			ui32Reg = ui8Tmp<<18;
			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x44, &ui8Tmp);
			ui32Reg += (ui8Tmp<<10);

			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x49, &ui8Tmp);
			ui32Reg += ((ui8Tmp&0x03)<<8);
			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x48, &ui8Tmp);
			ui32Reg += ui8Tmp;
		}
		else
		{
			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x43, &ui8Tmp);
			ui32Reg = ui8Tmp<<18;
			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x42, &ui8Tmp);
			ui32Reg += (ui8Tmp<<10);

			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x47, &ui8Tmp);
			ui32Reg += ((ui8Tmp&0x03)<<8);
			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x46, &ui8Tmp);
			ui32Reg += ui8Tmp;
		}

		ui8Status |= RTK_A_DVBSX_LatchOff(pDemod);
		if((ui8BWNum&0x07) > 0)
		{
			*pui32SymbolRateHz = ui32Reg; //result
		}
		else
		{
			*pui32SymbolRateHz = 0; //Default symbol rate = 0.

			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x2b, 0x0d, &u8tmp);
			if((u8tmp&0x03) == 0x02)
			{
				u8TimingLockFlag = 1;
			}
			if(u8TimingLockFlag) //Use Manual set symbol rate.
			{
				ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x55, &data[0]);
				ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x56, &data[1]);
				ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x57, &data[2]);
				ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x58, &data[3]);
				u32SymbolRate = (((data[3]&0x03)<<24) + (data[2]<<16) + (data[1]<<8) +data[0]);
				*pui32SymbolRateHz = u32SymbolRate;
			}
		}
	}
	else //Manual set symbol rate.
	{
		ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x55, &data[0]);
		ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x56, &data[1]);
		ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x57, &data[2]);
		ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x58, &data[3]);
		u32SymbolRate = (((data[3]&0x03)<<24) + (data[2]<<16) + (data[1]<<8) +data[0]);
		*pui32SymbolRateHz = u32SymbolRate;
	}

	return ui8Status;
}

/****************************************************************************
Function   :RTK_A_DVBSX_GetDVBSxLockedCFO
Parameters :i32CFO
Return     :Operate Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal.

Description:Get DVB-Sx CFO value in KHz when demodulator signal is locked.
Attention: the signal should be locked to get accurate CFO
******************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxLockedCFO(DVBSX_DEMOD_MODULE *pDemod, int *i32LockedCFO) /*Get DVB-S_S2 carrier offset.*/
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	int i32CFOResValue, i32CFO;
	int i32tmp = 0, i32InverseFlag = 1;
	RtkDemod64Data i64Data, i64Data1;
	unsigned char cforeg[2];

	ui8Status = RTK_A_DVBSX_LatchOn(pDemod);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x2b, 0x0c, &cforeg[1]);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x2b, 0x0b, &cforeg[0]);
	ui8Status = RTK_A_DVBSX_LatchOff(pDemod);
	//coarse_bw[13:0], coarse_cfo[14:0], fine_bw[18:0]
	//Calculate CFO
	i32CFOResValue = cforeg[1]*256 + cforeg[0];

	if(cforeg[1]&0x80)
	{
		i32CFO = (65536 - (cforeg[1]*256 + cforeg[0]))*(-1);
	}else
	{
		i32CFO = i32CFOResValue;
	}
	// 120 ADC clock, 
	//i32CFO = (i32CFO*(-120)) *1000/65536;

	if(i32CFO > 0)
	{
		i32InverseFlag = -1;
	}else
	{
		i32InverseFlag = 1;
		i32CFO = 0 - i32CFO;
	}
	i32tmp = gstRtkDemodDvbConfig.ui32DVBSAdcClkKHz;

	RTK_A_DVBSX_64Mult(&i64Data, i32CFO, i32tmp);
	RTK_A_DVBSX_64Mult(&i64Data1, 1, 65536);
	i32CFO = RTK_A_DVBSX_64Div(i64Data, i64Data1);
	i32CFO = i32CFO * i32InverseFlag;
	if(gDVBSxUnicableSpectrumInverse)
	{
		i32CFO = 0 -i32CFO;
	}
	//i32CFO = (i32CFO/2*120)/16384;
	//i32CFO = (i32CFO*120*1000)/(16384*2);
	*i32LockedCFO = i32CFO;

	return ui8Status;
}

/****************************************************************************
Function   :RTK_A_DVBSX_GetDVBSxCoarseCFO
Parameters :pi32CarrierOffsetKHz
Return     :Operate Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal.

Description
Gt DVBS, DVBS2 Carrier Offset value, Notice the unit is KHz.
The CFO is coarse CFO, the coarse CFO can be detected earlier than LockedCFO.
If signal locked, RTK_A_DVBSX_GetDVBSxLockedCFO provides more accurate CFO.
******************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxCoarseCFO(DVBSX_DEMOD_MODULE *pDemod, int *pi32CarrierOffsetKHz)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	int i32CFOResValue, i32CFO = 0;
	unsigned char cforeg[2];

	ui8Status = RTK_A_DVBSX_LatchOn(pDemod);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x09, 0xc2, &cforeg[1]);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x09, 0xc1, &cforeg[0]);
	ui8Status = RTK_A_DVBSX_LatchOff(pDemod);

	//coarse_bw[13:0], coarse_cfo[14:0], fine_bw[18:0]
	//Calculate CFO
	i32CFOResValue = cforeg[1]*256 + cforeg[0];

	if(cforeg[1]&0x80)
	{
		i32CFO = (65536 - (cforeg[1]*256 + cforeg[0]))*(-1);
	}else
	{
		i32CFO = i32CFOResValue;
	}
	//i32CFO = (i32CFO/2*120)/16384;
	i32CFO = (i32CFO*1215)/(327);//i32CFO = (i32CFO*120*1000)/(16384*2);

	if(gDVBSxUnicableSpectrumInverse)
	{
		i32CFO = 0 -i32CFO;
	}
	*pi32CarrierOffsetKHz = i32CFO;

	return ui8Status;
}

/*SDK internal used function*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetSymbolRate(DVBSX_DEMOD_MODULE *pDemod, unsigned int SymbolRateK)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned int SymbolRate = 0;
	unsigned int SymbolRatePoint = 0;
	unsigned char u8Ratio = 0;
	unsigned int AdcClkKHz = gstRtkDemodDvbConfig.ui32DVBSAdcClkKHz;
	unsigned char RegVal = 0;
	unsigned int cfo_limit1 = 0, cfo_limit2 = 0;
	RtkDemod64Data i64Data, i64Data1;
	unsigned int cfo_limitR = 0, cfo_limitL = 0;

	if(SymbolRateK > 46000)
	{
		return ui8Status;
	}

	SymbolRate = SymbolRateK*1000;
	ui8Status = RTK_A_DVBSX_Write(pDemod, 0x15, 0x55, (unsigned char)(SymbolRate&0xff));
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x56, (unsigned char)((SymbolRate>>8)&0xff));
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x57, (unsigned char)((SymbolRate>>16)&0xff));
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x58, (unsigned char)((SymbolRate>>24)&0x03));
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x59, 1); //symbol_rate_force_en 1

	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x3f, 0x00); //dvbc_and_dvbs_bw_bypass
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x95, 0x00); //symbol_rate_point_force_en, symbol_rate_point_use_reg,
	
	//symbol_rate_point_force = 1.0*SymbolRateK *32768 *32/(120 *1000)
	//SymbolRatePoint = (unsigned int)((1.0*SymbolRateK *32768)/(30.0 *125));

	RTK_A_DVBSX_64Mult(&i64Data, SymbolRateK, (32768*32));
	RTK_A_DVBSX_64Mult(&i64Data1, 1, AdcClkKHz);
	SymbolRatePoint = RTK_A_DVBSX_64Div(i64Data, i64Data1);
	//SymbolRatePoint = (unsigned int)(1.0*SymbolRateK *32768 *32/(AdcClkKHz *1000));

	//15 92 a0 // symbol_rate_point_force u(19, 5) = 20*32768/120*2^5
	//15 93 aa
	//15 94 02
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x92, (unsigned char)(SymbolRatePoint&0xff));
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x93, (unsigned char)((SymbolRatePoint>>8)&0xff));
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x94, (unsigned char)((SymbolRatePoint>>16)&0xff));

	//Set scan left limit CFO and right limit CFO.
	cfo_limitR = (unsigned int)((12*32768*1000)/AdcClkKHz); //cfo_limitR = (12*32768/FS_ADC)
	cfo_limitL = (unsigned int)(65536000 - ((12*32768*1000)/AdcClkKHz)); //cfo_limitL = (-12*32768/FS_ADC)
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x20, (unsigned char)(cfo_limitR&0xFF)); //dvbs_cfo_max_limit, default is 2730*2
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x21, (unsigned char)((cfo_limitR>>8)&0xFF));
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x22, (unsigned char)(cfo_limitL&0xFF)); //dvbs_cfo_min_limit, default is -2730*2
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x23, (unsigned char)((cfo_limitL>>8)&0xFF));
	//Set scan left range and right range.
	
	//2.set coarse symbol rate limit
	if(SymbolRateK <= 2000) //50%
	{
		u8Ratio = 128/2;
	}
	else if(SymbolRateK <= 5000) // %50
	{
		u8Ratio = 128/2;
	}
	else if(SymbolRateK <= 15000) // %33
	{
		u8Ratio = 42;
	}
	else if(SymbolRateK <= 30000) // %33
	{
		//u8Ratio = 128/5;
		u8Ratio = 42; //33%
		//u8Ratio = 128/4;
	}
	else if(SymbolRateK <= 45000) // %16
	{
		//u8Ratio = 128/6;
		u8Ratio = 30; //22%
		//u8Ratio = 128/4;
	}
	else
	{
		//u8Ratio = 26; //%14
		u8Ratio = 128/4;
	}
	u8Ratio |= 0x80;
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x34, u8Ratio);

	//AFT
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x82, 0x08);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x00, 0x14, &RegVal);
	RegVal |= 0x10;
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x14, RegVal);
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x83, 0x3F);

	#ifdef CONFIG_CUSTOMER_TV002
	if(SymbolRateK < 9000)
	{
		//cfo_limit1 = (unsigned int)((0x50000*1000)/AdcClkKHz); //cfo_limit1 = (5/FS_ADC)*2^16
		//cfo_limit2 = (unsigned int)((0xA0000*1000)/AdcClkKHz); //cfo_limit2 = (10/FS_ADC)*2^16
		cfo_limit1 = (unsigned int)((0x60D20*1000)/AdcClkKHz); //cfo_limit1 = (5/FS_ADC)*2^16 //CFO limit, 5M->6.0M, 10M->10.7M
		cfo_limit2 = (unsigned int)((0xAB335*1000)/AdcClkKHz); //cfo_limit2 = (10/FS_ADC)*2^16
		ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x7C, (unsigned char)(cfo_limit1&0xFF));
		ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x7D, (unsigned char)((cfo_limit1>>8)&0xFF));
		ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x7E, (unsigned char)(cfo_limit2&0xFF));
		ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x7F, (unsigned char)((cfo_limit2>>8)&0xFF));
	}
	else
	{
		//cfo_limit1 = (unsigned int)((0x50000*1000)/AdcClkKHz); //cfo_limit1 = (5/FS_ADC)*2^16
		//cfo_limit2 = (unsigned int)((0xA0000*1000)/AdcClkKHz); //cfo_limit2 = (10/FS_ADC)*2^16
		cfo_limit1 = (unsigned int)((0x4B335*1000)/AdcClkKHz); //cfo_limit1 = (5/FS_ADC)*2^16 //CFO limit, 5M->4.7M, 10M->10.7M
		cfo_limit2 = (unsigned int)((0xAB335*1000)/AdcClkKHz); //cfo_limit2 = (10/FS_ADC)*2^16
		ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x7C, (unsigned char)(cfo_limit1&0xFF));
		ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x7D, (unsigned char)((cfo_limit1>>8)&0xFF));
		ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x7E, (unsigned char)(cfo_limit2&0xFF));
		ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x7F, (unsigned char)((cfo_limit2>>8)&0xFF));
	}
	#else
	//cfo_limit1 = (unsigned int)((0x50000*1000)/AdcClkKHz); //cfo_limit1 = (5/FS_ADC)*2^16
	//cfo_limit2 = (unsigned int)((0xA0000*1000)/AdcClkKHz); //cfo_limit2 = (10/FS_ADC)*2^16
	cfo_limit1 = (unsigned int)((0x4B335*1000)/AdcClkKHz); //cfo_limit1 = (5/FS_ADC)*2^16 //CFO limit, 5M->4.7M, 10M->10.7M
	cfo_limit2 = (unsigned int)((0xAB335*1000)/AdcClkKHz); //cfo_limit2 = (10/FS_ADC)*2^16
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x7C, (unsigned char)(cfo_limit1&0xFF));
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x7D, (unsigned char)((cfo_limit1>>8)&0xFF));
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x7E, (unsigned char)(cfo_limit2&0xFF));
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x7F, (unsigned char)((cfo_limit2>>8)&0xFF));
	#endif

	if((SymbolRateK < RTK_DVBSX_LITE_SYMBOL_HZ/1000) && (u8DVBSx255kSupport))
	{
		REALTEK_A_DVBSX_INFO("Very Low Symbol Rate <= 0.98M\n");
		RTK_A_DVBSX_SetDvbSxVeryLowSrAdc(pDemod, 1);
		ui8Status |= RTK_A_DVBSX_ForceSymbolRateandCFO(pDemod, SymbolRateK, 0);
		ui8Status |= RTK_A_DVBSX_WriteRegArray(pDemod, DvbSxSmallSymbolRegs, sizeof(DvbSxSmallSymbolRegs)/sizeof(unsigned char));
	}
	else
	{
		REALTEK_A_DVBSX_INFO("Normal Symbol Rate > 0.98M\n");
		RTK_A_DVBSX_SetDvbSxVeryLowSrAdc(pDemod, 0);
		//not used will set auto matically.
		//ui8Status |= RtkDemodWriteRegArray(DvbSxLargeSymbolRegs, sizeof(DvbSxLargeSymbolRegs)/sizeof(unsigned char));
	}

	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x09, 0xA6, 0); //set non blindscan mode
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 1); // Reset DSP
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0); // Reset DSP
	
	return ui8Status;
}

//Internal used function.
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvBSxSymbolScan(DVBSX_DEMOD_MODULE *pDemod, unsigned char *u32LowPwrNoSig)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned char RegVal = 0;
	unsigned int cfo_limitR = 0, cfo_limitL = 0, max_spect = 0, min_spect = 0, FFT_Timeout = 100;
	unsigned long stime = 0;
	int i32AdcClkMHz = 0;
	unsigned char u8ModeReg = 0, u8ConfigdoneReg = 0;
	unsigned char u8TargetLevelReg03=0, u8TargetLevelReg04=0, u8TargetLevelReg05=0;
	#ifdef PRINT_SPECTRUM
	unsigned int x = 0, y = 0, avg_tmp = 0;
	#endif
	
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x00, 0x04, &u8ConfigdoneReg);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x00, 0x2b, &u8ModeReg);

	if((u8ModeReg == 1) && (u8ConfigdoneReg == 1)) //Current working on DVB-S/S2 mode.
	{
		ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x04, 0x00);  //fixme just re-config analog registers. may no required.
		ui8Status |= RTK_A_DVBSX_SetSysClkClockDVBSx(pDemod);
		ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x04, 0x01); //config done 2017.02.03
	}
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x09, 0xA6, 1); //set blind scan mode
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x59, 0x00);
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x95, 0x00);

	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x09, 0xff, 0x01);//FFT mode enable
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x09, 0x00);
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x0a, 0x00);
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x14, 0x00);
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x15, 0x00);

	i32AdcClkMHz = gstRtkDemodDvbConfig.ui32DVBSAdcClkKHz/1000;
	//12 -> 25
	cfo_limitR = (unsigned int)(25*32768/i32AdcClkMHz); //cfo_limitR = (12*32768/FS_ADC)
	cfo_limitL = (unsigned int)(65536 - 25*32768/i32AdcClkMHz); //cfo_limitL = (-12*32768/FS_ADC)
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x20, (unsigned char)(cfo_limitR&0xFF)); //dvbs_cfo_max_limit, default is 2730*2
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x21, (unsigned char)((cfo_limitR>>8)&0xFF));
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x22, (unsigned char)(cfo_limitL&0xFF)); //dvbs_cfo_min_limit, default is -2730*2
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x23, (unsigned char)((cfo_limitL>>8)&0xFF));

	//ui8Status = RTK_A_DVBSX_Write(pDemod, 0x15, 0x33, 0x16);
	
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x34, &RegVal);
	RegVal &= 0x7F;
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x34, RegVal);

	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 1); // Reset DSP
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0);
	
	//check tuner target level
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x10, 0x05, &u8TargetLevelReg05);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x10, 0x04, &u8TargetLevelReg04);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x10, 0x03, &u8TargetLevelReg03);
	REALTEK_A_DVBSX_INFO("\033[1;32;31m" "Check Tuner Target Level, 0x%2x 0x%2x 0x%2x\n" "\033[m", u8TargetLevelReg05, u8TargetLevelReg04, u8TargetLevelReg03);


	stime = tv_osal_time();
	while((tv_osal_time() - stime) < FFT_Timeout)
	{
		//No Signal Check
		ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x09, 0xcd, &RegVal);//Read FFT done
		if(RegVal == 1)
		{
			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x09, 0xd1, &RegVal);//Read Max_Spect
			max_spect = RegVal;
			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x09, 0xd2, &RegVal);//Read Max_Spect
			max_spect += (RegVal << 8);
			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x09, 0x85, &RegVal);//Read Max_Spect
			max_spect += (RegVal << 16);

			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x09, 0xce, &RegVal);//Read Min_Spect
			min_spect = RegVal;
			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x09, 0xcf, &RegVal);
			min_spect += (RegVal << 8);
			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x09, 0xd0, &RegVal);
			min_spect += (RegVal << 16);

			if((max_spect - min_spect) < NO_SIGNAL_IN_SPECTRUM_TH)
				*u32LowPwrNoSig = 1;
			else
				*u32LowPwrNoSig = 0;

			REALTEK_A_DVBSX_INFO("\033[1;32;31m" "max_spect = %u min_spect = %u M-m = %u LowPowerFlag = %u NoSigTH = %u\n" "\033[m", max_spect, min_spect, (max_spect-min_spect), *u32LowPwrNoSig, NO_SIGNAL_IN_SPECTRUM_TH);
			break;
		}
	}

	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x09, 0xff, 0x00); //FFT mode disable
#ifdef PRINT_SPECTRUM
	rtd_outl(DTV_DEMOD_MISC_ATB_FFT_READ_FUNC_1_reg, DTV_DEMOD_MISC_ATB_FFT_READ_FUNC_1_atb_fft_read_enable(0x1));//atb_fft_read_start_addr = 0x0, atb_fft_read_enable = 0x1
	for(x=0; x<1024; x++)
	{
		avg_tmp = 0;
		for(y=0; y<16; y++)
		{
			avg_tmp += DTV_DEMOD_MISC_ATB_FFT_READ_FUNC_2_get_atb_fft_read_data(rtd_inl(DTV_DEMOD_MISC_ATB_FFT_READ_FUNC_2_reg));//read atb_fft_read_data
			//REALTEK_A_DVBSX_INFO("[ %u \n", avg_tmp);
		}

		gDvbSxBsSpecialParams.u32SpectrumArray[x] = avg_tmp/16;
		//REALTEK_A_DVBSX_INFO("[ %u \n", SpectrumArray[x]);
	}
	REALTEK_A_DVBSX_INFO("====================================\n");
	for(x=0; x<1024; x++)
	{
		avg_tmp = 0;
		if(x>=4 && x<1020)
		{
			gDvbSxBsSpecialParams.u16SmoothArray[x] = (unsigned short)((gDvbSxBsSpecialParams.u32SpectrumArray[x-4]+gDvbSxBsSpecialParams.u32SpectrumArray[x-3]+gDvbSxBsSpecialParams.u32SpectrumArray[x-2]+gDvbSxBsSpecialParams.u32SpectrumArray[x-1]
			+ gDvbSxBsSpecialParams.u32SpectrumArray[x]+gDvbSxBsSpecialParams.u32SpectrumArray[x+1]+gDvbSxBsSpecialParams.u32SpectrumArray[x+2]+gDvbSxBsSpecialParams.u32SpectrumArray[x+3]+gDvbSxBsSpecialParams.u32SpectrumArray[x+4])/9);
		}
		else
		{
			gDvbSxBsSpecialParams.u16SmoothArray[x] = (unsigned short)gDvbSxBsSpecialParams.u32SpectrumArray[x];
		}
		REALTEK_A_DVBSX_INFO("[ %u \n", gDvbSxBsSpecialParams.u16SmoothArray[x]);
		if(x%100==0)
			tv_osal_msleep(500);
	}
	REALTEK_A_DVBSX_INFO("====================================\n");
	rtd_outl(DTV_DEMOD_MISC_ATB_FFT_READ_FUNC_1_reg, DTV_DEMOD_MISC_ATB_FFT_READ_FUNC_1_atb_fft_read_enable(0x0));//atb_fft_read_start_addr = 0x0, atb_fft_read_enable = 0x0
#endif

	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x09, 0x00);
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x0a, 0x00);
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x14, 0x00);
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x15, 0x00);
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 1); // Reset DSP
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0);

	return ui8Status;
}

//SDK internal used function, not for API.
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvBSxNoneSymbolScan(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;

	ui8Status = RTK_A_DVBSX_Write(pDemod, 0x09, 0xA6, 0x00); //set non blind scan mode 
	ui8Status = RTK_A_DVBSX_Write(pDemod, 0x15, 0x59, 0x01);
	ui8Status = RTK_A_DVBSX_Write(pDemod, 0x15, 0x95, 0x00);

	ui8Status = RTK_A_DVBSX_Write(pDemod, 0x00, 0x09, 0x10);
	ui8Status = RTK_A_DVBSX_Write(pDemod, 0x00, 0x0a, 0x00);
	ui8Status = RTK_A_DVBSX_Write(pDemod, 0x00, 0x14, 0x1c);
	ui8Status = RTK_A_DVBSX_Write(pDemod, 0x00, 0x15, 0x1f);

	return ui8Status;
}

/********************************************************************************
function    :RTK_A_DVBSX_DVBSxModeDetected
parameter   :pu8DVBSxMode returns the demodulator's received signal mode: RTK_DEMOD_DVBS_MODE or RTK_DEMOD_DVBS2_MODE
return      :Operate Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal.
Description
Call this function to detect the working status of demodulator,
DVB-S or DVB-S2 mode can be checked after FEC locked.
*********************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_DVBSxModeDetected(DVBSX_DEMOD_MODULE *pDemod, unsigned char *pu8DVBSxMode)
{
	unsigned char u8SyncS = 0;
	unsigned char u8ChipVersion = 0;
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;

	*pu8DVBSxMode = RTK_DEMOD_DVB_UNKNOWN;

	status = RTK_A_DVBSX_Read(pDemod, 0x29, 0xff, &u8SyncS);
	//status |= RTK_A_DVBSX_Read(pDemod, 0x2c, 0x18, &u8SyncS2);

	status |= RTK_A_DVBSX_Read(pDemod, 0x00, 0x00, &u8ChipVersion);

	if(u8ChipVersion == 0xa8)
	{
		//*pu8DVBSxMode = RTK_DEMOD_DVBS_MODE; //default Force DVBS mode, as no S2 mode. 9.10

		if((u8SyncS&0x01) && ((u8SyncS&0x02) != 0x02))
		{
			*pu8DVBSxMode = RTK_DEMOD_DVBS_MODE;
		}

		if(((u8SyncS&0x02) == 0x02) && ((u8SyncS&0x01) != 1))
		{
			*pu8DVBSxMode = RTK_DEMOD_DVBS2_MODE;
		}
	}

	return status;
}

/********************************************************************************
Function    :RTK_A_DVBSX_GetDVBSxSNR
Parameter   :pi32SNRx10
Return      :Operate Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal.
Description
Get DVB-Sx SNR, Attention the returned SNR value is scaled by x10,
for example. when SNR = 11.6dB, the returned value is 116.
*********************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxSNR(DVBSX_DEMOD_MODULE *pDemod, int *pi32SNRx10) /*Get DVBS-S_S2 SNR*10. scalar is 10*/
{
	int i32SNR = 0;
	unsigned int u32Data = 0;
	unsigned int i32BiasSnr = 0;
	unsigned char u8Tmp = 0;
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned char u8DVBSxMode = 0;

	*pi32SNRx10 = 0;

	status = RTK_A_DVBSX_DVBSxModeDetected(pDemod, &u8DVBSxMode);

	status |= RTK_A_DVBSX_LatchOn(pDemod);
	status |= RTK_A_DVBSX_Read(pDemod, 0x2c, 0x14, &u8Tmp);
	u32Data = u32Data + (u8Tmp&0xff)*256;

	status |= RTK_A_DVBSX_Read(pDemod, 0x2c, 0x13, &u8Tmp);
	u32Data = u32Data + u8Tmp;
	status |= RTK_A_DVBSX_LatchOff(pDemod);

	if(u8DVBSxMode == RTK_DEMOD_DVBS2_MODE)
	{
		u32Data = u32Data*2;
	}

	if(u32Data == 0)
	{
		*pi32SNRx10 = 0;
	}
	else
	{
		//dbData = dbData/pow(2.0,10);
		u32Data = (u32Data* 10)/1024;

		i32SNR = 2*RTK_A_DVBSX_Log10ConvertdBx10(u32Data); //u32Data added 20dB by multiply 10, is equal to i32BiasSnr

		i32BiasSnr = 2 * RTK_A_DVBSX_Log10ConvertdBx10(10);

		i32SNR = i32SNR - i32BiasSnr;
	
		*pi32SNRx10 = i32SNR;
	}
	
	if(*pi32SNRx10 > 500 || *pi32SNRx10 < 0)
	{
		//REALTEK_A_DVBSX_INFO("\033[1;32;33m" "SNR over range %d\n" "\033[m", *pi32SNRx10);
		*pi32SNRx10 = 0;
	}

	return status;
}

/*Internal used function*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetScanedSymbols(DVBSX_DEMOD_MODULE *pDemod)
{
	unsigned char ui8Value = 0;
	unsigned char bw_det_done = 0, dvbs_all_bw_num = 0;
	unsigned char SymbolRegs[6] = {0};
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned int u32SymbolBW = 0;
	unsigned int u32CoarseBW = 0;
	int i32CFOResValue = 0;
	int i32CFO = 0;
	unsigned char u8Iter = 0;
	int i = 0;
	unsigned long stime = 0;
	
	stime = tv_osal_time();
	while((tv_osal_time() - stime) < 1500)
	{
		ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x91, &ui8Value);
		bw_det_done = ui8Value&0x80;
		dvbs_all_bw_num = ui8Value&0x7f;
		if(bw_det_done)
		{
			#ifdef BS_SCAN_DEBUG
			REALTEK_A_DVBSX_INFO("RTK_A_DVBSX_GetScanedSymbols bw_det_done bwreg:0x%x all_bw_num=%d time:%lu ms\n", ui8Value, dvbs_all_bw_num, tv_osal_time() - stime);
			#endif
			break;
		}
		tv_osal_msleep(10);
	}

	if(ui8Value == 0xff)
	{
		dvbs_all_bw_num = 0;
	}
	//clear params array.
	gScanedParams.u8ScanedSymbolNum = 0;
	for(u8Iter = 0; u8Iter < MAX_SCANED_SYMBOL_NUM; u8Iter++)
	{
		gScanedParams.ScanedSymbols[u8Iter].u32SymbolRateKHz = 0;
		gScanedParams.ScanedSymbols[u8Iter].u32CoarseSymbolRateKHz = 0;
		gScanedParams.ScanedSymbols[u8Iter].i32ScanedCFO = 0;
		gScanedParams.ScanedSymbols[u8Iter].u32absCFO = 0;
	}

	//REALTEK_A_DVBSX_INFO("RTK_A_DVBSX_GetScanedSymbols bwreg:0x%x all_bw_num=%d time:%d*10ms\n", ui8Value, dvbs_all_bw_num, i);
	//Calculate and save scan params
	if((bw_det_done) && (dvbs_all_bw_num))
	{	
		if(dvbs_all_bw_num > MAX_SCANED_SYMBOL_NUM)
		{
			dvbs_all_bw_num = MAX_SCANED_SYMBOL_NUM;
		}
		for(u8Iter = 0; u8Iter < dvbs_all_bw_num; u8Iter++)
		{
			//RTK_A_DVBSX_LatchOn(pDemod);
			ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x8a, u8Iter);
			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x8B, &SymbolRegs[0]);
			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x8c, &SymbolRegs[1]);
			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x8d, &SymbolRegs[2]);
			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x8e, &SymbolRegs[3]);
			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x8f, &SymbolRegs[4]);
			ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x90, &SymbolRegs[5]);
			//RTK_A_DVBSX_LatchOff(pDemod);

			//coarse_bw[13:0], coarse_cfo[14:0], fine_bw[18:0]
			//Calculate CFO
			i32CFOResValue = (SymbolRegs[4]&0x03)*256*32 + SymbolRegs[3]*32 + (SymbolRegs[2]&0xf8)/8;
			if(SymbolRegs[4]&0x02)
			{
				i32CFO = (32768 - (SymbolRegs[4]&0x03)*256*32 - SymbolRegs[3]*32 - (SymbolRegs[2]&0xf8)/8)*(-1);
			}else
			{
				i32CFO = i32CFOResValue;
			}
			if(gDVBSxUnicableSpectrumInverse)
			{
				i32CFO = 0 - i32CFO;
			}
			//i32CFO = (i32CFO*120*1000)/16384;
			i32CFO = (i32CFO*1215)/164;

			//coarse symbol rate, X*120/16384
			u32CoarseBW = (SymbolRegs[5]*64) + ((SymbolRegs[4]&0xfc)/4);
			//u32CoarseBW = u32CoarseBW*120*1000/16384;
			u32CoarseBW = (u32CoarseBW*1215 + 82)/164;

			//Calculate Symbol rate.bit[18:0], 19bits
			u32SymbolBW = (SymbolRegs[2]&0x07)*256*256 + SymbolRegs[1]*256 + SymbolRegs[0];
			#ifdef BS_SCAN_DEBUG
			REALTEK_A_DVBSX_INFO("RTK_A_DVBSX_GetScanedSymbols u32SymbolBW(reg value): %d\n", u32SymbolBW);
			#endif
			//bw0 = bw0*120*1000/(16384*2)/pow(2, 5); //changed as below
			//u32SymbolBW = (u32SymbolBW*15*125 + (256*32))/(512*32); //Symbol rate using KHz, Max error: 0.5KHz
			u32SymbolBW = (u32SymbolBW*1215 + (5243))/(10487); //Symbol rate using KHz, Max error: 0.5KHz

			//Save Params in array.
			gScanedParams.ScanedSymbols[u8Iter].i32ScanedCFO = i32CFO;
			gScanedParams.ScanedSymbols[u8Iter].u32CoarseSymbolRateKHz = u32CoarseBW;
			gScanedParams.ScanedSymbols[u8Iter].u32SymbolRateKHz = u32SymbolBW;
			gScanedParams.u8ScanedSymbolNum = gScanedParams.u8ScanedSymbolNum + 1;
		}
		//REALTEK_A_DVBSX_INFO("RTK_A_DVBSX_GetScanedSymbols total scanedSymbol Num:%d\n", gScanedParams.u8ScanedSymbolNum);
		for(i = 0; i < dvbs_all_bw_num; i++)
		{
			#ifdef BS_SCAN_DEBUG
			REALTEK_A_DVBSX_INFO("[%d]CFO:%d symbol rate: %d\n", i, gScanedParams.ScanedSymbols[i].i32ScanedCFO, gScanedParams.ScanedSymbols[i].u32SymbolRateKHz);
			#endif
		}
		tv_osal_msleep(10);
	}

	return ui8Status;
}

/* Sort scanned signals base on CFO, internal used funciton*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SortSymbolRateAndCFO(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	int i = 0, j = 0;
	unsigned int u32absCFOMin = 0, u32absCFOTmp = 0;
	int CFOTmp = 0;
	unsigned int u32SymbolTmp = 0;

	if(gScanedParams.u8ScanedSymbolNum <= 1)
	{
		return status;
	}
	for(i = 0; i < gScanedParams.u8ScanedSymbolNum; i++)
	{
		u32absCFOMin = gScanedParams.ScanedSymbols[i].u32absCFO;
		for(j = i; j < gScanedParams.u8ScanedSymbolNum; j++)
		{
			if(gScanedParams.ScanedSymbols[j].u32absCFO < u32absCFOMin)
			{
				//Swap
				u32absCFOMin = gScanedParams.ScanedSymbols[j].u32absCFO;

				u32absCFOTmp = gScanedParams.ScanedSymbols[i].u32absCFO;
				CFOTmp = gScanedParams.ScanedSymbols[i].i32ScanedCFO;
				u32SymbolTmp = gScanedParams.ScanedSymbols[i].u32SymbolRateKHz;
				gScanedParams.ScanedSymbols[i].i32ScanedCFO = gScanedParams.ScanedSymbols[j].i32ScanedCFO;
				gScanedParams.ScanedSymbols[i].u32SymbolRateKHz = gScanedParams.ScanedSymbols[j].u32SymbolRateKHz;
				gScanedParams.ScanedSymbols[i].u32absCFO = gScanedParams.ScanedSymbols[j].u32absCFO;

				gScanedParams.ScanedSymbols[j].i32ScanedCFO = CFOTmp;
				gScanedParams.ScanedSymbols[j].u32SymbolRateKHz = u32SymbolTmp;
				gScanedParams.ScanedSymbols[j].u32absCFO = u32absCFOTmp;
			}
		}
	}

	return status;
}
#if 0
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetLdpc1p6XMaxIteration(DVBSX_DEMOD_MODULE *pDemod, unsigned int symbolrate, unsigned int acq_mode)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	RTK_DVBSx_SIGNAL_PARAMS stDVBSxParams;
	unsigned char u8i = 0, u8DVBSxMode = 0, u8ISINumber = 0, u8NewRxheaderMod = 0, u8NewRxheaderCod = 0, lock = 0;
	unsigned int MaxIterationNumber = 0, NewMaxIterationNumber = 0;
	int i32SNRx10 = 0;

	/*Set default unknown*/
	stDVBSxParams.DVBSxModu = RTK_DVBSx_MODU_UNKNOWN;
	stDVBSxParams.DVBS2CodeRate = RTK_DVBS2_CODE_RATE_UNKNOWN;
	stDVBSxParams.DVBS2XCodeRate = RTK_DVBS2X_CODE_RATE_UNKNOWN;
	
	status |= RTK_A_DVBSX_DVBSxModeDetected(pDemod, &u8DVBSxMode);
	
	if(status == RTK_DEMOD_SUCCESSFUL && u8DVBSxMode == RTK_DEMOD_DVBS2_MODE && symbolrate > 19900) /*DVB-S2/S2X mode && SR > 19.9M*/
	{
		status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x86, &u8ISINumber);
		if(u8ISINumber != 1)
		{
			REALTEK_A_DVBSX_INFO("VCM MODE BREAK MAX ITERATION SETTING u8ISINumber=%u\n", u8ISINumber);
			if(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_get_s2x_ldpc_config_iter_5_0(rtd_inl(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_reg)) != 0x0 || ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_get_s2x_ldpc_config_flag(rtd_inl(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_reg)) != 0x0 || SYS_REG_SYS_CLKDIV_get_dtv_demod_atb_clk_sel(rtd_inl(SYS_REG_SYS_CLKDIV_reg)) != SYS_REG_SYS_CLKDIV_dtv_demod_atb_clk_sel_mask)
			{
				REALTEK_A_DVBSX_INFO("Set Reg\n");
				rtd_maskl(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_reg, ~(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_s2x_ldpc_config_iter_5_0_mask), ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_s2x_ldpc_config_iter_5_0(0x0));
				rtd_maskl(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_reg, ~(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_s2x_ldpc_config_flag_mask), ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_s2x_ldpc_config_flag(0));
				rtd_maskl(SYS_REG_SYS_CLKDIV_reg, ~(SYS_REG_SYS_CLKDIV_dtv_demod_atb_clk_sel_mask), SYS_REG_SYS_CLKDIV_dtv_demod_atb_clk_sel(0x1));//Disble LDPC 1.6X Clock
			}
			return RTK_DEMOD_SUCCESSFUL;
		}
		
		status |= RTK_A_DVBSX_Read(pDemod, 0x16, 0xcb, &u8NewRxheaderMod);
		status |= RTK_A_DVBSX_Read(pDemod, 0x16, 0xcc, &u8NewRxheaderCod);
		
		if(u8NewRxheaderMod == 1)
			stDVBSxParams.DVBSxModu = RTK_DVBSx_MODU_QPSK;
		else if(u8NewRxheaderMod == 5)
			stDVBSxParams.DVBSxModu = RTK_DVBSx_MODU_8PSK;
		else if(u8NewRxheaderMod == 9)
			stDVBSxParams.DVBSxModu = RTK_DVBSx_MODU_8PSKL;
		else if(u8NewRxheaderMod == 6)
			stDVBSxParams.DVBSxModu = RTK_DVBSx_MODU_16APSK;
		else if(u8NewRxheaderMod == 7)
			stDVBSxParams.DVBSxModu = RTK_DVBSx_MODU_16APSKL;
		else if(u8NewRxheaderMod == 8 || u8NewRxheaderMod == 10)
			stDVBSxParams.DVBSxModu = RTK_DVBSx_MODU_32APSK;
		else if(u8NewRxheaderMod == 11)
			stDVBSxParams.DVBSxModu = RTK_DVBSx_MODU_32APSKL;
		else
			stDVBSxParams.DVBSxModu = RTK_DVBSx_MODU_UNKNOWN;
		
		if((stDVBSxParams.DVBSxModu == RTK_DVBSx_MODU_QPSK && u8NewRxheaderCod <= 10) || 
			(stDVBSxParams.DVBSxModu == RTK_DVBSx_MODU_8PSK && u8NewRxheaderCod >= 4 && u8NewRxheaderCod <= 10))
		{
			stDVBSxParams.DVBS2CodeRate = S2_Mod_CodeRate_table[u8NewRxheaderCod].DVBS2CodeRate;
		}
		else if((stDVBSxParams.DVBSxModu == RTK_DVBSx_MODU_QPSK && u8NewRxheaderCod <= 2) ||
			(stDVBSxParams.DVBSxModu == RTK_DVBSx_MODU_16APSK && u8NewRxheaderCod >= 5 && u8NewRxheaderCod <= 10) ||
			(stDVBSxParams.DVBSxModu == RTK_DVBSx_MODU_32APSK && u8NewRxheaderCod >= 6 && u8NewRxheaderCod <= 10))
		{
			stDVBSxParams.DVBS2CodeRate = S2_Mod_CodeRate_table[u8NewRxheaderCod].DVBS2CodeRate;
		}
		else if(stDVBSxParams.DVBSxModu == RTK_DVBSx_MODU_16APSK && u8NewRxheaderCod == 4)
		{
			stDVBSxParams.DVBS2XCodeRate = S2X_Mod_CodeRate_table[0].DVBS2XCodeRate;
		}
		else if(stDVBSxParams.DVBSxModu == RTK_DVBSx_MODU_32APSKL && u8NewRxheaderCod == 5)
		{
			stDVBSxParams.DVBS2XCodeRate = S2X_Mod_CodeRate_table[1].DVBS2XCodeRate;
		}
		else if((stDVBSxParams.DVBSxModu == RTK_DVBSx_MODU_QPSK || stDVBSxParams.DVBSxModu == RTK_DVBSx_MODU_8PSK || stDVBSxParams.DVBSxModu == RTK_DVBSx_MODU_8PSKL ||
			stDVBSxParams.DVBSxModu == RTK_DVBSx_MODU_16APSK || stDVBSxParams.DVBSxModu == RTK_DVBSx_MODU_16APSKL || stDVBSxParams.DVBSxModu == RTK_DVBSx_MODU_32APSK ||
			stDVBSxParams.DVBSxModu == RTK_DVBSx_MODU_32APSKL) && (u8NewRxheaderCod >= 24 && u8NewRxheaderCod <= 48))
		{
			stDVBSxParams.DVBS2XCodeRate = S2X_Mod_CodeRate_table[u8NewRxheaderCod-22].DVBS2XCodeRate;
		}
		
		REALTEK_A_DVBSX_INFO("[DVBS2 MODE] MOD=%u S2COD=%u S2XCOD=%u\n", stDVBSxParams.DVBSxModu, stDVBSxParams.DVBS2CodeRate, stDVBSxParams.DVBS2XCodeRate);
		if(stDVBSxParams.DVBS2XCodeRate == RTK_DVBS2X_CODE_RATE_9_20 || stDVBSxParams.DVBS2XCodeRate == RTK_DVBS2X_CODE_RATE_13_45)
		{
			REALTEK_A_DVBSX_INFO("FAIL CODERATE BREAK MAX ITERATION SETTING\n");
			if(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_get_s2x_ldpc_config_iter_5_0(rtd_inl(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_reg)) != 0x0 || ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_get_s2x_ldpc_config_flag(rtd_inl(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_reg)) != 0x0 || SYS_REG_SYS_CLKDIV_get_dtv_demod_atb_clk_sel(rtd_inl(SYS_REG_SYS_CLKDIV_reg)) != SYS_REG_SYS_CLKDIV_dtv_demod_atb_clk_sel_mask)
			{
				REALTEK_A_DVBSX_INFO("Set Reg\n");
				rtd_maskl(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_reg, ~(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_s2x_ldpc_config_iter_5_0_mask), ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_s2x_ldpc_config_iter_5_0(0x0));
				rtd_maskl(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_reg, ~(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_s2x_ldpc_config_flag_mask), ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_s2x_ldpc_config_flag(0));
				rtd_maskl(SYS_REG_SYS_CLKDIV_reg, ~(SYS_REG_SYS_CLKDIV_dtv_demod_atb_clk_sel_mask), SYS_REG_SYS_CLKDIV_dtv_demod_atb_clk_sel(0x1));//Disble LDPC 1.6X Clock
			}
			return RTK_DEMOD_SUCCESSFUL;
		}
		
		for(u8i = 0; u8i < DVBSX_LDPC_ITERATION_TABLE_SIZE; u8i++)
		{
			if(sDvbSxLdpcIterationTable[u8i].DVBSX_Modulation == stDVBSxParams.DVBSxModu &&
				sDvbSxLdpcIterationTable[u8i].DVBS2_CodeRate == stDVBSxParams.DVBS2CodeRate &&
				sDvbSxLdpcIterationTable[u8i].DVBS2X_CodeRate == stDVBSxParams.DVBS2XCodeRate)
			{
				status |= RTK_A_DVBSX_GetDVBSxSNR(pDemod, &i32SNRx10);
				if(acq_mode == 1 && i32SNRx10 > sDvbSxLdpcIterationTable[u8i].SnrThreshold)//SNR > SnrThreshold
				{
					REALTEK_A_DVBSX_INFO("HIGH SNR MODE BREAK MAX ITERATION SETTING, SNR %u, SNR_TH %u\n", i32SNRx10, sDvbSxLdpcIterationTable[u8i].SnrThreshold);
					if(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_get_s2x_ldpc_config_iter_5_0(rtd_inl(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_reg)) != 0x0 || ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_get_s2x_ldpc_config_flag(rtd_inl(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_reg)) != 0x0 || SYS_REG_SYS_CLKDIV_get_dtv_demod_atb_clk_sel(rtd_inl(SYS_REG_SYS_CLKDIV_reg)) != SYS_REG_SYS_CLKDIV_dtv_demod_atb_clk_sel_mask)
					{
						REALTEK_A_DVBSX_INFO("Set Reg\n");
						rtd_maskl(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_reg, ~(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_s2x_ldpc_config_iter_5_0_mask), ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_s2x_ldpc_config_iter_5_0(0x0));
						rtd_maskl(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_reg, ~(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_s2x_ldpc_config_flag_mask), ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_s2x_ldpc_config_flag(0));
						rtd_maskl(SYS_REG_SYS_CLKDIV_reg, ~(SYS_REG_SYS_CLKDIV_dtv_demod_atb_clk_sel_mask), SYS_REG_SYS_CLKDIV_dtv_demod_atb_clk_sel(0x1));//Disble LDPC 1.6X Clock
					}
					return RTK_DEMOD_SUCCESSFUL;
				}
				/*
				if(symbolrate < 5100)
				{
					MaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_5M_COEF/10000;
					NewMaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_5M_COEF*MAX_ITERATION_RATIO/100000;
				}
				else if(symbolrate < 10100)
				{
					MaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_10M_COEF/10000;
					NewMaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_10M_COEF*MAX_ITERATION_RATIO/100000;
				}
				else if(symbolrate < 15100)
				{
					MaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_15M_COEF/10000;
					NewMaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_15M_COEF*MAX_ITERATION_RATIO/100000;
				}
				else if(symbolrate < 20100)*/
				if(symbolrate < 20100)
				{
					MaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_20M_COEF/10000;
					NewMaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_20M_COEF*MAX_ITERATION_RATIO/100000;
				}
				else if(symbolrate < 25100)
				{
					MaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_25M_COEF/10000;
					NewMaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_25M_COEF*MAX_ITERATION_RATIO/100000;
				}
				else if(symbolrate < 30100)
				{
					MaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_30M_COEF/10000;
					NewMaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_30M_COEF*MAX_ITERATION_RATIO/100000;
				}
				else if(symbolrate < 35100)
				{
					MaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_35M_COEF/10000;
					NewMaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_35M_COEF*MAX_ITERATION_RATIO/100000;
				}
				else if(symbolrate < 40100)
				{
					MaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_40M_COEF/10000;
					NewMaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_40M_COEF*MAX_ITERATION_RATIO/100000;
				}
				else if(symbolrate < 45100)
				{
					MaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_45M_COEF/10000;
					NewMaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_45M_COEF*MAX_ITERATION_RATIO/100000;
				}
				else
				{
					MaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_45M_COEF/10000;
					NewMaxIterationNumber = sDvbSxLdpcIterationTable[u8i].DefaultBaseIteration*DVBSX_LDPC_ITERATION_45M_COEF*MAX_ITERATION_RATIO/100000;
				}
				
				if(NewMaxIterationNumber > 63)
					NewMaxIterationNumber = 63;
				
				if(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_get_s2x_ldpc_config_iter_5_0(rtd_inl(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_reg)) != NewMaxIterationNumber || ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_get_s2x_ldpc_config_flag(rtd_inl(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_reg)) != 0x1 || SYS_REG_SYS_CLKDIV_get_dtv_demod_atb_clk_sel(rtd_inl(SYS_REG_SYS_CLKDIV_reg)) != 0x0)
				{
					REALTEK_A_DVBSX_INFO("Set Reg, Pre MaxIter = %u, Change MaxIterationNumber(Max 63): %u => %u\n", ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_get_s2x_ldpc_config_iter_5_0(rtd_inl(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_reg)), MaxIterationNumber, NewMaxIterationNumber);
					rtd_maskl(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_reg, ~(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_s2x_ldpc_config_iter_5_0_mask), ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_ITER_5_0_s2x_ldpc_config_iter_5_0(NewMaxIterationNumber));
					rtd_maskl(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_reg, ~(ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_s2x_ldpc_config_flag_mask), ATB_DEMOD_FREQ_S2X_LDPC_CONFIG_FLAG_s2x_ldpc_config_flag(1));
					rtd_maskl(SYS_REG_SYS_CLKDIV_reg, ~(SYS_REG_SYS_CLKDIV_dtv_demod_atb_clk_sel_mask), SYS_REG_SYS_CLKDIV_dtv_demod_atb_clk_sel(0x0));//Enable LDPC 1.6X Clock
					//RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0x01); /* Reset DSP */
					//RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0x00); /* Start DSP */
				
					RTK_A_DVBSX_ChannelLockCheck(pDemod, &lock);
					REALTEK_A_DVBSX_INFO("Ldpc1p6X Demod Lock = %u\n", lock);
				}
				
				break;
			}
		}
	}

	return status;
}
#endif
/********************************************************************************
Function    :RTK_A_DVBSX_GetDVBSxSignalParams
Parameter   :pstDVBSxParams
Return      :Operate Status:RTK_DEMOD_SUCCESSFUL is Normal and other is abnormal.
Description
Get DVB-S/S2/S2X signal parameters.
*********************************************************************************/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxSignalParams(DVBSX_DEMOD_MODULE *pDemod, RTK_DVBSx_SIGNAL_PARAMS *pstDVBSxParams)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned int u32Reg = 0;
	unsigned int ui32SymbolRateHz = 0;
	unsigned char u8DVBSMode = 0, u8DVBS2Mode = 0, u8TSLockFlag = 0, u8Rxheader = 0, u8NewRxheaderMod = 0, u8NewRxheaderCod = 0;
	unsigned char u8DVBSxMode;

	status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x2a, &u8TSLockFlag);
	if(!u8TSLockFlag)
	{
		tv_osal_msleep(100);
		status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x2a, &u8TSLockFlag);
	}

	status |= RTK_A_DVBSX_DVBSxModeDetected(pDemod, &u8DVBSxMode);

	if(u8DVBSxMode == RTK_DEMOD_DVBS2_MODE) /*DVBS2 mode*/
	{
		u8DVBS2Mode = 1;
	}
	if(u8DVBSxMode == RTK_DEMOD_DVBS_MODE) /*DVBS mode*/
	{
		u8DVBSMode = 1;
	}

	/*Set default unknown*/
	pstDVBSxParams->DVBSxType = RTK_DEMOD_DVB_UNKNOWN;
	pstDVBSxParams->DVBSxModu = RTK_DVBSx_MODU_UNKNOWN;
	pstDVBSxParams->DVBSxSymbolRateHz = 0;
	pstDVBSxParams->DVBS2CodeRate = RTK_DVBS2_CODE_RATE_UNKNOWN;
	pstDVBSxParams->DVBS2XCodeRate = RTK_DVBS2X_CODE_RATE_UNKNOWN;
	pstDVBSxParams->DVBSCodeRate = RTK_DVBS_CODE_RATE_UNKNOWN;
	pstDVBSxParams->u8DVBSxISINumber = 0;

	if((u8DVBS2Mode == 1) && (status == RTK_DEMOD_SUCCESSFUL)) /*DVB-S2/S2X mode*/
	{
		status |= RTK_A_DVBSX_Read(pDemod, 0x16, 0xcb, &u8NewRxheaderMod);
		status |= RTK_A_DVBSX_Read(pDemod, 0x16, 0xcc, &u8NewRxheaderCod);
		
		if(u8NewRxheaderMod == 1)
			pstDVBSxParams->DVBSxModu = RTK_DVBSx_MODU_QPSK;
		else if(u8NewRxheaderMod == 5)
			pstDVBSxParams->DVBSxModu = RTK_DVBSx_MODU_8PSK;
		else if(u8NewRxheaderMod == 9)
			pstDVBSxParams->DVBSxModu = RTK_DVBSx_MODU_8PSKL;
		else if(u8NewRxheaderMod == 6)
			pstDVBSxParams->DVBSxModu = RTK_DVBSx_MODU_16APSK;
		else if(u8NewRxheaderMod == 7)
			pstDVBSxParams->DVBSxModu = RTK_DVBSx_MODU_16APSKL;
		else if(u8NewRxheaderMod == 8 || u8NewRxheaderMod == 10)
			pstDVBSxParams->DVBSxModu = RTK_DVBSx_MODU_32APSK;
		else if(u8NewRxheaderMod == 11)
			pstDVBSxParams->DVBSxModu = RTK_DVBSx_MODU_32APSKL;
		else
			pstDVBSxParams->DVBSxModu = RTK_DVBSx_MODU_UNKNOWN;
		
		if((pstDVBSxParams->DVBSxModu == RTK_DVBSx_MODU_QPSK && u8NewRxheaderCod >= 3 && u8NewRxheaderCod <= 10) || 
			(pstDVBSxParams->DVBSxModu == RTK_DVBSx_MODU_8PSK && u8NewRxheaderCod >= 4 && u8NewRxheaderCod <= 10))
		{
			//REALTEK_A_DVBSX_INFO("\033[1;32;33m" "RTK_DEMOD_DVBS2_MODE\n" "\033[m");
			pstDVBSxParams->DVBSxType = RTK_DEMOD_DVBS2_MODE;
			pstDVBSxParams->DVBS2CodeRate = S2_Mod_CodeRate_table[u8NewRxheaderCod].DVBS2CodeRate;
		}
		else if((pstDVBSxParams->DVBSxModu == RTK_DVBSx_MODU_QPSK && u8NewRxheaderCod <= 2) ||
			(pstDVBSxParams->DVBSxModu == RTK_DVBSx_MODU_16APSK && u8NewRxheaderCod >= 5 && u8NewRxheaderCod <= 10) ||
			(pstDVBSxParams->DVBSxModu == RTK_DVBSx_MODU_32APSK && u8NewRxheaderCod >= 6 && u8NewRxheaderCod <= 10))
		{
			//REALTEK_A_DVBSX_INFO("\033[1;32;33m" "RTK_DEMOD_DVBS2_OPTIONAL_MODE\n" "\033[m");
			pstDVBSxParams->DVBSxType = RTK_DEMOD_DVBS2_OPTIONAL_MODE;
			pstDVBSxParams->DVBS2CodeRate = S2_Mod_CodeRate_table[u8NewRxheaderCod].DVBS2CodeRate;
		}
		else if(pstDVBSxParams->DVBSxModu == RTK_DVBSx_MODU_16APSK && u8NewRxheaderCod == 4)
		{
			pstDVBSxParams->DVBSxType = RTK_DEMOD_DVBS2X_MODE;
			pstDVBSxParams->DVBS2XCodeRate = S2X_Mod_CodeRate_table[0].DVBS2XCodeRate;
		}
		else if(pstDVBSxParams->DVBSxModu == RTK_DVBSx_MODU_32APSKL && u8NewRxheaderCod == 5)
		{
			pstDVBSxParams->DVBSxType = RTK_DEMOD_DVBS2X_MODE;
			pstDVBSxParams->DVBS2XCodeRate = S2X_Mod_CodeRate_table[1].DVBS2XCodeRate;
		}
		else if((pstDVBSxParams->DVBSxModu == RTK_DVBSx_MODU_QPSK || pstDVBSxParams->DVBSxModu == RTK_DVBSx_MODU_8PSK || pstDVBSxParams->DVBSxModu == RTK_DVBSx_MODU_8PSKL ||
			pstDVBSxParams->DVBSxModu == RTK_DVBSx_MODU_16APSK || pstDVBSxParams->DVBSxModu == RTK_DVBSx_MODU_16APSKL || pstDVBSxParams->DVBSxModu == RTK_DVBSx_MODU_32APSK ||
			pstDVBSxParams->DVBSxModu == RTK_DVBSx_MODU_32APSKL) && (u8NewRxheaderCod >= 24 && u8NewRxheaderCod <= 48))
		{
			//REALTEK_A_DVBSX_INFO("\033[1;32;33m" "RTK_DEMOD_DVBS2X_MODE\n" "\033[m");
			pstDVBSxParams->DVBSxType = RTK_DEMOD_DVBS2X_MODE;
			pstDVBSxParams->DVBS2XCodeRate = S2X_Mod_CodeRate_table[u8NewRxheaderCod-22].DVBS2XCodeRate;
		}

		status |= RTK_A_DVBSX_GetDVBSxSymbolRate(pDemod, &ui32SymbolRateHz);
		if(status == RTK_DEMOD_SUCCESSFUL)
		{
			pstDVBSxParams->DVBSxSymbolRateHz = ui32SymbolRateHz;
		}
		
		/*Get ISI id list */
		RTK_A_DVBSX_GetDVBSxISI(pDemod, pstDVBSxParams->u8DVBSxIsiIdList, &pstDVBSxParams->u8DVBSxISINumber);
	}

	if((u8DVBSMode == 1) && (status == RTK_DEMOD_SUCCESSFUL)) /*DVB-S mode*/
	{
		//REALTEK_A_DVBSX_INFO("\033[1;32;33m" "RTK_DEMOD_DVBS_MODE\n" "\033[m");
		pstDVBSxParams->DVBSxType = RTK_DEMOD_DVBS_MODE;
		pstDVBSxParams->DVBSxModu = RTK_DVBSx_MODU_QPSK;
		status |= RTK_A_DVBSX_Read(pDemod, 0x2c, 0x15, &u8Rxheader);
		u8Rxheader &= 0x3f;/**/
		if(u8Rxheader <= 1)//1/2
		{
			u32Reg = 0;
		}
		else if(u8Rxheader <= 7)//2/3
		{
			u32Reg = 1;
		}
		else if(u8Rxheader <= 11)//3/4
		{
			u32Reg = 2;
		}
		else if(u8Rxheader <= 17)//5/6
		{
			u32Reg = 3;
		}
		else if(u8Rxheader <= 25)//7/8
		{
			u32Reg = 4;
		}
		else
		{
			u32Reg = 5;
		}

		if(u32Reg <= 4)
		{
			pstDVBSxParams->DVBSCodeRate = (RTK_DVBS_CODE_RATE_TYPE)(u32Reg);
		}
		status |= RTK_A_DVBSX_GetDVBSxSymbolRate(pDemod, &ui32SymbolRateHz);
		if(status == RTK_DEMOD_SUCCESSFUL)
		{
			pstDVBSxParams->DVBSxSymbolRateHz = ui32SymbolRateHz;
		}
	}

	return status;
}

RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxRollOff(DVBSX_DEMOD_MODULE *pDemod, RTK_DVBS2_ROLL_OFF_TYPE *RollOffType)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned long stime = 0;
	unsigned char u8RollOff = 0xFF, u8RegVal = 0, u8LowRollOff = 0;

	stime = tv_osal_time();
	while((tv_osal_time() - stime) < 40)
	{
		status = RTK_A_DVBSX_LatchOn(pDemod);
		status = RTK_A_DVBSX_Read(pDemod, 0x17, 0x85, &u8RegVal);
		u8RegVal = u8RegVal&0x03;
		status = RTK_A_DVBSX_LatchOff(pDemod);

		if(u8RegVal == 0x03)
		{
			u8LowRollOff = 1;
		}
		else
		{
			u8RollOff = u8RegVal;
		}

		if(u8LowRollOff == 1 && u8RollOff != 0xFF)
			break;

		tv_osal_msleep(4);
	}

	if(u8LowRollOff)
	{
		if(u8RollOff == 0x00)
			*RollOffType = RTK_DVBS2_ROLL_OFF_0p15;
		else if(u8RollOff == 0x01)
			*RollOffType = RTK_DVBS2_ROLL_OFF_0p10;
		else if(u8RollOff == 0x02)
			*RollOffType = RTK_DVBS2_ROLL_OFF_0p05;
	}
	else
	{
		if(u8RollOff == 0x00)
			*RollOffType = RTK_DVBS2_ROLL_OFF_0p35;
		else if(u8RollOff == 0x01)
			*RollOffType = RTK_DVBS2_ROLL_OFF_0p25;
		else if(u8RollOff == 0x02)
			*RollOffType = RTK_DVBS2_ROLL_OFF_0p20;
	}

	//REALTEK_A_DVBSX_INFO("\033[1;32;33m" "u8LowRollOff = %u, u8RollOff = %u, *RollOffType = %u\n" "\033[m", u8LowRollOff, u8RollOff, *RollOffType);

	return status;
}

RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDvBSxSpectrum(DVBSX_DEMOD_MODULE *pDemod, RTK_DVBSx_SIGNAL_SPECTRUM *pSignalSpectrum)
{
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;
	unsigned char u8ChipVersion = 0, ui8IQValue = 0, u8LockStatus = 0, ui8SxMode = 0;

	status = RTK_A_DVBSX_Read(pDemod, 0x00, 0x2b, &ui8SxMode);
	status |= RTK_A_DVBSX_Read(pDemod, 0x00, 0x00, &u8ChipVersion);
	//status |= RTK_A_DVBSX_TSLockFlag(&u8LockStatus);
	status |= RTK_A_DVBSX_FECLockFlag(pDemod, &u8LockStatus);
	*pSignalSpectrum = RTK_DVBSx_SIGNAL_SPECTRUM_UNKNOWN;
	if(((u8ChipVersion&0xff) == 0xa8) && (u8LockStatus) && ((ui8SxMode&0x01) == 0x01))
	{
		status |= RTK_A_DVBSX_Read(pDemod, 0x29, 0xff, &ui8IQValue);
		ui8IQValue = ui8IQValue&0x0f;

		if(ui8IQValue > 0x03)
		{
			*pSignalSpectrum = RTK_DVBSx_SIGNAL_SPECTRUM_INVERSE;
			REALTEK_A_DVBSX_INFO("DVBSx_SIGNAL_SPECTRUM_INVERSE\n");
		}
		else if((ui8IQValue == 0x01)||(ui8IQValue == 0x02))
		{
			*pSignalSpectrum = RTK_DVBSx_SIGNAL_SPECTRUM_NORMAL;
			REALTEK_A_DVBSX_INFO("DVBSx_SIGNAL_SPECTRUM_NORMAL\n");
		}
		else
		{
			*pSignalSpectrum = RTK_DVBSx_SIGNAL_SPECTRUM_UNKNOWN;
			REALTEK_A_DVBSX_INFO("DVBSx_SIGNAL_SPECTRUM_UNKNOWN\n");
		}
	}

	return status;
}

RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvbSxSymbolScan10M(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned char RegVal = 0;
	unsigned int cfo_limitR = 0, cfo_limitL = 0;
	int i32cfoLimitMHz = 0;
	int i32AdcClkMHz = 0;

	ui8Status = RTK_A_DVBSX_Write(pDemod, 0x09, 0xA6, 1); //set blind scan mode 
	ui8Status = RTK_A_DVBSX_Write(pDemod, 0x15, 0x59, 0x00);
	ui8Status = RTK_A_DVBSX_Write(pDemod, 0x15, 0x95, 0x00);

	ui8Status = RTK_A_DVBSX_Write(pDemod, 0x00, 0x09, 0x00);
	ui8Status = RTK_A_DVBSX_Write(pDemod, 0x00, 0x0a, 0x00);
	ui8Status = RTK_A_DVBSX_Write(pDemod, 0x00, 0x14, 0x00);
	ui8Status = RTK_A_DVBSX_Write(pDemod, 0x00, 0x15, 0x00);

	i32cfoLimitMHz = 10;
	i32AdcClkMHz = gstRtkDemodDvbConfig.ui32DVBSAdcClkKHz/1000;
	cfo_limitR = (unsigned int)(12*32768/i32AdcClkMHz); //cfo_limitR = (12*32768/FS_ADC)
	cfo_limitL = (unsigned int)(65536 - 12*32768/i32AdcClkMHz); //cfo_limitL = (-12*32768/FS_ADC)
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x20, (unsigned char)(cfo_limitR&0xFF)); //dvbs_cfo_max_limit, default is 2730*2
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x21, (unsigned char)((cfo_limitR>>8)&0xFF));
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x22, (unsigned char)(cfo_limitL&0xFF)); //dvbs_cfo_min_limit, default is -2730*2
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x23, (unsigned char)((cfo_limitL>>8)&0xFF));
	
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x15, 0x34, &RegVal);
	RegVal &= 0x7F;
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x34, RegVal);
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 1); // Reset DSP
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0);

	return ui8Status;
}

void RTK_A_DVBSX_AutoSymbolLockLoopCheck(DVBSX_DEMOD_MODULE *pDemod, unsigned char *pu8DvbSxLockStatus)
{
	int u32Iter = 0, u32FindIter = 0;
	int u32TmpAbsCfo = 0;
	unsigned char u8SameSymbolFound = 0, u8index = 0, u8LockFlag = 0;
	stBlindScanCheckedParams channelSymbols;

	channelSymbols.u8ScanedSymbolNum = 0;
	for(u32Iter = 0; u32Iter < MAX_SCANED_SYMBOL_NUM; u32Iter++)
	{
		channelSymbols.ScanedSymbols[u32Iter].u32SymbolRateKHz = 0;
		channelSymbols.ScanedSymbols[u32Iter].u32CoarseSymbolRateKHz = 0;
		channelSymbols.ScanedSymbols[u32Iter].i32ScanedCFO = 0;
		channelSymbols.ScanedSymbols[u32Iter].u32absCFO = 0;
	}

	for(u32Iter = 0; u32Iter < gScanedParams.u8ScanedSymbolNum; u32Iter++)
	{
		u32FindIter = channelSymbols.u8ScanedSymbolNum;
		u8SameSymbolFound = 0;
		u32TmpAbsCfo = gScanedParams.ScanedSymbols[u32Iter].u32absCFO;

		while(u32FindIter > 0)
		{
			if(RTK_A_DVBSX_AbsSub(gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz, channelSymbols.ScanedSymbols[u32FindIter -1].u32SymbolRateKHz) < (gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz/900))
			{
				u8SameSymbolFound = 1;
				break;
			}
			u32FindIter--;
		};
		//Add new symbol to Symbol list.
		if((!u8SameSymbolFound) && (gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz > 1990) && (gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz < 45100))
		{
			//Add new symbol to symbol list.
			u8index = channelSymbols.u8ScanedSymbolNum;
			channelSymbols.ScanedSymbols[u8index].i32ScanedCFO = gScanedParams.ScanedSymbols[u32Iter].i32ScanedCFO;
			channelSymbols.ScanedSymbols[u8index].u32SymbolRateKHz = gScanedParams.ScanedSymbols[u32Iter].u32SymbolRateKHz;
			channelSymbols.u8ScanedSymbolNum = channelSymbols.u8ScanedSymbolNum + 1;
		}
	}
	for(u32Iter = 0; u32Iter < channelSymbols.u8ScanedSymbolNum; u32Iter++)
	{
		RTK_A_DVBSX_SetDvbSxMode(pDemod, u32DefaultStandard, channelSymbols.ScanedSymbols[u32Iter].u32SymbolRateKHz*1000, DUMMY_FREQ_KHZ);
		RTK_A_DVBSX_ChannelLockCheck(pDemod, &u8LockFlag);
		REALTEK_A_DVBSX_INFO("RTK Demod RTK_A_DVBSX_AutoSymbolLockLoopCheck SymbolRate:%d K lock status:%d!\n", channelSymbols.ScanedSymbols[u32Iter].u32SymbolRateKHz, u8LockFlag);
		if(u8LockFlag == 1)
		{
			*pu8DvbSxLockStatus = u8LockFlag;
			return;
		}
	}
}

/*
Set auto symbol mode when symbol rate is unknown
RTK_A_DVBSX_SetDvbSxMode is recomended to set DVB-S/S2 mode if symbol rate was given.
In This function
1, Detect all symbols
2, Lock each signal until Locked will take more time than RTK_A_DVBSX_SetDvbSxMode fucntion.
*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDvbSxAutoSymbolScanMode(DVBSX_DEMOD_MODULE *pDemod, unsigned int *pui32SymbolRateKHz)
{
	unsigned char u8DVBSxLockedFlag = 0;
	unsigned int ui32SymbolRateHz = 0;
	RTK_A_DVBSX_STATUS status = RTK_DEMOD_SUCCESSFUL;

	status = RTK_A_DVBSX_SetDvbSxMode(pDemod, u32DefaultStandard, 45000000, DUMMY_FREQ_KHZ);
	//RTK_A_DVBSX_SetDvbSxModeAutoSymbolRate();
	status |= RTK_A_DVBSX_SetDvbSxSymbolScan10M(pDemod);
	RTK_A_DVBSX_GetScanedSymbols(pDemod);
	RTK_A_DVBSX_SortSymbolRateAndCFO(pDemod);
	RTK_A_DVBSX_AutoSymbolLockLoopCheck(pDemod, &u8DVBSxLockedFlag);
	if(!u8DVBSxLockedFlag)
	{
		REALTEK_A_DVBSX_INFO("RTK Demod RTK_A_DVBSX_SetDvbSxAutoSymbolScanMode unlocked!\n");
		status |= RTK_A_DVBSX_SetDvbSxMode(pDemod, u32DefaultStandard, 45000000, DUMMY_FREQ_KHZ);
		*pui32SymbolRateKHz = 45000;
	}
	else
	{
		REALTEK_A_DVBSX_INFO("RTK Demod RTK_A_DVBSX_SetDvbSxAutoSymbolScanMode locked symbolRate:%d!\n", ui32SymbolRateHz);
		RTK_A_DVBSX_GetDVBSxSymbolRate(pDemod, &ui32SymbolRateHz);
		*pui32SymbolRateKHz = ui32SymbolRateHz/1000;
	}
	
	return status;
}

/*Get ISI ID list and stream number*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_GetDVBSxISI(DVBSX_DEMOD_MODULE *pDemod, unsigned char *pu8ISI_Ids, unsigned char *u8s2_isi_num)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned char u8ISINumber = 0;
	unsigned char u8Iter = 0;
	unsigned char u8temp;
	unsigned char i, j;

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, 0x86, &u8ISINumber);
	for(u8Iter = 0; u8Iter < 8; u8Iter++)
	{
		pu8ISI_Ids[u8Iter] = 0;
	}
	if(u8ISINumber > 8) /*Maximum 8 streams*/
	{
		u8ISINumber = 8;
	}
	for(u8Iter = 0; u8Iter < u8ISINumber; u8Iter++)
	{
		ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x18, u8Iter+0x7e, &pu8ISI_Ids[u8Iter]);
	}
	//Sort the data list
	for(i = 0; i < u8ISINumber-1; i++)
	{
		for(j = 0; j < u8ISINumber-1-i; j++)
		{
			if(pu8ISI_Ids[j] > pu8ISI_Ids[j+1])
			{
				u8temp = pu8ISI_Ids[j];
				pu8ISI_Ids[j] = pu8ISI_Ids[j+1];
				pu8ISI_Ids[j+1] = u8temp;
			}
		}
	}
	*u8s2_isi_num = u8ISINumber;

	return ui8Status;
}

/*Set VCM selected stream ID.*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDVBSxISI(DVBSX_DEMOD_MODULE *pDemod, unsigned char u8SelectID)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;

	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x18, 0x89, u8SelectID);
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x18, 0x8a, 0x01);
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0x01);
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0x00);

	return ui8Status;
}

/*Internal used function */
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetSymbolRatePoint(DVBSX_DEMOD_MODULE *pDemod, unsigned int u32SymbolRteK)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned int SymbolRatePoint = 0, CSymbolRatePoint = 0;
	unsigned int AdcClkKHz = gstRtkDemodDvbConfig.ui32DVBSAdcClkKHz;

	if(u32SymbolRteK > 46000)
	{
		return ui8Status;
	}

	SymbolRatePoint = (u32SymbolRteK *32768 *32)/AdcClkKHz;

	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x92,(unsigned char)(SymbolRatePoint&0xff));
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x93,(unsigned char)((SymbolRatePoint>>8)&0xff));
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x94,(unsigned char)((SymbolRatePoint>>16)&0xff));

	SymbolRatePoint = (u32SymbolRteK *32768)/AdcClkKHz;

	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x09, 0x0d, (unsigned char)(CSymbolRatePoint&0xff)); // not used. coarse_symbol_maybe_end_prog_1 20*16834/120*2
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x09, 0x0e, (unsigned char)((CSymbolRatePoint>>8)&0xff));
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x09, 0x0f, (unsigned char)((CSymbolRatePoint>>16)&0xff));

	return ui8Status;
}

/*Internal used function */
RTK_A_DVBSX_STATUS RTK_A_DVBSX_ForceSymbolRateandCFO(DVBSX_DEMOD_MODULE *pDemod, unsigned int u32SymbolRteK, int i32CFOKHz)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	int i32CFOSet = 0;
	int AdcClkKHz = gstRtkDemodDvbConfig.ui32DVBSAdcClkKHz;

	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x09, 0xa6, 0x00); // params_blind_scan_state[0] num_fine_results2break[7:1]=0
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x59, 0x01); // symbol_rate_force_en

	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x95, 0x01); // symbol_rate_point_force_en symbol_rate_point_use_reg
	RTK_A_DVBSX_SetSymbolRatePoint(pDemod, u32SymbolRteK);
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x15, 0x3f, 0x01); //dvbc_bw_bypass

	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x09, 0x0c, 0x03); //coarse_symbol_prog_mode[0] coarse_cfo_prog_mode[1]
	i32CFOSet = (int)((i32CFOKHz*16384*2)/AdcClkKHz);
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x09, 0x91, (i32CFOSet&0xff)); //params_coarse_cfo_prog 5*16384/120*2
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x09, 0x92, ((i32CFOSet>>8)&0xff));
	//ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x09, 0x91, 0x00); //params_coarse_cfo_prog 5*16384/120*2
	//ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x09, 0x92, 0x00);

	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x0a, 0x72, 0x01); //cci bypass
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x09, 0xb0, 0x00); //params_DVBS_rate_auto_det

	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0x01); //cci bypass
	ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x00, 0x05, 0x00); //params_DVBS_rate_auto_det
	
	return ui8Status;
}

//Sample code for auto tuning when VMC stream, without tuner code.
//pseudo code
void DVBSxAutoScanAPI(DVBSX_DEMOD_MODULE *pDemod, unsigned int u32VCMSigalSymbolrateHz)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	RTK_DVBSx_SIGNAL_PARAMS stDVBSxParams;
	unsigned char VCMnumber = 0, u8Iter = 0, u8StreamID = 0, uLockfalg = 0;

	ui8Status |= RTK_A_DVBSX_SetDvbSxMode(pDemod, u32DefaultStandard, u32VCMSigalSymbolrateHz, DUMMY_FREQ_KHZ); //set demod
	ui8Status |= RTK_A_DVBSX_ChannelLockCheck(pDemod, &uLockfalg); //get lock status
	if(uLockfalg)
	{
		ui8Status |= RTK_A_DVBSX_GetDVBSxSignalParams(pDemod, &stDVBSxParams); //Get signal parameters
		if(stDVBSxParams.u8DVBSxISINumber > 1) //VCM signal
		{
			VCMnumber = stDVBSxParams.u8DVBSxISINumber;

			for(u8Iter = 0; u8Iter < VCMnumber; u8Iter++) //Iter each stream.
			{
				u8StreamID = stDVBSxParams.u8DVBSxIsiIdList[u8Iter];

				ui8Status |= RTK_A_DVBSX_SetDVBSxISI(pDemod, u8StreamID);

				ui8Status |= RTK_A_DVBSX_ChannelLockCheck(pDemod, &uLockfalg);
				if(uLockfalg)
				{
					//Play TV or parse and save programs information.
				}
				else
				{
					//Signal not locked
				}
			}
		}
		else //if(stDVBSxParams.u8DVBSxISINumber > 1)
		{
			//single stream
		}
	}
	else //if(uLockfalg)
	{
		//Signal not locked
	}
}

//Sample code for single channel tuning when VMC stream, without tuner code.
//pseudo code
void DVBSxSingleChannelAPI(DVBSX_DEMOD_MODULE *pDemod, unsigned int u32VCMSigalSymbolrateHz)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	RTK_DVBSx_SIGNAL_PARAMS stDVBSxParams;
	unsigned char VCMnumber = 0, u8StreamID = 0, uLockfalg = 0;

	ui8Status |= RTK_A_DVBSX_SetDvbSxMode(pDemod, u32DefaultStandard, u32VCMSigalSymbolrateHz, DUMMY_FREQ_KHZ); //set demod
	ui8Status |= RTK_A_DVBSX_ChannelLockCheck(pDemod, &uLockfalg); //get lock status
	if(uLockfalg)
	{
		ui8Status |= RTK_A_DVBSX_GetDVBSxSignalParams(pDemod, &stDVBSxParams); //Get signal parameters
		if(stDVBSxParams.u8DVBSxISINumber > 1) //VCM signal
		{
			VCMnumber = stDVBSxParams.u8DVBSxISINumber;
			/*
			get StreamID from SoC's database, (assume SoC already auto scaned and saved the streamID into DB)
			Here the SoC MUST set one ISI, unless the demod could select one ISI randomly to play 
			*/
			ui8Status |= RTK_A_DVBSX_SetDVBSxISI(pDemod, u8StreamID);

			ui8Status |= RTK_A_DVBSX_ChannelLockCheck(pDemod, &uLockfalg);
			if(uLockfalg)
			{
				//Play TV or parse programs information.
			}
			else
			{
				//Signal not locked
			}
			
		}
		else //if(stDVBSxParams.u8DVBSxISINumber > 1)
		{
			//single stream
		}
	}
	else //if(uLockfalg)
	{
		//Signal not locked
	}
}

/*
RTK_A_DVBSX_DPLLTrackingSpeedPatch is used to adjust DVB-S/S2 DPLL tracking speed when carrier frequency offset changes rapidly.
It should be called periodically if required.
*/
RTK_A_DVBSX_STATUS RTK_A_DVBSX_DPLLTrackingSpeedPatch(DVBSX_DEMOD_MODULE *pDemod)
{
	//ro_dpfll_reset_cnt=2c 55
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned char ui8RstCnt = 0, u8Thresh = 0, u8DVBSxMode = 0, u8RegThresh = 0x82, u8Rxheader = 0;
	RTK_DVBSx_SIGNAL_PARAMS stDVBSxParams;

	ui8Status = RTK_A_DVBSX_DVBSxModeDetected(pDemod, &u8DVBSxMode);
	ui8Status |= RTK_A_DVBSX_GetDVBSxSignalParams(pDemod, &stDVBSxParams);
	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x2c, 0x28, &u8Rxheader);

	if(stDVBSxParams.u8DVBSxISINumber == 1)
	{
		if(stPreSxParams.DVBSxType!=0 && stPreSxParams.DVBSxModu!=0 && stPreSxParams.DVBSCodeRate!=0 && stPreSxParams.DVBS2CodeRate!=0)
		{
			if(((u8Rxheader&0x1) != Pre_Plt) || (stDVBSxParams.DVBSxType != stPreSxParams.DVBSxType) || (stDVBSxParams.DVBSxModu != stPreSxParams.DVBSxModu) 
				|| (stDVBSxParams.DVBSCodeRate != stPreSxParams.DVBSCodeRate) || (stDVBSxParams.DVBS2CodeRate != stPreSxParams.DVBS2CodeRate))
			{
				REALTEK_A_DVBSX_INFO("\033[1;32;36m" "[Current] Plt=%u SxTpye=%u SxModu=%u DVBSCr=%u DVBS2Cr=%u\n" "\033[m", u8Rxheader&0x1, stDVBSxParams.DVBSxType, stDVBSxParams.DVBSxModu, stDVBSxParams.DVBSCodeRate, stDVBSxParams.DVBS2CodeRate);
				REALTEK_A_DVBSX_INFO("\033[1;32;36m" "Pre_Plt=%u Pre_SxTpye=%u Pre_SxModu=%u Pre_DVBSCr=%u Pre_DVBS2Cr=%u\n" "\033[m", Pre_Plt, stPreSxParams.DVBSxType, stPreSxParams.DVBSxModu, stPreSxParams.DVBSCodeRate, stPreSxParams.DVBS2CodeRate);
				ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x2a, 0x54, 0x10);
				ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x2a, 0x55, 0x1a);

				ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x29, 0x12, 0x4f);
				ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x29, 0x14, 0x02);

				ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x2a, 0x56, 0x80);
				ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x2a, 0x57, 0x10);

				ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x2d, 0x56, 0x01);
				ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x2a, 0xda, 0x03);
			}
		}

		if(u8DVBSxMode == RTK_DEMOD_DVBS2_MODE) /*DVB-S2 mode*/
		{
			if(stDVBSxParams.DVBSxModu == RTK_DVBSx_MODU_8PSK)
			{
				ui8Status |= RTK_A_DVBSX_LatchOn(pDemod);
				ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x2c, 0x55, &ui8RstCnt);
				ui8Status |= RTK_A_DVBSX_LatchOff(pDemod);
				ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x2a, 0x54, &u8Thresh);

				if((ui8RstCnt > 0) && (u8Thresh != u8RegThresh))
				{
					ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x2a, 0x54, u8RegThresh);
					ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x2a, 0x55, 0x34);

					ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x29, 0x12, 0x4f);
					ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x29, 0x14, 0x10);

					ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x2a, 0x56, 0xff);
					ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x2a, 0x57, 0xd1);
	
					ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x2d, 0x56, 0x00);
					ui8Status |= RTK_A_DVBSX_Write(pDemod, 0x2a, 0xda, 0x02);
				} //PLL module reset detected.
			} //8PSK
		} //S2 Mode
	}
	else
	{
		stPreSxParams.DVBSxType = 0;
		stPreSxParams.DVBSxModu = 0;
		stPreSxParams.DVBSCodeRate = 0;
		stPreSxParams.DVBS2CodeRate = 0;
	}

	//Keep signal info
	if(u8stLockCnt >= 3)
	{
		Pre_Plt = u8Rxheader & 0x1;
		stPreSxParams.DVBSxType = stDVBSxParams.DVBSxType;
		stPreSxParams.DVBSxModu = stDVBSxParams.DVBSxModu;
		stPreSxParams.DVBSCodeRate = stDVBSxParams.DVBSCodeRate;
		stPreSxParams.DVBS2CodeRate = stDVBSxParams.DVBS2CodeRate;
	}

	return ui8Status;
}

RTK_A_DVBSX_STATUS RTK_A_DVBSX_PhaseTrackingSpeedPatch(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned char phase_mode = 0, cmp_reg = 0;
	
	RTK_A_DVBSX_Read(pDemod, 0x09, 0x48, &cmp_reg);
	
	if(cmp_reg != 0x03)
	{
		RTK_A_DVBSX_LatchOn(pDemod);
		RTK_A_DVBSX_Read(pDemod, 0x2c, 0x59, &phase_mode);
		RTK_A_DVBSX_LatchOff(pDemod);
		
		if(phase_mode)
		{
			REALTEK_A_DVBSX_INFO("phase_noise_flag_ro %u, DAGC Resume\n", phase_mode);
			RTK_A_DVBSX_Write(pDemod, 0x09, 0x48, 0x03);
			RTK_A_DVBSX_Write(pDemod, 0x0a, 0x4b, 0x03);
			RTK_A_DVBSX_Write(pDemod, 0x09, 0x3f, 0x01);
			RTK_A_DVBSX_Write(pDemod, 0x09, 0x49, 0x01);
			RTK_A_DVBSX_Write(pDemod, 0x09, 0x40, 0x03);
			RTK_A_DVBSX_Write(pDemod, 0x09, 0x4a, 0x03);
			
			RTK_A_DVBSX_Write(pDemod, 0x0a, 0xf6, 0x05);
			RTK_A_DVBSX_Write(pDemod, 0x0a, 0xf7, 0x05);
			
			RTK_A_DVBSX_Write(pDemod, 0x10, 0x09, 0x80);
			RTK_A_DVBSX_Write(pDemod, 0x10, 0x0a, 0x20);
			RTK_A_DVBSX_Write(pDemod, 0x10, 0x0b, 0x04);
			RTK_A_DVBSX_Write(pDemod, 0x10, 0x0c, 0x01);
		}
	}
	
	return ui8Status;
}

RTK_A_DVBSX_STATUS RTK_A_DVBSX_PilotTrackingAdjust(DVBSX_DEMOD_MODULE *pDemod, unsigned int symbolrate)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	RTK_DVBSx_SIGNAL_PARAMS stDVBSxParams;
	unsigned char u8DVBSxMode = 0, u8Rxheader = 0, PrePlt = 0;
	//int Snr = 0;

	/*Set default unknown*/
	stDVBSxParams.DVBSxModu = RTK_DVBSx_MODU_UNKNOWN;
	//stDVBSxParams.DVBS2CodeRate = RTK_DVBS2_CODE_RATE_UNKNOWN;
	//stDVBSxParams.DVBS2XCodeRate = RTK_DVBS2X_CODE_RATE_UNKNOWN;

	ui8Status = RTK_A_DVBSX_DVBSxModeDetected(pDemod, &u8DVBSxMode);
	//ui8Status |= RTK_A_DVBSX_GetDVBSxSignalParams(pDemod, &stDVBSxParams);
	RTK_A_DVBSX_Read(pDemod, 0x2c, 0x28, &u8Rxheader);//pilot mode
	//RTK_A_DVBSX_GetDVBSxSNR(pDemod, &Snr);

	if((u8DVBSxMode == RTK_DEMOD_DVBS2_MODE) && ((u8Rxheader & 0x01) == 1))
	{
		RTK_A_DVBSX_Read(pDemod, 0x2d, 0x49, &PrePlt);
		if(PrePlt == 0x01)
		{
			REALTEK_A_DVBSX_INFO("\033[1;32;34m" "Lock Pilot advance\n" "\033[m");
			RTK_A_DVBSX_Write(pDemod, 0x2d, 0x49, 0x00);
			RTK_A_DVBSX_Write(pDemod, 0x2d, 0x23, 0x00);
			RTK_A_DVBSX_Write(pDemod, 0x2a, 0xed, 0x00); //pilot_train_use_pilot_phase
		}
	}
	else
	{
		RTK_A_DVBSX_Read(pDemod, 0x2d, 0x49, &PrePlt);
		if(PrePlt == 0x00)
		{
			REALTEK_A_DVBSX_INFO("\033[1;32;34m" " Resume Pilot advance\n" "\033[m");
			RTK_A_DVBSX_Write(pDemod, 0x2d, 0x49, 0x01);
			RTK_A_DVBSX_Write(pDemod, 0x2d, 0x23, 0x0e);
			RTK_A_DVBSX_Write(pDemod, 0x2a, 0xed, 0x04); //pilot_train_use_pilot_phase
		}
	}

	//REALTEK_A_DVBSX_INFO("Check Maxnum = %u SNR=%d \n", rtd_inl(ATB_DEMOD_FREQ_LDPC_ITER_NUM_5_0_reg), Snr);
	return ui8Status;
}

//CnPerformancePatch is for Old Demod Version
RTK_A_DVBSX_STATUS RTK_A_DVBSX_CnPerformancePatch(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned char ui8c1_pll_2 = 0, u8DVBSxMode = 0;

	ui8Status |= RTK_A_DVBSX_DVBSxModeDetected(pDemod, &u8DVBSxMode);

	if(u8DVBSxMode != 255)
	{
		ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x29, 0x26, &ui8c1_pll_2);
		if(u8DVBSxMode == RTK_DEMOD_DVBS_MODE) //DVB-S mode
		{
			//REALTEK_A_DVBSX_INFO("DVBS MODE CN PerformancePatch Read 0x2926 = 0x%x\n", ui8c1_pll_2);
			if(ui8c1_pll_2 != 0x02)
			{
				REALTEK_A_DVBSX_INFO("SET DVBS CN Registers\n");
				RTK_A_DVBSX_Write(pDemod, 0x29, 0x26, 0x02); //params_c1_pll_2
				RTK_A_DVBSX_Write(pDemod, 0x29, 0x27, 0x01); //params_c2_pll_2
				RTK_A_DVBSX_Write(pDemod, 0x29, 0x30, 0x09); //params_feed_f_tap_n
				RTK_A_DVBSX_Write(pDemod, 0x29, 0x31, 0x05); //Params_feed_b_tap_n
				RTK_A_DVBSX_Write(pDemod, 0x29, 0x32, 0x05); //Params_main_path_index 5->4
			}
		}
		else if(u8DVBSxMode == RTK_DEMOD_DVBS2_MODE) //DVB-S2 mode
		{
			//REALTEK_A_DVBSX_INFO("DVBS2 MODE CN PerformancePatch Read 0x2926 = 0x%x\n", ui8c1_pll_2);
			if(ui8c1_pll_2 != 0x05)
			{
				REALTEK_A_DVBSX_INFO("SET DVBS2 CN Registers\n");
				RTK_A_DVBSX_Write(pDemod, 0x29, 0x26, 0x05); //params_c1_pll_2
				RTK_A_DVBSX_Write(pDemod, 0x29, 0x27, 0x03); //params_c2_pll_2
				RTK_A_DVBSX_Write(pDemod, 0x29, 0x30, 0x15); //params_feed_f_tap_n
				RTK_A_DVBSX_Write(pDemod, 0x29, 0x31, 0x0a); //Params_feed_b_tap_n
				RTK_A_DVBSX_Write(pDemod, 0x29, 0x32, 0x0a); //Params_main_path_index
			}
		}
	}

	return ui8Status;
}

//CnPerformancePatch is for Old Demod Version
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDVBSCnPerformance(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned char ui8c1_pll_2 = 0;

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x29, 0x26, &ui8c1_pll_2);
	
	//REALTEK_A_DVBSX_INFO("Init SET DVBS CN Register\n");

	if(ui8c1_pll_2 != 0x02)
	{
		RTK_A_DVBSX_Write(pDemod, 0x29, 0x26, 0x02); //params_c1_pll_2
		RTK_A_DVBSX_Write(pDemod, 0x29, 0x27, 0x01); //params_c2_pll_2
		RTK_A_DVBSX_Write(pDemod, 0x29, 0x30, 0x09); //params_feed_f_tap_n
		RTK_A_DVBSX_Write(pDemod, 0x29, 0x31, 0x05); //Params_feed_b_tap_n
		RTK_A_DVBSX_Write(pDemod, 0x29, 0x32, 0x05); //Params_main_path_index 5->4
	}

	return ui8Status;
}

//CnPerformancePatch is for Old Demod Version
RTK_A_DVBSX_STATUS RTK_A_DVBSX_SetDVBS2CnPerformance(DVBSX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBSX_STATUS ui8Status = RTK_DEMOD_SUCCESSFUL;
	unsigned char ui8c1_pll_2 = 0;

	ui8Status |= RTK_A_DVBSX_Read(pDemod, 0x29, 0x26, &ui8c1_pll_2);
	
	//REALTEK_A_DVBSX_INFO("Init SET DVBS2 CN Register\n");

	if(ui8c1_pll_2 != 0x05)
	{
		RTK_A_DVBSX_Write(pDemod, 0x29, 0x26, 0x05); //params_c1_pll_2
		RTK_A_DVBSX_Write(pDemod, 0x29, 0x27, 0x03); //params_c2_pll_2
		RTK_A_DVBSX_Write(pDemod, 0x29, 0x30, 0x15); //params_feed_f_tap_n
		RTK_A_DVBSX_Write(pDemod, 0x29, 0x31, 0x0a); //Params_feed_b_tap_n
		RTK_A_DVBSX_Write(pDemod, 0x29, 0x32, 0x0a); //Params_main_path_index
	}

	return ui8Status;
}

/**

@brief   REALTEK_I DVBS2 demod module builder

Use BuildRealtekIDvbS2Module() to build REALTEK_I DVBS2 module, set all module function pointers with the corresponding
functions, and initialize module private variables.


@param [in]   ppDemod                        Pointer to REALTEK_I DVBS2 demod module pointer
@param [in]   pDvbs2DemodModuleMemory        Pointer to an allocated DVBS2 demod module memory
@param [in]   pBaseInterfaceModuleMemory     Pointer to an allocated base interface module memory
@param [in]   pI2cBridgeModuleMemory         Pointer to an allocated I2C bridge module memory
@param [in]   DeviceAddr                     REALTEK_I DVBS2 I2C device address
@param [in]   CrystalFreqHz                  REALTEK_I DVBS2 crystal frequency in Hz

@note
	-# One should call BuildRealtekIDvbS2Module() to build REALTEK_I DVBS2 module before using it.

*/
void
BuildRealtekIDvbs2Module(
	DVBSX_DEMOD_MODULE **ppDemod,
	DVBSX_DEMOD_MODULE *pDvbS2DemodModuleMemory,
	BASE_INTERFACE_MODULE *pBaseInterfaceModuleMemory,
	I2C_BRIDGE_MODULE *pI2cBridgeModuleMemory,
	unsigned char DeviceAddr,
	U32BITS CrystalFreqHz
)
{
	DVBSX_DEMOD_MODULE *pDemod;

	// Set demod module pointer and get demod module.
	*ppDemod = pDvbS2DemodModuleMemory;
	pDemod = *ppDemod;

	// Set base interface module pointer and I2C bridge module pointer.
	pDemod->pBaseInterface = pBaseInterfaceModuleMemory;
	pDemod->pI2cBridge     = pI2cBridgeModuleMemory;

	// Set demod type.
	pDemod->DemodType = DVBSX_DEMOD_TYPE_REALTEK_A;

	// Set demod I2C device address.
	pDemod->DeviceAddr = DeviceAddr;

	// Set demod crystal frequency in Hz.
	pDemod->CrystalFreqHz = CrystalFreqHz;

	// Set demod TS interface mode.
	pDemod->TsInterfaceMode = NO_USE;
	
	// Set demod current tuner.
	pDemod->CurrentTunerId = TV_TUNER_NO_TUNER;

	//DiSEqC Control
	pDemod->SetDiseqcContinue22kOnOff       = realtek_Diseqc_SetDiseqcContinue22kOnOff;
	pDemod->SetDiseqcCmd                    = realtek_Diseqc_SetDiseqcCmd;
	pDemod->SetDiseqcToneBurst              = realtek_Diseqc_SetDiseqcToneBurst;
	pDemod->SetDiseqcUnModToneBurst         = realtek_Diseqc_SetDiseqcUnModToneBurst;
	pDemod->SetDiseqcModToneBurst           = realtek_Diseqc_SetDiseqcModToneBurst;

	return;
}

