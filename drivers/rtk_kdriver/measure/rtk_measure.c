/*=============================================================
 * File:    rtk_measure.c
 *
 * Desc:    RTK Measure
 *
 * AUTHOR:  kevin_wang@realtek.com
 *
 * Vresion: 0.0.1
 *
 *------------------------------------------------------------
 * Copyright (c) Realtek Semiconductor Corporation, 2023
 *
 * All rights reserved.
 *============================================================*/
#include <rtk_kdriver/measure/rtk_measure.h>
#include "rtk_measure-priv.h"



/**********************************************************************************************
*
*   Variables
*
**********************************************************************************************/
unsigned char no_polarity_inverse_flag = FALSE;//allow vgip to set h v polarity inverse or not
static unsigned char is_measure_continuous_init = FALSE;
static unsigned int measure_not_finish_cnt = 0;
static unsigned int measure_start = 0;

/*------------------------------------------------
 * Array: tm_table
 *
 * Desc : Table for timing correction
 *
 *-----------------------------------------------*/
const TIMING_TABLE_T tm_table[] =
{
    {4096, 2160, 15, 10},   // 4k2K
    {3840, 2160, 15, 10},   // 4k2K
    {1920, 1080, 10, 5},    // 1080P
    {1920, 540, 5, 3},      // 1080i
    {1280, 720, 5, 5},      // 720P
    {720, 576, 3, 3},       // 576P
    {640, 480, 3, 3},       // 480P
    {720, 480, 3, 3},       // 480P
    {720, 288, 3, 3},       // 576i
    {720, 240, 3, 3},       // 480i
};

#define TM_TB_SIZE      (sizeof(tm_table)/sizeof(TIMING_TABLE_T))


/**********************************************************************************************
*
*   Funtion Declarations
*
**********************************************************************************************/

#ifdef CONFIG_SUPPORT_SCALER
    // exported in scalerdrv/mode.c   : off line measure is a shared soure, so we need the semaphore to prevent race condition
    extern struct semaphore *get_offline_semaphore(void);
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
*   Function Body
*
**********************************************************************************************/
void _dump_measure_status(const char *msg)
{
    MEASURE_WARN("%s : ofms status (MS3Ctrl:%08x, MS3Stus=%08x), measure_not_finish_cnt=%d\n",
                 msg, rtd_inl(OFFMS_SP_MS3Ctrl_reg), rtd_inl(OFFMS_SP_MS3Stus_reg), measure_not_finish_cnt);
}

void lib_hdmims_offms_config_before_startms(OFFMS_MSMODE ms_mode, unsigned char off_ms_source)
{
    switch(ms_mode)
    {
    case OFFMS_CONTINUOUS_MODE:
        if(is_measure_continuous_init == FALSE)
        {
            _dump_measure_status("OFFMS_CONTINUOUS_MODE init");
            rtd_maskl(OFFMS_SP_MS3Ctrl_reg, ~(OFFMS_SP_MS3Ctrl_off_meas_source_mask), OFFMS_SP_MS3Ctrl_off_meas_source(off_ms_source) );
            rtd_maskl(OFFMS_SP_MS3Ctrl_reg, ~(OFFMS_SP_MS3Ctrl_off_clk_sel_mask), OFFMS_SP_MS3Ctrl_off_clk_sel(1));
            rtd_maskl(OFFMS_SP_MS3Ctrl_reg, ~(OFFMS_SP_MS3Ctrl_off_meas_to_sel_mask), OFFMS_SP_MS3Ctrl_off_meas_to_sel(1));
            rtd_maskl(OFFMS_SP_MS3Ctrl_reg, ~(OFFMS_SP_MS3Ctrl_off_sycms_mode_mask),  OFFMS_SP_MS3Ctrl_off_sycms_mode(ANALOG_MODE_MEASUREMENT));
            //start continuouss mode
            rtd_maskl(OFFMS_SP_MS3Ctrl_reg, ~(OFFMS_SP_MS3Ctrl_off_blk_2frame_en_mask),  OFFMS_SP_MS3Ctrl_off_blk_2frame_en_mask);
            rtd_maskl(OFFMS_SP_MS3Ctrl_reg, ~(OFFMS_SP_MS3Ctrl_off_online_en_mask),  OFFMS_SP_MS3Ctrl_off_online_en_mask); //continuous mode en.

            //start measure
            rtd_maskl(OFFMS_SP_MS3Ctrl_reg, ~(OFFMS_SP_MS3Ctrl_off_start_ms_mask),  OFFMS_SP_MS3Ctrl_off_start_ms_mask);
#ifdef OFFMS_SP_MS3Stus_off_msdone_mask
            rtd_maskl(OFFMS_SP_MS3Stus_reg, ~OFFMS_SP_MS3Stus_off_msdone_mask, OFFMS_SP_MS3Stus_off_msdone_mask); //merlin6 new add,write 1 clear
#endif
            is_measure_continuous_init = TRUE;
        }
        break;

    case OFFMS_ONE_SHOT:
    default:
        _dump_measure_status("OFFMS_ONE_SHOT init");
        rtd_maskl(OFFMS_SP_MS3Ctrl_reg, ~(OFFMS_SP_MS3Ctrl_off_meas_source_mask), OFFMS_SP_MS3Ctrl_off_meas_source(off_ms_source) );
        rtd_maskl(OFFMS_SP_MS3Ctrl_reg, ~(OFFMS_SP_MS3Ctrl_off_clk_sel_mask), OFFMS_SP_MS3Ctrl_off_clk_sel(1));
        rtd_maskl(OFFMS_SP_MS3Ctrl_reg, ~(OFFMS_SP_MS3Ctrl_off_meas_to_sel_mask), OFFMS_SP_MS3Ctrl_off_meas_to_sel(1));
        rtd_maskl(OFFMS_SP_MS3Ctrl_reg, ~(OFFMS_SP_MS3Ctrl_off_sycms_mode_mask),  OFFMS_SP_MS3Ctrl_off_sycms_mode(ANALOG_MODE_MEASUREMENT));
        //set one time mode
        rtd_maskl(OFFMS_SP_MS3Ctrl_reg, ~(OFFMS_SP_MS3Ctrl_off_blk_2frame_en_mask),  0);
        rtd_maskl(OFFMS_SP_MS3Ctrl_reg, ~(OFFMS_SP_MS3Ctrl_off_online_en_mask),  0);    //one shot mode

        //reset start measure
        rtd_maskl(OFFMS_SP_MS3Ctrl_reg, ~(OFFMS_SP_MS3Ctrl_off_start_ms_mask),  0);
#ifdef OFFMS_SP_MS3Stus_off_msdone_mask
        rtd_maskl(OFFMS_SP_MS3Stus_reg, ~OFFMS_SP_MS3Stus_off_msdone_mask, OFFMS_SP_MS3Stus_off_msdone_mask); //merlin6 new add, write 1 clear
#endif
        is_measure_continuous_init = FALSE;

        break;
    }
}

void set_no_polarity_inverse(unsigned char TorF)
{
    //allow vgip to set h v polarity inverse or not
    no_polarity_inverse_flag = TorF;
}

EXPORT_SYMBOL(set_no_polarity_inverse);

unsigned char lib_measure_active_correction(unsigned int *h_active, unsigned int *v_active)
{
    unsigned char i;
    unsigned char is_correct = FALSE;
    for (i = 0; i < TM_TB_SIZE; i++) {
        if (MEASURE_ABS(*h_active, tm_table[i].h_act) < tm_table[i].h_tol &&
                MEASURE_ABS(*v_active, tm_table[i].v_act) < tm_table[i].v_tol) {

            if (*h_active != tm_table[i].h_act || *v_active != tm_table[i].v_act) {
                MEASURE_EMG("[HDMI][OFMS] Corrected H/V=(%d,%d) to (%d,%d)\n",
                            *h_active, *v_active, tm_table[i].h_act, tm_table[i].v_act);
            }
            *h_active = tm_table[i].h_act;
            *v_active = tm_table[i].v_act;
            is_correct = TRUE;
        }
    }

    //Correct other timing which is not at tm_table
    if ((*h_active) % 10 == 9) {
        MEASURE_WARN("Corrected H ++ Original=(%d)\n", *h_active);
        (*h_active)++;
        is_correct = TRUE;
    }
    else if ((*h_active) % 10 == 1) {
        MEASURE_WARN("Corrected H -- Original=(%d)\n", *h_active);
        (*h_active)--;
        is_correct = TRUE;
    }

    if ((*v_active) % 10 == 9) {
        MEASURE_WARN("Corrected V++ Original=(%d)\n", *v_active);
        (*v_active)++;
        is_correct = TRUE;
    }
    else if ((*v_active) % 10 == 1) {
        MEASURE_WARN("Corrected V-- Original=(%d)\n", *v_active);
        (*v_active)--;
        is_correct = TRUE;
    }

    return is_correct;
}
EXPORT_SYMBOL(lib_measure_active_correction);

#ifndef UT_flag

unsigned char lib_offline_measure(OFFMS_SOURCE_SEL sel, MEASURE_TIMING_T *tm, unsigned char frl_mode)
{
    return lib_offline_measure_ex(sel, tm, frl_mode, OFFMS_ONE_SHOT);
}

EXPORT_SYMBOL(lib_offline_measure);

unsigned char lib_offline_measure_ex(OFFMS_SOURCE_SEL sel, MEASURE_TIMING_T *tm, unsigned char frl_mode, OFFMS_MSMODE ms_mode)
{
    unsigned int raw_h_sync_width = 0;
    unsigned int raw_v_sync_width = 0;
    unsigned int off_source_sel = 0x8;
#if defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F)
    if(sel == OFFMS_SOURCE_DPRX)
        off_source_sel = 0xA;
#endif
    lib_hdmims_offms_config_before_startms(ms_mode, off_source_sel);
    if (!lib_measure_offms_start()) {
        measure_not_finish_cnt++;
        MEASURE_WARN("Hdmi_OffLineMeasure is not finish. measure_not_finish_cnt=%d\n", measure_not_finish_cnt);
        return FALSE;
    }

    tm->h_total = OFFMS_SP_MS3Rst1_get_off_hs_period_out_h12b(rtd_inl(OFFMS_SP_MS3Rst1_reg)) + 1;
    tm->v_total = OFFMS_SP_MS3Rst0_get_off_vs_period_out(rtd_inl(OFFMS_SP_MS3Rst0_reg)) + 1;
    tm->h_act_sta = OFFMS_MES_RESULT_HSTA_END_get_off_hx_sta(rtd_inl(OFFMS_MES_RESULT_HSTA_END_reg));
    tm->v_act_sta = OFFMS_MES_RESULT_VSTA_END_get_off_vx_sta(rtd_inl(OFFMS_MES_RESULT_VSTA_END_reg));
    tm->h_act_len = OFFMS_MES_RESULT_HSTA_END_get_off_hx_end(rtd_inl(OFFMS_MES_RESULT_HSTA_END_reg)) - tm->h_act_sta;
    tm->v_act_len = OFFMS_MES_RESULT_VSTA_END_get_off_vx_end(rtd_inl(OFFMS_MES_RESULT_VSTA_END_reg)) - tm->v_act_sta + 1;
    tm->polarity = (OFFMS_SP_MS3Stus_get_off_vs_pol_out(rtd_inl(OFFMS_SP_MS3Stus_reg)) << 1) |
                   OFFMS_SP_MS3Stus_get_off_hs_pol_out(rtd_inl(OFFMS_SP_MS3Stus_reg));

    tm->h_act_sta += 1;//willychou modify it for OFFMS_START_CORRECTION
    tm->v_act_sta -= 1;//willychou modify it for OFFMS_START_CORRECTION

    raw_h_sync_width = OFFMS_SP_MS3Rst2_get_off_hs_high_out(rtd_inl(OFFMS_SP_MS3Rst2_reg)) + 1;
    raw_v_sync_width = OFFMS_SP_MS3Rst2_get_off_vs_high_out(rtd_inl(OFFMS_SP_MS3Rst2_reg)) + 1;

    if (0 == (tm->polarity & _BIT0)) {
        //Hsync negative polarity
        if (tm->h_total >= raw_h_sync_width) {
            tm->h_act_sta += (tm->h_total - raw_h_sync_width);
            tm->IHSyncPulseCount = (tm->h_total - raw_h_sync_width);
        }
        else {
            MEASURE_WARN("Warning h_total=%d < raw_h_sync_width=%d\n", tm->h_total, raw_h_sync_width);
        }
    }
    else {
        // Hsync positive polarity
        tm->IHSyncPulseCount = raw_h_sync_width;
    }

    if (0 == (tm->polarity & _BIT1)) {
        //Vsync negative polarity
        if (tm->v_total >= raw_v_sync_width) {
            tm->v_act_sta += (tm->v_total - raw_v_sync_width);
            tm->IVSyncPulseCount = (tm->v_total - raw_v_sync_width);
        }
        else {
            MEASURE_WARN("Warning v_total=%d < raw_v_sync_width=%d\n", tm->v_total, raw_v_sync_width);
        }
    }
    else {
        // Vsync positive polarity
        tm->IVSyncPulseCount = raw_v_sync_width;
    }

    switch(sel)
    {
    case OFFMS_SOURCE_HDMI:
    case OFFMS_SOURCE_HDMI21:
    case OFFMS_SOURCE_DSCD:

#if MEASURE_FROM_TMDS_CLOCK
        if (tm->pixel_repeat > 0) {
            tm->h_act_sta /= (tm->pixel_repeat + 1);
            tm->h_total /= (tm->pixel_repeat + 1);
            tm->h_act_len /= (tm->pixel_repeat + 1);
        }

        if (COLOR_YUV420 == tm->colorspace) {
            tm->h_act_sta <<= 1;
            tm->h_total <<= 1;
            tm->h_act_len <<= 1;
        }

        if (HDMI_COLOR_DEPTH_10B == tm->colordepth) {
            // value / 1.25
            tm->h_act_sta = ((tm->h_act_sta * 4) + 3) / 5;
            tm->h_total = ((tm->h_total * 4) + 3) / 5;
            tm->h_act_len = ((tm->h_act_len * 4) + 3) / 5;

        }
        else if (HDMI_COLOR_DEPTH_12B == tm->colordepth) {
            // value / 1.5
            tm->h_act_sta = ((tm->h_act_sta * 2) + 2) / 3;
            tm->h_total = ((tm->h_total * 2) + 2) / 3;
            tm->h_act_len = ((tm->h_act_len * 2) + 2) / 3;

        }
        else if (HDMI_COLOR_DEPTH_16B == tm->colordepth) {
            // value / 2
            tm->h_act_sta = tm->h_act_sta >> 1;
            tm->h_total = tm->h_total >> 1;
            tm->h_act_len = tm->h_act_len >> 1;

        }
#endif

        if (frl_mode != MODE_TMDS) {
            //H5X 4 pixel mode
            MEASURE_INFO("[OFMS]H5X 4 Pixel mode!Hx4\n");
            tm->h_act_sta <<= 2;
            tm->h_total <<= 2;
            tm->h_act_len <<= 2;
            if (tm->pixel_repeat > 0)
            {
                MEASURE_WARN("Pixel Repeat =%d !H/(pixel_repeat+1)\n", tm->pixel_repeat);
                tm->h_act_sta /= (tm->pixel_repeat+1);
                tm->h_total /= (tm->pixel_repeat+1);
                tm->h_act_len /= (tm->pixel_repeat+1);
            }
        }

        if(COLOR_YUV420 == tm->colorspace)
        {
            if(frl_mode!=MODE_TMDS)
            {//For FRL YUV420
                MEASURE_WARN("HDMI2.1 YUV420, Hx2!\n");
                tm->h_act_sta <<= 1;
                tm->h_total <<= 1;
                tm->h_act_len <<= 1;
                tm->IHSyncPulseCount <<= 1;
            }
            else if (tm->pll_pixelclockx1024 > PLL_PIXELCLOCK_X1024_300M)
            {//For TMDS YUV420
                MEASURE_WARN("HDMI2.0 TMDS YUV420 over 300M, Hx2!,  pll_pixelclockx1024=%ld\n",  tm->pll_pixelclockx1024 );
                tm->h_act_sta <<= 1;
                tm->h_total <<= 1;
                tm->h_act_len <<= 1;
                tm->IHSyncPulseCount <<= 1;
            }
        }
        break;

     case OFFMS_SOURCE_DPRX:
        // DPRX pixel mode
        MEASURE_WARN("DPRX 2 Pixel mode!Hx2 (H/V mode)\n");
        tm->h_total <<= 1;
        tm->h_act_len <<= 1;
        break;

    default:
        break;
    }

    lib_measure_active_correction(&(tm->h_act_len), &(tm->v_act_len));

    MEASURE_WARN("[OFMS] IHTotal: %d\n", tm->h_total);
    MEASURE_WARN("[OFMS] IVTotal: %d\n", tm->v_total);
    MEASURE_WARN("[OFMS] IHAct: %d\n", tm->h_act_len);
    MEASURE_WARN("[OFMS] IVAct: %d\n", tm->v_act_len);
    MEASURE_WARN("[OFMS] IHStr: %d\n", tm->h_act_sta);
    MEASURE_WARN("[OFMS] IVStr: %d\n", tm->v_act_sta);
    MEASURE_WARN("[OFMS] Polarity: %d\n", tm->polarity);
    MEASURE_WARN("[OFMS] IHsyncWidth %d\n", tm->IHSyncPulseCount);
    MEASURE_WARN("[OFMS] IVsyncWidth %d\n", tm->IVSyncPulseCount);

    return TRUE;

}
EXPORT_SYMBOL(lib_offline_measure_ex);

unsigned char lib_offline_measure_demode(OFFMS_SOURCE_SEL sel, MEASURE_TIMING_T *tm, unsigned char frl_mode)
{
#if !OFFMS_SUPPORT_DEMODE
    return FALSE;  //Mac7p don't support demode
#else
    unsigned int off_source_sel = 0x8;
    unsigned long long pix_clk = 0;
    unsigned int de_vs_end = 0;
    unsigned int vfreq = 0;
#if defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F)
    if(sel == OFFMS_SOURCE_DPRX)
        off_source_sel = 0xA;
#endif
    rtd_maskl(OFFMS_SP_MS3Ctrl_reg,
              ~(OFFMS_SP_MS3Ctrl_off_meas_source_mask | OFFMS_SP_MS3Ctrl_off_clk_sel_mask
                | OFFMS_SP_MS3Ctrl_off_meas_to_sel_mask | OFFMS_SP_MS3Ctrl_off_sycms_mode_mask
                | OFFMS_SP_MS3Ctrl_off_popup_meas_mask | OFFMS_SP_MS3Ctrl_off_start_ms_mask),
              OFFMS_SP_MS3Ctrl_off_meas_source(off_source_sel) | OFFMS_SP_MS3Ctrl_off_clk_sel(1)
              | OFFMS_SP_MS3Ctrl_off_meas_to_sel(1) | OFFMS_SP_MS3Ctrl_off_sycms_mode(ANALOG_MODE_MEASUREMENT)
              | OFFMS_SP_MS3Ctrl_get_off_popup_meas(0) | OFFMS_SP_MS3Ctrl_get_off_start_ms(0));

    if (!lib_measure_offms_start()) {
        MEASURE_WARN("DE Mode, Hdmi_OffLineMeasure is not finish.\n");
        return FALSE;
    }

    tm->h_total = OFFMS_Demode_Hs_period_get_de_hs_period(rtd_inl(OFFMS_Demode_Hs_period_reg));
    //tm->v_total = OFFMS_SP_MS3Rst0_get_off_vs_period_out(rtd_inl(OFFMS_SP_MS3Rst0_reg))+1;
    //tm->h_act_sta = OFFMS_Demode_VsHs_Act_get_de_hs_act(rtd_inl(OFFMS_Demode_VsHs_Act_reg));
    tm->v_act_sta = OFFMS_Demode_Vs_STA_get_de_vs_sta(rtd_inl(OFFMS_Demode_Vs_STA_reg));
    tm->h_act_len =  OFFMS_Demode_VsHs_Act_get_de_hs_act(rtd_inl(OFFMS_Demode_VsHs_Act_reg));
    tm->v_act_len = OFFMS_Demode_VsHs_Act_get_de_vs_act(rtd_inl(OFFMS_Demode_VsHs_Act_reg));
    //tm->polarity = (OFFMS_SP_MS3Stus_get_off_vs_pol_out(rtd_inl(OFFMS_SP_MS3Stus_reg))<<1)|OFFMS_SP_MS3Stus_get_off_hs_pol_out(rtd_inl(OFFMS_SP_MS3Stus_reg));

    //tm->IVSyncPulseCount = OFFMS_SP_MS3Rst2_get_off_vs_high_out(rtd_inl(OFFMS_SP_MS3Rst2_reg))+1;
    //tm->IHSyncPulseCount = OFFMS_SP_MS3Rst2_get_off_hs_high_out(rtd_inl(OFFMS_SP_MS3Rst2_reg))+1;
    de_vs_end = OFFMS_Demode_Vs_END_get_de_vs_end(rtd_inl(OFFMS_Demode_Vs_END_reg));
    switch(sel)
    {
    case OFFMS_SOURCE_HDMI:
    case OFFMS_SOURCE_HDMI21:
    case OFFMS_SOURCE_DSCD:
        if (frl_mode != MODE_TMDS) {
            //H5X 4 pixel mode
            MEASURE_WARN("H5X 4 Pixel mode!Hx4\n");
            tm->h_total <<= 2;
            tm->h_act_len <<= 2;
            if (tm->pixel_repeat > 0) {
                MEASURE_WARN("Pixel Repeat =%d !H/(pixel_repeat+1)\n", tm->pixel_repeat);
                tm->h_total /= (tm->pixel_repeat + 1);
                tm->h_act_len /= (tm->pixel_repeat + 1);
            }
        }

        if ((frl_mode != MODE_TMDS) && (COLOR_YUV420 == tm->colorspace)) {
            MEASURE_WARN("YUV420, Hx2!\n");
            tm->h_total <<= 1;
            tm->h_act_len <<= 1;
        }
        break;

    case OFFMS_SOURCE_DPRX:
        // DPRX pixel mode
        MEASURE_WARN("DPRX 2 Pixel mode!Hx2\n");
        tm->h_total <<= 1;
        tm->h_act_len <<= 1;
        break;

    default:
        break;
    }

    // pixclk = (htotal*vact - hporch) * 27 / (de_vs_end - de_vs_star)    // MHZ
    // vfreq = pixclk/vfreq/htotal
    pix_clk = (unsigned long long)tm->h_total * tm->v_act_len *27*1000000/ (de_vs_end - tm->v_act_sta);   //Hz
    vfreq = lib_measure_calc_vfreq(lib_measure_get_vs_period());
    if(vfreq != 0 && tm->h_total != 0)
        tm->v_total = (pix_clk/ tm->h_total)*1000/vfreq;

    MEASURE_INFO("[OFMS] IHTotal: %d\n", tm->h_total);
    MEASURE_INFO("[OFMS] IHAct: %d\n", tm->h_act_len);
    MEASURE_INFO("[OFMS] IHAct_Frac: %d\n", OFFMS_Demode_VsHs_Act_get_de_hs_act_fraction(rtd_inl(OFFMS_Demode_VsHs_Act_reg)));

    MEASURE_INFO("[OFMS] IVTotal: %d\n", tm->v_total);
    MEASURE_INFO("[OFMS] IVAct: %d\n", tm->v_act_len);
    MEASURE_INFO("[OFMS] IVStr: %d\n", tm->v_act_sta);
    MEASURE_INFO("[OFMS] IVStr_End: %d\n", de_vs_end);

    return TRUE;
#endif
}
EXPORT_SYMBOL(lib_offline_measure_demode);


unsigned char lib_offline_measure_native(OFFMS_SOURCE_SEL sel, MEASURE_TIMING_T *tm, unsigned char frl_mode)
{
    unsigned int raw_h_sync_width = 0;
    unsigned int raw_v_sync_width = 0;
    unsigned int off_source_sel = 0x8;
#if defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F)
    if(sel == OFFMS_SOURCE_DPRX)
        off_source_sel = 0xA;
#endif
    rtd_maskl(OFFMS_SP_MS3Ctrl_reg,
              ~(OFFMS_SP_MS3Ctrl_off_meas_source_mask | OFFMS_SP_MS3Ctrl_off_clk_sel_mask
                | OFFMS_SP_MS3Ctrl_off_meas_to_sel_mask | OFFMS_SP_MS3Ctrl_off_sycms_mode_mask
                | OFFMS_SP_MS3Ctrl_off_popup_meas_mask | OFFMS_SP_MS3Ctrl_off_start_ms_mask),
              OFFMS_SP_MS3Ctrl_off_meas_source(off_source_sel) | OFFMS_SP_MS3Ctrl_off_clk_sel(1)
              | OFFMS_SP_MS3Ctrl_off_meas_to_sel(1) | OFFMS_SP_MS3Ctrl_off_sycms_mode(ANALOG_MODE_MEASUREMENT)
              | OFFMS_SP_MS3Ctrl_get_off_popup_meas(0) | OFFMS_SP_MS3Ctrl_get_off_start_ms(0));

    if (!lib_measure_offms_start()) {
        MEASURE_WARN("Hdmi_OffLineMeasure is not finish.\n");
        return FALSE;
    }

    tm->h_total = OFFMS_SP_MS3Rst1_get_off_hs_period_out_h12b(rtd_inl(OFFMS_SP_MS3Rst1_reg)) + 1;
    tm->v_total = OFFMS_SP_MS3Rst0_get_off_vs_period_out(rtd_inl(OFFMS_SP_MS3Rst0_reg)) + 1;
    tm->h_act_sta = OFFMS_MES_RESULT_HSTA_END_get_off_hx_sta(rtd_inl(OFFMS_MES_RESULT_HSTA_END_reg));
    tm->v_act_sta = OFFMS_MES_RESULT_VSTA_END_get_off_vx_sta(rtd_inl(OFFMS_MES_RESULT_VSTA_END_reg));
    tm->h_act_len = OFFMS_MES_RESULT_HSTA_END_get_off_hx_end(rtd_inl(OFFMS_MES_RESULT_HSTA_END_reg)) - tm->h_act_sta;
    tm->v_act_len = OFFMS_MES_RESULT_VSTA_END_get_off_vx_end(rtd_inl(OFFMS_MES_RESULT_VSTA_END_reg)) - tm->v_act_sta + 1;
    tm->polarity = (OFFMS_SP_MS3Stus_get_off_vs_pol_out(rtd_inl(OFFMS_SP_MS3Stus_reg)) << 1) |
                   OFFMS_SP_MS3Stus_get_off_hs_pol_out(rtd_inl(OFFMS_SP_MS3Stus_reg));

    tm->h_act_sta += 1;//willychou modify it for OFFMS_START_CORRECTION
    tm->v_act_sta -= 1;//willychou modify it for OFFMS_START_CORRECTION

    raw_h_sync_width = OFFMS_SP_MS3Rst2_get_off_hs_high_out(rtd_inl(OFFMS_SP_MS3Rst2_reg)) + 1;
    raw_v_sync_width = OFFMS_SP_MS3Rst2_get_off_vs_high_out(rtd_inl(OFFMS_SP_MS3Rst2_reg)) + 1;
    tm->IHSyncPulseCount = raw_h_sync_width;
    tm->IVSyncPulseCount = raw_v_sync_width;

    return TRUE;

}
EXPORT_SYMBOL(lib_offline_measure_native);


unsigned char lib_hdmi_dp_measure(OFFMS_SOURCE_SEL sel, MEASURE_TIMING_T *tm, unsigned char frl_mode,unsigned char is_HV_Mode)
{
    unsigned char ret = TRUE;
    if(is_HV_Mode== TRUE){
        ret &= lib_offline_measure_ex(sel, tm, frl_mode, OFFMS_ONE_SHOT);
        if (ret == TRUE) {
            return lib_measure_calc_freq(tm);
        }
        else {
            return ret;
        }
    }else
    {
        ret &= lib_offline_measure_demode(sel, tm, frl_mode);
        return lib_measure_calc_freq(tm);
    }
}

#define MEASURE_HTOTAL_DIFF_THD 1    //unit: pixel
#define MEASURE_VTOTAL_DIFF_THD 5    //unit: line
#define MEASURE_HFREQ_DIFF_THD 500     //unit: Hz
#define MEASURE_VFREQ_DIFF_THD 500     //unit: 0.001Hz
#define MEASURE_HACT_MIN 300    //unit: pixel
#define MEASURE_VACT_MIN 240     //unit: line

unsigned char newbase_hdmi_dp_measure_video_ex(OFFMS_SOURCE_SEL sel, unsigned char port, MEASURE_TIMING_T *tm, unsigned char frl_mode,unsigned char is_HV_Mode)
{
    unsigned char ms_done = 0;
    MEASURE_TIMING_T tm_cmp;
    memcpy(&tm_cmp, tm, sizeof(MEASURE_TIMING_T));

    newbase_measure_init();
#if defined(CONFIG_ARCH_RTK2885P)
    MEASURE_WARN("offms out sel = %d\n", sel);

    rtd_maskl(COMMON_RX_420_VIDEO_AND_OFFMS_PATH_SEL_reg, ~(COMMON_RX_420_VIDEO_AND_OFFMS_PATH_SEL_offms_out_sel_mask), COMMON_RX_420_VIDEO_AND_OFFMS_PATH_SEL_offms_out_sel(sel));
    switch(sel)
    {
    case OFFMS_SOURCE_HDMI:
    case OFFMS_SOURCE_HDMI21:
        if(frl_mode ==0)
        {
            //ms_src_clock_path = MEASURE_CLOCK_PATH_HDMI20;
            rtd_maskl(COMMON_RX_420_OFFMS_CTS_FIFO_CTRL_reg, ~(COMMON_RX_420_OFFMS_CTS_FIFO_CTRL_offms_en_bypass_mask), COMMON_RX_420_OFFMS_CTS_FIFO_CTRL_offms_en_bypass(0));
        }
        else
        {
            //ms_src_clock_path = MEASURE_CLOCK_PATH_HDMI21;
            rtd_maskl(COMMON_RX_420_OFFMS_CTS_FIFO_CTRL_reg, ~(COMMON_RX_420_OFFMS_CTS_FIFO_CTRL_offms_en_bypass_mask), COMMON_RX_420_OFFMS_CTS_FIFO_CTRL_offms_en_bypass(1));
        }
        break;
    case OFFMS_SOURCE_DPRX:
        //ms_src_clock_path = MEASURE_CLOCK_PATH_DP;
        break;
    default:
        MEASURE_WARN("[newbase_hdmi_dp_measure_video] ERROR, undefined input measure src=%d,",sel);
        break;
    }

#endif
    //MEASURE_WARN("[newbase_hdmims_measure_video port:%d] HVSync Measure Mode, vrr_mode=%d\n",
    //    port, newbase_hdmi_get_vrr_enable(port));

    _lock_offline_semaphore();
    ms_done = lib_hdmi_dp_measure(sel, tm, frl_mode, is_HV_Mode);
    _unlock_offline_semaphore();

    if (!ms_done) { return FALSE; }
    if (tm->h_act_len > tm->h_total) { return FALSE; }
    if (tm->v_act_len > tm->v_total) { return FALSE; }
    if (tm->h_act_len < MEASURE_HACT_MIN) { return FALSE; }
    if (tm->v_act_len < MEASURE_VACT_MIN) { return FALSE; }
#if 0// hdmrx
    if ((0 == GET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_FORCE_VIC_MEASURE))
            && (TRUE == lib_skiptiming_condition(tm))) {
        return FALSE;
    }
#endif
    _lock_offline_semaphore();
    ms_done = lib_hdmi_dp_measure(sel, &tm_cmp, frl_mode, is_HV_Mode);
    _unlock_offline_semaphore();

    if (!ms_done) { return FALSE; }
    if (MEASURE_ABS(tm->h_total, tm_cmp.h_total) > MEASURE_HTOTAL_DIFF_THD) { return FALSE; }
    if (MEASURE_ABS(tm->v_total, tm_cmp.v_total) > MEASURE_VTOTAL_DIFF_THD) { return FALSE; }
    if (tm->h_act_len != tm_cmp.h_act_len) { return FALSE; }
    if (tm->v_act_len != tm_cmp.v_act_len) { return FALSE; }
    if (MEASURE_ABS(tm->h_freq, tm_cmp.h_freq) > MEASURE_HFREQ_DIFF_THD) { return FALSE; }
    if (MEASURE_ABS(tm->v_freq, tm_cmp.v_freq) > MEASURE_VFREQ_DIFF_THD) { return FALSE; }

    return TRUE;
}

EXPORT_SYMBOL(newbase_hdmi_dp_measure_video_ex);

unsigned char newbase_hdmi_dp_measure_video(OFFMS_SOURCE_SEL sel, unsigned char port, MEASURE_TIMING_T *tm, unsigned char frl_mode)
{
    return newbase_hdmi_dp_measure_video_ex(sel, port, tm, frl_mode, TRUE);
}

EXPORT_SYMBOL(newbase_hdmi_dp_measure_video);


/*------------------------------------------------
 * Func : _offms_measure_hvmode
 *
 * Desc : measure timing with hvmode
 *
 * Para : sel   : source to measure
 *        tm    : pointer of timing info (output)
 *        flags : measure control flags
 *
 * Retn : TRUE : successed, FALSE : failed
 *-----------------------------------------------*/
static unsigned char _offms_measure_hvmode(
    OFFMS_SOURCE_SEL    sel,
    MEASURE_TIMING_T*   tm,
    OFFMS_CTRL_FALGS    flags
    )
{
    unsigned int raw_h_sync_width = 0;
    unsigned int raw_v_sync_width = 0;
    unsigned int off_source_sel = 0x8;

#if defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F)
    if (sel == OFFMS_SOURCE_DPRX)
        off_source_sel = 0xA;
#endif

    rtd_maskl(OFFMS_SP_MS3Ctrl_reg, ~(OFFMS_SP_MS3Ctrl_off_meas_source_mask |
                                      OFFMS_SP_MS3Ctrl_off_clk_sel_mask |
                                      OFFMS_SP_MS3Ctrl_off_meas_to_sel_mask |
                                      OFFMS_SP_MS3Ctrl_off_sycms_mode_mask |
                                      OFFMS_SP_MS3Ctrl_off_popup_meas_mask |
                                      OFFMS_SP_MS3Ctrl_off_start_ms_mask),
                                     (OFFMS_SP_MS3Ctrl_off_meas_source(off_source_sel) |
                                      OFFMS_SP_MS3Ctrl_off_clk_sel(1) |
                                      OFFMS_SP_MS3Ctrl_off_meas_to_sel(1) |
                                      OFFMS_SP_MS3Ctrl_off_sycms_mode(ANALOG_MODE_MEASUREMENT) |
                                      OFFMS_SP_MS3Ctrl_get_off_popup_meas(0) |
                                      OFFMS_SP_MS3Ctrl_get_off_start_ms(0)));

    if (!lib_measure_offms_start())
    {
        MEASURE_WARN("OFFMS is not finish.\n");
        return FALSE;
    }

    tm->h_total   = OFFMS_SP_MS3Rst1_get_off_hs_period_out_h12b(rtd_inl(OFFMS_SP_MS3Rst1_reg)) + 1;
    tm->v_total   = OFFMS_SP_MS3Rst0_get_off_vs_period_out(rtd_inl(OFFMS_SP_MS3Rst0_reg)) + 1;
    tm->h_act_sta = OFFMS_MES_RESULT_HSTA_END_get_off_hx_sta(rtd_inl(OFFMS_MES_RESULT_HSTA_END_reg));
    tm->v_act_sta = OFFMS_MES_RESULT_VSTA_END_get_off_vx_sta(rtd_inl(OFFMS_MES_RESULT_VSTA_END_reg));
    tm->h_act_len = OFFMS_MES_RESULT_HSTA_END_get_off_hx_end(rtd_inl(OFFMS_MES_RESULT_HSTA_END_reg)) - tm->h_act_sta;
    tm->v_act_len = OFFMS_MES_RESULT_VSTA_END_get_off_vx_end(rtd_inl(OFFMS_MES_RESULT_VSTA_END_reg)) - tm->v_act_sta + 1;
    tm->polarity  =(OFFMS_SP_MS3Stus_get_off_vs_pol_out(rtd_inl(OFFMS_SP_MS3Stus_reg)) << 1) |
                   OFFMS_SP_MS3Stus_get_off_hs_pol_out(rtd_inl(OFFMS_SP_MS3Stus_reg));

    tm->h_act_sta += 1;//willychou modify it for OFFMS_START_CORRECTION
    tm->v_act_sta -= 1;//willychou modify it for OFFMS_START_CORRECTION

    raw_h_sync_width = OFFMS_SP_MS3Rst2_get_off_hs_high_out(rtd_inl(OFFMS_SP_MS3Rst2_reg)) + 1;
    raw_v_sync_width = OFFMS_SP_MS3Rst2_get_off_vs_high_out(rtd_inl(OFFMS_SP_MS3Rst2_reg)) + 1;

    // Check HS priority
    if (0 == (tm->polarity & _BIT0))
    {
        //Hsync negative polarity
        if (tm->h_total >= raw_h_sync_width)
        {
            tm->h_act_sta += (tm->h_total - raw_h_sync_width);
            tm->IHSyncPulseCount = (tm->h_total - raw_h_sync_width);
        }
        else
        {
            MEASURE_WARN("Warning h_total=%d < raw_h_sync_width=%d\n", tm->h_total, raw_h_sync_width);
        }
    }
    else
    {
        // Hsync positive polarity
        tm->IHSyncPulseCount = raw_h_sync_width;
    }

    // Check VS priority
    if (0 == (tm->polarity & _BIT1))
    {
        //Vsync negative polarity
        if (tm->v_total >= raw_v_sync_width)
        {
            tm->v_act_sta += (tm->v_total - raw_v_sync_width);
            tm->IVSyncPulseCount = (tm->v_total - raw_v_sync_width);
        }
        else
        {
            MEASURE_WARN("Warning v_total=%d < raw_v_sync_width=%d\n", tm->v_total, raw_v_sync_width);
        }
    }
    else
    {
        // Vsync positive polarity
        tm->IVSyncPulseCount = raw_v_sync_width;
    }

    // Source related optimization
    switch(sel)
    {
    case OFFMS_SOURCE_HDMI:
    case OFFMS_SOURCE_HDMI21:

#if MEASURE_FROM_TMDS_CLOCK
        if (tm->pixel_repeat > 0)
        {
            tm->h_act_sta /= (tm->pixel_repeat + 1);
            tm->h_total /= (tm->pixel_repeat + 1);
            tm->h_act_len /= (tm->pixel_repeat + 1);
        }

        if (COLOR_YUV420 == tm->colorspace)
        {
            tm->h_act_sta <<= 1;
            tm->h_total <<= 1;
            tm->h_act_len <<= 1;
        }

        if (HDMI_COLOR_DEPTH_10B == tm->colordepth)
        {
            // value / 1.25
            tm->h_act_sta = ((tm->h_act_sta * 4) + 3) / 5;
            tm->h_total = ((tm->h_total * 4) + 3) / 5;
            tm->h_act_len = ((tm->h_act_len * 4) + 3) / 5;

        }
        else if (HDMI_COLOR_DEPTH_12B == tm->colordepth) {
            // value / 1.5
            tm->h_act_sta = ((tm->h_act_sta * 2) + 2) / 3;
            tm->h_total = ((tm->h_total * 2) + 2) / 3;
            tm->h_act_len = ((tm->h_act_len * 2) + 2) / 3;

        }
        else if (HDMI_COLOR_DEPTH_16B == tm->colordepth) {
            // value / 2
            tm->h_act_sta = tm->h_act_sta >> 1;
            tm->h_total = tm->h_total >> 1;
            tm->h_act_len = tm->h_act_len >> 1;

        }
#endif

        // Check FRL mode
        if (OFFMS_FLAGS_IS_HDMI_FRL_MODE(flags)) // is FRL mode
        {
            //H5X 4 pixel mode
            MEASURE_INFO("[OFMS]H5X 4 Pixel mode!Hx4\n");
            tm->h_act_sta <<= 2;
            tm->h_total <<= 2;
            tm->h_act_len <<= 2;
        }

        // Check FRL mode
        if (OFFMS_FLAGS_IS_HDMI_FRL_MODE(flags) &&
            OFFMS_FLAGS_IS_YUV420_MODE(flags))
        {
            MEASURE_WARN("YUV420, Hx2!\n");
            tm->h_act_sta <<= 1;
            tm->h_total <<= 1;
            tm->h_act_len <<= 1;
        }
        break;

    case OFFMS_SOURCE_DPRX:
    case OFFMS_SOURCE_DSCD:

        // calibrate ofms timing for YUV420
        if (OFFMS_FLAGS_IS_YUV420_MODE(flags))
        {
            MEASURE_WARN("YUV420, Hx2!\n");
            tm->h_total   <<= 1;
            tm->h_act_len <<= 1;
            tm->h_act_sta <<= 1;
        }

        // DPRX pixel mode
        switch (OFFMS_FLAGS_GET_PIXEL_MODE(flags))
        {
        case OFFMS_1_PIXEL_MODE:
            // do nothing...
            break;
        case OFFMS_2_PIXEL_MODE:
            MEASURE_WARN("DPRX 2 Pixel mode!Hx2 (H/V mode)\n");
            tm->h_total <<= 1;
            tm->h_act_len <<= 1;
            break;

        case OFFMS_4_PIXEL_MODE:
            MEASURE_WARN("DPRX 4 Pixel mode!Hx4 (H/V mode)\n");
            tm->h_total <<= 2;
            tm->h_act_len <<= 2;
            break;

        default:
            MEASURE_WARN("DPRX unknow pixel mode %lu\n", OFFMS_FLAGS_GET_PIXEL_MODE(flags));
            break;
        }
        break;

    default:
        break;
    }

    lib_measure_active_correction(&(tm->h_act_len), &(tm->v_act_len));

    MEASURE_WARN("[OFMS] IHTotal: %d\n", tm->h_total);
    MEASURE_WARN("[OFMS] IVTotal: %d\n", tm->v_total);
    MEASURE_WARN("[OFMS] IHAct: %d\n", tm->h_act_len);
    MEASURE_WARN("[OFMS] IVAct: %d\n", tm->v_act_len);
    MEASURE_WARN("[OFMS] IHStr: %d\n", tm->h_act_sta);
    MEASURE_WARN("[OFMS] IVStr: %d\n", tm->v_act_sta);
    MEASURE_WARN("[OFMS] Polarity: %d\n", tm->polarity);
    MEASURE_WARN("[OFMS] IHsyncWidth %d\n", tm->IHSyncPulseCount);
    MEASURE_WARN("[OFMS] IVsyncWidth %d\n", tm->IVSyncPulseCount);
    MEASURE_WARN("[OFMS] pixel mode %lu\n", OFFMS_FLAGS_GET_PIXEL_MODE(flags));
    return TRUE;
}

/*------------------------------------------------
 * Func : _offms_measure_demode
 *
 * Desc : measure timing
 *
 * Para : sel   : source to measure
 *        tm    : pointer of timing info (output)
 *        flags : measure control flags
 *
 * Retn : TRUE : successed, FALSE : failed
 *-----------------------------------------------*/
static unsigned char _offms_measure_demode(
    OFFMS_SOURCE_SEL    sel,
    MEASURE_TIMING_T*   tm,
    OFFMS_CTRL_FALGS    flags
    )
{
#if (OFFMS_SUPPORT_DEMODE)
    unsigned int off_source_sel = 0x8;
    unsigned long long pix_clk = 0;
    unsigned int de_vs_end = 0;
    unsigned int vfreq = 0;

#if defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F)
    if(sel == OFFMS_SOURCE_DPRX)
        off_source_sel = 0xA;
#endif

    memset(tm, 0, sizeof(MEASURE_TIMING_T));

    rtd_maskl(OFFMS_SP_MS3Ctrl_reg, ~(OFFMS_SP_MS3Ctrl_off_meas_source_mask |
                                      OFFMS_SP_MS3Ctrl_off_clk_sel_mask |
                                      OFFMS_SP_MS3Ctrl_off_meas_to_sel_mask |
                                      OFFMS_SP_MS3Ctrl_off_sycms_mode_mask |
                                      OFFMS_SP_MS3Ctrl_off_popup_meas_mask |
                                      OFFMS_SP_MS3Ctrl_off_start_ms_mask),
                                     (OFFMS_SP_MS3Ctrl_off_meas_source(off_source_sel) |
                                      OFFMS_SP_MS3Ctrl_off_clk_sel(1) |
                                      OFFMS_SP_MS3Ctrl_off_meas_to_sel(1) |
                                      OFFMS_SP_MS3Ctrl_off_sycms_mode(DIGITAL_MODE_MEASUREMENT) |
                                      OFFMS_SP_MS3Ctrl_get_off_popup_meas(0) |
                                      OFFMS_SP_MS3Ctrl_get_off_start_ms(0)));

    if (!lib_measure_offms_start())
    {
        MEASURE_WARN("DE Mode, Hdmi_OffLineMeasure is not finish.\n");
        return FALSE;
    }

    tm->h_total = OFFMS_Demode_Hs_period_get_de_hs_period(rtd_inl(OFFMS_Demode_Hs_period_reg));
    //tm->v_total = OFFMS_SP_MS3Rst0_get_off_vs_period_out(rtd_inl(OFFMS_SP_MS3Rst0_reg))+1;
    //tm->h_act_sta = OFFMS_Demode_VsHs_Act_get_de_hs_act(rtd_inl(OFFMS_Demode_VsHs_Act_reg));
    tm->v_act_sta = OFFMS_Demode_Vs_STA_get_de_vs_sta(rtd_inl(OFFMS_Demode_Vs_STA_reg));
    tm->h_act_len =  OFFMS_Demode_VsHs_Act_get_de_hs_act(rtd_inl(OFFMS_Demode_VsHs_Act_reg));
    tm->v_act_len = OFFMS_Demode_VsHs_Act_get_de_vs_act(rtd_inl(OFFMS_Demode_VsHs_Act_reg));
    //tm->polarity = (OFFMS_SP_MS3Stus_get_off_vs_pol_out(rtd_inl(OFFMS_SP_MS3Stus_reg))<<1)|OFFMS_SP_MS3Stus_get_off_hs_pol_out(rtd_inl(OFFMS_SP_MS3Stus_reg));
    tm->polarity = 0x3;  // force positive

    tm->IVSyncPulseCount = 5;//OFFMS_SP_MS3Rst2_get_off_vs_high_out(rtd_inl(OFFMS_SP_MS3Rst2_reg))+1;
    tm->IHSyncPulseCount = 5;//OFFMS_SP_MS3Rst2_get_off_hs_high_out(rtd_inl(OFFMS_SP_MS3Rst2_reg))+1;
    de_vs_end = OFFMS_Demode_Vs_END_get_de_vs_end(rtd_inl(OFFMS_Demode_Vs_END_reg));

    switch(sel)
    {
    case OFFMS_SOURCE_HDMI:
    case OFFMS_SOURCE_HDMI21:

        if (OFFMS_FLAGS_IS_HDMI_FRL_MODE(flags))
        {
            //H5X 4 pixel mode
            MEASURE_WARN("H5X 4 Pixel mode!Hx4\n");
            tm->h_total <<= 2;
            tm->h_act_len <<= 2;
            if (tm->pixel_repeat > 0)
            {
                MEASURE_WARN("Pixel Repeat =%d !H/(pixel_repeat+1)\n", tm->pixel_repeat);
                tm->h_total /= (tm->pixel_repeat + 1);
                tm->h_act_len /= (tm->pixel_repeat + 1);
            }
        }

        if (OFFMS_FLAGS_IS_HDMI_FRL_MODE(flags) && OFFMS_FLAGS_IS_YUV420_MODE(flags))
        {
            MEASURE_WARN("YUV420, Hx2!\n");
            tm->h_total <<= 1;
            tm->h_act_len <<= 1;
        }
        break;

    case OFFMS_SOURCE_DPRX:
    case OFFMS_SOURCE_DSCD:
        // DPRX pixel mode
        switch (OFFMS_FLAGS_GET_PIXEL_MODE(flags))
        {
        case OFFMS_1_PIXEL_MODE:
            // do nothing...
            break;
        case OFFMS_2_PIXEL_MODE:
            MEASURE_WARN("DPRX 2 Pixel mode!Hx2 (H/V mode)\n");
            tm->h_total <<= 1;
            tm->h_act_len <<= 1;
            break;

        case OFFMS_4_PIXEL_MODE:
            MEASURE_WARN("DPRX 4 Pixel mode!Hx4 (H/V mode)\n");
            tm->h_total <<= 2;
            tm->h_act_len <<= 2;
            break;

        default:
            MEASURE_WARN("DPRX unknow pixel mode %lu\n", OFFMS_FLAGS_GET_PIXEL_MODE(flags));
            break;
        }
        break;

    default:
        break;
    }

    // pixclk = (htotal*vact - hporch) * 27 / (de_vs_end - de_vs_star)    // MHZ
    // vfreq = pixclk/vfreq/htotal
    pix_clk = (unsigned long long)tm->h_total * tm->v_act_len *27*1000000/ (de_vs_end - tm->v_act_sta);   //Hz
    vfreq = lib_measure_calc_vfreq(lib_measure_get_vs_period());
    if (vfreq != 0 && tm->h_total != 0)
        tm->v_total = (pix_clk/ tm->h_total)*1000/vfreq;

    //tm->v_act_sta /= tm->h_total ; v_act_pixels = v_act_sta (number crystal clock) * ()pixel_per_sec / crystal_clockhtotal (number of crystal)
    //
    tm->v_act_sta /= ((tm->h_total * tm->v_total)/pix_clk);

    MEASURE_INFO("[OFMS] IHTotal: %d\n", tm->h_total);
    MEASURE_INFO("[OFMS] IHAct: %d\n", tm->h_act_len);
    MEASURE_INFO("[OFMS] IHAct_Frac: %d\n", OFFMS_Demode_VsHs_Act_get_de_hs_act_fraction(rtd_inl(OFFMS_Demode_VsHs_Act_reg)));

    MEASURE_INFO("[OFMS] IVTotal: %d\n", tm->v_total);
    MEASURE_INFO("[OFMS] IVAct: %d\n", tm->v_act_len);
    MEASURE_INFO("[OFMS] IVStr: %d\n", tm->v_act_sta);
    MEASURE_INFO("[OFMS] IVStr_End: %d\n", de_vs_end);
    return TRUE;
#else
    MEASURE_WARN("DE Mode not available, force to HV mode\n");
    return _offms_measure_hvmode(sel, tm, flags);
#endif
}

/*------------------------------------------------
 * Func : _do_offline_measure
 *
 * Desc : measure timing
 *
 * Para : sel   : source to measure
 *        tm    : pointer of timing info (output)
 *        flags : measure control flags
 *
 * Retn : TRUE : successed, FALSE : failed
 *-----------------------------------------------*/
static unsigned char _do_offline_measure(
    OFFMS_SOURCE_SEL    sel,
    MEASURE_TIMING_T*   tm,
    OFFMS_CTRL_FALGS    flags
    )
{
    int ret;

    if (OFFMS_FLAGS_IS_DEMODE(flags))
        ret = _offms_measure_demode(sel, tm, flags);
    else
        ret = _offms_measure_hvmode(sel, tm, flags);

    if (ret != TRUE)
        return ret;

    return lib_measure_calc_freq(tm);
}

/*------------------------------------------------
 * Func : rtk_offms_measure_video
 *
 * Desc : measure timing
 *
 * Para : sel   : source to measure
 *        tm    : pointer of timing info (output)
 *        flags : measure control flags
 *
 * Retn : TRUE : successed, FALSE : failed
 *-----------------------------------------------*/
unsigned char rtk_offms_measure_video(
    OFFMS_SOURCE_SEL    sel,
    MEASURE_TIMING_T*   tm,
    OFFMS_CTRL_FALGS    flags
    )
{
    unsigned char ms_done = 0;
    MEASURE_TIMING_T tm_cmp;
    memcpy(&tm_cmp, tm, sizeof(MEASURE_TIMING_T));

    newbase_measure_init();

#if defined(CONFIG_ARCH_RTK2885P)
    MEASURE_WARN("[%s][L%d]offms out sel = %d\n", __func__, __LINE__, sel);
    rtd_maskl(COMMON_RX_420_VIDEO_AND_OFFMS_PATH_SEL_reg, ~(COMMON_RX_420_VIDEO_AND_OFFMS_PATH_SEL_offms_out_sel_mask), COMMON_RX_420_VIDEO_AND_OFFMS_PATH_SEL_offms_out_sel(sel));
#endif

    //MEASURE_WARN("[newbase_hdmims_measure_video port:%d] HVSync Measure Mode, vrr_mode=%d\n",
    //    port, newbase_hdmi_get_vrr_enable(port));

    _lock_offline_semaphore();
    ms_done = _do_offline_measure(sel, tm, flags);
    _unlock_offline_semaphore();

    if (!ms_done) { return FALSE; }
    if (tm->h_act_len > tm->h_total) { return FALSE; }
    if (tm->v_act_len > tm->v_total) { return FALSE; }
    if (tm->h_act_len < MEASURE_HACT_MIN) { return FALSE; }
    if (tm->v_act_len < MEASURE_VACT_MIN) { return FALSE; }

    _lock_offline_semaphore();
    ms_done = _do_offline_measure(sel, &tm_cmp, flags);
    _unlock_offline_semaphore();

    if (!ms_done) { return FALSE; }
    if (MEASURE_ABS(tm->h_total, tm_cmp.h_total) > MEASURE_HTOTAL_DIFF_THD) { return FALSE; }
    if (MEASURE_ABS(tm->v_total, tm_cmp.v_total) > MEASURE_VTOTAL_DIFF_THD) { return FALSE; }
    if (tm->h_act_len != tm_cmp.h_act_len) { return FALSE; }
    if (tm->v_act_len != tm_cmp.v_act_len) { return FALSE; }
    if (MEASURE_ABS(tm->h_freq, tm_cmp.h_freq) > MEASURE_HFREQ_DIFF_THD) { return FALSE; }
    if (MEASURE_ABS(tm->v_freq, tm_cmp.v_freq) > MEASURE_VFREQ_DIFF_THD) { return FALSE; }

    return TRUE;
}
EXPORT_SYMBOL(rtk_offms_measure_video);

void newbase_measure_init(void)
{
    lib_measure_offms_crt_on();  // enable CRT of OFMS
    MEASURE_WARN("newbase_measure_init!\n");
}
EXPORT_SYMBOL(newbase_measure_init);

void newbase_hdmi_dp_measure_abort(void)
{
    if (measure_start) {
        measure_start = 0;
        MEASURE_WARN("abort timing measure!\n");
    }
}
EXPORT_SYMBOL(newbase_hdmi_dp_measure_abort);

unsigned char lib_measure_offms_start(void)
{
    unsigned int t1;
    offms_sp_ms3ctrl_RBUS ofms_ms3ctrl_reg;

    //_dump_measure_status("before start measure");

    ofms_ms3ctrl_reg.regValue = rtd_inl(OFFMS_SP_MS3Ctrl_reg);

    // measure once.[SOCTSRTK-81586] HDMI basic test fail.
    // when off_online_en is 1, the bit off_start_ms cannot be cleared
    // after the measurement is complete, resulting in OFFMS always failing.
    ofms_ms3ctrl_reg.off_online_en = 0;
    ofms_ms3ctrl_reg.off_start_ms = 1;
    rtd_outl(OFFMS_SP_MS3Ctrl_reg, ofms_ms3ctrl_reg.regValue);

    _dump_measure_status("start measure");

    measure_start = 1;

    t1 = 100;
    do {
        t1 --;
        ofms_ms3ctrl_reg.regValue = rtd_inl(OFFMS_SP_MS3Ctrl_reg);
        if (!ofms_ms3ctrl_reg.off_start_ms) { break; }
        msleep(2);//Minchay@20151029-m speed up performence
    } while (t1 > 0 && measure_start);

#ifdef HDMI_OFFLINE_MEASURE_FIX
    if (t1 == 0) {
        MEASURE_WARN("POP not finish \n");
    }
#else //#ifdef HDMI_OFFLINE_MEASURE_FIX
    if (t1 == 0 || !measure_start) {
        if (measure_start) {
            _dump_measure_status("user meassure abort");
        }
        else {
            measure_not_finish_cnt++;
            _dump_measure_status("wait ofms timeout, reset ofms");
            lib_measure_offms_crt_reset();
        }
        return FALSE;
    }
#endif //#ifdef HDMI_OFFLINE_MEASURE_FIX

    ofms_ms3ctrl_reg.off_popup_meas = 1;
    rtd_outl(OFFMS_SP_MS3Ctrl_reg, ofms_ms3ctrl_reg.regValue);

    t1 = 100;
    do {
        t1 --;
        ofms_ms3ctrl_reg.regValue = rtd_inl(OFFMS_SP_MS3Ctrl_reg);
        if (!ofms_ms3ctrl_reg.off_popup_meas) { break; }
        msleep(2);//Minchay@20151029-m speed up performence
    } while (t1 > 0 && measure_start);

    if (t1 == 0 || !measure_start) {
        if (!measure_start) {
            _dump_measure_status("pop up ofms user meassure abort");
        }
        else {
            measure_not_finish_cnt++;
            _dump_measure_status("pop up ofms timeout, reset ofms");
            lib_measure_offms_crt_reset();  // reset
        }
        return FALSE;
    }

    measure_not_finish_cnt = 0;
    measure_start = 0;
    _dump_measure_status("measure complete");
    return TRUE;
}
#endif // UT_flag


void lib_measure_offms_crt_on(void)
{
    // enable OFMS CRT
    if (!((rtd_inl(SYS_REG_SYS_CLKEN1_reg) & SYS_REG_SYS_CLKEN1_clken_offms_mask) &&
            (rtd_inl(SYS_REG_SYS_SRST1_reg) & SYS_REG_SYS_SRST1_rstn_offms_mask))) {
        MEASURE_WARN("CRT is not enabled, enable OFMS Clock\n");

        rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_offms_mask);
        udelay(5);
        rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_offms_mask);

        udelay(5);

        rtd_outl(SYS_REG_SYS_CLKEN1_reg, (SYS_REG_SYS_CLKEN1_clken_offms_mask | SYS_REG_SYS_CLKEN1_write_data_mask));
        udelay(5);

        rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_offms_mask);
        udelay(5);

        rtd_outl(SYS_REG_SYS_SRST1_reg, (SYS_REG_SYS_SRST1_rstn_offms_mask | SYS_REG_SYS_SRST1_write_data_mask));
        udelay(5);
        rtd_outl(SYS_REG_SYS_CLKEN1_reg, (SYS_REG_SYS_CLKEN1_clken_offms_mask | SYS_REG_SYS_CLKEN1_write_data_mask));
    }

    if ((rtd_inl(SYS_REG_SYS_CLKEN3_reg) & SYS_REG_SYS_CLKEN3_clken_dispi_mask) == 0 ||
            (rtd_inl(SYS_REG_SYS_SRST3_reg) & (SYS_REG_SYS_SRST3_rstn_dispi1_mask | SYS_REG_SYS_SRST3_rstn_dispi2_mask | SYS_REG_SYS_SRST3_rstn_dispi3_mask)) == 0) {
        MEASURE_WARN("DISPI CRT is not enabled, enable DISPPI Clock for OFMS \n");
        rtd_outl(SYS_REG_SYS_CLKEN3_reg, SYS_REG_SYS_CLKEN3_clken_dispi_mask |
                 SYS_REG_SYS_CLKEN3_write_data_mask);

        rtd_outl(SYS_REG_SYS_SRST3_reg,  SYS_REG_SYS_SRST3_rstn_dispi1_mask |
                 SYS_REG_SYS_SRST3_rstn_dispi2_mask |
                 SYS_REG_SYS_SRST3_rstn_dispi3_mask |
                 SYS_REG_SYS_CLKEN3_write_data_mask);
    }
}

void lib_measure_offms_crt_reset(void)
{
    MEASURE_WARN("OFFMS CRT reset !!!!\n");
    rtd_outl(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_offms_mask);   // disable clock
    udelay(5);
    rtd_outl(SYS_REG_SYS_SRST1_reg, SYS_REG_SYS_SRST1_rstn_offms_mask);  // reset crt
    udelay(5);
    lib_measure_offms_crt_on();
}

unsigned int lib_measure_vfreq_prescion_3_to_1(unsigned int original_vfreq)
{
    //Need to remove if Scaler patch has merged.
    unsigned int output_vfreq = 0;

    if ((original_vfreq - (original_vfreq / 1000) * 1000) > 985) {
        //XX.986~XX.999 -> (XX+1).000
        output_vfreq = original_vfreq / 100 + 1;
    }
    else {
        output_vfreq = original_vfreq / 100;
    }

    return output_vfreq;
}
EXPORT_SYMBOL(lib_measure_vfreq_prescion_3_to_1);

unsigned int lib_measure_calc_vfreq(unsigned int vs_period)
{
    // V freq (0.001Hz)
    unsigned long long de_vs_period = (unsigned long long)vs_period;
    unsigned long long temp_vfreq = 0;
    unsigned int vfreq = 0;

    //MEASURE_WARN("[OFMS] input vs_period=%lld \n", de_vs_period);
    if (de_vs_period != 0) {
        temp_vfreq = 27000000ULL * 100ULL * 10ULL;
        do_div(temp_vfreq, de_vs_period);
        vfreq = (unsigned int)temp_vfreq;
    }
    else {
        //MEASURE_WARN("[OFMS][lib_measure_calc_vfreq] Calculate Fail!!Divide 0!! de_vs_period=%lld \n", de_vs_period);
        vfreq = 0;
    }
    //MEASURE_WARN("[OFMS][lib_measure_calc_vfreq] IVFreq: %d (0.001Hz)\n", vfreq);
    return vfreq;
}
EXPORT_SYMBOL(lib_measure_calc_vfreq);

unsigned int lib_measure_get_vs_period(void)
{
#if (OFFMS_SUPPORT_DEMODE)
    return OFFMS_Demode_Vs_period_get_de_vs_period(rtd_inl(OFFMS_Demode_Vs_period_reg));
#else
    return OFFMS_SP_MS3Rst3_off_vs_period_xclk(rtd_inl(OFFMS_SP_MS3Rst3_reg));
#endif
}

EXPORT_SYMBOL(lib_measure_get_vs_period);

unsigned char lib_measure_calc_freq(MEASURE_TIMING_T *tm)
{
    unsigned char result = FALSE;
    unsigned int de_vs_period = lib_measure_get_vs_period();
    unsigned int vfreq = lib_measure_calc_vfreq(de_vs_period);

    MEASURE_WARN("[OFMS][lib_hdmims_calc_freq] vfreq=%d \n", vfreq);
    if (vfreq != 0) {
        tm->v_freq = vfreq;
        tm->h_freq = (tm->v_freq / 100) * tm->v_total / 10;
        result = TRUE;
    }
    else {
        MEASURE_WARN("[OFMS] Measure Fail!!Divide 0!! de_vs_period=%d \n", de_vs_period);
        result = FALSE;
    }

    MEASURE_WARN("[OFMS] IHFreq: %d (Hz)\n", tm->h_freq);
    MEASURE_WARN("[OFMS] IVFreq: %d (0.001Hz)\n", tm->v_freq);

    return result;
}
EXPORT_SYMBOL(lib_measure_calc_freq);

#if HDMI_MEASURE_I3DDMA_SMART_FIT
void lib_hdmims_set_vgip_clk_enable(HDMI_SMT_PATH vgip_ch)
{
    vgip_vgip_chn1_ctrl_RBUS vgip_vgip_chn1_ctrl_reg;
#ifdef CONFIG_DUAL_CHANNEL
    sub_vgip_vgip_chn2_ctrl_RBUS sub_vgip_vgip_chn2_ctrl_reg;
#endif
    dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_dma_vgip_ctrl_reg;

    switch (vgip_ch) {
        case HDMI_SMT_PATH_MAIN_VGIP:
            // Disable SRC_Clock_Enbale bit
            vgip_vgip_chn1_ctrl_reg.regValue = rtd_inl(VGIP_VGIP_CHN1_CTRL_reg);
            vgip_vgip_chn1_ctrl_reg.ch1_in_clk_en = _DISABLE; // clear previous setting
            rtd_outl(VGIP_VGIP_CHN1_CTRL_reg, vgip_vgip_chn1_ctrl_reg.regValue);

            // 2nd determine the input source
            vgip_vgip_chn1_ctrl_reg.regValue = rtd_inl(VGIP_VGIP_CHN1_CTRL_reg);
            vgip_vgip_chn1_ctrl_reg.ch1_in_sel = 0; //HDMI
            vgip_vgip_chn1_ctrl_reg.ch1_digital_mode = 1; //digital mode
            rtd_outl(VGIP_VGIP_CHN1_CTRL_reg, vgip_vgip_chn1_ctrl_reg.regValue);

            // 3rd enable SRC_Clock_Enbale bit
            vgip_vgip_chn1_ctrl_reg.regValue = rtd_inl(VGIP_VGIP_CHN1_CTRL_reg);
            vgip_vgip_chn1_ctrl_reg.ch1_in_clk_en = _ENABLE; //enable clock
            rtd_outl(VGIP_VGIP_CHN1_CTRL_reg, vgip_vgip_chn1_ctrl_reg.regValue);

            vgip_vgip_chn1_ctrl_reg.regValue = rtd_inl(VGIP_VGIP_CHN1_CTRL_reg);
            vgip_vgip_chn1_ctrl_reg.ch1_ivrun = _ENABLE; //Input VGIP run enable
            rtd_outl(VGIP_VGIP_CHN1_CTRL_reg, vgip_vgip_chn1_ctrl_reg.regValue);

            MEASURE_NOTICE("[HDMI] measure use VGIP1\n");
            break;

#ifdef CONFIG_DUAL_CHANNEL
        case HDMI_SMT_PATH_SUB_VGIP:
            // Disable SRC_Clock_Enbale bit
            sub_vgip_vgip_chn2_ctrl_reg.regValue = rtd_inl(SUB_VGIP_VGIP_CHN2_CTRL_reg);
            sub_vgip_vgip_chn2_ctrl_reg.ch2_in_clk_en = _DISABLE;
            rtd_outl(SUB_VGIP_VGIP_CHN2_CTRL_reg, sub_vgip_vgip_chn2_ctrl_reg.regValue);

            // 2nd determine the input source
            sub_vgip_vgip_chn2_ctrl_reg.regValue = rtd_inl(SUB_VGIP_VGIP_CHN2_CTRL_reg);
            sub_vgip_vgip_chn2_ctrl_reg.ch2_in_sel = 0; //HDMI
            sub_vgip_vgip_chn2_ctrl_reg.ch2_digital_mode = 1; //digital mode
            rtd_outl(SUB_VGIP_VGIP_CHN2_CTRL_reg, sub_vgip_vgip_chn2_ctrl_reg.regValue);

            // 3rd enable SRC_Clock_Enbale bit
            sub_vgip_vgip_chn2_ctrl_reg.regValue = rtd_inl(SUB_VGIP_VGIP_CHN2_CTRL_reg);
            sub_vgip_vgip_chn2_ctrl_reg.ch2_in_clk_en = _ENABLE;
            rtd_outl(SUB_VGIP_VGIP_CHN2_CTRL_reg, sub_vgip_vgip_chn2_ctrl_reg.regValue);

            sub_vgip_vgip_chn2_ctrl_reg.regValue = rtd_inl(SUB_VGIP_VGIP_CHN2_CTRL_reg);
            sub_vgip_vgip_chn2_ctrl_reg.ch2_ivrun = _ENABLE; //Input VGIP run enable
            rtd_outl(SUB_VGIP_VGIP_CHN2_CTRL_reg, sub_vgip_vgip_chn2_ctrl_reg.regValue);

            MEASURE_NOTICE("[HDMI] measure use VGIP2\n");
            break;
#endif

        case HDMI_SMT_PATH_DMA_VGIP:
            // Disable SRC_Clock_Enbale bit
            dma_vgip_dma_vgip_ctrl_reg.regValue = rtd_inl(DMA_VGIP_DMA_VGIP_CTRL_reg);
            dma_vgip_dma_vgip_ctrl_reg.dma_in_clk_en = _DISABLE;
            rtd_outl(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_dma_vgip_ctrl_reg.regValue);

            // 2nd determine the input source
            dma_vgip_dma_vgip_ctrl_reg.regValue = rtd_inl(DMA_VGIP_DMA_VGIP_CTRL_reg);
            dma_vgip_dma_vgip_ctrl_reg.dma_in_sel = 0; //HDMI
            dma_vgip_dma_vgip_ctrl_reg.dma_digital_mode = 1; //digital mode
            rtd_outl(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_dma_vgip_ctrl_reg.regValue);

            // 3rd enable SRC_Clock_Enbale bit
            dma_vgip_dma_vgip_ctrl_reg.regValue = rtd_inl(DMA_VGIP_DMA_VGIP_CTRL_reg);
            dma_vgip_dma_vgip_ctrl_reg.dma_in_clk_en = _ENABLE;
            rtd_outl(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_dma_vgip_ctrl_reg.regValue);

            dma_vgip_dma_vgip_ctrl_reg.regValue = rtd_inl(DMA_VGIP_DMA_VGIP_CTRL_reg);
            dma_vgip_dma_vgip_ctrl_reg.dma_ivrun = _ENABLE; //Input VGIP run enable
            rtd_outl(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_dma_vgip_ctrl_reg.regValue);

            MEASURE_NOTICE("[HDMI] measure use DMA-VGIP\n");
            break;

        default:
            MEASURE_EMG("[HDMI] measure VGIP Err=%d\n", vgip_ch);
            break;
    }
}
#else //#if HDMI_MEASURE_I3DDMA_SMART_FIT
void lib_hdmims_set_vgip_clk_enable(unsigned char vgip_ch)
{

    if (0 == vgip_ch) { //vgip 1
        // Disable SRC_Clock_Enbale bit
        rtd_maskl(VGIP_VGIP_CHN1_CTRL_reg, ~(VGIP_VGIP_CHN1_CTRL_ch1_in_clk_en_mask), 0); // clear previous setting
        // 2nd determine the input source
        rtd_maskl(VGIP_VGIP_CHN1_CTRL_reg, ~(VGIP_VGIP_CHN1_CTRL_ch1_in_sel_mask | VGIP_VGIP_CHN1_CTRL_ch1_digital_mode_mask), VGIP_VGIP_CHN1_CTRL_ch1_digital_mode_mask); //set analog mode
        // 3rd enable SRC_Clock_Enbale bit
        rtd_maskl(VGIP_VGIP_CHN1_CTRL_reg, ~(VGIP_VGIP_CHN1_CTRL_ch1_in_clk_en_mask), VGIP_VGIP_CHN1_CTRL_ch1_in_clk_en_mask); // enable clock
        rtd_maskl(VGIP_VGIP_CHN1_CTRL_reg, ~(VGIP_VGIP_CHN1_CTRL_ch1_ivrun_mask), VGIP_VGIP_CHN1_CTRL_ch1_ivrun_mask); // enable clock
        MEASURE_NOTICE("[HDMI] measure use VGIP1\n");

    }
    else {      // if ( _CHANNEL2 == channel )
        // Disable SRC_Clock_Enbale bit
        rtd_maskl(SUB_VGIP_VGIP_CHN2_CTRL_reg, ~(SUB_VGIP_VGIP_CHN2_CTRL_ch2_in_clk_en_mask), 0);
        // 2nd determine the input source
        rtd_maskl(SUB_VGIP_VGIP_CHN2_CTRL_reg, ~(SUB_VGIP_VGIP_CHN2_CTRL_ch2_in_sel_mask | SUB_VGIP_VGIP_CHN2_CTRL_ch2_digital_mode_mask), SUB_VGIP_VGIP_CHN2_CTRL_ch2_digital_mode_mask);
        // 3rd enable SRC_Clock_Enbale bit
        rtd_maskl(SUB_VGIP_VGIP_CHN2_CTRL_reg, ~(SUB_VGIP_VGIP_CHN2_CTRL_ch2_in_clk_en_mask), SUB_VGIP_VGIP_CHN2_CTRL_ch2_in_clk_en_mask);
        rtd_maskl(SUB_VGIP_VGIP_CHN2_CTRL_reg, ~(SUB_VGIP_VGIP_CHN2_CTRL_ch2_ivrun_mask), SUB_VGIP_VGIP_CHN2_CTRL_ch2_ivrun_mask);
        MEASURE_NOTICE("[HDMI] measure use VGIP2\n");
    }

}
#endif //#if HDMI_MEASURE_I3DDMA_SMART_FIT
#ifndef UT_flag
#if HDMI_MEASURE_I3DDMA_SMART_FIT
unsigned char lib_hdmims_get_hvstart(MEASURE_TIMING_T *tm, unsigned char vsc_ch)
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

    vgip_vgip_chn1_ctrl_reg.regValue = rtd_inl(VGIP_VGIP_CHN1_CTRL_reg);
#ifdef CONFIG_DUAL_CHANNEL
    sub_vgip_vgip_chn2_ctrl_reg.regValue = rtd_inl(SUB_VGIP_VGIP_CHN2_CTRL_reg);
#endif

    if (SLR_MAIN_DISPLAY == vsc_ch) {
#ifdef CONFIG_FORCE_RUN_I3DDMA
        dma_vgip_dma_vgip_ctrl_reg.regValue = rtd_inl(DMA_VGIP_DMA_VGIP_CTRL_reg);

        if (no_polarity_inverse_flag) {
            dma_vgip_dma_vgip_ctrl_reg.dma_vs_inv = 0;
            dma_vgip_dma_vgip_ctrl_reg.dma_hs_inv = 0;
        }
        else { //if(_FALSE == no_polarity_inverse_flag)
            if ((tm->polarity >> 1) & 0x01) {
                dma_vgip_dma_vgip_ctrl_reg.dma_vs_inv = 0;
            }
            else {
                dma_vgip_dma_vgip_ctrl_reg.dma_vs_inv = 1;
            }

            if ((tm->polarity) & 0x01) {
                dma_vgip_dma_vgip_ctrl_reg.dma_hs_inv = 0;
            }
            else {
                dma_vgip_dma_vgip_ctrl_reg.dma_hs_inv = 1;
            }
        }

        vgip_ch = HDMI_SMT_PATH_DMA_VGIP;
        rtd_outl(DMA_VGIP_DMA_VGIP_CTRL_reg, dma_vgip_dma_vgip_ctrl_reg.regValue);
#else //#ifdef CONFIG_FORCE_RUN_I3DDMA

        if (no_polarity_inverse_flag) {
            vgip_vgip_chn1_ctrl_reg.ch1_vs_inv = 0;
            vgip_vgip_chn1_ctrl_reg.ch1_hs_inv = 0;
        }
        else { // if(_FALSE == no_polarity_inverse_flag)
            if ((tm->polarity >> 1) & 0x01) {
                vgip_vgip_chn1_ctrl_reg.ch1_vs_inv = 0;
            }
            else {
                vgip_vgip_chn1_ctrl_reg.ch1_vs_inv = 1;
            }

            if ((tm->polarity) & 0x01) {
                vgip_vgip_chn1_ctrl_reg.ch1_hs_inv = 0;
            }
            else {
                vgip_vgip_chn1_ctrl_reg.ch1_hs_inv = 1;
            }
        }

        vgip_ch = HDMI_SMT_PATH_MAIN_VGIP;
        rtd_outl(VGIP_VGIP_CHN1_CTRL_reg, vgip_vgip_chn1_ctrl_reg.regValue);
#endif //#ifdef CONFIG_FORCE_RUN_I3DDMA
    }
#ifdef CONFIG_DUAL_CHANNEL
    else if (SLR_SUB_DISPLAY == vsc_ch) {
        if (no_polarity_inverse_flag) {
            sub_vgip_vgip_chn2_ctrl_reg.ch2_vs_inv = 0;
            sub_vgip_vgip_chn2_ctrl_reg.ch2_hs_inv = 0;
        }
        else { //if(_FALSE == no_polarity_inverse_flag)
            if ((tm->polarity >> 1) & 0x01) {
                sub_vgip_vgip_chn2_ctrl_reg.ch2_vs_inv = 0;
            }
            else {
                sub_vgip_vgip_chn2_ctrl_reg.ch2_vs_inv = 1;
            }

            if ((tm->polarity) & 0x01) {
                sub_vgip_vgip_chn2_ctrl_reg.ch2_hs_inv = 0;
            }
            else {
                sub_vgip_vgip_chn2_ctrl_reg.ch2_hs_inv = 1;
            }
        }

        vgip_ch = HDMI_SMT_PATH_SUB_VGIP;
        rtd_outl(SUB_VGIP_VGIP_CHN2_CTRL_reg, sub_vgip_vgip_chn2_ctrl_reg.regValue);
    }
#endif
    else {
        MEASURE_EMG("NOT valid HDMI VGIP path!!\n");
        return FALSE;
    }

    SCALERDRV_WAIT_VS();//willychou add for polarity change

    lib_hdmims_set_vgip_clk_enable(vgip_ch);

#ifdef CONFIG_FORCE_RUN_I3DDMA
    // setup VGIP capture for smartfit measure
    if (HDMI_SMT_PATH_DMA_VGIP == vgip_ch) {
        dma_vgip_dma_vgip_act_hsta_width_reg.regValue = rtd_inl(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg);
        dma_vgip_dma_vgip_act_hsta_width_reg.dma_ih_act_sta = 0;
        dma_vgip_dma_vgip_act_hsta_width_reg.dma_ih_act_wid = 300;
#ifdef CONFIG_DUAL_CHANNEL
        rtd_outl(DMA_VGIP_DMA_VGIP_ACT_HSTA_Width_reg, sub_vgip_vgip_chn2_act_hsta_width_reg.regValue);
#endif
        dma_vgip_dma_vgip_act_vsta_length_reg.regValue = rtd_inl(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg);
        dma_vgip_dma_vgip_act_vsta_length_reg.dma_iv_act_sta = 0;
        dma_vgip_dma_vgip_act_vsta_length_reg.dma_iv_act_len = 120;
        rtd_outl(DMA_VGIP_DMA_VGIP_ACT_VSTA_Length_reg, dma_vgip_dma_vgip_act_vsta_length_reg.regValue);
    }
#else //#ifdef CONFIG_FORCE_RUN_I3DDMA
    // setup VGIP capture for smartfit measure
    if (HDMI_SMT_PATH_MAIN_VGIP == vgip_ch) {
        vgip_vgip_chn1_act_hsta_width_reg.regValue = rtd_inl(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg);
        vgip_vgip_chn1_act_hsta_width_reg.ch1_ih_act_sta = 0;
        vgip_vgip_chn1_act_hsta_width_reg.ch1_ih_act_wid = 300;
        rtd_outl(VGIP_VGIP_CHN1_ACT_HSTA_Width_reg, vgip_vgip_chn1_act_hsta_width_reg.regValue);

        vgip_vgip_chn1_act_vsta_length_reg.regValue = rtd_inl(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg);
        vgip_vgip_chn1_act_vsta_length_reg.ch1_iv_act_sta = 0;
        vgip_vgip_chn1_act_vsta_length_reg.ch1_iv_act_len = 120;
        rtd_outl(VGIP_VGIP_CHN1_ACT_VSTA_Length_reg, vgip_vgip_chn1_act_vsta_length_reg.regValue);
    }
#endif //#ifdef CONFIG_FORCE_RUN_I3DDMA
#ifdef CONFIG_DUAL_CHANNEL

    else if (HDMI_SMT_PATH_SUB_VGIP == vgip_ch) {
        sub_vgip_vgip_chn2_act_hsta_width_reg.regValue = rtd_inl(SUB_VGIP_VGIP_CHN2_ACT_HSTA_Width_reg);
        sub_vgip_vgip_chn2_act_hsta_width_reg.ch2_ih_act_sta = 0;
        sub_vgip_vgip_chn2_act_hsta_width_reg.ch2_ih_act_wid = 300;
        rtd_outl(SUB_VGIP_VGIP_CHN2_ACT_HSTA_Width_reg, sub_vgip_vgip_chn2_act_hsta_width_reg.regValue);

        sub_vgip_vgip_chn2_act_vsta_length_reg.regValue = rtd_inl(SUB_VGIP_VGIP_CHN2_ACT_VSTA_Length_reg);
        sub_vgip_vgip_chn2_act_vsta_length_reg.ch2_iv_act_sta = 0;
        sub_vgip_vgip_chn2_act_vsta_length_reg.ch2_iv_act_sta = 120;
        rtd_outl(SUB_VGIP_VGIP_CHN2_ACT_VSTA_Length_reg, sub_vgip_vgip_chn2_act_vsta_length_reg.regValue);
    }
#endif
    else {
        MEASURE_EMG("NOT valid HDMI VGIP path!!\n");
        return _FALSE;
    }

    //disable smart fit
    rtd_outl(GET_SFT_AUTO_ADJ_VADDR(), 0x0);
    rtd_outl(GET_SFT_AUTO_ADJ_VADDR(), SMARTFIT_AUTO_ADJ_m_vgen_en_mask);

    // disable smartfit auto field selection
    rtd_maskl(GET_SFT_AUTO_FIELD_VADDR(), ~(SMARTFIT_AUTO_FIELD_odd_sel_en_mask | SMARTFIT_AUTO_FIELD_odd_sel_mask), 0);
    rtd_maskl(GET_SFT_AUTO_FIELD_VADDR(), ~(SMARTFIT_AUTO_FIELD_adj_source_mask), SMARTFIT_AUTO_FIELD_adj_source(vgip_ch));
    rtd_maskl(GET_SFT_AUTO_FIELD_VADDR(), ~(SMARTFIT_AUTO_FIELD_smartfit_src_sel_mask), SMARTFIT_AUTO_FIELD_smartfit_src_sel_mask);//forg mango big ball
    rtd_maskl(GET_SFT_AUTO_FIELD_VADDR(), ~(SMARTFIT_AUTO_FIELD_no_hs_mode_mask), 1);
    rtd_maskl(GET_SFT_AUTO_ADJ_VADDR(), ~(SMARTFIT_AUTO_ADJ_m_vgen_en_mask), 0);

    // set Smartfit larger active region
    rtd_outl(GET_SFT_AUTO_H_BOUNDARY_VADDR(), SMARTFIT_AUTO_H_BOUNDARY_hb_sta(0) |  SMARTFIT_AUTO_H_BOUNDARY_hb_end(5000));
    rtd_outl(GET_SFT_AUTO_V_BOUNDARY_VADDR(), SMARTFIT_AUTO_V_BOUNDARY_vb_sta(0) | SMARTFIT_AUTO_V_BOUNDARY_vb_end(3000));

    rtd_maskl(GET_SFT_AUTO_ADJ_VADDR(), ~(SMARTFIT_AUTO_ADJ_now_af_mask), SMARTFIT_AUTO_ADJ_now_af_mask);  //20140923 fix : AC on 1st measure will fail

    timeoutcnt = 30; // 3d need more time?!
    do {
        timeoutcnt --;
        if (!SMARTFIT_AUTO_ADJ_get_now_af(rtd_inl(GET_SFT_AUTO_ADJ_VADDR()))) {
            break;
        }
        msleep(5);
    } while (timeoutcnt > 0);

    if (0 == timeoutcnt) {
        MEASURE_EMG("[SFT][ERR] %s is not finish.\n", __func__);
        rtd_outl(GET_SFT_AUTO_FIELD_VADDR(), 0x0);
        return FALSE;
    }

    tm->v_act_sta = ((rtd_inl(GET_SFT_AUTO_RESULT_VSTA_END_VADDR()) >> 16) & 0xFFF) - 1;
    tm->h_act_sta = ((rtd_inl(GET_SFT_AUTO_RESULT_HSTA_END_VADDR()) >> 16) & 0xFFF) - 3;

    if (tm->is_interlace) {
        if ((tm->v_total & 0x01)) { //  ODD field
            tm->v_act_sta--;
        }
    }

    rtd_outl(GET_SFT_AUTO_FIELD_VADDR(), 0x0);

    MEASURE_NOTICE("[SFT] IHStr: %d\n", tm->h_act_sta);
    MEASURE_NOTICE("[SFT] IVStr: %d\n", tm->v_act_sta);

    return TRUE;
}
#else //#if HDMI_MEASURE_I3DDMA_SMART_FIT
unsigned char lib_hdmims_get_hvstart(MEASURE_TIMING_T *tm, unsigned char vsc_ch)
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
    vgip_vgip_chn1_ctrl_reg.regValue = rtd_inl(VGIP_VGIP_CHN1_CTRL_reg);

#ifdef CONFIG_DUAL_CHANNEL
    sub_vgip_vgip_chn2_ctrl_reg.regValue = rtd_inl(SUB_VGIP_VGIP_CHN2_CTRL_reg);
#endif

    if (0 == vgip_vgip_chn1_ctrl_reg.ch1_in_sel) {
        if (no_polarity_inverse_flag) {
            vgip_vgip_chn1_ctrl_reg.ch1_vs_inv = 0;
            vgip_vgip_chn1_ctrl_reg.ch1_hs_inv = 0;
        }
        else { // if(_FALSE == no_polarity_inverse_flag)
            if ((tm->polarity >> 1) & 0x01) {
                vgip_vgip_chn1_ctrl_reg.ch1_vs_inv = 0;
            }
            else {
                vgip_vgip_chn1_ctrl_reg.ch1_vs_inv = 1;
            }

            if ((tm->polarity) & 0x01) {
                vgip_vgip_chn1_ctrl_reg.ch1_hs_inv = 0;
            }
            else {
                vgip_vgip_chn1_ctrl_reg.ch1_hs_inv = 1;
            }
        }
        vgip_ch = 0;

        rtd_outl(VGIP_VGIP_CHN1_CTRL_reg, vgip_vgip_chn1_ctrl_reg.regValue);
    }
#ifdef CONFIG_DUAL_CHANNEL
    else if (0 == sub_vgip_vgip_chn2_ctrl_reg.ch2_in_sel) {
        if (no_polarity_inverse_flag) {
            sub_vgip_vgip_chn2_ctrl_reg.ch2_vs_inv = 0;
            sub_vgip_vgip_chn2_ctrl_reg.ch2_hs_inv = 0;
        }
        else { //if(_FALSE == no_polarity_inverse_flag)
            if ((tm->polarity >> 1) & 0x01) {
                sub_vgip_vgip_chn2_ctrl_reg.ch2_vs_inv = 0;
            }
            else {
                sub_vgip_vgip_chn2_ctrl_reg.ch2_vs_inv = 1;
            }

            if ((tm->polarity) & 0x01) {
                sub_vgip_vgip_chn2_ctrl_reg.ch2_hs_inv = 0;
            }
            else {
                sub_vgip_vgip_chn2_ctrl_reg.ch2_hs_inv = 1;
            }
        }

        vgip_ch = 1;
        rtd_outl(SUB_VGIP_VGIP_CHN2_CTRL_reg, sub_vgip_vgip_chn2_ctrl_reg.regValue);
    }
#endif
    else {
        MEASURE_EMG("NOT valid HDMI VGIP path!!\n");
        return FALSE;
    }
#else //#if 1
    if (((tm->polarity >> 1) & 0x01) || no_polarity_inverse_flag) { rtd_maskl(GET_VGIP_CHNx_CTRL_ADDR(), ~(VGIP_VGIP_CHN1_CTRL_ch1_vs_inv_mask), 0); }
    else { rtd_maskl(GET_VGIP_CHNx_CTRL_ADDR(), ~(VGIP_VGIP_CHN1_CTRL_ch1_vs_inv_mask), VGIP_VGIP_CHN1_CTRL_ch1_vs_inv_mask); }

    if (((tm->polarity) & 0x01) || no_polarity_inverse_flag) { rtd_maskl(GET_VGIP_CHNx_CTRL_ADDR(), ~(VGIP_VGIP_CHN1_CTRL_ch1_hs_inv_mask), 0); }
    else { rtd_maskl(GET_VGIP_CHNx_CTRL_ADDR(), ~(VGIP_VGIP_CHN1_CTRL_ch1_hs_inv_mask), VGIP_VGIP_CHN1_CTRL_ch1_hs_inv_mask); }
#endif //#if 1

    SCALERDRV_WAIT_VS();//willychou add for polarity change

    lib_hdmims_set_vgip_clk_enable(vgip_ch);

    // setup VGIP capture for smartfit measure
    rtd_outl(GET_VGIP_CHNx_V_CAP_ADDR(), (0 << 16) | (120));
    rtd_outl(GET_VGIP_CHNx_H_CAP_ADDR(), (0 << 16) | (300));

    //disable smart fit
    rtd_outl(GET_SFT_AUTO_ADJ_VADDR(), 0x0);
    rtd_outl(GET_SFT_AUTO_ADJ_VADDR(), SMARTFIT_AUTO_ADJ_m_vgen_en_mask);

    // disable smartfit auto field selection
    rtd_maskl(GET_SFT_AUTO_FIELD_VADDR(), ~(SMARTFIT_AUTO_FIELD_odd_sel_en_mask | SMARTFIT_AUTO_FIELD_odd_sel_mask), 0);
    rtd_maskl(GET_SFT_AUTO_FIELD_VADDR(), ~(SMARTFIT_AUTO_FIELD_adj_source_mask), SMARTFIT_AUTO_FIELD_adj_source(vgip_ch));
    rtd_maskl(GET_SFT_AUTO_FIELD_VADDR(), ~(SMARTFIT_AUTO_FIELD_smartfit_src_sel_mask), SMARTFIT_AUTO_FIELD_smartfit_src_sel_mask);//forg mango big ball

    rtd_maskl(GET_SFT_AUTO_FIELD_VADDR(), ~(SMARTFIT_AUTO_FIELD_no_hs_mode_mask), 1);
    rtd_maskl(GET_SFT_AUTO_ADJ_VADDR(), ~(SMARTFIT_AUTO_ADJ_m_vgen_en_mask), 0);

    // set Smartfit larger active region
    rtd_outl(GET_SFT_AUTO_H_BOUNDARY_VADDR(), SMARTFIT_AUTO_H_BOUNDARY_hb_sta(0) |  SMARTFIT_AUTO_H_BOUNDARY_hb_end(5000));
    rtd_outl(GET_SFT_AUTO_V_BOUNDARY_VADDR(), SMARTFIT_AUTO_V_BOUNDARY_vb_sta(0) | SMARTFIT_AUTO_V_BOUNDARY_vb_end(3000));

    rtd_maskl(GET_SFT_AUTO_ADJ_VADDR(), ~(SMARTFIT_AUTO_ADJ_now_af_mask), SMARTFIT_AUTO_ADJ_now_af_mask);  //20140923 fix : AC on 1st measure will fail

    timeoutcnt = 30; // 3d need more time?!
    do {
        timeoutcnt --;
        if (!SMARTFIT_AUTO_ADJ_get_now_af(rtd_inl(GET_SFT_AUTO_ADJ_VADDR()))) {
            break;
        }
        msleep(5);
    } while (timeoutcnt > 0);

    if (0 == timeoutcnt) {
        MEASURE_EMG("[SFT][ERR] %s is not finish.\n", __func__);
        rtd_outl(GET_SFT_AUTO_FIELD_VADDR(), 0x0);
        return FALSE;
    }

    tm->v_act_sta = ((rtd_inl(GET_SFT_AUTO_RESULT_VSTA_END_VADDR()) >> 16) & 0xFFF) - 1;
    tm->h_act_sta = ((rtd_inl(GET_SFT_AUTO_RESULT_HSTA_END_VADDR()) >> 16) & 0xFFF) - 3;

    if (tm->is_interlace) {
        if ((tm->v_total & 0x01)) { //  ODD field
            tm->v_act_sta--;
        }
    }

    rtd_outl(GET_SFT_AUTO_FIELD_VADDR(), 0x0);

    MEASURE_NOTICE("[SFT] IHStr: %d\n", tm->h_act_sta);
    MEASURE_NOTICE("[SFT] IVStr: %d\n", tm->v_act_sta);

    return TRUE;
}
#endif //#if HDMI_MEASURE_I3DDMA_SMART_FIT
EXPORT_SYMBOL(lib_hdmims_get_hvstart);
#endif // UT_flag
//USER:LewisLee DATE:2015/11/09
//Fix error timing let picture abnormal
unsigned char lib_skiptiming_condition(MEASURE_TIMING_T *timing)
{
    //WOSQRTK-9205, HDD Player, APPLE TV
    // 1832x540 progressive NG,
    if (MEASURE_ABS(timing->h_act_len, 1832) < 10) {
        if (MEASURE_ABS(timing->v_act_len, 540) < 10) {
            if (0 == timing->is_interlace) {
                MEASURE_NOTICE("SkipTiming h_act_len=%d, v_act_len=%d, v_total=%d, h_total=%d\n", timing->h_act_len, timing->v_act_len, timing->v_total, timing->h_total);
                return TRUE;
            }
        }
    }

    //WOSQRTK-9205, HDD Player, APPLE TV
    // 1280x716 NG
    if (MEASURE_ABS(timing->h_act_len, 1280) < 5) {
        if (MEASURE_ABS(timing->v_act_len, 716) < 2) {
            if (MEASURE_ABS(timing->v_freq, 501) < 2) {
                MEASURE_NOTICE("SkipTiming h_act_len=%d, v_act_len=%d, v_total=%d, h_total=%d\n", timing->h_act_len, timing->v_act_len, timing->v_total, timing->h_total);
                return TRUE;
            }
        }
    }

    //WOSQRTK-9205, HDD Player, APPLE TV
    // 1360x766x60 / 1360x760x60 1360x477x60 / 1360x764x60 NG
    if (MEASURE_ABS(timing->h_act_len, 1360) < 5) {
        if (timing->v_act_len <= 766) {
            if (MEASURE_ABS(timing->v_freq, 600) < 2) {
                MEASURE_NOTICE("SkipTiming h_act_len=%d, v_act_len=%d, v_total=%d, h_total=%d\n", timing->h_act_len, timing->v_act_len, timing->v_total, timing->h_total);
                return TRUE;
            }
        }
    }

    //WOSQRTK-9306, [40000] Radeon R9 390 OC D5 8GB Tri-X NITRO - SAPPHIRE
    // 2016x2160x57.7Hz progressive NG,
    // if DVI=> HDMI, it will have 1984x2160x57.7 Hz
    if (MEASURE_ABS(timing->v_act_len, 2160) < 10) {
        if (timing->h_act_len <= 3200) {
            MEASURE_NOTICE("SkipTiming h_act_len=%d, v_act_len=%d, v_total=%d, h_total=%d\n", timing->h_act_len, timing->v_act_len, timing->v_total, timing->h_total);
            return TRUE;
        }
    }

    /*
        if(MEASURE_ABS(timing->h_act_len, 1082) < 10)
        {
            if(MEASURE_ABS(timing->v_act_len, 499) < 10)
            {
                //BH9520, 1082x499 error timing
                MEASURE_NOTICE("SkipTiming h_act_len=%d, v_act_len=%d, v_total=%d, h_total=%d\n", timing->h_act_len, timing->v_act_len, timing->v_total, timing->h_total);
                return TRUE;
            }
        }
    */
    /*
        if(MEASURE_ABS(timing->v_act_len, 1080) < 10)
        {
            if(timing->h_act_len < 800)
            {
                // MSPG 7100 / 7600, change timing too fast, have short time 760x1080 error timing
                MEASURE_NOTICE("SkipTiming h_act_len=%d, v_act_len=%d, v_total=%d, h_total=%d\n", timing->h_act_len, timing->v_act_len, timing->v_total, timing->h_total);
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
                if(MEASURE_ABS(timing->h_act_len, 1920) <= 5)
                {
                    if(ucSkip_1080P_Cnt < 2 )
                    {
                        MEASURE_NOTICE("SkipTiming I, H=%d, V=%d, cnt=%d\n", timing->h_act_len, timing->v_act_len, ucSkip_1080P_Cnt);
                        ucSkip_1080P_Cnt++;
                        return TRUE;
                    }
                }
                else// if(MEASURE_ABS(timing->h_act_len, 1920) > 5)
                {
                    ucSkip_1080P_Cnt = 0;
                    MEASURE_NOTICE("SkipTiming II, H=%d, V=%d, cnt=%d\n", timing->h_act_len, timing->v_act_len, ucSkip_1080P_Cnt);
                    return TRUE;
                }
            }
        }
    */
    /*
        //blu-ray player, bd-c6900
        if(timing->h_act_len <= timing->v_act_len)
        {
            MEASURE_NOTICE("Abnormal Timing h_act_len=%d, v_act_len=%d, v_total=%d, h_total=%d\n", timing->h_act_len, timing->v_act_len, timing->v_total, timing->h_total);

            if(timing->h_act_len <= 600)
            {
                MEASURE_NOTICE("SkipTiming\n");
                return TRUE;
            }

            //Minchay@21060117#WOSQRTK-5224 Invalid Format - 960*2160 issue
            if((timing->v_act_len >= 2150)&& (timing->v_act_len <= 2170))
            {//v:2160
                if(timing->h_act_len <= 1500)
                {//h <= 1500
                    MEASURE_NOTICE("SkipTiming\n");
                    return TRUE;
                }
            }
        }
    */
    return FALSE;
}

MODULE_LICENSE("GPL");
