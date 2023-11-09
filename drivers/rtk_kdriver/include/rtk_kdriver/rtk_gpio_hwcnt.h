#ifndef __RTK_GPIO_HWCNT_H__
#define __RTK_GPIO_HWCNT_H__

#include <rtk_kdriver/rtk_gpio.h>
#include <rtk_gpio-debug.h>
#include <rtk_kdriver/rtk_gpio-db.h>
#include <rtk_kdriver/rtk_gpio-dev.h>
#ifdef CONFIG_ARCH_RTK2885P
#include <rbus/iso_gpio_reg.h>
#else
#include <rbus/gpio_reg.h>
#endif
extern int rtk_hwcnt_set_gpio_func( RTK_GPIO_IRQ_SET_T gpio_irq_set);
extern int rtk_hwcnt_set_enable(unsigned char On_off);
extern int rtk_hwcnt_get_enable(void);
extern int rtk_hwcnt_get_counter(void);


#endif /*__RTK_GPIO_HWCNT_H__ */

