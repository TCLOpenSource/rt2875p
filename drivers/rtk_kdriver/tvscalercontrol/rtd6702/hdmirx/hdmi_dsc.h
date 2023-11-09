#ifndef __HDMI_DSC_H__
#define __HDMI_DSC_H__

#include "hdmi_common.h"

///////////////////////////////////////////////////////////////////////////////////////
#define DSC_PPS_LEN         128


typedef enum
{//if Timing_monitor_sta0 == 14 ,Timing_monitor_end0 must be 7;if Timing_monitor_sta0== 15,Timing_monitor_end0 must be 3
    DSCD_TM_VS_RISING = 0,
    DSCD_TM_VS_FALLING,
    DSCD_TM_VACT_RISING,
    DSCD_TM_VACT_FALLING,
    DSCD_TM_HS_RISING,
    DSCD_TM_HS_FALLING,
    DSCD_TM_HACT_RISING,
    DSCD_TM_HACT_FALLING,
    DSCD_TM_DEN_RISING,
    DSCD_TM_DEN_FALLING,
    DSCD_TM_SPECIAL_SIGNAL_RISING,
    DSCD_TM_SPECIAL_SIGNAL_FALLING,
    DSCD_TM_DEN_LENGTH = 14,
    DSCD_TM_DEN_SUMMARY
}DSCD_TIMING_MONITOR_STA_END_TYPE;

typedef enum
{
    DSCD_TM_RESULT_1,
    DSCD_TM_RESULT_2,
    DSCD_TM_RESULT_3,
    DSCD_TM_RESULT_4,
    DSCD_TM_RESULT_MAX,
    DSCD_TM_RESULT_MIN,
    DSCD_TM_RESULT_NUM
}DSCD_TIMING_MONITOR_RESULT_INDEX;

typedef enum {
    DSCD_FSM_WAIT_LT_PASS = 0,
    DSCD_FSM_INIT,
    DSCD_FSM_WAIT_CVTEM,
    DSCD_FSM_SET_FW_PPS,
    DSCD_FSM_WAIT_PPS_READY,
    DSCD_FSM_PPS_MEASURE,
    DSCD_FSM_RUN,
} HDMI_DSCD_FSM_T;

typedef struct 
{
    // PPS0
    unsigned char   dsc_version_minor:4;  // [3:0]
    unsigned char   dsc_version_major:4;  // [7:4]
    
    // PPS1
    unsigned char   pps_identifier;
    
    // PPS2
    unsigned char   reserved;
    
    // PPS3    
    unsigned char   line_buffer_depth  : 4;  // [3:0]
    unsigned char   bits_per_component : 4;  // [7:4]
    
    // PPS4~5
    unsigned char   reserved2 : 2;
    unsigned short  block_pred_enable : 1;      
    unsigned short  convert_rgb  : 1;      
    unsigned short  simple_422  : 1;      
    unsigned short  vbr_enable  : 1;      
    unsigned short  bits_per_pixel  : 10;  
    
    // PPS6~7
    unsigned short  pic_height;
    
    // PPS8~9
    unsigned short  pic_width;

    // PPS10-11
    unsigned short  slice_height;

    // PPS12-13
    unsigned short  slice_width;
    
    // PPS14-15
    unsigned short  chunk_size;
    
    // PPS16-17
    unsigned short  reserved_16:6;
    unsigned short  initial_xmit_delay:10; 

    // PPS18-19
    unsigned short  initial_dec_delay;
    
    // PPS20-21
    unsigned short  reserved_20:10;    
    unsigned short  initial_scale_value:6;
    
    // PPS22-23
    unsigned short  scale_increment_interval;
    
    // PPS24-25
    unsigned short  reserved_24:4;    
    unsigned short  scale_decrement_interval:12;    
        
    // PPS26-27
    unsigned short  reserved_26:11;    
    unsigned short  first_line_bpg_offset:5;  
        
    // PPS28-29
    unsigned short  nfl_bpg_offset;    
    
    // PPS30-31
    unsigned short  slice_bpg_offset;          
    
    // PPS32-33
    unsigned short  initial_offset;
    
    // PPS34-35
    unsigned short  final_offset;
    
    // PPS36
    unsigned char  reserved_36:3;    
    unsigned char  flatness_min_qp:5;    
        
    // PPS37        
    unsigned char  flatness_max_qp:5; 
    unsigned char  reserved_37:3;         
        
    // PPS38 - 87
    unsigned char   rc_parameter_set[50];  
    
    // PPS88
    unsigned char   native_422:1;
    unsigned char   native_420:1;    
    unsigned char   reserved_88:6;
    
    // PPS89
    unsigned char   second_line_bpg_offset:5;    
    unsigned char   reserved_89:3;
            
    // PPS90~91
    unsigned short  nsl_bpg_offset;        

    // PPS92~93
    unsigned short  second_line_offset_adj;    
                        
    // PPS94~127
    unsigned char   reserved_94_127[34];
}DSC_PPS;

typedef struct
{
    HDMI_DSCD_FSM_T current_dscd_fsm;
    HDMI_DSCD_FSM_T pre_dscd_fsm;
    unsigned char run_dscd_port;
    unsigned int last_pps_crc;
    unsigned int wait_pps_cnt;
    HDMI_TIMING_T pps_timing;
}DSCD_FSM_STATUS;
extern DSCD_FSM_STATUS dscd_fsm_st;

#define DSCD_OUTPUT_HPORCH_MIN    80

#ifdef CONFIG_RTK_KDRV_HDMI_ENABLE_HDMI21

///////////////////////////////////////////////////////////////////////////////////////
extern void _parse_dsc_pps(unsigned char pps[DSC_PPS_LEN], DSC_PPS* p_pps);
extern void _dump_dsc_pps(DSC_PPS* p_pps);

///////////////////////////////////////////////////////////////////////////////////////
extern void lib_hdmi_dsc_crt_reset(unsigned char dscd_port);
extern void lib_hdmi_dsc_double_buffer_enable(unsigned char dscd_port, unsigned char enable);
extern void lib_hdmi_dsc_fw_mode_enable(unsigned char dscd_port, unsigned char enable);
extern void lib_hdmi_dsc_set_fw_pps(unsigned char dscd_port, unsigned char pps[DSC_PPS_LEN]);
extern void lib_hdmi_dsc_get_cur_pps(unsigned char dscd_port, unsigned char pps[DSC_PPS_LEN]);
extern unsigned char lib_hdmi_dsc_get_dscd_p_n_s_not_finish(unsigned char dscd_port);
extern void lib_hdmi_dsc_clr_dscd_p_n_s_not_finish(unsigned char dscd_port);
extern void lib_hdmi_dsc_enable(unsigned char dscd_port, unsigned char enable);
extern unsigned char lib_hdmi_get_dsc_enable(unsigned char dscd_port);
extern unsigned char lib_hdmi_dsc_get_dscd_out_pic_finish(unsigned char dscd_port);
extern void lib_hdmi_dsc_clr_dscd_out_pic_finish(unsigned char dscd_port);
extern void lib_hdmi_dsc_set_crc_en(unsigned char dscd_port, unsigned char enable);
extern unsigned int lib_hdmi_dsc_get_dscd_out_crc(unsigned char dscd_port);
extern unsigned int lib_hdmi_dsc_get_dscd_in_crc(unsigned char dscd_port);
extern unsigned int lib_hdmi_dsc_get_dscd_pps_crc(unsigned char dscd_port);
extern unsigned char lib_hdmi_dsc_get_crt_on(unsigned char dscd_port);
extern void lib_hdmi_dsc_set_output_hporch(unsigned char dscd_port, unsigned int value);
extern unsigned char lib_hdmi_dsc_get_output_error_status(unsigned char dscd_port);
extern void lib_hdmi_dsc_clr_output_error_status(unsigned char dscd_port);

//Timing Monitor
extern void lib_hdmi_dscd_timing_monitor_en(unsigned char dscd_port, unsigned char enable);
extern unsigned char lib_hdmi_dscd_is_timing_monitor_en(unsigned char dscd_port);
extern void lib_hdmi_dsc_set_timing_monitor_sta0(unsigned char dscd_port, DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type);
extern void lib_hdmi_dsc_set_timing_monitor_end0(unsigned char dscd_port, DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type);
extern void lib_hdmi_dsc_set_timing_monitor_sta1(unsigned char dscd_port, DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type);
extern void lib_hdmi_dsc_set_timing_monitor_end1(unsigned char dscd_port, DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type);
extern void lib_hdmi_dsc_set_timing_monitor_sta2(unsigned char dscd_port, DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type);
extern void lib_hdmi_dsc_set_timing_monitor_end2(unsigned char dscd_port, DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type);
extern unsigned int lib_hdmi_dsc_get_timing_monitor_result(unsigned char dscd_port, DSCD_TIMING_MONITOR_RESULT_INDEX result_index);


///////////////////////////////////////////////////////////////////////////////////////
extern int newbase_hdmi_hd21_dsc_get_dscd_port(unsigned char port, unsigned char* p_dscd_port);
extern void newbase_hdmi_hd21_dsc_enanble(unsigned char port, unsigned char enable);
extern unsigned int newbase_hdmi_dscd_current_output_vbackporch(void);
extern void newbase_hdmi_hd21_dsc_handler(unsigned char port);
extern unsigned char newbase_hdmi_check_dscd_output_valid(void);
extern HDMI_TIMING_T* newbase_hdmi_hd21_dsc_get_current_pps_timing(void);
extern HDMI_COLOR_DEPTH_T newbase_hdmi_hd21_dsc_get_colordepth(void);
extern const char* _hdmi_hd21_dsc_fsm_str(HDMI_DSCD_FSM_T fsm);

#define GET_DSCD_FSM() (dscd_fsm_st.current_dscd_fsm)
#define SET_DSCD_FSM(fsm)	do { if (dscd_fsm_st.current_dscd_fsm !=fsm) { rtd_pr_hdmi_warn("[FSM][DSCD] DSCD_FSM=%d(%s) -> %d(%s)\n", dscd_fsm_st.current_dscd_fsm, _hdmi_hd21_dsc_fsm_str(dscd_fsm_st.current_dscd_fsm), fsm, _hdmi_hd21_dsc_fsm_str(fsm)); dscd_fsm_st.current_dscd_fsm = fsm;  }  }while(0)

#define GET_DSCD_RUN_PORT() (dscd_fsm_st.run_dscd_port)

#else
#define lib_hdmi_dsc_crt_reset()
#define lib_hdmi_dsc_double_buffer_enable(...)
#define lib_hdmi_dsc_fw_mode_enable(enable)
#define lib_hdmi_dsc_set_fw_pps(...)
#define lib_hdmi_dsc_get_cur_pps(...)
#define lib_hdmi_dsc_enable(enable)
#define lib_hdmi_dsc_set_crc_en(enable)
#define lib_hdmi_dsc_get_dscd_out_crc()    (0)
#define lib_hdmi_dsc_get_dscd_in_crc()    (0)
#define lib_hdmi_dsc_get_dscd_pps_crc()    (0)
#define lib_hdmi_dsc_get_crt_on()   (0)
#define lib_hdmi_dscd_timing_monitor_en(enable)
#define lib_hdmi_dscd_is_timing_monitor_en()    (0)
#define lib_hdmi_dsc_set_timing_monitor_sta0(...)
#define lib_hdmi_dsc_set_timing_monitor_end0(...)
#define lib_hdmi_dsc_set_timing_monitor_sta1(...)
#define lib_hdmi_dsc_set_timing_monitor_end1(...)
#define lib_hdmi_dsc_get_timing_monitor_result(...)    (0)

#define newbase_hdmi_hd21_dsc_enanble(...)  (0)
#define newbase_hdmi_dscd_tm_vbackporch(...)  (0)
#define newbase_hdmi_hd21_dsc_handler(port)
#define newbase_hdmi_check_dscd_output_valid()
#define newbase_hdmi_hd21_dsc_get_current_pps_timing() (NULL)
#define newbase_hdmi_hd21_dsc_get_colordepth() (0)

#define GET_DSCD_FSM() (0)
#define SET_DSCD_FSM(fsm)	(0)

#define GET_DSCD_RUN_PORT() (0xF)

#endif

///////////////////////////////////////////////////////////////////////////////////////

//#define HDMI_DSC_DEBUG_EN
#ifdef  HDMI_DSC_DEBUG_EN
#define HDMI_DSC_DBG(fmt, args...)      rtd_pr_hdmi_info("[DSC][DBG]" fmt, ##args)
#else
#define HDMI_DSC_DBG(fmt, args...)
#endif

#define HDMI_DSC_INFO(fmt, args...)     rtd_pr_hdmi_info("[DSC][INFO]" fmt, ##args)
#define HDMI_DSC_ERR(fmt, args...)      rtd_pr_hdmi_err("[DSC][WARN]" fmt, ##args)

#endif //__HDMI_DSC_H__
