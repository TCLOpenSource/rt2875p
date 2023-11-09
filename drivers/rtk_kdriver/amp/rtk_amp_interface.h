#ifndef __RTK_AMP_INTERFACE_H__
#define __RTK_AMP_INTERFACE_H__
#include "alc1310.h"
#include "alc1312.h"
#include "ad82010.h"
#include "ad82120.h"
#include "ad82088.h"
#include "tas5751.h"
#include "tas5707.h"
#include "tas5711.h"
#include "tas5805m.h"
#include "wa6819.h"
#include "WA156819.h"
#include "ad82050.h"
#include "rtk_amp_device_id_table.h"
#include <rtk_kdriver/i2c-rtk-api.h>


#include <rtd_log/rtd_module_log.h>
#ifndef BUILD_QUICK_SHOW
#include <linux/printk.h>

#define AMP_INFO(fmt, args...)  rtd_pr_amp_debug(fmt, ## args)
#define AMP_NOTICE(fmt, args...) rtd_pr_amp_notice(fmt, ## args)
#define AMP_WARN(fmt, args...)  rtd_pr_amp_warn(fmt, ## args)
#define AMP_ERR(fmt, args...)   rtd_pr_amp_err(fmt, ## args)
#else
#include <printf.h>
#include "rtk_kdriver/i2c-rtk-api.h"
#define AMP_INFO(fmt, args...)
#define AMP_NOTICE(fmt, args...) printf2("[AMP]"fmt, ## args)
#define AMP_WARN(fmt, args...)
#define AMP_ERR(fmt, args...)   printf2("[AMP]"fmt, ## args)
#endif


#define AMP_ADDR_NULL 0
#define AMP_SEL_NULL 0

#define AMP_MUTE_ON     (1)
#define AMP_MUTE_OFF    (0)

struct amp_controller {
    int amp_i2c_id;
    int sel_index;
    int slave_addr;
    int addr_size;
    int data_size;
    char name[32];
    void (*ops)(int amp_i2c_id, unsigned short slave_addr);
    int (*param_get)(unsigned char *reg, int amp_i2c_id, unsigned short slave_addr);
    int (*param_set)(unsigned char *data_wr, int amp_i2c_id, unsigned short slave_addr);
    int (*mute_set)(int on_off, int amp_i2c_id, unsigned short slave_addr);
    int (*dump_all)(const char *buf, int amp_i2c_id, unsigned short slave_addr);
    void (*amp_reset)(void);
    void (*amp_device_suspend)(int);
};

int rtk_amp_pin_set(unsigned char *pcbname,unsigned char val);
int rtk_amp_i2c_id(void);
int amp_get_addr(char *addr_name);
#endif
