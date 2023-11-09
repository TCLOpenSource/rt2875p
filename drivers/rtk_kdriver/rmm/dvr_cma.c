
#include <linux/version.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <rtd_log/rtd_module_log.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/cma.h>
#include <linux/syscalls.h>
#include <linux/suspend.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include <linux/dma-map-ops.h>
#else
#include <linux/dma-contiguous.h>
#endif
#include <linux/pageremap.h>
#include <linux/rtkblueprint.h>

#include "cma.h" //-I$(srctree)/mm/


//FIXME_GDMA it'd be better to access GDMA register at GDMA module API and let rmm to invoke it
#if defined(CONFIG_RTK_KDRV_GDMA)
#include <rbus/sys_reg_reg.h>
#include <rbus/gdma_reg.h>
extern unsigned int gdma_dev_addr ;//kernel winfo addr
extern int GDMA_Check_Dvr_Reclaim(void);
#else
#if defined(CONFIG_RTK_KDRV_GDMA_MODULE)
int (*ptr_GDMA_Check_Dvr_Reclaim)(void)=0;
#endif

#endif
#include <rtk_kdriver/io.h>


static DEFINE_MUTEX(dvr_cma_mutex);

int cma_retry_new_range = 1;
#define CONFIG_CMA_RETRY_NEW_RANGE 1
/* try max 8 CMA region to avoid deadlock
 * on locked pages that belong to CMA allocating region.
 * (max 2 is not enough, report 1 time in 5 month QA stage)
 */
#define PN_MAX 8

void cma_sync_work(struct work_struct *work)
{
	ksys_sync_helper();
	kfree(work);
	pr_info("%s(): EBUSY Sync complete\n", __func__);
}

void cma_ebusy_sync_pinned_pages(void)
{
	struct work_struct *work;

	work = kmalloc(sizeof(*work), GFP_ATOMIC);
	if (work) {
		INIT_WORK(work, cma_sync_work);
		schedule_work(work);
	}
}

unsigned long dvr_cma_get_bitmap(const struct cma *cma)
{
	return (unsigned long)(cma->bitmap);
}

void *dma_get_allocator(struct device *dev)
{
	struct cma *cma = dev_get_cma_area(dev);

	if (!cma)
		return 0;
	else
		return (void *)dvr_cma_get_bitmap(cma);
}
EXPORT_SYMBOL(dma_get_allocator);

struct page *dvr_cma_alloc(struct cma *cma, size_t count, unsigned int align,
		       bool no_warn)
{
	unsigned long pfn = -1;
	size_t i;
	struct page *page = NULL;
	int ret = -ENOMEM;

	unsigned long pn[PN_MAX];
	unsigned long pageno;
	int pn_idx;
	bool sync_fs_once_when_ebusy = true;

	if (!cma || !cma->count)
		return NULL;

	rtd_pr_rmm_debug("%s(%s %lx, count %zu, align %d)\n", __func__, cma->name, (unsigned long)cma,
		 count, align);

	if (!count)
		return NULL;

	pageno = alloc_rtkbp_memory((struct mem_bp *)cma->bitmap, get_order(count * PAGE_SIZE));
//	show_rtkbp((struct mem_bp *)cma->bitmap);
	if (pageno == INVALID_VAL) {
		rtd_pr_rmm_err("%s %lx: order %lu free pages %lu\n", cma->name, (unsigned long)cma, (uintptr_t)get_order(count * PAGE_SIZE), ((struct mem_bp *)cma->bitmap)->avail_size);
//		dump_stack();
		return NULL;
	}

	pfn = cma->base_pfn + pageno;

	for (pn_idx = 0; pn_idx < PN_MAX; pn_idx++) {
		unsigned int tries;
		tries = 0;
retry:
		mutex_lock(&dvr_cma_mutex);
		ret = alloc_contig_range(pfn, pfn + count, MIGRATE_CMA,
					 GFP_KERNEL | (no_warn ? __GFP_NOWARN : 0));
		mutex_unlock(&dvr_cma_mutex);

		if (ret == 0) {
			page = pfn_to_page(pfn);
			break;
		} else if (ret != -EBUSY) {
			free_rtkbp_memory((struct mem_bp *)cma->bitmap, pageno, get_order(count * PAGE_SIZE));
			break;
		}

		if (++tries) {
			if (tries < 10) {
				rtd_pr_rmm_warn("cma %p: fail(0x%x) at pfn:0x%x, retry(%d) remapping...\n",
					(void *)cma, ret, pfn, tries);
				msleep(1L << tries);
				goto retry;
			}
		}

		// to remap next available CMA range, instead stuck the same pages.
		if (((CONFIG_CMA_RETRY_NEW_RANGE) != 0) && (0 != cma_retry_new_range)) {
			unsigned long next_pageno;
			next_pageno = alloc_rtkbp_memory((struct mem_bp *)cma->bitmap, get_order(count * PAGE_SIZE));
			//show_rtkbp((struct mem_bp *)cma->bitmap);
			if (next_pageno == INVALID_VAL) {
				free_rtkbp_memory((struct mem_bp *)cma->bitmap, pageno, get_order(count * PAGE_SIZE));
				rtd_pr_rmm_warn("cma %p: available pages %lu\n", (void *)cma, ((struct mem_bp *)cma->bitmap)->avail_size);
				page = NULL;
				break;
			}

			pn[pn_idx] = pageno;

			pageno = next_pageno;
			pfn = cma->base_pfn + pageno;
		}

		if (sync_fs_once_when_ebusy) {
			sync_fs_once_when_ebusy = false;
			cma_ebusy_sync_pinned_pages();
		}
	}
	while (pn_idx > 0) {
		pn_idx--;
		free_rtkbp_memory((struct mem_bp *)cma->bitmap, pn[pn_idx], get_order(count * PAGE_SIZE));
	}


	/*
	 * CMA can allocate multiple page blocks, which results in different
	 * blocks being marked with different tags. Reset the tags to ignore
	 * those page blocks.
	 */
	if (page) {
		for (i = 0; i < count; i++)
			page_kasan_tag_reset(page + i);
	}

	if (ret && !no_warn) {
		rtd_pr_rmm_err("%s: alloc failed, req-size: %zu pages, ret: %d\n",
			__func__, count, ret);

	}

	rtd_pr_rmm_debug("%s(): returned %p\n", __func__, page);
	return page;
}

bool dvr_cma_release(struct cma *cma, const struct page *pages, unsigned int count)
{
	unsigned long pfn;

	if (!cma || !pages)
		return false;

	rtd_pr_rmm_debug("%s(page %p)\n", __func__, (void *)pages);

	pfn = page_to_pfn(pages);

	if (pfn < cma->base_pfn || pfn >= cma->base_pfn + cma->count)
		return false;

	VM_BUG_ON(pfn + count > cma->base_pfn + cma->count);

	free_contig_range(pfn, count);
	free_rtkbp_memory((struct mem_bp *)cma->bitmap, pfn - cma->base_pfn, get_order(count * PAGE_SIZE));
//	show_rtkbp((struct mem_bp *)cma->bitmap);

	return true;
}

static unsigned long dvr_cma_bitmap_aligned_mask(const struct cma *cma,
					     unsigned int align_order)
{
	if (align_order <= cma->order_per_bit)
		return 0;
	return (1UL << (align_order - cma->order_per_bit)) - 1;
}

/*
 * Find the offset of the base PFN from the specified align_order.
 * The value returned is represented in order_per_bits.
 */
static unsigned long dvr_cma_bitmap_aligned_offset(const struct cma *cma,
					       unsigned int align_order)
{
	return (cma->base_pfn & ((1UL << align_order) - 1))
		>> cma->order_per_bit;
}

static unsigned long dvr_cma_bitmap_pages_to_bits(const struct cma *cma,
					      unsigned long pages)
{
	return ALIGN(pages, 1UL << cma->order_per_bit) >> cma->order_per_bit;
}

static void dvr_cma_clear_bitmap(struct cma *cma, unsigned long pfn,
			     unsigned int count)
{
	unsigned long bitmap_no, bitmap_count;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
	unsigned long flags;
#endif
	bitmap_no = (pfn - cma->base_pfn) >> cma->order_per_bit;
	bitmap_count = dvr_cma_bitmap_pages_to_bits(cma, count);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
	spin_lock_irqsave(&cma->lock, flags);
#else
	mutex_lock(&cma->lock);
#endif
	bitmap_clear(cma->bitmap, bitmap_no, bitmap_count);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
	spin_unlock_irqrestore(&cma->lock, flags);
#else
	mutex_unlock(&cma->lock);
#endif
}

struct page *dvr_cma_bitmap_alloc(struct cma *cma, size_t count, unsigned int align,
		       bool no_warn)
{
	unsigned long mask, offset;
	unsigned long pfn = -1;
	unsigned long start = 0;
	unsigned long bitmap_maxno, bitmap_no, bitmap_count;
	size_t i;
	struct page *page = NULL;
	int ret = -ENOMEM;
	bool sync_fs_once_when_ebusy = true;

	if (!cma || !cma->count)
		return NULL;

	rtd_pr_rmm_debug("%s(cma %p, count %zu, align %d)\n", __func__, (void *)cma,
		 count, align);

	if (!count)
		return NULL;

	mask = dvr_cma_bitmap_aligned_mask(cma, align);
	offset = dvr_cma_bitmap_aligned_offset(cma, align);
	bitmap_maxno = cma_bitmap_maxno(cma);
	bitmap_count = dvr_cma_bitmap_pages_to_bits(cma, count);

	if (bitmap_count > bitmap_maxno)
		return NULL;

	for (;;) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
		spin_lock_irq(&cma->lock);
#else
		mutex_lock(&cma->lock);
#endif
		bitmap_no = bitmap_find_next_zero_area_off(cma->bitmap,
				bitmap_maxno, start, bitmap_count, mask,
				offset);
		if (bitmap_no >= bitmap_maxno) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
			spin_unlock_irq(&cma->lock);
#else
			mutex_unlock(&cma->lock);
#endif
			break;
		}
		bitmap_set(cma->bitmap, bitmap_no, bitmap_count);
		/*
		 * It's safe to drop the lock here. We've marked this region for
		 * our exclusive use. If the migration fails we will take the
		 * lock again and unmark it.
		 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
		spin_unlock_irq(&cma->lock);
#else
		mutex_unlock(&cma->lock);
#endif
		pfn = cma->base_pfn + (bitmap_no << cma->order_per_bit);
		mutex_lock(&dvr_cma_mutex);
		ret = alloc_contig_range(pfn, pfn + count, MIGRATE_CMA,
				     GFP_KERNEL | (no_warn ? __GFP_NOWARN : 0));
		mutex_unlock(&dvr_cma_mutex);
		if (ret == 0) {
			page = pfn_to_page(pfn);
			break;
		}

		dvr_cma_clear_bitmap(cma, pfn, count);
		if (ret != -EBUSY)
			break;

		rtd_pr_rmm_debug("%s(): memory range at %p is busy, retrying\n",
			 __func__, pfn_to_page(pfn));

		if (sync_fs_once_when_ebusy) {
			sync_fs_once_when_ebusy = false;
			cma_ebusy_sync_pinned_pages();
		}
		/* try again with a bit different memory target */
		start = bitmap_no + mask + 1;
	}

	/*
	 * CMA can allocate multiple page blocks, which results in different
	 * blocks being marked with different tags. Reset the tags to ignore
	 * those page blocks.
	 */
	if (page) {
		for (i = 0; i < count; i++)
			page_kasan_tag_reset(page + i);
	}

	if (ret && !no_warn) {
		rtd_pr_rmm_err("%s: alloc failed, req-size: %zu pages, ret: %d\n",
			__func__, count, ret);

	}

	rtd_pr_rmm_debug("%s(): returned %p\n", __func__, page);
#ifdef CONFIG_RTK_FEATURE_FOR_GKI
	if (page) {
		cma->used += count;
		if(cma->used > cma->peak)
			cma->peak = cma->used;
		if(cma->peak > cma->count)
			cma->peak = cma->count;
		rtd_pr_rmm_debug("bitmap alloc %d bitmap used %d peak %d all %d order %d pfn %lx\n",
				 count, cma->used, cma->peak, cma->count, cma->order_per_bit, cma->base_pfn);
	}
#endif
	return page;
}

struct page *dvr_cma_bitmap_alloc_memory_specific_count(struct cma *cma, size_t count, unsigned int align,
											  unsigned long pfn, bool no_warn)
{
	struct page *page = NULL;
	int err = 0;
	size_t i;
	unsigned long mask, offset;
	unsigned long bitmap_no_pfn = -1;
	unsigned long start = 0;
	unsigned long bitmap_maxno, bitmap_no, bitmap_count;
	bool sync_fs_once_when_ebusy = true;

	if (!cma || !cma->count)
		return NULL;

	rtd_pr_rmm_debug("%s(cma %p, count %zu, align %d)\n", __func__, (void *)cma,
		 count, align);

	if (!count)
		return NULL;

	if (cma->order_per_bit != 0) {
		rtd_pr_rmm_err("%s() order_per_bit not 0, cma(%lx)\n", __func__, (unsigned long)cma);
		goto out;
	} else if (pfn < cma->base_pfn || pfn >= cma->base_pfn + cma->count) {
		rtd_pr_rmm_err("%s() pfn=%lx not in range (%lx ~ %lx)\n", __func__, pfn,
			cma->base_pfn, (cma->base_pfn + cma->count));
		goto out;
	}

	mask = dvr_cma_bitmap_aligned_mask(cma, align);
	offset = dvr_cma_bitmap_aligned_offset(cma, align);
	bitmap_maxno = cma_bitmap_maxno(cma);
	bitmap_count = dvr_cma_bitmap_pages_to_bits(cma, count);

	if (bitmap_count > bitmap_maxno)
		return NULL;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
		spin_lock_irq(&cma->lock);
#else
	mutex_lock(&cma->lock);
#endif
	start = (pfn - cma->base_pfn) >> cma->order_per_bit;
	rtd_pr_rmm_debug("%s() start = %ld, pfn = %ld, base_pfn = %ld\n", __func__, start, pfn, cma->base_pfn);
	bitmap_no = bitmap_find_next_zero_area_off(cma->bitmap,
			bitmap_maxno, start, bitmap_count, mask,
			offset);
	rtd_pr_rmm_debug("%s() bitmap_no = %ld, bitmap_count = %ld\n", __func__, bitmap_no, bitmap_count);
	if (bitmap_no >= bitmap_maxno) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
		spin_unlock_irq(&cma->lock);
#else
		mutex_unlock(&cma->lock);
#endif
		rtd_pr_rmm_err("%s() bitmap_allocate_region err, cma(%lx)\n", __func__,(unsigned long)cma);
		goto out;
	}
	bitmap_set(cma->bitmap, bitmap_no, bitmap_count);
	/*
	 * It's safe to drop the lock here. We've marked this region for
	 * our exclusive use. If the migration fails we will take the
	 * lock again and unmark it.
	 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
	spin_unlock_irq(&cma->lock);
#else
	mutex_unlock(&cma->lock);
#endif
	bitmap_no_pfn = cma->base_pfn + (bitmap_no << cma->order_per_bit);
	if (pfn != bitmap_no_pfn) {
		rtd_pr_rmm_err("%s() bitmap_no = %d err, pfn (%ld) is not equal to bitmap_no_pfn (%ld)\n", __func__, bitmap_no, pfn, bitmap_no_pfn);
		goto out;
	}

	for (;;) {
		mutex_lock(&dvr_cma_mutex);
		err = alloc_contig_range(pfn, pfn + count, MIGRATE_CMA,
				     GFP_KERNEL | (no_warn ? __GFP_NOWARN : 0));
		mutex_unlock(&dvr_cma_mutex);
		if (err == 0) {
			page = pfn_to_page(pfn);
			break;
		} else if (err != -EBUSY) {
			dvr_cma_clear_bitmap(cma, pfn, count);
			goto out;
		}

		rtd_pr_rmm_info("%s(): memory range at %p is busy, retrying\n",
			__func__, pfn_to_page(pfn));

		if (sync_fs_once_when_ebusy) {
			sync_fs_once_when_ebusy = false;
			cma_ebusy_sync_pinned_pages();
		}
		msleep(10);
	}

	/*
	 * CMA can allocate multiple page blocks, which results in different
	 * blocks being marked with different tags. Reset the tags to ignore
	 * those page blocks.
	 */
	if (page) {
		for (i = 0; i < count; i++)
			page_kasan_tag_reset(page + i);
	}

	rtd_pr_rmm_info("%s(), pfn(%lx) page(%lx)\n", __func__, pfn, (unsigned long)page);
#ifdef CONFIG_RTK_FEATURE_FOR_GKI
	if (page) {
		cma->used += count;
		if(cma->used > cma->peak)
			cma->peak = cma->used;
		if(cma->peak > cma->count)
			cma->peak = cma->count;
		rtd_pr_rmm_debug("bitmap alloc %d bitmap used %d peak %d all %d order %d pfn %lx\n",
				 count, cma->used, cma->peak, cma->count, cma->order_per_bit, cma->base_pfn);
	}
#endif
	return page;

out:
	return NULL;
}

struct page *dvr_cma_bitmap_alloc_memory_specific(struct cma *cma, unsigned int order, 
											  unsigned long pfn, bool no_warn)
{
	struct page *page = NULL;
    int count = (1 << order);
	int err = 0;
	size_t i;

	if (!cma || !cma->count)
		return NULL;

	if (!order)
		return NULL;

    if (cma->order_per_bit != 0) {
        rtd_pr_rmm_err("%s() order_per_bit not 0, cma(%lx)\n", __func__, (unsigned long)cma);
        goto out;
    } else if (pfn < cma->base_pfn || pfn >= cma->base_pfn + cma->count) {
        rtd_pr_rmm_err("%s() pfn=%lx not in range (%lx ~ %lx)\n", __func__, pfn, 
               cma->base_pfn, (cma->base_pfn + cma->count));
        goto out;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
	spin_lock_irq(&cma->lock);
#else
    mutex_lock(&cma->lock);
#endif
    err = bitmap_allocate_region(cma->bitmap, pfn - cma->base_pfn, order);
    if (err != 0) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
		spin_unlock_irq(&cma->lock);
#else
        mutex_unlock(&cma->lock);
#endif
        rtd_pr_rmm_err("%s() bitmap_allocate_region err, cma(%lx)\n", __func__,(unsigned long)cma);
        goto out;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
	spin_unlock_irq(&cma->lock);
#else
	mutex_unlock(&cma->lock);
#endif
	for (;;) {
		mutex_lock(&dvr_cma_mutex);
		err = alloc_contig_range(pfn, pfn + count, MIGRATE_CMA,
				     GFP_KERNEL | (no_warn ? __GFP_NOWARN : 0));
		mutex_unlock(&dvr_cma_mutex);

		if (err == 0) {
			page = pfn_to_page(pfn);
			break;
        } else if (err != -EBUSY) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
			unsigned long flags;
			spin_lock_irqsave(&cma->lock, flags);
#else
			mutex_lock(&cma->lock);
#endif
            bitmap_release_region(cma->bitmap, pfn - cma->base_pfn, order);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
			spin_unlock_irqrestore(&cma->lock, flags);
#else
            mutex_unlock(&cma->lock);
#endif
            goto out;
        }

		rtd_pr_rmm_info("%s(): memory range at %p is busy, retrying\n",
                 __func__, pfn_to_page(pfn));
        msleep(10);
    }

	/*
	 * CMA can allocate multiple page blocks, which results in different
	 * blocks being marked with different tags. Reset the tags to ignore
	 * those page blocks.
	 */
	if (page) {
		for (i = 0; i < count; i++)
			page_kasan_tag_reset(page + i);
	}

    rtd_pr_rmm_info("%s(), pfn(%lx) page(%lx)\n", __func__, pfn, (unsigned long)page);
#ifdef CONFIG_RTK_FEATURE_FOR_GKI
	if (page) {
		cma->used += count;
		if(cma->used > cma->peak)
			cma->peak = cma->used;
		if(cma->peak > cma->count)
			cma->peak = cma->count;
		rtd_pr_rmm_debug("bitmap alloc %d bitmap used %d peak %d all %d order %d pfn %lx\n", 
				 count, cma->used, cma->peak, cma->count, cma->order_per_bit, cma->base_pfn);
	}
#endif
    return page;

out:
    return NULL;
}

bool dvr_cma_bitmap_release(struct cma *cma, const struct page *pages, unsigned int count)
{
	unsigned long pfn;
//	unsigned long bitmap_maxno;

	if (!cma || !pages)
		return false;

	rtd_pr_rmm_debug("%s(page %p)\n", __func__, (void *)pages);

	pfn = page_to_pfn(pages);

	if (pfn < cma->base_pfn || pfn >= cma->base_pfn + cma->count)
		return false;

	VM_BUG_ON(pfn + count > cma->base_pfn + cma->count);

	free_contig_range(pfn, count);
	dvr_cma_clear_bitmap(cma, pfn, count);
#ifdef CONFIG_RTK_FEATURE_FOR_GKI
	cma->used -= count;
	if(cma->used < 0)
		cma->used = 0;
//	bitmap_maxno = cma_bitmap_maxno(cma);
	rtd_pr_rmm_debug("bitmap free %d bitmap used %d peak %d all %d\n", count, cma->used, cma->peak, cma->count);
#endif
	return true;
}

void dvr_cma_show_bitmap(struct cma *cma)
{
#ifdef CONFIG_RTK_FEATURE_FOR_GKI
	unsigned long cma_used_size = cma->used << PAGE_SHIFT;
	unsigned long cma_bitmap_all_size = cma->count << PAGE_SHIFT;
	unsigned long cma_peak_size = cma->peak << PAGE_SHIFT;
	unsigned long cma_free = 0, cma_free_size = 0;
	cma_free = cma->count - cma->used;
	cma_free_size = cma_free << PAGE_SHIFT;
	rtd_pr_rmm_warn("bitmap:base_pfn=0x%lx(0x%lx) used=%d(0x%lx) peak=%d(0x%lx) all=%d(0x%lx) free=%d(0x%lx) pages\n",
			cma->base_pfn, cma->base_pfn << PAGE_SHIFT, cma->used, cma_used_size, 
			cma->peak, cma_peak_size, cma->count, cma_bitmap_all_size,  cma_free, cma_free_size);
#else
	unsigned long cma_used_size = 0;
	unsigned long cma_bitmap_all_size = cma->count << PAGE_SHIFT;
	unsigned long cma_peak_size = 0;
	unsigned long cma_free = 0, cma_free_size = 0;
	rtd_pr_rmm_warn("bitmap:base_pfn=0x%lx(0x%lx) used=%d(0x%lx) peak=%d(0x%lx) all=%d(0x%lx) free=%d(0x%lx) pages\n",
			cma->base_pfn, cma->base_pfn << PAGE_SHIFT, 0, cma_used_size, 
			0, cma_peak_size, cma->count, cma_bitmap_all_size,  cma_free, cma_free_size);
#endif
}

struct page *dvr_alloc_from_contiguous(struct device *dev, size_t count,
				       unsigned int align, bool no_warn)
{
	if (align > CONFIG_CMA_ALIGNMENT)
		align = CONFIG_CMA_ALIGNMENT;

	return dvr_cma_alloc(dev_get_cma_area(dev), count, align, no_warn);
}

bool dvr_release_from_contiguous(struct device *dev, struct page *pages,
				 int count)
{
	return dvr_cma_release(dev_get_cma_area(dev), pages, count);
}

struct page *dvr_bitmap_alloc_from_contiguous(struct device *dev, size_t count,
				       unsigned int align, bool no_warn)
{
	if (align > CONFIG_CMA_ALIGNMENT)
		align = CONFIG_CMA_ALIGNMENT;

	return dvr_cma_bitmap_alloc(dev_get_cma_area(dev), count, align, no_warn);
}

struct page *dvr_bitmap_alloc_memory_specific_count(struct device *dev, size_t count, unsigned int align,
											  unsigned long pfn, bool no_warn)
{
	if (align > CONFIG_CMA_ALIGNMENT)
		align = CONFIG_CMA_ALIGNMENT;

	return dvr_cma_bitmap_alloc_memory_specific_count(dev_get_cma_area(dev), count, align, pfn, no_warn);
}

struct page *dvr_bitmap_alloc_memory_specific(struct device *dev, unsigned int order, 
											  unsigned long pfn, bool no_warn)
{
    return dvr_cma_bitmap_alloc_memory_specific(dev_get_cma_area(dev), order, pfn, no_warn);
}

bool dvr_bitmap_release_from_contiguous(struct device *dev, struct page *pages,
				 int count)
{
	return dvr_cma_bitmap_release(dev_get_cma_area(dev), pages, count);
}

void dvr_show_bitmap(struct device *dev)
{
	dvr_cma_show_bitmap(dev_get_cma_area(dev));
}

#ifndef CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE
int __init dvr_declare_contiguous(struct device *dev, phys_addr_t size,
				       phys_addr_t base, phys_addr_t limit, const char *name)
{
	struct cma *cma;
	int ret;
	ret = cma_declare_contiguous(base, size, limit, 0, 0, true,
					name, &cma);
	if (ret == 0) {
		if (dev)
			dev->cma_area = cma;
	} else {
		return -EINVAL;
	}

#ifdef RTK_MEM_LAYOUT_DEVICETREE
	/* let dts decide the default */ 
#else
	if (dev == NULL && dma_contiguous_default_area == NULL)
		dma_contiguous_default_area = cma;
	else if (dev == NULL)
		BUG();
#endif

	return ret;
}

int __init dvr_cma_declare_null(struct cma **res_cma)
{
	struct cma *cma;

	/* Sanity checks */
	if (cma_area_count == ARRAY_SIZE(cma_areas)) {
		rtd_pr_rmm_err("Not enough slots for CMA reserved regions!\n");
		return -ENOSPC;
	}

	cma = &cma_areas[cma_area_count];
	cma->base_pfn = 0;
	cma->count = 0;

	*res_cma = cma;
	cma_area_count++;

	rtd_pr_rmm_info("CMA: reserved null cma\n");

	return 0;
}

int __init dvr_declare_null(struct device *dev)
{
	struct cma *cma;
	int ret;

	ret = dvr_cma_declare_null(&cma);
	if (ret == 0) {
		if (dev)
			dev->cma_area = cma;
	}

	return ret;
}
#endif



#if 1 // CONFIG_BOOTUP_ANIMATION

#define SYNC_STRUCT_ADDR 0x000000cc

typedef enum {
	E_STAT_NOT_READY = 0,
	E_STAT_READY_TO_FINISH,
	E_STAT_FINISH
} BOOT_ANIMATION_STATUS;

void *pAnimation = 0;
void *pAnimation_1 = 0;
void *pLastImage = 0;
struct dma_buf *pQShowVBM = 0;

unsigned int notify_rtkvdec_release_pov = 0 ;

EXPORT_SYMBOL(pAnimation);
EXPORT_SYMBOL(pAnimation_1);
EXPORT_SYMBOL(pLastImage);
EXPORT_SYMBOL(pQShowVBM);

EXPORT_SYMBOL(notify_rtkvdec_release_pov);

#ifdef CONFIG_DMABUF_HEAPS_VBM
extern void vbm_heap_dump_chunk_list(void); // debug dump
#endif


//void __init_memblock memblock_dump(struct memblock_type *type);

void free_reserved_memory(unsigned long rsv_addr, unsigned long rsv_size)
{
    unsigned long addr = rsv_addr, size = rsv_size;
    struct page *page;
    int count, order;

    rtd_pr_rmm_notice("%s addr 0x%lx, size 0x%lx\n", __func__, addr, size);

    addr = PFN_PHYS(PFN_UP(addr));
    count = (PFN_DOWN(addr+size)-PFN_UP(addr));
    size = count * PAGE_SIZE;

#if 0 //[FIXME] unless we could check as memblock_is_region_reserved in GKI
    if (!(memblock_is_region_reserved(addr, size))) {
        rtd_pr_rmm_err("%s ERROR region 0x%lx,0x%lx is not reserved!!!\n", __func__, addr, size);;
        return -ENOMEM;
    }
#endif

    page = pfn_to_page(PFN_UP(addr));
    order = min(MAX_ORDER-1, get_order(size));

    rtd_pr_rmm_notice("%s addr 0x%lx count 0x%lx size 0x%lx page 0x%lx order %d\n", __func__, addr, count, size, page, order);

    do {
        int i;
        struct page *p;

        while (count < (1<<order)) {
        order--;
        }

        rtd_pr_rmm_debug(" - count 0x%lx page 0x%lx order %d\n", count, page, order);

        i = 1<<order;
        p = page;
        do {
            __clear_bit(PG_reserved, &p->flags); //__ClearPageReserved(p);
            atomic_set(&p->_refcount, 0); //set_page_count(p, 0);
        } while (++p, --i);

        atomic_set(&page->_refcount, 1); //set_page_count(page, 1);
        __free_pages(page, order);

        page = page + (1<<order);
        count = count - (1<<order);
    } while (count);

    adjust_managed_page_count(pfn_to_page(PFN_UP(addr)), size / PAGE_SIZE); // although 'memtotal' will be updated after free but not 'managed'
    memblock_free(addr, size); // although reserve was returned to buddy, but not memblock reserved list.

    // deubg check
//    memblock_dump(&memblock.memory);
//    memblock_dump(&memblock.reserved);

    rtd_pr_rmm_notice("%s addr 0x%lx, size 0x%lx done\n", __func__, addr, size);

}

/*
 * generic devicetree layout logo reclaim
 *  for legacy implementation overridden
 */

int __weak reclaim_logo = 1; // global oneshot reclaim
                             //[FIXME] unless we could check as memblock_is_region_reserved() in GKI
int __weak reclaim_logo_memory(void)
{
    struct device_node *np;
    unsigned long logo_addr=0, logo_size=0;
    u64 tmp64[2];

	if (reclaim_logo)
	{
		np = of_find_node_by_path("/reserved-memory/reclaim_memory_rsvd");
		if ((of_property_read_u64_array(np, "reg", tmp64, 2)) == 0) {
			if (tmp64[1] != 0) { // size 0 for disabled
				logo_addr = tmp64[0];
				logo_size = tmp64[1];
				rtd_pr_rmm_notice("found reclaim_memory_rsvd 0x%lx 0x%lx 0x%lx in ufdt\n", tmp64[0], tmp64[1], pAnimation);

				free_reserved_memory(logo_addr, logo_size);
				reclaim_logo = 0;
			}
			else {
				rtd_pr_rmm_notice("reclaim_memory_rsvd disabled in ufdt\n");
			}
		}
		else {
			rtd_pr_rmm_notice("NO reclaim_memory_rsvd in ufdt\n");
		}
	}

	return 0;
}

void do_dvr_reclaim(int cmd)
{
	int aStat = E_STAT_NOT_READY;
	int vStat = E_STAT_NOT_READY;
//	unsigned int *unlzma = (unsigned int *)phys_to_virt(UNLZMA_SYNC_ADDR);

#if 1
#if defined(CONFIG_RTK_KDRV_GDMA)
	if(GDMA_Check_Dvr_Reclaim())
		return;
#else
#if defined(CONFIG_RTK_KDRV_GDMA_MODULE)
	if(ptr_GDMA_Check_Dvr_Reclaim != 0)
		if(ptr_GDMA_Check_Dvr_Reclaim())
			return;
#endif
#endif
#else
#if defined(CONFIG_RTK_KDRV_GDMA)
	//FIXME_GDMA it'd be better to access GDMA register at GDMA module API and let rmm to invoke it
	unsigned int osdwi = (unsigned int)rtd_inl(GDMA_OSD1_WI_reg);
	if(SYS_REG_SYS_DISPCLKSEL_get_clken_disp_gdma(rtd_inl(SYS_REG_SYS_DISPCLKSEL_reg))) {
		if(osdwi && gdma_dev_addr && (osdwi < gdma_dev_addr)) {
			rtd_pr_mach_rtk_debug("[GDMA]Skip %s, used=%08x, gdma will free later..\n",__func__,osdwi);
			return;
		}
	}
#endif
#endif
	rtd_pr_rmm_err("\033[1;44m [%s] cmd=%d...\033[0m\n", __FUNCTION__, cmd);
	/*
	 * Reclaim pAnimation
	 *   cmd=1: will wait for animation done passively.
	 *   cmd=3: will force to stop animation actively.
	 */

	rtd_pr_rmm_info("[POV] cmd = %d, reclaim_logo = %d (pAnimation = %x)\n", cmd, reclaim_logo, pAnimation);
	if (((cmd == 1) || (cmd == 3)) && (reclaim_logo||pAnimation)) //[FIXME] always leverage devicetree
	{
		unsigned int sync_phy = ntohl(*(unsigned int *)phys_to_virt(SYNC_STRUCT_ADDR));
		unsigned int force_to_stop_video = 0;
		unsigned int force_to_stop_audio = 0;

		if (cmd == 3) { // flag for force to stop video/audio
			force_to_stop_video = 1;
			force_to_stop_audio = 1;
		}

		if (sync_phy) {
			volatile char *sync_tmp, *sync_ptr;
			unsigned int aEnable = 0;
			unsigned int vEnable = 0;

			rtd_pr_rmm_err("*****[S]check boot animation %lx...\n",(unsigned long) sync_phy);
//		#ifdef CONFIG_ARM64
//			sync_tmp = dvr_remap_cached_memory(sync_phy, PAGE_SIZE, __builtin_return_address(0));
//		#else
			sync_tmp = dvr_remap_uncached_memory(sync_phy, PAGE_SIZE, __builtin_return_address(0));
//		#endif

			sync_ptr = sync_tmp + (sync_phy & ~PAGE_MASK);
			aEnable = (unsigned int)*(unsigned int *)(sync_ptr+12);
			vEnable = (unsigned int)*(unsigned int *)(sync_ptr+8);

			if (aEnable == 0) {
				rtd_pr_rmm_err("=====[A]no animation audio...\n");
				aStat = E_STAT_FINISH;
			}

			if (vEnable == 0) {
				rtd_pr_rmm_err("=====[V]no animation video...\n");
				vStat = E_STAT_FINISH;
			}
			// KWarning: checked ok by eaton.chiou@realtek.com
			while (1) {
				/*
				 * Check POV status first.
				 */
				if (vEnable && (vStat != E_STAT_FINISH)) {
					rtd_pr_rmm_err("*****[V]wait for animation video done...\n");
					if (2 == sync_ptr[20]) { //video playback is halt or finished.
						if (vStat == E_STAT_NOT_READY) { //video is halt at last frame.
							rtd_pr_rmm_err("*****[V]get video done bit=0x%x\n", sync_ptr[20]);
							sync_ptr[20] = 1; //notify video f/w to stop playback.
							rtd_pr_rmm_err("*****[V]change video done bit=0x%x\n", sync_ptr[20]);
							vStat = E_STAT_READY_TO_FINISH;
						}
						else if (vStat == E_STAT_READY_TO_FINISH) { //finished.
							vStat = E_STAT_FINISH;
							rtd_pr_rmm_err("=====[V]animation video is ready...\n");
							rtd_pr_rmm_info("[POV] set notify release as 1\n");
							notify_rtkvdec_release_pov = 1 ;
						}
					}
					else if (0 == sync_ptr[20]) { //video is still running
						if (force_to_stop_video) {
							rtd_pr_rmm_err("#####[V]get video done bit=0x%x\n", sync_ptr[20]);
							sync_ptr[20] = 1; //notify video f/w to stop playback.
							rtd_pr_rmm_err("#####[V]force change video done bit=0x%x\n", sync_ptr[20]);
							vStat = E_STAT_READY_TO_FINISH;
						}
					}
				}

				/*
				 * Check POM status after POV is ready.
				 */
				if (aEnable && (vStat == E_STAT_FINISH) && (aStat != E_STAT_FINISH)) { //after video is ready, check audio status
					rtd_pr_rmm_err("*****[A]wait for animation audio done...\n");
					if (2 == sync_ptr[21]) { //audio playback is halt or finished.
						if (aStat == E_STAT_NOT_READY) { //audio is halt
							rtd_pr_rmm_err("*****[A]get audio done bit=0x%x\n", sync_ptr[21]);
							sync_ptr[21] = 1; //notify audio f/w to stop playback.
							rtd_pr_rmm_err("*****[A]change audio done bit=0x%x\n", sync_ptr[21]);
							aStat = E_STAT_READY_TO_FINISH;
						}
						else if (aStat == E_STAT_READY_TO_FINISH) { //finished.
							aStat = E_STAT_FINISH;
							rtd_pr_rmm_err("=====[A]animation audio is ready...\n");
						}
					}
					else if (0 == sync_ptr[21]) { //audio is still running
						if (force_to_stop_audio) {
							rtd_pr_rmm_err("#####[A]get audio done bit=0x%x\n", sync_ptr[21]);
							sync_ptr[21] = 1; //notify audio f/w to stop playback.
							rtd_pr_rmm_err("#####[A]force change audio done bit=0x%x\n", sync_ptr[21]);
							aStat = E_STAT_READY_TO_FINISH;
						}
					}
				}

				msleep(100);

				/*
				 * Check POV and POM are both ready, we can exit boot animation and reclaim memory.
				 */
				if ((aStat == E_STAT_FINISH) && (vStat == E_STAT_FINISH)) {
					break;
				}

			}

			dvr_unmap_memory((void *)sync_tmp, PAGE_SIZE);
		}
		#if 0
		else {
			/*
			 * FIXME
			 */
			rtd_pr_rmm_err("=====check self-unlzma flag...\n");
			while (1) {
				if (!aStat) {
					// check audio flag...
					if (!unlzma[0]) {
						aStat = 1;
						rtd_pr_rmm_err("*****audio is unlzma...\n");
					}
				}
				if (!vStat) {
					// check video flag...
					if (!unlzma[1]) {
						vStat = 1;
						rtd_pr_rmm_err("*****video is unlzma...\n");
					}
				}

				if (aStat && vStat)
					break;

				msleep(100);
			}
		}
		#endif

		reclaim_logo_memory();
	}

	/*
	 * Reclaim pLastImage by cmd=2
	 */
	rtd_pr_rmm_info("[POV] cmd = %d, pLastImage = %p\n", cmd, pLastImage);
	if ((cmd == 2) && (pLastImage)) {
		dvr_free(pLastImage);
		pLastImage = 0;
        rtd_pr_rmm_err("\033[1;44m [%s] cmd=%d...\033[0m\n", __FUNCTION__, cmd);

	}

	/*
	 * Reclaim QuickShow VBM co-buffer by [2] (cmd=4)
	 */
	rtd_pr_rmm_info("[POV] cmd = %d, pQShowVBM = %p\n", cmd, pQShowVBM);
	if (cmd & 0x4) {
		if (pQShowVBM) {

#ifdef CONFIG_DMABUF_HEAPS_VBM
			vbm_heap_dump_chunk_list(); // debug info
#endif

			rtd_pr_rmm_info("[RMM]%s qshow vbm(%p) 0x%lx@0x%lx\n", __func__, pQShowVBM, pQShowVBM->size, dvr_dmabuf_to_phys(pQShowVBM));
			dvr_dmabuf_free(pQShowVBM);

			pQShowVBM = NULL;
		}
		else {
			rtd_pr_rmm_info("[RMM]%s qshow vbm(%p)\n", __func__, pQShowVBM);
#ifdef CONFIG_DMABUF_HEAPS_VBM
			vbm_heap_dump_chunk_list(); // debug info
#endif
		}
	}

}
EXPORT_SYMBOL(do_dvr_reclaim);
#if defined(CONFIG_RTK_KDRV_GDMA_MODULE)
EXPORT_SYMBOL(ptr_GDMA_Check_Dvr_Reclaim);
#endif
#else
void do_dvr_reclaim(int cmd)
{
	if(pAnimation)
	{
#ifdef CONFIG_SUPPORT_GDMA
		GDMA_ConfigOSDxEnable(GDMA_PLANE_OSD2, 0);
#endif
#ifdef CONFIG_CMA_RTK_ALLOCATOR
		dvr_free(pAnimation);
#endif
		pAnimation = 0;
		rtd_pr_rmm_err("\033[1;44m [%s] stop bootlogo and reclaim the memory\033[0m\n", __FUNCTION__);
	}
}
#endif // CONFIG_BOOTUP_ANIMATION

MODULE_AUTHOR("Realtek.com");
MODULE_LICENSE("Dual BSD/GPL");
