#ifndef __RTK_MODULE_EVENT_TEST_S_H__
#define __RTK_MODULE_EVENT_TEST_S_H__
/*
######################################################################################
# TEST EVENT STRING DEFINE
######################################################################################
*/
static struct event_event_s sample_test1_event[]={
    {TEST1_START, "TEST1_START"},
    {EVENT_LIMIT_MAX,NULL},
};

static struct event_event_s sample_test2_event[]={
    {TEST2_END, "TEST2_END"},
    {EVENT_LIMIT_MAX,NULL},
};
/*
######################################################################################
# TEST EVENT TYPE STRING DEFINE
######################################################################################
*/

static struct event_eventtype_s sample_test_eventtype[]={
    {TEST_TYPE_1, "TEST_TYPE_1", sample_test1_event},
    {TEST_TYPE_2, "TEST_TYPE_2", sample_test2_event},
    {EVENT_LIMIT_MAX,NULL,NULL},
};
#endif /* __RTK_MODULE_EVENT_TEST_S_H__ */