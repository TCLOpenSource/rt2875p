
#ifndef BUILD_QUICK_SHOW
#include <linux/version.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/dma-buf.h>
#include <linux/ion.h>

#include <rtk_kdriver/rmm/rmm.h>
#include <rtk_kdriver/io.h>
#include <rtk_kdriver/quick_show/quick_show.h>
#include <linux/auth.h>
#include <linux/pageremap.h>

#include <mach/iomap.h>
#include <mach/common.h>
#include <mach/rtk_platform.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#else
#include <mach/rtk_platform.h>
#include <no_os/printk.h>
#include <malloc.h>
#endif

#include <rtk_kdriver/rtk_qos_export.h>
#include <rtd_log/rtd_module_log.h>
#include <rbus/dc_mc_reg.h>
#include <rbus/dc_sys_reg.h>

unsigned long DRAM_size = 0;
EXPORT_SYMBOL(DRAM_size);

#ifndef BUILD_QUICK_SHOW
#if defined(CONFIG_REALTEK_MEMORY_MANAGEMENT)
#define LAST_IMAGE_SIZE 4*1024*1024
extern phys_addr_t __initdata reserved_dvr_start;
extern phys_addr_t __initdata reserved_dvr_size;
extern phys_addr_t __initdata reserved_last_image_start;
extern phys_addr_t __initdata reserved_last_image_size;
extern void *pAnimation;
extern void *pAnimation_1;
extern void *pLastImage;
#endif

phys_addr_t ion_to_phys(int fd)
{
	struct dma_buf *dmabuf = NULL;
	struct ion_buffer *buffer = NULL;
	struct page *page = NULL;
	phys_addr_t ret = INVALID_VAL;

	dmabuf = dma_buf_get(fd);
	if (IS_ERR(dmabuf)) {
		rtd_pr_rmm_err("%s() fail, fd(%d)\n", __func__, fd);
		goto err;
	}

	buffer = dmabuf->priv;
	if (buffer == NULL) {
		rtd_pr_rmm_err("%s() no buffer, dmabuf(%lx) fd(%d)\n", __func__, (unsigned long)dmabuf, fd);
		goto err;
	}

	if (buffer->heap->type != ION_HEAP_TYPE_CUSTOM) {
		rtd_pr_rmm_err("%s() not sutable heap type(%d) fd(%d)\n", __func__, buffer->heap->type, fd);
		goto err;
	}

	if (buffer->sg_table == NULL) {
		rtd_pr_rmm_err("%s() no sg table, fd(%d)\n", __func__, fd);
		goto err;
	}

	page = sg_page(buffer->sg_table->sgl);
	if (page == NULL) {
		rtd_pr_rmm_err("%s() no page in sglist, fd(%d)\n", __func__, fd);
		goto err;
	}
	ret = PFN_PHYS(page_to_pfn(page));

	rtd_pr_rmm_debug("%s() fd(%d), addr=%x\n", __func__, fd, (unsigned int)ret);

err:
	if (!IS_ERR(dmabuf))
		dma_buf_put(dmabuf);
	
	return ret;
}
EXPORT_SYMBOL(ion_to_phys);
#endif

/**
   list by address order,
   {addr, size}
 */
#define _TBD_ 0

unsigned long carvedout_buf[MM_LAYOUT_CARVEDOUT_NUM][CARVEDOUT_NUM][2] = {
  // MM_LAYOUT_DEFAULT    for TV 4K 2GB
  {
	{0x00000000,  1*_MB_},							// CARVEDOUT_BOOTCODE
	{DEMOD_CARVED_OUT,  8*_MB_},					// CARVEDOUT_DEMOD
	{0x10000000,  0*_MB_},							// CARVEDOUT_AV_DMEM
//	{0x12280000,  0*_MB_/*17920*_KB_*/},			// CARVEDOUT_VDEC_COMEM
	{AUDIO_BUFFER_START, AUDIO_BUFFER_SIZE},		// CARVEDOUT_AUDIO
	{0x15f00000,  0*_MB_},							// CARVEDOUT_K_BOOT
	{0x16000000, 18*_MB_},							// CARVEDOUT_K_OS (optee)
	{0x17c00000, 4*_MB_},							// CARVEDOUT_MAP_GIC
	{RBUS_BASE_PHYS, RBUS_BASE_SIZE},				// CARVEDOUT_MAP_RBUS
	{0x1a900000 - RTK_GPU_FW_SIZE, RTK_GPU_FW_SIZE}, // CARVEDOUT_GPU_FW
	{0x1a900000, 8*_MB_},							// CARVEDOUT_V_OS
	{0x1b180000,  0x00080000},						// CARVEDOUT_MAP_RPC
	{0x1b200000, 11*_MB_},							// CARVEDOUT_A_OS
	{CONST_LOGBUF_MEM_ADDR_START, CONST_LOGBUF_MEM_SIZE}, // CARVEDOUT_LOGBUF
	{0x1fc00000, 0x00008000},						// CARVEDOUT_ROMCODE
	{0x1fd00000, 0x010000},							// CARVEDOUT_RAMOOPS
	{0x1ffff000, 0x00001000},						// CARVEDOUT_IR_TABLE
	{_TBD_, _TBD_},									// CARVEDOUT_DDR_BOUNDARY
	{_TBD_, _TBD_},									// CARVEDOUT_DDR_BOUNDARY_2
	{VIDEO_RINGBUFFER_START,  VIDEO_RINGBUFFER_SIZE}, // CARVEDOUT_VDEC_RINGBUF
//	{AUDIO_RINGBUFFER_START,  AUDIO_RINGBUFFER_SIZE}, // CARVEDOUT_AUDIO_RINGBUF
	{_TBD_, _TBD_},				// CARVEDOUT_GAL
	{_TBD_, _TBD_},									// CARVEDOUT_SNAPSHOT
	{SCLAER_MODULE_START, SCALER_MODULE_SIZE},		// CARVEDOUT_SCALER
	{SCALER_MEMC_START, SCALER_MEMC_SIZE},			// CARVEDOUT_SCALER_MEMC
	{SCALER_MDOMAIN_START, SCALER_MDOMAIN_SIZE},	// CARVEDOUT_SCALER_MDOMAIN
	{SCALER_DI_NR_START, SCALER_DI_NR_SIZE},		// CARVEDOUT_SCALER_DI_NR
	{SCALER_NN_START, SCALER_NN_SIZE},				// CARVEDOUT_SCALER_NN
	{SCALER_VIP_START, SCALER_VIP_SIZE},			// CARVEDOUT_SCALER_VIP
	{SCALER_OD_START, SCALER_OD_SIZE},				// CARVEDOUT_SCALER_OD
	{VDEC_BUFFER_START, VDEC_BUFFER_SIZE},			// CARVEDOUT_VDEC_VBM
	{DEMUX_TP_BUFFER_START, DEMUX_TP_BUFFER_SIZE},	// CARVEDOUT_TP

    // for desired cma size calculation
	{0x09000000, 112*_MB_},							// CARVEDOUT_CMA_LOW
	{0, _TBD_},										// CARVEDOUT_CMA_HIGH
	{0, _TBD_},										// CARVEDOUT_CMA_GPU_4K
	{0, _TBD_},										// CARVEDOUT_CMA_3
	{0, _TBD_},										// CARVEDOUT_CMA_LOW_LIMIT
	{0, _TBD_},										// CARVEDOUT_CMA_BW
	{VDEC_BUFFER_START, VDEC_BUFFER_SIZE},			// CARVEDOUT_CMA_VBM

	{0, _TBD_},										// CARVEDOUT_GPU_RESERVED
  },

  // MM_LAYOUT_DIAS_2G    for DIAS advanced 2GB
  {
	{0x00000000,  1*_MB_},							// CARVEDOUT_BOOTCODE
	{DEMOD_CARVED_OUT,  0*_MB_},					// CARVEDOUT_DEMOD
	{0x10000000,  0*_MB_},							// CARVEDOUT_AV_DMEM
//	{0x12280000,  0*_MB_/*17920*_KB_*/},			// CARVEDOUT_VDEC_COMEM
	{AUDIO_BUFFER_START, AUDIO_BUFFER_SIZE},		// CARVEDOUT_AUDIO
	{0x15f00000,  0*_MB_},							// CARVEDOUT_K_BOOT
	{0x16000000, 18*_MB_},							// CARVEDOUT_K_OS (optee)
	{0x17c00000, 4*_MB_},							// CARVEDOUT_MAP_GIC
	{RBUS_BASE_PHYS, RBUS_BASE_SIZE},				// CARVEDOUT_MAP_RBUS
	{0x1a900000 - RTK_GPU_FW_SIZE, RTK_GPU_FW_SIZE}, // CARVEDOUT_GPU_FW
	{0x1a900000, 8*_MB_},							// CARVEDOUT_V_OS
	{0x1b180000,  0x00080000},						// CARVEDOUT_MAP_RPC
	{0x1b200000, 11*_MB_},							// CARVEDOUT_A_OS
	{DDR_DIAS_2G_CONST_LOGBUF_MEM_ADDR_START, DDR_DIAS_2G_CONST_LOGBUF_MEM_SIZE}, // CARVEDOUT_LOGBUF
	{0x1fc00000, 0x00008000},						// CARVEDOUT_ROMCODE
	{0x1fd00000, 0x010000},							// CARVEDOUT_RAMOOPS
	{0x1ffff000, 0x00001000},						// CARVEDOUT_IR_TABLE
	{_TBD_, _TBD_},									// CARVEDOUT_DDR_BOUNDARY
	{_TBD_, _TBD_},									// CARVEDOUT_DDR_BOUNDARY_2
	{DDR_DIAS_2G_VIDEO_RINGBUFFER_START,  DDR_DIAS_2G_VIDEO_RINGBUFFER_SIZE}, // CARVEDOUT_VDEC_RINGBUF
//	{DDR_DIAS_2G_AUDIO_RINGBUFFER_START,  DDR_DIAS_2G_AUDIO_RINGBUFFER_SIZE}, // CARVEDOUT_AUDIO_RINGBUF
	{_TBD_, _TBD_},			// CARVEDOUT_GAL
	{_TBD_, _TBD_},												// CARVEDOUT_SNAPSHOT
	{DDR_DIAS_2G_SCLAER_MODULE_START, DDR_DIAS_2G_SCALER_MODULE_SIZE},	// CARVEDOUT_SCALER
	{DDR_DIAS_2G_SCALER_MEMC_START, DDR_DIAS_2G_SCALER_MEMC_SIZE},		// CARVEDOUT_SCALER_MEMC
	{DDR_DIAS_2G_SCALER_MDOMAIN_START, DDR_DIAS_2G_SCALER_MDOMAIN_SIZE}, // CARVEDOUT_SCALER_MDOMAIN
	{DDR_DIAS_2G_SCALER_DI_NR_START, DDR_DIAS_2G_SCALER_DI_NR_SIZE},	// CARVEDOUT_SCALER_DI_NR
	{DDR_DIAS_2G_SCALER_NN_START, DDR_DIAS_2G_SCALER_NN_SIZE},			// CARVEDOUT_SCALER_NN
	{DDR_DIAS_2G_SCALER_VIP_START, DDR_DIAS_2G_SCALER_VIP_SIZE},		// CARVEDOUT_SCALER_VIP
	{DDR_DIAS_2G_SCALER_OD_START, DDR_DIAS_2G_SCALER_OD_SIZE},			// CARVEDOUT_SCALER_OD
	{DDR_DIAS_2G_VDEC_BUFFER_START, DDR_DIAS_2G_VDEC_BUFFER_SIZE},		// CARVEDOUT_VDEC_VBM
	{DDR_DIAS_2G_DEMUX_TP_BUFFER_START, DDR_DIAS_2G_DEMUX_TP_BUFFER_SIZE}, // CARVEDOUT_TP

    // for desired cma size calculation
	{0x09000000, 112*_MB_},							// CARVEDOUT_CMA_LOW
	{0, _TBD_}, 									// CARVEDOUT_CMA_HIGH
	{0, _TBD_},										// CARVEDOUT_CMA_GPU_4K
	{0, _TBD_},										// CARVEDOUT_CMA_3
	{0, _TBD_},										// CARVEDOUT_CMA_LOW_LIMIT
	{0, _TBD_},										// CARVEDOUT_CMA_BW
	{DDR_DIAS_2G_VDEC_BUFFER_START, DDR_DIAS_2G_VDEC_BUFFER_SIZE},		// CARVEDOUT_CMA_VBM

	{0, _TBD_},										// CARVEDOUT_GPU_RESERVED
  },

};
EXPORT_SYMBOL(carvedout_buf);

#ifdef ENABLE_DC_API_SUPPORT

static __maybe_unused unsigned int __rtk_dc_get_small_dram_size(void)
{
	dc_sys_dc_sys_misc_RBUS dc_sys_misc;

	dc_sys_misc.regValue = rtd_inl(DC_SYS_DC_SYS_MISC_reg);

	if (dc_sys_misc.small_dram_4g)
		return 0x20000000; // 512MB
	if (dc_sys_misc.small_dram_2g)
		return 0x10000000; // 256MB
	if (dc_sys_misc.small_dram_1g)
		return 0x8000000; // 128MB
	if (dc_sys_misc.small_dram_512m)
		return 0x4000000; // 64MB

	return 0;
}

#if 0
static unsigned int __rtk_dc_get_small_dram_num(void)
{
	dc_sys_dc_sys_misc_RBUS dc_sys_misc;

	dc_sys_misc.regValue = rtd_inl(DC_SYS_DC_SYS_MISC_reg);

	switch(dc_sys_misc.small_dram_num){
		case 0:
			return 2; // 2 small dram
		case 1:
			return 1; // 1 small dram
		default:
			return 0; // no small dram
	}

	return 0;
}

static unsigned int __rtk_dc_get_dram_num(void)
{
	dc_sys_dc_sys_misc_RBUS dc_sys_misc;

	dc_sys_misc.regValue = rtd_inl(DC_SYS_DC_SYS_MISC_reg);

	switch(dc_sys_misc.mem_num){
		case 0:
			return 1; // 1-ddr
		case 1:
			return 2; // 2-ddr
		case 2:
			return 3; // 3-ddr
		default:
			return 0; // none
	}

	return 0;
}
#endif

static __maybe_unused unsigned int __rtk_dc_is_swapped(void)
{
	dc_sys_dc_sys_misc_RBUS dc_sys_misc;

	dc_sys_misc.regValue = rtd_inl(DC_SYS_DC_SYS_MISC_reg);

	if (dc_sys_misc.en_full_ddr_last == 1)
		return 1; // swapped
	else
		return 0; // non-swapped
}

static __maybe_unused unsigned int __rtk_dc_get_boundary(void)
{
#if 1
	return (unsigned int)INVALID_VAL; // error, no dram_num
#else
	dc_sys_dc_sys_misc_RBUS dc_sys_misc;
	int dram_num = 0;
	int small_dram_num = 0;
	unsigned int small_dram_size = 0;
	unsigned int boundary_addr = 0;

	dc_sys_misc.regValue = rtd_inl(DC_SYS_DC_SYS_MISC_reg);

	if (dc_sys_misc.en_ddr_diff_size == 0)
		return 0;

	dram_num = __rtk_dc_get_dram_num();
	small_dram_num = __rtk_dc_get_small_dram_num();
	small_dram_size = __rtk_dc_get_small_dram_size();

	if (dc_sys_misc.en_full_ddr_last == 0) // non-swapped
	{
		boundary_addr = small_dram_size * dram_num;
	}
	else
	{
		if (dram_num == 3)
			boundary_addr = small_dram_size * (dram_num - small_dram_num);
		else if (dram_num == 2)
			boundary_addr = small_dram_size;
	}

	return boundary_addr;
#endif
}

RTK_DC_RET_t rtk_dc_get_dram_info(RTK_DC_INFO_t *dram_info)
{
#if 1
	return RTK_DC_RET_FAIL; // error, no dram_num
#else
	if (dram_info == NULL) {
		rtd_pr_rmm_err("[ERR]%s(%d): dram_info is NULL\n", __FUNCTION__, __LINE__);
		return RTK_DC_RET_FAIL; // error
	}

	dram_info->dram_num = __rtk_dc_get_dram_num();
	dram_info->swap_en = __rtk_dc_is_swapped();
	dram_info->boundary = __rtk_dc_get_boundary();

	return RTK_DC_RET_SUCCESS;
#endif
}
EXPORT_SYMBOL(rtk_dc_get_dram_info);

RTK_DC_BW_TYPE_t rtk_dc_query_dram_region_by_addr(unsigned long phy_addr)
{
	RTK_DC_INFO_t dram_info;
	RTK_DC_RET_t ret;

	ret = rtk_dc_get_dram_info(&dram_info);

	if (ret != RTK_DC_RET_SUCCESS) {
		rtd_pr_rmm_err("[ERR]%s(%d): fail to get dram_info\n", __FUNCTION__, __LINE__);
		return RTK_DC_QUERY_FAIL; // error
	}

	if (dram_info.boundary == 0) // all dram size is the same
		return RTK_DC_HIGHER_BW;

	if ((dram_info.swap_en == 0) && (phy_addr < dram_info.boundary))
		return RTK_DC_HIGHER_BW;
	else if ((dram_info.swap_en == 1) && (phy_addr >= dram_info.boundary))
		return RTK_DC_HIGHER_BW;
	else
		return RTK_DC_LOWER_BW;
}
EXPORT_SYMBOL(rtk_dc_query_dram_region_by_addr);



static int dcib_config_init_done = 0;  // -1:failed 0:not init 1:init OK
static ib_region_aligned_info_t dc_ib_region_s[SW_IB_REGIONS_NUM];

int rtk_rmm_update_dcib_config(ib_region_aligned_info_t *dc_ib_region, int len)
{
	int i;
	ib_region_aligned_info_t * s;

	if(dc_ib_region == NULL)
	{
		rtd_pr_rmm_err("ERROR, dcib config NULL error\n");
		return 0;
	}
	if(len <= SW_IB_REGIONS_NUM){
		memcpy(dc_ib_region_s, dc_ib_region, sizeof(dc_ib_region_s));
		dcib_config_init_done = 1;
		for(i=0; i<SW_IB_REGIONS_NUM; i++)
		{
			s = &dc_ib_region_s[i];
			if(s->end > s->start)
			{
				rtd_pr_rmm_info("MC secure aligned [%lx,%lx], mc=%d\n", s->start,s->end, s->mc_count);
			}
		}
	}
	else {
		rtd_pr_rmm_err("ERROR, dcib config len error\n");
	}
	return 0;
}

EXPORT_SYMBOL_GPL(rtk_rmm_update_dcib_config);


unsigned int rtk_get_dc_secure_aligned_size(const unsigned int dc_addr)
{

	unsigned int align_size = MC_ALIGNED_SIZE;
	int i;
	ib_region_aligned_info_t * s;

	if(dcib_config_init_done != 1) //failed & succ
	{
		return  MC_ALIGNED_SIZE;
	}
	for(i=0; i<SW_IB_REGIONS_NUM; i++) {
		s = &dc_ib_region_s[i];
		if( dc_addr >= s->start  && dc_addr < s->end){
			align_size = s->mc_count*MC_ALIGNED_SIZE;
			rtd_pr_rmm_debug("[Secure Aligned] %x ALIGNED=%x\n", dc_addr, align_size);
			break;
		}
	}
	return align_size;
}


unsigned int rtk_get_dc_secure_aligned_addr(const unsigned int dc_addr, int is_aligned_up)
{

        unsigned int align_size;
        unsigned int align_addr;

        align_size = rtk_get_dc_secure_aligned_size(dc_addr);

        if(is_aligned_up)
                align_addr = ALIGNED_UP(dc_addr, align_size);
        else 
                align_addr = ALIGNED_DOWN(dc_addr, align_size);

        return align_addr;
}


#if 0 // test only
static int __init rtk_dram_info_dump(void)
{
	RTK_DC_INFO_t dram_info;
	RTK_DC_RET_t ret;

	rtd_pr_rmm_err("[RTK-DC] %s:\n", __FUNCTION__);

	ret = rtk_dc_get_dram_info(&dram_info);

	if (ret == RTK_DC_RET_SUCCESS)
	{
		rtd_pr_rmm_err("dram_info->dram_num is %x\n", dram_info.dram_num);
		rtd_pr_rmm_err("dram_info->swap_en is %x\n", dram_info.swap_en);
		rtd_pr_rmm_err("dram_info->boundary is %x\n", dram_info.boundary);
	}
	else
	{
		rtd_pr_rmm_err("fail to get dram info\n");
	}

	rtd_pr_rmm_err("0x12000000 is %s bw region\n", rtk_dc_query_dram_region_by_addr(0x12000000)?"lower":"higher");
	rtd_pr_rmm_err("0x20000000 is %s bw region\n", rtk_dc_query_dram_region_by_addr(0x20000000)?"lower":"higher");
	rtd_pr_rmm_err("0x21000200 is %s bw region\n", rtk_dc_query_dram_region_by_addr(0x21000200)?"lower":"higher");
	rtd_pr_rmm_err("0x28004000 is %s bw region\n", rtk_dc_query_dram_region_by_addr(0x28004000)?"lower":"higher");
	rtd_pr_rmm_err("0x34004200 is %s bw region\n", rtk_dc_query_dram_region_by_addr(0x34004200)?"lower":"higher");

	return 0;
}
late_initcall(rtk_dram_info_dump);
#endif
#endif // ENABLE_DC_API_SUPPORT

/* Decide if there are different ddr sizes */
static int is_ddr_diff_size_enabled(void)
{
#if defined (CONFIG_RTK_KDRIVER_SUPPORT) && defined (ENABLE_DC_API_SUPPORT)
	dc_sys_dc_sys_misc_RBUS dc_sys_misc;

	dc_sys_misc.regValue = rtd_inl(DC_SYS_DC_SYS_MISC_reg);

	return dc_sys_misc.en_ddr_diff_size;
#else
	return 0;
#endif
}

/* Get ddr boundary if there are different ddr sizes */
static __maybe_unused unsigned long get_diff_ddr_boundary(void)
{
#if defined (CONFIG_RTK_KDRIVER_SUPPORT) && defined (ENABLE_DC_API_SUPPORT)
	__maybe_unused RTK_DC_RET_t ret;
	__maybe_unused RTK_DC_INFO_t dc_info;
#endif

	if (!is_ddr_diff_size_enabled())
		return 0;

#if defined (CONFIG_RTK_KDRIVER_SUPPORT) && defined (ENABLE_DC_API_SUPPORT)
	ret = rtk_dc_get_dram_info(&dc_info);
	if (ret == RTK_DC_RET_FAIL) {
		rtd_pr_rmm_err("warning, dram info incorrect\n");
		return 0;
	}
	return dc_info.boundary;
#else
	return 0;
#endif
}

/* Decide if ddr swap is enabled */
int is_ddr_swap(void)
{
#if defined (CONFIG_RTK_KDRIVER_SUPPORT) && defined (ENABLE_DC_API_SUPPORT)
	RTK_DC_RET_t ret;
	RTK_DC_INFO_t dc_info;

	ret = rtk_dc_get_dram_info(&dc_info);
	if (ret == RTK_DC_RET_FAIL) {
		rtd_pr_rmm_err("warning, dram info incorrect\n");
		return 0;
	}
	return dc_info.swap_en;
#else
	return 0;
#endif
}

unsigned long get_memc_start_address(void)
{
	unsigned long memc_size = 0;
	unsigned long memc_addr = 0;

	memc_size = carvedout_buf_query(CARVEDOUT_SCALER_MEMC, (void **)&memc_addr);

	return memc_addr;
}
EXPORT_SYMBOL(get_memc_start_address);

int get_memc_size(void)
{
	unsigned long memc_addr = 0;

	return carvedout_buf_query(CARVEDOUT_SCALER_MEMC,(void **) &memc_addr);
}
EXPORT_SYMBOL(get_memc_size);

unsigned long get_scaler_mdomain_start_address(void)
{
	unsigned long mdomain_size = 0;
	unsigned long mdomain_addr = 0;

	mdomain_size = carvedout_buf_query(CARVEDOUT_SCALER_MDOMAIN, (void **)&mdomain_addr);

	return mdomain_addr;
}
EXPORT_SYMBOL(get_scaler_mdomain_start_address);

int get_scaler_mdomain_size(void)
{
	unsigned long mdomain_addr = 0;

	return carvedout_buf_query(CARVEDOUT_SCALER_MDOMAIN, (void **)&mdomain_addr);
}
EXPORT_SYMBOL(get_scaler_mdomain_size);

unsigned long get_scaler_od_start_address(void)
{
	unsigned long od_size = 0;
	unsigned long od_addr = 0;

	od_size = carvedout_buf_query(CARVEDOUT_SCALER_OD, (void **)&od_addr);

	return od_addr;
}
EXPORT_SYMBOL(get_scaler_od_start_address);

int get_scaler_od_size(void)
{
	unsigned long od_addr = 0;

	return carvedout_buf_query(CARVEDOUT_SCALER_OD, (void **)&od_addr);
}
EXPORT_SYMBOL(get_scaler_od_size);


unsigned int carvedout_buf_get_layout_idx(void)
{
    unsigned int layout_idx = MM_LAYOUT_DEFAULT;

	if (unlikely(DRAM_size==0)) {
		//debug test if DRAM_size was referred by before it's valid.
		rtd_pr_rmm_err("%s ERROR DRAM_size info is not available (%pS)\n", __func__, (void*)_RET_IP_);
	}

	if (get_product_type() == PRODUCT_TYPE_DIAS) {
		layout_idx = MM_LAYOUT_DIAS_2GB;
	} else { //TV
		layout_idx = MM_LAYOUT_DEFAULT;
	}

    return layout_idx;

}
EXPORT_SYMBOL(carvedout_buf_get_layout_idx);

unsigned long carvedout_buf_query(carvedout_buf_t what, void **addr)
{
	unsigned long size = 0;
	void *address = NULL;
	unsigned int layout_idx = carvedout_buf_get_layout_idx();

#ifdef RTK_MEM_LAYOUT_DEVICETREE

    //OBSOLETED can be removed after GKI completed.
	if (unlikely(DRAM_size==0)) {
        //debug test if DRAM_size was referred by before it's valid. [obsoleted]
		rtd_pr_rmm_err("%s ERROR DRAM_size info is not available (%pS)\n", __func__, (void*)_RET_IP_);
	}

#endif

	if (what >= CARVEDOUT_NUM) {
		rtd_pr_rmm_err("Input carvedout query index (%d) is out of range %d\n", what, CARVEDOUT_NUM);
		return 0;
	}

	/* this switch case is list by address order */
	switch (what)
	{
#if 0
	case CARVEDOUT_DDR_BOUNDARY:
		if (DRAM_size == 0x60000) {
			address = (void *)0x1ffff000; // DDR swap is enabled on 1.5GB, so boundary is at 512 MB
			size = 0x1000;
		} else {
			address = (void *)0;
			size = 0;
		}
		break;

	case CARVEDOUT_DDR_BOUNDARY_2:
		if (DRAM_size >= 0x40000) {
			size = PAGE_SIZE << pageblock_order;
		} else {
			size = PAGE_SIZE;
		}
		address = (void *)((DRAM_size << PAGE_SHIFT) - size);
		break;
#endif

	case CARVEDOUT_GPU_RESERVED:
#ifndef RTK_MEM_LAYOUT_DEVICETREE
		if (layout_idx == MM_LAYOUT_DEFAULT) {
			size = CMA_HIGHMEM_TV_4K;
		} else if (layout_idx == MM_LAYOUT_DIAS_2GB) {
			size = CMA_HIGHMEM_DIAS_2GB;
		} else {
			rtd_pr_rmm_err("carvedout_buf_query, CARVEDOUT_GPU_RESERVED, Invalid layout index %d\n", layout_idx);
			size = 0;
		}
		rtd_pr_rmm_info("[MEM] Highmem size is 0x%lx (%ld MB)\n", size, size/1024/1024);
#endif
		break;

#if 0
	case CARVEDOUT_CMA_LOW:
		{
			static bool qs_audio_cma_adjusted = false;

			if (is_QS_acpu_enable() && !qs_audio_cma_adjusted) {
				carvedout_buf[layout_idx][CARVEDOUT_CMA_LOW][1] -= AUDIO_QS_BUFFER_SIZE;
				qs_audio_cma_adjusted = true;
			}
		}
		address = (void *)carvedout_buf[layout_idx][what][0];
		size = carvedout_buf[layout_idx][what][1];
		break;
	case CARVEDOUT_QS_AUDIO:
		{
			static bool qs_audio_carved_adjusted = false;

			if (is_QS_acpu_enable() && !qs_audio_carved_adjusted) {
				carvedout_buf[layout_idx][what][1] = AUDIO_QS_BUFFER_SIZE;
				qs_audio_carved_adjusted = true;
			}
		}
		address = (void *)carvedout_buf[layout_idx][what][0];
		size = carvedout_buf[layout_idx][what][1];
		break;
#endif

	default:
#if 1//ndef CONFIG_ARM64  //FIXME
		address = (void *)carvedout_buf[layout_idx][what][0];
		size = carvedout_buf[layout_idx][what][1];
#else
		size = fdt_get_carvedout_mem_info(carvedout_mem_dts_name[what], &address);
#endif

#if defined(CONFIG_REALTEK_SECURE) && !IS_ENABLED(CONFIG_RTK_KDRV_TEE)
		if (what == CARVEDOUT_K_OS)
			size = 32*_MB_;
#endif
#if !defined(CONFIG_REALTEK_SECURE_DDK)
		if (what == CARVEDOUT_GPU_FW)
			size = _TBD_;
#endif
#if !IS_ENABLED(CONFIG_RTK_KDRV_RPC)
		if (what == CARVEDOUT_MAP_RPC)
			size = _TBD_;
#endif
#if !IS_ENABLED(CONFIG_REALTEK_LOGBUF)
		if (what == CARVEDOUT_LOGBUF)
			size = _TBD_;
#endif
#if !defined(CONFIG_PSTORE)
		if (what == CARVEDOUT_RAMOOPS)
			size = _TBD_;
#endif
#if !defined(CONFIG_CUSTOMER_TV006)
		if (what == CARVEDOUT_GAL)
			size = _TBD_;
#endif
#if !defined(CONFIG_LG_SNAPSHOT_BOOT)
		if (what == CARVEDOUT_SNAPSHOT)
			size = _TBD_;
#endif
#if !defined(RTK_MEM_LAYOUT_DEVICETREE) && !defined(BUILD_QUICK_SHOW)
//FIXME this special hack is supposed to be deprecated.
#if defined(CONFIG_VBM_CMA)
		if (what == CARVEDOUT_VDEC_VBM)
			size = _TBD_;
#else
		if (what == CARVEDOUT_CMA_VBM)
			size = _TBD_;
#endif
#endif
		break;

//  implement your own specific case
//	case :
//		break;
	}

	if (addr)
		*addr = address;

	return size;
}
EXPORT_SYMBOL(carvedout_buf_query);

#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
unsigned long carvedout_buf_query_secure(carvedout_buf_t what, void **addr)
{
	unsigned long size = 0, end = 0;
	void *address = NULL;
	unsigned int layout_idx = carvedout_buf_get_layout_idx();

#define SVP_PROT_ALIGN 1024

#ifdef RTK_MEM_LAYOUT_DEVICETREE //OBSOLETED

	if (unlikely(DRAM_size==0)) {
		rtd_pr_rmm_err("%s ERROR DRAM_size info is not available (%pS)\n", __func__, (void*)_RET_IP_);
	}

#endif

	if (what >= CARVEDOUT_NUM) {
		rtd_pr_rmm_err("Input carvedout query index (%d) is out of range %d\n", what, CARVEDOUT_NUM);
		return 0;
	}

	/* this switch case is list by address order */
	switch (what)
	{
	/* defaut protect MEMC / MDOMAIN / OD / DI_NR*/
	case CARVEDOUT_SCALER_MEMC:
	case CARVEDOUT_SCALER_MDOMAIN:
	case CARVEDOUT_SCALER_OD:
	case CARVEDOUT_SCALER_DI_NR:
	case CARVEDOUT_SCALER_VIP:
		address = (void *)carvedout_buf[layout_idx][what][0];
		size = carvedout_buf[layout_idx][what][1];

		if(address > 0 && size > 0) {
			end = (unsigned long)address + size;
			end = rtk_get_dc_secure_aligned_addr(((unsigned long)address + size), 0);
			address = (void *)(unsigned long)rtk_get_dc_secure_aligned_addr((unsigned long)address, 1);
			size = end - (unsigned long)address;
		}
		break;
	default:
#if 1//ndef CONFIG_ARM64 //FIXME
		address = (void *)carvedout_buf[layout_idx][what][0];
		size = carvedout_buf[layout_idx][what][1];
		if(address > 0 && size > 0) {
			end = (unsigned long)address + size;
			end = rtk_get_dc_secure_aligned_addr(((unsigned long)address + size), 0);
			address = (void *)(unsigned long)rtk_get_dc_secure_aligned_addr((unsigned long)address, 1);
			size = end - (unsigned long)address;
		}
		rtd_pr_rmm_info("[MEM] carvedout query secure addr=%p size=%lx\n", address, size);
#else
		size = fdt_get_carvedout_mem_info(carvedout_mem_dts_name[what], &address);
#endif

#if defined(CONFIG_REALTEK_SECURE) && !IS_ENABLED(CONFIG_RTK_KDRV_TEE)
		if (what == CARVEDOUT_K_OS)
			size = 32*_MB_;
#endif
#if !defined(CONFIG_REALTEK_SECURE_DDK)
		if (what == CARVEDOUT_GPU_FW)
			size = _TBD_;
#endif
#if !IS_ENABLED(CONFIG_RTK_KDRV_RPC)
		if (what == CARVEDOUT_MAP_RPC)
			size = _TBD_;
#endif
#if !IS_ENABLED(CONFIG_REALTEK_LOGBUF)
		if (what == CARVEDOUT_LOGBUF)
			size = _TBD_;
#endif
#if !defined(CONFIG_PSTORE)
		if (what == CARVEDOUT_RAMOOPS)
			size = _TBD_;
#endif
#if !defined(CONFIG_CUSTOMER_TV006)
		if (what == CARVEDOUT_GAL)
			size = _TBD_;
#endif
#if !defined(CONFIG_LG_SNAPSHOT_BOOT)
		if (what == CARVEDOUT_SNAPSHOT)
			size = _TBD_;
#endif
#ifndef RTK_MEM_LAYOUT_DEVICETREE
#if defined(CONFIG_VBM_CMA)
		if (what == CARVEDOUT_VDEC_VBM)
			size = _TBD_;
#else
		if (what == CARVEDOUT_CMA_VBM)
			size = _TBD_;
#endif
#endif
		break;

//  implement your own specific case
//	case :
//		break;
	}

	if (addr)
		*addr = address;

	return size;
}
#endif

unsigned long carvedout_buf_query_range(carvedout_buf_t idx_start, carvedout_buf_t idx_end, void **start, void **end)
{
	unsigned long size = 0;
	unsigned long min_addr = ULONG_MAX;
	unsigned long max_addr = 0;
	int i = 0;
	int idx_a, idx_b;

	if ((idx_start >= CARVEDOUT_NUM) || (idx_end >= CARVEDOUT_NUM)) {
		rtd_pr_rmm_err("Error! One of carvedout input range index (%d, %d) is out of bounds %d\n", idx_start, idx_end, CARVEDOUT_NUM);
		return 0;
	}

	// index rule : a < b
	idx_a = idx_start;
	idx_b = idx_end;
	if (idx_start > idx_end) { // swap
		idx_a = idx_end;
		idx_b = idx_start;
	}

	for (i = idx_a; i <= idx_b; i++) {
		unsigned long tmp_addr = 0;
		unsigned long tmp_size = 0;

		tmp_size = carvedout_buf_query((carvedout_buf_t) i, (void **)&tmp_addr);
		if (tmp_size) {
			min_addr = min(min_addr, tmp_addr);
			max_addr = max(max_addr, tmp_addr + tmp_size); // end address
			size += tmp_size;
			//rtd_pr_rmm_info("idx=%d, size=%lx, min_addr=%lx, max_addr=%lx\n", i, tmp_size, min_addr, max_addr);
		}
#ifdef CONFIG_VBM_CMA
		if (i == CARVEDOUT_VDEC_VBM) {
			tmp_size = carvedout_buf_query((carvedout_buf_t)CARVEDOUT_CMA_VBM , (void **)&tmp_addr);
			if (tmp_size) {
				min_addr = min(min_addr, tmp_addr);
				max_addr = max(max_addr, tmp_addr + tmp_size); // end address
				size += tmp_size;
				rtd_pr_rmm_info("idx=%d, size=%lx, min_addr=%lx, max_addr=%lx\n", i, tmp_size, min_addr, max_addr);
			}
		}
#endif
	}

	if (start && end) {
		*start = (void *)min_addr;
		*end   = (void *)max_addr;
		rtd_pr_rmm_info("carved-out total size 0x%lx (0x%lx@ %lx ~ %lx)\n", size, (unsigned long)(*end - *start), (unsigned long)*start, (unsigned long)*end);
	} else {
		rtd_pr_rmm_info("carved-out non given start or end address, size 0x%lx (0x%x ~ 0x%x)\n", size, (unsigned int)min_addr, (unsigned int)max_addr);
	}

	if (max_addr && ((max_addr - min_addr) != size)) {
		rtd_pr_rmm_info("carved-out mismatch? size (0x%lx, 0x%lx), choose min one\n", size, (max_addr - min_addr));
		size = min((max_addr - min_addr), size);
	}

	return size;
}

unsigned int carvedout_fallback_query(carvedout_buf_t what)
{
	switch (what)
	{
	case CARVEDOUT_TP:
		return CARVEDOUT_FALLBACK_TO_CMA;

	default:
		return CARVEDOUT_NO_FALLBACK;
	}
}

/*
 * return negative value for invalid query
 */
int carvedout_buf_query_is_in_range(unsigned long in_addr, void **start, void **end)
{
	int i = 0;
	carvedout_buf_t idx;
	carvedout_buf_t whitelist[] = {CARVEDOUT_BOOTCODE, CARVEDOUT_AUDIO, CARVEDOUT_VDEC_RINGBUF, CARVEDOUT_LOGBUF, CARVEDOUT_SCALER_NN, CARVEDOUT_VDEC_VBM, CARVEDOUT_TP};
	unsigned long size, addr;

	unsigned int whitelist_num = sizeof(whitelist)/sizeof(carvedout_buf_t);

	for (i = 0; i < whitelist_num; i++) { // except cma area
		idx = whitelist[i];
		size = carvedout_buf_query(idx, (void *)&addr);
		if ((idx == CARVEDOUT_BOOTCODE) && size) {
			if ((in_addr >= addr) && (in_addr < (addr + size))) {
				if (start && end) {
					*start = (void *)addr;
					*end   = (void *)(addr + 0x20000);
				}
				rtd_pr_rmm_debug("carvedout_bootcode query(%lx) in idx(%d), range(%lx-%lx)\n", in_addr, idx, addr, addr + size);
				return (int)idx;
			}
		}
		else if(addr && size) {
			if ((in_addr >= addr) && (in_addr < (addr + size))) {
				if (start && end) {
					*start = (void *)addr;
					*end   = (void *)(addr + size);
				}
				rtd_pr_rmm_debug("carvedout_buf query(%lx) in idx(%d), range(%lx-%lx)\n", in_addr, idx, addr, addr + size);
				return (int)idx;
			}
		}
	}

    return -1;
}
EXPORT_SYMBOL(carvedout_buf_query_is_in_range);

#ifndef BUILD_QUICK_SHOW
static const char * const realtek_carvedout_buf_of_match[] = {
	"realtek,rtd2885p",
	NULL
};

static int carvedout_buf_of_compatible_match(struct device_node *device,
			       const char *const *compat)
{
	unsigned int tmp, score = 0;

	if (!compat)
		return 0;

	while (*compat) {
		tmp = of_device_is_compatible(device, *compat);
		if (tmp > score)
			score = tmp;
		compat++;
	}

	return score;
}

static int carvedout_buf_get_memory(struct device_node *np)
{
    static int cnt = 0;
	struct device_node *node;
	struct resource r;
	int i = 0, ret = 0;

	if (unlikely(DRAM_size!=0)) {
		rtd_pr_rmm_err("%s ERROR DRAM_size was updated before (%pS)\n", __func__, (void*)_RET_IP_);
	}

	node = of_parse_phandle(np, "memory-region", 0);
	if (!node) {
		rtd_pr_rmm_err("no memory-region\n");
		return -EINVAL;
	}

	for (;;i++) {
		size_t size;
		ret = of_address_to_resource(node, i, &r);
		if (ret) {
			rtd_pr_rmm_debug("resource[%d] error(%d), stop\n", i, ret);
			break;
		}
		if (i > 10) { // avoid lock
			rtd_pr_rmm_err("resource[%d] too much, break\n", i);
			break;
		}
		size = resource_size(&r);

		rtd_pr_rmm_info("mem=%lldMB @ 0x%08llx\n", size / 1024 / 1024, r.start);
		if (!cnt++)
			add_memory_size(GFP_DCU1, (unsigned long)size);
		else
			add_memory_size(GFP_DCU2, (unsigned long)size);

		DRAM_size += (size >> PAGE_SHIFT);
	}

	rtd_pr_rmm_info("%s DRAM_size %d\n", __func__, DRAM_size);

	return 0;
}


#if 0 // refer carvedout_buf_layout_build for logbuf size update
#ifdef CONFIG_REALTEK_LOGBUF
__attribute__((weak)) unsigned long rtdlog_get_buffer_size(void)
{
    rtd_pr_rmm_err("%s[weak] is not available\n", __func__);
    return 0;
}
#endif
#endif


//TODO: it could be a common code
int carvedout_buf_layout_build(void)
{
	int ret = 0;
	struct device_node *np, *child, *rmm;
	const char *comp;
	unsigned int layout_idx;
	const char *rmm_child;

	// compatible info for memory layout
	rmm = NULL;
	do  {
		// find next node reserved-memory, or reserved-memory@XXX
		rmm = of_find_node_by_name(rmm, "reserved-memory");
		if (rmm) {
			rtd_pr_rmm_info("fdt 0x%x %s\n", rmm, rmm->full_name);
			if (strcmp(rmm->full_name, "reserved-memory") == 0) {
				rtd_pr_rmm_notice("found 0x%x %s\n", rmm, rmm->full_name);
				break;
			}
		}
		else {
			rtd_pr_rmm_err("ERROR no reserved-memory found\n");
			goto err;
		}
	} while (rmm);

	comp = (const char*)of_get_property(rmm, "compatible", NULL);
	if (comp) {
		rtd_pr_rmm_notice("%s compatible %s\n", rmm->name ,comp);
	}
	else {
		rtd_pr_rmm_err("ERROR no %s compatible %s\n", rmm->name, comp);
	}

	np = of_find_node_by_name(NULL, "carvedout_buf");
	if (!carvedout_buf_of_compatible_match(np, realtek_carvedout_buf_of_match))
	{
		rtd_pr_rmm_err("ERROR no carvedout_buf match\n");
		ret = -EINVAL;
		goto err;
	}

	if (carvedout_buf_get_memory(np) < 0) {
		rtd_pr_rmm_err("ERROR no memory-region\n");
		ret = -EINVAL;
		goto err;
	}
 	layout_idx = carvedout_buf_get_layout_idx();
	rtd_pr_rmm_notice("carvedout_buf selected idx %d\n", layout_idx);

	for_each_child_of_node(np, child) {
		size_t size;
		struct resource r;
		struct device_node *node;
		const __be32 *addrp;
		unsigned int carvedout_idx = -1;
		unsigned long *carvedout;

		addrp = of_get_address(child, 0, NULL, NULL);
		if (!addrp) {
			rtd_pr_rmm_err("ERROR no carvedout_buf idx for %s\n", child->full_name);
			ret = -EINVAL;
			continue;
		} else {
			carvedout_idx = (unsigned int)be32_to_cpu(*addrp);
		}

		// For not using overlay. carvedout_buf mapping to regions by name instead of phandle.
		//  If not using overlay and not multiple dtbs, all layouts can be defined in single dts
		//  with different names (e.g. 'reserved-memory#1', 'reserverd-memory#2', etc) and let loader
		//  to fixup selected one to be native node 'reserved-memory' at final fdt. But carvedout_buf
		//  mapping cannot not use same phandle between nodes and has to be /path instead.
		rmm_child = (const char*)of_get_property(child, "memory-region", NULL);
		node = of_get_child_by_name(rmm, rmm_child);
		if (!node) {
			rtd_pr_rmm_err("ERROR no memory-region for %s\n", child->full_name);
			ret = -EINVAL;
			continue;
		}

		ret = of_address_to_resource(node, 0, &r);
		if (ret) {
			rtd_pr_rmm_err("ERROR resource error(%d) for %s\n", ret, child->full_name);
			ret = -EINVAL;
			continue;
		}
		size = resource_size(&r);

		//! also not available if status not-'okay' or "disabled"
		if (!of_device_is_available(node)) {
			rtd_pr_rmm_notice("%s%s is disabled(size 0)\n", rmm_child, child->full_name);
			size = 0;
		}

		/*
		 * legacy kdriver and it's user (e.g. quick-show) may still refer predefined static in builtin carvedout_buf[] for some reason
		 * in case somehow it's NOT sync'd with devicetree, here's a favor being a Canary during update traversal.
		 * Precodintion:
		 * 1. both devicetree(*.dtb) and carvedout_buf[] refer the same carvedout index (rmm_carvedout.h)
		 * 2. layouts listed in carvedout mapping on devicetree
		 */
		carvedout = carvedout_buf[layout_idx][carvedout_idx];
		if (carvedout[1]) {
			if ((carvedout[1] != (unsigned long)size) || (carvedout[0] != (unsigned long)r.start)) {
				rtd_pr_rmm_err("%s%s (0x%08x/0x%08x)->(0x%08x/0x%08x) Mismatched \n",
				  rmm_child, child->full_name, carvedout[0], carvedout[1], (unsigned long)r.start, (unsigned long)size);
			}
		}
		else if (size) {
			rtd_pr_rmm_notice("%s%s (0x%08x/0x%08x)->(0x%08x/0x%08x) Updated\n",
			  rmm_child, child->full_name, carvedout[0], carvedout[1], (unsigned long)r.start, (unsigned long)size);

			carvedout[0] = (unsigned long)r.start;
			carvedout[1] = (unsigned long)size;
		}

#if 0 //debug
	{
        unsigned long size, addr;
        size = carvedout_buf_query(carvedout_idx, (void *)&addr);
        rtd_pr_rmm_info("carvedout %d start(%lx)..(%lx), size(%ld_MB))\n", carvedout_idx, addr, addr + size, (size/1024/1024));
	}
#endif

#if 0 // RTK_MEM_LAYOUT_DEVICETREE
// if uboot cannot directly adjust the logbuf bufsize in fdt, here return(free) the unused reserved region back to buddy system
// ref: cma_init_reserved_areas(...)
//  1. MemTotal info will be update after __free
//  2. managed info should be updated manually
//  3. memblock reserved region may need to free
//  4. rtdlog_get_buffer_size( ) should be ready early.
#ifdef CONFIG_REALTEK_LOGBUF
        if (carvedout_buf_idx == CARVEDOUT_LOGBUF)
        {
			//? rtdlog_parse_bufsize should be prior than rmm init
            unsigned long bufsize = rtdlog_get_buffer_size();
            struct page *pg;
            int cnt, odr;

            if (size > bufsize)
            {
                rtd_pr_rmm_info("%s(%x/%x) size updated %x \n", child->name, r.start, size, bufsize);

                pg = pfn_to_page(PFN_UP(r.start + bufsize));
                //odr = min(MAX_ORDER-1, get_order(bufsize));
                odr = 0;
                cnt = (size - bufsize) / PAGE_SIZE;
                do {
                    while (cnt < (1<<odr)) {
                        odr--;
                    }
                    rtd_pr_rmm_debug("%s free %x order %d\n", child->name, PFN_PHYS(page_to_pfn(pg)), odr);

                    __ClearPageReserved(pg);
                    //set_page_count(pg, 0);
                    set_page_count(pg, 1); //set_page_refcounted(pg);

                    // it could also be 2 loop phases:
                    // 1.clear flags for each page, 2.free pages from high order to 0-order.
                    __free_pages(pg, odr);

                    pg = pg + (1<<odr);
                    cnt = cnt - (1<<odr);
                } while (cnt);

                adjust_managed_page_count(pg, (size - bufsize) / PAGE_SIZE); // although 'memtotal' will be updated after free but not 'managed'
                memblock_free(r.start+bufsize, size-bufsize); // although reserve was returned to buddy, but not memblock reserved list.
            }
            else {
                rtd_pr_rmm_err("%s size %d overflow\n", child->name, bufsize);
            }
            carvedout_buf[layout_idx][CARVEDOUT_LOGBUF][1] = bufsize;
        }
#endif
#endif //RTK_MEM_LAYOUT_DEVICETREE

	}

    //[TODO] not to use CARVEDOUT_SCALER anymore or only one united scaler region in devicetree
    carvedout_buf[layout_idx][CARVEDOUT_SCALER][0] = carvedout_buf[layout_idx][CARVEDOUT_SCALER_MEMC][0];
    carvedout_buf[layout_idx][CARVEDOUT_SCALER][1] = DO_ALIGNMENT( (
      carvedout_buf[layout_idx][CARVEDOUT_SCALER_MEMC][1] +
      carvedout_buf[layout_idx][CARVEDOUT_SCALER_MDOMAIN][1] +
      carvedout_buf[layout_idx][CARVEDOUT_SCALER_DI_NR][1] +
      carvedout_buf[layout_idx][CARVEDOUT_SCALER_OD][1] +
      carvedout_buf[layout_idx][CARVEDOUT_SCALER_VIP][1] +
      carvedout_buf[layout_idx][CARVEDOUT_SCALER_NN][1] ), 4*_MB_);

	if (ret)
		goto err;

#if 1 //debug
    {
        unsigned long size, addr;
        size = carvedout_buf_query(CARVEDOUT_SCALER_MEMC, (void *)&addr);
        rtd_pr_rmm_info("carvedout memc start(%lx)..(%lx), size(%ld_MB))\n", addr, addr + size, (size/1024/1024));
        size = carvedout_buf_query(CARVEDOUT_SCALER_MDOMAIN, (void *)&addr);
        rtd_pr_rmm_info("carvedout mdomain start(%lx)..(%lx), size(%ld_MB))\n", addr, addr + size, (size/1024/1024));
        size = carvedout_buf_query(CARVEDOUT_SCALER_DI_NR, (void *)&addr);
        rtd_pr_rmm_info("carvedout di_nr start(%lx)..(%lx), size(%ld_MB))\n", addr, addr + size, (size/1024/1024));
        size = carvedout_buf_query(CARVEDOUT_SCALER_NN, (void *)&addr);
        rtd_pr_rmm_info("carvedout nn start(%lx)..(%lx), size(%ld_MB))\n", addr, addr + size, (size/1024/1024));
        size = carvedout_buf_query(CARVEDOUT_SCALER_VIP, (void *)&addr);
        rtd_pr_rmm_info("carvedout vip start(%lx)..(%lx), size(%ld_KB))\n", addr, addr + size, (size/1024));
        size = carvedout_buf_query(CARVEDOUT_SCALER_OD, (void *)&addr);
        rtd_pr_rmm_info("carvedout od start(%lx)..(%lx), size(%ld_KB))\n", addr, addr + size, (size/1024));
        size = carvedout_buf_query(CARVEDOUT_VDEC_VBM, (void *)&addr);
        rtd_pr_rmm_info("carvedout vbm start(%lx)..(%lx), size(%ld_KB))\n", addr, addr + size, (size/1024));
        size = carvedout_buf_query(CARVEDOUT_TP, (void *)&addr);
        rtd_pr_rmm_info("carvedout tp start(%lx)..(%lx), size(%ld_MB))\n", addr, addr + size, (size/1024/1024));
        size = carvedout_buf_query(CARVEDOUT_AUDIO, (void *)&addr);
        rtd_pr_rmm_info("carvedout audio start(%lx)..(%lx), size(%ld_MB))\n", addr, addr + size, (size/1024/1024));
        size = carvedout_buf_query(CARVEDOUT_DDR_BOUNDARY, (void *)&addr);
        if (size)
            rtd_pr_rmm_info("carvedout ddr boundary start(%lx)..(%lx), size(%ld_MB))\n", addr, addr + size, (size/1024/1024));
    }
#endif

	return 0;
err:
	return ret;
}
EXPORT_SYMBOL(carvedout_buf_layout_build);


/*
 * reclaim_logo_X weak override
 */
#if defined(CONFIG_REALTEK_MEMORY_MANAGEMENT) // legacy and built-in only

int reclaim_logo = 0; //TODO: it could be a generic logo reclaim solution not only for module

int reclaim_logo_memory(void)
{
	// legacy reserve_dvr_memory() from cma1

	rtd_pr_rmm_notice("%s[rtd2885p]\n", __func__);

	if (pAnimation) {
		dvr_free(pAnimation);
		pAnimation = 0;
	}
	if (pAnimation_1) {
		dvr_free(pAnimation_1);
		pAnimation_1 = 0;
	}

	return 0;
}
#endif


#ifndef CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE
/* RTK_patch: handle the power on video/logo/audio memory issue */
static int __init reserve_dvr_memory(void)
{
	rtd_pr_rmm_info("[POV] reserve_dvr_memory\n");

	/*
	 * if bootcode assign last_image= in kernel cmdline, use bootcode's memory setting.
	 * if kernel_cmd doesn't exist last_image=, last image buffer use 4MB after decode buffer region.
	 */
	if (reserved_dvr_size) {

#if IS_ENABLED(CONFIG_INIT_ON_ALLOC_DEFAULT_ON)
		// hack to temporarily disable CONFIG_INIT_ON_ALLOC_DEFAULT_ON enabled by GKI default
		//  carvedout/vbm reserve/reclaim is an one-time operation to sustain buffer hand-shaking
		//  before and after kernel, it's not supposed to do any harm to the optimization intention
		//  of static_key.
		bool key_enabled;
		key_enabled = static_key_enabled(&init_on_alloc.key);
		if (key_enabled) {
			static_key_disable(&init_on_alloc.key);
		}
#endif

		rtd_pr_rmm_info("[POV] reserved_dvr_start = 0x%x\n",(unsigned int)reserved_dvr_start);
		pAnimation = dvr_malloc_specific_addr(64*1024*1024,
											  GFP_DCU1, reserved_dvr_start);
		pAnimation_1 = dvr_malloc_specific_addr(reserved_dvr_size - 64*1024*1024,
												GFP_DCU1, reserved_dvr_start + 0x04000000);

		rtd_pr_rmm_info("[POV] pAnimation(%p/%p)\n", pAnimation, pAnimation_1);

		#ifdef CONFIG_LG_SNAPSHOT_BOOT
		register_cma_forbidden_region(__phys_to_pfn(reserved_dvr_start), reserved_dvr_size);
		#endif
		rtd_pr_rmm_info("[POV] reserved_last_image_start = 0x%x\n",(unsigned int)reserved_last_image_start);
		if (reserved_last_image_size) {
			pLastImage = dvr_malloc_specific_addr(reserved_last_image_size,
												  GFP_DCU1, reserved_last_image_start);
		}
		else {
			pLastImage = dvr_malloc_specific_addr(LAST_IMAGE_SIZE,
												  GFP_DCU1, reserved_dvr_start+reserved_dvr_size);
		}
		rtd_pr_rmm_info("[POV] pLastImage(%p)\n", pLastImage);

#if IS_ENABLED(CONFIG_INIT_ON_ALLOC_DEFAULT_ON)
		if (key_enabled) {
			static_key_enable(&init_on_alloc.key);
		}
#endif

	}
	return 0;
}
core_initcall_sync(reserve_dvr_memory);

#if IS_ENABLED(CONFIG_REALTEK_LOGBUF)
extern unsigned long rtdlog_get_buffer_size(void);
#endif

void __init rtk_reserve(void)
{
	unsigned int ret = 0;
	unsigned int layout_idx = carvedout_buf_get_layout_idx();
    unsigned long size = 0, addr = 0;

	rtd_pr_rmm_info("[MEM] rtd2885p rtk_reserve, DRAM_size = 0x%x, layout_idx = 0x%x\n", (unsigned int)DRAM_size, layout_idx);

	/* for audio carvedout memory  */
	size = carvedout_buf_query(CARVEDOUT_AUDIO, (void *)&addr);
	if (size)
		memblock_remove(addr, size);

	/* for bootcode data */
	memblock_reserve(carvedout_buf[layout_idx][CARVEDOUT_BOOTCODE][0], carvedout_buf[layout_idx][CARVEDOUT_BOOTCODE][1]);
#if defined(CONFIG_REALTEK_RPC) ||defined(CONFIG_RTK_KDRV_RPC)
	/* for RPC temporarily */
	memblock_reserve(carvedout_buf[layout_idx][CARVEDOUT_MAP_RPC][0], carvedout_buf[layout_idx][CARVEDOUT_MAP_RPC][1]);
#endif
	/* for demod fw */
	if (carvedout_buf[layout_idx][CARVEDOUT_DEMOD][1])
		ret |= add_ban_info(carvedout_buf[layout_idx][CARVEDOUT_DEMOD][0], carvedout_buf[layout_idx][CARVEDOUT_DEMOD][1], BAN_NOT_USED);

	/* for rbus & nor flash*/
	memblock_remove(carvedout_buf[layout_idx][CARVEDOUT_MAP_RBUS][0], carvedout_buf[layout_idx][CARVEDOUT_MAP_RBUS][1]);

#ifdef CONFIG_REALTEK_SECURE_DDK
	/* for GPU FW */
	memblock_reserve(carvedout_buf[layout_idx][CARVEDOUT_GPU_FW][0], carvedout_buf[layout_idx][CARVEDOUT_GPU_FW][1]);
#endif//

	/* for A & V fw */
	memblock_reserve(carvedout_buf[layout_idx][CARVEDOUT_V_OS][0], carvedout_buf[layout_idx][CARVEDOUT_V_OS][1]);
	memblock_reserve(carvedout_buf[layout_idx][CARVEDOUT_A_OS][0], carvedout_buf[layout_idx][CARVEDOUT_A_OS][1]);

#if IS_ENABLED(CONFIG_REALTEK_LOGBUF)
	carvedout_buf[layout_idx][CARVEDOUT_LOGBUF][1] = rtdlog_get_buffer_size();
	memblock_reserve(carvedout_buf[layout_idx][CARVEDOUT_LOGBUF][0], carvedout_buf[layout_idx][CARVEDOUT_LOGBUF][1]);
#endif

	/* for rom code */
	memblock_reserve(carvedout_buf[layout_idx][CARVEDOUT_ROMCODE][0], carvedout_buf[layout_idx][CARVEDOUT_ROMCODE][1]);

#ifdef CONFIG_PSTORE
	/* for ramoops */
	memblock_reserve(carvedout_buf[layout_idx][CARVEDOUT_RAMOOPS][0], carvedout_buf[layout_idx][CARVEDOUT_RAMOOPS][1]);
#endif

	/* for ib boundary */
	if (carvedout_buf[layout_idx][CARVEDOUT_IR_TABLE][1])
		memblock_reserve(carvedout_buf[layout_idx][CARVEDOUT_IR_TABLE][0], carvedout_buf[layout_idx][CARVEDOUT_IR_TABLE][1]);

	/* for ddr boundary */
    size = carvedout_buf_query(CARVEDOUT_DDR_BOUNDARY, (void *)&addr);
	if (size)
		memblock_reserve(addr, size);

	/* for ddr end boundary */
	size = carvedout_buf_query(CARVEDOUT_DDR_BOUNDARY_2, (void *)&addr);
	if (size == (PAGE_SIZE << pageblock_order)) {
		ret |= add_ban_info(addr, size, BAN_NOT_USED);
	} else if (size == PAGE_SIZE) {
		memblock_reserve(addr, size);
	}

	/* for audio & video DMEM */
	if (carvedout_buf[layout_idx][CARVEDOUT_AV_DMEM][1])
		ret |= add_ban_info(carvedout_buf[layout_idx][CARVEDOUT_AV_DMEM][0], carvedout_buf[layout_idx][CARVEDOUT_AV_DMEM][1], BAN_NORMAL);

	memblock_remove(carvedout_buf[layout_idx][CARVEDOUT_VDEC_RINGBUF][0], carvedout_buf[layout_idx][CARVEDOUT_VDEC_RINGBUF][1]);   // need content protection

//	if (carvedout_buf[layout_idx][CARVEDOUT_AUDIO_RINGBUF][1])
//		memblock_remove(carvedout_buf[layout_idx][CARVEDOUT_AUDIO_RINGBUF][0], carvedout_buf[layout_idx][CARVEDOUT_AUDIO_RINGBUF][1]);   // need content protection

#ifdef CONFIG_REALTEK_SECURE
	/* for secure boot */
	memblock_reserve(carvedout_buf[layout_idx][CARVEDOUT_K_BOOT][0], carvedout_buf[layout_idx][CARVEDOUT_K_BOOT][1]);
	/* for secure OS */
	size = carvedout_buf_query(CARVEDOUT_K_OS, (void *)&addr);
	if (size > 0)
		memblock_remove(addr, size);
#endif

#if 0
    ret |= add_ban_info(carvedout_buf[layout_idx][CARVEDOUT_KASAN][0], carvedout_buf[layout_idx][CARVEDOUT_KASAN][1], BAN_NOT_USED);
#endif


#ifdef CONFIG_CUSTOMER_TV006
{
	ret |= add_ban_info(carvedout_buf[layout_idx][CARVEDOUT_GAL][0], carvedout_buf[layout_idx][CARVEDOUT_GAL][1], BAN_NOT_USED);
}
#endif

#ifdef CONFIG_LG_SNAPSHOT_BOOT
	if (snapshot_enable) {
		if (!add_ban_info(carvedout_buf[layout_idx][CARVEDOUT_SNAPSHOT][0], carvedout_buf[layout_idx][CARVEDOUT_SNAPSHOT][1], BAN_NOT_USED))
			reserve_boot_memory = 1;
	}
#endif

#ifdef BOOT_MEMORY_END
	BUG_ON(SCLAER_MODULE_START != BOOT_MEMORY_END);
#endif

	/* for scaler (memc/mdomain/vip/OD) */
	size = carvedout_buf_query(CARVEDOUT_SCALER, (void *)&addr);
	if (size)
		memblock_remove(addr, size);

    /* for vdec framebuffer */
	size = carvedout_buf_query(CARVEDOUT_VDEC_VBM, (void *)&addr);
	if (size)
		memblock_remove(addr, size);

	/* for demux */
	size = carvedout_buf_query(CARVEDOUT_TP, (void *)&addr);
	if (size)
		memblock_remove(addr, size);

	if (ret)
		panic("add_ban_info fail\n");

#if 1 //debug
    {
        unsigned long size, addr;
        size = carvedout_buf_query(CARVEDOUT_SCALER_MEMC, (void *)&addr);
        rtd_pr_rmm_info("carvedout memc start(%lx)..(%lx), size(%ld_MB))\n", addr, addr + size, (size/1024/1024));
        size = carvedout_buf_query(CARVEDOUT_SCALER_MDOMAIN, (void *)&addr);
        rtd_pr_rmm_info("carvedout mdomain start(%lx)..(%lx), size(%ld_MB))\n", addr, addr + size, (size/1024/1024));
        size = carvedout_buf_query(CARVEDOUT_SCALER_DI_NR, (void *)&addr);
        rtd_pr_rmm_info("carvedout di_nr start(%lx)..(%lx), size(%ld_MB))\n", addr, addr + size, (size/1024/1024));
        size = carvedout_buf_query(CARVEDOUT_SCALER_NN, (void *)&addr);
        rtd_pr_rmm_info("carvedout nn start(%lx)..(%lx), size(%ld_MB))\n", addr, addr + size, (size/1024/1024));
        size = carvedout_buf_query(CARVEDOUT_SCALER_VIP, (void *)&addr);
        rtd_pr_rmm_info("carvedout vip start(%lx)..(%lx), size(%ld_KB))\n", addr, addr + size, (size/1024));
        size = carvedout_buf_query(CARVEDOUT_SCALER_OD, (void *)&addr);
        rtd_pr_rmm_info("carvedout od start(%lx)..(%lx), size(%ld_KB))\n", addr, addr + size, (size/1024));
        size = carvedout_buf_query(CARVEDOUT_VDEC_VBM, (void *)&addr);
        rtd_pr_rmm_info("carvedout vbm start(%lx)..(%lx), size(%ld_KB))\n", addr, addr + size, (size/1024));
        size = carvedout_buf_query(CARVEDOUT_TP, (void *)&addr);
        rtd_pr_rmm_info("carvedout tp start(%lx)..(%lx), size(%ld_MB))\n", addr, addr + size, (size/1024/1024));
        size = carvedout_buf_query(CARVEDOUT_AUDIO, (void *)&addr);
        rtd_pr_rmm_info("carvedout audio start(%lx)..(%lx), size(%ld_MB))\n", addr, addr + size, (size/1024/1024));
        size = carvedout_buf_query(CARVEDOUT_DDR_BOUNDARY, (void *)&addr);
        if (size)
            rtd_pr_rmm_info("carvedout ddr boundary start(%lx)..(%lx), size(%ld_MB))\n", addr, addr + size, (size/1024/1024));
        size = carvedout_buf_query(CARVEDOUT_DDR_BOUNDARY_2, (void *)&addr);
        if (size)
            rtd_pr_rmm_info("carvedout ddr end boundary start(%lx)..(%lx), size(%ld_MB))\n", addr, addr + size, (size/1024/1024));
    }
#endif
}
#endif

int cma_info_proc_show(struct seq_file *m, void *v)
{
	rtk_record_list_dump();
	return 0;
}

static int cma_info_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, cma_info_proc_show, NULL);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static const struct proc_ops cma_info_proc_fops = {
    .proc_open       = cma_info_proc_open,
    .proc_read       = seq_read,
    .proc_lseek     = seq_lseek,
    .proc_release    = single_release,
};
#else
static const struct file_operations cma_info_proc_fops = {
    .open       = cma_info_proc_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
};
#endif

static int __init proc_cma_info_init(void)
{
	struct proc_dir_entry *entry;

    entry = proc_create("cma_info", 0, NULL, &cma_info_proc_fops);
	if (!entry)
		return -1;

    return 0;
}

static int __init rmm_init(void)
{
#if defined(MODULE)
	extern void early_param_init(void);
	early_param_init();
#endif

#if defined(MODULE) || defined(RTK_MEM_LAYOUT_DEVICETREE)
	if (carvedout_buf_layout_build() < 0) {
		rtd_pr_rmm_err("carvedout_buf layout build fail\n");
		return -EINVAL;
	}
#endif

	if (auth_init() < 0)
		return -EINVAL;

	if (remap_init() < 0)
		return -EINVAL;

#ifdef CONFIG_REALTEK_MANAGE_OVERLAPPED_REGION
	init_overlapped_region(RBUS_BASE_PHYS >> PAGE_SHIFT, RBUS_BASE_SIZE >> PAGE_SHIFT);
#endif

	if (proc_cma_info_init() < 0)
		rtd_pr_rmm_err("%s create /proc ERR\n", __func__);
	else
		rtd_pr_rmm_info("%s create /proc OK\n", __func__);

	return 0;
}

static void __exit rmm_exit(void)
{
#ifdef CONFIG_REALTEK_MANAGE_OVERLAPPED_REGION
	exit_overlapped_region();
#endif
	remap_exit();
	auth_exit();
}

#if defined(CONFIG_REALTEK_MEMORY_MANAGEMENT)
pure_initcall(rmm_init);
#elif defined(CONFIG_REALTEK_MEMORY_MANAGEMENT_MODULE)
module_init(rmm_init);
module_exit(rmm_exit);
#endif
#endif //BUILD_QUICK_SHOW

MODULE_AUTHOR("Realtek.com");
MODULE_LICENSE("Dual BSD/GPL");
