#ifndef __RTK_IRTX_SYSFS_H__
#define __RTK_IRTX_SYSFS_H__
#include <linux/kobject.h>
extern int __init rtk_irtx_add_sysfs(struct kobject *kobj);
extern void __exit rtk_irtx_remove_sysfs(struct kobject *kobj);
#endif

