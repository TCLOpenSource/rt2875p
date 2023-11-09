/*
@file

@brief   REALTEK_R ATSC demod module definition

One can manipulate REALTEK_R ATSC demod through REALTEK_R ATSC module.
REALTEK_R ATSC module is derived from ATSC demod module.
*/


#include <linux/slab.h>
//#include <stdio.h>
#include "atsc_demod_rtk_r.h"
#include "tv_osal.h"


#define REALTEK_R_ATSC_8051FW_PATCH
//#define GET_SIGNAL_STRENGTH_CUSTOMER_SONY

/*
@brief   REALTEK_R ATSC demod module builder

Use BuildRealtekRAtscModule() to build REALTEK_R ATSC module, set all module function pointers with the corresponding
functions, and initialize module private variables.

@param [in]   ppDemod                        Pointer to REALTEK_R ATSC demod module pointer
@param [in]   pAtscDemodModuleMemory         Pointer to an allocated ATSC demod module memory
@param [in]   pBaseInterfaceModuleMemory     Pointer to an allocated base interface module memory
@param [in]   pI2cBridgeModuleMemory         Pointer to an allocated I2C bridge module memory
@param [in]   DeviceAddr                     REALTEK_R ATSC I2C device address
@param [in]   CrystalFreqHz                  REALTEK_R ATSC crystal frequency in Hz

@note
	-# One should call BuildRealtekRAtscModule() to build REALTEK_R ATSC module before using it.
*/
void
BuildRealtekRAtscModule(
	ATSC_DEMOD_MODULE **ppDemod,
	ATSC_DEMOD_MODULE *pAtscDemodModuleMemory,
	BASE_INTERFACE_MODULE *pBaseInterfaceModuleMemory,
	I2C_BRIDGE_MODULE *pI2cBridgeModuleMemory,
	unsigned char DeviceAddr,
	U32BITS CrystalFreqHz
)
{
	ATSC_DEMOD_MODULE *pDemod;

	// Set demod module pointer and get demod module.
	*ppDemod = pAtscDemodModuleMemory;
	pDemod = *ppDemod;

	// Set base interface module pointer and I2C bridge module pointer.
	pDemod->pExtra         = INVALID_POINTER_VALUE;
	pDemod->pBaseInterface = pBaseInterfaceModuleMemory;
	pDemod->pI2cBridge     = pI2cBridgeModuleMemory;

	// Set demod type.
	pDemod->DemodType = ATSC_DEMOD_TYPE_REALTEK_R;

	// Set demod I2C device address.
	pDemod->DeviceAddr = DeviceAddr;

	// Set demod crystal frequency in Hz.
	pDemod->CrystalFreqHz = CrystalFreqHz;

	// Set demod TS interface mode.
	pDemod->TsInterfaceMode = NO_USE;

	// Initialize demod parameter setting status
	pDemod->IsIfFreqHzSet      = NO;
	pDemod->IsSpectrumModeSet  = NO;

	// Initialize register tables in demod module.
	realtek_r_atsc_InitRegTable(pDemod);

	// Build I2C birdge module.
	realtek_r_atsc_BuildI2cBridgeModule(pDemod);

	// Set demod module I2C function pointers with default functions.
	pDemod->RegAccess.Addr16Bit.SetRegBytes        = atsc_demod_addr_16bit_default_SetRegBytes;
	pDemod->RegAccess.Addr16Bit.GetRegBytes        = atsc_demod_addr_16bit_default_GetRegBytes;
	pDemod->RegAccess.Addr16Bit.SetRegMaskBits     = atsc_demod_addr_16bit_default_SetRegMaskBits;
	pDemod->RegAccess.Addr16Bit.GetRegMaskBits     = atsc_demod_addr_16bit_default_GetRegMaskBits;
	pDemod->RegAccess.Addr16Bit.SetRegBits         = atsc_demod_addr_16bit_default_SetRegBits;
	pDemod->RegAccess.Addr16Bit.GetRegBits         = atsc_demod_addr_16bit_default_GetRegBits;

	// Set demod module I2C function pointers with particular functions.
	pDemod->RegAccess.Addr16Bit.SetRegMaskBits     = realtek_r_atsc_acceleration_SetRegMaskBits;

	// Set demod module manipulating function pointers with default functions.
	pDemod->GetDemodType     = atsc_demod_default_GetDemodType;
	pDemod->GetDeviceAddr    = atsc_demod_default_GetDeviceAddr;
	pDemod->GetCrystalFreqHz = atsc_demod_default_GetCrystalFreqHz;
	pDemod->GetIfFreqHz      = atsc_demod_default_GetIfFreqHz;
	pDemod->GetSpectrumMode  = atsc_demod_default_GetSpectrumMode;

	// Set demod module manipulating function pointers with particular functions.
	pDemod->IsConnectedToI2c  = realtek_r_atsc_IsConnectedToI2c;
	pDemod->SoftwareReset     = realtek_r_atsc_SoftwareReset;
	pDemod->TrPhaseIncReset   = realtek_r_atsc_TrPhaseInc_Reset;
	pDemod->Initialize        = realtek_r_atsc_Initialize;
	pDemod->SetIfFreqHz       = realtek_r_atsc_SetIfFreqHz;
	pDemod->SetSpectrumMode   = realtek_r_atsc_SetSpectrumMode;
	pDemod->GetRfAgc          = realtek_r_atsc_GetRfAgc;
	pDemod->GetIfAgc          = realtek_r_atsc_GetIfAgc;
	pDemod->GetDiAgc          = realtek_r_atsc_GetDiAgc;
	pDemod->GetTrOffsetPpm    = realtek_r_atsc_GetTrOffsetPpm;
	
	//pDemod->GetTrFsFreqSubOffsetPpm    = realtek_r_atsc_GetTrFsFreqSubOffsetPpm;
	pDemod->GetCrOffsetHz     = realtek_r_atsc_GetCrOffsetHz;
	pDemod->IsAagcLocked      = realtek_r_atsc_IsAagcLocked;
	pDemod->IsFrameLocked     = realtek_r_atsc_IsFrameLocked;
	pDemod->IsTrLocked     	  = realtek_r_atsc_IsTrLocked;
	pDemod->IsCrLocked        = realtek_r_atsc_IsCrLocked;
	pDemod->GetTrLoopSel      = realtek_r_atsc_GetTrLoopSel;
	pDemod->IsInnerLocked     = realtek_r_atsc_IsFrameLocked;
	pDemod->IsNoSig  		  = realtek_r_atsc_IsNoSig;
	pDemod->GetErrorRate      = realtek_r_atsc_GetErrorRate;
	pDemod->GetSnrDb          = realtek_r_atsc_GetSnrDb;
	pDemod->GetSignalStrength = realtek_r_atsc_GetSignalStrength;
	pDemod->GetSignalQuality  = realtek_r_atsc_GetSignalQuality;
	pDemod->UpdateFunction    = realtek_r_atsc_UpdateFunction;
	pDemod->ResetFunction     = realtek_r_atsc_ResetFunction;

	return;
}


/*
@see   ATSC_DEMOD_FP_IS_CONNECTED_TO_I2C
*/
void
realtek_r_atsc_IsConnectedToI2c(
	ATSC_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	U32BITS ReadingValue;

	// Set reading value to zero, and get SYS_VERSION value.
	// Note: Use GetRegBits() to get register bits with page setting.
	ReadingValue = 0;

	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_SYS_VERSION, &ReadingValue) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Compare SYS_VERSION value with REALTEK_R_ATSC_SYS_VERSION_VALUE.
	if (ReadingValue == REALTEK_R_ATSC_SYS_VERSION_VALUE)
		*pAnswer = YES;
	else
		*pAnswer = NO;

	return;

error_status_get_demod_registers:
	*pAnswer = NO;

	return;
}


/*
@see   ATSC_DEMOD_FP_SOFTWARE_RESET
*/
int
realtek_r_atsc_SoftwareReset(
	ATSC_DEMOD_MODULE *pDemod
)
{
	// Set and clear SOFT_RESET register bit.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_OPT_SOFT_RESET_ATSC, 1) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	tv_osal_usleep(20);

	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_OPT_SOFT_RESET_ATSC, 0) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	return FUNCTION_SUCCESS;

error_status_set_demod_registers:
	return FUNCTION_ERROR;
}


/*
@see   ATSC_DEMOD_FP_TR_PHASE_INC_RESET
*/
int
realtek_r_atsc_TrPhaseInc_Reset(
	ATSC_DEMOD_MODULE *pDemod
)
{
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_TR_PHASE_INC, 0x2824282) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	return FUNCTION_SUCCESS;

error_status_set_demod_registers:
	return FUNCTION_ERROR;
}


/*
@see   ATSC_DEMOD_FP_INITIALIZE
*/
int
realtek_r_atsc_Initialize(
	ATSC_DEMOD_MODULE *pDemod
)
{
	typedef struct {
		unsigned short RegStartAddr;
		unsigned char Msb;
		unsigned char Lsb;
		U32BITS WritingValue;
	}
	INIT_REG_ENTRY;

	static const INIT_REG_ENTRY InitRegTable[] = {
		// CR2 and CCI disable for Bring Up
		// RegStartAddr,	Msb,	Lsb,	WritingValue
		{0xe41c,			0,		0,		0x0				},
		{0xd004,			0,		0,		0x1				},// opt_atsc_on
		{0xd002,			1,		0,		0x2				},// cr_cr2_en_sel, CR2 setting, start
		{0xd003,			3,		0,		0x6				},
		{0xde47,			7,		0,		0xfa			},// ori 0xea, cr_cr2_cr_offset_large_cnt_max (Dat0033)
		{0xde49,			2,		0,		0x2 			},// ori 0x7, cr_cr2_cr_offset_large_cnt_inc[2:0]
		{0xde4a,			3,		0,		0x6				},// ori 0x5, cr_cr2_cargp_wb
		{0xde50,		    4,		0,		0x0				},// ori 0x4, cr_cr2_cr_offset_large_cnt_inc[7:3]
		{0xde50,		   10,		5,		0x0				},// ori 0x3f, cr_fmu_cr2_nb_comp
		{0xde51,			8,		3,		0x0 			},// ori 0x3f, cr_pmu_cr2_nb_comp
		{0xde52,			6,		1,		0x0 			},// ori 0x1, cr_fmu_cr2_wb_comp
		{0xde53,			7,		5,		0x2 			},// ori 0x3, cr_cr2_carb_wb
		{0xde54,			5,		3,		0x0 			},// ori 0x1, cr_cr2_cargi_wb
		{0xde55,			6,		1,		0x20 			},// ori 0x21, cr_cr2_start_state
		{0xde5c,			4,		1,		0x0 			},// cr_cr2_swbw_cond_sel
		{0xde5c,			5,		5,		0x0 			},// ori 0x1, cr_cr2_swbw_ofa (CN), CR2 setting, end
		{0xd060,		   12,		2,		0x7ef 			},// ori 0x7f7, fs2_wparallel_size
		{0xd069,		    4,		4,		0x0 			},
		{0xd061,		   15,		5,		0x11 			},// ori 0x809, fs2_wparallel_size2
		{0xd069,		    5,		5,		0x1 			},	
		{0xd067,			9,		7,		0x4 			},// FS2 setting(1920-taps FFE), start
		{0xd066,			8,		6,		0x6 			},
		{0xd067,			3,		1,		0x4 			},
		{0xd067,			6,		4,		0x4 			},
		{0xd078,		   11,		0,		0x6fe 			},
		{0xd07b,		   11,		0,		0x6fe 			},
		{0xd07c,		   15,		4,		0x6fe 			},
		{0xd06a,		   10,		0,		0x3c0 			},
		{0xd06f,		   14,		4,		0x3c0 			},
		{0xd06c,		   16,		6,		0x3c0 			},
		{0xd06e,		   11,		1,		0x3c0 			},
		{0xdeb9,		   11,		0,		0xc5f 			},// ori 0xaf1, fs2_parallel_addr_th1
		{0xdebb,		   11,		0,		0xaf1 			},// ori 0xac6, fs2_parallel_addr_th2
		{0xdebd,		   11,		0,		0xac6 			},// ori 0x0, fs2_parallel_addr_th3
		{0xdebf,		   11,		0,		0x3a1 			},// ori 0x0, fs2_parallel_addr_th4
		{0xdec5,		   12,		0,		0x800 			},// ori 0x15e, fs2_parallel_th1(pre-echo)
		{0xdec7,		   12,		0,		0x197 			},// ori 0x1f4, fs2_parallel_th2
		{0xdec9,		   12,		0,		0x1f4 			},// ori 0x15e, fs2_parallel_th3(pre-echo)
		{0xdecb,		   12,		0,		0x197 			},// ori 0x0, fs2_parallel_th4
		{0xdecd,		   12,		0,		0x800 			},// ori 0x0, fs2_parallel_th5
		{0xdedb,		   13,		4,		0x1c2 			},
		{0xdede,		    9,		0,		0x1c2 			},
		{0xdedf,		   11,		2,		0x1c2 			},
		{0xdee9,		    8,		0,		0xae 			},
		{0xdeeb,		   10,		2,		0xae 			},
		{0xdeec,		   11,		3,		0xae 			},
		{0xdeee,		    8,		0,		0xb8 			},
		{0xdeef,		    9,		1,		0x118 			},
		{0xdef0,		   10,		2,		0xb8 			},
		{0xdef1,		   11,		3,		0xb8 			},// FS2 setting(1920-taps FFE), end
		{0xd173,			7,		5,		0x2				},// tr_fs_pll_kp_exp
		{0xde63,			8,		6,		0x4				},// ori 0x2, cci_iir_freq_offset, CCI setting, start
		{0xde73,		   26,		6,		0x1c60bf		},// ori 0x1C606A, cci_iir_a_1_m0
		{0xde76,		   23,		3,		0xffa2d 		},// ori 0xFFD16, cci_iir_a_2_m0
		{0xde79,		   20,		0,		0xffd16 		},// ori 0xFFE8B, cci_iir_b_0_m0
		{0xde7b,		   25,		5,		0x1c60bf 		},// ori 0x1C606A, cci_iir_b_1_m0
		{0xde7e,		   22,		2,		0xffd16 		},// ori 0xFFE8B, cci_iir_b_2_m0
		{0xde80,		   27,		7,		0x1c6167 		},// ori 0x1C60BF, cci_iir_a_1_m1
		{0xde83,		   24,		4,		0xff45c 		},// ori 0xFFA2D, cci_iir_a_2_m1
		{0xde86,		   21,		1,		0xffa2e 		},// ori 0xFFD16, cci_iir_b_0_m1
		{0xde88,		   26,		6,		0x1c6167 		},// ori 0x1C60BF, cci_iir_b_1_m1
		{0xde8b,		   23,		3,		0xffa2e 		},// ori 0xFFD16, cci_iir_b_2_m1	
		{0xdea8,			2,		2,		0x0				},// cci_rej_en
		{0xdea9,			9,		4,		0x20			},// ori 0x10, cci_det_fft_n (0x3f for R2.2)
		{0xdeac,		   16,		4,		0x1d52			},// ori 0x1aa6, cci_rej_freq_neg, CCI setting, end
		{0xd1e0,			4,		0,		0xa				},// FSM setting, start
		{0xd1e1,			5,		0,		0x19			},
		{0xd1e2,			6,		0,		0xb				},
		{0xd1e1,			7,		0,		0x2				},
		{0xd1e0,			7,		0,		0x7				},
		{0xd1e2,			7,		0,		0xca			},
		{0xd1e0,			7,		0,		0x8				},
		{0xd1e2,			7,		0,		0xd2			},
		{0xd1e1,			7,		0,		0x3				},
		{0xd1e0,			7,		0,		0x6				},
		{0xd1e2,			7,		0,		0xe6			},
		{0xd1e1,			7,		0,		0x5				},
		{0xd1e0,			7,		0,		0x1				},
		{0xd1e2,			7,		0,		0xc0			},
		{0xd1e0,			7,		0,		0x2				},
		{0xd1e2,			7,		0,		0xe1			},
		{0xd1e0,			7,		0,		0x3				},
		{0xd1e2,			7,		0,		0x88			},
		{0xd1e0,			7,		0,		0x8				},
		{0xd1e2,			7,		0,		0x9c			},
		{0xd1e0,			7,		0,		0xe				},
		{0xd1e2,			7,		0,		0xa2			},
		{0xd1e1,			7,		0,		0x7				},
		{0xd1e0,			7,		0,		0x6				},
		{0xd1e2,			7,		0,		0xc2			},
		{0xd1e1,			7,		0,		0x8				},
		{0xd1e0,			7,		0,		0x3				},
		{0xd1e2,			7,		0,		0xa1			},
		{0xd1e0,			7,		0,		0x4				},
		{0xd1e2,			7,		0,		0x90			},
		{0xd1e0,			7,		0,		0x5				},
		{0xd1e2,			7,		0,		0x84			},
		{0xd1e0,			7,		0,		0x6				},
		{0xd1e2,			7,		0,		0xd2			},
		{0xd1e0,			7,		0,		0x7				},
		{0xd1e2,			7,		0,		0xf0			},
		{0xd1e0,			7,		0,		0x8				},
		{0xd1e2,			7,		0,		0x9f			},
		{0xd1e0,			7,		0,		0x9				},
		{0xd1e2,			7,		0,		0xd0			},
		{0xd1e0,			7,		0,		0xa				},
		{0xd1e2,			7,		0,		0x8f			},
		{0xd1e0,			7,		0,		0xb				},
		{0xd1e2,			7,		0,		0xa8			},
		{0xd1e0,			7,		0,		0xc				},
		{0xd1e2,			7,		0,		0xf6			},
		{0xd1e0,			7,		0,		0xd				},
		{0xd1e2,			7,		0,		0xf3			},
		{0xd1e0,			7,		0,		0xe				},
		{0xd1e2,			7,		0,		0xca			},
		{0xd1e0,			7,		0,		0xf				},
		{0xd1e2,			7,		0,		0xba			},
		{0xd1e0,			7,		0,		0x10			},
		{0xd1e2,			7,		0,		0xe1			},
		{0xd1e0,			7,		0,		0x11			},
		{0xd1e2,			7,		0,		0xf0			},
		{0xd1e0,			7,		0,		0x12			},
		{0xd1e2,			7,		0,		0xf0			},
		{0xd1e0,			7,		0,		0x13			},
		{0xd1e2,			7,		0,		0x86			},
		{0xd1e1,			7,		0,		0xf				},
		{0xd1e0,			7,		0,		0x0				},
		{0xd1e2,			7,		0,		0xcf			},
		{0xd1e0,			7,		0,		0x1				},
		{0xd1e2,			7,		0,		0x87			},
		{0xd1e0,			7,		0,		0x2				},
		{0xd1e2,			7,		0,		0xa2			},
		{0xd1e0,			7,		0,		0x3				},
		{0xd1e2,			7,		0,		0x80			},
		{0xd1e0,			7,		0,		0x4				},
		{0xd1e2,			7,		0,		0xc1			},
		{0xd1e0,			7,		0,		0x7				},
		{0xd1e2,			7,		0,		0xff			},
		{0xd1e0,			7,		0,		0x8				},
		{0xd1e2,			7,		0,		0xc8			},
		{0xd1e1,			7,		0,		0x10			},
		{0xd1e0,			7,		0,		0x8				},
		{0xd1e2,			7,		0,		0x9e			},
		{0xd1e1,			7,		0,		0x12			},
		{0xd1e0,			7,		0,		0x2				},
		{0xd1e2,			7,		0,		0xa4			},
		{0xd1e0,			7,		0,		0x5				},
		{0xd1e2,			7,		0,		0xa4			},
		{0xd1e0,			7,		0,		0x6				},
		{0xd1e2,			7,		0,		0xd3			},
		{0xd1e0,			7,		0,		0x7				},
		{0xd1e2,			7,		0,		0xf3			},
		{0xd1e0,			7,		0,		0x8				},
		{0xd1e2,			7,		0,		0x9c			},
		{0xd1e0,			7,		0,		0x9				},
		{0xd1e2,			7,		0,		0xd0			},
		{0xd1e0,			7,		0,		0xa				},
		{0xd1e2,			7,		0,		0x8f			},
		{0xd1e0,			7,		0,		0xb				},
		{0xd1e2,			7,		0,		0xa8			},
		{0xd1e0,			7,		0,		0xd				},
		{0xd1e2,			7,		0,		0xc9			},
		{0xd1e0,			7,		0,		0xe				},
		{0xd1e2,			7,		0,		0xaa			},
		{0xd1e0,			7,		0,		0xf				},
		{0xd1e2,			7,		0,		0xa5			},
		{0xd1e0,			7,		0,		0x10			},
		{0xd1e2,			7,		0,		0xc1			},
		{0xd1e0,			7,		0,		0x11			},
		{0xd1e2,			7,		0,		0xe0			},
		{0xd1e0,			7,		0,		0x12			},
		{0xd1e2,			7,		0,		0xd1			},
		{0xd1e0,			7,		0,		0x14			},
		{0xd1e2,			7,		0,		0x80			},
		{0xd1e0,			7,		0,		0x15			},
		{0xd1e2,			7,		0,		0x80			},
		{0xd1e0,			7,		0,		0x16			},
		{0xd1e2,			7,		0,		0x80			},
		{0xd1e0,			7,		0,		0x17			},
		{0xd1e2,			7,		0,		0x80			},
		{0xd1e1,			7,		0,		0x13			},
		{0xd1e0,			7,		0,		0x1				},
		{0xd1e2,			7,		0,		0xe0			},
		{0xd1e0,			7,		0,		0x2				},
		{0xd1e2,			7,		0,		0x94			},
		{0xd1e0,			7,		0,		0x3				},
		{0xd1e2,			7,		0,		0x98			},
		{0xd1e0,			7,		0,		0x4				},
		{0xd1e2,			7,		0,		0x85			},
		{0xd1e0,			7,		0,		0x5				},
		{0xd1e2,			7,		0,		0x8c			},
		{0xd1e0,			7,		0,		0x6				},
		{0xd1e2,			7,		0,		0x86			},
		{0xd1e0,			7,		0,		0x7				},
		{0xd1e2,			7,		0,		0xac			},
		{0xd1e0,			7,		0,		0x8				},
		{0xd1e2,			7,		0,		0x88			},
		{0xd1e0,			7,		0,		0x9				},
		{0xd1e2,			7,		0,		0xd1			},
		{0xd1e0,			7,		0,		0xa				},
		{0xd1e2,			7,		0,		0x8f			},
		{0xd1e0,			7,		0,		0xb				},
		{0xd1e2,			7,		0,		0xa8			},
		{0xd1e0,			7,		0,		0xd				},
		{0xd1e2,			7,		0,		0xfa			},
		{0xd1e0,			7,		0,		0xe				},
		{0xd1e2,			7,		0,		0xaa			},
		{0xd1e0,			7,		0,		0xf				},
		{0xd1e2,			7,		0,		0xa5			},
		{0xd1e0,			7,		0,		0x10			},
		{0xd1e2,			7,		0,		0x81			},
		{0xd1e0,			7,		0,		0x11			},
		{0xd1e2,			7,		0,		0xf0			},
		{0xd1e0,			7,		0,		0x12			},
		{0xd1e2,			7,		0,		0x91			},
		{0xd1e0,			7,		0,		0x13			},
		{0xd1e2,			7,		0,		0x80			},
		{0xd1e0,			7,		0,		0x14			},
		{0xd1e2,			7,		0,		0x80			},
		{0xd1e0,			7,		0,		0x15			},
		{0xd1e2,			7,		0,		0x80			},
		{0xd1e0,			7,		0,		0x16			},
		{0xd1e2,			7,		0,		0x80			},
		{0xd1e0,			7,		0,		0x17			},
		{0xd1e2,			7,		0,		0x90			},
		{0xd1e1,			7,		0,		0x14			},
		{0xd1e0,			7,		0,		0x5				},
		{0xd1e2,			7,		0,		0xaa			},
		{0xd1e0,			7,		0,		0x6				},
		{0xd1e2,			7,		0,		0x87			},
		{0xd1e0,			7,		0,		0x7				},
		{0xd1e2,			7,		0,		0xf5			},
		{0xd1e1,			7,		0,		0x16			},
		{0xd1e0,			7,		0,		0x1				},
		{0xd1e2,			7,		0,		0xc7			},
		{0xd1e0,			7,		0,		0x9				},
		{0xd1e2,			7,		0,		0xd1			},
		{0xd1e0,			7,		0,		0xa				},
		{0xd1e2,			7,		0,		0xdf			},
		{0xd1e1,			7,		0,		0x17			},
		{0xd1e0,			7,		0,		0x17			},
		{0xd1e2,			7,		0,		0x81			},
		{0xd1e1,			7,		0,		0x18			},
		{0xd1e0,			7,		0,		0x17			},
		{0xd1e2,			7,		0,		0x81			},
		{0xd1e1,			7,		0,		0x19			},
		{0xd1e0,			7,		0,		0x1				},
		{0xd1e2,			7,		0,		0x8d			},
		{0xd1e0,			7,		0,		0x2				},
		{0xd1e2,			7,		0,		0xc5			},
		{0xd1e0,			7,		0,		0x3				},
		{0xd1e2,			7,		0,		0xea			},
		{0xd1e0,			7,		0,		0x5				},
		{0xd1e2,			7,		0,		0xa8			},
		{0xd1e0,			7,		0,		0x6				},
		{0xd1e2,			7,		0,		0xd4			},
		{0xd1e0,			7,		0,		0x7				},
		{0xd1e2,			7,		0,		0x93			},
		{0xd1e0,			7,		0,		0x8				},
		{0xd1e2,			7,		0,		0xa3			},
		{0xd1e0,			7,		0,		0x9				},
		{0xd1e2,			7,		0,		0xa0			},
		{0xd1e0,			7,		0,		0xa				},
		{0xd1e2,			7,		0,		0x8b			},
		{0xd1e0,			7,		0,		0x17			},
		{0xd1e2,			7,		0,		0x81			},
		{0xd1e1,			7,		0,		0x1a			},
		{0xd1e0,			7,		0,		0x17			},
		{0xd1e2,			7,		0,		0x81			},
		{0xd1e1,			7,		0,		0x1b			},
		{0xd1e0,			7,		0,		0x17			},
		{0xd1e2,			7,		0,		0x81			},
		{0xd1e1,			7,		0,		0x1c			},
		{0xd1e0,			7,		0,		0x17			},
		{0xd1e2,			7,		0,		0x81			},
		{0xd1e1,			7,		0,		0x1d			},
		{0xd1e0,			7,		0,		0x17			},
		{0xd1e2,			7,		0,		0x81			},
		{0xd1e1,			7,		0,		0x1e			},
		{0xd1e0,			7,		0,		0x5				},
		{0xd1e2,			7,		0,		0xc0			},
		{0xd1e0,			7,		0,		0x17			},
		{0xd1e2,			7,		0,		0x81			},
		{0xd1e1,			7,		0,		0x1f			},
		{0xd1e0,			7,		0,		0x6				},
		{0xd1e2,			7,		0,		0xe0			},
		{0xd1e0,			7,		0,		0x17			},
		{0xd1e2,			7,		0,		0x81			},
		{0xd1e1,			7,		0,		0x20			},// 20130430 alan add for MM stream
		{0xd1e0,			7,		0,		0xe 			},
		{0xd1e2,			7,		0,		0x83			},
		{0xd1e0,			7,		0,		0xf 			},
		{0xd1e2,			7,		0,		0xdb			},
		{0xd1e1,			7,		0,		0x21			},
		{0xd1e0,			7,		0,		0xe 			},
		{0xd1e2,			7,		0,		0x83			},
		{0xd1e0,			7,		0,		0xf 			},
		{0xd1e2,			7,		0,		0xdb			},// 20130430 alan add for MM stream
		{0xd1e1,			7,		0,		0x25			},
		{0xd1e0,			7,		0,		0x17			},
		{0xd1e2,			7,		0,		0x82			},
		{0xd1e1,			7,		0,		0x26			},
		{0xd1e0,			7,		0,		0x1				},
		{0xd1e2,			7,		0,		0xd3			},
		{0xd1e0,			7,		0,		0x3				},
		{0xd1e2,			7,		0,		0xa0			},
		{0xd1e0,			7,		0,		0x4				},
		{0xd1e2,			7,		0,		0x90			},
		{0xd1e0,			7,		0,		0x5				},
		{0xd1e2,			7,		0,		0x88			},
		{0xd1e0,			7,		0,		0x6				},
		{0xd1e2,			7,		0,		0xc4		    },
		{0xd1e0,			7,		0,		0x7				},
		{0xd1e2,			7,		0,		0xf3			},
		{0xd1e0,			7,		0,		0x8				},
		{0xd1e2,			7,		0,		0x84			},
		{0xd1e0,			7,		0,		0x9				},
		{0xd1e2,			7,		0,		0xe9			},
		{0xd1e0,			7,		0,		0xa				},
		{0xd1e2,			7,		0,		0x87			},
		{0xd1e0,			7,		0,		0xb				},
		{0xd1e2,			7,		0,		0xa8			},
		{0xd1e0,			7,		0,		0xc				},
		{0xd1e2,			7,		0,		0xd6			},
		{0xd1e0,			7,		0,		0xd				},
		{0xd1e2,			7,		0,		0xc9			},
		{0xd1e0,			7,		0,		0xe				},
		{0xd1e2,			7,		0,		0xaa			},
		{0xd1e0,			7,		0,		0xf				},
		{0xd1e2,			7,		0,		0xa5			},
		{0xd1e0,			7,		0,		0x10			},
		{0xd1e2,			7,		0,		0xc1			},
		{0xd1e0,			7,		0,		0x11			},
		{0xd1e2,			7,		0,		0xf0			},
		{0xd1e0,			7,		0,		0x12			},
		{0xd1e2,			7,		0,		0xd1			},
		{0xd1e0,			7,		0,		0x13			},
		{0xd1e2,			7,		0,		0x84			},
		{0xd1e0,			7,		0,		0x14			},
		{0xd1e2,			7,		0,		0x80			},
		{0xd1e0,			7,		0,		0x15			},
		{0xd1e2,			7,		0,		0x80			},
		{0xd1e0,			7,		0,		0x16			},
		{0xd1e2,			7,		0,		0x80			},
		{0xd1e0,			7,		0,		0x17			},
		{0xd1e2,			7,		0,		0x80			},
		{0xe41d,			0,		0,		0x0				},// 20120814 add for bist test
		{0xe421,			0,		0,		0x0				},
		{0xe41d,			0,		0,		0x1				},
		{0xe421,			0,		0,		0x1				},
		{0xe41a,			0,		0,		0x1				},
		{0xd1e1,			7, 		0,		0x10 			},// A74 R2.1/R2.2 Patch use by FSM
		{0xd1e0,			7, 		0,		0x3  			},// A74 R2.1/R2.2 Patch use by FSM
		{0xd1e2,			7, 		0,		0x9a			},// A74 R2.1/R2.2 Patch use by FSM
		{0xd1e0,			7, 		0,		0x4  			},// A74 R2.1/R2.2 Patch use by FSM
		{0xd1e2,			7, 		0,		0x8e			},// A74 R2.1/R2.2 Patch use by FSM
		{0xd1e0,			7, 		0,		0x5  			},// A74 R2.1/R2.2 Patch use by FSM
		{0xd1e2,			7, 		0,		0xa3			},// A74 R2.1/R2.2 Patch use by FSM
		{0xd1e0,			7, 		0,		0x6  			},// A74 R2.1/R2.2 Patch use by FSM
		{0xd1e2,			7, 		0,		0x8b			},// A74 R2.1/R2.2 Patch use by FSM
		{0xd1e0,			7, 		0,		0x7  			},// A74 R2.1/R2.2 Patch use by FSM
		{0xd1e2,			7, 		0,		0xc6			},// A74 R2.1/R2.2 Patch use by FSM
		{0xd1e0,			7, 		0,		0x8  			},// A74 R2.1/R2.2 Patch use by FSM
		{0xd1e2,			7, 		0,		0x9c			},// A74 R2.1/R2.2 Patch use by FSM
		{0xd1e1,			7, 		0,		0x0				},// Pull in range +-400KHz/Locking time for LGE, start
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xaa			},
		{0xd1e1,			7, 		0,		0x1				},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xaa			},
		{0xd1e1,			7, 		0,		0x2				},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xaa			},
		{0xd1e1,			7, 		0,		0x3				},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xaa			},
		{0xd1e1,			7, 		0,		0x4				},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xaa			},
		{0xd1e1,			7, 		0,		0x5				},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xaa			},
		{0xd1e1,			7, 		0,		0x6				},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xd3			},
		{0xd1e1,			7, 		0,		0x7				},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xaa			},
		{0xd1e1,			7, 		0,		0x8				},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0x90			},
		{0xd1e1,			7, 		0,		0x9				},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0xa				},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0xb				},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0xc				},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0xd				},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0xe				},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0xf				},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0x10			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0x11			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0x12			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0x93			},
		{0xd1e1,			7, 		0,		0x13			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0x93			},
		{0xd1e1,			7, 		0,		0x14			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0x15			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0x16			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0x17			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0x18			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0x19			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0x1a			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0x1b			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0x1c			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0x1d			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0x1e			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0x1f			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0x20			},// st32,33,34, fsm_cr_fmu/fsm_cr_pmu = [3,3,3]/[5,5,3](static echoes), start
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xcf			},
		{0xd1e0,			7, 		0,		0xf				},
		{0xd1e2,			7, 		0,		0xda			},		
		{0xd1e1,			7, 		0,		0x21			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xcf			},
		{0xd1e0,			7, 		0,		0xf				},
		{0xd1e2,			7, 		0,		0xda			},		
		{0xd1e1,			7, 		0,		0x22			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xcf			},
		{0xd1e0,			7, 		0,		0xf				},
		{0xd1e2,			7, 		0,		0xd9			},// st32,33,34, fsm_cr_fmu/fsm_cr_pmu = [3,3,3]/[5,5,3](static echoes), end
		{0xd1e1,			7, 		0,		0x23			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0x24			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0x25			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},
		{0xd1e1,			7, 		0,		0x26			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0x93			},
		{0xd1e1,			7, 		0,		0x27			},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xda			},// Pull in range +-400KHz/Locking time for LGE, end
		{0xd1e1,			7, 		0,		0x1				},// TR1_3_b_plus, start
		{0xd1e0,			7, 		0,		0x9				},
		{0xd1e2,			7, 		0,		0xe5			},
		{0xd1e0,			7, 		0,		0xa				},
		{0xd1e2,			7, 		0,		0x80			},
		{0xd1e1,			7, 		0,		0x6				},
		{0xd1e0,			7, 		0,		0xe				},
		{0xd1e2,			7, 		0,		0xdb			},
		{0xd1e1,			7, 		0,		0x9				},
		{0xd1e0,			7, 		0,		0x9				},
		{0xd1e2,			7, 		0,		0xfd			},
		{0xd1e0,			7, 		0,		0xa				},
		{0xd1e2,			7, 		0,		0x87			},
		{0xd1e1,			7, 		0,		0xf				},
		{0xd1e0,			7, 		0,		0x2				},
		{0xd1e2,			7, 		0,		0x82			},
		{0xd1e0,			7, 		0,		0x3				},
		{0xd1e2,			7, 		0,		0x82			},
		{0xd1e0,			7, 		0,		0x6				},
		{0xd1e2,			7, 		0,		0xd0			},
		{0xd1e0,			7, 		0,		0x7				},
		{0xd1e2,			7, 		0,		0xf3			},
		{0xd1e0,			7, 		0,		0x8				},
		{0xd1e2,			7, 		0,		0xc8			},
		{0xd1e0,			7, 		0,		0x9				},
		{0xd1e2,			7, 		0,		0xfd			},
		{0xd1e0,			7, 		0,		0xa				},
		{0xd1e2,			7, 		0,		0x87			},
		{0xd1e1,			7, 		0,		0x10			},
		{0xd1e0,			7, 		0,		0x6				},
		{0xd1e2,			7, 		0,		0xe3			},
		{0xd1e0,			7, 		0,		0x7				},
		{0xd1e2,			7, 		0,		0xf3			},
		{0xd1e0,			7, 		0,		0x8				},
		{0xd1e2,			7, 		0,		0x9c			},
		{0xd1e0,			7, 		0,		0x9				},
		{0xd1e2,			7, 		0,		0xa9			},
		{0xd1e0,			7, 		0,		0xa				},
		{0xd1e2,			7, 		0,		0x80			},
		{0xd1e0,			7, 		0,		0x11			},
		{0xd1e2,			7, 		0,		0xf0			},
		{0xd1e1,			7, 		0,		0x14			},
		{0xd1e0,			7, 		0,		0x6				},
		{0xd1e2,			7, 		0,		0x95			},
		{0xd1e1,			7, 		0,		0x18			},
		{0xd1e0,			7, 		0,		0x14			},
		{0xd1e2,			7, 		0,		0x88			},
		{0xd1e0,			7, 		0,		0x16			},
		{0xd1e2,			7, 		0,		0x83			},
		{0xd1e1,			7, 		0,		0x19			},
		{0xd1e0,			7, 		0,		0x9 			},// cyclic training refine, start
		{0xd1e2,			7, 		0,		0x80			},
		{0xd1e0,			7, 		0,		0xa				},
		{0xd1e2,			7, 		0,		0x8f			},// cyclic training refine, end
		{0xd1e0,			7, 		0,		0x16			},
		{0xd1e2,			7, 		0,		0x83			},
		{0xd1e1,			7, 		0,		0x1a			},
		{0xd1e0,			7, 		0,		0x9 			},
		{0xd1e2,			7, 		0,		0x84			},
		{0xd1e0,			7, 		0,		0xa				},
		{0xd1e2,			7, 		0,		0x80			},
		{0xd1e1,			7, 		0,		0x1b			},
		{0xd1e0,			7, 		0,		0x9 			},
		{0xd1e2,			7, 		0,		0xfc			},
		{0xd1e0,			7, 		0,		0xa				},
		{0xd1e2,			7, 		0,		0x8a			},
		{0xd1e1,			7, 		0,		0x1d			},
		{0xd1e0,			7, 		0,		0x9 			},
		{0xd1e2,			7, 		0,		0x80			},
		{0xd1e0,			7, 		0,		0xa				},
		{0xd1e2,			7, 		0,		0x8d			},
		{0xd1e0,			7, 		0,		0xb				},
		{0xd1e2,			7, 		0,		0xa8			},
		{0xd1e0,			7, 		0,		0x15			},
		{0xd1e2,			7, 		0,		0xe2			},
		{0xd1e1,			7, 		0,		0x1e			},
		{0xd1e0,			7, 		0,		0x9 			},// cyclic training refine, start
		{0xd1e2,			7, 		0,		0x80			},
		{0xd1e0,			7, 		0,		0xa				},
		{0xd1e2,			7, 		0,		0x8b			},// cyclic training refine, end
		{0xd1e0,			7, 		0,		0xb 			},
		{0xd1e2,			7, 		0,		0xaa			},
		{0xd1e1,			7, 		0,		0x24			},
		{0xd1e0,			7, 		0,		0x17			},
		{0xd1e2,			7, 		0,		0x98			},// TR1_3_b_plus, end
	};

	int i;
	unsigned short RegStartAddr;
	unsigned char Msb;
	unsigned char Lsb;
	U32BITS WritingValue;

	// Initialize demod with register initializing table.
	for (i = 0; i < (sizeof(InitRegTable) / sizeof(INIT_REG_ENTRY)); i++) {
		// Get all information from each register initializing entry.
		RegStartAddr = InitRegTable[i].RegStartAddr;
		Msb          = InitRegTable[i].Msb;
		Lsb          = InitRegTable[i].Lsb;
		WritingValue = InitRegTable[i].WritingValue;

		// Set register mask bits.
		if (pDemod->RegAccess.Addr16Bit.SetRegMaskBits(pDemod, RegStartAddr, Msb, Lsb, WritingValue) != FUNCTION_SUCCESS)
			goto error_status_set_demod_registers;
	}

#ifdef REALTEK_R_ATSC_8051FW_PATCH
	// Download REALTEK_R ATSC 8051 firmware code to demod
	if (realtek_r_atsc_DownloadFirmwareCode(pDemod) != FUNCTION_SUCCESS)
		goto error_status_execute_function;
#endif

	return FUNCTION_SUCCESS;

error_status_set_demod_registers:
error_status_execute_function:
	return FUNCTION_ERROR;
}


/*
@see   ATSC_DEMOD_FP_SET_IF_FREQ_HZ
*/
int
realtek_r_atsc_SetIfFreqHz(
	ATSC_DEMOD_MODULE *pDemod,
	U32BITS IfFreqHz
)
{
	U32BITS CrystalFreqHz = 0;
	S32BITS AdjustedIfFreqHz = 0;
	MPI MpiAdjustedIfFreqHz, MpiCrystalFreqHz, MpiConst, MpiVar, MpiNone;
	S32BITS DdcPhaseInc = 0;

	memset(&MpiAdjustedIfFreqHz, 0 , sizeof(MpiAdjustedIfFreqHz));
	memset(&MpiCrystalFreqHz, 0 , sizeof(MpiCrystalFreqHz));
	memset(&MpiConst, 0 , sizeof(MpiConst));
	memset(&MpiVar, 0 , sizeof(MpiVar));
	memset(&MpiNone, 0 , sizeof(MpiNone));

	// Get demod crystal frequency in Hz.
	pDemod->GetCrystalFreqHz(pDemod, &CrystalFreqHz);

	// Get adjusted IF frequency.
	// Note: Step 1. AdjustedIfFreqHz = IfFreqHz - CrystalFreqHz * 2 * N
	//               a. N is an integer and N >= 0
	//               b. (- CrystalFreqHz) < AdjustedIfFreqHz <= CrystalFreqHz
	//       Step 2. AdjustedIfFreqHz = abs(AdjustedIfFreqHz)
	AdjustedIfFreqHz = IfFreqHz;

	while (AdjustedIfFreqHz > (S32BITS)CrystalFreqHz)
		AdjustedIfFreqHz -= CrystalFreqHz * 2;

	if (AdjustedIfFreqHz < 0)
		AdjustedIfFreqHz = - AdjustedIfFreqHz;

	// Calculate DDC_PHASE_INC value.
	// Note: Original formula:   DDC_PHASE_INC = round( (AdjustedIfFreqHz / CrystalFreqHz) * pow(2, 19) )
	//       Adjusted formula:   DDC_PHASE_INC = floor( ( (AdjustedIfFreqHz << 20) / CrystalFreqHz + 1 ) >> 1)
	MpiSetValue(&MpiCrystalFreqHz, (S32BITS)CrystalFreqHz);
	MpiSetValue(&MpiAdjustedIfFreqHz, (S32BITS)AdjustedIfFreqHz);
	MpiSetValue(&MpiConst, 1);
	MpiLeftShift(&MpiVar, MpiAdjustedIfFreqHz, 20);
	MpiDiv(&MpiVar, &MpiNone, MpiVar, MpiCrystalFreqHz);
	MpiAdd(&MpiVar, MpiVar, MpiConst);
	MpiRightShift(&MpiVar, MpiVar, 1);
	MpiGetValue(MpiVar, &DdcPhaseInc);

	// Set DDC_PHASE_INC with calculated value.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_DDC_PHASE_INC, DdcPhaseInc) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	// Set demod IF frequnecy parameter.
	pDemod->IfFreqHz      = IfFreqHz;
	pDemod->IsIfFreqHzSet = YES;

	return FUNCTION_SUCCESS;

error_status_set_demod_registers:
	return FUNCTION_ERROR;
}


/*
@see   ATSC_DEMOD_FP_SET_SPECTRUM_MODE
*/
int
realtek_r_atsc_SetSpectrumMode(
	ATSC_DEMOD_MODULE *pDemod,
	int SpectrumMode
)
{
	U32BITS SpecInvOn;

	// Determine SPEC_INV_ON according to spectrum mode.
	switch (SpectrumMode) {
	case SPECTRUM_NORMAL:
		SpecInvOn = 0x0;
		break;

	default:
	case SPECTRUM_INVERSE:
		SpecInvOn = 0x1;
		break;
	}

	// Set SPEC_INV_ON with determined value.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_SPEC_INV_ON, SpecInvOn) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	// Set demod spectrum mode parameter.
	pDemod->SpectrumMode      = SpectrumMode;
	pDemod->IsSpectrumModeSet = YES;

	return FUNCTION_SUCCESS;

error_status_set_demod_registers:
	return FUNCTION_ERROR;
}


/*
@see   ATSC_DEMOD_FP_GET_RF_AGC
*/
int
realtek_r_atsc_GetRfAgc(
	ATSC_DEMOD_MODULE *pDemod,
	S32BITS *pRfAgc
)
{
	U32BITS RfAgcBinary;

	// Set RO_STROBE with 0x1, then set RO_STROBE with 0x0.
	// Note: RO_STROBE is a write-only register.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	// Get RF AGC binary value from RO_RF_AAGC.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_RF_AAGC, &RfAgcBinary) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Convert RF AGC binary value to signed integer.
	*pRfAgc = BinToSignedInt((U32BITS)RfAgcBinary, REALTEK_R_ATSC_RF_AGC_VALUE_BIT_NUM);

	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
error_status_set_demod_registers:
	return FUNCTION_ERROR;
}


/*
@see   ATSC_DEMOD_FP_GET_IF_AGC
*/
int
realtek_r_atsc_GetIfAgc(
	ATSC_DEMOD_MODULE *pDemod,
	S32BITS *pIfAgc
)
{
	U32BITS IfAgcBinary;

	// Set RO_STROBE with 0x1, then set RO_STROBE with 0x0.
	// Note: RO_STROBE is a write-only register.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	// Get IF AGC binary value from RO_IF_AAGC.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_IF_AAGC, &IfAgcBinary) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Convert IF AGC binary value to signed integer.
	*pIfAgc = BinToSignedInt((U32BITS)IfAgcBinary, REALTEK_R_ATSC_IF_AGC_VALUE_BIT_NUM);

	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
error_status_set_demod_registers:
	return FUNCTION_ERROR;
}


/*
@see   ATSC_DEMOD_FP_GET_DI_AGC
*/
int
realtek_r_atsc_GetDiAgc(
	ATSC_DEMOD_MODULE *pDemod,
	U32BITS *pDiAgc
)
{
	// Set RO_STROBE with 0x1, then set RO_STROBE with 0x0.
	// Note: RO_STROBE is a write-only register.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	// Get digital AGC value from RO_DAGC_LEVEL_Q.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_DAGC_LEVEL_Q, pDiAgc) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
error_status_set_demod_registers:
	return FUNCTION_ERROR;
}


/*
@see   ATSC_DEMOD_FP_GET_TR_OFFSET_PPM
*/
int
realtek_r_atsc_GetTrOffsetPpm(
	ATSC_DEMOD_MODULE *pDemod,
	S32BITS *pTrOffsetPpm
)
{
	U32BITS SymbolRateHz = 0;
	U32BITS CrystalFreqHz = 0;
	U32BITS TrOffsetN1Binary = 0;
	U32BITS TrOffsetN2Binary = 0;
	S32BITS TrOffsetN1Int = 0;
	S32BITS          TrOffsetN2Int = 0;
	MPI MpiSymbolRateHz, MpiCrystalFreqHz, MpiTrOffsetN1Int, MpiTrOffsetN2Int;
	MPI MpiVarN1, MpiVarN2;
	MPI MpiConst, MpiVar0, MpiVar1, MpiNone;

	memset(&MpiSymbolRateHz, 0 , sizeof(MpiSymbolRateHz));
	memset(&MpiCrystalFreqHz, 0 , sizeof(MpiCrystalFreqHz));
	memset(&MpiTrOffsetN1Int, 0 , sizeof(MpiTrOffsetN1Int));
	memset(&MpiTrOffsetN2Int, 0 , sizeof(MpiTrOffsetN2Int));
	memset(&MpiVarN1, 0 , sizeof(MpiVarN1));
	memset(&MpiVarN2, 0 , sizeof(MpiVarN2));
	memset(&MpiConst, 0 , sizeof(MpiConst));
	memset(&MpiVar0, 0 , sizeof(MpiVar0));
	memset(&MpiVar1, 0 , sizeof(MpiVar1));
	memset(&MpiNone, 0 , sizeof(MpiNone));

	// Get demod symbol rate in Hz.
	SymbolRateHz = REALTEK_R_ATSC_SYMBOL_RATE_HZ;

	// Get demod crystal frequency in Hz.
	pDemod->GetCrystalFreqHz(pDemod, &CrystalFreqHz);

	// Set RO_STROBE with 0x1, then set RO_STROBE with 0x0.
	// Note: RO_STROBE is a write-only register.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	// Get TR offset N1 binary value from RO_TR_OFFSET_N1 register bits.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_TR_OFFSET_N1, &TrOffsetN1Binary) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Convert TR offset N1 binary value to signed integer.
	TrOffsetN1Int = BinToSignedInt((U32BITS)TrOffsetN1Binary, REALTEK_R_ATSC_TR_OFFSET_N1_BIT_NUM);

	// Get TR offset binary N2 value from RO_TR_OFFSET_N2 register bits.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_TR_OFFSET_N2, &TrOffsetN2Binary) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Convert TR offset N2 binary value to signed integer.
	TrOffsetN2Int = BinToSignedInt((U32BITS)TrOffsetN2Binary, REALTEK_R_ATSC_TR_OFFSET_N2_BIT_NUM);

	// Get TR offset in ppm.
	// Note: Original formula:   TrOffsetPpm = TrOffsetN1 - TrOffsetN2
	//                           TrOffsetN1  = (TrOffsetN1Int * SymbolRateHz * 1000000) / (CrystalFreqHz * pow(2, 19))
	//                           TrOffsetN2  = (TrOffsetN2Int * SymbolRateHz * 1000000) / (CrystalFreqHz * pow(2, 23))
	//       Adjusted formula:   TrOffsetPpm = TrOffsetN1 - TrOffsetN2
	//                           TrOffsetN1  = (TrOffsetN1Int * SymbolRateHz * 1000000) / (CrystalFreqHz << 19)
	//                           TrOffsetN2  = (TrOffsetN2Int * SymbolRateHz * 1000000) / (CrystalFreqHz << 23)
	MpiSetValue(&MpiSymbolRateHz, (S32BITS)SymbolRateHz);
	MpiSetValue(&MpiCrystalFreqHz, (S32BITS)CrystalFreqHz);
	MpiSetValue(&MpiTrOffsetN1Int, (S32BITS)TrOffsetN1Int);
	MpiSetValue(&MpiTrOffsetN2Int, (S32BITS)TrOffsetN2Int);
	MpiSetValue(&MpiConst, 1000000);
	MpiMul(&MpiVar0, MpiTrOffsetN1Int, MpiSymbolRateHz);
	MpiMul(&MpiVar0, MpiVar0, MpiConst);
	MpiLeftShift(&MpiVar1, MpiCrystalFreqHz, 19);
	MpiDiv(&MpiVarN1, &MpiNone, MpiVar0, MpiVar1);
	MpiMul(&MpiVar0, MpiTrOffsetN2Int, MpiSymbolRateHz);
	MpiMul(&MpiVar0, MpiVar0, MpiConst);
	MpiLeftShift(&MpiVar1, MpiCrystalFreqHz, 23);
	MpiDiv(&MpiVarN2, &MpiNone, MpiVar0, MpiVar1);
	MpiSub(&MpiVar0, MpiVarN1, MpiVarN2);
	MpiGetValue(MpiVar0, (S32BITS*)pTrOffsetPpm);

	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
error_status_set_demod_registers:
	return FUNCTION_ERROR;
}


/*
deltaf_ratio: 0xd6c0[11:5], set to 0@init.
ro_timing_offset_d_q: u(16,24f), 0xdf12[15:0]
ro_tr_fs_tr_freq : s(24,25f), 0xdf47[23:0]
tr_phase_inc: u(27,25f), 0xd165[26:0]
*/
int
realtek_r_atsc_GetTrFsFreqSubOffsetPpm(
	ATSC_DEMOD_MODULE *pDemod,
	S32BITS *pTrOffsetPpm
)
{
	U32BITS SymbolRateHz = 0;
	U32BITS CrystalFreqHz = 0;
	U32BITS TrOffsetN2Binary = 0;
	U32BITS TrFsFreqBinary = 0;
	U32BITS TrPhaseIncBinary, TrPhaseIncBinary_Final = 0;
	S32BITS TrOffsetN2Int = 0, TrOffset = 0;
	S32BITS TrFsFreqInt = 0, TrFsFreq = 0;
	S32BITS TrPhaseIncInt = 0, TrPhaseInc, TrPhaseInc_set = 0;
	//S32BITS debug_value = 0;
    unsigned int test_thd = 60;

	MPI MpiSymbolRateHz, MpiCrystalFreqHz, MpiTrOffsetN2Int, MpiTrFsFreqInt, MpiTrPhaseIncInt;
	MPI MpiVar_TrOffset, MpiVar_TrFsFreq, MpiVarN2;
	MPI MpiConst, MpiVar0, MpiVar1, MpiVar2, MpiNone;
	MPI MpiConst70, MpiConstN1, MpiConstP1;

	memset(&MpiSymbolRateHz, 0 , sizeof(MpiSymbolRateHz));
	memset(&MpiCrystalFreqHz, 0 , sizeof(MpiCrystalFreqHz));
	memset(&MpiTrOffsetN2Int, 0 , sizeof(MpiTrOffsetN2Int));
	memset(&MpiTrFsFreqInt, 0 , sizeof(MpiTrFsFreqInt));
	memset(&MpiTrPhaseIncInt, 0 , sizeof(MpiTrPhaseIncInt));
	memset(&MpiVar_TrOffset, 0 , sizeof(MpiVar_TrOffset));
	memset(&MpiVar_TrFsFreq, 0 , sizeof(MpiVar_TrFsFreq));
	memset(&MpiVarN2, 0 , sizeof(MpiVarN2));
	memset(&MpiConst, 0 , sizeof(MpiConst));
	memset(&MpiVar0, 0 , sizeof(MpiVar0));
	memset(&MpiVar1, 0 , sizeof(MpiVar1));
	memset(&MpiVar2, 0 , sizeof(MpiVar2));
	memset(&MpiNone, 0 , sizeof(MpiNone));
	memset(&MpiConst70, 0 , sizeof(MpiConst70));
	memset(&MpiConstN1, 0 , sizeof(MpiConstN1));
	memset(&MpiConstP1, 0 , sizeof(MpiConstP1));

	// Get demod symbol rate in Hz.
	SymbolRateHz = REALTEK_R_ATSC_SYMBOL_RATE_HZ;

	// Get demod crystal frequency in Hz.
	pDemod->GetCrystalFreqHz(pDemod, &CrystalFreqHz);

	// Set RO_STROBE with 0x1, then set RO_STROBE with 0x0.
	// Note: RO_STROBE is a write-only register.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	// Get TR offset binary value from RO_TR_OFFSET_N2 register bits.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_TR_OFFSET_N2, &TrOffsetN2Binary) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Convert TR offset binary value to signed integer.
	TrOffsetN2Int = BinToSignedInt((U32BITS)TrOffsetN2Binary, 16);

	// Get TR FS Freq binary value from RO_TR_FS_FREQ register bits.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_TR_FS_FREQ, &TrFsFreqBinary) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Convert TR offset binary value to signed integer.
	TrFsFreqInt = BinToSignedInt((U32BITS)TrFsFreqBinary, 24);

	// Get TR FS Freq binary value from TR_PHASE_INC register bits.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_TR_PHASE_INC, &TrPhaseIncBinary) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Convert TR pahse inc binary value to signed integer.
	TrPhaseIncInt = BinToSignedInt((U32BITS)TrPhaseIncBinary, 27);

	MpiSetValue(&MpiSymbolRateHz, (S32BITS)SymbolRateHz);
	MpiSetValue(&MpiCrystalFreqHz, (S32BITS)CrystalFreqHz);
	MpiSetValue(&MpiTrOffsetN2Int, (S32BITS)TrOffsetN2Int);
	MpiSetValue(&MpiTrFsFreqInt, (S32BITS)TrFsFreqInt);
	MpiSetValue(&MpiTrPhaseIncInt, (S32BITS)TrPhaseIncInt);
	MpiSetValue(&MpiConst,         1000000);

	//Cal Tr Offset
	MpiMul(&MpiVar0, MpiTrOffsetN2Int, MpiSymbolRateHz);
	MpiMul(&MpiVar0, MpiVar0, MpiConst);
	MpiLeftShift(&MpiVar1, MpiCrystalFreqHz, 23);
	MpiDiv(&MpiVarN2, &MpiNone, MpiVar0, MpiVar1);
	MpiGetValue(MpiVarN2, &TrOffset);

	//Cal Tr Fs Freq
	MpiMul(&MpiVar0, MpiTrFsFreqInt, MpiSymbolRateHz);
	MpiMul(&MpiVar0, MpiVar0, MpiConst);
	MpiLeftShift(&MpiVar1, MpiCrystalFreqHz, 24);
	MpiDiv(&MpiVarN2, &MpiNone, MpiVar0, MpiVar1);
	MpiGetValue(MpiVarN2, &TrFsFreq);

	//Cal Tr Pahse inc
	MpiMul(&MpiVar0, MpiTrPhaseIncInt, MpiSymbolRateHz);
	MpiMul(&MpiVar0, MpiVar0, MpiConst);
	MpiLeftShift(&MpiVar1, MpiCrystalFreqHz, 24);
	MpiDiv(&MpiVarN2, &MpiNone, MpiVar0, MpiVar1);
	MpiGetValue(MpiVarN2, &TrPhaseInc);

	if (TrOffset == 0) {
		return FUNCTION_ERROR;
	}
	
	if (abs(TrFsFreq - TrOffset) < test_thd) {
		if (TrOffset < 0)
			TrPhaseInc_set = -1 * (test_thd - abs(TrFsFreq - TrOffset));
		else
			TrPhaseInc_set = (test_thd - abs(TrFsFreq - TrOffset));


		MpiSetValue(&MpiTrPhaseIncInt, TrPhaseInc_set);
		MpiMul(&MpiVar0, MpiConst, MpiSymbolRateHz);
		MpiLeftShift(&MpiVar1, MpiCrystalFreqHz, 24);
		MpiDiv(&MpiVar2, &MpiNone, MpiVar1, MpiVar0);
		MpiMul(&MpiVar0, MpiTrPhaseIncInt, MpiVar2);
		MpiGetValue(MpiVar0, &TrPhaseIncBinary_Final);
		TrPhaseIncBinary_Final = TrPhaseIncBinary + TrPhaseIncBinary_Final;
		// Set ATSC_TR_PHASE_INC with calculated value.
		if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_TR_PHASE_INC, TrPhaseIncBinary_Final) != FUNCTION_SUCCESS)
			goto error_status_set_demod_registers;

		realtek_r_atsc_SoftwareReset(pDemod);
	}

	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
error_status_set_demod_registers:
	return FUNCTION_ERROR;
}


/*
Get TR offset, TR FS Freqency, and fbf_coeff_sum_h
then switch TR loop2 or loop3 for Single Static Echoes and R2.1/R2.2
*/
int
realtek_r_atsc_GetTrLoopSel(
	ATSC_DEMOD_MODULE *pDemod,
	S32BITS *pTrLoopSel
)
{
	U32BITS SymbolRateHz = 0;
	U32BITS CrystalFreqHz = 0;
	U32BITS TrOffsetN2Binary = 0;
	U32BITS TrFsFreqBinary = 0;
	U32BITS FbfCoeffSumHBinary = 0;
	U32BITS num_fbf_coeff_1 = 0, num_fbf_coeff_2 = 0, num_fbf_coeff_3 = 0;
	S32BITS TrOffsetN2Int = 0, TrOffset = 0;
	S32BITS TrFsFreqInt = 0, TrFsFreq = 0;
	S32BITS FbfCoeffSumHInt = 0;
	//S32BITS FbfCoeffSumH = 0;
	S32BITS FbfCoeffSumH_tmp1 = 0, FbfCoeffSumH_tmp2 = 0, FbfCoeffSumH_tmp3 = 0, FbfCoeffSumH_tmp2_3 = 0;
	//S32BITS FbfCoeffSumH_tmp = 0;
	//S32BITS FbfCoeffSumH_Final = 0;

	MPI MpiSymbolRateHz, MpiCrystalFreqHz, MpiTrOffsetN2Int, MpiTrFsFreqInt, MpiTrLoopSelInt;
	MPI MpiVarN2, MpiConst, MpiVar0, MpiVar1, MpiNone;

	memset(&MpiSymbolRateHz, 0 , sizeof(MpiSymbolRateHz));
	memset(&MpiCrystalFreqHz, 0 , sizeof(MpiCrystalFreqHz));
	memset(&MpiTrOffsetN2Int, 0 , sizeof(MpiTrOffsetN2Int));
	memset(&MpiTrFsFreqInt, 0 , sizeof(MpiTrFsFreqInt));
	memset(&MpiTrLoopSelInt, 0 , sizeof(MpiTrLoopSelInt));
	memset(&MpiVarN2, 0 , sizeof(MpiVarN2));
	memset(&MpiConst, 0 , sizeof(MpiConst));
	memset(&MpiVar0, 0 , sizeof(MpiVar0));
	memset(&MpiVar1, 0 , sizeof(MpiVar1));
	memset(&MpiNone, 0 , sizeof(MpiNone));

	// Get demod symbol rate in Hz.
	SymbolRateHz = REALTEK_R_ATSC_SYMBOL_RATE_HZ;

	// Get demod crystal frequency in Hz.
	pDemod->GetCrystalFreqHz(pDemod, &CrystalFreqHz);

	// Set RO_STROBE with 0x1, then set RO_STROBE with 0x0.
	// Note: RO_STROBE is a write-only register.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	// Get TR offset binary value from RO_TR_OFFSET_N2 register bits.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_TR_OFFSET_N2, &TrOffsetN2Binary) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Convert TR offset binary value to signed integer.
	TrOffsetN2Int = BinToSignedInt((U32BITS)TrOffsetN2Binary, 16);

	// Get TR FS Freq binary value from RO_TR_FS_FREQ register bits.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_TR_FS_FREQ, &TrFsFreqBinary) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Convert TR offset binary value to signed integer.
	TrFsFreqInt = BinToSignedInt((U32BITS)TrFsFreqBinary, 24);


	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FBF_RO_SUM_ADD, 0xb) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	for (num_fbf_coeff_1 = 0; num_fbf_coeff_1 < 9; num_fbf_coeff_1++) {
		// Get TR offset binary value from RO_TR_OFFSET_N2 register bits.
		if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_FBF_COEFF_SUM_H, &FbfCoeffSumHBinary) != FUNCTION_SUCCESS)
			goto error_status_get_demod_registers;

		// Convert TR offset binary value to signed integer.
		FbfCoeffSumHInt = BinToSignedInt((U32BITS)FbfCoeffSumHBinary, 8);
		FbfCoeffSumH_tmp1 = FbfCoeffSumH_tmp1 + FbfCoeffSumHInt;


		tv_osal_msleep(5);
	}

	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FBF_RO_SUM_ADD, 0x47) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	for (num_fbf_coeff_2 = 0; num_fbf_coeff_2 < 9; num_fbf_coeff_2++) {
		// Get TR offset binary value from RO_TR_OFFSET_N2 register bits.
		if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_FBF_COEFF_SUM_H, &FbfCoeffSumHBinary) != FUNCTION_SUCCESS)
			goto error_status_get_demod_registers;

		// Convert TR offset binary value to signed integer.
		FbfCoeffSumHInt = BinToSignedInt((U32BITS)FbfCoeffSumHBinary, 8);
		FbfCoeffSumH_tmp2 = FbfCoeffSumH_tmp2 + FbfCoeffSumHInt;


		tv_osal_msleep(5);
	}

	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FBF_RO_SUM_ADD, 0x51) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	for (num_fbf_coeff_3 = 0; num_fbf_coeff_3 < 9; num_fbf_coeff_3++) {
		// Get TR offset binary value from RO_TR_OFFSET_N2 register bits.
		if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_FBF_COEFF_SUM_H, &FbfCoeffSumHBinary) != FUNCTION_SUCCESS)
			goto error_status_get_demod_registers;

		// Convert TR offset binary value to signed integer.
		FbfCoeffSumHInt = BinToSignedInt((U32BITS)FbfCoeffSumHBinary, 8);
		FbfCoeffSumH_tmp3 = FbfCoeffSumH_tmp3 + FbfCoeffSumHInt;


		tv_osal_msleep(5);
	}

	FbfCoeffSumH_tmp2_3 = FbfCoeffSumH_tmp2 + FbfCoeffSumH_tmp3;


	MpiSetValue(&MpiSymbolRateHz, (S32BITS)SymbolRateHz);
	MpiSetValue(&MpiCrystalFreqHz, (S32BITS)CrystalFreqHz);
	MpiSetValue(&MpiTrOffsetN2Int, (S32BITS)TrOffsetN2Int);
	MpiSetValue(&MpiTrFsFreqInt, (S32BITS)TrFsFreqInt);
	MpiSetValue(&MpiConst, 1000000);

	//Cal Tr Offset
	MpiMul(&MpiVar0, MpiTrOffsetN2Int, MpiSymbolRateHz);
	MpiMul(&MpiVar0, MpiVar0, MpiConst);
	MpiLeftShift(&MpiVar1, MpiCrystalFreqHz, 23);
	MpiDiv(&MpiVarN2, &MpiNone, MpiVar0, MpiVar1);
	MpiGetValue(MpiVarN2, &TrOffset);

	//Cal TrFsFreq
	MpiMul(&MpiVar0, MpiTrFsFreqInt, MpiSymbolRateHz);
	MpiMul(&MpiVar0, MpiVar0, MpiConst);
	MpiLeftShift(&MpiVar1, MpiCrystalFreqHz, 24);
	MpiDiv(&MpiVarN2, &MpiNone, MpiVar0, MpiVar1);
	MpiGetValue(MpiVarN2, &TrFsFreq);

	//Cal fbf_const_coeff_sum
	//MpiDiv(&MpiVarN2, &MpiNone, MpiTrLoopSelInt, (S32BITS)16);
	//MpiGetValue(MpiVarN2, (S32BITS *)&TrLoopSel);
	//FbfConstCoeffSum = (double)FbfConstCoeffSumInt / (double)16;

	if ((FbfCoeffSumH_tmp1 >= 6) && (FbfCoeffSumH_tmp2_3 >= 1)) { //Dynamic echoes
		if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FFE_SP_HOLD, 0x0) != FUNCTION_SUCCESS)
			goto error_status_set_demod_registers;

		if (abs(TrOffset) > 0) {
			if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_TR_LOCK_TH2, 0x80) != FUNCTION_SUCCESS)
				goto error_status_set_demod_registers;

			realtek_r_atsc_SoftwareReset(pDemod);
		}

	} else { //Static echoes
		if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_FFE_SP_HOLD, 0x1) != FUNCTION_SUCCESS)
			goto error_status_set_demod_registers;

		if (abs(TrFsFreq) > 0) {
			if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_TR_LOCK_TH2, 0x7f) != FUNCTION_SUCCESS)
				goto error_status_set_demod_registers;

			realtek_r_atsc_SoftwareReset(pDemod);
		}

	}

	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
error_status_set_demod_registers:
	return FUNCTION_ERROR;
}


/*
@see   ATSC_DEMOD_FP_GET_CR_OFFSET_HZ
*/
int
realtek_r_atsc_GetCrOffsetHz(
	ATSC_DEMOD_MODULE *pDemod,
	S32BITS *pCrOffsetHz
)
{
	U32BITS CrystalFreqHz = 0;
	U32BITS CrOffsetN1Binary = 0;
	U32BITS CrOffsetN2BinaryLsb = 0, CrOffsetN2BinaryMsb = 0;
	S32BITS CrOffsetN1Int = 0;
	S32BITS          CrOffsetN2IntMsb = 0;
	MPI MpiCrystalFreqHz, MpiCrOffsetN1Int, MpiCrOffsetN2Int, MpiCrOffsetN2BinaryLsb;
	MPI MpiVar, MpiVarN2, MpiVarN1;

	memset(&MpiCrystalFreqHz, 0 , sizeof(MpiCrystalFreqHz));
	memset(&MpiCrOffsetN1Int, 0 , sizeof(MpiCrOffsetN1Int));
	memset(&MpiCrOffsetN2Int, 0 , sizeof(MpiCrOffsetN2Int));
	memset(&MpiCrOffsetN2BinaryLsb, 0 , sizeof(MpiCrOffsetN2BinaryLsb));
	memset(&MpiVar, 0 , sizeof(MpiVar));
	memset(&MpiVarN2, 0 , sizeof(MpiVarN2));
	memset(&MpiVarN1, 0 , sizeof(MpiVarN1));

	// Get demod crystal frequency in Hz.
	pDemod->GetCrystalFreqHz(pDemod, &CrystalFreqHz);

	// Set RO_STROBE with 0x1, then set RO_STROBE with 0x0.
	// Note: RO_STROBE is a write-only register.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	// Get CR offset N1 binary value from RO_CR_OFFSET_N1 register bits.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_CR_OFFSET_N1, &CrOffsetN1Binary) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Convert CR offset N1 binary value to signed integer.
	CrOffsetN1Int = BinToSignedInt((U32BITS)CrOffsetN1Binary, REALTEK_R_ATSC_CR_OFFSET_N1_BIT_NUM);

	// Get CR offset N2 LSB and MSB binary value from RO_CR_OFFSET_N2_25_0 and RO_CR_OFFSET_N2_32_26 register bits.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_CR_OFFSET_N2_25_0, &CrOffsetN2BinaryLsb) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_CR_OFFSET_N2_32_26, &CrOffsetN2BinaryMsb) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Convert CR offset N2 MSB binary value to signed integer.
	// Note: Because CR offset N2 is a 33-bit signed integer, we can not use BinToSignedInt() directly.
	//       Original formula:   CrOffsetN2Int = BinToSignedInt(CrOffsetN2Binary)
	//       Adjusted formula:   CrOffsetN2Int = (BinToSignedInt(CrOffsetN2BinaryMsb) << 26) + CrOffsetN2BinaryLsb
	CrOffsetN2IntMsb = BinToSignedInt((U32BITS)CrOffsetN2BinaryMsb, REALTEK_R_ATSC_CR_OFFSET_N2_32_26_BIT_NUM);

	// Get CR offset in Hz.
	// Note: Original formula:   CrOffsetHz   = CrOffsetN1Hz + CrOffsetN2Hz
	//                           CrOffsetN1Hz = (CrOffsetN1Int * CrystalFreqHz) / pow(2, 20)
	//                           CrOffsetN2Hz = (CrOffsetN2Int * CrystalFreqHz) / pow(2, 37)
	//       Adjusted formula:   CrOffsetHz   = CrOffsetN1Hz + CrOffsetN2Hz
	//                           CrOffsetN1Hz = (CrOffsetN1Int * CrystalFreqHz) >> 20
	//                           CrOffsetN2Hz = (((CrOffsetN2IntMsb << 26) + CrOffsetN2BinaryLsb) * CrystalFreqHz) >> 37
	//       Because the bit number of CrOffsetN2BinaryLsb is less than 32,
	//       there is no signed bit extension issue when use MpiSetValue().
	MpiSetValue(&MpiCrystalFreqHz, (S32BITS)CrystalFreqHz);
	MpiSetValue(&MpiCrOffsetN1Int, (S32BITS)CrOffsetN1Int);
	MpiSetValue(&MpiCrOffsetN2Int, (S32BITS)CrOffsetN2IntMsb);
	MpiSetValue(&MpiCrOffsetN2BinaryLsb, (S32BITS)CrOffsetN2BinaryLsb);
	MpiLeftShift(&MpiCrOffsetN2Int, MpiCrOffsetN2Int, REALTEK_R_ATSC_CR_OFFSET_N2_25_0_BIT_NUM);
	MpiAdd(&MpiCrOffsetN2Int, MpiCrOffsetN2Int, MpiCrOffsetN2BinaryLsb);
	MpiMul(&MpiVarN1, MpiCrOffsetN1Int, MpiCrystalFreqHz);
	MpiRightShift(&MpiVarN1, MpiVarN1, 20);
	MpiMul(&MpiVarN2, MpiCrOffsetN2Int, MpiCrystalFreqHz);
	MpiRightShift(&MpiVarN2, MpiVarN2, 37);
	MpiAdd(&MpiVar, MpiVarN1, MpiVarN2);
	MpiGetValue(MpiVar, (S32BITS*)pCrOffsetHz);

	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
error_status_set_demod_registers:
	return FUNCTION_ERROR;
}


/*
@see   ATSC_DEMOD_FP_IS_AAGC_LOCKED
*/
int
realtek_r_atsc_IsAagcLocked(
	ATSC_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	U32BITS RoAagcLock;

	// Set RO_STROBE with 0x1, then set RO_STROBE with 0x0.
	// Note: RO_STROBE is a write-only register.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	// Get RO_AAGC_LOCK.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_AAGC_LOCK, &RoAagcLock) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Determine AAGC lock status according to RO_AAGC_LOCK.
	switch (RoAagcLock) {
	default:
	case 0x0:
		*pAnswer = NO;
		break;

	case 0x1:
		*pAnswer = YES;
		break;
	}

	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
error_status_set_demod_registers:
	return FUNCTION_ERROR;
}


/*
@see   ATSC_DEMOD_FP_IS_FRAME_LOCKED
*/
int
realtek_r_atsc_IsFrameLocked(
	ATSC_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	U32BITS RoCurrentState;

	// Set RO_STROBE with 0x1, then set RO_STROBE with 0x0.
	// Note: RO_STROBE is a write-only register.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	// Get RO_CURRENT_STATE.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_CURRENT_STATE, &RoCurrentState) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Determine frame lock status according to RO_CURRENT_STATE.
	switch (RoCurrentState) {
	default:
		*pAnswer = NO;
		break;

	case 32:
	case 33:
	case 34:
		*pAnswer = YES;
		break;
	}

	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
error_status_set_demod_registers:
	return FUNCTION_ERROR;
}


int
realtek_r_atsc_IsTrLocked(
	ATSC_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
    U32BITS RoTrLockInd;

	// Set RO_STROBE with 0x1, then set RO_STROBE with 0x0.
	// Note: RO_STROBE is a write-only register.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x1) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x0) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Get RO_CURRENT_STATE.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_TR_LOCK_IND, &RoTrLockInd) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	if (RoTrLockInd == 1)
		*pAnswer = YES;
	else
		*pAnswer = NO;

	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
	return FUNCTION_ERROR;
}


int
realtek_r_atsc_IsCrLocked(
	ATSC_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	U32BITS RoCrLockInd;

	// Set RO_STROBE with 0x1, then set RO_STROBE with 0x0.
	// Note: RO_STROBE is a write-only register.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x1) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x0) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Get RO_CURRENT_STATE.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_CR_LOCK_IND, &RoCrLockInd) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	if (RoCrLockInd == 1)
		*pAnswer = YES;
	else
		*pAnswer = NO;

	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
	return FUNCTION_ERROR;
}


int
realtek_r_atsc_IsNoSig(
	ATSC_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	int ret = FUNCTION_SUCCESS;
	
	ret = realtek_r_atsc_IsCrLocked(pDemod, pAnswer);
	*pAnswer = (*pAnswer) ? NO : YES;
	
	return ret;
}


/*
@see   ATSC_DEMOD_FP_GET_ERROR_RATE
*/
int
realtek_r_atsc_GetErrorRate(
	ATSC_DEMOD_MODULE *pDemod,
	U32BITS TestVolume,
	unsigned int WaitTimeMsMax,
	U32BITS *pByteErrorRateNum,
	U32BITS *pByteErrorRateDen,
	U32BITS *pPacketErrorRateNum,
	U32BITS *pPacketErrorRateDen
)
{
	BASE_INTERFACE_MODULE *pBaseInterface;

	U32BITS RoAoRsSerTrig;
	U32BITS AoUnCorrCnt, AoErrByteCnt;

	// Get base interface.
	pBaseInterface = pDemod->pBaseInterface;

	// Set RO_STROBE with 0x1, then set RO_STROBE with 0x0.
	// Note: RO_STROBE is a write-only register.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	// Get RO_AO_RS_SER_TRIG.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_AO_RS_SER_TRIG, &RoAoRsSerTrig) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Get AO_ERR_BYTE_CNT.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_AO_ERR_BYTE_CNT, &AoErrByteCnt) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Get AO_UN_CORR_CNT.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_AO_UN_CORR_CNT, &AoUnCorrCnt) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Set byte error rate numerator and denominator.
	*pByteErrorRateNum = (RoAoRsSerTrig == 0x1) ? AoErrByteCnt : REALTEK_R_ATSC_BER_TEST_BYTE_NUM;
	*pByteErrorRateDen = REALTEK_R_ATSC_BER_TEST_BYTE_NUM;

	// Set packet error rate numerator and denominator.
	*pPacketErrorRateNum = (RoAoRsSerTrig == 0x1) ? AoUnCorrCnt : REALTEK_R_ATSC_BER_TEST_PACKET_NUM;
	*pPacketErrorRateDen = REALTEK_R_ATSC_BER_TEST_PACKET_NUM;

	return FUNCTION_SUCCESS;

error_status_set_demod_registers:
error_status_get_demod_registers:
	return FUNCTION_ERROR;
}


/*
@see   ATSC_DEMOD_FP_GET_SNR_DB
*/
int
realtek_r_atsc_GetSnrDb(
	ATSC_DEMOD_MODULE *pDemod,
	S32BITS *pSnrDbNum,
	S32BITS *pSnrDbDen
)
{
	U32BITS Mse2Lsb = 0, Mse2Msb = 0;
	MPI MpiMse, MpiMse2Lsb, MpiVar;
	S32BITS Var = 0;

	memset(&MpiMse, 0 , sizeof(MpiMse));
	memset(&MpiMse2Lsb, 0 , sizeof(MpiMse2Lsb));
	memset(&MpiVar, 0 , sizeof(MpiVar));

	// Set RO_STROBE with 0x1, then set RO_STROBE with 0x0.
	// Note: RO_STROBE is a write-only register.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	// Get MSE2 LSB and Msb value from RO_MSE2_23_0 and RO_MSE2_33_24.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_MSE2_23_0, &Mse2Lsb) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_MSE2_33_24, &Mse2Msb) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// SNR dB formula
	// Original formula: SNR_dB = 10 * log10(21 * pow(2, 24) / MSE)
	// Adjusted formula: SNR_dB = (SNR_DB_NUM_CONST - 10 * log2(MSE) * pow(2, SNR_FRAC_BIT_NUM)) / SNR_DB_DEN
	//                   SNR_DB_NUM_CONST = 10 * log2(21 * pow(2, 26)) * pow(2, SNR_FRAC_BIT_NUM)
	//                   SNR_DB_DEN       = log2(10) * pow(2, SNR_FRAC_BIT_NUM)

	// Calculate SNR dB numerator.
	// Note: Mse = (Mse2Msb << 24) + Mse2Lsb
	//       Because the bit numbers of Mse2Msb and Mse2Lsb are less than 32,
	//       there is no signed bit extension issue when use MpiSetValue().
	MpiSetValue(&MpiMse, (S32BITS)Mse2Msb);
	MpiSetValue(&MpiMse2Lsb, (S32BITS)Mse2Lsb);
	MpiLeftShift(&MpiMse, MpiMse, REALTEK_R_ATSC_MSE2_23_0_BIT_NUM);
	MpiAdd(&MpiMse, MpiMse, MpiMse2Lsb);
	MpiLog2(&MpiVar, MpiMse, REALTEK_R_ATSC_SNR_FRAC_BIT_NUM);
	MpiGetValue(MpiVar, &Var);

	*pSnrDbNum = REALTEK_R_ATSC_SNR_DB_NUM_CONST - 10 * Var;

	// Set SNR dB denominator.
	*pSnrDbDen = REALTEK_R_ATSC_SNR_DB_DEN;

	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
error_status_set_demod_registers:
	return FUNCTION_ERROR;
}


/*
@see   ATSC_DEMOD_FP_GET_SIGNAL_STRENGTH
*/
int
realtek_r_atsc_GetSignalStrength(
	ATSC_DEMOD_MODULE *pDemod,
	int TxRSSIdBm,
	U32BITS *pSignalStrength
)
{
#ifndef GET_SIGNAL_STRENGTH_CUSTOMER_SONY
	int FrameLock;
	S32BITS IfAgcValue;
#endif
	
#ifdef GET_SIGNAL_STRENGTH_CUSTOMER_SONY //Formula for SONY
	if (TxRSSIdBm >= -100 && TxRSSIdBm <= 0)
		*pSignalStrength = (U32BITS)(TxRSSIdBm + 100);
	else if (TxRSSIdBm > 0)
		*pSignalStrength = 100;
	else
		*pSignalStrength = 0;
	
	REALTEK_R_ATSC_BASE_DBG("TxRSSIdBm = %d, pSignalStrength = "PT_U32BITS"\n", TxRSSIdBm, *pSignalStrength);

	return FUNCTION_SUCCESS;
	
#else //Default Formula
	// Get demod frame lock status.
	if (pDemod->IsFrameLocked(pDemod, &FrameLock) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// If demod is not frame-locked, set signal strength with zero.
	if (FrameLock == NO) {
		*pSignalStrength = 0;
		goto success_status_non_frame_lock;
	}

	// Get IF AGC value.
	if (pDemod->GetIfAgc(pDemod, &IfAgcValue) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Determine signal strength according to IF AGC value.
	// Note: Map IF AGC value (8191 ~ -8192) to signal strength (0 ~ 100) linearly.
	*pSignalStrength = (819100 - IfAgcValue * 100) / 16383;

	REALTEK_R_ATSC_BASE_DBG("TxRSSIdBm = %d, pSignalStrength = "PT_U32BITS"\n", TxRSSIdBm, *pSignalStrength);

success_status_non_frame_lock:
	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
	return FUNCTION_ERROR;
	
#endif
}


/*
@see   ATSC_DEMOD_FP_GET_SIGNAL_QUALITY
*/
int
realtek_r_atsc_GetSignalQuality(
	ATSC_DEMOD_MODULE *pDemod,
	U32BITS *pSignalQuality
)
{
	int FrameLock;
	S32BITS SnrDbNum, SnrDbDen, SnrDbUnit0p01;

	// Get demod frame lock status.
	if (pDemod->IsFrameLocked(pDemod, &FrameLock) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// If demod is not frame-locked, set signal quality with zero.
	if (FrameLock == NO) {
		*pSignalQuality = 0;
		goto success_status_non_frame_lock;
	}

	// Get SNR in dB from demod.
	if (pDemod->GetSnrDb(pDemod, &SnrDbNum, &SnrDbDen) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Determine signal quality according to SNR value.
	// Note: Map SNR value 14 ~ 20 to signal quality (0 ~ 100) linearly.
	//       If SNR value < 14, signal quality is 0.
	//       If SNR value > 20, signal quality is 100.
	SnrDbUnit0p01 = (SnrDbNum * 100) / SnrDbDen;

	if (SnrDbUnit0p01 < 1400) {
		*pSignalQuality = 0;
	} else if (SnrDbUnit0p01 > 2000) {
		*pSignalQuality = 100;
	} else {
		*pSignalQuality = (SnrDbUnit0p01 * 100 - 140000) / 600;
	}

success_status_non_frame_lock:
	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
	return FUNCTION_ERROR;
}


/*
@see   ATSC_DEMOD_FP_UPDATE_FUNCTION
*/
int
realtek_r_atsc_UpdateFunction(
	ATSC_DEMOD_MODULE *pDemod
)
{
	//S32BITS CheckPpm = 0;
	S32BITS CheckTrLoop = 0;
	U32BITS RoCurrentState;

	// Set RO_STROBE with 0x1, then set RO_STROBE with 0x0.
	// Note: RO_STROBE is a write-only register.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_RO_STROBE, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	// Get RO_CURRENT_STATE.
	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_RO_CURRENT_STATE, &RoCurrentState) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	if ((RoCurrentState >= 32) && (RoCurrentState <= 34))
		//realtek_r_atsc_GetTrFsFreqSubOffsetPpm(pDemod, &CheckPpm);
		realtek_r_atsc_GetTrLoopSel(pDemod, &CheckTrLoop);

	return FUNCTION_SUCCESS;
	
error_status_get_demod_registers:
error_status_set_demod_registers:
	return FUNCTION_ERROR;
}


/*
@see   ATSC_DEMOD_FP_RESET_FUNCTION
*/
int
realtek_r_atsc_ResetFunction(
	ATSC_DEMOD_MODULE *pDemod
)
{
	// REALTEK_R ATSC does not use UpdateFunction(), so we just return FUNCTION_SUCCESS.
	return FUNCTION_SUCCESS;
}


/*
@see   I2C_BRIDGE_FP_FORWARD_I2C_READING_CMD
*/
int
realtek_r_atsc_ForwardI2cReadingCmd(
	I2C_BRIDGE_MODULE *pI2cBridge,
	unsigned char DeviceAddr,
	unsigned char *pReadingBytes,
	unsigned long ByteNum
)
{
	ATSC_DEMOD_MODULE *pDemod;
	BASE_INTERFACE_MODULE *pBaseInterface;

	// Get demod module and tuner device address.
	pDemod = (ATSC_DEMOD_MODULE *)pI2cBridge->pPrivateData;

	// Get base interface.
	pBaseInterface = pDemod->pBaseInterface;

	// Enable demod I2C relay.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_OPT_I2C_RELAY, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	// Send I2C reading command.
	if (pBaseInterface->I2cRead(pBaseInterface, DeviceAddr, pReadingBytes, ByteNum) != FUNCTION_SUCCESS)
		goto error_send_i2c_reading_command;

	return FUNCTION_SUCCESS;

error_send_i2c_reading_command:
error_status_set_demod_registers:
	return FUNCTION_ERROR;
}


/*
@see   I2C_BRIDGE_FP_FORWARD_I2C_WRITING_CMD
*/
int
realtek_r_atsc_ForwardI2cWritingCmd(
	I2C_BRIDGE_MODULE *pI2cBridge,
	unsigned char DeviceAddr,
	const unsigned char *pWritingBytes,
	unsigned long ByteNum
)
{
	ATSC_DEMOD_MODULE *pDemod;
	BASE_INTERFACE_MODULE *pBaseInterface;

	// Get demod module and tuner device address.
	pDemod = (ATSC_DEMOD_MODULE *)pI2cBridge->pPrivateData;

	// Get base interface.
	pBaseInterface = pDemod->pBaseInterface;

	// Enable demod I2C relay.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, ATSC_OPT_I2C_RELAY, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	// Send I2C writing command.
	if (pBaseInterface->I2cWrite(pBaseInterface, DeviceAddr, pWritingBytes, ByteNum) != FUNCTION_SUCCESS)
		goto error_send_i2c_writing_command;

	return FUNCTION_SUCCESS;

error_send_i2c_writing_command:
error_status_set_demod_registers:
	return FUNCTION_ERROR;
}


/*
@brief   Initialize register table

REALTEK_R ATSC builder will use realtek_r_atsc_InitRegTable() to initialize register table.

@param [in]   pDemod   The demod module pointer
@see   BuildRealtekRAtscModule()
*/
void
realtek_r_atsc_InitRegTable(
	ATSC_DEMOD_MODULE *pDemod
)
{
	static const ATSC_PRIMARY_REG_ENTRY_ADDR_16BIT PrimaryRegTable[] = {
		// Generality
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{ATSC_SYS_VERSION,					0xd001,			7,		0	},
		{ATSC_I2CT_EN_CTRL,					0xf001,			6,		6	},
		{ATSC_OPT_SOFT_RESET_ATSC,			0xd005,			0,		0	},

		// Miscellany
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{ATSC_RO_STROBE,					0xdf01,			0,		0	},

		// AAGC
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{ATSC_RF_AGC_MAX,					0xd10d,			7,		0	},
		{ATSC_RF_AGC_MIN,					0xd10c,			7,		0	},
		{ATSC_IF_AGC_MAX,					0xd10b,			7,		0	},
		{ATSC_IF_AGC_MIN,					0xd10a,			7,		0	},
		{ATSC_AAGC_TARGET,					0xd103,			7,		0	},
		{ATSC_POLAR_IFAGC,					0xd104,			0,		0	},
		{ATSC_AGC_MODE,					0xd104,			5,		5	},
		{ATSC_VTOP,					             0xd108,			6,		0	},
		{ATSC_KRF,						0xd109,			5,		0	},
		{ATSC_RO_AAGC_LOCK,					0xdf0a,			7,		7	},
		{ATSC_RO_RF_AAGC,					0xdf02,			13,		0	},
		{ATSC_RO_IF_AAGC,					0xdf04,			13,		0	},
		{ATSC_OPT_IF_AAGC_DRIVE,				0xd00a,			3,		3	},
		{ATSC_OPT_PAR_IF_SD_IB,			    		0xd00a,			4,		4	},
		
		// DAGC
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{ATSC_RO_DAGC_LEVEL_Q,			                	0xdf0e,			12,		0	},

		// Demod status
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{ATSC_RO_CURRENT_STATE,				0xdf09,			7,		2	},

		// DDC
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{ATSC_DDC_PHASE_INC,			                	0xd10f,			19,		0	},
		{ATSC_SPEC_INV_ON,					0xd111,			4,		4	},
		{ATSC_CR_CONST_INC,					0xd65a,			16,		0	},
		{ATSC_CR_ACQ_FREQ_UB,			                	0xd65d,			5,		0	},
		{ATSC_TR_PHASE_INC,					0xd165,			26,		0	},

		// Carrier recovery
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{ATSC_RO_CR_OFFSET_N1,			                	0xdf80,			17,		0	},
		{ATSC_RO_CR_OFFSET_N2_25_0,		                	0xdf82,			31,		6	},
		{ATSC_RO_CR_OFFSET_N2_32_26,	                                	0xdf86,			6,		0	},
		{ATSC_RO_CR_LOCK_IND,			                          0xdf82,			2,		2	},
		{ATSC_CR_FMU_UPD_OPT,                           	0xd5a0,         6,      4   },
        {ATSC_CR_LPF_SHIFT,                                 0xd5af,         15,     4   },
		{ATSC_CR_PMU_COMP1,                                 0xd635,         3,      0   },
		{ATSC_CR_FMU_UPD_CNT_TH,                            0xd5a7,         7,      0   },
		{ATSC_CR_CR2_CARB,                                 	0xd002,         4,      2   },

		// Timing recovery
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{ATSC_RO_TR_OFFSET_N1,			                	0xdf99,			10,		0	},
		{ATSC_RO_TR_OFFSET_N2,				             0xdf12,			15,		0	},
		{ATSC_RO_TR_LOCK_IND,		                		0xdf0a,			2,		2	},
		{ATSC_TR_MIN_SAT_EN,		                		0xd16d,			2,		2	},
		{ATSC_TR_LOCK_IND_MODE,		                	0xd16d,			1,		0	},
		{ATSC_RO_TR_FS_FREQ,								0xdf47, 			23,		0	},
		{ATSC_TR_LOCK_TH2,					0xd16a,			7,		0	},
		{ATSC_TR_FS_PLL_KP_EXP,				0xd173,			7,		5	},

		// Equalizer
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{ATSC_RO_MSE2_23_0,					0xdf24,			23,		0	},
		{ATSC_RO_MSE2_33_24,				             0xdf27,			9,		0	},
		{ATSC_FBF_RO_SUM_ADD,				0xd755,			9,		3	},
		{ATSC_RO_FBF_COEFF_SUM_H,			0xdfaf,			7,		0	},
		{ATSC_FFE_SP_HOLD,					0xd70a,			0,		0	},
		//{ATSC_LD_MSE_FIELD_TH_1,			0xd847,			33,		0	},//[ATSC_LD_MSE_FIELD_TH_H, ATSC_LD_MSE_FIELD_TH_L]
		{ATSC_LD_MSE_FIELD_TH_H,			0xd84b,			1,		0	},
		{ATSC_LD_MSE_FIELD_TH_L,			0xd847,			31,		0	},

		//FSM
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{ATSC_FSM_RAM_ROW,					0xd1e1,			7,		0	},
		{ATSC_FSM_RAM_COL,					0xd1e0,			7,		0	},
		{ATSC_FSM_RAM_DATA,					0xd1e2,			7,		0	},

		// Outer
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{ATSC_BER_RST,						0xd856,			7,		7	},
		{ATSC_TEST_VOLUME,					0xd856,			3,		0	},
		{ATSC_BER_ONCE,						0xd856,			4,		4	},
		{ATSC_BER_SEL,						0xd856,			5,		5	},
		{ATSC_RO_AO_RS_SER_TRIG,			0xdfe9,			4,		4	},
		{ATSC_AO_ERR_BYTE_CNT,				0xdfe6,			27,		0	},
		{ATSC_AO_PKT_CNT,					0xdfea,			23,		0	},
		{ATSC_AO_CORR_CNT,					0xdfed,			23,		0	},
		{ATSC_AO_UN_CORR_CNT,				0xdff0,			23,		0	},

		// MPEG TS output interface
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{ATSC_FIX_TEI,						0xd852,			0,		0	},
		{ATSC_CKOUTPAR,						0xd852,			1,		1	},
		{ATSC_CKOUT_PWR,					0xd852,			2,		2	},
		{ATSC_SYNC_DUR,						0xd852,			3,		3	},
		{ATSC_ERR_DUR,						0xd852,			4,		4	},
		{ATSC_ERR_LVL,						0xd852,			5,		5	},
		{ATSC_VAL_LVL,						0xd852,			6,		6	},
		{ATSC_SERIAL,						0xd853,			0,		0	},
		{ATSC_SER_LSB,						0xd853,			1,		1	},
		{ATSC_CDIV_PH0,						0xd854,			3,		0	},
		{ATSC_CDIV_PH1,						0xd854,			7,		4	},

		// Smart antenna
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{ATSC_OPT_ANT_RESET_N,				        0xd0c4,			0,		0	},
		{ATSC_OPT_ANT_ENABLE,				        0xd0c4,			1,		1	},

		// Dummy register
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{Dummy_FAKE_PER_EN,				        0xf07f,			7,		7	},
		{Dummy_8051_PATCH_IND,				    0xf081,			7,		0	},
		{Dummy_CHANGE_CHANNEL_IND,              0xf095,         7,      0   },
		{DW_8051_RST,			                0xf001,			5,		5	},
		{ATSC_LD_AI_EN_5,			            0xd833,			5,		5	},
	};

	int i;
	int RegBitName;

	// Initialize register table according to primary register table.
	// Note: 1. Register table rows are sorted by register bit name key.
	//       2. The default value of the IsAvailable variable is "NO".
	for (i = 0; i < ATSC_REG_TABLE_LEN_MAX; i++)
		pDemod->RegTable.Addr16Bit[i].IsAvailable  = NO;

	for (i = 0; i < sizeof(PrimaryRegTable) / sizeof(ATSC_PRIMARY_REG_ENTRY_ADDR_16BIT); i++) {
		RegBitName = PrimaryRegTable[i].RegBitName;

		pDemod->RegTable.Addr16Bit[RegBitName].IsAvailable  = YES;
		pDemod->RegTable.Addr16Bit[RegBitName].RegStartAddr = PrimaryRegTable[i].RegStartAddr;
		pDemod->RegTable.Addr16Bit[RegBitName].Msb          = PrimaryRegTable[i].Msb;
		pDemod->RegTable.Addr16Bit[RegBitName].Lsb          = PrimaryRegTable[i].Lsb;
	}

	return;
}


/*
@brief   Set REALTEK_R ATSC demod register mask bits with acceleration.

REALTEK_R ATSC upper level functions will use realtek_r_atsc_acceleration_SetRegMaskBits() to set demod register mask bits.

@param [in]   pDemod         The demod module pointer
@param [in]   RegStartAddr   Demod register start address
@param [in]   Msb            Mask MSB with 0-based index
@param [in]   Lsb            Mask LSB with 0-based index
@param [in]   WritingValue   The mask bits writing value

@retval   FUNCTION_SUCCESS   Set demod register mask bits successfully with writing value.
@retval   FUNCTION_ERROR     Set demod register mask bits unsuccessfully.

@note
	-# The realtek_r_qam_acceleration_SetRegMaskBits() function accelerate mask bits setting in byte writitng case.
	-# Need to set register page by QAM_DEMOD_FP_SET_REG_PAGE() before using realtek_r_qam_acceleration_SetRegMaskBits().
	-# The constraints of realtek_r_qam_acceleration_SetRegMaskBits() function usage are described as follows:
		-# The mask MSB and LSB must be 0~31.
		-# The mask MSB must be greater than or equal to LSB.
*/
int
realtek_r_atsc_acceleration_SetRegMaskBits(
	ATSC_DEMOD_MODULE *pDemod,
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
		if (atsc_demod_addr_16bit_default_SetRegBytes(pDemod, RegStartAddr, WritingBytes, ByteNum) != FUNCTION_SUCCESS)
			goto error_status_set_demod_registers;
	} else {
		// Use mask bit setting method to set register mask bits.
		if (atsc_demod_addr_16bit_default_SetRegMaskBits(pDemod, RegStartAddr, Msb, Lsb, WritingValue) !=
			FUNCTION_SUCCESS)
			goto error_status_set_demod_registers;
	}

	return FUNCTION_SUCCESS;

error_status_set_demod_registers:
	return FUNCTION_ERROR;
}


/*
@brief   Set I2C bridge module demod arguments.

REALTEK_R ATSC builder will use realtek_r_atsc_BuildI2cBridgeModule() to set I2C bridge module demod arguments.

@param [in]   pDemod   The demod module pointer
@see   BuildRealtekRAtscModule()
*/
void
realtek_r_atsc_BuildI2cBridgeModule(
	ATSC_DEMOD_MODULE *pDemod
)
{
	I2C_BRIDGE_MODULE *pI2cBridge;

	// Get I2C bridge module.
	pI2cBridge = pDemod->pI2cBridge;

	// Set I2C bridge module demod arguments.
	pI2cBridge->pPrivateData = (void *)pDemod;
	pI2cBridge->ForwardI2cReadingCmd = realtek_r_atsc_ForwardI2cReadingCmd;
	pI2cBridge->ForwardI2cWritingCmd = realtek_r_atsc_ForwardI2cWritingCmd;

	return;
}


/*
@brief   Download demod 8051 firmware code.

Use realtek_r_atsc_DownloadFirmwareCode() to download demod 8051 firmware code.

@param [in]   pDemod   The demod module pointer
*/
int
realtek_r_atsc_DownloadFirmwareCode(
	ATSC_DEMOD_MODULE *pDemod
)
{
	BASE_INTERFACE_MODULE *pBaseInterface;
	U32BITS ReadingValue;

	// Get base interface.
	pBaseInterface = pDemod->pBaseInterface;

	pBaseInterface->useI2C = 1;
	// Download REALTEK_R ATSC firmware code to demod.
	if (pDemod->RegAccess.Addr16Bit.SetRegMaskBits(pDemod, 0xf008, 7, 0, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegMaskBits(pDemod, 0xf009, 7, 0, 0xff) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegMaskBits(pDemod, 0xf007, 7, 0, REALTEK_R_ATSC_FIRMWARE_CODE_TABLE_LEN >> 8) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegMaskBits(pDemod, 0xf001, 7, 0, 0x81) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegBytes(pDemod, 0x3000, realtek_r_atsc_FirewareCodeTable,
			REALTEK_R_ATSC_FIRMWARE_CODE_TABLE_LEN) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegMaskBits(pDemod, 0xf001, 7, 0, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegMaskBits(pDemod, 0xf002, 7, 0, 0x80) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	//pBaseInterface->WaitMs(pBaseInterface, 20);
	tv_osal_msleep(20);

	if (pDemod->RegAccess.Addr16Bit.GetRegMaskBits(pDemod, 0xf002, 7, 0, &ReadingValue) != FUNCTION_SUCCESS)
		goto error_status_get_registers;

	if (ReadingValue != 0x8f)
		goto error_status_check_firmware_code;

	if (pDemod->RegAccess.Addr16Bit.SetRegMaskBits(pDemod, 0xf002, 7, 0, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	//if (pDemod->RegAccess.Addr16Bit.SetRegMaskBits(pDemod, 0xf001, 7, 0, 0x21) != FUNCTION_SUCCESS)
	//	goto error_status_set_registers;

	if (pDemod->RegAccess.Addr16Bit.SetRegMaskBits(pDemod, 0xf005, 7, 0, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_registers;
	
	pBaseInterface->useI2C = 0;

	return FUNCTION_SUCCESS;

error_status_check_firmware_code:
error_status_get_registers:
error_status_set_registers:
	return FUNCTION_ERROR;
}


// ATSC Demod 8051 FW V2.A (B815C1D4 = 0x2a)
// fsm_cr_fmu/fsm_cr_pmu (st32,33,34), KTASKWBS-5745 patch, ADJ_DET, and FS2 range Merlin5 (LGE K6), 20190724
// REALTEK_R ATSC 8051 demod firmware code table
unsigned char realtek_r_atsc_FirewareCodeTable[REALTEK_R_ATSC_FIRMWARE_CODE_TABLE_LEN] = {
0x75,
0x3d,
0x0,
0x75,
0x3e,
0x0,
0x90,
0xf0,
0x8,
0xe4,
0xf0,
0xe5,
0x3e,
0x45,
0x3d,
0x70,
0x3,
0x2,
0x30,
0xde,
0x12,
0x40,
0xbf,
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
0x3f,
0xd1,
0x12,
0x3e,
0xcb,
0x12,
0x3d,
0xbd,
0x90,
0x38,
0x20,
0x12,
0x1b,
0xc3,
0x12,
0x18,
0x9e,
0xff,
0x8f,
0x99,
0x90,
0x38,
0x20,
0x12,
0x1b,
0xc3,
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
0x86,
0x0,
0x0,
0x0,
0x0,
0x90,
0xf0,
0x30,
0x12,
0x1b,
0x52,
0xef,
0x65,
0x3c,
0xff,
0xee,
0x65,
0x3b,
0xfe,
0xed,
0x65,
0x3a,
0xfd,
0xec,
0x65,
0x39,
0xfc,
0x90,
0xf0,
0x30,
0x12,
0x1b,
0x7a,
0x90,
0xf0,
0x30,
0x12,
0x1b,
0x52,
0x12,
0x1c,
0x22,
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
0x34,
0x0,
0x75,
0x33,
0x0,
0x75,
0x32,
0x0,
0x75,
0x31,
0x0,
0xaf,
0x34,
0xae,
0x33,
0xad,
0x32,
0xac,
0x31,
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
0x1b,
0x2,
0x50,
0x1e,
0xe5,
0x34,
0x24,
0xff,
0xff,
0xe5,
0x33,
0x34,
0xff,
0xfe,
0xe5,
0x32,
0x34,
0xff,
0xfd,
0xe5,
0x31,
0x34,
0xff,
0xfc,
0x8f,
0x34,
0x8e,
0x33,
0x8d,
0x32,
0x8c,
0x31,
0x80,
0xcc,
0x90,
0x38,
0x20,
0x12,
0x1b,
0xc3,
0x12,
0x18,
0x9e,
0xff,
0x8f,
0x99,
0x90,
0x38,
0x20,
0x12,
0x1b,
0xc3,
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
0x8f,
0x22,
0xe5,
0x22,
0x64,
0x2,
0x60,
0x3,
0x2,
0x34,
0xd8,
0x90,
0xf0,
0x75,
0x74,
0x2a,
0xf0,
0x90,
0xf0,
0x76,
0x74,
0x2,
0xf0,
0x90,
0xf0,
0x95,
0xe0,
0xff,
0x8f,
0x22,
0xe5,
0x22,
0xb4,
0x1,
0x14,
0x90,
0xf0,
0x81,
0x74,
0x1,
0xf0,
0x90,
0xf0,
0x95,
0x74,
0x2,
0xf0,
0x90,
0xf0,
0x97,
0x74,
0x2,
0xf0,
0x80,
0xe0,
0x12,
0x39,
0x8,
0x12,
0x39,
0x2a,
0x8f,
0x23,
0x90,
0xf0,
0x82,
0xe5,
0x23,
0xf0,
0x90,
0xf0,
0x95,
0xe0,
0xff,
0x8f,
0x22,
0x90,
0xf0,
0x97,
0xe0,
0xff,
0x7e,
0x0,
0x8e,
0x29,
0x8f,
0x2a,
0xe5,
0x23,
0xd3,
0x94,
0x6,
0x50,
0x6,
0xe5,
0x22,
0x64,
0x2,
0x60,
0xb,
0xe5,
0x2a,
0x64,
0x2,
0x45,
0x29,
0x60,
0x3,
0x2,
0x31,
0xf3,
0x90,
0xf0,
0x97,
0x74,
0x1,
0xf0,
0x75,
0x24,
0x0,
0x75,
0x2b,
0x0,
0x75,
0x2c,
0x0,
0x5,
0x2c,
0xe5,
0x2c,
0x70,
0x2,
0x5,
0x2b,
0x90,
0xf0,
0x97,
0x74,
0x2,
0xf0,
0x12,
0x39,
0x8,
0x12,
0x39,
0x2a,
0x8f,
0x23,
0x90,
0xf0,
0x83,
0xe5,
0x23,
0xf0,
0xe5,
0x23,
0xc3,
0x94,
0x20,
0x40,
0xb,
0xe5,
0x23,
0xd3,
0x94,
0x22,
0x50,
0x4,
0x5,
0x24,
0x80,
0x3,
0x75,
0x24,
0x0,
0xe5,
0x24,
0xc3,
0x94,
0x5,
0x50,
0x4d,
0xae,
0x2b,
0xaf,
0x2c,
0xe4,
0xfc,
0xfd,
0x7b,
0x60,
0x7a,
0xea,
0x79,
0x0,
0x78,
0x0,
0xc3,
0x12,
0x1b,
0x2,
0x50,
0x11,
0xe5,
0x23,
0x64,
0x20,
0x60,
0xb,
0xe5,
0x23,
0x64,
0x21,
0x60,
0x5,
0xe5,
0x23,
0xb4,
0x22,
0x27,
0x7f,
0x1e,
0x7e,
0x0,
0x7d,
0x0,
0x7c,
0x0,
0x12,
0x38,
0xbf,
0xe5,
0x23,
0x64,
0x20,
0x60,
0xc,
0xe5,
0x23,
0x64,
0x21,
0x60,
0x6,
0xe5,
0x23,
0x64,
0x22,
0x70,
0x85,
0xe5,
0x24,
0xc3,
0x94,
0x5,
0x50,
0x3,
0x2,
0x31,
0x61,
0xe5,
0x24,
0xc3,
0x94,
0x5,
0x40,
0x6,
0x90,
0xf0,
0x97,
0x74,
0x3,
0xf0,
0x90,
0xf0,
0x95,
0xe0,
0xff,
0x8f,
0x22,
0x90,
0xf0,
0x97,
0xe0,
0xff,
0x7e,
0x0,
0x8e,
0x29,
0x8f,
0x2a,
0xe5,
0x2a,
0x64,
0x3,
0x45,
0x29,
0x60,
0x3,
0x2,
0x30,
0xfa,
0xe5,
0x22,
0x64,
0x2,
0x60,
0x3,
0x2,
0x30,
0xfa,
0x90,
0xf0,
0x97,
0x74,
0x4,
0xf0,
0x75,
0x26,
0x0,
0x75,
0x27,
0x0,
0x75,
0x28,
0x0,
0x12,
0x39,
0x8,
0x12,
0x39,
0x44,
0x8f,
0x25,
0x12,
0x39,
0x8,
0x12,
0x39,
0x65,
0x8e,
0x2d,
0x8f,
0x2e,
0x12,
0x39,
0x8,
0x12,
0x39,
0xad,
0x8e,
0x2f,
0x8f,
0x30,
0x75,
0x22,
0x0,
0xe5,
0x22,
0xc3,
0x94,
0x20,
0x50,
0x53,
0x7f,
0xa,
0x7e,
0x0,
0x7d,
0x0,
0x7c,
0x0,
0x12,
0x38,
0xbf,
0x12,
0x39,
0x8,
0x12,
0x3a,
0x36,
0x8f,
0x38,
0x8e,
0x37,
0x8d,
0x36,
0x8c,
0x35,
0x90,
0xf0,
0x8c,
0xe0,
0xff,
0xef,
0xd3,
0x95,
0x26,
0x40,
0x7,
0x90,
0xf0,
0x8c,
0xe0,
0xff,
0x8f,
0x26,
0x90,
0xf0,
0x8b,
0xe0,
0xff,
0xef,
0xd3,
0x95,
0x27,
0x40,
0x7,
0x90,
0xf0,
0x8b,
0xe0,
0xff,
0x8f,
0x27,
0x90,
0xf0,
0x8a,
0xe0,
0xff,
0xef,
0xd3,
0x95,
0x28,
0x40,
0x7,
0x90,
0xf0,
0x8a,
0xe0,
0xff,
0x8f,
0x28,
0x5,
0x22,
0x80,
0xa6,
0x90,
0xf0,
0x8f,
0xe5,
0x26,
0xf0,
0x90,
0xf0,
0x8e,
0xe5,
0x27,
0xf0,
0x90,
0xf0,
0x8d,
0xe5,
0x28,
0xf0,
0xe5,
0x26,
0xc3,
0x94,
0x1,
0x40,
0x3,
0x2,
0x34,
0x38,
0xe5,
0x27,
0xc3,
0x94,
0x1,
0x40,
0x3,
0x2,
0x34,
0x38,
0xe5,
0x28,
0xc3,
0x94,
0x96,
0x40,
0x3,
0x2,
0x34,
0x38,
0x90,
0xd5,
0xa7,
0x74,
0x1,
0xf0,
0x90,
0xd5,
0xaf,
0x74,
0x92,
0xf0,
0x90,
0xd5,
0xb0,
0x74,
0x21,
0xf0,
0x90,
0xd6,
0x35,
0xe4,
0xf0,
0xe5,
0x2e,
0x64,
0x3,
0x45,
0x2d,
0x60,
0x1c,
0xc3,
0xe5,
0x30,
0x94,
0xd5,
0xe5,
0x2f,
0x94,
0x3,
0x50,
0x3,
0x2,
0x33,
0xd8,
0xd3,
0xe5,
0x30,
0x94,
0xe1,
0xe5,
0x2f,
0x94,
0x3,
0x40,
0x3,
0x2,
0x33,
0xd8,
0xe5,
0x25,
0xd3,
0x94,
0x0,
0x40,
0x63,
0x90,
0xd1,
0xe1,
0x74,
0x20,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xe,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0xcf,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xf,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0xda,
0xf0,
0x90,
0xd1,
0xe1,
0x74,
0x21,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xe,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0xcf,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xf,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0xda,
0xf0,
0x90,
0xd1,
0xe1,
0x74,
0x22,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xe,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0xcf,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xf,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0xd9,
0xf0,
0x90,
0xf0,
0x81,
0x74,
0x2,
0xf0,
0x2,
0x34,
0x38,
0x90,
0xd1,
0xe1,
0x74,
0x20,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xe,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0x93,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xf,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0xdb,
0xf0,
0x90,
0xd1,
0xe1,
0x74,
0x21,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xe,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0x93,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xf,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0xdb,
0xf0,
0x90,
0xd1,
0xe1,
0x74,
0x22,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xe,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0x93,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xf,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0xda,
0xf0,
0x90,
0xf0,
0x81,
0x74,
0x3,
0xf0,
0x80,
0x60,
0x90,
0xd1,
0xe1,
0x74,
0x20,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xe,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0xcf,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xf,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0xda,
0xf0,
0x90,
0xd1,
0xe1,
0x74,
0x21,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xe,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0xcf,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xf,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0xda,
0xf0,
0x90,
0xd1,
0xe1,
0x74,
0x22,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xe,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0xcf,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xf,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0xd9,
0xf0,
0x90,
0xf0,
0x81,
0x74,
0x4,
0xf0,
0xe5,
0x28,
0xc3,
0x94,
0x96,
0x50,
0x11,
0xe5,
0x26,
0xd3,
0x94,
0x0,
0x50,
0xa,
0xe5,
0x27,
0xd3,
0x94,
0x0,
0x50,
0x3,
0x2,
0x30,
0xfa,
0x90,
0xd5,
0xa7,
0xe0,
0xff,
0xef,
0xd3,
0x94,
0x0,
0x50,
0x3,
0x2,
0x30,
0xfa,
0x90,
0xd5,
0xa7,
0xe4,
0xf0,
0x90,
0xd5,
0xaf,
0x74,
0x82,
0xf0,
0x90,
0xd5,
0xb0,
0x74,
0xc,
0xf0,
0x90,
0xd6,
0x35,
0x74,
0x1,
0xf0,
0x90,
0xd1,
0xe1,
0x74,
0x20,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xe,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0x97,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xf,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0xdb,
0xf0,
0x90,
0xd1,
0xe1,
0x74,
0x21,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xe,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0x97,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xf,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0xdb,
0xf0,
0x90,
0xd1,
0xe1,
0x74,
0x22,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xe,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0x97,
0xf0,
0x90,
0xd1,
0xe0,
0x74,
0xf,
0xf0,
0x90,
0xd1,
0xe2,
0x74,
0xdb,
0xf0,
0x90,
0xf0,
0x81,
0x74,
0x5,
0xf0,
0x2,
0x30,
0xfa,
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
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x72,
0x70,
0x63,
0x20,
0x49,
0x4f,
0x35,
0x31,
0x20,
0x72,
0x78,
0x20,
0x64,
0x61,
0x74,
0x61,
0x20,
0x66,
0x72,
0x6f,
0x6d,
0x20,
0x50,
0x43,
0x2e,
0x2e,
0x2e,
0x2e,
0x2e,
0xd,
0xa,
0x0,
0xff,
0x38,
0x0,
0x25,
0xde,
0xd0,
0xfb,
0x8f,
0x64,
0x67,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x0,
0x75,
0x89,
0x10,
0x75,
0x8b,
0xff,
0x75,
0x8d,
0xff,
0xd2,
0x8e,
0x30,
0x8f,
0xfd,
0xc2,
0x8e,
0xc2,
0x8f,
0x22,
0x8f,
0x18,
0x8e,
0x17,
0x8d,
0x16,
0x8c,
0x15,
0xaf,
0x18,
0xae,
0x17,
0xad,
0x16,
0xac,
0x15,
0x8f,
0x1c,
0x8e,
0x1b,
0x8d,
0x1a,
0x8c,
0x19,
0xc2,
0xaf,
0x12,
0x38,
0x50,
0xaf,
0x1c,
0xae,
0x1b,
0xad,
0x1a,
0xac,
0x19,
0x7b,
0xff,
0x7a,
0xff,
0x79,
0xff,
0x78,
0xff,
0x12,
0x19,
0xa3,
0x8f,
0x1c,
0x8e,
0x1b,
0x8d,
0x1a,
0x8c,
0x19,
0xaf,
0x1c,
0xae,
0x1b,
0xad,
0x1a,
0xac,
0x19,
0xec,
0x4d,
0x4e,
0x4f,
0x70,
0xd4,
0xd2,
0xaf,
0x22,
0x75,
0x89,
0x10,
0x75,
0x8b,
0x7b,
0x75,
0x8d,
0xfc,
0xd2,
0x8e,
0x30,
0x8f,
0xfd,
0xc2,
0x8e,
0xc2,
0x8f,
0x22,
0x8f,
0x42,
0x8e,
0x41,
0x8d,
0x40,
0x8c,
0x3f,
0xaf,
0x42,
0xae,
0x41,
0xad,
0x40,
0xac,
0x3f,
0x8f,
0x46,
0x8e,
0x45,
0x8d,
0x44,
0x8c,
0x43,
0xc2,
0xaf,
0x12,
0x38,
0xac,
0xaf,
0x46,
0xae,
0x45,
0xad,
0x44,
0xac,
0x43,
0x7b,
0xff,
0x7a,
0xff,
0x79,
0xff,
0x78,
0xff,
0x12,
0x19,
0xa3,
0x8f,
0x46,
0x8e,
0x45,
0x8d,
0x44,
0x8c,
0x43,
0xaf,
0x46,
0xae,
0x45,
0xad,
0x44,
0xac,
0x43,
0xec,
0x4d,
0x4e,
0x4f,
0x70,
0xd4,
0xd2,
0xaf,
0x22,
0x90,
0xdf,
0x1,
0x74,
0x1,
0xf0,
0x7f,
0x5,
0x7e,
0x0,
0x7d,
0x0,
0x7c,
0x0,
0x12,
0x38,
0xbf,
0x90,
0xdf,
0x1,
0xe4,
0xf0,
0x7f,
0x5,
0x7e,
0x0,
0x7d,
0x0,
0x7c,
0x0,
0x12,
0x38,
0xbf,
0x22,
0xc2,
0xaf,
0x90,
0xdf,
0x9,
0xe0,
0xff,
0x8f,
0x3f,
0xe5,
0x3f,
0x54,
0xfc,
0xff,
0xef,
0x13,
0x13,
0x54,
0x3f,
0xf5,
0x3f,
0xd2,
0xaf,
0xaf,
0x3f,
0x22,
0xc2,
0xaf,
0x90,
0xdf,
0xa,
0xe0,
0xff,
0x8f,
0x3f,
0xe5,
0x3f,
0x54,
0x40,
0xff,
0xef,
0xc4,
0x13,
0x13,
0x54,
0x3,
0xf5,
0x3f,
0x90,
0xf0,
0x90,
0xe5,
0x3f,
0xf0,
0xd2,
0xaf,
0xaf,
0x3f,
0x22,
0xc2,
0xaf,
0x90,
0xdf,
0x4d,
0xe0,
0xff,
0x7e,
0x0,
0x8e,
0x3f,
0x8f,
0x40,
0x53,
0x40,
0x1f,
0x53,
0x3f,
0x0,
0xaf,
0x40,
0x90,
0xf0,
0x88,
0xef,
0xf0,
0x90,
0xdf,
0x4c,
0xe0,
0xff,
0x7e,
0x0,
0x8e,
0x41,
0x8f,
0x42,
0x53,
0x42,
0xff,
0x53,
0x41,
0x0,
0xaf,
0x42,
0x90,
0xf0,
0x87,
0xef,
0xf0,
0xe5,
0x40,
0x7f,
0x0,
0xfe,
0xee,
0x45,
0x41,
0xf5,
0x43,
0xef,
0x45,
0x42,
0xf5,
0x44,
0xd2,
0xaf,
0xae,
0x43,
0xaf,
0x44,
0x22,
0xc2,
0xaf,
0x90,
0xdf,
0xd5,
0xe0,
0xff,
0x7e,
0x0,
0x8e,
0x3f,
0x8f,
0x40,
0x53,
0x40,
0x1,
0x53,
0x3f,
0x0,
0xaf,
0x40,
0x90,
0xf0,
0x86,
0xef,
0xf0,
0x90,
0xdf,
0xbe,
0xe0,
0xff,
0x7e,
0x0,
0x8e,
0x41,
0x8f,
0x42,
0x53,
0x42,
0x7f,
0x53,
0x41,
0x0,
0xaf,
0x42,
0x90,
0xf0,
0x85,
0xef,
0xf0,
0x90,
0xdf,
0xbd,
0xe0,
0xff,
0x7e,
0x0,
0x8e,
0x43,
0x8f,
0x44,
0x53,
0x44,
0xf0,
0x53,
0x43,
0x0,
0xaf,
0x44,
0x90,
0xf0,
0x84,
0xef,
0xf0,
0xe5,
0x42,
0xc4,
0xf8,
0x54,
0xf,
0xc8,
0x68,
0xfd,
0xe5,
0x41,
0xc4,
0x54,
0xf0,
0x48,
0xfc,
0xe5,
0x40,
0x7f,
0x0,
0x33,
0x33,
0x33,
0x54,
0xf8,
0xfe,
0xee,
0x4c,
0xfc,
0xef,
0x4d,
0xfd,
0xe5,
0x43,
0xc4,
0xf8,
0x54,
0xf0,
0xc8,
0x68,
0xfe,
0xe5,
0x44,
0xc4,
0x54,
0xf,
0x48,
0xff,
0xee,
0x4c,
0xf5,
0x45,
0xef,
0x4d,
0xf5,
0x46,
0xd2,
0xaf,
0xae,
0x45,
0xaf,
0x46,
0x22,
0xc2,
0xaf,
0x90,
0xde,
0x4f,
0xe0,
0xff,
0xe4,
0xfc,
0xfd,
0xfe,
0x8f,
0x42,
0x8e,
0x41,
0x8d,
0x40,
0x8c,
0x3f,
0xe5,
0x42,
0x54,
0x1,
0xff,
0xe5,
0x41,
0x54,
0x0,
0xfe,
0xe5,
0x40,
0x54,
0x0,
0xfd,
0xe5,
0x3f,
0x54,
0x0,
0xfc,
0x8f,
0x42,
0x8e,
0x41,
0x8d,
0x40,
0x8c,
0x3f,
0xaf,
0x42,
0x90,
0xf0,
0x8c,
0xef,
0xf0,
0x90,
0xde,
0x4e,
0xe0,
0xff,
0xe4,
0xfc,
0xfd,
0xfe,
0x8f,
0x46,
0x8e,
0x45,
0x8d,
0x44,
0x8c,
0x43,
0xe5,
0x46,
0x54,
0xff,
0xff,
0xe5,
0x45,
0x54,
0x0,
0xfe,
0xe5,
0x44,
0x54,
0x0,
0xfd,
0xe5,
0x43,
0x54,
0x0,
0xfc,
0x8f,
0x46,
0x8e,
0x45,
0x8d,
0x44,
0x8c,
0x43,
0xaf,
0x46,
0x90,
0xf0,
0x8b,
0xef,
0xf0,
0x90,
0xde,
0x4d,
0xe0,
0xff,
0xe4,
0xfc,
0xfd,
0xfe,
0x8f,
0x4a,
0x8e,
0x49,
0x8d,
0x48,
0x8c,
0x47,
0xe5,
0x4a,
0x54,
0xff,
0xff,
0xe5,
0x49,
0x54,
0x0,
0xfe,
0xe5,
0x48,
0x54,
0x0,
0xfd,
0xe5,
0x47,
0x54,
0x0,
0xfc,
0x8f,
0x4a,
0x8e,
0x49,
0x8d,
0x48,
0x8c,
0x47,
0xaf,
0x4a,
0x90,
0xf0,
0x8a,
0xef,
0xf0,
0x90,
0xde,
0x4c,
0xe0,
0xff,
0xe4,
0xfc,
0xfd,
0xfe,
0x8f,
0x4e,
0x8e,
0x4d,
0x8d,
0x4c,
0x8c,
0x4b,
0xe5,
0x4e,
0x54,
0xff,
0xff,
0xe5,
0x4d,
0x54,
0x0,
0xfe,
0xe5,
0x4c,
0x54,
0x0,
0xfd,
0xe5,
0x4b,
0x54,
0x0,
0xfc,
0x8f,
0x4e,
0x8e,
0x4d,
0x8d,
0x4c,
0x8c,
0x4b,
0xaf,
0x4e,
0x90,
0xf0,
0x89,
0xef,
0xf0,
0xaf,
0x46,
0xae,
0x45,
0xad,
0x44,
0xac,
0x43,
0x78,
0x10,
0x12,
0x1b,
0x3f,
0xc0,
0x4,
0xc0,
0x5,
0xc0,
0x6,
0xc0,
0x7,
0xaf,
0x42,
0xae,
0x41,
0xad,
0x40,
0xac,
0x3f,
0x78,
0x18,
0x12,
0x1b,
0x3f,
0xa8,
0x4,
0xa9,
0x5,
0xaa,
0x6,
0xab,
0x7,
0xd0,
0x7,
0xd0,
0x6,
0xd0,
0x5,
0xd0,
0x4,
0x12,
0x1a,
0xe8,
0xc0,
0x4,
0xc0,
0x5,
0xc0,
0x6,
0xc0,
0x7,
0xaf,
0x4a,
0xae,
0x49,
0xad,
0x48,
0xac,
0x47,
0x78,
0x8,
0x12,
0x1b,
0x3f,
0xa8,
0x4,
0xa9,
0x5,
0xaa,
0x6,
0xab,
0x7,
0xd0,
0x7,
0xd0,
0x6,
0xd0,
0x5,
0xd0,
0x4,
0x12,
0x1a,
0xe8,
0xef,
0x45,
0x4e,
0xff,
0xee,
0x45,
0x4d,
0xfe,
0xed,
0x45,
0x4c,
0xfd,
0xec,
0x45,
0x4b,
0xfc,
0x8f,
0x52,
0x8e,
0x51,
0x8d,
0x50,
0x8c,
0x4f,
0xd2,
0xaf,
0xaf,
0x52,
0xae,
0x51,
0xad,
0x50,
0xac,
0x4f,
0x22,
0x90,
0xf0,
0x2f,
0xe0,
0xff,
0x8f,
0x59,
0x90,
0xf0,
0x3f,
0xe0,
0xff,
0x8f,
0x5a,
0x53,
0x5a,
0xfe,
0xe5,
0x59,
0x54,
0x1,
0xff,
0xef,
0x45,
0x5a,
0xff,
0x90,
0xf0,
0x3f,
0xef,
0xf0,
0xe5,
0x59,
0x70,
0x3,
0x7f,
0x0,
0x22,
0x85,
0x51,
0x5f,
0xab,
0x51,
0xad,
0x51,
0xaf,
0x58,
0xae,
0x57,
0x12,
0x3,
0x68,
0x90,
0xf0,
0x30,
0xe0,
0xff,
0x8f,
0x50,
0xe5,
0x50,
0x60,
0x3,
0x2,
0x3c,
0xb9,
0x90,
0xf0,
0x31,
0xe0,
0xff,
0x8f,
0x52,
0x90,
0xf0,
0x32,
0xe0,
0xff,
0x8f,
0x53,
0x85,
0x52,
0x55,
0x85,
0x53,
0x56,
0xae,
0x55,
0xaf,
0x56,
0x8f,
0x82,
0x8e,
0x83,
0xe0,
0xff,
0x8f,
0x59,
0x90,
0xf0,
0x33,
0xe5,
0x59,
0xf0,
0xae,
0x55,
0xaf,
0x56,
0x8f,
0x82,
0x8e,
0x83,
0xa3,
0xe0,
0xff,
0x8f,
0x59,
0x90,
0xb1,
0x23
};


