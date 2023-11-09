#ifndef __HDMI_SCDC_H__
#define __HDMI_SCDC_H__
#include "hdmi_debug.h"

#define SCDC_SINK_VERSION                    0x01
#define SCDC_SOURCE_VERSION                  0x02
#define SCDC_UPDATE_FLAGS                    0x10
#define SCDC_TMDS_CONFIGURATION              0x20 
#define SCDC_TMDS_SCRAMBLER_STATUS           0x21   
#define SCDC_CONFIGURATION                   0x30
#define SCDC_CONFIGURATION_1                 0x31
#define SCDC_SOURCE_TEST_CONFIGURATION       0x35
#define SCDC_STATUS_FLAGS                    0x40
#define SCDC_STATUS_FLAGS_1                  0x41
#define SCDC_STATUS_FLAGS_2                  0x42
#define SCDC_LN0_CED_ERROR_L                  0x50
#define SCDC_LN0_CED_ERROR_H                  0x51
#define SCDC_LN1_CED_ERROR_L                  0x52
#define SCDC_LN1_CED_ERROR_H                  0x53
#define SCDC_LN2_CED_ERROR_L                  0x54
#define SCDC_LN2_CED_ERROR_H                  0x55
#define SCDC_CED_CHECKSUM                  0x56
#define SCDC_LN3_CED_ERROR_L                  0x57
#define SCDC_LN3_CED_ERROR_H                  0x58
#define SCDC_RS_CORRECTION_CNT_L                  0x59
#define SCDC_RS_CORRECTION_CNT_H                  0x5A


#define SCDC_CED_OFST                        0x50
#define SCDC_CED_LEN                         11

#define SCDC_MANUFACTURER_OUI_OFST           0xD0
#define SCDC_MANUFACTURER_OUI_LEN            3

#define SCDC_DEVICE_ID_OFST                  0xD3
#define SCDC_DEVICE_ID_LEN                   12

#define SCDC_TEST_READ_REQUEST               0xC0

// Update Flags (0x10)
#define SCDC_UPDATE_FLAGS_STATUS_UPDATE      (0x1)
#define SCDC_UPDATE_FLAGS_CED_UPDATE         (0x1<<1)
#define SCDC_UPDATE_FLAGS_RR_TEST            (0x1<<2)
#define SCDC_UPDATE_FLAGS_SOURCE_TEST_UPDATE (0x1<<3)
#define SCDC_UPDATE_FLAGS_FRL_START          (0x1<<4)
#define SCDC_UPDATE_FLAGS_FLT_UPDATE         (0x1<<5)
#define SCDC_UPDATE_FLAGS_RSED_UPDATE        (0x1<<6)   
   
// SCDC_TMDS_CONFIGURATION (0x20)
#define SCDC_SCAMBLING_ENABLE                (0x1)
#define SCDC_TMDS_BIT_CLOCK_RATIO            (0x1<<1)  
    
// SCDC_SCRAMBLER_STATUS (0x21)
#define SCDC_SCAMBLING_STATUS                (0x1)  
   
// SCDC_CONFIGURATION(0x30)
#define SCDC_RR_ENABLE                       (0x1)  
#define SCDC_FLT_NO_RETRAIN                       (0x1<<1)

// SCDC_CONFIGURATION(0x31)
#define SCDC_FRL_RATE                       (0xF)  
#define SCDC_FFE_LEVELS                       (0xF<<4)


// SCDC_CONFIGURATION(0x35)
#define SCDC_TXFFE_PRE_SHOOT_ONLY                       (0x1<<1)
#define SCDC_TXFFE_DE_EMPHASIS_ONLY                       (0x1<<2)
#define SCDC_TXFFE_NO_FFE                       (0x1<<3)
#define SCDC_FLT_NO_TIMEOUT                    (0x1<<5)
#define SCDC_DSC_FRL_MAX                      (0x1<<6)
#define SCDC_FRL_MAX                      (0x1<<7)


// SCDC_STATUS (0x40)
#define SCDC_STATUS_DSC_DECODE_FAIL          (0x1<<7)
#define SCDC_STATUS_FLT_READY                (0x1<<6)
#define SCDC_STATUS_LANE3_LOCKED             (0x1<<4)
#define SCDC_STATUS_CH2_LN2_LOCK             (0x1<<3)
#define SCDC_STATUS_CH1_LN1_LOCK             (0x1<<2)
#define SCDC_STATUS_CH0_LN0_LOCK             (0x1<<1)
#define SCDC_STATUS_CLOCK_DETECTED           (0x1)

// SCDC_STATUS_1 (0x41)
#define SCDC_STATUS_LN0_LTP_REQ           (0xF)
#define SCDC_STATUS_LN1_LTP_REQ           (0xF<<4)

#define SCDC_SHIFT_STATUS_LN0_LTP_REQ(x)           ((x & 0xF))
#define SCDC_SHIFT_STATUS_LN1_LTP_REQ(x)           ((x & 0xF)<<4)

// SCDC_STATUS_2 (0x42)
#define SCDC_STATUS_LN2_LTP_REQ           (0xF)
#define SCDC_STATUS_LN3_LTP_REQ           (0xF<<4)

#define SCDC_SHIFT_STATUS_LN2_LTP_REQ(x)           ((x & 0xF))
#define SCDC_SHIFT_STATUS_LN3_LTP_REQ(x)           ((x & 0xF)<<4)

// SCDC_LN0_CED_ERROR_H (0x51)
#define SCDC_CED_ERROR_LN0_VALID           (0x1<<7)

// SCDC_LN1_CED_ERROR_H (0x53)
#define SCDC_CED_ERROR_LN1_VALID           (0x1<<7)

// SCDC_LN2_CED_ERROR_H (0x55)
#define SCDC_CED_ERROR_LN2_VALID           (0x1<<7)

// SCDC_LN3_CED_ERROR_H (0x58)
#define SCDC_CED_ERROR_LN3_VALID           (0x1<<7)

// SCDC_RS_CORRECTION_CNT_H (0x5A)
#define SCDC_RS_C_VALID           (0x1<<7)




typedef enum
{
    HDMI_HD20_BIT_ERR = 0,
    HDMI_HD20_CHAR_ERR,
    HDMI_HD21_DISPARITY_ERR,
    HDMI_HD21_SYMBOL_ERR,
    HDMI_HD21_RS_ERR,
    HDMI_ERRDET_NUM
}HDMI_ERRDET_TYPE;

typedef 	struct
{
    HDMI_ERRDET_TYPE errdet_type;
    unsigned char is_enable;
    unsigned char is_continuous_mode;   // 0: one time mode, 1: continuous mode
    unsigned char is_polling_print;
    int max_err_count; //define by IC register length
    unsigned int every_x_ms;
    int over_y_cnt; // every x ms, over y count
    unsigned char err_det_name[16];
}HDMI_ERRDET_CONFIG_T;


typedef struct
{
    HDMI_ERRDET_TYPE errdet_type;
    int last_error_cnt[4]; // for max ced error
    int ced_max_correction_cnt[4]; // for max ced error
    unsigned int start_record_time[4]; // for period error use
    int start_record_error_index[4];    // for period error use
    unsigned char period_wait_timeout_flag[4];    // for period error use
    unsigned int period_err_correction_cnt[4];    //for period error use
    unsigned char is_error_increasing; //over y cnt = TRUE
    unsigned char is_error_overthd; //over y cnt = TRUE
}HDMI_PERIOD_ERRDET_STATUS_T;


//------------------------------------------------------------------------------
extern void lib_hdmi_scdc_enable(unsigned char nport, unsigned char enable);
extern void lib_hdmi_scdc_data_sel(unsigned char nport, unsigned char hdmi2p1);
extern void lib_hdmi_scdc_ced_error_det_sum_port_sel(unsigned char nport, unsigned char frl_lane);
extern void lib_hdmi_scdc_reset(unsigned char nport, unsigned char rst);
extern void lib_hdmi_scdc_reset_toggle(unsigned char nport);
extern void lib_hdmi_scdc_write(unsigned char nport, unsigned char addr,unsigned char value);
extern void lib_hdmi_scdc_write_mask(unsigned char nport, unsigned char addr, unsigned char mask, unsigned char value);
extern unsigned char lib_hdmi_scdc_read(unsigned char nport, unsigned char addr);
extern void lib_hdmi_scdc_ced_enable(unsigned char nport, unsigned char enable);

extern unsigned char lib_hdmi_scdc_get_tmds_config_updated(unsigned char nport);
extern void lib_hdmi_scdc_clr_tmds_config_update_status(unsigned char nport);

extern unsigned char lib_hdmi_scdc_is_sink_config_31_updated(unsigned char nport);
extern void lib_hdmi_scdc_clr_sink_config_31_update_status(unsigned char nport);
extern unsigned char lib_hdmi_scdc_get_sink_config_31(unsigned char nport);

//--------
extern void lib_hdmi_scdc_set_sink_version(unsigned char nport, unsigned char ver);
extern unsigned char lib_hdmi_scdc_get_source_version(unsigned char nport);
extern void lib_hdmi_scdc_set_update_flags(unsigned char nport, unsigned char flags);
extern unsigned char lib_hdmi_scdc_get_update_flags(unsigned char nport);
extern unsigned char lib_hdmi_scdc_get_tmds_config(unsigned char nport);
extern unsigned char lib_hdmi_scdc_get_tmds_scrambler_status(unsigned char nport);
extern unsigned char lib_hdmi_scdc_get_test_read_request(unsigned char nport);

extern void lib_hdmi_scdc_get_ced(unsigned char nport, unsigned char ced[SCDC_CED_LEN]);
extern void lib_hdmi_scdc_set_manufacturer_oui(unsigned char nport, unsigned char oui[SCDC_MANUFACTURER_OUI_LEN]);
extern void lib_hdmi_scdc_get_manufacturer_oui(unsigned char nport, unsigned char oui[SCDC_MANUFACTURER_OUI_LEN]);

extern void lib_hdmi_scdc_set_device_id(unsigned char nport, unsigned char id[SCDC_DEVICE_ID_LEN]);
extern void lib_hdmi_scdc_get_device_id(unsigned char nport, unsigned char id[SCDC_DEVICE_ID_LEN]);
extern void lib_hdmi_scdc_set_maximum_char_err(unsigned char nport, unsigned int max_cnt);

//------------------------------------------------------------------------------
extern void newbase_hdmi_scdc_flow_print(unsigned char port);
extern void newbase_hdmi_scdc_mask_update_flags(unsigned char nport, unsigned char mask, unsigned char enable);
extern void newbase_hdmi_scdc_hw_mask_enable(unsigned char nport, unsigned char enable);
extern void newbase_hdmi_scdc_init(unsigned char port);
extern void newbase_hdmi_scdc_handler(unsigned char port);
extern void lib_hdmi_scdc_set_update10_hwauto_en(unsigned char nport, unsigned int update_flag_mask, unsigned char enable);
//------------------------------------------------------------------------------
// for Link Training
//------------------------------------------------------------------------------
extern void lib_hdmi_scdc_set_flt_ready(unsigned char nport);
extern void lib_hdmi_scdc_set_frl_start(unsigned char nport);
extern void lib_hdmi_scdc_clr_frl_start(unsigned char nport);
extern unsigned char lib_hdmi_scdc_get_frl_rate(unsigned char nport);
extern void lib_hdmi_scdc_set_flt_update(unsigned char nport);
extern unsigned char lib_hdmi_scdc_get_flt_update(unsigned char nport);
extern void lib_hdmi_scdc_update_ltp_req(unsigned char nport, unsigned char ltp[4]);

//------------------------------------------------------------------------------
// for errdet
//------------------------------------------------------------------------------
extern HDMI_ERRDET_CONFIG_T* newbase_hdmi_get_errdet_config_table(HDMI_ERRDET_TYPE errdet_type);
extern HDMI_PERIOD_ERRDET_STATUS_T* newbase_hdmi_get_errdet_status(HDMI_ERRDET_TYPE errdet_type);
//------------------------------------------------------------------------------

extern void newbase_hdmi_sync_qs_scdc_status(unsigned char port);

#ifdef HDMI_SCDC_DBG_EN                        
    #define HDMI_SCDC_DBG(fmt, args...)       rtd_pr_hdmi_debug("[SCDC][DBG]"fmt, ##args)
#else
    #define HDMI_SCDC_DBG(fmt, args...)
#endif

#define HDMI_SCDC_INFO(fmt, args...)          rtd_pr_hdmi_info("[SCDC][Info]"fmt, ##args)       
#define HDMI_SCDC_WARN(fmt, args...)          rtd_pr_hdmi_warn("[SCDC][WARN]"fmt, ##args) 

#endif //__HDMI_SCDC_H__
