#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <rtk_kdriver/rtk_irrc_event.h>
#include <rtk_kdriver/rtk_leds.h>
#include <linux/dma-mapping.h>
#include <rtk_kdriver/rtk_gpio.h>

int rtk_panel_pin_get(unsigned char *pcbname)
{
    RTK_GPIO_PCBINFO_T pinInfo;
    int ret = 0;

    ret = rtk_gpio_get_pcb_info(pcbname,&pinInfo);
    if(ret == 0)
    {
        return rtk_gpio_output_get(pinInfo.gid);
    }

    rtd_pr_platform_err("panel %s not define in pcb_name\n",pcbname);
    return -1;
}

void rtk_tv002_leds_func(void)
{
    if((rtk_panel_pin_get("PIN_LVDS_ON_OFF") == 1) && (rtk_panel_pin_get("PIN_BL_ON_OFF") == 1)) // when tv on,  press IR LED flick
        rtk_led_oneshot_set("PIN_LED_IR",150,150,1);
}

IRRC_EVENT_HANDLER* g_ir_event_handler = NULL;
static int __init rtk_tv002_leds_init(void)
{
    g_ir_event_handler = (IRRC_EVENT_HANDLER *)kzalloc(sizeof(IRRC_EVENT_HANDLER), GFP_ATOMIC);
    if (!g_ir_event_handler) {
        rtd_pr_platform_err("IR create event handler failed\n");
        return -ENOMEM;
    }
    g_ir_event_handler->callback_func = rtk_tv002_leds_func;

    irrc_event_register_callback(g_ir_event_handler);
    return 0;
}


static void __exit rtk_tv002_leds_exit(void)
{
    irrc_event_unregister_callback(g_ir_event_handler);
    kfree(g_ir_event_handler);
}

module_init(rtk_tv002_leds_init);
module_exit(rtk_tv002_leds_exit);
