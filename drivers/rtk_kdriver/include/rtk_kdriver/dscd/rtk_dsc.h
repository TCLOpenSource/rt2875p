#ifndef __RTK_DSC_H__
#define __RTK_DSC_H__

#include <rtk_kdriver/measure/rtk_measure.h>
///////////////////////////////////////////////////////////////////////////////////////
#define RTK_DSC_PPS_LEN         128
#define EM_DSCD_CVTEM_INFO_LEN               192


typedef enum {
    //if Timing_monitor_sta0 == 14 ,Timing_monitor_end0 must be 7;if Timing_monitor_sta0== 15,Timing_monitor_end0 must be 3
    RTK_DSCD_TM_VS_RISING = 0,
    RTK_DSCD_TM_VS_FALLING,
    RTK_DSCD_TM_VACT_RISING,
    RTK_DSCD_TM_VACT_FALLING,
    RTK_DSCD_TM_HS_RISING,
    RTK_DSCD_TM_HS_FALLING,
    RTK_DSCD_TM_HACT_RISING,
    RTK_DSCD_TM_HACT_FALLING,
    RTK_DSCD_TM_DEN_RISING,
    RTK_DSCD_TM_DEN_FALLING,
    RTK_DSCD_TM_SPECIAL_SIGNAL_RISING,
    RTK_DSCD_TM_SPECIAL_SIGNAL_FALLING,
    RTK_DSCD_TM_DEN_LENGTH = 14,
    RTK_DSCD_TM_DEN_SUMMARY
} RTK_DSCD_TIMING_MONITOR_STA_END_TYPE;

typedef enum {
    RTK_DSCD_TM_RESULT_1,
    RTK_DSCD_TM_RESULT_2,
    RTK_DSCD_TM_RESULT_3,
    RTK_DSCD_TM_RESULT_4,
    RTK_DSCD_TM_RESULT_MAX,
    RTK_DSCD_TM_RESULT_MIN,
    RTK_DSCD_TM_RESULT_NUM
} RTK_DSCD_TIMING_MONITOR_RESULT_INDEX;

typedef enum {
    RTK_DSCD_FSM_INIT,
    RTK_DSCD_FSM_SET_FW_PPS,
    RTK_DSCD_FSM_WAIT_PPS_READY,
    RTK_DSCD_FSM_PPS_MEASURE,
    RTK_DSCD_FSM_RUN,
} RTK_DSCD_FSM_T;

typedef struct {
    // PPS0
    unsigned char   dsc_version_minor: 4; // [3:0]
    unsigned char   dsc_version_major: 4; // [7:4]

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
    unsigned short  reserved_16: 6;
    unsigned short  initial_xmit_delay: 10;

    // PPS18-19
    unsigned short  initial_dec_delay;

    // PPS20-21
    unsigned short  reserved_20: 10;
    unsigned short  initial_scale_value: 6;

    // PPS22-23
    unsigned short  scale_increment_interval;

    // PPS24-25
    unsigned short  reserved_24: 4;
    unsigned short  scale_decrement_interval: 12;

    // PPS26-27
    unsigned short  reserved_26: 11;
    unsigned short  first_line_bpg_offset: 5;

    // PPS28-29
    unsigned short  nfl_bpg_offset;

    // PPS30-31
    unsigned short  slice_bpg_offset;

    // PPS32-33
    unsigned short  initial_offset;

    // PPS34-35
    unsigned short  final_offset;

    // PPS36
    unsigned char  reserved_36: 3;
    unsigned char  flatness_min_qp: 5;

    // PPS37
    unsigned char  flatness_max_qp: 5;
    unsigned char  reserved_37: 3;

    // PPS38 - 87
    unsigned char   rc_parameter_set[50];

    // PPS88
    unsigned char   native_422: 1;
    unsigned char   native_420: 1;
    unsigned char   reserved_88: 6;

    // PPS89
    unsigned char   second_line_bpg_offset: 5;
    unsigned char   reserved_89: 3;

    // PPS90~91
    unsigned short  nsl_bpg_offset;

    // PPS92~93
    unsigned short  second_line_offset_adj;

    // PPS94~127
    unsigned char   reserved_94_127[34];
} RTK_DSC_PPS;

typedef struct {
    RTK_DSCD_FSM_T current_dscd_fsm;
    RTK_DSCD_FSM_T pre_dscd_fsm;
    unsigned char run_dscd_port;
    unsigned int last_pps_crc;
    unsigned int wait_pps_cnt;
    MEASURE_TIMING_T pps_timing;
} RTK_DSCD_FSM_STATUS;
#if 1
typedef struct {
    unsigned char   pps[128];
    unsigned short  hfront;
    unsigned short  hsync;
    unsigned short  hback;
    unsigned short  hcative_bytes;
} DSCD_CVTEM_INFO; // from hdmi_emp.h CVTEM_INFO


typedef struct {
    unsigned char   type;
    unsigned char   reserved: 6;
    unsigned char   last: 1;
    unsigned char   first: 1;
    unsigned char   seq_idx;
    unsigned char   pb[28];
    unsigned char   reserved_2; // for 32 bytes alignment
} DSCD_EMP_PKT; // from hdmi_emp.h EMP_PKT
#endif

extern RTK_DSCD_FSM_STATUS rtk_dscd_fsm_st;
extern DSCD_CVTEM_INFO dscd_cvtem_info[6];


#define RTK_DSCD_OUTPUT_HPORCH_MIN    80

#if IS_ENABLED(CONFIG_RTK_DSCD) || IS_ENABLED(CONFIG_RTK_DSCD_MODULE)
    ///////////////////////////////////////////////////////////////////////////////////////
    void dsc_crt_reset(void);
    void rtk_dsc_double_buffer_enable(unsigned char enable);
    void rtk_dsc_fw_mode_enable(unsigned char enable);
    void rtk_dsc_set_fw_pps(unsigned char pps[RTK_DSC_PPS_LEN]);
    void rtk_dsc_get_cur_pps(unsigned char pps[RTK_DSC_PPS_LEN]);
    extern void rtk_dsc_enable(unsigned char enable);
    void rtk_dsc_set_crc_en(unsigned char enable);
    unsigned int rtk_dsc_get_dscd_out_crc(void);
    unsigned int rtk_dsc_get_dscd_in_crc(void);
    unsigned int rtk_dsc_get_dscd_pps_crc(void);
    unsigned char rtk_dsc_get_crt_on(void);

    unsigned char rtk_dsc_get_output_error_status(void);
    void rtk_dsc_clr_output_error_status(void);
    void rtk_dsc_clr_dscd_out_pic_finish(void);
    unsigned char rtk_dsc_get_dscd_out_pic_finish(void);
    unsigned char rtk_dsc_get_dscd_p_n_s_not_finish(void);
    void rtk_dsc_clr_dscd_p_n_s_not_finish(void);
    void rtk_dsc_set_output_hporch(unsigned int value);
    unsigned char dscd_get_dsc_enable(void);
    unsigned int rtk_dsc_get_timing_monitor_result(RTK_DSCD_TIMING_MONITOR_RESULT_INDEX result_index);

    //Timing Monitor
    void rtk_dscd_timing_monitor_en(unsigned char enable);
    unsigned char rtk_dscd_is_timing_monitor_en(void);
    void rtk_dsc_set_timing_monitor_sta0(RTK_DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type);
    void rtk_dsc_set_timing_monitor_end0(RTK_DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type);
    void rtk_dsc_set_timing_monitor_sta1(RTK_DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type);
    void rtk_dsc_set_timing_monitor_end1(RTK_DSCD_TIMING_MONITOR_STA_END_TYPE sta_end_type);
    unsigned int rtk_dsc_get_timing_monitor_result(RTK_DSCD_TIMING_MONITOR_RESULT_INDEX result_index);
    void rtk_dsc_clr_irq_status(void);
    unsigned int rtk_dsc_get_dscd_irq_status(void);
    MEASURE_TIMING_T *dscd_get_current_pps_timing(void);
    unsigned char dscd_pps_measure(unsigned char port, MEASURE_TIMING_T *tm, DSCD_CVTEM_INFO *info);

    ///////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////
    extern void rtk_dscd_parse_dsc_pps(unsigned char pps[RTK_DSC_PPS_LEN], RTK_DSC_PPS *p_pps);
    extern void rtk_dscd_dump_dsc_pps(RTK_DSC_PPS *p_pps);
    extern unsigned char rtk_check_dscd_output_valid(void);
    extern HDMI_COLOR_DEPTH_T newbase_dsc_get_colordepth(void);
    extern const char *rtk_dsc_fsm_str(RTK_DSCD_FSM_T fsm);
    extern unsigned int rtk_dscd_get_current_output_vbackporch(unsigned int is_clk_div_en);
    extern void rtk_dsc_handler(unsigned char port, unsigned char is_hdmi, HDMI_COLOR_SPACE_T color_space);

    #define GET_DSCD_FSM() (rtk_dscd_fsm_st.current_dscd_fsm)
    #define SET_DSCD_FSM(fsm)   do { if (rtk_dscd_fsm_st.current_dscd_fsm !=fsm) { rtd_pr_dscd_warn("[FSM][DSCD] DSCD_FSM=%d(%s) -> %d(%s)\n", rtk_dscd_fsm_st.current_dscd_fsm, rtk_dsc_fsm_str(rtk_dscd_fsm_st.current_dscd_fsm), fsm, rtk_dsc_fsm_str(fsm)); rtk_dscd_fsm_st.current_dscd_fsm = fsm;  }  }while(0)

    #define GET_DSCD_RUN_PORT() (rtk_dscd_fsm_st.run_dscd_port)

#else
    #define dsc_crt_reset()
    #define rtk_dsc_double_buffer_enable(...)
    #define rtk_dsc_fw_mode_enable(enable)
    #define rtk_dsc_set_fw_pps(...)
    #define rtk_dsc_get_cur_pps(...)
    #define rtk_dsc_enable(enable)
    #define rtk_dsc_set_crc_en(enable)
    #define rtk_dsc_get_dscd_out_crc()    (0)
    #define rtk_dsc_get_dscd_in_crc()    (0)
    #define rtk_dsc_get_dscd_pps_crc()    (0)
    #define rtk_dsc_get_crt_on()   (0)
    #define rtk_dscd_timing_monitor_en(enable)
    #define rtk_dscd_is_timing_monitor_en()    (0)
    #define rtk_dsc_set_timing_monitor_sta0(...)
    #define rtk_dsc_set_timing_monitor_end0(...)
    #define rtk_dsc_set_timing_monitor_sta1(...)
    #define rtk_dsc_set_timing_monitor_end1(...)
    #define rtk_dsc_get_timing_monitor_result(...)    (0)

    #define newbase_dscd_tm_vbackporch(...)  (0)
    #define rtk_dsc_handler(port, is_hdmi)
    #define rtk_check_dscd_output_valid()
    #define dscd_get_current_pps_timing() (NULL)
    #define newbase_dsc_get_colordepth() (0)

    #define GET_DSCD_FSM() (0)
    #define SET_DSCD_FSM(fsm)   (0)

    #define GET_DSCD_RUN_PORT() (0xF)

#endif

#endif //__RTK_DSC_H__
