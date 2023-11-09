#include <linux/version.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <rtd_log/rtd_module_log.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/cma.h>
#include <linux/pageremap.h>
#include <linux/rtkblueprint.h>

#include "cma.h"



/****************************************set pte attribute******************************/
struct page_change_data {
    pgprot_t set_mask;
    pgprot_t clear_mask;
};


static struct mm_struct *record_mm_from_init_task = NULL;
static void   *kernel_pgd_addr = NULL;

static inline uint64_t read_ttbr1(void)
{ 
	uint64_t val64 = 0;

	asm volatile("mrs %0, ttbr1_el1" : "=r" (val64));
	//use phy value
	val64  =  val64 & 0xFFFFFFFFFFFE;
	return val64;
}

int rtk_set_attribute_init(void)
{
#if  (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
	/*for rtk_set_memory_valid record init_mm.pgd va*/
	record_mm_from_init_task = init_task.active_mm; 
 #endif 
	//rtd_pr_rmm_err("KIMAGE_VADDR=%llx v1=%llx v2=%llx\n", KIMAGE_VADDR, phys_to_virt(read_ttbr1()), read_ttbr1());
	kernel_pgd_addr = (void *)((read_ttbr1() + KIMAGE_VADDR));
	rtd_pr_rmm_err("rmm_svp: swapper_pgd_addr = %llx\n", kernel_pgd_addr);
	//rtd_pr_rmm_err("rmm_svp: init_mm.pgd = %llx\n", init_mm.pgd);
	return 0;
}

static int change_page_range(pte_t *ptep, unsigned long addr, void *data)
{
	struct page_change_data *cdata = data;
	pte_t pte = READ_ONCE(*ptep);

	pte = clear_pte_bit(pte, cdata->clear_mask);
	pte = set_pte_bit(pte, cdata->set_mask);

	set_pte(ptep, pte);
	return 0;
}
#if  (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
static int rtk_apply_to_page_range(struct mm_struct *mm, unsigned long saddr,
				unsigned long size, pte_fn_t fn,
				void *data)
{
	pgd_t *pgdp;
	pgd_t *pgd_h;
	pgd_t pgd;
	unsigned long addr = saddr;

	pgd_h  = (pgd_t *)kernel_pgd_addr;


	do{
		if(addr == saddr + size)
				break;
		pgdp = pgd_offset_pgd(pgd_h, addr);
		pgd = READ_ONCE(*pgdp);
		do {
			p4d_t *p4dp, p4d;
			pud_t *pudp, pud;
			pmd_t *pmdp, pmd;
			pte_t *ptep, pte;

			if (pgd_none(pgd) || pgd_bad(pgd))
					break;

			p4dp = p4d_offset(pgdp, addr);
			p4d = READ_ONCE(*p4dp);
			if (p4d_none(p4d) || p4d_bad(p4d))
					break;

			pudp = pud_offset(p4dp, addr);
			pud = READ_ONCE(*pudp);
			if (pud_none(pud) || pud_bad(pud))
					break;

			pmdp = pmd_offset(pudp, addr);
			pmd = READ_ONCE(*pmdp);
			if (pmd_none(pmd) || pmd_bad(pmd))
					break;

			ptep = pte_offset_map(pmdp, addr);
			pte = READ_ONCE(*ptep);
			change_page_range(ptep, addr, data);
			pte_unmap(ptep);
		} while(0);

		addr += PAGE_SIZE;
	}while(1);

	return 0;
}

#else
static int rtk_apply_to_page_range(struct mm_struct *mm, unsigned long saddr,
                                unsigned long size, pte_fn_t fn,
                                void *data){
	return 0;
}

#endif

static int __change_memory_common(unsigned long start, unsigned long size,
                pgprot_t set_mask, pgprot_t clear_mask)
{
    struct page_change_data data;
    int ret;

    struct mm_struct *mm =NULL;

	mm = record_mm_from_init_task ;
	data.set_mask = set_mask;
	data.clear_mask = clear_mask;

#if 0
#ifndef MODULE
	if(mm)
			ret = apply_to_page_range(mm, start, size, change_page_range,
							&data);
	else
#endif
#endif
			ret = rtk_apply_to_page_range(mm, start, size, change_page_range,
							&data);

    flush_tlb_kernel_range(start, start + size);
    return ret;
}

int rtk_set_memory_valid(unsigned long addr, unsigned long size, int enable)
{
    if (enable)
        return __change_memory_common(addr, size,
                    __pgprot(PTE_VALID),
                    __pgprot(0));
    else
        return __change_memory_common(addr, size,
                    __pgprot(0),
                    __pgprot(PTE_VALID));
}


EXPORT_SYMBOL(rtk_set_memory_valid);

MODULE_AUTHOR("Realtek.com");
MODULE_LICENSE("Dual BSD/GPL");
