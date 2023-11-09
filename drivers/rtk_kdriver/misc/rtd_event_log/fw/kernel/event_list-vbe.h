#ifndef __RTK_MODULE_EVENT_VBE_H__
#define __RTK_MODULE_EVENT_VBE_H__
/*
######################################################################################
# VBE EVENT DEFINE
######################################################################################
*/
/*
######################################################################################
# VBE EVENT TYPE DEFINE
######################################################################################
*/
typedef enum
{
    KERNEL_VBE_TYPE_MAX
}KERNEL_VBE_TYPE_ID;

/*
######################################################################################
# VBE EVENT MERGER DW1 DEFINE
######################################################################################
*/

#define rtd_vbe_event_log(type, event, event_val, module_reserved)     rtd_kernel_event_log(type, KERNEL_VBE_MODULE_ID, event, event_val, module_reserved)
#endif /* __RTK_MODULE_EVENT_VBE_H__ */

