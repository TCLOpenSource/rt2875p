#include "hdmi_common.h"
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/spinlock.h>
#include "hdmi_reg.h"
#include "hdmi_scdc.h"
#include "hdmi_mac_2p1.h"
#include "hdmi_vfe_config.h"
#include <mach/platform.h>

extern HDMI_PORT_INFO_T hdmi_rx[HDMI_PORT_TOTAL_NUM];
spinlock_t hdmi_scdc_lock;

unsigned short hdmi_scdc_debug_print_cnt[HDMI_PORT_TOTAL_NUM] = {0};
unsigned short hdmi_char_bit_debug_print_cnt[HDMI_PORT_TOTAL_NUM] = {0};

static HDMI_MAIN_FSM_T scdc_print_last_fsm[HDMI_PORT_TOTAL_NUM] = {MAIN_FSM_HDMI_WAIT_SYNC};
static unsigned char scdc_print_last_hd21_en[HDMI_PORT_TOTAL_NUM] = {0};

static unsigned char last_scdc_tmds_bit_clock_ratio[HDMI_PORT_TOTAL_NUM] = {0xFF};
static unsigned char last_scdc_scrambling_enable[HDMI_PORT_TOTAL_NUM] = {0xFF};


//for max ced error count handler
#define ERRDET_PRINT_COUNT    3

static HDMI_PERIOD_ERRDET_STATUS_T m_period_errdet_status[HDMI_ERRDET_NUM];

static HDMI_ERRDET_CONFIG_T m_hdmi_errdet_config_table[] =
{
    //err_det_id, is_enable, is_continuous_mode, is_polling_print, bit_length, every_x_ms, over_y_cnt, name
    {HDMI_HD20_BIT_ERR, TRUE,  TRUE, TRUE, 65535, 1000, 40,  "BIT_ERR"},
    {HDMI_HD20_CHAR_ERR, TRUE,  TRUE, TRUE, 32767, 1000, 60,  "CHAR_ERR"},
    {HDMI_HD21_DISPARITY_ERR, TRUE,  TRUE, TRUE, 65535, 1000, 40,  "DISPARITY_ERR"},
    {HDMI_HD21_SYMBOL_ERR, TRUE, TRUE, FALSE, 65535, 1000, 40,  "SYMBOL_ERR"},
    {HDMI_HD21_RS_ERR, FALSE, TRUE, TRUE, 32767, 1000, 40,  "RS_ERR"}
};
/*---------------------------------------------------
 * Func : lib_hdmi_scdc_enable
 *
 * Desc : enable SCDC
 *
 * Para : nport : HDMI port number 
 *        enable : 0 : disabled, others : enabled
 *   
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_enable(unsigned char nport, unsigned char enable)
{
    hdmi_mask(HDMI_STB_SCDC_CR_reg, ~HDMI_STB_SCDC_CR_scdc_en_mask, (enable) ? HDMI_STB_SCDC_CR_scdc_en_mask : 0);
}


/*---------------------------------------------------
 * Func : lib_hdmi_scdc_reset
 *
 * Desc : reset scdc hw. 
 *
 * Para : nport : HDMI port number 
 *        rst : reset scdc
 *   
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_reset(unsigned char nport, unsigned char rst)
{
    hdmi_mask(HDMI_STB_SCDC_CR_reg, ~(HDMI_STB_SCDC_CR_scdc_reset_mask | HDMI_STB_SCDC_CR_hw_mask_en_mask), (rst) ? HDMI_STB_SCDC_CR_scdc_reset_mask : 0);
}


/*---------------------------------------------------
 * Func : lib_hdmi_scdc_reset_toggle
 *
 * Desc : toggle reset bit to reset all SCDC register
 *        this function should be called when 5V signal 
 *        absent or HPD changed to low for 100ms or more 
 *
 * Para : nport : HDMI port number 
 *   
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_reset_toggle(unsigned char nport)
{
    lib_hdmi_scdc_reset(nport, 1);
    lib_hdmi_scdc_reset(nport, 0);
    HDMI_PRINTF("HDMI[p%d] SCDC reset toggle\n", nport);
}


/*---------------------------------------------------
 * Func : lib_hdmi_scdc_ced_enable
 *
 * Desc : enable/disable SCDC CED
 *
 * Para : nport : HDMI port number
 *        enable : 
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_ced_enable(
    unsigned char           nport, 
    unsigned char           enable
    )
{
    if (hdmi_rx[nport].hdmi_2p1_en)
    {
        hdmi_mask(HD21_cercr_reg, ~HDMI21_P0_hd21_cercr_scdc_ced_en_mask, 
                  (enable) ? HDMI21_P0_hd21_cercr_scdc_ced_en_mask : 0);
    }
    else
    {
    	hdmi_mask(HDMI_CERCR_reg, ~HDMI_CERCR_scdc_ced_en_mask, 
                  (enable) ? HDMI_CERCR_scdc_ced_en_mask : 0);
    }
}


/*---------------------------------------------------
 * Func : lib_hdmi_scdc_ced_reset
 *
 * Desc : reset SCDC CED
 *
 * Para : nport : HDMI port number
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_ced_reset(
    unsigned char           nport
    )
{
    if (hdmi_rx[nport].hdmi_2p1_en)
    {
        hdmi_mask(HD21_cercr_reg, ~HD21_cercr_reset_mask, HD21_cercr_reset_mask);
        udelay(10);
        hdmi_mask(HD21_cercr_reg, ~HD21_cercr_reset_mask, 0);
    }
    else
    {
        hdmi_mask(HDMI_CERCR_reg, ~HDMI_CERCR_reset_mask, HDMI_CERCR_reset_mask);
        udelay(10);
        hdmi_mask(HDMI_CERCR_reg, ~HDMI_CERCR_reset_mask, 0);
    }
}


/*---------------------------------------------------
 * Func : lib_hdmi_scdc_ced_continue_mode_en
 *
 * Desc : reset SCDC CED
 *
 * Para : nport : HDMI port number
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_ced_continue_mode_en(unsigned char nport, unsigned char continue_mode)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
    {
        hdmi_mask(HD21_cercr_reg, ~HD21_cercr_mode_mask, (continue_mode) ? HD21_cercr_mode_mask : 0);         
    }
    else
    {
        hdmi_mask(HDMI_CERCR_reg, ~HDMI_CERCR_mode_mask, (continue_mode) ? HDMI_CERCR_mode_mask : 0);
    }
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_ced_reset
 *
 * Desc : reset SCDC CED
 *
 * Para : nport : HDMI port number
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_ced_error_detect_enable(unsigned char nport, unsigned char enable)
{
    if (hdmi_rx[nport].hdmi_2p1_en)
    {
        hdmi_mask(HD21_cercr_reg, ~HD21_cercr_en_mask, (enable) ? HD21_cercr_en_mask : 0);
    }
    else
    {
        hdmi_mask(HDMI_CERCR_reg, ~HDMI_CERCR_en_mask, (enable) ? HDMI_CERCR_en_mask : 0);
    }
}


/*---------------------------------------------------
 * Func : lib_hdmi_scdc_data_sel
 *
 * Desc : Set source of SCDC
 *
 * Para : nport : HDMI port number
 *        hdmi2p1 : 1 : link to hdmi 2.1           
 *                  0 : link to hdmi 2.0     
 *   
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_data_sel(
    unsigned char           nport, 
    unsigned char           hdmi2p1
    )
{
    if (hdmi2p1)
        hdmi2p1 = 1;

    if (HDMI_STB_SCDC_CR_get_scdc_sel(hdmi_in(HDMI_STB_SCDC_CR_reg))==hdmi2p1)
        return;

    switch(nport)
    { // force write to P0 common
    case 0:
        hdmi_mask(HDMI_ot_reg, ~HDMI_ot_p0_scdc_sel_mask,  HDMI_ot_p0_scdc_sel(hdmi2p1));
        break;
    case 1:
        hdmi_mask(HDMI_ot_reg, ~HDMI_ot_p1_scdc_sel_mask,  HDMI_ot_p1_scdc_sel(hdmi2p1));
        break;
    case 2:
        hdmi_mask(HDMI_ot_reg, ~HDMI_ot_p2_scdc_sel_mask,  HDMI_ot_p2_scdc_sel(hdmi2p1));
        break;
    case 3:
        hdmi_mask(HDMI_ot_reg, ~HDMI_ot_p3_scdc_sel_mask,  HDMI_ot_p3_scdc_sel(hdmi2p1));
        break;
    default:
        HDMI_EMG("[lib_hdmi_scdc_data_sel] Invalid Port:%d!!!", nport);
        break;
    }

    hdmi_mask(HDMI_STB_SCDC_CR_reg, ~HDMI_STB_SCDC_CR_scdc_sel_mask, 
                    HDMI_STB_SCDC_CR_scdc_sel(hdmi2p1));
    
    HDMI_SCDC_INFO("[lib_hdmi_scdc_data_sel] port=%d, scdc src = %s\n",nport, (hdmi2p1) ? "hdmi 2.1" : "hdmi 2.0");
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_ced_error_det_sum_port_sel
 *
 * Desc : Set  scdc_err_det_sum of SCDC by lane count
 *
 * Para : nport : HDMI port number
 *        frl_lane : HDMI_4LANE: 4 Lanes, invert sum of 0x50~0x58
 *                  HDMI_3LANE : 3 Lanes, invert sum of 0x50~0x56   
 *   
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_ced_error_det_sum_port_sel(
    unsigned char           nport, 
    unsigned char           frl_lane
    )
{
    if (frl_lane == HDMI_4LANE)
        hdmi_mask(HDMI_STB_SCDC_CR_reg, ~HDMI_STB_SCDC_CR_scdc_err_det_sum_port_mask, HDMI_STB_SCDC_CR_scdc_err_det_sum_port(1));
    else
        hdmi_mask(HDMI_STB_SCDC_CR_reg, ~HDMI_STB_SCDC_CR_scdc_err_det_sum_port_mask, HDMI_STB_SCDC_CR_scdc_err_det_sum_port(0));
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_is_tmds_config_updated
 *
 * Desc : check the tmds config has been updated
 *
 * Para : nport : HDMI port number 
 *   
 * Retn : 0 : no updated, others : updated
 *--------------------------------------------------*/ 
unsigned char lib_hdmi_scdc_is_tmds_config_updated(
    unsigned char           nport
    )
{
    return HDMI_STB_SCDC_CR_get_write20_det_irq_flag(hdmi_in(HDMI_STB_SCDC_CR_reg));    
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_clr_tmds_config_update_status
 *
 * Desc : clear tmds config updated status
 *
 * Para : nport : HDMI port number 
 *   
 * Retn : N/A
 *--------------------------------------------------*/ 
void lib_hdmi_scdc_clr_tmds_config_update_status(
    unsigned char           nport
    )
{
    hdmi_mask(HDMI_STB_SCDC_CR_reg, ~HDMI_STB_SCDC_CR_write_1_clr_mask, 
                                     HDMI_STB_SCDC_CR_write20_det_irq_flag_mask);    
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_is_sink_config_31_updated
 *
 * Desc : check the sink config (byte31) has been updated
 *
 * Para : nport : HDMI port number 
 *   
 * Retn : 0 : no updated, others : updated
 *--------------------------------------------------*/ 
unsigned char lib_hdmi_scdc_is_sink_config_31_updated(
    unsigned char           nport
    )
{
    return HDMI_STB_SCDC_CR_get_write31_det_irq_flag(hdmi_in(HDMI_STB_SCDC_CR_reg));    
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_update10_hwauto_en
 *
 * Desc : Config scdc_update10_hwauto_en
 *
 * Para : nport : HDMI port number,
 *           config_value: 8bit config value
 *   
 * Retn : N/A
 *--------------------------------------------------*/ 
void lib_hdmi_scdc_set_update10_hwauto_en(
    unsigned char           nport,
    unsigned int          update_flag_mask,
    unsigned char          enable
    )
{
    hdmi_mask(HDMI_STB_SCDC_CR_reg, ~(update_flag_mask<<HDMI_STB_P0_SCDC_CR_update10_hwauto_en_shift), (enable ? (update_flag_mask<<HDMI_STB_P0_SCDC_CR_update10_hwauto_en_shift):0));
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_clr_sink_config_31_update_status
 *
 * Desc : clear sink config (byte31) updated status
 *
 * Para : nport : HDMI port number 
 *   
 * Retn : N/A
 *--------------------------------------------------*/ 
void lib_hdmi_scdc_clr_sink_config_31_update_status(
    unsigned char           nport
    )
{
    hdmi_mask(HDMI_STB_SCDC_CR_reg, ~HDMI_STB_SCDC_CR_write_1_clr_mask, 
                                HDMI_STB_SCDC_CR_write31_det_irq_flag_mask);    
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_read
 *
 * Desc : read SCDC register
 *
 * Para : nport : HDMI port number
 *        addr :  scdc register address
 *
 * Retn : SCDC register value
 *--------------------------------------------------*/
unsigned char lib_hdmi_scdc_read(
    unsigned char           nport,
    unsigned char           addr
    )
{
    unsigned char val;
    unsigned long flags;
    spin_lock_irqsave(&hdmi_scdc_lock, flags);  // add spinlock to prevent racing

    hdmi_out(HDMI_STB_SCDC_AP_reg, addr);
    val = HDMI_STB_SCDC_DP_get_dp1(hdmi_in(HDMI_STB_SCDC_DP_reg));

    spin_unlock_irqrestore(&hdmi_scdc_lock, flags);
    return val;
}


/*---------------------------------------------------
 * Func : lib_hdmi_scdc_write
 *
 * Desc : write SCDC register
 *
 * Para : nport : HDMI port number
 *        addr  : scdc register address 
 *        value : value to be written
 *   
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_write(
    unsigned char           nport, 
    unsigned char           addr,
    unsigned char           value
    )
{
    unsigned long flags;

    spin_lock_irqsave(&hdmi_scdc_lock, flags);  // add spinlock to prevent racing

    hdmi_out(HDMI_STB_SCDC_AP_reg, addr);
    hdmi_out(HDMI_STB_SCDC_DP_reg, value);

    spin_unlock_irqrestore(&hdmi_scdc_lock, flags);
}


/*---------------------------------------------------
 * Func : lib_hdmi_scdc_write_mask
 *
 * Desc : write SCDC register
 *
 * Para : nport : HDMI port number
 *        addr  : scdc register address 
 *        mask  : the masked bit won't be updated
 *        value : value to be written
 *   
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_write_mask(
    unsigned char           nport, 
    unsigned char           addr,
    unsigned char           mask,
    unsigned char           value
    )
{
    unsigned long flags;
    spin_lock_irqsave(&hdmi_scdc_lock, flags);  // add spinlock to prevent racing
    hdmi_out(HDMI_STB_SCDC_AP_reg, addr);
    hdmi_out(HDMI_STB_SCDC_DP_reg, HDMI_STB_SCDC_DP_dp1_rwmask(mask) |
                                   HDMI_STB_SCDC_DP_dp1(value));
    spin_unlock_irqrestore(&hdmi_scdc_lock, flags);
}


/*---------------------------------------------------
 * Func : lib_hdmi_scdc_set_sink_version
 *
 * Desc : set SCDC sink version
 *
 * Para : nport : HDMI port number
 *        ver   : sink version
 *   
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_set_sink_version(
    unsigned char           nport, 
    unsigned char           ver
    )
{
    lib_hdmi_scdc_write(nport, SCDC_SINK_VERSION, ver);
}


/*---------------------------------------------------
 * Func : lib_hdmi_scdc_get_source_version
 *
 * Desc : get SCDC source version
 *
 * Para : nport : HDMI port number 
 *   
 * Retn : source version
 *--------------------------------------------------*/
unsigned char lib_hdmi_scdc_get_source_version(
    unsigned char           nport
    ) 
{
    return lib_hdmi_scdc_read(nport, SCDC_SOURCE_VERSION);
}


/*---------------------------------------------------
 * Func : lib_hdmi_scdc_set_update_flags
 *
 * Desc : set update flags
 *
 * Para : nport : HDMI port number 
 *        val   : flags
 *   
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_set_update_flags(
    unsigned char           nport, 
    unsigned char           flags
    )
{
    lib_hdmi_scdc_write(nport, SCDC_UPDATE_FLAGS, flags);
}


/*---------------------------------------------------
 * Func : lib_hdmi_scdc_get_update_flags
 *
 * Desc : get update flags
 *
 * Para : nport : HDMI port number 
 *        val   : flags
 *   
 * Retn : status flag
 *--------------------------------------------------*/
unsigned char lib_hdmi_scdc_get_update_flags(
    unsigned char           nport
    )
{
    return lib_hdmi_scdc_read(nport,  SCDC_UPDATE_FLAGS);
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_get_tmds_config
 *
 * Desc : get TMDS config
 *
 * Para : nport : HDMI port number  
 *   
 * Retn : tmds config
 *--------------------------------------------------*/
unsigned char lib_hdmi_scdc_get_tmds_config(
    unsigned char           nport
    ) 
{
    return lib_hdmi_scdc_read(nport,  SCDC_TMDS_CONFIGURATION);
}

unsigned char hdmi_force_scramble_status_off = 0; // for tmds off sync test, 
                                                  // It will force the error handler to run in a false scrambled state
                                                  // to ensure that the error handler can operate correctly

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_get_tmds_scrambler_status
 *
 * Desc : get TMDS scramble status
 *
 * Para : nport : HDMI port number  
 *   
 * Retn : tmds scramble config
 *--------------------------------------------------*/
unsigned char lib_hdmi_scdc_get_tmds_scrambler_status(
    unsigned char           nport
    )
{
    if (hdmi_force_scramble_status_off)   // for TMDS off sync test only
        return 0;

    return lib_hdmi_scdc_read(nport,  SCDC_TMDS_SCRAMBLER_STATUS);
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_get_sink_config_31
 *
 * Desc : get sink config (byte 31)
 *
 * Para : nport : HDMI port number  
 *   
 * Retn : tmds scramble config
 *--------------------------------------------------*/
unsigned char lib_hdmi_scdc_get_sink_config_31(
    unsigned char           nport  
    )
{
    return lib_hdmi_scdc_read(nport,  SCDC_CONFIGURATION_1);
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_get_test_read_request
 *
 * Desc : get read request test status
 *
 * Para : nport : HDMI port number  
 *   
 * Retn : test read request status
 *--------------------------------------------------*/    
unsigned char lib_hdmi_scdc_get_test_read_request(
    unsigned char           nport
    )
{
    return lib_hdmi_scdc_read(nport,  SCDC_TEST_READ_REQUEST);
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_set_flt_ready
 *
 * Desc : set flt_ready status
 *
 * Para : nport : HDMI port number  
 *   
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_set_flt_ready(unsigned char nport)
{
    if ((lib_hdmi_scdc_read(nport,  SCDC_STATUS_FLAGS) & SCDC_STATUS_FLT_READY))
        return;

    lib_hdmi_scdc_write_mask(nport,
        SCDC_STATUS_FLAGS, ~SCDC_STATUS_FLT_READY,
        SCDC_STATUS_FLT_READY);
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_set_frl_start
 *
 * Desc : set frl start status
 *
 * Para : nport : HDMI port number  
 *   
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_set_frl_start(unsigned char nport)
{
    lib_hdmi_scdc_write_mask(nport,
        SCDC_UPDATE_FLAGS, ~SCDC_UPDATE_FLAGS_FRL_START,
        SCDC_UPDATE_FLAGS_FRL_START);
    HDMI_INFO("lib_hdmi_scdc_set_frl_start, port=%d\n", nport);
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_set_frl_start
 *
 * Desc : set frl start status
 *
 * Para : nport : HDMI port number  
 *   
 * Retn : N/A
 *--------------------------------------------------*/
unsigned char lib_hdmi_scdc_get_frl_start(unsigned char nport)
{
    return (lib_hdmi_scdc_read(nport, SCDC_UPDATE_FLAGS) & SCDC_UPDATE_FLAGS_FRL_START) ? 1 : 0;
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_clr_frl_start
 *
 * Desc : clear frl start status
 *
 * Para : nport : HDMI port number  
 *   
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_clr_frl_start(unsigned char nport)
{
    lib_hdmi_scdc_write_mask(nport,
        SCDC_UPDATE_FLAGS, ~SCDC_UPDATE_FLAGS_FRL_START,
        0);
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_set_src_clr_frl_start
 *
 * Desc : set frl start status
 *
 * Para : nport : HDMI port number
 *        enable : 0 : do not allow src clear frl_start
 *                 1 : allow src clear frl_start by SCDC
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_set_src_clr_frl_start(unsigned char nport, unsigned char enable)
{
    hdmi_mask(HDMI_STB_SCDC_CR_reg, ~HDMI_STB_SCDC_CR_frl_start_src_clr_en_mask,
            (enable) ? HDMI_STB_SCDC_CR_frl_start_src_clr_en_mask : 0);
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_get_frl_rate
 *
 * Desc : get frl rate
 *
 * Para : nport : HDMI port number
 *
 * Retn : frl rate
 *--------------------------------------------------*/
unsigned char lib_hdmi_scdc_get_frl_rate(unsigned char nport)
{
    return lib_hdmi_scdc_read(nport, SCDC_CONFIGURATION_1) & 0xF;
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_set_flt_update
 *
 * Desc : set flt_update status
 *
 * Para : nport : HDMI port number
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_set_flt_update(unsigned char nport)
{
    HDMI_WARN("lib_hdmi_scdc_set_flt_update!!, port=%d", nport);
    lib_hdmi_scdc_write_mask(nport,
        SCDC_UPDATE_FLAGS, ~SCDC_UPDATE_FLAGS_FLT_UPDATE,
        SCDC_UPDATE_FLAGS_FLT_UPDATE);
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_set_flt_update
 *
 * Desc : get flt_update status
 *
 * Para : nport : HDMI port number
 *
 * Retn : flt update status
 *--------------------------------------------------*/
unsigned char lib_hdmi_scdc_get_flt_update(unsigned char nport)
{
    return (lib_hdmi_scdc_read(nport, SCDC_UPDATE_FLAGS) & SCDC_UPDATE_FLAGS_FLT_UPDATE) ? 1 : 0;
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_update_ltp_req
 *
 * Desc : update ltp requeset
 *
 * Para : nport : HDMI port number  
 *   
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_update_ltp_req(
    unsigned char           nport,
    unsigned char           ltp[4]
    )
{
    lib_hdmi_scdc_write(nport, 
        SCDC_STATUS_FLAGS_1,
        SCDC_SHIFT_STATUS_LN1_LTP_REQ(ltp[1]) +
        SCDC_SHIFT_STATUS_LN0_LTP_REQ(ltp[0]));

    lib_hdmi_scdc_write(nport,
        SCDC_STATUS_FLAGS_2,
        SCDC_SHIFT_STATUS_LN3_LTP_REQ(ltp[3]) +
        SCDC_SHIFT_STATUS_LN2_LTP_REQ(ltp[2]));
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_get_ced
 *
 * Desc : get CED info
 *
 * Para : nport : HDMI port number
 *        ced   : character error detection
 *   
 * Retn : test read request status
 *--------------------------------------------------*/  
void lib_hdmi_scdc_get_ced(
    unsigned char           nport, 
    unsigned char           ced[SCDC_CED_LEN]
    )
{
    int i;    
    for (i=0; i<SCDC_CED_LEN; i++)
        ced[i] = lib_hdmi_scdc_read(nport, SCDC_CED_OFST + i);
}


/*---------------------------------------------------
 * Func : lib_hdmi_scdc_set_manufacturer_oui
 *
 * Desc : set manufacturer oui
 *
 * Para : nport : HDMI port number
 *        oui   : manufacturer oui  
 *   
 * Retn : test read request status
 *--------------------------------------------------*/  
void lib_hdmi_scdc_set_manufacturer_oui(
    unsigned char           nport, 
    unsigned char           oui[SCDC_MANUFACTURER_OUI_LEN]
    )
{
    int i;
    for (i=0; i<SCDC_MANUFACTURER_OUI_LEN; i++)            
        lib_hdmi_scdc_write(nport, SCDC_MANUFACTURER_OUI_OFST + i, oui[i]);    
}


/*---------------------------------------------------
 * Func : lib_hdmi_scdc_get_manufacturer_oui
 *
 * Desc : get manufacturer oui
 *
 * Para : nport : HDMI port number
 *        oui   : manufacturer oui  
 *   
 * Retn : test read request status
 *--------------------------------------------------*/  
void lib_hdmi_scdc_get_manufacturer_oui(
    unsigned char           nport, 
    unsigned char           oui[SCDC_MANUFACTURER_OUI_LEN]
    )
{
    int i;
    for (i=0; i<SCDC_MANUFACTURER_OUI_LEN; i++)    
        oui[i] = lib_hdmi_scdc_read(nport,  SCDC_MANUFACTURER_OUI_OFST + i);    
}


/*---------------------------------------------------
 * Func : lib_hdmi_scdc_set_device_id
 *
 * Desc : set device id
 *
 * Para : nport : HDMI port number
 *        id    : device id
 *   
 * Retn : N/A
 *--------------------------------------------------*/ 
void lib_hdmi_scdc_set_device_id(
    unsigned char           nport,
    unsigned char           id[SCDC_DEVICE_ID_LEN]
    )
{
    int i;
    for (i=0; i<SCDC_DEVICE_ID_LEN; i++)
        lib_hdmi_scdc_write(nport, SCDC_DEVICE_ID_OFST + i, id[i]);
}


/*---------------------------------------------------
 * Func : lib_hdmi_scdc_get_device_id
 *
 * Desc : get device id
 *
 * Para : nport : HDMI port number
 *        id    : device id
 *   
 * Retn : N/A
 *--------------------------------------------------*/ 
void lib_hdmi_scdc_get_device_id(
    unsigned char           nport, 
    unsigned char           id[SCDC_DEVICE_ID_LEN]
    )
{
    int i;    
    for (i=0; i<SCDC_DEVICE_ID_LEN; i++)
        id[i] = lib_hdmi_scdc_read(nport, SCDC_DEVICE_ID_OFST + i);
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_set_maximum_char_err
 *
 * Desc : Maximum number of error character over each 10 ms period.
          This value depends on Mega-characters/second/channel (Mcsc) of current data stream
 *
 * Para : nport : HDMI port number
 *        max_cnt : Maximum number of error character
 *
 * Retn : N/A
 *--------------------------------------------------*/
void lib_hdmi_scdc_set_maximum_char_err(
    unsigned char           nport,
    unsigned int            max_cnt
    )
{
	if(max_cnt > (0x1<<10)-1)
		max_cnt = 600; // HDMI2.0 spec define max char error is 600
	hdmi_mask(HDMI_CERCR_reg, ~(HDMI_CERCR_max_err_num_mask), HDMI_CERCR_max_err_num(max_cnt));
}

unsigned int commcrc_last_vs_cnt[4] = {0,0,0,0};
void newbase_hdmi_scdc_flow_print(unsigned char port)
{
    HDMIRX_PHY_STRUCT_T* p_phy_st;
    static unsigned char check_commcrc_times = 0;
    static unsigned int pre_commcrc = 0;
    unsigned int cur_commcrc = pre_commcrc;
    static unsigned int commcrc_not_cmp =0;

    if(port != newbase_hdmi_get_current_display_port())
    {
        lib_hdmi_crc_continue_mode_en(0);// not current port disable crc continue mode
        return;
    }

    lib_hdmi_crc_continue_mode_en(g_is_hdmi_crc_continue_mode);
    p_phy_st = newbase_rxphy_get_status(port);

    if(scdc_print_last_fsm[port] != newbase_hdmi_get_video_state(port))
    {
        scdc_print_last_fsm[port] = newbase_hdmi_get_video_state(port);
        hdmi_scdc_debug_print_cnt[port] = g_scdc_print_cycle_cnt_thd;
        hdmi_char_bit_debug_print_cnt[port] = g_char_bit_print_cycle_cnt_thd;
    }

    if(scdc_print_last_hd21_en[port] != hdmi_rx[port].hdmi_2p1_en)
    {
        scdc_print_last_hd21_en[port] = hdmi_rx[port].hdmi_2p1_en;
        hdmi_scdc_debug_print_cnt[port] = g_scdc_print_cycle_cnt_thd;
        hdmi_char_bit_debug_print_cnt[port] = g_char_bit_print_cycle_cnt_thd;
    }

    if((hdmi_rx[port].video_t.vs_valid != 0) && (hdmi_rx[port].video_t.vs_cnt != commcrc_last_vs_cnt[port]))
    {
        commcrc_last_vs_cnt[port] = hdmi_rx[port].video_t.vs_cnt;
        if(lib_hdmi_read_video_common_crc(0, &cur_commcrc,lib_hdmi_get_crc_continue_mode_status()) == 0){
            if(check_commcrc_times == 0){
                pre_commcrc = cur_commcrc;
            }

            if(check_commcrc_times <  g_check_commcrc_num)
            {
                if(pre_commcrc != cur_commcrc){
                    commcrc_not_cmp++;
                    check_commcrc_times = 0;
                }
                else
                {
                    check_commcrc_times++;
                }

                if(commcrc_not_cmp >= g_picture_non_static_thd)
                {
                    //CEDERR_FLOW_PRINTF("%s, %d, disable auto crc cmp\n", __FUNCTION__, __LINE__);
                    lib_hdmi_crc_auto_cmp_en(0);
                    lib_hdmi_set_des_crc(0);
                    commcrc_not_cmp = 0;
                }
            }
            else
            {  // picture_is_static
                check_commcrc_times = 0;
                //CEDERR_FLOW_PRINTF("%s, %d, enable auto crc cmp\n", __FUNCTION__, __LINE__);
                lib_hdmi_crc_auto_cmp_en(1);
                lib_hdmi_set_des_crc(pre_commcrc);
            }
        }

    }
    else
    {
        if(hdmi_rx[port].video_t.vs_valid == 0)
        {
            pre_commcrc = 0;
            lib_hdmi_crc_auto_cmp_en(0);
            lib_hdmi_set_des_crc(0);
            check_commcrc_times = 0;
        }
    }

    //---------------------------------------------
    // Signal STS
    //---------------------------------------------
    if (hdmi_char_bit_debug_print_cnt[port] >= g_char_bit_print_cycle_cnt_thd)
    {
        unsigned char video_state = newbase_hdmi_get_video_state(port);
        unsigned int time_ms=0;
        static unsigned char commcrc_fail_acc=0;
        unsigned int current_clk[4] ={0, 0, 0, 0};
        unsigned char is_5V[4] = {0, 0, 0, 0};

        if (p_phy_st==NULL)
        {
            HDMI_EMG("%s failed, port %d doesn't exist and can't get its signal status \n", __FUNCTION__ , port);
            hdmi_char_bit_debug_print_cnt[port] = 0;
            return ;
        }

        newbase_hdmi_clkdet_get_all_clk_counter(port, current_clk);

        is_5V[0] = newbase_hdmi_get_5v_state(0);
        is_5V[1] = newbase_hdmi_get_5v_state(1);
        is_5V[2] = newbase_hdmi_get_5v_state(2);

        if(HDMI_PORT_TOTAL_NUM == 4)
        {
            is_5V[3] = newbase_hdmi_get_5v_state(3);
        }


        if(p_phy_st->frl_mode != MODE_TMDS)
        {
            if(GET_FRL_LT_FSM(port) >= LT_FSM_LTSP_PASS)
            {
                unsigned char nport = port;
                unsigned int align_c_status = hdmi_in(HD21_channel_align_c_reg);
                unsigned int align_r_status = hdmi_in(HD21_channel_align_r_reg);
                unsigned int align_g_status = hdmi_in(HD21_channel_align_g_reg);
                unsigned int align_b_status = hdmi_in(HD21_channel_align_b_reg);
                unsigned int timing_gen_fifo_status = lib_hdmi_hd21_get_timing_gen_status(nport);
                unsigned int disparity_err[4] = {0, 0, 0, 0};

                lib_hdmi_hd21_fw_disparity_error_status(nport, disparity_err);
                if(g_hdmi_check_commcrc)
                {
                    time_ms = hdmi_get_system_time_ms();
                    if(video_state==MAIN_FSM_HDMI_VIDEO_READY && cur_commcrc!=g_hdmi_check_commcrc && time_ms>=g_hdmi_check_commcrc_over_ms)
                    {
                        ++commcrc_fail_acc;
                        if(commcrc_fail_acc>=g_hdmi_check_commcrc_max_fail_times)
                        {
                            HDMI_EMG("check commcrc fail %d times: target COMMCRC=0x%x, current COMMCRC=0x%x, system time=%dms \n",commcrc_fail_acc,g_hdmi_check_commcrc,cur_commcrc,time_ms);
                            commcrc_fail_acc=0;
                        }
                    }
                }
     
                CEDERR_FLOW_PRINTF("Signal2.1 STS Port[%d]: (b,g,r,c)align=0x(%2x,%2x,%2x,%2x); DISPARITY=(%d,%d,%d,%d); mac_clk=(%d,%d,%d,%d,%d); TG_ERR=0x%x, BCH_ERR cnt(1,2)=(%d,%d); BGRHV =0x%x;5V(%d,%d,%d,%d)\n",
                    nport,
                    (align_b_status&0xFF),
                    (align_g_status&0xFF),
                    (align_r_status&0xFF),
                    (align_c_status&0xFF),
                    disparity_err[0],
                    disparity_err[1],
                    disparity_err[2],
                    disparity_err[3],
                    current_clk[0],
                    current_clk[1],
                    current_clk[2],
                    current_clk[3],
                    newbase_hdmi_clkdet_calculate_tmds_character_rate(current_clk[3]),
                    timing_gen_fifo_status,
                    hdmi_rx[port].bch_1bit_err_cnt,
                    hdmi_rx[port].bch_err_cnt,
                    (lib_hdmi_read_rgb_hv_status(nport) & 0x000000F8) >> 3,
                    is_5V[0],
                    is_5V[1],
                    is_5V[2],
                    is_5V[3]);

                CEDERR_FLOW_PRINTF("FSM = %s(%d), des_crc=0x%x; COMMCRC=0x%x; PP_CRC=0x%08x, crc_err_cnt=%d, crc_auto_cmp=%d\n",
                    _hdmi_video_fsm_str(video_state),
                    video_state,
                    lib_hdmi_get_des_crc(),
                    cur_commcrc,
                    lib_hdmi_read_crc(port),
                    lib_hdmi_get_crc_err_cnt(),
                    lib_hdmi_get_crc_auto_cmp_status());
            }
        }
        else
        {
            unsigned int char_r_err = hdmi_rx[port].fw_char_error[2] , char_g_err = hdmi_rx[port].fw_char_error[1] , char_b_err = hdmi_rx[port].fw_char_error[0] ;

            lib_hdmi_crc_check(port); //for per port crc
            if(g_hdmi_check_commcrc)
            {
                time_ms = hdmi_get_system_time_ms();
                if(video_state==MAIN_FSM_HDMI_VIDEO_READY && cur_commcrc!=g_hdmi_check_commcrc && time_ms>=g_hdmi_check_commcrc_over_ms)
                {
                    ++commcrc_fail_acc;
                    if(commcrc_fail_acc>=g_hdmi_check_commcrc_max_fail_times)
                    {
                        HDMI_EMG("check commcrc fail %d times: target COMMCRC=0x%x, current COMMCRC=0x%x, system time=%dms \n",commcrc_fail_acc,g_hdmi_check_commcrc,cur_commcrc,time_ms);
                        commcrc_fail_acc=0;
                    }
                }
            }
            CEDERR_FLOW_PRINTF("Signal STS Port[%d]: PhyState=%d; CHAR(b,g,r)=(%d,%d,%d)); MIN=%d,mac_clk(b,g,r,c,rate)=(%d,%d,%d,%d,%d); BCH_ERR cnt(1,2)=(%d,%d); AVMUTE=%d; BGRHV=0x%x;5V(%d,%d,%d,%d)\n",
                port,
                p_phy_st->phy_proc_state,
                char_b_err,
                char_g_err,
                char_r_err,
                VALID_MIN_CLOCK,
                current_clk[0],
                current_clk[1],
                current_clk[2],
                current_clk[3],
                newbase_hdmi_clkdet_calculate_tmds_character_rate(current_clk[3]),
                hdmi_rx[port].bch_1bit_err_cnt,
                hdmi_rx[port].bch_err_cnt,
                lib_hdmi_get_avmute(port),
                (lib_hdmi_read_rgb_hv_status(port) & 0x000000F8) >> 3,
                is_5V[0],
                is_5V[1],
                is_5V[2],
                is_5V[3]);

            CEDERR_FLOW_PRINTF("FSM = %s(%d), des_crc=0x%x; COMMCRC=0x%x; PP_CRC=0x%08x, crc_err_cnt=%d, crc_auto_cmp=%d\n",
                _hdmi_video_fsm_str(video_state),
                video_state,
                lib_hdmi_get_des_crc(),
                cur_commcrc,
                lib_hdmi_read_crc(port),
                lib_hdmi_get_crc_err_cnt(),
                lib_hdmi_get_crc_auto_cmp_status());
        }

        newbase_hdmi_clkdet_set_pop_up(port);
        hdmi_char_bit_debug_print_cnt[port] = 0;
    }
    else
    {
        if((newbase_hdmi_ced_err_thd_check(port, DIAGNOSTIC_CED_ERROR_THD)) ||
            (newbase_hdmi_ced_err_thd_check(port, DIAGNOSTIC_CED_ERROR_THD)))
            hdmi_char_bit_debug_print_cnt[port] +=3;
        else
            hdmi_char_bit_debug_print_cnt[port] +=1;
    }

    if (hdmi_scdc_debug_print_cnt[port] >= g_scdc_print_cycle_cnt_thd)
    {
        if(p_phy_st->frl_mode != MODE_TMDS)
        {
            if(GET_FRL_LT_FSM(port) >= LT_FSM_LTSP_PASS)
            {
                unsigned char nport = port;
                unsigned int  cercr_hd21 = hdmi_in(HD21_cercr_reg);
                unsigned char video_state = newbase_hdmi_get_video_state(port);
                unsigned char scdc_10 = lib_hdmi_scdc_read(nport,  0x10);
                unsigned char scdc_11 = lib_hdmi_scdc_read(nport,  0x11);
                unsigned char scdc_20 = lib_hdmi_scdc_read(nport,  0x20);
                unsigned char scdc_21 = lib_hdmi_scdc_read(nport,  0x21);
                unsigned char scdc_30 = lib_hdmi_scdc_read(nport,  0x30);
                unsigned char scdc_31 = lib_hdmi_scdc_read(nport,  0x31);
                unsigned char scdc_40 = lib_hdmi_scdc_read(nport,  0x40);
                unsigned char scdc_41 = lib_hdmi_scdc_read(nport,  0x41);
                unsigned char scdc_42 = lib_hdmi_scdc_read(nport,  0x42);
                //unsigned char scdc_50 = lib_hdmi_scdc_read(nport,  0x50);
                //unsigned char scdc_51 = lib_hdmi_scdc_read(nport,  0x51);
                unsigned char scdc_52 = lib_hdmi_scdc_read(nport,  0x52);
                unsigned char scdc_53 = lib_hdmi_scdc_read(nport,  0x53);
                unsigned char scdc_54 = lib_hdmi_scdc_read(nport,  0x54);
                unsigned char scdc_55 = lib_hdmi_scdc_read(nport,  0x55);
                unsigned char scdc_56 = lib_hdmi_scdc_read(nport,  0x56);
                unsigned char scdc_57 = lib_hdmi_scdc_read(nport,  0x57);
                unsigned char scdc_58 = lib_hdmi_scdc_read(nport,  0x58);

                SCDC_FLOW_PRINTF("SCDC STS 2.1 Port[%d]: SCDCCR=%08x; CERCR=(%08x),SCDC:0x10=%02x,%02x;0x20=%02x,%02x;0x30=%02x,%02x;0x40=%02x,%02x,%2x;52_58=%x,%x,%x,%x,%x,%x,%x;FSM=%s(%d)\n",
                    port,
                    hdmi_in(HDMI_STB_SCDC_CR_reg),
                    cercr_hd21,
                    scdc_10,
                    scdc_11,
                    scdc_20,
                    scdc_21,
                    scdc_30,
                    scdc_31,
                    scdc_40,
                    scdc_41,
                    scdc_42,
                    scdc_52,
                    scdc_53,
                    scdc_54,
                    scdc_55,
                    scdc_56,
                    scdc_57,
                    scdc_58,
                    _hdmi_video_fsm_str(video_state),
                    video_state);
            }

        }
        else
        {
            unsigned char nport = port;
            unsigned int  cercr = hdmi_in(HDMI_CERCR_reg);
            unsigned char tmds_config = lib_hdmi_scdc_get_tmds_config(port);
            unsigned char video_state = newbase_hdmi_get_video_state(port);
            unsigned char scdc_10 = lib_hdmi_scdc_read(nport,  0x10);
            unsigned char scdc_11 = lib_hdmi_scdc_read(nport,  0x11);
            unsigned char scdc_20 = lib_hdmi_scdc_read(nport,  0x20);
            unsigned char scdc_21 = lib_hdmi_scdc_read(nport,  0x21);
            unsigned char scdc_30 = lib_hdmi_scdc_read(nport,  0x30);
            unsigned char scdc_31 = lib_hdmi_scdc_read(nport,  0x31);
            unsigned char scdc_40 = lib_hdmi_scdc_read(nport,  0x40);
            unsigned char scdc_41 = lib_hdmi_scdc_read(nport,  0x41);
            unsigned char scdc_42 = lib_hdmi_scdc_read(nport,  0x42);
            unsigned char scdc_52 = lib_hdmi_scdc_read(nport,  0x52);
            unsigned char scdc_53 = lib_hdmi_scdc_read(nport,  0x53);
            unsigned char scdc_54 = lib_hdmi_scdc_read(nport,  0x54);
            unsigned char scdc_55 = lib_hdmi_scdc_read(nport,  0x55);
            unsigned char scdc_56 = lib_hdmi_scdc_read(nport,  0x56);
            unsigned char scdc_57 = lib_hdmi_scdc_read(nport,  0x57);
            unsigned char scdc_58 = lib_hdmi_scdc_read(nport,  0x58);

            SCDC_FLOW_PRINTF("SCDC STS Port[%d]: SCDCCR=%08x; CERCR=(%08x),TMDS_config(reg,FW40x,FWscrb,FWtoggle)=(%02x,%d,%d,%d); SCDC:0x10=%02x,%02x;0x20=%02x,%02x;0x30=%02x,%02x;0x40=%02x,%02x,%02x;52_58=%x,%x,%x,%x,%x,%x,%x;FSM=%s(%d)\n",
                port,
                hdmi_in(HDMI_STB_SCDC_CR_reg),
                cercr,
                tmds_config,
                newbase_hdmi2p0_get_clock40x_flag(port),
                newbase_hdmi2p0_get_scramble_flag(port),
                newbase_hdmi2p0_get_tmds_toggle_flag(port),
                scdc_10,
                scdc_11,
                scdc_20,
                scdc_21,
                scdc_30,
                scdc_31,
                scdc_40,
                scdc_41,
                scdc_42,
                scdc_52,
                scdc_53,
                scdc_54,
                scdc_55,
                scdc_56,
                scdc_57,
                scdc_58,
                _hdmi_video_fsm_str(video_state),
                video_state);
        }
        hdmi_scdc_debug_print_cnt[port] = 0;
    }
    else
    {
        hdmi_scdc_debug_print_cnt[port] ++;
    }
}

/*---------------------------------------------------
 * Func : lib_hdmi_scdc_mask_update_flags
 *
 * Desc : set update flags to specified bit
 *
 * Para : nport : HDMI port number 
 *           mask : specified bit
 *           enable : set value
 * Retn : N/A
 *--------------------------------------------------*/
void newbase_hdmi_scdc_mask_update_flags(
    unsigned char           nport, 
    unsigned char           mask,
    unsigned char           enable
    )
{
    unsigned char original_value = lib_hdmi_scdc_get_update_flags(nport);
    if(enable)
    {
         lib_hdmi_scdc_set_update_flags(nport, (original_value | mask));
    }
    else
    {
         lib_hdmi_scdc_set_update_flags(nport, (original_value&~mask));
    }
}

/*---------------------------------------------------
 * Func : newbase_hdmi_scdc_hw_mask_enable
 *
 * Desc : set update flags to specified bit
 *
 * Para : nport : HDMI port number 
 *           mask : specified bit
 *           enable : set value
 * Retn : N/A
 *--------------------------------------------------*/
void newbase_hdmi_scdc_hw_mask_enable(
    unsigned char           nport, 
    unsigned char           enable
    )
{
    HDMI_INFO("[newbase_hdmi_scdc_hw_mask_enable port:%d] enable=%d, scdc_0x10=%x, scdc_0x40=%x,%x,%x\n",
        nport, enable, lib_hdmi_scdc_read(nport, SCDC_UPDATE_FLAGS), lib_hdmi_scdc_read(nport, SCDC_STATUS_FLAGS),  lib_hdmi_scdc_read(nport, SCDC_STATUS_FLAGS_1),  lib_hdmi_scdc_read(nport, SCDC_STATUS_FLAGS_2));
    hdmi_mask(HDMI_STB_SCDC_CR_reg, ~HDMI_STB_SCDC_CR_hw_mask_en_mask, HDMI_STB_SCDC_CR_hw_mask_en(enable));
}

/*---------------------------------------------------
 * Func : newbase_hdmi_errdet_status_init
 *
 * Desc : scdc ced_update_config init variable.
 *
 * Para : nport : HDMI port number 
 *
 * Retn : N/A
 *--------------------------------------------------*/
void newbase_hdmi_errdet_status_init(HDMI_ERRDET_TYPE errdet_type)
{
    unsigned char i = 0;
    m_period_errdet_status[errdet_type].errdet_type = errdet_type;

    for(i = 0; i<4 ; i++)
    {
        m_period_errdet_status[errdet_type].last_error_cnt[i] = 0;
        m_period_errdet_status[errdet_type].ced_max_correction_cnt[i] = 0;
        m_period_errdet_status[errdet_type].start_record_error_index[i] = 0;
        m_period_errdet_status[errdet_type].start_record_time[i] = 0;
        m_period_errdet_status[errdet_type].period_wait_timeout_flag[i] = FALSE;
        m_period_errdet_status[errdet_type].period_err_correction_cnt[i] = 0;
    }
}



 unsigned char newbase_hdmi_period_error_duration_check(HDMI_FRL_MODE_T frl_mode, HDMI_ERRDET_CONFIG_T* p_period_errdet_config, HDMI_PERIOD_ERRDET_STATUS_T* p_period_errdet_status, int current_err_ent[4])
{
    unsigned char is_error_cnt_over_max= FALSE;
    unsigned char lane_max = 0;
    unsigned char i = 0;
    int error_count_max = p_period_errdet_config->max_err_count;

    lane_max = newbase_hdmi_hd21_get_frl_lane(frl_mode);

    for(i = 0; i< lane_max; i++)
    {
        unsigned int current_time_ms = hdmi_get_system_time_ms();

        //===============================
        // period error handler
        //===============================
        if(p_period_errdet_status->period_wait_timeout_flag[i] == TRUE)
        {
            if((current_err_ent[i] -p_period_errdet_status->start_record_error_index[i]) >= (p_period_errdet_config->over_y_cnt-1) )
            {

                p_period_errdet_status->period_err_correction_cnt[i] ++;
                p_period_errdet_status->is_error_overthd= TRUE;

                p_period_errdet_status->period_wait_timeout_flag[i] = FALSE;
            }
            else
            {
                ;//error count is <5, do nothing
            }

            if( (current_time_ms - p_period_errdet_status->start_record_time[i]) > p_period_errdet_config->every_x_ms)
            {
                p_period_errdet_status->period_wait_timeout_flag[i] = FALSE;  // 1s timeout!
                HDMI_WARN("[ERR_P][%s][TOO_SMALL] Still no error update, Lane =%d,period_thd =%d, error_cnt(start, current) = (%5d, %5d), Time (start, current) = (%d, %d)\n",
                    p_period_errdet_config->err_det_name, i, p_period_errdet_config->over_y_cnt, p_period_errdet_status->start_record_error_index[i], current_err_ent[i], p_period_errdet_status->start_record_time[i], current_time_ms);
            }
        }
        else
        {

            if((p_period_errdet_status->last_error_cnt[i] < current_err_ent[i]) &&  (p_period_errdet_status->start_record_error_index[i] != current_err_ent[i]))
            {
                p_period_errdet_status->start_record_error_index[i] = current_err_ent[i];
                p_period_errdet_status->start_record_time[i] = hdmi_get_system_time_ms();
                p_period_errdet_status->period_wait_timeout_flag[i] = TRUE;
                p_period_errdet_status->is_error_increasing = TRUE;
            }
        }
 
        if((current_err_ent[i] < p_period_errdet_status->last_error_cnt[i]) && (current_err_ent[i] != p_period_errdet_status->last_error_cnt[i]))
        {
            HDMI_WARN("[ERR_P][%s][INVALID] Mode:%s(%d), last(0,1,2,3) = (%5d, %5d, %5d, %5d), current(0,1,2,3) =(%d, %d, %d, %d)\n",
                p_period_errdet_config->err_det_name,
                _hdmi_hd21_frl_mode_str(frl_mode),
                frl_mode,
                p_period_errdet_status->last_error_cnt[0],
                p_period_errdet_status->last_error_cnt[1],
                p_period_errdet_status->last_error_cnt[2],
                p_period_errdet_status->last_error_cnt[3],
                current_err_ent[0],
                current_err_ent[1],
                current_err_ent[2],
                current_err_ent[3]);
        }

        //===============================
        // Max error_count  handler
        //===============================
        if(current_err_ent[i]>=error_count_max)
        {
            if(p_period_errdet_status->last_error_cnt[i] != current_err_ent[i])
            {
                if(p_period_errdet_status->ced_max_correction_cnt[i] <ERRDET_PRINT_COUNT)
                {// print 3 times.
                    HDMI_WARN("[ERR_P][%s][INC_MAX] ver=%d, Lane =%d, current= %5d, max=%5d, current_ms = %d\n",
                        p_period_errdet_config->err_det_name, get_ic_version(), i, current_err_ent[i], error_count_max, current_time_ms);

                    p_period_errdet_status->ced_max_correction_cnt[i] ++;
                    is_error_cnt_over_max = TRUE;

                }
                else
                {
                    //HDMI_WARN("[CED_UPDATE] over max, Lane i =%d, error = %5d, CED Already Update =%d, correction no =%d\n", i, current_disparity_err[i], ced_update, m_ced_update_config[port].ced_max_correction_cnt[i]);
                }
            }
        }
        p_period_errdet_status->last_error_cnt[i] = current_err_ent[i];
    }

    return is_error_cnt_over_max;
}

/*---------------------------------------------------
 * Func : newbase_hdmi_ced_update_handler
 *
 * Desc : CED_Update Use firmware control on HDMI2.1 
 *
 * Para : nport : HDMI port number, frl_mode: HDMI2.1 FRL mode by Link Training
 *
 * Retn : N/A
 *--------------------------------------------------*/
void newbase_hdmi_errdet_handler(unsigned char port)
{
    int current_err_ent[HDMI_ERRDET_NUM][4];
    unsigned char i = 0;
    HDMIRX_PHY_STRUCT_T* p_phy_st = newbase_rxphy_get_status(port);
    unsigned char lane_max = 0;
    HDMI_FRL_MODE_T frl_mode = 0;
    unsigned char is_run_duration_check[HDMI_ERRDET_NUM];

    if(newbase_hdmi_get_current_display_port() != port)
    {
        return;
    }

    if ((newbase_hdmi_get_hpd(port)==0) || (newbase_hdmi_get_5v_state(port) ==0) || (newbase_hdmi_get_power_saving_state(port)==PS_FSM_POWER_SAVING_ON))
        return;

    if(p_phy_st == NULL)
    {
        HDMI_EMG("[newbase_hdmi_errdet_handler port:%d] ERROR!! NULL pointer p_phy_st\n", port);
        return;
    }

    frl_mode = p_phy_st->frl_mode;
    lane_max = newbase_hdmi_hd21_get_frl_lane(frl_mode);

    //Init setting
    for(i=0; i < HDMI_ERRDET_NUM; i ++)
    {
        is_run_duration_check[i] = FALSE;
        current_err_ent[i][0] = current_err_ent[i][1] = current_err_ent[i][2] = current_err_ent[i][3] = 0;

        m_period_errdet_status[i].errdet_type = m_hdmi_errdet_config_table[i].errdet_type;

        m_period_errdet_status[i].is_error_overthd=  FALSE;
        m_period_errdet_status[i].is_error_increasing=  FALSE;
        
        if(m_hdmi_errdet_config_table[i].is_enable == FALSE)
            continue;

        switch(m_hdmi_errdet_config_table[i].errdet_type)
        {
            case HDMI_HD20_BIT_ERR:
                if(frl_mode == MODE_TMDS)
                {
                    if(p_phy_st->phy_proc_state < PHY_PROC_DONE)
                    {//Wait phy stable
                        newbase_hdmi_errdet_status_init(m_hdmi_errdet_config_table[i].errdet_type);
                        newbase_hdmi_set_online_measure_error_flag(port, 0); //init
                        lib_hdmi_bit_err_start(port, 1, 16);
                        is_run_duration_check[i] = FALSE;
                        break;
                    }

                    if(m_hdmi_errdet_config_table[i].is_continuous_mode== TRUE)
                    {// continuous mode
                        lib_hdmi_bit_err_get_error(port, (unsigned int*)&current_err_ent[i][2], (unsigned int*)&current_err_ent[i][1], (unsigned int*)&current_err_ent[i][0]);
                        current_err_ent[i][3] = 0;
                        is_run_duration_check[i] = TRUE;
                    }
                    else
                    {// one time mode
                        if(GET_H_VIDEO_FSM(port) >= MAIN_FSM_HDMI_VIDEO_READY)
                        {
                            hdmi_rx[port].fw_bit_disp_error_detected = newbase_hdmi_bit_err_detection(port, &current_err_ent[i][2], &current_err_ent[i][1], &current_err_ent[i][0]);
                            is_run_duration_check[i] = FALSE;
                        }
                        break;
                    }
                }

                break;
            case HDMI_HD20_CHAR_ERR:
                if(frl_mode == MODE_TMDS)
                {
                    if(p_phy_st->phy_proc_state < PHY_PROC_DONE)
                    {//Wait phy stable
                        newbase_hdmi_errdet_status_init(m_hdmi_errdet_config_table[i].errdet_type);
                        lib_hdmi_char_err_start(port, 1, 16);
                        is_run_duration_check[i] = FALSE;
                        break;
                    }

                    if(m_hdmi_errdet_config_table[i].is_continuous_mode== TRUE)
                    {// continuous mode
                        lib_hdmi_char_err_get_error(port, (unsigned int*)&current_err_ent[i][2], (unsigned int*)&current_err_ent[i][1], (unsigned int*)&current_err_ent[i][0]);
                        current_err_ent[HDMI_HD20_CHAR_ERR][3] = 0;
                        is_run_duration_check[i] = TRUE;
                    }
                    else
                    {// one time mode
                        if(GET_H_VIDEO_FSM(port) >= MAIN_FSM_HDMI_VIDEO_READY)
                        {
                            newbase_hdmi_char_err_detection(port, &current_err_ent[i][2], &current_err_ent[i][1], &current_err_ent[i][0]);
                            is_run_duration_check[i] = FALSE;
                        }
                        break;
                    }
                }

                break;

            case HDMI_HD21_DISPARITY_ERR:
                if(frl_mode != MODE_TMDS)
                {
                    if(GET_FRL_LT_FSM(port)< LT_FSM_LTSP_PASS)
                    {
                        newbase_hdmi_errdet_status_init(m_hdmi_errdet_config_table[i].errdet_type);
                        is_run_duration_check[i] = FALSE;
                        break;
                    }
                    lib_hdmi_char_err_get_scdc_ced(port, frl_mode, (unsigned int*)current_err_ent[i]);
                    is_run_duration_check[i] = TRUE;
                }

                break;

            case HDMI_HD21_RS_ERR:
                if(frl_mode != MODE_TMDS)
                {
                    if(GET_FRL_LT_FSM(port)< LT_FSM_LTSP_PASS)
                    {
                        newbase_hdmi_errdet_status_init(m_hdmi_errdet_config_table[i].errdet_type);
                        is_run_duration_check[i] = FALSE;
                        break;
                    }
                    current_err_ent[i][0] = lib_hdmi_hd21_get_rs_err_cnt(port);
                    current_err_ent[i][1] = current_err_ent[i][2] = current_err_ent[i][3] = 0;
                    is_run_duration_check[i] = TRUE;
                }

                break;
            default:
                break;
        }
        
    }

    for(i = 0; i <  HDMI_ERRDET_NUM; i++)
    {
        if(m_hdmi_errdet_config_table[i].is_enable == FALSE)
            continue;
        if(is_run_duration_check[i] == FALSE)
            continue;

    	 // 1. Check error count over thd
        newbase_hdmi_period_error_duration_check(frl_mode, &m_hdmi_errdet_config_table[i], &m_period_errdet_status[i], current_err_ent[i]);

        // 2. ErrorDet handler
        switch(m_hdmi_errdet_config_table[i].errdet_type)
        {
        case HDMI_HD20_BIT_ERR:

            if(GET_H_VIDEO_FSM(port) >= MAIN_FSM_HDMI_VIDEO_READY)
            {
            // For diagionstic debugging
                if(m_period_errdet_status[i].is_error_overthd== TRUE)
                {
                    hdmi_rx[port].fw_bit_error[0] = (current_err_ent[i][0] -m_period_errdet_status[i].start_record_error_index[0]);
                    hdmi_rx[port].fw_bit_error[1] = (current_err_ent[i][1] -m_period_errdet_status[i].start_record_error_index[1]);
                    hdmi_rx[port].fw_bit_error[2] = (current_err_ent[i][2] -m_period_errdet_status[i].start_record_error_index[2]);
                    hdmi_rx[port].fw_bit_error[3] = (current_err_ent[i][3] -m_period_errdet_status[i].start_record_error_index[3]);

                    hdmi_rx[port].fw_bit_disp_error_detected = TRUE;
                }
                else
                {
                    if(hdmi_rx[port].fw_bit_disp_error_detected == TRUE)
                    {
                        hdmi_rx[port].fw_bit_disp_error_detected = FALSE;
                        HDMI_WARN("[ERR_P][%s][NO_INC] last fw_bit_error (0,1,2) = (%5d,%5d,%5d)), no need to do phy re-adaptive\n",
                            m_hdmi_errdet_config_table[i].err_det_name,  hdmi_rx[port].fw_bit_error[0], hdmi_rx[port].fw_bit_error[1], hdmi_rx[port].fw_bit_error[2]);
                    }
                }
            }
            break;
        case HDMI_HD20_CHAR_ERR:

            if((GET_H_VIDEO_FSM(port) >= MAIN_FSM_HDMI_VIDEO_READY) && (m_period_errdet_status[i].is_error_overthd== TRUE))
            {
            // For diagionstic debugging
                hdmi_rx[port].fw_char_error[0] = (current_err_ent[i][0] -m_period_errdet_status[i].start_record_error_index[0]);
                hdmi_rx[port].fw_char_error[1] = (current_err_ent[i][1] -m_period_errdet_status[i].start_record_error_index[1]);
                hdmi_rx[port].fw_char_error[2] = (current_err_ent[i][2] -m_period_errdet_status[i].start_record_error_index[2]);
                hdmi_rx[port].fw_char_error[3] = (current_err_ent[i][3] -m_period_errdet_status[i].start_record_error_index[3]);
            }
            if(GET_H_VIDEO_FSM(port) >=MAIN_FSM_HDMI_VIDEO_READY)
            {
                if((current_err_ent[i][0] >= m_hdmi_errdet_config_table[i].max_err_count) &&
                    (current_err_ent[i][1] >= m_hdmi_errdet_config_table[i].max_err_count)&&
                    (current_err_ent[i][2] >= m_hdmi_errdet_config_table[i].max_err_count)&&
                    (newbase_hdmi_get_online_measure_error_flag(port) == 0))
                {
                    newbase_hdmi_set_online_measure_error_flag(port, 1);
                    HDMI_WARN("[ERR_P][%s][FULL_MAX] max=%5d, need to do full reset phy when onms error occurred\n",
                        m_hdmi_errdet_config_table[i].err_det_name,  m_hdmi_errdet_config_table[i].max_err_count);
                }
            }
            break;

        default:
            break;
        }
        // 3. print result
        if((m_period_errdet_status[i].is_error_overthd ==TRUE) && (m_hdmi_errdet_config_table[i].is_polling_print == TRUE))
        {
            m_period_errdet_status[i].is_error_increasing = FALSE;

            if(lane_max==3)
            {
                HDMI_WARN("[ERR_P][%s][INC_OVERTHD] Port:%d, frl_mode:%s(%d), FSM:%s, current_err(0,1,2) = (%5d,%5d,%5d), diff(0,1,2) =(%d,%d,%d),OVER %d cnt in %d ms\n",
                    m_hdmi_errdet_config_table[i].err_det_name,
                    port,
                    _hdmi_hd21_frl_mode_str(frl_mode),
                    frl_mode,
                    _hdmi_video_fsm_str(GET_H_VIDEO_FSM(port)),
                    m_period_errdet_status[i].last_error_cnt[0],
                    m_period_errdet_status[i].last_error_cnt[1],
                    m_period_errdet_status[i].last_error_cnt[2],
                    (m_period_errdet_status[i].last_error_cnt[0] - m_period_errdet_status[i].start_record_error_index[0]),
                    (m_period_errdet_status[i].last_error_cnt[1] - m_period_errdet_status[i].start_record_error_index[1]),
                    (m_period_errdet_status[i].last_error_cnt[2] - m_period_errdet_status[i].start_record_error_index[2]),
                    m_hdmi_errdet_config_table[i].over_y_cnt,
                    m_hdmi_errdet_config_table[i].every_x_ms);
            }
            else
            {
                HDMI_WARN("[ERR_P][%s][INC_OVERTHD] Port:%d, frl_mode:%s(%d), FSM:%s, current_err(0,1,2,3) = (%5d,%5d,%5d,%5d), diff(0,1,2,3) =(%d,%d,%d,%d),OVER %d cnt in %d ms\n",
                    m_hdmi_errdet_config_table[i].err_det_name,
                    port,
                    _hdmi_hd21_frl_mode_str(frl_mode),
                    frl_mode,
                    _hdmi_video_fsm_str(GET_H_VIDEO_FSM(port)),
                    m_period_errdet_status[i].last_error_cnt[0],
                    m_period_errdet_status[i].last_error_cnt[1],
                    m_period_errdet_status[i].last_error_cnt[2],
                    m_period_errdet_status[i].last_error_cnt[3],
                    (m_period_errdet_status[i].last_error_cnt[0] - m_period_errdet_status[i].start_record_error_index[0]),
                    (m_period_errdet_status[i].last_error_cnt[1] - m_period_errdet_status[i].start_record_error_index[1]),
                    (m_period_errdet_status[i].last_error_cnt[2] - m_period_errdet_status[i].start_record_error_index[2]),
                    (m_period_errdet_status[i].last_error_cnt[3] - m_period_errdet_status[i].start_record_error_index[3]),
                    m_hdmi_errdet_config_table[i].over_y_cnt,
                    m_hdmi_errdet_config_table[i].every_x_ms);
            }
        }

        if((m_period_errdet_status[i].is_error_increasing==TRUE) && (m_hdmi_errdet_config_table[i].is_polling_print == TRUE))
        {
            m_period_errdet_status[i].is_error_overthd = FALSE;
            m_period_errdet_status[i].is_error_increasing = FALSE;
            if(lane_max==3)
            {
                HDMI_WARN("[ERR_P][%s][INC_ING] Port:%d, frl_mode:%s(%d), FSM:%s, current_err(0,1,2) = (%5d,%5d,%5d),in %d ms\n",
                    m_hdmi_errdet_config_table[i].err_det_name,
                    port,
                    _hdmi_hd21_frl_mode_str(frl_mode),
                    frl_mode,
                    _hdmi_video_fsm_str(GET_H_VIDEO_FSM(port)),
                    m_period_errdet_status[i].last_error_cnt[0],
                    m_period_errdet_status[i].last_error_cnt[1],
                    m_period_errdet_status[i].last_error_cnt[2],
                    m_hdmi_errdet_config_table[i].every_x_ms);
            }
            else
            {
                HDMI_WARN("[ERR_P][%s][INC_ING] Port:%d, frl_mode:%s(%d), FSM:%s, current_err(0,1,2,3) = (%5d,%5d,%5d,%5d),in %d ms\n",
                    m_hdmi_errdet_config_table[i].err_det_name,
                    port,
                    _hdmi_hd21_frl_mode_str(frl_mode),
                    frl_mode,
                    _hdmi_video_fsm_str(GET_H_VIDEO_FSM(port)),
                    m_period_errdet_status[i].last_error_cnt[0],
                    m_period_errdet_status[i].last_error_cnt[1],
                    m_period_errdet_status[i].last_error_cnt[2],
                    m_period_errdet_status[i].last_error_cnt[3],
                    m_hdmi_errdet_config_table[i].every_x_ms);

            }
        }
    }
}

HDMI_ERRDET_CONFIG_T* newbase_hdmi_get_errdet_config_table(HDMI_ERRDET_TYPE errdet_type)
{
    unsigned char i = 0;
    unsigned char target_index = 0;
    for(i = 0; i < HDMI_ERRDET_NUM; i++)
    {
        if(m_hdmi_errdet_config_table[i].errdet_type == errdet_type)
        {
            target_index = i;
            break;
        }
    }
    return &m_hdmi_errdet_config_table[target_index];
}


HDMI_PERIOD_ERRDET_STATUS_T* newbase_hdmi_get_errdet_status(HDMI_ERRDET_TYPE errdet_type)
{
    unsigned char i = 0;
    unsigned char target_index = 0;
    for(i = 0; i < HDMI_ERRDET_NUM; i++)
    {
        if(m_period_errdet_status[i].errdet_type == errdet_type)
        {
            target_index = i;
            break;
        }
    }
    return &m_period_errdet_status[target_index];
}


/*---------------------------------------------------
 * Func : newbase_hdmi_fw_ced_measure
 *
 * Desc : measure ced for a fixed period 
 *
 * Para : nport : HDMI port number
 *        ced   : CED value
 *        period : measure time (in ms)
 *
 * Retn : N/A
 *
 * Note : this function will be called by newbase_rxphy_isr 
 *        each 10 ms...
 *--------------------------------------------------*/
void newbase_hdmi_fw_ced_measure(unsigned char port, unsigned int ced[3], unsigned int period)
{
    unsigned char nport = port;

    if (hdmi_rx[nport].hdmi_2p1_en)
        hdmi_out(HD21_cercr_reg, 0x40001214);
    else
        hdmi_out(HDMI_CERCR_reg, 0x40001214);

    lib_hdmi_scdc_ced_enable(port, 1);
    lib_hdmi_scdc_ced_error_detect_enable(port, 0);
    lib_hdmi_scdc_ced_continue_mode_en(port, 0);
    lib_hdmi_scdc_ced_reset(port);
    lib_hdmi_scdc_ced_continue_mode_en(port, 1);
    lib_hdmi_scdc_ced_error_detect_enable(port, 1);

    msleep(period);
    lib_hdmi_char_err_get_error(port, &ced[0], &ced[1], &ced[2]);
}


/*---------------------------------------------------
 * Func : newbase_hdmi_scdc_init
 *
 * Desc : scdc initail setting 
 *
 * Para : nport : HDMI port number 
 *
 * Retn : N/A
 *--------------------------------------------------*/
void newbase_hdmi_scdc_init(unsigned char port)
{
    spin_lock_init(&hdmi_scdc_lock);
    lib_hdmi_scdc_enable(port, 1);
    lib_hdmi_scdc_set_sink_version(port, 0x01); // HDMI2.0/10.4.1.2/Sink Devices compliant with this version of the specification shall set Sink Version = 1.
    lib_hdmi_scdc_set_update10_hwauto_en(port, SCDC_UPDATE_FLAGS_STATUS_UPDATE, FALSE); //Status_update is FW controlling, other is HW auto
    lib_hdmi_scdc_set_update10_hwauto_en(port, SCDC_UPDATE_FLAGS_FLT_UPDATE, FALSE); //Flt_update use fw control.

    lib_hdmi_scdc_set_src_clr_frl_start(port, 1);//  allow srouce to clear frl start flag
}

/*---------------------------------------------------
 * Func : newbase_hdmi_scdc_handler
 *
 * Desc : scdc main function. the driver use this function to
 *        handle SCDC event. 
 *
 * Para : nport : HDMI port number
 *   
 * Retn : N/A
 *
 * Note : this function will be called by newbase_rxphy_isr 
 *        each 10 ms...
 *--------------------------------------------------*/
void newbase_hdmi_scdc_handler(unsigned char port)
{
    // this function will be called by newbase_rxphy_isr each 10 ms...

    newbase_hdmi_scdc_flow_print(port);

    if ((lib_hdmi_scdc_get_update_flags(port) & SCDC_UPDATE_FLAGS_RR_TEST))  // RR request is set
    {
        lib_hdmi_scdc_set_update_flags(port, 0x00);
        lib_hdmi_scdc_set_update_flags(port, SCDC_UPDATE_FLAGS_RR_TEST);
    }

    if (lib_hdmi_scdc_is_tmds_config_updated(port))
    {
        unsigned char config = lib_hdmi_scdc_get_tmds_config(port);
        unsigned char current_tmds_bit_clock_ratio = (config>>1) & 0x1;
        unsigned char current_scrambling_enable = (config & 0x1);
        lib_hdmi_scdc_clr_tmds_config_update_status(port);
        newbase_hdmi2p0_reset_scdc_toggle(port);        // reset SCDC toggle
        HDMI_SCDC_WARN("Port %d, TMDS config was updated (%02x - TMDS_Bit_Clock_Ratio=%d, Scrambling_En=%d)\n", 
            port, config, current_tmds_bit_clock_ratio, current_scrambling_enable);

        if((last_scdc_tmds_bit_clock_ratio[port] != current_tmds_bit_clock_ratio) ||
            (last_scdc_scrambling_enable[port] != current_scrambling_enable))
        {
            HDMI_SCDC_WARN("Port %d, TMDS config has changed!! Have to reset phy and Video FSM. TMDS_40x(last,now) = (%d,%d), scrambling(last,now) = (%d,%d)\n",
                port,
                last_scdc_tmds_bit_clock_ratio[port],
                current_tmds_bit_clock_ratio,
                last_scdc_scrambling_enable[port],
                current_scrambling_enable);

            last_scdc_tmds_bit_clock_ratio[port] = current_tmds_bit_clock_ratio;
            last_scdc_scrambling_enable[port] = current_scrambling_enable;

            //Reset all flow when 5V connected
            lib_hdmi_video_output(port, 0);
            lib_hdmi_audio_output(port, 0);

            SET_HDMI_DETECT_EVENT(port, HDMI_DETECT_TMDS_CONFIG_CHANGE);
            newbase_rxphy_reset_setphy_proc(port);    //reset phy state
            newbase_hdmi_hdcp_reset_fsm(port);    //reset hdcp
            lib_hdmi_misc_variable_initial(port);    //reset global variable

        }
        // TODO : should we update sink config here?                
        
    }
    if (GET_FLOW_CFG(HDMI_FLOW_CFG_ERR_DET, HDMI_FLOW_CFG4_ERRDET_ENABLE ) == TRUE)
    {
        newbase_hdmi_errdet_handler(port);    //Signal error count handler

    }

    newbase_hdmi_hd21_check_rsed_upd_flag(port);
    
    newbase_hdmi_hd21_link_training_fsm(port);

}
