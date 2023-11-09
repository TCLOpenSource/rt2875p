#ifndef __RTK_MODULE_EVENT_CORE_H__
#define __RTK_MODULE_EVENT_CORE_H__
/*
######################################################################################
# INCLUDE FW LIBRARY
######################################################################################
*/
#ifndef BUILD_QUICK_SHOW
#include <fw/kernel/osal.h>
#include <sample/osal.h>
#else
#include <fw/quickshow/osal.h>
#endif
/*
######################################################################################
# All FW Define
######################################################################################
*/
#include <fw/kernel/module_list.h>
#include <fw/quickshow/module_list.h>
#include <fw/android/module_list.h>
#include <sample/module_list.h>
/*
######################################################################################
# MAX DEFINE
######################################################################################
*/
#define FW_MAX                      16
#define MODULE_MAX                 256
#define EVENT_MAX                  256
#define EVENT_TYPE_MAX              16
#define EVENT_LIMIT_MAX            256
/*
######################################################################################
# HEAD TAG DEFINE
######################################################################################
*/
#define EVENT_SYNCBYTE                  (0x47)
/*
######################################################################################
# FW TAG DEFINE
######################################################################################
*/
#define EVENT_FW_KERNEL                 0x00
#define EVENT_FW_QUICKSHOW              0x02
#define EVENT_FW_RHAL                   0x01
#define EVENT_FW_OPTEE                  0x03
#define EVENT_FW_AUDIO                  0x04
#define EVENT_FW_VIDEO                  0x05
#define EVENT_FW_ANDROID                0x08
#define EVENT_FW_SAMPLE                 0x09
/*
######################################################################################
# MARCO FUNCTION
######################################################################################
*/
#define MERGER_EVENT_DW1(fw,event_type,module,event) (((EVENT_SYNCBYTE) << 24) | ((fw) << 20)| ((event_type) << 16) | (module << 8)| (event))
/*
######################################################################################
# lOG DEFINE
######################################################################################
*/
#ifndef BUILD_QUICK_SHOW
#define LOG_SIZE                        0x100000 //(4M size)
#else
#define LOG_SIZE                        0x10000 //(4M/16 size)
#endif
#define INT_LOG_SIZE                    (LOG_SIZE/4) //(4M size)
#define INT_PAGE_SIZE	PAGE_SIZE/4
#define EVENT_ERROR                     1
#define EVENT_OK                        0
#define DIFF_START                      1
#define DIFF_END                        2
#define BUF_SIZE                        256
/*
######################################################################################
# PRINTF DEFINE
######################################################################################
*/
#define event_alert_print  EV_ALERT

#define event_err_print  EV_ERR

#define event_warn_print  EV_WARNING

//rtd_printk(KERN_ERR,    MSG_LVL_TAG_PRINT "EVENTLOG", fmt, ##__VA_ARGS__)
/*
######################################################################################
# STRUCT DEFINE
######################################################################################
*/
struct module_filter_format {
    unsigned int module_enable : 1; // 1 bit
    unsigned short int event_type; // 16 bits
    unsigned int event[8]; // 256 bits
};

struct fw_filter_format {
    unsigned int fw_enable : 1;
    struct module_filter_format module_data[256];
};

struct event_format {
    /* DW1 */
    unsigned char syncbyte;
    unsigned char fw_type : 4;
    unsigned char event_type : 4;
    unsigned char module;
    unsigned char event;
    /* DW2 */
    unsigned int event_value;
    /* DW3 */
    unsigned int module_reserved;
    /* DW4 */
    unsigned int timestamp;
};
/*
######################################################################################
# EVENT STRING DEFINE
######################################################################################
*/
struct event_event_s{
    unsigned int event_id;
    char * name;
};
/*
######################################################################################
# EVENT TYPE STRING DEFINE
######################################################################################
*/
struct event_eventtype_s{
    unsigned int eventtype_id;
    char * name;
    struct event_event_s* event;
};
/*
######################################################################################
# MODULE STRING DEFINE
######################################################################################
*/
struct event_module_s{
    unsigned int module_id;
    char * name;
    struct event_eventtype_s* eventtype;
};
/*
######################################################################################
# FW STRING DEFINE
######################################################################################
*/
struct event_fw_s{
    unsigned int fw_id;
    char * name;
    struct event_module_s* module;
};
/*
######################################################################################
# EXTERN FUNCTION
######################################################################################
*/
extern unsigned int eventlog_endian_32_swap(unsigned int x);
//extern unsigned int eventlog_verify_format(unsigned int DW1, struct event_module_s* event_fw);
extern unsigned int eventlog_parser_printf(unsigned int DW1,unsigned int DW2, unsigned int DW3 ,unsigned int DW4, unsigned int time_unit);
extern void eventlog_parser_printf_all(void);
extern void eventlog_printf(unsigned int DW1,unsigned int DW2, unsigned int DW3 ,unsigned int DW4);
extern void eventlog_printf_all(void);
extern int eventlog_parser_file(unsigned int DW1,unsigned int DW2, unsigned int DW3 ,unsigned int DW4, unsigned int time_unit, char * file, unsigned int file_size);
extern void eventlog_show_define(void);
extern unsigned int event_log_save(unsigned int fw_type, unsigned int event_type, unsigned int module, unsigned int event, unsigned int event_val, unsigned int module_reserved);
extern void event_set_timeunit(bool time);
extern unsigned int eventlog_event_number(unsigned int fw_type, unsigned int event_type, unsigned int module, unsigned int event);
extern unsigned int eventlog_event_diff(unsigned int fw_type, unsigned int event_type, unsigned int module, unsigned int event);
extern void eventlog_90k_enable(void);
extern void eventlog_event_printf_number(unsigned int fw_type, unsigned int event_type, unsigned int module, unsigned int event);
extern unsigned int eventlog_event_two_diff(unsigned int DW11, unsigned int DW12);
extern unsigned int eventlog_merge_DW1(unsigned int fw_type, unsigned int event_type, unsigned int module, unsigned int event);
extern unsigned int event_log_save_DW1(unsigned int DW1, unsigned int event_val, unsigned int module_reserved);
extern unsigned int event_log_save_DW1_start(unsigned int DW1);
extern unsigned int event_log_save_DW1_end(unsigned int DW1);
extern unsigned int eventlog_DW1_diff(unsigned int DW1);
/*
######################################################################################
# EXTERN STRUCT and VARIABLE
######################################################################################
*/
extern struct fw_filter_format fw_filter_data[FW_MAX];
extern unsigned int log_memory[INT_LOG_SIZE];
extern unsigned int time_memory[INT_LOG_SIZE/4];
extern unsigned int output_buf_log[INT_LOG_SIZE];
extern unsigned int eventlog_time_unit;
extern unsigned int memory_count;
#endif /*__RTK_MODULE_EVENT_CORE_H__*/