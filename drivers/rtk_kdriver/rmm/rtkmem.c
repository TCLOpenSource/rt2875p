
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#include <linux/pageremap.h>
#include <mach/rtk_platform.h>
#include <rtd_log/rtd_module_log.h>

#define RTKMEM_NAME			"rtkmem"

#define MEM_IOCQALLOC           _IO('k', 0x20)
#define MEM_IOCQFREE            _IO('k', 0x21)

//for user space query
struct rtkmem_query_address {
    __u32 fd;
    __u64 address;
    __u64 stamp_id;
    __u32 reserved1;
    __u32 reserved2;
};

extern phys_addr_t ion_to_phys(int fd);

#define RTKMEM_IOC_QUERY_PHY   _IOR('k', 0x30, struct rtkmem_query_address)
#define RTKMEM_IOC_QUERY_PHY_DMAHEAP   _IOR('k', 0x31, struct rtkmem_query_address)


#if IS_ENABLED(CONFIG_DMABUF_HEAPS_DVR)
extern phys_addr_t dmaheap_to_phys(int fd);
extern uint64_t dmaheap_get_stamp(int fd);


#endif //



struct mem_record_head {
	struct list_head        list;
	struct mutex            mutex;
};

struct mem_record_node {
	struct list_head        list;
	unsigned long           addr;
};

static struct class *rtkmem_class;
int rtkmem_major = 0; // dynamic
int rtkmem_minor = 0;

static const struct vm_operations_struct rtkmem_physical_vm_ops = {
#ifdef CONFIG_HAVE_IOREMAP_PROT
	.access = generic_access_phys,
#endif
};

static int mmap_rtkmem(struct file *file, struct vm_area_struct *vma)
{
	size_t size = vma->vm_end - vma->vm_start;
	unsigned long start = 0, end = 0;
	int idx;
	int pa_query_map_ok = 0;

	if ( (idx = carvedout_buf_query_is_in_range(PFN_PHYS(vma->vm_pgoff), (void **)&start, (void **)&end)) >= 0 ) {
		if (size <= (end - start))
		pa_query_map_ok |= 1;
	}

	if (check_cma_memory(vma->vm_pgoff)) {
		if (check_cma_memory(PFN_DOWN(PFN_PHYS(vma->vm_pgoff - 1) + size)))
		pa_query_map_ok |= 2;
	}

	if (!pa_query_map_ok) {
		rtd_pr_rmm_err("(%s) mmap pa(%lx/%x) flag(%x) in not allow range\n", current->comm, PFN_PHYS(vma->vm_pgoff), size, pa_query_map_ok);
		return -EPERM;
	} else {
		rtd_pr_rmm_debug("(%s) mmap pa(%lx/%x) flag(%x) OK range\n", current->comm, PFN_PHYS(vma->vm_pgoff), size, pa_query_map_ok);
	}

//	if (!valid_mmap_phys_addr_range(vma->vm_pgoff, size))
//		return -EINVAL;

//	if (!private_mapping_ok(vma))
//		return -ENOSYS;

//	if (!range_is_allowed(vma->vm_pgoff, size))
//		return -EPERM;

//	if (!phys_mem_access_prot_allowed(file, vma->vm_pgoff, size,
//						&vma->vm_page_prot))
//		return -EINVAL;

	if (file->f_flags & O_DSYNC)
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	vma->vm_ops = &rtkmem_physical_vm_ops;

	/* Remap-pfn-range will mark the range VM_IO */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    vma->vm_pgoff,
			    size,
			    vma->vm_page_prot)) {
		return -EAGAIN;
	}

//	if (pfn_valid(vma->vm_pgoff))
//		vma->vm_flags = (vma->vm_flags | VM_PLI);

	return 0;
}

static int open_rtkmem(struct inode * inode, struct file * filp)
{
	return 0;
}

static int release_rtkmem(struct inode * inode, struct file * filp)
{
	if (filp->private_data != NULL) {
		struct mem_record_node *rec, *tmp;
		struct list_head *plist;

		plist = &((struct mem_record_head *)filp->private_data)->list;
		list_for_each_entry_safe(rec, tmp, plist, list) {
//			rtd_pr_rmm_info("*** remove2: %lx \n", rec->addr);
			dvr_free_page(pfn_to_page(rec->addr));
			list_del(&rec->list);
			kfree(rec);
		}

		kfree(filp->private_data);
		filp->private_data = NULL;
	}
	return 0;
}

static long ioctl_rtkmem(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long ret = 0;

	switch (cmd) {
	case MEM_IOCQALLOC:
	{
		struct page *page;
		struct mem_record_node *record;

		page = dvr_malloc_page(arg, 0);
		if (page != 0)
			ret = page_to_pfn(page) << PAGE_SHIFT;
		else
			return -ENOMEM;

		if (filp->private_data == NULL) {
			struct mem_record_head *head;
			head = kmalloc(sizeof(struct mem_record_head), GFP_KERNEL);
			INIT_LIST_HEAD(&head->list);
			mutex_init(&head->mutex);
			filp->private_data = head;
		}

		if (mutex_lock_interruptible(&((struct mem_record_head *)filp->private_data)->mutex)) {
			// we use return value 1 to represent an signal is caught...
			rtd_pr_rmm_info("mem: interrupted by an signal...\n");
			dvr_free_page(page);
			return -EAGAIN;
		}
		record = kmalloc(sizeof(struct mem_record_node), GFP_KERNEL);
		INIT_LIST_HEAD(&record->list);
		record->addr = page_to_pfn(page);
		list_add(&record->list, &((struct mem_record_head *)filp->private_data)->list);
		mutex_unlock(&((struct mem_record_head *)filp->private_data)->mutex);
	}
	break;

	case MEM_IOCQFREE:
	{
		if (filp->private_data != NULL) {
			int flag = 0;
			struct mem_record_node *rec, *tmp;
			struct list_head *plist;

			if (mutex_lock_interruptible(&((struct mem_record_head *)filp->private_data)->mutex)) {
				// we use return value 1 to represent an signal is caught...
				rtd_pr_rmm_info("mem: interrupted by an signal...\n");
				return -EAGAIN;
			}
			plist = &((struct mem_record_head *)filp->private_data)->list;
			list_for_each_entry_safe(rec, tmp, plist, list)
				if (rec->addr == (arg >> PAGE_SHIFT)) {
//					rtd_pr_rmm_info("*** remove1: %lx \n", rec->addr);
					list_del(&rec->list);
					kfree(rec);
					flag = 1;
					break;
				}
			mutex_unlock(&((struct mem_record_head *)filp->private_data)->mutex);

			if (flag) {
				dvr_free_page(pfn_to_page(arg >> PAGE_SHIFT));
			} else {
				rtd_pr_rmm_info("mem: invalid parameter %lu in MEM_IOCQFREE...\n", arg);
				ret = -EINVAL;
			}
		}
	}
	break;

	case RTKMEM_IOC_QUERY_PHY:
			{
				struct rtkmem_query_address query_info;
				phys_addr_t addr = 0;
				
				if (copy_from_user(&query_info, (struct rtkmem_query_address *)arg, sizeof(query_info))) {
					rtd_pr_rmm_err("IOC_QUERY_PHY fail 0\n");
					return -EFAULT;
				}
	
				addr =	ion_to_phys( query_info.fd);
	
				query_info.address = (u64)addr;
	
				if( addr == 0 || addr == INVALID_VAL )
				{
					query_info.address = 0;
					rtd_pr_rmm_err("rtkmem query phy fail %d addr %pa\n", query_info.fd, &addr );
	
					return -EINVAL;
				}
	
				if (copy_to_user((void *)arg, &(query_info), sizeof(query_info)) != 0) {
					rtd_pr_rmm_err("IOC_QUERY_PHY fail 1\n");
					return -EFAULT;
				}
				
			}
	
			break;
			
	#if IS_ENABLED(CONFIG_DMABUF_HEAPS_DVR)


	case RTKMEM_IOC_QUERY_PHY_DMAHEAP:
	{
		
		struct rtkmem_query_address query_info;
		phys_addr_t addr = 0;
		
		if (copy_from_user(&query_info, (struct rtkmem_query_address *)arg, sizeof(query_info))) {
		
			return -EFAULT;
		}

		addr = 	dmaheap_to_phys( query_info.fd);

		query_info.stamp_id = dmaheap_get_stamp( query_info.fd );
			
		query_info.address = (u64)addr;

		if( addr == 0 || addr == INVALID_VAL )
		{
			query_info.address = 0;
			rtd_pr_rmm_err("rtkmem query dmaH phy fail %d addr %pa\n", query_info.fd, &addr );

			return -EINVAL;
		}

		if (copy_to_user((void *)arg, &(query_info), sizeof(query_info)) != 0) {
			rtd_pr_rmm_err("rtkmem query copy_to_user fail %d addr %pa\n", query_info.fd, &addr );
            return -EFAULT;
		}

	}
		break;


	#endif// CONFIG_DMABUF_HEAPS_DVR

	default:
		rtd_pr_rmm_err("rtkmem invalid cmd :%d  others:%d %d\n", cmd, RTKMEM_IOC_QUERY_PHY, RTKMEM_IOC_QUERY_PHY_DMAHEAP);
		return -EINVAL;

	}
	return ret;
}

static char *rtkmem_devnode(struct device *dev, umode_t *mode)
{
	if (mode)
		*mode = 0660;
	return NULL;
}

static const struct file_operations rtkmem_fops = {
//	.llseek         = memory_lseek,
//	.read           = read_mem,
//	.write          = write_mem,
	.mmap           = mmap_rtkmem,
	.open           = open_rtkmem,
	.release        = release_rtkmem,
	.unlocked_ioctl = ioctl_rtkmem,
	.compat_ioctl   = ioctl_rtkmem,
};

#if 0//def CONFIG_SYSFS
#define RMM_ATTR_RO(_name)                    \
    static struct kobj_attribute rmm_##_name##_attr = \
        __ATTR_RO(_name)

#ifndef __ATTR_WO
#define __ATTR_WO(_name) { \
    .attr   = { .name = __stringify(_name), .mode = S_IWUSR },  \
		.store  = _name##_store,                    \
		 }
#endif

#define RMM_ATTR_WO(_name)                    \
    static struct kobj_attribute rmm_##_name##_attr = \
        __ATTR_WO(_name)

#define RMM_ATTR(_name)                   \
    static struct kobj_attribute rmm_##_name##_attr = \
        __ATTR(_name, 0644, _name##_show, _name##_store)

/* usage: 
 * cat /sys/kernel/mm/rmm/cma_dump 
 */
static ssize_t cma_dump_show(struct kobject *kobj,
							 struct kobj_attribute *attr, char *buf)
{
	size_t count = 0;

	count = sprintf(buf, "%s\n", "rmm: cma list dump");
	rtk_record_list_dump();

	return count;
}
RMM_ATTR_RO(cma_dump);

static struct attribute *rmm_attrs[] = {
    &rmm_cma_dump_attr.attr,
    NULL,
};

static struct attribute_group rmm_attr_group = {
    .attrs = rmm_attrs,
    .name = "rmm",
};
#endif

int __init rtkmem_init(void) {
	int result;

	result = register_chrdev(rtkmem_major, RTKMEM_NAME, &rtkmem_fops);
	if (result < 0) {
		rtd_pr_rmm_err("rtkmem: can not register (%d %d)...\n", rtkmem_major, rtkmem_minor);
		return -ENODEV;
	}

	if (rtkmem_major == 0)
		rtkmem_major = result;  // dynamic

	rtkmem_class = class_create(THIS_MODULE, RTKMEM_NAME);
	if (IS_ERR(rtkmem_class))
		return PTR_ERR(rtkmem_class);

	rtkmem_class->devnode = rtkmem_devnode;
	device_create(rtkmem_class, NULL, MKDEV(rtkmem_major, rtkmem_minor), NULL, RTKMEM_NAME);

	rtd_pr_rmm_info("rtkmem major number: %d\n", rtkmem_major);

#if 0//def CONFIG_SYSFS
	if (sysfs_create_group(mm_kobj, &rmm_attr_group))
		rtd_pr_rmm_err("rtkmem: cma_dump: register sysfs failed\n");
#endif
	return 0;
}

void __exit rtkmem_exit(void) {
#if 0//def CONFIG_SYSFS
	sysfs_remove_group(mm_kobj, &rmm_attr_group);
#endif
	device_destroy(rtkmem_class, MKDEV(rtkmem_major, rtkmem_minor));
	class_destroy(rtkmem_class);
	unregister_chrdev(rtkmem_major, RTKMEM_NAME);
}

#ifdef MODULE
/* Register Macros */
module_init(rtkmem_init);
module_exit(rtkmem_exit);
#else
fs_initcall(rtkmem_init);
#endif

MODULE_AUTHOR("Realtek.com");
MODULE_LICENSE("Dual BSD/GPL");
