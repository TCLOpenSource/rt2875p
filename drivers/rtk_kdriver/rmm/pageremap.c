/*
 *  linux/mm/pageremap.c
 *
 *  Copyright (C) 1991, 1992, 1993, 1994  Linus Torvalds
 *
 *  Support of page remapping, EJ Hsu
 */

#include <linux/version.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/module.h>
#include <rtd_log/rtd_module_log.h>
#include <linux/mmzone.h>
#include <linux/swap.h>
#include <linux/rmap.h>
#include <linux/semaphore.h>
#include <linux/mm_inline.h>
#include <linux/delay.h>
#include <linux/gfp.h>
#include <linux/smp.h>
#include <linux/memblock.h>
#include <linux/migrate.h>
#include <linux/vmalloc.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include <linux/dma-map-ops.h>
#else
#include <linux/dma-contiguous.h>
#endif
#include <linux/cma.h>
#ifdef CONFIG_CMA_TRACK_USE_PAGE_OWNER
#include <linux/page_owner.h>
#endif
#include <mach/rtk_platform.h>

#include <asm/cacheflush.h>
#include <rtk_kdriver/rmm/rtkcacheflush.h>
#include <rtk_kdriver/rmm/rmm_vendor_hook.h>
#include <rtk_kdriver/rmm/dvr_cma.h>
#include <rtk_kdriver/rmm/rtkblueprint.h>
#include <rtk_kdriver/rmm/rtkrecord.h>
#include <rtk_kdriver/rmm/auth.h>
#include <rtk_kdriver/rmm/pageremap.h>

#ifdef CONFIG_RTK_KDRIVER_SUPPORT
#include <rtk_kdriver/rtk_qos_export.h>
#include <rtk_kdriver/rmm/rmm.h>
#include <rtk_kdriver/rtk-kdrv-common.h>
#endif

#if IS_ENABLED(CONFIG_DMABUF_HEAPS_DVR) && IS_ENABLED(CONFIG_DMABUF_CARVEOUT_HEAP) && IS_ENABLED(CONFIG_DMABUF_HEAPS_VBM)
#include <linux/dma-buf.h>
#include <linux/dma-heap.h>
//#include <uapi/linux/dma-heap.h>
#include <rtk_kdriver/rmm/rmm_heap.h>

static struct dma_heap * dvr_heap = NULL;
static struct dma_heap * carveout_heap = NULL;
static struct dma_heap * vbm_heap = NULL;
#endif

#include "cma.h"

extern struct device	*auth_dev;
extern struct rtkcma rtkcma_list[];

typedef bool (*page_filter_t)(struct page *);

#ifdef CONFIG_CMA_FIRST_POLICY

extern int cma_reserved;

#ifdef CONFIG_QUICK_HIBERNATION
extern struct list_head dcu1_record_list;
extern struct list_head dcu2_record_list;

static record_struct *curr_record = NULL;
static struct page *pagelist = NULL;
static int page_max = 0;
static int page_cnt = 0;
#endif
#endif

#define REMAP_ZONE		0
#define REMAP_PAGE		1
#define REMAP_CMA1		2
#define REMAP_CMA2		3
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
#define REMAP_BW_CMA		4
#endif

#define REMAP_IN_SUSPEND	0x80000000

#define RESERVED_COUNT		1024

DEFINE_SEMAPHORE(remap_sem);
static unsigned long dcu1_size = 0;
static unsigned long dcu2_size = 0;
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
static unsigned long highmem_bw_size = 0;
#endif

static DEFINE_MUTEX(cache_mutex);

bool _debug_cma_guardband_enabled __read_mostly;
bool _debug_cma_guardband_hard;
bool _debug_cma_deferfree_enabled __read_mostly;
bool _debug_cma_deferfree_hard;
extern int cma_deferfree_delay;
extern int cma_debug_workq_delay;

bool in_cma_range(struct device *dev, unsigned long pfn)
{
	struct cma *cma = dev_get_cma_area(dev);

	if (!cma)
		return false;
	else {
		unsigned long cma_base_pfn = cma->base_pfn;
		unsigned long cma_count = cma->count;

		if ((pfn >= cma_base_pfn) && (pfn < (cma_base_pfn + cma_count)))
			return true;
		else
			return false;
	}
}
EXPORT_SYMBOL(in_cma_range);

unsigned long cma_get_avail_size(struct rtkcma *rtkcma)
{
	if (rtkcma->bp)
		return rtkcma->bp->avail_size;

	return 0;
}
EXPORT_SYMBOL(cma_get_avail_size);

void add_memory_size(int flags, unsigned long size)
{
	switch (flags) {
		case GFP_DCU1:
			dcu1_size += size;
			break;

		case GFP_DCU2:
			dcu2_size += size;
			break;

#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
		case GFP_HIGHMEM_BW:
			highmem_bw_size += size;
			break;
#endif
	}
}
EXPORT_SYMBOL(add_memory_size);

unsigned long get_memory_size(int flags)
{
	switch (flags) {
		case GFP_DCU1:
			return dcu1_size;

		case GFP_DCU2:
			return dcu2_size;

#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
		case GFP_HIGHMEM_BW:
			return highmem_bw_size;
#endif

		default:
			return 0;
	}
}
EXPORT_SYMBOL(get_memory_size);

bool check_cma_memory(unsigned long pfn)
{
    int dev_idx = 0;

    for (dev_idx = DEV_CMA1_DEFAULT; dev_idx < DEV_CMA_NUM; dev_idx++) {
        struct device *dev = ERR_PTR(INVALID_VAL);
        const char *name = NULL;
        struct page *cache = NULL;
        unsigned long *cache_bitmap = NULL;

        if (!rtkcma_list[dev_idx].exist)
            continue;

        if (dev_idx == DEV_CMA3_GAL)
            continue;

        dev = rtkcma_list[dev_idx].dev;
        cache = rtkcma_list[dev_idx].cache;
        cache_bitmap = rtkcma_list[dev_idx].cache_bitmap;
        name = (char *)rtkcma_list[dev_idx].name;

        if (in_cma_range(dev, pfn))
            return true;
    }

    return false;
}
EXPORT_SYMBOL(check_cma_memory);

unsigned long get_cma_avail_size(void)
{
	unsigned long size = 0;

	if (dcu1_size)
		size += cma_get_avail_size(&rtkcma_list[DEV_CMA1_DEFAULT]);
	if (dcu2_size)
		size += cma_get_avail_size(&rtkcma_list[DEV_CMA2]);
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
	if (carvedout_buf_query(CARVEDOUT_BW_HIGH_CMA, NULL) > 0) {
		if (highmem_bw_size)
			size += cma_get_avail_size(&rtkcma_list[DEV_CMA6_BW]);
	}
#endif

	if (size)
		return (size << PAGE_SHIFT);
	else
		return size;
}
EXPORT_SYMBOL(get_cma_avail_size);

static int cal_order(size_t size)
{
	return (size == (1 << (fls(size)-1))) ? fls(size)-1 : fls(size);
}

void *dma_get_allocator(struct device *dev);

static struct page *alloc_cache_page(int count, int flags)
{
    int pageno = 0;
    int dev_idx = 0;
    struct page *page = NULL;
    struct device *dev = ERR_PTR(INVALID_VAL);
    struct page *cache = NULL;
    unsigned long *cache_bitmap = NULL;
    const char *name = NULL;
    unsigned int cache_size = 0;
	bool retry = false;
	unsigned int retry_cnt = 0;

    if (count == 0)
        goto invalid_out;

retry_fallback:
    if (!(count <= MAX_CACHE_COUNT)) {
		if ((flags & GFP_DCU1_LIMIT) && (count <= 128)) // 512KB
			; // fall-through
		else if (flags & GFP_DCU2_VDEC_CMPHDR)
			; // fall-through
		else
			goto invalid_out;
	}

    if (flags & GFP_DCU1 || flags & GFP_DCU1_FIRST) {
        dev_idx = DEV_CMA1_DEFAULT;
        cache_size = DEF_CACHE_SIZE_LOW;
    } else if (flags & GFP_DCU1_LIMIT) {
        dev_idx = DEV_CMA4_CMA1_LIMIT;
        cache_size = 4*_MB_; //DEF_CACHE_SIZE_LOW;
    } else if (flags & GFP_DCU2_VDEC_CMPHDR) {
		if (retry) {
			dev_idx = DEV_CMA1_DEFAULT;
			cache_size = DEF_CACHE_SIZE_LOW;
		} else {
			dev_idx = DEV_CMA2;
			cache_size = DEF_CACHE_SIZE;
		}
    }
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
    else if (flags & GFP_HIGHMEM_BW || flags & GFP_HIGHMEM_BW_FIRST) {
		dev_idx = DEV_CMA5_BW;
		cache_size = DEF_CACHE_SIZE;
    }
#endif

    if ((dev_idx >= DEV_CMA1_DEFAULT) && (dev_idx < DEV_CMA_NUM)) {
        dev = rtkcma_list[dev_idx].dev;
        cache = rtkcma_list[dev_idx].cache;
        cache_bitmap = rtkcma_list[dev_idx].cache_bitmap;
        name = (char *)rtkcma_list[dev_idx].name;
    }

    if (!IS_ERR(dev)) {
        mutex_lock(&cache_mutex);
        if (!cache) {
            int bitmap_size = BITS_TO_LONGS(cache_size / PAGE_SIZE) * sizeof(long);
            cache = dvr_alloc_from_contiguous(dev, (cache_size / PAGE_SIZE), 0, true);
            if (!cache) {
                rtd_pr_rmm_debug("remap: can not allocate page cache on %s(%d)...\n", name, dev_idx);
                mutex_unlock(&cache_mutex);
                goto no_page;
            }
            cache_bitmap = kzalloc(bitmap_size, GFP_KERNEL);
            BUG_ON(!cache_bitmap);

            // update
            rtkcma_list[dev_idx].cache = cache;
            rtkcma_list[dev_idx].cache_bitmap = cache_bitmap;
        }
        pageno = bitmap_find_next_zero_area(cache_bitmap, (cache_size / PAGE_SIZE), 0, count, 0);
        if (pageno >= (cache_size / PAGE_SIZE)) {
            rtd_pr_rmm_debug("remap: not enough cache memory for %s(%d)...\n", name, dev_idx);
            mutex_unlock(&cache_mutex);
            goto no_page;
        } else {
            bitmap_set(cache_bitmap, pageno, count);
//            rtd_pr_rmm_info("weight: %d \n", __bitmap_weight(cache_bitmap, cache_size / PAGE_SIZE));

            page = cache + pageno;
            rtd_pr_rmm_debug("remap: alloc pageno: %d(pfn=0x%lx) count: %d for %s(%d)\n", pageno, page_to_pfn(page), count, name, dev_idx);
            mutex_unlock(&cache_mutex);

            SetPageReserved(page);
        }
    } else {
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
//			if (flags & GFP_HIGHMEM_BW_FIRST)
//				page = alloc_pages(GFP_KERNEL | __GFP_HIGHMEM, cal_order(count));
//			else if (flags & GFP_HIGHMEM_BW)
//				page = alloc_pages(GFP_KERNEL, cal_order(count));
//			else
#endif
//        if (flags & GFP_DCU2_FIRST)
//            page = alloc_pages(GFP_KERNEL | __GFP_HIGHMEM, cal_order(count));
//        else if (flags & GFP_DCU2)
//            page = alloc_pages(GFP_KERNEL, cal_order(count));

        if (page) {
            SetPageReserved(page);
            split_page(page, cal_order(count));
            rtd_pr_rmm_info("remap: alloc (high=%d)page %p(pfn=0x%lx) flags(%x) count: %d for %s(%d)\n", PageHighMem(page), page, page_to_pfn(page), flags, count, name, dev_idx);
        } else {
            rtd_pr_rmm_debug("remap: not enough cache memory for flags(%x), dev(%x) on %s(%d)...\n", flags, dev, name, dev_idx);
            goto no_page;
        }
    }

    return page;

no_page:
	/* Limited 125 page count (500KB) for 8K content cmphdr 
	 * which remain in cma2 if full of cma2 cache pool.
	 */
	if ((flags & GFP_DCU2_VDEC_CMPHDR) && (retry_cnt < 1) && (count < 125)) {  // only allow 1 time fallback
		retry = true;
		retry_cnt++;
		goto retry_fallback;
	}
invalid_out:
    return NULL;
}

/* return 1: cache page cleared, 0: not cleared */
static int free_cache_page(struct page *page, int count)
{
    int dev_idx = 0;
    int in_cma = 0;

    if (!PageReserved(page))
        return 0;

    for (dev_idx = DEV_CMA1_DEFAULT; dev_idx < DEV_CMA_NUM; dev_idx++) {
        struct device *dev = ERR_PTR(INVALID_VAL);
        const char *name = NULL;
        struct page *cache = NULL;
        unsigned long *cache_bitmap = NULL;

        if (!rtkcma_list[dev_idx].exist)
            continue;

        dev = rtkcma_list[dev_idx].dev;
        cache = rtkcma_list[dev_idx].cache;
        cache_bitmap = rtkcma_list[dev_idx].cache_bitmap;
        name = (char *)rtkcma_list[dev_idx].name;

        if (in_cma_range(dev, page_to_pfn(page))) {
			int i;
			for (i = 0; i < count; i++) {
				if (page_count(page + i) != 1) {
					rtd_pr_rmm_err("remap: page %lx on %s(%d) reference count is wrong...\n", (unsigned long)page, name, dev_idx);
					BUG();
				}
			}
			rtd_pr_rmm_debug("remap: free pageno: %ld(pfn=%lx) count: %d, dev(%lx) on %s(%d)\n", page - cache, page_to_pfn(page), count, 
                    (unsigned long)dev, name, dev_idx);
			ClearPageReserved(page);
			mutex_lock(&cache_mutex);
			bitmap_clear(cache_bitmap, page - cache, count);
			mutex_unlock(&cache_mutex);

            in_cma = 1;
        }
    }

    if (!in_cma) {
        if (page_count(page) != 1) {
            rtd_pr_rmm_err("remap: page %lx (high=%d) reference count is wrong...\n", (unsigned long)page, PageHighMem(page));
            BUG();
        }
        rtd_pr_rmm_info("remap: free (high=%d)page %p count: %d\n", PageHighMem(page), page, count);
        ClearPageReserved(page);
        free_contig_range(page_to_pfn(page), 1 << cal_order(count));
    }

    return 1;
}

#ifdef CONFIG_VBM_CMA
struct page *alloc_specific_contiguous_memory(size_t size, unsigned long addr, int flags, const void *caller)
{
//	void *ret = 0;
	int count = 0;
	struct page *page = NULL;

	BUG_ON(size & (PAGE_SIZE - 1));
	count = size >> PAGE_SHIFT;
	if (count <= 0) {
		rtd_pr_rmm_err("%s [CMA ERR] %pF requests invalid CMA size %ld (count %d) \n", __func__, caller, size, count);
		return 0;
	}

    page = alloc_cache_page(count, flags);
    if (page) {
        goto out;
	}

	if (!flags || (flags & GFP_VBM_CMA)) {
		page = dvr_bitmap_alloc_memory_specific_count(rtkcma_list[DEV_CMA_VBM].dev, count, 0, addr >> PAGE_SHIFT, true);
	} else {
		rtd_pr_rmm_err("%s remap: wrong allocation flags: %08x\n", __func__, flags);
		page = 0;
	}

//	if (!page)
//		goto out;
//
//	/* we will flush cache later if page is in high memory... */
//	if (!PageHighMem(page)) {
//		unsigned long phys_addr = page_to_pfn(page) << PAGE_SHIFT;
//
//		ret = page_address(page);
//		if (ret) {
//			dmac_flush_range(ret, ret + size);
//			outer_flush_range(phys_addr, phys_addr + size);
//		} else {
//			BUG();
//		}
//	}

	if (!page) {
		rtd_pr_rmm_err("%s [CMA ERR] %pF, flag=%d, size=%zu, addr=0x%lx\n", __func__, caller, flags, size, addr);
		rtk_record_list_dump();
	}
out:
	return page;
}
EXPORT_SYMBOL(alloc_specific_contiguous_memory);
#endif

struct page *alloc_contiguous_memory(size_t size, int flags, const void *caller)
{
//	void *ret = 0;
	int count = 0;
	struct page *page = NULL;

	BUG_ON(size & (PAGE_SIZE - 1));
	count = size >> PAGE_SHIFT;
	if (count <= 0) {
		rtd_pr_rmm_err("[CMA ERR] %pF requests invalid CMA size %ld (count %d) \n", caller, size, count);
		return 0;
	}

#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
	if (carvedout_buf_query(CARVEDOUT_BW_HIGH_CMA, NULL) == 0) {
		if (flags & GFP_HIGHMEM_BW) {
			flags &= ~GFP_HIGHMEM_BW;
			flags |= GFP_DCU2;
		}
		if (flags & GFP_HIGHMEM_BW_FIRST) {
			flags &= ~GFP_HIGHMEM_BW_FIRST;
			flags |= GFP_DCU2_FIRST;
		}
	}
#endif

	if ((flags & GFP_DCU1_LIMIT) && !rtkcma_list[DEV_CMA4_CMA1_LIMIT].exist) {
		flags = GFP_DCU1;
		rtd_pr_rmm_info("remap: %s not exist, turn to %s...\n", rtkcma_list[DEV_CMA4_CMA1_LIMIT].name, rtkcma_list[DEV_CMA1_DEFAULT].name);
	}

    page = alloc_cache_page(count, flags);
    if (page) {
        goto out;
	}

	if (!flags || (flags & GFP_DCU1_FIRST)) {
		page = dvr_alloc_from_contiguous(rtkcma_list[DEV_CMA1_DEFAULT].dev, count, 0, true);
		if (!page) {
			page = dvr_alloc_from_contiguous(auth_dev, count, 0, true);
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
			if (!page) {
				if (carvedout_buf_query(CARVEDOUT_BW_HIGH_CMA, NULL) > 0) {
					page = dvr_alloc_from_contiguous(rtkcma_list[DEV_CMA6_BW].dev, count, 0, true);
				}
			}
#endif
		}
	} else if (flags & GFP_DCU2_FIRST) {
		page = dvr_alloc_from_contiguous(auth_dev, count, 0, true);
		if (!page) {
			page = dvr_alloc_from_contiguous(rtkcma_list[DEV_CMA1_DEFAULT].dev, count, 0, true);
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
			if (!page) {
				if (carvedout_buf_query(CARVEDOUT_BW_HIGH_CMA, NULL) > 0) {
					page = dvr_alloc_from_contiguous(rtkcma_list[DEV_CMA6_BW].dev, count, 0, true);
				}
			}
#endif
		}
	} else if (flags & GFP_DCU1) {
		page = dvr_alloc_from_contiguous(rtkcma_list[DEV_CMA1_DEFAULT].dev, count, 0, true);
	} else if (flags & GFP_DCU1_LIMIT) {
		page = dvr_alloc_from_contiguous(rtkcma_list[DEV_CMA4_CMA1_LIMIT].dev, count, 0, true);
	} else if (flags & GFP_DCU2) {
		page = dvr_alloc_from_contiguous(auth_dev, count, 0, true);
	} else if (flags & GFP_DCU2_VDEC_CMPHDR) {
		page = dvr_alloc_from_contiguous(auth_dev, count, 0, true);
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
	} else if (flags & GFP_HIGHMEM_BW_FIRST) {
		if (carvedout_buf_query(CARVEDOUT_BW_HIGH_CMA, NULL) > 0) {
			page = dvr_alloc_from_contiguous(rtkcma_list[DEV_CMA6_BW].dev, count, 0, true);
			if (!page) {
				page = dvr_alloc_from_contiguous(rtkcma_list[DEV_CMA1_DEFAULT].dev, count, 0, true);
				if (!page && !strstr(caller, "vdec")) {
					page = dvr_alloc_from_contiguous(auth_dev, count, 0, true);
				}
			}
		}
	} else if (flags & GFP_HIGHMEM_BW) {
		if (carvedout_buf_query(CARVEDOUT_BW_HIGH_CMA, NULL) > 0) {
			page = dvr_alloc_from_contiguous(rtkcma_list[DEV_CMA6_BW].dev, count, 0, true);
		}
#endif
	} else if (flags & GFP_GPU_4K) {
		page = dvr_bitmap_alloc_from_contiguous(rtkcma_list[DEV_CMA_GPU_4K].dev, count, 0, true);
		if (!page) {
			rtd_pr_rmm_info("remap: CMA render target 4K fallback to CMA-2\n");
			page = dvr_alloc_from_contiguous(auth_dev, count, 0, true);
			if (!page) {
				rtd_pr_rmm_info("remap: CMA render target 4K fallback to CMA-1\n");
				page = dvr_alloc_from_contiguous(rtkcma_list[DEV_CMA1_DEFAULT].dev, count, 0, true);
			}
		}
	} else if (flags & GFP_CMA_64BIT) {
		page = dvr_alloc_from_contiguous(rtkcma_list[DEV_CMA_64BIT].dev, count, 0, true);
		if (!page) {
			page = dvr_alloc_from_contiguous(auth_dev, count, 0, true);
			if (!page) {
				page = dvr_alloc_from_contiguous(rtkcma_list[DEV_CMA1_DEFAULT].dev, count, 0, true);
			}
		}
#ifdef CONFIG_RTK_KDRV_PCIE
	} else if (flags & GFP_CMA_PCIE) {
		page = dvr_alloc_from_contiguous(rtkcma_list[DEV_CMA_PCIE].dev, count, 0, true);
		if (!page) {
			rtd_pr_rmm_err("remap: alloc PCIE CMA failed\n");
		}
#endif
	} else {
		rtd_pr_rmm_err("remap: wrong allocation flags: %08x\n", flags);
		page = 0;
	}

//	if (!page)
//		goto out;
//
//	/* we will flush cache later if page is in high memory... */
//	if (!PageHighMem(page)) {
//		unsigned long phys_addr = page_to_pfn(page) << PAGE_SHIFT;
//
//		ret = page_address(page);
//		if (ret) {
//			dmac_flush_range(ret, ret + size);
//			outer_flush_range(phys_addr, phys_addr + size);
//		} else {
//			BUG();
//		}
//	}

	if (!page) {
		rtd_pr_rmm_err("[CMA ERR] %pF, flag=%d, size=%zu\n", caller, flags, size);
		rtk_record_list_dump();
	}
out:
	return page;
}
EXPORT_SYMBOL(alloc_contiguous_memory);
 
int free_contiguous_memory(struct page *page, int count)
{
	bool ret = false;

    if (free_cache_page(page, count)) {
        return 0; // cache page cleared
	}

	ret |= dvr_release_from_contiguous(rtkcma_list[DEV_CMA1_DEFAULT].dev, page, count);
	ret |= dvr_release_from_contiguous(auth_dev, page, count);
#ifdef CONFIG_VBM_CMA
    if (rtkcma_list[DEV_CMA_VBM].exist)
		ret |= dvr_bitmap_release_from_contiguous(rtkcma_list[DEV_CMA_VBM].dev, page, count);
#endif
#ifdef CONFIG_OPTEE_SUPPORT_MC_ALLOCATOR
    if (rtkcma_list[DEV_CMA5_MC].exist)
		ret |= dvr_bitmap_release_from_contiguous(rtkcma_list[DEV_CMA5_MC].dev, page, count);
#endif
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
    if (rtkcma_list[DEV_CMA6_BW].exist)
		ret |= dvr_bitmap_release_from_contiguous(rtkcma_list[DEV_CMA6_BW].dev, page, count);
#endif

    if (rtkcma_list[DEV_CMA4_CMA1_LIMIT].exist) {
		ret |= dvr_release_from_contiguous(rtkcma_list[DEV_CMA4_CMA1_LIMIT].dev, page, count);
 	}

    if (rtkcma_list[DEV_CMA_GPU_4K].exist) {
		ret |= dvr_bitmap_release_from_contiguous(rtkcma_list[DEV_CMA_GPU_4K].dev, page, count);
	}

    if (rtkcma_list[DEV_CMA_64BIT].exist) {
		ret |= dvr_release_from_contiguous(rtkcma_list[DEV_CMA_64BIT].dev, page, count);
	}

#ifdef CONFIG_RTK_KDRV_PCIE
    if (rtkcma_list[DEV_CMA_PCIE].exist)
		ret |= dvr_release_from_contiguous(rtkcma_list[DEV_CMA_PCIE].dev, page, count);
#endif

	if (ret == false)
		BUG();

	return 0;
}
EXPORT_SYMBOL(free_contiguous_memory);

unsigned long pli_malloc(size_t size, int flags)
{
	int count = 0;

	int size_page, size_order;
	int count_page, count_order;

	struct page *page;
	unsigned long phys_addr = INVALID_VAL;

	size = size_page = PAGE_ALIGN(size);
	count = count_page = size >> PAGE_SHIFT;

	if (need_debug_cma_guardband()) {
		size = size_order = dvr_guardband_size(size);
		count = count_order = size_order >> PAGE_SHIFT;
	}

	rtd_pr_rmm_info("pli malloc size: %zu count: %d\n", size, count);

	page = alloc_contiguous_memory(size, flags, __builtin_return_address(0));
	if (!page)
		goto out;

	phys_addr = page_to_pfn(page) << PAGE_SHIFT;

	if (need_debug_cma_guardband()) {
		dvr_guardband_padding(page, count_order, count_page);
	}

	if (rtk_record_insert(phys_addr, RTK_SIGNATURE | BUDDY_ID | count_page, 0, __builtin_return_address(0), DVR_OWNER_NOID)) {
		free_contiguous_memory(page, count);

		return INVALID_VAL;
	}

	dmac_flush_range(phys_to_virt(phys_addr), phys_to_virt(phys_addr)+size);
	outer_flush_range(phys_addr, phys_addr + size);

out:
	rtd_pr_rmm_info("%s(): returned %lx\n", __func__, phys_addr);
	return phys_addr;
}
EXPORT_SYMBOL(pli_malloc);

unsigned long pli_malloc_mesg(size_t size, int flags, char *mesg)
{
	int count = 0;
	struct page *page;
	unsigned long phys_addr = INVALID_VAL;
	char *ptr = NULL;

	int size_page, size_order;
	int count_page, count_order;

	ptr = kmalloc(STR_LENGTH, GFP_KERNEL);
	if (unlikely(!ptr)){
		rtd_pr_rmm_err("%s:kmalloc fail!\n",__func__);
		return INVALID_VAL;
	}

	memset(ptr,0,STR_LENGTH);
	strncpy(ptr,mesg,STR_LENGTH-1);
	ptr[STR_LENGTH-1] = 0;

	size = size_page = PAGE_ALIGN(size);
	count = count_page = size >> PAGE_SHIFT;

	if (need_debug_cma_guardband()) {
		size = size_order = dvr_guardband_size(size);
		count = count_order = size_order >> PAGE_SHIFT;
	}

	rtd_pr_rmm_info("pli malloc size: %zu count: %d\n", size, count);

	page = alloc_contiguous_memory(size, flags, __builtin_return_address(0));
	if (!page)
		goto out;

	phys_addr = page_to_pfn(page) << PAGE_SHIFT;

	if (need_debug_cma_guardband()) {
		dvr_guardband_padding(page, count_order, count_page);
	}

	if (rtk_record_insert(phys_addr, RTK_SIGNATURE | BUDDY_MESG_ID | count_page, 0, ptr, DVR_OWNER_NOID)) {
		free_contiguous_memory(page, count);
		kfree(ptr);

		return INVALID_VAL;
	}

	dmac_flush_range(phys_to_virt(phys_addr), phys_to_virt(phys_addr)+size);
	outer_flush_range(phys_addr, phys_addr + size);

out:
	rtd_pr_rmm_info("%s(): returned %lx\n", __func__, phys_addr);
	return phys_addr;
}
EXPORT_SYMBOL(pli_malloc_mesg);

void pli_free_mesg(const unsigned long arg)
{
	int value, count;
	rtk_record *ptr = NULL;
	char *caller = NULL;

	value = rtk_record_lookup_ex(arg, &ptr, true);
	if ((value & 0xf0000000) != RTK_SIGNATURE) {
		rtd_pr_rmm_err("remap: free memory (%lx) signature error...\n", arg);
		BUG();
	} else {
		caller = (char *)ptr->caller;

		if (need_debug_cma_deferfree()) {
			rtk_record_tag_set(ptr, RTK_RECORD_TAG_DEFER_FREE);
			/* defer free not alloc, mark it */
			//rtk_record_delete((unsigned long)arg);
		} else 
			rtk_record_delete((unsigned long)arg);

		if ((value & ID_MASK) == BUDDY_MESG_ID) {
			if (need_debug_cma_guardband()) {
				count = dvr_guardband_check(arg, value);
			} else {
				count = value & 0x00ffffff;
			}

			if (need_debug_cma_deferfree()) {
				/* defer free not alloc, mark it */
				//free_contiguous_memory(pfn_to_page(arg >> PAGE_SHIFT), count);
				//if(caller)
				//	kfree((void *)caller);
				dvr_guardband_padding(pfn_to_page(arg >> PAGE_SHIFT), 1, 0);
			} else {
				free_contiguous_memory(pfn_to_page(arg >> PAGE_SHIFT), count);
				if(caller)
					kfree((void *)caller);
			}
		} else {
			BUG();
		}
	}
}
EXPORT_SYMBOL(pli_free_mesg);


void pli_free(const unsigned long arg)
{
	int value, count;
	rtk_record *ptr = NULL;

	value = rtk_record_lookup_ex(arg, &ptr, true);
	if ((value & 0xf0000000) != RTK_SIGNATURE) {
		rtd_pr_rmm_err("remap: free memory (%lx) signature error...\n", arg);
		BUG();
	} else {
		if (need_debug_cma_deferfree()) {
			rtk_record_tag_set(ptr, RTK_RECORD_TAG_DEFER_FREE);
			/* defer free not alloc, mark it */
			//rtk_record_delete((unsigned long)arg);
		} else {
			rtk_record_delete((unsigned long)arg);
		}

		if ((value & ID_MASK) == BUDDY_ID) {
			if (need_debug_cma_guardband()) {
				count = dvr_guardband_check(arg, value);
			} else {
				count = value & 0x00ffffff;
			}

			if (need_debug_cma_deferfree()) {
				/* defer free not alloc, mark it */
				//free_contiguous_memory(pfn_to_page(arg >> PAGE_SHIFT), count);
				dvr_guardband_padding(pfn_to_page(arg >> PAGE_SHIFT), 1, 0);
			} else {
				free_contiguous_memory(pfn_to_page(arg >> PAGE_SHIFT), count);
			}
		} else {
			BUG();
		}
	}
}
EXPORT_SYMBOL(pli_free);

static void *remap_memory(unsigned long phys_addr, size_t size, pgprot_t prot, const void *caller)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)) && defined(CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE)
	struct page ** pages;
	int count, i;
	void *addr;

	size = PAGE_ALIGN(size);
	count = size >> PAGE_SHIFT;

	pages = vmalloc(count * sizeof(struct page *));
	if(pages == NULL)
	{
		rtd_pr_rmm_err("err: remap fail (vmalloc)\n");
		return NULL;
	}

	for(i = 0; i < count; i++)
	{
		pages[i] = pfn_to_page((phys_addr + PAGE_SIZE*i) >> PAGE_SHIFT);
	}
	
	addr = vmap(pages, count, VM_MAP, prot);
	vfree(pages);

	return addr;
#else
	struct vm_struct *area;
	unsigned long addr;

	area = get_vm_area_caller(size, /*VM_DVR |*/ VM_IOREMAP | VM_USERMAP, caller);
	if (!area)
		return NULL;

	addr = (unsigned long)area->addr;
	area->phys_addr = phys_addr;

	if (ioremap_page_range(addr, addr + size, area->phys_addr, prot)) {
		vunmap((void *)addr);
		return NULL;
	}
#endif
	return (void *)addr;
}

void *dvr_remap_cached_memory(unsigned long phys_addr, size_t size, const void *caller)
{
#if defined (CONFIG_ARM64)
	return remap_memory(phys_addr, size, __pgprot(PROT_NORMAL), caller);
#else //#if defined (CONFIG_ARM64)
	return remap_memory(phys_addr, size, pgprot_kernel, caller);
#endif //#if defined (CONFIG_ARM64)
}
EXPORT_SYMBOL(dvr_remap_cached_memory);

void *dvr_remap_uncached_memory(unsigned long phys_addr, size_t size, const void *caller)
{
#if defined (CONFIG_ARM64)
	return remap_memory(phys_addr, size, __pgprot(PROT_NORMAL_NC), caller);
#else //#if defined (CONFIG_ARM64)
	return remap_memory(phys_addr, size, pgprot_noncached(pgprot_kernel), caller);
#endif //#if defined (CONFIG_ARM64)
}
EXPORT_SYMBOL(dvr_remap_uncached_memory);

void dvr_unmap_memory(void *cpu_addr, size_t size)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)) && defined(CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE)
	vunmap(cpu_addr);
#else
	unsigned int flags = /*VM_DVR |*/ VM_IOREMAP | VM_USERMAP;
	struct vm_struct *area = find_vm_area(cpu_addr);

	if (!area || (area->flags & flags) != flags) {
		WARN(1, "trying to free invalid uncached area: %p\n", cpu_addr);
		return;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
	vunmap_range((unsigned long)cpu_addr, (unsigned long)cpu_addr + size);
#else
	unmap_kernel_range((unsigned long)cpu_addr, size);
#endif
	vunmap(cpu_addr);
#endif
}
EXPORT_SYMBOL(dvr_unmap_memory);

unsigned long dvr_vmalloc_to_phys(const void *vmalloc_addr)
{
#ifdef CONFIG_ARM64
	unsigned long ret;
	struct page *page = NULL;

	if ((page = vmalloc_to_page(vmalloc_addr)) == NULL) {
		rtd_pr_rmm_err("%s: no pmd or bad with va(0x%lx)\n", __func__, vmalloc_addr);
		goto fallback;
	}

	return page_to_pfn(page) << PAGE_SHIFT;

fallback:
	ret = PAR((unsigned long)vmalloc_addr, 0);
	return (ret & 1) ? INVALID_VAL : (ret & (unsigned long)GENMASK_ULL(47, 12));
#else
	return vmalloc_to_pfn(vmalloc_addr) << PAGE_SHIFT;
#endif
}

unsigned long dvr_to_phys(const void *dvr_addr)
{
	if (((unsigned long)dvr_addr > PAGE_OFFSET) && ((unsigned long)dvr_addr < (unsigned long)high_memory))
		return __pa(dvr_addr);
	else if (is_vmalloc_addr(dvr_addr)) {
		return dvr_vmalloc_to_phys(dvr_addr);
	} else {
		rtd_pr_rmm_err("remap: %p is not eligible parameter...\n", dvr_addr);
		return 0;
	}
}
EXPORT_SYMBOL(dvr_to_phys);

__always_inline static void *dvr_malloc_specific_internal(size_t size, int flags, int id)
{
	void *ret = 0;
	int count = 0;

	int size_page, size_order;
	int count_page, count_order;

	struct page *page;
	unsigned long phys_addr = INVALID_VAL;

	size = size_page = PAGE_ALIGN(size);
	count = count_page = size >> PAGE_SHIFT;

	if (need_debug_cma_guardband()) {
		size = size_order = dvr_guardband_size(size);
		count = count_order = size_order >> PAGE_SHIFT;
	}

	rtd_pr_rmm_info("dvr malloc size: %zu count: %d\n", size, count);

	page = alloc_contiguous_memory(size, flags, __builtin_return_address(0));
	if (!page)
		goto out;

	phys_addr = page_to_pfn(page) << PAGE_SHIFT;

	if (PageHighMem(page)) {
		ret = dvr_remap_cached_memory(phys_addr, size, __builtin_return_address(0));
		if (!ret) {
			free_contiguous_memory(page, count);
			ret = 0;
			goto out;
		}
	} else {
		ret = page_address(page);
	}

	if (need_debug_cma_guardband()) {
		dvr_guardband_padding(page, count_order, count_page);
	}

	dmac_flush_range(ret, ret + size);
	outer_flush_range(phys_addr, phys_addr + size);

	if (rtk_record_insert(phys_addr, RTK_SIGNATURE | id | count_page, 0, __builtin_return_address(0), DVR_OWNER_NOID)) {
		free_contiguous_memory(page, count);
		ret = 0;
		goto out;
	}
out:
	rtd_pr_rmm_info("%s(): returned 0x%lx\n", __func__, (unsigned long)ret);
	return ret;
}

__always_inline void *dvr_malloc_specific(size_t size, int flags)
{
	return dvr_malloc_specific_internal(size, flags, DRIVER_ID);
}
EXPORT_SYMBOL(dvr_malloc_specific);

void *dvr_malloc(size_t size)
{
	return dvr_malloc_specific(size, 0);
}
EXPORT_SYMBOL(dvr_malloc);

void *dvr_malloc_specific_addr(size_t size, int flags, unsigned long addr)
{
	void *ret = 0;
	struct mem_bp *bp;
	unsigned long pfn;
	struct page *page;
	unsigned long phys_addr = INVALID_VAL;
	int order, count;

#ifdef CONFIG_VBM_CMA
	if (flags == GFP_VBM_CMA) {
		count = size >> PAGE_SHIFT;
		page = dvr_bitmap_alloc_memory_specific_count(rtkcma_list[DEV_CMA_VBM].dev, count, 0, addr >> PAGE_SHIFT, true);
		if (page == NULL) {
			ret = 0;
			goto out;
		} else {
			goto got_page;
		}
	}
#endif

	order = get_order(size);
	count = 1 << order;
	size = count << PAGE_SHIFT; // order align

#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
	if (carvedout_buf_query(CARVEDOUT_BW_HIGH_CMA, NULL) == 0) {
		if (flags == GFP_HIGHMEM_BW) {
			rtd_pr_rmm_err("No HIGHMEM_BW region, should never use GFP_HIGHMEM_BW for dvr_malloc_specific_addr()\n");
			goto out;
		}
	}
#endif
	
	switch (flags) {
	case GFP_DCU1:
		bp = dma_get_allocator(rtkcma_list[DEV_CMA1_DEFAULT].dev);
		break;
	case GFP_DCU2:
		bp = dma_get_allocator(auth_dev);
		break;
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
	case GFP_HIGHMEM_BW:
		bp = dma_get_allocator(rtkcma_list[DEV_CMA6_BW].dev);
		break;
#endif
	default:
		goto out;
	}

	pfn = alloc_rtkbp_memory_specific(bp, order, addr >> PAGE_SHIFT);
	if (pfn == INVALID_VAL) {
		goto out;
	}

	for (;;) {
		int err;

		err = alloc_contig_range(pfn, pfn + count, MIGRATE_CMA, true);
		if (err == 0) {
			page = pfn_to_page(pfn);
			break;
		} else if (err != -EBUSY) {
			free_rtkbp_memory(bp, pfn, order);
			goto out;
		}

		rtd_pr_rmm_info("remap: retry remapping...\n");
		msleep(10);
	}

#ifdef CONFIG_VBM_CMA
got_page:
#endif
	phys_addr = page_to_pfn(page) << PAGE_SHIFT;

	if (PageHighMem(page)) {
		ret = dvr_remap_cached_memory(phys_addr, size, __builtin_return_address(0));
		if (!ret) {
			free_contiguous_memory(page, count);
			ret = 0;
			goto out;
		}
	} else {
		ret = page_address(page);
	}
	dmac_flush_range(ret, ret + size);
	outer_flush_range(phys_addr, phys_addr + size);

	if (rtk_record_insert(phys_addr, RTK_SIGNATURE | DRIVER_ID | count, 0, __builtin_return_address(0), DVR_OWNER_NOID)) {
		free_contiguous_memory(page, count);
		ret = 0;
		goto out;
	}

#ifdef CONFIG_CMA_TRACK_USE_PAGE_OWNER
	if (page) {
		gfp_t gfp_mask;
		gfp_mask = __GFP_MOVABLE;
		#ifdef CONFIG_PAGE_OWNER_RECORD_COUNT
		set_page_owner(page, count, gfp_mask);
		#else
		set_page_owner(page, order, gfp_mask);
		#endif
	}
#endif

out:
	rtd_pr_rmm_info("%s(): returned %lx\n", __func__, (unsigned long)ret);
	return ret;
}

void *dvr_malloc_nosave_specific(size_t size, int flags)
{
	return dvr_malloc_specific_internal(size, flags, NOSAVE_ID);
}
EXPORT_SYMBOL(dvr_malloc_nosave_specific);

void *dvr_malloc_nosave(size_t size)
{
	return dvr_malloc_nosave_specific(size, 0);
}
EXPORT_SYMBOL(dvr_malloc_nosave);

__always_inline void *dvr_malloc_uncached_specific(size_t size, int flags, void **uaddr)
{
	void *ret = 0;
	int count = 0;

	int size_page, size_order;
	int count_page, count_order;

	struct page *page;
	unsigned long phys_addr = INVALID_VAL;

	size = size_page = PAGE_ALIGN(size);
	count = count_page = size >> PAGE_SHIFT;

	if (need_debug_cma_guardband()) {
		size = size_order = dvr_guardband_size(size);
		count = count_order = size_order >> PAGE_SHIFT;
	}

	rtd_pr_rmm_info("dvr malloc size: %zu count: %d\n", size, count);

	page = alloc_contiguous_memory(size, flags, __builtin_return_address(0));
	if (!page)
		goto out;

	phys_addr = page_to_pfn(page) << PAGE_SHIFT;

	if (PageHighMem(page)) {
		ret = dvr_remap_cached_memory(phys_addr, size, __builtin_return_address(0));
		if (!ret) {
			free_contiguous_memory(page, count);
			ret = 0;
			goto out;
		}
	} else {
		ret = page_address(page);
	}

	if (need_debug_cma_guardband()) {
		dvr_guardband_padding(page, count_order, count_page);
	}

	dmac_flush_range(ret, ret + size);
	outer_flush_range(phys_addr, phys_addr + size);

	if (uaddr) {
		*uaddr = dvr_remap_uncached_memory(phys_addr, size, __builtin_return_address(0));
		if (!*uaddr) {
			free_contiguous_memory(page, count);
			ret = 0;
			goto out;
		}

		if (rtk_record_insert(phys_addr, RTK_SIGNATURE | DRIVER_ID | count_page, (unsigned long)*uaddr, __builtin_return_address(0), DVR_OWNER_NOID)) {
			free_contiguous_memory(page, count);
			return 0;
		}
	} else {
		if (rtk_record_insert(phys_addr, RTK_SIGNATURE | DRIVER_ID | count_page, 0, __builtin_return_address(0), DVR_OWNER_NOID)) {
			free_contiguous_memory(page, count);
			return 0;
		}
	}
out:
	rtd_pr_rmm_info("%s(): returned %lx\n", __func__, (unsigned long)ret);
	return ret;
}
EXPORT_SYMBOL(dvr_malloc_uncached_specific);

void *dvr_malloc_uncached(size_t size, void **uaddr)
{
	return dvr_malloc_uncached_specific(size, 0, uaddr);
}
EXPORT_SYMBOL(dvr_malloc_uncached);

void *dvr_malloc_prot_specific(size_t size, int flags, pgprot_t prot)
{
	void *ret = 0;
	int count = 0;

	int size_page, size_order;
	int count_page, count_order;

	struct page *page;
	unsigned long phys_addr = INVALID_VAL;

	size = size_page = PAGE_ALIGN(size);
	count = count_page = size >> PAGE_SHIFT;

	if (need_debug_cma_guardband()) {
		size = size_order = dvr_guardband_size(size);
		count = count_order = size_order >> PAGE_SHIFT;
	}

	rtd_pr_rmm_info("dvr malloc size: %zu count: %d\n", size, count);

	page = alloc_contiguous_memory(size, flags, __builtin_return_address(0));
	if (!page)
		goto out;

	phys_addr = page_to_pfn(page) << PAGE_SHIFT;

	if (PageHighMem(page)) {
		ret = remap_memory(phys_addr, size, prot, __builtin_return_address(0));
		if (!ret) {
			free_contiguous_memory(page, count);
			ret = 0;
			goto out;
		}
	} else {
		ret = page_address(page);
	}

	if (need_debug_cma_guardband()) {
		dvr_guardband_padding(page, count_order, count_page);
	}

	dmac_flush_range(ret, ret + size);
	outer_flush_range(phys_addr, phys_addr + size);

	if (rtk_record_insert(phys_addr, RTK_SIGNATURE | DRIVER_ID | count_page, 0, __builtin_return_address(0), DVR_OWNER_NOID)) {
		free_contiguous_memory(page, count);
		ret = 0;
		goto out;
	}
out:
	rtd_pr_rmm_info("%s(): returned %lx\n", __func__, (unsigned long)ret);
	return ret;
}
EXPORT_SYMBOL(dvr_malloc_prot_specific);

void *dvr_malloc_prot(size_t size, pgprot_t prot)
{
	return dvr_malloc_prot_specific(size, 0, prot);
}
EXPORT_SYMBOL(dvr_malloc_prot);

void dvr_free(const void *virt_addr)
{
	int value, count;
	unsigned long phys_addr, priv;
	rtk_record *ptr = NULL;

	if (is_vmalloc_addr(virt_addr)) {
		phys_addr = vmalloc_to_pfn(virt_addr) << PAGE_SHIFT;
	} else {
		phys_addr = (unsigned long)(page_to_pfn((virt_to_page(virt_addr))) << PAGE_SHIFT);
	}
	value = rtk_record_lookup_ex(phys_addr, &ptr, true);
	if ((value & 0xf0000000) != RTK_SIGNATURE) {
		rtd_pr_rmm_err("remap: free memory (%lx/%p) signature error...\n", phys_addr, virt_addr);
		BUG();
	} else {
		priv = ptr->priv;

		if (need_debug_cma_deferfree()) {
			rtk_record_tag_set(ptr, RTK_RECORD_TAG_DEFER_FREE);
			/* defer free not alloc, mark it */
			//rtk_record_delete((unsigned long)phys_addr);
		} else {
			rtk_record_delete((unsigned long)phys_addr);
		}

		if (((value & ID_MASK) == DRIVER_ID) || ((value & ID_MASK) == NOSAVE_ID)) {
			if (need_debug_cma_guardband()) {
				count = dvr_guardband_check(phys_addr, value);
			} else {
				count = value & 0x00ffffff;
			}
			if (is_vmalloc_addr(virt_addr)) {
				dvr_unmap_memory((void *)virt_addr, count << PAGE_SHIFT);
			}
			if (priv) {
				dvr_unmap_memory((void *)priv, count << PAGE_SHIFT);
			}

			if (need_debug_cma_deferfree()) {
				/* defer free not alloc, mark it */
				//free_contiguous_memory(pfn_to_page(phys_addr >> PAGE_SHIFT), count);
				dvr_guardband_padding(pfn_to_page(phys_addr >> PAGE_SHIFT), 1, 0);
			} else {
				free_contiguous_memory(pfn_to_page(phys_addr >> PAGE_SHIFT), count);
			}
		} else {
			BUG();
		}
	}
}
EXPORT_SYMBOL(dvr_free);

struct page *dvr_malloc_page(size_t size, int flags)
{
	int count = 0;

	int size_page, size_order;
	int count_page, count_order;

	struct page *page;
	unsigned long phys_addr = INVALID_VAL;

	size = size_page = PAGE_ALIGN(size);
	count = count_page = size >> PAGE_SHIFT;

	if (need_debug_cma_guardband()) {
		size = size_order = dvr_guardband_size(size);
		count = count_order = size_order >> PAGE_SHIFT;
	}

	rtd_pr_rmm_info("dvr malloc size: %zu count: %d\n", size, count);

	page = alloc_contiguous_memory(size, flags, __builtin_return_address(0));
	if (!page)
		goto out;

	phys_addr = page_to_pfn(page) << PAGE_SHIFT;

	if (need_debug_cma_guardband()) {
		dvr_guardband_padding(page, count_order, count_page);
	}

	if (rtk_record_insert(phys_addr, RTK_SIGNATURE | DRIVER_ID | count_page, 0, __builtin_return_address(0), DVR_OWNER_NOID)) {
		free_contiguous_memory(page, count);
		page = 0;
		goto out;
	}
out:
	rtd_pr_rmm_info("%s(): returned %lx\n", __func__, (unsigned long)page);
	return page;
}
EXPORT_SYMBOL(dvr_malloc_page);

void dvr_free_page(const struct page *page)
{
	int value, count;
	unsigned long phys_addr;
	rtk_record *ptr = NULL;

	phys_addr = page_to_pfn(page) << PAGE_SHIFT;
	value = rtk_record_lookup_ex(phys_addr, &ptr, true);
	if ((value & 0xf0000000) != RTK_SIGNATURE) {
		rtd_pr_rmm_err("remap: free memory (%lx) signature error...\n", phys_addr);
		BUG();
	} else {
		if (need_debug_cma_deferfree()) {
			rtk_record_tag_set(ptr, RTK_RECORD_TAG_DEFER_FREE);
			/* defer free not alloc, mark it */
			//rtk_record_delete((unsigned long)phys_addr);
		} else {
			rtk_record_delete((unsigned long)phys_addr);
		}

		if ((value & ID_MASK) == DRIVER_ID) {
			if (need_debug_cma_guardband()) {
				count = dvr_guardband_check(phys_addr, value);
			} else {
				count = value & 0x00ffffff;
			}

			if (need_debug_cma_deferfree()) {
				/* defer free not alloc, mark it */
				//free_contiguous_memory(pfn_to_page(phys_addr >> PAGE_SHIFT), count);
				dvr_guardband_padding(pfn_to_page(phys_addr >> PAGE_SHIFT), 1, 0);
			} else {
				free_contiguous_memory(pfn_to_page(phys_addr >> PAGE_SHIFT), count);
			}
		} else {
			BUG();
		}
	}
}
EXPORT_SYMBOL(dvr_free_page);

#ifdef CONFIG_FAKEMEMORY_GRAPHICS
struct page *dvr_malloc_gpu_page(size_t size)
{
	int count = 0;

	int size_page, size_order;
	int count_page, count_order;

	struct page *page;
	unsigned long phys_addr = INVALID_VAL;

	size = size_page = PAGE_ALIGN(size);
	count = count_page = size >> PAGE_SHIFT;

	if (need_debug_cma_guardband()) {
		size = size_order = dvr_guardband_size(size);
		count = count_order = size_order >> PAGE_SHIFT;
	}

	rtd_pr_rmm_info("dvr malloc size: %zu count: %d\n", size, count);

	page = alloc_gpu_memory(size);
	if (!page) {
		rtd_pr_rmm_err("[CMA ERR] gpu, size=%zu\n", size);
		rtk_record_list_dump();
		goto out;
	}

	phys_addr = page_to_pfn(page) << PAGE_SHIFT;


	if (rtk_record_insert(phys_addr, RTK_SIGNATURE | BUDDY_MESG_ID | count_page, 0, "gpu", DVR_OWNER_NOID)) {
		free_gpu_memory(page);
		page = 0;
		goto out;
	}
out:
	rtd_pr_rmm_info("%s(): returned %lx\n", __func__, (unsigned long)page);
	return page;
}
EXPORT_SYMBOL(dvr_malloc_gpu_page);

void dvr_free_gpu_page(const struct page *page)
{
	int value, count;
	unsigned long phys_addr;
	rtk_record *ptr = NULL;

	phys_addr = page_to_pfn(page) << PAGE_SHIFT;
	value = rtk_record_lookup_ex(phys_addr, &ptr, true);
	if ((value & 0xf0000000) != RTK_SIGNATURE) {
		rtd_pr_rmm_err("remap: free memory (%lx) signature error...\n", phys_addr);
		BUG();
	} else {
			rtk_record_delete((unsigned long)phys_addr);
		if ((value & ID_MASK) == BUDDY_MESG_ID) {
				count = value & 0x00ffffff;

				free_gpu_memory(pfn_to_page(phys_addr >> PAGE_SHIFT));
		} else {
			BUG();
		}
	}
}
EXPORT_SYMBOL(dvr_free_gpu_page);
#endif

struct page *dvr_malloc_page_mesg(size_t size, int flags, char *mesg)
{
	int count = 0;

	int size_page, size_order;
	int count_page, count_order;

	struct page *page;
	unsigned long phys_addr = INVALID_VAL;
	char *ptr = NULL;

	ptr = kmalloc(STR_LENGTH, GFP_KERNEL);
	if (unlikely(!ptr)){
		rtd_pr_rmm_err("%s:kmalloc fail!\n",__func__);
		return (struct page *)INVALID_VAL;
	}
	memset(ptr, 0, STR_LENGTH);
	strncpy(ptr, mesg, STR_LENGTH-1);

	size = size_page = PAGE_ALIGN(size);
	count = count_page = size >> PAGE_SHIFT;

	if (need_debug_cma_guardband()) {
		size = size_order = dvr_guardband_size(size);
		count = count_order = size_order >> PAGE_SHIFT;
	}

	rtd_pr_rmm_info("dvr malloc size: %zu count: %d\n", size, count);

	//if (count == 0x2023) {
	if ((count > 0x1f00) && (count < 0x2100)) {
		rtd_pr_rmm_info("dvr malloc from CMA_GPU_4K\n");
		page = alloc_contiguous_memory(size, GFP_GPU_4K, __builtin_return_address(0));;
	} else {
		page = alloc_contiguous_memory(size, flags, __builtin_return_address(0));
	}
	if (!page)
		goto out;

	phys_addr = page_to_pfn(page) << PAGE_SHIFT;

	if (need_debug_cma_guardband()) {
		dvr_guardband_padding(page, count_order, count_page);
	}

	if (strcmp(ptr, " ") == 0) {
		rtd_pr_rmm_info("%s: Unknown string %s, replace for caller func %s\n", __func__, ptr, __builtin_return_address(0));
		memset(ptr, 0, STR_LENGTH);
		strncpy(ptr, __builtin_return_address(0), STR_LENGTH-1);
	}

	if (rtk_record_insert(phys_addr, RTK_SIGNATURE | DRIVER_MESG_ID | count_page, 0, ptr, DVR_OWNER_NOID)) {
		free_contiguous_memory(page, count);
		page = 0;
		goto out;
	}
out:
	rtd_pr_rmm_info("%s(): returned %lx\n", __func__, (unsigned long)page);
	return page;
}
EXPORT_SYMBOL(dvr_malloc_page_mesg);

void dvr_free_page_mesg(const struct page *page)
{
	int value, count;
	unsigned long phys_addr;
	rtk_record *ptr = NULL;
	char *caller = NULL;

	phys_addr = page_to_pfn(page) << PAGE_SHIFT;
	value = rtk_record_lookup_ex(phys_addr, &ptr, true);
	if ((value & 0xf0000000) != RTK_SIGNATURE) {
		rtd_pr_rmm_err("remap: free memory (%lx) signature error...\n", phys_addr);
		BUG();
	} else {
		caller = (char *)ptr->caller;
		if (need_debug_cma_deferfree()) {
			rtk_record_tag_set(ptr, RTK_RECORD_TAG_DEFER_FREE);
			/* defer free not alloc, mark it */
			//rtk_record_delete((unsigned long)phys_addr);
		} else {
			rtk_record_delete((unsigned long)phys_addr);
		}

		if ((value & ID_MASK) == DRIVER_MESG_ID) {
			if (need_debug_cma_guardband()) {
				count = dvr_guardband_check(phys_addr, value);
			} else {
				count = value & 0x00ffffff;
			}

			if (need_debug_cma_deferfree()) {
				/* defer free not alloc, mark it */
				//free_contiguous_memory(pfn_to_page(phys_addr >> PAGE_SHIFT), count);
				dvr_guardband_padding(pfn_to_page(phys_addr >> PAGE_SHIFT), 1, 0);
			} else {
				free_contiguous_memory(pfn_to_page(phys_addr >> PAGE_SHIFT), count);
			}
			if(caller)
				kfree((void *)caller);
		} else {
			BUG();
		}
	}
}
EXPORT_SYMBOL(dvr_free_page_mesg);

#if IS_ENABLED(CONFIG_DMABUF_HEAPS_DVR) && IS_ENABLED(CONFIG_DMABUF_CARVEOUT_HEAP) && IS_ENABLED(CONFIG_DMABUF_HEAPS_VBM)
unsigned long (*__dvr_heap_to_phys) (struct dma_buf *);
void dvr_heap_to_phys_register(unsigned long (*fp)(struct dma_buf *))
{
	if (fp != NULL)
		__dvr_heap_to_phys = fp;
}
EXPORT_SYMBOL(dvr_heap_to_phys_register);

unsigned long (*__vbm_heap_to_phys) (struct dma_buf *);
void vbm_heap_to_phys_register(unsigned long (*fp)(struct dma_buf *))
{
	if (fp != NULL)
		__vbm_heap_to_phys = fp;
}
EXPORT_SYMBOL(vbm_heap_to_phys_register);

unsigned long (*__carveout_heap_to_phys) (struct dma_buf *);
void carveout_heap_to_phys_register(unsigned long (*fp)(struct dma_buf *))
{
	if (fp != NULL)
		__carveout_heap_to_phys = fp;
}
EXPORT_SYMBOL(carveout_heap_to_phys_register);

unsigned long dvr_dmabuf_to_phys(struct dma_buf *dmabuf)
{
	unsigned long ret = INVALID_VAL;

	if (strncmp(dmabuf->exp_name, "dvr", 3) == 0) {
		if (__dvr_heap_to_phys) {
			ret = __dvr_heap_to_phys(dmabuf);
		} else {
			rtd_pr_rmm_err("no __dvr_heap_to_phys fp\n");
			ret = INVALID_VAL;
		}
	} else if (strncmp(dmabuf->exp_name, "carveout", 8) == 0) {
		if (__carveout_heap_to_phys) {
			ret = __carveout_heap_to_phys(dmabuf);
		} else {
			rtd_pr_rmm_err("no __carveout_heap_to_phys fp\n");
			ret = INVALID_VAL;
		}
	} else if (strncmp(dmabuf->exp_name, "vbm", 3) == 0) {
		if (__vbm_heap_to_phys) {
			ret = __vbm_heap_to_phys(dmabuf);
		} else {
			rtd_pr_rmm_err("no __vbm_heap_to_phys fp\n");
			ret = INVALID_VAL;
		}
	} else {
		rtd_pr_rmm_err("Error!! dmabuf from unexist heap %s\n", dmabuf->exp_name);
	}

	rtd_pr_rmm_info("Phys addr of dmabuf from heap %s is 0x%x\n", dmabuf->exp_name, ret);
		
	if (ret != INVALID_VAL)
		return ret;

	return INVALID_VAL;
}
EXPORT_SYMBOL(dvr_dmabuf_to_phys);

/* return phys address and dma_buf handler with argument */
struct dma_buf *dvr_dmabuf_malloc(size_t size, int flags, unsigned long **phys)
{
	struct dma_buf *dmabuf;
	unsigned int fd_flags = O_RDWR;

	if (!dvr_heap && !carveout_heap) {
		dvr_heap = dma_heap_find("dvr");
		carveout_heap = dma_heap_find("carveout");
		rtd_pr_rmm_info("%s, dvr(%lx), carveout(%lx)\n", __func__, dvr_heap, carveout_heap);
	}
	if (!vbm_heap) {
		vbm_heap = dma_heap_find("vbm");
		rtd_pr_rmm_info("%s, vbm(%lx)\n", __func__, vbm_heap);
	}

#ifdef CONFIG_VBM_CMA
	if (flags & GFP_VBM_CMA) {
		//dmabuf = dma_heap_buffer_alloc(carveout_heap, size, fd_flags, 0);
		dmabuf = dma_heap_buffer_alloc(vbm_heap, size, fd_flags, 0);
	}
	else
#endif
#ifdef CONFIG_VBM_CARVEOUT
	if (flags & GFP_VBM_CARVEOUT) {
		dmabuf = dma_heap_buffer_alloc(carveout_heap, size, fd_flags, 0);
	}
	else
#endif
	{
		unsigned int heap_flags = flags & (DMA_HEAP_FLAG_FROM_DCU1 | DMA_HEAP_FLAG_FROM_DCU2);
		dmabuf = dma_heap_buffer_alloc(dvr_heap, size, fd_flags, heap_flags);
	}

	if (IS_ERR(dmabuf)) {
		rtd_pr_rmm_err("%s invalid dmabuf(%lx)\n", __func__, (unsigned long)dmabuf);
		return (struct dma_buf *)PTR_ERR((void *)dmabuf);
	}

	if (phys)
		*phys = (unsigned long *)dvr_dmabuf_to_phys(dmabuf);

	return dmabuf;
}
EXPORT_SYMBOL(dvr_dmabuf_malloc);

#ifdef CONFIG_VBM_CMA
int (*__vbm_heap_to_dmabuf_list) (struct dma_buf *, unsigned long *, int );
void vbm_heap_to_dmabuf_list_register(int (*fp)(struct dma_buf *, unsigned long *, int ))
{
	if (fp != NULL)
		__vbm_heap_to_dmabuf_list = fp;
}
EXPORT_SYMBOL(vbm_heap_to_dmabuf_list_register);
#endif
#ifdef CONFIG_VBM_CARVEOUT
int (*__carveout_heap_to_dmabuf_list) (struct dma_buf *, unsigned long *, int );
void carveout_heap_to_dmabuf_list_register(int (*fp)(struct dma_buf *, unsigned long *, int ))
{
	if (fp != NULL)
		__carveout_heap_to_dmabuf_list = fp;
}
EXPORT_SYMBOL(carveout_heap_to_dmabuf_list_register);
#endif
int dvr_dmabuf_to_list(struct dma_buf *dmabuf, unsigned long buf_array[], int array_num)
{
	int ret = -ENOMEM;

	if (strncmp(dmabuf->exp_name, "dvr", 3) == 0) {
		; // no support
	} 
#ifdef CONFIG_VBM_CMA
	else if (strncmp(dmabuf->exp_name, "vbm", 3) == 0) {
		if (__vbm_heap_to_dmabuf_list) {
			ret = __vbm_heap_to_dmabuf_list(dmabuf, buf_array, array_num);
		} else {
			rtd_pr_rmm_err("no __vbm_heap_to_dmabuf_list fp\n");
			ret = -ENOMEM;
		}
	} 
#endif
#ifdef CONFIG_VBM_CARVEOUT
	else if (strncmp(dmabuf->exp_name, "carveout", 8) == 0) {
		if (__carveout_heap_to_dmabuf_list) {
			ret = __carveout_heap_to_dmabuf_list(dmabuf, buf_array, array_num);
		} else {
			rtd_pr_rmm_err("no __carveout_heap_to_dmabuf_list fp\n");
			ret = -ENOMEM;
		}
	} 
#endif
	else {
		rtd_pr_rmm_err("Error!! dmabuf from unexist heap %s\n", dmabuf->exp_name);
	}

	return ret;
}
EXPORT_SYMBOL(dvr_dmabuf_to_list);

/* return phys address and dma_buf handler with argument */
struct dma_buf *dvr_dmabuf_chunk_malloc(size_t size, int flags, int frame_type, bool need_protect, unsigned long **phys)
{
	struct dma_buf *dmabuf;
	unsigned int fd_flags = O_RDWR;
	enum PLATFORM_MODEL model = get_platform_model();
	struct dma_heap *tmp_heap = NULL;
	unsigned int heap_flags = 0;

	if (flags == VBM_BUF_DECOMP) {
		if (model == PLATFORM_MODEL_8K) {
			if (!carveout_heap) {
				carveout_heap = dma_heap_find("carveout");
				rtd_pr_rmm_info("%s, carveout(%lx)\n", __func__, carveout_heap);
			}
			tmp_heap = carveout_heap;
		} else {
			if (!vbm_heap) {
				vbm_heap = dma_heap_find("vbm");
				rtd_pr_rmm_info("%s, vbm(%lx)\n", __func__, vbm_heap);
			}
			tmp_heap = vbm_heap;
		}
	} else if (flags == VBM_BUF_DECIMATE) {
		if (model == PLATFORM_MODEL_8K) {
			if (!vbm_heap) {
				vbm_heap = dma_heap_find("vbm");
				rtd_pr_rmm_info("%s, vbm(%lx)\n", __func__, vbm_heap);
			}
			tmp_heap = vbm_heap;
		} else {
			rtd_pr_rmm_err("%s invalid vbm_buf_type(%lx) for model(%d)\n", __func__, flags, model);
			return NULL;
		}
	} else {
		rtd_pr_rmm_err("%s invalid flags(%lx)\n", __func__, flags);
		return NULL;
	}

#if defined(CONFIG_VBM_CMA) || defined(CONFIG_VBM_CARVEOUT)
	heap_flags |= DMA_HEAP_FLAG_ALLOC_CHUNKS;
	heap_flags |= 1 << (frame_type + DMA_HEAP_FLAG_USER_SHIFT);
	if (need_protect) {
		heap_flags |= DMA_HEAP_FLAG_NEED_SECURE_PROTECT;
	}
#endif

	dmabuf = dma_heap_buffer_alloc(tmp_heap, size, fd_flags, heap_flags);

	if (IS_ERR(dmabuf)) {
		rtd_pr_rmm_err("%s invalid dmabuf(%lx)\n", __func__, (unsigned long)dmabuf);
		return (struct dma_buf *)PTR_ERR((void *)dmabuf);
	}

	if (phys)
		*phys = (unsigned long *)dvr_dmabuf_to_phys(dmabuf);

	return dmabuf;
}
EXPORT_SYMBOL(dvr_dmabuf_chunk_malloc);

/* return first frame's phys address and dma_buf handler with argument */
struct dma_buf *dvr_dmabuf_frames_malloc(size_t frame_size, unsigned int frame_num, int flags, int frame_type, bool need_protect, unsigned long **first_frame_phys)
{
	struct dma_buf *dmabuf;
	unsigned int fd_flags = O_RDWR;
	enum PLATFORM_MODEL model = get_platform_model();
	struct dma_heap *tmp_heap = NULL;
	unsigned int heap_flags = 0;

	if (!frame_size || !frame_num) {
		rtd_pr_rmm_err("%s invalid frame size(0x%lx) or frame num(%d)\n", __func__, frame_size, frame_num);
	}

	if (flags == VBM_BUF_DECOMP) {
		if (model == PLATFORM_MODEL_8K) {
			if (!carveout_heap) {
				carveout_heap = dma_heap_find("carveout");
				rtd_pr_rmm_info("%s, carveout(%lx)\n", __func__, carveout_heap);
			}
			tmp_heap = carveout_heap;
		} else {
			if (!vbm_heap) {
				vbm_heap = dma_heap_find("vbm");
				rtd_pr_rmm_info("%s, vbm(%lx)\n", __func__, vbm_heap);
			}
			tmp_heap = vbm_heap;
		}
	} else if (flags == VBM_BUF_DECIMATE) {
		if (model == PLATFORM_MODEL_8K) {
			if (!vbm_heap) {
				vbm_heap = dma_heap_find("vbm");
				rtd_pr_rmm_info("%s, vbm(%lx)\n", __func__, vbm_heap);
			}
			tmp_heap = vbm_heap;
		} else {
			rtd_pr_rmm_err("%s invalid vbm_buf_type(%lx) for model(%d)\n", __func__, flags, model);
			return NULL;
		}
	} else {
		rtd_pr_rmm_err("%s invalid flags(%lx)\n", __func__, flags);
		return NULL;
	}

#if defined(CONFIG_VBM_CMA) || defined(CONFIG_VBM_CARVEOUT)
	heap_flags = DMA_HEAP_FLAG_FRAMES_NUM_MASK & frame_num;
	heap_flags |= DMA_HEAP_FLAG_ALLOC_FRAMES;
	heap_flags |= 1 << (frame_type + DMA_HEAP_FLAG_USER_SHIFT);
	if (need_protect) {
		heap_flags |= DMA_HEAP_FLAG_NEED_SECURE_PROTECT;
	}
#endif

	dmabuf = dma_heap_buffer_alloc(tmp_heap, frame_size * frame_num, fd_flags, heap_flags);

	if (IS_ERR(dmabuf)) {
		rtd_pr_rmm_err("%s invalid dmabuf(%lx)\n", __func__, (unsigned long)dmabuf);
		return (struct dma_buf *)PTR_ERR((void *)dmabuf);
	}

	if (first_frame_phys)
		*first_frame_phys = (unsigned long *)dvr_dmabuf_to_phys(dmabuf);

	return dmabuf;
}
EXPORT_SYMBOL(dvr_dmabuf_frames_malloc);

void dvr_dmabuf_free(struct dma_buf *dmabuf)
{
	struct task_struct *task = current;

	if (IS_ERR_OR_NULL(dmabuf)) {
		rtd_pr_rmm_err("%s invalid dmabuf(%lx)\n", __func__, (unsigned long)dmabuf);
		return;
	}

	if (WARN_ON(!dmabuf || !dmabuf->file))
		return;

	if (!(task->flags & PF_KTHREAD))
		fput(dmabuf->file);    // i.e. dma_buf_put()
	else {
		fput(dmabuf->file);
		flush_delayed_fput();
	}

	return;
}
EXPORT_SYMBOL(dvr_dmabuf_free);
#endif

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
unsigned long rpc_dvr_malloc(unsigned long size, unsigned long none)
{
	unsigned long addr;
	addr = pli_malloc(size, GFP_DCU1);
	if (addr == INVALID_VAL) {
		return 0;
	} else {
		struct page *page = pfn_to_page(addr >> PAGE_SHIFT);
		void *ptr = NULL;

		if (PageHighMem(page)) {
			rtd_pr_rmm_err("auth: AV cpu should not allocate highmem...\n");
			pli_free(addr);
			return 0;
		}
		ptr = page_address(page);
		dmac_flush_range((void *)ptr, (void *)(ptr+size));
		outer_flush_range(addr, addr+size);

		return addr;
	}
}
EXPORT_SYMBOL(rpc_dvr_malloc);

unsigned long rpc_dvr_free(unsigned long addr, unsigned long none)
{
	pli_free(addr);
	return 0;
}
EXPORT_SYMBOL(rpc_dvr_free);
#endif

/* 
 * 0: do alignment fail
 * non-zero: do alignment success 
 */
unsigned long dvr_memory_alignment(unsigned long in_addr, size_t size)
{
#ifndef CONFIG_BW_96B_ALIGNED
    return in_addr;
#else
#if defined (CONFIG_RTK_KDRIVER_SUPPORT) && defined (ENABLE_DC_API_SUPPORT)
    RTK_DC_INFO_t __maybe_unused dc_info;
    RTK_DC_RET_t __maybe_unused ret;
    RTK_DC_BW_TYPE_t __maybe_unused bw_type;
#endif
    unsigned long addr, phy_addr;

    unsigned int N_BYTES = 96;
    bool swap_en = 0;
    bool is_1ddr = 0; // 0: 2 or 3 ddr region
    int sw_mode = 0;  // sw:1 , hw:0
    unsigned long boundary;
    unsigned long delta; // offset
    unsigned int is_va = 0;
    delta = 0;

    phy_addr = in_addr;

	if ((in_addr > PAGE_OFFSET) && (in_addr < (unsigned long)high_memory)) {
        is_va = 1;
        phy_addr = __pa((void *)in_addr);
        rtd_pr_rmm_info("96b alignment: va(%lx)/pa(%lx) convert\n", in_addr, phy_addr);
	} else if (is_vmalloc_addr((void *)in_addr)) {
        is_va = 1;
        phy_addr = dvr_vmalloc_to_phys((void *)in_addr);
        rtd_pr_rmm_info("96b alignment: va(%lx)/pa(%lx) convert\n", in_addr, phy_addr);
	}

#if defined (CONFIG_RTK_KDRIVER_SUPPORT) && defined (ENABLE_DC_API_SUPPORT)
	{
    ret = rtk_dc_get_dram_info(&dc_info);
    if (ret == RTK_DC_RET_FAIL) {
        // rtd_pr_rmm_err("warning, 96byte alignment fail, qos driver enabled?\n");
        // addr = in_addr;
        // goto err_out;
        N_BYTES = 16;
		goto do_alignment;
    }

    swap_en = (bool)dc_info.swap_en;
    sw_mode = (bool)dc_info.sw_mode;
    delta = (unsigned long)dc_info.sw_offset;
    boundary = (unsigned long)dc_info.boundary;

    switch (dc_info.dram_num) {
    case 3:
//        N_BYTES = 16 * dc_info.dram_num;
        N_BYTES = 96; // LCM(32, 48) , lowest common multiple
        break;

	case 2:
    default:
        N_BYTES = 1;
        break;
    }

	/* dual-mc unbalance ddr system only, no need to align */
	if (dc_info.dram_num != 3) { 
//		addr = in_addr;
		sw_mode = 0;
		is_1ddr = 0;
		N_BYTES = 96;
		goto do_alignment;
	}

    // notice! this delta should use memblock_reserve in the last memory zone
    if (sw_mode && swap_en) {
        addr = boundary + ((phy_addr - boundary) + delta);
        bw_type = rtk_dc_query_dram_region_by_addr(addr);
    } else 
        bw_type = rtk_dc_query_dram_region_by_addr(phy_addr);

    if (bw_type == RTK_DC_QUERY_FAIL) {
        rtd_pr_rmm_err("warning, 96byte alignment query fail, qos driver enabled?\n");
        addr = in_addr;
        goto err_out;
    }
    is_1ddr = bw_type;
	}
#else
    addr = in_addr;
    goto err_out;
#endif

do_alignment:
    if (sw_mode) {
        if (swap_en) {
            if (!is_1ddr) {
                unsigned long addr_1 = addr - boundary;
                addr = boundary + ALIGN_NBYTES(addr_1, N_BYTES); // n_bytes align w/ delta
            } else { // in 1ddr region
                addr = phy_addr; 
            }
        } else {
            if (!is_1ddr) {
                addr = ALIGN_NBYTES(phy_addr, N_BYTES); // n_bytes align
            } else { // in 1ddr region
                addr = phy_addr;
            }
        }
    } else { // hardware mode
        if (!is_1ddr) {
            addr = ALIGN_NBYTES(phy_addr, N_BYTES);
        } else { // in 1ddr region
            addr = phy_addr;
        }
    }

#if 0 // FIXME! shoule lookup the carvedout_buf idx and avoid the over size access
    if ((addr + size) > (phy_addr + size)) {
        rtd_pr_rmm_err("dvr_memory_alignment fail, over size access(%lx/%lx/%x), config(%x/%x/%lx/%lx)\n", 
               addr, phy_addr, size, sw_mode, swap_en, boundary, delta);
        addr = 0;
    }
#endif

    if (is_va) {
        unsigned long offset = addr - phy_addr;
        addr = in_addr + offset;
    }

//    rtd_pr_rmm_info("dvr_memory_alignment (%lx/%lx/%x), N_BYTES(%d) config(%d/%d/%d/%d/%d)\n", 
//            addr, in_addr, size, N_BYTES, sw_mode, swap_en, boundary, delta, is_1ddr);

err_out:

    return addr;
#endif
}
EXPORT_SYMBOL(dvr_memory_alignment);

inline unsigned long dvr_size_alignment(unsigned long val)
{
#ifndef CONFIG_BW_96B_ALIGNED
    return val;
#else
    unsigned int N_BYTES = 96;
#if defined (CONFIG_RTK_KDRIVER_SUPPORT) && defined (ENABLE_DC_API_SUPPORT)
    RTK_DC_INFO_t __maybe_unused dc_info;
    RTK_DC_RET_t ret;

    ret = rtk_dc_get_dram_info(&dc_info);
    if (ret == RTK_DC_RET_FAIL) {
        // rtd_pr_rmm_err("warning, 96byte alignment fail, qos driver enabled?\n");
		// return val;
		N_BYTES = 16;
		return ALIGN_NBYTES(val, N_BYTES);
    }

	/* dual-mc unbalance ddr system only, no need to align */
//	if (dc_info.dram_num != 3)
//		return val;

//    if (IS_ALIGNED_NBYTES(val, N_BYTES))
//        return val;

    // we need to align extra if already alignment by itself
    // this is avoid over-access when doing start_addr alignment
    return ALIGN_NBYTES(val, N_BYTES);
#else
	return val;
#endif
#endif
}
EXPORT_SYMBOL(dvr_size_alignment);

inline unsigned long dvr_size_alignment_ex(unsigned long val, unsigned int n_bytes)
{
    if (IS_ALIGNED_NBYTES(val, n_bytes))
        return val;

    return ALIGN_NBYTES(val, n_bytes);
}
EXPORT_SYMBOL(dvr_size_alignment_ex);

/* Check size if 4K page alignment,
 * this is for 4K size align dynamic allocation usage.
 */
unsigned long dvr_size_alignment_cma(unsigned long val)
{
#ifndef CONFIG_BW_96B_ALIGNED
    return val;
#else
#define LINUX_PAGE_SIZE 4096
    unsigned int N_BYTES = 96;
    unsigned int multiply = PAGE_ALIGNED(val) ? ((unsigned int)val / LINUX_PAGE_SIZE) : ((unsigned int)val / LINUX_PAGE_SIZE) + 1;

#if defined (CONFIG_RTK_KDRIVER_SUPPORT) && defined (ENABLE_DC_API_SUPPORT)
    RTK_DC_INFO_t __maybe_unused dc_info;
    RTK_DC_RET_t ret;

    ret = rtk_dc_get_dram_info(&dc_info);
    if (ret == RTK_DC_RET_FAIL) {
        // rtd_pr_rmm_err("warning, 96byte alignment fail, qos driver enabled?\n");
		// return val;
		N_BYTES = 16;
    }

	/* dual-mc unbalance ddr system only, no need to align */
//	if (dc_info.dram_num != 3)
//		return val;

    if (IS_ALIGNED_NBYTES(val, LINUX_PAGE_SIZE) && IS_ALIGNED_NBYTES(val, N_BYTES)) {
        return ALIGN_NBYTES((val + N_BYTES), N_BYTES); // extra
    } else if (val <  ((((LINUX_PAGE_SIZE * multiply) / N_BYTES)    ) * N_BYTES) && 
               val >= ((((LINUX_PAGE_SIZE * multiply) / N_BYTES) - 1) * N_BYTES) ) {
        /* if input val is very close to page_size after aligned, 
         * it would be trucate to page size by cma_alloc.
         */
        return ALIGN_NBYTES((val + N_BYTES), N_BYTES); // extra
    }

    return ALIGN_NBYTES(val, N_BYTES);
#else
	return val;
#endif
#endif
}
EXPORT_SYMBOL(dvr_size_alignment_cma);


static void debug_cma_deferfree_param(char *buf)
{
	static const char *delim = ", ";
	char *token = NULL;
	int value;

	rtd_pr_rmm_debug("[cma deferfree] param \"%s\"\n", buf);

	// on(/off),hard(/soft),(99999)
	token = strsep(&buf, delim); // on(/off)
	if (strcmp(token, "on") == 0)
	{
		_debug_cma_deferfree_enabled = true;
		rtd_pr_rmm_notice("[cma deferfree] cma deferfree is enabled !\n");

		token = strsep(&buf, delim); // hard(/soft)
		if (token == NULL)
			goto end;

		if (strcmp(token, "hard") == 0) {
			_debug_cma_deferfree_hard = true;
		}

		token = strsep(&buf, delim);
		if (token && kstrtoint(token, 0, &value)==0) {
			if (value < cma_debug_workq_delay) { // delay is not supposed to be less than polling thread interval
				cma_deferfree_delay = cma_debug_workq_delay;
			}
			else {
				cma_deferfree_delay = value;
			}
		}
	}
	// global _debug_cma_deferfree_enabled is default 0(false)

end:
	rtd_pr_rmm_info("[cma deferfree] set %s,%s,%d\n",
		(_debug_cma_deferfree_enabled) ? "on" : "off",
		(_debug_cma_deferfree_hard) ? "hard" : "soft",
		cma_deferfree_delay );

}

static void debug_cma_guardband_param(char *buf)
{
	static const char *delim = ", ";
	char *token = NULL;

	rtd_pr_rmm_debug("[cma guardband] param \"%s\"\n", buf);

	// on(/off),hard(/soft)
	token = strsep(&buf, delim); // on(/off)
	if (strcmp(token, "on") == 0)
	{
		_debug_cma_guardband_enabled = true;
		rtd_pr_rmm_notice("[cma guardband] cma guardband is enabled !\n");

		token = strsep(&buf, delim); // hard(/soft)
		if (token == NULL)
			goto end;

		if (strcmp(token, "hard") == 0) {
			_debug_cma_guardband_hard = true;
		}
    }
	// global _debug_cma_guardband_enabled is default 0(false)


end:
	rtd_pr_rmm_info("[cma guardband] set %s,%s\n",
		(_debug_cma_guardband_enabled) ? "on" : "off",
		(_debug_cma_guardband_hard) ? "hard" : "soft" );

}

#ifndef MODULE

static int __init early_debug_cma_deferfree(char *buf)
{
	if (!buf)
		return -EINVAL;

	debug_cma_deferfree_param(buf);

	return 0;
}
early_param("debug_cma_deferfree", early_debug_cma_deferfree);

static int __init early_debug_cma_guardband(char *buf)
{
	if (!buf)
		return -EINVAL;

	debug_cma_guardband_param(buf);

	return 0;
}
early_param("debug_cma_guardband", early_debug_cma_guardband);

#else

void debug_cma_deferfree_set(void)
{
	char buf[20] = {0}; // "on(/off),hard(/soft),99999"

	if (rtk_parse_commandline_equal("debug_cma_deferfree", buf, sizeof(buf)) == 0)
	{
		rtd_pr_rmm_err("Error : can't get debug_cma_deferfree from bootargs\n");
		return;
	}

	debug_cma_deferfree_param(buf);
}

void debug_cma_guardband_set(void)
{
	char buf[20] = {0}; // "on(/off),hard(/soft)"

	if (rtk_parse_commandline_equal("debug_cma_guardband", buf, sizeof(buf)) == 0)
	{
		rtd_pr_rmm_err("Error : can't get debug_cma_guardband from bootargs\n");
		return;
	}

	debug_cma_guardband_param(buf);
}

void early_param_init(void)
{
	debug_cma_deferfree_set();
	debug_cma_guardband_set();
}

#endif

bool need_debug_cma_deferfree(void)
{
	if (!_debug_cma_deferfree_enabled)
		return false;

	return true;
}

bool need_debug_cma_guardband(void)
{
	if (!_debug_cma_guardband_enabled)
		return false;

	return true;
}

/* --- cma defer free --- */
void dvr_defer_free(rtk_record *ptr)
{
	int count;
	unsigned long value, phys_addr;

	if (!unlikely(ptr))
		return;

	value = ptr->type;
	phys_addr = ptr->addr;

	if ((value & 0xf0000000) != RTK_SIGNATURE) {
		rtd_pr_rmm_err("remap: defer free memory (%lx) signature error...\n", phys_addr);
		BUG();
	} else {
		/* move to rtkrecord.c, due to spinlock */
		//rtk_record_tag_clear(phys_addr, RTK_RECORD_TAG_DEFER_FREE);
		ptr->reserved = ptr->reserved & 0xfffffff0; // defer free clear

		if (((value & ID_MASK) == BUDDY_ID) || ((value & ID_MASK) == BUDDY_MESG_ID) ||
			((value & ID_MASK) == DRIVER_ID) || ((value & ID_MASK) == DRIVER_MESG_ID) || ((value & ID_MASK) == NOSAVE_ID)) {
			count = value & 0x00ffffff;

			if (dvr_deferfree_check(phys_addr)) {
				if (((value & ID_MASK) == BUDDY_MESG_ID))
					rtd_pr_rmm_err("[cma deferfree] id: %lx defer, addr: %lx size: %8lx (%6ld/%6ld) %s\n", (value & ID_MASK),
							phys_addr, (value & 0x00ffffff) << PAGE_SHIFT, ptr->pid, ptr->tgid, (const char *)ptr->caller);
				else
					rtd_pr_rmm_err("[cma deferfree] id: %lx defer, addr: %lx size: %8lx (%6ld/%6ld) %pF\n", (value & ID_MASK),
							phys_addr, (value & 0x00ffffff) << PAGE_SHIFT, ptr->pid, ptr->tgid, (const void *)ptr->caller);
			}

			if (need_debug_cma_guardband()) {
				count = dvr_guardband_size(count << PAGE_SHIFT) >> PAGE_SHIFT;
				//count_order = 1 << get_order(count_page << PAGE_SHIFT);
				//if ( ((count_order - count_page) == 0) && ((count_page << PAGE_SHIFT) < GUARDBAND_SIZE) ) // page is order alignment
				//	count_order = 1 << get_order((count_page + 1) << PAGE_SHIFT); // re-calculate
			}

			free_contiguous_memory(pfn_to_page(phys_addr >> PAGE_SHIFT), count);

			if (((value & ID_MASK) == BUDDY_MESG_ID) && ptr->caller) {
				rtd_pr_rmm_debug("[cma deferfree] pli_mesg_free defer, addr: %lx size: %8lx (%6ld/%6ld) %s\n",
						phys_addr, (value & 0x00ffffff) << PAGE_SHIFT, ptr->pid, ptr->tgid, (const char *)ptr->caller);
				kfree((void *)ptr->caller);
			}
		} else {
			BUG();
		}
	}
}
/* --- cma defer free --- */

/* --- cma guardband --- */

#define DVR_PADDING_BYTE 0x0F


int my_pagecmp(struct page *page, const char val)
{
	char *vaddr, *pdata, *end;
	int ret = 0; // OK

	vaddr = kmap_atomic(page);
	if (vaddr != NULL)
	{
		dmac_inv_range(vaddr, vaddr + PAGE_SIZE);

		// compare page byte by byte
		end = vaddr + PAGE_SIZE;
		for (pdata = vaddr; pdata < end; ++pdata)
			if (*pdata != val)
				break; // found first mismatch

		// dump information nearby
		if (pdata != end)
		{
			unsigned long phy_anchor;
			int j;

			ret = -1; // FAIL
			pdata = (char*)((unsigned long)pdata & ~0xf); // 16 bytes a line
			phy_anchor = (page_to_pfn(page) << PAGE_SHIFT) + (pdata - vaddr); // phy base

			for (j = 0; j < 8; j += 4) // first 2 items
				rtd_pr_rmm_err("%08x: %08x %08x %08x %08x\n", (int)(phy_anchor + (j*4)),
					*(int *)(((int *)(pdata)) + j + 0),
					*(int *)(((int *)(pdata)) + j + 1),
					*(int *)(((int *)(pdata)) + j + 2),
					*(int *)(((int *)(pdata)) + j + 3) );
			rtd_pr_rmm_err("--------  -------- -------- -------- --------\n");
		}

		kunmap_atomic(vaddr);
	}

	return ret;
}

/*
 * in : size is page aligned
 * out: size is order aligned
 */

int dvr_guardband_size(size_t size_page)
{
	int size_order;
	int count_page, count_order;

	count_page = size_page >> PAGE_SHIFT;

	count_order = 1 << get_order(size_page);

	if ((count_order - count_page) == 0 && size_page < GUARDBAND_SIZE)   // page is order alignment
	{
		count_page += 1; // extra 1 page

		// re-calculate
		count_order = 1 << get_order(count_page << PAGE_SHIFT);
	}

	size_order = count_order << PAGE_SHIFT;

	rtd_pr_rmm_debug("[cma guardband] size alignment calc page(%lx)/order(%x)\n", (uintptr_t)size_page, size_order);

	return size_order;
}


int dvr_guardband_padding(struct page* page, int count_order, int count_page)
{
	if (count_order == 0)
		return (int)INVALID_VAL;

	if (page)
	{
		int i = 0;
		unsigned long start_pfn = page_to_pfn(page);

		for (; i < count_order - count_page; i++)
		{
			struct page *page = pfn_to_page(start_pfn + count_page + i);
			char *vaddr = kmap_atomic(page);

			if (vaddr != NULL) {
				memset(vaddr, DVR_PADDING_BYTE, PAGE_SIZE);
				rtd_pr_rmm_debug("%08x: %08x %08x %08x %08x\n", (int)(start_pfn << PAGE_SHIFT),
					*(int *)(((int *)(vaddr)) + 0),
					*(int *)(((int *)(vaddr)) + 1),
					*(int *)(((int *)(vaddr)) + 2),
					*(int *)(((int *)(vaddr)) + 3) );
				dmac_flush_range(vaddr, vaddr + PAGE_SIZE);
				kunmap_atomic(vaddr);
			}
		}
	}

	return 0;
}

// simple version dvr_guardband_check to the head of memmory.
int dvr_deferfree_check(unsigned long phys_addr)
{
	unsigned long start_pfn = phys_addr >> PAGE_SHIFT;
	struct page *page = pfn_to_page(start_pfn);

	rtd_pr_rmm_debug("[cma deferfree] addr(%lx)\n", phys_addr);

	if (my_pagecmp(page, DVR_PADDING_BYTE))
	{
		rtd_pr_rmm_err("[cma deferfree] memory trash (addr: 0x%08lx)\n\n", phys_addr);

		if (_debug_cma_deferfree_hard) {
			panic("(deferfree)dvr use-after-free 0x%08lx\n", phys_addr);
		}
		else {
			dump_stack();
		}

		return -EFAULT;
	}

	return 0;
}


int dvr_guardband_check(unsigned long phys_addr, int value)
{
	int count_page, count_order, count_check;

	int i = 0, my_ret = 0;
	unsigned long start_pfn = phys_addr >> PAGE_SHIFT;

	count_page = value & 0x00ffffff;

	//count_order = 1 << get_order(count_page << PAGE_SHIFT);
	//if ( ((count_order - count_page) == 0) && ((count_page << PAGE_SHIFT) < GUARDBAND_SIZE) ) // page is order alignment
	//	count_order = 1 << get_order((count_page + 1) << PAGE_SHIFT); // re-calculate
	count_order = dvr_guardband_size(count_page << PAGE_SHIFT) >> PAGE_SHIFT;

	count_check = count_order - count_page;
	rtd_pr_rmm_debug("[cma guardband] addr(%lx), size alignment check page(%x)/order(%x)\n", phys_addr, (count_page << PAGE_SHIFT), (count_order << PAGE_SHIFT));

	for (i = 0; i < count_check; i++)
	{
		struct page *page = pfn_to_page(start_pfn + count_page + i);

		if (my_pagecmp(page, DVR_PADDING_BYTE)) {
			if (++my_ret >= 5)
				break;
		}
	}

#if 0
	// dump last 32bytes within last page of normal request.
	if (my_ret && count_check >= 1)
	{
		struct page *page = pfn_to_page(start_pfn + count_page - 1);
		char *vaddr = kmap_atomic(page);
		if (vaddr != NULL) {
			int j = 0;
			rtd_pr_rmm_err("-------- last 32 bytes dump -------\n");
			for (j = ((PAGE_SIZE/4) - 8); j < (PAGE_SIZE/4); j+=4) // last 2 items
				rtd_pr_rmm_err("%08x: %08x %08x %08x %08x\n", (int)((start_pfn + count_page - 1) << PAGE_SHIFT),
					   *(int *)(((int *)(vaddr)) + j + 0),
					   *(int *)(((int *)(vaddr)) + j + 1),
					   *(int *)(((int *)(vaddr)) + j + 2),
					   *(int *)(((int *)(vaddr)) + j + 3));

			rtd_pr_rmm_err("-------- -------- -------- --------\n");
			kunmap_atomic(vaddr);
		}
	}
#endif

	if (my_ret) {
		rtd_pr_rmm_err("[cma guardband] memory trash (addr: 0x%08lx, size: page(0x%x)/order(0x%x))\n\n",
			phys_addr, (count_page << PAGE_SHIFT), (count_order << PAGE_SHIFT));

		if (_debug_cma_guardband_hard) {
			panic("(guardband)dvr out-of-bounds 0x%08lx\n", phys_addr);
		}
		else {
			dump_stack();
		}
	}

	return count_order;
}
/* --- cma guardband --- */

#ifndef CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE
/*
 * Restrictions:
 * (1) Must be called with an elevated refcount on the page.
 * (2) The lru_locked mus be held.
 */
static int isolate_lru_page_locked(struct page *page)
{
	int ret = -EBUSY;

	BUG_ON(!PageLRU(page));

	// check if this page is in pagevec_release()...
	if (likely(get_page_unless_zero(page))) {
		ClearPageLRU(page);
		ret = 0;
	}

	return ret;
}

static struct page *remap_alloc_page(struct page *p, unsigned long private)
{
#ifdef CONFIG_QUICK_HIBERNATION
	int in_suspend = (private & REMAP_IN_SUSPEND) ? 1 : 0;
#endif
	private &= 0xff;
	if (private == REMAP_ZONE) {
		return alloc_page(GFP_HIGHUSER_MOVABLE);
	} else if (private == REMAP_PAGE) {
		return alloc_page(GFP_KERNEL);
#ifdef CONFIG_CMA_FIRST_POLICY
	} else if (private == REMAP_CMA1) {
#ifdef CONFIG_QUICK_HIBERNATION
		if (!in_suspend) {
#endif
			return alloc_page(GFP_HIGHUSER_MOVABLE);
#ifdef CONFIG_QUICK_HIBERNATION
		} else {
			struct page *page = NULL;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 7, 0))
			if (page_is_file_cache(p)) {
#else
			if (page_is_file_lru(p)) {
#endif
				return alloc_page(GFP_HIGHUSER_MOVABLE);
			}

			/* anonymous page */
			if (!curr_record) {
				return alloc_page(GFP_HIGHUSER_MOVABLE);
			}

			page = &pagelist[page_cnt++];
			get_page(page);
			if (page_cnt == page_max) {
				curr_record = list_entry(curr_record->list.next, record_struct, list);
				if (&curr_record->list == &dcu1_record_list) {
					rtd_pr_rmm_info("swsusp: run out of CMA1 preallocated pages....\n");
					curr_record = pagelist = NULL;
					return page;
				}
				pagelist = curr_record->page;
				page_max = MEM_CHUNK_SIZE / PAGE_SIZE;
				page_cnt = 0;
			}

			return page;
		}
#endif
	} else if (private == REMAP_CMA2) {
#ifdef CONFIG_QUICK_HIBERNATION
		if (!in_suspend) {
#endif
			return alloc_page(GFP_HIGHUSER_MOVABLE);
#ifdef CONFIG_QUICK_HIBERNATION
		} else {
			struct page *page = NULL;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 7, 0))
			if (page_is_file_cache(p)) {
#else
			if (page_is_file_lru(p)) {
#endif
				return alloc_page(GFP_HIGHUSER_MOVABLE);
			}

			/* anonymous page */
			if (!curr_record) {
				return alloc_page(GFP_HIGHUSER_MOVABLE);
			}

			page = &pagelist[page_cnt++];
			get_page(page);
			if (page_cnt == page_max) {
				curr_record = list_entry(curr_record->list.next, record_struct, list);
				if (&curr_record->list == &dcu2_record_list) {
					rtd_pr_rmm_info("swsusp: run out of CMA2 preallocated pages....\n");
					curr_record = pagelist = NULL;
					return page;
				}
				pagelist = curr_record->page;
				page_max = MEM_CHUNK_SIZE / PAGE_SIZE;
				page_cnt = 0;
			}

			return page;
		}
#endif
#endif
	} else {
		BUG();
	}
}

static int __maybe_unused remap_zone_pages(struct zone *zone, page_filter_t filter, int type)
{
	pg_data_t *pgdat = zone->zone_pgdat;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0))
	struct lruvec __maybe_unused *__lruvec = &pgdat->lruvec;
#else
	struct lruvec *__lruvec = &pgdat->__lruvec;
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	spinlock_t *lru_lock = &pgdat->lru_lock;
#else
	spinlock_t *lru_lock = &__lruvec->lru_lock;
#endif
	struct list_head *list;
	LIST_HEAD(page_list);
	int lru_done = 0, lru_fail = 0;
	int map_done = 0, map_fail = 0;
	int lru, ret;

	lru_add_drain_all();		/* flush pagevecs */

	spin_lock_irq(lru_lock);
	for_each_lru(lru) {
		struct page *page, *page2;
		list = &(__lruvec->lists[lru - LRU_BASE]);
		list_for_each_entry_safe(page, page2, list, lru) {
			if (filter && !filter(page))
				continue;

			if (isolate_lru_page_locked(page) == -EBUSY) {
				lru_fail++;
			} else {
				lru_done++;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0))
				del_page_from_lru_list(page, mem_cgroup_page_lruvec(page, zone->zone_pgdat), lru);
#else
				del_page_from_lru_list(page, mem_cgroup_page_lruvec(page));
#endif
				list_add_tail(&page->lru, &page_list);
				__mod_zone_page_state(zone, 
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 7, 0))
					NR_ISOLATED_ANON + page_is_file_cache(page), 
#else
					NR_ISOLATED_ANON + page_is_file_lru(page), 
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0))
					hpage_nr_pages(page)
#else
					thp_nr_pages(page)
#endif
					);
			}
		}
		rtd_pr_rmm_info("lru: %d\t\t done: %d\t fail: %d\n", lru, lru_done, lru_fail);
		map_done += lru_done;
		map_fail += lru_fail;
		lru_done = lru_fail = 0;
	}
	spin_unlock_irq(lru_lock);
	rtd_pr_rmm_info("total isolated:\t done: %d\t fail: %d\n", map_done, map_fail);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
	ret = migrate_pages(&page_list, remap_alloc_page, NULL, type, MIGRATE_SYNC, MR_CONTIG_RANGE);
#else
	ret = migrate_pages(&page_list, remap_alloc_page, NULL, type, MIGRATE_SYNC, MR_CONTIG_RANGE, NULL);
#endif
	map_done -= ret;
	map_fail += ret;
	rtd_pr_rmm_info("total migrated:\t done: %d\t fail: %d\n", map_done, ret);
	putback_movable_pages(&page_list);

	// flush again to ensure all delayed works in putback_lru_page() are done...
	lru_add_drain_all();		/* flush pagevecs */
	drain_all_pages(NULL);	/* flush per_cpu_pages */

	if (map_fail)
		return 1;
	else
		return 0;
}

int remap_one_page(struct page *page)
{
	struct zone *zone = (struct zone *)page_zone(page);
	pg_data_t *pgdat = page_pgdat(page);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0))
	struct lruvec __maybe_unused *__lruvec = &pgdat->lruvec;
#else
	struct lruvec *__lruvec = &pgdat->__lruvec;
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	spinlock_t *lru_lock = &pgdat->lru_lock;
#else
	spinlock_t *lru_lock = &__lruvec->lru_lock;
#endif
	LIST_HEAD(page_list);
	int lru, ret;

	lru_add_drain_all();		/* flush pagevecs */

	spin_lock_irq(lru_lock);
	if (!PageLRU(page)) {
		spin_unlock_irq(lru_lock);
		rtd_pr_rmm_info("remap: page %p (%p) might be isolated or reclaimed...\n", page, page->mapping);
		msleep(10);
		return 1;
	}
	lru = page_lru(page);
	if (isolate_lru_page_locked(page) == -EBUSY) {
		spin_unlock_irq(lru_lock);
		rtd_pr_rmm_info("remap: fail to isolate pfn %lx...\n", page_to_pfn(page));
		return 1;
	} else {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 12, 0))
		del_page_from_lru_list(page, mem_cgroup_page_lruvec(page, zone->zone_pgdat), lru);
#else
		del_page_from_lru_list(page, mem_cgroup_page_lruvec(page));
#endif
		list_add_tail(&page->lru, &page_list);
		__mod_zone_page_state(zone, 
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 7, 0))
			NR_ISOLATED_ANON + page_is_file_cache(page), 
#else
			NR_ISOLATED_ANON + page_is_file_lru(page), 
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0))
			hpage_nr_pages(page)
#else
			thp_nr_pages(page)
#endif
			);
	}
	spin_unlock_irq(lru_lock);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
	ret = migrate_pages(&page_list, remap_alloc_page, NULL, REMAP_PAGE, MIGRATE_SYNC, MR_CONTIG_RANGE);
#else
	ret = migrate_pages(&page_list, remap_alloc_page, NULL, REMAP_PAGE, MIGRATE_SYNC, MR_CONTIG_RANGE, NULL);
#endif
	rtd_pr_rmm_info("remap: remap pfn %lx return %d\n", page_to_pfn(page), ret);
	putback_movable_pages(&page_list);

	if (ret)
		return 1;
	else
		return 0;
}

#ifdef CONFIG_QUICK_HIBERNATION
int reserve_highmem(void)
{
#ifdef CONFIG_HIGHMEM
	struct zone *zone = &NODE_DATA(0)->node_zones[ZONE_HIGHMEM];

	if (zone && populated_zone(zone)) {
		set_bit(PGDAT_ALLOC_LOCKED, &zone->zone_pgdat->flags);
		rtd_pr_rmm_info("remap HIGHMEM zone...\n");
		remap_zone_pages(zone, NULL, REMAP_ZONE);
	}
#endif

	return 0;
}
EXPORT_SYMBOL(reserve_highmem);

int release_highmem(void)
{
#ifdef CONFIG_HIGHMEM
	struct zone *zone = &NODE_DATA(0)->node_zones[ZONE_HIGHMEM];

	if (zone && populated_zone(zone)) {
		clear_bit(PGDAT_ALLOC_LOCKED, &zone->zone_pgdat->flags);
	}
#endif

	return 0;
}
EXPORT_SYMBOL(release_highmem);

int reserve_movable(void)
{
	struct zone *zone;

	if (down_interruptible(&remap_sem)) {
		// we use return value 1 to represent an signal is caught...
		rtd_pr_rmm_info("remap: %s interrupted by an signal...\n", __func__);
		return -EINTR;
	}

	zone = &NODE_DATA(0)->node_zones[ZONE_MOVABLE];
	set_bit(PGDAT_ALLOC_LOCKED, &zone->zone_pgdat->flags);
	rtd_pr_rmm_info("remap MOVABLE zone...\n");
	remap_zone_pages(zone, NULL, REMAP_ZONE);

	return 0;
}
EXPORT_SYMBOL(reserve_movable);

int release_movable(void)
{
	struct zone *zone = &NODE_DATA(0)->node_zones[ZONE_MOVABLE];

	clear_bit(PGDAT_ALLOC_LOCKED, &zone->zone_pgdat->flags);
	up(&remap_sem);

	return 0;
}
EXPORT_SYMBOL(release_movable);
#endif
#endif

#ifdef CONFIG_CMA_FIRST_POLICY
static bool cma_page_filter(struct page *page)
{
	if (check_cma_memory(page_to_pfn(page)))
		return true;
	else
		return false;
}

int reserve_cma(int in_suspend)
{
	struct zone *zone;

	if (down_interruptible(&remap_sem)) {
		// we use return value 1 to represent an signal is caught...
		rtd_pr_rmm_info("remap: %s interrupted by an signal...\n", __func__);
		return -EINTR;
	}

	cma_reserved = 1;
#ifdef CONFIG_QUICK_HIBERNATION
	if (in_suspend) {
		if (dcu1_size) {
			rtd_pr_rmm_info("remap DCU1 CMA...\n");
			if (!list_empty(&dcu1_record_list)) {
				curr_record = list_entry(dcu1_record_list.next, record_struct, list);
				pagelist = curr_record->page;
				page_max = MEM_CHUNK_SIZE / PAGE_SIZE;
				page_cnt = 0;
			} else {
				curr_record = pagelist = 0;
				page_cnt = page_max = 0;
			}
			zone = &NODE_DATA(0)->node_zones[ZONE_NORMAL];
			remap_zone_pages(zone, cma_page_filter, REMAP_IN_SUSPEND | REMAP_CMA1);
			if (pagelist) {
				char *vaddr;
				while (page_cnt < page_max) {
					vaddr = kmap_atomic(&pagelist[page_cnt++]);
					memset(vaddr, 0, PAGE_SIZE);
					kunmap_atomic(vaddr);
				}
			}
		}
		if (dcu2_size) {
			rtd_pr_rmm_info("remap DCU2 CMA...\n");
			if (!list_empty(&dcu2_record_list)) {
				curr_record = list_entry(dcu2_record_list.next, record_struct, list);
				pagelist = curr_record->page;
				page_max = MEM_CHUNK_SIZE / PAGE_SIZE;
				page_cnt = 0;
			} else {
				curr_record = pagelist = 0;
				page_cnt = page_max = 0;
			}
			zone = &NODE_DATA(0)->node_zones[ZONE_MOVABLE];
			remap_zone_pages(zone, cma_page_filter, REMAP_IN_SUSPEND | REMAP_CMA2);
			if (pagelist) {
				char *vaddr;
				while (page_cnt < page_max) {
					vaddr = kmap_atomic(&pagelist[page_cnt++]);
					memset(vaddr, 0, PAGE_SIZE);
					kunmap_atomic(vaddr);
				}
			}
		}
	} else {
#endif
		if (dcu1_size) {
			rtd_pr_rmm_info("remap DCU1 CMA(lowmem)...\n");
			zone = &NODE_DATA(0)->node_zones[ZONE_NORMAL];
			remap_zone_pages(zone, cma_page_filter, REMAP_CMA1);
		}
		if (dcu2_size) {
			rtd_pr_rmm_info("remap DCU2 CMA(highmem)...\n");
			zone = &NODE_DATA(0)->node_zones[ZONE_MOVABLE];
			remap_zone_pages(zone, cma_page_filter, REMAP_CMA2);
		}
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
		if (highmem_bw_size) {
			rtd_pr_rmm_info("remap HIGHMEM_BW CMA(highmem_bw)...\n");
			zone = &NODE_DATA(0)->node_zones[ZONE_MOVABLE];
			remap_zone_pages(zone, cma_page_filter, REMAP_BW_CMA);
		}
#endif
#ifdef CONFIG_QUICK_HIBERNATION
	}
#endif

	return 0;
}
EXPORT_SYMBOL(reserve_cma);

int release_cma(int in_suspend)
{
	cma_reserved = 0;
	up(&remap_sem);

	return 0;
}
EXPORT_SYMBOL(release_cma);
#endif

int remap_list_mem(rtk_record *records)
{
	int ret;
	unsigned long addr = records->addr;
	unsigned long  type = records->type;
	const void *caller = records->caller;
	unsigned long  pid = records->pid; // process id
	unsigned long  tid = records->tgid; // thread id
	// timestamps
	unsigned long jiff = records->jiffies;
	unsigned long msec = jiffies_to_msecs(jiff);
	unsigned int defer = records->reserved & 0x0000000f;

	if ((type & 0xf0000000) != RTK_SIGNATURE) {
		rtd_pr_rmm_err("remap: list memory (%lx) signature error...\n", addr);
		return 0;
	}

	if ((type & ID_MASK) == DRIVER_ID) {
		rtd_pr_rmm_notice(" allocation address(%d): %08lx  size: %8lx (%6ld/%6ld)(by driver) %pS (%ld.%03ld sec)\n", defer, addr, (type & 0x00ffffff) << PAGE_SHIFT, pid, tid, caller, msec / 1000, msec % 1000);
		ret = (type & 0x00ffffff) << PAGE_SHIFT;
	} else if ((type & ID_MASK) == NOSAVE_ID) {
		rtd_pr_rmm_notice(" allocation address(%d): %08lx  size: %8lx (%6ld/%6ld)(by nosave) %pS (%ld.%03ld sec)\n", defer, addr, (type & 0x00ffffff) << PAGE_SHIFT, pid, tid, caller, msec / 1000, msec % 1000);
		ret = (type & 0x00ffffff) << PAGE_SHIFT;
	} else if ((type & ID_MASK) == CACHE_ID) {
		rtd_pr_rmm_notice(" allocation address(%d): %08lx  size: %8lx (%6ld/%6ld)(by pli cache) %pS (%ld.%03ld sec)\n", defer, addr, type & 0x00ffffff, pid, tid, caller, msec / 1000, msec % 1000);
		ret = type & 0x00ffffff;
	} else if ((type & ID_MASK) == BUDDY_ID) {
		rtd_pr_rmm_notice(" allocation address(%d): %08lx  size: %8lx (%6ld/%6ld)(by pli buddy) %pS (%ld.%03ld sec)\n", defer, addr, (type & 0x00ffffff) << PAGE_SHIFT, pid, tid, caller, msec / 1000, msec % 1000);
		ret = (type & 0x00ffffff) << PAGE_SHIFT;
	} else if ((type & ID_MASK) == BUDDY_MESG_ID) {
		rtd_pr_rmm_notice(" allocation address(%d): %08lx  size: %8lx (%6ld/%6ld)(by pli buddy for user layer) %s (%ld.%03ld sec)\n", defer, addr, (type & 0x00ffffff) << PAGE_SHIFT, pid, tid, (const char *)caller, msec / 1000, msec % 1000);
		ret = (type & 0x00ffffff) << PAGE_SHIFT;
	} else if ((type & ID_MASK) == DRIVER_MESG_ID) {
		rtd_pr_rmm_notice(" allocation address(%d): %08lx  size: %8lx (%6ld/%6ld)(by driver) %s (%ld.%03ld sec)\n", defer, addr, (type & 0x00ffffff) << PAGE_SHIFT, pid, tid, (const char *)caller, msec / 1000, msec % 1000);
		ret = (type & 0x00ffffff) << PAGE_SHIFT;
	}
	else {
		rtd_pr_rmm_err("Unknown memory ID...\n");
		return 0;
	}

	return ret;
}

int remap_free_mem(rtk_record *records)
{
	unsigned long addr = records->addr;
	unsigned long type = records->type;

	if ((type & 0xf0000000) != RTK_SIGNATURE) {
		rtd_pr_rmm_err("remap: free memory (%lx) signature error...\n", addr);
		return 1;
	}

	if ((type & ID_MASK) == DRIVER_ID) {
		rtd_pr_rmm_notice("\t0x%lx owned by module...\n", addr);
		return 1;
	} else if ((type & ID_MASK) == NOSAVE_ID) {
		rtd_pr_rmm_notice("\t0x%lx owned by nosave...\n", addr);
		return 1;
	} else if ((type & ID_MASK) == CACHE_ID) {
		kfree((void *)addr);
	} else if ((type & ID_MASK) == BUDDY_ID) {
		int count = type & 0x00ffffff;
		free_contiguous_memory(pfn_to_page(addr >> PAGE_SHIFT), count);
	} else {
		rtd_pr_rmm_notice("Unknown memory ID...\n");
		return 1;
	}

	return 0;
}

unsigned long get_page_cache_addr(void)
{
	int pageno = 0;
	char *vaddr;

    int dev_idx = DEV_CMA1_DEFAULT;
    struct page *cache = NULL;
    unsigned long *cache_bitmap = NULL;

    cache = rtkcma_list[dev_idx].cache;
    cache_bitmap = rtkcma_list[dev_idx].cache_bitmap;

	if (!cache) {
		return INVALID_VAL;
	}

	/* clear unused page cache */
	while (1) {
		pageno = bitmap_find_next_zero_area(cache_bitmap, DEF_CACHE_SIZE / PAGE_SIZE, pageno, 1, 0);
		if (pageno >= DEF_CACHE_SIZE / PAGE_SIZE)
			break;
//		rtd_pr_rmm_info("remap: clear pageno: %d \n", pageno);
		vaddr = kmap_atomic(cache + pageno);
		memset(vaddr, 0, PAGE_SIZE);
		kunmap_atomic(vaddr);
		pageno++;
	}

	return page_to_pfn(cache) << PAGE_SHIFT;
}

void show_cma_avaliable(void)
{
    int dev_idx = 0;

    for (dev_idx = DEV_CMA1_DEFAULT; dev_idx < DEV_CMA_NUM; dev_idx++) {
        struct device *dev = ERR_PTR(INVALID_VAL);
        const char *name = NULL;
        struct page *cache = NULL;
        unsigned long *cache_bitmap = NULL;

        if (!rtkcma_list[dev_idx].exist)
            continue;

        dev = rtkcma_list[dev_idx].dev;
        cache = rtkcma_list[dev_idx].cache;
        cache_bitmap = rtkcma_list[dev_idx].cache_bitmap;
        name = (char *)rtkcma_list[dev_idx].name;

        rtd_pr_rmm_err("\n\t%s:\t cache(%08lx)\n", name, cache ? (page_to_pfn(cache) << PAGE_SHIFT) : INVALID_VAL);

		if (dev_idx == DEV_CMA_GPU_4K)
			dvr_show_bitmap(dev);
#ifdef CONFIG_VBM_CMA
		else if (dev_idx == DEV_CMA_VBM)
			dvr_show_bitmap(dev);
#endif
		else
			show_rtkbp((struct mem_bp *)dma_get_allocator(dev));
    }
}

#ifndef CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE
static bool unmovable_page_filter(struct page *page)
{
	if (check_cma_memory(page_to_pfn(page)))
		return false;
	else
		return true;
}

int calc_unmovable(void)
{
	struct zone *zone = &NODE_DATA(0)->node_zones[ZONE_NORMAL];
	pg_data_t *pgdat = zone->zone_pgdat;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0))
	struct lruvec __maybe_unused *__lruvec = &pgdat->lruvec;
#else
	struct lruvec *__lruvec = &pgdat->__lruvec;
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	spinlock_t *lru_lock = &pgdat->lru_lock;
#else
	spinlock_t *lru_lock = &__lruvec->lru_lock;
#endif
	struct list_head *list;
	LIST_HEAD(page_list);
	int lru, ret;

	lru_add_drain_all();		/* flush pagevecs */

	spin_lock_irq(lru_lock);
	for_each_lru(lru) {
		struct page *page, *page2;
		ret = 0;
		list = &(__lruvec->lists[lru - LRU_BASE]);
		list_for_each_entry_safe(page, page2, list, lru) {
			if (!unmovable_page_filter(page))
				continue;
			ret++;
		}
		rtd_pr_rmm_alert(" ### lru: %d count: %d \n", lru, ret);
	}
	spin_unlock_irq(lru_lock);

	return 0;
}

static unsigned long remap_shrink_count(struct shrinker *s, struct shrink_control *sc)
{
	return 1;
}

/*
 * We will check if system need to do remapping by ourself, so this function always return -1 
 * to notify linux that linux reclaim mechanism does not need to check for us.
 */
static unsigned long remap_shrink(struct shrinker *s, struct shrink_control *sc)
{
	struct zone *zone = &NODE_DATA(0)->node_zones[ZONE_NORMAL];
	pg_data_t *pgdat = zone->zone_pgdat;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0))
	struct lruvec __maybe_unused *__lruvec = &pgdat->lruvec;
#else
	struct lruvec *__lruvec = &pgdat->__lruvec;
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	spinlock_t *lru_lock = &pgdat->lru_lock;
#else
	spinlock_t *lru_lock = &__lruvec->lru_lock;
#endif
	struct list_head *list;
	LIST_HEAD(page_list);
	int lru_done = 0, lru_fail = 0;
	int map_done = 0, map_fail = 0;
	int lru, ret;

//	BUG_ON(sc->nr_to_scan);

	/* we only allow kswpad to do shrink to avoid deadlock */
	if (!(current->flags & PF_KSWAPD))
		goto out;

#ifdef CONFIG_HIGHMEM
	if ((global_zone_page_state(NR_FREE_PAGES) - global_zone_page_state(NR_FREE_CMA_PAGES)
		- zone_page_state(&NODE_DATA(0)->node_zones[ZONE_HIGHMEM], NR_FREE_PAGES)) >= 2048)
#else
	if ((global_zone_page_state(NR_FREE_PAGES) - global_zone_page_state(NR_FREE_CMA_PAGES)
		- zone_page_state(&NODE_DATA(0)->node_zones[ZONE_NORMAL], NR_FREE_PAGES)) >= 2048)
#endif
		goto out;

	if (global_zone_page_state(NR_FREE_CMA_PAGES) <= totalreserve_pages)
		goto out;

	rtd_pr_rmm_info("remap: start to clean unmovable region...\n");
	spin_lock_irq(lru_lock);
	for_each_lru(lru) {
		struct page *page, *page2;

//		if (is_file_lru(lru))
//			continue;

		list = &(__lruvec->lists[lru - LRU_BASE]);
		list_for_each_entry_safe(page, page2, list, lru) {
			if (!unmovable_page_filter(page))
				continue;

			if (isolate_lru_page_locked(page) == -EBUSY) {
				lru_fail++;
			} else {
				/* now we are safe to access this page */
#ifdef CONFIG_EXT4_FS
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 7, 0))
				if (page_is_file_cache(page)) {
#else
				if (page_is_file_lru(page)) {
#endif
					int is_ext4_buddy_cache = 0;

					/* check if this inode is EXT4_BAD_INO */
					if (trylock_page(page)) {
						if (page->mapping && page->mapping->host->i_ino == 1)
							is_ext4_buddy_cache = 1;
						unlock_page(page);
					} else {
						SetPageLRU(page);
						put_page(page);
						lru_fail++;
						continue;
					}

					if (is_ext4_buddy_cache) {
						SetPageLRU(page);
						put_page(page);
						lru_fail++;
						continue;
					}
				}
#endif

				lru_done++;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0))
				del_page_from_lru_list(page, mem_cgroup_page_lruvec(page, zone->zone_pgdat), lru);
#else
				del_page_from_lru_list(page, mem_cgroup_page_lruvec(page));
#endif
				list_add_tail(&page->lru, &page_list);
				__mod_zone_page_state(zone, 
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 7, 0))
					NR_ISOLATED_ANON + page_is_file_cache(page), 
#else
					NR_ISOLATED_ANON + page_is_file_lru(page), 
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 9, 0))
					hpage_nr_pages(page));
#else
					thp_nr_pages(page));
#endif
			}

			if ((map_done + lru_done) >= SWAP_CLUSTER_MAX)
				break;
		}
		rtd_pr_rmm_info("lru: %d\t\t done: %d\t fail: %d\n", lru, lru_done, lru_fail);
		map_done += lru_done;
		map_fail += lru_fail;
		lru_done = lru_fail = 0;

		if (map_done >= SWAP_CLUSTER_MAX)
			break;
	}
	spin_unlock_irq(lru_lock);
	rtd_pr_rmm_info("total isolated:\t done: %d\t fail: %d\n", map_done, map_fail);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
	ret = migrate_pages(&page_list, remap_alloc_page, NULL, REMAP_ZONE, MIGRATE_SYNC, MR_CONTIG_RANGE);
#else
	ret = migrate_pages(&page_list, remap_alloc_page, NULL, REMAP_ZONE, MIGRATE_SYNC, MR_CONTIG_RANGE, NULL);
#endif
	map_done -= ret;
	map_fail += ret;
	rtd_pr_rmm_info("total migrated:\t done: %d\t fail: %d\n", map_done, ret);
	putback_movable_pages(&page_list);

out:
	return -1;
}

struct shrinker remap_shrinker = {
	.count_objects = remap_shrink_count,
	.scan_objects = remap_shrink,
	.seeks = DEFAULT_SEEKS * 4,
};
#endif

int __init remap_init(void)
{
#if 0 //IS_ENABLED(CONFIG_DMABUF_HEAPS_DVR) && IS_ENABLED(CONFIG_DMABUF_CARVEOUT_HEAP) && IS_ENABLED(CONFIG_DMABUF_HEAPS_VBM)
	dvr_heap = dma_heap_find("dvr_heap");
	carveout_heap = dma_heap_find("carveout_heap");
#endif
	rtk_set_attribute_init();
#ifndef CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE
	register_shrinker(&remap_shrinker);
#endif
	if (rmm_vendor_hook_init() < 0)
		return -EINVAL;

	rtd_pr_rmm_info("remap initialized\n");

	return 0;
}

void __exit remap_exit(void)
{
#if IS_ENABLED(CONFIG_DMABUF_HEAPS_DVR) && IS_ENABLED(CONFIG_DMABUF_CARVEOUT_HEAP) && IS_ENABLED(CONFIG_DMABUF_HEAPS_VBM)
	dma_heap_put(dvr_heap);
	dma_heap_put(carveout_heap);
#endif
#ifndef CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE
	unregister_shrinker(&remap_shrinker);
#endif
	rmm_vendor_hook_exit();

	rtd_pr_rmm_info("remap unloaded\n");
}
