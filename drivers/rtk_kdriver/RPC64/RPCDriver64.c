/*
 * $Id: RPCDriver.c,v 1.10 2004/8/4 09:25 Jacky Exp $
 */
#include <generated/autoconf.h>
/*
#if defined(CONFIG_MODVERSIONS) && !defined(MODVERSIONS)
	#define MODVERSIONS
#endif

#ifdef MODVERSIONS
	#include <linux/modversions.h>
#endif

#ifndef MODULE
#define MODULE
#endif
*/

#include <linux/init.h>
#include <linux/module.h>
#include <rtd_log/rtd_module_log.h>	/* rtd_pr_rpc_info() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/ioctl.h>	/* needed for the _IOW etc stuff used later */
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <rtk_kdriver/RPCDriver.h>
#include <asm/cacheflush.h>
#include <rtk_kdriver/avcpu.h>
#include <linux/pageremap.h>
#include <linux/suspend.h>

#include <linux/sched.h>
#ifdef CONFIG_OF
#include <linux/of_irq.h>
#include <linux/of_address.h>
#endif

#include <asm/io.h>
//#include <asm/system.h>		/* cli(), *_flags */
#include <linux/uaccess.h>	/* copy_to_user() copy_from_user() */

MODULE_LICENSE("Dual BSD/GPL");

struct file_operations *rpc_fop_array[] = {
	&rpc_poll_fops,		/* poll */
	&rpc_intr_fops		/* intr */
};

void **rpc_data_ptr_array[] = {
	(void **)&rpc_poll_devices,	/* poll */
	(void **)&rpc_intr_devices	/* intr */
};

int rpc_data_size_array[] = {
	sizeof(RPC_POLL_Dev),	/* poll */
	sizeof(RPC_INTR_Dev)	/* intr */
};

int rpc_major = 0;
const char *rpc_name = "RPC";

/*
 * Finally, the module stuff
 */

#ifdef CONFIG_DEVFS_FS
#ifndef KERNEL2_6
devfs_handle_t rpc_devfs_dir;
static char devname[4];
#endif
#else
static struct class *rpc_class;
#endif

int RPC_init_module(struct platform_device *pdev);

/*RPC MODULE need implement smp_send_rtk_rpc, remove from platform.c*/
#include <asm/arch_gicv3.h>
#include <linux/irqchip/arm-gic-v3.h>

#define MPIDR_TO_SGI_AFFINITY(cluster_id, level) \
    (MPIDR_AFFINITY_LEVEL(cluster_id, level) \
        << ICC_SGI1R_AFFINITY_## level ##_SHIFT)

static void gic_send_sgi(u64 cluster_id, u16 tlist, unsigned int irq)
{
    u64 val;

    val = (MPIDR_TO_SGI_AFFINITY(cluster_id, 3) |
           MPIDR_TO_SGI_AFFINITY(cluster_id, 2) |
           irq << ICC_SGI1R_SGI_ID_SHIFT        |
           MPIDR_TO_SGI_AFFINITY(cluster_id, 1) |
           tlist << ICC_SGI1R_TARGET_LIST_SHIFT);

    rtd_pr_rpc_debug("CPU%d: ICC_SGI1R_EL1 %llx\n", raw_smp_processor_id(), val);
    gic_write_sgi1r(val);
}

void rtk_gic_raise_softirq(const int cpu, unsigned int irq)
{
	unsigned int mpidr;
	unsigned long cluster_id;
	u16 tlist = 0;

	if (WARN_ON(irq >= 16))
		return;

	// cpu is affinity_1 id,
	mpidr = (cpu & MPIDR_LEVEL_MASK) << (MPIDR_LEVEL_BITS * 1);
	cluster_id = mpidr & ~0xffUL;

    /*
     * Ensure that stores to Normal memory are visible to the
     * other CPUs before issuing the IPI.
     */
    smp_wmb();

	tlist |= 1 << (mpidr & 0xf);
	gic_send_sgi(cluster_id, tlist, irq);

    /* Force the above writes to ICC_SGI1R_EL1 to be executed */
    isb();
}
EXPORT_SYMBOL(rtk_gic_raise_softirq);

/* IPI to audio cpu core */
#define IPI_RTK_RPC (15)
__attribute__((weak)) void smp_send_rtk_rpc(int cpu)
{
    rtd_pr_rpc_debug("rtk_rpc %d %d\n", cpu, raw_smp_processor_id());
    rtk_gic_raise_softirq(cpu, IPI_RTK_RPC);
}

int flush_sync_flag(void)
{
	dmac_flush_range(phys_to_virt(0x000000d0), phys_to_virt(0x000000e0));
	outer_flush_range(0x000000d0, 0x000000e0);
	return 0;
}

static void enable_rpc_interrupt(void)
{
	rtd_outl(SB2_CPU_SW_INT_EN_reg, (RPC_WRITE_ENABLE_SCPU|RPC_INT_WRITE_1));
}


static void disable_rpc_interrupt(void)
{
	rtd_outl(SB2_CPU_SW_INT_EN_reg, RPC_WRITE_ENABLE_SCPU);
}

static int rpc_event_notify(unsigned long action)
{

	switch (action) {
	case AVCPU_RESET_PREPARE:
		rtd_pr_rpc_info("[RPC]: AVCPU_RESET_PREPARE...\n");
		rpc_poll_pause();
		rpc_intr_pause();
		rpc_kern_pause();

		*((int *)phys_to_virt(SYNC_FLAG_ACPU)) = 0x00000000;
		*((int *)phys_to_virt(SYNC_FLAG_V1CPU)) = 0x00000000;
#ifdef CONFIG_REALTEK_RPC_VCPU2
		*((int *)phys_to_virt(SYNC_FLAG_V2CPU)) = 0x00000000;
#endif
#ifdef CONFIG_REALTEK_RPC_VCPU3
		*((int *)phys_to_virt(SYNC_FLAG_V3CPU)) = 0x00000000;
#endif

		flush_sync_flag();

		disable_rpc_interrupt();
		
		break;
	case AVCPU_RESET_DONE:
		rtd_pr_rpc_info("[RPC]: AVCPU_RESET_DONE...\n");
#if 0
		rpc_poll_init();
		rpc_intr_init();
		rpc_kern_init();
#else	//Not re-init RPC buffer
		rpc_poll_run();
		rpc_intr_run();
		rpc_kern_run();
#endif

		/* clear the inter-processor interrupts */
		rtd_outl(SB2_CPU_SW_INT_2_reg, RPC_INT_SCPU_A);
		rtd_outl(SB2_CPU_SW_INT_2_reg, RPC_INT_SCPU_V);
		rtd_outl(SB2_CPU_SW_INT_0_reg, RPC_INT_ACPU_S);
		rtd_outl(SB2_CPU_SW_INT_1_reg, RPC_INT_VCPU_S);
#ifdef CONFIG_REALTEK_RPC_VCPU2
		rtd_outl(SB2_CPU_SW_INT_3_reg, RPC_INT_VCPU2_S);
		rtd_outl(SB2_CPU_SW_INT_2_reg, RPC_INT_SCPU_V2);
#endif

		enable_rpc_interrupt();
		break;
	}

	return 0;
}

/* Nonzero if either X or Y is not aligned on a "int" boundary.  */
#define UNALIGNED(X, Y) \
    (((long)X & (sizeof(int) - 1)) | ((long)Y & (sizeof(int) - 1)))
/* How many bytes are copied each iteration of the 4X unrolled loop.  */
#define BIGBLOCKSIZE    (sizeof(int) << 2)
/* How many bytes are copied each iteration of the word copy loop.  */
#define LITTLEBLOCKSIZE (sizeof(int))
/* Threshhold for punting to the byte copier.  */
#define TOO_SMALL(LEN)  ((LEN) < LITTLEBLOCKSIZE)

int my_copy_to_user(int *des, int *src, int size)
{
	if (UNALIGNED(des, src))
		rtd_pr_rpc_err("my_copy_to_user: unaligned happen...\n");

	if (copy_to_user((void __user *)des, (int *)src, sizeof(char)*size)) {
		rtd_pr_rpc_err("my_copy_to_user: copy error happen...\n");
		return size;
	}

	return 0;
}

int my_copy_from_user(int *des, int *src, int size)
{
	if (UNALIGNED(des, src))
		rtd_pr_rpc_err("my_copy_from_user: unaligned happen...\n");

	if (copy_from_user((int *)des, (void __user *)src, sizeof(char)*size)) {
		rtd_pr_rpc_err("my_copy_from_user: copy error happen...\n");
		return size;
	}
	
	return 0;
}
int my_copy_user(int *des0, int *src0, int len0)
{
	char *dst = (char*)des0;
	const char *src = (char*)src0;
	int *aligned_dst;
	const int *aligned_src;
	
	if (UNALIGNED(dst, src))
		rtd_pr_rpc_err("my_copy_user: unaligned happen...\n");

	/* If the size is small, or either SRC or DST is unaligned,
	   then punt into the byte copy loop.  This should be rare.  */
	if (!TOO_SMALL(len0) && !UNALIGNED(src, dst)) {
		aligned_dst = (int *)dst;
		aligned_src = (int *)src;

		/* Copy 4X long words at a time if possible.  */
		while (len0 >= BIGBLOCKSIZE) {
			*aligned_dst++ = *aligned_src++;
			*aligned_dst++ = *aligned_src++;
			*aligned_dst++ = *aligned_src++;
			*aligned_dst++ = *aligned_src++;
			len0 -= BIGBLOCKSIZE;
		}

		/* Copy one long word at a time if possible.  */
		while (len0 >= LITTLEBLOCKSIZE) {
			*aligned_dst++ = *aligned_src++;
			len0 -= LITTLEBLOCKSIZE;
		}
		
		/* Pick up any residual with a byte copier.  */
		dst = (char *)aligned_dst;
		src = (char *)aligned_src;
	}

	while (len0--)
		*dst++ = *src++;

	return 0;
}

irqreturn_t rpc_isr(int irq, void *dev_id)
{
	int itr;
	PDEBUG("irq number %d...\n", irq);

	itr = rtd_inl((unsigned int)SB2_CPU_SW_INT_2_reg);

	if (itr & (RPC_INT_SCPU_A | RPC_INT_SCPU_V 
#ifdef CONFIG_REALTEK_RPC_VCPU2
		| RPC_INT_SCPU_V2
#endif
	)) {
		while (itr & (RPC_INT_SCPU_A | RPC_INT_SCPU_V 
#ifdef CONFIG_REALTEK_RPC_VCPU2
			| RPC_INT_SCPU_V2
#endif
		)) {

			/* ack the interrupt */
			if (itr & RPC_INT_SCPU_A) {
				/*to clear interrupt, set bit[0] to 0 then we can clear A2S int */
#ifdef CONFIG_DEBUG_KERNEL_RPC
				record_isr_time[RPC_AUDIO] = jiffies;
#endif
				rtd_outl(SB2_CPU_SW_INT_2_reg, RPC_INT_SCPU_A);
#ifdef CONFIG_REALTEK_RPC_MULTIPROCESS
				if (rpc_intr_devices[RPC_INTR_DEV_AS_ID1].ringIn != rpc_intr_devices[RPC_INTR_DEV_AS_ID1].ringOut){
#ifdef CONFIG_DEBUG_KERNEL_RPC
					record_isr_user_wake_up_time[RPC_AUDIO] = jiffies;
#endif
					tasklet_schedule(&(rpc_intr_extra[RPC_INTR_DEV_AS_ID1].tasklet));
				}
#else
				if (rpc_intr_devices[RPC_INTR_DEV_AS_ID1].ringIn != rpc_intr_devices[RPC_INTR_DEV_AS_ID1].ringOut){
#ifdef CONFIG_DEBUG_KERNEL_RPC
					record_isr_user_wake_up_time[RPC_AUDIO] = jiffies;
#endif
					wake_up_interruptible(&(rpc_intr_devices[RPC_INTR_DEV_AS_ID1].ptrSync->waitQueue));
				}
#endif
				if (rpc_kern_devices[RPC_KERN_DEV_AS_ID1].ringIn != rpc_kern_devices[RPC_KERN_DEV_AS_ID1].ringOut){
#ifdef CONFIG_DEBUG_KERNEL_RPC
					record_isr_wake_up_time[RPC_AUDIO] = jiffies;
#endif
					wake_up_interruptible(&(rpc_kern_devices[RPC_KERN_DEV_AS_ID1].ptrSync->waitQueue));
				}

			}
			if (itr & RPC_INT_SCPU_V) {
				/*to clear interrupt */
#ifdef CONFIG_DEBUG_KERNEL_RPC
				record_isr_time[RPC_VIDEO] = jiffies;//current_kernel_time().tv_sec;//jiffies;
#endif
				rtd_outl(SB2_CPU_SW_INT_2_reg, RPC_INT_SCPU_V);
#ifdef CONFIG_REALTEK_RPC_MULTIPROCESS
				if (rpc_intr_devices[RPC_INTR_DEV_V1S_ID3].ringIn != rpc_intr_devices[RPC_INTR_DEV_V1S_ID3].ringOut){
#ifdef CONFIG_DEBUG_KERNEL_RPC
					record_isr_user_wake_up_time[RPC_VIDEO] = jiffies;
#endif
					tasklet_schedule(&(rpc_intr_extra[RPC_INTR_DEV_V1S_ID3].tasklet));
				}
#else
				if (rpc_intr_devices[RPC_INTR_DEV_V1S_ID3].ringIn != rpc_intr_devices[RPC_INTR_DEV_V1S_ID3].ringOut){
#ifdef CONFIG_DEBUG_KERNEL_RPC
					record_isr_user_wake_up_time[RPC_VIDEO] = jiffies;
#endif
					wake_up_interruptible(&(rpc_intr_devices[RPC_INTR_DEV_V1S_ID3].ptrSync->waitQueue));
				}
#endif
				if (rpc_kern_devices[RPC_KERN_DEV_V1S_ID3].ringIn != rpc_kern_devices[RPC_KERN_DEV_V1S_ID3].ringOut){
#ifdef CONFIG_DEBUG_KERNEL_RPC
					record_isr_wake_up_time[RPC_VIDEO] = jiffies;//current_kernel_time().tv_sec;//jiffies;
#endif
					wake_up_interruptible(&(rpc_kern_devices[RPC_KERN_DEV_V1S_ID3].ptrSync->waitQueue));
				}
			}
#ifdef CONFIG_REALTEK_RPC_VCPU2
			if (itr & RPC_INT_SCPU_V2) {
				/*to clear interrupt */
				rtd_outl(SB2_CPU_SW_INT_2_reg, RPC_INT_SCPU_V2);
#ifdef CONFIG_REALTEK_RPC_MULTIPROCESS
				if (rpc_intr_devices[RPC_INTR_DEV_V2S_ID5].ringIn !=rpc_intr_devices[RPC_INTR_DEV_V2S_ID5].ringOut)
					tasklet_schedule(&(rpc_intr_extra[RPC_INTR_DEV_V2S_ID5].tasklet));
#else
				if (rpc_intr_devices[RPC_INTR_DEV_V2S_ID5].ringIn != rpc_intr_devices[RPC_INTR_DEV_V2S_ID5].ringOut)
					wake_up_interruptible(&(rpc_intr_devices[RPC_INTR_DEV_V2S_ID5].ptrSync->waitQueue));
#endif
				if (rpc_kern_devices[RPC_INTR_DEV_V2S_ID5].ringIn != rpc_kern_devices[RPC_INTR_DEV_V2S_ID5].ringOut)
					wake_up_interruptible(&(rpc_kern_devices[RPC_INTR_DEV_V2S_ID5].ptrSync->waitQueue));
			}
#endif
			itr = rtd_inl((unsigned int)SB2_CPU_SW_INT_2_reg);
		}
	} else {
		return IRQ_NONE;
	}

	return IRQ_HANDLED;
}

// cyhuang (2011/11/19) +++
//                          Add for new device PM driver.
#ifdef CONFIG_PM
#if IS_ENABLED(CONFIG_RTK_KDRV_AUDIO)
static struct device *rtkaudio_device_prt = 0;
void set_rtkaudio_device_ptr(struct device *dev)
{
        rtkaudio_device_prt = dev;
}
EXPORT_SYMBOL(set_rtkaudio_device_ptr);
#endif
extern int avcpu_suspend(void);
extern int avcpu_resume(void);
extern struct notifier_block avcpu_pm_notifier;

static int venus_rpc_pm_suspend(struct device *dev)
{
	rtd_pr_rpc_notice("[RPC] suspend\n");
#if IS_ENABLED(CONFIG_RTK_KDRV_AUDIO)
	if(rtkaudio_device_prt)
		device_pm_wait_for_dev(dev, rtkaudio_device_prt);
	else
		panic("Error rtkaudio driver don't call set_rtkaudio_device_ptr\n");
#endif
	avcpu_suspend();

	/* Disable the interrupt from system to audio & video */
	rpc_event_notify(AVCPU_RESET_PREPARE);
	return 0;
}

static int venus_rpc_pm_resume(struct device *dev) {
	/* Enable the interrupt from system to audio & video */
	rtd_pr_rpc_notice("[RPC] resume\n");
	rpc_event_notify(AVCPU_RESET_DONE);

	avcpu_resume();

	return 0;
}

static const struct dev_pm_ops venus_rpc_pm_ops = {
	.suspend = venus_rpc_pm_suspend,
	.resume = venus_rpc_pm_resume,
#ifdef CONFIG_HIBERNATION
	.freeze = venus_rpc_pm_suspend,
	.thaw = venus_rpc_pm_resume,
	.poweroff = venus_rpc_pm_suspend,
	.restore = venus_rpc_pm_resume,
#endif
};

#endif

void __iomem *rpc_base;

static
int venus_rpc_probe(struct platform_device *pdev)
{
	struct resource res;

	if (of_address_to_resource(pdev->dev.of_node, 0, &res)) {
		rtd_pr_rpc_err("rpc driver, can not find rpc resource\n");
		BUG();
	}
	rpc_base = dvr_remap_uncached_memory(res.start, PAGE_ALIGN(resource_size(&res)), __builtin_return_address(0));

	//rpc_base = of_iomap(pdev->dev.of_node, 0);
	PDEBUG("rpc_base = %p\n",rpc_base);
	WARN(!rpc_base, "unable to map venus_rpc area\n");

	RPC_init_module(pdev);
	return 0;
}


static int venus_rpc_remove(struct platform_device *pdev)
{
	int irq = 0;

	irq = platform_get_irq(pdev, 0);
	if (irq >= 0)
		free_irq(irq, (void *)RPC_ID);

	return 0;
}


static const struct of_device_id venus_rpc_of_match[] = {
	{
		.compatible = "realtek,venus-rpc",
	},
	{},
};
MODULE_DEVICE_TABLE(of, venus_rpc_of_match);

static struct platform_device *venus_rpc_devs;
/*
// +++ cyhuang (2011/03/23)
//                         Change device_driver to platform_driver.
*/
static struct platform_driver venus_rpc_driver = {
	.probe		= venus_rpc_probe,
	.remove		= venus_rpc_remove,
	.driver = {
		   .name = "Venus_RPC",
		   .bus = &platform_bus_type,
		   .of_match_table = venus_rpc_of_match,

#ifdef CONFIG_PM
		   .pm = &venus_rpc_pm_ops,
#endif
		   },
};

#ifdef CONFIG_PM
struct device* get_rpc_device_ptr(void)
{
	return &(venus_rpc_devs->dev);
}
EXPORT_SYMBOL(get_rpc_device_ptr);

#endif

/* Destructor of this module. */
void RPC_cleanup_module(void)
{
	int i;

	if (venus_rpc_devs != 0) {
		if (venus_rpc_devs->dev.driver != 0)
			platform_driver_unregister(&venus_rpc_driver);
		platform_device_unregister(venus_rpc_devs);
		venus_rpc_devs = 0;
	}

	unregister_pm_notifier(&avcpu_pm_notifier);
	/* call the cleanup functions for friend devices */
	rpc_poll_cleanup();
	rpc_intr_cleanup();

#ifdef CONFIG_DEVFS_FS
#ifndef KERNEL2_6
	devfs_unregister(rpc_devfs_dir);
#else
	devfs_remove("rpc");

	/* cleanup_module is never called if registering failed */
	unregister_chrdev(rpc_major, rpc_name);
#endif
#else
	for (i = 0; i < RPC_NR_DEVS; i++) {
		device_destroy(rpc_class, MKDEV(rpc_major, i));
	}
	device_destroy(rpc_class, MKDEV(rpc_major, 100));
#ifdef CONFIG_REALTEK_RPC_DEBUG
	device_destroy(rpc_class, MKDEV(rpc_major, 101));
#endif
	class_destroy(rpc_class);
	/* cleanup_module is never called if registering failed */
	unregister_chrdev(rpc_major, rpc_name);
#endif

	PDEBUG("Goodbye, RPC~\n");
}

static char *rpc_devnode(struct device *dev, umode_t *mode)
{
	*mode = 0666;
	return NULL;
}

/* Constructor of this module.*/
int RPC_init_module(struct platform_device *pdev)
{
	int result;
	int Virq=0;
#ifdef CONFIG_DEVFS_FS
	int i, j, k;
#ifndef KERNEL2_6
	devfs_handle_t *ptr;
#endif
#else
	int i;
#endif
	rtd_pr_rpc_err("Larger buffer! size of RPC_POLL_Dev %d and RPC_INTR_Dev %d...\n", (int)sizeof(RPC_POLL_Dev), (int)sizeof(RPC_INTR_Dev));
/*
	SET_MODULE_OWNER(&rpc_poll_fops);
	SET_MODULE_OWNER(&rpc_intr_fops);
*/
	PDEBUG("Hello, RPC~\n");
#ifdef CONFIG_DEVFS_FS
#ifndef KERNEL2_6
	/* If we have devfs, create /dev/rpc to put files in there */
	rpc_devfs_dir = devfs_mk_dir(NULL, "rpc", NULL);
	if (!rpc_devfs_dir) return -EBUSY;	/* problem */
#else
	/* create /dev/rpc to put files in there */
	devfs_mk_dir("rpc");

	/* register rpc_poll_fops as default file operation */
	result = register_chrdev(rpc_major, rpc_name, &rpc_poll_fops);
	if (result < 0) {
		PDEBUG("Can't get major %d\n", RPC_MAJOR);
		return result;
	}
	if (rpc_major == 0) rpc_major = result;	/* dynamic */
	PDEBUG("RPC major number: %d\n", rpc_major);
#endif
#else /* no devfs, do it the "classic" way */
	/* register rpc_poll_fops as default file operation */
	result = register_chrdev(rpc_major, rpc_name, &rpc_poll_fops);
	if (result < 0) {
		PDEBUG("Can't get major %d\n", RPC_MAJOR);
		return result;
	}
	if (rpc_major == 0) rpc_major = result;	/* dynamic */
	PDEBUG("RPC major number: %d\n", rpc_major);
#endif /* CONFIG_DEVFS_FS */

	register_pm_notifier(&avcpu_pm_notifier);
	/* At this point call the init function for any kind device */
	if ((result = rpc_poll_init()))
		goto fail;
	if ((result = rpc_intr_init()))
		goto fail;
	if ((result = rpc_kern_init()))
		goto fail;

#ifdef CONFIG_DEVFS_FS
#ifndef KERNEL2_6
	/* Create corresponding node in device file system */
	for (i = 0; i < RPC_NR_DEVS; i++) {
		j = i / RPC_NR_PAIR;	/* ordinal number */
		k = i % RPC_NR_PAIR;	/* device kind*/
		sprintf(devname, "%i", i);
		ptr = (devfs_handle_t *) (*rpc_data_ptr_array[k] + rpc_data_size_array[k] * j);
/*      PDEBUG("   ***ptr[%d] = 0x%8x\n", i, (int)ptr); */
		*ptr = devfs_register(rpc_devfs_dir, devname,
					  DEVFS_FL_AUTO_DEVNUM,
					  0, 0, S_IFCHR | S_IRUGO | S_IWUGO,
					  rpc_fop_array[k], ptr);
	}
#else
	/* Create corresponding node in device file system */
	for (i = 0; i < RPC_NR_DEVS; i++) {
		devfs_mk_cdev(MKDEV(rpc_major, i), S_IFCHR | S_IRUSR | S_IWUSR, "rpc/%d", i);
	}
	devfs_mk_cdev(MKDEV(rpc_major, 100), S_IFCHR | S_IRUSR | S_IWUSR, "rpc/100");
#ifdef CONFIG_REALTEK_RPC_DEBUG
	devfs_mk_cdev(MKDEV(rpc_major, 101), S_IFCHR|S_IRUSR|S_IWUSR, "rpc/debug");
#endif
#endif
#else
	rpc_class = class_create(THIS_MODULE, "rpc");
	if (IS_ERR(rpc_class))
		return PTR_ERR(rpc_class);

	rpc_class->devnode = rpc_devnode;
	for (i = 0; i < RPC_NR_DEVS; i++) {
		device_create(rpc_class, NULL, MKDEV(rpc_major, i), NULL, "rpc%d", i);
	}
	device_create(rpc_class, NULL, MKDEV(rpc_major, 100), NULL, "rpc100");
#ifdef CONFIG_REALTEK_RPC_DEBUG
	device_create(rpc_class, NULL, MKDEV(rpc_major, 101), NULL, "rpcdebug");
#endif
#endif

	Virq = platform_get_irq(pdev, 0);
	if(!Virq){
		WARN("%s there is no rpc irq config in dts\n",__func__);
		of_node_put(pdev->dev.of_node);
		return -EINVAL;
	}
	PDEBUG("PRC Virq is %d...\n",Virq);
	if (request_irq(Virq, rpc_isr, IRQF_SHARED, "rpc", (void *)RPC_ID) != 0)

		rtd_pr_rpc_err("PRC :Can't get assigned irq...\n");
	/* Enable the interrupt from system to audio & video */
	rtd_outl(SB2_CPU_SW_INT_EN_reg, (RPC_WRITE_ENABLE_SCPU|RPC_INT_WRITE_1));

	*((int *)phys_to_virt(SYNC_FLAG_ACPU)) = 0xffffffff;
	*((int *)phys_to_virt(SYNC_FLAG_V1CPU)) = 0xffffffff;
#ifdef CONFIG_REALTEK_RPC_VCPU2
	*((int *)phys_to_virt(SYNC_FLAG_V2CPU)) = 0xffffffff;
#endif
#ifdef CONFIG_REALTEK_RPC_VCPU3
	*((int *)phys_to_virt(SYNC_FLAG_V3CPU)) = 0xffffffff;
#endif

	flush_sync_flag();

	/* [[KTASKWBS-1065] Make AVCPU resume run at asynchronous for parallel resume time*/
	device_enable_async_suspend(&(pdev->dev));
	venus_rpc_devs = pdev;

	if (register_kernel_rpc(RPC_DVR_MALLOC, rpc_dvr_malloc) == 1)
		rtd_pr_rpc_err("register dvr remote malloc fail\n");
	if (register_kernel_rpc(RPC_DVR_FREE, rpc_dvr_free) == 1)
		rtd_pr_rpc_err("register dvr remote free fail\n");

	return 0;		/* succeed */

fail:
	PDEBUG("RPC error number: %d\n", result);
	RPC_cleanup_module();
	return result;
}

module_platform_driver(venus_rpc_driver);
