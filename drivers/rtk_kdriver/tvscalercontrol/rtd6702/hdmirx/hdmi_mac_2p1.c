/*=============================================================
 * Copyright (c)      Realtek Semiconductor Corporation, 2016
 *
 * All rights reserved.
 *
 *============================================================*/

/*======================= Description ============================
 *
 * file:    hdmi_mac.c
 *
 * author:  kevin_wang@realtek.com
 * date:
 * version:     3.0
 *
 *============================================================*/

/*========================Header Files============================*/
#include "hdmi_common.h"
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <mach/platform.h>
#include <rbus/stb_reg.h>
#include "hdmi_hdcp.h"
#include "hdmi_scdc.h"
#include "hdmi_reg.h"
#include "hdmi_mac_2p1.h"
#include "hdmi_vfe_config.h"
#include "hdmi_dsc.h"
#ifdef FORCE_VGIP_PIXEL_MODE
#include <tvscalercontrol/scalerdrv/scalerdrv.h> //CONFIG_FORCE_RUN_I3DDMA
#endif

FRL_LT_FSM_STATUS_T lt_fsm_status[HDMI_PORT_TOTAL_NUM];


extern HDMI_PORT_INFO_T hdmi_rx[HDMI_PORT_TOTAL_NUM];
lane_align_state        lane_st[4];  //bgrc

#define DATA_ALIGN_TIMEOUT_THD    25    // if align timeout, need realign
#define DATA_ALIGN_DEBOUNCE_CNT_THD   6   //need to 500us  10super block total 16320 bit
#define DATA_ALIGN_CHECK_LOOP_THD    600 //must > DATA_ALIGN_TIMEOUT_THD
#define ltp_err_cnt_bit_0    _BIT4     //avoid ltp err to clear

#define FW_POLARITY_CHECK_DEBOUNCE_THD    2

#define LTPASS_WAIT_FLTSTART_CLR_TIMEOUT_THD   5

//extern void newbase_rxphy_job(unsigned char p);

extern unsigned int hdmi_power_saving_enable;
unsigned char last_phy_hyst_mode = 0;

#define HD21_MEAS_SUPER_FAST_V_FREQ_THD    115000    // >=115HZ V FREQ
#define HD21_MEAS_FAST_V_FREQ_THD    31000    // >=31HZ V FREQ

#define HD21_MEAS_SUPER_FAST_V_FREQ_TOLERENCE    100    // >=115HZ V FREQ
#define HD21_MEAS_FAST_V_FREQ_TOLERENCE    400    // >=31HZ V FREQ
#define HD21_MEAS_SLOW_V_FREQ_TOLERENCE    200     // <31HZ V FREQ

//------------------------------------------------------------------
// HDMI 2.1 Control
//------------------------------------------------------------------


/*---------------------------------------------------
 * Func : lib_hdmi_is_hdmi_21_available
 *
 * Desc : check if the specified support HDMI 2.1 or not
 *
 * Para : nport : hdmi port id
 *
 * Retn : 0 : not support HDMI 2.1 ,
 *        1 : support HDMI 2.1
 *--------------------------------------------------*/
unsigned char lib_hdmi_is_hdmi_21_available(unsigned char nport)
{
    if((nport < HDMI21_MAC_NUM) && HD21_VALID_reg)
        return 1;
    else
        return 0;
}

/*---------------------------------------------------
 * Func : lib_hdmi_hd21_enable
 *
 * Desc : enable HDMI 2.1 function of the given HDMI port
 *
 * Para : nport : hdmi port id
 *        on    : 0 : disable / 1 : enable
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_hd21_enable(unsigned char nport, unsigned char on)
{
    if (lib_hdmi_is_hdmi_21_available(nport))
        hdmi_mask(HD21_CR_reg, ~HD21_CR_hd21_en_mask, HD21_CR_hd21_en((on) ? 1: 0));
}

/*---------------------------------------------------
 * Func : lib_hdmi_is_hdmi_21_enabled
 *
 * Desc : check the HDMI 2.1 function has been disabled or not
 *
 * Para : nport : hdmi port id
 *
 * Retn : 0 : HDMI 2.1 is disabled
 *        1 : HDMI 2.1 is enabled
 *--------------------------------------------------*/
unsigned char lib_hdmi_is_hdmi_21_enabled(unsigned char nport)
{
    if (lib_hdmi_is_hdmi_21_available(nport))
        return HD21_CR_get_hd21_en(hdmi_in(HD21_CR_reg));
    return 0;
}


void lib_hdmi_hd21_mac_crt_perport_off(unsigned char nport)
{//Alwasy use physical port 0 address
    unsigned int clken_pre_mask = HDMI_P0_hdmi_clken3_clken_hd21_pre0_mask << nport;
    unsigned int clken_hdcp_pre_mask = HDMI_P0_hdmi_clken3_clken_hd21_hdcp_pre0_mask << nport;
    unsigned int rstn_pre_mask = HDMI_P0_hdmi_rst3_rstn_hd21_pre0_mask << nport;
    unsigned int rstn_hdcp_pre_addr = HDMI_P0_hdmi_rst3_rstn_hd21_hdcp_pre0_mask << nport;

    if(!lib_hdmi_is_hdmi_21_available(nport))
        return;

    hdmi_mask(HDMI_P0_hdmi_clken3_reg,~(clken_pre_mask|clken_hdcp_pre_mask), 0x0);
    udelay(5);
    hdmi_mask(HDMI_P0_hdmi_rst3_reg,~(rstn_pre_mask|rstn_hdcp_pre_addr), 0x0);
}

void lib_hdmi_hd21_mac_crt_perport_on(unsigned char nport)
{//Alwasy use physical port 0 address
    unsigned int clken_pre_mask = HDMI_P0_hdmi_clken3_clken_hd21_pre0_mask << nport;
    unsigned int clken_hdcp_pre_mask = HDMI_P0_hdmi_clken3_clken_hd21_hdcp_pre0_mask << nport;
    unsigned int rstn_pre_mask = HDMI_P0_hdmi_rst3_rstn_hd21_pre0_mask << nport;
    unsigned int rstn_hdcp_pre_addr = HDMI_P0_hdmi_rst3_rstn_hd21_hdcp_pre0_mask << nport;

    if(!lib_hdmi_is_hdmi_21_available(nport))
        return;

    hdmi_mask(HDMI_P0_hdmi_rst3_reg,~(rstn_pre_mask|rstn_hdcp_pre_addr), (clken_pre_mask|clken_hdcp_pre_mask));
    udelay(5);
    hdmi_mask(HDMI_P0_hdmi_clken3_reg,~(clken_pre_mask|clken_hdcp_pre_mask), (clken_pre_mask|clken_hdcp_pre_mask));
}

void lib_hdmi_hd21_mac_crt_perport_reset(unsigned char nport)
{
    HDMI_INFO("[CRT]lib_hdmi_hd21_mac_crt_perport_reset, nport=%d\n", nport);

    if (!lib_hdmi_is_hdmi_21_available(nport))
        return ;

    lib_hdmi_hd21_mac_crt_perport_off(nport);
    udelay(5);
    lib_hdmi_hd21_mac_crt_perport_on(nport);

}

void lib_hdmi_hd21_mac_crt_common_reset(unsigned char nport)
{
    HDMI_INFO("[CRT]lib_hdmi_hd21_mac_crt_common_reset, nport=%d\n", nport);

    if (!lib_hdmi_is_hdmi_21_available(nport))
        return ;
    // clk en disable
    hdmi_mask(HDMI_P0_hdmi_clken3_reg, ~(HDMI_P0_hdmi_clken3_clken_hd21_common_mask | HDMI_P0_hdmi_clken3_clken_hd21_common2_mask), 0x0);
    udelay(5);
    // reset 0
    hdmi_mask(HDMI_P0_hdmi_rst3_reg, ~(HDMI_P0_hdmi_rst3_rstn_hd21_common_mask | HDMI_P0_hdmi_rst3_rstn_hd21_common2_mask), 0x0);
    udelay(5);
     // reset 1
    hdmi_mask(HDMI_P0_hdmi_rst3_reg, ~(HDMI_P0_hdmi_rst3_rstn_hd21_common_mask | HDMI_P0_hdmi_rst3_rstn_hd21_common2_mask), HDMI_P0_hdmi_rst3_rstn_hd21_common_mask | HDMI_P0_hdmi_rst3_rstn_hd21_common2_mask);
    udelay(5);
    // clk en enable
    hdmi_mask(HDMI_P0_hdmi_clken3_reg, ~(HDMI_P0_hdmi_clken3_clken_hd21_common_mask | HDMI_P0_hdmi_clken3_clken_hd21_common2_mask), HDMI_P0_hdmi_clken3_clken_hd21_common_mask | HDMI_P0_hdmi_clken3_clken_hd21_common2_mask);

}

void lib_hdmi_hd21_crt_off(void)
{//Alwasy use physical port 0 address
    // clk en disable
    hdmi_mask(HDMI_P0_hdmi_clken3_reg, ~(HDMI_P0_hdmi_clken3_clken_hd21_common_mask|HDMI_P0_hdmi_clken3_clken_hd21_common2_mask|
        HDMI_P0_hdmi_clken3_clken_hd21_pre0_mask|HDMI_P0_hdmi_clken3_clken_hd21_pre1_mask|
        HDMI_P0_hdmi_clken3_clken_hd21_hdcp_pre0_mask|HDMI_P0_hdmi_clken3_clken_hd21_hdcp_pre1_mask), 0x0);
    udelay(5);

    // reset 0
    hdmi_mask(HDMI_P0_hdmi_rst3_reg, ~(HDMI_P0_hdmi_rst3_rstn_hd21_common_mask|HDMI_P0_hdmi_rst3_rstn_hd21_common2_mask|
        HDMI_P0_hdmi_rst3_rstn_hd21_pre0_mask|HDMI_P0_hdmi_rst3_rstn_hd21_pre1_mask|
        HDMI_P0_hdmi_rst3_rstn_hd21_hdcp_pre0_mask|HDMI_P0_hdmi_rst3_rstn_hd21_hdcp_pre1_mask), 0x0);
}

void lib_hdmi_hd21_crt_on(void)
{//Alwasy use physical port 0 address
    // reset 1
    hdmi_mask(HDMI_P0_hdmi_rst3_reg, ~(HDMI_P0_hdmi_rst3_rstn_hd21_common_mask|HDMI_P0_hdmi_rst3_rstn_hd21_common2_mask|
        HDMI_P0_hdmi_rst3_rstn_hd21_pre0_mask|HDMI_P0_hdmi_rst3_rstn_hd21_pre1_mask|
        HDMI_P0_hdmi_rst3_rstn_hd21_hdcp_pre0_mask|HDMI_P0_hdmi_rst3_rstn_hd21_hdcp_pre1_mask),
        (HDMI_P0_hdmi_rst3_rstn_hd21_common_mask|
        HDMI_P0_hdmi_rst3_rstn_hd21_pre0_mask|HDMI_P0_hdmi_rst3_rstn_hd21_pre1_mask|
        HDMI_P0_hdmi_rst3_rstn_hd21_hdcp_pre0_mask|HDMI_P0_hdmi_rst3_rstn_hd21_hdcp_pre1_mask));
    udelay(5);
    // clk en enable
    hdmi_mask(HDMI_P0_hdmi_clken3_reg, ~(HDMI_P0_hdmi_clken3_clken_hd21_common_mask|HDMI_P0_hdmi_clken3_clken_hd21_common2_mask|
        HDMI_P0_hdmi_clken3_clken_hd21_pre0_mask|HDMI_P0_hdmi_clken3_clken_hd21_pre1_mask|
        HDMI_P0_hdmi_clken3_clken_hd21_hdcp_pre0_mask|HDMI_P0_hdmi_clken3_clken_hd21_hdcp_pre1_mask),
        (HDMI_P0_hdmi_clken3_clken_hd21_common_mask|
        HDMI_P0_hdmi_clken3_clken_hd21_pre0_mask|HDMI_P0_hdmi_clken3_clken_hd21_pre1_mask|
        HDMI_P0_hdmi_clken3_clken_hd21_hdcp_pre0_mask|HDMI_P0_hdmi_clken3_clken_hd21_hdcp_pre1_mask));
}


void lib_hdmi_hd21_mac_crt_reset(unsigned char nport)
{
    HDMI_INFO("[CRT]lib_hdmi_hd21_mac_crt_reset, nport=%d\n", nport);

    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    lib_hdmi_hd21_crt_off();
    udelay(5);
    lib_hdmi_hd21_crt_on();
}

/*---------------------------------------------------
 * Func : lib_hdmi_hd21_set_lane_mode_fw
 *
 * Desc : set FW lane mode
 *
 * Para : nport : hdmi port id
 *        lane_mode : 0 : 3 lane, others : 4 lane
 *
 * Retn : 0 : HDMI 2.1 is disabled
 *        1 : HDMI 2.1 is enabled
 *--------------------------------------------------*/
void lib_hdmi_hd21_set_lane_mode_fw(unsigned char nport, HDMI_FRL_MODE_T frl_mode)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;
    
    if (frl_mode >= MODE_FRL_6G_4_LANE )
    {
        hdmi_mask(HD21_CR_reg, ~HDMI21_P0_HD21_CR_lane_mode_fw_mask, HDMI21_P0_HD21_CR_lane_mode_fw(1));
    }
    else
    {
        hdmi_mask(HD21_CR_reg, ~HDMI21_P0_HD21_CR_lane_mode_fw_mask, HDMI21_P0_HD21_CR_lane_mode_fw(0));
    }
           
}

//------------------------------------------------------------------
// Phy FIFO
//------------------------------------------------------------------
void lib_hdmi_hd21_phy_afifo_enable(unsigned char nport, unsigned char enable)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    enable = (enable) ? 1 : 0 ;
    hdmi_mask(HD21_PHY_FIFO_CR0_reg, ~HD21_PHY_FIFO_CR0_port0_c_afifo_en_mask, HD21_PHY_FIFO_CR0_port0_c_afifo_en(enable));    //HD21 MAC ck affio enable
    hdmi_mask(HD21_PHY_FIFO_CR0_reg, ~HD21_PHY_FIFO_CR0_port1_c_afifo_en_mask, HD21_PHY_FIFO_CR0_port1_c_afifo_en(enable));    //HD21 MAC ck affio enable
}

//------------------------------------------------------------------
// Channel FIFO
//------------------------------------------------------------------

void lib_hdmi_hd21_afifo_enable(unsigned char nport, unsigned char enable)
{    
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    enable = (enable) ? 1 : 0 ;
    hdmi_mask(HD21_ch_sync_r_reg, ~HD21_ch_sync_r_ch_afifo_en_mask,
                                             HD21_ch_sync_r_ch_afifo_en(enable));

    hdmi_mask(HD21_ch_sync_g_reg, ~HD21_ch_sync_g_ch_afifo_en_mask,
                                             HD21_ch_sync_g_ch_afifo_en(enable));

    hdmi_mask(HD21_ch_sync_b_reg, ~HD21_ch_sync_b_ch_afifo_en_mask,
                                             HD21_ch_sync_b_ch_afifo_en(enable));

    hdmi_mask(HD21_ch_sync_c_reg, ~HD21_ch_sync_c_ch_afifo_en_mask,
                                             HD21_ch_sync_c_ch_afifo_en(enable));
}

//------------------------------------------------------------------
// Link Training
//------------------------------------------------------------------

void _dump_link_training_status(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;
    
    FRL_INFO("Port:%d, CR_reg=%08x, LN_ALL=%08x, LT_X(B/G/R/C)=%08x/%08x/%08x/%08x, current_ms=%ld, timeout_flag=%d,  Flt_update=%d, SCDC 0x40=%02x,%02x,%02x\n",
        nport,
        hdmi_in(HD21_CR_reg),
        hdmi_in(HD21_LT_ALL_reg),
        hdmi_in(HD21_LT_B_reg),
        hdmi_in(HD21_LT_G_reg),
        hdmi_in(HD21_LT_R_reg),
        hdmi_in(HD21_LT_C_reg),
        hdmi_get_system_time_ms(),
        HD21_LT_B_get_timeout_flag(hdmi_in(HD21_LT_B_reg)),
        lib_hdmi_scdc_get_flt_update(nport),
        lib_hdmi_scdc_read(nport, SCDC_STATUS_FLAGS),
        lib_hdmi_scdc_read(nport, SCDC_STATUS_FLAGS_1),
        lib_hdmi_scdc_read(nport, SCDC_STATUS_FLAGS_2));
}

/*---------------------------------------------------
 * Func : lib_hdmi_hd21_link_training_disable
 *
 * Desc : disable link training
 *
 * Para : nport : hdmi port id
 *
 * Retn : 0 : HDMI 2.1 is disabled
 *        1 : HDMI 2.1 is enabled
 *--------------------------------------------------*/
void lib_hdmi_hd21_link_training_disable(unsigned char nport)
{
    unsigned char ltp[4] = {0, 0, 0, 0};
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return ;
    FRL_INFO("[lib_hdmi_hd21_link_training_disable port:%d]\n", nport);
    hdmi_out(HD21_LT_ALL_reg, 0);
    hdmi_out(HD21_LT_R_reg, 0);
    hdmi_out(HD21_LT_G_reg, 0);
    hdmi_out(HD21_LT_B_reg, 0);
    hdmi_out(HD21_LT_C_reg, 0);
    lib_hdmi_scdc_update_ltp_req(nport, ltp);
    _dump_link_training_status(nport);
}

/*---------------------------------------------------
 * Func : lib_hdmi_hd21_link_training_enable
 *
 * Desc : enable link training
 *
 * Para : nport : hdmi port id
 *        rate  : training rate (GHz)
 *        timeout : training timeout (ms)
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_hd21_link_training_enable(
    unsigned char           nport,
    HDMI_FRL_MODE_T           frl_mode,
    unsigned int            timeout
    )
{
    unsigned char ltp3_sel = 4;

    if (!lib_hdmi_is_hdmi_21_available(nport))
        return ;

    switch(frl_mode)
    {
    case MODE_TMDS:    ltp3_sel = 4;    break;
    case MODE_FRL_3G_3_LANE:    ltp3_sel = 4;    break;
    case MODE_FRL_6G_3_LANE:    ltp3_sel = 3;    break;
    case MODE_FRL_6G_4_LANE:    ltp3_sel = 3;    break;
    case MODE_FRL_8G_4_LANE:    ltp3_sel = 2;    break;
    case MODE_FRL_10G_4_LANE:    ltp3_sel = 1;    break;
    case MODE_FRL_12G_4_LANE:    ltp3_sel = 0;    break;
    default:     ltp3_sel = 4;    break;
    }

    timeout *= 27000;

    FRL_INFO("[lib_hdmi_hd21_link_training_enable port:%d] Link Training HW enable, frl mode =%d, ltp3_sel =%d, timeout=%d\n", nport, frl_mode, ltp3_sel, timeout);

    hdmi_out(HD21_LT_ALL_reg, HD21_LT_ALL_ltp_en(1)|
                                        HD21_LT_ALL_ltp3_sel(ltp3_sel)|
                                        HD21_LT_ALL_timeout_set(timeout));

    _dump_link_training_status(nport);
}

/*---------------------------------------------------
 * Func : lib_hdmi_hd21_link_training_enable
 *
 * Desc : enable link training
 *
 * Para : nport : hdmi port id
 *        ltp   : link training pattern for each lane
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_hd21_link_training_start(
    unsigned char           nport, 
    unsigned char           ltp[4]
    )
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    FRL_INFO("Link Training start with LTP (B,G,R,C) = (%x,%x,%x,%x)\n",
            ltp[0], ltp[1], ltp[2], ltp[3]);

    hdmi_mask(HD21_LT_B_reg, ~(HD21_LT_B_ln_req_mask|ltp_err_cnt_bit_0), HD21_LT_B_ln_req(ltp[0]));
    hdmi_mask(HD21_LT_G_reg, ~(HD21_LT_G_ln_req_mask|ltp_err_cnt_bit_0), HD21_LT_G_ln_req(ltp[1]));
    hdmi_mask(HD21_LT_R_reg, ~(HD21_LT_R_ln_req_mask|ltp_err_cnt_bit_0), HD21_LT_R_ln_req(ltp[2]));
    hdmi_mask(HD21_LT_C_reg, ~(HD21_LT_C_ln_req_mask|ltp_err_cnt_bit_0), HD21_LT_C_ln_req(ltp[3]));

    hdmi_mask(HD21_LT_B_reg, ~(HD21_LT_B_lock_sel_mask|ltp_err_cnt_bit_0), HD21_LT_B_lock_sel(8));
    hdmi_mask(HD21_LT_G_reg, ~(HD21_LT_G_lock_sel_mask|ltp_err_cnt_bit_0), HD21_LT_G_lock_sel(8));
    hdmi_mask(HD21_LT_R_reg, ~(HD21_LT_R_lock_sel_mask|ltp_err_cnt_bit_0), HD21_LT_R_lock_sel(8));
    hdmi_mask(HD21_LT_C_reg, ~(HD21_LT_C_lock_sel_mask|ltp_err_cnt_bit_0), HD21_LT_C_lock_sel(8));

    hdmi_mask(HD21_LT_B_reg, ~(HD21_LT_B_reset_mask|ltp_err_cnt_bit_0), HD21_LT_B_reset_mask);
    hdmi_mask(HD21_LT_G_reg, ~(HD21_LT_G_reset_mask|ltp_err_cnt_bit_0), HD21_LT_G_reset_mask);
    hdmi_mask(HD21_LT_R_reg, ~(HD21_LT_R_reset_mask|ltp_err_cnt_bit_0), HD21_LT_R_reset_mask);
    hdmi_mask(HD21_LT_C_reg, ~(HD21_LT_C_reset_mask|ltp_err_cnt_bit_0), HD21_LT_C_reset_mask);

    _dump_link_training_status(nport);
}

/*---------------------------------------------------
 * Func : lib_hdmi_hd21_link_training_status
 *
 * Desc : get link training status
 *
 * Para : nport : hdmi port id
 *        err_cnt : error count
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_hd21_link_training_status(
    unsigned char               nport,
    unsigned int                lock[4],
    unsigned int                err_cnt[4]
    )
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;
            
    lock[0] = HD21_LT_B_get_lock_valid(hdmi_in(HD21_LT_B_reg));
    lock[1] = HD21_LT_G_get_lock_valid(hdmi_in(HD21_LT_G_reg));
    lock[2] = HD21_LT_R_get_lock_valid(hdmi_in(HD21_LT_R_reg));
    lock[3] = HD21_LT_C_get_lock_valid(hdmi_in(HD21_LT_C_reg));

    hdmi_mask(HD21_LT_B_reg, ~(HD21_LT_B_popup_mask|ltp_err_cnt_bit_0), HD21_LT_B_popup_mask);
    hdmi_mask(HD21_LT_G_reg, ~(HD21_LT_G_popup_mask|ltp_err_cnt_bit_0), HD21_LT_G_popup_mask);
    hdmi_mask(HD21_LT_R_reg, ~(HD21_LT_R_popup_mask|ltp_err_cnt_bit_0), HD21_LT_R_popup_mask);
    hdmi_mask(HD21_LT_C_reg, ~(HD21_LT_C_popup_mask|ltp_err_cnt_bit_0), HD21_LT_C_popup_mask);

    err_cnt[0] = (lock[0]) ? HD21_LT_B_get_err_cnt(hdmi_in(HD21_LT_B_reg)) : 0xFFFFFFFF;
    err_cnt[1] = (lock[1]) ? HD21_LT_G_get_err_cnt(hdmi_in(HD21_LT_G_reg)) : 0xFFFFFFFF;
    err_cnt[2] = (lock[2]) ? HD21_LT_R_get_err_cnt(hdmi_in(HD21_LT_R_reg)) : 0xFFFFFFFF;
    err_cnt[3] = (lock[3]) ? HD21_LT_C_get_err_cnt(hdmi_in(HD21_LT_C_reg)) : 0xFFFFFFFF; 

    _dump_link_training_status(nport);
}

unsigned char lib_hdmi_hd21_link_training_get_timeout_flag(unsigned char nport, unsigned char lane)
{
    unsigned int reg_addr = 0;
    
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return 1;

    switch(lane)
    {
        case 0:
            reg_addr = HD21_LT_B_reg;
            break;
        case 1:
            reg_addr = HD21_LT_G_reg;
            break;
        case 2:
            reg_addr = HD21_LT_R_reg;
            break;
        case 3:
            reg_addr = HD21_LT_C_reg;
            break;
        default:
            HDMI_EMG("[lib_hdmi_hd21_link_training_get_timeout_flag port:%d] Error parameter lane=%d\n", nport, lane);
            return 1;
    }
    return HD21_LT_B_get_timeout_flag(hdmi_in(reg_addr));
}


//------------------------------------------------------------------
// Data Align
//------------------------------------------------------------------
void lib_hdmi_hd21_clear_data_align_error_status(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    // error flag need to write 1 clear
    hdmi_mask(HD21_channel_align_c_reg,
        ~(HD21_channel_align_b_unex_comma_mask|
           HD21_channel_align_b_position_error_flag_mask|
           HD21_channel_align_b_position_error_cnt_mask),
           (HD21_channel_align_b_unex_comma_mask|
           HD21_channel_align_b_position_error_flag_mask|
           HD21_channel_align_b_position_error_cnt_mask));
    hdmi_mask(HD21_channel_align_r_reg,
        ~(HD21_channel_align_b_unex_comma_mask|
           HD21_channel_align_b_position_error_flag_mask|
           HD21_channel_align_b_position_error_cnt_mask),
           (HD21_channel_align_b_unex_comma_mask|
           HD21_channel_align_b_position_error_flag_mask|
           HD21_channel_align_b_position_error_cnt_mask));
    hdmi_mask(HD21_channel_align_g_reg,
        ~(HD21_channel_align_b_unex_comma_mask|
           HD21_channel_align_b_position_error_flag_mask|
           HD21_channel_align_b_position_error_cnt_mask),
           (HD21_channel_align_b_unex_comma_mask|
           HD21_channel_align_b_position_error_flag_mask|
           HD21_channel_align_b_position_error_cnt_mask));
    hdmi_mask(HD21_channel_align_b_reg,
        ~(HD21_channel_align_b_unex_comma_mask|
           HD21_channel_align_b_position_error_flag_mask|
           HD21_channel_align_b_position_error_cnt_mask),
           (HD21_channel_align_b_unex_comma_mask|
           HD21_channel_align_b_position_error_flag_mask|
           HD21_channel_align_b_position_error_cnt_mask));
}


void lib_hdmi_hd21_data_align_status(unsigned char nport, unsigned char lane)
{
    unsigned int reg_addr = HD21_channel_align_b_reg - (lane << 2);

    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    switch(lane_st[lane].state)
    {
    case init_state:
        if (HD21_channel_align_b_get_lock_bound(hdmi_in(reg_addr)))
            lane_st[lane].state = comma_seq_search_state;
        break;

    case comma_seq_search_state:
    {
        unsigned int hd21_channel_align_value = hdmi_in(reg_addr);
        unsigned int check_unex_comma =  HD21_channel_align_b_get_unex_comma(hd21_channel_align_value);
        unsigned int check_position_error_flag = HD21_channel_align_b_get_position_error_flag(hd21_channel_align_value);
        unsigned int check_position_error_cnt = HD21_channel_align_b_get_position_error_cnt (hd21_channel_align_value);
        unsigned int check_sr_detect = HD21_channel_align_b_get_sr_detect(hd21_channel_align_value);

        if (lane_st[lane].debounce_cnt >= DATA_ALIGN_DEBOUNCE_CNT_THD)
        {  //need to 500us  10super block total 16320 bit
            lane_st[lane].state = comma_seq_lock_state;
            //Use TX Mode, //hdmi_mask(reg_addr,~(HDMI21_P0_hd21_channel_align_b_com_seq_lock_mask),HDMI21_P0_hd21_channel_align_b_com_seq_lock_mask); // Use RX mode if TX didnot follow spec.
            break;
        }
        else
        {
             if ((check_unex_comma == 0) &&
                 (check_position_error_flag == 0) &&
                 (check_position_error_cnt == 0) &&
                 check_sr_detect)
             {
                 lane_st[lane].debounce_cnt++;
             }
             else
                 lane_st[lane].debounce_cnt = 0;

        }
        lane_st[lane].timeout++;

        if (lane_st[lane].timeout > DATA_ALIGN_TIMEOUT_THD)
        {
            HDMI_WARN("lane %d, lock timeout, reg_addr=%x, reg_val=%x, check_unex_comma=%d, check_position_error_flag=%d, check_position_error_cnt=%d, check_sr_detect=%d, debounce_cnt=%d\n",
            lane,
            reg_addr,
            hdmi_in(reg_addr),
            check_unex_comma,
            check_position_error_flag,
            check_position_error_cnt,
            check_sr_detect,
            lane_st[lane].debounce_cnt);

            lane_st[lane].state = init_state;
            lane_st[lane].debounce_cnt = 0;
            lane_st[lane].timeout = 0;
            hdmi_mask(reg_addr,~(HD21_channel_align_b_force_realign_mask),HD21_channel_align_b_force_realign_mask);
        }
    }
        break;

    case comma_seq_lock_state:
    {
        unsigned int hd21_channel_align_value = hdmi_in(reg_addr);
        unsigned int check_lock_bound = HD21_channel_align_b_get_lock_bound(hd21_channel_align_value);
        unsigned int check_unex_comma =  HD21_channel_align_b_get_unex_comma(hd21_channel_align_value);
        unsigned int check_position_error_flag = HD21_channel_align_b_get_position_error_flag(hd21_channel_align_value);
        unsigned int check_position_error_cnt = HD21_channel_align_b_get_position_error_cnt (hd21_channel_align_value);
        unsigned int check_sr_detect = HD21_channel_align_b_get_sr_detect(hd21_channel_align_value);

        if ((!check_lock_bound) ||
              check_unex_comma ||
              check_position_error_flag ||
             (check_position_error_cnt != 0) ||
             (!check_sr_detect))
        {
            lane_st[lane].state = init_state;
            lane_st[lane].debounce_cnt = 0;
            lane_st[lane].timeout = 0;
            hdmi_mask(reg_addr,~(HD21_channel_align_b_com_seq_lock_mask),0x0);
            hdmi_mask(reg_addr,~(HD21_channel_align_b_force_realign_mask),HD21_channel_align_b_force_realign_mask);
            HDMI_WARN("lane %d, comma_seq_lock_state check fail, lock_bound=%d, unex_comma=%d, position_error_flag=%d, position_error_cnt=%d, sr_detect=%d\n", lane, check_lock_bound, check_unex_comma, check_position_error_flag, check_position_error_cnt, check_sr_detect);
        }
    }
        break;
    }
}

/*---------------------------------------------------
 * Func : lib_hdmi_hd21_is_data_align_lock
 *
 * Desc : check data align lock or not
 *
 * Para : nport: physical hdmi2.1 port, lane: data lane
 *
 * Retn : 0: lock fail, 1: lock success
 *--------------------------------------------------*/
unsigned char lib_hdmi_hd21_is_data_align_lock(unsigned char nport, unsigned char lane)
{
    unsigned int reg_addr = 0;
    unsigned int hd21_channel_align_value = 0;
    unsigned int check_lock_bound = 0;
    unsigned int check_unex_comma =  0;
    unsigned int check_position_error_flag = 0;
    unsigned int check_position_error_cnt = 0;
    unsigned int check_sr_detect = 0;

    if (!lib_hdmi_is_hdmi_21_available(nport))
        return 0;

    reg_addr = HD21_channel_align_b_reg - (lane << 2);

    hd21_channel_align_value = hdmi_in(reg_addr);
    check_lock_bound = HD21_channel_align_b_get_lock_bound(hd21_channel_align_value);
    check_unex_comma =  HD21_channel_align_b_get_unex_comma(hd21_channel_align_value);
    check_position_error_flag = HD21_channel_align_b_get_position_error_flag(hd21_channel_align_value);
    check_position_error_cnt = HD21_channel_align_b_get_position_error_cnt (hd21_channel_align_value);
    check_sr_detect = HD21_channel_align_b_get_sr_detect(hd21_channel_align_value);

    if ((!check_lock_bound) ||
        check_unex_comma ||
        check_position_error_flag ||
        (check_position_error_cnt != 0) ||
        (!check_sr_detect))
    {
        HD21_WARN("[HD21] Port:%d Check data align fail!! lane=%d, lock_bound=%d, unex_comma=%d, position_error_flag=%d, position_error_cnt=%d, sr_detect=%d\n",
            nport, lane, check_lock_bound, check_unex_comma, check_position_error_flag, check_position_error_cnt, check_sr_detect);

        if (newbase_hdmi_get_power_saving_state(nport)==PS_FSM_POWER_SAVING_ON)
            return 1;

        return 0;
    }
    else
    {
        return 1;
    }
}

/*---------------------------------------------------
 * Func : lib_hdmi_hd21_force_realign
 *
 * Desc : set r/g/b/c force_realign bit
 *
 * Para : nport: port number
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_hd21_force_realign(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    hdmi_mask(HD21_channel_align_c_reg, ~HD21_channel_align_c_align_mode_mask, HD21_channel_align_c_align_mode_mask);
    hdmi_mask(HD21_channel_align_r_reg, ~HD21_channel_align_r_align_mode_mask, HD21_channel_align_r_align_mode_mask);
    hdmi_mask(HD21_channel_align_g_reg, ~HD21_channel_align_g_align_mode_mask, HD21_channel_align_g_align_mode_mask);
    hdmi_mask(HD21_channel_align_b_reg, ~HD21_channel_align_b_align_mode_mask, HD21_channel_align_b_align_mode_mask);

    hdmi_mask(HD21_channel_align_c_reg,~(HD21_channel_align_c_force_realign_mask),HD21_channel_align_c_force_realign_mask);
    hdmi_mask(HD21_channel_align_r_reg,~(HD21_channel_align_r_force_realign_mask),HD21_channel_align_r_force_realign_mask);
    hdmi_mask(HD21_channel_align_g_reg,~(HD21_channel_align_g_force_realign_mask),HD21_channel_align_g_force_realign_mask);
    hdmi_mask(HD21_channel_align_b_reg,~(HD21_channel_align_b_force_realign_mask),HD21_channel_align_b_force_realign_mask);

    hdmi_mask(HD21_channel_align_c_reg,~(HD21_channel_align_c_scdc_force_realign_mask),HD21_channel_align_c_scdc_force_realign_mask);
    hdmi_mask(HD21_channel_align_r_reg,~(HD21_channel_align_r_scdc_force_realign_mask),HD21_channel_align_r_scdc_force_realign_mask);
    hdmi_mask(HD21_channel_align_g_reg,~(HD21_channel_align_g_scdc_force_realign_mask),HD21_channel_align_g_scdc_force_realign_mask);
    hdmi_mask(HD21_channel_align_b_reg,~(HD21_channel_align_b_scdc_force_realign_mask),HD21_channel_align_b_scdc_force_realign_mask);

    HDMI_INFO("lib_hdmi_hd21_force_realign port:%d\n", nport);
}

/*---------------------------------------------------
 * Func : lib_hdmi_hd21_dynamic_realign
 *
 * Desc : set r/g/b/c force_realign bit
 *
 * Para : nport: port number
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_hd21_dynamic_realign(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    if(HD21_channel_align_c_get_force_realign(hdmi_in(HD21_channel_align_c_reg)))
    {
        hdmi_mask(HD21_channel_align_c_reg,~(HD21_channel_align_c_force_realign_mask),0);
    }
    if(HD21_channel_align_r_get_force_realign(hdmi_in(HD21_channel_align_r_reg)))
    {
        hdmi_mask(HD21_channel_align_r_reg,~(HD21_channel_align_r_force_realign_mask),0);
    }
    if(HD21_channel_align_g_get_force_realign(hdmi_in(HD21_channel_align_g_reg)))
    {
        hdmi_mask(HD21_channel_align_g_reg,~(HD21_channel_align_g_force_realign_mask),0);
    }
    if(HD21_channel_align_b_get_force_realign(hdmi_in(HD21_channel_align_b_reg)))
    {
        hdmi_mask(HD21_channel_align_b_reg,~(HD21_channel_align_b_force_realign_mask),0);
    }

    HDMI_INFO("lib_hdmi_hd21_dynamic_realign port:%d\n", nport);
}

//------------------------------------------------------------------
// Timing Gen
//------------------------------------------------------------------

void lib_hdmi_hd21_timing_gen_enable(unsigned char nport,unsigned char enable)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    HDMI_INFO("[lib_hdmi_hd21_timing_gen_enable port:%d] enable=%d\n", nport, enable);

    if (enable)
    {
        hdmi_out(HD21_TIMING_GEN_SR_reg, hdmi_in(HD21_TIMING_GEN_SR_reg));  // clear timing gen status
        hdmi_mask(HD21_TIMING_GEN_CR_reg, ~HD21_TIMING_GEN_CR_tg_en_mask, HD21_TIMING_GEN_CR_tg_en_mask);   // enable timing gen
    }
    else
    {
        hdmi_mask(HD21_TIMING_GEN_CR_reg, ~HD21_TIMING_GEN_CR_tg_en_mask, 0);     // disable timing gen
        hdmi_out(HD21_TIMING_GEN_SR_reg, hdmi_in(HD21_TIMING_GEN_SR_reg));  // clear timing gen status
    }
}


void lib_hdmi_hd21_timing_gen_check_error(unsigned char nport,unsigned char frl_mode)
{
    unsigned int tg_error;

    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    if (frl_mode == MODE_TMDS)
        return;

    tg_error = (HD21_TIMING_GEN_SR_de_data_mismatch_mask|HD21_TIMING_GEN_SR_dpc2tg_linebuff_mask|
               HD21_TIMING_GEN_SR_ms2tg_fifof_mask|HD21_TIMING_GEN_SR_video_timing_tog_err_mask);

    hdmi_mask(HD21_TIMING_GEN_SR_reg, ~tg_error, tg_error);  //write clear

    if (hdmi_in(HD21_TIMING_GEN_SR_reg)) //toggle tg enable
    {  
        hdmi_mask(HD21_TIMING_GEN_CR_reg,~HD21_TIMING_GEN_CR_tg_en_mask,0x0);
        udelay(20);
        hdmi_mask(HD21_TIMING_GEN_CR_reg,~HD21_TIMING_GEN_CR_tg_en_mask,HD21_TIMING_GEN_CR_tg_en_mask);

        hdmi_mask(HD21_TIMING_GEN_SR_reg,~tg_error,tg_error);  //write clear

        if (hdmi_in(HD21_TIMING_GEN_SR_reg)) {
            HD21_WARN("timing gen error\n");
        }
    }
}

void lib_hdmi_hd21_clk_div_en(unsigned char nport, unsigned char enable)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    hdmi_mask(HD21_CR_reg, ~HD21_CR_clk_div_en_mask, HD21_CR_clk_div_en(enable));

    HDMI_INFO("[lib_hdmi_hd21_clk_div_en nport=%d] enable=%d \n", nport, enable);
}

unsigned int lib_hdmi_hd21_is_clk_div_en(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return 0;

    return HD21_CR_get_clk_div_en(hdmi_in(HD21_CR_reg));
}

void lib_hdmi_hd21_timing_gen_reset_toggle(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    HDMI_INFO("[lib_hdmi_hd21_timing_gen_reset_toggle port:%d] Clear status and Toggle TG 0 -> 1\n", nport);
    #if 0//RX mode should implement with disparity error detect.
    hdmi_mask(HD21_channel_align_c_reg, ~HD21_channel_align_c_com_seq_lock_mask, HD21_channel_align_c_com_seq_lock_mask);
    hdmi_mask(HD21_channel_align_r_reg, ~HD21_channel_align_r_com_seq_lock_mask, HD21_channel_align_r_com_seq_lock_mask);
    hdmi_mask(HD21_channel_align_g_reg, ~HD21_channel_align_g_com_seq_lock_mask, HD21_channel_align_g_com_seq_lock_mask);
    hdmi_mask(HD21_channel_align_b_reg, ~HD21_channel_align_b_com_seq_lock_mask, HD21_channel_align_b_com_seq_lock_mask);
    #else
    hdmi_mask(HD21_channel_align_c_reg, ~HD21_channel_align_c_com_seq_lock_mask, 0);
    hdmi_mask(HD21_channel_align_r_reg, ~HD21_channel_align_r_com_seq_lock_mask, 0);
    hdmi_mask(HD21_channel_align_g_reg, ~HD21_channel_align_g_com_seq_lock_mask, 0);
    hdmi_mask(HD21_channel_align_b_reg, ~HD21_channel_align_b_com_seq_lock_mask, 0);

    hdmi_mask(HD21_channel_align_c_reg, ~HD21_channel_align_c_align_mode_mask, HD21_channel_align_c_align_mode_mask);
    hdmi_mask(HD21_channel_align_r_reg, ~HD21_channel_align_r_align_mode_mask, HD21_channel_align_r_align_mode_mask);
    hdmi_mask(HD21_channel_align_g_reg, ~HD21_channel_align_g_align_mode_mask, HD21_channel_align_g_align_mode_mask);
    hdmi_mask(HD21_channel_align_b_reg, ~HD21_channel_align_b_align_mode_mask, HD21_channel_align_b_align_mode_mask);
    #endif
    hdmi_mask(HD21_TIMING_GEN_CR_reg, ~HD21_TIMING_GEN_CR_tg_en_mask, 0);     // disable timing gen
    hdmi_out(HD21_TIMING_GEN_SR_reg, hdmi_in(HD21_TIMING_GEN_SR_reg));  // clear timing gen status
    hdmi_mask(HD21_TIMING_GEN_CR_reg, ~HD21_TIMING_GEN_CR_tg_en_mask, HD21_TIMING_GEN_CR_tg_en_mask);   // enable timing gen
    //HD21_WARN("HD21_TIMING_GEN_CR_reg = %08x, HD21_TIMING_GEN_SR_reg = %08x\n", hdmi_in(HD21_TIMING_GEN_CR_reg), hdmi_in(HD21_TIMING_GEN_SR_reg));
}

unsigned int lib_hdmi_hd21_get_timing_gen_status(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return 0xFFFFFFFF;

    return  hdmi_in(HD21_TIMING_GEN_SR_reg);
}

//------------------------------------------------------------------
// Error Status
//------------------------------------------------------------------

void lib_hdmi_hd21_fw_disparity_error_reset(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return ;

    // clear status of disparity error
    hdmi_mask(HD21_errdet_r_disparity_reg, ~HD21_errdet_r_disparity_error_cnt_acc_mask, 0xFFFF);

    hdmi_mask(HD21_errdet_g_disparity_reg, ~HD21_errdet_g_disparity_error_cnt_acc_mask, 0xFFFF);

    hdmi_mask(HD21_errdet_b_disparity_reg, ~HD21_errdet_b_disparity_error_cnt_acc_mask, 0xFFFF);

    hdmi_mask(HD21_errdet_c_disparity_reg, ~HD21_errdet_c_disparity_error_cnt_acc_mask, 0xFFFF);
    HDMI_INFO("lib_hdmi_hd21_fw_disparity_error_reset, port = %d\n", nport);
}

void lib_hdmi_hd21_fw_symbol_error_reset(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return ;

    // clear status of symbol error
    hdmi_mask(HD21_errdet_r_char_reg, ~HD21_errdet_r_char_error_cnt_acc_mask, 0xFFFF);

    hdmi_mask(HD21_errdet_g_char_reg, ~HD21_errdet_g_char_error_cnt_acc_mask, 0xFFFF);

    hdmi_mask(HD21_errdet_b_char_reg, ~HD21_errdet_b_char_error_cnt_acc_mask, 0xFFFF);

    hdmi_mask(HD21_errdet_c_char_reg, ~HD21_errdet_c_char_error_cnt_acc_mask, 0xFFFF);
    HDMI_INFO("lib_hdmi_hd21_fw_symbol_error_reset, port = %d\n", nport);
}

/*---------------------------------------------------
 * Func : lib_hdmi_hd21_fw_disparity_symbol_error_enable
 *
 * Desc : Enable count of r/g/b/c disparity and symbol error, and init to 0. 
 *
 * Para : N/A
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_hd21_fw_disparity_symbol_error_enable(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return ;

#ifdef HDMI_FIX_RL6583_2562
    // ignore the first 2 super blocks after lock for disparity check to make sure
    // tx/rx disparity is aligned
    hdmi_mask(HD21_disp_ctrl_reg, ~(HD21_disp_ctrl_mask_sb_cnt_mask|HD21_disp_ctrl_mask_en_mask),
                                   (HD21_disp_ctrl_mask_sb_cnt_mask|HD21_disp_ctrl_mask_sb_cnt(2)));
#endif

    // disparity error enable
    hdmi_mask(HD21_errdet_r_ctrl_reg, ~HDMI21_P0_hd21_errdet_r_ctrl_disp_en_mask,
                                             HD21_errdet_r_ctrl_disp_en(1));

    hdmi_mask(HD21_errdet_g_ctrl_reg, ~HDMI21_P0_hd21_errdet_g_ctrl_disp_en_mask,
                                             HD21_errdet_g_ctrl_disp_en(1));

    hdmi_mask(HD21_errdet_b_ctrl_reg, ~HDMI21_P0_hd21_errdet_b_ctrl_disp_en_mask,
                                             HD21_errdet_b_ctrl_disp_en(1));

    hdmi_mask(HD21_errdet_c_ctrl_reg, ~HDMI21_P0_hd21_errdet_c_ctrl_disp_en_mask,
                                             HD21_errdet_c_ctrl_disp_en(1));
    // char error enable
    hdmi_mask(HD21_errdet_r_ctrl_reg, ~HD21_errdet_r_ctrl_char_en_mask,
                                       HD21_errdet_r_ctrl_char_en(1));

    hdmi_mask(HD21_errdet_g_ctrl_reg, ~HD21_errdet_g_ctrl_char_en_mask,
                                       HD21_errdet_g_ctrl_char_en(1));

    hdmi_mask(HD21_errdet_b_ctrl_reg, ~HD21_errdet_b_ctrl_char_en_mask,
                                       HD21_errdet_b_ctrl_char_en(1));

    hdmi_mask(HD21_errdet_c_ctrl_reg, ~HD21_errdet_c_ctrl_char_en_mask,
                                       HD21_errdet_c_ctrl_char_en(1));

    HDMI_INFO("lib_hdmi_hd21_fw_disparity_symbol_error_enable, port = %d\n", nport);
}

/*---------------------------------------------------
 * Func : lib_hdmi_hd21_fw_disparity_symbol_error_disable
 *
 * Desc : Enable count of r/g/b/c disparity and symbol error, and init to 0. 
 *
 * Para : N/A
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_hd21_fw_disparity_symbol_error_disable(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return ;

    // disparity error disable
    hdmi_mask(HD21_errdet_r_ctrl_reg, ~HDMI21_P0_hd21_errdet_r_ctrl_disp_en_mask,
                                             HD21_errdet_r_ctrl_disp_en(0));

    hdmi_mask(HD21_errdet_g_ctrl_reg, ~HDMI21_P0_hd21_errdet_g_ctrl_disp_en_mask,
                                             HD21_errdet_g_ctrl_disp_en(0));

    hdmi_mask(HD21_errdet_b_ctrl_reg, ~HDMI21_P0_hd21_errdet_b_ctrl_disp_en_mask,
                                             HD21_errdet_b_ctrl_disp_en(0));

    hdmi_mask(HD21_errdet_c_ctrl_reg, ~HDMI21_P0_hd21_errdet_c_ctrl_disp_en_mask,
                                             HD21_errdet_c_ctrl_disp_en(0));

    // char error disable
    hdmi_mask(HD21_errdet_r_ctrl_reg, ~HD21_errdet_r_ctrl_char_en_mask,
                                       HD21_errdet_r_ctrl_char_en(0));

    hdmi_mask(HD21_errdet_g_ctrl_reg, ~HD21_errdet_g_ctrl_char_en_mask,
                                       HD21_errdet_g_ctrl_char_en(0));

    hdmi_mask(HD21_errdet_b_ctrl_reg, ~HD21_errdet_b_ctrl_char_en_mask,
                                       HD21_errdet_b_ctrl_char_en(0));

    hdmi_mask(HD21_errdet_c_ctrl_reg, ~HD21_errdet_c_ctrl_char_en_mask,
                                             HD21_errdet_c_ctrl_char_en(0));
    HDMI_INFO("lib_hdmi_hd21_fw_disparity_symbol_error_disable, port = %d\n", nport);
}

void lib_hdmi_hd21_fw_disparity_error_status(unsigned char nport, unsigned int disparity_err[4])
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return ;

    disparity_err[0] = (hdmi_in(HD21_errdet_b_disparity_reg) &  HD21_errdet_b_disparity_error_cnt_acc_mask);
    disparity_err[1] = (hdmi_in(HD21_errdet_g_disparity_reg) &  HD21_errdet_g_disparity_error_cnt_acc_mask);
    disparity_err[2] = (hdmi_in(HD21_errdet_r_disparity_reg) &  HD21_errdet_r_disparity_error_cnt_acc_mask);
    disparity_err[3] = (hdmi_in(HD21_errdet_c_disparity_reg) &  HD21_errdet_c_disparity_error_cnt_acc_mask);
}

void lib_hdmi_hd21_fw_symbol_error_status(unsigned char nport, unsigned int symbol_err[4])
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return ;

    symbol_err[0] = (hdmi_in(HD21_errdet_b_char_reg) &  HD21_errdet_b_char_error_cnt_acc_mask);
    symbol_err[1] = (hdmi_in(HD21_errdet_g_char_reg) &  HD21_errdet_g_char_error_cnt_acc_mask);
    symbol_err[2] = (hdmi_in(HD21_errdet_r_char_reg) &  HD21_errdet_r_char_error_cnt_acc_mask);
    symbol_err[3] = (hdmi_in(HD21_errdet_c_char_reg) &  HD21_errdet_c_char_error_cnt_acc_mask);
}

unsigned int lib_hdmi_hd21_get_rs_err_cnt(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return 0;

    return HD21_CERSR2_get_rs_err_cnt(hdmi_in(HD21_CERSR2_reg));
}

//------------------------------------------------------------------
// FEC RS Control
//------------------------------------------------------------------

void lib_hdmi_hd21_fec_rs_decode_enable(unsigned char nport, unsigned char enable)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return ;

    hdmi_mask(HD21_FEC_FEC_HDMI21_RS_0_reg, ~HD21_FEC_FEC_HDMI21_RS_0_rs_dc_en_mask, HD21_FEC_FEC_HDMI21_RS_0_rs_dc_en(enable));
}

void lib_hdmi_hd21_clr_rsed_upd_flag(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return ;

    hdmi_mask(HD21_FEC_FEC_HDMI21_RS_3_reg, ~HD21_FEC_FEC_HDMI21_RS_3_rsed_upd_mask, HD21_FEC_FEC_HDMI21_RS_3_rsed_upd_mask); //write 1 clear
}

unsigned char lib_hdmi_hd21_get_rsed_upd_flag(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return 0;

    return HD21_FEC_FEC_HDMI21_RS_3_get_rsed_upd(hdmi_in(HD21_FEC_FEC_HDMI21_RS_3_reg));
}


/*---------------------------------------------------
 * Func : lib_hdmi_hd21_fec_scdc_rs_cnt_enable
 *
 * Desc : Enable for getting status of SCDC 0x58, 0x59
 *
 * Para : nport : hdmi port id
 *        enable  : TRUE=1, FALSE=0
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_hd21_fec_scdc_rs_cnt_enable(unsigned char nport, unsigned char enable)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return ;

    hdmi_mask(HD21_FEC_FEC_HDMI21_RS_0_reg, ~HD21_FEC_FEC_HDMI21_RS_0_rs_cnt_en_mask, HD21_FEC_FEC_HDMI21_RS_0_rs_cnt_en(enable));
    //test for MOI
    hdmi_mask(HD21_FEC_FEC_HDMI21_RS_0_reg, ~HD21_FEC_FEC_HDMI21_RS_0_rs_cnt_valid_mask, HD21_FEC_FEC_HDMI21_RS_0_rs_cnt_valid_mask);
    hdmi_mask(HD21_FEC_FEC_HDMI21_RS_3_reg, ~HD21_FEC_FEC_HDMI21_RS_3_rsed_upd_en_mask, HD21_FEC_FEC_HDMI21_RS_3_rsed_upd_en_mask);
    hdmi_mask(HD21_FEC_FEC_HDMI21_RS_3_reg, ~HD21_FEC_FEC_HDMI21_RS_3_rsed_upd_once_mask, 0);
}

void lib_hdmi_hd21_fec_rs_cnt_keep_enable(unsigned char nport, unsigned char enable)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return ;

    hdmi_mask(HD21_FEC_FEC_HDMI21_RS_0_reg, ~HD21_FEC_FEC_HDMI21_RS_0_rs_cnt_keep_mask, HD21_FEC_FEC_HDMI21_RS_0_rs_cnt_keep((enable) ? 1:0));
}

void lib_hdmi_hd21_fec_frl_det_enable(unsigned char nport, unsigned char enable)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return ;

    hdmi_mask(HD21_FEC_FEC_HDMI21_FRL_0_reg, ~HD21_FEC_FEC_HDMI21_FRL_0_frl_det_en_mask, HD21_FEC_FEC_HDMI21_FRL_0_frl_det_en((enable) ? 1:0));
}

unsigned char lib_hdmi_hd21_get_fec_v_pkt_det(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return 0;

    return HD21_FEC_FEC_HDMI21_FRL_0_get_v_pkt_det(hdmi_in(HD21_FEC_FEC_HDMI21_FRL_0_reg));
}

void lib_hdmi_hd21_clr_v_pkt_det(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    hdmi_mask(HD21_FEC_FEC_HDMI21_FRL_0_reg, ~HD21_FEC_FEC_HDMI21_FRL_0_v_pkt_det_mask, HD21_FEC_FEC_HDMI21_FRL_0_v_pkt_det_mask ); //write 1 clear

    return;
}

unsigned char lib_hdmi_hd21_get_fec_frl_hs(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return 0;

    return HD21_FEC_FEC_HDMI21_FRL_0_get_frl_hs(hdmi_in(HD21_FEC_FEC_HDMI21_FRL_0_reg));
}

void lib_hdmi_hd21_get_clr_frl_hs(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    hdmi_mask(HD21_FEC_FEC_HDMI21_FRL_0_reg, ~HD21_FEC_FEC_HDMI21_FRL_0_frl_hs_mask, HD21_FEC_FEC_HDMI21_FRL_0_frl_hs_mask ); //write 1 clear

    return;
}
unsigned char lib_hdmi_hd21_get_fec_frl_vs(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return 0;

    return HD21_FEC_FEC_HDMI21_FRL_0_get_frl_vs(hdmi_in(HD21_FEC_FEC_HDMI21_FRL_0_reg));
}

void lib_hdmi_hd21_clr_fec_frl_vs(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    hdmi_mask(HD21_FEC_FEC_HDMI21_FRL_0_reg, ~HD21_FEC_FEC_HDMI21_FRL_0_frl_vs_mask, HD21_FEC_FEC_HDMI21_FRL_0_frl_vs_mask ); //write 1 clear

    return;
}

/*---------------------------------------------------
 * Func : lib_hdmi_hd21_fec_set_fw_vs_polarity
 *
 * Desc : set fw mode vs polarity
 *
 * Para : nport : hdmi port id
 *        fw_mode : 0 : hw mode, 1 : fw mode
 *        vs_inv : 0 :  non inverse, 1 : inverse
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_hd21_fec_set_fw_vs_polarity(unsigned char nport, unsigned char fw_mode, unsigned char vs_inv)
{
    if (lib_hdmi_is_hdmi_21_available(nport))
    {
        HDMI_INFO("Port = %d, set_fw_vs_polarity (fw_mode=%d, vs_inv= %d, before v pol=%d)\n",
            nport, fw_mode, vs_inv, lib_hdmi_hd21_meas_get_vs_polarity(nport));

        hdmi_mask(HD21_FEC_FEC_HDMI21_DI_0_reg,
           ~(HD21_FEC_FEC_HDMI21_DI_0_vs_polar_fw_mode_mask|
            HD21_FEC_FEC_HDMI21_DI_0_vs_polar_invert_mask),
            HD21_FEC_FEC_HDMI21_DI_0_vs_polar_fw_mode((fw_mode) ? 1 : 0)|
            HD21_FEC_FEC_HDMI21_DI_0_vs_polar_invert((vs_inv) ? 1 : 0));
    }
}

unsigned char lib_hdmi_hd21_fec_get_vs_polar_invert(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return 0;

    return HD21_FEC_FEC_HDMI21_DI_0_get_vs_polar_invert(hdmi_in(HD21_FEC_FEC_HDMI21_DI_0_reg));
}

//------------------------------------------------------------------
// Measure
//------------------------------------------------------------------

void lib_hdmi_hd21_fec_measure_enable(unsigned char nport, unsigned char is_dsc_mode, unsigned char enable)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    if(is_dsc_mode == TRUE)
    {
        hdmi_mask(HD21_MEAS_FRL_MEASURE_3_reg, ~HD21_MEAS_FRL_MEASURE_3_hs_sel_mask, HD21_MEAS_FRL_MEASURE_3_hs_sel_mask); // //virtual h sync
        hdmi_mask(HD21_MEAS_FRL_MEASURE_3_reg, ~HD21_MEAS_FRL_MEASURE_3_ht_meas_sel_mask, HD21_MEAS_FRL_MEASURE_3_ht_meas_sel_mask); //DE-TO-DE
        //hdmi_mask(HD21_MEAS_FRL_MEASURE_9_reg, ~HD21_MEAS_FRL_MEASURE_9_pol_det_mode_mask, 0);
    }
    else
    {
        hdmi_mask(HD21_MEAS_FRL_MEASURE_3_reg, ~HD21_MEAS_FRL_MEASURE_3_hs_sel_mask, 0); // //real h sync
        hdmi_mask(HD21_MEAS_FRL_MEASURE_3_reg, ~HD21_MEAS_FRL_MEASURE_3_ht_meas_sel_mask, 0); //hsync-to-hsync
        //hdmi_mask(HD21_MEAS_FRL_MEASURE_9_reg, ~HD21_MEAS_FRL_MEASURE_9_pol_det_mode_mask, HD21_MEAS_FRL_MEASURE_9_pol_det_mode_mask);
    }

    hdmi_mask(HD21_MEAS_FRL_MEASURE_0_reg, ~HD21_MEAS_FRL_MEASURE_0_measure_en_mask, HD21_MEAS_FRL_MEASURE_0_measure_en((enable) ? 1: 0));

}

unsigned char lib_hdmi_hd21_fec_is_measure_completed(unsigned char nport)
{
    if (lib_hdmi_is_hdmi_21_available(nport))
        return (HD21_MEAS_FRL_MEASURE_0_get_measure_en(hdmi_in(HD21_MEAS_FRL_MEASURE_0_reg))==0) ? 1 : 0;
    else
        return 0;
}

void lib_hdmi_hd21_fec_measure_restart(unsigned char nport, unsigned char is_dsc_mode)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    lib_hdmi_hd21_fec_measure_enable(nport, is_dsc_mode, FALSE);
    lib_hdmi_hd21_fec_measure_enable(nport, is_dsc_mode, TRUE);
}

unsigned char lib_hdmi_hd21_vsync_checked(unsigned char nport)
{
    unsigned char i;

    if (!lib_hdmi_is_hdmi_21_available(nport))
        return FALSE;

    if(HD21_FEC_FEC_HDMI21_FRL_0_get_frl_det_en(hdmi_in(HD21_FEC_FEC_HDMI21_FRL_0_reg)) == 0)
    {
        HDMI_EMG("[lib_hdmi_hd21_vsync_checked port:%d] Coding Error!!!! det_en must set 1\n", nport);
        return FALSE;
    }

    lib_hdmi_hd21_clr_fec_frl_vs(nport);
    for (i=0; i<10; i++) {
        if (lib_hdmi_hd21_get_fec_frl_vs(nport)) {
            return TRUE;
        }
        msleep(10);
    }
    HDMI_PRINTF("[lib_hdmi_hd21_vsync_checked] Vsync cannot seen!!! port=%d\n", nport);
    return FALSE;
}

void lib_hdmi_hd21_meas_clear_interlace_reg(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    hdmi_mask(HD21_MEAS_FRL_MEASURE_A_reg, ~HD21_MEAS_FRL_MEASURE_A_field_tog_mask, HD21_MEAS_FRL_MEASURE_A_field_tog_mask); //write 1 clear
    hdmi_mask(HD21_MEAS_FRL_MEASURE_A_reg, ~HD21_MEAS_FRL_MEASURE_A_interlaced_err_mask, HD21_MEAS_FRL_MEASURE_A_interlaced_err_mask); //write 1 clear
}

HDMI_INT_PRO_CHECK_RESULT lib_hdmi_hd21_meas_get_interlace(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return HDMI_IPCHECK_PROGRESSIVE;

    return HD21_MEAS_FRL_MEASURE_A_get_field_tog(hdmi_in(HD21_MEAS_FRL_MEASURE_A_reg));
}

/*---------------------------------------------------
 * Func : lib_hdmi_hd21_meas_get_vs_polarity
 *
 * Desc : get hdcp vs polarity
 *
 * Para : nport : hdmi port id
 *
 * Retn : 0 : Positive VS, 1 : Negative VS
 *--------------------------------------------------*/
unsigned char lib_hdmi_hd21_meas_get_vs_polarity(unsigned char nport)
{
    if (lib_hdmi_is_hdmi_21_available(nport))
        return HD21_MEAS_FRL_MEASURE_9_get_vs_polarity(hdmi_in(HD21_MEAS_FRL_MEASURE_9_reg));
    return 0;
}



/*---------------------------------------------------
 * Func : lib_hdmi_hd21_calc_hfreq
 *
 * Desc : calculate hfreq by hs_period
 *
 * Para : nport : hdmi port id
 *
 * Retn : hfreq
 *--------------------------------------------------*/
unsigned int lib_hdmi_hd21_calc_hfreq(unsigned char nport)
{
    unsigned int hfreq = 0;
    unsigned long long temp_hfreq = 0;
    unsigned long long hs_period = (unsigned long long)HD21_MEAS_FRL_MEASURE_18_get_hs_period(hdmi_in(HD21_MEAS_FRL_MEASURE_18_reg));

    if(hs_period!=0)
    {
        temp_hfreq = 27000000ULL;
        do_div(temp_hfreq, hs_period);
        hfreq = (unsigned int)temp_hfreq;
        return hfreq;
    }
    else
    {
        HDMI_WARN("[HD21MS] Measure Fail!!Divide 0!! hs_period=%lld \n", hs_period);
        return 0;
    }
}
static unsigned int lib_hdmi21_measure_get_hs_sta(unsigned char nport)
{
    unsigned int hs_width = 0;
    unsigned int hback_cnt = 0;
    hs_width = HD21_MEAS_FRL_MEASURE_B_get_hs_width(hdmi_in(HD21_MEAS_FRL_MEASURE_B_reg));
    hback_cnt = HD21_MEAS_FRL_MEASURE_D_get_hback_cnt(hdmi_in(HD21_MEAS_FRL_MEASURE_D_reg));
    return (hs_width + hback_cnt);
}


static unsigned int lib_hdmi21_measure_get_vs_sta(unsigned char nport)
{
    unsigned int vback_cnt = 0;
    unsigned int vs_width = 0;
    vback_cnt = HD21_MEAS_FRL_MEASURE_14_get_vback_cnt(hdmi_in(HD21_MEAS_FRL_MEASURE_14_reg)); // vs back

    vs_width = HD21_MEAS_FRL_MEASURE_13_get_vs_width(hdmi_in(HD21_MEAS_FRL_MEASURE_13_reg)); // vs width
    return (vback_cnt + vs_width);
}


unsigned int m_hd21_measure_last_vs_cnt[4] = {0,0,0,0};
HDMI21_MEASURE_RESULT lib_hdmi21_hd21_measure(unsigned char nport, unsigned char is_dsc_mode, HDMI_TIMING_T* update_timing)
{
    unsigned int vs2vs_cnt = 0;
    unsigned char is_vsync_get = 0;
    unsigned char is_ps_measure_complete = FALSE;
    unsigned char is_fec_measure_complete = FALSE;

    if (!lib_hdmi_is_hdmi_21_available(nport))
        return HDMI21_MEASURE_SETTING_WRONG;

    if(HD21_FEC_FEC_HDMI21_FRL_0_get_frl_det_en(hdmi_in(HD21_FEC_FEC_HDMI21_FRL_0_reg)) == 0)
    {
        HDMI_EMG("[HD21MS] port:%d, Coding Error!!!! det_en must set 1, is_dsc_mode=%d\n", nport, is_dsc_mode);
        return HDMI21_MEASURE_SETTING_WRONG;
    }

    is_vsync_get = lib_hdmi_hd21_get_fec_frl_vs(nport);
    if((hdmi_rx[nport].video_t.vs_valid != 0) && (hdmi_rx[nport].video_t.vs_cnt != m_hd21_measure_last_vs_cnt[nport]))
    {//frame count update, enter new frame
        is_ps_measure_complete=1;
        m_hd21_measure_last_vs_cnt[nport] = hdmi_rx[nport].video_t.vs_cnt;
    }
    else
    {
        is_ps_measure_complete= 0;
    }
    is_fec_measure_complete = lib_hdmi_hd21_fec_is_measure_completed(nport);
    #if 0 //for debugging
    HDMI_EMG("is_vsync_get=%d, is_ps_measure_complete=%d,is_fec_measure_complete=%d ,hdmi_rx[nport].video_t.vs_valid=%d,video_t=%d,m_hd21_measure_last_vs_cnt=%d\n",
        is_vsync_get, is_ps_measure_complete, is_fec_measure_complete,hdmi_rx[nport].video_t.vs_valid, hdmi_rx[nport].video_t.vs_cnt, m_hd21_measure_last_vs_cnt[nport]);
    #endif
    if (is_vsync_get && is_ps_measure_complete && is_fec_measure_complete)
    {
        vs2vs_cnt = hdmi_rx[nport].video_t.vs2vs_cnt;
        if(vs2vs_cnt !=0)
        {
            update_timing->v_freq = lib_hdmims_calc_vfreq(vs2vs_cnt);
        }
        else
        {
            HDMI_WARN("[HD21MS] port:%d, zero V count!! is_dsc_mode=%d\n", nport, is_dsc_mode);
            update_timing->v_freq = 0;
        }
    }
    else
    {
        update_timing->v_total = 0;
        update_timing->v_freq= 0;
        //HDMI_WARN("[HD21MS] port:%dis_vsync_get=%d,  is_ps_measure_complete=%d, is_fec_measure_complete=%d\n", nport, is_vsync_get, is_ps_measure_complete, is_fec_measure_complete);
        return HDMI21_MEASURE_WAIT_V_SYNC;
    }

    update_timing->h_freq = lib_hdmi_hd21_calc_hfreq(nport);
    update_timing->h_total = HDMI21_P0_MEAS_FRL_MEASURE_11_get_htotal_cnt(hdmi_in(HD21_MEAS_FRL_MEASURE_11_reg));
    update_timing->h_act_len = HD21_MEAS_FRL_MEASURE_F_get_hact_cnt(hdmi_in(HD21_MEAS_FRL_MEASURE_F_reg));
    update_timing->h_act_sta = lib_hdmi21_measure_get_hs_sta(nport); // hact_sta =hs _width + h_back

    update_timing->v_total = HD21_MEAS_FRL_MEASURE_5_get_vt_line_cnt1(hdmi_in(HD21_MEAS_FRL_MEASURE_5_reg));
    update_timing->v_act_len = HDMI21_P0_MEAS_FRL_MEASURE_16_get_vact_cnt(hdmi_in(HD21_MEAS_FRL_MEASURE_16_reg));
    update_timing->v_act_sta = lib_hdmi21_measure_get_vs_sta(nport);   // vact_sta = vs_width + v_back

    update_timing->polarity = (HDMI21_P0_MEAS_FRL_MEASURE_9_get_vs_polarity(hdmi_in(HD21_MEAS_FRL_MEASURE_9_reg))<<1) |HDMI21_P0_MEAS_FRL_MEASURE_9_get_hs_polarity(hdmi_in(HD21_MEAS_FRL_MEASURE_9_reg));
    update_timing->IHSyncPulseCount = HD21_MEAS_FRL_MEASURE_B_get_hs_width(hdmi_in(HD21_MEAS_FRL_MEASURE_B_reg));
    update_timing->IVSyncPulseCount = HD21_MEAS_FRL_MEASURE_13_get_vs_width(hdmi_in(HD21_MEAS_FRL_MEASURE_13_reg));

    //Correction by Deep Color, Color Space, Pixel Repeat
    if (update_timing->pixel_repeat > 0) {
        update_timing->h_act_sta /= (update_timing->pixel_repeat+1);
        update_timing->h_total /= (update_timing->pixel_repeat+1);
        update_timing->h_act_len /= (update_timing->pixel_repeat+1);
        update_timing->IHSyncPulseCount /= (update_timing->IHSyncPulseCount+1);
    }

    if (COLOR_YUV420 == update_timing->colorspace && (lib_hdmi_get_420_fva_ds_mode(nport) == 0)) {
        update_timing->h_act_sta <<= 1;
        update_timing->h_total <<= 1;
        update_timing->h_act_len <<= 1;
        update_timing->IHSyncPulseCount <<= 1;
    }

    if (HDMI_COLOR_DEPTH_10B == update_timing->colordepth)
    {
        // value / 1.25
        update_timing->h_act_sta = (update_timing->h_act_sta*100/125);
        update_timing->h_total = (update_timing->h_total*100/125);
        update_timing->h_act_len = (update_timing->h_act_len*100/125);
        update_timing->IHSyncPulseCount = (update_timing->IHSyncPulseCount*100/125);

    }
    else if(HDMI_COLOR_DEPTH_12B == update_timing->colordepth)
    {
        // value / 1.5
        update_timing->h_act_sta = (update_timing->h_act_sta*100/150);
        update_timing->h_total =  (update_timing->h_total*100/150);
        update_timing->h_act_len =  (update_timing->h_act_len*100/150);
        update_timing->IHSyncPulseCount =  (update_timing->IHSyncPulseCount*100/150);
    }
    else if (HDMI_COLOR_DEPTH_16B == update_timing->colordepth)
    {
        // value / 2
        update_timing->h_act_sta = update_timing->h_act_sta>>1;
        update_timing->h_total = update_timing->h_total>>1;
        update_timing->h_act_len = update_timing->h_act_len>>1;
        update_timing->IHSyncPulseCount = update_timing->IHSyncPulseCount>>1;
    }

    //Reset status for next measure
    lib_hdmi_hd21_clr_fec_frl_vs(nport);
    lib_hdmi_hd21_fec_measure_restart(nport, is_dsc_mode);

    lib_hdmims_active_correction(&(update_timing->h_act_len), &(update_timing->v_act_len));
    #if 0//for debugging
    HDMI_WARN("[HD21MS] measure_en=%d, MEASURE_3: 0x%x, MEASURE_4: 0x%x\n",
        HD21_ps_measure_ctrl_get_en(hdmi_in(HD21_ps_measure_ctrl_reg)), hdmi_in(HD21_MEAS_FRL_MEASURE_3_reg), hdmi_in(HD21_MEAS_FRL_MEASURE_4_reg));

    HDMI_WARN("[HD21MS] IHTotal: %d, ht_clk_cnt=%d\n", update_timing->h_total, HD21_MEAS_FRL_MEASURE_1_get_ht_clk_cnt(hdmi_in(HD21_MEAS_FRL_MEASURE_1_reg)));
    HDMI_WARN("[HD21MS] IVTotal: %d,IVTotal_3rd=%d\n", update_timing->v_total, HD21_MEAS_FRL_MEASURE_7_get_vt_line_cnt2(hdmi_in(HD21_MEAS_FRL_MEASURE_7_reg)));
    HDMI_WARN("[HD21MS] IHAct: %d\n", update_timing->h_act_len);
    HDMI_WARN("[HD21MS] IVAct: %d\n", update_timing->v_act_len);
    HDMI_WARN("[HD21MS] IVFreq: %d\n", update_timing->v_freq);
    HDMI_WARN("[HD21MS] IHFreq: %d\n", update_timing->h_freq);
    HDMI_WARN("[HD21MS] Polarity: %d\n", update_timing->polarity);
    HDMI_WARN("[HD21MS] IHsyncWidth %d\n", update_timing->IHSyncPulseCount);
    HDMI_WARN("[HD21MS] IVsyncWidth %d\n", update_timing->IVSyncPulseCount);
    #endif
    return HDMI21_MEASURE_SUCCESS;
}

//------------------------------------------------------------------
// Clock Detection
//------------------------------------------------------------------
void lib_hdmi_hd21_clkdet_en(unsigned char nport, unsigned char enable)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    hdmi_mask(HD21_CLKDET2CR_reg, ~HD21_CLKDETCR_clock_det_en_mask, HD21_CLKDETCR_clock_det_en((enable) ? 1:0));
}

/*
Input Bit Data lane source selction
00: Lane2, Red lane bit data transition
01: Lane1, Green lane bit data transition
10: Lane0, Blue lane bit data transition
11: Lane3, Clock lane bit data transition
*/
void lib_hdmi_hd21_clkdet_trans_meas_sel(unsigned char nport, unsigned char lane_sel)
{
    unsigned char trans_sel = 0;
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    switch(lane_sel)
    {
    case 0: //lane 0, Blue
        trans_sel = 2;
        break;
    case 1: // lane1, Green
        trans_sel = 1;
        break;
    case 2: // lane2, Red
        trans_sel = 0;
        break;
    case 3: // lane3, Clock
        trans_sel = 3;
        break;
    default:  //invalid value
        HDMI_ERR("[lib_hdmi_hd21_clkdet_trans_meas_sel port:%d] invalid lane_sel=%d\n", nport, lane_sel);
        return;
    }

    hdmi_mask(HD21_CLKDET2CR_reg, ~HD21_CLKDETCR_trans_meas_sel_mask, HD21_CLKDETCR_trans_meas_sel(trans_sel));
}

void lib_hdmi_hd21_clkdet_reset_counter(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    hdmi_mask(HD21_CLKDET2SR_reg, ~HD21_CLKDETSR_reset_counter_mask, HD21_CLKDETSR_reset_counter_mask);
}


void lib_hdmi_hd21_clkdet_set_pop_up(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    hdmi_mask(HD21_CLKDET2SR_reg, ~HD21_CLKDETSR_pop_out_mask, HD21_CLKDETSR_pop_out_mask);
}

unsigned char lib_hdmi_hd21_clkdet_get_clk_counter(unsigned char nport, unsigned int* get_clk)
{// 0: get fail, 1: get success
    unsigned char timeout = 10;

    if (!lib_hdmi_is_hdmi_21_available(nport))
        return 0;

    lib_hdmi_hd21_clkdet_set_pop_up(nport);
    while(timeout >0)
    {
        //HDMI_EMG("nport=%d, timeout=%d, pop=%d\n", nport, timeout, HDMI_CLKDETSR_get_pop_out(hdmi_in(HDMI_CLKDETSR_reg)));
        if(HD21_CLKDETSR_get_pop_out(hdmi_in(HDMI_CLKDETSR_reg)) ==0)
        {
            *get_clk = HD21_CLKDETSR_get_clk_counter(hdmi_in(HD21_CLKDET2SR_reg));
            lib_hdmi_hd21_clkdet_set_pop_up(nport);
            mdelay(5);
            return 1;
        }
        else
        {
            timeout--;
        }
    }
    return 0;
}

void lib_hdmi_hd21_init_clkdet_counter(unsigned char nport, unsigned char lane_sel)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;
    hdmi_mask(HD21_CLK_SETTING_00_reg, HD21_CLK_SETTING_00_dclk_cnt_end_mask, HD21_CLK_SETTING_00_dclk_cnt_end(DEFAULT_CLK_DET_CNT_END));

    lib_hdmi_hd21_clkdet_en(nport, TRUE);
    lib_hdmi_hd21_clkdet_trans_meas_sel(nport, lane_sel);
    lib_hdmi_hd21_clkdet_set_pop_up(nport);
}

void lib_hdmi_hd21_mac_init(unsigned char nport)
{
    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;


    // 7. HD21 PHY FIFO CK ch async fifo enable
    lib_hdmi_hd21_phy_afifo_enable(nport, TRUE);

    // 8.  HD21 PHY FIFO  fifo ckclk inv
    hdmi_mask(HD21_PHY_FIFO_CR1_reg,
        ~(HD21_PHY_FIFO_CR1_port0_ckclk_inv_mask|HD21_PHY_FIFO_CR1_port1_ckclk_inv_mask),
        0x0);    // Port0_ckclk_inv & Port1_ckclk_inv = 0
    hdmi_mask(HD21_PHY_FIFO_CR1_reg,
        ~(HD21_PHY_FIFO_CR1_port2_ckclk_inv_mask|HD21_PHY_FIFO_CR1_port3_ckclk_inv_mask),
        (HD21_PHY_FIFO_CR1_port2_ckclk_inv_mask|HD21_PHY_FIFO_CR1_port3_ckclk_inv_mask));    //Port2_ckclk_inv & Port3_ckclk_inv = 1

    // Enable HD21 Mac and set lane mode
    hdmi_out(HD21_CR_reg, HD21_CR_hd21_en(1) | HD21_CR_lane_mode_fw(1));       // lane mode = 4 & hd21_en = 1

    // Setup HDMI CTS fifo
    lib_hdmi_mac_out_sel(nport, 1);   // set source to hd21

    // channel async fifo enable
    lib_hdmi_hd21_afifo_enable(nport, TRUE);       

    #if 0
    hdmi_mask(HDMI_TMDS_UDC_reg, ~HDMI21_P0_HD21_UDC_debug_sel_mask, HDMI21_P0_HD21_UDC_debug_sel(0x3F));
    HDMI_EMG("HDMI21_P0_HD21_UDC_debug_sel\n");
    hdmi_mask(HDMI_HDMI_CTS_FIFO_CTL_reg, ~HDMI_HDMI_CTS_FIFO_CTL_hdmi_test_sel_mask, HDMI_P0_HDMI_CTS_FIFO_CTL_hdmi_test_sel(0xB));
    #endif
    // FEC Register init - for color space auto mode, md_sel must be set 1.
    hdmi_mask(HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_reg,
        ~(HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_md_sel_ctrl_mask|HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_md_sel_mask),
        (HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_md_sel_ctrl_mask|HD21_FEC_FEC_PORT_PAGE73_HDMI_SCR_md_sel_mask));

    // GCP init setting
    lib_hdmi_gcp_ackg_header_parsing_mode(nport, TRUE); //need update gcp setting after hdmi21 enable

    // lock mode, default is TX mode.
    hdmi_mask(HD21_channel_align_b_reg,~(HD21_channel_align_b_com_seq_lock_mask),0);
    hdmi_mask(HD21_channel_align_g_reg,~(HD21_channel_align_g_com_seq_lock_mask),0);
    hdmi_mask(HD21_channel_align_r_reg,~(HD21_channel_align_r_com_seq_lock_mask),0);
    hdmi_mask(HD21_channel_align_c_reg,~(HD21_channel_align_c_com_seq_lock_mask),0);
}

//------------------------------------------------------------------
// High level API
//------------------------------------------------------------------
void newbase_hdmi_hd21_init_fw_vs_polarity(unsigned char port)
{
    if (!lib_hdmi_is_hdmi_21_available(port))
        return ;

    hdmi_rx[port].hdmi_2p1_fw_pol_check_cnt = 0;
    hdmi_rx[port].hdmi_2p1_vs_pol = 0xF;
    lib_hdmi_hd21_fec_set_fw_vs_polarity(port, 1, 0); // reset invert to zero
}

unsigned char newbase_hdmi_hd21_check_data_align(unsigned char nport, HDMI_FRL_MODE_T frl_mode, HDMI_LANE_COUNT lane_mode)
{
    int lane = 0;
    int timeout_cnt = DATA_ALIGN_CHECK_LOOP_THD;

    if (!lib_hdmi_is_hdmi_21_available(nport) || (frl_mode == MODE_TMDS))
        return FALSE;

    HD21_INFO("------------- newbase_hdmi_hd21_check_data_align ----------\n");
    HD21_INFO("Port = %d, frl_mode =%d, lane_mode = %d\n", nport, frl_mode, lane_mode);

    for (lane = 0; lane < lane_mode ; lane++)
        memset(&lane_st[lane], 0, sizeof(lane_align_state));

    lib_hdmi_hd21_clear_data_align_error_status(nport);

    while (timeout_cnt--)
    {
        if(GET_FRL_LT_FSM(nport) < LT_FSM_LTSP_PASS)
        {
            HDMI_PRINTF("[newbase_hdmi_hd21_check_data_align] current_ms:%ld, Port:%d, Link Training not finished!!frl mode=%d, lt_state=%s\n!", hdmi_get_system_time_ms(), nport, frl_mode, _hdmi_hd21_frl_lt_fsm_str(GET_FRL_LT_FSM(nport)));
            return FALSE;
        }
        if(HD21_FLOW_PRINT_FLAG)
        {//only for debugging
            unsigned int align_c_status = hdmi_in(HD21_channel_align_c_reg);
            unsigned int align_r_status = hdmi_in(HD21_channel_align_r_reg);
            unsigned int align_g_status = hdmi_in(HD21_channel_align_g_reg);
            unsigned int align_b_status = hdmi_in(HD21_channel_align_b_reg);
            unsigned char scdc_10 = lib_hdmi_scdc_read(nport,  0x10);

            HD21_WARN("timeout_cnt =%d, AVMUTE 2.1=%x , scdc_10 =0x%x, (b,g,r,c)align=0x(%x,%x,%x,%x),  lane_st.state(0,1,2,3)=(%d, %d, %d, %d) \n",
                timeout_cnt,
                hdmi_in(HD21_HDMI_SR_reg),
                scdc_10,
                align_b_status,
                align_g_status,
                align_r_status,
                align_c_status,
                lane_st[0].state,
                lane_st[1].state,
                lane_st[2].state,
                lane_st[3].state);
        }

        for (lane = 0; lane < lane_mode ; lane++){
            lib_hdmi_hd21_data_align_status(nport, lane);
        }
        udelay(50);

        if (lane_mode == HDMI_4LANE)
        {
            if ((lane_st[0].state & lane_st[1].state & lane_st[2].state & lane_st[3].state) == comma_seq_lock_state)
            {
                HDMI_INFO("HDMI_4LANE data align finish, timeout_cnt =%d\n", timeout_cnt);
                break;
            }
        }
        else
        {
            if ((lane_st[0].state & lane_st[1].state & lane_st[2].state) == comma_seq_lock_state)
            {
                HDMI_INFO("HDMI_3LANE data align finish, timeout_cnt =%d\n", timeout_cnt);
                break;
            }
        }
    }
    if (timeout_cnt <= 0)
    {
        HDMI_INFO("data align timeout, data_align_state(b,g,r,c)=(%d,%d,%d,%d)\n", lane_st[0].state, lane_st[1].state, lane_st[2].state, lane_st[3].state);
        lib_hdmi_hd21_fw_disparity_error_reset(nport);
        lib_hdmi_hd21_fw_symbol_error_reset(nport);
        //return FALSE; //TBD, TO DO
    }
    else
    {
    HDMI_INFO("data align successed, timeout_cnt =%d, data_align_state(b,g,r,c)=(%d,%d,%d,%d)\n", timeout_cnt, lane_st[0].state, lane_st[1].state, lane_st[2].state, lane_st[3].state);
    }
    return TRUE;
}

void newbase_hdmi_hd21_mac_enable(unsigned char port)
{//enable before link training
    if (!lib_hdmi_is_hdmi_21_available(port))
    {
        HD21_WARN("Enable HDMI[p%d] 2.1 MAC failed, no HDMI 2.1 MAC exists\n", port);
        return;
    }



    hdmi_rx[port].hdmi_2p1_en = true;
    //lib_hdmi_hdcp_set_enc_tog_sel(port, 1);// only for merlin7; hd21  set enc_tog_sel  1

    // HD21 MAC Clock Enable
    lib_hdmi_hd21_mac_crt_perport_reset(port);

    lib_hdmi_hd21_mac_init(port);
    lib_hdmi_hd21_force_realign(port);

    lib_hdmi_hd21_fec_measure_restart(port, TRUE);
    lib_hdmi_hd21_fec_frl_det_enable(port, TRUE);

    newbase_hdmi_hd21_init_fw_vs_polarity(port);
 
    // HDMI2.1 SCDC setting
    lib_hdmi_scdc_data_sel(port, 1);

#ifdef HDMI_DISABLE_TMDS_INPUT_ON_FRL_MODE
    lib_hdmi_mac_hd20_tmds_input_eanbale(port, 0);    // gating HD20 TMDS in for power saving
#endif

    HD21_INFO("Enable HDMI[p%d] 2.1 MAC successed\n", port);
}

void newbase_hdmi_hd21_mac_disable(unsigned char port)
{
    if (!lib_hdmi_is_hdmi_21_available(port))
        return;

    if (hdmi_rx[port].hdmi_2p1_en==0)  // HDMI 2.1 is disabled already
        return ;

    lib_hdmi_hd21_enable(port, FALSE);
    lib_hdmi_mac_out_sel(port, 0);   // set source to hd2.0
    lib_hdmi_scdc_data_sel(port, 0);
    hdmi_rx[port].hdmi_2p1_en = false;
    HD21_INFO("Disable HDMI[p%d] 2.1 MAC successed\n", port);
    //lib_hdmi_hdcp_set_enc_tog_sel(port, 0); // only for merlin7; hd2.0  set enc_tog_sel  0
#ifdef HDMI_DISABLE_TMDS_INPUT_ON_FRL_MODE
    lib_hdmi_mac_hd20_tmds_input_eanbale(port, 1);    // gating HD20 TMDS in for power saving
#endif

    //------------------------------------------------------
    newbase_rxphy_reset_setphy_proc(port);
    //newbase_rxphy_job(port);  // call phy to swith to back to TMDS mode            
}

const char* _hdmi_hd21_frl_mode_str(HDMI_FRL_MODE_T frl_mode)
{
    switch (frl_mode)
    {
    case MODE_TMDS:    return "MODE_TMDS";
    case MODE_FRL_3G_3_LANE:    return "MODE_FRL_3G_3_LANE";
    case MODE_FRL_6G_3_LANE:    return "MODE_FRL_6G_3_LANE";
    case MODE_FRL_6G_4_LANE:    return "MODE_FRL_6G_4_LANE";
    case MODE_FRL_8G_4_LANE:    return "MODE_FRL_8G_4_LANE";
    case MODE_FRL_10G_4_LANE:    return "MODE_FRL_10G_4_LANE";
    case MODE_FRL_12G_4_LANE:    return "MODE_FRL_12G_4_LANE";

    default:
    return "MODE_UNKNOW";
    }
}

const char* _hdmi_hd21_frl_lt_fsm_str(unsigned int fsm)
{
    switch (fsm) {
    case LT_FSM_LTS2_FLT_READY:       return "LTS2_FLT_READY";
    case LT_FSM_LTS3_PREPARE_START: return "FSM_LTS3_PREPARE_START";
    case LT_FSM_LTS3_WAIT_FLT_UPDATE_CLEAR:         return "LTS3_WAIT_FLT_UPDATE_CLEAR";
    case LT_FSM_LTS3_PATTERN_TRAINING:      return "LTS3_PATTERN_TRAINING";
    case LT_FSM_LTSF_FAIL:     return "LTSF_FAIL";
    case LT_FSM_LTSL_FFE_CHANGE:     return "LTSL_FFE_CHANGE";
    case LT_FSM_LTSF_FRL_RATE_CHANGE:     return "FRL_RATE_CHANGE";
    case LT_FSM_LTSP_PASS:     return "LTSP_PASS";
    case LT_FSM_LTSP_CHECK:     return "LTSP_CHECK";

    default:
    return "UNKNOW";
    }
}

void newbase_hdmi_hd21_lt_fsm_status_init(unsigned char port)
{
    lt_fsm_status[port].frl_lt_fsm = LT_FSM_LTS2_FLT_READY;
    lt_fsm_status[port].current_frl_mode = MODE_TMDS;
    lt_fsm_status[port].max_tx_ffe_level = 0;
    lt_fsm_status[port].current_ffe_level[0]= 0;
    lt_fsm_status[port].current_ffe_level[1]= 0;
    lt_fsm_status[port].current_ffe_level[2]= 0;
    lt_fsm_status[port].current_ffe_level[3]= 0;
    lt_fsm_status[port].wait_flt_update_timeout_cnt = 0;
    lt_fsm_status[port].frl_lt_check_lock_timeout = hdmi_get_system_time_ms() +  GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_PT_FW_TIMEOUT_MS_THD );
    lt_fsm_status[port].ltp[0] = 0;
    lt_fsm_status[port].ltp[1] = 0;
    lt_fsm_status[port].ltp[2] = 0;
    lt_fsm_status[port].ltp[3] = 0;
    lt_fsm_status[port].ltp_pass = FALSE;
    lt_fsm_status[port].err_cnt[0] = 0xFFFF;
    lt_fsm_status[port].err_cnt[1] = 0xFFFF;
    lt_fsm_status[port].err_cnt[2] = 0xFFFF;
    lt_fsm_status[port].err_cnt[3] = 0xFFFF;
    lt_fsm_status[port].tx_flt_no_train = FALSE;
    lt_fsm_status[port].rx_flt_no_timeout= FALSE;
    lt_fsm_status[port].lt_error_status = LT_ERROR_NONE;
    lt_fsm_status[port].lt_fail_cnt = 0;
    lt_fsm_status[port].data_align_fail_cnt = 0;
}

unsigned char m_pre_scdc_0x31[4] = {0,0,0,0};
void newbase_hdmi_hd21_link_training_fsm(unsigned char port)
{
    unsigned char current_scdc_0x31_value = 0;
    unsigned char current_scdc_0x31_irq_det = 0;
    if (!lib_hdmi_is_hdmi_21_available(port))
    {
        return;
    }

    if (newbase_hdmi_get_5v_state(port)==0)
    {//do not run fsm when 5v disconnect.
        return;
    }

    if((GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_LINK_TRAINING_BYPASS )) > 0
        && (GET_FRL_LT_FSM(port) < LT_FSM_LTSP_PASS) && (port != newbase_hdmi_get_current_display_port()))
    {//Bypass LT mode only work for current port
        return;
    }

restart:

    current_scdc_0x31_value = lib_hdmi_scdc_read(port, SCDC_CONFIGURATION_1); //for debugging
    current_scdc_0x31_irq_det = lib_hdmi_scdc_is_sink_config_31_updated(port);
    if (current_scdc_0x31_irq_det)
    {

        HDMIRX_PHY_STRUCT_T* p_phy_st = newbase_rxphy_get_status(port);
        unsigned char last_phy_frl_mode = 0;
        unsigned char config = lib_hdmi_scdc_get_sink_config_31(port);
        unsigned char tx_flt_no_retrain = (lib_hdmi_scdc_read(port, SCDC_CONFIGURATION)&SCDC_FLT_NO_RETRAIN);
        unsigned char rx_flt_no_timeout = (lib_hdmi_scdc_read(port, SCDC_SOURCE_TEST_CONFIGURATION)&SCDC_FLT_NO_TIMEOUT);

        newbase_hdmi_hd21_lt_fsm_status_init(port);
        last_phy_frl_mode = p_phy_st->frl_mode;
        lt_fsm_status[port].current_frl_mode =  (config) & 0xF;
        lt_fsm_status[port].max_tx_ffe_level = (config>>4) & 0xF;
        lib_hdmi_scdc_clr_sink_config_31_update_status(port);
        HDMI_SCDC_WARN("[FRL] Sink Config 31 was updated (%02x - FFE_Levels=%x, FRL_Mode=%x), tx_flt_no_retrain=%d, rx_flt_no_timeout=%d\n",
            config, lt_fsm_status[port].max_tx_ffe_level, lt_fsm_status[port].current_frl_mode, tx_flt_no_retrain, rx_flt_no_timeout);
        HDMI_SCDC_WARN("[FRL] Sink Config 31 value change from %2x to %2x, scdc_0x31_irq_det=%d\n", m_pre_scdc_0x31[port], current_scdc_0x31_value, current_scdc_0x31_irq_det);

        m_pre_scdc_0x31[port] = current_scdc_0x31_value;
        current_scdc_0x31_irq_det = 0;

        if (tx_flt_no_retrain)
        {
            newbase_hdmi_scdc_hw_mask_enable(port, 0);
            hdmi_power_saving_enable = FALSE;
            HDMI_PRINTF("hysteresis on, Disable PWR mode\n");
            last_phy_hyst_mode = 1;		
            #if !HDMI_HYSTERESIS_EN
           // lib_hdmi_hysteresis_en(1);
            lib_hdmi21_hysteresis_en(1);
            #endif
        }
        else
        {
            if(last_phy_hyst_mode==1)
            {
               last_phy_hyst_mode = 0;
               #if !HDMI_HYSTERESIS_EN
               HDMI_PRINTF("hysteresis off\n");
               //lib_hdmi_hysteresis_en(0);
               lib_hdmi21_hysteresis_en(0);
               #endif		
            }	
        }
        //-----------------------------------------------------
        // 0. clear frl start and Initialize LTP pattern to 0/0/0/0
        //-----------------------------------------------------
        lib_hdmi_scdc_clr_frl_start(port);
        lib_hdmi_hd21_link_training_disable(port);
        

        lib_hdmi_scdc_ced_error_det_sum_port_sel(port, newbase_hdmi_hd21_get_frl_lane(lt_fsm_status[port].current_frl_mode));//SCDC setting by lane, need move to hdmi_scdc.c
        newbase_rxphy_set_frl_mode(port,  lt_fsm_status[port].current_frl_mode); //set phy frl mode

        if(last_phy_frl_mode != lt_fsm_status[port].current_frl_mode)
        {
            HDMI_SCDC_WARN("FRL Mode was changed, last %d -> now %d \n", last_phy_frl_mode,  lt_fsm_status[port].current_frl_mode);
            //TDDO: TBD
            newbase_hdmi_hdcp_reset_fsm(port);    //reset hdcp
            lib_hdmi_misc_variable_initial(port);    //reset global variable
        }
        SET_FRL_LT_FSM(port, LT_FSM_LTS3_PREPARE_START);
    }
    //HDMI_EMG("GET_FRL_LT_FSM(port) =%d, FRL Rate = %d, BYPASS WAIT_CNT=%d\n",GET_FRL_LT_FSM(port),  lt_fsm_status[port].current_frl_mode, newbase_hdmi_get_pcbinfo_entry(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_FLT_UPDATE_CLR_TIMEOUT_THD_10MS ));

    if((GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_LINK_TRAINING_BYPASS )) > 0
        && (GET_FRL_LT_FSM(port) < LT_FSM_LTSP_PASS))
    {//for debugging use, bypass link training
        lt_fsm_status[port].current_frl_mode = GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_LINK_TRAINING_BYPASS ) ;
        lt_fsm_status[port].max_tx_ffe_level = 0x03;
        HDMI_EMG("Bypass Link Training!! Force FRL Rate = %d\n", lt_fsm_status[port].current_frl_mode);

        // HDCP reset
        newbase_hdmi_hdcp_reset_fsm(port);    //reset hdcp
        // Initail global variable
        lib_hdmi_misc_variable_initial(port);    //reset global variable

        // 2.1mac enable
        newbase_hdmi_hd21_mac_enable(port);

        // lane mode setting
        lib_hdmi_hd21_set_lane_mode_fw(port, lt_fsm_status[port].current_frl_mode);

        // set phy
        newbase_hdmi_frl_set_phy(port, lt_fsm_status[port].current_frl_mode);
        msleep(40);

        //error detect initailize
        lib_hdmi_hd21_fw_disparity_symbol_error_enable(port);

        lib_hdmi_scdc_ced_enable(port, 1); //for hdmi2.1 need open more early

        lib_hdmi_hd21_fec_rs_cnt_keep_enable(port, TRUE); //keep rs cnt
        lib_hdmi_hd21_fec_rs_decode_enable(port, TRUE);
        lib_hdmi_hd21_fec_scdc_rs_cnt_enable(port, TRUE);

        lib_hdmi_hd21_fw_disparity_error_reset(port);
        lib_hdmi_hd21_fw_symbol_error_reset(port);
        lib_hdmi_scdc_char_err_rst(port, 0);

        SET_FRL_LT_FSM(port, LT_FSM_LTSP_PASS);
    }

    switch(GET_FRL_LT_FSM(port))
    {
    case LT_FSM_LTS2_FLT_READY:
        lib_hdmi_scdc_set_flt_ready(port);
        lib_hdmi_scdc_clr_frl_start(port);

        break;

    case LT_FSM_LTS3_PREPARE_START:
    {
        unsigned char rx_flt_no_timeout = (lib_hdmi_scdc_read(port, SCDC_SOURCE_TEST_CONFIGURATION)&SCDC_FLT_NO_TIMEOUT);

        if (lt_fsm_status[port].current_frl_mode == MODE_TMDS)
        {
            FRL_INFO("FRL Rate = Disabled, switch back to TMDS mode\n");

            HD21_INFO("TMDS mode detected, switch back HDMI 2.0 MAC\n");
            SET_FRL_LT_FSM(port, LT_FSM_LTS2_FLT_READY);
            newbase_hdmi_hd21_mac_disable(port);

            break;
        }
        else if(lt_fsm_status[port].current_frl_mode > GET_FLOW_CFG(HDMI_FLOW_CFG_GENERAL, HDMI_FLOW_CFG0_MAX_FRL_MODE))
        {
            SET_FRL_LT_FSM(port, LT_FSM_LTSF_FRL_RATE_CHANGE);
            FRL_WARN("un-supported frl rate - %d, Change different FRL rate\n", lt_fsm_status[port].current_frl_mode);
            lt_fsm_status[port].lt_error_status |= LT_ERROR_FRL_RATE_OVER_SPEC;
            break;
        }


        //-----------------------------------------------------
        // 2. start lpt_en
        //-----------------------------------------------------
        lib_hdmi_hd21_link_training_disable(port);

        newbase_hdmi_hd21_mac_enable(port);

        lib_hdmi_hd21_set_lane_mode_fw(port, lt_fsm_status[port].current_frl_mode);

        //-----------------------------------------------------
        // 3. select LTP Pattern
        //-----------------------------------------------------
        if (lt_fsm_status[port].current_frl_mode < MODE_FRL_6G_4_LANE)
        {
            lt_fsm_status[port].ltp[0] = GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT2, HDMI_FLOW_CFG3_PT_REQ_PATTERN_L0);
            lt_fsm_status[port].ltp[1] = GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT2, HDMI_FLOW_CFG3_PT_REQ_PATTERN_L1);
            lt_fsm_status[port].ltp[2] = GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT2, HDMI_FLOW_CFG3_PT_REQ_PATTERN_L2);
            lt_fsm_status[port].ltp[3] = 0;
        }
        else
        {
            lt_fsm_status[port].ltp[0] = GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT2, HDMI_FLOW_CFG3_PT_REQ_PATTERN_L0);
            lt_fsm_status[port].ltp[1] = GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT2, HDMI_FLOW_CFG3_PT_REQ_PATTERN_L1);
            lt_fsm_status[port].ltp[2] = GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT2, HDMI_FLOW_CFG3_PT_REQ_PATTERN_L2);
            lt_fsm_status[port].ltp[3] = GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT2, HDMI_FLOW_CFG3_PT_REQ_PATTERN_L3);
        }

        //-----------------------------------------------------
        // 3.1 update SCDC::LTP_REQ
        //-----------------------------------------------------
        lib_hdmi_scdc_update_ltp_req(port, lt_fsm_status[port].ltp);
        lib_hdmi_hd21_link_training_start(port, lt_fsm_status[port].ltp);

        //-----------------------------------------------------
        // 3.2 start lpt_en
        //-----------------------------------------------------
        //IC timer
        lib_hdmi_hd21_link_training_enable(port, lt_fsm_status[port].current_frl_mode, GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_PT_IC_TIMEOUT_MS_THD ));
        //FW timer
        lt_fsm_status[port].frl_lt_check_lock_timeout = hdmi_get_system_time_ms() +  GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_PT_FW_TIMEOUT_MS_THD );

        //-----------------------------------------------------
        // 3.3 start flt_update
        //-----------------------------------------------------
        lib_hdmi_scdc_set_flt_update(port); 
        FRL_INFO("[LT_FSM_LTS3_PREPARE_START] Set Ln[x]_LTP to (B,G,R,C) = (%x%x%x%x), RX FLT_no_timeout flag =%d, scdc_0x10=0x%x, scdc_0x40=0x%x,\n",
            lt_fsm_status[port].ltp[0], lt_fsm_status[port].ltp[1], lt_fsm_status[port].ltp[2], lt_fsm_status[port].ltp[3], rx_flt_no_timeout, lib_hdmi_scdc_read(port, SCDC_UPDATE_FLAGS), lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS));

        if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_LINK_TRAINING_NO_BREAK) == FALSE)
        {
            SET_FRL_LT_FSM(port, LT_FSM_LTS3_WAIT_FLT_UPDATE_CLEAR);
            break;    //break swtich case
        }
        else
        {
            SET_FRL_LT_FSM(port, LT_FSM_LTS3_PATTERN_TRAINING);
            goto restart;  // start pattern training without break. to avoid link training timeout
        }
    }

    case LT_FSM_LTS3_WAIT_FLT_UPDATE_CLEAR:
    {
        unsigned int wait_flt_update_timeout_cnt_thd = GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_CLR_FLTUPDATE_TIMEOUT_CNT );
        unsigned char tx_flt_no_retrain = (lib_hdmi_scdc_read(port, SCDC_CONFIGURATION)&SCDC_FLT_NO_RETRAIN);
        HDMI_FRL_LT_FSM_T lt_fsm_next_state = LT_FSM_LTS3_PATTERN_TRAINING;

        //-----------------------------------------------------
        // 4. wait source to send data
        //-----------------------------------------------------

        if((lt_fsm_status[port].ltp[0]==GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT2, HDMI_FLOW_CFG3_PT_REQ_PATTERN_L0)) &&
            (lt_fsm_status[port].ltp[1]==GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT2, HDMI_FLOW_CFG3_PT_REQ_PATTERN_L1)) &&
            (lt_fsm_status[port].ltp[2]==GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT2, HDMI_FLOW_CFG3_PT_REQ_PATTERN_L2)))
        {
            lt_fsm_next_state = LT_FSM_LTS3_PATTERN_TRAINING;
        }
        else if(lt_fsm_status[port].ltp[0] == 0xE)
        {
            lt_fsm_next_state = LT_FSM_LTS2_FLT_READY;
        }
        else if(lt_fsm_status[port].ltp[0] == 0xF)
        {
            lt_fsm_next_state = LT_FSM_LTS2_FLT_READY;
        }
        else if((lt_fsm_status[port].ltp[0] == 0x0) && (lt_fsm_status[port].ltp[1] == 0x0)  && (lt_fsm_status[port].ltp[2] == 0x0))
        {
            lt_fsm_next_state = LT_FSM_LTSP_PASS;
        }

        if(tx_flt_no_retrain)
        {
            wait_flt_update_timeout_cnt_thd = GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_CTS_CLR_FLTUPDATE_TIMEOUT_CNT ); //around 10 sec
        }
        else
        {
            wait_flt_update_timeout_cnt_thd = GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_CLR_FLTUPDATE_TIMEOUT_CNT ); //around 20ms~40ms
        }

        if(lt_fsm_status[port].wait_flt_update_timeout_cnt < wait_flt_update_timeout_cnt_thd)
        {
            lt_fsm_status[port].wait_flt_update_timeout_cnt ++;
            if (lib_hdmi_scdc_get_flt_update(port)==0)
            {
                FRL_INFO("Source cleared flt_update for LTP:%x, next FSM:%s, scdc_0x10=0x%x, scdc_0x40=0x%x\n", lt_fsm_status[port].ltp[0], _hdmi_hd21_frl_lt_fsm_str(lt_fsm_next_state), lib_hdmi_scdc_read(port, SCDC_UPDATE_FLAGS), lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS));
                SET_FRL_LT_FSM(port, lt_fsm_next_state);
                lt_fsm_status[port].wait_flt_update_timeout_cnt = 0;
            }
            else
            {
                SET_FRL_LT_FSM(port, LT_FSM_LTS3_WAIT_FLT_UPDATE_CLEAR);
            }
        }
        else
        {
            FRL_WARN("Wait FLT_UPDATE clear timeout!!Still run pattern training, LTP:%x, timeout_cnt = %d, scdc_0x10=0x%x, scdc_0x40=0x%x,%x,%x\n", 
                lt_fsm_status[port].ltp[0], lt_fsm_status[port].wait_flt_update_timeout_cnt,
                lib_hdmi_scdc_read(port, SCDC_UPDATE_FLAGS), lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS), lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS_1), lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS_2));
            lt_fsm_status[port].wait_flt_update_timeout_cnt = 0;
            SET_FRL_LT_FSM(port, lt_fsm_next_state);
        }
        break;
    }

    case LT_FSM_LTSL_FFE_CHANGE:
    {
        unsigned int i = 0;
        HDMI_LANE_COUNT lane_count = newbase_hdmi_hd21_get_frl_lane(lt_fsm_status[port].current_frl_mode);
        FRL_WARN("Channel Lock failed, try to change FFE by each lane\n");
        for(i=0; i<lane_count; i++)
        {
                if(lt_fsm_status[port].err_cnt[i] !=0)
                {
                    lt_fsm_status[port].ltp[i] = 0xE;
                    lt_fsm_status[port].current_ffe_level[i] ++;
                    if(lt_fsm_status[port].current_ffe_level[i] > lt_fsm_status[port].max_tx_ffe_level)
                    {
                        FRL_WARN("Current FFE Level %d OVER tx max:%d, down FRL Rate\n", lt_fsm_status[port].current_ffe_level[i], lt_fsm_status[port].max_tx_ffe_level);
                        SET_FRL_LT_FSM(port, LT_FSM_LTSF_FRL_RATE_CHANGE);
                        return;
                    }
                }
                else
                    lt_fsm_status[port].ltp[i] = 0;
        }

        lt_fsm_status[port].lt_error_status |= LT_ERROR_UP_FFE;

        FRL_INFO("[LT_FSM_LTSL_FFE_CHANGE] Set %d lane Ln[x]_LTP to %x%x%x%x, scdc_0x10=0x%x, scdc_0x40=%x,%x,%x\n",
            lane_count, lt_fsm_status[port].ltp[0], lt_fsm_status[port].ltp[1], lt_fsm_status[port].ltp[2], lt_fsm_status[port].ltp[3], lib_hdmi_scdc_read(port, SCDC_UPDATE_FLAGS),
            lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS), lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS_1), lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS_2));
        lib_hdmi_scdc_update_ltp_req(port, lt_fsm_status[port].ltp);
        lib_hdmi_scdc_set_flt_update(port);
        hdmi_rx[port].hdmi_2p1_en = true;
        SET_FRL_LT_FSM(port, LT_FSM_LTS3_WAIT_FLT_UPDATE_CLEAR);
    }
        break;

    case LT_FSM_LTSF_FRL_RATE_CHANGE:
        lt_fsm_status[port].ltp[0] = lt_fsm_status[port].ltp[1] = lt_fsm_status[port].ltp[2] = lt_fsm_status[port].ltp[3] = 0xF;

        if(lt_fsm_status[port].current_frl_mode == MODE_TMDS)
        {
            FRL_WARN("Now is TMDS mode!! End of Link Training!!\n");
            hdmi_rx[port].hdmi_2p1_en = true;

            SET_FRL_LT_FSM(port, LT_FSM_LTS2_FLT_READY);
        }
        else
        {
            lt_fsm_status[port].lt_error_status |= LT_ERROR_DOWN_FRL_RATE;

            FRL_WARN("Request TX to Change FRL rate!!, lt_error_status=%d\n", lt_fsm_status[port].lt_error_status);
            if(lib_hdmi_scdc_get_flt_update(port) == 1)
            {
                HDMI_EMG("[FRL][ERROR] Flt update still not clear on LTSF FRL_RATE_CHANGE!!Port =%d\n", port);
            }

            lib_hdmi_scdc_update_ltp_req(port, lt_fsm_status[port].ltp);

            lib_hdmi_scdc_set_flt_update(port);
            FRL_INFO("[LT_FSM_LTSF_FRL_RATE_CHANGE] Set Ln[x]_LTP to %x%x%x%x, scdc_0x10=0x%x, scdc_0x40=%x,%x,%x\n",
                lt_fsm_status[port].ltp[0], lt_fsm_status[port].ltp[1], lt_fsm_status[port].ltp[2], lt_fsm_status[port].ltp[3], lib_hdmi_scdc_read(port, SCDC_UPDATE_FLAGS),
                lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS), lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS_1),  lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS_2));

            hdmi_rx[port].hdmi_2p1_en = true;
            SET_FRL_LT_FSM(port, LT_FSM_LTS3_WAIT_FLT_UPDATE_CLEAR);
        }

        break;

    case LT_FSM_LTSF_FAIL:
        lt_fsm_status[port].lt_error_status |= LT_ERROR_NO_RETRAIN_OVER_MAX;
        hdmi_rx[port].hdmi_2p1_en = false;
        SET_FRL_LT_FSM(port, LT_FSM_LTS2_FLT_READY);
        break;      

    case LT_FSM_LTS3_PATTERN_TRAINING:
    {
        unsigned int lock[4] = {0,0,0,0};
        HDMI_LANE_COUNT lane_count = newbase_hdmi_hd21_get_frl_lane(lt_fsm_status[port].current_frl_mode);
        unsigned int pattern_training_try_count = GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_PT_TRY_CNT_MAX );
        unsigned int pattern_training_error_thd = GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT2, HDMI_FLOW_CFG3_PT_ERROR_THD );
        unsigned char check_point = 0;
        unsigned char tx_flt_no_retrain = (lib_hdmi_scdc_read(port, SCDC_CONFIGURATION)&SCDC_FLT_NO_RETRAIN);

        //-----------------------------------------------------
        // 5. start up ltp pattern training
        //-----------------------------------------------------
        //lib_hdmi_hd21_force_realign(port); //initial align lock status
	
        lib_hdmi_hd21_link_training_start(port, lt_fsm_status[port].ltp);

        if(tx_flt_no_retrain)
        {
            pattern_training_try_count = 20000;
            lt_fsm_status[port].tx_flt_no_train = TRUE;
            HDMI_WARN("[FRL] Port=%d, TX set FLT_no_retrain=%d, new pattern_training_try_count=%d\n", port, tx_flt_no_retrain, pattern_training_try_count);
        }

        FRL_INFO("[LT_FSM_LTS3_PATTERN_TRAINING] Start Pattern Link Training, current:%ld, check_lock_timeout:%ld, scdc_0x10=0x%x, scdc_0x40=%x,%x,%x\n", 
            hdmi_get_system_time_ms(),  lt_fsm_status[port].frl_lt_check_lock_timeout, lib_hdmi_scdc_read(port, SCDC_UPDATE_FLAGS),
            lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS), lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS_1),  lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS_2));

        while(pattern_training_try_count--)
        {
            if(tx_flt_no_retrain)
            {
                if((newbase_hdmi_get_5v_state(port) == 0) ||(newbase_hdmi_get_hpd(port)==0))
                {
                    unsigned char update_tx_flt_no_retrain = (lib_hdmi_scdc_read(port, SCDC_CONFIGURATION)&SCDC_FLT_NO_RETRAIN);

                    HDMI_WARN("[FRL] 5V=%d or HPD=%d, current no_train=%d, break no_retrain loop\n", newbase_hdmi_get_5v_state(port), newbase_hdmi_get_hpd(port), update_tx_flt_no_retrain);
                    break;
                }
            }
            else if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_PT_TIMEOUT_METHOD ) == 0)
            {// FW timer
                    if(time_before(hdmi_get_system_time_ms(), lt_fsm_status[port].frl_lt_check_lock_timeout)==0)
                    {
                        lt_fsm_status[port].lt_error_status |= LT_ERROR_PT_RX_TIMEOUT;
                        HDMI_WARN("[FRL] Link Training FW timer timeout, current=%ld, limited=%ld\n", hdmi_get_system_time_ms(), lt_fsm_status[port].frl_lt_check_lock_timeout);
                        break;
                    }
            }
            else if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_PT_TIMEOUT_METHOD )== 1)
            {// IC timeout flag
                    if(lib_hdmi_hd21_link_training_get_timeout_flag(port, 0)==1)
                    {
                        lt_fsm_status[port].lt_error_status |= LT_ERROR_PT_RX_TIMEOUT;
                        HDMI_WARN("[FRL] Link Training IC timer timeout!!\n");
                        break;
                    }
            }
            else
            {
                ; //no timeout method, until pass or try_count over max
            }


            if(!tx_flt_no_retrain)
            {
	           //  if((newbase_hdmi_get_longcable_flag(port))&&(!tx_flt_no_retrain))
	            if (newbase_hdmi_get_longcable_flag(port))
	            {
	                 HDMI_WARN("[FRL]Port=%d #1 Long Cable Set pattern_training_error_thd = 2000\n",port);
	                 pattern_training_error_thd = 2000;
	            }
	           // else if((lt_fsm_status[port].lt_fail_cnt>5)&&(!tx_flt_no_retrain))
	            else if (lt_fsm_status[port].lt_fail_cnt>5)
	            {
	                  HDMI_WARN("LT become fail count=%d!\n",lt_fsm_status[port].lt_fail_cnt);
	                 HDMI_WARN("[FRL]Port=%d  #2 Bad Cable Set pattern_training_error_thd = 2000\n",port);
	                 pattern_training_error_thd = 2000;
			         newbase_hdmi_set_longcable_flag(port,1);				 
	            }
            }
		
            // restart to set phy
            newbase_hdmi_frl_set_phy(port, lt_fsm_status[port].current_frl_mode);

            lib_hdmi_hd21_link_training_start(port, lt_fsm_status[port].ltp);

            //mdelay(20); //wait TX's pattern
            //udelay(50);
            mdelay(10);
            
	     lib_hdmi_hd21_ltp_phy_adaptive(port,lane_count);

            lib_hdmi_hd21_link_training_status(port, lock, lt_fsm_status[port].err_cnt);

            FRL_INFO("Link Training, tx_no_retrain=%d, current ms:%ld, lock lane(0,1,2,3)=%d/%d/%d/%d, Err lane(0,1,2,3)=%x/%x/%x/%x, Flt_update=%d, SCDC 0x40=%02x,%02x,%02x, check_point=%d, try_count=%d/%d\n",
                tx_flt_no_retrain,
                hdmi_get_system_time_ms(),
                lock[0], lock[1], lock[2], lock[3],
                lt_fsm_status[port].err_cnt[0], lt_fsm_status[port].err_cnt[1], lt_fsm_status[port].err_cnt[2], lt_fsm_status[port].err_cnt[3],
                lib_hdmi_scdc_get_flt_update(port),
                lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS),
                lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS_1),
                lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS_2),
                check_point,
                pattern_training_try_count,
                GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_PT_TRY_CNT_MAX ));

            if(GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_PT_BYPASS) == TRUE)
            {//bypass result of pattern training for debugging
                SET_FRL_LT_FSM(port, LT_FSM_LTS3_PREPARE_START);
                FRL_INFO("pattern training bypass!Do not check lock and error count!\n");
                lt_fsm_status[port].ltp_pass = TRUE;
                break;
            }

            if (lane_count==HDMI_4LANE)
            {   // 4 lane mode
                if ((lt_fsm_status[port].err_cnt[0]<=pattern_training_error_thd) &&
                    (lt_fsm_status[port].err_cnt[1]<=pattern_training_error_thd) &&
                    (lt_fsm_status[port].err_cnt[2]<=pattern_training_error_thd) &&
                    (lt_fsm_status[port].err_cnt[3]<=pattern_training_error_thd))
                {
                    check_point ++;
                    if(check_point >= GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_PT_CHECKPOINT_MAX ))
                    {
                        FRL_INFO("Link training 4 lane pass\n");
                        lt_fsm_status[port].ltp_pass = TRUE;
                        break;
                    }
                }
                else
                {
                    FRL_INFO("[FRL] Pattern Training Error, LT become fail. lock lane(0,1,2,3)=%d/%d/%d/%d, Err lane(0,1,2,3)=%x/%x/%x/%x, scdc_0x10=0x%x, scdc_0x40=%x,%x,%x\n", 
                        lock[0],
                        lock[1],
                        lock[2],
                        lock[3],
                        lt_fsm_status[port].err_cnt[0],
                        lt_fsm_status[port].err_cnt[1],
                        lt_fsm_status[port].err_cnt[2],
                        lt_fsm_status[port].err_cnt[3],
                        lib_hdmi_scdc_read(port, SCDC_UPDATE_FLAGS),
                        lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS),
                        lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS_1),
                        lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS_2));
                   lt_fsm_status[port].lt_fail_cnt++;		
                   HDMI_WARN("[FRL]  Port=%d 4LANE LT become fail count=%d!\n",port,lt_fsm_status[port].lt_fail_cnt);

                }
            }
            else
            {   // 3 lane mode
                if ((lt_fsm_status[port].err_cnt[0]<=pattern_training_error_thd) &&
                    (lt_fsm_status[port].err_cnt[1]<=pattern_training_error_thd) &&
                    (lt_fsm_status[port].err_cnt[2]<=pattern_training_error_thd))
                {
                    check_point ++;
                    if(check_point >= GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_PT_CHECKPOINT_MAX ))
                    {
                        FRL_INFO("Link training 3 lane pass\n");
                        lt_fsm_status[port].ltp_pass = TRUE;
                        break;
                    }
                }
                else
                {
                    FRL_INFO("[FRL] Pattern Training Error, LT become fail. lock lane(0,1,2)=%d/%d/%d, Err lane(0,1,2)=%x/%x/%x, scdc_0x10=0x%x, scdc_0x40=%x,%x,%x\n", 
                        lock[0],
                        lock[1],
                        lock[2],
                        lt_fsm_status[port].err_cnt[0],
                        lt_fsm_status[port].err_cnt[1],
                        lt_fsm_status[port].err_cnt[2],
                        lib_hdmi_scdc_read(port, SCDC_UPDATE_FLAGS),
                        lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS),
                        lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS_1),
                        lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS_2));

                    lt_fsm_status[port].lt_fail_cnt++;		
                   HDMI_WARN("[FRL]  Port=%d 3LANE LT become fail count=%d!\n",port,lt_fsm_status[port].lt_fail_cnt);
					
                }
            }

            if(pattern_training_try_count<=1)
            {
                HDMI_WARN("[newbase_hdmi_hd21_link_training_fsm][FRL] Invalid flow, port =%d!!Use timeout method=%d\n", port, GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_PT_TIMEOUT_METHOD ));
            }
        }

	if (!tx_flt_no_retrain) {
        	lib_hdmi_hd21_link_training_disable(port);
	}
	
        if (lt_fsm_status[port].ltp_pass == TRUE)
        {
            lib_hdmi_hd21_fw_disparity_symbol_error_enable(port);

            lib_hdmi_scdc_ced_enable(port, 1); //for hdmi2.1 need open more early

            lib_hdmi_hd21_fec_rs_decode_enable(port, TRUE);
            lib_hdmi_hd21_fec_scdc_rs_cnt_enable(port, TRUE);
        
            lt_fsm_status[port].ltp[0] = lt_fsm_status[port].ltp[1] = lt_fsm_status[port].ltp[2] = lt_fsm_status[port].ltp[3] = 0;
            lib_hdmi_scdc_update_ltp_req(port, lt_fsm_status[port].ltp);

            lib_hdmi_scdc_set_flt_update(port);
            FRL_INFO("[LT_FSM_LTSP_PASS] Set Ln[x]_LTP to %x%x%x%x, scdc_0x10=0x%x, scdc_0x40=%x,%x,%x\n", 
                lt_fsm_status[port].ltp[0], lt_fsm_status[port].ltp[1], lt_fsm_status[port].ltp[2], lt_fsm_status[port].ltp[3], lib_hdmi_scdc_read(port, SCDC_UPDATE_FLAGS),
                lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS), lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS_1), lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS_2));

            SET_FRL_LT_FSM(port, LT_FSM_LTS3_WAIT_FLT_UPDATE_CLEAR);

            break;
        }
        else
        {
            if(tx_flt_no_retrain)
            {
                HDMI_WARN("[newbase_hdmi_hd21_link_training_fsm][FRL] Pattern Training fail, NO RETRAIN, port =%d!!\n", port);
                SET_FRL_LT_FSM(port, LT_FSM_LTSF_FAIL);
            }
            else
            {
                HDMI_WARN("[newbase_hdmi_hd21_link_training_fsm][FRL] Pattern Training fail, port =%d!!Use timeout method=%d\n", port, GET_FLOW_CFG(HDMI_FLOW_CFG_HDMI21_LT1, HDMI_FLOW_CFG2_PT_TIMEOUT_METHOD ));
                SET_FRL_LT_FSM(port, LT_FSM_LTSL_FFE_CHANGE);
            }
            break;    //break swtich case
        }
    }
        
    case LT_FSM_LTSP_PASS:
    {
        unsigned char wait_timeout = LTPASS_WAIT_FLTSTART_CLR_TIMEOUT_THD;
        FRL_INFO("Channel locked @ FRL Rate=%d, Land=%d, LTS_P_TRAINING_PASS\n", lt_fsm_status[port].current_frl_mode, lt_fsm_status[port].ltp[3]);
        if(lib_hdmi_scdc_get_flt_update(port) == 1)
        {
            HDMI_EMG("[FRL][ERROR] Flt update still not clear on LTSP PASS!!Port =%d, wait_timeout =%d, scdc_0x40=%x\n", port, wait_timeout, lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS));
        }

        lib_hdmi_scdc_set_frl_start(port);

        
        do
        {//wait FRL start clear, follow HDMI2.1a standard
            msleep(10);
            if(lib_hdmi_scdc_get_frl_start(port) == 0)
            {
                HDMI_WARN("[FRL][WARN] frl_start not clear!!Wait device clear!!Port =%d, wait_timeout =%d, scdc_0x10=%x, scdc_0x40=%x\n", port, wait_timeout, lib_hdmi_scdc_read(port, SCDC_UPDATE_FLAGS), lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS));
            }
            else
            {
                break;
            }

            wait_timeout --;
        }while(wait_timeout>0);

        lib_hdmi_hd21_fw_disparity_error_reset(port);
        lib_hdmi_hd21_fw_symbol_error_reset(port);

        FRL_INFO("Link Training passed, set scdc: frl_start=1, port=%d, scdc_0x10=0x%x,  scdc_0x40=%x,%x,%x\n",
            port, lib_hdmi_scdc_read(port, SCDC_UPDATE_FLAGS), lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS), lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS_1), lib_hdmi_scdc_read(port, SCDC_STATUS_FLAGS_2));
        hdmi_rx[port].hdmi_2p1_en = true;
        newbase_hdmi_hd21_init_fw_vs_polarity(port);

        SET_FRL_LT_FSM(port, LT_FSM_LTSP_CHECK);
    }
        break;

    case LT_FSM_LTSP_CHECK:
    {
        unsigned char is_data_align = newbase_hdmi_hd21_is_data_align_lock(port);
        if (is_data_align==FALSE)
        {
            HDMIRX_PHY_STRUCT_T* p_phy_st = newbase_rxphy_get_status(port);
            unsigned char nport = port;

            lt_fsm_status[port].data_align_fail_cnt++;

            HDMI_WARN("[LT_FSM_LTSP_CHECK][FRL] Port:%d, Cheak data align fail!!Fail cnt=%d, Lock(b,g,r,c)=(%08x,%08x,%08x,%08x)\n",
                port, lt_fsm_status[port].data_align_fail_cnt, hdmi_in(HD21_channel_align_b_reg), hdmi_in(HD21_channel_align_g_reg), hdmi_in(HD21_channel_align_r_reg), hdmi_in(HD21_channel_align_c_reg));

            if( lt_fsm_status[port].data_align_fail_cnt%HDMI21_DATA_ALIGN_FAIL_DO_RETRAIN_THD==0)
            {
                newbase_hdmi_frl_set_phy(port, lt_fsm_status[port].current_frl_mode);
                lib_hdmi_hd21_fw_disparity_error_reset(port);
                lib_hdmi_hd21_fw_symbol_error_reset(port);
            }

            if((lt_fsm_status[port].data_align_fail_cnt%HDMI21_DATA_ALIGN_FAIL_RETRY_DEBOUNCE)==0)
            {
                lib_hdmi_hd21_dynamic_realign(port); //initial align lock status
            }

            newbase_hdmi_hd21_check_data_align(port, p_phy_st->frl_mode, newbase_hdmi_hd21_get_frl_lane(p_phy_st->frl_mode));

            newbase_hdmi_hd21_init_fw_vs_polarity(port);

        }
        else
        {
            lt_fsm_status[port].data_align_fail_cnt = 0;
        }
        if ((is_data_align == TRUE ) && (hdmi_rx[port].hdmi_2p1_fw_pol_check_cnt < FW_POLARITY_CHECK_DEBOUNCE_THD))
        {
            unsigned char v_pkt_det = lib_hdmi_hd21_get_fec_v_pkt_det(port);
            unsigned char frl_vs = lib_hdmi_hd21_get_fec_frl_vs(port);
            unsigned int vs_pol = 0xF;

            if((frl_vs == TRUE) && (v_pkt_det==TRUE))
            {
                lib_hdmi_hd21_clr_fec_frl_vs(port);
                lib_hdmi_hd21_clr_v_pkt_det(port);
            }
            else
            {
                break;
            }

            vs_pol = lib_hdmi_hd21_meas_get_vs_polarity(port);// ^ lib_hdmi_hd21_fec_get_vs_polar_invert(port);

            if (hdmi_rx[port].hdmi_2p1_vs_pol != vs_pol && hdmi_rx[port].hdmi_2p1_fw_pol_check_cnt)
            {
                hdmi_rx[port].hdmi_2p1_fw_pol_check_cnt = 0;

                HDMI_INFO("FW VS Polarity unstableStable (Polarity=%d->%d, Cnt=%d, is_invert=%d)\n",
                    hdmi_rx[port].hdmi_2p1_vs_pol, vs_pol,
                    hdmi_rx[port].hdmi_2p1_fw_pol_check_cnt,
                    lib_hdmi_hd21_fec_get_vs_polar_invert(port));
            }
            else
            {

                HDMI_INFO("FW VS Polarity Stable (Polarity=%d, Cnt=%d, is_invert=%d)\n",
                    vs_pol,
                    hdmi_rx[port].hdmi_2p1_fw_pol_check_cnt,
                    lib_hdmi_hd21_fec_get_vs_polar_invert(port));

                hdmi_rx[port].hdmi_2p1_fw_pol_check_cnt++;
            }

            hdmi_rx[port].hdmi_2p1_vs_pol = vs_pol;

            if (hdmi_rx[port].hdmi_2p1_fw_pol_check_cnt == FW_POLARITY_CHECK_DEBOUNCE_THD)
            {
                HDMI_INFO("FW VS Polarity Stable (Polarity=%d)\n", hdmi_rx[port].hdmi_2p1_vs_pol);
                lib_hdmi_hd21_fec_set_fw_vs_polarity(port, 1, hdmi_rx[port].hdmi_2p1_vs_pol);
            }
        }
        break;
    }
    default:
        break;
    }
}

HDMI_LANE_COUNT newbase_hdmi_hd21_get_frl_lane(HDMI_FRL_MODE_T frl_mode)
{
    switch(frl_mode)
    {
    case MODE_TMDS:
        return HDMI_3LANE;

    case MODE_FRL_3G_3_LANE:
        return HDMI_3LANE;

    case MODE_FRL_6G_3_LANE:
        return HDMI_3LANE;

    case MODE_FRL_6G_4_LANE:   // FRL:6G/4Ln
        return HDMI_4LANE;

    case MODE_FRL_8G_4_LANE:   // FRL:8G/4Ln
        return HDMI_4LANE;

    case MODE_FRL_10G_4_LANE:   // FRL:10G/4Ln
        return HDMI_4LANE;

    case MODE_FRL_12G_4_LANE:   // FRL:12G/4Ln
        return HDMI_4LANE;

    default:
        HDMI_EMG("[newbase_hdmi_hd21_get_frl_lane] ERROR un-supported frl_mode - %d, Change different frl_mode\n", frl_mode);
        return HDMI_3LANE;
    }
}

unsigned char newbase_hdmi_hd21_get_frl_rate(HDMI_FRL_MODE_T frl_mode)
{
    switch(frl_mode)
    {
    case MODE_TMDS:
        return 3;

    case MODE_FRL_3G_3_LANE:
        return 3;

    case MODE_FRL_6G_3_LANE:
        return 6;

    case MODE_FRL_6G_4_LANE:
        return 6;

    case MODE_FRL_8G_4_LANE:
        return 8;

    case MODE_FRL_10G_4_LANE:
        return 10;

    case MODE_FRL_12G_4_LANE:
        return 12;

    default:
        HDMI_EMG("[newbase_hdmi_hd21_get_frl_rate] ERROR un-supported frl_mode - %d, Change different frl_mode\n", frl_mode);
        return 3;
    }
}

unsigned char newbase_hdmi_hd21_is_data_align_lock(unsigned char port)
{
    HDMIRX_PHY_STRUCT_T* p_phy_st;
    HDMI_LANE_COUNT lane_max = HDMI_3LANE;
    unsigned int i =0;
    unsigned char check_count = 0;

    p_phy_st = newbase_rxphy_get_status(port);
    if(p_phy_st == NULL)
    {
        HDMI_EMG("[newbase_hdmi_hd21_is_data_align_lock] ERROR  NULL p_phy_st, port=%d\n", port);
        return FALSE;
    }

    if(p_phy_st->frl_mode == MODE_TMDS)
    {
        HDMI_WARN("[newbase_hdmi_hd21_is_data_align_lock] Invalid calling, TMDS mode don't call the function,  port=%d\n", port);
        return FALSE;
    }

    if(GET_FRL_LT_FSM(port)< LT_FSM_LTSP_PASS)
    {
        HD21_INFO("[newbase_hdmi_hd21_is_data_align_lock] Port:%d, Link Training not finished!!frl mode=%d, lt_state=%s\n!", port, p_phy_st->frl_mode, _hdmi_hd21_frl_lt_fsm_str(GET_FRL_LT_FSM(port)));
        return FALSE;
    }
    lane_max = newbase_hdmi_hd21_get_frl_lane(p_phy_st->frl_mode);

    for(i = 0; i<lane_max; i++)
    {
        if(lib_hdmi_hd21_is_data_align_lock(port, i))
        {
            check_count ++;
        }
    }

    if(check_count == lane_max)
    {
        return TRUE;
    }
    else
    {
        #if 0 //for debugging
        HDMI_PRINTF("[newbase_hdmi_hd21_is_data_align_lock Port:%d]  Cheak data align fail!! Lock(lane, Max)=(%d,%d)\n", port, check_count, lane_max);
        #endif
        return FALSE;
    }

}

extern int newbase_hdmi_get_common_port(unsigned char port, unsigned char *p_common_port);

void newbase_hdmi_hd21_clk_gen(unsigned char nport, unsigned char dsc_enable, unsigned char pixel_mode)
{ 
    unsigned int timing_gen_fifo_status = 0;
    unsigned char common_port;

    if (!lib_hdmi_is_hdmi_21_available(nport))
        return;

    HDMI_ERR("[newbase_hdmi_hd21_clk_gen port:%d] pixel mode=%d\n", nport, pixel_mode);
 
    newbase_hdmi_get_common_port(nport, &common_port);

#ifdef FORCE_DSC_TO_2P_MODE  // by default, DSC will use 2 p mode
    if (dsc_enable)
        pixel_mode = 2; // force 2 pixel mode if DSC is enabled (manatory for 8K60 video)
#endif

    switch(pixel_mode)
    {
    case 1:  // for pixel clock less than 1.2G
        hdmi_mask(HD21_CR_reg, ~HD21_CR_clk_div_sel_mask, HD21_CR_clk_div_sel(3));
        hdmi_mask(HD21_CR_reg, ~HD21_CR_clk_div_en_mask, HD21_CR_clk_div_en(1));
        hdmi_mask(HDMI_hdmi_20_21_ctrl_reg, ~HDMI_hdmi_20_21_ctrl_out_pixel_clk_div2_mask, HDMI_hdmi_20_21_ctrl_out_pixel_clk_div2(1));


        if (common_port==0)
            hdmi_mask(SYS_REG_SYS_CLKSEL_reg, ~SYS_REG_SYS_CLKSEL_hdmirx_2p_clk_sel_mask, SYS_REG_SYS_CLKSEL_hdmirx_2p_clk_sel_mask);
        else
            hdmi_mask(SYS_REG_SYS_CLKSEL_reg, ~SYS_REG_SYS_CLKSEL_hdmirx2_2p_clk_sel_mask, SYS_REG_SYS_CLKSEL_hdmirx2_2p_clk_sel_mask);

#if 0//def FORCE_VGIP_PIXEL_MODE control by scaler
    #ifdef CONFIG_FORCE_RUN_I3DDMA
        hdmi_mask(SYS_REG_SYS_DISPCLKSEL_reg, ~SYS_REG_SYS_DISPCLKSEL_i3_srcpll2p_en_mask, 0);  // I3D VGIP 1 p clock
    #else
        hdmi_mask(SYS_REG_SYS_DISPCLKSEL_reg, ~SYS_REG_SYS_DISPCLKSEL_i1_srcpll2p_en_mask, 0);  // I3D VGIP 1 p clock
    #endif
#endif
        break;

    case 2:  // for pixel clock more than 1.2G
        if(dsc_enable)
        {
            hdmi_mask(HD21_CR_reg, ~HD21_CR_clk_div_sel_mask, HD21_CR_clk_div_sel(1));
        }
        else
        {
            lib_hdmi_tmds_cps_pll_div2_en(nport,FALSE);
            hdmi_mask(HD21_CR_reg, ~HD21_CR_clk_div_sel_mask, HD21_CR_clk_div_sel(3));
        }
        hdmi_mask(HD21_CR_reg, ~HD21_CR_clk_div_en_mask, HD21_CR_clk_div_en(1));
        hdmi_mask(HDMI_hdmi_20_21_ctrl_reg, ~HDMI_hdmi_20_21_ctrl_out_pixel_clk_div2_mask, HDMI_hdmi_20_21_ctrl_out_pixel_clk_div2(0));

        if (common_port==0)
            hdmi_mask(SYS_REG_SYS_CLKSEL_reg, ~SYS_REG_SYS_CLKSEL_hdmirx_2p_clk_sel_mask, 0);
        else
            hdmi_mask(SYS_REG_SYS_CLKSEL_reg, ~SYS_REG_SYS_CLKSEL_hdmirx2_2p_clk_sel_mask, 0);

#if 0//def FORCE_VGIP_PIXEL_MODE control by scaler
    #ifdef CONFIG_FORCE_RUN_I3DDMA
        hdmi_mask(SYS_REG_SYS_DISPCLKSEL_reg, ~SYS_REG_SYS_DISPCLKSEL_i3_srcpll2p_en_mask, SYS_REG_SYS_DISPCLKSEL_i3_srcpll2p_en_mask);  // I3D VGIP 1 p clock
    #else
        hdmi_mask(SYS_REG_SYS_DISPCLKSEL_reg, ~SYS_REG_SYS_DISPCLKSEL_i1_srcpll2p_en_mask, SYS_REG_SYS_DISPCLKSEL_i1_srcpll2p_en_mask);  // MAIN VGIP 2p clock        
    #endif
#endif
        break;

    default:
        HDMI_ERR("[newbase_hdmi_hd21_clk_gen port:%d] invalid pixel mode =%d\n", nport, pixel_mode);
        return;
    }

    if (dsc_enable)
        newbase_hdmi_hd21_dsc_enanble(nport, 0);  // stop dsc

    lib_hdmi_hd21_timing_gen_reset_toggle(nport);  //disable and  enable HDMI 2.1 timing gen

    if ( dsc_enable)
        newbase_hdmi_hd21_dsc_enanble(nport, 1);

    timing_gen_fifo_status = hdmi_in(HD21_TIMING_GEN_SR_reg);

    HDMI_WARN("[newbase_hdmi_measure] HDMI2.1 port =%d, dsc_enable=%d, pixel_mode=%d, Check Timing Gen Error =0x%x\n", nport, dsc_enable, pixel_mode, timing_gen_fifo_status);
}

unsigned char newbase_hdmi_hd21_check_rsed_upd_flag(unsigned char port)
{
    unsigned char result =0;
    if (!lib_hdmi_is_hdmi_21_available(port))
        return 0;

    result = lib_hdmi_hd21_get_rsed_upd_flag(port);

    if(result)    //when resd upd=1, write 1 clear
        lib_hdmi_hd21_clr_rsed_upd_flag(port);

    return result;
}

unsigned char newbase_hdmi_hd21_is_timing_change(unsigned char port, unsigned char is_dsc_mode, HDMI_TIMING_T* last_timing)
{
    HDMI_TIMING_T current_measure_tm;
    unsigned char result = FALSE;

    if(last_timing == NULL)
    {
        HDMI_EMG("[newbase_hdmi_hd21_is_timing_change port:%d] NULL Pointer!!\n", port);
        return FALSE;
    }

    memcpy(&current_measure_tm, last_timing, sizeof(HDMI_TIMING_T)); // copy base timing info from HDMIRX, get color space/deep color/pixel repeat

    if(lib_hdmi21_hd21_measure(port, is_dsc_mode, &current_measure_tm) == HDMI21_MEASURE_SUCCESS)
    {
        unsigned int v_freq_tol = 0;

        if(HDMI_ABS(current_measure_tm.h_total, last_timing->h_total) > 5)
        {
            HDMI_WARN("[HD21MS] H Total Change!! Port=%d, DSC:%d, From %d to %d\n", port, is_dsc_mode, last_timing->h_total, current_measure_tm.h_total);
            result |= TRUE;
        }

        if (HDMI_ABS(current_measure_tm.v_total, last_timing->v_total) > 5)
        {
            HDMI_WARN("[HD21MS] V Total Change!! Port=%d, DSC:%d, From %d to %d\n", port, is_dsc_mode, last_timing->v_total, current_measure_tm.v_total);
            result |= TRUE;
        }

        if (current_measure_tm.h_act_len != last_timing->h_act_len)
        {
            HDMI_WARN("[HD21MS] H Active Change!! Port=%d, DSC:%d, From %d to %d\n", port, is_dsc_mode, last_timing->h_act_len, current_measure_tm.h_act_len);
            result |= TRUE;
        }

        if (current_measure_tm.v_act_len != last_timing->v_act_len)
        {
            HDMI_WARN("[HD21MS] V Active Change!! Port=%d, DSC:%d, From %d to %d\n", port, is_dsc_mode, last_timing->v_act_len, current_measure_tm.v_act_len);
            result |= TRUE;
        }

        if(last_timing->v_freq > HD21_MEAS_SUPER_FAST_V_FREQ_THD)
        {
            v_freq_tol = HD21_MEAS_SUPER_FAST_V_FREQ_TOLERENCE;
        }
        else if(last_timing->v_freq > HD21_MEAS_FAST_V_FREQ_THD)
        {
            v_freq_tol = HD21_MEAS_FAST_V_FREQ_THD;
        }
        else
        {
            v_freq_tol = HD21_MEAS_SLOW_V_FREQ_TOLERENCE;
        }

        #if 0 // for sensitive debugging
        HDMI_EMG("[HD21MS] last_timing->v_freq=%d, last_timing->v_freq=%d, v_freq_tol=%d\n", last_timing->v_freq, last_timing->v_freq, v_freq_tol);
        #endif
        if (HDMI_ABS(current_measure_tm.v_freq, last_timing->v_freq) > v_freq_tol)
        {
            HDMI_WARN("[HD21MS] V Freq Change!! Port=%d, DSC:%d, From %d to %d, tolerence:%d\n", port, is_dsc_mode, last_timing->v_freq, current_measure_tm.v_freq, v_freq_tol);
            result |= TRUE;
        }

    }
    else
    {
        result = FALSE;
    }

    return result;
}

