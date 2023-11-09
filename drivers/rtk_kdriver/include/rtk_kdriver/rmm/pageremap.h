#ifndef _LINUX_PAGEREMAP_H
#define _LINUX_PAGEREMAP_H

#define INVALID_VAL    ULONG_MAX

#include <linux/dma-buf.h>
#include <rtk_kdriver/rtk_dc_mt_config.h> // for MODULE_ID, MODULE_XXX
#include <rtk_kdriver/rmm/rtkrecord.h> // for kernel native mm/init.c fs/drop_caches use

#ifndef CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE
#define CONFIG_BOOTUP_ANIMATION
#endif

#define GFP_DCU1	0x00000001
#define GFP_DCU2	0x00000002
#define GFP_DCU1_FIRST	0x00000004
#define GFP_DCU2_FIRST	0x00000008
#define GFP_DCU2_GPU_CACHE_POOL	0x00000010
#define GFP_CMA_64BIT	0x00000020
#define GFP_DCU1_LIMIT	0x00000040
#ifdef CONFIG_OPTEE_SUPPORT_MC_ALLOCATOR
#define GFP_MC_CMA	0x00000080
#endif
#define GFP_DCU2_VDEC_CMPHDR	0x00000100
#define GFP_GPU_4K	0x00000200
#ifdef CONFIG_HIGHMEM_BW_CMA_REGION
#define GFP_HIGHMEM_BW	0x00000400
#define GFP_HIGHMEM_BW_FIRST	0x00000800
#endif
#define GFP_VBM_CMA    0x00001000
#define GFP_VBM_CARVEOUT	0x00002000
#ifdef CONFIG_RTK_KDRV_PCIE
#define GFP_CMA_PCIE    0x00004000
#endif

#define MEM_CHUNK_SIZE	8*1024*1024

#define MAX_CACHE_COUNT 4
#define DEF_CACHE_SIZE	8*1024*1024
#define DEF_CACHE_SIZE_LOW	4*1024*1024
#define DEF_CACHE_SIZE_GPU_CACHE_POOL	8*1024*1024

#ifdef CONFIG_CMA_RANK_RBTREE
#define PAGE_LOOKUP_ONLY	0x00000001
#endif

#define GUARDBAND_SIZE 4*1024*1024 // for page order alignment case

#define ALIGN_NBYTES(val, n_bytes)          (val + (n_bytes - (val % n_bytes)))
#define IS_ALIGNED_NBYTES(val, n_bytes)     ((val % n_bytes) == 0)

typedef struct {
	struct list_head	list;
	struct page		*page;
} record_struct;

#if defined(CONFIG_VBM_CMA) || defined(CONFIG_VBM_CARVEOUT)
#define __MB__  (1024*1024)

enum dma_heap_user_type {
	DMA_HEAP_USER_VIDEO_4K_LUMA,		/* request user is video 4K luma */
	DMA_HEAP_USER_VIDEO_4K_CHROMA,		/* request user is video 4K chroma */
	DMA_HEAP_USER_VIDEO_2K,				/* request user is video 2K */
	DMA_HEAP_USER_COBUFFER,				/* request user is VBM co-buffer */
	DMA_HEAP_USER_VIDEO_SUB_2K,	/* request user is video 2K decimate */

	MAX_NR_DMA_HEAP_USERS
};
#endif

enum vbm_buffer_type {
	VBM_BUF_DECOMP, 
	VBM_BUF_DECIMATE,
};

struct rtkcma;

#ifdef CONFIG_CMA_RTK_ALLOCATOR

void *dma_get_allocator(struct device *dev);
bool in_cma_range(struct device *dev, unsigned long pfn);
unsigned long cma_get_avail_size(struct rtkcma *rtkcma);

void add_memory_size(int flags, unsigned long size);
unsigned long get_memory_size(int flags);

bool check_cma_memory(unsigned long pfn);
unsigned long get_cma_avail_size(void);


#define DVR_OWNER_NOID   0xFFFE00FE // special cma user tag for legacy no id specified
#define DVR_OWNER_MULTI  0xFFFC00FE // special cma user tag for more than one write module
#define DVR_OWNER_GAL    0xFF0100FC //FIXME non-dcmt tag for most likely gpu user
#define DVR_OWNER_AUTH   0xFF0200FC //FIXME non-dcmt tag for auth ioctl
#define DVR_OWNER_RMM    0xFF0300FC //FIXME non-dcmt tag for rmm ioctl
#define DVR_OWNER_DECOMP 0xFF0400FC //FIXME non-dcmt tag for vdec/vo
#define DVR_OWNER_GPU    0xFF0500FC //FIXME non-dcmt tag for gpu user
#define DVR_OWNER_ION    0xFF0600FC //FIXME non-dcmt tag for gpu user
#define DVR_OWNER_HEAP   0xFF0700FC //FIXME non-dcmt tag for dvr_heap, vbm_heap, ion_dvr_heap via native api
#define DVR_OWNER_RPC    0xFF0800FC //FIXME non-dcmt tag for auth rpc_dvr
#define DVR_OWNER_VBM    0xFF0900FC //FIXME non-dcmt tag for vbm_heap

// The following functions are used by user to allocate continuous memory...
unsigned long pli_malloc(size_t size, int flags);
void pli_free(const unsigned long arg);
unsigned long pli_malloc_mesg(size_t size, int flags, char *mesg);
void pli_free_mesg(const unsigned long arg);

// This function is used to convert the virtual address return by dvr_XXX() to physical address...
unsigned long dvr_vmalloc_to_phys(const void *vmalloc_addr);
unsigned long dvr_to_phys(const void *dvr_addr);

void *dvr_remap_cached_memory(unsigned long phys_addr, size_t size, const void *caller);
void *dvr_remap_uncached_memory(unsigned long phys_addr, size_t size, const void *caller);
void dvr_unmap_memory(void *cpu_addr, size_t size);

// The following functions are used by driver to allocate continuous memory...
void *dvr_malloc(size_t size);
void *dvr_malloc_specific(size_t size, int flags);
void *dvr_malloc_specific_addr(size_t size, int flags, unsigned long addr);
void *dvr_malloc_nosave(size_t size);
void *dvr_malloc_nosave_specific(size_t size, int flags);
void *dvr_malloc_uncached(size_t size, void **uaddr);
void *dvr_malloc_uncached_specific(size_t size, int flags, void **uaddr);
void *dvr_malloc_prot(size_t size, pgprot_t prot);
void *dvr_malloc_prot_specific(size_t size, int flags, pgprot_t prot);
void dvr_free(const void *arg);

struct page *dvr_malloc_page(size_t size, int flags);
void dvr_free_page(const struct page *page);
#ifdef CONFIG_FAKEMEMORY_GRAPHICS
struct page *dvr_malloc_gpu_page(size_t size);
void dvr_free_gpu_page(const struct page *page);
#endif
struct page *dvr_malloc_page_mesg(size_t size, int flags, char *mesg);
void dvr_free_page_mesg(const struct page *page);

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
unsigned long rpc_dvr_malloc(unsigned long size, unsigned long none);
unsigned long rpc_dvr_free(unsigned long addr, unsigned long none);
#endif

#ifdef CONFIG_CMA_RANK_RBTREE
int dvr_free_page_count(struct page *page, size_t size, int flag);
#endif

// The following functions are used by rtkrecord
unsigned long get_page_cache_addr(void);
void show_cma_avaliable(void);
int remap_list_mem(rtk_record *records);
int remap_free_mem(rtk_record *records);

// The follwing functions are used for cma_guardband and cma_deferfree debug tool
bool need_debug_cma_deferfree(void);
bool need_debug_cma_guardband(void);
int dvr_guardband_size(size_t size_page);
int dvr_guardband_padding(struct page* page, int count_order, int count_page);
int dvr_guardband_check(unsigned long phys_addr, int value);
int dvr_deferfree_check(unsigned long phys_addr);
void dvr_defer_free(rtk_record *ptr);

/*
 * 0: do alignment fail
 * non-zero: do alignment success
 */
unsigned long dvr_memory_alignment(unsigned long addr, size_t size);
unsigned long dvr_size_alignment(unsigned long val);
unsigned long dvr_size_alignment_ex(unsigned long val, unsigned int n_bytes);
/* Check size if 4K page alignment,
 * this is for 4K size align dynamic allocation usage.
 */
unsigned long dvr_size_alignment_cma(unsigned long val);

#else

static inline void *dma_get_allocator(struct device *dev) { return NULL; }
static inline bool in_cma_range(struct device *dev, unsigned long pfn) { return false; }
static inline unsigned long cma_get_avail_size(struct rtkcma *rtkcma) { return 0; }

static inline void add_memory_size(int flags, unsigned long size) {}
static inline unsigned long get_memory_size(int flags) { return 0; }

static inline bool check_cma_memory(unsigned long pfn) { return false; }
static inline unsigned long get_cma_avail_size(void) { return 0; }

// The following functions are used by user to allocate continuous memory...
static inline unsigned long pli_malloc(size_t size, int flags) { return INVALID_VAL; }
static inline void pli_free(const unsigned long arg) {}
static inline unsigned long pli_malloc_mesg(size_t size, int flags, char *mesg) { return INVALID_VAL; }
static inline void pli_free_mesg(const unsigned long arg) {}

// This function is used to convert the virtual address return by dvr_XXX() to physical address...
static inline unsigned long dvr_vmalloc_to_phys(const void *vmalloc_addr) {return INVALID_VAL; }
static inline unsigned long dvr_to_phys(const void *dvr_addr) { return INVALID_VAL; }

static inline void *dvr_remap_cached_memory(unsigned long phys_addr, size_t size, const void *caller) { return NULL; }
static inline void *dvr_remap_uncached_memory(unsigned long phys_addr, size_t size, const void *caller) { return NULL; }
static inline void dvr_unmap_memory(void *cpu_addr, size_t size) {}

// The following functions are used by driver to allocate continuous memory...
static inline void *dvr_malloc(size_t size) { return NULL; }
static inline void *dvr_malloc_specific(size_t size, int flags) { return NULL; }
static inline void *dvr_malloc_specific_addr(size_t size, int flags, unsigned long addr) { return NULL; }
static inline void *dvr_malloc_nosave(size_t size) { return NULL; }
static inline void *dvr_malloc_nosave_specific(size_t size, int flags) { return NULL; }
static inline void *dvr_malloc_uncached(size_t size, void **uaddr) { return NULL; }
static inline void *dvr_malloc_uncached_specific(size_t size, int flags, void **uaddr) { return NULL; }
static inline void *dvr_malloc_prot(size_t size, pgprot_t prot) { return NULL; }
static inline void *dvr_malloc_prot_specific(size_t size, int flags, pgprot_t prot) { return NULL; }
static inline void dvr_free(const void *arg) {}

static inline struct page *dvr_malloc_page(size_t size, int flags) { return NULL; }
static inline void dvr_free_page(const struct page *page) {}
#ifdef CONFIG_FAKEMEMORY_GRAPHICS
static inline struct page *dvr_malloc_gpu_page(size_t size) { return NULL; }
static inline void dvr_free_gpu_page(const struct page *page) {}
#endif
static inline struct page *dvr_malloc_page_mesg(size_t size, int flags, char *mesg) { return NULL; }
static inline void dvr_free_page_mesg(const struct page *page) {}

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
unsigned long rpc_dvr_malloc(unsigned long size, unsigned long none) { return NULL; }
unsigned long rpc_dvr_free(unsigned long addr, unsigned long none) { return INVALID_VAL; }
#endif

/* 
 * 0: do alignment fail
 * non-zero: do alignment success 
 */
static inline unsigned long dvr_memory_alignment(unsigned long addr, size_t size) { return addr; }
static inline unsigned long dvr_size_alignment(unsigned long val) { return val; }
static inline unsigned long dvr_size_alignment_ex(unsigned long val, unsigned int n_bytes) { return val; }
/* Check size if 4K page alignment,
 * this is for 4K size align dynamic allocation usage.
 */
static inline unsigned long dvr_size_alignment_cma(unsigned long val) { return val; }

#endif

// The following functions are used to reserve the movable zone for special purpose...
int reserve_highmem(void);
int release_highmem(void);
int reserve_movable(void);
int release_movable(void);

#ifdef CONFIG_CMA_FIRST_POLICY
int reserve_cma(int in_suspend);
int release_cma(int in_suspend);
#endif

// This function is used by kernel to remap one page only...
int remap_one_page(struct page *page);

int calc_unmovable(void);

#if IS_ENABLED(CONFIG_DMABUF_HEAPS_DVR) && IS_ENABLED(CONFIG_DMABUF_CARVEOUT_HEAP) && IS_ENABLED(CONFIG_DMABUF_HEAPS_VBM)
unsigned long dvr_dmabuf_to_phys(struct dma_buf *dmabuf);
struct dma_buf *dvr_dmabuf_malloc(size_t size, int flags, unsigned long **phys);
void dvr_dmabuf_free(struct dma_buf *dmabuf);
int dvr_dmabuf_to_list(struct dma_buf *dmabuf, unsigned long buf_array[], int array_num);
struct dma_buf *dvr_dmabuf_chunk_malloc(size_t size, int flags, int frame_type, bool need_protect, unsigned long **phys);
struct dma_buf *dvr_dmabuf_frames_malloc(size_t frame_size, unsigned int frame_num, int flags, int frame_type, bool need_protect, unsigned long **first_frame_phys);
#else
static inline unsigned long dvr_dmabuf_to_phys(struct dma_buf *dmabuf) {return INVALID_VAL;}
static inline struct dma_buf *dvr_dmabuf_malloc(size_t size, int flags, unsigned long **phys) {return NULL;}
static inline void dvr_dmabuf_free(struct dma_buf *dmabuf) {}
static inline int dvr_dmabuf_to_list(struct dma_buf *dmabuf, unsigned long buf_array[], int array_num) {return -ENOMEM;}
static inline struct dma_buf *dvr_dmabuf_chunk_malloc(size_t size, int flags, int frame_type, bool need_protect, unsigned long **phys) {return NULL;}
static inline struct dma_buf *dvr_dmabuf_frames_malloc(size_t frame_size, unsigned int frame_num, int flags, int frame_type, bool need_protect, unsigned long **first_frame_phys) {return NULL;}
#endif


int rtk_set_attribute_init(void);
int rtk_set_memory_valid(unsigned long addr, unsigned long size, int enable);
int __init remap_init(void);
void __exit remap_exit(void);

#endif /* _LINUX_PAGEREMAP_H */
