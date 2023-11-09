/**

@file

@brief   RTK_R ISDB-T demod module definition

One can manipulate RTK_R ISDB-T demod through RTK_R ISDB-T module.
RTK_R ISDB-T module is derived from ISDB-T demod module.

*/

//#include <stdio.h>
//#include <assert.h>
//#include <string.h>
#include <linux/slab.h>
#include <io.h>
#include "isdbt_demod_rtk_r.h"
#include "rbus/rtk_dtv_demod_isdbt_reg.h"
#include "demod_rtk_r_isdbt_priv.h"
#include "isdbt_demod_rtk_r_base.h"

#define IsdbtUseMerlin4InitValue 1
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
	if (pBaseInterface->I2cRead(pBaseInterface, pDemod->DeviceAddr, &Nothing, LEN_1_BYTE) == FUNCTION_ERROR)
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

	// Set SOFT_RST_N with 0x0. Then, set SOFT_RST_N with 0x1.
	if (pDemod->SetRegBits(pDemod, ISDBT_SOFT_RST_N, 0x0) != FUNCTION_SUCCESS)
		goto error_status;

	if (pDemod->SetRegBits(pDemod, ISDBT_SOFT_RST_N, 0x1) != FUNCTION_SUCCESS)
		goto error_status;


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
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

	if (pDemod->SetRegBits(pDemod, ISDBT_SOFT_RST_N, 0x0) != FUNCTION_SUCCESS)
		goto error_status;

	if (pExtra->ShareMemoryOfst & 0xfff) {
		RTK_DEMOD_ISDBT_INFO("[RTK_R_ISDBT] Invalid Share Memory Offset : share memory of ISDB-T demod should be 4KB alignment\n");
		//assert(0);
	} else {
		RTK_DEMOD_ISDBT_INFO("[RTK_R_ISDBT] Set Share Memory Offset = ("PT_H32BITS") \n", pExtra->ShareMemoryOfst);
		pDemod->SetRegBits(pDemod, ISDBT_DDR_ADDR_OFFSET, pExtra->ShareMemoryOfst >> 3);
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
	U32BITS CdivPh0;
	U32BITS CdivPh1;



	// Determine SERIAL, CDIV_PH0, and CDIV_PH1 setting value according to TS interface mode.
	switch (TsInterfaceMode) {
	case TS_INTERFACE_PARALLEL:
		Serial = 0;
		CdivPh0 = 6;
		CdivPh1 = 6;
		break;
	case TS_INTERFACE_SERIAL:
		Serial = 1;
		CdivPh0 = 0;
		CdivPh1 = 0;
		break;

	default:
		goto error_status;
	}


	// Set SERIAL, CDIV_PH0, and CDIV_PH1 with determined value.
	if (pDemod->SetRegBits(pDemod, ISDBT_SERIAL, Serial) != FUNCTION_SUCCESS)
		goto error_status;

	if (pDemod->SetRegBits(pDemod, ISDBT_CDIV_PH0, CdivPh0) != FUNCTION_SUCCESS)
		goto error_status;

	if (pDemod->SetRegBits(pDemod, ISDBT_CDIV_PH1, CdivPh1) != FUNCTION_SUCCESS)
		goto error_status;


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

	if (pDemod->SetRegBits(pDemod, ISDBT_CKOUTPAR, TsClkPol) != FUNCTION_SUCCESS)
		goto error_status;


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
	U32BITS IfFreqHz
)
{
	U32BITS CrystalFreqHz = 0;
	U32BITS AdcFreqHz = 0;

	U32BITS IfFreqHzAdj = 0;

	MPI MpiVar, MpiConst, MpiNone;

	S32BITS IffreqInt = 0;
	U32BITS IffreqBinary = 0;

	memset(&MpiVar, 0 , sizeof(MpiVar));
	memset(&MpiNone, 0 , sizeof(MpiNone));
	memset(&MpiConst, 0 , sizeof(MpiConst));;



	// Get demod crystal frequency in Hz.
	pDemod->GetCrystalFreqHz(pDemod, &CrystalFreqHz);

	// Calculate demod ADC frequency in Hz according to demod crystal frequency in Hz.
	AdcFreqHz = CrystalFreqHz * RTK_R_ISDBT_ADC_TO_CRYSTAL_FREQ_RATIO;


	// Calculate IFFREQ value.
	// Note: Case 1: IfFreqHz < (AdcFreqHz / 2),  IfFreqHzAdj = IfFreqHz;
	//       Case 2: IfFreqHz >= (AdcFreqHz / 2), IfFreqHzAdj = AdcFreqHz - IfFreqHz;
	//       IFFREQ = - round( IfFreqHzAdj * pow(2, IFFREQ_BIT_NUM) / AdcFreqHz )
	//              = - floor( (IfFreqHzAdj * pow(2, IFFREQ_BIT_NUM) + (AdcFreqHz / 2)) / AdcFreqHz )
	//       IFFREQ_BIT_NUM = 10
	IfFreqHzAdj = (IfFreqHz < (AdcFreqHz / 2)) ? IfFreqHz : (AdcFreqHz - IfFreqHz);

	MpiSetValue(&MpiVar, (long)IfFreqHzAdj);
	MpiLeftShift(&MpiVar, MpiVar, RTK_R_ISDBT_IFFREQ_BIT_NUM);

	MpiSetValue(&MpiConst, (long)(AdcFreqHz / 2));
	MpiAdd(&MpiVar, MpiVar, MpiConst);

	MpiSetValue(&MpiConst, (long)AdcFreqHz);
	MpiDiv(&MpiVar, &MpiNone, MpiVar, MpiConst);

	MpiGetValue(MpiVar, &IffreqInt);
	IffreqInt = - IffreqInt;

	IffreqBinary = SignedIntToBin((long)IffreqInt, RTK_R_ISDBT_IFFREQ_BIT_NUM);


	// Set IFFREQ with calculated value.
	if (pDemod->SetRegBits(pDemod, ISDBT_IFFREQ, IffreqBinary) != FUNCTION_SUCCESS)
		goto error_status;


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
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
	U32BITS EnSpInv;



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


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
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

	static const SETTING_ENTRY_GROUP_2 SettingTableGroup2[] = {
		// RegStartAddr,   Msb,   Lsb,   WritingValue
		//                               {SN=1,   SN=3,   SN=13  }   SN: Segment number
		{0xe62a,           2,     0,     {2,      3,      0      }   },
		{0xe7b6,           7,     0,     {28,     30,     64     }   },
		{0xe831,           12,    0,     {1,      7,      8191   }   },
	};

	static const SETTING_ENTRY_GROUP_3 SettingTableGroup3[] = {
		// RegStartAddr,   Msb,   Lsb,   WritingValue
		//                               {SN=1,3,   SN=13     }   SN: Segment number
		{0xe792,           0,     0,     {1,        1,        }   },
		{0xe7b8,           2,     0,     {1,        3,        }   },
		{0xe7b9,           6,     0,     {4,        16,       }   },
		{0xe7ba,           6,     0,     {2,        8,        }   },
	};

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

	U32BITS Private0 = 0;
	U32BITS Reg0 = 0;
	U32BITS Reg1 = 0;



	// Determine PRIVATE_0 and REG_0 value according to bandwidth mode.
	switch (BandwidthMode) {
	case ISDBT_BANDWIDTH_6MHZ:
		Private0 = 6967296;
		Reg0 = 0;
		break;
	case ISDBT_BANDWIDTH_7MHZ:
		Private0 = 5971968;
		Reg0 = 0;
		break;
	case ISDBT_BANDWIDTH_8MHZ:
		Private0 = 5225472;
		Reg0 = 1;
		break;

	}

	// Set PRIVATE_0 and REG_0 with determined value.
	if (pDemod->SetRegBits(pDemod, ISDBT_PRIVATE_0, Private0) != FUNCTION_SUCCESS)
		goto error_status;

	if (pDemod->SetRegMaskBits(pDemod, 0xe7c4, 3, 3, Reg0) != FUNCTION_SUCCESS)
		goto error_status;


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


	// Determine REG_1 value according to sub-channel number.
	Reg1 = (SegmentNum == 13) ? 22 : SubChannelNum;

	// Set REG_1 with determined value.
	if (pDemod->SetRegMaskBits(pDemod, 0xe7c7, 5, 0, Reg1) != FUNCTION_SUCCESS)
		goto error_status;


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
	U32BITS Rsber;



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

	// Determine answer according to RSBER_0_VALID, RSBER_1_VALID, RSBER_2_VALID values.
	if (((Rsber0Valid == 0x1) || (Rsber1Valid == 0x1) || (Rsber2Valid == 0x1)) /*&& (Rsber != RTK_R_ISDBT_PER_DEN_VALUE)*/) {
		*pAnswer = YES;
	} else {
		*pAnswer = NO;
	}


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
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

	if (pDemod->GetRegBits(pDemod, ISDBT_SIG_PRESENT, &SigPresent) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	if (SigPresent == 1)
		*pAnswer = NO;
	else
		*pAnswer = YES;

	return FUNCTION_SUCCESS;


error_status_get_demod_registers:
	return FUNCTION_ERROR;
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
	int RegBitNameRsber;
	U32BITS Rsber;

	MPI MpiVar;
	S32BITS Var;



	// Determine target RSBER register bit name according to hierarchical layer index.
	switch (HierarchicalLayerIndex) {
	case ISDBT_HIERARCHICAL_LAYER_A:
		RegBitNameRsber = ISDBT_RSBER_0;
		break;
	case ISDBT_HIERARCHICAL_LAYER_B:
		RegBitNameRsber = ISDBT_RSBER_1;
		break;
	case ISDBT_HIERARCHICAL_LAYER_C:
		RegBitNameRsber = ISDBT_RSBER_2;
		break;

	default:
		goto error_status;
	}

	// Get target RSBER with determined register bit name.
	if (pDemod->GetRegBits(pDemod, RegBitNameRsber, &Rsber) != FUNCTION_SUCCESS)
		goto error_status;


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
	int RegBitNameVberValid, RegBitNameVber;
	U32BITS VberValid, Vber;



	// Determine target VBER_VALID and VBER register bit name according to hierarchical layer index.
	switch (HierarchicalLayerIndex) {
	case ISDBT_HIERARCHICAL_LAYER_A:

		RegBitNameVberValid = ISDBT_VBER_0_VALID;
		RegBitNameVber = ISDBT_VBER_0;

		break;

	case ISDBT_HIERARCHICAL_LAYER_B:

		RegBitNameVberValid = ISDBT_VBER_1_VALID;
		RegBitNameVber = ISDBT_VBER_1;

		break;

	case ISDBT_HIERARCHICAL_LAYER_C:

		RegBitNameVberValid = ISDBT_VBER_2_VALID;
		RegBitNameVber = ISDBT_VBER_2;

		break;

	default:
		goto error_status;
	}

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
	int RegBitNameRsber;
	U32BITS Rsber;



	// Determine target RSBER register bit name according to hierarchical layer index.
	switch (HierarchicalLayerIndex) {
	case ISDBT_HIERARCHICAL_LAYER_A:
		RegBitNameRsber = ISDBT_RSBER_0;
		break;
	case ISDBT_HIERARCHICAL_LAYER_B:
		RegBitNameRsber = ISDBT_RSBER_1;
		break;
	case ISDBT_HIERARCHICAL_LAYER_C:
		RegBitNameRsber = ISDBT_RSBER_2;
		break;

	default:
		goto error_status;
	}

	// Get target RSBER with determined register bit name.
	if (pDemod->GetRegBits(pDemod, RegBitNameRsber, &Rsber) != FUNCTION_SUCCESS)
		goto error_status;


	// Set PER numerator according to RSBER.
	*pLayerPerNum = Rsber;

	// Set PER denominator.
	*pLayerPerDen = RTK_R_ISDBT_PER_DEN_VALUE;


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



	// Get EST_SNR.
	if (pDemod->GetRegBits(pDemod, ISDBT_EST_SNR, &EstSnr) != FUNCTION_SUCCESS)
		goto error_status;


	// Set SNR dB numerator according to EST_SNR.
	*pSnrDbNum = BinToSignedInt(EstSnr, RTK_R_ISDBT_EST_SNR_BIT_NUM);

	// Set SNR dB denominator.
	*pSnrDbDen = RTK_R_ISDBT_SNR_DB_DEN_VALUE;


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
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
	U32BITS IfAgcVal;



	// Get IF_AGC_VAL.
	if (pDemod->GetRegBits(pDemod, ISDBT_IF_AGC_VAL, &IfAgcVal) != FUNCTION_SUCCESS)
		goto error_status;


	// Store IF_AGC_VAL to IfAgc.
	// Note: The range of IF_AGC_VAL is -8192 ~ 8191.
	*pIfAgc = (long)BinToSignedInt(IfAgcVal, RTK_R_ISDBT_IF_AGC_REG_BIT_NUM);


	return FUNCTION_SUCCESS;


error_status:
	return FUNCTION_ERROR;
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
	if (pDemod->GetRegBits(pDemod, ISDBT_PRIVATE_0, &Private0) != FUNCTION_SUCCESS)
		goto error_status;

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
	if (pDemod->GetRegBits(pDemod, ISDBT_PRIVATE_0, &Private0) != FUNCTION_SUCCESS)
		goto error_status;

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
	if (pBaseInterface->I2cRead(pBaseInterface, DeviceAddr, pReadingBytes, ByteNum) != FUNCTION_SUCCESS)
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
		{ISDBT_IFFREQ,            0xe730,         9,     0     },

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
		{ISDBT_PRIVATE_0,         0xe74d,         25,    0     },
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

