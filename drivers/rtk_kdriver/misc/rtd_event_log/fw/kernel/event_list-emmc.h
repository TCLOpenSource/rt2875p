#ifndef __RTK_MODULE_EVENT_EMMC_H__
#define __RTK_MODULE_EVENT_EMMC_H__
/*
######################################################################################
# EMMC EVENT DEFINE
######################################################################################
*/
typedef enum
{
    KERNEL_EMMC_EVENT_START_EVENT_ID,
    KERNEL_EMMC_EVENT_EVENT_MAX
}KERNEL_EMMC_EVENT_EVENT_ID;

/*
######################################################################################
# EMMC EVENT TYPE DEFINE
######################################################################################
*/
typedef enum
{
    KERNEL_EMMC_EVENT_TYPE_ID,
    KERNEL_EMMC_TYPE_MAX
}KERNEL_EMMC_TYPE_ID;

/*
######################################################################################
# EMMC EVENT MERGER DW1 DEFINE
######################################################################################
*/
#define   EMMC_DW1_EVENT_START    MERGER_EVENT_DW1(EVENT_FW_KERNEL, KERNEL_EMMC_MODULE_ID, KERNEL_EMMC_EVENT_type_ID, KERNEL_EMMC_EVENT_START_EVENT_ID)

#define rtd_emmc_event_log(type, event, event_val, module_reserved)     rtd_kernel_event_log(type, KERNEL_EMMC_MODULE_ID, event, event_val, module_reserved)
#endif /* __RTK_MODULE_EVENT_EMMC_H__ */

