#ifndef __RTK_MODULE_EVENT_ANDROID_S_H__
#define __RTK_MODULE_EVENT_ANDROID_S_H__
#include <fw/android/module_list.h>
#include <fw/android/event_string_list-miracast.h>


/*
######################################################################################
# MODULE STRING DEFINE
######################################################################################
*/
struct event_module_s event_android_module[]={
    {ANDROID_MODULE_VALUE_MIRACAST, "MIRACAST", android_miracast_eventtype},
    {EVENT_LIMIT_MAX,NULL,NULL},
};

#endif /* __RTK_MODULE_EVENT_ANDROID_S_H__ */