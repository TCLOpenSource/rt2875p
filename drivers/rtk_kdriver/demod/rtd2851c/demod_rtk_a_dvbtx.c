/***************************************************************************************************
  File        : demod_rtk_dvbt2.cpp
  Description : REALTEK_I DVB-T/T2 demod
  Author      : Kevin Wang
****************************************************************************************************
    Update List :
----------------------------------------------------------------------------------------------------
    20120207    | create phase
***************************************************************************************************/
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <rtk_kdriver/io.h>
#include "foundation.h"
#include "demod_rtk_a_dvbtx.h"
#include "dvbtx_demod_rtk_a.h"
#include "demod_rtk_a_dvbtx_priv.h"
#include "rbus/dtv_demod_misc_reg.h"
#include "rbus/atb_demod_ce_ro_reg.h"

//#include "rbus/demod_sd_reg.h"
//#include "rbus/dtv_frontend_reg.h"

//#define SUPPORT_DEMOD_AUTOTEST


extern unsigned char DvbTxRomArray[];
extern unsigned long DvbTxRomArraySize;
static unsigned long *pDvbtxShareMemory = NULL;
extern unsigned long *pgShareMemory;
static unsigned long gShareMemoryPhysicalAddress = 0;
#if IS_ENABLED(CONFIG_RTK_KDRV_PCMCIA)
extern int rtk_pcmcia_get_card_status_ext(void);
#endif
extern struct device *gDemodDev;

//------------------------------------------------------------------------------------------
//DVBT2 SSI
#define   LOWER_SSI_LEVEL                   -15  // Lower range of Power level in dBm
#define   MED1_SSI_LEVEL                    0    // First threshold for power level
#define   MED2_SSI_LEVEL                    20   // Second threshold for power level
#define   UPPER_SSI_LEVEL                   35   // Upper level for power threshold
#define   CEIL_SSI_LEVEL                    100  // Maximum level of SSI which is an indication for high input power
#define   DVBT2_NUM_PLP_CONSTELLATIONS      4
#define   DVBT2_NUM_DATA_PLP_CODE_RATES     8
#define   DVBT2_SYNDERR_RETRY_MAX           100

/* Array to hold the Pref level as in Nordig 2.3 pp 43 */
/*
static signed char prefLevel[DVBT2_NUM_PLP_CONSTELLATIONS][DVBT2_NUM_DATA_PLP_CODE_RATES] = {
	// Rate:  1/2    3/5     2/3     3/4     4/5     5/6    1/3     2/5
	{ -96,   -95,    -94,    -93,    -92,    -92,   -98,    -97},  // QPSK
	{ -91,   -89,    -88,    -87,    -86,    -86,   -94,    -93},  // 16-QAM
	{ -86,   -85,    -83,    -82,    -81,    -80,   -90,    -88},  // 64-QAM
	{ -82,   -80,    -78,    -76,    -75,    -74,   -87,    -85}   // 256-QAM
};

*/
//------------------------------------------------------------------------------------------
//DVBT SSI
#define DVBT_SSI_NEG100p5_Q8    (-25728)
#define DVBT_SSI_NEG15_Q8       (-3840)
#define DVBT_SSI_10_Q8          (2560)
#define DVBT_SSI_15_Q8          (3840)
#define DVBT_SSI_20_Q8          (5120)
#define DVBT_SSI_35_Q8          (8960)
#define DVBT_SSI_90_Q8          (23040)
#define DVBT_SSI_100_Q8         (25600)
#define DVBT_SSI_ZERO_Q8        (0)
#define PHY_ONE_Q8              (1<<8)

/* Lookup tables for SSI/SQI */
static const signed char PHY_Pref[3][5] = {
	/* 1R2  2R3  3R4  5R6  7R8 */
	{  -93, -91, -90, -89, -88 },   /* QPSK  */
	{  -87, -85, -84, -83, -82 },   /* 16QAM */
	{  -82, -80, -78, -77, -76 }    /* 64QAM */
};

static unsigned long acquire_sig_stime;

#define PRE_SQI_MAX_QUANTITY 5
static unsigned char PreSqi[PRE_SQI_MAX_QUANTITY] = {0};
static unsigned char PreSqiNum = 0;
static unsigned int SqiContinuousZeroNum = 0;

#define TSCLK_DEFAULT       9200

static unsigned int gDvbT2DataPlpNumber = 0;


/*----------------------------------------------------------------------
 * Func : REALTEK_A_DVBTX
 *
 * Desc : constructor
 *
 * Parm : device_addr : device address
 *        output_mode : RTK_DEMOD_OUT_IF_SERIAL / RTK_DEMOD_OUT_IF_PARALLEL
 *        output_freq : RTK_DEMOD_OUT_FREQ_6M / RTK_DEMOD_OUT_FREQ_10M
 *        pComm       : hadle of COMM
 *
 * Retn : handle of demod
 *----------------------------------------------------------------------*/
void REALTEK_A_DVBTX_Constructors(DEMOD* pDemodKernel, unsigned char addr, unsigned char output_mode, unsigned char output_freq, COMM* pComm)
{
	REALTEK_A_DVBTX* pRealtekADVBTX;

	pDemodKernel->private_data = kmalloc(sizeof(REALTEK_A_DVBTX), GFP_KERNEL);
	if (pDemodKernel->private_data == NULL) {
		goto kmalloc_fail;
	}

	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);

	Demod_Constructors(pDemodKernel);

	pDemodKernel->m_id                 = DTV_DEMOD_REALTEK_A_DVBTX;
	pDemodKernel->m_addr               = addr;
	pDemodKernel->m_pTuner             = NULL;
	pDemodKernel->m_pComm              = pComm;
	pDemodKernel->m_ScanRange          = SCAN_RANGE_3_6;
	pDemodKernel->m_Capability         = TV_SYS_TYPE_DVBT2 | TV_SYS_TYPE_DVBT;
	pDemodKernel->m_update_interval_ms = 300;
	pDemodKernel->m_if.freq            = 5000000;
	pDemodKernel->m_if.inversion       = 0;
	pDemodKernel->m_if.agc_pola        = 0;
	pDemodKernel->m_clock              = CRYSTAL_FREQ_24000000HZ;
	pRealtekADVBTX->m_output_freq        = output_freq;

	pRealtekADVBTX->m_private            = (void*) AllocRealtekADvbtxDriver(pDemodKernel->m_pComm, pDemodKernel->m_addr, pDemodKernel->m_clock);

	pDemodKernel->m_ts.mode            = (output_mode == RTK_DEMOD_OUT_IF_PARALLEL) ? PARALLEL_MODE : SERIAL_MODE;
	pDemodKernel->m_ts.data_order      = MSB_D7;
	pDemodKernel->m_ts.datapin         = MSB_FIRST;
	pDemodKernel->m_ts.err_pol         = ACTIVE_HIGH;
	pDemodKernel->m_ts.sync_pol        = ACTIVE_HIGH;
	pDemodKernel->m_ts.val_pol         = ACTIVE_HIGH;
	pDemodKernel->m_ts.clk_pol         = FALLING_EDGE;
	pDemodKernel->m_ts.internal_demod_input = 1;
	pDemodKernel->m_ts.tsOutEn = 1;
	pDemodKernel->m_ScanMode           = CH_UNKNOW;
	pRealtekADVBTX->m_TunerOptRegCnt = 0;
	pRealtekADVBTX->m_acquire_sig_en = 0;
	pRealtekADVBTX->m_pTunerOptReg = NULL;

	//pRealtekADVBTX->m_dvbt2_AgcScale = 48;
	//pRealtekADVBTX->m_dvbt2_ext_gain_dis = RTK_DEMOD_XML_EXT_GAIN_EN;
	//pRealtekADVBTX->m_dvbt_AgcScale = 64;
	//pRealtekADVBTX->m_dvbt_ext_gain_dis = RTK_DEMOD_XML_EXT_GAIN_EN;
	pRealtekADVBTX->m_unlock_cnt = 0;
	pRealtekADVBTX->m_auto_tune_en = 0;
	pRealtekADVBTX->m_dvbt2_sfo = 0;
	pRealtekADVBTX->m_ts_clk_KHz = 0;

	pDemodKernel->m_pComm->SetTargetName(pDemodKernel->m_pComm, pDemodKernel->m_addr, "REALTEK_A_DVBTX");

	pDemodKernel->Init = REALTEK_A_DVBTX_Init;
	pDemodKernel->Reset = Demod_Reset;
	pDemodKernel->AcquireSignal = REALTEK_A_DVBTX_AcquireSignal;
	pDemodKernel->ScanSignal = REALTEK_A_DVBTX_ScanSignal;
	pDemodKernel->SetIF = REALTEK_A_DVBTX_SetIF;
	pDemodKernel->SetTSMode = REALTEK_A_DVBTX_SetTSMode;
	pDemodKernel->SetTVSysEx = REALTEK_A_DVBTX_SetTVSysEx;
	pDemodKernel->SetMode = REALTEK_A_DVBTX_SetMode;
	pDemodKernel->SetTVSys = REALTEK_A_DVBTX_SetTVSys;
	pDemodKernel->GetLockStatus = REALTEK_A_DVBTX_GetLockStatus;
	pDemodKernel->GetSegmentNum = Demod_GetSegmentNum;
	pDemodKernel->GetSignalInfo = REALTEK_A_DVBTX_GetSignalInfo;
	pDemodKernel->GetSignalQuality = REALTEK_A_DVBTX_GetSignalQuality;
	pDemodKernel->GetT2PLPInfo = REALTEK_A_DVBTX_GetT2PLPInfo;
	pDemodKernel->ChangeT2PLP = REALTEK_A_DVBTX_ChangeT2PLP;
	pDemodKernel->GetT2L1PreInfo = Demod_GetT2L1PreInfo;
	pDemodKernel->GetT2L1PostInfo = Demod_GetT2L1PostInfo;
	pDemodKernel->GetDemodStatus = Demod_GetDemodStatus;
	pDemodKernel->GetCarrierOffset = REALTEK_A_DVBTX_GetCarrierOffset;
	pDemodKernel->GetDebugLogBuf = REALTEK_A_DVBTX_GetDebugLogBuf;
	pDemodKernel->ForwardI2C = REALTEK_A_DVBTX_ForwardI2C;
	pDemodKernel->AutoTune = REALTEK_A_DVBTX_AutoTune;
	pDemodKernel->KernelAutoTune = REALTEK_A_DVBTX_KernelAutoTune;
	pDemodKernel->GetCapability = Demod_GetCapability;
	pDemodKernel->Suspend = Demod_Suspend;
	pDemodKernel->Resume = Demod_Resume;
	pDemodKernel->SetTvMode = REALTEK_A_DVBTX_SetTvMode;
	pDemodKernel->InspectSignal = REALTEK_A_DVBTX_InspectSignal;    // Add monitor function that used to monitor demod status
	pDemodKernel->AttachTuner = Demod_AttachTuner;
	pDemodKernel->GetTSParam = Demod_GetTSParam;
	pDemodKernel->GetTsClkRate = REALTEK_A_DVBTX_GetTsClkRate;
	//pDemodKernel->Lock = Demod_Lock;
	//pDemodKernel->Unlock = Demod_Unlock;
	pDemodKernel->Lock = REALTEK_A_DVBTX_Lock;
	pDemodKernel->Unlock = REALTEK_A_DVBTX_Unlock;

	//pDemodKernel->Activate = REALTEK_A_DVBTX_Activate;
	pDemodKernel->Destory = REALTEK_A_DVBTX_Destructors;

kmalloc_fail:
	REALTEK_A_DVBTX_WARNING("REALTEK_A_DVBTX_Constructors kmalloc Failed\n");
}


/*----------------------------------------------------------------------
 * Func : ~REALTEK_A_DVBTX
 *
 * Desc : Destructor of REALTEK_A_DVBTX
 *
 * Parm : pDemod : handle of Demod
 *
 * Retn : N/A
 *----------------------------------------------------------------------*/
void REALTEK_A_DVBTX_Destructors(DEMOD* pDemodKernel)
{
	REALTEK_A_DVBTX* pRealtekADVBTX;
	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);

	if (pRealtekADVBTX->m_private != NULL)
		ReleaseRealtekADvbtxDriver((REALTEK_A_DVBTX_DRIVER_DATA*)pRealtekADVBTX->m_private);

	kfree(pRealtekADVBTX);
	//pDemodKernel->Destory(pDemodKernel);
	REALTEK_A_DVBTX_INFO("REALTEK_A_DVBTX_Destructors Complete\n");
}


/*----------------------------------------------------------------------
 * Func : Init
 *
 * Desc : Initial realtekI
 *
 * Parm : N/A
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_Init(DEMOD* pDemodKernel)
{
	int ret = TUNER_CTRL_OK;
	int i;
	//unsigned int DemodVirAddrStart = 0, DvbtxRomAddrStart = 0;
	unsigned int AllocateSize = 0;
	unsigned long ShareMemoryPhysicalAddress = 0, DvbtxRomPhysicalAddrStart, DvbtxRomVirAddrStart, rst;
	//unsigned char* recv_buf;

	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;

	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);

	//REALTEK_A_DVBTX_INFO("\033[1;32;31m" "%s %s %d\n" "\033[m", __FILE__, __func__, __LINE__);

	if (pDemodKernel->m_pComm == NULL || pRealtekADVBTX->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*) pRealtekADVBTX->m_private)->pDemod;

	//////////////////////////////////////////////////
	//DVBTX_DEMOD_MODULE* pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*) pRealtekADVBTX->m_private)->pDemod;

	//////////////////////////////////////////////////
	pDemodKernel->Lock(pDemodKernel);
	pRealtekADVBTX->m_auto_tune_en = 0;
	pRealtekADVBTX->m_unlock_cnt = 0;
	pRealtekADVBTX->m_ts_clk_KHz = 0;
	PreSqiNum = 0;
	SqiContinuousZeroNum = 0;


	TvFrontendPowerControl(0);                    //Turn off TVALL
	if (pDvbtxShareMemory == NULL) {
		ret = DtvDemodMemRemappingInt((RTK_A_DVBT2_DDR_SIZE + DvbTxRomArraySize), &ShareMemoryPhysicalAddress, &AllocateSize);
		if (ret != TUNER_CTRL_OK) {
			REALTEK_A_DVBTX_WARNING("allocate share memory failed\n");
			goto init_fail;
		}
		pDvbtxShareMemory = pgShareMemory;
		gShareMemoryPhysicalAddress = ShareMemoryPhysicalAddress;
		DvbtxRomPhysicalAddrStart = ShareMemoryPhysicalAddress + RTK_A_DVBT2_DDR_SIZE;


		DvbtxRomVirAddrStart = (unsigned long)phys_to_virt(DvbtxRomPhysicalAddrStart);

		REALTEK_A_DVBTX_INFO("allocate share memory(dvbtx) - Phy_addr=0x%08lx, Vir_addr=0x%08lx,size=0x%08x\n", ShareMemoryPhysicalAddress, DvbtxRomVirAddrStart, AllocateSize);


		//CopyROM data into DRAM
		memcpy((void *)DvbtxRomVirAddrStart, DvbTxRomArray, DvbTxRomArraySize);
		if (gDemodDev != NULL) {
			rst = (unsigned long) dma_map_single(gDemodDev, (void *)DvbtxRomVirAddrStart, DvbTxRomArraySize, DMA_BIDIRECTIONAL);
			REALTEK_A_DVBTX_INFO("Load Dvbtx ROM data to DDR  Rst=0x%08lx\n", rst);
		} else {
			REALTEK_A_DVBTX_WARNING("CopyROM data into DRAM fail!!!\n");
			goto init_fail;
		}
	}

	DtvDemodInitialization(RTK_DEMOD_MODE_DVBT2, 1);

	//DTV demod MISC setting
	rtd_maskl(DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_3_reg, ~DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_3_atb_ldpc_rom_dma_cfg_addr_mask, DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_3_atb_ldpc_rom_dma_cfg_addr(0x70));
	rtd_outl(DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_8_reg, gShareMemoryPhysicalAddress + RTK_A_DVBT2_DDR_SIZE);
	rtd_maskl(DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_2_reg, ~DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_2_atb_ldpc_rom_dma_cfg_wr_mask, DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_2_atb_ldpc_rom_dma_cfg_wr(1));
	rtd_maskl(DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_DBG_2_reg, ~DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_DBG_2_atb_ldpc_rom_dma_dbg_data_sel_mask, DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_DBG_2_atb_ldpc_rom_dma_dbg_data_sel(0x210));
	rtd_outl(DTV_DEMOD_MISC_atb_wrapper_addr_offset_reg, gShareMemoryPhysicalAddress);

	rtd_maskl(DTV_DEMOD_MISC_atb_wrapper_cmd_limit_reg, ~(DTV_DEMOD_MISC_atb_wrapper_cmd_limit_atb_wrapper_reset_flow_en_mask | DTV_DEMOD_MISC_atb_wrapper_cmd_limit_atb_wrapper_rcmd_limit_atb_mask), (DTV_DEMOD_MISC_atb_wrapper_cmd_limit_atb_wrapper_reset_flow_en(0x1) | DTV_DEMOD_MISC_atb_wrapper_cmd_limit_atb_wrapper_rcmd_limit_atb(0x16)));
	rtd_outl(DTV_DEMOD_MISC_DTV_DEMOD_SEL_reg, 0x00000001);			//DTV Dvbt enable<bit0> = 0x1(1)


	rtd_maskl(DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_1_reg, ~DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_1_atb_ldpc_rom_dma_sw_reset_option_mask, DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_1_atb_ldpc_rom_dma_sw_reset_option(0x1));//enable LDPC ROM DMA (sw_reset option) for mac8p CDC issue.

	pRealtekADVBTX->m_dvbt2_dvbt_mode = RTK_A_DVBTX_DVBT2_MODE;

	pDemodKernel->Unlock(pDemodKernel);


	if (pDemodKernel->m_pTuner) {

		TUNER_PARAM Param;
		if (pDemodKernel->m_pTuner->GetParam(pDemodKernel->m_pTuner, TV_SYS_DVBT2_8M, &Param) == TUNER_CTRL_OK) {
			switch (Param.id) {
			case TV_TUNER_NXP_TDA18275A:
				REALTEK_A_DVBTX_INFO("Tuner id %u, TV_TUNER_NXP_TDA1827X\n", Param.id);
				pRealtekADVBTX->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) TDA18275A_OPT_VALUE_DVBT2;
				pRealtekADVBTX->m_TunerOptRegCnt = sizeof(TDA18275A_OPT_VALUE_DVBT2) / sizeof(REG_VALUE_ENTRY_RBUS);
				break;
			case TV_TUNER_SILICONLABS_SI2151:
			case TV_TUNER_SILICONLABS_SI2178B:
			case TV_TUNER_NO_TUNER:
				REALTEK_A_DVBTX_INFO("Tuner id %u, TV_TUNER_SILICONLABS_SI21XX\n", Param.id);
				pRealtekADVBTX->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) SI2151_OPT_VALUE_DVBT2;
				pRealtekADVBTX->m_TunerOptRegCnt = sizeof(SI2151_OPT_VALUE_DVBT2) / sizeof(REG_VALUE_ENTRY_RBUS);
				break;
			case TV_TUNER_RAFAEL_R840:
				REALTEK_A_DVBTX_INFO("Tuner id %u, TV_TUNER_RAFAEL_R840\n", Param.id);
				pRealtekADVBTX->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) R840_OPT_VALUE_DVBT2;
				pRealtekADVBTX->m_TunerOptRegCnt = sizeof(R840_OPT_VALUE_DVBT2) / sizeof(REG_VALUE_ENTRY_RBUS);
				break;
			case TV_TUNER_MAXLINEAR_MXL661:
				REALTEK_A_DVBTX_INFO("Tuner id %u, TV_TUNER_MAXLINEAR_MXL6X1\n", Param.id);
				pRealtekADVBTX->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) MXL661_OPT_VALUE_DVBT2;
				pRealtekADVBTX->m_TunerOptRegCnt = sizeof(MXL661_OPT_VALUE_DVBT2) / sizeof(REG_VALUE_ENTRY_RBUS);
				break;
			case TV_TUNER_SONY_SUTRX243:
			case TV_TUNER_SONY_SUTRX243_RA:
				REALTEK_A_DVBTX_INFO("Tuner id %u, TV_TUNER_SONY_SUTRX243\n", Param.id);
				pRealtekADVBTX->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) SUTRX243_OPT_VALUE_DVBT2;
				pRealtekADVBTX->m_TunerOptRegCnt = sizeof(SUTRX243_OPT_VALUE_DVBT2) / sizeof(REG_VALUE_ENTRY_RBUS);
				break;
			default:
				REALTEK_A_DVBTX_INFO("Unkonw tuner id %u, using default setting\n", Param.id);
				pRealtekADVBTX->m_pTunerOptRegRbus = (REG_VALUE_ENTRY_RBUS*) SI2151_OPT_VALUE_DVBT2;
				pRealtekADVBTX->m_TunerOptRegCnt = sizeof(SI2151_OPT_VALUE_DVBT2) / sizeof(REG_VALUE_ENTRY_RBUS);
				break;
			}
			pDemodKernel->m_if.freq      = Param.if_freq;
			pDemodKernel->m_if.inversion = Param.if_inversion;
			pDemodKernel->m_if.agc_pola  = Param.if_agc_pola;

			//if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK)  goto init_fail;

			REALTEK_A_DVBTX_INFO("if_freq = "PT_U32BITS", if_inversion = %u, if_agc_pola = %u\n", pDemodKernel->m_if.freq, pDemodKernel->m_if.inversion, pDemodKernel->m_if.agc_pola);
		}
	}
	//DtvPGAControl(PGAEnable, PGAGain);

	// Initial Driver
	if (pDemod->Initialize(pDemod) != FUNCTION_SUCCESS)
		goto init_fail;

	// Run Optimize Configuration for specified tuner
	for (i = 0; i < pRealtekADVBTX->m_TunerOptRegCnt; i++) {
		U32BITS Reg = pRealtekADVBTX->m_pTunerOptRegRbus[i].RegBitNameRbus;
		U32BITS Val = pRealtekADVBTX->m_pTunerOptRegRbus[i].ValueRbus;
		rtd_maskl(Reg, 0xffffff00, Val);
	}

	if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK)
		goto init_fail;


	if (pDemodKernel->SetTSMode(pDemodKernel, &pDemodKernel->m_ts) != TUNER_CTRL_OK) {
		goto init_fail;
	}

	REALTEK_A_DVBTX_DBG("REALTEK_ATB_DVBT2_Init Complete\n");


	return TUNER_CTRL_OK;

init_fail:
	REALTEK_A_DVBTX_WARNING("REALTEK_ATB_DVBT2_Init Fail\n");
	//pDemodKernel->Unlock(pDemodKernel);

	return TUNER_CTRL_FAIL;
}


/*----------------------------------------------------------------------
 * Func : SetTVSys
 *
 * Desc : Set TV System
 *
 * Parm : sys : TV System
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_SetTVSys(DEMOD* pDemodKernel, TV_SYSTEM sys)
{
	int ret;
	TV_SIG_INFO *pSigInfo;
	REALTEK_A_DVBTX* pRealtekADVBTX;

	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pSigInfo = kmalloc(sizeof(TV_SIG_INFO), GFP_KERNEL);
	if (pSigInfo == NULL) {
		REALTEK_A_DVBTX_WARNING("REALTEK_A_DVBTX_SetTVSys kmalloc Failed\n");
		return TUNER_CTRL_FAIL;
	}

	REALTEK_A_DVBTX_INFO("\033[1;32;31m" "%s %s %d\n" "\033[m", __FILE__, __func__, __LINE__);

	pSigInfo->dvbt2_ofdm.scan_mode = CH_NORMAL_MODE;
	pSigInfo->dvbt2_ofdm.mod = TV_MODULATION_DVBT2_OFDM;
	pSigInfo->dvbt2_ofdm.constellation = QAM_CONST_64;
	pSigInfo->dvbt2_ofdm.profile = 0;
	PreSqiNum = 0;
	SqiContinuousZeroNum = 0;
	gDvbT2DataPlpNumber = 0;

	ret = pDemodKernel->SetTVSysEx(pDemodKernel, sys, pSigInfo);
	kfree(pSigInfo);

	return ret;
}


/*----------------------------------------------------------------------
 * Func : SetTVSys
 *
 * Desc : Set TV System
 *
 * Parm : sys : TV System
 *        pSigInfo : signal information
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_SetTVSysEx(DEMOD* pDemodKernel, TV_SYSTEM sys, const TV_SIG_INFO* pSigInfo)
{
	int ret = TUNER_CTRL_OK;
	TUNER_PARAM Param;
	unsigned char SpectrumInvert = 0;

	//unsigned long tmp_time = 0;
	//tmp_time = tv_osal_time();

	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;
	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*) pRealtekADVBTX->m_private)->pDemod;

	//REALTEK_A_DVBTX_DBG("\033[1;32;31m" "%s %s %d\n" "\033[m", __FILE__, __func__, __LINE__);

	if (IsDvbt2Sys(sys) == 0 && IsDvbtSys(sys) == 0) {

		REALTEK_A_DVBTX_WARNING("Set TV system failed, unsupported TV system\n");
		return TUNER_CTRL_FAIL;
	}

	if (!pSigInfo) {

		REALTEK_A_DVBTX_WARNING("Siginfo doesn't specified, SetTVSysEx Fail\n");
		return TUNER_CTRL_FAIL;
	}

	DtvDemodTPOutEn(0);
	pRealtekADVBTX->m_auto_tune_en = 0;
	pDemodKernel->m_tv_sys = sys;
	pDemodKernel->m_ScanMode = pSigInfo->dvbt2_ofdm.scan_mode ? CH_SCAN_MODE : CH_NORMAL_MODE;


	//TUNER_PARAM Param;
	if (pDemodKernel->m_pTuner->GetParam(pDemodKernel->m_pTuner, sys, &Param) == TUNER_CTRL_OK) {
		if ((pDemodKernel->m_if.freq != Param.if_freq) || (pDemodKernel->m_if.inversion != Param.if_inversion) || (pDemodKernel->m_if.agc_pola  != Param.if_agc_pola)) {
			pDemodKernel->m_if.freq = Param.if_freq;
			pDemodKernel->m_if.inversion = Param.if_inversion;
			pDemodKernel->m_if.agc_pola = Param.if_agc_pola;

			if (pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK)  return  TUNER_CTRL_FAIL;
		}
	}

	pRealtekADVBTX->m_plp_id  = ((pSigInfo->dvbt2_ofdm.plp_select < 256) && (pSigInfo->dvbt2_ofdm.plp_select > 0)) ? pSigInfo->dvbt2_ofdm.plp_select : 0;
	pRealtekADVBTX->m_profileHP = (pSigInfo->dvbt2_ofdm.bProfileHP >= 1) ? HIERARCHY_MODE_HIGH : HIERARCHY_MODE_LOW;


	//Set Demod for Avoiding Any Problems
	switch (sys) {
	case TV_SYS_DVBT2_1D7M_PLP:
	case TV_SYS_DVBT2_1D7M:

		pRealtekADVBTX->m_current_bw = RTK_A_DVBTX_BANDWIDTH_1D7MHZ;
		break;
	case TV_SYS_DVBT2_5M_PLP:
	case TV_SYS_DVBT2_5M:
		pRealtekADVBTX->m_current_bw = RTK_A_DVBTX_BANDWIDTH_5MHZ;
		break;
	case TV_SYS_DVBT2_6M_PLP:
	case TV_SYS_DVBT2_6M:
	case TV_SYS_DVBT_6M:
	case TV_SYS_AUTOSCAN_6M:
		pRealtekADVBTX->m_current_bw = RTK_A_DVBTX_BANDWIDTH_6MHZ;
		break;
	case TV_SYS_DVBT2_7M_PLP:
	case TV_SYS_DVBT2_7M:
	case TV_SYS_DVBT_7M:
	case TV_SYS_AUTOSCAN_7M:
		pRealtekADVBTX->m_current_bw = RTK_A_DVBTX_BANDWIDTH_7MHZ;
		break;
	case TV_SYS_DVBT2_8M_PLP:
	case TV_SYS_DVBT2_8M:
	case TV_SYS_DVBT_8M:
	case TV_SYS_AUTOSCAN_8M:
	default:
		pRealtekADVBTX->m_current_bw = RTK_A_DVBTX_BANDWIDTH_8MHZ;
		break;

	}

	REALTEK_A_DVBTX_INFO("System = %d, BW = %uKHz, scan mode = %d, bSpectrumInv = %u, ProfileHP(T) = %u, PLP_ID(T2) = %u\n",
						 sys, pRealtekADVBTX->m_current_bw, pDemodKernel->m_ScanMode, SpectrumInvert, pRealtekADVBTX->m_profileHP, pRealtekADVBTX->m_plp_id);

	pDemodKernel->Lock(pDemodKernel);
	rtd_maskl(DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_1_reg, 0xFFFFEFFF, 0x1000);  //restore to default for Uganda field

	RTK_A_DVBTX_Write(pDemod, 0x17, 0xBE, 0x01); // clear synderr flag
	RTK_A_DVBTX_Write(pDemod, 0x17, 0xBE, 0x00);
	//FEC Reset Turn On(Default setting)
	RTK_A_DVBTX_Write(pDemod, 0x17, 0xBB, 0xF1);
	RTK_A_DVBTX_Write(pDemod, 0x17, 0xD9, 0xEF);
	RTK_A_DVBTX_Write(pDemod, 0x17, 0x30, 0x00); //for muitiPLP, it should use default value

	ret |= DECODE_RET(pDemod->SetBandwidthMode(pDemod, pRealtekADVBTX->m_current_bw));

	if (pDemodKernel->m_ScanMode == CH_NORMAL_MODE) {
		ret |= DECODE_RET(pDemod->SetT2PlpID(pDemod, pRealtekADVBTX->m_plp_id));
		ret |= DECODE_RET(pDemod->SetTHierarchy(pDemod, pRealtekADVBTX->m_profileHP));
		//ret |= DECODE_RET(pDemod->SetCellIdCheckOn(pDemod, 0));
	} else {
		ret |= DECODE_RET(pDemod->SetTHierarchy(pDemod, pRealtekADVBTX->m_profileHP));
		//ret |= DECODE_RET(pDemod->SetCellIdCheckOn(pDemod, 1));
	}

	pDemodKernel->Unlock(pDemodKernel);

	pRealtekADVBTX->m_status_checking_stime = tv_osal_time() + DVBTX_STATUS_CHECK_INTERVAL_MS;
	pRealtekADVBTX->m_acquire_sig_en = 1;
	pRealtekADVBTX->m_unlock_cnt = 0;
	pRealtekADVBTX->m_auto_tune_en = 1;

	acquire_sig_stime = tv_osal_time();
	//REALTEK_A_DVBTX_INFO("System = %d, BW = %uKHz, scan mode = %d, bSpectrumInv = %u, ProfileHP(T) = %u, PLP_ID(T2) = %u, time = %u\n",
	//					 sys, pRealtekADVBTX->m_current_bw, pDemodKernel->m_ScanMode, SpectrumInvert, pRealtekADVBTX->m_profileHP, pRealtekADVBTX->m_plp_id,  tv_osal_time() - tmp_time);


	return ret;
}


/*----------------------------------------------------------------------
 * Func : SetIF
 *
 * Desc : Set IF Parameter
 *
 * Parm : pParam : IF Param
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_SetIF(DEMOD* pDemodKernel, IF_PARAM* pParam)
{
	int ret = TUNER_CTRL_OK;

	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;
	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*) pRealtekADVBTX->m_private)->pDemod;

	//REALTEK_A_DVBTX_INFO("\033[1;32;31m" "%s %s %d\n" "\033[m", __FILE__, __func__, __LINE__);
	pDemodKernel->Lock(pDemodKernel);

	ret |= DECODE_RET(pDemod->SetIfFreqHz(pDemod, (unsigned int) pParam->freq));
	ret |= DECODE_RET(pDemod->SetAgcPol(pDemod, pParam->agc_pola));


	REALTEK_A_DVBTX_DBG("SetIF, IF_freq = "PT_U32BITS", spectrumn inv = %u, agc_pola = %u\n", pParam->freq, pParam->inversion, pParam->agc_pola);
	pDemodKernel->m_if = *pParam;

	pDemodKernel->Unlock(pDemodKernel);

	return ret;
}


/*----------------------------------------------------------------------
 * Func : SetMode
 *
 * Desc : Set operation mode of demod
 *
 * Parm : mode : Demod opreation mode
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_SetMode(DEMOD* pDemodKernel, TV_OP_MODE mode)
{
	REALTEK_A_DVBTX* pRealtekADVBTX;
	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);

	//REALTEK_A_DVBTX_INFO("\033[1;32;31m" "%s %s %d\n" "\033[m", __FILE__, __func__, __LINE__);

	switch (mode) {
	case TV_OP_MODE_NORMAL:
	case TV_OP_MODE_SCANING:
		break;

	case TV_OP_MODE_STANDBY:
		break;

	case TV_OP_MODE_ATV_NORMAL:
	case TV_OP_MODE_TP_ONLY:
		break;

	default:
		break;
	}
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : SetTSMode
 *
 * Desc : Set mode of transport stream output
 *
 * Parm : pParam : TS Param
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_SetTSMode(DEMOD* pDemodKernel, TS_PARAM* pParam)
{
	int ret = TUNER_CTRL_OK;

	RTK_A_DVBTX_MPEGTSMode stTSMode;

	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;
	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*) pRealtekADVBTX->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);

	stTSMode.ui8TSTransferType = (pParam->mode) ? RTK_A_DVBTX_TS_SERIAL_MODE : RTK_A_DVBTX_TS_PARALLEL_MODE;
	stTSMode.ui8OutputEdge = (pParam->clk_pol) ? RTK_A_DVBTX_TS_OUTPUT_RISING_EDGE : RTK_A_DVBTX_TS_OUTPUT_FALLING_EDGE;
	stTSMode.ui8TSSPIMSBSelection = (pParam->data_order) ? RTK_A_DVBTX_TS_SPI_MSB_ON_DATA_BIT0 : RTK_A_DVBTX_TS_SPI_MSB_ON_DATA_BIT7;
	stTSMode.ui8TSSSIOutputSelection = RTK_A_DVBTX_TS_SSI_OUTPUT_ON_DATA_BIT0;
	stTSMode.ui8SPIClockConstantOutput = RTK_A_DVBTX_TS_CLOCK_CONST_OUTPUT;
	stTSMode.ui8DVBTxCTSClockVariableMode  = RTK_A_DVBTX_TS_CLOCK_VARIABLE;


	ret |= DECODE_RET(pDemod->SetTsInterfaceMode(pDemod, stTSMode));

	DtvDemodTPOutEn(pParam->tsOutEn);


	if (ret == TUNER_CTRL_OK)
		pDemodKernel->m_ts = *pParam;

	pDemodKernel->Unlock(pDemodKernel);

	return ret;
}


/*----------------------------------------------------------------------
 * Func : GetLockStatus
 *
 * Desc : Get Demod Lock Status
 *
 * Parm : pLock : Lock Status
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_GetLockStatus(DEMOD* pDemodKernel, unsigned char* pLock)
{
	int ret = TUNER_CTRL_OK;
	unsigned char StandardSearchResult = 0;
	int LockStatus = 0;

	int SyncT2LockFlag = 0, T2L1LockFlag = 0;
	unsigned int Snr = 0;
	unsigned int Snr_thd = 250000;
	unsigned char u8Synderr = 0;
	unsigned int retry_cnt = 0;

	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;
	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*)pRealtekADVBTX->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);

RETRY_LOCKSTATUS:
	ret |= DECODE_RET(pDemod->GetDVBTxMode(pDemod, &StandardSearchResult));

	ret |= DECODE_RET(pDemod->IsSignalLocked(pDemod, &LockStatus));

	if (LockStatus == 0) {
		ret |= DECODE_RET(pDemod->IsDvbt2SyncLocked(pDemod, &SyncT2LockFlag));
		ret |= DECODE_RET(pDemod->IsDvbt2L1Locked(pDemod, &T2L1LockFlag));

		if ((SyncT2LockFlag == 1) && (T2L1LockFlag == 1)) {
			ret |= DECODE_RET(pDemod->GetSnrDb(pDemod, &Snr));
			Snr = Snr * 1000; //(Snr/10)*10000
			if (Snr > Snr_thd)
				LockStatus = YES;
		}
	}
	RTK_A_DVBTX_Read(pDemod, 0x17, 0xF6,  &u8Synderr);
	if ((LockStatus == 0) && ((u8Synderr & 0x20) == 0x20)  && (retry_cnt < DVBT2_SYNDERR_RETRY_MAX)) {
		retry_cnt++;
		tv_osal_msleep(10);
		goto RETRY_LOCKSTATUS;
	}
	if ((u8Synderr & 0x20) == 0x20) {
		RTK_A_DVBTX_Write(pDemod, 0x17, 0xBE, 0x01); // clear synderr flag
		RTK_A_DVBTX_Write(pDemod, 0x17, 0xBE, 0x00);
	}
	if (ret == TUNER_CTRL_OK) {
		pRealtekADVBTX->m_dvbt2_dvbt_mode = StandardSearchResult;
		*pLock = LockStatus ? DTV_SIGNAL_LOCK : DTV_SIGNAL_NOT_LOCK;
	} else {
		*pLock = DTV_SIGNAL_NOT_LOCK;
	}

	REALTEK_A_DVBTX_DBG("[lock status] IsDVBT = %u, *pLock = %u\n", (pRealtekADVBTX->m_dvbt2_dvbt_mode == RTK_A_DVBTX_DVBT_MODE) ? 1 : 0, *pLock);
	pDemodKernel->Unlock(pDemodKernel);

	return ret;
}


/*----------------------------------------------------------------------
 * Func : GetSignalInfo
 *
 * Desc : Get Signal Information
 *
 * Parm : pInfo : Siganl Information
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_GetSignalInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo)
{
	int ret = TUNER_CTRL_OK;

	RTK_A_DVBTX_DVBT2_PARAMS Dvbt2Params;
	RTK_A_DVBTX_DVBT_PARAMS DvbtParams;
	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;

	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*)pRealtekADVBTX->m_private)->pDemod;

	if (pRealtekADVBTX->m_dvbt2_dvbt_mode == RTK_A_DVBTX_DVBT2_MODE) {
		pInfo->mod = TV_MODULATION_DVBT2_OFDM;

		pDemodKernel->Lock(pDemodKernel);

		ret |= DECODE_RET(pDemod->GetSignalParamsT2(pDemod, &Dvbt2Params));

		pDemodKernel->Unlock(pDemodKernel);

		switch (pRealtekADVBTX->m_current_bw) {
		case RTK_A_DVBTX_BANDWIDTH_1D7MHZ:
			pInfo->dvbt2_ofdm.bandwidth = 1700000;
			break;
		case RTK_A_DVBTX_BANDWIDTH_5MHZ:
			pInfo->dvbt2_ofdm.bandwidth = 5000000;
			break;
		case RTK_A_DVBTX_BANDWIDTH_6MHZ:
			pInfo->dvbt2_ofdm.bandwidth = 6000000;
			break;
		case RTK_A_DVBTX_BANDWIDTH_7MHZ:
			pInfo->dvbt2_ofdm.bandwidth = 7000000;
			break;
		case RTK_A_DVBTX_BANDWIDTH_8MHZ:
		default:
			pInfo->dvbt2_ofdm.bandwidth = 8000000;
			break;

		}

		switch (Dvbt2Params.Dvbt2FFTMode) {
		case 0://FFT 1K
			pInfo->dvbt2_ofdm.fft_mode = OFDM_FFT_1K;
			break;
		case 1://FFT 2K
			pInfo->dvbt2_ofdm.fft_mode = OFDM_FFT_2K;
			break;
		case 2://FFT 4K
			pInfo->dvbt2_ofdm.fft_mode = OFDM_FFT_4K;
			break;
		case 3://FFT 8K
			pInfo->dvbt2_ofdm.fft_mode = OFDM_FFT_8K;
			break;
		case 4://FFT 16K
			pInfo->dvbt2_ofdm.fft_mode = OFDM_FFT_16K;
			break;
		case 5://FFT 32K
		default:
			pInfo->dvbt2_ofdm.fft_mode = OFDM_FFT_32K;
			break;
		}

		//T2 bandwidth extension type
		pInfo->dvbt2_ofdm.bwExt = (Dvbt2Params.Dvbt2BandwidthExt == RTK_A_DVBTX_DVBT2_BANDWIDTH_EXTENDED) ? 1 : 0;


		switch (Dvbt2Params.Dvbt2GuardInterval) {
		case 0://Guard 1/32
			pInfo->dvbt2_ofdm.guard_interval = OFDM_GI_1_32;
			break;
		case 1://Guard 1/16
			pInfo->dvbt2_ofdm.guard_interval = OFDM_GI_1_16;
			break;
		case 2://Guard 1/8
			pInfo->dvbt2_ofdm.guard_interval = OFDM_GI_1_8;
			break;
		case 3://Guard 1/4
			pInfo->dvbt2_ofdm.guard_interval = OFDM_GI_1_4;
			break;
		case 4://Guard 1/128
			pInfo->dvbt2_ofdm.guard_interval = OFDM_GI_1_128;
			break;
		case 5://Guard 19/128
			pInfo->dvbt2_ofdm.guard_interval = OFDM_GI_19_128;
			break;
		case 6://Guard 19/256
		default:
			pInfo->dvbt2_ofdm.guard_interval = OFDM_GI_19_256;
			break;
		}

		switch (Dvbt2Params.Dvbt2CodeRate) {
		case 0://Code Rate 1/2
			pInfo->dvbt2_ofdm.code_rate_hp = OFDM_CODE_RATE_1_2;
			break;
		case 1://Code Rate 3/5
			pInfo->dvbt2_ofdm.code_rate_hp = OFDM_CODE_RATE_3_5;
			break;
		case 2://Code Rate 2/3
			pInfo->dvbt2_ofdm.code_rate_hp = OFDM_CODE_RATE_2_3;
			break;
		case 3://Code Rate 3/4
			pInfo->dvbt2_ofdm.code_rate_hp = OFDM_CODE_RATE_3_4;
			break;
		case 4://Code Rate 4/5
			pInfo->dvbt2_ofdm.code_rate_hp = OFDM_CODE_RATE_4_5;
			break;
		case 5://Code Rate 5/6
		default:
			pInfo->dvbt2_ofdm.code_rate_hp = OFDM_CODE_RATE_5_6;
			break;
		}

		switch (Dvbt2Params.Dvbt2Constellation) {
		case 0://QPSK
			pInfo->dvbt2_ofdm.constellation = OFDM_CONST_4;
			break;
		case 1://16QAM
			pInfo->dvbt2_ofdm.constellation = OFDM_CONST_16;
			break;
		case 2://64QAM
			pInfo->dvbt2_ofdm.constellation = OFDM_CONST_64;
			break;
		case 3://256QAM
		default:
			pInfo->dvbt2_ofdm.constellation = OFDM_CONST_256;
			break;
		}


		switch (Dvbt2Params.Dvbt2PilotPattern) {
		case RTK_A_DVBTX_DVBT2_PP1:
			pInfo->dvbt2_ofdm.PilotPattern = DVBT2_PP1;
			break;
		case RTK_A_DVBTX_DVBT2_PP2:
			pInfo->dvbt2_ofdm.PilotPattern = DVBT2_PP2;
			break;
		case RTK_A_DVBTX_DVBT2_PP3:
			pInfo->dvbt2_ofdm.PilotPattern = DVBT2_PP3;
			break;
		case RTK_A_DVBTX_DVBT2_PP4:
			pInfo->dvbt2_ofdm.PilotPattern = DVBT2_PP4;
			break;
		case RTK_A_DVBTX_DVBT2_PP5:
			pInfo->dvbt2_ofdm.PilotPattern = DVBT2_PP5;
			break;
		case RTK_A_DVBTX_DVBT2_PP6:
			pInfo->dvbt2_ofdm.PilotPattern = DVBT2_PP6;
			break;
		case RTK_A_DVBTX_DVBT2_PP7:
			pInfo->dvbt2_ofdm.PilotPattern = DVBT2_PP7;
			break;
		case RTK_A_DVBTX_DVBT2_PP8:
			pInfo->dvbt2_ofdm.PilotPattern = DVBT2_PP8;
			break;
		case RTK_A_DVBTX_DVBT2_PP_RESERVED8:
			pInfo->dvbt2_ofdm.PilotPattern = DVBT2_PP_RESERVED8;
			break;
		case RTK_A_DVBTX_DVBT2_PP_RESERVED9:
			pInfo->dvbt2_ofdm.PilotPattern = DVBT2_PP_RESERVED9;
			break;
		case RTK_A_DVBTX_DVBT2_PP_RESERVED10:
			pInfo->dvbt2_ofdm.PilotPattern = DVBT2_PP_RESERVED10;
			break;
		case RTK_A_DVBTX_DVBT2_PP_RESERVED11:
			pInfo->dvbt2_ofdm.PilotPattern = DVBT2_PP_RESERVED11;
			break;
		case RTK_A_DVBTX_DVBT2_PP_RESERVED12:
			pInfo->dvbt2_ofdm.PilotPattern = DVBT2_PP_RESERVED12;
			break;
		case RTK_A_DVBTX_DVBT2_PP_RESERVED13:
			pInfo->dvbt2_ofdm.PilotPattern = DVBT2_PP_RESERVED13;
			break;
		case RTK_A_DVBTX_DVBT2_PP_RESERVED14:
			pInfo->dvbt2_ofdm.PilotPattern = DVBT2_PP_RESERVED14;
			break;
		case RTK_A_DVBTX_DVBT2_PP_RESERVED15:
			pInfo->dvbt2_ofdm.PilotPattern = DVBT2_PP_RESERVED15;
			break;

		case RTK_A_DVBTX_DVBT2_PP_UNKNOWN:
		default:
			pInfo->dvbt2_ofdm.PilotPattern = DVBT2_PP_UNKNOWN;
			break;

		}

		pInfo->dvbt2_ofdm.cell_ID = Dvbt2Params.Dvbt2CellId;
		pInfo->dvbt2_ofdm.bSpectrumInv = Dvbt2Params.Dvbt2IQSwap;
		pInfo->dvbt2_ofdm.plp_select = Dvbt2Params.u8ActivePlpId;

		REALTEK_A_DVBTX_INFO("T2::GetSignalInfo, FFT = %u, GUARD_INT = %u, CODE_RATE = %x, CONSTELLATION = %u, CELL_ID = %u, SPEC_INV = %u, bandwidth = %uHz, BW_Ext_type=%u, CELL_ID = %u\n",
							 pInfo->dvbt2_ofdm.fft_mode, pInfo->dvbt2_ofdm.guard_interval, pInfo->dvbt2_ofdm.code_rate_hp, pInfo->dvbt2_ofdm.constellation, pInfo->dvbt2_ofdm.cell_ID, pInfo->dvbt2_ofdm.bSpectrumInv, pInfo->dvbt2_ofdm.bandwidth, pInfo->dvbt2_ofdm.bwExt, pInfo->dvbt2_ofdm.plp_select);
	} else {
		pInfo->mod = TV_MODULATION_OFDM;

		pDemodKernel->Lock(pDemodKernel);

		ret |= DECODE_RET(pDemod->GetSignalParamsT(pDemod, &DvbtParams));

		pDemodKernel->Unlock(pDemodKernel);

		pInfo->dvbt2_ofdm.bSpectrumInv = DvbtParams.DvbtIQSwap;
		pInfo->dvbt2_ofdm.bProfileHP = DvbtParams.DvbtProfile ? 0 : 1;

		switch (pRealtekADVBTX->m_current_bw) {
		case RTK_A_DVBTX_BANDWIDTH_1D7MHZ:
			pInfo->dvbt2_ofdm.bandwidth = 1700000;
			break;
		case RTK_A_DVBTX_BANDWIDTH_5MHZ:
			pInfo->dvbt2_ofdm.bandwidth = 5000000;
			break;
		case RTK_A_DVBTX_BANDWIDTH_6MHZ:
			pInfo->dvbt2_ofdm.bandwidth = 6000000;
			break;
		case RTK_A_DVBTX_BANDWIDTH_7MHZ:
			pInfo->dvbt2_ofdm.bandwidth = 7000000;
			break;
		case RTK_A_DVBTX_BANDWIDTH_8MHZ:
		default:
			pInfo->dvbt2_ofdm.bandwidth = 8000000;
			break;

		}

		switch (DvbtParams.DvbtHierarchy) {
		case 0://Alpha 0
			pInfo->dvbt2_ofdm.hierarchy = OFDM_ALPHA_0;
			break;
		case 1://Alpha 1
			pInfo->dvbt2_ofdm.hierarchy = OFDM_ALPHA_1;
			break;
		case 2://Alpha 2
			pInfo->dvbt2_ofdm.hierarchy = OFDM_ALPHA_2;
			break;
		case 3://Alpha 4
		default:
			pInfo->dvbt2_ofdm.hierarchy = OFDM_ALPHA_4;
			break;
		}

		switch (DvbtParams.DvbtFFTMode) {
		case 0://FFT 2K
			pInfo->dvbt2_ofdm.fft_mode = OFDM_FFT_2K;
			break;
		case 1://FFT 8K
		default:
			pInfo->dvbt2_ofdm.fft_mode = OFDM_FFT_8K;
			break;
		}

		switch (DvbtParams.DvbtGuardInterval) {
		case 0://Guard 1/32
			pInfo->dvbt2_ofdm.guard_interval = OFDM_GI_1_32;
			break;
		case 1://Guard 1/16
			pInfo->dvbt2_ofdm.guard_interval = OFDM_GI_1_16;
			break;
		case 2://Guard 1/8
			pInfo->dvbt2_ofdm.guard_interval = OFDM_GI_1_8;
			break;
		case 3://Guard 1/4
		default:
			pInfo->dvbt2_ofdm.guard_interval = OFDM_GI_1_4;
			break;
		}

		switch (DvbtParams.DvbtHpCodeRate) {
		case 0://Code Rate 1/2
			pInfo->dvbt2_ofdm.code_rate_hp = OFDM_CODE_RATE_1_2;
			break;
		case 1://Code Rate 2/3
			pInfo->dvbt2_ofdm.code_rate_hp = OFDM_CODE_RATE_2_3;
			break;
		case 2://Code Rate 3/4
			pInfo->dvbt2_ofdm.code_rate_hp = OFDM_CODE_RATE_3_4;
			break;
		case 3://Code Rate 5/6
			pInfo->dvbt2_ofdm.code_rate_hp = OFDM_CODE_RATE_5_6;
			break;
		case 4://Code Rate 7/8
		default:
			pInfo->dvbt2_ofdm.code_rate_hp = OFDM_CODE_RATE_7_8;
			break;
		}

		switch (DvbtParams.DvbtConstellation) {
		case 0://QPSK
			pInfo->dvbt2_ofdm.constellation = OFDM_CONST_4;
			break;
		case 1://16QAM
			pInfo->dvbt2_ofdm.constellation = OFDM_CONST_16;
			break;
		case 2://64QAM
		default:
			pInfo->dvbt2_ofdm.constellation = OFDM_CONST_64;
			break;
		}

		pInfo->dvbt2_ofdm.cell_ID = DvbtParams.DvbtCellId;

		REALTEK_A_DVBTX_INFO("T::GetSignalInfo, SPEC_INV = %u, PROFILE = %u, ALPHA = %u, FFT = %u, GUARD_INT = %u, CODE_RATE = %x, CONSTELLATION = %u, bandwidth = %uHz, CELL_ID = %u\n",
							 pInfo->dvbt2_ofdm.bSpectrumInv, pInfo->dvbt2_ofdm.bProfileHP, pInfo->dvbt2_ofdm.hierarchy, pInfo->dvbt2_ofdm.fft_mode, pInfo->dvbt2_ofdm.guard_interval, pInfo->dvbt2_ofdm.code_rate_hp, pInfo->dvbt2_ofdm.constellation, pInfo->dvbt2_ofdm.bandwidth, pInfo->dvbt2_ofdm.cell_ID);
	}

	return ret;
}


/*----------------------------------------------------------------------
 * Func : GetSignalQuality
 *
 * Desc : Get Signal Information
 *
 * Parm : id : Type of Quality Information
 *        pQuality : Siganl Quality
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_GetSignalQuality(DEMOD* pDemodKernel, ENUM_TV_QUAL id, TV_SIG_QUAL* pQuality)
{
	int ret = TUNER_CTRL_OK;
	unsigned int Snr, PakN, BadPakN, BitN, BadBitN;
	unsigned int Agc;
	unsigned int Sqi, Ssi;
	unsigned short SqiTmp = 0;
	int i;
	int TxRSSIdBm;

	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;
	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*) pRealtekADVBTX->m_private)->pDemod;

	//if (!DtvIsDemodClockEnable(RTK_DEMOD_MODE_DVBT2))
	//return TUNER_CTRL_FAIL;

	pDemodKernel->Lock(pDemodKernel);

	switch (id) {
	case TV_QUAL_SNR:
		ret |= DECODE_RET(pDemod->GetSnrDb(pDemod, &Snr));
		pQuality->snr = Snr * 1000; //(Snr/10)*10000
		break;
	case TV_QUAL_BER:
	case TV_QUAL_BEC:
		ret |= DECODE_RET(pDemod->GetBer(pDemod, &BadBitN, &BitN));
		if (BitN > 0) {
			pQuality->layer_ber.ber_num = BadBitN;
			pQuality->layer_ber.ber_den = BitN;
			pQuality->ber = (BadBitN * 10000) / BitN;
		} else {
			pQuality->layer_ber.ber_num = 0xffffffff;
			pQuality->layer_ber.ber_den = 0xffffffff;
			pQuality->ber = 0xffffffff;
		}

		break;
	case TV_QUAL_PER:
	case TV_QUAL_PEC:
	case TV_QUAL_UCBLOCKS:
		ret |= DECODE_RET(pDemod->GetPer(pDemod, &BadPakN, &PakN));
		if (PakN > 0) {
			pQuality->layer_per.per_num = BadPakN;
			pQuality->layer_per.per_den = PakN;
			pQuality->per = (BadPakN * 10000) / PakN;
		} else {
			pQuality->layer_per.per_num = 0xffffffff;
			pQuality->layer_per.per_den = 0xffffffff;
			pQuality->per = 0xffffffff;
		}

		pQuality->ucblocks = BadPakN;
		break;
	case TV_QUAL_SIGNAL_STRENGTH:
		TxRSSIdBm = (int) pQuality-> rflevel;
		ret |= DECODE_RET(pDemod->GetSignalStrength(pDemod, TxRSSIdBm, &Ssi));
		pQuality->strength = (unsigned char) Ssi;
		break;
	case TV_QUAL_SIGNAL_QUALITY:
		ret |= DECODE_RET(pDemod->GetLayerSignalQuality(pDemod, &Sqi));
		pQuality->quality = (unsigned char) Sqi;
		break;
	case TV_QUAL_AGC:
		ret |= DECODE_RET(pDemod->GetIfAgc(pDemod, &Agc));
		pQuality->agc = (U32BITS)Agc;
		break;
	case TV_QUAL_DEMODFWVERSION:
		pQuality->DemodFwVersion = 0x00000109;//V1.0.9
		break;
	case TV_QUAL_CUSTOMER_LG:
		TxRSSIdBm = (int) pQuality-> rflevel;
		ret |= DECODE_RET(pDemod->GetSnrDb(pDemod, &Snr));
		ret |= DECODE_RET(pDemod->GetBer(pDemod, &BadBitN, &BitN));
		ret |= DECODE_RET(pDemod->GetPer(pDemod, &BadPakN, &PakN));
		ret |= DECODE_RET(pDemod->GetSignalStrength(pDemod, TxRSSIdBm, &Ssi));
		ret |= DECODE_RET(pDemod->GetLayerSignalQuality(pDemod, &Sqi));
		pQuality->strength = (unsigned char) Ssi;

		pQuality->snr = Snr * 1000; //(Snr/10)*10000

		if (PakN > 0) {
			pQuality->layer_per.per_num = BadPakN;
			pQuality->layer_per.per_den = PakN;
			pQuality->layer_ber.ber_num = BadBitN;
			pQuality->layer_ber.ber_den = BitN;
		} else {
			pQuality->layer_per.per_num = 0xffffffff;
			pQuality->layer_per.per_den = 0xffffffff;
			pQuality->layer_ber.ber_num = 0xffffffff;
			pQuality->layer_ber.ber_den = 0xffffffff;
		}


#if 1
		//Sqi with 5 windows[begin]
		if (Sqi == 0) {
			if (PreSqiNum == 0) {
				REALTEK_A_DVBTX_INFO("Case1: SqiContinuousZeroNum = %d, PreSqiNum = %d\n",
									 SqiContinuousZeroNum, PreSqiNum);
				SqiTmp = 0;
			} else if (SqiContinuousZeroNum == 0) {
				REALTEK_A_DVBTX_INFO("Case2: SqiContinuousZeroNum = %d, PreSqiNum = %d\n",
									 SqiContinuousZeroNum, PreSqiNum);
				for (i = 0; i < PreSqiNum; i++) {
					SqiTmp += PreSqi[i];
				}
				SqiTmp = SqiTmp / PreSqiNum;
				SqiContinuousZeroNum++;
			} else {
				REALTEK_A_DVBTX_INFO("Case3: SqiContinuousZeroNum = %d, PreSqiNum = %d\n",
									 SqiContinuousZeroNum, PreSqiNum);
				SqiContinuousZeroNum++;
				PreSqiNum = 0;
				SqiTmp = 0;
			}

		} else { //Sqi > 0

			SqiContinuousZeroNum = 0;
			if (PreSqiNum < PRE_SQI_MAX_QUANTITY) {
				PreSqi[PreSqiNum] = Sqi;
				PreSqiNum ++;
			} else {
				for (i = 0; i < (PRE_SQI_MAX_QUANTITY - 1); i++) {
					PreSqi[i] = PreSqi[i + 1];
				}
				PreSqi[PRE_SQI_MAX_QUANTITY - 1] = Sqi;
			}

			for (i = 0; i < PreSqiNum; i++) {
				SqiTmp += PreSqi[i];
			}
			SqiTmp = SqiTmp / PreSqiNum;
		}
		REALTEK_A_DVBTX_INFO("PreSqi[0] = %d, PreSqi[1] = %d, PreSqi[2] = %d, PreSqi[3] = %d, PreSqi[4] = %d, Sqi =%d, PreSqiNum =%d, SqiTmp =%d, SqiContinuousZeroNum = %d\n",
							 PreSqi[0], PreSqi[1], PreSqi[2], PreSqi[3], PreSqi[4], Sqi, PreSqiNum, SqiTmp, SqiContinuousZeroNum);
		pQuality->quality = SqiTmp;
		//Sqi with 5 windows[end]
#endif
		ret |= DECODE_RET(pDemod->GetIfAgc(pDemod, &Agc));
		pQuality->agc = (U32BITS)Agc;

		REALTEK_A_DVBTX_INFO("\033[1;32;31m" "IsDVBT = %u, SNR = "PT_U32BITS", AGC = "PT_U32BITS", pkt_error = "PT_U32BITS", total_pkt = "PT_U32BITS"\n" "\033[m",
							 (pRealtekADVBTX->m_dvbt2_dvbt_mode == RTK_A_DVBTX_DVBT_MODE) ? 1 : 0, pQuality->snr, pQuality->agc, pQuality->layer_per.per_num, pQuality->layer_per.per_den);
		break;
	default:
		break;
	}

	pDemodKernel->Unlock(pDemodKernel);

	return ret;
}


/*----------------------------------------------------------------------
 * Func : GetCarrierOffset
 *
 * Desc : Get the carrier offset.
 *
 * Parm : pOffset : carrier frequency offset output (unit : Hz)
 *
 *          Note: the offset value is only valid as demod is on lock stage.
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_GetCarrierOffset(DEMOD* pDemodKernel, S32BITS* pOffset)
{
	int ret = TUNER_CTRL_OK;
	int Cfo = 0;

	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;
	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*) pRealtekADVBTX->m_private)->pDemod;

	//if (!DtvIsDemodClockEnable(RTK_DEMOD_MODE_DVBT2))
	//return TUNER_CTRL_FAIL;

	pDemodKernel->Lock(pDemodKernel);

	ret = DECODE_RET(pDemod->GetCrOffsetHz(pDemod, &Cfo));
	//*pOffset = (S32BITS)((-1) * Cfo);
	*pOffset = (S32BITS) Cfo;

	pDemodKernel->Unlock(pDemodKernel);
	REALTEK_A_DVBTX_INFO("GetCarrierOffset = "PT_S32BITS"Hz\n", *pOffset);

	return ret;
}
/*----------------------------------------------------------------------
 * Func : GetDebugLogBuf
 *
 * Desc : Get the debug log Buf.
 *
 * Parm :
 *
 *
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_GetDebugLogBuf(DEMOD* pDemodKernel, unsigned int* LogBufAddr, unsigned int* LogBufSize)
{
	int ret = TUNER_CTRL_OK;

	return ret;
}


/*----------------------------------------------------------------------
 * Func : AcquireSignal
 *
 * Desc : Reset Demodulator and Acquire Signal.
 *        This function should be called when channel changed.
 *
 * Parm : WaitSignalLock : wait until signal lock
 *          0 : don't wait,
 *          others: wait for signal lock
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_AcquireSignal(DEMOD* pDemodKernel, unsigned char WaitSignalLock)
{

#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	unsigned long cur_time;
#endif

	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;
	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*) pRealtekADVBTX->m_private)->pDemod;


#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	cur_time = tv_osal_time();

	while ((tv_osal_time() - cur_time) <= 15) {

		if (pRealtekADVBTX->m_acquire_sig_en == 0)
			return TUNER_CTRL_OK;

		tv_osal_msleep(5);
	}

	return TUNER_CTRL_FAIL;

#else
	REALTEK_A_DVBTX_AcquireSignalThread(pDemodKernel);
	pRealtekADVBTX->m_autotune_stime = tv_osal_time() + AUTODETECT_SIG_UPDATE_INTERVAL_MS;
	pRealtekADVBTX->m_autotune_TsTime = tv_osal_time() + AUTOTUNE_UPDATE_TS_CLK_INTERVAL_MS;
	pRealtekADVBTX->m_dvbt2_PLPcheck_stime = tv_osal_time() + AUTOTUNE_T2PLP_CHECK_INTERVAL_MS;
	return TUNER_CTRL_OK;
#endif
}


/*----------------------------------------------------------------------
 * Func : AcquireSignalThread
 *
 * Desc : Reset Demodulator and Acquire Signal.
 *        This function should be called when channel changed.
 *
 * Parm :
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_AcquireSignalThread(DEMOD* pDemodKernel)
{
	int ret = TUNER_CTRL_OK;
	unsigned long det_stime = 0, det_t_stime;
	int SyncTLockFlag = 0, SyncT2LockFlag = 0, FecLockFlag = 0, u8SignalLockedFlag = 0, lock = 0, T2L1LockFlag = 0;
	unsigned int Snr = 0;
	RTK_A_DVBTX_DVBT2_PARAMS DvbT2Params;
	unsigned int dvbt_timeout = 1200, dvbt2_timeout = 1500, NoSignalTimeOut = 320, dvbt_prelock_timeout = 380, SignalLockedFlagTimeout = 950;  //550->950 tmp changed, 2018.5.25.
	unsigned int Snr_thd = 260000, dvbt2_L1LockCheck_timeout = 380;
	unsigned char StandardSearchResult = 0, u8Read_data = 0, u8Read_data1 = 0;
	int SyncTLockFlagTmp = 0;
	unsigned char Sync1FecRstDisableFlag = 0;
	unsigned int PakN, BadPakN;
	unsigned long syncT_PecCheck_Timeout = 3000;
	unsigned int  plp_intf_fsm = 0, plpIntfFsmCnt = 0, dbgData = 0;
	unsigned char plpIntfFsmStuckCheckFlag = 0;

	RTK_A_DVBTX_DVBT_PARAMS DvbtParams;

	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;


	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*) pRealtekADVBTX->m_private)->pDemod;

	if (pRealtekADVBTX->m_acquire_sig_en == 0)
		return TUNER_CTRL_OK;

	REALTEK_A_DVBTX_DBG("\033[1;32;32m" "%s %s %d\n" "\033[m", __FILE__, __func__, __LINE__);
	pDemodKernel->Lock(pDemodKernel);


	switch (pRealtekADVBTX->m_current_bw) {
	case RTK_A_DVBTX_BANDWIDTH_1D7MHZ:
		dvbt_prelock_timeout = 500;
		break;
	case RTK_A_DVBTX_BANDWIDTH_5MHZ:
		dvbt_prelock_timeout = 500;
		break;
	case RTK_A_DVBTX_BANDWIDTH_6MHZ:
		dvbt_prelock_timeout = 450;
		break;
	case RTK_A_DVBTX_BANDWIDTH_7MHZ:
		dvbt_prelock_timeout = 420;
		break;
	case RTK_A_DVBTX_BANDWIDTH_8MHZ:
	default:
		dvbt_prelock_timeout = 380;
		break;

	}

	det_stime = tv_osal_time();

	while ((tv_osal_time() - acquire_sig_stime) <= dvbt2_timeout) {
		if ((tv_osal_time() - acquire_sig_stime) > NoSignalTimeOut) {
			pDemod->MonitorT2TxID(pDemod);  //check L1 signals
		}

		ret |= DECODE_RET(pDemod->IsSignalLocked(pDemod, &FecLockFlag));                    //DVBT FEC locked
		ret |= DECODE_RET(pDemod->IsDvbtSyncLocked(pDemod, &SyncTLockFlag));                //DVBT Sync locked
		ret |= DECODE_RET(pDemod->IsDvbt2SyncLocked(pDemod, &SyncT2LockFlag));
		ret |= DECODE_RET(pDemod->GetSignalParamsLockFlag(pDemod, &u8SignalLockedFlag));    //DVBT FEC locked
		ret |= DECODE_RET(pDemod->IsDvbt2L1Locked(pDemod, &T2L1LockFlag));

		if (FecLockFlag == 1) {
			lock = YES;
			break;
		}

		if ((T2L1LockFlag == 1) && (plpIntfFsmStuckCheckFlag == 0)) {
			//patch for Uganda
			plp_intf_fsm = ATB_DEMOD_CE_RO_PLP_INTF_FSM_2_0_get_plp_intf_fsm_2_0(rtd_inl(ATB_DEMOD_CE_RO_PLP_INTF_FSM_2_0_reg));
			if (plp_intf_fsm == 2)
				plpIntfFsmCnt ++;
			else
				plpIntfFsmCnt = 0;

			if (plpIntfFsmCnt > 2) {

				rtd_maskl(DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_DBG_0_reg, 0xfffffffe, 0x1);
				rtd_maskl(DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_DBG_2_reg, ~DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_DBG_2_atb_ldpc_rom_dma_dbg_data_sel_mask, 0x200);
				dbgData = (rtd_inl(DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_DBG_8_reg) >> 4) & 0x3;
				rtd_maskl(DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_DBG_0_reg, 0xfffffffe, 0x0);
				REALTEK_A_DVBTX_INFO("plpIntfFsmCnt = %u, dbgData = 0x%x !!\n", plpIntfFsmCnt, dbgData);

				if (dbgData == 0) {
					rtd_maskl(DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_1_reg, 0xFFFFEFFF, 0x0);
					RTK_A_DVBTX_Write(pDemod, 0x00, 0x05, 0x01);     /*Reset DSP*/
					RTK_A_DVBTX_Write(pDemod, 0x00, 0x05, 0x00);     /*Start DSP*/
					plpIntfFsmStuckCheckFlag = 1;
					acquire_sig_stime = tv_osal_time();
					det_stime = tv_osal_time();
					REALTEK_A_DVBTX_INFO("stuck in plpIntfFsm!! disable no fec decode reset~~\n");
				}
			}
		}


		if (SyncTLockFlag)
			SyncTLockFlagTmp++;
		if ((SyncTLockFlag == 0) && (SyncT2LockFlag == 0) && ((tv_osal_time() - acquire_sig_stime) > NoSignalTimeOut)) {
			lock = NO;

			if (SyncTLockFlagTmp > 5) {


				RTK_A_DVBTX_Read(pDemod, 0x17, 0xBB, &u8Read_data);
				RTK_A_DVBTX_Read(pDemod, 0x17, 0xD9, &u8Read_data1);
				REALTEK_A_DVBTX_DBG("[DVBT] 0x17BB = %x, 0x17D9 = %x !!\n", u8Read_data, u8Read_data1);
				u8Read_data &= 0xBF;
				u8Read_data1 &= 0xF7;
				RTK_A_DVBTX_Write(pDemod, 0x17, 0xBB, u8Read_data);    // dvbt sync lock, FEC unlock, turn off FEC reset
				RTK_A_DVBTX_Write(pDemod, 0x17, 0xD9, u8Read_data1);    //


				Sync1FecRstDisableFlag = 1;
				REALTEK_A_DVBTX_INFO("[DVBT] Sync1_Fec Rest Turn off !!\n");


				det_t_stime = tv_osal_time();
				while ((tv_osal_time() - det_t_stime) <= dvbt2_timeout) {
					tv_osal_msleep(5);
					ret |= DECODE_RET(pDemod->IsSignalLocked(pDemod, &FecLockFlag));
					//REALTEK_A_DVBTX_INFO("[DVBT] Sync0_FecRestOff_TPS = %d\n",FecLockFlag);
					//ret |= DECODE_RET(pDemod->IsDvbtSyncLocked(pDemod, &SyncTLockFlag));
					//REALTEK_A_DVBTX_INFO("[DVBT] Sync0_FecRestOff_Sync = %d\n",SyncTLockFlag);
					//ret |= DECODE_RET(pDemod->GetSignalParamsLockFlag(pDemod, &u8SignalLockedFlag));
					//REALTEK_A_DVBTX_INFO("[DVBT] Sync0_FecRestOff_PamsLockFlag = %d\n",u8SignalLockedFlag);

					if (FecLockFlag == 1) {
						lock = YES;
						break;
					}

				}
			}
			break;
		}



		if ((SyncTLockFlag == 1) && (u8SignalLockedFlag == 0) && ((tv_osal_time() - acquire_sig_stime) > dvbt_prelock_timeout)) {
			lock = NO;


			RTK_A_DVBTX_Read(pDemod, 0x17, 0xBB, &u8Read_data);
			RTK_A_DVBTX_Read(pDemod, 0x17, 0xD9, &u8Read_data1);
			REALTEK_A_DVBTX_DBG("[DVBT] 0x17BB = %x, 0x17D9 = %x !!\n", u8Read_data, u8Read_data1);
			u8Read_data &= 0xBF;
			u8Read_data1 &= 0xF7;
			RTK_A_DVBTX_Write(pDemod, 0x17, 0xBB, u8Read_data);    // dvbt sync lock, FEC unlock, turn off FEC reset
			RTK_A_DVBTX_Write(pDemod, 0x17, 0xD9, u8Read_data1);    //


			Sync1FecRstDisableFlag = 1;
			REALTEK_A_DVBTX_INFO("[DVBT] SyncTLockFlag = 1 Sync1_Fec Rest Turn off !!\n");


			syncT_PecCheck_Timeout = 3000;
			det_t_stime = tv_osal_time();
			while ((tv_osal_time() - det_t_stime) <= syncT_PecCheck_Timeout) {
				tv_osal_msleep(20);
				ret |= DECODE_RET(pDemod->IsSignalLocked(pDemod, &FecLockFlag));
				//REALTEK_A_DVBTX_INFO("[DVBT] Sync1_FecRestOff_TPS = %d\n",FecLockFlag);
				//ret |= DECODE_RET(pDemod->IsDvbtSyncLocked(pDemod, &SyncTLockFlag));
				//REALTEK_A_DVBTX_INFO("[DVBT] Sync1_FecRestOff_Sync = %d\n",SyncTLockFlag);
				//ret |= DECODE_RET(pDemod->GetSignalParamsLockFlag(pDemod, &u8SignalLockedFlag));
				//REALTEK_A_DVBTX_INFO("[DVBT] Sync1_FecRestOff_PamsLockFlag = %d\n",u8SignalLockedFlag);
				if (FecLockFlag == 1) {
					ret |= DECODE_RET(pDemod->GetPer(pDemod, &BadPakN, &PakN));
					REALTEK_A_DVBTX_INFO("[DVBT] FecLockFlag = 1, Sync1_FecRestOff BadPakN = %u, PakN = %u, time =%lu \n", BadPakN, PakN, tv_osal_time() - det_t_stime);
				}


				if ((FecLockFlag == 1) && (syncT_PecCheck_Timeout != 6000)) {
					syncT_PecCheck_Timeout = 6000;
					REALTEK_A_DVBTX_INFO("[DVBT] FecLockFlag = 1, Sync1_FecRestOff  set syncT_PecCheck_Timeout = 6000 !!!\n");
				}

				if ((FecLockFlag == 1) && (BadPakN < (PakN / 2))) {
					lock = YES;
					break;
				}
			}
			break;
		}

		if ((u8SignalLockedFlag == 0) && ((tv_osal_time() - acquire_sig_stime) > SignalLockedFlagTimeout)) {
			REALTEK_A_DVBTX_INFO("DVBT2 u8SignalLockedFlag = %d, SignalLockedFlagTimeout %u\n", u8SignalLockedFlag, SignalLockedFlagTimeout);
			lock = NO;
			break;
		}

		if ((SyncT2LockFlag != 1) && ((tv_osal_time() - acquire_sig_stime) > dvbt_timeout)) {
			lock = FecLockFlag ? YES : NO;
			break;
		}

		if ((SyncT2LockFlag == 1) && (T2L1LockFlag == 1) && ((tv_osal_time() - acquire_sig_stime) > dvbt2_L1LockCheck_timeout)) {
			ret |= DECODE_RET(pDemod->GetSnrDb(pDemod, &Snr));
			Snr = Snr * 1000; //(Snr/10)*10000
			if (Snr > Snr_thd) {
				REALTEK_A_DVBTX_INFO("SyncT2LockFlag = %d, T2L1LockFlag = %d, SNR = %u\n",
									 SyncT2LockFlag, T2L1LockFlag, Snr);
				lock = YES;
				break;
			}
		}

		tv_osal_msleep(5);
	}

	REALTEK_A_DVBTX_INFO("\033[1;32;32m" "Signal Lock = %u, API execution Time = %lu, total_acquire_time = %lu\n" "\033[m",
						 lock, tv_osal_time() - det_stime, tv_osal_time() - acquire_sig_stime);

	if ((lock == NO) && (Sync1FecRstDisableFlag == 1)) {                                     //Check signal parameter & Snr value

		RTK_A_DVBTX_Read(pDemod, 0x17, 0xBB, &u8Read_data);
		RTK_A_DVBTX_Read(pDemod, 0x17, 0xD9, &u8Read_data1);
		u8Read_data |= 0x40;
		u8Read_data1 |= 0x08;
		RTK_A_DVBTX_Write(pDemod, 0x17, 0xBB, u8Read_data);    //Set to default value
		RTK_A_DVBTX_Write(pDemod, 0x17, 0xD9, u8Read_data1);

		REALTEK_A_DVBTX_INFO("[DVBT] Set Default Value!!\n");
	}



	if ((lock == YES) && (Sync1FecRstDisableFlag == 1)) {                                     //Check signal parameter & Snr value
		Snr = 0;
		ret |= DECODE_RET(pDemod->GetSignalParamsT(pDemod, &DvbtParams));
		ret |= DECODE_RET(pDemod->GetSnrDb(pDemod, &Snr));
		Snr = Snr * 1000; //(Snr/10)*10000
		//REALTEK_A_DVBTX_DBG("[DVBT]Check SNR = %u\n",Snr);

		if ((pRealtekADVBTX->m_current_bw != RTK_A_DVBTX_BANDWIDTH_7MHZ) || (DvbtParams.DvbtHpCodeRate != RTK_A_DVBTX_DVBT_CODE_RATE_3_4) || (DvbtParams.DvbtConstellation != RTK_A_DVBTX_DVBT_CONSTELLATION_64QAM)\
			|| (DvbtParams.DvbtGuardInterval != RTK_A_DVBTX_DVBT_GI_1_16) || (DvbtParams.DvbtFFTMode != RTK_A_DVBTX_DVBT_MODE_8K) || (Snr < 220000)) {
			RTK_A_DVBTX_Read(pDemod, 0x17, 0xBB, &u8Read_data);
			RTK_A_DVBTX_Read(pDemod, 0x17, 0xD9, &u8Read_data1);
			u8Read_data |= 0x40;
			u8Read_data1 |= 0x08;
			RTK_A_DVBTX_Write(pDemod, 0x17, 0xBB, u8Read_data);    //Set to default value
			RTK_A_DVBTX_Write(pDemod, 0x17, 0xD9, u8Read_data1);

			REALTEK_A_DVBTX_INFO("[DVBT] Set Default Value!!\n");
		}
	}


	if (lock == YES) {

		if (SyncT2LockFlag == 1) {

			/*Get PLP Data and PLP Type*/
			ret |= DECODE_RET(pDemod->GetSignalParamsT2(pDemod, &DvbT2Params));

			if (DvbT2Params.u8DataPlpNumber == 1) {
				RTK_A_DVBTX_Write(pDemod, 0x17, 0x30, 0x01);
				REALTEK_A_DVBTX_INFO("[DVBT2] single PLP !!\n");
				gDvbT2DataPlpNumber = 1;
			} else {
				REALTEK_A_DVBTX_INFO("[DVBT2] multiple PLP !!\n");
				gDvbT2DataPlpNumber = DvbT2Params.u8DataPlpNumber;
			}

		}
		ret |= DECODE_RET(pDemod->GetDVBTxMode(pDemod, &StandardSearchResult));
		pRealtekADVBTX->m_dvbt2_dvbt_mode = StandardSearchResult;
		REALTEK_A_DVBTX_TPOCheck(pDemodKernel, StandardSearchResult);
		//DtvDemodTPOutEn(0);
		if (StandardSearchResult == RTK_A_DVBTX_DVBT_MODE) {
			REALTEK_A_DVBTX_INFO("[DVBT] calculate TPO !!\n");
			REALTEK_A_DVBTX_CalculateThroughput(pDemodKernel);
		} else {
			REALTEK_A_DVBTX_INFO("[DVBT2] Use Default TPO !!\n");
		}
	}

	pDemodKernel->Unlock(pDemodKernel);
	pRealtekADVBTX->m_acquire_sig_en = 0;
	pRealtekADVBTX->m_status_checking_stime = tv_osal_time() + DVBTX_STATUS_CHECK_INTERVAL_MS;

	return TUNER_CTRL_OK;
}
void REALTEK_A_DVBTX_TPOCheck(DEMOD* pDemodKernel, int isDVBT)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_PCMCIA)
	int isCIinsert = rtk_pcmcia_get_card_status_ext();
#else
	int isCIinsert = 1;
#endif

	if (isCIinsert == 1 && isDVBT == RTK_A_DVBTX_DVBT_MODE) {
		REALTEK_A_DVBTX_INFO("[DVBT] calculate TPO !!\n");
		//REALTEK_A_DVBTX_CalculateThroughput(pDemodKernel);
	} else if (isCIinsert == 1) {
		REALTEK_A_DVBTX_INFO("[DVBT2] calculate TPO !!\n");
		//REALTEK_A_DVBTX_TSBitRateCheck(pDemodKernel);
	} else {
		REALTEK_A_DVBTX_INFO("No CI card inserted!!\n");
		DtvDemodTPOutEn(1);
	}
}

/*----------------------------------------------------------------------
 * Func : ScanSignal
 *
 * Desc : Ask demodulator to Search signal within specified range
 *        This function should be called every time the channel changed.
 *
 * Parm : pOffset : Offset Output
 *        range   : Scan Range
 *          SCAN_RANGE_NONE : center only
 *          SCAN_RANGE_1_6  : (-)1/6 MHz
 *          SCAN_RANGE_2_6  : (-)2/6 MHz
 *          SCAN_RANGE_3_6  : (-)3/6 MHz
 *
 * Retn : TUNER_CTRL_OK        : signal found and locked
 *        TUNER_CTRL_FAIL      : error occured during scan channel
 *        TUNER_CTRL_NO_SIGNAL : no signal found in specified range
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_ScanSignal(DEMOD* pDemodKernel, S32BITS* pOffset, SCAN_RANGE range)
{
	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;
	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*) pRealtekADVBTX->m_private)->pDemod;

	//REALTEK_A_DVBTX_INFO("\033[1;32;31m" "%s %s %d\n" "\033[m", __FILE__, __func__, __LINE__);

	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : ForwardI2C
 *
 * Desc :
 *
 * Parm :
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_ForwardI2C(DEMOD* pDemodKernel, unsigned char on_off)
{
	int ret = TUNER_CTRL_OK;

	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;
	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*) pRealtekADVBTX->m_private)->pDemod;

	//REALTEK_A_DVBTX_INFO("\033[1;32;31m" "%s %s %d\n" "\033[m", __FILE__, __func__, __LINE__);


	return ret;
}


/*----------------------------------------------------------------------
 * Func : AutoTune
 *
 * Desc :
 *
 * Parm :
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_AutoTune(DEMOD* pDemodKernel)
{
	int ret = TUNER_CTRL_OK;

	return ret;
}


/*----------------------------------------------------------------------
 * Func : AutoTune
 *
 * Desc :
 *
 * Parm :
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_KernelAutoTune(DEMOD* pDemodKernel)
{
	unsigned long cur_time ;
	unsigned char Lock = 0;

	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;


	RTK_A_DVBTX_DVBT2_PARAMS DvbT2Params;
	unsigned char  u8PlpValid = 0, u8PlpManualEn = 0;

#ifndef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	unsigned char LockStatus = 0;
	TV_SIG_QUAL Quality;
#endif


	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*) pRealtekADVBTX->m_private)->pDemod;


	if (pRealtekADVBTX->m_auto_tune_en == 0)
		return TUNER_CTRL_OK;


	cur_time = tv_osal_time();


#ifndef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	if ((cur_time > pRealtekADVBTX->m_status_checking_stime) || ((pRealtekADVBTX->m_status_checking_stime - cur_time) > DVBTX_STATUS_CHECK_INTERVAL_MS)) {
		REALTEK_A_DVBTX_GetLockStatus(pDemodKernel, &LockStatus);
		REALTEK_A_DVBTX_INFO("[KernelAutoTune] Lock = %d\n", LockStatus);
		if (LockStatus == 1)
			REALTEK_A_DVBTX_GetSignalQuality(pDemodKernel, TV_QUAL_CUSTOMER_LG, &Quality);
		pRealtekADVBTX->m_status_checking_stime = tv_osal_time() + DVBTX_STATUS_CHECK_INTERVAL_MS;

	}
#endif


	if (pRealtekADVBTX->m_acquire_sig_en) {

#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
		REALTEK_A_DVBTX_AcquireSignalThread(pDemodKernel);
		pRealtekADVBTX->m_autotune_stime = tv_osal_time() + AUTODETECT_SIG_UPDATE_INTERVAL_MS;
		pRealtekADVBTX->m_autotune_TsTime = tv_osal_time() + AUTOTUNE_UPDATE_TS_CLK_INTERVAL_MS;
		pRealtekADVBTX->m_dvbt2_PLPcheck_stime = tv_osal_time() + AUTOTUNE_T2PLP_CHECK_INTERVAL_MS;
#endif

	} else {


		REALTEK_A_DVBTX_GetLockStatus(pDemodKernel, &Lock);

		pDemodKernel->Lock(pDemodKernel);
		if (Lock == 0) {
			pDemod->MonitorT2TxID(pDemod);

			if (((cur_time > pRealtekADVBTX->m_dvbt2_PLPcheck_stime) || ((pRealtekADVBTX->m_dvbt2_PLPcheck_stime - cur_time) > AUTOTUNE_T2PLP_CHECK_INTERVAL_MS)) && (pRealtekADVBTX->m_dvbt2_dvbt_mode == RTK_A_DVBTX_DVBT2_MODE) && (pDemodKernel->m_ScanMode == CH_NORMAL_MODE)) {
				RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1); //Latch on
				RTK_A_DVBTX_Read(pDemod, 0x24, 0x04,  &u8PlpValid);
				RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 0);//Latch off
				RTK_A_DVBTX_Read(pDemod, 0x24, 0x03,  &u8PlpManualEn);
				REALTEK_A_DVBTX_DBG("PLPValid = %u, u8PlpManualEn = %u \n", u8PlpValid, u8PlpManualEn);
				if ((u8PlpManualEn & 0x01) == 1) {
					pDemod->GetSignalParamsT2(pDemod, &DvbT2Params);

					REALTEK_A_DVBTX_DBG("PLP number = %u, m_plp_id = %u, PLP_ID= %u \n", DvbT2Params.u8DataPlpNumber, pRealtekADVBTX->m_plp_id, DvbT2Params.u8DataPlpIdArray[0]);
					if (gDvbT2DataPlpNumber == 1) {
						RTK_A_DVBTX_Write(pDemod, 0x24, 0x03, 0x00);     /*Disable PLP selection,*/
						RTK_A_DVBTX_Write(pDemod, 0x00, 0x05, 0x01);     /*Reset DSP*/
						RTK_A_DVBTX_Write(pDemod, 0x00, 0x05, 0x00);     /*Start DSP*/
						REALTEK_A_DVBTX_DBG("\033[1;32;31m" "change PLL ID selection mode from manual mode to auto mode!!\n" "\033[m");
					}

				}
				pRealtekADVBTX->m_dvbt2_PLPcheck_stime = tv_osal_time() + AUTOTUNE_T2PLP_CHECK_INTERVAL_MS;
			}



		} else {
			pDemod->MonitorT2DyStaticCH(pDemod);
			pDemod->MonitorT2CheckbyStaicChPwrChange(pDemod);   //add MonitorT2CheckbyStaicChPwrChange function
			//REALTEK_A_DVBTX_DBG("Check MonitorT2CheckbyStaicChPwrChange\n");
			if ((cur_time > pRealtekADVBTX->m_autotune_TsTime) || ((pRealtekADVBTX->m_autotune_TsTime - cur_time) > AUTOTUNE_UPDATE_TS_CLK_INTERVAL_MS)) {
				if (pRealtekADVBTX->m_dvbt2_dvbt_mode == RTK_A_DVBTX_DVBT2_MODE) {
					REALTEK_A_DVBTX_TSBitRateCheck(pDemodKernel);
				} else {
					REALTEK_A_DVBTX_CalculateThroughput(pDemodKernel);
				}
				pRealtekADVBTX->m_autotune_TsTime = tv_osal_time() + AUTOTUNE_UPDATE_TS_CLK_INTERVAL_MS;
			}


		}
		pDemod->GetSignalParamsT2(pDemod, &DvbT2Params);
		pDemodKernel->Unlock(pDemodKernel);

	}
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : InspectSignal
 *
 * Desc : Inspect Signal Info (for Debug Purpose)
 *
 * Parm : pBuff : message buffer
 *        BufferSize : size of message buffer
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_InspectSignal(DEMOD* pDemodKernel)
{
	int ret = TUNER_CTRL_OK;

	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;
	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*) pRealtekADVBTX->m_private)->pDemod;

	return ret;
}


/*----------------------------------------------------------------------
 * Func : Activate
 *
 * Desc : Activate RTK_DEMOD_DVBT2 Demod
 *
 * Parm : N/A
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_Activate(DEMOD* pDemodKernel, unsigned char force_rst)
{
	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;
	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*) pRealtekADVBTX->m_private)->pDemod;

	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : SetTvMode
 *
 * Desc : Set Mode of RTK_DEMOD_DVBT2
 *
 * Parm : mode :    RTK_DEMOD_MODE_DVBT2 : DVBT2 mode
 *                  RTK_DEMOD_MODE_DVBC  : DVBC mode
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_SetTvMode(DEMOD* pDemodKernel, TV_SYSTEM_TYPE mode)
{
	int ret = TUNER_CTRL_FAIL;

	REALTEK_A_DVBTX* pRealtekADVBTX;
	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);

	pDemodKernel->Lock(pDemodKernel);
	REALTEK_A_DVBTX_INFO("REALTEK_A_DVBTX::SetTvMode(%d)\n", mode);

	switch (mode) {
	case TV_SYS_TYPE_DVBT:
	case TV_SYS_TYPE_DVBT2:
		REALTEK_A_DVBTX_INFO("Set to DVBT2\n");
		ret = pDemodKernel->Init(pDemodKernel);
		if (ret != TUNER_CTRL_OK) {

			REALTEK_A_DVBTX_WARNING("Set REALTEK_A_DVBTX to DVBT2 mode failed\n");
			goto set_demod_mode_failed;
		}
		break;

	default:
		goto set_demod_mode_failed;
	}

	ret = TUNER_CTRL_OK;
	REALTEK_A_DVBTX_INFO("Set REALTEK_A_DVBTX to appointed DTV mode successed\n");

	pDemodKernel->Unlock(pDemodKernel);
	return ret;

set_demod_mode_failed:
	pDemodKernel->Unlock(pDemodKernel);

	return TUNER_CTRL_FAIL;
}

void REALTEK_A_DVBTX_Lock(DEMOD* pDemodKernel)
{
	//mutex_lock(&pDemodKernel->m_lock);
}

void REALTEK_A_DVBTX_Unlock(DEMOD* pDemodKernel)
{
	//mutex_unlock(&pDemodKernel->m_lock);
}


/*-----------------------------------------------------------------
 *     BASE_INTERFACE for REALTEK_A_DVBTX API
 *----------------------------------------------------------------*/
void ReleaseRealtekADvbtxDriver(REALTEK_A_DVBTX_DRIVER_DATA *pDriver)
{
	if (pDvbtxShareMemory) {
		DtvDemodFreeAllocMem((void *)pDvbtxShareMemory);
		pDvbtxShareMemory = NULL;
	}
	kfree(pDriver);
}


REALTEK_A_DVBTX_DRIVER_DATA* AllocRealtekADvbtxDriver(
	COMM*               pComm,
	unsigned char       Addr,
	U32BITS       CrystalFreq
)
{
	REALTEK_A_DVBTX_DRIVER_DATA* pDriver = (REALTEK_A_DVBTX_DRIVER_DATA*) kmalloc(
			sizeof(REALTEK_A_DVBTX_DRIVER_DATA) +
			sizeof(DVBTX_DEMOD_MODULE)   +
			sizeof(I2C_BRIDGE_MODULE)   +
			sizeof(BASE_INTERFACE_MODULE), GFP_KERNEL);

	if (pDriver) {
		BASE_INTERFACE_MODULE* pBIF;
		DVBTX_DEMOD_MODULE* pDemod;
		memset(pDriver, 0, sizeof(REALTEK_A_DVBTX_DRIVER_DATA));

		pDriver->pDemod                = (DVBTX_DEMOD_MODULE*)(((unsigned char*)pDriver) + sizeof(REALTEK_A_DVBTX_DRIVER_DATA));
		pDriver->pBaseInterface        = (BASE_INTERFACE_MODULE*)(((unsigned char*)pDriver->pDemod) + sizeof(DVBTX_DEMOD_MODULE));
		pDriver->pI2CBridge            = (I2C_BRIDGE_MODULE*)(((unsigned char*)pDriver->pBaseInterface) + sizeof(BASE_INTERFACE_MODULE));
		pDriver->DeviceAddr            = Addr;
		pDriver->CrystalFreqHz         = CrystalFreq;

		// Init Base IF
		pBIF = pDriver->pBaseInterface;

		pBIF->I2cReadingByteNumMax      = RTK_DEMOD_BIF_TX_FIFO_DEPTH;
		pBIF->I2cWritingByteNumMax      = RTK_DEMOD_BIF_RX_FIFO_DEPTH;
		pBIF->I2cRead                   = __realtek_a_rbus_i2c_read;
		pBIF->I2cWrite                  = __realtek_a_rbus_i2c_write;
		pBIF->WaitMs                    = __realtek_wait_ms;
		pBIF->SetUserDefinedDataPointer = base_interface_SetUserDefinedDataPointer;
		pBIF->GetUserDefinedDataPointer = base_interface_GetUserDefinedDataPointer;
		pBIF->SetUserDefinedDataPointer(pBIF, (void*)pComm);

		// Init Demod Driver
		BuildRtkaDvbtxModule(&pDemod,
							 pDriver->pDemod,
							 pDriver->pBaseInterface,
							 pDriver->pI2CBridge,
							 Addr,
							 (unsigned int) CrystalFreq);            // Spectrum mode is inverse.
	}

	return pDriver;
}


/*----------------------------------------------------------------------
 * Func : GetT2PLPInfo
 *
 * Desc : Get T2 PLP Information
 *
 * Parm : pInfo : PLP Information
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_GetT2PLPInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo)
{
	int ret = TUNER_CTRL_OK;
	unsigned char plp_index = 0;

	RTK_A_DVBTX_DVBT2_PARAMS DvbT2Params;
	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;

	if (pDemodKernel == NULL) {

		ret = TUNER_CTRL_FAIL;
		goto GetT2PLPInfo_End;
	}

	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*) pRealtekADVBTX->m_private)->pDemod;

	/*
		if (pRealtekADVBTX->m_dvbt2_dvbt_mode != RTK_A_DVBTX_DVBT2_MODE) {

			REALTEK_A_DVBTX_INFO("\033[1;32;32m" "REALTEK_ATB_DVBT2_GetT2PLPInfo Fail: The standard not DVBT2 now!!!!!!!!\n" "\033[m");
			//return TUNER_CTRL_OK;
		}
	*/

	pDemodKernel->Lock(pDemodKernel);
	/*Get PLP Data and PLP Type*/
	ret |= DECODE_RET(pDemod->GetSignalParamsT2(pDemod, &DvbT2Params));

	pDemodKernel->Unlock(pDemodKernel);

	if (ret != TUNER_CTRL_OK) {
		ret = TUNER_CTRL_FAIL;
		goto GetT2PLPInfo_End;
	}

	pInfo->dvbt2_ofdm.plp_cnt = DvbT2Params.u8DataPlpNumber;
	REALTEK_A_DVBTX_DBG("pInfo->dvbt2_ofdm.plp_cnt = %u\n", pInfo->dvbt2_ofdm.plp_cnt);


	for (plp_index = 0; plp_index < pInfo->dvbt2_ofdm.plp_cnt; plp_index++) {
		pInfo->dvbt2_ofdm.plp[plp_index] = DvbT2Params.u8DataPlpIdArray[plp_index];
		pInfo->dvbt2_ofdm.plp_type[plp_index] = (unsigned char)DvbT2Params.Dvbt2DataPLPType;
		REALTEK_A_DVBTX_DBG("PLP_ID[%u] = %u, PLP_Type[%u] = %u\n", plp_index, pInfo->dvbt2_ofdm.plp[plp_index], plp_index, pInfo->dvbt2_ofdm.plp_type[plp_index]);
	}

GetT2PLPInfo_End:
	return ret;
}

/*----------------------------------------------------------------------
 * Func : ChangeT2PLP
 *
 * Desc : Get T2 PLP Information
 *
 * Parm : pInfo : PLP Information
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBTX_ChangeT2PLP(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo)
{
	int ret = TUNER_CTRL_OK;
	int plp_selcet = 0;
	unsigned long stime = 0;
	unsigned int LockStatus = 0;

	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;

	if (pDemodKernel == NULL)
		return TUNER_CTRL_FAIL;

	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*) pRealtekADVBTX->m_private)->pDemod;

	/*
		if (pRealtekADVBTX->m_dvbt2_dvbt_mode != RTK_A_DVBTX_DVBT2_MODE) {

			REALTEK_A_DVBTX_INFO("\033[1;32;32m" "REALTEK_ATB_DVBT2_ChangeT2PLP Fail: The standard not DVBT2 now!!!!!!!!\n" "\033[m");
			//return TUNER_CTRL_OK;
		}
	*/

	pDemodKernel->Lock(pDemodKernel);
	plp_selcet = ((pInfo->dvbt2_ofdm.plp_select < 256) && (pInfo->dvbt2_ofdm.plp_select > 0)) ? pInfo->dvbt2_ofdm.plp_select : 0;

	ret |= DECODE_RET(pDemod->SetT2PlpID(pDemod, plp_selcet));
	REALTEK_A_DVBTX_INFO("[ChangeT2PLP]Not First PLP, plp_selcet = %d\n", plp_selcet);


	if (ret != TUNER_CTRL_OK)
		goto ChangeT2PLP_Fail;

	stime = tv_osal_time();

	while ((tv_osal_time() - stime) <= 1500) {
		tv_osal_msleep(5);
		ret |= DECODE_RET(pDemod->IsSignalLocked(pDemod, &LockStatus));

		if (LockStatus == 1 && ret == TUNER_CTRL_OK) {
			break;
		}

		if (stime > tv_osal_time())
			stime = 0;
	}

	if (LockStatus != 1 || ret != TUNER_CTRL_OK)
		goto ChangeT2PLP_Fail;

	pDemodKernel->Unlock(pDemodKernel);
	pRealtekADVBTX->m_plp_id = plp_selcet;
	REALTEK_A_DVBTX_INFO("[ChangeT2PLP]PLP ID = %d, Switch Time = %lu, LockStatus = %u\n", plp_selcet, tv_osal_time() - stime, LockStatus);

	return TUNER_CTRL_OK;

ChangeT2PLP_Fail:
	pDemodKernel->Unlock(pDemodKernel);
	REALTEK_A_DVBTX_INFO("[ChangeT2PLP]Fail!!!!!!!!\n");

	return ret;
}

int REALTEK_A_DVBTX_CalculateThroughput(DEMOD* pDemodKernel)
{
	int ret = TUNER_CTRL_OK;
	TV_SIG_INFO* pInfo;
	unsigned char mod_bits = 0;
	unsigned int bw_t = 0;
	unsigned int code_rate = 0;
	unsigned int TSThroughputBps = 0, TsClkKHz = TSCLK_DEFAULT;

	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;
	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*) pRealtekADVBTX->m_private)->pDemod;
	pInfo = kmalloc(sizeof(TV_SIG_INFO), GFP_KERNEL);
	if (pInfo == NULL) {
		REALTEK_A_DVBTX_INFO("REALTEK_A_DVBTX_CalculateThroughput kmalloc failed\n");
		return TUNER_CTRL_FAIL;
	}

	pDemodKernel->Lock(pDemodKernel);
	//pRealtekADVBTX->m_dvbt2_dvbt_mode = RTK_A_DVBTX_DVBT_MODE;
	ret = REALTEK_A_DVBTX_GetSignalInfo(pDemodKernel, pInfo);
	if (ret != TUNER_CTRL_OK)
		goto cal_throughput_done;

	switch (pInfo->dvbt2_ofdm.constellation) {
	case OFDM_CONST_4://QPSK
		mod_bits = 2;
		break;
	case OFDM_CONST_16://16QAM
		mod_bits = 4;
		break;
	case OFDM_CONST_64://64QAM
	default:
		mod_bits = 6;
		break;
	}
	switch (pInfo->dvbt2_ofdm.code_rate_hp) {
	case OFDM_CODE_RATE_1_2://Code Rate 1/2
		code_rate = 1 * 100 / 2;
		break;
	case OFDM_CODE_RATE_2_3://Code Rate 2/3
		code_rate = 2 * 100 / 3;
		break;
	case OFDM_CODE_RATE_3_4://Code Rate 3/4
		code_rate = 300 / 4;
		break;
	case OFDM_CODE_RATE_5_6://Code Rate 5/6
		code_rate = 5 * 100 / 6;
		break;
	case OFDM_CODE_RATE_7_8://Code Rate 7/8
	default:
		code_rate = 7 * 100 / 8;
		break;
	}
	switch (pInfo->dvbt2_ofdm.bandwidth) {
	case 6000://6M
		bw_t = 480 / 7;
		break;
	case 7000://7M
		bw_t = 560 / 7;
		break;
	case 8000://8M
	default:
		bw_t = 640 / 7;
		break;
	}
	//Throughput = 6044 * MOD_bits * code_rate * (188/204) / (8192 * (1+GI) * T)
	TSThroughputBps = 6044 * mod_bits * code_rate / 204 * 188 * bw_t  / (pInfo->dvbt2_ofdm.guard_interval + 1) * pInfo->dvbt2_ofdm.guard_interval  / 8192;

	if (TSThroughputBps != 0)
		TsClkKHz =  TSThroughputBps * 107 / 100 / 8;
	else
		TsClkKHz = TSCLK_DEFAULT;

	if (ABS(TsClkKHz, pRealtekADVBTX->m_ts_clk_KHz) > 50) {
		DemodTsOutClkCtrl(TsClkKHz);
		pRealtekADVBTX->m_ts_clk_KHz = TsClkKHz;
		REALTEK_A_DVBTX_INFO("\033[1;32;33m" "[TCalBitRate] throughput_bps = %u, TsClkKHz = %u\n" "\033[m", TSThroughputBps, TsClkKHz);
	}
	DtvDemodTPOutEn(1);

cal_throughput_done:
	kfree(pInfo);
	pDemodKernel->Unlock(pDemodKernel);
	return ret;

}

int REALTEK_A_DVBTX_TSBitRateCheck(DEMOD* pDemodKernel)
{

	int ret = TUNER_CTRL_OK;
	int LockStatus = 0;
	unsigned int TSThroughputBps = 0, TsClkKHz = TSCLK_DEFAULT;
	unsigned long stime = 0;

	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;
	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*) pRealtekADVBTX->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);

	ret |= DECODE_RET(pDemod->IsSignalLocked(pDemod, &LockStatus));

	if (LockStatus != 1)
		goto ts_clk_check_done;

	stime = tv_osal_time();
	while ((tv_osal_time() - stime) < 500) {
		ret |= DECODE_RET(pDemod->GetTsThroughput(pDemod, &TSThroughputBps));
		if (TSThroughputBps != 0)
			break;

		tv_osal_msleep(10);
	}

	if (ret != TUNER_CTRL_OK)
		goto ts_clk_check_done;

	if (TSThroughputBps != 0)
		TsClkKHz =  TSThroughputBps * 107 / 100 / 8;
	else
		TsClkKHz = TSCLK_DEFAULT;

	if ((TsClkKHz > 7812) && (TsClkKHz < 10417)) { //for 72Mbps issue, TPO from index 13:10417KHz ~18:8333KHz, all set index 16:9259Khz
		TsClkKHz = 9259;
	}

	if (ABS(TsClkKHz, pRealtekADVBTX->m_ts_clk_KHz) > 50) {
		DemodTsOutClkCtrl(TsClkKHz);
		pRealtekADVBTX->m_ts_clk_KHz = TsClkKHz;
		REALTEK_A_DVBTX_INFO("\033[1;32;33m" "[TSBitRateCheck] throughput_bps = %u, TsClkKHz = %u\n" "\033[m", TSThroughputBps, TsClkKHz);
	}
	DtvDemodTPOutEn(1);

ts_clk_check_done:

	pDemodKernel->Unlock(pDemodKernel);
	return ret;


}

int REALTEK_A_DVBTX_GetTsClkRate(DEMOD* pDemodKernel, unsigned int *pTsClkRate)
{
	int ret = TUNER_CTRL_OK;
	REALTEK_A_DVBTX* pRealtekADVBTX;
	DVBTX_DEMOD_MODULE* pDemod;
	pRealtekADVBTX = (REALTEK_A_DVBTX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBTX_DRIVER_DATA*) pRealtekADVBTX->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);
	*pTsClkRate = pRealtekADVBTX->m_ts_clk_KHz;
	REALTEK_A_DVBTX_DBG("\033[1;32;33m" "[REALTEK_A_DVBTX_GetTsClkRate] TsClkKHz = %u\n" "\033[m", *pTsClkRate);
	pDemodKernel->Unlock(pDemodKernel);

	return ret;
}
