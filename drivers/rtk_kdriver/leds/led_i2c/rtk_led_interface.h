#ifndef __RTK_LED_INTERFACE_H__
#define __RTK_LED_INTERFACE_H__
#include "led.h"

#include <rtd_log/rtd_module_log.h>
#include <linux/printk.h>

#define LED_INFO(fmt, args...)  rtd_pr_led_debug(fmt, ## args)
#define LED_WARN(fmt, args...)  rtd_pr_led_warn(fmt, ## args)
#define LED_ERR(fmt, args...)   rtd_pr_led_err(fmt, ## args)

#define LED_ADDR_NULL 0
#define LED_SEL_NULL 0

#define	LED_DEVICE_ADDRESS 	         0x68

/* IOCTL */
#define RTK_LED_IOC_MAGIC                        'l'
#define RTK_LED_IOC_SET_COLOR           _IOW(RTK_LED_IOC_MAGIC, 1, int)
#define RTK_LED_IOC_DETECT           _IOR(RTK_LED_IOC_MAGIC, 1, int)
#define RTK_LED_IOC_INIT           _IOR(RTK_LED_IOC_MAGIC, 2, int)

struct led_controller {
    int led_i2c_id;
    int sel_index;
    int slave_addr;
    void (*ops)(int led_i2c_id, unsigned short slave_addr,int color[],int count);
    int (*detect)(int led_i2c_id, unsigned short slave_addr);
    int (*init)(int led_i2c_id, unsigned short slave_addr);
    int (*dump_all)(const char *buf, int led_i2c_id, unsigned short slave_addr);
};

typedef enum rtk_amp_ic_type {
    CLEAR   = 0,
    ACL1310 = 1,
    AD82010 = 2,
    TAS5751 = 3,
} rtk_amp_ic_type_e;


typedef enum led_type {
    ASSISTANT_1,
    ASSISTANT_2,
    ASSISTANT_3,
    ASSISTANT_4,

    count,
} LEDTYPE;

struct LedLight {
    LEDTYPE type;
};

/*

struct LedLight {
    LedType type;
};


* The type of the LED Indicator.
*
* The assistant lights should cycle left to right, or
* clockwise starting from top, starting from the first
* light. A minimum of four lights is required.


enum LedType : int32_t {
    ASSISTANT_1,
    ASSISTANT_2,
    ASSISTANT_3,
    ASSISTANT_4,

    COUNT,
};

interface ILed {
    
     * Sets the list of LED light to the desired color.
     *
     * This is a non-blocking call and the order of execution among successive
     * calls is not guaranteed.
     *
     * @param leds The list of LED light to set color.
     * @param colors The corresponding color (0x00RRGGBB) of the LED light to set to.
     
    @callflow(next={"*"})
    oneway setColor(vec<LedLight> leds, vec<uint32_t> colors);
};
*/

typedef struct {
    int count;
    int colors[count];
} LED_COLOR_INPUT;

int rtk_led_i2c_id(void);

#endif
