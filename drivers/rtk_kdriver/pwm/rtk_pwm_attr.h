#ifndef __RTK_PWM_ATTR_H__
#define __RTK_PWM_ATTR_H__

#ifndef BUILD_QUICK_SHOW
void rtk_pwm_create_attr(struct device *device);
void rtk_pwm_remove_attr(struct device *device);
#endif

#endif
