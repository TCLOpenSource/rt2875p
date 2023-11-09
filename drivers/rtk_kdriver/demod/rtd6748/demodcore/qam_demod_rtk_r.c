/**

@file

@brief   REALTEK_R QAM demod module definition

One can manipulate REALTEK_R QAM demod through REALTEK_R QAM module.
REALTEK_R QAM module is derived from QAM demod module.

*/


//#include <stdio.h>
//#include <string.h>
#include <rtk_kdriver/io.h>
#include "qam_demod_rtk_r.h"
#include "rbus/dvbc_reg.h"
#include "rbus/rtk_dtv_demod_sys_reg.h"

#define U8  unsigned char
#define U16 unsigned int
#define U32 unsigned long
#define S8  char
#define S16 int
#define S32 long

#define BYTE0(var) (*( unsigned char *) &var)
#define BYTE1(var) (*((unsigned char *) &var + 1))
#define BYTE2(var) (*((unsigned char *) &var + 2))
#define BYTE3(var) (*((unsigned char *) &var + 3))

const S32 SymbolRateOffset[] = {0, 5, -5, 10, -10, 15, -15, 20, -20, 25, -25}; //kHz
const U8 SizeOfSymbolRateOffset = sizeof(SymbolRateOffset) / sizeof(S32);

U32BITS m_BerNum = 0, m_BerDen = 0, m_PerNum = 0, m_PerDen = 0;
unsigned int gSfi_cancel_wn_15_8 = 0, gSfi_cancel_wn_7_0 = 0;
unsigned int gCcfo_idx_7_0 = 0, gCcfo_idx_12_8 = 0;
unsigned char gCorse_sfi_found = 0;
//#define GET_SIGNAL_STRENGTH_CUSTOMER_SONY

U8 read_reg(QAM_DEMOD_MODULE *pDemod, U16 addr, S8 *ret)
{
#if 0
	unsigned long r_reg;

	*ret = 0;
	if (pDemod->RegAccess.Addr16Bit.GetRegMaskBits(pDemod, addr, 7, 0, &r_reg) != FUNCTION_SUCCESS)
		*ret = -1;
	return ((U8)r_reg);
#endif
	return 0;
}


U8 write_reg(QAM_DEMOD_MODULE *pDemod, U16 addr, U8 val, S8 *ret)
{
#if 0
	*ret = 0;
	if (pDemod->RegAccess.Addr16Bit.SetRegMaskBits(pDemod, addr, 7, 0, val) != FUNCTION_SUCCESS)
		*ret = -1;
#endif
	return 0;
}


U8 dvbc_Readbits1B(QAM_DEMOD_MODULE *pDemod, U16 start_adr, U8 mask, U8 lsb)
{
#if 0
	U8 r_reg;
	S8 ret = 0;

	read_reg(pDemod, start_adr, &ret);
	if (ret < 0)
		return FUNCTION_ERROR;

	r_reg = (r_reg & mask) >> lsb;

	return r_reg;
#endif
	return 0;
}


#if 0
static U8 dvbc_Writebits1B(QAM_DEMOD_MODULE *pDemod, U16 start_adr, U8 mask, U8 lsb, U8 reg)
{
#if 0
	U8 w_reg, r_reg;
	S8 ret = 0;

	read_reg(pDemod, start_adr, &ret);
	if (ret < 0)
		return FUNCTION_ERROR;
	w_reg = (r_reg & ~mask);
	w_reg = w_reg | (reg << lsb);
	write_reg(pDemod, start_adr, w_reg, &ret);
	if (ret < 0)
		return FUNCTION_ERROR;
#endif
	return 0;
}
#endif


U8 dvbc_Getmask(U8 msb, U8 lsb)
{
#if 0
	U8 i, mask;

	mask = 0;
	for (i = lsb; i < msb + 1; i++)
		mask = mask | (0x1 << i);

	return mask;
#endif
	return 0;
}


#if 0
static S8   dvbc_Readbits1BS(QAM_DEMOD_MODULE *pDemod, U16 start_adr, U8 mask, U8 msb, U8 lsb)
{
#if 0
	S8 r_reg;
	S8 ret = 0;
	r_reg = read_reg(pDemod, start_adr, &ret);

	r_reg = (r_reg & mask) << (7 - msb);
	r_reg = (r_reg) >> (7 - msb + lsb);

	return r_reg;
#endif
	return 0;
}
#endif


/**

@brief   REALTEK_R QAM demod module builder

Use BuildRealtekRQamModule() to build REALTEK_R QAM module, set all module function pointers with the corresponding
functions, and initialize module private variables.

@param [in]   ppDemod                        Pointer to REALTEK_R QAM demod module pointer
@param [in]   pQamDemodModuleMemory          Pointer to an allocated QAM demod module memory
@param [in]   pBaseInterfaceModuleMemory     Pointer to an allocated base interface module memory
@param [in]   pI2cBridgeModuleMemory         Pointer to an allocated I2C bridge module memory
@param [in]   DeviceAddr                     REALTEK_R QAM I2C device address
@param [in]   CrystalFreqHz                  REALTEK_R QAM crystal frequency in Hz
@param [in]   ConfigMode                     REALTEK_R QAM configuration mode for setting

@note
	-# One should call BuildRealtekRQamModule() to build REALTEK_R QAM module before using it.

*/
void
BuildRealtekRQamModule(
	QAM_DEMOD_MODULE **ppDemod,
	QAM_DEMOD_MODULE *pQamDemodModuleMemory,
	BASE_INTERFACE_MODULE *pBaseInterfaceModuleMemory,
	I2C_BRIDGE_MODULE *pI2cBridgeModuleMemory,
	unsigned char DeviceAddr,
	U32BITS CrystalFreqHz,
	int ConfigMode
)
{
	QAM_DEMOD_MODULE *pDemod;
	REALTEK_R_QAM_EXTRA_MODULE *pExtra;

	// Set demod module pointer and get demod module.
	*ppDemod = pQamDemodModuleMemory;
	pDemod = *ppDemod;

	// Get demod extra module.
	pExtra = &(pDemod->Extra.RealtekRQam);

	// Set base interface module pointer and I2C bridge module pointer.
	pDemod->pBaseInterface = pBaseInterfaceModuleMemory;
	//pDemod->pI2cBridge     = pI2cBridgeModuleMemory;

	// Set demod type.
	pDemod->DemodType = QAM_DEMOD_TYPE_REALTEK_R_QAM;

	// Set demod I2C device address.
	pDemod->DeviceAddr = DeviceAddr;

	// Set demod crystal frequency in Hz.
	pDemod->CrystalFreqHz = CrystalFreqHz;

	// Set demod TS interface mode.
	pDemod->TsInterfaceMode = NO_USE;

	// Initialize demod parameter setting status
	pDemod->IsQamModeSet       = NO;
	pDemod->IsSymbolRateHzSet  = NO;
	pDemod->IsAlphaModeSet     = NO;
	pDemod->IsIfFreqHzSet      = NO;
	pDemod->IsSpectrumModeSet  = NO;

	// Initialize register tables in demod extra module.
	//realtek_r_qam_InitBaseRegTable(pDemod);
	//realtek_r_qam_InitMonitorRegTable(pDemod);

	// Build I2C birdge module.
	//realtek_r_qam_BuildI2cBridgeModule(pDemod);

	// Set demod module I2C function pointers with default functions.
	//pDemod->RegAccess.Addr16Bit.SetRegBytes        = qam_demod_addr_16bit_default_SetRegBytes;
	//pDemod->RegAccess.Addr16Bit.GetRegBytes        = qam_demod_addr_16bit_default_GetRegBytes;
	//pDemod->RegAccess.Addr16Bit.GetRegMaskBits     = qam_demod_addr_16bit_default_GetRegMaskBits;
	//pDemod->RegAccess.Addr16Bit.SetRegBits         = qam_demod_addr_16bit_default_SetRegBits;
	//pDemod->RegAccess.Addr16Bit.GetRegBits         = qam_demod_addr_16bit_default_GetRegBits;

	// Set demod module I2C function pointers with particular functions.
	//pDemod->RegAccess.Addr16Bit.SetRegMaskBits     = realtek_r_qam_acceleration_SetRegMaskBits;

	// Set demod module manipulating function pointers with default functions.
	pDemod->GetDemodType     = qam_demod_default_GetDemodType;
	pDemod->GetDeviceAddr    = qam_demod_default_GetDeviceAddr;
	pDemod->GetCrystalFreqHz = qam_demod_default_GetCrystalFreqHz;

	pDemod->GetQamMode       = qam_demod_default_GetQamMode;
	pDemod->GetSymbolRateHz  = qam_demod_default_GetSymbolRateHz;
	pDemod->GetAlphaMode     = qam_demod_default_GetAlphaMode;
	pDemod->GetIfFreqHz      = qam_demod_default_GetIfFreqHz;
	pDemod->GetSpectrumMode  = qam_demod_default_GetSpectrumMode;

	// Set demod module manipulating function pointers with particular functions.
	// Note: Need to assign manipulating function pointers according to configuration mode.
	pDemod->IsConnectedToI2c  = realtek_r_qam_IsConnectedToI2c;
	pDemod->SoftwareReset     = realtek_r_qam_SoftwareReset;

	pDemod->SetSymbolRateHz   = realtek_r_qam_SetSymbolRateHz;
	pDemod->SetIfFreqHz       = realtek_r_qam_SetIfFreqHz;
	pDemod->SetSpectrumMode   = realtek_r_qam_SetSpectrumMode;
	pDemod->SetAagcLoopPol    = realtek_r_qam_SetAagcLoopPol;
	pDemod->SetFftMode	= realtek_r_qam_SetFftMode;

	pDemod->GetRfAgc          = realtek_r_qam_GetRfAgc;
	pDemod->GetIfAgc          = realtek_r_qam_GetIfAgc;
	pDemod->GetDiAgc          = realtek_r_qam_GetDiAgc;
	pDemod->GetTrOffsetPpm    = realtek_r_qam_GetTrOffsetPpm;
	pDemod->GetCrOffsetHz     = realtek_r_qam_GetCrOffsetHz;

	pDemod->IsAagcLocked      = realtek_r_qam_IsAagcLocked;
	pDemod->IsEqLocked        = realtek_r_qam_IsEqLocked;

	pDemod->GetSnrDb          = realtek_r_qam_GetSnrDb;

	//pDemod->GetSignalStrength = realtek_r_qam_GetSignalStrength;
	//pDemod->GetSignalQuality  = realtek_r_qam_GetSignalQuality;

	pDemod->RSReset = realtek_r_qam_RSReset;

	pDemod->GetSrsCcfoDoneFlag = realtek_r_qam_GetSrsCcfoDoneFlag;


	switch (ConfigMode) {
	case REALTEK_R_QAM_CONFIG_OC:
	case REALTEK_R_QAM_CONFIG_OC_AM_HUM_EN:
		REALTEK_R_QAM_BASE_INFO("ConfigMode REALTEK_R_QAM_CONFIG_OC\n");
		pDemod->Initialize     = realtek_r_oc_Initialize;
		pDemod->SetQamMode     = realtek_r_oc_SetQamMode;
		pDemod->SetAlphaMode   = realtek_r_oc_SetAlphaMode;
		pDemod->GetErrorRate   = realtek_r_oc_GetErrorRate;
		pDemod->GetSignalStrength = realtek_r_oc_GetSignalStrength;
		pDemod->GetSignalQuality  = realtek_r_oc_GetSignalQuality;
		pDemod->IsFrameLocked  = realtek_r_oc_IsFrameLocked;
		pDemod->IsTrLocked     = realtek_r_oc_IsTrLocked;
		pDemod->IsCrLocked     = realtek_r_oc_IsCrLocked;
		pDemod->IsInnerLocked  = realtek_r_oc_IsInnerLocked;
		pDemod->IsNoSig  = realtek_r_oc_IsNoSig;
		pDemod->UpdateFunction = realtek_r_oc_UpdateFunction;
		pDemod->ResetFunction  = realtek_r_oc_ResetFunction;
		pDemod->NbdAndSrs      = realtek_r_oc_NbdAndSrs;
		pDemod->AutoQamd       = realtek_r_oc_Qamd;
		//pDemod->FixFsm         = realtek_r_oc_FixFsm;
		pDemod->AutoMode = realtek_r_oc_AutoMode;
		pDemod->AutoModePreset = realtek_r_oc_AutoModePreset;
		pDemod->AutoModePostset = realtek_r_oc_AutoModePostset;
		pDemod->SetSfiCancel = realtek_r_oc_SetSfiCancel;
		pDemod->SfiFullTracking = realtek_r_oc_SfiFullTracking;
		break;

	case REALTEK_R_QAM_CONFIG_DVBC:
	case REALTEK_R_QAM_CONFIG_DVBC_AM_HUM_EN:
		REALTEK_R_QAM_BASE_INFO("ConfigMode REALTEK_R_QAM_CONFIG_DVBC\n");
		pDemod->Initialize     = realtek_r_dvbc_Initialize;
		pDemod->SetQamMode     = realtek_r_dvbc_SetQamMode;
		pDemod->GetQamMode       = realtek_r_dvbc_GetQamMode;
		pDemod->SetAlphaMode   = realtek_r_dvbc_SetAlphaMode;
		pDemod->GetErrorRate   = realtek_r_dvbc_GetErrorRate;
		pDemod->GetSignalStrength = realtek_r_qam_GetSignalStrength;
		pDemod->GetSignalQuality  = realtek_r_qam_GetSignalQuality;
		pDemod->IsFrameLocked  = realtek_r_dvbc_IsFrameLocked;
		pDemod->IsTrLocked     = realtek_r_dvbc_IsTrLocked;
		pDemod->IsCrLocked     = realtek_r_dvbc_IsCrLocked;
		pDemod->IsInnerLocked  = realtek_r_dvbc_IsInnerLocked;
		pDemod->IsNoSig  = realtek_r_dvbc_IsNoSig;
		pDemod->UpdateFunction = realtek_r_dvbc_UpdateFunction;
		pDemod->ResetFunction  = realtek_r_dvbc_ResetFunction;
		pDemod->NbdAndSrs = realtek_r_dvbc_NbdAndSrs;
		pDemod->AutoQamd = realtek_r_dvbc_Qamd;
		pDemod->AutoQamdRegPreset =  realtek_r_dvbc_QamRegPreset;
		pDemod->AutoQamdRegPostset =  realtek_r_dvbc_QamRegPostset;
		pDemod->AutoQamdPreCheck =  realtek_r_dvbc_QamPreCheck;
		pDemod->SetCcfoMethodEn = realtek_r_dvbc_SetCcfoMethodEn;
		pDemod->IsCcfoMethodEn = realtek_r_dvbc_IsCcfoMethodEn;
		pDemod->GetCcfoInitFreq = realtek_r_dvbc_GetCcfoInitFreq;
		pDemod->SetCcfoInitFreq = realtek_r_dvbc_SetCcfoInitFreq;
		pDemod->GetTsThroughput = realtek_r_dvbc_TSBitAccu;
		pDemod->SetAutoModeEn = realtek_r_dvbc_SetAutoModeEn;
		pDemod->AutoMode = realtek_r_dvbc_AutoMode;
		pDemod->AutoModePreset = realtek_r_dvbc_AutoModePreset;
		pDemod->AutoModePostset = realtek_r_dvbc_AutoModePostset;
		pDemod->AutoModeSetSrHz = realtek_r_dvbc_AutoModeSetSrHz;
		break;

	default:
		REALTEK_R_QAM_BASE_WARNING("Unknown ConfigMode\n");
		break;
	}

	// Set demod extra module variables.
	pExtra->ConfigMode   = ConfigMode;
	pExtra->MpegOffSyncCount  = 0;

	return;
}


/**

@see   QAM_DEMOD_FP_IS_CONNECTED_TO_I2C

*/
void
realtek_r_qam_IsConnectedToI2c(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	U32BITS ReadingValue, DvbcReadchip;

	// Set reading value to zero, and get SYS_VERSION value.
	// Note: Use GetRegBitsWithPage() to get register bits with page setting.
	ReadingValue = 0;
	DvbcReadchip = 0;
	DvbcReadchip = rtd_inl(DVBC_CHIP_VERSION_reg);
	//if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_SYS_VERSION, &ReadingValue) != FUNCTION_SUCCESS)
	ReadingValue = DVBC_CHIP_VERSION_get_chip_version(DvbcReadchip);

	// Compare SYS_VERSION value with REALTEK_R_QAM_SYS_VERSION_VALUE.
	if (ReadingValue == REALTEK_R_QAM_SYS_VERSION_VALUE)
		*pAnswer = YES;
	else
		*pAnswer = NO;

	return;
}


/**

@see   QAM_DEMOD_FP_SOFTWARE_RESET

*/
int
realtek_r_qam_SoftwareReset(
	QAM_DEMOD_MODULE *pDemod
)
{
	rtd_maskl(DVBC_OPT_QAM_ON_reg, ~DVBC_OPT_QAM_ON_opt_qam_on_mask, DVBC_OPT_QAM_ON_opt_qam_on(0));
	rtd_maskl(DVBC_OPT_QAM_ON_reg, ~DVBC_OPT_QAM_ON_opt_qam_on_mask, DVBC_OPT_QAM_ON_opt_qam_on(1));
	//pDemod->RegAccess.Addr16Bit.SetRegMaskBits(pDemod, 0xe004, 0, 0, 0);
	//pDemod->RegAccess.Addr16Bit.SetRegMaskBits(pDemod, 0xe004, 0, 0, 1);
	// Set and clear SOFT_RESET register bit.
	//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_SOFT_RESET, ON) != FUNCTION_SUCCESS)
	rtd_maskl(DVBC_OPT_SOFT_RESET_QAM_reg, ~DVBC_OPT_SOFT_RESET_QAM_opt_soft_reset_qam_mask, DVBC_OPT_SOFT_RESET_QAM_opt_soft_reset_qam(1));

	//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_SOFT_RESET, OFF) != FUNCTION_SUCCESS)
	rtd_maskl(DVBC_OPT_SOFT_RESET_QAM_reg, ~DVBC_OPT_SOFT_RESET_QAM_opt_soft_reset_qam_mask, DVBC_OPT_SOFT_RESET_QAM_opt_soft_reset_qam(0));

	return FUNCTION_SUCCESS;
}


/**

@see   QAM_DEMOD_FP_RSReset

*/
int
realtek_r_qam_RSReset(
	QAM_DEMOD_MODULE *pDemod
)
{
	//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_SOFT_RESET_FF, 1) != FUNCTION_SUCCESS)
	//	goto error_status_set_demod_registers;
	rtd_maskl(DVBC_VT_LSB_HD_reg, ~DVBC_VT_LSB_HD_soft_reset_ff_mask, DVBC_VT_LSB_HD_soft_reset_ff(1));
	//REALTEK_R_QAM_BASE_WARNING("<Outer RS Reset>\n");

	return FUNCTION_SUCCESS;
}


/**

@see   QAM_DEMOD_FP_SET_SYMBOL_RATE_HZ

*/
int
realtek_r_qam_SetSymbolRateHz(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS SymbolRateHz
)
{
	typedef struct {
		U32BITS TrDeciRatioRangeMin;
		unsigned char SymbolRateReg0;
		U32BITS SymbolRateValue[REALTEK_R_QAM_SYMBOL_RATE_VALUE_TABLE_LEN];
	}
	SYMBOL_RATE_ENTRY;

	static const SYMBOL_RATE_ENTRY SymbolRateTable[REALTEK_R_QAM_SYMBOL_RATE_TABLE_LEN] = {
		// TrDeciRatioRangeMin,	SymbolRateReg0,	{SymbolRateValue                              }
		{0x1a0000,				0x4,			{0,    0,   10,  14,    1,   988,  955,   977,   68,   257,   438}	},//extend length to avoid null point
		{0x160000,				0x5,			{0,    0,    2,  15,   19,  1017,  967,   950,   12,   208,   420}	},
		{0x0,					0x6,			{3,   1,  1015,  1013,   13,    35,    3,   952,   952,   107,   379} },
	};

	int i = 0;
	MPI MpiCrystalFreqHz, MpiSymbolRateHz, MpiMiddleResult, MpiNone;
	U32BITS CrystalFreqHz = 0;
	const SYMBOL_RATE_ENTRY *pSymbolRateEntry;
	S32BITS TrDeciRatio = 0;
	unsigned char SymbolRateReg0 = 0;
	U32BITS SymbolRateValue = 0;
	memset(&pSymbolRateEntry, 0 , sizeof(pSymbolRateEntry));
	memset(&MpiCrystalFreqHz, 0 , sizeof(MpiCrystalFreqHz));
	memset(&MpiSymbolRateHz, 0 , sizeof(MpiSymbolRateHz));
	memset(&MpiMiddleResult, 0 , sizeof(MpiMiddleResult));
	memset(&MpiNone, 0 , sizeof(MpiNone));

	// Get demod crystal frequency in Hz.
	pDemod->GetCrystalFreqHz(pDemod, &CrystalFreqHz);

	// Calculate TR_DECI_RATIO value.
	// Note: TR_DECI_RATIO = (crystal frequency in Hz) * pow(2, 18) / (symbol rate in Hz)
	MpiSetValue(&MpiCrystalFreqHz, CrystalFreqHz);
	MpiSetValue(&MpiSymbolRateHz,  SymbolRateHz);

	MpiLeftShift(&MpiMiddleResult, MpiCrystalFreqHz, REALTEK_R_QAM_TR_DECI_RATIO_FRAC_BIT_NUM);
	MpiDiv(&MpiMiddleResult, &MpiNone, MpiMiddleResult, MpiSymbolRateHz);
	MpiGetValue(MpiMiddleResult, &TrDeciRatio);

	pSymbolRateEntry = &SymbolRateTable[0];

	// Determine symbol rate entry according to TR_DECI_RATIO value and minimum of TR_DECI_RATIO range.
	for (i = 0; i < REALTEK_R_QAM_SYMBOL_RATE_TABLE_LEN; i++) {
		if (TrDeciRatio >= SymbolRateTable[i].TrDeciRatioRangeMin) {
			pSymbolRateEntry = &SymbolRateTable[i];

			break;
		}
	}

	// Set TR_DECI_RATIO with calculated value.
	//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_TR_DECI_RATIO, TrDeciRatio) != FUNCTION_SUCCESS)
	rtd_maskl(DVBC_OPT_TR_DECI_RATIO_reg, ~DVBC_OPT_TR_DECI_RATIO_opt_tr_deci_ratio_7_0_mask, DVBC_OPT_TR_DECI_RATIO_opt_tr_deci_ratio_7_0(TrDeciRatio & 0xff));
	rtd_maskl(DVBC_OPT_TR_DECI_RATIO_1_reg, ~DVBC_OPT_TR_DECI_RATIO_1_opt_tr_deci_ratio_15_8_mask , DVBC_OPT_TR_DECI_RATIO_1_opt_tr_deci_ratio_15_8((TrDeciRatio & 0xff00) >> 8));
	rtd_maskl(DVBC_OPT_TR_DECI_RATIO_2_reg, ~DVBC_OPT_TR_DECI_RATIO_2_opt_tr_deci_ratio_23_16_mask , DVBC_OPT_TR_DECI_RATIO_2_opt_tr_deci_ratio_23_16((TrDeciRatio & 0xff0000) >> 16));
	rtd_maskl(DVBC_AUTO_SRS_reg, ~DVBC_AUTO_SRS_opt_tr_deci_ratio_auto_mask, DVBC_AUTO_SRS_opt_tr_deci_ratio_auto(0));//diable auto set tr deci

	// Set SPEC_SYMBOL_RATE_REG_0 value with determined symbol rate entry.
	SymbolRateReg0 = pSymbolRateEntry->SymbolRateReg0;

	//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_SPEC_SYMBOL_RATE_REG_0, SymbolRateReg0) != FUNCTION_SUCCESS)
	rtd_maskl(DVBC_OPT_DDC_LPF_SCALING_reg, ~DVBC_OPT_DDC_LPF_SCALING_opt_ddc_lpf_scaling_mask, DVBC_OPT_DDC_LPF_SCALING_opt_ddc_lpf_scaling(SymbolRateReg0 & 0x7));

	// Set symbol rate value with determined symbol rate entry.
	for (i = 0; i < REALTEK_R_QAM_SYMBOL_RATE_VALUE_TABLE_LEN; i++) {
		// Get symbol rate value.
		SymbolRateValue = pSymbolRateEntry->SymbolRateValue[i];

		// Set symbol rate register selection.
		//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_SPEC_SYMBOL_RATE_SEL, i) != FUNCTION_SUCCESS)
		rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_sel_mask, DVBC_DDC_WR_STB_ddc_sel(i));

		// Set symbol rate register value.
		//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_SPEC_SYMBOL_RATE_VAL, SymbolRateValue) != FUNCTION_SUCCESS)
		rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_coeff_2_0_mask, DVBC_DDC_WR_STB_ddc_coeff_2_0(SymbolRateValue & 0x07));

		rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask, DVBC_DDC_WR_STB_1_ddc_coeff_9_3((SymbolRateValue & 0x3F8) >> 3));

		// Set symbol rate register strobe.
		// Note: REALTEK_R QAM hardware will clear strobe automatically.
		//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_SPEC_SYMBOL_RATE_STROBE, ON) != FUNCTION_SUCCESS)
		rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_wr_stb_mask, DVBC_DDC_WR_STB_ddc_wr_stb(1));
	}

	// Set demod symbol rate parameter.
	pDemod->SymbolRateHz      = SymbolRateHz;
	pDemod->IsSymbolRateHzSet = YES;

	return FUNCTION_SUCCESS;
}


/**

@see   QAM_DEMOD_FP_SET_IF_FREQ_HZ

*/
int
realtek_r_qam_SetIfFreqHz(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS IfFreqHz
)
{
	U32BITS CrystalFreqHz = 0;
	S32BITS DdcFreq = 0;

	MPI MpiCrystalFreqHz, MpiMiddleResult, MpiNone;

	memset(&MpiCrystalFreqHz, 0 , sizeof(MpiCrystalFreqHz));
	memset(&MpiMiddleResult, 0 , sizeof(MpiMiddleResult));
	memset(&MpiNone, 0 , sizeof(MpiNone));

	// Get demod crystal frequency in Hz.
	pDemod->GetCrystalFreqHz(pDemod, &CrystalFreqHz);

	// Calculate DDC_FREQ value.
	// Note: DDC_FREQ = floor((CrystalFreqHz - (IfFreqHz % CrystalFreqHz)) * pow(2, 15) / CrystalFreqHz)
	MpiSetValue(&MpiCrystalFreqHz, CrystalFreqHz);
	MpiSetValue(&MpiMiddleResult, CrystalFreqHz - (IfFreqHz % CrystalFreqHz));
	MpiLeftShift(&MpiMiddleResult, MpiMiddleResult, REALTEK_R_QAM_DDC_FREQ_FRAC_BIT_NUM);
	MpiDiv(&MpiMiddleResult, &MpiNone, MpiMiddleResult, MpiCrystalFreqHz);
	MpiGetValue(MpiMiddleResult, &DdcFreq);

	// Set DDC_FREQ with calculated value.
	// Note: Use SetRegBitsWithPage() to set register bits with page setting.
	//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_DDC_FREQ, DdcFreq) != FUNCTION_SUCCESS)
	rtd_maskl(DVBC_OPT_DDC_FREQ_reg, ~DVBC_OPT_DDC_FREQ_opt_ddc_freq_7_0_mask, DVBC_OPT_DDC_FREQ_opt_ddc_freq_7_0(DdcFreq)); //Set opt_ddc_freq[7:0]

	rtd_maskl(DVBC_OPT_DDC_FREQ_1_reg, ~DVBC_OPT_DDC_FREQ_1_opt_ddc_freq_14_8_mask, DVBC_OPT_DDC_FREQ_1_opt_ddc_freq_14_8((DdcFreq & 0x7F00) >> 8)); //Set opt_ddc_freq[14:8]

	// Set demod IF frequnecy parameter.
	pDemod->IfFreqHz      = IfFreqHz;
	pDemod->IsIfFreqHzSet = YES;

	return FUNCTION_SUCCESS;
}


/**

@see   QAM_DEMOD_FP_SET_SPECTRUM_MODE

*/
int
realtek_r_qam_SetSpectrumMode(
	QAM_DEMOD_MODULE *pDemod,
	int SpectrumMode
)
{
	static const char SpecInvValueTable[SPECTRUM_MODE_NUM] = {
		// SpecInv
		0,				// Normal spectrum
		1,				// Inverse spectrum
	};

	U32BITS SpecInv;

	// Get SPEC_INV value from spectrum inverse value table according to spectrum mode.
	SpecInv = SpecInvValueTable[SpectrumMode];

	// Set SPEC_INV with gotten value.
	// Note: Use SetRegBitsWithPage() to set register bits with page setting.
	//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_SPEC_INV, SpecInv) != FUNCTION_SUCCESS)
	rtd_maskl(DVBC_OPT_DDC_FREQ_1_reg, ~DVBC_OPT_DDC_FREQ_1_opt_spec_inv_mask, DVBC_OPT_DDC_FREQ_1_opt_spec_inv(SpecInv));

	// Set demod spectrum mode parameter.
	pDemod->SpectrumMode      = SpectrumMode;
	pDemod->IsSpectrumModeSet = YES;

	return FUNCTION_SUCCESS;
}


/**

@see   QAM_DEMOD_FP_SET_AAGC_LOOP_POL

*/
int
realtek_r_qam_SetAagcLoopPol(
	QAM_DEMOD_MODULE *pDemod,
	int AagcLoopPol
)
{
	// Set AAGC LOOP POL with gotten value.
	if (AagcLoopPol == 0x1)
		rtd_maskl(DVBC_OPT_ADC_NB_ON_reg, ~DVBC_OPT_ADC_NB_ON_opt_aagc_loop_pol_mask, DVBC_OPT_ADC_NB_ON_opt_aagc_loop_pol(0x1));
	else
		rtd_maskl(DVBC_OPT_ADC_NB_ON_reg, ~DVBC_OPT_ADC_NB_ON_opt_aagc_loop_pol_mask, DVBC_OPT_ADC_NB_ON_opt_aagc_loop_pol(0x0));

	return FUNCTION_SUCCESS;
}


/**

@see   QAM_DEMOD_FP_GET_RF_AGC

*/
int
realtek_r_qam_GetRfAgc(
	QAM_DEMOD_MODULE *pDemod,
	S32BITS *pRfAgc
)
{
	U32BITS RfAgcBinary;

	// Get RF AGC binary value from RF_AGC_VALUE monitor register bits.
	if (realtek_r_qam_GetMonitorRegBits(pDemod, QAM_RF_AGC_VALUE, &RfAgcBinary) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Convert RF AGC binary value to signed integer.
	*pRfAgc = BinToSignedInt(RfAgcBinary, REALTEK_R_QAM_RF_AGC_VALUE_BIT_NUM);

	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
	return FUNCTION_ERROR;
}


/**

@see   QAM_DEMOD_FP_GET_IF_AGC

*/
int
realtek_r_qam_GetIfAgc(
	QAM_DEMOD_MODULE *pDemod,
	S32BITS *pIfAgc
)
{
	U32BITS IfAgcBinary;

	// Get IF AGC binary value from IF_AGC_VALUE monitor register bits.
	if (realtek_r_qam_GetMonitorRegBits(pDemod, QAM_IF_AGC_VALUE, &IfAgcBinary) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Convert IF AGC binary value to signed integer.
	*pIfAgc = BinToSignedInt(IfAgcBinary, REALTEK_R_QAM_IF_AGC_VALUE_BIT_NUM);

	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
	return FUNCTION_ERROR;
}


/**

@see   QAM_DEMOD_FP_GET_DI_AGC

*/
int
realtek_r_qam_GetDiAgc(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS *pDiAgc
)
{
	// Get digital AGC value from DAGC_VALUE monitor register bits.
	if (realtek_r_qam_GetMonitorRegBits(pDemod, QAM_DAGC_VALUE, pDiAgc) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
	return FUNCTION_ERROR;
}


/**

@see   QAM_DEMOD_FP_GET_TR_OFFSET_PPM

*/
int
realtek_r_qam_GetTrOffsetPpm(
	QAM_DEMOD_MODULE *pDemod,
	S32BITS *pTrOffsetPpm
)
{
	U32BITS SymbolRateHz = 0;
	U32BITS CrystalFreqHz = 0;
	U32BITS TrOffsetBinary = 0;
	S32BITS          TrOffsetInt = 0;

	MPI MpiTrOffsetInt, MpiSymbolRateHz, MpiCrystalFreqHz, MpiVar0, MpiVar1;

	memset(&MpiTrOffsetInt, 0 , sizeof(MpiTrOffsetInt));
	memset(&MpiSymbolRateHz, 0 , sizeof(MpiSymbolRateHz));
	memset(&MpiCrystalFreqHz, 0 , sizeof(MpiCrystalFreqHz));
	memset(&MpiVar0, 0 , sizeof(MpiVar0));
	memset(&MpiVar1, 0 , sizeof(MpiVar1));

	// Get demod symbol rate in Hz.
	if (pDemod->GetSymbolRateHz(pDemod, &SymbolRateHz) != FUNCTION_SUCCESS)
		goto error_status_get_demod_symbol_rate;

	// Get demod crystal frequency in Hz.
	pDemod->GetCrystalFreqHz(pDemod, &CrystalFreqHz);

	// Get TR offset binary value from TR_OFFSET monitor register bits.
	if (realtek_r_qam_GetMonitorRegBits(pDemod, QAM_TR_OFFSET, &TrOffsetBinary) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Convert TR offset binary value to signed integer.
	TrOffsetInt = BinToSignedInt(TrOffsetBinary, REALTEK_R_QAM_TR_OFFSET_BIT_NUM);

	// Get TR offset in ppm.
	// Note: (TR offset in ppm) = ((TR offset integer) * (symbol rate in Hz) * 1000000) /
	//                            ((pow(2, 35) * (crystal frequency in Hz))
	//       TR offset integer is 31 bit value.
	MpiSetValue(&MpiTrOffsetInt, (S32BITS)TrOffsetInt);
	MpiSetValue(&MpiSymbolRateHz, (S32BITS)SymbolRateHz);
	MpiSetValue(&MpiCrystalFreqHz, (S32BITS)CrystalFreqHz);
	MpiSetValue(&MpiVar0, 1000000);
	MpiMul(&MpiVar0, MpiVar0, MpiTrOffsetInt);
	MpiMul(&MpiVar0, MpiVar0, MpiSymbolRateHz);
	MpiLeftShift(&MpiVar1, MpiCrystalFreqHz, 35);
	MpiDiv(&MpiVar0, &MpiVar1, MpiVar0, MpiVar1);
	MpiGetValue(MpiVar0, (S32BITS*)pTrOffsetPpm);

	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
error_status_get_demod_symbol_rate:
	return FUNCTION_ERROR;
}


/**

@see   QAM_DEMOD_FP_GET_CR_OFFSET_HZ

*/
int
realtek_r_qam_GetCrOffsetHz(
	QAM_DEMOD_MODULE *pDemod,
	S32BITS *pCrOffsetHz
)
{
	U32BITS SymbolRateHz = 0;
	U32BITS CrOffsetBinary = 0;
	S32BITS          CrOffsetInt = 0;
	S32BITS CroffsetRst = 0;

	MPI MpiCrOffsetInt, MpiSymbolRateHz, MpiMiddleResult;

	memset(&MpiCrOffsetInt, 0 , sizeof(MpiCrOffsetInt));
	memset(&MpiSymbolRateHz, 0 , sizeof(MpiSymbolRateHz));
	memset(&MpiMiddleResult, 0 , sizeof(MpiMiddleResult));

	// Get demod symbol rate in Hz.
	if (pDemod->GetSymbolRateHz(pDemod, &SymbolRateHz) != FUNCTION_SUCCESS)
		goto error_status_get_demod_symbol_rate;

	// Get CR offset binary value from CR_OFFSET monitor register bits.
	if (realtek_r_qam_GetMonitorRegBits(pDemod, QAM_CR_OFFSET, &CrOffsetBinary) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Convert CR offset binary value to signed integer.
	CrOffsetInt = BinToSignedInt(CrOffsetBinary, REALTEK_R_QAM_CR_OFFSET_BIT_NUM);

	// Get CR offset in Hz.
	// Note: (CR offset in Hz) = (CR offset integer) * (symbol rate in Hz) / pow(2, 34)
	//       CR offset integer is 32 bit value.
	MpiSetValue(&MpiCrOffsetInt, CrOffsetInt);
	MpiSetValue(&MpiSymbolRateHz, (S32BITS)SymbolRateHz);
	MpiMul(&MpiMiddleResult, MpiCrOffsetInt, MpiSymbolRateHz);
	MpiRightShift(&MpiMiddleResult, MpiMiddleResult, 34);
	MpiGetValue(MpiMiddleResult, &CroffsetRst);

	*pCrOffsetHz = CroffsetRst;
	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
error_status_get_demod_symbol_rate:
	return FUNCTION_ERROR;
}


/**

@see   QAM_DEMOD_FP_IS_AAGC_LOCKED

*/
int
realtek_r_qam_IsAagcLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	U32BITS LockStatus = NOT_LOCKED;

	// Get AAGC lock status from AAGC_LD inner strobe register bits.
	if (realtek_r_qam_GetInnerStrobeRegBits(pDemod, QAM_AAGC_LD, &LockStatus) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Determine answer according to AAGC lock status.
	if (LockStatus == LOCKED)
		*pAnswer = YES;
	else
		*pAnswer = NO;

	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
	return FUNCTION_ERROR;
}


/**

@see   QAM_DEMOD_FP_IS_EQ_LOCKED

*/
int
realtek_r_qam_IsEqLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	U32BITS LockStatus = NOT_LOCKED;

	// Get EQ lock status from EQ_LD inner strobe register bits.
	if (realtek_r_qam_GetInnerStrobeRegBits(pDemod, QAM_EQ_LD, &LockStatus) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Determine answer according to EQ lock status.
	if (LockStatus == LOCKED)
		*pAnswer = YES;
	else
		*pAnswer = NO;

	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
	return FUNCTION_ERROR;
}


/**

@see   QAM_DEMOD_FP_GET_SNR_DB

*/
int
realtek_r_qam_GetSnrDb(
	QAM_DEMOD_MODULE *pDemod,
	S32BITS *pSnrDbNum,
	S32BITS *pSnrDbDen
)
{
	static const U32BITS SnrDbNumConst[QAM_QAM_MODE_NUM] = {
		215040,			// for    4-QAM mode
		238817,			// for   16-QAM mode
		249057,			// for   32-QAM mode
		260017,			// for   64-QAM mode
		269901,			// for  128-QAM mode
		280672,			// for  256-QAM mode
		290381,			// for  512-QAM mode
		301196,			// for 1024-QAM mode
	};

	int QamMode = 0;

	U32BITS Mse = 0;
	S32BITS Var = 0;
	MPI MpiMse, MpiVar;

	memset(&MpiMse, 0 , sizeof(MpiMse));
	memset(&MpiVar, 0 , sizeof(MpiVar));

	// Get demod QAM mode.
	if (pDemod->GetQamMode(pDemod, &QamMode) != FUNCTION_SUCCESS)
		goto error_status_get_demod_qam_mode;

	// Get mean-square error from MSE.
	if (realtek_r_qam_GetInnerStrobeRegBits(pDemod, QAM_MSE, &Mse) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// SNR dB formula
	// Original formula: SNR_dB = 10 * log10(ConstWithQamMode * pow(2, 20) / MSE)
	// Adjusted formula: SNR_dB = (SNR_DB_NUM_CONST - 10 * log2(MSE) * pow(2, SNR_FRAC_BIT_NUM)) / SNR_DB_DEN
	//                   SNR_DB_NUM_CONST = 10 * log2(ConstWithQamMode * pow(2, 20)) * pow(2, SNR_FRAC_BIT_NUM)
	//                   SNR_DB_DEN       = log2(10) * pow(2, SNR_FRAC_BIT_NUM)
	// ConstWithQamMode: 2     for    4-QAM
	//                   10    for   16-QAM
	//                   20    for   32-QAM
	//                   42    for   64-QAM
	//                   82    for  128-QAM
	//                   170   for  256-QAM
	//                   328   for  512-QAM
	//                   682   for 1024-QAM

	// Calculate SNR dB numerator.
	MpiSetValue(&MpiMse, (S32BITS)Mse);
	MpiLog2(&MpiVar, MpiMse, REALTEK_R_QAM_SNR_FRAC_BIT_NUM);
	MpiGetValue(MpiVar, &Var);

	*pSnrDbNum = SnrDbNumConst[QamMode] - 10 * Var;

	// Set SNR dB denominator.
	*pSnrDbDen = REALTEK_R_QAM_SNR_DB_DEN;

	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
error_status_get_demod_qam_mode:
	return FUNCTION_ERROR;
}


/**

@see   QAM_DEMOD_FP_GET_SIGNAL_STRENGTH

*/
int
realtek_r_qam_GetSignalStrength(
	QAM_DEMOD_MODULE *pDemod,
	int TxRSSIdBm,
	U32BITS *pSignalStrength
)
{
	int FrameLock;
	S32BITS IfAgcValue;

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
	// Note: Map IF AGC value (1023 ~ -1024) to signal strength (0 ~ 100).
	*pSignalStrength = (102300 - IfAgcValue * 100) / 2047;

success_status_non_frame_lock:
	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
	return FUNCTION_ERROR;
}


/**

@see   QAM_DEMOD_FP_GET_SIGNAL_QUALITY

*/
int
realtek_r_qam_GetSignalQuality(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS *pSignalQuality
)
{
	int FrameLock = 0;
	U32BITS Mse = 0;
	S32BITS MiddleResult = 0;
	MPI MpiMse, MpiResult;

	memset(&MpiMse, 0 , sizeof(MpiMse));
	memset(&MpiResult, 0 , sizeof(MpiResult));

	// Get demod frame lock status.
	if (pDemod->IsFrameLocked(pDemod, &FrameLock) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// If demod is not frame-locked, set signal quality with zero.
	if (FrameLock == NO) {
		*pSignalQuality = 0;
		goto success_status_non_frame_lock;
	}

	// Get mean-square error from MSE.
	if (realtek_r_qam_GetInnerStrobeRegBits(pDemod, QAM_MSE, &Mse) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Determine signal quality according to MSE value.
	// Note: Map MSE value (pow(2, 19) ~ pow(2, 17)) to signal quality (0 ~ 100).
	//       If MSE value < pow(2, 17), signal quality is 100.
	//       If MSE value > pow(2, 19), signal quality is 0.
	if (Mse > 524288) {
		*pSignalQuality = 0;
	} else if (Mse < 131072) {
		*pSignalQuality = 100;
	} else {
		MpiSetValue(&MpiMse, (S32BITS)Mse);
		MpiLog2(&MpiResult, MpiMse, REALTEK_R_QAM_SIGNAL_QUALITY_FRAC_BIT_NUM);
		MpiGetValue(MpiResult, &MiddleResult);

		*pSignalQuality = (243200 - MiddleResult * 100) / 256;
	}

success_status_non_frame_lock:
	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
	return FUNCTION_ERROR;
}


/**

@see   QAM_DEMOD_FP_INITIALIZE

*/
int
realtek_r_oc_Initialize(
	QAM_DEMOD_MODULE *pDemod
)
{

#ifdef OC_AutoDetectMode
	U32BITS CrystalFreqHz = 25200000;
	unsigned int sr_idx_max = 0, sr_idx_min = 0, oc_sym_rate1 = 0, oc_sym_rate2 = 0;
	#endif


	REALTEK_R_QAM_BASE_INFO("realtek_r_oc_Initialize\n");

	//-----------------------------------------------------------------------------------------//
	//[Start] RTK OpenCable Init Table, Sync from Merlin K2L, 0xe22e~0xe266 sync from Mac6p DVB-C

	//rtd_part_outl(RTK_DTV_DEMOD_SYS_DEMOD_SEL_reg, 1, 0, 0x01);	//0xe401[1:0] = 0x01, demod_sel, 0x1: DVB-C
	rtd_maskl(RTK_DTV_DEMOD_SYS_DEMOD_SEL_reg, 0xfffffffc, 0x01);
	//rtd_part_outl(DVBC_OPT_QAM_STD_reg, 0, 0, 0x01);	//0xe003[0]   = 0x01, opt_qam_std (outer standard), 0x1: OpenCable
	rtd_maskl(DVBC_OPT_QAM_STD_reg, 0xfffffffe, 0x01);
	//rtd_part_outl(DVBC_OPT_QAM_ON_reg, 0, 0, 0x01);	//0xe004[0]   = 0x01
	//rtd_maskl(DVBC_OPT_QAM_ON_reg, 0xfffffffe, 0x01);

	//rtd_part_outl(DVBC_OPT_RF_AAGC_DRIVE_reg, 7, 0, 0x80);	//0xe006[7:0] = 0xc4,
	rtd_maskl(DVBC_OPT_RF_AAGC_DRIVE_reg, 0xffffff00, 0xc4);
	//rtd_part_outl(DVBC_OPT_RF_AAGC_DRIVE_reg, 5, 5, 0x00);	//0xe006[5]   = 0x01, [5]: 0x1: push-pull
	rtd_maskl(DVBC_OPT_RF_AAGC_DRIVE_reg, 0xffffffdf, 0x20);
	//rtd_part_outl(DVBC_DEBUG_REG_2_reg, 5, 3, 0x00);	//0xe00f[5:3] = 0x00
	rtd_maskl(DVBC_DEBUG_REG_2_reg, 0xffffffc7, 0x00);
	//rtd_part_outl(DVBC_OPT_MPEG_OUT_SEL_reg, 1, 0, 0x01);	//0xe026[7:0] = 0x01
	rtd_maskl(DVBC_OPT_MPEG_OUT_SEL_reg, 0xffffff00, 0x01);
	//rtd_part_outl(DVBC_REG_ADC_CLK_XOR_reg, 7, 0, 0x28);	//0xe060[2]   = 0x00
	rtd_maskl(DVBC_REG_ADC_CLK_XOR_reg, 0xfffffffb, 0x00);
	//rtd_part_outl(DVBC_OPT_AAGC_TARGET_AMP_reg, 7, 0, 0x64);	//0xe105[7:0] = 0x64
	rtd_maskl(DVBC_OPT_AAGC_TARGET_AMP_reg, 0xffffff00, 0x64);
	//rtd_part_outl(DVBC_OPT_DDC_FREQ_reg, 7, 0, 0xb7);	//0xe10d[7:0] = 0xb7
	rtd_maskl(DVBC_OPT_DDC_FREQ_reg, 0xffffff00, 0xb7);
	//rtd_part_outl(DVBC_OPT_DDC_FREQ_1_reg, 7, 0, 0x6d);	//0xe10e[7:0] = 0x6d
	rtd_maskl(DVBC_OPT_DDC_FREQ_1_reg, 0xffffff00, 0x6d);
	//rtd_part_outl(DVBC_OPT_DDC_LPF_SCALING_reg, 7, 0, 0x16);	//0xe10f[7:0] = 0x16
	rtd_maskl(DVBC_OPT_DDC_LPF_SCALING_reg, 0xffffff00, 0x16);
	//rtd_part_outl(DVBC_OPT_CR_ACQ_PWR_TH_reg, 7, 0, 0x00);	//0xe118[7:0] = 0x00
	rtd_maskl(DVBC_OPT_CR_ACQ_PWR_TH_reg, 0xffffff00, 0x00);
	//rtd_part_outl(DVBC_OPT_CR_METRIC_DD_MAX_reg, 7, 0, 0xd4);	//0xe119[7:0] = 0xd4
	rtd_maskl(DVBC_OPT_CR_METRIC_DD_MAX_reg, 0xffffff00, 0xd4);
	//rtd_part_outl(DVBC_OPT_BN_EQ_STOP_EN_reg, 7, 0, 0x6a);	//0xe11a[7:0] = 0x6a
	rtd_maskl(DVBC_OPT_BN_EQ_STOP_EN_reg, 0xffffff00, 0x6a);
	//rtd_part_outl(DVBC_OPT_BN_MSE_TH_reg, 7, 0, 0xc7);	//0xe11b[7:0] = 0xc7
	rtd_maskl(DVBC_OPT_BN_MSE_TH_reg, 0xffffff00, 0xc7);
	//rtd_part_outl(DVBC_OPT_BN_COND1_CNT_TH_reg, 7, 0, 0x1c);	//0xe11c[7:0] = 0x1c
	rtd_maskl(DVBC_OPT_BN_COND1_CNT_TH_reg, 0xffffff00, 0x1c);
	//rtd_part_outl(DVBC_OPT_BN_ERR_ACC_TH_reg, 7, 0, 0x07);	//0xe11d[7:0] = 0x07
	rtd_maskl(DVBC_OPT_BN_ERR_ACC_TH_reg, 0xffffff00, 0x07);
#ifdef ENABLE_ATSC_DEMOD_SPEED_UP
	//rtd_part_outl(DVBC_OPT_AK_DD_CR_POW_TH_reg, 7, 0, 0xd2);	//0xe11e[7:0] = 0xd2
	rtd_maskl(DVBC_OPT_AK_DD_CR_POW_TH_reg, 0xffffff00, 0xd2);
#else
	//rtd_part_outl(DVBC_OPT_AK_DD_CR_POW_TH_reg, 7, 0, 0x52);	//0xe11e[7:0] = 0x52
	rtd_maskl(DVBC_OPT_AK_DD_CR_POW_TH_reg, 0xffffff00, 0x52);
#endif
	//rtd_part_outl(DVBC_OPT_TR_DECI_RATIO_reg, 7, 0, 0xd9);	//0xe11f[7:0] = 0xd9
	rtd_maskl(DVBC_OPT_TR_DECI_RATIO_reg, 0xffffff00, 0xd9);
	//rtd_part_outl(DVBC_OPT_U_FFF_ACQ0_reg, 7, 0, 0x08);	//0xe13b[7:0] = 0x08
	rtd_maskl(DVBC_OPT_U_FFF_ACQ0_reg, 0xffffff00, 0x08);
	//rtd_part_outl(DVBC_OPT_U_FFF_ACQ2_reg, 7, 0, 0x21);	//0xe13c[7:0] = 0x21
	rtd_maskl(DVBC_OPT_U_FFF_ACQ2_reg, 0xffffff00, 0x21);
	//rtd_part_outl(DVBC_OPT_U_FBF_ACQ2_reg, 7, 0, 0x51);	//0xe13d[7:0] = 0x51
	rtd_maskl(DVBC_OPT_U_FBF_ACQ2_reg, 0xffffff00, 0x51);
	//rtd_part_outl(DVBC_OPT_U_FFF_DD0_reg, 7, 0, 0x11);	//0xe140[7:0] = 0x11
	rtd_maskl(DVBC_OPT_U_FFF_DD0_reg, 0xffffff00, 0x11);
	//rtd_part_outl(DVBC_OPT_U_FFF_DD2_reg, 7, 0, 0x4c);	//0xe141[7:0] = 0x4c
	rtd_maskl(DVBC_OPT_U_FFF_DD2_reg, 0xffffff00, 0x4c);
	//rtd_part_outl(DVBC_OPT_MSE_TH_COND6_1_reg, 6, 6, 0x01);	//0xe12a[6]   = 0x01
	rtd_maskl(DVBC_OPT_MSE_TH_COND6_1_reg, 0xffffffbf, 0x40);
	//rtd_part_outl(DVBC_OPT_RT_LOOP_CNT_MIN_reg, 7, 0, 0x30);	//0xe12c[7:0] = 0x30
	rtd_maskl(DVBC_OPT_RT_LOOP_CNT_MIN_reg, 0xffffff00, 0x30);
	//rtd_part_outl(DVBC_OPT_RT_MSE_TH_reg, 7, 0, 0xc8);	//0xe12d[7:0] = 0xc8
	rtd_maskl(DVBC_OPT_RT_MSE_TH_reg, 0xffffff00, 0xc8);
	//rtd_part_outl(DVBC_OPT_RT_FSM_STATE1_reg, 7, 0, 0xaa);	//0xe17a[7:0] = 0xaa
	rtd_maskl(DVBC_OPT_RT_FSM_STATE1_reg, 0xffffff00, 0xaa);
	//rtd_part_outl(DVBC_OPT_RT_FSM_CNT_MIN_reg, 7, 0, 0x03);	//0xe17b[7:0] = 0x03
	rtd_maskl(DVBC_OPT_RT_FSM_CNT_MIN_reg, 0xffffff00, 0x03);
	//rtd_part_outl(DVBC_OPT_RT_FSM_F_CNT_MIN_reg, 7, 0, 0x0a);	//0xe17c[7:0] = 0x0a
	rtd_maskl(DVBC_OPT_RT_FSM_F_CNT_MIN_reg, 0xffffff00, 0x0a);
	//rtd_part_outl(DVBC_OPT_AAGC_CNT_TH_reg, 7, 0, 0x08);	//0xe17d[7:0] = 0x08
	rtd_maskl(DVBC_OPT_AAGC_CNT_TH_reg, 0xffffff00, 0x08);
	//rtd_part_outl(DVBC_OPT_RT_FSM_F_J_CNT_MIN_reg, 7, 0, 0x14);	//0xe17e[7:0] = 0x14
	rtd_maskl(DVBC_OPT_RT_FSM_F_J_CNT_MIN_reg, 0xffffff00, 0x14);
	//rtd_part_outl(DVBC_OPT_RST_TR_reg, 7, 0, 0x0f);	//0xe17f[7:0] = 0x0f
	rtd_maskl(DVBC_OPT_RST_TR_reg, 0xffffff00, 0x0f);
	//rtd_part_outl(DVBC_LD_OUTER_TH_reg, 0, 0, 0x01);	//0xe188[0]   = 0x01
	rtd_maskl(DVBC_LD_OUTER_TH_reg, 0xfffffffe, 0x01);
	//rtd_part_outl(DVBC_OPT_U_FFF2_DD2_reg, 7, 0, 0x15);	//0xe198[7:0] = 0x15
	rtd_maskl(DVBC_OPT_U_FFF2_DD2_reg, 0xffffff00, 0x15);
	//rtd_part_outl(DVBC_OPT_U_FFF3_DD1_reg, 7, 0, 0x2b);	//0xe199[7:0] = 0x2b
	rtd_maskl(DVBC_OPT_U_FFF3_DD1_reg, 0xffffff00, 0x2b);
	//rtd_part_outl(DVBC_OPT_U_FFF4_DD0_reg, 7, 0, 0x1a);	//0xe19a[7:0] = 0x1a
	rtd_maskl(DVBC_OPT_U_FFF4_DD0_reg, 0xffffff00, 0x1a);
	//rtd_part_outl(DVBC_OPT_U_FFF4_DD2_reg, 7, 0, 0x75);	//0xe19b[7:0] = 0x75
	rtd_maskl(DVBC_OPT_U_FFF4_DD2_reg, 0xffffff00, 0x75);
	//rtd_part_outl(DVBC_OPT_U_FBF3_DD2_reg, 7, 0, 0xfb);	//0xe19c[7:0] = 0xfb
	rtd_maskl(DVBC_OPT_U_FBF3_DD2_reg, 0xffffff00, 0xfb);
	//rtd_part_outl(DVBC_OPT_AAGC_IQ_SWAP_reg, 7, 0, 0x00);	//0xe1b8[7:0] = 0x00
	rtd_maskl(DVBC_OPT_AAGC_IQ_SWAP_reg, 0xffffff00, 0x00);
	//rtd_part_outl(DVBC_OPT_FE_DCC1_EN_reg, 7, 0, 0x00);	//0xe1b9[7:0] = 0x00
	rtd_maskl(DVBC_OPT_FE_DCC1_EN_reg, 0xffffff00, 0x00);
	//rtd_part_outl(DVBC_OPT_FE_DCC1_U_reg, 7, 0, 0xc0);	//0xe1bb[7:0] = 0xc0
	rtd_maskl(DVBC_OPT_FE_DCC1_U_reg, 0xffffff00, 0xc0);
	//rtd_part_outl(DVBC_OPT_AAGC_LOOP_IN_reg, 7, 0, 0x00);	//0xe1bc[7:0] = 0x00
	rtd_maskl(DVBC_OPT_AAGC_LOOP_IN_reg, 0xffffff00, 0x00);
	//rtd_part_outl(DVBC_OPT_FE_DCC2_EN_reg, 7, 0, 0x00);	//0xe1bd[7:0] = 0x00
	rtd_maskl(DVBC_OPT_FE_DCC2_EN_reg, 0xffffff00, 0x00);
	//rtd_part_outl(DVBC_OPT_FE_DCC2_U_reg, 7, 0, 0x80);	//0xe1bf[7:0] = 0x80
	rtd_maskl(DVBC_OPT_FE_DCC2_U_reg, 0xffffff00, 0x80);
	//rtd_part_outl(DVBC_OPT_FE_DCC3_EN_reg, 7, 0, 0x02);	//0xe1c0[7:0] = 0x02
	rtd_maskl(DVBC_OPT_FE_DCC3_EN_reg, 0xffffff00, 0x02);
	//rtd_part_outl(DVBC_OPT_FE_DCC3_U_reg, 7, 0, 0x80);	//0xe1c2[7:0] = 0x80
	rtd_maskl(DVBC_OPT_FE_DCC3_U_reg, 0xffffff00, 0x80);
	//rtd_part_outl(DVBC_OPT_FE_IQ_U_reg, 7, 0, 0x80);	//0xe1c3[7:0] = 0x80
	rtd_maskl(DVBC_OPT_FE_IQ_U_reg, 0xffffff00, 0x80);
	//rtd_part_outl(DVBC_OPT_FE_IQAMP_EN_reg, 7, 0, 0x02);	//0xe1c4[7:0] = 0x02
	rtd_maskl(DVBC_OPT_FE_IQAMP_EN_reg, 0xffffff00, 0x02);
	//rtd_part_outl(DVBC_OPT_FE_IQPH_EN_reg, 7, 0, 0x02);	//0xe1c8[7:0] = 0x02
	rtd_maskl(DVBC_OPT_FE_IQPH_EN_reg, 0xffffff00, 0x02);

#ifdef OC_AutoDetectMode
	//{3,   1,  1015,  1013,   13,    35,    3,   952,   952,   107,   379} DDC_HD for automode
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x00));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x3));
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x0));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));
	
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x01));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x1));
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x0));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));
	
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x02));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x7));
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x7E));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));
	
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x03));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x5));
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x7E));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));
	
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x04));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x5));
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x1));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));
	
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x05));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x3));
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x4));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));
	
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x06));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x3));//3
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x0));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));
	
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x07));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x0));//952
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x77));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));
	
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x08));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x0));//952
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x77));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));
	
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x09));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x3));//107
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0xD));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));
	
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x0a));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x3));//379
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x2f));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));
#else
	//-------------[Start] Set ddc_hd(DDC LPF shape)------------------
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xffffffe0, 0x00);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xffffff00, 0x60);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, 0xffffff00, 0x7f);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xfffffffe, 0x01);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xffffffe0, 0x02);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xffffff00, 0x22);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, 0xffffff00, 0x7f);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xfffffffe, 0x01);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xffffffe0, 0x04);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xffffff00, 0x24);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, 0xffffff00, 0x01);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xfffffffe, 0x01);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xffffffe0, 0x06);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xffffff00, 0xa6);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, 0xffffff00, 0x03);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xfffffffe, 0x01);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xffffffe0, 0x08);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xffffff00, 0x68);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xfffffffe, 0x01);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xffffffe0, 0x0a);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xffffff00, 0xaa);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, 0xffffff00, 0x77);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xfffffffe, 0x01);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xffffffe0, 0x0c);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xffffff00, 0x8c);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, 0xffffff00, 0x77);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xfffffffe, 0x01);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xffffffe0, 0x0e);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xffffff00, 0x2e);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, 0xffffff00, 0x0d);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xfffffffe, 0x01);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xffffffe0, 0x10);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xffffff00, 0x30);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, 0xffffff00, 0x2f);
	rtd_maskl(DVBC_DDC_WR_STB_reg, 0xfffffffe, 0x01);
	//-------------[End] Set ddc_hd(DDC LPF shape)--------------------
#endif
	//rtd_part_outl(DVBC_FS_ACQ_MODE_reg, 7, 0, 0x52);	//0xe30a[7:0] = 0x52
	rtd_maskl(DVBC_FS_ACQ_MODE_reg, 0xffffff00, 0x52);
	//rtd_part_outl(DVBC_OC_BER_EN_reg, 7, 0, 0x28);	//0xe313[7:0] = 0x28
	rtd_maskl(DVBC_OC_BER_EN_reg, 0xffffff00, 0x28);
	//rtd_part_outl(DVBC_OC_CDIV_PH0_reg, 7, 0, 0x99);	//0xe31d[7:0] = 0x99
	rtd_maskl(DVBC_OC_CDIV_PH0_reg, 0xffffff00, 0x99);
	//rtd_part_outl(DVBC_OC_MPEG_SYNC_MODE_reg, 7, 0, 0xa6);	//0xe31f[7:0] = 0xa6
	rtd_maskl(DVBC_OC_MPEG_SYNC_MODE_reg, 0xffffff00, 0xa6);
	////rtd_part_outl(DVBC_OC_MPEG_MISMATCH_reg, 7, 0, 0xdf);	//0xe320[7:0] = 0xdf
	//rtd_maskl(DVBC_OC_MPEG_MISMATCH_reg, 0xffffff00, 0xdf);
	//rtd_part_outl(DVBC_OC_MPEG_MISMATCH_reg, 7, 0, 0xd1);	//0xe320[7:0] = 0xd1
	rtd_maskl(DVBC_OC_MPEG_MISMATCH_reg, 0xffffff00, 0xd1);
	//rtd_part_outl(DVBC_OC_MPEG_ACQ2_K_MODE_reg, 7, 0, 0xf3);	//0xe327[7:0] = 0xf3
	rtd_maskl(DVBC_OC_MPEG_ACQ2_K_MODE_reg, 0xffffff00, 0xf3);

	//-------------[Start] Set FSM ----------------------------------
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x00);   //FSM state 0
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0xd0);
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0x49);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x8c);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0xf2);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0x01);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa0);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0x98);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x18);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x01);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x02);   //FSM state 1
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0x21);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0x22);
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0x89);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x8c);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0xf2);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0x01);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0x80);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x4b);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa4);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0x98);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x38);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x03);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x04);  //FSM state 2
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0x22);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0x32);
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0x89);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x8c);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0x72);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0xc0);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x46);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa4);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0xf8);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x38);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x05);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x06);  //FSM state 3
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0x43);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0x44);
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0x8b);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x0c);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0xf2);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0x5f);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0xb6);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x44);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa4);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0xf8);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x32);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x07);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x08);  //FSM state 4
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0x54);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0x55);
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0xcb);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x1c);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0xf3);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0x4f);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0xb6);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x44);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa4);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0xf8);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x32);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x09);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x0a);  //FSM state 5
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0x65);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0x66);
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0xcb);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x1c);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0xf5);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0xcd);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0xb5);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x44);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa4);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0xf8);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x32);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x0b);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x0c);  //FSM state 6
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0x76);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0x77);
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0xcb);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x9c);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0xf7);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0x4b);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0x75);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x40);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa4);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0xf8);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x32);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x0d);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x0e);  //FSM state 7
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0x87);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0x88);
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0xcb);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x2c);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0x48);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0xc3);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0x73);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x40);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa4);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0xf8);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x32);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x0f);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x10);  //FSM state 8
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0x98);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0x99);
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0xcc);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x3c);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0x48);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0xa3);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0x73);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x40);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa4);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0xfa);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x32);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x11);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x12);  //FSM state 9
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0x99);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0xa9);
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0xc9);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x4e);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0x48);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0xa2);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0x73);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x40);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa4);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0xfe);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x32);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x13);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x14);  //FSM state 10
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0xaa);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0xbb);
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0xee);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x5e);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0x48);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0xa2);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0x73);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x60);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa6);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0xfe);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x32);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x15);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x16);  //FSM state 11
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0xbb);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0xc8);
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0xcb);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x6e);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0x24);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0x9a);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0x73);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x60);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa6);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0xfe);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x73);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x17);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x1a);  //FSM state 13
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0x1d);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0x1d);
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0x4f);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x8c);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0xf2);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0x01);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa0);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0x98);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x18);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x1b);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x1c);  //FSM state 14
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0x1e);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0x1f);
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0x4f);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x8c);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0xf2);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0x01);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa0);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0x98);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x18);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x1d);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x1e);  //FSM state 15
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0x1f);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0x11);
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0x4f);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x8c);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0xf2);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0x01);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa0);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0x98);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x18);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x1f);
	//-------------[End] Set FSM ----------------------------------

	//rtd_part_outl(DVBC_RXMF_WR_STB_reg, 7, 0, 0x00);	//0xe157[7:0] = 0x00
	rtd_maskl(DVBC_RXMF_WR_STB_reg, 0xffffff00, 0x00);
	//rtd_part_outl(DVBC_RXMF_WR_STB_1_reg, 6, 0, 0x00);	//0xe158[7:0] = 0x00
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, 0xffffff00, 0x00);
	//rtd_part_outl(DVBC_RXMF_WR_STB_reg, 7, 0, 0x01);	//0xe157[7:0] = 0x01
	rtd_maskl(DVBC_RXMF_WR_STB_reg, 0xffffff00, 0x01);

	//rtd_part_outl(DVBC_RXMF_WR_STB_reg, 7, 0, 0xc4);	//0xe157[7:0] = 0xc4
	rtd_maskl(DVBC_RXMF_WR_STB_reg, 0xffffff00, 0xc4);
	//rtd_part_outl(DVBC_RXMF_WR_STB_1_reg, 6, 0, 0x03);	//0xe158[7:0] = 0x03
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, 0xffffff00, 0x03);
	//rtd_part_outl(DVBC_RXMF_WR_STB_reg, 7, 0, 0xc5);	//0xe157[7:0] = 0xc5
	rtd_maskl(DVBC_RXMF_WR_STB_reg, 0xffffff00, 0xc5);

	//rtd_part_outl(DVBC_RXMF_WR_STB_reg, 7, 0, 0xcc);	//0xe157[7:0] = 0xcc
	rtd_maskl(DVBC_RXMF_WR_STB_reg, 0xffffff00, 0xcc);
	//rtd_part_outl(DVBC_RXMF_WR_STB_1_reg, 6, 0, 0x7e);	//0xe158[7:0] = 0x7e
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, 0xffffff00, 0x7e);
	//rtd_part_outl(DVBC_RXMF_WR_STB_reg, 7, 0, 0xcd);	//0xe157[7:0] = 0xcd
	rtd_maskl(DVBC_RXMF_WR_STB_reg, 0xffffff00, 0xcd);

	//rtd_part_outl(DVBC_RXMF_WR_STB_reg, 7, 0, 0xd0);	//0xe157[7:0] = 0xd0
	rtd_maskl(DVBC_RXMF_WR_STB_reg, 0xffffff00, 0xd0);
	//rtd_part_outl(DVBC_RXMF_WR_STB_1_reg, 6, 0, 0x01);	//0xe158[7:0] = 0x01
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, 0xffffff00, 0x01);
	//rtd_part_outl(DVBC_RXMF_WR_STB_reg, 7, 0, 0xd1);	//0xe157[7:0] = 0xd1
	rtd_maskl(DVBC_RXMF_WR_STB_reg, 0xffffff00, 0xd1);

	//rtd_part_outl(DVBC_OPT_QAM_MODE_reg, 5, 3, 0x01);	//0xe102[5:3] = 0x01
	rtd_maskl(DVBC_OPT_QAM_MODE_reg, 0xffffffc7, 0x08);
	//rtd_part_outl(DVBC_OPT_QAM_ON_reg, 0, 0, 0x01);	//0xe004[0:0] = 0x01
	rtd_maskl(DVBC_OPT_QAM_ON_reg, 0xfffffffe, 0x01);
	//rtd_part_outl(DVBC_OPT_DSP_FIRST_OUT_IDX_reg, 7, 0, 0x00);	//0xe22e[7:0] = 0x00
	rtd_maskl(DVBC_OPT_DSP_FIRST_OUT_IDX_reg, 0xffffff00, 0x00);
	//rtd_part_outl(DVBC_OPT_DSP_FIRST_OUT_IDX_1_reg, 4, 0, 0x10);	//0xe22f[4:0] = 0x10
	rtd_maskl(DVBC_OPT_DSP_FIRST_OUT_IDX_1_reg, 0xffffffe0, 0x10);
	//rtd_part_outl(DVBC_OPT_FFT_ABS_OUT_reg, 0, 0, 0x01);	//0xe230[0]   = 0x01
	rtd_maskl(DVBC_OPT_FFT_ABS_OUT_reg, 0xfffffffe, 0x01);
	//rtd_part_outl(DVBC_OPT_FFT_DSP_READ_reg, 0, 0, 0x01);	//0xe231[0]   = 0x00
	rtd_maskl(DVBC_OPT_FFT_DSP_READ_reg, 0xfffffffe, 0x00);

	//rtd_part_outl(DVBC_OPT_POS_FRQ_SLOT_SR_EN_reg, 7, 0, 0x01);	//0xe232[1:0] = 0x01
	//rtd_maskl(DVBC_OPT_POS_FRQ_SLOT_SR_EN_reg, 0xffffff00, 0x01);
	//rtd_part_outl(DVBC_OPT_POS_FRQ_SLOT_SR_EN_1_reg, 7, 0, 0x00);	//0xe233[1:0] = 0x00
	//rtd_maskl(DVBC_OPT_POS_FRQ_SLOT_SR_EN_1_reg, 0xffffff00, 0x00);

	//rtd_part_outl(DVBC_OPT_FFT_OUT_MODE_reg, 0, 0, 0x00);	//0xe234[0]   = 0x00
	rtd_maskl(DVBC_OPT_FFT_OUT_MODE_reg, 0xfffffffe, 0x00);

	//rtd_part_outl(DVBC_OPT_PAPD_PAPR_SEL_reg, 2, 0, 0x02);	//0xe235[2:0] = 0x02
	//rtd_maskl(DVBC_OPT_PAPD_PAPR_SEL_reg, 0xfffffff8, 0x02);

	//rtd_part_outl(DVBC_OPT_FFT_OUTPUT_ACC_NUM_reg, 3, 0, 0x02);	//0xe236[5:0] = 0x00
	rtd_maskl(DVBC_OPT_FFT_OUTPUT_ACC_NUM_reg, 0xffffffc0, 0x00);
	//rtd_part_outl(DVBC_OPT_NBD_TDFT_IDX_reg, 3, 0, 0x00);	//0xe237[3:0] = 0x00
	rtd_maskl(DVBC_OPT_NBD_TDFT_IDX_reg, 0xfffffff0, 0x00);

	//issue
	//rtd_part_outl(DVBC_OPT_SLOT_AVG_RATIO_SEL_reg, 3, 0, 0x03);	//0xe238[2:0] = 0x03
	//rtd_maskl(DVBC_OPT_SLOT_AVG_RATIO_SEL_reg, 0xfffffff8, 0x03);
	//rtd_part_outl(DVBC_OPT_COMP_SLOT_AVG_THD_reg, 7, 0, 0x14);	//0xe239[7:0] = 0x14
	//rtd_maskl(DVBC_OPT_COMP_SLOT_AVG_THD_reg, 0xffffff00, 0x14);
	//rtd_part_outl(DVBC_OPT_COMP_SLOT_AVG_THD_1_reg, 7, 0, 0x00);	//0xe23a[4:0] = 0x00
	//rtd_maskl(DVBC_OPT_COMP_SLOT_AVG_THD_1_reg, 0xffffff00, 0x00);

	//rtd_part_outl(OPT_FIRST_TONE_CRIT_reg, 0, 0, 0x00);	//0xe23b[0]   = 0x00
	//rtd_maskl(OPT_FIRST_TONE_CRIT_reg, 0xfffffffe, 0x00);
	//rtd_part_outl(OPT_NEG_FRQ_COMP_CRIT_SEL_reg, 2, 0, 0x00);	//0xe23c[2:0] = 0x00
	//rtd_maskl(OPT_NEG_FRQ_COMP_CRIT_SEL_reg, 0xfffffff8, 0x00);
	//rtd_part_outl(OPT_POS_FRQ_COMP_CRIT_SEL_reg, 1, 0, 0x00);	//0xe23d[1:0] = 0x00
	//rtd_maskl(OPT_POS_FRQ_COMP_CRIT_SEL_reg, 0xfffffffc, 0x00);
#ifndef OC_AutoDetectMode
	//rtd_part_outl(DVBC_OPT_POS_FRQ_MAX_SR_reg, 7, 0, 0x73);	//0xe23e[7:0] = 0x73
	rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_reg, 0xffffff00, 0x73);
	//rtd_part_outl(DVBC_OPT_POS_FRQ_MAX_SR_1_reg, 3, 0, 0x0f);	//0xe23f[4:0] = 0x0f
	rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_1_reg, 0xffffffe0, 0x0f);
	//rtd_part_outl(DVBC_OPT_POS_FRQ_MIN_SR_reg, 7, 0, 0x00);		//0xe240[7:0] = 0x00
	rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_reg, 0xffffff00, 0x00);
	//rtd_part_outl(DVBC_OPT_POS_FRQ_MIN_SR_1_reg, 3, 0, 0x08);	//0xe241[4:0] = 0x08
	rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_1_reg, 0xffffffe0, 0x08);
#endif
	//rtd_part_outl(DVBC_OPT_START_SRS_NBD_reg, 0, 0, 0x00);	//0xe268[0]   = 0x00
	rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, 0xfffffffe, 0x00);
	//rtd_part_outl(DVBC_OPT_QAMD_CNT_STOP_reg, 7, 0, 0xff);	//0xe242[7:0] = 0xff
	rtd_maskl(DVBC_OPT_QAMD_CNT_STOP_reg, 0xffffff00, 0xff);
	//rtd_part_outl(DVBC_OPT_QAMD_CNT_STOP_1_reg, 3, 0, 0x1f);	//0xe243[4:0] = 0x1f
	rtd_maskl(DVBC_OPT_QAMD_CNT_STOP_1_reg, 0xffffffe0, 0x1f);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN0_MAX_reg, 7, 0, 0x30);	//0xe244[7:0] = 0x30
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN0_MAX_reg, 0xffffff00, 0x30);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN0_MAX_1_reg, 1, 0, 0x00);	//0xe245[1:0] = 0x00
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN0_MAX_1_reg, 0xfffffffc, 0x00);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN0_MIN_reg, 7, 0, 0x20);	//0xe246[7:0] = 0x20
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN0_MIN_reg, 0xffffff00, 0x20);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN0_MIN_1_reg, 1, 0, 0x00);	//0xe247[1:0] = 0x00
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN0_MIN_1_reg, 0xfffffffc, 0x00);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN1_MAX_reg, 7, 0, 0x58);	//0xe248[7:0] = 0x58
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN1_MAX_reg, 0xffffff00, 0x58);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN1_MAX_1_reg, 1, 0, 0x00);	//0xe249[1:0] = 0x00
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN1_MAX_1_reg, 0xfffffffc, 0x00);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN1_MIN_reg, 7, 0, 0x48);	//0xe24a[7:0] = 0x48
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN1_MIN_reg, 0xffffff00, 0x48);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN1_MIN_1_reg, 1, 0, 0x00);	//0xe24b[1:0] = 0x00
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN1_MIN_1_reg, 0xfffffffc, 0x00);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN2_MAX_reg, 7, 0, 0xa8);	//0xe24c[7:0] = 0xa8
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN2_MAX_reg, 0xffffff00, 0xa8);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN2_MAX_1_reg, 1, 0, 0x00);	//0xe24d[1:0] = 0x00
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN2_MAX_1_reg, 0xfffffffc, 0x00);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN2_MIN_reg, 7, 0, 0x98);	//0xe24e[7:0] = 0x98
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN2_MIN_reg, 0xffffff00, 0x98);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN2_MIN_1_reg, 1, 0, 0x00);	//0xe24f[1:0] = 0x00
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN2_MIN_1_reg, 0xfffffffc, 0x00);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN3_MAX_reg, 7, 0, 0xe8);	//0xe250[7:0] = 0xe8
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN3_MAX_reg, 0xffffff00, 0xe8);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN3_MAX_1_reg, 1, 0, 0x00);	//0xe251[1:0] = 0x00
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN3_MAX_1_reg, 0xfffffffc, 0x00);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN3_MIN_reg, 7, 0, 0xd8);	//0xe252[7:0] = 0xd8
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN3_MIN_reg, 0xffffff00, 0xd8);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN3_MIN_1_reg, 1, 0, 0x00);	//0xe253[1:0] = 0x00
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN3_MIN_1_reg, 0xfffffffc, 0x00);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN4_MAX_reg, 7, 0, 0x10);	//0xe254[7:0] = 0x10
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN4_MAX_reg, 0xffffff00, 0x10);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN4_MAX_1_reg, 1, 0, 0x01);	//0xe255[1:0] = 0x01
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN4_MAX_1_reg, 0xfffffffc, 0x01);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN4_MIN_reg, 7, 0, 0x00);	//0xe256[7:0] = 0x00
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN4_MIN_reg, 0xffffff00, 0x00);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN4_MIN_1_reg, 1, 0, 0x01);	//0xe257[1:0] = 0x01
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN4_MIN_1_reg, 0xfffffffc, 0x01);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN5_MAX_reg, 7, 0, 0x08);	//0xe258[7:0] = 0x08
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN5_MAX_reg, 0xffffff00, 0x08);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN5_MAX_1_reg, 1, 0, 0x02);	//0xe259[1:0] = 0x02
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN5_MAX_1_reg, 0xfffffffc, 0x02);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN5_MIN_reg, 7, 0, 0xa4);	//0xe25a[7:0] = 0xa4
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN5_MIN_reg, 0xffffff00, 0xa4);
	//rtd_part_outl(DVBC_OPT_QAMD_EQP_ZN5_MIN_1_reg, 1, 0, 0x01);	//0xe25b[1:0] = 0x01
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN5_MIN_1_reg, 0xfffffffc, 0x01);
	//rtd_part_outl(DVBC_OPT_QAMD_SELF_RESET_reg, 0, 0, 0x01);	//0xe25c[0]   = 0x01
	rtd_maskl(DVBC_OPT_QAMD_SELF_RESET_reg, 0xfffffffe, 0x01);
	//rtd_part_outl(DVBC_OPT_QAMD_START_STATE_reg, 3, 0, 0x05);	//0xe25d[3:0] = 0x05
	rtd_maskl(DVBC_OPT_QAMD_START_STATE_reg, 0xfffffff0, 0x05);
	//rtd_part_outl(DVBC_OPT_QAMD_THD_128QAM_reg, 7, 0, 0x28);	//0xe25e[7:0] = 0x28
	rtd_maskl(DVBC_OPT_QAMD_THD_128QAM_reg, 0xffffff00, 0x28);
	//rtd_part_outl(DVBC_OPT_QAMD_THD_128QAM_1_reg, 1, 0, 0x00);	//0xe25f[1:0] = 0x00
	rtd_maskl(DVBC_OPT_QAMD_THD_128QAM_1_reg, 0xfffffffc, 0x00);
	//rtd_part_outl(DVBC_OPT_QAMD_THD_16QAM_reg, 7, 0, 0x2c);	//0xe260[7:0] = 0x2c
	rtd_maskl(DVBC_OPT_QAMD_THD_16QAM_reg, 0xffffff00, 0x2c);
	//rtd_part_outl(DVBC_OPT_QAMD_THD_16QAM_1_reg, 1, 0, 0x01);	//0xe261[1:0] = 0x01
	rtd_maskl(DVBC_OPT_QAMD_THD_16QAM_1_reg, 0xfffffffc, 0x01);
	//rtd_part_outl(DVBC_OPT_QAMD_THD_256QAM_reg, 7, 0, 0x28);	//0xe262[7:0] = 0x28
	rtd_maskl(DVBC_OPT_QAMD_THD_256QAM_reg, 0xffffff00, 0x28);
	//rtd_part_outl(DVBC_OPT_QAMD_THD_256QAM_1_reg, 1, 0, 0x00);	//0xe263[1:0] = 0x00
	rtd_maskl(DVBC_OPT_QAMD_THD_256QAM_1_reg, 0xfffffffc, 0x00);
	//rtd_part_outl(DVBC_OPT_QAMD_THD_32QAM_reg, 7, 0, 0x96);	//0xe264[7:0] = 0x96
	rtd_maskl(DVBC_OPT_QAMD_THD_32QAM_reg, 0xffffff00, 0x96);
	//rtd_part_outl(DVBC_OPT_QAMD_THD_32QAM_1_reg, 1, 0, 0x00);	//0xe265[1:0] = 0x00
	rtd_maskl(DVBC_OPT_QAMD_THD_32QAM_1_reg, 0xfffffffc, 0x00);
	//rtd_part_outl(DVBC_OPT_QAMD_THD_64QAM_reg, 7, 0, 0x22);	//0xe266[7:0] = 0x22
	rtd_maskl(DVBC_OPT_QAMD_THD_64QAM_reg, 0xffffff00, 0x22);
	//rtd_part_outl(DVBC_OPT_QAMD_THD_64QAM_1_reg, 1, 0, 0x01);	//0xe267[1:0] = 0x01
	rtd_maskl(DVBC_OPT_QAMD_THD_64QAM_1_reg, 0xfffffffc, 0x01);

#ifdef OC_AutoDetectMode
	//------------------------For Auto mode CSR setting------------------------//
	rtd_maskl(DVBC_CSRS_CSR_THD_WEIGHT_reg, ~0x0f, 0x05);//4 to 5
	//CSR min bin to 5.056941 - 0.5 = 4.556941M ->  4.556941*8192/CrystalFreqHz = 1481 =0x5c9   //0x1ff -->3.1469M
	rtd_maskl(DVBC_CSRS_MIN_SPACE_reg, ~(DVBC_CSRS_MIN_SPACE_reg_csrs_min_space_7_0_mask), DVBC_CSRS_MIN_SPACE_reg_csrs_min_space_7_0(0xff));//CSR min bin to 3.146M
	rtd_maskl(DVBC_CSRS_CSR_THD_WEIGHT_reg, ~(DVBC_CSRS_CSR_THD_WEIGHT_reg_csrs_min_space_9_8_mask), DVBC_CSRS_CSR_THD_WEIGHT_reg_csrs_min_space_9_8(0x3));
	//CSR max bin to 5.360537 + 0.5 =5.860537M ->  5.860537*8192/CrystalFreqHz = 1906 = 0x772
	rtd_maskl(DVBC_CSR_NO_SIGNAL_reg, ~(DVBC_CSR_NO_SIGNAL_reg_csr_fft_bin_max_7_0_mask), DVBC_CSR_NO_SIGNAL_reg_csr_fft_bin_max_7_0(0x72));//CSR max bin to 5.86M
	rtd_maskl(DVBC_CSR_NO_SIGNAL2_reg, ~(DVBC_CSR_NO_SIGNAL2_reg_csr_fft_bin_max_11_8_mask), DVBC_CSR_NO_SIGNAL2_reg_csr_fft_bin_max_11_8(0x7));
	//------------------------For Auto mode FSR setting------------------------//

	//search range 4.978M ~ 5.367M (Merlin), SymRate (OC) = srs_fft_idx*(ADC_freq/2/fft_size)
	pDemod->GetCrystalFreqHz(pDemod, &CrystalFreqHz);

	sr_idx_max = (((5367) << 14) / (CrystalFreqHz / 1000)); //Mike need modify
	sr_idx_min = (((4978) << 14) / (CrystalFreqHz / 1000)); //Mike need modify
	oc_sym_rate1 =  0xCD8;    //(5057 *8192 *2) / (CrystalFreqHz / 1000);   //5.056941 MHz
	oc_sym_rate2 =  0xD9D;    //(5361 *8192 *2) / (CrystalFreqHz / 1000);   //5.360537 MHz


	rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_reg, ~DVBC_OPT_POS_FRQ_MAX_SR_opt_pos_frq_max_sr_7_0_mask, DVBC_OPT_POS_FRQ_MAX_SR_opt_pos_frq_max_sr_7_0(sr_idx_max & 0xff));
	rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_1_reg, ~DVBC_OPT_POS_FRQ_MAX_SR_1_opt_pos_frq_max_sr_11_8_mask, DVBC_OPT_POS_FRQ_MAX_SR_1_opt_pos_frq_max_sr_11_8((sr_idx_max >> 8) & 0x0f));
	rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_reg, ~DVBC_OPT_POS_FRQ_MIN_SR_opt_pos_frq_min_sr_7_0_mask, DVBC_OPT_POS_FRQ_MIN_SR_opt_pos_frq_min_sr_7_0(sr_idx_min & 0xff));
	rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_1_reg, ~DVBC_OPT_POS_FRQ_MIN_SR_1_opt_pos_frq_min_sr_11_8_mask, DVBC_OPT_POS_FRQ_MIN_SR_1_opt_pos_frq_min_sr_11_8((sr_idx_min >> 8) & 0x0f));

	rtd_maskl(DVBC_OC_SYM_RATE0_reg, ~DVBC_OC_SYM_RATE0_opt_oc_sym_rate1_7_0_mask, DVBC_OC_SYM_RATE0_opt_oc_sym_rate1_7_0(oc_sym_rate1 & 0xff));
	rtd_maskl(DVBC_OC_SYM_RATE2_reg, ~DVBC_OC_SYM_RATE2_opt_oc_sym_rate1_11_8_mask, DVBC_OC_SYM_RATE2_opt_oc_sym_rate1_11_8((oc_sym_rate1 >> 8) & 0x0f));
	rtd_maskl(DVBC_OC_SYM_RATE1_reg, ~DVBC_OC_SYM_RATE1_opt_oc_sym_rate2_7_0_mask, DVBC_OC_SYM_RATE1_opt_oc_sym_rate2_7_0(oc_sym_rate2 & 0xff));
	rtd_maskl(DVBC_OC_SYM_RATE2_reg, ~DVBC_OC_SYM_RATE2_opt_oc_sym_rate2_11_8_mask, DVBC_OC_SYM_RATE2_opt_oc_sym_rate2_11_8((oc_sym_rate2 >> 8) & 0x0f));

	//rtd_maskl(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_1_reg, ~0x0f, 0x00);	//[11:8]larger FSR search range half
	rtd_maskl(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_SUB_reg, ~0xff, 0xcd);	//[7:0] FSR search range half sub 0xA3=500K 0xC4=600K 0xe4=700K 0xcd = 630K  //reg*ADC/8192

	rtd_maskl(DVBC_OPT_POS_NEG_DIFF_1_reg, ~0xf0, 0xa0);   //modifed pos_neg_diff_idx = 0x2
	//rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_reg, ~0xff, 0xd0);	//0x23e=0x73, [7:0]opt_pos_frq_max_sr[7:0]=0x73
	//rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_1_reg, ~0x0f, 0x0c);
	//rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_reg, ~0xff, 0x63);
	//rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_1_reg, ~0x0f, 0x06);

	//rtd_maskl(DVBC_FFT_INPUT_BITWISE_AUTO_reg, ~0xf0, 0x00); //[7:4]FSR input shift ori=0x1
	//------------------------For Auto mode CCFO setting-----------------------//

	rtd_maskl(DVBC_FFT_INPUT_BITWISE_AUTO_reg, ~0x0f, 0x02); //match C platform
	rtd_maskl(DVBC_OPT_POS_FRQ_SR_CCFO_reg, ~0xff, 0xcb);
	rtd_maskl(DVBC_OPT_POS_FRQ_SR_CCFO_2_reg, ~0x0f, 0x02);
	rtd_maskl(DVBC_FFT_ACC_NUM_AUTO_1_reg, ~0x1f, 0x0f); //increase CCFO acc num to 15
	rtd_outl(DVBC_OPT_KI_CR_OUT_MAX_reg, 0xa4);//for CR range
	rtd_maskl(DVBC_CCFO_PAPR_reg, ~0x01, 0x0); //enable papr ccfo

	//------------------------For Auto mode Qamd  setting-----------------------//
	rtd_maskl(DVBC_OPT_QAMD_START_STATE_reg, ~0x0f, 0x05); //opt_qamd_start_state[3:0]
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN9_MIN_reg, ~0xff, 0x3d);//opt_qamd_eqp_zn9_min[7:0] for auto qam match C platform
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN10_MIN_reg, ~0xff, 0x98);//opt_qamd_eqp_zn10_min[7:0]

	//--------------------------Enable erasure decode---------------------------//
	//rtd_part_outl(DVBC_DVBC_ERASURE_RS_CTRL0_reg, 0, 0,0x1);
	//rtd_part_outl(DVBC_DVBC_ERASURE_RS_CTRL1_reg, 2, 0,0x4);

	//--------------------------For SFI---------------------------//
	rtd_outl(DVBC_FFT_ACC_NUM_AUTO_reg, 0x07);//CSR acc num = def 0x7
	rtd_outl(DVBC_FFT_ACC_NUM_AUTO_2_reg, 0x1f);//FSR acc num = 31
	//rtd_outl(DVBC_FSFI_CTRL0_reg, 0x07);//fine sfi ratio = 0x7
	rtd_outl(DVBC_SFI_DET_CTRL0_reg, 0x0f);//corse sfi ratio = 0xf
	rtd_maskl(DVBC_SFI_DET_CTRL7_reg, ~0x01,0x01);//[0]opt_sfi_det enable old SFI detect
	rtd_maskl(DVBC_SFI_DET_CTRL7_reg, ~0x02,0x02);//[1]opt_sfi_trk_en=1

	//-------------Start NEW FSM with auto mode-------------------------//
        //modified FSM for auto mode (FSM 0 -> 14 (auto mode) ->1 -> ....)
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x00);   //FSM state 0
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0xe0);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0xee);
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0x4f);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x8c);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0xf2);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0x01);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa0);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0x98);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x18);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x01);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x1c);  //FSM state 14
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0x1e);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0x11);
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0x40);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x8c);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0xf2);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0x01);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa0);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0x98);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x18);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x18);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x1d);
#endif
	//[End] RTK OpenCable Init Table, Sync from Merlin K2L, 0xe22e~0xe266 sync from Mac6p DVB-C
	//-----------------------------------------------------------------------------------------//

	/*
		int i;
		unsigned short RegStartAddr;
		unsigned char Msb;
		unsigned char Lsb;
		U32BITS WritingValue;
	*/

	// Initialize demod with register initializing table.
	/*
	for (i = 0; i < sizeof(InitRegTable) / sizeof(INIT_REG_ENTRY); i++) {
			// Get all information from each register initializing entry.
			RegStartAddr = InitRegTable[i].RegStartAddr;
			Msb          = InitRegTable[i].Msb;
			Lsb          = InitRegTable[i].Lsb;
			WritingValue = InitRegTable[i].WritingValue;

			// Set register mask bits.
			if (pDemod->RegAccess.Addr16Bit.SetRegMaskBits(pDemod, RegStartAddr, Msb, Lsb, WritingValue) !=
				FUNCTION_SUCCESS)
				goto error_status_set_demod_registers;
		}
	*/

	return FUNCTION_SUCCESS;
}


/**

@see   QAM_DEMOD_FP_SET_QAM_MODE

*/
int
realtek_r_oc_SetQamMode(
	QAM_DEMOD_MODULE *pDemod,
	int QamMode
)
{
	/*
	typedef struct {
			unsigned short RegStartAddr;
			unsigned char Msb;
			unsigned char Lsb;
			U32BITS WritingValue[REALTEK_R_OC_QAM_MODE_NUM];
		}
		QAM_MODE_REG_ENTRY;
	*/

	/*
	static const QAM_MODE_REG_ENTRY QamModeRegTable[REALTEK_R_OC_QAM_MODE_REG_TABLE_LEN] = {
			// Register,                       WritingValue according to QAM mode
			// RegStartAddr,   Msb,    Lsb,    {64-QAM,       256-QAM,      64-QAM-AH,    256-QAM-AH,   }
			{0xe102,           2,      0,      {0x2,          0x4,          0x2,          0x4,          },   },
			{0xe120,           7,      0,      {0xee,         0xcd,         0xee,         0xcd,         },   },
			{0xe121,           7,      0,      {0x13,         0x12,         0x13,         0x12,         },   },
			{0xe12e,           7,      0,      {0xc8,         0xd0,         0xc8,         0xd0,         },   },
			{0xe12f,           7,      0,      {0xc0,         0x80,         0xc0,         0x80,         },   },
			{0xe130,           7,      0,      {0x21,         0x43,         0x21,         0x43,         },   },
			{0xe131,           7,      0,      {0x5,          0xa,          0x5,          0xa,          },   },
			{0xe15d,           7,      0,      {0x18,         0x18,         0x18,         0x18,         },   },
			{0xe15e,           31,     0,      {0x7f00ccc8,   0x7f00ccc8,   0x7ec0ccc8,   0x7ec0ccc8,   },   },
			{0xe162,           31,     0,      {0x205bdc28,   0x205bdc28,   0x205bdc28,   0x205bdc28,   },   },
			{0xe166,           23,     0,      {0x737ea9,     0x737ea9,     0x737ea9,     0x737ea9,     },   },
			{0xe1cc,           7,      0,      {0x0,          0x0,          0x0,          0x0,          },   },
			{0xe15d,           7,      0,      {0x19,         0x19,         0x19,         0x19,         },   },
			{0xe157,           15,     0,      {0x62,         0x362,        0x62,         0x362,        },   },
			{0xe157,           7,      0,      {0x63,         0x63,         0x63,         0x63,         },   },
			{0xe157,           15,     0,      {0x3c6,        0x1c6,        0x3c6,        0x1c6,        },   },
			{0xe157,           7,      0,      {0xc7,         0xc7,         0xc7,         0xc7,         },   },
			{0xe157,           15,     0,      {0xc8,         0x1c8,        0xc8,         0x1c8,        },   },
			{0xe157,           7,      0,      {0xc9,         0xc9,         0xc9,         0xc9,         },   },
			{0xe157,           15,     0,      {0x7fca,       0x7eca,       0x7fca,       0x7eca,       },   },
			{0xe157,           7,      0,      {0xcb,         0xcb,         0xcb,         0xcb,         },   },
			{0xe157,           15,     0,      {0x14e,        0x24e,        0x14e,        0x24e,        },   },
			{0xe157,           7,      0,      {0x4f,         0x4f,         0x4f,         0x4f,         },   },
			{0xe157,           15,     0,      {0x7d52,       0x7b52,       0x7d52,       0x7b52,       },   },
			{0xe157,           7,      0,      {0x53,         0x53,         0x53,         0x53,         },   },
			{0xe157,           15,     0,      {0x7dd4,       0x7ed4,       0x7dd4,       0x7ed4,       },   },
			{0xe157,           7,      0,      {0xd5,         0xd5,         0xd5,         0xd5,         },   },
			{0xe157,           15,     0,      {0x5b6,        0x6b6,        0x5b6,        0x6b6,        },   },
			{0xe157,           7,      0,      {0xb7,         0xb7,         0xb7,         0xb7,         },   },
			{0xe157,           15,     0,      {0x2b8,        0x1b8,        0x2b8,        0x1b8,        },   },
			{0xe157,           7,      0,      {0xb9,         0xb9,         0xb9,         0xb9,         },   },
			{0xe157,           15,     0,      {0x747a,       0x737a,       0x747a,       0x737a,       },   },
			{0xe157,           7,      0,      {0x7b,         0x7b,         0x7b,         0x7b,         },   },
			{0xe157,           15,     0,      {0x7d3c,       0x7e3c,       0x7d3c,       0x7e3c,       },   },
			{0xe157,           7,      0,      {0x3d,         0x3d,         0x3d,         0x3d,         },   },
			{0xe157,           15,     0,      {0x267e,       0x277e,       0x267e,       0x277e,       },   },
			{0xe157,           7,      0,      {0x7f,         0x7f,         0x7f,         0x7f,         },   },
		};
	*/

	/*
	int i;
	unsigned short RegStartAddr;
	unsigned char Msb;
	unsigned char Lsb;
	U32BITS WritingValue;
	*/

	int SelectionIndex;

	rtd_maskl(DVBC_AUTO_SRS_reg, ~(DVBC_AUTO_SRS_opt_csr_en_mask | DVBC_AUTO_SRS_opt_auto_srs_qam_start_mask), (DVBC_AUTO_SRS_opt_csr_en(1) | DVBC_AUTO_SRS_opt_auto_srs_qam_start(1)));//[0]Auto srs start [4]Auto CSR enable
	rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~(DVBC_OPT_START_SRS_NBD_opt_ccfo_auto_mask), DVBC_OPT_START_SRS_NBD_opt_ccfo_auto(1));//enable ccfo update
	rtd_maskl(DVBC_FFT_ACC_NUM_AUTO_2_reg, ~(DVBC_FFT_ACC_NUM_AUTO_2_opt_fft_output_acc_num_fsrs_mask), 0x1);//FSR acc num = 1
	if(gCorse_sfi_found == 1){
		rtd_maskl(DVBC_FSM_FIX_reg, ~(DVBC_FSM_FIX_opt_fsm_fix_ste_en_mask | DVBC_FSM_FIX_opt_fsm_fix_ste_end_mask)  , (DVBC_FSM_FIX_opt_fsm_fix_ste_en(1) | DVBC_FSM_FIX_opt_fsm_fix_ste_end(0)));//fix fsm at st0
		rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask, DVBC_OPT_RST_STATE0_ON_opt_retry_on(0));//retry off
	} else {
		rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask, DVBC_OPT_RST_STATE0_ON_opt_retry_on(1));//retry on
	}
	// Determine selection index according to QAM mode and AM-hum mode.
	switch (pDemod->Extra.RealtekRQam.ConfigMode) {

	case REALTEK_R_QAM_CONFIG_OC:
		switch (QamMode) {
		case QAM_QAM_64:
			REALTEK_R_QAM_BASE_INFO("========[OC Set to 64QAM]========\n");
			SelectionIndex = 0;
			rtd_maskl(DVBC_OPT_QAM_MODE_reg, ~DVBC_OPT_QAM_MODE_opt_qam_mode_mask, DVBC_OPT_QAM_MODE_opt_qam_mode(0x02));	//0xe102[2:0] = 0x02
			rtd_outl(DVBC_OPT_TR_DECI_RATIO_1_reg, 	0xee);  	//0xe120[7:0]  = 0xee
			rtd_outl(DVBC_OPT_TR_DECI_RATIO_2_reg, 	0x13);		//0xe121[7:0]  = 0x13
			rtd_outl(DVBC_OPT_DD_EQ_BOUND_reg,		0xc8);		//0xe12e[7:0]  = 0xc8
			rtd_outl(DVBC_OPT_SIGN_DATA_reg,		0xc0);		//0xe12f[7:0]  = 0xc0
			rtd_outl(DVBC_OPT_SIGN_DATA_1_reg, 		0x21);    	//0xe130[7:0]  = 0x21
			rtd_outl(DVBC_OPT_CIRCLE_reg, 			0x05);  	//0xe131[7:0]  = 0x5
			rtd_outl(DVBC_FSM_WR_STB_reg, 			0x18); 		//0xe15d[7:0]  = 0x18
			rtd_outl(DVBC_FSM_WORD_reg,				0xc8); 	  	//0xe15e[31:0] = 0x7f00ccc8
			rtd_outl(DVBC_FSM_WORD_1_reg,			0xcc);   	//0xe15e[31:0] = 0x7f00ccc8
			rtd_outl(DVBC_FSM_WORD_2_reg, 			0x00); 		//0xe15e[31:0] = 0x7f00ccc8
			rtd_outl(DVBC_FSM_WORD_3_reg,			0x7f); 		//0xe15e[31:0] = 0x7f00ccc8
			rtd_outl(DVBC_FSM_WORD_4_reg,			0x28); 		//0xe162[31:0] = 0x205bdc28
			rtd_outl(DVBC_FSM_WORD_5_reg, 			0xdc); 		//0xe162[31:0] = 0x205bdc28
			rtd_outl(DVBC_FSM_WORD_6_reg, 			0x5b); 		//0xe162[31:0] = 0x205bdc28
			rtd_outl(DVBC_FSM_WORD_7_reg,			0x20); 		//0xe162[31:0] = 0x205bdc28
			rtd_outl(DVBC_FSM_WORD_8_reg, 			0xa9); 		//0xe166[23:0] = 0x737ea9
			rtd_outl(DVBC_FSM_WORD_9_reg, 			0x7e); 		//0xe166[23:0] = 0x737ea9
			rtd_outl(DVBC_FSM_WORD_10_reg, 			0x73); 		//0xe166[23:0] = 0x737ea9
			rtd_outl(DVBC_FSM_WORD_11_reg, 			0x00); 		//0xe1cc[7:0]  = 0x0
			rtd_outl(DVBC_FSM_WR_STB_reg, 			0x19); 		//0xe15d[7:0]  = 0x19

			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x62); 		//0xe157[15:0] = 0x62
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x00); 		//0xe157[15:0] = 0x62
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x63); 		//0xe157[7:0]  = 0x63
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xc6); 		//0xe157[15:0] = 0x3c6
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x03); 		//0xe157[15:0] = 0x3c6
			rtd_outl(DVBC_RXMF_WR_STB_reg,			0xc7); 		//0xe157[7:0]  = 0xc7
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xc8); 		//0xe157[15:0] = 0xc8
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x00); 		//0xe157[15:0] = 0xc8
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xc9); 		//0xe157[7:0]  = 0xc9
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xca); 		//0xe157[15:0] = 0x7fca
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7f); 		//0xe157[15:0] = 0x7fca
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xcb); 		//0xe157[7:0]  = 0xcb
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x4e); 		//0xe157[15:0] = 0x14e
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x01); 		//0xe157[15:0] = 0x14e
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x4f); 		//0xe157[7:0]  = 0x4f
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x52); 		//0xe157[15:0] = 0x7d52
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7d); 		//0xe157[15:0] = 0x7d52
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x53); 		//0xe157[7:0]  = 0x53
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xd4); 		//0xe157[15:0] = 0x7dd4
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7d); 		//0xe157[15:0] = 0x7dd4
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xd5); 		//0xe157[7:0]  = 0xd5
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb6); 		//0xe157[15:0] = 0x5b6
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x05); 		//0xe157[15:0] = 0x5b6
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb7); 		//0xe157[7:0]  = 0xb7
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb8); 		//0xe157[15:0] = 0x2b8
			rtd_outl(DVBC_RXMF_WR_STB_1_reg,		0x02); 		//0xe157[15:0] = 0x2b8
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb9); 		//0xe157[7:0]  = 0xb9
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x7a); 		//0xe157[15:0] = 0x747a
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x74); 		//0xe157[15:0] = 0x747a
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x7b); 		//0xe157[7:0]  = 0x7b
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x3c); 		//0xe157[15:0] = 0x7d3c
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7d); 		//0xe157[15:0] = 0x7d3c
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x3d); 		//0xe157[7:0]  = 0x3d
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x7e); 		//0xe157[15:0] = 0x267e
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x26); 		//0xe157[15:0] = 0x267e
			rtd_outl(DVBC_RXMF_WR_STB_reg,			0x7f); 		//0xe157[7:0]  = 0x7f
			break;

		case QAM_QAM_256:
			REALTEK_R_QAM_BASE_INFO("========[OC Set to 256QAM]========\n");
			SelectionIndex = 1;
			rtd_maskl(DVBC_OPT_QAM_MODE_reg, ~DVBC_OPT_QAM_MODE_opt_qam_mode_mask, DVBC_OPT_QAM_MODE_opt_qam_mode(0x04));	//0xe102[2:0] = 0x04
			rtd_outl(DVBC_OPT_TR_DECI_RATIO_1_reg,  0xcd);  	//0xe120[7:0]  = 0xcd
			rtd_outl(DVBC_OPT_TR_DECI_RATIO_2_reg, 	0x12);		//0xe121[7:0]  = 0x12
			rtd_outl(DVBC_OPT_DD_EQ_BOUND_reg,		0xd0);		//0xe12e[7:0]  = 0xd0
			rtd_outl(DVBC_OPT_SIGN_DATA_reg,		0x80);		//0xe12f[7:0]  = 0x80
			rtd_outl(DVBC_OPT_SIGN_DATA_1_reg, 		0x43); 		//0xe130[7:0]  = 0x43
			rtd_outl(DVBC_OPT_CIRCLE_reg, 			0x0a);  	//0xe131[7:0]  = 0xa
			rtd_outl(DVBC_FSM_WR_STB_reg, 			0x18); 		//0xe15d[7:0]  = 0x18
			rtd_outl(DVBC_FSM_WORD_reg,				0xc8); 		//0xe15e[31:0] = 0x7f00ccc8
			rtd_outl(DVBC_FSM_WORD_1_reg,			0xcc); 		//0xe15e[31:0] = 0x7f00ccc8
			rtd_outl(DVBC_FSM_WORD_2_reg, 			0x00); 		//0xe15e[31:0] = 0x7f00ccc8
			rtd_outl(DVBC_FSM_WORD_3_reg,			0x7f); 		//0xe15e[31:0] = 0x7f00ccc8
			rtd_outl(DVBC_FSM_WORD_4_reg,			0x28); 		//0xe162[31:0] = 0x205bdc28
			rtd_outl(DVBC_FSM_WORD_5_reg, 			0xdc); 		//0xe162[31:0] = 0x205bdc28
			rtd_outl(DVBC_FSM_WORD_6_reg, 			0x5b); 		//0xe162[31:0] = 0x205bdc28
			rtd_outl(DVBC_FSM_WORD_7_reg,			0x20); 		//0xe162[31:0] = 0x205bdc28
			rtd_outl(DVBC_FSM_WORD_8_reg, 			0xa9); 		//0xe166[23:0] = 0x737ea9
			rtd_outl(DVBC_FSM_WORD_9_reg, 			0x7e); 		//0xe166[23:0] = 0x737ea9
			rtd_outl(DVBC_FSM_WORD_10_reg, 			0x73); 		//0xe166[23:0] = 0x737ea9
			rtd_outl(DVBC_FSM_WORD_11_reg, 			0x00); 		//0xe1cc[7:0]  = 0x0
			rtd_outl(DVBC_FSM_WR_STB_reg, 			0x19); 		//0xe15d[7:0]  = 0x19

			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x62); 		//0xe157[15:0] = 0x362
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x03); 		//0xe157[15:0] = 0x362
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x63); 		//0xe157[7:0]  = 0x63
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xc6); 		//0xe157[15:0] = 0x1c6
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x01); 		//0xe157[15:0] = 0x1c6
			rtd_outl(DVBC_RXMF_WR_STB_reg,			0xc7); 		//0xe157[7:0]  = 0xc7
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xc8); 		//0xe157[15:0] = 0x1c8
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x01); 		//0xe157[15:0] = 0x1c8
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xc9); 		//0xe157[7:0]  = 0xc9
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xca); 		//0xe157[15:0] = 0x7eca
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7e); 		//0xe157[15:0] = 0x7eca
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xcb); 		//0xe157[7:0]  = 0xcb
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x4e); 		//0xe157[15:0] = 0x24e
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x02); 		//0xe157[15:0] = 0x24e
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x4f); 		//0xe157[7:0]  = 0x4f
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x52); 		//0xe157[15:0] = 0x7b52
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7b); 		//0xe157[15:0] = 0x7b52
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x53); 		//0xe157[7:0]  = 0x53
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xd4); 		//0xe157[15:0] = 0x7ed4
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7e); 		//0xe157[15:0] = 0x7ed4
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xd5); 		//0xe157[7:0]  = 0xd5
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb6); 		//0xe157[15:0] = 0x6b6
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x06); 		//0xe157[15:0] = 0x6b6
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb7); 		//0xe157[7:0]  = 0xb7
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb8); 		//0xe157[15:0] = 0x1b8
			rtd_outl(DVBC_RXMF_WR_STB_1_reg,		0x01); 		//0xe157[15:0] = 0x1b8
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb9); 		//0xe157[7:0]  = 0xb9
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x7a); 		//0xe157[15:0] = 0x737a
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x73); 		//0xe157[15:0] = 0x737a
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x7b); 		//0xe157[7:0]  = 0x7b
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x3c); 		//0xe157[15:0] = 0x7e3c
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7e); 		//0xe157[15:0] = 0x7e3c
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x3d); 		//0xe157[7:0]  = 0x3d
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x7e); 		//0xe157[15:0] = 0x277e
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x27); 		//0xe157[15:0] = 0x277e
			rtd_outl(DVBC_RXMF_WR_STB_reg,			0x7f); 		//0xe157[7:0]  = 0x7f
			break;

		default:
			REALTEK_R_QAM_BASE_WARNING("Unsupported QAM Mode\n");
			goto error_status_get_unsupported_qam_mode;
		}
		break;

	case REALTEK_R_QAM_CONFIG_OC_AM_HUM_EN:
		switch (QamMode) {
		case QAM_QAM_64:
			REALTEK_R_QAM_BASE_INFO("========[OC_AM Set to 64QAM]========\n");
			SelectionIndex = 2;
			rtd_maskl(DVBC_OPT_QAM_MODE_reg, ~DVBC_OPT_QAM_MODE_opt_qam_mode_mask, DVBC_OPT_QAM_MODE_opt_qam_mode(0x02));	//0xe102[2:0] = 0x02
			rtd_outl(DVBC_OPT_TR_DECI_RATIO_1_reg, 	0xee);  	//0xe120[7:0]  = 0xee
			rtd_outl(DVBC_OPT_TR_DECI_RATIO_2_reg, 	0x13);		//0xe121[7:0]  = 0x13
			rtd_outl(DVBC_OPT_DD_EQ_BOUND_reg,		0xc8);		//0xe12e[7:0]  = 0xc8
			rtd_outl(DVBC_OPT_SIGN_DATA_reg,		0xc0);		//0xe12f[7:0]  = 0xc0
			rtd_outl(DVBC_OPT_SIGN_DATA_1_reg, 		0x21);    	//0xe130[7:0]  = 0x21
			rtd_outl(DVBC_OPT_CIRCLE_reg, 			0x05);  	//0xe131[7:0]  = 0x5
			rtd_outl(DVBC_FSM_WR_STB_reg, 			0x18); 		//0xe15d[7:0]  = 0x18
			rtd_outl(DVBC_FSM_WORD_reg,				0xc8); 	  	//0xe15e[31:0] = 0x7ec0ccc8
			rtd_outl(DVBC_FSM_WORD_1_reg,			0xcc);   	//0xe15e[31:0] = 0x7ec0ccc8
			rtd_outl(DVBC_FSM_WORD_2_reg, 			0xc0); 		//0xe15e[31:0] = 0x7ec0ccc8
			rtd_outl(DVBC_FSM_WORD_3_reg,			0x7e); 		//0xe15e[31:0] = 0x7ec0ccc8
			rtd_outl(DVBC_FSM_WORD_4_reg,			0x28); 		//0xe162[31:0] = 0x205bdc28
			rtd_outl(DVBC_FSM_WORD_5_reg, 			0xdc); 		//0xe162[31:0] = 0x205bdc28
			rtd_outl(DVBC_FSM_WORD_6_reg, 			0x5b); 		//0xe162[31:0] = 0x205bdc28
			rtd_outl(DVBC_FSM_WORD_7_reg,			0x20); 		//0xe162[31:0] = 0x205bdc28
			rtd_outl(DVBC_FSM_WORD_8_reg, 			0xa9); 		//0xe166[23:0] = 0x737ea9
			rtd_outl(DVBC_FSM_WORD_9_reg, 			0x7e); 		//0xe166[23:0] = 0x737ea9
			rtd_outl(DVBC_FSM_WORD_10_reg, 			0x73); 		//0xe166[23:0] = 0x737ea9
			rtd_outl(DVBC_FSM_WORD_11_reg, 			0x00); 		//0xe1cc[7:0]  = 0x0
			rtd_outl(DVBC_FSM_WR_STB_reg, 			0x19); 		//0xe15d[7:0]  = 0x19

			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x62); 		//0xe157[15:0] = 0x62
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x00); 		//0xe157[15:0] = 0x62
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x63); 		//0xe157[7:0]  = 0x63
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xc6); 		//0xe157[15:0] = 0x3c6
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x03); 		//0xe157[15:0] = 0x3c6
			rtd_outl(DVBC_RXMF_WR_STB_reg,			0xc7); 		//0xe157[7:0]  = 0xc7
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xc8); 		//0xe157[15:0] = 0xc8
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x00); 		//0xe157[15:0] = 0xc8
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xc9); 		//0xe157[7:0]  = 0xc9
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xca); 		//0xe157[15:0] = 0x7fca
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7f); 		//0xe157[15:0] = 0x7fca
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xcb); 		//0xe157[7:0]  = 0xcb
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x4e); 		//0xe157[15:0] = 0x14e
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x01); 		//0xe157[15:0] = 0x14e
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x4f); 		//0xe157[7:0]  = 0x4f
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x52); 		//0xe157[15:0] = 0x7d52
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7d); 		//0xe157[15:0] = 0x7d52
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x53); 		//0xe157[7:0]  = 0x53
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xd4); 		//0xe157[15:0] = 0x7dd4
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7d); 		//0xe157[15:0] = 0x7dd4
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xd5); 		//0xe157[7:0]  = 0xd5
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb6); 		//0xe157[15:0] = 0x5b6
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x05); 		//0xe157[15:0] = 0x5b6
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb7); 		//0xe157[7:0]  = 0xb7
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb8); 		//0xe157[15:0] = 0x2b8
			rtd_outl(DVBC_RXMF_WR_STB_1_reg,		0x02); 		//0xe157[15:0] = 0x2b8
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb9); 		//0xe157[7:0]  = 0xb9
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x7a); 		//0xe157[15:0] = 0x747a
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x74); 		//0xe157[15:0] = 0x747a
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x7b); 		//0xe157[7:0]  = 0x7b
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x3c); 		//0xe157[15:0] = 0x7d3c
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7d); 		//0xe157[15:0] = 0x7d3c
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x3d); 		//0xe157[7:0]  = 0x3d
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x7e); 		//0xe157[15:0] = 0x267e
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x26); 		//0xe157[15:0] = 0x267e
			rtd_outl(DVBC_RXMF_WR_STB_reg,			0x7f); 		//0xe157[7:0]  = 0x7f
			break;

		case QAM_QAM_256:
			REALTEK_R_QAM_BASE_INFO("========[OC_AM Set to 256QAM]========\n");
			SelectionIndex = 3;
			rtd_maskl(DVBC_OPT_QAM_MODE_reg, ~DVBC_OPT_QAM_MODE_opt_qam_mode_mask, DVBC_OPT_QAM_MODE_opt_qam_mode(0x04));	//0xe102[2:0] = 0x04
			rtd_outl(DVBC_OPT_TR_DECI_RATIO_1_reg,  0xcd);  	//0xe120[7:0]  = 0xcd
			rtd_outl(DVBC_OPT_TR_DECI_RATIO_2_reg, 	0x12);		//0xe121[7:0]  = 0x12
			rtd_outl(DVBC_OPT_DD_EQ_BOUND_reg,		0xd0);		//0xe12e[7:0]  = 0xd0
			rtd_outl(DVBC_OPT_SIGN_DATA_reg,		0x80);		//0xe12f[7:0]  = 0x80
			rtd_outl(DVBC_OPT_SIGN_DATA_1_reg, 		0x43); 		//0xe130[7:0]  = 0x43
			rtd_outl(DVBC_OPT_CIRCLE_reg, 			0x0a);  	//0xe131[7:0]  = 0xa
			rtd_outl(DVBC_FSM_WR_STB_reg, 			0x18); 		//0xe15d[7:0]  = 0x18
			rtd_outl(DVBC_FSM_WORD_reg,				0xc8); 		//0xe15e[31:0] = 0x7ec0ccc8
			rtd_outl(DVBC_FSM_WORD_1_reg,			0xcc); 		//0xe15e[31:0] = 0x7ec0ccc8
			rtd_outl(DVBC_FSM_WORD_2_reg, 			0xc0); 		//0xe15e[31:0] = 0x7ec0ccc8
			rtd_outl(DVBC_FSM_WORD_3_reg,			0x7e); 		//0xe15e[31:0] = 0x7ec0ccc8
			rtd_outl(DVBC_FSM_WORD_4_reg,			0x28); 		//0xe162[31:0] = 0x205bdc28
			rtd_outl(DVBC_FSM_WORD_5_reg, 			0xdc); 		//0xe162[31:0] = 0x205bdc28
			rtd_outl(DVBC_FSM_WORD_6_reg, 			0x5b); 		//0xe162[31:0] = 0x205bdc28
			rtd_outl(DVBC_FSM_WORD_7_reg,			0x20); 		//0xe162[31:0] = 0x205bdc28
			rtd_outl(DVBC_FSM_WORD_8_reg, 			0xa9); 		//0xe166[23:0] = 0x737ea9
			rtd_outl(DVBC_FSM_WORD_9_reg, 			0x7e); 		//0xe166[23:0] = 0x737ea9
			rtd_outl(DVBC_FSM_WORD_10_reg, 			0x73); 		//0xe166[23:0] = 0x737ea9
			rtd_outl(DVBC_FSM_WORD_11_reg, 			0x00); 		//0xe1cc[7:0]  = 0x0
			rtd_outl(DVBC_FSM_WR_STB_reg, 			0x19); 		//0xe15d[7:0]  = 0x19

			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x62); 		//0xe157[15:0] = 0x362
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x03); 		//0xe157[15:0] = 0x362
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x63); 		//0xe157[7:0]  = 0x63
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xc6); 		//0xe157[15:0] = 0x1c6
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x01); 		//0xe157[15:0] = 0x1c6
			rtd_outl(DVBC_RXMF_WR_STB_reg,			0xc7); 		//0xe157[7:0]  = 0xc7
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xc8); 		//0xe157[15:0] = 0x1c8
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x01); 		//0xe157[15:0] = 0x1c8
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xc9); 		//0xe157[7:0]  = 0xc9
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xca); 		//0xe157[15:0] = 0x7eca
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7e); 		//0xe157[15:0] = 0x7eca
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xcb); 		//0xe157[7:0]  = 0xcb
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x4e); 		//0xe157[15:0] = 0x24e
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x02); 		//0xe157[15:0] = 0x24e
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x4f); 		//0xe157[7:0]  = 0x4f
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x52); 		//0xe157[15:0] = 0x7b52
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7b); 		//0xe157[15:0] = 0x7b52
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x53); 		//0xe157[7:0]  = 0x53
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xd4); 		//0xe157[15:0] = 0x7ed4
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7e); 		//0xe157[15:0] = 0x7ed4
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xd5); 		//0xe157[7:0]  = 0xd5
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb6); 		//0xe157[15:0] = 0x6b6
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x06); 		//0xe157[15:0] = 0x6b6
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb7); 		//0xe157[7:0]  = 0xb7
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb8); 		//0xe157[15:0] = 0x1b8
			rtd_outl(DVBC_RXMF_WR_STB_1_reg,		0x01); 		//0xe157[15:0] = 0x1b8
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb9); 		//0xe157[7:0]  = 0xb9
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x7a); 		//0xe157[15:0] = 0x737a
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x73); 		//0xe157[15:0] = 0x737a
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x7b); 		//0xe157[7:0]  = 0x7b
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x3c); 		//0xe157[15:0] = 0x7e3c
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7e); 		//0xe157[15:0] = 0x7e3c
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x3d); 		//0xe157[7:0]  = 0x3d
			rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x7e); 		//0xe157[15:0] = 0x277e
			rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x27); 		//0xe157[15:0] = 0x277e
			rtd_outl(DVBC_RXMF_WR_STB_reg,			0x7f); 		//0xe157[7:0]  = 0x7f
			break;

		default:
			REALTEK_R_QAM_BASE_WARNING("Unsupported QAM Mode\n");
			goto error_status_get_unsupported_qam_mode;
		}
		break;

	default:
		REALTEK_R_QAM_BASE_WARNING("Unknown ConfigMode\n");
		goto error_status_get_unsupported_qam_mode;
	}

	/*
	// Set demod QAM mode with QAM mode register setting table.
		for (i = 0; i < REALTEK_R_OC_QAM_MODE_REG_TABLE_LEN; i++) {
			// Get all information from each register setting entry according to selection index.
			RegStartAddr = QamModeRegTable[i].RegStartAddr;
			Msb          = QamModeRegTable[i].Msb;
			Lsb          = QamModeRegTable[i].Lsb;
			WritingValue = QamModeRegTable[i].WritingValue[SelectionIndex];

			// Set register mask bits.
			if (pDemod->RegAccess.Addr16Bit.SetRegMaskBits(pDemod, RegStartAddr, Msb, Lsb, WritingValue) !=
				FUNCTION_SUCCESS)
				goto error_status_set_demod_registers;
		}
	*/

	// Set demod QAM mode parameter.
	pDemod->QamMode = QamMode;
	pDemod->IsQamModeSet = YES;
	return FUNCTION_SUCCESS;

error_status_get_unsupported_qam_mode:
	return FUNCTION_ERROR;
}


/**

@see   QAM_DEMOD_FP_SET_ALPHA_MODE

*/
int
realtek_r_oc_SetAlphaMode(
	QAM_DEMOD_MODULE *pDemod,
	int AlphaMode
)
{
	static const U32BITS AlphaValueTable[QAM_ALPHA_MODE_NUM][REALTEK_R_QAM_ALPHA_VALUE_TABLE_LEN] = {
		{6,    28,   24,   8,    9,  1011,  1013,  22,  13,  990,   1010,  55,  15,  924,  1008,  314},	// alpha = 0.12
		{5,    30,   25,   6,    9,  1013,  1013,  20,  13,  992,   1009,  53,  16,  925,  1007,  313},	// alpha = 0.13
		{3,    1,    27,   2,    8,  1017,  1013,  16,  14,  996,   1008,  50,  18,  927,  1004,  310},	// alpha = 0.15
		{0,    3,    30,   30,   6,  1022,  1014,  10,  14,  1002,  1006,  45,  21,  931,  1001,  307},	// alpha = 0.18
		{31,   4,    31,   29,   4,  1,     1016,  6,   13,  1006,  1006,  41,  23,  934,  998,   304},	// alpha = 0.20
	};

	int i;
	U32BITS AlphaValue;

	// Set demod alpha mode with alpha value table.
	for (i = 0; i < REALTEK_R_QAM_ALPHA_VALUE_TABLE_LEN; i++) {
		// Get alpha value from alpha value entry according to alpha mode.
		AlphaValue = AlphaValueTable[AlphaMode][i];

		// Set alpha register selection.
		//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_SPEC_ALPHA_SEL, i) != FUNCTION_SUCCESS)
		rtd_maskl(DVBC_RXMF_WR_STB_reg, ~DVBC_RXMF_WR_STB_rxmf_sel_mask, DVBC_RXMF_WR_STB_rxmf_sel(i));

		// Set alpha register value.
		//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_SPEC_ALPHA_VAL, AlphaValue) != FUNCTION_SUCCESS)
		rtd_maskl(DVBC_RXMF_WR_STB_reg, ~DVBC_RXMF_WR_STB_rxmf_coeff_2_0_mask, DVBC_RXMF_WR_STB_rxmf_coeff_2_0(AlphaValue));
		rtd_maskl(DVBC_RXMF_WR_STB_1_reg, ~DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3_mask, DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3(AlphaValue >> 3));

		// Set alpha register strobe.
		// Note: REALTEK_R QAM hardware will clear strobe automatically.
		//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_SPEC_ALPHA_STROBE, ON) != FUNCTION_SUCCESS)
		rtd_maskl(DVBC_RXMF_WR_STB_reg, ~DVBC_RXMF_WR_STB_rxmf_wr_stb_mask, DVBC_RXMF_WR_STB_rxmf_wr_stb(0x1));

	}

	// Set demod alpha mode parameter.
	pDemod->AlphaMode      = AlphaMode;
	pDemod->IsAlphaModeSet = YES;

	return FUNCTION_SUCCESS;
}


/**

@see   QAM_DEMOD_FP_IS_FRAME_LOCKED

*/
int
realtek_r_oc_IsFrameLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	U32BITS LossStatus;

	// Get frame loss status from FS_SYNC_LOST register bits with frame sync. strobe.
	//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_FS_SYNC_STROBE, ON) != FUNCTION_SUCCESS)
	//	goto error_status_set_demod_registers;
	rtd_maskl(DVBC_VT_LSB_HD_reg, ~DVBC_VT_LSB_HD_viterbi_rd_stb_mask, DVBC_VT_LSB_HD_viterbi_rd_stb(0x1));

	//if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_FS_SYNC_LOST, &LossStatus) != FUNCTION_SUCCESS)
	LossStatus = DVBC_INTL_CHG_FLAG_get_fs_sync_lost(rtd_inl(DVBC_INTL_CHG_FLAG_reg));

	// Determine answer according to frame loss status.
	if (LossStatus == NOT_LOST)
		*pAnswer = YES;
	else
		*pAnswer = NO;

	return FUNCTION_SUCCESS;
}


int
realtek_r_oc_IsTrLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	S32BITS tr_ld_o;//, cr_ld_o;

	// Get TR lock status from register bits.
	//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_INNER_DATA_STROBE, ON) != FUNCTION_SUCCESS)
	rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));

	//if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_SPEC_SIGNAL_INDICATOR, &tr_ld_o) != FUNCTION_SUCCESS)
	//tr_ld_o = ((rtd_inl(DVBC_INRX_MO2_1_reg) & 0x80 >> 7) | (rtd_inl(DVBC_INRX_MO2_2_reg) & 0x1F << 1));//Mac6
	tr_ld_o = ((rtd_inl(DVBC_INRX_MO2_1_reg) & 0x38) >> 3);//Merlin4

	//if(pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_CR_LD, &cr_ld_o) != FUNCTION_SUCCESS)
	//	goto error_status_get_demod_registers;

	if (tr_ld_o > 0x1f) tr_ld_o = 0x20 - tr_ld_o;
	//if(cr_ld_o>0x1f) cr_ld_o=0x20-cr_ld_o;

	// Determine answer according to tr_ld_o.
	if ((tr_ld_o >= 1) && (tr_ld_o < 7))
		//if((tr_ld_o > 1)||(cr_ld_o > 1))
		*pAnswer = YES;
	else
		*pAnswer = NO;

	return FUNCTION_SUCCESS;
}


int
realtek_r_oc_IsCrLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	S32BITS cr_ld_o;

	// Get TR lock status from register bits.
	//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_INNER_DATA_STROBE, ON) != FUNCTION_SUCCESS)
	rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));

	//if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_CR_LD, &cr_ld_o) != FUNCTION_SUCCESS)
	cr_ld_o = ((rtd_inl(DVBC_INRX_MO2_1_reg) & 0xE0 >> 5) | (rtd_inl(DVBC_INRX_MO2_2_reg) & 0x07 << 3));

	if (cr_ld_o > 0x1f) cr_ld_o = 0x20 - cr_ld_o;

	// Determine answer according to cr_ld_o.
	if (cr_ld_o > 1)
		*pAnswer = YES;
	else
		*pAnswer = NO;

	return FUNCTION_SUCCESS;
}


int
realtek_r_oc_IsInnerLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	U32BITS fsm_state_o;

	// Get FSM state status from register bits.
	//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_INNER_DATA_STROBE, ON) != FUNCTION_SUCCESS)
	rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));

	//if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_FSM_STATE, &fsm_state_o) != FUNCTION_SUCCESS)
	fsm_state_o = ((rtd_inl(DVBC_INRX_MO2_reg) & 0x78) >> 3);

	// Determine answer according to fsm_state_o.
	if (fsm_state_o == 12)
		*pAnswer = YES;
	else
		*pAnswer = NO;

	return FUNCTION_SUCCESS;
}


int
realtek_r_oc_IsNoSig(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	int ret;
	ret = realtek_r_oc_IsTrLocked(pDemod, pAnswer);
	*pAnswer = (*pAnswer) ? NO : YES;
	return ret;
}


/**

@see   QAM_DEMOD_FP_GET_ERROR_RATE

*/
int
realtek_r_oc_GetErrorRate(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS TestVolume,
	unsigned int WaitTimeMsMax,
	U32BITS *pBerNum,
	U32BITS *pBerDen,
	U32BITS *pPerNum,
	U32BITS *pPerDen
)
{
	BASE_INTERFACE_MODULE *pBaseInterface;

	unsigned int i;
	U32BITS TestPacketNum;
	unsigned int  WaitCnt;
	int FrameLock;
	U32BITS BerReg2, BerReg2Msb, BerReg2Lsb;
	U32BITS BerReg0, BerReg1;
	U32BITS BerData_1, BerData_2;
	U32BITS Reg0Data_1, Reg0Data_2, Reg1Data_1, Reg1Data_2, Reg1Data_3;
    unsigned int TestVolumeNoWait = 5;

	// Get base interface.
	pBaseInterface = pDemod->pBaseInterface;

	if (TestVolume != 0) {
	// Calculate test packet number and wait counter value.
	TestPacketNum = 0x1 << (TestVolume * 2 + 4);
	WaitCnt = WaitTimeMsMax / REALTEK_R_QAM_BER_WAIT_TIME_MS;

	// Set TEST_VOLUME with test volume.
	//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_OC_TEST_VOLUME, TestVolume) != FUNCTION_SUCCESS)
	rtd_maskl(DVBC_OC_BER_EN_reg, ~DVBC_OC_BER_EN_oc_volume_mask, DVBC_OC_BER_EN_oc_volume(TestVolume));

	// Clear and enable error counter.
	//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_OC_BERT_EN, OFF) != FUNCTION_SUCCESS)
	rtd_maskl(DVBC_OC_BER_EN_reg, ~DVBC_OC_BER_EN_oc_ber_en_mask, DVBC_OC_BER_EN_oc_ber_en(0x0));

	//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_OC_BERT_EN, ON) != FUNCTION_SUCCESS)
	rtd_maskl(DVBC_OC_BER_EN_reg, ~DVBC_OC_BER_EN_oc_ber_en_mask, DVBC_OC_BER_EN_oc_ber_en(0x1));

	rtd_maskl(DVBC_OC_BER_EN_reg, ~DVBC_OC_BER_EN_ber_en_stb_oc_mask, DVBC_OC_BER_EN_ber_en_stb_oc(0x1));

	// Check if error test is finished.
	for (i = 0; i < WaitCnt; i++) {
		// Check if demod is frame-locked.
		if (pDemod->IsFrameLocked(pDemod, &FrameLock) != FUNCTION_SUCCESS)
			goto error_status_get_demod_registers;

		if (FrameLock == NO)
			goto error_status_frame_lock;

		// Wait a minute.
		// Note: The input unit of WaitMs() is ms.
		pBaseInterface->WaitMs(pBaseInterface, REALTEK_R_QAM_BER_WAIT_TIME_MS);

		// Set error counter strobe.
		// Note: REALTEK_R QAM hardware will clear strobe automatically.
		//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_OC_BER_RD_STROBE, ON) != FUNCTION_SUCCESS)
		rtd_maskl(DVBC_OC_RSD_DISABLE_reg, ~DVBC_OC_RSD_DISABLE_ber_rd_strobe_mask, DVBC_OC_RSD_DISABLE_ber_rd_strobe(0x1));

		//Mac6: IC net wrong connection by DIC
		//rtd_maskl(DVBC_OPT_RSD_DISABLE_reg, ~DVBC_OPT_RSD_DISABLE_opt_ber_rd_strobe_mask, DVBC_OPT_RSD_DISABLE_opt_ber_rd_strobe(0x1));

		// Check if error test is finished.
		//if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_OC_BER_REG2_15_0, &BerReg2Lsb) != FUNCTION_SUCCESS)
		BerData_1 = DVBC_RS_BLOCK_COUNT_0_get_rs_block_count_0(rtd_inl(DVBC_RS_BLOCK_COUNT_0_reg));
		BerData_2 = (DVBC_RS_BLOCK_COUNT_1_get_rs_block_count_1(rtd_inl(DVBC_RS_BLOCK_COUNT_1_reg)) << 8);
		BerReg2Lsb = (BerData_2 + BerData_1);

		//if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_OC_BER_REG2_18_16, &BerReg2Msb) != FUNCTION_SUCCESS)
		BerReg2Msb = DVBC_RS_BER_COUNT_2_get_rs_block_count_2(rtd_inl(DVBC_RS_BER_COUNT_2_reg));
		BerReg2 = ((BerReg2Msb << REALTEK_R_OC_BER_REG2_MSB_SHIFT) | BerReg2Lsb);

		if (BerReg2 == TestPacketNum)
			break;
	}

	// Check time-out status.
	if (i == WaitCnt)
		goto error_status_time_out;
    
	} else {
	
		// Check if demod is frame-locked.
		if (pDemod->IsFrameLocked(pDemod, &FrameLock) != FUNCTION_SUCCESS) {
			goto error_status_get_demod_registers;
		}

		if (FrameLock == NO) {
			goto error_status_frame_lock;
		}

		TestPacketNum = 0x1 << (TestVolumeNoWait * 2 + 4);


        rtd_maskl(DVBC_OC_RSD_DISABLE_reg, ~DVBC_OC_RSD_DISABLE_ber_rd_strobe_mask, DVBC_OC_RSD_DISABLE_ber_rd_strobe(0x1));
        BerData_1 = DVBC_RS_BLOCK_COUNT_0_get_rs_block_count_0(rtd_inl(DVBC_RS_BLOCK_COUNT_0_reg));
		BerData_2 = (DVBC_RS_BLOCK_COUNT_1_get_rs_block_count_1(rtd_inl(DVBC_RS_BLOCK_COUNT_1_reg)) << 8);
		BerReg2Lsb = (BerData_2 + BerData_1);

        BerReg2Msb = DVBC_RS_BER_COUNT_2_get_rs_block_count_2(rtd_inl(DVBC_RS_BER_COUNT_2_reg));
		BerReg2 = ((BerReg2Msb << REALTEK_R_OC_BER_REG2_MSB_SHIFT) | BerReg2Lsb);
		//TestPacketNum = BerReg2;

		if ((BerReg2 != 0) && (BerReg2 != TestPacketNum))
			goto error_test_not_ready;


	}

	// Get BER register 0 from BER_REG0.
	//if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_OC_BER_REG0, &BerReg0) != FUNCTION_SUCCESS)
	Reg0Data_1 = 0, Reg0Data_2 = 0;
	Reg0Data_1 = DVBC_RS_UNCORR_COUNT_0_get_rs_uncorr_count_0(rtd_inl(DVBC_RS_UNCORR_COUNT_0_reg));
	Reg0Data_2 = (DVBC_RS_UNCORR_COUNT_1_get_rs_uncorr_count_1(rtd_inl(DVBC_RS_UNCORR_COUNT_1_reg)) << 8);
	BerReg0 = (Reg0Data_1 + Reg0Data_2);

	// Get BER register 1 from BER_REG1.
	//if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_OC_BER_REG1, &BerReg1) != FUNCTION_SUCCESS)
	Reg1Data_1 = 0, Reg1Data_2 = 0, Reg1Data_3 = 0;
	Reg1Data_1 = DVBC_RS_BER_COUNT_0_get_rs_ber_count_0(rtd_inl(DVBC_RS_BER_COUNT_0_reg));
	Reg1Data_2 = (DVBC_RS_BER_COUNT_1_get_rs_ber_count_1(rtd_inl(DVBC_RS_BER_COUNT_1_reg)) << 8);
	Reg1Data_3 = (DVBC_RS_BER_COUNT_2_get_rs_ber_count_2(rtd_inl(DVBC_RS_BER_COUNT_2_reg)) << 16);
	BerReg1 = (Reg1Data_1 + Reg1Data_2 + Reg1Data_3);

	// Set BER numerator and denominator.
	*pBerNum = 12 * BerReg0 + BerReg1;
	*pBerDen = 896 * TestPacketNum;

	// Set PER numerator and denominator.
	*pPerNum = BerReg0;
	*pPerDen = TestPacketNum;

    // Set TEST_VOLUME with test volume.
    rtd_maskl(DVBC_OC_BER_EN_reg, ~DVBC_OC_BER_EN_oc_volume_mask, DVBC_OC_BER_EN_oc_volume(TestVolumeNoWait));
    rtd_maskl(DVBC_OC_BER_EN_reg, ~DVBC_OC_BER_EN_oc_ber_en_mask, DVBC_OC_BER_EN_get_oc_ber_en(0x0));
    rtd_maskl(DVBC_OC_BER_EN_reg, ~DVBC_OC_BER_EN_oc_ber_en_mask, DVBC_OC_BER_EN_get_oc_ber_en(0x1));
    rtd_maskl(DVBC_OC_BER_EN_reg, ~DVBC_OC_BER_EN_ber_en_stb_oc_mask, DVBC_OC_BER_EN_ber_en_stb_oc(0x1));
    
	m_BerNum = *pBerNum;
	m_BerDen = *pBerDen;

	// Set PER numerator and denominator.
	m_PerNum = *pPerNum;
	m_PerDen = *pPerDen;


	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
error_status_frame_lock:
error_status_time_out:

     // Set TEST_VOLUME with test volume.
    rtd_maskl(DVBC_OC_BER_EN_reg, ~DVBC_OC_BER_EN_oc_volume_mask, DVBC_OC_BER_EN_oc_volume(TestVolumeNoWait));
    rtd_maskl(DVBC_OC_BER_EN_reg, ~DVBC_OC_BER_EN_oc_ber_en_mask, DVBC_OC_BER_EN_get_oc_ber_en(0x0));
    rtd_maskl(DVBC_OC_BER_EN_reg, ~DVBC_OC_BER_EN_oc_ber_en_mask, DVBC_OC_BER_EN_get_oc_ber_en(0x1));
    rtd_maskl(DVBC_OC_BER_EN_reg, ~DVBC_OC_BER_EN_ber_en_stb_oc_mask, DVBC_OC_BER_EN_ber_en_stb_oc(0x1));
	return FUNCTION_ERROR;

error_test_not_ready:

	// Set BER numerator and denominator.
	*pBerNum = m_BerNum;
	*pBerDen = m_BerDen;

	// Set PER numerator and denominator.
	*pPerNum = m_PerNum;
	*pPerDen = m_PerDen;
	return FUNCTION_SUCCESS;

}


/**

@see   QAM_DEMOD_FP_GET_SIGNAL_STRENGTH

*/
int
realtek_r_oc_GetSignalStrength(
	QAM_DEMOD_MODULE *pDemod,
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

	REALTEK_R_QAM_BASE_DBG("TxRSSIdBm = %d, pSignalStrength = "PT_U32BITS"\n", TxRSSIdBm, *pSignalStrength);

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
	// Note: Map IF AGC value (1023 ~ -1024) to signal strength (0 ~ 100).
	*pSignalStrength = (102300 - IfAgcValue * 100) / 2047;

	REALTEK_R_QAM_BASE_DBG("TxRSSIdBm = %d, pSignalStrength = "PT_U32BITS"\n", TxRSSIdBm, *pSignalStrength);

success_status_non_frame_lock:
	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
	return FUNCTION_ERROR;

#endif
}


/**

@see   QAM_DEMOD_FP_GET_SIGNAL_QUALITY

*/
int
realtek_r_oc_GetSignalQuality(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS *pSignalQuality
)
{
	int FrameLock = 0;
	U32BITS Mse = 0;
	S32BITS MiddleResult = 0;
	MPI MpiMse, MpiResult;

	memset(&MpiMse, 0 , sizeof(MpiMse));
	memset(&MpiResult, 0 , sizeof(MpiResult));

	// Get demod frame lock status.
	if (pDemod->IsFrameLocked(pDemod, &FrameLock) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// If demod is not frame-locked, set signal quality with zero.
	if (FrameLock == NO) {
		*pSignalQuality = 0;
		goto success_status_non_frame_lock;
	}

	// Get mean-square error from MSE.
	if (realtek_r_qam_GetInnerStrobeRegBits(pDemod, QAM_MSE, &Mse) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Determine signal quality according to MSE value.
	// Note: Map MSE value (pow(2, 19) ~ pow(2, 17)) to signal quality (0 ~ 100).
	//       If MSE value < pow(2, 17), signal quality is 100.
	//       If MSE value > pow(2, 19), signal quality is 0.
	if (Mse > 524288) {
		*pSignalQuality = 0;
	} else if (Mse < 131072) {
		*pSignalQuality = 100;
	} else {
		MpiSetValue(&MpiMse, (S32BITS)Mse);
		MpiLog2(&MpiResult, MpiMse, REALTEK_R_QAM_SIGNAL_QUALITY_FRAC_BIT_NUM);
		MpiGetValue(MpiResult, &MiddleResult);

		*pSignalQuality = (243200 - MiddleResult * 100) / 256;
	}

success_status_non_frame_lock:
	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
	return FUNCTION_ERROR;
}



/**

@see   QAM_DEMOD_FP_UPDATE_FUNCTION

*/
int
realtek_r_oc_UpdateFunction(
	QAM_DEMOD_MODULE *pDemod
)
{
	int FrameLock;
	U32BITS MpegSyncMode;
	REALTEK_R_QAM_EXTRA_MODULE *pExtra = &(pDemod->Extra.RealtekRQam);

	pDemod->IsFrameLocked(pDemod, &FrameLock);
	REALTEK_R_QAM_BASE_DBG("[OpenCable_THREAD]MPEG Sync Decoding Mode Check, FrameLock = %d\n", FrameLock);

	if (FrameLock != YES) {
		pExtra->MpegOffSyncCount = 0;
		return FUNCTION_SUCCESS;
	}

	//pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_OC_MPEG_SYNC_MODE, &MpegSyncMode);
	MpegSyncMode = DVBC_OC_MPEG_SYNC_MODE_get_oc_mpeg_sync_mode(rtd_inl(DVBC_OC_MPEG_SYNC_MODE_reg));
	REALTEK_R_QAM_BASE_DBG("[OpenCable_THREAD]MpegSyncMode = "PT_U32BITS", pExtra->MpegOffSyncCount = %d\n", MpegSyncMode, pExtra->MpegOffSyncCount);

	if (MpegSyncMode == 0) {
		pExtra->MpegOffSyncCount ++;

		if (pExtra->MpegOffSyncCount >= 2) {
			//pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_SOFT_RESET_FF, 1);
			rtd_maskl(DVBC_VT_LSB_HD_reg, ~DVBC_VT_LSB_HD_soft_reset_ff_mask, DVBC_VT_LSB_HD_soft_reset_ff(0x1));
			REALTEK_R_QAM_BASE_DBG("[OpenCable_THREAD]MPEG Sync Decoding TimeOut..., Do Outer Demod Reset\n");
			pExtra->MpegOffSyncCount = 0;
		}
	} else {
		pExtra->MpegOffSyncCount = 0;
	}

	return FUNCTION_SUCCESS;
}


/**

@see   QAM_DEMOD_FP_RESET_FUNCTION

*/
int
realtek_r_oc_ResetFunction(
	QAM_DEMOD_MODULE *pDemod
)
{
	// 20120814 alan modified for update function
	REALTEK_R_QAM_EXTRA_MODULE *pExtra;

	// Get demod extra module;
	pExtra = &(pDemod->Extra.RealtekRQam);
	pExtra->MpegOffSyncCount = 0;
	// REALTEK_R OpenCable does not use UpdateFunction(), so we just return FUNCTION_SUCCESS.

	return FUNCTION_SUCCESS;
}


/*
int
realtek_r_oc_FixFsm(
	QAM_DEMOD_MODULE *pDemod,
	unsigned char enable
)
{
	typedef struct {
		unsigned short RegStartAddr;
		unsigned char Msb;
		unsigned char Lsb;
		U32BITS WritingValue;
	}
	FSM_REG_ENTRY;

	static const FSM_REG_ENTRY FsmRegTableA[] = {
		// RegStartAddr,   Msb,    Lsb,    WritingValue
		{0xe12b,           1,      	1,      0x0,          	},
		{0xe15d,           7,      	0,      0x0,          	},
		{0xe15e,           31,     	0,      0x7f00cccc, },
		{0xe162,           31,     	0,      0x205bdc28, },
		{0xe166,           23,     	0,      0x737fa9,   	},
		{0xe1cc,           7,      	0,      0x0,          	},
		{0xe15d,           7,      	0,      0x1,          	},
		{0xe15d,           7,      	0,      0x18,         	},
		{0xe15e,           31,     	0,      0x7f00cccc, },
		{0xe162,           31,     	0,      0x205bdc28, },
		{0xe166,           23,     	0,      0x737ea9,    	},
		{0xe1cc,           7,      	0,      0x0,          	},
		{0xe15d,           7,      	0,      0x19,         	},
	};

	static const FSM_REG_ENTRY FsmRegTableB[] = {
		// RegStartAddr,   Msb,    Lsb,    WritingValue
		{0xe12b,           1,      	1,      0x1,          	},
		{0xe15d,           7,      	0,      0x0,          	},
		{0xe15e,           31,     	0,      0x8c49d000,	},
		{0xe162,           31,     	0,      0x1f2,        	},
		{0xe166,           23,     	0,      0x1898a0,   	},
		{0xe1cc,           7,      	0,      0x0,          	},
		{0xe15d,           7,      	0,      0x1,          	},
		{0xe15d,           7,      	0,      0x18,         	},
		{0xe15e,           31,     	0,      0x7f00ccc8, },
		{0xe162,           31,     	0,      0x205bdc28, },
		{0xe166,           23,     	0,      0x737ea9,    	},
		{0xe1cc,           7,      	0,      0x0,          	},
		{0xe15d,           7,      	0,      0x19,         	},
	};

	int i;
	unsigned short RegStartAddr;
	unsigned char Msb;
	unsigned char Lsb;
	U32BITS WritingValue;

	if (enable == 1){
		for (i = 0; i < sizeof(FsmRegTableA) / sizeof(FSM_REG_ENTRY); i++) {
			RegStartAddr = FsmRegTableA[i].RegStartAddr;
			Msb = FsmRegTableA[i].Msb;
			Lsb = FsmRegTableA[i].Lsb;
			WritingValue = FsmRegTableA[i].WritingValue;

			if (pDemod->RegAccess.Addr16Bit.SetRegMaskBits(pDemod, RegStartAddr, Msb, Lsb, WritingValue) != FUNCTION_SUCCESS)
				goto error_status_set_demod_registers;
		}
	} else {
		for (i = 0; i < sizeof(FsmRegTableB) / sizeof(FSM_REG_ENTRY); i++) {
			RegStartAddr = FsmRegTableB[i].RegStartAddr;
			Msb = FsmRegTableB[i].Msb;
			Lsb = FsmRegTableB[i].Lsb;
			WritingValue = FsmRegTableB[i].WritingValue;

			if (pDemod->RegAccess.Addr16Bit.SetRegMaskBits(pDemod, RegStartAddr, Msb, Lsb, WritingValue) != FUNCTION_SUCCESS)
				goto error_status_set_demod_registers;
		}
	}

    return FUNCTION_SUCCESS;

error_status_set_demod_registers:
    return FUNCTION_ERROR;
}
*/


/**

@see   QAM_DEMOD_FP_NBD_SRS

*/
int
realtek_r_oc_NbdAndSrs(
	QAM_DEMOD_MODULE *pDemod,
	unsigned char *pNosig,
	U32BITS *pPowerDiff,
	U32BITS *pSymbolRate,
	unsigned char WaitAgcStableFlag
)
{
	unsigned long stime = 0;
	U32BITS Srs_nbd_done = 0, pow_diff = 0, PosToneDetect = 0, CrystalFreqHz = 0;
	int i = 0;//, aagc_ld=0;
	unsigned int PosToneMaxIdx[3], NegToneMaxIdx[3];
	unsigned int SrsWeight[3], SrsWeightMax = 0;

	//turn off HW reset
	rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~(DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask)  , DVBC_OPT_RST_STATE0_ON_opt_retry_on(0));

	if (WaitAgcStableFlag) {
		pDemod->SoftwareReset(pDemod);
		tv_osal_msleep(30);
	}

	rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_start_srs_nbd_mask, DVBC_OPT_START_SRS_NBD_opt_start_srs_nbd(0));
	rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_start_srs_nbd_mask, DVBC_OPT_START_SRS_NBD_opt_start_srs_nbd(1));

	stime = tv_osal_time();
	while ((tv_osal_time() - stime) <= 50) { //nbd wait 50ms per times
		Srs_nbd_done = DVBC_SRS_NBD_DONE_get_srs_nbd_done(rtd_inl(DVBC_SRS_NBD_DONE_reg));  //QAMD_DONE
		tv_osal_msleep(5);
		REALTEK_R_QAM_BASE_DBG("[NBD&SYS] Srs_nbd_done="PT_U32BITS", time=%lu\n", Srs_nbd_done, tv_osal_time() - stime);
		if (Srs_nbd_done == 1) break;
	}

	if (Srs_nbd_done != 1) {
		REALTEK_R_QAM_BASE_WARNING("[Nbd and Srs] NBD and SRS checked TIMEOUT...... \n");
		return FUNCTION_ERROR;
	}

	pow_diff = (DVBC_SOLT_AVG_POW_DIF_1_get_solt_avg_pow_dif_15_8(rtd_inl(DVBC_SOLT_AVG_POW_DIF_1_reg)) << 8) | DVBC_SOLT_AVG_POW_DIF_get_solt_avg_pow_dif_7_0(rtd_inl(DVBC_SOLT_AVG_POW_DIF_reg));
	PosToneMaxIdx[0] = (DVBC_POS_TONE_MAX_IDX_0_1_get_pos_tone_max_idx_0_11_8(rtd_inl(DVBC_POS_TONE_MAX_IDX_0_1_reg)) << 8) | DVBC_POS_TONE_MAX_IDX_0_get_pos_tone_max_idx_0_7_0(rtd_inl(DVBC_POS_TONE_MAX_IDX_0_reg));
	PosToneMaxIdx[1] = (DVBC_POS_TONE_MAX_IDX_1_1_get_pos_tone_max_idx_1_11_8(rtd_inl(DVBC_POS_TONE_MAX_IDX_1_1_reg)) << 8) | DVBC_POS_TONE_MAX_IDX_1_get_pos_tone_max_idx_1_7_0(rtd_inl(DVBC_POS_TONE_MAX_IDX_1_reg));
	PosToneMaxIdx[2] = (DVBC_POS_TONE_MAX_IDX_2_1_get_pos_tone_max_idx_2_11_8(rtd_inl(DVBC_POS_TONE_MAX_IDX_2_1_reg)) << 8) | DVBC_POS_TONE_MAX_IDX_2_get_pos_tone_max_idx_2_7_0(rtd_inl(DVBC_POS_TONE_MAX_IDX_2_reg));
	NegToneMaxIdx[0] = (DVBC_NEG_TONE_MAX_IDX_0_1_get_neg_tone_max_idx_0_11_8(rtd_inl(DVBC_NEG_TONE_MAX_IDX_0_1_reg)) << 8) | DVBC_NEG_TONE_MAX_IDX_0_get_neg_tone_max_idx_0_7_0(rtd_inl(DVBC_NEG_TONE_MAX_IDX_0_reg));
	NegToneMaxIdx[1] = (DVBC_NEG_TONE_MAX_IDX_1_1_get_neg_tone_max_idx_1_11_8(rtd_inl(DVBC_NEG_TONE_MAX_IDX_1_1_reg)) << 8) | DVBC_NEG_TONE_MAX_IDX_1_get_neg_tone_max_idx_1_7_0(rtd_inl(DVBC_NEG_TONE_MAX_IDX_1_reg));
	NegToneMaxIdx[2] = (DVBC_NEG_TONE_MAX_IDX_2_1_get_neg_tone_max_idx_2_11_8(rtd_inl(DVBC_NEG_TONE_MAX_IDX_2_1_reg)) << 8) | DVBC_NEG_TONE_MAX_IDX_2_get_neg_tone_max_idx_2_7_0(rtd_inl(DVBC_NEG_TONE_MAX_IDX_2_reg));

	pDemod->GetCrystalFreqHz(pDemod, &CrystalFreqHz);

	if (PosToneMaxIdx[0] > 0) {
		if (ABS(PosToneMaxIdx[0], NegToneMaxIdx[0]) < 3) {
			SrsWeight[0] = 6;
		} else if (ABS(PosToneMaxIdx[0], NegToneMaxIdx[1]) < 3) {
			SrsWeight[0] = 5;
		} else if (ABS(PosToneMaxIdx[0], NegToneMaxIdx[2]) < 3) {
			SrsWeight[0] = 4;
		} else {
			SrsWeight[0] = 0;
		}
	} else {
		SrsWeight[0] = 0;
	}

	REALTEK_R_QAM_BASE_DBG("[NBD&SYS] PosToneMaxIdx[0] = %u, NegToneMaxIdx[0] = %u, SrsWeight[0] = %u \n", PosToneMaxIdx[0], NegToneMaxIdx[0], SrsWeight[0]);

	if (PosToneMaxIdx[1] > 0) {
		if (ABS(PosToneMaxIdx[1], NegToneMaxIdx[0]) < 3) {
			SrsWeight[1] = 5;
		} else if (ABS(PosToneMaxIdx[1], NegToneMaxIdx[1]) < 3) {
			SrsWeight[1] = 4;
		} else if (ABS(PosToneMaxIdx[1], NegToneMaxIdx[2]) < 3) {
			SrsWeight[1] = 3;
		} else {
			SrsWeight[1] = 0;
		}
	} else {
		SrsWeight[1] = 0;
	}

	REALTEK_R_QAM_BASE_DBG("[NBD&SYS] PosToneMaxIdx[1] = %u, NegToneMaxIdx[1] = %u, SrsWeight[1] = %u \n", PosToneMaxIdx[1], NegToneMaxIdx[1], SrsWeight[1]);

	if (PosToneMaxIdx[2] > 0) {
		if (ABS(PosToneMaxIdx[2], NegToneMaxIdx[0]) < 3) {
			SrsWeight[2] = 4;
		} else if (ABS(PosToneMaxIdx[2], NegToneMaxIdx[1]) < 3) {
			SrsWeight[2] = 3;
		} else if (ABS(PosToneMaxIdx[2], NegToneMaxIdx[2]) < 3) {
			SrsWeight[2] = 2;
		} else {
			SrsWeight[2] = 0;
		}
	} else {
		SrsWeight[2] = 0;
	}

	REALTEK_R_QAM_BASE_DBG("[NBD&SYS] PosToneMaxIdx[2] = %u, NegToneMaxIdx[2] = %u, SrsWeight[2] = %u \n", PosToneMaxIdx[2], NegToneMaxIdx[2], SrsWeight[2]);

	for (i = 0; i < 3; i++) {

		if (SrsWeight[i] > SrsWeightMax) {
			SrsWeightMax = SrsWeight[i];
			PosToneDetect = PosToneMaxIdx[i];
		}

	}

	if (SrsWeightMax == 0) {
		*pNosig = 1;
		*pPowerDiff = pow_diff;
		*pSymbolRate = 0; //No signal

	} else {
		*pNosig = 0;
		*pPowerDiff = pow_diff;
		*pSymbolRate = (PosToneDetect * (CrystalFreqHz >> 10)) >> 4;
		//symbol_rate_det = (ADC_freq/2) * (pos_frq_tone_idx / 8192);	//fft_size :8192
	}

	REALTEK_R_QAM_BASE_DBG("[NBD&SYS] pNosig = %u, pPowerDiff = "PT_U32BITS", pSymbolRate = "PT_U32BITS" \n", *pNosig, *pPowerDiff, *pSymbolRate);

	//turn on HW reset
	rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~(DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask)  , DVBC_OPT_RST_STATE0_ON_opt_retry_on(1));

	return FUNCTION_SUCCESS;
}


/**

@see   QAM_DEMOD_FP_QAMD

*/
int
realtek_r_oc_Qamd(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS *pQammode,
	unsigned char MetricEn
)
{
	unsigned long stime = 0;
	unsigned int reg_qamd_done = 0;
	int detect_dataZ0 = 0, detect_dataZ1 = 0, detect_dataZ2 = 0, detect_dataZ3 = 0, detect_dataZ4 = 0, detect_dataZ5 = 0;

	//fix FSM to 5
	rtd_maskl(DVBC_FSM_FIX_reg, ~(DVBC_FSM_FIX_opt_fsm_fix_ste_en_mask | DVBC_FSM_FIX_opt_fsm_fix_ste_end_mask)  , (DVBC_FSM_FIX_opt_fsm_fix_ste_en(1) | DVBC_FSM_FIX_opt_fsm_fix_ste_end(5)));
	rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask, DVBC_OPT_RST_STATE0_ON_opt_retry_on(0));

	stime = tv_osal_time();

	rtd_maskl(DVBC_OPT_QAMD_SELF_RESET_reg, ~DVBC_OPT_QAMD_SELF_RESET_opt_qamd_self_reset_mask, 0x0); //reset auto qam
	rtd_maskl(DVBC_OPT_QAMD_SELF_RESET_reg, ~DVBC_OPT_QAMD_SELF_RESET_opt_qamd_self_reset_mask, 0x1); //reset auto qam

	while ((tv_osal_time() - stime) <= 150) {
		tv_osal_msleep(5);
		rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, 0x1); //QAM_INNER_DATA_STROBE
		reg_qamd_done = DVBC_REG_QAMD_DONE_get_reg_qamd_done(rtd_inl(DVBC_REG_QAMD_DONE_reg));  //QAMD_DONE
		if (reg_qamd_done == 1) break;
	}
	REALTEK_R_QAM_BASE_DBG("[AUTO QAM] reg_qamd_done=%d; time=%lu\n", reg_qamd_done, tv_osal_time() - stime);

	if (reg_qamd_done == 1) {
		detect_dataZ0 = (DVBC_QAMD_RV_ZN0_CDF_1_get_qamd_rv_zn0_cdf_9_8(rtd_inl(DVBC_QAMD_RV_ZN0_CDF_1_reg)) << 8)  | DVBC_QAMD_RV_ZN0_CDF_get_qamd_rv_zn0_cdf_7_0(rtd_inl(DVBC_QAMD_RV_ZN0_CDF_reg));
		detect_dataZ1 = (DVBC_QAMD_RV_ZN1_CDF_1_get_qamd_rv_zn1_cdf_9_8(rtd_inl(DVBC_QAMD_RV_ZN1_CDF_1_reg)) << 8)  | DVBC_QAMD_RV_ZN1_CDF_get_qamd_rv_zn1_cdf_7_0(rtd_inl(DVBC_QAMD_RV_ZN1_CDF_reg));
		detect_dataZ2 = (DVBC_QAMD_RV_ZN2_CDF_1_get_qamd_rv_zn2_cdf_9_8(rtd_inl(DVBC_QAMD_RV_ZN2_CDF_1_reg)) << 8)  | DVBC_QAMD_RV_ZN2_CDF_get_qamd_rv_zn2_cdf_7_0(rtd_inl(DVBC_QAMD_RV_ZN2_CDF_reg));
		detect_dataZ3 = (DVBC_QAMD_RV_ZN3_CDF_1_get_qamd_rv_zn3_cdf_9_8(rtd_inl(DVBC_QAMD_RV_ZN3_CDF_1_reg)) << 8)  | DVBC_QAMD_RV_ZN3_CDF_get_qamd_rv_zn3_cdf_7_0(rtd_inl(DVBC_QAMD_RV_ZN3_CDF_reg));
		detect_dataZ4 = (DVBC_QAMD_RV_ZN4_CDF_1_get_qamd_rv_zn4_cdf_9_8(rtd_inl(DVBC_QAMD_RV_ZN4_CDF_1_reg)) << 8)  | DVBC_QAMD_RV_ZN4_CDF_get_qamd_rv_zn4_cdf_7_0(rtd_inl(DVBC_QAMD_RV_ZN4_CDF_reg));
		detect_dataZ5 = (DVBC_QAMD_RV_ZN5_CDF_1_get_qamd_rv_zn5_cdf_9_8(rtd_inl(DVBC_QAMD_RV_ZN5_CDF_1_reg)) << 8)  | DVBC_QAMD_RV_ZN5_CDF_get_qamd_rv_zn5_cdf_7_0(rtd_inl(DVBC_QAMD_RV_ZN5_CDF_reg));

		REALTEK_R_QAM_BASE_DBG("[Qamd] detect_dataZ0=%d, detect_dataZ1 = %d, detect_dataZ2 = %d, detect_dataZ3 = %d, detect_dataZ4 = %d, detect_dataZ5 = %d\n", detect_dataZ0, detect_dataZ1, detect_dataZ2, detect_dataZ3, detect_dataZ4, detect_dataZ5);
		REALTEK_R_QAM_BASE_DBG("[Qamd] d0 = %d, THR_16 = %d\n", ((detect_dataZ0 - detect_dataZ4) + (detect_dataZ2 - detect_dataZ3) + (detect_dataZ5 - detect_dataZ1)), QAMD_THD_16QAM);
		REALTEK_R_QAM_BASE_DBG("[Qamd] d1 = %d, THR_32 = %d\n", ((detect_dataZ1 - detect_dataZ0) + (detect_dataZ1 - detect_dataZ2) + (detect_dataZ3 - detect_dataZ2)), QAMD_THD_32QAM);
		REALTEK_R_QAM_BASE_DBG("[Qamd] d2 = %d, THR_64 = %d\n", ((detect_dataZ0 - detect_dataZ2) + (detect_dataZ0 - detect_dataZ3) + (detect_dataZ0 - detect_dataZ4) + (detect_dataZ4 - detect_dataZ2)), QAMD_THD_64QAM);
		REALTEK_R_QAM_BASE_DBG("[Qamd] d3 = %d, THR_128 = %d\n", detect_dataZ5, QAMD_THD_128QAM);
		REALTEK_R_QAM_BASE_DBG("[Qamd] d4 = %d, THR_256 = %d\n", ((detect_dataZ2 - detect_dataZ4) + (detect_dataZ3 - detect_dataZ0) + (detect_dataZ3 - detect_dataZ4)), QAMD_THD_256QAM);

		if (((detect_dataZ0 - detect_dataZ4) + (detect_dataZ2 - detect_dataZ3) + (detect_dataZ5 - detect_dataZ1)) > QAMD_THD_16QAM) {
			*pQammode = QAM_QAM_16;
			REALTEK_R_QAM_BASE_INFO("[AUTO QAM] Constellation = 16QAM\n");
		} else if (((detect_dataZ1 - detect_dataZ0) + (detect_dataZ1 - detect_dataZ2) + (detect_dataZ3 - detect_dataZ2)) > QAMD_THD_32QAM) {
			*pQammode = QAM_QAM_32;
			REALTEK_R_QAM_BASE_INFO("[AUTO QAM] Constellation = 32QAM\n");
		} else if (((detect_dataZ0 - detect_dataZ2) + (detect_dataZ0 - detect_dataZ3) + (detect_dataZ0 - detect_dataZ4) + (detect_dataZ4 - detect_dataZ2)) > QAMD_THD_64QAM) {
			*pQammode = QAM_QAM_64;
			REALTEK_R_QAM_BASE_INFO("[AUTO QAM] Constellation = 64QAM\n");
			//} else if (((detect_dataZ3 - detect_dataZ1) + (detect_dataZ4 - detect_dataZ0) + (detect_dataZ4 - detect_dataZ1)) > QAMD_THD_128QAM) {
		} else if (detect_dataZ5 < QAMD_THD_128QAM) {
			*pQammode = QAM_QAM_128;
			REALTEK_R_QAM_BASE_INFO("[AUTO QAM] Constellation = 128QAM\n");
		} else if (((detect_dataZ2 - detect_dataZ4) + (detect_dataZ3 - detect_dataZ0) + (detect_dataZ3 - detect_dataZ4)) > QAMD_THD_256QAM) {
			*pQammode = QAM_QAM_256;
			REALTEK_R_QAM_BASE_INFO("[AUTO QAM] Constellation = 256QAM\n");
		} else {
			*pQammode = QAM_QAM_64;
			REALTEK_R_QAM_BASE_WARNING("[AUTO QAM] QAM mode detect Fail...... (set to dafault 64QAM)  \n");
		}

	} else {
		*pQammode = QAM_QAM_64;
		REALTEK_R_QAM_BASE_WARNING("[AUTO QAM] QAM mode detect Fail...... (set to dafault 64QAM)  \n");

	}

	rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask, DVBC_OPT_RST_STATE0_ON_opt_retry_on(1));
	rtd_maskl(DVBC_FSM_FIX_reg, ~(DVBC_FSM_FIX_opt_fsm_fix_ste_en_mask | DVBC_FSM_FIX_opt_fsm_fix_ste_end_mask)  , (DVBC_FSM_FIX_opt_fsm_fix_ste_en(0) | DVBC_FSM_FIX_opt_fsm_fix_ste_end(0)));

	return FUNCTION_SUCCESS;
}

int
realtek_r_oc_AutoModePreset(
	QAM_DEMOD_MODULE *pDemod
)
{
	//REALTEK_R_QAM_BASE_INFO("Set OC Auto Mode Default 256QAM, Fix ST05\n");
	rtd_maskl(DVBC_OPT_QAM_MODE_reg, ~DVBC_OPT_QAM_MODE_opt_qam_mode_mask, DVBC_OPT_QAM_MODE_opt_qam_mode(0x04));	//0xe102[2:0] = 0x04
	rtd_outl(DVBC_OPT_TR_DECI_RATIO_1_reg,  0xcd);  	//0xe120[7:0]  = 0xcd
	rtd_outl(DVBC_OPT_TR_DECI_RATIO_2_reg, 	0x12);		//0xe121[7:0]  = 0x12
	rtd_outl(DVBC_OPT_DD_EQ_BOUND_reg,		0xd0);		//0xe12e[7:0]  = 0xd0
	rtd_outl(DVBC_OPT_SIGN_DATA_reg,		0x80);		//0xe12f[7:0]  = 0x80
	rtd_outl(DVBC_OPT_SIGN_DATA_1_reg, 		0x43); 		//0xe130[7:0]  = 0x43
	rtd_outl(DVBC_OPT_CIRCLE_reg, 			0x0a);  	//0xe131[7:0]  = 0xa
	rtd_outl(DVBC_FSM_WR_STB_reg, 			0x18); 		//0xe15d[7:0]  = 0x18
	rtd_outl(DVBC_FSM_WORD_reg,				0xc8); 		//0xe15e[31:0] = 0x7f00ccc8
	rtd_outl(DVBC_FSM_WORD_1_reg,			0xcc); 		//0xe15e[31:0] = 0x7f00ccc8
	rtd_outl(DVBC_FSM_WORD_2_reg, 			0x00); 		//0xe15e[31:0] = 0x7f00ccc8
	rtd_outl(DVBC_FSM_WORD_3_reg,			0x7f); 		//0xe15e[31:0] = 0x7f00ccc8
	rtd_outl(DVBC_FSM_WORD_4_reg,			0x28); 		//0xe162[31:0] = 0x205bdc28
	rtd_outl(DVBC_FSM_WORD_5_reg, 			0xdc); 		//0xe162[31:0] = 0x205bdc28
	rtd_outl(DVBC_FSM_WORD_6_reg, 			0x5b); 		//0xe162[31:0] = 0x205bdc28
	rtd_outl(DVBC_FSM_WORD_7_reg,			0x20); 		//0xe162[31:0] = 0x205bdc28
	rtd_outl(DVBC_FSM_WORD_8_reg, 			0xa9); 		//0xe166[23:0] = 0x737ea9
	rtd_outl(DVBC_FSM_WORD_9_reg, 			0x7e); 		//0xe166[23:0] = 0x737ea9
	rtd_outl(DVBC_FSM_WORD_10_reg, 			0x73); 		//0xe166[23:0] = 0x737ea9
	rtd_outl(DVBC_FSM_WORD_11_reg, 			0x00); 		//0xe1cc[7:0]  = 0x0
	rtd_outl(DVBC_FSM_WR_STB_reg, 			0x19); 		//0xe15d[7:0]  = 0x19

	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x62); 		//0xe157[15:0] = 0x362
	rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x03); 		//0xe157[15:0] = 0x362
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x63); 		//0xe157[7:0]  = 0x63
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xc6); 		//0xe157[15:0] = 0x1c6
	rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x01); 		//0xe157[15:0] = 0x1c6
	rtd_outl(DVBC_RXMF_WR_STB_reg,			0xc7); 		//0xe157[7:0]  = 0xc7
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xc8); 		//0xe157[15:0] = 0x1c8
	rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x01); 		//0xe157[15:0] = 0x1c8
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xc9); 		//0xe157[7:0]  = 0xc9
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xca); 		//0xe157[15:0] = 0x7eca
	rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7e); 		//0xe157[15:0] = 0x7eca
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xcb); 		//0xe157[7:0]  = 0xcb
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x4e); 		//0xe157[15:0] = 0x24e
	rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x02); 		//0xe157[15:0] = 0x24e
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x4f); 		//0xe157[7:0]  = 0x4f
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x52); 		//0xe157[15:0] = 0x7b52
	rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7b); 		//0xe157[15:0] = 0x7b52
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x53); 		//0xe157[7:0]  = 0x53
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xd4); 		//0xe157[15:0] = 0x7ed4
	rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7e); 		//0xe157[15:0] = 0x7ed4
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xd5); 		//0xe157[7:0]  = 0xd5
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb6); 		//0xe157[15:0] = 0x6b6
	rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x06); 		//0xe157[15:0] = 0x6b6
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb7); 		//0xe157[7:0]  = 0xb7
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb8); 		//0xe157[15:0] = 0x1b8
	rtd_outl(DVBC_RXMF_WR_STB_1_reg,		0x01); 		//0xe157[15:0] = 0x1b8
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0xb9); 		//0xe157[7:0]  = 0xb9
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x7a); 		//0xe157[15:0] = 0x737a
	rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x73); 		//0xe157[15:0] = 0x737a
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x7b); 		//0xe157[7:0]  = 0x7b
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x3c); 		//0xe157[15:0] = 0x7e3c
	rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x7e); 		//0xe157[15:0] = 0x7e3c
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x3d); 		//0xe157[7:0]  = 0x3d
	rtd_outl(DVBC_RXMF_WR_STB_reg, 			0x7e); 		//0xe157[15:0] = 0x277e
	rtd_outl(DVBC_RXMF_WR_STB_1_reg, 		0x27); 		//0xe157[15:0] = 0x277e
	rtd_outl(DVBC_RXMF_WR_STB_reg,			0x7f); 		//0xe157[7:0]  = 0x7f

	//64QAM SR=5037KHz 256QAM SR=5360KHz
	//TR_DECI_RATIO = (ADC_frequency in Hz) * pow(2, 18) / (symbol rate in Hz) = 25.2 * 262144 / 5.037 = 1311500 =>0x14030c
	//TR_DECI_RATIO = (ADC_frequency in Hz) * pow(2, 18) / (symbol rate in Hz) = 25.2 * 262144 / 5.360537 = 1232344 =>0x12CDD8
	rtd_maskl(DVBC_OPT_TR_DECI_RATIO_reg, ~DVBC_OPT_TR_DECI_RATIO_opt_tr_deci_ratio_7_0_mask, 0xD8);
	rtd_maskl(DVBC_OPT_TR_DECI_RATIO_1_reg, ~DVBC_OPT_TR_DECI_RATIO_1_opt_tr_deci_ratio_15_8_mask , 0xCD);
	rtd_maskl(DVBC_OPT_TR_DECI_RATIO_2_reg, ~DVBC_OPT_TR_DECI_RATIO_2_opt_tr_deci_ratio_23_16_mask , 0x12);

	//fix fsm st05
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x0a);  //FSM state 5
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0x55);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0x55);  //fix at st05
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0xcb);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x1c);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0xf5);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0xcd);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0xb5);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x44);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa4);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0xf8);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x32);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x0b);

#ifdef CONFIG_CUSTOMER_TV002
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x00);   //FSM state 0
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0x4f);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x8c);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0xf2);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0x01);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa0);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0x98);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x18);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x01);
#endif
	pDemod->QamMode = QAM_QAM_256;
	return FUNCTION_SUCCESS;
}

int
realtek_r_oc_AutoModePostset(
	QAM_DEMOD_MODULE *pDemod
)
{
	//REALTEK_R_QAM_BASE_INFO("Restore ST05\n");
	//Restore ST05 FSM
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x0a);  //FSM state 5
	rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0x65);
	rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0x66);
	rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0xcb);
	rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x1c);
	rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0xf5);
	rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0xcd);
	rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0xb5);
	rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x44);
	rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa4);
	rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0xf8);
	rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x32);
	rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
	rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x0b);

	//SR max 5390K = 3504 = 0xdb0
	//rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_reg, ~(DVBC_OPT_POS_FRQ_MAX_SR_opt_pos_frq_max_sr_7_0_mask), DVBC_OPT_POS_FRQ_MAX_SR_opt_pos_frq_max_sr_7_0(0xb0));	//0x23e=0x73, [7:0]opt_pos_frq_max_sr[7:0]=0x73
	//rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_1_reg, ~(DVBC_OPT_POS_FRQ_MAX_SR_1_opt_pos_frq_max_sr_11_8_mask), DVBC_OPT_POS_FRQ_MAX_SR_1_opt_pos_frq_max_sr_11_8(0xd));
	////SR min 5026K = 3267 = 0xcc3
	//rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_reg, ~(DVBC_OPT_POS_FRQ_MIN_SR_opt_pos_frq_min_sr_7_0_mask), DVBC_OPT_POS_FRQ_MIN_SR_opt_pos_frq_min_sr_7_0(0xc3));
	//rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_1_reg, ~(DVBC_OPT_POS_FRQ_MIN_SR_1_opt_pos_frq_min_sr_11_8_mask), DVBC_OPT_POS_FRQ_MIN_SR_1_opt_pos_frq_min_sr_11_8(0xc));

	//disable notch
	rtd_outl(DVBC_SFI_NOTCH1_CTRL0_reg, 0x0);//[1]opt_notch1_en=0
	rtd_outl(DVBC_SFI_NOTCH1_CTRL1_reg, 0x0);//notct1 r[13:6]

	return FUNCTION_SUCCESS;
}

#define OC_AUTUMODE_FSR_CNT 2
int
realtek_r_oc_AutoMode(
	QAM_DEMOD_MODULE *pDemod,
	unsigned char *pNosig,
	U32BITS *pCSR,
	U32BITS *pFSR,
	S32BITS *pCCFO,
	unsigned char *pQamMode
)
{
	unsigned long stime = 0;
	unsigned int srs_nbd_done = 0, srs_state = 0, srs_nbd_done_checked = 0;
	unsigned int fsm_state_o = 0, tr_ld_o = 0;
	unsigned int PosToneMaxIdx[3] = {0}, NegToneMaxIdx[3] = {0}, KeepNewTrRatioRst[3] = {0};
	unsigned int srs_cnt = 0, csr_rst_l = 0, csr_rst_r = 0, csr_KHz = 0, fsr_KHz = 0, fsr_tmp = 0;
	unsigned int ccfo_done = 0, ccfo_max_value = 0, est_ccfo_idx = 0, papr_ccfo_max_value = 0, est_papr_ccfo_idx = 0;
	unsigned int fsr_retry_cnt = 0;
	int est_ccfo = 0, PAPR_CCFO_CFO = 0, CCFO_CFO = 0;
	U32BITS dagc_msb = 0;
#ifdef CONFIG_CUSTOMER_TV002
	unsigned char fix_ste = 0;
#endif
	unsigned int fine_sfi_done = 0, sfi_cancel_wn = 0, sfi_max_val = 0, sfi_max_idx = 0, sfi_slot_avg = 0, cal_comp_en = 0, trk_det_ratio = 0;
	unsigned char sfi_trk_comp_en = 0, sfi_cancel_wn_offset = 0;
	//unsigned int sfi_grp_num = 0, sfi_grp_idx[3] = {0}, sfi_grp_val[3] = {0};
	int SFI_MAX_FREQ = 0;// SFI_IDX1 = 0;

	pDemod->AutoModePreset(pDemod);

	gCorse_sfi_found = 0;//clear sfi grp
	rtd_outl(DVBC_OPT_INITIAL_CR_FREQ_reg, 0x0);//Reset init CR CFO
	rtd_outl(DVBC_OPT_INITIAL_CR_FREQ_1_reg, 0x0);
	//disable notch
	rtd_outl(DVBC_SFI_NOTCH1_CTRL0_reg, 0x0);//[1]opt_notch1_en=0
	rtd_outl(DVBC_SFI_NOTCH1_CTRL1_reg, 0x0);//notct1 r[13:6]
	//restore mu BW
	rtd_outl(DVBC_SFI_CANCEL_CTRL2_reg, 0x53);//mu,mu1
	rtd_outl(DVBC_SFI_CANCEL_CTRL3_reg, 0x37);//mu2
	rtd_maskl(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_reg, ~(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_reg_fsrs_search_range_half_add_7_0_mask), 0x41);	//[7:0] FSR search range half add 0x41=200K 0xA3=500K 0xC4=600K 0xe4=700K  //reg*ADC/8192
	rtd_outl(DVBC_OPT_FFT_OUTPUT_ACC_NUM_reg, 0x00);//restore acc output num
	rtd_maskl(DVBC_OPT_SRS_INPUT_SEL_reg, ~(DVBC_OPT_SRS_INPUT_SEL_opt_srs_input_sel_mask), DVBC_OPT_SRS_INPUT_SEL_opt_srs_input_sel(0));//opt_srs_input_sel = 0x0
	rtd_maskl(DVBC_FFT_ACC_NUM_AUTO_SFI_reg, ~(DVBC_FFT_ACC_NUM_AUTO_SFI_opt_fft_input_bitwise_sel_sfi_mask), DVBC_FFT_ACC_NUM_AUTO_SFI_opt_fft_input_bitwise_sel_sfi(1));//sfi bitwise sel = 0x1
	rtd_maskl(DVBC_FFT_ACC_NUM_AUTO_2_reg, ~DVBC_FFT_ACC_NUM_AUTO_2_opt_fft_output_acc_num_fsrs_mask, 0x1f);//increase FSR acc num to 31
	rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_ccfo_auto_mask, DVBC_OPT_START_SRS_NBD_opt_ccfo_auto(1));//enable ccfo update
	//rtd_maskl(DVBC_FSM_FIX_reg, ~(DVBC_FSM_FIX_opt_fsm_fix_ste_en_mask | DVBC_FSM_FIX_opt_fsm_fix_ste_end_mask)  , (DVBC_FSM_FIX_opt_fsm_fix_ste_en(0) | DVBC_FSM_FIX_opt_fsm_fix_ste_end(0)));
	rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask, DVBC_OPT_RST_STATE0_ON_opt_retry_on(0));//retry off
	rtd_maskl(DVBC_AUTO_SRS_reg, ~(DVBC_AUTO_SRS_opt_auto_srs_qam_start_mask | DVBC_AUTO_SRS_opt_tr_deci_ratio_auto_mask | DVBC_AUTO_SRS_opt_csr_en_mask),
		(DVBC_AUTO_SRS_opt_auto_srs_qam_start(1) | DVBC_AUTO_SRS_opt_tr_deci_ratio_auto(1) | DVBC_AUTO_SRS_opt_csr_en(1)));//[0]Auto srs start [2]Auto set tr_deci_ratio [4]Auto CSR enable
	rtd_maskl(DVBC_FSFI_CTRL3_reg, ~(DVBC_FSFI_CTRL3_opt_sfi_ftrk_en_mask | DVBC_FSFI_CTRL3_opt_sfi_cancel_pset_mask), (DVBC_FSFI_CTRL3_opt_sfi_ftrk_en(1) | DVBC_FSFI_CTRL3_opt_sfi_cancel_pset(1)));//[4]sfi fine trk=1 [7] opt_sfi_cancel_pset=1
	rtd_maskl(DVBC_FSFI_CTRL3_reg, ~(DVBC_FSFI_CTRL3_opt_sfi_trk_comp_en_pset_mask), DVBC_FSFI_CTRL3_opt_sfi_trk_comp_en_pset(0));//[6]opt_sfi_trk_com_en_pset disable

	////SR max 5390K = 3504 = 0xdb0
	//rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_reg, ~(DVBC_OPT_POS_FRQ_MAX_SR_opt_pos_frq_max_sr_7_0_mask), DVBC_OPT_POS_FRQ_MAX_SR_opt_pos_frq_max_sr_7_0(0xb0));	//0x23e=0x73, [7:0]opt_pos_frq_max_sr[7:0]=0x73
	//rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_1_reg, ~(DVBC_OPT_POS_FRQ_MAX_SR_1_opt_pos_frq_max_sr_11_8_mask), DVBC_OPT_POS_FRQ_MAX_SR_1_opt_pos_frq_max_sr_11_8(0xd));
	////SR min 5026K = 3267 = 0xcc3
	//rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_reg, ~(DVBC_OPT_POS_FRQ_MIN_SR_opt_pos_frq_min_sr_7_0_mask), DVBC_OPT_POS_FRQ_MIN_SR_opt_pos_frq_min_sr_7_0(0xc3));
	//rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_1_reg, ~(DVBC_OPT_POS_FRQ_MIN_SR_1_opt_pos_frq_min_sr_11_8_mask), DVBC_OPT_POS_FRQ_MIN_SR_1_opt_pos_frq_min_sr_11_8(0xc));

AutoModeBegin:
	srs_nbd_done_checked = 0;
	*pNosig = 0;
	pDemod->SoftwareReset(pDemod);

	stime = tv_osal_time();
	while ((tv_osal_time() - stime) < 300) {
		srs_nbd_done = DVBC_SRS_NBD_DONE_get_srs_nbd_done(rtd_inl(DVBC_SRS_NBD_DONE_reg));
		ccfo_done = DVBC_SRS_NBD_DONE_get_ccfo_done(rtd_inl(DVBC_SRS_NBD_DONE_reg));
		rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));
		fsm_state_o = (rtd_inl(DVBC_INRX_MO2_reg) & 0x78) >> 3;
		tr_ld_o = DVBC_INRX_MO2_1_get_inrx_mo2_12_7(rtd_inl(DVBC_INRX_MO2_1_reg));
		srs_state = rtd_inl(DVBC_AUTO_SRS_STATE_reg);
		pDemod->GetDiAgc(pDemod, &dagc_msb);
		//REALTEK_R_QAM_BASE_INFO("[AutoMode] dagc_msb=0x"PT_H32BITS" srs_nbd_done=%u, ccfo_done=%u, fsm_state=%u, tr_ld_o=%u, srs_state=0x%x, time="PT_U32BITS"\n", dagc_msb, srs_nbd_done, ccfo_done, fsm_state_o, tr_ld_o, srs_state, tv_osal_time() - stime);

#ifdef CONFIG_CUSTOMER_TV002
		if(((tv_osal_time() - stime) > 40) && (fix_ste == 0))
		{
			REALTEK_R_QAM_BASE_INFO("Release fix ste!!! fsm_state=%u, tr_ld_o=%u, srs_state=0x%x, time="PT_U32BITS"\n", fsm_state_o, tr_ld_o, srs_state, tv_osal_time() - stime);
			rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x00);   //FSM state 0
			rtd_maskl(DVBC_FSM_WORD_reg, 0xffffff00, 0xe0);
			rtd_maskl(DVBC_FSM_WORD_1_reg, 0xffffff00, 0xee);
			rtd_maskl(DVBC_FSM_WORD_2_reg, 0xffffff00, 0x4f);
			rtd_maskl(DVBC_FSM_WORD_3_reg, 0xffffff00, 0x8c);
			rtd_maskl(DVBC_FSM_WORD_4_reg, 0xffffff00, 0xf2);
			rtd_maskl(DVBC_FSM_WORD_5_reg, 0xffffff00, 0x01);
			rtd_maskl(DVBC_FSM_WORD_6_reg, 0xffffff00, 0x00);
			rtd_maskl(DVBC_FSM_WORD_7_reg, 0xffffff00, 0x00);
			rtd_maskl(DVBC_FSM_WORD_8_reg, 0xffffff00, 0xa0);
			rtd_maskl(DVBC_FSM_WORD_9_reg, 0xffffff00, 0x98);
			rtd_maskl(DVBC_FSM_WORD_10_reg, 0xffffff00, 0x18);
			rtd_maskl(DVBC_FSM_WORD_11_reg, 0xffffff00, 0x00);
			rtd_maskl(DVBC_FSM_WR_STB_reg, 0xffffff00, 0x01);
			fix_ste = 1;
		}
#endif
		if (srs_nbd_done == 0x1 && srs_nbd_done_checked == 0x0) {
			REALTEK_R_QAM_BASE_INFO("[AutoMode] srs_nbd_done=%u, ccfo_done=%u, fsm_state=%u, tr_ld_o=%u, srs_state=0x%x, time="PT_U32BITS"\n", srs_nbd_done, ccfo_done, fsm_state_o, tr_ld_o, srs_state, tv_osal_time() - stime);
			srs_nbd_done_checked = 1;

			//Get CSR
			srs_cnt = DVBC_CSRS_RESULT_get_cp_cnt(rtd_inl(DVBC_CSRS_RESULT_reg)); //[2:0] number of search
			//CSR_KHz = (cp_r -cp_l) /8192 *ADC_clk_KHz
			csr_rst_r = ((DVBC_CSRS_RESULT0_1_get_cp_r_0_12_8(rtd_inl(DVBC_CSRS_RESULT0_1_reg)) << 8) | DVBC_CSRS_RESULT0_0_get_cp_r_0_7_0(rtd_inl(DVBC_CSRS_RESULT0_0_reg)));
			csr_rst_l = ((DVBC_CSRS_RESULT0_3_get_cp_l_0_12_8(rtd_inl(DVBC_CSRS_RESULT0_3_reg)) << 8) | DVBC_CSRS_RESULT0_2_get_cp_l_0_7_0(rtd_inl(DVBC_CSRS_RESULT0_2_reg)));
			csr_KHz = ((csr_rst_r - csr_rst_l) * OC_SAMPLE_RATE_KHZ) / FFT_SIZE ;
			REALTEK_R_QAM_BASE_INFO("[CSR] srs_cnt=%u, csr_KHz=%u [csr_l=%u,csr_r=%u]\n", srs_cnt, csr_KHz, csr_rst_l, csr_rst_r);
			*pCSR = csr_KHz;

			//Get FSR
			PosToneMaxIdx[0] = ((DVBC_POS_TONE_MAX_IDX_0_1_FSRS_get_pos_tone_max_idx_0_fsrs_11_8(rtd_inl(DVBC_POS_TONE_MAX_IDX_0_1_FSRS_reg)) << 8) | DVBC_POS_TONE_MAX_IDX_0_FSRS_get_pos_tone_max_idx_0_fsrs_7_0(rtd_inl(DVBC_POS_TONE_MAX_IDX_0_FSRS_reg)));
			PosToneMaxIdx[1] = ((DVBC_POS_TONE_MAX_IDX_1_1_FSRS_get_pos_tone_max_idx_1_fsrs_11_8(rtd_inl(DVBC_POS_TONE_MAX_IDX_1_1_FSRS_reg)) << 8) | DVBC_POS_TONE_MAX_IDX_1_FSRS_get_pos_tone_max_idx_1_fsrs_7_0(rtd_inl(DVBC_POS_TONE_MAX_IDX_1_FSRS_reg)));
			PosToneMaxIdx[2] = ((DVBC_POS_TONE_MAX_IDX_2_1_FSRS_get_pos_tone_max_idx_2_fsrs_11_8(rtd_inl(DVBC_POS_TONE_MAX_IDX_2_1_FSRS_reg)) << 8) | DVBC_POS_TONE_MAX_IDX_2_FSRS_get_pos_tone_max_idx_2_fsrs_7_0(rtd_inl(DVBC_POS_TONE_MAX_IDX_2_FSRS_reg)));
			NegToneMaxIdx[0] = ((DVBC_NEG_TONE_MAX_IDX_0_1_FSRS_get_neg_tone_max_idx_0_fsrs_11_8(rtd_inl(DVBC_NEG_TONE_MAX_IDX_0_1_FSRS_reg)) << 8) | DVBC_NEG_TONE_MAX_IDX_0_FSRS_get_neg_tone_max_idx_0_fsrs_7_0(rtd_inl(DVBC_NEG_TONE_MAX_IDX_0_FSRS_reg)));
			NegToneMaxIdx[1] = ((DVBC_NEG_TONE_MAX_IDX_1_1_FSRS_get_neg_tone_max_idx_1_fsrs_11_8(rtd_inl(DVBC_NEG_TONE_MAX_IDX_1_1_FSRS_reg)) << 8) | DVBC_NEG_TONE_MAX_IDX_1_FSRS_get_neg_tone_max_idx_1_fsrs_7_0(rtd_inl(DVBC_NEG_TONE_MAX_IDX_1_FSRS_reg)));
			NegToneMaxIdx[2] = ((DVBC_NEG_TONE_MAX_IDX_2_1_FSRS_get_neg_tone_max_idx_2_fsrs_11_8(rtd_inl(DVBC_NEG_TONE_MAX_IDX_2_1_FSRS_reg)) << 8) | DVBC_NEG_TONE_MAX_IDX_2_FSRS_get_neg_tone_max_idx_2_fsrs_7_0(rtd_inl(DVBC_NEG_TONE_MAX_IDX_2_FSRS_reg)));
			KeepNewTrRatioRst[0] = DVBC_TR_DECI_RATIO_SRS_0_get_tr_deci_ratio_fsrs_7_0(rtd_inl(DVBC_TR_DECI_RATIO_SRS_0_reg));
			KeepNewTrRatioRst[1] = DVBC_TR_DECI_RATIO_SRS_1_get_tr_deci_ratio_fsrs_15_8(rtd_inl(DVBC_TR_DECI_RATIO_SRS_1_reg));
			KeepNewTrRatioRst[2] = DVBC_TR_DECI_RATIO_SRS_2_get_tr_deci_ratio_fsrs_23_16(rtd_inl(DVBC_TR_DECI_RATIO_SRS_2_reg));
			REALTEK_R_QAM_BASE_INFO("New TR deci 0x%x%02x%02x, POS %u, %u, %u, NEG %u, %u, %u\n", KeepNewTrRatioRst[2], KeepNewTrRatioRst[1], KeepNewTrRatioRst[0], PosToneMaxIdx[0], PosToneMaxIdx[1], PosToneMaxIdx[2], NegToneMaxIdx[0], NegToneMaxIdx[1], NegToneMaxIdx[2]);

			if ((KeepNewTrRatioRst[0] == 0xff) && (KeepNewTrRatioRst[1] == 0xff) && (KeepNewTrRatioRst[2] == 0xff)) {
				REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "FSR all 0xff\n" "\033[m");
				*pNosig = 1;
				goto AutoModeEnd;
			} else {
				//symbol_rate_det = (ADC_freq/2) * (pos_frq_tone_idx / 8192);	//fft_size :8192
				fsr_tmp = ((KeepNewTrRatioRst[2] << 16) | (KeepNewTrRatioRst[1] << 8) | KeepNewTrRatioRst[0]) / 10;
				fsr_KHz = (OC_SAMPLE_RATE_10KHZ * 262144 / fsr_tmp);
				*pFSR = fsr_KHz * 1000;
				REALTEK_R_QAM_BASE_INFO("[FSR] fsr_tmp=%u, FSR=%u KHz\n", fsr_tmp, fsr_KHz);

				if((fsr_KHz > csr_KHz) && (fsr_retry_cnt < OC_AUTUMODE_FSR_CNT))
				{
					fsr_retry_cnt++;
					rtd_maskl(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_reg, ~(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_reg_fsrs_search_range_half_add_7_0_mask), DVBC_OPT_FSRS_SEARCH_RANGE_HALF_reg_fsrs_search_range_half_add_7_0(0x10)); //[7:0] FSR search range half add 0x10=50K 0x41=200K //reg*ADC/8192
					REALTEK_R_QAM_BASE_INFO("\033[1;32;36m" "FSR > CSR, lower add range retry %u times\n" "\033[m", fsr_retry_cnt);
					goto AutoModeBegin;
				}
				if(((fsr_KHz > 5060) && (fsr_KHz < 5355)) || (fsr_KHz > 5364) || (fsr_KHz < 5026))
				{
					if(fsr_retry_cnt < OC_AUTUMODE_FSR_CNT)
					{
						fsr_retry_cnt++;
						rtd_maskl(DVBC_SFI_NOTCH1_CTRL0_reg, ~(DVBC_SFI_NOTCH1_CTRL0_opt_notch1_r_5_0_mask), DVBC_SFI_NOTCH1_CTRL0_opt_notch1_r_5_0(0x64));	//notch1 r [5:0]
						rtd_outl(DVBC_SFI_NOTCH1_CTRL1_reg, 0xff);//notct1 r[13:6] 0x3fd9
						rtd_maskl(DVBC_SFI_NOTCH1_CTRL0_reg, ~(DVBC_SFI_NOTCH1_CTRL0_opt_notch1_en_mask),DVBC_SFI_NOTCH1_CTRL0_opt_notch1_en(1));//[0]opt_notch1_en=1
						REALTEK_R_QAM_BASE_INFO("\033[1;32;36m" "FSR not found, enable notch1 retry %u times\n" "\033[m", fsr_retry_cnt);
						goto AutoModeBegin;
					}
					else
					{
						*pNosig = 1;
						goto AutoModeEnd;
					}
				}
			}
#if 0
			//Get 1st SFI detect
			sfi_grp_num = DVBC_SFI_DET_STATUS1_get_sfi_det_grp_num(rtd_inl(DVBC_SFI_DET_STATUS1_reg));
			sfi_grp_idx[0] = ((DVBC_SFI_DET_STATUS1_get_sfi_det_grp_idx_0_13_8(rtd_inl(DVBC_SFI_DET_STATUS1_reg)) << 8) | DVBC_SFI_DET_STATUS0_get_sfi_det_grp_idx_0_7_0(rtd_inl(DVBC_SFI_DET_STATUS0_reg)));
			sfi_grp_idx[1] = ((DVBC_SFI_DET_STATUS3_get_sfi_det_grp_idx_1_13_8(rtd_inl(DVBC_SFI_DET_STATUS3_reg)) << 8) | DVBC_SFI_DET_STATUS2_get_sfi_det_grp_idx_1_7_0(rtd_inl(DVBC_SFI_DET_STATUS2_reg)));
			sfi_grp_idx[2] = ((DVBC_SFI_DET_STATUS5_get_sfi_det_grp_idx_2_13_8(rtd_inl(DVBC_SFI_DET_STATUS5_reg)) << 8) | DVBC_SFI_DET_STATUS4_get_sfi_det_grp_idx_2_7_0(rtd_inl(DVBC_SFI_DET_STATUS4_reg)));
			sfi_grp_val[0] = ((DVBC_SFI_DET_STATUS7_get_sfi_det_grp_val_0_15_8(rtd_inl(DVBC_SFI_DET_STATUS7_reg)) << 8) | DVBC_SFI_DET_STATUS6_get_sfi_det_grp_val_0_7_0(rtd_inl(DVBC_SFI_DET_STATUS6_reg)));
			sfi_grp_val[1] = ((DVBC_SFI_DET_STATUS9_get_sfi_det_grp_val_1_15_8(rtd_inl(DVBC_SFI_DET_STATUS9_reg)) << 8) | DVBC_SFI_DET_STATUS8_get_sfi_det_grp_val_1_7_0(rtd_inl(DVBC_SFI_DET_STATUS8_reg)));
			sfi_grp_val[2] = ((DVBC_SFI_DET_STATUS11_get_sfi_det_grp_val_2_15_8(rtd_inl(DVBC_SFI_DET_STATUS11_reg)) << 8) | DVBC_SFI_DET_STATUS10_get_sfi_det_grp_val_2_7_0(rtd_inl(DVBC_SFI_DET_STATUS10_reg)));
			SFI_IDX1 = (sfi_grp_idx[0] < 4096) ? ((int)sfi_grp_idx[0]*12600/8192) : (((int)sfi_grp_idx[0] - 8192)*12600/8192);
			REALTEK_R_QAM_BASE_INFO("[SFI Det] sfi num=%u SFI0 at %d KHz\n", sfi_grp_num, SFI_IDX1);
			//REALTEK_R_QAM_BASE_INFO("[grp0] value0=%u, idx0=0x%x\n", sfi_grp_val[0], sfi_grp_idx[0]);
			//REALTEK_R_QAM_BASE_INFO("[grp1] value1=%u, idx1=0x%x\n", sfi_grp_val[1], sfi_grp_idx[1]);
			//REALTEK_R_QAM_BASE_INFO("[grp2] value2=%u, idx2=0x%x\n", sfi_grp_val[2], sfi_grp_idx[2]);
#endif
		}
		else if(srs_nbd_done == 0x1 && ccfo_done == 0x1)
		{
			//Get CCFO
			REALTEK_R_QAM_BASE_INFO("[AutoMode] srs_nbd_done=%u, ccfo_done=%u, fsm_state=%u, tr_ld_o=%u, srs_state=0x%x, time="PT_U32BITS"\n", srs_nbd_done, ccfo_done, fsm_state_o, tr_ld_o, srs_state, tv_osal_time() - stime);
			ccfo_max_value = (rtd_inl(DVBC_FFTOUT_MAX0_reg) | (rtd_inl(DVBC_FFTOUT_MAX1_reg) << 8));
			est_ccfo_idx = (DVBC_FFTOUT_MAX1_get_ccfo_local_max_val_15_8(rtd_inl(DVBC_FFTOUT_MAX3_reg)) << 8)  | (DVBC_FFTOUT_MAX2_get_ccfo_local_max_idx_7_0(rtd_inl(DVBC_FFTOUT_MAX2_reg)));
			CCFO_CFO = (est_ccfo_idx < 4096) ? ((int)est_ccfo_idx*6300/8192) : (((int)est_ccfo_idx - 8192)*6300/8192);
			est_ccfo = CCFO_CFO * 1000;
			//New PAPR CCFO
			papr_ccfo_max_value = (rtd_inl(DVBC_CCFO_PAPR_MAX0_reg) | (rtd_inl(DVBC_CCFO_PAPR_MAX1_reg) << 8));
			est_papr_ccfo_idx = ((rtd_inl(DVBC_CCFO_PAPR_IDX1_reg) & 0x1f) << 8) + (rtd_inl(DVBC_CCFO_PAPR_IDX0_reg) & 0xff);
			gCcfo_idx_12_8 = rtd_inl(DVBC_CCFO_PAPR_IDX1_reg) & 0x1f;
			gCcfo_idx_7_0 = rtd_inl(DVBC_CCFO_PAPR_IDX0_reg) & 0xff;
			PAPR_CCFO_CFO = (est_papr_ccfo_idx < 4096) ? ((int)est_papr_ccfo_idx*6300/8192) : (((int)est_papr_ccfo_idx - 8192)*6300/8192);
			REALTEK_R_QAM_BASE_INFO("ORI ccfo_max_value=%u est_ccfo_idx=%u CCFO_CFO=%d KHz\n", ccfo_max_value, est_ccfo_idx, CCFO_CFO);
			REALTEK_R_QAM_BASE_INFO("NEW papr_ccfo_max_value=%u est_papr_ccfo_idx=%u PAPR_CCFO_CFO=%d KHz\n", papr_ccfo_max_value, est_papr_ccfo_idx, PAPR_CCFO_CFO);
			*pCCFO = PAPR_CCFO_CFO * 1000;
			break;
		}
		tv_osal_msleep(5);
	}

	//SFI detect
	fine_sfi_done = 0;
	stime = tv_osal_time();
	while ((tv_osal_time() - stime) < 100)
	{
		fine_sfi_done = DVBC_SFI_CANCEL_STATUS11_get_fine_sfi_done(rtd_inl(DVBC_SFI_CANCEL_STATUS11_reg));

		if(fine_sfi_done ==1)
		{
			sfi_max_val = ((rtd_inl(DVBC_SFI_CANCEL_STATUS5_reg) <<8) | rtd_inl(DVBC_SFI_CANCEL_STATUS4_reg));
			sfi_slot_avg = ((rtd_inl(DVBC_SFI_CANCEL_STATUS3_reg) <<8) | rtd_inl(DVBC_SFI_CANCEL_STATUS2_reg));
			trk_det_ratio = DVBC_FSFI_CTRL0_get_opt_sfi_trk_det_ratio(rtd_inl(DVBC_FSFI_CTRL0_reg));
			cal_comp_en = (sfi_slot_avg * trk_det_ratio)/64;
			sfi_trk_comp_en = DVBC_SFI_CANCEL_CTRL7_get_sfi_trk_comp_en(rtd_inl(DVBC_SFI_CANCEL_CTRL7_reg));
			sfi_cancel_wn_offset = DVBC_SFI_CANCEL_CTRL7_get_sfi_cancel_wn_offset(rtd_inl(DVBC_SFI_CANCEL_CTRL7_reg));
			gCorse_sfi_found = (sfi_max_val > cal_comp_en) ? 1:0;
			gSfi_cancel_wn_7_0 = DVBC_SFI_CANCEL_STATUS0_get_sfi_cancel_wn_7_0(rtd_inl(DVBC_SFI_CANCEL_STATUS0_reg));
			gSfi_cancel_wn_15_8 = DVBC_SFI_CANCEL_STATUS1_get_sfi_cancel_wn_15_8(rtd_inl(DVBC_SFI_CANCEL_STATUS1_reg));
			sfi_max_idx = ((DVBC_SFI_CANCEL_STATUS7_get_sfi_max_idx_12_8(rtd_inl(DVBC_SFI_CANCEL_STATUS7_reg)) << 8) | DVBC_SFI_CANCEL_STATUS6_get_sfi_max_idx_7_0(rtd_inl(DVBC_SFI_CANCEL_STATUS6_reg)));
			sfi_cancel_wn = ((gSfi_cancel_wn_15_8 <<8) | gSfi_cancel_wn_7_0);
			SFI_MAX_FREQ = (sfi_max_idx < 4096) ? (((int)sfi_max_idx*fsr_KHz)/8192) : -1*((8192 - sfi_max_idx)*fsr_KHz/8192);
			REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "[SFI] trk_det_ratio=%u cal_comp_en=%u sfi_max_val=%u sfi_slot_avg=%u\n" "\033[m", trk_det_ratio, cal_comp_en, sfi_max_val, sfi_slot_avg);
			REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "[SFI] fine_sfi_done=%u sfi_cancel_wn=0x%x sfi_max_idx=%u SFI Freq=%dKHz\n" "\033[m", fine_sfi_done,sfi_cancel_wn, sfi_max_idx, SFI_MAX_FREQ);
			//REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "[SFI] gSfi_cancel_wn_15_8=0x%x gSfi_cancel_wn_7_0=0x%x sfi_cancel_wn_offset=%u\n" "\033[m", gSfi_cancel_wn_15_8, gSfi_cancel_wn_7_0, sfi_cancel_wn_offset);
			break;
		}
	}
	if(fine_sfi_done == 0)
	{
		REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "Fine SFI detect not done!!!\n" "\033[m");
	}

	if(srs_nbd_done == 0x0 || ccfo_done == 0x0)
	{
		REALTEK_R_QAM_BASE_INFO("[AutoMode] srs_nbd_done=%u, ccfo_done=%u, fsm_state=%u, tr_ld_o=%u, srs_state=0x%x, time="PT_U32BITS"\n", srs_nbd_done, ccfo_done, fsm_state_o, tr_ld_o, srs_state, tv_osal_time() - stime);
		REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "(srs_nbd_done == 0x0 || ccfo_done == 0x0)\n" "\033[m");
		*pNosig = 1;
		goto AutoModeEnd;
	}

AutoModeEnd:

	if (*pNosig == 1)
		return FUNCTION_SUCCESS;


	if (*pFSR >= 5026941 && *pFSR <= 5086941) {	//+-0.03MHz Margin for 64/256QAM SR
		//REALTEK_R_QAM_BASE_INFO("Check FSR to 64QAM\n");
		*pQamMode =  QAM_QAM_64;
	} else if (*pFSR >= 5330537 && *pFSR <= 5390537) {
		//REALTEK_R_QAM_BASE_INFO("Check FSR to 256QAM\n");
		*pQamMode =  QAM_QAM_256;
	} else {
		REALTEK_R_QAM_BASE_INFO("Double Check SymbolRateDet as No Signal\n");
		*pNosig = 1;
	}

//auto mode no_signal_fsrs issue at Me5. no CCFO & qam detect
	/*

		stime = tv_osal_time();
		while ((tv_osal_time() - stime) < 300) {

			srs_nbd_done = DVBC_SRS_NBD_DONE_get_srs_nbd_done(rtd_inl(DVBC_SRS_NBD_DONE_reg));
			ccfo_done = DVBC_SRS_NBD_DONE_get_ccfo_done(rtd_inl(DVBC_SRS_NBD_DONE_reg));
			rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));
			fsm_state_o = (rtd_inl(DVBC_INRX_MO2_reg) & 0x78) >> 3;
			tr_ld_o = DVBC_INRX_MO2_1_get_inrx_mo2_12_7(rtd_inl(DVBC_INRX_MO2_1_reg));
			srs_state = rtd_inl(DVBC_AUTO_SRS_STATE_reg);

	              REALTEK_R_QAM_BASE_INFO("[AutoMode - wait ccfo done] srs_nbd_done=%u, ccfo_done=%u, fsm_state=%u, tr_ld_o=%u, srs_state=0x%x, time="PT_U32BITS"\n", srs_nbd_done, ccfo_done, fsm_state_o, tr_ld_o, srs_state, tv_osal_time() - stime);
			if (ccfo_done == 0x1) {
				REALTEK_R_QAM_BASE_INFO("[AutoMode - wait ccfo done] srs_nbd_done=%u, ccfo_done=%u, fsm_state=%u, tr_ld_o=%u, srs_state=0x%x, time="PT_U32BITS"\n", srs_nbd_done, ccfo_done, fsm_state_o, tr_ld_o, srs_state, tv_osal_time() - stime);
				break;
			}

		}

	*/

	pDemod->AutoModePostset(pDemod);
	return FUNCTION_SUCCESS;
}

int
realtek_r_oc_SetSfiCancel(
	QAM_DEMOD_MODULE *pDemod
)
{
	unsigned int sfi_max_val = 0, sfi_slot_avg = 0, trk_det_ratio = 0, cal_comp_en = 0, sfi_trk_comp_en = 0;
	
	//opt_sfi_cancel_trk_prd * 2^14 /(symbol rate *2)
	rtd_outl(DVBC_SFI_CANCEL_CTRL5_reg, 0x80); //opt_sfi_trk_period[7:0]

	sfi_max_val = ((DVBC_SFI_CANCEL_STATUS5_get_sfi_max_val_15_8(rtd_inl(DVBC_SFI_CANCEL_STATUS5_reg)) << 8) | DVBC_SFI_CANCEL_STATUS4_get_sfi_max_val_7_0(rtd_inl(DVBC_SFI_CANCEL_STATUS4_reg)));
	sfi_slot_avg = ((DVBC_SFI_CANCEL_STATUS3_get_sfi_slot_avg_15_8(rtd_inl(DVBC_SFI_CANCEL_STATUS3_reg)) << 8) | DVBC_SFI_CANCEL_STATUS2_get_sfi_slot_avg_7_0(rtd_inl(DVBC_SFI_CANCEL_STATUS2_reg)));
	trk_det_ratio = DVBC_FSFI_CTRL0_get_opt_sfi_trk_det_ratio(rtd_inl(DVBC_FSFI_CTRL0_reg));
	cal_comp_en = (sfi_slot_avg * trk_det_ratio)/64;
	sfi_trk_comp_en = DVBC_SFI_CANCEL_CTRL7_get_sfi_trk_comp_en(rtd_inl(DVBC_SFI_CANCEL_CTRL7_reg));
	//REALTEK_R_QAM_BASE_INFO("[SetSfiCancel] sfi_max_val=%u cal_comp_en=%u sfi_slot_avg=%u cal_comp_en=%u", sfi_max_val, cal_comp_en, sfi_slot_avg, cal_comp_en);

	if((gCorse_sfi_found ==1) && (DVBC_FSFI_CTRL3_get_opt_sfi_trk_comp_en_pset(rtd_inl(DVBC_FSFI_CTRL3_reg))== 0))
	{
		REALTEK_R_QAM_BASE_INFO("\033[1;32;36m" "SFI preset enable !!! \n" "\033[m");
		rtd_maskl(DVBC_SFI_CANCEL_CTRL2_reg, ~(DVBC_SFI_CANCEL_CTRL2_opt_sfi_cancel_val_mu1_mask), DVBC_SFI_CANCEL_CTRL2_opt_sfi_cancel_val_mu1(0x7));//mu1_val
		rtd_maskl(DVBC_OPT_SRS_INPUT_SEL_reg, ~(DVBC_OPT_SRS_INPUT_SEL_opt_srs_input_sel_mask), DVBC_OPT_SRS_INPUT_SEL_opt_srs_input_sel(0x03));//opt_srs_input_sel = 0x3
		rtd_outl(DVBC_OPT_FFT_OUTPUT_ACC_NUM_reg, 0x24);//opt_fft_output_acc_num = opt_fft_output_acc_num_sfi
		rtd_outl(DVBC_FSFI_CTRL5_reg, gSfi_cancel_wn_15_8);
		rtd_outl(DVBC_FSFI_CTRL4_reg, gSfi_cancel_wn_7_0);
		rtd_maskl(DVBC_FSFI_CTRL3_reg, ~(DVBC_FSFI_CTRL3_opt_sfi_trk_comp_en_pset_mask), DVBC_FSFI_CTRL3_opt_sfi_trk_comp_en_pset(1));//[6]opt_sfi_trk_com_en_pset enable
	}

	if((gCorse_sfi_found ==0) && (DVBC_FSFI_CTRL3_get_opt_sfi_trk_comp_en_pset(rtd_inl(DVBC_FSFI_CTRL3_reg))== 1))
	{
		REALTEK_R_QAM_BASE_INFO("\033[1;32;36m" "SFI preset disable !!! \n" "\033[m");
		rtd_maskl(DVBC_FSFI_CTRL3_reg, ~(DVBC_FSFI_CTRL3_opt_sfi_trk_comp_en_pset_mask), DVBC_FSFI_CTRL3_opt_sfi_trk_comp_en_pset(0));//[6]opt_sfi_trk_com_en_pset disable
	}

	return FUNCTION_SUCCESS;
}

int
realtek_r_oc_SfiFullTracking(
	QAM_DEMOD_MODULE *pDemod
)
{
	unsigned long trk_time = 0;
	unsigned int sfi_cancel_wn_15_8_trk = 0, sfi_cancel_wn_7_0_trk = 0;
	unsigned int sfi_max_val = 0, sfi_slot_avg = 0, trk_det_ratio = 0, cal_comp_en = 0, SfiWaitTime = 100;
	unsigned char aagc_ld = 0, fsm_state_o = 0, tr_ld_o = 0;

	sfi_max_val = ((DVBC_SFI_CANCEL_STATUS5_get_sfi_max_val_15_8(rtd_inl(DVBC_SFI_CANCEL_STATUS5_reg)) << 8) | DVBC_SFI_CANCEL_STATUS4_get_sfi_max_val_7_0(rtd_inl(DVBC_SFI_CANCEL_STATUS4_reg)));
	sfi_slot_avg = ((DVBC_SFI_CANCEL_STATUS3_get_sfi_slot_avg_15_8(rtd_inl(DVBC_SFI_CANCEL_STATUS3_reg)) << 8) | DVBC_SFI_CANCEL_STATUS2_get_sfi_slot_avg_7_0(rtd_inl(DVBC_SFI_CANCEL_STATUS2_reg)));
	trk_det_ratio = DVBC_FSFI_CTRL0_get_opt_sfi_trk_det_ratio(rtd_inl(DVBC_FSFI_CTRL0_reg));
	cal_comp_en = (sfi_slot_avg * trk_det_ratio)/64;
	sfi_cancel_wn_7_0_trk = DVBC_SFI_CANCEL_STATUS0_get_sfi_cancel_wn_7_0(rtd_inl(DVBC_SFI_CANCEL_STATUS0_reg));
	sfi_cancel_wn_15_8_trk = DVBC_SFI_CANCEL_STATUS1_get_sfi_cancel_wn_15_8(rtd_inl(DVBC_SFI_CANCEL_STATUS1_reg));
	//REALTEK_R_QAM_BASE_INFO("[Autotune] sfi_max_val=%u cal_comp_en=%u sfi_slot_avg=%u", sfi_max_val, cal_comp_en, sfi_slot_avg);

	if((DVBC_FSFI_CTRL3_get_opt_sfi_trk_comp_en_pset(rtd_inl(DVBC_FSFI_CTRL3_reg))== 1) && sfi_max_val != 0)
	{
		if((sfi_cancel_wn_15_8_trk != gSfi_cancel_wn_15_8) && (sfi_cancel_wn_7_0_trk != gSfi_cancel_wn_7_0))
		{
			REALTEK_R_QAM_BASE_INFO("\033[1;32;36m" "[SFI trk] cur sfi_cancel_wn_15_8_trk=0x%x sfi_cancel_wn_7_0_trk=0x%x" "\033[m", sfi_cancel_wn_15_8_trk, sfi_cancel_wn_7_0_trk);
			REALTEK_R_QAM_BASE_INFO("\033[1;32;36m" "[SFI trk] gSfi_cancel_wn_15_8=0x%x sfi_cancel_wn_7_0_trk=0x%x" "\033[m",gSfi_cancel_wn_15_8, gSfi_cancel_wn_7_0);
			REALTEK_R_QAM_BASE_INFO("\033[1;32;36m" "SFI Freq Changed!!!!!!!!\n" "\033[m");

			rtd_maskl(DVBC_FSM_FIX_reg, ~(DVBC_FSM_FIX_opt_fsm_fix_ste_en_mask | DVBC_FSM_FIX_opt_fsm_fix_ste_end_mask)  , (DVBC_FSM_FIX_opt_fsm_fix_ste_en(1) | DVBC_FSM_FIX_opt_fsm_fix_ste_end(0)));
			rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask, DVBC_OPT_RST_STATE0_ON_opt_retry_on(0));//retry off
			rtd_maskl(DVBC_FSFI_CTRL3_reg, ~(DVBC_FSFI_CTRL3_opt_sfi_trk_comp_en_pset_mask), DVBC_FSFI_CTRL3_opt_sfi_trk_comp_en_pset(0));//[6]opt_sfi_trk_com_en_pset OFF
			//restore mu BW
			rtd_maskl(DVBC_SFI_CANCEL_CTRL2_reg, ~(DVBC_SFI_CANCEL_CTRL2_opt_sfi_cancel_val_mu_mask), DVBC_SFI_CANCEL_CTRL2_opt_sfi_cancel_val_mu(0x3));

			pDemod->SoftwareReset(pDemod);
			trk_time = tv_osal_time();
			while((tv_osal_time() - trk_time) < 160)
			{
				rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));//stb INRX_MO
				fsm_state_o = (rtd_inl(DVBC_INRX_MO2_reg) & 0x78) >> 3;
				tr_ld_o = DVBC_INRX_MO2_1_get_inrx_mo2_12_7(rtd_inl(DVBC_INRX_MO2_1_reg));
				aagc_ld = rtd_inl(DVBC_INRX_MO2_reg) & 0x1;
				//REALTEK_R_QAM_BASE_INFO("[Autotune] sfi_max_val=%u sfi_slot_avg=%u fsm_state_o=%u tr_ld_o=%u", sfi_max_val, sfi_slot_avg, fsm_state_o, tr_ld_o);

				if(aagc_ld == 1){
					//pDemod->SetSfiCancel(pDemod);//sfi cancel
					gSfi_cancel_wn_15_8 = sfi_cancel_wn_15_8_trk;
					gSfi_cancel_wn_7_0 = sfi_cancel_wn_7_0_trk;
					rtd_outl(DVBC_FSFI_CTRL5_reg, gSfi_cancel_wn_15_8);
					rtd_outl(DVBC_FSFI_CTRL4_reg, gSfi_cancel_wn_7_0);
					rtd_maskl(DVBC_FSFI_CTRL3_reg, ~(DVBC_FSFI_CTRL3_opt_sfi_trk_comp_en_pset_mask), DVBC_FSFI_CTRL3_opt_sfi_trk_comp_en_pset(1));//[6]opt_sfi_trk_com_en_pset ON
				}

				if(((tv_osal_time() - trk_time) > SfiWaitTime) && (DVBC_SFI_CANCEL_CTRL2_get_opt_sfi_cancel_val_mu(rtd_inl(DVBC_SFI_CANCEL_CTRL2_reg)) == 0x3))
				{
					rtd_maskl(DVBC_SFI_CANCEL_CTRL2_reg, ~(DVBC_SFI_CANCEL_CTRL2_opt_sfi_cancel_val_mu_mask), DVBC_SFI_CANCEL_CTRL2_opt_sfi_cancel_val_mu(0x5));
				}

				if(((tv_osal_time() - trk_time) > (SfiWaitTime + 30)) && (DVBC_SFI_CANCEL_CTRL2_get_opt_sfi_cancel_val_mu(rtd_inl(DVBC_SFI_CANCEL_CTRL2_reg)) == 0x5))
				{
					rtd_maskl(DVBC_SFI_CANCEL_CTRL2_reg, ~(DVBC_SFI_CANCEL_CTRL2_opt_sfi_cancel_val_mu_mask), DVBC_SFI_CANCEL_CTRL2_opt_sfi_cancel_val_mu(0x7));
				}

				if((tv_osal_time() - trk_time) > (SfiWaitTime + 50))
				{
					rtd_maskl(DVBC_FSM_FIX_reg, ~(DVBC_FSM_FIX_opt_fsm_fix_ste_en_mask | DVBC_FSM_FIX_opt_fsm_fix_ste_end_mask)  , (DVBC_FSM_FIX_opt_fsm_fix_ste_en(0) | DVBC_FSM_FIX_opt_fsm_fix_ste_end(0)));
					rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask, DVBC_OPT_RST_STATE0_ON_opt_retry_on(1));//retry on
					break;
				}
			}
		}
	}

	if((sfi_max_val < cal_comp_en) && (sfi_max_val != 0))
		gCorse_sfi_found = 0;
	if((sfi_max_val >= cal_comp_en) && (sfi_max_val != 0))
		gCorse_sfi_found = 1;

	pDemod->SetSfiCancel(pDemod);//sfi cancel

	return FUNCTION_SUCCESS;
}


/**

@see   QAM_DEMOD_FP_INITIALIZE

*/
int
realtek_r_dvbc_Initialize(
	QAM_DEMOD_MODULE *pDemod
)
{
	REALTEK_R_QAM_BASE_INFO("realtek_r_dvbc_Initialize\n");
	//-------RTK DVBC Merlin5 Init Table Start-------------//
	rtd_maskl(RTK_DTV_DEMOD_SYS_DEMOD_SEL_reg, ~(RTK_DTV_DEMOD_SYS_DEMOD_SEL_demod_sel_mask), RTK_DTV_DEMOD_SYS_DEMOD_SEL_demod_sel(0x1));	//0xe401=0x1 demod select to DVBC[1:0] = 0x1
	rtd_maskl(DVBC_OPT_QAM_STD_reg, ~(DVBC_OPT_QAM_STD_opt_qam_std_mask), DVBC_OPT_QAM_STD_opt_qam_std(0x0));	//0x003=0x0, opt_qam_std, DVBC: 0x0, Open Cable = 0x1
	rtd_maskl(DVBC_OPT_RF_AAGC_DRIVE_reg, ~(0xff), (DVBC_OPT_RF_AAGC_DRIVE_opt_if_aagc_oe(0x1)|DVBC_OPT_RF_AAGC_DRIVE_opt_par_if_sd_ib(0x1)|DVBC_OPT_RF_AAGC_DRIVE_opt_if_aagc_drive(0x1)));	//0x006=0x80, [7:4] set if_agc, IF_AGC output enable, if_inverted, push pull mode; [3:0]set rf_agc
	rtd_maskl(DVBC_DEBUG_REG_2_reg, ~(DVBC_DEBUG_REG_2_debug_reg_23_16_mask), DVBC_DEBUG_REG_2_debug_reg_23_16(0x00));	//0x00f=0x0, [7:0] debug_reg[23:16]
	
	//sync CR settings from K5
	rtd_maskl(DVBC_OPT_BN_EQ_STOP_EN_reg, ~(0xff), (DVBC_OPT_BN_EQ_STOP_EN_bn_cond5_en(0x1)|DVBC_OPT_BN_EQ_STOP_EN_bn_cond4_en(0x1)|DVBC_OPT_BN_EQ_STOP_EN_bn_cond2_en(0x1)|DVBC_OPT_BN_EQ_STOP_EN_opt_bn_cr_blank_c2(0x1)));
	rtd_maskl(DVBC_OPT_BN_COND1_CNT_TH_reg, ~(0xff), (DVBC_OPT_BN_COND1_CNT_TH_opt_bn_cond2_cnt_th(0x3)|DVBC_OPT_BN_COND1_CNT_TH_opt_bn_cond1_cnt_th(0x4)));
	rtd_maskl(DVBC_OPT_BN_ERR_ACC_TH_reg, ~(0xff), DVBC_OPT_BN_ERR_ACC_TH_opt_bn_err_acc_th(0x07));
	rtd_maskl(DVBC_OPT_AK_DD_CR_POW_TH_reg, ~(0xff), (DVBC_OPT_AK_DD_CR_POW_TH_opt_ak_pow_in(0x1)|DVBC_OPT_AK_DD_CR_POW_TH_opt_ak_dd_cr_pow_th(0x12)));
	
	rtd_maskl(DVBC_OPT_MPEG_OUT_SEL_reg, ~(DVBC_OPT_MPEG_OUT_SEL_opt_mpeg_out_sel_mask), DVBC_OPT_MPEG_OUT_SEL_opt_mpeg_out_sel(0x1));	//0x026=0x01, [1:0]opt_mpeg_out_sel
	rtd_maskl(DVBC_REG_ADC_CLK_XOR_reg, ~(0xff), (DVBC_REG_ADC_CLK_XOR_reg_aagc_cal_payload(0x1)|DVBC_REG_ADC_CLK_XOR_reg_en_gi_pga(0x1)));	//0x060=0x28, reg_adc_clk_xor, [5]reg_aagc_cal_payload = 0x1, [3]reg_en_gi_pga = 0x1
	rtd_maskl(DVBC_OPT_AAGC_TARGET_AMP_reg, ~(DVBC_OPT_AAGC_TARGET_AMP_opt_aagc_target_amp_mask), DVBC_OPT_AAGC_TARGET_AMP_opt_aagc_target_amp(0x64));	//0x105=0x64, [7:0]opt_aagc_target_amp, aagc loop reference amplitude


	rtd_maskl(DVBC_OPT_DDC_FREQ_reg, ~(DVBC_OPT_DDC_FREQ_opt_ddc_freq_7_0_mask), DVBC_OPT_DDC_FREQ_opt_ddc_freq_7_0(0x38));	//0x10d=0x4b, opt_ddc_freq[7:0]   IFfreqHZ=5MHz, ADC = 36MHz :		(36-5)/36*32768 = 28216 =>0x6e38
	rtd_maskl(DVBC_OPT_DDC_FREQ_1_reg, ~(DVBC_OPT_DDC_FREQ_1_opt_spec_inv_mask|DVBC_OPT_DDC_FREQ_1_opt_ddc_freq_14_8_mask), DVBC_OPT_DDC_FREQ_1_opt_ddc_freq_14_8(0x6e));	//**[7]opt_spec_inv=0x0, [6:0]opt_ddc_freq[14:8]
	rtd_maskl(DVBC_OPT_DDC_LPF_SCALING_reg, ~(0xff), (DVBC_OPT_DDC_LPF_SCALING_opt_auto_spec(0x1)|DVBC_OPT_DDC_LPF_SCALING_opt_ddc_lpf_scaling(0x6)));
	
	//TR_DECI_RATIO = (ADC_frequency in Hz) * pow(2, 18) / (symbol rate in Hz) = 36 * 262144 / 6.875 = 1372681 =>0x14f209
	rtd_maskl(DVBC_OPT_TR_DECI_RATIO_reg, ~(DVBC_OPT_TR_DECI_RATIO_opt_tr_deci_ratio_7_0_mask), DVBC_OPT_TR_DECI_RATIO_opt_tr_deci_ratio_7_0(0xa5));//0x09
	rtd_maskl(DVBC_OPT_TR_DECI_RATIO_1_reg, ~(DVBC_OPT_TR_DECI_RATIO_1_opt_tr_deci_ratio_15_8_mask), DVBC_OPT_TR_DECI_RATIO_1_opt_tr_deci_ratio_15_8(0xb6));//0xf2
	rtd_maskl(DVBC_OPT_TR_DECI_RATIO_2_reg, ~(DVBC_OPT_TR_DECI_RATIO_2_opt_tr_deci_ratio_23_16_mask), DVBC_OPT_TR_DECI_RATIO_2_opt_tr_deci_ratio_23_16(0x14));//0x14
	
	//TP related
	rtd_maskl(DVBC_OPT_DESC_DISABLE_reg, ~(0xff), (DVBC_OPT_DESC_DISABLE_opt_dvbc_smooth(0x1)|DVBC_OPT_DESC_DISABLE_opt_fix_tei(0x1)));	//0x210=0x88, [7]opt_dvbc_smooth=0x1(enable), [3]opt_fix_tei=0x1(enable setting)
	rtd_maskl(DVBC_OPT_CKOUTPAR_reg, ~(0xff), (DVBC_OPT_CKOUTPAR_opt_sync_byte(0x0)|DVBC_OPT_CKOUTPAR_opt_serial(0x0)|DVBC_OPT_CKOUTPAR_opt_ckout_pwr(0x0)|DVBC_OPT_CKOUTPAR_opt_ckoutpar(0x0)));	//0x211=0x08, OPT_CKOUTPAR, [3]opt_sync_byte=0x1(byte mode)
	rtd_maskl(DVBC_OPT_CDIV_PH0_reg, ~(DVBC_OPT_CDIV_PH0_opt_cdiv_ph1_mask|DVBC_OPT_CDIV_PH0_opt_cdiv_ph0_mask), (DVBC_OPT_CDIV_PH0_opt_cdiv_ph1(0x9)|DVBC_OPT_CDIV_PH0_opt_cdiv_ph0(0x9)));	//0x212=0x99, [7:0]OPT_CDIV_PH0, PH1 related to TP clk
	
	//-------------set ddc_hd(DDC LPF shape)----------------------------//
	//6.875M/36M
	//{3fb,3fa,004,016,018,3f7,3c0,3b1,00e,0d5,1a6}
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x00));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x3));
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x7f));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x01));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x2));
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x7f));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x02));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x4));
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x00));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x03));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x6));
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x02));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x04));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x0));
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x03));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x05));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x7));
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x7e));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x06));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x0));
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x78));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x07));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x1));
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x76));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x08));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x6));
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x01));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x09));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x5));
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x1a));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_sel_mask), DVBC_DDC_WR_STB_ddc_sel(0x0a));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_coeff_2_0_mask), DVBC_DDC_WR_STB_ddc_coeff_2_0(0x6));
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~(DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask), DVBC_DDC_WR_STB_1_ddc_coeff_9_3(0x34));
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~(DVBC_DDC_WR_STB_ddc_wr_stb_mask), DVBC_DDC_WR_STB_ddc_wr_stb(0x1));
	//-------------End set ddc_hd(DDC LPF shape)----------------------------//

	//-------------Set Alpha Roll-off Factor Related = 0.15---------------------//
	//	AlphaValueTable[QAM_ALPHA_MODE_NUM][RTD299S_QAM_ALPHA_VALUE_TABLE_LEN] = {
	//		{3,    1,    27,   2,    8,  1017,  1013,  16,  14,  996,   1008,  50,  18,  927,  1004,  310},	// alpha = 0.15 };
	//-------------------------------------------------------------------------------------------------------------------------//
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_sel_mask), DVBC_RXMF_WR_STB_rxmf_sel(0x00));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_coeff_2_0_mask), DVBC_RXMF_WR_STB_rxmf_coeff_2_0(0x3));
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, ~(DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3_mask), DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3(0x00));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_wr_stb_mask), DVBC_RXMF_WR_STB_rxmf_wr_stb(0x1));

	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_sel_mask), DVBC_RXMF_WR_STB_rxmf_sel(0x01));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_coeff_2_0_mask), DVBC_RXMF_WR_STB_rxmf_coeff_2_0(0x1));
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, ~(DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3_mask), DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3(0x00));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_wr_stb_mask), DVBC_RXMF_WR_STB_rxmf_wr_stb(0x1));

	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_sel_mask), DVBC_RXMF_WR_STB_rxmf_sel(0x02));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_coeff_2_0_mask), DVBC_RXMF_WR_STB_rxmf_coeff_2_0(0x3));
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, ~(DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3_mask), DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3(0x03));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_wr_stb_mask), DVBC_RXMF_WR_STB_rxmf_wr_stb(0x1));

	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_sel_mask), DVBC_RXMF_WR_STB_rxmf_sel(0x03));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_coeff_2_0_mask), DVBC_RXMF_WR_STB_rxmf_coeff_2_0(0x2));
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, ~(DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3_mask), DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3(0x00));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_wr_stb_mask), DVBC_RXMF_WR_STB_rxmf_wr_stb(0x1));

	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_sel_mask), DVBC_RXMF_WR_STB_rxmf_sel(0x04));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_coeff_2_0_mask), DVBC_RXMF_WR_STB_rxmf_coeff_2_0(0x0));
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, ~(DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3_mask), DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3(0x01));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_wr_stb_mask), DVBC_RXMF_WR_STB_rxmf_wr_stb(0x1));

	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_sel_mask), DVBC_RXMF_WR_STB_rxmf_sel(0x05));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_coeff_2_0_mask), DVBC_RXMF_WR_STB_rxmf_coeff_2_0(0x1));
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, ~(DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3_mask), DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3(0x7f));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_wr_stb_mask), DVBC_RXMF_WR_STB_rxmf_wr_stb(0x1));

	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_sel_mask), DVBC_RXMF_WR_STB_rxmf_sel(0x06));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_coeff_2_0_mask), DVBC_RXMF_WR_STB_rxmf_coeff_2_0(0x5));
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, ~(DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3_mask), DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3(0x7e));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_wr_stb_mask), DVBC_RXMF_WR_STB_rxmf_wr_stb(0x1));

	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_sel_mask), DVBC_RXMF_WR_STB_rxmf_sel(0x07));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_coeff_2_0_mask), DVBC_RXMF_WR_STB_rxmf_coeff_2_0(0x0));
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, ~(DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3_mask), DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3(0x02));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_wr_stb_mask), DVBC_RXMF_WR_STB_rxmf_wr_stb(0x1));

	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_sel_mask), DVBC_RXMF_WR_STB_rxmf_sel(0x08));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_coeff_2_0_mask), DVBC_RXMF_WR_STB_rxmf_coeff_2_0(0x6));
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, ~(DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3_mask), DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3(0x01));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_wr_stb_mask), DVBC_RXMF_WR_STB_rxmf_wr_stb(0x1));

	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_sel_mask), DVBC_RXMF_WR_STB_rxmf_sel(0x09));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_coeff_2_0_mask), DVBC_RXMF_WR_STB_rxmf_coeff_2_0(0x4));
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, ~(DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3_mask), DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3(0x7c));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_wr_stb_mask), DVBC_RXMF_WR_STB_rxmf_wr_stb(0x1));

	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_sel_mask), DVBC_RXMF_WR_STB_rxmf_sel(0x0a));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_coeff_2_0_mask), DVBC_RXMF_WR_STB_rxmf_coeff_2_0(0x0));
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, ~(DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3_mask), DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3(0x7e));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_wr_stb_mask), DVBC_RXMF_WR_STB_rxmf_wr_stb(0x1));

	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_sel_mask), DVBC_RXMF_WR_STB_rxmf_sel(0x0b));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_coeff_2_0_mask), DVBC_RXMF_WR_STB_rxmf_coeff_2_0(0x2));
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, ~(DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3_mask), DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3(0x06));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_wr_stb_mask), DVBC_RXMF_WR_STB_rxmf_wr_stb(0x1));

	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_sel_mask), DVBC_RXMF_WR_STB_rxmf_sel(0x0c));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_coeff_2_0_mask), DVBC_RXMF_WR_STB_rxmf_coeff_2_0(0x2));
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, ~(DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3_mask), DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3(0x02));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_wr_stb_mask), DVBC_RXMF_WR_STB_rxmf_wr_stb(0x1));

	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_sel_mask), DVBC_RXMF_WR_STB_rxmf_sel(0x0d));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_coeff_2_0_mask), DVBC_RXMF_WR_STB_rxmf_coeff_2_0(0x7));
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, ~(DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3_mask), DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3(0x73));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_wr_stb_mask), DVBC_RXMF_WR_STB_rxmf_wr_stb(0x1));

	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_sel_mask), DVBC_RXMF_WR_STB_rxmf_sel(0x0e));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_coeff_2_0_mask), DVBC_RXMF_WR_STB_rxmf_coeff_2_0(0x4));
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, ~(DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3_mask), DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3(0x7d));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_wr_stb_mask), DVBC_RXMF_WR_STB_rxmf_wr_stb(0x1));

	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_sel_mask), DVBC_RXMF_WR_STB_rxmf_sel(0x0f));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_coeff_2_0_mask), DVBC_RXMF_WR_STB_rxmf_coeff_2_0(0x6));
	rtd_maskl(DVBC_RXMF_WR_STB_1_reg, ~(DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3_mask), DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3(0x26));
	rtd_maskl(DVBC_RXMF_WR_STB_reg, ~(DVBC_RXMF_WR_STB_rxmf_wr_stb_mask), DVBC_RXMF_WR_STB_rxmf_wr_stb(0x1));
	//------------------------End of set Alpha value----------------------------//
	//----------------------------End set Coeff -------------------------//

	// Old IC sample rate sync, NO NEED
	//rtd_maskl(DVBC_OPT_QAM_MODE_reg, ~(DVBC_OPT_QAM_MODE_opt_adc_samp_ph_mask), DVBC_OPT_QAM_MODE_opt_adc_samp_ph(0x1));	//0x102=0x08, [5:3]opt_adc_samp_ph=0x1
	rtd_maskl(DVBC_OPT_QAM_ON_reg, ~(DVBC_OPT_QAM_ON_opt_qam_on_mask), DVBC_OPT_QAM_ON_opt_qam_on(0x1));	//0x004=0x01, [0]opt_qam_on=0x1
	rtd_maskl(DVBC_PAGE2_RW21_RESERVED_reg, ~0x01, 0x00);	//0x221=0x00, [7:0]page2_rw21_reserved=0x0
	rtd_maskl(DVBC_OPT_DSP_FIRST_OUT_IDX_reg, ~(DVBC_OPT_DSP_FIRST_OUT_IDX_opt_dsp_first_out_idx_7_0_mask), DVBC_OPT_DSP_FIRST_OUT_IDX_opt_dsp_first_out_idx_7_0(0x00));	//0x22e=0x00, [7:0]opt_dsp_first_out_idx=0x00
	rtd_maskl(DVBC_OPT_DSP_FIRST_OUT_IDX_1_reg, ~(DVBC_OPT_DSP_FIRST_OUT_IDX_1_opt_dsp_first_out_idx_12_8_mask), DVBC_OPT_DSP_FIRST_OUT_IDX_1_opt_dsp_first_out_idx_12_8(0x10));	//0x22f=0x10, [12:8]opt_dsp_first_out_idx=0x10
	rtd_maskl(DVBC_OPT_FFT_ABS_OUT_reg, ~(DVBC_OPT_FFT_ABS_OUT_opt_fft_abs_out_mask), DVBC_OPT_FFT_ABS_OUT_opt_fft_abs_out(0x1));	//0x230=0x01, [0]opt_fft_abs_out=0x1
	rtd_maskl(DVBC_OPT_FFT_DSP_READ_reg, ~(DVBC_OPT_FFT_DSP_READ_opt_fft_dsp_read_mask), DVBC_OPT_FFT_DSP_READ_opt_fft_dsp_read(0x1));	//0x231=0x01, [0]opt_fft_dsp_read=0x1

	//------------------------NEW adaptive CR BW----------------------------//
	rtd_maskl(DVBC_ADAP_CR_BW_EN_reg, ~(DVBC_ADAP_CR_BW_EN_opt_adap_cr_bw_en_mask), DVBC_ADAP_CR_BW_EN_opt_adap_cr_bw_en(0x1)); //adaptive cr enable
	rtd_maskl(DVBC_ADAP_CR_BW_EN_reg, ~(DVBC_ADAP_CR_BW_EN_opt_adap_cr_bw_chk_prd_mask), DVBC_ADAP_CR_BW_EN_opt_adap_cr_bw_chk_prd(0x1)); //opt_adap_cr_bw_chk_prd = 1
	rtd_maskl(DVBC_ADAP_CR_BW_D2_NUM_reg, ~(DVBC_ADAP_CR_BW_D2_NUM_opt_adap_cr_bw_d2_num_7_0_mask), DVBC_ADAP_CR_BW_D2_NUM_opt_adap_cr_bw_d2_num_7_0(0x80)); //opt_adap_cr_bw_d2_num = 128
	rtd_maskl(DVBC_ADAP_CR_BW_EN_reg, ~(DVBC_ADAP_CR_BW_EN_opt_adap_cr_bw_d2_num_9_8_mask), DVBC_ADAP_CR_BW_EN_opt_adap_cr_bw_d2_num_9_8(0x0));
	rtd_maskl(DVBC_ADAP_CR_BW_U2_NUM_reg, ~(DVBC_ADAP_CR_BW_U2_NUM_opt_adap_cr_bw_u2_num_7_0_mask), DVBC_ADAP_CR_BW_U2_NUM_opt_adap_cr_bw_u2_num_7_0(0x00)); //opt_adap_cr_bw_u2_num = 512
	rtd_maskl(DVBC_ADAP_CR_BW_EN_reg, ~(DVBC_ADAP_CR_BW_EN_opt_adap_cr_bw_u2_num_9_8_mask), DVBC_ADAP_CR_BW_EN_opt_adap_cr_bw_u2_num_9_8(0x2));
	rtd_maskl(DVBC_ADAP_CR_BW_CHK_THD_reg, ~(DVBC_ADAP_CR_BW_CHK_THD_opt_adap_cr_bw_chk_thd1_mask|DVBC_ADAP_CR_BW_CHK_THD_opt_adap_cr_bw_chk_thd0_mask), (DVBC_ADAP_CR_BW_CHK_THD_opt_adap_cr_bw_chk_thd1(0xb)|DVBC_ADAP_CR_BW_CHK_THD_opt_adap_cr_bw_chk_thd0(0x2))); //[7:4]thd1 = -5 [3:0]thd0 = 2
	rtd_maskl(DVBC_ADAP_CR_BW_CHK_PRD1_reg, ~(DVBC_ADAP_CR_BW_CHK_PRD1_opt_adap_cr_bw_chk_prd1_mask), DVBC_ADAP_CR_BW_CHK_PRD1_opt_adap_cr_bw_chk_prd1(0x2)); //opt_adap_cr_bw_chk_prd1 = 2
	rtd_maskl(DVBC_ADAP_CR_BW_D2_NUM1_reg, ~(DVBC_ADAP_CR_BW_D2_NUM1_opt_adap_cr_bw_d2_num1_7_0_mask), DVBC_ADAP_CR_BW_D2_NUM1_opt_adap_cr_bw_d2_num1_7_0(0xc0)); //opt_adap_cr_bw_d2_num1 = 192
	rtd_maskl(DVBC_ADAP_CR_BW_CHK_PRD1_reg, ~(DVBC_ADAP_CR_BW_CHK_PRD1_opt_adap_cr_bw_d2_num1_9_8_mask), DVBC_ADAP_CR_BW_CHK_PRD1_opt_adap_cr_bw_d2_num1_9_8(0x0));
	rtd_maskl(DVBC_ADAP_CR_BW_U2_NUM1_reg, ~(DVBC_ADAP_CR_BW_U2_NUM1_opt_adap_cr_bw_u2_num1_7_0_mask), DVBC_ADAP_CR_BW_U2_NUM1_opt_adap_cr_bw_u2_num1_7_0(0x80)); //opt_adap_cr_bw_u2_num1 = 128
	rtd_maskl(DVBC_ADAP_CR_BW_CHK_PRD1_reg, ~(DVBC_ADAP_CR_BW_CHK_PRD1_opt_adap_cr_bw_u2_num1_9_8_mask), DVBC_ADAP_CR_BW_CHK_PRD1_opt_adap_cr_bw_u2_num1_9_8(0x0));
	rtd_maskl(DVBC_ADAP_CR_BW_CHK_PRD2_reg, ~(DVBC_ADAP_CR_BW_CHK_PRD2_opt_adap_cr_bw_chk_prd2_mask), DVBC_ADAP_CR_BW_CHK_PRD2_opt_adap_cr_bw_chk_prd2(0x2)); //opt_adap_cr_bw_chk_prd2 = 2
	rtd_maskl(DVBC_ADAP_CR_BW_D2_NUM2_reg, ~(DVBC_ADAP_CR_BW_D2_NUM2_opt_adap_cr_bw_d2_num2_7_0_mask), DVBC_ADAP_CR_BW_D2_NUM2_opt_adap_cr_bw_d2_num2_7_0(0x00));
	rtd_maskl(DVBC_ADAP_CR_BW_CHK_PRD2_reg, ~(DVBC_ADAP_CR_BW_CHK_PRD2_opt_adap_cr_bw_d2_num2_9_8_mask), DVBC_ADAP_CR_BW_CHK_PRD2_opt_adap_cr_bw_d2_num2_9_8(0x1)); //pt_adap_cr_bw_d2_num2 = 256
	rtd_maskl(DVBC_ADAP_CR_BW_U2_NUM2_reg, ~(DVBC_ADAP_CR_BW_U2_NUM2_opt_adap_cr_bw_u2_num2_7_0_mask), DVBC_ADAP_CR_BW_U2_NUM2_opt_adap_cr_bw_u2_num2_7_0(0x80)); //opt_adap_cr_bw_u2_num2 = 128
	rtd_maskl(DVBC_ADAP_CR_BW_CHK_PRD2_reg, ~(DVBC_ADAP_CR_BW_CHK_PRD2_opt_adap_cr_bw_u2_num2_9_8_mask), DVBC_ADAP_CR_BW_CHK_PRD2_opt_adap_cr_bw_u2_num2_9_8(0x0));
	
	//------------------------For Auto mode CSR setting------------------------//
	rtd_maskl(DVBC_CSRS_CSR_THD_WEIGHT_reg, ~(DVBC_CSRS_CSR_THD_WEIGHT_reg_csrs_csr_threshold_weight_mask), DVBC_CSRS_CSR_THD_WEIGHT_reg_csrs_csr_threshold_weight(0x4));//field notch 5 to 4
	rtd_maskl(DVBC_CSRS_MIN_SPACE_reg, ~(DVBC_CSRS_MIN_SPACE_reg_csrs_min_space_7_0_mask), DVBC_CSRS_MIN_SPACE_reg_csrs_min_space_7_0(0xc8));//CSR min bin to 2.0M
	rtd_maskl(DVBC_CSRS_CSR_THD_WEIGHT_reg, ~(DVBC_CSRS_CSR_THD_WEIGHT_reg_csrs_min_space_9_8_mask), DVBC_CSRS_CSR_THD_WEIGHT_reg_csrs_min_space_9_8(0x1));
	rtd_maskl(DVBC_CSR_NO_SIGNAL_reg, ~(DVBC_CSR_NO_SIGNAL_reg_csr_fft_bin_max_7_0_mask), DVBC_CSR_NO_SIGNAL_reg_csr_fft_bin_max_7_0(0x34));//CSR max bin to 8.1M
	rtd_maskl(DVBC_CSR_NO_SIGNAL2_reg, ~(DVBC_CSR_NO_SIGNAL2_reg_csr_fft_bin_max_11_8_mask), DVBC_CSR_NO_SIGNAL2_reg_csr_fft_bin_max_11_8(0x7));
	//------------------------For Auto mode FSR setting------------------------//
	rtd_maskl(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_1_reg, ~(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_1_reg_fsrs_search_range_half_add_11_8_mask), DVBC_OPT_FSRS_SEARCH_RANGE_HALF_1_reg_fsrs_search_range_half_add_11_8(0x0));	//[11:8]larger FSR search range half add
	rtd_maskl(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_reg, ~(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_reg_fsrs_search_range_half_add_7_0_mask), DVBC_OPT_FSRS_SEARCH_RANGE_HALF_reg_fsrs_search_range_half_add_7_0(0xa0));	//[7:0] FSR search range half add 0x72=500K 0x89=600K 0xe4=1000K 0xcd=900K 0x94=650K 0xa0=700K
	rtd_maskl(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_SUB1_reg, ~(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_SUB1_reg_fsrs_search_range_half_sub_11_8_mask), DVBC_OPT_FSRS_SEARCH_RANGE_HALF_SUB1_reg_fsrs_search_range_half_sub_11_8(0x0));	//[11:8]larger FSR search range half sub
	rtd_maskl(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_SUB_reg, ~(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_SUB_reg_fsrs_search_range_half_sub_7_0_mask), DVBC_OPT_FSRS_SEARCH_RANGE_HALF_SUB_reg_fsrs_search_range_half_sub_7_0(0xab));	//[7:0] FSR search range half sub 0x72=500K 0x89=600K 0xe4=1000K 0xcd=900K 0x94=650K 0xa0=700K 0xab=750K
	rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_reg, ~(DVBC_OPT_POS_FRQ_MAX_SR_opt_pos_frq_max_sr_7_0_mask), DVBC_OPT_POS_FRQ_MAX_SR_opt_pos_frq_max_sr_7_0(0xd0));	//0x23e=0x73, [7:0]opt_pos_frq_max_sr[7:0]=0x73
	rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_1_reg, ~(DVBC_OPT_POS_FRQ_MAX_SR_1_opt_pos_frq_max_sr_11_8_mask), DVBC_OPT_POS_FRQ_MAX_SR_1_opt_pos_frq_max_sr_11_8(0xc));
	rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_reg, ~(DVBC_OPT_POS_FRQ_MIN_SR_opt_pos_frq_min_sr_7_0_mask), DVBC_OPT_POS_FRQ_MIN_SR_opt_pos_frq_min_sr_7_0(0x63));
	rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_1_reg, ~(DVBC_OPT_POS_FRQ_MIN_SR_1_opt_pos_frq_min_sr_11_8_mask), DVBC_OPT_POS_FRQ_MIN_SR_1_opt_pos_frq_min_sr_11_8(0x6));
	//for 163p5 field stream
	rtd_maskl(DVBC_OPT_POS_NEG_DIFF_reg, ~(DVBC_OPT_POS_NEG_DIFF_opt_pos_neg_diff_idx_thd_7_0_mask), DVBC_OPT_POS_NEG_DIFF_opt_pos_neg_diff_idx_thd_7_0(0x7e));	//pos_neg_diff_idx_thd=0xa7e for ADC=36MHz
	rtd_maskl(DVBC_OPT_POS_NEG_DIFF_1_reg, ~(0xff), (DVBC_OPT_POS_NEG_DIFF_1_opt_pos_neg_diff_idx_low(0x1)|DVBC_OPT_POS_NEG_DIFF_1_opt_pos_neg_diff_idx_thd_11_8(0xa)));
	//------------------------For Auto mode CCFO setting-----------------------//
	rtd_maskl(DVBC_FFT_INPUT_BITWISE_AUTO_reg, ~(DVBC_FFT_INPUT_BITWISE_AUTO_opt_fft_input_bitwise_sel_ccfo_mask), DVBC_FFT_INPUT_BITWISE_AUTO_opt_fft_input_bitwise_sel_ccfo(0x2)); //match C platform
	rtd_maskl(DVBC_OPT_POS_FRQ_SR_CCFO_reg, ~(DVBC_OPT_POS_FRQ_SR_CCFO_opt_pos_frq_max_sr_ccfo_7_0_mask), DVBC_OPT_POS_FRQ_SR_CCFO_opt_pos_frq_max_sr_ccfo_7_0(0xf5));
	rtd_maskl(DVBC_OPT_POS_FRQ_SR_CCFO_2_reg, ~(DVBC_OPT_POS_FRQ_SR_CCFO_2_opt_pos_frq_max_sr_ccfo_11_8_mask), DVBC_OPT_POS_FRQ_SR_CCFO_2_opt_pos_frq_max_sr_ccfo_11_8(0x1));
	rtd_maskl(DVBC_FFT_ACC_NUM_AUTO_1_reg, ~(DVBC_FFT_ACC_NUM_AUTO_1_opt_fft_output_acc_num_ccfo_mask), DVBC_FFT_ACC_NUM_AUTO_1_opt_fft_output_acc_num_ccfo(0x0f)); //increase CCFO acc num to 15
	rtd_outl(DVBC_OPT_KI_CR_OUT_MAX_reg, DVBC_OPT_KI_CR_OUT_MAX_opt_ki_cr_out_max(0xa4));//for CR range
	rtd_maskl(DVBC_CCFO_PAPR_reg, ~(DVBC_CCFO_PAPR_opt_ccfo_max_papr_en_mask), DVBC_CCFO_PAPR_opt_ccfo_max_papr_en(0x0)); //enable papr ccfo
	//------------------------For Auto mode Qamd  setting-----------------------//
	rtd_maskl(DVBC_OPT_QAMD_START_STATE_reg, ~(DVBC_OPT_QAMD_START_STATE_opt_qamd_start_state_mask), DVBC_OPT_QAMD_START_STATE_opt_qamd_start_state(0x5)); //opt_qamd_start_state[3:0]
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN9_MIN_reg, ~(DVBC_OPT_QAMD_EQP_ZN9_MIN_opt_qamd_eqp_zn9_min_7_0_mask), DVBC_OPT_QAMD_EQP_ZN9_MIN_opt_qamd_eqp_zn9_min_7_0(0x3d));//opt_qamd_eqp_zn9_min[7:0] for auto qam match C platform
	rtd_maskl(DVBC_OPT_QAMD_EQP_ZN10_MIN_reg, ~(DVBC_OPT_QAMD_EQP_ZN10_MIN_opt_qamd_eqp_zn10_min_7_0_mask), DVBC_OPT_QAMD_EQP_ZN10_MIN_opt_qamd_eqp_zn10_min_7_0(0x98));//opt_qamd_eqp_zn10_min[7:0]

	//--------------------------Enable erasure decode---------------------------//
	rtd_maskl(DVBC_DVBC_ERASURE_RS_CTRL0_reg, ~(DVBC_DVBC_ERASURE_RS_CTRL0_opt_erasure_rs_mask), DVBC_DVBC_ERASURE_RS_CTRL0_opt_erasure_rs(0x1));
	rtd_maskl(DVBC_DVBC_ERASURE_RS_CTRL1_reg, ~(DVBC_DVBC_ERASURE_RS_CTRL1_opt_erasure_num_mask), DVBC_DVBC_ERASURE_RS_CTRL1_opt_erasure_num(0x4));

	//-------------Start NEW FSM with auto mode-------------------------//
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x00);	//ST00 Sel
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x00);
	rtd_outl(DVBC_FSM_WORD_10_reg, 0x18);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0x98);
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa0);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x00);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0x00);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0x01);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0xf2);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x8e);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0x4f);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0xee);
	rtd_outl(DVBC_FSM_WORD_reg, 0xe0);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x01);	//ST00 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x02);	//ST01 Sel
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x00);
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xb8);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0x98);
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa4);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x4b);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0x80);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0x01);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0xf2);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x8e);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0x89);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0x22);
	rtd_outl(DVBC_FSM_WORD_reg, 0x21);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x03);	//ST01 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x04);	//ST02 Sel
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x03);
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xb8);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0xd8);	//orig: 0xf8  disable upd_b 20181130
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa4);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x46);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0xc0);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0x00);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0x72);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x8e);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0x89);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0x32);
	rtd_outl(DVBC_FSM_WORD_reg, 0x22);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x05);	//ST02 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x06);	//ST03 Sel
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x07);
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xb2);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0xd8);	//orig: 0xf8  disable upd_b 20181130
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa4);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x44);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0xb5);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0xdd);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0xf2);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x0e);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0x8b);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0x44);
	rtd_outl(DVBC_FSM_WORD_reg, 0x43);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x07);	//ST03 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x08);	//ST04 Sel
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x07);
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xb2);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0xd8);	//orig: 0xf8  disable upd_b 20181130
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa4);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x44);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0xb5);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0x4d);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0xf3);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x1e);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0xcb);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0x55);
	rtd_outl(DVBC_FSM_WORD_reg, 0x54);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x09);	//ST04 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x0a);	//ST05 Sel
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x07);
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xb2);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0xd8);	//orig: 0xf8  disable upd_b 20181130
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa4);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x44);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0xb4);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0x4b);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0xf5);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x1e);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0xcb);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0x66);
	rtd_outl(DVBC_FSM_WORD_reg, 0x65);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x0b);	//ST05 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x0c);	//ST06 Sel
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x07);
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xb2);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0xd8);	//orig: 0xf8  disable upd_b 20181130
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa4);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x40);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0x73);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0xc7);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0xf7);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x9e);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0xcb);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0x77);
	rtd_outl(DVBC_FSM_WORD_reg, 0x76);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x0d);	//ST06 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x0e);	//ST07 Sel
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x07);
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xb2);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0xd8);	//orig: 0xf8  disable upd_b 20181130
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa4);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x40);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0x72);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0x41);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0x48);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x2e);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0xcb);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0x88);
	rtd_outl(DVBC_FSM_WORD_reg, 0x87);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x0f);	//ST07 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x10);	//ST08 Sel
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x07);
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xb2);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0xda);	//orig: 0xfa  disable upd_b 20181130
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa4);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x40);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0x71);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0x21);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0x48);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x3e);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0xcc);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0x99);
	rtd_outl(DVBC_FSM_WORD_reg, 0x98);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x11);	//ST08 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x12);	//ST09 Sel
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x07);
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xb2);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0xfe);
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa4);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x40);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0x71);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0x20);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0x48);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x4e);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0xc9);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0xa9);
	rtd_outl(DVBC_FSM_WORD_reg, 0x99);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x13);	//ST09 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x14);	//ST10 Sel
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x07);
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xb2);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0xfe);
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa6);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x60);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0x71);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0x20);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0x48);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x5e);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0xee);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0xbb);
	rtd_outl(DVBC_FSM_WORD_reg, 0xaa);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x15);	//ST10 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x16);	//ST11 Sel
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x07);
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xf3);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0xfe);
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa6);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x60);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0x73);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0x18);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0x24);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x6e);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0xcb);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0xc8);
	rtd_outl(DVBC_FSM_WORD_reg, 0xbb);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x17);	//ST11 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x18);	//ST12 Sel
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x07);
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xf3);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0x7f);
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa9);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x20);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0x4b);
	//rtd_outl(DVBC_FSM_WORD_5_reg, 0xda);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0xdc);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0x28);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x7f);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0x00);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0xcc);
	rtd_outl(DVBC_FSM_WORD_reg, 0xc8);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x19);	//ST12 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x1a);	//ST13 Sel
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x00);
	rtd_outl(DVBC_FSM_WORD_10_reg, 0x18);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0x98);
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa0);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x00);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0x00);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0x01);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0xf2);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x8e);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0x4f);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0x1d);
	rtd_outl(DVBC_FSM_WORD_reg, 0x1d);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x1b);	//ST13 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x1c);	//ST14 Sel
	//rtd_outl(DVBC_FSM_WORD_11_reg, 0x00);
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x18);
	rtd_outl(DVBC_FSM_WORD_10_reg, 0x18);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0x98);
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa0);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x00);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0x00);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0x01);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0xf2);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x8e);
	//rtd_outl(DVBC_FSM_WORD_2_reg, 0x4f);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0x40);
	//rtd_outl(DVBC_FSM_WORD_1_reg, 0x1f);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0x11);
	rtd_outl(DVBC_FSM_WORD_reg, 0x1e);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x1d);	//ST14 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x1e);	//ST15 Sel
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x00);
	rtd_outl(DVBC_FSM_WORD_10_reg, 0x18);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0x98);
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa0);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x00);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0x00);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0x01);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0xf2);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x8e);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0x4f);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0x11);
	rtd_outl(DVBC_FSM_WORD_reg, 0x1f);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x1f);	//ST15 Write
	//----------------------------End set FSM -------------------------//
//---------------------------------End DVBC Initial Table--------------------------------//
	
	pDemod->QamMode = QAM_QAM_256;
	
	return FUNCTION_SUCCESS;
}


/**

@see   QAM_DEMOD_FP_SET_QAM_MODE

*/
int
realtek_r_dvbc_SetQamMode(
	QAM_DEMOD_MODULE *pDemod,
	int QamMode
)
{
	int SelectionIndex;

	//Fix SR, Set auto mode enable with ccfo only
	rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_ccfo_auto_mask, DVBC_OPT_START_SRS_NBD_opt_ccfo_auto(1));//enable ccfo update
	rtd_outl(DVBC_AUTO_SRS_reg, (DVBC_AUTO_SRS_opt_auto_srs_qam_start(0x1)|DVBC_AUTO_SRS_opt_csr_en(0x1)));//[0]Auto srs start [4]Auto CSR enable
	rtd_maskl(DVBC_FFT_ACC_NUM_AUTO_2_reg, ~DVBC_FFT_ACC_NUM_AUTO_2_opt_fft_output_acc_num_fsrs_mask, 0x01);//FSR acc num to 1
	rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask, DVBC_OPT_RST_STATE0_ON_opt_retry_on(1));//retry on

	switch (QamMode) {
		case QAM_QAM_16:
			pDemod->QamMode = QAM_QAM_16;
			SelectionIndex = 0;
			REALTEK_R_QAM_BASE_INFO("========[Set to 16QAM]========\n");
			rtd_maskl(DVBC_OPT_QAM_MODE_reg, ~DVBC_OPT_QAM_MODE_opt_qam_mode_mask, DVBC_OPT_QAM_MODE_opt_qam_mode(0x00));     //0xe102=0x0, 16QAM
			rtd_outl(DVBC_OPT_CR_ACQ_PWR_TH_reg,    0xdb);      //0xe118=0xdb
			rtd_outl(DVBC_OPT_CR_METRIC_DD_MAX_reg, 0xd4);      //0xe119=0xd4
			rtd_outl(DVBC_OPT_DD_EQ_BOUND_reg,      0xc4);      //0xe12e=0xc4
			rtd_outl(DVBC_OPT_SIGN_DATA_reg,        0x40);      //0xe12f=0x40
			rtd_outl(DVBC_OPT_SIGN_DATA_1_reg,      0x10);      //0xe130=0x10
			rtd_maskl(DVBC_OPT_CIRCLE_reg, ~DVBC_OPT_CIRCLE_opt_circle_mask, DVBC_OPT_CIRCLE_opt_circle(0x03));     //0xe131=0x3
			rtd_outl(DVBC_OPT_U_FFF_DD0_reg,        0x11);      //0xe140=0x11
			rtd_outl(DVBC_OPT_U_FFF_DD2_reg,        0x4c);      //0xe141=0x4c
			rtd_outl(DVBC_OPT_U_FFF2_DD0_reg,       0x1a);      //0xe197=0x1a
			rtd_outl(DVBC_OPT_U_FFF2_DD2_reg,       0x15);      //0xe198=0x15
			rtd_outl(DVBC_OPT_U_FFF3_DD1_reg,       0x2b);      //0xe199=0x2b
			rtd_outl(DVBC_OPT_U_FFF4_DD0_reg,       0x1a);      //0xe19a=0x1a
			rtd_outl(DVBC_OPT_U_FFF4_DD2_reg,       0x75);      //0xe19b=0x75
			break;

		case QAM_QAM_32:
			pDemod->QamMode = QAM_QAM_32;
			SelectionIndex = 1;
			REALTEK_R_QAM_BASE_INFO("Set to 32QAM\n");
			rtd_maskl(DVBC_OPT_QAM_MODE_reg, ~DVBC_OPT_QAM_MODE_opt_qam_mode_mask, DVBC_OPT_QAM_MODE_opt_qam_mode(0x01));     //0xe102=0x1, 32QAM
			rtd_outl(DVBC_OPT_CR_ACQ_PWR_TH_reg,    0x79);      //0xe118=0x79
			rtd_outl(DVBC_OPT_CR_METRIC_DD_MAX_reg, 0xcf);      //0xe119=0xcf
			rtd_outl(DVBC_OPT_DD_EQ_BOUND_reg,      0xc6);      //0xe12e=0xc6
			rtd_outl(DVBC_OPT_SIGN_DATA_reg,        0x20);      //0xe12f=0x20
			rtd_outl(DVBC_OPT_SIGN_DATA_1_reg,      0x17);      //0xe130=0x17
			rtd_maskl(DVBC_OPT_CIRCLE_reg, ~DVBC_OPT_CIRCLE_opt_circle_mask, DVBC_OPT_CIRCLE_opt_circle(0x04));     //0xe131=0x4
			rtd_outl(DVBC_OPT_U_FFF_DD0_reg,        0x11);      //0xe140=0x11
			rtd_outl(DVBC_OPT_U_FFF_DD2_reg,        0x4c);      //0xe141=0x4c
			rtd_outl(DVBC_OPT_U_FFF2_DD0_reg,       0x1a);      //0xe197=0x1a
			rtd_outl(DVBC_OPT_U_FFF2_DD2_reg,       0x15);      //0xe198=0x15
			rtd_outl(DVBC_OPT_U_FFF3_DD1_reg,       0x2b);      //0xe199=0x2b
			rtd_outl(DVBC_OPT_U_FFF4_DD0_reg,       0x1a);      //0xe19a=0x1a
			rtd_outl(DVBC_OPT_U_FFF4_DD2_reg,       0x75);      //0xe19b=0x75

			rtd_outl(DVBC_FSM_WR_STB_reg,           0x06);	//ST03
			rtd_outl(DVBC_FSM_WORD_reg,             0x43);
			rtd_outl(DVBC_FSM_WORD_1_reg ,          0x44);
			rtd_outl(DVBC_FSM_WORD_2_reg ,          0x8b);
			rtd_outl(DVBC_FSM_WORD_3_reg ,          0x0e);
			rtd_outl(DVBC_FSM_WORD_4_reg ,          0xf2);
			rtd_outl(DVBC_FSM_WORD_5_reg ,          0xd9);	//ori 0xdd
			rtd_outl(DVBC_FSM_WORD_6_reg ,          0xb4);	//ori 0xb5
			rtd_outl(DVBC_FSM_WORD_7_reg ,          0x44);
			rtd_outl(DVBC_FSM_WORD_8_reg ,          0xa4);
			rtd_outl(DVBC_FSM_WORD_9_reg ,          0xd8);	//orig: 0xf8  disable upd_b 20181130
			rtd_outl(DVBC_FSM_WORD_10_reg ,         0xb2);
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~DVBC_FSM_WORD_11_fsm_word_92_88_mask, DVBC_FSM_WORD_11_fsm_word_92_88(0x07));
			rtd_outl(DVBC_FSM_WR_STB_reg,           0x07);

			rtd_outl(DVBC_FSM_WR_STB_reg,           0x08);	//ST04
			rtd_outl(DVBC_FSM_WORD_reg,             0x54);
			rtd_outl(DVBC_FSM_WORD_1_reg ,          0x55);
			rtd_outl(DVBC_FSM_WORD_2_reg ,          0xcb);
			rtd_outl(DVBC_FSM_WORD_3_reg ,          0x1e);
			rtd_outl(DVBC_FSM_WORD_4_reg ,          0xf3);
			rtd_outl(DVBC_FSM_WORD_5_reg ,          0x49);	//ori 0x4d
			rtd_outl(DVBC_FSM_WORD_6_reg ,          0xb4);	//ori 0xb5
			rtd_outl(DVBC_FSM_WORD_7_reg ,          0x44);
			rtd_outl(DVBC_FSM_WORD_8_reg ,          0xa4);
			rtd_outl(DVBC_FSM_WORD_9_reg ,          0xd8);	//orig: 0xf8  disable upd_b 20181130
			rtd_outl(DVBC_FSM_WORD_10_reg ,         0xb2);
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~DVBC_FSM_WORD_11_fsm_word_92_88_mask, DVBC_FSM_WORD_11_fsm_word_92_88(0x07));
			rtd_outl(DVBC_FSM_WR_STB_reg,           0x09);

			rtd_outl(DVBC_FSM_WR_STB_reg,           0x0a);	//ST05
			rtd_outl(DVBC_FSM_WORD_reg,             0x65);
			rtd_outl(DVBC_FSM_WORD_1_reg ,          0x66);
			rtd_outl(DVBC_FSM_WORD_2_reg ,          0xcb);
			rtd_outl(DVBC_FSM_WORD_3_reg ,          0x1e);
			rtd_outl(DVBC_FSM_WORD_4_reg ,          0xf5);
			rtd_outl(DVBC_FSM_WORD_5_reg ,          0x47);	//ori 0x4b
			rtd_outl(DVBC_FSM_WORD_6_reg ,          0xb3);	//ori 0xb4
			rtd_outl(DVBC_FSM_WORD_7_reg ,          0x44);
			rtd_outl(DVBC_FSM_WORD_8_reg ,          0xa4);
			rtd_outl(DVBC_FSM_WORD_9_reg ,          0xd8);	//orig: 0xf8  disable upd_b 20181130
			rtd_outl(DVBC_FSM_WORD_10_reg ,         0xb2);
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~DVBC_FSM_WORD_11_fsm_word_92_88_mask, DVBC_FSM_WORD_11_fsm_word_92_88(0x07));
			rtd_outl(DVBC_FSM_WR_STB_reg,           0x0b);

			rtd_outl(DVBC_FSM_WR_STB_reg,           0x0c);	//ST06
			rtd_outl(DVBC_FSM_WORD_reg,             0x76);
			rtd_outl(DVBC_FSM_WORD_1_reg ,          0x77);
			rtd_outl(DVBC_FSM_WORD_2_reg ,          0xcb);
			rtd_outl(DVBC_FSM_WORD_3_reg ,          0x9e);
			rtd_outl(DVBC_FSM_WORD_4_reg ,          0xf7);
			rtd_outl(DVBC_FSM_WORD_5_reg ,          0xc3);	//ori 0xc7
			rtd_outl(DVBC_FSM_WORD_6_reg ,          0x72);	//ori 0x73
			rtd_outl(DVBC_FSM_WORD_7_reg ,          0x40);
			rtd_outl(DVBC_FSM_WORD_8_reg ,          0xa4);
			rtd_outl(DVBC_FSM_WORD_9_reg ,          0xd8);	//orig: 0xf8  disable upd_b 20181130
			rtd_outl(DVBC_FSM_WORD_10_reg ,         0xb2);
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~DVBC_FSM_WORD_11_fsm_word_92_88_mask, DVBC_FSM_WORD_11_fsm_word_92_88(0x07));
			rtd_outl(DVBC_FSM_WR_STB_reg,           0x0d);

			rtd_outl(DVBC_FSM_WR_STB_reg,           0x12);	//ST09
			rtd_outl(DVBC_FSM_WORD_reg,             0x99);
			rtd_outl(DVBC_FSM_WORD_1_reg ,          0xa9);
			rtd_outl(DVBC_FSM_WORD_2_reg ,          0xc9);
			rtd_outl(DVBC_FSM_WORD_3_reg ,          0x4e);
			rtd_outl(DVBC_FSM_WORD_4_reg ,          0x48);
			rtd_outl(DVBC_FSM_WORD_5_reg ,          0x10);	//ori 0x20
			rtd_outl(DVBC_FSM_WORD_6_reg ,          0x70);	//ori 0x71
			rtd_outl(DVBC_FSM_WORD_7_reg ,          0x40);
			rtd_outl(DVBC_FSM_WORD_8_reg ,          0xa4);
			rtd_outl(DVBC_FSM_WORD_9_reg ,          0xfe);
			rtd_outl(DVBC_FSM_WORD_10_reg ,         0xb2);
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~DVBC_FSM_WORD_11_fsm_word_92_88_mask, DVBC_FSM_WORD_11_fsm_word_92_88(0x07));
			rtd_outl(DVBC_FSM_WR_STB_reg,           0x13); //0xe15d=0x13

			rtd_outl(DVBC_FSM_WR_STB_reg,           0x14); //ST10
			rtd_outl(DVBC_FSM_WORD_reg,             0xaa);
			rtd_outl(DVBC_FSM_WORD_1_reg ,          0xbb);
			rtd_outl(DVBC_FSM_WORD_2_reg ,          0xee);
			rtd_outl(DVBC_FSM_WORD_3_reg ,          0x5e);
			rtd_outl(DVBC_FSM_WORD_4_reg ,          0x48);
			rtd_outl(DVBC_FSM_WORD_5_reg ,          0x10);	//ori 0x20
			rtd_outl(DVBC_FSM_WORD_6_reg ,          0x70);	//ori 0x71
			rtd_outl(DVBC_FSM_WORD_7_reg ,          0x60);
			rtd_outl(DVBC_FSM_WORD_8_reg ,          0xa6);
			rtd_outl(DVBC_FSM_WORD_9_reg ,          0xfe);
			rtd_outl(DVBC_FSM_WORD_10_reg ,         0xb2);
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~DVBC_FSM_WORD_11_fsm_word_92_88_mask, DVBC_FSM_WORD_11_fsm_word_92_88(0x07));
			rtd_outl(DVBC_FSM_WR_STB_reg,           0x15); //0xe15d=0x15

			rtd_outl(DVBC_FSM_WR_STB_reg,           0x18); //ST12
			rtd_outl(DVBC_FSM_WORD_reg,             0xc8);
			rtd_outl(DVBC_FSM_WORD_1_reg ,          0xcc);
			rtd_outl(DVBC_FSM_WORD_2_reg ,          0x00);
			rtd_outl(DVBC_FSM_WORD_3_reg ,          0x7f);
			rtd_outl(DVBC_FSM_WORD_4_reg ,          0x28);
			rtd_outl(DVBC_FSM_WORD_5_reg ,          0xda); //ori 0xdc
			rtd_outl(DVBC_FSM_WORD_6_reg ,          0x4b);
			rtd_outl(DVBC_FSM_WORD_7_reg ,          0x20);
			rtd_outl(DVBC_FSM_WORD_8_reg ,          0xa9);
			rtd_outl(DVBC_FSM_WORD_9_reg ,          0x7f);
			rtd_outl(DVBC_FSM_WORD_10_reg ,         0x73);
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~DVBC_FSM_WORD_11_fsm_word_92_88_mask, DVBC_FSM_WORD_11_fsm_word_92_88(0x07));
			rtd_outl(DVBC_FSM_WR_STB_reg,           0x19); //0xe15d=0x19
			break;

		case QAM_QAM_64:
			pDemod->QamMode = QAM_QAM_64;
			SelectionIndex = 2;
			REALTEK_R_QAM_BASE_INFO("========[Set to 64QAM]========\n");
			rtd_maskl(DVBC_OPT_QAM_MODE_reg, ~DVBC_OPT_QAM_MODE_opt_qam_mode_mask, DVBC_OPT_QAM_MODE_opt_qam_mode(0x02));     //0xe102=0x2, 64QAM
			rtd_outl(DVBC_OPT_CR_ACQ_PWR_TH_reg,    0x00);      //0xe118=0x0
			rtd_outl(DVBC_OPT_CR_METRIC_DD_MAX_reg, 0xd4);      //0xe119=0xd4
			rtd_outl(DVBC_OPT_DD_EQ_BOUND_reg,      0xc8);      //0xe12e=0xc8
			rtd_outl(DVBC_OPT_SIGN_DATA_reg,        0xc0);      //0xe12f=0xc0
			rtd_outl(DVBC_OPT_SIGN_DATA_1_reg,      0x21);      //0xe130=0x21
			rtd_maskl(DVBC_OPT_CIRCLE_reg, ~DVBC_OPT_CIRCLE_opt_circle_mask, DVBC_OPT_CIRCLE_opt_circle(0x05));     //0xe131=0x5
			rtd_outl(DVBC_OPT_U_FFF_DD0_reg,        0x11);      //0xe140=0x11
			rtd_outl(DVBC_OPT_U_FFF_DD2_reg,        0x4c);      //0xe141=0x4c
			rtd_outl(DVBC_OPT_U_FFF2_DD0_reg,       0x1a);      //0xe197=0x1a
			rtd_outl(DVBC_OPT_U_FFF2_DD2_reg,       0x15);      //0xe198=0x15
			rtd_outl(DVBC_OPT_U_FFF3_DD1_reg,       0x2b);      //0xe199=0x2b
			rtd_outl(DVBC_OPT_U_FFF4_DD0_reg,       0x1a);      //0xe19a=0x1a
			rtd_outl(DVBC_OPT_U_FFF4_DD2_reg,       0x75);      //0xe19b=0x75
			break;

		case QAM_QAM_128:
			pDemod->QamMode = QAM_QAM_128;
			SelectionIndex = 3;
			REALTEK_R_QAM_BASE_INFO("========[Set to 128QAM]========\n");
			rtd_maskl(DVBC_OPT_QAM_MODE_reg, ~DVBC_OPT_QAM_MODE_opt_qam_mode_mask, DVBC_OPT_QAM_MODE_opt_qam_mode(0x03));     //0xe102=0x2, 128QAM
			rtd_outl(DVBC_OPT_CR_ACQ_PWR_TH_reg,    0x8a);      //0xe118=0x8a
			rtd_outl(DVBC_OPT_CR_METRIC_DD_MAX_reg, 0xcf);      //0xe119=0xcf
			rtd_outl(DVBC_OPT_DD_EQ_BOUND_reg,      0xcc);      //0xe12e=0xcc
			rtd_outl(DVBC_OPT_SIGN_DATA_reg,        0xe0);      //0xe12f=0xe0
			rtd_outl(DVBC_OPT_SIGN_DATA_1_reg,      0x2e);      //0xe130=0x2e
			rtd_maskl(DVBC_OPT_CIRCLE_reg, ~DVBC_OPT_CIRCLE_opt_circle_mask, DVBC_OPT_CIRCLE_opt_circle(0x08));     //0xe131=0x8
			rtd_outl(DVBC_OPT_U_FFF_DD0_reg,        0x11);      //0xe140=0x11
			rtd_outl(DVBC_OPT_U_FFF_DD2_reg,        0x4c);      //0xe141=0x4c
			rtd_outl(DVBC_OPT_U_FFF2_DD0_reg,       0x1a);      //0xe197=0x1a
			rtd_outl(DVBC_OPT_U_FFF2_DD2_reg,       0x15);      //0xe198=0x15
			rtd_outl(DVBC_OPT_U_FFF3_DD1_reg,       0x2b);      //0xe199=0x2b
			rtd_outl(DVBC_OPT_U_FFF4_DD0_reg,       0x1a);      //0xe19a=0x1a
			rtd_outl(DVBC_OPT_U_FFF4_DD2_reg,       0x75);      //0xe19b=0x75

			rtd_outl(DVBC_FSM_WR_STB_reg,           0x06);	//ST03
			rtd_outl(DVBC_FSM_WORD_reg,             0x43);
			rtd_outl(DVBC_FSM_WORD_1_reg ,          0x44);
			rtd_outl(DVBC_FSM_WORD_2_reg ,          0x8b);
			rtd_outl(DVBC_FSM_WORD_3_reg ,          0x0e);
			rtd_outl(DVBC_FSM_WORD_4_reg ,          0xf2);
			rtd_outl(DVBC_FSM_WORD_5_reg ,          0xd9);	//ori 0xdd
			rtd_outl(DVBC_FSM_WORD_6_reg ,          0xb4);	//ori 0xb5
			rtd_outl(DVBC_FSM_WORD_7_reg ,          0x44);
			rtd_outl(DVBC_FSM_WORD_8_reg ,          0xa4);
			rtd_outl(DVBC_FSM_WORD_9_reg ,          0xd8);	//orig: 0xf8  disable upd_b 20181130
			rtd_outl(DVBC_FSM_WORD_10_reg ,         0xb2);
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~DVBC_FSM_WORD_11_fsm_word_92_88_mask, DVBC_FSM_WORD_11_fsm_word_92_88(0x07));
			rtd_outl(DVBC_FSM_WR_STB_reg,           0x07);

			rtd_outl(DVBC_FSM_WR_STB_reg,           0x08);	//ST04
			rtd_outl(DVBC_FSM_WORD_reg,             0x54);
			rtd_outl(DVBC_FSM_WORD_1_reg ,          0x55);
			rtd_outl(DVBC_FSM_WORD_2_reg ,          0xcb);
			rtd_outl(DVBC_FSM_WORD_3_reg ,          0x1e);
			rtd_outl(DVBC_FSM_WORD_4_reg ,          0xf3);
			rtd_outl(DVBC_FSM_WORD_5_reg ,          0x49);	//ori 0x4d
			rtd_outl(DVBC_FSM_WORD_6_reg ,          0xb4);	//ori 0xb5
			rtd_outl(DVBC_FSM_WORD_7_reg ,          0x44);
			rtd_outl(DVBC_FSM_WORD_8_reg ,          0xa4);
			rtd_outl(DVBC_FSM_WORD_9_reg ,          0xd8);	//orig: 0xf8  disable upd_b 20181130
			rtd_outl(DVBC_FSM_WORD_10_reg ,         0xb2);
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~DVBC_FSM_WORD_11_fsm_word_92_88_mask, DVBC_FSM_WORD_11_fsm_word_92_88(0x07));
			rtd_outl(DVBC_FSM_WR_STB_reg,           0x09);

			rtd_outl(DVBC_FSM_WR_STB_reg,           0x0a);	//ST05
			rtd_outl(DVBC_FSM_WORD_reg,             0x65);
			rtd_outl(DVBC_FSM_WORD_1_reg ,          0x66);
			rtd_outl(DVBC_FSM_WORD_2_reg ,          0xcb);
			rtd_outl(DVBC_FSM_WORD_3_reg ,          0x1e);
			rtd_outl(DVBC_FSM_WORD_4_reg ,          0xf5);
			rtd_outl(DVBC_FSM_WORD_5_reg ,          0x47);	//ori 0x4b
			rtd_outl(DVBC_FSM_WORD_6_reg ,          0xb3);	//ori 0xb4
			rtd_outl(DVBC_FSM_WORD_7_reg ,          0x44);
			rtd_outl(DVBC_FSM_WORD_8_reg ,          0xa4);
			rtd_outl(DVBC_FSM_WORD_9_reg ,          0xd8);	//orig: 0xf8  disable upd_b 20181130
			rtd_outl(DVBC_FSM_WORD_10_reg ,         0xb2);
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~DVBC_FSM_WORD_11_fsm_word_92_88_mask, DVBC_FSM_WORD_11_fsm_word_92_88(0x07));
			rtd_outl(DVBC_FSM_WR_STB_reg,           0x0b);

			rtd_outl(DVBC_FSM_WR_STB_reg,           0x0c);	//ST06
			rtd_outl(DVBC_FSM_WORD_reg,             0x76);
			rtd_outl(DVBC_FSM_WORD_1_reg ,          0x77);
			rtd_outl(DVBC_FSM_WORD_2_reg ,          0xcb);
			rtd_outl(DVBC_FSM_WORD_3_reg ,          0x9e);
			rtd_outl(DVBC_FSM_WORD_4_reg ,          0xf7);
			rtd_outl(DVBC_FSM_WORD_5_reg ,          0xc3);	//ori 0xc7
			rtd_outl(DVBC_FSM_WORD_6_reg ,          0x72);	//ori 0x73
			rtd_outl(DVBC_FSM_WORD_7_reg ,          0x40);
			rtd_outl(DVBC_FSM_WORD_8_reg ,          0xa4);
			rtd_outl(DVBC_FSM_WORD_9_reg ,          0xd8);	//orig: 0xf8  disable upd_b 20181130
			rtd_outl(DVBC_FSM_WORD_10_reg ,         0xb2);
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~DVBC_FSM_WORD_11_fsm_word_92_88_mask, DVBC_FSM_WORD_11_fsm_word_92_88(0x07));
			rtd_outl(DVBC_FSM_WR_STB_reg,           0x0d);

			rtd_outl(DVBC_FSM_WR_STB_reg,           0x12);	//ST09
			rtd_outl(DVBC_FSM_WORD_reg,             0x99);
			rtd_outl(DVBC_FSM_WORD_1_reg ,          0xa9);
			rtd_outl(DVBC_FSM_WORD_2_reg ,          0xc9);
			rtd_outl(DVBC_FSM_WORD_3_reg ,          0x4e);
			rtd_outl(DVBC_FSM_WORD_4_reg ,          0x48);
			rtd_outl(DVBC_FSM_WORD_5_reg ,          0x10);	//ori 0x20
			rtd_outl(DVBC_FSM_WORD_6_reg ,          0x70);	//ori 0x71
			rtd_outl(DVBC_FSM_WORD_7_reg ,          0x40);
			rtd_outl(DVBC_FSM_WORD_8_reg ,          0xa4);
			rtd_outl(DVBC_FSM_WORD_9_reg ,          0xfe);
			rtd_outl(DVBC_FSM_WORD_10_reg ,         0xb2);
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~DVBC_FSM_WORD_11_fsm_word_92_88_mask, DVBC_FSM_WORD_11_fsm_word_92_88(0x07));
			rtd_outl(DVBC_FSM_WR_STB_reg,           0x13); //0xe15d=0x13

			rtd_outl(DVBC_FSM_WR_STB_reg,           0x14); //ST10
			rtd_outl(DVBC_FSM_WORD_reg,             0xaa);
			rtd_outl(DVBC_FSM_WORD_1_reg ,          0xbb);
			rtd_outl(DVBC_FSM_WORD_2_reg ,          0xee);
			rtd_outl(DVBC_FSM_WORD_3_reg ,          0x5e);
			rtd_outl(DVBC_FSM_WORD_4_reg ,          0x48);
			rtd_outl(DVBC_FSM_WORD_5_reg ,          0x10);	//ori 0x20
			rtd_outl(DVBC_FSM_WORD_6_reg ,          0x70);	//ori 0x71
			rtd_outl(DVBC_FSM_WORD_7_reg ,          0x60);
			rtd_outl(DVBC_FSM_WORD_8_reg ,          0xa6);
			rtd_outl(DVBC_FSM_WORD_9_reg ,          0xfe);
			rtd_outl(DVBC_FSM_WORD_10_reg ,         0xb2);
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~DVBC_FSM_WORD_11_fsm_word_92_88_mask, DVBC_FSM_WORD_11_fsm_word_92_88(0x07));
			rtd_outl(DVBC_FSM_WR_STB_reg,           0x15); //0xe15d=0x15

			rtd_outl(DVBC_FSM_WR_STB_reg,           0x18); //ST12
			rtd_outl(DVBC_FSM_WORD_reg,             0xc8);
			rtd_outl(DVBC_FSM_WORD_1_reg ,          0xcc);
			rtd_outl(DVBC_FSM_WORD_2_reg ,          0x00);
			rtd_outl(DVBC_FSM_WORD_3_reg ,          0x7f);
			rtd_outl(DVBC_FSM_WORD_4_reg ,          0x28);
			rtd_outl(DVBC_FSM_WORD_5_reg ,          0xda); //ori 0xdc
			rtd_outl(DVBC_FSM_WORD_6_reg ,          0x4b);
			rtd_outl(DVBC_FSM_WORD_7_reg ,          0x20);
			rtd_outl(DVBC_FSM_WORD_8_reg ,          0xa9);
			rtd_outl(DVBC_FSM_WORD_9_reg ,          0x7f);
			rtd_outl(DVBC_FSM_WORD_10_reg ,         0x73);
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~DVBC_FSM_WORD_11_fsm_word_92_88_mask, DVBC_FSM_WORD_11_fsm_word_92_88(0x07));
			rtd_outl(DVBC_FSM_WR_STB_reg,           0x19); //0xe15d=0x19
			break;

		case QAM_QAM_256:
			pDemod->QamMode = QAM_QAM_256;
			SelectionIndex = 4;
			REALTEK_R_QAM_BASE_INFO("========[Set to 256QAM]========\n");
			rtd_maskl(DVBC_OPT_QAM_MODE_reg, ~DVBC_OPT_QAM_MODE_opt_qam_mode_mask, DVBC_OPT_QAM_MODE_opt_qam_mode(0x04));   //0xe102=0x2, 256QAM
			rtd_outl(DVBC_OPT_CR_ACQ_PWR_TH_reg,    0x00);
			rtd_outl(DVBC_OPT_CR_METRIC_DD_MAX_reg, 0xd4);
			rtd_outl(DVBC_OPT_DD_EQ_BOUND_reg,      0xd0);
			rtd_outl(DVBC_OPT_SIGN_DATA_reg,        0x80);
			rtd_outl(DVBC_OPT_SIGN_DATA_1_reg,      0x43);
			rtd_maskl(DVBC_OPT_CIRCLE_reg, ~DVBC_OPT_CIRCLE_opt_circle_mask, DVBC_OPT_CIRCLE_opt_circle(0x0a));     //0xe131=0xa
			rtd_outl(DVBC_OPT_U_FFF_DD0_reg,        0x11);      //0xe140=0x11
			rtd_outl(DVBC_OPT_U_FFF_DD2_reg,        0x4c);      //0xe141=0x4c
			rtd_outl(DVBC_OPT_U_FFF2_DD0_reg,       0x1a);      //0xe197=0x1a
			rtd_outl(DVBC_OPT_U_FFF2_DD2_reg,       0x15);      //0xe198=0x15
			rtd_outl(DVBC_OPT_U_FFF3_DD1_reg,       0x2b);      //0xe199=0x2b
			rtd_outl(DVBC_OPT_U_FFF4_DD0_reg,       0x1a);      //0xe19a=0x1a
			rtd_outl(DVBC_OPT_U_FFF4_DD2_reg,       0x75);      //0xe19b=0x75
			break;

		default:
			REALTEK_R_QAM_BASE_WARNING("Unsupported QAM Mode\n");
			goto error_status_get_unsupported_qam_mode;
	}

	// Set demod QAM mode parameter.
	pDemod->IsQamModeSet = YES;

	return FUNCTION_SUCCESS;

error_status_get_unsupported_qam_mode:
	return FUNCTION_ERROR;
}


/**

@see   QAM_DEMOD_FP_GET_QAM_MODE

*/

int
realtek_r_dvbc_GetQamMode(
	QAM_DEMOD_MODULE *pDemod,
	int *pQamMode
)
{
#if 1
	*pQamMode = pDemod->QamMode;
	return FUNCTION_SUCCESS;
#else
	int qam_mode;

	qam_mode = dvbc_Readbits1B(pDemod, 0xE102, 0x07, 0);
	switch (qam_mode) {
	case 0:
		*pQamMode = QAM_QAM_16;
		break;
	case 1:
		*pQamMode = QAM_QAM_32;
		break;
	default:
	case 2:
		*pQamMode = QAM_QAM_64;
		break;
	case 3:
		*pQamMode = QAM_QAM_128;
		break;
	case 4:
		*pQamMode = QAM_QAM_256;
		break;
	}
#endif

	return FUNCTION_SUCCESS;

}


/**

@see   QAM_DEMOD_FP_SET_ALPHA_MODE

*/
int
realtek_r_dvbc_SetAlphaMode(
	QAM_DEMOD_MODULE *pDemod,
	int AlphaMode
)
{
	static const U32BITS AlphaValueTable[QAM_ALPHA_MODE_NUM][REALTEK_R_QAM_ALPHA_VALUE_TABLE_LEN] = {
		{6,    28,   24,   8,    9,  1011,  1013,  22,  13,  990,   1010,  55,  15,  924,  1008,  314},	// alpha = 0.12
		{5,    30,   25,   6,    9,  1013,  1013,  20,  13,  992,   1009,  53,  16,  925,  1007,  313},	// alpha = 0.13
		{3,    1,    27,   2,    8,  1017,  1013,  16,  14,  996,   1008,  50,  18,  927,  1004,  310},	// alpha = 0.15
		{0,    3,    30,   30,   6,  1022,  1014,  10,  14,  1002,  1006,  45,  21,  931,  1001,  307},	// alpha = 0.18
		{31,   4,    31,   29,   4,  1,     1016,  6,   13,  1006,  1006,  41,  23,  934,  998,   304},	// alpha = 0.20
	};

	int i;
	U32BITS AlphaValue;

	// Set demod alpha mode with alpha value table.
	for (i = 0; i < REALTEK_R_QAM_ALPHA_VALUE_TABLE_LEN; i++) {
		// Get alpha value from alpha value entry according to alpha mode.
		AlphaValue = AlphaValueTable[AlphaMode][i];

		// Set alpha register selection.
		//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_SPEC_ALPHA_SEL, i) != FUNCTION_SUCCESS)
		rtd_maskl(DVBC_RXMF_WR_STB_reg, ~DVBC_RXMF_WR_STB_rxmf_sel_mask, DVBC_RXMF_WR_STB_rxmf_sel(i));

		// Set alpha register value.
		//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_SPEC_ALPHA_VAL, AlphaValue) != FUNCTION_SUCCESS)
		rtd_maskl(DVBC_RXMF_WR_STB_reg, ~DVBC_RXMF_WR_STB_rxmf_coeff_2_0_mask, DVBC_RXMF_WR_STB_rxmf_coeff_2_0(AlphaValue));
		rtd_maskl(DVBC_RXMF_WR_STB_1_reg, ~DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3_mask, DVBC_RXMF_WR_STB_1_rxmf_coeff_9_3((AlphaValue) >> 3));

		// Set alpha register strobe.
		// Note: REALTEK_R QAM hardware will clear strobe automatically.
		//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_SPEC_ALPHA_STROBE, ON) != FUNCTION_SUCCESS)
		rtd_maskl(DVBC_RXMF_WR_STB_reg, ~DVBC_RXMF_WR_STB_rxmf_wr_stb_mask, DVBC_RXMF_WR_STB_rxmf_wr_stb(0x1));
	}

	// Set demod alpha mode parameter.
	pDemod->AlphaMode      = AlphaMode;
	pDemod->IsAlphaModeSet = YES;

	return FUNCTION_SUCCESS;
}


/**

@see   QAM_DEMOD_FP_IS_FRAME_LOCKED

*/
int
realtek_r_dvbc_IsFrameLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	U32BITS LossStatus;

	// Get frame loss status from SYNCLOST register bits.
	//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_FS_SYNC_STROBE, ON) != FUNCTION_SUCCESS)
	rtd_maskl(DVBC_VT_LSB_HD_reg, ~DVBC_VT_LSB_HD_viterbi_rd_stb_mask, DVBC_VT_LSB_HD_viterbi_rd_stb(0x1));

	//if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_SYNCLOST, &LossStatus) != FUNCTION_SUCCESS)
	LossStatus = DVBC_OPT_FREEZE_get_synclost(rtd_inl(DVBC_OPT_FREEZE_reg));

	// Determine answer according to frame loss status.
	if (LossStatus == NOT_LOST)
		*pAnswer = YES;
	else
		*pAnswer = NO;

	return FUNCTION_SUCCESS;
}


int
realtek_r_dvbc_IsTrLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	S32BITS tr_ld_o;//, cr_ld_o;

	tr_ld_o = DVBC_INRX_MO2_1_get_inrx_mo2_12_7(rtd_inl(DVBC_INRX_MO2_1_reg));
	REALTEK_R_QAM_BASE_INFO("tr_ld_o = "PT_U32BITS"\n", tr_ld_o);
	// Determine answer according to tr_ld_o.
	if (tr_ld_o >= 32)
		//if((tr_ld_o > 1)||(cr_ld_o > 1))
		*pAnswer = NO;
	else
		*pAnswer = YES;

	return FUNCTION_SUCCESS;
}


int
realtek_r_dvbc_IsCrLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	S32BITS cr_ld_o;

	// Get TR lock status from register bits.
	//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_INNER_DATA_STROBE, ON) != FUNCTION_SUCCESS)
	rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));

	//if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_CR_LD, &cr_ld_o) != FUNCTION_SUCCESS)
	cr_ld_o = (((rtd_inl(DVBC_INRX_MO2_1_reg) & 0xe0) >> 5) | ((rtd_inl(DVBC_INRX_MO2_2_reg) & 0x7) << 3)) ;

	if (cr_ld_o > 0x1f) cr_ld_o = 0x20 - cr_ld_o;

	// Determine answer according to cr_ld_o.
	if (cr_ld_o > 1)
		*pAnswer = YES;
	else
		*pAnswer = NO;

	return FUNCTION_SUCCESS;
}


int
realtek_r_dvbc_IsInnerLocked(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	U32BITS fsm_state_o;

	// Get FSM state status from register bits.
	//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_INNER_DATA_STROBE, ON) != FUNCTION_SUCCESS)
	rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));

	//if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_FSM_STATE, &fsm_state_o) != FUNCTION_SUCCESS)
	fsm_state_o = (rtd_inl(DVBC_INRX_MO2_reg) & 0x78) >> 3;

	// Determine answer according to fsm_state_o.
	if (fsm_state_o == 12)
		*pAnswer = YES;
	else
		*pAnswer = NO;

	return FUNCTION_SUCCESS;
}


int
realtek_r_dvbc_IsNoSig(
	QAM_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	int ret;
	ret = realtek_r_dvbc_IsTrLocked(pDemod, pAnswer);
	*pAnswer = (*pAnswer) ? YES : NO;

	return ret;
}


/**

@see   QAM_DEMOD_FP_GET_ERROR_RATE

*/
int
realtek_r_dvbc_GetErrorRate(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS TestVolume,
	unsigned int WaitTimeMsMax,
	U32BITS *pBerNum,
	U32BITS *pBerDen,
	U32BITS *pPerNum,
	U32BITS *pPerDen
)
{
	BASE_INTERFACE_MODULE *pBaseInterface;

	unsigned int i;
	U32BITS TestPacketNum;
	unsigned int  WaitCnt;
	int FrameLock;
	U32BITS BerReg2;
	U32BITS BerReg0, BerReg1;
	U32BITS Reg0Data_1, Reg0Data_2, Reg1Data_1, Reg1Data_2, Reg1Data_3, Reg2Data_1, Reg2Data_2, Reg2Data_3;
	unsigned int TestVolumeNoWait = 5;

	// Get base interface.
	pBaseInterface = pDemod->pBaseInterface;


	if (TestVolume != 0) {
		// Calculate test packet number and wait counter value.
		TestPacketNum = 0x1 << (TestVolume * 2 + 4);
		WaitCnt = WaitTimeMsMax / REALTEK_R_QAM_BER_WAIT_TIME_MS;

		// Set TEST_VOLUME with test volume.
		//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_DVBC_TEST_VOLUME, TestVolume) != FUNCTION_SUCCESS)
		rtd_maskl(DVBC_OPT_BER_EN_reg, ~DVBC_OPT_BER_EN_opt_volume_mask, DVBC_OPT_BER_EN_opt_volume(TestVolume));

		// Clear and enable error counter.
		//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_DVBC_BERT_EN, OFF) != FUNCTION_SUCCESS)
		rtd_maskl(DVBC_OPT_BER_EN_reg, ~DVBC_OPT_BER_EN_opt_ber_en_mask, DVBC_OPT_BER_EN_opt_ber_en(0x0));

		//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_DVBC_BERT_EN, ON) != FUNCTION_SUCCESS)
		rtd_maskl(DVBC_OPT_BER_EN_reg, ~DVBC_OPT_BER_EN_opt_ber_en_mask, DVBC_OPT_BER_EN_opt_ber_en(0x1));

		rtd_maskl(DVBC_OPT_BER_EN_reg, ~DVBC_OPT_BER_EN_ber_en_stb_mask, DVBC_OPT_BER_EN_ber_en_stb(0x1));

		// Check if error test is finished.
		for (i = 0; i < WaitCnt; i++) {
			// Check if demod is frame-locked.
			if (pDemod->IsFrameLocked(pDemod, &FrameLock) != FUNCTION_SUCCESS) {
				goto error_status_get_demod_registers;
			}

			if (FrameLock == NO) {
				goto error_status_frame_lock;
			}

			// Wait a minute.
			// Note: The input unit of WaitMs() is ms.
			pBaseInterface->WaitMs(pBaseInterface, REALTEK_R_QAM_BER_WAIT_TIME_MS);

			// Set error counter strobe.
			// Note: REALTEK_R QAM hardware will clear strobe automatically.
			//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_DVBC_BER_RD_STROBE, ON) != FUNCTION_SUCCESS)
			//rtd_maskl(DVBC_OPT_RSD_DISABLE_reg, ~DVBC_OPT_RSD_DISABLE_opt_ber_rd_strobe_mask, DVBC_OPT_RSD_DISABLE_opt_ber_rd_strobe(ON));

			//IC net wrong connection by DIC
			rtd_maskl(DVBC_OPT_RSD_DISABLE_reg, ~DVBC_OPT_RSD_DISABLE_opt_ber_rd_strobe_mask, DVBC_OPT_RSD_DISABLE_opt_ber_rd_strobe(0x1));

			// Check if error test is finished.
			//if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_DVBC_BER_REG2_15_0, &BerReg2Lsb) != FUNCTION_SUCCESS)
			Reg2Data_1 = DVBC_BLOCK_COUNT_get_block_count_7_0(rtd_inl(DVBC_BLOCK_COUNT_reg));
			Reg2Data_2 = DVBC_BLOCK_COUNT_1_get_block_count_15_8(rtd_inl(DVBC_BLOCK_COUNT_1_reg));
			Reg2Data_3 = DVBC_BER_COUNT_2_get_block_count_18_16(rtd_inl(DVBC_BER_COUNT_2_reg));

			BerReg2 = (Reg2Data_3 << 16) | (Reg2Data_2 << 8) | Reg2Data_1;

			if (BerReg2 == TestPacketNum)
				break;
		}

		// Check time-out status.
		if (i == WaitCnt) {
			goto error_status_time_out;
		}

	} else {


		// Check if demod is frame-locked.
		if (pDemod->IsFrameLocked(pDemod, &FrameLock) != FUNCTION_SUCCESS) {
			goto error_status_get_demod_registers;
		}

		if (FrameLock == NO) {
			goto error_status_frame_lock;
		}


		rtd_maskl(DVBC_OPT_RSD_DISABLE_reg, ~DVBC_OPT_RSD_DISABLE_opt_ber_rd_strobe_mask, DVBC_OPT_RSD_DISABLE_opt_ber_rd_strobe(0x1));

		// Check if error test is finished.
		//if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_DVBC_BER_REG2_15_0, &BerReg2Lsb) != FUNCTION_SUCCESS)
		Reg2Data_1 = DVBC_BLOCK_COUNT_get_block_count_7_0(rtd_inl(DVBC_BLOCK_COUNT_reg));
		Reg2Data_2 = DVBC_BLOCK_COUNT_1_get_block_count_15_8(rtd_inl(DVBC_BLOCK_COUNT_1_reg));
		Reg2Data_3 = DVBC_BER_COUNT_2_get_block_count_18_16(rtd_inl(DVBC_BER_COUNT_2_reg));

		BerReg2 = (Reg2Data_3 << 16) | (Reg2Data_2 << 8) | Reg2Data_1;

		TestPacketNum = BerReg2;


	}


	// Get BER register 0 from BER_REG0.
	//if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_DVBC_BER_REG0, &BerReg0) != FUNCTION_SUCCESS)
	Reg0Data_1 = 0, Reg0Data_2 = 0;
	Reg0Data_1 = DVBC_UNCORR_COUNT_get_uncorr_count_7_0(rtd_inl(DVBC_UNCORR_COUNT_reg));
	Reg0Data_2 = (DVBC_UNCORR_COUNT_1_get_uncorr_count_15_8(rtd_inl(DVBC_UNCORR_COUNT_1_reg)) << 8);
	BerReg0 = (Reg0Data_1 + Reg0Data_2);

	// Get BER register 1 from BER_REG1.
	//if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, QAM_DVBC_BER_REG1, &BerReg1) != FUNCTION_SUCCESS)
	Reg1Data_1 = 0, Reg1Data_2 = 0, Reg1Data_1 = 0;
	Reg1Data_1 = DVBC_BER_COUNT_get_ber_count_7_0(rtd_inl(DVBC_BER_COUNT_reg));
	Reg1Data_2 = (DVBC_BER_COUNT_1_get_ber_count_15_8(rtd_inl(DVBC_BER_COUNT_1_reg)) << 8);
	Reg1Data_3 = (DVBC_BER_COUNT_2_get_ber_count_20_16(rtd_inl(DVBC_BER_COUNT_2_reg)) << 16);
	BerReg1 = (Reg1Data_1 + Reg1Data_2 + Reg1Data_3);

	// Set BER numerator and denominator.
	*pBerNum = 27 * BerReg0 + BerReg1;
	*pBerDen = 1632 * TestPacketNum;

	// Set PER numerator and denominator.
	*pPerNum = BerReg0;
	*pPerDen = TestPacketNum;


	// Set TEST_VOLUME with test volume.
	rtd_maskl(DVBC_OPT_BER_EN_reg, ~DVBC_OPT_BER_EN_opt_volume_mask, DVBC_OPT_BER_EN_opt_volume(TestVolumeNoWait));
	// Clear and enable error counter.
	rtd_maskl(DVBC_OPT_BER_EN_reg, ~DVBC_OPT_BER_EN_opt_ber_en_mask, DVBC_OPT_BER_EN_opt_ber_en(0x0));
	rtd_maskl(DVBC_OPT_BER_EN_reg, ~DVBC_OPT_BER_EN_opt_ber_en_mask, DVBC_OPT_BER_EN_opt_ber_en(0x1));
	rtd_maskl(DVBC_OPT_BER_EN_reg, ~DVBC_OPT_BER_EN_ber_en_stb_mask, DVBC_OPT_BER_EN_ber_en_stb(0x1));
	return FUNCTION_SUCCESS;

error_status_get_demod_registers:
error_status_frame_lock:
error_status_time_out:

	// Set TEST_VOLUME with test volume.
	rtd_maskl(DVBC_OPT_BER_EN_reg, ~DVBC_OPT_BER_EN_opt_volume_mask, DVBC_OPT_BER_EN_opt_volume(TestVolumeNoWait));
	rtd_maskl(DVBC_OPT_BER_EN_reg, ~DVBC_OPT_BER_EN_opt_ber_en_mask, DVBC_OPT_BER_EN_opt_ber_en(0x0));
	rtd_maskl(DVBC_OPT_BER_EN_reg, ~DVBC_OPT_BER_EN_opt_ber_en_mask, DVBC_OPT_BER_EN_opt_ber_en(0x1));
	rtd_maskl(DVBC_OPT_BER_EN_reg, ~DVBC_OPT_BER_EN_ber_en_stb_mask, DVBC_OPT_BER_EN_ber_en_stb(0x1));

	return FUNCTION_ERROR;
}


/**

@see   QAM_DEMOD_FP_UPDATE_FUNCTION

*/
int
realtek_r_dvbc_UpdateFunction(
	QAM_DEMOD_MODULE *pDemod
)
{
	return FUNCTION_SUCCESS;
}


/**

@see   QAM_DEMOD_FP_RESET_FUNCTION

*/
int
realtek_r_dvbc_ResetFunction(
	QAM_DEMOD_MODULE *pDemod
)
{
	// REALTEK_R DVB-C does not use UpdateFunction(), so we just return FUNCTION_SUCCESS.
	return FUNCTION_SUCCESS;
}


/**

@see   QAM_DEMOD_FP_NBD_SRS

*/
int
realtek_r_dvbc_NbdAndSrs(
	QAM_DEMOD_MODULE *pDemod,
	unsigned char *pNosig,
	U32BITS *pPowerDiff,
	U32BITS *pSymbolRate,
	unsigned char WaitAgcStableFlag
)
{
	unsigned long stime = 0;
	U32BITS Srs_nbd_done = 0, pow_diff = 0, PosToneDetect = 0, CrystalFreqHz = 0;
	int i = 0;//, aagc_ld=0;
	unsigned int PosToneMaxIdx[3], NegToneMaxIdx[3];
	unsigned int SrsWeight[3], SrsWeightMax = 0;
	unsigned int delay = pDemod->Symbolratedelay;

	//turn off HW reset
	rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~(DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask)  , DVBC_OPT_RST_STATE0_ON_opt_retry_on(0));

	if (WaitAgcStableFlag) {
		pDemod->SoftwareReset(pDemod);
		if (delay != 0)
			tv_osal_msleep(delay);
	}

	rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_start_srs_nbd_mask, DVBC_OPT_START_SRS_NBD_opt_start_srs_nbd(0));
	rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_start_srs_nbd_mask, DVBC_OPT_START_SRS_NBD_opt_start_srs_nbd(1));

	stime = tv_osal_time();
	while ((tv_osal_time() - stime) <= 80) { //nbd wait 50ms per times
		Srs_nbd_done = DVBC_SRS_NBD_DONE_get_srs_nbd_done(rtd_inl(DVBC_SRS_NBD_DONE_reg));  //QAMD_DONE
		tv_osal_msleep(5);
		REALTEK_R_QAM_BASE_DBG("[NBD&SYS] Srs_nbd_done="PT_U32BITS", time=%lu\n", Srs_nbd_done, tv_osal_time() - stime);
		if (Srs_nbd_done == 1) break;
	}

	if (Srs_nbd_done != 1) {
		REALTEK_R_QAM_BASE_WARNING("[Nbd and Srs] NBD and SRS checked TIMEOUT...... \n");
		return FUNCTION_ERROR;
	}

	pow_diff = (DVBC_SOLT_AVG_POW_DIF_1_get_solt_avg_pow_dif_15_8(rtd_inl(DVBC_SOLT_AVG_POW_DIF_1_reg)) << 8) | DVBC_SOLT_AVG_POW_DIF_get_solt_avg_pow_dif_7_0(rtd_inl(DVBC_SOLT_AVG_POW_DIF_reg));
	PosToneMaxIdx[0] = (DVBC_POS_TONE_MAX_IDX_0_1_get_pos_tone_max_idx_0_11_8(rtd_inl(DVBC_POS_TONE_MAX_IDX_0_1_reg)) << 8) | DVBC_POS_TONE_MAX_IDX_0_get_pos_tone_max_idx_0_7_0(rtd_inl(DVBC_POS_TONE_MAX_IDX_0_reg));
	PosToneMaxIdx[1] = (DVBC_POS_TONE_MAX_IDX_1_1_get_pos_tone_max_idx_1_11_8(rtd_inl(DVBC_POS_TONE_MAX_IDX_1_1_reg)) << 8) | DVBC_POS_TONE_MAX_IDX_1_get_pos_tone_max_idx_1_7_0(rtd_inl(DVBC_POS_TONE_MAX_IDX_1_reg));
	PosToneMaxIdx[2] = (DVBC_POS_TONE_MAX_IDX_2_1_get_pos_tone_max_idx_2_11_8(rtd_inl(DVBC_POS_TONE_MAX_IDX_2_1_reg)) << 8) | DVBC_POS_TONE_MAX_IDX_2_get_pos_tone_max_idx_2_7_0(rtd_inl(DVBC_POS_TONE_MAX_IDX_2_reg));
	NegToneMaxIdx[0] = (DVBC_NEG_TONE_MAX_IDX_0_1_get_neg_tone_max_idx_0_11_8(rtd_inl(DVBC_NEG_TONE_MAX_IDX_0_1_reg)) << 8) | DVBC_NEG_TONE_MAX_IDX_0_get_neg_tone_max_idx_0_7_0(rtd_inl(DVBC_NEG_TONE_MAX_IDX_0_reg));
	NegToneMaxIdx[1] = (DVBC_NEG_TONE_MAX_IDX_1_1_get_neg_tone_max_idx_1_11_8(rtd_inl(DVBC_NEG_TONE_MAX_IDX_1_1_reg)) << 8) | DVBC_NEG_TONE_MAX_IDX_1_get_neg_tone_max_idx_1_7_0(rtd_inl(DVBC_NEG_TONE_MAX_IDX_1_reg));
	NegToneMaxIdx[2] = (DVBC_NEG_TONE_MAX_IDX_2_1_get_neg_tone_max_idx_2_11_8(rtd_inl(DVBC_NEG_TONE_MAX_IDX_2_1_reg)) << 8) | DVBC_NEG_TONE_MAX_IDX_2_get_neg_tone_max_idx_2_7_0(rtd_inl(DVBC_NEG_TONE_MAX_IDX_2_reg));

	pDemod->GetCrystalFreqHz(pDemod, &CrystalFreqHz);

	if (PosToneMaxIdx[0] > 0) {
		if (ABS(PosToneMaxIdx[0], NegToneMaxIdx[0]) < 3) {
			SrsWeight[0] = 6;
		} else if (ABS(PosToneMaxIdx[0], NegToneMaxIdx[1]) < 3) {
			SrsWeight[0] = 5;
		} else if (ABS(PosToneMaxIdx[0], NegToneMaxIdx[2]) < 3) {
			SrsWeight[0] = 4;
		} else {
			SrsWeight[0] = 0;
		}
	} else {
		SrsWeight[0] = 0;
	}

	REALTEK_R_QAM_BASE_DBG("[NBD&SYS] PosToneMaxIdx[0] = %u, NegToneMaxIdx[0] = %u, SrsWeight[0] = %u \n", PosToneMaxIdx[0], NegToneMaxIdx[0], SrsWeight[0]);

	if (PosToneMaxIdx[1] > 0) {
		if (ABS(PosToneMaxIdx[1], NegToneMaxIdx[0]) < 3) {
			SrsWeight[1] = 5;
		} else if (ABS(PosToneMaxIdx[1], NegToneMaxIdx[1]) < 3) {
			SrsWeight[1] = 4;
		} else if (ABS(PosToneMaxIdx[1], NegToneMaxIdx[2]) < 3) {
			SrsWeight[1] = 3;
		} else {
			SrsWeight[1] = 0;
		}
	} else {
		SrsWeight[1] = 0;
	}

	REALTEK_R_QAM_BASE_DBG("[NBD&SYS] PosToneMaxIdx[1] = %u, NegToneMaxIdx[1] = %u, SrsWeight[1] = %u \n", PosToneMaxIdx[1], NegToneMaxIdx[1], SrsWeight[1]);

	if (PosToneMaxIdx[2] > 0) {
		if (ABS(PosToneMaxIdx[2], NegToneMaxIdx[0]) < 3) {
			SrsWeight[2] = 4;
		} else if (ABS(PosToneMaxIdx[2], NegToneMaxIdx[1]) < 3) {
			SrsWeight[2] = 3;
		} else if (ABS(PosToneMaxIdx[2], NegToneMaxIdx[2]) < 3) {
			SrsWeight[2] = 2;
		} else {
			SrsWeight[2] = 0;
		}
	} else {
		SrsWeight[2] = 0;
	}

	REALTEK_R_QAM_BASE_DBG("[NBD&SYS] PosToneMaxIdx[2] = %u, NegToneMaxIdx[2] = %u, SrsWeight[2] = %u \n", PosToneMaxIdx[2], NegToneMaxIdx[2], SrsWeight[2]);

	for (i = 0; i < 3; i++) {

		if (SrsWeight[i] > SrsWeightMax) {
			SrsWeightMax = SrsWeight[i];
			PosToneDetect = PosToneMaxIdx[i];
		}

	}

	if (SrsWeightMax == 0) {
		*pNosig = 1;
		*pPowerDiff = pow_diff;
		*pSymbolRate = 0; //No signal

	} else {
		*pNosig = 0;
		*pPowerDiff = pow_diff;
		*pSymbolRate = (PosToneDetect * (CrystalFreqHz >> 10)) >> 4;
		//symbol_rate_det = (ADC_freq/2) * (pos_frq_tone_idx / 8192);	//fft_size :8192
	}

	REALTEK_R_QAM_BASE_DBG("[NBD&SYS] pNosig = %u, pPowerDiff = "PT_U32BITS", pSymbolRate = "PT_U32BITS" \n", *pNosig, *pPowerDiff, *pSymbolRate);

	//turn on HW reset
	rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~(DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask)  , DVBC_OPT_RST_STATE0_ON_opt_retry_on(1));

	return FUNCTION_SUCCESS;

}


/**

@see   QAM_DEMOD_FP_QAMD

*/
int
realtek_r_dvbc_Qamd(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS *pQammode,
	unsigned char MetricEn
)
{
	unsigned long stime;
	U32BITS reg_qamd_done = 0;
	int detect_dataZ0 = 0, detect_dataZ1 = 0, detect_dataZ2 = 0, detect_dataZ3 = 0, detect_dataZ4 = 0, detect_dataZ5 = 0;

	//fix FSM to 5
	//rtd_maskl(DVBC_FSM_FIX_reg, ~(DVBC_FSM_FIX_opt_fsm_fix_ste_en_mask | DVBC_FSM_FIX_opt_fsm_fix_ste_end_mask)  , (DVBC_FSM_FIX_opt_fsm_fix_ste_en(1) | DVBC_FSM_FIX_opt_fsm_fix_ste_end(5)));

	if (MetricEn == 1) {
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN0_MAX_reg, ~(DVBC_OPT_QAMD_EQP_ZN0_MAX_opt_qamd_eqp_zn0_max_7_0_mask), DVBC_OPT_QAMD_EQP_ZN0_MAX_opt_qamd_eqp_zn0_max_7_0(0x0c));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN0_MAX_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN0_MAX_1_opt_qamd_eqp_zn0_max_9_8_mask), DVBC_OPT_QAMD_EQP_ZN0_MAX_1_opt_qamd_eqp_zn0_max_9_8(0x0));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN0_MIN_reg, ~(DVBC_OPT_QAMD_EQP_ZN0_MIN_opt_qamd_eqp_zn0_min_7_0_mask), DVBC_OPT_QAMD_EQP_ZN0_MIN_opt_qamd_eqp_zn0_min_7_0(0x04));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN0_MIN_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN0_MIN_1_opt_qamd_eqp_zn0_min_9_8_mask), DVBC_OPT_QAMD_EQP_ZN0_MIN_1_opt_qamd_eqp_zn0_min_9_8(0x0));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN1_MAX_reg, ~(DVBC_OPT_QAMD_EQP_ZN1_MAX_opt_qamd_eqp_zn1_max_7_0_mask), DVBC_OPT_QAMD_EQP_ZN1_MAX_opt_qamd_eqp_zn1_max_7_0(0x18));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN1_MAX_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN1_MAX_1_opt_qamd_eqp_zn1_max_9_8_mask), DVBC_OPT_QAMD_EQP_ZN1_MAX_1_opt_qamd_eqp_zn1_max_9_8(0x0));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN1_MIN_reg, ~(DVBC_OPT_QAMD_EQP_ZN1_MIN_opt_qamd_eqp_zn1_min_7_0_mask), DVBC_OPT_QAMD_EQP_ZN1_MIN_opt_qamd_eqp_zn1_min_7_0(0x10));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN1_MIN_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN1_MIN_1_opt_qamd_eqp_zn1_min_9_8_mask), DVBC_OPT_QAMD_EQP_ZN1_MIN_1_opt_qamd_eqp_zn1_min_9_8(0x0));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN2_MAX_reg, ~(DVBC_OPT_QAMD_EQP_ZN2_MAX_opt_qamd_eqp_zn2_max_7_0_mask), DVBC_OPT_QAMD_EQP_ZN2_MAX_opt_qamd_eqp_zn2_max_7_0(0x2a));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN2_MAX_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN2_MAX_1_opt_qamd_eqp_zn2_max_9_8_mask), DVBC_OPT_QAMD_EQP_ZN2_MAX_1_opt_qamd_eqp_zn2_max_9_8(0x0));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN2_MIN_reg, ~(DVBC_OPT_QAMD_EQP_ZN2_MIN_opt_qamd_eqp_zn2_min_7_0_mask), DVBC_OPT_QAMD_EQP_ZN2_MIN_opt_qamd_eqp_zn2_min_7_0(0x22));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN2_MIN_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN2_MIN_1_opt_qamd_eqp_zn2_min_9_8_mask), DVBC_OPT_QAMD_EQP_ZN2_MIN_1_opt_qamd_eqp_zn2_min_9_8(0x0));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN3_MAX_reg, ~(DVBC_OPT_QAMD_EQP_ZN3_MAX_opt_qamd_eqp_zn3_max_7_0_mask), DVBC_OPT_QAMD_EQP_ZN3_MAX_opt_qamd_eqp_zn3_max_7_0(0x45));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN3_MAX_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN3_MAX_1_opt_qamd_eqp_zn3_max_9_8_mask), DVBC_OPT_QAMD_EQP_ZN3_MAX_1_opt_qamd_eqp_zn3_max_9_8(0x0));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN3_MIN_reg, ~(DVBC_OPT_QAMD_EQP_ZN3_MIN_opt_qamd_eqp_zn3_min_7_0_mask), DVBC_OPT_QAMD_EQP_ZN3_MIN_opt_qamd_eqp_zn3_min_7_0(0x3d));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN3_MIN_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN3_MIN_1_opt_qamd_eqp_zn3_min_9_8_mask), DVBC_OPT_QAMD_EQP_ZN3_MIN_1_opt_qamd_eqp_zn3_min_9_8(0x0));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN4_MAX_reg, ~(DVBC_OPT_QAMD_EQP_ZN4_MAX_opt_qamd_eqp_zn4_max_7_0_mask), DVBC_OPT_QAMD_EQP_ZN4_MAX_opt_qamd_eqp_zn4_max_7_0(0xa0));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN4_MAX_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN4_MAX_1_opt_qamd_eqp_zn4_max_9_8_mask), DVBC_OPT_QAMD_EQP_ZN4_MAX_1_opt_qamd_eqp_zn4_max_9_8(0x0));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN4_MIN_reg, ~(DVBC_OPT_QAMD_EQP_ZN4_MIN_opt_qamd_eqp_zn4_min_7_0_mask), DVBC_OPT_QAMD_EQP_ZN4_MIN_opt_qamd_eqp_zn4_min_7_0(0x98));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN4_MIN_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN4_MIN_1_opt_qamd_eqp_zn4_min_9_8_mask), DVBC_OPT_QAMD_EQP_ZN4_MIN_1_opt_qamd_eqp_zn4_min_9_8(0x0));

	} else {
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN0_MAX_reg, ~(DVBC_OPT_QAMD_EQP_ZN0_MAX_opt_qamd_eqp_zn0_max_7_0_mask), DVBC_OPT_QAMD_EQP_ZN0_MAX_opt_qamd_eqp_zn0_max_7_0(0x30));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN0_MAX_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN0_MAX_1_opt_qamd_eqp_zn0_max_9_8_mask), DVBC_OPT_QAMD_EQP_ZN0_MAX_1_opt_qamd_eqp_zn0_max_9_8(0x0));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN0_MIN_reg, ~(DVBC_OPT_QAMD_EQP_ZN0_MIN_opt_qamd_eqp_zn0_min_7_0_mask), DVBC_OPT_QAMD_EQP_ZN0_MIN_opt_qamd_eqp_zn0_min_7_0(0x20));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN0_MIN_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN0_MIN_1_opt_qamd_eqp_zn0_min_9_8_mask), DVBC_OPT_QAMD_EQP_ZN0_MIN_1_opt_qamd_eqp_zn0_min_9_8(0x0));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN1_MAX_reg, ~(DVBC_OPT_QAMD_EQP_ZN1_MAX_opt_qamd_eqp_zn1_max_7_0_mask), DVBC_OPT_QAMD_EQP_ZN1_MAX_opt_qamd_eqp_zn1_max_7_0(0x58));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN1_MAX_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN1_MAX_1_opt_qamd_eqp_zn1_max_9_8_mask), DVBC_OPT_QAMD_EQP_ZN1_MAX_1_opt_qamd_eqp_zn1_max_9_8(0x0));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN1_MIN_reg, ~(DVBC_OPT_QAMD_EQP_ZN1_MIN_opt_qamd_eqp_zn1_min_7_0_mask), DVBC_OPT_QAMD_EQP_ZN1_MIN_opt_qamd_eqp_zn1_min_7_0(0x48));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN1_MIN_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN1_MIN_1_opt_qamd_eqp_zn1_min_9_8_mask), DVBC_OPT_QAMD_EQP_ZN1_MIN_1_opt_qamd_eqp_zn1_min_9_8(0x0));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN2_MAX_reg, ~(DVBC_OPT_QAMD_EQP_ZN2_MAX_opt_qamd_eqp_zn2_max_7_0_mask), DVBC_OPT_QAMD_EQP_ZN2_MAX_opt_qamd_eqp_zn2_max_7_0(0xa8));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN2_MAX_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN2_MAX_1_opt_qamd_eqp_zn2_max_9_8_mask), DVBC_OPT_QAMD_EQP_ZN2_MAX_1_opt_qamd_eqp_zn2_max_9_8(0x0));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN2_MIN_reg, ~(DVBC_OPT_QAMD_EQP_ZN2_MIN_opt_qamd_eqp_zn2_min_7_0_mask), DVBC_OPT_QAMD_EQP_ZN2_MIN_opt_qamd_eqp_zn2_min_7_0(0x98));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN2_MIN_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN2_MIN_1_opt_qamd_eqp_zn2_min_9_8_mask), DVBC_OPT_QAMD_EQP_ZN2_MIN_1_opt_qamd_eqp_zn2_min_9_8(0x0));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN3_MAX_reg, ~(DVBC_OPT_QAMD_EQP_ZN3_MAX_opt_qamd_eqp_zn3_max_7_0_mask), DVBC_OPT_QAMD_EQP_ZN3_MAX_opt_qamd_eqp_zn3_max_7_0(0xe8));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN3_MAX_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN3_MAX_1_opt_qamd_eqp_zn3_max_9_8_mask), DVBC_OPT_QAMD_EQP_ZN3_MAX_1_opt_qamd_eqp_zn3_max_9_8(0x0));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN3_MIN_reg, ~(DVBC_OPT_QAMD_EQP_ZN3_MIN_opt_qamd_eqp_zn3_min_7_0_mask), DVBC_OPT_QAMD_EQP_ZN3_MIN_opt_qamd_eqp_zn3_min_7_0(0xd8));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN3_MIN_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN3_MIN_1_opt_qamd_eqp_zn3_min_9_8_mask), DVBC_OPT_QAMD_EQP_ZN3_MIN_1_opt_qamd_eqp_zn3_min_9_8(0x0));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN4_MAX_reg, ~(DVBC_OPT_QAMD_EQP_ZN4_MAX_opt_qamd_eqp_zn4_max_7_0_mask), DVBC_OPT_QAMD_EQP_ZN4_MAX_opt_qamd_eqp_zn4_max_7_0(0x10));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN4_MAX_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN4_MAX_1_opt_qamd_eqp_zn4_max_9_8_mask), DVBC_OPT_QAMD_EQP_ZN4_MAX_1_opt_qamd_eqp_zn4_max_9_8(0x1));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN4_MIN_reg, ~(DVBC_OPT_QAMD_EQP_ZN4_MIN_opt_qamd_eqp_zn4_min_7_0_mask), DVBC_OPT_QAMD_EQP_ZN4_MIN_opt_qamd_eqp_zn4_min_7_0(0x00));
		rtd_maskl(DVBC_OPT_QAMD_EQP_ZN4_MIN_1_reg, ~(DVBC_OPT_QAMD_EQP_ZN4_MIN_1_opt_qamd_eqp_zn4_min_9_8_mask), DVBC_OPT_QAMD_EQP_ZN4_MIN_1_opt_qamd_eqp_zn4_min_9_8(0x1));

	}


	//pDemod->SoftwareReset(pDemod);
	//tv_osal_msleep(30);
	rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask, DVBC_OPT_RST_STATE0_ON_opt_retry_on(0));

	stime = tv_osal_time();

	rtd_maskl(DVBC_OPT_QAMD_SELF_RESET_reg, ~DVBC_OPT_QAMD_SELF_RESET_opt_qamd_self_reset_mask, 0x0); //reset auto qam
	rtd_maskl(DVBC_OPT_QAMD_SELF_RESET_reg, ~DVBC_OPT_QAMD_SELF_RESET_opt_qamd_self_reset_mask, 0x1); //reset auto qam

	while ((tv_osal_time() - stime) <= 150) {
		tv_osal_msleep(5);
		rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, 0x1); //QAM_INNER_DATA_STROBE
		reg_qamd_done = DVBC_REG_QAMD_DONE_get_reg_qamd_done(rtd_inl(DVBC_REG_QAMD_DONE_reg));  //QAMD_DONE
		if (reg_qamd_done == 1) break;
	}
	REALTEK_R_QAM_BASE_DBG("[AUTO QAM] reg_qamd_done="PT_U32BITS"; time=%lu\n", reg_qamd_done, tv_osal_time() - stime);

	if (reg_qamd_done == 1) {
		detect_dataZ0 = (DVBC_QAMD_RV_ZN0_CDF_1_get_qamd_rv_zn0_cdf_9_8(rtd_inl(DVBC_QAMD_RV_ZN0_CDF_1_reg)) << 8)  | DVBC_QAMD_RV_ZN0_CDF_get_qamd_rv_zn0_cdf_7_0(rtd_inl(DVBC_QAMD_RV_ZN0_CDF_reg));
		detect_dataZ1 = (DVBC_QAMD_RV_ZN1_CDF_1_get_qamd_rv_zn1_cdf_9_8(rtd_inl(DVBC_QAMD_RV_ZN1_CDF_1_reg)) << 8)  | DVBC_QAMD_RV_ZN1_CDF_get_qamd_rv_zn1_cdf_7_0(rtd_inl(DVBC_QAMD_RV_ZN1_CDF_reg));
		detect_dataZ2 = (DVBC_QAMD_RV_ZN2_CDF_1_get_qamd_rv_zn2_cdf_9_8(rtd_inl(DVBC_QAMD_RV_ZN2_CDF_1_reg)) << 8)  | DVBC_QAMD_RV_ZN2_CDF_get_qamd_rv_zn2_cdf_7_0(rtd_inl(DVBC_QAMD_RV_ZN2_CDF_reg));
		detect_dataZ3 = (DVBC_QAMD_RV_ZN3_CDF_1_get_qamd_rv_zn3_cdf_9_8(rtd_inl(DVBC_QAMD_RV_ZN3_CDF_1_reg)) << 8)  | DVBC_QAMD_RV_ZN3_CDF_get_qamd_rv_zn3_cdf_7_0(rtd_inl(DVBC_QAMD_RV_ZN3_CDF_reg));
		detect_dataZ4 = (DVBC_QAMD_RV_ZN4_CDF_1_get_qamd_rv_zn4_cdf_9_8(rtd_inl(DVBC_QAMD_RV_ZN4_CDF_1_reg)) << 8)  | DVBC_QAMD_RV_ZN4_CDF_get_qamd_rv_zn4_cdf_7_0(rtd_inl(DVBC_QAMD_RV_ZN4_CDF_reg));
		detect_dataZ5 = (DVBC_QAMD_RV_ZN5_CDF_1_get_qamd_rv_zn5_cdf_9_8(rtd_inl(DVBC_QAMD_RV_ZN5_CDF_1_reg)) << 8)  | DVBC_QAMD_RV_ZN5_CDF_get_qamd_rv_zn5_cdf_7_0(rtd_inl(DVBC_QAMD_RV_ZN5_CDF_reg));

		REALTEK_R_QAM_BASE_DBG("[Qamd] ZONEVALUE %d %d %d %d %d %d\n", detect_dataZ0, detect_dataZ1, detect_dataZ2, detect_dataZ3, detect_dataZ4, detect_dataZ5);
		REALTEK_R_QAM_BASE_DBG("[Qamd] detect_dataZ0=%d, detect_dataZ1 = %d, detect_dataZ2 = %d, detect_dataZ3 = %d, detect_dataZ4 = %d, detect_dataZ5 = %d\n", detect_dataZ0, detect_dataZ1, detect_dataZ2, detect_dataZ3, detect_dataZ4, detect_dataZ5);

		if (MetricEn == 0) {
			REALTEK_R_QAM_BASE_DBG("[Qamd] d0 = %d, THR_16 = %d\n", ((detect_dataZ0 - detect_dataZ4) + (detect_dataZ2 - detect_dataZ3) + (detect_dataZ5 - detect_dataZ1)), QAMD_THD_16QAM);
			REALTEK_R_QAM_BASE_DBG("[Qamd] d1 = %d, THR_32 = %d\n", ((detect_dataZ1 - detect_dataZ0) + (detect_dataZ1 - detect_dataZ2) + (detect_dataZ3 - detect_dataZ2)), QAMD_THD_32QAM);
			REALTEK_R_QAM_BASE_DBG("[Qamd] d2 = %d, THR_64 = %d\n", ((detect_dataZ0 - detect_dataZ2) + (detect_dataZ0 - detect_dataZ3) + (detect_dataZ0 - detect_dataZ4) + (detect_dataZ4 - detect_dataZ2)), QAMD_THD_64QAM);
			REALTEK_R_QAM_BASE_DBG("[Qamd] d3 = %d, THR_128 = %d\n", detect_dataZ5, QAMD_THD_128QAM);
			REALTEK_R_QAM_BASE_DBG("[Qamd] d4 = %d, THR_256 = %d\n", ((detect_dataZ2 - detect_dataZ4) + (detect_dataZ3 - detect_dataZ0) + (detect_dataZ3 - detect_dataZ4)), QAMD_THD_256QAM);

			if (((detect_dataZ0 - detect_dataZ4) + (detect_dataZ2 - detect_dataZ3) + (detect_dataZ5 - detect_dataZ1)) > QAMD_THD_16QAM) {
				*pQammode = QAM_QAM_16;
				REALTEK_R_QAM_BASE_INFO("[AUTO QAM] Constellation = 16QAM\n");
			} else if (((detect_dataZ1 - detect_dataZ0) + (detect_dataZ1 - detect_dataZ2) + (detect_dataZ3 - detect_dataZ2)) > QAMD_THD_32QAM) {
				*pQammode = QAM_QAM_32;
				REALTEK_R_QAM_BASE_INFO("[AUTO QAM] Constellation = 32QAM\n");
			} else if ((((detect_dataZ0 - detect_dataZ2) + (detect_dataZ0 - detect_dataZ3) + (detect_dataZ0 - detect_dataZ4) + (detect_dataZ4 - detect_dataZ2)) > QAMD_THD_64QAM) && (detect_dataZ5 < 57)) {
				*pQammode = QAM_QAM_64;
				REALTEK_R_QAM_BASE_INFO("[AUTO QAM] Constellation = 64QAM\n");
			} else if (detect_dataZ5 < QAMD_THD_128QAM) {
				*pQammode = QAM_QAM_128;
				REALTEK_R_QAM_BASE_INFO("[AUTO QAM] Constellation = 128QAM\n");
			} else if ((((detect_dataZ2 - detect_dataZ4) + (detect_dataZ3 - detect_dataZ0) + (detect_dataZ3 - detect_dataZ4)) > QAMD_THD_256QAM) || (detect_dataZ5 > 40)) {
				*pQammode = QAM_QAM_256;
				REALTEK_R_QAM_BASE_INFO("[AUTO QAM] Constellation = 256QAM\n");
			} else {
				*pQammode = QAM_QAM_64;
				REALTEK_R_QAM_BASE_WARNING("[AUTO QAM] QAM mode detect Fail...... (set to dafault 64QAM)  \n");
			}

		} else {
			REALTEK_R_QAM_BASE_DBG("[AUTO QAM][metric_64_256] metric_64_256 = %d\n", ((detect_dataZ2 - detect_dataZ1) + (detect_dataZ2 - detect_dataZ3) + (detect_dataZ2 - detect_dataZ4) + (detect_dataZ0 - detect_dataZ1)));

			if (((detect_dataZ2 - detect_dataZ1) + (detect_dataZ2 - detect_dataZ3) + (detect_dataZ2 - detect_dataZ4) + (detect_dataZ0 - detect_dataZ1)) > 180) {
				*pQammode = QAM_QAM_64;
				REALTEK_R_QAM_BASE_INFO("[AUTO QAM][metric_64_256] Constellation = 64QAMn");
			} else {
				*pQammode = QAM_QAM_256;
				REALTEK_R_QAM_BASE_INFO("[AUTO QAM][metric_64_256] Constellation = 256QAM\n");
			}
		}

	} else {
		*pQammode = QAM_QAM_64;
		REALTEK_R_QAM_BASE_WARNING("[AUTO QAM] QAM mode detect Fail...... (set to dafault 64QAM)  \n");

	}

	rtd_maskl(DVBC_OPT_QAMD_SELF_RESET_reg, ~DVBC_OPT_QAMD_SELF_RESET_opt_qamd_self_reset_mask, 0x0); //reset auto qam
	//rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask, DVBC_OPT_RST_STATE0_ON_opt_retry_on(1));
	//rtd_maskl(DVBC_FSM_FIX_reg, ~(DVBC_FSM_FIX_opt_fsm_fix_ste_en_mask | DVBC_FSM_FIX_opt_fsm_fix_ste_end_mask)  , (DVBC_FSM_FIX_opt_fsm_fix_ste_en(0) | DVBC_FSM_FIX_opt_fsm_fix_ste_end(0)));

	if ((reg_qamd_done == 1) && (((detect_dataZ0 + detect_dataZ1) < 55) || (detect_dataZ0 == 0) || (detect_dataZ1 == 0))) {
		return FUNCTION_ERROR;
	}


	return FUNCTION_SUCCESS;
}


int
realtek_r_dvbc_QamRegPreset(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS SymbolRateHz
)
{

	rtd_maskl(DVBC_OPT_QAMD_SELF_RESET_reg, ~DVBC_OPT_QAMD_SELF_RESET_opt_qamd_self_reset_mask, 0x0); //reset auto qam

	rtd_maskl(DVBC_FSM_FIX_reg, ~(DVBC_FSM_FIX_opt_fsm_fix_ste_en_mask | DVBC_FSM_FIX_opt_fsm_fix_ste_end_mask)  , (DVBC_FSM_FIX_opt_fsm_fix_ste_en(1) | DVBC_FSM_FIX_opt_fsm_fix_ste_end(5)));
	rtd_maskl(DVBC_OPT_QAMD_START_STATE_reg, ~DVBC_OPT_QAMD_START_STATE_opt_qamd_start_state_mask, DVBC_OPT_QAMD_START_STATE_opt_qamd_start_state(0x5));


	/*

		if ((SymbolRateHz > 5010000) || (SymbolRateHz < 4510000)) {

			//rtd_part_outl(DVBC_FSM_WR_STB_reg, 7, 0, 0xc);	//0x15d=0x0c, FSM_WR_STB, [4:1]FSM state number=0x6(u_dagc_exp_fsm)
			rtd_maskl(DVBC_FSM_WR_STB_reg, ~0xff, 0x0c);
			//rtd_part_outl(DVBC_FSM_WORD_reg, 7, 0, 0x76);		//0x15e=0x76, fsm_word[7:0]
			rtd_maskl(DVBC_FSM_WORD_reg, ~0xff, 0x76);
			//rtd_part_outl(DVBC_FSM_WORD_1_reg, 7, 0, 0x77);	//0x15f=0x77, fsm_word[15:8]
			rtd_maskl(DVBC_FSM_WORD_1_reg, ~0xff, 0x77);
			//rtd_part_outl(DVBC_FSM_WORD_2_reg, 7, 0, 0xcb);	//0x160=0xcb, fsm_word[23:16]
			rtd_maskl(DVBC_FSM_WORD_2_reg, ~0xff, 0xcb);
			//rtd_part_outl(DVBC_FSM_WORD_3_reg, 7, 0, 0x9e);	//0x161=0x9e, fsm_word[31:24]
			rtd_maskl(DVBC_FSM_WORD_3_reg, ~0xff, 0x9e);
			//rtd_part_outl(DVBC_FSM_WORD_4_reg, 7, 0, 0xf7);	//0x162=0xf7, fsm_word[39:32]
			rtd_maskl(DVBC_FSM_WORD_4_reg, ~0xff, 0xf7);
			//rtd_part_outl(DVBC_FSM_WORD_5_reg, 7, 0, 0x4b);	//0x163=0x4b, fsm_word[47:40]
			rtd_maskl(DVBC_FSM_WORD_5_reg, ~0xff, 0x4b);
			//rtd_part_outl(DVBC_FSM_WORD_6_reg, 7, 0, 0x75);	//0x164=0x75, fsm_word[55:48]
			rtd_maskl(DVBC_FSM_WORD_6_reg, ~0xff, 0x75);
			//rtd_part_outl(DVBC_FSM_WORD_7_reg, 7, 0, 0x40);	//0x165=0x40, fsm_word[63:56]
			rtd_maskl(DVBC_FSM_WORD_7_reg, ~0xff, 0x40);
			//rtd_part_outl(DVBC_FSM_WORD_8_reg, 7, 0, 0xa4);	//0x166=0xa4, fsm_word[71:64]
			rtd_maskl(DVBC_FSM_WORD_8_reg, ~0xff, 0xa4);
			//rtd_part_outl(DVBC_FSM_WORD_9_reg, 7, 0, 0xf8);	//0x167=0xf8, fsm_word[79:72]
			rtd_maskl(DVBC_FSM_WORD_9_reg, ~0xff, 0xf8);
			//rtd_part_outl(DVBC_FSM_WORD_10_reg, 7, 0, 0x32);	//0x168=0x32, fsm_word[87:80]
			rtd_maskl(DVBC_FSM_WORD_10_reg, ~0xff, 0x32);
			//rtd_part_outl(DVBC_FSM_WORD_11_reg, 2, 0, 0x0);	//0x1cc=0x0, fsm_word[90:88]
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~0x07, 0x00);
			//rtd_part_outl(DVBC_FSM_WR_STB_reg, 7, 0, 0xd);	//0x15d=0x0d, FSM_WR_STB, [4:1]FSM state number=0x6,[0]write bit = 0x1
			rtd_maskl(DVBC_FSM_WR_STB_reg, ~0xff, 0x0d);
			//rtd_part_outl(DVBC_FSM_WR_STB_reg, 7, 0, 0xe);	//0x15d=0x0e, FSM_WR_STB, [4:1]FSM state number=0x7(u_aagc_exp_fsm)
			rtd_maskl(DVBC_FSM_WR_STB_reg, ~0xff, 0x0e);
			//rtd_part_outl(DVBC_FSM_WORD_reg, 7, 0, 0x87);		//0x15e=0x87, fsm_word[7:0]
			rtd_maskl(DVBC_FSM_WORD_reg, ~0xff, 0x87);
			//rtd_part_outl(DVBC_FSM_WORD_1_reg, 7, 0, 0x88);	//0x15f=0x88, fsm_word[15:8]
			rtd_maskl(DVBC_FSM_WORD_1_reg, ~0xff, 0x88);
			//rtd_part_outl(DVBC_FSM_WORD_2_reg, 7, 0, 0xcb);	//0x160=0xcb, fsm_word[23:16]
			rtd_maskl(DVBC_FSM_WORD_2_reg, ~0xff, 0xcb);
			//rtd_part_outl(DVBC_FSM_WORD_3_reg, 7, 0, 0x2e);	//0x161=0x2e, fsm_word[31:24]
			rtd_maskl(DVBC_FSM_WORD_3_reg, ~0xff, 0x2e);
			//rtd_part_outl(DVBC_FSM_WORD_4_reg, 7, 0, 0x48);	//0x162=0x48, fsm_word[39:32]
			rtd_maskl(DVBC_FSM_WORD_4_reg, ~0xff, 0x48);
			//rtd_part_outl(DVBC_FSM_WORD_5_reg, 7, 0, 0xc3);	//0x163=0xc3, fsm_word[47:40]
			rtd_maskl(DVBC_FSM_WORD_5_reg, ~0xff, 0xc3);
			//rtd_part_outl(DVBC_FSM_WORD_6_reg, 7, 0, 0x73);	//0x164=0x73, fsm_word[55:48]
			rtd_maskl(DVBC_FSM_WORD_6_reg, ~0xff, 0x73);
			//rtd_part_outl(DVBC_FSM_WORD_7_reg, 7, 0, 0x40);	//0x165=0x40, fsm_word[63:56]
			rtd_maskl(DVBC_FSM_WORD_7_reg, ~0xff, 0x40);
			//rtd_part_outl(DVBC_FSM_WORD_8_reg, 7, 0, 0xa4);	//0x166=0xa4, fsm_word[71:64]
			rtd_maskl(DVBC_FSM_WORD_8_reg, ~0xff, 0xa4);
			//rtd_part_outl(DVBC_FSM_WORD_9_reg, 7, 0, 0xf8);	//0x167=0xf8, fsm_word[79:72]
			rtd_maskl(DVBC_FSM_WORD_9_reg, ~0xff, 0xf8);
			//rtd_part_outl(DVBC_FSM_WORD_10_reg, 7, 0, 0x32);	//0x168=0x32, fsm_word[87:80]
			rtd_maskl(DVBC_FSM_WORD_10_reg, ~0xff, 0x32);
			//rtd_part_outl(DVBC_FSM_WORD_11_reg, 2, 0, 0x0);	//0x1cc=0x0, fsm_word[90:88]
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~0x07, 0x0);
			//rtd_part_outl(DVBC_FSM_WR_STB_reg, 7, 0, 0xf);	//0x15d=0x0f, FSM_WR_STB, [4:1]FSM state number=0x7,[0]write bit = 0x1
			rtd_maskl(DVBC_FSM_WR_STB_reg, ~0xff, 0xf);
			//rtd_part_outl(DVBC_FSM_WR_STB_reg, 7, 0, 0x10);	//0x15d=0x10, FSM_WR_STB, [4:1]FSM state number=0x8(u_dc_exp_fsm)
			rtd_maskl(DVBC_FSM_WR_STB_reg, ~0xff, 0x10);
			//rtd_part_outl(DVBC_FSM_WORD_reg, 7, 0, 0x98);		//0x15e=0x98, fsm_word[7:0]
			rtd_maskl(DVBC_FSM_WORD_reg, ~0xff, 0x98);
			//rtd_part_outl(DVBC_FSM_WORD_1_reg, 7, 0, 0x99);	//0x15f=0x99, fsm_word[15:8]
			rtd_maskl(DVBC_FSM_WORD_1_reg, ~0xff, 0x99);
			//rtd_part_outl(DVBC_FSM_WORD_2_reg, 7, 0, 0xcc);	//0x160=0xcc, fsm_word[23:16]
			rtd_maskl(DVBC_FSM_WORD_2_reg, ~0xff, 0xcc);
			//rtd_part_outl(DVBC_FSM_WORD_3_reg, 7, 0, 0x3e);	//0x161=0x3e, fsm_word[31:24]
			rtd_maskl(DVBC_FSM_WORD_3_reg, ~0xff, 0x3e);
			//rtd_part_outl(DVBC_FSM_WORD_4_reg, 7, 0, 0x48);	//0x162=0x48, fsm_word[39:32]
			rtd_maskl(DVBC_FSM_WORD_4_reg, ~0xff, 0x48);
			//rtd_part_outl(DVBC_FSM_WORD_5_reg, 7, 0, 0xa3);	//0x163=0xa3, fsm_word[47:40]
			rtd_maskl(DVBC_FSM_WORD_5_reg, ~0xff, 0xa3);
			//rtd_part_outl(DVBC_FSM_WORD_6_reg, 7, 0, 0x73);	//0x164=0x73, fsm_word[55:48]
			rtd_maskl(DVBC_FSM_WORD_6_reg, ~0xff, 0x73);
			//rtd_part_outl(DVBC_FSM_WORD_7_reg, 7, 0, 0x40);	//0x165=0x40, fsm_word[63:56]
			rtd_maskl(DVBC_FSM_WORD_7_reg, ~0xff, 0x40);
			//rtd_part_outl(DVBC_FSM_WORD_8_reg, 7, 0, 0xa4);	//0x166=0xa4, fsm_word[71:64]
			rtd_maskl(DVBC_FSM_WORD_8_reg, ~0xff, 0xa4);
			//rtd_part_outl(DVBC_FSM_WORD_9_reg, 7, 0, 0xfa);	//0x167=0xfa, fsm_word[79:72]
			rtd_maskl(DVBC_FSM_WORD_9_reg, ~0xff, 0xfa);
			//rtd_part_outl(DVBC_FSM_WORD_10_reg, 7, 0, 0x32);	//0x168=0x32, fsm_word[87:80]
			rtd_maskl(DVBC_FSM_WORD_10_reg, ~0xff, 0x32);
			//rtd_part_outl(DVBC_FSM_WORD_11_reg, 2, 0, 0x0);	//0x1cc=0x0, fsm_word[90:88]
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~0x07, 0x0);
			//rtd_part_outl(DVBC_FSM_WR_STB_reg, 7, 0, 0x11);	//0x15d=0x11, FSM_WR_STB, [4:1]FSM state number=0x8,[0]write bit = 0x1


			rtd_maskl(DVBC_FSM_FIX_reg, ~(DVBC_FSM_FIX_opt_fsm_fix_ste_en_mask | DVBC_FSM_FIX_opt_fsm_fix_ste_end_mask)  , (DVBC_FSM_FIX_opt_fsm_fix_ste_en(1) | DVBC_FSM_FIX_opt_fsm_fix_ste_end(5)));
			rtd_maskl(DVBC_OPT_QAMD_START_STATE_reg, ~DVBC_OPT_QAMD_START_STATE_opt_qamd_start_state_mask, DVBC_OPT_QAMD_START_STATE_opt_qamd_start_state(0x5));
			REALTEK_R_QAM_BASE_DBG("[QamRegPreset] high symbol rate!!\n");


		} else {



			//rtd_part_outl(DVBC_FSM_WR_STB_reg, 7, 0, 0xc);	//0x15d=0x0c, FSM_WR_STB, [4:1]FSM state number=0x6(u_dagc_exp_fsm)
			rtd_maskl(DVBC_FSM_WR_STB_reg, ~0xff, 0x0c);
			//rtd_part_outl(DVBC_FSM_WORD_reg, 7, 0, 0x76);		//0x15e=0x76, fsm_word[7:0]
			rtd_maskl(DVBC_FSM_WORD_reg, ~0xff, 0x76);
			//rtd_part_outl(DVBC_FSM_WORD_1_reg, 7, 0, 0x77);	//0x15f=0x77, fsm_word[15:8]
			rtd_maskl(DVBC_FSM_WORD_1_reg, ~0xff, 0x77);
			//rtd_part_outl(DVBC_FSM_WORD_2_reg, 7, 0, 0xcb);	//0x160=0xcb, fsm_word[23:16]
			rtd_maskl(DVBC_FSM_WORD_2_reg, ~0xff, 0xcb);   //0xcb
			//rtd_part_outl(DVBC_FSM_WORD_3_reg, 7, 0, 0x9e);	//0x161=0x9e, fsm_word[31:24]
			rtd_maskl(DVBC_FSM_WORD_3_reg, ~0xff, 0x9e);
			//rtd_part_outl(DVBC_FSM_WORD_4_reg, 7, 0, 0xf7);	//0x162=0xf7, fsm_word[39:32]
			rtd_maskl(DVBC_FSM_WORD_4_reg, ~0xff, 0xf7);
			//rtd_part_outl(DVBC_FSM_WORD_5_reg, 7, 0, 0x4b);	//0x163=0x4b, fsm_word[47:40]
			rtd_maskl(DVBC_FSM_WORD_5_reg, ~0xff, 0x6b);
			//rtd_part_outl(DVBC_FSM_WORD_6_reg, 7, 0, 0x75);	//0x164=0x75, fsm_word[55:48]
			rtd_maskl(DVBC_FSM_WORD_6_reg, ~0xff, 0x75);
			//rtd_part_outl(DVBC_FSM_WORD_7_reg, 7, 0, 0x40);	//0x165=0x40, fsm_word[63:56]
			rtd_maskl(DVBC_FSM_WORD_7_reg, ~0xff, 0x40);
			//rtd_part_outl(DVBC_FSM_WORD_8_reg, 7, 0, 0xa4);	//0x166=0xa4, fsm_word[71:64]
			rtd_maskl(DVBC_FSM_WORD_8_reg, ~0xff, 0xa4);
			//rtd_part_outl(DVBC_FSM_WORD_9_reg, 7, 0, 0xf8);	//0x167=0xf8, fsm_word[79:72]
			rtd_maskl(DVBC_FSM_WORD_9_reg, ~0xff, 0xf8);
			//rtd_part_outl(DVBC_FSM_WORD_10_reg, 7, 0, 0x32);	//0x168=0x32, fsm_word[87:80]
			rtd_maskl(DVBC_FSM_WORD_10_reg, ~0xff, 0x32);
			//rtd_part_outl(DVBC_FSM_WORD_11_reg, 2, 0, 0x0);	//0x1cc=0x0, fsm_word[90:88]
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~0x07, 0x00);
			//rtd_part_outl(DVBC_FSM_WR_STB_reg, 7, 0, 0xd);	//0x15d=0x0d, FSM_WR_STB, [4:1]FSM state number=0x6,[0]write bit = 0x1
			rtd_maskl(DVBC_FSM_WR_STB_reg, ~0xff, 0x0d);
			//rtd_part_outl(DVBC_FSM_WR_STB_reg, 7, 0, 0xe);	//0x15d=0x0e, FSM_WR_STB, [4:1]FSM state number=0x7(u_aagc_exp_fsm)
			rtd_maskl(DVBC_FSM_WR_STB_reg, ~0xff, 0x0e);
			//rtd_part_outl(DVBC_FSM_WORD_reg, 7, 0, 0x87);		//0x15e=0x87, fsm_word[7:0]
			rtd_maskl(DVBC_FSM_WORD_reg, ~0xff, 0x87);
			//rtd_part_outl(DVBC_FSM_WORD_1_reg, 7, 0, 0x88);	//0x15f=0x88, fsm_word[15:8]
			rtd_maskl(DVBC_FSM_WORD_1_reg, ~0xff, 0x88);
			//rtd_part_outl(DVBC_FSM_WORD_2_reg, 7, 0, 0xcb);	//0x160=0xcb, fsm_word[23:16]
			rtd_maskl(DVBC_FSM_WORD_2_reg, ~0xff, 0xcb);  //0xcb
			//rtd_part_outl(DVBC_FSM_WORD_3_reg, 7, 0, 0x2e);	//0x161=0x2e, fsm_word[31:24]
			rtd_maskl(DVBC_FSM_WORD_3_reg, ~0xff, 0x2e);
			//rtd_part_outl(DVBC_FSM_WORD_4_reg, 7, 0, 0x48);	//0x162=0x48, fsm_word[39:32]
			rtd_maskl(DVBC_FSM_WORD_4_reg, ~0xff, 0x48);
			//rtd_part_outl(DVBC_FSM_WORD_5_reg, 7, 0, 0xc3);	//0x163=0xc3, fsm_word[47:40]
			rtd_maskl(DVBC_FSM_WORD_5_reg, ~0xff, 0xf3);
			//rtd_part_outl(DVBC_FSM_WORD_6_reg, 7, 0, 0x73);	//0x164=0x73, fsm_word[55:48]
			rtd_maskl(DVBC_FSM_WORD_6_reg, ~0xff, 0x73);
			//rtd_part_outl(DVBC_FSM_WORD_7_reg, 7, 0, 0x40);	//0x165=0x40, fsm_word[63:56]
			rtd_maskl(DVBC_FSM_WORD_7_reg, ~0xff, 0x40);
			//rtd_part_outl(DVBC_FSM_WORD_8_reg, 7, 0, 0xa4);	//0x166=0xa4, fsm_word[71:64]
			rtd_maskl(DVBC_FSM_WORD_8_reg, ~0xff, 0xa4);
			//rtd_part_outl(DVBC_FSM_WORD_9_reg, 7, 0, 0xf8);	//0x167=0xf8, fsm_word[79:72]
			rtd_maskl(DVBC_FSM_WORD_9_reg, ~0xff, 0xf8);
			//rtd_part_outl(DVBC_FSM_WORD_10_reg, 7, 0, 0x32);	//0x168=0x32, fsm_word[87:80]
			rtd_maskl(DVBC_FSM_WORD_10_reg, ~0xff, 0x32);
			//rtd_part_outl(DVBC_FSM_WORD_11_reg, 2, 0, 0x0);	//0x1cc=0x0, fsm_word[90:88]
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~0x07, 0x0);
			//rtd_part_outl(DVBC_FSM_WR_STB_reg, 7, 0, 0xf);	//0x15d=0x0f, FSM_WR_STB, [4:1]FSM state number=0x7,[0]write bit = 0x1
			rtd_maskl(DVBC_FSM_WR_STB_reg, ~0xff, 0xf);
			//rtd_part_outl(DVBC_FSM_WR_STB_reg, 7, 0, 0x10);	//0x15d=0x10, FSM_WR_STB, [4:1]FSM state number=0x8(u_dc_exp_fsm)
			rtd_maskl(DVBC_FSM_WR_STB_reg, ~0xff, 0x10);
			//rtd_part_outl(DVBC_FSM_WORD_reg, 7, 0, 0x98);		//0x15e=0x98, fsm_word[7:0]
			rtd_maskl(DVBC_FSM_WORD_reg, ~0xff, 0x98);
			//rtd_part_outl(DVBC_FSM_WORD_1_reg, 7, 0, 0x99);	//0x15f=0x99, fsm_word[15:8]
			rtd_maskl(DVBC_FSM_WORD_1_reg, ~0xff, 0x99);
			//rtd_part_outl(DVBC_FSM_WORD_2_reg, 7, 0, 0xcc);	//0x160=0xcc, fsm_word[23:16]
			rtd_maskl(DVBC_FSM_WORD_2_reg, ~0xff, 0xcc);  //0xcc
			//rtd_part_outl(DVBC_FSM_WORD_3_reg, 7, 0, 0x3e);	//0x161=0x3e, fsm_word[31:24]
			rtd_maskl(DVBC_FSM_WORD_3_reg, ~0xff, 0x3e);
			//rtd_part_outl(DVBC_FSM_WORD_4_reg, 7, 0, 0x48);	//0x162=0x48, fsm_word[39:32]
			rtd_maskl(DVBC_FSM_WORD_4_reg, ~0xff, 0x48);
			//rtd_part_outl(DVBC_FSM_WORD_5_reg, 7, 0, 0xa3);	//0x163=0xa3, fsm_word[47:40]
			rtd_maskl(DVBC_FSM_WORD_5_reg, ~0xff, 0xf3);
			//rtd_part_outl(DVBC_FSM_WORD_6_reg, 7, 0, 0x73);	//0x164=0x73, fsm_word[55:48]
			rtd_maskl(DVBC_FSM_WORD_6_reg, ~0xff, 0x73);
			//rtd_part_outl(DVBC_FSM_WORD_7_reg, 7, 0, 0x40);	//0x165=0x40, fsm_word[63:56]
			rtd_maskl(DVBC_FSM_WORD_7_reg, ~0xff, 0x40);
			//rtd_part_outl(DVBC_FSM_WORD_8_reg, 7, 0, 0xa4);	//0x166=0xa4, fsm_word[71:64]
			rtd_maskl(DVBC_FSM_WORD_8_reg, ~0xff, 0xa4);
			//rtd_part_outl(DVBC_FSM_WORD_9_reg, 7, 0, 0xfa);	//0x167=0xfa, fsm_word[79:72]
			rtd_maskl(DVBC_FSM_WORD_9_reg, ~0xff, 0xfa);
			//rtd_part_outl(DVBC_FSM_WORD_10_reg, 7, 0, 0x32);	//0x168=0x32, fsm_word[87:80]
			rtd_maskl(DVBC_FSM_WORD_10_reg, ~0xff, 0x32);
			//rtd_part_outl(DVBC_FSM_WORD_11_reg, 2, 0, 0x0);	//0x1cc=0x0, fsm_word[90:88]
			rtd_maskl(DVBC_FSM_WORD_11_reg, ~0x07, 0x0);
			//rtd_part_outl(DVBC_FSM_WR_STB_reg, 7, 0, 0x11);	//0x15d=0x11, FSM_WR_STB, [4:1]FSM state number=0x8,[0]write bit = 0x1


			rtd_maskl(DVBC_FSM_FIX_reg, ~(DVBC_FSM_FIX_opt_fsm_fix_ste_en_mask | DVBC_FSM_FIX_opt_fsm_fix_ste_end_mask)  , (DVBC_FSM_FIX_opt_fsm_fix_ste_en(1) | DVBC_FSM_FIX_opt_fsm_fix_ste_end(9)));
			rtd_maskl(DVBC_OPT_QAMD_START_STATE_reg, ~DVBC_OPT_QAMD_START_STATE_opt_qamd_start_state_mask, DVBC_OPT_QAMD_START_STATE_opt_qamd_start_state(0x9));
			REALTEK_R_QAM_BASE_DBG("[QamRegPreset] low symbol rate!!\n");

		}

	*/


	return FUNCTION_SUCCESS;
}

int
realtek_r_dvbc_QamRegPostset(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS SymbolRateHz
)
{

	/*
		//rtd_part_outl(DVBC_FSM_WR_STB_reg, 7, 0, 0xc);	//0x15d=0x0c, FSM_WR_STB, [4:1]FSM state number=0x6(u_dagc_exp_fsm)
		rtd_maskl(DVBC_FSM_WR_STB_reg, ~0xff, 0x0c);
		//rtd_part_outl(DVBC_FSM_WORD_reg, 7, 0, 0x76);		//0x15e=0x76, fsm_word[7:0]
		rtd_maskl(DVBC_FSM_WORD_reg, ~0xff, 0x76);
		//rtd_part_outl(DVBC_FSM_WORD_1_reg, 7, 0, 0x77);	//0x15f=0x77, fsm_word[15:8]
		rtd_maskl(DVBC_FSM_WORD_1_reg, ~0xff, 0x77);
		//rtd_part_outl(DVBC_FSM_WORD_2_reg, 7, 0, 0xcb);	//0x160=0xcb, fsm_word[23:16]
		rtd_maskl(DVBC_FSM_WORD_2_reg, ~0xff, 0xcb);
		//rtd_part_outl(DVBC_FSM_WORD_3_reg, 7, 0, 0x9e);	//0x161=0x9e, fsm_word[31:24]
		rtd_maskl(DVBC_FSM_WORD_3_reg, ~0xff, 0x9e);
		//rtd_part_outl(DVBC_FSM_WORD_4_reg, 7, 0, 0xf7);	//0x162=0xf7, fsm_word[39:32]
		rtd_maskl(DVBC_FSM_WORD_4_reg, ~0xff, 0xf7);
		//rtd_part_outl(DVBC_FSM_WORD_5_reg, 7, 0, 0x4b);	//0x163=0x4b, fsm_word[47:40]
		rtd_maskl(DVBC_FSM_WORD_5_reg, ~0xff, 0x4b);
		//rtd_part_outl(DVBC_FSM_WORD_6_reg, 7, 0, 0x75);	//0x164=0x75, fsm_word[55:48]
		rtd_maskl(DVBC_FSM_WORD_6_reg, ~0xff, 0x75);
		//rtd_part_outl(DVBC_FSM_WORD_7_reg, 7, 0, 0x40);	//0x165=0x40, fsm_word[63:56]
		rtd_maskl(DVBC_FSM_WORD_7_reg, ~0xff, 0x40);
		//rtd_part_outl(DVBC_FSM_WORD_8_reg, 7, 0, 0xa4);	//0x166=0xa4, fsm_word[71:64]
		rtd_maskl(DVBC_FSM_WORD_8_reg, ~0xff, 0xa4);
		//rtd_part_outl(DVBC_FSM_WORD_9_reg, 7, 0, 0xf8);	//0x167=0xf8, fsm_word[79:72]
		rtd_maskl(DVBC_FSM_WORD_9_reg, ~0xff, 0xf8);
		//rtd_part_outl(DVBC_FSM_WORD_10_reg, 7, 0, 0x32);	//0x168=0x32, fsm_word[87:80]
		rtd_maskl(DVBC_FSM_WORD_10_reg, ~0xff, 0x32);
		//rtd_part_outl(DVBC_FSM_WORD_11_reg, 2, 0, 0x0);	//0x1cc=0x0, fsm_word[90:88]
		rtd_maskl(DVBC_FSM_WORD_11_reg, ~0x07, 0x00);
		//rtd_part_outl(DVBC_FSM_WR_STB_reg, 7, 0, 0xd);	//0x15d=0x0d, FSM_WR_STB, [4:1]FSM state number=0x6,[0]write bit = 0x1
		rtd_maskl(DVBC_FSM_WR_STB_reg, ~0xff, 0x0d);
		//rtd_part_outl(DVBC_FSM_WR_STB_reg, 7, 0, 0xe);	//0x15d=0x0e, FSM_WR_STB, [4:1]FSM state number=0x7(u_aagc_exp_fsm)
		rtd_maskl(DVBC_FSM_WR_STB_reg, ~0xff, 0x0e);
		//rtd_part_outl(DVBC_FSM_WORD_reg, 7, 0, 0x87);		//0x15e=0x87, fsm_word[7:0]
		rtd_maskl(DVBC_FSM_WORD_reg, ~0xff, 0x87);
		//rtd_part_outl(DVBC_FSM_WORD_1_reg, 7, 0, 0x88);	//0x15f=0x88, fsm_word[15:8]
		rtd_maskl(DVBC_FSM_WORD_1_reg, ~0xff, 0x88);
		//rtd_part_outl(DVBC_FSM_WORD_2_reg, 7, 0, 0xcb);	//0x160=0xcb, fsm_word[23:16]
		rtd_maskl(DVBC_FSM_WORD_2_reg, ~0xff, 0xcb);
		//rtd_part_outl(DVBC_FSM_WORD_3_reg, 7, 0, 0x2e);	//0x161=0x2e, fsm_word[31:24]
		rtd_maskl(DVBC_FSM_WORD_3_reg, ~0xff, 0x2e);
		//rtd_part_outl(DVBC_FSM_WORD_4_reg, 7, 0, 0x48);	//0x162=0x48, fsm_word[39:32]
		rtd_maskl(DVBC_FSM_WORD_4_reg, ~0xff, 0x48);
		//rtd_part_outl(DVBC_FSM_WORD_5_reg, 7, 0, 0xc3);	//0x163=0xc3, fsm_word[47:40]
		rtd_maskl(DVBC_FSM_WORD_5_reg, ~0xff, 0xc3);
		//rtd_part_outl(DVBC_FSM_WORD_6_reg, 7, 0, 0x73);	//0x164=0x73, fsm_word[55:48]
		rtd_maskl(DVBC_FSM_WORD_6_reg, ~0xff, 0x73);
		//rtd_part_outl(DVBC_FSM_WORD_7_reg, 7, 0, 0x40);	//0x165=0x40, fsm_word[63:56]
		rtd_maskl(DVBC_FSM_WORD_7_reg, ~0xff, 0x40);
		//rtd_part_outl(DVBC_FSM_WORD_8_reg, 7, 0, 0xa4);	//0x166=0xa4, fsm_word[71:64]
		rtd_maskl(DVBC_FSM_WORD_8_reg, ~0xff, 0xa4);
		//rtd_part_outl(DVBC_FSM_WORD_9_reg, 7, 0, 0xf8);	//0x167=0xf8, fsm_word[79:72]
		rtd_maskl(DVBC_FSM_WORD_9_reg, ~0xff, 0xf8);
		//rtd_part_outl(DVBC_FSM_WORD_10_reg, 7, 0, 0x32);	//0x168=0x32, fsm_word[87:80]
		rtd_maskl(DVBC_FSM_WORD_10_reg, ~0xff, 0x32);
		//rtd_part_outl(DVBC_FSM_WORD_11_reg, 2, 0, 0x0);	//0x1cc=0x0, fsm_word[90:88]
		rtd_maskl(DVBC_FSM_WORD_11_reg, ~0x07, 0x0);
		//rtd_part_outl(DVBC_FSM_WR_STB_reg, 7, 0, 0xf);	//0x15d=0x0f, FSM_WR_STB, [4:1]FSM state number=0x7,[0]write bit = 0x1
		rtd_maskl(DVBC_FSM_WR_STB_reg, ~0xff, 0xf);
		//rtd_part_outl(DVBC_FSM_WR_STB_reg, 7, 0, 0x10);	//0x15d=0x10, FSM_WR_STB, [4:1]FSM state number=0x8(u_dc_exp_fsm)
		rtd_maskl(DVBC_FSM_WR_STB_reg, ~0xff, 0x10);
		//rtd_part_outl(DVBC_FSM_WORD_reg, 7, 0, 0x98);		//0x15e=0x98, fsm_word[7:0]
		rtd_maskl(DVBC_FSM_WORD_reg, ~0xff, 0x98);
		//rtd_part_outl(DVBC_FSM_WORD_1_reg, 7, 0, 0x99);	//0x15f=0x99, fsm_word[15:8]
		rtd_maskl(DVBC_FSM_WORD_1_reg, ~0xff, 0x99);
		//rtd_part_outl(DVBC_FSM_WORD_2_reg, 7, 0, 0xcc);	//0x160=0xcc, fsm_word[23:16]
		rtd_maskl(DVBC_FSM_WORD_2_reg, ~0xff, 0xcc);
		//rtd_part_outl(DVBC_FSM_WORD_3_reg, 7, 0, 0x3e);	//0x161=0x3e, fsm_word[31:24]
		rtd_maskl(DVBC_FSM_WORD_3_reg, ~0xff, 0x3e);
		//rtd_part_outl(DVBC_FSM_WORD_4_reg, 7, 0, 0x48);	//0x162=0x48, fsm_word[39:32]
		rtd_maskl(DVBC_FSM_WORD_4_reg, ~0xff, 0x48);
		//rtd_part_outl(DVBC_FSM_WORD_5_reg, 7, 0, 0xa3);	//0x163=0xa3, fsm_word[47:40]
		rtd_maskl(DVBC_FSM_WORD_5_reg, ~0xff, 0xa3);
		//rtd_part_outl(DVBC_FSM_WORD_6_reg, 7, 0, 0x73);	//0x164=0x73, fsm_word[55:48]
		rtd_maskl(DVBC_FSM_WORD_6_reg, ~0xff, 0x73);
		//rtd_part_outl(DVBC_FSM_WORD_7_reg, 7, 0, 0x40);	//0x165=0x40, fsm_word[63:56]
		rtd_maskl(DVBC_FSM_WORD_7_reg, ~0xff, 0x40);
		//rtd_part_outl(DVBC_FSM_WORD_8_reg, 7, 0, 0xa4);	//0x166=0xa4, fsm_word[71:64]
		rtd_maskl(DVBC_FSM_WORD_8_reg, ~0xff, 0xa4);
		//rtd_part_outl(DVBC_FSM_WORD_9_reg, 7, 0, 0xfa);	//0x167=0xfa, fsm_word[79:72]
		rtd_maskl(DVBC_FSM_WORD_9_reg, ~0xff, 0xfa);
		//rtd_part_outl(DVBC_FSM_WORD_10_reg, 7, 0, 0x32);	//0x168=0x32, fsm_word[87:80]
		rtd_maskl(DVBC_FSM_WORD_10_reg, ~0xff, 0x32);
		//rtd_part_outl(DVBC_FSM_WORD_11_reg, 2, 0, 0x0);	//0x1cc=0x0, fsm_word[90:88]
		rtd_maskl(DVBC_FSM_WORD_11_reg, ~0x07, 0x0);
		//rtd_part_outl(DVBC_FSM_WR_STB_reg, 7, 0, 0x11);	//0x15d=0x11, FSM_WR_STB, [4:1]FSM state number=0x8,[0]write bit = 0x1

	*/

	rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask, DVBC_OPT_RST_STATE0_ON_opt_retry_on(1));
	rtd_maskl(DVBC_FSM_FIX_reg, ~(DVBC_FSM_FIX_opt_fsm_fix_ste_en_mask | DVBC_FSM_FIX_opt_fsm_fix_ste_end_mask)  , (DVBC_FSM_FIX_opt_fsm_fix_ste_en(0) | DVBC_FSM_FIX_opt_fsm_fix_ste_end(0)));

	return FUNCTION_SUCCESS;
}

int
realtek_r_dvbc_QamPreCheck(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS SymbolRateHz
)
{

	unsigned long   stime;
	U32BITS fsm_state_o = 0;
	unsigned int cr_ld_out = 0xff, tr_ld_out = 0xff;
	unsigned int reset_cnt = 0;
	int est_ccfo = 0;
	unsigned int CcfoInitFreq = 0;

	stime = tv_osal_time();

	if (SymbolRateHz <= 5010000) {


		while ((tv_osal_time() - stime) <= 300) {


			tv_osal_msleep(5);
			rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));
			fsm_state_o = ((rtd_inl(DVBC_INRX_MO2_reg) & 0x78) >> 3);
			cr_ld_out = rtd_inl(DVBC_INRX_MO2_2_reg);
			tr_ld_out = rtd_inl(DVBC_INRX_MO2_1_reg);

			REALTEK_R_QAM_BASE_DBG("[QamPreCheck] fsm_state_o = "PT_U32BITS", cr_ld_out = %d, tr_ld_out = %d \n", fsm_state_o, cr_ld_out, tr_ld_out);

			if (fsm_state_o >= 9) { //5

				if ((cr_ld_out <= 31) && (tr_ld_out <= 31) && (cr_ld_out > 25)) {
					break;
				} else {

					pDemod->SoftwareReset(pDemod);
					reset_cnt++;
					pDemod->GetCcfoInitFreq(pDemod, SymbolRateHz, &CcfoInitFreq, &est_ccfo);
					REALTEK_R_QAM_BASE_INFO("[QamPreCheck][reset] CcfoInitFreq = %x, est_ccfo = % d\n", CcfoInitFreq, est_ccfo);
					tv_osal_msleep(20);
					if (reset_cnt > 1)
						break;
				}
			}
		}
	}
	return FUNCTION_SUCCESS;
}

/**

@see   QAM_DEMOD_FP_SET_CCFO_EN

*/
int
realtek_r_dvbc_SetCcfoMethodEn(
	QAM_DEMOD_MODULE *pDemod,
	unsigned char enable
)
{
	REALTEK_R_QAM_BASE_INFO("SetCcfoEn = %u", enable);
	if (enable) {
		rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_start_srs_nbd_mask, DVBC_OPT_START_SRS_NBD_opt_start_srs_nbd(0));
		rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_ccfo_fft_method_en_mask, DVBC_OPT_START_SRS_NBD_opt_ccfo_fft_method_en(enable & 0x1));
		rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_start_srs_nbd_mask, DVBC_OPT_START_SRS_NBD_opt_start_srs_nbd(1));
	} else {
		rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_start_srs_nbd_mask, DVBC_OPT_START_SRS_NBD_opt_start_srs_nbd(0));
		rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_ccfo_fft_method_en_mask, DVBC_OPT_START_SRS_NBD_opt_ccfo_fft_method_en(enable & 0x1));
	}

	return FUNCTION_SUCCESS;
}



int
realtek_r_dvbc_IsCcfoMethodEn(
	QAM_DEMOD_MODULE *pDemod,
	unsigned int* pccfo_en_flag
)
{
	*pccfo_en_flag = DVBC_OPT_START_SRS_NBD_get_opt_ccfo_fft_method_en(rtd_inl(DVBC_OPT_START_SRS_NBD_reg));

	return FUNCTION_SUCCESS;
}



/**

@see   QAM_DEMOD_FP_CCFO

*/
void
realtek_r_dvbc_GetCcfoInitFreq(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS SymbolRate,
	unsigned int *pCfoValue,
	int *pEstCcfo
)
{
	unsigned int est_ccfo_idx = 0, ccfo_init_freq = 0;
	int est_ccfo = 0;

	est_ccfo_idx = (DVBC_FFTOUT_MAX1_get_ccfo_local_max_val_15_8(rtd_inl(DVBC_FFTOUT_MAX3_reg)) << 8)  | (DVBC_FFTOUT_MAX2_get_ccfo_local_max_idx_7_0(rtd_inl(DVBC_FFTOUT_MAX2_reg)));
	//est_ccfo = (est_ccfo_idx < (fft_size / 2)) ? (est_ccfo_idx * (CrystalFreqHz / fft_size) / 4) : ((fft_size - est_ccfo_idx) * (CrystalFreqHz / fft_size) / 4);
	est_ccfo = (est_ccfo_idx < 4096) ? ((int)est_ccfo_idx*(9000000/8192)) : (((int)est_ccfo_idx - 8192)*(9000000/8192));

	REALTEK_R_QAM_BASE_INFO("[CCFO] est_ccfo_idx = 0x%x, est_ccfo = %d Hz\n", est_ccfo_idx, est_ccfo);

	*pEstCcfo = est_ccfo;
	*pCfoValue = ccfo_init_freq;
	REALTEK_R_QAM_BASE_DBG("[CCFO] ccfo_init_freq = 0x%x (%u)\n", ccfo_init_freq, ccfo_init_freq);

	return ;
}


/**

@see   QAM_DEMOD_FP_CCFO

*/
int
realtek_r_dvbc_SetCcfoInitFreq(
	QAM_DEMOD_MODULE *pDemod,
	unsigned int CfoValue
)
{
	REALTEK_R_QAM_BASE_DBG("SetCcfoInitFreq = 0x%x (%u)", CfoValue, CfoValue);
	rtd_outl(DVBC_OPT_INITIAL_CR_FREQ_reg, (CfoValue & 0xff));
	rtd_outl(DVBC_OPT_INITIAL_CR_FREQ_1_reg, (CfoValue >> 8) & 0x1f);
	return FUNCTION_SUCCESS;
}

int
realtek_r_qam_GetSrsCcfoDoneFlag(
	QAM_DEMOD_MODULE *pDemod,
	unsigned int *SrsCcfoDoneFlag
)
{
	*SrsCcfoDoneFlag = DVBC_SRS_NBD_DONE_get_srs_nbd_done(rtd_inl(DVBC_SRS_NBD_DONE_reg));
	return FUNCTION_SUCCESS;
}


int  realtek_r_dvbc_TSBitAccu(
	QAM_DEMOD_MODULE *pDemod,
	unsigned int *TSThroughputKBps
)
{
	/*
	For DVB-C,
	TS bit-rate = Symbol rate x (188/204) x Q
	Where Q
	= 8 for 256QAM
	= 7 for 128QAM
	= 6 for 64QAM
	= 5 for 32QAM
	= 4 for 16-QAM
	*/

	int ret = FUNCTION_SUCCESS;
	int QamMode = 0;
	unsigned char q_val[] = {0, 4, 5, 6, 7, 8}; // *QPSK suit for qam mode defined, 16QAM, 32QAM, 64QAM, 128QAM, 256QAM
	U32BITS SymbolRateHz = 0;

	ret |= pDemod->GetQamMode(pDemod, &QamMode);
	ret |= pDemod->GetSymbolRateHz(pDemod, &SymbolRateHz);

	*TSThroughputKBps = (ret == FUNCTION_SUCCESS) ? ((SymbolRateHz / 1000) * q_val[QamMode] * 188 / 204) : 0;
	REALTEK_R_QAM_BASE_DBG("[TSBitACCU] QamMode = %d, SymbolRateHz = "PT_U32BITS", TSThroughputBps = %u\n", QamMode, SymbolRateHz, *TSThroughputKBps);

	return FUNCTION_SUCCESS;

}


/**

@see   REALTEK_R_QAM_FP_GET_CONFIG_MODE

*/
void
realtek_r_qam_GetConfigMode(
	QAM_DEMOD_MODULE *pDemod,
	int *pConfigMode
)
{
	REALTEK_R_QAM_EXTRA_MODULE *pExtra;

	// Get demod extra module;
	pExtra = &(pDemod->Extra.RealtekRQam);

	// Get configurtion mode from demod extra module.
	*pConfigMode = pExtra->ConfigMode;

	return;
}


/**

@see   QAM_DEMOD_FP_SET_FFT_MODE

*/
int
realtek_r_qam_SetFftMode(
	QAM_DEMOD_MODULE *pDemod,
	FFTMODE mode
)
{
	U32BITS CrystalFreqHz;
	unsigned int sr_idx_max, sr_idx_min;

	pDemod->GetCrystalFreqHz(pDemod, &CrystalFreqHz);
	//symbol_rate_idx = (symbol_rate * 2 * fft_size) /ADC_freq; //fft_size=8192

	switch (mode) {
	case REALTEK_R_DVBC_SRS_FULL:
		//search range 3.6M ~ 7.2M
		sr_idx_max = (((7210) << 14) / (CrystalFreqHz / 1000));
		sr_idx_min = (((3600) << 14) / (CrystalFreqHz / 1000));
		rtd_maskl(DVBC_OPT_SRS_INPUT_SEL_reg, ~DVBC_OPT_SRS_INPUT_SEL_opt_srs_input_sel_mask, DVBC_OPT_SRS_INPUT_SEL_opt_srs_input_sel(0x1));
		rtd_maskl(DVBC_OPT_FFT_OUTPUT_ACC_NUM_reg, ~DVBC_OPT_FFT_OUTPUT_ACC_NUM_opt_fft_output_acc_num_mask, DVBC_OPT_FFT_OUTPUT_ACC_NUM_opt_fft_output_acc_num(0xf));
		rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_reg, ~DVBC_OPT_POS_FRQ_MAX_SR_opt_pos_frq_max_sr_7_0_mask, DVBC_OPT_POS_FRQ_MAX_SR_opt_pos_frq_max_sr_7_0(sr_idx_max & 0xff));
		rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_1_reg, ~DVBC_OPT_POS_FRQ_MAX_SR_1_opt_pos_frq_max_sr_11_8_mask, DVBC_OPT_POS_FRQ_MAX_SR_1_opt_pos_frq_max_sr_11_8((sr_idx_max >> 8) & 0x0f));
		rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_reg, ~DVBC_OPT_POS_FRQ_MIN_SR_opt_pos_frq_min_sr_7_0_mask, DVBC_OPT_POS_FRQ_MIN_SR_opt_pos_frq_min_sr_7_0(sr_idx_min & 0xff));
		rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_1_reg, ~DVBC_OPT_POS_FRQ_MIN_SR_1_opt_pos_frq_min_sr_11_8_mask, DVBC_OPT_POS_FRQ_MIN_SR_1_opt_pos_frq_min_sr_11_8((sr_idx_min >> 8) & 0x0f));
		rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_fsm_srs_en_ste_mask, DVBC_OPT_START_SRS_NBD_opt_fsm_srs_en_ste(0x2));
		rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_ccfo_fft_method_en_mask, DVBC_OPT_START_SRS_NBD_opt_ccfo_fft_method_en(0x0));
		rtd_maskl(DVBC_CCFO_CTRL_reg, ~DVBC_CCFO_CTRL_opt_fft_input_bitwise_sel_mask, DVBC_CCFO_CTRL_opt_fft_input_bitwise_sel(0x1));
		break;

	case REALTEK_R_DVBC_SRS_HBR:
		//search range 6.5M ~ 7.2M
		sr_idx_max = (((7210) << 14) / (CrystalFreqHz / 1000));
		sr_idx_min = (((6500) << 14) / (CrystalFreqHz / 1000));
		rtd_maskl(DVBC_OPT_SRS_INPUT_SEL_reg, ~DVBC_OPT_SRS_INPUT_SEL_opt_srs_input_sel_mask, DVBC_OPT_SRS_INPUT_SEL_opt_srs_input_sel(0x1));
		rtd_maskl(DVBC_OPT_FFT_OUTPUT_ACC_NUM_reg, ~DVBC_OPT_FFT_OUTPUT_ACC_NUM_opt_fft_output_acc_num_mask, DVBC_OPT_FFT_OUTPUT_ACC_NUM_opt_fft_output_acc_num(0xf));
		rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_reg, ~DVBC_OPT_POS_FRQ_MAX_SR_opt_pos_frq_max_sr_7_0_mask, DVBC_OPT_POS_FRQ_MAX_SR_opt_pos_frq_max_sr_7_0(sr_idx_max & 0xff));
		rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_1_reg, ~DVBC_OPT_POS_FRQ_MAX_SR_1_opt_pos_frq_max_sr_11_8_mask, DVBC_OPT_POS_FRQ_MAX_SR_1_opt_pos_frq_max_sr_11_8((sr_idx_max >> 8) & 0x0f));
		rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_reg, ~DVBC_OPT_POS_FRQ_MIN_SR_opt_pos_frq_min_sr_7_0_mask, DVBC_OPT_POS_FRQ_MIN_SR_opt_pos_frq_min_sr_7_0(sr_idx_min & 0xff));
		rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_1_reg, ~DVBC_OPT_POS_FRQ_MIN_SR_1_opt_pos_frq_min_sr_11_8_mask, DVBC_OPT_POS_FRQ_MIN_SR_1_opt_pos_frq_min_sr_11_8((sr_idx_min >> 8) & 0x0f));
		rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_fsm_srs_en_ste_mask, DVBC_OPT_START_SRS_NBD_opt_fsm_srs_en_ste(0x2));
		rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_ccfo_fft_method_en_mask, DVBC_OPT_START_SRS_NBD_opt_ccfo_fft_method_en(0x0));
		rtd_maskl(DVBC_CCFO_CTRL_reg, ~DVBC_CCFO_CTRL_opt_fft_input_bitwise_sel_mask, DVBC_CCFO_CTRL_opt_fft_input_bitwise_sel(0x1));
		break;

	case REALTEK_R_OC_SRS:
		//search range 4.978M ~ 5.367M (Merlin), SymRate (OC) = srs_fft_idx*(ADC_freq/2/fft_size)
		sr_idx_max = (((5367) << 14) / (CrystalFreqHz / 1000));
		sr_idx_min = (((4978) << 14) / (CrystalFreqHz / 1000));
		rtd_maskl(DVBC_OPT_SRS_INPUT_SEL_reg, ~DVBC_OPT_SRS_INPUT_SEL_opt_srs_input_sel_mask, DVBC_OPT_SRS_INPUT_SEL_opt_srs_input_sel(0x1));
		rtd_maskl(DVBC_OPT_FFT_OUTPUT_ACC_NUM_reg, ~DVBC_OPT_FFT_OUTPUT_ACC_NUM_opt_fft_output_acc_num_mask, DVBC_OPT_FFT_OUTPUT_ACC_NUM_opt_fft_output_acc_num(0xf));
		rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_reg, ~DVBC_OPT_POS_FRQ_MAX_SR_opt_pos_frq_max_sr_7_0_mask, DVBC_OPT_POS_FRQ_MAX_SR_opt_pos_frq_max_sr_7_0(sr_idx_max & 0xff));
		rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_1_reg, ~DVBC_OPT_POS_FRQ_MAX_SR_1_opt_pos_frq_max_sr_11_8_mask, DVBC_OPT_POS_FRQ_MAX_SR_1_opt_pos_frq_max_sr_11_8((sr_idx_max >> 8) & 0x0f));
		rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_reg, ~DVBC_OPT_POS_FRQ_MIN_SR_opt_pos_frq_min_sr_7_0_mask, DVBC_OPT_POS_FRQ_MIN_SR_opt_pos_frq_min_sr_7_0(sr_idx_min & 0xff));
		rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_1_reg, ~DVBC_OPT_POS_FRQ_MIN_SR_1_opt_pos_frq_min_sr_11_8_mask, DVBC_OPT_POS_FRQ_MIN_SR_1_opt_pos_frq_min_sr_11_8((sr_idx_min >> 8) & 0x0f));
		rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_fsm_srs_en_ste_mask, DVBC_OPT_START_SRS_NBD_opt_fsm_srs_en_ste(0x2));
		rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_ccfo_fft_method_en_mask, DVBC_OPT_START_SRS_NBD_opt_ccfo_fft_method_en(0x0));
		rtd_maskl(DVBC_CCFO_CTRL_reg, ~DVBC_CCFO_CTRL_opt_fft_input_bitwise_sel_mask, DVBC_CCFO_CTRL_opt_fft_input_bitwise_sel(0x1));
		break;

	case REALTEK_R_CCFO:
	default:
		rtd_maskl(DVBC_OPT_SRS_INPUT_SEL_reg, ~DVBC_OPT_SRS_INPUT_SEL_opt_srs_input_sel_mask, DVBC_OPT_SRS_INPUT_SEL_opt_srs_input_sel(0x2));
		rtd_maskl(DVBC_OPT_FFT_OUTPUT_ACC_NUM_reg, ~DVBC_OPT_FFT_OUTPUT_ACC_NUM_opt_fft_output_acc_num_mask, DVBC_OPT_FFT_OUTPUT_ACC_NUM_opt_fft_output_acc_num(0x3));  //0x6
		rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_reg, ~DVBC_OPT_POS_FRQ_MAX_SR_opt_pos_frq_max_sr_7_0_mask, DVBC_OPT_POS_FRQ_MAX_SR_opt_pos_frq_max_sr_7_0(0x7e));
		rtd_maskl(DVBC_OPT_POS_FRQ_MAX_SR_1_reg, ~DVBC_OPT_POS_FRQ_MAX_SR_1_opt_pos_frq_max_sr_11_8_mask, DVBC_OPT_POS_FRQ_MAX_SR_1_opt_pos_frq_max_sr_11_8(0x4));
		rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_reg, ~DVBC_OPT_POS_FRQ_MIN_SR_opt_pos_frq_min_sr_7_0_mask, DVBC_OPT_POS_FRQ_MIN_SR_opt_pos_frq_min_sr_7_0(0x0));
		rtd_maskl(DVBC_OPT_POS_FRQ_MIN_SR_1_reg, ~DVBC_OPT_POS_FRQ_MIN_SR_1_opt_pos_frq_min_sr_11_8_mask, DVBC_OPT_POS_FRQ_MIN_SR_1_opt_pos_frq_min_sr_11_8(0x0));
		rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_fsm_srs_en_ste_mask, DVBC_OPT_START_SRS_NBD_opt_fsm_srs_en_ste(0x2));
		rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_ccfo_fft_method_en_mask, DVBC_OPT_START_SRS_NBD_opt_ccfo_fft_method_en(0x0));
		rtd_maskl(DVBC_CCFO_CTRL_reg, ~DVBC_CCFO_CTRL_opt_fft_input_bitwise_sel_mask, DVBC_CCFO_CTRL_opt_fft_input_bitwise_sel(0x2));
		rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_start_srs_nbd_mask, DVBC_OPT_START_SRS_NBD_opt_start_srs_nbd(0x0));
		break;

	}

	return FUNCTION_SUCCESS;
}

int
realtek_r_dvbc_SetAutoModeEn(QAM_DEMOD_MODULE *pDemod, unsigned char enable)
{
	REALTEK_R_QAM_BASE_INFO("SetAutoModeEn=%u", enable);
	if (enable) {
		rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask, DVBC_OPT_RST_STATE0_ON_opt_retry_on(0));//retry off
		rtd_outl(DVBC_AUTO_SRS_reg, 0x15); //[0]Auto srs start [2]Auto set tr_deci [4]Auto CSR enable
	} else {
		rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask, DVBC_OPT_RST_STATE0_ON_opt_retry_on(1));//retry on
		rtd_outl(DVBC_AUTO_SRS_reg, 0x0); //[0]Auto srs start [2]Auto set tr_deci [4]Auto CSR enable
	}

	return FUNCTION_SUCCESS;
}

int
realtek_r_dvbc_AutoModePreset(
	QAM_DEMOD_MODULE *pDemod
)
{
	//REALTEK_R_QAM_BASE_INFO("Set Auto Mode Default, Fix ST05\n");
	rtd_maskl(DVBC_OPT_QAM_MODE_reg, ~DVBC_OPT_QAM_MODE_opt_qam_mode_mask, 0x04); //0xe102=0x2, 256QAM
	rtd_maskl(DVBC_OPT_CR_ACQ_PWR_TH_reg, ~DVBC_OPT_CR_ACQ_PWR_TH_opt_cr_acq_pwr_th_mask, 0x00); //0xe118=0x00
	rtd_outl(DVBC_OPT_CR_METRIC_DD_MAX_reg, 0xd4); //0xe119=0xd4
	rtd_maskl(DVBC_OPT_DD_EQ_BOUND_reg, ~0xff, 0xd0); //0xe12e=0xd0
	rtd_maskl(DVBC_OPT_SIGN_DATA_reg, ~0xff, 0x80); //0xe12f=0x80
	rtd_maskl(DVBC_OPT_SIGN_DATA_1_reg, ~0xff, 0x43); //0xe130=0x30
	rtd_maskl(DVBC_OPT_CIRCLE_reg, ~DVBC_OPT_CIRCLE_opt_circle_mask, 0x0a); //0xe131=0xa
	//--------------------Set after known Qam mode
	rtd_outl(DVBC_OPT_U_FFF_DD0_reg, 0x11); //0xe140=0x11
	rtd_outl(DVBC_OPT_U_FFF_DD2_reg, 0x4c); //0xe141=0x4c
	rtd_outl(DVBC_OPT_U_FFF2_DD0_reg, 0x1a); //0xe197=0x1a
	rtd_outl(DVBC_OPT_U_FFF2_DD2_reg, 0x15); //0xe198=0x15
	rtd_outl(DVBC_OPT_U_FFF3_DD1_reg, 0x2b); //0xe199=0x2b
	rtd_outl(DVBC_OPT_U_FFF4_DD0_reg, 0x1a); //0xe19a=0x1a
	rtd_outl(DVBC_OPT_U_FFF4_DD2_reg, 0x75); //0xe19b=0x75

	//Set symbol rate = 6.952/36M
	//TR_DECI_RATIO = (ADC_frequency in Hz) * pow(2, 18) / (symbol rate in Hz) = 36 * 262144 / 6.875 = 1372681 =>0x14f209
	//TR_DECI_RATIO = (ADC_frequency in Hz) * pow(2, 18) / (symbol rate in Hz) = 36 * 262144 / 6.952 = 1357477 =>0x14b6a5
	rtd_maskl(DVBC_OPT_TR_DECI_RATIO_reg, ~DVBC_OPT_TR_DECI_RATIO_opt_tr_deci_ratio_7_0_mask, 0xa5); //[7:0]opt_tr_deci_ratio[7:0], 2*symbol rate/ADC frequency
	rtd_maskl(DVBC_OPT_TR_DECI_RATIO_1_reg, ~DVBC_OPT_TR_DECI_RATIO_1_opt_tr_deci_ratio_15_8_mask, 0xb6); //[7:0]opt_tr_deci_ratio[15:8], 2*symbol rate/ADC frequency
	rtd_maskl(DVBC_OPT_TR_DECI_RATIO_2_reg, ~DVBC_OPT_TR_DECI_RATIO_2_opt_tr_deci_ratio_23_16_mask, 0x14); //[7:0]opt_tr_deci_ratio[23:16], 2*symbol rate/ADC frequency
	
	//DDC 6.875M/36M
	//{3fb,3fa,004,016,018,3f7,3c0,3b1,00e,0d5,1a6}
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_sel_mask, 0x0);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_coeff_2_0_mask, 0x60);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask, 0x7f);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_wr_stb_mask, 0x01);

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_sel_mask, 0x02);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_coeff_2_0_mask, 0x40);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask, 0x7f);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_wr_stb_mask, 0x01);

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_sel_mask, 0x04);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_coeff_2_0_mask, 0x80);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask, 0x00);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_wr_stb_mask, 0x01);

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_sel_mask, 0x06);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_coeff_2_0_mask, 0xc0);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask, 0x02);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_wr_stb_mask, 0x01);

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_sel_mask, 0x08);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_coeff_2_0_mask, 0x00);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask, 0x03);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_wr_stb_mask, 0x01);

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_sel_mask, 0x0a);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_coeff_2_0_mask, 0xe0);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask, 0x7e);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_wr_stb_mask, 0x01);

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_sel_mask, 0x0c);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_coeff_2_0_mask, 0x00);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask, 0x78);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_wr_stb_mask, 0x01);

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_sel_mask, 0x0e);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_coeff_2_0_mask, 0x20);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask, 0x76);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_wr_stb_mask, 0x01);

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_sel_mask, 0x10);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_coeff_2_0_mask, 0xc0);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask, 0x01);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_wr_stb_mask, 0x01);

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_sel_mask, 0x12);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_coeff_2_0_mask, 0xa0);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask, 0x1a);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_wr_stb_mask, 0x01);

	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_sel_mask, 0x14);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_coeff_2_0_mask, 0xc0);
	rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask, 0x34);
	rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_wr_stb_mask, 0x01);
	//End set DDC

	//Pre SRS
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x06); //ST03 Sel
	rtd_outl(DVBC_FSM_WORD_reg, 0x43);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0x44);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0x8b);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x0e);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0xf2);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0xdd);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0xb5);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x44);
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa4);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0xf8); //orig: 0xf8 disable upd_b=0xd8 20181130
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xb2);
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x07);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x07); //ST03 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x08); //ST04 Sel
	rtd_outl(DVBC_FSM_WORD_reg, 0x54);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0x55);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0xcb);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x1e);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0xf3);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0x4d);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0xb5);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x44);
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa4);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0xf8); //orig: 0xf8 disable upd_b=0xd8 20181130
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xb2);
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x07);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x09); //ST04 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x0a); //ST05 Sel
	rtd_outl(DVBC_FSM_WORD_reg, 0x55);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0x55); //Fix at ST05
	rtd_outl(DVBC_FSM_WORD_2_reg, 0xcb);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x1e);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0xf5);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0x4b);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0xb4);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x44);
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa4);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0xf8); //orig: 0xf8 disable upd_b=0xd8 20181130
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xb2);
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x07);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x0b); //ST05 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x0c); //ST06 Sel
	rtd_outl(DVBC_FSM_WORD_reg, 0x76);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0x77);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0xcb);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x9e);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0xf7);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0xc7);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0x73);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x40);
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa4);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0xf8); //d8//orig: 0xf8 disable upd_b 20181130
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xb2);
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x07);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x0d); //ST06 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x12); //ST09 Sel
	rtd_outl(DVBC_FSM_WORD_reg, 0x99);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0xa9);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0xc9);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x4e);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0x48);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0x20);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0x71);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x40);
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa4);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0xfe);
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xb2);
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x07);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x13); //ST09 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x14); //ST10 Sel
	rtd_outl(DVBC_FSM_WORD_reg, 0xaa);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0xbb);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0xee);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x5e);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0x48);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0x20);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0x71);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x60);
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa6);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0xfe);
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xb2);
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x07);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x15); //ST10 Write

	rtd_outl(DVBC_FSM_WR_STB_reg, 0x18); //ST12 Sel
	rtd_outl(DVBC_FSM_WORD_reg, 0xc8);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0xcc);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0x00);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x7f);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0x28);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0xdc);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0x4b);
	//rtd_outl(DVBC_FSM_WORD_6_reg, 0xda);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x20);
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa9);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0x7f);
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xf3);
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x07);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x19); //ST12 Write
	
	pDemod->QamMode = QAM_QAM_256;

	return FUNCTION_SUCCESS;
}

int
realtek_r_dvbc_AutoModePostset(
	QAM_DEMOD_MODULE *pDemod
)
{
	//REALTEK_R_QAM_BASE_INFO("Restore ST05\n");
	//Restore ST05 FSM
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x0a); //ST05 Sel
	rtd_outl(DVBC_FSM_WORD_reg, 0x65);
	rtd_outl(DVBC_FSM_WORD_1_reg, 0x66);
	rtd_outl(DVBC_FSM_WORD_2_reg, 0xcb);
	rtd_outl(DVBC_FSM_WORD_3_reg, 0x1e);
	rtd_outl(DVBC_FSM_WORD_4_reg, 0xf5);
	rtd_outl(DVBC_FSM_WORD_5_reg, 0x4b);
	rtd_outl(DVBC_FSM_WORD_6_reg, 0xb4);
	rtd_outl(DVBC_FSM_WORD_7_reg, 0x44);
	rtd_outl(DVBC_FSM_WORD_8_reg, 0xa4);
	rtd_outl(DVBC_FSM_WORD_9_reg, 0xd8); //d8//orig: 0xf8 disable upd_b 20181130
	rtd_outl(DVBC_FSM_WORD_10_reg, 0xb2);
	rtd_outl(DVBC_FSM_WORD_11_reg, 0x07);
	rtd_outl(DVBC_FSM_WR_STB_reg, 0x0b); //ST05 Write

	return FUNCTION_SUCCESS;
}

int
realtek_r_dvbc_AutoModeSetSrHz(
	QAM_DEMOD_MODULE *pDemod,
	U32BITS SymbolRateHz
)
{
	typedef struct {
		U32BITS TrDeciRatioRangeMin;
		unsigned char SymbolRateReg0;
		U32BITS SymbolRateValue[11];
	}
	SYMBOL_RATE_ENTRY;

	//ADC36M TrDeciRatioRangeMin, SymbolRateReg0, {SymbolRateValue}
	static const SYMBOL_RATE_ENTRY SymbolRateTable[3] = {
		{5236000,		6,				{0x3fb,0x3fa,0x4,0x16,0x18,0x3f7,0x3c0,0x3b1,0xe,0xd5,0x1a6}},
		{4431000,		5,				{0x3,0xc,0x12,0x7,0x3e8,0x3c2,0x3b9,0x3f1,0x77,0x129,0x1c3}},
		{0,				4,				{0x9,0xc,0x5,0x3ee,0x3cf,0x3bd,0x3d3,0x25,0xaf,0x14f,0x1cf}},
	};

	int i = 0;
	MPI MpiCrystalFreqHz, MpiSymbolRateHz, MpiMiddleResult, MpiNone;
	U32BITS CrystalFreqHz = 0;
	const SYMBOL_RATE_ENTRY *pSymbolRateEntry;
	S32BITS TrDeciRatio = 0;
	unsigned char SymbolRateReg0 = 0;
	U32BITS SymbolRateValue = 0;
	memset(&pSymbolRateEntry, 0 , sizeof(pSymbolRateEntry));
	memset(&MpiCrystalFreqHz, 0 , sizeof(MpiCrystalFreqHz));
	memset(&MpiSymbolRateHz, 0 , sizeof(MpiSymbolRateHz));
	memset(&MpiMiddleResult, 0 , sizeof(MpiMiddleResult));
	memset(&MpiNone, 0 , sizeof(MpiNone));

	pSymbolRateEntry = &SymbolRateTable[0];
	for (i = 0; i < 3; i++) {
		if (SymbolRateHz >= SymbolRateTable[i].TrDeciRatioRangeMin) {
			pSymbolRateEntry = &SymbolRateTable[i];
			break;
		}
	}

	// Set TR_DECI_RATIO with calculated value.
	// Get demod crystal frequency in Hz.
	pDemod->GetCrystalFreqHz(pDemod, &CrystalFreqHz);

	// Calculate TR_DECI_RATIO value.
	// Note: TR_DECI_RATIO = (crystal frequency in Hz) * pow(2, 18) / (symbol rate in Hz)
	MpiSetValue(&MpiCrystalFreqHz, CrystalFreqHz);
	MpiSetValue(&MpiSymbolRateHz, SymbolRateHz);

	MpiLeftShift(&MpiMiddleResult, MpiCrystalFreqHz, REALTEK_R_QAM_TR_DECI_RATIO_FRAC_BIT_NUM);
	MpiDiv(&MpiMiddleResult, &MpiNone, MpiMiddleResult, MpiSymbolRateHz);
	MpiGetValue(MpiMiddleResult, &TrDeciRatio);
	rtd_maskl(DVBC_OPT_TR_DECI_RATIO_reg, ~DVBC_OPT_TR_DECI_RATIO_opt_tr_deci_ratio_7_0_mask, DVBC_OPT_TR_DECI_RATIO_opt_tr_deci_ratio_7_0(TrDeciRatio & 0xff));
	rtd_maskl(DVBC_OPT_TR_DECI_RATIO_1_reg, ~DVBC_OPT_TR_DECI_RATIO_1_opt_tr_deci_ratio_15_8_mask , DVBC_OPT_TR_DECI_RATIO_1_opt_tr_deci_ratio_15_8((TrDeciRatio & 0xff00) >> 8));
	rtd_maskl(DVBC_OPT_TR_DECI_RATIO_2_reg, ~DVBC_OPT_TR_DECI_RATIO_2_opt_tr_deci_ratio_23_16_mask , DVBC_OPT_TR_DECI_RATIO_2_opt_tr_deci_ratio_23_16((TrDeciRatio & 0xff0000) >> 16));
	rtd_maskl(DVBC_AUTO_SRS_reg, ~DVBC_AUTO_SRS_opt_tr_deci_ratio_auto_mask, DVBC_AUTO_SRS_opt_tr_deci_ratio_auto(0));//diable auto set tr deci
	
	// Set SPEC_SYMBOL_RATE_REG_0 value with determined symbol rate entry.
	SymbolRateReg0 = pSymbolRateEntry->SymbolRateReg0;
	rtd_maskl(DVBC_OPT_DDC_LPF_SCALING_reg, ~DVBC_OPT_DDC_LPF_SCALING_opt_ddc_lpf_scaling_mask, DVBC_OPT_DDC_LPF_SCALING_opt_ddc_lpf_scaling(SymbolRateReg0 & 0x7));
	// Set symbol rate value with determined symbol rate entry.
	for (i = 0; i < 11; i++) {
		// Get symbol rate value.
		SymbolRateValue = pSymbolRateEntry->SymbolRateValue[i];
		//REALTEK_R_QAM_BASE_INFO("symbolRateValue[i] = %u", i, SymbolRateValue);

		// Set symbol rate register selection.
		rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_sel_mask, DVBC_DDC_WR_STB_ddc_sel(i));
		// Set symbol rate register value.
		rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_coeff_2_0_mask, DVBC_DDC_WR_STB_ddc_coeff_2_0(SymbolRateValue & 0x07));
		rtd_maskl(DVBC_DDC_WR_STB_1_reg, ~DVBC_DDC_WR_STB_1_ddc_coeff_9_3_mask, DVBC_DDC_WR_STB_1_ddc_coeff_9_3((SymbolRateValue & 0x3F8) >> 3));

		// Set symbol rate register strobe.
		// Note: REALTEK_R QAM hardware will clear strobe automatically.
		rtd_maskl(DVBC_DDC_WR_STB_reg, ~DVBC_DDC_WR_STB_ddc_wr_stb_mask, DVBC_DDC_WR_STB_ddc_wr_stb(1));
	}
	
	// Set demod symbol rate parameter.
	pDemod->SymbolRateHz = SymbolRateHz;
	pDemod->IsSymbolRateHzSet = YES;

	return FUNCTION_SUCCESS;
}

/**

@see   QAM_DEMOD_FP_NBD_SRS

*/
#define DVBC_AUTUMODE_CSR_CNT 2
#define DVBC_AUTUMODE_FSR_CNT 1
#define DVBC_AUTUMODE_FSR_7P2_CNT 3
#define DVBC_AUTUMODE_TR_CNT 1
#define DVBC_AUTUMODE_CCFO_CNT 1
#define DVBC_AUTUMODE_LOW_SR_CNT 2
int
realtek_r_dvbc_AutoMode(
	QAM_DEMOD_MODULE *pDemod,
	unsigned char *pNosig,
	U32BITS *pCSR,
	U32BITS *pFSR,
	S32BITS *pCCFO,
	unsigned char *pQamMode
)
{
	unsigned long stime = 0;
	unsigned int srs_nbd_done = 0, srs_state = 0, srs_nbd_done_checked = 0;
	unsigned int fsm_state_o = 0, tr_ld_o = 0, cr_ld_o = 0;
	unsigned int csr_cfo_center = 0;
	unsigned int PosToneMaxIdx[3] = {0}, NegToneMaxIdx[3] = {0}, KeepNewTrRatioRst[3] = {0};
	unsigned int srs_cnt = 0, csr_rst_l = 0, csr_rst_r = 0, csr_KHz = 0, fsr_KHz = 0, fsr_tmp = 0;
	unsigned int ccfo_done = 0, ccfo_max_value = 0, est_ccfo_idx = 0, papr_ccfo_max_value = 0, est_papr_ccfo_idx = 0;
	unsigned int qam_done = 0, qam_mode = 0;
	unsigned char qamd_result_1 = 0xaa, qamd_result_2 = 0xbb, qamd_result_3 = 0xcc;
	unsigned char csr_retry_cnt = 0, fsr_7p2_retry_cnt = 0, fsr_retry_cnt = 0, tr_retry_cnt = 0, ccfo_retry_cnt = 0, low_sr_retry_cnt = 0, srs_retry_cnt = 0;
	int est_ccfo = 0, CSR_CFO = 0, CCFO_CFO = 0, csr_cfo_diff = 0, est_papr_ccfo = 0, PAPR_CCFO_CFO = 0, csr_papr_cfo_diff = 0;
	unsigned int tr_threshold = pDemod->TrLdThreshold;
	unsigned int automode_srs_cnt = pDemod->AutoModeSRSCnt;

	pDemod->AutoModePreset(pDemod);
	
	rtd_outl(DVBC_OPT_INITIAL_CR_FREQ_reg, 0x0);//Reset init CR CFO
	rtd_outl(DVBC_OPT_INITIAL_CR_FREQ_1_reg, 0x0);
	rtd_maskl(DVBC_FFT_ACC_NUM_AUTO_2_reg, ~DVBC_FFT_ACC_NUM_AUTO_2_opt_fft_output_acc_num_fsrs_mask, 0x1e);//increase FSR acc num to 31
	rtd_maskl(DVBC_OPT_START_SRS_NBD_reg, ~DVBC_OPT_START_SRS_NBD_opt_ccfo_auto_mask, DVBC_OPT_START_SRS_NBD_opt_ccfo_auto(1));//enable ccfo update
	rtd_maskl(DVBC_OPT_RST_STATE0_ON_reg, ~DVBC_OPT_RST_STATE0_ON_opt_retry_on_mask, DVBC_OPT_RST_STATE0_ON_opt_retry_on(0));//retry off
	rtd_outl(DVBC_AUTO_SRS_reg, 0x15);//[0]Auto srs start [2]Auto set tr_deci_ratio [4]Auto CSR enable
	
	rtd_maskl(DVBC_CSRS_CSR_THD_WEIGHT_reg, ~(DVBC_CSRS_CSR_THD_WEIGHT_reg_csrs_csr_threshold_weight_mask), DVBC_CSRS_CSR_THD_WEIGHT_reg_csrs_csr_threshold_weight(0x4)); //reset csr th
	rtd_maskl(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_1_reg, ~(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_1_reg_fsrs_search_range_half_add_11_8_mask), DVBC_OPT_FSRS_SEARCH_RANGE_HALF_1_reg_fsrs_search_range_half_add_11_8(0x0));	//[11:8]larger FSR search range half add
	rtd_maskl(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_reg, ~(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_reg_fsrs_search_range_half_add_7_0_mask), DVBC_OPT_FSRS_SEARCH_RANGE_HALF_reg_fsrs_search_range_half_add_7_0(0xa0));	//[7:0] FSR search range half add 0x72=500K 0x89=600K 0xe4=1000K 0xcd=900K 0x94=650K 0xa0=700K
	rtd_maskl(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_SUB1_reg, ~(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_SUB1_reg_fsrs_search_range_half_sub_11_8_mask), DVBC_OPT_FSRS_SEARCH_RANGE_HALF_SUB1_reg_fsrs_search_range_half_sub_11_8(0x0));	//[11:8]larger FSR search range half sub
	rtd_maskl(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_SUB_reg, ~(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_SUB_reg_fsrs_search_range_half_sub_7_0_mask), DVBC_OPT_FSRS_SEARCH_RANGE_HALF_SUB_reg_fsrs_search_range_half_sub_7_0(0xab));	//[7:0] FSR search range half sub 0x72=500K 0x89=600K 0xe4=1000K 0xcd=900K 0x94=650K 0xa0=700K 0xab=750K
	
AutoModeBegin:
	srs_nbd_done_checked = 0;
	*pNosig = 0;
	pDemod->SoftwareReset(pDemod);

	stime = tv_osal_time();
	while ((tv_osal_time() - stime) < 300)
	{
		srs_nbd_done = DVBC_SRS_NBD_DONE_get_srs_nbd_done(rtd_inl(DVBC_SRS_NBD_DONE_reg));
		ccfo_done = DVBC_SRS_NBD_DONE_get_ccfo_done(rtd_inl(DVBC_SRS_NBD_DONE_reg));
		rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));
		fsm_state_o = (rtd_inl(DVBC_INRX_MO2_reg) & 0x78) >> 3;
		tr_ld_o = DVBC_INRX_MO2_1_get_inrx_mo2_12_7(rtd_inl(DVBC_INRX_MO2_1_reg));
		srs_state = rtd_inl(DVBC_AUTO_SRS_STATE_reg);
		//REALTEK_R_QAM_BASE_INFO("[AutoMode] srs_nbd_done=%u, ccfo_done=%u, fsm_state=%u, tr_ld_o=%u, srs_state=0x%x, time="PT_U32BITS"\n", srs_nbd_done, ccfo_done, fsm_state_o, tr_ld_o, srs_state, tv_osal_time() - stime);
		REALTEK_R_QAM_BASE_INFO("\033[1;32;33m" "[AutoMode] tr_ld_o threshold=%u\n" "\033[m", tr_threshold);

		if(srs_nbd_done == 0x1 && srs_nbd_done_checked == 0x0)
		{
			REALTEK_R_QAM_BASE_INFO("[AutoMode] srs_nbd_done=%u, ccfo_done=%u, fsm_state=%u, tr_ld_o=%u, srs_state=0x%x, time="PT_U32BITS"\n", srs_nbd_done, ccfo_done, fsm_state_o, tr_ld_o, srs_state, tv_osal_time() - stime);
			srs_nbd_done_checked = 1;
			
			//Get CSR
			srs_cnt = DVBC_CSRS_RESULT_get_cp_cnt(rtd_inl(DVBC_CSRS_RESULT_reg)); //[2:0] number of search
			//CSR_KHz = (cp_r -cp_l) /8192 *ADC_clk_KHz
			csr_rst_r = ((DVBC_CSRS_RESULT0_1_get_cp_r_0_12_8(rtd_inl(DVBC_CSRS_RESULT0_1_reg)) << 8) | DVBC_CSRS_RESULT0_0_get_cp_r_0_7_0(rtd_inl(DVBC_CSRS_RESULT0_0_reg)));
			csr_rst_l = ((DVBC_CSRS_RESULT0_3_get_cp_l_0_12_8(rtd_inl(DVBC_CSRS_RESULT0_3_reg)) << 8) | DVBC_CSRS_RESULT0_2_get_cp_l_0_7_0(rtd_inl(DVBC_CSRS_RESULT0_2_reg)));
			csr_KHz = ((csr_rst_r - csr_rst_l)*SAMPLE_RATE_KHZ) / FFT_SIZE ;
			csr_cfo_center = (csr_rst_r + csr_rst_l)/2;
			REALTEK_R_QAM_BASE_INFO("[CSR] srs_cnt=%u, csr_KHz=%u \n", srs_cnt, csr_KHz);
			*pCSR = csr_KHz;
			
			//Get FSR
			PosToneMaxIdx[0] = (DVBC_POS_TONE_MAX_IDX_0_1_get_pos_tone_max_idx_0_11_8(rtd_inl(DVBC_POS_TONE_MAX_IDX_0_1_reg)) << 8) | DVBC_POS_TONE_MAX_IDX_0_get_pos_tone_max_idx_0_7_0(rtd_inl(DVBC_POS_TONE_MAX_IDX_0_reg));
			PosToneMaxIdx[1] = (DVBC_POS_TONE_MAX_IDX_1_1_get_pos_tone_max_idx_1_11_8(rtd_inl(DVBC_POS_TONE_MAX_IDX_1_1_reg)) << 8) | DVBC_POS_TONE_MAX_IDX_1_get_pos_tone_max_idx_1_7_0(rtd_inl(DVBC_POS_TONE_MAX_IDX_1_reg));
			PosToneMaxIdx[2] = (DVBC_POS_TONE_MAX_IDX_2_1_get_pos_tone_max_idx_2_11_8(rtd_inl(DVBC_POS_TONE_MAX_IDX_2_1_reg)) << 8) | DVBC_POS_TONE_MAX_IDX_2_get_pos_tone_max_idx_2_7_0(rtd_inl(DVBC_POS_TONE_MAX_IDX_2_reg));
			NegToneMaxIdx[0] = (DVBC_NEG_TONE_MAX_IDX_0_1_get_neg_tone_max_idx_0_11_8(rtd_inl(DVBC_NEG_TONE_MAX_IDX_0_1_reg)) << 8) | DVBC_NEG_TONE_MAX_IDX_0_get_neg_tone_max_idx_0_7_0(rtd_inl(DVBC_NEG_TONE_MAX_IDX_0_reg));
			NegToneMaxIdx[1] = (DVBC_NEG_TONE_MAX_IDX_1_1_get_neg_tone_max_idx_1_11_8(rtd_inl(DVBC_NEG_TONE_MAX_IDX_1_1_reg)) << 8) | DVBC_NEG_TONE_MAX_IDX_1_get_neg_tone_max_idx_1_7_0(rtd_inl(DVBC_NEG_TONE_MAX_IDX_1_reg));
			NegToneMaxIdx[2] = (DVBC_NEG_TONE_MAX_IDX_2_1_get_neg_tone_max_idx_2_11_8(rtd_inl(DVBC_NEG_TONE_MAX_IDX_2_1_reg)) << 8) | DVBC_NEG_TONE_MAX_IDX_2_get_neg_tone_max_idx_2_7_0(rtd_inl(DVBC_NEG_TONE_MAX_IDX_2_reg));
			KeepNewTrRatioRst[0] = DVBC_TR_DECI_RATIO_SRS_0_get_tr_deci_ratio_fsrs_7_0(rtd_inl(DVBC_TR_DECI_RATIO_SRS_0_reg));
			KeepNewTrRatioRst[1] = DVBC_TR_DECI_RATIO_SRS_1_get_tr_deci_ratio_fsrs_15_8(rtd_inl(DVBC_TR_DECI_RATIO_SRS_1_reg));
			KeepNewTrRatioRst[2] = DVBC_TR_DECI_RATIO_SRS_2_get_tr_deci_ratio_fsrs_23_16(rtd_inl(DVBC_TR_DECI_RATIO_SRS_2_reg));
			REALTEK_R_QAM_BASE_INFO("New TR deci 0x%x%02x%02x, POS 0x%x, 0x%x, 0x%x, NEG 0x%x, 0x%x, 0x%x\n", KeepNewTrRatioRst[2], KeepNewTrRatioRst[1], KeepNewTrRatioRst[0], PosToneMaxIdx[0], PosToneMaxIdx[1], PosToneMaxIdx[2], NegToneMaxIdx[0], NegToneMaxIdx[1], NegToneMaxIdx[2]);
			
			if(csr_KHz > 8100)//Check CSR for AGC lock
			{
				csr_retry_cnt++;
				if(csr_retry_cnt <= DVBC_AUTUMODE_CSR_CNT)
				{
					REALTEK_R_QAM_BASE_INFO("\033[1;32;32m" "CSR too large, AGC unstable, retry %u times\n" "\033[m", csr_retry_cnt);
					//tv_osal_msleep(10);
					goto AutoModeBegin;
				}
				else
				{
					REALTEK_R_QAM_BASE_INFO("\033[1;32;32m" "CSR too large, AGC unstable retry over %d times\n" "\033[m", DVBC_AUTUMODE_CSR_CNT);
					*pNosig = 1;
					goto AutoModeEnd;
				}
			}
			
			if((csr_KHz > 7740 && csr_KHz < 9000) || (csr_KHz > 0 && csr_KHz < 3100))//CSR over normal range
			{
				REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "((csr_KHz > 7740 && csr_KHz < 9000) || (csr_KHz > 0 && csr_KHz < 3100))\n" "\033[m");
				*pNosig = 1;
				goto AutoModeEnd;
			}
			
			if((KeepNewTrRatioRst[0] == 0xff) && (KeepNewTrRatioRst[1] == 0xff) && (KeepNewTrRatioRst[2] == 0xff))
			{
				REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "FSR all 0xff\n" "\033[m");
				
				if(csr_KHz > 7400 && csr_KHz < 7650)
				{
					CSR_CFO = ((int)csr_cfo_center-4096)*SAMPLE_RATE_KHZ/FFT_SIZE;
					REALTEK_R_QAM_BASE_INFO("csr_KHz=%u, CSR_CFO=%d\n", csr_KHz, CSR_CFO);
					if((CSR_CFO < -100 && CSR_CFO > -310) || (CSR_CFO < 310 && CSR_CFO > 100))
					{
						REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "SR 7.2M WA" "\033[m");
						fsr_7p2_retry_cnt++;
						if(fsr_7p2_retry_cnt <= DVBC_AUTUMODE_FSR_7P2_CNT)
						{
							REALTEK_R_QAM_BASE_INFO("FSR 7p2 not found, retry %u times\n", fsr_7p2_retry_cnt);
							goto AutoModeBegin;
						}
						else
						{
							REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "FSR 7p2 not found retry over %d times\n" "\033[m", DVBC_AUTUMODE_FSR_7P2_CNT);
							*pNosig = 1;
							goto AutoModeEnd;
						}
					}
					else
					{
						REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "SR 7M WA" "\033[m");
						fsr_7p2_retry_cnt++;
						if(fsr_7p2_retry_cnt <= DVBC_AUTUMODE_FSR_7P2_CNT)
						{
							REALTEK_R_QAM_BASE_INFO("FSR 7M not found, retry %u times\n", fsr_7p2_retry_cnt);
							goto AutoModeBegin;
						}
						else
						{
							REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "FSR 7M not found retry over %d times\n" "\033[m", DVBC_AUTUMODE_FSR_7P2_CNT);
							*pNosig = 1;
							goto AutoModeEnd;
						}
					}
				}
				else
				{
					fsr_retry_cnt++;
					if((fsr_retry_cnt <= DVBC_AUTUMODE_FSR_CNT) && (csr_KHz > 5000 && csr_KHz < 6000))
					{
						REALTEK_R_QAM_BASE_INFO("FSR not found, retry %u times\n", fsr_retry_cnt);
						goto AutoModeBegin;
					}
					else
					{
						REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "CSR isn't in retry range or FSR not found retry over %d times\n" "\033[m", DVBC_AUTUMODE_FSR_CNT);
						*pNosig = 1;
						goto AutoModeEnd;
					}
				}
			}
			else
			{
				fsr_tmp = ((KeepNewTrRatioRst[2] << 16) | (KeepNewTrRatioRst[1] << 8) | KeepNewTrRatioRst[0])/10;
				fsr_KHz = (SAMPLE_RATE_10KHZ * 262144 / fsr_tmp);
				REALTEK_R_QAM_BASE_INFO("[FSR] fsr_tmp=%u, FSR=%u KHz\n", fsr_tmp, fsr_KHz);
				*pFSR = fsr_KHz *1000;
				
				if((low_sr_retry_cnt <= DVBC_AUTUMODE_LOW_SR_CNT) && (csr_KHz >= 3100 && csr_KHz < 4410 && fsr_KHz >= 4499 && fsr_KHz <= 4501))
				{
					low_sr_retry_cnt++;
					REALTEK_R_QAM_BASE_INFO("tuner low symbolrate fsr retry %u/%d times\n", low_sr_retry_cnt, DVBC_AUTUMODE_LOW_SR_CNT);
					rtd_maskl(DVBC_CSRS_CSR_THD_WEIGHT_reg, ~(DVBC_CSRS_CSR_THD_WEIGHT_reg_csrs_csr_threshold_weight_mask), DVBC_CSRS_CSR_THD_WEIGHT_reg_csrs_csr_threshold_weight(0xa)); //reset csr th
					rtd_maskl(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_1_reg, ~(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_1_reg_fsrs_search_range_half_add_11_8_mask), DVBC_OPT_FSRS_SEARCH_RANGE_HALF_1_reg_fsrs_search_range_half_add_11_8(0x0));	//[11:8]larger FSR search range half add
					rtd_maskl(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_reg, ~(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_reg_fsrs_search_range_half_add_7_0_mask), DVBC_OPT_FSRS_SEARCH_RANGE_HALF_reg_fsrs_search_range_half_add_7_0(0x40));	//[7:0] FSR search range half add 0x72=500K 0x89=600K 0xe4=1000K 0xcd=900K 0x94=650K 0xa0=700K
					rtd_maskl(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_SUB1_reg, ~(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_SUB1_reg_fsrs_search_range_half_sub_11_8_mask), DVBC_OPT_FSRS_SEARCH_RANGE_HALF_SUB1_reg_fsrs_search_range_half_sub_11_8(0x0));	//[11:8]larger FSR search range half sub
					rtd_maskl(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_SUB_reg, ~(DVBC_OPT_FSRS_SEARCH_RANGE_HALF_SUB_reg_fsrs_search_range_half_sub_7_0_mask), DVBC_OPT_FSRS_SEARCH_RANGE_HALF_SUB_reg_fsrs_search_range_half_sub_7_0(0x40));	//[7:0] FSR search range half sub 0x72=500K 0x89=600K 0xe4=1000K 0xcd=900K 0x94=650K 0xa0=700K 0xab=750K
					goto AutoModeBegin;
				}
			}
		}
		else if(srs_nbd_done == 0x0 && srs_nbd_done_checked == 0x0 && (tv_osal_time() - stime) > 60)
		{
			srs_retry_cnt ++;
			if(srs_retry_cnt <= automode_srs_cnt)
			{
				REALTEK_R_QAM_BASE_INFO("[AutoMode] srs_nbd_done=%u, ccfo_done=%u, fsm_state=%u, tr_ld_o=%u, srs_state=0x%x, time="PT_U32BITS"\n", srs_nbd_done, ccfo_done, fsm_state_o, tr_ld_o, srs_state, tv_osal_time() - stime);
				REALTEK_R_QAM_BASE_INFO("SRS not done, retry %u times\n", srs_retry_cnt);
				goto AutoModeBegin;
			}
			else
			{
				REALTEK_R_QAM_BASE_INFO("[AutoMode] srs_nbd_done=%u, ccfo_done=%u, fsm_state=%u, tr_ld_o=%u, srs_state=0x%x, time="PT_U32BITS"\n", srs_nbd_done, ccfo_done, fsm_state_o, tr_ld_o, srs_state, tv_osal_time() - stime);
				REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "srs_nbd_done = 0\n" "\033[m");
				*pNosig = 1;
				goto AutoModeEnd;
			}
		}
		else if(srs_nbd_done == 0x1 && ccfo_done == 0x1 && fsm_state_o >= 4 && fsm_state_o < 14)
		{
			REALTEK_R_QAM_BASE_INFO("[AutoMode] srs_nbd_done=%u, ccfo_done=%u, fsm_state=%u, tr_ld_o=%u, srs_state=0x%x, time="PT_U32BITS"\n", srs_nbd_done, ccfo_done, fsm_state_o, tr_ld_o, srs_state, tv_osal_time() - stime);
			//if(tr_ld_o < 32)
			if(tr_ld_o < tr_threshold) //default tr_threshold = 32
			{
				ccfo_max_value = (rtd_inl(DVBC_FFTOUT_MAX0_reg) | (rtd_inl(DVBC_FFTOUT_MAX1_reg) << 8));
				est_ccfo_idx = (DVBC_FFTOUT_MAX1_get_ccfo_local_max_val_15_8(rtd_inl(DVBC_FFTOUT_MAX3_reg)) << 8)  | (DVBC_FFTOUT_MAX2_get_ccfo_local_max_idx_7_0(rtd_inl(DVBC_FFTOUT_MAX2_reg)));
				//CCFO_CFO = est_ccfo_idx*36000/4/8192;
				//CCFO_CFO = (est_ccfo_idx-8192)*36000/4/8192;
				CCFO_CFO = (est_ccfo_idx < 4096) ? ((int)est_ccfo_idx*9000/8192) : (((int)est_ccfo_idx - 8192)*9000/8192);
				CSR_CFO = ((int)csr_cfo_center-4096)*SAMPLE_RATE_KHZ/FFT_SIZE;
				csr_cfo_diff = ABS(CSR_CFO, CCFO_CFO);
				est_ccfo = CCFO_CFO * 1000;
				
				//New PAPR CCFO
				papr_ccfo_max_value = (rtd_inl(DVBC_CCFO_PAPR_MAX0_reg) | (rtd_inl(DVBC_CCFO_PAPR_MAX1_reg) << 8));
				est_papr_ccfo_idx = ((rtd_inl(DVBC_CCFO_PAPR_IDX1_reg) & DVBC_CCFO_PAPR_IDX1_ccfo_max_idx_12_8_mask) << 8) + (rtd_inl(DVBC_CCFO_PAPR_IDX0_reg) & DVBC_CCFO_PAPR_IDX0_ccfo_max_idx_7_0_mask);
				//PAPR_CCFO_CFO = est_papr_ccfo_idx*36000/4/8192;
				//PAPR_CCFO_CFO = (est_papr_ccfo_idx-8192)*36000/4/8192;
				PAPR_CCFO_CFO = (est_papr_ccfo_idx < 4096) ? ((int)est_papr_ccfo_idx*9000/8192) : (((int)est_papr_ccfo_idx - 8192)*9000/8192);
				//CSR_CFO = ((int)csr_cfo_center-4096)*SAMPLE_RATE_KHZ/FFT_SIZE;
				csr_papr_cfo_diff = ABS(CSR_CFO, PAPR_CCFO_CFO);
				est_papr_ccfo = PAPR_CCFO_CFO * 1000;
				
				REALTEK_R_QAM_BASE_INFO("ORI CSR_CFO=%d, CCFO_CFO=%d, diff=%d KHz\n", CSR_CFO, CCFO_CFO, csr_cfo_diff);
				REALTEK_R_QAM_BASE_INFO("NEW CSR_CFO=%d, PAPR_CCFO_CFO=%d, diff=%d KHz\n", CSR_CFO, PAPR_CCFO_CFO, csr_papr_cfo_diff);
				
				if(csr_papr_cfo_diff < 550)//KHz
				{
					REALTEK_R_QAM_BASE_INFO("ABS(CSR, PAPR_CCFO)<550 KHz, Compare OK\n");
					REALTEK_R_QAM_BASE_INFO("[PAPR CCFO] ccfo_done=0x%x, papr_ccfo_max_value=%u, est_papr_ccfo_idx=%u, est_papr_ccfo=%d Hz\n", ccfo_done, papr_ccfo_max_value, est_papr_ccfo_idx, est_papr_ccfo);
					*pCCFO = est_papr_ccfo;
				}
				else
				{
					REALTEK_R_QAM_BASE_INFO("ccfo_retry_cnt=%u, Diff=%u PAPR_CCFO error....try again\n", ccfo_retry_cnt, csr_papr_cfo_diff);
					REALTEK_R_QAM_BASE_INFO("[PAPR CCFO] ccfo_done=0x%x, papr_ccfo_max_value=%u, est_papr_ccfo_idx=%u, est_papr_ccfo=%d \n", ccfo_done, papr_ccfo_max_value, est_papr_ccfo_idx, est_papr_ccfo);

					ccfo_retry_cnt++;
					if(ccfo_retry_cnt <= DVBC_AUTUMODE_CCFO_CNT)
					{
						//set SR, disable auto update
						rtd_maskl(DVBC_OPT_TR_DECI_RATIO_reg, ~DVBC_OPT_TR_DECI_RATIO_opt_tr_deci_ratio_7_0_mask, KeepNewTrRatioRst[0]); //[7:0]opt_tr_deci_ratio[7:0], 2*symbol rate/ADC frequency
						rtd_maskl(DVBC_OPT_TR_DECI_RATIO_1_reg, ~DVBC_OPT_TR_DECI_RATIO_1_opt_tr_deci_ratio_15_8_mask, KeepNewTrRatioRst[1]); //[7:0]opt_tr_deci_ratio[15:8], 2*symbol rate/ADC frequency
						rtd_maskl(DVBC_OPT_TR_DECI_RATIO_2_reg, ~DVBC_OPT_TR_DECI_RATIO_2_opt_tr_deci_ratio_23_16_mask, KeepNewTrRatioRst[2]); //[7:0]opt_tr_deci_ratio[23:16], 2*symbol rate/ADC frequency
						rtd_maskl(DVBC_AUTO_SRS_reg, ~DVBC_AUTO_SRS_opt_tr_deci_ratio_auto_mask, DVBC_AUTO_SRS_opt_tr_deci_ratio_auto(0));//diable auto set tr deci
						
						REALTEK_R_QAM_BASE_INFO("PAPR CCFO vs CSR not correct(fill FSR correct ratio), retry %u times\n", ccfo_retry_cnt);
						goto AutoModeBegin;
					}
					else
					{
						REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "PAPR CCFO vs CSR not correct, retry over %d times\n" "\033[m", DVBC_AUTUMODE_CCFO_CNT);
						*pNosig = 1;
						goto AutoModeEnd;
					}
				}
				break;
			}
			else //tr_ld_o >= 32
			{
				tr_retry_cnt++;
				if(tr_retry_cnt <= DVBC_AUTUMODE_TR_CNT)
				{
					REALTEK_R_QAM_BASE_INFO("TR not correct, retry %u times\n", tr_retry_cnt);
					goto AutoModeBegin;
				}
				else
				{
					REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "TR not correct, retry over %d times\n" "\033[m", DVBC_AUTUMODE_TR_CNT);
					*pNosig = 1;
					goto AutoModeEnd;
				}
			}
		}
		tv_osal_msleep(5);
	}
	if(srs_nbd_done == 0x0 || ccfo_done == 0x0)
	{
		REALTEK_R_QAM_BASE_INFO("[AutoMode] srs_nbd_done=%u, ccfo_done=%u, fsm_state=%u, tr_ld_o=%u, srs_state=0x%x, time="PT_U32BITS"\n", srs_nbd_done, ccfo_done, fsm_state_o, tr_ld_o, srs_state, tv_osal_time() - stime);
		REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "(srs_nbd_done == 0x0 || ccfo_done == 0x0)\n" "\033[m");
		*pNosig = 1;
		goto AutoModeEnd;
	}
	
	//Check QAMD done
	REALTEK_R_QAM_BASE_INFO("[QAMD 1]***************\n");
	qam_done = 0;
	stime = tv_osal_time();
	while((tv_osal_time() - stime) < 60)
	{
		qam_done = DVBC_REG_QAMD_DONE_get_reg_qamd_done(rtd_inl(DVBC_REG_QAMD_DONE_reg)); //get qamd_done flag
		rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));//stb INRX_MO
		fsm_state_o = (rtd_inl(DVBC_INRX_MO2_reg) & 0x78) >> 3;
		tr_ld_o = DVBC_INRX_MO2_1_get_inrx_mo2_12_7(rtd_inl(DVBC_INRX_MO2_1_reg));
		cr_ld_o = DVBC_INRX_MO2_2_get_inrx_mo2_18_13(rtd_inl(DVBC_INRX_MO2_2_reg));
		srs_state = rtd_inl(DVBC_AUTO_SRS_STATE_reg);
		
		if(qam_done == 1) 
		{
			REALTEK_R_QAM_BASE_INFO("qam_done=0x%x, fsm_state_o=%u, srs_state=0x%x, tr_ld_o=%u, cr_ld_o=%u, time="PT_U32BITS"\n", qam_done, fsm_state_o, srs_state, tr_ld_o, cr_ld_o, tv_osal_time() - stime);
			
			qam_mode = DVBC_QAM_CON_BIT_DET_get_qam_con_bit_det_1(rtd_inl(DVBC_QAM_CON_BIT_DET_reg)); //get QAM detect
			REALTEK_R_QAM_BASE_INFO("qam_mode=0x%x\n", qam_mode);
			qamd_result_1 = qam_mode;
			break;
		}
		tv_osal_msleep(5);
	}
	if(qam_done == 0)
	{
		REALTEK_R_QAM_BASE_INFO("qam_done=0x%x, fsm_state_o=%u, srs_state=0x%x, tr_ld_o=%u, cr_ld_o=%u, time="PT_U32BITS"\n", qam_done, fsm_state_o, srs_state, tr_ld_o, cr_ld_o, tv_osal_time() - stime);
		REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "qam_done == 0\n" "\033[m");
		*pNosig = 1;
		goto AutoModeEnd;
	}

	REALTEK_R_QAM_BASE_INFO("[QAMD 2]***************\n");
	//QAMD restart
	rtd_maskl(DVBC_OPT_QAMD_SELF_RESET_reg, ~DVBC_OPT_QAMD_SELF_RESET_opt_qamd_self_reset_mask, 0x0); //reset auto qam
	rtd_maskl(DVBC_OPT_QAMD_SELF_RESET_reg, ~DVBC_OPT_QAMD_SELF_RESET_opt_qamd_self_reset_mask, 0x1); //reset auto qam
	
	qam_done = 0;
	stime = tv_osal_time();
	while((tv_osal_time() - stime) < 60)
	{
		qam_done = DVBC_REG_QAMD_DONE_get_reg_qamd_done(rtd_inl(DVBC_REG_QAMD_DONE_reg)); //get qamd_done flag
		rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));//stb INRX_MO
		fsm_state_o = (rtd_inl(DVBC_INRX_MO2_reg) & 0x78) >> 3;
		tr_ld_o = DVBC_INRX_MO2_1_get_inrx_mo2_12_7(rtd_inl(DVBC_INRX_MO2_1_reg));
		cr_ld_o = DVBC_INRX_MO2_2_get_inrx_mo2_18_13(rtd_inl(DVBC_INRX_MO2_2_reg));
		srs_state = rtd_inl(DVBC_AUTO_SRS_STATE_reg);
		
		if(qam_done == 1) 
		{
			REALTEK_R_QAM_BASE_INFO("qam_done=0x%x, fsm_state_o=%u, srs_state=0x%x, tr_ld_o=%u, cr_ld_o=%u, time="PT_U32BITS"\n", qam_done, fsm_state_o, srs_state, tr_ld_o, cr_ld_o, tv_osal_time() - stime);
			
			qam_mode = DVBC_QAM_CON_BIT_DET_get_qam_con_bit_det_1(rtd_inl(DVBC_QAM_CON_BIT_DET_reg)); //get QAM detect
			REALTEK_R_QAM_BASE_INFO("qam_mode=0x%x\n", qam_mode);
			qamd_result_2 = qam_mode;
			break;
		}
		tv_osal_msleep(5);
	}
	if(qam_done == 0)
	{
		REALTEK_R_QAM_BASE_INFO("qam_done=0x%x, fsm_state_o=%u, srs_state=0x%x, tr_ld_o=%u, cr_ld_o=%u, time="PT_U32BITS"\n", qam_done, fsm_state_o, srs_state, tr_ld_o, cr_ld_o, tv_osal_time() - stime);
		REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "qam_done == 0\n" "\033[m");
		*pNosig = 1;
		goto AutoModeEnd;
	}
	
	if(qamd_result_1 == qamd_result_2)
	{
		REALTEK_R_QAM_BASE_INFO("break qamd\n")
		*pNosig = 0;
		goto AutoModeEnd;
	}
	else
	{
		REALTEK_R_QAM_BASE_INFO("need do third qamd\n")
	}
	
	REALTEK_R_QAM_BASE_INFO("[QAMD 3]***************\n");
	//QAMD restart
	rtd_maskl(DVBC_OPT_QAMD_SELF_RESET_reg, ~DVBC_OPT_QAMD_SELF_RESET_opt_qamd_self_reset_mask, 0x0); //reset auto qam
	rtd_maskl(DVBC_OPT_QAMD_SELF_RESET_reg, ~DVBC_OPT_QAMD_SELF_RESET_opt_qamd_self_reset_mask, 0x1); //reset auto qam
	
	qam_done = 0;
	stime = tv_osal_time();
	while((tv_osal_time() - stime) < 60)
	{
		qam_done = DVBC_REG_QAMD_DONE_get_reg_qamd_done(rtd_inl(DVBC_REG_QAMD_DONE_reg)); //get qamd_done flag
		rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));//stb INRX_MO
		fsm_state_o = (rtd_inl(DVBC_INRX_MO2_reg) & 0x78) >> 3;
		tr_ld_o = DVBC_INRX_MO2_1_get_inrx_mo2_12_7(rtd_inl(DVBC_INRX_MO2_1_reg));
		cr_ld_o = DVBC_INRX_MO2_2_get_inrx_mo2_18_13(rtd_inl(DVBC_INRX_MO2_2_reg));
		srs_state = rtd_inl(DVBC_AUTO_SRS_STATE_reg);
		
		if(qam_done == 1) 
		{
			srs_state = rtd_inl(DVBC_AUTO_SRS_STATE_reg);
			REALTEK_R_QAM_BASE_INFO("qam_done=0x%x, fsm_state_o=%u, srs_state=0x%x, tr_ld_o=%u, cr_ld_o=%u, time="PT_U32BITS"\n", qam_done, fsm_state_o, srs_state, tr_ld_o, cr_ld_o, tv_osal_time() - stime);
			
			qam_mode = DVBC_QAM_CON_BIT_DET_get_qam_con_bit_det_1(rtd_inl(DVBC_QAM_CON_BIT_DET_reg)); //get QAM detect
			REALTEK_R_QAM_BASE_INFO("qam_mode=0x%x\n", qam_mode);
			qamd_result_3 = qam_mode;
			break;
		}
		tv_osal_msleep(5);
	}
	if(qam_done == 0)
	{
		REALTEK_R_QAM_BASE_INFO("qam_done=0x%x, fsm_state_o=%u, srs_state=0x%x, tr_ld_o=%u, cr_ld_o=%u, time="PT_U32BITS"\n", qam_done, fsm_state_o, srs_state, tr_ld_o, cr_ld_o, tv_osal_time() - stime);
		REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "qam_done == 0\n" "\033[m");
		*pNosig = 1;
		goto AutoModeEnd;
	}
	
AutoModeEnd:
	if(*pNosig == 1)
	{
		REALTEK_R_QAM_BASE_INFO("\033[1;32;31m" "No Signal Stop Demod!!!\n" "\033[m");
		rtd_maskl(DVBC_OPT_QAM_ON_reg, ~DVBC_OPT_QAM_ON_opt_qam_on_mask, DVBC_OPT_QAM_ON_opt_qam_on(0));
		rtd_maskl(DVBC_OPT_SOFT_RESET_QAM_reg, ~DVBC_OPT_SOFT_RESET_QAM_opt_soft_reset_qam_mask, DVBC_OPT_SOFT_RESET_QAM_opt_soft_reset_qam(1));
	}

	if(qamd_result_1 == qamd_result_2)
		*pQamMode = qamd_result_1;
	else if(qamd_result_2 == qamd_result_3)
		*pQamMode = qamd_result_2;
	else if(qamd_result_1 == qamd_result_3)
		*pQamMode = qamd_result_1;
	else
		*pQamMode = qamd_result_3;
	
	REALTEK_R_QAM_BASE_INFO("Final QAMD 0x%x (0x%x, 0x%x, 0x%x)\n", *pQamMode, qamd_result_1, qamd_result_2, qamd_result_3);
	REALTEK_R_QAM_BASE_INFO("Auto Mode Finish [CSR]="PT_U32BITS" KHz, [FSR]="PT_U32BITS" Hz, [QAM]=0x%x\n", *pCSR, *pFSR, *pQamMode);
	
	pDemod->AutoModePostset(pDemod);
	
	return FUNCTION_SUCCESS;
}

/**

@see   I2C_BRIDGE_FP_FORWARD_I2C_READING_CMD

*/
/*
int
realtek_r_qam_ForwardI2cReadingCmd(
	I2C_BRIDGE_MODULE *pI2cBridge,
	unsigned char DeviceAddr,
	unsigned char *pReadingBytes,
	U32BITS ByteNum
)
{
	QAM_DEMOD_MODULE *pDemod;
	BASE_INTERFACE_MODULE *pBaseInterface;

	// Get demod module and tuner device address.
	pDemod = (QAM_DEMOD_MODULE *)pI2cBridge->pPrivateData;

	// Get base interface.
	pBaseInterface = pDemod->pBaseInterface;

	// Enable demod I2C relay.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_OPT_I2C_RELAY, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	// Send I2C reading command.
	if (pBaseInterface->I2cRead(pBaseInterface, DeviceAddr, pReadingBytes, ByteNum) != FUNCTION_SUCCESS)
		goto error_send_i2c_reading_command;

	return FUNCTION_SUCCESS;

error_send_i2c_reading_command:
error_status_set_demod_registers:
	return FUNCTION_ERROR;
}
*/

/**

@see   I2C_BRIDGE_FP_FORWARD_I2C_WRITING_CMD

*/
/*
int
realtek_r_qam_ForwardI2cWritingCmd(
	I2C_BRIDGE_MODULE *pI2cBridge,
	unsigned char DeviceAddr,
	const unsigned char *pWritingBytes,
	U32BITS ByteNum
)
{
	QAM_DEMOD_MODULE *pDemod;
	BASE_INTERFACE_MODULE *pBaseInterface;

	// Get demod module and tuner device address.
	pDemod = (QAM_DEMOD_MODULE *)pI2cBridge->pPrivateData;

	// Get base interface.
	pBaseInterface = pDemod->pBaseInterface;

	// Enable demod I2C relay.
	if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_OPT_I2C_RELAY, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_demod_registers;

	// Send I2C writing command.
	if (pBaseInterface->I2cWrite(pBaseInterface, DeviceAddr, pWritingBytes, ByteNum) != FUNCTION_SUCCESS)
		goto error_send_i2c_writing_command;

	return FUNCTION_SUCCESS;

error_send_i2c_writing_command:
error_status_set_demod_registers:
	return FUNCTION_ERROR;
}
*/


/**

@brief   Initialize base register table

REALTEK_R QAM builder will use realtek_r_qam_InitBaseRegTable() to initialize base register table.


@param [in]   pDemod   The demod module pointer


@see   BuildRealtekRQamModule()

*/
/*
void
realtek_r_qam_InitBaseRegTable(
	QAM_DEMOD_MODULE *pDemod
)
{
	static const QAM_PRIMARY_BASE_REG_ENTRY_ADDR_16BIT PrimaryBaseRegTable[REALTEK_R_QAM_BASE_REG_TABLE_LEN] = {
		// Generality
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{QAM_SYS_VERSION,					0xe001,			7,		0	},
		{QAM_OPT_I2C_RELAY,					0xf001,			7,		7	},
		{QAM_SOFT_RESET,					0xe005,			0,		0	},
		{QAM_SOFT_RESET_FF,					0xe302,			2,		2	},

		// Miscellany
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{QAM_INNER_DATA_STROBE,				0xe169,			0,		0	},
		{QAM_INNER_DATA_SEL1,				0xe148,			7,		0	},
		{QAM_INNER_DATA_SEL2,				0xe149,			7,		0	},
		{QAM_INNER_DATA1,					0xe16a,			15,		0	},
		{QAM_INNER_DATA2,					0xe16c,			15,		0	},

		// QAM mode
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{QAM_QAM_MODE,						0xe102,			2,		0	},

		// AAGC
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{QAM_OPT_RF_AAGC_DRIVE,				0xe006,			1,		1	},
		{QAM_OPT_IF_AAGC_DRIVE,				0xe006,			5,		5	},
		{QAM_OPT_RF_AAGC_OE,				0xe006,			3,		3	},
		{QAM_OPT_IF_AAGC_OE,				0xe006,			7,		7	},
		{QAM_PAR_RF_SD_IB,					0xe006,			2,		2	},
		{QAM_PAR_IF_SD_IB,					0xe006,			6,		6	},
		{QAM_AAGC_FZ_OPTION,				0xe104,			5,		4	},
		{QAM_AAGC_TARGET,					0xe105,			7,		0	},
		{QAM_RF_AAGC_MAX,					0xe106,			7,		0	},
		{QAM_RF_AAGC_MIN,					0xe107,			7,		0	},
		{QAM_IF_AAGC_MAX,					0xe108,			7,		0	},
		{QAM_IF_AAGC_MIN,					0xe109,			7,		0	},
		{QAM_VTOP,							0xe10b,			7,		0	},
		{QAM_KRF,							0xe10c,			6,		3	},
		{QAM_AAGC_MODE_SEL,					0xe10c,			7,		7	},
		{QAM_AAGC_LD,						0xe172,			0,		0	},
		{QAM_AAGC_INIT_LEVEL,				0xe10a,			7,		0	},

		// DDC
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{QAM_DDC_FREQ,						0xe10d,			14,		0	},
		{QAM_SPEC_INV,						0xe10e,			7,		7	},

		// Timing recovery
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{QAM_TR_DECI_RATIO,					0xe11f,			23,		0	},

		// Carrier recovery
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{QAM_CR_LD,							0xe174,			5,		0	},

		// Equalizer
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{QAM_EQ_LD,							0xe172,			1,		1	},
		{QAM_FSM_STATE,						0xe172,			6,		3	},
		{QAM_MSE,							0xe176,			21,		0	},

		// Frame sync. indicator
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{QAM_SYNCLOST,						0xe202,			7,		7	},
		{QAM_FS_SYNC_STROBE,				0xe302,			7,		7	},
		{QAM_FS_SYNC_LOST,					0xe308,			1,		1	},
		{QAM_OC_MPEG_SYNC_MODE,				0xe31f,			0,		0	},

		// BER for OpenCable
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{QAM_OC_BER_RD_STROBE,				0xe312,			7,		7	},
		{QAM_OC_BERT_EN,					0xe313,			0,		0	},
		{QAM_OC_BERT_HOLD,					0xe313,			1,		1	},
		{QAM_OC_DIS_AUTO_MODE,				0xe313,			2,		2	},
		{QAM_OC_TEST_VOLUME,				0xe313,			5,		3	},
		{QAM_OC_BER_REG0,					0xe31b,			15,		0	},
		{QAM_OC_BER_REG1,					0xe314,			20,		0	},
		{QAM_OC_BER_REG2_15_0,				0xe317,			15,		0	},
		{QAM_OC_BER_REG2_18_16,				0xe316,			7,		5	},

		// BER for DVB-C
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{QAM_DVBC_BER_RD_STROBE,			0xe205,			7,		7	},
		{QAM_DVBC_BERT_EN,					0xe206,			0,		0	},
		{QAM_DVBC_BERT_HOLD,				0xe206,			1,		1	},
		{QAM_DVBC_DIS_AUTO_MODE,			0xe206,			2,		2	},
		{QAM_DVBC_TEST_VOLUME,				0xe206,			5,		3	},
		{QAM_DVBC_BER_REG0,					0xe20e,			15,		0	},
		{QAM_DVBC_BER_REG1,					0xe207,			20,		0	},
		{QAM_DVBC_BER_REG2_15_0,			0xe20a,			15,		0	},
		{QAM_DVBC_BER_REG2_18_16,			0xe209,			7,		5	},

		// MPEG TS output interface
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{QAM_OPT_MPEG_OUT_SEL,				0xe026,			1,		0	},

		// MPEG TS output interface for OpenCable
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{QAM_OC_CKOUTPAR,					0xe31e,			1,		1	},
		{QAM_OC_CKOUT_PWR,					0xe31e,			2,		2	},
		{QAM_OC_CDIV_PH0,					0xe31d,			3,		0	},
		{QAM_OC_CDIV_PH1,					0xe31d,			7,		4	},
		{QAM_OC_SERIAL,						0xe31e,			0,		0	},

		// MPEG TS output interface for DVB-C
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{QAM_DVBC_CKOUTPAR,					0xe211,			0,		0	},
		{QAM_DVBC_CKOUT_PWR,				0xe211,			1,		1	},
		{QAM_DVBC_CDIV_PH0,					0xe212,			3,		0	},
		{QAM_DVBC_CDIV_PH1,					0xe212,			7,		4	},
		{QAM_DVBC_NO_REINVERT,				0xe210,			2,		2	},
		{QAM_DVBC_FIX_TEI,					0xe210,			3,		3	},
		{QAM_DVBC_SERIAL,					0xe211,			2,		2	},

		// Monitor
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{QAM_ADC_CLIP_CNT_REC,				0xe16a,			15,		4	},
		{QAM_DAGC_LEVEL_26_11,				0xe16a,			15,		0	},
		{QAM_DAGC_LEVEL_10_0,				0xe16c,			15,		5	},
		{QAM_RF_AAGC_SD_IN,					0xe16a,			15,		5	},
		{QAM_IF_AAGC_SD_IN,					0xe16c,			15,		5	},
		{QAM_KI_TR_OUT_30_15,				0xe16a,			15,		0	},
		{QAM_KI_TR_OUT_14_0,				0xe16c,			15,		1	},
		{QAM_KI_CR_OUT_15_0,				0xe16a,			15,		0	},
		{QAM_KI_CR_OUT_31_16,				0xe16c,			15,		0	},

		// Specific register
		// RegBitName,						RegStartAddr,	Msb,	Lsb
		{QAM_SPEC_SIGNAL_INDICATOR,			0xe173,			5,		3	},
		{QAM_SPEC_ALPHA_STROBE,			    0xe157,			0,		0	},
		{QAM_SPEC_ALPHA_SEL,				0xe157,			4,		1	},
		{QAM_SPEC_ALPHA_VAL,				0xe157,			14,		5	},
		{QAM_SPEC_SYMBOL_RATE_REG_0,		0xe10f,			2,		0	},
		{QAM_SPEC_SYMBOL_RATE_STROBE,		0xe15b,			0,		0	},
		{QAM_SPEC_SYMBOL_RATE_SEL,			0xe15b,			4,		1	},
		{QAM_SPEC_SYMBOL_RATE_VAL,			0xe15b,			14,		5	},
		{QAM_SPEC_REG_0_STROBE,			    0xe15d,			0,		0	},
		{QAM_SPEC_REG_0_SEL,				0xe15d,			4,		1	},
		{QAM_SPEC_MODE,					    0xe10f,			5,		5	},
	};

	int i;
	int RegBitName;

	// Initialize base register table according to primary base register table.
	// Note: 1. Base register table rows are sorted by register bit name key.
	//       2. The default value of the IsAvailable variable is "NO".
	for (i = 0; i < QAM_BASE_REG_TABLE_LEN_MAX; i++)
		pDemod->BaseRegTable.Addr16Bit[i].IsAvailable  = NO;

	for (i = 0; i < REALTEK_R_QAM_BASE_REG_TABLE_LEN; i++) {
		RegBitName = PrimaryBaseRegTable[i].RegBitName;

		pDemod->BaseRegTable.Addr16Bit[RegBitName].IsAvailable  = YES;
		pDemod->BaseRegTable.Addr16Bit[RegBitName].RegStartAddr = PrimaryBaseRegTable[i].RegStartAddr;
		pDemod->BaseRegTable.Addr16Bit[RegBitName].Msb          = PrimaryBaseRegTable[i].Msb;
		pDemod->BaseRegTable.Addr16Bit[RegBitName].Lsb          = PrimaryBaseRegTable[i].Lsb;
	}

	return;
}
*/


/**

@brief   Initialize monitor register table

REALTEK_R QAM builder will use realtek_r_qam_InitMonitorRegTable() to initialize monitor register table.


@param [in]   pDemod   The demod module pointer


@see   BuildRealtekRQamModule()

*/
/*
void
realtek_r_qam_InitMonitorRegTable(
	QAM_DEMOD_MODULE *pDemod
)
{
	static const QAM_PRIMARY_MONITOR_REG_ENTRY_ADDR_16BIT PrimaryMonitorRegTable[REALTEK_R_QAM_MONITOR_REG_TABLE_LEN] = {
		// Generality
		// MonitorRegBitName,		InfoNum,		{SelRegAddr,	SelValue,	RegBitName,				Shift	}
		{
			QAM_ADC_CLIP_CNT,			1,			{	{0xe148,		0x1,		QAM_ADC_CLIP_CNT_REC,	0		},
				{NO_USE,		NO_USE,		NO_USE,					NO_USE	},
			}
		},

		{
			QAM_DAGC_VALUE,			1,			{	{0xe148,		0x20,		QAM_DAGC_LEVEL_26_11,	0		},
				{NO_USE,		NO_USE,		NO_USE,					NO_USE	},
			}
		},

		{
			QAM_RF_AGC_VALUE,			1,			{	{0xe148,		0x80,		QAM_RF_AAGC_SD_IN,		0		},
				{NO_USE,		NO_USE,		NO_USE,					NO_USE	},
			}
		},

		{
			QAM_IF_AGC_VALUE,			1,			{	{0xe149,		0x80,		QAM_IF_AAGC_SD_IN,		0		},
				{NO_USE,		NO_USE,		NO_USE,					NO_USE	},
			}
		},

		{
			QAM_TR_OFFSET,				2,			{	{0xe148,		0xc2,		QAM_KI_TR_OUT_30_15,	15		},
				{0xe149,		0xc2,		QAM_KI_TR_OUT_14_0,		0		},
			}
		},

		{
			QAM_CR_OFFSET,				2,			{	{0xe148,		0xc3,		QAM_KI_CR_OUT_15_0,		0		},
				{0xe149,		0xc3,		QAM_KI_CR_OUT_31_16,	16		},
			}
		},
	};

	int i, j;
	int MonitorRegBitName;

	// Initialize monitor register table according to primary monitor register table.
	// Note: 1. Monitor register table rows are sorted by monitor register name key.
	//       2. The default value of the IsAvailable variable is "NO".
	for (i = 0; i < QAM_MONITOR_REG_TABLE_LEN_MAX; i++)
		pDemod->MonitorRegTable.Addr16Bit[i].IsAvailable  = NO;

	for (i = 0; i < REALTEK_R_QAM_MONITOR_REG_TABLE_LEN; i++) {
		MonitorRegBitName = PrimaryMonitorRegTable[i].MonitorRegBitName;

		pDemod->MonitorRegTable.Addr16Bit[MonitorRegBitName].IsAvailable = YES;
		pDemod->MonitorRegTable.Addr16Bit[MonitorRegBitName].InfoNum = PrimaryMonitorRegTable[i].InfoNum;

		for (j = 0; j < QAM_MONITOR_REG_INFO_TABLE_LEN; j++) {
			pDemod->MonitorRegTable.Addr16Bit[MonitorRegBitName].InfoTable[j].SelRegAddr =
				PrimaryMonitorRegTable[i].InfoTable[j].SelRegAddr;
			pDemod->MonitorRegTable.Addr16Bit[MonitorRegBitName].InfoTable[j].SelValue =
				PrimaryMonitorRegTable[i].InfoTable[j].SelValue;
			pDemod->MonitorRegTable.Addr16Bit[MonitorRegBitName].InfoTable[j].RegBitName =
				PrimaryMonitorRegTable[i].InfoTable[j].RegBitName;
			pDemod->MonitorRegTable.Addr16Bit[MonitorRegBitName].InfoTable[j].Shift =
				PrimaryMonitorRegTable[i].InfoTable[j].Shift;
		}
	}

	return;
}
*/


/**

@brief   Set REALTEK_R QAM demod register mask bits with acceleration.

REALTEK_R QAM upper level functions will use realtek_r_qam_acceleration_SetRegMaskBits() to set demod register mask bits.


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
/*
int
realtek_r_qam_acceleration_SetRegMaskBits(
	QAM_DEMOD_MODULE *pDemod,
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
		if (qam_demod_addr_16bit_default_SetRegBytes(pDemod, RegStartAddr, WritingBytes, ByteNum) != FUNCTION_SUCCESS)
			goto error_status_set_demod_registers;
	} else {
		// Use mask bit setting method to set register mask bits.
		if (qam_demod_addr_16bit_default_SetRegMaskBits(pDemod, RegStartAddr, Msb, Lsb, WritingValue) !=FUNCTION_SUCCESS)
			goto error_status_set_demod_registers;
	}

	return FUNCTION_SUCCESS;

error_status_set_demod_registers:
	return FUNCTION_ERROR;
}
*/


/**

@brief   Get inner strobe register bits.

REALTEK_R QAM upper level functions will use realtek_r_qam_GetInnerStrobeRegBits() to get register bits with inner strobe.


@param [in]    pDemod          The demod module pointer
@param [in]    RegBitName      Pre-defined demod register bit name
@param [out]   pReadingValue   Pointer to an allocated memory for storing reading value


@retval   FUNCTION_SUCCESS   Get demod register bits successfully with bit name and inner strobe.
@retval   FUNCTION_ERROR     Get demod register bits unsuccessfully.


@note
	-# Don't need to set register page before using realtek_r_qam_GetInnerStrobeRegBits().

*/
int
realtek_r_qam_GetInnerStrobeRegBits(
	QAM_DEMOD_MODULE *pDemod,
	int RegBitName,
	U32BITS *pReadingValue
)
{
	U32BITS ReadData_1 = 0, ReadData_2 = 0, ReadData_3 = 0;
	// Set inner data strobe.
	// Note: REALTEK_R QAM hardware will clear strobe automatically.
	//if (pDemod->RegAccess.Addr16Bit.SetRegBits(pDemod, QAM_INNER_DATA_STROBE, ON) != FUNCTION_SUCCESS)
	rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));

	// Get the inner strobe register bits.
	switch (RegBitName) {
	case QAM_AAGC_LD :
		*pReadingValue = rtd_inl(DVBC_INRX_MO2_reg) & 0x01;
		break;

	case QAM_EQ_LD :
		*pReadingValue = rtd_inl(DVBC_INRX_MO2_reg) & 0x02;
		break;

	case QAM_MSE :
		ReadData_1 = DVBC_INRX_MO3_get_inrx_mo3_7_0(rtd_inl(DVBC_INRX_MO3_reg));
		ReadData_2 = DVBC_INRX_MO3_1_get_inrx_mo3_15_8(rtd_inl(DVBC_INRX_MO3_1_reg));
		ReadData_3 = rtd_inl(DVBC_INRX_MO3_2_reg) & 0x3F;
		*pReadingValue = ReadData_1 + (ReadData_2 << 8) + (ReadData_3 << 16);
		break;
	default :
		REALTEK_R_QAM_BASE_WARNING("Unknown RegBitName\n");
		return FUNCTION_ERROR;
	}

	//if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, RegBitName, pReadingValue) != FUNCTION_SUCCESS)
	//	goto error_status_get_demod_registers;

	return FUNCTION_SUCCESS;
}


/**

@brief   Get monitor register bits.

REALTEK_R QAM upper level functions will use realtek_r_qam_GetMonitorRegBits() to get monitor register bits.


@param [in]    pDemod              The demod module pointer
@param [in]    MonitorRegBitName   Pre-defined demod monitor register bit name
@param [out]   pReadingValue       Pointer to an allocated memory for storing reading value


@retval   FUNCTION_SUCCESS   Get demod monitor register bits successfully with monitor bit name.
@retval   FUNCTION_ERROR     Get demod monitor register bits unsuccessfully.


@note
	-# Don't need to set register page before using realtek_r_qam_GetMonitorRegBits().

*/
int
realtek_r_qam_GetMonitorRegBits(
	QAM_DEMOD_MODULE *pDemod,
	int MonitorRegBitName,
	U32BITS *pReadingValue
)
{
	unsigned int buffer_1 = 0, buffer_2 = 0, buffer_3 = 0, buffer_4 = 0;
	*pReadingValue = 0;

	switch (MonitorRegBitName) {
	case QAM_ADC_CLIP_CNT:
		rtd_maskl(DVBC_OPT_DB_SIGNAL_SEL1_reg, ~DVBC_OPT_DB_SIGNAL_SEL1_opt_db_signal_sel1_mask, DVBC_OPT_DB_SIGNAL_SEL1_opt_db_signal_sel1(0x1));
		rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));
		buffer_1 = DVBC_TST_OUT1_get_tst_out1_7_0(rtd_inl(DVBC_TST_OUT1_reg));
		buffer_2 = DVBC_TST_OUT1_1_get_tst_out1_15_8(rtd_inl(DVBC_TST_OUT1_1_reg));
		*pReadingValue = ((buffer_2 << 8) | (buffer_1)) >> 4;
		break;

	case QAM_DAGC_VALUE:
		rtd_maskl(DVBC_OPT_DB_SIGNAL_SEL1_reg, ~DVBC_OPT_DB_SIGNAL_SEL1_opt_db_signal_sel1_mask, DVBC_OPT_DB_SIGNAL_SEL1_opt_db_signal_sel1(0x20));
		rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));
		buffer_1 = DVBC_TST_OUT1_get_tst_out1_7_0(rtd_inl(DVBC_TST_OUT1_reg));
		buffer_2 = DVBC_TST_OUT1_1_get_tst_out1_15_8(rtd_inl(DVBC_TST_OUT1_1_reg));
		*pReadingValue = ((buffer_2 << 8) | (buffer_1));
		break;

	case QAM_RF_AGC_VALUE:
		rtd_maskl(DVBC_OPT_DB_SIGNAL_SEL1_reg, ~DVBC_OPT_DB_SIGNAL_SEL1_opt_db_signal_sel1_mask, DVBC_OPT_DB_SIGNAL_SEL1_opt_db_signal_sel1(0x80));
		rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));
		buffer_1 = DVBC_TST_OUT1_get_tst_out1_7_0(rtd_inl(DVBC_TST_OUT1_reg));
		buffer_2 = DVBC_TST_OUT1_1_get_tst_out1_15_8(rtd_inl(DVBC_TST_OUT1_1_reg));
		*pReadingValue = ((buffer_2 << 8) | (buffer_1)) >> 5;
		break;

	case QAM_IF_AGC_VALUE:
		rtd_maskl(DVBC_OPT_DB_SIGNAL_SEL2_reg, ~DVBC_OPT_DB_SIGNAL_SEL2_opt_db_signal_sel2_mask , DVBC_OPT_DB_SIGNAL_SEL2_opt_db_signal_sel2(0x80));
		rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));
		buffer_1 = DVBC_TST_OUT2_get_tst_out2_7_0(rtd_inl(DVBC_TST_OUT2_reg));
		buffer_2 = DVBC_TST_OUT2_1_get_tst_out2_15_8(rtd_inl(DVBC_TST_OUT2_1_reg));
		*pReadingValue = ((buffer_2 << 8) | (buffer_1)) >> 5;
		break;

	case QAM_TR_OFFSET:
		rtd_maskl(DVBC_OPT_DB_SIGNAL_SEL1_reg, ~DVBC_OPT_DB_SIGNAL_SEL1_opt_db_signal_sel1_mask , DVBC_OPT_DB_SIGNAL_SEL1_opt_db_signal_sel1(0xc2));
		rtd_maskl(DVBC_OPT_DB_SIGNAL_SEL2_reg, ~DVBC_OPT_DB_SIGNAL_SEL2_opt_db_signal_sel2_mask , DVBC_OPT_DB_SIGNAL_SEL2_opt_db_signal_sel2(0xc2));
		rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));
		buffer_3 = DVBC_TST_OUT1_get_tst_out1_7_0(rtd_inl(DVBC_TST_OUT1_reg));
		buffer_4 = DVBC_TST_OUT1_1_get_tst_out1_15_8(rtd_inl(DVBC_TST_OUT1_1_reg));
		buffer_1 = DVBC_TST_OUT2_get_tst_out2_7_0(rtd_inl(DVBC_TST_OUT2_reg));
		buffer_2 = DVBC_TST_OUT2_1_get_tst_out2_15_8(rtd_inl(DVBC_TST_OUT2_1_reg));
		*pReadingValue = ((buffer_4 << 24) | (buffer_3 << 16) | (buffer_2 << 8) | (buffer_1)) >> 1;
		break;

	case QAM_CR_OFFSET:
		rtd_maskl(DVBC_OPT_DB_SIGNAL_SEL1_reg, ~DVBC_OPT_DB_SIGNAL_SEL1_opt_db_signal_sel1_mask , DVBC_OPT_DB_SIGNAL_SEL1_opt_db_signal_sel1(0xc3));
		rtd_maskl(DVBC_OPT_DB_SIGNAL_SEL2_reg, ~DVBC_OPT_DB_SIGNAL_SEL2_opt_db_signal_sel2_mask , DVBC_OPT_DB_SIGNAL_SEL2_opt_db_signal_sel2(0xc3));
		rtd_maskl(DVBC_TST_OUT_STB_reg, ~DVBC_TST_OUT_STB_tst_out_stb_mask, DVBC_TST_OUT_STB_tst_out_stb(0x1));
		buffer_1 = DVBC_TST_OUT1_get_tst_out1_7_0(rtd_inl(DVBC_TST_OUT1_reg));
		buffer_2 = DVBC_TST_OUT1_1_get_tst_out1_15_8(rtd_inl(DVBC_TST_OUT1_1_reg));
		buffer_3 = DVBC_TST_OUT2_get_tst_out2_7_0(rtd_inl(DVBC_TST_OUT2_reg));
		buffer_4 = DVBC_TST_OUT2_1_get_tst_out2_15_8(rtd_inl(DVBC_TST_OUT2_1_reg));
		*pReadingValue = (buffer_4 << 24) | (buffer_3 << 16) | (buffer_2 << 8) | (buffer_1);
		break;

	default :
		REALTEK_R_QAM_BASE_WARNING("Unknown MonitorRegBitName\n");
		return FUNCTION_ERROR;
	}

	return FUNCTION_SUCCESS;

}


/**

@brief   Set I2C bridge module demod arguments.

REALTEK_R QAM builder will use realtek_r_qam_BuildI2cBridgeModule() to set I2C bridge module demod arguments.


@param [in]   pDemod   The demod module pointer


@see   BuildRealtekRQamModule()

*/
/*
void
realtek_r_qam_BuildI2cBridgeModule(
	QAM_DEMOD_MODULE *pDemod
)
{
	I2C_BRIDGE_MODULE *pI2cBridge;

	// Get I2C bridge module.
	pI2cBridge = pDemod->pI2cBridge;

	// Set I2C bridge module demod arguments.
	pI2cBridge->pPrivateData = (void *)pDemod;
	pI2cBridge->ForwardI2cReadingCmd = realtek_r_qam_ForwardI2cReadingCmd;
	pI2cBridge->ForwardI2cWritingCmd = realtek_r_qam_ForwardI2cWritingCmd;

	return;
}
*/


