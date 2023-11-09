#include <linux/sysfs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <asm/io.h>
#include <rtk_kdriver/rtk_ddc_lib.h>
#include <linux/version.h>
#include "rtk_ddc_sysfs.h"
#include "rtk_ddc_priv.h"
#include "rtk_ddc_dbg.h"

static ssize_t rtd_ddc_show(
    struct class*           pclass,
    struct class_attribute* attr,
    char*                   buf
    )
{
    srtk_ddc_chip* pchip = container_of(pclass, srtk_ddc_chip, class);

    RTK_DDC_ERR("attr->name=%s, pchip->id=%d\n", attr->attr.name, pchip->id);

    if (strcmp(attr->attr.name, "edid")==0)
    {
        rtk_ddc_lib_read_edid(pchip->id, buf, (pchip->ddc_type == DDC_VGA) ? 128 : 256);
        return 256;
    }
    else if (strcmp(attr->attr.name, "read_delay")==0)
    {
        return sprintf(buf,"%d\n", rtk_ddc_drv_get_read_delay(pchip->id));
    }
    else if (strcmp(attr->attr.name, "write_debounce")==0)
    {
        return sprintf(buf,"%d\n", rtk_ddc_drv_get_write_debounce(pchip->id));
    }
    else if (strcmp(attr->attr.name, "enable")==0)
    {
        return sprintf(buf,"%d\n", rtk_ddc_drv_is_enable(pchip->id));
    }
    else if (strcmp(attr->attr.name, "SGID_Enable")==0)
    {
        return sprintf(buf,"%d\n", rtk_ddc_drv_get_segen(pchip->id));
    }

    else
        return -1;

    return 0;
}


static ssize_t rtd_ddc_store(
    struct class*               pclass,
    struct class_attribute*     attr,
    const char*                 buf,
    size_t                      count
    )
{
    srtk_ddc_chip* pchip = container_of(pclass, srtk_ddc_chip, class);
    int val;

    //RTK_DDC_ERR("attr->name=%s, pchip->id=%d\n", attr->attr.name, pchip->id);

    if (strcmp(attr->attr.name, "edid")==0)
    {
        if (pchip->ddc_type == DDC_VGA)
        {            
            if (count!=128)
            {
                RTK_DDC_ERR("invalid edid size (%zd), EDID size for VGA should be 128\n", count);
                return -EINVAL;
            }                            
        }
        else
        {
            if (count!=128 &&count!=256 &&count!=384 &&count!=512)
            {
                RTK_DDC_ERR("invalid edid size (%zd), EDID size should be 128 ,256, 384 or 512\n", count);
                return -EINVAL;
            }
        }        
            
        rtk_ddc_lib_write_edid(pchip->id, (unsigned char*)buf, count);                                                            
    }
    else if (strcmp(attr->attr.name, "read_delay")==0)
    {
        if (sscanf(buf, "%d", &val)!=1 || (val<0) || (val >7))
        {
            RTK_DDC_ERR("invalid delay - %s, read delay shoulb be 0 ~ 7\n", buf);
            return -EINVAL;
        }

        rtk_ddc_drv_set_read_delay(pchip->id, val);
    }
    else if (strcmp(attr->attr.name, "write_debounce")==0)
    {
        if (sscanf(buf, "%d", &val)!=1 || (val<=0) || (val >3))
        {
            RTK_DDC_ERR("invalid debounce value (%s), write debounce shoulb be 1 ~ 3\n", buf);
            return -EINVAL;
        }

        rtk_ddc_drv_set_write_debounce(pchip->id, val);
    }
    else if (strcmp(attr->attr.name, "enable")==0)
    {
        if (sscanf(buf, "%d", &val)!=1 || (val<0) || (val>1))
        {
            RTK_DDC_ERR("invalid enable value (%s), enable be 0 or 1\n", buf);
            return -EINVAL;
        }

        rtk_ddc_drv_set_enable(pchip->id, val);
        return count;
    }
    else if (strcmp(attr->attr.name, "SGID_Enable")==0)
    {
        if (sscanf(buf, "%d", &val)!=1 || (val<0) || (val>1))
        {
            RTK_DDC_ERR("invalid enable value (%s), SGID_Enable be 0 or 1\n", buf);
            return -EINVAL;
        }

        rtk_ddc_drv_set_segen(pchip->id, val);
        rtk_ddc_drv_set_isr(pchip->id, val);

        return count;
    }
    else
    {
        return -1;
    }

    return count;
}


static struct class_attribute rtk_ddc_sysfs_attrs[] =
{
    __ATTR(edid,            S_IRUGO|S_IWUSR,    rtd_ddc_show,   rtd_ddc_store),
    __ATTR(read_delay,      S_IRUGO|S_IWUSR,    rtd_ddc_show,   rtd_ddc_store),
    __ATTR(write_debounce,  S_IRUGO|S_IWUSR,    rtd_ddc_show,   rtd_ddc_store),
    __ATTR(enable,          S_IRUGO|S_IWUSR,    rtd_ddc_show,   rtd_ddc_store),
    __ATTR(SGID_Enable,     S_IRUGO|S_IWUSR,    rtd_ddc_show,   rtd_ddc_store),
    __ATTR_NULL,
};


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))    

static struct attribute *class_rtk_ddc_attrs[] = {
	&rtk_ddc_sysfs_attrs[0].attr,
	&rtk_ddc_sysfs_attrs[1].attr,
	&rtk_ddc_sysfs_attrs[2].attr,
	&rtk_ddc_sysfs_attrs[3].attr,
    &rtk_ddc_sysfs_attrs[4].attr,
	NULL,	
};

ATTRIBUTE_GROUPS(class_rtk_ddc);

#endif


int rtk_ddc_sysfs_create(srtk_ddc_chip* pchip)
{
    int ret  = 0;

    if(NULL == pchip)
    {
        RTK_DDC_ERR("fail to init ddc_drv sysfs\n");
        return -EINVAL;
    }
    pchip->is_fs_registered     = 0;
    pchip->class.name           = pchip->name;
    pchip->class.owner          = THIS_MODULE;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))    
    pchip->class.class_groups   = class_rtk_ddc_groups;
#else
    pchip->class.class_attrs    = rtk_ddc_sysfs_attrs;
#endif    

    ret = class_register(&pchip->class);
    if(ret)
    {
        RTK_DDC_ERR("fail to register rtk_ddc class\n");
        return ret;
    }
    pchip->is_fs_registered = 1;

    return ret;
}

int rtk_ddc_sysfs_destroy(srtk_ddc_chip* pchip)
{
    if(NULL != pchip && pchip->is_fs_registered)
        class_unregister(&pchip->class);
    return 0;
}
