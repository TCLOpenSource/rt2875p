#ifndef _DVR_CMA_H
#define _DVR_CMA_H

#include <linux/mm.h>
#include <linux/device.h>

struct page;

struct page *dvr_alloc_from_contiguous(struct device *dev, size_t count,
				       unsigned int align, bool no_warn);
bool dvr_release_from_contiguous(struct device *dev, struct page *pages,
				       int count);
struct page *dvr_bitmap_alloc_from_contiguous(struct device *dev, size_t count,
				       unsigned int align, bool no_warn);
struct page *dvr_bitmap_alloc_memory_specific_count(struct device *dev, size_t count, unsigned int align,
				       unsigned long pfn, bool no_warn);
struct page *dvr_bitmap_alloc_memory_specific(struct device *dev, unsigned int order, 
				       unsigned long pfn, bool no_warn);
bool dvr_bitmap_release_from_contiguous(struct device *dev, struct page *pages,
				       int count);
void dvr_show_bitmap(struct device *dev);

#ifndef CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE
int __init dvr_declare_contiguous(struct device *dev, phys_addr_t size,
				       phys_addr_t base, phys_addr_t limit, const char *name);
int __init dvr_declare_null(struct device *dev);
#endif

#endif /* _DVR_CMA_H */