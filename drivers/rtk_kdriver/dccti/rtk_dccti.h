#ifndef __RTK_DCCTI_H__
#define __RTK_DCCTI_H__
#include <rtd_log/rtd_module_log.h>

#define DDCTI_DRV_NAME          "rtk_dccti"
#define DDCTI_TAG               "DDTCI"

#define DCCTI_DEBUG                 0

#if DCCTI_DEBUG
#define DCCTI_DBG(fmt, args...)     rtd_pr_ddc_ci_emerg("[ERR] " fmt, ## args)
#else
#define DCCTI_DBG(fmt, args...)     if(g_dccti_chip.dbg_en) rtd_pr_ddc_ci_emerg("[DBG] " fmt, ## args)
#endif
#define DCCTI_ERR(fmt, args...)     rtd_pr_ddc_ci_emerg("[ERR] " fmt, ## args)


struct rtk_dccti_chip{
    char name[16];
    int dbg_en;
    struct class    dccti_class;
};
typedef struct rtk_dccti_chip rtk_dccti_chip;

#endif
