/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2016
 *
 * All rights reserved.
 *
 *============================================================*/

/*======================= Description ============================
 *
 * file: 		hdmiMs.c
 *
 * author: 	The God of Thunder, Thor
 * date:
 * version: 	3.0
 *
 *============================================================*/

/*========================Header Files============================*/
#include "hdmi_common.h"

#include <rbus/smartfit_reg.h>
#include <rbus/offms_reg.h>
#include <rbus/onms_reg.h>
#include <rbus/sub_vgip_reg.h>
#include <rbus/vgip_reg.h>

#if HDMI_MEASURE_I3DDMA_SMART_FIT
#include <tvscalercontrol/scalerdrv/scalerdrv.h> //CONFIG_FORCE_RUN_I3DDMA
#include <rbus/dma_vgip_reg.h>
#endif 

#include "hdmi_vfe_config.h"
#include "cea_861.h"
#include <rbus/sys_reg_reg.h>

#ifdef BUILD_QUICK_SHOW
#include <div64.h>
#endif

/**********************************************************************************************
*
*	Marco or Definitions
*
**********************************************************************************************/
#define ANALOG_MODE_MEASUREMENT      0
#define DIGITAL_MODE_MEASUREMENT     1
#define MEASURE_FROM_TMDS_CLOCK 0          // 0: PIXEL CLOCK  1:TMDS CLOCK
//#define TM_TB_SIZE		4
#define TM_TB_SIZE		10


#define OFFMS_SUPPORT_DEMODE    0    //    MAC7P not support
#define ONMS_SUPPORT_DEMODE     0     // MAC7P not support
/**********************************************************************************************
*
*	Const Declarations
*
**********************************************************************************************/
const HDMI_ACTIVE_SPACE_TABLE_T hdmi_active_space_table[] = {
	{ "1080P FramePacking",  1, 1920,  1080*2 + 45, 1080, 45, 0 },
	{ "720P FramePacking",  1, 1280,  720*2 + 30, 720, 30, 0 },
	{ "480P FramePacking",  1, 720,  480*2 + 45, 480, 45, 0 },
	{ "576P FramePacking",  1, 720,  576*2 + 49, 576, 49, 0 },
	{ "1080I FramePacking",  0, 1920,  540*4 + 22 + 23*2, 540, 23, 22 },
	{ "576I FramePacking",  0, 720,  288*4 + 32 + 21*2, 288, 21, 32 },
	{ "480I FramePacking",  0, 720,  240*4 + 22 + 23*2, 240, 23, 22 },
	{ NULL,  1, 1920,  520, 40, 0 },
};

/**********************************************************************************************
*
*	Variables
*
**********************************************************************************************/
unsigned char no_polarity_inverse_flag = FALSE;//allow vgip to set h v polarity inverse or not


const HDMI_TIMING_TABLE_T tm_table[TM_TB_SIZE] = {
	{4096, 2160, 15, 10},	// 4k2K
	{3840, 2160, 15, 10},	// 4k2K
	{1920, 1080, 10, 5},	// 1080P
	{1920, 540, 5, 3},		// 1080i
	{1280, 720, 5, 5},		// 720P
	{720, 576, 3, 3},		// 576P
	{640, 480, 3, 3},		// 480P
	{720, 480, 3, 3},		// 480P
	{720, 288, 3, 3},		// 576i
	{720, 240, 3, 3},		// 480i
};

static unsigned int measure_not_finish_cnt = 0;
static unsigned int measure_start = 0;

/**********************************************************************************************
*
*	Funtion Declarations
*
**********************************************************************************************/
void lib_hdmims_offms_crt_on(void);
void lib_hdmims_offms_crt_reset(void);
unsigned char lib_hdmims_offms_start(void);
unsigned char lib_hdmims_offms_is_measure_start_pending(void);
unsigned char lib_hdmims_onms1_start(void);
unsigned char lib_hdmims_onms3_start(void);
unsigned char lib_hdmi_skiptiming_condition(HDMI_TIMING_T *timing);

#ifdef CONFIG_SUPPORT_SCALER
  // exported in scalerdrv/mode.c   : off line measure is a shared soure, so we need the semaphore to prevent race condition
  extern struct semaphore* get_offline_semaphore(void);
  #define _lock_offline_semaphore()           down(get_offline_semaphore())
  #define _unlock_offline_semaphore()         up(get_offline_semaphore())
#else
  // to fix compiler error of offline measure semaphore.
  // TODO: waiting for Scaler fix the compiler error
  #define _lock_offline_semaphore()
  #define _unlock_offline_semaphore()
#endif

/**********************************************************************************************
*
*	Function Body
*
**********************************************************************************************/


void set_no_polarity_inverse(unsigned char TorF)
{//allow vgip to set h v polarity inverse or not
	no_polarity_inverse_flag = TorF;
}

unsigned char lib_hdmims_active_correction(unsigned int *h_active, unsigned int *v_active)
{
	unsigned char i;
	unsigned char is_correct = FALSE;
	for (i=0; i<TM_TB_SIZE; i++)
	{
		if (HDMI_ABS(*h_active, tm_table[i].h_act) < tm_table[i].h_tol &&
			HDMI_ABS(*v_active, tm_table[i].v_act) < tm_table[i].v_tol)
			{

			if (*h_active != tm_table[i].h_act || *v_active != tm_table[i].v_act)
			{
				HDMI_EMG("[HDMI][OFMS] Corrected H/V=(%d,%d) to (%d,%d)\n",
					*h_active, *v_active, tm_table[i].h_act, tm_table[i].v_act);
			}
			*h_active = tm_table[i].h_act;
			*v_active = tm_table[i].v_act;
			is_correct = TRUE;
		}
	}

	//Correct other timing which is not at tm_table
	if ((*h_active)%10 == 9)
	{
		HDMI_OFMS_WARN("Corrected H ++ Original=(%d)\n",*h_active);
		(*h_active)++;
		is_correct = TRUE;
	}
	else if ((*h_active)%10 == 1)
	{
		HDMI_OFMS_WARN("Corrected H -- Original=(%d)\n",*h_active);
		(*h_active)--;
		is_correct = TRUE;
	}

	if ((*v_active)%10 == 9)
	{
		HDMI_OFMS_WARN("Corrected V++ Original=(%d)\n",*v_active);
		(*v_active)++;
		is_correct = TRUE;
	}
	else if ((*v_active)%10 == 1)
	{
		HDMI_OFMS_WARN("Corrected V-- Original=(%d)\n",*v_active);
		(*v_active)--;
		is_correct = TRUE;
	}

	return is_correct;
}

unsigned char lib_hdmims_offline_measure(HDMI_TIMING_T *tm, unsigned char frl_mode)
{
	unsigned int raw_h_sync_width = 0;
	unsigned int raw_v_sync_width = 0;

	hdmi_mask(OFFMS_SP_MS3Ctrl_reg,
		~(OFFMS_SP_MS3Ctrl_off_meas_source_mask | OFFMS_SP_MS3Ctrl_off_clk_sel_mask
		| OFFMS_SP_MS3Ctrl_off_meas_to_sel_mask | OFFMS_SP_MS3Ctrl_off_sycms_mode_mask
		| OFFMS_SP_MS3Ctrl_off_popup_meas_mask | OFFMS_SP_MS3Ctrl_off_start_ms_mask),
		OFFMS_SP_MS3Ctrl_off_meas_source(8) | OFFMS_SP_MS3Ctrl_off_clk_sel(1)
		| OFFMS_SP_MS3Ctrl_off_meas_to_sel(1) | OFFMS_SP_MS3Ctrl_off_sycms_mode(ANALOG_MODE_MEASUREMENT)
		|OFFMS_SP_MS3Ctrl_get_off_popup_meas(0)|OFFMS_SP_MS3Ctrl_get_off_start_ms(0));

	if (!lib_hdmims_offms_start()) {
		HDMI_OFMS_WARN("Hdmi_OffLineMeasure is not finish.\n");
		return FALSE;
	}

	tm->h_total = OFFMS_SP_MS3Rst1_get_off_hs_period_out_h12b(hdmi_in(OFFMS_SP_MS3Rst1_reg))+1;
	tm->v_total = OFFMS_SP_MS3Rst0_get_off_vs_period_out(hdmi_in(OFFMS_SP_MS3Rst0_reg))+1;
	tm->h_act_sta = OFFMS_MES_RESULT_HSTA_END_get_off_hx_sta(hdmi_in(OFFMS_MES_RESULT_HSTA_END_reg));
	tm->v_act_sta = OFFMS_MES_RESULT_VSTA_END_get_off_vx_sta(hdmi_in(OFFMS_MES_RESULT_VSTA_END_reg));
	tm->h_act_len = OFFMS_MES_RESULT_HSTA_END_get_off_hx_end(hdmi_in(OFFMS_MES_RESULT_HSTA_END_reg))-tm->h_act_sta;
	tm->v_act_len = OFFMS_MES_RESULT_VSTA_END_get_off_vx_end(hdmi_in(OFFMS_MES_RESULT_VSTA_END_reg))-tm->v_act_sta+1;
	tm->polarity = (OFFMS_SP_MS3Stus_get_off_vs_pol_out(hdmi_in(OFFMS_SP_MS3Stus_reg))<<1)|
			OFFMS_SP_MS3Stus_get_off_hs_pol_out(hdmi_in(OFFMS_SP_MS3Stus_reg));

	tm->h_act_sta += 1;//willychou modify it for OFFMS_START_CORRECTION
	tm->v_act_sta -= 1;//willychou modify it for OFFMS_START_CORRECTION

	raw_h_sync_width = OFFMS_SP_MS3Rst2_get_off_hs_high_out(hdmi_in(OFFMS_SP_MS3Rst2_reg))+1;
	raw_v_sync_width = OFFMS_SP_MS3Rst2_get_off_vs_high_out(hdmi_in(OFFMS_SP_MS3Rst2_reg))+1;

	if(0 == (tm->polarity & _BIT0))
	{//Hsync negative polarity
		if(tm->h_total >= raw_h_sync_width)
		{
			tm->h_act_sta += (tm->h_total - raw_h_sync_width);
			tm->IHSyncPulseCount = (tm->h_total - raw_h_sync_width);
		}
		else
		{
			HDMI_OFMS_WARN("Warning h_total=%d < raw_h_sync_width=%d\n", tm->h_total, raw_h_sync_width);
		}
	}
	else
	{// Hsync positive polarity
		tm->IHSyncPulseCount = raw_h_sync_width;
	}

	if(0 == (tm->polarity & _BIT1))
	{//Vsync negative polarity
		if(tm->v_total >= raw_v_sync_width)
		{
			tm->v_act_sta += (tm->v_total - raw_v_sync_width);
			tm->IVSyncPulseCount = (tm->v_total - raw_v_sync_width);
		}
		else
		{
			HDMI_OFMS_WARN("Warning v_total=%d < raw_v_sync_width=%d\n", tm->v_total, raw_v_sync_width);
		}
	}
	else
	{// Vsync positive polarity
		tm->IVSyncPulseCount = raw_v_sync_width;
	}

#if MEASURE_FROM_TMDS_CLOCK
	if (tm->pixel_repeat > 0) {
		tm->h_act_sta /= (tm->pixel_repeat+1);
		tm->h_total /= (tm->pixel_repeat+1);
		tm->h_act_len /= (tm->pixel_repeat+1);
	}

	if (COLOR_YUV420 == tm->colorspace) {
		tm->h_act_sta <<= 1;
		tm->h_total <<= 1;
		tm->h_act_len <<= 1;
	}

	if (HDMI_COLOR_DEPTH_10B == tm->colordepth) {
		// value / 1.25
		tm->h_act_sta = ((tm->h_act_sta*4)+3)/5;
		tm->h_total = ((tm->h_total*4)+3)/5;
		tm->h_act_len = ((tm->h_act_len*4)+3)/5;

	} else if (HDMI_COLOR_DEPTH_12B == tm->colordepth) {
		// value / 1.5
		tm->h_act_sta = ((tm->h_act_sta*2)+2)/3;
		tm->h_total = ((tm->h_total*2)+2)/3;
		tm->h_act_len = ((tm->h_act_len*2)+2)/3;

	} else if (HDMI_COLOR_DEPTH_16B == tm->colordepth) {
		// value / 2
		tm->h_act_sta = tm->h_act_sta>>1;
		tm->h_total = tm->h_total>>1;
		tm->h_act_len = tm->h_act_len>>1;

	}
#endif

	if(frl_mode!=MODE_TMDS) 
	{//H5X 4 pixel mode
		HDMI_INFO("[OFMS]H5X 4 Pixel mode!Hx4\n");
		tm->h_act_sta <<= 2;
		tm->h_total <<= 2;
		tm->h_act_len <<= 2;
	}

	if((frl_mode!=MODE_TMDS) && (COLOR_YUV420 == tm->colorspace))
	{
		HDMI_OFMS_WARN("YUV420, Hx2!\n");
		tm->h_act_sta <<= 1;
		tm->h_total <<= 1;
		tm->h_act_len <<= 1;
	}

	lib_hdmims_active_correction(&(tm->h_act_len), &(tm->v_act_len));

	HDMI_OFMS_WARN("[OFMS] IHTotal: %d\n", tm->h_total);
	HDMI_OFMS_WARN("[OFMS] IVTotal: %d\n", tm->v_total);
	HDMI_OFMS_WARN("[OFMS] IHAct: %d\n", tm->h_act_len);
	HDMI_OFMS_WARN("[OFMS] IVAct: %d\n", tm->v_act_len);
	HDMI_OFMS_WARN("[OFMS] IHStr: %d\n", tm->h_act_sta);
	HDMI_OFMS_WARN("[OFMS] IVStr: %d\n", tm->v_act_sta);
	HDMI_OFMS_WARN("[OFMS] Polarity: %d\n", tm->polarity);
	HDMI_OFMS_WARN("[OFMS] IHsyncWidth %d\n", tm->IHSyncPulseCount);
	HDMI_OFMS_WARN("[OFMS] IVsyncWidth %d\n", tm->IVSyncPulseCount);

	return TRUE;

}

unsigned char lib_hdmims_offline_measure_demode(HDMI_TIMING_T *tm, unsigned char frl_mode)
{
#if !OFFMS_SUPPORT_DEMODE
	return FALSE;  //Mac7p don't support demode
#else
	hdmi_mask(OFFMS_SP_MS3Ctrl_reg,
		~(OFFMS_SP_MS3Ctrl_off_meas_source_mask | OFFMS_SP_MS3Ctrl_off_clk_sel_mask
		| OFFMS_SP_MS3Ctrl_off_meas_to_sel_mask | OFFMS_SP_MS3Ctrl_off_sycms_mode_mask
		| OFFMS_SP_MS3Ctrl_off_popup_meas_mask | OFFMS_SP_MS3Ctrl_off_start_ms_mask),
		OFFMS_SP_MS3Ctrl_off_meas_source(8) | OFFMS_SP_MS3Ctrl_off_clk_sel(1)
		| OFFMS_SP_MS3Ctrl_off_meas_to_sel(1) | OFFMS_SP_MS3Ctrl_off_sycms_mode(ANALOG_MODE_MEASUREMENT)
		|OFFMS_SP_MS3Ctrl_get_off_popup_meas(0)|OFFMS_SP_MS3Ctrl_get_off_start_ms(0));

	if (!lib_hdmims_offms_start()) {
		HDMI_OFMS_WARN("DE Mode, Hdmi_OffLineMeasure is not finish.\n");
		return FALSE;
	}

	tm->h_total = OFFMS_Demode_Hs_period_get_de_hs_period(hdmi_in(OFFMS_Demode_Hs_period_reg));
	//tm->v_total = OFFMS_SP_MS3Rst0_get_off_vs_period_out(hdmi_in(OFFMS_SP_MS3Rst0_reg))+1;
	//tm->h_act_sta = OFFMS_Demode_VsHs_Act_get_de_hs_act(hdmi_in(OFFMS_Demode_VsHs_Act_reg));
	tm->v_act_sta = OFFMS_Demode_Vs_STA_get_de_vs_sta(hdmi_in(OFFMS_Demode_Vs_STA_reg));
	tm->h_act_len =  OFFMS_Demode_VsHs_Act_get_de_hs_act(hdmi_in(OFFMS_Demode_VsHs_Act_reg));
	tm->v_act_len = OFFMS_Demode_VsHs_Act_get_de_vs_act(hdmi_in(OFFMS_Demode_VsHs_Act_reg));
	//tm->polarity = (OFFMS_SP_MS3Stus_get_off_vs_pol_out(hdmi_in(OFFMS_SP_MS3Stus_reg))<<1)|OFFMS_SP_MS3Stus_get_off_hs_pol_out(hdmi_in(OFFMS_SP_MS3Stus_reg));

	//tm->IVSyncPulseCount = OFFMS_SP_MS3Rst2_get_off_vs_high_out(hdmi_in(OFFMS_SP_MS3Rst2_reg))+1;
	//tm->IHSyncPulseCount = OFFMS_SP_MS3Rst2_get_off_hs_high_out(hdmi_in(OFFMS_SP_MS3Rst2_reg))+1;



       if(frl_mode!=MODE_TMDS) 
       {//H5X 4 pixel mode
		HDMI_OFMS_WARN("H5X 4 Pixel mode!Hx4\n");
		tm->h_total <<= 2;
		tm->h_act_len <<= 2;
		if (tm->pixel_repeat > 0)
		{
			HDMI_OFMS_WARN("Pixel Repeat =%d !H/(pixel_repeat+1)\n", tm->pixel_repeat);
			tm->h_total /= (tm->pixel_repeat+1);
			tm->h_act_len /= (tm->pixel_repeat+1);
		}
	}

	if((frl_mode!=MODE_TMDS) && (COLOR_YUV420 == tm->colorspace))
	{
		HDMI_OFMS_WARN("YUV420, Hx2!\n");
		tm->h_total <<= 1;
		tm->h_act_len <<= 1;
	}

	if(GET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_SUPPORT_DSC))
	{//Only for testing
		HDMI_INFO("[OFMS]DSC, Hx2!\n");
		tm->h_total <<= 1;
		tm->h_act_len <<= 1;
	}

	HDMI_INFO("[OFMS] IHTotal: %d\n", tm->h_total);
	HDMI_INFO("[OFMS] IHAct: %d\n", tm->h_act_len);
	HDMI_INFO("[OFMS] IHAct_Frac: %d\n", OFFMS_Demode_VsHs_Act_get_de_hs_act_fraction(hdmi_in(OFFMS_Demode_VsHs_Act_reg)));

	HDMI_INFO("[OFMS] IVAct: %d\n", tm->v_act_len);
	HDMI_INFO("[OFMS] IVStr: %d\n", tm->v_act_sta);
	HDMI_INFO("[OFMS] IVStr_End: %d\n", OFFMS_Demode_Vs_END_get_de_vs_end(hdmi_in(OFFMS_Demode_Vs_END_reg)));

	return TRUE;
#endif
}

unsigned char lib_hdmims_offline_measure_native(HDMI_TIMING_T *tm, unsigned char frl_mode)
{
	unsigned int raw_h_sync_width = 0;
	unsigned int raw_v_sync_width = 0;

	hdmi_mask(OFFMS_SP_MS3Ctrl_reg,
		~(OFFMS_SP_MS3Ctrl_off_meas_source_mask | OFFMS_SP_MS3Ctrl_off_clk_sel_mask
		| OFFMS_SP_MS3Ctrl_off_meas_to_sel_mask | OFFMS_SP_MS3Ctrl_off_sycms_mode_mask
		| OFFMS_SP_MS3Ctrl_off_popup_meas_mask | OFFMS_SP_MS3Ctrl_off_start_ms_mask),
		OFFMS_SP_MS3Ctrl_off_meas_source(8) | OFFMS_SP_MS3Ctrl_off_clk_sel(1)
		| OFFMS_SP_MS3Ctrl_off_meas_to_sel(1) | OFFMS_SP_MS3Ctrl_off_sycms_mode(ANALOG_MODE_MEASUREMENT)
		|OFFMS_SP_MS3Ctrl_get_off_popup_meas(0)|OFFMS_SP_MS3Ctrl_get_off_start_ms(0));

	if (!lib_hdmims_offms_start()) {
		HDMI_OFMS_WARN("Hdmi_OffLineMeasure is not finish.\n");
		return FALSE;
	}

	tm->h_total = OFFMS_SP_MS3Rst1_get_off_hs_period_out_h12b(hdmi_in(OFFMS_SP_MS3Rst1_reg))+1;
	tm->v_total = OFFMS_SP_MS3Rst0_get_off_vs_period_out(hdmi_in(OFFMS_SP_MS3Rst0_reg))+1;
	tm->h_act_sta = OFFMS_MES_RESULT_HSTA_END_get_off_hx_sta(hdmi_in(OFFMS_MES_RESULT_HSTA_END_reg));
	tm->v_act_sta = OFFMS_MES_RESULT_VSTA_END_get_off_vx_sta(hdmi_in(OFFMS_MES_RESULT_VSTA_END_reg));
	tm->h_act_len = OFFMS_MES_RESULT_HSTA_END_get_off_hx_end(hdmi_in(OFFMS_MES_RESULT_HSTA_END_reg))-tm->h_act_sta;
	tm->v_act_len = OFFMS_MES_RESULT_VSTA_END_get_off_vx_end(hdmi_in(OFFMS_MES_RESULT_VSTA_END_reg))-tm->v_act_sta+1;
	tm->polarity = (OFFMS_SP_MS3Stus_get_off_vs_pol_out(hdmi_in(OFFMS_SP_MS3Stus_reg))<<1)|
			OFFMS_SP_MS3Stus_get_off_hs_pol_out(hdmi_in(OFFMS_SP_MS3Stus_reg));

	tm->h_act_sta += 1;//willychou modify it for OFFMS_START_CORRECTION
	tm->v_act_sta -= 1;//willychou modify it for OFFMS_START_CORRECTION

	raw_h_sync_width = OFFMS_SP_MS3Rst2_get_off_hs_high_out(hdmi_in(OFFMS_SP_MS3Rst2_reg))+1;
	raw_v_sync_width = OFFMS_SP_MS3Rst2_get_off_vs_high_out(hdmi_in(OFFMS_SP_MS3Rst2_reg))+1;
	tm->IHSyncPulseCount = raw_h_sync_width;
	tm->IVSyncPulseCount = raw_v_sync_width;

	return TRUE;

}


unsigned char lib_hdmims_measure(HDMI_TIMING_T *tm, unsigned char frl_mode)
{
	unsigned char ret = TRUE;

	ret &= lib_hdmims_offline_measure(tm, frl_mode);
	if(ret == TRUE)
		return lib_hdmims_calc_freq(tm);
	else
		return ret;
}

void _hdmi_ms_dump_measure_status(const char* msg)
{
	HDMI_OFMS_WARN("%s : ofms status (MS3Ctrl:%08x, MS3Stus=%08x), measure_not_finish_cnt=%d\n",
		msg, hdmi_in(OFFMS_SP_MS3Ctrl_reg), hdmi_in(OFFMS_SP_MS3Stus_reg), measure_not_finish_cnt);
}

unsigned char newbase_hdmims_measure_video(unsigned char port, HDMI_TIMING_T *tm, unsigned char frl_mode)
{
	unsigned char ms_done = 0;
	HDMI_TIMING_T tm_cmp;
	unsigned char is_vrr_mode = tm->run_vrr;
	unsigned char is_get_avi = 0;
	HDMI_AVI_T avi_info;

	memcpy(&tm_cmp, tm, sizeof(HDMI_TIMING_T));

	newbase_hdmims_measure_init();

	HDMI_OFMS_WARN("[newbase_hdmims_measure_video port:%d] MAC hsync(HO)=%d  VRR_Mode=%d, measure_not_finish_cnt = %d\n",
		port, lib_hdmi_get_mac_h_sync(port), is_vrr_mode, measure_not_finish_cnt);

	if(is_vrr_mode==TRUE)
	{
		unsigned char wait_avi_count = 5;

		HDMI_OFMS_WARN("VRR timing, port=%d, waiting for AVI info : wait_avi_count=%d\n", port, wait_avi_count);

		while(wait_avi_count)
		{
			// wait AVI info if necessary
			is_get_avi = newbase_hdmi_get_avi_infoframe(port, &avi_info);

			if ((is_get_avi != 0) || wait_avi_count==0)
				wait_avi_count= 0;   // stop wait avi
			else
			{
				wait_avi_count--;
				msleep(20);
			}
			HDMI_OFMS_WARN("VRR timing, port=%d, waiting for AVI info : wait_avi_count=%d...\n", port, wait_avi_count);
		}
		if((is_get_avi != 0) && (avi_info.VIC !=0))
		{
			HDMI_OFMS_WARN("Measure mode: VRR1 with VIC, port=%d, Use AVI VIC=%d\n", port, avi_info.VIC);
			return newbase_hdmi_vic_correction(port, tm, TRUE);
		}
	}

	_lock_offline_semaphore();
	ms_done = lib_hdmims_measure(tm, frl_mode);
	_unlock_offline_semaphore();

	if (!ms_done)			return FALSE;
	if (tm->h_act_len > tm->h_total)	return FALSE;
	if (tm->v_act_len > tm->v_total)	return FALSE;
	if (tm->h_act_len < 300)		return FALSE;
	if (tm->v_act_len < 240)		return FALSE;
	if((0==GET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_FORCE_VIC_MEASURE))
		&&(TRUE == lib_hdmi_skiptiming_condition(tm)))
		return FALSE;

	_lock_offline_semaphore();
	ms_done = lib_hdmims_measure(&tm_cmp, frl_mode);
	_unlock_offline_semaphore();

	if (!ms_done)					return FALSE;
	if (tm->h_total != tm_cmp.h_total)			return FALSE;
	if (HDMI_ABS(tm->v_total, tm_cmp.v_total) > 5)	return FALSE;
	if (tm->h_act_len != tm_cmp.h_act_len)		return FALSE;
	if (tm->v_act_len != tm_cmp.v_act_len)		return FALSE;
	if (HDMI_ABS(tm->h_freq, tm_cmp.h_freq) > 5)	return FALSE;
	if (HDMI_ABS(tm->v_freq, tm_cmp.v_freq) > 5)	return FALSE;

	return TRUE;
}

void newbase_hdmims_measure_init(void)
{
	lib_hdmims_offms_crt_on();  // enable CRT of OFMS
	HDMI_OFMS_WARN("newbase_hdmims_measure_init!\n");
}

void newbase_hdmims_measure_abort(void)
{
	if (measure_start)
	{
		measure_start = 0;
		HDMI_OFMS_WARN("abort timing measure!\n");
	}
}

unsigned char lib_hdmims_offms_start(void)
{
	unsigned int t1;
	offms_sp_ms3ctrl_RBUS ofms_ms3ctrl_reg;

	//_hdmi_ms_dump_measure_status("before start measure");

	ofms_ms3ctrl_reg.regValue = hdmi_in(OFFMS_SP_MS3Ctrl_reg);

	ofms_ms3ctrl_reg.off_start_ms = 1;
	hdmi_out(OFFMS_SP_MS3Ctrl_reg, ofms_ms3ctrl_reg.regValue);

	_hdmi_ms_dump_measure_status("start measure");

	measure_start = 1;

	t1 = 100;
	do {
		t1 --;
		ofms_ms3ctrl_reg.regValue = hdmi_in(OFFMS_SP_MS3Ctrl_reg);
		if ( !ofms_ms3ctrl_reg.off_start_ms )  break;
		msleep(2);//Minchay@20151029-m speed up performence
	} while (t1 > 0 && measure_start);

#ifdef HDMI_OFFLINE_MEASURE_FIX
	if (t1 == 0) {
		HDMI_OFMS_WARN("POP not finish \n");
	}
#else //#ifdef HDMI_OFFLINE_MEASURE_FIX
	if ( t1==0 || !measure_start)
	{
		if (measure_start)
		{
			_hdmi_ms_dump_measure_status("user meassure abort");
		}
		else
		{
			measure_not_finish_cnt++;
			_hdmi_ms_dump_measure_status("wait ofms timeout, reset ofms");
			lib_hdmims_offms_crt_reset();
		}
		return FALSE;
	}
#endif //#ifdef HDMI_OFFLINE_MEASURE_FIX

	ofms_ms3ctrl_reg.off_popup_meas = 1;
	hdmi_out(OFFMS_SP_MS3Ctrl_reg, ofms_ms3ctrl_reg.regValue);

	t1 = 100;
	do {
		t1 --;
		ofms_ms3ctrl_reg.regValue = hdmi_in(OFFMS_SP_MS3Ctrl_reg);
		if ( !ofms_ms3ctrl_reg.off_popup_meas )  break;
		msleep(2);//Minchay@20151029-m speed up performence
	} while (t1 > 0 && measure_start);

	if (t1 == 0 || !measure_start)
	{
		if (!measure_start)
		{
			_hdmi_ms_dump_measure_status("pop up ofms user meassure abort");
		}
		else
		{
			measure_not_finish_cnt++;
			_hdmi_ms_dump_measure_status("pop up ofms timeout, reset ofms");
			lib_hdmims_offms_crt_reset();  // reset
		}
		return FALSE;
	}

	measure_not_finish_cnt = 0;
	measure_start = 0;
	_hdmi_ms_dump_measure_status("measure complete");
	return TRUE;
}

void lib_hdmims_online1_set_ms_src_sel(unsigned char ms_src_sel)
{//0: MAIN VGIP input, 1: output
	hdmi_mask(ONMS_onms1_ctrl_reg, ~ONMS_onms1_ctrl_on1_ms_src_sel_mask, ONMS_onms1_ctrl_on1_ms_src_sel((ms_src_sel)?1:0));
}

unsigned char lib_hdmims_online1_get_ms_src_sel(void)
{
	return ONMS_onms1_ctrl_get_on1_ms_src_sel(hdmi_in(ONMS_onms1_ctrl_reg));
}

unsigned char lib_hdmims_online1_measure(HDMI_TIMING_T *tm)
{
	hdmi_mask(ONMS_onms1_ctrl_reg,
		~(ONMS_onms1_ctrl_1_on1_vtotal_of_th_mask
		| ONMS_onms1_ctrl_on1_htotal_of_th_mask | ONMS_onms1_ctrl_on1_meas_to_sel_mask 
		| ONMS_onms1_ctrl_on1_sycms_clk_mask|ONMS_onms1_ctrl_on1_ms_conti_mask
		|ONMS_onms1_ctrl_on1_start_ms_mask),
		ONMS_onms1_ctrl_1_on1_vtotal_of_th(1)
		| ONMS_onms1_ctrl_on1_htotal_of_th(1) | ONMS_onms1_ctrl_on1_meas_to_sel(1) 
		| ONMS_onms1_ctrl_on1_sycms_clk(1) |ONMS_onms1_ctrl_on1_ms_conti(0)
		|ONMS_onms1_ctrl_on1_start_ms(0));

	if (!lib_hdmims_onms1_start()) {
		HDMI_EMG("[ONMS1] OnLineMeasure is not finish.\n");
		return FALSE;
	}

	tm->h_act_len = ONMS_onms1_hact_get_on1_hact(hdmi_in(ONMS_onms1_hact_reg));
	tm->v_act_len = ONMS_onms1_vs_period_get_on1_vtotalde(hdmi_in(ONMS_onms1_vs_period_reg));
	tm->h_total = ONMS_onms1_hs_period_get_on1_htotal_h12b(hdmi_in(ONMS_onms1_hs_period_reg));
	tm->v_total = ONMS_onms1_vs_period_get_on1_vtotal(hdmi_in(ONMS_onms1_vs_period_reg));
	tm->h_act_sta = ONMS_onms1_hact_sta_get_on1_hact_start(hdmi_in(ONMS_onms1_hact_sta_reg));
	tm->v_act_sta = ONMS_onms1_vact_sta_get_on1_vact_start(hdmi_in(ONMS_onms1_vact_sta_reg));
	tm->IHSyncPulseCount = ONMS_onms1_vshs_high_period_get_on1_hs_high_period(hdmi_in(ONMS_onms1_vshs_high_period_reg));
	tm->IVSyncPulseCount = ONMS_onms1_vshs_high_period_get_on1_vs_high_period(hdmi_in(ONMS_onms1_vshs_high_period_reg));
	HDMI_OFMS_INFO("[ONMS1] Measure sel: %d (0: VGIP INPUT, 1: VGIP OUTPUT)\n", lib_hdmims_online1_get_ms_src_sel());
	HDMI_OFMS_INFO("[ONMS1] IHTotal: %d\n", tm->h_total);
	HDMI_OFMS_INFO("[ONMS1] IVTotal: %d\n", tm->v_total);
	HDMI_OFMS_INFO("[ONMS1] IHAct: %d\n", tm->h_act_len);
	HDMI_OFMS_INFO("[ONMS1] IVAct: %d\n", tm->v_act_len);
	HDMI_OFMS_INFO("[ONMS1] IHStr: %d\n", tm->h_act_sta);
	HDMI_OFMS_INFO("[ONMS1] IVStr: %d\n", tm->v_act_sta);
	HDMI_OFMS_INFO("[ONMS1] IHsyncWidth %d\n", tm->IHSyncPulseCount);
	HDMI_OFMS_INFO("[ONMS1] IVsyncWidth %d\n", tm->IVSyncPulseCount);

	return TRUE;

}

unsigned char lib_hdmims_offms_is_measure_start_pending(void)
{
    return (hdmi_in(OFFMS_SP_MS3Ctrl_reg) & OFFMS_SP_MS3Ctrl_off_start_ms_mask) ? 1 : 0;
}

void lib_hdmims_offms_crt_on(void)
{
    // enable OFMS CRT
    if (!((hdmi_in(SYS_REG_SYS_CLKEN1_reg) & SYS_REG_SYS_CLKEN1_clken_offms_mask) &&
          (hdmi_in(SYS_REG_SYS_SRST1_reg) & SYS_REG_SYS_SRST1_rstn_offms_mask)))
    {
        HDMI_OFMS_WARN("CRT is not enabled, enable OFMS Clock\n");
        
        hdmi_out(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_offms_mask);
        udelay(5);
        hdmi_out(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_offms_mask);

        udelay(5);
        
        hdmi_out(SYS_REG_SYS_CLKEN1_reg, (SYS_REG_SYS_CLKEN1_clken_offms_mask|SYS_REG_SYS_CLKEN1_write_data_mask));
        udelay(5);

        hdmi_out(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_offms_mask);
        udelay(5);

        hdmi_out(SYS_REG_SYS_SRST1_reg, (SYS_REG_SYS_SRST1_rstn_offms_mask|SYS_REG_SYS_SRST1_write_data_mask));
        udelay(5);
        hdmi_out(SYS_REG_SYS_CLKEN1_reg, (SYS_REG_SYS_CLKEN1_clken_offms_mask|SYS_REG_SYS_CLKEN1_write_data_mask));
    }

    if ((hdmi_in(SYS_REG_SYS_CLKEN3_reg) & SYS_REG_SYS_CLKEN3_clken_dispi_mask)==0 ||
        (hdmi_in(SYS_REG_SYS_SRST3_reg) & (SYS_REG_SYS_SRST3_rstn_dispi1_mask | SYS_REG_SYS_SRST3_rstn_dispi2_mask | SYS_REG_SYS_SRST3_rstn_dispi3_mask))==0)        
    {
        HDMI_OFMS_WARN("DISPI CRT is not enabled, enable DISPPI Clock for OFMS \n");
        hdmi_out(SYS_REG_SYS_CLKEN3_reg, SYS_REG_SYS_CLKEN3_clken_dispi_mask | 
                                         SYS_REG_SYS_CLKEN3_write_data_mask);
                                         
        hdmi_out(SYS_REG_SYS_SRST3_reg,  SYS_REG_SYS_SRST3_rstn_dispi1_mask | 
                                         SYS_REG_SYS_SRST3_rstn_dispi2_mask | 
                                         SYS_REG_SYS_SRST3_rstn_dispi3_mask | 
                                         SYS_REG_SYS_CLKEN3_write_data_mask);
    }
}

void lib_hdmims_offms_crt_reset(void)
{
	HDMI_OFMS_WARN("OFFMS CRT reset !!!!\n");
	hdmi_out(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_offms_mask);   // disable clock
	udelay(5);
	hdmi_out(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_offms_mask);  // reset crt
	udelay(5);
	lib_hdmims_offms_crt_on();
}

unsigned char lib_hdmims_onms1_start(void)
{
	unsigned int t1;

	hdmi_mask(ONMS_onms1_ctrl_reg, ~ONMS_onms1_ctrl_on1_start_ms_mask, ONMS_onms1_ctrl_on1_start_ms_mask);

	t1 = 100;
	do {
		t1 --;
		if (!ONMS_onms1_ctrl_get_on1_start_ms(hdmi_in(ONMS_onms1_ctrl_reg))) break;
		msleep(2);
	} while (t1 > 0);

#if 1
	if (t1 == 0) {
		HDMI_EMG("[Warning][ONMS1] meas is not finish \n");
	}
#else
	if ( t1==0 ) return FALSE;
#endif

	hdmi_mask(ONMS_onms1_ctrl_reg, ~ONMS_onms1_ctrl_on1_popup_meas_mask, ONMS_onms1_ctrl_on1_popup_meas_mask);

	t1 = 100;
	do {
		t1 --;
		if (!ONMS_onms1_ctrl_get_on1_popup_meas(hdmi_in(ONMS_onms1_ctrl_reg))) break;
		msleep(2);
	} while (t1 > 0);

	if (t1 == 0) return FALSE;

	return TRUE;
}

void lib_hdmims_online3_set_ms_src_sel(unsigned char ms_src_sel)
{//0: DMA VGIP input, 1: output
	hdmi_mask(ONMS_onms3_ctrl_reg, ~ONMS_onms3_ctrl_on3_ms_src_sel_mask, ONMS_onms3_ctrl_on3_ms_src_sel((ms_src_sel)?1:0));
}

unsigned char lib_hdmims_online3_get_ms_src_sel(void)
{
	return ONMS_onms3_ctrl_get_on3_ms_src_sel(hdmi_in(ONMS_onms3_ctrl_reg));
}

unsigned char lib_hdmims_online3_measure(HDMI_TIMING_T *tm)
{
	hdmi_mask(ONMS_onms3_ctrl_reg,
		~(ONMS_onms3_ctrl_1_on3_vtotal_of_th_mask | ONMS_onms3_ctrl_on3_ms_src_sel_mask
		| ONMS_onms3_ctrl_on3_htotal_of_th_mask | ONMS_onms3_ctrl_on3_meas_to_sel_mask 
		| ONMS_onms3_ctrl_on3_sycms_clk_mask | ONMS_onms3_ctrl_on3_ms_conti_mask
		| ONMS_onms3_ctrl_on3_start_ms_mask),
		ONMS_onms3_ctrl_1_on3_vtotal_of_th(1) | ONMS_onms3_ctrl_on3_ms_src_sel(1)
		| ONMS_onms3_ctrl_on3_htotal_of_th(1) | ONMS_onms3_ctrl_on3_meas_to_sel(1) 
		| ONMS_onms3_ctrl_on3_sycms_clk(1) |ONMS_onms3_ctrl_on3_ms_conti(0)
		| ONMS_onms3_ctrl_on3_start_ms(0));

	if (!lib_hdmims_onms3_start()) {
		HDMI_EMG("[lib_hdmims_online3_measure] OnLineMeasure is not finish.\n");
		return FALSE;
	}

	tm->h_act_len = ONMS_onms3_hact_get_on3_hact(hdmi_in(ONMS_onms3_hact_reg));
	tm->v_act_len = ONMS_onms3_vs_period_get_on3_vtotalde(hdmi_in(ONMS_onms3_vs_period_reg));
	tm->h_total = ONMS_onms3_hs_period_get_on3_htotal_h12b(hdmi_in(ONMS_onms3_hs_period_reg));
	tm->v_total = ONMS_onms3_vs_period_get_on3_vtotal(hdmi_in(ONMS_onms3_vs_period_reg));
	tm->h_act_sta = ONMS_onms3_hact_sta_get_on3_hact_start(hdmi_in(ONMS_onms3_hact_sta_reg));
	tm->v_act_sta = ONMS_onms3_vact_sta_get_on3_vact_start(hdmi_in(ONMS_onms3_vact_sta_reg));
	tm->IHSyncPulseCount = ONMS_onms3_vshs_high_period_get_on3_hs_high_period(hdmi_in(ONMS_onms3_vshs_high_period_reg));
	tm->IVSyncPulseCount = ONMS_onms3_vshs_high_period_get_on3_vs_high_period(hdmi_in(ONMS_onms3_vshs_high_period_reg));
	HDMI_OFMS_INFO("[ONMS3] IHTotal: %d\n", tm->h_total);
	HDMI_OFMS_INFO("[ONMS3] IVTotal: %d\n", tm->v_total);
	HDMI_OFMS_INFO("[ONMS3] IHAct: %d\n", tm->h_act_len);
	HDMI_OFMS_INFO("[ONMS3] IVAct: %d\n", tm->v_act_len);
	HDMI_OFMS_INFO("[ONMS3] IHStr: %d\n", tm->h_act_sta);
	HDMI_OFMS_INFO("[ONMS3] IVStr: %d\n", tm->v_act_sta);
	HDMI_OFMS_INFO("[ONMS3] IHsyncWidth %d\n", tm->IHSyncPulseCount);
	HDMI_OFMS_INFO("[ONMS3] IVsyncWidth %d\n", tm->IVSyncPulseCount);
	return TRUE;

}

unsigned char lib_hdmims_onms3_start(void)
{
	unsigned int t1;

	hdmi_mask(ONMS_onms3_ctrl_reg, ~ONMS_onms3_ctrl_on3_start_ms_mask, ONMS_onms3_ctrl_on3_start_ms_mask);

	t1 = 100;
	do {
		t1 --;
		if (!ONMS_onms3_ctrl_get_on3_start_ms(hdmi_in(ONMS_onms3_ctrl_reg))) break;
		msleep(2);
	} while (t1 > 0);

#if 1
	if (t1 == 0) {
		HDMI_EMG("[Warning][ONMS3] meas is not finish \n");
	}
#else
	if ( t1==0 ) return FALSE;
#endif

	hdmi_mask(ONMS_onms3_ctrl_reg, ~ONMS_onms3_ctrl_on3_popup_meas_mask, ONMS_onms3_ctrl_on3_popup_meas_mask);

	t1 = 100;
	do {
		t1 --;
		if (!ONMS_onms3_ctrl_get_on3_popup_meas(hdmi_in(ONMS_onms3_ctrl_reg))) break;
		msleep(2);
	} while (t1 > 0);

	if (t1 == 0) return FALSE;

	return TRUE;
}

unsigned char lib_hdmims_online3_demode_measure(HDMI_TIMING_T *tm)
{
#if !ONMS_SUPPORT_DEMODE
	return FALSE;    //Mac7p don't support DEmode
#else
	unsigned char result = FALSE;
	unsigned long long de_vs_period = 0;
	unsigned long long temp_vfreq = 0;
	unsigned int vfreq = 0;

	hdmi_mask(ONMS_Onms3_Demode_ctrl_reg,
		~(ONMS_Onms3_Demode_ctrl_on3_demode_en_mask),  ONMS_Onms3_Demode_ctrl_on3_demode_en(1));

	if (!lib_hdmims_onms3_start()) {
		HDMI_EMG("[lib_hdmims_online3_demode_measure] OnLineMeasure is not finish.\n");
		return FALSE;
	}

	tm->h_act_len = ONMS_Onms3_Demode_Act_get_on3_de_hact(hdmi_in(ONMS_Onms3_Demode_Act_reg));
	tm->v_act_len = ONMS_Onms3_Demode_Act_get_on3_de_vact(hdmi_in(ONMS_Onms3_Demode_Act_reg));
	tm->h_total = ONMS_Onms3_Demode_Hs_period_get_on3_de_hs_period(hdmi_in(ONMS_Onms3_Demode_Hs_period_reg));
	//tm->v_total = ONMS_Onms3_Demode_Vs_period_get_on3_de_vs_period(hdmi_in(ONMS_Onms3_Demode_Vs_period_reg));
	tm->v_act_sta = ONMS_Onms3_Demode_Vs_STA_get_on3_de_vs_sta(hdmi_in(ONMS_Onms3_Demode_Vs_STA_reg));

	de_vs_period = (unsigned long long)ONMS_Onms3_Demode_Vs_period_get_on3_de_vs_period(hdmi_in(ONMS_Onms3_Demode_Vs_period_reg));
	HDMI_OFMS_INFO("[ONMS3_DE] de_vs_period=%lld \n", de_vs_period);
	if(de_vs_period!=0)
	{
		temp_vfreq = 27000000ULL * 100ULL * 10ULL;
		do_div(temp_vfreq, de_vs_period);
		vfreq = (unsigned int)temp_vfreq;

		if((vfreq - (vfreq/1000)*1000) == 999)
		{// XX.999 -> 00.000
			vfreq = vfreq/100 + 1;
		}
		else
		{
			vfreq = vfreq/100;
		}

		tm->v_freq = vfreq;
		tm->h_freq = tm->v_freq * tm->v_total / 10;

		tm->h_freq /= 100;    //(0.1KHz), For unit alignment
		result = TRUE;
	}
	
	HDMI_OFMS_INFO("[ONMS3_DE] IHTotal: %d\n", tm->h_total);
	//HDMI_OFMS_INFO("[ONMS3_DE] IVTotal: %d\n", tm->v_total);
	HDMI_OFMS_INFO("[ONMS3_DE] IHAct: %d\n", tm->h_act_len);
	HDMI_OFMS_INFO("[ONMS3_DE] IVAct: %d\n", tm->v_act_len);
	HDMI_OFMS_INFO("[ONMS3_DE] IVStr: %d, End: %d\n", tm->v_act_sta, ONMS_Onms3_Demode_Vs_END_get_on3_de_vs_end(ONMS_Onms3_Demode_Vs_END_reg));
	HDMI_OFMS_INFO("[ONMS3_DE] IHFreq: %d (0.1KHz)\n", tm->h_freq);
	HDMI_OFMS_INFO("[ONMS3_DE] IVFreq: %d (0.1Hz)\n", tm->v_freq);
	HDMI_OFMS_INFO("[ONMS3_DE] IVFreq(LL): %d (0.001Hz)\n", (unsigned int)temp_vfreq);

	return TRUE;
#endif

}

void lib_hdmims_onms_set_watchdog_int_en(HDMIMS_ONMS_CHANNEL onms_channel, unsigned int value)
{
	unsigned int onms_watchdog_addr = 0;
	unsigned int onms_int_addr = 0;
	
	switch(onms_channel)
	{
	case HDMIMS_ONMS1_MAIN:
		onms_watchdog_addr = ONMS_onms1_watchdog_en_reg;
		onms_int_addr = ONMS_onms1_interrupt_en_reg;
		break;
	case HDMIMS_ONMS3_DMA:
		onms_watchdog_addr = ONMS_onms3_watchdog_en_reg;
		onms_int_addr = ONMS_onms3_interrupt_en_reg;
		break;
	default:
		onms_watchdog_addr = ONMS_onms1_watchdog_en_reg;
		onms_int_addr = ONMS_onms1_interrupt_en_reg;

		HDMI_EMG("[lib_hdmims_onms_set_watchdog_int_en]Error!! invalid onms_channel=%d, value=%d\n", onms_channel, value);
		break;
	}
	hdmi_out(onms_watchdog_addr, value);
	hdmi_out(onms_int_addr, value);
}

unsigned int lib_hdmims_onms_get_watchdog_en(HDMIMS_ONMS_CHANNEL onms_channel)
{
	unsigned int onms_watchdog_addr = 0;
	switch(onms_channel)
	{
	case HDMIMS_ONMS1_MAIN:
		onms_watchdog_addr = ONMS_onms1_watchdog_en_reg;
		break;
	case HDMIMS_ONMS3_DMA:
		onms_watchdog_addr = ONMS_onms3_watchdog_en_reg;
		break;
	default:
		HDMI_EMG("[lib_hdmims_onms_get_watchdog_int_en]Error!! invalid onms_channel=%d\n", onms_channel);
		break;
	}

	return hdmi_in(onms_watchdog_addr);
}

unsigned int lib_hdmims_onms_get_int_en(HDMIMS_ONMS_CHANNEL onms_channel)
{
	unsigned int onms_int_addr = 0;
	switch(onms_channel)
	{
	case HDMIMS_ONMS1_MAIN:
		onms_int_addr = ONMS_onms1_interrupt_en_reg;
		break;
	case HDMIMS_ONMS3_DMA:
		onms_int_addr = ONMS_onms3_interrupt_en_reg;
		break;
	default:
		HDMI_EMG("[lib_hdmims_onms_get_int_en]Error!! invalid onms_channel=%d\n", onms_channel);
		break;
	}

	return hdmi_in(onms_int_addr);
}


unsigned int lib_hdmims_calc_vfreq(unsigned int vs_period)
{// V freq (0.001Hz)
	unsigned long long de_vs_period = (unsigned long long)vs_period;
	unsigned long long temp_vfreq = 0;
	unsigned int vfreq = 0;

	//HDMI_OFMS_WARN("[OFMS] input vs_period=%lld \n", de_vs_period);
	if(de_vs_period!=0)
	{
		temp_vfreq = 27000000ULL * 100ULL * 10ULL;
		do_div(temp_vfreq, de_vs_period);
		vfreq = (unsigned int)temp_vfreq;
	}
	else
	{
		//HDMI_OFMS_WARN("[OFMS][lib_hdmims_calc_vfreq] Calculate Fail!!Divide 0!! de_vs_period=%lld \n", de_vs_period);
		vfreq = 0;
	}
	//HDMI_OFMS_WARN("[OFMS][lib_hdmims_calc_vfreq] IVFreq: %d (0.001Hz)\n", vfreq);
	return vfreq;
}

unsigned char lib_hdmims_calc_freq(HDMI_TIMING_T *tm)
{
	unsigned int Q, D;

	if (tm->tmds_clk_b < 100 || tm->v_total < 100) 
	{
		tm->h_freq = 0;
		tm->v_freq = 0;
		return FALSE;
	}

	// implement Q = (27000000 * hdmi.b / 256 * RatioM) / RatioN
	D = (tm->pixel_repeat+1) * tm->h_total;
	Q = ((27000000UL * dpll_ratio[tm->colordepth].RatioM)) / (dpll_ratio[tm->colordepth].RatioN * 256);

	Q = (tm->tmds_clk_b * Q);
	Q =   ((Q + (D >> 1)) / D);

	tm->h_freq = (Q + 50) / 100;
	tm->v_freq = (Q * 10 + (tm->v_total>> 1)) / tm->v_total;

	if (tm->colorspace == COLOR_YUV420)
		tm->v_freq <<= 1;

	HDMI_OFMS_WARN("[OFMS] IHFreq: %d (0.1KHz)\n", tm->h_freq);
	HDMI_OFMS_WARN("[OFMS] IVFreq: %d (0.1Hz)\n", tm->v_freq);
	return TRUE;
}

#if HDMI_MEASURE_I3DDMA_SMART_FIT
void lib_hdmims_set_vgip_clk_enable(HDMI_SMT_PATH vgip_ch)
{
	vgip_vgip_chn1_ctrl_RBUS vgip_vgip_chn1_ctrl_reg;
#ifdef CONFIG_DUAL_CHANNEL
	sub_vgip_vgip_chn2_ctrl_RBUS sub_vgip_vgip_chn2_ctrl_reg;
#endif
	dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_dma_vgip_ctrl_reg;

	switch(vgip_ch)
	{
		case HDMI_SMT_PATH_MAIN_VGIP:
			// Disable SRC_Clock_Enbale bit
			vgip_vgip_chn1_ctrl_reg.regValue = hdmi_in(VGIP_VGIP_CHN1_CTRL_reg);
			vgip_vgip_chn1_ctrl_reg.ch1_in_clk_en = _DISABLE; // clear previous setting
			hdmi_out(VGIP_VGIP_CHN1_CTRL_reg, vgip_vgip_chn1_ctrl_reg.regValue);

			// 2nd determine the input source
			vgip_vgip_chn1_ctrl_reg.regValue = hdmi_in(VGIP_VGIP_CHN1_CTRL_reg);
			vgip_vgip_chn1_ctrl_reg.ch1_in_sel = 0; //HDMI
			vgip_vgip_chn1_ctrl_reg.ch1_digital_mode = 1; //digital mode
			hdmi_out(VGIP_VGIP_CHN1_CTRL_reg, vgip_vgip_chn1_ctrl_reg.regValue);

			// 3rd enable SRC_Clock_Enbale bit
			vgip_vgip_chn1_ctrl_reg.regValue = hdmi_in(VGIP_VGIP_CHN1_CTRL_reg);
			vgip_vgip_chn1_ctrl_reg.ch1_in_clk_en = _ENABLE; //enable clock
			hdmi_out(VGIP_VGIP_CHN1_CTRL_reg, vgip_vgip_chn1_ctrl_reg.regValue);

			vgip_vgip_chn1_ctrl_reg.regValue = hdmi_in(VGIP_VGIP_CHN1_CTRL_reg);
			vgip_vgip_chn1_ctrl_reg.ch1_ivrun = _ENABLE; //Input VGIP run enable
			hdmi_out(VGIP_VGIP_CHN1_CTRL_reg, vgip_vgip_chn1_ctrl_reg.regValue);

			HDMI_PRINTF("[HDMI] measure use VGIP1\n");
		break;

#ifdef CONFIG_DUAL_CHANNEL
		case HDMI_SMT_PATH_SUB_VGIP:
			// Disable SRC_Clock_Enbale bit
			sub_vgip_vgip_chn2_ctrl_reg.regValue = hdmi_in(SUB_VGIP_VGIP_CHN2_CTRL_reg);
			sub_vgip_vgip_chn2_ctrl_reg.ch2_in_clk_en = _DISABLE;
			hdmi_out(SUB_VGIP_VGIP_CHN2_CTRL_reg, sub_vgip_vgip_chn2_ctrl_reg.regValue);

			// 2nd determine the input source
			sub_vgip_vgip_chn2_ctrl_reg.regValue = hdmi_in(SUB_VGIP_VGIP_CHN2_CTRL_reg);
			sub_vgip_vgip_chn2_ctrl_reg.ch2_in_sel = 0; //HDMI
			sub_vgip_vgip_chn2_ctrl_reg.ch2_digital_mode = 1; //digital mode
			hdmi_out(SUB_VGIP_VGIP_CHN2_CTRL_reg, sub_vgip_vgip_chn2_ctrl_reg.regValue);

			// 3rd enable SRC_Clock_Enbale bit
			sub_vgip_vgip_chn2_ctrl_reg.regValue = hdmi_in(SUB_VGIP_VGIP_CHN2_CTRL_reg);
			sub_vgip_vgip_chn2_ctrl_reg.ch2_in_clk_en = _ENABLE;
			hdmi_out(SUB_VGIP_VGIP_CHN2_CTRL_reg, sub_vgip_vgip_chn2_ctrl_reg.regValue);

			sub_vgip_vgip_chn2_ctrl_reg.regValue = hdmi_in(SUB_VGIP_VGIP_CHN2_CTRL_reg);
			sub_vgip_vgip_chn2_ctrl_reg.ch2_ivrun = _ENABLE; //Input VGIP run enable
			hdmi_out(SUB_VGIP_VGIP_CHN2_CTRL_reg, sub_vgip_vgip_chn2_ctrl_reg.regValue);

			HDMI_PRINTF("[HDMI] measure use VGIP2\n");
		break;
#endif

		case HDMI_SMT_PATH_DMA_VGIP:
			// Disable SRC_Clock_Enbale bit
			dma_vgip_dma_vgip_ctrl_reg.regValue = hdmi_in(DMA_VGIP_DMA_VGIP_CTRL_reg);
			dma_vgip_dma_vgip_ctrl_reg.dma_in_clk_en = _DISABLE;
			hdmi_out(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_dma_vgip_ctrl_reg.regValue);

			// 2nd determine the input source
			dma_vgip_dma_vgip_ctrl_reg.regValue = hdmi_in(DMA_VGIP_DMA_VGIP_CTRL_reg);
			dma_vgip_dma_vgip_ctrl_reg.dma_in_sel = 0; //HDMI
			dma_vgip_dma_vgip_ctrl_reg.dma_digital_mode = 1; //digital mode
			hdmi_out(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_dma_vgip_ctrl_reg.regValue);

			// 3rd enable SRC_Clock_Enbale bit
			dma_vgip_dma_vgip_ctrl_reg.regValue = hdmi_in(DMA_VGIP_DMA_VGIP_CTRL_reg);
			dma_vgip_dma_vgip_ctrl_reg.dma_in_clk_en = _ENABLE;
			hdmi_out(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_dma_vgip_ctrl_reg.regValue);

			dma_vgip_dma_vgip_ctrl_reg.regValue = hdmi_in(DMA_VGIP_DMA_VGIP_CTRL_reg);
			dma_vgip_dma_vgip_ctrl_reg.dma_ivrun = _ENABLE; //Input VGIP run enable
			hdmi_out(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_dma_vgip_ctrl_reg.regValue);

			HDMI_PRINTF("[HDMI] measure use DMA-VGIP\n");
		break;

		default:
			HDMI_EMG("[HDMI] measure VGIP Err=%d\n", vgip_ch);
		break;
	}
}
#else //#if HDMI_MEASURE_I3DDMA_SMART_FIT
void lib_hdmims_set_vgip_clk_enable(unsigned char vgip_ch)
{

	if (0 == vgip_ch) { //vgip 1
		// Disable SRC_Clock_Enbale bit
		hdmi_mask(VGIP_VGIP_CHN1_CTRL_reg, ~(VGIP_VGIP_CHN1_CTRL_ch1_in_clk_en_mask), 0); // clear previous setting
		// 2nd determine the input source
		hdmi_mask(VGIP_VGIP_CHN1_CTRL_reg, ~(VGIP_VGIP_CHN1_CTRL_ch1_in_sel_mask|VGIP_VGIP_CHN1_CTRL_ch1_digital_mode_mask), VGIP_VGIP_CHN1_CTRL_ch1_digital_mode_mask); //set analog mode
		// 3rd enable SRC_Clock_Enbale bit
		hdmi_mask(VGIP_VGIP_CHN1_CTRL_reg, ~(VGIP_VGIP_CHN1_CTRL_ch1_in_clk_en_mask), VGIP_VGIP_CHN1_CTRL_ch1_in_clk_en_mask); // enable clock
		hdmi_mask(VGIP_VGIP_CHN1_CTRL_reg, ~(VGIP_VGIP_CHN1_CTRL_ch1_ivrun_mask), VGIP_VGIP_CHN1_CTRL_ch1_ivrun_mask); // enable clock
		HDMI_PRINTF("[HDMI] measure use VGIP1\n");

	} else {	// if ( _CHANNEL2 == channel )
		// Disable SRC_Clock_Enbale bit
		hdmi_mask(SUB_VGIP_VGIP_CHN2_CTRL_reg,~(SUB_VGIP_VGIP_CHN2_CTRL_ch2_in_clk_en_mask),0);
		// 2nd determine the input source
		hdmi_mask(SUB_VGIP_VGIP_CHN2_CTRL_reg, ~(SUB_VGIP_VGIP_CHN2_CTRL_ch2_in_sel_mask|SUB_VGIP_VGIP_CHN2_CTRL_ch2_digital_mode_mask),SUB_VGIP_VGIP_CHN2_CTRL_ch2_digital_mode_mask);
		// 3rd enable SRC_Clock_Enbale bit
		hdmi_mask(SUB_VGIP_VGIP_CHN2_CTRL_reg,~(SUB_VGIP_VGIP_CHN2_CTRL_ch2_in_clk_en_mask),SUB_VGIP_VGIP_CHN2_CTRL_ch2_in_clk_en_mask);
		hdmi_mask(SUB_VGIP_VGIP_CHN2_CTRL_reg,~(SUB_VGIP_VGIP_CHN2_CTRL_ch2_ivrun_mask),SUB_VGIP_VGIP_CHN2_CTRL_ch2_ivrun_mask);
		HDMI_PRINTF("[HDMI] measure use VGIP2\n");
	}

}
#endif //#if HDMI_MEASURE_I3DDMA_SMART_FIT

#if HDMI_MEASURE_I3DDMA_SMART_FIT
unsigned char lib_hdmims_get_hvstart(HDMI_TIMING_T *tm, unsigned char vsc_ch)
{
// pre-condition:
//
// 0xb800d098[3] = 1

	int timeoutcnt;
	HDMI_SMT_PATH vgip_ch = HDMI_SMT_PATH_MAIN_VGIP;
	vgip_vgip_chn1_ctrl_RBUS vgip_vgip_chn1_ctrl_reg;
#ifdef CONFIG_DUAL_CHANNEL
	sub_vgip_vgip_chn2_ctrl_RBUS sub_vgip_vgip_chn2_ctrl_reg;
	sub_vgip_vgip_chn2_act_hsta_width_RBUS sub_vgip_vgip_chn2_act_hsta_width_reg;
	sub_vgip_vgip_chn2_act_vsta_length_RBUS sub_vgip_vgip_chn2_act_vsta_length_reg;
#endif
#ifndef CONFIG_FORCE_RUN_I3DDMA
	vgip_vgip_chn1_act_hsta_width_RBUS vgip_vgip_chn1_act_hsta_width_reg;
	vgip_vgip_chn1_act_vsta_length_RBUS vgip_vgip_chn1_act_vsta_length_reg;
#else
	dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_dma_vgip_ctrl_reg;
	dma_vgip_dma_vgip_act_hsta_width_RBUS dma_vgip_dma_vgip_act_hsta_width_reg;
	dma_vgip_dma_vgip_act_vsta_length_RBUS dma_vgip_dma_vgip_act_vsta_length_reg;
#endif

	vgip_vgip_chn1_ctrl_reg.regValue = hdmi_in(VGIP_VGIP_CHN1_CTRL_reg);
#ifdef CONFIG_DUAL_CHANNEL
	sub_vgip_vgip_chn2_ctrl_reg.regValue = hdmi_in(SUB_VGIP_VGIP_CHN2_CTRL_reg);
#endif

	if(SLR_MAIN_DISPLAY == vsc_ch)
	{
#ifdef CONFIG_FORCE_RUN_I3DDMA
		dma_vgip_dma_vgip_ctrl_reg.regValue = hdmi_in(DMA_VGIP_DMA_VGIP_CTRL_reg);

		if(no_polarity_inverse_flag)
		{
			dma_vgip_dma_vgip_ctrl_reg.dma_vs_inv = 0;
			dma_vgip_dma_vgip_ctrl_reg.dma_hs_inv = 0;
		}
		else //if(_FALSE == no_polarity_inverse_flag)
		{
			if ((tm->polarity>>1)&0x01)
				dma_vgip_dma_vgip_ctrl_reg.dma_vs_inv = 0;
			else
				dma_vgip_dma_vgip_ctrl_reg.dma_vs_inv = 1;

			if ((tm->polarity)&0x01)
				dma_vgip_dma_vgip_ctrl_reg.dma_hs_inv = 0;
			else
				dma_vgip_dma_vgip_ctrl_reg.dma_hs_inv = 1;
		}

		vgip_ch = HDMI_SMT_PATH_DMA_VGIP;
		hdmi_out(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_dma_vgip_ctrl_reg.regValue);
#else //#ifdef CONFIG_FORCE_RUN_I3DDMA

		if(no_polarity_inverse_flag)
		{
			vgip_vgip_chn1_ctrl_reg.ch1_vs_inv = 0;
			vgip_vgip_chn1_ctrl_reg.ch1_hs_inv = 0;
		}
		else// if(_FALSE == no_polarity_inverse_flag)
		{
			if ((tm->polarity>>1)&0x01)
				vgip_vgip_chn1_ctrl_reg.ch1_vs_inv = 0;
			else
				vgip_vgip_chn1_ctrl_reg.ch1_vs_inv = 1;

			if ((tm->polarity)&0x01)
				vgip_vgip_chn1_ctrl_reg.ch1_hs_inv = 0;
			else
				vgip_vgip_chn1_ctrl_reg.ch1_hs_inv = 1;
		}

		vgip_ch = HDMI_SMT_PATH_MAIN_VGIP;
		hdmi_out(VGIP_VGIP_CHN1_CTRL_reg, vgip_vgip_chn1_ctrl_reg.regValue);
#endif //#ifdef CONFIG_FORCE_RUN_I3DDMA
	}
#ifdef CONFIG_DUAL_CHANNEL
	else if(SLR_SUB_DISPLAY == vsc_ch)
	{
		if(no_polarity_inverse_flag)
		{
			sub_vgip_vgip_chn2_ctrl_reg.ch2_vs_inv = 0;
			sub_vgip_vgip_chn2_ctrl_reg.ch2_hs_inv = 0;
		}
		else //if(_FALSE == no_polarity_inverse_flag)
		{
			if ((tm->polarity>>1)&0x01)
				sub_vgip_vgip_chn2_ctrl_reg.ch2_vs_inv = 0;
			else
				sub_vgip_vgip_chn2_ctrl_reg.ch2_vs_inv = 1;

			if ((tm->polarity)&0x01)
				sub_vgip_vgip_chn2_ctrl_reg.ch2_hs_inv = 0;
			else
				sub_vgip_vgip_chn2_ctrl_reg.ch2_hs_inv = 1;
		}

		vgip_ch = HDMI_SMT_PATH_SUB_VGIP;
		hdmi_out(SUB_VGIP_VGIP_CHN2_CTRL_reg, sub_vgip_vgip_chn2_ctrl_reg.regValue);
	}
#endif
	else
	{
		HDMI_EMG("NOT valid HDMI VGIP path!!\n");
		return FALSE;
	}

	SCALERDRV_WAIT_VS();//willychou add for polarity change

	lib_hdmims_set_vgip_clk_enable(vgip_ch);
	
#ifdef CONFIG_FORCE_RUN_I3DDMA
	// setup VGIP capture for smartfit measure
	if(HDMI_SMT_PATH_DMA_VGIP == vgip_ch)
	{
		dma_vgip_dma_vgip_act_hsta_width_reg.regValue = hdmi_in(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg);
		dma_vgip_dma_vgip_act_hsta_width_reg.dma_ih_act_sta = 0;
		dma_vgip_dma_vgip_act_hsta_width_reg.dma_ih_act_wid = 300;
#ifdef CONFIG_DUAL_CHANNEL
		hdmi_out(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg, sub_vgip_vgip_chn2_act_hsta_width_reg.regValue);
#endif
		dma_vgip_dma_vgip_act_vsta_length_reg.regValue = hdmi_in(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg);
		dma_vgip_dma_vgip_act_vsta_length_reg.dma_iv_act_sta = 0;
		dma_vgip_dma_vgip_act_vsta_length_reg.dma_iv_act_len = 120;
		hdmi_out(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg, dma_vgip_dma_vgip_act_vsta_length_reg.regValue);
	}
#else //#ifdef CONFIG_FORCE_RUN_I3DDMA
	// setup VGIP capture for smartfit measure
	if(HDMI_SMT_PATH_MAIN_VGIP == vgip_ch)
	{
		vgip_vgip_chn1_act_hsta_width_reg.regValue = hdmi_in(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg);
		vgip_vgip_chn1_act_hsta_width_reg.ch1_ih_act_sta = 0;
		vgip_vgip_chn1_act_hsta_width_reg.ch1_ih_act_wid = 300;
		hdmi_out(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg, vgip_vgip_chn1_act_hsta_width_reg.regValue);

		vgip_vgip_chn1_act_vsta_length_reg.regValue = hdmi_in(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg);
		vgip_vgip_chn1_act_vsta_length_reg.ch1_iv_act_sta = 0;
		vgip_vgip_chn1_act_vsta_length_reg.ch1_iv_act_len = 120;
		hdmi_out(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg, vgip_vgip_chn1_act_vsta_length_reg.regValue);
	}
#endif //#ifdef CONFIG_FORCE_RUN_I3DDMA
#ifdef CONFIG_DUAL_CHANNEL

	else if(HDMI_SMT_PATH_SUB_VGIP == vgip_ch)
	{
		sub_vgip_vgip_chn2_act_hsta_width_reg.regValue = hdmi_in(SUB_VGIP_VGIP_CHN2_ACT_HSTA_Width_reg);
		sub_vgip_vgip_chn2_act_hsta_width_reg.ch2_ih_act_sta = 0;
		sub_vgip_vgip_chn2_act_hsta_width_reg.ch2_ih_act_wid = 300;
		hdmi_out(SUB_VGIP_VGIP_CHN2_ACT_HSTA_Width_reg, sub_vgip_vgip_chn2_act_hsta_width_reg.regValue);

		sub_vgip_vgip_chn2_act_vsta_length_reg.regValue = hdmi_in(SUB_VGIP_VGIP_CHN2_ACT_VSTA_Length_reg);
		sub_vgip_vgip_chn2_act_vsta_length_reg.ch2_iv_act_sta = 0;
		sub_vgip_vgip_chn2_act_vsta_length_reg.ch2_iv_act_sta = 120;
		hdmi_out(SUB_VGIP_VGIP_CHN2_ACT_VSTA_Length_reg, sub_vgip_vgip_chn2_act_vsta_length_reg.regValue);
	}
#endif
	else
	{
		HDMI_EMG("NOT valid HDMI VGIP path!!\n");
		return _FALSE;
	}

	//disable smart fit
	hdmi_out(GET_SFT_AUTO_ADJ_VADDR(), 0x0);
	hdmi_out(GET_SFT_AUTO_ADJ_VADDR(), SMARTFIT_AUTO_ADJ_m_vgen_en_mask);

	// disable smartfit auto field selection
	hdmi_mask(GET_SFT_AUTO_FIELD_VADDR(), ~(SMARTFIT_AUTO_FIELD_odd_sel_en_mask|SMARTFIT_AUTO_FIELD_odd_sel_mask), 0);
	hdmi_mask(GET_SFT_AUTO_FIELD_VADDR(), ~(SMARTFIT_AUTO_FIELD_adj_source_mask), SMARTFIT_AUTO_FIELD_adj_source(vgip_ch));
	hdmi_mask(GET_SFT_AUTO_FIELD_VADDR(), ~(SMARTFIT_AUTO_FIELD_smartfit_src_sel_mask), SMARTFIT_AUTO_FIELD_smartfit_src_sel_mask);//forg mango big ball
	hdmi_mask(GET_SFT_AUTO_FIELD_VADDR(), ~(SMARTFIT_AUTO_FIELD_no_hs_mode_mask), 1);
	hdmi_mask(GET_SFT_AUTO_ADJ_VADDR(), ~(SMARTFIT_AUTO_ADJ_m_vgen_en_mask), 0);

	// set Smartfit larger active region
	hdmi_out(GET_SFT_AUTO_H_BOUNDARY_VADDR(), SMARTFIT_AUTO_H_BOUNDARY_hb_sta(0) |  SMARTFIT_AUTO_H_BOUNDARY_hb_end(5000));
	hdmi_out(GET_SFT_AUTO_V_BOUNDARY_VADDR(), SMARTFIT_AUTO_V_BOUNDARY_vb_sta(0) | SMARTFIT_AUTO_V_BOUNDARY_vb_end(3000));

	hdmi_mask(GET_SFT_AUTO_ADJ_VADDR(), ~(SMARTFIT_AUTO_ADJ_now_af_mask), SMARTFIT_AUTO_ADJ_now_af_mask);  //20140923 fix : AC on 1st measure will fail

	timeoutcnt = 30; // 3d need more time?!
	do {
		timeoutcnt --;
		if (!SMARTFIT_AUTO_ADJ_get_now_af(hdmi_in(GET_SFT_AUTO_ADJ_VADDR())))
			break;
		msleep(5);
	} while (timeoutcnt > 0);

	if (0 == timeoutcnt) {
		HDMI_EMG("[SFT][ERR] %s is not finish.\n", __func__);
		hdmi_out(GET_SFT_AUTO_FIELD_VADDR(), 0x0);
		return FALSE;
	}

	tm->v_act_sta = ((hdmi_in(GET_SFT_AUTO_RESULT_VSTA_END_VADDR()) >> 16) & 0xFFF)-1;
	tm->h_act_sta = ((hdmi_in(GET_SFT_AUTO_RESULT_HSTA_END_VADDR()) >> 16) & 0xFFF)-3;

	if (tm->is_interlace) {
		if ((tm->v_total & 0x01)) {	//  ODD field
			tm->v_act_sta--;
		}
	}

	hdmi_out(GET_SFT_AUTO_FIELD_VADDR() , 0x0);

	HDMI_PRINTF("[SFT] IHStr: %d\n", tm->h_act_sta);
	HDMI_PRINTF("[SFT] IVStr: %d\n", tm->v_act_sta);

	return TRUE;
}
#else //#if HDMI_MEASURE_I3DDMA_SMART_FIT
unsigned char lib_hdmims_get_hvstart(HDMI_TIMING_T *tm, unsigned char vsc_ch)
{
// pre-condition:
//
// 0xb800d098[3] = 1

	int timeoutcnt;
	unsigned char vgip_ch = 0; //need to check: Hdmi_GetVGIPChannel();
	vgip_vgip_chn1_ctrl_RBUS vgip_vgip_chn1_ctrl_reg;
#ifdef CONFIG_DUAL_CHANNEL
	sub_vgip_vgip_chn2_ctrl_RBUS sub_vgip_vgip_chn2_ctrl_reg;
#endif

#if 1
	vgip_vgip_chn1_ctrl_reg.regValue = hdmi_in(VGIP_VGIP_CHN1_CTRL_reg);

#ifdef CONFIG_DUAL_CHANNEL
	sub_vgip_vgip_chn2_ctrl_reg.regValue = hdmi_in(SUB_VGIP_VGIP_CHN2_CTRL_reg);
#endif

	if(0 == vgip_vgip_chn1_ctrl_reg.ch1_in_sel)
	{
		if(no_polarity_inverse_flag)
		{
			vgip_vgip_chn1_ctrl_reg.ch1_vs_inv = 0;
			vgip_vgip_chn1_ctrl_reg.ch1_hs_inv = 0;
		}
		else// if(_FALSE == no_polarity_inverse_flag)
		{
			if ((tm->polarity>>1)&0x01)
				vgip_vgip_chn1_ctrl_reg.ch1_vs_inv = 0;
			else
				vgip_vgip_chn1_ctrl_reg.ch1_vs_inv = 1;

			if ((tm->polarity)&0x01)
				vgip_vgip_chn1_ctrl_reg.ch1_hs_inv = 0;
			else
				vgip_vgip_chn1_ctrl_reg.ch1_hs_inv = 1;
		}
		vgip_ch = 0;

		hdmi_out(VGIP_VGIP_CHN1_CTRL_reg, vgip_vgip_chn1_ctrl_reg.regValue);
	}
#ifdef CONFIG_DUAL_CHANNEL
	else if(0 == sub_vgip_vgip_chn2_ctrl_reg.ch2_in_sel)
	{
		if(no_polarity_inverse_flag)
		{
			sub_vgip_vgip_chn2_ctrl_reg.ch2_vs_inv = 0;
			sub_vgip_vgip_chn2_ctrl_reg.ch2_hs_inv = 0;
		}
		else //if(_FALSE == no_polarity_inverse_flag)
		{
			if ((tm->polarity>>1)&0x01)
				sub_vgip_vgip_chn2_ctrl_reg.ch2_vs_inv = 0;
			else
				sub_vgip_vgip_chn2_ctrl_reg.ch2_vs_inv = 1;

			if ((tm->polarity)&0x01)
				sub_vgip_vgip_chn2_ctrl_reg.ch2_hs_inv = 0;
			else
				sub_vgip_vgip_chn2_ctrl_reg.ch2_hs_inv = 1;
		}

		vgip_ch = 1;
		hdmi_out(SUB_VGIP_VGIP_CHN2_CTRL_reg, sub_vgip_vgip_chn2_ctrl_reg.regValue);
	}
#endif
	else {
		HDMI_EMG("NOT valid HDMI VGIP path!!\n");
		return FALSE;
	}
#else //#if 1
	if (((tm->polarity>>1)&0x01) || no_polarity_inverse_flag)   hdmi_mask(GET_VGIP_CHNx_CTRL_ADDR(), ~(VGIP_VGIP_CHN1_CTRL_ch1_vs_inv_mask), 0);
	else                       hdmi_mask(GET_VGIP_CHNx_CTRL_ADDR(), ~(VGIP_VGIP_CHN1_CTRL_ch1_vs_inv_mask), VGIP_VGIP_CHN1_CTRL_ch1_vs_inv_mask);

	if (((tm->polarity)&0x01) || no_polarity_inverse_flag)    hdmi_mask(GET_VGIP_CHNx_CTRL_ADDR(), ~(VGIP_VGIP_CHN1_CTRL_ch1_hs_inv_mask), 0);
	else                        hdmi_mask(GET_VGIP_CHNx_CTRL_ADDR(), ~(VGIP_VGIP_CHN1_CTRL_ch1_hs_inv_mask), VGIP_VGIP_CHN1_CTRL_ch1_hs_inv_mask);
#endif //#if 1

	SCALERDRV_WAIT_VS();//willychou add for polarity change

	lib_hdmims_set_vgip_clk_enable(vgip_ch);

	// setup VGIP capture for smartfit measure
	hdmi_out(GET_VGIP_CHNx_V_CAP_ADDR(), (0 << 16) | (120));
	hdmi_out(GET_VGIP_CHNx_H_CAP_ADDR(), (0 << 16) | (300));

	//disable smart fit
	hdmi_out(GET_SFT_AUTO_ADJ_VADDR(), 0x0);
	hdmi_out(GET_SFT_AUTO_ADJ_VADDR(), SMARTFIT_AUTO_ADJ_m_vgen_en_mask);

	// disable smartfit auto field selection
	hdmi_mask(GET_SFT_AUTO_FIELD_VADDR(), ~(SMARTFIT_AUTO_FIELD_odd_sel_en_mask|SMARTFIT_AUTO_FIELD_odd_sel_mask), 0);
	hdmi_mask(GET_SFT_AUTO_FIELD_VADDR(), ~(SMARTFIT_AUTO_FIELD_adj_source_mask), SMARTFIT_AUTO_FIELD_adj_source(vgip_ch));
	hdmi_mask(GET_SFT_AUTO_FIELD_VADDR(), ~(SMARTFIT_AUTO_FIELD_smartfit_src_sel_mask), SMARTFIT_AUTO_FIELD_smartfit_src_sel_mask);//forg mango big ball

	hdmi_mask(GET_SFT_AUTO_FIELD_VADDR(), ~(SMARTFIT_AUTO_FIELD_no_hs_mode_mask), 1);
	hdmi_mask(GET_SFT_AUTO_ADJ_VADDR(), ~(SMARTFIT_AUTO_ADJ_m_vgen_en_mask), 0);

	// set Smartfit larger active region
	hdmi_out(GET_SFT_AUTO_H_BOUNDARY_VADDR(), SMARTFIT_AUTO_H_BOUNDARY_hb_sta(0) |  SMARTFIT_AUTO_H_BOUNDARY_hb_end(5000));
	hdmi_out(GET_SFT_AUTO_V_BOUNDARY_VADDR(), SMARTFIT_AUTO_V_BOUNDARY_vb_sta(0) | SMARTFIT_AUTO_V_BOUNDARY_vb_end(3000));

	hdmi_mask(GET_SFT_AUTO_ADJ_VADDR(), ~(SMARTFIT_AUTO_ADJ_now_af_mask), SMARTFIT_AUTO_ADJ_now_af_mask);  //20140923 fix : AC on 1st measure will fail

	timeoutcnt = 30; // 3d need more time?!
	do {
		timeoutcnt --;
		if (!SMARTFIT_AUTO_ADJ_get_now_af(hdmi_in(GET_SFT_AUTO_ADJ_VADDR())))
			break;
		msleep(5);
	} while (timeoutcnt > 0);

	if (0 == timeoutcnt) {
		HDMI_EMG("[SFT][ERR] %s is not finish.\n", __func__);
		hdmi_out(GET_SFT_AUTO_FIELD_VADDR(), 0x0);
		return FALSE;
	}

	tm->v_act_sta = ((hdmi_in(GET_SFT_AUTO_RESULT_VSTA_END_VADDR()) >> 16) & 0xFFF)-1;
	tm->h_act_sta = ((hdmi_in(GET_SFT_AUTO_RESULT_HSTA_END_VADDR()) >> 16) & 0xFFF)-3;

	if (tm->is_interlace) {
		if ((tm->v_total & 0x01)) {	//  ODD field
			tm->v_act_sta--;
		}
	}

	hdmi_out(GET_SFT_AUTO_FIELD_VADDR() , 0x0);

	HDMI_PRINTF("[SFT] IHStr: %d\n", tm->h_act_sta);
	HDMI_PRINTF("[SFT] IVStr: %d\n", tm->v_act_sta);

	return TRUE;
}
#endif //#if HDMI_MEASURE_I3DDMA_SMART_FIT

//USER:LewisLee DATE:2015/11/09
//Fix error timing let picture abnormal
unsigned char lib_hdmi_skiptiming_condition(HDMI_TIMING_T *timing)
{
	//WOSQRTK-9205, HDD Player, APPLE TV
	// 1832x540 progressive NG,
	if(HDMI_ABS(timing->h_act_len, 1832) < 10)
	{
		if(HDMI_ABS(timing->v_act_len, 540) < 10)
		{
			if(0 == timing->is_interlace)
			{
				HDMI_PRINTF("SkipTiming h_act_len=%d, v_act_len=%d, v_total=%d, h_total=%d\n", timing->h_act_len, timing->v_act_len, timing->v_total, timing->h_total);
				return TRUE;
			}
		}
	}

	//WOSQRTK-9205, HDD Player, APPLE TV
	// 1280x716 NG
	if(HDMI_ABS(timing->h_act_len, 1280) < 5)
	{
		if(HDMI_ABS(timing->v_act_len, 716) < 2)
		{
			if(HDMI_ABS(timing->v_freq, 501) < 2)
			{
				HDMI_PRINTF("SkipTiming h_act_len=%d, v_act_len=%d, v_total=%d, h_total=%d\n", timing->h_act_len, timing->v_act_len, timing->v_total, timing->h_total);
				return TRUE;
			}
		}
	}

	//WOSQRTK-9205, HDD Player, APPLE TV
	// 1360x766x60 / 1360x760x60 1360x477x60 / 1360x764x60 NG
	if(HDMI_ABS(timing->h_act_len, 1360) < 5)
	{
		if(timing->v_act_len <= 766)
		{
			if(HDMI_ABS(timing->v_freq, 600) < 2)
			{
				HDMI_PRINTF("SkipTiming h_act_len=%d, v_act_len=%d, v_total=%d, h_total=%d\n", timing->h_act_len, timing->v_act_len, timing->v_total, timing->h_total);
				return TRUE;
			}
		}
	}

	//WOSQRTK-9306, [40000] Radeon R9 390 OC D5 8GB Tri-X NITRO - SAPPHIRE
	// 2016x2160x57.7Hz progressive NG,
	// if DVI=> HDMI, it will have 1984x2160x57.7 Hz
	if(HDMI_ABS(timing->v_act_len, 2160) < 10)
	{
		if(timing->h_act_len <= 3200)
		{
			HDMI_PRINTF("SkipTiming h_act_len=%d, v_act_len=%d, v_total=%d, h_total=%d\n", timing->h_act_len, timing->v_act_len, timing->v_total, timing->h_total);
			return TRUE;
		}
	}

/*
	if(HDMI_ABS(timing->h_act_len, 1082) < 10)
	{
		if(HDMI_ABS(timing->v_act_len, 499) < 10)
		{
			//BH9520, 1082x499 error timing
			HDMI_PRINTF("SkipTiming h_act_len=%d, v_act_len=%d, v_total=%d, h_total=%d\n", timing->h_act_len, timing->v_act_len, timing->v_total, timing->h_total);
			return TRUE;
		}
	}
*/
/*
	if(HDMI_ABS(timing->v_act_len, 1080) < 10)
	{
		if(timing->h_act_len < 800)
		{
			// MSPG 7100 / 7600, change timing too fast, have short time 760x1080 error timing
			HDMI_PRINTF("SkipTiming h_act_len=%d, v_act_len=%d, v_total=%d, h_total=%d\n", timing->h_act_len, timing->v_act_len, timing->v_total, timing->h_total);
			return TRUE;
		}
	}
*/
/*
	//USER:LewisLee DATE:2016/05/24
	//move from Hdmi_OffLineMeasure()
	if (timing->v_act_len == 1080)
	{
		//cloud modify for 1910 problem
		if ((timing->h_act_len > 1905 && timing->h_act_len < 1919) || (timing->h_act_len > 1921 && timing->h_act_len < 1935))
		{
			if(HDMI_ABS(timing->h_act_len, 1920) <= 5)
			{
				if(ucSkip_1080P_Cnt < 2 )
				{
					HDMI_PRINTF("SkipTiming I, H=%d, V=%d, cnt=%d\n", timing->h_act_len, timing->v_act_len, ucSkip_1080P_Cnt);
					ucSkip_1080P_Cnt++;
					return TRUE;
				}
			}
			else// if(HDMI_ABS(timing->h_act_len, 1920) > 5)
			{
				ucSkip_1080P_Cnt = 0;
				HDMI_PRINTF("SkipTiming II, H=%d, V=%d, cnt=%d\n", timing->h_act_len, timing->v_act_len, ucSkip_1080P_Cnt);
				return TRUE;
			}
		}
	}
*/
/*
	//blu-ray player, bd-c6900
	if(timing->h_act_len <= timing->v_act_len)
	{
		HDMI_PRINTF("Abnormal Timing h_act_len=%d, v_act_len=%d, v_total=%d, h_total=%d\n", timing->h_act_len, timing->v_act_len, timing->v_total, timing->h_total);

		if(timing->h_act_len <= 600)
		{
			HDMI_PRINTF("SkipTiming\n");
			return TRUE;
		}

		//Minchay@21060117#WOSQRTK-5224 Invalid Format - 960*2160 issue
		if((timing->v_act_len >= 2150)&& (timing->v_act_len <= 2170))
		{//v:2160
			if(timing->h_act_len <= 1500)
			{//h <= 1500
				HDMI_PRINTF("SkipTiming\n");
				return TRUE;
			}
		}
	}
*/
	return FALSE;
}

