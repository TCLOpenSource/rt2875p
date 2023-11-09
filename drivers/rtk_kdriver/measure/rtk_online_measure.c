/*=============================================================
 * File:    rtk_online_measure.c
 *
 * Desc:    Online Measure
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
#include "rtk_online_measure.h"


/*------------------------------------------------
 * Func : _onms_start
 *
 * Desc : start measure
 *
 * Para : id : measure ch
 *
 * Retn : online measure name
 *-----------------------------------------------*/
const char* _onms_str(
    MEASURE_ONMS_CHANNEL    ch
    )
{
    switch(ch)
	{
	case MEASURE_ONMS1_MAIN: return "ONMS1";
	case MEASURE_ONMS3_DMA:  return "ONMS3";
	default:		         return "ONMS_NONE";
	}
}

#define ONMS_INFO(ch, fmt, args...)     MEASURE_EMG("[%s]" fmt, _onms_str(ch), ##args)
#define ONMS_EMG(ch, fmt, args...)      MEASURE_EMG("[%s]" fmt, _onms_str(ch), ##args)

/*------------------------------------------------
 * Func : _dump_measure_timing
 *
 * Desc : set source online1 measure src
 *
 * Para : ch :
 *        p_timing
 *
 * Retn : N/A
 *-----------------------------------------------*/
static void _dump_measure_timing(
    MEASURE_ONMS_CHANNEL    ch,
    MEASURE_TIMING_T*       p_timing
    )
{
    MEASURE_INFO("[%s] Measure sel: %d (0: VGIP INPUT, 1: VGIP OUTPUT)\n", _onms_str(ch),lib_online1_get_ms_src_sel());
    MEASURE_INFO("[%s] IHTotal: %d\n",  _onms_str(ch),p_timing->h_total);
    MEASURE_INFO("[%s] IVTotal: %d\n",  _onms_str(ch),p_timing->v_total);
    MEASURE_INFO("[%s] IHAct: %d\n",    _onms_str(ch),p_timing->h_act_len);
    MEASURE_INFO("[%s] IVAct: %d\n",    _onms_str(ch),p_timing->v_act_len);
    MEASURE_INFO("[%s] IHStr: %d\n",    _onms_str(ch),p_timing->h_act_sta);
    MEASURE_INFO("[%s] IVStr: %d\n",    _onms_str(ch),p_timing->v_act_sta);
    MEASURE_INFO("[%s] IHsyncWidth %d\n", _onms_str(ch),p_timing->IHSyncPulseCount);
    MEASURE_INFO("[%s] IVsyncWidth %d\n", _onms_str(ch),p_timing->IVSyncPulseCount);
}

//----------------------------------------------------------------------------------------
// Low Level API
//----------------------------------------------------------------------------------------


/*------------------------------------------------
 * Func : lib_measure_onms_init
 *
 * Desc : start measure
 *
 * Para : ch : measure ch
 *
 * Retn : N/A
 *-----------------------------------------------*/
void lib_measure_onms_init(
    MEASURE_ONMS_CHANNEL ch
    )
{
    switch(ch)
    {
    case MEASURE_ONMS1_MAIN:
        rtd_maskl(ONMS_onms1_ctrl_reg, ~(ONMS_onms1_ctrl_1_on1_vtotal_of_th_mask |
                                         ONMS_onms1_ctrl_on1_htotal_of_th_mask |
                                         ONMS_onms1_ctrl_on1_meas_to_sel_mask |
                                         ONMS_onms1_ctrl_on1_sycms_clk_mask |
                                         ONMS_onms1_ctrl_on1_ms_conti_mask |
                                         ONMS_onms1_ctrl_on1_start_ms_mask),
                                        (ONMS_onms1_ctrl_1_on1_vtotal_of_th(1) |
                                         ONMS_onms1_ctrl_on1_htotal_of_th(1) |
                                         ONMS_onms1_ctrl_on1_meas_to_sel(1) |
                                         ONMS_onms1_ctrl_on1_sycms_clk(1) |
                                         ONMS_onms1_ctrl_on1_ms_conti(0) |
                                         ONMS_onms1_ctrl_on1_start_ms(0)));
        break;
    case MEASURE_ONMS3_DMA:
         rtd_maskl(ONMS_onms3_ctrl_reg, ~(ONMS_onms3_ctrl_1_on3_vtotal_of_th_mask |
                                          ONMS_onms3_ctrl_on3_ms_src_sel_mask |
                                          ONMS_onms3_ctrl_on3_htotal_of_th_mask |
                                          ONMS_onms3_ctrl_on3_meas_to_sel_mask |
                                          ONMS_onms3_ctrl_on3_sycms_clk_mask |
                                          ONMS_onms3_ctrl_on3_ms_conti_mask |
                                          ONMS_onms3_ctrl_on3_start_ms_mask),
                                         (ONMS_onms3_ctrl_1_on3_vtotal_of_th(1) |
                                          ONMS_onms3_ctrl_on3_ms_src_sel(1) |
                                          ONMS_onms3_ctrl_on3_htotal_of_th(1) |
                                          ONMS_onms3_ctrl_on3_meas_to_sel(1) |
                                          ONMS_onms3_ctrl_on3_sycms_clk(1) |
                                          ONMS_onms3_ctrl_on3_ms_conti(0) |
                                          ONMS_onms3_ctrl_on3_start_ms(0)));
        break;
    default:
        //
        break;
    }
}

/*------------------------------------------------
 * Func : lib_measure_onms_set_ms_src_sel
 *
 * Desc : set source online measure src
 *
 * Para : ch : measure ch
 *        ms_src_sel :
 *         0: MAIN VGIP input
 *         1: MAIN VGIP output
 *
 * Retn : N/A
 *-----------------------------------------------*/
void lib_measure_onms_set_ms_src_sel(
    MEASURE_ONMS_CHANNEL ch,
    unsigned char        ms_src_sel
    )
{
    switch(ch)
    {
    case MEASURE_ONMS1_MAIN:
        rtd_maskl(ONMS_onms1_ctrl_reg, ~ONMS_onms1_ctrl_on1_ms_src_sel_mask,
                                        ONMS_onms1_ctrl_on1_ms_src_sel((ms_src_sel) ? 1 : 0));
        break;
    case MEASURE_ONMS3_DMA:
        //0: DMA VGIP input, 1: output
        rtd_maskl(ONMS_onms3_ctrl_reg, ~ONMS_onms3_ctrl_on3_ms_src_sel_mask,
                                        ONMS_onms3_ctrl_on3_ms_src_sel((ms_src_sel) ? 1 : 0));
        break;
    default:
        //
        break;
    }
}

/*------------------------------------------------
 * Func : lib_measure_onms_get_ms_src_sel
 *
 * Desc : get source online measure src
 *
 * Para : ch : measure ch
 *
 * Retn : 0: VGIP input / 1: VGIP output
 *-----------------------------------------------*/
unsigned char lib_measure_onms_get_ms_src_sel(
    MEASURE_ONMS_CHANNEL ch
    )
{
    switch(ch)
    {
    case MEASURE_ONMS1_MAIN: return ONMS_onms1_ctrl_get_on1_ms_src_sel(rtd_inl(ONMS_onms1_ctrl_reg));
    case MEASURE_ONMS3_DMA: return ONMS_onms3_ctrl_get_on3_ms_src_sel(rtd_inl(ONMS_onms3_ctrl_reg));
    default: return 0;
    }
}


/*------------------------------------------------
 * Func : lib_measure_onms_set_measure_start
 *
 * Desc : start measure
 *
 * Para : ch : measure ch
 *
 * Retn : N/A
 *-----------------------------------------------*/
void lib_measure_onms_set_measure_start(
    MEASURE_ONMS_CHANNEL ch
    )
{
    switch(ch)
    {
    case MEASURE_ONMS1_MAIN:
        rtd_maskl(ONMS_onms1_ctrl_reg, ~ONMS_onms1_ctrl_on1_start_ms_mask, ONMS_onms1_ctrl_on1_start_ms_mask);
        break;
    case MEASURE_ONMS3_DMA:
        rtd_maskl(ONMS_onms3_ctrl_reg, ~ONMS_onms3_ctrl_on3_start_ms_mask, ONMS_onms3_ctrl_on3_start_ms_mask);
        break;
    default:
        //
        break;
    }
}

/*------------------------------------------------
 * Func : lib_measure_onms_get_measure_start
 *
 * Desc : get source online measure start status
 *
 * Para : ch : measure ch
 *
 * Retn : 0: measure stop / 1: measure is running
 *-----------------------------------------------*/
unsigned char lib_measure_onms_get_measure_start(
    MEASURE_ONMS_CHANNEL ch
    )
{
    switch(ch)
    {
    case MEASURE_ONMS1_MAIN: return ONMS_onms1_ctrl_get_on1_start_ms(rtd_inl(ONMS_onms1_ctrl_reg));
    case MEASURE_ONMS3_DMA:  return ONMS_onms3_ctrl_get_on3_start_ms(rtd_inl(ONMS_onms3_ctrl_reg));
    default: return 0;
    }
}

/*------------------------------------------------
 * Func : lib_measure_onms_set_measure_popup
 *
 * Desc : popup measure result
 *
 * Para : ch : measure ch
 *
 * Retn : N/A
 *-----------------------------------------------*/
void lib_measure_onms_set_measure_popup(
    MEASURE_ONMS_CHANNEL ch
    )
{
    switch(ch)
    {
    case MEASURE_ONMS1_MAIN:
        rtd_maskl(ONMS_onms1_ctrl_reg, ~ONMS_onms1_ctrl_on1_popup_meas_mask, ONMS_onms1_ctrl_on1_popup_meas_mask);
        break;
    case MEASURE_ONMS3_DMA:
        rtd_maskl(ONMS_onms3_ctrl_reg, ~ONMS_onms3_ctrl_on3_popup_meas_mask, ONMS_onms3_ctrl_on3_popup_meas_mask);
        break;
    default:
        //
        break;
    }
}

/*------------------------------------------------
 * Func : lib_measure_onms_get_measure_popup
 *
 * Desc : get source online measure popup status
 *
 * Para : ch : measure ch
 *
 * Retn : 0: measure popup / 1: measure is not popup
 *-----------------------------------------------*/
unsigned char lib_measure_onms_get_measure_popup(
    MEASURE_ONMS_CHANNEL ch
    )
{
    switch(ch)
    {
    case MEASURE_ONMS1_MAIN: return ONMS_onms1_ctrl_get_on1_popup_meas(rtd_inl(ONMS_onms1_ctrl_reg));
    case MEASURE_ONMS3_DMA:  return ONMS_onms3_ctrl_get_on3_popup_meas(rtd_inl(ONMS_onms3_ctrl_reg));
    default: return 0;
    }
}

/*------------------------------------------------
 * Func : lib_measure_onms_get_measure_timing
 *
 * Desc : get measured timing
 *
 * Para : MEASURE_ONMS_CHANNEL ch
 *        p_timing
 *
 * Retn : 0: measure popup / 1: measure is not popup
 *-----------------------------------------------*/
void lib_measure_onms_get_measure_timing(
    MEASURE_ONMS_CHANNEL ch,
    MEASURE_TIMING_T*    p_timing
    )
{
    switch(ch)
    {
    case MEASURE_ONMS1_MAIN:
        p_timing->h_act_len = ONMS_onms1_hact_get_on1_hact(rtd_inl(ONMS_onms1_hact_reg));
        p_timing->v_act_len = ONMS_onms1_vs_period_get_on1_vtotalde(rtd_inl(ONMS_onms1_vs_period_reg));
        p_timing->h_total   = ONMS_onms1_hs_period_get_on1_htotal_h12b(rtd_inl(ONMS_onms1_hs_period_reg));
        p_timing->v_total   = ONMS_onms1_vs_period_get_on1_vtotal(rtd_inl(ONMS_onms1_vs_period_reg));
        p_timing->h_act_sta = ONMS_onms1_hact_sta_get_on1_hact_start(rtd_inl(ONMS_onms1_hact_sta_reg));
        p_timing->v_act_sta = ONMS_onms1_vact_sta_get_on1_vact_start(rtd_inl(ONMS_onms1_vact_sta_reg));
        p_timing->IHSyncPulseCount = ONMS_onms1_vshs_high_period_get_on1_hs_high_period(rtd_inl(ONMS_onms1_vshs_high_period_reg));
        p_timing->IVSyncPulseCount = ONMS_onms1_vshs_high_period_get_on1_vs_high_period(rtd_inl(ONMS_onms1_vshs_high_period_reg));
        break;
    case MEASURE_ONMS3_DMA:
        p_timing->h_act_len = ONMS_onms3_hact_get_on3_hact(rtd_inl(ONMS_onms3_hact_reg));
        p_timing->v_act_len = ONMS_onms3_vs_period_get_on3_vtotalde(rtd_inl(ONMS_onms3_vs_period_reg));
        p_timing->h_total   = ONMS_onms3_hs_period_get_on3_htotal_h12b(rtd_inl(ONMS_onms3_hs_period_reg));
        p_timing->v_total   = ONMS_onms3_vs_period_get_on3_vtotal(rtd_inl(ONMS_onms3_vs_period_reg));
        p_timing->h_act_sta = ONMS_onms3_hact_sta_get_on3_hact_start(rtd_inl(ONMS_onms3_hact_sta_reg));
        p_timing->v_act_sta = ONMS_onms3_vact_sta_get_on3_vact_start(rtd_inl(ONMS_onms3_vact_sta_reg));
        p_timing->IHSyncPulseCount = ONMS_onms3_vshs_high_period_get_on3_hs_high_period(rtd_inl(ONMS_onms3_vshs_high_period_reg));
        p_timing->IVSyncPulseCount = ONMS_onms3_vshs_high_period_get_on3_vs_high_period(rtd_inl(ONMS_onms3_vshs_high_period_reg));
        break;
    default:
        p_timing->h_act_len = 0;
        p_timing->v_act_len = 0;
        p_timing->h_total   = 0;
        p_timing->v_total   = 0;
        p_timing->h_act_sta = 0;
        p_timing->v_act_sta = 0;
        p_timing->IHSyncPulseCount = 0;
        p_timing->IVSyncPulseCount = 0;
        break;
    }
}

/*------------------------------------------------
 * Func : lib_measure_onms_set_watchdog_int_en
 *
 * Desc : set online measure watchdog interrupt
 *
 * Para : ch : measure ch
 *        value        : watch dog value
 *
 * Retn : N/A
 *-----------------------------------------------*/
void lib_measure_onms_set_watchdog_int_en(
    MEASURE_ONMS_CHANNEL ch,
    unsigned int         value
    )
{
	unsigned int onms_watchdog_addr = 0;
	unsigned int onms_int_addr = 0;

	switch(ch)
	{
	case MEASURE_ONMS1_MAIN:
		onms_watchdog_addr = ONMS_onms1_watchdog_en_reg;
		onms_int_addr = ONMS_onms1_interrupt_en_reg;
		break;
	case MEASURE_ONMS3_DMA:
		onms_watchdog_addr = ONMS_onms3_watchdog_en_reg;
		onms_int_addr = ONMS_onms3_interrupt_en_reg;
		break;
	default:
		onms_watchdog_addr = ONMS_onms1_watchdog_en_reg;
		onms_int_addr = ONMS_onms1_interrupt_en_reg;

		MEASURE_EMG("[lib_measure_onms_set_watchdog_int_en]Error!! invalid ch=%s, value=%d\n", _onms_str(ch),value);
		break;
	}
	rtd_outl(onms_watchdog_addr, value);
	rtd_outl(onms_int_addr, value);
}
EXPORT_SYMBOL(lib_measure_onms_set_watchdog_int_en);


/*------------------------------------------------
 * Func : lib_measure_onms_get_watchdog_en
 *
 * Desc : get online measure watchdog enable
 *
 * Para : ch : measure ch
 *
 * Retn : interrupt enable mask
 *-----------------------------------------------*/
unsigned int lib_measure_onms_get_watchdog_en(
    MEASURE_ONMS_CHANNEL ch
    )
{
	unsigned int onms_watchdog_addr = 0;
	switch(ch)
	{
	case MEASURE_ONMS1_MAIN:
		onms_watchdog_addr = ONMS_onms1_watchdog_en_reg;
		break;
	case MEASURE_ONMS3_DMA:
		onms_watchdog_addr = ONMS_onms3_watchdog_en_reg;
		break;
	default:
		MEASURE_EMG("[lib_measure_onms_get_watchdog_int_en]Error!! invalid ch=%d\n", ch);
		break;
	}

	return rtd_inl(onms_watchdog_addr);
}
EXPORT_SYMBOL(lib_measure_onms_get_watchdog_en);


/*------------------------------------------------
 * Func : lib_measure_onms_get_int_en
 *
 * Desc : get online measure watchdog interrupt enable
 *
 * Para : ch : measure ch
 *
 * Retn : interrupt enable mask
 *-----------------------------------------------*/
unsigned int lib_measure_onms_get_int_en(
    MEASURE_ONMS_CHANNEL    ch
    )
{
	unsigned int onms_int_addr = 0;
	switch(ch)
	{
	case MEASURE_ONMS1_MAIN:
		onms_int_addr = ONMS_onms1_interrupt_en_reg;
		break;
	case MEASURE_ONMS3_DMA:
		onms_int_addr = ONMS_onms3_interrupt_en_reg;
		break;
	default:
		MEASURE_EMG("[lib_measure_onms_get_int_en]Error!! invalid ch=%d\n", ch);
		break;
	}

	return rtd_inl(onms_int_addr);
}

//----------------------------------------------------------------------------------------
// High level API
//----------------------------------------------------------------------------------------

/*------------------------------------------------
 * Func : _onms_start
 *
 * Desc : start measure
 *
 * Para : ch : measure ch
 *
 * Retn : TRUE : success
 *        FALSE : failed
 *-----------------------------------------------*/
static unsigned char _do_onms_measure(
    MEASURE_ONMS_CHANNEL    ch
    )
{
    unsigned int t1;

    // start measure
    lib_measure_onms_set_measure_start(ch);

    t1 = 100;
    do
    {
        t1--;

        if (lib_measure_onms_get_measure_start(ch)==0)
            break;

        msleep(2);
    } while (t1 > 0);

#if 1
    if (t1 == 0) {
        MEASURE_EMG("[Warning][%s] meas is not finish \n", _onms_str(ch));
    }
#else
    if (t1 == 0)
        return FALSE;
#endif

    // popup result
    lib_measure_onms_set_measure_popup(ch);

    t1 = 100;
    do
    {
        t1--;
        if (lib_measure_onms_get_measure_popup(ch)==0)
            break;

        msleep(2);
    } while (t1 > 0);

    if (t1 == 0)
        return FALSE;

    return TRUE;
}


/*------------------------------------------------
 * Func : _do_online_measure
 *
 * Desc : do online1 measure
 *
 * Para : ch : measure ch
 *        p_timing : timing
 *
 * Retn : TRUE : success
 *        FALSE : failed
 *-----------------------------------------------*/
unsigned char _do_online_measure(
    MEASURE_ONMS_CHANNEL    ch,
    MEASURE_TIMING_T*       p_timing
    )
{
    lib_measure_onms_init(ch);

    if (!_do_onms_measure(ch))
    {
        MEASURE_EMG("[%s] OnLineMeasure is not finish.\n", _onms_str(ch));
        return FALSE;
    }

    lib_measure_onms_get_measure_timing(ch,p_timing);

    _dump_measure_timing(ch,p_timing);
    return TRUE;

}

//----------------------------------------------------------------------------------------
// High level API (Export to other module)
//----------------------------------------------------------------------------------------

/*------------------------------------------------
 * Func : lib_online1_set_ms_src_sel
 *
 * Desc : set source online1 measure src
 *
 * Para : ms_src_sel :
 *         0: MAIN VGIP input
 *         1: MAIN VGIP output
 *
 * Retn : N/A
 *-----------------------------------------------*/
void lib_online1_set_ms_src_sel(unsigned char ms_src_sel)
{
    lib_measure_onms_set_ms_src_sel(MEASURE_ONMS1_MAIN, ms_src_sel);
}

/*------------------------------------------------
 * Func : lib_online1_get_ms_src_sel
 *
 * Desc : get source online1 measure src
 *
 * Para : N/A
 *
 * Retn : 0: MAIN VGIP input / 1: MAIN VGIP output
 *-----------------------------------------------*/
unsigned char lib_online1_get_ms_src_sel(void)
{
    return lib_measure_onms_get_ms_src_sel(MEASURE_ONMS1_MAIN);
}

/*------------------------------------------------
 * Func : lib_online1_measure
 *
 * Desc : do online1 measure
 *
 * Para : p_timing : timing
 *
 * Retn : 0: MAIN VGIP input / 1: MAIN VGIP output
 *-----------------------------------------------*/
unsigned char lib_online1_measure(
    MEASURE_TIMING_T*   p_timing
    )
{
    return _do_online_measure(MEASURE_ONMS1_MAIN, p_timing);
}

EXPORT_SYMBOL(lib_online1_set_ms_src_sel);
EXPORT_SYMBOL(lib_online1_get_ms_src_sel);
EXPORT_SYMBOL(lib_online1_measure);

//----------------------------------------------------------------------------------------
// online3 API
//----------------------------------------------------------------------------------------

/*------------------------------------------------
 * Func : lib_online3_set_ms_src_sel
 *
 * Desc : set source online3 measure src
 *
 * Para : ms_src_sel :
 *         0: MAIN VGIP input
 *         1: MAIN VGIP output
 *
 * Retn : N/A
 *-----------------------------------------------*/
void lib_online3_set_ms_src_sel(unsigned char ms_src_sel)
{
    lib_measure_onms_set_ms_src_sel(MEASURE_ONMS3_DMA, ms_src_sel);
}

/*------------------------------------------------
 * Func : lib_online3_get_ms_src_sel
 *
 * Desc : get source online1 measure src
 *
 * Para : N/A
 *
 * Retn : 0: MAIN VGIP input / 1: MAIN VGIP output
 *-----------------------------------------------*/
unsigned char lib_online3_get_ms_src_sel(void)
{
    return lib_measure_onms_get_ms_src_sel(MEASURE_ONMS3_DMA);
}

/*------------------------------------------------
 * Func : lib_online3_measure
 *
 * Desc : do online3 measure
 *
 * Para : p_timing :
 *
 * Retn : TRUE / FALSE
 *-----------------------------------------------*/
unsigned char lib_online3_measure(
    MEASURE_TIMING_T *p_timing
    )
{
    return _do_online_measure(MEASURE_ONMS3_DMA, p_timing);
}

/*------------------------------------------------
 * Func : lib_online3_measure
 *
 * Desc : do online3 measure
 *
 * Para : p_timing :
 *
 * Retn : TRUE / FALSE
 *-----------------------------------------------*/
unsigned char lib_online3_demode_measure(MEASURE_TIMING_T *p_timing)
{
#if !ONMS_SUPPORT_DEMODE
    return FALSE;    //Mac7p don't support DEmode
#else
    unsigned char result = FALSE;
    unsigned long long de_vs_period = 0;
    unsigned int vfreq = 0;

    rtd_maskl(ONMS_Onms3_Demode_ctrl_reg, ~(ONMS_Onms3_Demode_ctrl_on3_demode_en_mask),
                                           (ONMS_Onms3_Demode_ctrl_on3_demode_en(1)));

    if (!_do_onms_measure(MEASURE_ONMS3_DMA))
    {
        MEASURE_EMG("[lib_hdmims_online3_demode_measure] OnLineMeasure is not finish.\n");
        return FALSE;
    }

    p_timing->h_act_len = ONMS_Onms3_Demode_Act_get_on3_de_hact(rtd_inl(ONMS_Onms3_Demode_Act_reg));
    p_timing->v_act_len = ONMS_Onms3_Demode_Act_get_on3_de_vact(rtd_inl(ONMS_Onms3_Demode_Act_reg));
    p_timing->h_total   = ONMS_Onms3_Demode_Hs_period_get_on3_de_hs_period(rtd_inl(ONMS_Onms3_Demode_Hs_period_reg));
    //p_timing->v_total = ONMS_Onms3_Demode_Vs_period_get_on3_de_vs_period(rtd_inl(ONMS_Onms3_Demode_Vs_period_reg));
    p_timing->v_act_sta = ONMS_Onms3_Demode_Vs_STA_get_on3_de_vs_sta(rtd_inl(ONMS_Onms3_Demode_Vs_STA_reg));

    de_vs_period = (unsigned long long)ONMS_Onms3_Demode_Vs_period_get_on3_de_vs_period(rtd_inl(ONMS_Onms3_Demode_Vs_period_reg));

    MEASURE_INFO("[ONMS3_DE] de_vs_period=%lld \n", de_vs_period);

    if (de_vs_period != 0)
    {
        vfreq = lib_measure_calc_vfreq(de_vs_period);
        p_timing->v_freq = vfreq;
        p_timing->h_freq = (p_timing->v_freq / 100) * p_timing->v_total / 10;
        result = TRUE;
    }

    MEASURE_INFO("[ONMS3_DE] IHTotal: %d\n", p_timing->h_total);
    //MEASURE_INFO("[ONMS3_DE] IVTotal: %d\n", p_timing->v_total);
    MEASURE_INFO("[ONMS3_DE] IHAct: %d\n", p_timing->h_act_len);
    MEASURE_INFO("[ONMS3_DE] IVAct: %d\n", p_timing->v_act_len);
    MEASURE_INFO("[ONMS3_DE] IVStr: %d, End: %d\n", p_timing->v_act_sta, ONMS_Onms3_Demode_Vs_END_get_on3_de_vs_end(ONMS_Onms3_Demode_Vs_END_reg));
    MEASURE_INFO("[ONMS3_DE] IHFreq: %d (Hz)\n", p_timing->h_freq);
    MEASURE_INFO("[ONMS3_DE] IVFreq: %d (0.001Hz)\n", p_timing->v_freq);
    return TRUE;
#endif
}

EXPORT_SYMBOL(lib_online3_set_ms_src_sel);
EXPORT_SYMBOL(lib_online3_get_ms_src_sel);
EXPORT_SYMBOL(lib_online3_measure);
EXPORT_SYMBOL(lib_online3_demode_measure);
