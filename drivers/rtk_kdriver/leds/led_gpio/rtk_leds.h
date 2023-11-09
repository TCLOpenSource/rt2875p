#ifndef __RTK_LEDS_H__
#define __RTK_LEDS_H__

//#include <linux/gpio.h>
#include <linux/leds.h>

//#define DEFAULT_TRIGGER "gpio"
#define DEV_NAME "led"

#define MAX_LEDS_COUNT 10
#define PCB_ENUM_NAME_LEN_MAX   64

#define RTK_LED_DEBUG(fmt, args...) rtd_pr_led_debug(fmt, ## args)
#define RTK_LED_INFO(fmt, args...)  rtd_pr_led_info(fmt, ## args)
#define RTK_LED_WARN(fmt, args...)  rtd_pr_led_warn(fmt, ## args)
#define RTK_LED_ERR(fmt, args...)   rtd_pr_led_err(fmt, ## args)

typedef struct linux_pcb_leds_enum_t {
    char name[PCB_ENUM_NAME_LEN_MAX];
    unsigned long long value;
}RTK_PCB_LEDS_ENUM;

int rtk_led_set(char *pcb_name, int enable);
int rtk_led_blink_set(char *name,unsigned long delayon, unsigned long delayoff);
int rtk_led_oneshot_set(char *name,unsigned long delayon, unsigned long delayoff, int invert);

int rtk_leds_parse_and_register_dev(RTK_PCB_LEDS_ENUM *pcbinfo, struct gpio_led *led);



#endif /*__RTK_IR_H__ */
