
#ifndef __RTK_MODULE_EVENT_QUICKSHOW_STRING_DEFINE_H__
#define __RTK_MODULE_EVENT_QUICKSHOW_STRING_DEFINE_H__
#include <fw/quickshow/module_list.h>
#include <fw/quickshow/event_string_list-vby1.h>
/*
######################################################################################
# MODULE STRING DEFINE
######################################################################################
*/
struct event_module_s quickshow_module[]={
    {QUICKSHOW_VBY1_MODULE_ID, "VBY1", quickshow_vby1_eventtype},
    {EVENT_LIMIT_MAX,NULL,NULL},
};
#endif /* __RTK_MODULE_EVENT_QUICKSHOW_STRING_DEFINE_H__ */
