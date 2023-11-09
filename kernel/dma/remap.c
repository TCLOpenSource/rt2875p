// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2012 ARM Ltd.
 * Copyright (c) 2014 The Linux Foundation
 */
#include <linux/dma-direct.h>
#include <linux/dma-noncoherent.h>
#include <linux/dma-contiguous.h>
#include <linux/init.h>
#include <linux/genalloc.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#ifdef CONFIG_REALTEK_MEMORY_MANAGEMENT
#include <linux/pageremap.h>
#include <linux/rtkrecord.h>

extern void *return_address(unsigned int);
extern struct page *alloc_contiguous_memory(size_t size, int flags, const void *caller);
extern int free_contiguous_memory(struct page *page, int count);
#endif

struct page **dma_common_find_pages(void *cpu_addr)
{
	struct vm_struct *area = find_vm_area(cpu_addr);

	if (!area || area->flags != VM_DMA_COHERENT)
		return NULL;
	return area->pages;
}

static struct vm_struct *__dma_common_pages_remap(struct page **pages,
			size_t size, pgprot_t prot, const void *caller)
{
	struct vm_struct *area;

	area = get_vm_area_caller(size, VM_DMA_COHERENT, caller);
	if (!area)
		return NULL;

	if (map_vm_area(area, prot, pages)) {
		vunmap(area->addr);
		return NULL;
	}

	return area;
}

/*
 * Remaps an array of PAGE_SIZE pages into another vm_area.
 * Cannot be used in non-sleeping contexts
 */
void *dma_common_pages_remap(struct page **pages, size_t size,
			 pgprot_t prot, const void *caller)
{
	struct vm_struct *area;

	area = __dma_common_pages_remap(pages, size, prot, caller);
	if (!area)
		return NULL;

	area->pages = pages;

	return area->addr;
}

/*
 * Remaps an allocated contiguous region into another vm_area.
 * Cannot be used in non-sleeping contexts
 */
void *dma_common_contiguous_remap(struct page *page, size_t size,
			pgprot_t prot, const void *caller)
{
	int i;
	struct page **pages;
	struct vm_struct *area;

	pages = kmalloc(sizeof(struct page *) << get_order(size), GFP_KERNEL);
	if (!pages)
		return NULL;

	for (i = 0; i < (size >> PAGE_SHIFT); i++)
		pages[i] = nth_page(page, i);

	area = __dma_common_pages_remap(pages, size, prot, caller);

	kfree(pages);

	if (!area)
		return NULL;
	return area->addr;
}

/*
 * Unmaps a range previously mapped by dma_common_*_remap
 */
void dma_common_free_remap(void *cpu_addr, size_t size)
{
	struct vm_struct *area = find_vm_area(cpu_addr);

	if (!area || area->flags != VM_DMA_COHERENT) {
		WARN(1, "trying to free invalid coherent area: %p\n", cpu_addr);
		return;
	}

	unmap_kernel_range((unsigned long)cpu_addr, PAGE_ALIGN(size));
	vunmap(cpu_addr);
}

#ifdef CONFIG_DMA_DIRECT_REMAP
static struct gen_pool *atomic_pool __ro_after_init;

#define DEFAULT_DMA_COHERENT_POOL_SIZE  SZ_256K
static size_t atomic_pool_size __initdata = DEFAULT_DMA_COHERENT_POOL_SIZE;

static int __init early_coherent_pool(char *p)
{
	atomic_pool_size = memparse(p, &p);
	return 0;
}
early_param("coherent_pool", early_coherent_pool);

static gfp_t dma_atomic_pool_gfp(void)
{
	if (IS_ENABLED(CONFIG_ZONE_DMA))
		return GFP_DMA;
	if (IS_ENABLED(CONFIG_ZONE_DMA32))
		return GFP_DMA32;
	return GFP_KERNEL;
}

static int __init dma_atomic_pool_init(void)
{
	unsigned int pool_size_order = get_order(atomic_pool_size);
	unsigned long nr_pages = atomic_pool_size >> PAGE_SHIFT;
	struct page *page;
	void *addr;
	int ret;
#ifdef CONFIG_REALTEK_MEMORY_MANAGEMENT
	size_t size, count;
	int size_page, size_order;
	int count_page, count_order;

	size = size_page = PAGE_ALIGN(atomic_pool_size);
	count = count_page = size >> PAGE_SHIFT;

	if (need_debug_cma_guardband()) {
		size = size_order = dvr_guardband_size(size);
		count = count_order = size_order >> PAGE_SHIFT;
	}
	atomic_pool_size = size;
	nr_pages = atomic_pool_size >> PAGE_SHIFT;
#endif

	if (dev_get_cma_area(NULL))
#ifndef CONFIG_REALTEK_MEMORY_MANAGEMENT
		page = dma_alloc_from_contiguous(NULL, nr_pages,
						 pool_size_order, false);
#else
		page = alloc_contiguous_memory(size, GFP_DCU1,
				return_address(3));
#endif
	else
		page = alloc_pages(dma_atomic_pool_gfp(), pool_size_order);
	if (!page)
		goto out;

	arch_dma_prep_coherent(page, atomic_pool_size);

	atomic_pool = gen_pool_create(PAGE_SHIFT, -1);
	if (!atomic_pool)
		goto free_page;

	addr = dma_common_contiguous_remap(page, atomic_pool_size,
					   pgprot_dmacoherent(PAGE_KERNEL),
					   __builtin_return_address(0));
	if (!addr)
		goto destroy_genpool;

	ret = gen_pool_add_virt(atomic_pool, (unsigned long)addr,
				page_to_phys(page), atomic_pool_size, -1);
	if (ret)
		goto remove_mapping;
	gen_pool_set_algo(atomic_pool, gen_pool_first_fit_order_align, NULL);

	pr_info("DMA: preallocated %zu KiB pool for atomic allocations\n",
		atomic_pool_size / 1024);

#ifdef CONFIG_REALTEK_MEMORY_MANAGEMENT
	if (rtk_record_insert(page_to_pfn(page) << PAGE_SHIFT,
		RTK_SIGNATURE | DRIVER_ID | count_page, 0,
		(const void *)dma_atomic_pool_init, DVR_OWNER_NOID)) {
		goto record_delete;
	}

	if (need_debug_cma_guardband()) {
		dvr_guardband_padding(page, count_order, count_page);
	}
#endif
	return 0;

#ifdef CONFIG_REALTEK_MEMORY_MANAGEMENT
record_delete:
	{
	int value;
	value = rtk_record_lookup(page_to_pfn(page) << PAGE_SHIFT, NULL);
	if ((value & 0xf0000000) != RTK_SIGNATURE) {
		pr_err("remap: free memory (%lx) signature error...\n", page_to_pfn(page) << PAGE_SHIFT);
		BUG();
	}
	rtk_record_delete(page_to_pfn(page) << PAGE_SHIFT);
	}
#endif
remove_mapping:
	dma_common_free_remap(addr, atomic_pool_size);
destroy_genpool:
	gen_pool_destroy(atomic_pool);
	atomic_pool = NULL;
free_page:
#ifdef CONFIG_REALTEK_MEMORY_MANAGEMENT
	free_contiguous_memory(page, count);
#else
	if (!dma_release_from_contiguous(NULL, page, nr_pages))
		__free_pages(page, pool_size_order);
#endif
out:
	pr_err("DMA: failed to allocate %zu KiB pool for atomic coherent allocation\n",
		atomic_pool_size / 1024);
	return -ENOMEM;
}
postcore_initcall(dma_atomic_pool_init);

bool dma_in_atomic_pool(void *start, size_t size)
{
	if (unlikely(!atomic_pool))
		return false;

	return addr_in_gen_pool(atomic_pool, (unsigned long)start, size);
}

void *dma_alloc_from_pool(size_t size, struct page **ret_page, gfp_t flags)
{
	unsigned long val;
	void *ptr = NULL;

	if (!atomic_pool) {
		WARN(1, "coherent pool not initialised!\n");
		return NULL;
	}

	val = gen_pool_alloc(atomic_pool, size);
	if (val) {
		phys_addr_t phys = gen_pool_virt_to_phys(atomic_pool, val);

		*ret_page = pfn_to_page(__phys_to_pfn(phys));
		ptr = (void *)val;
		memset(ptr, 0, size);
	}

	return ptr;
}

bool dma_free_from_pool(void *start, size_t size)
{
	if (!dma_in_atomic_pool(start, size))
		return false;
	gen_pool_free(atomic_pool, (unsigned long)start, size);
	return true;
}

void *arch_dma_alloc(struct device *dev, size_t size, dma_addr_t *dma_handle,
		gfp_t flags, unsigned long attrs)
{
	struct page *page = NULL;
	void *ret;
#ifdef CONFIG_REALTEK_MEMORY_MANAGEMENT
	size_t count;
	int size_page, size_order;
	int count_page, count_order;
#endif

	size = PAGE_ALIGN(size);

	if (!gfpflags_allow_blocking(flags)) {
		ret = dma_alloc_from_pool(size, &page, flags);
		if (!ret)
			return NULL;
		goto done;
	}

#ifdef CONFIG_REALTEK_MEMORY_MANAGEMENT
	size = size_page = PAGE_ALIGN(size);
	count = count_page = size >> PAGE_SHIFT;

	if (need_debug_cma_guardband()) {
		size = size_order = dvr_guardband_size(size);
		count = count_order = size_order >> PAGE_SHIFT;
	}
#endif
#ifndef CONFIG_REALTEK_MEMORY_MANAGEMENT
	page = __dma_direct_alloc_pages(dev, size, dma_handle, flags, attrs);
#else
    page = alloc_contiguous_memory(size, GFP_DCU1, return_address(3));
#endif
	if (!page)
		return NULL;

#ifdef CONFIG_REALTEK_MEMORY_MANAGEMENT
	if (rtk_record_insert(page_to_pfn(page) << PAGE_SHIFT, RTK_SIGNATURE | DRIVER_ID | count_page, 0, return_address(3), DVR_OWNER_NOID)) {
		free_contiguous_memory(page, count);
		return NULL;
	}
#endif

	/* remove any dirty cache lines on the kernel alias */
	arch_dma_prep_coherent(page, size);

	/* create a coherent mapping */
	ret = dma_common_contiguous_remap(page, size,
			dma_pgprot(dev, PAGE_KERNEL, attrs),
			__builtin_return_address(0));
	if (!ret) {
#ifdef CONFIG_REALTEK_MEMORY_MANAGEMENT
		int value;

		value = rtk_record_lookup(page_to_pfn(page) << PAGE_SHIFT, NULL);
		if ((value & 0xf0000000) != RTK_SIGNATURE) {
			pr_err("remap: free memory (%lx) signature error...\n", page_to_pfn(page) << PAGE_SHIFT);
			BUG();
		}
		rtk_record_delete(page_to_pfn(page) << PAGE_SHIFT);
		free_contiguous_memory(page, count);
#else
		__dma_direct_free_pages(dev, size, page);
#endif
		return ret;
	}

	memset(ret, 0, size);

#ifdef CONFIG_REALTEK_MEMORY_MANAGEMENT
	if (need_debug_cma_guardband()) {
		dvr_guardband_padding(page, count_order, count_page);
	}
#endif

done:
	*dma_handle = phys_to_dma(dev, page_to_phys(page));
	return ret;
}

void arch_dma_free(struct device *dev, size_t size, void *vaddr,
		dma_addr_t dma_handle, unsigned long attrs)
{
	if (!dma_free_from_pool(vaddr, PAGE_ALIGN(size))) {
		phys_addr_t phys = dma_to_phys(dev, dma_handle);
		struct page *page = pfn_to_page(__phys_to_pfn(phys));

#ifdef CONFIG_REALTEK_MEMORY_MANAGEMENT
		int count = size >> PAGE_SHIFT;
		int value;
		rtk_record *ptr = NULL;

		value = rtk_record_lookup_ex(page_to_pfn(page) << PAGE_SHIFT, &ptr, true);
		if ((value & 0xf0000000) != RTK_SIGNATURE) {
			pr_err("remap: free memory (%lx) signature error...\n", page_to_pfn(page) << PAGE_SHIFT);
			BUG();
		}


		if (need_debug_cma_deferfree()) {
			rtk_record_tag_set(ptr, RTK_RECORD_TAG_DEFER_FREE);
			/* defer free not alloc, mark it */
			//rtk_record_delete(page_to_pfn(page) << PAGE_SHIFT);
		} else {
			rtk_record_delete(page_to_pfn(page) << PAGE_SHIFT);
		}

		if (((value & ID_MASK) == DRIVER_ID)) {
			if (need_debug_cma_guardband()) {
				count = dvr_guardband_check(page_to_pfn(page) << PAGE_SHIFT, value);
			} else {
				count = value & 0x00ffffff;
			}
		}
#endif

		vunmap(vaddr);
#ifdef CONFIG_REALTEK_MEMORY_MANAGEMENT
		if (need_debug_cma_deferfree()) {
			/* defer free not alloc, mark it */
			//dma_release_from_contiguous(dev, page, size >> PAGE_SHIFT);
			dvr_guardband_padding(page, 1, 0);
		} else
			free_contiguous_memory(page, count);
#else
		__dma_direct_free_pages(dev, size, page);
#endif
	}
}

long arch_dma_coherent_to_pfn(struct device *dev, void *cpu_addr,
		dma_addr_t dma_addr)
{
	return __phys_to_pfn(dma_to_phys(dev, dma_addr));
}
#endif /* CONFIG_DMA_DIRECT_REMAP */
