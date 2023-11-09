/***************************************************************************************************
  File        : demod_rtk.cpp
  Description : REALTEK A DVB-S2 demod
  Author      : Timo.Wu
****************************************************************************************************
    Update List :
----------------------------------------------------------------------------------------------------
    20180315    | create phase
***************************************************************************************************/
#include <linux/slab.h>
#include <asm/cacheflush.h>
#include <linux/dma-mapping.h>
#include <linux/pageremap.h>
#include "comm.h"
#include "foundation.h"
#include "demod_rtk_a_dvbsx.h"
#include "demod_rtk_a_dvbsx_priv.h"
#include "dvbsx_demod_rtk_a.h"
#include <rtk_kdriver/io.h>
#include "rbus/dtv_demod_misc_reg.h"
#include "rbus/tv_sb1_ana_reg.h"

extern unsigned char DvbSxRomArray[];
extern unsigned long DvbSxRomArraySize;
unsigned char u8stLockCnt = 0;
static unsigned long *pDvbsxShareMemory = NULL;
extern unsigned long *pgShareMemory;
static unsigned long gShareMemoryPhysicalAddress = 0;
extern struct device *gDemodDev;

typedef struct _s_DvbS2SignalQualityBound
{
	unsigned int HighBound;
	unsigned int MidBound;
	unsigned int LowBound;
}s_DvbS2SignalQualityBound;

typedef struct _s_DvbS2SignalQualityMapping
{
	unsigned int Modulation;
	unsigned int CodeRate;
	s_DvbS2SignalQualityBound sBound;
}s_DvbS2SignalQualityMapping;

static const s_DvbS2SignalQualityMapping _gsDvbs2SigQualityMappingTable[] =
{
	{DVBS2_MOD_QPSK,  DVBS2_CR_1_2,    {80, 20, 12}},
	{DVBS2_MOD_QPSK,  DVBS2_CR_3_5,    {92, 32, 25}},
	{DVBS2_MOD_QPSK,  DVBS2_CR_2_3,    {101, 41, 32}},
	{DVBS2_MOD_QPSK,  DVBS2_CR_3_4,    {110, 50, 42}},
	{DVBS2_MOD_QPSK,  DVBS2_CR_4_5,    {117, 57, 49}},
	{DVBS2_MOD_QPSK,  DVBS2_CR_5_6,    {122, 62, 54}},
	{DVBS2_MOD_QPSK,  DVBS2_CR_8_9,    {132, 72, 64}},
	{DVBS2_MOD_QPSK,  DVBS2_CR_9_10,   {134, 74, 66}},
	{DVBS2_MOD_8PSK,  DVBS2_CR_3_5,    {125, 65, 60}},
	{DVBS2_MOD_8PSK,  DVBS2_CR_2_3,    {136, 76, 68}},
	{DVBS2_MOD_8PSK,  DVBS2_CR_3_4,    {149, 89, 82}},
	{DVBS2_MOD_8PSK,  DVBS2_CR_5_6,    {164, 104, 96}},
	{DVBS2_MOD_8PSK,  DVBS2_CR_8_9,    {177, 117, 110}},
	{DVBS2_MOD_8PSK,  DVBS2_CR_9_10,   {180, 120, 113}}
};
#define DVBS2_SIG_QUALITY_TABLE_SIZE sizeof(_gsDvbs2SigQualityMappingTable)/sizeof(s_DvbS2SignalQualityMapping)

typedef struct _s_DvbSSignalQualityBound
{
	unsigned int HighBound;
	unsigned int MidBound;
	unsigned int LowBound;
}s_DvbSSignalQualityBound;

typedef struct _s_DvbSSignalQualityMapping
{
	unsigned int CodeRate;
	s_DvbSSignalQualityBound sBound;
}s_DvbSSignalQualityMapping;

static const s_DvbSSignalQualityMapping _gsDvbSSigQualityMappingTable[] =
{
	{DVBS_CR_1_2,    {88, 38, 25}},
	{DVBS_CR_2_3,    {106, 56, 42}},
	{DVBS_CR_3_4,    {117, 67, 51}},
	{DVBS_CR_5_6,    {127, 77, 60}},
	{DVBS_CR_7_8,    {134, 84, 67}}
};
#define DVBS_SIG_QUALITY_TABLE_SIZE sizeof(_gsDvbSSigQualityMappingTable)/sizeof(s_DvbSSignalQualityMapping)

#ifndef CONFIG_CUSTOMER_TV006
extern unsigned int DvbS2_Keep_22k_on_off;
#endif
unsigned long acquire_sig_stime_s = 0;
extern struct LiteSymbolRateSignalListStruct gLiteymbolRateSignalList;
/*----------------------------------------------------------------------
 * Func : REALTEK_A_DVBSX
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
void REALTEK_A_DVBSX_Constructors(DEMOD* pDemodKernel, unsigned char addr, unsigned char output_mode, unsigned char output_freq, COMM* pComm)
{
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	
	pDemodKernel->private_data = dvr_malloc(sizeof(REALTEK_A_DVBSX));
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	Demod_Constructors(pDemodKernel);
	
	pDemodKernel->m_id = DTV_DEMOD_REALTEK_A_DVBSX;
	pDemodKernel->m_addr = addr;
	pDemodKernel->m_pTuner = NULL;
	pDemodKernel->m_pComm = pComm;
	pDemodKernel->m_ScanRange = SCAN_RANGE_3_6;
	pDemodKernel->m_Capability = TV_SYS_TYPE_DVBS2;
	pDemodKernel->m_update_interval_ms = 1000;
	pDemodKernel->m_if.freq = 0;
	pDemodKernel->m_if.inversion = 0;
	pDemodKernel->m_if.agc_pola = 0;
	pDemodKernel->m_clock = CRYSTAL_FREQ_27000000HZ;
	
	pRealtekIDVBSX->m_private = (void*)AllocRealtekIDvbSxDriver(pDemodKernel->m_pComm, pDemodKernel->m_addr, pDemodKernel->m_clock);
	
	pDemodKernel->m_ts.mode = (output_mode == RTK_DEMOD_OUT_IF_PARALLEL) ? PARALLEL_MODE : SERIAL_MODE;
	pDemodKernel->m_ts.data_order = MSB_D7;
	pDemodKernel->m_ts.datapin = MSB_FIRST;
	pDemodKernel->m_ts.err_pol = ACTIVE_HIGH;
	pDemodKernel->m_ts.sync_pol = ACTIVE_HIGH;
	pDemodKernel->m_ts.val_pol = ACTIVE_HIGH;
	pDemodKernel->m_ts.clk_pol = FALLING_EDGE;
	pDemodKernel->m_ts.internal_demod_input = 1;
	pDemodKernel->m_ts.tsOutEn = 1;
	pRealtekIDVBSX->m_auto_tune_en = 0;
	pRealtekIDVBSX->m_acquire_sig_en = 0;
	pRealtekIDVBSX->m_ts_clk_khz = 0;
	pRealtekIDVBSX->m_bs_init_done = 0;
	pRealtekIDVBSX->m_dis_dvb2x_rolloff_mode = 0;
	pRealtekIDVBSX->m_lock_pivot = 0;
	
	pDemodKernel->m_pComm->SetTargetName(pDemodKernel->m_pComm, pDemodKernel->m_addr, "REALTEK_A_DVBSX");
	
	pDemodKernel->Init = REALTEK_A_DVBSX_Init;
	pDemodKernel->Reset = Demod_Reset;
	pDemodKernel->AcquireSignal = REALTEK_A_DVBSX_AcquireSignal;
	pDemodKernel->ScanSignal = REALTEK_A_DVBSX_ScanSignal;
	pDemodKernel->SetIF = REALTEK_A_DVBSX_SetIF;
	pDemodKernel->SetTSMode = REALTEK_A_DVBSX_SetTSMode;
	pDemodKernel->SetTVSysEx = REALTEK_A_DVBSX_SetTVSysEx;
	pDemodKernel->SetMode = REALTEK_A_DVBSX_SetMode;
	pDemodKernel->SetTVSys = REALTEK_A_DVBSX_SetTVSys;
	pDemodKernel->GetLockStatus = REALTEK_A_DVBSX_GetLockStatus;
	pDemodKernel->GetSegmentNum = Demod_GetSegmentNum;
	pDemodKernel->GetSignalInfo = REALTEK_A_DVBSX_GetSignalInfo;
	pDemodKernel->GetSignalQuality = REALTEK_A_DVBSX_GetSignalQuality;
	pDemodKernel->ChangeT2PLP = Demod_ChangeT2PLP;
	pDemodKernel->GetT2PLPInfo = Demod_GetT2PLPInfo;
	pDemodKernel->GetT2L1PreInfo = Demod_GetT2L1PreInfo;
	pDemodKernel->GetT2L1PostInfo = Demod_GetT2L1PostInfo;
	pDemodKernel->GetDemodStatus = Demod_GetDemodStatus;
	pDemodKernel->GetCarrierOffset = REALTEK_A_DVBSX_GetCarrierOffset;
	pDemodKernel->GetChannelInfo = REALTEK_A_DVBSX_GetChannelInfo;
	pDemodKernel->ForwardI2C = REALTEK_A_DVBSX_ForwardI2C;
	pDemodKernel->AutoTune = REALTEK_A_DVBSX_AutoTune;
	pDemodKernel->KernelAutoTune = REALTEK_A_DVBSX_KernelAutoTune;
	pDemodKernel->GetCapability = Demod_GetCapability;
	pDemodKernel->Suspend = Demod_Suspend;
	pDemodKernel->Resume = Demod_Resume;
	pDemodKernel->SetTvMode = REALTEK_A_DVBSX_SetTvMode;
	pDemodKernel->InspectSignal = REALTEK_A_DVBSX_InspectSignal; // Add monitor function that used to monitor demod status
	pDemodKernel->AttachTuner = Demod_AttachTuner;
	pDemodKernel->GetTSParam = Demod_GetTSParam;
	
	pDemodKernel->BlindScan_Init = REALTEK_A_DVBSX_BlindScan_Init;
	pDemodKernel->BlindScan_GetTunerFreq = REALTEK_A_DVBSX_BlindScan_GetTunerFreq;
	pDemodKernel->BlindScan_NextFreq = REALTEK_A_DVBSX_BlindScan_NextFreq;
	pDemodKernel->BlindScan_GetStatus = REALTEK_A_DVBSX_BlindScan_GetStatus;
	pDemodKernel->BlindScan_End = REALTEK_A_DVBSX_BlindScan_End;
	pDemodKernel->BlindScan_GetFoundTP = REALTEK_A_DVBSX_BlindScan_GetFoundTP;
	pDemodKernel->BlindScan_AcquireSignal = REALTEK_A_DVBSX_BlindScan_AcquireSignal;
	
	pDemodKernel->SetDiseqcContinue22kOnOff = REALTEK_A_DVBSX_SetDiseqcContinue22kOnOff;
	pDemodKernel->SetDiseqcCmd = REALTEK_A_DVBSX_SetDiseqcCmd;
	pDemodKernel->SetDiseqcToneBurst = REALTEK_A_DVBSX_SetDiseqcToneBurst;
	pDemodKernel->SetDiseqcUnModToneBurst = REALTEK_A_DVBSX_SetDiseqcUnModToneBurst;
	pDemodKernel->SetDiseqcModToneBurst = REALTEK_A_DVBSX_SetDiseqcModToneBurst;
	
	pDemodKernel->Lock = REALTEK_A_DVBSX_Lock;
	pDemodKernel->Unlock = REALTEK_A_DVBSX_Unlock;
	
	//pDemodKernel->Activate = REALTEK_A_DVBSX_Activate;
	pDemodKernel->Destory = REALTEK_A_DVBSX_Destructors;
	
	pRealtekIDVBSX->m_info.dvbs.IsBlindsearch = 0;
}


/*----------------------------------------------------------------------
 * Func : ~REALTEK_A_DVBSX
 *
 * Desc : Destructor of REALTEK_A_DVBSX
 *
 * Parm : pDemod : handle of Demod
 *
 * Retn : N/A
 *----------------------------------------------------------------------*/
void REALTEK_A_DVBSX_Destructors(DEMOD* pDemodKernel)
{
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	
	if(pRealtekIDVBSX->m_private != NULL)
		ReleaseRealtekIDvbs2Driver((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private);
	
	dvr_free(pRealtekIDVBSX);
	//pDemodKernel->Destory(pDemodKernel);
	REALTEK_A_DVBSX_INFO("REALTEK_A_DVBSX_Destructors Complete\n");
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
int REALTEK_A_DVBSX_Init(DEMOD* pDemodKernel)
{
	int ret = TUNER_CTRL_OK;
	unsigned int AllocateSize = 0;
	unsigned long ShareMemoryPhysicalAddress, DvbsxRomPhysicalAddrStart, DvbsxRomVirAddrStart, rst;
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);

	if(pDemodKernel->m_pComm == NULL || pRealtekIDVBSX->m_private == NULL)
		return TUNER_CTRL_FAIL;

	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);

	pRealtekIDVBSX->m_auto_tune_en = 0;
	pRealtekIDVBSX->m_ts_clk_khz = 0;

	TvFrontendPowerControl(0);                    //Turn off TVALL
	if(pDvbsxShareMemory == NULL) {
		ret = DtvDemodMemRemappingInt((RTK_A_DVBSX_DDR_SIZE + DvbSxRomArraySize), &ShareMemoryPhysicalAddress, &AllocateSize);
		if(ret != TUNER_CTRL_OK){
			REALTEK_A_DVBSX_WARNING("allocate share memory failed\n");
			return TUNER_CTRL_FAIL;
		}
		pDvbsxShareMemory = pgShareMemory;
		gShareMemoryPhysicalAddress = ShareMemoryPhysicalAddress;
		DvbsxRomPhysicalAddrStart = ShareMemoryPhysicalAddress + RTK_A_DVBSX_DDR_SIZE;
		DvbsxRomVirAddrStart = (unsigned long)phys_to_virt(DvbsxRomPhysicalAddrStart);
		REALTEK_A_DVBSX_INFO("allocate share memory(dvbsx) - Phy_addr=0x%08lx, Vir_addr=0x%08lx,size=0x%08x\n", ShareMemoryPhysicalAddress, DvbsxRomVirAddrStart, AllocateSize);

		//CopyROM data into DRAM
		memcpy((void *)DvbsxRomVirAddrStart, DvbSxRomArray, DvbSxRomArraySize);
		if(gDemodDev != NULL){
			rst = (unsigned long) dma_map_single(gDemodDev, (void *)DvbsxRomVirAddrStart, DvbSxRomArraySize, DMA_BIDIRECTIONAL);
			REALTEK_A_DVBSX_INFO("Load Dvbsx ROM data to DDR  Rst=0x%08lx\n", rst); 
		} else {
			REALTEK_A_DVBSX_INFO("CopyROM data into DRAM fail!!!\n");
			ret |= TUNER_CTRL_FAIL;
		}
	}

	//SOC init
	DtvDemodInitialization(RTK_DEMOD_MODE_DVBS, 1);

	//DvbSx demod MISC setting
	rtd_maskl(DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_3_reg, ~DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_3_atb_ldpc_rom_dma_cfg_addr_mask, DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_3_atb_ldpc_rom_dma_cfg_addr(0x70));
	rtd_outl(DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_8_reg, gShareMemoryPhysicalAddress + RTK_A_DVBSX_DDR_SIZE);
	rtd_maskl(DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_2_reg, ~DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_2_atb_ldpc_rom_dma_cfg_wr_mask, DTV_DEMOD_MISC_ATB_LDPC_ROM_DMA_2_atb_ldpc_rom_dma_cfg_wr(0x1));
	rtd_maskl(DTV_DEMOD_MISC_DTV_DEMOD_SEL_reg, ~(DTV_DEMOD_MISC_DTV_DEMOD_SEL_atb_dvbt_en_mask|DTV_DEMOD_MISC_DTV_DEMOD_SEL_atb_dvbs_en_mask|DTV_DEMOD_MISC_DTV_DEMOD_SEL_atb_dvbc_en_mask|DTV_DEMOD_MISC_DTV_DEMOD_SEL_atb_dvbc2_en_mask), DTV_DEMOD_MISC_DTV_DEMOD_SEL_atb_dvbs_en(0x1)); //DTV Dvbs enable<bit1> =0x1(2)

	if (pDemodKernel->m_pTuner) {
		TUNER_PARAM Param;
		if (pDemodKernel->m_pTuner->GetParam(pDemodKernel->m_pTuner, TV_SYS_DVBS2, &Param) == TUNER_CTRL_OK) {
			switch (Param.id) {
			case TV_TUNER_AV2018:
				pDemod->CurrentTunerId = TV_TUNER_AV2018;
				REALTEK_A_DVBSX_INFO("Tuner id %u, TV_TUNER_AV2018\n", Param.id);
				break;
			case TV_TUNER_RT710:
				pDemod->CurrentTunerId = TV_TUNER_RT710;
				rtd_maskl(TV_SB1_ANA_IQADC_CTRL5_reg, ~(TV_SB1_ANA_IQADC_CTRL5_reg_ipga_d_mask|TV_SB1_ANA_IQADC_CTRL5_reg_qpga_d_mask), (TV_SB1_ANA_IQADC_CTRL5_reg_ipga_d(0x2)|TV_SB1_ANA_IQADC_CTRL5_reg_qpga_d(0x2)));//PGA +2dB for RT710
				REALTEK_A_DVBSX_INFO("Tuner id %u, TV_TUNER_RT710\n", Param.id);
				break;
			case TV_TUNER_RDA5815M:
				pDemod->CurrentTunerId = TV_TUNER_RDA5815M;
				REALTEK_A_DVBSX_INFO("Tuner id %u, TV_TUNER_RDA5815M\n", Param.id);
				break;
			default:
				pDemod->CurrentTunerId = TV_TUNER_AV2018;
				REALTEK_A_DVBSX_INFO("Unkonw tuner id %u, using default TV_TUNER_AV2018 setting\n", Param.id);
				break;
			}
		}
	}

	RTK_A_DVBSX_PowerOnInit(pDemod);

	#ifndef CONFIG_CUSTOMER_TV006
	pDemod->SetDiseqcContinue22kOnOff(DvbS2_Keep_22k_on_off);
	#endif

	if (pDemodKernel->SetTSMode(pDemodKernel, &pDemodKernel->m_ts) != TUNER_CTRL_OK) {
		ret |= TUNER_CTRL_FAIL;
	}

	ClearDemodReset(RTK_DEMOD_MODE_DVBS);

	pDemodKernel->Unlock(pDemodKernel);

	return ret;
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
int REALTEK_A_DVBSX_SetTVSys(DEMOD* pDemodKernel, TV_SYSTEM sys)
{
	int ret = TUNER_CTRL_OK;
	TV_SIG_INFO *pSigInfo;
	pSigInfo = dvr_malloc(sizeof(TV_SIG_INFO));
	
	pSigInfo->mod = TV_MODULATION_DVBS2;
	pSigInfo->dvbs.mod = TV_MODULATION_DVBS2;
	pSigInfo->dvbs.symbol_rate = 27500000;
	pSigInfo->dvbs.lnb_mode = LNB_MODE_13V_WITH_22K_BURST;
	
	ret = pDemodKernel->SetTVSysEx(pDemodKernel, sys, pSigInfo);
	
	dvr_free(pSigInfo);
	
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
int REALTEK_A_DVBSX_SetTVSysEx(DEMOD* pDemodKernel, TV_SYSTEM sys, const TV_SIG_INFO* pSigInfo)
{
	int ret = TUNER_CTRL_OK;
	#ifdef DYNAMIC_TPO_CLK
	unsigned int TsClkKHz = 12000;
	#endif
	TUNER_PARAM Param;
	
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	pRealtekIDVBSX->m_auto_tune_en = 0;
	pRealtekIDVBSX->m_dis_dvb2x_rolloff_mode = 0;
	
	if(!pSigInfo){
		REALTEK_A_DVBSX_INFO("Siginfo doesn't specified, SetTVSysEx Fail\n");
	
		return TUNER_CTRL_FAIL;
	}
	
	if(pSigInfo->dvbs.symbol_rate == 0 || pSigInfo->dvbs.symbol_rate > 50000000)
	{
		REALTEK_A_DVBSX_INFO("Null Symbol Rate, SetTVSysEx Break\n");
		return TUNER_CTRL_OK;
	}
	
	pDemodKernel->Lock(pDemodKernel);
	
	//TUNER_PARAM Param;
	if(pDemodKernel->m_pTuner->GetParam(pDemodKernel->m_pTuner, sys, &Param) == TUNER_CTRL_OK){
		if((pDemodKernel->m_if.freq != Param.if_freq) || (pDemodKernel->m_if.inversion != Param.if_inversion) || (pDemodKernel->m_if.agc_pola != Param.if_agc_pola)){
			pDemodKernel->m_if.freq = Param.if_freq;
			pDemodKernel->m_if.inversion = Param.if_inversion;
			pDemodKernel->m_if.agc_pola = Param.if_agc_pola;
			if(pDemodKernel->SetIF(pDemodKernel, &pDemodKernel->m_if) != TUNER_CTRL_OK) return TUNER_CTRL_FAIL;
	
		}
	}
	
	pDemodKernel->Unlock(pDemodKernel);
	
	pRealtekIDVBSX->m_info.dvbs.freq_khz = pSigInfo->dvbs.freq_khz;
	pRealtekIDVBSX->m_info.dvbs.symbol_rate = pSigInfo->dvbs.symbol_rate;
	pRealtekIDVBSX->m_info.dvbs.IsBlindsearch = pSigInfo->dvbs.IsBlindsearch;
	pRealtekIDVBSX->m_info.dvbs.scan_mode = pSigInfo->dvbs.scan_mode;
	
	pDemodKernel->m_tv_sys = sys;
	
	RTK_A_DVBSX_SetDvbSxMode(pDemod, 0, (unsigned int)pSigInfo->dvbs.symbol_rate, pSigInfo->dvbs.freq_khz); //Default Setting
	
	if(pDemodKernel->m_tv_sys == TV_SYS_DVBS)
	{
		REALTEK_A_DVBSX_INFO("\033[1;32;33m" "[TV_SYS_DVBS] dvbs.freq_khz = "PT_U32BITS", dvbs.symbol_rate = "PT_U32BITS", BS_mode = %u, Scan_mode = %u\n" "\033[m",
			pRealtekIDVBSX->m_info.dvbs.freq_khz, pRealtekIDVBSX->m_info.dvbs.symbol_rate, (unsigned int)pRealtekIDVBSX->m_info.dvbs.IsBlindsearch, (unsigned int)pRealtekIDVBSX->m_info.dvbs.scan_mode);
	}
	else if(pDemodKernel->m_tv_sys == TV_SYS_DVBS2)
	{
		REALTEK_A_DVBSX_INFO("\033[1;32;33m" "[TV_SYS_DVBS2] dvbs.freq_khz = "PT_U32BITS", dvbs.symbol_rate = "PT_U32BITS", BS_mode = %u, Scan_mode = %u\n" "\033[m",
			pRealtekIDVBSX->m_info.dvbs.freq_khz, pRealtekIDVBSX->m_info.dvbs.symbol_rate, (unsigned int)pRealtekIDVBSX->m_info.dvbs.IsBlindsearch, (unsigned int)pRealtekIDVBSX->m_info.dvbs.scan_mode);
	}
	
	#ifdef DYNAMIC_TPO_CLK
	//Set DVBS2 8PSK with Setting Symbol Rate for Default TPO Clock
	TsClkKHz = ((pRealtekIDVBSX->m_info.dvbs.symbol_rate/1000) * 27 / 80) * 107 / 100;//(pRealtekIDVBSX->m_info.dvbs.symbol_rate/1000) * 3 *9 / 10 / 8 * 107 /100
	REALTEK_A_DVBSX_INFO("\033[1;32;31m" "Set Default TPO Clock %u KHz\n" "\033[m", TsClkKHz);
	DemodTsOutClkCtrl(TsClkKHz);
	pRealtekIDVBSX->m_ts_clk_khz = TsClkKHz;
	#endif

	pRealtekIDVBSX->m_acquire_sig_en = 1;
	acquire_sig_stime_s = tv_osal_time();
	
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
int REALTEK_A_DVBSX_SetIF(DEMOD* pDemodKernel, IF_PARAM* pParam)
{
	int ret = TUNER_CTRL_OK;
	
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	pDemodKernel->Lock(pDemodKernel);
	
	REALTEK_A_DVBSX_DBG("SetIF, IF_freq="PT_U32BITS" spectrumn inv=%d, agc_pola=%d\n", pParam->freq, pParam->inversion, pParam->agc_pola);
	
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
int REALTEK_A_DVBSX_SetMode(DEMOD* pDemodKernel, TV_OP_MODE mode)
{
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	
	switch (mode){
	case TV_OP_MODE_NORMAL:
	case TV_OP_MODE_SCANING:
		break;
	
	case TV_OP_MODE_STANDBY:
		break;
	
	case TV_OP_MODE_ATV_NORMAL:
	case TV_OP_MODE_TP_ONLY:
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
int REALTEK_A_DVBSX_SetTSMode(DEMOD* pDemodKernel, TS_PARAM* pParam)
{
	int ret = TUNER_CTRL_OK;
	unsigned char TsOutEn;
	RTK_DEMOD_MPEGTSMode stTSMode;

	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;

	stTSMode.ui8TSTransferType = (pParam->mode) ? RTK_DEMOD_TS_SERIAL_MODE : RTK_DEMOD_TS_PARALLEL_MODE;
	stTSMode.ui8OutputEdge = (pParam->clk_pol) ? RTK_DEMOD_TS_OUTPUT_RISING_EDGE : RTK_DEMOD_TS_OUTPUT_FALLING_EDGE;
	stTSMode.ui8TSSPIMSBSelection = (pParam->data_order) ? RTK_DEMOD_TS_SPI_MSB_ON_DATA_BIT0 : RTK_DEMOD_TS_SPI_MSB_ON_DATA_BIT7;
	stTSMode.ui8TSSSIOutputSelection = RTK_DEMOD_TS_SSI_OUTPUT_ON_DATA_BIT0;
	stTSMode.ui8SPIClockConstantOutput = RTK_DEMOD_TS_CLOCK_CONST_OUTPUT;
	stTSMode.ui8DVBSxTSClockVariableMode  = RTK_A_DEMOD_TS_CLOCK_VARIABLE;

	TsOutEn = (pParam->tsOutEn == 1) ? RTK_DEMOD_TS_ON : RTK_DEMOD_TS_OFF;

	ret |= RTK_A_DVBSX_SetTsInterfaceMode(pDemod, stTSMode);

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
int REALTEK_A_DVBSX_GetLockStatus(DEMOD* pDemodKernel, unsigned char* pLock)
{
	unsigned char lock = 0, timing_lock = 0, ts_lock = 0, fec_lock = 0, pBwFlag = 0;
	unsigned char lock_cnt = 3;
	int ret = TUNER_CTRL_OK, Snr = 0;
	//unsigned long stime = 0;
	
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	if(!DtvIsDemodClockEnable(RTK_DEMOD_MODE_DVBS2))
		return TUNER_CTRL_FAIL;
	
	pDemodKernel->Lock(pDemodKernel);
	
	//stime = tv_osal_time();
	
	while(lock_cnt > 0)
	{
		ret = RTK_A_DVBSX_LatchOn(pDemod);
		ret = RTK_A_DVBSX_TimingLockFlag(pDemod, &timing_lock);
		ret = RTK_A_DVBSX_FECLockFlag(pDemod, &fec_lock);
		ret = RTK_A_DVBSX_TSLockFlag(pDemod, &ts_lock);
		ret = RTK_A_DVBSX_Read(pDemod, 0x2c, 0x59, &pBwFlag);
		ret = RTK_A_DVBSX_LatchOff(pDemod);
		if(pRealtekIDVBSX->m_info.dvbs.symbol_rate > RTK_DVBSX_LITE_SYMBOL_HZ)
		{
			REALTEK_A_DVBSX_INFO("\033[1;32;36m" "Timing Lock = %u, FEC Lock = %u, TS Lock = %u (%u)\n" "\033[m", timing_lock, fec_lock, ts_lock, pBwFlag);
			if(fec_lock == 1 || ts_lock == 1)
			{
				lock = 1;
				pRealtekIDVBSX->m_lock_pivot = 3;
				//while((tv_osal_time() - stime) < 10)
				//{
				//	tv_osal_msleep(3);
				//}
				break;
			}
			if(timing_lock == 1)
			{
				if(pRealtekIDVBSX->m_lock_pivot > 0)
				{
					lock = 1;
					pRealtekIDVBSX->m_lock_pivot--;
				}
				else
				{
					lock = 0;
					pRealtekIDVBSX->m_lock_pivot = 0;
				}
				//while((tv_osal_time() - stime) < 10)
				//{
				//	tv_osal_msleep(3);
				//}
				break;
			}
		}
		else
		{
			REALTEK_A_DVBSX_INFO("\033[1;32;36m" "Timing Lock = %u, TS Lock = %u (%u)\n" "\033[m", timing_lock, ts_lock, pBwFlag);
			if(timing_lock == 1 || ts_lock == 1)
			{
				lock = 1;
				//while((tv_osal_time() - stime) < 10)
				//{
				//	tv_osal_msleep(3);
				//}
				break;
			}
		}
		lock_cnt--;
		tv_osal_msleep(4);
	}
	
	if(pRealtekIDVBSX->m_dis_dvb2x_rolloff_mode == 1)
		lock = 0;
	
	lock = (lock == 1) ? YES : NO;
	
	if(ret == TUNER_CTRL_OK)
		*pLock = DECODE_LOCK(lock);
	else
		*pLock = 0;
	
	ret = RTK_A_DVBSX_GetDVBSxSNR(pDemod, &Snr);
	REALTEK_A_DVBSX_INFO("\033[1;32;36m" "SNR = %d\n" "\033[m", Snr);
	
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
int REALTEK_A_DVBSX_GetSignalInfo(DEMOD* pDemodKernel, TV_SIG_INFO* pInfo)
{
	RTK_DVBSx_SIGNAL_PARAMS pDVBSxParams;
	RTK_DVBSx_SIGNAL_SPECTRUM pSignalSpectrum;
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	int ret = TUNER_CTRL_OK;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	pDemodKernel->Lock(pDemodKernel);
	
	RTK_A_DVBSX_GetDvBSxSpectrum(pDemod, &pSignalSpectrum);
	RTK_A_DVBSX_GetDVBSxSignalParams(pDemod, &pDVBSxParams);
	
	pDemodKernel->Unlock(pDemodKernel);
	
	pInfo->dvbs.spec_invert = pSignalSpectrum;
	pInfo->dvbs.symbol_rate = pRealtekIDVBSX->m_info.dvbs.symbol_rate/1000;
	
	if(pDVBSxParams.DVBSxType == RTK_DEMOD_DVBS2_MODE)
	{
		pInfo->dvbs.mod = TV_MODULATION_DVBS2; //bIsDVBS2
		pInfo->mod = TV_MODULATION_DVBS2; //bIsDVBS2
	
		switch (pDVBSxParams.DVBS2CodeRate)
		{
			case RTK_DVBS2_CODE_RATE_1_2://Code Rate 1/2
				pInfo->dvbs.code_rate = DVBS2_CR_1_2;
				break;
			case RTK_DVBS2_CODE_RATE_2_3://Code Rate 2/3
				pInfo->dvbs.code_rate = DVBS2_CR_2_3;
				break;
			case RTK_DVBS2_CODE_RATE_3_4://Code Rate 3/4
				pInfo->dvbs.code_rate = DVBS2_CR_3_4;
				break;
			case RTK_DVBS2_CODE_RATE_5_6://Code Rate 5/6
				pInfo->dvbs.code_rate = DVBS2_CR_5_6;
				break;
			case RTK_DVBS2_CODE_RATE_3_5://Code Rate 3/5
				pInfo->dvbs.code_rate = DVBS2_CR_3_5;
				break;
			case RTK_DVBS2_CODE_RATE_4_5://Code Rate 4/5
				pInfo->dvbs.code_rate = DVBS2_CR_4_5;
				break;
			case RTK_DVBS2_CODE_RATE_8_9://Code Rate 8/9
				pInfo->dvbs.code_rate = DVBS2_CR_8_9;
				break;
			case RTK_DVBS2_CODE_RATE_9_10://Code Rate 9/10
				pInfo->dvbs.code_rate = DVBS2_CR_9_10;
				break;
			case RTK_DVBS2_CODE_RATE_1_4:
			case RTK_DVBS2_CODE_RATE_1_3:
			case RTK_DVBS2_CODE_RATE_2_5:
			case RTK_DVBS2_CODE_RATE_DUMMY:
			case RTK_DVBS2_CODE_RATE_UNKNOWN:
			default:
				pInfo->dvbs.code_rate = DVBS2_CR_UNKNOWN;
				break;
		}
	
		switch (pDVBSxParams.DVBSxModu)
		{
			case RTK_DVBSx_MODU_QPSK: //QPSK
				pInfo->dvbs.modulation = DVBS2_MOD_QPSK;
				break;
			case RTK_DVBSx_MODU_8PSK: //8PSK
				pInfo->dvbs.modulation = DVBS2_MOD_8PSK;
				break;
			case RTK_DVBSx_MODU_16APSK:
				pInfo->dvbs.modulation = DVBS2_MOD_16APSK;
				break;
			case RTK_DVBSx_MODU_32APSK:
				pInfo->dvbs.modulation = DVBS2_MOD_32APSK;
				break;
			case RTK_DVBSx_MODU_UNKNOWN:
			default:
				pInfo->dvbs.modulation = DVBS2_MOD_UNKNOWN;
				break;
		}
	}
	else if(pDVBSxParams.DVBSxType == RTK_DEMOD_DVBS_MODE)
	{
		pInfo->dvbs.mod = TV_MODULATION_DVBS; //bIsDVBS2
		pInfo->mod = TV_MODULATION_DVBS; //bIsDVBS2
	
		switch (pDVBSxParams.DVBSCodeRate)
		{
			case RTK_DVBS_CODE_RATE_1_2://Code Rate 1/2
				pInfo->dvbs.code_rate = DVBS_CR_1_2;
				break;
			case RTK_DVBS_CODE_RATE_2_3://Code Rate 2/3
				pInfo->dvbs.code_rate = DVBS_CR_2_3;
				break;
			case RTK_DVBS_CODE_RATE_3_4://Code Rate 3/4
				pInfo->dvbs.code_rate = DVBS_CR_3_4;
				break;
			case RTK_DVBS_CODE_RATE_5_6://Code Rate 5/6
				pInfo->dvbs.code_rate = DVBS_CR_5_6;
				break;
			case RTK_DVBS_CODE_RATE_7_8://Code Rate 7/8
				pInfo->dvbs.code_rate = DVBS_CR_7_8;
				break;
			case RTK_DVBS_CODE_RATE_UNKNOWN:
			default:
				pInfo->dvbs.code_rate = DVBS_CR_UNKNOWN;
				break;
		}
	
		pInfo->dvbs.modulation = DVBS2_MOD_QPSK;
	}
	else if(pDVBSxParams.DVBSxType == RTK_DEMOD_DVBS2X_MODE)
	{
		pInfo->dvbs.mod = TV_MODULATION_DVBS2; //bIsDVBS2
		pInfo->mod = TV_MODULATION_DVBS2; //bIsDVBS2
		
		switch (pDVBSxParams.DVBS2XCodeRate)
		{
			case RTK_DVBS2X_CODE_RATE_DUMMY:
				pInfo->dvbs.code_rate = DVBS2X_CR_DUMMY;
				break;
			case RTK_DVBS2X_CODE_RATE_13_45://Code Rate 13/45
				pInfo->dvbs.code_rate = DVBS2X_CR_13_45;
				break;
			case RTK_DVBS2X_CODE_RATE_9_20:
				pInfo->dvbs.code_rate = DVBS2X_CR_9_20;
				break;
			case RTK_DVBS2X_CODE_RATE_90_180:
				pInfo->dvbs.code_rate = DVBS2X_CR_90_180;
				break;
			case RTK_DVBS2X_CODE_RATE_96_180:
				pInfo->dvbs.code_rate = DVBS2X_CR_96_180;
				break;
			case RTK_DVBS2X_CODE_RATE_11_20:
				pInfo->dvbs.code_rate = DVBS2X_CR_11_20;
				break;
			case RTK_DVBS2X_CODE_RATE_100_180:
				pInfo->dvbs.code_rate = DVBS2X_CR_100_180;
				break;
			case RTK_DVBS2X_CODE_RATE_26_45:
				pInfo->dvbs.code_rate = DVBS2X_CR_26_45;
				break;
			case RTK_DVBS2X_CODE_RATE_18_30:
				pInfo->dvbs.code_rate = DVBS2X_CR_18_30;
				break;
			case RTK_DVBS2X_CODE_RATE_28_45:
				pInfo->dvbs.code_rate = DVBS2X_CR_28_45;
				break;
			case RTK_DVBS2X_CODE_RATE_23_36:
				pInfo->dvbs.code_rate = DVBS2X_CR_23_36;
				break;
			case RTK_DVBS2X_CODE_RATE_20_30:
				pInfo->dvbs.code_rate = DVBS2X_CR_20_30;
				break;
			case RTK_DVBS2X_CODE_RATE_25_36:
				pInfo->dvbs.code_rate = DVBS2X_CR_25_36;
				break;
			case RTK_DVBS2X_CODE_RATE_128_180:
				pInfo->dvbs.code_rate = DVBS2X_CR_128_180;
				break;
			case RTK_DVBS2X_CODE_RATE_13_18:
				pInfo->dvbs.code_rate = DVBS2X_CR_13_18;
				break;
			case RTK_DVBS2X_CODE_RATE_22_30:
				pInfo->dvbs.code_rate = DVBS2X_CR_22_30;
				break;
			case RTK_DVBS2X_CODE_RATE_7_9:
			case RTK_DVBS2X_CODE_RATE_140_180:
				pInfo->dvbs.code_rate = DVBS2X_CR_7_9;
				break;
			case RTK_DVBS2X_CODE_RATE_154_180:
				pInfo->dvbs.code_rate = DVBS2X_CR_154_180;
				break;
			case RTK_DVBS2X_CODE_RATE_UNKNOWN:
				pInfo->dvbs.code_rate = DVBS2X_CR_UNKNOWN;
				break;
			default:
				pInfo->dvbs.code_rate = DVBS2X_CR_UNKNOWN;
				break;
		}
		
		switch (pDVBSxParams.DVBSxModu)
		{
			case RTK_DVBSx_MODU_QPSK: //QPSK
				pInfo->dvbs.modulation = DVBS2_MOD_QPSK;
				break;
			case RTK_DVBSx_MODU_8PSK: //8PSK
				pInfo->dvbs.modulation = DVBS2_MOD_8PSK;
				break;
			case RTK_DVBSx_MODU_16APSK:
				pInfo->dvbs.modulation = DVBS2_MOD_16APSK;
				break;
			case RTK_DVBSx_MODU_32APSK:
				pInfo->dvbs.modulation = DVBS2_MOD_32APSK;
				break;
			case RTK_DVBSx_MODU_UNKNOWN:
			default:
				pInfo->dvbs.modulation = DVBS2_MOD_UNKNOWN;
				break;
		}
	}
	
	REALTEK_A_DVBSX_INFO("GetSignalInfo, STANDARD = %u, SYMBOL_RATE = "PT_U32BITS", MODULATION = %u, CODE_RATE = %u, SPEC_INV = %u\n", 
		(unsigned int)pInfo->dvbs.mod, pInfo->dvbs.symbol_rate, (unsigned int)pInfo->dvbs.modulation, (unsigned int)pInfo->dvbs.code_rate, (unsigned int)pInfo->dvbs.spec_invert);	
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
int REALTEK_A_DVBSX_GetSignalQuality(DEMOD* pDemodKernel, ENUM_TV_QUAL id, TV_SIG_QUAL* pQuality)
{
	int Snr = 0;
	//unsigned int Ber = 0, Per = 0, PakN = 0, BadPakN = 0;
	unsigned int PakN = 0, BadPakN = 0;
	unsigned short Ubc = 0, Agc = 0;
	unsigned char DvbSxMode = 0;
	int ret = TUNER_CTRL_OK;
	
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	if(!DtvIsDemodClockEnable(RTK_DEMOD_MODE_DVBS2))
		return TUNER_CTRL_FAIL;
	
	pDemodKernel->Lock(pDemodKernel);
	
	switch (id){
	case TV_QUAL_SNR:
		RTK_A_DVBSX_GetDVBSxSNR(pDemod, &Snr);
		pQuality->snr = Snr*1000;//SNR/10*10000
		//REALTEK_A_DVBSX_INFO("Snr %d ("PT_U32BITS")\n", Snr, pQuality->snr);
		break;
	case TV_QUAL_BER:
		//RTK_A_DVBSX_GetDVBSxBER(pDemod, &Ber);
		//pQuality->ber = Ber/100000;
		RTK_A_DVBSX_GetDVBSxBEC(pDemod, &BadPakN, &PakN);
		if(PakN > 0)
		{
			pQuality->layer_ber.ber_num = BadPakN;
			pQuality->layer_ber.ber_den = PakN;
			pQuality->ber = (BadPakN * 10000) / PakN;
		}
		else
		{
			pQuality->layer_ber.ber_num = 0xffffffff;
			pQuality->layer_ber.ber_den = 0xffffffff;
			pQuality->ber = 0xffffffff;
		}
		break;
	case TV_QUAL_PER:
		//RTK_A_DVBSX_GetDVBSxPER(pDemod, &Per);
		//pQuality->per = Per/100000;
		RTK_A_DVBSX_GetPEC(pDemod, &BadPakN, &PakN);
		if(PakN > 0)
		{
			pQuality->layer_per.per_num = BadPakN;
			pQuality->layer_per.per_den = PakN;
			pQuality->per = (BadPakN * 10000) / PakN;
		}
		else
		{
			pQuality->layer_per.per_num = 0xffffffff;
			pQuality->layer_per.per_den = 0xffffffff;
			pQuality->per = 0xffffffff;
		}
		break;
	case TV_QUAL_UCBLOCKS:
		RTK_A_DVBSX_GetDVBPktErrorCounter(pDemod, &Ubc);
		pQuality->ucblocks = Ubc;
		break;
	case TV_QUAL_SIGNAL_STRENGTH:
		pQuality->strength = REALTEK_A_DVBSX_CalDvbS2Ssi(pDemodKernel);
		break;
	case TV_QUAL_SIGNAL_QUALITY:
		RTK_A_DVBSX_DVBSxModeDetected(pDemod, &DvbSxMode);
		if(DvbSxMode == RTK_DEMOD_DVBS2_MODE)
		{
			pQuality->quality = REALTEK_A_DVBSX_CalDvbS2Sqi(pDemodKernel);
		}
		else if(DvbSxMode == RTK_DEMOD_DVBS_MODE)
		{
			pQuality->quality = REALTEK_A_DVBSX_CalDvbSSqi(pDemodKernel);
		}
		else
		{
			pQuality->quality = 0;
		}
		break;
	case TV_QUAL_AGC:
		RTK_A_DVBSX_SignalIFAGC(pDemod, &Agc);
		Agc = (100-(((Agc)*100)/1024)*2)*3/4+6;//Modify for TCL Spec
		pQuality->agc = Agc;
		break;
	case TV_QUAL_BEC:
		RTK_A_DVBSX_GetDVBSxBEC(pDemod, &BadPakN, &PakN);
		pQuality->layer_ber.ber_num = BadPakN;
		pQuality->layer_ber.ber_den = PakN;
		REALTEK_A_DVBSX_DBG("pQuality->layer_ber.ber_num "PT_U32BITS"\n", pQuality->layer_ber.ber_num);
		REALTEK_A_DVBSX_DBG("pQuality->layer_ber.ber_den "PT_U32BITS"\n", pQuality->layer_ber.ber_den);
		break;
	case TV_QUAL_PEC:
		RTK_A_DVBSX_GetPEC(pDemod, &BadPakN, &PakN);
		pQuality->layer_per.per_num = BadPakN;
		pQuality->layer_per.per_den = PakN;
		#ifndef PER_COLOR_ENABLE
		REALTEK_A_DVBSX_DBG("pQuality->layer_per.per_den "PT_U32BITS"\n", pQuality->layer_per.per_den);
		REALTEK_A_DVBSX_DBG("pQuality->layer_per.per_num "PT_U32BITS"\n", pQuality->layer_per.per_num);
		#else
		if(pQuality->layer_per.per_num > 0)
		{
			REALTEK_A_DVBSX_INFO("\033[1;32;33m" "!!!!!!!!!! Packet Err "PT_U32BITS"\n" "\033[m", pQuality->layer_per.per_num);
		}
		else
		{
			REALTEK_A_DVBSX_INFO("\033[1;32;32m" "Packet Err "PT_U32BITS"\n" "\033[m", pQuality->layer_per.per_num);
		}
		#endif
		break;
	case TV_QUAL_DEMODFWVERSION:
		pQuality->DemodFwVersion = 0x000001;
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
 * Note : the offset value is only valid as demod is on lock stage.
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_GetCarrierOffset(DEMOD* pDemodKernel, S32BITS* pOffset)
{
	int ret = TUNER_CTRL_OK;
	int Cfo = 0;
	
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	if(!DtvIsDemodClockEnable(RTK_DEMOD_MODE_DVBS2))
		return TUNER_CTRL_FAIL;
	
	pDemodKernel->Lock(pDemodKernel);
	
	RTK_A_DVBSX_GetDVBSxLockedCFO(pDemod, &Cfo);
	*pOffset = Cfo*1000;
	
	pDemodKernel->Unlock(pDemodKernel);
	
	REALTEK_A_DVBSX_INFO("GetCarrierOffset "PT_S32BITS" Hz\n", *pOffset);
	
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
int REALTEK_A_DVBSX_GetDebugLogBuf(DEMOD* pDemodKernel, unsigned int* LogBufAddr, unsigned int* LogBufSize)
{
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : SpecAnGetSpectrum
 *
 * Desc : Get spectrum data.
 *
 * Parm : Mode : Select mode for getting spectrum
 *        SpectrumArray : Spectrum data result
 *        SpectrumLength : Spectrum data length
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_SpecAnGetSpectrum(DEMOD* pDemodKernel, unsigned int *SpectrumArray, unsigned int *SpectrumLength)
{
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : SpecAnGetCp1Cp2
 *
 * Desc : Get cross points in the spectrum.
 *
 * Parm : SpectrumArray : Input spectrum data
 *        CutTh : Set threshold for finding cross points
 *        CutSp : Set start point for finding cross points
 *        CutEp : Set end point for finding cross points
 *        ReturnCp1 : Cross point 1 result
 *        ReturnCp2 : Cross point 2 result
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_SpecAnGetCp1Cp2(
	DEMOD* pDemodKernel,
	unsigned int *SpectrumArray,
	unsigned int SkipDc,
	unsigned int CutSp,
	unsigned int CutEp,
	short *AllCutCandidateNum,
	short *AllCutCandidateCp1,
	short *AllCutCandidateCp2,
	unsigned int CutTh_1st,
	unsigned int CutTh_2nd,
	unsigned int CutTh_3rd,
	unsigned int CutTh_4th,
	unsigned int CutTh_5th,
	unsigned int CutTh_6th)
{
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : SpecAnCalCfoCsr
 *
 * Desc : Calculate signal carrier frequency offset and course symbol rate.
 *
 * Parm : Mode : Calculation mode
 *        Cp1 : Input cross point 1
 *        Cp2 : Input cross point 2
 *        SigCfo : Carrier frequency offset calculation result
 *        SigCsr : Course symbol rate calculation result
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_SpecAnCalCfoCsr(DEMOD* pDemodKernel, short Cp1, short Cp2, int *SigCfo, unsigned int *SigCsr)
{
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : SpecAnSearchMaxMin
 *
 * Desc : Get max and min level in the spectrum.
 *
 * Parm : SpectrumArray : Input spectrum data
 *        Sp : Start search point
 *        Ep : End search point
 *        MaxPower : Max level of input spectrum between Sp and Ep
 *        MinPower : Min level of input spectrum between Sp and Ep
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_SpecAnSearchMaxMin(
	DEMOD* pDemodKernel,
	unsigned int *SpectrumArray,
	unsigned int Sp,
	unsigned int Ep,
	unsigned int *MaxPower,
	unsigned int *MaxPowerPos,
	unsigned int *MinPower,
	unsigned int *MinPowerPos
)
{
	return TUNER_CTRL_OK;
}


int REALTEK_A_DVBSX_BlindscanCheckSignalLock(DEMOD* pDemodKernel, unsigned int CCfo, unsigned int Csr, unsigned char *SigLock, unsigned int *FSr, int *FCfo)
{
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : BlindscanAnalyzeSpectrum
 *
 * Desc : Analyze spectrum.
 *
 * Parm : SignalFound : Is there a signal in the spectrum
 *        SigCfo : Signal carrier offest in the spectrum (unit : Hz)
 *        SigCsr : Signal course symbol rate in the spectrum (unit : Hz)
 *        NextStep : Next tuner move step (unit : Hz)
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_BlindscanAnalyzeSpectrum(DEMOD* pDemodKernel, unsigned char *SignalFound, int *SigCfo, unsigned int *SigCsr, unsigned int *NextStep)
{
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : BlindScan_Init
 *
 * Desc : Initiate blindscan.
 *
 * Parm : StartFreqMhz : start frequency for blindscan (unit : Hz)
 *        EndFreqMhz : end frequency for blindscan (unit : Hz)
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_BlindScan_Init(DEMOD* pDemodKernel, unsigned int StartFreqMhz, unsigned int EndFreqMhz)
{
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	REALTEK_A_DVBSX_INFO("REALTEK_A_DVBSX_BlindScan_Init %u %u =========================================\n", StartFreqMhz, EndFreqMhz);
	
	pRealtekIDVBSX->m_auto_tune_en = 0;// Disable autotune when scan signal
	RTK_A_DVBSX_DvBSxBlindScanInit(pDemod);
	RTK_A_DVBSX_SetDvBSxBlindScanParams(pDemod, StartFreqMhz*1000, EndFreqMhz*1000);
	
	pRealtekIDVBSX->m_bs_stime = tv_osal_time();
	pRealtekIDVBSX->m_bs_init_done = 1;
	
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : BlindScan_GetTunerFreq
 *
 * Desc : Get tuner frequency and tuner bandwidth for current blindscan step.
 *
 * Parm : TunerCenterFreqMhz : tuner frequency (unit : Hz)
 *        TunerCutOffFreqKhz : tuner bandwidth (unit : Hz)
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_BlindScan_GetTunerFreq(DEMOD* pDemodKernel, unsigned int *TunerCenterFreqMhz, unsigned int *TunerCutOffFreqKhz)
{
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	//REALTEK_A_DVBSX_INFO("REALTEK_A_DVBSX_BlindScan_GetTunerFreq\n");
	
	RTK_A_DVBSX_BlindScan_GetTunerFreq(pDemod, TunerCenterFreqMhz, TunerCutOffFreqKhz);
	
	REALTEK_A_DVBSX_INFO("REALTEK_A_DVBSX_BlindScan_GetTunerFreq %d %d\n", *TunerCenterFreqMhz, *TunerCutOffFreqKhz);
	
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : BlindScan_NextFreq
 *
 * Desc : Check is blindscan done.
 *
 * Parm : bBlindScanEnd : Is blindscan done?
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_BlindScan_NextFreq(DEMOD* pDemodKernel, unsigned int *bBlindScanEnd)
{
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	//REALTEK_A_DVBSX_INFO("REALTEK_A_DVBSX_BlindScan_NextFreq\n");
	
	RTK_A_DVBSX_BlindScan_NextFreq(pDemod, bBlindScanEnd);
	
	return TUNER_CTRL_OK;
}


int REALTEK_A_DVBSX_BlindscanRemoveDuplicateTp(DEMOD* pDemodKernel, unsigned char mode)
{
	return 0;
}


/*----------------------------------------------------------------------
 * Func : BlindScan_GetStatus
 *
 * Desc : Get blindscan status.
 *
 * Parm : Status : blindscan status
 *        bBlindScanLock : Does blindscan search a signal?
  *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_BlindScan_GetStatus(DEMOD* pDemodKernel, unsigned char *Status, unsigned int *bBlindScanLock)
{
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	//REALTEK_A_DVBSX_INFO("REALTEK_A_DVBSX_BlindScan_GetStatus\n");
	
	RTK_A_DVBSX_BlindScan_GetStatus(pDemod, Status, bBlindScanLock);
	
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : BlindScan_End
 *
 * Desc : End blindscan.
 *
 * Parm : N/A
  *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_BlindScan_End(DEMOD* pDemodKernel)
{
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	REALTEK_A_DVBSX_INFO("REALTEK_A_DVBSX_BlindScan_End =========================================\n");
	
	if(pRealtekIDVBSX->m_bs_init_done)
	{
		REALTEK_A_DVBSX_INFO("=========================================\n");
		pRealtekIDVBSX->m_bs_etime = tv_osal_time();
		REALTEK_A_DVBSX_INFO("BlindScan Time(sec) %u\n", (pRealtekIDVBSX->m_bs_etime-pRealtekIDVBSX->m_bs_stime)/1000);
		REALTEK_A_DVBSX_INFO("=========================================\n");
		RTK_A_DVBSX_BlindScan_End(pDemod);
		REALTEK_A_DVBSX_INFO("=========================================\n");
		pRealtekIDVBSX->m_bs_init_done = 0;
	}
	else
	{
		REALTEK_A_DVBSX_INFO("Blindscan Not Init\n");
	}

	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : BlindScan_GetFoundTP
 *
 * Desc : Get blindscan search result.
 *
 * Parm : FreqMhz : frequency of found TP (unit : MHz)
 *        SymbolRateKhz : symbol rate of found TP (unit : MHz)
  *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_BlindScan_GetFoundTP(DEMOD* pDemodKernel, unsigned int *FreqMhz, unsigned int *SymbolRateKhz)
{
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	//REALTEK_A_DVBSX_INFO"REALTEK_A_DVBSX_BlindScan_GetFoundTP\n");
	
	RTK_A_DVBSX_BlindScan_GetFoundTP(pDemod, FreqMhz, SymbolRateKhz);
	
	REALTEK_A_DVBSX_INFO("\033[1;32;31m" "REALTEK_A_DVBSX_BlindScan_GetFoundTP %d %d\n" "\033[m", *FreqMhz, *SymbolRateKhz);
	
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : BlindScan_AcquireSignal
 *
 * Desc : Acquire Signal in Blindscan
 *
 * Parm : 
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_BlindScan_AcquireSignal(DEMOD* pDemodKernel,unsigned int *FrequencyKhz, unsigned int *SymbolRateKhz, int *Fcfo, unsigned char *bTpLock)
{
	unsigned long stime = 0, etime = 0;
	unsigned char lock = 0;
	unsigned char re_acq_cnt = 1;
	int CoarseCFO = 0, LockedCFO = 0, Snr = 0;
	RTK_DVBSx_SIGNAL_PARAMS pDVBSxParams;
	#ifdef DISABLE_DVBS2X
	RTK_DVBS2_ROLL_OFF_TYPE RollOffType;
	#endif
	#ifdef DYNAMIC_TPO_CLK
	unsigned long tpo_stime = 0, tpo_etime = 0;
	#endif

	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*) pRealtekIDVBSX->m_private)->pDemod;

	pRealtekIDVBSX->m_info.dvbs.symbol_rate = *SymbolRateKhz*1000;
	pRealtekIDVBSX->m_info.dvbs.freq_khz = *FrequencyKhz;

	if(pRealtekIDVBSX->m_info.dvbs.symbol_rate == 0 || pRealtekIDVBSX->m_info.dvbs.symbol_rate > 50000000)
	{
		REALTEK_A_DVBSX_WARNING("pRealtekIDVBSX->m_info.dvbs.symbol_rate="PT_U32BITS", REALTEK_A_DVBSX_BlindScan_AcquireSignal Break\n", pRealtekIDVBSX->m_info.dvbs.symbol_rate);
		return TUNER_CTRL_NO_SIGNAL;
	}

re_bs_acq:
	pRealtekIDVBSX->m_auto_tune_en = 0;
	pRealtekIDVBSX->m_acquire_sig_en = 0;
	pRealtekIDVBSX->m_info.dvbs.scan_mode = 1;

	RTK_A_DVBSX_SetDvbSxMode(pDemod, 0, pRealtekIDVBSX->m_info.dvbs.symbol_rate, DUMMY_FREQ_KHZ); //Default Setting
	acquire_sig_stime_s = tv_osal_time();

	stime = tv_osal_time();
	RTK_A_DVBSX_ChannelLockCheck(pDemod, &lock);

	if(lock)
	{
		RTK_A_DVBSX_GetDVBSxCoarseCFO(pDemod, &CoarseCFO);
		RTK_A_DVBSX_GetDVBSxLockedCFO(pDemod, &LockedCFO);
		RTK_A_DVBSX_GetDVBSxSNR(pDemod, &Snr);
	
		RTK_A_DVBSX_GetDVBSxSignalParams(pDemod, &pDVBSxParams);
		if(!(pRealtekIDVBSX->m_info.dvbs.scan_mode == 1 && pRealtekIDVBSX->m_info.dvbs.symbol_rate >= 19000000 && pDVBSxParams.DVBSxType == RTK_DEMOD_DVBS2_MODE && pDVBSxParams.DVBSxModu == RTK_DVBSx_MODU_8PSK && (pDVBSxParams.DVBS2CodeRate == RTK_DVBS2_CODE_RATE_3_4 || pDVBSxParams.DVBS2CodeRate == RTK_DVBS2_CODE_RATE_5_6)))
		{
			RTK_A_DVBSX_Write(pDemod, 0x2a, 0xe0, 0x20);
		}
		else
		{
			REALTEK_A_DVBSX_INFO("\033[1;32;34m" "Special SR & MODCOD\n" "\033[m");
		}
		
		if(pRealtekIDVBSX->m_info.dvbs.symbol_rate > 29700000 && pRealtekIDVBSX->m_info.dvbs.symbol_rate < 29740000 && pDVBSxParams.DVBS2CodeRate != RTK_DVBS2_CODE_RATE_5_6)
		{
			REALTEK_A_DVBSX_INFO("Lock DAGC Resume\n");
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

		if(pRealtekIDVBSX->m_info.dvbs.symbol_rate > 17000000)
		{
			RTK_A_DVBSX_PilotTrackingAdjust(pDemod, pRealtekIDVBSX->m_info.dvbs.symbol_rate/1000);
		}
		
		if(pDemod->CurrentTunerId == TV_TUNER_RT710 && pDVBSxParams.DVBSxType == RTK_DEMOD_DVBS_MODE)
		{
			REALTEK_A_DVBSX_INFO("\033[1;32;34m" "RT710 DVBS increase target level and reduce reacq delay time for special field stream\n" "\033[m");
			RTK_A_DVBSX_Write(pDemod, 0x10, 0x05, 0x03);
			RTK_A_DVBSX_Write(pDemod, 0x10, 0x04, 0x30);
			RTK_A_DVBSX_Write(pDemod, 0x10, 0x03, 0xff);
			
			RTK_A_DVBSX_Write(pDemod, 0x29, 0x12, 0x01);
			RTK_A_DVBSX_Write(pDemod, 0x29, 0x14, 0x01);
		}

		#ifdef DISABLE_DVBS2X
		if(pDVBSxParams.DVBSxType == RTK_DEMOD_DVBS2X_MODE || pDVBSxParams.DVBSxType == RTK_DEMOD_DVBS2_OPTIONAL_MODE)
		{
			REALTEK_A_DVBSX_INFO("\033[1;32;34m" "Acquire - DVBS2 OPTIONAL & DVBS2X Force Unlock\n" "\033[m");
			//RTK_A_DVBSX_Write(pDemod, 0x18, 0x01, 0x01);//Set TS Serial Mode to Disable TS Output
			DtvDemodTPOutEn(0);  //disable TS output
			lock = 0;
		}
		else
		{
			if(pDVBSxParams.DVBSxType != RTK_DEMOD_DVBS_MODE)
			{
				//REALTEK_A_DVBSX_INFO("\033[1;32;33m" "DVBS2 Mode\n" "\033[m");
				RTK_A_DVBSX_GetDVBSxRollOff(pDemod, &RollOffType);
				if(RollOffType == RTK_DVBS2_ROLL_OFF_0p15 || RollOffType == RTK_DVBS2_ROLL_OFF_0p10 || RollOffType == RTK_DVBS2_ROLL_OFF_0p05)
				{
					REALTEK_A_DVBSX_INFO("\033[1;32;34m" "Acquire - Low Roll-Off Force Unlock\n" "\033[m");
					//RTK_A_DVBSX_Write(pDemod, 0x18, 0x01, 0x01);//Set TS Serial Mode to Disable TS Output
					DtvDemodTPOutEn(0);  //disable TS output
					lock = 0;
				}
				else
				{
					//RTK_A_DVBSX_Write(pDemod, 0x18, 0x01, 0x00);//Set TS Parallel Mode to Enable TS Output
					DtvDemodTPOutEn(1);  //enable TS output
				}
			}
			else
			{
				//REALTEK_A_DVBSX_INFO("\033[1;32;33m" "DVBS/Others Mode\n" "\033[m");
				//RTK_A_DVBSX_Write(pDemod, 0x18, 0x01, 0x00);//Set TS Parallel Mode to Enable TS Output
				DtvDemodTPOutEn(1);  //enable TS output
			}
		}
		#endif

		#ifdef DYNAMIC_TPO_CLK
		tpo_stime = tv_osal_time();
		REALTEK_A_DVBSX_TSBitRateCheck(pDemodKernel);
		tpo_etime = tv_osal_time();
		#endif

		pRealtekIDVBSX->m_auto_tune_en = 1;
		pRealtekIDVBSX->m_lock_pivot = 1;
	}
	else
	{
		RTK_A_DVBSX_Write(pDemod, 0x2a, 0xe0, 0x20);
		pRealtekIDVBSX->m_lock_pivot = 0;
		
		if(pRealtekIDVBSX->m_info.dvbs.symbol_rate > 29700000 && pRealtekIDVBSX->m_info.dvbs.symbol_rate < 29740000)
		{
			REALTEK_A_DVBSX_INFO("Unlock DAGC Resume\n");
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
	etime = tv_osal_time();

	*bTpLock = lock;
	*SymbolRateKhz = pRealtekIDVBSX->m_info.dvbs.symbol_rate/1000;
	*Fcfo = LockedCFO*1000;

	#ifdef DYNAMIC_TPO_CLK
	REALTEK_A_DVBSX_INFO("\033[1;32;31m" "BS Acquire %u   "PT_U32BITS" "PT_U32BITS"   Demod Lock = %u, Global Lock Time = %lu, Local Lock Time = %lu, TPO Time = %lu, Snr = %d, CoarseCFO = %d KHz, LockedCFO = %d KHz\n" "\033[m", 
		re_acq_cnt, pRealtekIDVBSX->m_info.dvbs.freq_khz/1000, pRealtekIDVBSX->m_info.dvbs.symbol_rate/1000, (unsigned int)lock, etime-acquire_sig_stime_s, etime-stime, tpo_etime-tpo_stime, Snr, CoarseCFO, LockedCFO);
	#else
	REALTEK_A_DVBSX_INFO("\033[1;32;31m" "BS Acquire %u   "PT_U32BITS" "PT_U32BITS"   Demod Lock = %u, Global Lock Time = %lu, Local Lock Time = %lu, Snr = %d, CoarseCFO = %d KHz, LockedCFO = %d KHz\n" "\033[m", 
		re_acq_cnt, pRealtekIDVBSX->m_info.dvbs.freq_khz/1000, pRealtekIDVBSX->m_info.dvbs.symbol_rate/1000, (unsigned int)lock, etime-acquire_sig_stime_s, etime-stime, Snr, CoarseCFO, LockedCFO);
	#endif

	if(pDemod->CurrentTunerId == TV_TUNER_RT710 && lock == 0 && re_acq_cnt < 3)
	{
		REALTEK_A_DVBSX_INFO("\033[1;32;32m" "RT710 retry\n" "\033[m");
		re_acq_cnt++;
		goto re_bs_acq;
	}

	return TUNER_CTRL_OK;
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
int REALTEK_A_DVBSX_AcquireSignal(DEMOD* pDemodKernel, unsigned char WaitSignalLock)
{
#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	unsigned long cur_time = 0;
#endif
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*) pRealtekIDVBSX->m_private)->pDemod;
	
	//if (WaitSignalLock == 0)
	//	return TUNER_CTRL_OK;
	
#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	cur_time = tv_osal_time();
	
	while((tv_osal_time() - cur_time) <= 15)
	{
		if(pRealtekIDVBSX->m_acquire_sig_en == 0)
			return TUNER_CTRL_OK;
	
		tv_osal_msleep(5);
	}
	
	return TUNER_CTRL_FAIL;
	
#else
	REALTEK_A_DVBSX_AcquireSignalThread(pDemodKernel);
	
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
int REALTEK_A_DVBSX_AcquireSignalThread(DEMOD* pDemodKernel)
{
	#ifdef DISABLE_DVBS2X
	RTK_DVBS2_ROLL_OFF_TYPE RollOffType;
	#endif
	RTK_DVBSx_SIGNAL_PARAMS pDVBSxParams;
	unsigned char lock = 0;
	unsigned long stime = 0, etime = 0;
	#ifdef DYNAMIC_TPO_CLK
	unsigned long tpo_stime = 0, tpo_etime = 0;
	#endif
	int CoarseCFO = 0, LockedCFO = 0, Snr = 0;
	
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;

	if (pRealtekIDVBSX->m_acquire_sig_en == 0)
		return TUNER_CTRL_OK;
	
	stime = tv_osal_time();
	RTK_A_DVBSX_ChannelLockCheck(pDemod, &lock);
	
	pRealtekIDVBSX->m_acquire_sig_en = 0;
	
	if(lock)
	{
		RTK_A_DVBSX_GetDVBSxCoarseCFO(pDemod, &CoarseCFO);
		RTK_A_DVBSX_GetDVBSxLockedCFO(pDemod, &LockedCFO);
		RTK_A_DVBSX_GetDVBSxSNR(pDemod, &Snr);
		
		RTK_A_DVBSX_GetDVBSxSignalParams(pDemod, &pDVBSxParams);
		if(!(pRealtekIDVBSX->m_info.dvbs.scan_mode == 1 && pRealtekIDVBSX->m_info.dvbs.symbol_rate >= 19000000 && pDVBSxParams.DVBSxType == RTK_DEMOD_DVBS2_MODE && pDVBSxParams.DVBSxModu == RTK_DVBSx_MODU_8PSK && (pDVBSxParams.DVBS2CodeRate == RTK_DVBS2_CODE_RATE_3_4 || pDVBSxParams.DVBS2CodeRate == RTK_DVBS2_CODE_RATE_5_6)))
		{
			RTK_A_DVBSX_Write(pDemod, 0x2a, 0xe0, 0x20);
		}
		else
		{
			REALTEK_A_DVBSX_INFO("\033[1;32;34m" "Special SR & MODCOD\n" "\033[m");
		}
		
		if(pRealtekIDVBSX->m_info.dvbs.symbol_rate > 29700000 && pRealtekIDVBSX->m_info.dvbs.symbol_rate < 29740000 && pDVBSxParams.DVBS2CodeRate != RTK_DVBS2_CODE_RATE_5_6)
		{
			REALTEK_A_DVBSX_INFO("Lock DAGC Resume\n");
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
		
		if(pRealtekIDVBSX->m_info.dvbs.symbol_rate > 17000000)
		{
			RTK_A_DVBSX_PilotTrackingAdjust(pDemod, pRealtekIDVBSX->m_info.dvbs.symbol_rate/1000);
		}

		#ifdef DISABLE_DVBS2X
		if(pDVBSxParams.DVBSxType == RTK_DEMOD_DVBS2X_MODE || pDVBSxParams.DVBSxType == RTK_DEMOD_DVBS2_OPTIONAL_MODE)
		{
			REALTEK_A_DVBSX_INFO("\033[1;32;34m" "Acquire - DVBS2 OPTIONAL & DVBS2X Force Unlock\n" "\033[m");
			//RTK_A_DVBSX_Write(pDemod, 0x18, 0x01, 0x01);//Set TS Serial Mode to Disable TS Output
			DtvDemodTPOutEn(0);  //disable TS output
			pRealtekIDVBSX->m_dis_dvb2x_rolloff_mode = 1;
			lock = 0;
		}
		else
		{
			if(pDVBSxParams.DVBSxType != RTK_DEMOD_DVBS_MODE)
			{
				//REALTEK_A_DVBSX_INFO("\033[1;32;33m" "DVBS2 Mode\n" "\033[m");
				RTK_A_DVBSX_GetDVBSxRollOff(pDemod, &RollOffType);
				if(RollOffType == RTK_DVBS2_ROLL_OFF_0p15 || RollOffType == RTK_DVBS2_ROLL_OFF_0p10 || RollOffType == RTK_DVBS2_ROLL_OFF_0p05)
				{
					REALTEK_A_DVBSX_INFO("\033[1;32;34m" "Acquire - Low Roll-Off Force Unlock\n" "\033[m");
					//RTK_A_DVBSX_Write(pDemod, 0x18, 0x01, 0x01);//Set TS Serial Mode to Disable TS Output
					DtvDemodTPOutEn(0);  //disable TS output
					pRealtekIDVBSX->m_dis_dvb2x_rolloff_mode = 1;
					lock = 0;
				}
				else
				{
					//RTK_A_DVBSX_Write(pDemod, 0x18, 0x01, 0x00);//Set TS Parallel Mode to Enable TS Output
					DtvDemodTPOutEn(1);  //enable TS output
					pRealtekIDVBSX->m_dis_dvb2x_rolloff_mode = 0;
					//DtvDemodTPOutEn(0);  //disable TS output
					//RTK_A_DVBSX_SetLdpc1p6XMaxIteration(pDemod, pRealtekIDVBSX->m_info.dvbs.symbol_rate/1000, 1);
					//DtvDemodTPOutEn(1);  //enable TS output
				}
			}
			else
			{
				//REALTEK_A_DVBSX_INFO("\033[1;32;33m" "DVBS/Others Mode\n" "\033[m");
				//RTK_A_DVBSX_Write(pDemod, 0x18, 0x01, 0x00);//Set TS Parallel Mode to Enable TS Output
				DtvDemodTPOutEn(1);  //enable TS output
				pRealtekIDVBSX->m_dis_dvb2x_rolloff_mode = 0;
				//DtvDemodTPOutEn(0);  //disable TS output
				//RTK_A_DVBSX_SetLdpc1p6XMaxIteration(pDemod, pRealtekIDVBSX->m_info.dvbs.symbol_rate/1000, 1);
				//DtvDemodTPOutEn(1);  //enable TS output
			}
		}
		#else
		//DtvDemodTPOutEn(0);  //disable TS output
		//RTK_A_DVBSX_SetLdpc1p6XMaxIteration(pDemod, pRealtekIDVBSX->m_info.dvbs.symbol_rate/1000, 1);
		//DtvDemodTPOutEn(1);  //enable TS output
		#endif

		#ifdef DYNAMIC_TPO_CLK
		tpo_stime = tv_osal_time();
		REALTEK_A_DVBSX_TSBitRateCheck(pDemodKernel);
		tpo_etime = tv_osal_time();
		#endif

		pRealtekIDVBSX->m_auto_tune_en = 1;
		pRealtekIDVBSX->m_lock_pivot = 3;
	}
	else
	{
		RTK_A_DVBSX_Write(pDemod, 0x2a, 0xe0, 0x20);
		pRealtekIDVBSX->m_lock_pivot = 0;
		
		if(pRealtekIDVBSX->m_info.dvbs.symbol_rate > 29700000 && pRealtekIDVBSX->m_info.dvbs.symbol_rate < 29740000)
		{
			REALTEK_A_DVBSX_INFO("Unlock DAGC Resume\n");
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
	etime = tv_osal_time();

	#ifdef DYNAMIC_TPO_CLK
	REALTEK_A_DVBSX_INFO("\033[1;32;31m" "Acquire  "PT_U32BITS"    "PT_U32BITS"    Demod Lock = %u, Global Lock Time = %lu, Local Lock Time = %lu, TPO Time = %lu, Snr = %d, CoarseCFO = %d KHz, LockedCFO = %d KHz\n" "\033[m", 
		pRealtekIDVBSX->m_info.dvbs.freq_khz/1000, pRealtekIDVBSX->m_info.dvbs.symbol_rate/1000, (unsigned int)lock, etime-acquire_sig_stime_s, etime-stime, tpo_etime-tpo_stime, Snr, CoarseCFO, LockedCFO);
	#else
	REALTEK_A_DVBSX_INFO("\033[1;32;31m" "Acquire  "PT_U32BITS"    "PT_U32BITS"    Demod Lock = %u, Global Lock Time = %lu, Local Lock Time = %lu, Snr = %d, CoarseCFO = %d KHz, LockedCFO = %d KHz\n" "\033[m", 
		pRealtekIDVBSX->m_info.dvbs.freq_khz/1000, pRealtekIDVBSX->m_info.dvbs.symbol_rate/1000, (unsigned int)lock, etime-acquire_sig_stime_s, etime-stime, Snr, CoarseCFO, LockedCFO);
	#endif

	return TUNER_CTRL_OK;
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
 * Retn : TUNER_CTRL_OK : signal found and locked
 *        TUNER_CTRL_FAIL : error occured during scan channel
 *        TUNER_CTRL_NO_SIGNAL : no signal found in specified range
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_ScanSignal(DEMOD* pDemodKernel, S32BITS* pOffset, SCAN_RANGE range)
{
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
int REALTEK_A_DVBSX_ForwardI2C(DEMOD* pDemodKernel, unsigned char on_off)
{
	return TUNER_CTRL_OK;
}

int REALTEK_A_DVBSX_TSBitRateCheck(DEMOD* pDemodKernel)
{
	int ret = TUNER_CTRL_OK;
	unsigned int TSThroughputBps = 96000, TsClkKHz = 12000;
	unsigned char lock = 0, ts_lock = 0;
	unsigned long cur_time = 0;

	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;

	pDemodKernel->Lock(pDemodKernel);

	if(pRealtekIDVBSX->m_info.dvbs.symbol_rate > RTK_DVBSX_LITE_SYMBOL_HZ)
	{
		RTK_A_DVBSX_FECLockFlag(pDemod, &lock);
		RTK_A_DVBSX_TSLockFlag(pDemod, &ts_lock);
	}
	else 
	{
		RTK_A_DVBSX_TimingLockFlag(pDemod, &lock);
		RTK_A_DVBSX_TSLockFlag(pDemod, &ts_lock);
	}

	if ((lock&ts_lock) != 1)
	{
		goto ts_clk_check_done;
	}

	cur_time = tv_osal_time();
	while((tv_osal_time() - cur_time) <= 100)
	{
		ret |= RTK_A_DVBSX_TSBitAccu(pDemod, &TSThroughputBps);
		if(TSThroughputBps != 0)
			break;

		tv_osal_msleep(5);
	}

	if (ret != TUNER_CTRL_OK)
		goto ts_clk_check_done;

	TsClkKHz = TSThroughputBps * 107 / 100 / 8;

	if (RTK_A_DVBSX_AbsSub(TsClkKHz, pRealtekIDVBSX->m_ts_clk_khz) > 30  && TsClkKHz != 0)
	{
		DemodTsOutClkCtrl(TsClkKHz);
		pRealtekIDVBSX->m_ts_clk_khz = TsClkKHz;
		REALTEK_A_DVBSX_INFO("\033[1;32;33m" "[TSBitRateCheck] throughput_bps = %u, TsClkKHz = %u\n" "\033[m", TSThroughputBps, TsClkKHz); 
	}

ts_clk_check_done:

	pDemodKernel->Unlock(pDemodKernel);
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
int REALTEK_A_DVBSX_AutoTune(DEMOD* pDemodKernel)
{
#ifndef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	stRTK_LOCKING_STATUS sRTKLockingStatus;
	#ifdef DISABLE_DVBS2X
	RTK_DVBSx_SIGNAL_PARAMS pDVBSxParams;
	RTK_DVBS2_ROLL_OFF_TYPE RollOffType;
	static unsigned char DIS_dvbs2x_rolloff_cnt = 0;
	unsigned char ts_lock = 0, fec_lock = 0;
	#endif
	static unsigned char DPLL_patch_cnt = 0;
	static unsigned char TPO_patch_cnt = 0;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	if(pRealtekIDVBSX->m_acquire_sig_en && (pRealtekIDVBSX->m_info.dvbs.scan_mode==0))
	{
		REALTEK_A_DVBSX_AcquireSignalThread(pDemodKernel);
	}

	if(pRealtekIDVBSX->m_auto_tune_en && (pRealtekIDVBSX->m_info.dvbs.symbol_rate > RTK_DVBSX_LITE_SYMBOL_HZ))
	{
		if(DPLL_patch_cnt == 0)
		{
			DPLL_patch_cnt = 7;
			RTK_A_DVBSX_GetLockingStatus(pDemod, &sRTKLockingStatus);

			if(sRTKLockingStatus.u8FecLockFlag==1 && sRTKLockingStatus.u8TSLockFlag==1)	{
				u8stLockCnt++;
				if(u8stLockCnt>3) u8stLockCnt = 3;
			}
			else {
				u8stLockCnt = 0;
			}

			if(sRTKLockingStatus.u8TimingLockFlag)
			{
				//REALTEK_A_DVBSX_INFO("DPLLTrackingSpeedPatch in AutoTune\n");
				RTK_A_DVBSX_DPLLTrackingSpeedPatch(pDemod);
				RTK_A_DVBSX_PhaseTrackingSpeedPatch(pDemod);
			}

			if(pRealtekIDVBSX->m_info.dvbs.symbol_rate > 17000000)
			{
				RTK_A_DVBSX_PilotTrackingAdjust(pDemod, pRealtekIDVBSX->m_info.dvbs.symbol_rate/1000);
			}
		}
		DPLL_patch_cnt--;

		if(TPO_patch_cnt == 0)
		{
			//REALTEK_A_DVBSX_INFO("TSBitRateCheck in AutoTune\n");
			TPO_patch_cnt = 20;
			REALTEK_A_DVBSX_TSBitRateCheck(pDemodKernel);
			//RTK_A_DVBSX_SetLdpc1p6XMaxIteration(pDemod, pRealtekIDVBSX->m_info.dvbs.symbol_rate/1000, 0);
		}
		TPO_patch_cnt--;
	}
	
	#ifdef DISABLE_DVBS2X
	if(pRealtekIDVBSX->m_auto_tune_en)
	{
		if(DIS_dvbs2x_rolloff_cnt == 0)
		{
			DIS_dvbs2x_rolloff_cnt = 25;
			RTK_A_DVBSX_FECLockFlag(pDemod, &fec_lock);
			RTK_A_DVBSX_TSLockFlag(pDemod, &ts_lock);
			if(fec_lock == 1 || ts_lock == 1)
			{
				RTK_A_DVBSX_GetDVBSxSignalParams(pDemod, &pDVBSxParams);
				if(pDVBSxParams.DVBSxType == RTK_DEMOD_DVBS2X_MODE || pDVBSxParams.DVBSxType == RTK_DEMOD_DVBS2_OPTIONAL_MODE)
				{
					REALTEK_A_DVBSX_INFO("\033[1;32;34m" "AutoTune - DVBS2 OPTIONAL & DVBS2X Force Unlock\n" "\033[m");
					//RTK_A_DVBSX_Write(pDemod, 0x18, 0x01, 0x01);//Set TS Serial Mode to Disable TS Output
					DtvDemodTPOutEn(0);  //disable TS output
					pRealtekIDVBSX->m_dis_dvb2x_rolloff_mode = 1;
				}
				else
				{
					if(pDVBSxParams.DVBSxType != RTK_DEMOD_DVBS_MODE)
					{
						//REALTEK_A_DVBSX_INFO("\033[1;32;33m" "DVBS2 Mode\n" "\033[m");
						RTK_A_DVBSX_GetDVBSxRollOff(pDemod, &RollOffType);
						if(RollOffType == RTK_DVBS2_ROLL_OFF_0p15 || RollOffType == RTK_DVBS2_ROLL_OFF_0p10 || RollOffType == RTK_DVBS2_ROLL_OFF_0p05)
						{
							REALTEK_A_DVBSX_INFO("\033[1;32;34m" "AutoTune - Low Roll-Off Force Unlock\n" "\033[m");
							//RTK_A_DVBSX_Write(pDemod, 0x18, 0x01, 0x01);//Set TS Serial Mode to Disable TS Output
							DtvDemodTPOutEn(0);  //disable TS output
							pRealtekIDVBSX->m_dis_dvb2x_rolloff_mode = 1;
						}
						else
						{
							//RTK_A_DVBSX_Write(pDemod, 0x18, 0x01, 0x00);//Set TS Parallel Mode to Enable TS Output
							DtvDemodTPOutEn(1);  //enable TS output
							pRealtekIDVBSX->m_dis_dvb2x_rolloff_mode = 0;
						}
					}
					else
					{
						//REALTEK_A_DVBSX_INFO("\033[1;32;33m" "DVBS/Others Mode\n" "\033[m");
						//RTK_A_DVBSX_Write(pDemod, 0x18, 0x01, 0x00);//Set TS Parallel Mode to Enable TS Output
						DtvDemodTPOutEn(1);  //enable TS output
						pRealtekIDVBSX->m_dis_dvb2x_rolloff_mode = 0;
					}
				}
			}
		}
		DIS_dvbs2x_rolloff_cnt--;
	}
	#endif
#endif
	
	return TUNER_CTRL_OK;
}

/*----------------------------------------------------------------------
 * Func : KernelAutoTune
 *
 * Desc :
 *
 * Parm :
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_KernelAutoTune(DEMOD* pDemodKernel)
{
#ifdef CONFIG_RTK_KDRV_DEMOD_SCAN_THREAD_ENABLE
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	stRTK_LOCKING_STATUS sRTKLockingStatus;
	#ifdef DISABLE_DVBS2X
	RTK_DVBSx_SIGNAL_PARAMS pDVBSxParams;
	RTK_DVBS2_ROLL_OFF_TYPE RollOffType;
	static unsigned char DIS_dvbs2x_rolloff_cnt = 0;
	unsigned char ts_lock = 0, fec_lock = 0;
	#endif
	static unsigned char DPLL_patch_cnt = 0;
	static unsigned char TPO_patch_cnt = 0;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	if(pRealtekIDVBSX->m_acquire_sig_en)
	{
		REALTEK_A_DVBSX_AcquireSignalThread(pDemodKernel);
	}
	
	if(pRealtekIDVBSX->m_auto_tune_en && (pRealtekIDVBSX->m_info.dvbs.symbol_rate > RTK_DVBSX_LITE_SYMBOL_HZ))
	{
		if(DPLL_patch_cnt == 0)
		{
			DPLL_patch_cnt = 7;
			RTK_A_DVBSX_GetLockingStatus(pDemod, &sRTKLockingStatus);

			if(sRTKLockingStatus.u8FecLockFlag==1 && sRTKLockingStatus.u8TSLockFlag==1)	{
				u8stLockCnt++;
				if(u8stLockCnt>3) u8stLockCnt = 3;
			}
			else {
				u8stLockCnt = 0;
			}

			if(sRTKLockingStatus.u8TimingLockFlag)
			{
				//REALTEK_A_DVBSX_INFO("DPLLTrackingSpeedPatch in KernelAutoTune\n");
				RTK_A_DVBSX_DPLLTrackingSpeedPatch(pDemod);
				RTK_A_DVBSX_PhaseTrackingSpeedPatch(pDemod);
			}

			if(pRealtekIDVBSX->m_info.dvbs.symbol_rate > 17000000)
			{
				RTK_A_DVBSX_PilotTrackingAdjust(pDemod, pRealtekIDVBSX->m_info.dvbs.symbol_rate/1000);
			}
		}
		DPLL_patch_cnt--;

		if(TPO_patch_cnt == 0)
		{
			//REALTEK_A_DVBSX_INFO("TSBitRateCheck in KernelAutoTune\n");
			TPO_patch_cnt = 20;
			REALTEK_A_DVBSX_TSBitRateCheck(pDemodKernel);
			//RTK_A_DVBSX_SetLdpc1p6XMaxIteration(pDemod, pRealtekIDVBSX->m_info.dvbs.symbol_rate/1000, 0);
		}
		TPO_patch_cnt--;
	}
	
	#ifdef DISABLE_DVBS2X
	if(pRealtekIDVBSX->m_auto_tune_en)
	{
		if(DIS_dvbs2x_rolloff_cnt == 0)
		{
			DIS_dvbs2x_rolloff_cnt = 25;
			RTK_A_DVBSX_FECLockFlag(pDemod, &fec_lock);
			RTK_A_DVBSX_TSLockFlag(pDemod, &ts_lock);
			if(fec_lock == 1 || ts_lock == 1)
			{
				RTK_A_DVBSX_GetDVBSxSignalParams(pDemod, &pDVBSxParams);
				if(pDVBSxParams.DVBSxType == RTK_DEMOD_DVBS2X_MODE || pDVBSxParams.DVBSxType == RTK_DEMOD_DVBS2_OPTIONAL_MODE)
				{
					REALTEK_A_DVBSX_INFO("\033[1;32;34m" "KernelAutoTune - DVBS2 OPTIONAL & DVBS2X Force Unlock\n" "\033[m");
					//RTK_A_DVBSX_Write(pDemod, 0x18, 0x01, 0x01);//Set TS Serial Mode to Disable TS Output
					DtvDemodTPOutEn(0);  //disable TS output
					pRealtekIDVBSX->m_dis_dvb2x_rolloff_mode = 1;
				}
				else
				{
					if(pDVBSxParams.DVBSxType != RTK_DEMOD_DVBS_MODE)
					{
						//REALTEK_A_DVBSX_INFO("\033[1;32;33m" "DVBS2 Mode\n" "\033[m");
						RTK_A_DVBSX_GetDVBSxRollOff(pDemod, &RollOffType);
						if(RollOffType == RTK_DVBS2_ROLL_OFF_0p15 || RollOffType == RTK_DVBS2_ROLL_OFF_0p10 || RollOffType == RTK_DVBS2_ROLL_OFF_0p05)
						{
							REALTEK_A_DVBSX_INFO("\033[1;32;34m" "KernelAutoTune - Low Roll-Off Force Unlock\n" "\033[m");
							//RTK_A_DVBSX_Write(pDemod, 0x18, 0x01, 0x01);//Set TS Serial Mode to Disable TS Output
							DtvDemodTPOutEn(0);  //disable TS output
							pRealtekIDVBSX->m_dis_dvb2x_rolloff_mode = 1;
						}
						else
						{
							//RTK_A_DVBSX_Write(pDemod, 0x18, 0x01, 0x00);//Set TS Parallel Mode to Enable TS Output
							DtvDemodTPOutEn(1);  //enable TS output
							pRealtekIDVBSX->m_dis_dvb2x_rolloff_mode = 0;
						}
					}
					else
					{
						//REALTEK_A_DVBSX_INFO("\033[1;32;33m" "DVBS/Others Mode\n" "\033[m");
						//RTK_A_DVBSX_Write(pDemod, 0x18, 0x01, 0x00);//Set TS Parallel Mode to Enable TS Output
						DtvDemodTPOutEn(1);  //enable TS output
						pRealtekIDVBSX->m_dis_dvb2x_rolloff_mode = 0;
					}
				}
			}
		}
		DIS_dvbs2x_rolloff_cnt--;
	}
	#endif
#endif
	
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
int REALTEK_A_DVBSX_InspectSignal(DEMOD* pDemodKernel)
{
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : Activate
 *
 * Desc : Activate REALTEK_A_DVBSX Demod
 *
 * Parm : N/A
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_Activate(DEMOD* pDemodKernel, unsigned char force_rst)
{
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	pDemodKernel->Init(pDemodKernel);
	
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : SetTvMode
 *
 * Desc : Set Mode of REALTEK_A_ALL
 *
 * Parm : mode : RTK_DEMOD_MODE_DVBS2 : DVBS2 mode
 *               RTK_DEMOD_MODE_DVBC : DVBC mode
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_SetTvMode(DEMOD* pDemodKernel, TV_SYSTEM_TYPE mode)
{
	int ret = TUNER_CTRL_FAIL;
	
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	
	pDemodKernel->Lock(pDemodKernel);
	
	REALTEK_A_DVBSX_INFO("REALTEK_A_DVBSX::SetTvMode(%d)\n", mode);
	
	switch (mode){
	
	case TV_SYS_TYPE_DVBS2:
	
		REALTEK_A_DVBSX_INFO("Set to DVBS2\n");
		ret = pDemodKernel->Init(pDemodKernel);
	
		if(ret != TUNER_CTRL_OK){
			REALTEK_A_DVBSX_WARNING("Set REALTEK_A_DVBSX to DVBS2 mode failed\n");
			goto set_demod_mode_failed;
		}
	
		break;
	
	default:
		goto set_demod_mode_failed;
	}
	
	ret = TUNER_CTRL_OK;
	REALTEK_A_DVBSX_INFO("Set REALTEK_A_DVBSX to appointed DTV mode successed\n");
	
	
//end_proc:
	
	pDemodKernel->Unlock(pDemodKernel);
	return ret;
	
//--------------
set_demod_mode_failed:
	pDemodKernel->Unlock(pDemodKernel);
	
	return TUNER_CTRL_FAIL;
}


void REALTEK_A_DVBSX_Lock(DEMOD* pDemodKernel)
{
	//mutex_lock(&pDemodKernel->m_lock);
}


void REALTEK_A_DVBSX_Unlock(DEMOD* pDemodKernel)
{
	//mutex_unlock(&pDemodKernel->m_lock);
}


/*-----------------------------------------------------------------
 *     BASE_INTERFACE for REALTEK_A_DVBSX API
 *----------------------------------------------------------------*/
void ReleaseRealtekIDvbs2Driver(REALTEK_A_DVBSX_DRIVER_DATA *pDriver)
{
	if(pDvbsxShareMemory){
		DtvDemodFreeAllocMem((void *)pDvbsxShareMemory);
		pDvbsxShareMemory = NULL;
	}
	dvr_free(pDriver);
}


REALTEK_A_DVBSX_DRIVER_DATA* AllocRealtekIDvbSxDriver(
	COMM* pComm,
	unsigned char Addr,
	U32BITS CrystalFreq
)
{
	REALTEK_A_DVBSX_DRIVER_DATA* pDriver = (REALTEK_A_DVBSX_DRIVER_DATA*) dvr_malloc(
											sizeof(REALTEK_A_DVBSX_DRIVER_DATA) +
											sizeof(DVBSX_DEMOD_MODULE) +
											sizeof(I2C_BRIDGE_MODULE) +
											sizeof(BASE_INTERFACE_MODULE));
	
	if(pDriver){
		BASE_INTERFACE_MODULE* pBIF;
		DVBSX_DEMOD_MODULE* pDemod;
		memset(pDriver, 0, sizeof(REALTEK_A_DVBSX_DRIVER_DATA));
	
		pDriver->pDemod = (DVBSX_DEMOD_MODULE*)(((unsigned char*)pDriver) + sizeof(REALTEK_A_DVBSX_DRIVER_DATA));
		pDriver->pBaseInterface = (BASE_INTERFACE_MODULE*)(((unsigned char*)pDriver->pDemod) + sizeof(DVBSX_DEMOD_MODULE));
		pDriver->pI2CBridge = (I2C_BRIDGE_MODULE*)(((unsigned char*)pDriver->pBaseInterface) + sizeof(BASE_INTERFACE_MODULE));
		pDriver->DeviceAddr = Addr;
		pDriver->CrystalFreqHz = CrystalFreq;
	
		// Init Base IF
		pBIF = pDriver->pBaseInterface;
	
		pBIF->I2cReadingByteNumMax = RTK_DEMOD_BIF_TX_FIFO_DEPTH;
		pBIF->I2cWritingByteNumMax = RTK_DEMOD_BIF_RX_FIFO_DEPTH;
		pBIF->I2cRead = __realtek_a_rbus_i2c_read;
		pBIF->I2cWrite = __realtek_a_rbus_i2c_write;
		pBIF->WaitMs = __realtek_wait_ms;
		pBIF->SetUserDefinedDataPointer = base_interface_SetUserDefinedDataPointer;
		pBIF->GetUserDefinedDataPointer = base_interface_GetUserDefinedDataPointer;
		pBIF->SetUserDefinedDataPointer(pBIF, (void*)pComm);
	
		// Init Demod Driver
		BuildRealtekIDvbs2Module(&pDemod,
							pDriver->pDemod,
							pDriver->pBaseInterface,
							pDriver->pI2CBridge,
							Addr,
							CrystalFreq); // Spectrum mode is inverse.
	}
	
	return pDriver;
}


/*----------------------------------------------------------------------
 * Func : GetChannelInfo
 *
 * Desc : GetChannelInfo
 *        This function should be called after acquisition lock.
 *
 * Parm : 
 *
 *
 * Retn : TUNER_CTRL_OK / TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_GetChannelInfo(DEMOD* pDemodKernel, unsigned char* Standard, unsigned char* Modulation, unsigned char* CodeRate, unsigned char* PilotOnOff, unsigned char* Inverse)
{
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : ChangeStandard
 *
 * Desc :
 *
 * Parm :
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_ChangeStandard(DEMOD* pDemodKernel, unsigned int Mode)
{
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : CalDvbS2Ssi
 *
 * Desc :
 *
 * Parm :
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_CalDvbS2Ssi(DEMOD* pDemodKernel)
{
	unsigned int Ssi = 0;
	unsigned short IfAgcGain = 0;
	
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	pDemodKernel->Lock(pDemodKernel);
	
	RTK_A_DVBSX_SignalIFAGC(pDemod, &IfAgcGain);
	
	pDemodKernel->Unlock(pDemodKernel);
	
	Ssi = ((IfAgcGain)*100)/1024;
	
	if(Ssi > 100)
		Ssi = 100;
	else if(Ssi < 50)
		Ssi = 50;
	
	return Ssi;
}


/*----------------------------------------------------------------------
 * Func : CalDvbS2Sqi
 *
 * Desc :
 *
 * Parm :
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_CalDvbS2Sqi(DEMOD* pDemodKernel)
{
	int ret = TUNER_CTRL_OK;
	unsigned int CalHighBound = 0, CalMidBound = 0, CalLowBound = 0, Modulation = 0, CodeRate = 0, Sqi = 0, Snr = 0;
	unsigned char u8i = 0, ts_lock = 0, fec_lock = 0;
	
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	RTK_DVBSx_SIGNAL_PARAMS DVBSxParams;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*) pRealtekIDVBSX->m_private)->pDemod;
	
	pDemodKernel->Lock(pDemodKernel);
	
	ret = RTK_A_DVBSX_TSLockFlag(pDemod, &ts_lock);
	ret = RTK_A_DVBSX_FECLockFlag(pDemod, &fec_lock);
	
	if(ts_lock == 1 || fec_lock == 1)
	{
		RTK_A_DVBSX_GetDVBSxSignalParams(pDemod, &DVBSxParams);
		if(DVBSxParams.DVBSxModu == RTK_DVBSx_MODU_QPSK)
			Modulation = DVBS2_MOD_QPSK;
		else if(DVBSxParams.DVBSxModu == RTK_DVBSx_MODU_8PSK)
			Modulation = DVBS2_MOD_8PSK;
		
		if(DVBSxParams.DVBS2CodeRate == RTK_DVBS2_CODE_RATE_1_2)
			CodeRate = DVBS2_CR_1_2;
		else if(DVBSxParams.DVBS2CodeRate == RTK_DVBS2_CODE_RATE_2_3)
			CodeRate = DVBS2_CR_2_3;
		else if(DVBSxParams.DVBS2CodeRate == RTK_DVBS2_CODE_RATE_3_4)
			CodeRate = DVBS2_CR_3_4;
		else if(DVBSxParams.DVBS2CodeRate == RTK_DVBS2_CODE_RATE_5_6)
			CodeRate = DVBS2_CR_5_6;
		else if(DVBSxParams.DVBS2CodeRate == RTK_DVBS2_CODE_RATE_3_5)
			CodeRate = DVBS2_CR_3_5;
		else if(DVBSxParams.DVBS2CodeRate == RTK_DVBS2_CODE_RATE_4_5)
			CodeRate = DVBS2_CR_4_5;
		else if(DVBSxParams.DVBS2CodeRate == RTK_DVBS2_CODE_RATE_8_9)
			CodeRate = DVBS2_CR_8_9;
		else if(DVBSxParams.DVBS2CodeRate == RTK_DVBS2_CODE_RATE_9_10)
			CodeRate = DVBS2_CR_9_10;
		
		RTK_A_DVBSX_GetDVBSxSNR(pDemod, &Snr);
		pRealtekIDVBSX->m_dvbsx_snr = Snr;
		
		for(u8i = 0; u8i < DVBS2_SIG_QUALITY_TABLE_SIZE; u8i++)
		{
			if(_gsDvbs2SigQualityMappingTable[u8i].Modulation == Modulation &&
			_gsDvbs2SigQualityMappingTable[u8i].CodeRate == CodeRate)
			{
				CalHighBound = _gsDvbs2SigQualityMappingTable[u8i].sBound.HighBound;
				CalMidBound = _gsDvbs2SigQualityMappingTable[u8i].sBound.MidBound;
				CalLowBound = _gsDvbs2SigQualityMappingTable[u8i].sBound.LowBound;
				break;
			}
		}

		if(pRealtekIDVBSX->m_dvbsx_snr >= CalHighBound)
		{
			Sqi = 100;
		}
		else if(pRealtekIDVBSX->m_dvbsx_snr >= CalMidBound)
		{
			Sqi = ((((pRealtekIDVBSX->m_dvbsx_snr - CalMidBound)*100) / (CalHighBound - CalMidBound))*90)/100 + 10;
		}
		else if(pRealtekIDVBSX->m_dvbsx_snr >= CalLowBound)
		{
			Sqi = ((((pRealtekIDVBSX->m_dvbsx_snr - CalLowBound)*100) / (CalMidBound - CalLowBound))*10)/100;
		}
		else
		{
			Sqi = 0;
		}
	}
	else
	{
		Sqi = 0;
	}
	
	pDemodKernel->Unlock(pDemodKernel);
	
	return Sqi;
}


/*----------------------------------------------------------------------
 * Func : CalDvbS2XSsi
 *
 * Desc :
 *
 * Parm :
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_CalDvbS2XSsi(DEMOD* pDemodKernel)
{
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : CalDvbS2XSqi
 *
 * Desc :
 *
 * Parm :
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_CalDvbS2XSqi(DEMOD* pDemodKernel)
{
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : CalDvbSSsi
 *
 * Desc :
 *
 * Parm :
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_CalDvbSSsi(DEMOD* pDemodKernel)
{
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : CalDvbSSqi
 *
 * Desc :
 *
 * Parm :
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_CalDvbSSqi(DEMOD* pDemodKernel)
{
	int ret = TUNER_CTRL_OK;
	unsigned int CalHighBound = 0, CalMidBound = 0, CalLowBound = 0, Modulation = 0, CodeRate = 0, Sqi = 0, Snr = 0;
	unsigned char u8i = 0, ts_lock = 0, fec_lock = 0;
	
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	RTK_DVBSx_SIGNAL_PARAMS DVBSxParams;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*) pRealtekIDVBSX->m_private)->pDemod;
	
	pDemodKernel->Lock(pDemodKernel);
	
	ret = RTK_A_DVBSX_TSLockFlag(pDemod, &ts_lock);
	ret = RTK_A_DVBSX_FECLockFlag(pDemod, &fec_lock);
	
	if(ts_lock == 1 || fec_lock == 1)
	{
		RTK_A_DVBSX_GetDVBSxSignalParams(pDemod, &DVBSxParams);
		Modulation = 0;
		
		if(DVBSxParams.DVBSCodeRate == RTK_DVBS_CODE_RATE_1_2)
			CodeRate = DVBS_CR_1_2;
		else if(DVBSxParams.DVBSCodeRate == RTK_DVBS_CODE_RATE_2_3)
			CodeRate = DVBS_CR_2_3;
		else if(DVBSxParams.DVBSCodeRate == RTK_DVBS_CODE_RATE_3_4)
			CodeRate = DVBS_CR_3_4;
		else if(DVBSxParams.DVBSCodeRate == RTK_DVBS_CODE_RATE_5_6)
			CodeRate = DVBS_CR_5_6;
		else if(DVBSxParams.DVBSCodeRate == RTK_DVBS_CODE_RATE_7_8)
			CodeRate = DVBS_CR_7_8;
		
		RTK_A_DVBSX_GetDVBSxSNR(pDemod, &Snr);
		pRealtekIDVBSX->m_dvbsx_snr = Snr;
		
		for(u8i = 0; u8i < DVBS_SIG_QUALITY_TABLE_SIZE; u8i++)
		{
			if(_gsDvbSSigQualityMappingTable[u8i].CodeRate == CodeRate)
			{
				CalHighBound = _gsDvbSSigQualityMappingTable[u8i].sBound.HighBound;
				CalMidBound = _gsDvbSSigQualityMappingTable[u8i].sBound.MidBound;
				CalLowBound = _gsDvbSSigQualityMappingTable[u8i].sBound.LowBound;
				break;
			}
		}
	
		if(pRealtekIDVBSX->m_dvbsx_snr >= CalHighBound)
		{
			Sqi = 100;
		}
		else if(pRealtekIDVBSX->m_dvbsx_snr >= CalMidBound)
		{
			//SNR shift 0.6dB to Calculate
			Sqi = (((((pRealtekIDVBSX->m_dvbsx_snr + 6) - CalMidBound)*100) / (CalHighBound - CalMidBound))*90)/100 + 10;
		}
		else if(pRealtekIDVBSX->m_dvbsx_snr >= CalLowBound)
		{
			Sqi = ((((pRealtekIDVBSX->m_dvbsx_snr - CalLowBound)*100) / (CalMidBound - CalLowBound))*10)/100;
		}
		else
		{
			Sqi = 0;
		}
	}
	else
	{
		Sqi = 0;
	}
	
	pDemodKernel->Unlock(pDemodKernel);
	
	return Sqi;
}


/*----------------------------------------------------------------------
 * Func : SetDiseqcContinue22kOnOff
 *
 * Desc : Set Diseqc 22K always ON/OFF
 *
 * Parm : 
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_SetDiseqcContinue22kOnOff(DEMOD* pDemodKernel, unsigned int OnOff)
{
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	REALTEK_A_DVBSX_INFO("REALTEK_A_DVBSX_SetDiseqcContinue22kOnOff %d\n", OnOff);
	
	//Direct link to diseqc driver, not here
	pDemod->SetDiseqcContinue22kOnOff(OnOff);
	
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : SetDiseqcCmd
 *
 * Desc : Set Diseqc command
 *
 * Parm : 
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_SetDiseqcCmd(DEMOD* pDemodKernel, unsigned int Length, unsigned char *Data)
{
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	//Direct link to diseqc driver, not here
	pDemod->SetDiseqcCmd(Length, Data);
	
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : SetDiseqcToneBurst
 *
 * Desc : Set Un-Modulated Tone Burst(SA)
 *
 * Parm : 
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_SetDiseqcToneBurst(DEMOD* pDemodKernel, DISEQC_TONE_MODE mode)
{
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	//Direct link to diseqc driver, not here
	pDemod->SetDiseqcToneBurst(mode);
	
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : SetDiseqcUnModToneBurst
 *
 * Desc : Set Un-Modulated Tone Burst(SA)
 *
 * Parm : 
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_SetDiseqcUnModToneBurst(DEMOD* pDemodKernel)
{
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	//Direct link to diseqc driver, not here
	pDemod->SetDiseqcUnModToneBurst();
	
	return TUNER_CTRL_OK;
}


/*----------------------------------------------------------------------
 * Func : SetDiseqcModToneBurst
 *
 * Desc : Set Modulated Tone Burst(SB)
 *
 * Parm : 
 *
 * Retn : TUNER_CTRL_OK/TUNER_CTRL_FAIL
 *----------------------------------------------------------------------*/
int REALTEK_A_DVBSX_SetDiseqcModToneBurst(DEMOD* pDemodKernel)
{
	REALTEK_A_DVBSX* pRealtekIDVBSX;
	DVBSX_DEMOD_MODULE* pDemod;
	pRealtekIDVBSX = (REALTEK_A_DVBSX*)(pDemodKernel->private_data);
	pDemod = ((REALTEK_A_DVBSX_DRIVER_DATA*)pRealtekIDVBSX->m_private)->pDemod;
	
	//Direct link to diseqc driver, not here
	pDemod->SetDiseqcModToneBurst();
	
	return TUNER_CTRL_OK;
}
