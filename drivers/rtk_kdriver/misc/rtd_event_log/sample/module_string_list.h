#ifndef __RTK_MODULE_EVENT_SAMPLE_S_H__
#define __RTK_MODULE_EVENT_SAMPLE_S_H__
#include <sample/module_list.h>
#include <sample/event_string_list-test.h>
/*
######################################################################################
# MODULE STRING DEFINE
######################################################################################
*/
struct event_module_s event_sample_module[]={
    {SAMPLE_MODULE_ID_TEST, "SAMPLE_MODULE_ID_TEST", sample_test_eventtype},
    {EVENT_LIMIT_MAX,NULL,NULL},
};

#endif /* __RTK_MODULE_EVENT_SAMPLE_S_H__ */