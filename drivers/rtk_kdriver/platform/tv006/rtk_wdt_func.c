/*----------------------------------------------------*
 * Realtek WDT driver for TV006
 * 
 * Desc: This file was requested by customer to allow
 *       external micom to wakup up TV SOC event handler
 *       by toggle a specified GPIO
 *----------------------------------------------------*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <rtk_kdriver/rtk_gpio.h>
#include <rtk_kdriver/pcbMgr.h>
#ifdef CONFIG_WEBOS
#include "../../staging/webos/logger/wdt_log.h"
#endif

RTK_GPIO_ID micom_pin;

void rtk_gpio_micom(void);

extern void wakeup_wdt_thread(void);      // provided by customer

void rtk_gpio_micom_isr(
    RTK_GPIO_ID     gid,
    unsigned char   assert, 
    void*           dev_id
    )
{
    //Customer's ISR function for MICOM
    //RTK_GPIO_WARNING("==> rtk_gpio_micom_isr\n");
    wakeup_wdt_thread();            
}


/*-----------------------------------------------*
 * Module
 *-----------------------------------------------*/

static int __init rtk_wdt_init(void)
{    
    RTK_GPIO_WARNING("==> rtk_gpio_micom\n");
    
    int ret = 0 ;
    int index, type = 0;
    static unsigned long long value = 0;
    
    ret = pcb_mgr_get_enum_info_byname("PIN_SYSTEM_DEBUG", &value);
    
    if (ret) {
        RTK_GPIO_WARNING("[%s]- get enum error. \n",
                         __func__);
        return -EINVAL;
    }
    
    if(GET_PIN_TYPE(value) ==PCB_PIN_TYPE_GPIO)
        type = MIS_GPIO;
    else if(GET_PIN_TYPE(value) == PCB_PIN_TYPE_ISO_GPIO)
        type = ISO_GPIO;
    
    index = GET_PIN_INDEX(value);
    
    micom_pin = rtk_gpio_id(type, index);
    
    rtk_gpio_set_dir(micom_pin, 0);
    
    rtk_gpio_set_debounce(micom_pin, RTK_GPIO_DEBOUNCE_100us);    /* 100 us */
    
    rtk_gpio_set_irq_polarity(micom_pin, 1); //Set high level active
    
    rtk_gpio_request_irq(micom_pin, rtk_gpio_micom_isr, "LG_micom_monitor", rtk_gpio_micom_isr);
    
    rtk_gpio_set_irq_enable(micom_pin, 1);

    RTK_GPIO_WARNING("rtk_gpio_micom for %s GPIO %d (%x) ready\n",
            gpio_type(gpio_group(micom_pin)), 
            gpio_idx(micom_pin), 
            micom_pin);    
            
    return 0;
}


static void __exit rtk_wdt_exit(void)
{
    rtk_gpio_set_irq_enable(micom_pin, 0);
    rtk_gpio_free_irq(micom_pin, 0);    
}

module_init(rtk_wdt_init);
module_exit(rtk_wdt_exit);
