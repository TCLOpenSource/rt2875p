// SPDX-License-Identifier: GPL-2.0
/*
 * DMABUF VBM heap exporter
 *
 * Copyright (C) 2021 Realtek, Inc.
 *
 */

#define pr_fmt(fmt) "dmabuf_vbm: " fmt

#ifdef BUILD_QUICK_SHOW

// QuickShow only required heap chunk information sync'd
#include <rtk_kdriver/rmm/rmm.h>
#include <rtd_log/rtd_module_log.h>

#else // BUILD_QUICK_SHOW

#include <linux/version.h>
#include <linux/of.h>
//#include <linux/of_fdt.h>
#include <linux/cma.h>
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <linux/genalloc.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include <linux/dma-map-ops.h>
#else
#include <linux/dma-contiguous.h>
#endif
#include <linux/err.h>
#include <linux/highmem.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/pageremap.h>
#include <linux/scatterlist.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>


//#include <uapi/linux/dma-heap.h>
#include <rtk_kdriver/rmm/rmm_heap.h>
#include <rtk_kdriver/quick_show/quick_show.h>

#include <rtd_log/rtd_module_log.h>

#ifdef CONFIG_VBM_CMA
#include <mach/rtk_platform.h>
#include <rtk_kdriver/rtk_vdec_svp.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>

extern struct page *alloc_specific_contiguous_memory(size_t size, unsigned long addr, int flags, const void *caller);
extern int free_contiguous_memory(struct page *page, int count);

static int __init proc_vbm_heap_info_init(void);

#define CONFIG_CHUNK_REUSE

struct vbm_heap_pool {
	struct gen_pool *pool;
	bool growup;
};

typedef struct vbm_chunk {
	unsigned long	chunk_size;
	unsigned long	chunk_addr;
	unsigned int	reserved;

	unsigned int	used;
	unsigned long	free_size;  // free size if this chunk is not fully used

#ifdef CONFIG_CHUNK_REUSE
	struct vbm_heap_list *list;
	struct vbm_heap_pool genpool;
#endif
} VBM_CHUNK_INFO;


VBM_CHUNK_INFO vbm_heap_chunk_list[CHUNK_ZONE_COUNT][MAX_CHUNKS_PER_ZONE];

struct page *vbm_chunk_res_pages[CHUNK_ZONE_COUNT][MAX_CHUNKS_PER_ZONE];


static DEFINE_MUTEX(chunk_list_mutex);

struct vbm_heap_list_priv_data {
	int alloc_zone_idx;
	int chunk_start_idx;
	int chunk_cnt;
	unsigned long protect_addr;
	unsigned long protect_size;
	int cnt_total;
	int cnt_remain;
	bool is4K;
	bool is_protected;
};

struct vbm_heap_list {
	struct list_head head;
	struct mutex lock;
	struct vbm_heap_list_priv_data data;
};

#endif

#define VBM_CHUNK_FLAGS_USED            0x1
#define VBM_CHUNK_FLAGS_RESERVED        0x2
#define VBM_CHUNK_FLAGS_BUDDY_ALLOCATED 0x4

struct vbm_chunk_array_info {
	int alloc_zone_index;
	int chunk_start_index;
	int chunk_end_index;
	unsigned long protect_address;
	unsigned long protect_size;
	bool need_protect;
	unsigned long new_size_delta;
	struct vbm_heap_list *reuse_list;
	struct vbm_heap_pool *reuse_genpool;
	unsigned long	alloc_addr_array[MAX_CHUNKS_PER_ZONE];   // as chunk addr for release
	unsigned long	alloc_size_array[MAX_CHUNKS_PER_ZONE];   // as chunk size for resease
	unsigned long	free_size_array[MAX_CHUNKS_PER_ZONE];
	unsigned long	growup_size_array[MAX_CHUNKS_PER_ZONE];
	unsigned int	flags_array[MAX_CHUNKS_PER_ZONE];
};

struct vbm_heap {
	struct dma_heap *heap;
#ifdef CONFIG_VBM_CMA
	phys_addr_t base;
	phys_addr_t size;
#endif
};

struct vbm_heap_buffer {
	struct vbm_heap *heap;
	struct list_head attachments;
	struct mutex lock;
	unsigned long len;
	struct page *vbm_pages;
	struct page **pages;
	pgoff_t pagecount;
	int vmap_cnt;
	void *vaddr;

	unsigned long phys_addr;
	unsigned long flags;
	bool uncached;

#ifdef CONFIG_VBM_CMA
	int alloc_zone_idx;
	int chunk_start_idx;
	int chunk_cnt;
	bool is_protected;
	bool is_occupied;
	unsigned long protect_addr;
	unsigned long protect_size;

	struct vbm_heap_list *list;
	struct list_head node;
	unsigned long resv1;
	unsigned long resv2;
#endif
};

struct vbm_heap_attachment {
	struct device *dev;
	struct sg_table table;
	struct list_head list;
	bool mapped;

	bool uncached;
};

#endif // BUILD_QUICK_SHOW


#if defined(CONFIG_VBM_CMA) || defined(BUILD_QUICK_SHOW)
// QuickShow only required heap chunk information sync'd

extern unsigned long DRAM_size;

unsigned long VBM_HEAP_4K_CHUNK_SIZE[CHUNK_ZONE_COUNT][MAX_CHUNKS_PER_ZONE] = {
	VBM_HEAP_4K_CHUNK_SIZE_ARRAY
};

unsigned char PRE_ALLOCATE_SIZE_INDEX_2D_BIT[CHUNK_ZONE_COUNT][MAX_CHUNKS_PER_ZONE] = {
        PRE_ALLOCATE_SIZE_INDEX_2D_BIT_ARRAY
};

unsigned long VBM_HEAP_4K_DIAS_CHUNK_SIZE[CHUNK_ZONE_COUNT][MAX_CHUNKS_PER_ZONE] = {
	VBM_HEAP_4K_DIAS_CHUNK_SIZE_ARRAY
};

unsigned long VBM_HEAP_4K_DIAS_MIRACAST_CHUNK_SIZE[CHUNK_ZONE_COUNT][MAX_CHUNKS_PER_ZONE] = {
	VBM_HEAP_4K_DIAS_MIRACAST_CHUNK_SIZE_ARRAY
};

unsigned long VBM_HEAP_2K_CHUNK_SIZE[CHUNK_ZONE_COUNT][MAX_CHUNKS_PER_ZONE] = {
	VBM_HEAP_2K_CHUNK_SIZE_ARRAY
};

unsigned long * vbm_heap_get_allocator(void)
{
	if (get_product_type() == PRODUCT_TYPE_DIAS) {
		if (DRAM_size >= 0x60000) // ((get_memory_size(GFP_DCU1) + get_memory_size(GFP_DCU2)) >= 0x60000000)
			return (unsigned long *)&VBM_HEAP_4K_DIAS_CHUNK_SIZE;
		else
			return (unsigned long *)&VBM_HEAP_4K_DIAS_MIRACAST_CHUNK_SIZE;
	} else {
		if (get_platform_model() == PLATFORM_MODEL_2K)
			return (unsigned long *)&VBM_HEAP_2K_CHUNK_SIZE;
		else
			return (unsigned long *)&VBM_HEAP_4K_CHUNK_SIZE;
	}
}

// BUILD_QUICK_SHOW
//  for QShow, it intends to use zone1 of vbm heap for hdmi source and needs to get fixed address of zone1 base.
//  for kernel with QShow enable (and required size defined in dts), it should be preserved at vbm_heap_create()
//    and store in pQShowVBM for later reclaim.

//#define HACK_QSHOW_VBM_FROM_ZONE0 // to compatible legacy usage, reserved from the beginning of zone0,
                                  // if not, always reserved from the beginning of small zone (zone1).

unsigned long get_vbm_heap_qshow_scaler_address(void)
{
	int i, j, zone_idx = 1; // always preserve zone1 for qshow_scaler.
	unsigned long vbm_size, vbm_addr;

	// get dts defined size for QShow usage
	vbm_size = carvedout_buf_query(CARVEDOUT_CMA_VBM, (void **)&vbm_addr);

	if (vbm_size) {
		// there is no vbm_heap_cma_init() for BUILD_QUICK_SHOW but zone1 base
		//   is derived from pre-defined vbm heap setting.
		unsigned long chunk_offset = 0;
		unsigned long (*allocator)[MAX_CHUNKS_PER_ZONE];
		allocator = (unsigned long (*)[MAX_CHUNKS_PER_ZONE])vbm_heap_get_allocator();
#ifdef HACK_QSHOW_VBM_FROM_ZONE0
		zone_idx = 0;
#else
		zone_idx = 1; // always preserve zone 1 for qshow_scaler
#endif
		for (i = 0; i < zone_idx; i++) {
			for (j = 0; j < MAX_CHUNKS_PER_ZONE; j++) {
				chunk_offset += allocator[i][j];
			}
		}

		rtd_pr_rmm_notice("%s calc qshow_scaler vbm zone %d 0x%lx\n", __func__, zone_idx, (vbm_addr + chunk_offset));
		return (vbm_addr + chunk_offset);
	}

	rtd_pr_rmm_alert("%s no CARVEDOUT_CMA_VBM\n", __func__);
	return 0;
}

#endif // CONFIG_VBM_CMA


#ifndef BUILD_QUICK_SHOW

#ifdef CONFIG_VBM_CMA
static void __pool_get_min_base(struct gen_pool *pool, struct gen_pool_chunk *chunk, void *data)
{
	unsigned long addr = INVALID_VAL;
	if (data)
		addr = *(unsigned long *)data;

	if (chunk->start_addr > 1)
		addr = min(chunk->start_addr, addr);

	if (data)
		*(unsigned long *)data = addr;
}

static inline unsigned long vbm_heap_pool_get_min_base(struct vbm_heap_pool *genpool)
{
	struct gen_pool *pool = NULL;
	unsigned long min_base = INVALID_VAL;

	if (!genpool) {
		rtd_pr_rmm_err("%s:%d no genpool\n", __func__, __LINE__);
		return min_base;
	} else {
		pool = genpool->pool;
	}

	if (pool) {
		gen_pool_for_each_chunk(pool, __pool_get_min_base, &min_base);
	} else {
		rtd_pr_rmm_err("%s:%d no pool\n", __func__, __LINE__);
	}

	return min_base;
}

#define ADDR_OVER_POOL_END  -2
#define ADDR_OVER_POOL_BASE -3
static inline int vbm_heap_pool_addr_in_pool(struct vbm_heap_pool *genpool, unsigned long addr, unsigned long size, 
											 unsigned long *over_base, unsigned long *over_size)
{
	struct gen_pool *pool = NULL;

	if (!genpool) {
		rtd_pr_rmm_err("%s:%d no genpool\n", __func__, __LINE__);
		return -1;
	} else {
		pool = genpool->pool;
	}

	if (!pool) {
		return -1;
	} else {
		unsigned long pool_base = vbm_heap_pool_get_min_base(genpool);
		unsigned long pool_size = gen_pool_size(pool);
		unsigned long pool_end = pool_base + pool_size;
		unsigned long end = addr + size;

		if ((pool_base != INVALID_VAL) && (addr >= pool_base && addr < pool_end) &&
			(end > pool_base && end <= pool_end)) {
			; // fall-through
		} else {
			rtd_pr_rmm_err("pool: addr(%lx/%lx/%lx) is not in pool(%lx/%lx/%lx) range\n", 
				   addr, size, end,
				   pool_base, pool_size, pool_end);

			if ((addr >= pool_base && addr < pool_end) && (end > pool_end)) {
				if (over_base)
					*over_base = pool_end;
				if (over_size)
					*over_size = end - pool_end;
				return ADDR_OVER_POOL_END;
			} else if ((addr < pool_base) && (end > pool_base && end <= pool_end)) {
				if (over_base)
					*over_base = addr;
				if (over_size)
					*over_size = pool_base - addr;
				return ADDR_OVER_POOL_BASE;
			} else {
				return -1;
			}
		}
	}

	return 0;
}

static inline int vbm_heap_pool_new_or_growup(struct vbm_heap_pool *genpool, unsigned long addr, unsigned long size)
{
	struct gen_pool *pool = NULL;
	int error = 0;

	if (!genpool) {
		rtd_pr_rmm_err("%s:%d no genpool\n", __func__, __LINE__);
		return -1;
	} else {
		pool = genpool->pool;
	}

	if (!pool) {
		pool = gen_pool_create(PAGE_SHIFT, -1);
		if (!pool) {
			rtd_pr_rmm_warn("reuse pool create fail\n");
			return -1;
		}

		error = gen_pool_add(pool, addr, size, -1);
		if (error < 0) {
			rtd_pr_rmm_warn("reuse pool add fail, addr/size(%lx/%lx)\n", addr, size);
			gen_pool_destroy(pool);
			return error;
		}

		genpool->pool = pool;
	} else {
		unsigned long min_base = vbm_heap_pool_get_min_base(genpool);
		size_t prev_size = gen_pool_size(pool);

		if (vbm_heap_pool_addr_in_pool(genpool, addr, size, NULL, NULL) < 0) {
			if (min_base != addr) {
				rtd_pr_rmm_err("reuse pool growup assert, mismatch base(%lx/%lx)\n", min_base, addr);
				BUG();
			}
			if (size != prev_size && size > prev_size) {
				unsigned long new_addr = addr + prev_size;
				error = __gen_pool_growup(pool, addr, size, -1);
				if (error < 0) {
					rtd_pr_rmm_warn("reuse pool growup fail, addr/size(%lx/%lx)\n", new_addr, size - prev_size);
					return error;
				}
				genpool->growup = true;
				rtd_pr_rmm_info("reuse pool growup success, addr/size(%lx/%lx)\n", new_addr, size - prev_size);
			}
		} else {
			rtd_pr_rmm_err("reuse pool growup no-need, new(%lx/%lx) range is inside the original(%lx/%lx) one\n",
				   addr, size, min_base, gen_pool_size(pool));
			//BUG();
		}
	}

	return 0;
}

static inline void vbm_heap_pool_delete(struct vbm_heap_pool *genpool)
{
	struct gen_pool *pool = NULL;

	if (!genpool) {
		rtd_pr_rmm_err("%s:%d no genpool\n", __func__, __LINE__);
		return;
	} else {
		pool = genpool->pool;
	}

	if (pool) {
		gen_pool_destroy(pool);
	} else {
		rtd_pr_rmm_err("%s:%d no pool\n", __func__, __LINE__);
	}
}

static inline unsigned long vbm_heap_pool_alloc(struct vbm_heap_pool *genpool, size_t size)
{
	struct gen_pool *pool = NULL;

	if (!genpool) {
		rtd_pr_rmm_err("%s:%d no genpool\n", __func__, __LINE__);
		return INVALID_VAL;
	} else {
		pool = genpool->pool;
	}

	if (pool) {
		unsigned long offset = INVALID_VAL;

		offset = gen_pool_alloc(pool, size);
		return offset ? offset : INVALID_VAL;
	} else {
		rtd_pr_rmm_err("%s:%d no pool\n", __func__, __LINE__);
		return INVALID_VAL;
	}
}

static void vbm_heap_pool_free(struct vbm_heap_pool *genpool, unsigned long addr, size_t size)
{
	struct gen_pool *pool = NULL;

	if (!genpool) {
		rtd_pr_rmm_err("%s:%d no genpool\n", __func__, __LINE__);
		return;
	} else {
		pool = genpool->pool;
	}

	if (pool) {
		gen_pool_free(pool, addr, size);
	} else {
		rtd_pr_rmm_err("%s:%d no pool\n", __func__, __LINE__);
	}
}

static size_t vbm_heap_pool_used(struct vbm_heap_pool *genpool)
{
	struct gen_pool *pool = NULL;
	size_t used = 0;
	size_t avail, size;

	if (!genpool) {
		rtd_pr_rmm_err("%s:%d no genpool\n", __func__, __LINE__);
		return used;
	} else {
		pool = genpool->pool;
	}

	if (pool) {
		avail = gen_pool_avail(pool);
		size = gen_pool_size(pool);
		used = size - avail;
	} else {
		rtd_pr_rmm_err("%s:%d no pool\n", __func__, __LINE__);
	}

	return used;
}

static inline size_t vbm_heap_pool_size(struct vbm_heap_pool *genpool)
{
	struct gen_pool *pool = NULL;

	if (!genpool) {
		rtd_pr_rmm_err("%s:%d no genpool\n", __func__, __LINE__);
		return 0;
	} else {
		pool = genpool->pool;
	}

	if (pool) {
		return gen_pool_size(pool);
	} else {
		rtd_pr_rmm_err("%s:%d no pool\n", __func__, __LINE__);
	}

	return 0;
}

static inline int vbm_heap_pool_empty(struct vbm_heap_pool *genpool)
{
	struct gen_pool *pool = NULL;
	int ret = 0; // not empty
	size_t avail, size;

	if (!genpool) {
		rtd_pr_rmm_err("%s:%d no genpool\n", __func__, __LINE__);
		return 1;
	} else {
		pool = genpool->pool;
	}

	if (pool) {
		avail = gen_pool_avail(pool);
		size = gen_pool_size(pool);
		if (avail == size) // empty
			ret = 1;
		else {
			rtd_pr_rmm_info("%s: pool not empty, avail(%lx)size(%lx)\n", __func__, (unsigned long)avail, (unsigned long)size);
			ret = 0;
		}
	} else {
		rtd_pr_rmm_err("%s:%d no pool\n", __func__, __LINE__);
		ret = 1;
	}

	return ret;
}

void vbm_heap_dump_chunk_list(void)
{
	int zone_idx, chunk_idx;

	mutex_lock(&chunk_list_mutex);
	for (zone_idx = 0; zone_idx < CHUNK_ZONE_COUNT; zone_idx++) {
		rtd_pr_rmm_err("[vbm_heap] zone %d:\n", zone_idx);
		for (chunk_idx = 0; chunk_idx < MAX_CHUNKS_PER_ZONE; chunk_idx++) {
			if (vbm_heap_chunk_list[zone_idx][chunk_idx].chunk_size > 0) {
				VBM_CHUNK_INFO *_chunk = NULL;
				struct vbm_heap_pool *reuse_genpool = NULL;
				struct gen_pool *pool = NULL;

				_chunk = &vbm_heap_chunk_list[zone_idx][chunk_idx];
				reuse_genpool = &_chunk->genpool;

				rtd_pr_rmm_err("    chunk %d: size = %ld MB, phys_addr = 0x%lx, reserved = %d, used = %d, free = %ld MB\n",
					chunk_idx,
					(vbm_heap_chunk_list[zone_idx][chunk_idx].chunk_size)/__MB__,
					vbm_heap_chunk_list[zone_idx][chunk_idx].chunk_addr,
					vbm_heap_chunk_list[zone_idx][chunk_idx].reserved,
					vbm_heap_chunk_list[zone_idx][chunk_idx].used,
					(vbm_heap_chunk_list[zone_idx][chunk_idx].free_size)/__MB__);

				if (pool) {
					rtd_pr_rmm_info("    pool(%lx), size(%lx), used(%lx)\n", (unsigned long)reuse_genpool->pool,
							(unsigned long)vbm_heap_pool_size(reuse_genpool) ,(unsigned long)vbm_heap_pool_used(reuse_genpool));
				}
			}
		}
	}
	mutex_unlock(&chunk_list_mutex);
}
EXPORT_SYMBOL(vbm_heap_dump_chunk_list);

static void vbm_heap_list_reset_buffer_data(struct vbm_heap_list *list, 
					struct vbm_heap_list_priv_data *data)
{
	struct vbm_heap_buffer *buffer;
	int i = 0;

	if (list) {
		mutex_lock(&list->lock);
		list_for_each_entry(buffer, &list->head, node) {
			/* Must consider the consecutive chunks for 
			 * last and current source are different.
			 * Make sure the current consecutive chunks is 
			 * the largest.
			 */ 
			if (data->chunk_start_idx+data->chunk_cnt-1 > buffer->chunk_start_idx+buffer->chunk_cnt-1) {
				rtd_pr_rmm_info("buffer[%d](0x%lx) [*]curr:zone[%d]chunk[%d-%d], last:zone[%d]chunk[%d-%d]\n", i, buffer->phys_addr, 
						data->alloc_zone_idx, data->chunk_start_idx, data->chunk_start_idx+data->chunk_cnt-1,
						buffer->alloc_zone_idx, buffer->chunk_start_idx, buffer->chunk_start_idx+buffer->chunk_cnt-1);

				buffer->alloc_zone_idx = data->alloc_zone_idx;
				buffer->chunk_start_idx = data->chunk_start_idx;
				buffer->chunk_cnt = data->chunk_cnt;
			} else if (data->chunk_start_idx+data->chunk_cnt-1 < buffer->chunk_start_idx+buffer->chunk_cnt-1) {
				rtd_pr_rmm_info("buffer[%d](0x%lx) curr:zone[%d]chunk[%d-%d], [*]last:zone[%d]chunk[%d-%d]\n", i, buffer->phys_addr, 
						data->alloc_zone_idx, data->chunk_start_idx, data->chunk_start_idx+data->chunk_cnt-1,
						buffer->alloc_zone_idx, buffer->chunk_start_idx, buffer->chunk_start_idx+buffer->chunk_cnt-1);

				data->alloc_zone_idx = buffer->alloc_zone_idx;
				data->chunk_start_idx = buffer->chunk_start_idx;
				data->chunk_cnt = buffer->chunk_cnt;
			}

			if (data->is_protected && buffer->is_protected) {
				if (data->protect_size > buffer->protect_size) {
					rtd_pr_rmm_info("buffer[%d](0x%lx) protect [*]curr:%d,addr=%lx,size=%lx, last:%d,addr=%lx,size=%lx\n", i, buffer->phys_addr, 
							data->is_protected, data->protect_addr, data->protect_size,
							buffer->is_protected, buffer->protect_addr, buffer->protect_size);
					buffer->protect_size = data->protect_size;
				} else if (data->protect_size < buffer->protect_size) {
					rtd_pr_rmm_info("buffer[%d](0x%lx) protect curr:%d,addr=%lx,size=%lx, [*]last:%d,addr=%lx,size=%lx\n", i, buffer->phys_addr, 
							data->is_protected, data->protect_addr, data->protect_size,
							buffer->is_protected, buffer->protect_addr, buffer->protect_size);
					data->protect_size = buffer->protect_size;
				}
			} else if (data->is_protected && !buffer->is_protected) {
				rtd_pr_rmm_info("buffer[%d](0x%lx) protect [*]curr:%d,addr=%lx,size=%lx, last:%d,addr=%lx,size=%lx\n", i, buffer->phys_addr, 
						data->is_protected, data->protect_addr, data->protect_size,
						buffer->is_protected, buffer->protect_addr, buffer->protect_size);
				buffer->is_protected = data->is_protected;
				buffer->protect_addr = data->protect_addr;
				buffer->protect_size = data->protect_size;
			} else if (!data->is_protected && buffer->is_protected) {
				rtd_pr_rmm_info("buffer[%d](0x%lx) reset protect [*]curr:%d,addr=%lx,size=%lx, last:%d,addr=%lx,size=%lx\n", i, buffer->phys_addr, 
						data->is_protected, data->protect_addr, data->protect_size,
						buffer->is_protected, buffer->protect_addr, buffer->protect_size);
				if (buffer->is_protected) {
					if (rtkvdec_svp_disable_protection(buffer->protect_addr, buffer->protect_size, TYPE_SVP_PROTECT_VBM) != 0) {
						rtd_pr_rmm_err("Error! Disable protection failed for addr 0x%lx, size 0x%lx",
							   buffer->protect_addr, buffer->protect_size);
						vbm_heap_dump_chunk_list();
						BUG();
					}
				}
				buffer->is_protected = data->is_protected;
				buffer->protect_addr = data->protect_addr;
				buffer->protect_size = data->protect_size;
			}

			buffer->is_occupied = true;

			i++;
		}
		mutex_unlock(&list->lock);
	} else {
		rtd_pr_rmm_err("%s: no list\n", __func__);
	}
}

static inline void vbm_heap_list_set_priv_data(struct vbm_heap_list *list, 
					struct vbm_heap_list_priv_data *data)
{
	if (list) {
		mutex_lock(&list->lock);
		if (data)
			memcpy(&list->data, data, sizeof(struct vbm_heap_list_priv_data));
		mutex_unlock(&list->lock);
	}
	return;
}

/* return: first node of list */
static inline struct vbm_heap_buffer *vbm_heap_list_get_priv_data(struct vbm_heap_list *list, 
											struct vbm_heap_list_priv_data *data)
{
	if (list) {
		struct vbm_heap_buffer *buffer = NULL;

		mutex_lock(&list->lock);
		buffer = list_first_entry_or_null(&list->head, struct vbm_heap_buffer, node);
		if (data)
			memcpy(data, &list->data, sizeof(struct vbm_heap_list_priv_data));
		mutex_unlock(&list->lock);

		return buffer;
	} else 
		return NULL;
}

static inline int vbm_heap_list_empty(struct vbm_heap_buffer *buffer)
{
	int empty = 0;

	if (buffer->list) {
		mutex_lock(&buffer->list->lock);
		empty = list_empty(&buffer->list->head);
		mutex_unlock(&buffer->list->lock);
		return empty;
	}

	return -ENOMEM;
}

static inline void vbm_heap_list_init(struct vbm_heap_buffer *buffer)
{
	if (buffer->list != NULL) {
		rtd_pr_rmm_err("%s: error: list(%lx) already allocated, belong to chunk (%d/%d)\n", __func__,
			   (unsigned long)buffer->list, buffer->chunk_start_idx, buffer->chunk_cnt);
		BUG();
	}

	buffer->list = kmalloc(sizeof(struct vbm_heap_list), GFP_KERNEL);
	memset(buffer->list, 0, sizeof(struct vbm_heap_list));
	if (buffer->list) {
		mutex_init(&buffer->list->lock);
		INIT_LIST_HEAD(&buffer->list->head);
	}
}

static inline void vbm_heap_list_deinit(struct vbm_heap_buffer *buffer)
{
	if (buffer->list == NULL) {
		rtd_pr_rmm_err("%s: error: frame already freed? list(%lx), belong to chunk (%d/%d)\n", __func__,
			   (unsigned long)buffer->list, buffer->chunk_start_idx, buffer->chunk_cnt);
//		BUG();
	}

	if (buffer->list) {
		mutex_destroy(&buffer->list->lock);
		INIT_LIST_HEAD(&buffer->list->head);  // re-init, do we need to del head node?
		kfree(buffer->list);
		buffer->list = NULL;
	}
}

static inline void vbm_heap_list_add(struct vbm_heap_buffer *buffer)
{
	if (buffer->list == NULL) {
		rtd_pr_rmm_err("%s: error: not the first frame? list(%lx), belong to chunk (%d/%d)\n", __func__,
			   (unsigned long)buffer->list, buffer->chunk_start_idx, buffer->chunk_cnt);
//		BUG();
	}

	if (buffer->list) {
		mutex_lock(&buffer->list->lock);
		list_add_tail(&buffer->node, &buffer->list->head);
		mutex_unlock(&buffer->list->lock);
	}
}

static inline void vbm_heap_list_del(struct vbm_heap_buffer *buffer)
{
	if (buffer->list) {
		mutex_lock(&buffer->list->lock);

		(&buffer->list->data)->cnt_remain--;
		if ((&buffer->list->data)->cnt_remain < 0)
			(&buffer->list->data)->cnt_remain = 0;

		list_del(&buffer->node);
		mutex_unlock(&buffer->list->lock);
	}
}

void vbm_heap_list_dump(struct vbm_heap_buffer *buffer)
{
	struct vbm_heap_buffer *buf_obj;

	if (buffer->list) {
		mutex_lock(&buffer->list->lock);
		list_for_each_entry(buf_obj, &buffer->list->head, node) {
			rtd_pr_rmm_debug("%s: dmabuf hndl(%lx) chunk(%d/%d) phys_addr = %lx, size=%lx(pagecount=%lx)\n", __func__,
				buf_obj->resv1, buf_obj->chunk_start_idx, buf_obj->chunk_cnt,
				buf_obj->phys_addr, buf_obj->len, (unsigned long)buf_obj->pagecount);
		}
		mutex_unlock(&buffer->list->lock);
	} else {
		rtd_pr_rmm_err("%s: no list\n", __func__);
	}
}

int vbm_heap_to_dmabuf_list(struct dma_buf *dmabuf, unsigned long buf_array[], int array_num)
{
	struct vbm_heap_buffer *buf_obj;
	struct vbm_heap_buffer *buffer = dmabuf->priv;
	int i = 0;
	int ret = 0;

	if (buffer->list) {
		mutex_lock(&buffer->list->lock);
		list_for_each_entry(buf_obj, &buffer->list->head, node) {
			if (buf_obj->is_occupied == true)
				continue;
			if (i >= array_num) {
				ret = -EINVAL;
				break;
			}
			buf_array[i] = (unsigned long)buf_obj->resv1;
			rtd_pr_rmm_debug("dmabuf[%d] hndl(%lx)\n", i, buf_array[i]);
			i++;
		}
		mutex_unlock(&buffer->list->lock);
	} else {
		ret = -ENOMEM;
	}

	if (ret < 0)
		return ret;
	else
		return i;
}

#endif

static int vbm_heap_attach(struct dma_buf *dmabuf,
			   struct dma_buf_attachment *attachment)
{
	struct vbm_heap_buffer *buffer = dmabuf->priv;
	struct vbm_heap_attachment *a;
	int ret;

	a = kzalloc(sizeof(*a), GFP_KERNEL);
	if (!a)
		return -ENOMEM;

	ret = sg_alloc_table_from_pages(&a->table, buffer->pages,
					buffer->pagecount, 0,
					buffer->pagecount << PAGE_SHIFT,
					GFP_KERNEL);
	if (ret) {
		kfree(a);
		return ret;
	}

	a->dev = attachment->dev;
	INIT_LIST_HEAD(&a->list);
	a->mapped = false;
	a->uncached = buffer->uncached;

	attachment->priv = a;

	mutex_lock(&buffer->lock);
	list_add(&a->list, &buffer->attachments);
	mutex_unlock(&buffer->lock);

	return 0;
}

static void vbm_heap_detach(struct dma_buf *dmabuf,
			    struct dma_buf_attachment *attachment)
{
	struct vbm_heap_buffer *buffer = dmabuf->priv;
	struct vbm_heap_attachment *a = attachment->priv;

	mutex_lock(&buffer->lock);
	list_del(&a->list);
	mutex_unlock(&buffer->lock);

	sg_free_table(&a->table);
	kfree(a);
}

static struct sg_table *vbm_heap_map_dma_buf(struct dma_buf_attachment *attachment,
					     enum dma_data_direction direction)
{
	struct vbm_heap_attachment *a = attachment->priv;
	struct sg_table *table = &a->table;
	int attr = 0;
	int ret;

	if (a->uncached)
		attr = DMA_ATTR_SKIP_CPU_SYNC;

	ret = dma_map_sgtable(attachment->dev, table, direction, attr);
	if (ret)
		return ERR_PTR(-ENOMEM);

	a->mapped = true;
	return table;
}

static void vbm_heap_unmap_dma_buf(struct dma_buf_attachment *attachment,
				   struct sg_table *table,
				   enum dma_data_direction direction)
{
	struct vbm_heap_attachment *a = attachment->priv;
	int attr = 0;

	if (a->uncached)
		attr = DMA_ATTR_SKIP_CPU_SYNC;

	a->mapped = false;
	dma_unmap_sgtable(attachment->dev, table, direction, attr);
}

static int vbm_heap_dma_buf_begin_cpu_access(struct dma_buf *dmabuf,
					     enum dma_data_direction direction)
{
	struct vbm_heap_buffer *buffer = dmabuf->priv;
	struct vbm_heap_attachment *a;

	mutex_lock(&buffer->lock);

	if (buffer->vmap_cnt)
		invalidate_kernel_vmap_range(buffer->vaddr, buffer->len);

	if (!buffer->uncached) {
		list_for_each_entry(a, &buffer->attachments, list) {
			if (!a->mapped)
				continue;
			dma_sync_sgtable_for_cpu(a->dev, &a->table, direction);
		}
	}
	mutex_unlock(&buffer->lock);

	return 0;
}

static int vbm_heap_dma_buf_end_cpu_access(struct dma_buf *dmabuf,
					   enum dma_data_direction direction)
{
	struct vbm_heap_buffer *buffer = dmabuf->priv;
	struct vbm_heap_attachment *a;

	mutex_lock(&buffer->lock);

	if (buffer->vmap_cnt)
		flush_kernel_vmap_range(buffer->vaddr, buffer->len);

	if (!buffer->uncached) {
		list_for_each_entry(a, &buffer->attachments, list) {
			if (!a->mapped)
				continue;
			dma_sync_sgtable_for_device(a->dev, &a->table, direction);
		}
	}
	mutex_unlock(&buffer->lock);

	return 0;
}

static int vbm_heap_mmap(struct dma_buf *dmabuf, struct vm_area_struct *vma)
{
	struct vbm_heap_buffer *buffer = dmabuf->priv;
	unsigned long user_count = (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;
	unsigned long count = PAGE_ALIGN(buffer->len) >> PAGE_SHIFT;
	unsigned long pfn = buffer->phys_addr >> PAGE_SHIFT;
	unsigned long off = vma->vm_pgoff;
	int ret = -ENXIO;

	if ((vma->vm_flags & (VM_SHARED | VM_MAYSHARE)) == 0)
		return -EINVAL;

	if (buffer->uncached) {
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
	}

	if (off < count && user_count <= (count - off)) {
		rtd_pr_rmm_debug("Map %lx to %lx size %lx uncached(%d)\n", 
			(pfn + off) << PAGE_SHIFT, vma->vm_start, user_count << PAGE_SHIFT, buffer->uncached);
	        ret = remap_pfn_range(vma, vma->vm_start,
	                              pfn + off,
	                              user_count << PAGE_SHIFT,
	                              vma->vm_page_prot);
	}

	return ret;
}

static void *vbm_heap_do_vmap(struct vbm_heap_buffer *buffer)
{
	pgprot_t pgprot = PAGE_KERNEL;
	void *vaddr;

	if (buffer->uncached)
		pgprot = pgprot_writecombine(PAGE_KERNEL);

	vaddr = vmap(buffer->pages, buffer->pagecount, VM_MAP, pgprot);
	if (!vaddr)
		return ERR_PTR(-ENOMEM);

	return vaddr;
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0))

static int vbm_heap_vmap(struct dma_buf *dmabuf, struct dma_buf_map *map)
{
	struct vbm_heap_buffer *buffer = dmabuf->priv;
	void *vaddr;
	int ret = 0;

	mutex_lock(&buffer->lock);
	if (buffer->vmap_cnt) {
		buffer->vmap_cnt++;
		dma_buf_map_set_vaddr(map, buffer->vaddr);
		goto out;
	}

	vaddr = vbm_heap_do_vmap(buffer);
	if (IS_ERR(vaddr))
		goto out;

	buffer->vaddr = vaddr;
	buffer->vmap_cnt++;
	dma_buf_map_set_vaddr(map, buffer->vaddr);
out:
	mutex_unlock(&buffer->lock);

	return ret;
}

static void vbm_heap_vunmap(struct dma_buf *dmabuf, struct dma_buf_map *map)
{
	struct vbm_heap_buffer *buffer = dmabuf->priv;

	mutex_lock(&buffer->lock);
	if (!--buffer->vmap_cnt) {
		vunmap(buffer->vaddr);
		buffer->vaddr = NULL;
	}
	mutex_unlock(&buffer->lock);
	dma_buf_map_clear(map);
}

#else

static void *vbm_heap_vmap(struct dma_buf *dmabuf)
{
	struct vbm_heap_buffer *buffer = dmabuf->priv;
	void *vaddr;

	mutex_lock(&buffer->lock);
	if (buffer->vmap_cnt) {
		buffer->vmap_cnt++;
		vaddr = buffer->vaddr;
		goto out;
	}

	vaddr = vbm_heap_do_vmap(buffer);
	if (IS_ERR(vaddr))
		goto out;

	buffer->vaddr = vaddr;
	buffer->vmap_cnt++;
out:
	mutex_unlock(&buffer->lock);

	return vaddr;
}

static void vbm_heap_vunmap(struct dma_buf *dmabuf, void *vaddr)
{
	struct vbm_heap_buffer *buffer = dmabuf->priv;

	mutex_lock(&buffer->lock);
	if (!--buffer->vmap_cnt) {
		vunmap(buffer->vaddr);
		buffer->vaddr = NULL;
	}
	mutex_unlock(&buffer->lock);
}

#endif

static void vbm_heap_dma_buf_release(struct dma_buf *dmabuf)
{
	struct vbm_heap_buffer *buffer = dmabuf->priv;
#ifdef CONFIG_VBM_CMA
	int zone_idx;
	int chunk_idx;
	int ret = 0;
#endif
#ifdef CONFIG_CHUNK_REUSE
	VBM_CHUNK_INFO *_chunk = NULL;
	struct vbm_heap_list *reuse_list = NULL;
	struct vbm_heap_pool *reuse_genpool = NULL;
#endif

	if (!buffer) {
		rtd_pr_rmm_err("%s: ERROR!! No buffer in dmabuf %lx\n", __func__, (unsigned long)buffer);
		return;
	}

	rtd_pr_rmm_info("Release buffer %lx, phy(0x%lx)\n", (unsigned long)buffer, (unsigned long)buffer->phys_addr);

	if (buffer->vmap_cnt > 0) {
		WARN(1, "%s: buffer still mapped in the kernel\n", __func__);
		vunmap(buffer->vaddr);
	}

	/* free page list */
	vfree(buffer->pages);
	/* release memory */
#ifdef CONFIG_VBM_CMA
	_chunk = &vbm_heap_chunk_list[buffer->alloc_zone_idx][buffer->chunk_start_idx];
	reuse_list = _chunk->list;
	reuse_genpool = &_chunk->genpool;
	vbm_heap_pool_free(reuse_genpool, buffer->phys_addr, buffer->len);

	vbm_heap_list_del(buffer);
	ret = vbm_heap_list_empty(buffer);
	if (ret > 0) {
		VBM_CHUNK_INFO chunk_info[MAX_CHUNKS_PER_ZONE] = {0};
		unsigned long remain_pool_size = vbm_heap_pool_size(reuse_genpool);

		if (reuse_list && ((&reuse_list->data)->chunk_cnt != buffer->chunk_cnt)) {
			rtd_pr_rmm_info("reuse_list chunk_cnt(%d/%d)\n", (&reuse_list->data)->chunk_cnt, buffer->chunk_cnt);
		}

		rtd_pr_rmm_info("%s: Free phys_addr from VBM heap 0x%lx (zone[%d]chunk[%d-%d])\n", __func__,
			buffer->phys_addr, buffer->alloc_zone_idx, 
				buffer->chunk_start_idx, (buffer->chunk_start_idx + buffer->chunk_cnt - 1));
		rtd_pr_rmm_info("	 (protected = %d, protect_addr = 0x%lx, protect_size = 0x%lx)\n",
			buffer->is_protected, buffer->protect_addr, buffer->protect_size);

		zone_idx = buffer->alloc_zone_idx;

		mutex_lock(&chunk_list_mutex);
		for (chunk_idx = buffer->chunk_start_idx; chunk_idx < (buffer->chunk_start_idx + buffer->chunk_cnt); chunk_idx++) {
			chunk_info[chunk_idx].used = vbm_heap_chunk_list[zone_idx][chunk_idx].used;
			chunk_info[chunk_idx].chunk_addr = vbm_heap_chunk_list[zone_idx][chunk_idx].chunk_addr;
			chunk_info[chunk_idx].chunk_size = vbm_heap_chunk_list[zone_idx][chunk_idx].chunk_size;
			chunk_info[chunk_idx].free_size = vbm_heap_chunk_list[zone_idx][chunk_idx].free_size;
			chunk_info[chunk_idx].reserved = vbm_heap_chunk_list[zone_idx][chunk_idx].reserved;
		}
		mutex_unlock(&chunk_list_mutex);

		if (buffer->is_protected) {
			if (rtkvdec_svp_disable_protection(buffer->protect_addr, buffer->protect_size, TYPE_SVP_PROTECT_VBM) != 0) {
				rtd_pr_rmm_err("Error! Disable protection failed for addr 0x%lx, size 0x%lx",
					buffer->protect_addr, buffer->protect_size);
				vbm_heap_dump_chunk_list();
				BUG();
			}
		}

		for (chunk_idx = buffer->chunk_start_idx; chunk_idx < (buffer->chunk_start_idx + buffer->chunk_cnt); chunk_idx++)
		{
			if (chunk_info[chunk_idx].used) {
				struct page *vbm_chunk_pages;
				int vbm_chunk_count;

				vbm_chunk_pages = pfn_to_page(chunk_info[chunk_idx].chunk_addr >> PAGE_SHIFT);
				vbm_chunk_count = (chunk_info[chunk_idx].chunk_size - chunk_info[chunk_idx].free_size) >> PAGE_SHIFT;
				if (chunk_info[chunk_idx].free_size != 0) {
					if (remain_pool_size >= chunk_info[chunk_idx].chunk_size)
						vbm_chunk_count = chunk_info[chunk_idx].chunk_size >> PAGE_SHIFT;
					else if (remain_pool_size > (vbm_chunk_count << PAGE_SHIFT))
						vbm_chunk_count = remain_pool_size >> PAGE_SHIFT;
				}

				if (chunk_info[chunk_idx].reserved) {
					rtd_pr_rmm_info("free reserved zone[%d] chunk[%d], size(%x) BYPASS!!\n", zone_idx, chunk_idx, vbm_chunk_count << PAGE_SHIFT);
				} else {
					if (free_contiguous_memory(vbm_chunk_pages, vbm_chunk_count)) {
						rtd_pr_rmm_err("free zone[%d] chunk[%d] failed\n", zone_idx, chunk_idx);
					} else {
						rtd_pr_rmm_info("free zone[%d] chunk[%d] page(%lx), pfn(%lx), size(%x) successfully\n", zone_idx, chunk_idx, 
								vbm_chunk_pages, page_to_pfn(vbm_chunk_pages), vbm_chunk_count << PAGE_SHIFT);
					}
				}

				if (remain_pool_size)
					remain_pool_size -= (vbm_chunk_count << PAGE_SHIFT);

			} else {
				rtd_pr_rmm_err("%s: ERROR!! Release unused zone[%d] chunk[%d], addr 0x%lx, size 0x%lx\n", __func__,
					zone_idx, chunk_idx, chunk_info[chunk_idx].chunk_addr, chunk_info[chunk_idx].chunk_size);
				vbm_heap_dump_chunk_list();
				BUG();
			}
		}
		vbm_heap_pool_delete(reuse_genpool);

		mutex_lock(&chunk_list_mutex);
		for (chunk_idx = buffer->chunk_start_idx; chunk_idx < (buffer->chunk_start_idx + buffer->chunk_cnt); chunk_idx++)
		{
			vbm_heap_chunk_list[zone_idx][chunk_idx].used = 0;
			vbm_heap_chunk_list[zone_idx][chunk_idx].free_size = vbm_heap_chunk_list[zone_idx][chunk_idx].chunk_size;
#ifdef CONFIG_CHUNK_REUSE
			vbm_heap_chunk_list[zone_idx][chunk_idx].list = NULL;
			(&vbm_heap_chunk_list[zone_idx][chunk_idx].genpool)->pool = NULL;
			(&vbm_heap_chunk_list[zone_idx][chunk_idx].genpool)->growup = false;
#endif
		}
		mutex_unlock(&chunk_list_mutex);

		vbm_heap_list_deinit(buffer);
	} else if (ret == 0) {
		buffer->list = NULL;
	} else {
		rtd_pr_rmm_err("%s: no buffer list, ret=%d\n", __func__, ret);
	}
#else
	dvr_free_page_mesg(buffer->vbm_pages);
#endif
	kfree(buffer);
}

static const struct dma_buf_ops vbm_heap_buf_ops = {
	.attach = vbm_heap_attach,
	.detach = vbm_heap_detach,
	.map_dma_buf = vbm_heap_map_dma_buf,
	.unmap_dma_buf = vbm_heap_unmap_dma_buf,
	.begin_cpu_access = vbm_heap_dma_buf_begin_cpu_access,
	.end_cpu_access = vbm_heap_dma_buf_end_cpu_access,
	.mmap = vbm_heap_mmap,
	.vmap = vbm_heap_vmap,
	.vunmap = vbm_heap_vunmap,
	.release = vbm_heap_dma_buf_release,
};

unsigned long vbm_heap_to_phys(struct dma_buf *dmabuf)
{
	struct vbm_heap_buffer *buffer;
	unsigned long ret = INVALID_VAL;

	if (IS_ERR(dmabuf)) {
		rtd_pr_rmm_err("%s invalid dmabuf(%lx)\n", __func__, (unsigned long)dmabuf);
		return INVALID_VAL;
	}

	if ((buffer = dmabuf->priv) == NULL) {
		rtd_pr_rmm_err("%s() no buffer, dmabuf(%lx)\n", __func__, (unsigned long)dmabuf);
		goto err_out;
	}

	ret = (unsigned long)buffer->phys_addr;

	if (ret == 0)
		return INVALID_VAL;
	else
		return ret;

err_out:
	return INVALID_VAL;
}

static int vbm_heap_clear_chunk(struct page *chunk_page, size_t size)
{
	//FIXME! needed?
#if 0
	pgoff_t pagecount = size >> PAGE_SHIFT;

	if (PageHighMem(chunk_page)) {
		unsigned long nr_clear_pages = pagecount;
		struct page *page = chunk_page;

		while (nr_clear_pages > 0) {
			void *vaddr = kmap_atomic(page);

			memset(vaddr, 0, PAGE_SIZE);
			kunmap_atomic(vaddr);
			/*
			 * Avoid wasting time zeroing memory if the process
			 * has been killed by by SIGKILL
			 */
			if (fatal_signal_pending(current))
				return -ESRCH;
			page++;
			nr_clear_pages--;
		}
	} else {
		memset(page_address(chunk_page), 0, size);
	}
#endif

	return 0;
}

static struct dma_buf *vbm_heap_chunk_allocate_frame_single(struct dma_heap *heap,
					 struct page *vbm_pages, unsigned long size,
					 unsigned long fd_flags,
					 unsigned long heap_flags)
{
	struct vbm_heap *vbm_heap = dma_heap_get_drvdata(heap);
	struct vbm_heap_buffer *buffer;
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	pgoff_t pagecount = size >> PAGE_SHIFT;
	struct dma_buf *dmabuf;
	int ret = -ENOMEM;
	pgoff_t pg;

	BUG_ON(!PAGE_ALIGNED(size));

	rtd_pr_rmm_debug("%s: Request frame size 0x%lx\n", __func__, size);

	buffer = kzalloc(sizeof(*buffer), GFP_KERNEL);
	if (!buffer) {
		rtd_pr_rmm_err("Can't allocate buffer info\n");
		return ERR_PTR(-ENOMEM);
	}

	INIT_LIST_HEAD(&buffer->attachments);
	mutex_init(&buffer->lock);
	buffer->len = size;
	buffer->flags = heap_flags;
	buffer->uncached = (buffer->flags & DMA_HEAP_FLAG_CACHED) ? false : true;

	/* Clear the chunk allocated page */
	//vbm_heap_clear_chunk(vbm_pages, size);

	buffer->pages = vmalloc(pagecount * sizeof(*buffer->pages));
	if (!buffer->pages) {
		ret = -ENOMEM;
		rtd_pr_rmm_err("Fail to allocate page array\n");
		goto free_buffer;
	}

	for (pg = 0; pg < pagecount; pg++)
		buffer->pages[pg] = &vbm_pages[pg];

	buffer->vbm_pages = vbm_pages;
	buffer->heap = vbm_heap;
	buffer->pagecount = pagecount;
	buffer->phys_addr = page_to_phys(vbm_pages);

	/* create the dmabuf */
	exp_info.ops = &vbm_heap_buf_ops;
	exp_info.size = buffer->len;
	exp_info.flags = fd_flags;
	exp_info.priv = buffer;
	dmabuf = dma_buf_export(&exp_info);
	if (IS_ERR(dmabuf)) {
		ret = PTR_ERR(dmabuf);
		goto free_pages;
	}

	rtd_pr_rmm_info("%s: alloc 0x%lx %lx uncached(%d)\n", __func__, buffer->phys_addr, buffer->len, buffer->uncached);
	return dmabuf;

free_pages:
	vfree(buffer->pages);
free_buffer:
	kfree(buffer);

	return ERR_PTR(ret);
}

#define SEEK_CHUNKS_STATE_OK					0
#define SEEK_CHUNKS_STATE_ERROR				1
#define SEEK_CHUNKS_STATE_REUSE_CHUNK_WAIT		2

/* Return state */
static int vbm_heap_seek_chunks(struct vbm_chunk_array_info *info, struct vbm_heap_pool *reuse_genpool, 
									 int zone_start_idx, size_t size, unsigned int frame_count, unsigned long flags)
{
	int z_idx, c_idx;
	int prev_source_chunk_start_index = -1;
	int prev_source_chunk_end_index = -1;
	bool find_chunks = false;
	unsigned long chunk_available_size = 0;
	size_t new_size = size;
	int ret = SEEK_CHUNKS_STATE_OK;
	int chunk_reseek_start_idx = 0;

	mutex_lock(&chunk_list_mutex);
	for (z_idx = zone_start_idx; z_idx < zone_start_idx + CHUNK_ZONE_COUNT; z_idx++)
 	{
		/* this flag should not be reset while it need to be re-seek */
		chunk_reseek_start_idx = 0;

chunk_reseek:
		info->alloc_zone_index = z_idx % CHUNK_ZONE_COUNT;
		info->chunk_start_index = chunk_reseek_start_idx;
		chunk_available_size = 0;

		prev_source_chunk_start_index = -1;
		prev_source_chunk_end_index = -1;
		new_size = size;
		info->reuse_list = NULL;
		reuse_genpool->pool = NULL;

		for (c_idx = 0; c_idx < MAX_CHUNKS_PER_ZONE; c_idx++)
		{
			info->alloc_addr_array[c_idx] = 0;
			info->alloc_size_array[c_idx] = 0;
			info->growup_size_array[c_idx] = 0;
		}

		for (c_idx = chunk_reseek_start_idx; c_idx < MAX_CHUNKS_PER_ZONE; c_idx++)
		{
			if (vbm_heap_chunk_list[info->alloc_zone_index][c_idx].used) {
#ifdef CONFIG_CHUNK_REUSE
				VBM_CHUNK_INFO *_chunk = NULL;
				struct vbm_heap_list_priv_data data[1] = {0};
				struct vbm_heap_buffer *buffer;

				_chunk = &vbm_heap_chunk_list[info->alloc_zone_index][c_idx];
				buffer = vbm_heap_list_get_priv_data(_chunk->list, data);

				if (buffer && _chunk->list) {
					/* Condition for Reuse */
					if (data->cnt_remain) {
						if ((info->alloc_zone_index == buffer->alloc_zone_idx) && 
							(data->is4K == (flags & DMA_HEAP_FLAG_USER_VIDEO_4K_LUMA || 
										   flags & DMA_HEAP_FLAG_USER_VIDEO_4K_CHROMA)) &&
							!(flags & DMA_HEAP_FLAG_USER_COBUFFER) &&
							(data->cnt_total != data->cnt_remain) ) 
						{
							unsigned int frame_size = size / frame_count;

							/* fall-through */
							if ((&_chunk->genpool)->pool == NULL) {
								rtd_pr_rmm_info("reuse: zone[%d]chunk[%d] wait pool handler\n", info->alloc_zone_index, c_idx);
								ret = SEEK_CHUNKS_STATE_REUSE_CHUNK_WAIT;
								goto seek_out;
							}
							info->reuse_list = _chunk->list;
							reuse_genpool->pool = (&_chunk->genpool)->pool;
							prev_source_chunk_start_index = buffer->chunk_start_idx;
							prev_source_chunk_end_index = buffer->chunk_start_idx + buffer->chunk_cnt - 1;

							// [precondition] new frames alloaction skips keep_frames and allocated right after keep_frames's end.
							// keep_frames could be more than 2 and discontiguous,
							// for conservative estimation, always preserve 1 (keep+new) size for every 1 keep frame.
							new_size = size + (data->cnt_remain * ((buffer->pagecount << PAGE_SHIFT) + frame_size));
							rtd_pr_rmm_info("reuse: zone[%d]chunk[%d] new_size 0x%lx -> 0x%lx is4K(%d), rmn(%d)(0x%lx,0x%lx)) \n", info->alloc_zone_index, c_idx, size, new_size, data->is4K, data->cnt_remain, (buffer->pagecount << PAGE_SHIFT), (unsigned long)frame_size);

						} else {
							rtd_pr_rmm_info("reuse: zone(%d/%d) or list_cnt(%d/%d) not match, is4K(%d), heap_flags(%lx), chunk(curr[%d]) seek next\n", 
									info->alloc_zone_index, buffer->alloc_zone_idx, 
									data->cnt_total, data->cnt_remain,
									data->is4K, flags, c_idx);

							chunk_available_size = 0;
							info->chunk_start_index = c_idx + 1;

							/* reset for not consective chunks */
							prev_source_chunk_start_index = -1;
							prev_source_chunk_end_index = -1;
							new_size = size;
							info->reuse_list = NULL;
							reuse_genpool->pool = NULL;

							continue;
						}
					} else {
						// TODO, debug
						rtd_pr_rmm_err("(reuse):%d has buffer node but no remain count? list of cnt_total(%d) cnt_remain(%d)\n", 
							   __LINE__, data->cnt_total, data->cnt_remain);
						BUG();
					}
				} else if (_chunk->list) {
					/* The chunk list should be exist within release stage, 
					 * but not clear at this time. So all we can do for this 
					 * stage is wait for next seek chunk, try to unlock mutex, 
					 * and let release stage be finished.
					 */
					rtd_pr_rmm_err("(reuse):%d no buffer node? list of cnt_total(%d) cnt_remain(%d), zone(%d), chunk(curr[%d])\n", 
								   __LINE__, data->cnt_total, data->cnt_remain,
								   info->alloc_zone_index, c_idx);
					ret = SEEK_CHUNKS_STATE_REUSE_CHUNK_WAIT;
					goto seek_out;
				}
				else
#endif
				{
					chunk_available_size = 0;
					info->chunk_start_index = c_idx + 1;
					continue;
				}
			}

			info->alloc_addr_array[c_idx] = vbm_heap_chunk_list[info->alloc_zone_index][c_idx].chunk_addr;
			info->alloc_size_array[c_idx] = vbm_heap_chunk_list[info->alloc_zone_index][c_idx].chunk_size;
			if (vbm_heap_chunk_list[info->alloc_zone_index][c_idx].reserved)
				info->flags_array[c_idx] |= VBM_CHUNK_FLAGS_RESERVED;

			chunk_available_size += vbm_heap_chunk_list[info->alloc_zone_index][c_idx].chunk_size;

			if (chunk_available_size >= new_size) {
				unsigned long last_chunk_unused_size = chunk_available_size - new_size;

				info->chunk_end_index = c_idx;
				find_chunks = true;
				info->alloc_size_array[c_idx] -= last_chunk_unused_size;

				if (prev_source_chunk_end_index != -1) {
					if (info->chunk_end_index == prev_source_chunk_end_index) {
						unsigned long prev_source_chunk_end_alloc_size = 
							vbm_heap_chunk_list[info->alloc_zone_index][c_idx].chunk_size - 
							vbm_heap_chunk_list[info->alloc_zone_index][c_idx].free_size;

						if (prev_source_chunk_end_alloc_size < info->alloc_size_array[c_idx])
							info->growup_size_array[c_idx] = info->alloc_size_array[c_idx] - prev_source_chunk_end_alloc_size;
					} else if (info->chunk_end_index > prev_source_chunk_end_index) {
						info->growup_size_array[c_idx] = info->alloc_size_array[c_idx];
					} else if (prev_source_chunk_start_index != -1 && (info->chunk_start_index != prev_source_chunk_start_index)) {
						rtd_pr_rmm_info("reuse: zone[%d], chunk start idx(%d/%d) not match, re-seek\n",
										info->alloc_zone_index, info->chunk_start_index, prev_source_chunk_start_index);
						find_chunks = false;
						chunk_reseek_start_idx = prev_source_chunk_start_index;
						goto chunk_reseek;
					}
				}
					
				break;
			}

			/* For previous source end index not match with current source end index */
			if (prev_source_chunk_end_index != -1) {
				if (c_idx == prev_source_chunk_end_index) {
					unsigned long prev_source_chunk_end_alloc_size = 
						vbm_heap_chunk_list[info->alloc_zone_index][c_idx].chunk_size - 
						vbm_heap_chunk_list[info->alloc_zone_index][c_idx].free_size;

					if (prev_source_chunk_end_alloc_size < info->alloc_size_array[c_idx])
						info->growup_size_array[c_idx] = info->alloc_size_array[c_idx] - prev_source_chunk_end_alloc_size;
				} else if (c_idx > prev_source_chunk_end_index) {
					info->growup_size_array[c_idx] = info->alloc_size_array[c_idx];
				}
			}
		}

		if (find_chunks) {
			/* Set the chunk as used */
			for (c_idx = info->chunk_start_index; c_idx <= info->chunk_end_index; c_idx++) {
				info->flags_array[c_idx] |= VBM_CHUNK_FLAGS_BUDDY_ALLOCATED;
				vbm_heap_chunk_list[info->alloc_zone_index][c_idx].used = 1;
				vbm_heap_chunk_list[info->alloc_zone_index][c_idx].free_size = 
					vbm_heap_chunk_list[info->alloc_zone_index][c_idx].chunk_size - info->alloc_size_array[c_idx];

				if (info->growup_size_array[c_idx]) {
					rtd_pr_rmm_info("reuse: end_idx(prev=%d, curr=%d), growup.z[%d]c[%d].size=%lx, total_new_size=%lx\n", 
							prev_source_chunk_end_index, info->chunk_end_index, info->alloc_zone_index, c_idx, 
							info->growup_size_array[c_idx], new_size);
				}
			}
			info->new_size_delta = new_size - size;
			break;
		} else if (new_size != size) {
			rtd_pr_rmm_info("reuse: not available(0x%lx) for zone[%d] size(0x%lx to 0x%lx)\n", 
							chunk_available_size, info->alloc_zone_index, size, new_size);
		}
	}

	mutex_unlock(&chunk_list_mutex);

	if (!find_chunks || ((z_idx == (zone_start_idx + CHUNK_ZONE_COUNT)) && (c_idx == MAX_CHUNKS_PER_ZONE))) {
		rtd_pr_rmm_err("[vbm_cma] No free space for VBM CMA allocation find?(%d) zidx=%d cidx=%d, size = 0x%lx\n",
			   find_chunks, z_idx, c_idx, new_size);
		vbm_heap_dump_chunk_list();
		ret = SEEK_CHUNKS_STATE_ERROR;
	}

	return ret;

seek_out:
	mutex_unlock(&chunk_list_mutex);
	return ret;
}

/* Return phys offset of first chunk */
static unsigned long vbm_heap_alloc_chunks(struct vbm_chunk_array_info *info, struct vbm_heap_pool *reuse_genpool)
{
	int c_idx;
	char type[8] = {'\0'};
	unsigned long offset = INVALID_VAL;

	// we will use alloc_zone_index and chunk_start_index/chunk_end_index to alloc cma
	for (c_idx = info->chunk_start_index; c_idx <= info->chunk_end_index; c_idx++)
 	{
		struct page *vbm_chunk_pages = NULL;
 		unsigned long vbm_chunk_phys_addr = INVALID_VAL;
		unsigned long alloc_addr = info->alloc_addr_array[c_idx];
		unsigned long alloc_size = info->alloc_size_array[c_idx];

		vbm_chunk_phys_addr = alloc_addr;

		if ((info->flags_array[c_idx] & VBM_CHUNK_FLAGS_BUDDY_ALLOCATED) && !info->growup_size_array[c_idx])
			strncpy(type, "(resv)", 6);
		else if ((info->flags_array[c_idx] & VBM_CHUNK_FLAGS_BUDDY_ALLOCATED) && info->growup_size_array[c_idx]) {
			strncpy(type, "(grow)", 6);
			alloc_addr += (info->alloc_size_array[c_idx] - info->growup_size_array[c_idx]); // prev_source_chunk_end_alloc_size
			alloc_size = info->growup_size_array[c_idx];
		}
 
 		/* Allocate chunk memory by specific size and address */
		if (info->flags_array[c_idx] & VBM_CHUNK_FLAGS_RESERVED) {
			vbm_chunk_pages = vbm_chunk_res_pages[info->alloc_zone_index][c_idx];
			if (info->growup_size_array[c_idx])
				vbm_chunk_pages += ((info->alloc_size_array[c_idx] - info->growup_size_array[c_idx]) >> PAGE_SHIFT);
		} else if (!info->reuse_list || info->growup_size_array[c_idx]) {
			unsigned long over_base = INVALID_VAL, over_size = 0; 
			int ret = vbm_heap_pool_addr_in_pool(reuse_genpool, alloc_addr, alloc_size, &over_base, &over_size);
			if (ret < 0) {
				if (ret == ADDR_OVER_POOL_END) {
					rtd_pr_rmm_err("%s allocate new cma range, zone[%d] chunk[%d], size 0x%lx, address 0x%lx, reason(ADDR_OVER_POOL_END)\n",  __func__,
						   info->alloc_zone_index, c_idx, over_size, over_base);
					alloc_specific_contiguous_memory(over_size, over_base, GFP_VBM_CMA, __func__);
					vbm_chunk_pages = pfn_to_page(alloc_addr >> PAGE_SHIFT);
				} else if (ret == ADDR_OVER_POOL_BASE) {
					rtd_pr_rmm_err("%s Fail to allocate zone[%d] chunk[%d], size 0x%lx, address 0x%lx, reason(ADDR_OVER_POOL_BASE)\n",  __func__,
						   info->alloc_zone_index, c_idx, alloc_size, alloc_addr);
					BUG();
				} else {
					vbm_chunk_pages = alloc_specific_contiguous_memory(alloc_size, alloc_addr, GFP_VBM_CMA, __func__);
				}

				if (!vbm_chunk_pages) {
					rtd_pr_rmm_err("%s Fail to allocate zone[%d] chunk[%d], size 0x%lx, address 0x%lx\n",  __func__,
						   info->alloc_zone_index, c_idx, alloc_size, alloc_addr);
					return INVALID_VAL;
				}
			} else {
				rtd_pr_rmm_info("%s allocated cma range, zone[%d] chunk[%d], size 0x%lx, address 0x%lx\n", __func__,
						info->alloc_zone_index, c_idx, alloc_size, alloc_addr);
				vbm_chunk_pages = pfn_to_page(alloc_addr >> PAGE_SHIFT);
			}
		}

 		/* Exam if allocated chunk address is matched to chunk list */
		if (vbm_chunk_pages)
			vbm_chunk_phys_addr = page_to_pfn(vbm_chunk_pages) << PAGE_SHIFT;

		rtd_pr_rmm_info("[vbm_cma] %sAlloc phys_addr from VBM heap zone[%d] chunk[%d] is 0x%lx, size is 0x%lx\n", 
				type, info->alloc_zone_index, c_idx, vbm_chunk_phys_addr, alloc_size);
		if (vbm_chunk_phys_addr != alloc_addr)
		{
			rtd_pr_rmm_err("%s allocated addr 0x%lx mismatch zone[%d] chunk[%d] addr(0x%lx), size is 0x%lx\n", __func__, vbm_chunk_phys_addr,
				info->alloc_zone_index, c_idx, alloc_addr, alloc_size);
			if (vbm_chunk_pages)
				free_contiguous_memory(vbm_chunk_pages, alloc_size >> PAGE_SHIFT);
			return INVALID_VAL;
		}

		/* Clear the chunk allocated page */
		if (vbm_chunk_pages && vbm_heap_clear_chunk(vbm_chunk_pages, alloc_size))
		{
			rtd_pr_rmm_err("%s clear zone[%d] chunk[%d] page failed\n", __func__, info->alloc_zone_index, c_idx);
			return INVALID_VAL;
		}

 		/* Set the chunk as used */
		info->flags_array[c_idx] |= VBM_CHUNK_FLAGS_USED;
 
 		/* Record the first allocated chunk page as vbm_pages */
		if (c_idx == info->chunk_start_index)
 		{
 			offset = info->alloc_addr_array[c_idx]; // return the chunk start address
 		}
 	}

	return offset;
}

static struct dma_buf *vbm_heap_alloc_frames(struct dma_heap *heap, struct vbm_chunk_array_info *info, 
						struct vbm_heap_pool *reuse_genpool, bool need_protect,
						size_t size, unsigned int frame_count, unsigned long fd_flags, unsigned long heap_flags)
{
	int i = 0, frame_idx = 0;
	int occupied_frame = 0;
	unsigned int frame_size = size / frame_count;
	unsigned long base_offset = 0;
	struct dma_buf *dmabuf;
	struct dma_buf *first_dmabuf;
	struct vbm_heap_buffer *first_buffer = NULL;
	struct vbm_heap_buffer *buffer = NULL;
	struct vbm_heap_list_priv_data data[1] = {0};

	BUG_ON(!PAGE_ALIGNED(frame_size));

	data->alloc_zone_idx = info->alloc_zone_index;
	data->chunk_start_idx = info->chunk_start_index;
	data->chunk_cnt = info->chunk_end_index - info->chunk_start_index + 1;
	if (need_protect)
		data->is_protected = true;
	else
		data->is_protected = false;
	data->protect_addr = info->protect_address;
	data->protect_size = info->protect_size;

	if (info->reuse_list) {
		struct vbm_heap_list_priv_data list_data[1] = {0};
		first_buffer = vbm_heap_list_get_priv_data(info->reuse_list, list_data);
		if (!first_buffer) {
			rtd_pr_rmm_err("(reuse):%d no buffer node? (snoop without lock) list of cnt_total(%d) cnt_remain(%d)\n", 
				   __LINE__,  list_data->cnt_total, list_data->cnt_remain);
			BUG();
		}
		occupied_frame = list_data->cnt_remain;

		data->cnt_total = data->cnt_remain = occupied_frame + frame_count;
		data->is4K = (heap_flags & DMA_HEAP_FLAG_USER_VIDEO_4K_LUMA || 
					 heap_flags & DMA_HEAP_FLAG_USER_VIDEO_4K_CHROMA) ? true : false;
		vbm_heap_list_reset_buffer_data(info->reuse_list, data);
		vbm_heap_list_set_priv_data(info->reuse_list, data);
	}

	rtd_pr_rmm_info("%s: alloc frame from %d(occupied) to %d\n", __func__, occupied_frame, frame_count + occupied_frame);

	// for each for current allocation
	for (i = occupied_frame; i < (frame_count + occupied_frame); i++) { // each single frame
		struct page *base_pages = NULL;

		base_offset = vbm_heap_pool_alloc(reuse_genpool, frame_size);
		if (base_offset == INVALID_VAL) {
			rtd_pr_rmm_err("frame[%d] pool alloc failure, pool used(%lx), size(%lx)\n", i, 
						   vbm_heap_pool_used(reuse_genpool), vbm_heap_pool_size(reuse_genpool));
			BUG();
//			return -ENOMEM;
		}

		base_pages = pfn_to_page(base_offset >> PAGE_SHIFT);
		dmabuf = vbm_heap_chunk_allocate_frame_single(heap, base_pages, frame_size, fd_flags, heap_flags);
		if (IS_ERR(dmabuf)) {
			BUG();
		}
		buffer = dmabuf->priv;
		buffer->alloc_zone_idx = data->alloc_zone_idx;
		buffer->chunk_start_idx = data->chunk_start_idx;
		buffer->chunk_cnt = data->chunk_cnt;
		buffer->is_protected = data->is_protected;
		buffer->protect_addr = data->protect_addr;
		buffer->protect_size = data->protect_size;
		buffer->resv1 = (unsigned long)dmabuf; // for outer tracking

		rtd_pr_rmm_debug("vbm buffer pagecount = %ld, phys_addr = 0x%lx, zone idx = %d, chunk start idx = %d, chunk count = %d\n",
				 buffer->pagecount, buffer->phys_addr, buffer->alloc_zone_idx, buffer->chunk_start_idx, buffer->chunk_cnt);
		rtd_pr_rmm_debug("    is_protected = %d, protect_addr = 0x%lx, protect_size = 0x%lx\n",
				 buffer->is_protected, buffer->protect_addr, buffer->protect_size);

		if (frame_idx == 0) { // first frame for current allocation
			first_dmabuf = dmabuf;

			if (first_buffer == NULL) { // no reuse
				data->cnt_total = data->cnt_remain = frame_count;
				data->is4K = (heap_flags & DMA_HEAP_FLAG_USER_VIDEO_4K_LUMA || 
							 heap_flags & DMA_HEAP_FLAG_USER_VIDEO_4K_CHROMA) ? true : false;

				first_buffer = buffer;
				vbm_heap_list_init(first_buffer);
				vbm_heap_list_set_priv_data(buffer->list, data);
			} else {
				// list private data already re-apply at the beginning
				buffer->list = first_buffer->list;
			}

			rtd_pr_rmm_debug("%s:%d: first buffer list(%lx), reuse(%lx)\n", __func__, __LINE__, buffer->list, info->reuse_list);
		} else {
			buffer->list = first_buffer->list;
		}

		vbm_heap_list_add(buffer);
		frame_idx ++;
	}

	// debug
	vbm_heap_list_dump(first_dmabuf->priv);

	return first_dmabuf;
}

static struct dma_buf *vbm_heap_allocate(struct dma_heap *heap,
					 unsigned long len,
					 unsigned long fd_flags,
					 unsigned long heap_flags)
{
	struct vbm_heap_buffer *buffer = NULL;
	size_t size = PAGE_ALIGN(len);
#ifndef CONFIG_VBM_CMA
	struct page *vbm_pages;
	struct dma_buf *dmabuf;
#endif
	int ret = -ENOMEM;

#ifdef CONFIG_VBM_CMA
	int z_idx;
	int zone_start_idx;
	int c_idx;
	int chunk_seek_state = 0;
	unsigned int frame_count = 0;
	struct dma_buf *first_dmabuf;
	unsigned long offset;

	bool need_protect = false;
#ifdef CONFIG_CHUNK_REUSE
	struct vbm_heap_pool reuse_genpool[] = {{ .pool = NULL, .growup = false }};
#endif
	struct vbm_chunk_array_info info[] = {0};

	rtd_pr_rmm_info("Request buffer allocation len 0x%lx\n", len);

	memset(info, 0, sizeof(struct vbm_chunk_array_info));

	// find big chunk
	/* Decide zone start index*/
	if (!(heap_flags & DMA_HEAP_FLAG_USER_MASK)) {
		rtd_pr_rmm_err("Invalid zone argument of heap_flags 0x%lx\n", heap_flags);
		goto free_buffer;
	} else {
		if (heap_flags & DMA_HEAP_FLAG_USER_VIDEO_4K_LUMA) {
			zone_start_idx = CHUNK_ZONE_4K_LUMA;
		} else if (heap_flags & DMA_HEAP_FLAG_USER_COBUFFER) {
			zone_start_idx = CHUNK_ZONE_COBUFFER;
		} else if (heap_flags & DMA_HEAP_FLAG_USER_VIDEO_SUB_2K) {
			for (z_idx = CHUNK_ZONE_SMALL_1; z_idx < CHUNK_ZONE_COUNT; z_idx++) {
				for (c_idx = 0; c_idx < MAX_CHUNKS_PER_ZONE; c_idx++) {
					if (vbm_heap_chunk_list[z_idx][c_idx].used) {
						break;
					}
				}
				if (c_idx == MAX_CHUNKS_PER_ZONE) {
					break;
				}
			}
			zone_start_idx = z_idx;
			rtd_pr_rmm_info("SUB 2K, zone_start_idx = %d\n", zone_start_idx);
		} else {
			zone_start_idx = CHUNK_ZONE_OTHER;
		}
	}

	/* Get frame count for frame based allocation*/
	if (heap_flags & DMA_HEAP_FLAG_ALLOC_FRAMES) {
		frame_count = heap_flags & DMA_HEAP_FLAG_FRAMES_NUM_MASK;
	} else {
		frame_count = 1;
	}

	if (heap_flags & DMA_HEAP_FLAG_NEED_SECURE_PROTECT) {
		need_protect = true;
	}

wait_for_reuse_pool:
	/* Seeking avaliable consecutive chunks within a zone */
	chunk_seek_state = vbm_heap_seek_chunks(info, reuse_genpool, zone_start_idx, size, frame_count, heap_flags);
	if (chunk_seek_state == SEEK_CHUNKS_STATE_REUSE_CHUNK_WAIT)
		goto wait_for_reuse_pool;
	else if (chunk_seek_state == SEEK_CHUNKS_STATE_ERROR)
		goto free_buffer;

	/* Seek chunks OK */
	rtd_pr_rmm_info("alloc_zone_index = %d, chunk_start_index = %d, chunk_end_index = %d\n",
			info->alloc_zone_index, info->chunk_start_index, info->chunk_end_index);

	/* Allocate the occupied chunks from seek */
	offset = vbm_heap_alloc_chunks(info, reuse_genpool);
	if (offset == INVALID_VAL) {
		rtd_pr_rmm_err("alloc chunks assert: offset invalid\n");
		BUG();
	}
#ifdef CONFIG_CHUNK_REUSE
	if (vbm_heap_pool_new_or_growup(reuse_genpool, offset, size + info->new_size_delta) < 0) {
		rtd_pr_rmm_warn("reuse pool new or growup fail, addr/size(%lx/%lx), zone/chunk(%d/%d/%d), continue\n", 
				offset, size, info->alloc_zone_index, info->chunk_start_index, info->chunk_end_index);

		if (reuse_genpool->pool && info->reuse_list)
			goto free_buffer; // pool growup failure
		else if (reuse_genpool->pool && !info->reuse_list) {
			rtd_pr_rmm_err("assert: has pool but the first source allocation, impossle reach\n");
			BUG();
		} else {
			// no pool, don't need to consider pool free or destroy
			goto free_vbm;
		}
	}
#endif
	if (need_protect) {
		if (reuse_genpool->growup) {
			/* FIXME, growup need to re-apply the new range for 
			 * memory protection without wasting a new one 
			 * protection region set
			 */
			rtd_pr_rmm_warn("reuse: warning for enabling the protection\n");
		}

		info->protect_address = offset;
		info->protect_size = size + info->new_size_delta;
		ret = rtkvdec_svp_enable_protection(info->protect_address, info->protect_size, TYPE_SVP_PROTECT_VBM);
		if (ret != 0) {
			rtd_pr_rmm_err("%s Secure protect address 0x%lx size 0x%lx failed\n", __func__, info->protect_address, info->protect_size);
			goto free_vbm;
		}
	}

	/* Export each single frame within a big chunk
	 * TODO: handle the error return code
	 */
	first_dmabuf = vbm_heap_alloc_frames(heap, info, reuse_genpool, need_protect, size, frame_count, fd_flags, heap_flags);

#ifdef CONFIG_CHUNK_REUSE
	mutex_lock(&chunk_list_mutex);
	for (c_idx = info->chunk_start_index; c_idx <= info->chunk_end_index; c_idx++) {
		struct vbm_heap_buffer *first_buffer = first_dmabuf->priv;
		VBM_CHUNK_INFO *_chunk = NULL;

		_chunk = &vbm_heap_chunk_list[info->alloc_zone_index][c_idx];

		// duplicated needed information
		_chunk->list = first_buffer->list;
		(&_chunk->genpool)->pool = reuse_genpool->pool;
		(&_chunk->genpool)->growup = reuse_genpool->growup;
	}
	mutex_unlock(&chunk_list_mutex);
#endif

	return first_dmabuf;
#else
	pgoff_t pg;
	struct vbm_heap *vbm_heap = dma_heap_get_drvdata(heap);
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	pgoff_t pagecount = size >> PAGE_SHIFT;

	rtd_pr_rmm_info("Request buffer allocation len 0x%lx\n", len);

	buffer = kzalloc(sizeof(*buffer), GFP_KERNEL);
	if (!buffer) {
		rtd_pr_rmm_err("Can't allocate buffer info\n");
		return ERR_PTR(-ENOMEM);
	}

	INIT_LIST_HEAD(&buffer->attachments);
	mutex_init(&buffer->lock);
	buffer->len = size;
	buffer->flags = heap_flags;
	buffer->uncached = (buffer->flags & DMA_HEAP_FLAG_CACHED) ? false : true;

	if (buffer->flags & DMA_HEAP_FLAG_FROM_DCU1) {
		vbm_pages = dvr_malloc_page_mesg(size, GFP_DCU1, "DMABUF:01:");
	} else if (buffer->flags & DMA_HEAP_FLAG_FROM_DCU2) {
		vbm_pages = dvr_malloc_page_mesg(size, GFP_DCU2, "DMABUF:02:");
	} else {
		vbm_pages = dvr_malloc_page_mesg(size, GFP_DCU2_FIRST, "DMABUF:08:");
	}
	if (!vbm_pages) {
		rtd_pr_rmm_err("Fail to allocate buffer\n");
		goto free_buffer;
	}

	/* Clear the chunk allocated page */
	vbm_heap_clear_chunk(vbm_pages, size);

	buffer->pages = vmalloc(pagecount * sizeof(*buffer->pages));
	if (!buffer->pages) {
		ret = -ENOMEM;
		rtd_pr_rmm_err("Fail to allocate page array\n");
		goto free_vbm;
	}

	for (pg = 0; pg < pagecount; pg++)
		buffer->pages[pg] = &vbm_pages[pg];

	buffer->vbm_pages = vbm_pages;
	buffer->heap = vbm_heap;
	buffer->pagecount = pagecount;
	buffer->phys_addr = page_to_phys(vbm_pages);

	/* create the dmabuf */
	exp_info.ops = &vbm_heap_buf_ops;
	exp_info.size = buffer->len;
	exp_info.flags = fd_flags;
	exp_info.priv = buffer;
	dmabuf = dma_buf_export(&exp_info);
	if (IS_ERR(dmabuf)) {
		ret = PTR_ERR(dmabuf);
		goto free_pages;
	}

	rtd_pr_rmm_info("vbm heap alloc 0x%lx %ld uncached(%d)\n", buffer->phys_addr, buffer->len, buffer->uncached);
	return dmabuf;

free_pages:
	vfree(buffer->pages);
#endif
free_vbm:
	rtd_pr_rmm_err("%s free_vbm\n", __func__);
#ifdef CONFIG_VBM_CMA
	/*  free pages from specific zone's chunk start index to chunk end index */
	for (c_idx = info->chunk_start_index; c_idx <= info->chunk_end_index; c_idx++)
	{
		if (info->flags_array[c_idx] & VBM_CHUNK_FLAGS_USED) {
			struct page *vbm_chunk_pages;
			int vbm_chunk_count;

			vbm_chunk_pages = pfn_to_page(info->alloc_addr_array[c_idx] >> PAGE_SHIFT);
			vbm_chunk_count = info->alloc_size_array[c_idx] >> PAGE_SHIFT;

			if ((info->flags_array[c_idx] & VBM_CHUNK_FLAGS_BUDDY_ALLOCATED) && info->growup_size_array[c_idx]) {
				unsigned long alloc_addr = info->alloc_addr_array[c_idx] + 
					(info->alloc_size_array[c_idx] - info->growup_size_array[c_idx]); // prev_source_chunk_end_alloc_size

				vbm_chunk_pages = pfn_to_page(alloc_addr >> PAGE_SHIFT);
				vbm_chunk_count = info->growup_size_array[c_idx] >> PAGE_SHIFT;
			}

			if (info->flags_array[c_idx] & VBM_CHUNK_FLAGS_RESERVED) {
				rtd_pr_rmm_info("free resevred zone[%d] chunk[%d], BYPASS\n", info->alloc_zone_index, c_idx);
			} else if (!info->reuse_list || info->growup_size_array[c_idx]) {
				if (free_contiguous_memory(vbm_chunk_pages, vbm_chunk_count)) {
					rtd_pr_rmm_err("free zone[%d] chunk[%d] failed\n", info->alloc_zone_index, c_idx);
				} else {
					rtd_pr_rmm_info("free zone[%d] chunk[%d] successfully\n", info->alloc_zone_index, c_idx);
				}
			}
		}
	}

	if (reuse_genpool->growup) {
		rtd_pr_rmm_err("reuse: can not handle growed pool entry\n");
		BUG();
	}

	mutex_lock(&chunk_list_mutex);
	for (c_idx = info->chunk_start_index; c_idx <= info->chunk_end_index; c_idx++) {
		vbm_heap_chunk_list[info->alloc_zone_index][c_idx].used = 0;
		vbm_heap_chunk_list[info->alloc_zone_index][c_idx].free_size = 
			vbm_heap_chunk_list[info->alloc_zone_index][c_idx].chunk_size;
	}
	mutex_unlock(&chunk_list_mutex);
#else
	dvr_free_page_mesg(vbm_pages);
#endif
free_buffer:
	if (buffer)
		kfree(buffer);

	return ERR_PTR(ret);
}

static const struct dma_heap_ops vbm_heap_ops = {
	.allocate = vbm_heap_allocate,
};

#ifdef CONFIG_VBM_CMA

void vbm_heap_cma_init(struct vbm_heap *vbm_heap)
{
	unsigned long total_chunk_size = 0;
	unsigned long *p = NULL;
	unsigned long (*allocator)[MAX_CHUNKS_PER_ZONE];
	int pre_allocate[CHUNK_ZONE_COUNT][MAX_CHUNKS_PER_ZONE] = {0};

	int i, j;

	mutex_lock(&chunk_list_mutex);
	p = vbm_heap_get_allocator();
	allocator = (unsigned long (*)[MAX_CHUNKS_PER_ZONE])p;

	for (i = 0; i < CHUNK_ZONE_COUNT; i++) {
		for (j = 0; j < MAX_CHUNKS_PER_ZONE; j++) {
			vbm_heap_chunk_list[i][j].chunk_addr = vbm_heap->base + total_chunk_size;
			vbm_heap_chunk_list[i][j].free_size = vbm_heap_chunk_list[i][j].chunk_size = allocator[i][j];
			vbm_heap_chunk_list[i][j].used = 0;
#ifdef CONFIG_CHUNK_REUSE
			vbm_heap_chunk_list[i][j].list = NULL;
#endif

			total_chunk_size += vbm_heap_chunk_list[i][j].chunk_size;

			if ((PRE_ALLOCATE_SIZE_THRESHOLD != INVALID_VAL) && (vbm_heap_chunk_list[i][j].chunk_size >= PRE_ALLOCATE_SIZE_THRESHOLD)) {
				vbm_heap_chunk_list[i][j].reserved = pre_allocate[i][j] = 1;
			} else if ((PRE_ALLOCATE_SIZE_THRESHOLD == INVALID_VAL) && (unlikely((unsigned char *)&PRE_ALLOCATE_SIZE_INDEX_2D_BIT != NULL)) && (PRE_ALLOCATE_SIZE_INDEX_2D_BIT[i][j] != 0)) {
				vbm_heap_chunk_list[i][j].reserved = pre_allocate[i][j] = 1;
			} else {
				vbm_heap_chunk_list[i][j].reserved = pre_allocate[i][j] = 0;
			}

			vbm_chunk_res_pages[i][j] = 0;
		}
	}
	mutex_unlock(&chunk_list_mutex);

	if (total_chunk_size > vbm_heap->size) {
		rtd_pr_rmm_err("ERROR, total_chunk_size %ld MB > vbm_size %lld MB\n", total_chunk_size/__MB__, vbm_heap->size/__MB__);
		return;
	}

	for (i = 0; i < CHUNK_ZONE_COUNT; i++) {
		for (j = 0; j < MAX_CHUNKS_PER_ZONE; j++) {
			if (pre_allocate[i][j] && vbm_heap_chunk_list[i][j].chunk_size) {
				vbm_chunk_res_pages[i][j] = alloc_specific_contiguous_memory(vbm_heap_chunk_list[i][j].chunk_size,
					vbm_heap_chunk_list[i][j].chunk_addr, GFP_VBM_CMA, __func__);
				if (!vbm_chunk_res_pages[i][j]) {
					rtd_pr_rmm_err("%s Fail to pre-allocate zone[%d] chunk[%d], size 0x%lx, address 0x%lx\n",  __func__,
						i, j, vbm_heap_chunk_list[i][j].chunk_size, vbm_heap_chunk_list[i][j].chunk_addr);
					goto free_res_chunk;
				}
			}
		}
	}

	vbm_heap_dump_chunk_list();

	return;

free_res_chunk:
	rtd_pr_rmm_err("%s error!! free_res_chunk\n", __func__);
	for (i = 0; i < CHUNK_ZONE_COUNT; i++) {
		for (j = 0; j < MAX_CHUNKS_PER_ZONE; j++) {
			if (vbm_chunk_res_pages[i][j]) {
				if (free_contiguous_memory(vbm_chunk_res_pages[i][j], vbm_heap_chunk_list[i][j].chunk_size >> PAGE_SHIFT)) {
					rtd_pr_rmm_err("%s Fail to free pre-allocate zone[%d] chunk[%d], size 0x%lx, address 0x%lx\n",  __func__,
						i, j, vbm_heap_chunk_list[i][j].chunk_size, vbm_heap_chunk_list[i][j].chunk_addr);
				} else {
					vbm_chunk_res_pages[i][j] = 0;
					vbm_heap_chunk_list[i][j].reserved = 0;
				}
			}
		}
	}

}
#endif


extern struct dma_buf *pQShowVBM;
extern void vbm_heap_to_phys_register(unsigned long (*fp)(struct dma_buf *dmabuf));
extern void vbm_heap_to_dmabuf_list_register(int (*fp)(struct dma_buf *dmabuf, unsigned long buf_array[], int array_num));
static int vbm_heap_create(void)
{
	struct vbm_heap *vbm_heap = NULL;
	struct dma_heap_export_info exp_info;
#ifdef CONFIG_VBM_CMA
	unsigned long size = 0, base = 0;
#endif

	// for getting qshow_memory devicetree information
	struct device_node *qshow_np;
	unsigned long qshow_start = 0, qshow_size = 0;
	u64 tmp64[2];

	vbm_heap = kzalloc(sizeof(*vbm_heap), GFP_KERNEL);
	if (!vbm_heap) {
		rtd_pr_rmm_err("no vbm heap\n");
		return -ENOMEM;
	}

#ifdef CONFIG_VBM_CMA
	size = (unsigned int)carvedout_buf_query(CARVEDOUT_CMA_VBM, (void *)&base);
	if (!size) {
		kfree(vbm_heap);
		rtd_pr_rmm_err("no reserved space for vbm heap\n");
		return -ENOMEM;
	}
	size = ALIGN(size, 1*_MB_);
	vbm_heap->base = base;
	vbm_heap->size = size;
#endif

	exp_info.name = "vbm";
	exp_info.ops = &vbm_heap_ops;
	exp_info.priv = vbm_heap;

	vbm_heap->heap = dma_heap_add(&exp_info);
	if (IS_ERR(vbm_heap->heap)) {
		int ret = PTR_ERR(vbm_heap->heap);

		kfree(vbm_heap);
		return ret;
	}

#ifdef CONFIG_VBM_CMA
	rtd_pr_rmm_info("[vbm_cma] VBM CMA initialized\n");
	vbm_heap_cma_init(vbm_heap);
	proc_vbm_heap_info_init();

	rtd_pr_rmm_info("add vbm_heap base = 0x%lx, size = 0x%lx\n", (unsigned long)vbm_heap->base, (unsigned long)vbm_heap->size);
#endif

	vbm_heap_to_phys_register(vbm_heap_to_phys);
	vbm_heap_to_dmabuf_list_register(vbm_heap_to_dmabuf_list);

	/*
	 * It's the early time for pre-allocate VBM chunk to preserve memory region the quickshow used at boot
	 */
	// get quick-show dts rtk_cma info
	qshow_np = of_find_node_by_path("/rtk_cma/qshow_scaler");
	if ((of_property_read_u64_array(qshow_np, "reg", tmp64, 2)) == 0 && is_QS_active()) {
		rtd_pr_rmm_info("found qshow 0x%lx 0x%lx in ufdt\n", tmp64[0], tmp64[1]);
		if (tmp64[1] != 0) { // size 0 for disabled
			qshow_start = tmp64[0];
			qshow_size = tmp64[1];
		}
		else {
			rtd_pr_rmm_info("qshow disabled in ufdt\n");
		}
	}
	else {
		rtd_pr_rmm_info("NO qshow in ufdt\n");
	}

	if (qshow_size) {
		unsigned long phy_addr = 0;
		struct dma_buf *dmabuf = NULL;

		if (!pQShowVBM) {
#ifdef HACK_QSHOW_VBM_FROM_ZONE0
			if (qshow_start) { // early allocate (pre-occupy) from the beginning of vbm heap
				dmabuf = dvr_dmabuf_chunk_malloc(qshow_size, VBM_BUF_DECOMP, DMA_HEAP_USER_VIDEO_4K_LUMA, false, (unsigned long **)&phy_addr);
				//dmabuf = dma_heap_buffer_alloc(vbmheap, qshow_size, O_RDWR, DMA_HEAP_FLAG_USER_VIDEO_4K_LUMA);

				if (phy_addr != qshow_start) {
					rtd_pr_rmm_err("%s QShow preserved vbm heap is unexpcected 0x%lx@0x%lx 0x%lx\n", __func__, qshow_size, qshow_start, phy_addr);
				}
			}
#else // always use zone1 for qshow_scaler, ignore qshow_start in dts
			// allocate(preserve) from smaller chunk zone instead
			//  it should be early enough to exactly allocate from zone1 beginning becuase it should be 
			//  a predefined hardcode value becuase QS will also get it by get_vbm_heap_qshow_scaler_address()
			dmabuf = dvr_dmabuf_chunk_malloc(qshow_size, VBM_BUF_DECOMP, DMA_HEAP_USER_COBUFFER, false, (unsigned long **)&phy_addr);
			rtd_pr_rmm_notice("%s QShow preserves VBM USER_COBUFFER 0x%lx\n", __func__, phy_addr);
#endif
			// test get_vbm_heap_qshow_scaler_address
			if (phy_addr != get_vbm_heap_qshow_scaler_address()) {
				rtd_pr_rmm_err("%s QShow preserved vbm heap is unexpcected 0x%lx@0x%lx 0x%lx\n", __func__, qshow_size, phy_addr, get_vbm_heap_qshow_scaler_address());
			}
			if ( IS_ERR_OR_NULL(dmabuf) ) {
				rtd_pr_rmm_err("%s QShow cannot preserve vbm heap 0x%lx\n", __func__, qshow_size);
			}
			pQShowVBM = dmabuf;
		}
		else {
			rtd_pr_rmm_err("%s pQShowVBM is not empty\n", __func__);
		}

		//debug info
		rtd_pr_rmm_info("%s QShow preserved 0x%lx for qshow vbm(%p) 0x%lx@0x%lx \n", __func__, (unsigned long *)dvr_dmabuf_to_phys(pQShowVBM), pQShowVBM, qshow_size, qshow_start);
		vbm_heap_dump_chunk_list();

	}

	return 0;
}
module_init(vbm_heap_create);

#ifdef CONFIG_VBM_CMA
int vbm_heap_info_proc_show(struct seq_file *m, void *v)
{
	int zone_idx, chunk_idx;

	mutex_lock(&chunk_list_mutex);
	for (zone_idx = 0; zone_idx < CHUNK_ZONE_COUNT; zone_idx++) {
		seq_printf(m, "[vbm_heap] zone %d:\n", zone_idx);
		for (chunk_idx = 0; chunk_idx < MAX_CHUNKS_PER_ZONE; chunk_idx++) {
			if (vbm_heap_chunk_list[zone_idx][chunk_idx].chunk_size > 0) {
				VBM_CHUNK_INFO *_chunk = NULL;
				struct vbm_heap_pool *reuse_genpool = NULL;
				struct gen_pool *pool = NULL;

				_chunk = &vbm_heap_chunk_list[zone_idx][chunk_idx];
				reuse_genpool = &_chunk->genpool;

				seq_printf(m, "    chunk %d: size = %ld MB, phys_addr = 0x%lx, reserved = %d, used = %d, free = %ld MB\n",
					chunk_idx,
					(vbm_heap_chunk_list[zone_idx][chunk_idx].chunk_size)/__MB__,
					vbm_heap_chunk_list[zone_idx][chunk_idx].chunk_addr,
					vbm_heap_chunk_list[zone_idx][chunk_idx].reserved,
					vbm_heap_chunk_list[zone_idx][chunk_idx].used,
					(vbm_heap_chunk_list[zone_idx][chunk_idx].free_size)/__MB__);

				if (pool) {
					seq_printf(m, "    pool(%lx), size(%lx), used(%lx)\n", (unsigned long)reuse_genpool->pool,
							   (unsigned long)vbm_heap_pool_size(reuse_genpool) ,(unsigned long)vbm_heap_pool_used(reuse_genpool));
				}
			}
		}
	}
	mutex_unlock(&chunk_list_mutex);

	return 0;
}

static int vbm_heap_info_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, vbm_heap_info_proc_show, NULL);
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static const struct proc_ops vbm_heap_info_proc_fops = {
	.proc_open		= vbm_heap_info_proc_open,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release	= single_release,
};
#else
static const struct file_operations vbm_heap_info_proc_fops = {
	.open		= vbm_heap_info_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif

static int __init proc_vbm_heap_info_init(void)
{
	proc_create("vbm_heap_info", 0, NULL, &vbm_heap_info_proc_fops);

	return 0;
}
#endif

MODULE_LICENSE("GPL v2");

#endif // BUILD_QUICK_SHOW
