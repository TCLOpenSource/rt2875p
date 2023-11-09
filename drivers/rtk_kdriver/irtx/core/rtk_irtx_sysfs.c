#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/sysfs.h>
#include "rtk_irtx.h"
#include "rtk_irtx_dev.h"

static struct attribute * irtx_attrs[] =
{
    NULL,
};

static struct attribute_group irtx_attr_group =
{
    .attrs = irtx_attrs,
};

int __init rtk_irtx_add_sysfs(struct kobject *kobj)
{
    int ret = sysfs_create_group(kobj, &irtx_attr_group);
    return ret;
}


void __exit rtk_irtx_remove_sysfs(struct kobject *kobj)
{
    sysfs_remove_group(kobj, &irtx_attr_group);
}

