#ifndef __RTK_EXTCON_GPIO_H__
#define __RTK_EXTCON_GPIO_H__

#include <linux/rwsem.h>
#include <linux/extcon-provider.h>

#define RTK_EXTCON_GPIO_DEV_NUM 32
#define INIT_LOCK(lock)         init_rwsem(&(lock))
#define WRITE_LOCK(lock)     up_write(&(lock))
#define WRITE_UNLOCK(lock)   down_write(&(lock))
#define READ_LOCK(lock)      up_read(&(lock))
#define READ_UNLOCK(lock)        down_read(&(lock))

typedef enum{
    HP_OUT_JD      = 0,
    SWITCH_WOV     = 1,
    GENERAL_TYPE   = 2,
}EXTCON_GPIO_TYPE;

//HEADSET Type From Android Framework
typedef enum{
    PLUG_OUT_HEADSET      = 0,
    PLUG_IN_NO_MIC        = 1,
    PLUG_IN_HEADSET_ANLG  = 2,
}HEADSET_STATE;


typedef struct gpio_rtkextcon_data {
    struct extcon_dev *edev;
    unsigned gpio;
    bool gpio_active_low;
    const char *name_on;
    const char *name_off;
    const char *state_on;
    const char *state_off;
    struct work_struct work;
    EXTCON_GPIO_TYPE type;
    char name[16];
}GPIO_EXTCON_DEV;

typedef struct gpio_rtkextcon_list
{
    struct rw_semaphore lock;
    unsigned int gpio_extcon_num;
    GPIO_EXTCON_DEV *gpio_extcons[RTK_EXTCON_GPIO_DEV_NUM];
}GPIO_EXTCON_LIST;

#define RTK_EXTCON_DEBUG(fmt, args...)	rtd_pr_extcon_debug(fmt, ## args)
#define RTK_EXTCON_INFO(fmt, args...)	rtd_pr_extcon_info(fmt, ## args)
#define RTK_EXTCON_WARN(fmt, args...)	rtd_pr_extcon_warn(fmt, ## args)
#define RTK_EXTCON_ERR(fmt, args...)	rtd_pr_extcon_err(fmt, ## args)

#endif
