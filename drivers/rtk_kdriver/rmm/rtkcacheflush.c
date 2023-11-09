#include <linux/version.h>

#ifdef CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#ifdef CONFIG_ARM64
#include <linux/dma-mapping.h>
#include <rtk_kdriver/rmm/rmm.h>
#include <rtk_kdriver/rmm/rtkcacheflush.h>
struct device rtkcache_dev = { .dma_ops = NULL, .dma_coherent = 0 };

extern void __rtk_dma_flush_area(const void *, unsigned long);
extern void __rtk_dma_clean_area(const void *, unsigned long);
extern void __rtk_dma_inv_area(const void *, unsigned long);

/* invalidate + clean 
 * input: virtual address
 */
void dmac_flush_range(const void *virt_start, const void *virt_end)
{
	__rtk_dma_flush_area(virt_start, (unsigned long)(virt_end - virt_start));
}
EXPORT_SYMBOL(dmac_flush_range);

/* clean only 
 * input: virtual address
 */
void dmac_clean_range(const void *virt_start, const void *virt_end)
{
	__rtk_dma_clean_area(virt_start, (unsigned long)(virt_end - virt_start));
}
EXPORT_SYMBOL(dmac_clean_range);

/* invalidate only 
 * input: virtual address
 */
void dmac_inv_range(const void *virt_start, const void *virt_end)
{
	__rtk_dma_inv_area(virt_start, (unsigned long)(virt_end - virt_start));
}
EXPORT_SYMBOL(dmac_inv_range);

void rtk_flush_range(const void *virt_start, const void *virt_end)
{
	dmac_flush_range(virt_start, virt_end);
#ifdef CONFIG_OUTER_CACHE
	outer_flush_range(__pa(virt_start), __pa(virt_end));
#endif
}
EXPORT_SYMBOL(rtk_flush_range);

void rtk_clean_range(const void *virt_start, const void *virt_end)
{
	dmac_clean_range(virt_start, virt_end);
#ifdef CONFIG_OUTER_CACHE
	outer_clean_range(__pa(virt_start), __pa(virt_end));
#endif
}
EXPORT_SYMBOL(rtk_clean_range);

void rtk_inv_range(const void *virt_start, const void *virt_end)
{
	dmac_inv_range(virt_start, virt_end);
#ifdef CONFIG_OUTER_CACHE
	outer_inv_range(__pa(virt_start), __pa(virt_end));
#endif
}
EXPORT_SYMBOL(rtk_inv_range);
#else
#include <asm/cacheflush.h>
#endif // CONFIG_ARM64
#endif

#endif // CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE