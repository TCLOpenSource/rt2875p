#ifndef _RMM_H_
#define _RMM_H_


#ifdef BUILD_QUICK_SHOW
#define DIV_ROUND_UP(n, d)  (((n) + (d) - 1) / (d))
#endif

#define _KB_ 1024
#define _MB_ 1024*1024
#define DO_ALIGNMENT(x, y) (DIV_ROUND_UP(x, y))*y
#define DO_ALIGNMENT_CHECK(x, y) (x & (y - 1))

#ifdef BUILD_QUICK_SHOW
#include <sysdefs.h>
#include <mach/rtk_platform.h>
#include <rtk_kdriver/quick_show/quick_show.h>
#include <no_os/export.h>

#define RBUS_BASE_PHYS          0x18000000 // use supersection mapping
#define RBUS_BASE_SIZE          0x00200000
#define PAGE_SIZE 4096
#define PAGE_SHIFT 12
#define MAX_ORDER 11
#ifndef true
#define true TRUE
#endif
#ifndef false
#define false FALSE
#endif
#ifndef min
#define min MIN
#endif
#ifndef max
#define max MAX
#endif
#define ULONG_MAX   (~0UL)

static unsigned int pageblock_order = MAX_ORDER - 1;

#define ALIGN_NBYTES(val, n_bytes)          (val + (n_bytes - (val % n_bytes)))
#define IS_ALIGNED_NBYTES(val, n_bytes)     ((val % n_bytes) == 0)
static inline unsigned long dvr_size_alignment_ex(unsigned long val, unsigned int n_bytes)
{
    if (IS_ALIGNED_NBYTES(val, n_bytes))
        return val;

    return ALIGN_NBYTES(val, n_bytes);
}

#ifndef MODULE_AUTHOR
#define MODULE_AUTHOR(x)
#endif

#endif //BUILD_QUICK_SHOW



#include <rtk_kdriver/rmm/rmm_carvedout.h>


typedef int carvedout_buf_t;

enum {
	/* refer rmm_carvedout.h */

	/* for reserved size in highmem */
	CARVEDOUT_HIGH_START = CARVEDOUT_SCALER,
	CARVEDOUT_HIGH_END = CARVEDOUT_TP
};

/*
 * legacy kdriver and it's user (e.g. quick-show) may still refer predefined static in builtin carvedout_buf[] for some reason
 *   it may need to go without devicetree but support multple layouts in one-bin instead of building single layout from deivcetree. 
 */

typedef enum {
	MM_LAYOUT_DEFAULT = 0,
	MM_LAYOUT_1G5,
	MM_LAYOUT_1GB,
	MM_LAYOUT_2K_MODEL,

	MM_LAYOUT_CARVEDOUT_NUM
} mm_layout_index_t;

extern unsigned long carvedout_buf[MM_LAYOUT_CARVEDOUT_NUM][CARVEDOUT_NUM][2];

typedef enum {
	CARVEDOUT_NO_FALLBACK = 0,
	CARVEDOUT_FALLBACK_TO_CMA,
	CARVEDOUT_FALLBACK_TO_VBM
} carvedout_fallback_type_t;

enum chunk_zone_type {
	CHUNK_ZONE_LARGE,
	CHUNK_ZONE_SMALL_1,
//	CHUNK_ZONE_SMALL_2,
	CHUNK_ZONE_COUNT, 

	/* for coding logic */
	CHUNK_ZONE_4K_LUMA = CHUNK_ZONE_LARGE,
	CHUNK_ZONE_COBUFFER = CHUNK_ZONE_LARGE, 
	CHUNK_ZONE_OTHER = CHUNK_ZONE_SMALL_1,
};

#define MAX_CHUNKS_PER_ZONE 3
#define PRE_ALLOCATE_SIZE_THRESHOLD (~0UL) //INVALID_VAL
#define PRE_ALLOCATE_SIZE_INDEX_BIT BIT(0) //bit0: chunk0, bit1: chunk1, ...

#define PRE_ALLOCATE_SIZE_INDEX_2D_BIT_ARRAY \
    {0, 0, 0}, \
    {0, 0, 0}, \
    {0, 0, 0},

#define VBM_HEAP_4K_CHUNK_SIZE_ARRAY \
    { 0*_MB_,  0*_MB_,  0*_MB_}, \
    { 0*_MB_,  0*_MB_,  0*_MB_},

#define VBM_HEAP_4K_DIAS_CHUNK_SIZE_ARRAY \
    { 0*_MB_,  0*_MB_,  0*_MB_}, \
    { 0*_MB_,  0*_MB_,  0*_MB_},

#define VBM_HEAP_4K_DIAS_MIRACAST_CHUNK_SIZE_ARRAY \
    { 0*_MB_,  0*_MB_,  0*_MB_}, \
    { 0*_MB_,  0*_MB_,  0*_MB_},

#define VBM_HEAP_2K_CHUNK_SIZE_ARRAY \
    { 0*_MB_,  0*_MB_,  0*_MB_}, \
    { 0*_MB_,  0*_MB_,  0*_MB_},

#define CARVEOUT_HEAP_8K_CHUNK_SIZE_ARRAY \
    { 0*_MB_,  0*_MB_,  0*_MB_}, \
    { 0*_MB_,  0*_MB_,  0*_MB_},

#define CARVEOUT_HEAP_8K_DISP_CHUNK_SIZE_ARRAY \
    { 0*_MB_,  0*_MB_,  0*_MB_}, \
    { 0*_MB_,  0*_MB_,  0*_MB_},

unsigned int carvedout_buf_get_layout_idx(void);
unsigned long carvedout_buf_query(carvedout_buf_t what, void **addr);
int carvedout_buf_query_is_in_range(unsigned long in_addr, void **start, void **end);
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
unsigned long carvedout_buf_query_secure(carvedout_buf_t what, void **addr);
#endif
unsigned long carvedout_buf_query_range(carvedout_buf_t idx_start, carvedout_buf_t idx_end, void **start, void **end);
unsigned int carvedout_fallback_query(carvedout_buf_t what);
int carvedout_buf_layout_build(void);

#ifdef CONFIG_CMA_MONITOR
#define MAX_CMA_LIMIT_DEMUX 17*_MB_
#define MAX_CMA_LIMIT_RTKVDEC 57*_MB_
#define MAX_CMA_LIMIT_SCALER 10*_MB_
#endif

#ifdef CONFIG_ARM64
static unsigned long __maybe_unused PAR(unsigned long addr, int user)
{
	unsigned long ret;
	__asm__ __volatile__ (
		"cmp  %w2, #0 \n\t"
		"beq 2f\n\t"
		"at s1e0r, %1 \n\t"
		"bne 1f \n\t"
		"2: \n\t"
		"at s1e1r, %1 \n\t"
		"1: \n\t"
		"isb \n\t"
		"dsb sy\n\t"
		"mrs %0, par_el1\n\t"
		"dsb sy\n\t"
		:"=r"(ret)
		:"r"(addr), "r"(user));

	return ret;
}
#endif

#endif // _RMM_H_