// SPDX-License-Identifier: GPL-2.0-only
/*
 * Based on arch/arm/mm/init.c
 *
 * Copyright (C) 1995-2005 Russell King
 * Copyright (C) 2012 ARM Ltd.
 */

#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/errno.h>
#include <linux/swap.h>
#include <linux/init.h>
#include <linux/cache.h>
#include <linux/mman.h>
#include <linux/nodemask.h>
#include <linux/initrd.h>
#include <linux/gfp.h>
#include <linux/memblock.h>
#include <linux/sort.h>
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/dma-mapping.h>
#include <linux/dma-contiguous.h>
#include <linux/efi.h>
#include <linux/swiotlb.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/kexec.h>
#include <linux/crash_dump.h>

#include <asm/boot.h>
#include <asm/fixmap.h>
#include <asm/kasan.h>
#include <asm/kernel-pgtable.h>
#include <asm/memory.h>
#include <asm/numa.h>
#include <asm/sections.h>
#include <asm/setup.h>
#include <linux/sizes.h>
#include <asm/tlb.h>
#include <asm/alternative.h>

#ifdef CONFIG_REALTEK_MEMORY_MANAGEMENT
#include <linux/pageremap.h>
#include <linux/auth.h>
#include <mach/common.h>
#include <mach/rtk_platform.h>

#define CARVEDOUT_LIMIT 512*_MB_
#endif

/*
 * We need to be able to catch inadvertent references to memstart_addr
 * that occur (potentially in generic code) before arm64_memblock_init()
 * executes, which assigns it its actual value. So use a default value
 * that cannot be mistaken for a real physical address.
 */
s64 memstart_addr __ro_after_init = -1;
EXPORT_SYMBOL(memstart_addr);

s64 physvirt_offset __ro_after_init;
EXPORT_SYMBOL(physvirt_offset);

struct page *vmemmap __ro_after_init;
EXPORT_SYMBOL(vmemmap);

phys_addr_t arm64_dma_phys_limit __ro_after_init;

unsigned int arm64_zone_dma32_bits __ro_after_init = 32;
EXPORT_SYMBOL(arm64_zone_dma32_bits);

#ifdef CONFIG_KEXEC_CORE
/*
 * reserve_crashkernel() - reserves memory for crash kernel
 *
 * This function reserves memory area given in "crashkernel=" kernel command
 * line parameter. The memory reserved is used by dump capture kernel when
 * primary kernel is crashing.
 */
static void __init reserve_crashkernel(void)
{
	unsigned long long crash_base, crash_size;
	int ret;

	ret = parse_crashkernel(boot_command_line, memblock_phys_mem_size(),
				&crash_size, &crash_base);
	/* no crashkernel= or invalid value specified */
	if (ret || !crash_size)
		return;

	crash_size = PAGE_ALIGN(crash_size);

	if (crash_base == 0) {
		/* Current arm64 boot protocol requires 2MB alignment */
		crash_base = memblock_find_in_range(0, ARCH_LOW_ADDRESS_LIMIT,
				crash_size, SZ_2M);
		if (crash_base == 0) {
			pr_warn("cannot allocate crashkernel (size:0x%llx)\n",
				crash_size);
			return;
		}
	} else {
		/* User specifies base address explicitly. */
		if (!memblock_is_region_memory(crash_base, crash_size)) {
			pr_warn("cannot reserve crashkernel: region is not memory\n");
			return;
		}

		if (memblock_is_region_reserved(crash_base, crash_size)) {
			pr_warn("cannot reserve crashkernel: region overlaps reserved memory\n");
			return;
		}

		if (!IS_ALIGNED(crash_base, SZ_2M)) {
			pr_warn("cannot reserve crashkernel: base address is not 2MB aligned\n");
			return;
		}
	}
	memblock_reserve(crash_base, crash_size);

	pr_info("crashkernel reserved: 0x%016llx - 0x%016llx (%lld MB)\n",
		crash_base, crash_base + crash_size, crash_size >> 20);

	crashk_res.start = crash_base;
	crashk_res.end = crash_base + crash_size - 1;
}
#else
static void __init reserve_crashkernel(void)
{
}
#endif /* CONFIG_KEXEC_CORE */

#ifdef CONFIG_CRASH_DUMP
static int __init early_init_dt_scan_elfcorehdr(unsigned long node,
		const char *uname, int depth, void *data)
{
	const __be32 *reg;
	int len;

	if (depth != 1 || strcmp(uname, "chosen") != 0)
		return 0;

	reg = of_get_flat_dt_prop(node, "linux,elfcorehdr", &len);
	if (!reg || (len < (dt_root_addr_cells + dt_root_size_cells)))
		return 1;

	elfcorehdr_addr = dt_mem_next_cell(dt_root_addr_cells, &reg);
	elfcorehdr_size = dt_mem_next_cell(dt_root_size_cells, &reg);

	return 1;
}

/*
 * reserve_elfcorehdr() - reserves memory for elf core header
 *
 * This function reserves the memory occupied by an elf core header
 * described in the device tree. This region contains all the
 * information about primary kernel's core image and is used by a dump
 * capture kernel to access the system memory on primary kernel.
 */
static void __init reserve_elfcorehdr(void)
{
	of_scan_flat_dt(early_init_dt_scan_elfcorehdr, NULL);

	if (!elfcorehdr_size)
		return;

	if (memblock_is_region_reserved(elfcorehdr_addr, elfcorehdr_size)) {
		pr_warn("elfcorehdr is overlapped\n");
		return;
	}

	memblock_reserve(elfcorehdr_addr, elfcorehdr_size);

	pr_info("Reserving %lldKB of memory at 0x%llx for elfcorehdr\n",
		elfcorehdr_size >> 10, elfcorehdr_addr);
}
#else
static void __init reserve_elfcorehdr(void)
{
}
#endif /* CONFIG_CRASH_DUMP */
/*
 * Return the maximum physical address for ZONE_DMA32 (DMA_BIT_MASK(32)). It
 * currently assumes that for memory starting above 4G, 32-bit devices will
 * use a DMA offset.
 */
static phys_addr_t __init max_zone_dma_phys(void)
{
	phys_addr_t offset = memblock_start_of_DRAM() & GENMASK_ULL(63, arm64_zone_dma32_bits);
	return min(offset + (1ULL << arm64_zone_dma32_bits), memblock_end_of_DRAM());
}

#ifdef CONFIG_NUMA

static void __init zone_sizes_init(unsigned long min, unsigned long max)
{
	unsigned long max_zone_pfns[MAX_NR_ZONES]  = {0};

#ifdef CONFIG_ZONE_DMA32
	max_zone_pfns[ZONE_DMA32] = PFN_DOWN(max_zone_dma_phys());
#endif
	max_zone_pfns[ZONE_NORMAL] = max;

	free_area_init_nodes(max_zone_pfns);
}

#else

static void __init zone_sizes_init(unsigned long min, unsigned long max)
{
	struct memblock_region *reg;
	unsigned long zone_size[MAX_NR_ZONES], zhole_size[MAX_NR_ZONES];
	unsigned long max_dma = min;

	memset(zone_size, 0, sizeof(zone_size));

	/* 4GB maximum for 32-bit only capable devices */
#ifdef CONFIG_ZONE_DMA32
	max_dma = PFN_DOWN(arm64_dma_phys_limit);
	zone_size[ZONE_DMA32] = max_dma - min;
#endif
	zone_size[ZONE_NORMAL] = max - max_dma;

	memcpy(zhole_size, zone_size, sizeof(zhole_size));

	for_each_memblock(memory, reg) {
		unsigned long start = memblock_region_memory_base_pfn(reg);
		unsigned long end = memblock_region_memory_end_pfn(reg);

		if (start >= max)
			continue;

#ifdef CONFIG_ZONE_DMA32
		if (start < max_dma) {
			unsigned long dma_end = min(end, max_dma);
			zhole_size[ZONE_DMA32] -= dma_end - start;
		}
#endif
		if (end > max_dma) {
			unsigned long normal_end = min(end, max);
			unsigned long normal_start = max(start, max_dma);
			zhole_size[ZONE_NORMAL] -= normal_end - normal_start;
		}
	}

#if 1 // debug
	{
		int i;
		for (i = 0; i < __MAX_NR_ZONES; i++)
			printk("[MEM] zone[%d] size %lx hole %lx\n", i, zone_size[i], zhole_size[i]);
	}
#endif

	free_area_init_node(0, zone_size, min, zhole_size);
}

static int __init early_arm64_zone_dma32_bits(char *p)
{
	unsigned int value = 32;

    if (!p) {
        return 1;
	}

	value = (unsigned int)memparse(p, &p);
	if (value >= 24 && value < 32) {
		arm64_zone_dma32_bits = value;
		pr_notice("zone_dma32 size adjust to %u MB\n", (1U << arm64_zone_dma32_bits) >> 20);
	}

    return 0;
}
early_param("arm64_zone_dma32_bits", early_arm64_zone_dma32_bits);

#endif /* CONFIG_NUMA */

int pfn_valid(unsigned long pfn)
{
	phys_addr_t addr = pfn << PAGE_SHIFT;

	if ((addr >> PAGE_SHIFT) != pfn)
		return 0;

#ifdef CONFIG_SPARSEMEM
	if (pfn_to_section_nr(pfn) >= NR_MEM_SECTIONS)
		return 0;

	if (!valid_section(__nr_to_section(pfn_to_section_nr(pfn))))
		return 0;

	/*
	 * ZONE_DEVICE memory does not have the memblock entries.
	 * memblock_is_map_memory() check for ZONE_DEVICE based
	 * addresses will always fail. Even the normal hotplugged
	 * memory will never have MEMBLOCK_NOMAP flag set in their
	 * memblock entries. Skip memblock search for all non early
	 * memory sections covering all of hotplug memory including
	 * both normal and ZONE_DEVICE based.
	 */
	if (!early_section(__pfn_to_section(pfn)))
		return pfn_section_valid(__pfn_to_section(pfn), pfn);
#endif
	return memblock_is_map_memory(addr);
}
EXPORT_SYMBOL(pfn_valid);

static phys_addr_t memory_limit = PHYS_ADDR_MAX;

/*
 * Limit the memory size that was specified via FDT.
 */
static int __init early_mem(char *p)
{
	if (!p)
		return 1;

	memory_limit = memparse(p, &p) & PAGE_MASK;
	pr_notice("Memory limited to %lldMB\n", memory_limit >> 20);

	return 0;
}
early_param("mem", early_mem);

static int __init early_init_dt_scan_usablemem(unsigned long node,
		const char *uname, int depth, void *data)
{
	struct memblock_region *usablemem = data;
	const __be32 *reg;
	int len;

	if (depth != 1 || strcmp(uname, "chosen") != 0)
		return 0;

	reg = of_get_flat_dt_prop(node, "linux,usable-memory-range", &len);
	if (!reg || (len < (dt_root_addr_cells + dt_root_size_cells)))
		return 1;

	usablemem->base = dt_mem_next_cell(dt_root_addr_cells, &reg);
	usablemem->size = dt_mem_next_cell(dt_root_size_cells, &reg);

	return 1;
}

static void __init fdt_enforce_memory_region(void)
{
	struct memblock_region reg = {
		.size = 0,
	};

	of_scan_flat_dt(early_init_dt_scan_usablemem, &reg);

	if (reg.size)
		memblock_cap_memory_range(reg.base, reg.size);
}

void __init arm64_memblock_init(void)
{
	const s64 linear_region_size = BIT(vabits_actual - 1);

	/* Handle linux,usable-memory-range property */
	fdt_enforce_memory_region();

	/* Remove memory above our supported physical address size */
	memblock_remove(1ULL << PHYS_MASK_SHIFT, ULLONG_MAX);

	/*
	 * Select a suitable value for the base of physical memory.
	 */
	memstart_addr = round_down(memblock_start_of_DRAM(),
				   ARM64_MEMSTART_ALIGN);

	physvirt_offset = PHYS_OFFSET - PAGE_OFFSET;

	vmemmap = ((struct page *)VMEMMAP_START - (memstart_addr >> PAGE_SHIFT));

	/*
	 * If we are running with a 52-bit kernel VA config on a system that
	 * does not support it, we have to offset our vmemmap and physvirt_offset
	 * s.t. we avoid the 52-bit portion of the direct linear map
	 */
	if (IS_ENABLED(CONFIG_ARM64_VA_BITS_52) && (vabits_actual != 52)) {
		vmemmap += (_PAGE_OFFSET(48) - _PAGE_OFFSET(52)) >> PAGE_SHIFT;
		physvirt_offset = PHYS_OFFSET - _PAGE_OFFSET(48);
	}

	/*
	 * Remove the memory that we will not be able to cover with the
	 * linear mapping. Take care not to clip the kernel which may be
	 * high in memory.
	 */
	memblock_remove(max_t(u64, memstart_addr + linear_region_size,
			__pa_symbol(_end)), ULLONG_MAX);
	if (memstart_addr + linear_region_size < memblock_end_of_DRAM()) {
		/* ensure that memstart_addr remains sufficiently aligned */
		memstart_addr = round_up(memblock_end_of_DRAM() - linear_region_size,
					 ARM64_MEMSTART_ALIGN);
		memblock_remove(0, memstart_addr);
	}

	/*
	 * Apply the memory limit if it was set. Since the kernel may be loaded
	 * high up in memory, add back the kernel region that must be accessible
	 * via the linear mapping.
	 */
	if (memory_limit != PHYS_ADDR_MAX) {
		memblock_mem_limit_remove_map(memory_limit);
		memblock_add(__pa_symbol(_text), (u64)(_end - _text));
	}

	if (IS_ENABLED(CONFIG_BLK_DEV_INITRD) && phys_initrd_size) {
		/*
		 * Add back the memory we just removed if it results in the
		 * initrd to become inaccessible via the linear mapping.
		 * Otherwise, this is a no-op
		 */
		u64 base = phys_initrd_start & PAGE_MASK;
		u64 size = PAGE_ALIGN(phys_initrd_start + phys_initrd_size) - base;

		/*
		 * We can only add back the initrd memory if we don't end up
		 * with more memory than we can address via the linear mapping.
		 * It is up to the bootloader to position the kernel and the
		 * initrd reasonably close to each other (i.e., within 32 GB of
		 * each other) so that all granule/#levels combinations can
		 * always access both.
		 */
		if (WARN(base < memblock_start_of_DRAM() ||
			 base + size > memblock_start_of_DRAM() +
				       linear_region_size,
			"initrd not fully accessible via the linear mapping -- please check your bootloader ...\n")) {
			phys_initrd_size = 0;
		} else {
			memblock_remove(base, size); /* clear MEMBLOCK_ flags */
			memblock_add(base, size);
			memblock_reserve(base, size);
		}
	}

	if (IS_ENABLED(CONFIG_RANDOMIZE_BASE)) {
		extern u16 memstart_offset_seed;
		u64 range = linear_region_size -
			    (memblock_end_of_DRAM() - memblock_start_of_DRAM());

		/*
		 * If the size of the linear region exceeds, by a sufficient
		 * margin, the size of the region that the available physical
		 * memory spans, randomize the linear region as well.
		 */
		if (memstart_offset_seed > 0 && range >= ARM64_MEMSTART_ALIGN) {
			range /= ARM64_MEMSTART_ALIGN;
			memstart_addr -= ARM64_MEMSTART_ALIGN *
					 ((range * memstart_offset_seed) >> 16);
		}
	}

	/*
	 * Register the kernel text, kernel data, initrd, and initial
	 * pagetables with memblock.
	 */
	memblock_reserve(__pa_symbol(_text), _end - _text);
	if (IS_ENABLED(CONFIG_BLK_DEV_INITRD) && phys_initrd_size) {
		/* the generic initrd code expects virtual addresses */
		initrd_start = __phys_to_virt(phys_initrd_start);
		initrd_end = initrd_start + phys_initrd_size;
	}

#if defined(RTK_MEM_LAYOUT_DEVICETREE) || defined(CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE)
    // no rtk_reserve, will do early_init_fdt_scan_reserved_mem / rmem_cma_setup

#ifndef CONFIG_ARM64 //OBSOLETED: all dts layout are 5.4+ 64
    /* M6PRTANOM-100, Reserved to avoid STR resume secondary_core text section be allocated.*/
	memblock_reserve(__pa(&_text), PAGE_SIZE);
#endif

#else // CONFIG_REALTEK_MEMORY_MANAGEMENT
	rtk_reserve(); //FIXME

#endif

	early_init_fdt_scan_reserved_mem();

	/* 4GB maximum for 32-bit only capable devices */
	if (IS_ENABLED(CONFIG_ZONE_DMA32))
		arm64_dma_phys_limit = max_zone_dma_phys();
	else
		arm64_dma_phys_limit = PHYS_MASK + 1;

	reserve_crashkernel();

	reserve_elfcorehdr();

	high_memory = __va(memblock_end_of_DRAM() - 1) + 1;

#ifndef CONFIG_REALTEK_MEMORY_MANAGEMENT
	dma_contiguous_reserve(arm64_dma_phys_limit);
#else

#if defined(RTK_MEM_LAYOUT_DEVICETREE)
    // leave it native upstream for GKI, don't dedicate cma2 to movable zone.
	dma_contiguous_reserve(arm64_dma_phys_limit);

#else //!defined(TEST_MEMORY_LAYOUT_DEVICETREE)

	rtk_reserve_cma(arm64_dma_phys_limit, arm64_dma_phys_limit);

	{
#ifdef CONFIG_VBM_CMA
		unsigned long carvedout_cma_vbm_size, carvedout_cma_vbm_start;
#endif

		int carvedout_in_highmem = 0;
		unsigned long carvedout_start = 0;
		unsigned long carvedout_end = 0;
		unsigned long carvedout_cma_high_size = 0;  // HighMem reserved size (GPU)
		unsigned long carvedout_high_size = 0;      // carvedout memory size in highmem
		int ret = 0;
		unsigned int layout_idx = carvedout_buf_get_layout_idx();

		carvedout_cma_high_size = carvedout_buf_query(CARVEDOUT_GPU_RESERVED, NULL);
		carvedout_high_size = carvedout_buf_query_range(CARVEDOUT_HIGH_START, CARVEDOUT_HIGH_END, (void **)&carvedout_start, (void **)&carvedout_end);
//		carvedout_high_size = fdt_get_carvedout_mem_size_in_highmem(high_start, max);

		if (arm64_dma_phys_limit == 0) {
			pr_err("[MEM] err: no lowmem limit\n");
			BUG();
		}

		if (memblock_end_of_DRAM() < CARVEDOUT_LIMIT) {
			pr_err("[MEM] err: memblock_end_of_DRAM() < 512MB\n");
			BUG();
		}
		kernel_highmem_size = memblock_end_of_DRAM() - CARVEDOUT_LIMIT; // hardcode for cma size calculation 

		if (carvedout_start >= CARVEDOUT_LIMIT) {
			carvedout_in_highmem = 1;
			printk("[MEM] %s(): carvedout is in highmem\n", __func__);
		} else if (carvedout_end > CARVEDOUT_LIMIT) {
			pr_err("[MEM] err: carvedout region cross memory zone !\n");
			BUG();
		}

		if (!carvedout_in_highmem) {
			carvedout_high_size = 0;
			printk("[MEM] carvedout in Low Mem, so needless to calculate carved-out for HighMem CMA\n");
		}

		if ((carvedout_high_size + ZRAM_RESERVED_SIZE + carvedout_cma_high_size) > kernel_highmem_size) {
			if (carvedout_high_size > kernel_highmem_size) {
				pr_err("[MEM] err: carvedout size(%lx) bigger than kernel highmem size(%lx)\n", carvedout_high_size, kernel_highmem_size);
				BUG();
			} else
				cma_highmem_size = kernel_highmem_size - carvedout_high_size;
		} else {
			cma_highmem_size = kernel_highmem_size - (carvedout_high_size + ZRAM_RESERVED_SIZE + carvedout_cma_high_size);
		}
		cma_highmem_start = memblock_end_of_DRAM() - cma_highmem_size;

		if (carvedout_in_highmem && (carvedout_end == memblock_end_of_DRAM())) { // carvedout is in last
			cma_highmem_start = memblock_end_of_DRAM() - carvedout_high_size - cma_highmem_size;
		}
		printk("[MEM] HighMem carved-out = 0x%x, high_cma_reserved = 0x%x\n", (unsigned int)carvedout_high_size, (unsigned int)carvedout_cma_high_size);

#if 0 // debug info for chekcing runtime cma2 arrangement
		printk("memblock_end_of_DRAM 0x%lx arm64_dma_phys_limit 0x%lx\n", memblock_end_of_DRAM(), arm64_dma_phys_limit);
		printk("carvedout_cma_high_size 0x%lx carvedout_high_size 0x%lx\n", carvedout_cma_high_size, carvedout_high_size);
		printk("kernel_highmem_size %lx cma_highmem_start 0x%lx size 0x%lx\n", kernel_highmem_size, cma_highmem_start, cma_highmem_size);
		printk("carvedout_cpb_addr 0x%08x CMA_GPU_4K_SIZE 0x%08x", carvedout_cpb_addr, CMA_GPU_4K_SIZE);
		printk("ui_res %d\n", get_ui_resolution());
#endif

		if ((get_ui_resolution() == UI_RESOLUTION_4K) && (cma_highmem_size > CMA_GPU_4K_SIZE)) {
#ifdef CONFIG_RTK_BUSINESS_DISPLAY_MODEL
			carvedout_buf[layout_idx][CARVEDOUT_CMA_GPU_4K][0] = CARVEDOUT_LIMIT;
#else
			carvedout_buf[layout_idx][CARVEDOUT_CMA_GPU_4K][0] = cma_highmem_start;
#endif
			carvedout_buf[layout_idx][CARVEDOUT_CMA_GPU_4K][1] = CMA_GPU_4K_SIZE;
			carvedout_buf[layout_idx][CARVEDOUT_CMA_HIGH][0] = cma_highmem_start + CMA_GPU_4K_SIZE;
			carvedout_buf[layout_idx][CARVEDOUT_CMA_HIGH][1] = cma_highmem_size - CMA_GPU_4K_SIZE;
		} else {
			carvedout_buf[layout_idx][CARVEDOUT_CMA_HIGH][0] = cma_highmem_start;
			carvedout_buf[layout_idx][CARVEDOUT_CMA_HIGH][1] = cma_highmem_size;
		}
		printk("reserve %08lx - %08lx for high memory cma...\n",
			   carvedout_buf[layout_idx][CARVEDOUT_CMA_HIGH][0], carvedout_buf[layout_idx][CARVEDOUT_CMA_HIGH][0] + carvedout_buf[layout_idx][CARVEDOUT_CMA_HIGH][1]);
		ret = memblock_reserve(carvedout_buf[layout_idx][CARVEDOUT_CMA_HIGH][0], carvedout_buf[layout_idx][CARVEDOUT_CMA_HIGH][1]);
		if (ret)
			pr_err("[MEM] memblock_reserve for highmem CMA failed, errno = %d\n", ret);

		if (carvedout_buf[layout_idx][CARVEDOUT_CMA_GPU_4K][1]) {
			pr_info("reserve %08lx - %08lx for cma gpu_4k...\n",
					carvedout_buf[layout_idx][CARVEDOUT_CMA_GPU_4K][0],
					carvedout_buf[layout_idx][CARVEDOUT_CMA_GPU_4K][0] + carvedout_buf[layout_idx][CARVEDOUT_CMA_GPU_4K][1]);
			ret = memblock_reserve(carvedout_buf[layout_idx][CARVEDOUT_CMA_GPU_4K][0], carvedout_buf[layout_idx][CARVEDOUT_CMA_GPU_4K][1]);

			if (ret)
				pr_err("[MEM] memblock_reserve for CMA_GPU_4K failed, errno = %d\n", ret);
		}

#ifdef CONFIG_VBM_CMA
		carvedout_cma_vbm_size = carvedout_buf_query(CARVEDOUT_CMA_VBM, (void **)&carvedout_cma_vbm_start);
		if (carvedout_cma_vbm_size) {
			pr_info("reserve %08lx - %08lx for high memory vbm cma...\n",
					carvedout_cma_vbm_start, carvedout_cma_vbm_start + carvedout_cma_vbm_size);
			ret = memblock_reserve(carvedout_cma_vbm_start, carvedout_cma_vbm_size);

			if (ret)
				pr_err("[MEM] memblock_reserve for CMA-VBM failed, errno = %d\n", ret);
		}
#endif

	}
#endif // defined(TEST_MEMORY_LAYOUT_DEVICETREE)

#endif
}

void __init bootmem_init(void)
{
	unsigned long min, max;

	min = PFN_UP(memblock_start_of_DRAM());
	max = PFN_DOWN(memblock_end_of_DRAM());

	early_memtest(min << PAGE_SHIFT, max << PAGE_SHIFT);

	max_pfn = max_low_pfn = max;
	min_low_pfn = min;

	arm64_numa_init();
	/*
	 * Sparsemem tries to allocate bootmem in memory_present(), so must be
	 * done after the fixed reservations.
	 */
	memblocks_present();

	sparse_init();
	zone_sizes_init(min, max);

	memblock_dump_all();
}

#ifndef CONFIG_SPARSEMEM_VMEMMAP
static inline void free_memmap(unsigned long start_pfn, unsigned long end_pfn)
{
	struct page *start_pg, *end_pg;
	unsigned long pg, pgend;

	/*
	 * Convert start_pfn/end_pfn to a struct page pointer.
	 */
	start_pg = pfn_to_page(start_pfn - 1) + 1;
	end_pg = pfn_to_page(end_pfn - 1) + 1;

	/*
	 * Convert to physical addresses, and round start upwards and end
	 * downwards.
	 */
	pg = (unsigned long)PAGE_ALIGN(__pa(start_pg));
	pgend = (unsigned long)__pa(end_pg) & PAGE_MASK;

	/*
	 * If there are free pages between these, free the section of the
	 * memmap array.
	 */
	if (pg < pgend)
		memblock_free(pg, pgend - pg);
}

/*
 * The mem_map array can get very big. Free the unused area of the memory map.
 */
static void __init free_unused_memmap(void)
{
	unsigned long start, prev_end = 0;
	struct memblock_region *reg;

	for_each_memblock(memory, reg) {
		start = __phys_to_pfn(reg->base);

#ifdef CONFIG_SPARSEMEM
		/*
		 * Take care not to free memmap entries that don't exist due
		 * to SPARSEMEM sections which aren't present.
		 */
		start = min(start, ALIGN(prev_end, PAGES_PER_SECTION));
#endif
		/*
		 * If we had a previous bank, and there is a space between the
		 * current bank and the previous, free it.
		 */
		if (prev_end && prev_end < start)
			free_memmap(prev_end, start);

		/*
		 * Align up here since the VM subsystem insists that the
		 * memmap entries are valid from the bank end aligned to
		 * MAX_ORDER_NR_PAGES.
		 */
		prev_end = ALIGN(__phys_to_pfn(reg->base + reg->size),
				 MAX_ORDER_NR_PAGES);
	}

#ifdef CONFIG_SPARSEMEM
	if (!IS_ALIGNED(prev_end, PAGES_PER_SECTION))
		free_memmap(prev_end, ALIGN(prev_end, PAGES_PER_SECTION));
#endif
}
#endif	/* !CONFIG_SPARSEMEM_VMEMMAP */

/*
 * mem_init() marks the free areas in the mem_map and tells us how much memory
 * is free.  This is done after various parts of the system have claimed their
 * memory after the kernel image.
 */
void __init mem_init(void)
{
	if (swiotlb_force == SWIOTLB_FORCE ||
	    max_pfn > (arm64_dma_phys_limit >> PAGE_SHIFT))
		swiotlb_init(1);
	else
		swiotlb_force = SWIOTLB_NO_FORCE;

	set_max_mapnr(max_pfn - PHYS_PFN_OFFSET);

#ifndef CONFIG_SPARSEMEM_VMEMMAP
	free_unused_memmap();
#endif
	/* this will put all unused low memory onto the freelists */
	memblock_free_all();

	mem_init_print_info(NULL);

	/*
	 * Check boundaries twice: Some fundamental inconsistencies can be
	 * detected at build time already.
	 */
#ifdef CONFIG_COMPAT
	BUILD_BUG_ON(TASK_SIZE_32 > DEFAULT_MAP_WINDOW_64);
#endif

	if (PAGE_SIZE >= 16384 && get_num_physpages() <= 128) {
		extern int sysctl_overcommit_memory;
		/*
		 * On a machine this small we won't get anywhere without
		 * overcommit, so turn it on by default.
		 */
		sysctl_overcommit_memory = OVERCOMMIT_ALWAYS;
	}
}

void free_initmem(void)
{
	free_reserved_area(lm_alias(__init_begin),
			   lm_alias(__init_end),
			   0, "unused kernel");
	/*
	 * Unmap the __init region but leave the VM area in place. This
	 * prevents the region from being reused for kernel modules, which
	 * is not supported by kallsyms.
	 */
	unmap_kernel_range((u64)__init_begin, (u64)(__init_end - __init_begin));
}

#ifdef CONFIG_BLK_DEV_INITRD
void __init free_initrd_mem(unsigned long start, unsigned long end)
{
	unsigned long aligned_start, aligned_end;

	aligned_start = __virt_to_phys(start) & PAGE_MASK;
	aligned_end = PAGE_ALIGN(__virt_to_phys(end));
	memblock_free(aligned_start, aligned_end - aligned_start);
	free_reserved_area((void *)start, (void *)end, 0, "initrd");
}
#endif

/*
 * Dump out memory limit information on panic.
 */
static int dump_mem_limit(struct notifier_block *self, unsigned long v, void *p)
{
	if (memory_limit != PHYS_ADDR_MAX) {
		pr_emerg("Memory Limit: %llu MB\n", memory_limit >> 20);
	} else {
		pr_emerg("Memory Limit: none\n");
	}
	return 0;
}

static struct notifier_block mem_limit_notifier = {
	.notifier_call = dump_mem_limit,
};

static int __init register_mem_limit_dumper(void)
{
	atomic_notifier_chain_register(&panic_notifier_list,
				       &mem_limit_notifier);
	return 0;
}
__initcall(register_mem_limit_dumper);
