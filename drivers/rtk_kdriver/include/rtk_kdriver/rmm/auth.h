#ifndef AUTH_H
#define AUTH_H

#include <linux/version.h>
#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */
#include <linux/sched.h>
#include <linux/dcache.h>
#include <linux/memblock.h>
#include <linux/device.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
#include <linux/dma-map-ops.h>
#else
#include <linux/dma-contiguous.h>
#endif

#include <mach/iomap.h>

//#define DEBUG_MSG

#define AUTH_MAJOR		0 // for dynamic alocation
#define AUTH_NAME		"auth device"

#define AUTH_EXT_MAJOR		0 // for dynamic alocation
#define AUTH_EXT_NAME		"auth_ext device"

typedef enum {
	DEV_CMA1_DEFAULT = 1, 
	DEV_CMA2,
	DEV_CMA3_GAL,
	DEV_CMA4_CMA1_LIMIT,
#ifdef CONFIG_OPTEE_SUPPORT_MC_ALLOCATOR
	DEV_CMA5_MC,
#endif
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
	DEV_CMA6_BW,
#endif
	DEV_CMA_GPU_4K,
	DEV_CMA_64BIT,
#ifdef CONFIG_VBM_CMA
	DEV_CMA_VBM,
#endif
#ifdef CONFIG_RTK_KDRV_PCIE
	DEV_CMA_PCIE,
#endif
	DEV_CMA_NUM
} rtkcma_dev_t;

#define DEF_MAP_ADDR		0x20000000
#define MAX_MAP_ADDR		0x60000000
#define DEF_MEM_SIZE		0x10000000
#define DEF_MAP_SIZE		0x20000000
#define STR_LENGTH	32

#define RBUS_USER_MAP_SIZE (RBUS_BASE_SIZE >> 2)  // here we just map 512KB

#define MMAP_PLI_LOW	(DEF_MAP_ADDR - 0x1000) // 4KB gap
#define MMAP_PLI_HIGH	(DEF_MAP_ADDR + DEF_MAP_SIZE + RBUS_USER_MAP_SIZE + 0x1000) // given 4KB gap

extern unsigned long cma_highmem_start;
extern unsigned long cma_highmem_size;
extern unsigned long kernel_highmem_size;

#ifdef	CONFIG_REALTEK_SCHED_LOG
typedef struct {
	unsigned long		addr;
	unsigned long		size;
} sched_log_struct;

typedef char COMM_STR[TASK_COMM_LEN];
#endif // CONFIG_REALTEK_SCHED_LOG

typedef char AUTH_STR[20];

typedef struct {
	unsigned long		size;
	unsigned long		flag;
	char			str[STR_LENGTH];
} mem_alloc_struct;

typedef struct {
	unsigned long		addr;
	unsigned long		size;
} mem_region_struct;

typedef struct {
	unsigned long		addr;
	unsigned long		*bitmap;
} mem_map_info;

#ifdef CONFIG_COMPAT
typedef struct {
    unsigned int        size;
    unsigned int        flag;
    char		 str[STR_LENGTH];
} compat_mem_alloc_struct;

typedef struct {
    unsigned int        addr;
    unsigned int        size;
} compat_mem_region_struct;

typedef struct {
    unsigned int        addr;
    unsigned int        *bitmap;
} compat_mem_map_info;
#endif

//extern struct device	*auth_dev;

struct rtkcma {
	const char *name;
	struct device *dev;
    struct page *cache;
    unsigned long *cache_bitmap;
	struct mem_bp *bp;				// blueprint
    int exist;
};
//extern struct rtkcma rtkcma_list[];

void			rtktlb_init(void);
void			rtktlb_exit(void);
unsigned long		rtktlb_mmap(unsigned long addr);
unsigned long		rtktlb_munmap(unsigned long addr);

unsigned long pli_get_unmapped_area(struct file *filp, unsigned long addr,
									unsigned long len, unsigned long pgoff, unsigned long flags);

unsigned long		pli_map_memory(struct file *filp, unsigned long phys_addr, int size);
unsigned long		pli_unmap_memory(struct file *filp, unsigned long virt_addr);

#ifdef CONFIG_REALTEK_MANAGE_OVERLAPPED_REGION
void			init_overlapped_region(unsigned long base_pfn, int count);
void			exit_overlapped_region(void);
unsigned long		alloc_overlapped_memory(size_t size);
void			free_overlapped_memory(unsigned long phys_addr);
#endif

#ifdef CONFIG_FAKEMEMORY_GRAPHICS
struct page*	alloc_gpu_memory(size_t size);
void			free_gpu_memory(const struct page* page);
#endif

int __init auth_init(void);
void __exit auth_exit(void);

/*
 * Ioctl definitions
 */

/* Use 'k' as magic number */
#define AUTH_IOC_MAGIC	'k'

/*
 * S means "Set"			 : through a ptr,
 * T means "Tell"			 : directly with the argument value
 * G means "Get"			 : reply by setting through a pointer
 * Q means "Query"			 : response is on the return value
 * X means "eXchange"			 : G and S atomically
 * H means "sHift"			 : T and Q atomically
 */
#define AUTH_IOCQ_MAP			 _IOW(AUTH_IOC_MAGIC, 1, AUTH_STR)
#define AUTH_IOCQ_ALLOC			 _IOW(AUTH_IOC_MAGIC, 2, mem_alloc_struct)
#define AUTH_IOCQ_FREE			 _IO(AUTH_IOC_MAGIC, 3)
#define AUTH_IOCQ_FREEALL		 _IO(AUTH_IOC_MAGIC, 4)
#define AUTH_IOCQ_LISTALL		 _IO(AUTH_IOC_MAGIC, 5)
#define AUTH_IOCS_FLUSHVIRT		 _IOW(AUTH_IOC_MAGIC, 6, mem_region_struct)
#define AUTH_IOCS_FLUSHPHYS		 _IOW(AUTH_IOC_MAGIC, 7, mem_region_struct)
#define AUTH_IOCQ_ALLOCAV		 _IOW(AUTH_IOC_MAGIC, 8, mem_alloc_struct)
#define AUTH_IOCQ_FREEAV		 _IO(AUTH_IOC_MAGIC, 9)

#define AUTH_IOCH_DUMPMEM		 _IO(AUTH_IOC_MAGIC, 10)

#ifdef	CONFIG_REALTEK_SCHED_LOG
#define AUTH_IOCSINITLOGBUF		 _IOW(AUTH_IOC_MAGIC, 11, sched_log_struct)
#define AUTH_IOCTFREELOGBUF		 _IO(AUTH_IOC_MAGIC, 12)
#define AUTH_IOCTLOGSTART		 _IO(AUTH_IOC_MAGIC, 13)
#define AUTH_IOCGLOGSTOP		 _IOR(AUTH_IOC_MAGIC, 14, sched_log_struct)
#define AUTH_IOCXLOGNAME		 _IOWR(AUTH_IOC_MAGIC, 15, COMM_STR)
#define AUTH_IOCSTHREADNAME		 _IOW(AUTH_IOC_MAGIC, 16, COMM_STR)
#define AUTH_IOCTLOGEVENT		 _IO(AUTH_IOC_MAGIC, 17)
#endif // CONFIG_REALTEK_SCHED_LOG

#define AUTH_IOCQ_DCUSIZE		 _IO(AUTH_IOC_MAGIC, 20)

#ifdef CONFIG_REALTEK_MANAGE_OVERLAPPED_REGION
#define AUTH_IOCQ_ALLOC_OVERLAPPED	 _IO(AUTH_IOC_MAGIC, 30)
#define AUTH_IOCQ_FREE_OVERLAPPED	 _IO(AUTH_IOC_MAGIC, 31)
#endif

#define AUTH_IOCQ_MEMC_START_ADDRESS	 _IO(AUTH_IOC_MAGIC, 40)
#define AUTH_IOCQ_MEMC_SIZE		 _IO(AUTH_IOC_MAGIC, 41)
#define AUTH_IOCQ_MDOMAIN_START_ADDRESS	 _IO(AUTH_IOC_MAGIC, 42)
#define AUTH_IOCQ_MDOMAIN_SIZE		 _IO(AUTH_IOC_MAGIC, 43)
#define AUTH_IOCQ_OD_START_ADDRESS	 _IO(AUTH_IOC_MAGIC, 44)
#define AUTH_IOCQ_OD_SIZE		 _IO(AUTH_IOC_MAGIC, 45)

#ifdef CONFIG_COMPAT
#define COMPAT_AUTH_IOCQ_ALLOC       _IOW(AUTH_IOC_MAGIC, 2, compat_mem_alloc_struct)
#define COMPAT_AUTH_IOCS_FLUSHVIRT   _IOW(AUTH_IOC_MAGIC, 6, compat_mem_region_struct)
#define COMPAT_AUTH_IOCS_FLUSHPHYS   _IOW(AUTH_IOC_MAGIC, 7, compat_mem_region_struct)
#define COMPAT_AUTH_IOCQ_ALLOCAV     _IOW(AUTH_IOC_MAGIC, 8, compat_mem_alloc_struct)
#endif

#endif // AUTH_H

