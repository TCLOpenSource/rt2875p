// SPDX-License-Identifier: GPL-2.0
/*
 * ION DVR heap exporter
 *
 * Copyright (C) 2021 Realtek, Inc.
 *
 */

#define pr_fmt(fmt) "ion_dvr: " fmt

#include <linux/device.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/dma-mapping.h>
#include <linux/pageremap.h>
#include <rtk_kdriver/rmm/rmm.h>
#include <linux/vmalloc.h>
#include <linux/highmem.h>
#include <linux/io.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <rtd_log/rtd_module_log.h>
#include <linux/jiffies.h>
#include <linux/seq_file.h>
#include <linux/dma-buf.h>
#include <linux/ion.h>

#include <rtk_kdriver/io.h>


#define ION_DVR_ALLOCATE_FAILED -1


// note: #define ION_FLAG_CACHED_NEEDS_SYNC 2   in android user space 

#define ION_FLAG_FOR_VE (0x80)  //bit 7
#define ION_FLAG_FOR_FM (0x40)  //bit 6
#define ION_FLAG_FOR_MULTI (0x10)  //bit 5



#if 0
	//use 4M as a unit log_2(4M) = 22

const int  COUNT_SHIFT = 22;

#define DVR_CHUNKS_NUM (3)

#elif 0
// 2MB 
const int  COUNT_SHIFT = 21;

#define DVR_CHUNKS_NUM (8 )


#else
  // 8M 
const int  COUNT_SHIFT = 23;

#define DVR_CHUNKS_NUM (3)


#endif//

unsigned long  DVR_CHUNK_UNIT_SIZE = (1u << COUNT_SHIFT);




#define ION_DEBUG_PRINT(level,fmt, arg...) do { \
       if( unlikely(gION_DVR_DEBUG >= level) ) { rtd_pr_rmm_emerg("TT:%lld "  fmt, ktime_to_us(ktime_get()), ##arg ); } \
   } while(0)
   

static int gION_DVR_DEBUG = 0;

struct device null_dev[1] = {0};

struct ion_dvr_heap {
	struct ion_heap heap;
	struct device *dev;
};

#define to_dvr_heap(x) container_of(x, struct ion_dvr_heap, heap)

struct ion_dvr_buffer_info {
	struct page *dvr_page;
	unsigned long phys_addr;
	unsigned long flags;
	struct sg_table *table;

	unsigned long ve_addr;
	unsigned long ve_addr_2;
	int from_vbm;

	unsigned long buf_size;
	unsigned int chunk_count ; 
	struct page* chunk_pages[DVR_CHUNKS_NUM];	  //first dvr_malloc still store in dvr_page
	unsigned long chunk_size[DVR_CHUNKS_NUM];

};

int ion_dvr_check_youtube_enter_enable = 0;



#ifdef ION_REC_BUFF_DEBUG

#include <linux/semaphore.h>

static unsigned long ion_dvr_start_time = 0;

#if (defined(CONFIG_FB_720P))
#define FB_SIZE 1280 * 720 * 4
#elif (defined(CONFIG_FB_1080P))
#define FB_SIZE 1920 * 1080 * 4
#elif (defined(CONFIG_FB_2160P))
#define FB_SIZE 3840 * 2160 * 4
#elif (defined(CONFIG_FB_768P))
#define FB_SIZE 1366 * 768 * 4
#else
#define FB_SIZE 1920 * 1080 * 4    // if no config is set, use 1080p as default
#endif

#define MAX_FB_NUM_PER_TASK       8
#define MAX_TASK_CMA_ALLOC_SIZE       (MAX_FB_NUM_PER_TASK * FB_SIZE)

#define MAX_REC_NUM       512 //128

#define ION_DEFER_FREE_WAIT_SECS       (5 * 60)  // 5 minutes



typedef struct {
	struct list_head list;

	char task_name[TASK_COMM_LEN];
	unsigned long num;
	unsigned long total_cma;

	unsigned long cma_status[MAX_REC_NUM]; //0 : no use
	unsigned long cma_ptr[MAX_REC_NUM];
	unsigned long phys_addr[MAX_REC_NUM];
	unsigned long size[MAX_REC_NUM];
	unsigned long pid[MAX_REC_NUM];
	unsigned long timestamp[MAX_REC_NUM];
} ion_rec_buff;

typedef struct {
	struct list_head list;
	unsigned int qlen;
} ion_rec_buff_head;

static int ion_rec_init=0;

struct semaphore ion_sem;

ion_rec_buff_head ion_rec_queue;


void ion_rec_queue_head_init(ion_rec_buff_head *head)
{
	INIT_LIST_HEAD(&head->list);
	head->qlen = 0;
}

void ion_rec_queue_head(ion_rec_buff_head *head, ion_rec_buff *ion_cmb)
{
	list_add(&ion_cmb->list, &head->list);
	head->qlen++;
}

void ion_rec_dequeue_node(ion_rec_buff_head *head, char *task_name)
{
	ion_rec_buff *ion_rec_temp = NULL;

	ion_rec_temp = list_entry(head->list.next, ion_rec_buff, list);

	while (ion_rec_temp->list.next != &head->list)
	{
		if (!strncmp(task_name, ion_rec_temp->task_name, TASK_COMM_LEN))
		{
			list_del_init(&ion_rec_temp->list);
			head->qlen--;
			break;
		}
		ion_rec_temp = list_entry(ion_rec_temp->list.next, ion_rec_buff, list);
	}
}

ion_rec_buff *ion_rec_find_node(ion_rec_buff_head *head, char *task_name)
{
	ion_rec_buff *ion_rec_temp = NULL;
	int ret = 0;
	int i;

	ion_rec_temp = list_entry(head->list.next, ion_rec_buff, list);

	while (ion_rec_temp->list.next != &head->list)
	{
		if (ion_rec_temp->task_name == NULL) {
			rtd_pr_rmm_err("[ION_REC] ERROR!! task_name is NULL\nList items of this node:\n");
			for (i = 0; i < MAX_REC_NUM; i++) {
				if (ion_rec_temp->cma_ptr[i] != 0) {
					rtd_pr_rmm_err("ION cma[%lx] status[%lx] phys_addr[%lx] size[%lx] task_name[%s] PID[%ld] ts[%ld.%03ld sec]\n",
						ion_rec_temp->cma_ptr[i], ion_rec_temp->cma_status[i],
						ion_rec_temp->phys_addr[i], ion_rec_temp->size[i],
						ion_rec_temp->task_name, ion_rec_temp->pid[i],
						ion_rec_temp->timestamp[i] / 1000, ion_rec_temp->timestamp[i] % 1000);
				}
			}
		}

		if ((ion_rec_temp->task_name != NULL) && (!strncmp(task_name, ion_rec_temp->task_name, TASK_COMM_LEN)))
		{
			ret = 1;
			rtd_pr_rmmc_debug("func=%s line=%d  ion_rec_temp->task_name=%s task_name=%s\n", __FUNCTION__, __LINE__,
				ion_rec_temp->task_name, task_name);
			break;
		}
		ion_rec_temp = list_entry(ion_rec_temp->list.next, ion_rec_buff, list);
	}
	if(ret)
		return ion_rec_temp;

	return NULL;
}

ion_rec_buff *ion_rec_find_removed_item(ion_rec_buff_head *head, unsigned long cma_ptr)
{
	ion_rec_buff *ion_rec_temp = NULL;
	int i,ret=0;
	rtd_pr_rmmc_debug("func=%s line=%d cma_ptr=%lx\n",__FUNCTION__,__LINE__,cma_ptr);
	ion_rec_temp = list_entry(head->list.next, ion_rec_buff, list);

	while (ion_rec_temp->list.next != &head->list)
	{
		//find the empty
		for (i = 0; i < MAX_REC_NUM; i++) {
			if ((ion_rec_temp->cma_ptr[i] == cma_ptr) && (ion_rec_temp->cma_status[i] == 1)) {
				ion_rec_temp->cma_status[i] = 0;
				ion_rec_temp->num--;
				ion_rec_temp->total_cma -= ion_rec_temp->size[i];
				rtd_pr_rmmc_debug("func=%s line=%d task_name=%s pid=%ld size=0x%lx phys_addr=0x%lx\n", __FUNCTION__, __LINE__,
						ion_rec_temp->task_name, ion_rec_temp->pid[i], ion_rec_temp->size[i],ion_rec_temp->phys_addr[i]);
				ret = 1;
				goto del_exit;
			}
		}
		ion_rec_temp = list_entry(ion_rec_temp->list.next, ion_rec_buff, list);
	}

del_exit:
	if(ret)
		return ion_rec_temp;

	return NULL;
}

void ion_rec_print(struct seq_file *s)
{
	ion_rec_buff *ion_rec_temp = NULL;
	int i;
	ion_rec_buff_head *head= &ion_rec_queue;

	if(!ion_rec_init)
		return;

	down(&ion_sem);
	ion_rec_temp = list_entry(head->list.next, ion_rec_buff, list);

	if (s != NULL) {
		seq_printf(s, "====== ION cma record ======\n\n");
	} else {
		rtd_pr_rmm_err("====== ION cma record ======\n\n");
	}

	while (ion_rec_temp->list.next != &head->list)
	{
		for (i = 0; i < MAX_REC_NUM; i++) {
			if ((ion_rec_temp->cma_ptr[i] != 0) && (ion_rec_temp->cma_status[i] != 0)) {
				if (s != NULL) {
					seq_printf(s, "ION cma[%lx] status[%lx] phys_addr[%lx] size[%lx] task_name[%s] PID[%ld] ts[%ld.%03ld sec]\n",
						ion_rec_temp->cma_ptr[i], ion_rec_temp->cma_status[i],
						ion_rec_temp->phys_addr[i], ion_rec_temp->size[i],
						ion_rec_temp->task_name, ion_rec_temp->pid[i],
						ion_rec_temp->timestamp[i] / 1000, ion_rec_temp->timestamp[i] % 1000);
				} else {
					rtd_pr_rmm_err("ION cma[%lx] status[%lx] phys_addr[%lx] size[%lx] task_name[%s] PID[%ld] ts[%ld.%03ld sec]\n",
						ion_rec_temp->cma_ptr[i], ion_rec_temp->cma_status[i],
						ion_rec_temp->phys_addr[i], ion_rec_temp->size[i],
						ion_rec_temp->task_name, ion_rec_temp->pid[i],
						ion_rec_temp->timestamp[i] / 1000, ion_rec_temp->timestamp[i] % 1000);
				}
			}
		}
		ion_rec_temp = list_entry(ion_rec_temp->list.next, ion_rec_buff, list);
	}
	up(&ion_sem);

	return;
}
EXPORT_SYMBOL(ion_rec_print);

ion_rec_buff *alloc_ion_cmb(char *task_name, unsigned long pid, unsigned long cma_ptr, unsigned long phys_addr, unsigned long size)
{
	ion_rec_buff *ion_cmb = (ion_rec_buff *) kmalloc(sizeof(ion_rec_buff), GFP_KERNEL);

	if (ion_cmb) {
		INIT_LIST_HEAD(&ion_cmb->list);

		memset(ion_cmb->task_name,'\0',TASK_COMM_LEN);
		memset(ion_cmb->cma_status,0x0,MAX_REC_NUM*sizeof(unsigned long));
		memset(ion_cmb->cma_ptr,0x0,MAX_REC_NUM*sizeof(unsigned long));
		memset(ion_cmb->phys_addr,0x0,MAX_REC_NUM*sizeof(unsigned long));
		memset(ion_cmb->size,0x0,MAX_REC_NUM*sizeof(unsigned long));

		strncpy(ion_cmb->task_name, task_name, TASK_COMM_LEN-1);
		ion_cmb->num = 1;
		ion_cmb->total_cma = size;

		ion_cmb->cma_status[0] = 1;
		ion_cmb->cma_ptr[0] = cma_ptr;
		ion_cmb->phys_addr[0] = phys_addr;
		ion_cmb->size[0] = size;
		ion_cmb->pid[0] = pid;
		ion_cmb->timestamp[0] = jiffies_to_msecs(jiffies - ion_dvr_start_time);
	} else {
		rtd_pr_rmm_err("[ION_REC]!!! ERROR !!! No Memory for task %s (pid=%ld) \n", task_name, pid);
	}

	return ion_cmb;
}

void kfree_ion_cmb(ion_rec_buff *ion_cmb)
{
	if (ion_cmb)
		kfree(ion_cmb);
}

void ion_cmb_add_rec(ion_rec_buff *ion_cmb, char *task_name, unsigned long pid, unsigned long cma_ptr, unsigned long phys_addr, unsigned long size)
{
	int i, j;

	if (strncmp(task_name, ion_cmb->task_name, TASK_COMM_LEN)) {
		rtd_pr_rmm_err("[ION_REC]!!! ERROR !!! func=%s line=%d task_name not match, ion_cmb->task_name=%s task_name=%s\n", __FUNCTION__, __LINE__,
			ion_cmb->task_name, task_name);
		return;
	}

	//find the empty
	for (i = 0; i < MAX_REC_NUM; i++) {
		if (ion_cmb->cma_status[i] == 0)
			break;
	}

	if (i >= MAX_REC_NUM) {
		rtd_pr_rmm_err("[ION_REC]!!! ERROR !!! over %d rec task_name = %s\n", MAX_REC_NUM, task_name);
		// List all ION CMA of this task "if too many ION CMA"
		rtd_pr_rmm_err("[ION_REC] Too many ION CMA of this task, list all ION CMA for task (%s)\n", ion_cmb->task_name);
		for (j = 0; j < MAX_REC_NUM; j++) {
			rtd_pr_rmm_err("[ION_REC] ION cma[%lx] phys_addr[%lx] size[%lx] task_name[%s] PID[%ld] ts[%ld.%03ld sec]\n",
					ion_cmb->cma_ptr[j], ion_cmb->phys_addr[j], ion_cmb->size[j],
					ion_cmb->task_name, ion_cmb->pid[j],
					ion_cmb->timestamp[j] / 1000, ion_cmb->timestamp[j] % 1000);
		}

		rtd_pr_rmm_err("\n[ION_REC] Dump CMA usage to verify\n");
		list_all_rtk_memory_allocation_sort(list_mem_generic, NULL, NULL);
		rtd_pr_rmm_err("\n[ION_REC] CMA usage dumped finished\n");

		return;
	}

	ion_cmb->cma_status[i] = 1;
	ion_cmb->cma_ptr[i] = cma_ptr;
	ion_cmb->phys_addr[i] = phys_addr;
	ion_cmb->size[i] = size;
	ion_cmb->pid[i] = pid;
	ion_cmb->timestamp[i] = jiffies_to_msecs(jiffies - ion_dvr_start_time);
	ion_cmb->num++;
	ion_cmb->total_cma += size;

	return ;
}

int ion_cmb_reclaim(ion_rec_buff *ion_cmb)
{
	int i = 0;
	int reclaim_times = 0;
	struct task_struct *ptask;
	unsigned long reclaim_timestamp = jiffies_to_msecs(jiffies - ion_dvr_start_time);

	// List all ION CMA of this task "before RECLAIM"
	rtd_pr_rmm_info("[ION_REC] Before RECLAIM, List all ION CMA for task (%s)\n", ion_cmb->task_name);
	for (i = 0; i < MAX_REC_NUM; i++) {
		if (ion_cmb->cma_status[i] == 1) {
			rtd_pr_rmm_info("[ION_REC] ION cma[%lx] phys_addr[%lx] size[%lx] task_name[%s] PID[%ld] ts[%ld.%03ld sec]\n",
					ion_cmb->cma_ptr[i], ion_cmb->phys_addr[i], ion_cmb->size[i],
					ion_cmb->task_name, ion_cmb->pid[i],
					ion_cmb->timestamp[i] / 1000, ion_cmb->timestamp[i] % 1000);
		}
	}

	for (i = 0; i < MAX_REC_NUM; i++) {
		if ((ion_cmb->cma_status[i] == 1) && (reclaim_timestamp / 1000 - ion_cmb->timestamp[i] / 1000 > ION_DEFER_FREE_WAIT_SECS)) {
			ptask = get_pid_task(find_pid_ns((int)ion_cmb->pid[i], &init_pid_ns), PIDTYPE_PID);

			if (!ptask) {
				// do reclaim
				rtd_pr_rmm_err("[ION_REC] Error!! pid (%ld) of task (%s) is not exist, Do Reclaim !!!\n", ion_cmb->pid[i], ion_cmb->task_name);
				/* release memory */
				dvr_free_page_mesg((struct page *)(ion_cmb->cma_ptr[i]));
				ion_cmb->cma_status[i] = 0;
				ion_cmb->num--;
				ion_cmb->total_cma -= ion_cmb->size[i];

				rtd_pr_rmm_err("[ION_REC] Free ION cma[%lx] phys_addr[%lx] size[%lx] task_name[%s] PID[%ld] ts[%ld.%03ld sec]\n",
						ion_cmb->cma_ptr[i], ion_cmb->phys_addr[i], ion_cmb->size[i],
						ion_cmb->task_name, ion_cmb->pid[i],
						ion_cmb->timestamp[i] / 1000, ion_cmb->timestamp[i] % 1000);

				reclaim_times++;
			}
		}
	}

	if (ion_cmb->num == 0) {
		rtd_pr_rmm_err("[ION_REC] After RECLAIM, All (%d) ION CMA of task (%s) are released,  cma = %ld !!!\n", reclaim_times, ion_cmb->task_name, ion_cmb->total_cma);
	} else {
		if (reclaim_times > 0) {
			// List all ION CMA of this task "after RECLAIM"
			rtd_pr_rmm_err("[ION_REC] RECLAIM finished, release %d ION CMA, list rest ION CMA for task (%s)\n", reclaim_times, ion_cmb->task_name);
			for (i = 0; i < MAX_REC_NUM; i++) {
				if (ion_cmb->cma_status[i] == 1) {
					rtd_pr_rmm_err("[ION_REC] ION cma[%lx] phys_addr[%lx] size[%lx] task_name[%s] PID[%ld] ts[%ld.%03ld sec]\n",
							ion_cmb->cma_ptr[i], ion_cmb->phys_addr[i], ion_cmb->size[i],
							ion_cmb->task_name, ion_cmb->pid[i],
							ion_cmb->timestamp[i] / 1000, ion_cmb->timestamp[i] % 1000);
				}
			}
		}
	}

	return 0;
}

#endif

/*
 * Create scatter-list for the already allocated DMA buffer.
 * This function could be replaced by dma_common_get_sgtable
 * as soon as it will avalaible.
 */
static int ion_dvr_get_sgtable(struct sg_table *sgt,
			       void *virt_addr, unsigned long phys_addr, size_t size, struct ion_dvr_buffer_info *info)
{
	int ret;

	int count = 0;
	struct page *page = phys_to_page(phys_addr);
		
	if( (info->flags & ION_FLAG_FOR_MULTI) == ION_FLAG_FOR_MULTI  ) 
	{
		count = 1 + info->chunk_count;


	}
	else
	{
		count = 1;

	}


	ret = sg_alloc_table(sgt, count, GFP_KERNEL);
	if (unlikely(ret))
		return ret;


	if( (info->flags & ION_FLAG_FOR_MULTI) == ION_FLAG_FOR_MULTI  )
	{
		struct scatterlist *sg = sgt->sgl;
		int cc = 0;
	

		if( size <=  DVR_CHUNK_UNIT_SIZE ) 
		{
			//not really multi ion
			sg_set_page( sg, page, (size), 0);

			ION_DEBUG_PRINT(7, "ionMulti single entry 0x%lx! page_to_addr: 0x%lx %ld\n", phys_addr, page_to_phys(page), size );
		}
		else 
		{
			sg_set_page( sg, page, (DVR_CHUNK_UNIT_SIZE), 0);
			
			ION_DEBUG_PRINT(7, "ionMulti sgSet0  0x%lx! size:%ld \n",   page_to_phys(page), (DVR_CHUNK_UNIT_SIZE) );

			sg =  sg_next(sg);
			
			for (cc= 0; cc < info->chunk_count; cc++) {

				page = info->chunk_pages[cc];

				if( page== NULL) {
					rtd_pr_rmm_crit( "ion_dvr_get_sgtable fail %d %ld\n", cc, info->chunk_size[cc] );
					goto err_set_sgtable;
				}

				if( info->chunk_size[cc] != DVR_CHUNK_UNIT_SIZE )
			 	{
					if( cc != info->chunk_count -1 )  {
						 //only last one can < DVR_CHUNK_UNIT_SIZE
						 rtd_pr_rmm_emerg("invalid ionMulti chunk algo %d %ld\n", cc,  info->chunk_size[cc] );
					}
			 	}
				
				//sg_set_page(sg, page, PAGE_ALIGN(info->chunk_size[cc]), 0);
				sg_set_page(sg, page, info->chunk_size[cc], 0);
				sg = sg_next(sg);

				ION_DEBUG_PRINT(7, "ionMulti sgSet idx:%d 0x%lx! size:%ld \n", cc, page_to_phys(page), info->chunk_size[cc] );
			}


		}
		


				
	}
	else
	{
		//org
		sg_set_page(sgt->sgl, page, (size), 0);

		ION_DEBUG_PRINT(7, "ion sgSet org 0x%lx! \n", page_to_phys(page) );
	}
	
	return 0;


	err_set_sgtable:


		return -1;
}

void ion_dvr_check_youtube_enter(int *data)
{
	*data = ion_dvr_check_youtube_enter_enable;
}
EXPORT_SYMBOL(ion_dvr_check_youtube_enter);

void ion_dvr_set_youtube_enter(int data)
{
	ion_dvr_check_youtube_enter_enable = data;
}

#ifdef ION_REC_BUFF_DEBUG
static int gSetNotify=0;
void ion_dvr_set_vNotify(int enable)
{

    if(enable == 0 ) {
        rtd_pr_rmm_warn("ion vNotify disable\n");
        rtd_clearbits(0xB806012C, BIT(0) );
    }
    else if(enable == 1 ) {
        rtd_pr_rmm_warn("ion vNotify enable\n");
        rtd_setbits(0xB806012C, BIT(0));
    }

    
}
#endif

void ion_dvr_enable_debug(int loglevel)
{
	gION_DVR_DEBUG = loglevel;
}

EXPORT_SYMBOL(ion_dvr_enable_debug);



#if 1 //  v_addr, g_addr
#ifdef CONFIG_ARCH_RTK2851A
	typedef int (*RT_ION_FREE_CALLBACK)(unsigned long, void*);
#else

typedef int (*RT_ION_FREE_CALLBACK)(unsigned long, uint64_t, void*);

#endif//


static DEFINE_MUTEX(callback_mutex);
static RT_ION_FREE_CALLBACK gFreeNotify = NULL;
int ion_free_register_callback(RT_ION_FREE_CALLBACK cb)
{
	mutex_lock(&callback_mutex);
	gFreeNotify = cb;
	mutex_unlock(&callback_mutex);

	return 0;
}
EXPORT_SYMBOL(ion_free_register_callback);
#endif


int ion_dvr_free_ve_addr(struct ion_buffer *buffer, unsigned long addr, unsigned long addr_2)
{
	if( buffer == NULL)
	{
		if( addr != 0 ) {
			pli_free_mesg(addr);

			ION_DEBUG_PRINT(2, "ionFree 0 FM00 addr:0x%lx \n", addr );
		}

		if( addr_2 != 0 ) {
			pli_free_mesg(addr_2);

			ION_DEBUG_PRINT(2, "ionFree 1 FM00 addr:0x%lx \n", addr_2 );
		}
		
		return 0;
	}
	else
	{
		//double check consistent ?
		struct ion_dvr_buffer_info *info = NULL;
		info = buffer->priv_virt;

		if(info == NULL) {
			rtd_pr_rmm_crit( "ionFree err check null\n", buffer, addr );

			return -5;
		}


		if( info->ve_addr != addr ) {
			rtd_pr_rmm_crit( "ionFree err check addr fail\n", buffer, addr );
			return -5;

		}

		#if 0 // calll decmp api to mark mem as free
		if( info->from_vbm ) {
			extern int ion_VbmDMAHeap_free_ve_addr(struct ion_buffer *buffer, unsigned long addr, unsigned long addr_2);

			if( ion_VbmDMAHeap_free_ve_addr(buffer, info->ve_addr, 0 ) == 0 ) {
				rtd_pr_rmm_crit( "kkk ionFree  vbm ret \n", buffer, info->ve_addr );
			}

			return 0;
		}

		#endif//

		//pli_free_mesg(addr);
		if( info->ve_addr != 0) {
			pli_free_mesg(info->ve_addr);
			ION_DEBUG_PRINT(3, " ionFree__ FM0 addr:0x%lx\n", info->ve_addr );
		}

		if( info->ve_addr_2 != 0 ) {
			pli_free_mesg(info->ve_addr_2);
			ION_DEBUG_PRINT(3, " ionFree___ FM0 addr:0x%lx\n", info->ve_addr_2 );
		}

		ION_DEBUG_PRINT(2, "ionFree  FM0 addr:0x%lx (0x%lx 0x%lx) %p %d  f:0x%x\n", info->phys_addr, info->ve_addr, info->ve_addr_2, buffer, 
			(uint32_t)buffer->size, buffer->flags );
		
		
		return 0;
	}
	

	return -1;
	
}
EXPORT_SYMBOL(ion_dvr_free_ve_addr);


unsigned long ion_dvr_alloc_ve_addr(int size)
{
	unsigned long phyaddr = 0;

	phyaddr = pli_malloc_mesg(size, GFP_DCU2_FIRST, "VE_R");
	
	if (phyaddr == INVALID_VAL)	{


		#if 0
			struct page *dvr_malloc_gpu_page(size_t size);

		#endif//

		rtd_pr_rmm_crit("alloc veAddr fail! %d\n", size);
		
		return 0;
	}

	ION_DEBUG_PRINT(2, "ionAlloc FMve addr:0x%lx 0x%x\n", phyaddr, size );
	
	return phyaddr;
}
EXPORT_SYMBOL( ion_dvr_alloc_ve_addr );


// note:  caller should validate consistent of 'buffer' pointer
int ion_dvr_set_ve_addr(struct ion_buffer *buffer, unsigned long ve_addr, unsigned long ve_addr_2 )
{
	struct ion_dvr_buffer_info *info = NULL;
	
	#if 0
	if (buffer->heap->type != ION_HEAP_TYPE_CUSTOM) {
		rtd_pr_rmm_crit("%s() not sutable heap type(%d) fd(%d)\n", __func__, buffer->heap->type, fd);
		goto err;
	}

	if (buffer->sg_table == NULL) {
		rtd_pr_rmm_err("%s() no sg table,  \n", __func__, buffer);
		goto err;
	}
	#endif//


	info = buffer->priv_virt;

	if(info == NULL) {
		rtd_pr_rmm_crit( "fail to set addr for p:%llx  0x%llx 0x%llx\n", buffer, ve_addr, ve_addr_2 );

		return -5;
	}

	if( ve_addr_2 == 0xFFFFFFF0 ) //ML7BU-1555
	{  //vbm caverout (dmabuf) 
		info->ve_addr = ve_addr;
		info->from_vbm = 1;
	}
	else {
		info->ve_addr = ve_addr;
		info->ve_addr_2 = ve_addr_2;

		info->from_vbm = 0;
	}


	return 0;
	
}
EXPORT_SYMBOL( ion_dvr_set_ve_addr );



/* ION CMA heap operations functions */
static int ion_dvr_allocate(struct ion_heap *heap, struct ion_buffer *buffer,
							unsigned long len, unsigned long flags)
{
	struct ion_dvr_heap *dvr_heap = to_dvr_heap(heap);
	struct ion_dvr_buffer_info *info = 0;
	unsigned int pid_flags = 0;
	unsigned char *name_ptr = 0;
	struct task_struct *ptask;
#ifdef ION_REC_BUFF_DEBUG
	unsigned char *task_name_ptr = 0;
	ion_rec_buff *ion_cmb = NULL;
#endif
	//struct pid_namespace tmp_pid_ns;
	struct scatterlist *sg; 	int i;


	rtd_pr_rmm_debug("Request buffer allocation len %ld\n", len);

//	if (buffer->flags & ION_FLAG_CACHED)
//		return -EINVAL;

	pid_flags = (flags & 0xffffff00)>>8;
	flags = flags & 0x000000ff;
	/* rtd_pr_rmm_info("func=%s line=%d pid_flags=%d \n",__FUNCTION__,__LINE__,pid_flags); */
	name_ptr = kmalloc(TASK_COMM_LEN*4,GFP_KERNEL);
	if(!name_ptr)
	    return -ENOMEM;

	memset(name_ptr,'\0',TASK_COMM_LEN);
	//strcpy(name_ptr,"ION_DRV:");
    snprintf(name_ptr, 20, "ION_DRV:%d:",pid_flags);

	// RTK_FIXME_64 : added for GPU:decomp
	if (buffer->flags & ION_FLAG_FOR_VE )
		snprintf(name_ptr, 20, "graVE:%u:",pid_flags);

	if (buffer->flags & ION_FLAG_FOR_FM )
		snprintf(name_ptr, 20, "graFM:%u:",pid_flags);

	if (buffer->flags & ION_FLAG_FOR_MULTI )
		snprintf(name_ptr, 20, "graMulti:%u:", pid_flags);
		

#ifdef ION_REC_BUFF_DEBUG
	task_name_ptr = kmalloc(TASK_COMM_LEN, GFP_KERNEL);
	if (!task_name_ptr)
	    goto err;
	memset(task_name_ptr, '\0', TASK_COMM_LEN);
#endif

	if(pid_flags != 0){
		ptask = get_pid_task(find_pid_ns((int)pid_flags, &init_pid_ns), PIDTYPE_PID);
		if (ptask){
		strncat(name_ptr, ptask->comm, (TASK_COMM_LEN*4 - strlen(name_ptr) - 1));
#ifdef ION_REC_BUFF_DEBUG
	        strncpy(task_name_ptr, ptask->comm, TASK_COMM_LEN);
#endif
		}
	}
	/* rtd_pr_rmm_info("func=%s line=%d name_ptr=%s \n",__FUNCTION__,__LINE__,name_ptr); */

	info = kzalloc(sizeof(struct ion_dvr_buffer_info), GFP_KERNEL);
	if (!info) {
		rtd_pr_rmm_err("Can't allocate buffer info\n");
		goto err;
	}
	memset(info, 0, sizeof(struct ion_dvr_buffer_info));
	info->flags = flags;

	info->buf_size = len;

	if (buffer->flags & ION_FLAG_FROM_DCU1) {
		info->dvr_page = dvr_malloc_page_mesg(len, GFP_DCU1, name_ptr);
		//info->dvr_page = dvr_malloc_page(len, GFP_DCU1);
	} else if (buffer->flags & ION_FLAG_FROM_DCU2) {
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
		info->dvr_page = dvr_malloc_page_mesg(len, GFP_HIGHMEM_BW_FIRST, name_ptr);
#else
		info->dvr_page = dvr_malloc_page_mesg(len, GFP_DCU2, name_ptr);
		//info->dvr_page = dvr_malloc_page(len, GFP_DCU2);
#endif
	} else if (buffer->flags & ION_FLAG_FOR_FM )
	{
		#ifdef CONFIG_FAKEMEMORY_GRAPHICS
			info->dvr_page = dvr_malloc_gpu_page(len);

			//rtd_pr_rmm_crit("KKK ion FM addr: 0x%llx  size:%ld\n", page_to_phys(info->dvr_page), len );

			ION_DEBUG_PRINT(2, "ionAlloc FMmem0 %p %d phy(0x%lx)\n", buffer, (uint32_t)buffer->size, page_to_phys(info->dvr_page) );

		#else
			info->dvr_page = dvr_malloc_page_mesg(len, GFP_DCU2_FIRST, name_ptr);

			ION_DEBUG_PRINT(2, "ionAlloc FMmem1 %p %d phy(0x%lx)\n", buffer, (uint32_t)buffer->size, page_to_phys(info->dvr_page) );

		#endif//CONFIG_FAKEMEMORY_GRAPHICS

		
	} 
	else if ( (buffer->flags & ION_FLAG_FOR_MULTI)  )
	{
		unsigned long cur_length = len;
		
		int ii = 0;
		unsigned int unit_size = DVR_CHUNK_UNIT_SIZE;
		
		unsigned long count_unit = (cur_length + DVR_CHUNK_UNIT_SIZE -1)  >> COUNT_SHIFT;

		 // reset info->flags  ION_FLAG_FOR_MULTI ?

		if( cur_length <= DVR_CHUNK_UNIT_SIZE ) {
			unit_size = cur_length;
		}
		
		info->dvr_page = dvr_malloc_page_mesg(unit_size, GFP_DCU2_FIRST, name_ptr);

		if( info->dvr_page == NULL ) {
			rtd_pr_rmm_err("Can't allocate buffer for ION MULTI:%lu %lu\n", unit_size, cur_length);
			goto err;
		}

		ION_DEBUG_PRINT(3, "ionAlloc Multi org addr:0x%lx (0x%lx) %p %d  f:0x%x\n", page_to_phys(info->dvr_page),  buffer, (uint32_t)unit_size, buffer->flags );

		//--count; // first count is original dvr
		cur_length -= unit_size;

		if( (count_unit -1) > DVR_CHUNKS_NUM ) {
			rtd_pr_rmm_crit(" ionMulti can't allocate > %d  chunks  inSize:%lu\n", count_unit,  len);
			goto err;
		}

		for( ii=0; ii < count_unit -1 ; ++ii)
		{

			unit_size = DVR_CHUNK_UNIT_SIZE;
			
			if( cur_length <= DVR_CHUNK_UNIT_SIZE ) {
				unit_size = cur_length;
			}
			
			{
				name_ptr[10] = ii;
				info->chunk_pages[ii] = dvr_malloc_page_mesg(unit_size, GFP_DCU2_FIRST, name_ptr);

				info->chunk_size[ii] = unit_size;
			}
			

			if( info->chunk_pages[ii] == NULL) {
				
				rtd_pr_rmm_crit("Can't allocate buffer for ION MULTI:%lu idx:%d %ld %ld \n", info->chunk_size[ii], ii, cur_length, unit_size);
				info->chunk_size[ii] = 0;
				
				goto err;
			}


			cur_length -= info->chunk_size[ii];

			
			ION_DEBUG_PRINT(5, "ionAlloc Multi idx:%d ok  phy:0x%lx, size:%lu rem:%ld total:%ld\n", ii, page_to_phys(info->chunk_pages[ii]), info->chunk_size[ii], 
				cur_length, len );
			
		}

		if( count_unit > 0)
			info->chunk_count = count_unit - 1; //skip original info->dvr_page
		
	}
	
	else {
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
		info->dvr_page = dvr_malloc_page_mesg(len, GFP_HIGHMEM_BW_FIRST, name_ptr);
#else
		info->dvr_page = dvr_malloc_page_mesg(len, GFP_DCU2_FIRST, name_ptr);
		//info->dvr_page = dvr_malloc_page(len, GFP_DCU2_FIRST);
#endif
	}
	

	if (name_ptr) {
		kfree(name_ptr);
		name_ptr = NULL;
	}
	if (!info->dvr_page) {
		rtd_pr_rmm_err("Fail to allocate buffer\n");
		goto err;
	}

#ifdef ION_REC_BUFF_DEBUG
	strncpy(buffer->caller_task, task_name_ptr, (TASK_COMM_LEN-1));
	buffer->caller_pid = pid_flags;
	rtd_pr_rmmc_debug("[ION] %s (%d) calls ion_dvr_allocate, use buffer %p\n", buffer->caller_task, buffer->caller_pid, buffer);

	if (ion_rec_init) {
		down(&ion_sem);
		ion_cmb = ion_rec_find_node(&ion_rec_queue, task_name_ptr);
		rtd_pr_rmmc_debug("func=%s line=%d ion_cmb=%p task_name=%s pid_flags=%d info->dvr_page=%p phys_addr=%lx\n", __FUNCTION__, __LINE__,
			ion_cmb, task_name_ptr, pid_flags, info->dvr_page, page_to_pfn(info->dvr_page) << PAGE_SHIFT);

		if (ion_cmb) {
			ion_cmb_add_rec(ion_cmb, task_name_ptr, pid_flags, (unsigned long)info->dvr_page, page_to_pfn(info->dvr_page) << PAGE_SHIFT, len);
		} else {
			if (!task_name_ptr) {
				rtd_pr_rmm_err("[ION_REC] Strange!! Task name is null, but it will insert as ION record node !!!\n");
			}
			ion_cmb = alloc_ion_cmb(task_name_ptr, pid_flags, (unsigned long)info->dvr_page, page_to_pfn(info->dvr_page) << PAGE_SHIFT, len);
			if(ion_cmb) {
				rtd_pr_rmmc_debug("func=%s line=%d Insert node, ion_cmb=%p pid_flags=%d info->dvr_page=%p phys_addr=%lx\n",__FUNCTION__,__LINE__,ion_cmb,pid_flags,info->dvr_page,page_to_pfn(info->dvr_page) << PAGE_SHIFT);
				ion_rec_queue_head(&ion_rec_queue,ion_cmb);
			}

            if(!strcmp(task_name_ptr, "test-opengl")) {
    	    	gSetNotify = 1;
                ion_dvr_set_vNotify(1);
            }
		}

		if(!strcmp(task_name_ptr,"roid.youtube.tv"))
		{
			ion_dvr_set_youtube_enter(1);
		}

		up(&ion_sem);
	}

	if (task_name_ptr) {
		kfree(task_name_ptr);
		task_name_ptr = NULL;
	}
#endif

	info->phys_addr = page_to_phys(info->dvr_page);

		

	info->table = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
	if (!info->table) {
		rtd_pr_rmm_err("Fail to allocate sg table\n");
		goto err;
	}

	if (ion_dvr_get_sgtable(info->table, NULL, info->phys_addr, len, info))
		goto err;

	/* keep this for memory release */
	buffer->priv_virt = info;
	buffer->sg_table = info->table;
	rtd_pr_rmm_debug("dvr ion alloc 0x%lx %ld \n", info->phys_addr, len);

	buffer->phy_address= info->phys_addr; // RTK_FIXME_64 : added for GPU:decomp


	for_each_sg(info->table->sgl, sg, info->table->nents, i) 
	{
		struct page *page = sg_page(sg);
		unsigned long sg_len = sg->length;


		if ( !(buffer->flags & ION_FLAG_FOR_FM ) )
		{
			
			if (PageHighMem( page )) {
				unsigned long nr_clear_pages = PAGE_ALIGN(sg_len) >> PAGE_SHIFT;

				while (nr_clear_pages > 0) {
					void *vaddr = kmap_atomic(page);

					memset(vaddr, 0, PAGE_SIZE);
					kunmap_atomic(vaddr);
					page++;
					nr_clear_pages--;
				}
			} else {
				//memset( page_address(page), 0, PAGE_ALIGN(sg_len) );
				memset( page_address(page), 0, sg_len );

			}
			
			//dma_sync_single_for_cpu(dvr_heap->dev, page_to_phys(page), PAGE_ALIGN(sg_len), DMA_BIDIRECTIONAL);
			dma_sync_single_for_cpu(dvr_heap->dev, page_to_phys(page), sg_len, DMA_BIDIRECTIONAL);
			
		}


		ION_DEBUG_PRINT(8, "ionAlloc clear idx:%d phy:0x%x, size:%lu\n", i, page_to_phys( page ), sg_len);
	}


	

	return 0;

err:
	if (info) {
		if (info->table) {
			sg_free_table(info->table);
			
			kfree(info->table);
		}
		if (info->dvr_page) {

			if (buffer->flags & ION_FLAG_FOR_FM ) {

				#ifdef CONFIG_FAKEMEMORY_GRAPHICS
					
						dvr_free_gpu_page(info->dvr_page);
				#else

						dvr_free_page_mesg(info->dvr_page);
				#endif//

			}
			else {
				dvr_free_page_mesg(info->dvr_page);

				ION_DEBUG_PRINT(5, "kkk2 ionFail free pfn:%d phy:0x%lx, size:%lu\n", page_to_pfn(info->dvr_page), page_to_phys(info->dvr_page), len );
			}
		}
		
		
		if ( info->chunk_pages[0] != NULL )
		{
			int idd = 0;
			
			if(  (buffer->flags & ION_FLAG_FOR_MULTI) == 0  ) 
			{
				rtd_pr_rmm_crit("invalid ion_multi case!  %p %ld 0x%x\n", info->phys_addr, info->buf_size, info->flags );
			}


			for( idd=0; idd < DVR_CHUNKS_NUM; ++idd )
			{
			
				if( info->chunk_pages[idd]  != NULL ) {
					
					if( info->chunk_size[idd] <= 0 ) {
						rtd_pr_rmm_crit("invalid ion_multi error %p %ld 0x%x %ld\n", info->phys_addr, info->buf_size, info->flags, info->chunk_size[idd] );
					}

					dvr_free_page_mesg( info->chunk_pages[idd] );
					
				}


			}
		}
		


		
		kfree(info);
	}
#ifdef ION_REC_BUFF_DEBUG
	if (task_name_ptr) {
		kfree(task_name_ptr);
	}
#endif
	if (name_ptr) {
		kfree(name_ptr);
	}

	rtd_pr_rmm_err("Fail to allocate dvr %p %ld flags:0x%lx\n", buffer, len, buffer->flags );
	
	return -ENOMEM;
}

static void ion_dvr_free(struct ion_buffer *buffer)
{
	struct ion_dvr_buffer_info *info = buffer->priv_virt;

	unsigned long ve_addr = info->ve_addr ;
	unsigned long ve_addr_2 = info->ve_addr_2 ;


#ifdef ION_REC_BUFF_DEBUG
	ion_rec_buff_head *head;
	ion_rec_buff *ion_cmb = NULL;

	rtd_pr_rmmc_debug("[ION] %s (%d) calls ion_dvr_free, release buffer %p\n", buffer->caller_task, buffer->caller_pid, buffer);

	if (ion_rec_init) {
		down(&ion_sem);
		ion_cmb = ion_rec_find_removed_item(&ion_rec_queue, (unsigned long)info->dvr_page);
		if (!ion_cmb) {
			rtd_pr_rmm_err("[ION_REC] ERROR!! Could not find func=%s line=%d cma_ptr=%p\n", __FUNCTION__, __LINE__, info->dvr_page);
			up(&ion_sem);
			ion_rec_print(NULL);
			goto free_mem;
		} else {
			rtd_pr_rmmc_debug("find func=%s line=%d task_name=%s num=%ld total_cma=%lx\n", __FUNCTION__, __LINE__,
				ion_cmb->task_name, ion_cmb->num, ion_cmb->total_cma);
		}
		if (ion_cmb->num == 0) {
			if(!strcmp(ion_cmb->task_name,"roid.youtube.tv"))
			{
				ion_dvr_set_youtube_enter(0);
			}

            if(!strcmp(ion_cmb->task_name,"test-opengl") && gSetNotify != 0){
				gSetNotify = 0;
                ion_dvr_set_vNotify(0);
            }

            
			rtd_pr_rmmc_debug("ion_cmb->num = 0, dequeue ion node (task_name = %s)\n", ion_cmb->task_name);
			ion_rec_dequeue_node(&ion_rec_queue, ion_cmb->task_name);
			kfree_ion_cmb(ion_cmb);
		}
		up(&ion_sem);
	}

free_mem:
#endif

	rtd_pr_rmm_debug("Release buffer %lx, phy(0x%lx)\n", (unsigned long)buffer, (unsigned long)info->phys_addr);
	/* release memory */


	{
		int flags = buffer->flags;
		unsigned long phy_addr = buffer->phy_address;
		
	
		if (gFreeNotify != NULL && ( (flags & ION_FLAG_FOR_VE) ||  (flags & ION_FLAG_FOR_FM)  )
			)
		{
			#ifdef CONFIG_ARCH_RTK2851A
				gFreeNotify(phy_addr, NULL);

			#else
			gFreeNotify(ve_addr, phy_addr, NULL);
			#endif//
			
		}
			
	}
	
	if ( buffer->flags & ION_FLAG_FOR_FM ) {

		#ifdef CONFIG_FAKEMEMORY_GRAPHICS
		
			dvr_free_gpu_page(info->dvr_page);

			//if( ve_addr != 0 ) 
			{
				ion_dvr_free_ve_addr( buffer,  ve_addr, ve_addr_2 );
			}

			

			
		#else
			ION_DEBUG_PRINT(2, "ionFree FM1 addr:0x%lx (0x%lx 0x%lx) %p %d  f:0x%x\n", info->phys_addr, ve_addr, ve_addr_2, buffer, (uint32_t)buffer->size, buffer->flags );

			dvr_free_page_mesg(info->dvr_page);

			
		#endif//

		
		
	}
	else {
		dvr_free_page_mesg(info->dvr_page);

		ION_DEBUG_PRINT(2, "ionFree norm addr:0x%x (0x%x) %p %d  f:0x%x\n", info->phys_addr, ve_addr, buffer, (uint32_t)buffer->size, buffer->flags );

	}

	if ( info->chunk_pages[0] != NULL )
	{
		int idd = 0;
		
		if(  (buffer->flags & ION_FLAG_FOR_MULTI) == 0  ) 
		{
			rtd_pr_rmm_alert("ERROR invalid ion_multi when free !  %p %ld 0x%x\n", info->phys_addr, info->buf_size, info->flags );
		}


		for( idd=0; idd < DVR_CHUNKS_NUM; ++idd )
		{
		
			if( info->chunk_pages[idd]  != NULL ) {
				
				if( info->chunk_size[idd] <= 0 ) {
					rtd_pr_rmm_crit("ERROR invalid ion_multi size. idx:%d when free %p %ld 0x%x %ld\n", idd, info->phys_addr, info->buf_size, info->flags, info->chunk_size[idd] );
				}

				dvr_free_page_mesg( info->chunk_pages[idd] );
				
				ION_DEBUG_PRINT(5, "kkk2 ionMulti free idx:%d ok  phy:0x%lx, size:%lu\n", idd, page_to_phys(info->chunk_pages[idd]), info->chunk_size[idd]);
			}


		}
	}

	
	//dvr_free_page(info->dvr_page);
	/* release sg table */
	sg_free_table(info->table);
	buffer->sg_table = NULL;
	kfree(info->table);
	kfree(info);

#ifdef ION_REC_BUFF_DEBUG
	if (ion_reclaim_enable == 1) {
		/* Do ION reclaim check */
		if(!ion_rec_init)
			return;

		down(&ion_sem);
		head = &ion_rec_queue;
		ion_cmb = list_entry(head->list.next, ion_rec_buff, list);
		while (ion_cmb->list.next != &head->list)
		{
			if (ion_cmb->total_cma > MAX_TASK_CMA_ALLOC_SIZE) {
				rtd_pr_rmm_warn("[ION_REC] %s alloc CMA %lx > %x, check if this task has ION leak\n", ion_cmb->task_name, ion_cmb->total_cma, MAX_TASK_CMA_ALLOC_SIZE);
				ion_cmb_reclaim(ion_cmb);

				if ((ion_cmb->num == 0) && (ion_cmb->total_cma == 0)) {
					rtd_pr_rmm_err("[ION_REC] After RECLAIM, task (%s) has no ION left, Dequeue it !!!\n", ion_cmb->task_name);
					ion_rec_dequeue_node(&ion_rec_queue, ion_cmb->task_name);
					kfree_ion_cmb(ion_cmb);
					rtd_pr_rmm_err("[ION_REC] Dequeue finished, break......\n");
					break;
				}
			}
			ion_cmb = list_entry(ion_cmb->list.next, ion_rec_buff, list);
		}
		up(&ion_sem);
	}
#endif

}


static int dvr_heap_map_user(struct ion_dvr_buffer_info *info, struct ion_buffer *buffer,
			  struct vm_area_struct *vma)
{
	struct sg_table *table = buffer->sg_table;
	unsigned long addr = vma->vm_start;
	unsigned long offset = vma->vm_pgoff * PAGE_SIZE;
	struct scatterlist *sg;
	int i;
	int ret;

	

	for_each_sg(table->sgl, sg, table->nents, i) {
		struct page *page = sg_page(sg);
		unsigned long remainder = vma->vm_end - addr;
		unsigned long len = sg->length;

		//ION_DEBUG_PRINT(8, "  sg idx:%d phy:0x%x len:%ld offset:%ld\n", i, page_to_phys(page), len, offset );


		if (offset >= sg->length) {
			offset -= sg->length;
			continue;
		} else if (offset) {
			page += offset / PAGE_SIZE;
			len = sg->length - offset;
			offset = 0;
		}
		len = min(len, remainder);
		ret = remap_pfn_range(vma, addr, page_to_pfn(page), len,
					  vma->vm_page_prot);
		if (ret)
			return ret;

		//ION_DEBUG_PRINT(8, "  sgremap_pfn_range idx:%d phy:0x%x len:%ld addr:0x%x\n", i, page_to_phys(page), len, addr );
		
		addr += len;
		if (addr >= vma->vm_end)
			return 0;
	}

	return 0;
}



static int ion_dvr_mmap(struct dma_buf *dmabuf, struct vm_area_struct *vma)
{
	struct ion_buffer *buffer = dmabuf->priv;
	struct ion_dvr_buffer_info *info = buffer->priv_virt;
	int ret = -ENXIO;

	#if 0
	unsigned long user_count = (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;
	unsigned long count = PAGE_ALIGN(buffer->size) >> PAGE_SHIFT;
	unsigned long pfn = info->phys_addr >> PAGE_SHIFT;
	unsigned long off = vma->vm_pgoff;

	

	if (!(info->flags & ION_FLAG_CACHED)) {
		#if 0
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
		#else
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
		#endif
	}

	if (off < count && user_count <= (count - off)) {
		rtd_pr_rmm_debug("Map %lx to %lx size %lx\n", 
			(pfn + off) << PAGE_SHIFT, vma->vm_start, user_count << PAGE_SHIFT);
	        ret = remap_pfn_range(vma, vma->vm_start,
	                              pfn + off,
	                              user_count << PAGE_SHIFT,
	                              vma->vm_page_prot);
	}
	#else

		mutex_lock(&buffer->lock);

		if (!(info->flags & ION_FLAG_CACHED)) {
			#if 0
			vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
			#else
			vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
			#endif
		}


		ret = dvr_heap_map_user(info, buffer, vma);



		mutex_unlock(&buffer->lock);


	#endif//

	return ret;
}



static struct ion_heap_ops ion_dvr_ops = {
	.allocate = ion_dvr_allocate,
	.free = ion_dvr_free,
};

//static struct dma_buf_ops ion_dvr_buf_ops = {
//	.mmap = ion_dvr_mmap,
//
//};

static struct ion_dvr_heap dvr_heap = {
	.heap = {
		.ops = &ion_dvr_ops,
		//.buf_ops = &ion_dvr_buf_ops,
		//in 5.4 .buf_ops isn't a pointer like .ops but a struct
		.buf_ops = {.mmap = ion_dvr_mmap,},
		
		.type = ION_HEAP_TYPE_CUSTOM,
		.name = "ion_dvr_heap",
	},
};

static int __init ion_dvr_heap_init(void)
{
#ifdef ION_REC_BUFF_DEBUG
	if (!ion_rec_init) {
		ion_rec_buff *ion_cmb = NULL;

		ion_rec_queue_head_init(&ion_rec_queue);
		sema_init(&ion_sem, 1);

		// insert as the tail
		ion_dvr_start_time = jiffies;
		ion_cmb = alloc_ion_cmb("ION_TAIL", 0, 0, 0, 0);
		if (ion_cmb) {
			ion_rec_queue_head(&ion_rec_queue, ion_cmb);
		}

		ion_rec_init = 1;
	}
#endif
	dvr_heap.dev = null_dev;
	return ion_device_add_heap(&dvr_heap.heap);
}

static void __exit ion_dvr_heap_exit(void)
{
	ion_device_remove_heap(&dvr_heap.heap);
}

module_init(ion_dvr_heap_init);
module_exit(ion_dvr_heap_exit);
MODULE_LICENSE("GPL v2");
