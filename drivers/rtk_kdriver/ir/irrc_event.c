#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <rtk_kdriver/rtk_irrc_event.h>

void (*irrc_event_callback_func)(void);

int irrc_event_register_callback(IRRC_EVENT_HANDLER * p_handler)
{
    int ret = -1;
    if(p_handler == NULL)
        return ret;

    irrc_event_callback_func = p_handler->callback_func;
    return 0;
}

int irrc_event_unregister_callback(IRRC_EVENT_HANDLER * p_handler)
{
    int ret = -1;
    if (!p_handler)
        return ret;

    return 0;
}

void irrc_event_notify(void)
{
    irrc_event_callback_func();
}