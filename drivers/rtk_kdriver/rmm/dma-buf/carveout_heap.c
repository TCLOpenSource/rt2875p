// SPDX-License-Identifier: GPL-2.0
/*
 * DMABUF Carveout heap exporter
 *
 * Copyright (C) 2021 Realtek, Inc.
 *
 */

#define pr_fmt(fmt) "dmabuf_carveout: " fmt

#include <linux/version.h>
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
#include <linux/genalloc.h>
#include <linux/err.h>
#include <linux/highmem.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/scatterlist.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#include <linux/pageremap.h>
//#include <uapi/linux/dma-heap.h>
#include <rtk_kdriver/rmm/rmm_heap.h>
#include <rtd_log/rtd_module_log.h>

#include <mach/rtk_platform.h>

#ifdef CONFIG_VBM_CARVEOUT
#include <rtk_kdriver/rtk_vdec_svp.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define CONFIG_CHUNK_REUSE

struct carveout_heap_pool {
	struct gen_pool *pool;
	bool growup;
};

typedef struct carveout_heap_chunk {
	unsigned long	chunk_size;
	unsigned long	chunk_addr;
	unsigned int	reserved;

	unsigned int	used;
	unsigned long	free_size;  // free size if this chunk is not fully used

#ifdef CONFIG_CHUNK_REUSE
	struct carveout_heap_list *list;
	struct carveout_heap_pool genpool;
#endif
} CARVEOUT_CHUNK_INFO;

/* For 8K decode frame buffer */
unsigned long CARVEOUT_HEAP_8K_CHUNK_SIZE[CHUNK_ZONE_COUNT][MAX_CHUNKS_PER_ZONE] = {
	CARVEOUT_HEAP_8K_CHUNK_SIZE_ARRAY
};

unsigned long CARVEOUT_HEAP_8K_DISP_CHUNK_SIZE[CHUNK_ZONE_COUNT][MAX_CHUNKS_PER_ZONE] = {
	CARVEOUT_HEAP_8K_DISP_CHUNK_SIZE_ARRAY
};

CARVEOUT_CHUNK_INFO carveout_heap_chunk_list[CHUNK_ZONE_COUNT][MAX_CHUNKS_PER_ZONE];

static DEFINE_MUTEX(chunk_list_mutex);

struct carveout_heap_list_priv_data {
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

struct carveout_heap_list {
	struct list_head head;
	struct mutex lock;
	struct carveout_heap_list_priv_data data;
};
#endif

#define CARVEOUT_ALLOCATE_FAIL	-1

#define CARVEOUT_CHUNK_FLAGS_USED            0x1
#define CARVEOUT_CHUNK_FLAGS_RESERVED        0x2
#define CARVEOUT_CHUNK_FLAGS_BUDDY_ALLOCATED 0x4

struct carveout_chunk_array_info {
	int alloc_zone_index;
	int chunk_start_index;
	int chunk_end_index;
	unsigned long protect_address;
	unsigned long protect_size;
	bool need_protect;
	unsigned long new_size_delta;
	struct carveout_heap_list *reuse_list;
	struct carveout_heap_pool *reuse_genpool;
	unsigned long	alloc_addr_array[MAX_CHUNKS_PER_ZONE];   // as chunk addr for release
	unsigned long	alloc_size_array[MAX_CHUNKS_PER_ZONE];   // as chunk size for resease
	unsigned long	free_size_array[MAX_CHUNKS_PER_ZONE];
	unsigned long	growup_size_array[MAX_CHUNKS_PER_ZONE];
	unsigned int	flags_array[MAX_CHUNKS_PER_ZONE];
};

struct carveout_heap {
	struct dma_heap *heap;
	struct gen_pool *pool;
	phys_addr_t base;
	phys_addr_t size;
};

struct carveout_heap_buffer {
	struct carveout_heap *heap;
	struct list_head attachments;
	struct mutex lock;
	unsigned long len;
	struct sg_table sg_table;
	int vmap_cnt;
	void *vaddr;

	unsigned long phys_addr;
	unsigned long flags;
	bool uncached;

#ifdef CONFIG_VBM_CARVEOUT
	int alloc_zone_idx;
	int chunk_start_idx;
	int chunk_cnt;
	bool is_protected;
	bool is_occupied;
	unsigned long protect_addr;
	unsigned long protect_size;

	struct carveout_heap_list *list;
	struct list_head node;
	unsigned long resv1;
	unsigned long resv2;
#endif
};

struct carveout_heap_attachment {
	struct device *dev;
	struct sg_table *table;
	struct list_head list;
	bool mapped;

	bool uncached;
};

static int __init proc_carveout_heap_info_init(void);

#ifdef CONFIG_VBM_CARVEOUT
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

static inline unsigned long carveout_heap_pool_get_min_base(struct carveout_heap_pool *genpool)
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
static inline int carveout_heap_pool_addr_in_pool(struct carveout_heap_pool *genpool, unsigned long addr, unsigned long size,
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
		unsigned long pool_base = carveout_heap_pool_get_min_base(genpool);
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

static inline int carveout_heap_pool_new_or_growup(struct carveout_heap_pool *genpool, unsigned long addr, unsigned long size)
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
		unsigned long min_base = carveout_heap_pool_get_min_base(genpool);
		size_t prev_size = gen_pool_size(pool);

		if (carveout_heap_pool_addr_in_pool(genpool, addr, size, NULL, NULL) < 0) {
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

static inline void carveout_heap_pool_delete(struct carveout_heap_pool *genpool)
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

static inline unsigned long carveout_heap_pool_alloc(struct carveout_heap_pool *genpool, size_t size)
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

static void carveout_heap_pool_free(struct carveout_heap_pool *genpool, unsigned long addr, size_t size)
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

static size_t carveout_heap_pool_used(struct carveout_heap_pool *genpool)
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

static inline size_t carveout_heap_pool_size(struct carveout_heap_pool *genpool)
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

static inline int carveout_heap_pool_empty(struct carveout_heap_pool *genpool)
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

void carveout_heap_dump_chunk_list(void)
{
	int zone_idx, chunk_idx;

	mutex_lock(&chunk_list_mutex);
	for (zone_idx = 0; zone_idx < CHUNK_ZONE_COUNT; zone_idx++) {
		rtd_pr_rmm_info("[carveout_heap] zone %d:\n", zone_idx);
		for (chunk_idx = 0; chunk_idx < MAX_CHUNKS_PER_ZONE; chunk_idx++) {
			if (carveout_heap_chunk_list[zone_idx][chunk_idx].chunk_size > 0) {
				CARVEOUT_CHUNK_INFO *_chunk = NULL;
				struct carveout_heap_pool *reuse_genpool = NULL;
				struct gen_pool *pool = NULL;

				_chunk = &carveout_heap_chunk_list[zone_idx][chunk_idx];
				reuse_genpool = &_chunk->genpool;

				rtd_pr_rmm_info("    chunk %d: size = %ld MB, phys_addr = 0x%lx, used = %d, free = %ld MB\n",
					chunk_idx,
					(carveout_heap_chunk_list[zone_idx][chunk_idx].chunk_size)/__MB__,
					carveout_heap_chunk_list[zone_idx][chunk_idx].chunk_addr,
					carveout_heap_chunk_list[zone_idx][chunk_idx].used,
					(carveout_heap_chunk_list[zone_idx][chunk_idx].free_size)/__MB__);

				if (pool) {
					rtd_pr_rmm_info("    pool(%lx), size(%lx), used(%lx)\n", (unsigned long)reuse_genpool->pool,
							(unsigned long)carveout_heap_pool_size(reuse_genpool) ,(unsigned long)carveout_heap_pool_used(reuse_genpool));
				}
			}
		}
	}
	mutex_unlock(&chunk_list_mutex);
}

static void carveout_heap_list_reset_buffer_data(struct carveout_heap_list *list, 
					struct carveout_heap_list_priv_data *data)
{
	struct carveout_heap_buffer *buffer;
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
						carveout_heap_dump_chunk_list();
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

static inline void carveout_heap_list_set_priv_data(struct carveout_heap_list *list, 
					struct carveout_heap_list_priv_data *data)
{
	if (list) {
		mutex_lock(&list->lock);
		if (data)
			memcpy(&list->data, data, sizeof(struct carveout_heap_list_priv_data));
		mutex_unlock(&list->lock);
	}
	return;
}

/* return: first node of list */
static inline struct carveout_heap_buffer *carveout_heap_list_get_priv_data(struct carveout_heap_list *list, 
											struct carveout_heap_list_priv_data *data)
{
	if (list) {
		struct carveout_heap_buffer *buffer = NULL;

		mutex_lock(&list->lock);
		buffer = list_first_entry_or_null(&list->head, struct carveout_heap_buffer, node);
		if (data)
			memcpy(data, &list->data, sizeof(struct carveout_heap_list_priv_data));
		mutex_unlock(&list->lock);

		return buffer;
	} else 
		return NULL;
}

static inline int carveout_heap_list_empty(struct carveout_heap_buffer *buffer)
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

static inline void carveout_heap_list_init(struct carveout_heap_buffer *buffer)
{
	if (buffer->list != NULL) {
		rtd_pr_rmm_err("%s: error: list(%lx) already allocated, belong to chunk (%d/%d)\n", __func__,
			   (unsigned long)buffer->list, buffer->chunk_start_idx, buffer->chunk_cnt);
		BUG();
	}

	buffer->list = kmalloc(sizeof(struct carveout_heap_list), GFP_KERNEL);
	memset(buffer->list, 0, sizeof(struct carveout_heap_list));
	if (buffer->list) {
		mutex_init(&buffer->list->lock);
		INIT_LIST_HEAD(&buffer->list->head);
	}
}

static inline void carveout_heap_list_deinit(struct carveout_heap_buffer *buffer)
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

static inline void carveout_heap_list_add(struct carveout_heap_buffer *buffer)
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

static inline void carveout_heap_list_del(struct carveout_heap_buffer *buffer)
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

void carveout_heap_list_dump(struct carveout_heap_buffer *buffer)
{
	struct carveout_heap_buffer *buf_obj;

	if (buffer->list) {
		mutex_lock(&buffer->list->lock);
		list_for_each_entry(buf_obj, &buffer->list->head, node) {
			rtd_pr_rmm_debug("%s: dmabuf hndl(%lx) chunk(%d/%d) phys_addr = %lx, size=%lx\n", __func__,
				buf_obj->resv1, buf_obj->chunk_start_idx, buf_obj->chunk_cnt,
				buf_obj->phys_addr, buf_obj->len);
		}
		mutex_unlock(&buffer->list->lock);
	} else {
		rtd_pr_rmm_err("%s: no list\n", __func__);
	}
}

int carveout_heap_to_dmabuf_list(struct dma_buf *dmabuf, unsigned long buf_array[], int array_num)
{
	struct carveout_heap_buffer *buf_obj;
	struct carveout_heap_buffer *buffer = dmabuf->priv;
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

static struct sg_table *dup_sg_table(struct sg_table *table)
{
	struct sg_table *new_table;
	int ret, i;
	struct scatterlist *sg, *new_sg;

	new_table = kzalloc(sizeof(*new_table), GFP_KERNEL);
	if (!new_table)
		return ERR_PTR(-ENOMEM);

	ret = sg_alloc_table(new_table, table->orig_nents, GFP_KERNEL);
	if (ret) {
		kfree(new_table);
		return ERR_PTR(-ENOMEM);
	}

	new_sg = new_table->sgl;
	for_each_sgtable_sg(table, sg, i) {
		sg_set_page(new_sg, sg_page(sg), sg->length, sg->offset);
		new_sg = sg_next(new_sg);
	}

	return new_table;
}

static int carveout_heap_attach(struct dma_buf *dmabuf,
			      struct dma_buf_attachment *attachment)
{
	struct carveout_heap_buffer *buffer = dmabuf->priv;
	struct carveout_heap_attachment *a;
	struct sg_table *table;

	a = kzalloc(sizeof(*a), GFP_KERNEL);
	if (!a)
		return -ENOMEM;

	table = dup_sg_table(&buffer->sg_table);
	if (IS_ERR(table)) {
		kfree(a);
		return -ENOMEM;
	}

	a->table = table;
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

static void carveout_heap_detach(struct dma_buf *dmabuf,
			       struct dma_buf_attachment *attachment)
{
	struct carveout_heap_buffer *buffer = dmabuf->priv;
	struct carveout_heap_attachment *a = attachment->priv;

	mutex_lock(&buffer->lock);
	list_del(&a->list);
	mutex_unlock(&buffer->lock);

	sg_free_table(a->table);
	kfree(a->table);
	kfree(a);
}

static struct sg_table *carveout_heap_map_dma_buf(struct dma_buf_attachment *attachment,
						enum dma_data_direction direction)
{
	struct carveout_heap_attachment *a = attachment->priv;
	struct sg_table *table = a->table;
	int attr = 0;
	int ret;

	if (a->uncached)
		attr = DMA_ATTR_SKIP_CPU_SYNC;

	ret = dma_map_sgtable(attachment->dev, table, direction, attr);
	if (ret)
		return ERR_PTR(ret);

	a->mapped = true;
	return table;
}

static void carveout_heap_unmap_dma_buf(struct dma_buf_attachment *attachment,
				      struct sg_table *table,
				      enum dma_data_direction direction)
{
	struct carveout_heap_attachment *a = attachment->priv;
	int attr = 0;

	if (a->uncached)
		attr = DMA_ATTR_SKIP_CPU_SYNC;
	a->mapped = false;
	dma_unmap_sgtable(attachment->dev, table, direction, attr);
}

static int carveout_heap_dma_buf_begin_cpu_access(struct dma_buf *dmabuf,
						enum dma_data_direction direction)
{
	struct carveout_heap_buffer *buffer = dmabuf->priv;
	struct carveout_heap_attachment *a;

	mutex_lock(&buffer->lock);

	if (buffer->vmap_cnt)
		invalidate_kernel_vmap_range(buffer->vaddr, buffer->len);

	if (!buffer->uncached) {
		list_for_each_entry(a, &buffer->attachments, list) {
			if (!a->mapped)
				continue;
			dma_sync_sgtable_for_cpu(a->dev, a->table, direction);
		}
	}
	mutex_unlock(&buffer->lock);

	return 0;
}

static int carveout_heap_dma_buf_end_cpu_access(struct dma_buf *dmabuf,
					      enum dma_data_direction direction)
{
	struct carveout_heap_buffer *buffer = dmabuf->priv;
	struct carveout_heap_attachment *a;

	mutex_lock(&buffer->lock);

	if (buffer->vmap_cnt)
		flush_kernel_vmap_range(buffer->vaddr, buffer->len);

	if (!buffer->uncached) {
		list_for_each_entry(a, &buffer->attachments, list) {
			if (!a->mapped)
				continue;
			dma_sync_sgtable_for_device(a->dev, a->table, direction);
		}
	}
	mutex_unlock(&buffer->lock);

	return 0;
}

static int carveout_heap_mmap(struct dma_buf *dmabuf, struct vm_area_struct *vma)
{
	struct carveout_heap_buffer *buffer = dmabuf->priv;
	struct sg_table *table = &buffer->sg_table;
	unsigned long addr = vma->vm_start;
	struct sg_page_iter piter;
	int ret;

	if (buffer->uncached)
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);

	for_each_sgtable_page(table, &piter, vma->vm_pgoff) {
		struct page *page = sg_page_iter_page(&piter);

		ret = remap_pfn_range(vma, addr, page_to_pfn(page), PAGE_SIZE,
				      vma->vm_page_prot);
		if (ret)
			return ret;
		addr += PAGE_SIZE;
		if (addr >= vma->vm_end)
			return 0;
	}
	return 0;
}

static void *carveout_heap_do_vmap(struct carveout_heap_buffer *buffer)
{
	struct sg_table *table = &buffer->sg_table;
	int npages = PAGE_ALIGN(buffer->len) / PAGE_SIZE;
	struct page **pages = vmalloc(sizeof(struct page *) * npages);
	struct page **tmp = pages;
	struct sg_page_iter piter;
	pgprot_t pgprot = PAGE_KERNEL;
	void *vaddr;

	if (!pages)
		return ERR_PTR(-ENOMEM);

	if (buffer->uncached)
		pgprot = pgprot_writecombine(PAGE_KERNEL);

	for_each_sgtable_page(table, &piter, 0) {
		WARN_ON(tmp - pages >= npages);
		*tmp++ = sg_page_iter_page(&piter);
	}

	vaddr = vmap(pages, npages, VM_MAP, pgprot);
	vfree(pages);

	if (!vaddr)
		return ERR_PTR(-ENOMEM);

	return vaddr;
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 11, 0))

static int carveout_heap_vmap(struct dma_buf *dmabuf, struct dma_buf_map *map)
{
	struct carveout_heap_buffer *buffer = dmabuf->priv;
	void *vaddr;
	int ret = 0;

	mutex_lock(&buffer->lock);
	if (buffer->vmap_cnt) {
		buffer->vmap_cnt++;
		dma_buf_map_set_vaddr(map, buffer->vaddr);
		goto out;
	}

	vaddr = carveout_heap_do_vmap(buffer);
	if (IS_ERR(vaddr)) {
		ret = PTR_ERR(vaddr);
		goto out;
	}

	buffer->vaddr = vaddr;
	buffer->vmap_cnt++;
	dma_buf_map_set_vaddr(map, buffer->vaddr);
out:
	mutex_unlock(&buffer->lock);

	return ret;
}

static void carveout_heap_vunmap(struct dma_buf *dmabuf, struct dma_buf_map *map)
{
	struct carveout_heap_buffer *buffer = dmabuf->priv;

	mutex_lock(&buffer->lock);
	if (!--buffer->vmap_cnt) {
		vunmap(buffer->vaddr);
		buffer->vaddr = NULL;
	}
	mutex_unlock(&buffer->lock);
	dma_buf_map_clear(map);
}

#else

static void *carveout_heap_vmap(struct dma_buf *dmabuf)
{
	struct carveout_heap_buffer *buffer = dmabuf->priv;
	void *vaddr;

	mutex_lock(&buffer->lock);
	if (buffer->vmap_cnt) {
		buffer->vmap_cnt++;
		vaddr = buffer->vaddr;
		goto out;
	}

	vaddr = carveout_heap_do_vmap(buffer);
	if (IS_ERR(vaddr))
		goto out;

	buffer->vaddr = vaddr;
	buffer->vmap_cnt++;
out:
	mutex_unlock(&buffer->lock);

	return vaddr;
}

static void carveout_heap_vunmap(struct dma_buf *dmabuf, void *vaddr)
{
	struct carveout_heap_buffer *buffer = dmabuf->priv;

	mutex_lock(&buffer->lock);
	if (!--buffer->vmap_cnt) {
		vunmap(buffer->vaddr);
		buffer->vaddr = NULL;
	}
	mutex_unlock(&buffer->lock);
}

#endif

static void carveout_heap_dma_buf_release(struct dma_buf *dmabuf)
{
	struct carveout_heap_buffer *buffer = dmabuf->priv;
//	struct carveout_heap *carveout_heap = buffer->heap;
	struct sg_table *table;
#ifdef CONFIG_VBM_CARVEOUT
	int zone_idx;
	int chunk_idx;
	int ret = 0;
#endif
#ifdef CONFIG_CHUNK_REUSE
	CARVEOUT_CHUNK_INFO *_chunk = NULL;
	struct carveout_heap_list *reuse_list = NULL;
	struct carveout_heap_pool *reuse_genpool = NULL;
#endif

	if (!buffer) {
		rtd_pr_rmm_err("%s: ERROR!! No buffer in dmabuf %lx\n", __func__, (unsigned long)buffer);
		return;
	}

	rtd_pr_rmm_info("Release buffer %x, phy(0x%lx)\n", (unsigned long)buffer, (unsigned long)buffer->phys_addr);

	if (buffer->vmap_cnt > 0) {
		WARN(1, "%s: buffer still mapped in the kernel\n", __func__);
		vunmap(buffer->vaddr);
	}

	if (buffer->phys_addr == CARVEOUT_ALLOCATE_FAIL)
		BUG();

#ifdef CONFIG_VBM_CARVEOUT
	_chunk = &carveout_heap_chunk_list[buffer->alloc_zone_idx][buffer->chunk_start_idx];
	reuse_list = _chunk->list;
	reuse_genpool = &_chunk->genpool;
	if (buffer->resv2) {
		rtd_pr_rmm_info("pli_free: 0x%lx\n", buffer->phys_addr);
		pli_free_mesg(buffer->phys_addr);
	} else {
		carveout_heap_pool_free(reuse_genpool, buffer->phys_addr, buffer->len);
	}

	carveout_heap_list_del(buffer);
	ret = carveout_heap_list_empty(buffer);
	if (ret > 0) {
		CARVEOUT_CHUNK_INFO chunk_info[MAX_CHUNKS_PER_ZONE] = {0};
		unsigned long remain_pool_size = carveout_heap_pool_size(reuse_genpool);

		if (reuse_list && ((&reuse_list->data)->chunk_cnt != buffer->chunk_cnt)) {
			rtd_pr_rmm_info("reuse_list chunk_cnt(%d/%d)\n", (&reuse_list->data)->chunk_cnt, buffer->chunk_cnt);
 		}

		rtd_pr_rmm_info("%s: Free phys_addr from CARVEOUT heap 0x%lx (zone[%d]chunk[%d-%d])\n", __func__,
			buffer->phys_addr, buffer->alloc_zone_idx, 
				buffer->chunk_start_idx, (buffer->chunk_start_idx + buffer->chunk_cnt - 1));
		rtd_pr_rmm_info("	 (protected = %d, protect_addr = 0x%lx, protect_size = 0x%lx)\n",
			buffer->is_protected, buffer->protect_addr, buffer->protect_size);

		zone_idx = buffer->alloc_zone_idx;

		mutex_lock(&chunk_list_mutex);
		for (chunk_idx = buffer->chunk_start_idx; chunk_idx < (buffer->chunk_start_idx + buffer->chunk_cnt); chunk_idx++) {
			chunk_info[chunk_idx].used = carveout_heap_chunk_list[zone_idx][chunk_idx].used;
			chunk_info[chunk_idx].chunk_addr = carveout_heap_chunk_list[zone_idx][chunk_idx].chunk_addr;
			chunk_info[chunk_idx].chunk_size = carveout_heap_chunk_list[zone_idx][chunk_idx].chunk_size;
			chunk_info[chunk_idx].free_size = carveout_heap_chunk_list[zone_idx][chunk_idx].free_size;
		}
		mutex_unlock(&chunk_list_mutex);

		if (buffer->is_protected) {
			if (rtkvdec_svp_disable_protection(buffer->protect_addr, buffer->protect_size, TYPE_SVP_PROTECT_VBM) != 0) {
				rtd_pr_rmm_err("Error! Disable protection failed for addr 0x%lx, size 0x%lx",
					buffer->protect_addr, buffer->protect_size);
				carveout_heap_dump_chunk_list();
				BUG();
			}
		}

		for (chunk_idx = buffer->chunk_start_idx; chunk_idx < (buffer->chunk_start_idx + buffer->chunk_cnt); chunk_idx++)
		{
			if (chunk_info[chunk_idx].used) {
				unsigned long size;
				size = chunk_info[chunk_idx].chunk_size - chunk_info[chunk_idx].free_size;

				if (chunk_info[chunk_idx].free_size != 0) {
					if (remain_pool_size >= chunk_info[chunk_idx].chunk_size)
						size = chunk_info[chunk_idx].chunk_size;
					else if (remain_pool_size > size)
						size = remain_pool_size;
				}

				rtd_pr_rmm_info("free zone[%d] chunk[%d], size(%lx) successfully\n", zone_idx, chunk_idx, size);

				if (remain_pool_size)
					remain_pool_size -= (size << PAGE_SHIFT);
			} else {
				rtd_pr_rmm_err("%s: ERROR!! Release unused zone[%d] chunk[%d], addr 0x%lx, size 0x%lx\n", __func__,
					zone_idx, chunk_idx, chunk_info[chunk_idx].chunk_addr, chunk_info[chunk_idx].chunk_size);
				carveout_heap_dump_chunk_list();
				BUG();
			}
 		}
		carveout_heap_pool_delete(reuse_genpool);

		mutex_lock(&chunk_list_mutex);
		for (chunk_idx = buffer->chunk_start_idx; chunk_idx < (buffer->chunk_start_idx + buffer->chunk_cnt); chunk_idx++)
		{
			carveout_heap_chunk_list[zone_idx][chunk_idx].used = 0;
			carveout_heap_chunk_list[zone_idx][chunk_idx].free_size = carveout_heap_chunk_list[zone_idx][chunk_idx].chunk_size;
#ifdef CONFIG_CHUNK_REUSE
			carveout_heap_chunk_list[zone_idx][chunk_idx].list = NULL;
			(&carveout_heap_chunk_list[zone_idx][chunk_idx].genpool)->pool = NULL;
			(&carveout_heap_chunk_list[zone_idx][chunk_idx].genpool)->growup = false;
#endif
		}
		mutex_unlock(&chunk_list_mutex);

		carveout_heap_list_deinit(buffer);
	} else if (ret == 0) {
		buffer->list = NULL;
	} else {
		rtd_pr_rmm_err("%s: no buffer list, ret=%d\n", __func__, ret);
 	}
#else
	gen_pool_free(carveout_heap->pool, buffer->phys_addr, buffer->len);
#endif

	table = &buffer->sg_table;
	sg_free_table(table);
	kfree(buffer);
}

static const struct dma_buf_ops carveout_heap_buf_ops = {
	.attach = carveout_heap_attach,
	.detach = carveout_heap_detach,
	.map_dma_buf = carveout_heap_map_dma_buf,
	.unmap_dma_buf = carveout_heap_unmap_dma_buf,
	.begin_cpu_access = carveout_heap_dma_buf_begin_cpu_access,
	.end_cpu_access = carveout_heap_dma_buf_end_cpu_access,
	.mmap = carveout_heap_mmap,
	.vmap = carveout_heap_vmap,
	.vunmap = carveout_heap_vunmap,
	.release = carveout_heap_dma_buf_release,
};

unsigned long carveout_heap_to_phys(struct dma_buf *dmabuf)
{
	struct carveout_heap_buffer *buffer;
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

static struct dma_buf *carveout_heap_chunk_allocate_frame_single(struct dma_heap *heap,
					 unsigned long offset, unsigned long size,
					 unsigned long fd_flags,
					 unsigned long heap_flags)
{
	struct carveout_heap *carveout_heap = dma_heap_get_drvdata(heap);
	struct carveout_heap_buffer *buffer;
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	struct dma_buf *dmabuf;
	struct sg_table *table;
	int ret = -ENOMEM;

	BUG_ON(!PAGE_ALIGNED(size));

	rtd_pr_rmm_debug("%s: Request buffer allocation len 0x%lx\n", __func__, size);

	buffer = kzalloc(sizeof(*buffer), GFP_KERNEL);
	if (!buffer) {
		rtd_pr_rmm_err("Can't allocate buffer info\n");
		return ERR_PTR(-ENOMEM);
	}

	INIT_LIST_HEAD(&buffer->attachments);
	mutex_init(&buffer->lock);
	buffer->heap = carveout_heap;
	buffer->len = size;
	buffer->flags = heap_flags;
	buffer->uncached = true;  // default

	table = &buffer->sg_table;
	if (sg_alloc_table(table, 1, GFP_KERNEL)) {
		rtd_pr_rmm_err("sg_alloc_table fail\n");
		goto free_buffer;
	}

	buffer->phys_addr = offset;
	if (buffer->phys_addr == INVALID_VAL) {
		buffer->phys_addr = pli_malloc_mesg(size, GFP_DCU2_FIRST, "DMABUF_CARVEOUT:08:");
		buffer->resv2 = 1;
	}

	sg_set_page(table->sgl, pfn_to_page(PFN_DOWN(buffer->phys_addr)), buffer->len, 0);

	/* create the dmabuf */
	exp_info.ops = &carveout_heap_buf_ops;
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
	sg_free_table(table);
free_buffer:
	kfree(buffer);

	return ERR_PTR(ret);
}

#define SEEK_CHUNKS_STATE_OK					0
#define SEEK_CHUNKS_STATE_ERROR					1
#define SEEK_CHUNKS_STATE_REUSE_CHUNK_WAIT		2

/* Return state */
static int carveout_heap_seek_chunks(struct carveout_chunk_array_info *info, struct carveout_heap_pool *reuse_genpool, 
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
			if (carveout_heap_chunk_list[info->alloc_zone_index][c_idx].used) {
#ifdef CONFIG_CHUNK_REUSE
				CARVEOUT_CHUNK_INFO *_chunk = NULL;
				struct carveout_heap_list_priv_data data[1] = {0};
				struct carveout_heap_buffer *buffer;

				_chunk = &carveout_heap_chunk_list[info->alloc_zone_index][c_idx];
				buffer = carveout_heap_list_get_priv_data(_chunk->list, data);

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
							new_size = size + ((data->cnt_remain << 1) * frame_size);
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

			info->alloc_addr_array[c_idx] = carveout_heap_chunk_list[info->alloc_zone_index][c_idx].chunk_addr;
			info->alloc_size_array[c_idx] = carveout_heap_chunk_list[info->alloc_zone_index][c_idx].chunk_size;

			chunk_available_size += carveout_heap_chunk_list[info->alloc_zone_index][c_idx].chunk_size;

			if (chunk_available_size >= new_size) {
				unsigned long last_chunk_unused_size = chunk_available_size - new_size;

				info->chunk_end_index = c_idx;
				find_chunks = true;
				info->alloc_size_array[c_idx] -= last_chunk_unused_size;

				if (prev_source_chunk_end_index != -1) {
					if (info->chunk_end_index == prev_source_chunk_end_index) {
						unsigned long prev_source_chunk_end_alloc_size = 
							carveout_heap_chunk_list[info->alloc_zone_index][c_idx].chunk_size - 
							carveout_heap_chunk_list[info->alloc_zone_index][c_idx].free_size;

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
						carveout_heap_chunk_list[info->alloc_zone_index][c_idx].chunk_size - 
						carveout_heap_chunk_list[info->alloc_zone_index][c_idx].free_size;

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
				info->flags_array[c_idx] |= CARVEOUT_CHUNK_FLAGS_BUDDY_ALLOCATED;
				carveout_heap_chunk_list[info->alloc_zone_index][c_idx].used = 1;
				carveout_heap_chunk_list[info->alloc_zone_index][c_idx].free_size = 
					carveout_heap_chunk_list[info->alloc_zone_index][c_idx].chunk_size - info->alloc_size_array[c_idx];

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
		rtd_pr_rmm_err("[vbm_carveout] No free space for VBM CARVEOUT allocation find?(%d) zidx=%d cidx=%d, size = 0x%lx\n",
			   find_chunks, z_idx, c_idx, new_size);
		carveout_heap_dump_chunk_list();
		ret = SEEK_CHUNKS_STATE_ERROR;
	}

	return ret;

seek_out:
	mutex_unlock(&chunk_list_mutex);
	return ret;
}

/* Return phys offset of first chunk */
static unsigned long carveout_heap_alloc_chunks(struct carveout_chunk_array_info *info, struct carveout_heap_pool *reuse_genpool)
{
	int c_idx;
	char type[8] = {'\0'};
	unsigned long offset = INVALID_VAL;

	// we will use alloc_zone_index and chunk_start_index/chunk_end_index to alloc cma
	for (c_idx = info->chunk_start_index; c_idx <= info->chunk_end_index; c_idx++)
 	{
 		unsigned long carveout_chunk_phys_addr = INVALID_VAL;
		unsigned long alloc_addr = info->alloc_addr_array[c_idx];
		unsigned long alloc_size = info->alloc_size_array[c_idx];
 
 		/* Allocate chunk memory by specific size and address */

 		/* Exam if allocated chunk address is matched to chunk list */
		if ((info->flags_array[c_idx] & CARVEOUT_CHUNK_FLAGS_BUDDY_ALLOCATED) && !info->growup_size_array[c_idx])
			strncpy(type, "(resv)", 6);
		else if ((info->flags_array[c_idx] & CARVEOUT_CHUNK_FLAGS_BUDDY_ALLOCATED) && info->growup_size_array[c_idx]) {
			strncpy(type, "(grow)", 6);
			alloc_addr += (info->alloc_size_array[c_idx] - info->growup_size_array[c_idx]); // prev_source_chunk_end_alloc_size
			alloc_size = info->growup_size_array[c_idx];
		}

 		carveout_chunk_phys_addr = alloc_addr;
		rtd_pr_rmm_info("[vbm_carveout] %sAlloc phys_addr from CARVEOUT heap zone[%d] chunk[%d] is 0x%lx, size is 0x%lx\n", 
				type, info->alloc_zone_index, c_idx, carveout_chunk_phys_addr, alloc_size);

 		/* Set the chunk as used */
		info->flags_array[c_idx] |= CARVEOUT_CHUNK_FLAGS_USED;
 
 		/* Record the first allocated chunk page as vbm_pages */
		if (c_idx == info->chunk_start_index)
 		{
 			offset = info->alloc_addr_array[c_idx]; // return the chunk start address
 		}
 	}

	return offset;
}

static struct dma_buf *carveout_heap_alloc_frames(struct dma_heap *heap, struct carveout_chunk_array_info *info, 
						struct carveout_heap_pool *reuse_genpool, bool need_protect,
						size_t size, unsigned int frame_count, unsigned long fd_flags, unsigned long heap_flags)
{
	int i = 0, frame_idx = 0;
	int occupied_frame = 0;
	unsigned int frame_size = size / frame_count;
	unsigned long base_offset = 0;
	struct dma_buf *dmabuf;
	struct dma_buf *first_dmabuf;
	struct carveout_heap_buffer *first_buffer = NULL;
	struct carveout_heap_buffer *buffer = NULL;
	struct carveout_heap_list_priv_data data[1] = {0};

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
		struct carveout_heap_list_priv_data list_data[1] = {0};
		first_buffer = carveout_heap_list_get_priv_data(info->reuse_list, list_data);
		if (!first_buffer) {
			rtd_pr_rmm_err("(reuse):%d no buffer node? (snoop without lock) list of cnt_total(%d) cnt_remain(%d)\n", 
				   __LINE__,  list_data->cnt_total, list_data->cnt_remain);
			BUG();
		}
		occupied_frame = list_data->cnt_remain;

		data->cnt_total = data->cnt_remain = occupied_frame + frame_count;
		data->is4K = (heap_flags & DMA_HEAP_FLAG_USER_VIDEO_4K_LUMA || 
					 heap_flags & DMA_HEAP_FLAG_USER_VIDEO_4K_CHROMA) ? true : false;
		carveout_heap_list_reset_buffer_data(info->reuse_list, data);
		carveout_heap_list_set_priv_data(info->reuse_list, data);
	}

	rtd_pr_rmm_info("%s: alloc frame from %d(occupied) to %d\n", __func__, occupied_frame, frame_count + occupied_frame);

	// for each for current allocation
	for (i = occupied_frame; i < (frame_count + occupied_frame); i++) { // each single frame

		base_offset = carveout_heap_pool_alloc(reuse_genpool, frame_size);
		if (base_offset == INVALID_VAL) {
			rtd_pr_rmm_err("frame[%d] pool alloc failure, pool used(%lx), size(%lx)\n", i, 
						   carveout_heap_pool_used(reuse_genpool), carveout_heap_pool_size(reuse_genpool));
//			BUG();
//			return -ENOMEM;
		}
		dmabuf = carveout_heap_chunk_allocate_frame_single(heap, base_offset, frame_size, fd_flags, heap_flags);
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

		rtd_pr_rmm_debug("vbm_carveout buffer len = %ld, phys_addr = 0x%lx, zone idx = %d, chunk start idx = %d, chunk count = %d\n",
				 buffer->len, buffer->phys_addr, buffer->alloc_zone_idx, buffer->chunk_start_idx, buffer->chunk_cnt);
		rtd_pr_rmm_debug("    is_protected = %d, protect_addr = 0x%lx, protect_size = 0x%lx\n",
				 buffer->is_protected, buffer->protect_addr, buffer->protect_size);

		if (frame_idx == 0) { // first frame for current allocation
			first_dmabuf = dmabuf;

			if (first_buffer == NULL) { // no reuse
				data->cnt_total = data->cnt_remain = frame_count;
				data->is4K = (heap_flags & DMA_HEAP_FLAG_USER_VIDEO_4K_LUMA || 
							 heap_flags & DMA_HEAP_FLAG_USER_VIDEO_4K_CHROMA) ? true : false;

				first_buffer = buffer;
				carveout_heap_list_init(first_buffer);
				carveout_heap_list_set_priv_data(buffer->list, data);
			} else {
				// list private data already re-apply at the beginning
				buffer->list = first_buffer->list;
			}

			rtd_pr_rmm_debug("%s:%d: first buffer list(%lx), reuse(%lx)\n", __func__, __LINE__, buffer->list, info->reuse_list);
		} else {
			buffer->list = first_buffer->list;
		}

		carveout_heap_list_add(buffer);
		frame_idx ++;
	}

	// debug
	carveout_heap_list_dump(first_dmabuf->priv);

	return first_dmabuf;
}

static struct dma_buf *carveout_heap_allocate(struct dma_heap *heap,
					 unsigned long len,
					 unsigned long fd_flags,
					 unsigned long heap_flags)
{
#ifndef CONFIG_VBM_CARVEOUT
	struct carveout_heap *carveout_heap = dma_heap_get_drvdata(heap);
#endif
	struct carveout_heap_buffer *buffer = NULL;
	size_t size = PAGE_ALIGN(len);
#ifndef CONFIG_VBM_CARVEOUT
	struct dma_buf *dmabuf;
	struct sg_table *table;
#endif
	unsigned long offset;
	int ret = -ENOMEM;

#ifdef CONFIG_VBM_CARVEOUT
	int zone_start_idx;
	int c_idx;
	int chunk_seek_state = 0;
	unsigned int frame_count = 0;
	struct dma_buf *first_dmabuf;

	bool need_protect = false;
#ifdef CONFIG_CHUNK_REUSE
	struct carveout_heap_pool reuse_genpool[] = {{ .pool = NULL, .growup = false }};
#endif

	struct carveout_chunk_array_info info[] = {0};

	rtd_pr_rmm_info("Request buffer allocation len 0x%lx\n", len);

	memset(info, 0, sizeof(struct carveout_chunk_array_info));

	// find big chunk
	/* Decide zone start index*/
	if (!(heap_flags & DMA_HEAP_FLAG_USER_MASK)) {
		rtd_pr_rmm_err("Invalid zone argument of heap_flags 0x%lx\n", heap_flags);
		goto free_buffer;
	} else {
		if (heap_flags & DMA_HEAP_FLAG_USER_VIDEO_4K_LUMA)
			zone_start_idx = CHUNK_ZONE_4K_LUMA;
		else if (heap_flags & DMA_HEAP_FLAG_USER_COBUFFER)
			zone_start_idx = CHUNK_ZONE_COBUFFER;
		else
			zone_start_idx = CHUNK_ZONE_OTHER;
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
	chunk_seek_state = carveout_heap_seek_chunks(info, reuse_genpool, zone_start_idx, size, frame_count, heap_flags);
	if (chunk_seek_state == SEEK_CHUNKS_STATE_REUSE_CHUNK_WAIT)
		goto wait_for_reuse_pool;
	else if (chunk_seek_state == SEEK_CHUNKS_STATE_ERROR)
		goto free_buffer;

	/* Seek chunks OK */
	rtd_pr_rmm_info("alloc_zone_index = %d, chunk_start_index = %d, chunk_end_index = %d\n",
			info->alloc_zone_index, info->chunk_start_index, info->chunk_end_index);

	/* Allocate the occupied chunks from seek */
	offset = carveout_heap_alloc_chunks(info, reuse_genpool);
	if (offset == INVALID_VAL) {
		rtd_pr_rmm_err("alloc chunks assert: offset invalid\n");
		BUG();
	}
#ifdef CONFIG_CHUNK_REUSE
	if (carveout_heap_pool_new_or_growup(reuse_genpool, offset, size + info->new_size_delta) < 0) {
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
	first_dmabuf = carveout_heap_alloc_frames(heap, info, reuse_genpool, need_protect, size, frame_count, fd_flags, heap_flags);

#ifdef CONFIG_CHUNK_REUSE
	mutex_lock(&chunk_list_mutex);
	for (c_idx = info->chunk_start_index; c_idx <= info->chunk_end_index; c_idx++) {
		struct carveout_heap_buffer *first_buffer = first_dmabuf->priv;
		CARVEOUT_CHUNK_INFO *_chunk = NULL;

		_chunk = &carveout_heap_chunk_list[info->alloc_zone_index][c_idx];

		// duplicated needed information
		_chunk->list = first_buffer->list;
		(&_chunk->genpool)->pool = reuse_genpool->pool;
		(&_chunk->genpool)->growup = reuse_genpool->growup;
	}
	mutex_unlock(&chunk_list_mutex);
#endif

	return first_dmabuf;
#else
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);

	rtd_pr_rmm_info("Request buffer allocation len %ld\n", len);

	buffer = kzalloc(sizeof(*buffer), GFP_KERNEL);
	if (!buffer) {
		rtd_pr_rmm_err("Can't allocate buffer info\n");
		return ERR_PTR(-ENOMEM);
	}

	INIT_LIST_HEAD(&buffer->attachments);
	mutex_init(&buffer->lock);
	buffer->heap = carveout_heap;
	buffer->len = len;
	buffer->flags = heap_flags;
	buffer->uncached = true;  // default

	table = &buffer->sg_table;
	if (sg_alloc_table(table, 1, GFP_KERNEL)) {
		rtd_pr_rmm_err("sg_alloc_table fail\n");
		goto free_buffer;
	}

	offset = gen_pool_alloc(carveout_heap->pool, len);
	if (!offset) {
		buffer->phys_addr = CARVEOUT_ALLOCATE_FAIL;
		rtd_pr_rmm_err("pool alloc fail\n");
		goto free_pages;
	}

	buffer->phys_addr = offset;
	sg_set_page(table->sgl, pfn_to_page(PFN_DOWN(buffer->phys_addr)), len, 0);

	/* create the dmabuf */
	exp_info.ops = &carveout_heap_buf_ops;
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
#endif

 free_vbm:
	rtd_pr_rmm_err("%s free_carveout\n", __func__);
#ifdef CONFIG_VBM_CARVEOUT
	/*  free pages from specific zone's chunk start index to chunk end index */
	for (c_idx = info->chunk_start_index; c_idx <= info->chunk_end_index; c_idx++)
	{
		if (info->flags_array[c_idx] & CARVEOUT_CHUNK_FLAGS_USED) {
			rtd_pr_rmm_info("free zone[%d] chunk[%d] successfully\n", info->alloc_zone_index, c_idx);
		}
	}

	mutex_lock(&chunk_list_mutex);
	for (c_idx = info->chunk_start_index; c_idx <= info->chunk_end_index; c_idx++) {
		carveout_heap_chunk_list[info->alloc_zone_index][c_idx].used = 0;
		carveout_heap_chunk_list[info->alloc_zone_index][c_idx].free_size = 
			carveout_heap_chunk_list[info->alloc_zone_index][c_idx].chunk_size;
	}
	mutex_unlock(&chunk_list_mutex);
#else
free_pages:
	sg_free_table(table);
#endif
free_buffer:
	if (buffer)
		kfree(buffer);

	return ERR_PTR(ret);
}

static const struct dma_heap_ops carveout_heap_ops = {
	.allocate = carveout_heap_allocate,
};

#ifdef CONFIG_VBM_CARVEOUT
static unsigned long * carveout_heap_get_allocator(void)
{
	if (get_display_resolution() == DISPLAY_RESOLUTION_8K)
		return (unsigned long *)&CARVEOUT_HEAP_8K_DISP_CHUNK_SIZE;
	else
		return (unsigned long *)&CARVEOUT_HEAP_8K_CHUNK_SIZE;
}

void carveout_heap_cma_init(struct carveout_heap *carveout_heap)
{
	unsigned long total_chunk_size = 0;
	unsigned long *p = NULL;
	unsigned long (*allocator)[MAX_CHUNKS_PER_ZONE];

	int i, j;

	mutex_lock(&chunk_list_mutex);
	p = carveout_heap_get_allocator();
	allocator = (unsigned long (*)[MAX_CHUNKS_PER_ZONE])p;

	for (i = 0; i < CHUNK_ZONE_COUNT; i++) {
		for (j = 0; j < MAX_CHUNKS_PER_ZONE; j++) {
			carveout_heap_chunk_list[i][j].chunk_addr = carveout_heap->base + total_chunk_size;
			carveout_heap_chunk_list[i][j].free_size = carveout_heap_chunk_list[i][j].chunk_size = allocator[i][j];
			carveout_heap_chunk_list[i][j].used = 0;
#ifdef CONFIG_CHUNK_REUSE
			carveout_heap_chunk_list[i][j].list = NULL;
#endif

			total_chunk_size += carveout_heap_chunk_list[i][j].chunk_size;
		}
	}
	mutex_unlock(&chunk_list_mutex);

	if (total_chunk_size > carveout_heap->size) {
		rtd_pr_rmm_err("ERROR, total_chunk_size %ld MB > vbm_size %lld MB\n", total_chunk_size/__MB__, carveout_heap->size/__MB__);
		return;
	}

	carveout_heap_dump_chunk_list();
}
#endif

extern void carveout_heap_to_phys_register(unsigned long (*fp)(struct dma_buf *dmabuf));
extern void carveout_heap_to_dmabuf_list_register(int (*fp)(struct dma_buf *dmabuf, unsigned long buf_array[], int array_num));
static int carveout_heap_create(void)
{
	struct carveout_heap *carveout_heap = NULL;
	struct dma_heap_export_info exp_info;
	unsigned long size = 0, base = 0;
#ifndef CONFIG_VBM_CARVEOUT
	int error;
#endif
	carveout_heap = kzalloc(sizeof(*carveout_heap), GFP_KERNEL);
	if (!carveout_heap) {
		rtd_pr_rmm_err("no heap\n");
		return -ENOMEM;
	}

#ifdef CONFIG_VBM_CARVEOUT
	size = (unsigned int)carvedout_buf_query(CARVEDOUT_VDEC_VBM, (void *)&base) ;	
	if (!size) {
		kfree(carveout_heap);
		rtd_pr_rmm_err("no carvedout space(%lx)\n", size);
		return -ENOMEM;
	}
#else
	carveout_heap->pool = gen_pool_create(PAGE_SHIFT, -1);
	if (!carveout_heap->pool) {
		kfree(carveout_heap);
		rtd_pr_rmm_err("no pool(%lx)\n", (unsigned long)carveout_heap->pool);
		return -ENOMEM;
	}
#endif
	size = ALIGN(size, 1*_MB_);
	carveout_heap->base = base;
	carveout_heap->size = size;

#ifndef CONFIG_VBM_CARVEOUT
	error = gen_pool_add(carveout_heap->pool, carveout_heap->base, carveout_heap->size,
		     -1);
	if (error < 0) {
		rtd_pr_rmm_err("can not add to pool\n");
		gen_pool_destroy(carveout_heap->pool);
		kfree(carveout_heap);
		return -ENOMEM;
	}
#endif

	exp_info.name = "carveout";
	exp_info.ops = &carveout_heap_ops;
	exp_info.priv = carveout_heap;

	carveout_heap->heap = dma_heap_add(&exp_info);
	if (IS_ERR(carveout_heap->heap)) {
		int ret = PTR_ERR(carveout_heap->heap);

		gen_pool_destroy(carveout_heap->pool);
		kfree(carveout_heap);
		return ret;
	}

#ifdef CONFIG_VBM_CARVEOUT
	rtd_pr_rmm_info("[vbm_carveout] VBM CARVEOUT initialized\n");
	carveout_heap_cma_init(carveout_heap);
	proc_carveout_heap_info_init();
#endif

	rtd_pr_rmm_info("add carveout_heap base = 0x%lx, size = 0x%lx\n", (unsigned long)carveout_heap->base, (unsigned long)carveout_heap->size);

	carveout_heap_to_phys_register(carveout_heap_to_phys);
	carveout_heap_to_dmabuf_list_register(carveout_heap_to_dmabuf_list);

	return 0;
}
module_init(carveout_heap_create);

#ifdef CONFIG_VBM_CARVEOUT
int carveout_heap_info_proc_show(struct seq_file *m, void *v)
{
	int zone_idx, chunk_idx;

	mutex_lock(&chunk_list_mutex);
	for (zone_idx = 0; zone_idx < CHUNK_ZONE_COUNT; zone_idx++) {
		seq_printf(m, "[carveout_heap] zone %d:\n", zone_idx);
		for (chunk_idx = 0; chunk_idx < MAX_CHUNKS_PER_ZONE; chunk_idx++) {
			if (carveout_heap_chunk_list[zone_idx][chunk_idx].chunk_size > 0) {
				CARVEOUT_CHUNK_INFO *_chunk = NULL;
				struct carveout_heap_pool *reuse_genpool = NULL;
				struct gen_pool *pool = NULL;

				_chunk = &carveout_heap_chunk_list[zone_idx][chunk_idx];
				reuse_genpool = &_chunk->genpool;

				seq_printf(m, "    chunk %d: size = %ld MB, phys_addr = 0x%lx, used = %d, free = %ld MB\n",
					chunk_idx,
					(carveout_heap_chunk_list[zone_idx][chunk_idx].chunk_size)/__MB__,
					carveout_heap_chunk_list[zone_idx][chunk_idx].chunk_addr,
					carveout_heap_chunk_list[zone_idx][chunk_idx].used,
					(carveout_heap_chunk_list[zone_idx][chunk_idx].free_size)/__MB__);

				if (pool) {
					seq_printf(m, "    pool(%lx), size(%lx), used(%lx)\n", (unsigned long)reuse_genpool->pool,
							   (unsigned long)carveout_heap_pool_size(reuse_genpool) ,(unsigned long)carveout_heap_pool_used(reuse_genpool));
				}
			}
		}
	}
	mutex_unlock(&chunk_list_mutex);

	return 0;
}

static int carveout_heap_info_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, carveout_heap_info_proc_show, NULL);
}


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static const struct proc_ops carveout_heap_info_proc_fops = {
	.proc_open		= carveout_heap_info_proc_open,
	.proc_read		= seq_read,
	.proc_lseek		= seq_lseek,
	.proc_release	= single_release,
};
#else
static const struct file_operations carveout_heap_info_proc_fops = {
	.open		= carveout_heap_info_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif

static int __init proc_carveout_heap_info_init(void)
{
	proc_create("carveout_heap_info", 0, NULL, &carveout_heap_info_proc_fops);

	return 0;
}
#endif

MODULE_LICENSE("GPL v2");