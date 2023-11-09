#ifndef __RTK_PWM_REG_H__
#define __RTK_PWM_REG_H__

#ifndef BUILD_QUICK_SHOW
#include <linux/io.h>
#include <rtk_kdriver/io.h>
#else
#include <sysdefs.h>
#endif
#include <rbus/rbus_types.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/stb_reg.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/misc_pwm_reg.h>
#include <rbus/iso_misc_reg.h>
#include <rbus/iso_misc_pwm_reg.h>

//ISO_PWM_INIT
#define  PWM_ISO_INT_CTRL            ISO_MISC_INT_CTRL_reg

//SYS_CLCK
#define  PWM_SYS_CLKDIV              SYS_REG_SYS_CLKDIV_reg
#define  ISO_PWM_SYS_CLKDIV          STB_ST_CLKMUX_reg

#define  PWM_RESERVED_VADDR          (0xFFFFFFFF)

//DTG
#define DISPLAY_TIMING_CTRL1_VADDR   PPOVERLAY_Display_Timing_CTRL1_reg
#define PREDICT_CTRL_VADDR           PPOVERLAY_PWM_predict_ctrl_reg
#define FRC_TO_FRAME_SYNC_VADDR      PPOVERLAY_frc_to_frame_sync_den_end_ctrl_reg

//ISO
#define  ISO_PWM_DB_CTRL_VADDR       ISO_MISC_PWM_MIS_PWM_DB_CTRL_reg
#define  ISO_PWM0_CTRL_VADDR         ISO_MISC_PWM_MIS_PWM0_CTRL_reg
#define  ISO_PWM0_TIMING_CTRL_VADDR  ISO_MISC_PWM_MIS_PWM0_Timing_CTRL_reg
#define  ISO_PWM0_DUTY_SET_VADDR     ISO_MISC_PWM_MIS_PWM0_DUTY_SET_reg
#define  ISO_PWM0_ALIGN_VADDR        ISO_MISC_PWM_MIS_PWM0_ALIGN_reg
#define  ISO_PWM0_TRACKVALUE_VADDR   ISO_MISC_PWM_MIS_PWM0_TRACKVALUE_reg

#define  ISO_PWM1_CTRL_VADDR         ISO_MISC_PWM_MIS_PWM1_CTRL_reg
#define  ISO_PWM1_TIMING_CTRL_VADDR  ISO_MISC_PWM_MIS_PWM1_Timing_CTRL_reg
#define  ISO_PWM1_DUTY_SET_VADDR     ISO_MISC_PWM_MIS_PWM1_DUTY_SET_reg
#define  ISO_PWM1_ALIGN_VADDR        ISO_MISC_PWM_MIS_PWM1_ALIGN_reg
#define  ISO_PWM1_TRACKVALUE_VADDR   ISO_MISC_PWM_MIS_PWM1_TRACKVALUE_reg

#define  ISO_PWM_OD_MODE_VADDR       ISO_MISC_PWM_PWM_ODMODE_CR_reg
#define  ISO_PWM_NONE                (PWM_RESERVED_VADDR)

//MISC
#define  MIS_PWM_DB0_CTRL_VADDR                    MISC_PWM_PWM_DB0_CTRL_reg
#define  MIS_PWM_DB1_CTRL_VADDR                    MISC_PWM_PWM_DB1_CTRL_reg
#define  MIS_PWM0_CTRL_VADDR                       MISC_PWM_PWM0_CTRL_reg
#define  MIS_PWM0_TIMING_CTRL_VADDR                MISC_PWM_PWM0_Timing_CTRL_reg
#define  MIS_PWM0_DUTY_SET_VADDR                   MISC_PWM_PWM0_DUTY_SET_reg
#define  MIS_PWM0_DVS_PERIOD_VADDR                 MISC_PWM_PWM0_DVS_PERIOD_reg
#define  MIS_PWM0_DVS_MONITOR_VADDR                MISC_PWM_PWM0_DVS_MONITOR_reg
#define  MIS_PWM0_MONITOR_VADDR                    MISC_PWM_PWM0_MONITOR_reg
#define  MIS_PWM0_ERR_STATUS_VADDR                 MISC_PWM_PWM0_ERR_STATUS_reg
#define  MIS_PWM0_TRACK_SET_VADDR                  MISC_PWM_PWM0_TRACK_SET_reg
#define  MIS_PWM0_BL_CTR_VADDR                     MISC_PWM_PWM0_BL_CTR
#define  MIS_PWM0_BL_COMPENSATE_VADDR              MISC_PWM_PWM0_BL_COMPENSATE
#define  MIS_PWM0_BL_Thr1_VADDR                    MISC_PWM_PWM0_BL_Thr1
#define  MIS_PWM0_BL_Thr2_VADDR                    MISC_PWM_PWM0_BL_Thr2


#define  MIS_PWM1_CTRL_VADDR                       MISC_PWM_PWM1_CTRL_reg
#define  MIS_PWM1_TIMING_CTRL_VADDR                MISC_PWM_PWM1_Timing_CTRL_reg
#define  MIS_PWM1_DUTY_SET_VADDR                   MISC_PWM_PWM1_DUTY_SET_reg
#define  MIS_PWM1_DVS_PERIOD_VADDR                 MISC_PWM_PWM1_DVS_PERIOD_reg
#define  MIS_PWM1_DVS_MONITOR_VADDR                MISC_PWM_PWM1_DVS_MONITOR_reg
#define  MIS_PWM1_MONITOR_VADDR                    MISC_PWM_PWM1_MONITOR_reg
#define  MIS_PWM1_ERR_STATUS_VADDR                 MISC_PWM_PWM1_ERR_STATUS_reg
#define  MIS_PWM1_TRACK_SET_VADDR                  MISC_PWM_PWM1_TRACK_SET_reg
#define  MIS_PWM1_BL_CTR_VADDR                     MISC_PWM_PWM1_BL_CTR
#define  MIS_PWM1_BL_COMPENSATE_VADDR              MISC_PWM_PWM1_BL_COMPENSATE
#define  MIS_PWM1_BL_Thr1_VADDR                    MISC_PWM_PWM1_BL_Thr1
#define  MIS_PWM1_BL_Thr2_VADDR                    MISC_PWM_PWM1_BL_Thr2

#define  MIS_PWM2_CTRL_VADDR                       MISC_PWM_PWM2_CTRL_reg
#define  MIS_PWM2_TIMING_CTRL_VADDR                MISC_PWM_PWM2_Timing_CTRL_reg
#define  MIS_PWM2_DUTY_SET_VADDR                   MISC_PWM_PWM2_DUTY_SET_reg
#define  MIS_PWM2_DVS_PERIOD_VADDR                 (PWM_RESERVED_VADDR)
#define  MIS_PWM2_DVS_MONITOR_VADDR                (PWM_RESERVED_VADDR)
#define  MIS_PWM2_MONITOR_VADDR                    (PWM_RESERVED_VADDR)
#define  MIS_PWM2_ERR_STATUS_VADDR                 (PWM_RESERVED_VADDR)
#define  MIS_PWM2_TRACK_SET_VADDR                  (PWM_RESERVED_VADDR)

#define  MIS_PWM3_CTRL_VADDR                       MISC_PWM_PWM3_CTRL_reg
#define  MIS_PWM3_TIMING_CTRL_VADDR                MISC_PWM_PWM3_Timing_CTRL_reg
#define  MIS_PWM3_DUTY_SET_VADDR                   MISC_PWM_PWM3_DUTY_SET_reg
#define  MIS_PWM3_DVS_PERIOD_VADDR                 (PWM_RESERVED_VADDR)
#define  MIS_PWM3_DVS_MONITOR_VADDR                (PWM_RESERVED_VADDR)
#define  MIS_PWM3_MONITOR_VADDR                    (PWM_RESERVED_VADDR)
#define  MIS_PWM3_ERR_STATUS_VADDR                 (PWM_RESERVED_VADDR)
#define  MIS_PWM3_TRACK_SET_VADDR                  (PWM_RESERVED_VADDR)

#define  MIS_PWM4_CTRL_VADDR                       MISC_PWM_PWM4_CTRL_reg
#define  MIS_PWM4_TIMING_CTRL_VADDR                MISC_PWM_PWM4_Timing_CTRL_reg
#define  MIS_PWM4_DUTY_SET_VADDR                   MISC_PWM_PWM4_DUTY_SET_reg
#define  MIS_PWM4_DVS_PERIOD_VADDR                 (PWM_RESERVED_VADDR)
#define  MIS_PWM4_DVS_MONITOR_VADDR                (PWM_RESERVED_VADDR)
#define  MIS_PWM4_MONITOR_VADDR                    (PWM_RESERVED_VADDR)
#define  MIS_PWM4_ERR_STATUS_VADDR                 (PWM_RESERVED_VADDR)
#define  MIS_PWM4_TRACK_SET_VADDR                  (PWM_RESERVED_VADDR)

#define  MIS_PWM5_CTRL_VADDR                       MISC_PWM_PWM5_CTRL_reg
#define  MIS_PWM5_TIMING_CTRL_VADDR                MISC_PWM_PWM5_Timing_CTRL_reg
#define  MIS_PWM5_DUTY_SET_VADDR                   MISC_PWM_PWM5_DUTY_SET_reg
#define  MIS_PWM5_DVS_PERIOD_VADDR                 (PWM_RESERVED_VADDR)
#define  MIS_PWM5_DVS_MONITOR_VADDR                (PWM_RESERVED_VADDR)
#define  MIS_PWM5_MONITOR_VADDR                    (PWM_RESERVED_VADDR)
#define  MIS_PWM5_ERR_STATUS_VADDR                 (PWM_RESERVED_VADDR)
#define  MIS_PWM5_TRACK_SET_VADDR                  (PWM_RESERVED_VADDR)

#define  MIS_PWM6_CTRL_VADDR                       MISC_PWM_PWM6_CTRL_reg
#define  MIS_PWM6_TIMING_CTRL_VADDR                MISC_PWM_PWM6_Timing_CTRL_reg
#define  MIS_PWM6_DUTY_SET_VADDR                   MISC_PWM_PWM6_DUTY_SET_reg
#define  MIS_PWM6_DVS_PERIOD_VADDR                 (PWM_RESERVED_VADDR)
#define  MIS_PWM6_DVS_MONITOR_VADDR                (PWM_RESERVED_VADDR)
#define  MIS_PWM6_MONITOR_VADDR                    (PWM_RESERVED_VADDR)
#define  MIS_PWM6_ERR_STATUS_VADDR                 (PWM_RESERVED_VADDR)
#define  MIS_PWM6_TRACK_SET_VADDR                  (PWM_RESERVED_VADDR)

#define  MIS_PWM7_CTRL_VADDR                       MISC_PWM_PWM7_CTRL_reg
#define  MIS_PWM7_TIMING_CTRL_VADDR                MISC_PWM_PWM7_Timing_CTRL_reg
#define  MIS_PWM7_DUTY_SET_VADDR                   MISC_PWM_PWM7_DUTY_SET_reg
#define  MIS_PWM7_DVS_PERIOD_VADDR                 (PWM_RESERVED_VADDR)
#define  MIS_PWM7_DVS_MONITOR_VADDR                (PWM_RESERVED_VADDR)
#define  MIS_PWM7_MONITOR_VADDR                    (PWM_RESERVED_VADDR)
#define  MIS_PWM7_ERR_STATUS_VADDR                 (PWM_RESERVED_VADDR)
#define  MIS_PWM7_TRACK_SET_VADDR                  (PWM_RESERVED_VADDR)

#define iso_pwm_clk_sel_RBUS                       stb_st_clkmux_RBUS

#define  MIS_PWM_HW_PREDICT_FROM_D_DOMAIN_VADDR    (PWM_RESERVED_VADDR)
#define  MIS_PWM_OD_MODE_VADDR                     MISC_PWM_PWM_OD_MODE_reg
#define  MIS_DUMMY_VADDR                           MISC_PWM_PWM_DUMMY_reg

#define  MISC_PWM_DVS_HIGH_VADDR                   MISC_PWM_DVS_HIGH_reg
#define  MISC_PWM_DVS_TOTAL_VADDR                  MISC_PWM_DVS_TOTAL_reg
#define  MISC_PWM_GEN_DVS_VADDR                    MISC_PWM_GEN_DVS_reg
#define  MISC_PWM_PWM_INT_MONITOR_VADDR            MISC_PWM_PWM_INT_MONITOR_reg

#define  MISC_PWM_PWM0_MONITOR_CNT_VADDR           MISC_PWM_PWM0_MONITOR_CNT_reg
#define  MISC_PWM_PWM0_CNT_SET_VADDR               MISC_PWM_PWM0_CNT_SET_reg
#define  MISC_PWM_PWM0_PHASE_THR_VADDR             MISC_PWM_PWM0_PHASE_THR_reg
#define  MISC_PWM_PWM0_INT_EN_VADDR                MISC_PWM_PWM0_INT_EN_reg
#define  MISC_PWM_PWM0_INT_EVENT_VADDR             MISC_PWM_PWM0_INT_EVENT_reg

#define  MISC_PWM_PWM1_MONITOR_CNT_VADDR           MISC_PWM_PWM1_MONITOR_CNT_reg
#define  MISC_PWM_PWM1_CNT_SET_VADDR               MISC_PWM_PWM1_CNT_SET_reg
#define  MISC_PWM_PWM1_PHASE_THR_VADDR             MISC_PWM_PWM1_PHASE_THR_reg
#define  MISC_PWM_PWM1_INT_EN_VADDR                MISC_PWM_PWM1_INT_EN_reg
#define  MISC_PWM_PWM1_INT_EVENT_VADDR             MISC_PWM_PWM1_INT_EVENT_reg

#define  MISC_PWM_PWM2_MONITOR_CNT_VADDR           MISC_PWM_PWM2_MONITOR_CNT_reg
#define  MISC_PWM_PWM2_CNT_SET_VADDR               MISC_PWM_PWM2_CNT_SET_reg
#define  MISC_PWM_PWM2_PHASE_THR_VADDR             MISC_PWM_PWM2_PHASE_THR_reg
#define  MISC_PWM_PWM2_INT_EN_VADDR                MISC_PWM_PWM2_INT_EN_reg
#define  MISC_PWM_PWM2_INT_EVENT_VADDR             MISC_PWM_PWM2_INT_EVENT_reg

#define  MISC_PWM_PWM3_MONITOR_CNT_VADDR           MISC_PWM_PWM3_MONITOR_CNT_reg
#define  MISC_PWM_PWM3_CNT_SET_VADDR               MISC_PWM_PWM3_CNT_SET_reg
#define  MISC_PWM_PWM3_PHASE_THR_VADDR             MISC_PWM_PWM3_PHASE_THR_reg
#define  MISC_PWM_PWM3_INT_EN_VADDR                MISC_PWM_PWM3_INT_EN_reg
#define  MISC_PWM_PWM3_INT_EVENT_VADDR             MISC_PWM_PWM3_INT_EVENT_reg

#define  MISC_PWM_PWM4_MONITOR_CNT_VADDR           MISC_PWM_PWM4_MONITOR_CNT_reg
#define  MISC_PWM_PWM4_CNT_SET_VADDR               MISC_PWM_PWM4_CNT_SET_reg
#define  MISC_PWM_PWM4_PHASE_THR_VADDR             MISC_PWM_PWM4_PHASE_THR_reg
#define  MISC_PWM_PWM4_INT_EN_VADDR                MISC_PWM_PWM4_INT_EN_reg
#define  MISC_PWM_PWM4_INT_EVENT_VADDR             MISC_PWM_PWM4_INT_EVENT_reg

#define  MISC_PWM_PWM5_MONITOR_CNT_VADDR           MISC_PWM_PWM5_MONITOR_CNT_reg
#define  MISC_PWM_PWM5_CNT_SET_VADDR               MISC_PWM_PWM5_CNT_SET_reg
#define  MISC_PWM_PWM5_PHASE_THR_VADDR             MISC_PWM_PWM5_PHASE_THR_reg
#define  MISC_PWM_PWM5_INT_EN_VADDR                MISC_PWM_PWM5_INT_EN_reg
#define  MISC_PWM_PWM5_INT_EVENT_VADDR             MISC_PWM_PWM5_INT_EVENT_reg

#define  MISC_PWM_PWM6_MONITOR_CNT_VADDR           MISC_PWM_PWM6_MONITOR_CNT_reg
#define  MISC_PWM_PWM6_CNT_SET_VADDR               MISC_PWM_PWM6_CNT_SET_reg
#define  MISC_PWM_PWM6_PHASE_THR_VADDR             MISC_PWM_PWM6_PHASE_THR_reg
#define  MISC_PWM_PWM6_INT_EN_VADDR                MISC_PWM_PWM6_INT_EN_reg
#define  MISC_PWM_PWM6_INT_EVENT_VADDR             MISC_PWM_PWM6_INT_EVENT_reg

#define  MISC_PWM_PWM7_MONITOR_CNT_VADDR           MISC_PWM_PWM7_MONITOR_CNT_reg
#define  MISC_PWM_PWM7_CNT_SET_VADDR               MISC_PWM_PWM7_CNT_SET_reg
#define  MISC_PWM_PWM7_PHASE_THR_VADDR             MISC_PWM_PWM7_PHASE_THR_reg
#define  MMISC_PWM_PWM7_INT_EN_VADDR               MISC_PWM_PWM7_INT_EN_reg
#define  MISC_PWM_PWM7_INT_EVENT_VADDR             MISC_PWM_PWM7_INT_EVENT_reg

//MISC DB Function Register
#define misc_pwm_db0_ctrl_RBUS                     misc_pwm_pwm_db0_ctrl_RBUS
#define misc_pwm_mis_db1_ctrl_RBUS                 misc_pwm_pwm_db1_ctrl_RBUS

//MISC Basic Function Register
#define misc_pwm_ctrl_RBUS                         misc_pwm_pwm0_ctrl_RBUS
#define misc_pwm_timing_ctrl_RBUS                  misc_pwm_pwm0_timing_ctrl_RBUS
#define misc_pwm_duty_set_RBUS                     misc_pwm_pwm0_duty_set_RBUS
#define misc_pwm_mis_dvs_period_RBUS               misc_pwm_pwm0_dvs_period_RBUS
#define misc_pwm_mis_dvs_monitor_RBUS              misc_pwm_pwm0_dvs_monitor_RBUS
#define misc_pwm_mis_monitor_RBUS                  misc_pwm_pwm0_monitor_RBUS
#define misc_pwm_mis_err_status_RBUS               misc_pwm_pwm0_err_status_RBUS

// MISC PWM tracking mode
#define misc_pwm_track_set_RBUS                   misc_pwm_pwm0_track_set_RBUS
// ISO PWM
#define iso_pwm_db0_ctrl_RBUS                      iso_misc_pwm_mis_pwm_db_ctrl_RBUS
#define iso_pwm_ctrl_RBUS                          iso_misc_pwm_mis_pwm0_ctrl_RBUS

#define pwm_timing_ctrl_RBUS                       misc_pwm_pwm0_timing_ctrl_RBUS
#define iso_pwm_timing_ctrl_RBUS                   iso_misc_pwm_mis_pwm0_timing_ctrl_RBUS

#define pwm_duty_set_RBUS                          misc_pwm_pwm0_duty_set_RBUS
#define iso_pwm_duty_set_RBUS                      iso_misc_pwm_mis_pwm0_duty_set_RBUS

#define pwm_mis_dvs_period_RBUS                    misc_pwm_pwm0_dvs_period_RBUS
#define pwm_mis_dvs_monitor_RBUS                   misc_pwm_pwm0_dvs_monitor_RBUS
#define pwm_mis_monitor_RBUS                       misc_pwm_pwm0_monitor_RBUS
#define pwm_mis_err_status_RBUS                    misc_pwm_pwm0_err_status_RBUS

//MISC Tracking Mode Register
#define pwm_mis_track_set_RBUS                     misc_pwm_pwm0_track_set_RBUS
#define pwm_mis_track_fsm_RBUS                     misc_pwm_pwm0_track_fsm_RBUS
#define pwm_mis_track_pti_RBUS                     misc_pwm_pwm0_track_pti_RBUS

//MISC Tracking Mode Register(Frequency)
#define pwm_mis_freq_0_RBUS                        misc_pwm_pwm0_freq_0_RBUS
#define pwm_mis_freq_0_RBUS                        misc_pwm_pwm0_freq_0_RBUS
#define pwm_mis_freq_1_RBUS                        misc_pwm_pwm0_freq_1_RBUS

//MISC Tracking Mode Register(Phase)
#define pwm_mis_phase_0_RBUS                       misc_pwm_pwm0_phase_0_RBUS
#define pwm_mis_phase_1_RBUS                       misc_pwm_pwm0_phase_1_RBUS

//MISC Predict Mode Register
#define pwm_mis_hw_predict_RBUS 
#define pwm_mis_sw_predict_RBUS 
#define pwm_mis_predict_ratio_RBUS 

//DTG Predict Register
#define pwm_mis_predict_ratio_RBUS 
#define pwm_iso_align_RBUS 
#define pwm_iso_trackvalue_RBUS 


#ifndef LITTLE_ENDIAN    // apply BIG_ENDIAN
typedef union
{
    RBus_UInt32    regValue;
    struct
    {
        RBus_UInt32    pwm7_clksel:1;
        RBus_UInt32    pwm6_clksel:1;
        RBus_UInt32    pwm5_clksel:1;
        RBus_UInt32    pwm4_clksel:1;
        RBus_UInt32    pwm3_clksel:1;
        RBus_UInt32    pwm2_clksel:1;
        RBus_UInt32    pwm1_clksel:1;
        RBus_UInt32    pwm0_clksel:1;
        RBus_UInt32    reserved_1:7;
        RBus_UInt32    i2c4_clksel:1;
        RBus_UInt32    i2c3_clksel:1;
        RBus_UInt32    dtv_demod_sel:1;
        RBus_UInt32    i2c2_clksel:1;
        RBus_UInt32    i2c1_clksel:1;
        RBus_UInt32    dtv_demod_hdic_en:1;
        RBus_UInt32    NF_CLKSEL:3;
        RBus_UInt32    pwm_pll_clksel:1;
        RBus_UInt32    AUD_DTV2_FREQ_SEL:3;
        RBus_UInt32    Dummy:1;
        RBus_UInt32    AUD_DTV_FREQ_SEL:3;
    };
}pwm_sys_clk_div_RBUS;

#else    // apply LITTLE_ENDIAN

typedef union
{
    RBus_UInt32    regValue;
    struct
    {
        RBus_UInt32    AUD_DTV_FREQ_SEL:3;
        RBus_UInt32    Dummy:1;
        RBus_UInt32    AUD_DTV2_FREQ_SEL:3;
        RBus_UInt32    pwm_pll_clksel:1;
        RBus_UInt32    NF_CLKSEL:3;
        RBus_UInt32    dtv_demod_hdic_en:1;
        RBus_UInt32    i2c1_clksel:1;
        RBus_UInt32    i2c2_clksel:1;
        RBus_UInt32    dtv_demod_sel:1;
        RBus_UInt32    i2c3_clksel:1;
        RBus_UInt32    i2c4_clksel:1;
        RBus_UInt32    reserved_1:7;
        RBus_UInt32    pwm0_clksel:1;
        RBus_UInt32    pwm1_clksel:1;
        RBus_UInt32    pwm2_clksel:1;
        RBus_UInt32    pwm3_clksel:1;
        RBus_UInt32    pwm4_clksel:1;
        RBus_UInt32    pwm5_clksel:1;
        RBus_UInt32    pwm6_clksel:1;
        RBus_UInt32    pwm7_clksel:1;
    };
}pwm_sys_clk_div_RBUS;

#endif

typedef struct rtk_pwm_reg_map_t{
    union {
        unsigned long MISC_CTRL;
        unsigned long ISO_CTRL;
    };
    union {
        unsigned long MISC_TIMING_CTRL;
        unsigned long ISO_TIMING_CTRL;
    };
    union {
        unsigned long MISC_DUTY_SET;
        unsigned long ISO_DUTY_SET;
    };
    union {
        unsigned long DVS_PERIOD;
        unsigned long ALIGN_SET;
    };
    union {
        unsigned long DV_MONITOR;
        unsigned long TRACKVALUE_SET;
    };
    union {
        unsigned long MONITOR;
        unsigned long PWM_UNUSED1;
    };
    union {
        unsigned long ERR_STATUS;
        unsigned long PWM_UNUSED2;
    };
    union {
        unsigned long TRACK_SET;
        unsigned long PWM_UNUSED3;
    };
    union {
        unsigned long TRACK_SET2;
        unsigned long PWM_UNUSED4;
    };
    union {
        unsigned long TRACK_STEP;
        unsigned long PWM_UNUSED5;
    };
    union {
        unsigned long TRACK_STATUS;
        unsigned long PWM_UNUSED6;
    };
    union {
        unsigned long BL_CTR;
        unsigned long PWM_UNUSED7;
    };
    union {
        unsigned long BL_COMPENSATE;
        unsigned long PWM_UNUSED8;
    };
    union {
        unsigned long BL_Thr1;
        unsigned long PWM_UNUSED9;
    };
    union {
        unsigned long BL_Thr2;
        unsigned long PWM_UNUSED10;
    };
}RTK_PWM_REG_MAP;

typedef struct rtk_pwm_reg_map_t rtk_pwm_reg_map;

struct rtk_pwm_phy_t {
    const rtk_pwm_reg_map *p_reg_map;
};
typedef struct rtk_pwm_phy_t rtk_pwm_phy;

static const rtk_pwm_reg_map mis_pwm0_reg = {
    .MISC_CTRL = MIS_PWM0_CTRL_VADDR,
    .MISC_TIMING_CTRL = MIS_PWM0_TIMING_CTRL_VADDR,
    .MISC_DUTY_SET = MIS_PWM0_DUTY_SET_VADDR,
    .DVS_PERIOD = MIS_PWM0_DVS_PERIOD_VADDR,
    .DV_MONITOR = MIS_PWM0_DVS_MONITOR_VADDR,
    .MONITOR = MIS_PWM0_MONITOR_VADDR,
    .ERR_STATUS = MIS_PWM0_ERR_STATUS_VADDR,
    .TRACK_SET = MIS_PWM0_TRACK_SET_VADDR,
    .BL_CTR     = MIS_PWM0_BL_CTR_VADDR,
    .BL_COMPENSATE     = MIS_PWM0_BL_COMPENSATE_VADDR,
    .BL_Thr1     = MIS_PWM0_BL_Thr1_VADDR,
    .BL_Thr2     = MIS_PWM0_BL_Thr2_VADDR,
};

static const rtk_pwm_reg_map mis_pwm1_reg = {
    .MISC_CTRL = MIS_PWM1_CTRL_VADDR,
    .MISC_TIMING_CTRL = MIS_PWM1_TIMING_CTRL_VADDR,
    .MISC_DUTY_SET = MIS_PWM1_DUTY_SET_VADDR,
    .DVS_PERIOD = MIS_PWM1_DVS_PERIOD_VADDR,
    .DV_MONITOR = MIS_PWM1_DVS_MONITOR_VADDR,
    .MONITOR = MIS_PWM1_MONITOR_VADDR,
    .ERR_STATUS = MIS_PWM1_ERR_STATUS_VADDR,
    .TRACK_SET = MIS_PWM1_TRACK_SET_VADDR,
    .BL_CTR     = MIS_PWM1_BL_CTR_VADDR,
    .BL_COMPENSATE     = MIS_PWM1_BL_COMPENSATE_VADDR,
    .BL_Thr1     = MIS_PWM1_BL_Thr1_VADDR,
    .BL_Thr2     = MIS_PWM1_BL_Thr2_VADDR,
};

static const rtk_pwm_reg_map mis_pwm2_reg = {
    .MISC_CTRL = MIS_PWM2_CTRL_VADDR,
    .MISC_TIMING_CTRL = MIS_PWM2_TIMING_CTRL_VADDR,
    .MISC_DUTY_SET = MIS_PWM2_DUTY_SET_VADDR,
    .DVS_PERIOD = MIS_PWM2_DVS_PERIOD_VADDR,
    .DV_MONITOR = MIS_PWM2_DVS_MONITOR_VADDR,
    .MONITOR = MIS_PWM2_MONITOR_VADDR,
    .ERR_STATUS = MIS_PWM2_ERR_STATUS_VADDR,
    .TRACK_SET = MIS_PWM2_TRACK_SET_VADDR,

};

static const rtk_pwm_reg_map mis_pwm3_reg = {
    .MISC_CTRL = MIS_PWM3_CTRL_VADDR,
    .MISC_TIMING_CTRL = MIS_PWM3_TIMING_CTRL_VADDR,
    .MISC_DUTY_SET = MIS_PWM3_DUTY_SET_VADDR,
    .DVS_PERIOD = MIS_PWM3_DVS_PERIOD_VADDR,
    .DV_MONITOR = MIS_PWM3_DVS_MONITOR_VADDR,
    .MONITOR = MIS_PWM3_MONITOR_VADDR,
    .ERR_STATUS = MIS_PWM3_ERR_STATUS_VADDR,
    .TRACK_SET = MIS_PWM3_TRACK_SET_VADDR,

};

static const rtk_pwm_reg_map mis_pwm4_reg = {
    .MISC_CTRL = MIS_PWM4_CTRL_VADDR,
    .MISC_TIMING_CTRL = MIS_PWM4_TIMING_CTRL_VADDR,
    .MISC_DUTY_SET = MIS_PWM4_DUTY_SET_VADDR,
    .DVS_PERIOD = MIS_PWM4_DVS_PERIOD_VADDR,
    .DV_MONITOR = MIS_PWM4_DVS_MONITOR_VADDR,
    .MONITOR = MIS_PWM4_MONITOR_VADDR,
    .ERR_STATUS = MIS_PWM4_ERR_STATUS_VADDR,
    .TRACK_SET = MIS_PWM4_TRACK_SET_VADDR,

};

static const rtk_pwm_reg_map mis_pwm5_reg = {
    .MISC_CTRL = MIS_PWM5_CTRL_VADDR,
    .MISC_TIMING_CTRL = MIS_PWM5_TIMING_CTRL_VADDR,
    .MISC_DUTY_SET = MIS_PWM5_DUTY_SET_VADDR,
    .DVS_PERIOD = MIS_PWM5_DVS_PERIOD_VADDR,
    .DV_MONITOR = MIS_PWM5_DVS_MONITOR_VADDR,
    .MONITOR = MIS_PWM5_MONITOR_VADDR,
    .ERR_STATUS = MIS_PWM5_ERR_STATUS_VADDR,
    .TRACK_SET = MIS_PWM5_TRACK_SET_VADDR,

};

static const rtk_pwm_reg_map mis_pwm6_reg = {
    .MISC_CTRL = MIS_PWM6_CTRL_VADDR,
    .MISC_TIMING_CTRL = MIS_PWM6_TIMING_CTRL_VADDR,
    .MISC_DUTY_SET = MIS_PWM6_DUTY_SET_VADDR,
    .DVS_PERIOD = MIS_PWM6_DVS_PERIOD_VADDR,
    .DV_MONITOR = MIS_PWM6_DVS_MONITOR_VADDR,
    .MONITOR = MIS_PWM6_MONITOR_VADDR,
    .ERR_STATUS = MIS_PWM6_ERR_STATUS_VADDR,
    .TRACK_SET = MIS_PWM6_TRACK_SET_VADDR,
};

static const rtk_pwm_reg_map mis_pwm7_reg = {
    .MISC_CTRL = MIS_PWM7_CTRL_VADDR,
    .MISC_TIMING_CTRL = MIS_PWM7_TIMING_CTRL_VADDR,
    .MISC_DUTY_SET = MIS_PWM7_DUTY_SET_VADDR,
    .DVS_PERIOD = MIS_PWM7_DVS_PERIOD_VADDR,
    .DV_MONITOR = MIS_PWM7_DVS_MONITOR_VADDR,
    .MONITOR = MIS_PWM7_MONITOR_VADDR,
    .ERR_STATUS = MIS_PWM7_ERR_STATUS_VADDR,
    .TRACK_SET = MIS_PWM7_TRACK_SET_VADDR,
};

static const rtk_pwm_reg_map iso_pwm0_reg = {
    .ISO_CTRL = ISO_PWM0_CTRL_VADDR,
    .ISO_TIMING_CTRL = ISO_PWM0_TIMING_CTRL_VADDR,
    .ISO_DUTY_SET = ISO_PWM0_DUTY_SET_VADDR,
    .ALIGN_SET = ISO_PWM0_ALIGN_VADDR,
    .TRACKVALUE_SET = ISO_PWM0_TRACKVALUE_VADDR,
    .PWM_UNUSED1  = ISO_PWM_NONE,
    .PWM_UNUSED2  = ISO_PWM_NONE,
    .PWM_UNUSED3  = ISO_PWM_NONE,
    .PWM_UNUSED4  = ISO_PWM_NONE,
    .PWM_UNUSED5  = ISO_PWM_NONE,
    .PWM_UNUSED6  = ISO_PWM_NONE,
};

static const rtk_pwm_reg_map iso_pwm1_reg = {
    .ISO_CTRL = ISO_PWM1_CTRL_VADDR,
    .ISO_TIMING_CTRL = ISO_PWM1_TIMING_CTRL_VADDR,
    .ISO_DUTY_SET = ISO_PWM1_DUTY_SET_VADDR,
    .ALIGN_SET = ISO_PWM1_ALIGN_VADDR,
    .TRACKVALUE_SET = ISO_PWM1_TRACKVALUE_VADDR,
    .PWM_UNUSED1  = ISO_PWM_NONE,
    .PWM_UNUSED2  = ISO_PWM_NONE,
    .PWM_UNUSED3  = ISO_PWM_NONE,
    .PWM_UNUSED4  = ISO_PWM_NONE,
    .PWM_UNUSED5  = ISO_PWM_NONE,
    .PWM_UNUSED6  = ISO_PWM_NONE,
};

static const rtk_pwm_phy pwm_iso_phy[] = {
    {&iso_pwm0_reg},
    {&iso_pwm1_reg},
};

static const rtk_pwm_phy pwm_mis_phy[] = {
    {&mis_pwm0_reg},
    {&mis_pwm1_reg},
    {&mis_pwm2_reg},
    {&mis_pwm3_reg},
    {&mis_pwm4_reg},
    {&mis_pwm5_reg},
    {&mis_pwm6_reg},
    {&mis_pwm7_reg},
};
void PWM_ISO_INT_CTR_REG_R(iso_misc_int_ctrl_RBUS *pwm_iso_int_ctrl_reg);
void PWM_ISO_INT_CTR_REG_W(iso_misc_int_ctrl_RBUS *pwm_iso_int_ctrl_reg);
void M_PWM_DB0_CTRL_REG_R(int index,misc_pwm_db0_ctrl_RBUS* pwm_mis_db0_ctrl_reg);
void M_PWM_DB0_CTRL_REG_W(int index,misc_pwm_db0_ctrl_RBUS* pwm_mis_db0_ctrl_reg);
void M_PMM_DB1_CTRL_REG_R(int index,misc_pwm_mis_db1_ctrl_RBUS* pwm_mis_db1_ctrl_reg);
void M_PWM_DB1_CTRL_REG_W(int index,misc_pwm_mis_db1_ctrl_RBUS* pwm_mis_db1_ctrl_reg);
void M_PWM_CTRL_REG_R(int index,misc_pwm_ctrl_RBUS* pwm_mis_ctrl_reg);
void M_PWM_CTRL_REG_W(int index,misc_pwm_ctrl_RBUS* pwm_mis_ctrl_reg);
void M_PWM_TIMING_CTRL_REG_R(int index,misc_pwm_timing_ctrl_RBUS* pwm_timing_ctrl_reg);
void M_PWM_TIMING_CTRL_REG_W(int index,misc_pwm_timing_ctrl_RBUS* pwm_timing_ctrl_reg);
void M_PWM_DUTY_SET_REG_R(int index,misc_pwm_duty_set_RBUS* pwm_duty_set_reg);
void M_PWM_DUTY_SET_REG_W(int index,misc_pwm_duty_set_RBUS* pwm_duty_set_reg);
void M_PWM_DVS_PERIOD_REG_R(int index,misc_pwm_mis_dvs_period_RBUS* pwm_dvs_period_reg);
void M_PWM_DVS_PERIOD_REG_W(int index,misc_pwm_mis_dvs_period_RBUS* pwm_dvs_period_reg);
void M_PWM_DVS_MONITOR_REG_R(int index,misc_pwm_mis_dvs_monitor_RBUS* pwm_dvs_monitor_reg);
void M_PWM_DVS_MONITOR_REG_W(int index,misc_pwm_mis_dvs_monitor_RBUS* pwm_dvs_monitor_reg);
void M_PWM_MONITOR_REG_R(int index,misc_pwm_mis_monitor_RBUS* pwm_monitor_reg);
void M_PWM_MONITOR_REG_W(int index,misc_pwm_mis_monitor_RBUS* pwm_monitor_reg);
void M_PWM_ERR_STATUS_R(int index,misc_pwm_mis_err_status_RBUS* pwm_err_status_reg);
void M_PWM_ERR_STATUS_W(int index,misc_pwm_mis_err_status_RBUS* pwm_err_status_reg);
// MISC PWM Tracking mode
void M_PWM_TRACK_SET_R(int index,misc_pwm_track_set_RBUS* pwm_track_set_reg);
void M_PWM_TRACK_SET_W(int index,misc_pwm_track_set_RBUS* pwm_track_set_reg);
void I_PWM_DB_CTRL_REG_R(int index,iso_pwm_db0_ctrl_RBUS* pwm_iso_db_ctrl_reg);
void I_PWM_DB_CTRL_REG_W(int index,iso_pwm_db0_ctrl_RBUS* pwm_iso_db_ctrl_reg);
void I_PWM_CTRL_REG_R(int index,iso_pwm_ctrl_RBUS* pwm_iso_ctrl_reg);
void I_PWM_CTRL_REG_W(int index,iso_pwm_ctrl_RBUS* pwm_iso_ctrl_reg);
void I_PWM_TIMING_CTRL_REG_R(int index,iso_pwm_timing_ctrl_RBUS* pwm_timing_ctrl_reg);
void I_PWM_TIMING_CTRL_REG_W(int index,iso_pwm_timing_ctrl_RBUS* pwm_timing_ctrl_reg);
void I_PWM_DUTY_SET_REG_R(int index,iso_pwm_duty_set_RBUS* pwm_duty_set_reg);
void I_PWM_DUTY_SET_REG_W(int index,iso_pwm_duty_set_RBUS* pwm_duty_set_reg);

#endif
