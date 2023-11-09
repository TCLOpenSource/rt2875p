#ifndef __RTK_PWM_FUNC_H__
#define __RTK_PWM_FUNC_H__

#ifndef BUILD_QUICK_SHOW
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#else
#include <no_os/spinlock_types.h>
#endif
#include <rtk_kdriver/rtk_pwm.h>
#include "rtk_pwm-reg.h"
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/rtk_gpio.h>
#include <rtk_kdriver/rtk_gpio-db.h>
#include <rtd_log/rtd_module_log.h>

#define ABS(x, y)    ((x) > (y) ? ((x)-(y)) : ((y)-(x)))
#define MAX_PWM_NODE  10
#define PPOVERLAY_DH_TOTAL_LAST_LINE_LENGTH_VADDR  PPOVERLAY_DH_Total_Last_Line_Length_reg
#define PPOVERLAY_DV_TOTAL_VADDR                   PPOVERLAY_DV_total_reg

#define DVS_COUNT (0xb80282dc)
#define IVS_COUNT (0xb8025cdc)

#define RTK_PWM_INDEX(pchip2)    GET_PIN_INDEX(pchip2->ullPcbMgrValue)
#define RTK_PWM_TYPE(pchip2)     GET_PIN_TYPE(pchip2->ullPcbMgrValue)
#define PWM_MISC_TYPE(pchip2)    (RTK_PWM_TYPE(pchip2) == PCB_PIN_TYPE_PWM)
#define PWM_ISO_TYPE(pchip2)     (RTK_PWM_TYPE(pchip2) == PCB_PIN_TYPE_ISO_PWM)

struct pwm_track_private {
    unsigned long real_totalcnt;
    unsigned long real_duty;
    bool track_freq_complete;
    bool track_phase_complete;
};

typedef struct RTK_CHIP_T {
    int index;
    bool mapped;
    bool rtk_freq_changed;
    bool rtk_force_update;
    unsigned long long ullPcbMgrValue;
    struct pwm_chip *chip;
    char nodename[16];
    char pcbname[PCB_ENUM_NAME_LEN_MAX];
    const rtk_pwm_reg_map *reg;    /*hw phy address */
#ifndef BUILD_QUICK_SHOW
    struct device *pwm_device_st;    /* PWM-I-x or PWM-M-x */
    //struct pwm_device *pst_rtk_pwm_dev;    /* get from linux pwm_get */
#endif
    int rtk_duty_max;
    int rtk_totalcnt;
    int rtk_freq;
    int rtk_freq_100times;
    int lg_freq48n;
    int lg_freq50n;
    int lg_freq60n;
    int vt;            /*v-total */
    int ht;            /*h-total */
    UINT64 dclk;        /*dclk */
    int rtk_adapt_freq;
    unsigned int rtk_db_reg;
    int rtk_vsync;
    int rtk_enable;
    int rtk_duty;
    int rtk_polarity;
    int rtk_pos_start;
    unsigned int rtk_pos_start_clk;
    int rtk_adpt_en;
    int rtk_clk_src;
    int rtk_clk_sel;
    int rtk_freq_range;
    int rtk_vsync_interval;
    int cycle_max;
    int force_mode;
    int Max_step;   /* For TackValue */
    int min_thr;    /* For TackValue */
    int duty_ratio; /* For Vsync Align */
    int align_mode; /* For Vsync Align */
    int align_en;   /* For Vsync Align */
    rtk_pwm_pin_reg    pin_mux_info;
    RTK_GPIO_ID        gid;
    PIN_MODE_T         currentMode;
#ifndef BUILD_QUICK_SHOW
    struct work_struct work;
    wait_queue_head_t wq;
#endif
    spinlock_t lock;
    int fsm_state;
    int vsync_mode;
    int irq;          /* allocated irq number */
    struct pwm_track_private track_priv;
} R_CHIP_T;

/** Display Scaler call back function **/
unsigned int pwm_get_disp_htotal(void);
unsigned int pwm_get_disp_vtotal(void);
unsigned int pwm_get_disp_dclk(void);
unsigned int pwm_get_disp_refresh_rate(void);

/** PWM GPIO mode **/
int rtk_pwm_gpio_value(R_CHIP_T *pchip2);
void rtk_change_to_gpio_mode(R_CHIP_T *pchip2, GPIOPOT outputValue);
void rtk_change_to_pwm_mode(R_CHIP_T *pchip2);
int rtk_pwm_gpio_pinmux_init(R_CHIP_T *pwmPin);

/** Backlight Export Fucntion**/
int rtk_pwm_backlight_enable(int enable);
int rtk_pwm_backlight_status(void);
int rtk_pwm_backlight_set_duty( int duty );

/** CHIP PCB Fucntion**/
int rtk_pwm_pcb_mgr_get(const char *pcbname,int ricindex);
int rtk_pwm_pcb_mgr_get_value(RTK_PCB_ENUM *pcbinfo,int ricindex);
int rtk_pwm_pcb_mgr_get_totalcnt(R_CHIP_T *pwmPin);
int rtk_pwm_chip_index_get(int ricindex);
unsigned long long rtk_pwm_chip_pcb_mgr_value_get(int ricindex);
const rtk_pwm_reg_map * rtk_pwm_chip_reg_get(int ricindex);
#ifndef BUILD_QUICK_SHOW
void rtk_pwm_chip_device_set(struct platform_device *pdev);
#endif
R_CHIP_T* rtk_pwm_chip_get(int m_index);
R_CHIP_T* rtk_pwm_chip_get_by_index(int m_index,int m_type);
R_CHIP_T* rtk_pwm_chip_get_by_name(char* pcbname);

/** ioctl debug Fucntion**/
void m_ioctl_printk_set(int en_printk);
int m_ioctl_printk_get(void);
void m_ioctl_disable_set(int disable);
int m_ioctl_disable_get(void);
void m_ioctl_counter_inc(void);
void m_ioctl_counter_set(unsigned int counter);
unsigned int m_ioctl_counter_get(void);

/** Probe Initial Fucntion**/
int rtk_pwm_hw_type_init(R_CHIP_T *pwmPin);
void rtk_pwm_hw_chip_init(R_CHIP_T *pwmPin, int ricindex);
void rtk_pwm_ctrl_probe_init(R_CHIP_T *pchip2,int pwm_mode,int invert);
void rtk_pwm_timing_probe_init(R_CHIP_T *pchip2,int pwm_freq_hz, int duty_max,int *totalcnt);
void rtk_pwm_duty_probe_init(R_CHIP_T *pchip2,int duty, int duty_max,int totalcntg);
void rtk_pwm_totalcnt_init(R_CHIP_T *pchip2,int rtk_totalcnt);
void rtk_pwm_db_probe_init(R_CHIP_T *pchip2);

/** General Fucntion**/
void rtk_pwm_user_setting(R_CHIP_T *pchip2,RTK_PWM_INFO_T *stpwminfo);
void rtk_pwm_timing_freq_set_misc(R_CHIP_T *pchip2,misc_pwm_timing_ctrl_RBUS* pwm_timing_ctrl_reg);
void rtk_pwm_timing_freq_set_iso(R_CHIP_T *pchip2,iso_pwm_timing_ctrl_RBUS* pwm_timing_ctrl_reg);
void getHVTotal(int *htotal, int *vtotal);
void get_v_delay(R_CHIP_T *pchip2);
void getDClk(UINT64 *dclk);
int rtk_get_pwm_duty_base_change(int duty,int duty_max,int totalcnt);
unsigned long rtk_hw_get_clock(int mode);
unsigned long rtk_cpu_ns_get(int mode);
void rtk_hw_get_divider(int pwm_freq_hz, int duty_max, int *totalcnt,
                    unsigned short *M_best, unsigned short *N_best,
                    int pwm_freq_min, int mode);

int rtk_pwm_get_adapt_freq(R_CHIP_T *pchip2);
int rtk_get_pos(R_CHIP_T *pchip2,int delay_time,int m_freq);
int rtk_pwm_duty_get(int duty,int duty_max,int totalcnt);
unsigned long pwm_duty_ratio_get(unsigned long duty ,unsigned long pwm0_totalcnt);

/** Clock Source Fucntion**/

void rtk_pwm_clock_source(R_CHIP_T *pchip2);
void rtk_pwm_clock_freq_set(R_CHIP_T *pchip2);

/** Global variable Fucntion**/
void rtk_pwm_panel_mode_set(int mode);
int rtk_pwm_panel_mode_get(void);
void pin_index_count(void);
int pin_index_get(void);
void rtk_pwm_backlight_pin_set(int ricindex);
int rtk_pwm_backlight_pin_get(void);

/** PWM ISO Interrupt Control**/
void rtk_pwm_iso_interrupt_ctrl_enable_w(int enable);

/** Register Fucntion**/
void rtk_global_dump_set(int en_printk);
int  rtk_global_dump_get(void);
void rtk_pwm_global_apply_wr(int enable);
int rtk_pwm_global_enable(R_CHIP_T *pchip2,int enable);
int rtk_pwm_global_enable_mw(R_CHIP_T *pchip2,int enable);
int rtk_pwm_global_enable_iw(R_CHIP_T *pchip2,int enable);
int rtk_pwm_db_enable(R_CHIP_T *pchip2,int enable);
int rtk_pwm_db_enable_mw(R_CHIP_T *pchip2,int enable);
int rtk_pwm_db_enable_iw(R_CHIP_T *pchip2,int enable);
int rtk_pwm_db_sel_enable(R_CHIP_T *pchip2,int enable);
int rtk_pwm_db_sel_enable_mw(R_CHIP_T *pchip2,int enable);
int rtk_pwm_db_sel_enable_iw(R_CHIP_T *pchip2,int enable);

void rtk_pwm_db_wb(R_CHIP_T *pchip2);
void rtk_pwm_db1_wb(R_CHIP_T *pchip2);
int rtk_pwm_ctrl_duty_select_r(R_CHIP_T *pchip2);
void rtk_pwm_ctrl_duty_select_w(R_CHIP_T *pchip2,int enable);
void rtk_pwm_ctrl_polarity_w(R_CHIP_T *pchip2,int enable);
int rtk_pwm_ctrl_polarity_r(R_CHIP_T *pchip2);
void rtk_pwm_freq_w(R_CHIP_T *pchip2);
int rtk_pwm_freq_r(R_CHIP_T *pchip2);
void rtk_pwm_mn_r(R_CHIP_T *pchip2,unsigned long *M, unsigned long *N);
void rtk_pwm_num_w(R_CHIP_T *pchip2,int num);
void rtk_pwm_duty_w(R_CHIP_T *pchip2,int duty);
int rtk_pwm_duty_r(R_CHIP_T *pchip2);
void rtk_pwm_totalcnt_w(R_CHIP_T *pchip2,int totalcnt);
int rtk_pwm_totalcnt_r(R_CHIP_T *pchip2);
void rtk_pwm_vsync_w(R_CHIP_T *pchip2,int vsync);
int rtk_pwm_vsync_r(R_CHIP_T *pchip2);
void rtk_pwm_dvs_mux_w(R_CHIP_T *pchip2,int vsync);
int rtk_pwm_dvs_mux_r(R_CHIP_T *pchip2);
void rtk_pwm_output_w(R_CHIP_T *pchip2,int output);
int rtk_pwm_output_r(R_CHIP_T *pchip2);
void rtk_pwm_vs_delay_w(R_CHIP_T *pchip2,int delay);
unsigned int rtk_pwm_vs_delay_r(R_CHIP_T *pchip2);
void rtk_pwm_vsync_interval_w(R_CHIP_T *pchip2,int num);
int rtk_pwm_vsync_interval_r(R_CHIP_T *pchip2);
void rtk_pwm_cycle_max_w(R_CHIP_T *pchip2,int cycle_max);
int rtk_pwm_cycle_max_r(R_CHIP_T *pchip2);
void rtk_pwm_force_mode_w(R_CHIP_T *pchip2,int force_mode);
int rtk_pwm_force_mode_r(R_CHIP_T *pchip2);
void rtk_pwm_duty_ratio_w(R_CHIP_T *pchip2,unsigned long ratio);


/** Vsync Tracking & Predict Register Function**/
void rtk_pwm_dvs_max_period_w(R_CHIP_T *pchip2,unsigned long vs_max_period);
unsigned long rtk_pwm_dvs_max_period_r(R_CHIP_T *pchip2);
void rtk_pwm_dvs_min_period_w(R_CHIP_T *pchip2,unsigned long vs_min_period);
unsigned long rtk_pwm_dvs_min_period_r(R_CHIP_T *pchip2);
unsigned long rtk_pwm_dvs_monitor_period_r(R_CHIP_T *pchip2);
unsigned long rtk_pwm_dvs_monitor_realtime_r(R_CHIP_T *pchip2);
int rtk_pwm_dvs_monitor_interrupt_r(R_CHIP_T *pchip2);
unsigned long rtk_pwm_monitor_real_totalcnt_r(R_CHIP_T *pchip2);
unsigned long rtk_pwm_monitor_real_duty_r(R_CHIP_T *pchip2);
int rtk_pwm_err2_int_r(R_CHIP_T *pchip2);
int rtk_pwm_unstable_status_r(R_CHIP_T *pchip2);
void rtk_pwm_track_set_fixed_phase_en_w(R_CHIP_T *pchip2,int en);
void rtk_pwm_track_set_track_en_w(R_CHIP_T *pchip2,int en);
void rtk_pwm_track_set2_phase_offset_w(R_CHIP_T *pchip2,unsigned int phase_offset);
void rtk_pwm_track_set2_dvs_range_w(R_CHIP_T *pchip2,unsigned int dvs_range);
void rtk_pwm_track_step_max_step_w(R_CHIP_T *pchip2,unsigned int max_step);
int rtk_pwm_track_status_track_freq_r(R_CHIP_T *pchip2);
int rtk_pwm_track_status_phase_offset_r(R_CHIP_T *pchip2);
int rtk_pwm_track_status_pwm_full_r(R_CHIP_T *pchip2);
int pwm_vsync_disable(int index);
int pwm_db_apply(int index);
void rtk_pwm_enable_tracking_mode(int index);
void rtk_pwm_disable_tracking_mode(int index);

#ifndef BUILD_QUICK_SHOW
/** PWM ISR & Work queue Function**/
void rtk_pwm_work(struct work_struct *work);
irqreturn_t rtk_pwm_isr(int this_irq, void *dev_id);
#endif

/** Parameter Fucntion**/
int rtk_pwm_set_param_ex(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);
int rtk_pwm_get_param_ex(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);
int rtk_pwm_set_all_param_ex(R_CHIP_T *pchip2,RTK_PWM_INFO_EX_T* param_info);
int rtk_pwm_get_all_param_ex(R_CHIP_T *pchip2,RTK_PWM_INFO_EX_T* param_info);
int rtk_pwm_set_info(RTK_PWM_INFO_T* stpwminfo);
void rtk_pwm_polarity_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);

int rtk_pwm_frequency_param_set(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);
void rtk_pwm_frequency_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);
int rtk_pwm_duty_param_set(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);
void rtk_pwm_duty_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);
int rtk_pwm_totalcnt_param_set(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);
void rtk_pwm_totalcnt_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);
int rtk_pwm_vsync_param_set(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);
void rtk_pwm_vsync_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);
int rtk_pwm_output_param_set(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);
void rtk_pwm_output_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);
void rtk_pwm_vs_ineterval_param_set(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);
void rtk_pwm_vs_ineterval_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);
int rtk_pwm_vs_delay_param_set(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);
void rtk_pwm_vs_delay_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);
void rtk_pwm_cycle_max_param_set(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);
void rtk_pwm_cycle_max_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);
void rtk_pwm_force_mode_param_set(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);
void rtk_pwm_force_mode_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param);

#define PWM_DEBUG(fmt, args...) rtd_pr_pwm_debug(fmt, ## args)
#define PWM_INFO(fmt, args...)  rtd_pr_pwm_info(fmt, ## args)
#define PWM_WARN(fmt, args...)  rtd_pr_pwm_warn(fmt, ## args)
#define PWM_ERR(fmt, args...)   rtd_pr_pwm_err(fmt, ## args)

#endif
