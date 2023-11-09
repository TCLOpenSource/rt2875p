#ifndef _LINUX_RTKBLUEPRINT_H
#define _LINUX_RTKBLUEPRINT_H

#include <linux/list.h>
#include <linux/memblock.h>

#ifdef __KERNEL__

#define RTK_BP_MAX_ORDER	16
#define RTK_BUDDY_FLAG		0x80000000
#define RTK_BUDDY_MASK		0x7fffffff

#define BAN_ALL_TYPE		0xffffffff
#define BAN_NORMAL		1
#define BAN_NOT_USED		2

struct mem_area {
	struct list_head	free_list;
	unsigned long		nr_free;
};

struct mem_unit {
	struct list_head	lru;
	unsigned long		order;
};

struct mem_bp {
	struct mem_area		rtk_free_area[RTK_BP_MAX_ORDER];
	struct mem_unit		*rtk_mem_map;
	struct mem_unit		*max_mem_map;
	unsigned long		base_pfn;
	unsigned long		avail_size;
};

#ifdef CONFIG_CMA_RTK_ALLOCATOR

int add_ban_info(unsigned long start, unsigned long size, int type);
bool is_banned(unsigned long pfn, int mask);
void init_rtkbp(struct mem_bp *bp, unsigned long base_pfn, int count);
void exit_rtkbp(struct mem_bp *bp);
unsigned long alloc_rtkbp_memory(struct mem_bp *bp, unsigned int order);
unsigned long alloc_rtkbp_memory_specific(struct mem_bp *bp, unsigned int order, unsigned long pageno);
void free_rtkbp_memory(struct mem_bp *bp, unsigned long pageno, unsigned int order);
void show_rtkbp(struct mem_bp *bp);

#else

#ifndef INVALID_VAL
#define INVALID_VAL    ULONG_MAX
#endif

static inline int add_ban_info(unsigned long start, unsigned long size, int type)
{
	if (type == BAN_NOT_USED)
		return memblock_reserve(start, size);

	return 0;
}
static inline bool is_banned(unsigned long pfn, int mask) { return false; }
static inline void init_rtkbp(struct mem_bp *bp, unsigned long base_pfn, int count) {}
static inline void exit_rtkbp(struct mem_bp *bp) {}
static inline unsigned long alloc_rtkbp_memory(struct mem_bp *bp, unsigned int order) { return INVALID_VAL; }
static inline unsigned long alloc_rtkbp_memory_specific(struct mem_bp *bp, unsigned int order, unsigned long pageno) { return INVALID_VAL; }
static inline void free_rtkbp_memory(struct mem_bp *bp, unsigned long pageno, unsigned int order) {}
static inline void show_rtkbp(struct mem_bp *bp) {}

#endif

#endif /* __KERNEL__ */
#endif /* _LINUX_RTKBLUEPRINT_H */

