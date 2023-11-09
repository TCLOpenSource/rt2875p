
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/genalloc.h>

#include <rtd_log/rtd_module_log.h>

struct genpool_resize_info {
	struct gen_pool_chunk *resize_chunk;
	size_t used;
	int count;
	bool copy_done;
};

static void __maybe_unused chunk_debug_show_areas(struct gen_pool *pool, struct gen_pool_chunk *chunk)
{
	unsigned long next_zero_bit, next_set_bit, nr_zero;
	unsigned long start = 0;
	unsigned long nr_part, nr_total = 0;
	int order = pool->min_alloc_order;
	size_t size = chunk->end_addr - chunk->start_addr + 1;
	unsigned long nbits = size >> order;

	rtd_pr_rmm_debug("number of available pages: ");
	for (;;) {
		next_zero_bit = find_next_zero_bit(chunk->bits, nbits, start);
		if (next_zero_bit >= nbits)
			break;
		next_set_bit = find_next_bit(chunk->bits, nbits, next_zero_bit);
		nr_zero = next_set_bit - next_zero_bit;
		nr_part = nr_zero << order;
		rtd_pr_rmm_debug("%s%lu@%lu", nr_total ? "+" : "", nr_part,
			next_zero_bit);
		nr_total += nr_part;
		start = next_zero_bit + nr_zero;
	}
	rtd_pr_rmm_debug("=> %lu free of %lu total pages\n", nr_total, size);
}

static void __gen_pool_chunk_resize(struct gen_pool *pool, struct gen_pool_chunk *chunk, void *data)
{
	struct genpool_resize_info *info = NULL;
	size_t size = chunk->end_addr - chunk->start_addr + 1;

	if (data) {
		info = (struct genpool_resize_info *)(*(unsigned long *)data);
	}

	/* only accept one chunk inside a genpool list */
	if (info && (info->count == 0) && (size != 0)) {
		unsigned long nbits = size >> pool->min_alloc_order;
		unsigned long nbytes = sizeof(struct gen_pool_chunk) +
			BITS_TO_LONGS(nbits) * sizeof(long);

//		chunk_debug_show_areas(pool, chunk);

		memcpy(info->resize_chunk, chunk, nbytes);
		info->copy_done = true;

		chunk->phys_addr = -1;
		chunk->start_addr = 1; // for chunk_size()
		chunk->end_addr = 0;   // for chunk_size()
		chunk->owner = NULL;
		info->used = size - atomic_long_read(&chunk->avail);
		atomic_long_set(&chunk->avail, 0);
	}

	if (info)
		info->count ++;
}

int __gen_pool_growup(struct gen_pool *pool, unsigned long addr, size_t size, int nid)
{
	struct gen_pool_chunk *chunk;
	struct genpool_resize_info info;
	unsigned long data = (unsigned long)&info;
	unsigned long nbits = size >> pool->min_alloc_order;
	unsigned long nbytes = sizeof(struct gen_pool_chunk) +
				BITS_TO_LONGS(nbits) * sizeof(long);

	chunk = vzalloc_node(nbytes, nid);
	if (unlikely(chunk == NULL))
		return -ENOMEM;

	info.resize_chunk = chunk;
	info.used = 0;
	info.count = 0;
	info.copy_done = false;
	gen_pool_for_each_chunk(pool, __gen_pool_chunk_resize, &data);

	if (info.copy_done == false) {
		vfree(info.resize_chunk);
		return -ENOMEM;
	}

	// over write new setting
	chunk->phys_addr = -1;
	chunk->start_addr = addr;
	chunk->end_addr = addr + size - 1;
	chunk->owner = NULL;
	atomic_long_set(&chunk->avail, size - info.used);

	spin_lock(&pool->lock);
	list_add_rcu(&chunk->next_chunk, &pool->chunks);
	spin_unlock(&pool->lock);

	rtd_pr_rmm_info("new chunk (%lx/%lx/%lx) avail(%lx) pool_avail(%lx), chunk cnt(%d)\n", 
					chunk->start_addr, gen_pool_size(pool), chunk->end_addr+1, 
					atomic_long_read(&chunk->avail), gen_pool_avail(pool), 
					info.count);
//	chunk_debug_show_areas(pool, chunk);

	return 0;
}
