#ifndef __RTK_DDCCI_SYSFS_H__
#define __RTK_DDCCI_SYSFS_H__
#include "rtk_ddcci_priv.h"

extern int rtk_ddcci_dbg_thread(void *arg);

int rtk_ddcci_sysfs_create(struct device *dev);

#endif // __RTK_DDCCI_SYSFS_H__
