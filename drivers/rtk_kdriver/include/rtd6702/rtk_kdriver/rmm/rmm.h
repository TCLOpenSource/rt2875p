#ifndef _RMM_H_
#define _RMM_H_

#include <rtk_kdriver/rmm/rmm_carvedout.h>

#define _KB_ 1024
#define _MB_ 1024*1024
#define DO_ALIGNMENT(x, y) (DIV_ROUND_UP(x, y))*y
#define DO_ALIGNMENT_CHECK(x, y) (x & (y - 1))

#define ENABLE_DC_API_SUPPORT

typedef int carvedout_buf_t;

enum {
	/* refer rmm_carvedout.h */

	/* for reserved size in highmem */
	CARVEDOUT_HIGH_START = CARVEDOUT_SCALER,
	CARVEDOUT_HIGH_END = CARVEDOUT_TP
};

typedef enum {
	MM_LAYOUT_DEFAULT = 0,
#ifdef RTK_MEM_LAYOUT_DEVICETREE
    // only 1 carvedout_buf build from dts layout
#else
	MM_LAYOUT_1G5,
	MM_LAYOUT_1GB,
	MM_LAYOUT_DISP_8K,
	MM_LAYOUT_DISP_4K,
#endif
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
	CHUNK_ZONE_SMALL_2,
	CHUNK_ZONE_COUNT, 

	/* for coding logic */
	CHUNK_ZONE_4K_LUMA = CHUNK_ZONE_LARGE,
	CHUNK_ZONE_COBUFFER = CHUNK_ZONE_SMALL_1,
	CHUNK_ZONE_OTHER = CHUNK_ZONE_SMALL_1,
};

#define MAX_CHUNKS_PER_ZONE 4
#define PRE_ALLOCATE_SIZE_THRESHOLD 32*_MB_
#define PRE_ALLOCATE_SIZE_INDEX_BIT 0          //bit0: chunk0, bit1: chunk1, ...

#define PRE_ALLOCATE_SIZE_INDEX_2D_BIT_ARRAY \
    {0, 0, 0, 0}, \
    {0, 0, 0, 0}, \
    {0, 0, 0, 0},

#define VBM_HEAP_4K_CHUNK_SIZE_ARRAY \
    { 0*_MB_,   0*_MB_,   0*_MB_,  0*_MB_}, \
    { 0*_MB_,   0*_MB_,   0*_MB_,  0*_MB_}, \
    { 0*_MB_,   0*_MB_,   0*_MB_,  0*_MB_},

#define VBM_HEAP_4K_DIAS_CHUNK_SIZE_ARRAY \
    { 0*_MB_,   0*_MB_,   0*_MB_,  0*_MB_}, \
    { 0*_MB_,   0*_MB_,   0*_MB_,  0*_MB_}, \
    { 0*_MB_,   0*_MB_,   0*_MB_,  0*_MB_},

#define VBM_HEAP_4K_DIAS_MIRACAST_CHUNK_SIZE_ARRAY \
    { 0*_MB_,   0*_MB_,   0*_MB_,  0*_MB_}, \
    { 0*_MB_,   0*_MB_,   0*_MB_,  0*_MB_}, \
    { 0*_MB_,   0*_MB_,   0*_MB_,  0*_MB_},

#define VBM_HEAP_2K_CHUNK_SIZE_ARRAY \
    { 0*_MB_,   0*_MB_,   0*_MB_,  0*_MB_}, \
    { 0*_MB_,   0*_MB_,   0*_MB_,  0*_MB_}, \
    { 0*_MB_,   0*_MB_,   0*_MB_,  0*_MB_},

#define CARVEOUT_HEAP_8K_CHUNK_SIZE_ARRAY \
    {24*_MB_,  72*_MB_,  72*_MB_, 36*_MB_}, \
    {15*_MB_,  36*_MB_,  36*_MB_, 15*_MB_}, \
    { 0*_MB_,   0*_MB_,   0*_MB_,  0*_MB_},

#define CARVEOUT_HEAP_8K_DISP_CHUNK_SIZE_ARRAY \
    {51*_MB_, 102*_MB_, 102*_MB_, (77+51)*_MB_}, \
    {25*_MB_,  49*_MB_,  49*_MB_, (43+26)*_MB_}, \
    { 0*_MB_,   0*_MB_,   0*_MB_,  0*_MB_},

phys_addr_t ion_to_phys(int fd);

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
#define SYS_PAR_EL1_PA_MASK (0xFFFFFFFFFF<<12)

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

#ifdef ENABLE_DC_API_SUPPORT
#define RTK_KDRV_SUPPORT_DC_API // MACRO for mm alignment check

#define DCIB_Invalid 0xFFFFFFFF
#define ALIGN_SIZE_1_2DDR (1024)
#define ALIGN_SIZE_3DDR (3*1024)

typedef struct {
	unsigned int dram_num;
	unsigned int swap_en;
	unsigned int sw_mode;
	unsigned int sw_offset;
	unsigned int boundary;
	unsigned int has_IB2to1;
	bool if_init_ib_table;
} RTK_DC_INFO_t;

typedef enum {
	RTK_DC_RET_SUCCESS = 0,
	RTK_DC_RET_FAIL = 1
} RTK_DC_RET_t;

typedef enum {
	RTK_DC_HIGHER_BW = 0,
	RTK_DC_LOWER_BW = 1,
	RTK_DC_QUERY_FAIL = 2
} RTK_DC_BW_TYPE_t;

extern RTK_DC_RET_t rtk_dc_get_dram_info(RTK_DC_INFO_t *dram_info);
extern RTK_DC_BW_TYPE_t rtk_dc_query_dram_region_by_addr(unsigned long phy_addr);
extern unsigned int rtk_get_dc_secure_aligned_addr(const unsigned int dc_addr, int is_aligned_up);
extern unsigned int rtk_get_dc_secure_aligned_size(const unsigned int dc_addr);

#else

static inline unsigned int rtk_get_dc_secure_aligned_addr(const unsigned int dc_addr, int is_aligned_up) { return dc_addr;};
static inline unsigned int rtk_get_dc_secure_aligned_size(const unsigned int dc_addr) { return 0; };

#endif // DC_API

typedef struct
{       
	unsigned long start;
	unsigned long end;
	unsigned int mc_count;
}ib_region_aligned_info_t;
 
 #define MC_ALIGNED_SIZE  1024
 #define SW_IB_REGIONS_NUM 8
int rtk_rmm_update_dcib_config(ib_region_aligned_info_t *dc_ib_region, int len);

#endif // _RMM_H_
