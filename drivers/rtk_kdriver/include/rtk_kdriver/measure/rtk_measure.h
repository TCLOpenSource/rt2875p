#ifndef RTK_MEASURE_H
#define RTK_MEASURE_H

typedef enum
{
    OFFMS_ONE_SHOT = 0,
    OFFMS_CONTINUOUS_MODE = 1
}OFFMS_MSMODE;

typedef enum {
    HDMI_SMT_PATH_MAIN_VGIP = 0,
    HDMI_SMT_PATH_SUB_VGIP,
    HDMI_SMT_PATH_DMA_VGIP,
    HDMI_SMT_PATH_UNKNOW,
} HDMI_SMT_PATH;

typedef enum {
    MODE_TMDS,
    MODE_FRL_3G_3_LANE,
    MODE_FRL_6G_3_LANE,
    MODE_FRL_6G_4_LANE,
    MODE_FRL_8G_4_LANE,
    MODE_FRL_10G_4_LANE,
    MODE_FRL_12G_4_LANE
} HDMI_FRL_MODE_T;

typedef enum {
    MODE_DVI    = 0x0,    // DVI mode
    MODE_HDMI   = 0x1,    // HDMI 1.4/2.0
    MODE_HDMI21 = 0x2,    // HDMI 2.1
    MODE_UNKNOW = 0x3,
} HDMI_DVI_MODE_T;

typedef enum {
    HDMI_COLORIMETRY_NOSPECIFIED = 0,
    HDMI_COLORIMETRY_601,
    HDMI_COLORIMETRY_709,
    HDMI_COLORIMETRY_XYYCC601,
    HDMI_COLORIMETRY_XYYCC709,
    HDMI_COLORIMETRY_SYCC601,
    HDMI_COLORIMETRY_ADOBE_YCC601,
    HDMI_COLORIMETRY_ADOBE_RGB,
    HDMI_COLORIMETRY_BT2020_YCC,
    HDMI_COMORIMETRY_BT2020_RGB_OR_YCC,
    HDMI_COMORIMETRY_ST_2113_P3D65,
    HDMI_COMORIMETRY_ST_2113_P3DCI,
    HDMI_COMORIMETRY_BT_2100
} HDMI_COLORIMETRY_T;

typedef enum {
    COLOR_RGB   = 0x00,
    COLOR_YUV422,
    COLOR_YUV444,
    COLOR_YUV420,
    COLOR_UNKNOW
} HDMI_COLOR_SPACE_T;

typedef enum {

    HDMI_COLOR_DEPTH_8B = 0,
    HDMI_COLOR_DEPTH_10B,
    HDMI_COLOR_DEPTH_12B,
    HDMI_COLOR_DEPTH_16B,

} HDMI_COLOR_DEPTH_T;

typedef enum {
    HVF_NO,
    HVF_EXT,
    HVF_3D,
    HVF_RESERVED
} HDMI_HVF_E;

typedef enum {

    HDMI3D_FRAME_PACKING = 0,
    HDMI3D_FIELD_ALTERNATIVE = 1,
    HDMI3D_LINE_ALTERNATIVE = 2,
    HDMI3D_SIDE_BY_SIDE_FULL = 3,
    HDMI3D_L_DEPTH = 4,
    HDMI3D_L_DEPTH_GPX = 5,
    HDMI3D_TOP_AND_BOTTOM = 6,
    HDMI3D_FRAMESEQUENCE = 7,
    HDMI3D_SIDE_BY_SIDE_HALF = 8,
    // --- new 3D format ---
    HDMI3D_LINE_BY_LINE = 9,
    HDMI3D_VERTICAL_STRIPE = 10,
    HDMI3D_CHECKER_BOARD = 11,
    HDMI3D_REALID = 12,
    HDMI3D_SENSIO = 13,
    // -------------------
    HDMI3D_RSV1,
    HDMI3D_2D_ONLY,
    HDMI3D_UNKOWN = 0xFFFFFFFF
} HDMI_3D_T;

typedef struct {
    unsigned int h_act;
    unsigned int v_act;
    unsigned int h_tol;
    unsigned int v_tol;
} TIMING_TABLE_T;

typedef enum {
    ONE_PIXEL_MODE = 1,
    TWO_PIXEL_MODE = 2,
    FOUR_PIXEL_MODE = 4,
    PIXEL_MODE_RESERVED
} PIXEL_MODE_E;


typedef struct {

    unsigned int v_total;
    unsigned int h_total;
    unsigned int v_act_len;
    unsigned int h_act_len;
    unsigned int v_act_sta;
    unsigned int h_act_sta;
    unsigned int v_freq;
    unsigned int h_freq;
    unsigned int polarity;
    unsigned int mode_id;
    unsigned int modetable_index;
    unsigned char is_interlace;
    unsigned int IVSyncPulseCount;  //Input VSYNC High Period Measurement Result
    unsigned int IHSyncPulseCount;  //Input HSYNC High Period Measurement Result

    HDMI_DVI_MODE_T     mode;
    HDMI_COLOR_DEPTH_T colordepth;
    HDMI_COLOR_SPACE_T colorspace;
    HDMI_COLORIMETRY_T  colorimetry;
    unsigned char pixel_repeat;

    HDMI_HVF_E hvf;
    HDMI_3D_T h3dformat;

    unsigned int tmds_clk_b;
    unsigned long pll_pixelclockx1024;

    // only for fram packing
    unsigned int v_active_space1;
    unsigned int v_active_space2;
    unsigned char avi_vic;
    unsigned char run_vrr;
    unsigned char run_dsc;    // 0: do not run dsc flow, 1: run dsc flow
    unsigned char fva_factor;    // 0: fva disabled, > 0: fva multiplier - 1
    PIXEL_MODE_E pixel_mode; // 1: 1pixel , 2: 2pixel, 4:4pixel
    unsigned char is_v_drop; // 1: v drop, 0:not v drop
} MEASURE_TIMING_T;

typedef enum{
	MEASURE_ONMS1_MAIN,
	MEASURE_ONMS2_SUB,
	MEASURE_ONMS3_DMA,
	MEASURE_ONMS3_NUM
}MEASURE_ONMS_CHANNEL;

typedef enum {
    OFFMS_SOURCE_HDMI = 0x0,
    OFFMS_SOURCE_HDMI21 = 0x1,
    OFFMS_SOURCE_PTG    = 0x2,
    OFFMS_SOURCE_DPRX   = 0x3,
    OFFMS_SOURCE_DSCD   = 0x4
} OFFMS_SOURCE_SEL;

#define NEW_OFFMS_API_SUPPORT

typedef struct
{
    OFFMS_SOURCE_SEL src;

    // color info
    unsigned char yuv420   : 1;

    // pixel mode
    unsigned char pixel_mode  : 4;

    #define OFFMS_1_PIXEL_MODE  1
    #define OFFMS_2_PIXEL_MODE  2
    #define OFFMS_4_PIXEL_MODE  4

    unsigned char reserved    : 3;

    // src specifiec info
    union
    {
        struct
        {
            unsigned char frl_mode;
        }hdmi;
    }src_info;

}OFFMS_CTRL;


// OFFMS_CTRL_FALGS : to provide detail info to offms
#define OFFMS_CTRL_FALGS        unsigned long

// b0: HDMI_FRL mode (0: TMDS mode, 1: HDMI_FRL mode), only valid when SOURCE is HDMI
#define OFFMS_FLAGS_HDMI_FRL_MODE(x)    (x & 0x1)
#define OFFMS_FLAGS_IS_HDMI_FRL_MODE(x) (x & 0x1)

// b1: IS_YUV420
#define OFFMS_FLAGS_YUV420_MODE(x)      ((x & 0x1)<<1)
#define OFFMS_FLAGS_IS_YUV420_MODE(x)   ((x >>1) & 0x1)

// b2: IS_DEMOD
#define OFFMS_FLAGS_DEMODE(x)           ((x & 0x1)<<2)
#define OFFMS_FLAGS_IS_DEMODE(x)        ((x >>2) & 0x1)

// b7:4: Pixel mode info
#define OFFMS_FLAGS_PIXEL_MODE(x)       ((x & 0xF)<<4)
#define OFFMS_FLAGS_GET_PIXEL_MODE(x)   ((x>>4) & 0xF)
#define OFFMS_1_PIXEL_MODE      1
#define OFFMS_2_PIXEL_MODE      2
#define OFFMS_4_PIXEL_MODE      4

typedef enum {
    //-------------Measure success code-------------
    MEASURE_SUCCESS_NORMAL_BY_HV = 0x00,
    MEASURE_SUCCESS_VRR_BY_VIC,
    MEASURE_SUCCESS_VRR_BY_OFMS,
    MEASURE_SUCCESS_NO_H_WITH_VIC,
    MEASURE_SUCCESS_NO_H_FIND_TABLE_BY_DE,
    MEASURE_SUCCESS_NUM = 0x0F,

    //-------------Measure fail code--------------
    MEASURE_FAIL_OFMS_NOT_FINISH = 0x10,
    MEASURE_FAIL_HACT_OVER_HTOTAL,
    MEASURE_FAIL_VACT_OVER_VTOTAL,
    MEASURE_FAIL_HACT_UNDER_MIN,
    MEASURE_FAIL_VACT_UNDER_MIN,
    MEASURE_FAIL_IS_SKIP_TIMING,
    MEASURE_FAIL_HTOTAL_DIFF,
    MEASURE_FAIL_VTOTAL_DIFF,
    MEASURE_FAIL_HACT_DIFF,
    MEASURE_FAIL_VACT_DIFF,
    MEASURE_FAIL_HFREQ_DIFF,
    MEASURE_FAIL_VFREQ_DIFF,
    MEASURE_FAIL_SEARCH_VIC_TABLE,
    MEASURE_FAIL_CORRECTION_BY_VIC,
} MEASURE_RESULT_CODE;


#if defined(CONFIG_RTK_MEASURE) || defined(CONFIG_RTK_MEASURE_MODULE)
    void newbase_measure_init(void);
    unsigned char lib_measure_calc_freq(MEASURE_TIMING_T *tm);
    extern unsigned int lib_measure_vfreq_prescion_3_to_1(unsigned int original_vfreq);

    extern void set_no_polarity_inverse(unsigned char TorF);//allow vgip to set h v polarity inverse or not
    extern unsigned char lib_hdmi_dp_measure(OFFMS_SOURCE_SEL sel, MEASURE_TIMING_T *tm, unsigned char frl_mode,unsigned char is_HV_Mode);
    extern unsigned char lib_hdmims_get_hvstart(MEASURE_TIMING_T *tm, unsigned char vsc_ch);
    extern unsigned char lib_measure_active_correction(unsigned int *h_active, unsigned int *v_active);
    extern void newbase_hdmi_dp_measure_abort(void);

    extern unsigned char lib_offline_measure(OFFMS_SOURCE_SEL sel, MEASURE_TIMING_T *tm, unsigned char frl_mode); // legacy API
    extern unsigned char lib_offline_measure_ex(OFFMS_SOURCE_SEL sel, MEASURE_TIMING_T *tm, unsigned char frl_mode, OFFMS_MSMODE ms_mode);
    extern unsigned char lib_offline_measure_demode(OFFMS_SOURCE_SEL sel, MEASURE_TIMING_T *tm, unsigned char frl_mode);
    extern unsigned char lib_offline_measure_native(OFFMS_SOURCE_SEL sel, MEASURE_TIMING_T *tm, unsigned char frl_mode);
    extern unsigned char lib_online1_get_ms_src_sel(void);
    extern void lib_online1_set_ms_src_sel(unsigned char ms_src_sel);
    extern unsigned char lib_online1_measure(MEASURE_TIMING_T *tm);
    extern unsigned char lib_online3_get_ms_src_sel(void);
    extern void lib_online3_set_ms_src_sel(unsigned char ms_src_sel);
    extern unsigned char lib_online3_measure(MEASURE_TIMING_T *tm);
    extern unsigned char lib_online3_demode_measure(MEASURE_TIMING_T *tm);
    extern void lib_measure_onms_set_watchdog_int_en(MEASURE_ONMS_CHANNEL onms_channel, unsigned int value);
    extern unsigned int lib_measure_onms_get_watchdog_en(MEASURE_ONMS_CHANNEL onms_channel);
    extern unsigned int lib_measure_onms_get_int_en(MEASURE_ONMS_CHANNEL onms_channel);
    extern unsigned int lib_measure_calc_vfreq(unsigned int vs_period);
    extern unsigned int lib_measure_get_vs_period(void);
    extern unsigned char newbase_hdmi_dp_measure_video(OFFMS_SOURCE_SEL sel, unsigned char port, MEASURE_TIMING_T *tm, unsigned char frl_mode);  // Legacy API
    extern unsigned char newbase_hdmi_dp_measure_video_ex(OFFMS_SOURCE_SEL sel, unsigned char port, MEASURE_TIMING_T *tm, unsigned char frl_mode,unsigned char is_HV_Mode);

    // new API
    extern unsigned char rtk_offms_measure_video(OFFMS_SOURCE_SEL sel, MEASURE_TIMING_T *tm, OFFMS_CTRL_FALGS flags);
#else

    #define newbase_measure_init()
    #define lib_measure_calc_freq(tm)                               0
    #define lib_measure_vfreq_prescion_3_to_1(original_vfreq)       0
    #define set_no_polarity_inverse(TorF)
    #define lib_hdmi_dp_measure(sel, tm, frl_mode)                  0
    #define lib_hdmi_dp_measure_ex(sel, tm, frl_mode,is_HV_Mode)    0
    #define lib_hdmims_get_hvstart(tm, vsc_ch)                      0
    #define lib_measure_active_correction(h_active, v_active)       0
    #define newbase_hdmi_dp_measure_abort()
    #define lib_offline_measure(sel, tm, frl_mode)                  0
    #define lib_offline_measure_demode(sel, tm, frl_mode)           0
    #define lib_offline_measure_native(sel, tm, frl_mode)           0
    #define lib_online1_get_ms_src_sel()                            0
    #define lib_online1_set_ms_src_sel(ms_src_sel)
    #define lib_online1_measure(tm)                                 0
    #define lib_online3_get_ms_src_sel()                            0
    #define lib_online3_set_ms_src_sel(ms_src_sel)
    #define lib_online3_measure(tm)                                 0
    #define lib_online3_demode_measure(tm)                          0
    #define lib_measure_onms_set_watchdog_int_en(onms_channel, value)
    #define lib_measure_onms_get_watchdog_en(onms_channel)          0
    #define lib_measure_onms_get_int_en(onms_channel)               0
    #define lib_measure_calc_vfreq(vs_period)                       0
    #define lib_measure_get_vs_period()                             0
    #define newbase_hdmi_dp_measure_video(sel,port,tm,frl_mode)     0
    #define newbase_hdmi_dp_measure_video_ex(sel,port,tm,frl_mode, is_HV_Mode)     0
    #define newbase_offms_measure_video(sel, tm, flags)         0
#endif
#endif
