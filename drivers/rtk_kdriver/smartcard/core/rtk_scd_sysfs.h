#ifndef __RTK_SCD_SYSFS_H__
#define __RTK_SCD_SYSFS_H__

#include <linux/device.h>

extern void rtk_scd_sysfs_create(struct device *device);
extern void rtk_scd_sysfs_destroy(struct device *device);

#endif /*__RTK_SCD_SYSFS_H__*/
