#define __AVD__
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/delay.h>
#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
//#include <mach/RPCDriver.h>
#include <rtk_kdriver/RPCDriver.h>//sunray_liao add
#include <linux/pageremap.h>
#include <uapi/linux/const.h>
#include <linux/mm.h>
#include <linux/string.h>/*memset*/
#include <linux/freezer.h>
#include <linux/timer.h>
#include <scaler_vfedev.h>
#include <ioctrl/scaler/vfe_cmd_id.h>
#include <tvscalercontrol/vdc/video.h>
#include <tvscalercontrol/scalerdrv/inprocvideo.h>
#include <tvscalercontrol/pcbsource/pcbSource.h>

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerDrvCommon.h>
#else
#include <scalercommon/scalerDrvCommon.h>
#endif

#include <tvscalercontrol/scalerdrv/syncproc.h>
//#include <tvscalercontrol/frontend/ifd_new.h>
//#include <rbus/adc_reg.h>
#include <rbus/vdtop_reg.h>
#include <mach/pcbMgr.h>//sunray
#include <tvscalercontrol/vdc/yc_separation_vpq.h>
#include <tvscalercontrol/vdc/video_factory.h>
// FixMe
//#include <include/rbus/scaler/rbusLSADCReg.h>
#include <tvscalercontrol/vdc/video_input.h>
#include <tvscalercontrol/adcsource/adcctrl.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
#include <tvscalercontrol/scaler/scalervideo.h>
#endif
#include <scaler_vscdev.h>
#include <scaler_vpqdev.h>

//#include <tvscalercontrol/tve/tve.h>
#include <tvscalercontrol/vdac/vdac.h>

#include <tvscalercontrol/scalerdrv/power.h>
//#include <rbus/rbusVDPQReg.h>
#include <rbus/vdpq_reg.h>//sunray
//#include <rbus/crt_reg.h>
#include <rbus/sys_reg_reg.h>//sunray
#include <rbus/pll27x_reg_reg.h>
//#include <mach/rtk_crt.h>
#include <rtk_kdriver/rtk_crt.h>//sunray
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/vdc/video.h>
#include <tvscalercontrol/avd/avdctrl_vd.h>
#include <tvscalercontrol/avd/avdctrl.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <rbus/tv_sb1_ana_reg.h>//add for audio
#include <rbus/audio_reg.h>//add for audio

#ifdef CONFIG_CUSTOMER_TV010
#include <rbus/vgip_reg.h>
#include <tvscalercontrol/scalerdrv/mode.h>

#define MORE_FRONT_PORCH	(10)
#define _FIELD_CHANGE_THRESHOLD			10
#endif

/**********************************************/
//#define VSTATE_STABLE 3
#define Interlaced  0
#define Progressive 1
//#define ATV_VSTATE_STABLE 3
//#define ATV_DETECT_STAGE_INIT 0
//#define ATV_DETECT_STAGE_HVLOCK 1
//#define ATV_DETECT_STAGE_VSTATE_READY 2
//#define ATV_DETECT_STAGE_FINAL 0xff


//==jj refine===============================
unsigned char g_ucAtvDetectStage = DETECT_STAGE_INIT;

unsigned char g_ucVDMode = _MODE_UNKNOWN;
unsigned char g_ucVDPreMode = _MODE_UNKNOWN;
unsigned char g_ucModeCurr = _MODE_UNKNOWN;
KADP_VFE_AVD_SCART_FB_STATUS_T g_ucScartMode = _MODE_UNKNOWN;
KADP_VFE_AVD_DEMOD_TYPE g_tAVDDemodType = KADP_AVD_EXTERNAL_DEMOD;
KADP_VFE_AVD_COLOR_SYSTEM_T g_tSupportedColorSys = -1;

unsigned int g_ulVDCPhyAddr = 0;
extern void Scaler_AVD_Set_VDC_Power_Setting(VDC_POWER_SETTING pow, unsigned char source);

//==jj refine===============================

//#define V_lock_num 6
#define _MODE_MIXED 0xff
#define _MODE_UNKNOWN 0xff

//unsigned char g_ucAtvVideoMode = COLOR_SYSTEM_PAL_G;
//unsigned char g_ucAtvVDMode = 0xff;
//static unsigned char v_lock_count;
static StructDisplayInfo g_tAVDScalerDispInfo;
static ModeInformationType g_tAVDScalerTimingInfo;
static KADP_VFE_AVD_TIMING_INFO_T g_tAVDLGETimingInfo = {0};
static KADP_VFE_AVD_AVDECODER_VIDEO_MODE_T g_tLGEVideoMode = KADP_VIDEO_AVDEC_MODE_UNKNOWN;
static KADP_VFE_AVD_CC_TTX_STATUS_T g_tCCTTXStatus = KADP_VFE_AVD_NOTHING_IS_SUPPORTED;

static unsigned char g_ucSyncDetForTuning = TRUE;

#ifdef CONFIG_CUSTOMER_TV010
static unsigned short *tAV_PRESET_TABLE = NULL;
static UINT8 ucFieldChanged_Case0_Counter	= 0;
static UINT8 ucFieldChanged_Case1_Counter	= 0;
static UINT8 ucFieldChanged_Case2_Counter	= 0;
static unsigned char bFieldChangedFlag = FALSE;
#endif

//static bool g_bRunSearchState;
//static bool g_bAVDDetectTimingAgain;
bool g_bMainRunSearchState;
bool g_bSubRunSearchState;

#ifdef CONFIG_CUSTOMER_TV010
static bool g_bFVBiasRunScaler;
#endif

static bool g_AutoTuneMode;

unsigned long g_ulVDCPhyAddr_cache = 0;
unsigned int *g_ulVDCPhyAddr_nonCache = 0;

unsigned int g_ulInitStageCount;
//extern unsigned char vdc_mode_detect;
//unsigned char g_ucAVDInDetectTiming = false;
unsigned char g_ucStopVDTimer = TRUE;

unsigned char avd_good_timing_ready = FALSE;
unsigned char check_dma_3d_stable = FALSE;
unsigned char vdpq_dma_3d_ready = FALSE;
bool g_AVD_Main_HalVscConnect = FALSE;
bool g_AVD_Sub_HalVscConnect = FALSE;

// refactor sta ==================================================>
static bool g_bChangeSystem = true;
static bool g_bStartChangeSystem = true;
static bool atv_channel_change = false;
static bool atv_channel_change_start = false;

static unsigned char g_ucDetectStage = DETECT_STAGE_INIT;
static unsigned char g_ucMode_Curr = _MODE_480I;

unsigned char g_ucSrcType = _SRC_TV;

// refactor end ==================================================<

//extern unsigned char g_ucVSCCheckATVChanelChange;
//extern unsigned char g_ucVSCATVChanelChangeStart;
//extern unsigned char g_ucVSCATVSetWinBlankCount;


#ifdef CONFIG_CUSTOMER_TV010
unsigned short g_usIsVtotalNoGood=0;
unsigned short g_usIsGetTimeInfo=0;
unsigned short g_usCurVHaldTotal;
unsigned short g_usCurVActLen;
#endif

/********************************************************************************/
void ClearAVDLGETimingInfo(void);
static char CheckSupportedColorSys(KADP_VFE_AVD_AVDECODER_VIDEO_MODE_T a_tColorSystem);
static KADP_VFE_AVD_AVDECODER_VIDEO_MODE_T TransColorSysRTK2LGE(unsigned char a_ucVDMode);
static unsigned char GetLGESupportedMode(void);
static int getSCART_pin8(void);
unsigned char get_avd_print_flag(void);
void AVD_UpdateLGE_wid_len(unsigned int len,unsigned int wid);

//static void AVD_check_av_color_std(unsigned char bInCheckMode);
/********************************************************************************/
extern void Scaler_VBI_disconnect(void);
extern void drv_ifd_analog_preset(void);
extern void drv_ifd_adc_clock_init_162M(void);
#ifdef CONFIG_CUSTOMER_TV010
extern void drv_ifd_set_video_lpf_bw_1M(bool enable);
#endif
void Scaler_AVD_ClearTimingInfo(void);
char Scaler_AVD_Timer_Init(void);
void Scaler_AVD_Timer_Delete(void);
void SendAVDSourceInfo(unsigned char a_ucSrcType, unsigned char a_ucDemodType);
void  IFD_SIF_ADC_Pll_Initial(void);//add for audio requirements
void  IFD_SIF_ADC_Initial_Flow(void);//add for audio requirements
void Scaler_AVD_Set_VDC_Power(VDC_POWER_SETTING pow, unsigned char source);

// refactor sta ==================================================>
unsigned char Scaler_AVD_Get_g_ucMode_Curr(void)
{
	return g_ucMode_Curr;
}

void Scaler_AVD_Set_g_ucMode_Curr(unsigned char a_ucMode)
{
	g_ucMode_Curr = a_ucMode;
}

bool Scaler_AVD_Get_g_bChangeSystem(void)
{
	return  g_bChangeSystem;
}

void Scaler_AVD_Set_g_bChangeSystem(bool a_bFlag)
{
	g_bChangeSystem = a_bFlag;
}

bool Scaler_AVD_Get_g_bStartChangeSystem(void)
{
	return  g_bStartChangeSystem;
}

void Scaler_AVD_Set_g_bStartChangeSystem(bool a_bFlag)
{
	g_bStartChangeSystem = a_bFlag;
}

unsigned char Scaler_AVD_Get_g_ucSrcType(void)
{
	return g_ucSrcType;
}

void Scaler_AVD_Set_g_ucSrcType(unsigned char a_ucSource)
{
	g_ucSrcType = a_ucSource;
}

unsigned char Scaler_AVD_GetDetectStage(void)
{
	return g_ucDetectStage;
}

void Scaler_AVD_SetDetectStage(unsigned char stage)
{
	g_ucDetectStage = stage;
}

bool Scaler_AVD_Get_ChannelChange(void)
{
	return atv_channel_change;
}

void Scaler_AVD_Set_ChannelChange(bool a_bFlag)
{
	atv_channel_change = a_bFlag;
}

bool Scaler_AVD_GetIsChannelChange(void)
{
	return atv_channel_change_start;
}

void Scaler_AVD_SetIsChannelChange(bool a_bFlag)
{
	atv_channel_change_start = a_bFlag;
}


// refactor new ==================================================<

void Set_Val_AVD_Good_Timing_Ready(unsigned char ucFlag)
{
	avd_good_timing_ready = ucFlag;
}

unsigned char avd_drv_timing_ready(void)
{
	if((_SRC_CVBS == Scaler_AVD_Get_g_ucSrcType()) || (_SRC_TV == Scaler_AVD_Get_g_ucSrcType()))
		return avd_good_timing_ready;
	else
		return FALSE;
}

unsigned char (*Get_Val_AVD_good_timing_ready)(void) = avd_drv_timing_ready;

unsigned char Scaler_AVD_GetVDPreMode(void)
{
	return g_ucVDPreMode;
}

unsigned char  (*Get_Val_AVD_g_ucVDPreMode)(void) = Scaler_AVD_GetVDPreMode;
// refactor end ==================================================<

#ifdef SPEEDUP_NEW_SCALER_FLOW
unsigned char avd_start_need_compensation = FALSE;//this is for avd start compensation for i3ddma vgip
unsigned char get_avd_start_need_compensation(void)
{
	return avd_start_need_compensation;
}

void set_avd_start_need_compensation(unsigned char TorF)
{
	avd_start_need_compensation = TorF;
}
#endif

StructDisplayInfo *Get_AVD_ScalerDispinfo(void)
{
	if (FALSE == module_vdc_check_vd_clock()) {
		VDC_DEBUG_MSG(VDC_MSG_ERROR, "module_vdc_check_vd_clock fail!\n");
		drvif_module_vdc_ClockOn();
	}
	if(TRUE == drvif_module_vdc_get_needopenpowerflag()) {
		rtd_pr_avd_info("Get_AVD_ScalerDispinfo Turn On VDC power \n");
		Scaler_AVD_Set_VDC_Power_Setting(VDC_Power_on, get_AVD_Input_Source());
	}
	return &g_tAVDScalerDispInfo;
}

ModeInformationType *Get_AVD_ScalerTiminginfo(void)
{
	return &g_tAVDScalerTimingInfo;
}

KADP_VFE_AVD_TIMING_INFO_T *Get_AVD_LGETiminginfo(void)
{
	return &g_tAVDLGETimingInfo;
}

void Scaler_AVD_SetSyncDetectForTuning(unsigned char a_ucFlag)
{
	g_ucSyncDetForTuning = a_ucFlag;
}

unsigned char Scaler_AVD_DoesSyncExist(void)
{
	unsigned char ucSyncExist = false;
#if 1
	if (FALSE == module_vdc_check_vd_clock()) {
		drvif_module_vdc_ClockOn();
	}

	ucSyncExist= drvif_module_vdc_CheckHVisLocked();

	//rtd_pr_avd_emerg("#####[%s(%d)] ucSyncExist=%x\n",__func__,__LINE__,ucSyncExist);
#else
	/* For ATV Tunning*/
	if (TRUE == g_ucSyncDetForTuning) {
		ucSyncExist = drvif_video_status_reg(VDC_hv_lock);
	}
	/*For Channel change*/
	else {
		ucSyncExist = drvif_video_status_reg(VDC_vlock);
	}
	rtd_pr_avd_emerg("Scaler_AVD_DoesSyncExist  ucSyncExist=%d\n", ucSyncExist);
#endif
	return ucSyncExist;
}

#ifdef CONFIG_CUSTOMER_TV010
unsigned char Scaler_AVD_HV_Coarse_Lock(void)
{
	unsigned char ucSyncExist = false;

	/* For ATV Tunning*/
	ucSyncExist= drvif_module_vdc_CheckHVisLocked_TVScan();
	drv_ifd_set_video_lpf_bw_1M(FALSE);
	//rtd_pr_avd_emerg("Scaler_AVD_HV_Coarse_Lock=%x\n", ucSyncExist);

	return ucSyncExist;
}
#endif

int Scaler_AVD_Init(KADP_VFE_AVD_CC_TTX_STATUS_T a_tCCTTXStatus)
{
	int ulRet = 0;
	g_tCCTTXStatus = a_tCCTTXStatus;
	ClearAVDLGETimingInfo();
	memset(&g_tAVDScalerDispInfo, 0, sizeof(StructDisplayInfo));
	memset(&g_tAVDScalerTimingInfo, 0, sizeof(ModeInformationType));
	return ulRet;
}

char Scaler_AVD_Uninit(void)
{
	char cRet = 0;
	ClearAVDLGETimingInfo();
	memset(&g_tAVDScalerDispInfo, 0, sizeof(StructDisplayInfo));
	memset(&g_tAVDScalerTimingInfo, 0, sizeof(ModeInformationType));
	return cRet;
}

#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
void Scaler_AVD_EnableVD27MMode(VD_27M_OVERSAMPLING_MODE mode)
{
	if (VD_27M_OFF != mode)
	{
		Scaler_Set_VDC_27MHz_Mode_Enable(TRUE);
		fw_video_set_27mhz_mode(mode);
		drvif_module_vdc_set_27M_mode(TRUE);
	}
	else
	{
		Scaler_Set_VDC_27MHz_Mode_Enable(FALSE);
		fw_video_set_27mhz_mode(VD_27M_OFF);
		drvif_module_vdc_set_27M_mode(FALSE);
	}
}
#endif

char Scaler_AVD_Open(void)
{
	char cRet = 0;

	rtd_pr_avd_info("Scaler_Set_InputSrc()\n");

	Check_PLL27X_Settings();

	/*CVBS*/
	down(get_avd_power_semaphore());
	drvif_module_vdc_ClockOn();
	drvif_vdac_tvdac_power_on(OFF);
	up(get_avd_power_semaphore());

	/*Memory allocate*/
	if(AVD_vpq_mem_ctrl(_ENABLE) == TRUE)
	{
		module_vdc_set_Out444_sel(VDC_444output);
		drvif_module_vdc_27M_FrontDigitalFilterEnable(VDC_27M_FDF_BYPASS);
		module_vdc_Reset();

		//disable auto change for 3D to 2D when signal is noisy.
		vdcf_maskl(VDTOP_SIGNAL_STM_CONTROL_reg, ~_BIT4, _ZERO);
		drvif_inprocvideo_init();
	}
	else
	{
		rtd_pr_avd_info("Allocate memory fail\n");
	}

	return cRet;
}

char Scaler_AVD_Close(void)
{
	char cRet = 0;
	rtd_pr_avd_info("Scaler_AVD_Close\n");

#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
	Scaler_AVD_EnableVD27MMode(VD_27M_OFF);
#endif

#if 0
	//rtd_pr_avd_emerg("Need to wait detect timing done\n");
	while(g_ucAVDInDetectTiming)
	{
		mdelay(100);
		rtd_pr_avd_emerg("Wait AVD detect timing done. g_ucAVDInDetectTiming=%d \n", g_ucAVDInDetectTiming);
	}
#endif

	AVD_vpq_mem_ctrl(_DISABLE);
	ClearAVDLGETimingInfo();
	Scaler_AVD_ClearTimingInfo();

	down(get_avd_power_semaphore());
	rtd_pr_avd_info("Turn off VDC power\n");
	drvif_module_vdc_PowerCtrl(OFF, _SRC_TV);
	Scaler_AVD_Set_VDC_Power_Setting(VDC_Power_off, _SRC_TV);
	up(get_avd_power_semaphore());

	avd_good_timing_ready = FALSE;

	memset(&g_tAVDScalerDispInfo, 0, sizeof(StructDisplayInfo));
	memset(&g_tAVDScalerTimingInfo, 0, sizeof(ModeInformationType));
	return cRet;
}


void reset_avd_timing_ready(void)
{//auto run scaler to use. reset timing ready
#ifdef SPEEDUP_NEW_SCALER_FLOW
	if (_SRC_CVBS == get_AVD_Input_Source()) {
		StructDisplayInfo *ptAVDScalerDispInfo = Get_AVD_ScalerDispinfo();
		ModeInformationType *ptAVDScalerTimingInfo = Get_AVD_ScalerTiminginfo();
		avd_good_timing_ready = FALSE;
		memset(ptAVDScalerDispInfo, 0, sizeof(StructDisplayInfo));
		memset(ptAVDScalerTimingInfo, 0, sizeof(ModeInformationType));
		Scaler_AVD_SetDetectStage(DETECT_STAGE_INIT);
	}
#endif
}

unsigned char avd_drv_vdpq_dma_3d_ready(void)
{
	if (g_ucSrcType == _SRC_CVBS || g_ucSrcType == _SRC_TV)
		return vdpq_dma_3d_ready;
	else
		return FALSE;
}

char Scaler_AVD_AV_Connect(unsigned short srcinput)
{
	char cRet = 0;
	unsigned char ucSize = 0, i = 0;
	StructSourceInput *srcinput_pt = PcbSource_GetTable();

	rtd_pr_avd_info("#####[%s(%d)] AVD_Input=%d (_AV_INPUT1=259, _AV_INPUT2=515, _AV_INPUT3=771)\n",__func__,__LINE__,srcinput);

	g_ulInitStageCount = 0;

	drvif_module_vdc_SECAM_ColorReduction(_DISABLE);/*patch for HW issue 20160105,CSFC*/
#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
	Scaler_AVD_EnableVD27MMode(VD_27M_HSD960_DI5A);
#endif

	down(get_avd_power_semaphore());
	drvif_module_vdc_PowerCtrl(ON, _SRC_CVBS);
	Scaler_AVD_Set_VDC_Power_Setting(VDC_Power_saving, _SRC_CVBS);
	up(get_avd_power_semaphore());
	mdelay(2);

	g_ucSrcType = _SRC_CVBS;
	ucSize = PcbSource_GetTableSize();
	for (i = 0 ; i < ucSize ; i++) {
		if (srcinput_pt->src_index == srcinput)
			break;
		srcinput_pt++;
	}
	if (i == ucSize) {
		rtd_pr_avd_info("Can't find srcinput in pcbsource=%x\n", srcinput);
		return -1;
	}
	g_tAVDScalerDispInfo.input_src = i;
	g_tAVDScalerDispInfo.data_array_index = i;
	g_tAVDScalerDispInfo.osd_input_src = i;

	drvif_module_vdc_SelectInput(_SRC_CVBS, (VDC_INITIAL_OPTIONS *)srcinput_pt->InitOptions);
//	drvif_module_vdc_set_burst_gate_position(VDC_MODE_AUTO);/*add by lichun zhao 20120105*/
	drvif_module_vdc_set_vsync_freerun_enable(_DISABLE);

#ifdef CONFIG_CUSTOMER_TV010
	if(g_ucSrcType == _SRC_CVBS)
		drvif_vdac_init();
#endif

	cRet = Scaler_AVD_Timer_Init();

	return cRet;
}

char Scaler_AVD_AV_Disconnect(void)
{
	char cRet = 0;
	rtd_pr_avd_info("Scaler_AVD_AV_Disconnet()\n");

	// Delete timer
	Scaler_AVD_Timer_Delete();

#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
	Scaler_AVD_EnableVD27MMode(VD_27M_OFF);
#endif

	vdc_LockStatus = 0;
	Scaler_VBI_disconnect();
	ClearAVDLGETimingInfo();
	Scaler_AVD_ClearTimingInfo();
	memset(&g_tAVDScalerDispInfo, 0, sizeof(StructDisplayInfo));
	memset(&g_tAVDScalerTimingInfo, 0, sizeof(ModeInformationType));

	g_tSupportedColorSys = -1;
	g_ucVDMode = _MODE_UNKNOWN;

	return cRet;
}

char Scaler_AVD_SetLGEColorSystem(KADP_VFE_AVD_COLOR_SYSTEM_T a_tColorSystem)
{
	char ret = 0;
	g_tSupportedColorSys = a_tColorSystem;
	rtd_pr_avd_info("#####[%s(%d)] a_tColorSystem=0x%x, g_tSupportedColorSys=0x%x\n", __FUNCTION__, __LINE__, a_tColorSystem, g_tSupportedColorSys);
	return ret;
}

static char CheckSupportedColorSys(KADP_VFE_AVD_AVDECODER_VIDEO_MODE_T a_tColorSystem)
{
	KADP_VFE_AVD_COLOR_SYSTEM_T tColorSys = BIT0_VFE_AVD << a_tColorSystem;

	//rtd_pr_avd_emerg("kernel Scaler_CheckSupportedColorSys() g_tSupportedColorSys=%x, a_tColorSystem=%x\n", g_tSupportedColorSys, a_tColorSystem);

	if ((g_tSupportedColorSys & tColorSys) == 0) {
		//rtd_pr_avd_debug("Color sys not support, tColorSys=%x\n", tColorSys);
		//rtd_pr_avd_emerg("Color sys not support, tColorSys=%x\n", tColorSys);
		return -1;
	} else {
		//rtd_pr_avd_debug("Color sys support, tColorSys=%x\n", tColorSys);
		//rtd_pr_avd_emerg("Color sys support, tColorSys=%x\n", tColorSys);
		return 0;
	}
}

char Scaler_AVD_ATV_Connect(unsigned short srcinput)
{
	char cRet = 0;
	unsigned char ucSize = 0, i = 0;
	StructSourceInput *srcinput_pt = PcbSource_GetTable();

	rtd_pr_avd_info("#####[%s(%d)] srcinput=0x%x\n",__func__,__LINE__,srcinput);

	g_ucVDPreMode = _MODE_UNKNOWN;

	drvif_module_vdc_SECAM_ColorReduction(_DISABLE);/*patch for HW issue 20160105,CSFC*/
#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
	//Scaler_AVD_EnableVD27MMode(VD_27M_HSD960_DI5A);
	Scaler_AVD_EnableVD27MMode(VD_27M_OFF);
#endif

	g_ulInitStageCount = 0;

	//rtd_pr_avd_emerg("0   Scaler_AVD_ATV_Connect SYS_REG_SYS_SRST1_reg=%x\n", rtd_inl(SYS_REG_SYS_SRST1_reg));
	if (KADP_AVD_EXTERNAL_DEMOD == g_tAVDDemodType) {
		srcinput=_AV_INPUT2;
		//IFD_SIF_ADC_Pll_Initial();//add for audio requirements
	}

	g_ucSrcType = _SRC_TV;
	drvif_module_vdc_set_noSignalCounter_Value(0x03);
	ucSize = PcbSource_GetTableSize();
	for (i = 0 ; i < ucSize ; i++) {
		if (srcinput_pt->src_index == srcinput)
			break;
		srcinput_pt++;
	}
	if (i == ucSize) {
		rtd_pr_avd_debug("Can't find srcinput in pcbsource=%x\n", srcinput);
		return -1;
	}
	if (KADP_AVD_INTERNAL_DEMOD == g_tAVDDemodType) {
		down(get_avd_power_semaphore());
		drvif_module_vdc_PowerCtrl(ON, _SRC_TV);
		Scaler_AVD_Set_VDC_Power_Setting(VDC_Power_saving, _SRC_TV);
		up(get_avd_power_semaphore());
		mdelay(2);
		drvif_module_vdc_SelectInput(_SRC_TV, (VDC_INITIAL_OPTIONS *)srcinput_pt->InitOptions);
	} else if (KADP_AVD_EXTERNAL_DEMOD == g_tAVDDemodType) {
		down(get_avd_power_semaphore());
		drvif_module_vdc_PowerCtrl(ON, _SRC_CVBS);
		Scaler_AVD_Set_VDC_Power_Setting(VDC_Power_saving, _SRC_CVBS);
		drvif_module_vdc_set_external_ch_change_flag(FALSE);//For k6418
		//add for audio requirements
		if (_SRC_TV == g_ucSrcType) {
			IFD_SIF_ADC_Initial_Flow();
		}
		up(get_avd_power_semaphore());
		mdelay(2);
		//drvif_module_vdc_SelectInput(_SRC_CVBS, (VDC_INITIAL_OPTIONS *)srcinput_pt->InitOptions);
		drvif_module_vdc_for_demod_external(_SRC_CVBS, (VDC_INITIAL_OPTIONS *)srcinput_pt->InitOptions);
	}
	g_tAVDScalerDispInfo.input_src = i;
	g_tAVDScalerDispInfo.data_array_index = i;
	g_tAVDScalerDispInfo.osd_input_src = i;
//	drvif_module_vdc_set_burst_gate_position(VDC_MODE_AUTO);
	drvif_module_vdc_set_vsync_freerun_enable(_DISABLE);
//	v_lock_count = 0;

	cRet = Scaler_AVD_Timer_Init();
	//rtd_pr_avd_emerg("1   Scaler_AVD_ATV_Connect SYS_REG_SYS_SRST1_reg=%x\n", rtd_inl(SYS_REG_SYS_SRST1_reg));
	return cRet;
}

char Scaler_AVD_ATV_Disconnect(void)
{
	char cRet = 0;
	rtd_pr_avd_info("Scaler_AVD_ATV_Disconnect\n");

	g_ucVDPreMode = _MODE_UNKNOWN;

    // clear the auto scan flag
    drvif_module_vdc_set_internal_auto_scan_flag(FALSE);
    drvif_module_vdc_set_external_auto_scan_flag(FALSE);

	Scaler_AVD_Set_ChannelChange(false);
	Scaler_AVD_SetIsChannelChange(false);

	// Delete timer
	Scaler_AVD_Timer_Delete();
	vdc_LockStatus = 0;
	drv_module_vdc_SetInterrupt(VDC_Mode_irq_disable);
	drvif_module_vdc_FreeRun(VDC_FREERUN_DISABLE);
	Scaler_VBI_disconnect();
	ClearAVDLGETimingInfo();
	Scaler_AVD_ClearTimingInfo();
	memset(&g_tAVDScalerDispInfo, 0, sizeof(StructDisplayInfo));
	memset(&g_tAVDScalerTimingInfo, 0, sizeof(ModeInformationType));

	g_tSupportedColorSys = -1;
	g_ucVDMode = _MODE_UNKNOWN;

	return cRet;
}


KADP_VFE_AVD_TIMING_INFO_T *Scaler_AVD_GetLGETimingInfo(unsigned char *a_ucRet)
{
	*a_ucRet = 0;
//	VFE_AVD_TIMING_INFO_T *ptLGETimingInfo;

#ifdef CONFIG_CUSTOMER_TV010
	g_usIsGetTimeInfo++;
	if (g_usIsGetTimeInfo <= 3 || g_usIsGetTimeInfo%40==0) {
	    rtd_pr_avd_info("g_tAVDLGETimingInfo.standard=%d, vTotal=%d(0x%x), active.h=%d(0x%x)\n", g_tAVDLGETimingInfo.standard, g_tAVDLGETimingInfo.vTotal, g_tAVDLGETimingInfo.vTotal, g_tAVDLGETimingInfo.active.h, g_tAVDLGETimingInfo.active.h);
	    rtd_pr_avd_info("format=%d, htotal=%d(0x%x), active.w=%d(0x%x)\n", g_tAVDLGETimingInfo.format, g_tAVDLGETimingInfo.hTotal, g_tAVDLGETimingInfo.hTotal, g_tAVDLGETimingInfo.active.w, g_tAVDLGETimingInfo.active.w);
	}
#endif

	SetAVDLGETimingInfo();

#if 0
	rtd_pr_avd_debug( "active.h=%d\n", ptLGETimingInfo->active.h);
	rtd_pr_avd_debug( "active.w=%d\n", ptLGETimingInfo->active.w);
	rtd_pr_avd_debug( "active.x=%d\n", ptLGETimingInfo->active.x);
	rtd_pr_avd_debug( "active.y=%d\n", ptLGETimingInfo->active.y);
	rtd_pr_avd_debug( "format=%d\n", ptLGETimingInfo->format);
	rtd_pr_avd_debug( "hFreq=%d\n", ptLGETimingInfo->hFreq);
	rtd_pr_avd_debug( "vFreq=%d\n", ptLGETimingInfo->vFreq);
	rtd_pr_avd_debug( "hTotal=%d\n", ptLGETimingInfo->hTotal);
	rtd_pr_avd_debug( "vTotal=%d\n", ptLGETimingInfo->vTotal);
	rtd_pr_avd_debug( "hPorch=%d\n", ptLGETimingInfo->hPorch);
	rtd_pr_avd_debug( "vPorch=%d\n", ptLGETimingInfo->vPorch);
	rtd_pr_avd_debug( "scanType=%d\n", ptLGETimingInfo->scanType);
	rtd_pr_avd_debug( "vdLock=%d\n", ptLGETimingInfo->vdLock);
	rtd_pr_avd_debug( "HLock=%d\n", ptLGETimingInfo->HLock);
	rtd_pr_avd_debug( "VLock=%d\n", ptLGETimingInfo->VLock);
#endif

	//rtd_pr_avd_emerg("g_tAVDLGETimingInfo.standard=%x, hTotal=%x, format=%x\n", g_tAVDLGETimingInfo.standard, g_tAVDLGETimingInfo.hTotal, g_tAVDLGETimingInfo.format);
	//rtd_pr_avd_emerg("s=%x, ht=%x, h=%x, w=%x\n", g_tAVDLGETimingInfo.standard, g_tAVDLGETimingInfo.hTotal, g_tAVDLGETimingInfo.active.h, g_tAVDLGETimingInfo.active.w);
	return &g_tAVDLGETimingInfo;
}

void SetAVDLGETimingInfo(void)
{
	bool Support480I = false, Support576I = false;
	unsigned char ucModeCur = 0;
	StructDisplayInfo *ptAVDScalerDispInfo;
	//ModeInformationType *ptAVDScalerTimingInfo;
	KADP_VFE_AVD_TIMING_INFO_T *ptLGETimingInfo;

	unsigned char uc625_flag = 0xff;

	ptAVDScalerDispInfo = Get_AVD_ScalerDispinfo();;
	//ptAVDScalerTimingInfo = Get_AVD_ScalerTiminginfo();
	ptLGETimingInfo = Get_AVD_LGETiminginfo();
	ucModeCur = ptAVDScalerDispInfo->ucMode_Curr;
	if ((g_tSupportedColorSys & KADP_COLOR_SYSTEM_NTSC_M) == KADP_COLOR_SYSTEM_NTSC_M
		|| (g_tSupportedColorSys & KADP_COLOR_SYSTEM_NTSC_443) == KADP_COLOR_SYSTEM_NTSC_443
		|| (g_tSupportedColorSys & KADP_COLOR_SYSTEM_PAL_60) == KADP_COLOR_SYSTEM_PAL_60
		|| (g_tSupportedColorSys & KADP_COLOR_SYSTEM_PAL_M) == KADP_COLOR_SYSTEM_PAL_M)
	{
		Support480I = true;
	}
	if ((g_tSupportedColorSys & KADP_COLOR_SYSTEM_PAL_G) == KADP_COLOR_SYSTEM_PAL_G
		|| (g_tSupportedColorSys & KADP_COLOR_SYSTEM_PAL_NC_358)	== KADP_COLOR_SYSTEM_PAL_NC_358
		|| (g_tSupportedColorSys & KADP_COLOR_SYSTEM_SECAM) == KADP_COLOR_SYSTEM_SECAM
//		|| (g_tSupportedColorSys & KADP_COLOR_SYSTEM_PAL_60) == KADP_COLOR_SYSTEM_PAL_60
		|| g_tSupportedColorSys == -1) {
		Support576I = true;
	}
	/*No signal*/
	if (DETECT_STAGE_FINAL != Scaler_AVD_GetDetectStage()) {
		ClearAVDLGETimingInfo();
		// When no siganl, only ATV needs to set active.h, active.w vFreq
		// In other VD sources, all timing info is zero

		if ( _SRC_TV == g_ucSrcType)
		{
			//rtd_pr_avd_info("no siganl\n");
			if (Support480I == true && Support576I == true && g_tSupportedColorSys != -1) {
				KADP_VFE_AVD_COLOR_SYSTEM_T LastVideoMode = -1;
				LastVideoMode = BIT0_VFE_AVD << g_tLGEVideoMode;
				if ((LastVideoMode & KADP_COLOR_SYSTEM_NTSC_M) == KADP_COLOR_SYSTEM_NTSC_M
					|| (LastVideoMode & KADP_COLOR_SYSTEM_NTSC_443) == KADP_COLOR_SYSTEM_NTSC_443
					|| (LastVideoMode & KADP_COLOR_SYSTEM_PAL_60) == KADP_COLOR_SYSTEM_PAL_60
					|| (LastVideoMode & KADP_COLOR_SYSTEM_PAL_M) == KADP_COLOR_SYSTEM_PAL_M)
				{
					Support480I = true;
					Support576I = false;
				}
				else if ((LastVideoMode & KADP_COLOR_SYSTEM_PAL_G) == KADP_COLOR_SYSTEM_PAL_G
					|| (LastVideoMode & KADP_COLOR_SYSTEM_PAL_NC_358) == KADP_COLOR_SYSTEM_PAL_NC_358
//					|| (LastVideoMode & KADP_COLOR_SYSTEM_PAL_60) == KADP_COLOR_SYSTEM_PAL_60
					|| (LastVideoMode & 	KADP_COLOR_SYSTEM_SECAM) == KADP_COLOR_SYSTEM_SECAM)
				{
					Support480I = false;
					Support576I = true;
				}
			}
			if ((Support480I == true && Support576I == false)||(Scaler_AVD_Get_g_ucMode_Curr() == _MODE_480I))  {
				ptLGETimingInfo->active.h = 480;
				ptLGETimingInfo->active.w = 704;
				ptLGETimingInfo->vFreq = 599;
			} else if ((Support576I == true && Support480I == false)||(Scaler_AVD_Get_g_ucMode_Curr() == _MODE_576I))  {
				ptLGETimingInfo->active.h = 576;
				ptLGETimingInfo->active.w = 704;
				ptLGETimingInfo->vFreq = 500;
			} else if (Support576I == true && Support480I == true) 	{
				ptLGETimingInfo->active.h = 480;
				ptLGETimingInfo->active.w = 704;
				ptLGETimingInfo->vFreq = 599;
			} else {
				ptLGETimingInfo->active.h = 480;
				ptLGETimingInfo->active.w = 704;
				ptLGETimingInfo->vFreq = 599;
			}


			// External demod auto scan (For China model)
			// External demod
			if (0 == drvif_module_vdc_read_Clamping_path())
			{
				// Auto scan
				if (drvif_module_vdc_read_external_autoscan_flag())
				{
					uc625_flag = drvif_video_get_625_flag();

					VD_recheck_625flag_loop(&uc625_flag);

					if (uc625_flag != 0xff)
					{
						if (0 == uc625_flag)
						{
							ptLGETimingInfo->active.h = 480;
							ptLGETimingInfo->active.w = 704;
							ptLGETimingInfo->vFreq = 599;

							//rtd_pr_avd_emerg("autoscan 525 line, std=%x\n", ptLGETimingInfo->standard);
						}
						else
						{
							ptLGETimingInfo->active.h = 576;
							ptLGETimingInfo->active.w = 704;
							ptLGETimingInfo->vFreq = 500;

							//rtd_pr_avd_emerg("autoscan 625 line, std=%x\n", ptLGETimingInfo->standard);
						}
					}
				}
				else
				{
					//rtd_pr_avd_emerg("Not in autoscan\n");
				}
			}
		}
		return;
	}
	/*Neet to translate color std from Realtek to LGE	*/
	g_tLGEVideoMode = TransColorSysRTK2LGE(g_ucVDMode);
	/*Input color std is supported by this TV.*/
	// Only ATV need to do CheckSupportedColorSys
	if ( _SRC_TV != g_ucSrcType || 0 == CheckSupportedColorSys(g_tLGEVideoMode)) {

		//rtd_pr_avd_debug("Not ATV source, don't do CheckSupportedColorSys()\n");
		/*480i*/
		if (_MODE_480I == ucModeCur) {
			g_ucModeCurr = _MODE_480I;
#ifndef CONFIG_CUSTOMER_TV010
			if (Support480I == false && Support576I == true) {
				ptLGETimingInfo->active.h = 576;
				ptLGETimingInfo->active.w = 704;
				ptLGETimingInfo->vFreq = 500;
			}
			else
#endif
			{
				ptLGETimingInfo->active.h = 480;
				ptLGETimingInfo->active.w = 704;

#ifdef CONFIG_CUSTOMER_TV010
			if (ptAVDScalerDispInfo->IVTotal > 22)
				ptLGETimingInfo->active.h = (ptAVDScalerDispInfo->IVTotal - 22)*2;
#endif
				if (g_ucSrcType == _SRC_TV) {
					ptLGETimingInfo->vFreq = 599;
				}
				else {
					ptLGETimingInfo->vFreq = 600;
				}
			}
		} else if (_MODE_576I == ucModeCur) {
			/*576i*/
			g_ucModeCurr = _MODE_576I;
#ifndef CONFIG_CUSTOMER_TV010
			if (Support480I == true && Support576I == false) {
				ptLGETimingInfo->active.h = 480;
				ptLGETimingInfo->active.w = 704;
				if (g_ucSrcType == _SRC_TV) {
					ptLGETimingInfo->vFreq = 599;
				}
				else {
					ptLGETimingInfo->vFreq = 600;
				}
			}
			else
#endif
			{
				ptLGETimingInfo->active.h = 576;
				ptLGETimingInfo->active.w = 704;
				ptLGETimingInfo->vFreq = 500;
#ifdef CONFIG_CUSTOMER_TV010
			if (ptAVDScalerDispInfo->IVTotal > 25)
				ptLGETimingInfo->active.h = (ptAVDScalerDispInfo->IVTotal - 25)*2;
#endif
			}
		} else {
			ptLGETimingInfo->active.h = 576;
			ptLGETimingInfo->active.w = 704;
			g_ucModeCurr = _MODE_576I;
			ptLGETimingInfo->vFreq = 500;
		}
		ptLGETimingInfo->active.x = ptAVDScalerDispInfo->IPH_ACT_STA_PRE;
#ifdef CONFIG_CUSTOMER_TV010
		ptLGETimingInfo->active.y = ptAVDScalerDispInfo->IPV_ACT_STA_PRE;
#else
		ptLGETimingInfo->active.y = ptAVDScalerDispInfo->IPV_ACT_LEN_PRE;
#endif
		/*this is not used in HAL function*/
		ptLGETimingInfo->format = 0;
		ptLGETimingInfo->hFreq = (ptAVDScalerDispInfo->IHFreq) * 100;
		ptLGETimingInfo->hTotal = ptAVDScalerDispInfo->IHTotal;
		ptLGETimingInfo->vTotal = ptAVDScalerDispInfo->IVTotal;
		ptLGETimingInfo->hPorch = ptAVDScalerDispInfo->IPH_ACT_STA_PRE;
		ptLGETimingInfo->vPorch = ptAVDScalerDispInfo->IPV_ACT_LEN_PRE;
		ptLGETimingInfo->scanType = Interlaced;/* 0 : Interlaced, 1 : Progressive*/
		ptLGETimingInfo->vdLock = TRUE;
		ptLGETimingInfo->HLock = TRUE;
		ptLGETimingInfo->VLock = TRUE;
		ptLGETimingInfo->standard = g_tLGEVideoMode;
	} /* end of if (0 == CheckSupportedColorSys(g_tLGEVideoMode))*/
	/*Input color std is "NOT" supported by this TV.*/
	else {
		ClearAVDLGETimingInfo();
	}
#if 0
	rtd_pr_avd_debug( "active.h=%d\n", ptLGETimingInfo->active.h);
	rtd_pr_avd_debug( "active.w=%d\n", ptLGETimingInfo->active.w);
	rtd_pr_avd_debug( "active.x=%d\n", ptLGETimingInfo->active.x);
	rtd_pr_avd_debug( "active.y=%d\n", ptLGETimingInfo->active.y);
	rtd_pr_avd_debug( "format=%d\n", ptLGETimingInfo->format);
	rtd_pr_avd_debug( "hFreq=%d\n", ptLGETimingInfo->hFreq);
	rtd_pr_avd_debug( "vFreq=%d\n", ptLGETimingInfo->vFreq);
	rtd_pr_avd_debug( "hTotal=%d\n", ptLGETimingInfo->hTotal);
	rtd_pr_avd_debug( "vTotal=%d\n", ptLGETimingInfo->vTotal);
	rtd_pr_avd_debug( "hPorch=%d\n", ptLGETimingInfo->hPorch);
	rtd_pr_avd_debug( "vPorch=%d\n", ptLGETimingInfo->vPorch);
	rtd_pr_avd_debug( "scanType=%d\n", ptLGETimingInfo->scanType);
	rtd_pr_avd_debug( "vdLock=%d\n", ptLGETimingInfo->vdLock);
	rtd_pr_avd_debug( "HLock=%d\n", ptLGETimingInfo->HLock);
	rtd_pr_avd_debug( "VLock=%d\n", ptLGETimingInfo->VLock);
#endif
}

static unsigned char GetLGESupportedMode(void)
{
	unsigned char ucMode = 0;
	if ((g_tSupportedColorSys & KADP_COLOR_SYSTEM_NTSC_M) == KADP_COLOR_SYSTEM_NTSC_M
		|| (g_tSupportedColorSys & KADP_COLOR_SYSTEM_NTSC_443) == KADP_COLOR_SYSTEM_NTSC_443
		|| (g_tSupportedColorSys & KADP_COLOR_SYSTEM_PAL_60) == KADP_COLOR_SYSTEM_PAL_60
		|| (g_tSupportedColorSys & KADP_COLOR_SYSTEM_PAL_M) == KADP_COLOR_SYSTEM_PAL_M)
	{
		ucMode = _MODE_480I;
	}
	if ((g_tSupportedColorSys & KADP_COLOR_SYSTEM_PAL_G) == KADP_COLOR_SYSTEM_PAL_G
		|| (g_tSupportedColorSys & KADP_COLOR_SYSTEM_PAL_NC_358) 	== KADP_COLOR_SYSTEM_PAL_NC_358
		|| (g_tSupportedColorSys & 	KADP_COLOR_SYSTEM_SECAM) == KADP_COLOR_SYSTEM_SECAM
//		|| (g_tSupportedColorSys & KADP_COLOR_SYSTEM_PAL_60) == KADP_COLOR_SYSTEM_PAL_60
		|| g_tSupportedColorSys == -1)
	{
		if (_MODE_480I == ucMode) {
			/*Mixed mode*/
			ucMode = _MODE_MIXED;
		} else {
			ucMode = _MODE_576I;
		}
	}
	return ucMode;
}

void ClearAVDLGETimingInfo(void)
{
	KADP_VFE_AVD_TIMING_INFO_T *ptLGETimingInfo = Get_AVD_LGETiminginfo();
	unsigned char ucModeSupported = 0;
#ifdef CONFIG_CUSTOMER_TV010
	StructDisplayInfo *ptAVDScalerDispInfo;
#endif
	/*rtd_pr_avd_debug("ClearAVDLGETimingInfo\n");*/
	/*rtd_pr_avd_debug("file=%s, function=%s, line=%d\n", __FILE__, __FUNCTION__, 	__LINE__);*/
	memset(ptLGETimingInfo, 0, sizeof(KADP_VFE_AVD_TIMING_INFO_T));

	// Non-ATV source don't need to check supported color std
	if ( _SRC_TV != g_ucSrcType)
		return;

	ucModeSupported = GetLGESupportedMode();
	/*Mixed Mode*/
	if (_MODE_MIXED == ucModeSupported) {
		/*Use previous mode*/
		if (_MODE_UNKNOWN != g_ucModeCurr) {
			ucModeSupported = g_ucModeCurr;
		} else {
			ucModeSupported = _MODE_480I;
		}
	}
	/*480i*/
	if (_MODE_480I == ucModeSupported) {
		ptLGETimingInfo->active.h = 480;
		ptLGETimingInfo->active.w = 704;
		ptLGETimingInfo->vFreq = 599;
		//} else if (_MODE_576I == ucModeSupported) {
	} else {
		/*576i*/
		ptLGETimingInfo->active.h = 576;
		ptLGETimingInfo->active.w = 704;
		ptLGETimingInfo->vFreq = 500;
	}

#ifdef CONFIG_CUSTOMER_TV010
	if(vdc_needopenpowerflag != TRUE)
	{
		ptAVDScalerDispInfo = Get_AVD_ScalerDispinfo();;
		if (576 == ptLGETimingInfo->active.h) {
			if (ptAVDScalerDispInfo != NULL && ptAVDScalerDispInfo->IVTotal > 25)
				ptLGETimingInfo->active.h = (ptAVDScalerDispInfo->IVTotal - 25)*2;
		}
		else {
			if (ptAVDScalerDispInfo != NULL && ptAVDScalerDispInfo->IVTotal > 22)
				ptLGETimingInfo->active.h = (ptAVDScalerDispInfo->IVTotal - 22)*2;
		}
	}
#endif

	ptLGETimingInfo->standard = KADP_VIDEO_AVDEC_MODE_UNKNOWN;
	//g_ucVDMode = _MODE_UNKNOWN;
}

static KADP_VFE_AVD_AVDECODER_VIDEO_MODE_T TransColorSysRTK2LGE(unsigned char a_ucVDMode)
{
	//rtd_pr_avd_info("#####[%s(%d)] a_ucVDMode=%x\n",__func__, __LINE__, a_ucVDMode);
#ifdef CONFIG_SUPPORT_COLORSYSTEM_MANUAL_SET
    //*****Manual set color system didin't display "No signal" ****//
    if(0xff != a_ucVDMode) {
		switch (a_ucVDMode) {
		/*480i*/
		case VDC_MODE_NTSC:
			return KADP_VIDEO_AVDEC_MODE_NTSC;
		case VDC_MODE_NTSC443:
			 if (TV_SECAM == g_tSupportedColorSys){
                return KADP_VIDEO_AVDEC_MODE_SECAM;
             }else if(TV_SetPALN == g_tSupportedColorSys){
                return KADP_VIDEO_AVDEC_MODE_PAL_NC_358;
             }else if(TV_PAL == g_tSupportedColorSys){
                return KADP_VIDEO_AVDEC_MODE_PAL;
             }else {
                return KADP_VIDEO_AVDEC_MODE_NTSC_443;
             }
		case VDC_MODE_PAL60:
             if (TV_PAL == g_tSupportedColorSys){
                return KADP_VIDEO_AVDEC_MODE_PAL;
             }else{
                return KADP_VIDEO_AVDEC_MODE_PAL_60;
             }
		case VDC_MODE_PALM:
			return KADP_VIDEO_AVDEC_MODE_PAL_M;
		/*576i*/
		case VDC_MODE_NTSC50:
			/*FixMe*/
			//return KADP_VIDEO_AVDEC_MODE_UNKNOWN_625;  
			//return KADP_VIDEO_AVDEC_MODE_PAL;
            if (TV_ATSC == g_tSupportedColorSys){
                return KADP_VIDEO_AVDEC_MODE_NTSC;
			}else if (TV_SetPALM == g_tSupportedColorSys){
                return KADP_VIDEO_AVDEC_MODE_PAL_M;
			}else if(TV_NTSC443 == g_tSupportedColorSys) {
		        return KADP_VIDEO_AVDEC_MODE_NTSC_443;
  			}else if(TV_PAL60 == g_tSupportedColorSys) {
		        return KADP_VIDEO_AVDEC_MODE_PAL_60;
  			}else {
		        return KADP_VIDEO_AVDEC_MODE_PAL;
            }
		case VDC_MODE_PALN:
			return KADP_VIDEO_AVDEC_MODE_PAL_NC_358;
		case VDC_MODE_PALI:
			return KADP_VIDEO_AVDEC_MODE_PAL;
		case VDC_MODE_SECAM:
			return KADP_VIDEO_AVDEC_MODE_SECAM;
		default:
			rtd_pr_avd_debug("Color std error, a_ucVDMode=%x\n", a_ucVDMode);
		}
    }
   	return KADP_VIDEO_AVDEC_MODE_UNKNOWN;
#else  //*******Original Setting*******//
    if(0xff != a_ucVDMode) {
        switch (a_ucVDMode) {               
		/*480i*/
		case VDC_MODE_NTSC:
			return KADP_VIDEO_AVDEC_MODE_NTSC;
		case VDC_MODE_NTSC443:
			return KADP_VIDEO_AVDEC_MODE_NTSC_443;
		case VDC_MODE_PAL60:
			return KADP_VIDEO_AVDEC_MODE_PAL_60;
		case VDC_MODE_PALM:
			return KADP_VIDEO_AVDEC_MODE_PAL_M;
		/*576i*/
		case VDC_MODE_NTSC50:
			/*FixMe*/
			//return KADP_VIDEO_AVDEC_MODE_UNKNOWN_625;
			return KADP_VIDEO_AVDEC_MODE_PAL;
		case VDC_MODE_PALN:
			return KADP_VIDEO_AVDEC_MODE_PAL_NC_358;
		case VDC_MODE_PALI:
			return KADP_VIDEO_AVDEC_MODE_PAL;
		case VDC_MODE_SECAM:
			return KADP_VIDEO_AVDEC_MODE_SECAM;
		default:
			rtd_pr_avd_debug("Color std error, a_ucVDMode=%x\n", a_ucVDMode);
		}
	}
    return KADP_VIDEO_AVDEC_MODE_UNKNOWN;
#endif
}


char Scaler_AVD_GetLGEColorSystem(KADP_VFE_AVD_AVDECODER_VIDEO_MODE_T *a_ptColorSystem)
{
	char ret = 0;
	StructDisplayInfo *p_tAVDDispInfo = Get_AVD_ScalerDispinfo();
	unsigned char SupportSystem = 0;
	SupportSystem = GetLGESupportedMode();
	if (DETECT_STAGE_FINAL != Scaler_AVD_GetDetectStage()) {
		*a_ptColorSystem = KADP_VIDEO_AVDEC_MODE_UNKNOWN;
	} else {
		KADP_VFE_AVD_AVDECODER_VIDEO_MODE_T CurrentVideoMode = TransColorSysRTK2LGE(g_ucVDMode);
		/*Input color std is supported by this TV.*/
		if (_SRC_TV != g_ucSrcType || 0 == CheckSupportedColorSys(CurrentVideoMode)) {
			*a_ptColorSystem = CurrentVideoMode;
		}
		/*Input color std is "NOT" supported by this TV.*/
		else {
			if (_MODE_480I == p_tAVDDispInfo->ucMode_Curr) {
#if 0 /* Modify based on the Question_detail.xlsx*/
				*a_ptColorSystem =  VIDEO_AVDEC_MODE_UNKNOWN_525;
#else
				if (SupportSystem == _MODE_576I) {
					*a_ptColorSystem = KADP_VIDEO_AVDEC_MODE_PAL;
				}
				else {
					*a_ptColorSystem = KADP_VIDEO_AVDEC_MODE_NTSC;
				}
#endif
			} else if (_MODE_576I == p_tAVDDispInfo->ucMode_Curr) {
#if 0 /* Modify based on the Question_detail.xlsx*/
				*a_ptColorSystem = VIDEO_AVDEC_MODE_UNKNOWN_625;
#else
				if (SupportSystem == _MODE_480I) {
					*a_ptColorSystem = KADP_VIDEO_AVDEC_MODE_NTSC;
				}
				else {
					*a_ptColorSystem = KADP_VIDEO_AVDEC_MODE_PAL;
				}
#endif
			} else {
				*a_ptColorSystem = KADP_VIDEO_AVDEC_MODE_UNKNOWN;
			}
		}
	}

	if(get_avd_print_flag()){
		rtd_pr_avd_info("#####[%s(%d)] a_ptColorSystem=%x, ucMode_Curr=%x\n", __func__, __LINE__, *a_ptColorSystem, p_tAVDDispInfo->ucMode_Curr);
	}

	return ret;
}

#if 0
static void AVD_check_av_color_std(unsigned char bInCheckMode)
{
	unsigned char mode = drvif_module_vdc_ReadMode(VDC_SET);
	unsigned char getmode;
	mode = mode | 0x10;

	if (bInCheckMode && mode != ZAUTO) {
		if ((drv_module_vdc_GetVdcCurrMode() == VDC_MODE_PALI) && (mode == ZPAL_I))
			return;
		if ((drv_module_vdc_GetVdcCurrMode() == VDC_MODE_NTSC) && (mode == ZNTSC))
			return;
	}
	switch (mode) {
	case ZAUTO:
		if ((drvif_module_vdc_ReadAutomode()) == 0) {
			UINT8 i = 0;
			drvif_module_vdc_SetMode(VDC_MODE_AUTO);
			for (i = 0 ; i < 50 ; i++) {
				if (drvif_module_vdc_CheckHVisLocked()) {
					rtd_pr_avd_debug("[%x]vd lock:%x\n", i, (drvif_video_status_reg(VDC_STATUS_REG1)));
					break;
				}
				msleep(10);
			}
		}
		break;
	case ZNTSC:
		getmode = drv_module_vdc_GetVdcCurrMode();
		if (getmode == ZNTSC_443)
			drvif_module_vdc_SetMode(VDC_MODE_NTSC443);
		else
			drvif_module_vdc_SetMode(VDC_MODE_NTSC);
		break;
	case ZPAL_I:
		drvif_module_vdc_SetMode(VDC_MODE_PALI);
		break;
	case ZNTSC_50:
		drvif_module_vdc_SetMode(VDC_MODE_NTSC50);
		break;
	case ZNTSC_443:
		drvif_module_vdc_SetMode(VDC_MODE_NTSC443);
		break;
	case ZPAL_M:
		drvif_module_vdc_SetMode(VDC_MODE_PALM);
		break;
	case ZPAL_N:
		drvif_module_vdc_SetMode(VDC_MODE_PALN);
		break;
	case ZPAL_60:
		drvif_module_vdc_SetMode(VDC_MODE_PAL60);
		break;
	case ZSECAM:
	case ZSECAML:
	case ZSECAMLA:
		drvif_module_vdc_SetMode(VDC_MODE_SECAM);
		break;
	default:
		break;
	}
}
#endif

void AVD_OnlineMeasureError_Handler(int srcType)
{
	StructDisplayInfo *ptAVDScalerDispInfo = Get_AVD_ScalerDispinfo();
	ModeInformationType *ptAVDScalerTimingInfo = Get_AVD_ScalerTiminginfo();
	/*For ATV, only go to init state of Scaler_AVD_DetectTiming().*/
	if (_SRC_TV == g_ucSrcType) {
#if 0
		if (AVD_EXTERNAL_DEMOD == g_tAVDDemodType) {
			//drvif_module_vdc_SoftReset();
		}
#endif
		return;
	}
	avd_good_timing_ready = FALSE;
	memset(ptAVDScalerDispInfo, 0, sizeof(StructDisplayInfo));
	memset(ptAVDScalerTimingInfo, 0, sizeof(ModeInformationType));
	Scaler_AVD_SetDetectStage(DETECT_STAGE_INIT);
}

void Scaler_AVD_ClearTimingInfo(void)
{
	StructDisplayInfo *ptAVDScalerDispInfo = Get_AVD_ScalerDispinfo();
	ModeInformationType *ptAVDScalerTimingInfo = Get_AVD_ScalerTiminginfo();
	avd_good_timing_ready = FALSE;
	memset(ptAVDScalerDispInfo, 0, sizeof(StructDisplayInfo));
	memset(ptAVDScalerTimingInfo, 0, sizeof(ModeInformationType));
	Scaler_AVD_SetDetectStage(DETECT_STAGE_INIT);

#if jj_test_print
	rtd_pr_avd_emerg("Scaler_AVD_ClearTimingInfo\n");
#endif
}

char Scaler_AVD_SetDemodType(KADP_VFE_AVD_DEMOD_TYPE a_tDemodType)
{
	char  cRet = 0;
	if (a_tDemodType == KADP_AVD_INTERNAL_DEMOD || a_tDemodType == KADP_AVD_EXTERNAL_DEMOD) {
		g_tAVDDemodType = a_tDemodType;
		cRet = 0;
	} else {
		rtd_pr_avd_notice("Scaler_AVD_SetDemodType fail, a_tDemodType=%d\n", a_tDemodType);
		cRet = -1;
	}
	return cRet;
}
KADP_VFE_AVD_DEMOD_TYPE Scaler_AVD_GetDemodType(void)
{
	return g_tAVDDemodType;
}

/*Scart CVBS*/
char Scaler_AVD_Scart_Connect(unsigned short srcinput)
{
	char cRet = 0;
	unsigned char ucSize = 0, i = 0;
	StructSourceInput *srcinput_pt = PcbSource_GetTable();
	rtd_pr_avd_debug("Scaler_AVD_SCART_CONNECT()\n");

	down(get_avd_power_semaphore());
	drvif_module_vdc_PowerCtrl(ON, _SRC_SCART);
	Scaler_AVD_Set_VDC_Power_Setting(VDC_Power_saving, _SRC_SCART);
	up(get_avd_power_semaphore());
	/*It means scart cvbs*/
	g_ucSrcType = _SRC_SCART;
	ucSize = PcbSource_GetTableSize();
	for (i = 0 ; i < ucSize ; i++) {
		if (srcinput_pt->src_index == srcinput)
			break;
		srcinput_pt++;
	}
	if (i == ucSize) {
		rtd_pr_avd_debug("Can't find srcinput in pcbsource=%x\n", srcinput);
		return -1;
	}
	g_tAVDScalerDispInfo.input_src = i;
	g_tAVDScalerDispInfo.data_array_index = i;
	g_tAVDScalerDispInfo.osd_input_src = i;
	g_ucScartMode = _MODE_UNKNOWN;
	drvif_module_vdc_SelectInput(_SRC_SCART, (VDC_INITIAL_OPTIONS *)srcinput_pt->InitOptions);
	drvif_module_vdc_set_vsync_freerun_enable(_DISABLE);
	return cRet;
}

char Scaler_AVD_ScartRGB_Connect(unsigned short srcinput)
{
	char cRet = 0;
	unsigned char ucSize = 0, i = 0;
	StructSourceInput *srcinput_pt = PcbSource_GetTable();
	rtd_pr_avd_debug("Scaler_AVD_SCART_CONNECT()\n");
	down(get_avd_power_semaphore());
	drvif_module_vdc_PowerCtrl(ON, _SRC_SCART_RGB);
	Scaler_AVD_Set_VDC_Power_Setting(VDC_Power_saving, _SRC_SCART_RGB);
	up(get_avd_power_semaphore());

	g_ucSrcType = _SRC_SCART_RGB;
	ucSize = PcbSource_GetTableSize();
	for (i = 0 ; i < ucSize ; i++) {
		if (srcinput_pt->src_index == srcinput)
			break;
		srcinput_pt++;
	}
	if (i == ucSize) {
		rtd_pr_avd_debug("Can't find srcinput in pcbsource=%x\n", srcinput);
		return -1;
	}
	g_tAVDScalerDispInfo.input_src = i;
	g_tAVDScalerDispInfo.data_array_index = i;
	g_tAVDScalerDispInfo.osd_input_src = i;
	g_ucScartMode = _MODE_UNKNOWN;
	drvif_module_vdc_SelectInput(_SRC_SCART_RGB, (VDC_INITIAL_OPTIONS *)srcinput_pt->InitOptions);
	drvif_module_vdc_set_vsync_freerun_enable(_DISABLE);
	return cRet;
}

char Scaler_AVD_Scart_Disconnect(void)
{
	char cRet = 0;
	rtd_pr_avd_debug("Scaler_AVD_Scart_Disconnect()\n");
	vdc_LockStatus = 0;
	Scaler_VBI_disconnect();
	ClearAVDLGETimingInfo();
	Scaler_AVD_ClearTimingInfo();
	memset(&g_tAVDScalerDispInfo, 0, sizeof(StructDisplayInfo));
	memset(&g_tAVDScalerTimingInfo, 0, sizeof(ModeInformationType));
	return cRet;
}

void Scaler_AVD_Set_Scart_Mode(void)
{
	/*Scart RGB*/
	if (_SRC_SCART_RGB == g_ucSrcType) {
		drvif_video_SCART_RGB_set(_ENABLE);
		drvif_module_vdc_Cpeak_set(_ENABLE);
		drvif_module_vdc_auto_secamFlag_select(SEL_PALh_SECAMl);
		module_vdc_SECAM_flag_select(0);
	} else if (_SRC_SCART == g_ucSrcType) {
		/*Scart CVBS*/
		drvif_video_SCART_RGB_set(_DISABLE);
		drvif_module_vdc_Cpeak_set(_DISABLE);
		drvif_module_vdc_auto_secamFlag_select(SEL_PAL);
		module_vdc_SECAM_flag_select(1);
	} else {
		rtd_pr_avd_debug("Scaler_AVD_Set_Scart_Mode scart mode error=%d\n", g_ucSrcType);
	}
	drvif_module_vpq_YcSeparation(_SRC_SCART, _ENABLE);
}
#ifdef CONFIG_SUPPORT_SRC_ADC
char Scaler_AVD_GetScartFastBlankingStatus(KADP_VFE_AVD_SCART_FB_STATUS_T *pStatus)
{
	int ret = 0;
	adc_scart_ctrl1_RBUS scart_ctrl1_reg;//sunray modify
	ADC_Power_Control(ON);
	scart_ctrl1_reg.regValue = IoReg_Read32(ADC_SCART_CTRL1_reg);
	scart_ctrl1_reg.vdadc_sct_ctrl_7_6= 0x1;
	scart_ctrl1_reg.vdadc_sct_ctrl_5_4= 0x0;
	scart_ctrl1_reg.vdadc_sct_ctrl_3 = 0x0;
	scart_ctrl1_reg.vdadc_sct_ctrl_2 = 0x1;
	scart_ctrl1_reg.vdadc_sct_ctrl_1 = 0x1;
	scart_ctrl1_reg.vdadc_sct_ctrl_0 = 0x1;
	IoReg_Write32(ADC_SCART_CTRL1_reg, scart_ctrl1_reg.regValue);
	msleep(30);
	if (drvif_module_vdc_read_FSW_value() >= cSCART_FSW_Level) {
		g_ucScartMode = KADP_VIDEO_DDI_FB_STATUS_RGB;
	} else {
		g_ucScartMode = KADP_VIDEO_DDI_FB_STATUS_CVBS;
	}
	*pStatus = g_ucScartMode;
	ADC_Power_Control(OFF);
	return ret;
}
#endif
char Scaler_AVD_GetScartID(KADP_VFE_AVD_SCART_AR_T *pScartAr)
{
	int ret = 0;
	SCART_PIN8_LEVEL Scart;
	Scart = getSCART_pin8();
	if (Scart == SCART_PIN8_LEVEL_16_9)
		*pScartAr = KADP_VIDEO_DDI_SCART_AR_16_9;
	else if (Scart == SCART_PIN8_LEVEL_4_3)
		*pScartAr = KADP_VIDEO_DDI_SCART_AR_4_3;
	else
		*pScartAr = KADP_VIDEO_DDI_SCART_AR_INVALID;
	return ret;
}

int lsadc_init(int index, int voltage_mode)
{
// FixMe

#if 0
	/*st_lsadc_ctrl_RBUS st_lsadc_ctrl_reg;*/
	/*st_lsadc_analog_ctrl_RBUS 	st_lsadc_analog_ctrl_reg;*/
	st_pad0_RBUS st_pad0_reg;
	if (index >= 8)
		return -1;
	st_pad0_reg.regValue = 0;
	st_pad0_reg.pad0_active = 1;
	st_pad0_reg.pad0_sw = index;
	if (voltage_mode)
		st_pad0_reg.pad0_ctrl = 0;
	else
		st_pad0_reg.pad0_ctrl = 1;
	rtd_outl(ST_PAD0_VADDR + index * 4, st_pad0_reg.regValue);
#endif
	return 0;
}

int lsdac_get(int index)
{
// FixMe

#if 0
	st_lsadc_ctrl_RBUS st_lsadc_ctrl_reg;
	st_pad0_RBUS st_pad0_reg;
	if (index >= 8)
		return -1;
	st_lsadc_ctrl_reg.regValue = rtd_inl(ST_LSADC_CTRL_VADDR);
	if (st_lsadc_ctrl_reg.enable == 0) {
		st_lsadc_ctrl_reg.enable = 1;
		rtd_outl(ST_LSADC_CTRL_VADDR, st_lsadc_ctrl_reg.regValue);
	}
	st_pad0_reg.regValue = rtd_inl(ST_PAD0_VADDR + index * 4);
	return st_pad0_reg.adc_val0;
#endif

// FixMe
	return 0;
}

int getSCART_pin8(void)
{
	char *pin_name[3] = {"PIN_SCART_LEVEL_NONE", "PIN_SCART_LEVEL_16_9", "PIN_SCART_LEVEL_4_3"};
	unsigned long long value;
	int idx, upper, lower;
	int adc_val = 0, i = 0;
	static bool is_lsadc_init;
	for (i = 0 ; i < 3 ; i++) {
		if ((pcb_mgr_get_enum_info_byname(pin_name[i], &value) == 0) && (GET_PIN_TYPE(value) == PCB_PIN_TYPE_LSADC)) {
			if (!is_lsadc_init) {
				lsadc_init(value, 1);
				msleep(10);
				is_lsadc_init = true;
			}
			idx  = GET_PIN_INDEX(value);
			upper = GET_LSADC_HITVALUE(value) + GET_LSADC_TOLERANCE(value);
			lower = GET_LSADC_HITVALUE(value) - GET_LSADC_TOLERANCE(value);
			adc_val = lsdac_get(idx);
			if ((adc_val >= lower) && (adc_val <= upper)) {
				return i;
			}
		}
	}
	return 0;
}

/*static unsigned char fw_video_detectmode_avd(unsigned char CurrentSrc)*/
/* porting from fw_video_detectmode()*/
void fw_video_SetModeInfo(unsigned char CurrentSrc)
{
	if (CurrentSrc == _SRC_TV || CurrentSrc == _SRC_CVBS) {
		if (drvif_module_vdc_GetFrameRate()) {
			drv_module_vdc_SetVarFieldVTotal(263);
			rtd_pr_avd_debug("[VDC] 480i \n");
		} else {
			drv_module_vdc_SetVarFieldVTotal(313);
			rtd_pr_avd_debug("[VDC] 576i \n");
		}
#if 0	/* jj modify 20150814, tv reset move to channel change */
		if (CurrentSrc == _SRC_TV)
			drvif_module_vdc_tv_Reset();
		else
			drvif_module_vdc_ResetReg();
#endif

#ifdef AV_HSYNC_PULSE_START_REFINE
        if (CurrentSrc == _SRC_TV) {
            if(drvif_module_vdc_ReadMode(VDC_DETECT) == VDC_MODE_SECAM){
                module_vdc_Hactive_Start(FwMode, 0x93);//for ATV SECAM
            }else{
                module_vdc_Hactive_Start(FwMode, 0xc2);//for ATV
            }
		}
#endif

		if (CurrentSrc == _SRC_CVBS) {
			drvif_module_vdc_ResetReg();
			module_vdc_set_Clamp_position(drvif_module_vdc_ReadMode(VDC_SET));
		}
	}
}



//For K4L
bool Scaler_AVD_SetChannelChange(void)
{
	StructDisplayInfo *ptAVDScalerDispInfo  = Get_AVD_ScalerDispinfo();
	ModeInformationType *ptAVDScalerTimingInfo = Get_AVD_ScalerTiminginfo();
	int ret = 0;
	#ifdef _PRINT_AVD_COST_TIME
	rtd_pr_avd_info("_PRINT_AVD_COST_TIME: %s(%d) time = %d\n",__FUNCTION__,__LINE__,IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg)/90);
	#endif
	rtd_pr_avd_info("####[%s(%d)] status=%d\n", __func__, __LINE__, get_AVD_Global_Status());
	if(SRC_CONNECT_DONE == get_AVD_Global_Status())
		return ret;

	avd_good_timing_ready = FALSE;
	Scaler_AVD_Set_ChannelChange(true);
	Scaler_AVD_SetIsChannelChange(true);

	if (KADP_AVD_EXTERNAL_DEMOD == g_tAVDDemodType) {
		drvif_module_vdc_set_external_ch_change_flag(TRUE);
	}else{
		drvif_module_vdc_set_ch_change_flag(TRUE);
	}
	memset(ptAVDScalerDispInfo, 0, sizeof(StructDisplayInfo));
	memset(ptAVDScalerTimingInfo, 0, sizeof(ModeInformationType));

	drvif_module_vdc_SoftReset();

	return ret;
}

// Channel change at this time
void Scaler_AVD_SetChangeSystem(bool Change)
{
	rtd_pr_avd_debug("Scaler_AVD_SetChangeSystem\n");
	if(_SRC_CVBS == g_ucSrcType) {
		rtd_pr_avd_debug("[AVD] Not TV Source!! \n");
		return;
	}

	Scaler_AVD_Set_g_bStartChangeSystem(Change);

	if (true == Change) {
		Scaler_AVD_ClearTimingInfo();
	}

	Scaler_AVD_SetDetectStage(DETECT_STAGE_INIT);
	avd_good_timing_ready = FALSE;
	ClearAVDLGETimingInfo();
}

bool Scaler_AVD_GetRunSearchState(unsigned char a_ucChannel)
{
	if (SLR_MAIN_DISPLAY  == a_ucChannel)
		return g_bMainRunSearchState;
	else
		return g_bSubRunSearchState;
}

void Scaler_AVD_SetRunSearchState(unsigned char a_ucChannel, bool a_bFlag)
{
	rtd_pr_avd_debug("Scaler_AVD_SetRunSearchState=%x, a_ucChannel=%x\n", a_bFlag, a_ucChannel);


	if (SLR_MAIN_DISPLAY  == a_ucChannel)
		g_bMainRunSearchState = a_bFlag;
	else
		g_bSubRunSearchState = a_bFlag;

}

unsigned char Scaler_AVD_GetAvdSource(void)
{
	return g_ucSrcType;
}

#if 0
bool Scaler_AVD_GetDetectTimingAgain(unsigned char a_ucChannel)
{
	return g_bAVDDetectTimingAgain;
}
void Scaler_AVD_SetDetectTimingAgain(unsigned char a_ucChannel, bool a_bFlag)
{
	g_bAVDDetectTimingAgain = a_bFlag;
}
#endif

void Scaler_AVD_SetAutoTuneMode(bool AutoTuneMode)
{
	g_AutoTuneMode = AutoTuneMode;
}

bool Scaler_AVD_GetAutoTuneMode(void)
{
	return g_AutoTuneMode;
}


void Scaler_AVD_SetHalVscConnect(unsigned char a_ucChannel,bool AvdVscConnect)
{
	rtd_pr_avd_info("Scaler_AVD_SetHalVscConnect a_ucChannel=%d connect=%d\n", a_ucChannel,AvdVscConnect);
	if (SLR_MAIN_DISPLAY  == a_ucChannel)
		g_AVD_Main_HalVscConnect = AvdVscConnect;
	else
		g_AVD_Sub_HalVscConnect = AvdVscConnect;
}

bool Scaler_AVD_GetHalVscConnect(unsigned char a_ucChannel)
{
	if (SLR_MAIN_DISPLAY  == a_ucChannel)
		return g_AVD_Main_HalVscConnect;
	else
		return g_AVD_Sub_HalVscConnect;
}

/* True: Timing change
   False: Timing is the same */
//unsigned char Scaler_AVD_CheckATVTimingChange(void)
unsigned char Scaler_AVD_CheckATVTimingChange(unsigned char a_ucChannel)
{
	// ATV change channel with different color std
	//rtd_pr_avd_debug("Scaler_AVD_GetRunSearchState=%x, channel=%x, src=%x\n", Scaler_AVD_GetRunSearchState(a_ucChannel), a_ucChannel, Scaler_AVD_GetAvdSource());
	if (Scaler_AVD_GetAvdSource() == _SRC_TV && Scaler_AVD_GetRunSearchState(a_ucChannel))
	{
		rtd_pr_avd_info("#####[%s(%d)]Scaler_AVD_GetRunSearchState=1, channel=%d(0-main,1-sub)\n",__func__,__LINE__, a_ucChannel);
		Scaler_AVD_SetRunSearchState(a_ucChannel, false);
		return true;
	}
#ifdef CONFIG_CUSTOMER_TV010
	else if (TRUE == g_bFVBiasRunScaler)
	{
		return true;
	}
#endif
	return false;
}

char Scaler_AVD_IsSupportedColorSys(void)
{
	return CheckSupportedColorSys(TransColorSysRTK2LGE(g_ucVDMode));
}


//add for audio requirements
void  IFD_SIF_ADC_Pll_Initial(void)
{
//K3L
/*
	rtd_pr_avd_emerg("\n[SIF_ADC]IFD_SIF_ADC_Pll_Initial\n");
	//PLDIF
	//rtd_maskl(0xb80004c8, 0xfffffff8, 0x0);
	//rtd_maskl(0xb80004c8, 0xfffffff8, 0x6);
	//rtd_maskl(0xb80004c8, 0xfffffff8, 0x7);
	//rtd_maskl(0xb80004c8, 0x7fffffff, 0x80000000);

	//PLL27X
	//rtd_maskl(PLL27X_REG_SYS_PLL_27X2_reg, 0xfff8ffff, 0x00070000);
	//rtd_maskl(0xb80004d8, 0xffffffe8, 0x17);
	//rtd_maskl(PLL27X_REG_SYS_PLL_27X4_reg, 0xfDfffefe, 0x02000100);
	//PLL27X_IFADC
	rtd_maskl(PLL27X_REG_IFADC_1_reg, 0xfffffeef, 0x00000110);
	rtd_maskl(PLL27X_REG_IFADC_3_reg, 0xfffffff8, 0x7);

	//rtd_maskl(PLL27X_REG_SYS_PLL_27X4_reg, 0XFDF3FEFF, 0x020C0100);//Enable Output
*/
}

void  IFD_SIF_ADC_Initial_Flow(void)
{
/*
	//K4L
	rtd_pr_avd_debug( "\n[SIF_ADC]IFD_SIF_ADC_Initial_Flow\n");
	tv_sb1_ana_ifadc_ctrl1_RBUS ifadc_ctrl1;
	tv_sb1_ana_ifadc_ctrl2_RBUS ifadc_ctrl2;

	tv_sb1_ana_iqadc_ctrl0_RBUS iqadc_ctrl0;
	tv_sb1_ana_iqadc_ctrl2_RBUS iqadc_ctrl2;
	tv_sb1_ana_iqadc_ctrl3_RBUS iqadc_ctrl3;
	tv_sb1_ana_iqadc_ctrl4_RBUS iqadc_ctrl4;
	tv_sb1_ana_iqadc_ctrl5_RBUS iqadc_ctrl5;
	tv_sb1_ana_iqadc_ctrl7_RBUS iqadc_ctrl7;

	ifadc_ctrl1.regValue = IoReg_Read32(TV_SB1_ANA_IFADC_CTRL1_reg);
	ifadc_ctrl2.regValue = IoReg_Read32(TV_SB1_ANA_IFADC_CTRL2_reg);

	iqadc_ctrl0.regValue = IoReg_Read32(TV_SB1_ANA_IQADC_CTRL0_reg);
	iqadc_ctrl2.regValue = IoReg_Read32(TV_SB1_ANA_IQADC_CTRL2_reg);
	iqadc_ctrl3.regValue = IoReg_Read32(TV_SB1_ANA_IQADC_CTRL3_reg);
	iqadc_ctrl4.regValue = IoReg_Read32(TV_SB1_ANA_IQADC_CTRL4_reg);
	iqadc_ctrl5.regValue = IoReg_Read32(TV_SB1_ANA_IQADC_CTRL5_reg);
	iqadc_ctrl7.regValue = IoReg_Read32(TV_SB1_ANA_IQADC_CTRL7_reg);

	//IFADC+IFPGA power on
	ifadc_ctrl1.reg_pow_mbias = 1; //[16]REG_POW_MBIAS
	rtd_pr_avd_debug("SIF ADC 33100 %x\n",ifadc_ctrl1.regValue);
	IoReg_Write32(TV_SB1_ANA_IFADC_CTRL1_reg, ifadc_ctrl1.regValue);

	ifadc_ctrl2.reg_adc2x_difpow = 1; //[24]REG_ADC2X_DIFPOW
	rtd_pr_avd_debug("SIF ADC 33104 %x\n",ifadc_ctrl2.regValue);
	IoReg_Write32(TV_SB1_ANA_IFADC_CTRL2_reg, ifadc_ctrl2.regValue);

	//IQADC+IQPGA power on
	iqadc_ctrl7.reg_qadc_meta_pow = 1; //[12]REG_QADC_META_POW, [8]REG_IADC_META_POW
	iqadc_ctrl7.reg_iadc_meta_pow = 1;
	rtd_pr_avd_debug("SIF ADC 33134 %x\n",iqadc_ctrl7.regValue);
	IoReg_Write32(TV_SB1_ANA_IQADC_CTRL7_reg, iqadc_ctrl7.regValue);

	iqadc_ctrl3.reg_pow_q_pga = 1; //[12]REG_POW_Q_PGA, [8]REG_POW_I_PGA, [4]REG_POW_Q_ADC, [0]REG_POW_I_ADC
	iqadc_ctrl3.reg_pow_q_adc = 1;
	iqadc_ctrl3.reg_pow_i_pga = 1;
	iqadc_ctrl3.reg_pow_i_adc = 1;
	rtd_pr_avd_debug("SIF ADC 33124 %x\n",iqadc_ctrl3.regValue);
	IoReg_Write32(TV_SB1_ANA_IQADC_CTRL3_reg, iqadc_ctrl3.regValue);

	//Fix IFPGA 2dB
	iqadc_ctrl2.reg_if_fix_en= 1;
	rtd_pr_avd_debug("SIF ADC 33120 %x\n",iqadc_ctrl2.regValue);
	IoReg_Write32(TV_SB1_ANA_IQADC_CTRL2_reg, iqadc_ctrl2.regValue);

	iqadc_ctrl5.reg_ipga_d = 2; //[3:0]REG_IPGA_D, IPGA gain adjust 0x2: 2dB
	rtd_pr_avd_debug("SIF ADC 3312c %x\n",iqadc_ctrl5.regValue);
	IoReg_Write32(TV_SB1_ANA_IQADC_CTRL5_reg, iqadc_ctrl5.regValue);

	// SIF Audio related setting

	iqadc_ctrl0.reg_ckout_sel_q = 0; //REG_CKOUT_SEL_Q--> 0:Q_CKOUT
	rtd_pr_avd_debug("SIF ADC 33118 %x\n",iqadc_ctrl0.regValue);
	IoReg_Write32(TV_SB1_ANA_IQADC_CTRL0_reg, iqadc_ctrl0.regValue);

	iqadc_ctrl4.reg_ipga_fix_gain = 0; //[14:12]REG_IPGA_FIX_GAIN, Fix IFPGA 2dB, [0]REG_IF_FIX_EN
	iqadc_ctrl4.reg_qpga_input_sel = 0x4; //REG_QPGA_INPUT_SEL  100: SIF input
	rtd_pr_avd_debug("SIF ADC 33128 %x\n",iqadc_ctrl4.regValue);
	IoReg_Write32(TV_SB1_ANA_IQADC_CTRL4_reg, iqadc_ctrl4.regValue);

	iqadc_ctrl7.reg_qadc_vref_sel = 0; //1.23V
	iqadc_ctrl7.reg_iadc_vref_sel = 0; //1.23V
	iqadc_ctrl7.reg_qadc_ldo_sel = 0; //0.95V
	iqadc_ctrl7.reg_iadc_ldo_sel = 0; //0.95V
	rtd_pr_avd_debug("SIF ADC 33134 %x\n",iqadc_ctrl7.regValue);
	IoReg_Write32(TV_SB1_ANA_IQADC_CTRL7_reg, iqadc_ctrl7.regValue);

	iqadc_ctrl2.reg_iqcmp_isel = 0x4; //50uA(default)
	rtd_pr_avd_debug("SIF ADC 33120 %x\n",iqadc_ctrl2.regValue);
	IoReg_Write32(TV_SB1_ANA_IQADC_CTRL2_reg, iqadc_ctrl2.regValue);

	iqadc_ctrl5.reg_q_ckin_sel = 0x2; //REG_Q_CKIN_SEL 10: CK_24M From Digital
	rtd_pr_avd_debug("SIF ADC 3312c %x\n",iqadc_ctrl5.regValue);
	IoReg_Write32(TV_SB1_ANA_IQADC_CTRL5_reg, iqadc_ctrl5.regValue);
*/
}
//add for audio requirements end 2016-07-12

#ifdef CONFIG_CUSTOMER_TV010
unsigned char CTVCheckFVBias(void)
{
	unsigned short VHalfTotal;
	unsigned short ipv_act_sta = 0;
	unsigned short ipv_act_len = 0;
	unsigned short ipv_act_len_from_reg = 0;
	unsigned short ipv_act_len_pre = 0;
	//unsigned short FieldVTotal = drv_module_vdc_GetVarFieldVTotal();
	unsigned short FieldVTotal = 0, testFieldVTotal1 = 0, testFieldVTotal2 = 0, usVLineOut = 0;
	unsigned char mode = 0;
	unsigned short usNoramlPorch = 0;
	static int logCTVCheckFVBiasCount = 0, logCTVCheckFVBiasDiffCount = 0;

	StructDisplayInfo *ptAVDScalerDispInfo  = Get_AVD_ScalerDispinfo();

	logCTVCheckFVBiasCount++;
	drvif_module_vdc_GetVHalfTotalLen(&FieldVTotal);
	//mode = Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
	if (FieldVTotal > 290)
		mode = _MODE_576I;
	else
		mode = _MODE_480I;

	//if(logCTVCheckFVBiasCount%200==0) {
	//	rtd_pr_avd_notice("HYDBG 000, FieldVTotal=%d\n", FieldVTotal);
	//}

	if (mode == _MODE_480I && FieldVTotal > 259)
		return TRUE;
	else if (FieldVTotal > 309)
		return TRUE;

	ipv_act_sta = (unsigned short)((IoReg_Read32(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg)>>16)&0x3fff);
	ipv_act_len_from_reg = (unsigned short)((IoReg_Read32(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg))&0x00003fff);
	ipv_act_len = (unsigned short)(Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN));
	ipv_act_len_pre = ptAVDScalerDispInfo->IPV_ACT_LEN_PRE;
	if (g_bFVBiasRunScaler && (ipv_act_len_pre >= (ipv_act_sta + ipv_act_len_from_reg))) {
		g_bFVBiasRunScaler = FALSE;
		rtd_pr_avd_emerg("oliverrrHYTDBG clearrrrrrrrrrr  ipv_act_len_from_reg=%d, ipv_act_len_pre=%d\n", ipv_act_len_from_reg, ipv_act_len_pre);

	}

	drvif_module_vdc_GetVHalfTotalLen(&testFieldVTotal1);
	usVLineOut = drvif_module_vdc_Read_Vline_out();
	drvif_module_vdc_GetVHalfTotalLen(&testFieldVTotal2);

	//if(logCTVCheckFVBiasCount%200==0 || (g_usCurVHaldTotal !=FieldVTotal && FieldVTotal >=100 && logCTVCheckFVBiasDiffCount%20==0)){
	//	rtd_pr_avd_notice("HYTDBG g_tAVDLGETimingInfo.hTotal = %d, g_tAVDLGETimingInfo.vTotal = %d, usVLineOut=%d\n", g_tAVDLGETimingInfo.hTotal, g_tAVDLGETimingInfo.vTotal, usVLineOut);
	//	rtd_pr_avd_notice("HYTDBG Scaler_DispGetInputInfo = %d, ipv_act_sta=%d, ipv_act_len=%d, FieldVTotal=%d,g_usCurVHaldTotal=%d, Vstate=%d\n",
	//	mode, ipv_act_sta, ipv_act_len, FieldVTotal, g_usCurVHaldTotal, drvif_module_vdc_Read_Vstate());
	//	rtd_pr_avd_notice("HYTDBG testFieldVTotal1 %d, testFieldVTotal2 %d\n", testFieldVTotal1, testFieldVTotal2);
	//}

	if (ABS(testFieldVTotal1, FieldVTotal ) >1)
		return TRUE;
	if (ABS(testFieldVTotal2, FieldVTotal ) >1)
		return TRUE;
	if (ABS(usVLineOut, FieldVTotal*2 ) >2)
		return TRUE;

	//if(logCTVCheckFVBiasCount%200==0) {
	//	rtd_pr_avd_notice("HYDBG 222\n");
	//}

	if (drvif_module_vdc_Read_Vstate() <= 1)
		return TRUE;


	if (g_usCurVHaldTotal !=FieldVTotal && FieldVTotal >=100)
		logCTVCheckFVBiasDiffCount++;
	else
		logCTVCheckFVBiasDiffCount=0;

	//if((mode==_MODE_576I && FieldVTotal <= 311)  || (mode==_MODE_480I && FieldVTotal <= 261))
	//{
	//	//FieldVTotal = FieldVTotal - 3; //get smaller value
	//}

	int vtotal_diff = ABS(g_usCurVHaldTotal,FieldVTotal);
#if 0
	if ( (vtotal_diff > 2  ||
		(mode==_MODE_576I && FieldVTotal <= 311 && g_usCurVHaldTotal > FieldVTotal)  ||
		(mode==_MODE_480I && FieldVTotal <= 261 && g_usCurVHaldTotal > FieldVTotal)  )
		&& FieldVTotal >=100)
#else
	if (vtotal_diff != 0 && FieldVTotal >= 100)
#endif
	{
		int i=0;
		g_usIsVtotalNoGood = 1;
		Scaler_AVD_ClearTimingInfo();
		Scaler_AVD_ClearTiming();
		g_bMainRunSearchState = true;
		g_bSubRunSearchState = true;
		g_usIsGetTimeInfo = 0;
		g_bFVBiasRunScaler = true;
		rtd_pr_avd_emerg("oliverrrHYTDBG g_usCurVHaldTotal=%d\n", g_usCurVHaldTotal);
		msleep(25);
		for (i=0; i< 20 && g_usIsGetTimeInfo < 3; i++)
			msleep(25);

		g_usCurVHaldTotal = FieldVTotal;
		rtd_pr_avd_emerg("oliverrrHYTDBG g_tAVDLGETimingInfo.hTotal = %d, g_tAVDLGETimingInfo.vTotal = %d, i=%d\n", g_tAVDLGETimingInfo.hTotal, g_tAVDLGETimingInfo.vTotal, i);

		rtd_pr_avd_emerg("oliverrrHYTDBG Scaler_DispGetInputInfo = %d, ipv_act_sta=%d, ipv_act_len=%d, FieldVTotal=%d\n", \
		mode, ipv_act_sta, ipv_act_len, FieldVTotal);

		rtd_pr_avd_emerg("oliverrrHYTDBG ipv_act_len_from_reg=%d, ipv_act_len_pre=%d\n", ipv_act_len_from_reg, ipv_act_len_pre);

		return FALSE;
	}
	#if 0
	else if (((mode==_MODE_576I && (ipv_act_len+25) > FieldVTotal)  || (mode==_MODE_480I && (ipv_act_len+22) > FieldVTotal))
		&& g_usCurVActLen != ipv_act_len
		&& FieldVTotal >=100)
	{
		Scaler_AVD_ClearTimingInfo();
		Scaler_AVD_ClearTiming();

		//msleep(25);
		rtd_pr_avd_notice("oliverrrHYTDBG g_usCurVActLen=%d\n", g_usCurVActLen);
		g_usCurVActLen = ipv_act_len;
		rtd_pr_avd_notice("222 oliverrrHYTDBG g_tAVDLGETimingInfo.hTotal = %d, g_tAVDLGETimingInfo.vTotal = %d\n", g_tAVDLGETimingInfo.hTotal, g_tAVDLGETimingInfo.vTotal);

		rtd_pr_avd_notice("222 oliverrrHYTDBG Scaler_DispGetInputInfo = %d, ipv_act_sta=%d, ipv_act_len=%d, FieldVTotal=%d\n", \
		mode, ipv_act_sta, ipv_act_len, FieldVTotal);

		return FALSE;
	}
	#endif

	return TRUE; // default is no change
}

//For CVBS //temp here for debue huangyiteng
static UINT16 g_tAV_PRESET_TABLE_AVD[][9] =
{
	// VIDEO-60(NTSC)
	{
		858,				//IH_TOTAL
		208,				//IH_ACT_STA
		720,				//IH_ACT_WID
		15,					//IV_ACT_STA
		240,				//IV_ACT_LEN
		_DISPLAY_RATIO_95,	//H_Ratio percent
		_DISPLAY_RATIO_98,	//V_Ratio percent
		36,					//H_Ratio value
		5					//V_Ratio value
	},
	// VIDEO-50(PAL& SECAM)
	{
		864,				//IH_TOTAL
		208,				//IH_ACT_STA
		720,				//IH_ACT_WID
		15,					//IV_ACT_STA
		288,				//IV_ACT_LEN
		_DISPLAY_RATIO_92,	//H_Ratio percent
		_DISPLAY_RATIO_98,	//V_Ratio percent
		45,					//H_Ratio value
		8					//V_Ratio value
	},
};

extern void drv_ifd_set_clamping_scale(IFD_CLAMP_SCALE clamping_scale);
void AVD_TV_AV_Non_Standard_Process(void)
{
	UINT32 nstvalue;
	nstvalue = GetNonStandardFlag();
	//rtd_pr_avd_info("AVD--nstvalue=%08x\n", nstvalue);
	if(_SRC_TV == g_ucSrcType){
		if(nstvalue & ATVNONSTANDFLAG_0){
			//rtd_pr_avd_notice("ATVNONSTAND_0\n",nstvalue);
			drv_ifd_set_clamping_scale((IFD_CLAMP_SCALE) 10);
		}
	}
	else if(_SRC_CVBS == g_ucSrcType){
	}
}
#endif

unsigned char AVD_vpq_mem_ctrl(unsigned char bEnable)
{
	// bEnable = 1, allocate memory and enable VDPQ memory access
	// bEnable = 0, disable VDPQ memory access and free memory (delay 1/60 sec)

	if (FALSE == module_vdc_check_pll27x_setting()  || FALSE == module_vdc_check_vd_clock())
	{
		rtd_pr_avd_err("[ERROR] VDC Clock is off\n");
		return FALSE;
	}

	if(bEnable == _ENABLE)
	{
		AVD_Memory_Allocate();
	}
	else
	{
		// Bound end address to start address for protection
		vdpq_mem0_addr_ini_RBUS mem0_addr_ini_reg;
		vdpq_mem1_addr_ini_RBUS mem1_addr_ini_reg;
		vdpq_mem2_addr_ini_RBUS mem2_addr_ini_reg;
		vdpq_mem3_addr_ini_RBUS mem3_addr_ini_reg;
		vdpq_mem0_addr_end_RBUS mem0_addr_end_reg;
		vdpq_mem1_addr_end_RBUS mem1_addr_end_reg;
		vdpq_mem2_addr_end_RBUS mem2_addr_end_reg;
		vdpq_mem3_addr_end_RBUS mem3_addr_end_reg;

		// Disble VDPQ memory access
		drvif_module_vpq_DmaAccessCtrl(OFF);
		mem0_addr_ini_reg.regValue = vdcf_inl(VDPQ_MEM0_ADDR_INI_reg);
		mem1_addr_ini_reg.regValue = vdcf_inl(VDPQ_MEM1_ADDR_INI_reg);
		mem2_addr_ini_reg.regValue = vdcf_inl(VDPQ_MEM2_ADDR_INI_reg);
		mem3_addr_ini_reg.regValue = vdcf_inl(VDPQ_MEM3_ADDR_INI_reg);
		mem0_addr_end_reg.regValue = vdcf_inl(VDPQ_MEM0_ADDR_END_reg);
		mem1_addr_end_reg.regValue = vdcf_inl(VDPQ_MEM1_ADDR_END_reg);
		mem2_addr_end_reg.regValue = vdcf_inl(VDPQ_MEM2_ADDR_END_reg);
		mem3_addr_end_reg.regValue = vdcf_inl(VDPQ_MEM3_ADDR_END_reg);

		mem0_addr_end_reg.regValue = mem0_addr_ini_reg.regValue;
		mem1_addr_end_reg.regValue = mem1_addr_ini_reg.regValue;
		mem2_addr_end_reg.regValue = mem2_addr_ini_reg.regValue;
		mem3_addr_end_reg.regValue = mem3_addr_ini_reg.regValue;

		vdcf_outl(VDPQ_MEM0_ADDR_END_reg, mem0_addr_end_reg.regValue);
		vdcf_outl(VDPQ_MEM1_ADDR_END_reg, mem1_addr_end_reg.regValue);
		vdcf_outl(VDPQ_MEM2_ADDR_END_reg, mem2_addr_end_reg.regValue);
		vdcf_outl(VDPQ_MEM3_ADDR_END_reg, mem3_addr_end_reg.regValue);

		AVD_Memory_Free();
	}

	if(bEnable == _ENABLE && g_ulVDCPhyAddr != 0)
		return TRUE;
	else if(bEnable == _DISABLE && g_ulVDCPhyAddr == 0)
		return TRUE;
	else
		return FALSE;

}

unsigned int AVD_vpq_mem_isAllocated(void)
{
	return g_ulVDCPhyAddr;
}

unsigned char get_avd_print_flag(void)
{
	//enable by 0xb80196b4
	return 0;//g_vdc_msg_level;
}

void Scaler_AVD_Update_Wid_Len(unsigned int len,unsigned int wid)
{
	KADP_VFE_AVD_TIMING_INFO_T *ptLGETimingInfo;
	ptLGETimingInfo = Get_AVD_LGETiminginfo();
	ptLGETimingInfo->active.h = len;
	ptLGETimingInfo->active.w = wid;
	rtd_pr_avd_info("AVD_UpdateLGE_len_wid=%d,%d\n",ptLGETimingInfo->active.h,ptLGETimingInfo->active.w);
}
KADP_VFE_AVD_CC_TTX_STATUS_T Scaler_VBI_GetLGEColorSystem(void)
{
	if (g_tSupportedColorSys == KADP_COLOR_SYSTEM_NTSC_M) {
		return KADP_VFE_AVD_CC_IS_SUPPORTED;
	} else if ((g_tSupportedColorSys & KADP_COLOR_SYSTEM_NTSC_M) &&
		(g_tSupportedColorSys & KADP_COLOR_SYSTEM_PAL_G)&&
		(g_tSupportedColorSys & KADP_COLOR_SYSTEM_SECAM)) {
		return KADP_VFE_AVD_TTX_IS_SUPPORTED;
	} else {
		return KADP_VFE_AVD_NOTHING_IS_SUPPORTED;
	}
}
