#ifndef __ARCH_ARM_RTK_THERMAL_H
#define __ARCH_ARM_RTK_THERMAL_H
#include <linux/kref.h>
#include <linux/list.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#define RTK_THERMAL_DRIVER_TEMP_MAPPING
void init_rtk_thermal_sensor(void);
void exit_rtk_thermal_sensor(void);
int rtk_get_thermal_value(void);
void rtk_thermal_sensor_enable(int enable);
unsigned int rtk_get_thermal_reg(void);
int rtk_get_thermal_value_ext(void);
int rtk_thermal_trigger_low(int temp_low);
int rtk_thermal_trigger_high(int temp_high);
int rtk_thermal_int_status(void);
int rtk_thermal_sensor_init(void);
void rtk_thermal_sensor_exit(void);
int rtk_monitor_init(void);
int rtk_monitor_resume(void);
typedef int (*register_temp_fun_ptr)(int, void *,void*, char*);
int register_cpu_thermal_handler(register_temp_fun_ptr opt);
void rtk_monitor_disable(void);

#define THERMAL_DBG(fmt, args...)               rtd_pr_thermal_debug("[DBG] " fmt, ## args)
#define THERMAL_INFO(fmt, args...)              rtd_pr_thermal_info("[Info] " fmt, ## args)
#define THERMAL_WARNING(fmt, args...)      rtd_pr_thermal_warn("[Warn] " fmt, ## args)
#define THERMAL_ERR(fmt, args...)               rtd_pr_thermal_err("[Err] " fmt, ## args)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#define RTK_THERMAL_THROTTLE_STEP0	90
#define RTK_THERMAL_THROTTLE_STEP1	100
#define RTK_THERMAL_THROTTLE_STEP2	120
#define RTK_THERMAL_THROTTLE_STEP3	140	//shutdown
#endif
#ifdef THERMAL_DRIVER_GET_FACTORY
#define THROTTLE_LINE_MAX  10
#define THROTTLE_CONTENT_MAX  8
typedef enum
{
        MODULE_NAME_KEY_E = 0,
        THROTTLE_ENABLE_KEY_E,
        THROTTLE_0_KEY_E,
        THROTTLE_1_KEY_E,
        THROTTLE_2_KEY_E,
        THROTTLE_3_KEY_E,
        THROTTLE_MAX_KEY_E,
}THROTTLE_KEY_E;

typedef struct
{
        char module_name_content[THROTTLE_CONTENT_MAX];
        int content[THROTTLE_MAX_KEY_E-1];
}THROTTLE_CONTENT_T;

typedef struct
{
        unsigned int magic_num;
        int check_sum;
        bool valid;
        char reserve1;
        char reserve2;
        char reserve3;
        THROTTLE_CONTENT_T throttle_content[THROTTLE_LINE_MAX];
}THROTTLE_RESOURC_T;
#endif
#endif
