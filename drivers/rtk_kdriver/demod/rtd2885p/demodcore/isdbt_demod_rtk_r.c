/**

@file

@brief   RTK_R ISDB-T demod module definition

One can manipulate RTK_R ISDB-T demod through RTK_R ISDB-T module.
RTK_R ISDB-T module is derived from ISDB-T demod module.

*/

//#include <stdio.h>
//#include <assert.h>
//#include <string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <rtk_kdriver/io.h>
#include "isdbt_demod_rtk_r.h"
#include "rbus/rtk_dtv_demod_isdbt_reg.h"
#include "demod_rtk_r_isdbt_priv.h"
#include "isdbt_demod_rtk_r_base.h"
#include "rbus/atb_demod_rxflt_reg.h"
#include "rbus/atb_demod_rxflt2_reg.h"
#include "rbus/atb_demod_sync_reg.h"
#include "rbus/dtv_demod_misc_reg.h"

#define IsdbtUseMerlin4InitValue 0
#define PAPR_NO_SIGNAL_THD            30
stRtkrIsdbtConfig gstRTK_R_ISDBT_DvbConfig;
//#define rtd_outl(addr, value)   pli_writeReg32(addr, value)
//#define rtd_inl(addr)           pli_readReg32(addr)
//#define rtd_maskl(addr, mask, value)    rtd_outl(addr, ((rtd_inl(addr) & mask) | value))


/**

@brief   RTK_R ISDB-T demod module builder

Use BuildRtkrIsdbtModule() to build RTK_R ISDB-T module, set all module function pointers with the corresponding
functions, and initialize module private variables.


@param [in]   ppDemod                      Pointer to RTK_R ISDB-T demod module pointer
@param [in]   pDvbtDemodModuleMemory       Pointer to an allocated ISDB-T demod module memory
@param [in]   pBaseInterfaceModuleMemory   Pointer to an allocated base interface module memory
@param [in]   pI2cBridgeModuleMemory       Pointer to an allocated I2C bridge module memory
@param [in]   DeviceAddr                   RTK_R ISDB-T I2C device address
@param [in]   CrystalFreqHz                RTK_R ISDB-T crystal frequency in Hz
@param [in]   TsInterfaceMode              RTK_R ISDB-T TS interface mode for setting
@param [in]   IfFreqHz                     RTK_R ISDB-T IF frequency in Hz for setting
@param [in]   SpectrumMode                 RTK_R ISDB-T Spectrum mode for setting
@param [in]   ShareMemoryOfst              RTK_R ISDB-T Offset address of shared memory
@param [in]   ChipVersion                  RTK_R ISDB-T Chip Version

@note
	-# One should call BuildRtkrIsdbtModule() to build RTK_R ISDB-T module before using it.

*/
void
BuildRtkrIsdbtModule(
	ISDBT_DEMOD_MODULE **ppDemod,
	ISDBT_DEMOD_MODULE *pDvbtDemodModuleMemory,
	BASE_INTERFACE_MODULE *pBaseInterfaceModuleMemory,
	I2C_BRIDGE_MODULE *pI2cBridgeModuleMemory,
	unsigned char DeviceAddr,
	U32BITS CrystalFreqHz,
	int TsInterfaceMode,
	U32BITS IfFreqHz,
	int SpectrumMode,
	U32BITS ShareMemoryOfst,
	unsigned char ChipVersion
)
{
	ISDBT_DEMOD_MODULE *pDemod;
	RTK_R_ISDBT_EXTRA_MODULE *pExtra;


	// Set demod module pointer,
	*ppDemod = pDvbtDemodModuleMemory;

	// Get demod module.
	pDemod = *ppDemod;

	// Set base interface module pointer and I2C bridge module pointer.
	pDemod->pBaseInterface = pBaseInterfaceModuleMemory;
	pDemod->pI2cBridge     = pI2cBridgeModuleMemory;

	// Get demod extra module.
	pExtra = &(pDemod->Extra.RtkrIsdbt);


	// Set demod type.
	pDemod->DemodType = ISDBT_DEMOD_TYPE_REALTEK_R;

	// Set demod I2C device address.
	pDemod->DeviceAddr = DeviceAddr;

	// Set demod crystal frequency in Hz.
	pDemod->CrystalFreqHz = CrystalFreqHz;

	// Set demod TS interface mode, IF frequency in Hz, and spectrum mode.
	pDemod->TsInterfaceMode = TsInterfaceMode;
	pDemod->IfFreqHz        = IfFreqHz;
	pDemod->SpectrumMode    = SpectrumMode;


	// Initialize demod parameter setting status
	pDemod->IsBandwidthModeSet = NO;
	pDemod->IsSegmentNumSet    = NO;
	pDemod->IsSubChannelNumSet = NO;


	// Initialize demod register table.
	realtek_r_isdbt_InitRegTable(pDemod);


	// Build I2C birdge module.
	realtek_r_isdbt_BuildI2cBridgeModule(pDemod);


	// Set demod module I2C function pointers with default functions.
	pDemod->SetRegBytes        = isdbt_demod_default_SetRegBytes;
	pDemod->GetRegBytes        = isdbt_demod_default_GetRegBytes;
	pDemod->GetRegMaskBits     = isdbt_demod_default_GetRegMaskBits;
	pDemod->SetRegBits         = isdbt_demod_default_SetRegBits;
	pDemod->GetRegBits         = isdbt_demod_default_GetRegBits;

	// Set demod module I2C function pointers with particular functions.
	pDemod->SetRegMaskBits     = realtek_r_isdb_acceleration_SetRegMaskBits;


	// Set demod module manipulating function pointers with default functions.
	pDemod->GetDemodType     = isdbt_demod_default_GetDemodType;
	pDemod->GetDeviceAddr    = isdbt_demod_default_GetDeviceAddr;
	pDemod->GetCrystalFreqHz = isdbt_demod_default_GetCrystalFreqHz;

	pDemod->GetBandwidthMode = isdbt_demod_default_GetBandwidthMode;
	pDemod->GetSegmentNum    = isdbt_demod_default_GetSegmentNum;
	pDemod->GetSubChannelNum = isdbt_demod_default_GetSubChannelNum;


	// Set demod module manipulating function pointers with particular functions.
	pDemod->IsConnectedToI2c        = realtek_r_isdbt_IsConnectedToI2c;
	pDemod->RestartFsm              = realtek_r_isdbt_RestartFsm;
	pDemod->Initialize              = realtek_r_isdbt_Initialize;
	pDemod->SetParameters           = realtek_r_isdbt_SetParameters;

	pDemod->IsSignalLocked          = realtek_r_isdbt_IsSignalLocked;

	pDemod->IsTrLocked     = realtek_r_isdbt_IsTrLocked;
	pDemod->IsCrLocked     = realtek_r_isdbt_IsCrLocked;
	pDemod->IsInnerLocked  = realtek_r_isdbt_IsInnerLocked;
	pDemod->IsNoSig  = realtek_r_isdbt_IsNoSig;

	pDemod->GetSignalStrength       = realtek_r_isdbt_GetSignalStrength;
	pDemod->GetLayerSignalQuality   = realtek_r_isdbt_GetLayerSignalQuality;

	pDemod->GetLayerBer             = realtek_r_isdbt_GetLayerBer;
	pDemod->GetLayerPer             = realtek_r_isdbt_GetLayerPer;
	pDemod->GetSnrDb                = realtek_r_isdbt_GetSnrDb;

	pDemod->SetAgcPol               = realtek_a_isdbt_SetAgcPol;
	pDemod->GetRfAgc                = realtek_r_isdbt_GetRfAgc;
	pDemod->GetIfAgc                = realtek_r_isdbt_GetIfAgc;
	pDemod->GetDiAgc                = realtek_r_isdbt_GetDiAgc;

	pDemod->GetTrOffsetPpm          = realtek_r_isdbt_GetTrOffsetPpm;
	pDemod->GetCrOffsetHz           = realtek_r_isdbt_GetCrOffsetHz;

	pDemod->GetLayerConstellation   = realtek_r_isdbt_GetLayerConstellation;
	pDemod->GetLayerCodeRate        = realtek_r_isdbt_GetLayerCodeRate;
	pDemod->GetLayerInterleavingLen = realtek_r_isdbt_GetLayerInterleavingLen;
	pDemod->GetLayerSegmentNum      = realtek_r_isdbt_GetLayerSegmentNum;

	pDemod->GetGuardInterval        = realtek_r_isdbt_GetGuardInterval;
	pDemod->GetFftMode              = realtek_r_isdbt_GetFftMode;

	// 20120207 added by kevin
	pDemod->SetTsInterfaceMode      = realtek_r_isdbt_SetTsInterfaceMode;
	pDemod->SetTsInterfaceClkPol      = realtek_r_isdbt_SetTsInterfaceClkPol;
	pDemod->SetIfFreqHz             = realtek_r_isdbt_SetIfFreqHz;
	pDemod->SetSpectrumMode         = realtek_r_isdbt_SetSpectrumMode;
	pExtra->ShareMemoryOfst         = ShareMemoryOfst;  // start address of shared memory
	pExtra->ChipVersion             = ChipVersion;
	return;
}





/**

@see   ISDBT_DEMOD_FP_IS_CONNECTED_TO_I2C

*/
void
realtek_r_isdbt_IsConnectedToI2c(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	BASE_INTERFACE_MODULE *pBaseInterface;

	unsigned char Nothing;



	// Get base interface.
	pBaseInterface = pDemod->pBaseInterface;


	// Send read command.
	// Note: The number of reading bytes must be greater than 0.
	if (pBaseInterface->I2cRead(pBaseInterface, pDemod->DeviceAddr, NULL, 0, &Nothing, LEN_1_BYTE) == FUNCTION_ERROR)
		goto error_status;


	// Set I2cConnectionStatus with YES.
	*pAnswer = YES;


	return;


error_status:

	// Set I2cConnectionStatus with NO.
	*pAnswer = NO;


	return;
}





/**

@see   ISDBT_DEMOD_FP_RESTART_FSM

*/
int
realtek_r_isdbt_RestartFsm(
	ISDBT_DEMOD_MODULE *pDemod
)
{
/*
	// Set SOFT_RST_N with 0x0. Then, set SOFT_RST_N with 0x1.
	if (pDemod->SetRegBits(pDemod, ISDBT_SOFT_RST_N, 0x0) != FUNCTION_SUCCESS)
		goto error_status;

	if (pDemod->SetRegBits(pDemod, ISDBT_SOFT_RST_N, 0x1) != FUNCTION_SUCCESS)
		goto error_status;
*/
	rtd_maskl(0xb8159804, 0xfffffffe, 0x1);			//[0]:soft_rst_n = 1
	rtd_outl(0xb8160010, 0x00000001);					//[0]:softcfg_done_r
	rtd_outl(0xb8160014, 0x00000001);					//[0]:softrst_rxdsp_r
	rtd_outl(0xb8160014, 0x00000000);					//[0]:softrst_rxdsp_r
	return FUNCTION_SUCCESS;

/*
error_status:
	return FUNCTION_ERROR;
*/
}





/**

@see   ISDBT_DEMOD_FP_INITIALIZE

*/
int
realtek_r_isdbt_Initialize(
	ISDBT_DEMOD_MODULE *pDemod
)
{
#if IsdbtUseMerlin4InitValue
	// Initializing entry only used in Initialize()
	typedef struct {
		unsigned short RegStartAddr;
		unsigned char Msb;
		unsigned char Lsb;
		U32BITS WritingValue;
	}
	INIT_ENTRY;

	// not A version IC
	static const INIT_ENTRY InitTable_VerB[] = {
		// RegStartAddr,   Msb,   Lsb,   WritingValue
		{0xe401,           1,     0,     0x3            },
		{0xe701,           0,     0,     0x0            },
		{0xe704,           7,     0,     0x26           }, //agc target modify to 0x26
		{0xe867,           7,     4,     0x8            },
		{0xe868,           3,     0,     0x9            },
		{0xe629,           2,     0,     0x1            },
		{0xe90f,           23,    0,     0x838382       },
		{0xe91d,           23,    0,     0x858544       },
		{0xe82c,           0,     0,     0x1            },
		{0xe82c,           5,     1,     0xc            },
		{0xe84f,           6,     5,     0x1            },	// 20130530 dt_nfgrp_mode : : group when CCI  or large NF ratio   1:always group   2: no group
		{0xea1a,           4,     1,     0x2            },	// 20121127 for B cut change
		{0xea2e,           1,     1,     0x0            },	// 20121127 for B cut change
		{0xea34,           0,     0,     0x1            },	// 20121127 for B cut change
		{0xea31,           7,     0,     0x0            },	// 20121127 for B cut change
		{0xea32,           5,     0,     0x0            },	// 20121127 for B cut change
		{0xea2f,           5,     1,     0x6            },	// 20121127 for B cut change
		{0xea30,           4,     0,     0x6            },	// 20121127 for B cut change
		{0xea01,           2,     0,     0x7            },	// 20121127 for B cut change
		{0xea2e,           2,     2,     0x0            },	// 20121127 for B cut change
		{0xea2e,           3,     3,     0x1            },	// 20121127 for B cut change
		{0xea2e,           7,     7,     0x0            },	// 20121127 for B cut change
		{0xea2f,           0,     0,     0x0            },	// 20121127 for B cut change
		{0xea2f,           6,     6,     0x1            },	// 20121127 for B cut change
		{0xe421,		   0,	  0,	 0x0			},
		{0xe421,		   0,	  0,	 0x1			},
		{0xe40d,		   0,	  0,	 0x1			},  //demod adc clk phase invesrse
		{0xe906,		   15,	  0,	 0x2000	        },//st0, outer_rst_n disable
		{0xe90d,		   15,	  0,	 0x1800	        },//st1, outer_rst_n enable
		{0xe437,			1,		1,		0x0			},  //reg_wrapper_dtmb_isdbt_sel   0:ISDBT  1:DTMB
		{0xe439,		   2,	  0,	 0x3			},  //e439[0]reg_isdbt_max_read_cmd_en=1;e439[2:1]reg_isdbt_max_read_cmd_sel=0x1
		{0xea35,		   4,	  0,	 0x8			},  //Burst Legnth by yukichen 20150820
 		//{0xea1a,           4,     1,     0x2            },  // 20180720 for PER issue, change reper blcok size  
        {0xea1b,           3,     0,     0x1            },  // 20180720 for PER issue, change reper blcok size
        {0xea1b,           7,     4,     0x1            },  // 20180720 for PER issue, change reper blcok size
	};
#endif

	BASE_INTERFACE_MODULE *pBaseInterface;
	RTK_R_ISDBT_EXTRA_MODULE *pExtra;

	int TsInterfaceMode;
	U32BITS IfFreqHz;
	int SpectrumMode;
#if IsdbtUseMerlin4InitValue
	int i;
	unsigned short RegStartAddr;
	unsigned char Msb;
	unsigned char Lsb;
	U32BITS WritingValue;
	INIT_ENTRY*   InitTable = (INIT_ENTRY*) InitTable_VerB;
	unsigned int  InitTableSize = (sizeof(InitTable_VerB) / sizeof(INIT_ENTRY));
#endif
	// Get base interface and demod extra module.
	pBaseInterface = pDemod->pBaseInterface;
	pExtra = &(pDemod->Extra.RtkrIsdbt);

	// Get TS interface mode, IF frequency in Hz, and spectrum mode.
	TsInterfaceMode = pDemod->TsInterfaceMode;
	IfFreqHz        = pDemod->IfFreqHz;
	SpectrumMode    = pDemod->SpectrumMode;

	/* 	//Demodulator ISDBT setting*/
	gstRTK_R_ISDBT_DvbConfig.IsdbtTunerInput.ui8TxIFAgcPolarity = RTK_A_ISDBT_IF_AGC_POLARITY_NORMAL;
	/*Demodulator Clock Setting           */
	gstRTK_R_ISDBT_DvbConfig.ui32DemodClkKHz = 27000; /*KHz, 27MHz*/
	if (pDemod->SetRegBits(pDemod, ISDBT_SOFT_RST_N, 0x0) != FUNCTION_SUCCESS)
		goto error_status;

	if (pExtra->ShareMemoryOfst & 0xfff) {
		RTK_DEMOD_ISDBT_INFO("[RTK_R_ISDBT] Invalid Share Memory Offset : share memory of ISDB-T demod should be 4KB alignment\n");
		//assert(0);
	} else {
		RTK_DEMOD_ISDBT_INFO("[RTK_R_ISDBT] Set Share Memory Offset = ("PT_H32BITS") \n", pExtra->ShareMemoryOfst);
		//pDemod->SetRegBits(pDemod, ISDBT_DDR_ADDR_OFFSET, pExtra->ShareMemoryOfst >> 3);
		rtd_outl(DTV_DEMOD_MISC_atb_wrapper_addr_offset_reg, pExtra->ShareMemoryOfst);

	}

	//Initial setting
	rtd_outl(RTK_DTV_DEMOD_ISDBT_OUTER_RSPER_POOL_0_reg, 0x0);       //Disable RSPER
	
	// Re-start demod FSM.
	if (pDemod->RestartFsm(pDemod) != FUNCTION_SUCCESS)
		goto error_status;

#if IsdbtUseMerlin4InitValue
	// Initialize demod registers according to the initializing table.
	for (i = 0; i < InitTableSize; i++) {
		// Get all information from each entry.
		RegStartAddr = InitTable[i].RegStartAddr;
		Msb          = InitTable[i].Msb;
		Lsb          = InitTable[i].Lsb;
		WritingValue = InitTable[i].WritingValue;

		// Set register mask bits.
		if (pDemod->SetRegMaskBits(pDemod, RegStartAddr, Msb, Lsb, WritingValue) != FUNCTION_SUCCESS)
			goto error_status;
	}
#endif

	// Set TS interface mode.
	if (realtek_r_isdbt_SetTsInterfaceMode(pDemod, TsInterfaceMode) != FUNCTION_SUCCESS)
		goto error_status;

	// Set IF frequency in Hz.
	if (realtek_r_isdbt_SetIfFreqHz(pDemod, IfFreqHz) != FUNCTION_SUCCESS)
		goto error_status;

	// Set spectrum mode.
	if (realtek_r_isdbt_SetSpectrumMode(pDemod, SpectrumMode) != FUNCTION_SUCCESS)
		goto error_status;
	RTK_DEMOD_ISDBT_INFO("[RTK_R_ISDBT] Merlin8 ATB CE setting \n");

	//Merlin8 ATB CE setting
	//Merlin8 reuse ATB CE setting from sky
	rtd_maskl(0xB816f068, 0xfffffffe, 0x00000001);
	rtd_maskl(0xB816f004, 0xdfffe000, 0x00000000);// from 0x20000000 to 0x0; IC need to transfer to signed
	rtd_maskl(0xB816f028, 0xf0ffffff, 0x01000000);
	rtd_outl(0xB8160120, 0x00000001);
	rtd_outl(0xB8162118, 0x00000019);					//[4:0]:dvbt_sfi_thr;SFI thd in sync from 0xa
	rtd_outl(0xB8162820, 0x00000017);					//[7:0]:rate_ratio[31:24],params_rate_ratio
	rtd_outl(0xB816281c, 0x000000a0);					//[7:0]:rate_ratio[23:16],params_rate_ratio
	rtd_outl(0xB8165c00, 0x000000ff);					//for continus/all error reset
	rtd_outl(0xB815a314, 0x00000000);					//[5]:sel_rtk_atb_ce_fix;[4]:sel_atb_rtk_ce_fixd;[0]:sel_atb_rtk_ce_auto,fix ATB CE
	rtd_outl(0xB8163bf0, 0x00000009);					//[0]:params_sym_idx_mod4_renew_en_isdbt;[3]:params_dvbt_fft_win_adj_ro_sel
	rtd_outl(0xB81621f8, 0x00000001);					//[0]:cfoint_iqswap_det_method
	rtd_outl(0xB815a2f8, 0x00000000);					//[7:0]:dt_cesm_phase_rot_on
	rtd_outl(0xB815a300, 0x00000011);					//[4]:post, [0]:preframe lock forecast enable
	//rtd_outl(0xB815a320, 0x00000090);					//[7:0]:reg_pilot_snr_th[7:0];for ATB CE, preframe lock pilot thd use this one*
	//rtd_outl(0xB815a324, 0x00000000);					//[0]:reg_pilot_snr_th[8];for ATB CE, preframe lock pilot thd use this one*
	rtd_outl(0xB81625ac, 0x000000fc);					//params_threshold_power_compare_7_0
	rtd_outl(0xB8162a0c, 0x00000024);					//params_interf_threshold
	rtd_outl(0xB8162ae4, 0x00000002);					//BW_MODE=6M
	rtd_outl(0xB816dcbc, 0x000000af);					//[7:0]:cfo_max_dvbt
	//rtd_outl(0xB8163b94, 0x000000ff);					//[7:0]:params_dcci_det_start_num
	//rtd_outl(0xB8163b98, 0x00000040);					//[7:0]:params_dcci_det_main_path_power_th_isdbt
	//rtd_outl(0xB8163bb8, 0x00000008);					//[4:0]:params_dcci_det_2_th_in_isdbt
	//rtd_outl(0xB8163bac, 0x000000ff);					//[7:0]:params_dcci_noise_var_th_isdbt
	//ATB CE
	rtd_outl(0xB8163418, 0x00000008);					//[7:0]:params_dpll_c1_converge_48k
	rtd_outl(0xB816341C, 0x00000010);					//[7:0]:params_dpll_c2_converge_48k
	rtd_outl(0xB8163440, 0x00000010);					//[7:0]:params_cfo_dpll_c1_converge_48k
	rtd_outl(0xB8163444, 0x00000040);					//[7:0]:params_cfo_dpll_c2_converge_48k
	//rtd_outl(0xB81600dc, 0x10);//option1: 0x181600dc=> 0x10(prefrm lock delay32EQ work)
	rtd_outl(0xB81600dc, 0x20);//option2: 0x181600dc=>0x20(frm lock delay 32EQ work)

	rtd_outl(0xB815a304, 0x00000000);					//forcast snr from 0x90
	rtd_outl(0xB815a308, 0x00000000);					//post snr from 0x90
	rtd_outl(0xB815a30c, 0x0000001E);					//data snr from 0x78
	rtd_outl(0xB815a310, 0x00000000);					//data snr thd
	// for PXP/FPGA alignment
	//rtd_outl(0xB8163960, 0x00000005);
	//rtd_outl(0xB8163964, 0x00000005);
	//rtd_outl(0xB8163b3c, 0x00000005);
	// for PXP/FPGA alignment
	//full-seg
	rtd_outl(0xB8162bec, 0x00000002);					//lp_filter_mode: 2 for ISDBT 6M
	rtd_outl(0xB8162824, 0x00000003);					//[7:0]:lpf_coef_reg00[7:0]
	rtd_outl(0xB8162828, 0x00000000);					//[3:0]:lpf_coef_reg00[11:8]
	rtd_outl(0xB816282c, 0x00000008);					//[7:0]:lpf_coef_reg01[7:0]
	rtd_outl(0xB8162830, 0x00000000);					//[3:0]:lpf_coef_reg01[11:8]
	rtd_outl(0xB8162834, 0x00000008);					//[7:0]:lpf_coef_reg02[7:0]
	rtd_outl(0xB8162838, 0x00000000);					//[3:0]:lpf_coef_reg02[11:8]
	rtd_outl(0xB816283c, 0x000000ff);					//[7:0]:lpf_coef_reg03[7:0]
	rtd_outl(0xB8162840, 0x0000000f);					//[3:0]:lpf_coef_reg03[11:8]
	rtd_outl(0xB8162844, 0x000000f3);					//[7:0]:lpf_coef_reg04[7:0]
	rtd_outl(0xB8162848, 0x0000000f);					//[3:0]:lpf_coef_reg04[11:8]
	rtd_outl(0xB816284c, 0x000000ef);					//[7:0]:lpf_coef_reg05[7:0]
	rtd_outl(0xB8162850, 0x0000000f);					//[3:0]:lpf_coef_reg05[11:8]
	rtd_outl(0xB8162854, 0x000000f9);					//[7:0]:lpf_coef_reg06[7:0]
	rtd_outl(0xB8162858, 0x0000000f);					//[3:0]:lpf_coef_reg06[11:8]
	rtd_outl(0xB816285c, 0x0000000f);					//[7:0]:lpf_coef_reg07[7:0]
	rtd_outl(0xB8162860, 0x00000000);					//[3:0]:lpf_coef_reg07[11:8]
	rtd_outl(0xB8162864, 0x0000001e);					//[7:0]:lpf_coef_reg08[7:0]
	rtd_outl(0xB8162868, 0x00000000);					//[3:0]:lpf_coef_reg08[11:8]
	rtd_outl(0xB816286c, 0x00000016);					//[7:0]:lpf_coef_reg09[7:0]
	rtd_outl(0xB8162870, 0x00000000);					//[3:0]:lpf_coef_reg09[11:8]
	rtd_outl(0xB8162874, 0x000000f6);					//[7:0]:lpf_coef_reg10[7:0]
	rtd_outl(0xB8162878, 0x0000000f);					//[3:0]:lpf_coef_reg10[11:8]
	rtd_outl(0xB816287c, 0x000000d5);					//[7:0]:lpf_coef_reg11[7:0]
	rtd_outl(0xB8162880, 0x0000000f);					//[3:0]:lpf_coef_reg11[11:8]
	rtd_outl(0xB8162884, 0x000000d2);					//[7:0]:lpf_coef_reg12[7:0]
	rtd_outl(0xB8162888, 0x0000000f);					//[3:0]:lpf_coef_reg12[11:8]
	rtd_outl(0xB816288c, 0x000000f9);					//[7:0]:lpf_coef_reg13[7:0]
	rtd_outl(0xB8162890, 0x0000000f);					//[3:0]:lpf_coef_reg13[11:8]
	rtd_outl(0xB8162894, 0x00000033);					//[7:0]:lpf_coef_reg14[7:0]
	rtd_outl(0xB8162898, 0x00000000);					//[3:0]:lpf_coef_reg14[11:8]
	rtd_outl(0xB816289c, 0x0000004e);					//[7:0]:lpf_coef_reg15[7:0]
	rtd_outl(0xB81628a0, 0x00000000);					//[3:0]:lpf_coef_reg15[11:8]
	rtd_outl(0xB81628a4, 0x00000028);					//[7:0]:lpf_coef_reg16[7:0]
	rtd_outl(0xB81628a8, 0x00000000);					//[3:0]:lpf_coef_reg16[11:8]
	rtd_outl(0xB81628ac, 0x000000d2);					//[7:0]:lpf_coef_reg17[7:0]
	rtd_outl(0xB81628b0, 0x0000000f);					//[3:0]:lpf_coef_reg17[11:8]
	rtd_outl(0xB81628b4, 0x0000008d);					//[7:0]:lpf_coef_reg18[7:0]
	rtd_outl(0xB81628b8, 0x0000000f);					//[3:0]:lpf_coef_reg18[11:8]
	rtd_outl(0xB81628bc, 0x0000009f);					//[7:0]:lpf_coef_reg19[7:0]
	rtd_outl(0xB81628c0, 0x0000000f);					//[3:0]:lpf_coef_reg19[11:8]
	rtd_outl(0xB81628c4, 0x00000012);					//[7:0]:lpf_coef_reg20[7:0]
	rtd_outl(0xB81628c8, 0x00000000);					//[3:0]:lpf_coef_reg20[11:8]
	rtd_outl(0xB81628cc, 0x00000099);					//[7:0]:lpf_coef_reg21[7:0]
	rtd_outl(0xB81628d0, 0x00000000);					//[3:0]:lpf_coef_reg21[11:8]
	rtd_outl(0xB81628d4, 0x000000bc);					//[7:0]:lpf_coef_reg22[7:0]
	rtd_outl(0xB81628d8, 0x00000000);					//[3:0]:lpf_coef_reg22[11:8]
	rtd_outl(0xB81628dc, 0x00000037);					//[7:0]:lpf_coef_reg23[7:0]
	rtd_outl(0xB81628e0, 0x00000000);					//[3:0]:lpf_coef_reg23[11:8]
	rtd_outl(0xB81628e4, 0x00000047);					//[7:0]:lpf_coef_reg24[7:0]
	rtd_outl(0xB81628e8, 0x0000000f);					//[3:0]:lpf_coef_reg24[11:8]
	rtd_outl(0xB81628ec, 0x000000a0);					//[7:0]:lpf_coef_reg25[7:0]
	rtd_outl(0xB81628f0, 0x0000000e);					//[3:0]:lpf_coef_reg25[11:8]
	rtd_outl(0xB81628f4, 0x0000000a);					//[7:0]:lpf_coef_reg26[7:0]
	rtd_outl(0xB81628f8, 0x0000000f);					//[3:0]:lpf_coef_reg26[11:8]
	rtd_outl(0xB81628fc, 0x000000cf);					//[7:0]:lpf_coef_reg27[7:0]
	rtd_outl(0xB8162900, 0x00000000);					//[3:0]:lpf_coef_reg27[11:8]
	rtd_outl(0xB8162904, 0x0000006d);					//[7:0]:lpf_coef_reg28[7:0]
	rtd_outl(0xB8162908, 0x00000003);					//[3:0]:lpf_coef_reg28[11:8]
	rtd_outl(0xB816290c, 0x000000c4);					//[7:0]:lpf_coef_reg29[7:0]
	rtd_outl(0xB8162910, 0x00000005);					//[3:0]:lpf_coef_reg29[11:8]
	rtd_outl(0xB8162914, 0x000000b4);					//[7:0]:lpf_coef_reg30[7:0]
	rtd_outl(0xB8162918, 0x00000006);					//[3:0]:lpf_coef_reg30[11:8]
	//page3
	rtd_outl(0xB816da40, 0x00000002);					//[3:0]:chan_change_peak_reg1_factor_dense_dvbt[11:8]
	rtd_outl(0xB816da48, 0x00000002);					//[3:0]:chan_change_peak_reg2_factor_dense_dvbt[11:8]
	rtd_outl(0xB816d9e0, 0x000000ca);					//[7:0]:params_dense_chan_det_num_th_out_8k_dvbt[7:0]
	rtd_outl(0xB816d9dc, 0x00000002);					//[3:0]:params_dense_chan_det_ratio_out_dvbt
	rtd_outl(0xB816d9d8, 0x00000004);					//[3:0]:params_dense_chan_symbol_in_dvbt
	rtd_outl(0xB816d9ec, 0x00000008);					//[3:0]:params_dense_chan_symbol_out_dvbt
	rtd_outl(0xB816da34, 0x0000002e);					//[7:0]:params_noise_change_var_reg1_factor_dense_dvbt
	rtd_outl(0xB816da38, 0x0000002e);					//[7:0]:params_noise_change_var_reg2_factor_dense_dvbt
	rtd_outl(0xB816da9c, 0x00000000);					//[3:0]:params_chan_change_peak_fo_factor_3_dvbt[11:8]
	rtd_outl(0xB816da98, 0x00000004);					//[7:0]:params_chan_change_peak_fo_factor_3_dvbt[7:0]
	rtd_outl(0xB816daa4, 0x000000ff);					//[7:0]:params_min_noise_counter_number_dvbt
	rtd_outl(0xB816d99c, 0x00000010);					//[7:0]:params_short_chan_det_factor_dvbt
	rtd_outl(0xB816d880, 0x00000030);					//[7:0]:params_short_chan_det_factor_dvbt_2
	rtd_outl(0xB816d9f0, 0x00000000);					//[7:0]:params_chan_state4_det_mode_dvbt
	rtd_outl(0xB816d9bc, 0x00000001);					//[0]:params_noise_var_on_carriers_filter_en_dvbt

	rtd_outl(0xB816600c, 0x00000001);					//[0]:tso_constclk_en
	rtd_outl(0xB816e624, 0x00000011);					//[0]:reg_isdbt_mr_en;[4]:reg_isdbt_mr_opt
	rtd_outl(0xB816c4d8, 0x00000000);					//[0]:cr_sfb_en_t

	rtd_outl(0xB816e644, 0x00000001);					//[0]:opt_isdbt_per_window;option for PER window (Add by Alger @20220825)
	rtd_outl(0xB816e64c, 0x00000001);					//[0]:opt_isdbt_depunc_zero;option for Deinterleaver output (Add by Alger @20220825)
	rtd_maskl(0xB8163b5c, 0xfffffffe, 0x0);				//[0]:params_noise_search_alpha_filter_en_isdbt;[7:4]:params_noise_search_alpha_isdbt;noise_search_alpha_filter_en_isdbt (Add by Sherlock @20220826)
	//End of Merlin8 ATB CE setting

/*
	//set ATB Dfe default value: 4_Merlin8_isdbt register file.tbl
	//rtd_outl(0x1815980c, 0x00000002);//z_agc=2
	//rtd_outl(0x18159c10, 0x00000026);//aagc_target_value
	//rtd_outl(RTK_DTV_DEMOD_ISDBT_GAIN_STEP_reg, 0x000000b5);//0x18159cac merlin8 removed
	//rtd_outl(0x1815a8fc, 0x00000015);//0x1815A8F0~0x1815A90C for DDR address setting
	//rtd_outl(0x1815a8f8, 0x00000040);
	//rtd_outl(0x1815a8f4, 0x00000000);
	//rtd_outl(0x1815a8f0, 0x00000000);
	//rtd_outl(0x1815a994, 0x00000000);//Dasable RSPER in initail setting

	//rtd_outl(0x18162a0c, 0x0000003c); //params_interf_threshold
	rtd_outl(ATB_DEMOD_RXFLT_PARAMS_INTERF_THRESHOLD_reg, 0x00000024);//params_interf_threshold
	rtd_outl(ATB_DEMOD_RXFLT_PARAMS_INV_FS_ADC_17_16_reg, 0x00000003);//params_inv_fs_adc
	rtd_outl(ATB_DEMOD_RXFLT_PARAMS_INV_FS_ADC_15_8_reg, 0x0000002f);
	rtd_outl(ATB_DEMOD_RXFLT_PARAMS_INV_FS_ADC_7_0_reg, 0x00000068);
	rtd_outl(ATB_DEMOD_RXFLT_FREQ_LIF_35_32_reg, 0x00000002);//params_freq_lif
	rtd_outl(ATB_DEMOD_RXFLT_FREQ_LIF_31_24_reg, 0x000000f6);
	rtd_outl(ATB_DEMOD_RXFLT_FREQ_LIF_23_16_reg, 0x00000084);
	rtd_outl(ATB_DEMOD_RXFLT_FREQ_LIF_15_8_reg, 0x000000bd);
	rtd_outl(ATB_DEMOD_RXFLT_FREQ_LIF_7_0_reg, 0x000000a1);
	rtd_outl(ATB_DEMOD_RXFLT_LP_FILTER_MODE_reg, 0x00000002);//params_lp_filter_mode
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG00_11_8_reg, 0x00000000);//params_lp_coef_reg00
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG00_7_0_reg, 0x00000003);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG01_11_8_reg, 0x0000000f);//params_lp_coef_reg01
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG01_7_0_reg, 0x000000fa);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG02_11_8_reg, 0x0000000f);//params_lp_coef_reg02
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG02_7_0_reg, 0x000000f5);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG03_11_8_reg, 0x0000000f);//params_lp_coef_reg03
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG03_7_0_reg, 0x000000f9);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG04_11_8_reg, 0x00000000);//params_lp_coef_reg04
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG04_7_0_reg, 0x00000005);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG05_11_8_reg, 0x00000000);//params_lp_coef_reg05
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG05_7_0_reg, 0x00000012);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG06_11_8_reg, 0x00000000);//params_lp_coef_reg06
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG06_7_0_reg, 0x00000015);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG07_11_8_reg, 0x00000000);//params_lp_coef_reg07
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG07_7_0_reg, 0x00000007);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG08_11_8_reg, 0x0000000f);//params_lp_coef_reg08
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG08_7_0_reg, 0x000000ee);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG09_11_8_reg, 0x0000000f);//params_lp_coef_reg09
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG09_7_0_reg, 0x000000dc);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG10_11_8_reg, 0x0000000f);//params_lp_coef_reg10
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG10_7_0_reg, 0x000000e0);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG11_11_8_reg, 0x0000000f);//params_lp_coef_reg11
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG11_7_0_reg, 0x000000fe);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG12_11_8_reg, 0x00000000);//params_lp_coef_reg12
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG12_7_0_reg, 0x00000028);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG13_11_8_reg, 0x00000000);//params_lp_coef_reg13
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG13_7_0_reg, 0x0000003f);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG14_11_8_reg, 0x00000000);//params_lp_coef_reg14
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG14_7_0_reg, 0x0000002d);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG15_11_8_reg, 0x0000000f);//params_lp_coef_reg15
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG15_7_0_reg, 0x000000f4);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG16_11_8_reg, 0x0000000f);//params_lp_coef_reg16
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG16_7_0_reg, 0x000000b4);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG17_11_8_reg, 0x0000000f);//params_lp_coef_reg17
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG17_7_0_reg, 0x0000009b);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG18_11_8_reg, 0x0000000f);//params_lp_coef_reg18
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG18_7_0_reg, 0x000000c7);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG19_11_8_reg, 0x00000000);//params_lp_coef_reg19
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG19_7_0_reg, 0x0000002b);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG20_11_8_reg, 0x00000000);//params_lp_coef_reg20
	rtd_outl(ATB_DEMOD_RXFLT_DC_ALPHA_9_8_reg, 0x0000008d);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG21_11_8_reg, 0x00000000);//params_lp_coef_reg21
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG21_7_0_reg, 0x000000a2);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG22_11_8_reg, 0x00000000);//params_lp_coef_reg22
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG22_7_0_reg, 0x00000043);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG23_11_8_reg, 0x0000000f);//params_lp_coef_reg23
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG23_7_0_reg, 0x0000008d);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG24_11_8_reg, 0x0000000e);//params_lp_coef_reg24
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG24_7_0_reg, 0x000000e8);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG25_11_8_reg, 0x0000000e);//params_lp_coef_reg25
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG25_7_0_reg, 0x000000d9);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG26_11_8_reg, 0x0000000f);//params_lp_coef_reg26
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG26_7_0_reg, 0x000000b7);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG27_11_8_reg, 0x00000001);//params_lp_coef_reg27
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG27_7_0_reg, 0x00000007);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG28_11_8_reg, 0x00000003);//params_lp_coef_reg28
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG28_7_0_reg, 0x00000090);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG29_11_8_reg, 0x00000005);//params_lp_coef_reg29
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG29_7_0_reg, 0x0000004a);
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG30_11_8_reg, 0x00000005);//params_lp_coef_reg30
	rtd_outl(ATB_DEMOD_RXFLT_LPF_COEF_REG30_7_0_reg, 0x000000f4);
	rtd_outl(ATB_DEMOD_RXFLT_RATE_RATIO_31_24_reg, 0x0000001a);//params_rate_ratio
	rtd_outl(ATB_DEMOD_RXFLT_RATE_RATIO_23_16_reg, 0x00000094);
	rtd_outl(ATB_DEMOD_RXFLT_RATE_RATIO_15_8_reg, 0x00000000);
	rtd_outl(ATB_DEMOD_RXFLT_RATE_RATIO_7_0_reg, 0x00000000);
	rtd_outl(ATB_DEMOD_RXFLT_PARAMS_BW_MODE_reg, 0x00000002);//params_bw_mode
	rtd_outl(ATB_DEMOD_RXFLT_PARAMS_FS_ADC_21_16_reg, 0x00000036);//params_fs_adc
	rtd_outl(ATB_DEMOD_RXFLT_PARAMS_FS_ADC_15_8_reg, 0x00000000);
	rtd_outl(ATB_DEMOD_RXFLT_PARAMS_FS_ADC_7_0_reg, 0x00000000);
	//params_interf_threshold 0x3c to 0x24
	rtd_outl(ATB_DEMOD_RXFLT2_PARAMS_THRESHOLD_POWER_COMPARE_7_0_reg, 0xFD);//power_compare_th(gain)
	//disable FastDagc
	rtd_outl(ATB_DEMOD_SYNC_T2_DISABLE_FAST_DAGC_reg, 0x01);
	//end of set ATB Dfe default value: : 4_Merlin8_isdbt register file.tbl
*/	
	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_SET_TS_INTERFACE_MODE

*/
int
realtek_r_isdbt_SetTsInterfaceMode(
	ISDBT_DEMOD_MODULE *pDemod,
	int TsInterfaceMode
)
{
	U32BITS Serial;
	//U32BITS CdivPh0;
	//U32BITS CdivPh1;



	// Determine SERIAL, CDIV_PH0, and CDIV_PH1 setting value according to TS interface mode.
	switch (TsInterfaceMode) {
	case TS_INTERFACE_PARALLEL:
		Serial = 0;
		//CdivPh0 = 6;
		//CdivPh1 = 6;
		break;
	case TS_INTERFACE_SERIAL:
		Serial = 1;
		//CdivPh0 = 0;
		//CdivPh1 = 0;
		break;

	default:
		goto error_status;
	}


	// Set SERIAL, CDIV_PH0, and CDIV_PH1 with determined value.
	rtd_maskl(0xB8166004, 0xfffffffe, Serial);
/*
rm
	if (pDemod->SetRegBits(pDemod, ISDBT_SERIAL, Serial) != FUNCTION_SUCCESS)
		goto error_status;

	if (pDemod->SetRegBits(pDemod, ISDBT_CDIV_PH0, CdivPh0) != FUNCTION_SUCCESS)
		goto error_status;

	if (pDemod->SetRegBits(pDemod, ISDBT_CDIV_PH1, CdivPh1) != FUNCTION_SUCCESS)
		goto error_status;
*/

	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}


int
realtek_r_isdbt_SetTsInterfaceClkPol(
	ISDBT_DEMOD_MODULE *pDemod,
	int TsInterfaceClkPol
)
{
	U32BITS TsClkPol;

	switch (TsInterfaceClkPol) {
	case TS_INTERFACE_CLK_POL_FALLING:
		TsClkPol = 0;
		break;
	case TS_INTERFACE_CLK_POL_RISING:
		TsClkPol = 1;
		break;

	default:
		goto error_status;
	}
	rtd_maskl(0xB8166008, 0xfffffffe, ~TsClkPol);//0x18166008[0]: 1: aligned with negative edge / 0: aligned with postive edge
/*rm
	if (pDemod->SetRegBits(pDemod, ISDBT_CKOUTPAR, TsClkPol) != FUNCTION_SUCCESS)
		goto error_status;
*/

	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}


/**

@see   ISDBT_DEMOD_FP_SET_IF_FREQ_HZ

*/
int
realtek_r_isdbt_SetIfFreqHz(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned int IfFreqHz
)
{
	int ui8Status = FUNCTION_SUCCESS;
	char  ui8FreqLifHigh = 0;
	unsigned int ui32FreqLifLow = 0;
	RTK_R_ISDBT_64Data  i64Data, i64Data1, i64Rst;
	unsigned int ui32TunerIFKHz = 5000;
	unsigned int ui32ParamsIFFreq = 0;
	unsigned int CrystalFreqHz = 0;


	ui32TunerIFKHz = (unsigned int)(IfFreqHz / 1000);
	// Get demod crystal frequency in Hz.
	pDemod->GetCrystalFreqHz(pDemod, &CrystalFreqHz);
	gstRTK_R_ISDBT_DvbConfig.ui32DemodClkKHz = (unsigned int)(CrystalFreqHz / 1000);


	/* 	ui32TunerIFKHz = 5000;*/
	/*freq Calculate*/
	if (ui32TunerIFKHz < gstRTK_R_ISDBT_DvbConfig.ui32DemodClkKHz) { /*use equation dbTunerIF/dbDemodClk*/
		RTK_R_ISDBT_64Mult(&i64Data, RTK_R_ISDBT_FixPow(2, 30), ui32TunerIFKHz * 64);
		i64Data1.ui32High = 0;
		i64Data1.ui32Low = gstRTK_R_ISDBT_DvbConfig.ui32DemodClkKHz;
		i64Rst  = RTK_R_ISDBT_64DivReturn64(i64Data, i64Data1);
	} else { /*use equation (dbTunerIF-dbDemodClk)/dbDemodClk*/
		RTK_R_ISDBT_64Mult(&i64Data, RTK_R_ISDBT_FixPow(2, 30), (ui32TunerIFKHz - gstRTK_R_ISDBT_DvbConfig.ui32DemodClkKHz) * 64);
		i64Data1.ui32High = 0;
		i64Data1.ui32Low = gstRTK_R_ISDBT_DvbConfig.ui32DemodClkKHz;
		i64Rst  = RTK_R_ISDBT_64DivReturn64(i64Data, i64Data1);
	}
	ui32FreqLifLow = i64Rst.ui32Low;/*64bit system */
	ui8FreqLifHigh = (unsigned char)(i64Rst.ui32High);

	/*start write register*/
	rtd_outl(ATB_DEMOD_RXFLT_FREQ_LIF_7_0_reg, (unsigned char)(ui32FreqLifLow));
	rtd_outl(ATB_DEMOD_RXFLT_FREQ_LIF_15_8_reg, (unsigned char)(ui32FreqLifLow >> 8));
	rtd_outl(ATB_DEMOD_RXFLT_FREQ_LIF_23_16_reg, (unsigned char)(ui32FreqLifLow >> 16));
	rtd_outl(ATB_DEMOD_RXFLT_FREQ_LIF_31_24_reg, (unsigned char)(ui32FreqLifLow >> 24));

	/*ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x0a,0x04,&ui8Tmp);*/
	rtd_outl(ATB_DEMOD_RXFLT_FREQ_LIF_35_32_reg, ui8FreqLifHigh);

	ui32ParamsIFFreq = (ui32TunerIFKHz * 32768 + 250) / 500; /*round(IF_MHz*Pow(2,16)) //Add 2014.3.24*/
	rtd_outl(ATB_DEMOD_RXFLT_PARAMS_IF_FREQ_7_0_reg, (unsigned char)(ui32ParamsIFFreq));
	rtd_outl(ATB_DEMOD_RXFLT_PARAMS_IF_FREQ_15_8_reg, (unsigned char)(ui32ParamsIFFreq >> 8));
	rtd_outl(ATB_DEMOD_RXFLT_PARAMS_IF_FREQ_21_16_reg, (unsigned char)(ui32ParamsIFFreq >> 16));
	return ui8Status;
}



/**

@see   ISDBT_DEMOD_FP_SET_SPECTRUM_MODE

*/
int
realtek_r_isdbt_SetSpectrumMode(
	ISDBT_DEMOD_MODULE *pDemod,
	int SpectrumMode
)
{
	//U32BITS EnSpInv;


/*
	// Determine EN_SP_INV setting value according to spectrum mode.
	switch (SpectrumMode) {
	case SPECTRUM_NORMAL:
		EnSpInv = 0;
		break;
	case SPECTRUM_INVERSE:
		EnSpInv = 1;
		break;

	default:
		goto error_status;
	}


	// Set EN_SP_INV with determined value.
	if (pDemod->SetRegBits(pDemod, ISDBT_EN_SP_INV, EnSpInv) != FUNCTION_SUCCESS)
		goto error_status;
*/
	RTK_DEMOD_ISDBT_INFO("[RTK_R_ISDBT] ISDBT auto detect spectrum inversion.\n");
	return FUNCTION_SUCCESS;

/*
error_status:
	return FUNCTION_ERROR;
*/
}





/**

@see   ISDBT_DEMOD_FP_SET_PARAMETERS

*/
int
realtek_r_isdbt_SetParameters(
	ISDBT_DEMOD_MODULE *pDemod,
	int BandwidthMode,
	unsigned char SegmentNum,
	unsigned char SubChannelNum
)
{
	/*
		// GROUP_0 setting entry only used in SetParameters()
		typedef struct {
			unsigned short RegStartAddr;
			unsigned char Msb;
			unsigned char Lsb;
			S32BITS WritingValue[RTK_R_ISDBT_GROUP_0_SELECTION_NUM];
		}
		SETTING_ENTRY_GROUP_0;

		// GROUP_1 setting entry only used in SetParameters()
		typedef struct {
			unsigned short RegStartAddr;
			unsigned char Msb;
			unsigned char Lsb;
			S32BITS WritingValue[RTK_R_ISDBT_GROUP_1_SELECTION_NUM];
		}
		SETTING_ENTRY_GROUP_1;

		// GROUP_2 setting entry only used in SetParameters()
		typedef struct {
			unsigned short RegStartAddr;
			unsigned char Msb;
			unsigned char Lsb;
			U32BITS WritingValue[RTK_R_ISDBT_GROUP_2_SELECTION_NUM];
		}
		SETTING_ENTRY_GROUP_2;

		// GROUP_3 setting entry only used in SetParameters()
		typedef struct {
			unsigned short RegStartAddr;
			unsigned char Msb;
			unsigned char Lsb;
			U32BITS WritingValue[RTK_R_ISDBT_GROUP_3_SELECTION_NUM];
		}
		SETTING_ENTRY_GROUP_3;

		// GROUP_4 setting entry only used in SetParameters()
		typedef struct {
			unsigned short RegStartAddr;
			unsigned char Msb;
			unsigned char Lsb;
			U32BITS WritingValue[RTK_R_ISDBT_GROUP_4_SELECTION_NUM];
		}
		SETTING_ENTRY_GROUP_4;

	*/
/*
	static const SETTING_ENTRY_GROUP_0 SettingTableGroup0[] = {
		// RegStartAddr,   Msb,   Lsb,   WritingValue
		//                               {SN=1,      SN=3,      SN=13,     SN=13      }   SN: Segment number
		//                               {BW=6,7,8,  BW=6,7,8,  BW=6,7,    BW=8       }   BW: Bandwidth in MHz
		{0xe735,           11,    0,     {0,         0,         0,         0,         }   },
		{0xe737,           11,    0,     {0,         0,         30,        30,        }   },
		{0xe739,           11,    0,     {0,         0,         -215,      -215,      }   },
		{0xe73b,           11,    0,     {1024,      1024,      1210,      1210,      }   },
		{0xe73d,           11,    0,     {5,         -16,       20,        -11,       }   },
		{0xe73f,           11,    0,     {26,        -48,       63,        25,        }   },
		{0xe741,           11,    0,     {74,        -69,       46,        92,        }   },
		{0xe743,           11,    0,     {159,       -24,       -112,      0,         }   },
		{0xe745,           11,    0,     {276,       141,       -251,      -256,      }   },
		{0xe747,           11,    0,     {408,       424,       16,        -153,      }   },
		{0xe749,           11,    0,     {524,       737,       785,       704,       }   },
		{0xe74b,           11,    0,     {593,       944,       1511,      1669,      }   },
	};
*/
/*
	static const SETTING_ENTRY_GROUP_1 SettingTableGroup1[] = {
		// RegStartAddr,   Msb,   Lsb,   WritingValue
		//                               {SN=1,   SN=3,   SN=13  }   SN: Segment number
		{0xe793,           11,    0,     {0,      -5,     -1     }   },
		{0xe795,           11,    0,     {0,      -3,     6      }   },
		{0xe797,           11,    0,     {0,      -1,     -18    }   },
		{0xe799,           11,    0,     {0,      1,      44     }   },
		{0xe79b,           11,    0,     {0,      2,      -92    }   },
		{0xe79d,           11,    0,     {0,      3,      184    }   },
		{0xe79f,           11,    0,     {0,      3,      -384   }   },
		{0xe7a1,           11,    0,     {1027,   1027,   1286   }   },
		{0xe7a3,           11,    0,     { -98,    -98,    -84    }   },
		{0xe7a5,           11,    0,     { -53,    -53,    60     }   },
		{0xe7a7,           11,    0,     {39,     39,     139    }   },
		{0xe7a9,           11,    0,     {172,    172,    -182   }   },
		{0xe7ab,           11,    0,     {328,    328,    277    }   },
		{0xe7ad,           11,    0,     {483,    483,    -103   }   },
		{0xe7af,           11,    0,     {608,    608,    -378   }   },
		{0xe7b1,           11,    0,     {677,    677,    2047   }   },
	};
*/
	static const SETTING_ENTRY_GROUP_2 SettingTableGroup2[] = {
		// RegStartAddr,   Msb,   Lsb,   WritingValue
		//                               {SN=1,   SN=3,   SN=13  }   SN: Segment number
		{0xe62a,           2,     0,     {2,      3,      0      }   },
		//{0xe7b6,           7,     0,     {28,     30,     64     }   },//rm 0xe7b6  = 18159ED8
		{0xe831,           12,    0,     {1,      7,      8191   }   },
	};
/*
	static const SETTING_ENTRY_GROUP_3 SettingTableGroup3[] = {
		// RegStartAddr,   Msb,   Lsb,   WritingValue
		//                               {SN=1,3,   SN=13     }   SN: Segment number
		{0xe792,           0,     0,     {1,        1,        }   },
		{0xe7b8,           2,     0,     {1,        3,        }   },
		{0xe7b9,           6,     0,     {4,        16,       }   },
		{0xe7ba,           6,     0,     {2,        8,        }   },
	};
*/
	static const SETTING_ENTRY_GROUP_4 SettingTableGroup4[] = {
		// RegStartAddr,   Msb,   Lsb,   WritingValue
		//                               {SN=13,   SN=Otherwise     }   SN: Segment number
		{
			0xe86c,           10,    0,     {
				2047,    1828, 1059, 9,    1479, 1958, 984,  1319,
				2047, 1947, 982,  930,  1339, 756,  531
			}
		},

		{
			0xe86e,           10,    0,     {
				2047,    1312, 1856, 105,  2013, 520,  1977, 1479,
				2047, 1339, 41,   1166, 1220, 871,  1668
			}
		},

		{
			0xe870,           10,    0,     {
				2047,    1111, 520,  195,  1977, 1059, 1479, 984,
				2047, 982,  1339, 531,  41,   208,  1166
			}
		},
	};


	int i = 0;

	unsigned short RegStartAddr = 0;
	unsigned char Msb = 0;
	unsigned char Lsb = 0;
	U32BITS WritingValue = 0;

	int SelectionIndex = 0;

	//U32BITS Private0 = 0;
	U32BITS Reg0 = 0;
	U32BITS Reg1 = 0;



	// Determine PRIVATE_0 and REG_0 value according to bandwidth mode.
	switch (BandwidthMode) {
	case ISDBT_BANDWIDTH_6MHZ:
		//Private0 = 6967296;
		Reg0 = 0;
		break;
	case ISDBT_BANDWIDTH_7MHZ:
		//Private0 = 5971968;
		Reg0 = 0;
		break;
	case ISDBT_BANDWIDTH_8MHZ:
		//Private0 = 5225472;
		Reg0 = 1;
		break;

	}

	// Set PRIVATE_0 and REG_0 with determined value.
	//if (pDemod->SetRegBits(pDemod, ISDBT_PRIVATE_0, Private0) != FUNCTION_SUCCESS)//rm 0xe74d = 18159D34~9d40
	//	goto error_status;
/*merlin8 rm
	if (pDemod->SetRegMaskBits(pDemod, 0xe7c4, 3, 3, Reg0) != FUNCTION_SUCCESS)
		goto error_status;
*/

	// Determine GROUP_0 selection index according to segment number and bandwidth mode.
	switch (SegmentNum) {
	case ISDBT_SEGMENT_NUM_1:
		SelectionIndex = 0;
		break;
	case ISDBT_SEGMENT_NUM_3:
		SelectionIndex = 1;
		break;

	case ISDBT_SEGMENT_NUM_13:

		switch (BandwidthMode) {
		case ISDBT_BANDWIDTH_6MHZ:
		case ISDBT_BANDWIDTH_7MHZ:
			SelectionIndex = 2;
			break;
		case ISDBT_BANDWIDTH_8MHZ:
			SelectionIndex = 3;
			break;

		default:
			goto error_status;
			break;
		}

		break;

	}
/*
	// Set GROUP_0 registers according to the GROUP_0 setting table and determined selection index.
	for (i = 0; i < (sizeof(SettingTableGroup0) / sizeof(SETTING_ENTRY_GROUP_0)); i++) {
		// Get all information from each entry.
		RegStartAddr = SettingTableGroup0[i].RegStartAddr;
		Msb          = SettingTableGroup0[i].Msb;
		Lsb          = SettingTableGroup0[i].Lsb;
		WritingValue = SignedIntToBin((long) SettingTableGroup0[i].WritingValue[SelectionIndex],
									  RTK_R_ISDBT_GROUP_0_BIT_NUM);

		// Set register mask bits.
		if (pDemod->SetRegMaskBits(pDemod, RegStartAddr, Msb, Lsb, WritingValue) != FUNCTION_SUCCESS)
			goto error_status;
	}
*/

	// Determine GROUP_1 selection index according to segment number.
	switch (SegmentNum) {
	case ISDBT_SEGMENT_NUM_1:
		SelectionIndex = 0;
		break;
	case ISDBT_SEGMENT_NUM_3:
		SelectionIndex = 1;
		break;
	case ISDBT_SEGMENT_NUM_13:
		SelectionIndex = 2;
		break;

	}
/*
	// Set GROUP_1 registers according to the GROUP_1 setting table and determined selection index.
	for (i = 0; i < (sizeof(SettingTableGroup1) / sizeof(SETTING_ENTRY_GROUP_1)); i++) {
		// Get all information from each entry.
		RegStartAddr = SettingTableGroup1[i].RegStartAddr;
		Msb          = SettingTableGroup1[i].Msb;
		Lsb          = SettingTableGroup1[i].Lsb;
		WritingValue = SignedIntToBin((long)SettingTableGroup1[i].WritingValue[SelectionIndex],
									  RTK_R_ISDBT_GROUP_1_BIT_NUM);

		// Set register mask bits.
		if (pDemod->SetRegMaskBits(pDemod, RegStartAddr, Msb, Lsb, WritingValue) != FUNCTION_SUCCESS)
			goto error_status;
	}
*/
	// Set GROUP_2 registers according to the GROUP_2 setting table and determined selection index.
	for (i = 0; i < (sizeof(SettingTableGroup2) / sizeof(SETTING_ENTRY_GROUP_2)); i++) {
		// Get all information from each entry.
		RegStartAddr = SettingTableGroup2[i].RegStartAddr;
		Msb          = SettingTableGroup2[i].Msb;
		Lsb          = SettingTableGroup2[i].Lsb;
		WritingValue = SettingTableGroup2[i].WritingValue[SelectionIndex];

		// Set register mask bits.
		if (pDemod->SetRegMaskBits(pDemod, RegStartAddr, Msb, Lsb, WritingValue) != FUNCTION_SUCCESS)
			goto error_status;
	}


	// Determine GROUP_3 selection index according to segment number.
	switch (SegmentNum) {
	case ISDBT_SEGMENT_NUM_1:
	case ISDBT_SEGMENT_NUM_3:
		SelectionIndex = 0;
		break;
	case ISDBT_SEGMENT_NUM_13:
		SelectionIndex = 1;
		break;

	}
/*
	// Set GROUP_3 registers according to the GROUP_3 setting table and determined selection index.
	for (i = 0; i < (sizeof(SettingTableGroup3) / sizeof(SETTING_ENTRY_GROUP_3)); i++) {
		// Get all information from each entry.
		RegStartAddr = SettingTableGroup3[i].RegStartAddr;
		Msb          = SettingTableGroup3[i].Msb;
		Lsb          = SettingTableGroup3[i].Lsb;
		WritingValue = SettingTableGroup3[i].WritingValue[SelectionIndex];

		// Set register mask bits.
		if (pDemod->SetRegMaskBits(pDemod, RegStartAddr, Msb, Lsb, WritingValue) != FUNCTION_SUCCESS)
			goto error_status;
	}
*/

	// Determine REG_1 value according to sub-channel number.
	Reg1 = (SegmentNum == 13) ? 22 : SubChannelNum;
/*merlin8 rm
	// Set REG_1 with determined value.
	if (pDemod->SetRegMaskBits(pDemod, 0xe7c7, 5, 0, Reg1) != FUNCTION_SUCCESS)
		goto error_status;
*/

	// Determine GROUP_4 selection index according to segment number.
	// Note: Original formula:   SelectionIndex = round(SubChannelNum / 3) % 14
	//       Adjusted formula:   SelectionIndex = floor((SubChannelNum * 2 + 3) / 6) % 14
	switch (SegmentNum) {
	case ISDBT_SEGMENT_NUM_1:
	case ISDBT_SEGMENT_NUM_3:
		SelectionIndex = ((SubChannelNum * 2 + 3) / 6) % 14;
		break;
	case ISDBT_SEGMENT_NUM_13:
		SelectionIndex = 0;
		break;

	}

	// Set GROUP_4 registers according to the GROUP_4 setting table and determined selection index.
	for (i = 0; i < (sizeof(SettingTableGroup4) / sizeof(SETTING_ENTRY_GROUP_4)); i++) {
		// Get all information from each entry.
		RegStartAddr = SettingTableGroup4[i].RegStartAddr;
		Msb = SettingTableGroup4[i].Msb;
		Lsb = SettingTableGroup4[i].Lsb;
		WritingValue = SettingTableGroup4[i].WritingValue[SelectionIndex];

		// Set register mask bits.
		if (pDemod->SetRegMaskBits(pDemod, RegStartAddr, Msb, Lsb, WritingValue) != FUNCTION_SUCCESS)
			goto error_status;
	}


	// Re-start demod FSM.
	if (pDemod->RestartFsm(pDemod) != FUNCTION_SUCCESS)
		goto error_status;


	// Store demod setting parameters.
	pDemod->BandwidthMode      = BandwidthMode;
	pDemod->IsBandwidthModeSet = YES;
	pDemod->SegmentNum         = SegmentNum;
	pDemod->IsSegmentNumSet    = YES;
	pDemod->SubChannelNum      = SubChannelNum;
	pDemod->IsSubChannelNumSet = YES;


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_IS_SIGNAL_LOCKED

*/
int
realtek_r_isdbt_IsSignalLocked(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	U32BITS Rsber0Valid, Rsber1Valid, Rsber2Valid;
	//U32BITS Rsber;
	U32BITS lock_tmp;


	lock_tmp = (rtd_inl(0xB816E4DC) & 0x7);//[0]: layerA, [1]:layerB, [2]:layerC
	Rsber0Valid = lock_tmp & 0x1;
	Rsber1Valid = (lock_tmp >> 1) & 0x1;
	Rsber2Valid = (lock_tmp >> 2) & 0x1;
/*	
	// Get RSBER_0_VALID, RSBER_1_VALID, RSBER_2_VALID.
	if (pDemod->GetRegBits(pDemod, ISDBT_RSBER_0_VALID, &Rsber0Valid) != FUNCTION_SUCCESS)
		goto error_status;

	if (pDemod->GetRegBits(pDemod, ISDBT_RSBER_1_VALID, &Rsber1Valid) != FUNCTION_SUCCESS)
		goto error_status;

	if (pDemod->GetRegBits(pDemod, ISDBT_RSBER_2_VALID, &Rsber2Valid) != FUNCTION_SUCCESS)
		goto error_status;

	// Get target RSBER with determined register bit name.
	if (pDemod->GetRegBits(pDemod, ISDBT_RSBER_0, &Rsber) != FUNCTION_SUCCESS)
		goto error_status;
*/
	// Determine answer according to RSBER_0_VALID, RSBER_1_VALID, RSBER_2_VALID values.
	if (((Rsber0Valid == 0x1) || (Rsber1Valid == 0x1) || (Rsber2Valid == 0x1)) /*&& (Rsber != RTK_R_ISDBT_PER_DEN_VALUE)*/) {
		*pAnswer = YES;
	} else {
		*pAnswer = NO;
	}


	return FUNCTION_SUCCESS;

/*
error_status:
	return FUNCTION_ERROR;
*/
}

int
realtek_r_isdbt_IsTrLocked(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	//return FUNCTION_ERROR;
	//need implement this function
	//printf("@@@@Yuki Debug=>tr_lock=%d\n", *pAnswer);
	return FUNCTION_SUCCESS;


//error_status_get_demod_registers:
	//return FUNCTION_ERROR;
}

int
realtek_r_isdbt_IsCrLocked(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{

//	return FUNCTION_ERROR;
	//need implement this function
	//printf("@@@@Yuki Debug=>cr_lock=%d\n", *pAnswer);
	return FUNCTION_SUCCESS;


//error_status_get_demod_registers:
//	return FUNCTION_ERROR;
}

int
realtek_r_isdbt_IsInnerLocked(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	U32BITS FrameLock;
	// Get TPS Lock
	if (pDemod->GetRegBits(pDemod, ISDBT_FRM_LOCK, &FrameLock) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	if (FrameLock == 1)
		*pAnswer = YES;
	else
		*pAnswer = NO;

	return FUNCTION_SUCCESS;


error_status_get_demod_registers:
	return FUNCTION_ERROR;
}


int
realtek_r_isdbt_IsNoSig(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	U32BITS   SigPresent = 0;
	U32BITS   PAPR_2K, PAPR_4K, PAPR_8K;

	rtd_outl(0xb8160434, 0x1);   //RTK_A_DVBTX_LatchOn();
	PAPR_2K = ((rtd_inl(0xb816DF54) & 0x1) << 8) + rtd_inl(0xb816DF50);
	PAPR_4K = ((rtd_inl(0xb816DF78) & 0x1) << 8) + rtd_inl(0xb816DF74);
	PAPR_8K = ((rtd_inl(0xb816DF60) & 0x1) << 8) + rtd_inl(0xb816DF5C);
	rtd_outl(0xb8160434, 0x0);   //RTK_A_DVBTX_LatchOff();

	if(PAPR_2K > PAPR_NO_SIGNAL_THD)
		SigPresent = 1;
	if(PAPR_4K > PAPR_NO_SIGNAL_THD)
		SigPresent = 1;
	if(PAPR_8K > PAPR_NO_SIGNAL_THD)
		SigPresent = 1;
	RTK_DEMOD_ISDBT_INFO("[RTK_R_ISDBT] PAPR_2K =%d, PAPR_4K = %d, PAPR_8K = %d, SigPresent =%d\n", PAPR_2K, PAPR_4K, PAPR_8K, SigPresent);
/*rm
	if (pDemod->GetRegBits(pDemod, ISDBT_SIG_PRESENT, &SigPresent) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;
*/
	if (SigPresent == 1)
		*pAnswer = NO;
	else
		*pAnswer = YES;

	return FUNCTION_SUCCESS;

/*rm
error_status_get_demod_registers:
	return FUNCTION_ERROR;
*/
}




/**

@see   ISDBT_DEMOD_FP_GET_SIGNAL_STRENGTH

*/
int
realtek_r_isdbt_GetSignalStrength(
	ISDBT_DEMOD_MODULE *pDemod,
	U32BITS *pSignalStrength
)
{
	int Answer;
	S32BITS IfAgc;



	// Get signal lock answer and IF AGC value.
	if (pDemod->IsSignalLocked(pDemod, &Answer) != FUNCTION_SUCCESS)
		goto error_status;

	if (pDemod->GetIfAgc(pDemod, &IfAgc) != FUNCTION_SUCCESS)
		goto error_status;


	//  Determine signal strength according to signal lock answer and IF AGC value.
	if (Answer != YES)
		*pSignalStrength = 0;
	else
		*pSignalStrength = 55 - IfAgc / 182;


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_GET_LAYER_SIGNAL_QUALITY

*/
int
realtek_r_isdbt_GetLayerSignalQuality(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	U32BITS *pLayerSignalQuality
)
{
	//int RegBitNameRsber;
	U32BITS Rsber;

	MPI MpiVar;
	S32BITS Var;



	// Determine target RSBER register bit name according to hierarchical layer index.
	switch (HierarchicalLayerIndex) {
	case ISDBT_HIERARCHICAL_LAYER_A:
		//RegBitNameRsber = ISDBT_RSBER_0;
		rtd_outl(0xB8160434, 0x1);   //RTK_A_DVBTX_LatchOn();
		Rsber = (rtd_inl(0xB816E5A4) << 16) + (rtd_inl(0xB816e48c)<<8) +rtd_inl(0xB816e488);
		//*pLayerBerDen = (rtd_inl(0xB816e484)<<8) + rtd_inl(0xB816e480);
		rtd_outl(0xB8160434, 0x0);   //RTK_A_DVBTX_LatchOff();
		break;
	case ISDBT_HIERARCHICAL_LAYER_B:
		//RegBitNameRsber = ISDBT_RSBER_1;
		rtd_outl(0xB8160434, 0x1);   //RTK_A_DVBTX_LatchOn();
		Rsber = (rtd_inl(0xb816E590) << 16) + (rtd_inl(0xb816e58c)<<8) +rtd_inl(0xb816e588);
		//*pLayerBerDen = (rtd_inl(0xB816e484)<<8) + rtd_inl(0xB816e480);
		rtd_outl(0xB8160434, 0x0);   //RTK_A_DVBTX_LatchOff();
		break;
	case ISDBT_HIERARCHICAL_LAYER_C:
		//RegBitNameRsber = ISDBT_RSBER_2;
		rtd_outl(0xB8160434, 0x1);   //RTK_A_DVBTX_LatchOn();
		Rsber = (rtd_inl(0xb816E608) << 16) + (rtd_inl(0xb816e604)<<8) +rtd_inl(0xb816e600);
		//*pLayerBerDen = (rtd_inl(0xB816e484)<<8) + rtd_inl(0xB816e480);
		rtd_outl(0xB8160434, 0x0);   //RTK_A_DVBTX_LatchOff();
		break;

	default:
		goto error_status;
	}
/*
	// Get target RSBER with determined register bit name.
	if (pDemod->GetRegBits(pDemod, RegBitNameRsber, &Rsber) != FUNCTION_SUCCESS)
		goto error_status;
*/

	// Determine signal quality according to target RSBER value.
	// Note: Map RSBER value 8192 ~ 128 to 10 ~ 100
	//       Original formula: SignalQuality = 205 - 15 * log2(RSBER)
	//       Adjusted formula: SignalQuality = ((205 << 5) - 15 * (log2(RSBER) << 5)) >> 5
	//       If RSBER > 8192, signal quality is 10.
	//       If RSBER < 128, signal quality is 100.
	if (Rsber > 8192) {
		*pLayerSignalQuality = 10;
	} else if (Rsber < 128) {
		*pLayerSignalQuality = 100;
	} else {
		MpiSetValue(&MpiVar, (long)Rsber);
		MpiLog2(&MpiVar, MpiVar, RTK_R_ISDBT_SIGNAL_QUALITY_FRAC_BIT_NUM);
		MpiGetValue(MpiVar, &Var);

		*pLayerSignalQuality = ((205 << RTK_R_ISDBT_SIGNAL_QUALITY_FRAC_BIT_NUM) - 15 * Var) >>
							   RTK_R_ISDBT_SIGNAL_QUALITY_FRAC_BIT_NUM;
	}


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_GET_LAYER_BER

*/
int
realtek_r_isdbt_GetLayerBer(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	U32BITS *pLayerBerNum,
	U32BITS *pLayerBerDen
)
{
	//int RegBitNameVberValid, RegBitNameVber;
	//U32BITS VberValid, Vber;



	// Determine target VBER_VALID and VBER register bit name according to hierarchical layer index.
	switch (HierarchicalLayerIndex) {
	case ISDBT_HIERARCHICAL_LAYER_A:
		rtd_outl(0xB8160434, 0x1);   //RTK_A_DVBTX_LatchOn();
		*pLayerBerNum = (rtd_inl(0xB816E5A4) << 16) + (rtd_inl(0xB816e48c)<<8) +rtd_inl(0xB816e488);
		*pLayerBerDen = (rtd_inl(0xB816e484)<<8) + rtd_inl(0xB816e480);
		rtd_outl(0xB8160434, 0x0);   //RTK_A_DVBTX_LatchOff();
/*
		RegBitNameVberValid = ISDBT_VBER_0_VALID;
		RegBitNameVber = ISDBT_VBER_0;
*/
		break;

	case ISDBT_HIERARCHICAL_LAYER_B:
		rtd_outl(0xB8160434, 0x1);   //RTK_A_DVBTX_LatchOn();
		*pLayerBerNum = (rtd_inl(0xb816E590) << 16) + (rtd_inl(0xb816e58c)<<8) +rtd_inl(0xb816e588);
		*pLayerBerDen = (rtd_inl(0xB816e484)<<8) + rtd_inl(0xB816e480);
		rtd_outl(0xB8160434, 0x0);   //RTK_A_DVBTX_LatchOff();
/*
		RegBitNameVberValid = ISDBT_VBER_1_VALID;
		RegBitNameVber = ISDBT_VBER_1;
*/
		break;

	case ISDBT_HIERARCHICAL_LAYER_C:
		rtd_outl(0xB8160434, 0x1);   //RTK_A_DVBTX_LatchOn();
		*pLayerBerNum = (rtd_inl(0xb816E608) << 16) + (rtd_inl(0xb816e604)<<8) +rtd_inl(0xb816e600);
		*pLayerBerDen = (rtd_inl(0xB816e484)<<8) + rtd_inl(0xB816e480);
		rtd_outl(0xB8160434, 0x0);   //RTK_A_DVBTX_LatchOff();
/*
		RegBitNameVberValid = ISDBT_VBER_2_VALID;
		RegBitNameVber = ISDBT_VBER_2;
*/
		break;

	default:
		goto error_status;
	}
/*
	// Get target VBER_VALID with determined register bit name.
	if (pDemod->GetRegBits(pDemod, RegBitNameVberValid, &VberValid) != FUNCTION_SUCCESS)
		goto error_status;

	// Get target VBER with determined register bit name.
	if (pDemod->GetRegBits(pDemod, RegBitNameVber, &Vber) != FUNCTION_SUCCESS)
		goto error_status;


	// Set BER numerator according to VBER_VALID and VBER.
	*pLayerBerNum = VberValid ? Vber : RTK_R_ISDBT_BER_NUM_VALUE_ERROR;

	// Set BER denominator.
	*pLayerBerDen = RTK_R_ISDBT_BER_DEN_VALUE;
*/

	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_GET_LAYER_PER

*/
int
realtek_r_isdbt_GetLayerPer(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	U32BITS *pLayerPerNum,
	U32BITS *pLayerPerDen
)
{
	//int RegBitNameRsber;
	U32BITS Rsber;



	// Determine target RSBER register bit name according to hierarchical layer index.
	switch (HierarchicalLayerIndex) {
	case ISDBT_HIERARCHICAL_LAYER_A:
		//RegBitNameRsber = ISDBT_RSBER_0;
		rtd_outl(0xb8160434, 0x1);   //RTK_A_DVBTX_LatchOn();
		Rsber = ((rtd_inl(0xB8166254) & 0xf) << 16) + ((rtd_inl(0xB8166250) & 0xff) << 8) + (rtd_inl(0xB816624c) &0xff);// layerA
		*pLayerPerDen = ((rtd_inl(0xB8166248) & 0xf) << 16) + ((rtd_inl(0xB8166244) & 0xff) << 8) + (rtd_inl(0xB8166240) &0xff);// layerA
		rtd_outl(0xb8160434, 0x0);   //RTK_A_DVBTX_LatchOff();
		break;
	case ISDBT_HIERARCHICAL_LAYER_B:
		//RegBitNameRsber = ISDBT_RSBER_1;
		rtd_outl(0xb8160434, 0x1);   //RTK_A_DVBTX_LatchOn();
		Rsber = ((rtd_inl(0xB81662B0) & 0xf) << 16) + ((rtd_inl(0xB81662AC) & 0xff) << 8) + (rtd_inl(0xB81662A8) &0xff);// layerB
		*pLayerPerDen = ((rtd_inl(0xB81662A4) & 0xf) << 16) + ((rtd_inl(0xB81662A0) & 0xff) << 8) + (rtd_inl(0xB816629C) &0xff);// layerB
		rtd_outl(0xb8160434, 0x0);   //RTK_A_DVBTX_LatchOff();
		break;
	case ISDBT_HIERARCHICAL_LAYER_C:
		//RegBitNameRsber = ISDBT_RSBER_2;
		rtd_outl(0xb8160434, 0x1);   //RTK_A_DVBTX_LatchOn();
		Rsber = ((rtd_inl(0xb81662C8) & 0xf) << 16) + ((rtd_inl(0xb81662C4) & 0xff) << 8) + (rtd_inl(0xb81662C0) &0xff);// layerC
		*pLayerPerDen = ((rtd_inl(0xb81662BC) & 0xf) << 16) + ((rtd_inl(0xb81662B8) & 0xff) << 8) + (rtd_inl(0xb81662B4) &0xff);// layerC
		rtd_outl(0xb8160434, 0x0);   //RTK_A_DVBTX_LatchOff();
		break;

	default:
		goto error_status;
	}
/*rm rtk
	// Get target RSBER with determined register bit name.
	if (pDemod->GetRegBits(pDemod, RegBitNameRsber, &Rsber) != FUNCTION_SUCCESS)
		goto error_status;
*/

	// Set PER numerator according to RSBER.
	*pLayerPerNum = Rsber;
/*
	// Set PER denominator.
	*pLayerPerDen = RTK_R_ISDBT_PER_DEN_VALUE;
*/

	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_GET_SNR_DB

*/
int
realtek_r_isdbt_GetSnrDb(
	ISDBT_DEMOD_MODULE *pDemod,
	S32BITS *pSnrDbNum,
	S32BITS *pSnrDbDen
)
{
	U32BITS EstSnr;
	U32BITS ui32RoNoiseVar = 0, ui32RoSignalVar = 0, u32tmp = 0;
	RTK_R_ISDBT_64Data i64Data, i64Data1;


	if((rtd_inl(0xB815B638) & 0x01) == 0x1){//RTK CE
	// Get EST_SNR.
	if (pDemod->GetRegBits(pDemod, ISDBT_EST_SNR, &EstSnr) != FUNCTION_SUCCESS)
		goto error_status;


	// Set SNR dB numerator according to EST_SNR.
	*pSnrDbNum = BinToSignedInt(EstSnr, RTK_R_ISDBT_EST_SNR_BIT_NUM);

	// Set SNR dB denominator.
	*pSnrDbDen = RTK_R_ISDBT_SNR_DB_DEN_VALUE;
	} else {//ATB CE
		/*Latch On*/
		rtd_outl(0xB8160434, 0x1);   //RTK_A_DVBTX_LatchOn();
		ui32RoNoiseVar = (rtd_inl(0xB816478c) << 16) + (rtd_inl(0xB8164788) << 8) + rtd_inl(0xB8164784);

		ui32RoSignalVar = (rtd_inl(0xB8164774) << 16) | (rtd_inl(0xB8164770) << 8) | rtd_inl(0xB816476c);
		ui32RoSignalVar = ui32RoSignalVar * 10;
		/*Latch Off*/
		rtd_outl(0xB8160434, 0x0);   //RTK_A_DVBTX_LatchOff();
		


		if (ui32RoNoiseVar == 0 || ui32RoSignalVar == 0) {
			*pSnrDbNum = 0;
			*pSnrDbDen = RTK_R_ISDBT_SNR_DB_DEN_VALUE;
			return FUNCTION_SUCCESS;
		}
		i64Data1.ui32High = 0;
		i64Data1.ui32Low = ui32RoNoiseVar;
		RTK_R_ISDBT_64Mult(&i64Data, ui32RoSignalVar, 4);
		/*RTK_A_DVBTX_64Mult(&i64Data,ui32RoSignalVar,4);*/
		u32tmp = RTK_R_ISDBT_Log10ConvertdBx10(RTK_R_ISDBT_64Div(i64Data, i64Data1));
		if (u32tmp > 100) {
			u32tmp =  u32tmp - 100;
		} else {
			u32tmp = 0;
		}
		u32tmp = u32tmp * 4 / 10;

		*pSnrDbNum = (S32BITS)u32tmp;//(unsigned int)SNR *4 
		*pSnrDbDen = RTK_R_ISDBT_SNR_DB_DEN_VALUE;
	}

	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_GET_RF_AGC

*/
int
realtek_a_isdbt_SetAgcPol(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned char agcpola
)
{
// agc gain, 0: postive, 1:negative

	gstRTK_R_ISDBT_DvbConfig.IsdbtTunerInput.ui8TxIFAgcPolarity =  agcpola ? RTK_A_ISDBT_IF_AGC_POLARITY_INVERTED : RTK_A_ISDBT_IF_AGC_POLARITY_NORMAL;


	if (gstRTK_R_ISDBT_DvbConfig.IsdbtTunerInput.ui8TxIFAgcPolarity == RTK_A_ISDBT_IF_AGC_POLARITY_INVERTED) {
		rtd_maskl(0xB8160080, 0xFFFFFFFD, 0x02);
	} else {
		rtd_maskl(0xB8160080, 0xFFFFFFFD, 0x00);
	}

	return FUNCTION_SUCCESS;

}

/**

@see   ISDBT_DEMOD_FP_GET_RF_AGC

*/
int
realtek_r_isdbt_GetRfAgc(
	ISDBT_DEMOD_MODULE *pDemod,
	S32BITS *pRfAgc
)
{
	return FUNCTION_SUCCESS;
/* wo use
	U32BITS RfAgcVal;



	// Get RF_AGC_VAL.
	if (pDemod->GetRegBits(pDemod, ISDBT_RF_AGC_VAL, &RfAgcVal) != FUNCTION_SUCCESS)
		goto error_status;


	// Store RF_AGC_VAL to RfAgc.
	// Note: The range of RF_AGC_VAL is -8192 ~ 8191.
	*pRfAgc = (long)BinToSignedInt(RfAgcVal, RTK_R_ISDBT_RF_AGC_REG_BIT_NUM);


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
*/
}





/**

@see   ISDBT_DEMOD_FP_GET_IF_AGC

*/
int
realtek_r_isdbt_GetIfAgc(
	ISDBT_DEMOD_MODULE *pDemod,
	S32BITS *pIfAgc
)
{
	//U32BITS IfAgcVal;


	unsigned char ui8Pwm0 = 0, ui8Pwm1 = 0, u8AGCPolarity;
	unsigned int u16AGCValue = 0;
	rtd_outl(0xB8160434, 0x1);   //RTK_A_DVBTX_LatchOn();
	u8AGCPolarity = rtd_inl(0xB8000080);
	ui8Pwm0 = rtd_inl(0xB81640a0);
	ui8Pwm1 = rtd_inl(0xB81640a4);
	rtd_outl(0xB8160434, 0x0);   //RTK_A_DVBTX_LatchOff();

	u16AGCValue = ((ui8Pwm1 & 0x03) << 8) | ui8Pwm0;
	if ((u8AGCPolarity & 0x02) == 0x02) { //AGV inverted
		*pIfAgc = ((1024 - u16AGCValue) * 100) / 1024;
	} else {
		*pIfAgc  = (u16AGCValue * 100) / 1024;
	}
/*rm 
	// Get IF_AGC_VAL.
	if (pDemod->GetRegBits(pDemod, ISDBT_IF_AGC_VAL, &IfAgcVal) != FUNCTION_SUCCESS)
		goto error_status;
*/
/*
	// Store IF_AGC_VAL to IfAgc.
	// Note: The range of IF_AGC_VAL is -8192 ~ 8191.
	*pIfAgc = (long)BinToSignedInt(IfAgcVal, RTK_R_ISDBT_IF_AGC_REG_BIT_NUM);
*/

	return FUNCTION_SUCCESS;

/*
error_status:
	return FUNCTION_ERROR;
*/
}





/**

@see   ISDBT_DEMOD_FP_GET_DI_AGC

*/
int
realtek_r_isdbt_GetDiAgc(
	ISDBT_DEMOD_MODULE *pDemod,
	S32BITS *pDiAgc
)
{
	return FUNCTION_SUCCESS;

/* wo use
	U32BITS GainOutR;



	// Get GAIN_OUT_R.
	if (pDemod->GetRegBits(pDemod, ISDBT_GAIN_OUT_R, &GainOutR) != FUNCTION_SUCCESS)
		goto error_status;

	// Store GAIN_OUT_R to DiAgc.
	// Note: The range of GAIN_OUT_R is 0 ~ 4095.
	*pDiAgc = (long)GainOutR;


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
*/
}





/**

@see   ISDBT_DEMOD_FP_GET_TR_OFFSET_PPM

*/
int
realtek_r_isdbt_GetTrOffsetPpm(
	ISDBT_DEMOD_MODULE *pDemod,
	S32BITS *pTrOffsetPpm
)
{
	U32BITS Private0 = 0;
	U32BITS SfoOutBinary = 0;
	S32BITS SfoOutInt = 0;

	MPI MpiPrivate0, MpiSfoOutInt;
	MPI MpiVar, MpiConst, MpiNone;


	memset(&MpiPrivate0, 0 , sizeof(MpiPrivate0));
	memset(&MpiSfoOutInt, 0 , sizeof(MpiSfoOutInt));
	memset(&MpiVar, 0 , sizeof(MpiVar));
	memset(&MpiNone, 0 , sizeof(MpiNone));
	memset(&MpiConst, 0 , sizeof(MpiConst));


	// Check if demod bandwidth mode is set.
	// Note: PRIVATE_0 is related to bandwidth mode setting.
	if (pDemod->IsBandwidthModeSet != YES)
		goto error_status;

	// Get PRIVATE_0.
	//if (pDemod->GetRegBits(pDemod, ISDBT_PRIVATE_0, &Private0) != FUNCTION_SUCCESS)////rm 0xe74d = 18159D34~9d40
	//	goto error_status;

	// Get SFO_OUT binary value and convert it to signed integer.
	if (pDemod->GetRegBits(pDemod, ISDBT_SFO_OUT, &SfoOutBinary) != FUNCTION_SUCCESS)
		goto error_status;

	SfoOutInt = BinToSignedInt(SfoOutBinary, RTK_R_ISDBT_SFO_OUT_BIT_NUM);


	// Get TR offset in ppm.
	// Note: Original formula:   TrOffsetPpm = (SFO_OUT * 0.5 * 1000000) / PRIVATE_0
	//       Adjusted formula:   TrOffsetPpm = (SFO_OUT * 500000) / PRIVATE_0
	MpiSetValue(&MpiPrivate0, (long)Private0);
	MpiSetValue(&MpiSfoOutInt, (long)SfoOutInt);
	MpiSetValue(&MpiConst, 500000);

	MpiMul(&MpiVar, MpiSfoOutInt, MpiConst);
	MpiDiv(&MpiVar, &MpiNone, MpiVar, MpiPrivate0);

	MpiGetValue(MpiVar, (S32BITS*)pTrOffsetPpm);


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_GET_CR_OFFSET_HZ

*/
int
realtek_r_isdbt_GetCrOffsetHz(
	ISDBT_DEMOD_MODULE *pDemod,
	S32BITS *pCrOffsetHz
)
{
	U32BITS CrystalFreqHz = 0;
	U32BITS AdcFreqHz = 0;

	U32BITS Private0 = 0;
	U32BITS CfoEstBinary = 0;
	S32BITS CfoEstInt = 0;

	MPI MpiAdcFreqHz, MpiPrivate0, MpiCfoEstInt;
	MPI MpiVar0, MpiVar1, MpiNone;

	memset(&MpiAdcFreqHz, 0 , sizeof(MpiAdcFreqHz));
	memset(&MpiPrivate0, 0 , sizeof(MpiPrivate0));
	memset(&MpiCfoEstInt, 0 , sizeof(MpiCfoEstInt));
	memset(&MpiVar0, 0 , sizeof(MpiVar0));
	memset(&MpiVar1, 0 , sizeof(MpiVar1));
	memset(&MpiNone, 0 , sizeof(MpiNone));


	// Get demod crystal frequency in Hz.
	pDemod->GetCrystalFreqHz(pDemod, &CrystalFreqHz);

	// Calculate demod ADC frequency in Hz according to demod crystal frequency in Hz.
	AdcFreqHz = CrystalFreqHz * RTK_R_ISDBT_ADC_TO_CRYSTAL_FREQ_RATIO;


	// Check if demod bandwidth mode is set.
	// Note: PRIVATE_0 is related to bandwidth mode setting.
	if (pDemod->IsBandwidthModeSet != YES)
		goto error_status;

	// Get PRIVATE_0.
	//if (pDemod->GetRegBits(pDemod, ISDBT_PRIVATE_0, &Private0) != FUNCTION_SUCCESS)//
	//	goto error_status;

	// Get CFO_EST binary value and convert it to signed integer.
	if (pDemod->GetRegBits(pDemod, ISDBT_CFO_EST, &CfoEstBinary) != FUNCTION_SUCCESS)
		goto error_status;

	CfoEstInt = BinToSignedInt(CfoEstBinary, RTK_R_ISDBT_CFO_EST_BIT_NUM);


	// Get Cr offset in Hz.
	// Note: Original formula:   CrOffsetHz = (CFO_EST * AdcFreqHz) / (Private0 * 64)
	//       Adjusted formula:   CrOffsetHz = (CFO_EST * AdcFreqHz) / (Private0 << 6)
	MpiSetValue(&MpiAdcFreqHz, (long)AdcFreqHz);
	MpiSetValue(&MpiPrivate0, (long)Private0);
	MpiSetValue(&MpiCfoEstInt, (long)CfoEstInt);

	MpiMul(&MpiVar0, MpiCfoEstInt, MpiAdcFreqHz);
	MpiLeftShift(&MpiVar1, MpiPrivate0, 6);
	MpiDiv(&MpiVar0, &MpiNone, MpiVar0, MpiVar1);

	MpiGetValue(MpiVar0, (S32BITS*)pCrOffsetHz);


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_GET_LAYER_CONSTELLATION

*/
int
realtek_r_isdbt_GetLayerConstellation(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	int *pLayerConstellation
)
{
	int RegBitNameRxQam;
	U32BITS RxQam;



	// Determine target RX_QAM register bit name according to hierarchical layer index.
	switch (HierarchicalLayerIndex) {
	case ISDBT_HIERARCHICAL_LAYER_A:
		RegBitNameRxQam = ISDBT_RX_QAM0;
		break;
	case ISDBT_HIERARCHICAL_LAYER_B:
		RegBitNameRxQam = ISDBT_RX_QAM1;
		break;
	case ISDBT_HIERARCHICAL_LAYER_C:
		RegBitNameRxQam = ISDBT_RX_QAM2;
		break;

	default:
		goto error_status;
	}

	// Get target RX_QAM with determined register bit name.
	if (pDemod->GetRegBits(pDemod, RegBitNameRxQam, &RxQam) != FUNCTION_SUCCESS)
		goto error_status;


	// Determine constellation mode according to target RX_QAM.
	switch (RxQam) {
	case 0:
		*pLayerConstellation = ISDBT_CONSTELLATION_DQPSK;
		break;
	case 1:
		*pLayerConstellation = ISDBT_CONSTELLATION_QPSK;
		break;
	case 2:
		*pLayerConstellation = ISDBT_CONSTELLATION_16QAM;
		break;
	case 3:
		*pLayerConstellation = ISDBT_CONSTELLATION_64QAM;
		break;

	default:
		goto error_status;
	}


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_GET_LAYER_CODE_RATE

*/
int
realtek_r_isdbt_GetLayerCodeRate(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	int *pLayerCodeRate
)
{
	int RegBitNameRxCrate;
	U32BITS RxCrate;



	// Determine target RX_CRATE register bit name according to hierarchical layer index.
	switch (HierarchicalLayerIndex) {
	case ISDBT_HIERARCHICAL_LAYER_A:
		RegBitNameRxCrate = ISDBT_RX_CRATE0;
		break;
	case ISDBT_HIERARCHICAL_LAYER_B:
		RegBitNameRxCrate = ISDBT_RX_CRATE1;
		break;
	case ISDBT_HIERARCHICAL_LAYER_C:
		RegBitNameRxCrate = ISDBT_RX_CRATE2;
		break;

	default:
		goto error_status;
	}

	// Get target RX_CRATE with determined register bit name.
	if (pDemod->GetRegBits(pDemod, RegBitNameRxCrate, &RxCrate) != FUNCTION_SUCCESS)
		goto error_status;


	// Determine code rate mode according to target RX_CRATE.
	switch (RxCrate) {
	case 0:
		*pLayerCodeRate = ISDBT_CODE_RATE_1_OVER_2;
		break;
	case 1:
		*pLayerCodeRate = ISDBT_CODE_RATE_2_OVER_3;
		break;
	case 2:
		*pLayerCodeRate = ISDBT_CODE_RATE_3_OVER_4;
		break;
	case 3:
		*pLayerCodeRate = ISDBT_CODE_RATE_5_OVER_6;
		break;
	case 4:
		*pLayerCodeRate = ISDBT_CODE_RATE_7_OVER_8;
		break;

	default:
		goto error_status;
	}


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_GET_LAYER_INTERLEAVING_LEN

*/
int
realtek_r_isdbt_GetLayerInterleavingLen(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	int *pLayerInterleavingLen
)
{
	int FftMode;
	int RegBitNameRxIntlv;
	U32BITS RxIntlv;



	// Get FFT mode.
	if (pDemod->GetFftMode(pDemod, &FftMode) != FUNCTION_SUCCESS)
		goto error_status;

	// Determine target RX_INTLV register bit name according to hierarchical layer index.
	switch (HierarchicalLayerIndex) {
	case ISDBT_HIERARCHICAL_LAYER_A:
		RegBitNameRxIntlv = ISDBT_RX_INTLV0;
		break;
	case ISDBT_HIERARCHICAL_LAYER_B:
		RegBitNameRxIntlv = ISDBT_RX_INTLV1;
		break;
	case ISDBT_HIERARCHICAL_LAYER_C:
		RegBitNameRxIntlv = ISDBT_RX_INTLV2;
		break;

	default:
		goto error_status;
	}

	// Get target RX_INTLV with determined register bit name.
	if (pDemod->GetRegBits(pDemod, RegBitNameRxIntlv, &RxIntlv) != FUNCTION_SUCCESS)
		goto error_status;


	// Determine interleaving length mode according to FFT mode and target RX_INTLV.
	switch (FftMode) {
	case ISDBT_FFT_MODE_2K:
		switch (RxIntlv) {
		case 0:
			*pLayerInterleavingLen = ISDBT_INTERLEAVING_LEN_0;
			break;
		case 1:
			*pLayerInterleavingLen = ISDBT_INTERLEAVING_LEN_4;
			break;
		case 2:
			*pLayerInterleavingLen = ISDBT_INTERLEAVING_LEN_8;
			break;
		case 3:
			*pLayerInterleavingLen = ISDBT_INTERLEAVING_LEN_16;
			break;

		default:
			goto error_status;
		}
		break;

	case ISDBT_FFT_MODE_4K:
		switch (RxIntlv) {
		case 0:
			*pLayerInterleavingLen = ISDBT_INTERLEAVING_LEN_0;
			break;
		case 1:
			*pLayerInterleavingLen = ISDBT_INTERLEAVING_LEN_2;
			break;
		case 2:
			*pLayerInterleavingLen = ISDBT_INTERLEAVING_LEN_4;
			break;
		case 3:
			*pLayerInterleavingLen = ISDBT_INTERLEAVING_LEN_8;
			break;

		default:
			goto error_status;
		}
		break;

	case ISDBT_FFT_MODE_8K:
		switch (RxIntlv) {
		case 0:
			*pLayerInterleavingLen = ISDBT_INTERLEAVING_LEN_0;
			break;
		case 1:
			*pLayerInterleavingLen = ISDBT_INTERLEAVING_LEN_1;
			break;
		case 2:
			*pLayerInterleavingLen = ISDBT_INTERLEAVING_LEN_2;
			break;
		case 3:
			*pLayerInterleavingLen = ISDBT_INTERLEAVING_LEN_4;
			break;

		default:
			goto error_status;
		}
		break;

	default:
		goto error_status;
	}


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_GET_LAYER_SEGMENT_NUM

*/
int
realtek_r_isdbt_GetLayerSegmentNum(
	ISDBT_DEMOD_MODULE *pDemod,
	int HierarchicalLayerIndex,
	unsigned char *pLayerSegmentNum
)
{
	int RegBitNameRxNoSeg;
	U32BITS RxNoSeg;



	// Determine target RX_NO_SEG register bit name according to hierarchical layer index.
	switch (HierarchicalLayerIndex) {
	case ISDBT_HIERARCHICAL_LAYER_A:
		RegBitNameRxNoSeg = ISDBT_RX_NO_SEG0;
		break;
	case ISDBT_HIERARCHICAL_LAYER_B:
		RegBitNameRxNoSeg = ISDBT_RX_NO_SEG1;
		break;
	case ISDBT_HIERARCHICAL_LAYER_C:
		RegBitNameRxNoSeg = ISDBT_RX_NO_SEG2;
		break;

	default:
		goto error_status;
	}

	// Get target RX_NO_SEG with determined register bit name.
	if (pDemod->GetRegBits(pDemod, RegBitNameRxNoSeg, &RxNoSeg) != FUNCTION_SUCCESS)
		goto error_status;


	// Store RX_NO_SEG to SegmentNum.
	*pLayerSegmentNum = ((unsigned char)RxNoSeg == 0xf)? 0: (unsigned char)RxNoSeg;


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_GET_GUARD_INTERVAL

*/
int
realtek_r_isdbt_GetGuardInterval(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pGuardInterval
)
{
	U32BITS GiModeR;



	// Get GI_MODE_R.
	if (pDemod->GetRegBits(pDemod, ISDBT_GI_MODE_R, &GiModeR) != FUNCTION_SUCCESS)
		goto error_status;

	// Determine guard interval mode according to GI_MODE_R.
	switch (GiModeR) {
	case 0:
		*pGuardInterval = ISDBT_GUARD_INTERVAL_1_OVER_32;
		break;
	case 1:
		*pGuardInterval = ISDBT_GUARD_INTERVAL_1_OVER_16;
		break;
	case 2:
		*pGuardInterval = ISDBT_GUARD_INTERVAL_1_OVER_8;
		break;
	case 3:
		*pGuardInterval = ISDBT_GUARD_INTERVAL_1_OVER_4;
		break;

	default:
		goto error_status;
	}


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   ISDBT_DEMOD_FP_GET_FFT_MODE

*/
int
realtek_r_isdbt_GetFftMode(
	ISDBT_DEMOD_MODULE *pDemod,
	int *pFftMode
)
{
	U32BITS FftModeR;



	// Get FFT_MODE_R.
	if (pDemod->GetRegBits(pDemod, ISDBT_FFT_MODE_R, &FftModeR) != FUNCTION_SUCCESS)
		goto error_status;

	// Determine FFT mode according to RX_INTLV.
	switch (FftModeR) {
	case 0:
		*pFftMode = ISDBT_FFT_MODE_2K;
		break;
	case 1:
		*pFftMode = ISDBT_FFT_MODE_8K;
		break;
	case 2:
		*pFftMode = ISDBT_FFT_MODE_4K;
		break;

	default:
		goto error_status;
	}


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}





/**

@see   I2C_BRIDGE_FP_FORWARD_I2C_READING_CMD

*/
int
realtek_r_isdbt_ForwardI2cReadingCmd(
	I2C_BRIDGE_MODULE *pI2cBridge,
	unsigned char DeviceAddr,
	unsigned char *pReadingBytes,
	unsigned long ByteNum
)
{
	ISDBT_DEMOD_MODULE *pDemod;
	BASE_INTERFACE_MODULE *pBaseInterface;



	// Get demod module.
	pDemod = (ISDBT_DEMOD_MODULE *)pI2cBridge->pPrivateData;


	// Get base interface.
	pBaseInterface = pDemod->pBaseInterface;


	// Send I2C reading command.
	if (pBaseInterface->I2cRead(pBaseInterface, DeviceAddr, NULL, 0, pReadingBytes, ByteNum) != FUNCTION_SUCCESS)
		goto error_send_i2c_reading_command;


	return FUNCTION_SUCCESS;


error_send_i2c_reading_command:
	return FUNCTION_ERROR;
}





/**

@see   I2C_BRIDGE_FP_FORWARD_I2C_WRITING_CMD

*/
int
realtek_r_isdbt_ForwardI2cWritingCmd(
	I2C_BRIDGE_MODULE *pI2cBridge,
	unsigned char DeviceAddr,
	const unsigned char *pWritingBytes,
	unsigned long ByteNum
)
{
	ISDBT_DEMOD_MODULE *pDemod;
	BASE_INTERFACE_MODULE *pBaseInterface;



	// Get demod module.
	pDemod = (ISDBT_DEMOD_MODULE *)pI2cBridge->pPrivateData;


	// Get base interface.
	pBaseInterface = pDemod->pBaseInterface;


	// Send I2C writing command.
	if (pBaseInterface->I2cWrite(pBaseInterface, DeviceAddr, pWritingBytes, ByteNum) != FUNCTION_SUCCESS)
		goto error_send_i2c_writing_command;


	return FUNCTION_SUCCESS;


error_send_i2c_writing_command:
	return FUNCTION_ERROR;
}





/**

@brief   Initialize RTK_R ISDB-T register table.

Use realtek_r_isdbt_InitRegTable() to initialize RTK_R ISDB-T register table.


@param [in]   pDemod   RTK_R ISDB-T demod module pointer


@note
	-# The realtek_r_isdbt_InitRegTable() function will be called by BuildRtkrIsdbtModule().

*/
void
realtek_r_isdbt_InitRegTable(
	ISDBT_DEMOD_MODULE *pDemod
)
{
	static const ISDBT_PRIMARY_REG_ENTRY PrimaryRegTable[] = {
		// System control
		// RegBitName,            RegStartAddr,   MSB,   LSB
		{ISDBT_DEMOD_SEL,         0xe401,         1,     0     },

		// I2C repeater control
		// RegBitName,            RegStartAddr,   MSB,   LSB
		{ISDBT_I2CT_EN_CTRL,      0xf001,         6,     6     },

		// Auxiliary channel status
		// RegBitName,            RegStartAddr,   MSB,   LSB
		{ISDBT_AC_DECODE_OK,      0xed64,         4,     4     },
		{ISDBT_AC_CRC_OK,         0xed64,         5,     5     },
		{ISDBT_AC_MSG_INT_FLAG,   0xed64,         6,     6     },
		{ISDBT_AC_LOCK,           0xed64,         7,     7     },
		{ISDBT_AC_MSG_WD0,        0xed65,         15,    0     },
		{ISDBT_AC_MSG_DW1,        0xed68,         31,    0     },
		{ISDBT_AC_MSG_DW2,        0xed6c,         31,    0     },
		{ISDBT_AC_MSG_DW3,        0xed70,         31,    0     },
		{ISDBT_AC_MSG_DW4,        0xed74,         31,    0     },
		{ISDBT_AC_MSG_DW5,        0xed78,         31,    0     },
		{ISDBT_AC_MSG_END,        0xed7c,         26,    0     },
		{ISDBT_AC1_DATA_SEGA,     0xed4c,         7,     0     },
		{ISDBT_AC2_DATA_SEGA,     0xed4d,         18,    0     },
		{ISDBT_AC1_DATA_SEGB,     0xed50,         7,     0     },
		{ISDBT_AC2_DATA_SEGB,     0xed51,         18,    0     },
		{ISDBT_AC_OFDM_NUM,       0xed4f,         7,     3     },

		// AGC status
		// RegBitName,            RegStartAddr,   MSB,   LSB
		{ISDBT_RF_AGC_VAL,        0xec10,         13,    0     },
		{ISDBT_IF_AGC_VAL,        0xec12,         13,    0     },
		{ISDBT_AAGC_LOCK,         0xec13,         6,     6     },
		{ISDBT_GAIN_OUT_R,        0xec44,         11,    0     },

		// Signal parameter status
		// RegBitName,            RegStartAddr,   MSB,   LSB
		{ISDBT_FFT_MODE_R,        0xec90,         1,     0     },
		{ISDBT_GI_MODE_R,         0xec90,         3,     2     },
		{ISDBT_RX_QAM0,           0xed5c,         2,     0     },
		{ISDBT_RX_QAM1,           0xed5c,         5,     3     },
		{ISDBT_RX_QAM2,           0xed5d,         2,     0     },
		{ISDBT_SYSTEM_ID,         0xed5c,         7,     6     },
		{ISDBT_EMERGENCY_FLAG,    0xed5d,         3,     3     },
		{ISDBT_PARTIAL_FLAG,      0xed5d,         4,     4     },
		{ISDBT_RX_CRATE0,         0xed5e,         2,     0     },
		{ISDBT_RX_CRATE1,         0xed5e,         5,     3     },
		{ISDBT_RX_CRATE2,         0xed5d,         7,     5     },
		{ISDBT_RX_INTLV0,         0xed5f,         2,     0     },
		{ISDBT_RX_INTLV1,         0xed5f,         5,     3     },
		{ISDBT_RX_INTLV2,         0xed60,         2,     0     },
		{ISDBT_RX_NO_SEG0,        0xed60,         6,     3     },
		{ISDBT_RX_NO_SEG1,        0xed61,         3,     0     },
		{ISDBT_RX_NO_SEG2,        0xed61,         7,     4     },

		// Demod status
		// RegBitName,            RegStartAddr,   MSB,   LSB
		{ISDBT_CFO_EST,           0xec80,         26,    0     },
		{ISDBT_SFO_OUT,           0xec84,         14,    0     },
		{ISDBT_FSM_STATE_R,       0xec94,         4,     0     },
		{ISDBT_EST_SNR,           0xed20,         8,     0     },
		{ISDBT_SIG_PRESENT,       0xec74,         5,     5     },
		{ISDBT_FRM_LOCK,          0xed63,         2,     2     },
		{ISDBT_VBER_0_VALID,      0xee1c,         0,     0     },
		{ISDBT_VBER_0,            0xee1c,         14,    1     },
		{ISDBT_VBER_1_VALID,      0xee20,         0,     0     },
		{ISDBT_VBER_1,            0xee20,         14,    1     },
		{ISDBT_VBER_2_VALID,      0xee24,         0,     0     },
		{ISDBT_VBER_2,            0xee24,         14,    1     },
		{ISDBT_RSBER_0_VALID,     0xee28,         0,     0     },
		{ISDBT_RSBER_0,           0xee28,         18,    2     },
		{ISDBT_RSBER_1_VALID,     0xee2c,         0,     0     },
		{ISDBT_RSBER_1,           0xee2c,         18,    2     },
		{ISDBT_RSBER_2_VALID,     0xee30,         0,     0     },
		{ISDBT_RSBER_2,           0xee30,         18,    2     },
#if USE_PREFRM_LOCK
		{ISDBT_DPBSK_Z_AVG_DBG,   0xed44,         18,    0     },
		{ISDBT_DPBSK_N_AVG_DBG,   0xed48,         23,    0     },
#endif
		// Demod control
		// RegBitName,            RegStartAddr,   MSB,   LSB
		{ISDBT_SOFT_RST_N,        0xe601,         0,     0     },

		// AGC control
		// RegBitName,            RegStartAddr,   MSB,   LSB
		{ISDBT_RFAGC_MAX,         0xe710,         7,     0     },
		{ISDBT_RFAGC_MIN,         0xe711,         7,     0     },
		{ISDBT_IFAGC_MAX,         0xe712,         7,     0     },
		{ISDBT_IFAGC_MIN,         0xe713,         7,     0     },
		{ISDBT_AAGC_HOLD,         0xe706,         5,     5     },
		{ISDBT_PSET_IFAGC,        0xe714,         0,     0     },
		{ISDBT_PSET_RFAGC,        0xe715,         0,     0     },
		{ISDBT_PSET_IFAGC_VAL,    0xe716,         13,    0     },
		{ISDBT_PSET_RFAGC_VAL,    0xe718,         13,    0     },
		{ISDBT_POLAR_IFAGC,       0xe71a,         0,     0     },
		{ISDBT_POLAR_RFAGC,       0xe71a,         1,     1     },
		{ISDBT_EN_IFAGC,          0xe71a,         2,     2     },
		{ISDBT_EN_RFAGC,          0xe71a,         3,     3     },
		{ISDBT_LOOPGAIN1,         0xe705,         4,     0     },
		{ISDBT_Z_AGC,             0xe603,         1,     0     },
		{ISDBT_AAGC_TARGET_VAL,   0xe704,         7,     0     },

		// Down conversion control
		// RegBitName,            RegStartAddr,   MSB,   LSB
		{ISDBT_EN_SP_INV,         0xe72f,         1,     1     },
		//{ISDBT_IFFREQ,            0xe730,         9,     0     },//rm

		// Auxiliary channel control
		// RegBitName,            RegStartAddr,   MSB,   LSB
		{ISDBT_SEG0_AC_DEC_SRC,   0xe81f,         6,     6     },
		{ISDBT_SEGA_AC_DEC_SRC,   0xe820,         1,     0     },
		{ISDBT_SEGA_NUM,          0xe820,         5,     2     },
		{ISDBT_SEGB_AC_DEC_SRC,   0xe820,         7,     6     },
		{ISDBT_SEGB_NUM,          0xe821,         3,     0     },
		{ISDBT_IT_AC_OUT_FRZ,     0xe82d,         7,     7     },

		// Outer control
		// RegBitName,            RegStartAddr,   MSB,   LSB
		{ISDBT_LAYER_SEL,         0xea01,         2,     0     },		// Formal address: 0xea01

		// MPEG interface control
		// RegBitName,            RegStartAddr,   MSB,   LSB
		{ISDBT_CKOUTPAR,          0xea2e,         0,     0     },		// Formal address: 0xea2e
		{ISDBT_SYNC_DUR,          0xea2e,         2,     2     },		// Formal address: 0xea2e
		{ISDBT_ERR_DUR,           0xea2e,         3,     3     },		// Formal address: 0xea2e
		{ISDBT_SERIAL,            0xea2e,         7,     7     },		// Formal address: 0xea2e
		{ISDBT_SER_LSB,           0xea2f,         0,     0     },		// Formal address: 0xea2f
		{ISDBT_CDIV_PH0,          0xea2f,         5,     1     },		// Formal address: 0xea2f
		{ISDBT_CDIV_PH1,          0xea30,         4,     0     },		// Formal address: 0xea30
		{ISDBT_FIX_TEI,           0xea2f,         6,     6     },		// Formal address: 0xea2f

		// Private
		// RegBitName,            RegStartAddr,   MSB,   LSB
		//{ISDBT_PRIVATE_0,         0xe74d,         25,    0     },//rm 0xe74d = 18159D34~9d40
		{ISDBT_DDR_ADDR_OFFSET,   0xea3c,         31,    3     }, 

		// Private
		// RegBitName,            RegStartAddr,   MSB,   LSB
		{ISDBT_RSPER0_BAVG,       0xea1a,         4,     1     },       //Add reg_rsper0_bavg
		{ISDBT_RSPER1_BAVG,       0xea1b,         3,     0     },       //Add reg_rsper1_bavg
		{ISDBT_RSPER2_BAVG,       0xea1b,         7,     4     },       //Add reg_rsper2_bavg
	};


	int i;
	int RegBitName;



	// Initialize register table according to primary register table.
	// Note: 1. Register table rows are sorted by register bit name key.
	//       2. The default value of the IsAvailable variable is "NO".
	for (i = 0; i < ISDBT_REG_TABLE_LEN_MAX; i++)
		pDemod->RegTable[i].IsAvailable  = NO;

	for (i = 0; i < (sizeof(PrimaryRegTable) / sizeof(ISDBT_PRIMARY_REG_ENTRY)); i++) {
		RegBitName = PrimaryRegTable[i].RegBitName;

		pDemod->RegTable[RegBitName].IsAvailable  = YES;
		pDemod->RegTable[RegBitName].RegStartAddr = PrimaryRegTable[i].RegStartAddr;
		pDemod->RegTable[RegBitName].Msb          = PrimaryRegTable[i].Msb;
		pDemod->RegTable[RegBitName].Lsb          = PrimaryRegTable[i].Lsb;
	}


	return;
}





/**

@brief   Set I2C bridge module demod arguments.

RTK_R ISDB-T builder will use realtek_r_isdbt_BuildI2cBridgeModule() to set I2C bridge module demod arguments.


@param [in]   pDemod   The demod module pointer


@see   BuildRtkrIsdbtModule()

*/
void
realtek_r_isdbt_BuildI2cBridgeModule(
	ISDBT_DEMOD_MODULE *pDemod
)
{
	I2C_BRIDGE_MODULE *pI2cBridge;



	// Get I2C bridge module.
	pI2cBridge = pDemod->pI2cBridge;

	// Set I2C bridge module demod arguments.
	pI2cBridge->pPrivateData = (void *)pDemod;
	pI2cBridge->ForwardI2cReadingCmd = realtek_r_isdbt_ForwardI2cReadingCmd;
	pI2cBridge->ForwardI2cWritingCmd = realtek_r_isdbt_ForwardI2cWritingCmd;


	return;
}





/**

Download firmware code (for demo only) // A version IC

*/
int
realtek_r_isdbt_DownloadFirmwareCode(
	ISDBT_DEMOD_MODULE *pDemod
)
{
	// Initializing entry only used in realtek_r_isdbt_DownloadFirmwareCode()
	typedef struct {
		unsigned short RegStartAddr;
		unsigned char Msb;
		unsigned char Lsb;
		U32BITS WritingValue;
	}
	INIT_ENTRY;



	static const INIT_ENTRY InitTable[] = {
		// RegStartAddr,   Msb,   Lsb,   WritingValue
		{0xf080,           4,     1,     0x2            },
		{0xf081,           1,     1,     0x0            },
		{0xf082,           0,     0,     0x1            },
		{0xf083,           7,     0,     0x0            },
		{0xf084,           5,     0,     0x0            },
		{0xf085,           5,     1,     0x6            },
		{0xf086,           4,     0,     0x6            },
		{0xf08a,           2,     0,     0x7            },
		{0xf08b,           2,     2,     0x0            },
		{0xf08b,           3,     3,     0x1            },
		{0xf08b,           7,     7,     0x0            },
		{0xf08c,           0,     0,     0x0            },
		{0xf08d,           6,     6,     0x1            },
	};

	static const unsigned char FirmwareCode[] = {
		0x75,
		0x34,
		0x0,
		0x75,
		0x35,
		0x0,
		0x90,
		0xf0,
		0x8,
		0xe4,
		0xf0,
		0xe5,
		0x35,
		0x45,
		0x34,
		0x70,
		0x3,
		0x2,
		0x30,
		0xde,
		0x12,
		0x50,
		0x0,
		0x12,
		0x14,
		0x12,
		0x12,
		0x0,
		0x2e,
		0x12,
		0x16,
		0xd3,
		0x12,
		0x39,
		0x9a,
		0x12,
		0x38,
		0x94,
		0x12,
		0x37,
		0x86,
		0x90,
		0x34,
		0xf0,
		0x12,
		0x1b,
		0xb6,
		0x12,
		0x18,
		0x9e,
		0xff,
		0x8f,
		0x99,
		0x90,
		0x34,
		0xf0,
		0x12,
		0x1b,
		0xb6,
		0x75,
		0x82,
		0x1,
		0x75,
		0x83,
		0x0,
		0x12,
		0x18,
		0xb7,
		0xff,
		0x8f,
		0x99,
		0x90,
		0xf0,
		0x30,
		0x12,
		0x1b,
		0x79,
		0x0,
		0x0,
		0x0,
		0x0,
		0x90,
		0xf0,
		0x30,
		0x12,
		0x1b,
		0x45,
		0xef,
		0x65,
		0x33,
		0xff,
		0xee,
		0x65,
		0x32,
		0xfe,
		0xed,
		0x65,
		0x31,
		0xfd,
		0xec,
		0x65,
		0x30,
		0xfc,
		0x90,
		0xf0,
		0x30,
		0x12,
		0x1b,
		0x6d,
		0x90,
		0xf0,
		0x30,
		0x12,
		0x1b,
		0x45,
		0x12,
		0x1c,
		0x15,
		0x30,
		0xc0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x30,
		0x80,
		0x75,
		0x2c,
		0x0,
		0x75,
		0x2b,
		0x0,
		0x75,
		0x2a,
		0x0,
		0x75,
		0x29,
		0x0,
		0xaf,
		0x2c,
		0xae,
		0x2b,
		0xad,
		0x2a,
		0xac,
		0x29,
		0x7b,
		0x0,
		0x7a,
		0x0,
		0x79,
		0x0,
		0x78,
		0x0,
		0xd3,
		0x12,
		0x1a,
		0xf5,
		0x50,
		0x1e,
		0xe5,
		0x2c,
		0x24,
		0xff,
		0xff,
		0xe5,
		0x2b,
		0x34,
		0xff,
		0xfe,
		0xe5,
		0x2a,
		0x34,
		0xff,
		0xfd,
		0xe5,
		0x29,
		0x34,
		0xff,
		0xfc,
		0x8f,
		0x2c,
		0x8e,
		0x2b,
		0x8d,
		0x2a,
		0x8c,
		0x29,
		0x80,
		0xcc,
		0x90,
		0x34,
		0xf0,
		0x12,
		0x1b,
		0xb6,
		0x12,
		0x18,
		0x9e,
		0xff,
		0x8f,
		0x99,
		0x90,
		0x34,
		0xf0,
		0x12,
		0x1b,
		0xb6,
		0x75,
		0x82,
		0x1,
		0x75,
		0x83,
		0x0,
		0x12,
		0x18,
		0xb7,
		0xff,
		0x8f,
		0x99,
		0x90,
		0xe4,
		0x1,
		0xe0,
		0xff,
		0xef,
		0x64,
		0x3,
		0x60,
		0x3,
		0x2,
		0x32,
		0x2e,
		0x90,
		0xf0,
		0x60,
		0x74,
		0xdb,
		0xf0,
		0x7b,
		0x0,
		0x7d,
		0x1f,
		0x7f,
		0x94,
		0x7e,
		0xc6,
		0x12,
		0x4,
		0xc8,
		0xef,
		0xc3,
		0x94,
		0x1,
		0x40,
		0xef,
		0x7b,
		0x1,
		0x7d,
		0x1e,
		0x7f,
		0x80,
		0x7e,
		0xf0,
		0x12,
		0x4,
		0xc8,
		0x8f,
		0x26,
		0x75,
		0x57,
		0x1,
		0x7b,
		0x1e,
		0xad,
		0x26,
		0x7f,
		0x1a,
		0x7e,
		0xc4,
		0x12,
		0x3,
		0x68,
		0x7b,
		0x1,
		0x7d,
		0x2,
		0x7f,
		0x81,
		0x7e,
		0xf0,
		0x12,
		0x4,
		0xc8,
		0x8f,
		0x26,
		0x75,
		0x57,
		0x1,
		0x7b,
		0x2,
		0xad,
		0x26,
		0x7f,
		0x2e,
		0x7e,
		0xc4,
		0x12,
		0x3,
		0x68,
		0x7b,
		0x0,
		0x7d,
		0x1,
		0x7f,
		0x82,
		0x7e,
		0xf0,
		0x12,
		0x4,
		0xc8,
		0x8f,
		0x26,
		0x75,
		0x57,
		0x0,
		0x7b,
		0x1,
		0xad,
		0x26,
		0x7f,
		0x34,
		0x7e,
		0xc4,
		0x12,
		0x3,
		0x68,
		0x7b,
		0x0,
		0x7d,
		0xff,
		0x7f,
		0x83,
		0x7e,
		0xf0,
		0x12,
		0x4,
		0xc8,
		0x8f,
		0x26,
		0x75,
		0x57,
		0x0,
		0x7b,
		0xff,
		0xad,
		0x26,
		0x7f,
		0x31,
		0x7e,
		0xc4,
		0x12,
		0x3,
		0x68,
		0x7b,
		0x0,
		0x7d,
		0x3f,
		0x7f,
		0x84,
		0x7e,
		0xf0,
		0x12,
		0x4,
		0xc8,
		0x8f,
		0x26,
		0x75,
		0x57,
		0x0,
		0x7b,
		0x3f,
		0xad,
		0x26,
		0x7f,
		0x32,
		0x7e,
		0xc4,
		0x12,
		0x3,
		0x68,
		0x7b,
		0x1,
		0x7d,
		0x3e,
		0x7f,
		0x85,
		0x7e,
		0xf0,
		0x12,
		0x4,
		0xc8,
		0x8f,
		0x26,
		0x75,
		0x57,
		0x1,
		0x7b,
		0x3e,
		0xad,
		0x26,
		0x7f,
		0x2f,
		0x7e,
		0xc4,
		0x12,
		0x3,
		0x68,
		0x7b,
		0x0,
		0x7d,
		0x1f,
		0x7f,
		0x86,
		0x7e,
		0xf0,
		0x12,
		0x4,
		0xc8,
		0x8f,
		0x26,
		0x75,
		0x57,
		0x0,
		0x7b,
		0x1f,
		0xad,
		0x26,
		0x7f,
		0x30,
		0x7e,
		0xc4,
		0x12,
		0x3,
		0x68,
		0x7b,
		0x0,
		0x7d,
		0x7,
		0x7f,
		0x8a,
		0x7e,
		0xf0,
		0x12,
		0x4,
		0xc8,
		0x8f,
		0x26,
		0x75,
		0x57,
		0x0,
		0x7b,
		0x7,
		0xad,
		0x26,
		0x7f,
		0x1,
		0x7e,
		0xc4,
		0x12,
		0x3,
		0x68,
		0x7b,
		0x2,
		0x7d,
		0x8c,
		0x7f,
		0x8b,
		0x7e,
		0xf0,
		0x12,
		0x4,
		0xc8,
		0x8f,
		0x26,
		0x75,
		0x57,
		0x2,
		0x7b,
		0x8c,
		0xad,
		0x26,
		0x7f,
		0x2e,
		0x7e,
		0xc4,
		0x12,
		0x3,
		0x68,
		0x7b,
		0x0,
		0x7d,
		0x1,
		0x7f,
		0x8c,
		0x7e,
		0xf0,
		0x12,
		0x4,
		0xc8,
		0x8f,
		0x26,
		0x75,
		0x57,
		0x0,
		0x7b,
		0x1,
		0xad,
		0x26,
		0x7f,
		0x2f,
		0x7e,
		0xc4,
		0x12,
		0x3,
		0x68,
		0x7b,
		0x6,
		0x7d,
		0x40,
		0x7f,
		0x8d,
		0x7e,
		0xf0,
		0x12,
		0x4,
		0xc8,
		0x8f,
		0x26,
		0x75,
		0x57,
		0x6,
		0x7b,
		0x40,
		0xad,
		0x26,
		0x7f,
		0x2f,
		0x7e,
		0xc4,
		0x12,
		0x3,
		0x68,
		0x2,
		0x30,
		0xf1,
		0x22,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0x0,
		0xab,
		0xcd,
	};


	int i;

	unsigned short RegStartAddr;
	unsigned char Msb;
	unsigned char Lsb;
	U32BITS WritingValue;



	// Initialize demod registers according to the initializing table.
	for (i = 0; i < (sizeof(InitTable) / sizeof(INIT_ENTRY)); i++) {
		// Get all information from each entry.
		RegStartAddr = InitTable[i].RegStartAddr;
		Msb          = InitTable[i].Msb;
		Lsb          = InitTable[i].Lsb;
		WritingValue = InitTable[i].WritingValue;

		// Set register mask bits.
		if (pDemod->SetRegMaskBits(pDemod, RegStartAddr, Msb, Lsb, WritingValue) != FUNCTION_SUCCESS)
			goto error_status;
	}


	// Enable firmware code downloading hardware.
	if (pDemod->SetRegMaskBits(pDemod, 0xf007, 7, 0, 0x4) != FUNCTION_SUCCESS)
		goto error_status;

	if (pDemod->SetRegMaskBits(pDemod, 0xf001, 7, 0, 0x81) != FUNCTION_SUCCESS)
		goto error_status;


	// Donwload firmware code.
	for (RegStartAddr = 0x3000, i = 0; i < (sizeof(FirmwareCode) / sizeof(unsigned char)); i++, RegStartAddr++) {
		// Set register bytes.
		if (pDemod->SetRegBytes(pDemod, RegStartAddr, &(FirmwareCode[i]), LEN_1_BYTE) != FUNCTION_SUCCESS)
			goto error_status;
	}


	// Run firmware code.
	if (pDemod->SetRegMaskBits(pDemod, 0xf001, 7, 0, 0x1) != FUNCTION_SUCCESS)
		goto error_status;

	if (pDemod->SetRegMaskBits(pDemod, 0xf001, 7, 0, 0x21) != FUNCTION_SUCCESS)
		goto error_status;

	if (pDemod->SetRegMaskBits(pDemod, 0xf005, 7, 0, 0x1) != FUNCTION_SUCCESS)
		goto error_status;


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
}








/**

@brief   Set RTK_R ISDBT demod register mask bits with acceleration.

RTK_R ISDBT upper level functions will use realtek_r_isdb_acceleration_SetRegMaskBits() to set demod register mask bits.


@param [in]   pDemod         The demod module pointer
@param [in]   RegStartAddr   Demod register start address
@param [in]   Msb            Mask MSB with 0-based index
@param [in]   Lsb            Mask LSB with 0-based index
@param [in]   WritingValue   The mask bits writing value


@retval   FUNCTION_SUCCESS   Set demod register mask bits successfully with writing value.
@retval   FUNCTION_ERROR     Set demod register mask bits unsuccessfully.


@note
	-# The realtek_r_isdb_acceleration_SetRegMaskBits() function accelerate mask bits setting in byte writitng case.
	-# Need to set register page by ISDBT_DEMOD_FP_SET_REG_PAGE() before using realtek_r_isdb_acceleration_SetRegMaskBits().
	-# The constraints of realtek_r_isdb_acceleration_SetRegMaskBits() function usage are described as follows:
		-# The mask MSB and LSB must be 0~31.
		-# The mask MSB must be greater than or equal to LSB.

*/
int
realtek_r_isdb_acceleration_SetRegMaskBits(
	ISDBT_DEMOD_MODULE *pDemod,
	unsigned short RegStartAddr,
	unsigned char Msb,
	unsigned char Lsb,
	const U32BITS WritingValue
)
{
	int i;

	unsigned char WritingBytes[LEN_4_BYTE];
	unsigned char ByteNum;


	if ((Lsb == 0) && (((Msb + 1) % BYTE_BIT_NUM) == 0)) {
		// Calculate writing byte number according to MSB.
		ByteNum = Msb / BYTE_BIT_NUM + LEN_1_BYTE;

		// Separate writing value into writing bytes.
		// Note: Pick up lower address byte from value LSB.
		//       Pick up upper address byte from value MSB.
		for (i = 0; i < ByteNum; i++)
			WritingBytes[i] = (unsigned char)((WritingValue >> (BYTE_SHIFT * i)) & BYTE_MASK);

		// Use byte setting method to set register mask bits.
		if (isdbt_demod_default_SetRegBytes(pDemod, RegStartAddr, WritingBytes, ByteNum) != FUNCTION_SUCCESS)
			goto error_status_set_demod_registers;
	} else {
		// Use mask bit setting method to set register mask bits.
		if (isdbt_demod_default_SetRegMaskBits(pDemod, RegStartAddr, Msb, Lsb, WritingValue) !=
			FUNCTION_SUCCESS)
			goto error_status_set_demod_registers;
	}


	return FUNCTION_SUCCESS;


error_status_set_demod_registers:
	return FUNCTION_ERROR;
}

