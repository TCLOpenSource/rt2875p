#include <linux/version.h>
#include <linux/module.h>
#include <rtd_log/rtd_module_log.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/auth.h>
#include <linux/memblock.h>
#include <linux/highmem.h>

#include <asm/cacheflush.h>
#include <rtk_kdriver/rmm/rtkcacheflush.h>
#include <mach/iomap.h>

#ifdef CONFIG_CMA_RTK_ALLOCATOR
#include <linux/rtkrecord.h>
#include <linux/rtkblueprint.h>
#include <linux/pageremap.h>
#include <linux/auth.h>

void *dma_get_allocator(struct device *dev);
#endif

#ifndef CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0))
#define MMAP_LOCK_READ_DOWN(x)	down_read(& x->mmap_lock)
#define MMAP_LOCK_READ_UP(x)	up_read(& x->mmap_lock)
#define MMAP_LOCK_WRITE_DOWN(x)	down_write(& x->mmap_lock)
#define MMAP_LOCK_WRITE_UP(x)	up_write(& x->mmap_lock)

#define DO_MMAP(f, addr, l, prot, flags, off, pop, uf) do_mmap(f, addr, l, prot, flags, off, pop, uf)
#define DO_MUNMAP(mm, s, l, uf) do_munmap(mm, s, l, uf)
#else
#define MMAP_LOCK_READ_DOWN(x)	down_read(& x->mmap_sem)
#define MMAP_LOCK_READ_UP(x)	up_read(& x->mmap_sem)
#define MMAP_LOCK_WRITE_DOWN(x)	down_write(& x->mmap_sem)
#define MMAP_LOCK_WRITE_UP(x)	up_write(& x->mmap_sem)

#define DO_MMAP(f, addr, l, prot, flags, off, pop, uf) do_mmap_pgoff(f, addr, l, prot, flags, off, pop, uf)
#define DO_MUNMAP(mm, s, l, uf) do_munmap(mm, s, l, uf)
#endif
#define TLBMAP_FILE NULL

#else /* CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE */
#define MMAP_LOCK_READ_DOWN(x)
#define MMAP_LOCK_READ_UP(x)
#define MMAP_LOCK_WRITE_DOWN(x)
#define MMAP_LOCK_WRITE_UP(x)

#define DO_MMAP(f, addr, l, prot, flags, off, pop, uf) vm_mmap(f, addr, l, prot, flags, off)
#define DO_MUNMAP(mm, s, l, uf) vm_munmap(s, l)

#define TLBMAP_FILE filp
#endif

#define PAD_SIZE       0x10000
//#define DEBUG_MSG

static DEFINE_MUTEX(bitmap_mutex);

void rtktlb_init(void)
{
	rtd_pr_rmm_info("rtktlb: Hello, Realtek TLB Mapper...\n");
}

void rtktlb_exit(void)
{
	rtd_pr_rmm_info("rtktlb: Goodbye, Realtek TLB Mapper...\n");
}

#ifdef DEBUG_MSG
static void print_page_entry(unsigned long addr)
{
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *ptep, pte;
//	spinlock_t *ptl;

	pgd = pgd_offset(current->mm, addr);
	pud = pud_offset(pgd, addr);
	pmd = pmd_offset(pud, addr);
//	ptep = pte_offset_map_lock(current->mm, pmd, addr, &ptl);
	ptep = pte_offset_kernel(pmd, addr);
	pte = *ptep;
	if ((int)(pte_val(pte)) == 0)
		rtd_pr_rmm_info(" address: 0x%lx has no value...\n", addr);
	else {
		rtd_pr_rmm_info(" address: 0x%lx value %08x...\n", addr, (int)pte_val(pte));
		pte = *(ptep+512);
		rtd_pr_rmm_info(" address: 0x%lx value2 %08x...\n", addr, (int)pte_val(pte));
	}
//	pte_unmap_unlock(ptep, ptl);
}
#endif

static const char *rtktlb_arch_vma_name(struct vm_area_struct *vma)
{
	return NULL;
}

static void show_map_vma(struct vm_area_struct *vma)
{
	struct mm_struct *mm = vma->vm_mm;
	struct file *file = vma->vm_file;
	vm_flags_t flags = vma->vm_flags;
	unsigned long ino = 0;
	unsigned long long pgoff = 0;
	unsigned long start, end;
	dev_t dev = 0;
	const char *name = NULL;
	char buf[20];

	if (file) {
		struct inode *inode = file_inode(vma->vm_file);
		dev = inode->i_sb->s_dev;
		ino = inode->i_ino;
		pgoff = ((loff_t)vma->vm_pgoff) << PAGE_SHIFT;
	}

	/* We don't show the stack guard page in /proc/maps */
	start = vma->vm_start;
	end = vma->vm_end;

	rtd_pr_rmm_err("%08lx-%08lx %c%c%c%c %08llx %02x:%02x %6lu",
			start,
			end,
			flags & VM_READ ? 'r' : '-',
			flags & VM_WRITE ? 'w' : '-',
			flags & VM_EXEC ? 'x' : '-',
			flags & VM_MAYSHARE ? 's' : 'p',
			pgoff,
			MAJOR(dev), MINOR(dev), ino);

	/*
	 * Print the dentry name for named mappings, and a
	 * special [heap] marker for the heap:
	 */
	if (file) {
		buf[0] = '\0';
		name = d_path(&file->f_path, buf, sizeof(buf));
		buf[sizeof(buf)-1] = '\0';
		goto done;
	}

	name = rtktlb_arch_vma_name(vma);
	if (!name) {
		if (!mm) {
			name = "[vdso]";
			goto done;
		}

		if (vma->vm_start <= mm->brk &&
		    vma->vm_end >= mm->start_brk) {
			name = "[heap]";
			goto done;
		}

		if (vma->vm_start <= mm->start_stack &&
		    vma->vm_end >= mm->start_stack) {
			name = "[stack]";
			goto done;
		}
	}

done:
	if (!IS_ERR_OR_NULL(name))
		pr_cont("\t%s", name);
	pr_cont("\n");
}

/* using be careful, no protection here. */
void show_maps(struct mm_struct *mm)
{
	struct vm_area_struct *vma;

	MMAP_LOCK_READ_DOWN(mm);

	for (vma = mm->mmap; vma; vma = vma->vm_next) {
		show_map_vma(vma);
	}

	MMAP_LOCK_READ_UP(mm);
}

unsigned long pli_get_unmapped_area(struct file *filp, unsigned long addr,
unsigned long len, unsigned long pgoff, unsigned long flags)
{
	struct vm_area_struct * vmm;
#ifdef CONFIG_CMA_RTK_ALLOCATOR
	char info[16];
#endif	
	vmm = find_vma(current->mm, addr);
	if (!vmm || addr + len <= vmm->vm_start) {
		return addr;
	} else {
		rtd_pr_rmm_err("%s(%d): %08lx find a conflicting vma: %08lx-%08lx \n", __FUNCTION__, __LINE__, 
			addr, vmm->vm_start, vmm->vm_end);

#ifdef CONFIG_CMA_RTK_ALLOCATOR

		memset(info,0,sizeof(info));
		sprintf(info, "%lu", (unsigned long)current->pid);
		list_all_rtk_memory_allocation_sort(list_pid_mem, NULL, info);
#endif

		show_maps(current->mm);

		BUG();
		return -EINVAL;
	}
}

unsigned long pli_map_memory(struct file *filp, unsigned long phys_addr, int size)
{
	unsigned long caddr, uaddr, pageno;
	struct vm_area_struct *pli_area;
#ifndef CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE
	unsigned long (*get_unmapped_area) (struct file *, unsigned long, unsigned long, unsigned long, unsigned long);
#endif
	mem_map_info *map = (mem_map_info *)filp->private_data;
	unsigned long *bitmap = map->bitmap;
	unsigned long populate = 0;

	BUG_ON(phys_addr&~PAGE_MASK);
	size = PAGE_ALIGN(size);

	mutex_lock(&bitmap_mutex);
	pageno = bitmap_find_next_zero_area(bitmap, DEF_MEM_SIZE / PAGE_SIZE, 0, 
			(size + PAD_SIZE) / PAGE_SIZE, 0);
	if (pageno >= DEF_MEM_SIZE / PAGE_SIZE) {
		rtd_pr_rmm_err("%s(%d): not enough virtual memory in user space...\n", __FUNCTION__, __LINE__);
		BUG();
	}
	bitmap_set(bitmap, pageno, (size + PAD_SIZE) / PAGE_SIZE);
	mutex_unlock(&bitmap_mutex);

	MMAP_LOCK_WRITE_DOWN(current->mm);
#ifndef CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE
	get_unmapped_area = current->mm->get_unmapped_area;
	current->mm->get_unmapped_area = pli_get_unmapped_area;
#endif
	caddr = DO_MMAP(TLBMAP_FILE, (pageno << PAGE_SHIFT)+DEF_MAP_ADDR, size, PROT_READ | PROT_WRITE,
			MAP_FIXED | MAP_SHARED | MAP_NORESERVE, 0, &populate, NULL);

        if ((caddr & 0x80000000) || populate)
                rtd_pr_rmm_info("caddr-1=%lx/%lx\n", caddr, populate);
	BUG_ON((caddr & 0x80000000) || populate);

	uaddr = DO_MMAP(TLBMAP_FILE, (pageno << PAGE_SHIFT)+DEF_MAP_ADDR+DEF_MEM_SIZE, size, PROT_READ | PROT_WRITE,
			MAP_FIXED | MAP_SHARED | MAP_NORESERVE, 0, &populate, NULL);

        if ((uaddr & 0x80000000) || populate)
                rtd_pr_rmm_info("uaddr-2=%lx/%lx\n", uaddr, populate);
	BUG_ON((caddr & 0x80000000) || populate);

	rtd_pr_rmm_info("+++ map phy: %lx vir: %lx\n", phys_addr, caddr);

	// map cacheable zone
	pli_area = find_vma(current->mm, caddr);
	BUG_ON(pli_area == NULL);
	BUG_ON(pli_area->vm_start != caddr);
//	pli_area->vm_page_prot = pgprot_cached(pli_area->vm_page_prot);
	BUG_ON(remap_pfn_range(pli_area, caddr, phys_addr >> PAGE_SHIFT, size, pli_area->vm_page_prot));

//	pli_area->vm_flags = (pli_area->vm_flags /*| VM_PLI*/);
	pli_area->vm_flags = (pli_area->vm_flags & ~VM_IO);

	// map uncacheable zone
	pli_area = find_vma(current->mm, uaddr);
	BUG_ON(pli_area == NULL);
	BUG_ON(pli_area->vm_start != uaddr);
#ifdef CONFIG_ARM64
	pli_area->vm_page_prot = pgprot_writecombine(pli_area->vm_page_prot); // normal memory type
#else
	pli_area->vm_page_prot = pgprot_noncached(pli_area->vm_page_prot);
#endif
	BUG_ON(remap_pfn_range(pli_area, uaddr, phys_addr >> PAGE_SHIFT, size, pli_area->vm_page_prot));

//	pli_area->vm_flags = (pli_area->vm_flags /*| VM_PLI*/);
	pli_area->vm_flags = (pli_area->vm_flags & ~VM_IO);
#ifndef CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE
	current->mm->get_unmapped_area = get_unmapped_area;
#endif
	MMAP_LOCK_WRITE_UP(current->mm);

	/* work-around for tlb table-walk problem */
	{
		unsigned long *vaddr;
		struct page *page = pfn_to_page(phys_addr >> PAGE_SHIFT);

		vaddr = kmap_atomic(page);
		*vaddr = phys_addr;
		kunmap_atomic(vaddr);
	}

	// we need to flush cache after we map the memory into user space...
	uaccess_ttbr0_enable();
	dmac_flush_range((void *)caddr, (void *)caddr + size);
	uaccess_ttbr0_disable();
	outer_flush_range(phys_addr, phys_addr + size);

	return caddr;
}

unsigned long pli_unmap_memory(struct file *filp, unsigned long virt_addr)
{
	struct vm_area_struct *pli_area;
	unsigned long pfn;
	int ret, size;
	mem_map_info *map = (mem_map_info *)filp->private_data;
	unsigned long *bitmap = map->bitmap;

	rtd_pr_rmm_info("--- unmap virtual: %lx\n", virt_addr);

	MMAP_LOCK_WRITE_DOWN(current->mm);

	pli_area = find_vma(current->mm, virt_addr);
	BUG_ON(pli_area == NULL);
	BUG_ON(pli_area->vm_start != virt_addr);
	BUG_ON(follow_pfn(pli_area, virt_addr, &pfn));
	size = pli_area->vm_end - pli_area->vm_start;
	if (current->mm) {
		ret = DO_MUNMAP(current->mm, pli_area->vm_start + DEF_MEM_SIZE, size, NULL);
		BUG_ON(ret);
		ret = DO_MUNMAP(current->mm, pli_area->vm_start, size, NULL);
		BUG_ON(ret);
	}

	MMAP_LOCK_WRITE_UP(current->mm);

	mutex_lock(&bitmap_mutex);
	bitmap_clear(bitmap, (virt_addr - DEF_MAP_ADDR) >> PAGE_SHIFT,
			(size + PAD_SIZE) / PAGE_SIZE);
	mutex_unlock(&bitmap_mutex);
	return pfn;
}

unsigned long rtktlb_mmap(unsigned long addr)
{
	struct vm_area_struct *rtk_area;
	unsigned long startaddr;
	unsigned long populate = 0;
#ifdef DEBUG_MSG
	int i, j;
#endif

	MMAP_LOCK_WRITE_DOWN(current->mm);
	startaddr = DO_MMAP(NULL, addr + DEF_MAP_SIZE, RBUS_USER_MAP_SIZE, PROT_READ | PROT_WRITE, 
		MAP_FIXED | MAP_SHARED | MAP_NORESERVE, 0, &populate, NULL);
	BUG_ON(populate);

	if (startaddr != (addr + DEF_MAP_SIZE)) {
		DO_MUNMAP(current->mm, startaddr, RBUS_USER_MAP_SIZE, NULL);
		MMAP_LOCK_WRITE_UP(current->mm);
		return -EINVAL;
	} else {
		startaddr = addr;
	}

	rtk_area = find_vma(current->mm, addr + DEF_MAP_SIZE);
	BUG_ON(!rtk_area);
#if defined(CONFIG_ARM64)
	rtk_area->vm_page_prot = pgprot_device(rtk_area->vm_page_prot);
#else
	rtk_area->vm_page_prot = pgprot_rtk_device(rtk_area->vm_page_prot);
#endif
	if (remap_pfn_range(rtk_area, addr + DEF_MAP_SIZE, RBUS_BASE_PHYS >> 12, RBUS_USER_MAP_SIZE, rtk_area->vm_page_prot))
		rtd_pr_rmm_err("rtktlb: error in remap_page_range() for I/O...\n");

	MMAP_LOCK_WRITE_UP(current->mm);

#ifdef DEBUG_MSG
	i = addr+DEF_MAP_SIZE;
	j = addr+DEF_MAP_SIZE+RBUS_USER_MAP_SIZE;
	while (i < j) {
		print_page_entry((unsigned long)i);
		i += 0x1000;
	}
#endif

	return startaddr;
}

unsigned long rtktlb_munmap(unsigned long addr)
{
	int ret = 0;

	if (current->mm) {
		MMAP_LOCK_WRITE_DOWN(current->mm);
		ret = DO_MUNMAP(current->mm, addr + DEF_MAP_SIZE, RBUS_USER_MAP_SIZE, NULL);
		MMAP_LOCK_WRITE_UP(current->mm);
	}

	return ret;
}
