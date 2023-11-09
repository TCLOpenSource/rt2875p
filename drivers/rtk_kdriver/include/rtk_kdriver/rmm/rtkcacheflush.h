#ifndef _RTK_CACHEFLUSH_H_
#define _RTK_CACHEFLUSH_H_

#ifdef CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#ifdef CONFIG_ARM64

#define flush_cache_all()

/* invalidate + clean 
 * input: virtual address
 */
void dmac_flush_range(const void *virt_start, const void *virt_end);

/* clean only 
 * input: virtual address
 */
void dmac_clean_range(const void *virt_start, const void *virt_end);

/* invalidate only 
 * input: virtual address
 */
void dmac_inv_range(const void *virt_start, const void *virt_end);


/* invalidate + clean 
 * input: physical address
 */
#define outer_flush_range(a, b)
/* clean only 
 * input: physical address
 */
#define outer_clean_range(a, b)
/* invalidate only 
 * input: physical address
 */
#define outer_inv_range(a, b)

#endif // CONFIG_ARM64

/* invalidate + clean */
void rtk_flush_range(const void *virt_start, const void *virt_end);
/* clean only */
void rtk_clean_range(const void *virt_start, const void *virt_end);
/* invalidate only */
void rtk_inv_range(const void *virt_start, const void *virt_end);

#endif
#endif // CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE

#endif // _RTK_CACHEFLUSH_H_