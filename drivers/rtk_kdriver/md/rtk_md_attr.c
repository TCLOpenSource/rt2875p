#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include "rtk_md_attr.h"
#include "rtk_md_test.h"
#include "rtk_md_hw.h"
#include "rtk_md_debug.h"
#include <rtk_kdriver/md/rtk_md.h>


extern struct md_dev *md_drv_dev;
bool md_dbg_onoff = false;
unsigned int md_cmd_buffer = 32;
static unsigned int md_len = 4096;
unsigned int md_test_loops = 100;

static void _md_dump_mem(unsigned char* p_data, unsigned int len, unsigned int phy_addr_base)
{
    int i;
    int n;
    char str_buff[512];
    char *ptr = str_buff;
    int count = sizeof(str_buff);

    memset(str_buff, 0, count);

    for (i = 0; i < len; i++) {
        if ((i & 0xF) == 0) {
            n = scnprintf(ptr, count, "\n%08x | ", phy_addr_base + i);
            ptr += n;
            count -= n;
        }

        n = scnprintf(ptr, count, "%02x ", p_data[i]);
        ptr += n;
        count -= n;

        if ((i & 0xF) == 0xF || i >= (len - 1)) {
            PDEERR("%s\n", str_buff);
            ptr = str_buff;
            count = sizeof(str_buff);
            memset(str_buff, 0, count);
        }
    }
}

void _do_md_dump_mem(unsigned int phyaddr, unsigned int len)
{
    unsigned int  buff_sz = PAGE_ALIGN(len);
    unsigned char* pbuff = kmalloc(buff_sz , GFP_KERNEL);
    dma_addr_t buff_addr;

    if (pbuff == NULL)
        return ;

    buff_addr = dma_map_single(NULL, pbuff, buff_sz, DMA_FROM_DEVICE);

    rtk_md_memcpy(buff_addr, phyaddr, len, 0, 1);

    dma_unmap_single(NULL, buff_addr, buff_sz, DMA_FROM_DEVICE);


    _md_dump_mem(pbuff, len, phyaddr);

    kfree(pbuff);
}

static ssize_t md_debug_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    if (strcmp(attr->attr.name, "md_dbg_onoff") == 0) {
        buf[0] = 0;
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "md_dbg_onoff = %d\n", md_dbg_onoff);
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "echo 1/0 > md_dbg_onoff  to enable or disable md_debug\n");
        return strlen(buf);
    } else if(strcmp(attr->attr.name, "md_test_loops") == 0) {
        buf[0] = 0;
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "md_test_loops = %d\n", md_test_loops);
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "echo loops > md_test_loops\n");
        return strlen(buf);
    } else if(strcmp(attr->attr.name, "md_len") == 0) {
        buf[0] = 0;
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "md_len = %d\n", md_len);
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "echo data_len > md_test_len\n");
        return strlen(buf);
    } else if(strcmp(attr->attr.name, "md_test_case") == 0) {
        buf[0] = 0;
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "echo 1 > md_test_case  to do md_unit_test\n");
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "echo 2 > md_test_case  to do rtk_md_stress_test\n");
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "echo 3 > md_test_case  to do rtk_md_fill_test\n");
#ifndef CONFIG_RTK_MD_USE_SW			
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "echo 4 > md_test_case  to do rtk_md_unit_test\n");
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "echo 5 > md_test_case  to do multi_vs_signal\n");
#endif		
        return strlen(buf);
    } 
#ifndef CONFIG_RTK_MD_USE_SW	
	else if (strcmp(attr->attr.name, "md_cmd_buffer") == 0) {
        buf[0] = 0;
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "md_cmd_buffer = %d\n", md_cmd_buffer);
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "echo len > md_cmd_buffer  to dump the latest <len> cmd\n");
        md_dump_cmd_ring(md_drv_dev, md_cmd_buffer, buf);
        return strlen(buf);
    } else if(strcmp(attr->attr.name, "md_reg_status") == 0) {
        buf[0] = 0;
        snprintf(buf + strlen(buf), PAGE_SIZE - strlen(buf), "md_reg_status\n");
        rtk_md_dump_reg(buf);
        return strlen(buf);
    }
#endif	
    return 0;
}

static ssize_t md_debug_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{

    static unsigned int md_val = 1;
    static unsigned int md_addr = 0;
    if(strcmp(attr->attr.name, "md_dbg_onoff") == 0) {
        int val;
        if (sscanf(buf, "%d", &val) == 1) {
            md_dbg_onoff = val;
            return n;
        }
    } else if(strcmp(attr->attr.name, "md_test_loops") == 0) {
        int loops;
        if (sscanf(buf, "%d", &loops) == 1) {
            md_test_loops = loops;
            return n;
        }
    } else if(strcmp(attr->attr.name, "md_len") == 0) {
        int len;
        if (sscanf(buf, "%d", &len) == 1) {
            md_len = len;
            return n;
        }
    } else if(strcmp(attr->attr.name, "md_test_case") == 0) {
        unsigned int md_test_case = 0;
        if (sscanf(buf, "%u", &md_test_case) == 1) {

            if(md_test_case == 1)
                rtk_md_memcpy_test(md_len);
            else if (md_test_case == 2)
                rtk_md_stress_test(md_len, md_test_loops);
            else if (md_test_case == 3)
                rtk_md_fill_test();
#ifndef CONFIG_RTK_MD_USE_SW			
            else if(md_test_case == 4)
                rtk_md_unit_test();
            else if(md_test_case == 5) {
                rtk_multi_vs_signal(md_len, md_test_loops);
            }
#endif
            return n;
        }
    } else if(strcmp(attr->attr.name, "md_cmd_buffer") == 0) {
        int val;
        if (sscanf(buf, "%u", &val) == 1) {
            md_cmd_buffer = val;
            return n;
        }
    } else if(strcmp(attr->attr.name, "md_addr") == 0) {
        unsigned int phyaddr;
        if (sscanf(buf, "%x", &phyaddr) == 1) {
            md_addr =    phyaddr;
            return n;
        }
    } else if(strcmp(attr->attr.name, "md_len") == 0) {
        unsigned int len;
        if (sscanf(buf, "%u", &len) == 1) {
            md_len = len;
            return n;
        }
    } else if(strcmp(attr->attr.name, "md_val") == 0) {
        unsigned int len;
        if (sscanf(buf, "%u", &len) == 1) {
            md_val = len;
            return n;
        }
    } else if(strcmp(attr->attr.name, "md_dbg_ctrl") == 0) {
        unsigned int md_dbg_ctrl = 0;
        if (sscanf(buf, "%u", &md_dbg_ctrl) == 1) {
            if(md_dbg_ctrl == 1) {
                _do_md_dump_mem(md_addr, md_len);
                return n;
            } else if(md_dbg_ctrl == 2) {
                smd_memfill(md_addr, md_len, (md_val & 0xFF));
                _do_md_dump_mem(md_addr, md_len);
                return n;
            }
        }
    }
    return -EINVAL;
}


#define md_attr(_name) \
static struct kobj_attribute _name##_attr = {   \
    .attr   = {                             \
    .name = __stringify(_name),     \
    .mode = 0644,                   \
    },                                      \
    .show   = md_debug_show,                        \
    .store  = md_debug_store,               \
}

md_attr(md_dbg_onoff);
md_attr(md_test_case);
md_attr(md_len);
md_attr(md_addr);
md_attr(md_val);
md_attr(md_test_loops);
md_attr(md_cmd_buffer);
md_attr(md_reg_status);
md_attr(md_dbg_ctrl);



static struct attribute * md_attrs[] = {
    &md_dbg_onoff_attr.attr,
    &md_test_case_attr.attr,
    &md_len_attr.attr,
    &md_test_loops_attr.attr,
    &md_cmd_buffer_attr.attr,
    &md_reg_status_attr.attr,
    &md_dbg_ctrl_attr.attr,
    &md_addr_attr.attr,
    &md_val_attr.attr,
    NULL,
};

struct attribute_group md_attr_group = {
    .attrs = md_attrs,
};

struct kobject *md_kobj;

int rtk_md_create_attr(void)
{
    int ret = 0;
    md_kobj = kobject_create_and_add("md", NULL);
    if(!md_kobj) {
        ret = -ENOMEM;
        goto FAIL_KOBJECT_CREATE;
    }
    ret = sysfs_create_group(md_kobj, &md_attr_group);
    if(ret != 0) {
        goto FAIL_SYSFS_CREATE_GROUP;
    }
    return ret;
FAIL_SYSFS_CREATE_GROUP:
    kobject_put(md_kobj);
FAIL_KOBJECT_CREATE:
    return ret;
}

void rtk_md_remove_attr(void)
{
    kobject_put(md_kobj);
}

