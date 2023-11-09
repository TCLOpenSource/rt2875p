/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2016
 *
 * All rights reserved.
 *
 *============================================================*/

/*======================= Description ============================
 *
 * file: 		hdmi_driver.c
 *
 * author: 	Spider Man, Peter Parker
 * date:
 * version: 	3.0
 *
 *============================================================*/

/*========================Header Files============================*/
#include "hdmi_common.h"
#include "hdmi_edid.h"
#include "hdmi_hdr.h"
#include "hdmi_scdc.h"
#include "hdmi_reg.h"
#include "hdmi_perf_log.h"
#include "hdmi_vfe_config.h"
#include "hdmi_debug.h"
#include <tvscalercontrol/hdmirx/hdmi_vfe.h>
//#include <tvscalercontrol/scalerdrv/scaler_hdr_ctrl.h>
#include <rbus/h3ddma_reg.h>
#include <rbus/onms_reg.h>
#include "hdmi_edid.h"
#include "hdmi_table.h"
#include <hdcp/hdcp2_optee.h>
#include <hdcp/hdcp14_optee.h>
#ifndef BUILD_QUICK_SHOW
#include <generated/autoconf.h>
#endif
#include <rtk_kdriver/quick_show/quick_show.h>

extern unsigned char get_hdmi_vrr_framerate(unsigned short *framerate);

/**********************************************************************************************
*
*	Marco or Definitions
*
**********************************************************************************************/
#define HDCP_AKSV_SIZE_BYTES 	5
#define TOTAL_3D 		16

#define TIMER_CLOCK 	27000000
#define CONFIG_SYS_HZ 	1000
#define CONFIG_STR_HPD_LOW_SPEEDUP_FROM_BOOTCODE    1

/**********************************************************************************************
*
*	Const Declarations
*
**********************************************************************************************/




typedef struct {
	unsigned char port;
	unsigned char pre_state;
              unsigned char cur_state;
} hdmi_drv_connect_state_st;

#ifdef CONFIG_PM

typedef struct {
	unsigned char channel;
	int pre_b_value;
} hdmi_drv_resume_st;

#endif /*CONFIG_PM*/


/* 3d format mapping */
unsigned char mapping3dformat[TOTAL_3D] = {
	VFE_HDMI_3D_FORMAT_FRAME_PACKING,
	VFE_HDMI_3D_FORMAT_FIELD_ALTERNATIVE,
	VFE_HDMI_3D_FORMAT_LINE_ALTERNATIVE,
	VFE_HDMI_3D_FORMAT_SIDEBYSIDE_FULL,
	VFE_HDMI_3D_FORMAT_2D, // rtk:HDMI3D_L_DEPTH //to check 3
	VFE_HDMI_3D_FORMAT_2D, // rtk:HDMI3D_L_DEPTH_GPX //to check 3
	VFE_HDMI_3D_FORMAT_TOP_BOTTOM,
	VFE_HDMI_3D_FORMAT_FRAME_SEQ,
	VFE_HDMI_3D_FORMAT_SIDEBYSIDE_HALF,
	VFE_HDMI_3D_FORMAT_LINE_INTERLEAVE_HALF, // rtk:HDMI3D_LINE_BY_LINE // to check 3
	VFE_HDMI_3D_FORMAT_COLUMN_INTERLEAVE, // rtk:HDMI3D_VERTICAL_STRIPE // to check 3
	VFE_HDMI_3D_FORMAT_CHECKER_BOARD,
	VFE_HDMI_3D_FORMAT_2D, // rtk:HDMI3D_REALID // to check 3
	VFE_HDMI_3D_FORMAT_2D, // rtk:HDMI3D_SENSIO // to check 3
	VFE_HDMI_3D_FORMAT_2D,
	VFE_HDMI_3D_FORMAT_2D
};



/**********************************************************************************************
*
*	Variables
*
**********************************************************************************************/
extern unsigned int current_hdmi_onms_watchdog_setting;
extern unsigned char current_hdmi_onms_is_i3ddma;
unsigned int g_hdmi_drv_state = VFE_HDMI_DRV_SUSPEND_STATE;
#ifdef HDMI_PORT_MUX
unsigned char g_hdmi_channel_connect_flag[HDMI_RX_PORT_MAX_NUM] = {0, 0, 0, 0};
unsigned char g_hdmi_hpd_flag[HDMI_RX_PORT_MAX_NUM] = {0, 0, 0, 0};

#else
unsigned char g_hdmi_channel_connect_flag[HDMI_RX_PORT_MAX_NUM] = {0, 0, 0};
unsigned char g_hdmi_hpd_flag[HDMI_RX_PORT_MAX_NUM] = {0, 0, 0};

#endif


vfe_hdmi_override_eotf_e g_hdmi_drm_override_eotf[HDMI_RX_PORT_MAX_NUM];



#ifdef CONFIG_PM
hdmi_drv_resume_st g_resume_param;
#endif /*CONFIG_PM*/

static unsigned char resume_hpd_control = 0;

int flag_dolby_vsif_change=0;

//#define AVI_INFO_COMPARE	1

char* g_avi_csc_string_table[4] = {"CSC_RGB", "CSC_YCBCR422", "CSC_YCBCR444","CSC_FUTURE"};
char* g_avi_bi_string_table[4] = {"INVALID", "V", "H","VERTHORIZVALID"};
char* g_avi_scan_info_string_table[4] = {"NODATA", "OVERSCANNED", "UNDERSCANNED","FUTURE"};
char* g_avi_colorimetry_string_table[4] = {"NODATA", "SMPTE170", "ITU709","COLORIMETRY_FUTURE"};
char* g_avi_arc_string_table[4] = {"NODATA", "4_3", "16_9","FUTURE"};
char* g_avi_active_arc_string_table[5] = {"ARC_PICTURE", "4_3CENTER", "16_9CENTER","14_9CENTER", "OTHER"};
char* g_avi_scaling_string_table[4] = {"NOSCALING", "HS", "VS","HVS"};
char* g_avi_itcontent_string_table[3] = {"NODATA", "ITCONTENT"};
char* g_avi_extcolorimetry_string_table[8] = {"XVYCC601", "XVYCC709","sYCC601","AdobeYCC601","AdobeRGB","ITURBT2020YCCBCCRC,","ITURBT2020RGBorYCBCR", "RESERED"};
char* g_avi_rgb_quantization_string_table[4] = {"DEFAULT", "LIMIT", "FULL","RESERVED"};
char* g_avi_ycc_quantization_string_table[3] = {"LIMIT", "FULL", "RESERVED"};
char* g_avi_content_type_string_table[5] = {"GRAPHICS ", "PHOTO", "CINEMA","GAME","MAX"};

UINT8 HDCP_key_loaded[VFE_HDCP_MAX] = {0};

#define HDCP_2P2_RECEIVER_ID_SIZE	5
UINT8 HDCP_2p2_ReceiverdId[HDCP_2P2_RECEIVER_ID_SIZE] = {0};

/**********************************************************************************************
*
*	Funtion Declarations
*
**********************************************************************************************/

#ifdef AVI_INFO_COMPARE
bool Cmp_AviInfoFrame(vfe_hdmi_avi_t *info_frameA,vfe_hdmi_avi_t *info_frameB);
#endif //AVI_INFO_COMPARE


/**********************************************************************************************
*
*	Function Body
*
**********************************************************************************************/


/*-------------------------------------------
 * Func : HdmiTable_Init
 *
 * Desc : init EDID & HDCP key resource
 *
 * Para : N/A
 *
 * Retn : N/A
 *-------------------------------------------*/
static void HdmiTable_Init(void)
{
	unsigned int ch;
	HDMI_CHANNEL_T_PCB_INFO port_config;
	
	board_hdmi_drv_port_mapping_Initial();  // load channel -> port mapping

	newbase_hdmi_init_pcbinfo(); //init each port for NON-USED
	
	// load configuration for each hdmi channel
	for (ch=0; ch < HDMI_RX_PORT_MAX_NUM; ch++)
	{
		unsigned char hdmi_port;
		
		if (hdmi_vfe_get_hdmi_port(ch, &hdmi_port)<0) 
			continue;  // no HDMI port associated
		
		HDMI_INFO("HDMI_CHANNEL[%d] ==> HDMI_PORT[%d]\n", ch, hdmi_port);
		
		// load configuration of specified hdmi port
		memset(&port_config, 0, sizeof(port_config));
		port_config.phy_port = hdmi_port;
		port_config.ddc_selected = hdmi_port;
		port_config.edid_type = HDMI_EDID_ONCHIP;
		
		if (hdmi_vfe_get_rx_port_info(hdmi_port, &port_config)<0) {
			HDMI_EMG("HdmiTable_Init failed, get config of HDMI%d (HDMI_PORT_%d) failed\n", ch, hdmi_port);
			BUG();
		}
		
		// set config of the given hdmi port
		newbase_hdmi_save_pcbinfo(hdmi_port, &port_config);
#ifndef BUILD_QUICK_SHOW
		// load configuration of specified hdmi port
		if (!newbase_hdmi_is_edid_initialized(hdmi_port))
		{
			HDMI_EDID_INFO("EDID init, Use FW default EDID table, hdmi_port=%d\n", hdmi_port);
			newbase_hdmi_save_edid(hdmi_port, p_default_hdmi_edid_table[hdmi_port], 256);
		}
#endif
		g_hdmi_drm_override_eotf[hdmi_port] = VFE_HDMI_OVERRIDE_EOTF_AUTO;
	}
#ifndef BUILD_QUICK_SHOW
#if USE_DEFAULT_HDCP14
	newbase_hdmi_hdcp14_write_hdcp_key((unsigned char *)newbase_hdmi_get_default_hdcp14_bksv(), (unsigned char *)newbase_hdmi_get_default_hdcp14_key(), 0);  // just save it, do not apply
#endif
#endif

	return;
}

int hdmi_vfe_get_connected_channel(unsigned char *p_ch)
{
	unsigned char ch;

	for (ch = 0; ch < HDMI_RX_PORT_MAX_NUM; ch++)
	{
		if (g_hdmi_channel_connect_flag[ch]) 
		{
			*p_ch = ch;
			return 0;
		}
	}
	
	return -1;
}

int hdmi_vfe_get_connected_port(unsigned char *p_port)
{
	unsigned char ch;
	
	if (hdmi_vfe_get_connected_channel(&ch)<0)
		return -1;
		
	return hdmi_vfe_get_hdmi_port(ch, p_port);
}

int hdmi_vfe_is_valid_channel(unsigned char ch)
{
	unsigned char port;	
	return hdmi_vfe_get_hdmi_port(ch, &port);
}

static unsigned int resume_hpd_low_start[HDMI_PORT_TOTAL_NUM] = {0};
static unsigned int resume_hpd_low_end[HDMI_PORT_TOTAL_NUM] = {0};

void vfe_hdmi_hdcp2_loadkey(void)
{
 //add access rmpb here
#ifdef CONFIG_OPTEE_HDCP2
	unsigned char uc_lc128[16]={0};
	optee_hdcp2_main(HDMI_HDCP2_MODE, HDCP2_CMD_LOAD_KEY, 0, uc_lc128, 16,uc_lc128,16);
#endif
}

void vfe_hdmi_hdcp14_getkey(void)
{
#ifdef CONFIG_OPTEE_HDCP14
	unsigned char  bksv[5] = {0};
	unsigned char  hdcp_key[320] = {0};
	int ret = 0;
	int i;

	ret = optee_hdcp14_get_key(bksv, 5, hdcp_key, 320);
	if(ret == 0){
		for (i = 0; i < 5; i++) {
			if (bksv[i] != 0) {
				newbase_hdmi_hdcp14_write_hdcp_key(bksv, hdcp_key, 0);
				HDMI_WARN("save hdcp14 optee key\n");
				return;
			}
		}
		HDMI_WARN("get key failed, not burn hdcp14 optee key\n");
	}else{
		HDMI_WARN("get hdcp14 optee key failed\n");
	}
#endif
}

void vfe_hdmi_hdcp14_loadkey(unsigned char          mode)
{
#ifdef CONFIG_OPTEE_HDCP14
	optee_hdcp14_load_key(HDMI_HDCP_MODE, mode);  //load and write key to 
	vfe_hdmi_hdcp14_getkey();
#endif
}

/**********************************************************************************************
*
*	Function Body
*
**********************************************************************************************/
/*-------------------------------------------
 * Func : vfe_hdmi_qs_drv_init
 *
 * Desc : init HDMI video frontend when quick show down.
 *
 * Para : N/A
 *
 * Retn : 0 : success, others : failed
 *-------------------------------------------*/
int vfe_hdmi_qs_drv_init(void)
{
	unsigned char ch = 0;
	int qs_portnum = 0;
	unsigned char port = 0;
	HDMI_INFO("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SUSPEND_STATE)
	{
		if (g_hdmi_drv_state==VFE_HDMI_DRV_SERVICE_STATE)
		{
			HDMI_EMG("[- STATE ] vfe_hdmi_drv_qs_init successed, it has been initialized already fsm = %d\n", g_hdmi_drv_state);
			return HDMI_DRV_NO_ERR;
		}
		return HDMI_DRV_STATE_ERR;
	}

	g_hdmi_drv_state = VFE_HDMI_DRV_INIT_STATE;
	qs_portnum = get_QS_portnum();
	if ( qs_portnum < HDMI_PORT0 || qs_portnum >= HDMI_PORT_TOTAL_NUM)
		return HDMI_DRV_STATE_ERR;

	port = (unsigned char)qs_portnum;
	// sync qs scdc status
	newbase_hdmi_sync_qs_scdc_status(port);

	//sync qs phy status
	newbase_set_qs_rxphy_status(port);


	newbase_hdmi_set_edid_initialized(port, 1);// do not write edid in qs mode
	HdmiTable_Init();

	newbase_hdmi_qs_init(port);

	//vfe_hdmi_hdcp2_loadkey();

	g_hdmi_drv_state = VFE_HDMI_DRV_SERVICE_STATE;
	memset(g_hdmi_channel_connect_flag, 0, sizeof(g_hdmi_channel_connect_flag));  // reset data structure
	g_hdmi_channel_connect_flag[port] = 1;

	for(ch = 0; ch < HDMI_RX_PORT_MAX_NUM; ch++)
	{
		if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
			continue;  // no HDMI port associated

		newbase_hdmi_qs_init_source(port);
	}

#ifdef CONFIG_HDMI_DRV_HPD
	newbase_hdmi_enable_hpd_with_delay(10,EVERY_HDMI_PORT);  // enable HPD automatically
#endif

	// connect setting
	qs_portnum = get_QS_portnum();
	if ( qs_portnum < HDMI_PORT0 || qs_portnum >= HDMI_PORT_TOTAL_NUM)
		return HDMI_DRV_STATE_ERR;

	port = (unsigned char)qs_portnum;
	newbase_hdmi_power_saving_wakeup(port, PS_WAKEUP_ACTIVE_SOURCE | PS_WAKEUP_IMMEDIATELY);  // wakeup hdmi port
	newbase_hdmi_hdr10_enable(_ENABLE);     //Minchay@20160607-a add hdr10

	if(newbase_hdmi_get_thermal_mode())
	{
		newbase_hdmi_set_hpd(port, 1);//set hpd high let phy pw open in thermal mode
	}

	//newbase_hdmi_qs_init_source(port);
	return HDMI_DRV_NO_ERR;
}

/*-------------------------------------------
 * Func : vfe_hdmi_get_qs_current_port
 *
 * Desc : get hdmi current display port in quick show.
 *
 * Para : N/A
 *
 * Retn : 0/1/2/3 : success, others : failed
 *-------------------------------------------*/

int vfe_hdmi_get_qs_current_port(void)
{
	HDMI_INFO("[%s %d] starts\n", __func__, __LINE__);
	if(is_QS_hdmi_enable())
		return get_QS_portnum();
	else
		return -1;
}

void vfe_hdmi_sync_qs_status(void)
{
	unsigned char is_get_avi = 0;
	int port = 0;
	HDMI_AVI_T avi_info;
	unsigned char edid[256];
	port = get_QS_portnum();

	if (port < HDMI_PORT0 || port >= HDMI_PORT_TOTAL_NUM){
		HDMI_EMG("%s fail : qs port number is invalid\n", __func__);
		return;
	}
	memset(&avi_info, 0, sizeof(HDMI_AVI_T));
	// need to wait infoframe thread run in qs init
	is_get_avi = newbase_hdmi_get_avi_infoframe(port, &avi_info);
	if(is_get_avi){
		newbase_hdmi_set_current_vic(port, avi_info.VIC) ;
	}
	newbase_hdmi_read_edid(port, edid, 256);
	newbase_hdmi_save_edid(port, edid, 256);
}

/*-------------------------------------------
 * Func : vfe_hdmi_drv_init
 *
 * Desc : init HDMI video frontend. 
 *
 * Para : N/A
 *
 * Retn : 0 : success, others : failed
 *-------------------------------------------*/
int vfe_hdmi_drv_init(void)
{
	unsigned char ch = 0;
	unsigned char port = 0;
	HDMI_INFO("[%s %d] starts\n", __func__, __LINE__);
	hdmi_time_mesaure_start();

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SUSPEND_STATE)
	{
		if (g_hdmi_drv_state==VFE_HDMI_DRV_SERVICE_STATE)
		{
			HDMI_EMG("[- STATE ] vfe_hdmi_drv_init successed, it has been initialized already fsm = %d\n", g_hdmi_drv_state);
			return HDMI_DRV_STATE_ERR;
		}
		//HDMI_EMG("[- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}
	
	g_hdmi_drv_state = VFE_HDMI_DRV_INIT_STATE;

	HdmiTable_Init();

	newbase_hdmi_init();

	vfe_hdmi_hdcp2_loadkey();
	vfe_hdmi_hdcp14_loadkey(APPLY_KEY_IMMEDATELY);

	g_hdmi_drv_state = VFE_HDMI_DRV_SERVICE_STATE;
	memset(g_hdmi_channel_connect_flag, 0, sizeof(g_hdmi_channel_connect_flag));  // reset data structure

	for(ch = 0; ch < HDMI_RX_PORT_MAX_NUM; ch++)
	{		
		if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
			continue;  // no HDMI port associated
			
		newbase_hdmi_init_source(port);
#ifdef BUILD_QUICK_SHOW
		newbase_hdmi_set_hpd(port, 1);
#else

		// Even though most of devices read EDID after HPD status change, However,
		// some devices, such as Digital Zone G6, ties to read EDID under some special 
		// condition (for example, Z0 or HDCP status ready), if the device unable to get
		// correct EDID in time, it may run into abnormal condition and stop video output.
		// more over, the device won't read EDID again even the HPD goes high. To prevent 
		// from this problem, we enable EDID access after the default EDID is applied. No
		// matter the HPD will be set or not at the initial stage.	
		newbase_hdmi_edid_enable(port, 1);
#endif
	}

#ifndef BUILD_QUICK_SHOW
	if(GET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_V4L2_ENABLE) != HDMI_PCB_FORCE_ENABLE)
	{
		resume_hpd_control = 0;
		newbase_hdmi_enable_hpd(EVERY_HDMI_PORT);
	}
#endif

	return HDMI_DRV_NO_ERR;
}

int vfe_hdmi_drv_uninit(void)
{
	unsigned char ch;
	HDMI_INFO("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[vfe_hdmi_drv_uninit][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	for (ch = 0; ch < HDMI_RX_PORT_MAX_NUM; ch++)
	{
		unsigned char hdmi_port;
		if (hdmi_vfe_get_hdmi_port(ch, &hdmi_port)==0)
		{
			newbase_hdmi_release_source(hdmi_port);
			newbase_hdmi_set_edid_initialized(hdmi_port, FALSE);    //uninit edid
		}
	}
	
	newbase_hdmi_uninit();

	g_hdmi_drv_state = VFE_HDMI_DRV_SUSPEND_STATE;

	return HDMI_DRV_NO_ERR;
}

int vfe_hdmi_drv_open(void)
{
	HDMI_INFO("[%s %d] starts\n", __func__, __LINE__);

	hdmi_time_mesaure_start();

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[vfe_hdmi_drv_open][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}
	
	hdmi_time_mesaure_stop();

	return HDMI_DRV_NO_ERR;
}

int vfe_hdmi_drv_close(void)
{
	HDMI_INFO("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[vfe_hdmi_drv_close][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	return HDMI_DRV_NO_ERR;
}

int vfe_hdmi_drv_connect(unsigned char ch)
{
	unsigned char hdmi_port;
	
	HDMI_INFO("[%s %d] starts, ch=%d\n", __func__, __LINE__, ch);

	hdmi_time_mesaure_start();

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[vfe_hdmi_drv_connect][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}
	
	if (hdmi_vfe_get_hdmi_port(ch, &hdmi_port)<0) {
		HDMI_EMG("[vfe_hdmi_drv_connect][- ARG ERR] get hdmi port for ch = %d failed\n", ch);
		return HDMI_DRV_ARG_ERR;
	}

	g_hdmi_channel_connect_flag[ch] = 1;    // set hdmi channel connected
	newbase_hdmi_power_saving_wakeup(hdmi_port, PS_WAKEUP_ACTIVE_SOURCE | PS_WAKEUP_IMMEDIATELY);  // wakeup hdmi port
	newbase_hdmi_hdr10_enable(_ENABLE);     //Minchay@20160607-a add hdr10
	if(g_hdmi_bypass_connect_disconnect)
	{
		//do not close video output
		newbase_hdmi_set_current_display_port(hdmi_port);
		lib_hdmi_mac_select_port(hdmi_port);
	}
	else
	{
		newbase_hdmi_port_select(hdmi_port, newbase_rxphy_get_frl_mode(hdmi_port));
	}

	if(newbase_hdmi_get_thermal_mode())
	{
		newbase_hdmi_set_hpd(hdmi_port, 1);//set hpd high let phy pw open in thermal mode
	}

	newbase_hdmi_set_current_display_port(hdmi_port);
	hdmi_time_mesaure_stop();

	if((GET_FLOW_CFG(HDMI_FLOW_CFG_HPD, HDMI_FLOW_CFG1_DISABLE_PORT_FAST_SWITCH) == HDMI_PCB_FORCE_ENABLE))
	{// Disable Fast switch, driver should set HPD when source switched.
		newbase_hdmi_set_hpd(hdmi_port, 0);

		msleep(10*GET_FLOW_CFG(HDMI_FLOW_CFG_HPD, HDMI_FLOW_CFG1_DEF_HPD_LOW_10MS));

		newbase_hdmi_set_hpd(hdmi_port, 1);
		newbase_hdmi_connect_reset_all(hdmi_port, 1);
	}

	return HDMI_DRV_NO_ERR;
}
EXPORT_SYMBOL(vfe_hdmi_drv_connect);

int vfe_hdmi_drv_disconnect(unsigned char ch)
{
	unsigned char hdmi_port;
	HDMI_INFO("[%s %d] starts, ch=%d\n", __func__, __LINE__, ch);
	if(g_hdmi_bypass_connect_disconnect)
	{
		return HDMI_DRV_NO_ERR;
	}
	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[vfe_hdmi_drv_disconnect][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}
	
	if (hdmi_vfe_get_hdmi_port(ch, &hdmi_port)<0) {
		HDMI_EMG("[vfe_hdmi_drv_disconnect][- ARG ERR] get hdmi port for ch = %d failed\n", ch);
		return HDMI_DRV_ARG_ERR;
	}
	g_hdmi_channel_connect_flag[ch] = 0; //Before executed reset all, global value should be updated in first line. Avoid racing issue.

	newbase_hdmi_disconnect_source(hdmi_port, 0); // do not full reset, just keep it alive in the background
	newbase_hdmi_hdr_disable();//Minchay@20160607-a-s add hdr10 and dolby version hdr

	if((GET_FLOW_CFG(HDMI_FLOW_CFG_HPD, HDMI_FLOW_CFG1_DISABLE_PORT_FAST_SWITCH) == HDMI_PCB_FORCE_ENABLE))
	{// Disable Fast switch, driver should set HPD when source switched.
		newbase_hdmi_connect_reset_all(hdmi_port, 0);
	}
	return HDMI_DRV_NO_ERR;
}
EXPORT_SYMBOL(vfe_hdmi_drv_disconnect);

int vfe_hdmi_drv_is_current_channel(unsigned char ch)
{
    unsigned char curr_ch = 0;
    return (hdmi_vfe_get_connected_channel(&curr_ch)==0 && curr_ch==ch) ? 1 : 0;
}

unsigned char vfe_hdmi_drv_detect_mode(void)
{
	unsigned char ch;
	//rtd_pr_hdmi_notice("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		//HDMI_EMG("[- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}
	
	_lock_timing_detect_sem();

	for (ch = 0; ch < HDMI_RX_PORT_MAX_NUM; ch++)
	{
		if (g_hdmi_channel_connect_flag[ch])
		{
			unsigned char hdmi_port;
			
			if (hdmi_vfe_get_hdmi_port(ch, &hdmi_port)==0)
			{
				newbase_hdmi_detect_mode(hdmi_port);
				break;
			}
		}
	}

	_unlock_timing_detect_sem();	

	return HDMI_DRV_NO_ERR;
}

int vfe_hdmi_drv_get_port_timing_info(vfe_hdmi_timing_info_t *timing_info)
{    
	unsigned char ch = 0x0F;
	unsigned char port = 0x0F;
	MEASURE_TIMING_T tm;
	unsigned char allm_mode = 0;
	uint8_t dolby_l11_content_type[4] = { 0 };

	//HDMI_PRINTF("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[vfe_hdmi_drv_get_port_timing_info][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if (timing_info == NULL) {
		HDMI_EMG("[- ARG ERR] timing_info is NULL \n");
		return HDMI_DRV_ARG_ERR;
	}

	ch = timing_info->port;

       if(HAL_VFE_HDMI_CURRENT_PORT== ch)
       {//Detect non-specified port, force use current port.
           INFOFRAME_FLOW_PRINTF("[vfe_hdmi_drv_get_port_timing_info] Non specified port, use current port!! ch = %d\n", ch);
           if (hdmi_vfe_get_connected_port(&port)<0)
           {
		HDMI_EMG("[WARNING1] vfe_hdmi_drv_get_port_timing_info can't get current port\n");
		return HDMI_DRV_ARG_ERR;
	    }
       }
	else if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
	{
		VFE_FLOW_PRINTF("get timig info failed, get connected port failed ch:%d, port: %d\n", ch, port);
		return HDMI_DRV_ARG_ERR;
	}

	memset(timing_info, 0, sizeof(vfe_hdmi_timing_info_t));

	//port info had modify at VFE_IOC_HDMI_GET_PORT_TIMINGINFO
	if (!newbase_hdmi_get_timing(port, &tm))
	{
		VFE_FLOW_PRINTF("get timig info failed, invalid port -%d\n", port);
		return HDMI_DRV_NO_ERR;
	}

	timing_info->h_freq    = tm.h_freq/100; //0.1khz
	timing_info->v_freq    = lib_measure_vfreq_prescion_3_to_1(tm.v_freq); //0.1hz
	timing_info->h_total   = tm.h_total;
	timing_info->v_total   = tm.v_total;
	timing_info->h_porch   = tm.h_act_sta;
	timing_info->v_porch   = tm.v_act_sta;
	timing_info->active.x = 0;
	timing_info->active.y = 0;
	timing_info->active.w  = tm.h_act_len;
	timing_info->active.h  = tm.v_act_len;
	timing_info->scan_type = (!tm.is_interlace);
#ifdef CONFIG_ENABLE_DOLBY_VISION_HDMI_AUTO_DETECT

    #ifdef CONFIG_SUPPORT_DOLBY_VSIF
        if (flag_dolby_vsif_change)
        {
            timing_info->isDolbyHDR = 0;
            flag_dolby_vsif_change=0;
        }
        else
        {
           if(get_HDMI_Dolby_VSIF_mode() == DOLBY_HDMI_VSIF_LL)//10 or 11
               timing_info->isDolbyHDR = 1;
           else if (get_HDMI_Dolby_VSIF_mode() == DOLBY_HDMI_VSIF_STD || get_HDMI_Dolby_VSIF_mode() == DOLBY_HDMI_h14B_VSIF)
		   timing_info->isDolbyHDR = 1;//
	   else
		   timing_info->isDolbyHDR = 0;
        }
    #else
        timing_info->isDolbyHDR = Scaler_HDR_DolbyVision_Hdmi_Get_Detect_Result();//rtd_inl(HDMI_MDD_SR_reg) & _BIT0;
    #endif
#else
	timing_info->isDolbyHDR = 0;
#endif
	if (newbase_hdmi_get_vrr_enable(port)) {
		timing_info->vsyncMode = VFE_HDMI_VSYNC_MODE_GAMINGVRR;
	}
	else if (newbase_hdmi_get_freesync_enable(port))
	{
		timing_info->vsyncMode = VFE_HDMI_VSYNC_MODE_FREESYNC;
	}
	else if (newbase_hdmi_get_qms_enable(port))
	{
		timing_info->vsyncMode = VFE_HDMI_VSYNC_MODE_QMSVRR;
	}
	else
	{
		timing_info->vsyncMode = VFE_HDMI_VSYNC_MODE_NON_VRR;
	}

	if (tm.h_act_len > 0 &&  tm.v_act_len > 0)
		timing_info->vidoe_fmt = (vfe_hdmi_vsi_video_format_e)tm.hvf;
	else
		timing_info->vidoe_fmt = 0;

	timing_info->struct_3d = (vfe_hdmi_vsi_3d_structure_e) tm.h3dformat;
	/* for some timing is pixel repeat */
	timing_info->pixel_repeat = tm.pixel_repeat;

	timing_info->mode_table_index = tm.modetable_index;
	timing_info->mode_id = tm.mode_id;
//=============================================
// Extra Timing Info
//=============================================
	timing_info->hsync = tm.h_total;
	timing_info->vsync = tm.v_total;

	timing_info->mode_id = tm.mode_id;
	timing_info->mode_table_index = tm.modetable_index;

	timing_info->polarity = tm.polarity;

	timing_info->color_space  = newbase_hdmi_get_colorspace_reg(port);
	timing_info->color_depth  = (enum vfe_hdmi_color_depth_e)tm.colordepth;
	timing_info->color_imetry = (enum vfe_hdmi_color_imetry_e)tm.colorimetry;
	timing_info->IVSyncPulseCount = tm.IVSyncPulseCount;
	if(TRUE == newbase_hdmi_get_fvs_allm(port, &allm_mode))
	{
	    timing_info->isALLM = allm_mode;
	}
	else
	{
	    timing_info->isALLM = 0;
	}
	timing_info->dvi_hdmi = ((newbase_hdmi_get_hdmi_mode(port)==0)? 0 : 1);

	//USER:LewisLee DATE:2022/02/08
	//DolbyVision Game mode, it can get from DOLBYVISIONEDR_DRV_GET_CONTENT_TYPE
	if((1 == timing_info->isDolbyHDR) && (0 == timing_info->isALLM))
	{
            if(true == drvif_Hdmi_GetDolbyL11Content(dolby_l11_content_type))
            {
                if(2 == (dolby_l11_content_type[0] & 0xf))
                {
                    timing_info->isALLM = 1;
                }
            }
	}

//=============================================
// Extra Timing Info
//=============================================
#if 0
	rtd_pr_hdmi_emerg("[- HDMI TIMING]\n");
	rtd_pr_hdmi_emerg("==========================================\n");
	rtd_pr_hdmi_emerg("timing_info->h_freq	 = %d\n", timing_info->h_freq);
	rtd_pr_hdmi_emerg("timing_info->v_freq	 = %d\n", timing_info->v_freq);
	rtd_pr_hdmi_emerg("timing_info->h_total	 = %d\n", timing_info->h_total);
	rtd_pr_hdmi_emerg("timing_info->v_total	 = %d\n", timing_info->v_total);
	rtd_pr_hdmi_emerg("timing_info->active.x  = %d\n", timing_info->active.x);
	rtd_pr_hdmi_emerg("timing_info->active.y  = %d\n", timing_info->active.y);
	rtd_pr_hdmi_emerg("timing_info->active.w  = %d\n", timing_info->active.w);
	rtd_pr_hdmi_emerg("timing_info->active.h  = %d\n", timing_info->active.h);
	rtd_pr_hdmi_emerg("timing_info->scan_type = %d\n", timing_info->scan_type);
	rtd_pr_hdmi_emerg("timing_info->x_offset 		= %d\n", timing_info->x_offset);
	rtd_pr_hdmi_emerg("timing_info->y_offset 		= %d\n", timing_info->y_offset);
	rtd_pr_hdmi_emerg("timing_info->mode_id			= %d\n", timing_info->mode_id);
	rtd_pr_hdmi_emerg("timing_info->mode_table_index = %d\n", timing_info->mode_table_index);
	rtd_pr_hdmi_emerg("timing_info->polarity 		= %d\n", timing_info->polarity);
	rtd_pr_hdmi_emerg("timing_info->color_space		= %d\n", timing_info->color_space);
	rtd_pr_hdmi_emerg("timing_info->color_depth		= %d\n", timing_info->color_depth);
	rtd_pr_hdmi_emerg("timing_info->color_imetry 	= %d\n", timing_info->color_imetry);
	rtd_pr_hdmi_emerg("timing_info->full_3d_timing	= %d\n",
		timing_info->full_3d_timing);
	rtd_pr_hdmi_emerg("timing_info->vidoe_fmt(eVideoFormat) = %d\n",
		timing_info->vidoe_fmt);
	rtd_pr_hdmi_emerg("timing_info->struct_3d(e3DStructure) = %d\n",
		timing_info->struct_3d);
	rtd_pr_hdmi_emerg("==========================================\n");
#endif
#if 0//for debug timing data change
	if (Cmp_TiminginfoFrame(&tim_info_T,timing_info)  == FALSE)
	{
		HDMI_PRINTF("[ERROR] Cmp_TimingInfoFrame False\n");
	}
	memcpy(&tim_info_T,timing_info,sizeof(vfe_hdmi_timing_info_t));
#endif

	return HDMI_DRV_NO_ERR;
}

//USER : Lewislee DATE : 20170518
//for scaler refine
unsigned char vfe_hdmi_drv_timing_ready(void)
{
	unsigned char ch;	
	extern unsigned char hdmi_good_timing_ready;

	//rtd_pr_hdmi_notice("[%s %d] starts\n", __func__, __LINE__);
#ifdef SPEEDUP_NEW_SCALER_FLOW
	if ((g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) || (hdmi_good_timing_ready == FALSE))
	{
		return _FALSE;
	}
#else
	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE)
	{
		return _FALSE;
	}
#endif

	for (ch = 0; ch < HDMI_RX_PORT_MAX_NUM; ch++)
	{
		if (g_hdmi_channel_connect_flag[ch])
		{		    
			unsigned char port;
		
			if (hdmi_vfe_get_hdmi_port(ch, &port)!=0)
				continue;
		 
			if (MAIN_FSM_HDMI_VIDEO_READY == newbase_hdmi_get_video_state(port)) {
				//HDMI_PRINTF( "HDMI[%d] timing ready\n", port);
				if(_FALSE == newbase_rxphy_is_clk_stable(port))
					return _FALSE;

				return _TRUE;
			}
		}
	}

	return _FALSE;
}


#if 0//for debug avi data change
bool Cmp_TiminginfoFrame(vfe_hdmi_timing_info_t *info_frameA,vfe_hdmi_timing_info_t *info_frameB)
{
	bool result =TRUE;
	if (info_frameA->h_freq != info_frameB->h_freq)
	{
		HDMI_PRINTF("C(1)\n");
		result= FALSE;
	}

	if (info_frameA->v_freq != info_frameB->v_freq)
	{
		HDMI_PRINTF("C(3)\n");
		result= FALSE;
	}
	if (info_frameA->h_total!= info_frameB->h_total)
	{
		HDMI_PRINTF("C(4)\n");
		result= FALSE;
	}
	if (info_frameA->v_total != info_frameB->v_total)
	{
		HDMI_PRINTF("C(5)\n");
		result= FALSE;
	}
	if (info_frameA->h_porch != info_frameB->h_porch)
	{
		HDMI_PRINTF("C(6)\n");
		result= FALSE;
	}
	if (info_frameA->active.x!= info_frameB->active.x)
	{
		HDMI_PRINTF("C(7)\n");
		result= FALSE;
	}
	
	if (info_frameA->active.y != info_frameB->active.y)
	{
		HDMI_PRINTF("C(8)\n");
		result= FALSE;
	}
	if (info_frameA->active.w != info_frameB->active.w)
	{
		HDMI_PRINTF("C(9)\n");
		result= FALSE;
	}
	if (info_frameA->active.h!= info_frameB->active.h)
	{
		HDMI_PRINTF("C(10)\n");
		result= FALSE;
	}
	if (info_frameA->scan_type != info_frameB->scan_type)
	{
		HDMI_PRINTF("C(11)\n");
		result= FALSE;
	}
	if (info_frameA->full_3d_timing != info_frameB->full_3d_timing)
	{
		HDMI_PRINTF("C(12)\n");
		result= FALSE;
	}
	if (info_frameA->video_lr_order!= info_frameB->video_lr_order)
	{
		HDMI_PRINTF("C(13)\n");
		result= FALSE;
	}
	if (info_frameA->vidoe_fmt != info_frameB->vidoe_fmt)
	{
		HDMI_PRINTF("C(14)\n");
		result= FALSE;
	}
	if (info_frameA->struct_3d != info_frameB->struct_3d)
	{
		HDMI_PRINTF("C(15)\n");
		result= FALSE;
	}
	if (info_frameA->mode_id != info_frameB->mode_id)
	{
		HDMI_PRINTF("C(16)\n");
		result= FALSE;
	}
	if (info_frameA->mode_table_index != info_frameB->mode_table_index) 
	{
		HDMI_PRINTF("C(17)\n");
		result= FALSE;
	}
	if (info_frameA->polarity != info_frameB->polarity)
	{
		HDMI_PRINTF("C(18)\n");
		result= FALSE;
	}
	if (info_frameA->color_space != info_frameB->color_space) 
	{
		HDMI_PRINTF("C(19)\n");
		result= FALSE;
	}
	if (info_frameA->color_depth != info_frameB->color_depth) 
	{
		HDMI_PRINTF("C(20)\n");
		result= FALSE;
	}
	if (info_frameA->color_imetry != info_frameB->color_imetry)	{
		HDMI_PRINTF("C(21)\n");
		result= FALSE;
	}

	return result;
}
#endif

int vfe_hdmi_drv_get_display_timing_info(vfe_hdmi_timing_info_t *timing_info, unsigned char vsc_ch)
{//Get current port timing info
	unsigned char port = newbase_hdmi_get_current_display_port();
	unsigned char ch = 0;
	MEASURE_TIMING_T tm;

	memset(&tm, 0, sizeof(MEASURE_TIMING_T));

	VFE_FLOW_PRINTF("[vfe_hdmi_drv_get_display_timing_info %d] starts, vsc_ch=%d\n", __LINE__, vsc_ch);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[vfe_hdmi_drv_get_display_timing_info][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if (timing_info == NULL) {
		HDMI_EMG("[vfe_hdmi_drv_get_display_timing_info][- ARG ERR]\n");
		return HDMI_DRV_ARG_ERR;
	}
	hdmi_vfe_get_hdmi_channel(port, &ch);

	timing_info->port = ch;
	if (HDMI_DRV_NO_ERR != vfe_hdmi_drv_get_port_timing_info(timing_info))
		return HDMI_DRV_STATE_ERR;

	if (newbase_hdmi_get_hvstart(port, &tm, vsc_ch))
	{
		timing_info->x_offset = tm.h_act_sta;
		timing_info->y_offset = tm.v_act_sta;
	}
	else
	{
		return HDMI_DRV_HW_ERR;	/* Need to return fail */
	}
	HDMI_WARN("[vfe_hdmi_drv_get_display_timing_info] OFMS Port:%d, ch:%d, HxV Active: %d x %d, Total: %d x %d, Freq: %d x %d\n",
		port, ch,  timing_info->active.w, timing_info->active.h, timing_info->h_total, timing_info->v_total, timing_info->h_freq, timing_info->v_freq);
	return HDMI_DRV_NO_ERR;
}

void vfe_hdmi_drv_handle_on_line_measure_error(unsigned char vfe_call)
{
	unsigned char port = 0xF;
	unsigned int onms_status_wdg_mask;
	unsigned char is_onms_error = FALSE;
	unsigned char is_reset_phy_flow = FALSE;
	unsigned char is_qms_vrr = FALSE;
	HDMI_PORT_INFO_T* p_hdmi_rx_status;
#ifdef CONFIG_FORCE_RUN_I3DDMA
	onms_onms3_status_RBUS onms_onms3_status_reg;

#endif
	onms_onms1_status_RBUS onms_onms1_status_reg;
	HDMI_WARN("[vfe_hdmi_drv_handle_on_line_measure_error] start!! vfe_call=%d\n", vfe_call);

	if (hdmi_vfe_get_connected_port(&port)<0) {
		HDMI_WARN("[vfe_hdmi_drv_handle_on_line_measure_error] no connected hdmi port=%d do nothing!\n", port);
		return ;
	}

	is_qms_vrr = newbase_hdmi_get_qms_enable(port);
#ifdef CONFIG_FORCE_RUN_I3DDMA
	onms_onms3_status_reg.regValue = hdmi_in(ONMS_onms3_status_reg);
	onms_status_wdg_mask =  current_hdmi_onms_watchdog_setting&onms_onms3_status_reg.regValue;
	HDMI_WARN("[online_i3ddma][port:%d] is_qms_vrr=%d, run_dma_onms=%d, status reg:%x, watchdog: %x, int:%x, current_hdmi_onms_watchdog_setting=%x, wdg_error=%x\n",
		port,
		is_qms_vrr,
		current_hdmi_onms_is_i3ddma, 
		onms_onms3_status_reg.regValue,
		hdmi_in(ONMS_onms3_watchdog_en_reg),
		hdmi_in(ONMS_onms3_interrupt_en_reg),
		current_hdmi_onms_watchdog_setting,
		onms_status_wdg_mask);
	if(current_hdmi_onms_is_i3ddma&&onms_status_wdg_mask)
	{
		is_onms_error |= TRUE;
		HDMI_WARN("[online_i3ddma][port:%d] onms3 watchdog detect error!!watchdog err=%x\n",port,onms_status_wdg_mask);
	}

#endif
	onms_onms1_status_reg.regValue = hdmi_in(ONMS_onms1_status_reg);
	onms_status_wdg_mask =  current_hdmi_onms_watchdog_setting&onms_onms1_status_reg.regValue;

	HDMI_WARN("[online_main][port:%d] is_qms_vrr=%d, run_dma_onms=%d, status reg:%x, watchdog: %x, int:%x, current_hdmi_onms_watchdog_setting=%x, wdg_error=%x\n",
		port,
		is_qms_vrr,
		current_hdmi_onms_is_i3ddma, 
		onms_onms1_status_reg.regValue,
		hdmi_in(ONMS_onms1_watchdog_en_reg),
		hdmi_in(ONMS_onms1_interrupt_en_reg),
		current_hdmi_onms_watchdog_setting,
		onms_status_wdg_mask);

	if((current_hdmi_onms_is_i3ddma==FALSE) &&onms_status_wdg_mask)
	{
		is_onms_error |= TRUE;
		HDMI_WARN("[online_main][port:%d] onms1 watchdog detect error!!, watchdog err=%x\n",port, onms_status_wdg_mask);
	}

	p_hdmi_rx_status = newbase_hdmi_get_rx_port_info(port);
	if(p_hdmi_rx_status !=NULL)
	{
		HDMI_WARN("[vfe_hdmi_drv_handle_on_line_measure_error port:%d] FSM:%s, is_video_output=%d, is_5V_HPD=%d, raw_tmds_phy_md_clk=%d\n",
			port,
			_hdmi_video_fsm_str(p_hdmi_rx_status->video_fsm),
			lib_hdmi_is_video_output(port),
			p_hdmi_rx_status->cable_conn,
			p_hdmi_rx_status->raw_tmds_phy_md_clk);

		if(p_hdmi_rx_status->video_fsm != MAIN_FSM_HDMI_VIDEO_READY)
		{
			;//scaler enter run flow!!
			HDMI_EMG("[vfe_hdmi_drv_handle_on_line_measure_error port:%d] FSM:%s, vfe_call=%d, ONMS watchdog check HDMIRX timing detect!!!! It is invalid flow!!\n",
				port, _hdmi_video_fsm_str(p_hdmi_rx_status->video_fsm), vfe_call);
			if(newbase_hdmi_get_check_mode_result(port) ==1)
			{
				HDMI_WARN("[vfe_hdmi_drv_handle_on_line_measure_error port:%d] Have to reset PLL only!Do not reset anything!!\n", port);
				newbase_hdmi_set_check_mode_result(port, 0);
				is_reset_phy_flow = 0;
				return;
			}
		}
	}

	is_reset_phy_flow = (vfe_call || is_onms_error);
       if(newbase_hdmi_get_online_measure_error_flag(port) == 1)
       {
           is_reset_phy_flow = 1;
           newbase_hdmi_set_online_measure_error_flag(port, 0);
           HDMI_WARN("[vfe_hdmi_drv_handle_on_line_measure_error port:%d] BAD Signal, force reset phy!!\n", port);
       }
	newbase_hdmi_detect_flow_startover(port, (is_reset_phy_flow ? 1 : 0));

#ifdef CONFIG_SUPPORT_DOLBY_VSIF
	drvif_Hdmi_SetDolbyVsifUpdateNoPktThreshold(NO_DVS_INFO_MAX);
	vfe_hdmi_drv_set_infoframe_thread_stop(_FALSE);
	set_HDMI_Dolby_VSIF_mode(DOLBY_HDMI_VSIF_DISABLE);
#endif

#if HDMI_TOGGLE_HPD_Z0_ENABLE
	lib_hdmi_toggle_hpd_z0_check(port, _TRUE);
#endif //#if HDMI_TOGGLE_HPD_Z0_ENABLE

	HDMI_WARN("[vfe_hdmi_drv_handle_on_line_measure_error port:%d] End, is_reset_phy_flow=%d (vfe_call =%d, is_onms_error=%d)\n", port, is_reset_phy_flow,  vfe_call, is_onms_error);
}


int vfe_hdmi_drv_set_drm_override_eotf(unsigned char ch, vfe_hdmi_override_eotf_e override_eotf_type)
{
	unsigned char port;
	VFE_FLOW_PRINTF("[vfe_hdmi_drv_set_drm_override_eotf] starts, ch=%d, override_eotf_type=%d\n", ch, override_eotf_type);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[vfe_hdmi_drv_set_drm_override_eotf][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
	{
		HDMI_WARN("[vfe_hdmi_drv_set_drm_override_eotf] invalid ch:%d, port: %d\n", ch, port);
		return HDMI_DRV_ARG_ERR;
	}

       if(override_eotf_type <= VFE_HDMI_OVERRIDE_EOTF_AUTO)
	    g_hdmi_drm_override_eotf[port] = override_eotf_type;
	else
	    g_hdmi_drm_override_eotf[port] = VFE_HDMI_OVERRIDE_EOTF_AUTO;

	return HDMI_DRV_NO_ERR;
}

int vfe_hdmi_drv_get_drm_override_eotf(unsigned char ch, vfe_hdmi_override_eotf_e* p_override_eotf_type)
{
	unsigned char port;
	VFE_FLOW_PRINTF("[vfe_hdmi_drv_set_drm_override_eotf] starts, ch=%d\n", ch);


	if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
	{
		HDMI_WARN("[vfe_hdmi_drv_set_drm_override_eotf] invalid ch:%d, port: %d\n", ch, port);
		return HDMI_DRV_ARG_ERR;
	}

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[vfe_hdmi_drv_set_drm_override_eotf][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if (p_override_eotf_type == NULL) {
		HDMI_EMG("[vfe_hdmi_drv_get_drm_override_eotf][- ARG ERR] p_override_eotf_type is NULL \n");
		return HDMI_DRV_ARG_ERR;
	}

	*p_override_eotf_type = g_hdmi_drm_override_eotf[port];

	return HDMI_DRV_NO_ERR;
}

int vfe_hdmi_drv_get_drm_info(vfe_hdmi_drm_t *info_frame)
{
	unsigned char port;
	HDMI_DRM_T drm_info;
	VFE_FLOW_PRINTF("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[vfe_hdmi_drv_get_drm_info][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if (hdmi_vfe_get_connected_port(&port)<0) {
		memset(info_frame, 0, sizeof(vfe_hdmi_drm_t));
		HDMI_EMG("[WARNING1] drm_info forget to connect hdmi port\n");
		return HDMI_DRV_ARG_ERR;
	}

	if(g_hdmi_drm_override_eotf[port] == VFE_HDMI_OVERRIDE_EOTF_AUTO)
	{
		if (newbase_hdmi_get_drm_infoframe(port, &drm_info))
			memcpy(info_frame, &drm_info, sizeof(vfe_hdmi_drm_t));
		else
			memset(info_frame, 0, sizeof(vfe_hdmi_drm_t));
	}
	else
	{
		memset(&drm_info, 0, sizeof(HDMI_DRM_T)); // initial to 0.
		drm_info.type_code = TYPE_CODE_DRM_PACKET;

		drm_info.ver = 1;
		drm_info.len = 26;

		drm_info.eEOTFtype = g_hdmi_drm_override_eotf[port];
		memcpy(info_frame, &drm_info, sizeof(vfe_hdmi_drm_t));
	}

#if 0
	if (lib_hdmi_get_fw_debug_bit(DEBUG_16_PRINT_DRM))
	{
		HDMI_EMG("[willychou]%s drm_info (eotf:%d, len:%d, ver:%d, x0:%d, y0:%d)\n",__func__ ,
			info_frame->eEOTFtype, info_frame->len, 
			info_frame->ver, info_frame->display_primaries_x0, 
			info_frame->display_primaries_y0);

		HDMI_EMG("[willychou]%s drm_info (x1:%d, y1:%d, x2:%d, y2:%d, wx:%d, wy:%d)\n",__func__, 
			info_frame->display_primaries_x1, info_frame->display_primaries_y1, 
			info_frame->display_primaries_x2, info_frame->display_primaries_y2, 
			info_frame->white_point_x, info_frame->white_point_y);

		HDMI_EMG("[willychou]%s drm_info (max_lum:%d, min_lum:%d, max_cont:%d, max_Frame:%d)\n",__func__, 
			info_frame->max_display_mastering_luminance, info_frame->min_display_mastering_luminance, 
			info_frame->maximum_content_light_level, info_frame->maximum_frame_average_light_level);
	}
#endif
	return HDMI_DRV_NO_ERR;
}

int vfe_hdmi_drv_get_port_vsi_info(vfe_hdmi_vsi_t *info_frame)
{
	unsigned char ch = 0x0F;
	unsigned char port = 0x0F;
	HDMI_VSI_T vsi_info;

	VFE_FLOW_PRINTF("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[vfe_hdmi_drv_get_port_vsi_info][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if (info_frame == NULL) {
		HDMI_EMG("[vfe_hdmi_drv_get_port_vsi_info][- ARG ERR] info_frame is NULL \n");
		return HDMI_DRV_ARG_ERR;
	}

	ch = info_frame->port;

	if(HAL_VFE_HDMI_CURRENT_PORT== ch)
	{//Detect non-specified port, force use current port.
		INFOFRAME_FLOW_PRINTF("[vfe_hdmi_drv_get_port_vsi_info] Non specified port, use current port!! ch = %d\n", ch);
		if (hdmi_vfe_get_connected_port(&port)<0)
		{
			HDMI_WARN("vfe_hdmi_drv_get_port_vsi_info can't get current port\n");
			return HDMI_DRV_ARG_ERR;
		}
	}
	else if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
	{
		memset(info_frame, 0, sizeof(vfe_hdmi_vsi_t));
		info_frame->port = HDMI_RX_PORT_MAX_NUM;
		HDMI_WARN("vfe_hdmi_drv_get_port_vsi_info invalid ch:%d, port: %d\n", ch, port);
		return HDMI_DRV_ARG_ERR;
	}

	memset(info_frame, 0, sizeof(vfe_hdmi_vsi_t));

	if (newbase_hdmi_get_vs_infoframe(port, &vsi_info))
	{
		info_frame->header.type    = vsi_info.VSIF_TypeCode;
		info_frame->header.version = vsi_info.VSIF_Version;
		info_frame->header.length  = vsi_info.Length;
		info_frame->checksum       = vsi_info.Checksum;
		
		memcpy(info_frame->ieee_reg_id, vsi_info.Reg_ID, VFE_HDMI_VENDOR_SPECIFIC_REGID_LEN);
		memcpy(info_frame->payload, vsi_info.Payload, VFE_HDMI_VENDOR_SPECIFIC_PAYLOAD_LEN) ;
		
		info_frame->vidoe_fmt      = newbase_hdmi_get_video_format(port);
		info_frame->struct_3d      = newbase_hdmi_get_3d_structure(port);
		info_frame->extdata_3d     = lib_hdmi_get_3d_extdata(port);
		info_frame->vic            = lib_hdmi_get_vsi_vic(port);;
		info_frame->packetStatus   = VFE_HDMI_PACKET_STATUS_UPDATED;
	}
	else
	{
		memset(info_frame, 0, sizeof(vfe_hdmi_vsi_t));
		info_frame->header.type =TYPE_CODE_VS_PACKET;
	}
	info_frame->port = ch;

	return HDMI_DRV_NO_ERR;
}
EXPORT_SYMBOL(vfe_hdmi_drv_get_port_vsi_info);

int vfe_hdmi_drv_get_port_spd_info(vfe_hdmi_spd_t *info_frame)
{
	unsigned char ch = 0x0F;
	unsigned char port = 0x0F;
	HDMI_SPD_T spd_info;

	VFE_FLOW_PRINTF("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		//HDMI_PRINTF("[vfe_hdmi_drv_get_port_spd_info][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if (info_frame == NULL) {
		HDMI_PRINTF("[vfe_hdmi_drv_get_port_spd_info][- ARG ERR] info_frame is NULL \n");
		return HDMI_DRV_ARG_ERR;
	}

	ch = info_frame->port;

	if(HAL_VFE_HDMI_CURRENT_PORT == ch)
	{//Detect non-specified port, force use current port.
		INFOFRAME_FLOW_PRINTF("[vfe_hdmi_drv_get_port_spd_info] Non specified port, use current port!! ch = %d\n", ch);
		if (hdmi_vfe_get_connected_port(&port)<0)
		{
			HDMI_PRINTF("[WARNING1] vfe_hdmi_drv_get_port_spd_info can't get current port\n");
			return HDMI_DRV_ARG_ERR;
		}
	}
	else if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
	{
		memset(info_frame, 0, sizeof(vfe_hdmi_spd_t));
		info_frame->port = HDMI_RX_PORT_MAX_NUM;
		//HDMI_WARN("vfe_hdmi_drv_get_port_spd_info invalid ch:%d, port: %d\n", ch, port);
		return HDMI_DRV_ARG_ERR;
	}

	memset(info_frame, 0, sizeof(vfe_hdmi_spd_t));

	if ((newbase_hdmi_get_spd_infoframe(port, &spd_info) == TRUE)&&
		((spd_info.ver==1 && spd_info.len==25) ||(newbase_hdmi_check_freesync_info_version(&spd_info) != FREESYNC_INFO_INVALID))) 
	{// // Report SPD info that followed CTA-861-G standard or AMD Freesync standard
		info_frame->header.type    = spd_info.type_code;
		info_frame->header.version = spd_info.ver;
		info_frame->header.length  = spd_info.len;

		memcpy(info_frame->vendor_name, spd_info.VendorName, 8);
		memcpy(info_frame->product_desc, spd_info.ProductDesc, 16);

		info_frame->src_dev_info   = spd_info.SourceInfo;
		info_frame->packetStatus   = VFE_HDMI_PACKET_STATUS_UPDATED;
	}
	else
	{
		memset(info_frame, 0, sizeof(vfe_hdmi_spd_t));
		info_frame->header.type =TYPE_CODE_SPD_PACKET;
	}

	info_frame->port = ch;
	
	return HDMI_DRV_NO_ERR;
}

int vfe_hdmi_drv_get_port_avi_info(vfe_hdmi_avi_t *info_frame)
{
	unsigned char ch = 0x0F;
	unsigned char port = 0x0F;
	HDMI_AVI_T avi_info;

#ifdef AVI_INFO_COMPARE
	static vfe_hdmi_avi_t avi_info_T;
#endif //#ifdef AVI_INFO_COMPARE

	VFE_FLOW_PRINTF("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_PRINTF("[vfe_hdmi_drv_get_port_avi_info][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if (info_frame == NULL) {
		HDMI_PRINTF("[vfe_hdmi_drv_get_port_avi_info][- ARG ERR] info_frame is NULL \n");
		return HDMI_DRV_ARG_ERR;
	}

	ch = info_frame->port;

	if(HAL_VFE_HDMI_CURRENT_PORT == ch)
	{//Detect non-specified port, force use current port.
		INFOFRAME_FLOW_PRINTF("[vfe_hdmi_drv_get_port_avi_info] Non specified port, use current port!! ch = %d\n", ch);
		if (hdmi_vfe_get_connected_port(&port)<0)
		{
			HDMI_PRINTF("[WARNING1] vfe_hdmi_drv_get_port_avi_info can't get current port\n");
			return HDMI_DRV_ARG_ERR;
		}
	}
	else if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
	{
		memset(info_frame, 0, sizeof(vfe_hdmi_avi_t));
		info_frame->port = HDMI_RX_PORT_MAX_NUM;
		HDMI_WARN("vfe_hdmi_drv_get_port_avi_info invalid ch:%d, port: %d\n", ch, port);
		return HDMI_DRV_ARG_ERR;
	}

	memset(info_frame, 0, sizeof(vfe_hdmi_avi_t));

	if (newbase_hdmi_get_avi_infoframe(port, &avi_info))
	{
		info_frame->header.type          = avi_info.type_code;
		info_frame->header.version       = avi_info.ver;
		info_frame->header.length        = avi_info.len;
		info_frame->mode                 = 1;
		info_frame->pixel_encoding       = (vfe_hdmi_avi_csc_e) avi_info.Y;
		info_frame->active_info          = (vfe_hdmi_avi_active_info_e) avi_info.A;
		info_frame->bar_info             = (vfe_hdmi_avi_bar_info_e) avi_info.B;
		info_frame->scan_info            = (vfe_hdmi_avi_scan_info_e) avi_info.S;
		info_frame->colorimetry          = (vfe_hdmi_avi_colorimetry_e) avi_info.C;
		info_frame->picture_aspect_ratio = (vfe_hdmi_avi_picture_arc_e) avi_info.M;

		info_frame->active_format_aspect_ratio = (vfe_hdmi_avi_active_format_arc_e) avi_info.R;
		info_frame->scaling                  = (vfe_hdmi_avi_scaling_e) avi_info.SC;
		info_frame->video_id_code            = (unsigned char) avi_info.VIC;
		info_frame->pixel_repeat             = avi_info.PR;
		info_frame->it_content               = (vfe_hdmi_avi_it_content_e) avi_info.ITC;

		info_frame->extended_colorimetry     = (vfe_hdmi_avi_ext_colorimetry_e) avi_info.EC;
		info_frame->rgb_quantization_range   = (vfe_hdmi_avi_rgb_quantization_range_e) avi_info.Q;
		info_frame->ycc_quantization_range   = (vfe_hdmi_avi_ycc_quantization_range_e) avi_info.YQ;
		info_frame->content_type             = (vfe_hdmi_avi_content_type_e) avi_info.CN;
		info_frame->it_content               = (vfe_hdmi_avi_it_content_e) avi_info.ITC;

		info_frame->TopBarEndLineNumber      = (avi_info.ETB15_08<<8 | avi_info.ETB07_00);
		info_frame->BottomBarStartLineNumber = (avi_info.SBB15_08<<8 | avi_info.SBB07_00);
		info_frame->LeftBarEndPixelNumber    = (avi_info.ELB15_08<<8 | avi_info.ELB07_00);
		info_frame->RightBarEndPixelNumber   = (avi_info.SRB15_08<<8 | avi_info.SRB07_00);
		info_frame->additional_colorimetry = (vfe_hdmi_avi_additional_colorimetry_e) avi_info.ACE;
		info_frame->f14= avi_info.F14;
		info_frame->packetStatus             = VFE_HDMI_PACKET_STATUS_UPDATED;
	}
	else
	{
		memset(info_frame, 0, sizeof(vfe_hdmi_avi_t));
		info_frame->header.type =TYPE_CODE_AVI_PACKET;

		//USER:Lewislee DATE:2016/10/05
		//if no avi info, it remain need send correct HDMI mode for webos
		if (newbase_hdmi_get_hdmi_mode_infoframe(port))		
			info_frame->mode = 1;
	}

	if (lib_hdmi_get_fw_debug_bit(DEBUG_15_PRINT_AVI))
	{
		HDMI_EMG("[AVI] header len = %d, PE = %s\n",info_frame->header.length,g_avi_csc_string_table[info_frame->pixel_encoding]);
		HDMI_EMG("[AVI] AVI_ACTIVE_INFO = %d, bar_info = %s\n",info_frame->active_info, g_avi_bi_string_table[info_frame->bar_info]);
		HDMI_EMG("[AVI] scan_info = %s, colorimetry = %s\n",g_avi_scan_info_string_table[info_frame->scan_info], g_avi_colorimetry_string_table[info_frame->colorimetry]);
		HDMI_EMG("[AVI] picture_aspect_ratio = %s, active_format_aspect_ratio = %d\n",g_avi_arc_string_table[info_frame->picture_aspect_ratio], info_frame->active_format_aspect_ratio);
		HDMI_EMG("[AVI] scaling = %s, video_id_code = %d\n",g_avi_scaling_string_table[info_frame->scaling], info_frame->video_id_code);
		HDMI_EMG("[AVI] pixel_repeat = %d, it_content = %s\n",info_frame->pixel_repeat, g_avi_itcontent_string_table[info_frame->it_content]);
		HDMI_EMG("[AVI] extended_colorimetry = %s(%d), rgb_quantization_range = %s\n",g_avi_extcolorimetry_string_table[info_frame->extended_colorimetry],info_frame->extended_colorimetry,  g_avi_rgb_quantization_string_table[info_frame->rgb_quantization_range]);
		HDMI_EMG("[AVI] ycc_quantization_range = %s, content_type = %s\n",g_avi_ycc_quantization_string_table[info_frame->ycc_quantization_range], g_avi_content_type_string_table[info_frame->content_type]);
		HDMI_EMG("[AVI] active_info = %s, bar_info = %s\n",g_avi_csc_string_table[info_frame->active_info], g_avi_csc_string_table[info_frame->bar_info]);
		HDMI_EMG("[AVI] TopBar=%d, BottomBar=%d, LeftBar=%d, RightBar=%d\n",info_frame->TopBarEndLineNumber, info_frame->BottomBarStartLineNumber, info_frame->LeftBarEndPixelNumber, info_frame->RightBarEndPixelNumber);
	}

	info_frame->port = ch;

#ifdef AVI_INFO_COMPARE //for debug avi data change
	if (FALSE == Cmp_AviInfoFrame(&avi_info_T,info_frame))
	{
		HDMI_PRINTF(KERN_EMERG "[ERROR] Cmp_AviInfoFrame False\n");
	}

	memcpy(&avi_info_T,info_frame,sizeof(vfe_hdmi_avi_t));
#endif //#ifdef AVI_INFO_COMPARE

	return HDMI_DRV_NO_ERR;
}

#ifdef AVI_INFO_COMPARE //for debug avi data change
bool Cmp_AviInfoFrame(vfe_hdmi_avi_t *info_frameA,vfe_hdmi_avi_t *info_frameB)
{
	bool result =TRUE;
	if (info_frameA->header.type != info_frameB->header.type)
	{
		HDMI_PRINTF("C(1), A:%x, B:%x\n", info_frameA->header.type, info_frameB->header.type);
		result= FALSE;
	}

	if (info_frameA->mode != info_frameB->mode)
	{
		HDMI_PRINTF("C(3), A:%x, B:%x\n", info_frameA->mode, info_frameB->mode);
		result= FALSE;
	}

	if (info_frameA->pixel_encoding!= info_frameB->pixel_encoding)
	{
		HDMI_PRINTF("C(4), A:%x, B:%x\n", info_frameA->pixel_encoding, info_frameB->pixel_encoding);
		result= FALSE;
	}

	if (info_frameA->active_info != info_frameB->active_info)
	{
		HDMI_PRINTF("C(5), A:%x, B:%x\n", info_frameA->active_info, info_frameB->active_info);
		result= FALSE;
	}

	if (info_frameA->bar_info != info_frameB->bar_info)
	{
		HDMI_PRINTF("C(6), A:%x, B:%x\n", info_frameA->bar_info, info_frameB->bar_info);
		result= FALSE;
	}

	if (info_frameA->scan_info!= info_frameB->scan_info)
	{
		HDMI_PRINTF("C(7), A:%x, B:%x\n", info_frameA->scan_info, info_frameB->scan_info);
		result= FALSE;
	}

	if (info_frameA->colorimetry != info_frameB->colorimetry)
	{
		HDMI_PRINTF("C(8), A:%x, B:%x\n", info_frameA->colorimetry, info_frameB->colorimetry);
		result= FALSE;
	}

	if (info_frameA->picture_aspect_ratio != info_frameB->picture_aspect_ratio)
	{
		HDMI_PRINTF("C(9), A:%x, B:%x\n", info_frameA->picture_aspect_ratio, info_frameB->picture_aspect_ratio);
		result= FALSE;
	}

	if (info_frameA->active_format_aspect_ratio!= info_frameB->active_format_aspect_ratio)
	{
		HDMI_PRINTF("C(10), A:%x, B:%x\n", info_frameA->active_format_aspect_ratio, info_frameB->active_format_aspect_ratio);
		result= FALSE;
	}

	if (info_frameA->scaling != info_frameB->scaling)
	{
		HDMI_PRINTF("C(11), A:%x, B:%x\n", info_frameA->scaling, info_frameB->scaling);
		result= FALSE;
	}

	if (info_frameA->video_id_code != info_frameB->video_id_code)
	{
		HDMI_PRINTF("C(12), A:%x, B:%x\n", info_frameA->video_id_code, info_frameB->video_id_code);
		result= FALSE;
	}

	if (info_frameA->pixel_repeat!= info_frameB->pixel_repeat)
	{
		HDMI_PRINTF("C(13), A:%x, B:%x\n", info_frameA->pixel_repeat, info_frameB->pixel_repeat);
		result= FALSE;
	}

	if (info_frameA->it_content != info_frameB->it_content)
	{
		HDMI_PRINTF("C(14), A:%x, B:%x\n", info_frameA->it_content, info_frameB->it_content);
		result= FALSE;
	}

	if (info_frameA->extended_colorimetry != info_frameB->extended_colorimetry)
	{
		HDMI_PRINTF("C(15), A:%x, B:%x\n", info_frameA->extended_colorimetry, info_frameB->extended_colorimetry);
		result= FALSE;
	}

	if (info_frameA->rgb_quantization_range != info_frameB->rgb_quantization_range)
	{
		HDMI_PRINTF("C(16), A:%x, B:%x\n", info_frameA->rgb_quantization_range, info_frameB->rgb_quantization_range);
		result= FALSE;
	}

	if (info_frameA->ycc_quantization_range != info_frameB->ycc_quantization_range)
	{
		HDMI_PRINTF("C(17), A:%x, B:%x\n", info_frameA->ycc_quantization_range, info_frameB->ycc_quantization_range);
		result= FALSE;
	}

	if (info_frameA->content_type != info_frameB->content_type)
	{
		HDMI_PRINTF("C(18), A:%x, B:%x\n", info_frameA->content_type, info_frameB->content_type);
		result= FALSE;
	}

	if (info_frameA->TopBarEndLineNumber != info_frameB->TopBarEndLineNumber)
	{
		HDMI_PRINTF("C(19), A:%x, B:%x\n", info_frameA->TopBarEndLineNumber, info_frameB->TopBarEndLineNumber);
		result= FALSE;
	}

	if (info_frameA->BottomBarStartLineNumber != info_frameB->BottomBarStartLineNumber)
	{
		HDMI_PRINTF("C(20), A:%x, B:%x\n", info_frameA->BottomBarStartLineNumber, info_frameB->BottomBarStartLineNumber);
		result= FALSE;
	}

	if (info_frameA->LeftBarEndPixelNumber != info_frameB->LeftBarEndPixelNumber)
	{
		HDMI_PRINTF("C(21), A:%x, B:%x\n", info_frameA->LeftBarEndPixelNumber, info_frameB->LeftBarEndPixelNumber);
		result= FALSE;
	}

	if (info_frameA->TopBarEndLineNumber != info_frameB->TopBarEndLineNumber)
	{
		HDMI_PRINTF("C(22), A:%x, B:%x\n", info_frameA->TopBarEndLineNumber, info_frameB->TopBarEndLineNumber);
		result= FALSE;
	}

	if (info_frameA->RightBarEndPixelNumber != info_frameB->RightBarEndPixelNumber)
	{
		HDMI_PRINTF("C(23), A:%x, B:%x\n", info_frameA->RightBarEndPixelNumber, info_frameB->RightBarEndPixelNumber);
		result= FALSE;
	}

	if (info_frameA->packetStatus != info_frameB->packetStatus)
	{
		HDMI_PRINTF("C(24), A:%x, B:%x\n", info_frameA->packetStatus, info_frameB->packetStatus);
		result= FALSE;
	}

	if (info_frameA->header.length != info_frameB->header.length)
	{
		HDMI_PRINTF("C(2):(%d,%d)\n",info_frameA->header.length,info_frameB->header.length);
		result= FALSE;
	}
/*
	if(FALSE == result)
	{

		HDMI_PRINTF("header.type, A:%x, B:%x\n",info_frameA->header.type, info_frameB->header.type);
		HDMI_PRINTF("header.version, A:%x, B:%x\n",info_frameA->header.version, info_frameB->header.version);
		HDMI_PRINTF("header.length, A:%x, B:%x\n",info_frameA->header.length, info_frameB->header.length);
		HDMI_PRINTF("mode, A:%x, B:%x\n",info_frameA->mode, info_frameB->mode);
		HDMI_PRINTF("pixel_encoding, A:%x, B:%x\n",info_frameA->pixel_encoding, info_frameB->pixel_encoding);
		HDMI_PRINTF("active_info, A:%x, B:%x\n",info_frameA->active_info, info_frameB->active_info);
		HDMI_PRINTF("bar_info, A:%x, B:%x\n",info_frameA->bar_info, info_frameB->bar_info);
		HDMI_PRINTF("scan_info, A:%x, B:%x\n",info_frameA->scan_info, info_frameB->scan_info);
		HDMI_PRINTF("colorimetry, A:%x, B:%x\n",info_frameA->colorimetry, info_frameB->colorimetry);
		HDMI_PRINTF("picture_aspect_ratio, A:%x, B:%x\n",info_frameA->picture_aspect_ratio, info_frameB->picture_aspect_ratio);
		HDMI_PRINTF("active_format_aspect_ratio, A:%x, B:%x\n",info_frameA->active_format_aspect_ratio, info_frameB->active_format_aspect_ratio);
		HDMI_PRINTF("scaling, A:%x, B:%x\n",info_frameA->scaling, info_frameB->scaling);
		HDMI_PRINTF("video_id_code, A:%x, B:%x\n",info_frameA->video_id_code, info_frameB->video_id_code);
		HDMI_PRINTF("pixel_repeat, A:%x, B:%x\n",info_frameA->pixel_repeat, info_frameB->pixel_repeat);
		HDMI_PRINTF("it_content, A:%x, B:%x\n",info_frameA->it_content, info_frameB->it_content);
		HDMI_PRINTF("extended_colorimetry, A:%x, B:%x\n",info_frameA->extended_colorimetry, info_frameB->extended_colorimetry);
		HDMI_PRINTF("rgb_quantization_range, A:%x, B:%x\n",info_frameA->rgb_quantization_range, info_frameB->rgb_quantization_range);
		HDMI_PRINTF("it_content, A:%x, B:%x\n",info_frameA->it_content, info_frameB->it_content);
		HDMI_PRINTF("TopBarEndLineNumber, A:%x, B:%x\n",info_frameA->TopBarEndLineNumber, info_frameB->TopBarEndLineNumber);
		HDMI_PRINTF("BottomBarStartLineNumber, A:%x, B:%x\n",info_frameA->BottomBarStartLineNumber, info_frameB->BottomBarStartLineNumber);
		HDMI_PRINTF("LeftBarEndPixelNumber, A:%x, B:%x\n",info_frameA->LeftBarEndPixelNumber, info_frameB->LeftBarEndPixelNumber);
		HDMI_PRINTF("RightBarEndPixelNumber, A:%x, B:%x\n",info_frameA->RightBarEndPixelNumber, info_frameB->RightBarEndPixelNumber);
		HDMI_PRINTF("packetStatus, A:%x, B:%x\n",info_frameA->packetStatus, info_frameB->packetStatus);
		HDMI_PRINTF("port, A:%x, B:%x\n",info_frameA->port, info_frameB->port);
	}
*/
	return result;

}
#endif //#ifdef AVI_INFO_COMPARE

int vfe_hdmi_drv_get_port_allpacket_info(vfe_hdmi_all_packet_t *info_frame)
{
	unsigned char avi_ch = 0x0F;
	unsigned char spd_ch = 0x0F;
	unsigned char vsi_ch = 0x0F;
	unsigned char port = 0x0F;
	VFE_FLOW_PRINTF("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_PRINTF("[vfe_hdmi_drv_get_port_allpacket_info][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if ( info_frame == NULL) {
		HDMI_PRINTF("[[vfe_hdmi_drv_get_port_allpacket_info]][- ARG ERR]\n");
		return HDMI_DRV_ARG_ERR;
	}

	avi_ch = info_frame->stAVI.port;
	spd_ch = info_frame->stSPD.port;
	vsi_ch = info_frame->stVSI.port;

	if (hdmi_vfe_get_hdmi_port(avi_ch, &port)<0)
	{
		HDMI_WARN("vfe_hdmi_drv_get_port_allpacket_info invalid ch:%d, port: %d\n", avi_ch, port);
		return HDMI_DRV_ARG_ERR;
	}

	memset(info_frame, 0, sizeof(vfe_hdmi_all_packet_t));

	info_frame->stAVI.port = avi_ch;
	info_frame->stSPD.port = spd_ch;
	info_frame->stVSI.port = vsi_ch;

	if (newbase_hdmi_get_video_state(port) < MAIN_FSM_HDMI_MEASURE) {
		HDMI_PRINTF( "[WARNING1] get_port_allpacket no data\n");
		return HDMI_DRV_NO_ERR;
	}

	if (vfe_hdmi_drv_get_port_avi_info(&info_frame->stAVI) != HDMI_DRV_NO_ERR) {
		//memset(&info_frame->stAVI, 0, sizeof(vfe_hdmi_avi_t));
		//info_frame->stAVI.packetStatus = VFE_HDMI_PACKET_STATUS_NOT_RECEIVED;
		info_frame->mode = info_frame->stAVI.mode;
	}

	if (vfe_hdmi_drv_get_port_spd_info(&info_frame->stSPD) != HDMI_DRV_NO_ERR) {
		//memset(&info_frame->stSPD, 0, sizeof(vfe_hdmi_spd_t));
		//info_frame->stSPD.packetStatus = VFE_HDMI_PACKET_STATUS_NOT_RECEIVED;
	}

	if (vfe_hdmi_drv_get_port_vsi_info(&info_frame->stVSI) != HDMI_DRV_NO_ERR) {
		//memset(&info_frame->stVSI, 0, sizeof(vfe_hdmi_vsi_t));
		//info_frame->stVSI.packetStatus = VFE_HDMI_PACKET_STATUS_NOT_RECEIVED;
	}

	return HDMI_DRV_NO_ERR;
}
EXPORT_SYMBOL(vfe_hdmi_drv_get_port_allpacket_info);

int vfe_hdmi_drv_get_port_acp_info(vfe_hdmi_acp_t *info_frame)
{
	unsigned char ch = 0x0F;
	unsigned char port = 0x0F;
	HDMI_ACP_T acp_info;

	pr_debug("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		//HDMI_PRINTF("[vfe_hdmi_drv_get_port_acp_info][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if (info_frame == NULL) {
		HDMI_PRINTF("[vfe_hdmi_drv_get_port_acp_info][- ARG ERR] info_frame is NULL \n");
		return HDMI_DRV_ARG_ERR;
	}

	ch = info_frame->port;

	if(HAL_VFE_HDMI_CURRENT_PORT == ch)
	{//Detect non-specified port, force use current port.
		INFOFRAME_FLOW_PRINTF("[vfe_hdmi_drv_get_port_acp_info] Non specified port, use current port!! ch = %d\n", ch);
		if (hdmi_vfe_get_connected_port(&port)<0)
		{
			HDMI_PRINTF("[WARNING1] vfe_hdmi_drv_get_port_acp_info can't get current port\n");
			return HDMI_DRV_ARG_ERR;
		}
	}
	else if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
	{
		memset(info_frame, 0, sizeof(vfe_hdmi_acp_t));
		info_frame->port = HDMI_RX_PORT_MAX_NUM;
		//HDMI_WARN("vfe_hdmi_drv_get_port_acp_info invalid ch:%d, port: %d\n", ch, port);
		return HDMI_DRV_ARG_ERR;
	}

	memset(info_frame, 0, sizeof(vfe_hdmi_acp_t));

	if (newbase_hdmi_get_acp_infoframe(port, &acp_info) == TRUE)
	{ // // Report ACP packet, follow HDMI 1.4 spec
		info_frame->packet_type	=	acp_info.ACP_Packet_Type;
		info_frame->acp_type	=	acp_info.ACP_Type;
		info_frame->reserved		=	acp_info.Reserved_0;

		memcpy(info_frame->acp_type_dependent, acp_info.ACP_Type_Dependent, VFE_HDMI_ACP_TYPE_DEPENDENT_LEN);
	}
	else
	{
		memset(info_frame, 0, sizeof(vfe_hdmi_acp_t));
	}

	info_frame->port = ch;

	return HDMI_DRV_NO_ERR;
}


#define HDMI_VSIF_CHECKSUM_LEN 1
/*------------------------------------------------------------------
 * Func : vfe_hdmi_drv_get_port_multi_vsi_info
 *
 * Desc : get multi vsi info from the given hdmi channel
 *
 * Para : [IN] ch    : HDMI channel (0~N)
 *        [IN] p_vsi : VSIF output list
 *        [IN] n_vsi : size of VSIF output array (
 *
 * Retn : 0 : successed, others : failed
 *------------------------------------------------------------------*/
int vfe_hdmi_drv_get_port_multi_vsi_info(
	unsigned char           ch, 
	KADP_HAL_VFE_HDMI_IN_PACKET_T* p_vsi,
	unsigned char           n_vsi
	)
{
	HDMI_VSI_T vsi_info[4] = {{0}};
	unsigned char port = 0x0F;
	unsigned char cnt = sizeof(vsi_info)/sizeof(HDMI_VSI_T);
	unsigned char i;

	VFE_FLOW_PRINTF("[%s %d] starts, ch=%d, n_vsi=%d\n", __func__, __LINE__, ch, n_vsi);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if (p_vsi == NULL || n_vsi==0) {
		HDMI_EMG("[- ARG ERR] p_vsi_list is NULL \n");
		return HDMI_DRV_ARG_ERR;
	}

	if(HAL_VFE_HDMI_CURRENT_PORT== ch)
	{//Detect non-specified port, force use current port.
		INFOFRAME_FLOW_PRINTF("[vfe_hdmi_drv_get_port_multi_vsi_info] Non specified port, use current port!! ch = %d\n", ch);
		if (hdmi_vfe_get_connected_port(&port)<0)
		{
			HDMI_WARN("[WARNING1] vfe_hdmi_drv_get_port_multi_vsi_info can't get current port\n");
			return HDMI_DRV_ARG_ERR;
		}
	}
	else if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
	{
		ch = HDMI_RX_PORT_MAX_NUM;
		HDMI_WARN("[WARNING1] vfe_hdmi_drv_get_port_multi_vsi_info invalid ch:%d, port: %d\n", ch, port);

		return HDMI_DRV_ARG_ERR;
	}


	for (i=0; i<n_vsi; i++) {
		p_vsi[i].type = 0; // set the type to reserved
	}

	if (cnt > n_vsi)
		cnt = n_vsi;

	cnt = newbase_hdmi_get_multi_vs_infoframe(port, vsi_info, cnt);

	for (i=0; i<cnt; i++) {
		unsigned char index = 0, len = 0;

		// Humm... should we convert it to CEA-861 format ????
		p_vsi[i].type = vsi_info[i].VSIF_TypeCode;
		p_vsi[i].version = vsi_info[i].VSIF_Version;
		p_vsi[i].length = vsi_info[i].Length;

		/* PB0:0 : Checksum */
		p_vsi[i].dataBytes[index] = vsi_info[i].Checksum;

		/* PB1:3 : IEEE ID (0x000C03) */
		index += HDMI_VSIF_CHECKSUM_LEN;
		len = HAL_VFE_HDMI_VENDOR_SPECIFIC_REGID_LEN;

		if (index + len > HAL_VFE_HDMI_PACKET_DATA_LENGTH)
			len = HAL_VFE_HDMI_PACKET_DATA_LENGTH - index;

		memcpy(&p_vsi[i].dataBytes[index], &vsi_info[i].Reg_ID, len);

		/* PB4:27 : Data Bytes */
		index += len;
		len = HAL_VFE_HDMI_VENDOR_SPECIFIC_PAYLOAD_LEN - HDMI_VSIF_CHECKSUM_LEN; /* data is 24bytes */

		if (index + len > HAL_VFE_HDMI_PACKET_DATA_LENGTH)
			len = HAL_VFE_HDMI_PACKET_DATA_LENGTH - index;

		memcpy(&p_vsi[i].dataBytes[index], &vsi_info[i].Payload, len);
	}

	return HDMI_DRV_NO_ERR;
}


int vfe_hdmi_get_emp(unsigned char ch, vfe_hdmi_em_type_e type, unsigned char* pkt_buf, unsigned short len)
{
	unsigned char port;
	
	VFE_FLOW_PRINTF("[%s %d] starts, ch=%d, type=%d, len=%d\n", __func__, __LINE__, ch, type, len);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[vfe_hdmi_get_emp][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return -1;
	}

	if (pkt_buf == NULL) {
		HDMI_EMG("[vfe_hdmi_get_emp][- ARG ERR] p_vsi_list is NULL \n");
		return -1;
	}

	if (hdmi_vfe_get_connected_channel(&ch)<0 ||
		hdmi_vfe_get_hdmi_port(ch, &port)<0) 	
		return -1;

	switch(type)
	{
	case KADP_VFE_HDMI_EMP_VSEMDS:
		return (len >= EM_VSEM_INFO_LEN) ? newbase_hdmi_get_vsem_emp(port, pkt_buf) : -1;

	case KADP_VFE_HDMI_EMP_HDRDM:
		return (len >= MAX_EM_HDR_INFO_LEN) ? newbase_hdmi_get_hdr_emp(port, pkt_buf) : -1;

	case KADP_VFE_HDMI_EMP_VTEM:
	{
		int result = 0;
		if(len >= EM_VTEM_INFO_LEN) 
		{
			result = newbase_hdmi_get_vtem_emp(port, pkt_buf);
			//ECN RL6748-677? (7f c0 00 84 00 01 00 01 00 04 xx yy zz ww) for 
			pkt_buf[0] = TYPE_CODE_EMP_PACKET;
			pkt_buf[1] = 0xc0;
			pkt_buf[2] = 0x00;
			if((pkt_buf[10] & (_BIT2)))
			    pkt_buf[3] = 0x86; //QMS_EN=1, sync =1
			else
			    pkt_buf[3] = 0x84; //QMS_EN=0, sync=0
			pkt_buf[4] = 0x00;
			pkt_buf[5] = 0x01;
			pkt_buf[6] = 0x00;
			pkt_buf[7] = 0x01;
			pkt_buf[8] = 0x00;
			pkt_buf[9] = 0x04;
		}
		else
		{
			result =-1;
		}
		return result;
	}

	case KADP_VFE_HDMI_EMP_CVTEM:
		return (len >= EM_CVTEM_INFO_LEN) ? newbase_hdmi_get_cvtem_emp(port, pkt_buf) : -1;

	case KADP_VFE_HDMI_EMP_SBTM:
		return (len >= EM_SBTM_INFO_LEN) ? newbase_hdmi_get_sbtm_emp(port, pkt_buf) : -1;

	case KADP_VFE_HDMI_EMP_RSV0:
		return (len >= RSV0_EM_LEN) ? newbase_hdmi_get_rsv0_emp(port, pkt_buf) : -1;

	case KADP_VFE_HDMI_EMP_RSV1:
		return (len >= RSV1_EM_LEN) ? newbase_hdmi_get_rsv1_emp(port, pkt_buf) : -1;

	default:
		return -1;
	}
}


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

#ifdef CONFIG_SUPPORT_DOLBY_VSIF
void vfe_hdmi_drv_set_infoframe_thread_stop(unsigned char ucStop)
{

	if(ucStop != vfe_hdmi_drv_get_infoframe_thread_stop())
		rtd_pr_hdmi_emerg("Info chg=%d\n", ucStop);
	hdmi_set_infoframe_thread_stop(ucStop);
}
#endif

void vfe_hdmi_drv_set_infoframe_thread_stop_nonseamphore(unsigned char ucStop)// for ISR use
{
    hdmi_set_infoframe_thread_stop_nonseamphore(ucStop);
}

unsigned char vfe_hdmi_drv_get_infoframe_thread_stop(void)
{
#ifdef CONFIG_SUPPORT_DOLBY_VSIF
	return hdmi_get_infoframe_thread_stop();
#else
	return _FALSE;
#endif
}

void vfe_hdmi_drv_infoframe(void)
{
    unsigned char port;

    if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
        return ;
    }

    if (hdmi_vfe_get_connected_port(&port)<0) {
        return;
    }

	newbase_hdmi_infoframe_update();
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

int vfe_hdmi_drv_get_connection_state(vfe_hdmi_connect_state_t *con_state)
{
	unsigned char port;
	
	if (hdmi_vfe_get_hdmi_port(con_state->port, &port)==0 &&
		newbase_hdmi_get_5v_state(port))
		con_state->state = 1;
	else
		con_state->state = 0;

	return HDMI_DRV_NO_ERR;
}
EXPORT_SYMBOL(vfe_hdmi_drv_get_connection_state);

int vfe_hdmi_drv_enable_hpd(void)
{
	unsigned char ch;
	HDMI_EDID_INFO("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[vfe_hdmi_drv_enable_hpd][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if (resume_hpd_control) {
		HDMI_EMG("need to ensure minimum hpd_low period.\n");
		resume_hpd_control = 0;
		return HDMI_DRV_NO_ERR;
	}

	for (ch=0; ch<HDMI_PORT_TOTAL_NUM; ch++)
	{
		unsigned char port;

		if (hdmi_vfe_get_hdmi_port(ch, &port)==0)
		{
			lib_hdmi_z0_set(port, LN_ALL, 1);   // Q10235 PS4 toggle HPD only, it causes noise
			newbase_hdmi_set_hpd(port, 1);
		}
	}

	return HDMI_DRV_NO_ERR;

}

int vfe_hdmi_drv_disable_hpd(void)
{
	unsigned char ch;
	HDMI_EDID_INFO("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[vfe_hdmi_drv_disable_hpd][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}
	
	newbase_hdmi_cancel_delayed_hpd(); // cancel delayed hpd if necessary

	for (ch=0; ch<HDMI_PORT_TOTAL_NUM; ch++)
	{
		unsigned char port;
		
		if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
			continue;
		
		// Q10235 PS4 toggle HPD only, it causes noise
		if (newbase_hdmi_get_video_state(port) >= MAIN_FSM_HDMI_DISPLAY_ON)
			lib_hdmi_z0_set(port, LN_ALL, 0);
		
		newbase_hdmi_set_hpd(port, 0);
	}

	return HDMI_DRV_NO_ERR;
}

int vfe_hdmi_drv_restart_hpd(
	unsigned char           ch
	)
{
	unsigned char port;
	unsigned int hpd_dealy = 0;

	HDMI_EDID_INFO("[vfe_hdmi_drv_restart_hpd] starts, ch=%d\n", ch);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EDID_EMG("[vfe_hdmi_drv_restart_hpd][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
	{
		HDMI_EDID_ERR("%s ch%d convert Physical HDMI port fail\n" , __func__ , ch);
		return HDMI_DRV_ARG_ERR;
	}

	hpd_dealy = newbase_hdmi_get_hpd_low_delay();
	HDMI_EDID_INFO("vfe_hdmi_drv_restart_hpd with default delay=%d ms \n", hpd_dealy);

	newbase_hdmi_set_hpd(port, 0);//pull low HPD for specify ports

	newbase_hdmi_enable_hpd_with_delay(hpd_dealy,port);

	return HDMI_DRV_NO_ERR;
}
EXPORT_SYMBOL(vfe_hdmi_drv_restart_hpd);

int vfe_hdmi_drv_disable_internal_edid_access(unsigned char ch)
{
	unsigned char port;
	
	HDMI_EDID_INFO("[%s %d] starts, ch=%d\n", __func__, __LINE__, ch);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EDID_EMG("[vfe_hdmi_drv_disable_internal_edid_access][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if (hdmi_vfe_get_hdmi_port(ch, &port)<0) {
		HDMI_EDID_EMG("[- ARG ERR]\n");
		return HDMI_DRV_ARG_ERR;
	}
	
	newbase_hdmi_edid_enable(port, 0);

	return HDMI_DRV_NO_ERR;
}
EXPORT_SYMBOL(vfe_hdmi_drv_disable_internal_edid_access);

/*------------------------------------------------------------------
 * Func :  vfe_hdmi_drv_read_edid
 *
 * Desc : Read EDID from a given HDMI channel
 *
 * Para : [IN] ch     : HDMI channel (0~N-1)
 *        [IN] p_edid : EDID data to be written
 *        [IN] len    : size of EDID data. the possible value of it is 256 & 512
 *
 * Retn :   0 : ok; others fail 
 *------------------------------------------------------------------*/
int vfe_hdmi_drv_read_edid(
	unsigned char           ch, 
	unsigned char*          p_edid, 
	unsigned short          len
	)
{
	unsigned char port;

	HDMI_EDID_INFO("[%s %d] starts, ch=%d, len=%d\n", __func__, __LINE__, ch, len);

	if (g_hdmi_drv_state == VFE_HDMI_DRV_SUSPEND_STATE) {
		HDMI_EDID_EMG("read edid of HDMI-%d failed, unable update edid in suspend state\n", ch);
		return HDMI_DRV_STATE_ERR;
	}

	if (p_edid==NULL || (len!=256 && len!=384 && len!=512))
	{
		HDMI_EDID_EMG("read edid of HDMI-%d failed, invalid parameter\n", ch);
		return HDMI_DRV_ARG_ERR;
	}
	
	if (hdmi_vfe_get_hdmi_port(ch, &port)<0) {
		HDMI_EDID_EMG("read edid of HDMI-%d failed, get HDMI port failed\n", ch);
		return HDMI_DRV_ARG_ERR;
	}

	if(!newbase_hdmi_read_edid(port, p_edid, len))
	{
		HDMI_EDID_EMG("%s: err Read EDID fail\n", __func__);
		return HDMI_DRV_HW_ERR;
	}

	return HDMI_DRV_NO_ERR;
}    
EXPORT_SYMBOL(vfe_hdmi_drv_read_edid);

/*------------------------------------------------------------------
 * Func : vfe_hdmi_drv_write_edid
 *
 * Desc : Write EDID to a given HDMI channel
 *
 * Para : [IN] ch     : HDMI channel (0~N-1)
 *        [IN] p_edid : EDID data to be written
 *        [IN] len    : size of EDID data. the possible value of it is 256 & 512
 *
 * Retn :   0 : ok; others fail 
 *------------------------------------------------------------------*/ 
int vfe_hdmi_drv_write_edid(
	unsigned char           ch,
	const unsigned char*    p_edid,
	unsigned short          len
	)
{
	unsigned char port = 0xF;
	unsigned char edid[512];

	HDMI_EDID_INFO("[vfe_hdmi_drv_write_edid] starts, ch=%d, len=%d\n", ch, len);

	if (g_hdmi_drv_state == VFE_HDMI_DRV_SUSPEND_STATE) {
		HDMI_EDID_EMG("write edid of HDMI-%d failed, unable update edid in suspend state\n", ch);
		return HDMI_DRV_STATE_ERR;
	}
	
	if (p_edid==NULL || (len!=256 && len!=384 && len!=512))
	{
		HDMI_EDID_EMG("write edid of HDMI-%d failed, invalid parameter\n", ch);
		return HDMI_DRV_ARG_ERR;
	}

	if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
	{
		HDMI_EDID_ERR("[vfe_hdmi_drv_write_edid]write edid of HDMI-%d failed, get HDMI port failed\n", ch);
		return HDMI_DRV_ARG_ERR;
	}

#if ENABLE_FORCE_DRIVER_EDID_MODIFY  //Need to control by Android
	if((GET_PANEL_REFRESH_RATE() == 144))
	{// for 4k144 special setting
		len =384;
		p_edid = p_default_hdmi_edid_384_table[0];
		HDMI_EMG("[vfe_hdmi_drv_write_edid] ch:%d, Use 4K144 384B EDID special setting\n", ch);
	}
	else if(GET_PANEL_REFRESH_RATE() == 165)
	{// for 3k165 special setting
		len =384;
		p_edid = p_default_hdmi_edid_384_table[2];
		HDMI_EMG("[vfe_hdmi_drv_write_edid] ch:%d, Use 3K165 384B EDID special setting\n", ch);
	}
#endif

	memset(edid, 0, len);                    // copy edid for modification		
	
	memcpy(edid, p_edid, len);                    // copy edid for modification
	newbase_hdmi_check_hdmi_edid(port, edid + 128);   // update edid if necessary
	if(newbase_hdmi_save_edid(port, edid, len))
	{
		if(newbase_hdmi_write_edid(port, edid, len)==0)
		{
			HDMI_EDID_EMG("[vfe_hdmi_drv_write_edid] err write EDID fail\n");
			return HDMI_DRV_HW_ERR;
		}
	}
	else
	{
		HDMI_EDID_INFO("No need to write, ch=%d, port=%d\n", ch, port);
	}
	HDMI_EDID_INFO("[vfe_hdmi_drv_write_edid] End, ch=%d, len=%d\n", ch, len);
	return HDMI_DRV_NO_ERR;
}
EXPORT_SYMBOL(vfe_hdmi_drv_write_edid);

int vfe_hdmi_drv_read_hdcp(unsigned char pksv[5], unsigned char device_key[320])
{
	HDMI_INFO("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[vfe_hdmi_drv_read_hdcp][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if (pksv==NULL || device_key==NULL) {
		HDMI_EMG("[- ARG ERR]\n");
		return HDMI_DRV_ARG_ERR;
	}

	newbase_hdmi_hdcp14_read_hdcp_key(pksv, device_key);
	return HDMI_DRV_NO_ERR;
}
EXPORT_SYMBOL(vfe_hdmi_drv_read_hdcp);

int vfe_hdmi_drv_write_hdcp(unsigned char pksv[5], unsigned char device_key[320])
{
	HDMI_PRINTF("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[vfe_hdmi_drv_write_hdcp][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}
	
	if (pksv==NULL || device_key==NULL) {
		HDMI_EMG("[- ARG ERR]\n");
		return HDMI_DRV_ARG_ERR;
	}

	newbase_hdmi_hdcp14_write_hdcp_key(pksv, device_key, APPLY_KEY_IMMEDATELY);

	return HDMI_DRV_NO_ERR;
}
EXPORT_SYMBOL(vfe_hdmi_drv_write_hdcp);

int vfe_hdmi_drv_get_src_ksv_data(unsigned char ch, unsigned char ksv[5])
{
	unsigned char port;
	HDMI_INFO("[%s %d] starts, ch=%d\n", __func__, __LINE__, ch);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[vfe_hdmi_drv_get_src_ksv_data][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if (ksv == NULL) {
		HDMI_EMG("[vfe_hdmi_drv_get_src_ksv_data][- ARG ERR] ksv_data is NULL \n");
		return HDMI_DRV_ARG_ERR;
	}
	
	if (hdmi_vfe_get_hdmi_port(ch, &port)<0) {
		HDMI_EMG("[vfe_hdmi_drv_get_src_ksv_data][- ARG ERR]\n");
		return HDMI_DRV_ARG_ERR;
	}

	if (ch >= HDMI_RX_PORT_MAX_NUM || g_hdmi_channel_connect_flag[ch]==0) {
		memset(ksv, 0, sizeof(HDCP_AKSV_SIZE_BYTES));
		HDMI_WARN("[WARNING] forget to connect hdmi channel %d\n", ch);
	} else {
		newbase_hdmi_hdcp14_read_aksv(port, ksv);
		HDMI_PRINTF("%s = %02x %02x %02x %02x %02x\n", 
		    __func__, ksv[0], ksv[1], ksv[2], ksv[3], ksv[4]);
	}

	return HDMI_DRV_NO_ERR;
}

unsigned char vfe_hdmi_drv_get_clock_status(unsigned char ch)
{
	unsigned char port;
	
	HDMI_INFO("[%s %d] starts, ch=%d\n", __func__, __LINE__, ch);
	
	if (hdmi_vfe_get_connected_port(&port)<0) {
		HDMI_EMG("[WARNING] forget to connect hdmi channel\n");
		return false;
	}

	if (newbase_hdmi_get_video_state(port) < MAIN_FSM_HDMI_DISPLAY_ON) {
		HDMI_EMG("[%s]VIDEO_FSM not stable\n", __func__);
		return false;
	}

	if (newbase_hdmi_get_tmds_clockx10(port) > 250)
		return true;        // clock locked

	return false;
}


int vfe_hdmi_drv_get_audio_coding_type(unsigned char *coding_type)
{
	HDMI_AUDIO_ST audio_status;
	unsigned char port;

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		*coding_type = VFE_HDMI_AUDIO_HDMI_NO_AUDIO;
		return HDMI_DRV_STATE_ERR;
	}

	// ToDO : calling Audio API to get related information
	HDMI_WARN("Get audio coding type failed, need to call audio API to instead\n");
	
	port = newbase_hdmi_get_current_display_port();
	
	if (newbase_hdmi_audio_get_audio_status(port, &audio_status)<0)
		return HDMI_DRV_STATE_ERR;
	
	switch (audio_status.coding_type) {
	case AUDIO_FORMAT_DVI:         *coding_type = VFE_HDMI_AUDIO_HDMI_DVI; break;
	case AUDIO_FORMAT_NO_AUDIO:    *coding_type = VFE_HDMI_AUDIO_HDMI_NO_AUDIO; break;
	case AUDIO_FORMAT_PCM:         *coding_type = VFE_HDMI_AUDIO_HDMI_PCM; break;
	case AUDIO_FORMAT_AC3:         *coding_type = VFE_HDMI_AUDIO_HDMI_AC3; break;
	case AUDIO_FORMAT_DTS:         *coding_type = VFE_HDMI_AUDIO_HDMI_DTS; break;
	case AUDIO_FORMAT_AAC:         *coding_type = VFE_HDMI_AUDIO_HDMI_AAC; break;
	case AUDIO_FORMAT_DEFAULT:     *coding_type = VFE_HDMI_AUDIO_HDMI_PCM; break;
	case AUDIO_FORMAT_MPEG:        *coding_type = VFE_HDMI_AUDIO_HDMI_MPEG; break;
	case AUDIO_FORMAT_DTS_HD_MA:   *coding_type = VFE_HDMI_AUDIO_HDMI_DTS_HD_MA; break;
	case AUDIO_FORMAT_DTS_EXPRESS: *coding_type = VFE_HDMI_AUDIO_HDMI_DTS_EXPRESS; break;
	case AUDIO_FORMAT_DTS_CD:      *coding_type = VFE_HDMI_AUDIO_HDMI_DTS_CD; break;
	case AUDIO_FORMAT_EAC3:        *coding_type = VFE_HDMI_AUDIO_HDMI_DTS_CD; break;
	case AUDIO_FORMAT_EAC3_ATMOS:  *coding_type = VFE_HDMI_AUDIO_HDMI_DTS_CD; break;
	case AUDIO_FORMAT_MAT:         *coding_type = VFE_HDMI_AUDIO_HDMI_DTS_CD; break;
	case AUDIO_FORMAT_MAT_ATMOS:   *coding_type = VFE_HDMI_AUDIO_HDMI_DTS_CD; break;
	case AUDIO_FORMAT_TRUEHD:      *coding_type = VFE_HDMI_AUDIO_HDMI_DTS_CD; break;
	default:                       *coding_type = VFE_HDMI_AUDIO_HDMI_NO_AUDIO; break; 
	}

	return HDMI_DRV_NO_ERR;
}
EXPORT_SYMBOL(vfe_hdmi_drv_get_audio_coding_type);

bool vfe_hdmi_drv_get_avmute(void)
{
	unsigned char port;

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		rtd_pr_hdmi_debug("[- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if (hdmi_vfe_get_connected_port(&port)<0) {
		rtd_pr_hdmi_debug("[WARNING] forget to connect hdmi channel\n");
		return false;
	}
	
	return newbase_hdmi_get_avmute(port);
}


int vfe_hdmi_drv_write_hdcp_apply(unsigned char hdcp22,  unsigned char *pksv, unsigned char  apply)
{
	rtd_pr_hdmi_notice("[%s %d] starts\n", __func__, __LINE__);

	if(apply)
	{
		if(hdcp22)
		{
			newbase_hdmi_save_customer_hdcp2p2(pksv, 878);
		}
		else
		{
			newbase_hdmi_save_customer_hdcp1p4(pksv, 325);
			newbase_hdmi_hdcp14_write_hdcp_key(pksv, pksv+5,apply);

		}
	}
	else
	{
		if(hdcp22)
		{
			newbase_hdmi_save_customer_hdcp2p2(pksv, 878);
		}
		else
		{
			newbase_hdmi_save_customer_hdcp1p4(pksv, 325);
		}
	}

	return HDMI_DRV_NO_ERR;
}
EXPORT_SYMBOL(vfe_hdmi_drv_write_hdcp_apply);

int vfe_hdmi_drv_get_rx_port_cnt(unsigned char* port_cnt)
{
	int i;
	unsigned char port;
	unsigned char count = 0;

	for (i=0; i<HDMI_RX_PORT_MAX_NUM; i++) {
		if (hdmi_vfe_get_hdmi_port(i, &port)==0)
			count++;
	}

	*port_cnt = count;

	return HDMI_DRV_NO_ERR;
}

char vfe_hdmi_drv_get_arc_channel(void)
{
	char arc_port = 0;
	unsigned char arc_ui_channel = 0;

	arc_port = newbase_hdmi_get_arc_port_index();

	if((arc_port > HDMI_PCB_ARC_EARC_NONE) && (arc_port < HDMI_RX_PORT_MAX_NUM) && (hdmi_vfe_get_hdmi_channel(arc_port, &arc_ui_channel) >= 0))
	{
		return arc_ui_channel;
	}
	else
	{
		if(HDMI_PCB_ARC_EARC_MULTI == arc_port)
			return 0xf;
	}

	return HDMI_PCB_ARC_EARC_NONE;
}

char vfe_hdmi_drv_get_earc_channel(void)
{
	char earc_port = 0;
	unsigned char earc_ui_channel = 0;

	earc_port = newbase_hdmi_get_earc_port_index();

	if((earc_port > HDMI_PCB_ARC_EARC_NONE) && (earc_port < HDMI_RX_PORT_MAX_NUM) && (hdmi_vfe_get_hdmi_channel(earc_port, &earc_ui_channel) >= 0))
	{
		return earc_ui_channel;
	}
	else
	{
		if(HDMI_PCB_ARC_EARC_MULTI == earc_port)
			return 0xf;
	}

	return HDMI_PCB_ARC_EARC_NONE;
}

int vfe_hdmi_drv_get_hdmi_support_version(vfe_hdmi_support_ver_t* hdmi_version)
{
	unsigned char ch = hdmi_version->port;
	unsigned char port;
	
	HDMI_INFO("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE)
		return HDMI_DRV_STATE_ERR;
	
	hdmi_version->hdmi_ver = 0; 
	
	if (hdmi_vfe_get_hdmi_port(ch, &port)==0)
	{
		//hdmi_ver : [bit 0] : HDMI 1.4 / [Bit 1] : HDMI 2.0
		hdmi_version->hdmi_ver = 3;
	}
	else //if(hdcp_ver->port >= HDMI_RX_PORT_MAX_NUM)
	{
		hdmi_version->hdmi_ver = 0; 
	}


	return HDMI_DRV_NO_ERR;
}


int vfe_hdmi_drv_get_hdcp_version(vfe_hdmi_hdcp_ver_t* hdcp_version)
{
	unsigned char ch = hdcp_version->port;
	unsigned char port;
	unsigned char result = HDMI_DRV_STATE_ERR;
	
	HDMI_INFO("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE)
		return HDMI_DRV_STATE_ERR;

	hdmi_time_mesaure_start();
	
	if (hdmi_vfe_get_hdmi_port(ch, &port)==0)
	{
		HDMI_HDCP_E newbase_hdcp_version = newbase_hdcp_get_auth_mode(port);
		switch(newbase_hdcp_version)
		{
		case NO_HDCP:
			hdcp_version->hdcp_ver = VFE_NO_HDCP;
			break;
		case HDCP14:
			hdcp_version->hdcp_ver = VFE_HDCP_1P4;
			break;
		case HDCP22:
			hdcp_version->hdcp_ver = VFE_HDCP_2P2;
			break;
		default:
			hdcp_version->hdcp_ver = VFE_HDCP_1P4;
			break;
		}

		result =  HDMI_DRV_NO_ERR;
	}
	else	
	{
		result =  HDMI_DRV_ARG_ERR;
		hdcp_version->hdcp_ver = 0;
		HDMI_WARN("vfe_hdmi_drv_get_hdcp_version invalid ch:%d, port: %d\n", ch, port);
	}

	hdmi_time_mesaure_stop();

	return result;
}

int vfe_hdmi_drv_set_hdcp_support(vfe_hdcp_table_t hdcp_support)
{
	HDMI_INFO("[%s %d] starts\n", __func__, __LINE__);
	
	switch (hdcp_support.hdcp_ver)
	{
	case VFE_HDCP_1P4:
		HDCP_key_loaded[VFE_HDCP_1P4] = hdcp_support.key_load;
		break;
		
	case VFE_HDCP_2P2:
		HDCP_key_loaded[VFE_HDCP_2P2] = hdcp_support.key_load;
		break;
		
	default:
		HDMI_EMG("[WARNING] invalid hdcp version %d\n", hdcp_support.hdcp_ver);
		return HDMI_DRV_ARG_ERR;
	}

	return HDMI_DRV_NO_ERR;
}

int vfe_hdmi_drv_get_hdcp_support(vfe_hdcp_table_t* hdcp_support)
{
	HDMI_INFO("[%s %d] starts\n", __func__, __LINE__);
	
	switch (hdcp_support->hdcp_ver)
	{
	case VFE_HDCP_1P4:
		hdcp_support->key_load = HDCP_key_loaded[VFE_HDCP_1P4];
		break;
		
	case VFE_HDCP_2P2:
		hdcp_support->key_load = HDCP_key_loaded[VFE_HDCP_2P2];
		break;
		
	default:
		hdcp_support->key_load = false;
		HDMI_EMG("[WARNING] invalid hdcp version %d\n", hdcp_support->hdcp_ver);
		return HDMI_DRV_ARG_ERR;
	}

	return HDMI_DRV_NO_ERR;
}

int vfe_hdmi_drv_SetHDCPReceiverId(unsigned char* ReceiverId)
{
	HDMI_INFO("[%s %d] starts\n", __func__, __LINE__);

	if (ReceiverId== NULL)
		return HDMI_DRV_ARG_ERR;

	memcpy(HDCP_2p2_ReceiverdId, ReceiverId, HDCP_2P2_RECEIVER_ID_SIZE);

	return HDMI_DRV_NO_ERR;
}

int vfe_hdmi_drv_GetHDCPReceiverId(unsigned char* ReceiverId)
{
	HDMI_INFO("[%s %d] starts\n", __func__, __LINE__);

	if (ReceiverId == NULL)
		return HDMI_DRV_ARG_ERR;

	memcpy(ReceiverId, HDCP_2p2_ReceiverdId, HDCP_2P2_RECEIVER_ID_SIZE);	

	return HDMI_DRV_NO_ERR;
}

int vfe_hdmi_drv_get_HDCPStatus(unsigned char ch, unsigned int* hdcp_status)
{
    unsigned char port = 0x0F;
    unsigned char aksv[5] = {0};
    unsigned char ri[2] = {0};
    //unsigned char hdcp2p2_auth_state = 0;
    HDMI_HDCP_E newbase_hdcp_version =NO_HDCP;

    HDMI_INFO("[%s %d] starts, ch=%d\n", __func__, __LINE__, ch);

    if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
        HDMI_EMG("[vfe_hdmi_drv_get_HDCPStatus][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
        return HDMI_DRV_STATE_ERR;
    }

    if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
    {
        HDMI_ERR("convert Physical HDMI port fail\n");
        return HDMI_DRV_ARG_ERR;
    }
    newbase_hdcp_version = newbase_hdcp_get_auth_mode(port);
    switch(newbase_hdcp_version)
    {
    case NO_HDCP:
        *hdcp_status = (VFE_NO_HDCP <<16) | VFE_HDCP_STATUS_IDLE;
        break;
    case HDCP14:
        if (newbase_hdmi_hdcp14_read_aksv(port,aksv) && newbase_hdmi_hdcp14_read_ri(port,ri))
            *hdcp_status = (VFE_HDCP_1P4 <<16) | VFE_HDCP_STATUS_AUTH_PASS;
        else
            *hdcp_status = (VFE_HDCP_1P4 <<16) | VFE_HDCP_STATUS_AUTH_FAIL;
        break;
    case HDCP22:
        if(lib_hdmi_hdcp22_get_auth_done(port))
            *hdcp_status = (VFE_HDCP_2P2 <<16) | VFE_HDCP_STATUS_AUTH_PASS;
        else
            *hdcp_status = (VFE_HDCP_2P2 <<16) | VFE_HDCP_STATUS_AUTH_FAIL;
        break;
    default:
        *hdcp_status = (VFE_NO_HDCP <<16) | VFE_HDCP_STATUS_IDLE;
        break;
    }
    return HDMI_DRV_NO_ERR;
}

int vfe_hdmi_drv_hdcp_repeater_en(unsigned char en)
{
    newbase_hdmi_hdcp_repeater_enable((en) ? 1 : 0);
    return 0;
}

int vfe_hdmi_drv_set_down_stream_hdcp_topology(const DOWN_STREAM_HDCP_TOPOLOGY_INFO* p_topology)
{
    unsigned short hdcp_status[2];

    if (p_topology==NULL)
        return -1;

    hdcp_status[0] = p_topology->hdcp14_status;
    hdcp_status[1] = p_topology->hdcp22_status;

    return newbase_hdmi_hdcp_repeater_update_downstream_topology(hdcp_status, p_topology->p_topology, p_topology->topology_size);
}

int vfe_hdmi_drv_set_hpd(vfe_hdmi_hpd_t* hpd_value)
{
	unsigned char port;
	HDMI_EDID_INFO("[vfe_hdmi_drv_set_hpd ch:%d] hpd_ctl=%d\n", hpd_value->port, hpd_value->hpd_ctl);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EDID_EMG("[vfe_hdmi_drv_set_hpd][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}
	
	if (hdmi_vfe_get_hdmi_port(hpd_value->port, &port)<0)
	{
		HDMI_EDID_ERR("convert Physical HDMI port fail\n");
		return HDMI_DRV_ARG_ERR;
	}

	hdmi_time_mesaure_start();

	if(hpd_value->hpd_ctl) {
		lib_hdmi_z0_set(port, LN_ALL, 1);   // Q10235 PS4 toggle HPD only, it causes noise
		newbase_hdmi_set_hpd(port, 1);
	} else {
		lib_hdmi_z0_set(port, LN_ALL, 0);	// Q10235 PS4 toggle HPD only, it causes noise
		newbase_hdmi_set_hpd(port, 0);
	}

	hdmi_time_mesaure_stop();

	return HDMI_DRV_NO_ERR;
}

int vfe_hdmi_drv_get_logical_port(unsigned char* ch)
{
	HDMI_INFO("[%s %d] starts\n", __func__, __LINE__);
	
	if (ch==NULL)
		return HDMI_DRV_ARG_ERR;
	
	if (hdmi_vfe_get_connected_channel(ch)<0)
		*ch = HDMI_RX_PORT_MAX_NUM;   // If HDMI not connect, it will return HDMI_RX_PORT_MAX_NUM	

	return HDMI_DRV_NO_ERR;
}


/*------------------------------------------------------------------
 * Func : vfe_hdmi_drv_set_config
 *
 * Desc : api that used for hdmi control
 *
 * Para : [IN] ch     : HDMI channel (0~N-1)
 *        [IN] cmd    : command id (defined in vfe_cmd_id.h)
 *        [IN] param  : comamnd value 
 *
 * Retn :   0 : ok; others fail 
 *------------------------------------------------------------------*/ 
int vfe_hdmi_drv_set_config(
    unsigned char               ch, 
    unsigned short              cmd, 
    unsigned int                param,
    unsigned int                reserved
    )
{
    unsigned char port;

    HDMI_INFO("[%s %d] starts, ch=%d, cmd=%d, param=%d\n", __func__, __LINE__, ch, cmd, param);

    switch(cmd)
    {
    case VFE_HDMI_HPD_LOW_DELAY:

        newbase_hdmi_set_hpd_low_delay(param);
        break;

    //----------------------------------------------
    // PHY related
    //----------------------------------------------
    case VFE_HDMI_PHY_MANUAL_EQ_EN:

        if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
            return -1;

        newbase_hdmi_set_eq_mode(port, param);
        newbase_hdmi_manual_eq(port,7,7,7); 
        break;
    
    case VFE_HDMI_PHY_MANUAL_EQ_PARM:

        if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
            return -1;

        //reserved is B/G/R channel
        newbase_hdmi_manual_eq_ch(port,reserved,param);
        break;

    //----------------------------------------------
    // HDCP related
    //----------------------------------------------
    case VFE_HDMI_HDCP22_ENABLE:
        
        if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
            return -1;

        newbase_hdmi_hdcp_disable_hdcp2(port, (param) ? 0 : 1);
        break;
        
    case VFE_HDMI_HDCP22_REAUTH:
                
        if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
            return -1;
            
        newbase_hdmi_hdcp22_set_reauth(port);
        break;

    default:
        return -1;
        break;
    }

    return 0;
}

/*------------------------------------------------------------------
 * Func : vfe_hdmi_drv_get_config
 *
 * Desc : api that used for hdmi control
 *
 * Para : [IN] ctrl     : ctrl structure
 *
 * Retn :   0 : ok; others fail 
 *------------------------------------------------------------------*/ 
int vfe_hdmi_drv_get_config(
    unsigned char               ch, 
    unsigned short              cmd, 
    unsigned int*                param
    )
{
    int result = -1;
    switch(cmd)
    {
    case VFE_HDMI_VRR_GET_VFREQ:
    {
        unsigned short u16VFreq = 0;
        result = vfe_hdmi_drv_get_currentVRRVFrequency(ch, &u16VFreq);
        *param = (unsigned int)u16VFreq;
        break;
    }
    case VFE_HDMI_PHY_TMDS_STATUS:
    {
        unsigned char u8Tmds_status = 0;
        result = vfe_hdmi_drv_get_tmds_status(ch, &u8Tmds_status);
        *param = (unsigned int)u8Tmds_status;
        break;
    }
    case VFE_HDMI_HDCP_STATUS:
    {
        result = vfe_hdmi_drv_get_HDCPStatus(ch, param);
        break;
    }
    case VFE_HDMI_SCDC_GET_STATUS:
    {
        unsigned int val = 0;
        result = vfe_hdmi_drv_get_SCDC_Status(ch, &val);
        *param = val;
        break;
    }
    default:
        return -1;
        break;
    }
    return result;
}

/*------------------------------------------------------------------
 * Func : vfe_hdmi_drv_get_diagnostic_data
 *
 * Desc : api that used for diagnostic
 *
 * Para : [IN] ch     : HDMI channel (0~N-1)
 *        [IN] cmd    : command id (defined in vfe_cmd_id.h)
 *        [IN] param  : comamnd value
 *
 * Retn :   0 : ok; others fail
 *------------------------------------------------------------------*/
int vfe_hdmi_drv_get_diagnostic_data(vfe_hdmi_diagnostic_data* data,unsigned char* pdata)
{
	//rtd_pr_hdmi_notice("[%s %d] starts\n", __func__, __LINE__);
	KADP_HAL_VFE_HDMI_DIAGNOSTICS_INFO_T dagnostics_info;
	unsigned int dagnostics_error =0;
	HDMI_AUDIO_ST audio_status;
	HDMI_DRM_T drm_status;
	unsigned char ch_num;
	unsigned char ch;
	unsigned char port = 0;
	HDMI_HDCP_ST* p_hdcp_st;

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if (data == NULL) {
		HDMI_EMG("[- ARG ERR] data is NULL \n");
		return HDMI_DRV_ARG_ERR;
	}

	if (pdata == NULL) {
		HDMI_EMG("[- ARG ERR] pdata is NULL \n");
		return HDMI_DRV_ARG_ERR;
	}

	switch(data->id)
	{
	case GENERAL_INFO:

		memset(pdata, 0, sizeof(KADP_HAL_VFE_HDMI_DIAGNOSTICS_INFO_T));
		if (hdmi_vfe_get_connected_channel(&ch)<0 || hdmi_vfe_get_hdmi_port(ch, &port)<0)
		{
			HDMI_WARN("get_diagnostic  info forget to connect hdmi port\n");
			return HDMI_DRV_ARG_ERR;
		}
		if (newbase_hdmi_get_video_state(port) < MAIN_FSM_HDMI_DISPLAY_ON) {
			HDMI_WARN("get_diagnostic info is not ready\n");
			break;
		}

		if(data->data_len != sizeof(KADP_HAL_VFE_HDMI_DIAGNOSTICS_INFO_T)){
			HDMI_PRINTF("DIAGNOSTICS size is wrong \n");
		}

		dagnostics_info.general.port = data->port;
		dagnostics_info.general.hdmi_5v =(BOOLEAN)newbase_hdmi_get_5v_state(port);
		dagnostics_info.general.hpd =(BOOLEAN)newbase_hdmi_get_hpd(port);
		dagnostics_info.general.RxSense = (BOOLEAN)newbase_hdmi_get_hpd(port);
		dagnostics_info.general.eHDMIMode =(KADP_HAL_VFE_HDMI_MODE_T)newbase_hdmi_get_hdmi_mode(port);
		dagnostics_info.general.video_width = (data->port == ch)?(GET_MODE_IHWIDTH()):0;//timing_info.h_act_len;
		dagnostics_info.general.video_height = (data->port == ch)?(GET_MODE_IVHEIGHT()):0;//timing_info.v_act_len;
		dagnostics_info.general.frame_rate_x100_hz = (data->port == ch)?(GET_MODE_IVFREQ()*10):0;
		dagnostics_info.general.color_space=(KADP_HAL_VFE_HDMI_AVI_CSC_T)newbase_hdmi_get_colorspace_reg(port);
		dagnostics_info.general.color_depth = (KADP_HAL_VFE_HDMI_COLOR_DEPTH_T)newbase_hdmi_get_colordepth(port);
		dagnostics_info.general.colorimetry = (KADP_HAL_VFE_HDMI_AVI_EXT_COLORIMETRY_T)newbase_hdmi_get_colorimetry(port);//diffs with lg
		dagnostics_info.general.audio_format = 0;
		dagnostics_info.general.sampling_freq = 0;
		dagnostics_info.general.audio_channel_number = 0;

		if (newbase_hdmi_get_drm_infoframe(port, &drm_status))
		{
			switch (drm_status.eEOTFtype)
			{
			case 0:
			case 1:
				dagnostics_info.general.hdr_type = KADP_VFE_HDMI_HDR_TYPE_SDR;
				break;
			case 2:
				dagnostics_info.general.hdr_type = KADP_VFE_HDMI_HDR_TYPE_HDR10;
				break;
			case 3:
				dagnostics_info.general.hdr_type = KADP_VFE_HDMI_HDR_TYPE_HLG;
				break;
			case 4:
				dagnostics_info.general.hdr_type = KADP_VFE_HDMI_HDR_TYPE_DOLBYVISION;
				break;
			default:
				dagnostics_info.general.hdr_type = 0xFF;	//UNKNOW
				break;
			}
		}
		else
		{
			dagnostics_info.general.hdr_type = KADP_VFE_HDMI_HDR_TYPE_SDR;
		}
		
		dagnostics_info.phy.lockStaus = (BOOLEAN)newbase_rxphy_get_setphy_done(port);
		dagnostics_info.phy.tmdsClockKHz = newbase_hdmi_get_tmds_clockx10(port)*100;
		dagnostics_info.phy.tmdsBandwidthMbps = newbase_hdmi_get_tmds_clockx10(port); //unit Mbps

		for (ch_num=0; ch_num<MAX_NUMBER_OF_CHANNEL; ch_num++)
		{
			dagnostics_info.phy.phyCtleEQMinRage[ch_num] = 0;
			dagnostics_info.phy.phyCtleEQMaxRage[ch_num]= 0;
			dagnostics_info.phy.phyCtleEQResult[ch_num]=0;
			dagnostics_info.phy.phyError[ch_num]= 0;
		}

		dagnostics_info.link.video_width_real = (data->port == ch)?(GET_MODE_IHWIDTH()):0;
		dagnostics_info.link.video_height_real = (data->port == ch)?(GET_MODE_IVHEIGHT()):0;
		dagnostics_info.link.video_htotal_real= (data->port == ch)?(GET_MODE_IHTOTAL()):0;
		dagnostics_info.link.video_vtotal_real= (data->port == ch)?(GET_MODE_IVTOTAL()):0;
		dagnostics_info.link.video_pixel_clock = newbase_hdmi_get_pixelclock(dagnostics_info.link.video_htotal_real , dagnostics_info.link.video_vtotal_real , (dagnostics_info.general.frame_rate_x100_hz/100) , dagnostics_info.general.color_depth);//unit kHz

		if (newbase_hdmi_audio_get_audio_status(port, &audio_status)==0)
		{
			dagnostics_info.audio.ChannelStatusBits= lib_hdmi_audio_get_channel_status_bit(port);
			dagnostics_info.audio.LayoutBitValue= lib_hdmi_audio_get_audio_packet_layout_value(port);
			dagnostics_info.audio.pcm_N= audio_status.acr_info.n;
			dagnostics_info.audio.pcm_CTS= audio_status.acr_info.cts;

			dagnostics_info.general.audio_format = (KADP_HAL_AUDIO_HDMI_TYPE_T)(audio_status.coding_type);
			dagnostics_info.general.sampling_freq = audio_status.acr_info.acr_freq;
			dagnostics_info.general.audio_channel_number = audio_status.channel_num;
		}
		else
		{
			dagnostics_info.audio.ChannelStatusBits= 0;
			dagnostics_info.audio.LayoutBitValue= 0;
			dagnostics_info.audio.pcm_N= 0;
			dagnostics_info.audio.pcm_CTS= 0;
			dagnostics_info.general.audio_format = 0;
			dagnostics_info.general.sampling_freq = 0;
			dagnostics_info.general.audio_channel_number = 0;
		}

		dagnostics_info.hdcp14.encEn_14 =(BOOLEAN)(lib_hdmi_hdcp22_is_onoff(port)==0 && lib_hdmi_hdcp14_is_onoff(port))?1:0;
		dagnostics_info.hdcp14.status = dagnostics_info.hdcp14.encEn_14;
		newbase_hdmi_hdcp14_read_an(port , dagnostics_info.hdcp14.An);
		newbase_hdmi_hdcp14_read_aksv(port ,dagnostics_info.hdcp14.Aksv);
		newbase_hdmi_hdcp14_read_bksv(port ,dagnostics_info.hdcp14.Bksv);
		newbase_hdmi_hdcp14_read_ri(port ,dagnostics_info.hdcp14.Ri);
		newbase_hdmi_hdcp14_read_bcaps(port ,(unsigned char*) &(dagnostics_info.hdcp14.Bcaps));
		newbase_hdmi_hdcp14_read_bstatus(port ,(unsigned char*) &(dagnostics_info.hdcp14.Bstatus));
		dagnostics_info.hdcp22.encEn_22 = (BOOLEAN)(lib_hdmi_hdcp22_is_onoff(port) && lib_hdmi_hdcp14_is_onoff(port)==0)?1:0;;
		dagnostics_info.hdcp22.status = dagnostics_info.hdcp22.encEn_22;

		dagnostics_info.scdc.bit_ratio= (BOOLEAN)(lib_hdmi_scdc_read(port,  SCDC_TMDS_CONFIGURATION)&_BIT1)>>1;
		dagnostics_info.scdc.scramble_enable= (BOOLEAN)(lib_hdmi_scdc_read(port,  SCDC_TMDS_CONFIGURATION)&_BIT0);
		dagnostics_info.scdc.scramble_status= (BOOLEAN)(lib_hdmi_scdc_read(port,  SCDC_TMDS_SCRAMBLER_STATUS)&_BIT0);
		dagnostics_info.scdc.clock_detect= (BOOLEAN)(lib_hdmi_scdc_read(port,  SCDC_STATUS_FLAGS)&_BIT0);

		for (ch_num=0; ch_num<MAX_NUMBER_OF_CHANNEL; ch_num++)
		{
			dagnostics_info.scdc.ch_locked[ch_num]=(BOOLEAN)((lib_hdmi_scdc_read(port,  SCDC_STATUS_FLAGS)&(0x01<<(ch_num+1)))?1:0);
			dagnostics_info.scdc.LTP_req[ch_num]=0;
		}
		dagnostics_info.scdc.ch_error_count[0]=newbase_hdmi_get_ced_error_cnt(port,0);
		dagnostics_info.scdc.ch_error_count[1]=newbase_hdmi_get_ced_error_cnt(port,1);
		dagnostics_info.scdc.ch_error_count[2]=newbase_hdmi_get_ced_error_cnt(port,2);
		dagnostics_info.scdc.ch_error_count[3]= 0;
		dagnostics_info.scdc.FLT_Ready = 0;
		dagnostics_info.scdc.RS_Counter_Bits = 0;
		dagnostics_info.scdc.DSCFailed= 0;
		dagnostics_info.scdc.FRLMode= 0;
		dagnostics_info.scdc.FFELevel= 0;
		dagnostics_info.scdc.FRL_Start= 0;
		dagnostics_info.scdc.FLT_Update= 0;

		memcpy(pdata,&dagnostics_info,sizeof(KADP_HAL_VFE_HDMI_DIAGNOSTICS_INFO_T));
		break;

	case SET_ERR_INIT_VAL:

		break;

	case GET_ERR_INFO:

		memset(pdata, 0, sizeof(unsigned char));

		if (hdmi_vfe_get_hdmi_port(data->port, &port)<0)
		{
			dagnostics_error = 0xFF;//KADP_HDMI_ERROR_FAILED
			memcpy(pdata,&dagnostics_error,sizeof(unsigned char));
			return HDMI_DRV_ARG_ERR;
		}

		p_hdcp_st = newbase_hdcp_get_hdcp_status(port);

		if(newbase_hdmi_ced_err_thd_check(port, DIAGNOSTIC_CED_ERROR_THD))//CED_ERROR //4 frame(64ms) will get one result
		{
			dagnostics_error |= 0x20;
			HDMI_EMG("Diagnostics CED error\n");
			//KADP_HDMI_ERROR_CED_ERROR;
		}

		if(newbase_hdmi_tmds_err_thd_check(port, DIAGNOSTIC_TMDS_ERROR_THD))//TMDS_ERROR //4 frame(64ms) will get one result
		{
			dagnostics_error |= 0x04;
			HDMI_EMG("Diagnostics TMDS error\n");
			//KADP_HDMI_ERROR_TMDS_ERROR;
		}

		if(newbase_hdmi_gcp_err_det(port))//GCP_ERROR
		{
			dagnostics_error |= 0x01;
			HDMI_EMG("Diagnostics GCP error\n");
			//KADP_HDMI_ERROR_GCP_ERROR;
		}

		if((newbase_hdcp_get_auth_mode(port) == HDCP22) && (p_hdcp_st->hdcp_reauth_cnt>0))
		{//hdcp 2p2 enable
			dagnostics_error |= 0x02;//KADP_HDMI_ERROR_HDCP22_REAUTH;
			HDMI_EMG("Diagnostics HDCP22_REAUTH error, port=%d, hdcp_reauth_cnt=%d\n", port, p_hdcp_st->hdcp_reauth_cnt);
		}

		if(newbase_hdmi_get_online_measure_error_flag(port) && (newbase_hdmi_get_video_state(port) >= MAIN_FSM_HDMI_DISPLAY_ON))
		{
			dagnostics_error |= 0x80;
			//KADP_HDMI_ERROR_UNSTABLE_SYNC;
		}
		memcpy(pdata,&dagnostics_error,sizeof(unsigned char));
		break;
	}
	
	return HDMI_DRV_NO_ERR;
}
EXPORT_SYMBOL(vfe_hdmi_drv_get_diagnostic_data);

int vfe_hdmi_drv_get_currentVRRVFrequency(unsigned char ch, unsigned short *vFreq)
{
	unsigned char port = 0x0F;
	unsigned char result = HDMI_DRV_HW_ERR;
	*vFreq = 0;
	
	//HDMI_PRINTF("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_EMG("[vfe_hdmi_drv_get_currentVRRVFrequency][- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}
	
	if (hdmi_vfe_get_connected_port(&port)<0) 
	{//get current port
		HDMI_EMG("[WARNING] vfe_hdmi_drv_get_currentVRRVFrequency forget to connect hdmi port\n");
		return HDMI_DRV_ARG_ERR;
	}

	if (newbase_hdmi_get_vrr_enable(port) ||newbase_hdmi_get_qms_enable(port))
	{
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
		unsigned short get_v_freq = 0;
		if(VRR_FRAMERATE_FROM_SCALER == get_hdmi_vrr_framerate(&get_v_freq))
		{
			*vFreq = get_v_freq;
			result = HDMI_DRV_NO_ERR;
		}
		else
#endif
		{
			*vFreq = 0;
			result = HDMI_DRV_STATE_ERR;
		}
	}
	else
	{// Reply 0, when non vrr.
		*vFreq = 0;
		result = HDMI_DRV_NO_ERR;	//TBD, non VRR mode
	}

	return result;
}

/*------------------------------------------------------------------
 * Func :  vfe_hdmi_drv_get_tmds_status
 *
 * Desc : Read TMDS status  from a given HDMI channel
 *
 * Para : [IN] ch     : HDMI channel (0~N-1)
 *        [IN] p_tmds_status : tmes status, 0: set phy is not done. 1: set phy done after get tmds clock, >2: TBD
 *
 * Retn :   0 : ok; others fail 
 *------------------------------------------------------------------*/
int vfe_hdmi_drv_get_tmds_status(unsigned char ch, unsigned char *p_tmds_status)
{
	unsigned char port = 0x0F;
	unsigned char result = HDMI_DRV_HW_ERR;

	VFE_FLOW_PRINTF("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_PRINTF("[- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if (p_tmds_status == NULL) {
		HDMI_PRINTF("[- ARG ERR] p_tmds_status is NULL \n");
		return HDMI_DRV_ARG_ERR;
	}

	*p_tmds_status = 0;

       if(HAL_VFE_HDMI_CURRENT_PORT == ch)
       {//Detect non-specified port, force use current port.
           INFOFRAME_FLOW_PRINTF("[vfe_hdmi_drv_get_port_avi_info] Non specified port, use current port!! ch = %d\n", ch);
           if (hdmi_vfe_get_connected_port(&port)<0)
           {
		HDMI_PRINTF("[WARNING1] vfe_hdmi_drv_get_port_avi_info can't get current port\n");
		return HDMI_DRV_ARG_ERR;
	    }
       }
       else if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
	{
		HDMI_PRINTF("[WARNING1] vfe_hdmi_drv_get_clock_status  invalid ch:%d, port: %d\n", ch, port);
		return HDMI_DRV_ARG_ERR;
	}

	*p_tmds_status = newbase_rxphy_get_setphy_done(port);
	//HDMI_PRINTF("p_tmds_status =%d, io_result =%d\n", *p_tmds_status, result);
	result = HDMI_DRV_NO_ERR;

	return result;
}

int vfe_hdmi_drv_get_SCDC_Status(unsigned char ch, unsigned int *val)
{
	unsigned char port = 0x0F;
	unsigned char result = HDMI_DRV_HW_ERR;
	unsigned short tmds_config_updated_counter =0; //TO DO

	rtd_pr_hdmi_debug("[%s %d] starts\n", __func__, __LINE__);

	if (g_hdmi_drv_state != VFE_HDMI_DRV_SERVICE_STATE) {
		HDMI_ERR("[- STATE ERR] fsm = %d\n", g_hdmi_drv_state);
		return HDMI_DRV_STATE_ERR;
	}

	if (hdmi_vfe_get_connected_port(&port)<0)
	{//get current port
		HDMI_WARN("[WARNING] vfe_hdmi_drv_get_SCDC_STATUS forget to connect hdmi port\n");
		return HDMI_DRV_ARG_ERR;
	}

	*val = lib_hdmi_scdc_read(port,  SCDC_TMDS_CONFIGURATION) |
	(lib_hdmi_scdc_read(port,  SCDC_STATUS_FLAGS)<<8) |
	(tmds_config_updated_counter <<16);

	result = HDMI_DRV_NO_ERR;

	return result;
}


#ifdef CONFIG_PM

extern void HDMI_set_detect_flag(unsigned char enable);

void vfe_hdmi_drv_suspend(void)
{
	unsigned char ch;
	unsigned char port;
	HDMI_INFO("[%s %d] starts\n", __func__, __LINE__);
	memset(&g_resume_param, 0, sizeof(hdmi_drv_resume_st));
	g_resume_param.channel = HDMI_RX_PORT_MAX_NUM;

	if (g_hdmi_drv_state == VFE_HDMI_DRV_SERVICE_STATE) 
	{
		/*get which port is connect*/
		for (ch = 0; ch < HDMI_RX_PORT_MAX_NUM; ch++) 
		{
			if (g_hdmi_channel_connect_flag[ch])
				g_resume_param.channel = ch;

			if (hdmi_vfe_get_hdmi_port(ch, &port)!=0)
				continue;

			newbase_hdmi_release_source(port);  // release resource
		}

		/* STR ON, DC ON:
		ori  : lo->webos(hi)->resume_init(lo)->webos(hi)
		new: lo->resume_init(lo)->webos(hi)  */
		HDMI_PRINTF("vfe_hdmi_drv_suspend : resume channel :%d\n",g_resume_param.channel);

		newbase_hdmi_uninit();
	}
}


void vfe_hdmi_drv_resume_init(void)
{
	unsigned int ch;
	int i = 0;
	unsigned char port;
	unsigned int hpd_compensation[HDMI_PORT_TOTAL_NUM] = {0};
	unsigned int hdmi_resume_curtime_from_bootup = 0;
	unsigned int str_low_duration = 0;
	HDMI_INFO("[vfe_hdmi_drv_resume_init] starts!!\n");
	
	newbase_hdmi_init();
	
	for(ch = 0; ch < HDMI_RX_PORT_MAX_NUM; ch++)
	{
		g_hdmi_channel_connect_flag[ch] = 0;
		
		if (hdmi_vfe_get_hdmi_port(ch, &port)<0)
			continue;
		
		newbase_hdmi_set_hpd(port, 0);  //pull low HPD 

		newbase_hdmi_init_source(port);
	}

#if CONFIG_STR_HPD_LOW_SPEEDUP_FROM_BOOTCODE
	hdmi_resume_curtime_from_bootup =   (hdmi_in(TIMER_TC7CVR_reg) / (TIMER_CLOCK / CONFIG_SYS_HZ)); //ms. start from AUX code. only use for dc on.
#endif

	for(i = 0; i < HDMI_PORT_TOTAL_NUM; i++)
	{
		str_low_duration = newbase_hdmi_get_str_hpd_low_ms(i);

#if HDMI_STR_TOGGLE_HPD_ENABLE
		if (HDMI_STR_HPD_TOGGLE_ENTER == lib_hdmi_get_str_toggle_hpd_state(i)) {
			lib_hdmi_set_str_toggle_hpd_state(port, HDMI_STR_HPD_TOGGLE_READY);
			if (newbase_hdmi_get_comp_hpd_low_ms(i) != 0)
				str_low_duration = newbase_hdmi_get_comp_hpd_low_ms(i);  // COMP: some TX behaves abnormal with STR low duration, replace with specific low duration.
		}
#endif //#if HDMI_STR_TOGGLE_HPD_ENABLE

		if (str_low_duration > (hdmi_resume_curtime_from_bootup))
			hpd_compensation[i] = str_low_duration - hdmi_resume_curtime_from_bootup;
		else
			hpd_compensation[i] = 0;

		resume_hpd_low_start[i] = hdmi_get_system_time_ms();
		resume_hpd_low_end[i] = resume_hpd_low_start[i];

		resume_hpd_control = 1;
		resume_hpd_low_end[i] +=hpd_compensation[i];
		newbase_hdmi_enable_hpd_with_delay(hpd_compensation[i], i);

		HDMI_INFO("[vfe_hdmi_drv_resume_init port:%d] hpd_compensation=%d ms (current_ms=%ld, hpd_guard_time=%d (resume_hpd_low_start=%d, resume_hpd_low_end=%d) \n",
			 i,hpd_compensation[i], hdmi_get_system_time_ms(), str_low_duration,
			resume_hpd_low_start[i], resume_hpd_low_end[i]);
	}
}


void vfe_hdmi_drv_resume(void)
{
	HDMI_INFO("[%s %d] starts :%d \n", __func__, __LINE__,g_resume_param.channel);
	
	if (g_resume_param.channel < HDMI_RX_PORT_MAX_NUM)
		g_hdmi_channel_connect_flag[g_resume_param.channel] = 1;
	//newbase_hdmi_hdr10_enable(_ENABLE);//Minchay@20160607-a add hdr10
}

void vfe_hdmi_drv_resume_std(void)
{
	unsigned int ch;
	unsigned char port;
	HDMI_INFO("[%s %d] starts\n", __func__, __LINE__);
	
	newbase_hdmi_init();
	
	for(ch = 0; ch < HDMI_RX_PORT_MAX_NUM; ch++)
	{
		g_hdmi_channel_connect_flag[ch] = 0;
		if (hdmi_vfe_get_hdmi_port(ch, &port)==0)
			newbase_hdmi_init_source(port);
	}

	resume_hpd_control = 1;

	newbase_hdmi_enable_hpd(EVERY_HDMI_PORT);  // force enable HPD
	g_hdmi_channel_connect_flag[g_resume_param.channel] = 1;
	newbase_hdmi_hdr10_enable(_ENABLE);//Minchay@20160607-a add hdr10
}


void vfe_hdmi_drv_suspend_no_connect_setting(void)
{
	g_resume_param.channel = HDMI_RX_PORT_MAX_NUM;//for default value can't Zero,avoid resume port is 0 case

	HDMI_INFO("[vfe_hdmi_drv_suspend_no_connect_setting] g_resume_param.channel=%d\n", g_resume_param.channel);

	if (VFE_HDMI_DRV_SERVICE_STATE != g_hdmi_drv_state)
		return;

	newbase_hdmi_uninit();
}

#endif /*CONFIG_PM*/
