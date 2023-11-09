#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/moduleparam.h>
#include <linux/workqueue.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include "rtk_ice.h"
#if IS_ENABLED(CONFIG_RTK_KDRV_SERIAL_8250)	
 extern void disable_printk(void);
extern void enable_printk(void);
extern void rtice_putchar(char c);
#endif
unsigned char g_ByPassRTICECmd = 0;

int rtice_enter_debug_mode(void)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_SERIAL_8250)	
	disable_printk();
#endif
	return 0;
}

int rtice_exit_debug_mode(void)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_SERIAL_8250)	
	enable_printk();
#endif
	return 0;
}


extern volatile unsigned char fDebugModeEnable;
/*----------------------------------------------------------------------
 * Func : rtice_register_command_handler
 *
 * Desc : register a command handler
 *
 * Parm : cmd : handle of RTICE command
 *
 * Retn : handle of rtice command buffer
 *----------------------------------------------------------------------*/
int rtice_uart_handler(unsigned char data)
{
	int ret = 0;
	if(g_ByPassRTICECmd > 0)
		return 0;
#if IS_ENABLED(CONFIG_RTK_KDRV_SERIAL_8250)
	ret = rtice_input(&data, 1);
#endif
	if(fDebugModeEnable)
		return ret;
	else
		return 0;
}


/*----------------------------------------------------------------------
 * Func : rtice_output
 *
 * Desc : output data
 *
 * Parm :
 *
 * Retn :
 *----------------------------------------------------------------------*/
void rtice_output(unsigned char *data, unsigned int len)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_SERIAL_8250)	
	while (len--)
		rtice_putchar(*(data++));
#endif	
}
