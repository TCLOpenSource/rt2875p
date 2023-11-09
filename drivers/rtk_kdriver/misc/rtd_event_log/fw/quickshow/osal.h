#ifndef __RTK_EVENT_QUICKSHOW_H__
#define __RTK_EVENT_QUICKSHOW_H__
#include <string.h>
#include <vsprintf.h>
#include <printf.h>
#include <rtk_kdriver/io.h>
#include <rbus/timer_reg.h>
/************************************************************************
 *  rtk log define
 ************************************************************************/
#define EV_EMERG


#define EV_ALERT   printf2

#define EV_CRIT


#define EV_ERR


#define EV_WARNING


#define EV_NOTICE

#define EV_INFO

#define EV_DEBUG
/*
######################################################################################
# STATIC FUNCTION
######################################################################################
*/
unsigned int rtk_timer_misc_90k_us( void );
unsigned int rtk_timer_misc_90k_ms( void );


/*
######################################################################################
# Define Save
######################################################################################
*/

#if defined(CONFIG_REALTEK_EVENT_LOG)
#define rtd_quickshow_event_log(event_type, module, event, event_val, module_reserved)    event_log_save(EVENT_FW_QUICKSHOW, event_type, module, event, event_val, module_reserved)
#else /* CONFIG_REALTEK_EVENT_LOG */
#define rtd_quickshow_event_log(event_type, module, event, event_val, module_reserved)
#endif /* CONFIG_REALTEK_EVENT_LOG */

#endif /* __RTK_EVENT_QUICKSHOW_H__ */