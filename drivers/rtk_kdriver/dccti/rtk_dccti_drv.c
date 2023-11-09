#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/pageremap.h>
#include <linux/migrate.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <rtk_kdriver/md/rtk_md.h>
#include "rtk_dccti.h"
#include "rtk_dccti_app.h"
#include <linux/semaphore.h>

static rtk_dccti_chip g_dccti_chip;
static rtk_dccti_info g_dccti_info;
static DEFINE_MUTEX(rtk_dccti_lock);

static int rtk_dccti_read_mem(rtk_dccti_info* pinfo)
{
    int i                   = 0;
    size_t size             = 0;
    unsigned int offset     = 0;
    unsigned char *vaddr    = 0;
    if(NULL == pinfo)
    {
        DCCTI_ERR("pInfo is null\n");
        return -1;
    }

    offset = pinfo->phy_addr & 0xfff;
    DCCTI_DBG("rtk_dccti_read_mem phy_addr=0x%8lx offset=0x%x data_len=%u\n"
            ,pinfo->phy_addr
            ,offset
            ,pinfo->data_len);
    size = PAGE_ALIGN(pinfo->data_len);
    vaddr = (unsigned char *) dvr_remap_uncached_memory(pinfo->phy_addr, 2*size,__builtin_return_address(0));
    memcpy((void*)pinfo->data,(void*)(vaddr+offset),pinfo->data_len);
    if(g_dccti_chip.dbg_en)
    {
//        unsigned int* pdata =(unsigned int*) pinfo->data;
        DCCTI_DBG("%s\n",__func__);
//        if(memcmp(pinfo->data,vaddr,pinfo->data_len))
        {
            for(i=0;i<pinfo->data_len/4;i++)
                DCCTI_DBG("read[0x%lx] d=0x%x o=0x%x %d\n"
                            ,pinfo->phy_addr+4*i
                            ,pinfo->data[i]
                            ,vaddr[i+offset]
                            ,vaddr[i+offset]==pinfo->data[i]);
//            for(i=0;i<pinfo->data_len/4;i++)
//                DCCTI_DBG("read[0x%lx] d=0x%x o=0x%x\n"
//                            ,pinfo->phy_addr+4*i
//                            ,pdata[i]
//                            ,i);
        }
//        else
//            DCCTI_DBG("[%s]read data is the same\n",__func__);
    }
    dvr_unmap_memory(vaddr,size);
    return 0;
}

static int rtk_dccti_write_mem(rtk_dccti_info* pinfo)
{
    int i = 0;
    size_t size         = 0;
    unsigned int offset  = 0;
    unsigned char *vaddr = 0;

    if(NULL == pinfo)
    {
        DCCTI_ERR("pInfo is null\n");
        return -1;
    }

    offset = pinfo->phy_addr & 0xfff;
    DCCTI_DBG("rtk_dccti_write_mem phy_addr=0x%8lx offset=0x%x data_len=%u\n"
               ,pinfo->phy_addr,offset,pinfo->data_len);

    size = PAGE_ALIGN(pinfo->data_len);
    vaddr = (unsigned char *) dvr_remap_uncached_memory(pinfo->phy_addr, 2*size,__builtin_return_address(0));
    memcpy((void*)(vaddr+offset),(void*)pinfo->data,pinfo->data_len);
    if(g_dccti_chip.dbg_en)
    {
        DCCTI_DBG("%s\n",__func__);
        if(memcmp(pinfo->data,(vaddr+offset),pinfo->data_len))
            for(i=0;i<pinfo->data_len;i++)
            {
                DCCTI_DBG("write[%d] i=0x%u w=0x%u %d\n"
                        ,i
                        ,pinfo->data[i]
                        ,vaddr[i+offset]
                        ,(vaddr[i+offset]==pinfo->data[i]));
            }
        else
            DCCTI_DBG("[%s]write data is the same\n",__func__);
    }
    dvr_unmap_memory(vaddr,size);
    return 0;
}

static int rtk_dccti_mem_read_test(unsigned long phy_addr, size_t size)
{
    unsigned int *vaddr = 0;
    unsigned int *vaddr2 = 0;
    unsigned long phy_addr_end = phy_addr + 0x100000;
    size_t size2 = PAGE_ALIGN(size);

    int i = 0;
    unsigned int tmp = 0;
    DCCTI_DBG("mem_read_test phy=0x%lx end=0x%lx size=%u\n",phy_addr,phy_addr_end,size);
    vaddr = (unsigned int *) dvr_remap_uncached_memory(phy_addr, size2,__builtin_return_address(0));
    {
        memset(vaddr,0x55,size2);
        for(i=0;i<size/4;i++)
        {
            tmp = *(vaddr+i);
            DCCTI_DBG("mem_read_test 0 [%d]=0x%x\n",i,tmp);
        }
    }

    vaddr2 = (unsigned int *) dvr_remap_uncached_memory(phy_addr_end, size2,__builtin_return_address(0));
    {
        smd_memcpy(phy_addr_end,phy_addr,size2);
        smd_checkComplete();
        for(i=0;i<size/4;i++)
        {
            tmp = *(vaddr2+i);
            DCCTI_DBG("mem_read_test2 [%d]=0x%x\n",i,tmp);
        }
    }
    for(i=0;i<size/4;i++)
    {
        DCCTI_DBG("mem_read_test [%d]=[1]0x%x [2]0x%x [=]%d\n"
                ,i
                ,*(vaddr+i)
                ,*(vaddr2+i)
                ,(*(vaddr+i))==(*(vaddr2+i)));
    }
    dvr_unmap_memory((void*)vaddr,size2);
    dvr_unmap_memory((void*)vaddr2,size2);
    return 0;
}

static int rtk_dccti_mem_write_test(unsigned long phy_addr, size_t size)
{
    unsigned int *vaddr = 0;
    size_t size2        = PAGE_ALIGN(size);
    {
        int i = 0;
        unsigned int tmp = 0;
        //paddr = pli_malloc(size, GFP_DCU2_FIRST);
        DCCTI_DBG("mem_test phy=%lx size=%u\n",phy_addr,size);
        vaddr = (unsigned int *) dvr_remap_uncached_memory(phy_addr, size2,__builtin_return_address(0));
        for(i=0;i<size/4;i++)
        {
            tmp = *(vaddr+i);
            DCCTI_DBG("mem_test_write0 [%d]=0x%x\n",i,tmp);
        }

//         pli_free(paddr);
    }
    //md
    {
        smd_memfill(phy_addr,size,0xaa);
    }

    {
        int i = 0;
        unsigned int tmp = 0;
        for(i=0;i<size/4;i++)
        {
            tmp = *(vaddr+i);
            DCCTI_DBG("mem_test_write1 [%d]=0x%x\n",i,tmp);
        }
    }
    dvr_unmap_memory((void*)vaddr,size2);
    return 0;
}

static int rtk_dccti_mem_test(unsigned long phy_addr, size_t size)
{
    unsigned int *vaddr = 0;
    size_t size2        = PAGE_ALIGN(size);
    {
        int i = 0;
        unsigned int tmp = 0;
        DCCTI_DBG("mem_test phy=%lx size=%u size2=%u\n",phy_addr,size,size2);
        vaddr = (unsigned int *) dvr_remap_uncached_memory(phy_addr, size2,__builtin_return_address(0));
        for(i=0;i<size2/4;i++)
        {
            tmp = *(vaddr+i) = phy_addr+4*i;
            DCCTI_DBG("mem_test0 [%d]=0x%x\n",i,tmp);
        }
        dvr_unmap_memory((void*)vaddr,size2);
    }

    return 0;
}

static ssize_t rtk_dccti_mem_show(struct class* pclass,struct class_attribute* attr,char* buf)
{
    return snprintf(buf, PAGE_SIZE, "1. echo m [phy_addr] [size] [w/r/m] > mem\n"
                                     "2. echo w [phy_addr] [size] > mem\n"
                                     "3. echo r [phy_addr] [size] > mem\n"
                                     " PS.\n"
                                     " phy_addr=[0x02800000]\n"
                                     " size < 2048\n");
}

static ssize_t rtk_dccti_mem_store(struct class* pclass,struct class_attribute* attr,const char* buf,size_t count)
{
    //rtk_dccti_chip* pchip = container_of(pclass,rtk_dccti_chip, class);
    const char * const d = " ";
    char* substr = NULL;
    char* cur    = (char*)buf;
    unsigned int val = 0;
    unsigned int size= 0;
    //size_t size = PAGE_ALIGN(2048);

    substr = strsep(&cur,d);

    DCCTI_DBG("%s %s size=%u\n",__func__,substr,size);
    if(0 == strncmp(substr,"m",1))
    {
        substr = strsep(&cur,d);
        if(0==kstrtouint(substr,16,&val))
        {
            //0x02800000
            DCCTI_DBG("phy addr=0x%x\n",val);
            substr = strsep(&cur,d);
            if(0==kstrtouint(substr,16,&size))
            {
                DCCTI_DBG("size=0x%x\n",size);
                substr = strsep(&cur,d);

                if(0 == strncmp(substr,"w",1))
                    rtk_dccti_mem_write_test(val,size);
                else if(0 == strncmp(substr,"r",1))
                    rtk_dccti_mem_read_test(val,size);
                else if(0 == strncmp(substr,"m",1))
                    rtk_dccti_mem_test(val,size);
            }
        }
    }
    else if(0 == strncmp(substr,"r",1))
    {
        substr = strsep(&cur,d);
        if (0==kstrtouint(substr, 16, &val))
        {
            DCCTI_DBG("phy addr=0x%x\n",val);
            g_dccti_info.phy_addr = val;
            substr = strsep(&cur,d);
            if (0==kstrtouint(substr, 10, &val))
            {
                int i = 0;
                unsigned int* pdata = NULL;
                DCCTI_DBG("size=%u\n",val);
                g_dccti_info.data_len = val;
                rtk_dccti_read_mem(&g_dccti_info);
                pdata = (unsigned int*)g_dccti_info.data;
                for(i=0;i<g_dccti_info.data_len/4;i++)
                    DCCTI_DBG("read %d phy0x%lx= 0x%x\n",i,g_dccti_info.phy_addr+4*i,pdata[i]);
                return count;
            }
        }
    }
    else if(0 == strncmp(substr,"w",1))
    {
        substr = strsep(&cur,d);
        if (0==kstrtouint(substr, 16, &val))
        {
            DCCTI_DBG("phy addr=0x%x\n",val);
            g_dccti_info.phy_addr = val;
            substr = strsep(&cur,d);
            if (0==kstrtouint(substr, 10, &val))
            {
                DCCTI_DBG("size=%u\n",val);
                g_dccti_info.data_len = val;
                memset(g_dccti_info.data,0x22,g_dccti_info.data_len);
                rtk_dccti_write_mem(&g_dccti_info);
                return count;
            }
        }
    }

    return count;
}

static ssize_t rtk_dccti_dbg_en_show(struct class* pclass,struct class_attribute* attr,char* buf)
{
    rtk_dccti_chip* pchip = container_of(pclass,struct rtk_dccti_chip, dccti_class);
    return snprintf(buf, PAGE_SIZE, "dbg_en=%d\n", pchip->dbg_en);
}

static ssize_t rtk_dccti_dbg_en_store(struct class* pclass,struct class_attribute* attr,const char* buf,size_t count)
{
    unsigned int val = 0;
    rtk_dccti_chip* pchip = container_of(pclass,struct rtk_dccti_chip, dccti_class);

     if (kstrtouint(buf, 10, &val) == 0) {
         pchip->dbg_en = val ? 1 : 0;
         return count;
     }
    return -EINVAL;
}

static struct class_attribute rtk_dccti_class_attrs[] =
{
    __ATTR(mem,     S_IRUGO|S_IWUSR,    rtk_dccti_mem_show,    rtk_dccti_mem_store),
    __ATTR(dbg_en,  S_IRUGO|S_IWUSR,    rtk_dccti_dbg_en_show, rtk_dccti_dbg_en_store),
    __ATTR_NULL,
};

static struct attribute *class_dccti_attrs[] = {
    &rtk_dccti_class_attrs[0].attr,
    &rtk_dccti_class_attrs[1].attr,
    NULL,
};
ATTRIBUTE_GROUPS(class_dccti);

static int rtk_dccti_create_class(void)
{
    int ret = 0;
    g_dccti_chip.dccti_class.name          = g_dccti_chip.name;
    g_dccti_chip.dccti_class.owner         = THIS_MODULE;
    g_dccti_chip.dccti_class.class_groups  = class_dccti_groups;

    ret = class_register(&g_dccti_chip.dccti_class);
    if(ret)
    {
        DCCTI_ERR("fail to register rtk_dccti_class\n");
        return ret;
    }
    return 0;
}

static int rtk_dccti_open(struct inode *pinode, struct file *pfile)
{
    DCCTI_DBG("%s\n",__func__);
    return 0;
}

static int rtk_dccti_release(struct inode *pinode, struct file *pfile)
{
    DCCTI_DBG("%s\n",__func__);
    return 0;
}

static long rtk_dccti_ioctl(struct file *pfile, unsigned int cmd, unsigned long args)
{
    int err = 0;

    if(_IOC_TYPE(cmd)!=RTK_DCCTI_DEV_TYPE)
    {
        DCCTI_ERR("[IOCTL] dev type is wrong\n");
        return -EINVAL;
    }

    if(_IOC_NR(cmd) > RTK_DCCTI_MAX_NR)
    {
        DCCTI_ERR("[IOCTL] cmd nr is wrong\n");
        return -EINVAL;
    }
    if(_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE,(void __user*)args,_IOC_SIZE(cmd));
    else if(_IOC_DIR(cmd) & _IOC_WRITE)
        err = !access_ok(VERIFY_READ,(void __user*)args,_IOC_SIZE(cmd));
    if(err)
    {
        DCCTI_ERR("[IOCTL] DIR/Read/Write is wrong\n");
        return -EFAULT;
    }
    //roger
    //memset(&g_dccti_info,0,sizeof(rtk_dccti_info));
    memset(&g_dccti_info,0xff,sizeof(rtk_dccti_info));
    mutex_lock(&rtk_dccti_lock);

    switch(cmd)
    {
    case RTK_DCCTI_IOCTL_MEM_READ:
        if(copy_from_user(&g_dccti_info,(rtk_dccti_info __user *)args,sizeof(rtk_dccti_info)))
        {
            DCCTI_ERR("[IOCTL] fail to copy from user\n");
            return -EFAULT;
        }
        if(RTK_DCCTI_MAX_DATA_LEN < g_dccti_info.data_len)
            DCCTI_ERR("[IOCTL] read data len is too large %u\n",g_dccti_info.data_len);
        else
        {
            rtk_dccti_read_mem(&g_dccti_info);
            if(copy_to_user((rtk_dccti_info __user*)args,&g_dccti_info,sizeof(rtk_dccti_info)))
            {
                DCCTI_ERR("[IOCTL] fail to copy to user\n");
                return -EFAULT;
            }
        }
        break;

    case RTK_DCCTI_IOCTL_MEM_WRITE:
        if(copy_from_user(&g_dccti_info,(rtk_dccti_info __user *)args,sizeof(rtk_dccti_info)))
        {
            DCCTI_ERR("[IOCTL] fail to copy from user\n");
            return -EFAULT;
        }
        if(RTK_DCCTI_MAX_DATA_LEN < g_dccti_info.data_len)
            DCCTI_ERR("[IOCTL] write data len is too large %u\n",g_dccti_info.data_len);
        else
            rtk_dccti_write_mem(&g_dccti_info);
        break;
    }

    mutex_unlock(&rtk_dccti_lock);
    return 0;
}

static struct file_operations dccti_ops = {
        .owner          = THIS_MODULE,
        .open           = rtk_dccti_open,
        .release        = rtk_dccti_release,
        .unlocked_ioctl = rtk_dccti_ioctl,
};

static __init int rtk_ddci_drv_init(void)
{
    int ret = 0;
    DCCTI_ERR("ddci_dev_init\n");
    g_dccti_chip.dbg_en = DCCTI_DEBUG;
    strncpy(g_dccti_chip.name,DDCTI_DRV_NAME,strlen(DDCTI_DRV_NAME));
    proc_create(g_dccti_chip.name, 0, NULL, &dccti_ops);

    ret = rtk_dccti_create_class();
    return ret;
}

static __exit void rtk_ddci_drv_exit(void)
{}

module_init(rtk_ddci_drv_init);
module_exit(rtk_ddci_drv_exit);

MODULE_DESCRIPTION("Realtek DDCTI");
MODULE_AUTHOR("Roger Kan <roger.kan@realtek.com>");
MODULE_LICENSE("GPL");
