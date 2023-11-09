#ifndef __RTK_HW_MONITOR_H__
#define __RTK_HW_MONITOR_H__

#define VIRT_MODULE_SCPU 0x00000100
#define VIRT_MODULE_NULL 0xffffffff

#define FRED_START "\033[1;31;40m"
#define FGREEN_START "\033[1;32;40m"
#define FYELLOW_START "\033[1;33;40m"
#define FBLUE_START "\033[1;34;40m"
#define FCOLOR_END "\033[0m"

#define CONFIG_RTK_KDRV_SUPPORT_VCPU3

typedef enum
{
    HWM_RW  = 0,
    HWM_R ,
    HWM_W ,
    HWM_PERMISION_INVALID
}HWM_PERMISSION;

typedef enum
{
    HWM_MODE_NULL = 0,
    HWM_MODE_1 ,
    HWM_MODE_2 ,
    HWM_MODE_3 ,
    HWM_MODE_INVALID
}HWM_MODE;

typedef struct hwm_t
{
        unsigned int start; //monitor range >= start
        unsigned int end; //monitor range < ( end(64B Align) + 64B )
        unsigned int ip[8]; // DC_SYS IP and CPUs
        unsigned char ip_permission[8]; // r/w/rw permission
        unsigned char mode; //mode 1,2,3
        unsigned char keep_go; //keep go or not when trash, 0:panic for SCPU; 1:non-panic for SCPU
}hwm_t;

typedef struct arm_wrapper_t
{
        unsigned int start; //monitor range >= start
        unsigned int end; //monitor range < ( end(64B Align) + 64B )
        int id;
        unsigned int permission;
}arm_wrapper_t;

enum RANGE_SYNC
{
        RANGE_SYNC_A_START = 0,
        RANGE_SYNC_A_END,
        RANGE_SYNC_V_START,
        RANGE_SYNC_V_END,
        RANGE_SYNC_V2_START,
        RANGE_SYNC_V2_END,
        RANGE_SYNC_V3_START,
        RANGE_SYNC_V3_END,
        RANGE_SYNC_K_START,
        RANGE_SYNC_K_END,
        RANGE_SYNC_SECURE_START,
        RANGE_SYNC_SECURE_END,
        RANGE_SYNC_NUM
};

extern void flush_dcache_page(struct page *page);
//#include <linux/mount.h>
extern dev_t name_to_dev_t(const char *name);
static inline unsigned long get_kernel_default_monitor_text_start_addr (void)
{    
    return ((unsigned long)name_to_dev_t)&PAGE_MASK;
} 
static inline unsigned long get_kernel_default_monitor_text_end_addr (void)
{    
    return (((unsigned long)&kimage_vaddr+PAGE_SIZE)&PAGE_MASK)-1;
}
static inline unsigned long get_kernel_default_monitor_end_addr (void)
{    
        return 0;
    //return (((unsigned long)&of_root+PAGE_SIZE)&PAGE_MASK)-1;
}

#define HWM_TAG "HWM"
#define HWM_WARNING(fmt, args...) rtd_pr_hwm_warn(fmt, ## args)
#define HWM_ERR(fmt, args...) rtd_pr_hwm_err(fmt, ## args)
#define HWM_DBG(fmt, args...) rtd_pr_hwm_debug(fmt, ## args)

#define HWM_DVP(fmt, args...) rtd_pr_hwm_emerg(fmt, ## args) //develop phase api

#define DCMT_DISABLE_TIP "DCMT is not enabled! (CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)"
#define ARM_WRAPPER_DISABLE_TIP "ARM_WRAPPER is not enabled! (CONFIG_REALTEK_ARM_WRAPPER)"
#define SB2_DISABLE_TIP "SB2 is not enabled! (CONFIG_RTK_KDRV_SB2)"

void memprison_err_handler(void);

#endif /*__RTK_HW_MONITOR_H__*/
