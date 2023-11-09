#include "rtk_pwm-reg.h"
#include <rbus/misc_pwm_reg.h>

#define rtk_pwm_mr(index,reg)      rtd_inl(pwm_mis_phy[index].p_reg_map->reg);
#define rtk_pwm_mw(index,reg,name) rtd_outl(pwm_mis_phy[index].p_reg_map->reg,name->regValue);

void PWM_ISO_INT_CTR_REG_R(iso_misc_int_ctrl_RBUS *pwm_iso_int_ctrl_reg)
{    pwm_iso_int_ctrl_reg->regValue = rtd_inl(PWM_ISO_INT_CTRL);}

void PWM_ISO_INT_CTR_REG_W(iso_misc_int_ctrl_RBUS *pwm_iso_int_ctrl_reg)
{    rtd_outl(PWM_ISO_INT_CTRL,pwm_iso_int_ctrl_reg->regValue);}

void M_PWM_DB0_CTRL_REG_R(int index,misc_pwm_db0_ctrl_RBUS* pwm_mis_db0_ctrl_reg)
{    pwm_mis_db0_ctrl_reg->regValue = rtd_inl(MIS_PWM_DB0_CTRL_VADDR);    }

void M_PWM_DB0_CTRL_REG_W(int index,misc_pwm_db0_ctrl_RBUS* pwm_mis_db0_ctrl_reg)
{    rtd_outl(MIS_PWM_DB0_CTRL_VADDR,pwm_mis_db0_ctrl_reg->regValue);     }

void M_PWM_DB1_CTRL_REG_R(int index,misc_pwm_mis_db1_ctrl_RBUS* pwm_mis_db1_ctrl_reg)
{    pwm_mis_db1_ctrl_reg->regValue = rtd_inl(MIS_PWM_DB1_CTRL_VADDR);    }

void M_PWM_DB1_CTRL_REG_W(int index,misc_pwm_mis_db1_ctrl_RBUS* pwm_mis_db1_ctrl_reg)
{    rtd_outl(MIS_PWM_DB1_CTRL_VADDR,pwm_mis_db1_ctrl_reg->regValue );    }

void M_PWM_CTRL_REG_R(int index,misc_pwm_ctrl_RBUS* pwm_mis_ctrl_reg)
{    pwm_mis_ctrl_reg->regValue = rtk_pwm_mr(index,MISC_CTRL);    }

void M_PWM_CTRL_REG_W(int index,misc_pwm_ctrl_RBUS* pwm_mis_ctrl_reg)
{    rtk_pwm_mw(index,MISC_CTRL,pwm_mis_ctrl_reg);    }

void M_PWM_TIMING_CTRL_REG_R(int index,misc_pwm_timing_ctrl_RBUS* pwm_timing_ctrl_reg)
{    pwm_timing_ctrl_reg->regValue = rtk_pwm_mr(index,MISC_TIMING_CTRL);    }

void M_PWM_TIMING_CTRL_REG_W(int index,misc_pwm_timing_ctrl_RBUS* pwm_timing_ctrl_reg)
{    rtk_pwm_mw(index,MISC_TIMING_CTRL,pwm_timing_ctrl_reg);    }

void M_PWM_DUTY_SET_REG_R(int index,misc_pwm_duty_set_RBUS* pwm_duty_set_reg)
{    pwm_duty_set_reg->regValue = rtk_pwm_mr(index,MISC_DUTY_SET);    }

void M_PWM_DUTY_SET_REG_W(int index,misc_pwm_duty_set_RBUS* pwm_duty_set_reg)
{    rtk_pwm_mw(index,MISC_DUTY_SET,pwm_duty_set_reg);    }

void M_PWM_DVS_PERIOD_REG_R(int index,misc_pwm_mis_dvs_period_RBUS* pwm_dvs_period_reg)
{    pwm_dvs_period_reg->regValue = rtk_pwm_mr(index,DVS_PERIOD);    }

void M_PWM_DVS_PERIOD_REG_W(int index,misc_pwm_mis_dvs_period_RBUS* pwm_dvs_period_reg)
{    rtk_pwm_mw(index,DVS_PERIOD,pwm_dvs_period_reg);    }

void M_PWM_DVS_MONITOR_REG_R(int index,misc_pwm_mis_dvs_monitor_RBUS* pwm_dvs_monitor_reg)
{    pwm_dvs_monitor_reg->regValue = rtk_pwm_mr(index,DV_MONITOR);    }

void M_PWM_DVS_MONITOR_REG_W(int index,misc_pwm_mis_dvs_monitor_RBUS* pwm_dvs_monitor_reg)
{    rtk_pwm_mw(index,DV_MONITOR,pwm_dvs_monitor_reg);    }

void M_PWM_MONITOR_REG_R(int index,misc_pwm_mis_monitor_RBUS* pwm_monitor_reg)
{    pwm_monitor_reg->regValue = rtk_pwm_mr(index,MONITOR);    }

void M_PWM_MONITOR_REG_W(int index,misc_pwm_mis_monitor_RBUS* pwm_monitor_reg)
{    rtk_pwm_mw(index,MONITOR,pwm_monitor_reg);    }

void M_PWM_ERR_STATUS_R(int index,misc_pwm_mis_err_status_RBUS* pwm_err_status_reg)
{    pwm_err_status_reg->regValue = rtk_pwm_mr(index,ERR_STATUS);    }

void M_PWM_ERR_STATUS_W(int index,misc_pwm_mis_err_status_RBUS* pwm_err_status_reg)
{    rtk_pwm_mw(index,ERR_STATUS,pwm_err_status_reg);    }

// MISC PWM Tracking mode
void M_PWM_TRACK_SET_R(int index,misc_pwm_track_set_RBUS* pwm_track_set_reg)
{    pwm_track_set_reg->regValue = rtk_pwm_mr(index,TRACK_SET);    }

void M_PWM_TRACK_SET_W(int index,misc_pwm_track_set_RBUS* pwm_track_set_reg)
{    rtk_pwm_mw(index,TRACK_SET,pwm_track_set_reg);    }

#if !defined(CONFIG_ARCH_RTK2851A) && !defined(CONFIG_ARCH_RTK2851C) && !defined(CONFIG_ARCH_RTK2851F)
void M_PWM_TRACK_SET2_R(int index,misc_pwm_track_set2_RBUS* pwm_track_set2_reg)
{    pwm_track_set2_reg->regValue = rtk_pwm_mr(index,TRACK_SET2);    }

void M_PWM_TRACK_SET2_W(int index,misc_pwm_track_set2_RBUS* pwm_track_set2_reg)
{    rtk_pwm_mw(index,TRACK_SET2,pwm_track_set2_reg);    }

void M_PWM_TRACK_STEP_R(int index,misc_pwm_track_step_RBUS* pwm_track_step_reg)
{    pwm_track_step_reg->regValue = rtk_pwm_mr(index,TRACK_STEP);    }

void M_PWM_TRACK_STEP_W(int index,misc_pwm_track_step_RBUS* pwm_track_step_reg)
{    rtk_pwm_mw(index,TRACK_STEP,pwm_track_step_reg);    }

void M_PWM_TRACK_STATUS_R(int index,misc_pwm_track_status_RBUS* pwm_track_status_reg)
{    pwm_track_status_reg->regValue = rtk_pwm_mr(index,TRACK_STATUS);    }

#elif defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F)
void M_PWM_BL_CTR_R(int index, misc_pwm_pwm0_bl_ctr_RBUS* pwm_bl_ctr_reg)
{    pwm_bl_ctr_reg->regValue = rtk_pwm_mr(index,BL_CTR);    }
void M_PWM_BL_CTR_W(int index, misc_pwm_pwm0_bl_ctr_RBUS* pwm_bl_ctr_reg)
{    rtk_pwm_mw(index,BL_CTR,pwm_bl_ctr_reg);    }
void M_PWM_BL_COMPENSATE_R(int index, misc_pwm_pwm0_bl_compensate_RBUS* pwm_bl_compensate_reg)
{    pwm_bl_compensate_reg->regValue = rtk_pwm_mr(index,BL_COMPENSATE);    }
void M_PWM_BL_COMPENSATE_W(int index, misc_pwm_pwm0_bl_compensate_RBUS* pwm_bl_compensate_reg)
{    rtk_pwm_mw(index,BL_COMPENSATE,pwm_bl_compensate_reg);    }
void M_PWM_BL_THR1_R(int index, misc_pwm_pwm0_bl_thr1_RBUS* pwm_bl_thr1_reg)
{    pwm_bl_thr1_reg->regValue = rtk_pwm_mr(index,BL_Thr1);    }
void M_PWM_BL_THR1_W(int index, misc_pwm_pwm0_bl_thr1_RBUS* pwm_bl_thr1_reg)
{    rtk_pwm_mw(index,BL_Thr1,pwm_bl_thr1_reg);    }
void M_PWM_BL_THR2_R(int index, misc_pwm_pwm0_bl_thr2_RBUS* pwm_bl_thr2_reg)
{    pwm_bl_thr2_reg->regValue = rtk_pwm_mr(index,BL_Thr2);    }
void M_PWM_BL_THR2_W(int index, misc_pwm_pwm0_bl_thr2_RBUS* pwm_bl_thr2_reg)
{    rtk_pwm_mw(index,BL_Thr2,pwm_bl_thr2_reg);    }

#endif

//ISO
#define rtk_pwm_ir(index,reg)      rtd_inl(pwm_iso_phy[index].p_reg_map->reg);
#define rtk_pwm_iw(index,reg,name) rtd_outl(pwm_iso_phy[index].p_reg_map->reg,name->regValue);
void I_PWM_DB_CTRL_REG_R(int index,iso_pwm_db0_ctrl_RBUS* pwm_iso_db_ctrl_reg)
{    pwm_iso_db_ctrl_reg->regValue = rtd_inl(ISO_PWM_DB_CTRL_VADDR);    }

void I_PWM_DB_CTRL_REG_W(int index,iso_pwm_db0_ctrl_RBUS* pwm_iso_db_ctrl_reg)
{    rtd_outl(ISO_PWM_DB_CTRL_VADDR,pwm_iso_db_ctrl_reg->regValue);     }

void I_PWM_CTRL_REG_R(int index,iso_pwm_ctrl_RBUS* pwm_iso_ctrl_reg)
{    pwm_iso_ctrl_reg->regValue = rtk_pwm_ir(index,ISO_CTRL);    }

void I_PWM_CTRL_REG_W(int index,iso_pwm_ctrl_RBUS* pwm_iso_ctrl_reg)
{    rtk_pwm_iw(index,ISO_CTRL,pwm_iso_ctrl_reg);    }

void I_PWM_TIMING_CTRL_REG_R(int index,iso_pwm_timing_ctrl_RBUS* pwm_timing_ctrl_reg)
{    pwm_timing_ctrl_reg->regValue = rtk_pwm_ir(index,ISO_TIMING_CTRL);    }

void I_PWM_TIMING_CTRL_REG_W(int index,iso_pwm_timing_ctrl_RBUS* pwm_timing_ctrl_reg)
{    rtk_pwm_iw(index,ISO_TIMING_CTRL,pwm_timing_ctrl_reg);    }

void I_PWM_DUTY_SET_REG_R(int index,iso_pwm_duty_set_RBUS* pwm_duty_set_reg)
{    pwm_duty_set_reg->regValue = rtk_pwm_ir(index,ISO_DUTY_SET);    }

void I_PWM_DUTY_SET_REG_W(int index,iso_pwm_duty_set_RBUS* pwm_duty_set_reg)
{    rtk_pwm_iw(index,ISO_DUTY_SET,pwm_duty_set_reg);    }
