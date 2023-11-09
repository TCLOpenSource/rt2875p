/*
 * Dora DDR test 
 *
 * Copyright (C) 2017 Realtek
 *
 */

/* product ID::
 * LxE K3  0
 * COMMON  1
 * LxE K*  2
 * SKYW    3
 * others  ?
 */
#define PRODUCT_ID 2
//#define SCAN_DRV_VER  0 //K3*
//#define SCAN_DRV_VER  1  //mac5p, s4ap
#define SCAN_DRV_VER  2  //k4* and later ?
#define _VER(prod_id, drv_ver)  ((((prod_id) & 0x0ff)<<0x8) | (((drv_ver) & 0x0ff)))
#include <rtd_log/rtd_module_log.h>
#define VERSION() _VER(PRODUCT_ID, SCAN_DRV_VER)
#include <rtd_log/rtd_module_log.h>

///////////register interface ///////////////////
/* these register should be depended on CHIP ID.
 * 
 */
#if IS_ENABLED(CONFIG_RTK_DRV_DDR_SCAN)
/**
   @ SW INT : 
   0x0xb801A10c[8]
   Command register:
   @ Function enable :
   0x180c2748[29:28] => 0: test off; 1: test on; 2: return fail address; 3: return fail data
   @ Status register:
   0x180c274C[23:16] for error status; [31:0] for return fail address or fail data
   0x180c274C[31] => DM busy bit
**/
#define DMSCAN_CMD_REG      DC_MC_MC_DMY_WDG_RST_FW2_reg   //(0xB80C2748)
#define APP_CMD_REG         DC_MC_MC_DMY_WDG_RST_FW2_reg   //(0xB80C2748) 
#define APP_CMD_REG2        DC_MC_MC_DMY_WDG_RST_FW0_reg   //(0xB80C2740)
#define DMSCAN_STATUS_REG   DC_MC_MC_DMY_WDG_RST_FW3_reg   //(0xB80C274C)
#define DMSCAN_STATUS_BIT  (16)
#define DMSCAN_INDICATOR_REG DC_MC_MC_DMY_WDG_RST_FW3_reg   //(0xB80C274C)  //ML7BU-1051
#define DMSCAN_INDICATOR_BIT (31)  //ML7BU-1051
#define DMSCAN_UPDATE_STATUS(status)					\
	do {								\
		rtd_hwsem_2_lock();					\
		rtd_maskl(DMSCAN_STATUS_REG, ~(0x0ff<<DMSCAN_STATUS_BIT), (!!(status))<<DMSCAN_STATUS_BIT); \
		rtd_hwsem_2_unlock();					\
	} while(0)

#define DMSCAN_INDICATE(cpu, running)					\
	do {								\
		if (!running)						\
			atomic_andnot(1<<cpu, &dmscan_cpu_running);	\
		else							\
			atomic_or(1<<cpu, &dmscan_cpu_running);		\
									\
		rtd_hwsem_2_lock();					\
		rtd_maskl(DMSCAN_INDICATOR_REG, ~(0x01<<DMSCAN_INDICATOR_BIT), (!!(atomic_read(&dmscan_cpu_running)))<<DMSCAN_INDICATOR_BIT); \
		rtd_hwsem_2_unlock();					\
	} while(0)


/* Register::CPU_SW_INT_EN	0x1801A100 */
#define CMDREG_INT_EN_REG   SB2_CPU_SW_INT_EN_reg   //0xb801A100
#define CMDREG_INT_EN_BIT 3   //scpu sw interrupt enable

/* Register::CPU_SW_INT<Set::3>	0x1801A104 */
#define CMDREG_INT_REG      SB2_CPU_SW_INT_2_reg    //0xb801A10C
#define CMDREG_INT_BIT 8 //SCPU int from DDR_SCAN tool trigger



//////////////////////////////////////////////////////////////////
#include <linux/mm.h>
#include <linux/cpu.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/smpboot.h>
#include <linux/sched/rt.h>
#include <linux/tick.h>
#include <linux/dma-mapping.h>
#include <linux/kthread.h>
#include <linux/gfp.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <asm/delay.h>
#include <rbus/sb2_reg.h>
#include <rbus/dc_mc_reg.h>
//#include "rtk_semaphore.h"
#include "rtk_kdriver/io.h"
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 1))
#include <linux/sched/types.h>
#endif
#include <uapi/linux/sched/types.h>
#include <linux/pageremap.h>
/* this file provide a low priority thread and 2 interfaces:
 * 1) low priority thread: DM scan state machine.
 * 2) a register interface with IRQ trigger. (shared IRQ with A2S interrupt).
 * 3) /proc interface.
 * 
 * the status/action is isolated for easy query by 2 interfaces.
 * - status simply queried by interfaces.
 */

static DEFINE_MUTEX(dmscan_mutex);

static int dmscan_running;
static int dmscan_status = -1;
static int dmscan_forever = 1; //no stop 
static u64 dmscan_due; //// dmscan test due time
atomic_t dmscan_cpu_running = ATOMIC_INIT(0);


static DECLARE_BITMAP(dmscan_cpumask_bits, CONFIG_NR_CPUS) = CPU_BITS_ALL;
static struct cpumask *dmscan_cpumask = to_cpumask(dmscan_cpumask_bits);

static void rtd_hwsem_2_lock(void);
static void rtd_hwsem_2_unlock(void);

static int dmscan_set_cpumask(char *s);
__setup("dmscan", dmscan_set_cpumask);


/* Helper for online, unparked cpus. */
#define for_each_dmscan_cpu(cpu)   \
	for_each_cpu_and((cpu), cpu_online_mask, dmscan_cpumask)

int dmscan_update_status(int cpu);
int dmscan_clear_status(void);


/*
 * 1) 'dmscan_state' denotes whether dmscan thread is running.
 *   - also provide user interface to start/stop again.
 * 2) 'dmscan_period' the scan period is 30 seconds, if dmsca_forever is zero.
 */
static DEFINE_PER_CPU(int, dmscan_state); //// START, STOP
static DEFINE_PER_CPU(int, dmscan_result); //// dmscan test per-cpu result
static DEFINE_PER_CPU(void *, dmscan_area); //// dmscan test per-cpu sand box
static DEFINE_PER_CPU(struct page*, dmscan_page); //// handler of sand box.
static DEFINE_PER_CPU(struct task_struct *, dmscan_task); //to store task_struct ptr of per-cpu kthread

///////////////////////

enum { DMSCAN_STOP = 0, DMSCAN_START = 1 };


// FIXME: porting for bring-up compile error
void rtk_dump_ddr_reg(void)
{
}

static void dmscan_pcpu_set_prio(unsigned int policy, int prio)
{
	struct sched_param param = { .sched_priority = prio };

	sched_setscheduler(current, policy, &param);
}

static void dmscan_pcpu_set_state(unsigned int cpu, int state)
{
	*per_cpu_ptr(&dmscan_state, cpu) = state;
}

#define __unused __attribute__ ((unused))
__unused  static void dmscan_pcpu_start(unsigned int cpu) 
{
	rtd_pr_ddr_scan_info("%s \n", __FUNCTION__);
	dmscan_pcpu_set_state(cpu, DMSCAN_START);

	DMSCAN_INDICATE(cpu, 1);
}

static void dmscan_pcpu_stop(unsigned int cpu)
{
	rtd_pr_ddr_scan_info("%s \n", __FUNCTION__);
	dmscan_pcpu_set_state(cpu, DMSCAN_STOP);

	DMSCAN_INDICATE(cpu, 0);
}

static void dmscan_pcpu_setup(unsigned int cpu)
{
	void *ptr;
        struct page *page;

	if (!per_cpu(dmscan_area, cpu)) {
                page = dvr_malloc_page(PAGE_SIZE, GFP_DCU1_FIRST);
                ptr = dvr_remap_uncached_memory(__pfn_to_phys(page_to_pfn(page)), PAGE_SIZE, __builtin_return_address(0));
		if (!ptr)
			return;
		
		preempt_disable();
		per_cpu(dmscan_area, cpu) = ptr;
		per_cpu(dmscan_page, cpu) = page;
		preempt_enable();

                rtd_pr_ddr_scan_info("%s(%d), dmscan_area: 0x%p phys: 0x%08lx\n\n", __FUNCTION__, cpu,  ptr, (unsigned long)__pfn_to_phys(page_to_pfn(page)));
	}

	per_cpu(dmscan_result, cpu) = 0;
	
	//lowest priority.
	dmscan_pcpu_set_prio(SCHED_NORMAL, MAX_NICE);

	DMSCAN_INDICATE(cpu, 1);
}


static void dmscan_pcpu_cleanup(unsigned int cpu, bool online)
{
	void *ptr = per_cpu(dmscan_area, cpu);
	struct page *page = per_cpu(dmscan_page, cpu);

	rtd_pr_ddr_scan_info("%s(%d), dmscan_area: 0x%p\n", __FUNCTION__, cpu,  per_cpu(dmscan_area, cpu));
	if (ptr) {
		per_cpu(dmscan_area, cpu) = NULL;
                dvr_unmap_memory(ptr, PAGE_SIZE);
                dvr_free_page(page);
	}

	DMSCAN_INDICATE(cpu, 0);
}


static int dmscan_pcpu_should_run(unsigned int cpu)
{
	return (__this_cpu_read(dmscan_state) != DMSCAN_STOP);
}


#define DC_BURST_LEN (32) //half burt len
#define DC(addr) ((((unsigned long)(addr) /512) & 1) + 1)  //index of DC , starting from 1.
#define DDR(addr) ((((unsigned long)(addr) /16) & 3) + 1)  //index of DDR , starting from 1.
#define BYTE(addr) ({ unsigned long __p = (unsigned long)(addr); __p %= 16; __p=(((__p&(1<<3))>>2)|(__p&1)) + 1; })  //index of BYTE , starting from 1.

const char test_pattern[] = { 0x00, 0xff, 0x01, 0xfe, 0x3, 0xfc, 0x7, 0xf1, 0x0f, 0xf0, 0x1f, 0xe0, 0x3f, 0xc0, 0x7f, 0x80, 0xff, 0x00};

/*
 * The dmscan thread function .
 *
 */
static void dmscan_pcpu_fn(unsigned int cpu)
{
	volatile char *p, c; 
	int result = 0;
	int ofs;
	int i;
	int dm_statistic_rec[2][2];
	int pat_size = sizeof(test_pattern) / sizeof(char);
	void *area = __this_cpu_read(dmscan_area);
	int size = PAGE_SIZE;
	struct page *page = __this_cpu_read(dmscan_page);

	// resize to DC_BURST_LEN aligned
	int mod = size / DC_BURST_LEN;

	rtd_pr_ddr_scan_debug("%s(%d), dmscan_area: 0x%p, due:%llu, %llu, %d \n",
		 __FUNCTION__, cpu,  per_cpu(dmscan_area, cpu),
		 dmscan_due, jiffies_64, !!time_is_before_jiffies((unsigned long)dmscan_due));
	
	if (!area)
		return;

	if ((dmscan_forever == 0) && time_is_before_jiffies((unsigned long)dmscan_due)) {
		//dmscan_pcpu_stop(cpu);
		kthread_park(current);
		return;
	}

	// write test pattern
	for (ofs = 0; ofs < DC_BURST_LEN; ofs++) {
		p = ((char *)area) + ofs;
		
		for (i=0; i < mod; i++, p+= DC_BURST_LEN)  {
			*(volatile char *)p = test_pattern[ofs % pat_size];
			dma_wmb();
		}
	}


	//read back to check
	p = (char *)area;
	for (ofs = 0, i = 0; i < size; i++, p++, ofs= (ofs+1) % DC_BURST_LEN) {
		if ((c = *(volatile char *)p) != test_pattern[ofs % pat_size]) {
			rtd_pr_ddr_scan_debug("\ndmscan(%x): addr:0x%08lx (phys:0x%08lx) data:0x%08x(again: 0x%08x) \n",
				 ofs, (long)p, (unsigned long)__pfn_to_phys(page_to_pfn(page)), c, *(volatile char *)p);

			result++;

			dm_statistic_rec[ DC(p)-1 ][ BYTE(p)-1 ] ++;
			rtd_pr_ddr_scan_debug("DC(%d) BYTE(%d): %d \n", 1, 1, dm_statistic_rec[0][0]);
			rtd_pr_ddr_scan_debug("DC(%d) BYTE(%d): %d \n", 1, 2, dm_statistic_rec[0][1]);
			rtd_pr_ddr_scan_debug("DC(%d) BYTE(%d): %d \n", 2, 1, dm_statistic_rec[1][0]);
			rtd_pr_ddr_scan_debug("DC(%d) BYTE(%d): %d \n", 2, 2, dm_statistic_rec[1][1]);
		}
	}

	__this_cpu_add(dmscan_result, result);

	//update result register
	dmscan_update_status(cpu);
	
	return;
}




static struct smp_hotplug_thread dmscan_threads = {
	.store			= &dmscan_task,
	.thread_should_run	= dmscan_pcpu_should_run,
	.thread_fn		= dmscan_pcpu_fn,
	.thread_comm		= "dmscan_pcpu/%u",
	.create			= dmscan_pcpu_start,
	.setup			= dmscan_pcpu_setup,
	.cleanup		= dmscan_pcpu_cleanup,
	.park			= dmscan_pcpu_stop,
	.unpark			= dmscan_pcpu_start,
};


//////////////////////////////////////////////////////////////////


static int dmscan_park_threads(void)
{
	int cpu, ret = 0;

	rtd_pr_ddr_scan_debug("%s\n", __FUNCTION__);
	
	for_each_dmscan_cpu(cpu) {
		ret = kthread_park(per_cpu(dmscan_task, cpu));
		if (ret)
			break;
	}

	return ret;
}

static void dmscan_unpark_threads(void)
{
	int cpu;

	rtd_pr_ddr_scan_debug("%s\n", __FUNCTION__);

	for_each_dmscan_cpu(cpu)
		kthread_unpark(per_cpu(dmscan_task, cpu));
}

////////// status implementation/////////////////
static void rtd_hwsem_2_lock(void);
static void rtd_hwsem_2_unlock(void);

int dmscan_update_status(int cpu)
{
	int cpu_result = per_cpu(dmscan_result, cpu);

	if (dmscan_status ^ cpu_result) {
		mutex_lock(&dmscan_mutex);
		dmscan_status |= cpu_result;

		/* fixme */
		DMSCAN_UPDATE_STATUS(dmscan_status);
	
		mutex_unlock(&dmscan_mutex);
	}
	rtd_pr_ddr_scan_debug("%s: cpu:%d, 0x%x, status:0x%x \n",
		 __FUNCTION__, cpu, cpu_result, dmscan_status);

	DMSCAN_INDICATE(cpu, 1);

	return 0;
}

int dmscan_clear_status(void)
{
	int cpu;

	dmscan_status = 0;
	for_each_dmscan_cpu(cpu) {
		per_cpu(dmscan_result, cpu) = 0;
	}

	rtd_pr_ddr_scan_debug("%s: \n", __FUNCTION__);

	return 0;
}

int dmscan_get_status(void)
{
	return dmscan_status;
}


/** 
 * '0' to denote as disabled cpu.
 * otherwise, a integer to denote cpu bitmask enabled.
 **/
static int dmscan_set_cpumask(char *s)
{
	unsigned long bits_cpu;
	
	rtd_pr_ddr_scan_info("%s, %s,(0x%0lx)\n", __FUNCTION__, s, *dmscan_cpumask_bits);

	if (dmscan_running != 0) {
		rtd_pr_ddr_scan_err("dmscan is running \n");
		return -EBUSY;
	}

	if (kstrtoul(s, 0, &bits_cpu)) {
		rtd_pr_ddr_scan_err("ignore invalid default state: %s \n", s);
		return 0;
	}

	bitmap_copy(dmscan_cpumask_bits,  &bits_cpu, NR_CPUS);

	return 0;
}


//////////////////////////
static int update_dmscan_all_cpus(void)
{
	int ret;

	rtd_pr_ddr_scan_debug("%s\n", __FUNCTION__);

	ret = dmscan_park_threads();
	if (ret) {
		rtd_pr_ddr_scan_err("cannot stop all dmscan \n"); //fixme
		return ret;
	}
	
	dmscan_unpark_threads();

	return 0;
}

static void dmscan_disable_all_cpus(void)
{
	rtd_pr_ddr_scan_info("%s\n", __FUNCTION__);

	if (dmscan_running) {
		dmscan_running = 0;
		dmscan_park_threads();
		smpboot_unregister_percpu_thread(&dmscan_threads);
	}
}


static int dmscan_enable_all_cpus(void)
{
	int err = 0;

	rtd_pr_ddr_scan_info("%s: running:%d cpumask:0x%lx\n", __FUNCTION__, dmscan_running, *dmscan_cpumask_bits);

	/* clear status */
	dmscan_clear_status();

	if (!dmscan_running) {
		/* int cpu; */
		
		/* //for test, set start state */
		/* for_each_dmscan_cpu(cpu)  */
		/* 	dmscan_pcpu_start(cpu); */
		
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
		// commit 167a88677b05d6a810f23b8 : remove cpumask from the API.
		err = smpboot_register_percpu_thread(&dmscan_threads);
#else
		err = smpboot_register_percpu_thread_cpumask(&dmscan_threads,
							     dmscan_cpumask);
#endif
		if (err)
			rtd_pr_ddr_scan_err("Failed to create dmscan threads, disabled\n");
		else 
			dmscan_running = 1;
	} else {
		err = update_dmscan_all_cpus();

		if (err) {
			dmscan_disable_all_cpus();
			rtd_pr_ddr_scan_err("Failed to update dmscan threads, disabled\n");
		}
	}

	if (!err)
		dmscan_due = jiffies_64 + 30*HZ;

	return err;
}


///////// the control interface   /////////
ssize_t dmscan_cmd_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", dmscan_running);
}

enum {CMD_STOP=0, CMD_START=1};
ssize_t dmscan_cmd_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned long cmd;
	
	if ((sscanf(buf, "0x%lx", &cmd) != 1) && (sscanf(buf, "%ld", &cmd) != 1)) {
		rtd_pr_ddr_scan_info("wrong cmd.\n");
		return -EINVAL;
	}
		
	if (cmd == CMD_STOP)
		dmscan_disable_all_cpus();
	else	
		dmscan_enable_all_cpus();

	return count;
}

ssize_t dmscan_status_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "0x%08x\n", dmscan_get_status());
}


/* fixme, should we have this API ?*/
ssize_t dmscan_status_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	mutex_lock(&dmscan_mutex);
	dmscan_status = 0;
	mutex_unlock(&dmscan_mutex);

	return count;
}

ssize_t dmscan_cpumask_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	char bits[10];
	int ret;

	ret = sscanf(buf, "%9s", bits);
	if (ret != 1)
		return -EINVAL;

	dmscan_set_cpumask(bits);

	return count;
}

ssize_t dmscan_cpumask_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "0x%08lx\n", *dmscan_cpumask_bits);
}





static struct kobj_attribute dmscan_cmd_attr =
	__ATTR(cmd, 0644, dmscan_cmd_show, dmscan_cmd_store);

static struct kobj_attribute dmscan_status_attr =
	__ATTR(status, 0644, dmscan_status_show, dmscan_status_store);

static struct kobj_attribute dmscan_cpumask_attr =
	__ATTR(cpumask, 0644, dmscan_cpumask_show, dmscan_cpumask_store);


static struct attribute *dmscan_attrs[] = {
	&dmscan_cmd_attr.attr,
	&dmscan_status_attr.attr,
	&dmscan_cpumask_attr.attr,
	NULL,
};

static struct attribute_group dmscan_attr_group = {
	.attrs = dmscan_attrs,
};

static struct kobject *ddr_kobj;
static struct kobject *dmscan_kobj;

int cmdreg_register_isr(void);

int dmscan_init(void)
{
	int ret;

	rtd_pr_ddr_scan_debug("%s\n", __FUNCTION__);

	
	cmdreg_register_isr();

	// default enable all cpu 
	//dmscan_enable_all_cpus();

	if (!ddr_kobj)
		ddr_kobj = kobject_create_and_add("ddr", NULL);

	if (!ddr_kobj) {
		rtd_pr_ddr_scan_info("no board object, cannot create dmscan sysfs control \n");
		return -EEXIST;
	}


	dmscan_kobj = kobject_create_and_add("dmscan", ddr_kobj);
	if (!dmscan_kobj) {
		rtd_pr_ddr_scan_info("fail to create dmscan sysfs control \n");
		return -EFAULT;
	}
	
	ret = sysfs_create_group(dmscan_kobj, &dmscan_attr_group);
	if (!ret)
		return -EFAULT;

	kobject_put(dmscan_kobj);
	return 0;
}


/////////////// cmdreg interface /////////////////////////////////

int cmdreg_dmscan_handler(int reg, int data)
{
	int cmd;
	
	if (reg != DMSCAN_CMD_REG)
		return -1;

	
	cmd = (data >> 28) & 0x3;

	if (cmd == 1)
		dmscan_enable_all_cpus();
	else if (cmd == 0)
		dmscan_disable_all_cpus();
	/* this command is supported from SCAN_DRV_VER=1. */
#if 0 //(SCAN_DRV_VER > 0)	
	else if (cmd == 2) {
		/* fixme, skip hw_semaphore protection. this should be first use. */
		rtd_outl(DMSCAN_STATUS_REG, VERSION());
		rtd_pr_ddr_scan_info("version: 0x%x \n", VERSION());
	}
#endif //#if (SCAN_DRV_VER > 0)	

	return 0;
}




//////////////// CMDREG ISR mechanism ////////////////////

typedef int (*cmdreg_handler_t)(int, int);


struct cmdreg_struct_t {
	unsigned long reg;
	cmdreg_handler_t handler;
	int data;
	int tag;
};




int cmdreg_app_handler(int reg, int data);
	
/* noted, end entite is reg=0 */
struct cmdreg_struct_t cmdreg_table[] = {
	{ DMSCAN_CMD_REG, cmdreg_dmscan_handler, 0, 0},
	{ APP_CMD_REG, cmdreg_app_handler, 0, 0},
	{ APP_CMD_REG2, cmdreg_app_handler, 0, 0},
	{ 0 },
};

#define CMDREG_TABLE_SIZE (sizeof(cmdreg_table)/sizeof(cmdreg_table[0]))

irqreturn_t cmdreg_thread_fn(int irq, void *dev_id)
{
	struct cmdreg_struct_t *t = cmdreg_table;

	while (t->reg != 0) {
		rtd_pr_ddr_scan_debug("%s: t.reg(0x%lx), data(0x%0x), tag(0x%0x) \n", __FUNCTION__, t->reg, t->data, t->tag);
		if (t->tag != 0) {
			int ret;
		
			ret = t->handler(t->reg, t->data);
			t->tag = 0;
		}
		
		t++;
	}

	return IRQ_HANDLED;
}


/* check if any registers are changed, and tagged as 1 */
irqreturn_t cmdreg_isr(int irq, void *dev_id)
{
	int int_probe = 0;
	struct cmdreg_struct_t *t = cmdreg_table;

	rtd_pr_ddr_scan_debug("%s: irq(%d), int(0x%0x) \n", __FUNCTION__, irq, rtd_inl(CMDREG_INT_REG));
	
// only dedicated IRQ acked.
/* FIXME, for (SCAN_DRV_VER != 1).
 * mac5p is shared IRQ, that may be ack-ed by another ISR.
 */
#if (SCAN_DRV_VER != 1)
	
	if ((rtd_inl(CMDREG_INT_REG) & (1<<CMDREG_INT_BIT)) == 0)
		return IRQ_NONE;
#endif //#if (SCAN_DRV_VER != 1)
	
#if 0
#if (SCAN_DRV_VER > 0)  

	/* use CMD_REG[31] as fake interrupt status, 
	 * to avoid conflict of sharing IRQ with RPC 
	 */
	if (!(rtd_inl(DMSCAN_CMD_REG) & (1<<31)))
		return IRQ_NONE;

	int_probe ++;
	// this is to ack fake interrupt.
	rtd_maskl(DMSCAN_CMD_REG, ~(1<<31), 0);
#endif //#if (SCAN_DRV_VER != 1)
#endif 	
	while (t->reg != 0) {
		int data = rtd_inl(t->reg);
		
		rtd_pr_ddr_scan_debug("%s: t.reg(0x%lx), data(0x%0x), last_data(0x%0x) \n", __FUNCTION__, t->reg, data, t->data);
		if (data & (1<<31)) {
			data &= ~(1<<31);
			
			if (1) {// data != t->data) {
				int_probe ++;
				t->data = data;
				t->tag = 1;
				rtd_pr_ddr_scan_debug("%s: t.reg(0x%lx), data(0x%0x), tag(%d) \n", __FUNCTION__, t->reg, data, t->tag);
			}
		}
		t++;
	}

	if (int_probe == 0)
		return IRQ_NONE;

	//ack
	rtd_outl(CMDREG_INT_REG, (1<<CMDREG_INT_BIT));

	//ask thread context
	return IRQ_WAKE_THREAD;
}

#define DTS_SB2_DEV_NAME       "sb2"
#define DTS_SB2_SW_INT_NAME    "SW_INT"
int cmdreg_register_isr(void)
{
	int ret = 0, irq = 0;
	struct device_node *sb2 = of_find_node_by_name(NULL, DTS_SB2_DEV_NAME);

	if (!sb2) {
		rtd_pr_ddr_scan_err("%s: can not find [%s] device from DT\n", __FUNCTION__, DTS_SB2_DEV_NAME);
		return -ENODEV;
	}

	irq = of_irq_get_byname(sb2, DTS_SB2_SW_INT_NAME);
	if (irq < 0) {
		rtd_pr_ddr_scan_err("%s: can not find %s from %s \n", __FUNCTION__, DTS_SB2_SW_INT_NAME, DTS_SB2_DEV_NAME);
		return irq;
	}

	ret = request_threaded_irq(irq, cmdreg_isr, cmdreg_thread_fn,
			IRQF_SHARED, "cmdreq", (void *)0x2266);
	if (ret) {
		rtd_pr_ddr_scan_err("%s: can not register cmdreg irq (ret:%d) \n", __FUNCTION__, ret);
	}

	//enable int_en
	rtd_outl(CMDREG_INT_EN_REG, ((1<<CMDREG_INT_EN_BIT) | 1));

	return 0;
}

unsigned int saved_swint_en = 0;
void cmdreg_save_swint_en(void)
{
	saved_swint_en = rtd_inl(CMDREG_INT_EN_REG);
}
void cmdreg_restore_swint_en(void)
{
	rtd_outl(CMDREG_INT_EN_REG, saved_swint_en | (1<<0)); 
}


///////////////// misc. /////////////////
#if !defined (RTK_SEMAPHORE_H)

/* Notice: this API is semantically diff from hw_semaphore_try_lock.
 * return 1: get lock
 * return 0: not get lock.
 */
static int rtd_hwsem_2_trylock(void)
{
	int ret;
	ret =  (*(unsigned int volatile *)GET_MAPPED_RBUS_ADDR(SB2_HD_SEM_NEW_2_reg));
	return ret;
}


static void rtd_hwsem_2_lock(void)
{
	while (!rtd_hwsem_2_trylock()) {
		udelay(5);
	}
}

static void rtd_hwsem_2_unlock(void)
{
	*(unsigned int volatile *)GET_MAPPED_RBUS_ADDR(SB2_HD_SEM_NEW_2_reg) = 0;
	dsb(sy);
}


#else
static void rtd_hwsem_2_lock(void)
{
	rtd_hwsem_lock(MAKE_SEMA_UUID(SWSEM_2, SEM_SCPU, NUM0, 0));
}

static void rtd_hwsem_2_unlock(void)
{
	rtd_hwsem_unlock(MAKE_SEMA_UUID(SWSEM_2, SEM_SCPU, NUM0, 0));
}

#endif 



//////// export rtk_scan device for command register monitor ////////

static ssize_t cmdreg_app_read(struct file *f, struct kobject *kobj, struct bin_attribute *attr, char *buf, loff_t pos, size_t size);
static ssize_t cmdreg_app_write(struct file *f, struct kobject *kobj, struct bin_attribute *attr, char *buf, loff_t pos, size_t count);

static struct bin_attribute cmdreg_app_attr =
	__BIN_ATTR(cmdreg, 0644, cmdreg_app_read, cmdreg_app_write, 0);
static struct bin_attribute cmdreg2_app_attr =
	__BIN_ATTR(cmdreg2, 0644, cmdreg_app_read, cmdreg_app_write, 0);



atomic_t cmdreg_completion = ATOMIC_INIT(0);
atomic_t cmdreg2_completion = ATOMIC_INIT(0);
unsigned int app_cmd;
unsigned int app_cmd2;
int cmdreg_app_handler(int reg, int data)
{
	
	if ((reg != APP_CMD_REG) && (reg != APP_CMD_REG2))
		return -1;

	
	if (reg == APP_CMD_REG) {
		app_cmd = (data & 0xfff00);
		atomic_set(&cmdreg_completion, 1);
		sysfs_notify(ddr_kobj, NULL, "cmdreg");  //cmdreg_app_attr.attr.name
	}		
	else if (reg == APP_CMD_REG2) {
		app_cmd2 = (data & 0x07f);
		atomic_set(&cmdreg2_completion, 1);
		sysfs_notify(ddr_kobj, NULL, "cmdreg2");
	}

	
	rtd_pr_ddr_scan_info("%s: reg(0x%0x), data(0x%0x) \n", __FUNCTION__, reg, data);

	return 0;
}

static ssize_t cmdreg_app_read(struct file *f, struct kobject *kobj, struct bin_attribute *attr, char *buf, loff_t pos, size_t size)
{
	int ret = atomic_read(&cmdreg_completion) || atomic_read(&cmdreg2_completion);

	if (!ret) 
		return 0;
	
	/* return cmdreg in format of binary */
	if (attr == &cmdreg_app_attr) {
		*(unsigned int *)buf = app_cmd; //fixme, aligned to 4 ?
		atomic_set(&cmdreg_completion, 0);
		rtd_pr_ddr_scan_info("app_read: 0x%x \n", app_cmd);
	}
	else if (attr == &cmdreg2_app_attr) {
		*(unsigned int *)buf = app_cmd2; //fixme, aligned to 4 ?
		atomic_set(&cmdreg2_completion, 0);
		rtd_pr_ddr_scan_info("app_read: 0x%x \n", app_cmd2);
	}

	return 4;

}

static ssize_t cmdreg_app_write(struct file *f, struct kobject *kobj, struct bin_attribute *attr, char *buf, loff_t pos, size_t count)
{
	int val;
	
	/* return cmdreg in format of binary */
	val = *(unsigned int *)buf; //fixme, aligned to 4 ?


	/* write back to rbus and trigger SW int */
	if (attr == &cmdreg_app_attr) {
		rtd_outl(APP_CMD_REG, val | (1<<31));
		rtd_outl(CMDREG_INT_REG, (1<<CMDREG_INT_BIT) | (1<<0));
		rtd_pr_ddr_scan_info("app_write: 0x%x: 0x%x \n", APP_CMD_REG, val);
	}
	else {
		rtd_pr_ddr_scan_info("ignore write data \n");
	}
	
	return count;
}



static struct bin_attribute *attrs[] = {
	&cmdreg_app_attr,
	&cmdreg2_app_attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

static struct attribute_group attr_group = {
	 .bin_attrs = attrs,
};


static int __init cmdreg_init(void)
{
	int retval;

	if (!ddr_kobj)
		ddr_kobj = kobject_create_and_add("ddr", NULL);
	
	if (!ddr_kobj)
		return -ENOMEM;

	retval = sysfs_create_group(ddr_kobj, &attr_group);
	if (retval)
		kobject_put(ddr_kobj);

	dmscan_init();

	rtd_pr_ddr_scan_info("cmdreg_inti ! \n");
	
	return retval;
}


module_init(cmdreg_init);
MODULE_AUTHOR("Realtek.com");
MODULE_LICENSE("GPL");
#endif //#if IS_ENABLED(CONFIG_RTK_DRV_DDR_SCAN)
