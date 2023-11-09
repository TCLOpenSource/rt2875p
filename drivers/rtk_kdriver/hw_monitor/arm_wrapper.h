#ifndef __ARM_WRAPPER_H__
#define __ARM_WRAPPER_H__

#include "rtk_hw_monitor.h"

// SET 4
#define WRAPPER_NUM 4

#define WRAP_TAG "WRAP"

#define ARMWRAP_DBG(fmt, args...) rtd_pr_arm_warp_debug(fmt, ## args)
#define ARMWRAP_INFO(fmt, args...) rtd_pr_arm_warp_info(fmt, ## args)
#define ARMWRAP_ERR(fmt, args...) rtd_pr_arm_warp_err(fmt, ## args)

typedef enum
{
    WRAPPER_READ_WRITE = HWM_RW,
    WRAPPER_READ = HWM_R,
    WRAPPER_WRTIE = HWM_W,
}WRAPPER_RW;

enum WRAPPER_DFT_ENTRY
{
    WRAPPER_DFT_ENTRY_SCPU_TEXT = 1,
    WRAPPER_DFT_ENTRY_OVERRANGE,
    WRAPPER_DFT_ENTRY_ACPU_TEXT,
    WRAPPER_DFT_ENTRY_VCPU_TEXT,
};

#endif /* __ARM_WRAPPER_H__ */

