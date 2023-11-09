/*
 *  linux/mm/rtkrecord.c
 *
 *  Copyright (C) 1991, 1992, 1993, 1994  Linus Torvalds
 *
 *  Support for blueprint of dvr zone, I-Chieh Hsu
 */

#include <linux/mm.h>
#include <linux/module.h>
#include <rtd_log/rtd_module_log.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/sort.h>
#include <mach/rtk_platform.h>
#include <rtk_kdriver/rmm/rtkrecord.h>
#include <rtk_kdriver/rmm/auth.h>
#include <rtk_kdriver/rmm/pageremap.h>

#define RTK_RECORD_ADDR_MASK 0x1ffffffff

extern struct device	*auth_dev;

static unsigned long rtk_record_start_time = 0;
static struct radix_tree_root	rtk_tree;
static spinlock_t		rtk_tree_lock;
static unsigned long tree_node_count = 0;
static unsigned long tree_node_tag_count[RADIX_TREE_MAX_TAGS];
static struct workqueue_struct *cma_deferfree_workq;
static struct delayed_work delayed_cma_deferfree_work;

int cma_deferfree_delay = RTK_RECORD_DEFER_FREE_TIMEOUT; // free defer delay
int cma_debug_workq_delay = 2000; // workqueue thread polling interval

extern struct rtkcma rtkcma_list[];

static unsigned long current_cma_page_total[DEV_CMA_NUM] = {0};
unsigned long cma_alloc_peak_value[DEV_CMA_NUM] = {0};

#ifdef CONFIG_CMA_MONITOR
static cma_monitor_record cma_monitor_modules[] = {
	{"DEMUX",	MAX_CMA_LIMIT_DEMUX >> PAGE_SHIFT, 0, 0},
	{"rtkvdec",	MAX_CMA_LIMIT_RTKVDEC >> PAGE_SHIFT, 0, 0},
	{"Scaler",	MAX_CMA_LIMIT_SCALER >> PAGE_SHIFT, 0, 0},
};

static int cma_monitor_check_mem(unsigned long type, const void *caller, bool isHighMem, char *module_keyword);
static int cma_monitor_release_mem(unsigned long type, void *caller, bool isHighMem);
#endif

void *dma_get_allocator(struct device *dev);

//extern void cma_list_dump_register (void (*fp)(void));
void rtk_record_list_dump(void)
{
	list_all_rtk_memory_allocation_sort(list_mem_generic,NULL,NULL);
	show_cma_avaliable();
}
EXPORT_SYMBOL(rtk_record_list_dump);

void rtk_record_init(void)
{
	int i = 0;
	rtk_record_start_time = jiffies;
	INIT_RADIX_TREE(&rtk_tree, GFP_ATOMIC);
	spin_lock_init(&rtk_tree_lock);

	for (i = 0; i < RADIX_TREE_MAX_TAGS; i++)
		tree_node_tag_count[i] = 0;

	if (need_debug_cma_deferfree()) {
		cma_deferfree_workq = create_singlethread_workqueue("cma_deferfree");
		INIT_DELAYED_WORK(&delayed_cma_deferfree_work, rtk_record_defer_free);
		if (cma_deferfree_workq)
			queue_delayed_work(cma_deferfree_workq, &delayed_cma_deferfree_work, msecs_to_jiffies(cma_debug_workq_delay));
	}

//	cma_list_dump_register(rtk_record_list_dump);
}

void rtk_record_tag_set(rtk_record *ptr, unsigned int tag)
{
	spin_lock_irq(&rtk_tree_lock);

	if (ptr) {
		unsigned long addr = ptr->addr;

		radix_tree_tag_set(&rtk_tree, (addr&RTK_RECORD_ADDR_MASK) >> 2, tag);
		if (tag == RTK_RECORD_TAG_DEFER_FREE)
			tree_node_tag_count[RTK_RECORD_TAG_DEFER_FREE] ++;
		
		ptr->reserved = (ptr->reserved & 0xfffffff0) | 0x00000001; // defer free set
		ptr->jiffies = (jiffies - rtk_record_start_time) + msecs_to_jiffies(cma_deferfree_delay);
	}

	spin_unlock_irq(&rtk_tree_lock);
}

void rtk_record_tag_clear(unsigned long addr, unsigned int tag)
{
	spin_lock_irq(&rtk_tree_lock);

	radix_tree_tag_clear(&rtk_tree, (addr&RTK_RECORD_ADDR_MASK) >> 2, tag);
	if (tag == RTK_RECORD_TAG_DEFER_FREE)
		tree_node_tag_count[RTK_RECORD_TAG_DEFER_FREE] --;

	spin_unlock_irq(&rtk_tree_lock);
}

static inline void __record_tag_clear(unsigned long addr, unsigned int tag)
{
	radix_tree_tag_clear(&rtk_tree, (addr&RTK_RECORD_ADDR_MASK) >> 2, tag);
	if (tag == RTK_RECORD_TAG_DEFER_FREE)
		tree_node_tag_count[RTK_RECORD_TAG_DEFER_FREE] --;
}

int rtk_record_tag_get(unsigned long addr, unsigned int tag)
{
	int ret = 0;
	spin_lock_irq(&rtk_tree_lock);

	ret = radix_tree_tag_get(&rtk_tree, (addr&RTK_RECORD_ADDR_MASK) >> 2, tag);

	spin_unlock_irq(&rtk_tree_lock);
	return ret;
}

static inline int __record_tag_get(unsigned long addr, unsigned int tag)
{
	int ret = 0;
	ret = radix_tree_tag_get(&rtk_tree, (addr&RTK_RECORD_ADDR_MASK) >> 2, tag);
	return ret;
}

int rtk_record_insert(unsigned long addr, unsigned long type, unsigned long priv, const void *caller, unsigned long owner)
{
	rtk_record *ptr;
	int error;
	int dev_idx = 0;
	
#ifdef CONFIG_CMA_MONITOR
	char module_keyword[MODULE_KEYWORDS_LENGTH];
	memset(module_keyword, 0, MODULE_KEYWORDS_LENGTH);
#endif

	ptr = kmalloc(sizeof(rtk_record), GFP_KERNEL);
	if (unlikely(!ptr))
		return -ENOMEM;
	ptr->addr = addr;
	ptr->type = type;
	ptr->priv = priv;
	ptr->caller = caller;
	ptr->pid = current->pid;
	ptr->tgid = current->tgid;
	ptr->owner = owner;
	ptr->jiffies = jiffies - rtk_record_start_time;
	ptr->reserved = 0;
	addr = (addr&RTK_RECORD_ADDR_MASK) >> 2;	// use WORD as minimum unit
	error = radix_tree_preload(GFP_KERNEL);
	if (error == 0) {
		spin_lock_irq(&rtk_tree_lock);
		error = radix_tree_insert(&rtk_tree, addr, ptr);

#ifdef CONFIG_CMA_MONITOR
		if (cma_monitor_check_mem(type, caller, PageHighMem(page), module_keyword) < 0) {
			spin_unlock_irq(&rtk_tree_lock);
			radix_tree_preload_end();
			list_all_rtk_memory_allocation_sort(list_func_mem, cmp_caller_ascending_order, module_keyword);
			BUG();
		}
#endif

		for (dev_idx = DEV_CMA1_DEFAULT; dev_idx < DEV_CMA_NUM; dev_idx++) {
			if (!rtkcma_list[dev_idx].exist)
				continue;
			if (in_cma_range(rtkcma_list[dev_idx].dev, ptr->addr >> PAGE_SHIFT)) {
				current_cma_page_total[dev_idx] += (type & 0x00ffffff);
				if (cma_alloc_peak_value[dev_idx] < current_cma_page_total[dev_idx])
					cma_alloc_peak_value[dev_idx] = current_cma_page_total[dev_idx];
				break;
			}
		}

		if (error) {
			rtd_pr_rmm_err("radix tree insert error..., error = %d\n", error);
			kfree((void *)ptr);
			dump_stack();
		} else 
			tree_node_count ++;

		spin_unlock_irq(&rtk_tree_lock);
		radix_tree_preload_end();
	}

	return error;
}

int rtk_record_lookup(unsigned long addr, unsigned long *priv)
{
	rtk_record *ptr;
	int ret = 0;

	spin_lock_irq(&rtk_tree_lock);
	ptr = radix_tree_lookup(&rtk_tree, (addr&RTK_RECORD_ADDR_MASK) >> 2);
	if ((ptr) && (ptr->addr == addr)) {
		ret = ptr->type;
		if (priv)
			*priv = ptr->priv;
	} else
		rtd_pr_rmm_err("radix tree lookup error...\n");
	spin_unlock_irq(&rtk_tree_lock);

	return ret;
}

int rtk_record_lookup_ex(unsigned long addr, rtk_record **ptr, bool lock)
{
	int ret = 0;
	rtk_record *tmp = NULL;

	if (lock)
		spin_lock_irq(&rtk_tree_lock);

	tmp = radix_tree_lookup(&rtk_tree, (addr&RTK_RECORD_ADDR_MASK) >> 2);
	if ((tmp) && (tmp->addr == addr)) {
		ret = tmp->type;
		if (ptr)
			*ptr = tmp;
	} else
		rtd_pr_rmm_err("radix tree lookup error...\n");

	if (lock)
		spin_unlock_irq(&rtk_tree_lock);

	return ret;
}

void rtk_record_delete(unsigned long addr)
{
	rtk_record *ptr;
	int type;
	unsigned long index = 0;
	int dev_idx = 0;

	type = rtk_record_lookup(addr,NULL);
	index = (addr&RTK_RECORD_ADDR_MASK) >> 2;	// use WORD as minimum unit
	spin_lock_irq(&rtk_tree_lock);
	ptr = radix_tree_delete(&rtk_tree, index);
	tree_node_count --;

#ifdef CONFIG_CMA_MONITOR
	cma_monitor_release_mem(type, ptr->caller, PageHighMem(page));
#endif

	for (dev_idx = DEV_CMA1_DEFAULT; dev_idx < DEV_CMA_NUM; dev_idx++) {
		if (!rtkcma_list[dev_idx].exist)
			continue;
		if (in_cma_range(rtkcma_list[dev_idx].dev, addr >> PAGE_SHIFT)) {
			current_cma_page_total[dev_idx] -= (type & 0x00ffffff);
			break;
		}
	}

	spin_unlock_irq(&rtk_tree_lock);
	if (ptr)
		kfree((void *)ptr);
	else
		rtd_pr_rmm_err("radix tree delete error...\n");
}

static inline void __record_delete(unsigned long addr)
{
	rtk_record *ptr;

	addr = (addr&RTK_RECORD_ADDR_MASK) >> 2;	// use WORD as minimum unit
	ptr = radix_tree_delete(&rtk_tree, addr);
	tree_node_count --;

	if (ptr)
		kfree((void *)ptr);
	else
		rtd_pr_rmm_err("radix tree delete error...\n");
}

#ifdef CONFIG_CMA_MONITOR
int cma_monitor_check_mem(unsigned long type, const void *caller, bool isHighMem, char *module_keyword)
{
	int ret = 0;
	cma_monitor_record *record;
	unsigned long cma_request_size = (type & 0x00ffffff);
	char caller_name[CMA_CALLER_STR_LENGTH];
	memset(caller_name, 0, CMA_CALLER_STR_LENGTH);

	if ( ((type & ID_MASK) == BUDDY_MESG_ID) || ((type & ID_MASK) == DRIVER_MESG_ID)) {
		strncpy(caller_name, caller, sizeof(caller_name));
	} else {
		snprintf(caller_name, sizeof(caller_name), "%pF", caller);
	}
	rtd_pr_rmmc_debug("[CMA_MONITOR] %s requests %8lx from CMA\n", caller_name, cma_request_size << PAGE_SHIFT);

	for (record = cma_monitor_modules; record < &cma_monitor_modules + 1; record++) {
		if (caller_name && strstr(caller_name, record->module_name)) {
			unsigned long module_cma_amount;

			strncpy(module_keyword, record->module_name, MODULE_KEYWORDS_LENGTH);

			if(!isHighMem) {
				record->low_cma_amount += cma_request_size;
			} else {
				record->high_cma_amount += cma_request_size;
			}

			module_cma_amount = record->low_cma_amount + record->high_cma_amount;

			if (module_cma_amount > record->max_cma_limit) {
				rtd_pr_rmm_err("[CMA ERR] Module %s request CMA over limit (%d MB)\n", record->module_name, MB_calculate(record->max_cma_limit << PAGE_SHIFT) );
				rtd_pr_rmm_err("[CMA_MONITOR] (%s) requests size %8lx, (module total: %8lx, DCU1: %8lx, DCU2: %8lx)\n",
					caller_name, cma_request_size << PAGE_SHIFT,
					module_cma_amount << PAGE_SHIFT,
					record->low_cma_amount << PAGE_SHIFT,
					record->high_cma_amount << PAGE_SHIFT);
				return -1;
			}
			break;
		}
	}

	return ret;
}

int cma_monitor_release_mem(unsigned long type, void *caller, bool isHighMem)
{
	int ret = 0;
	char caller_name[CMA_CALLER_STR_LENGTH];
	cma_monitor_record *record;
	unsigned long cma_request_size= (type & 0x00ffffff);

	if ( ((type & ID_MASK) == BUDDY_MESG_ID) || ((type & ID_MASK) == DRIVER_MESG_ID) ) {
		strncpy(caller_name, caller, sizeof(caller_name));
	} else {
		snprintf(caller_name, sizeof(caller_name), "%pF", caller);
	}
	rtd_pr_rmmc_debug("[CMA_MONITOR] %s release CMA (size: 0x%x)\n", caller_name, cma_request_size << PAGE_SHIFT);

	for (record = cma_monitor_modules; record < &cma_monitor_modules + 1; record++) {
		if (strstr(caller_name, record->module_name)) {
			if(!isHighMem) {
				record->low_cma_amount -= cma_request_size;
			} else {
				record->high_cma_amount -= cma_request_size;
			}
			break;
		}
	}

	return ret;
}
#endif

void rtk_record_defer_free(struct work_struct *unused)
{
	rtk_record **pRecords = NULL;
	int tmp, cnt, start_index = 0;
	unsigned long local_tree_node_count = 0;
	int ret = 0;
	int dev_idx = 0;

	if (need_debug_cma_deferfree() == false)
		return;

	spin_lock(&rtk_tree_lock);
	local_tree_node_count = tree_node_tag_count[RTK_RECORD_TAG_DEFER_FREE];
	spin_unlock(&rtk_tree_lock);

	if (!local_tree_node_count) {
		rtd_pr_rmm_debug("[cma defer_free] rtk record radix tree no tag inside...\n");
		goto out;
	}

	pRecords = (rtk_record **)kmalloc(sizeof(rtk_record *) * local_tree_node_count, GFP_KERNEL);
	if (!pRecords) {
		ret = -ENOMEM;
		goto out;
	}

	spin_lock_irq(&rtk_tree_lock);

	tmp = radix_tree_gang_lookup_tag(&rtk_tree, (void **)pRecords, start_index, local_tree_node_count, RTK_RECORD_TAG_DEFER_FREE);
	rtd_pr_rmm_debug("[cma defer_free] tag count(%d/%ld), total(%ld)\n", tmp, local_tree_node_count, tree_node_count);

	if(tmp != local_tree_node_count) {
		rtd_pr_rmm_err("[cma defer_free] tree node tag(%d) is not equal tag count(%ld)...\n", tmp, local_tree_node_count);
		ret = -1;
	}

	spin_unlock_irq(&rtk_tree_lock);

	for (cnt = 0; cnt < tmp; cnt++) {
		int type;
		struct page *page = NULL;

		if (!(pRecords[cnt]->reserved & 0x0000000f)) {
			rtd_pr_rmm_err("[cma defer_free] not defer free\n");
			continue;
		} else if (!rtk_record_tag_get(pRecords[cnt]->addr, RTK_RECORD_TAG_DEFER_FREE)) {
			rtd_pr_rmm_err("[cma defer_free] not defer free tag\n");
			continue;
		}

//		rtd_pr_rmm_info("[cma defer_free] cnt=%d, type=%lx, addr=%lx, (%d)\n", cnt, pRecords[cnt]->type, pRecords[cnt]->addr, jiffies_to_msecs(jiffies - pRecords[cnt]->jiffies));

		if (time_before(jiffies - rtk_record_start_time, pRecords[cnt]->jiffies)) { // not timeout
//			rtd_pr_rmm_info("[cma defer_free] not defer free, timeout\n");
			continue;
		}

		type = pRecords[cnt]->type;
		page = pfn_to_page(pRecords[cnt]->addr >> PAGE_SHIFT);

		for (dev_idx = DEV_CMA1_DEFAULT; dev_idx < DEV_CMA_NUM; dev_idx++) {
			if (!rtkcma_list[dev_idx].exist)
				continue;
			if (in_cma_range(rtkcma_list[dev_idx].dev, pRecords[cnt]->addr >> PAGE_SHIFT)) {
				current_cma_page_total[dev_idx] -= (type & 0x00ffffff);
				break;
			}
		}

		dvr_defer_free((rtk_record *)pRecords[cnt]);

        spin_lock_irq(&rtk_tree_lock);
		__record_tag_clear(pRecords[cnt]->addr, RTK_RECORD_TAG_DEFER_FREE);
		__record_delete(pRecords[cnt]->addr);
        spin_unlock_irq(&rtk_tree_lock);
	}

out:

	if (pRecords)
		kfree(pRecords);

	if (cma_deferfree_workq)
		queue_delayed_work(cma_deferfree_workq, &delayed_cma_deferfree_work, msecs_to_jiffies(cma_debug_workq_delay));

//	return ret;
	return;
}

int list_all_rtk_memory_allocation_sort(list_rtk_mem_t list_func,int (*cmp_func)(const void *, const void *),void *data)
{
	rtk_record **pRecords = NULL;
	int tmp, start_index = 0;
	unsigned long local_tree_node_count = 0;
	int ret;

	spin_lock(&rtk_tree_lock);
	local_tree_node_count = tree_node_count;
	spin_unlock(&rtk_tree_lock);

	if (unlikely(!local_tree_node_count)){
		rtd_pr_rmm_err(" rtk record radix tree is empty...\n");
		return 0;
	}

	pRecords = (rtk_record **)kmalloc(sizeof(rtk_record *) * local_tree_node_count, GFP_KERNEL);
	if (unlikely(!pRecords)){
		return -ENOMEM;
	}

	spin_lock_irq(&rtk_tree_lock);

	tmp = radix_tree_gang_lookup(&rtk_tree, (void **)pRecords, start_index, local_tree_node_count);

	if(tmp != local_tree_node_count){
		rtd_pr_rmm_err(" tree node(%d) is not equal tree_node_count(%ld)...\n",tmp, local_tree_node_count);
		ret = -1;
		goto out;
	}

	if(cmp_func)
		sort(pRecords, local_tree_node_count,sizeof(rtk_record *),cmp_func,NULL);

	ret = list_func(pRecords, local_tree_node_count,data);

	// if user only want to list specific CMA data, it's not proper to print whole CMA peak value
	if (!data) {
		int dev_idx = 0;

		for (dev_idx = DEV_CMA1_DEFAULT; dev_idx < DEV_CMA_NUM; dev_idx++) {
			if (current_cma_page_total[dev_idx] > 0)
				rtd_pr_rmm_info("current alloc size at %s: %8lx \n", rtkcma_list[dev_idx].name, current_cma_page_total[dev_idx] << PAGE_SHIFT);
		}

		rtd_pr_rmm_info("\n");

		for (dev_idx = DEV_CMA1_DEFAULT; dev_idx < DEV_CMA_NUM; dev_idx++) {
			if (cma_alloc_peak_value[dev_idx] > 0)
				rtd_pr_rmm_info("allocated peak size at %s: %8lx \n", rtkcma_list[dev_idx].name, cma_alloc_peak_value[dev_idx] << PAGE_SHIFT);
		}
	}

out:
	spin_unlock_irq(&rtk_tree_lock);

	kfree(pRecords);
	return ret;

}

void free_all_rtk_memory_allocation(free_rtk_mem_t free_func)
{
	const int record_number = 10;
	rtk_record **pRecords = NULL;
	int tmp, cnt, start_index = 0;

	pRecords = (rtk_record **)kmalloc(sizeof(rtk_record *) * record_number, GFP_KERNEL);
	if (unlikely(!pRecords))
		return;

	rtd_pr_rmm_info("===== flush rtk memory =====\n");
	spin_lock(&rtk_tree_lock);
	while (1) {
		tmp = radix_tree_gang_lookup(&rtk_tree, (void **)pRecords, start_index, record_number);
		for (cnt = 0; cnt < tmp; cnt++) {
			rtd_pr_rmm_info(" release address: %lx \n", pRecords[cnt]->addr);
			start_index = pRecords[cnt]->addr;
			if (!free_func(pRecords[cnt]))
				__record_delete(pRecords[cnt]->addr);
		}

		if (tmp != record_number)
			break;
		start_index = ((start_index&RTK_RECORD_ADDR_MASK) >> 2) + 1;
	}
	spin_unlock(&rtk_tree_lock);

	kfree(pRecords);
}

int cmp_addr_ascending_order(const void *a, const void *b)
{
	rtk_record *tmp_a;
	rtk_record *tmp_b;
	//int ret;
	
	tmp_a = (rtk_record *)*(unsigned long *)a;
	tmp_b = (rtk_record *)*(unsigned long *)b;
	
	return (tmp_a->addr - tmp_b->addr);
}


int cmp_pid_ascending_order(const void *a, const void *b)
{
	rtk_record *tmp_a;
	rtk_record *tmp_b;
	int ret;
	
	tmp_a = (rtk_record *)*(unsigned long *)a;
	tmp_b = (rtk_record *)*(unsigned long *)b;
	
	ret = tmp_a->pid - tmp_b->pid;	//fisrt compare pid
	if(ret == 0){
		ret = (tmp_b->type & 0x00ffffff) - (tmp_a->type & 0x00ffffff); //if pid equal, size descending order
	}

	return ret;
}

int cmp_caller_ascending_order(const void *a, const void *b)
{
	rtk_record *tmp_a;
	rtk_record *tmp_b;
	long cmp;
	int ret;
	
	tmp_a = (rtk_record *)*(unsigned long *)a;
	tmp_b = (rtk_record *)*(unsigned long *)b;
	
	cmp = (unsigned long)tmp_a->caller - (unsigned long)tmp_b->caller;	//fisrt compare pid
	if(cmp == 0){
		ret = (tmp_b->type & 0x00ffffff) - (tmp_a->type & 0x00ffffff); //if pid equal, size descending order
	}
	else if(cmp > 0){
		ret = 1;
	}
	else{
		ret = -1;
	}

	return ret;

}

int cmp_size_descending_order(const void *a, const void *b)
{
	rtk_record *tmp_a;
	rtk_record *tmp_b;

	tmp_a = (rtk_record *)*(unsigned long *)a;
	tmp_b = (rtk_record *)*(unsigned long *)b;

	return (tmp_b->type & 0x00ffffff) - (tmp_a->type & 0x00ffffff);
}

int list_mem_generic(rtk_record **pRecords,unsigned long node_count,void *data)
{
	int cnt,ret=0;
	int total_alloc = 0;

	for(cnt = 0;cnt < node_count; cnt++){
		ret = remap_list_mem((rtk_record *)pRecords[cnt]);
		if (ret > 0) {
			if ( (((pRecords[cnt]->type & ID_MASK) == BUDDY_ID) && need_debug_cma_guardband()) ||
				 (((pRecords[cnt]->type & ID_MASK) == DRIVER_ID) && need_debug_cma_guardband()) ||
				 (((pRecords[cnt]->type & ID_MASK) == BUDDY_MESG_ID) && need_debug_cma_guardband()) ||
				 (((pRecords[cnt]->type & ID_MASK) == DRIVER_MESG_ID) && need_debug_cma_guardband()) )
			{
				dvr_guardband_check(pRecords[cnt]->addr, pRecords[cnt]->type & 0x00ffffff);
			}
		}
		total_alloc += ret;
	}

	rtd_pr_rmm_err("total allocated: %8x \n", total_alloc);
	return ret;
}

int list_addr_mem(rtk_record **pRecords,unsigned long node_count,void *data)
{
	int cnt,ret;
	int total_alloc = 0;
	int total_tmp = 0;
	unsigned long addr1 = 0,addr2 = 0;
	unsigned long start = 0,end = 0;
	int flag = 0;
	struct mem_bp *bp = NULL;
	phys_addr_t dcu2_start_addr;
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
	struct mem_bp *bw_bp = NULL;
	phys_addr_t highmem_bw_start_addr;
	int total_dcu2 = 0;
	int total_highmem_bw = 0;
#endif

	if(data){
		sscanf(data,"%x %x",(unsigned int *)&addr1,(unsigned int *)&addr2);
		if(addr2 == 0){
			end = start = addr1;
			flag = 1;
		}
		else{
			start = (addr1 < addr2)?addr1:addr2;
			end = (addr1 < addr2)?addr2:addr1;
		}

		for(cnt = 0;cnt < node_count; cnt++){
			if((pRecords[cnt]->addr >=start) 
				&& (pRecords[cnt]->addr  <= end))
				ret = remap_list_mem((rtk_record *)pRecords[cnt]);
			else
				ret = 0;
			total_alloc += ret;
		}
		if(flag)
			rtd_pr_rmm_info("address(%x) total allocated: %8x \n", (unsigned int)start,total_alloc);
		else
			rtd_pr_rmm_info("address(%x - %x) total allocated: %8x \n", (unsigned int)start,(unsigned int)end,total_alloc);
	}
	else{
		bp = (struct mem_bp *)dma_get_allocator(auth_dev);
		dcu2_start_addr = __pfn_to_phys(bp->base_pfn);
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
		if (carvedout_buf_query(CARVEDOUT_BW_HIGH_CMA, NULL) > 0) {
			bw_bp = (struct mem_bp *)dma_get_allocator(bw_auth_dev);
			highmem_bw_start_addr = __pfn_to_phys(bw_bp->base_pfn);
		}
#endif

		for(cnt = 0;cnt < node_count; cnt++){
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
			if(((pRecords[cnt]->addr >= dcu2_start_addr) || (pRecords[cnt]->addr >= highmem_bw_start_addr)) && (flag == 0))
#else
			if((pRecords[cnt]->addr >= dcu2_start_addr) && (flag == 0))
#endif
			{
				rtd_pr_rmm_info("total allocated at DCU1: %8x \n\n", total_tmp);
				flag = 1;
				total_tmp = 0;
			}
			ret = remap_list_mem((rtk_record *)pRecords[cnt]);
			total_alloc += ret;
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
			if (carvedout_buf_query(CARVEDOUT_BW_HIGH_CMA, NULL) > 0) {
				if (in_cma_range(bw_auth_dev, pRecords[cnt]->addr >> PAGE_SHIFT)) {
					total_highmem_bw += ret;
				}
			} else if (in_cma_range(auth_dev, pRecords[cnt]->addr >> PAGE_SHIFT)) {
				total_dcu2 += ret;
			} else {
				rtd_pr_rmm_err("Error: HighMem CMA record is not in DCU2 and HIGHMEM_BW\n");
			}
#else
			total_tmp += ret;
#endif
		}
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
		rtd_pr_rmm_info("total allocated at DCU2: %8x \n\n", total_dcu2);
		rtd_pr_rmm_info("total allocated at HIGHMEM_BW: %8x \n\n", total_highmem_bw);
#else
		rtd_pr_rmm_info("total allocated at DCU2: %8x \n\n", total_tmp);
#endif
		rtd_pr_rmm_info("total allocated: %8x \n", total_alloc);
	}
	return 0;
}


int list_pid_mem(rtk_record **pRecords,unsigned long node_count,void *data)
{
	int cnt,ret;
	int total_alloc = 0;
	int pid_alloc = 0;
	unsigned long pid = 0;

	if(data){
		sscanf(data,"%d",(unsigned int *)&pid);
		for(cnt = 0;cnt < node_count; cnt++){
			if(pid == pRecords[cnt]->pid)
				ret = remap_list_mem((rtk_record *)pRecords[cnt]);
			else
				ret = 0;
			total_alloc += ret;
		}
		if(total_alloc)
			rtd_pr_rmm_info("pid(%ld) total allocated: %8x \n", pid,total_alloc);
		else
			rtd_pr_rmm_info("pid(%ld) not allocated cma memory\n", pid);	
	}
	else{
		for(cnt = 0;cnt < node_count; cnt++){
			if((pid != ((rtk_record *)pRecords[cnt])->pid) && (pid != 0)){
				rtd_pr_rmm_info("pid(%ld) total allocated: %8x \n\n",pid,pid_alloc);
				pid_alloc = 0;
			}
			ret = remap_list_mem(pRecords[cnt]);
			pid_alloc += ret;
			total_alloc += ret;
			pid = ((rtk_record *)pRecords[cnt])->pid;
		}
		if(cnt == node_count){
			rtd_pr_rmm_info("pid(%ld) total allocated: %8x \n\n",pid,pid_alloc);
		}
		rtd_pr_rmm_info("total allocated: %8x \n", total_alloc);
	}
	
	return 0;

}

int list_mem_size_descending_order(rtk_record **pRecords,unsigned long node_count,void *data)
{
	int cnt,ret;
	int total_alloc = 0;
	unsigned long size1 = 0,size2 = 0;
	unsigned long start = 0,end = 0;
	int flag = 0;
	
	if(data){
		sscanf(data,"%x %x",(unsigned int *)&size1,(unsigned int *)&size2);
		if(size2 == 0){
			end = start = size1; 
			flag = 1;
		}
		else{
			start = (size1 < size2)?size1:size2;
			end = (size1 < size2)?size2:size1;
		}
		
		for(cnt = 0;cnt < node_count; cnt++){
			if((((pRecords[cnt]->type & 0x00ffffff)<< PAGE_SHIFT) >=start) 
				&& (((pRecords[cnt]->type & 0x00ffffff)<< PAGE_SHIFT) <= end))
				ret = remap_list_mem((rtk_record *)pRecords[cnt]);
			else
				ret = 0;
			total_alloc += ret;
		}
		if(flag)
			rtd_pr_rmm_info("size(%x) total allocated: %8x \n", (unsigned int)start,total_alloc);
		else
			rtd_pr_rmm_info("size(%x - %x) total allocated: %8x \n", (unsigned int)start,(unsigned int)end,total_alloc);		
	}
	else{
		for(cnt = 0;cnt < node_count; cnt++){
			ret = remap_list_mem((rtk_record *)pRecords[cnt]);
			total_alloc += ret;
		}
		rtd_pr_rmm_info("total allocated: %8x \n", total_alloc);
	}
	return 0;
}

int list_func_mem(rtk_record **pRecords,unsigned long node_count,void *data)
{
	int cnt,ret;
	int total_alloc = 0;
	int func_alloc = 0;
	unsigned char name[128],caller[128];

	memset(name,0,sizeof(name));
	memset(caller,0,sizeof(caller));
	if(data){
		sscanf(data,"%s",name);
		
		for(cnt = 0;cnt < node_count; cnt++){
			snprintf(caller,sizeof(caller) - 1,"%pf",pRecords[cnt]->caller);
			if(strstr(caller,name)){
				ret = remap_list_mem((rtk_record *)pRecords[cnt]);
			}
			else{
				ret = 0;
			}
			total_alloc += ret;
		}
		if(total_alloc)
			rtd_pr_rmm_info("function has %s total allocated: %8x \n", name,total_alloc);
		else
			rtd_pr_rmm_info("function has %s not allocated cma memory\n", name);
	}
	else{
		for(cnt = 0;cnt < node_count; cnt++){
			snprintf(caller,sizeof(caller) - 1,"%pf",pRecords[cnt]->caller);
			if((strcmp(caller,name)) && (name[0] != 0)){
				rtd_pr_rmm_info("func(%s) total allocated: %8x \n\n",name,func_alloc);
				func_alloc = 0;
			}
			ret = remap_list_mem(pRecords[cnt]);
			func_alloc += ret;
			total_alloc += ret;
			snprintf(name,sizeof(name) - 1,"%pf",pRecords[cnt]->caller);
		}
		if(cnt == node_count){
			rtd_pr_rmm_info("func(%s) total allocated: %8x \n\n",name,func_alloc);
		}
		rtd_pr_rmm_info("total allocated: %8x \n", total_alloc);
	}

	return 0;
}

