#ifndef __RTK_PWM_FUNC_TV002_H__
#define __RTK_PWM_FUNC_TV002_H__

#define RTK_PWM_0 "PIN_PWM0_DIMMER"      //F10
#define RTK_PWM_1 "PIN_BL_ADJ"           //G10
#define RTK_PWM_2 "PIN_PWM2_DIMMER"      //E9
#define RTK_PWM_3 "PIN_DC_DIMMER_IP_REF" //F9

typedef enum{
    PWM_POS_DELAY    = 0,
    PWM_CLK_DELAY    = 1,
}RTK_PWM_DELAY_MODE;

int rtk_pwm_backlight_set_freq_ex(const char *pcbname, int freq );
int rtk_pwm_backlight_set_vsync_ex(const char *pcbname, int vsync );
int rtk_pwm_backlight_set_vsync_delay_ex(const char *pcbname, int mode, int vsync_delay);
int rtk_pwm_backlight_set_duty_ex(const char *pcbname, int duty );
int rtk_pwm_backlight_apply(const char *pcbname);

#endif
