/**

@file

@brief   RTK_A DVB-T2 demod module definition

One can manipulate RTK_A DVB-T2 demod through RTK_A DVB-T2 module.
RTK_A DVB-T2 module is derived from DVB-T2 demod module.

*/

#include <linux/slab.h>
#include <rtk_kdriver/io.h>

#include "demod_rtk_common.h"
#include "dvbtx_demod_rtk_a.h"
#include "dvbtx_demod_rtk_a_base.h"
#include "rbus/atb_demod_tps_reg.h"
#include "rbus/atb_demod_top_reg.h"
#include "rbus/atb_demod_ce_t2_2_reg.h"



stRtkaDvbtxConfig gstRTK_A_DVBTX_DvbConfig;
unsigned char gRTK_A_DVBTX_PowerOnInited = 0;
static unsigned char u8sDVBTxBandwidth = 8;

/*If version number > 2.55, should add another reserved register, current only one register used. */
unsigned char         u8RTK_A_DVBTX_SoftwareVersion = 109;     /*SW guide version = ui8ATBM783xSoftwareVersion, just record it */
unsigned char         u8RTK_A_DVBTX_SoftwareSEVersion = 7;     /*SE version number, just record for special edition */


static unsigned char Rtkadvbtx8mFilter[] = {
	0x0A, 0x09, 0x00,
	0x0A, 0x0A, 0x00,
	0x0A, 0x0B, 0x00,
	0x0A, 0x0C, 0x00,
	0x0A, 0x0D, 0x00,
	0x0A, 0x0E, 0x00,
	0x0A, 0x0F, 0x00,
	0x0A, 0x10, 0x00,
	0x0A, 0x11, 0x00,
	0x0A, 0x12, 0x00,
	0x0A, 0x13, 0x1F,
	0x0A, 0x14, 0x00,
	0x0A, 0x15, 0x0E,
	0x0A, 0x16, 0x00,
	0x0A, 0x17, 0xF0,
	0x0A, 0x18, 0x0F,
	0x0A, 0x19, 0xD9,
	0x0A, 0x1A, 0x0F,
	0x0A, 0x1B, 0xDA,
	0x0A, 0x1C, 0x0F,
	0x0A, 0x1D, 0xF7,
	0x0A, 0x1E, 0x0F,
	0x0A, 0x1F, 0x22,
	0x0A, 0x20, 0x00,
	0x0A, 0x21, 0x3F,
	0x0A, 0x22, 0x00,
	0x0A, 0x23, 0x38,
	0x0A, 0x24, 0x00,
	0x0A, 0x25, 0x08,
	0x0A, 0x26, 0x00,
	0x0A, 0x27, 0xC7,
	0x0A, 0x28, 0x0F,
	0x0A, 0x29, 0x9C,
	0x0A, 0x2A, 0x0F,
	0x0A, 0x2B, 0xAC,
	0x0A, 0x2C, 0x0F,
	0x0A, 0x2D, 0xF9,
	0x0A, 0x2E, 0x0F,
	0x0A, 0x2F, 0x5F,
	0x0A, 0x30, 0x00,
	0x0A, 0x31, 0xA0,
	0x0A, 0x32, 0x00,
	0x0A, 0x33, 0x86,
	0x0A, 0x34, 0x00,
	0x0A, 0x35, 0x07,
	0x0A, 0x36, 0x00,
	0x0A, 0x37, 0x56,
	0x0A, 0x38, 0x0F,
	0x0A, 0x39, 0xDB,
	0x0A, 0x3A, 0x0E,
	0x0A, 0x3B, 0x00,
	0x0A, 0x3C, 0x0F,
	0x0A, 0x3D, 0xFA,
	0x0A, 0x3E, 0x0F,
	0x0A, 0x3F, 0xA9,
	0x0A, 0x40, 0x01,
	0x0A, 0x41, 0x95,
	0x0A, 0x42, 0x03,
	0x0A, 0x43, 0x1A,
	0x0A, 0x44, 0x05,
	0x0A, 0x45, 0xAE,
	0x0A, 0x46, 0x05,
	0x0A, 0xfb, 0x01, //8M
	0x0A, 0xb9, 0x00, //8M
	0x0A, 0x08, 0x15 //24M 8M bandwidth
};

/***************************default register value group for dvbt and dvbt2 *********************************************/
static short i16DvbxBandWidthFilter[10][31] = {
	{ -7, -9, -10, -9, -5, 1, 10, 20, 29, 35, 37, 33, 21, 1, -24, -53, -80, -102, -112, -106, -80, -32, 38, 127, 230, 339, 445, 541, 616, 664, 681 },                                            /*1.7MHz 24*/
	{8, 12, 16, 20, 21, 21, 18, 11, 2, -11, -25, -41, -56, -69, -78, -81, -75, -61, -37, -2, 43, 97, 158, 223, 289, 352, 410, 459, 497, 520, 528 },                                             /*1.7MHz 30.4*/
	{ -4, -7, -4, 5, 13, 15, 6, -12, -28, -28, -8, 25, 51, 48, 11, -46, -87, -80, -13, 83, 150, 133, 15, -158, -286, -259, -16, 415, 915, 1313, 1465},                                           /*5MHz 24*/
	{10, 0, -10, -18, -17, -7, 10, 27, 36, 30, 8, -25, -54, -66, -51, -9, 49, 100, 119, 89, 9, -99, -195, -235, -178, -10, 257, 574, 874, 1090, 1168},                                          /*5MHz 30.4*/
	{ -2, 5, 8, 1, -10, -16, -7, 12, 27, 21, -9, -40, -43, -5, 50, 75, 37, -47, -113, -93, 21, 152, 184, 51, -187, -350, -241, 211, 878, 1473, 1711},                                            /*6MHz 24*/
	{ -9, 2, 13, 17, 10, -6, -24, -31, -21, 7, 37, 53, 41, 0, -52, -85, -76, -18, 66, 134, 140, 63, -78, -220, -279, -184, 86, 486, 912, 1237, 1359},                                            /*6MHz 30.4*/
	{6, -2, -9, -7, 7, 17, 9, -14, -28, -12, 25, 44, 15, -41, -65, -18, 65, 95, 21, -102, -140, -23, 163, 218, 25, -291, -399, -26, 779, 1607, 1957},                                           /*7MHz 24*/
	{9, -4, -15, -15, -1, 18, 28, 18, -10, -38, -43, -16, 31, 67, 60, 5, -68, -109, -76, 24, 134, 173, 90, -95, -275, -310, -98, 352, 911, 1372, 1550},                                        /*7MHz 30.4*/
	{ -5, -1, 8, 8, -5, -16, -7, 17, 24, -3, -35, -25, 26, 54, 12, -61, -67, 22, 106, 61, -88, -156, -17, 199, 201, -108, -417, -233, 590, 1594, 2047},                                          /*8MHz 24*/
	{0, 0, 0, 0, 0, -15, -17, 3, 27, 31, 4, -37, -53, -22, 40, 81, 55, -31, -112, -110, -1, 143, 196, 75, -170, -353, -263, 187, 871, 1489, 1737}                                              /*8MHz 30.4*/
};

static unsigned char ui8DvbxFilterMod[] = { 2, 2, 2, 2, 2, 2, 2, 2,  1,  0};
/* ui16Alpha [] = {69,54,69	,54	,69	,54	,69	,54	,69	,54}; //using default setting PHY*/
/*
static int  i32DvbxInvFSADC [] = {87381, 68985	, 87381	, 68985	, 87381	, 68985	, 87381	, 68985	, 87381	, 68985};
*/

static int  i32DvbxFSADC    [] = {1572864, 1992300, 1572864, 1992300, 1572864, 1992300, 1572864, 1992300, 1572864, 1992300}; /*U(22,16)*/

static unsigned char ui8DvbxBWMode[] = {4, 4, 3, 3, 2, 2, 1, 1, 0, 0};
/*unsigned int  i32DvbxRateRatio[] = {1745855027,2211416367,563714458,714038313,469762048,595031927,402653184,510027366,352321536,446273946};*/


/*Pre defined DVB-T signal parameters string, you can define any customized string on requirement */
/*
static const char *gStrDvbTFFTMode[]         = { "2K", "8K", "RESERVED", "UNKNOWN"};
static const char *gStrDvbTGuardInterval[]   = { "1/32", "1/16", "1/8", "1/4", "UNKNOWN"};
static const char *gStrDvbTHierarchy[]       = { "NONE", "a=1", "a=2", "a=4",  "UNKNOWN"};
static const char *gStrDvbTConstellation[]   = { "QPSK", "16QAM", "64QAM",  "UNKNOWN"};
static const char *gStrDvbTCodeRate[]        = { "1/2", "2/3", "3/4", "5/6", "7/8", "RESERVED5", "RESERVED6", "UNKNOWN"};
*/

/*Pre defined DVB-T2 signal parameters string, you can define any customized string on requirement */
/*
static const char *gStrDvbT2FFTMode[]       = { "1K", "2K", "4K", "8K", "16K", "32K" , "RESERVED", "UNKNOWN"};
static const char *gStrDvbT2GuardInterval[] = { "1/32", "1/16", "1/8", "1/4", "1/128", "19/128", "19/256", "UNKNOWN"};
static const char *gStrDvbT2Version[]       = { "V1.11", "V1.21", "V1.31",  "UNKNOWN"};
static const char *gStrDvbT2CodeRate[]      = { "1/2", "3/5", "2/3", "3/4", "4/5", "5/6", "1/3", "2/5", "UNKNOWN"};
static const char *gStrDvbT2Constellation[] = { "QPSK", "16QAM", "64QAM", "256QAM", "RESERVED4", "RESERVED5", \
												"RESERVED6", "RESERVED7", "UNKNOWN"
											  };
static const char *gStrDvbT2SisoMiso[]      = { "SISO", "MISO", "NON-T2",  "RESERVED3", "RESERVED4", "RESERVED5", \
												"RESERVED6", "RESERVED7", "UNKNOWN"
											  };

*/

/**

@brief   RTK_A DVB-T2 demod module builder

Use BuildRtkaDvbtModule() to build RTK_A DVB-T2 module, set all module function pointers with the corresponding
functions, and initialize module private variables.


@param [in]   ppDemod                      Pointer to RTK_A DVB-T2 demod module pointer
@param [in]   pDvbtDemodModuleMemory       Pointer to an allocated DVB-T2 demod module memory
@param [in]   pBaseInterfaceModuleMemory   Pointer to an allocated base interface module memory
@param [in]   pI2cBridgeModuleMemory       Pointer to an allocated I2C bridge module memory
@param [in]   DeviceAddr                   RTK_A DVB-T2 I2C device address
@param [in]   CrystalFreqHz                RTK_A DVB-T2 crystal frequency in Hz
@param [in]   TsInterfaceMode              RTK_A DVB-T2 TS interface mode for setting
@param [in]   IfFreqHz                     RTK_A DVB-T2 IF frequency in Hz for setting
@param [in]   SpectrumMode                 RTK_A DVB-T2 Spectrum mode for setting


@note
    -# One should call BuildRtkaIsdbtModule() to build RTK_A DVB-T2 module before using it.

*/
void
BuildRtkaDvbtxModule(
	DVBTX_DEMOD_MODULE **ppDemod,
	DVBTX_DEMOD_MODULE *pDvbtDemodModuleMemory,
	BASE_INTERFACE_MODULE *pBaseInterfaceModuleMemory,
	I2C_BRIDGE_MODULE *pI2cBridgeModuleMemory,
	unsigned char DeviceAddr,
	unsigned int CrystalFreqHz
)
{
	DVBTX_DEMOD_MODULE *pDemod;
	RTK_A_DVBTX_EXTRA_MODULE *pExtra;


	// Set demod module pointer,
	*ppDemod = pDvbtDemodModuleMemory;

	// Get demod module.
	pDemod = *ppDemod;

	// Get demod extra module.
	pExtra = &(pDemod->Extra.RtkaDvbtx);


	pDemod->pBaseInterface = pBaseInterfaceModuleMemory;
	pDemod->pI2cBridge     = pI2cBridgeModuleMemory;


	// Set demod type.
	pDemod->DemodType = DVBTX_DEMOD_TYPE_REALTEK_A;

	// Set demod crystal frequency in Hz.
	pDemod->CrystalFreqHz = CrystalFreqHz;

	pDemod->GetCrystalFreqHz        = realtek_a_dvbtx_GetCrystalFreqHz;
	// Set demod module manipulating function pointers with default functions.
	//pDemod->GetBandwidthMode        = dvbtx_demod_default_GetBandwidthMode;



	// Set demod module manipulating function pointers with particular functions.
	pDemod->Initialize              = realtek_a_dvbtx_Initialize;
	pDemod->SetBandwidthMode        = realtek_a_dvbtx_SetBandwidthMode;

	pDemod->IsSignalLocked          = realtek_a_dvbtx_IsSignalLocked;

	pDemod->IsSignalSyncLocked  = realtek_a_dvbtx_IsDVBTxSignalSyncLock;
	pDemod->IsDvbtSyncLocked  = realtek_a_dvbtx_IsDVBTSignalSyncLock;
	pDemod->IsDvbt2SyncLocked  = realtek_a_dvbtx_IsDVBT2SignalSyncLock;
	pDemod->IsDvbt2L1Locked  = realtek_a_dvbtx_IsDVBT2L1Lock;

	pDemod->GetSignalParamsLockFlag  = realtek_a_dvbtx_GetDVBTxSignalParamsLockFlag;

	pDemod->GetSignalStrength       = realtek_a_dvbtx_GetSignalStrength;
	pDemod->GetLayerSignalQuality   = realtek_a_dvbtx_GetLayerSignalQuality;

	pDemod->GetBer             = realtek_a_dvbtx_GetBer;
	pDemod->GetPer             = realtek_a_dvbtx_GetPer;
	pDemod->GetSnrDb                = realtek_a_dvbtx_GetSnrDb;

	pDemod->SetAgcPol                = realtek_a_dvbtx_SetAgcPol;
	pDemod->GetIfAgc                = realtek_a_dvbtx_GetIfAgc;

	pDemod->GetTrOffsetPpm          = realtek_a_dvbtx_GetTrOffsetPpm;
	pDemod->GetCrOffsetHz           = realtek_a_dvbtx_GetCrOffsetHz;

	pDemod->GetCellId              = realtek_a_dvbtx_GetCellId;

	// 20120207 added by kevin
	pDemod->SetTsInterfaceMode      = realtek_a_dvbtx_SetTsInterfaceMode;
	pDemod->SetTSOutputOnOff      = realtek_a_dvbtx_SetTSOutputOnOff;
	pDemod->SetIfFreqHz             = realtek_a_dvbtx_SetIfFreqHz;
	pDemod->SetSpectrumMode         = realtek_a_dvbtx_SetSpectrumMode;
	pDemod->SetTHierarchy            = realtek_a_dvbtx_SetTHierarchy;
	pDemod->SetT2PlpID            = realtek_a_dvbtx_SetT2PlpID;

	pDemod->GetDVBTxMode              = realtek_a_dvbtx_GetDVBTxMode;
	pDemod->GetSignalParamsT2              = realtek_a_dvbtx_GetSignalParamsT2;
	pDemod->GetSignalParamsT              = realtek_a_dvbtx_GetSignalParamsT;
	pDemod->GetTsThroughput              = realtek_a_dvbtx_TSBitAccu;
	pDemod->SetCellIdCheckOn              = realtek_a_dvbtx_CellIDOnOff;
	pDemod->MonitorT2TxID              = realtek_a_dvbtx_CheckT2TxIDErrSignal;
	pDemod->MonitorT2DyStaticCH              = realtek_a_dvbtx_CheckDyStaicChl;
	// 20180616 add byKen
	pDemod->MonitorT2CheckbyStaicChPwrChange              = realtek_a_dvbtx_CheckbyStaicChPwrChange;


	return;
}


/**

@see   DVBT_DEMOD_FP_GET_CRYSTAL_FREQ_HZ

*/
void
realtek_a_dvbtx_GetCrystalFreqHz(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *pCrystalFreqHz
)
{
	// Get demod crystal frequency in Hz from demod module.
	*pCrystalFreqHz = pDemod->CrystalFreqHz;
	return;
}



/**

@see   DVBT_DEMOD_FP_INITIALIZE

*/
int
realtek_a_dvbtx_Initialize(
	DVBTX_DEMOD_MODULE *pDemod
)
{
	RTK_A_DVBTX_STATUS status = RTK_A_DVBTX_SUCCESSFUL;


	/*Demodulator TS interface  setting	*/
	gstRTK_A_DVBTX_DvbConfig.stMPEGTSMode.ui8TSTransferType           = RTK_A_DVBTX_TS_PARALLEL_MODE;
	gstRTK_A_DVBTX_DvbConfig.stMPEGTSMode.ui8OutputEdge               = RTK_A_DVBTX_TS_OUTPUT_FALLING_EDGE;
	gstRTK_A_DVBTX_DvbConfig.stMPEGTSMode.ui8SPIClockConstantOutput   = RTK_A_DVBTX_TS_CLOCK_CONST_OUTPUT;
	gstRTK_A_DVBTX_DvbConfig.stMPEGTSMode.ui8TSSPIMSBSelection        = RTK_A_DVBTX_TS_SPI_MSB_ON_DATA_BIT7;
	gstRTK_A_DVBTX_DvbConfig.stMPEGTSMode.ui8TSSSIOutputSelection     = RTK_A_DVBTX_TS_SSI_OUTPUT_ON_DATA_BIT0;
	gstRTK_A_DVBTX_DvbConfig.stMPEGTSMode.ui8DVBTxCTSClockVariableMode = RTK_A_DVBTX_TS_CLOCK_VARIABLE;             /*TS clock frequency fixed or variable with real TS rate*/
	/* 	//Demodulator DVB-T/T2 setting*/
	gstRTK_A_DVBTX_DvbConfig.DvbTxTunerInput.ui8IQSwapMode = RTK_A_DVBTX_INPUT_IQ_NORMAL;
	gstRTK_A_DVBTX_DvbConfig.DvbTxTunerInput.ui8TxIFAgcPolarity = RTK_A_DVBTX_IF_AGC_POLARITY_NORMAL;
	gstRTK_A_DVBTX_DvbConfig.DvbTxTunerInput.u32IFFrequencyKHz = 5000; /*5MHz, Here the unit is KHz,  Should be equal to the input of Tuner IF frequency.*/
	gstRTK_A_DVBTX_DvbConfig.DvbTxTunerInput.ui8BandwidthType = RTK_A_DVBTX_BANDWIDTH_8MHZ;



	/*Demodulator Clock Setting           */
	gstRTK_A_DVBTX_DvbConfig.ui32DemodClkKHz = 24000; /*KHz, 24MHz, or 30400 for 30.4MHz*/

	status |= RTK_A_DVBTX_Write(pDemod, 0x00, 0x03, 1); /*Reset*/

	/* 	RTK_A_DVBTX_SetSysClkClock();*/
	//status |= RTK_A_DVBTX_SetSysClkClockDVBTx();
	status   |=  RTK_A_DVBTX_Write(pDemod, 0x01, 0x0c, 0x00);
	//tv_osal_msleep(1); /*Delay 1 millisecond.*/
	mdelay(1);


	status |= RTK_A_DVBTX_Write(pDemod, 0x00, 0x04, 0x00); /*set register to default value,  RTK_A_DVBTX_PowerOnInit should be called once after power up */

	gRTK_A_DVBTX_PowerOnInited = 1;


	status |= RTK_A_DVBTX_Write(pDemod, 0x08, 0x22, 0x23);	//default set as base profile, 2018.05 05
	//status |= RTK_A_DVBTX_Write(pDemod, 0x28, 0x2e, 0x07); //tps_done should wait cell id decoded

	//reset for plp id change
	//rtd_maskl(ATB_DEMOD_TIME_EN_SYNC_TIMEOUT_RST_reg, ~ATB_DEMOD_TIME_EN_SYNC_TIMEOUT_RST_en_mac_blk_rst_all_mask, ATB_DEMOD_TIME_EN_SYNC_TIMEOUT_RST_en_mac_blk_rst_all(1));

	rtd_maskl(ATB_DEMOD_TOP_GPO5_REG_reg, ~ATB_DEMOD_TOP_GPO5_REG_gpo5_reg_mask, ATB_DEMOD_TOP_GPO5_REG_gpo5_reg(0x2)); //data delay for sd ram to sync buf  // need check setting at Mark2

	rtd_maskl(ATB_DEMOD_CE_T2_2_PARAMS_AVERAGE_CP_EN_reg, ~ATB_DEMOD_CE_T2_2_PARAMS_AVERAGE_CP_EN_params_average_cp_en_mask, ATB_DEMOD_CE_T2_2_PARAMS_AVERAGE_CP_EN_params_average_cp_en(0x01));//enable CP average

	status |= RTK_A_DVBTX_Write(pDemod, 0x00, 0x04, 1); /*config done*/


	//return status;

	if (status == RTK_A_DVBTX_SUCCESSFUL)
		return FUNCTION_SUCCESS;
	else
		return FUNCTION_ERROR;


}



/**

@see   DVBT_DEMOD_FP_SET_TS_INTERFACE_MODE

*/
int
realtek_a_dvbtx_SetTsInterfaceMode(
	DVBTX_DEMOD_MODULE *pDemod,
	RTK_A_DVBTX_MPEGTSMode stTSMode
)
{
	unsigned char ui8SerialEnabled = 0x0; /*parallel clock out */
	unsigned char ui8TSOutputEdegeMode;
	unsigned char ui8TsReg = 0;
	unsigned char ui8TsClockMode = stTSMode.ui8SPIClockConstantOutput;
	RTK_A_DVBTX_STATUS status = RTK_A_DVBTX_SUCCESSFUL;

	ui8TSOutputEdegeMode = stTSMode.ui8OutputEdge;

	/*0x00: SPI data align with rising edge of TS CLK; 0x01: SPI data align with falling edge of TS CLK*/
	status |= RTK_A_DVBTX_Write(pDemod, 0x18, 0x02, ui8TSOutputEdegeMode);
	status |= RTK_A_DVBTX_Write(pDemod, 0x18, 0x03, ui8TsClockMode); /*either free running or only active when data are valid*/


	status |= RTK_A_DVBTX_Write(pDemod, 0x26, 0x24, 0x05);
	status |= RTK_A_DVBTX_Write(pDemod, 0x26, 0x25, 0xff);
	status |= RTK_A_DVBTX_Write(pDemod, 0x26, 0x26, 0x7); /*either free running or only active when data are valid*/

	if (stTSMode.ui8TSTransferType == RTK_A_DVBTX_TS_SERIAL_MODE) {
		ui8SerialEnabled = 0x01;    /*enable serial TS mode */

		/*SSI_sync_hold_cycle = 0x00; //0x00:1bit sync; 0x01: 8bit sync*/
		/*0: serial sync  holds 1 cycle; 1: serial sync holds 8 cycles*/
		/*RTK_A_DVBTX_Write(pDemod, 0x17,0xb0, SSI_sync_hold_cycle);  */

		/*0:disable serial mode; 1: enable serial mode */
		status |= RTK_A_DVBTX_Write(pDemod, 0x18, 0x01, ui8SerialEnabled);
		if (stTSMode.ui8TSSSIOutputSelection == RTK_A_DVBTX_TS_SSI_OUTPUT_ON_DATA_BIT0) {
			status |= RTK_A_DVBTX_Write(pDemod, 0x18, 0x04, 0x00);
		} else {
			status |= RTK_A_DVBTX_Write(pDemod, 0x18, 0x04, 0x01);
		}

		status |= RTK_A_DVBTX_Write(pDemod, 0x26, 0x27, 0x00); /*TS delay clock*/
		status |= RTK_A_DVBTX_Write(pDemod, 0x26, 0x29, 0x03); /*Disable TS delay*/

	} else {
		ui8SerialEnabled = 0x0; /*parallel clock output */
		/*0: disable serial mode; 1:enable serial mode */
		status |= RTK_A_DVBTX_Write(pDemod, 0x18, 0x01, ui8SerialEnabled);
		if (stTSMode.ui8TSSPIMSBSelection == RTK_A_DVBTX_TS_SPI_MSB_ON_DATA_BIT7) {
			status |= RTK_A_DVBTX_Write(pDemod, 0x18, 0x04, 0x00);
		} else {
			status |= RTK_A_DVBTX_Write(pDemod, 0x18, 0x04, 0x01);
		}

		status |= RTK_A_DVBTX_Write(pDemod, 0x26, 0x27, 0x00);
		status |= RTK_A_DVBTX_Write(pDemod, 0x26, 0x29, 0x03);
	}

	if (gstRTK_A_DVBTX_DvbConfig.stMPEGTSMode.ui8DVBTxCTSClockVariableMode == RTK_A_DVBTX_TS_CLOCK_VARIABLE) {
		status |= RTK_A_DVBTX_Write(pDemod, 0x00, 0x3c, 0x03);
		status |= RTK_A_DVBTX_Read(pDemod, 0x06, 0x06, &ui8TsReg);
		ui8TsReg &= 0x7f; //ts_n_fb
		status |= RTK_A_DVBTX_Write(pDemod, 0x18, 0x9e, ui8TsReg);

		realtek_a_dvbtx_SetTSVarThresh(pDemod, ui8TsReg);

		status |= RTK_A_DVBTX_Read(pDemod, 0x06, 0x0a, &ui8TsReg);
		ui8TsReg &= 0x1f; //ts_m_fb, divider
		status |= RTK_A_DVBTX_Write(pDemod, 0x18, 0x9f, ui8TsReg);
		status |= RTK_A_DVBTX_Write(pDemod, 0x18, 0x9c, 0x95); //TS rate clock,More Margin 2017.02.03
	} else {
		status |= RTK_A_DVBTX_Write(pDemod, 0x00, 0x3c, 0x00);
	}


	return (status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}

int realtek_a_dvbtx_SetTSOutputOnOff(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned char ui8TSOnOff
)
{
	RTK_A_DVBTX_STATUS status = RTK_A_DVBTX_SUCCESSFUL;
	if (ui8TSOnOff == RTK_A_DVBTX_TS_OFF) {
		//serial
		/*0:disable serial mode; 1: enable serial mode */
		status |= RTK_A_DVBTX_Write(pDemod, 0x18, 0x01, 0x01);
	} else {
		//parallel
		/*0:disable serial mode; 1: enable serial mode */
		status |= RTK_A_DVBTX_Write(pDemod, 0x18, 0x01, 0x00);
	}
	return (status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}


/**

@see   DVBT_DEMOD_FP_SET_IF_FREQ_HZ

*/
int
realtek_a_dvbtx_SetIfFreqHz(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int IfFreqHz
)
{
	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;
	char  ui8FreqLifHigh = 0;
	unsigned int ui32FreqLifLow = 0;
	RTK_A_DVBTX_64Data  i64Data, i64Data1, i64Rst;
	unsigned int ui32TunerIFKHz = 5000;
	unsigned int ui32ParamsIFFreq = 0;
	unsigned int CrystalFreqHz = 0;


	ui32TunerIFKHz = (unsigned int)(IfFreqHz / 1000);
	// Get demod crystal frequency in Hz.
	pDemod->GetCrystalFreqHz(pDemod, &CrystalFreqHz);
	gstRTK_A_DVBTX_DvbConfig.ui32DemodClkKHz = (unsigned int)(CrystalFreqHz / 1000);


	/* 	ui32TunerIFKHz = 5000;*/
	/*freq Calculate*/
	if (ui32TunerIFKHz < gstRTK_A_DVBTX_DvbConfig.ui32DemodClkKHz) { /*use equation dbTunerIF/dbDemodClk*/
		RTK_A_DVBTX_64Mult(&i64Data, RTK_A_DVBTX_FixPow(2, 30), ui32TunerIFKHz * 64);
		i64Data1.ui32High = 0;
		i64Data1.ui32Low = gstRTK_A_DVBTX_DvbConfig.ui32DemodClkKHz;
		i64Rst  = RTK_A_DVBTX_64DivReturn64(i64Data, i64Data1);
	} else { /*use equation (dbTunerIF-dbDemodClk)/dbDemodClk*/
		RTK_A_DVBTX_64Mult(&i64Data, RTK_A_DVBTX_FixPow(2, 30), (ui32TunerIFKHz - gstRTK_A_DVBTX_DvbConfig.ui32DemodClkKHz) * 64);
		i64Data1.ui32High = 0;
		i64Data1.ui32Low = gstRTK_A_DVBTX_DvbConfig.ui32DemodClkKHz;
		i64Rst  = RTK_A_DVBTX_64DivReturn64(i64Data, i64Data1);
	}
	ui32FreqLifLow = i64Rst.ui32Low;/*64bit system */
	ui8FreqLifHigh = (unsigned char)(i64Rst.ui32High);

	/*start write register*/
	ui8Status = RTK_A_DVBTX_Write(pDemod, 0x0a, 0x00, (unsigned char)(ui32FreqLifLow));
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a, 0x01, (unsigned char)(ui32FreqLifLow >> 8));
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a, 0x02, (unsigned char)(ui32FreqLifLow >> 16));
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a, 0x03, (unsigned char)(ui32FreqLifLow >> 24));

	/*ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x0a,0x04,&ui8Tmp);*/
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a, 0x04, ui8FreqLifHigh);

	ui32ParamsIFFreq = (ui32TunerIFKHz * 32768 + 250) / 500; /*round(IF_MHz*Pow(2,16)) //Add 2014.3.24*/
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a, 0xbc, (unsigned char)(ui32ParamsIFFreq));
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a, 0xbd, (unsigned char)(ui32ParamsIFFreq >> 8));
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a, 0xbe, (unsigned char)(ui32ParamsIFFreq >> 16));
	return (ui8Status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}


/**

@see   DVBT_DEMOD_FP_SET_SPECTRUM_MODE

*/
int
realtek_a_dvbtx_SetSpectrumMode(
	DVBTX_DEMOD_MODULE *pDemod,
	int SpectrumMode
)
{

	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;

	gstRTK_A_DVBTX_DvbConfig.DvbTxTunerInput.ui8IQSwapMode = (SpectrumMode == SPECTRUM_NORMAL) ? RTK_A_DVBTX_INPUT_IQ_NORMAL : RTK_A_DVBTX_INPUT_IQ_SWAP;

	if (gstRTK_A_DVBTX_DvbConfig.DvbTxTunerInput.ui8IQSwapMode) {
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x03, 0x02, 0x1);
	} else {
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x03, 0x02, 0x0);
	}


	return (ui8Status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;

}





/**

@see   DVBT_DEMOD_FP_SET_BANDWIDTH_MODE

*/
int
realtek_a_dvbtx_SetBandwidthMode(
	DVBTX_DEMOD_MODULE *pDemod,
	int BandwidthMode
)
{
	//ui8Status |= RTK_A_DVBTX_SetBandWidthPara(gstRTK_A_DVBTX_DvbConfig);

	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;
	unsigned char ui8Tmp = 0;
	unsigned char ui8Iter = 0, ui8Numerator = 0, ui8Denominator = 0;
	unsigned int ui32RatioRate = 0;
	unsigned char u8InputBandwidthType = 0;
	RTK_A_DVBTX_64Data i64Data, i64Data1;

	u8InputBandwidthType =  BandwidthMode;


	gstRTK_A_DVBTX_DvbConfig.DvbTxTunerInput.ui8BandwidthType = u8InputBandwidthType; /*default */



	ui8Iter = gstRTK_A_DVBTX_DvbConfig.DvbTxTunerInput.ui8BandwidthType * 2 + (((int)gstRTK_A_DVBTX_DvbConfig.ui32DemodClkKHz / 1000) == 24 ? 0 : 1);
	if (ui8Iter > 9) {
		ui8Iter = 8; //set default value for not valid setting.
	}
	u8sDVBTxBandwidth = gstRTK_A_DVBTX_DvbConfig.DvbTxTunerInput.ui8BandwidthType;


	if (gstRTK_A_DVBTX_DvbConfig.DvbTxTunerInput.ui8BandwidthType != RTK_A_DVBTX_BANDWIDTH_8MHZ) { /*Default is for 8MHz Bandwidth.*/
		for (ui8Tmp = 0; ui8Tmp < 31; ui8Tmp++) {
			ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a, 0x09 + ui8Tmp * 2, (unsigned char)i16DvbxBandWidthFilter[ui8Iter][ui8Tmp]);
			ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a, 0x09 + ui8Tmp * 2 + 1, (unsigned char)(i16DvbxBandWidthFilter[ui8Iter][ui8Tmp] >> 8));

		}

	}
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a, 0xfb, ui8DvbxFilterMod[ui8Iter]);

	/* 		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a,0x05,(unsigned char)i32DvbxRateRatio[ui8Iter]);*/
	/* 		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a,0x06,(unsigned char)(i32DvbxRateRatio[ui8Iter]>>8));*/
	/* 		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a,0x07,(unsigned char)(i32DvbxRateRatio[ui8Iter]>>16));*/
	/* 		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a,0x08,(unsigned char)(i32DvbxRateRatio[ui8Iter]>>24));*/
	/*24MHz using default registers*/

	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a, 0xbf, (unsigned char)i32DvbxFSADC[ui8Iter]);
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a, 0xc0, (unsigned char)(i32DvbxFSADC[ui8Iter] >> 8));
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a, 0xc1, (unsigned char)(i32DvbxFSADC[ui8Iter] >> 16));

	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a, 0xb9, ui8DvbxBWMode[ui8Iter]);
	if (gstRTK_A_DVBTX_DvbConfig.DvbTxTunerInput.ui8BandwidthType == RTK_A_DVBTX_BANDWIDTH_8MHZ) {
		ui8Status |= RTK_A_DVBTX_WriteRegArray(pDemod, Rtkadvbtx8mFilter, sizeof(Rtkadvbtx8mFilter) / sizeof(unsigned char));
	}


	if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_1D7MHZ) {
		ui8Numerator = 71;
		ui8Denominator = 131;

	} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_5MHZ) {
		ui8Numerator = 7;
		ui8Denominator = 40;
	} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_6MHZ) {
		ui8Numerator = 7;
		ui8Denominator = 48;
	} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_7MHZ) {
		ui8Numerator = 1;
		ui8Denominator = 8;
	} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_8MHZ) {
		ui8Numerator = 7;
		ui8Denominator = 64;
	} else {
		ui8Numerator = 7;
		ui8Denominator = 80;
	}


	/*
	Rate_ratio = floor(FS_ADC/(2*BW)*2^28)   = FS_ADC/BW*2^27 = FS_ADC*ui8Numerator*2^27/ui8Denominator;
	BW is bandwidth��8M: 64/7e6; BW = 1/T
	*/

	RTK_A_DVBTX_64Mult(&i64Data, gstRTK_A_DVBTX_DvbConfig.ui32DemodClkKHz * ui8Numerator, RTK_A_DVBTX_FixPow(2, 27));
	RTK_A_DVBTX_64Mult(&i64Data1, ui8Denominator, 1);

	i64Data = RTK_A_DVBTX_64DivReturn64(i64Data, i64Data1); /*1000;//clock is KHz*/
	RTK_A_DVBTX_64Mult(&i64Data1, 1000, 1);
	ui32RatioRate = RTK_A_DVBTX_64Div(i64Data, i64Data1); /*1000;//clock is KHz*/
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a, 0x05, (unsigned char)ui32RatioRate);
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a, 0x06, (unsigned char)(ui32RatioRate >> 8));
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a, 0x07, (unsigned char)(ui32RatioRate >> 16));
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0a, 0x08, (unsigned char)(ui32RatioRate >> 24));

	/*ratio rate*/



	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x18, 0x65, 0x01);
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x17, 0xf8, 0x08);

	//ui8Status |= RTK_A_DVBTX_SetAutoPLPT2();
	ui8Status =  RTK_A_DVBTX_Write(pDemod, 0x24, 0x03, 0x00);    /* Disable PLP selection, for single PLP or Multi-PLP, always receive first Data PLP */


	if (realtek_a_dvbtx_T2TxIDErrSignalRestore(pDemod) != FUNCTION_SUCCESS)
		return FUNCTION_ERROR;


	if (realtek_a_dvbtx_DyStaitcCh_InitSet(pDemod) != FUNCTION_SUCCESS)
		return FUNCTION_ERROR;


	//ui8Status |= RTK_A_DVBTX_SetDVBT2BaseProfile();  //RTK_A_DVBTX_SetDVBTxMode default set as base profile and auto PLP.
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x00, 0x05, 0x01);  /*Reset DSP*/

	/*
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x88, 0x02, 0x08);
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x88, 0x03, 0x08);
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x88, 0x04, 0x08);
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x88, 0x05, 0x08);
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x88, 0x0a, 0x07);
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x88, 0x06, 0x08);  //Default
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x88, 0x07, 0x1a);  //Default
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x88, 0x08, 0x2c);  //Default

		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x17, 0xd8, 0x1f);

	*/
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x00, 0x05, 0x00);  /*Start DSP*/


	return (ui8Status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;

}


/**

@see   DVBT_DEMOD_FP_IS_SIGNAL_LOCKED

*/
int
realtek_a_dvbtx_IsSignalLocked(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	RTK_A_DVBTX_STATUS status = RTK_A_DVBTX_SUCCESSFUL;
	unsigned char u8BlockFEC = 0;
	unsigned char  u8DVBTxMode = 0, u8TPSDone = 0, u8DvbtFecLock = 0;
	unsigned char u8Read_data = 0;
	int i = 0, cnt = 0;

	*pAnswer = 0;

	if (realtek_a_dvbtx_GetDVBTxMode(pDemod, &u8DVBTxMode) != FUNCTION_SUCCESS)
		return FUNCTION_ERROR;
	//DVBT mode
	if (u8DVBTxMode == RTK_A_DVBTX_DVBT_MODE) {
		for (i = 0; i <= 4; i++) {                                  //check 5 time locked
			status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x2c, &u8TPSDone);
			status |= RTK_A_DVBTX_Read(pDemod, 0x95, 0x37, &u8DvbtFecLock);

			if ((u8DvbtFecLock & 0x01) && (u8TPSDone & 0x01)) { /*TPS done and FEC lock, DVB-T locked*/
				cnt++;
				if (cnt >= 5) {
					*pAnswer = 1;
					return (status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
				}
			}
			//tv_osal_msleep(1);
			mdelay(1);
		}

		status |= RTK_A_DVBTX_Read(pDemod, 0x17, 0xD9, &u8Read_data); //Check FEC RST bit OFF
		if (((u8Read_data & 0x08) != 0x08) && (u8TPSDone & 0x01)) { /*TPS done and FEC Reset Off , DVB-T locked*/
			*pAnswer = 1;
			return (status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
		}
	}

	//DVBT2 mode
	if (u8DVBTxMode == RTK_A_DVBTX_DVBT2_MODE) {
		status |= RTK_A_DVBTX_Read(pDemod, 0x17, 0xEC, &u8BlockFEC);
		if (u8BlockFEC & 0x01) {  //DVB-T2 FEC locked
			*pAnswer = 1;
			return (status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
		}
	}

	return status;
}




/********************************************************************************
function    :RTK_A_DVBTX_DVBTxSignalSyncLockFlag
parameter   :pAnswer, 1--Locked;0--Unlocked
return      :Operate  Status:RTK_A_DVBTX_SUCCESSFUL is Normal and other is abnormal.
Description:Call this function to Check DVB-T or DVB-T2 signal sync lock status.
            1: Sync locked indicate DVB-T or DVB-T2 signal is detected. will wait for later decoding.
            0: All return 0, No DVB-T or DVB-T2 signal be detected(not sync or no signal).
            This function checks signal sync lock status.
			If signal checked lock status == 1. else lock status == 0.
Using sample:
From 0 to 350ms
Call this function every 100ms
if all get lock  status value is 0, we can judge there is no DVB-T_T2 signal on current channel.
*********************************************************************************/
int
realtek_a_dvbtx_IsDVBTxSignalSyncLock(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pAnswer
)

{
	RTK_A_DVBTX_STATUS status = RTK_A_DVBTX_SUCCESSFUL;
	//unsigned char u8SyncT = 0, u8T2DetDone = 0, u8cfol = 0, u8cfoh = 0;
	unsigned char u8SyncT = 0, u8T2DetDone = 0;

	//unsigned char u8DvbReceivingMode = RTK_A_DVBTX_DVB_UNKNOWN;

	*pAnswer  = 0; /*Assuming not locked*/

	status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x6D, &u8SyncT);
	status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x71, &u8T2DetDone);

	//status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x87, &u8cfol);
	//status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x88, &u8cfoh);
	/*
	a)	b0=t2_det_done_hold(88 71)
	    Not used b1=��88H87H>0��||(88H88H>0)
	b)88H 71H bit1-> p1_search_done_hold
	c)	b2=t_det_done_reg(88 6D)
	*/
	/*
	if((u8T2DetDone&0x02) == 0x02)
	{
	//	u8DvbReceivingMode = RTK_A_DVBTX_DVBT2_MODE;
		*pAnswer	= 1;
	}*/
	if ((u8T2DetDone & 0x01) == 0x01) {
		//u8DvbReceivingMode = RTK_A_DVBTX_DVBT2_MODE;
		*pAnswer	= 1;

	}
	if (u8SyncT == 1) {
		//	u8DvbReceivingMode = RTK_A_DVBTX_DVBT_MODE;
		*pAnswer = 1;
	}
	return (status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}

/********************************************************************************
function    :RTK_A_DVBTX_DVBTxSignalSyncLockFlag
parameter   :pAnswer, 1--Locked;0--Unlocked
return      :Operate  Status:RTK_A_DVBTX_SUCCESSFUL is Normal and other is abnormal.
Description:Call this function to Check DVB-T or DVB-T2 signal sync lock status.
            1: Sync locked indicate DVB-T or DVB-T2 signal is detected. will wait for later decoding.
            0: All return 0, No DVB-T or DVB-T2 signal be detected(not sync or no signal).
            This function checks signal sync lock status.
			If signal checked lock status == 1. else lock status == 0.
Using sample:
From 0 to 350ms
Call this function every 100ms
if all get lock  status value is 0, we can judge there is no DVB-T_T2 signal on current channel.
*********************************************************************************/
int
realtek_a_dvbtx_IsDVBTSignalSyncLock(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pAnswer
)

{
	RTK_A_DVBTX_STATUS status = RTK_A_DVBTX_SUCCESSFUL;
	unsigned char u8SyncT = 0;

	//unsigned char u8DvbReceivingMode = RTK_A_DVBTX_DVB_UNKNOWN;

	*pAnswer  = 0; /*Assuming not locked*/

	status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x6D, &u8SyncT);

	if (u8SyncT == 1) {
		//	u8DvbReceivingMode = RTK_A_DVBTX_DVBT_MODE;
		*pAnswer = 1;
	}
	return (status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}


/********************************************************************************
function    :RTK_A_DVBTX_DVBTxSignalSyncLockFlag
parameter   :pAnswer, 1--Locked;0--Unlocked
return      :Operate  Status:RTK_A_DVBTX_SUCCESSFUL is Normal and other is abnormal.
Description:Call this function to Check DVB-T or DVB-T2 signal sync lock status.
            1: Sync locked indicate DVB-T or DVB-T2 signal is detected. will wait for later decoding.
            0: All return 0, No DVB-T or DVB-T2 signal be detected(not sync or no signal).
            This function checks signal sync lock status.
			If signal checked lock status == 1. else lock status == 0.
Using sample:
From 0 to 350ms
Call this function every 100ms
if all get lock  status value is 0, we can judge there is no DVB-T_T2 signal on current channel.
*********************************************************************************/
int
realtek_a_dvbtx_IsDVBT2SignalSyncLock(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pAnswer
)

{
	RTK_A_DVBTX_STATUS status = RTK_A_DVBTX_SUCCESSFUL;
	unsigned char u8T2DetDone = 0;

	//unsigned char u8DvbReceivingMode = RTK_A_DVBTX_DVB_UNKNOWN;

	*pAnswer  = 0; /*Assuming not locked*/

	status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x71, &u8T2DetDone);

	if ((u8T2DetDone & 0x01) == 0x01) {
		//u8DvbReceivingMode = RTK_A_DVBTX_DVBT2_MODE;
		*pAnswer	= 1;

	}

	return (status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}


/********************************************************************************
function    :RTK_A_DVBTX_DVBT2L1Lock
parameter   :pAnswer, 1--Locked;0--Unlocked
return      :Operate  Status:RTK_A_DVBTX_SUCCESSFUL is Normal and other is abnormal.
Description:Call this function to Check DVB-T or DVB-T2 signal sync lock status.
            1: Sync locked indicate DVB-T or DVB-T2 signal is detected. will wait for later decoding.
            0: All return 0, No DVB-T or DVB-T2 signal be detected(not sync or no signal).
            This function checks signal sync lock status.
			If signal checked lock status == 1. else lock status == 0.
Using sample:
From 0 to 350ms
Call this function every 100ms
if all get lock  status value is 0, we can judge there is no DVB-T_T2 signal on current channel.
*********************************************************************************/
int
realtek_a_dvbtx_IsDVBT2L1Lock(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pAnswer
)

{
	RTK_A_DVBTX_STATUS status = RTK_A_DVBTX_SUCCESSFUL;
	unsigned char u8T2L1Pre = 0, u8T2L1Pst = 0;

	//unsigned char u8DvbReceivingMode = RTK_A_DVBTX_DVB_UNKNOWN;

	*pAnswer  = 0; /*Assuming not locked*/

	status |= RTK_A_DVBTX_Read(pDemod, 0x0b, 0x01, &u8T2L1Pre);
	status |= RTK_A_DVBTX_Read(pDemod, 0x0b, 0x02, &u8T2L1Pst);

	if (((u8T2L1Pre & 0x01) == 0x01) && ((u8T2L1Pst & 0x01) == 0x01)) {
		//u8DvbReceivingMode = RTK_A_DVBTX_DVBT2_MODE;
		*pAnswer	= 1;

	}

	return (status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}


/********************************************************************************
function    :RTK_A_DVBTX_DVBTxSignalParamsLockFlag
parameter   :pAnswer, 1--Locked;0--Unlocked
return      :Operate  Status:RTK_A_DVBTX_SUCCESSFUL is Normal and other is abnormal.
Description:Call this function to Check DVB-T or DVB-T2 signal params lock status.
            1: locked indicate DVB-T or DVB-T2 signal is detected.
            0: No DVB-T or DVB-T2 signal be detected(not sync or no signal).
            This function checks signal params lock status.
			If signal checked lock status == 1. else lock status == 0.
Using sample:
From 0--900ms(concerns weak or unstable signal.)
Call this function every 50ms
if all get lock  status value is 0, we can judge there is no DVB-T_T2 signal on current channel.
*********************************************************************************/

int
realtek_a_dvbtx_GetDVBTxSignalParamsLockFlag(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pAnswer
)

{
	RTK_A_DVBTX_STATUS status = RTK_A_DVBTX_SUCCESSFUL;
	unsigned char u8SyncT = 0, u8SyncT2 = 0;
	unsigned char u8L1PreCRCRight = 0, u8L1PostCRCRight = 0;

	unsigned char u8DvbReceivingMode = RTK_A_DVBTX_DVB_UNKNOWN;

	unsigned char u8DvbtFecLock = 0;
	//unsigned char u8TPSDone = 0;

	*pAnswer  = 0; /*Assuming not locked*/
	status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x6C, &u8SyncT2);
	status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x6D, &u8SyncT);

	if (u8SyncT2 == 1) {
		u8DvbReceivingMode = RTK_A_DVBTX_DVBT2_MODE;
	}
	if (u8SyncT == 1) {
		u8DvbReceivingMode = RTK_A_DVBTX_DVBT_MODE;
	}
	if (u8DvbReceivingMode == RTK_A_DVBTX_DVBT2_MODE) { /*Sync State indicate DVB-T2 signal may be received*/
		status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x01, &u8L1PreCRCRight);
		status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x02, &u8L1PostCRCRight);
		if ((u8L1PostCRCRight & 0x01) || (u8L1PreCRCRight != 0x00)) { /*CRC right indicate T2 signal can be decoded*/
			*pAnswer = 1;
		}
	}
	/*Check only if DVB-T Sync detected*/
	if (u8DvbReceivingMode == RTK_A_DVBTX_DVBT_MODE) { /*Sync State indicate DVB-T signal may be received*/
		//status |=RTK_A_DVBTX_Read(pDemod, 0x28, 0x2c, &u8TPSDone);
		status |= RTK_A_DVBTX_Read(pDemod, 0x95, 0x37, &u8DvbtFecLock);
		if ((u8DvbtFecLock & 0x01)) {
			*pAnswer = 1;
		}
	}
	return (status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}

/**

@see   DVBT_DEMOD_FP_GET_SIGNAL_STRENGTH

*/
int
realtek_a_dvbtx_GetSignalStrength(
	DVBTX_DEMOD_MODULE *pDemod,
	int TxRSSIdBm,
	unsigned int *pSignalStrength
)

{
	RTK_A_DVBTX_STATUS status = RTK_A_DVBTX_SUCCESSFUL;
	int i32Prec = 0, i32Prel = 0; /*Received RSSI */
	unsigned char u8DVBTxMode = 0, u8Tmp = 0, u8Modu = 0, u8CR = 0, u8Alpha = 0, u8index = 0, u8SSI = 0;

	static int i32DvbtPref[] = {
		-93, -91, -90, -89, -88, /*QPSK*/
			-87, -85, -84, -83, -82, /*16QAM*/
			-82, -80, -78, -77, -76 /*64QAM*/
		};

	static int i32Dvbt2Pref[] = {
		-96, -95, -94, -93, -92, -92, /*QPSK*/
			-91, -89, -88, -87,	-86, -86, /*16QAM*/
			-86, -85, -83, -82, -81, -80, /*64QAM*/
			-82, -80, -78, -76,	-75, -74  /*256QAM*/
		};

	i32Prel = -100;
	i32Prec = (int)TxRSSIdBm;

	if (realtek_a_dvbtx_GetDVBTxMode(pDemod, &u8DVBTxMode) != FUNCTION_SUCCESS)
		return FUNCTION_ERROR;


	if (u8DVBTxMode == RTK_A_DVBTX_DVBT_MODE) {
		status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x22, &u8Tmp); /*modulation*/
		u8Modu = u8Tmp & 0x03;

		status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x24, &u8Tmp); /*HP code rate*/
		u8CR =  u8Tmp & 0x07;

		status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x23, &u8Tmp); /*alpha*/
		u8Alpha = u8Tmp & 0x07;

		if ((u8Alpha > 0) && (u8Alpha < 4)) {
			/*default HP profile, when Hierarchy used and LP selected, 0x282b = 1*/
			status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x2b, &u8Tmp); /*DVB-T profile*/
			if ((u8Tmp & 0x01) == 0x01) {
				status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x25, &u8Tmp); /*LP code rate*/
				u8CR    = u8Tmp & 0x07;
				u8Modu = 0;
			}
		}
		u8index = u8Modu * 5 + u8CR;
		if (u8index < 15) {
			i32Prel = i32Prec - i32DvbtPref[u8index];
		}

	}

	if (u8DVBTxMode == RTK_A_DVBTX_DVBT2_MODE) {
		/*code rate:  000:1/2, 001:3/5, 010:2/3, 011:3/4, 100:4/5, 101:5/6  */
		status |= RTK_A_DVBTX_Read(pDemod, 0x0C, 0x38, &u8CR);
		/*QPSK, 16QAM, 64QAM, 256QAM*/
		status |= RTK_A_DVBTX_Read(pDemod, 0x0C, 0x2F, &u8Modu);

		u8index = u8Modu * 6 + u8CR;
		if (u8index < 24) {
			i32Prel = i32Prec - i32Dvbt2Pref[u8index];
		}
	}

	/*
	SSI = 0 if Prel < -15dB
	SSI = (2/3) * (Prel + 15) if -15 dB �� Prel < 0dB
	SSI = 4 * Prel + 10 if 0 dB �� Prel < 20 dB
	SSI = (2/3) * (Prel - 20) + 90 if 20 dB �� Prel < 35 dB
	SSI = 100 if Prel �� 35 dB
	Prel = Prec - Pref
	*/

	if (i32Prel >= 35) {
		u8SSI = 100;
	} else if ((i32Prel >= 20) && (i32Prel < 35)) {
		u8SSI = (unsigned char)(((i32Prel - 20) * 2) / 3 + 90);
	} else if ((i32Prel >= 0) && (i32Prel < 20)) {
		u8SSI  = (unsigned char)(4 * i32Prel + 10);
	} else if ((i32Prel >= -15) && (i32Prel < 0)) {
		u8SSI = (unsigned char)(((i32Prel + 15) * 2) / 3);
	} else {
		u8SSI = 0;
	}
	if (i32Prec < -90) {
		u8SSI = 0;
	}
	*pSignalStrength = u8SSI;

	return (status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}


/**

@see   DVBT_DEMOD_FP_GET_LAYER_SIGNAL_QUALITY

*/
int
realtek_a_dvbtx_GetLayerSignalQuality(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *pSignalQuality
)

{
	unsigned char ui8CodeRate = 0, ui8PlpModu = 0;
	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;
	unsigned int ui32SqiValue = 0;
	unsigned int ui32Ddvbt2BER = 0;
	unsigned int ui32BerSqi = 1;
	unsigned int ui32SnrX10 = 0, ui32SnrRel = 0;
	unsigned char u8L1S1 = 0, u8LiteUsed = 0;
	unsigned int ui32BERNum = 0;
	unsigned int ui32BERDen = 0;
	RTK_A_DVBTX_64Data i64Data, i64Data1;


	/*00 QPSK,  1:16QAM,  2:64QAM,  3:Reserved*/
	unsigned char ui8Constellation = 0, ui8Value = 0, u8TSLockFlag = 0, u8TmpGI = 0, u8TmpFFTMode = 0;
	/*Hierarchy information, */
	/*0:Non hierarchical, 1:��= 1,  2:��= 2,  3:��= 4*/
	unsigned char ui8HierarchyInfo = 0;
	unsigned char ui8LpSelect = 0;
	/*3bits 000:1/2,    001:2/3,      010:3/4,  011: 5/6,  100: 7/8,  101: reserved*/
	unsigned char ui8HPCodeRate = 0, ui8LPCodeRate = 0;
	unsigned char ui8BerSqi = 0;
	int i32WinLength;
	int i32CNnordigx10 = 0;
	int i32CNreceviedx10 = 0;
	int i32CNrelx10 = 0;
	unsigned int ui32Tmp = 0;

	unsigned int ui32dPreRSBER = 0;

	static unsigned char sU8SQI[3] = {0};
	unsigned char U8SQItmp;

	/*CN of NorDig Profile 1 for Non Hierarchical signal*/
	static  unsigned int NordigDVBTdBx10[3][5] = {
		/* 1/2,   2/3,   3/4,   5/6,   7/8 */
		{51, 69, 79, 89, 97},         /* QPSK */
		{108, 131, 146, 156, 160},    /* 16QAM */
		{165, 187, 202, 216, 225}     /* 64QAM */
	};

	/*CN of NorDig Profile 1 for Hierarchical HP signal*/
	static  unsigned int NordigHierHpDVBTdBx10[3][2][5] = {
		/* alpha = 1 */
		{  /* 1/2,   2/3,    3/4,    5/6,    7/8 */
			{91,   120,  136,  150,  166},    /* LP = 16QAM */
			{109,  141,  157,  194,  206}     /* LP = 64QAM */
		},/* alpha = 2 */
		{  /* 1/2,   2/3,    3/4,    5/6,    7/8 */
			{68,    91,  104,  119,  127},    /* LP = 16QAM */
			{85,   110,  128,  150,  160}     /* LP = 64QAM */
		},/* alpha = 4 */
		{  /* 1/2,   2/3,    3/4,    5/6,    7/8 */
			{58,    79,  91,   103,  121},    /* LP = 16QAM */
			{80,    93,  116,  130,  129}     /* LP = 64QAM */
		}
	};

	/*CN of NorDig Profile 1 for Hierarchical LP signal*/
	static  unsigned int NordigHierLpDVBTdBx10[3][2][5] = {
		/* alpha = 1 */
		{  /* 1/2,   2/3,    3/4,    5/6,    7/8 */
			{125,  143,  153,  163,  169},    /* 16QAM */
			{167,  191,  209,  225,  237}     /* 64QAM */
		}, /* alpha = 2 */
		{  /* 1/2,   2/3,    3/4,    5/6,    7/8 */
			{150,  172,  184,  191,  201},    /* 16QAM */
			{185,  212,  236,  247,  259}     /* 64QAM */
		}, /* alpha = 4 */
		{  /* 1/2,   2/3,    3/4,    5/6,    7/8 */
			{195,  214,  225,  237,  247},    /* 16QAM */
			{219,  242,  256,  269,  278}     /* 64QAM */
		}
	};


	static  unsigned int ui32PSnrNordigX10[4][8] = {
		/*   1/2,   3/5,   2/3,   3/4,    4/5,    5/6 ; Lite:1/3 2/5*/
		{35, 47, 56, 66, 72, 77, 10,  20},         /* QPSK   */
		{87, 101, 114, 125, 133, 138, 40, 50},     /* 16-QAM */
		{130, 148, 162,  177, 187, 194, 130, 130}, /* 64-QAM */
		{170, 194, 208, 229, 243, 251, 170, 170},  /* 256-QAM */
	};


	unsigned char u8DVBTxMode = 0;

	if (realtek_a_dvbtx_GetDVBTxMode(pDemod, &u8DVBTxMode) != FUNCTION_SUCCESS)
		return FUNCTION_ERROR;

	if (realtek_a_dvbtx_GetSnrDb(pDemod, &ui32SnrX10) != FUNCTION_SUCCESS) /*snr�Ѿ�*10*/
		return FUNCTION_ERROR;

	if (realtek_a_dvbtx_GetBer(pDemod, &ui32BERNum, &ui32BERDen) != FUNCTION_SUCCESS)
		return FUNCTION_ERROR;


	if (ui32BERDen != 0) {
		RTK_A_DVBTX_64Mult(&i64Data, ui32BERNum, 1000000000);
		RTK_A_DVBTX_64Mult(&i64Data1, ui32BERDen, 1);
		ui32Ddvbt2BER = RTK_A_DVBTX_64Div(i64Data, i64Data1);

	} else
		ui32Ddvbt2BER = 1000000000;



	if (u8DVBTxMode == RTK_A_DVBTX_DVBT2_MODE) {

		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x05,  &u8L1S1);
		if (((u8L1S1 & 0x07) == 0x03) || ((u8L1S1 & 0x07) == 0x04)) {
			u8LiteUsed = 1;
		}
		/*Value:code rate
		000:1/2, 001:3/5, 010:2/3, 011:3/4, 100:4/5, 101:5/6
		*/
		ui8Status = RTK_A_DVBTX_Read(pDemod, 0x0C, 0x38, &ui8CodeRate);
		/*QPSK, 16QAM, 64QAM, 256QAM*/
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x0C, 0x2F, &ui8PlpModu);
		if ((ui8CodeRate < 6) && (ui8PlpModu < 4) && (u8LiteUsed == 0)) {
			ui32SnrRel = ui32PSnrNordigX10[ui8PlpModu][ui8CodeRate];
		} else if ((ui8CodeRate < 8) && (ui8PlpModu < 4) && (u8LiteUsed == 1)) {
			ui32SnrRel = ui32PSnrNordigX10[ui8PlpModu][ui8CodeRate]; /*For T2 Lite*/
		} else {
			*pSignalQuality = 0;
			return FUNCTION_ERROR; /*error*/
		}


		//if(ui32SnrX10>6)
		//{
		//  ui32SnrX10 = ui32SnrX10 - 3; //0.3dB calibration
		//}

		if (ui32SnrX10 > ui32SnrRel + 30) { /*SNR >3dB + C/Nrel, SQI = 100*/
			ui32SqiValue = 100;
		} else if ((ui32SnrX10  <= ui32SnrRel + 30) && (ui32SnrX10  >= ui32SnrRel - 30)) {
			/*
			BER_SQI = 0             if BER>1.0E-4
			BER_SQI = (100/15)      if 1.0E-4 <= BER <= 1.0E-7
			BER_SQI = (100/6)       if BER<1.0E-7
			*/
			//ui8Status |= RTK_A_DVBTX_GetDVBT2BER(&ui32Ddvbt2BER);


			/* 		 if( ui32Ddvbt2BER >0.000101)*/
			if (ui32Ddvbt2BER >= 101000) {
				ui32BerSqi = 0;
				ui32SqiValue = ui32BerSqi;
				*pSignalQuality = ui32SqiValue;
				return (ui8Status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
			}
			/*       else if((ui32Ddvbt2BER < 0.0001)&&(ui32Ddvbt2BER >0.0000001))*/
			else if ((ui32Ddvbt2BER < 101000) && (ui32Ddvbt2BER > 100))

			{
				/* 			 ui32BerSqi = 100.0/15;*/
				ui32BerSqi = 15;
			} else {
				/* 			 ui32BerSqi = 100.0/6;*/
				ui32BerSqi = 6;
			}
			/*SQI = (CR_received - CR_nordig + 3) * BER_SQI*/
			/* 		 ui32SqiValue = ((ui32SnrX10 - ui32SnrRel + 30.0)/10) * ui32BerSqi;*/
			if (ui32BerSqi != 0) {
				ui32SqiValue = (((ui32SnrX10 + 30) - ui32SnrRel) * 10) / ui32BerSqi;
			}
		} else {
			ui32SqiValue = 0;
		}
		if ((ui32SqiValue >= 5) && (ui32SqiValue < 10)) {
			ui32SqiValue = ui32SqiValue - 2;
		}
		if (ui32SqiValue < 5) {
			ui32SqiValue = 0;
		}

		if ((ui32SqiValue < 30) && (ui32SqiValue >= 10) && (ui8CodeRate == 3)) {
			ui32SqiValue = ui32SqiValue - ((80 - ui32SqiValue) / 10);
		}
		if (ui8Status) {
			*pSignalQuality = 0;
			return FUNCTION_ERROR;
		}
		*pSignalQuality = (unsigned char)(ui32SqiValue);

	}


	if (u8DVBTxMode == RTK_A_DVBTX_DVBT_MODE) {

		/* 	*quality_per100 = 0; //default return value.*/
		*pSignalQuality = 0;  /*Default set 0*/
		ui8Status = RTK_A_DVBTX_Read(pDemod, 0x28, 0x22, &ui8Constellation);
		ui8Constellation &= 0x03;
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x23, &ui8HierarchyInfo);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x24, &ui8HPCodeRate);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x25, &ui8LPCodeRate);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x2b, &ui8LpSelect);

		ui32Tmp =  ui32SnrX10;

		i32CNreceviedx10 = (int)(ui32Tmp);/* (int)(10 * snr); returned value is SNRx10 */

		if ((ui8Constellation > 2) || (ui8HPCodeRate > 4) || (ui8HierarchyInfo > 3)) {
			return 0;
		}

		if (ui8HierarchyInfo == 0) { /*0:Non hierarchical*/
			i32CNnordigx10 = NordigDVBTdBx10[ui8Constellation][ui8HPCodeRate];
		} else { /*>1:Hierarchical*/
			if (ui8LpSelect == 0) { /*HP*/
				i32CNnordigx10 = NordigHierHpDVBTdBx10[ui8HierarchyInfo - 1][ui8Constellation - 1][ui8HPCodeRate];
			} else { /*LP*/
				i32CNnordigx10 = NordigHierLpDVBTdBx10[ui8HierarchyInfo - 1][ui8Constellation - 1][ui8LPCodeRate];
			}
		}

		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1);   //RTK_A_DVBTX_LatchOn();


		/* 		ui8Status = RTK_A_DVBTX_Read(pDemod, 0x11,0x0a,&ui8Value)&0xf;*/
		/* 		ui32CFO = (ui8Value<<24);*/
		/* 		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11,0x09,&ui8Value);*/
		/* 		ui32CFO = (ui8Value<<16);*/
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x95, 0x61, &ui8Value);
		i32WinLength = (ui8Value << 8);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x95, 0x60, &ui8Value);
		i32WinLength += ui8Value;

		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x95, 0x64, &ui8Value);
		ui32Tmp = (ui8Value << 16);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x95, 0x63, &ui8Value);
		ui32Tmp = ui32Tmp  + (ui8Value << 8);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x95, 0x62, &ui8Value);
		ui32Tmp += ui8Value;

		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1);   //RTK_A_DVBTX_LatchOff();

		if (ui8Status) {
			*pSignalQuality = 0;
			sU8SQI[2] = *pSignalQuality;
			sU8SQI[0] = sU8SQI[1];
			sU8SQI[1] = sU8SQI[2];
			return  FUNCTION_ERROR;
		}

		/*ui32dPreRSBER = ui32Tmp*1.0/i32WinLength/204/8;///PreRS_BER*/
		RTK_A_DVBTX_64Mult(&i64Data, ui32Tmp, 1000000000);
		RTK_A_DVBTX_64Mult(&i64Data1, i32WinLength, 204 * 8);
		ui32dPreRSBER = RTK_A_DVBTX_64Div(i64Data, i64Data1);

		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x18, 0x2A, &u8TSLockFlag);

		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x26, &u8TmpGI); /*TPS GI*/
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x27, &u8TmpFFTMode); /*Transmission mode*/

		/* 			if(ui32dPreRSBER <1.0e-7)*/
		if ((ui32dPreRSBER < 100) && (u8TSLockFlag & 0x01)) {
			ui8BerSqi = 100;
		}
		/* 			else if((ui32dPreRSBER < 1.0E-3)&&(ui32dPreRSBER >= 1.0E-7))*/
		else if ((ui32dPreRSBER < 1000000) && (ui32dPreRSBER >= 100)) {
			/* 				ui8BerSqi = 2*RTK_A_DVBTX_Log10ConvertdB(1/ui32dPreRSBER) - 40;*/
			ui8BerSqi = (unsigned char)(2 * RTK_A_DVBTX_Log10ConvertdB(1000000000 / ui32dPreRSBER) - 40);
		} else {
			//64QAM, CR2/3, GI1/32, 8K
			if ((ui32dPreRSBER != 0) && ((ui8Constellation & 0x03) == 0x02) && ((ui8HPCodeRate & 0x07) == 0x01) && ((u8TmpGI & 0x03) == 0x00) && ((u8TmpFFTMode & 0x03) == 0x01) && (i32CNreceviedx10 < 170)) {
				ui8BerSqi = (unsigned char)(2 * RTK_A_DVBTX_Log10ConvertdB(1000000000 / ui32dPreRSBER) - 40);
			} else {
				ui8BerSqi = 0;
			}
		}

		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x26, &u8TmpGI); /*TPS GI*/
		if (RTK_A_DVBTX_DVBT_GI_1_4 == (RTK_A_DVBTX_DVBT_GUARD_INTERVAL_TYPE)(u8TmpGI & 0x03)) {
			i32CNreceviedx10 = i32CNreceviedx10 - 6; //GI =1_4, CN is about 0.5dB better
		}

		/*Calculate signal quality*/
		i32CNrelx10 = i32CNreceviedx10 - i32CNnordigx10;
		if ((ui8BerSqi > 0) && (ui8BerSqi < 100)) {
			i32CNrelx10 = i32CNrelx10 + 5; //0.5dB improved
		}
		if (i32CNrelx10 < -70) { /*CNrel < -7 dB*/
			*pSignalQuality =  0;
		} else if ((i32CNrelx10 >= -70) && (i32CNrelx10 < 30)) { /*-7 dB ��CNrel < +3 dB*/
			/*SQI = (((C/Nrel -3)/10) + 1) * ui8BerSqi*/
			*pSignalQuality = (unsigned char)(((((i32CNrelx10 - 30)) + 100) * ui8BerSqi) / 100);
		} else { /*CNrel ��+3 dB*/
			*pSignalQuality =  ui8BerSqi;
		}
		//if((i32CNrelx10 > 60)&&(u8TSLockFlag&0x01)) // > 6dB
		//{
		//*pSignalQuality= 100;
		//}
		sU8SQI[2] = *pSignalQuality;
		if ((sU8SQI[2] > 0) && (sU8SQI[2] < 100) && (ui8Constellation == 2)) { /*get middle value when 0<SQI <100*/
			RTK_A_DVBTX_MiddleValueu8(sU8SQI[0], sU8SQI[1], sU8SQI[2], &U8SQItmp);
			*pSignalQuality = (unsigned int) U8SQItmp;

			if ((*pSignalQuality > 2) && (*pSignalQuality < 70)) {
				*pSignalQuality  = *pSignalQuality  + 6;
			}
			if ((*pSignalQuality > 2) && (*pSignalQuality < 50)) {
				*pSignalQuality  = *pSignalQuality  + 3;
			}
		}
		sU8SQI[0] = sU8SQI[1];
		sU8SQI[1] = sU8SQI[2];


	}



	return (ui8Status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}


/**

@see   DVBT_DEMOD_FP_GET_LAYER_BER

*/

int
realtek_a_dvbtx_GetBer(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *ui32BERNum,
	unsigned int *ui32BERDen
)
{
	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;
	unsigned char ui8PlpFecType = 0, ui8CodeRate = 0;
	unsigned char u8L1S1 = 0, u8LiteUsed = 0;
	unsigned char ui8PWindow[3], ui8PBERCount[3];
	unsigned int  ui16PBCHLenTable[2][8] = {
		{7200,  9720,  10800, 11880, 12600, 13320, 5400, 6480}, /* Short FEC */
		{32400, 38880, 43200, 48600, 51840, 54000, 32400, 32400}  /* Normal FEC, the last two is not used */
	};
	unsigned int  ui16NBCHLen = 1;
	unsigned int ui32BlockWindow = 1, ui32BERCount = 1;
	unsigned char  ui8Tmp;
	unsigned int ui16WinLen = 0, ui32Tmp = 0;


	unsigned char u8DVBTxMode;


	if (realtek_a_dvbtx_GetDVBTxMode(pDemod, &u8DVBTxMode) != FUNCTION_SUCCESS)
		return FUNCTION_ERROR;

	if (u8DVBTxMode == RTK_A_DVBTX_DVBT2_MODE) {

		ui8Status = RTK_A_DVBTX_Read(pDemod, 0x0C, 0x38, &ui8CodeRate);

		/* ui8PlpFecType:0 16K, 1:64K*/
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x0C, 0x2E, &ui8PlpFecType);

		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x05,  &u8L1S1);
		if (((u8L1S1 & 0x07) == 0x03) || ((u8L1S1 & 0x07) == 0x04)) {
			u8LiteUsed = 1;
		}
		if ((ui8PlpFecType < 2) && (ui8CodeRate < 6) && ((u8L1S1 & 0x03) <= 0x01)) {
			ui16NBCHLen = ui16PBCHLenTable[ui8PlpFecType][ui8CodeRate];
		} else if ((ui8PlpFecType < 2) && (ui8CodeRate < 8) && (u8LiteUsed == 1)) {
			ui16NBCHLen = ui16PBCHLenTable[ui8PlpFecType][ui8CodeRate];
		} else {
			*ui32BERNum = 0xffffffff; /*No signal*/
			*ui32BERDen = 0x0; /*No signal*/
			return (ui8Status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;

		}

		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1);   //RTK_A_DVBTX_LatchOn();
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x18, 0x18, &ui8PWindow[2]);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x18, 0x17, &ui8PWindow[1]);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x18, 0x16, &ui8PWindow[0]);

		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x18, 0x1f, &ui8PBERCount[0]);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x18, 0x20, &ui8PBERCount[1]);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x18, 0x21, &ui8PBERCount[2]);
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 0);   //RTK_A_DVBTX_LatchOff();
		if (ui8Status != RTK_A_DVBTX_SUCCESSFUL) {
			*ui32BERNum = 0xffffffff;  /*I2C error or other error cases*/
			*ui32BERDen = 0x0;  /*I2C error or other error cases*/
			return FUNCTION_ERROR;

		}
		ui32BlockWindow = (ui8PWindow[2] & 0x0f) * 256 * 256 + ui8PWindow[1] * 256 + ui8PWindow[0];
		ui32BERCount = ui8PBERCount[2] * 256 * 256 + ui8PBERCount[1] * 256 + ui8PBERCount[0];

		if (ui32BlockWindow != 0) {
			//*ui32TxBERNum = ui32BERCount*1000000000;
			*ui32BERNum = ui32BERCount;
			*ui32BERDen = ui32BlockWindow * ui16NBCHLen;
		} else {
			*ui32BERNum = 0xffffffff;
			*ui32BERDen = 0;
		}
	} else if (u8DVBTxMode == RTK_A_DVBTX_DVBT_MODE) {

		ui8Status = RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1);   //RTK_A_DVBTX_LatchOn();
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x95, 0x61, &ui8Tmp);
		ui16WinLen = ui8Tmp << 8;
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x95, 0x60, &ui8Tmp);
		ui16WinLen = ui16WinLen + ui8Tmp;

		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x95, 0x64, &ui8Tmp);
		ui32Tmp = ui8Tmp << 16;
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x95, 0x63, &ui8Tmp);
		ui32Tmp = ui32Tmp  + (ui8Tmp << 8);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x95, 0x62, &ui8Tmp);
		ui32Tmp = ui32Tmp + ui8Tmp;
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 0);   //RTK_A_DVBTX_LatchOff();

		//*ui32BERNum = ui32Tmp*1000000000;
		*ui32BERNum = ui32Tmp;
		*ui32BERDen = ui16WinLen * 204 * 8;


	}


	return (ui8Status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;


}


/**

@see   DVBT_DEMOD_FP_GET_LAYER_PER

*/

/****************************************************************************
Function:	RTK_A_DVBTX_GetDVBT2PostBCHBlockER
Parameters: ui32T2BlockER: return dvbt2  Post BCH block error rate by this pointer.
Return: 	Operate  Status:RTK_A_DVBTX_SUCCESSFUL is Normal and other is abnormal
Description:
          Call this function to get DVBT2 BlockErrorRate value, can be used to evaluate TS output error rate.
          the *ui32T2BlockER value is the error rate value * 1e9.so ,if you want to get the correct
          BlockER value,you should use the value/1e9.
	      Block error rate can be used to indicate demodulator's output error rate.
******************************************************************************/


int
realtek_a_dvbtx_GetPer(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *ui32PEC_NUM,
	unsigned int *ui32PEC_DEN
)
{
	unsigned char ui8Tmp = 0;
	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;
	unsigned int ui32Tmp = 0, ui32WinLen = 0;


	ui8Status = RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1);   //RTK_A_DVBTX_LatchOn();
	ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x18, 0x92, &ui8Tmp);
	ui32WinLen = (ui8Tmp & 0x0f) << 16;

	ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x18, 0x91, &ui8Tmp);
	ui32WinLen = ui32WinLen + (ui8Tmp << 8);

	ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x18, 0x90, &ui8Tmp);
	ui32WinLen = ui32WinLen + ui8Tmp;

	ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x18, 0x95, &ui8Tmp);
	ui32Tmp = (ui8Tmp & 0x0f) << 16;

	ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x18, 0x94, &ui8Tmp);
	ui32Tmp = ui32Tmp + (ui8Tmp << 8);

	ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x18, 0x93, &ui8Tmp);
	ui32Tmp = ui32Tmp + ui8Tmp;

	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 0);   //RTK_A_DVBTX_LatchOff();

	if (ui32WinLen != 0) {
		*ui32PEC_NUM = ui32Tmp;
		*ui32PEC_DEN = ui32WinLen;
	} else {
		*ui32PEC_NUM = -1;
		*ui32PEC_DEN = -1;
	}
	return (ui8Status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}


/**

@see   DVBT_DEMOD_FP_GET_SNR_DB

*/
int
realtek_a_dvbtx_GetSnrDb(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *ui32SNRValuex10
)
{
	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;
	unsigned char ui8Value = 0;
	unsigned int ui32RoSignalVar = 0, ui32RoNoiseVar = 0, u32tmp = 0;
	RTK_A_DVBTX_64Data i64Data, i64Data1;
	unsigned char u8DVBTxMode;


	if (realtek_a_dvbtx_GetDVBTxMode(pDemod, &u8DVBTxMode) != FUNCTION_SUCCESS)
		return FUNCTION_ERROR;

	if (u8DVBTxMode == RTK_A_DVBTX_DVBT2_MODE) {


		/*Latch On*/
		ui8Status = RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1);   //RTK_A_DVBTX_LatchOn();

		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0x5c, &ui8Value);
		ui32RoSignalVar = (ui8Value << 16);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0x5b, &ui8Value);
		ui32RoSignalVar += (ui8Value << 8);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0x5a, &ui8Value);
		ui32RoSignalVar += ui8Value;

		ui32RoSignalVar = ui32RoSignalVar * 10;

		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0x62, &ui8Value);
		ui32RoNoiseVar = (ui8Value << 16);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0x61, &ui8Value);
		ui32RoNoiseVar += (ui8Value << 8);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0x60, &ui8Value);
		ui32RoNoiseVar += ui8Value;

		/*Latch Off*/
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 0);   //RTK_A_DVBTX_LatchOff();
		if (ui8Status) {
			*ui32SNRValuex10 = 0;
			return FUNCTION_ERROR;
		}
		if (ui32RoNoiseVar == 0 || ui32RoSignalVar == 0) {
			*ui32SNRValuex10 = 0;
			return FUNCTION_SUCCESS;
		}
		i64Data1.ui32High = 0;
		i64Data1.ui32Low = ui32RoNoiseVar;
		RTK_A_DVBTX_64Mult(&i64Data, ui32RoSignalVar, 4);
		u32tmp = RTK_A_DVBTX_Log10ConvertdBx10(RTK_A_DVBTX_64Div(i64Data, i64Data1));
		if (u32tmp > 100) {
			u32tmp =  u32tmp - 100;
		} else {
			u32tmp = 0;
		}
		*ui32SNRValuex10 = u32tmp;

	} else if (u8DVBTxMode == RTK_A_DVBTX_DVBT_MODE) {

		/*Latch On*/
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1);   //RTK_A_DVBTX_LatchOn();

		ui8Status = RTK_A_DVBTX_Read(pDemod, 0x11, 0xe3, &ui8Value);
		ui32RoNoiseVar = (ui8Value << 16);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0xe2, &ui8Value);
		ui32RoNoiseVar += (ui8Value << 8);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0xe1, &ui8Value);
		ui32RoNoiseVar += ui8Value;


		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0xdd, &ui8Value);
		ui32RoSignalVar = (ui8Value << 16);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0xdc, &ui8Value);
		ui32RoSignalVar += (ui8Value << 8);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0xdb, &ui8Value);
		ui32RoSignalVar += ui8Value;
		ui32RoSignalVar = ui32RoSignalVar * 10;
		/*Latch Off*/
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 0);   //RTK_A_DVBTX_LatchOff();
		if (ui8Status) {
			*ui32SNRValuex10 = 0;
			return FUNCTION_ERROR;
		}

		if (ui32RoNoiseVar == 0 || ui32RoSignalVar == 0) {
			*ui32SNRValuex10 = 0;
			return FUNCTION_SUCCESS;
		}
		i64Data1.ui32High = 0;
		i64Data1.ui32Low = ui32RoNoiseVar;
		RTK_A_DVBTX_64Mult(&i64Data, ui32RoSignalVar, 4);
		/*RTK_A_DVBTX_64Mult(&i64Data,ui32RoSignalVar,4);*/
		u32tmp = RTK_A_DVBTX_Log10ConvertdBx10(RTK_A_DVBTX_64Div(i64Data, i64Data1));
		if (u32tmp > 100) {
			u32tmp =  u32tmp - 100;
		} else {
			u32tmp = 0;
		}
		*ui32SNRValuex10 = u32tmp;

	}



	return FUNCTION_SUCCESS;
}



/**

@see   DVBT_DEMOD_FP_GET_RF_AGC

*/
int
realtek_a_dvbtx_SetAgcPol(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned char agcpola
)
{
// agc gain, 0: postive, 1:negative


	unsigned char RegVal = 0;

	gstRTK_A_DVBTX_DvbConfig.DvbTxTunerInput.ui8TxIFAgcPolarity =  agcpola ? RTK_A_DVBTX_IF_AGC_POLARITY_INVERTED : RTK_A_DVBTX_IF_AGC_POLARITY_NORMAL;


	if (gstRTK_A_DVBTX_DvbConfig.DvbTxTunerInput.ui8TxIFAgcPolarity == RTK_A_DVBTX_IF_AGC_POLARITY_INVERTED) {
		RTK_A_DVBTX_Read(pDemod, 0x00, 0x20, &RegVal);
		RegVal |= 0x02;
		RTK_A_DVBTX_Write(pDemod, 0x00, 0x20, RegVal);
	} else {
		RTK_A_DVBTX_Read(pDemod, 0x00, 0x20, &RegVal);
		RegVal &= 0xFD;
		RTK_A_DVBTX_Write(pDemod, 0x00, 0x20, RegVal);
	}

	return FUNCTION_SUCCESS;

}





/**

@see   DVBT_DEMOD_FP_GET_IF_AGC

*/
int
realtek_a_dvbtx_GetIfAgc(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *pIfAgc
)
{
	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;
	unsigned char ui8Pwm0 = 0, ui8Pwm1 = 0, u8AGCPolarity;
	unsigned int u16AGCValue = 0;
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1);   //RTK_A_DVBTX_LatchOn();
	ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x00, 0x20, &u8AGCPolarity);
	ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x10, 0x28, &ui8Pwm0);
	ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x10, 0x29, &ui8Pwm1);
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 0);   //RTK_A_DVBTX_LatchOff();
	if (ui8Status) {
		*pIfAgc = 0;
		return FUNCTION_ERROR;
	}

	u16AGCValue = ((ui8Pwm1 & 0x03) << 8) | ui8Pwm0;
	if ((u8AGCPolarity & 0x02) == 0x02) { //AGV inverted
		*pIfAgc = ((1024 - u16AGCValue) * 100) / 1024;
	} else {
		*pIfAgc  = (u16AGCValue * 100) / 1024;
	}

	return FUNCTION_SUCCESS;
}


/**

@see   DVBT_DEMOD_FP_GET_TR_OFFSET_PPM

*/
int
realtek_a_dvbtx_GetTrOffsetPpm(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pTrOffsetPpm
)
{

	return FUNCTION_SUCCESS;

}





/**

@see   DVBT_DEMOD_FP_GET_CR_OFFSET_HZ

*/
int
realtek_a_dvbtx_GetCrOffsetHz(
	DVBTX_DEMOD_MODULE *pDemod,
	int *pCrOffsetHz
)
{
	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;
	signed char ui8DataA = 0;
	unsigned char ui8Value = 0, u8iqswap = 0;
	unsigned char ui8Numerator = 0, ui8Denominator = 0;
	int i32Signal = 1;
	int i32DataC = 0;
	int i32CFO = 0, i32Rst = 0, i32cfotmp = 0;
	RTK_A_DVBTX_64Data i64Data, i64Data1;
	unsigned int ui32Tmp = 0;
	int i32Value = 0;



	unsigned char u8DVBTxMode;


	if (realtek_a_dvbtx_GetDVBTxMode(pDemod, &u8DVBTxMode) != FUNCTION_SUCCESS)
		return FUNCTION_ERROR;

	if (u8DVBTxMode == RTK_A_DVBTX_DVBT2_MODE) {

		ui8Status = RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1);   //RTK_A_DVBTX_LatchOn();
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0x0a, &ui8Value);
		i32CFO = ((ui8Value & 0xf) << 24);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0x09, &ui8Value);
		i32CFO += (ui8Value << 16);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0x08, &ui8Value);
		i32CFO += (ui8Value << 8);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0x07, &ui8Value);
		i32CFO += ui8Value;
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 0);   //RTK_A_DVBTX_LatchOff();

		if (ui8Status) {
			*pCrOffsetHz = 0;
			return FUNCTION_ERROR;
		}
		if (i32CFO & 0x8000000) {
			i32CFO = i32CFO - 268435456; /*ui32CFO - 2^28*/
			i32Signal = -1;
			i32CFO = 0 - i32CFO;/*first convert it to Positive data*/
		}

		/* 	RTK_A_DVBTX_64Mult(&i64Data,ui32CFO,1000);//KHz*/

		/*6487ED344    first should store the data to the Dividend   this data is PI*2^33 's Result*/
		i64Data1.ui32High = 6;
		i64Data1.ui32Low = 0x487ED345;


		if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_1D7MHZ) { /*1.7 MHz Only support for T2 */
			RTK_A_DVBTX_64Mult(&i64Data, i32CFO, 1000 * 131); /*KHz*/
			i32Rst = RTK_A_DVBTX_64Div(i64Data, i64Data1) / 71;
		} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_5MHZ) { /*6MHz*/
			RTK_A_DVBTX_64Mult(&i64Data, i32CFO, 1000 * 40); /*KHz*/
			i32Rst = RTK_A_DVBTX_64Div(i64Data, i64Data1) / 7;
		} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_6MHZ) { /*6MHz*/
			RTK_A_DVBTX_64Mult(&i64Data, i32CFO, 1000 * 48); /*KHz*/
			i32Rst = RTK_A_DVBTX_64Div(i64Data, i64Data1) / 7;
		} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_7MHZ) { /*7MHz*/
			RTK_A_DVBTX_64Mult(&i64Data, i32CFO, 1000 * 8); /*KHz*/
			i32Rst = RTK_A_DVBTX_64Div(i64Data, i64Data1);
		} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_8MHZ) { /*8MHz*/
			RTK_A_DVBTX_64Mult(&i64Data, i32CFO, 1000 * 64); /*KHz*/
			i32Rst = RTK_A_DVBTX_64Div(i64Data, i64Data1) / 7;
		} else { /*other  (10MHZ)*/
			RTK_A_DVBTX_64Mult(&i64Data, i32CFO, 1000 * 80); /*KHz*/
			i32Rst = RTK_A_DVBTX_64Div(i64Data, i64Data1) / 7;
		}

		i32Rst = i32Rst * i32Signal;
		/*i8Signal = 1;*/
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1);   //RTK_A_DVBTX_LatchOn();/**/
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x88, &ui8Value);
		i32DataC = ((ui8Value & 0x3f) << 8);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x87, &ui8Value);
		i32DataC += ui8Value;
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 0);   //RTK_A_DVBTX_LatchOff();
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x89, &ui8Value);
		ui8DataA  = ui8Value;
		if (ui8Status) {
			*pCrOffsetHz = 0;
			return FUNCTION_ERROR;
		}
		if (i32DataC & 0x2000) {
			i32DataC = i32DataC - 16384;
			/*i8Signal = -1;*/
			/*i32DataC = i32DataC *i8Signal;*/
		}
		i32DataC = i32DataC * 1000 / 12868 + ui8DataA * 1000;
		if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_1D7MHZ) {
			ui8Numerator = 71;
			ui8Denominator = 131;

		} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_5MHZ) {
			ui8Numerator = 7;
			ui8Denominator = 40;
		} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_6MHZ) {
			ui8Numerator = 7;
			ui8Denominator = 48;
		} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_7MHZ) {
			ui8Numerator = 1;
			ui8Denominator = 8;
		} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_8MHZ) {
			ui8Numerator = 7;
			ui8Denominator = 64;
		} else {
			ui8Numerator = 7;
			ui8Denominator = 80;
		}
		i32DataC = (i32DataC * ui8Denominator) / (ui8Numerator * 1024);

		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x70, &u8iqswap);


		/*Tuner input is inverted. here invert it, 2106.12.28*/
		if (gstRTK_A_DVBTX_DvbConfig.DvbTxTunerInput.ui8IQSwapMode == RTK_A_DVBTX_INPUT_IQ_SWAP) {
			if (u8iqswap & 0x01) {
				i32cfotmp = i32Rst + i32DataC;
				RTK_DEMOD_DVBTX_INFO("RTK_A_DVBTX_Debug DVB-T2 spectrum normal\n");

			} else {
				i32cfotmp = i32Rst + i32DataC;
				i32cfotmp = i32cfotmp * (-1);
				RTK_DEMOD_DVBTX_INFO("RTK_A_DVBTX_Debug DVB-T2 spectrum inverted\n");
			}

		} else {
			if (u8iqswap & 0x01) {
				i32cfotmp = i32Rst + i32DataC;
				i32cfotmp = i32cfotmp * (-1);
				RTK_DEMOD_DVBTX_INFO("RTK_A_DVBTX_Debug DVB-T2 spectrum inverted\n");
			} else {
				i32cfotmp = i32Rst + i32DataC;
				RTK_DEMOD_DVBTX_INFO("RTK_A_DVBTX_Debug DVB-T2 spectrum normal\n");
			}
		}
		*pCrOffsetHz = i32cfotmp * 1000;
		RTK_DEMOD_DVBTX_INFO("RTK_A_DVBTX_Debug DVB-T2 i32Rst=%d i32DataC=%d\n", i32Rst, i32DataC);

	}

	if (u8DVBTxMode == RTK_A_DVBTX_DVBT_MODE) {

		/*Latch registers*/
		ui8Status = RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1);   //RTK_A_DVBTX_LatchOn();

		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0x86, &ui8Value);
		i32CFO = ((ui8Value) << 24);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0x85, &ui8Value);
		i32CFO += (ui8Value << 16);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0x84, &ui8Value);
		i32CFO += (ui8Value << 8);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0x83, &ui8Value);
		i32CFO += ui8Value;
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x68, &ui8Value);
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 0);    //RTK_A_DVBTX_LatchOff();
		/*Unlatch registers*/
		ui32Tmp = ui8Value;
		if (ui8Status) {
			*pCrOffsetHz = 0;
			return FUNCTION_ERROR;
		}
		ui32Tmp = ui32Tmp & 0xf;
		i32CFO = (i32CFO & 0xfffffff) | (ui32Tmp << 28);

		if (i32CFO & 0x80000000) {
			/*ui32CFO = ui32CFO -0xffffffff-1;*/
			i32Signal = -1;
			i32CFO = 0 - i32CFO;/*first convert it to Positive data*/
		}

		/*RTK_A_DVBTX_64Mult(&i64Data,ui32CFO,1000);KHz*/
		/*6487ED344    first should store the data to the Dividend   this data is PI*2^33 's Result*/
		i64Data1.ui32High = 6;
		i64Data1.ui32Low = 0x487ED345;


		if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_1D7MHZ) { /*1.7 MHz Only support for T2 */
			RTK_A_DVBTX_64Mult(&i64Data, i32CFO, 1000 * 131); /*KHz*/
			i32Rst = RTK_A_DVBTX_64Div(i64Data, i64Data1) / 71;
		} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_5MHZ) { /*6MHz*/
			RTK_A_DVBTX_64Mult(&i64Data, i32CFO, 1000 * 40); /*KHz*/
			i32Rst = RTK_A_DVBTX_64Div(i64Data, i64Data1) / 7;
		} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_6MHZ) { /*6MHz*/
			RTK_A_DVBTX_64Mult(&i64Data, i32CFO, 1000 * 48); /*KHz*/
			i32Rst = RTK_A_DVBTX_64Div(i64Data, i64Data1) / 7;
		} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_7MHZ) { /*7MHz*/
			RTK_A_DVBTX_64Mult(&i64Data, i32CFO, 1000 * 8); /*KHz*/
			i32Rst = RTK_A_DVBTX_64Div(i64Data, i64Data1);
		} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_8MHZ) { /*8MHz*/
			RTK_A_DVBTX_64Mult(&i64Data, i32CFO, 1000 * 64); /*KHz*/
			i32Rst = RTK_A_DVBTX_64Div(i64Data, i64Data1) / 7;
		} else { /*other  (10MHZ)*/
			RTK_A_DVBTX_64Mult(&i64Data, i32CFO, 1000 * 80); /*KHz*/
			i32Rst = RTK_A_DVBTX_64Div(i64Data, i64Data1) / 7;
		}
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x6f, &u8iqswap);

		if (gstRTK_A_DVBTX_DvbConfig.DvbTxTunerInput.ui8IQSwapMode == RTK_A_DVBTX_INPUT_IQ_SWAP) {
			if (u8iqswap & 0x01) {
				i32Value = i32Rst * i32Signal * (-1);
				RTK_DEMOD_DVBTX_INFO("RTK_A_DVBTX_Debug DVB-T spectrum normal\n");
			} else {
				i32Value = i32Rst * i32Signal;
				RTK_DEMOD_DVBTX_INFO("RTK_A_DVBTX_Debug DVB-T spectrum inverted\n");
			}

		} else {
			if (u8iqswap & 0x01) {
				i32Value = i32Rst * i32Signal;
				RTK_DEMOD_DVBTX_INFO("RTK_A_DVBTX_Debug DVB-T spectrum inverted\n");
			} else {
				i32Value = i32Rst * i32Signal * (-1);
				RTK_DEMOD_DVBTX_INFO("RTK_A_DVBTX_Debug DVB-T spectrum normal\n");
			}
		}

		/*Patch for DVBT CFO*/
		i32Signal = 1;
		ui8Status = RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1);   //RTK_A_DVBTX_LatchOn();
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0x0a, &ui8Value);
		i32CFO = ((ui8Value & 0xf) << 24);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0x09, &ui8Value);
		i32CFO += (ui8Value << 16);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0x08, &ui8Value);
		i32CFO += (ui8Value << 8);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x11, 0x07, &ui8Value);
		i32CFO += ui8Value;
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 0);   //RTK_A_DVBTX_LatchOff();

		if (ui8Status) {
			i32Value = 0;
			return ui8Status;
		}
		if (i32CFO & 0x8000000) {
			i32CFO = i32CFO - 268435456; /*ui32CFO - 2^28*/
			i32Signal = -1;
			i32CFO = 0 - i32CFO;/*first convert it to Positive data*/
		}

		/*6487ED344    first should store the data to the Dividend   this data is PI*2^33 's Result*/
		i64Data1.ui32High = 6;
		i64Data1.ui32Low = 0x487ED345;

		if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_1D7MHZ) { /*1.7 MHz Only support for T2 */
			RTK_A_DVBTX_64Mult(&i64Data, i32CFO, 1000 * 131); /*KHz*/
			i32Rst = RTK_A_DVBTX_64Div(i64Data, i64Data1) / 71;
		} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_5MHZ) { /*6MHz*/
			RTK_A_DVBTX_64Mult(&i64Data, i32CFO, 1000 * 40); /*KHz*/
			i32Rst = RTK_A_DVBTX_64Div(i64Data, i64Data1) / 7;
		} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_6MHZ) { /*6MHz*/
			RTK_A_DVBTX_64Mult(&i64Data, i32CFO, 1000 * 48); /*KHz*/
			i32Rst = RTK_A_DVBTX_64Div(i64Data, i64Data1) / 7;
		} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_7MHZ) { /*7MHz*/
			RTK_A_DVBTX_64Mult(&i64Data, i32CFO, 1000 * 8); /*KHz*/
			i32Rst = RTK_A_DVBTX_64Div(i64Data, i64Data1);
		} else if (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_8MHZ) { /*8MHz*/
			RTK_A_DVBTX_64Mult(&i64Data, i32CFO, 1000 * 64); /*KHz*/
			i32Rst = RTK_A_DVBTX_64Div(i64Data, i64Data1) / 7;
		} else { /*other  (10MHZ)*/
			RTK_A_DVBTX_64Mult(&i64Data, i32CFO, 1000 * 80); /*KHz*/
			i32Rst = RTK_A_DVBTX_64Div(i64Data, i64Data1) / 7;
		}
		i32Rst = i32Rst * i32Signal;
		/*End of patch*/
		if (gstRTK_A_DVBTX_DvbConfig.DvbTxTunerInput.ui8IQSwapMode == RTK_A_DVBTX_INPUT_IQ_SWAP) {
			if (u8iqswap & 0x01) {
				i32Value = i32Value - i32Rst;
			} else {
				i32Value = i32Value + i32Rst;
			}
		} else {
			if (u8iqswap & 0x01) {
				i32Value = i32Value + i32Rst;
			} else {
				i32Value = i32Value - i32Rst;
			}
		}

		*pCrOffsetHz = i32Value * 1000;

	}



	return ui8Status;


}



/**

@see   DVBT_DEMOD_FP_GET_CELL_ID

*/
int
realtek_a_dvbtx_GetCellId(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned int *pCellId
)
{
	unsigned char u8ids[2];
	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;
	unsigned char u8DVBTxMode;

	*pCellId = 0;

	if (realtek_a_dvbtx_GetDVBTxMode(pDemod, &u8DVBTxMode) != FUNCTION_SUCCESS)
		return FUNCTION_ERROR;

	if (RTK_A_DVBTX_DVBT2_MODE == u8DVBTxMode) {
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1);   //RTK_A_DVBTX_LatchOn();
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x15,  &u8ids[0]);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x16,  &u8ids[1]);
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 0);    //RTK_A_DVBTX_LatchOff();
		*pCellId = (u8ids[1] << 8) + u8ids[0];
	}
	if (RTK_A_DVBTX_DVBT_MODE == u8DVBTxMode) {
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1);   //RTK_A_DVBTX_LatchOn();
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x28, &u8ids[0]);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x29, &u8ids[1]);
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 0);   //RTK_A_DVBTX_LatchOff();
		*pCellId = (u8ids[1] << 8) + u8ids[0];
	}

	return (ui8Status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}

/**

@see   DVBT_DEMOD_FP_SET_HIERARCHY

*/
int
realtek_a_dvbtx_SetTHierarchy(
	DVBTX_DEMOD_MODULE *pDemod,
	int Hierarchy
)
{

	RTK_A_DVBTX_STATUS status = RTK_A_DVBTX_SUCCESSFUL;
	/* DVB-T Enable*/
	unsigned char u8Hierarchy = (Hierarchy == HIERARCHY_MODE_LOW) ? 1 : 0;

	status = RTK_A_DVBTX_Write(pDemod, 0x00, 0x05, 0x01);  /*Reset DSP*/
	status |= RTK_A_DVBTX_Write(pDemod, 0x28, 0x2b, u8Hierarchy);
	status |= RTK_A_DVBTX_Write(pDemod, 0x00, 0x05, 0x00); /*Start DSP*/

	if (status != RTK_A_DVBTX_SUCCESSFUL) {
		RTK_DEMOD_DVBTX_INFO("Error in set HierarchyHpT\n");
	}
	return (status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}


/********************************************************************************
function    : RTK_A_DVBTX_SetPLPForMultiPLPDVBT2
parameter   :
	u8PLPid: PLP ID to receive
return      :Operate  Status:RTK_A_DVBTX_SUCCESSFUL is Normal and other is abnormal.
Description:Call this function to set PLP ID for multiPLP signal.
			If to receive first PLP in multiPLP or single PLP signal, should use RTK_A_DVBTX_SetAutoPLPT2
*********************************************************************************/

int
realtek_a_dvbtx_SetT2PlpID(
	DVBTX_DEMOD_MODULE *pDemod,
	int T2PlpID
)
{
	RTK_A_DVBTX_STATUS status = RTK_A_DVBTX_SUCCESSFUL;
	unsigned char u8PLPid = (unsigned char) T2PlpID;



	status |= RTK_A_DVBTX_Write(pDemod, 0x24, 0x02, u8PLPid);  /*Set PLP ID*/
	status |= RTK_A_DVBTX_Write(pDemod, 0x24, 0x03, 0x01);     /*Enable PLP selection,*/

	status |= RTK_A_DVBTX_Write(pDemod, 0x17, 0xBE, 0x01); // clear synderr flag
	status |= RTK_A_DVBTX_Write(pDemod, 0x17, 0xBE, 0x00);
	//status |= RTK_A_DVBTX_Write(pDemod, 0x17, 0xc3, DVBT2_BCH_L); /*BCH packet counter window*/
	//status |= RTK_A_DVBTX_Write(pDemod, 0x17, 0xc4, DVBT2_BCH_H); /*BCH packet counter window*/
	//status |= RTK_A_DVBTX_Write(pDemod, 0x18, 0x17, 0x04);
	//status |= RTK_A_DVBTX_Write(pDemod, 0x18, 0x16, 0x00);        /*LDPC BER counter window */

	status |= RTK_A_DVBTX_Write(pDemod, 0x00, 0x05, 0x01);     /*Reset DSP*/
	status |= RTK_A_DVBTX_Write(pDemod, 0x00, 0x05, 0x00);     /*Start DSP*/
	return (status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}


/********************************************************************************
function    :RTK_A_DVBTX_DVBTxModeDetected
parameter   :pu8DVBTxMode,RTK_A_DVBTX_DVBT_MODE/RTK_A_DVBTX_DVBT2_MODE is valid
return      :Operate  Status:RTK_A_DVBTX_SUCCESSFUL is Normal and other is abnormal.
Description:Call this function to check signal mode, DVB-T or DVB-T2 mode
            Attention DVB-T or DVB-T2 can be detected after signal Sync done.
			Typically within 300ms for DVB-T2 and 120ms for DVB-T after DSP start working.
*********************************************************************************/
int
realtek_a_dvbtx_GetDVBTxMode(
	DVBTX_DEMOD_MODULE *pDemod,
	unsigned char * pu8DVBTxMode
)

{
	unsigned char u8SyncT2 = 0, u8SyncT = 0;

	RTK_A_DVBTX_STATUS status = RTK_A_DVBTX_SUCCESSFUL;
	*pu8DVBTxMode = RTK_A_DVBTX_DVB_UNKNOWN;

	status = RTK_A_DVBTX_Read(pDemod, 0x88, 0x6D, &u8SyncT);
	if ((u8SyncT == 1) && (status == RTK_A_DVBTX_SUCCESSFUL)) {
		*pu8DVBTxMode = RTK_A_DVBTX_DVBT_MODE;
	}
	status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x6C, &u8SyncT2);

	if ((u8SyncT2 == 1) && (status == RTK_A_DVBTX_SUCCESSFUL)) {
		*pu8DVBTxMode = RTK_A_DVBTX_DVBT2_MODE;
	}
	return (status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}


/********************************************************************************
function    :RTK_A_DVBTX_GetSignalParamsT2
parameter   :pDvbTParams:pointer to DVB-T2 signal parameters.
return      :Operate  Status:RTK_A_DVBTX_SUCCESSFUL is Normal and other is abnormal
Description :Call this function to Get DVB-T2 signal parameters.
*********************************************************************************/

int
realtek_a_dvbtx_GetSignalParamsT2(
	DVBTX_DEMOD_MODULE *pDemod,
	RTK_A_DVBTX_DVBT2_PARAMS *pDvbT2Params
)
{
	RTK_A_DVBTX_STATUS status = RTK_A_DVBTX_SUCCESSFUL;
	/*T2 System Parameters */
	unsigned char u8FFTSize = 0, u8Ext = 0;
	unsigned char u8GuardInterval = 0;
	unsigned char u8PilotPattern = 0;
	unsigned char u8SisoMiso = 0;
	unsigned char u8T2Version = 0;
	unsigned char u8PreS1 = 0;
	unsigned char u8ids[2];
	unsigned char u8FefType = 0;
	unsigned char u8Tmp[4];
	unsigned char   u8FefInterval = 0;
	unsigned char  u8iqswap = 0;


	/*PLP Parameters */
	unsigned char u8DataPlpNumber = 0;              /*Number of Data PLP*/

	unsigned char u8Iter = 0, u8TmpPlpID  = 0, u8plpIndex = 0;

	unsigned char u8PlpSet = 0, u8PlpValid = 0;
	unsigned char u8Plpid = 0;  /* Current activated PLP ID */
	unsigned char u8CR = 0, u8Modu = 0, u8Plptype = 0, u8Payloadtype = 0, u8Fectype = 0, u8Rotation = 0, u8Streammode = 0, u8Issy = 0, u8Issytype = 0;

	/*Set default Unknown State*/
	//status = RTK_A_DVBTX_SetDefaultParamsT2(pDvbT2Params);
	pDvbT2Params->Dvbt2BandwidthExt = RTK_A_DVBTX_DVBT2_BANDWIDTH_EXT_UNKNOWN;
	pDvbT2Params->Dvbt2CodeRate      = RTK_A_DVBTX_DVBT2_CODE_RATE_UNKNOWN;
	pDvbT2Params->Dvbt2Constellation = RTK_A_DVBTX_DVBT2_QAM_UNKNOWN;
	pDvbT2Params->Dvbt2DataPLPType = RTK_A_DVBTX_DVBT2_PLP_TYPE_UNKNOWN;
	pDvbT2Params->Dvbt2FecLengthType = RTK_A_DVBTX_DVBT2_LDPC_LENGTH_UNKNOWN;

	pDvbT2Params->Dvbt2FFTMode          = RTK_A_DVBTX_DVBT2_MODE_UNKNOWN;
	pDvbT2Params->Dvbt2GuardInterval  = RTK_A_DVBTX_DVBT2_GI_UNKNOWN;
	pDvbT2Params->Dvbt2IssyType         = RTK_A_DVBTX_DVBT2_ISSY_UNKNOWN;
	pDvbT2Params->Dvbt2PayloadType   = RTK_A_DVBTX_DVBT2_PAYLOAD_UNKNOWN;
	pDvbT2Params->Dvbt2PilotPattern     = RTK_A_DVBTX_DVBT2_PP_UNKNOWN;

	pDvbT2Params->Dvbt2SisoMiso         = RTK_A_DVBTX_DVBT2_SISO_MISO_UNKNOWN;
	pDvbT2Params->Dvbt2StreamType   = RTK_A_DVBTX_DVBT2_STREAM_MODE_UNKNOWN;
	pDvbT2Params->Dvbt2Version          = RTK_A_DVBTX_DVBT2_VERSION_UNKNOWN;
	pDvbT2Params->Dvbt2BaseOrLiteType   = RTK_A_DVBTX_DVBT2_BASE_LITE_UNKNOWN;
	pDvbT2Params->u32FefLength          = 0;
	pDvbT2Params->u8FefInterval          = 1;
	pDvbT2Params->u8FefType              = 0;
	pDvbT2Params->u8ActivePlpId         = 0;
	pDvbT2Params->u8ConstellationRotation = 0;
	pDvbT2Params->u8DataPlpNumber = 0;


	/*T2 System Parameters */
	status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x63, &u8FFTSize);
	pDvbT2Params->Dvbt2FFTMode = (RTK_A_DVBTX_DVBT2_FFT_MODE_TYPE)(u8FFTSize & 0x07);

	status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x04, &u8Ext);
	pDvbT2Params->Dvbt2BandwidthExt = (RTK_A_DVBTX_DVBT2_BANDWIDTH_EXT_TYPE)(u8Ext & 0x01);

	status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x64, &u8GuardInterval);
	pDvbT2Params->Dvbt2GuardInterval = (RTK_A_DVBTX_DVBT2_GUARD_INTERVAL_TYPE)(u8GuardInterval & 0x07);

	status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x13,  &u8PilotPattern);
	pDvbT2Params->Dvbt2PilotPattern = (RTK_A_DVBTX_DVBT2_PILOT_PATTERN_TYPE)(u8PilotPattern & 0x0f);

	status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x05,  &u8SisoMiso);
	pDvbT2Params->Dvbt2SisoMiso = (RTK_A_DVBTX_DVBT2_SISO_MISO_TYPE)(u8SisoMiso & 0x07);

	status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x22,  &u8T2Version);
	if ((u8T2Version & 0x0f) < 3) { /*0:V1.11,   1:v1.21,    2: v1.31 */
		pDvbT2Params->Dvbt2Version = (RTK_A_DVBTX_DVBT2_VERSION_TYPE)(u8T2Version & 0x0f);
	} else {
		pDvbT2Params->Dvbt2Version = RTK_A_DVBTX_DVBT2_VERSION_UNKNOWN;
	}

	status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x05,  &u8PreS1);
	if ((u8PreS1 & 0x07) <= 1) {
		pDvbT2Params->Dvbt2BaseOrLiteType = RTK_A_DVBTX_DVBT2_BASE_PROFILE;
	}
	if (((u8PreS1 & 0x07) == 0x03) || ((u8PreS1 & 0x07) == 0x04)) {
		pDvbT2Params->Dvbt2BaseOrLiteType = RTK_A_DVBTX_DVBT2_LITE_PROFILE;
	}
	//status |= RTK_A_DVBTX_LatchOn();
	status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1);

	status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x15,  &u8ids[0]);
	status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x16,  &u8ids[1]);
	pDvbT2Params->Dvbt2CellId = (u8ids[1] << 8) + u8ids[0];

	status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x17,  &u8ids[0]);
	status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x18,  &u8ids[1]);
	pDvbT2Params->Dvbt2NetworkId = (u8ids[1] << 8) + u8ids[0];

	status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x19,  &u8ids[0]);
	status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x1a,  &u8ids[1]);
	pDvbT2Params->Dvbt2T2SystemId = (u8ids[1] << 8) + u8ids[0];

	status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x72,  &u8FefType);
	status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x6e,  &u8FefInterval);

	status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x6f,  &u8Tmp[0]);
	status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x70,  &u8Tmp[1]);
	status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x71,  &u8Tmp[2]);

	pDvbT2Params->u8FefType = u8FefType & 0x0F;
	pDvbT2Params->u32FefLength = (u8Tmp[2] << 16) + (u8Tmp[1] << 8) + u8Tmp[0];
	pDvbT2Params->u8FefInterval = u8FefInterval;


	/*PLP parameters */
	status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1);
	status |= RTK_A_DVBTX_Read(pDemod, 0x24, 0x04,  &u8PlpValid);
	status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 0);
	if (u8PlpValid == 1) {
		status |= RTK_A_DVBTX_Read(pDemod, 0x24, 0x05,  &u8DataPlpNumber);
		pDvbT2Params->u8DataPlpNumber = u8DataPlpNumber;

		/*Read all available Data PLP_ID*/
		for (u8Iter = 0; u8Iter < u8DataPlpNumber; u8Iter++) {
			u8plpIndex = u8Iter;
			status |= RTK_A_DVBTX_Read(pDemod, 0x23, u8plpIndex,  &u8TmpPlpID);
			pDvbT2Params->u8DataPlpIdArray[u8Iter] = u8TmpPlpID;
		}

		/*
		Read Current active Data PLP ID
		0x2403 ==1, PLP selected, 0x2402 is the valid PLP id.
		0x2403 ==0, The default first Data PLP can be read from 0x2300
		*/
		status |= RTK_A_DVBTX_Read(pDemod, 0x24, 0x03,  &u8PlpSet);
		if ((u8PlpSet & 0x01) == 0x01) {
			status |= RTK_A_DVBTX_Read(pDemod, 0x24, 0x02,  &u8Plpid);
		} else {
			status |= RTK_A_DVBTX_Read(pDemod, 0x23, 0x00,  &u8Plpid);
		}
		pDvbT2Params->u8ActivePlpId = u8Plpid;
	}

	/*code rate:  000:1/2, 001:3/5, 010:2/3, 011:3/4, 100:4/5, 101:5/6  */
	status |= RTK_A_DVBTX_Read(pDemod, 0x0C, 0x38, &u8CR);
	/*QPSK, 16QAM, 64QAM, 256QAM*/
	status |= RTK_A_DVBTX_Read(pDemod, 0x0C, 0x2F, &u8Modu);
	pDvbT2Params->Dvbt2CodeRate = (RTK_A_DVBTX_DVBT2_CODE_RATE_TYPE)(u8CR & 0x07);
	pDvbT2Params->Dvbt2Constellation = (RTK_A_DVBTX_DVBT2_CONSTELLATION_TYPE)(u8Modu & 0x07);

	status |= RTK_A_DVBTX_Read(pDemod, 0x0C, 0x30, &u8Plptype); /*TYPE1 PLP or TYPE2 PLP*/
	pDvbT2Params->Dvbt2DataPLPType = (RTK_A_DVBTX_DVBT2_PLP_TYPE)(u8Plptype & 0x07);
	status |= RTK_A_DVBTX_Read(pDemod, 0x0C, 0x3D, &u8Payloadtype); /*GSE, TS*/
	if ((u8Payloadtype & 0x1f) < 4) {
		pDvbT2Params->Dvbt2PayloadType = (RTK_A_DVBTX_DVBT2_PLP_PAYLOAD_TYPE)(u8Payloadtype & 0x01f);
	} else {
		pDvbT2Params->Dvbt2PayloadType = RTK_A_DVBTX_DVBT2_PAYLOAD_UNKNOWN;
	}

	status |= RTK_A_DVBTX_Read(pDemod, 0x0C, 0x2E, &u8Fectype);
	pDvbT2Params->Dvbt2FecLengthType = (RTK_A_DVBTX_DVBT2_FEC_LENGTH_TYPE)(u8Fectype & 0x03);

	status |= RTK_A_DVBTX_Read(pDemod, 0x0C, 0x3E, &u8Rotation);
	pDvbT2Params->u8ConstellationRotation = u8Rotation & 0x01;

	status |= RTK_A_DVBTX_Read(pDemod, 0x17, 0x7a, &u8Streammode);
	if (u8Streammode & 0x01) {
		pDvbT2Params->Dvbt2StreamType = RTK_A_DVBTX_DVBT2_HEM_MODE;
	} else {
		pDvbT2Params->Dvbt2StreamType = RTK_A_DVBTX_DVBT2_NORMAL_MODE;
	}

	status |= RTK_A_DVBTX_Read(pDemod, 0x17, 0x84, &u8Issy);
	if (u8Issy & 0x01) {
		status |= RTK_A_DVBTX_Read(pDemod, 0x17, 0x83, &u8Issytype);
		if (u8Issytype & 0x01) {
			pDvbT2Params->Dvbt2IssyType = RTK_A_DVBTX_DVBT2_ISSY_LONG;
		} else {
			pDvbT2Params->Dvbt2IssyType = RTK_A_DVBTX_DVBT2_ISSY_SHORT;
		}
	} else {
		pDvbT2Params->Dvbt2IssyType = RTK_A_DVBTX_DVBT2_ISSY_NO;
	}

	/******Get IQ Swap, LGE *****/
	/* Dvbt2IQSwap�� RTK_A_DVBTX_INPUT_IQ_NORMAL, RTK_A_DVBTX_INPUT_IQ_SWAP(inverted) ***/
	/****************************/
	status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x70, &u8iqswap);
	if (gstRTK_A_DVBTX_DvbConfig.DvbTxTunerInput.ui8IQSwapMode == RTK_A_DVBTX_INPUT_IQ_SWAP) {
		if (u8iqswap & 0x01) {
			pDvbT2Params->Dvbt2IQSwap = RTK_A_DVBTX_INPUT_IQ_NORMAL;
		} else {
			pDvbT2Params->Dvbt2IQSwap = RTK_A_DVBTX_INPUT_IQ_SWAP;
		}

	} else {
		if (u8iqswap & 0x01) {
			pDvbT2Params->Dvbt2IQSwap = RTK_A_DVBTX_INPUT_IQ_SWAP;
		} else {
			pDvbT2Params->Dvbt2IQSwap = RTK_A_DVBTX_INPUT_IQ_NORMAL;
		}
	}


	//status |= RTK_A_DVBTX_LatchOff();
	status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 0);
	return (status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}




/********************************************************************************
function    : RTK_A_DVBTX_GetSignalParamsT
parameter   :pDvbTParams:pointer to DVB-T signal parameters.
return      :Operate  Status:RTK_A_DVBTX_SUCCESSFUL is Normal and other is abnormal
Description :Call this function to Get DVB-T signal parameters.
*********************************************************************************/
int
realtek_a_dvbtx_GetSignalParamsT(
	DVBTX_DEMOD_MODULE *pDemod,
	RTK_A_DVBTX_DVBT_PARAMS *pDvbTParams
)

{
	RTK_A_DVBTX_STATUS status = RTK_A_DVBTX_SUCCESSFUL;
	unsigned char u8TpsDone = 0, u8SyncT = 0;
	unsigned char u8Tmp = 0;
	unsigned char u8Alpha = 0;
	unsigned char u8iqswap = 0;
	unsigned char u8ids[2];
	int i = 0;
	unsigned int CellIdVld = 0;

	/*Init parameters */
	pDvbTParams->DvbtFFTMode = RTK_A_DVBTX_DVBT_MODE_UNKNOWN;
	pDvbTParams->DvbtGuardInterval = RTK_A_DVBTX_DVBT_GI_UNKNOWN;
	pDvbTParams->DvbtHierarchy = RTK_A_DVBTX_DVBT_HIERARCHY_UNKNOWN;
	pDvbTParams->DvbtConstellation = RTK_A_DVBTX_DVBT_CONSTELLATION_UNKNOWN;
	pDvbTParams->DvbtHpCodeRate = RTK_A_DVBTX_DVBT_CODERATE_UNKNOWN;
	pDvbTParams->DvbtLpCodeRate	= RTK_A_DVBTX_DVBT_CODERATE_UNKNOWN;
	pDvbTParams->DvbtProfile = RTK_A_DVBTX_DVBT_HP_PROFILE;

	status = RTK_A_DVBTX_Read(pDemod, 0x28, 0x2c, &u8TpsDone); /*TPS done*/
	if (((u8TpsDone & 0x01) == 0x01) && (status == RTK_A_DVBTX_SUCCESSFUL)) { /*TPS decoded */
		status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x27,  &u8Tmp); /*TPS FFT*/
		pDvbTParams->DvbtFFTMode = (RTK_A_DVBTX_DVBT_FFT_MODE_TYPE)(u8Tmp & 0x03);

		status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x26, &u8Tmp); /*TPS GI*/
		pDvbTParams->DvbtGuardInterval = (RTK_A_DVBTX_DVBT_GUARD_INTERVAL_TYPE)(u8Tmp & 0x03);

		status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x22, &u8Tmp); /*modulation*/
		pDvbTParams->DvbtConstellation = (RTK_A_DVBTX_DVBT_CONSTELLATION_TYPE)(u8Tmp & 0x03);

		status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x23, &u8Tmp); /*alpha*/
		u8Alpha = u8Tmp & 0x07;
		switch (u8Alpha) {
		case 0:
			pDvbTParams->DvbtHierarchy = RTK_A_DVBTX_DVBT_HIERARCHY_NON;
			break;

		case 1:
			pDvbTParams->DvbtHierarchy = RTK_A_DVBTX_DVBT_HIERARCHY_1;
			break;

		case 2:
			pDvbTParams->DvbtHierarchy = RTK_A_DVBTX_DVBT_HIERARCHY_2;
			break;

		case 3:
			pDvbTParams->DvbtHierarchy = RTK_A_DVBTX_DVBT_HIERARCHY_4;
			break;

		default:
			pDvbTParams->DvbtHierarchy = RTK_A_DVBTX_DVBT_HIERARCHY_UNKNOWN;
			break;
		}

		status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x24, &u8Tmp); /*HP code rate*/
		pDvbTParams->DvbtHpCodeRate = (RTK_A_DVBTX_DVBT_CODE_RATE_TYPE)(u8Tmp & 0x07);
		if ((u8Alpha > 0) && (u8Alpha < 4)) {
			status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x25, &u8Tmp); /*LP code rate*/
			pDvbTParams->DvbtLpCodeRate = (RTK_A_DVBTX_DVBT_CODE_RATE_TYPE)(u8Tmp & 0x07);
			/*default HP profile, when Hierarchy used and LP selected, 0x282b = 1*/
			status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x2b, &u8Tmp); /*DVB-T profile*/
			if ((u8Tmp & 0x01) == 0x01) {
				pDvbTParams->DvbtProfile = RTK_A_DVBTX_DVBT_LP_PROFILE;
			}
		}
	} else { /*TPS not decoded, Check Sync state*/
		status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x6D, &u8SyncT);
		if ((u8SyncT & 0x01) && (status == RTK_A_DVBTX_SUCCESSFUL)) {
			status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x63, &u8Tmp); /* Sync detected FFT mode */

			if ((u8Tmp & 0x07) == 0x01) {
				pDvbTParams->DvbtFFTMode = RTK_A_DVBTX_DVBT_MODE_2K;
			}
			if ((u8Tmp & 0x07) == 0x03) {
				pDvbTParams->DvbtFFTMode = RTK_A_DVBTX_DVBT_MODE_8K;
			}
			/* Sync detected Guard interval */
			status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x64, &u8Tmp);
			if ((u8Tmp & 0x07) < 0x04) {
				pDvbTParams->DvbtGuardInterval = (RTK_A_DVBTX_DVBT_GUARD_INTERVAL_TYPE)(u8Tmp & 0x07);
			}
		}
	}

	/******Get IQ Swap*****/
	/* DvbtIQSwap�� RTK_A_DVBTX_INPUT_IQ_NORMAL / RTK_A_DVBTX_INPUT_IQ_SWAP (inverted) ***/
	/****************************/
	status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x6f, &u8iqswap);
	if (gstRTK_A_DVBTX_DvbConfig.DvbTxTunerInput.ui8IQSwapMode == RTK_A_DVBTX_INPUT_IQ_SWAP) {
		if (u8iqswap & 0x01) {
			pDvbTParams->DvbtIQSwap = RTK_A_DVBTX_INPUT_IQ_NORMAL;
		} else {
			pDvbTParams->DvbtIQSwap = RTK_A_DVBTX_INPUT_IQ_SWAP;
		}

	} else {
		if (u8iqswap & 0x01) {
			pDvbTParams->DvbtIQSwap = RTK_A_DVBTX_INPUT_IQ_SWAP;
		} else {
			pDvbTParams->DvbtIQSwap = RTK_A_DVBTX_INPUT_IQ_NORMAL;
		}
	}


	for (i = 0; i < 20; i++) {

		CellIdVld = ATB_DEMOD_TPS_RO_RESERV_BIT_5_0_get_cell_id_vld(rtd_inl(ATB_DEMOD_TPS_RO_RESERV_BIT_5_0_reg));
		if (CellIdVld == 1) {
			RTK_DEMOD_DVBTX_INFO("CellIdVld = %u i = %d \n", CellIdVld, i);
			break;
		}
		tv_osal_msleep(5);
	}


	status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1);  //RTK_A_DVBTX_LatchOn();
	status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x28, &u8ids[0]);
	status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x29, &u8ids[1]);
	status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 0);  //RTK_A_DVBTX_LatchOff();

	pDvbTParams->DvbtCellId = (u8ids[1] << 8) + u8ids[0];


	return (status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}

void realtek_a_dvbtx_SetTSVarThresh(DVBTX_DEMOD_MODULE *pDemod, unsigned char Nvco)
{
	unsigned char ui8nVco = Nvco;
	unsigned char u8BitN = 0;
	unsigned int u16TsVarCtl = 0xffff, u16tmp = 0;
	/*TS range changed */

	if (Nvco == 50) { //mode 2
		RTK_A_DVBTX_Write(pDemod, 0x18, 0xA0, 0x00);
		RTK_A_DVBTX_Write(pDemod, 0x18, 0xA1, 0x00);
		RTK_A_DVBTX_Write(pDemod, 0x18, 0xA2, 0xff);
		RTK_A_DVBTX_Write(pDemod, 0x18, 0xA3, 0xff);
		return;
	}

	if (ui8nVco < 40) {
		ui8nVco = 40;
	}
	if (ui8nVco > 55) {
		ui8nVco = 55;
	}
	u8BitN = 13 - (ui8nVco - 40) / 2; //Bits should clear

	u16tmp = 16 - u8BitN;
	u16tmp = (unsigned int)(RTK_A_DVBTX_FixPow(2, u16tmp));
	u16TsVarCtl = u16tmp - 1;


	//Under Thresh
	RTK_A_DVBTX_Write(pDemod, 0x18, 0xA3, ((u16TsVarCtl >> 8) & 0xff));
	RTK_A_DVBTX_Write(pDemod, 0x18, 0xA2, (u16TsVarCtl & 0xff));

	//Upper thresh
	if (Nvco <= 44) {
		RTK_A_DVBTX_Write(pDemod, 0x18, 0xA1, 0xf8);
		RTK_A_DVBTX_Write(pDemod, 0x18, 0xA0, 0x00);
	} else if (Nvco <= 48) {
		RTK_A_DVBTX_Write(pDemod, 0x18, 0xA1, 0xf0);
		RTK_A_DVBTX_Write(pDemod, 0x18, 0xA0, 0x00);
	} else {
		RTK_A_DVBTX_Write(pDemod, 0x18, 0xA1, 0xe0);
		RTK_A_DVBTX_Write(pDemod, 0x18, 0xA0, 0x00);
	}

}



/*Get counted TS bit rate*/
int  realtek_a_dvbtx_TSBitAccu(DVBTX_DEMOD_MODULE *pDemod, unsigned int *TSThroughputBps)
{
	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;
	unsigned char ui8PData[4] = {0};
	unsigned int u32Data = 0;
	unsigned char ui8TSwin = 0;
	/*
	18H     8CH     ts_cal_res[7:0] [7:0]   Read Only Latch 8'b0000_0000
	18H     8DH     ts_cal_res[15:8]        [7:0]   Read Only Latch 8'b0000_0000
	18H     8EH     ts_cal_res[19:16]       [3:0]   Read Only Latch 4'b0000
	18H     8FH     per_cal_win     [7:0]   Read Write      8'b0111_0010    1s as
	*/
	ui8Status = RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1);
	ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x18, 0x8c, &ui8PData[0]);
	ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x18, 0x8d, &ui8PData[1]);
	ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x18, 0x8e, &ui8PData[2]);
	ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x18, 0x8b, &ui8TSwin);
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 0);
	u32Data = ((ui8PData[2] & 0x0f) << 16) + (ui8PData[1] << 8) + ui8PData[0];
	*TSThroughputBps = (u32Data * 188 * 8 * 115) / (1000 * (1 + ui8TSwin));
	*TSThroughputBps = *TSThroughputBps * 105 / 100;
	return (ui8Status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}




/********************************************************************************
function    : realtek_a_dvbtx_CellIDOnOff
parameter   : ui8CellIDOnOff, 1:enable, 0:disable
return      : Operate  Status:RTK_A_DVBTX_SUCCESSFUL is Normal and other is abnormal
Description : DVB-T ui8CellIDOnOff parse on off
Disable Cell id parse reduce DVB-T lock time.
*********************************************************************************/
int realtek_a_dvbtx_CellIDOnOff(DVBTX_DEMOD_MODULE *pDemod, unsigned char ui8CellIDOnOff)
{
	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;
	if (ui8CellIDOnOff) {
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x28, 0x2e, 0x01);
	} else {
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x28, 0x2e, 0x00);
	}
	return (ui8Status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}


int realtek_a_dvbtx_DyStaitcCh_InitSet(DVBTX_DEMOD_MODULE *pDemod)
{
	/*
		72 a6 4//params_chan_change_peak_fo_factor_3_dvbt[7:0]
		72 67 10//params_short_chan_det_factor_dvbt
		72 77 2//params_dense_chan_det_ratio_out_dvbt
		72 76 4//params_dense_chan_symbol_in_dvbt
		72 7b 8//params_dense_chan_symbol_out_dvbt
		72 78 ca//params_dense_chan_det_num_th_out_8k_dvbt[7:0]
		72 8d 2e//params_noise_change_var_reg1_factor_dense_dvbt
		72 8e 2e//params_noise_change_var_reg2_factor_dense_dvbt
		72 90 2//params_chan_change_peak_reg1_factor_dense_dvbt[11:8]
		72 92 2//params_chan_change_peak_reg2_factor_dense_dvbt[11:8]
	*/

	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0xa6, 0x04);  //params_chan_change_peak_fo_factor_3_dvbt[7:0]
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x67, 0x10);  //params_short_chan_det_factor_dvbt
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x77, 0x02); //params_dense_chan_det_ratio_out_dvbt
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x76, 0x04); //params_dense_chan_symbol_in_dvbt
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x7b, 0x08); //params_dense_chan_symbol_out_dvbt
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x78, 0xca);  //params_dense_chan_det_num_th_out_8k_dvbt[7:0]
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x8d, 0x2e);  //params_noise_change_var_reg1_factor_dense_dvbt
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x8e, 0x2e);  //params_noise_change_var_reg2_factor_dense_dvbt
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x90, 0x02); //params_chan_change_peak_reg1_factor_dense_dvbt[11:8]
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x92, 0x02); //params_chan_change_peak_reg2_factor_dense_dvbt[11:8]

	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x20, 0x20);  //init set as 0x20
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0xa9, 0xff);  //add 05 27
//72 a9 ff
	return (ui8Status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;

}


//should be called periodically.
int realtek_a_dvbtx_CheckDyStaicChl(DVBTX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;
	unsigned char  u8DVBTxMode, u8Tmp, u8ChanSignalflag = 0, ui8Value;

	unsigned int ui32RoSignalVar = 0, ui32RoNoiseVar = 1;
	unsigned char  u8TmpModu = 0, u8TmpCr = 0, u8TmpGI = 0;

	if (realtek_a_dvbtx_GetDVBTxMode(pDemod, &u8DVBTxMode) != FUNCTION_SUCCESS)
		return FUNCTION_ERROR;


	if ((RTK_A_DVBTX_DVBT_MODE == u8DVBTxMode) && (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_8MHZ)) { //DVB-T2 mode
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x63, &u8Tmp); /* Sync detected FFT mode */
		//ui8Status |=RTK_A_DVBTX_Read(pDemod, 0x72,0xc5, &u8ChanStateflag);   /* Sync detected FFT mode */



		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x26, &u8TmpGI); /*TPS GI*/
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x24, &u8TmpCr); /*HP code rate*/
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x22, &u8TmpModu); /*modulation*/
		RTK_DEMOD_DVBTX_DBG("u8TmpGI :%d  u8TmpCr:%d u8TmpModu:%d \n", u8TmpGI, u8TmpCr, u8TmpModu);
		if ((RTK_A_DVBTX_DVBT_GI_1_4 == (RTK_A_DVBTX_DVBT_GUARD_INTERVAL_TYPE)(u8TmpGI & 0x03))  && (RTK_A_DVBTX_DVBT_CODE_RATE_1_2
				== (RTK_A_DVBTX_DVBT_CODE_RATE_TYPE)(u8TmpCr & 0x07)) && (RTK_A_DVBTX_DVBT_CONSTELLATION_64QAM    == (RTK_A_DVBTX_DVBT_CONSTELLATION_TYPE)(u8TmpModu & 0x03))) {
			u8ChanSignalflag = 1;
		}

		RTK_DEMOD_DVBTX_DBG("DVB-T 8K(fft mode should be 0x03):%d  u8ChanSignalflag(64QAMCR1_2_GI_1_4):%d\n", u8Tmp, u8ChanSignalflag);
		if (((u8Tmp & 0x07) == 0x03) && (u8ChanSignalflag == 1)) { //8k and 64QAM 1_2, GI1_4 mode ,then check noise and signal min
			/*Latch On*/
			ui8Status = RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 1);   //RTK_A_DVBTX_LatchOn();;

			ui8Status = RTK_A_DVBTX_Read(pDemod, 0x72, 0xc8, &ui8Value);
			ui32RoNoiseVar = (ui8Value << 16);
			ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x72, 0xc7, &ui8Value);
			ui32RoNoiseVar += (ui8Value << 8);
			ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x72, 0xc6, &ui8Value);
			ui32RoNoiseVar += ui8Value;

			ui32RoNoiseVar = ui32RoNoiseVar / 16;

			ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x72, 0xcb, &ui8Value);
			ui32RoSignalVar = (ui8Value << 16);
			ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x72, 0xca, &ui8Value);
			ui32RoSignalVar += (ui8Value << 8);
			ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x72, 0xc9, &ui8Value);
			ui32RoSignalVar += ui8Value;

			ui32RoSignalVar = ui32RoSignalVar / 4;
			/*Latch Off*/
			ui8Status = RTK_A_DVBTX_Write(pDemod, 0x01, 0x0d, 0);   //RTK_A_DVBTX_LatchOff();

			RTK_DEMOD_DVBTX_DBG("ui32RoSignalVar:%u  ui32RoNoiseVar:%u \n", ui32RoSignalVar, ui32RoNoiseVar);
			if (ui32RoSignalVar > (ui32RoNoiseVar * 100)) {
				ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x20, 0x60);
				ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x70, 0x00);
				ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0xa4, 0x00);
				ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0xa0, 0x32);
				ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x89, 0x04);
				ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x87, 0x00);
				/*
				72 20 60
				72 70 0
				72 a4 0
				72 a0 32
				72 89 4
				72 87 0*/
				RTK_DEMOD_DVBTX_DBG("Set 0x7220=0x60\n");

			}

			/*
			72 20 20
			72 70 1
			72 a4 8
			72 a0 7d
			72 89 50
			72 87 5*/




			else {
				ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x20, 0x20);
				ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x70, 0x01);
				ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0xa4, 0x08);
				ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0xa0, 0x7d);
				ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x89, 0x50);
				ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x87, 0x05);
			}

		} else {
			ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x20, 0x20);
			ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x70, 0x01);
			ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0xa4, 0x08);
			ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0xa0, 0x7d);
			ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x89, 0x50);
			ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x87, 0x05);
		}
	} else { //none DVB-T mode
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x20, 0x20);
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x70, 0x01);
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0xa4, 0x08);
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0xa0, 0x7d);
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x89, 0x50);
		ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x72, 0x87, 0x05);
	}
	return (ui8Status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}



//should be called periodically.
int realtek_a_dvbtx_CheckbyStaicChPwrChange(DVBTX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;
	//unsigned char u8ChanSignalflag = 0;
	unsigned char  u8DVBTxMode;
	unsigned char  u8TmpModu = 0, u8TmpCr = 0;
	unsigned char  u8dagc_scale_1 = 0, u8dagc_scale_0 = 0;

	if (realtek_a_dvbtx_GetDVBTxMode(pDemod, &u8DVBTxMode) != FUNCTION_SUCCESS)
		return FUNCTION_ERROR;


	if ((RTK_A_DVBTX_DVBT_MODE == u8DVBTxMode) && (u8sDVBTxBandwidth == RTK_A_DVBTX_BANDWIDTH_8MHZ)) { //DVB-T2 mode
		//ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x26, &u8TmpGI); /*TPS GI*/
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x24, &u8TmpCr); /*HP code rate*/
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x28, 0x22, &u8TmpModu); /*modulation*/
		//RTK_DEMOD_DVBTX_INFO("u8TmpCr:%d u8TmpModu:%d \n",u8TmpCr, u8TmpModu);
		if (RTK_A_DVBTX_DVBT_CONSTELLATION_QPSK == (RTK_A_DVBTX_DVBT_CONSTELLATION_TYPE)(u8TmpModu & 0x03)) {
			//u8ChanSignalflag = 1;
			ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x78, &u8dagc_scale_1);
			ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x88, 0x77, &u8dagc_scale_0);
			//RTK_DEMOD_DVBTX_DBG("Get dagc_scale_0x8878 = %d, dagc_scale_0x8877 = %d \n", u8dagc_scale_1, u8dagc_scale_0);
			if ((u8dagc_scale_1 == 128) && (u8dagc_scale_0 == 0)) {
				ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x00, 0x05, 0x01); /*Reset DSP*/
				ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x00, 0x05, 0x00); /*Start DSP*/
				tv_osal_msleep(1000);
				RTK_DEMOD_DVBTX_DBG("CheckbyStaicChPwrChange enable Reset DSP!!\n");
			}
		}

		//RTK_DEMOD_DVBTX_DBG("realtek_a_dvbtx_CheckbyStaicChPwrChange Done\n");
		//RTK_DEMOD_DVBTX_DBG("DVB-T 8K(fft mode should be 0x03):%d  u8ChanSignalflag(64QAMCR1_2_GI_1_4):%d\n", u8Tmp, u8ChanSignalflag);
		//return (ui8Status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
	}
	return (ui8Status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}


/********************************************************************************
function    :realtek_a_dvbtx_CheckT2TxIDErrSignal
parameter   : void
return      :
Description : restore registers to normal case setting changed by
realtek_a_dvbtx_CheckT2TxIDErrSignal.
*********************************************************************************/
int realtek_a_dvbtx_T2TxIDErrSignalRestore(DVBTX_DEMOD_MODULE *pDemod)
{

	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;
	unsigned char u8Rstenb = 0;

	//fixme, each channel set, must be called
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x21, 0x82, 0x00);  // restore as default
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0b, 0xdf, 0x00);  // restore as default
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x17, 0xbb, 0xf1);  // restore as default
	ui8Status |= RTK_A_DVBTX_Read(pDemod,  0x24, 0x09, &u8Rstenb);
	u8Rstenb  |= 0x15;	//bit 0,2,4 set 1
	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x24, 0x09, u8Rstenb);	  //2018.5.18

	ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x24, 0x12, 0x00);  // restore as default
	return (ui8Status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}



/********************************************************************************
function    :realtek_a_dvbtx_T2TxIDErrSignalRestore
parameter   : void
return      : Operate  Status:RTK_A_DVBTX_SUCCESSFUL is Normal and other is abnormal
Description : DVB-T2 signal received different Tx id from 2 more TX Tower.
L1-pre error, and L1-post right. called periodically.
*********************************************************************************/
int realtek_a_dvbtx_CheckT2TxIDErrSignal(DVBTX_DEMOD_MODULE *pDemod)
{
	RTK_A_DVBTX_STATUS ui8Status = RTK_A_DVBTX_SUCCESSFUL;
	unsigned char u8DVBTxMode;
	unsigned char u8L1PreCRCRight = 0, u8L1PostCRCRight = 0;
	unsigned char u8L1set = 0;
	unsigned char u8Iter = 0, u8FEC = 0, u8RstMask = 0, u8Rstenb = 0;
	unsigned int ui32T2SNRValuex10 = 0;
	unsigned char T2MaxcheckTimes = 24;

	if (realtek_a_dvbtx_GetDVBTxMode(pDemod, &u8DVBTxMode) != FUNCTION_SUCCESS)
		return FUNCTION_ERROR;

	realtek_a_dvbtx_GetSnrDb(pDemod, &ui32T2SNRValuex10);

	RTK_DEMOD_DVBTX_DBG("DVB-T2 mode Sync done! u8DVBTxMode:%d Tx snr:%u\n", u8DVBTxMode, ui32T2SNRValuex10);
	if (RTK_A_DVBTX_DVBT2_MODE == u8DVBTxMode) { //DVB-T2 mode
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x01, &u8L1PreCRCRight);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x02, &u8L1PostCRCRight);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x21, 0x82, &u8L1set);
		ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x17, 0xbb, &u8RstMask);

		if (((u8L1PreCRCRight & 0x01) != 0x01) && ((u8L1PostCRCRight & 0x01) != 0x01)) { /* Check L1pre error and L1post CRC right*/

			tv_osal_msleep(50); /*wait more 50 miliseconds, and read L1 again*/
			ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x01, &u8L1PreCRCRight);
			ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x02, &u8L1PostCRCRight);
		}


		RTK_DEMOD_DVBTX_DBG("u8L1PreCRCRight:%d u8L1PostCRCRight:%d L1Set:%d\n", u8L1PreCRCRight, u8L1PostCRCRight, u8L1set);
		if (((u8L1PreCRCRight & 0x01) != 0x01) && ((u8L1PostCRCRight & 0x01) != 0x01)) { /* Check L1pre error and L1post CRC right*/
			if (u8L1set != 0x01) {
				ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x24, 0x12, 0x00);  // restore as default
				ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x24, 0x09, 0x3f);  // restore as default

				ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x21, 0x82, 0x01);
				ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0b, 0xdf, 0x03);
				u8RstMask &= 0xef;  //bit4 set 0
				ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x17, 0xbb, u8RstMask);

				//L1 post wait 600ms timeout.
				for (u8Iter = 0; u8Iter < 16; u8Iter++) {
					//dsp reset required?, not required
					realtek_a_dvbtx_GetSnrDb(pDemod, &ui32T2SNRValuex10);
					ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x17, 0xec, &u8FEC); //debug info, check fec lock  time
					ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x02, &u8L1PostCRCRight);
					RTK_DEMOD_DVBTX_DBG("u8FEC:%d u8L1PostCRCRight:%d SNR:%u \n", u8FEC, u8L1PostCRCRight, ui32T2SNRValuex10);
					if ((u8FEC & 0x01) || (u8L1PostCRCRight)) {
						break;
					}
					tv_osal_msleep(50); /*Delay 50 millisecond.*/
				}
				if (u8L1PostCRCRight) { //L1 right
					//ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x00, 0x05, 0x01);
					//ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x00, 0x05, 0x00); //DSP reset is not required.
					if (ui32T2SNRValuex10 > 200) { //defaut 24*50ms, signal >20dB
						T2MaxcheckTimes = 150;
					}
					for (u8Iter = 0; u8Iter < T2MaxcheckTimes; u8Iter++) {   //check 800ms timeout
						//dsp reset required?
						realtek_a_dvbtx_GetSnrDb(pDemod, &ui32T2SNRValuex10);
						ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x17, 0xec, &u8FEC);
						ui8Status |= RTK_A_DVBTX_Read(pDemod, 0x0B, 0x02, &u8L1PostCRCRight);
						RTK_DEMOD_DVBTX_DBG("L1 post locked, u8FEC:%d u8L1PostCRCRight:%d %u\n", u8FEC, u8L1PostCRCRight, ui32T2SNRValuex10);

						//please test:
						// 1, after FEC locked disable L1 reset.
						// 2, after FEC lock or L1 post right,  disable L1 reset.

						if (u8FEC & 0x01) {
							//if((u8FEC & 0x01)||(u8L1PostCRCRight)) {

							ui8Status |= RTK_A_DVBTX_Read(pDemod,  0x24, 0x09, &u8Rstenb);
							u8Rstenb &= 0xea;	//bit 0 set 0, bit 2 set 0, bit 4 set 0
							ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x24, 0x09, u8Rstenb);

							ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x24, 0x12, 0x05);

							ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x0b, 0xdf, 0x00);
							ui8Status |= RTK_A_DVBTX_Write(pDemod, 0x21, 0x82, 0x00);
							break;
						}
						tv_osal_msleep(50); /*Delay 100 millisecond.*/
					}

				}
			} else	{
				//do nothing.
			}

			//if FEC not locked, restore register to normal setting.
			if ((u8FEC & 0x01) != 0x01) {
				realtek_a_dvbtx_T2TxIDErrSignalRestore(pDemod);
			}
		} else { //L1 right, signal be normal case.
			//L1 righ  do nothing
		}

	}
	return (ui8Status == RTK_A_DVBTX_SUCCESSFUL) ? FUNCTION_SUCCESS : FUNCTION_ERROR;
}


