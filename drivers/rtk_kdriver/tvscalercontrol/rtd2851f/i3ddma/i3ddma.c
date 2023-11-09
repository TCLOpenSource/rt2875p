/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
*/
//#include <../../../../../linux-3.10.0/arch/mips/include/asm/mach-generic/spaces.h>
#ifndef BUILD_QUICK_SHOW
#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/pageremap.h>
#include <linux/auth.h>
#include <rtk_kdriver/RPCDriver.h>
#include <uapi/linux/const.h>
#include <linux/mm.h>
#endif
#include <rtd_log/rtd_module_log.h>
#include <tvscalercontrol/i3ddma/i3ddma.h>
#include <rbus/dma_vgip_reg.h>
//#include <rbus/scaler/rbusI3DDMAReg.h>
#include <rbus/h3ddma_reg.h>
#include <rbus/vodma_reg.h>
//#include <H3DDMA_fw.h>
//#include <rbusI3DDMA_ScaleDownReg.h>
#include <rbus/h3ddma_rgb2yuv_dither_4xxto4xx_reg.h>
//#include <rbus/h3ddma_hsd_reg.h>
//#include <H3DDMA_fw.h>
#include <rbus/ppoverlay_reg.h>

#ifndef BUILD_QUICK_SHOW

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <VideoRPC_System.h>
	#include <VideoRPC_System_data.h>
#else
	#include <rtk_kdriver/rpc/VideoRPC_System.h>
	#include <rtk_kdriver/rpc/VideoRPC_System_data.h>
#endif
#endif
#include <tvscalercontrol/scaler/modeState.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/i3ddma/i3ddma_drv.h>
#include <tvscalercontrol/i3ddma/i3ddma_uzd.h>
#include <tvscalercontrol/i3ddma/i3ddmacolorfmtconvert.h>
#include <tvscalercontrol/scalerdrv/scaler_i2rnd.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/panel/panelapi.h>
#include <scaler_vscdev.h>
//#include <mach/RPCDriver.h>
#include <tvscalercontrol/hdmirx/hdmifun.h>
#include <rtk_kdriver/dprx/dprxfun.h>
#ifndef BUILD_QUICK_SHOW
#include <vgip_isr/scalerAI.h>
#if defined CONFIG_SCALER_BRING_UP
#include <rbus/hdr_all_top_reg.h>
#include <rbus/dm_reg.h>
#endif
#else
#include <no_os/printk.h>
#include <rtd_log/rtd_module_log.h>
#include <timer.h>
#include <rbus/hdr_all_top_reg.h>
#include <rbus/dm_reg.h>
#include <div64.h>
#include <malloc.h>
#endif
I3DDMA_TIMING_T       inputTiming;
I3DDMA_TIMING_T       genTiming;

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
#include <scaler/scalerDrvCommon.h>
#else
#include <scalercommon/scalerDrvCommon.h>
#endif

#undef rtd_outl
#define rtd_outl(x, y)    IoReg_Write32(x,y)
#undef rtd_inl
#define rtd_inl(x)    IoReg_Read32(x)
#undef rtd_maskl
#define rtd_maskl(x, y, z)    IoReg_Mask32(x,y,z)
#undef rtd_setbits
#define rtd_setbits(offset, Mask)    rtd_outl(offset, (rtd_inl(offset) | Mask))
#undef rtd_clearbits
#define rtd_clearbits(offset, Mask)    rtd_outl(offset, ((rtd_inl(offset) & ~(Mask))))
#ifdef BUILD_QUICK_SHOW
#define  dvr_size_alignment(x)  ((unsigned long)x)
#define  dvr_memory_alignment(x, y)  ((unsigned long)x)

#endif
I3DDMA_3DDMA_CTRL_T i3ddmaCtrl;
I3DDMA_3DDMA_CTRL_T metadataCtrl;

/*4k and 96 align*/
#define __12KPAGE  0x3000

I3DDMA_3DDMA_CTRL_T *Get_Val_i3ddmaCtrl_addr(void)
{
	return &i3ddmaCtrl;
}

void Set_Val_i3ddmaCtrl_addr_init_value(void)
{
    memset(&i3ddmaCtrl, 0, sizeof(I3DDMA_3DDMA_CTRL_T));
}

void Set_Val_i3ddmaCtrl_addr_cap_buffer_size(unsigned char ucIndex, unsigned int size)
{
    if(ucIndex < CAP_BUF_NUM_MAX)
    {
        i3ddmaCtrl.cap_buffer[ucIndex].getsize = size;
    }
}

void Set_Val_i3ddmaCtrl_addr_cap_buffer_addr(unsigned char ucIndex, unsigned int addr)
{
    if(ucIndex < CAP_BUF_NUM_MAX)
    {
        i3ddmaCtrl.cap_buffer[ucIndex].phyaddr= addr;
    }
}

#ifdef CONFIG_ENABLE_HDMI_NN
#define NN_CAP_BUFFER_NUM 3
#define NN_CAP_OUTPUT_WID_SIZE 960
#define NN_CAP_OUTPUT_LEN_SIZE 540
I3DDMA_CAPTURE_BUFFER_T g_tNN_Cap_Buffer[NN_CAP_BUFFER_NUM];
unsigned char g_ucNNReadIdx, g_ucNNWriteIdx;


//unsigned long g_cacheaddr, g_uncacheaddr;

void h3ddma_run_film_mode_enable(unsigned int enable);

typedef struct _SZIE {
	unsigned int	nWidth;
	unsigned int	nLength;
} SIZE;

unsigned int g_ulNNOutWidth=0; //960;
unsigned int g_ulNNOutLength=0; //540;

extern void I3DDMA_ultrazoom_config_scaling_down_for_NN(SIZE* in_Size, SIZE* out_Size, unsigned char panorama);


#endif

#ifdef CONFIG_RTK_KDRV_DV
extern bool check_hdmi_dolby_vision_rgb_and_full_hd(void);
#endif

#define CAP_4K120_BUFFER_NUM	3
I3DDMA_CAPTURE_BUFFER_T g_4K120_Cap_Y_Buffer[CAP_4K120_BUFFER_NUM];
I3DDMA_CAPTURE_BUFFER_T g_4K120_Cap_C_Buffer[CAP_4K120_BUFFER_NUM];
I3DDMA_CAPTURE_BUFFER_T g_4K120_SE_Buffer[CAP_4K120_BUFFER_NUM];
unsigned int cur_idma_cap_index = 0;
unsigned char i3ddma_4k120_flag = 0;
I3DDMA_CAPTURE_BUFFER_T *Get_Val_g_4k120_Se_Buffer(void)
{
    return g_4K120_SE_Buffer;
}

/*
 * This gives the physical RAM offset.
 */
#define UNCAC_BASE		_AC(0xa0000000, UL)

#define IDMA_CAP_FRAME_SIZE_SRC_2K2K_3D 	(1920 * 1080 * 4) 	// (2k2k * 1/2) 10bit 444 (4byte/pixel) -- for HDMI 1080p FP source
#define IDMA_CAP_FRAME_SIZE_SRC_2K1K_3D 	(1920 * 540 * 4) 	// (2k1k * 1/2) 10bit 444 (4byte/pixel) -- non-1080pFP 3D source

#define VODMA_IV2PV_DELAY_CAL_MARGIN 1
#define I3DDMA_DRAM_MARGIN 3
// dynamic allocate IDMA capture buffer memory from video FW
//#define I3DDMA_DYNAMIC_MEMORY_ALLOCATE_ENABLE

// allocate/release IDMA memory dynamic
#define ENABLE_I3DDMA_ALLOC_RELEASE_MEMORY_DYNAMIC

//extern int send_rpc_command(int opt, unsigned long command, unsigned long param1, unsigned long param2, unsigned long *retvalue);
//int send_rpc_command(int opt, unsigned long command, unsigned long param1, unsigned long param2, unsigned long *retvalue);
//[K3LG-282] because can't allocate mem issue
#ifdef CONFIG_VBM_HEAP
int VBM_ReturnMemChunk(unsigned long dmabuf_hndl, unsigned long buffer_addr_in, unsigned long buffer_size_in);
int VBM_BorrowMemChunk(unsigned long *dmabuf_hndl, unsigned long *buffer_addr, unsigned long *buffer_size, UINT32 reqSize, UINT32 type);
#else
extern int VBM_ReturnMemChunk(unsigned long buffer_addr, unsigned long buffer_size);
extern int VBM_BorrowMemChunk(unsigned long *buffer_addr, unsigned long *buffer_size, UINT32 reqSize , UINT32 type);
#endif
#ifdef CONFIG_HDR_SDR_SEAMLESS
I3DDMA_TIMING_T hdmi_source_info_backup;//backup hdmi source timing and color format
static DEFINE_SPINLOCK(I3DDMA_CHANGE_SPINLOCK);/*Spin lock no context switch. When we need to change i3ddma format*/
unsigned char i3ddma_change_format_flag = FALSE;//i3ddma format need to change
unsigned char target_output_color_format = I3DDMA_COLOR_UNKNOW;//target i3ddma color format
#endif
static I3DDMA_3D_T i3ddma3Dformat = I3DDMA_2D_ONLY;
static I3DDMA_3D_OPMODE_T i3ddmaOpMode = I3DDMA_3D_OPMODE_HW;

//#include <tvscalercontrol/scaler/modeState.h>
typedef struct  {

    char *name;
    int progressive;
    unsigned int h_act_len;
    unsigned int v_act_len;
    unsigned int lr_v_act_len;
    unsigned int v_active_space1;
    unsigned int v_active_space2;

}I3DDMA_ACTIVE_SPACE_TABLE_T;

I3DDMA_ACTIVE_SPACE_TABLE_T i3ddma_active_space_table[] = {
	{ "1080P FramePacking",  1, 1920,  1080*2 + 45, 1080, 45, 0 },
	{ "720P FramePacking",  1, 1280,  720*2 + 30, 720, 30, 0 },
	{ "480P FramePacking",  1, 720,  480*2 + 45, 480, 45, 0 },
	{ "576P FramePacking",  1, 720,  576*2 + 49, 576, 49, 0 },
	{ "1080I FramePacking",  0, 1920,  540*4 + 22 + 23*2, 540, 23, 22 },
	{ "576I FramePacking",  0, 1440,  288*4 + 32 + 21*2, 288, 21, 32 },
	{ "480I FramePacking",  0, 1440,  240*4 + 22 + 23*2, 240, 23, 22 },
	{ NULL,  1, 1920,  520, 40, 0 },
};


//#ifdef HDMI_HAVE_HDMI_3D_TG
#if 0
unsigned char g_ucFp1080i_status=FALSE, g_ucFp1080i_fix_vStart_status=FALSE;
void I3DDMA_Set_FP1080i_status(I3DDMA_TIMING_T *gen_timing)
{
	g_ucFp1080i_status = ((gen_timing->i3ddma_3dformat == I3DDMA_FRAME_PACKING) && (gen_timing->progressive == 0));
	return;
}

unsigned char I3DDMA_Get_FP1080i_status(void)
{
	return g_ucFp1080i_status;
}


// [FIX-ME] change v-sync start position for HDMI 1080iFP 3D to 2D mode (3->1)
void I3DDMA_Set_FP1080i_3Dcvt2D_fix_vStart_status(unsigned char mode)
{
	g_ucFp1080i_fix_vStart_status = mode;
}

unsigned char I3DDMA_Get_FP1080i_3Dcvt2D_fix_vStart_status(void)
{
	return g_ucFp1080i_fix_vStart_status;
}
#endif

VO_CHROMA_FMT VOColorMap[4] = {
	VO_CHROMA_RGB888,
	VO_CHROMA_YUV422,
	VO_CHROMA_YUV444,
	VO_CHROMA_YUV411,

};

#if 0
char I3DDMA_ResetVODMA4K2K(void)
{

	VIDEO_RPC_VOUT_CONFIG_HDMI_3D config;
	CLNT_STRUCT clnt;

	rtd_pr_i3ddma_info("%s\n", __FUNCTION__);
	clnt = prepareCLNT(BLOCK_MODE | USE_INTR_BUF | SEND_VIDEO_CPU, VIDEO_SYSTEM, VERSION);

	memset(&config, 0, sizeof(VIDEO_RPC_VOUT_CONFIG_HDMI_3D));
	config.width = 0;
	config.height = 0;
	config.videoPlane = VO_VIDEO_PLANE_V1;
	HRESULT* hr = VIDEO_RPC_VOUT_ToAgent_ConfigHDMI_3D_0(&config, &clnt);
	if (hr < 0) {
		rtd_pr_i3ddma_info("CONFIG HDMI 3D RPC fail\n");
		return FALSE;
	}
    free(hr);

	return TRUE;

}

#endif


unsigned char I3DDMA_Get3DAllocReleaseMemoryDynamic(void)
{
#ifdef ENABLE_I3DDMA_ALLOC_RELEASE_MEMORY_DYNAMIC
	return TRUE;
#endif
	return FALSE;
}

char I3DDMA_ForceVODMA2D(UINT8 enable, UINT8 LR) {
#ifndef BUILD_QUICK_SHOW

	VIDEO_RPC_VOUT_FORCE_HDMI_2D *config;
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
#ifndef CONFIG_SCALER_BRING_UP
	unsigned long ret;
#endif
    unsigned int  phy_addr;
#endif
	unsigned long  vir_addr, vir_addr_noncache = 0;
	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_VOUT_FORCE_HDMI_2D), GFP_DCU1, (void*)&vir_addr_noncache);
	//phy_addr = (unsigned int)virt_to_phys((void*)vir_addr);
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	phy_addr = (unsigned int)dvr_to_phys((void*)vir_addr);
#endif

	config = (VIDEO_RPC_VOUT_FORCE_HDMI_2D *)vir_addr_noncache;
	config->force2d = enable;
	config->l_flag = LR;
	config->videoPlane = VO_VIDEO_PLANE_V1;
	config->videoPlane = htonl(config->videoPlane);
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
#ifndef CONFIG_SCALER_BRING_UP
	if (send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_ToAgent_Force_HDMI_2D, phy_addr, 0, &ret))
		rtd_pr_i3ddma_info("RPC fail!!\n");
#endif
#endif
	dvr_free((void *)vir_addr);
#endif

	return TRUE;
}

void I3DDMA_SetupLRSeparator(I3DDMA_TIMING_T *tx_timing, I3DDMA_TIMING_T *gen_timing) {

	int genvs_startpos_pixels;
	int genvs_endpos_pixels;
//	unsigned int h_act_len;

	h3ddma_i3ddma_enable_RBUS 	i3ddma_i3ddma_enable_reg;
	h3ddma_lr_separate_ctrl1_RBUS   i3ddma_lr_separate_ctrl1_reg;
	h3ddma_lr_separate_ctrl2_RBUS   i3ddma_lr_separate_ctrl2_reg;
	h3ddma_lr_separate_insert_vs_start_RBUS i3ddma_lr_separate_insert_vs_start_reg;
	h3ddma_lr_separate_insert_vs_end_RBUS  i3ddma_lr_separate_insert_vs_end_reg;
	h3ddma_cap1_cti_dma_wr_ctrl_RBUS i3ddma_byte_channel_swap_reg;
	h3ddma_i3ddma_ctrl_0_RBUS h3ddma_i3ddma_ctrl_0_reg;
#ifdef CONFIG_ENABLE_HDMI_NN
	h3ddma_cap2_cti_dma_wr_ctrl_RBUS i3ddma_byte_channe2_swap_reg;
#endif
	rtd_pr_i3ddma_info("!!!!!!!!!!!! %s\n", __FUNCTION__);

/*
	// update FP 1080i status
	I3DDMA_Set_FP1080i_status(gen_timing);
	I3DDMA_Set_FP1080i_3Dcvt2D_fix_vStart_status(FALSE);
*/

	i3ddma_i3ddma_enable_reg.regValue = rtd_inl(H3DDMA_I3DDMA_enable_reg);
	i3ddma_i3ddma_enable_reg.lr_separate_en = 0;
	rtd_outl(H3DDMA_I3DDMA_enable_reg, i3ddma_i3ddma_enable_reg.regValue);


#if 0 // [To Be Confirmed] Input HS/VS Signal Polarity Invert setup move to Main VGIP
	//Fix me : benwang 20121128
	//rtd_maskl(GET_VGIP_CHNx_CTRL_VADDR(), ~(VGIP_CHN1_CTRL_ch1_vs_inv_mask | VGIP_CHN1_CTRL_ch1_hs_inv_mask ), 0); // Let HS negtive polarity to positive polarity
	rtd_maskl(VGIP_CHN1_CTRL_reg, ~(VGIP_CHN1_CTRL_ch1_vs_inv_mask | VGIP_CHN1_CTRL_ch1_hs_inv_mask ), 0); // Let HS negtive polarity to positive polarity

	switch(tx_timing->polarity) {

		case _SYNC_HP_VP:
			rtd_maskl(I3DDMA_LR_SEPARATE_CTRL1_VADDR, ~(I3DDMA_LR_SEPARATE_CTRL1_vs_inv_mask | I3DDMA_LR_SEPARATE_CTRL1_hs_inv_mask), 0);
			break;
		case _SYNC_HN_VP:
			rtd_maskl(I3DDMA_LR_SEPARATE_CTRL1_VADDR, ~(I3DDMA_LR_SEPARATE_CTRL1_vs_inv_mask | I3DDMA_LR_SEPARATE_CTRL1_hs_inv_mask),
								I3DDMA_LR_SEPARATE_CTRL1_vs_inv(0) | I3DDMA_LR_SEPARATE_CTRL1_hs_inv(1));
			break;
		case _SYNC_HP_VN:
			rtd_maskl(I3DDMA_LR_SEPARATE_CTRL1_VADDR, ~(I3DDMA_LR_SEPARATE_CTRL1_vs_inv_mask | I3DDMA_LR_SEPARATE_CTRL1_hs_inv_mask),
				I3DDMA_LR_SEPARATE_CTRL1_vs_inv(1) | I3DDMA_LR_SEPARATE_CTRL1_hs_inv(0));

			break;
		case _SYNC_HN_VN:
			rtd_maskl(I3DDMA_LR_SEPARATE_CTRL1_VADDR, ~(I3DDMA_LR_SEPARATE_CTRL1_vs_inv_mask | I3DDMA_LR_SEPARATE_CTRL1_hs_inv_mask),
				I3DDMA_LR_SEPARATE_CTRL1_vs_inv(1) | I3DDMA_LR_SEPARATE_CTRL1_hs_inv(1));
			break;
	}
#endif

	// Setup LR Separate for :Frame packing and TopBottom, Line alternative
	// [HDMI-DMA] HDMI DMA capture control for 4k2k 2D timing
	i3ddma_lr_separate_ctrl1_reg.regValue = rtd_inl(H3DDMA_LR_Separate_CTRL1_reg);
	if((tx_timing->i3ddma_3dformat == I3DDMA_2D_ONLY) && (gen_timing->i3ddma_3dformat == I3DDMA_2D_ONLY)){
		i3ddma_lr_separate_ctrl1_reg.hdmi_3d_structure = 0xa;
	}else if(tx_timing->i3ddma_3dformat == I3DDMA_VERTICAL_STRIPE){
		i3ddma_lr_separate_ctrl1_reg.hdmi_3d_structure = 0xf;
	}else if(tx_timing->i3ddma_3dformat == I3DDMA_CHECKER_BOARD){
		i3ddma_lr_separate_ctrl1_reg.hdmi_3d_structure = 0xf;
	}else if((tx_timing->i3ddma_3dformat == I3DDMA_SENSIO)|| (tx_timing->i3ddma_3dformat == I3DDMA_REALID)){
		i3ddma_lr_separate_ctrl1_reg.hdmi_3d_structure = I3DDMA_SIDE_BY_SIDE_HALF;
	}else if(tx_timing->i3ddma_3dformat == I3DDMA_FRAMESEQUENCE){
		i3ddma_lr_separate_ctrl1_reg.hdmi_3d_structure = I3DDMA_FRAME_PACKING;
	}
	else
		i3ddma_lr_separate_ctrl1_reg.hdmi_3d_structure = gen_timing->i3ddma_3dformat;


	if(gen_timing->i3ddma_3dformat == I3DDMA_FRAMESEQUENCE){	// Framesequence use progressive LR separator setting, use to 3D type set Framepacking
		i3ddma_lr_separate_ctrl1_reg.progressive = 1;

	}else{
		i3ddma_lr_separate_ctrl1_reg.progressive = gen_timing->progressive;
	}

	rtd_pr_i3ddma_debug("[IDMA] Fmt/Ctrl=%d/0x%x\n", tx_timing->i3ddma_3dformat, rtd_inl(H3DDMA_LR_Separate_CTRL1_reg));


	i3ddma_lr_separate_ctrl1_reg.fp_vact_space1 = gen_timing->v_active_space1;
	i3ddma_lr_separate_ctrl1_reg.fp_vact_space2 = gen_timing->v_active_space2;
	rtd_outl(H3DDMA_LR_Separate_CTRL1_reg, i3ddma_lr_separate_ctrl1_reg.regValue);

	i3ddma_lr_separate_ctrl2_reg.regValue = rtd_inl(H3DDMA_LR_Separate_CTRL2_reg);
	if ((gen_timing->i3ddma_3dformat == I3DDMA_SIDE_BY_SIDE_HALF) || (gen_timing->i3ddma_3dformat == I3DDMA_SENSIO)|| (gen_timing->i3ddma_3dformat == I3DDMA_REALID)) {
			i3ddma_lr_separate_ctrl2_reg.vact = gen_timing->v_act_len;
			i3ddma_lr_separate_ctrl2_reg.hact = gen_timing->h_act_len*2;
	} else if (gen_timing->i3ddma_3dformat == I3DDMA_TOP_AND_BOTTOM) {
			i3ddma_lr_separate_ctrl2_reg.vact = gen_timing->v_act_len*2;
			i3ddma_lr_separate_ctrl2_reg.hact = gen_timing->h_act_len;
	} else if(gen_timing->i3ddma_3dformat == I3DDMA_VERTICAL_STRIPE) {
			i3ddma_lr_separate_ctrl2_reg.vact = gen_timing->v_act_len;
			i3ddma_lr_separate_ctrl2_reg.hact = gen_timing->h_act_len*2;
	}else if(gen_timing->i3ddma_3dformat == I3DDMA_CHECKER_BOARD) {
			i3ddma_lr_separate_ctrl2_reg.vact = gen_timing->v_act_len;
			i3ddma_lr_separate_ctrl2_reg.hact = gen_timing->h_act_len*2;
	}
	else {
			i3ddma_lr_separate_ctrl2_reg.hact = gen_timing->h_act_len;
			i3ddma_lr_separate_ctrl2_reg.vact = gen_timing->v_act_len;
	}
	rtd_outl(H3DDMA_LR_Separate_CTRL2_reg, i3ddma_lr_separate_ctrl2_reg.regValue);

	if (gen_timing->i3ddma_3dformat == I3DDMA_FRAME_PACKING && gen_timing->progressive == 0) {
//          FIXME:
		genvs_startpos_pixels = (tx_timing->v_total * tx_timing->h_total + 1) / 2;
		genvs_startpos_pixels -= ((gen_timing->v_act_sta + gen_timing->v_active_space1 + gen_timing->v_act_len * 2) * gen_timing->h_total);
		genvs_endpos_pixels = genvs_startpos_pixels + tx_timing->h_total * 2;
/*
		// [FIX-ME]: 1080i60 FP 3Dto2D unknown 1 lines diff (3->1)
		if((i3ddmaCtrl.force_3dto2d_enable == TRUE) && (genvs_startpos_pixels > (2*tx_timing->h_total))){
			rtd_pr_i3ddma_info("[FP1080i] V.start=%d->%d\n", genvs_startpos_pixels/tx_timing->h_total, (genvs_startpos_pixels-(2*tx_timing->h_total))/tx_timing->h_total);
			genvs_startpos_pixels -= (2*tx_timing->h_total);
			I3DDMA_Set_FP1080i_3Dcvt2D_fix_vStart_status(TRUE);
		}
*/
		i3ddma_lr_separate_insert_vs_start_reg.regValue = rtd_inl(H3DDMA_LR_Separate_insert_vs_start_reg);
		i3ddma_lr_separate_insert_vs_start_reg.vstart = genvs_startpos_pixels/tx_timing->h_total;
		i3ddma_lr_separate_insert_vs_start_reg.hstart = genvs_startpos_pixels%tx_timing->h_total;
		rtd_outl(H3DDMA_LR_Separate_insert_vs_start_reg, i3ddma_lr_separate_insert_vs_start_reg.regValue);

		i3ddma_lr_separate_insert_vs_end_reg.regValue = rtd_inl(H3DDMA_LR_Separate_insert_vs_end_reg);
		i3ddma_lr_separate_insert_vs_end_reg.vend = genvs_endpos_pixels/tx_timing->h_total;
		i3ddma_lr_separate_insert_vs_end_reg.hend = genvs_endpos_pixels%tx_timing->h_total;
		rtd_outl(H3DDMA_LR_Separate_insert_vs_end_reg, i3ddma_lr_separate_insert_vs_end_reg.regValue);
	}
	//rtd_maskl(I3DDMA_LR_SEPARATE_CTRL1_VADDR, ~(I3DDMA_LR_SEPARATE_CTRL1_lr_separate_en_mask),
					//I3DDMA_LR_SEPARATE_CTRL1_lr_separate_en(1));

	i3ddma_byte_channel_swap_reg.regValue = rtd_inl(H3DDMA_CAP1_CTI_DMA_WR_Ctrl_reg);
#ifdef CONFIG_ENABLE_HDMI_NN
	i3ddma_byte_channel_swap_reg.cap1_dma_1byte_swap = 1;
	i3ddma_byte_channel_swap_reg.cap1_dma_2byte_swap = 1;
	i3ddma_byte_channel_swap_reg.cap1_dma_4byte_swap = 1;
	i3ddma_byte_channel_swap_reg.cap1_dma_8byte_swap = 1;
#else
	i3ddma_byte_channel_swap_reg.cap1_dma_1byte_swap = 0;
	i3ddma_byte_channel_swap_reg.cap1_dma_2byte_swap = 0;
	i3ddma_byte_channel_swap_reg.cap1_dma_4byte_swap = 0;
	i3ddma_byte_channel_swap_reg.cap1_dma_8byte_swap = 0;
#endif
	rtd_outl(H3DDMA_CAP1_CTI_DMA_WR_Ctrl_reg, i3ddma_byte_channel_swap_reg.regValue);

//lsy
#ifdef CONFIG_ENABLE_HDMI_NN
	i3ddma_byte_channe2_swap_reg.regValue = rtd_inl(H3DDMA_CAP2_CTI_DMA_WR_Ctrl_reg);
	i3ddma_byte_channe2_swap_reg.cap2_dma_1byte_swap = 1;
	i3ddma_byte_channe2_swap_reg.cap2_dma_2byte_swap = 1;
	i3ddma_byte_channe2_swap_reg.cap2_dma_4byte_swap = 1;
	i3ddma_byte_channe2_swap_reg.cap2_dma_8byte_swap = 1;
	rtd_outl(H3DDMA_CAP2_CTI_DMA_WR_Ctrl_reg, i3ddma_byte_channe2_swap_reg.regValue);
#endif


	h3ddma_i3ddma_ctrl_0_reg.regValue= rtd_inl(H3DDMA_I3DDMA_ctrl_0_reg);
	h3ddma_i3ddma_ctrl_0_reg.cap0_channel_swap=0;
	rtd_outl(H3DDMA_I3DDMA_ctrl_0_reg, h3ddma_i3ddma_ctrl_0_reg.regValue);

	i3ddma_i3ddma_enable_reg.regValue = rtd_inl(H3DDMA_I3DDMA_enable_reg);
	i3ddma_i3ddma_enable_reg.lr_separate_en = 1;
	i3ddma_i3ddma_enable_reg.field_sample_by_vs = 0; //dic's suggest always set 0, sync with field

	//if dma vgip fake_porch_en=1 && fake_porch_sel=1(gen vsync by den failing), enable vodma gen vsync by first den rising
	if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_DP)
		&& (vbe_disp_get_freesync_mode_flag() || vbe_disp_get_VRR_timingMode_flag())
		&& (Get_Val_DP_Vsync_Regen_Pos() == DP_REGEN_VSYNC_AT_DMA_VGIP))
		i3ddma_i3ddma_enable_reg.i3tovo_vs_sel = 1;
	else
		i3ddma_i3ddma_enable_reg.i3tovo_vs_sel = 0;
	rtd_outl(H3DDMA_I3DDMA_enable_reg, i3ddma_i3ddma_enable_reg.regValue);

}

void I3DDMA_reset_i3tovo_vs_sel(void)
{
	h3ddma_i3ddma_enable_RBUS i3ddma_i3ddma_enable_reg;

	i3ddma_i3ddma_enable_reg.regValue = rtd_inl(H3DDMA_I3DDMA_enable_reg);
	i3ddma_i3ddma_enable_reg.i3tovo_vs_sel = 0;
	rtd_outl(H3DDMA_I3DDMA_enable_reg, i3ddma_i3ddma_enable_reg.regValue);
}

I3DDMA_ERR_T I3DDMA_MeasureActiveSpace(I3DDMA_TIMING_T *tx_timing, I3DDMA_TIMING_T *gen_timing) {

	int i = 0;
	int fields;

	//rtd_pr_i3ddma_debug("tx_timing->h_act_len = %d\n", tx_timing->h_act_len);
	//rtd_pr_i3ddma_debug("tx_timing->v_act_len = %d\n", tx_timing->v_act_len);


	while(i3ddma_active_space_table[i].name) {
			//rtd_pr_i3ddma_debug(" hdmi_active_space_table[%d].h_act_len = %d\n", i, i3ddma_active_space_table[i].h_act_len);
			//rtd_pr_i3ddma_debug(" hdmi_active_space_table[%d].v_act_len = %d\n", i, i3ddma_active_space_table[i].v_act_len);

		if (/*tx_timing->h_act_len == hdmi_active_space_table[i].h_act_len &&*/ tx_timing->v_act_len == i3ddma_active_space_table[i].v_act_len) {

			gen_timing->v_act_len = i3ddma_active_space_table[i].lr_v_act_len;
			tx_timing->v_active_space1 = i3ddma_active_space_table[i].v_active_space1;
			tx_timing->v_active_space2 = i3ddma_active_space_table[i].v_active_space2;
			gen_timing->v_active_space1 = i3ddma_active_space_table[i].v_active_space1;
			gen_timing->v_active_space2 = i3ddma_active_space_table[i].v_active_space2;
			gen_timing->progressive = i3ddma_active_space_table[i].progressive;
			fields = gen_timing->progressive ? 2 : 4;
		//	gen_timing->progressive = tx_timing->progressive;
			gen_timing->color = tx_timing->color;
			gen_timing->depth = tx_timing->depth;
			gen_timing->v_freq = (gen_timing->progressive? (drvif_scaler3d_decide_3d_SG_data_FRC()? tx_timing->v_freq * 2: 1200): tx_timing->v_freq * fields);
			gen_timing->v_total = tx_timing->v_total / fields;
			gen_timing->h_freq = tx_timing->h_freq;
			gen_timing->h_act_len = tx_timing->h_act_len;
			gen_timing->h_total = tx_timing->h_total;
			gen_timing->h_act_sta = tx_timing->h_act_sta;
			gen_timing->v_act_sta = tx_timing->v_act_sta;	 // no used
			rtd_pr_i3ddma_debug("%s \n", i3ddma_active_space_table[i].name);
			return I3DDMA_ERR_NO;
		}
		i++;
	}

	gen_timing->progressive = 1;
	fields = gen_timing->progressive ? 2 : 4;
//	gen_timing->progressive = tx_timing->progressive;
	gen_timing->color = tx_timing->color;
	gen_timing->depth = tx_timing->depth;

	gen_timing->v_freq = (gen_timing->progressive? 1200: tx_timing->v_freq * fields);
	gen_timing->v_total = tx_timing->v_total / fields;
	gen_timing->h_freq = tx_timing->h_freq;
	gen_timing->h_act_len = tx_timing->h_act_len;
	gen_timing->h_total = tx_timing->h_total;
	gen_timing->h_act_sta = tx_timing->h_act_sta;
	gen_timing->v_act_sta = tx_timing->v_act_sta;	 // no used
	gen_timing->v_act_len = (tx_timing->v_act_len - tx_timing->v_act_sta) / 2;
	gen_timing->v_active_space1 = tx_timing->v_act_sta;
	gen_timing->v_active_space2 = 0;
	tx_timing->v_active_space1 = tx_timing->v_act_sta;
	tx_timing->v_active_space2 = 0;

	rtd_pr_i3ddma_debug("Unknow frame packing fomat and force to 2D\n");

	return I3DDMA_EER_MEASURE_ACTIVESPACE_FAIL;

}


void I3DDMA_Setup3Dtg(I3DDMA_TIMING_T *timing,  char frame_rate_x2) {
#if 0
	int fields, i;
	int genvs_start_pixels, genvs_end_pixels;
	int genvs_width, genvs_period;

	rtd_pr_i3ddma_info("!!!!!!!!!!!! %s\n", __FUNCTION__);

	frame_rate_x2 = 1;

	if (frame_rate_x2){
		fields = 8;
		genvs_width = 3;
		genvs_period = timing->v_total / 2;
	}else if(I3DDMA_Get_FP1080i_status() && i3ddmaCtrl.force_3dto2d_enable){
		fields = 2;
		genvs_width = 3;
		rtd_pr_i3ddma_info("[ I3DDMA] FP1080i 3Dto2D mode\n");
	} else {
//		fields = 4;
		fields = 2;
		genvs_width = 3;
		genvs_period = timing->v_total / 2;
	}

	rtd_pr_i3ddma_info("[HDMI-3DTG] FR*2=%d, field/period[%d/%d]\n", frame_rate_x2, fields, genvs_period);

	if(I3DDMA_Get_FP1080i_3Dcvt2D_fix_vStart_status()){
		// Ignore 3DTG v-sync insertion in HDMI 1080i FP video 3D to 2D mode (set vStart/vEnd=0xfff)
		genvs_start_pixels = (timing->h_total << 12)-timing->h_total;
		genvs_end_pixels = genvs_start_pixels;
	}
	// [HDMI-DMA] HDMI DMA capture timing for 4k2k 2D video
	else if(timing->i3ddma_3dformat == I3DDMA_2D_ONLY){
		// Ignore 3DTG v-sync insertion in 4k2k 2D mode (set vStart/vEnd=0xfff)
		genvs_start_pixels = 0; //(timing->h_total << 12)-timing->h_total;
		genvs_end_pixels = 0; //genvs_start_pixels;
		genvs_period = 0;
	}
	else{
		rtd_pr_i3ddma_info("[DBG] FP1080i/3Dto2D=%d/%d\n", I3DDMA_Get_FP1080i_status(), hdmi.force_3dto2d_enable);
		genvs_start_pixels = (timing->v_total * timing->h_total  + (fields/2))/fields;
		genvs_end_pixels = genvs_start_pixels + 3 * timing->h_total;
	}

	rtd_pr_i3ddma_info("[ I3DDMA-3DTG] start pixel/end pixel [%d /%d], vtotal/htotal [%d/%d]\n", genvs_start_pixels, genvs_end_pixels, timing->v_total, timing->h_total);


	// Timing Gen setting
	rtd_maskl(I3DDMA_TG_V1_END_reg, ~(I3DDMA_TG_V1_END_tg_vend1_mask | I3DDMA_TG_V1_END_tg_hend1_mask),
					I3DDMA_TG_V1_END_tg_vend1(3) | I3DDMA_TG_V1_END_tg_hend1(0));

	// FP 1080i 3Dto2D: field == 2
	if((fields == 2) && I3DDMA_Get_FP1080i_status() && i3ddmaCtrl.force_3dto2d_enable){
		rtd_maskl(I3DDMA_TG_V2_START_reg, ~(I3DDMA_TG_V2_START_tg_vstart2_mask | I3DDMA_TG_V2_START_tg_hstart2_mask) ,
						(I3DDMA_TG_V2_START_tg_vstart2(genvs_period) | I3DDMA_TG_V2_START_tg_hstart2(genvs_start_pixels%timing->h_total)));
		rtd_maskl(I3DDMA_TG_V2_END_reg, ~(I3DDMA_TG_V2_END_tg_vend2_mask | I3DDMA_TG_V2_END_tg_hend2_mask) ,
					(I3DDMA_TG_V2_END_tg_vend2(genvs_end_pixels/timing->h_total) | I3DDMA_TG_V2_END_tg_hend2(genvs_end_pixels%timing->h_total)));
	}
	else if(fields == 2){
		rtd_maskl(I3DDMA_TG_V2_START_reg, ~(I3DDMA_TG_V2_START_tg_vstart2_mask | I3DDMA_TG_V2_START_tg_hstart2_mask) ,
						(I3DDMA_TG_V2_START_tg_vstart2(genvs_period) | I3DDMA_TG_V2_START_tg_hstart2(genvs_start_pixels%timing->h_total)));
		rtd_maskl(I3DDMA_TG_V2_END_reg, ~(I3DDMA_TG_V2_END_tg_vend2_mask | I3DDMA_TG_V2_END_tg_hend2_mask) ,
					(I3DDMA_TG_V2_END_tg_vend2(genvs_end_pixels/timing->h_total) | I3DDMA_TG_V2_END_tg_hend2(genvs_end_pixels%timing->h_total)));
	}
	else{
		for (i=0; i<(fields/2-1); i++) {
	//		rtd_maskl(I3DDMA_TG_V2_START_reg + 8 * i, ~(I3DDMA_TG_V2_START_tg_vstart2_mask | I3DDMA_TG_V2_START_tg_hstart2_mask) ,
	//						(I3DDMA_TG_V2_START_tg_vstart2(genvs_start / timing->h_total) | I3DDMA_TG_V2_START_tg_hstart2(genvs_start % timing->h_total)));
	//		rtd_maskl(I3DDMA_TG_V2_END_reg+ 8 * i, ~(I3DDMA_TG_V2_END_tg_vend2_mask | I3DDMA_TG_V2_END_tg_hend2_mask) ,
	//					(I3DDMA_TG_V2_END_tg_vend2(genvs_end / timing->h_total) | I3DDMA_TG_V2_END_tg_hend2(genvs_end % timing->h_total)));
	//#ifdef ENABLE_HDMI_3DDMA_TEST
	#if 0
			rtd_maskl(I3DDMA_TG_V2_START_reg + 8 * i, ~(I3DDMA_TG_V2_START_tg_vstart2_mask | I3DDMA_TG_V2_START_tg_hstart2_mask) ,
							(I3DDMA_TG_V2_START_tg_vstart2(genvs_period) | I3DDMA_TG_V2_START_tg_hstart2(genvs_start_pixels%timing->h_total)));
			rtd_maskl(I3DDMA_TG_V2_END_reg+ 8 * i, ~(I3DDMA_TG_V2_END_tg_vend2_mask | I3DDMA_TG_V2_END_tg_hend2_mask) ,
						(I3DDMA_TG_V2_END_tg_vend2(genvs_period+genvs_width) | I3DDMA_TG_V2_END_tg_hend2(genvs_end_pixels%timing->h_total)));
	#else
			rtd_maskl(I3DDMA_TG_V2_START_reg + 8 * i, ~(I3DDMA_TG_V2_START_tg_vstart2_mask | I3DDMA_TG_V2_START_tg_hstart2_mask) ,
							(I3DDMA_TG_V2_START_tg_vstart2(genvs_start_pixels/timing->h_total) | I3DDMA_TG_V2_START_tg_hstart2(genvs_start_pixels%timing->h_total)));
			rtd_maskl(I3DDMA_TG_V2_END_reg+ 8 * i, ~(I3DDMA_TG_V2_END_tg_vend2_mask | I3DDMA_TG_V2_END_tg_hend2_mask) ,
						(I3DDMA_TG_V2_END_tg_vend2(genvs_end_pixels/timing->h_total) | I3DDMA_TG_V2_END_tg_hend2(genvs_end_pixels%timing->h_total)));
	#endif
		}
	}

	if((timing->i3ddma_3dformat != I3DDMA_2D_ONLY)){
		rtd_maskl(I3DDMA_LR_SEPARATE_CTRL1_VADDR, ~(I3DDMA_LR_SEPARATE_CTRL1_hdmi_3d_sel_mask),
		I3DDMA_LR_SEPARATE_CTRL1_hdmi_3d_sel(1));
	}

	// enable double frame rate
	//if(I3DDMA_Get_FP1080i_status() == TRUE){
		rtd_maskl(I3DDMA_TG_V1_END_reg, ~(I3DDMA_TG_V1_END_pulldown_en_mask),
						I3DDMA_TG_V1_END_pulldown_en(frame_rate_x2!=0));
	//}

	// timing gen enable
	rtd_maskl(I3DDMA_TG_V1_END_reg, ~(I3DDMA_TG_V1_END_tg_en_mask),
					I3DDMA_TG_V1_END_tg_en(1));

#else

	//int fields;
	//int i;
	//int genvs_start_pixels, genvs_end_pixels;
	//int genvs_width, genvs_period;
	//int start_pixels_tmp;

	h3ddma_tg_v1_end_RBUS i3ddma_tg_v1_end_reg;
	//h3ddma_tg_v2_start_RBUS i3ddma_tg_v2_start_reg;
	//h3ddma_tg_v2_end_RBUS i3ddma_tg_v2_end_reg;
//	h3ddma_lr_separate_ctrl1_RBUS   i3ddma_lr_separate_ctrl1_reg;
	h3ddma_i3ddma_enable_RBUS 	i3ddma_i3ddma_enable_reg;
	vodma_vodma_i2rnd_fifo_th_RBUS vodma_vodma_i2rnd_fifo_th_Reg;
	vodma_vodma_i2rnd_dma_info_RBUS vodma_vodma_i2rnd_dma_info_Reg;
#if 0
	rtd_pr_i3ddma_debug("!!!!!!!!!!!! 2 %s\n", __FUNCTION__);

	if(frame_rate_x2){
		fields = 8;
		genvs_width = 3;
		genvs_period = timing->v_total / 2;
	}else if((timing->progressive == 0) && (timing->i3ddma_3dformat != I3DDMA_2D_ONLY)
			&& (timing->i3ddma_3dformat != I3DDMA_FRAME_PACKING)){
		fields = 4;
		genvs_width = 3;
	} else{
		fields = 2;
		genvs_width = 3;
		genvs_period = timing->v_total / 2;
	}

	if(timing->i3ddma_3dformat == I3DDMA_2D_ONLY){
		genvs_start_pixels = 0; //(timing->h_total << 12)-timing->h_total;
		genvs_end_pixels = 0; //genvs_start_pixels;
		genvs_period = 0;
	}
	else{
		genvs_start_pixels = (timing->v_total * timing->h_total); //(timing->v_total * timing->h_total  + (fields/2))/fields;
		genvs_end_pixels = genvs_start_pixels + 3 * timing->h_total;
	}

	rtd_pr_i3ddma_debug("[ I3DDMA-3DTG] start pixel/end pixel [%d /%d], vtotal/htotal [%d/%d]\n", genvs_start_pixels, genvs_end_pixels, timing->v_total, timing->h_total);
	rtd_pr_i3ddma_debug("[ I3DDMA-3DTG] progressive / fields / 3d format  [%d/%d/%d]\n", timing->progressive, fields, timing->i3ddma_3dformat);

#endif
	// Timing Gen setting
	i3ddma_tg_v1_end_reg.regValue = rtd_inl(H3DDMA_TG_v1_end_reg);
	i3ddma_tg_v1_end_reg.tg_vend1 = 3;
	i3ddma_tg_v1_end_reg.tg_hend1 = 0;
	rtd_outl(H3DDMA_TG_v1_end_reg, i3ddma_tg_v1_end_reg.regValue);

#if 0 //@jiawei_tang for k6 compile error

	if(fields == 2){
		i3ddma_tg_v2_start_reg.regValue = rtd_inl(H3DDMA_TG_v2_start_reg);
		i3ddma_tg_v2_start_reg.tg_vstart2 = genvs_start_pixels/timing->h_total;
		i3ddma_tg_v2_start_reg.tg_hstart2 = genvs_start_pixels%timing->h_total;
		rtd_outl(H3DDMA_TG_v2_start_reg, i3ddma_tg_v2_start_reg.regValue);

		i3ddma_tg_v2_end_reg.regValue = rtd_inl(H3DDMA_TG_v2_end_reg);
		i3ddma_tg_v2_end_reg.tg_vend2 = genvs_end_pixels/timing->h_total;
		i3ddma_tg_v2_end_reg.tg_hend2 = genvs_end_pixels%timing->h_total;
		rtd_outl(H3DDMA_TG_v2_end_reg, i3ddma_tg_v2_end_reg.regValue);
	}
	else if(fields == 4){	//interlace mode

		start_pixels_tmp = genvs_start_pixels;

		for (i=0; i<(fields/2+1); i++) {

			genvs_start_pixels = (i+1) * start_pixels_tmp;
			genvs_end_pixels = genvs_start_pixels + 3 * timing->h_total;


			i3ddma_tg_v2_start_reg.regValue = rtd_inl(H3DDMA_TG_v2_start_reg + 8*i);
			i3ddma_tg_v2_start_reg.tg_vstart2 = genvs_start_pixels/timing->h_total;
			i3ddma_tg_v2_start_reg.tg_hstart2 = genvs_start_pixels%timing->h_total;
			rtd_outl(H3DDMA_TG_v2_start_reg + 8*i, i3ddma_tg_v2_start_reg.regValue);

			i3ddma_tg_v2_end_reg.regValue = rtd_inl(H3DDMA_TG_v2_end_reg + 8*i);
			i3ddma_tg_v2_end_reg.tg_vend2 = genvs_end_pixels/timing->h_total;
			i3ddma_tg_v2_end_reg.tg_hend2 = genvs_end_pixels%timing->h_total;
			rtd_outl(H3DDMA_TG_v2_end_reg + 8*i, i3ddma_tg_v2_end_reg.regValue);
		}
	}

	if((timing->i3ddma_3dformat != I3DDMA_2D_ONLY)){
		i3ddma_lr_separate_ctrl1_reg.regValue = rtd_inl(H3DDMA_LR_Separate_CTRL1_reg);
		i3ddma_lr_separate_ctrl1_reg.hdmi_3d_sel = 1;
		rtd_outl(H3DDMA_LR_Separate_CTRL1_reg, i3ddma_lr_separate_ctrl1_reg.regValue);

	}
#endif
	// enable double frame rate
	i3ddma_tg_v1_end_reg.regValue = rtd_inl(H3DDMA_TG_v1_end_reg);
	i3ddma_tg_v1_end_reg.pullup_en = frame_rate_x2!=0;
	rtd_outl(H3DDMA_TG_v1_end_reg, i3ddma_tg_v1_end_reg.regValue);

	// timing gen enable
//	rtd_maskl(I3DDMA_TG_V1_END_VADDR, ~(I3DDMA_TG_V1_END_tg_en_mask),	I3DDMA_TG_V1_END_tg_en(1));


	vodma_vodma_i2rnd_fifo_th_Reg.regValue = IoReg_Read32(VODMA_vodma_i2rnd_fifo_th_reg);
	vodma_vodma_i2rnd_dma_info_Reg.regValue = 0;

	vodma_vodma_i2rnd_fifo_th_Reg.dma_info_th_dispm = 2;
	vodma_vodma_i2rnd_fifo_th_Reg.dma_info_th_i3ddma = 2;

	vodma_vodma_i2rnd_dma_info_Reg.dma_info_rptr_fw_set_dispm = 1;
	vodma_vodma_i2rnd_dma_info_Reg.dma_info_rptr_fw_set_i3ddma = 1;
	vodma_vodma_i2rnd_dma_info_Reg.dma_info_wptr_fw_set_dispm = 1;
	vodma_vodma_i2rnd_dma_info_Reg.dma_info_wptr_fw_set_i3ddma = 1;

	IoReg_Write32(VODMA_vodma_i2rnd_fifo_th_reg, vodma_vodma_i2rnd_fifo_th_Reg.regValue);
	IoReg_Write32(VODMA_vodma_i2rnd_dma_info_reg, vodma_vodma_i2rnd_dma_info_Reg.regValue);


	i3ddma_i3ddma_enable_reg.regValue = rtd_inl(H3DDMA_I3DDMA_enable_reg);
	i3ddma_i3ddma_enable_reg.tg_en = 1;
	rtd_outl(H3DDMA_I3DDMA_enable_reg, i3ddma_i3ddma_enable_reg.regValue);


#endif

}

//#endif  //#endif HDMI_HAVE_HDMI_3D_TG

//#ifdef HDMI_HAVE_HDMI_DMA
unsigned int DEPTH2BITS[4] = {
	8,
	10,
	10,
	10,
};

unsigned int components[4] = {
	12,
	8,
	12,
	6,
};

unsigned char I3DDMA_check_idma_shareMemWithVideoFW(void)
{
#ifdef I3DDMA_DYNAMIC_MEMORY_ALLOCATE_ENABLE
	return TRUE;
#else
	return FALSE;
#endif
}


#ifdef I3DDMA_DYNAMIC_MEMORY_ALLOCATE_ENABLE
unsigned int I3DDMA_check_idma_memory_space(I3DDMA_TIMING_T *gen_timing)
{
	unsigned int	IDMA_rpcAllocStartAddr=0, IDMA_rpcAllocSize=0;
	unsigned int capFramesize, i;

	if(gen_timing){
		if((drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_SRC_TYPE) == _SRC_HDMI) /*&& (gen_timing->i3ddma_3dformat == I3DDMA_FRAME_PACKING)*/
			&& (gen_timing->h_act_len * gen_timing->v_act_len > (1920*540)))
		{
			capFramesize = IDMA_CAP_FRAME_SIZE_SRC_2K2K_3D;	// (1920x1080) 10bit 444 (4byte/pixel) -- for HDMI 1080p FP source
		}else{
			capFramesize = IDMA_CAP_FRAME_SIZE_SRC_2K1K_3D;	// (1920x540) 10bit 444 (4byte/pixel) -- non-1080pFP 3D source
		}
		rtd_pr_i3ddma_debug("[IDMA] alloc=%d * 4KB, 3D Fmt/Wid/Len=%d/%d/%d\n", capFramesize >> 10,
			gen_timing->i3ddma_3dformat, gen_timing->h_act_len, gen_timing->v_act_len);
	}else{
		rtd_pr_i3ddma_debug("\n**** [IDMA] ERR: %s ****\n\n", __FUNCTION__);
		Rt_Sleep(5000);
		return 1;
	}

	// [IDMA] IDMA capture buffer share memory with video decoder
	SCALERDRV_ALLOCBUFFER_INFO stAllocBufferInfo_idma;
	if(drvif_scaler_getValidMemInfo_fromVideo(&stAllocBufferInfo_idma) < 0){
		rtd_pr_i3ddma_debug("[IDMA] fail to get memory information\n");
		IDMA_rpcAllocStartAddr = 0;
		IDMA_rpcAllocSize = 0;
	}else{
		if(drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_SRC_TYPE) == _SRC_VO){
			IDMA_rpcAllocStartAddr = stAllocBufferInfo_idma.optimize_mem_addr;
			IDMA_rpcAllocSize = stAllocBufferInfo_idma.optimize_mem_size;
		}else{
			IDMA_rpcAllocStartAddr = stAllocBufferInfo_idma.normal_mem_addr;
			IDMA_rpcAllocSize = stAllocBufferInfo_idma.normal_mem_size;
		}
		rtd_pr_i3ddma_debug("[IDMA] Src[%d], RPC addr=%x, size req/alloc=%d/%d KB\n", drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_INPUT_SRC_TYPE),
			IDMA_rpcAllocStartAddr, (capFramesize * 4)>> 10, IDMA_rpcAllocSize >> 10);
	}

	if(IDMA_rpcAllocStartAddr && (IDMA_rpcAllocSize >= (capFramesize * 4))){
		for (i=0; i<4; i++) {
			i3ddmaCtrl.cap_buffer[i].cache = (void*)(IDMA_rpcAllocStartAddr + i* capFramesize);
			i3ddmaCtrl.cap_buffer[i].phyaddr = (unsigned long)i3ddmaCtrl.cap_buffer[i].cache;// & ~0x80000000) | 0x40000000;
			i3ddmaCtrl.cap_buffer[i].size = capFramesize;
			rtd_pr_i3ddma_debug("[IDMA] cap_buf[%d]=%x(%d KB)\n", i, (UINT32)i3ddmaCtrl.cap_buffer[i].cache, i3ddmaCtrl.cap_buffer[i].size >> 10);
		}
	}else{
		rtd_pr_i3ddma_debug("\n*** [IDMA] ERR: capFramesize req/alloc=%d/%d (KB) ***\n\n", (capFramesize * 4) >> 10, IDMA_rpcAllocSize >> 10);
		Rt_Sleep(5000);
		return 1;
	}

	return 0;
}
#endif


unsigned char I3DDMA_3D_Init(void);
char I3DDMA_SetupCaptureInit(I3DDMA_CAPTURE_BUFFER_T *cap_buffer, I3DDMA_TIMING_T *timing, I3DDMA_3D_OPMODE_T mode) {

	int /*size,*/ rem, len/*, num*/;
	unsigned int h_act_len;
	int i;

	//h3ddma_cap0_cap_ctl0_RBUS i3ddma_cap_ctl0_reg;
	//h3ddma_cap_ctl1_RBUS i3ddma_cap_ctl1_reg;
//	h3ddma_i3ddma_enable_RBUS i3ddma_i3ddma_enable_reg;
	h3ddma_cap0_cti_dma_wr_ctrl_RBUS h3ddma_cap0_cti_dma_wr_ctrl_reg;

	rtd_pr_i3ddma_debug("!!!!!!!!!!!! %s\n", __FUNCTION__);

	//rtd_maskl(I3DDMA_CAP_CTL0_VADDR, ~(I3DDMA_CAP_CTL0_cap_en_mask),
	//			I3DDMA_CAP_CTL0_cap_en(0));
	h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue = rtd_inl(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_enable = 0;
	rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue);
#if 0
#ifdef I3DDMA_DYNAMIC_MEMORY_ALLOCATE_ENABLE
		int i, ret;

		// allocate IDMA capture buffer address dynamic
	ret = I3DDMA_check_idma_memory_space(timing);
		if(ret == 0){
			for(i=0; i<4; i++)
				cap_buffer[i].phyaddr = i3ddmaCtrl.cap_buffer[i].phyaddr;
		}else{
			rtd_pr_i3ddma_debug("*** [IDMA] ERR: MEMORY ALLOC FAIL!! ***\n\n");
			return I3DDMA_ERR_3D_NO_MEM;
		}
#else

	// [IDMA] allocate/release IDMA memory dynamic
	if(I3DDMA_Get3DAllocReleaseMemoryDynamic() == TRUE){
		int i, ret;
		// [DolbyVison] IDMA capture buffer allocate
		if(get_HDMI_HDR_mode() == HDR_DOLBY_HDMI || get_HDMI_HDR_mode() == HDR_HDR10_HDMI)
			ret = I3DDMA_DolbyVision_HDMI_Init();
		else
			ret = I3DDMA_3D_Init();

		if(ret == 0){
			for(i=0; i<4; i++)
				cap_buffer[i].phyaddr = i3ddmaCtrl.cap_buffer[i].phyaddr;
		}else{
			rtd_pr_i3ddma_debug("*** [IDMA] ERR: Dynamic MEMORY ALLOC FAIL!! ***\n\n");
			return I3DDMA_ERR_3D_NO_MEM;
		}
	}
#endif
#endif

	for(i=0; i<4; i++)
		cap_buffer[i].phyaddr = i3ddmaCtrl.cap_buffer[i].phyaddr;

	switch (mode){

		case I3DDMA_3D_OPMODE_HW:
			//size = cap_buffer[0].size / 4;
			rtd_outl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg, cap_buffer[0].phyaddr);
			rtd_outl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_r0_reg, cap_buffer[1].phyaddr);
			rtd_outl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_1_reg, cap_buffer[2].phyaddr);
			rtd_outl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_r1_reg, cap_buffer[3].phyaddr);
		break;
		case I3DDMA_3D_OPMODE_HW_2DONLY_L:
			//size = cap_buffer[0].size / 2;
			rtd_outl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg, cap_buffer[0].phyaddr);
			rtd_outl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_1_reg, cap_buffer[1].phyaddr);
			rtd_outl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_r0_reg, 0);
			rtd_outl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_r1_reg, 0);

		break;
		case I3DDMA_3D_OPMODE_HW_2DONLY_R:
			//size = cap_buffer[0].size / 2;
			rtd_outl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_r0_reg, cap_buffer[0].phyaddr);
			rtd_outl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_r1_reg, cap_buffer[1].phyaddr);
			rtd_outl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg, 0);
			rtd_outl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_1_reg, 0);
		break;

		case I3DDMA_3D_OPMODE_SW:
        case I3DDMA_3D_OPMODE_DISABLE:
        case I3DDMA_3D_OPMODE_HW_NODMA:
		default:

		break;
	}
	if(drvif_i3ddma_triplebuf_flag())
		rtd_outl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_2_reg, cap_buffer[4].phyaddr);

	// 128 bits = 16 bytes
	rtd_pr_i3ddma_debug("DEPTH2BITS[timing->depth] = %d\n", DEPTH2BITS[timing->depth]);
	rtd_pr_i3ddma_debug("components[timing->color] = %d\n", components[timing->color]);
	rtd_pr_i3ddma_debug("timing->h_act_len = %d\n", timing->h_act_len);

	if(dvrif_i3ddma_compression_get_enable() == TRUE){
		h_act_len = timing->h_act_len;
		if((h_act_len % 32) != 0){
			//drop bits
			h_act_len = h_act_len + (32 - (h_act_len % 32));
		}
		h_act_len = h_act_len * dvrif_i3ddma_get_compression_bits() + 256;
		h_act_len = (h_act_len + 127) / 128;
	}else{
		h_act_len  = (DEPTH2BITS[timing->depth] * (timing->h_act_len /4) * components[timing->color] + 127) / 128;
	}
	rtd_pr_i3ddma_debug("h_act_len = %d\n", h_act_len);
	rem = 12;
	len = 32;
	// 96 align  ,unit 128bit=16byte
	len = drvif_memory_get_data_align((unsigned int)len, DMA_SPEEDUP_ALIGN_VALUE/16);
	//num = (h_act_len - rem) / len;
	rem = (h_act_len - rem) % len + rem;
	rtd_pr_i3ddma_debug("len = %d\n", len);
	rtd_pr_i3ddma_debug("rem = %d\n", rem);
	//rtd_pr_i3ddma_debug("num = %d\n", num);
#if 0
	i3ddma_cap_ctl0_reg.regValue = rtd_inl(H3DDMA_Cap_CTL0_reg);
	i3ddma_cap_ctl0_reg.cap_write_len = len;
	i3ddma_cap_ctl0_reg.cap_write_rem = rem;
	rtd_outl(H3DDMA_Cap_CTL0_reg, i3ddma_cap_ctl0_reg.regValue);

	i3ddma_cap_ctl1_reg.regValue = rtd_inl(H3DDMA_Cap_CTL1_reg);
	i3ddma_cap_ctl1_reg.cap_water_lv = len;
	i3ddma_cap_ctl1_reg.cap_write_num = num;
	rtd_outl(H3DDMA_Cap_CTL1_reg, i3ddma_cap_ctl1_reg.regValue);
#endif
	drvif_h3ddma_set_sequence_capture0(timing);
#ifdef CONFIG_ENABLE_HDMI_NN
	h3ddma_run_film_mode_capture_config(timing);
#endif
#ifndef BUILD_QUICK_SHOW
	if(get_i3ddma_4k120_flag() == TRUE)
		h3ddma_run_4k120_capture_config(timing);
#endif
//	rtd_outl(I3DDMA_CAP_BOUNDARYADDR1_VADDR, cap_buffer[0].phyaddr);
//	rtd_outl(I3DDMA_CAP_BOUNDARYADDR2_VADDR, cap_buffer[0].phyaddr + cap_buffer->size);
	rtd_outl(H3DDMA_CAP0_Cap_Status_reg, 0x1F);

	return I3DDMA_ERR_NO;
}

void drvif_I3DDMA_freeze(unsigned char bFreeze)
{
       h3ddma_cap0_cap_ctl0_RBUS i3ddma_cap_ctl0_reg;
       i3ddma_cap_ctl0_reg.regValue = rtd_inl(H3DDMA_CAP0_Cap_CTL0_reg);
       if(bFreeze == TRUE)
               i3ddma_cap_ctl0_reg.cap0_freeze_en = 1;
       else
               i3ddma_cap_ctl0_reg.cap0_freeze_en = 0;
       rtd_outl(H3DDMA_CAP0_Cap_CTL0_reg, i3ddma_cap_ctl0_reg.regValue);
}



void I3DDMA_Get3DGenTiming(I3DDMA_TIMING_T *tx_timing, I3DDMA_TIMING_T *gen_timing) {

//	int fields;
#if 0	//remove path policy
 #ifdef ENABLE_HDMI_DMA_FOR_4K2K // [HDMI-DMA] HDMI DMA capture enable control for 4k2k timing
	if((tx_timing->h_act_len == 3840) && (tx_timing->v_act_len == 2160) && (tx_timing->v_freq <= 310))
		Scaler_SetHdmiDmaCapture_Enable(TRUE);

	if(Scaler_GetHdmiDmaCapture_Enable())
 		i3ddmaCtrl.path_policy = I3DDMA_PATH_POLICY_3D_DMA;
	else
		i3ddmaCtrl.path_policy = I3DDMA_PATH_POLICY_3D_NODMA;
	rtd_pr_i3ddma_info("[DBG][HDMI] path_policy=%d\n", i3ddmaCtrl.path_policy);
//#elif ENABLE_HDMI_3DDMA_TEST
//	Scaler_SetI3DDMACapture_Enable(TRUE);
 #endif

	// determine output 3D format
	switch (i3ddmaCtrl.path_policy) {

		case I3DDMA_PATH_POLICY_2D_ONLY:
			gen_timing->i3ddma_3dformat = I3DDMA_2D_ONLY;
			i3ddmaCtrl.hw_i3ddma_dma = I3DDMA_3D_OPMODE_DISABLE;
		break;
		case I3DDMA_PATH_POLICY_3D_NODMA:
			i3ddmaCtrl.hw_i3ddma_dma = I3DDMA_3D_OPMODE_DISABLE;
			if (tx_timing->i3ddma_3dformat == I3DDMA_FRAME_PACKING)
				gen_timing->i3ddma_3dformat = I3DDMA_FRAME_PACKING;
			else
				gen_timing->i3ddma_3dformat = I3DDMA_2D_ONLY;
		break;
		case I3DDMA_PATH_POLICY_3D_DMA:
			if (i3ddmaCtrl.i3ddma_3d_capability == 0) gen_timing->i3ddma_3dformat = I3DDMA_2D_ONLY;
			else if (i3ddmaCtrl.force_all_3d_disable) gen_timing->i3ddma_3dformat = I3DDMA_2D_ONLY;
			else if (i3ddmaCtrl.force_2dto3d_enable && tx_timing->i3ddma_3dformat == I3DDMA_2D_ONLY) {
					gen_timing->i3ddma_3dformat = i3ddmaCtrl.force_2dto3d_mode;
					i3ddmaCtrl.hw_i3ddma_dma = I3DDMA_3D_OPMODE_HW;
			} else if (i3ddmaCtrl.force_3dto2d_enable && tx_timing->i3ddma_3dformat != I3DDMA_2D_ONLY) {
					if (i3ddmaCtrl.force_3dto2d_lr_sel) {
							i3ddmaCtrl.hw_i3ddma_dma = I3DDMA_3D_OPMODE_HW_2DONLY_L;
					} else {
							i3ddmaCtrl.hw_i3ddma_dma = I3DDMA_3D_OPMODE_HW_2DONLY_R;
					}
			} else {
				gen_timing->i3ddma_3dformat = tx_timing->i3ddma_3dformat;
				if (gen_timing->i3ddma_3dformat != I3DDMA_2D_ONLY) {
					i3ddmaCtrl.hw_i3ddma_dma = I3DDMA_3D_OPMODE_HW;
				} else {
					i3ddmaCtrl.hw_i3ddma_dma = I3DDMA_3D_OPMODE_DISABLE;
				}
			}
		break;
	}
#endif
    if(tx_timing == NULL || gen_timing == NULL)
    {
        return;
    }

	gen_timing->i3ddma_3dformat = tx_timing->i3ddma_3dformat;
	gen_timing->colorimetry = tx_timing->colorimetry;
	gen_timing->color = i3ddmaCtrl.targetColor; //tx_timing->color;
	gen_timing->depth = i3ddmaCtrl.targetDepth; //tx_timing->depth;
//	gen_timing->quincunx_en = 0;
    gen_timing->src_v_freq = tx_timing->v_freq;

	switch (gen_timing->i3ddma_3dformat) {
			case I3DDMA_2D_ONLY:
				if (tx_timing->i3ddma_3dformat != I3DDMA_2D_ONLY) {  // 3d convert to 2D , don't do DI
					gen_timing->progressive = 1;
				} else {
					gen_timing->progressive = tx_timing->progressive;
				}
				gen_timing->v_total = tx_timing->v_total;
				gen_timing->v_act_len = tx_timing->v_act_len;
				gen_timing->v_act_sta = tx_timing->v_act_sta;
				gen_timing->h_total = tx_timing->h_total;
				gen_timing->h_act_len = tx_timing->h_act_len;
				gen_timing->h_act_sta = tx_timing->h_act_sta;
				gen_timing->v_freq = tx_timing->v_freq;
				gen_timing->h_freq = tx_timing->h_freq;
			break;

			case I3DDMA_FRAMESEQUENCE:
				rtd_pr_i3ddma_debug("I3DDMA_FRAMESEQUENCE \n");
				gen_timing->progressive = tx_timing->progressive;
				gen_timing->color = tx_timing->color;
				gen_timing->depth = tx_timing->depth;
				gen_timing->v_total = tx_timing->v_total;
				//gen_timing->v_freq = tx_timing->v_freq*2;
				gen_timing->v_freq = 1200;
				gen_timing->h_freq = tx_timing->h_freq;
				gen_timing->h_act_len = tx_timing->h_act_len;
				gen_timing->h_total = tx_timing->h_total;
				gen_timing->h_act_sta = tx_timing->h_act_sta;
				gen_timing->v_act_sta = tx_timing->v_act_sta;
				gen_timing->v_act_len = tx_timing->v_act_len;
				gen_timing->v_active_space1 = tx_timing->v_act_sta;
				gen_timing->v_active_space2 = 0;
				tx_timing->v_active_space1 = tx_timing->v_act_sta;
				tx_timing->v_active_space2 = 0;
			break;

			case I3DDMA_FRAME_PACKING:
				I3DDMA_MeasureActiveSpace(tx_timing, gen_timing);
			break;
			case I3DDMA_LINE_ALTERNATIVE:
				gen_timing->progressive = tx_timing->progressive;
				gen_timing->v_total = tx_timing->v_total/2;
				gen_timing->v_act_len = tx_timing->v_act_len/2;
				gen_timing->v_act_sta = tx_timing->v_act_sta;
				gen_timing->h_total = tx_timing->h_total;
				gen_timing->h_act_len = tx_timing->h_act_len;
				gen_timing->h_act_sta = tx_timing->h_act_sta;
				//gen_timing->v_freq = tx_timing->v_freq*2;
				gen_timing->v_freq = 1200;
				gen_timing->h_freq = tx_timing->h_freq;
			break;

			case I3DDMA_SIDE_BY_SIDE_FULL:
				//rtd_pr_i3ddma_debug("I3DDMA_SIDE_BY_SIDE_FULL\n");
				gen_timing->progressive = tx_timing->progressive;
				gen_timing->v_total = tx_timing->v_total/2;
				gen_timing->v_act_len = tx_timing->v_act_len;
				gen_timing->v_act_sta = tx_timing->v_act_sta;
				gen_timing->h_total = tx_timing->h_total/2;
				gen_timing->h_act_len = tx_timing->h_act_len/2;
				gen_timing->h_act_sta = tx_timing->h_act_sta/2;
				//gen_timing->v_freq = tx_timing->v_freq*2;
				gen_timing->v_freq = 1200;
				gen_timing->h_freq = tx_timing->h_freq;
			break;
			case I3DDMA_TOP_AND_BOTTOM:
				//rtd_pr_i3ddma_debug("I3DDMA_TOP_AND_BOTTOM\n");
				gen_timing->progressive = tx_timing->progressive;
				gen_timing->v_total = tx_timing->v_total/2;
				gen_timing->v_act_len = tx_timing->v_act_len/2;
				gen_timing->v_act_sta = tx_timing->v_act_sta/2;
				gen_timing->h_total = tx_timing->h_total;
				gen_timing->h_act_len = tx_timing->h_act_len;
				gen_timing->h_act_sta = tx_timing->h_act_sta;
				//gen_timing->v_freq = tx_timing->v_freq*2;
				gen_timing->v_freq = 1200;
				gen_timing->h_freq = tx_timing->h_freq;
			break;

			case I3DDMA_REALID:
			case I3DDMA_SENSIO:
			case I3DDMA_SIDE_BY_SIDE_HALF:
				//rtd_pr_i3ddma_debug("%s\n", (gen_timing->i3ddma_3dformat == I3DDMA_SIDE_BY_SIDE_HALF? "I3DDMA_SIDE_BY_SIDE_HALF": "I3DDMA_SENSIO"));
				gen_timing->progressive = tx_timing->progressive;
				gen_timing->v_total = tx_timing->v_total/2;
				gen_timing->v_act_len = tx_timing->v_act_len;
				gen_timing->v_act_sta = tx_timing->v_act_sta;
				gen_timing->h_total = tx_timing->h_total/2;
				gen_timing->h_act_len = tx_timing->h_act_len/2;
				gen_timing->h_act_sta = tx_timing->h_act_sta/2;
				//gen_timing->v_freq = tx_timing->v_freq*2;
				gen_timing->v_freq = 1200;
				gen_timing->h_freq = tx_timing->h_freq;

			break;
			case I3DDMA_FIELD_ALTERNATIVE:
				//rtd_pr_i3ddma_debug("I3DDMA_FIELD_ALTERNATIVE\n");
				gen_timing->progressive = tx_timing->progressive;
				gen_timing->v_total = tx_timing->v_total/2;
				gen_timing->v_act_len = tx_timing->v_act_len;
				gen_timing->v_act_sta = tx_timing->v_act_sta;
				gen_timing->h_total = tx_timing->h_total;
				gen_timing->h_act_len = tx_timing->h_act_len;
				gen_timing->h_act_sta = tx_timing->h_act_sta;
				//gen_timing->v_freq = tx_timing->v_freq*2;
				gen_timing->v_freq = 1200;
				gen_timing->h_freq = tx_timing->h_freq;
			break;

			case I3DDMA_VERTICAL_STRIPE:
				//rtd_pr_i3ddma_debug("I3DDMA_VERTICAL_STRIPE\n");
				gen_timing->progressive = tx_timing->progressive;
				gen_timing->v_total = tx_timing->v_total/2;
				gen_timing->v_act_len = tx_timing->v_act_len;
				gen_timing->v_act_sta = tx_timing->v_act_sta;
				gen_timing->h_total = tx_timing->h_total/2;
				gen_timing->h_act_len = tx_timing->h_act_len/2;
				gen_timing->h_act_sta = tx_timing->h_act_sta/2;
				//gen_timing->v_freq = tx_timing->v_freq*2;
				gen_timing->v_freq = 1200;
				gen_timing->h_freq = tx_timing->h_freq;
			break;

			case I3DDMA_CHECKER_BOARD:
				//rtd_pr_i3ddma_debug("I3DDMA_CHECKER_BOARD\n");
				gen_timing->progressive = tx_timing->progressive;
				gen_timing->v_total = tx_timing->v_total/2;
				gen_timing->v_act_len = tx_timing->v_act_len;
				gen_timing->v_act_sta = tx_timing->v_act_sta;
				gen_timing->h_total = tx_timing->h_total/2;
				gen_timing->h_act_len = tx_timing->h_act_len/2;
				gen_timing->h_act_sta = tx_timing->h_act_sta/2;
				//gen_timing->v_freq = tx_timing->v_freq*2;
				gen_timing->v_freq = 1200;
				gen_timing->h_freq = tx_timing->h_freq;
			break;
            case I3DDMA_L_DEPTH:
            case I3DDMA_L_DEPTH_GPX:
            case I3DDMA_RSV1:
            case I3DDMA_UNKOWN:
			default:
			break;
	}

	// [IDMA] interlaced video data FRC in M-domain, IDMA VO go auto mode
#ifdef CONFIG_FORCE_RUN_I3DDMA
	if((drvif_scaler3d_decide_3d_SG_data_FRC() == TRUE) || drvif_scaler3d_decide_3d_PR_enable_IDMA())
#else
	if(/*(tx_timing->progressive == 0) ||*/ (drvif_scaler3d_decide_3d_SG_data_FRC() == TRUE) || drvif_scaler3d_decide_3d_PR_enable_IDMA())
#endif
	{
		if((gen_timing->i3ddma_3dformat != I3DDMA_FRAME_PACKING) && (get_HDMI_HDR_mode() == HDR_MODE_DISABLE))//dolby and hdr have no 3d till now
			gen_timing->v_freq = tx_timing->v_freq*2;

		rtd_pr_i3ddma_debug("[IDMA] 3DFmt[%d]@%c%d Hz\n", gen_timing->i3ddma_3dformat, (tx_timing->progressive? 'p': 'i'), gen_timing->v_freq);
	}

	rtd_pr_i3ddma_debug("[3DDMA] frame_rate_x2 %x, h_freq : %x \n", i3ddmaCtrl.frame_rate_x2, gen_timing->h_freq);

	if(i3ddmaCtrl.frame_rate_x2 == 1){
		gen_timing->v_freq = gen_timing->v_freq*2;
	}

	if(drvif_i3ddma_triplebuf_flag()){
		// [MAC5P-2291] force VO output frame rate >= 48hz
#ifdef CONFIG_MEMC_BYPASS
		if(tx_timing->v_freq <= 310 && (get_current_driver_pattern(DRIVER_FREERUN_PATTERN))){
			// 24->60Hz, 25/30->50/60Hz
			gen_timing->v_freq = (tx_timing->v_freq > 241? tx_timing->v_freq * 2: 600);
			rtd_pr_i3ddma_emerg("[IDMA][Force Data FS] Frame rate = %d->%d\n", tx_timing->v_freq, gen_timing->v_freq);
		}else{
#endif
			if((tx_timing->v_freq > 600) && (tx_timing->v_freq <= 1100))
				gen_timing->v_freq = 500;
			else
				gen_timing->v_freq = 600;
		}
	}
	rtd_pr_i3ddma_debug("[3DDMA] hw_i3ddma_dma : %x \n", i3ddmaCtrl.hw_i3ddma_dma);
	rtd_pr_i3ddma_debug("[3DDMA] i3ddma_3dformat : %x \n", gen_timing->i3ddma_3dformat);
	rtd_pr_i3ddma_debug("[3DDMA] progressive : %x \n", gen_timing->progressive);
	rtd_pr_i3ddma_debug("[3DDMA] chroma : %x \n", gen_timing->color);
	rtd_pr_i3ddma_debug("[3DDMA] v_total : %x \n", gen_timing->v_total);
	rtd_pr_i3ddma_debug("[3DDMA] v_act_len : %x \n", gen_timing->v_act_len);
	rtd_pr_i3ddma_debug("[3DDMA] v_act_sta : %x \n", gen_timing->v_act_sta);
	rtd_pr_i3ddma_debug("[3DDMA] h_total : %x \n", gen_timing->h_total);
	rtd_pr_i3ddma_debug("[3DDMA] h_act_len : %x \n", gen_timing->h_act_len);
	rtd_pr_i3ddma_debug("[3DDMA] h_act_sta : %x \n", gen_timing->h_act_sta);
	rtd_pr_i3ddma_debug("[3DDMA] v_freq : %x \n", gen_timing->v_freq);
	rtd_pr_i3ddma_debug("[3DDMA] h_freq : %x \n", gen_timing->h_freq);

}


unsigned int encodemap[4] = {
	0,
	1,
	0,
	2,
};


char I3DDMA_Setup3DDMA(I3DDMA_3DDMA_CTRL_T* ctrl, I3DDMA_3D_OPMODE_T opmode,unsigned char display) {

	//i3ddma_enable_RBUS 	i3ddma_i3ddma_enable_reg;
	h3ddma_i3ddma_enable_RBUS  i3ddma_i3ddma_enable_reg;
	//lr_separate_ctrl1_RBUS   i3ddma_lr_separate_ctrl1_reg;
//	h3ddma_lr_separate_ctrl1_RBUS  i3ddma_lr_separate_ctrl1_reg;
	//cap_ctl0_RBUS i3ddma_cap_ctl0_reg;
	h3ddma_cap0_cap_ctl0_RBUS  i3ddma_cap_ctl0_reg;
	h3ddma_cap0_wr_dma_pxltobus_RBUS h3ddma_cap0_wr_dma_pxltobus_reg;
	h3ddma_cap0_cti_dma_wr_ctrl_RBUS h3ddma_cap0_cti_dma_wr_ctrl_reg;
	h3ddma_tg_sf_htotal_RBUS h3ddma_tg_sf_htotal_reg;
	dma_vgip_dma_vgip_ctrl_RBUS dma_vgip_dma_vgip_ctrl_reg;



	switch (opmode){

		case I3DDMA_3D_OPMODE_DISABLE:
			h3ddma_tg_sf_htotal_reg.regValue = IoReg_Read32(H3DDMA_TG_SF_HTOTAL_reg);
			h3ddma_tg_sf_htotal_reg.tg_slf_htotal = 0;
			h3ddma_tg_sf_htotal_reg.tg_slf_htotal_en = 0;
			IoReg_Write32(H3DDMA_TG_SF_HTOTAL_reg, h3ddma_tg_sf_htotal_reg.regValue);
			//rtd_pr_i3ddma_debug("I3DDMA_3D_OPMODE_DISABLE\n");

//			rtd_maskl(I3DDMA_CAP_CTL0_VADDR, ~(I3DDMA_CAP_CTL0_cap_en_mask), I3DDMA_CAP_CTL0_cap_en(0));
			#if 0
			i3ddma_i3ddma_enable_reg.regValue = rtd_inl(H3DDMA_I3DDMA_enable_reg);
			i3ddma_lr_separate_ctrl1_reg.regValue = rtd_inl(H3DDMA_LR_Separate_CTRL1_reg);
			i3ddma_lr_separate_ctrl1_reg.hdmi_3d_sel = 0;
			i3ddma_i3ddma_enable_reg.cap_en = 0;
			rtd_outl(H3DDMA_LR_Separate_CTRL1_reg, i3ddma_lr_separate_ctrl1_reg.regValue);
			rtd_outl(H3DDMA_I3DDMA_enable_reg, i3ddma_i3ddma_enable_reg.regValue);
			#endif
			i3ddmaCtrl.enable_3ddma = 0;

		break;
		case I3DDMA_3D_OPMODE_HW:
            rtd_pr_i3ddma_info("[%s %d]\n", __FUNCTION__, __LINE__);
			dma_vgip_dma_vgip_ctrl_reg.regValue = IoReg_Read32(DMA_VGIP_DMA_VGIP_CTRL_reg);
			if(dma_vgip_dma_vgip_ctrl_reg.dma_digital_mode)
			{//digital mode
				h3ddma_tg_sf_htotal_reg.regValue = IoReg_Read32(H3DDMA_TG_SF_HTOTAL_reg);
				h3ddma_tg_sf_htotal_reg.tg_slf_htotal = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_H_LEN);//set h total
				h3ddma_tg_sf_htotal_reg.tg_slf_htotal_en = 1;
				IoReg_Write32(H3DDMA_TG_SF_HTOTAL_reg, h3ddma_tg_sf_htotal_reg.regValue);
			}
			else
			{//analog mode
				h3ddma_tg_sf_htotal_reg.regValue = IoReg_Read32(H3DDMA_TG_SF_HTOTAL_reg);
				h3ddma_tg_sf_htotal_reg.tg_slf_htotal = 0;
				h3ddma_tg_sf_htotal_reg.tg_slf_htotal_en = 0;
				IoReg_Write32(H3DDMA_TG_SF_HTOTAL_reg, h3ddma_tg_sf_htotal_reg.regValue);
			}
			//rtd_pr_i3ddma_debug("I3DDMA_3D_OPMODE_HW\n");
			//rtd_maskl(I3DDMA_CAP_CTL0_VADDR, ~(I3DDMA_CAP_CTL0_cap_en_mask), I3DDMA_CAP_CTL0_cap_en(0));

			h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue = rtd_inl(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg);
			h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_enable = 0;
			rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue);


			h3ddma_cap0_wr_dma_pxltobus_reg.regValue = rtd_inl(H3DDMA_CAP0_WR_DMA_pxltobus_reg);
			h3ddma_cap0_wr_dma_pxltobus_reg.cap0_bit_sel = ctrl->pgen_timing->depth != I3DDMA_COLOR_DEPTH_8B;
			h3ddma_cap0_wr_dma_pxltobus_reg.cap0_pixel_encoding = encodemap[ctrl->pgen_timing->color];
			rtd_outl(H3DDMA_CAP0_WR_DMA_pxltobus_reg, h3ddma_cap0_wr_dma_pxltobus_reg.regValue);

			I3DDMA_SetupLRSeparator(ctrl->ptx_timing, ctrl->pgen_timing);
			I3DDMA_Setup3Dtg(ctrl->pgen_timing, ctrl->frame_rate_x2);
			I3DDMA_SetupCaptureInit(ctrl->cap_buffer, ctrl->pgen_timing, opmode);

			i3ddma_cap_ctl0_reg.regValue = rtd_inl(H3DDMA_CAP0_Cap_CTL0_reg);
#ifndef BUILD_QUICK_SHOW
			if(get_i3ddma_4k120_flag())
				i3ddma_cap_ctl0_reg.cap0_auto_block_sel_en = 0;
			else
#endif
				i3ddma_cap_ctl0_reg.cap0_auto_block_sel_en = 1;
			i3ddma_cap_ctl0_reg.cap0_half_wr_sel = 0;
			i3ddma_cap_ctl0_reg.cap0_half_wr_en = 0;
			if(drvif_i3ddma_triplebuf_flag())
				i3ddma_cap_ctl0_reg.cap0_triple_buf_en = 1;
			else
				i3ddma_cap_ctl0_reg.cap0_triple_buf_en = 0;
			rtd_outl(H3DDMA_CAP0_Cap_CTL0_reg, i3ddma_cap_ctl0_reg.regValue);
			//rtd_maskl(I3DDMA_CAP_CTL0_VADDR, ~(I3DDMA_CAP_CTL0_cap_en_mask),
				//I3DDMA_CAP_CTL0_cap_en(1));

			h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue = rtd_inl(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg);
			h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_enable = 1;
			rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue);

			I3DDMA_SetupVODMA(ctrl->pgen_timing,display);
#ifdef ENABLE_HDMI_DMA_FOR_4K2K // [HDMI-DMA] HDMI/VO DMA enabled
			if(Scaler_GetHdmiDmaCapture_Status() != HDMI_DMA_4K2K_STATE_CHANGE_SRC)
				Scaler_SetHdmiDmaCapture_Status(HDMI_DMA_4K2K_STATE_DMA_ENABLE);
#endif
			i3ddmaCtrl.enable_3ddma = 1;
		break;

		case I3DDMA_3D_OPMODE_HW_2DONLY_L:
			//rtd_pr_i3ddma_debug("I3DDMA_3D_OPMODE_HW_2DONLY_L\n");
			//rtd_maskl(I3DDMA_CAP_CTL0_VADDR, ~(I3DDMA_CAP_CTL0_cap_en_mask), I3DDMA_CAP_CTL0_cap_en(0));
			i3ddma_i3ddma_enable_reg.regValue = rtd_inl(H3DDMA_I3DDMA_enable_reg);
			//i3ddma_i3ddma_enable_reg.cap_en = 0;
			rtd_outl(H3DDMA_I3DDMA_enable_reg, i3ddma_i3ddma_enable_reg.regValue);

			h3ddma_cap0_wr_dma_pxltobus_reg.regValue = rtd_inl(H3DDMA_CAP0_WR_DMA_pxltobus_reg);
			h3ddma_cap0_wr_dma_pxltobus_reg.cap0_bit_sel = ctrl->pgen_timing->depth != I3DDMA_COLOR_DEPTH_8B;
			h3ddma_cap0_wr_dma_pxltobus_reg.cap0_pixel_encoding = encodemap[ctrl->pgen_timing->color];
			rtd_outl(H3DDMA_CAP0_WR_DMA_pxltobus_reg, h3ddma_cap0_wr_dma_pxltobus_reg.regValue);

			I3DDMA_SetupLRSeparator(ctrl->ptx_timing, ctrl->pgen_timing);
			I3DDMA_Setup3Dtg(ctrl->pgen_timing, ctrl->frame_rate_x2);
			I3DDMA_SetupCaptureInit(ctrl->cap_buffer, ctrl->pgen_timing, opmode);

			i3ddma_cap_ctl0_reg.regValue = rtd_inl(H3DDMA_CAP0_Cap_CTL0_reg);
			i3ddma_cap_ctl0_reg.cap0_auto_block_sel_en = 1;
			i3ddma_cap_ctl0_reg.cap0_half_wr_sel = 0;
			i3ddma_cap_ctl0_reg.cap0_half_wr_en = 1;
			rtd_outl(H3DDMA_CAP0_Cap_CTL0_reg, i3ddma_cap_ctl0_reg.regValue);

			//rtd_maskl(I3DDMA_CAP_CTL0_VADDR, ~(I3DDMA_CAP_CTL0_cap_en_mask),I3DDMA_CAP_CTL0_cap_en(1));
			i3ddma_i3ddma_enable_reg.regValue = rtd_inl(H3DDMA_I3DDMA_enable_reg);
		//	i3ddma_i3ddma_enable_reg.cap_en = 1;
			rtd_outl(H3DDMA_I3DDMA_enable_reg, i3ddma_i3ddma_enable_reg.regValue);
			I3DDMA_SetupVODMA(ctrl->pgen_timing,display);
			i3ddmaCtrl.enable_3ddma = 1;

		break;
		case I3DDMA_3D_OPMODE_HW_2DONLY_R:
			//rtd_pr_i3ddma_debug("I3DDMA_3D_OPMODE_HW_2DONLY_R\n");
			//rtd_maskl(I3DDMA_CAP_CTL0_VADDR, ~(I3DDMA_CAP_CTL0_cap_en_mask), I3DDMA_CAP_CTL0_cap_en(0));
			i3ddma_i3ddma_enable_reg.regValue = rtd_inl(H3DDMA_I3DDMA_enable_reg);
//			i3ddma_i3ddma_enable_reg.cap_en = 0;
			rtd_outl(H3DDMA_I3DDMA_enable_reg, i3ddma_i3ddma_enable_reg.regValue);

			h3ddma_cap0_wr_dma_pxltobus_reg.regValue = rtd_inl(H3DDMA_CAP0_WR_DMA_pxltobus_reg);
			h3ddma_cap0_wr_dma_pxltobus_reg.cap0_bit_sel = ctrl->pgen_timing->depth != I3DDMA_COLOR_DEPTH_8B;
			h3ddma_cap0_wr_dma_pxltobus_reg.cap0_pixel_encoding = encodemap[ctrl->pgen_timing->color];
			rtd_outl(H3DDMA_CAP0_WR_DMA_pxltobus_reg, h3ddma_cap0_wr_dma_pxltobus_reg.regValue);

			I3DDMA_SetupLRSeparator(ctrl->ptx_timing, ctrl->pgen_timing);
			I3DDMA_Setup3Dtg(ctrl->pgen_timing, ctrl->frame_rate_x2);
			I3DDMA_SetupCaptureInit(ctrl->cap_buffer, ctrl->pgen_timing, opmode);

			i3ddma_cap_ctl0_reg.regValue = rtd_inl(H3DDMA_CAP0_Cap_CTL0_reg);
			i3ddma_cap_ctl0_reg.cap0_auto_block_sel_en = 1;
			i3ddma_cap_ctl0_reg.cap0_half_wr_sel = 1;
			i3ddma_cap_ctl0_reg.cap0_half_wr_en = 1;
			rtd_outl(H3DDMA_CAP0_Cap_CTL0_reg, i3ddma_cap_ctl0_reg.regValue);

			//rtd_maskl(I3DDMA_CAP_CTL0_VADDR, ~(I3DDMA_CAP_CTL0_cap_en_mask),I3DDMA_CAP_CTL0_cap_en(1));
			i3ddma_i3ddma_enable_reg.regValue = rtd_inl(H3DDMA_I3DDMA_enable_reg);
		//	i3ddma_i3ddma_enable_reg.cap_en = 1;
			rtd_outl(H3DDMA_I3DDMA_enable_reg, i3ddma_i3ddma_enable_reg.regValue);
			I3DDMA_SetupVODMA(ctrl->pgen_timing,display);
			i3ddmaCtrl.enable_3ddma = 1;


		break;
		case I3DDMA_3D_OPMODE_HW_NODMA:
			//rtd_pr_i3ddma_debug("I3DDMA_3D_OPMODE_HW_NODMA\n");
			I3DDMA_SetupLRSeparator(ctrl->ptx_timing, ctrl->pgen_timing);
			I3DDMA_Setup3Dtg(ctrl->pgen_timing, ctrl->frame_rate_x2);
			rtd_pr_i3ddma_debug("%s %d\n", __FUNCTION__, __LINE__);
			i3ddmaCtrl.enable_3ddma = 0;
		break;
		case I3DDMA_3D_OPMODE_SW:
		default:
			return I3DDMA_ERR_3D_WRONG_OPMODE;
		break;

	}


	return I3DDMA_ERR_NO;

}

#ifdef CONFIG_FORCE_RUN_I3DDMA //dump i3ddma buf for debug
#define I3DDMA_FILE_PATH "/tmp/i3ddma_dump.raw"
#define I3DDMA_FILE_PATH_Y "/tmp/i3ddma_y_dump.raw"
#define I3DDMA_FILE_PATH_C "/tmp/i3ddma_c_dump.raw"
#include <linux/fs.h>

#ifdef CONFIG_RTK_FEATURE_FOR_GKI
static struct file* file_open(const char* path, int flags, int rights) {
	struct file* filp = NULL;
	mm_segment_t oldfs;
	int err = 0;

	oldfs = get_fs();
	set_fs(get_ds());
	filp = filp_open(path, flags, rights);
	set_fs(oldfs);
	if(IS_ERR(filp)) {
		err = PTR_ERR(filp);
		return NULL;
	}
	return filp;
}

static void file_close(struct file* file) {
	filp_close(file, NULL);
}

/*static int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(get_ds());

	ret = vfs_read(file, data, size, &offset);

	set_fs(oldfs);
	return ret;
}*/


static int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(get_ds());

	ret = vfs_write(file, data, size, &offset);

	set_fs(oldfs);
	return ret;
}

static int file_sync(struct file* file) {
	vfs_fsync(file, 0);
	return 0;
}
#else
static struct file* file_open(const char* path, int flags, int rights) {
	return NULL;
}

static void file_close(struct file* file) {
}

static int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
	return 0;
}

static int file_sync(struct file* file) {
	return 0;
}
#endif

#ifdef CONFIG_ENABLE_HDMI_NN
unsigned int g_ulFileCount = 0;
int i3ddma_dump_data_to_file(unsigned int bufferindex)
{
	struct file* filp = NULL;
	unsigned long outfileOffset = 0;
	unsigned int size = (g_tNN_Cap_Buffer[0].size + g_tNN_Cap_Buffer[0].size / 2) * NN_CAP_BUFFER_NUM;
	unsigned char* pVirStartAdrr = NULL;

	//char y_path[30] = "/tmp/i3ddma_y_dump.raw_";
	//char c_path[30] = "/tmp/i3ddma_c_dump.raw_";


	char y_path[30] = "/tmp/i3ddma_y_dump.raw";
	char c_path[30] = "/tmp/i3ddma_c_dump.raw";

	char filecount[5];

	//unsigned int i=0, j=0;
	//unsigned char temp= 0;

	g_ulFileCount++;
	sprintf(filecount, "%d", g_ulFileCount);

	//strcat(y_path, filecount);
	//strcat(c_path, filecount);

	rtd_pr_i3ddma_emerg("y_file=%s\n", y_path);

	size = drvif_memory_get_data_align(size, __12KPAGE);
	pVirStartAdrr = (unsigned char*)dvr_remap_uncached_memory((unsigned int)g_tNN_Cap_Buffer[0].phyaddr, size, __builtin_return_address(0));

	//if (i3ddmaCtrl.cap_buffer[bufferindex].cache != NULL) {
	if (pVirStartAdrr != 0) {
//		rtd_pr_i3ddma_emerg("\n\n\n\n *****************  i3ddma_dump_data_to_file start  ******%x***************\n\n\n\n",i3ddmaCtrl.cap_buffer[bufferindex].cache);
		//filp = file_open(I3DDMA_FILE_PATH_Y, O_RDWR | O_CREAT, 0);
		rtd_pr_i3ddma_emerg("y_path:%s\n", y_path);
		filp = file_open(y_path, O_RDWR | O_CREAT | O_TRUNC, 0666);
		if (filp == NULL) {
			rtd_pr_i3ddma_emerg("[%s, %d]file open fail\n", __FUNCTION__, __LINE__);
			return FALSE;
		}

		// Disable cap1, cap2
		//h3ddma_run_film_mode_enable(0);

		#if 0
		rtd_pr_i3ddma_emerg("Do byte swap\n");
		// Byte swap (unit: 16byte)
		for (i = 0; i < g_tNN_Cap_Buffer[0].size/16; i++)
		{
			for (j = 0; j < 8; j++)
			{
				// j=0, temp=byte 0
				temp = *(unsigned char*)(g_uncacheaddr + i * 16 + j);

				// byte 0 = byte 15
				*(unsigned char*)(g_uncacheaddr + i * 16 + j) = *(unsigned char*)(g_uncacheaddr + i * 16 + 15-j);

				// byte 15 = temp
				*(unsigned char*)(g_uncacheaddr + i * 16 + 15-j) = temp;
			}
		}
		#endif


		//unsigned int g_ulNNOutWidth=960;
		//unsigned int g_ulNNOutLength=540;
		//file_write(filp, outfileOffset, (unsigned char*)i3ddmaCtrl.cap_buffer[bufferindex].cache, i3ddmaCtrl.cap_buffer[bufferindex].size);
		//file_write(filp, outfileOffset, (unsigned char*)g_uncacheaddr, i3ddmaCtrl.cap_buffer[bufferindex].size);
		rtd_pr_i3ddma_emerg("dump y data uncache add=%x, size=%d\n", (unsigned int)(pVirStartAdrr+bufferindex*g_tNN_Cap_Buffer[0].size), g_ulNNOutWidth*g_ulNNOutLength);
		//file_write(filp, outfileOffset, (unsigned char*)g_uncacheaddr, 1920*1080);
		file_write(filp, outfileOffset, (unsigned char*)pVirStartAdrr+bufferindex*g_tNN_Cap_Buffer[0].size, g_ulNNOutWidth*g_ulNNOutLength);

		file_sync(filp);
		file_close(filp);

#if 1
		rtd_pr_i3ddma_emerg("\n\n\n\n *****************  i3ddma_dump_data_to_file Y end  *********************\n\n\n\n");

		//filp = file_open(I3DDMA_FILE_PATH_C, O_RDWR | O_CREAT, 0);
		filp = file_open(c_path, O_RDWR | O_CREAT | O_TRUNC, 0666);
		if (filp == NULL) {
			rtd_pr_i3ddma_emerg("[%s, %d]file open fail\n", __FUNCTION__, __LINE__);
			return FALSE;
		}
		//file_write(filp, outfileOffset, (unsigned char*)i3ddmaCtrl.cap_buffer[bufferindex].cache, i3ddmaCtrl.cap_buffer[bufferindex].size);
		rtd_pr_i3ddma_emerg("dump c data uncache add=%x, size=%d\n", (unsigned int)(pVirStartAdrr +  (g_tNN_Cap_Buffer[0].size)*NN_CAP_BUFFER_NUM + (g_tNN_Cap_Buffer[0].size/2)*bufferindex), g_ulNNOutWidth*g_ulNNOutLength/2);
		//file_write(filp, outfileOffset, (unsigned char*)(g_uncacheaddr +  (g_tNN_Cap_Buffer[0].size)/2), 1920*1080/2);
		file_write(filp, outfileOffset, (unsigned char*)(pVirStartAdrr +  (g_tNN_Cap_Buffer[0].size)*NN_CAP_BUFFER_NUM + (g_tNN_Cap_Buffer[0].size/2)*bufferindex), g_ulNNOutWidth*g_ulNNOutLength/2);
		file_sync(filp);
		file_close(filp);

		rtd_pr_i3ddma_emerg("\n\n\n\n *****************  i3ddma_dump_data_to_file C end  *********************\n\n\n\n");
#endif
		dvr_unmap_memory(pVirStartAdrr, size);
		return TRUE;
	} else {
		rtd_pr_i3ddma_emerg("\n\n\n\n *****************  i3ddma_dump_data_to_file NG for null buffer address  *********************\n\n\n\n");
		return FALSE;
	}
}

void h3ddma_get_NN_read_buffer_test(unsigned int *a_pulYAddr, unsigned int *a_pulCAddr)
{
	*a_pulYAddr = IoReg_Read32(H3DDMA_CAP1_WR_DMA_num_bl_wrap_addr_0_reg);
	*a_pulCAddr = IoReg_Read32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_addr_0_reg);
}

#ifdef CONFIG_FORCE_RUN_I3DDMA
void i3ddma_nn_buffer_dump_debug(void)
{
	// Test NN read buffer, check 0xB8025800[27]=1 ?
	if (((IoReg_Read32(H3DDMA_LR_Separate_CTRL1_reg) & 0x08000000)>>27) == 1)
	{
		h3ddma_get_NN_read_buffer(&ulNNYAddr, &ulNNCAddr);

		rtd_pr_i3ddma_emerg("NNYAddr=%x, NNCAddr=%x\n", ulNNYAddr, ulNNCAddr);

		//h3ddma_get_NN_read_buffer_test(&ulNNYAddr, &ulNNCAddr);

		//rtd_pr_i3ddma_emerg("test NNYAddr=%x, NNCAddr=%x\n", ulNNYAddr, ulNNCAddr);
	}

	// Dump NN data once, 0xB8025800[28]=1
	if (((IoReg_Read32(H3DDMA_LR_Separate_CTRL1_reg) & 0x10000000)>>28) == 1)
	{
		i3ddma_dump_data_to_file(0);
		IoReg_ClearBits(H3DDMA_LR_Separate_CTRL1_reg, _BIT28);
	}
}
#endif

#endif //end of #ifdef CONFIG_ENABLE_HDMI_NN


int i3ddma_dump_cap0cap2_to_file(unsigned int bufferindex)
{
	struct file* filp = NULL;
	unsigned long outfileOffset = 0;

	char y_path[32] = "/tmp/i3ddma_cap0_dump.raw";
	char c_path[32] = "/tmp/i3ddma_cap2_dump.raw";
	char se_path[32] = "/tmp/i3ddma_se_dump.raw";
	unsigned int video_size = g_4K120_Cap_C_Buffer[0].size;

	bufferindex = 0;
	video_size = drvif_memory_get_data_align(video_size, __12KPAGE);;
	g_4K120_Cap_Y_Buffer[0].uncache = dvr_remap_uncached_memory(g_4K120_Cap_Y_Buffer[0].phyaddr, video_size * 2, __builtin_return_address(0));
	g_4K120_Cap_C_Buffer[0].uncache = dvr_remap_uncached_memory(g_4K120_Cap_C_Buffer[0].phyaddr, video_size, __builtin_return_address(0));
	g_4K120_SE_Buffer[0].uncache = dvr_remap_uncached_memory(g_4K120_SE_Buffer[0].phyaddr, video_size, __builtin_return_address(0));
	rtd_pr_i3ddma_notice("[4k120]uncache addr[%lx,%lx,%lx]\n",(unsigned long)g_4K120_Cap_Y_Buffer[0].uncache,(unsigned long)g_4K120_Cap_C_Buffer[0].uncache,(unsigned long)g_4K120_SE_Buffer[0].uncache);

	filp = file_open(y_path, O_RDWR | O_CREAT, 0);
	if (filp == NULL) {
		rtd_pr_i3ddma_emerg("file open fail\n");
		return FALSE;
	}
	rtd_pr_i3ddma_emerg("dump cap0 data uncache addr=%lx, size=%x\n", (unsigned long)g_4K120_Cap_Y_Buffer[0].uncache, g_4K120_Cap_Y_Buffer[0].size);

	file_write(filp, outfileOffset, g_4K120_Cap_Y_Buffer[0].uncache, g_4K120_Cap_Y_Buffer[0].size);
	file_sync(filp);
	file_close(filp);
	rtd_pr_i3ddma_emerg("\n\n\n\n *****************  i3ddma_dump_cap0cap2_to_file cap0 end  *********************\n\n\n\n");

	filp = file_open(c_path, O_RDWR | O_CREAT, 0);
	if (filp == NULL) {
		rtd_pr_i3ddma_emerg("file open fail\n");
			return FALSE;
	}

	rtd_pr_i3ddma_emerg("dump cap2 data uncache addr=%lx, size=%x\n", (unsigned long)g_4K120_Cap_C_Buffer[0].uncache, video_size);
	file_write(filp, outfileOffset, (unsigned char*)g_4K120_Cap_C_Buffer[0].uncache, g_4K120_Cap_C_Buffer[0].size);
	file_sync(filp);
	file_close(filp);
	rtd_pr_i3ddma_emerg("\n\n\n\n *****************  i3ddma_dump_cap0cap2_to_file cap2 end  *********************\n\n\n\n");

	filp = file_open(se_path, O_RDWR | O_CREAT, 0);
	if (filp == NULL) {
		rtd_pr_i3ddma_emerg("file open fail\n");
			return FALSE;
	}

	rtd_pr_i3ddma_emerg("dump se data uncache addr=%lx, size=%x\n", (unsigned long)g_4K120_SE_Buffer[0].uncache, g_4K120_SE_Buffer[0].size);
	file_write(filp, outfileOffset, (unsigned char*)g_4K120_SE_Buffer[0].uncache, g_4K120_SE_Buffer[0].size);
	file_sync(filp);
	file_close(filp);
	rtd_pr_i3ddma_emerg("\n\n\n\n *****************  i3ddma_dump_cap0cap2_to_file se end  *********************\n\n\n\n");

	dvr_unmap_memory(g_4K120_Cap_Y_Buffer[0].uncache, video_size * 2);
	dvr_unmap_memory(g_4K120_Cap_C_Buffer[0].uncache, video_size);
	dvr_unmap_memory(g_4K120_SE_Buffer[0].uncache, video_size);
	return TRUE;
}
#endif

unsigned long I3DDMA_GetMemChunkAddr(unsigned int size)
{
#define _OFFSET_BYTES    (24*1024*1024)//offset 30M Bytes//32*1024*1024
    unsigned long ulPhyaddr = 0;

    if(i3ddmaCtrl.cap_buffer[0].phyaddr)
	{

		if (size >  i3ddmaCtrl.cap_buffer[0].getsize - _OFFSET_BYTES)
		{
			rtd_pr_i3ddma_emerg("I3DDMA_GetMemChunkAddr get size(%d) fai\n",size);
		 	return 0;
		}


	    ulPhyaddr = i3ddmaCtrl.cap_buffer[0].phyaddr+_OFFSET_BYTES;
	}
	return ulPhyaddr;
}
void disable_i3ddma_cap0(void)
{//disable capture 0 for 4k120 borrow i3ddma memory
	unsigned int timeoutcount = 5;
	IoReg_ClearBits(H3DDMA_H3DDMA_PQ_CMP_DOUBLE_BUFFER_CTRL_reg, H3DDMA_H3DDMA_PQ_CMP_DOUBLE_BUFFER_CTRL_db_read_sel_mask);
	drvif_I3DDMA_Reset_Vgip_Source_Sel();
	h3ddma_set_capture_enable(0,0);//disable i3ddma capture 0. becasue need alloc memocy again
	do {
		if((H3DDMA_CAP0_CTI_DMA_WR_Ctrl_get_cap0_dma_enable(IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg)) == 0))
			break;

		msleep(10);
		timeoutcount--;
	}while(timeoutcount);

	rtd_pr_i3ddma_notice("### %s i3ddma capture disable status (%d), timeoutcnt=%d db=%x", __FUNCTION__, (H3DDMA_CAP0_CTI_DMA_WR_Ctrl_get_cap0_dma_enable(IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg))),
				timeoutcount, IoReg_Read32(H3DDMA_H3DDMA_PQ_CMP_DOUBLE_BUFFER_CTRL_reg));

	//recovery i3ddma read level
	IoReg_SetBits(H3DDMA_H3DDMA_PQ_CMP_DOUBLE_BUFFER_CTRL_reg, H3DDMA_H3DDMA_PQ_CMP_DOUBLE_BUFFER_CTRL_db_read_sel_mask);
}
#ifdef BUILD_QUICK_SHOW
unsigned int I3DDMA_CalcBufferSize(unsigned char compressEnable, MemCompression_Bits eCompressBit)
{
    unsigned int ulBufferSize = 0;

    if(compressEnable)
    {
        ulBufferSize = (_4K_WID * eCompressBit + 256) * _2K_LEN / 8;
    }
    else
    {
        ulBufferSize = (_4K_WID * COMPRESSION_12_BITS + 256) * _2K_LEN / 8;
    }

    return ulBufferSize;
}
#endif
unsigned char I3DDMA_DolbyVision_HDMI_Init_impl(void) {
//#define _4K_2K_10BIT_SIZE      (_4K_WID * _2K_LEN * 30/8)
//#define _2K_1K_10BIT_SIZE      (1920 * 1080 * 30/8)
	unsigned int size_metadata = _1M_SIZE; // 1MB for metadata buffer
	unsigned int size_video;
//	int i;
	int allocSize = 0;
	unsigned long getSize = 0;
	char bAlloc = false;
	unsigned long malloccacheaddr_metadata = 0;
	unsigned long malloccacheaddr_video_phyaddr = 0;//(unsigned int)dvr_malloc_uncached_specific(size_video*2, GFP_DCU2_FIRST, NULL);//(unsigned int)dvr_malloc(size_video*2);//for double buffer use
	int cur_act_w=0, cur_act_h=0;
	#ifdef CONFIG_ARM64
	unsigned long query_addr = 0, align_addr = 0;
	#else
	unsigned int query_addr = 0, align_addr = 0;
	#endif
	unsigned int uhd_flag = 0;

	//if(dvrif_i3ddma_compression_get_enable() == TRUE)
		//size_video = (_4K_WID * _2K_LEN * dvrif_i3ddma_get_compression_bits()) / 8;
	//else
#ifndef CONFIG_RTK_KDRV_DV_IDK_DUMP_RGB
		size_video = _4K_2K_10BIT_SIZE ;//4096*2160 RGB444 10bit sequence buffer
#else
		size_video = _4K_2K_12BIT_SIZE;
#endif

	//rtd_pr_i3ddma_emerg("I3DDMA_DolbyVision_HDMI_Init size_video=%x \n",size_video);

#if 0//def CONFIG_FORCE_RUN_I3DDMA
		if(i3ddmaCtrl.i3ddma_3d_capability == 1)
		{
			rtd_pr_i3ddma_debug("I3DDMA_DolbyVision_HDMI_Init memory have alloc\n");
			return 0;
		}
#endif
#ifdef CONFIG_I2RND_ENABLE
	if(Scaler_I2rnd_get_enable() == _ENABLE){
			size_video = _2K_1K_10BIT_SIZE ;
	}
    else
#endif
    {
		cur_act_w = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPH_ACT_WID_PRE);//Orignal source size
		if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE))
			cur_act_h = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE)*2;//Orignal source size
		else
			cur_act_h = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN_PRE);//Orignal source size
		//rtd_pr_i3ddma_emerg("[Minchay] cur_act_w:%d, cur_act_h:%d \n",cur_act_w,cur_act_h);
		if(cur_act_w * cur_act_h <= _2K_WID *_1K_LEN)//20180507 pinyen, fix 1600x1200 timing ,k5ap can just allocate 2k1k size
		{//video under 2k*1k size, set vmb size to 2k*1k from 4k*2k
				size_video = _2K_1K_10BIT_SIZE ;
				uhd_flag = 0;
		}
		else
			uhd_flag = 1;
	}
//#ifndef BUILD_QUICK_SHOW
#if 1
	rtd_pr_i3ddma_debug("%s.%d\n", __FUNCTION__, __LINE__);
	if(!is_2k_memory_buffer_source(SLR_MAIN_DISPLAY))
	{
		size_video = _4K_2K_MDOMAIN_16BIT_SIZE ;
		size_video = dvr_size_alignment(size_video);
		allocSize = size_video*3;// 1 m domain cave out  2 vbm memory
		disable_i3ddma_cap0();//diable i3ddma

	}
 	else
    {
		if(dvrif_i3ddma_compression_get_enable() == TRUE)
		{
			if(uhd_flag)
			{
				if(dvrif_i3ddma_get_compression_bits() == COMPRESSION_20_BITS) //pc mode
				{
					size_video = _4K_2K_20BIT_SIZE;
				}
				else
				{
					size_video = _4K_2K_12BIT_SIZE;
				}
			}
			else
			{
				if(dvrif_i3ddma_get_compression_bits() == COMPRESSION_20_BITS) //pc mode
				{
					size_video = _2K_1K_20BIT_SIZE;
				}
				else
				{
					size_video = _2K_1K_12BIT_SIZE;
				}
			}
#ifdef CONFIG_RTK_KDRV_DV
		} else if (check_hdmi_dolby_vision_rgb_and_full_hd()) {
			size_video = _2K_1K_36BIT_SIZE;
			pr_info("[DOLBY] hdmi dolby vision RGB888 12 bit mode, video size = %d bytes\n", size_video);
#endif
		}

    	if(drvif_i3ddma_triplebuf_flag())
    	{
    		size_video = dvr_size_alignment(size_video);
    		allocSize = size_video*3;
    	}
    	else
    	{
    		size_video = dvr_size_alignment(size_video);
    	    allocSize = size_video*2;
    	}
#ifndef BUILD_QUICK_SHOW
		if(get_i3ddma_4k120_flag())
		{
			if(h3ddma_allocate_4k120_se_buffer() == FALSE)
			{
				rtd_pr_i3ddma_emerg("i3ddma 4K120 alloc memory fail!\n");
				return 1;
			}
		}
#endif
 	}
#else
    size_video = I3DDMA_CalcBufferSize(dvrif_i3ddma_compression_get_enable(),dvrif_i3ddma_get_compression_bits());
    if(drvif_i3ddma_triplebuf_flag())
    {
        allocSize = size_video*3;
    }
    else
    {
        allocSize = size_video*2;
    }
#endif
	//rtd_pr_i3ddma_emerg("allocSize=%x\n", allocSize);

	/*
	rtd_outl(H3DDMA_Cap_L1_Start_reg, cap_buffer[0].phyaddr);
	rtd_outl(H3DDMA_Cap_L2_Start_reg, cap_buffer[2].phyaddr);
	rtd_outl(H3DDMA_Cap_R1_Start_reg, cap_buffer[1].phyaddr);
	rtd_outl(H3DDMA_Cap_R2_Start_reg, cap_buffer[3].phyaddr);
	*/
	//video capture buffer
	bAlloc = false;
	if(i3ddmaCtrl.cap_buffer[0].phyaddr == (unsigned long)NULL)
	{
	   bAlloc = true;
	}
	else if((i3ddmaCtrl.cap_buffer[0].getsize < allocSize)){
	// capture mem < allocSize need realloc mem
	    rtd_pr_i3ddma_emerg("[DolbyVision][Minchay][%s] Error: vbm mem has alloced size(getSize:%d ) < allocSize(%d) \n", __FUNCTION__ , i3ddmaCtrl.cap_buffer[0].getsize, allocSize);
#ifndef BUILD_QUICK_SHOW
        I3DDMA_DolbyVision_HDMI_DeInit();
#endif
	    bAlloc = true;
	}
	if(bAlloc)
    {
#ifdef CONFIG_VBM_HEAP
		unsigned long dmabuf_hndl = 0;
#endif

#ifdef CONFIG_SCALER_BRING_UP
			malloccacheaddr_video_phyaddr = (unsigned long)dvr_malloc_uncached_specific(allocSize, GFP_DCU1,(void **)&i3ddmaCtrl.cap_buffer[0].uncache);
			if(malloccacheaddr_video_phyaddr == (unsigned long)NULL)
			{
				rtd_pr_i3ddma_emerg("[DolbyVision] alloc idma sequence metadata memory failed[%lx]\n", malloccacheaddr_video_phyaddr);
				return 1; //malloc memory fail
			}
			i3ddmaCtrl.cap_buffer[0].cache= (void*)malloccacheaddr_video_phyaddr;
			malloccacheaddr_video_phyaddr=(unsigned int)dvr_to_phys((void*)malloccacheaddr_video_phyaddr);
#else
#ifndef BUILD_QUICK_SHOW
        rtd_pr_i3ddma_debug("[DolbyVision][%s] call VBM_BorrowMemChunk\n",__FUNCTION__);
		// buffer[0][2] -- video capture buffer
#ifdef CONFIG_VBM_HEAP
		if (VBM_BorrowMemChunk(&dmabuf_hndl, (unsigned long *)(&malloccacheaddr_video_phyaddr), (unsigned long *)(&getSize), allocSize,0) != 0) {
#else
		if (VBM_BorrowMemChunk((unsigned long *)(&malloccacheaddr_video_phyaddr), (unsigned long *)(&getSize), allocSize,0) != 0) {
#endif
			rtd_pr_i3ddma_emerg("[DolbyVision] alloc idma video sequence memory failed[%lx]\n",  malloccacheaddr_video_phyaddr);
			return 1; //malloc memory fail
		}
		if(getSize < _4K_2K_10BIT_SIZE)
		    rtd_pr_i3ddma_emerg("[DolbyVision][Minchay][%s] Error: vbm mem size(getSize:%ld) is not 64M \n", __FUNCTION__ ,getSize);
#else
    //malloccacheaddr_video_phyaddr = get_query_start_address(QUERY_IDX_I3DDMA);
    extern unsigned long get_vbm_heap_qshow_scaler_address(void);
    malloccacheaddr_video_phyaddr = get_vbm_heap_qshow_scaler_address();
#endif
#endif
		//merlin4 memory do 96 bytes align @Crixus 20180321
		#ifdef CONFIG_ARM64
		query_addr = (unsigned long)malloccacheaddr_video_phyaddr;
		//size_video = dvr_size_alignment(size_video);
		align_addr = dvr_memory_alignment((unsigned long)query_addr, size_video);
		if (align_addr == (unsigned long)NULL)
		{
			//BUG();
			rtd_pr_i3ddma_err("[%s %d]align_addr is NULL\n", __FUNCTION__, __LINE__);
            return 1;
		}
		#else
		query_addr = (unsigned int)malloccacheaddr_video_phyaddr;
		//size_video = dvr_size_alignment(size_video);
		align_addr = dvr_memory_alignment((unsigned long)query_addr, size_video);
		if (align_addr == (unsigned int)NULL)
		{
			//BUG();
			rtd_pr_i3ddma_err("[%s %d]align_addr is NULL\n", __FUNCTION__, __LINE__);
            return 1;
		}

		rtd_pr_i3ddma_notice("query_addr=%x, align_addr=%x, size_video=%x, \n", query_addr, align_addr, size_video);
		#endif
		rtd_pr_i3ddma_notice("[VBM] allocSize:%d, getSize:%ld w:%d h:%d\n", allocSize, getSize, cur_act_w, cur_act_h);
		i3ddmaCtrl.cap_buffer[0].cache= NULL;//(void*)(malloccacheaddr_video);
		i3ddmaCtrl.cap_buffer[0].phyaddr =  align_addr;//(unsigned int)dvr_to_phys((void*)i3ddmaCtrl.cap_buffer[0].cache);
		i3ddmaCtrl.cap_buffer[0].phyaddr_unalign = (unsigned int)malloccacheaddr_video_phyaddr;//save un align address for free memory
		i3ddmaCtrl.cap_buffer[0].getsize = getSize;
#ifdef CONFIG_VBM_HEAP
		i3ddmaCtrl.cap_buffer[0].dmabuf_hndl = dmabuf_hndl;
#endif
    }
	if(!is_2k_memory_buffer_source(SLR_MAIN_DISPLAY))
	{
        //Conditional redundancy
		//if(i3ddmaCtrl.cap_buffer[0].phyaddr)
		{
    		i3ddmaCtrl.cap_buffer[0].size = size_video;
			return 0;
		}
	}
    //Conditional redundancy
    //if(i3ddmaCtrl.cap_buffer[0].phyaddr)
    {
		// buffer[0][2] -- video capture buffer
		//i3ddmaCtrl.cap_buffer[0].cache= NULL;//(void*)(malloccacheaddr_video);
		//i3ddmaCtrl.cap_buffer[0].phyaddr =  (unsigned int)malloccacheaddr_video_phyaddr;//(unsigned int)dvr_to_phys((void*)i3ddmaCtrl.cap_buffer[0].cache);
		i3ddmaCtrl.cap_buffer[0].size = size_video;
		//i3ddmaCtrl.cap_buffer[0].getsize = getSize;
		rtd_pr_i3ddma_info("[DolbyVision][Video] buf[0]=%lx(%d KB), phy(%lx) \n", (unsigned long)i3ddmaCtrl.cap_buffer[0].cache, i3ddmaCtrl.cap_buffer[0].size >> 10, (unsigned long)i3ddmaCtrl.cap_buffer[0].phyaddr);

		i3ddmaCtrl.cap_buffer[2].cache= NULL;//(void*)(malloccacheaddr_video + size_video);
		i3ddmaCtrl.cap_buffer[2].phyaddr = drvif_memory_get_data_align((unsigned int)i3ddmaCtrl.cap_buffer[0].phyaddr + size_video, DMA_SPEEDUP_ALIGN_VALUE); //(unsigned int)dvr_to_phys((void*)i3ddmaCtrl.cap_buffer[2].cache);
		i3ddmaCtrl.cap_buffer[2].size = size_video;
		//i3ddmaCtrl.cap_buffer[2].getsize = getSize;
		rtd_pr_i3ddma_info("[DolbyVision][Video] buf[2]=%lx(%d KB), phy(%lx) \n", (unsigned long)i3ddmaCtrl.cap_buffer[2].cache, i3ddmaCtrl.cap_buffer[2].size >> 10, (unsigned long)i3ddmaCtrl.cap_buffer[2].phyaddr);

		if(drvif_i3ddma_triplebuf_flag())
		{
			i3ddmaCtrl.cap_buffer[4].cache= NULL;//(void*)(malloccacheaddr_video + size_video);
			i3ddmaCtrl.cap_buffer[4].phyaddr = drvif_memory_get_data_align((unsigned int)i3ddmaCtrl.cap_buffer[0].phyaddr + size_video*2, DMA_SPEEDUP_ALIGN_VALUE); //(unsigned int)dvr_to_phys((void*)i3ddmaCtrl.cap_buffer[2].cache);
			i3ddmaCtrl.cap_buffer[4].size = size_video;
			//i3ddmaCtrl.cap_buffer[4].getsize = getSize;
			rtd_pr_i3ddma_info("[DolbyVision][Video] buf[2]=%lx(%d KB), phy(%lx) \n", (unsigned long)i3ddmaCtrl.cap_buffer[4].cache, i3ddmaCtrl.cap_buffer[4].size >> 10, (unsigned long)i3ddmaCtrl.cap_buffer[4].phyaddr);
		}
		//i3ddma protection

		rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_low_reg, i3ddmaCtrl.cap_buffer[0].phyaddr); //cap0 low limit
		rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_up_reg, i3ddmaCtrl.cap_buffer[0].phyaddr + size_video); //cap0 up limit
		rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_low_1_reg, i3ddmaCtrl.cap_buffer[2].phyaddr); //cap1 low limit
		rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_up_1_reg, i3ddmaCtrl.cap_buffer[2].phyaddr + size_video); //cap1 up limit
		if(drvif_i3ddma_triplebuf_flag())
		{
			rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_low_2_reg, i3ddmaCtrl.cap_buffer[4].phyaddr); //cap2 low limit
			rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_up_2_reg, i3ddmaCtrl.cap_buffer[4].phyaddr + size_video);//cap 2high limit
		}

		rtd_pr_i3ddma_info("i3ddma cap0 boundary low=%x, high=%x\n", rtd_inl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_low_reg), rtd_inl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_up_reg));
		rtd_pr_i3ddma_info("i3ddma cap1 boundary low=%x, high=%x\n", rtd_inl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_low_1_reg), rtd_inl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_up_1_reg));
		rtd_pr_i3ddma_info("i3ddma cap2 boundary low=%x, high=%x\n", rtd_inl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_low_2_reg), rtd_inl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_up_2_reg));
    }
#ifndef BUILD_QUICK_SHOW
    if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)&&
	   ((get_HDMI_HDR_mode() == HDR_DOLBY_HDMI) || (get_HDMI_HDR_mode() == HDR_HDR10_HDMI))) {
		//not force i3ddma case
		if (i3ddmaCtrl.cap_buffer[1].cache == NULL ){
			malloccacheaddr_metadata = (unsigned long)dvr_malloc_uncached_specific(size_metadata*2, GFP_DCU1, (void **)&i3ddmaCtrl.cap_buffer[1].uncache);//(unsigned int)dvr_malloc(size_metadata*2);//for double buffer use
			if(malloccacheaddr_metadata == (unsigned long)NULL)
			{
				rtd_pr_i3ddma_emerg("[DolbyVision] alloc idma sequence metadata memory failed[%lx]\n", malloccacheaddr_metadata);
				return 1; //malloc memory fail
			}

			// buffer[1] -- metadata capture buffer
			i3ddmaCtrl.cap_buffer[1].cache= (void*)(malloccacheaddr_metadata);
			i3ddmaCtrl.cap_buffer[1].phyaddr =  (unsigned int)dvr_to_phys((void*)i3ddmaCtrl.cap_buffer[1].cache);
			i3ddmaCtrl.cap_buffer[1].size = size_metadata;
			rtd_pr_i3ddma_debug("[DolbyVision][Meta] buf[1]=%lx(%d KB), phy(%lx) \n", (unsigned long)i3ddmaCtrl.cap_buffer[1].cache, i3ddmaCtrl.cap_buffer[1].size >> 10, (unsigned long)i3ddmaCtrl.cap_buffer[1].phyaddr);

			// buffer[3] -- not used yet
			i3ddmaCtrl.cap_buffer[3].cache= (void*)(malloccacheaddr_metadata + size_metadata);
			i3ddmaCtrl.cap_buffer[3].phyaddr =  (unsigned int)dvr_to_phys((void*)i3ddmaCtrl.cap_buffer[3].cache);
			i3ddmaCtrl.cap_buffer[3].size = size_metadata;
			rtd_pr_i3ddma_debug("[DolbyVision][Meta] buf[3]=%lx(%d KB), phy(%lx) \n", (unsigned long)i3ddmaCtrl.cap_buffer[3].cache, i3ddmaCtrl.cap_buffer[3].size >> 10, (unsigned long)i3ddmaCtrl.cap_buffer[3].phyaddr);
	    }

		//metadate boundary protection
		rtd_outl(H3DDMA_CAP3_CTI_DMA_WR_Rule_check_low_reg, i3ddmaCtrl.cap_buffer[1].phyaddr); //low limit
		rtd_outl(H3DDMA_CAP3_CTI_DMA_WR_Rule_check_up_reg, i3ddmaCtrl.cap_buffer[3].phyaddr + size_metadata);//high limit
	}
#endif
#if 0
	for (i=0; i<4; i++)
	{
		if ((i3ddmaCtrl.cap_buffer[i].cache == NULL)&&(i3ddmaCtrl.cap_buffer[i].phyaddr == NULL)) {
			rtd_pr_i3ddma_emerg("[DolbyVision] malloc HDMI buffer fail .......................................\n");
			i3ddmaCtrl.i3ddma_3d_capability = 0;
			return 1;
		}
	}
#endif
#if 0//def CONFIG_FORCE_RUN_I3DDMA
		i3ddmaCtrl.i3ddma_3d_capability = 1;
#endif

	return 0;
}


I3DDMA_DolbyVision_HDMI_Init_Fn I3DDMA_DolbyVision_HDMI_Init = I3DDMA_DolbyVision_HDMI_Init_impl;
#ifndef BUILD_QUICK_SHOW
void I3DDMA_DolbyVision_buf_setting(void)
{
	rtd_outl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg, metadataCtrl.cap_buffer[0].phyaddr);
	rtd_outl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_r0_reg, metadataCtrl.cap_buffer[1].phyaddr);
	rtd_outl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_1_reg, metadataCtrl.cap_buffer[2].phyaddr);
	rtd_outl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_r1_reg, metadataCtrl.cap_buffer[3].phyaddr);
}

unsigned char I3DDMA_DolbyVision_MetaData_Init(void) {
	unsigned int size_metadata = 1024 * 1024; // 1MB for metadata buffer
	unsigned int size_video = 3840 * 40 * 3 ;//RGB444 8bit sequence buffer
	int i;
    int getSize = 0;
    unsigned long malloccacheaddr_video_phyaddr = 0;
	unsigned long malloccacheaddr_metadata;
#ifdef CONFIG_VBM_HEAP
	unsigned long dmabuf_hndl = 0;
#endif

	if(metadataCtrl.i3ddma_3d_capability == 1)
	{
		rtd_pr_i3ddma_emerg("metadata memory have exsit, so return!\n");
		return 0;
	}
	malloccacheaddr_metadata = (unsigned long)dvr_malloc_uncached_specific(size_metadata*2, GFP_DCU1, NULL);//(unsigned int)dvr_malloc(size_metadata*2);//for double buffer use
	//unsigned int malloccacheaddr_video = (unsigned int)dvr_malloc_uncached_specific(size_video*2, GFP_DCU2_FIRST, NULL);//(unsigned int)dvr_malloc(size_video*2);//for double buffer use

	rtd_pr_i3ddma_debug("%s.%d\n", __FUNCTION__, __LINE__);
	if(malloccacheaddr_metadata == (unsigned long)NULL/*|| (malloccacheaddr_video == (unsigned int)NULL)*/)
	{
		rtd_pr_i3ddma_emerg("[DolbyVision] alloc idma sequence metadata memory failed[%lx]\n", malloccacheaddr_metadata);
		return 1; //malloc memory fail
	}
    rtd_pr_i3ddma_debug("[DolbyVision][%s] call VBM_BorrowMemChunk\n",__FUNCTION__);
#ifdef CONFIG_VBM_HEAP
	if (VBM_BorrowMemChunk(&dmabuf_hndl, (unsigned long *)(&malloccacheaddr_video_phyaddr), (unsigned long *)(&getSize), size_video*2, 0) != 0) {
#else
	if (VBM_BorrowMemChunk((unsigned long *)(&malloccacheaddr_video_phyaddr), (unsigned long *)(&getSize), size_video*2, 0) != 0) {
#endif
		rtd_pr_i3ddma_emerg("[DolbyVision] alloc idma video sequence memory failed[%lx]\n",  malloccacheaddr_video_phyaddr);
		return 1; //malloc memory fail
	}

	/*
	rtd_outl(H3DDMA_Cap_L1_Start_reg, cap_buffer[0].phyaddr);
	rtd_outl(H3DDMA_Cap_L2_Start_reg, cap_buffer[2].phyaddr);
	rtd_outl(H3DDMA_Cap_R1_Start_reg, cap_buffer[1].phyaddr);
	rtd_outl(H3DDMA_Cap_R2_Start_reg, cap_buffer[3].phyaddr);
	*/

	// buffer[0][2] -- video capture buffer
	metadataCtrl.cap_buffer[0].cache= NULL;//(void*)(malloccacheaddr_video);
	metadataCtrl.cap_buffer[0].phyaddr = (unsigned int)malloccacheaddr_video_phyaddr; //(unsigned int)dvr_to_phys((void*)metadataCtrl.cap_buffer[0].cache);
	metadataCtrl.cap_buffer[0].size = size_video;
	metadataCtrl.cap_buffer[0].getsize = getSize;
#ifdef CONFIG_VBM_HEAP
	metadataCtrl.cap_buffer[0].dmabuf_hndl = dmabuf_hndl;
#endif

	rtd_pr_i3ddma_debug("[DolbyVision][Video] buf[0]=%lx(%d KB), phy(%lx) \n", (unsigned long)metadataCtrl.cap_buffer[0].cache, metadataCtrl.cap_buffer[0].size >> 10, (unsigned long)metadataCtrl.cap_buffer[0].phyaddr);

	metadataCtrl.cap_buffer[2].cache= NULL;//(void*)(malloccacheaddr_video + size_video);
	metadataCtrl.cap_buffer[2].phyaddr = (unsigned int)malloccacheaddr_video_phyaddr + size_video;//(unsigned int)dvr_to_phys((void*)metadataCtrl.cap_buffer[2].cache);
	metadataCtrl.cap_buffer[2].size = size_video;
	metadataCtrl.cap_buffer[2].getsize = getSize;
	rtd_pr_i3ddma_debug("[DolbyVision][Video] buf[2]=%lx(%d KB), phy(%lx) \n", (unsigned long)metadataCtrl.cap_buffer[2].cache, metadataCtrl.cap_buffer[2].size >> 10, (unsigned long)metadataCtrl.cap_buffer[2].phyaddr);

	// buffer[1] -- metadata capture buffer
	metadataCtrl.cap_buffer[1].cache= (void*)(malloccacheaddr_metadata);
	metadataCtrl.cap_buffer[1].phyaddr =  (unsigned int)dvr_to_phys((void*)metadataCtrl.cap_buffer[1].cache);
	metadataCtrl.cap_buffer[1].size = size_metadata;
	rtd_pr_i3ddma_debug("[DolbyVision][Meta] buf[1]=%lx(%d KB), phy(%lx) \n", (unsigned long)metadataCtrl.cap_buffer[1].cache, metadataCtrl.cap_buffer[1].size >> 10, (unsigned long)metadataCtrl.cap_buffer[1].phyaddr);

	// buffer[3] -- not used yet
	metadataCtrl.cap_buffer[3].cache= (void*)(malloccacheaddr_metadata + size_metadata);
	metadataCtrl.cap_buffer[3].phyaddr =  (unsigned int)dvr_to_phys((void*)metadataCtrl.cap_buffer[3].cache);
	metadataCtrl.cap_buffer[3].size = size_metadata;
	rtd_pr_i3ddma_debug("[DolbyVision][Meta] buf[3]=%lx(%d KB), phy(%lx) \n", (unsigned long)metadataCtrl.cap_buffer[3].cache, metadataCtrl.cap_buffer[3].size >> 10, (unsigned long)metadataCtrl.cap_buffer[3].phyaddr);

	metadataCtrl.i3ddma_3d_capability = 1;
	for (i=0; i<4; i++)
	{
		if ((metadataCtrl.cap_buffer[i].cache == NULL)&&(metadataCtrl.cap_buffer[i].phyaddr == 0)) {
			rtd_pr_i3ddma_emerg("[DolbyVision] malloc HDMI buffer fail .......................................\n");
			metadataCtrl.i3ddma_3d_capability = 0;
			return 1;
		}
	}


	return 0;
}

unsigned char I3DDMA_DolbyVision_MetaData_DeInit(void){
	int i;
	if ((metadataCtrl.cap_buffer[0].cache)||(metadataCtrl.cap_buffer[0].phyaddr))
	{
		//dvr_free(metadataCtrl.cap_buffer[0].cache);
		if(metadataCtrl.cap_buffer[0].phyaddr)
		{
		    rtd_pr_i3ddma_debug("[DolbyVision][%s] call VBM_ReturnMemChunk\n",__FUNCTION__);
#ifdef CONFIG_VBM_HEAP
			VBM_ReturnMemChunk(metadataCtrl.cap_buffer[0].dmabuf_hndl, metadataCtrl.cap_buffer[0].phyaddr,metadataCtrl.cap_buffer[0].getsize);
#else
			VBM_ReturnMemChunk(metadataCtrl.cap_buffer[0].phyaddr,metadataCtrl.cap_buffer[0].getsize);
#endif
		}
	}

	if (metadataCtrl.cap_buffer[1].cache)
	{
		dvr_free(metadataCtrl.cap_buffer[1].cache);
	}

	for (i=0; i<4; i++) {
		if ((metadataCtrl.cap_buffer[i].cache)||(metadataCtrl.cap_buffer[i].phyaddr)) {
			metadataCtrl.cap_buffer[i].cache = NULL;
			metadataCtrl.cap_buffer[i].phyaddr = (unsigned long)NULL;
			metadataCtrl.cap_buffer[i].size = 0;
			metadataCtrl.cap_buffer[i].getsize = 0;
#ifdef CONFIG_VBM_HEAP
			metadataCtrl.cap_buffer[i].dmabuf_hndl = 0;
#endif
		}
	}
	metadataCtrl.i3ddma_3d_capability= 0;
	rtd_pr_i3ddma_debug("[DolbyVision] Deinit done!\n");

	return 0;
}


unsigned char I3DDMA_3D_Init(void) {
	unsigned int size = 1920 * 1080 * 4 ;//up 444 10bit sequence buffer
	int i;
	unsigned long malloccacheaddr = (unsigned long)dvr_malloc(size*4);//for double buffer use

	if(malloccacheaddr == (unsigned long)NULL)
	{
		rtd_pr_i3ddma_debug("alloc idma sequence memory failed\n");
		return 1;
	}

	for (i=0; i<4; i++) {
		if(i3ddmaCtrl.cap_buffer[i].cache == NULL){
			i3ddmaCtrl.cap_buffer[i].cache= (void*)(malloccacheaddr+i*size);
			i3ddmaCtrl.cap_buffer[i].phyaddr =  (unsigned int)dvr_to_phys((void*)i3ddmaCtrl.cap_buffer[i].cache);
			i3ddmaCtrl.cap_buffer[i].size = size;
			rtd_pr_i3ddma_debug("[HDMI-3D] buf[%d]=%lx(%d KB), phy(%lx) \n", i, (unsigned long)i3ddmaCtrl.cap_buffer[i].cache, i3ddmaCtrl.cap_buffer[i].size >> 10, (unsigned long)i3ddmaCtrl.cap_buffer[i].phyaddr);
		}

		if (i3ddmaCtrl.cap_buffer[i].cache == NULL) {
			rtd_pr_i3ddma_debug("malloc HDMI buffer fail .......................................\n");
			i3ddmaCtrl.i3ddma_3d_capability = 0;
			return 1;
		}
	}

	return 0;
}

unsigned char I3DDMA_3DDMAInit(void) {
	unsigned int size = 1920 * 1080 * 4 * 2;//up 422 sequence buffer 4k2k
	int i;
	unsigned long malloccacheaddr = (unsigned long)dvr_malloc_uncached_specific(size*2, GFP_DCU2_FIRST, NULL);

	if(malloccacheaddr == (unsigned long)NULL)
	{
		rtd_pr_i3ddma_debug("alloc idma sequence memory failed\n");
		return 1;
	}

	for (i=0; i<2; i++) {
		if(i3ddmaCtrl.cap_buffer[i].cache == NULL){
			i3ddmaCtrl.cap_buffer[i].cache= (void*)(malloccacheaddr+i*size);
			//i3ddmaCtrl.cap_buffer[i].phyaddr =  (unsigned int)virt_to_phys((void*)i3ddmaCtrl.cap_buffer[i].cache);
			i3ddmaCtrl.cap_buffer[i].phyaddr =  (unsigned int)dvr_to_phys((void*)i3ddmaCtrl.cap_buffer[i].cache);
			i3ddmaCtrl.cap_buffer[i].size = size;
			rtd_pr_i3ddma_debug("[HDMI-3D] buf[%d]=%lx(%d KB), phy(%lx) \n", i, (unsigned long)i3ddmaCtrl.cap_buffer[i].cache, i3ddmaCtrl.cap_buffer[i].size >> 10, (unsigned long)i3ddmaCtrl.cap_buffer[i].phyaddr);
		}

		if (i3ddmaCtrl.cap_buffer[i].cache == NULL) {
			rtd_pr_i3ddma_debug("malloc HDMI buffer fail .......................................\n");
			i3ddmaCtrl.i3ddma_3d_capability = 0;
			return 1;
		}
	}

	return 0;
}


void I3DDMA_DolbyVision_HDMI_DeInit(void) {
	int i;

	if ((i3ddmaCtrl.cap_buffer[0].cache)|| (i3ddmaCtrl.cap_buffer[0].phyaddr))
	{
		//dvr_free(i3ddmaCtrl.cap_buffer[0].cache);
		if(i3ddmaCtrl.cap_buffer[0].phyaddr)
		{
		    rtd_pr_i3ddma_debug("[DolbyVision][%s] call VBM_ReturnMemChunk\n",__FUNCTION__);
#ifdef CONFIG_VBM_HEAP
			VBM_ReturnMemChunk(i3ddmaCtrl.cap_buffer[0].dmabuf_hndl, i3ddmaCtrl.cap_buffer[0].phyaddr_unalign, i3ddmaCtrl.cap_buffer[0].getsize);
#else
			VBM_ReturnMemChunk(i3ddmaCtrl.cap_buffer[0].phyaddr_unalign, i3ddmaCtrl.cap_buffer[0].getsize);
#endif
		}
	}

	if (i3ddmaCtrl.cap_buffer[1].cache)
	{
		dvr_free(i3ddmaCtrl.cap_buffer[1].cache);
	}

	for (i=0; i<4; i++) {
		if ((i3ddmaCtrl.cap_buffer[i].cache)||(i3ddmaCtrl.cap_buffer[i].phyaddr)) {
			i3ddmaCtrl.cap_buffer[i].cache = NULL;
			i3ddmaCtrl.cap_buffer[i].phyaddr = (unsigned long)NULL;
			i3ddmaCtrl.cap_buffer[i].phyaddr_unalign = (unsigned long)NULL;
			i3ddmaCtrl.cap_buffer[i].size = 0;
			i3ddmaCtrl.cap_buffer[i].getsize = 0;//test
			i3ddmaCtrl.cap_buffer[i].uncache = 0;
		}
	}

	h3ddma_free_4k120_se_buffer();

	i3ddmaCtrl.i3ddma_3d_capability= 0;
	rtd_pr_i3ddma_debug("[DolbyVision] Deinit done!\n");
}


void I3DDMA_3DDMADeInit(void) {
	int i;
	if (i3ddmaCtrl.cap_buffer[0].cache)
	{
		dvr_free(i3ddmaCtrl.cap_buffer[0].cache);
	}
	for (i=0; i<2; i++) {
		if (i3ddmaCtrl.cap_buffer[i].cache) {
			//pli_freeContinuousMemory(i3ddmaCtrl.cap_buffer[i].cache);
			i3ddmaCtrl.cap_buffer[i].cache = NULL;
			i3ddmaCtrl.cap_buffer[i].phyaddr = (unsigned long)NULL;
			i3ddmaCtrl.cap_buffer[i].size = 0;
		}
	}
	i3ddmaCtrl.i3ddma_3d_capability= 0;
	rtd_pr_i3ddma_debug("[HDMI-3D] Deinit done!\n");
}
#endif
//[ECN][RL6543-17]
#define VODMA_VSTART_PREREAD_LINE 5

extern int Get_GamemodeSaveI3ddmaGetcurMode(void);
extern unsigned int VSYNCGEN_get_vodma_clk_0(void);
unsigned char drv_I3ddmaVodma_GameMode_iv2pv_delay(unsigned char enable){

	INT32 id2pv_delay_1 = 0, id2pv_delay_2 = 0;
//	UINT32 timeout;
#ifdef BUILD_QUICK_SHOW
	vodma_vodma_pvs0_gen_RBUS vodma_vodma_pvs0_gen_Reg;
#endif
	UINT32 voHtotal, voVactive, voVstart, VoClock;
	UINT32 i3ddmaVtotal = 0, i3ddmaVstart = 0, i3ddmaVactive = 0, i3ddmaVfreq = 0;
	unsigned long long temp1;
	vodma_vodma_read_start_RBUS vodma_vodma_read_start_reg;
	unsigned char ret;
	SLR_VOINFO *pVOInfo = NULL;

	//rtd_pr_i3ddma_info("\r\n####func:%s start line:%d####%d\r\n", __FUNCTION__, __LINE__,enable);
	//rtd_pr_i3ddma_info("[qiangzhou]SLR_INPUT_MODE_CURR = %d\n",Get_GamemodeSaveI3ddmaGetcurMode());
	/*all i3ddma source  vodma will sync i3ddma, so all i3ddma source, need setting gamemode iv2pv delay*/
	if((get_hdmi_4k_hfr_mode() == HDMI_4K120_2_1) || (get_force_hdmi_hdr_flow_enable(SLR_MAIN_DISPLAY) == true))
    {
		return FALSE;//avoid crash
    }

	if (enable == _ENABLE){
		/*Modify to use formula to calculate the iv2pv delay @Crixus 20161130*/
		voHtotal = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_H_LEN);
		voVactive = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_LEN);
		voVstart = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_IPV_ACT_STA);

		pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
		if (pVOInfo == NULL) {
			rtd_pr_i3ddma_info("[%s][WARNING] VO info is null!!!\n",__FUNCTION__);
			return FALSE;
		}
#ifdef BUILD_QUICK_SHOW
        voHtotal = pVOInfo->h_total;
        voVactive = pVOInfo->v_length;
        voVstart = pVOInfo->v_start;
#endif
		if(Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC)) {
			VoClock = VSYNCGEN_get_vodma_clk_0();
		}
		else
			VoClock = pVOInfo->pixel_clk;

		if(i3ddmaCtrl.pgen_timing == NULL)
		{
			rtd_pr_i3ddma_emerg("[i3ddma][%s]timing is null,return\n",__FUNCTION__);
			return FALSE;
		}
		i3ddmaVtotal = i3ddmaCtrl.pgen_timing->v_total;
		//i3ddmaHtotal = i3ddmaCtrl.pgen_timing->h_total;
		i3ddmaVstart = i3ddmaCtrl.pgen_timing->v_act_sta;
		i3ddmaVactive = i3ddmaCtrl.pgen_timing->v_act_len;
		i3ddmaVfreq = i3ddmaCtrl.pgen_timing->v_freq;
		if ((get_current_driver_pattern(DRIVER_FREERUN_PATTERN) == 1)
		    && (get_hdmi_4k_hfr_mode() == HDMI_NON_4K120)
		    && (!(vbe_disp_get_VRR_timingMode_flag() || vbe_disp_get_freesync_mode_flag()))
		    && (drv_memory_get_game_mode() == _FALSE)) {//fix 4k max size
			voVactive = (pVOInfo->src_v_len*68)/100;
			i3ddmaVstart = i3ddmaVstart+ (pVOInfo->src_v_len*30)/100;
			voVstart=voVstart+ (pVOInfo->src_v_len*30)/100;
		}
		rtd_pr_i3ddma_info("[iv2dv_dly][%d,%d,%d,%d,%d,%d,%d,%d]\n",voHtotal, voVactive, voVstart, VoClock,i3ddmaVtotal, i3ddmaVstart, i3ddmaVactive, i3ddmaVfreq);
		/*if(i3ddmaVfreq % 10 > 4)
			i3ddmaVfreq = i3ddmaVfreq / 10 + 1;
		else
			i3ddmaVfreq = i3ddmaVfreq / 10;*/

		vodma_vodma_read_start_reg.regValue = IoReg_Read32(VODMA_VODMA_READ_START_reg);
		vodma_vodma_read_start_reg.line_cnt = VODMA_VSTART_PREREAD_LINE;
		IoReg_Write32(VODMA_VODMA_READ_START_reg, vodma_vodma_read_start_reg.regValue);

		//rtd_pr_i3ddma_emerg("[crixus]VoClock = %d, voHtotal = %d, voVstart = %d, voVactive = %d\n", VoClock, voHtotal, voVstart, voVactive);
		//rtd_pr_i3ddma_emerg("[crixus]i3ddmaVtotal = %d, i3ddmaVstart = %d, i3ddmaVactive = %d, i3ddmaVfreq = %d\n", i3ddmaVtotal, i3ddmaVstart, i3ddmaVactive, i3ddmaVfreq);
		/*
			T1: i3ddma act start
			T2: vodma act start
			N: DDR writing time
			Note: ECN for vodma_pre_read 0xb8005144[11:0]
			(before ecn : vodma pre_read = 0)

			formula:
			         T1 <= T2
			         => 1/i3ddma_clock * i3ddma_Htotal * (i3ddma_Vstart + N) <= 1/vo_clock * vo_Htotal * (iv2pv_delay+vodma pre_read)

			T1 = 1/(i3ddma_Htotal*i3ddma_Vtotal*i3ddma_freq) * i3ddma_Htotal * (i3ddma_Vstart + N)
				   = 1/(i3ddma_Vtotal*i3ddma_freq) * (i3ddma_Vstart + N)
				   = (i3ddma_Vstart + N) / (i3ddma_Vtotal*i3ddma_freq)

			T1 <= T2
			=> T1 <= 1/vo_clock * vo_Htotal * iv2pv_delay
			=> T1 * (vo_clock / vo_Htotal) <= iv2pv_delay
			=> iv2pv_delay >= T1 * (vo_clock / vo_Htotal);
		*/
		temp1 = (unsigned long long)(i3ddmaVstart + I3DDMA_DRAM_MARGIN) * 10000000;
		do_div(temp1 , (i3ddmaVtotal * i3ddmaVfreq));
		//rtd_pr_i3ddma_emerg("[crixus]1.temp1 = %lld\n", temp1);

		temp1 = temp1 * (unsigned long long)(VoClock / voHtotal);
		do_div(temp1, 10000);/*Modify to use VO real clock to calculate @Crixus 20161230*/
		if((int)temp1 % 100 > 0){
			do_div(temp1,100);
			temp1 = temp1 + 1;
		}
		else{
			do_div(temp1,100);
		}
		//rtd_pr_i3ddma_emerg("[crixus]2.temp1 = %lld\n", temp1);

		id2pv_delay_1 = (int)temp1 + VODMA_IV2PV_DELAY_CAL_MARGIN - VODMA_VSTART_PREREAD_LINE;
		//rtd_pr_i3ddma_emerg("[crixus]id2pv_delay_1 line = %d\n", id2pv_delay_1);

		/*
			T3: i3ddma act end
			T4: vodma act end

			formula:
			         T3 <= T4
			         => 1/i3ddma_clock * i3ddma_Htotal * (i3ddma_Vactive + i3ddma_Vstart + N) <= 1/vo_clock * vo_Htotal * (vo_Vsatrt + vo_Vactive + iv2pv_delay)

			T3 = 1/(i3ddma_Htotal*i3ddma_Vtotal*i3ddma_freq) * i3ddma_Htotal * (i3ddma_Vactive + i3ddma_Vstart + N)
				   = 1/(i3ddma_Vtotal*i3ddma_freq) * (i3ddma_Vactive + i3ddma_Vstart + N)
				   = (i3ddma_Vactive + i3ddma_Vstart + N) / (i3ddma_Vtotal*i3ddma_freq)
			iv2pv_delay >= temp1 * (vo_clock / vo_Htotal) - (vo_Vsatrt + vo_Vactive);
		*/

		temp1 = (unsigned long long)(i3ddmaVactive + i3ddmaVstart + I3DDMA_DRAM_MARGIN) * 10000000;
		do_div(temp1, (i3ddmaVtotal * i3ddmaVfreq));
		//rtd_pr_i3ddma_emerg("[crixus]1.temp1 = %lld\n", temp1);

		temp1 = temp1 * (unsigned long long)(VoClock / voHtotal);
		do_div(temp1,10000);
		if((int)temp1 % 100 > 0){
			do_div(temp1,100);
			temp1 = temp1 + 1;
		}
		else{
			do_div(temp1,100);
		}
		temp1 = temp1 - (unsigned long long)(voVactive + voVstart);/*Modify to use VO real clock to calculate @Crixus 20161230*/
		//rtd_pr_i3ddma_emerg("[crixus]2.temp1 = %lld\n", temp1);

		id2pv_delay_2 = (int)temp1 + VODMA_IV2PV_DELAY_CAL_MARGIN;
		//rtd_pr_i3ddma_emerg("[crixus]id2pv_delay_2 line = %d\n", id2pv_delay_2);

		if(id2pv_delay_1 > id2pv_delay_2)
			id2pv_delay_1 = id2pv_delay_1 * Scaler_DispGetInputInfo(SLR_INPUT_H_LEN);
		else
			id2pv_delay_1 = id2pv_delay_2 * Scaler_DispGetInputInfo(SLR_INPUT_H_LEN);
		//rtd_pr_i3ddma_emerg("[crixus]id2pv_delay = 0x%x\n", id2pv_delay_1);
	}
	else{
		id2pv_delay_1 = 2;//default setting
	}

#if 0
	//AV need fine tune iv2pv delay
	if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_AVD)  && (get_AVD_Input_Source() != _SRC_TV)){
		if((i3ddmaVfreq > 595) && (i3ddmaVfreq < 605)){
			//normal mode 60Hz case, iv2pv = i3_vtotal * i3_htotal
			id2pv_delay_1 = i3ddmaVtotal * i3ddmaHtotal;
		}
		else if((i3ddmaVfreq > 495) && (i3ddmaVfreq < 505)){
			//normal mode 50Hz case, iv2pv = original iv2pv + 200 line (fine tune)
			id2pv_delay_1 = id2pv_delay_1 + (200 * i3ddmaHtotal);
		}
	}
#endif
#ifdef BUILD_QUICK_SHOW
    rtd_pr_i3ddma_info("[%s %d]id2pv_delay_1:0x%x\n", __FUNCTION__, __LINE__, id2pv_delay_1);
	vodma_vodma_pvs0_gen_Reg.regValue = IoReg_Read32(VODMA_VODMA_PVS0_Gen_reg);
	vodma_vodma_pvs0_gen_Reg.iv2pv_dly = id2pv_delay_1;
	IoReg_Write32(VODMA_VODMA_PVS0_Gen_reg, vodma_vodma_pvs0_gen_Reg.regValue);
#else
	rtd_pr_i3ddma_debug("[Game Mode] i3ddma vodma iv2pv delay = %x\n", id2pv_delay_1);
	if (0 != (ret = Scaler_SendRPC(SCALERIOC_SET_IV2PV_DELAY, id2pv_delay_1, 1))){	//info video fw change iv2pv_delay
		rtd_pr_i3ddma_err("ret=%d, SCALERIOC_SET_IV2PV_DELAY RPC fail !!!\n", ret);
	}
#endif
    return TRUE;
}
#if 0
void drv_AV_GameMode_reset_iv2pv_delay(unsigned int iv2dv_delay)
{
	UINT32 i3ddmaHtotal, i3ddmaVtotal;
	UINT32 iv2pv_delay = 0;
	UINT32 dCLK,dHtotal,dVtotal,VO_framerate;
	unsigned char ret;

	i3ddmaVtotal = i3ddmaCtrl.pgen_timing->v_total;
	i3ddmaHtotal = i3ddmaCtrl.pgen_timing->h_total;
	dCLK = Get_DISPLAY_CLOCK_TYPICAL();
	dHtotal = Get_DISP_HORIZONTAL_TOTAL();
	VO_framerate = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);

	if(VO_framerate == 0){
		rtd_pr_i3ddma_err("[%s] Invalid VO_framerate %d, set to default 60Hz!!!\n",__FUNCTION__,VO_framerate);
		VO_framerate = 600;
	}
	dVtotal = dCLK/dHtotal/(VO_framerate/10);
	//rtd_pr_i3ddma_emerg("[Game Mode][%s][%d] dCLK = %d, dHtotal = %d, VO_framerate = %d\n",__FUNCTION__,__LINE__,dCLK,dHtotal,VO_framerate);
	//rtd_pr_i3ddma_emerg("[Game Mode][%s][%d] iv2dv_delay = %d, i3ddmaHtotal = %d, i3ddmaVtotal = %d, dHtotal = %d, dVtotal = %d\n",__FUNCTION__,__LINE__,iv2dv_delay,i3ddmaHtotal,i3ddmaVtotal,dHtotal,dVtotal);

	if((VO_framerate > 595) && (VO_framerate < 605)){
		//60Hz case, iv2pv = (iv2dv /dv_total)*i3_vtotal, margin = 3
		iv2pv_delay = ( iv2dv_delay * i3ddmaVtotal / dVtotal ) + 3;
		//rtd_pr_i3ddma_emerg("[GAME][%s][%d] iv2pv_delay line = %d\n",__FUNCTION__,__LINE__,iv2pv_delay);
		iv2pv_delay = iv2pv_delay * i3ddmaHtotal;
	}
	else if((VO_framerate > 495) && (VO_framerate < 505)){
		//50Hz case, iv2pv = i3d_vtotal - 41 line (fine tune)
		iv2pv_delay = ((i3ddmaVtotal - 41) * i3ddmaHtotal);
	}

	//rtd_pr_i3ddma_emerg("[Game Mode][%s][%d] iv2pv_delay = %d\n",__FUNCTION__,__LINE__,iv2pv_delay);

	if (0 != (ret = Scaler_SendRPC(SCALERIOC_SET_IV2PV_DELAY, iv2pv_delay, 1))){	//info video fw change iv2pv_delay
		rtd_pr_i3ddma_err("ret=%d, SCALERIOC_SET_IV2PV_DELAY RPC fail !!!\n", ret);
	}
}
#endif
extern StructSrcRect get_main_input_size(void);
unsigned int drv_I3ddmaVodma_get_vodma_overscan_shift_data(void)
{
	StructSrcRect main_input_size;
	unsigned int data_bit_width;
	unsigned int vpos;
	unsigned int videoWidth;
	unsigned int shift_pixel;
	unsigned int shift_vertical_data;
	unsigned int temp_size = 0;

	main_input_size = get_main_input_size();

	//check i3ddma timing gen is ready
	if(i3ddmaCtrl.pgen_timing == NULL){
		rtd_pr_i3ddma_emerg("[%s] i3ddma pgen_timing == NULL \n", __FUNCTION__);
		return 0;
	}

	//VOColorMap size = 4
	if(i3ddmaCtrl.pgen_timing->color >= I3DDMA_COLOR_UNKNOW){
		rtd_pr_i3ddma_emerg("[%s] over color size (%d)\n", __FUNCTION__, i3ddmaCtrl.pgen_timing->color);
		return 0;
	}

	if(i3ddmaCtrl.pgen_timing->progressive == 0){
		vpos = main_input_size.srcy/2;
	}else{
		vpos = main_input_size.srcy;
	}
	videoWidth = i3ddmaCtrl.pgen_timing->h_act_len;

	//plane->shift_vertical_data = vpos * ((config_hdmi_info_save.data_bit_width * (plane->displayWin.width/4) * shift_pixel + 127) / 128)*16;
	data_bit_width = (i3ddmaCtrl.pgen_timing->depth == I3DDMA_COLOR_DEPTH_8B)? 8: 10;

	if (VOColorMap[i3ddmaCtrl.pgen_timing->color] == VO_CHROMA_YUV420) {
		shift_pixel = 6;
	} else if (VOColorMap[i3ddmaCtrl.pgen_timing->color] == VO_CHROMA_YUV422) {
		shift_pixel = 8;
	} else if (VOColorMap[i3ddmaCtrl.pgen_timing->color] == VO_CHROMA_YUV444) {
		shift_pixel = 12;
	} else if (VOColorMap[i3ddmaCtrl.pgen_timing->color] == VO_CHROMA_ARGB8888) {
		shift_pixel = 16;
	} else if (VOColorMap[i3ddmaCtrl.pgen_timing->color] == VO_CHROMA_RGB888) {
		shift_pixel = 12;
	} else {
		shift_pixel = 12;
	}
	if(dvrif_i3ddma_compression_get_enable())
	{
		if(videoWidth % 0x20 != 0)
			videoWidth = (videoWidth + 0x20) & ~0x1F;
		data_bit_width = dvrif_i3ddma_get_compression_bits();
		temp_size = videoWidth * data_bit_width + 256;
		temp_size = (temp_size + 127) / 128;
		shift_vertical_data = vpos*temp_size*16;
	}
	else
		shift_vertical_data = vpos * ((data_bit_width * (videoWidth/4) * shift_pixel + 127) / 128) * 16;
	//rtd_pr_i3ddma_emerg("[%s] main_input_size (%d.%d.%d.%d) \n", __FUNCTION__, main_input_size.srcx, main_input_size.srcy, main_input_size.src_wid, main_input_size.src_height);
	//rtd_pr_i3ddma_emerg("[%s] vpos=%d, progressive = %d\n", __FUNCTION__, vpos, i3ddmaCtrl.pgen_timing->progressive);
	//rtd_pr_i3ddma_emerg("[%s] videoWidth=%d\n", __FUNCTION__, videoWidth);
	//rtd_pr_i3ddma_emerg("[%s] data_bit_width=%d, targetDepth=%d\n", __FUNCTION__, data_bit_width, i3ddmaCtrl.targetDepth);
	//rtd_pr_i3ddma_emerg("[%s] shift_pixel=%d, VOColorMap=%d, color=%d\n", __FUNCTION__, shift_pixel, VOColorMap[i3ddmaCtrl.pgen_timing->color], i3ddmaCtrl.pgen_timing->color);
	//rtd_pr_i3ddma_emerg("[%s] shift_vertical_data=%d\n", __FUNCTION__, shift_vertical_data);
	return shift_vertical_data;
}

extern unsigned char get_AVD_Input_Source(void);
void drv_I3ddmaVodma_SingleBuffer_GameMode(unsigned char enable)
{
	StructSrcRect main_input_size;
	unsigned int shift_vertical_data = 0;
	VSC_INPUT_TYPE_T srctype;
	unsigned char ch = Scaler_Get_CurVoInfo_plane();
	unsigned int v_len;//,interlace;
	extern StructDisplayInfo *Get_VO_Dispinfo(unsigned char ch);
	int block_offset=0;
	vodma_vodma_i2rnd_fifo_th_RBUS vodma_vodma_i2rnd_fifo_th_Reg;
	vodma_vodma_i2rnd_dma_info_RBUS vodma_vodma_i2rnd_dma_info_Reg;
	h3ddma_i3ddma_enable_RBUS  i3ddma_i3ddma_enable_reg;

	vodma_vodma_i2rnd_fifo_th_Reg.regValue = IoReg_Read32(VODMA_vodma_i2rnd_fifo_th_reg);
	vodma_vodma_i2rnd_dma_info_Reg.regValue = 0;

	rtd_pr_i3ddma_notice("[%s] enable=%d\n", __FUNCTION__, enable);
	if(get_current_driver_pattern(DRIVER_FREERUN_PATTERN)){
		if ((Get_VO_Dispinfo(ch)->disp_status&_BIT8)>>8) { // interlace
			v_len=(Get_VO_Dispinfo(ch)->IPV_ACT_LEN_PRE<<1);
			//interlace=1;
		} else {
			v_len=Get_VO_Dispinfo(ch)->IPV_ACT_LEN_PRE;
			//interlace=0;
		}

		main_input_size = get_main_input_size();

		rtd_pr_i3ddma_notice("main_input_size.src_height= %d, v_len = %d \n", main_input_size.src_height, v_len);

		//vo not do overscan
		if((main_input_size.src_height != v_len) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_HDMI)){
			return;
		}

		srctype = Get_DisplayMode_Src(SLR_MAIN_DISPLAY);
		if((srctype != VSC_INPUTSRC_VDEC) && (srctype != VSC_INPUTSRC_JPEG) ){
			//vodma do overscan, so the vo display buffer should cut overscan address.
			shift_vertical_data = drv_I3ddmaVodma_get_vodma_overscan_shift_data();
			rtd_pr_i3ddma_notice("[%s] shift_vertical_data=%d\n", __FUNCTION__, shift_vertical_data);
		}
	}

    // vflip buffer start address offset control
    if(Get_PANEL_VFLIP_ENABLE() && i3ddmaCtrl.pgen_timing){
        unsigned int TotalBits;
        unsigned char data_bit_width, chroma_bit_width;
        data_bit_width = i3ddmaCtrl.pgen_timing->depth == I3DDMA_COLOR_DEPTH_8B? 8: 10;
        chroma_bit_width = VOColorMap[i3ddmaCtrl.pgen_timing->color] == VO_CHROMA_YUV422? 2: 3;
        TotalBits = i3ddmaCtrl.pgen_timing->h_act_len * chroma_bit_width * data_bit_width ;
        block_offset = (TotalBits * (i3ddmaCtrl.pgen_timing->v_act_len - 1) ) / 8;

        rtd_pr_i3ddma_debug("[Game Mode][VFlip] H/V=%dx%d, depth/chroma/size=%d/%d/%d\n",
            i3ddmaCtrl.pgen_timing->h_act_len, i3ddmaCtrl.pgen_timing->v_act_len,
            data_bit_width, chroma_bit_width, TotalBits);

        if(!enable){
            rtd_pr_i3ddma_debug("[Game Mode][VFlip] buffer[2]=%x->%x+%d\n", IoReg_Read32(VODMA_VODMA_V1_SEQ_3D_L2_reg), (unsigned int)i3ddmaCtrl.cap_buffer[2].phyaddr, block_offset);
        }
    }
#if 0
    else
        rtd_pr_i3ddma_debug("[Game Mode] VFLip/PGen=%d/%x\n", Get_PANEL_VFLIP_ENABLE(), (unsigned int)i3ddmaCtrl.pgen_timing);
#endif

	if(enable == TRUE){
		IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_1_reg, i3ddmaCtrl.cap_buffer[0].phyaddr);
		IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_low_1_reg, i3ddmaCtrl.cap_buffer[0].phyaddr); //cap1 low limit
		IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_up_1_reg, i3ddmaCtrl.cap_buffer[0].phyaddr + i3ddmaCtrl.cap_buffer[0].size); //cap1 up limit
		#if 1 //def CONFIG_ALL_SOURCE_DATA_FS
		if(Get_PANEL_VFLIP_ENABLE() && i3ddmaCtrl.pgen_timing)
		    IoReg_Write32(VODMA_VODMA_V1_SEQ_3D_L2_reg, i3ddmaCtrl.cap_buffer[0].phyaddr + block_offset - shift_vertical_data);
		else
		    IoReg_Write32(VODMA_VODMA_V1_SEQ_3D_L2_reg, i3ddmaCtrl.cap_buffer[0].phyaddr + block_offset + shift_vertical_data);
		#else
			IoReg_Write32(VODMA_VODMA_V1_SEQ_3D_L2_reg, i3ddmaCtrl.cap_buffer[0].phyaddr);
		#endif
		//Eric@20180628 i3ddma->vo input fast case, need use new mode.
		//old mode: send flag before capture
		//new mode: send flag after capture done
		i3ddma_i3ddma_enable_reg.regValue = IoReg_Read32(H3DDMA_I3DDMA_enable_reg);
		i3ddma_i3ddma_enable_reg.block_sel_to_flag_fifo_option = 0;
		IoReg_Write32(H3DDMA_I3DDMA_enable_reg, i3ddma_i3ddma_enable_reg.regValue);

		vodma_vodma_i2rnd_fifo_th_Reg.dram_buf_num_i3ddma = 1;
		vodma_vodma_i2rnd_fifo_th_Reg.dma_info_th_dispm = 1;
		vodma_vodma_i2rnd_fifo_th_Reg.dma_info_th_i3ddma = 1;

		vodma_vodma_i2rnd_dma_info_Reg.dma_info_rptr_fw_set_dispm = 1;
		vodma_vodma_i2rnd_dma_info_Reg.dma_info_rptr_fw_set_i3ddma = 1;
		vodma_vodma_i2rnd_dma_info_Reg.dma_info_wptr_fw_set_dispm = 1;
		vodma_vodma_i2rnd_dma_info_Reg.dma_info_wptr_fw_set_i3ddma = 1;
	}else if(i3ddmaCtrl.cap_buffer[2].phyaddr){
		IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_1_reg, i3ddmaCtrl.cap_buffer[2].phyaddr);
		IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_low_1_reg, i3ddmaCtrl.cap_buffer[2].phyaddr); //cap1 low limit
		IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_up_1_reg, i3ddmaCtrl.cap_buffer[2].phyaddr + i3ddmaCtrl.cap_buffer[2].size); //cap1 up limit
		#if 1 //def CONFIG_ALL_SOURCE_DATA_FS
		if(Get_PANEL_VFLIP_ENABLE() && i3ddmaCtrl.pgen_timing)
		    IoReg_Write32(VODMA_VODMA_V1_SEQ_3D_L2_reg, i3ddmaCtrl.cap_buffer[2].phyaddr + block_offset - shift_vertical_data);
		else
		    IoReg_Write32(VODMA_VODMA_V1_SEQ_3D_L2_reg, i3ddmaCtrl.cap_buffer[2].phyaddr + block_offset + shift_vertical_data);
		#else
			IoReg_Write32(VODMA_VODMA_V1_SEQ_3D_L2_reg, i3ddmaCtrl.cap_buffer[2].phyaddr);
		#endif
		//Eric@20180628 i3ddma->vo input fast case, need use new mode.
		//old mode: send flag before capture
		//new mode: send flag after capture done
		i3ddma_i3ddma_enable_reg.regValue = IoReg_Read32(H3DDMA_I3DDMA_enable_reg);
		i3ddma_i3ddma_enable_reg.block_sel_to_flag_fifo_option = 1;
		IoReg_Write32(H3DDMA_I3DDMA_enable_reg, i3ddma_i3ddma_enable_reg.regValue);

		vodma_vodma_i2rnd_fifo_th_Reg.dram_buf_num_i3ddma = 3;
		vodma_vodma_i2rnd_fifo_th_Reg.dma_info_th_dispm = 1;
		vodma_vodma_i2rnd_fifo_th_Reg.dma_info_th_i3ddma = 1;

		vodma_vodma_i2rnd_dma_info_Reg.dma_info_rptr_fw_set_dispm = 1;
		vodma_vodma_i2rnd_dma_info_Reg.dma_info_rptr_fw_set_i3ddma = 1;
		vodma_vodma_i2rnd_dma_info_Reg.dma_info_wptr_fw_set_dispm = 1;
		vodma_vodma_i2rnd_dma_info_Reg.dma_info_wptr_fw_set_i3ddma = 1;
	}
	IoReg_Write32(VODMA_vodma_i2rnd_fifo_th_reg, vodma_vodma_i2rnd_fifo_th_Reg.regValue);
	IoReg_Write32(VODMA_vodma_i2rnd_dma_info_reg, vodma_vodma_i2rnd_dma_info_Reg.regValue);
	rtd_pr_i3ddma_debug("[Game Mode] i3ddma and vodma game mode done enable=%d!!, L2=%x\n",enable, IoReg_Read32(VODMA_VODMA_V1_SEQ_3D_L2_reg));
}
#if defined CONFIG_SCALER_BRING_UP || defined BUILD_QUICK_SHOW
extern int VODMA_ConfigHDMI(VIDEO_RPC_VOUT_CONFIG_HDMI_3D *HDMI_INFO);
#endif

#ifdef CONFIG_ENABLE_HDMI_NN
void I3DDMA_ResetCap0(void)
{
	h3ddma_cap0_cti_dma_wr_ctrl_RBUS h3ddma_cap0_cti_dma_wr_ctrl_reg;

	h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_force_all_rst = 1;
	IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue);
	rtd_pr_i3ddma_emerg("0   0xb8025b0c=%x\n", IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg));

	mdelay(2);
	h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_force_all_rst = 0;
	IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue);
	rtd_pr_i3ddma_emerg("1   0xb8025b0c=%x\n", IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg));
}

void I3DDMA_ResetCap1(void)
{
	h3ddma_cap1_cti_dma_wr_ctrl_RBUS h3ddma_cap1_cti_dma_wr_ctrl_reg;
	vgip_vgip_chn1_ctrl_RBUS vgip_vgip_chn1_ctrl_reg;
	vgip_vgip_chn1_delay_RBUS vgip_vgip_chn1_delay_reg;

	rtd_pr_i3ddma_emerg("I3DDMA_ResetCap1\n");

	h3ddma_cap1_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP1_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap1_cti_dma_wr_ctrl_reg.cap1_force_all_rst = 1;
	IoReg_Write32(H3DDMA_CAP1_CTI_DMA_WR_Ctrl_reg, h3ddma_cap1_cti_dma_wr_ctrl_reg.regValue);
	rtd_pr_i3ddma_emerg("0   0xb8025b8c=%x\n", IoReg_Read32(H3DDMA_CAP1_CTI_DMA_WR_Ctrl_reg));

	//mdelay(2);
	//h3ddma_cap1_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP1_CTI_DMA_WR_Ctrl_reg);
	//h3ddma_cap1_cti_dma_wr_ctrl_reg.cap1_force_all_rst = 0;
	//IoReg_Write32(H3DDMA_CAP1_CTI_DMA_WR_Ctrl_reg, h3ddma_cap1_cti_dma_wr_ctrl_reg.regValue);
	//rtd_pr_i3ddma_emerg("1   0xb8025b8c=%x\n", IoReg_Read32(H3DDMA_CAP1_CTI_DMA_WR_Ctrl_reg));


	// Main vgip change to HDMI source
	vgip_vgip_chn1_ctrl_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);
	vgip_vgip_chn1_ctrl_reg.ch1_in_sel = 0;
	IoReg_Write32(VGIP_VGIP_CHN1_CTRL_reg, vgip_vgip_chn1_ctrl_reg.regValue);

	// Set ivs delay = 2
	vgip_vgip_chn1_delay_reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_DELAY_reg);
	vgip_vgip_chn1_delay_reg.ch1_ivs_dly = 2;
	IoReg_Write32(VGIP_VGIP_CHN1_DELAY_reg, vgip_vgip_chn1_delay_reg.regValue);

}
#endif

VIDEO_RPC_VOUT_CONFIG_HDMI_3D *Get_Val_GetI3DDMAConfigData_impl(unsigned long vir_addr_noncache)
{
    return (VIDEO_RPC_VOUT_CONFIG_HDMI_3D *)vir_addr_noncache;
}
VIDEO_RPC_VOUT_CONFIG_HDMI_3D *(*GetI3DDMAConfigData)(unsigned long vir_addr_noncache) = Get_Val_GetI3DDMAConfigData_impl;

char I3DDMA_SetupVODMA(I3DDMA_TIMING_T *timing,unsigned char display) {
#if 0//[mark by crixus-mac3 linux based]
	VIDEO_RPC_VOUT_CONFIG_HDMI_3D config;
	CLNT_STRUCT clnt;

	rtd_pr_i3ddma_info("%s\n", __FUNCTION__);
	clnt = prepareCLNT(BLOCK_MODE | USE_INTR_BUF | SEND_VIDEO_CPU, VIDEO_SYSTEM, VERSION);

	memset(&config, 0, sizeof(VIDEO_RPC_VOUT_CONFIG_HDMI_3D));
	config.chroma_fmt = VOColorMap[timing->color];
	config.width = timing->h_act_len;
	config.height = timing->v_act_len;
	config.framerate = timing->v_freq * 100;

	// [IDMA] interlaced video data FRC in M-domain, IDMA VO go auto mode
	if((timing->progressive == 0)|| (drvif_scaler3d_decide_3d_SG_data_FRC() == TRUE))
		config.hw_auto_mode = 1;
	else
		config.hw_auto_mode = 0;

/*
	if (timing->i3ddma_3dformat == I3DDMA_SIDE_BY_SIDE_HALF) {
		config.quincunx_en = timing->quincunx_en;
		config.quincunx_mode_fw = timing->quincunx_mode;
	} else {
		config.quincunx_en = 0;
	}
*/
	config.quincunx_en = 0;

	config.l1_st_adr = rtd_inl(I3DDMA_CAP_L1_START_VADDR);
	config.r1_st_adr = rtd_inl(I3DDMA_CAP_R1_START_VADDR);
	config.l2_st_adr = rtd_inl(I3DDMA_CAP_L2_START_VADDR);
	config.r2_st_adr = rtd_inl(I3DDMA_CAP_R2_START_VADDR);
	config.progressive = timing->progressive;
	rtd_pr_i3ddma_info("~~~~~~ I3DDMA_SetupVODMA ~~~ progressive (%d) \n", config.progressive);
	config.videoPlane = VO_VIDEO_PLANE_V1;
	config.data_bit_width = (timing->depth == I3DDMA_COLOR_DEPTH_8B)? 8: 10;
	config.sensio3D_en = (timing->i3ddma_3dformat == I3DDMA_SENSIO? 1: 0);
	config.sg3d_dataFrc = drvif_scaler3d_decide_3d_SG_data_FRC()|| drvif_scaler3d_decide_3d_PR_enable_IDMA();

	HRESULT* hr = VIDEO_RPC_VOUT_ToAgent_ConfigHDMI_3D_0(&config, &clnt);
	if (hr < 0) {
		rtd_pr_i3ddma_info("CONFIG HDMI 3D RPC fail\n");
		return FALSE;
	}
	free(hr);
#endif
#if 1

#ifndef CONFIG_SCALER_BRING_UP
#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
	unsigned long result = 0;
#endif
#endif
	VIDEO_RPC_VOUT_CONFIG_HDMI_3D *config_data;
#ifndef BUILD_QUICK_SHOW
	//struct page *page = 0;
	//page = alloc_page(GFP_KERNEL);
	//config_datatmp = (VIDEO_RPC_VOUT_SET_3D_MODE *)page_address(page);
	unsigned long vir_addr, vir_addr_noncache = 0;
	unsigned int phy_addr;

	vir_addr = (unsigned long)dvr_malloc_uncached_specific(sizeof(VIDEO_RPC_VOUT_CONFIG_HDMI_3D), GFP_DCU1, (void **)&vir_addr_noncache);
	phy_addr = (unsigned int)dvr_to_phys((void*)vir_addr);

	config_data = GetI3DDMAConfigData(vir_addr_noncache);
#else
    VIDEO_RPC_VOUT_CONFIG_HDMI_3D localConfigData = {0};
    config_data = &localConfigData;
#endif

	memset(config_data, 0, sizeof(VIDEO_RPC_VOUT_CONFIG_HDMI_3D));
	config_data->width = timing->h_act_len;
	config_data->height = timing->v_act_len;
	config_data->h_start = timing->h_act_sta;
	config_data->v_start = timing->v_act_sta;
	if(timing->color < I3DDMA_COLOR_UNKNOW)	//Klocwork: buffer may overflow.
		config_data->chroma_fmt = VOColorMap[timing->color];
	else
		config_data->chroma_fmt = VOColorMap[I3DDMA_COLOR_RGB];
	config_data->data_bit_width = (timing->depth == I3DDMA_COLOR_DEPTH_8B)? 8: 10;
	config_data->framerate =  timing->v_freq * 100;
	config_data->framerate_ori = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ) * 100;
    config_data->srcframerate =  timing->src_v_freq * 100;
	config_data->progressive = timing->progressive;
	config_data->l1_st_adr = rtd_inl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg);
	config_data->r1_st_adr = rtd_inl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_r0_reg);
	config_data->l2_st_adr = rtd_inl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_1_reg);
	config_data->r2_st_adr = rtd_inl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_r1_reg);
	config_data->l3_st_adr = rtd_inl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_2_reg);//input fast use 3-buffer @Crixus 20170508
	config_data->r3_st_adr = rtd_inl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_r2_reg);
	//config_data->hw_auto_mode = 1;
	// Dolby Vision force in FW mode
#ifdef CONFIG_FORCE_RUN_I3DDMA
#if 0
	if(drvif_i3ddma_triplebuf_flag())
		config_data->hw_auto_mode = 0;
	else
		config_data->hw_auto_mode = 1;
#endif
	config_data->hw_auto_mode = 1;
#else
	config_data->hw_auto_mode = 1;//((get_HDMI_HDR_mode() == HDR_DOLBY_HDMI)|| (get_HDMI_HDR_mode() == HDR_HDR10_HDMI)? 0: 1);
#endif
	config_data->quincunx_en = 0;
	config_data->videoPlane = VO_VIDEO_PLANE_V1;
	config_data->i2rnd_sub_src = _DISABLE;

#ifdef CONFIG_I2RND_ENABLE
	//force cmd use s0 diaply idx
	config_data->i2rnd_display_table = I2RND_MAIN_S0_TABLE;
#else
	config_data->i2rnd_display_table = I2RND_TABLE_MAX;
#endif

	config_data->htotal = timing->h_total;
	config_data->vtotal = timing->v_total;
	config_data->enable_comps = dvrif_i3ddma_compression_get_enable();
	if(dvrif_i3ddma_compression_get_enable() == TRUE)
	{
		config_data->comps_bits = dvrif_i3ddma_get_compression_bits();//dvrif_i3ddma_get_compression_totalsize();
		config_data->comps_line_mode = dvrif_i3ddma_get_compression_mode();//default use line mode
	}
	else
	{
		config_data->comps_bits =  0;
		config_data->comps_line_mode = 0;
	}
	//fix me and recover later scaler_overscan_vertical_size(display,config_data->progressive,config_data->height,&config_data->overscan_v_len,&config_data->overscan_v_start);
	if (Get_DisplayMode_Src(display) == VSC_INPUTSRC_HDMI) {
		config_data->hdr_source = drvif_Hdmi_IsDRMInfoReady();
		config_data->vrr_en = vbe_disp_get_VRR_timingMode_flag();
		config_data->freesync_en = vbe_disp_get_freesync_mode_flag();
	} else {
		config_data->hdr_source =0;
		config_data->vrr_en = 0;
		config_data->freesync_en = 0;
	}
#ifndef BUILD_QUICK_SHOW
	if(get_i3ddma_4k120_flag() == TRUE)
	{
		config_data->hdmi_4k120_flag = 1;
		config_data->hw_auto_mode = 0;
		config_data->width = timing->h_act_len * 2;
		config_data->height = timing->v_act_len;
		config_data->r1_st_adr = g_4K120_SE_Buffer[0].phyaddr; //send se_addr for 4k120
	}
	else
	{
		config_data->hdmi_4k120_flag = 0;
	}
#endif
#if 0
	mdelay(100);
	i3ddma_dump_data_to_file(0);
#endif

#if 0 //for dump debug
	if(get_i3ddma_4k120_flag() == TRUE){
		mdelay(100);
		i3ddma_dump_cap0cap2_to_file(0);
	}
#endif

#if defined CONFIG_SCALER_BRING_UP || defined BUILD_QUICK_SHOW
	rtd_pr_i3ddma_info("width = %d\n", config_data->width);
	rtd_pr_i3ddma_info("height = %d\n", config_data->height);
	rtd_pr_i3ddma_info("progressive = %d\n", config_data->progressive);
	rtd_pr_i3ddma_info("framerate = %d\n", config_data->framerate);
	rtd_pr_i3ddma_info("framerate_ori = %d\n", config_data->framerate_ori);
	rtd_pr_i3ddma_info("chroma_fmt = %d\n", config_data->chroma_fmt);
	rtd_pr_i3ddma_info("data_bit_width = %d\n", config_data->data_bit_width);
	rtd_pr_i3ddma_info("hw_auto_mode = %d\n", config_data->hw_auto_mode);
	rtd_pr_i3ddma_info("l1_sta_adr = %x\n", config_data->l1_st_adr);
	rtd_pr_i3ddma_info("r1_sta_adr = %x\n", config_data->r1_st_adr);
	rtd_pr_i3ddma_info("l2_sta_adr = %x\n", config_data->l2_st_adr);
	rtd_pr_i3ddma_info("r2_sta_adr = %x\n", config_data->r2_st_adr);
	rtd_pr_i3ddma_info("quincunx_en = %d\n", config_data->quincunx_en);
	rtd_pr_i3ddma_info("quincunx_mode_fw = %d\n", config_data->quincunx_mode_fw);
	rtd_pr_i3ddma_info("sensio3D_en = %d\n", config_data->sensio3D_en);
	rtd_pr_i3ddma_info("sg3d_dataFrc = %d\n", config_data->sg3d_dataFrc);
	rtd_pr_i3ddma_info("i2rnd_sub_src = %d\n", config_data->i2rnd_sub_src);
	rtd_pr_i3ddma_info("i2rnd_display_table = %d\n", config_data->i2rnd_display_table);
	rtd_pr_i3ddma_info("htotal = %d\n", config_data->htotal);
	rtd_pr_i3ddma_info("vtotal = %d\n", config_data->vtotal);
	rtd_pr_i3ddma_info("hw_auto_mode = %d\n", config_data->hw_auto_mode);
    rtd_pr_i3ddma_info("srcframerate = %d\n", config_data->srcframerate);
	VODMA_ConfigHDMI(config_data);
#else

	config_data->width = htonl(config_data->width);
	config_data->height = htonl(config_data->height);
	config_data->chroma_fmt = htonl(config_data->chroma_fmt);
	config_data->data_bit_width = htonl(config_data->data_bit_width);
	config_data->framerate =  htonl(config_data->framerate);
	config_data->framerate_ori =  htonl(config_data->framerate_ori);
    config_data->srcframerate =  htonl(config_data->srcframerate);
	config_data->progressive = htonl(config_data->progressive);
	config_data->l1_st_adr = htonl(config_data->l1_st_adr);
	config_data->r1_st_adr = htonl(config_data->r1_st_adr);
	config_data->l2_st_adr = htonl(config_data->l2_st_adr);
	config_data->r2_st_adr = htonl(config_data->r2_st_adr);
	config_data->l3_st_adr = htonl(config_data->l3_st_adr);//input fast use 3-buffer @Crixus 20170508
	config_data->r3_st_adr = htonl(config_data->r3_st_adr);
	config_data->hw_auto_mode = htonl(config_data->hw_auto_mode);
	config_data->quincunx_en = htonl(config_data->quincunx_en);
	config_data->videoPlane = htonl(config_data->videoPlane);
	config_data->i2rnd_sub_src = htonl(config_data->i2rnd_sub_src);
	config_data->i2rnd_display_table = htonl(config_data->i2rnd_display_table);
	config_data->htotal = htonl(config_data->htotal);
	config_data->vtotal = htonl(config_data->vtotal);
	config_data->h_start = htonl(config_data->h_start);
	config_data->v_start = htonl(config_data->v_start);
	config_data->enable_comps = htonl(config_data->enable_comps);
	config_data->comps_bits = htonl(config_data->comps_bits);
	config_data->comps_line_mode = htonl(config_data->comps_line_mode);
	config_data->overscan_v_len = htonl(config_data->overscan_v_len);
	config_data->overscan_v_start = htonl(config_data->overscan_v_start);
	config_data->hdr_source = htonl(config_data->hdr_source);
	config_data->hdmi_4k120_flag = htonl(config_data->hdmi_4k120_flag);
	config_data->vrr_en = htonl(config_data->vrr_en);
	config_data->freesync_en = htonl(config_data->freesync_en);

#if IS_ENABLED(CONFIG_RTK_KDRV_RPC)
#ifndef CONFIG_SCALER_BRING_UP
	if (RPC_FAIL==send_rpc_command(RPC_VIDEO, VIDEO_RPC_VOUT_ToAgent_ConfigHDMI_3D,phy_addr, 0, &result))
	{
		rtd_pr_i3ddma_debug("RPC fail!!\n");
		dvr_free((void *)vir_addr);
		return FALSE;
	}
#endif
#endif
#endif
#ifndef BUILD_QUICK_SHOW
	dvr_free((void *)vir_addr);
#endif
#endif
	return TRUE;

}

//I3DDMA_CAPTURE_BUFFER_T idma_cap_buf[3];
I3DDMA_CAPTURE_BUFFER_T idma_cap_buf[4]; //fix coverity issue,the index must >=4
//#define DISP2TVE_IDMA_CAPSIZE  1920 * 1080 * 3// per byte => 3*8 = 24bits
#define SCART_OUT_TVE_IDMA_CAPSIZE  720 * 576 * 2// per byte => 2*8 = 16bits
#ifndef BUILD_QUICK_SHOW
unsigned char I3DDMA_disp2tve_CapBufInit(void) {

	unsigned int size = SCART_OUT_TVE_IDMA_CAPSIZE;
	unsigned int buffer_size = 0, align_addr = 0, query_addr = 0;

#ifdef I3DDMA_DYNAMIC_MEMORY_ALLOCATE_ENABLE
		// allocate IDMA capture buffer before enable IDMA
		rtd_pr_i3ddma_debug("[IDMA] Dynamic memory alloc!!\n");
		return 0;
#endif

		if(idma_cap_buf[0].phyaddr == 0)
		{
				size = drvif_memory_get_data_align(size, DMA_SPEEDUP_ALIGN_VALUE);

				buffer_size = dvr_size_alignment_cma(size*4 + 512);
				/*Use pli memory allocation @Crixus 20150801*/
				//idma_cap_buf[0].phyaddr=pli_malloc((size*4 + 512), GFP_DCU2_FIRST);

				query_addr = pli_malloc(buffer_size, GFP_DCU2_FIRST);
				align_addr = dvr_memory_alignment((unsigned long)query_addr, buffer_size);

				idma_cap_buf[0].phyaddr = align_addr;
				idma_cap_buf[0].phyaddr_unalign = query_addr;

				if (idma_cap_buf[0].phyaddr == INVALID_VAL) {
					rtd_pr_i3ddma_debug("malloc idma buffer fail .......................................\n");
					return 1;
				}

				idma_cap_buf[0].size = size;

				rtd_pr_i3ddma_debug("[idma-cap-buf] buf[0] phy=%x(%d KB)\n", (UINT32)idma_cap_buf[0].phyaddr, idma_cap_buf[0].size >> 10);

				idma_cap_buf[1].phyaddr = idma_cap_buf[0].phyaddr+ idma_cap_buf[0].size;
				idma_cap_buf[1].size = size;
				rtd_pr_i3ddma_debug("[idma-cap-buf] buf[1] phy=%x(%d KB)\n", (UINT32)idma_cap_buf[1].phyaddr, idma_cap_buf[1].size >> 10);

				idma_cap_buf[2].phyaddr = idma_cap_buf[1].phyaddr+ idma_cap_buf[1].size;
				idma_cap_buf[2].size = size;
				rtd_pr_i3ddma_debug("[idma-cap-buf] buf[2] phy=%x(%d KB)\n", (UINT32)idma_cap_buf[2].phyaddr, idma_cap_buf[2].size >> 10);

				idma_cap_buf[3].phyaddr = idma_cap_buf[2].phyaddr+ idma_cap_buf[2].size;
				idma_cap_buf[3].size = size;
				rtd_pr_i3ddma_debug("[idma-cap-buf] buf[3] phy=%x(%d KB)\n", (UINT32)idma_cap_buf[3].phyaddr, idma_cap_buf[3].size >> 10);

				rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_low_reg, idma_cap_buf[0].phyaddr );
				rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_up_reg, idma_cap_buf[3].phyaddr + idma_cap_buf[3].size);

		}
	return 0;
}

//add to disconnect api
void I3DDMA_disp2tve_CapBufDeInit(void) {
	if (idma_cap_buf[0].phyaddr) {
		/*Use pli memory allocation @Crixus 20150801*/
		pli_free(idma_cap_buf[0].phyaddr_unalign);
		idma_cap_buf[0].phyaddr = 0;
		idma_cap_buf[0].phyaddr_unalign = 0;

		rtd_pr_i3ddma_debug("[idma-cap-buf] idma cap buffer is not empty!!free the buffer!\n");
	}

	rtd_pr_i3ddma_debug("[idma-cap-buf] Deinit done!\n");
}
#endif
void gamemode_delaytime_print_msg(void){
#ifndef UT_flag
	UINT32 voHtotal;
	UINT32 voVactive;
	UINT32 VoClock;
	UINT32 i3ddmaVtotal;
	UINT32 i3ddmaHtotal;
	UINT32 i3ddmaVfreq;
	UINT32 i3ddmaClock;
	UINT32 i3ddmaVsta;
	UINT32 iv2dv_delay;
	UINT32 iv2pv_delay;
	SLR_VOINFO *pVOInfo = NULL;
	vodma_vodma_pvs0_gen_RBUS vodma_vodma_pvs0_gen_Reg;
//	ppoverlay_fs_iv_dv_fine_tuning5_RBUS fs_iv_dv_fine_tuning5_reg;

	voHtotal = Scaler_DispGetInputInfoByDisp(0, SLR_INPUT_H_LEN);
	voVactive = Scaler_DispGetInputInfoByDisp(0, SLR_INPUT_IPV_ACT_LEN);
	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
	VoClock = pVOInfo->pixel_clk;

	i3ddmaHtotal = i3ddmaCtrl.pgen_timing->h_total;
	i3ddmaVtotal = i3ddmaCtrl.pgen_timing->v_total;
	i3ddmaVfreq = i3ddmaCtrl.pgen_timing->v_freq;
	i3ddmaVsta = i3ddmaCtrl.pgen_timing->v_act_sta;
	i3ddmaClock = ((i3ddmaHtotal*i3ddmaVtotal)/10*i3ddmaVfreq);

	vodma_vodma_pvs0_gen_Reg.regValue = IoReg_Read32(VODMA_VODMA_PVS0_Gen_reg);
	iv2pv_delay = (vodma_vodma_pvs0_gen_Reg.iv2pv_dly)/voHtotal;
	//fs_iv_dv_fine_tuning5_reg.regValue = IoReg_Read32(PPOVERLAY_FS_IV_DV_Fine_Tuning5_reg);
	//iv2dv_delay = fs_iv_dv_fine_tuning5_reg.iv2dv_line;
	if(IoReg_Read32(0xb8022288)&_BIT19){ //for debug only
		iv2dv_delay = drv_GameMode_decided_iv2dv_delay_old_mode();
	}else{
		iv2dv_delay = drv_GameMode_decided_iv2dv_delay_new_mode();
	}

	rtd_pr_i3ddma_emerg(" ===== [GameModeDelayPrint] ======= \n");
	rtd_pr_i3ddma_emerg(" ===== [source] Htotal   : %d ===== \n", i3ddmaHtotal);
	rtd_pr_i3ddma_emerg(" ===== [source] clock    : %d ===== \n", i3ddmaClock);
	rtd_pr_i3ddma_emerg(" ===== [vodma]  voHtotal : %d ===== \n", voHtotal);
	rtd_pr_i3ddma_emerg(" ===== [vodma]  VoClock  : %d ===== \n", VoClock);
	rtd_pr_i3ddma_emerg(" ===== iv2pv_delay       : %d ===== \n", iv2pv_delay);
	rtd_pr_i3ddma_emerg(" ===== iv2dv_delay       : %d ===== \n", iv2dv_delay);
	rtd_pr_i3ddma_emerg(" ===== dma Vgip Vsta     : %d ===== \n", i3ddmaVsta);
	rtd_pr_i3ddma_emerg(" ===== Interlaced        : %d ===== \n", Scaler_DispGetStatus(0, SLR_DISP_INTERLACE));
#endif
}

#ifdef CONFIG_HDR_SDR_SEAMLESS
void backup_hdmi_src_info(I3DDMA_TIMING_T *src_info)
{//backup hdmi timing and color info
	memcpy(&hdmi_source_info_backup, src_info, sizeof(I3DDMA_TIMING_T));
}

I3DDMA_TIMING_T *get_backup_hdmi_info(void)
{//get hdmi backup data
	return &hdmi_source_info_backup;
}

spinlock_t* get_i3ddma_change_spinlock(void)
{//this is for  i3ddma_change_format_flag protection
	return &I3DDMA_CHANGE_SPINLOCK;
}

void set_i3ddma_change_format_flag(unsigned char TorF)
{
	i3ddma_change_format_flag = TorF;
}

unsigned char get_i3ddma_change_format_flag(void)
{
	return i3ddma_change_format_flag;
}


void set_target_i3ddma_color_format(I3DDMA_COLOR_SPACE_T color_format)
{//set final target i3ddma output format
	target_output_color_format = color_format;
}

I3DDMA_COLOR_SPACE_T get_target_i3ddma_color_format(void)
{//get final target i3ddma output format
	return target_output_color_format;
}


I3DDMA_COLOR_SPACE_T decide_i3ddma_output_foramt(I3DDMA_TIMING_T *timing)
{//decide i3ddma output color format
	// In Vertical strip & Checker board 3D, I3DDMA only support YUV444 capture
	if(dvrif_i3ddma_compression_get_enable()){
		return I3DDMA_COLOR_YUV444;
	}
	if((timing->i3ddma_3dformat == I3DDMA_VERTICAL_STRIPE)|| (timing->i3ddma_3dformat == I3DDMA_CHECKER_BOARD))
	{
		return I3DDMA_COLOR_YUV444;
	} // [Dolby Vision] IDMA capture in RGB format
	else if((get_HDMI_HDR_mode() == HDR_DOLBY_HDMI) && (timing->color == I3DDMA_COLOR_RGB)){
		return I3DDMA_COLOR_RGB;
	}
	else if((get_HDMI_HDR_mode() == HDR_HDR10_HDMI) /*&& (i3ddma_timing.color != I3DDMA_COLOR_RGB)*/){
		return I3DDMA_COLOR_YUV422;
	}else if(timing->color == I3DDMA_COLOR_RGB) {
		return I3DDMA_COLOR_YUV444;
	}else {
        // [WOSQRTK-10007][BW] Force enable HDMI 444(420) to 422!!
        if((timing->h_act_len >= 3800) && (timing->v_freq > 480)
            && (timing->color == I3DDMA_COLOR_YUV444) && (timing->depth > 0))
        {
        	return I3DDMA_COLOR_YUV422;
        }else {
			return timing->color;
    	}
	}
}


void trigger_seamless_flow(VSC_INPUT_TYPE_T src, unsigned char hdr_enable, unsigned char hdr_type)
{//enable seamless flow. vide FW is at video
	extern bool get_support_vo_force_v_top(unsigned int func_flag);
	unsigned long flags;//for spin_lock_irqsave
    seamless_change_sync_info *seamless_info_sharememory = NULL;
    //rtd_pr_i3ddma_notice("\r\n####func:% src:%d hdr_type:%d#####\r\n",__FUNCTION__,src, hdr_type);
	seamless_info_sharememory = (seamless_change_sync_info *)Scaler_GetShareMemVirAddr(SCALERIOC_SEAMLESS_CHANGE_SHAREMEMORY_INFO_SYNC_FLAG);

	if(!hdr_enable)
	{//sdr case
		if(get_support_vo_force_v_top(support_sdr_max_rgb))
		{//sdr force vtop maxRGB case
			hdr_enable = 1;
			hdr_type = HDR_DM_SDR_MAX_RGB;
		}
	}

	if(src == VSC_INPUTSRC_HDMI)
    {
    	if(seamless_info_sharememory)
    	{
	    	spin_lock_irqsave(get_i3ddma_change_spinlock(), flags);//lock i3ddma change spinlock
	        seamless_info_sharememory->hdmi_target_hdr_info = (hdr_type << 1) | hdr_enable; //bit0 = enable/disable bit1-9 hdr type
			seamless_info_sharememory->hdmi_target_color_format = target_output_color_format;
			set_i3ddma_change_format_flag(TRUE);
			spin_unlock_irqrestore(get_i3ddma_change_spinlock(), flags);//unlock i3ddma change spinlock
			rtd_pr_i3ddma_info("####func:%s hdr enable:%d type:%d#####\r\n",__FUNCTION__, hdr_enable, hdr_type);
    	}
	}
/* trigger by video fw
    else if(src == VSC_INPUTSRC_VDEC)
    {
    	if(seamless_info_sharememory)
    	{
	    	spin_lock_irqsave(get_vdec_seamless_change_spinlock(), flags);//lock vdec change spinlock
			set_vdec_seamless_change_flag(TRUE);
			seamless_info_sharememory->vdec_target_hdr_enable = hdr_type;
			spin_unlock_irqrestore(get_vdec_seamless_change_spinlock(), flags);//unlock vdec change spinlock
    	}
	}
*/
}


void I3ddma_format_changed_seamless(I3DDMA_TIMING_T *timing, I3DDMA_COLOR_SPACE_T target)
{//seamless change i3ddma capture setting
	h3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_RBUS i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg;
	h3ddma_hsd_i3ddma_vsd_ctrl0_RBUS i3ddma_vsd_ctrl0_reg;
	/*i3ddma capture setting*/
	int rem, len, num;
	unsigned int h_act_len;
	//h3ddma_i3ddma_enable_RBUS  i3ddma_i3ddma_enable_reg;
//	h3ddma_cap0_cap_ctl0_RBUS  i3ddma_cap_ctl0_reg;
//	h3ddma_cap_ctl1_RBUS i3ddma_cap_ctl1_reg;
	h3ddma_cap0_wr_dma_pxltobus_RBUS h3ddma_cap0_wr_dma_pxltobus_reg;

	I3DDMA_COLOR_SPACE_T source = timing->color;

	/*i3ddma 422 444 setting*/
	i3ddma_vsd_ctrl0_reg.regValue = IoReg_Read32(H3DDMA_HSD_I3DDMA_VSD_Ctrl0_reg);
	if(source == I3DDMA_COLOR_YUV422)
		i3ddma_vsd_ctrl0_reg.sort_fmt = 1; //yuv422 bypass (1)
	else if(source == I3DDMA_COLOR_RGB) {
		if(target == I3DDMA_COLOR_YUV422)
	 		i3ddma_vsd_ctrl0_reg.sort_fmt = 0; //444 to 422
	 	else
			i3ddma_vsd_ctrl0_reg.sort_fmt = 1;//keep 444
	} else {//input 444
		if(target == I3DDMA_COLOR_YUV422)
			i3ddma_vsd_ctrl0_reg.sort_fmt = 0; //444 to 422
		else
			i3ddma_vsd_ctrl0_reg.sort_fmt = 1;//keep 444
	}
	IoReg_Write32(H3DDMA_HSD_I3DDMA_VSD_Ctrl0_reg, i3ddma_vsd_ctrl0_reg.regValue);

	/*i3ddma rgb2yuv setting*/
	if (source == I3DDMA_COLOR_RGB) {
		if(DMA_VGIP_DMA_VGIP_CTRL_get_dma_in_sel(rtd_inl(DMA_VGIP_DMA_VGIP_CTRL_reg)) == VGIP_SRC_ATV) {
			/*av source, idma bypass rgb2yuv*/
			i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.regValue = rtd_inl(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_CTRL_reg);
			i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.en_rgb2yuv = 0;
			rtd_outl(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_CTRL_reg, i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.regValue);
		}
		else if(target != I3DDMA_COLOR_RGB) {
			i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.regValue = rtd_inl(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_CTRL_reg);
			i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.en_rgb2yuv = 1;
			i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.sel_rgb = 1;
			i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.set_uv_out_offset = 1;
			rtd_outl(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_CTRL_reg, i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.regValue);
		} else {
			i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.regValue = rtd_inl(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_CTRL_reg);
			i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.en_rgb2yuv = 0;
			i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.sel_rgb = 0;
			i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.set_uv_out_offset = 0;
			rtd_outl(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_CTRL_reg, i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.regValue);
		}
	} else {
		i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.regValue = rtd_inl(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_CTRL_reg);
		i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.en_rgb2yuv = 0;
		i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.sel_rgb = 0;
		i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.set_uv_out_offset = 0;
		rtd_outl(H3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_RGB2YUV_CTRL_reg, i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_rgb2yuv_ctrl_reg.regValue);
	}

	/*i3ddma capture setting*/
	h_act_len  = (DEPTH2BITS[timing->depth] * (timing->h_act_len /4) * components[target] + 127) / 128;
	rem = 12;
	len = 32;
	num = (h_act_len - rem) / len;
	rem = (h_act_len - rem) % len + rem;

	//i3ddma_i3ddma_enable_reg.regValue = rtd_inl(H3DDMA_I3DDMA_enable_reg);
	//i3ddma_i3ddma_enable_reg.cap_en = 0;
	//rtd_outl(H3DDMA_I3DDMA_enable_reg, i3ddma_i3ddma_enable_reg.regValue);

	h3ddma_cap0_wr_dma_pxltobus_reg.regValue = rtd_inl(H3DDMA_CAP0_WR_DMA_pxltobus_reg);
	h3ddma_cap0_wr_dma_pxltobus_reg.cap0_pixel_encoding = encodemap[target];
	rtd_outl(H3DDMA_CAP0_WR_DMA_pxltobus_reg, h3ddma_cap0_wr_dma_pxltobus_reg.regValue);
#if 0
	i3ddma_cap_ctl0_reg.regValue = rtd_inl(H3DDMA_Cap_CTL0_reg);
	i3ddma_cap_ctl0_reg.cap_write_len = len;
	i3ddma_cap_ctl0_reg.cap_write_rem = rem;
	rtd_outl(H3DDMA_Cap_CTL0_reg, i3ddma_cap_ctl0_reg.regValue);

	i3ddma_cap_ctl1_reg.regValue = rtd_inl(H3DDMA_Cap_CTL1_reg);
	i3ddma_cap_ctl1_reg.cap_water_lv = len;
	i3ddma_cap_ctl1_reg.cap_write_num = num;
	rtd_outl(H3DDMA_Cap_CTL1_reg, i3ddma_cap_ctl1_reg.regValue);

	i3ddma_i3ddma_enable_reg.regValue = rtd_inl(H3DDMA_I3DDMA_enable_reg);
	i3ddma_i3ddma_enable_reg.cap_en = 1;
	rtd_outl(H3DDMA_I3DDMA_enable_reg, i3ddma_i3ddma_enable_reg.regValue);
#endif
	return;
}
#endif


static unsigned char enable_idma_compression = FALSE;
static unsigned char idma_compression_bits = 0;
static unsigned char idma_compression_mode = I3DDMA_COMP_NONE_MODE;

static unsigned char dvrif_i3ddma_compression_get_enable_impl(void)
{
	return enable_idma_compression;
}
void dvrif_i3ddma_compression_set_enable(unsigned char enable)
{
	enable_idma_compression = enable;
}

unsigned char (*dvrif_i3ddma_compression_get_enable)(void) = dvrif_i3ddma_compression_get_enable_impl;

unsigned char dvrif_i3ddma_get_compression_bits(void)
{
	return idma_compression_bits;
}
void dvrif_i3ddma_set_compression_bits(unsigned char comp_bits)
{
	idma_compression_bits = comp_bits;
}

unsigned char dvrif_i3ddma_get_compression_mode(void)
{
	return idma_compression_mode;
}

void dvrif_i3ddma_set_compression_mode(unsigned char comp_mode)
{
	idma_compression_mode = comp_mode;
}

#define I3DDMA_PQC_LINE_MARGIN 6
extern unsigned char pc_mode_run_444(void);
void dvrif_i3ddma_comp_setting(UINT8 enable_compression, UINT16 comp_wid, UINT16 comp_len, UINT8 comp_ratio){
	//i3ddma->vodma use PQC and PQDC from merlin3
	h3ddma_h3ddma_pq_cmp_RBUS h3ddma_h3ddma_pq_cmp_reg;
	h3ddma_h3ddma_pq_cmp_pair_RBUS h3ddma_h3ddma_pq_cmp_pair_reg;
	h3ddma_h3ddma_pq_cmp_bit_RBUS h3ddma_h3ddma_pq_cmp_bit_reg;
        //>>>> [start]K5LG-1492:modify for i3ddma and mdomain compression
	h3ddma_h3ddma_pq_cmp_enable_RBUS h3ddma_h3ddma_pq_cmp_enable_reg;
	vodma_vodma_pq_decmp_sat_en_RBUS vodma_vodma_pq_decmp_sat_en_reg;
	h3ddma_h3ddma_pq_cmp_allocate_RBUS h3ddma_h3ddma_pq_cmp_allocate_reg;
	h3ddma_h3ddma_pq_cmp_poor_RBUS h3ddma_h3ddma_pq_cmp_poor_Reg;

	h3ddma_h3ddma_pq_cmp_guarantee_RBUS h3ddma_h3ddma_pq_cmp_guarantee_reg;
	h3ddma_h3ddma_pq_cmp_blk0_add0_RBUS h3ddma_h3ddma_pq_cmp_blk0_add0_reg;
	h3ddma_h3ddma_pq_cmp_blk0_add1_RBUS h3ddma_h3ddma_pq_cmp_blk0_add1_reg;

	h3ddma_h3ddma_pq_cmp_balance_RBUS h3ddma_h3ddma_pq_cmp_balance_reg;
	h3ddma_h3ddma_pq_cmp_smooth_RBUS h3ddma_h3ddma_pq_cmp_smooth_reg;
	//<<<< [end]K5LG-1492:modify for i3ddma and mdomain compression
	unsigned int TotalSize = 0;
	#if 0
	vodma_vodma_pq_decmp_RBUS vodma_vodma_pq_decmp_reg;
	vodma_vodma_pq_decmp_pair_RBUS vodma_vodma_pq_decmp_pair_reg;
	#endif

	rtd_pr_i3ddma_info("enable_compression = %d, comp_wid = %d, comp_len = %d, comp_ratio = %d\n",enable_compression, comp_wid, comp_len, comp_ratio);

	if(enable_compression == TRUE){
		/*
			width need to align 32
		*/

		if((comp_wid % 32) != 0){
			//drop bits
			//IoReg_Write32(MDOMAIN_DISP_DDR_MainAddrDropBits_reg, (32 - (comp_wid % 32)));
			comp_wid = comp_wid + (32 - (comp_wid % 32));
		}

		TotalSize = (comp_wid * comp_ratio + 256) * 10 / 128;
		if((TotalSize % 10) != 0){
			TotalSize = TotalSize / 10 + 1;
		}else{
			TotalSize = TotalSize / 10;
		}

		//i3ddma capture compression setting
		h3ddma_h3ddma_pq_cmp_reg.regValue = IoReg_Read32(H3DDMA_H3DDMA_PQ_CMP_reg);
		h3ddma_h3ddma_pq_cmp_reg.cmp_height = comp_len;//set length
		h3ddma_h3ddma_pq_cmp_reg.cmp_width_div32 = comp_wid / 32;//set width
		IoReg_Write32(H3DDMA_H3DDMA_PQ_CMP_reg, h3ddma_h3ddma_pq_cmp_reg.regValue);

		//compression bits setting
		h3ddma_h3ddma_pq_cmp_bit_reg.regValue = IoReg_Read32(H3DDMA_H3DDMA_PQ_CMP_BIT_reg);
		if(dvrif_i3ddma_get_compression_mode() == I3DDMA_COMP_LINE_MODE)
		{
			h3ddma_h3ddma_pq_cmp_bit_reg.frame_limit_bit = comp_ratio;
			h3ddma_h3ddma_pq_cmp_bit_reg.line_limit_bit = comp_ratio;
		}
		else{//frame mode, line limit bit need to add margin to avoid the peak bandwidth
			h3ddma_h3ddma_pq_cmp_bit_reg.frame_limit_bit = comp_ratio;
			h3ddma_h3ddma_pq_cmp_bit_reg.line_limit_bit = comp_ratio + I3DDMA_PQC_LINE_MARGIN;
		}
		h3ddma_h3ddma_pq_cmp_bit_reg.block_limit_bit = 0x3f;//PQC fine tune setting
		h3ddma_h3ddma_pq_cmp_bit_reg.first_line_more_bit = 8;
		IoReg_Write32(H3DDMA_H3DDMA_PQ_CMP_BIT_reg, h3ddma_h3ddma_pq_cmp_bit_reg.regValue);

		//compression other setting
		h3ddma_h3ddma_pq_cmp_pair_reg.regValue = IoReg_Read32(H3DDMA_H3DDMA_PQ_CMP_PAIR_reg);

#if 0
		if(pc_mode_run_444())
		{//hdmi input yuv444 pc mode
			h3ddma_h3ddma_pq_cmp_pair_reg.cmp_data_bit_width = 1; //10 bits
		} else {
			if(i3ddmaCtrl.pgen_timing->depth == I3DDMA_COLOR_DEPTH_10B)
				h3ddma_h3ddma_pq_cmp_pair_reg.cmp_data_bit_width = 1; //10 bits
			else
				h3ddma_h3ddma_pq_cmp_pair_reg.cmp_data_bit_width = 0;//8 bits
		}
#endif
		h3ddma_h3ddma_pq_cmp_pair_reg.cmp_data_bit_width = 1; //10 bits

		#if 0
		if(i3ddmaCtrl.pgen_timing->color == I3DDMA_COLOR_YUV422){
			//h3ddma_h3ddma_pq_cmp_pair_reg.cmp_data_format = 1;//422
			h3ddma_h3ddma_pq_cmp_pair_reg.cmp_data_color = 1;
		}else{
			//h3ddma_h3ddma_pq_cmp_pair_reg.cmp_data_format = 0;//444
			h3ddma_h3ddma_pq_cmp_pair_reg.cmp_data_color = 1;//currently no case rgb channel
		}
		#endif

		h3ddma_h3ddma_pq_cmp_pair_reg.cmp_data_color = 1;
		h3ddma_h3ddma_pq_cmp_pair_reg.cmp_pair_para = 1;

		if(pc_mode_run_444())
			h3ddma_h3ddma_pq_cmp_pair_reg.cmp_data_format = 0;//444
		else
			h3ddma_h3ddma_pq_cmp_pair_reg.cmp_data_format = 1;//422

		if(dvrif_i3ddma_get_compression_mode() == I3DDMA_COMP_LINE_MODE)
		{
			// use line mode
			h3ddma_h3ddma_pq_cmp_pair_reg.cmp_line_mode = 1;
			h3ddma_h3ddma_pq_cmp_pair_reg.cmp_line_sum_bit = TotalSize;
			//h3ddma_h3ddma_pq_cmp_pair_reg.two_line_prediction_en = 0;
		}
		else
		{
			//use frame mode
			h3ddma_h3ddma_pq_cmp_pair_reg.cmp_line_mode = 0;
			h3ddma_h3ddma_pq_cmp_pair_reg.cmp_line_sum_bit = 0;
			//h3ddma_h3ddma_pq_cmp_pair_reg.two_line_prediction_en = 1;
		}

		if(Get_PANEL_VFLIP_ENABLE()){
			h3ddma_h3ddma_pq_cmp_pair_reg.two_line_prediction_en = 0;
		
        } else if(is_rotate_180_bypass_vi(SLR_MAIN_DISPLAY))
        {
            h3ddma_h3ddma_pq_cmp_pair_reg.two_line_prediction_en = 0;
        } else {
			if ((dvrif_i3ddma_get_compression_mode() == I3DDMA_COMP_LINE_MODE) && (get_current_driver_pattern(DRIVER_FREERUN_PATTERN) == 1)) {
				h3ddma_h3ddma_pq_cmp_pair_reg.two_line_prediction_en = 0;
			} else {
				h3ddma_h3ddma_pq_cmp_pair_reg.two_line_prediction_en = 1;
			}
		}
#if 0
		/*KTASKWBS-9826,K5lP without Vflip ,Tien-Hung recomment even line mode can enable two_line_prediction_en*/
		h3ddma_h3ddma_pq_cmp_pair_reg.two_line_prediction_en = 1;
#endif
		h3ddma_h3ddma_pq_cmp_pair_reg.cmp_qp_mode = 0;
		//h3ddma_h3ddma_pq_cmp_pair_reg.cmp_dic_mode_en = 1;
		IoReg_Write32(H3DDMA_H3DDMA_PQ_CMP_PAIR_reg, h3ddma_h3ddma_pq_cmp_pair_reg.regValue);

                //>>>>[start]K5LG-1492:modify for i3ddma and mdomain compression
		h3ddma_h3ddma_pq_cmp_enable_reg.regValue = IoReg_Read32(H3DDMA_H3DDMA_PQ_CMP_ENABLE_reg);
                //RGB444 PQC mode do_422_mode=2,comment from DIC Tien-Hung
        if(Get_Val_vsc_run_pc_mode()){
			if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_COLOR_SPACE) == I3DDMA_COLOR_YUV422))
			{//hdmi input yuv422 pc mode
				h3ddma_h3ddma_pq_cmp_enable_reg.do_422_mode = 1;//422 format
				h3ddma_h3ddma_pq_cmp_enable_reg.g_ratio = 14;
			}
			else
			{
				h3ddma_h3ddma_pq_cmp_enable_reg.do_422_mode = 2;//444 format
				h3ddma_h3ddma_pq_cmp_enable_reg.g_ratio = 12;
			}
		}
		else{
			h3ddma_h3ddma_pq_cmp_enable_reg.do_422_mode = 1;//422 format
			h3ddma_h3ddma_pq_cmp_enable_reg.g_ratio = 14;
		}
		h3ddma_h3ddma_pq_cmp_enable_reg.first_predict_nc_en = 1;
		h3ddma_h3ddma_pq_cmp_enable_reg.first_predict_nc_mode = 1;
		h3ddma_h3ddma_pq_cmp_enable_reg.guarantee_max_qp_en = 0;
		h3ddma_h3ddma_pq_cmp_enable_reg.fisrt_line_more_en = 1;
		h3ddma_h3ddma_pq_cmp_enable_reg.blk0_add_en = 1;
		h3ddma_h3ddma_pq_cmp_enable_reg.blk0_add_half_en = 1;
		h3ddma_h3ddma_pq_cmp_enable_reg.add_last2blk_over_curve_bit = 1;
		h3ddma_h3ddma_pq_cmp_enable_reg.balance_en = 1;
		h3ddma_h3ddma_pq_cmp_enable_reg.variation_maxmin_en = 1;
		h3ddma_h3ddma_pq_cmp_enable_reg.dynamic_allocate_ratio_en = 1;
		h3ddma_h3ddma_pq_cmp_enable_reg.dynamic_sum_line_rst = 1;
		h3ddma_h3ddma_pq_cmp_enable_reg.not_enough_bit_do_422_en = 1;
		h3ddma_h3ddma_pq_cmp_enable_reg.not_rich_do_422_en = 1;
		h3ddma_h3ddma_pq_cmp_enable_reg.rb_lossy_do_422_en = 1;
		h3ddma_h3ddma_pq_cmp_enable_reg.poor_limit_min_qp_en = 1;
		IoReg_Write32(H3DDMA_H3DDMA_PQ_CMP_ENABLE_reg,h3ddma_h3ddma_pq_cmp_enable_reg.regValue);

		h3ddma_h3ddma_pq_cmp_allocate_reg.regValue = IoReg_Read32(H3DDMA_H3DDMA_PQ_CMP_ALLOCATE_reg);
		//RGB444 setting for PQC ratio_max and ratio_min, comment from DIC Tien-Hung
		if (Get_Val_vsc_run_pc_mode()){
			if((Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_COLOR_SPACE) == I3DDMA_COLOR_YUV422))
			{//hdmi input yuv422 pc mode
				h3ddma_h3ddma_pq_cmp_allocate_reg.dynamic_allocate_ratio_max = 16;
				h3ddma_h3ddma_pq_cmp_allocate_reg.dynamic_allocate_ratio_min = 13;
			}
			else
			{
				h3ddma_h3ddma_pq_cmp_allocate_reg.dynamic_allocate_ratio_max = 13;
				h3ddma_h3ddma_pq_cmp_allocate_reg.dynamic_allocate_ratio_min = 9;
			}
		}
		else{
			h3ddma_h3ddma_pq_cmp_allocate_reg.dynamic_allocate_ratio_max = 16;
			h3ddma_h3ddma_pq_cmp_allocate_reg.dynamic_allocate_ratio_min = 13;
		}

		h3ddma_h3ddma_pq_cmp_allocate_reg.dynamic_allocate_less = 4;
		h3ddma_h3ddma_pq_cmp_allocate_reg.dynamic_allocate_more = 4;
		h3ddma_h3ddma_pq_cmp_allocate_reg.dynamic_allocate_line = 4;
		IoReg_Write32(H3DDMA_H3DDMA_PQ_CMP_ALLOCATE_reg, h3ddma_h3ddma_pq_cmp_allocate_reg.regValue);

		h3ddma_h3ddma_pq_cmp_poor_Reg.regValue=IoReg_Read32(H3DDMA_H3DDMA_PQ_CMP_POOR_reg);

		#if 0
		if(pc_mode_run_444())
		{//hdmi input yuv444 pc mode
			h3ddma_h3ddma_pq_cmp_poor_Reg.poor_limit_th_qp4 = 0x14;
			h3ddma_h3ddma_pq_cmp_poor_Reg.poor_limit_th_qp3 = 0x06;
			h3ddma_h3ddma_pq_cmp_poor_Reg.poor_limit_th_qp2 = 0x03;
			h3ddma_h3ddma_pq_cmp_poor_Reg.poor_limit_th_qp1 = 0x03;
		} else {
			h3ddma_h3ddma_pq_cmp_poor_Reg.poor_limit_th_qp4 = 0x14;
			h3ddma_h3ddma_pq_cmp_poor_Reg.poor_limit_th_qp3 = 0x0c;
			h3ddma_h3ddma_pq_cmp_poor_Reg.poor_limit_th_qp2 = 0x06;
			h3ddma_h3ddma_pq_cmp_poor_Reg.poor_limit_th_qp1 = 0x03;
		}
		#endif

		h3ddma_h3ddma_pq_cmp_poor_Reg.poor_limit_th_qp1 = 3;
		h3ddma_h3ddma_pq_cmp_poor_Reg.poor_limit_th_qp2 = 6;
		h3ddma_h3ddma_pq_cmp_poor_Reg.poor_limit_th_qp3 = 12;
		h3ddma_h3ddma_pq_cmp_poor_Reg.poor_limit_th_qp4 = 20;
		IoReg_Write32(H3DDMA_H3DDMA_PQ_CMP_POOR_reg, h3ddma_h3ddma_pq_cmp_poor_Reg.regValue);

		//default decompress saturation setting as enable,comment from DIC Tien-Hung
		vodma_vodma_pq_decmp_sat_en_reg.regValue=IoReg_Read32(VODMA_VODMA_PQ_DECMP_SAT_EN_reg);
		vodma_vodma_pq_decmp_sat_en_reg.saturation_en=1;

		IoReg_Write32(VODMA_VODMA_PQ_DECMP_SAT_EN_reg,vodma_vodma_pq_decmp_sat_en_reg.regValue);
                //<<<<[end]K5LG-1492:modify for i3ddma and mdomain compression

		#if 0
		//vodma de-compression setting
		vodma_vodma_pq_decmp_reg.regValue = IoReg_Read32(VODMA_VODMA_PQ_DECMP_reg);
		vodma_vodma_pq_decmp_reg.decmp_height = comp_len + 1;
		vodma_vodma_pq_decmp_reg.decmp_width_div32 = comp_wid / 32;
		IoReg_Write32(VODMA_VODMA_PQ_DECMP_reg, vodma_vodma_pq_decmp_reg.regValue);

		//de-compression bits setting
		vodma_vodma_pq_decmp_pair_reg.regValue = IoReg_Read32(VODMA_VODMA_PQ_DECMP_PAIR_reg);
		if(i3ddmaCtrl.pgen_timing->depth == I3DDMA_COLOR_DEPTH_10B)
			vodma_vodma_pq_decmp_pair_reg.decmp_data_bit_width = 1;//10 bits
		else
			vodma_vodma_pq_decmp_pair_reg.decmp_data_bit_width = 0;//8 bits
		//decompression YUV444
		if(i3ddmaCtrl.pgen_timing->color == I3DDMA_COLOR_YUV422){
			vodma_vodma_pq_decmp_pair_reg.decmp_data_format = 1;//422
			vodma_vodma_pq_decmp_pair_reg.decmp_data_color = 1;
		}else{
			vodma_vodma_pq_decmp_pair_reg.decmp_data_format = 0;//444
			vodma_vodma_pq_decmp_pair_reg.decmp_data_color = 1;
		}
		//default use frame mode
		vodma_vodma_pq_decmp_pair_reg.decmp_line_mode = 0;
		IoReg_Write32(VODMA_VODMA_PQ_DECMP_PAIR_reg, vodma_vodma_pq_decmp_pair_reg.regValue);
		#endif
		//enable compression
		h3ddma_h3ddma_pq_cmp_reg.regValue = IoReg_Read32(H3DDMA_H3DDMA_PQ_CMP_reg);
		h3ddma_h3ddma_pq_cmp_reg.cmp_en = 1;
		IoReg_Write32(H3DDMA_H3DDMA_PQ_CMP_reg, h3ddma_h3ddma_pq_cmp_reg.regValue);
		#if 0
		//enable de-compression
		vodma_vodma_pq_decmp_reg.regValue = IoReg_Read32(VODMA_VODMA_PQ_DECMP_reg);
		vodma_vodma_pq_decmp_reg.decmp_en = 1;
		IoReg_Write32(VODMA_VODMA_PQ_DECMP_reg, vodma_vodma_pq_decmp_reg.regValue);
		#endif

		// Guarantee
		h3ddma_h3ddma_pq_cmp_guarantee_reg.regValue = IoReg_Read32(H3DDMA_H3DDMA_PQ_CMP_Guarantee_reg);
		h3ddma_h3ddma_pq_cmp_guarantee_reg.guarantee_max_g_qp = 0;
		h3ddma_h3ddma_pq_cmp_guarantee_reg.guarantee_max_rb_qp = 0;
		// Dictionary
		h3ddma_h3ddma_pq_cmp_guarantee_reg.dic_mode_qp_th_g = 0;
		h3ddma_h3ddma_pq_cmp_guarantee_reg.dic_mode_qp_th_rb = 0;
		IoReg_Write32(H3DDMA_H3DDMA_PQ_CMP_Guarantee_reg, h3ddma_h3ddma_pq_cmp_guarantee_reg.regValue);

		// Quota distribution
		h3ddma_h3ddma_pq_cmp_blk0_add0_reg.regValue = IoReg_Read32(H3DDMA_H3DDMA_PQ_CMP_BLK0_ADD0_reg);
		h3ddma_h3ddma_pq_cmp_blk0_add0_reg.blk0_add_value0 = 1;
		h3ddma_h3ddma_pq_cmp_blk0_add0_reg.blk0_add_value16 = 8;
		h3ddma_h3ddma_pq_cmp_blk0_add0_reg.blk0_add_value32 = 16;

		if(pc_mode_run_444())
		{//hdmi input yuv444 pc mode
			// Dictionary
			h3ddma_h3ddma_pq_cmp_blk0_add0_reg.dic_mode_tolerance = 1;
		}
		else
		{
			// Dictionary
			h3ddma_h3ddma_pq_cmp_blk0_add0_reg.dic_mode_tolerance = 0;
		}
		IoReg_Write32(H3DDMA_H3DDMA_PQ_CMP_BLK0_ADD0_reg, h3ddma_h3ddma_pq_cmp_blk0_add0_reg.regValue);


		h3ddma_h3ddma_pq_cmp_blk0_add1_reg.regValue = IoReg_Read32(H3DDMA_H3DDMA_PQ_CMP_BLK0_ADD1_reg);
		h3ddma_h3ddma_pq_cmp_blk0_add1_reg.blk0_add_value1 = 1;
		h3ddma_h3ddma_pq_cmp_blk0_add1_reg.blk0_add_value2 = 2;
		h3ddma_h3ddma_pq_cmp_blk0_add1_reg.blk0_add_value4 = 4;
		h3ddma_h3ddma_pq_cmp_blk0_add1_reg.blk0_add_value8 = 6;
		IoReg_Write32(H3DDMA_H3DDMA_PQ_CMP_BLK0_ADD1_reg, h3ddma_h3ddma_pq_cmp_blk0_add1_reg.regValue);

		// Balance
		h3ddma_h3ddma_pq_cmp_balance_reg.regValue = IoReg_Read32(H3DDMA_H3DDMA_PQ_CMP_BALANCE_reg);
		h3ddma_h3ddma_pq_cmp_balance_reg.balance_g_give = 3;
		h3ddma_h3ddma_pq_cmp_balance_reg.balance_g_ud_th = 0;
		h3ddma_h3ddma_pq_cmp_balance_reg.balance_g_ov_th = 3;
		h3ddma_h3ddma_pq_cmp_balance_reg.balance_rb_give = 3;
		h3ddma_h3ddma_pq_cmp_balance_reg.balance_rb_ud_th = 0;
		h3ddma_h3ddma_pq_cmp_balance_reg.balance_rb_ov_th = 3;

		// smooth
		h3ddma_h3ddma_pq_cmp_balance_reg.variation_maxmin_th = 30;
		h3ddma_h3ddma_pq_cmp_balance_reg.variation_maxmin_th2 = 3;
		IoReg_Write32(H3DDMA_H3DDMA_PQ_CMP_BALANCE_reg, h3ddma_h3ddma_pq_cmp_balance_reg.regValue);

		// Smooth
		h3ddma_h3ddma_pq_cmp_smooth_reg.regValue = IoReg_Read32(H3DDMA_H3DDMA_PQ_CMP_SMOOTH_reg);
		h3ddma_h3ddma_pq_cmp_smooth_reg.variation_near_num_th = 4;
		h3ddma_h3ddma_pq_cmp_smooth_reg.variation_value_th = 3;
		h3ddma_h3ddma_pq_cmp_smooth_reg.variation_num_th = 3;

		// Dynamically 422 decision
		h3ddma_h3ddma_pq_cmp_smooth_reg.rb_lossy_do_422_qp_level = 0;
		h3ddma_h3ddma_pq_cmp_smooth_reg.not_rich_do_422_th = 4;
		h3ddma_h3ddma_pq_cmp_smooth_reg.not_enough_bit_do_422_qp = 5;

		// DIctionary
		h3ddma_h3ddma_pq_cmp_smooth_reg.dic_mode_entry_th = 15;
		IoReg_Write32(H3DDMA_H3DDMA_PQ_CMP_SMOOTH_reg, h3ddma_h3ddma_pq_cmp_smooth_reg.regValue);
	}
	else{
		//disable compression
		h3ddma_h3ddma_pq_cmp_reg.regValue = IoReg_Read32(H3DDMA_H3DDMA_PQ_CMP_reg);
		h3ddma_h3ddma_pq_cmp_reg.cmp_en = 0;
		IoReg_Write32(H3DDMA_H3DDMA_PQ_CMP_reg, h3ddma_h3ddma_pq_cmp_reg.regValue);

		#if 0
		//disable de-compression
		vodma_vodma_pq_decmp_reg.regValue = IoReg_Read32(VODMA_VODMA_PQ_DECMP_reg);
		vodma_vodma_pq_decmp_reg.decmp_en = 0;
		IoReg_Write32(VODMA_VODMA_PQ_DECMP_reg, vodma_vodma_pq_decmp_reg.regValue);
		#endif

	}
}

void drvif_scaler_i3ddma_vodma_fifo_input_fast(unsigned char display, unsigned int input_frame_rate, unsigned char interlace){
       vodma_vodma_i2rnd_fifo_th_RBUS vodma_vodma_i2rnd_fifo_th_reg;
       vodma_vodma_i2rnd_fifo_th_reg.regValue = IoReg_Read32(VODMA_vodma_i2rnd_fifo_th_reg);

       //rtd_pr_i3ddma_emerg("[crixus][%s]display = %d, input_frame_rate = %d\n", __FUNCTION__, display, input_frame_rate);
       // decide input fast mode
       if(display == SLR_MAIN_DISPLAY){
		vodma_vodma_i2rnd_fifo_th_reg.flag_fifo_style_i3ddma = ((input_frame_rate > 600)? 1: 0);
		vodma_vodma_i2rnd_fifo_th_reg.flag_fifo_fast_int_i3ddma = (((input_frame_rate > 600) && interlace)? 1: 0);//merlin4 can support interlace faster mode
       }else{
		vodma_vodma_i2rnd_fifo_th_reg.flag_fifo_style_dispm = ((input_frame_rate > 600)? 1: 0);
		vodma_vodma_i2rnd_fifo_th_reg.flag_fifo_fast_int_dispm = (((input_frame_rate > 600) && interlace)? 1: 0);//merlin4 can support interlace faster mode
	}
       IoReg_Write32(VODMA_vodma_i2rnd_fifo_th_reg, vodma_vodma_i2rnd_fifo_th_reg.regValue);
       //rtd_pr_i3ddma_emerg("[crixus]VODMA_vodma_i2rnd_fifo_th_reg = %x\n", IoReg_Read32(VODMA_vodma_i2rnd_fifo_th_reg));
}

void h3ddma_set_capture_enable(unsigned char chanel,unsigned char enable)
{
	h3ddma_cap0_cti_dma_wr_ctrl_RBUS h3ddma_cap0_cti_dma_wr_ctrl_reg;
	h3ddma_cap1_cti_dma_wr_ctrl_RBUS h3ddma_cap1_cti_dma_wr_ctrl_reg;
	h3ddma_cap2_cti_dma_wr_ctrl_RBUS h3ddma_cap2_cti_dma_wr_ctrl_reg;
	h3ddma_cap3_cti_dma_wr_ctrl_RBUS h3ddma_cap3_cti_dma_wr_ctrl_reg;

	if(chanel==0)
	{
		h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg);
		h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_enable= enable;
		IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue);
	}
	else if(chanel==1)
	{
		h3ddma_cap1_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP1_CTI_DMA_WR_Ctrl_reg);
		h3ddma_cap1_cti_dma_wr_ctrl_reg.cap1_dma_enable= enable;
		IoReg_Write32(H3DDMA_CAP1_CTI_DMA_WR_Ctrl_reg, h3ddma_cap1_cti_dma_wr_ctrl_reg.regValue);
	}
	else if(chanel==2)
	{
		h3ddma_cap2_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP2_CTI_DMA_WR_Ctrl_reg);
		h3ddma_cap2_cti_dma_wr_ctrl_reg.cap2_dma_enable= enable;
		IoReg_Write32(H3DDMA_CAP2_CTI_DMA_WR_Ctrl_reg, h3ddma_cap2_cti_dma_wr_ctrl_reg.regValue);
	}
	else if(chanel==3)
	{
		h3ddma_cap3_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP3_CTI_DMA_WR_Ctrl_reg);
		h3ddma_cap3_cti_dma_wr_ctrl_reg.cap3_dma_enable= enable;
		IoReg_Write32(H3DDMA_CAP3_CTI_DMA_WR_Ctrl_reg, h3ddma_cap3_cti_dma_wr_ctrl_reg.regValue);
	}
}


#ifdef CONFIG_ENABLE_HDMI_NN
unsigned char h3ddma_allocate_nv12_mermory(UINT32 width,UINT32 height,unsigned char buffernub)
{
    unsigned int size_video, allocSize;
	//unsigned long  vir_addr, vir_addr_noncache;
	unsigned long phyaddr;
	int i;

	// Reset buffer index
	g_ucNNReadIdx = 0;
	g_ucNNWriteIdx = 0;

	if(g_tNN_Cap_Buffer[0].phyaddr != 0)
	{
		h3ddma_free_nv12_mermory();
	}

	//size_video = ALIGN((width*height),__12KPAGE)+ ALIGN((width*height),__12KPAGE);
	size_video = ALIGN((width*height),__12KPAGE);

	allocSize = (size_video + size_video / 2) * buffernub;

	rtd_pr_i3ddma_info("h3ddma_allocate_nv12_mermory, widh=%d, height=%d, buffernub=%d, size_video=%d, allocSize=%d\n", width, height, buffernub, size_video, allocSize);

	//rtd_pr_i3ddma_info("origianl  [h3ddma nv12] buf[0]=%lx(%d KB), phy(%lx) \n", (unsigned long)i3ddmaCtrl.cap_buffer[5].cache, i3ddmaCtrl.cap_buffer[5].size >> 10, phyaddr);
	//g_cacheaddr=(unsigned long)dvr_malloc_uncached_specific(allocSize, GFP_DCU1,(void **)&g_uncacheaddr);
	//phyaddr = (unsigned long)dvr_to_phys((void*)g_cacheaddr);
	phyaddr = pli_malloc(allocSize, GFP_DCU1);

	//i3ddmaCtrl.cap_buffer[5].cache = (void *)g_cacheaddr;
	//i3ddmaCtrl.cap_buffer[5].uncache = (unsigned char *)g_uncacheaddr;

	//rtd_pr_i3ddma_info("cacheaddr=%lx, uncacheaddr=%lx\n", g_cacheaddr, g_uncacheaddr);

	if(phyaddr==0)
	{
		rtd_pr_i3ddma_emerg("h3ddma_allocate_nv12_mermory faile %lx\n", phyaddr);
		return FALSE;
	}

	for(i=0;i<buffernub;i++)
	{
		//i3ddmaCtrl.cap_buffer[i].phyaddr =  (phyaddr + i*size_video);
		//i3ddmaCtrl.cap_buffer[i].size=size_video;
		g_tNN_Cap_Buffer[i].phyaddr = (phyaddr + i*size_video);
		g_tNN_Cap_Buffer[i].size = size_video;
	}
	//rtd_pr_i3ddma_info("new [h3ddma nv12] buf[%d]=%lx(%d KB), phy(%lx) \n", i, (unsigned long)i3ddmaCtrl.cap_buffer[5].cache, i3ddmaCtrl.cap_buffer[5].size >> 10, (unsigned long)i3ddmaCtrl.cap_buffer[5].phyaddr);
	rtd_pr_i3ddma_info("new [h3ddma nv12] buf[%d]=%lx(%d KB), phy(%lx) \n", i, (unsigned long)g_tNN_Cap_Buffer[0].cache, g_tNN_Cap_Buffer[0].size >> 10, (unsigned long)g_tNN_Cap_Buffer[0].phyaddr);

	IoReg_Write32(H3DDMA_CAP1_CTI_DMA_WR_Rule_check_low_reg, phyaddr);
	IoReg_Write32(H3DDMA_CAP1_CTI_DMA_WR_Rule_check_up_reg, phyaddr + size_video * buffernub);

	IoReg_Write32(H3DDMA_CAP2_CTI_DMA_WR_Rule_check_low_reg, phyaddr + size_video * buffernub);
	IoReg_Write32(H3DDMA_CAP2_CTI_DMA_WR_Rule_check_up_reg, phyaddr + (size_video + size_video/2) * buffernub);

	return TRUE;

}



void h3ddma_free_nv12_mermory(void)
{
	int i;

	rtd_pr_i3ddma_info("!!!h3ddma_free_nv12_mermory free nn memory\n");

	h3ddma_run_film_mode_enable(0);

	if(g_tNN_Cap_Buffer[0].phyaddr !=0 )
	{
		pli_free(g_tNN_Cap_Buffer[0].phyaddr);
	}

	for(i=0;i<NN_CAP_BUFFER_NUM;i++)
	{
		g_tNN_Cap_Buffer[i].phyaddr =  0;
		g_tNN_Cap_Buffer[i].size=0;
	}
	g_tNN_Cap_Buffer[0].cache=NULL;
	g_tNN_Cap_Buffer[0].uncache=NULL;
    g_ulNNOutWidth = 0;
    g_ulNNOutLength = 0;
}


void h3ddma_set_nv12_buffer_addr(UINT32 width,UINT32 height,unsigned long addr)
{
	unsigned long y_addr,c_addr;

	y_addr=addr;
	c_addr=addr+ALIGN((width*height),__12KPAGE);

	IoReg_Write32( H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg, y_addr);
	IoReg_Write32( H3DDMA_CAP2_WR_DMA_num_bl_wrap_addr_0_reg, c_addr);
}

void h3ddma_set_nv12capture_enable(unsigned char enable)
{
	h3ddma_cap0_cti_dma_wr_ctrl_RBUS h3ddma_cap0_cti_dma_wr_ctrl_reg;
	h3ddma_cap2_cti_dma_wr_ctrl_RBUS h3ddma_cap2_cti_dma_wr_ctrl_reg;

	h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_enable= enable;
	IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue);

	h3ddma_cap2_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP2_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap2_cti_dma_wr_ctrl_reg.cap2_dma_enable= enable;
	IoReg_Write32(H3DDMA_CAP2_CTI_DMA_WR_Ctrl_reg, h3ddma_cap2_cti_dma_wr_ctrl_reg.regValue);


}

void drvif_h3ddma_set_nv12_capture(UINT32 width,UINT32 height)
{
	h3ddma_i3ddma_enable_RBUS                      h3ddma_i3ddma_enable_reg;
	h3ddma_cap0_cap_ctl0_RBUS                      h3ddma_cap0_cap_ctl0_reg;
	h3ddma_cap2_cap_ctl0_RBUS                      h3ddma_cap2_cap_ctl0_reg;
	h3ddma_cap0_wr_dma_num_bl_wrap_word_RBUS       h3ddma_cap0_wr_dma_num_bl_wrap_word_reg;
	h3ddma_cap0_wr_dma_num_bl_wrap_ctl_RBUS		   h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg;
	h3ddma_cap0_wr_dma_num_bl_wrap_line_step_RBUS  h3ddma_cap0_wr_dma_num_bl_wrap_line_step_reg;
	h3ddma_cap2_wr_dma_num_bl_wrap_word_RBUS 	   h3ddma_cap2_wr_dma_num_bl_wrap_word_reg;
	h3ddma_cap2_wr_dma_pxltobus_RBUS	           h3ddma_cap2_wr_dma_pxltobus_reg;
	h3ddma_cap2_wr_dma_num_bl_wrap_ctl_RBUS		   h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg;
	h3ddma_cap2_wr_dma_num_bl_wrap_line_step_RBUS  h3ddma_cap2_wr_dma_num_bl_wrap_line_step_reg;
	h3ddma_lr_separate_ctrl2_RBUS                  h3ddma_lr_separate_ctrl2_reg;
	h3ddma_lr_separate_ctrl3_RBUS                  h3ddma_lr_separate_ctrl3_reg;
	h3ddma_lr_separate_ctrl1_RBUS                  h3ddma_lr_separate_ctrl1_reg;

	UINT32 bitsPerLine = width * 8;
	UINT32 fifoWidth = 128;
	UINT32 fifoElementsPerLine;

	h3ddma_set_nv12capture_enable(0);

	if (bitsPerLine % fifoWidth)
		  fifoElementsPerLine = (UINT32) (bitsPerLine/fifoWidth) + 1;
	else
		  fifoElementsPerLine = (UINT32) (bitsPerLine/fifoWidth);

	//cap0 block mode
	h3ddma_i3ddma_enable_reg.regValue = IoReg_Read32(H3DDMA_I3DDMA_enable_reg);
	h3ddma_i3ddma_enable_reg.cap0_seq_blk_sel = 1;
	//h3ddma_i3ddma_enable_reg.cap2_seq_blk_sel = 1;
	IoReg_Write32(H3DDMA_I3DDMA_enable_reg, h3ddma_i3ddma_enable_reg.regValue);

    h3ddma_cap0_cap_ctl0_reg.regValue = IoReg_Read32(H3DDMA_CAP0_Cap_CTL0_reg);
    h3ddma_cap0_cap_ctl0_reg.cap0_auto_block_sel_en = 0;
    rtd_outl(H3DDMA_CAP0_Cap_CTL0_reg, h3ddma_cap0_cap_ctl0_reg.regValue);

    h3ddma_cap2_cap_ctl0_reg.regValue = IoReg_Read32(H3DDMA_CAP2_Cap_CTL0_reg);
    h3ddma_cap2_cap_ctl0_reg.cap2_auto_block_sel_en = 0;
    IoReg_Write32(H3DDMA_CAP2_Cap_CTL0_reg, h3ddma_cap2_cap_ctl0_reg.regValue);

		//data_y
	h3ddma_cap0_wr_dma_num_bl_wrap_word_reg.regValue      = IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_word_reg);
	h3ddma_cap0_wr_dma_num_bl_wrap_word_reg.cap0_line_burst_num = fifoElementsPerLine;
	IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_word_reg, h3ddma_cap0_wr_dma_num_bl_wrap_word_reg.regValue);


	h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg.regValue	   = IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_ctl_reg);
	h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg.cap0_burst_len = 32;
	h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg.cap0_line_num = height;
	IoReg_Write32( H3DDMA_CAP0_WR_DMA_num_bl_wrap_ctl_reg, h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg.regValue);

	h3ddma_cap0_wr_dma_num_bl_wrap_line_step_reg.regValue = IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_line_step_reg);
	h3ddma_cap0_wr_dma_num_bl_wrap_line_step_reg.cap0_line_step = fifoElementsPerLine;
	IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_line_step_reg, h3ddma_cap0_wr_dma_num_bl_wrap_line_step_reg.regValue);

	  //data_c
	h3ddma_cap2_wr_dma_num_bl_wrap_word_reg.regValue	   = IoReg_Read32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_word_reg);
	h3ddma_cap2_wr_dma_num_bl_wrap_word_reg.cap2_line_burst_num = fifoElementsPerLine;
	IoReg_Write32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_word_reg, h3ddma_cap2_wr_dma_num_bl_wrap_word_reg.regValue);

	h3ddma_cap2_wr_dma_pxltobus_reg.regValue = IoReg_Read32(H3DDMA_CAP2_WR_DMA_pxltobus_reg);
	h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg.regValue	   = IoReg_Read32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_ctl_reg);
	if(h3ddma_cap2_wr_dma_pxltobus_reg.cap2_pixel_encoding == 2)
		h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg.cap2_line_num = height/2;
	else
		h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg.cap2_line_num = height;
	h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg.cap2_burst_len = 32;
	IoReg_Write32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_ctl_reg, h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg.regValue);

	h3ddma_cap2_wr_dma_num_bl_wrap_line_step_reg.regValue = IoReg_Read32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_line_step_reg);
	h3ddma_cap2_wr_dma_num_bl_wrap_line_step_reg.cap2_line_step = fifoElementsPerLine;
	IoReg_Write32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_line_step_reg, h3ddma_cap2_wr_dma_num_bl_wrap_line_step_reg.regValue);

	h3ddma_lr_separate_ctrl2_reg.regValue=IoReg_Read32(H3DDMA_LR_Separate_CTRL2_reg);
	h3ddma_lr_separate_ctrl2_reg.hact = width;
	h3ddma_lr_separate_ctrl2_reg.vact = height;
	IoReg_Write32(H3DDMA_LR_Separate_CTRL2_reg,h3ddma_lr_separate_ctrl2_reg.regValue);

	h3ddma_lr_separate_ctrl3_reg.regValue = IoReg_Read32(H3DDMA_LR_Separate_CTRL3_reg);
	h3ddma_lr_separate_ctrl3_reg.hact2 = width;
	h3ddma_lr_separate_ctrl3_reg.vact2 = height;
	IoReg_Write32(H3DDMA_LR_Separate_CTRL3_reg,h3ddma_lr_separate_ctrl3_reg.regValue);

	h3ddma_lr_separate_ctrl1_reg.regValue = IoReg_Read32(H3DDMA_LR_Separate_CTRL1_reg);
	h3ddma_lr_separate_ctrl1_reg.fp_vact_space1=1;
	h3ddma_lr_separate_ctrl1_reg.fp_vact_space2=5;
	h3ddma_lr_separate_ctrl1_reg.progressive=1;
	h3ddma_lr_separate_ctrl1_reg.hdmi_3d_structure=10;
	IoReg_Write32(H3DDMA_LR_Separate_CTRL1_reg,h3ddma_lr_separate_ctrl1_reg.regValue);

	h3ddma_i3ddma_enable_reg.regValue = IoReg_Read32(H3DDMA_I3DDMA_enable_reg);
	h3ddma_i3ddma_enable_reg.lr_separate_en =1;
	IoReg_Write32(H3DDMA_I3DDMA_enable_reg,h3ddma_i3ddma_enable_reg.regValue);

	h3ddma_set_nv12_buffer_addr(width,height,i3ddmaCtrl.cap_buffer[0].phyaddr);

	h3ddma_set_nv12capture_enable(1);


}

void h3ddma_set_MDP_enable(unsigned char enable){

	h3ddma_cap3_cti_dma_wr_ctrl_RBUS h3ddma_cap3_cti_dma_wr_ctrl_reg;
	h3ddma_cap3_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP3_CTI_DMA_WR_Ctrl_reg);
    h3ddma_cap3_cti_dma_wr_ctrl_reg.cap3_dma_enable = enable;
    IoReg_Write32(H3DDMA_CAP3_CTI_DMA_WR_Ctrl_reg, h3ddma_cap3_cti_dma_wr_ctrl_reg.regValue);
}

void h3ddma_set_mdp_buffer_addr(void)
{


	h3ddma_cap3_wr_dma_num_bl_wrap_addr_0_RBUS cap3_sta_addr_0;
	h3ddma_cap3_wr_dma_num_bl_wrap_addr_1_RBUS cap3_sta_addr_1;
	h3ddma_cap3_wr_dma_num_bl_wrap_addr_2_RBUS cap3_sta_addr_2;
	h3ddma_cap3_wr_dma_num_bl_wrap_addr_3_RBUS cap3_sta_addr_3;
	unsigned int mdCapM1, mdCapM2, mdCapM3, mdCapM4;

	if(get_hdr_semaless_active())//(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)
	{


		// use IDMA R-frame
		h3ddma_cap0_wr_dma_num_bl_wrap_addr_r0_RBUS cap0_r0_start_reg;
		//h3ddma_cap0_wr_dma_num_bl_wrap_addr_r1_RBUS cap0_r1_start_reg;
		cap0_r0_start_reg.regValue = rtd_inl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_r0_reg);
		//cap0_r1_start_reg.regValue = rtd_inl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_r1_reg);
		// IDMA md_parser capture size = 128 * 100 * 3= 0x9600

		mdCapM1 = cap0_r0_start_reg.cap0_sta_addr_r0 + 0x0;
		mdCapM2 = cap0_r0_start_reg.cap0_sta_addr_r0 + (0x10000>>3);
		mdCapM3 = cap0_r0_start_reg.cap0_sta_addr_r0 + (0x20000>>3);
		mdCapM4 = cap0_r0_start_reg.cap0_sta_addr_r0 + (0x30000>>3);
	}
	else{


		rtd_pr_i3ddma_debug("\n*** [DV][ERROR] IDMA NOT ENABLED YET!! ***\n\n");
		return;
	}

	cap3_sta_addr_0.cap3_sta_addr_0 = mdCapM1;
	cap3_sta_addr_1.cap3_sta_addr_1 = mdCapM2;
	cap3_sta_addr_2.cap3_sta_addr_2 = mdCapM3;
	cap3_sta_addr_3.cap3_sta_addr_3 = mdCapM4;

	rtd_outl(H3DDMA_CAP3_WR_DMA_num_bl_wrap_addr_0_reg, cap3_sta_addr_0.regValue);
	rtd_outl(H3DDMA_CAP3_WR_DMA_num_bl_wrap_addr_1_reg, cap3_sta_addr_1.regValue);
	rtd_outl(H3DDMA_CAP3_WR_DMA_num_bl_wrap_addr_2_reg, cap3_sta_addr_2.regValue);
	rtd_outl(H3DDMA_CAP3_WR_DMA_num_bl_wrap_addr_3_reg, cap3_sta_addr_3.regValue);

	rtd_pr_i3ddma_debug("[DV] MD_Cap[1]=%x\n", cap3_sta_addr_0.regValue);
	rtd_pr_i3ddma_debug("[DV] MD_Cap[2]=%x\n", cap3_sta_addr_1.regValue);
	rtd_pr_i3ddma_debug("[DV] MD_Cap[3]=%x\n", cap3_sta_addr_2.regValue);
	rtd_pr_i3ddma_debug("[DV] MD_Cap[4]=%x\n", cap3_sta_addr_3.regValue);
}

void  drvif_h3ddma_set_MDP_capture(int pktNum, int burstPktNum)
{
        #define FIFO_WIDTH   128 //128 bits

		int bitsPerFrame; //in bits
        int fifoElementsPerFrame; //# of 128 bits
        int burstLen;

		h3ddma_cap3_wr_dma_num_bl_wrap_word_RBUS  h3ddma_cap3_wr_dma_num_bl_wrap_word_reg;
		h3ddma_cap3_wr_dma_num_bl_wrap_ctl_RBUS  h3ddma_cap3_wr_dma_num_bl_wrap_ctl_reg;
		h3ddma_cap3_wr_dma_num_bl_wrap_line_step_RBUS  h3ddma_cap3_wr_dma_num_bl_wrap_line_step_reg;
		h3ddma_cap3_cti_dma_wr_ctrl_RBUS h3ddma_cap3_cti_dma_wr_ctrl_reg;
		h3ddma_cap3_wr_dma_pxltobus_RBUS h3ddma_cap3_wr_dma_pxltobus_reg;
		//h3ddma_hsd_i3ddma_vsd_ctrl0_RBUS h3ddma_hsd_i3ddma_vsd_ctrl0_reg;
		h3ddma_i3ddma_mdp_cr_RBUS h3ddma_i3ddma_mdp_cr_reg;
		h3ddma_interrupt_enable_RBUS h3ddma_interrupt_enable_reg;

		h3ddma_cap3_wr_dma_num_bl_wrap_word_reg.regValue = IoReg_Read32(H3DDMA_CAP3_WR_DMA_num_bl_wrap_word_reg);
		h3ddma_cap3_wr_dma_num_bl_wrap_ctl_reg.regValue = IoReg_Read32(H3DDMA_CAP3_WR_DMA_num_bl_wrap_ctl_reg);
		h3ddma_cap3_wr_dma_num_bl_wrap_line_step_reg.regValue = IoReg_Read32(H3DDMA_CAP3_WR_DMA_num_bl_wrap_line_step_reg);
		h3ddma_cap3_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP3_CTI_DMA_WR_Ctrl_reg);
		//h3ddma_hsd_i3ddma_vsd_ctrl0_reg.regValue = IoReg_Read32(H3DDMA_HSD_I3DDMA_VSD_Ctrl0_reg);

		bitsPerFrame =  pktNum * 128 * 8; // each pkt is 128*8 bits
        if(bitsPerFrame%FIFO_WIDTH)
            fifoElementsPerFrame = (bitsPerFrame/FIFO_WIDTH)+1;
        else
            fifoElementsPerFrame = (bitsPerFrame/FIFO_WIDTH);

		burstLen =  0x10; //burstPktNum*24;


		h3ddma_cap3_wr_dma_num_bl_wrap_word_reg.cap3_line_burst_num = fifoElementsPerFrame;
		IoReg_Write32(H3DDMA_CAP3_WR_DMA_num_bl_wrap_word_reg, h3ddma_cap3_wr_dma_num_bl_wrap_word_reg.regValue);

		h3ddma_cap3_wr_dma_num_bl_wrap_ctl_reg.cap3_burst_len = burstLen;
		h3ddma_cap3_wr_dma_num_bl_wrap_ctl_reg.cap3_line_num = 1;
		IoReg_Write32(H3DDMA_CAP3_WR_DMA_num_bl_wrap_ctl_reg, h3ddma_cap3_wr_dma_num_bl_wrap_ctl_reg.regValue);

		h3ddma_cap3_wr_dma_num_bl_wrap_line_step_reg.cap3_line_step = fifoElementsPerFrame; // in 64bits
		IoReg_Write32(H3DDMA_CAP3_WR_DMA_num_bl_wrap_line_step_reg, h3ddma_cap3_wr_dma_num_bl_wrap_line_step_reg.regValue);

		h3ddma_cap3_cti_dma_wr_ctrl_reg.cap3_dma_8byte_swap = 1;
		h3ddma_cap3_cti_dma_wr_ctrl_reg.cap3_dma_4byte_swap = 1;
		h3ddma_cap3_cti_dma_wr_ctrl_reg.cap3_dma_2byte_swap = 1;
		h3ddma_cap3_cti_dma_wr_ctrl_reg.cap3_dma_1byte_swap = 1;
		//h3ddma_cap3_cti_dma_wr_ctrl_reg.cap3_dma_enable=1;
		IoReg_Write32(H3DDMA_CAP3_CTI_DMA_WR_Ctrl_reg, h3ddma_cap3_cti_dma_wr_ctrl_reg.regValue);

		//h3ddma_hsd_i3ddma_vsd_ctrl0_reg.out_bit = 0;
	//	h3ddma_hsd_i3ddma_vsd_ctrl0_reg.sort_fmt = 0;
       	//IoReg_Write32(H3DDMA_HSD_I3DDMA_VSD_Ctrl0_reg, h3ddma_hsd_i3ddma_vsd_ctrl0_reg.regValue);

        h3ddma_i3ddma_mdp_cr_reg.regValue = IoReg_Read32(H3DDMA_I3DDMA_MDP_CR_reg);
        h3ddma_i3ddma_mdp_cr_reg.mdp_max_packet_num = 3;
        h3ddma_i3ddma_mdp_cr_reg.mdp_mode = 1;
        h3ddma_i3ddma_mdp_cr_reg.mdp_en = 1;
       	IoReg_Write32(H3DDMA_I3DDMA_MDP_CR_reg, h3ddma_i3ddma_mdp_cr_reg.regValue);

		//DIC: in cap3 case, Cap3_Pixel_encoding should set 1
    	h3ddma_cap3_wr_dma_pxltobus_reg.regValue = rtd_inl(H3DDMA_CAP3_WR_DMA_pxltobus_reg);
    	h3ddma_cap3_wr_dma_pxltobus_reg.cap3_pixel_encoding =1;
    	rtd_outl(H3DDMA_CAP3_WR_DMA_pxltobus_reg, h3ddma_cap3_wr_dma_pxltobus_reg.regValue);

#ifdef CONFIG_HDR_SDR_SEAMLESS
    	//down(&i3ddma_reg_Semaphore);  //todo: enable semaphore
    	h3ddma_interrupt_enable_reg.regValue = rtd_inl(H3DDMA_Interrupt_Enable_reg);
    	h3ddma_interrupt_enable_reg.cap3_last_wr_ie =1;
    	rtd_outl(H3DDMA_Interrupt_Enable_reg, h3ddma_interrupt_enable_reg.regValue);
    	//up(&i3ddma_reg_Semaphore);  //todo: enable semaphore
#else
    	h3ddma_interrupt_enable_reg.regValue = rtd_inl(H3DDMA_Interrupt_Enable_reg);
    	h3ddma_interrupt_enable_reg.cap3_last_wr_ie =1;
    	rtd_outl(H3DDMA_Interrupt_Enable_reg, h3ddma_interrupt_enable_reg.regValue);
#endif

	   	h3ddma_set_mdp_buffer_addr();
	  	h3ddma_set_MDP_enable(1);
}
#endif
#ifdef CONFIG_ENABLE_HDMI_NN
void h3ddma_set_film_buffer_addr(void)
{
	unsigned int ucBufAddr = g_tNN_Cap_Buffer[g_ucNNWriteIdx].phyaddr;
	unsigned int ucSize = g_tNN_Cap_Buffer[g_ucNNWriteIdx].size;

	if (0 == ucBufAddr)
	{
		rtd_pr_i3ddma_info("h3ddma_set_film_buffer_addr error, Idx=%d\n", g_ucNNReadIdx);
		return;
	}

	//buffer 0
	IoReg_Write32( H3DDMA_CAP1_WR_DMA_num_bl_wrap_addr_0_reg , ucBufAddr);
	ucBufAddr = g_tNN_Cap_Buffer[0].phyaddr + ucSize * NN_CAP_BUFFER_NUM + (ucSize/2) * g_ucNNWriteIdx;
    IoReg_Write32( H3DDMA_CAP2_WR_DMA_num_bl_wrap_addr_0_reg, ucBufAddr);

	// set getsize and this buffer is valid
	g_tNN_Cap_Buffer[g_ucNNWriteIdx].getsize = g_tNN_Cap_Buffer[g_ucNNWriteIdx].size;

	//rtd_pr_i3ddma_info("h3ddma_set_film_buffer_addr idx=%d, phyaddr=%x\n", g_ucNNWriteIdx, ucBufAddr);

	g_ucNNWriteIdx = (g_ucNNWriteIdx + 1) % NN_CAP_BUFFER_NUM;


	// boundary
	#if 0
	IoReg_Write32(H3DDMA_CAP1_CTI_DMA_WR_Rule_check_low_reg, ucBufAddr);
	IoReg_Write32(H3DDMA_CAP1_CTI_DMA_WR_Rule_check_up_reg, ucBufAddr+ucSize);

	IoReg_Write32(H3DDMA_CAP2_CTI_DMA_WR_Rule_check_low_reg, ucBufAddr+ucSize);
	IoReg_Write32(H3DDMA_CAP2_CTI_DMA_WR_Rule_check_up_reg, ucBufAddr+ucSize+ucSize/2);

	IoReg_Write32(H3DDMA_CAP1_CTI_DMA_WR_Rule_check_low_reg, 0);
	IoReg_Write32(H3DDMA_CAP1_CTI_DMA_WR_Rule_check_up_reg, 0xffffffff);

	IoReg_Write32(H3DDMA_CAP2_CTI_DMA_WR_Rule_check_low_reg, 0);
	IoReg_Write32(H3DDMA_CAP2_CTI_DMA_WR_Rule_check_up_reg, 0xffffffff);
	#endif


#if 0
	IoReg_Write32( H3DDMA_CAP1_WR_DMA_num_bl_wrap_addr_0_reg , g_tNN_Cap_Buffer[0].phyaddr);
        IoReg_Write32( H3DDMA_CAP2_WR_DMA_num_bl_wrap_addr_0_reg, g_tNN_Cap_Buffer[0].phyaddr+g_tNN_Cap_Buffer[0].size/2);

	IoReg_Write32(H3DDMA_CAP1_CTI_DMA_WR_Rule_check_low_reg, g_tNN_Cap_Buffer[0].phyaddr);
	IoReg_Write32(H3DDMA_CAP1_CTI_DMA_WR_Rule_check_up_reg, g_tNN_Cap_Buffer[0].phyaddr+g_tNN_Cap_Buffer[0].size/2);

	IoReg_Write32(H3DDMA_CAP2_CTI_DMA_WR_Rule_check_low_reg, g_tNN_Cap_Buffer[0].phyaddr+g_tNN_Cap_Buffer[0].size/2);
	IoReg_Write32(H3DDMA_CAP2_CTI_DMA_WR_Rule_check_up_reg, g_tNN_Cap_Buffer[0].phyaddr+g_tNN_Cap_Buffer[0].size);
#endif
	//rtd_pr_i3ddma_emerg("i3ddma cap1 boundary low=%x, high=%x\n", IoReg_Read32(H3DDMA_CAP1_CTI_DMA_WR_Rule_check_low_reg), IoReg_Read32(H3DDMA_CAP1_CTI_DMA_WR_Rule_check_up_reg));
	//rtd_pr_i3ddma_emerg("i3ddma cap2 boundary low=%x, high=%x\n", IoReg_Read32(H3DDMA_CAP2_CTI_DMA_WR_Rule_check_low_reg), IoReg_Read32(H3DDMA_CAP2_CTI_DMA_WR_Rule_check_up_reg));
}
void drvif_h3ddma_set_film_capture(UINT32 width,UINT32 height)
{
	h3ddma_i3ddma_enable_RBUS                      h3ddma_i3ddma_enable_reg;
	h3ddma_cap1_cap_ctl0_RBUS                      h3ddma_cap1_cap_ctl0_reg;
	h3ddma_cap1_wr_dma_num_bl_wrap_word_RBUS       h3ddma_cap1_wr_dma_num_bl_wrap_word_reg;
	h3ddma_cap1_wr_dma_num_bl_wrap_ctl_RBUS		   h3ddma_cap1_wr_dma_num_bl_wrap_ctl_reg;
	h3ddma_cap1_wr_dma_num_bl_wrap_line_step_RBUS  h3ddma_cap1_wr_dma_num_bl_wrap_line_step_reg;
	h3ddma_cap2_cap_ctl0_RBUS                      h3ddma_cap2_cap_ctl0_reg;
	h3ddma_cap2_wr_dma_num_bl_wrap_word_RBUS 	   h3ddma_cap2_wr_dma_num_bl_wrap_word_reg;
	h3ddma_cap2_wr_dma_pxltobus_RBUS	           h3ddma_cap2_wr_dma_pxltobus_reg;
	h3ddma_cap2_wr_dma_num_bl_wrap_ctl_RBUS		   h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg;
	h3ddma_cap2_wr_dma_num_bl_wrap_line_step_RBUS  h3ddma_cap2_wr_dma_num_bl_wrap_line_step_reg;
//	h3ddma_lr_separate_ctrl2_RBUS                  h3ddma_lr_separate_ctrl2_reg;
	h3ddma_lr_separate_ctrl3_RBUS                  h3ddma_lr_separate_ctrl3_reg;
//	h3ddma_lr_separate_ctrl1_RBUS                  h3ddma_lr_separate_ctrl1_reg;

	//h3ddma_i3ddma_ctrl_0_RBUS h3ddma_i3ddma_ctrl_0_reg;


	UINT32 bitsPerLine = width * 8;
	UINT32 fifoWidth = 128;
	UINT32 fifoElementsPerLine;

#if 0
	h3ddma_i3ddma_ctrl_0_reg.regValue = IoReg_Read32(H3DDMA_I3DDMA_ctrl_0_reg);
	h3ddma_i3ddma_ctrl_0_reg.cap2_channel_swap = 1;
	IoReg_Write32(H3DDMA_I3DDMA_ctrl_0_reg, h3ddma_i3ddma_ctrl_0_reg.regValue);
#endif

	h3ddma_set_capture_enable(1,0);
	h3ddma_set_capture_enable(2,0);

	if (bitsPerLine % fifoWidth)
		  fifoElementsPerLine = (UINT32) (bitsPerLine/fifoWidth) + 1;
	else
		  fifoElementsPerLine = (UINT32) (bitsPerLine/fifoWidth);


	// Disable double bfffer
	IoReg_ClearBits(H3DDMA_H3DDMA_PQ_CMP_DOUBLE_BUFFER_CTRL_reg, _BIT0);

	//cap0 block mode
	h3ddma_i3ddma_enable_reg.regValue = IoReg_Read32(H3DDMA_I3DDMA_enable_reg);
	h3ddma_i3ddma_enable_reg.cap1_seq_blk_sel = 1;
	h3ddma_i3ddma_enable_reg.cap2_seq_blk_sel = 1;
	IoReg_Write32(H3DDMA_I3DDMA_enable_reg, h3ddma_i3ddma_enable_reg.regValue);

	h3ddma_cap1_cap_ctl0_reg.regValue = IoReg_Read32(H3DDMA_CAP1_Cap_CTL0_reg);
	h3ddma_cap1_cap_ctl0_reg.cap1_auto_block_sel_en = 0;
	rtd_outl(H3DDMA_CAP1_Cap_CTL0_reg, h3ddma_cap1_cap_ctl0_reg.regValue);
	h3ddma_cap2_cap_ctl0_reg.regValue = IoReg_Read32(H3DDMA_CAP2_Cap_CTL0_reg);
	h3ddma_cap2_cap_ctl0_reg.cap2_auto_block_sel_en = 0;
	IoReg_Write32(H3DDMA_CAP2_Cap_CTL0_reg, h3ddma_cap2_cap_ctl0_reg.regValue);

	//data_y
	h3ddma_cap1_wr_dma_num_bl_wrap_word_reg.regValue      = IoReg_Read32(H3DDMA_CAP1_WR_DMA_num_bl_wrap_word_reg);
	h3ddma_cap1_wr_dma_num_bl_wrap_word_reg.cap1_line_burst_num = fifoElementsPerLine;
	IoReg_Write32(H3DDMA_CAP1_WR_DMA_num_bl_wrap_word_reg, h3ddma_cap1_wr_dma_num_bl_wrap_word_reg.regValue);

	h3ddma_cap1_wr_dma_num_bl_wrap_ctl_reg.regValue	   = IoReg_Read32(H3DDMA_CAP1_WR_DMA_num_bl_wrap_ctl_reg);
	h3ddma_cap1_wr_dma_num_bl_wrap_ctl_reg.cap1_burst_len = 32;
	h3ddma_cap1_wr_dma_num_bl_wrap_ctl_reg.cap1_line_num = height;
	IoReg_Write32( H3DDMA_CAP1_WR_DMA_num_bl_wrap_ctl_reg, h3ddma_cap1_wr_dma_num_bl_wrap_ctl_reg.regValue);

	h3ddma_cap1_wr_dma_num_bl_wrap_line_step_reg.regValue = IoReg_Read32(H3DDMA_CAP1_WR_DMA_num_bl_wrap_line_step_reg);
	h3ddma_cap1_wr_dma_num_bl_wrap_line_step_reg.cap1_line_step = fifoElementsPerLine;
	IoReg_Write32(H3DDMA_CAP1_WR_DMA_num_bl_wrap_line_step_reg, h3ddma_cap1_wr_dma_num_bl_wrap_line_step_reg.regValue);

	//data_c
	h3ddma_cap2_wr_dma_num_bl_wrap_word_reg.regValue	   = IoReg_Read32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_word_reg);
	h3ddma_cap2_wr_dma_num_bl_wrap_word_reg.cap2_line_burst_num = fifoElementsPerLine;
	IoReg_Write32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_word_reg, h3ddma_cap2_wr_dma_num_bl_wrap_word_reg.regValue);

	h3ddma_cap2_wr_dma_pxltobus_reg.regValue = IoReg_Read32(H3DDMA_CAP2_WR_DMA_pxltobus_reg);
	h3ddma_cap2_wr_dma_pxltobus_reg.cap2_pixel_encoding = 2;
	IoReg_Write32(H3DDMA_CAP2_WR_DMA_pxltobus_reg, h3ddma_cap2_wr_dma_pxltobus_reg.regValue);
	h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg.regValue	   = IoReg_Read32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_ctl_reg);
	if(h3ddma_cap2_wr_dma_pxltobus_reg.cap2_pixel_encoding == 2)
		h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg.cap2_line_num = height/2;
	else
		h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg.cap2_line_num = height;
	h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg.cap2_burst_len = 32;
	IoReg_Write32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_ctl_reg, h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg.regValue);

	h3ddma_cap2_wr_dma_num_bl_wrap_line_step_reg.regValue = IoReg_Read32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_line_step_reg);
	h3ddma_cap2_wr_dma_num_bl_wrap_line_step_reg.cap2_line_step = fifoElementsPerLine;
	IoReg_Write32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_line_step_reg, h3ddma_cap2_wr_dma_num_bl_wrap_line_step_reg.regValue);

	h3ddma_lr_separate_ctrl3_reg.regValue = IoReg_Read32(H3DDMA_LR_Separate_CTRL3_reg);
	h3ddma_lr_separate_ctrl3_reg.hact2 = width;
	h3ddma_lr_separate_ctrl3_reg.vact2 = height;
	IoReg_Write32(H3DDMA_LR_Separate_CTRL3_reg,h3ddma_lr_separate_ctrl3_reg.regValue);
	h3ddma_set_film_buffer_addr();
}

#endif

void h3ddma_set_sequence_cap0_buffer_addr(void)
{
		IoReg_Write32( H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg , 0);
        IoReg_Write32( H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_1_reg, 0);
}

void  drvif_h3ddma_set_sequence_capture0(I3DDMA_TIMING_T *timing)
{
#define FIFO_WIDTH   128


	int bitsPerLine;
    int fifoElementsPerFrame;
    int burstLen;
	unsigned int h_act_len;

	h3ddma_cap0_cap_ctl0_RBUS h3ddma_cap0_cap_ctl0_reg;
	h3ddma_lr_separate_ctrl1_RBUS h3ddma_lr_separate_ctrl1_reg;
	h3ddma_lr_separate_ctrl2_RBUS h3ddma_lr_separate_ctrl2_reg;
	h3ddma_cap0_cti_dma_wr_ctrl_RBUS h3ddma_cap0_cti_dma_wr_ctrl_reg;
//	h3ddma_i3ddma_enable_RBUS i3ddma_enable_reg;
	h3ddma_cap0_wr_dma_pxltobus_RBUS h3ddma_cap0_wr_dma_pxltobus_reg;
	h3ddma_cap0_wr_dma_num_bl_wrap_word_RBUS  h3ddma_cap0_wr_dma_num_bl_wrap_word_reg;
	h3ddma_cap0_wr_dma_num_bl_wrap_ctl_RBUS  h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg;
	h3ddma_cap0_wr_dma_num_bl_wrap_line_step_RBUS  h3ddma_cap0_wr_dma_num_bl_wrap_line_step_reg;

	if(timing  == NULL)
		return;
	// IDMA LR separate control
	h3ddma_lr_separate_ctrl1_reg.regValue = IoReg_Read32(H3DDMA_LR_Separate_CTRL1_reg);
	h3ddma_lr_separate_ctrl1_reg.progressive = (timing->progressive)? 1 : 0;
	h3ddma_lr_separate_ctrl1_reg.hdmi_3d_structure = 10; // frame 2D
	h3ddma_lr_separate_ctrl1_reg.fp_vact_space1=1;
	h3ddma_lr_separate_ctrl1_reg.fp_vact_space2=5;
	IoReg_Write32(H3DDMA_LR_Separate_CTRL1_reg, h3ddma_lr_separate_ctrl1_reg.regValue);

	h3ddma_lr_separate_ctrl2_reg.regValue = IoReg_Read32(H3DDMA_LR_Separate_CTRL2_reg);
	h3ddma_lr_separate_ctrl2_reg.hact = timing->h_act_len;
	h3ddma_lr_separate_ctrl2_reg.vact = timing->v_act_len;
	IoReg_Write32(H3DDMA_LR_Separate_CTRL2_reg,h3ddma_lr_separate_ctrl2_reg.regValue);

	if(dvrif_i3ddma_compression_get_enable() == TRUE){
		h_act_len = timing->h_act_len;
		if((h_act_len % 32) != 0){
			//drop bits
			h_act_len = h_act_len + (32 - (h_act_len % 32));
		}
		bitsPerLine = h_act_len * dvrif_i3ddma_get_compression_bits() + 256;
	}
	else
		bitsPerLine =(DEPTH2BITS[timing->depth] * (timing->h_act_len /4) * components[timing->color]);
	if(bitsPerLine%FIFO_WIDTH)
		fifoElementsPerFrame = (bitsPerLine/FIFO_WIDTH)+1;
	else
		fifoElementsPerFrame = (bitsPerLine/FIFO_WIDTH);
	burstLen = 32;

	h3ddma_cap0_wr_dma_num_bl_wrap_word_reg.regValue = IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_word_reg);
	h3ddma_cap0_wr_dma_num_bl_wrap_word_reg.cap0_line_burst_num = fifoElementsPerFrame;
	IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_word_reg, h3ddma_cap0_wr_dma_num_bl_wrap_word_reg.regValue);

	h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg.regValue = IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_ctl_reg);
	h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg.cap0_burst_len = burstLen;
	h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg.cap0_line_num = timing->v_act_len;
	IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_ctl_reg, h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg.regValue);

	h3ddma_cap0_wr_dma_num_bl_wrap_line_step_reg.regValue = IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_line_step_reg);
	h3ddma_cap0_wr_dma_num_bl_wrap_line_step_reg.cap0_line_step = fifoElementsPerFrame;
	IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_line_step_reg, h3ddma_cap0_wr_dma_num_bl_wrap_line_step_reg.regValue);

	h3ddma_cap0_cap_ctl0_reg.regValue = rtd_inl(H3DDMA_CAP0_Cap_CTL0_reg);
	h3ddma_cap0_cap_ctl0_reg.cap0_triple_buf_en = 0;
	h3ddma_cap0_cap_ctl0_reg.cap0_quad_buf_en = 0;
	h3ddma_cap0_cap_ctl0_reg.cap0_freeze_en = 0;
	IoReg_Write32(H3DDMA_CAP0_Cap_CTL0_reg, h3ddma_cap0_cap_ctl0_reg.regValue);

	h3ddma_cap0_wr_dma_pxltobus_reg.regValue=IoReg_Read32(H3DDMA_CAP0_WR_DMA_pxltobus_reg);
//#ifndef CONFIG_RTK_KDRV_DV_IDK_DUMP
#if 0
	h3ddma_cap0_wr_dma_pxltobus_reg.cap0_bit_sel=0;
	h3ddma_cap0_wr_dma_pxltobus_reg.cap0_pixel_encoding=0;
#else
	h3ddma_cap0_wr_dma_pxltobus_reg.cap0_bit_sel = timing->depth != I3DDMA_COLOR_DEPTH_8B;
	h3ddma_cap0_wr_dma_pxltobus_reg.cap0_pixel_encoding = encodemap[timing->color];
#endif
	IoReg_Write32(H3DDMA_CAP0_WR_DMA_pxltobus_reg, h3ddma_cap0_wr_dma_pxltobus_reg.regValue);

	if(I3DDMA_3D_OPMODE_HW==drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_HW_I3DDMA_DMA))
	{
		//for normal i3ddma vodma
		h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg);
		h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_1byte_swap = 0;
		h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_2byte_swap = 0;
		h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_4byte_swap = 0;
		h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_8byte_swap = 0;
		IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue);
	}
	else
	{
		//for capture
		h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg);
		h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_1byte_swap = 0;
		h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_2byte_swap = 1;
		h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_4byte_swap = 1;
		h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_8byte_swap = 1;
		//h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_enable=1; //enable IDMA capture3
		IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue);
	}


}

#ifdef CONFIG_ENABLE_HDMI_NN
void h3ddma_run_film_mode_enable(unsigned int enable)
{
//	h3ddma_cap0_cti_dma_wr_ctrl_RBUS h3ddma_cap0_cti_dma_wr_ctrl_reg;
	h3ddma_cap1_cti_dma_wr_ctrl_RBUS h3ddma_cap1_cti_dma_wr_ctrl_reg;
	h3ddma_cap2_cti_dma_wr_ctrl_RBUS h3ddma_cap2_cti_dma_wr_ctrl_reg;
//	h3ddma_cap3_cti_dma_wr_ctrl_RBUS h3ddma_cap3_cti_dma_wr_ctrl_reg;

	rtd_pr_i3ddma_emerg("0xb8025b0c=%x\n", IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg));
#if 0
	h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_enable=enable;
	IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue);
#endif
	h3ddma_cap1_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP1_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap1_cti_dma_wr_ctrl_reg.cap1_dma_enable=enable;
	IoReg_Write32(H3DDMA_CAP1_CTI_DMA_WR_Ctrl_reg, h3ddma_cap1_cti_dma_wr_ctrl_reg.regValue);

	h3ddma_cap2_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP2_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap2_cti_dma_wr_ctrl_reg.cap2_dma_enable=enable;
	IoReg_Write32(H3DDMA_CAP2_CTI_DMA_WR_Ctrl_reg, h3ddma_cap2_cti_dma_wr_ctrl_reg.regValue);

#if 0
	h3ddma_cap3_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP3_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap3_cti_dma_wr_ctrl_reg.cap3_dma_enable=enable;
	IoReg_Write32(H3DDMA_CAP3_CTI_DMA_WR_Ctrl_reg, h3ddma_cap3_cti_dma_wr_ctrl_reg.regValue);
#endif

}



void h3ddma_run_film_mode_capture_config(I3DDMA_TIMING_T *timing)
{
	h3ddma_i3ddma_enable_RBUS h3ddma_i3ddma_enable_reg;
	SIZE insize,outsize;

	h3ddma_run_film_mode_enable(0);

	// 4k > input size > 2k
	if ((timing->h_act_len) > 1920 && (timing->v_act_len) > 1080)
	{
		g_ulNNOutWidth = (timing->h_act_len) / 4;
		g_ulNNOutLength = (timing->v_act_len) / 4;
	}
	// 2k > input size > 1k
	else if ((timing->h_act_len) >= 960 && (timing->v_act_len) >= 540)
	{
		g_ulNNOutWidth = (timing->h_act_len) / 2;
		g_ulNNOutLength = (timing->v_act_len) / 2;
	}
	else
	{
		g_ulNNOutWidth = timing->h_act_len;
		g_ulNNOutLength = timing->v_act_len;
	}

	h3ddma_allocate_nv12_mermory(g_ulNNOutWidth,g_ulNNOutLength, NN_CAP_BUFFER_NUM);
	//drvif_h3ddma_set_sequence_capture0(timing);

	rtd_pr_i3ddma_emerg("NN output size, width=%d, height=%d\n", g_ulNNOutWidth, g_ulNNOutLength);

	//drvif_h3ddma_set_film_capture(timing->h_act_len,timing->v_act_len);
	drvif_h3ddma_set_film_capture(g_ulNNOutWidth, g_ulNNOutLength);

	//drvif_h3ddma_set_MDP_capture(3,1);

	h3ddma_i3ddma_enable_reg.regValue = IoReg_Read32(H3DDMA_I3DDMA_enable_reg);
	h3ddma_i3ddma_enable_reg.uzd_mux_sel = 1;
	//h3ddma_i3ddma_enable_reg.lr_separate_en=1;
	IoReg_Write32(H3DDMA_I3DDMA_enable_reg, h3ddma_i3ddma_enable_reg.regValue);


	insize.nWidth = timing->h_act_len;
	insize.nLength = timing->v_act_len;

	outsize.nWidth = g_ulNNOutWidth;
	outsize.nLength = g_ulNNOutLength;


	I3DDMA_ultrazoom_config_scaling_down_for_NN(&insize, &outsize, 1);

	//rtd_pr_i3ddma_emerg("Don't run h3ddma_run_film_mode_enable\n");

	h3ddma_run_film_mode_enable(1);

}

unsigned int h3ddma_get_NN_start_buffer_addr(void)
{
	return g_tNN_Cap_Buffer[0].phyaddr;
}

int h3ddma_get_NN_read_buffer(unsigned int *a_pulYAddr, unsigned int *a_pulCAddr)
{
	static unsigned int readCnt = 0;
	static unsigned int pre_readCnt = 0;
	unsigned int ulBufAddr = 0;
	unsigned int ulBufSize = 0;


	if ((g_ucNNReadIdx == g_ucNNWriteIdx) && (readCnt == pre_readCnt))
	{
		rtd_pr_i3ddma_info("h3ddma_get_NN_read_buffer error, ReadIdx=WriteIdx=%d\n", g_ucNNReadIdx);
		return -1;
	}

	// Get previous buffer of write index
	g_ucNNReadIdx = (g_ucNNWriteIdx +  NN_CAP_BUFFER_NUM - 1) % NN_CAP_BUFFER_NUM;

	ulBufAddr = g_tNN_Cap_Buffer[g_ucNNReadIdx].phyaddr;
	ulBufSize = g_tNN_Cap_Buffer[g_ucNNReadIdx].size;

	if (0 == ulBufAddr)
	{
		rtd_pr_i3ddma_info("h3ddma_get_NN_read_buffer error (phyaddr = 0), Idx=%d\n", g_ucNNReadIdx);
		return -1;
	}

	if (0 == g_tNN_Cap_Buffer[g_ucNNReadIdx].getsize)
	{
		rtd_pr_i3ddma_info("h3ddma_get_NN_read_buffer error (getsize = 0), Idx=%d\n", g_ucNNReadIdx);
		return -1;
	}

	*a_pulYAddr = ulBufAddr;
	*a_pulCAddr = g_tNN_Cap_Buffer[0].phyaddr + ulBufSize * NN_CAP_BUFFER_NUM + (ulBufSize/2)*g_ucNNReadIdx;

	// Clear getszie, this buffer is invalid
	g_tNN_Cap_Buffer[g_ucNNReadIdx].getsize = 0;

	rtd_pr_i3ddma_debug("h3ddma_get_NN_read_buffer idx=%d, phyaddr=%x, Y_Addr=%x, C_Addr=%x\n", g_ucNNReadIdx, ulBufAddr, *a_pulYAddr, *a_pulCAddr);

	pre_readCnt = readCnt;
	readCnt ++;

	return g_ucNNReadIdx;
}

void h3ddma_get_NN_output_size(unsigned int *outputWidth, unsigned int *outputLength)
{
   *outputWidth = g_ulNNOutWidth;
   *outputLength = g_ulNNOutLength;
}

#endif //end of #ifdef CONFIG_ENABLE_HDMI_NN
#ifndef BUILD_QUICK_SHOW
unsigned int get_i3ddma_4k120_width(void)
{
	if(i3ddmaCtrl.pgen_timing)
		return i3ddmaCtrl.pgen_timing->h_act_len;
	else
		return 0;
}

unsigned int get_i3ddma_4k120_length(void)
{
	if(i3ddmaCtrl.pgen_timing)
		return i3ddmaCtrl.pgen_timing->v_act_len;
	else
		return 0;
}

int h3ddma_allocate_4k120_se_buffer_impl(void)
{
	unsigned int video_size = _2K_2K_8BIT_SIZE;
	unsigned int se_alloc_size = video_size * 3;

	if(g_4K120_SE_Buffer[0].phyaddr_unalign == 0)
	{
		g_4K120_SE_Buffer[0].phyaddr_unalign = pli_malloc(se_alloc_size, GFP_DCU1);
		if(g_4K120_SE_Buffer[0].phyaddr_unalign == 0)
		{
			rtd_pr_i3ddma_emerg("i3ddma se alloc memory fail!\n");
			return false;
		}
	}

	g_4K120_SE_Buffer[0].phyaddr = drvif_memory_get_data_align(g_4K120_SE_Buffer[0].phyaddr_unalign, DMA_SPEEDUP_ALIGN_VALUE);
	g_4K120_SE_Buffer[0].size = video_size;
	g_4K120_SE_Buffer[1].phyaddr = drvif_memory_get_data_align(g_4K120_SE_Buffer[0].phyaddr + video_size, DMA_SPEEDUP_ALIGN_VALUE);
	g_4K120_SE_Buffer[1].size = video_size;
	g_4K120_SE_Buffer[2].phyaddr = drvif_memory_get_data_align(g_4K120_SE_Buffer[1].phyaddr + video_size, DMA_SPEEDUP_ALIGN_VALUE);
	g_4K120_SE_Buffer[2].size = video_size;
	rtd_pr_i3ddma_notice("[4k120]SE_buf[0]=%lx,SE_buf[1]=%lx,SE_buf[2]=%lx,c_size=%x",g_4K120_SE_Buffer[0].phyaddr,g_4K120_SE_Buffer[1].phyaddr,g_4K120_SE_Buffer[2].phyaddr,g_4K120_SE_Buffer[0].size);

	return true;
}

int (*h3ddma_allocate_4k120_se_buffer)(void) = h3ddma_allocate_4k120_se_buffer_impl;

void h3ddma_free_4k120_se_buffer(void)
{
	int i = 0;
	if(g_4K120_SE_Buffer[0].phyaddr_unalign)
	{
		pli_free(g_4K120_SE_Buffer[0].phyaddr_unalign);

	}

	for(i = 0; i < 3; i++)
	{
		g_4K120_SE_Buffer[i].phyaddr_unalign = 0;
		g_4K120_SE_Buffer[i].phyaddr = 0;
		g_4K120_SE_Buffer[i].cache=NULL;
		g_4K120_SE_Buffer[i].uncache=NULL;
	}

	return;

}

void h3ddma_remap_4k120_memory(void)
{
#define _2K_LEN 2160
#define _2K_WID 1920
#ifdef CONFIG_BW_96B_ALIGNED
	#define _2K_2K_8BIT_SIZE (drvif_memory_get_data_align((_2K_WID * 8 + 256) / 8, DMA_SPEEDUP_ALIGN_VALUE) * _2K_LEN)
#else
	#define _2K_2K_8BIT_SIZE  ((_2K_WID * 8 + 256)  * _2K_LEN / 8)
#endif

	unsigned int video_size = _2K_2K_8BIT_SIZE;

	//total:48M cap0(yy):2k*2k*2*3=24M  cap2(c):2k*2k*3=12M se(c):12M
	if(i3ddmaCtrl.cap_buffer[0].phyaddr == 0)
	{
		rtd_pr_i3ddma_emerg("i3ddma vbm no alloc memory!\n");
		I3DDMA_DolbyVision_HDMI_Init();
	}

	g_4K120_Cap_Y_Buffer[0].phyaddr = drvif_memory_get_data_align(i3ddmaCtrl.cap_buffer[0].phyaddr, DMA_SPEEDUP_ALIGN_VALUE);
	g_4K120_Cap_Y_Buffer[0].size = video_size * 2;
	g_4K120_Cap_Y_Buffer[1].phyaddr = drvif_memory_get_data_align(g_4K120_Cap_Y_Buffer[0].phyaddr + video_size * 2, DMA_SPEEDUP_ALIGN_VALUE);
	g_4K120_Cap_Y_Buffer[1].size = video_size * 2;
	g_4K120_Cap_Y_Buffer[2].phyaddr = drvif_memory_get_data_align(g_4K120_Cap_Y_Buffer[1].phyaddr + video_size * 2, DMA_SPEEDUP_ALIGN_VALUE);
	g_4K120_Cap_Y_Buffer[2].size = video_size * 2;
	rtd_pr_i3ddma_notice("[4k120]cap_y_buf[0]=%lx,cap_y_buf[1]=%lx,cap_y_buf[2]=%lx,y_size=%x",g_4K120_Cap_Y_Buffer[0].phyaddr,g_4K120_Cap_Y_Buffer[1].phyaddr,g_4K120_Cap_Y_Buffer[2].phyaddr,g_4K120_Cap_Y_Buffer[0].size);
	IoReg_Write32( H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg , g_4K120_Cap_Y_Buffer[0].phyaddr);
	IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_low_reg, g_4K120_Cap_Y_Buffer[0].phyaddr);
	IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_up_reg, g_4K120_Cap_Y_Buffer[0].phyaddr + video_size * 6);


	g_4K120_Cap_C_Buffer[0].phyaddr = drvif_memory_get_data_align(g_4K120_Cap_Y_Buffer[2].phyaddr + video_size * 2, DMA_SPEEDUP_ALIGN_VALUE);
	g_4K120_Cap_C_Buffer[0].size = video_size;
	g_4K120_Cap_C_Buffer[1].phyaddr = drvif_memory_get_data_align(g_4K120_Cap_C_Buffer[0].phyaddr + video_size, DMA_SPEEDUP_ALIGN_VALUE);
	g_4K120_Cap_C_Buffer[1].size = video_size;
	g_4K120_Cap_C_Buffer[2].phyaddr = drvif_memory_get_data_align(g_4K120_Cap_C_Buffer[1].phyaddr + video_size, DMA_SPEEDUP_ALIGN_VALUE);
	g_4K120_Cap_Y_Buffer[2].size = video_size;
	rtd_pr_i3ddma_notice("[4k120]cap_c_buf[0]=%lx,cap_c_buf[1]=%lx,cap_c_buf[2]=%lx,c_size=%x",g_4K120_Cap_C_Buffer[0].phyaddr,g_4K120_Cap_C_Buffer[1].phyaddr,g_4K120_Cap_C_Buffer[2].phyaddr,g_4K120_Cap_C_Buffer[0].size);
	IoReg_Write32( H3DDMA_CAP2_WR_DMA_num_bl_wrap_addr_0_reg , g_4K120_Cap_C_Buffer[0].phyaddr);
	IoReg_Write32(H3DDMA_CAP2_CTI_DMA_WR_Rule_check_low_reg, g_4K120_Cap_C_Buffer[0].phyaddr);
	IoReg_Write32(H3DDMA_CAP2_CTI_DMA_WR_Rule_check_up_reg, g_4K120_Cap_C_Buffer[0].phyaddr + video_size * 3);

}

int get_i3ddma_4k120_se_index(void)
{
	return (cur_idma_cap_index + 1) %  CAP_4K120_BUFFER_NUM;
}

void get_i3ddma_4k120_se_addr(unsigned int width, unsigned int *u_addr, unsigned int *v_addr, unsigned int *se_addr)
{
	int index = get_i3ddma_4k120_se_index();
	//rtd_pr_i3ddma_emerg("se_tsk:cur_idma_se_index=%d\n",index);
	if(index >= 0 && index < CAP_4K120_BUFFER_NUM)
	{
		*u_addr = g_4K120_Cap_C_Buffer[index].phyaddr;
		*v_addr = g_4K120_Cap_C_Buffer[index].phyaddr + width;
		*se_addr = g_4K120_SE_Buffer[index].phyaddr;
	}
}

void update_i3ddma_4k120_cap_isr(void)
{
	rtd_outl(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg , g_4K120_Cap_Y_Buffer[cur_idma_cap_index].phyaddr);
	rtd_outl(H3DDMA_CAP2_WR_DMA_num_bl_wrap_addr_0_reg , g_4K120_Cap_C_Buffer[cur_idma_cap_index].phyaddr);
	//rtd_pr_i3ddma_emerg("vgip_isr:cur_idma_cap_index=%d\n",cur_idma_cap_index);
	cur_idma_cap_index = (cur_idma_cap_index + 1) %  CAP_4K120_BUFFER_NUM;

}

void drvif_h3ddma_set_4k120_capture(UINT32 width,UINT32 height)
{
//4K2Kyuv420->2k2kyyc444
	h3ddma_i3ddma_enable_RBUS					   h3ddma_i3ddma_enable_reg;
	h3ddma_cap0_cap_ctl0_RBUS					   h3ddma_cap0_cap_ctl0_reg;
	h3ddma_lr_separate_ctrl2_RBUS				   h3ddma_lr_separate_ctrl2_reg;
	h3ddma_i3ddma_ctrl_0_RBUS					   h3ddma_i3ddma_ctrl_0_reg;
	h3ddma_cap0_wr_dma_num_bl_wrap_word_RBUS	   h3ddma_cap0_wr_dma_num_bl_wrap_word_reg;
	h3ddma_cap0_wr_dma_num_bl_wrap_ctl_RBUS 	   h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg;
	h3ddma_cap0_wr_dma_num_bl_wrap_line_step_RBUS  h3ddma_cap0_wr_dma_num_bl_wrap_line_step_reg;
	h3ddma_cap0_wr_dma_pxltobus_RBUS			   h3ddma_cap0_wr_dma_pxltobus_reg;
	h3ddma_cap0_cti_dma_wr_ctrl_RBUS			   h3ddma_cap0_cti_dma_wr_ctrl_reg;

	h3ddma_cap2_cap_ctl0_RBUS					   h3ddma_cap2_cap_ctl0_reg;
	h3ddma_lr_separate_ctrl3_RBUS				   h3ddma_lr_separate_ctrl3_reg;
	h3ddma_cap2_wr_dma_num_bl_wrap_word_RBUS	   h3ddma_cap2_wr_dma_num_bl_wrap_word_reg;
	h3ddma_cap2_wr_dma_pxltobus_RBUS			   h3ddma_cap2_wr_dma_pxltobus_reg;
	h3ddma_cap2_wr_dma_num_bl_wrap_ctl_RBUS 	   h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg;
	h3ddma_cap2_wr_dma_num_bl_wrap_line_step_RBUS  h3ddma_cap2_wr_dma_num_bl_wrap_line_step_reg;
	h3ddma_cap2_cti_dma_wr_ctrl_RBUS			   h3ddma_cap2_cti_dma_wr_ctrl_reg;

	UINT32 y_bitsPerLine = width * 16;
	UINT32 y_fifoWidth = 128;
	UINT32 y_fifoElementsPerLine;
	UINT32 c_bitsPerLine = width * 8;
	UINT32 c_fifoWidth = 128;
	UINT32 c_fifoElementsPerLine;

	h3ddma_set_capture_enable(0,0);
	h3ddma_set_capture_enable(2,0);

	// Disable double bfffer
	IoReg_ClearBits(H3DDMA_H3DDMA_PQ_CMP_DOUBLE_BUFFER_CTRL_reg, _BIT0);

	//cap0 seq mode, cap2 block mode
	h3ddma_i3ddma_enable_reg.regValue = IoReg_Read32(H3DDMA_I3DDMA_enable_reg);
	h3ddma_i3ddma_enable_reg.cap0_seq_blk_sel = 0;
	//h3ddma_i3ddma_enable_reg.cap2_seq_blk_sel = 1;
	IoReg_Write32(H3DDMA_I3DDMA_enable_reg, h3ddma_i3ddma_enable_reg.regValue);


	/******************cap0 setting***********************/
	//hact 1920, vact 2160
	h3ddma_lr_separate_ctrl2_reg.regValue = IoReg_Read32(H3DDMA_LR_Separate_CTRL2_reg);
	h3ddma_lr_separate_ctrl2_reg.hact = width;
	h3ddma_lr_separate_ctrl2_reg.vact = height;
	IoReg_Write32(H3DDMA_LR_Separate_CTRL2_reg, h3ddma_lr_separate_ctrl2_reg.regValue);

	//cap0 channel swap YYX -> YXY
	h3ddma_i3ddma_ctrl_0_reg.regValue = IoReg_Read32(H3DDMA_I3DDMA_ctrl_0_reg);
	h3ddma_i3ddma_ctrl_0_reg.cap0_channel_swap = 1;
	IoReg_Write32(H3DDMA_I3DDMA_ctrl_0_reg, h3ddma_i3ddma_ctrl_0_reg.regValue);

	//data_y
	if (y_bitsPerLine % y_fifoWidth)
		  y_fifoElementsPerLine = (UINT32) (y_bitsPerLine/y_fifoWidth) + 1;
	else
		  y_fifoElementsPerLine = (UINT32) (y_bitsPerLine/y_fifoWidth);

	h3ddma_cap0_wr_dma_num_bl_wrap_word_reg.regValue	  = IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_word_reg);
	h3ddma_cap0_wr_dma_num_bl_wrap_word_reg.cap0_line_burst_num = y_fifoElementsPerLine;
	IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_word_reg, h3ddma_cap0_wr_dma_num_bl_wrap_word_reg.regValue);

	h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg.regValue    = IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_ctl_reg);
	h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg.cap0_burst_len = 0x40;
	h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg.cap0_line_num = height;
	IoReg_Write32( H3DDMA_CAP0_WR_DMA_num_bl_wrap_ctl_reg, h3ddma_cap0_wr_dma_num_bl_wrap_ctl_reg.regValue);

	h3ddma_cap0_wr_dma_num_bl_wrap_line_step_reg.regValue = IoReg_Read32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_line_step_reg);
	h3ddma_cap0_wr_dma_num_bl_wrap_line_step_reg.cap0_line_step = y_fifoElementsPerLine;
	IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_line_step_reg, h3ddma_cap0_wr_dma_num_bl_wrap_line_step_reg.regValue);

	// 422, 8bit
	h3ddma_cap0_wr_dma_pxltobus_reg.regValue = IoReg_Read32(H3DDMA_CAP0_WR_DMA_pxltobus_reg);
	h3ddma_cap0_wr_dma_pxltobus_reg.cap0_bit_sel = 0;
	h3ddma_cap0_wr_dma_pxltobus_reg.cap0_pixel_encoding = 1;
	IoReg_Write32(H3DDMA_CAP0_WR_DMA_pxltobus_reg, h3ddma_cap0_wr_dma_pxltobus_reg.regValue);

	// byte_swap = 4'b1111
	h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_1byte_swap = 1;
	h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_2byte_swap = 1;
	h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_4byte_swap = 1;
	h3ddma_cap0_cti_dma_wr_ctrl_reg.cap0_dma_8byte_swap = 1;
	IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, h3ddma_cap0_cti_dma_wr_ctrl_reg.regValue);

	//cap0 run sw mode, line mode
	h3ddma_cap0_cap_ctl0_reg.regValue = IoReg_Read32(H3DDMA_CAP0_Cap_CTL0_reg);
	h3ddma_cap0_cap_ctl0_reg.cap0_auto_block_sel_en = 0;
	h3ddma_cap0_cap_ctl0_reg.cap0_frame_access_mode = 0;
	rtd_outl(H3DDMA_CAP0_Cap_CTL0_reg, h3ddma_cap0_cap_ctl0_reg.regValue);


	/******************cap2 setting***********************/

	//hact 1920, vact 2160
	h3ddma_lr_separate_ctrl3_reg.regValue = IoReg_Read32(H3DDMA_LR_Separate_CTRL3_reg);
	h3ddma_lr_separate_ctrl3_reg.hact2 = width;
	h3ddma_lr_separate_ctrl3_reg.vact2 = height;
	IoReg_Write32(H3DDMA_LR_Separate_CTRL3_reg, h3ddma_lr_separate_ctrl3_reg.regValue);

	//cap2 channel swap
	h3ddma_i3ddma_ctrl_0_reg.regValue = IoReg_Read32(H3DDMA_I3DDMA_ctrl_0_reg);
	//h3ddma_i3ddma_ctrl_0_reg.cap2_channel_swap = 0;
	IoReg_Write32(H3DDMA_I3DDMA_ctrl_0_reg, h3ddma_i3ddma_ctrl_0_reg.regValue);

	//data_c
	if (c_bitsPerLine % c_fifoWidth)
		  c_fifoElementsPerLine = (UINT32) (c_bitsPerLine/c_fifoWidth) + 1;
	else
		  c_fifoElementsPerLine = (UINT32) (c_bitsPerLine/c_fifoWidth);

	h3ddma_cap2_wr_dma_num_bl_wrap_word_reg.regValue	   = IoReg_Read32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_word_reg);
	h3ddma_cap2_wr_dma_num_bl_wrap_word_reg.cap2_line_burst_num = c_fifoElementsPerLine;
	IoReg_Write32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_word_reg, h3ddma_cap2_wr_dma_num_bl_wrap_word_reg.regValue);

	h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg.regValue    = IoReg_Read32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_ctl_reg);
	h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg.cap2_line_num = height;
	h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg.cap2_burst_len = 0x40;
	IoReg_Write32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_ctl_reg, h3ddma_cap2_wr_dma_num_bl_wrap_ctl_reg.regValue);

	h3ddma_cap2_wr_dma_num_bl_wrap_line_step_reg.regValue = IoReg_Read32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_line_step_reg);
	h3ddma_cap2_wr_dma_num_bl_wrap_line_step_reg.cap2_line_step = c_fifoElementsPerLine;
	IoReg_Write32(H3DDMA_CAP2_WR_DMA_num_bl_wrap_line_step_reg, h3ddma_cap2_wr_dma_num_bl_wrap_line_step_reg.regValue);

	// 422, 8bit
	h3ddma_cap2_wr_dma_pxltobus_reg.regValue = IoReg_Read32(H3DDMA_CAP2_WR_DMA_pxltobus_reg);
	h3ddma_cap2_wr_dma_pxltobus_reg.cap2_bit_sel = 0;
	h3ddma_cap2_wr_dma_pxltobus_reg.cap2_pixel_encoding = 1;
	IoReg_Write32(H3DDMA_CAP2_WR_DMA_pxltobus_reg, h3ddma_cap2_wr_dma_pxltobus_reg.regValue);


	// byte_swap = 4'b1111
	h3ddma_cap2_cti_dma_wr_ctrl_reg.regValue = IoReg_Read32(H3DDMA_CAP2_CTI_DMA_WR_Ctrl_reg);
	h3ddma_cap2_cti_dma_wr_ctrl_reg.cap2_dma_1byte_swap = 1;
	h3ddma_cap2_cti_dma_wr_ctrl_reg.cap2_dma_2byte_swap = 1;
	h3ddma_cap2_cti_dma_wr_ctrl_reg.cap2_dma_4byte_swap = 1;
	h3ddma_cap2_cti_dma_wr_ctrl_reg.cap2_dma_8byte_swap = 1;
	IoReg_Write32(H3DDMA_CAP2_CTI_DMA_WR_Ctrl_reg, h3ddma_cap2_cti_dma_wr_ctrl_reg.regValue);

	//cap2 run sw mode, line mode
	h3ddma_cap2_cap_ctl0_reg.regValue = IoReg_Read32(H3DDMA_CAP2_Cap_CTL0_reg);
	h3ddma_cap2_cap_ctl0_reg.cap2_auto_block_sel_en = 0;
	h3ddma_cap2_cap_ctl0_reg.cap2_frame_access_mode = 0;
	IoReg_Write32(H3DDMA_CAP2_Cap_CTL0_reg, h3ddma_cap2_cap_ctl0_reg.regValue);

	h3ddma_set_capture_enable(0,1);
	h3ddma_set_capture_enable(2,1);
}

void set_i3ddma_4k120_flag(unsigned char enable)
{
	i3ddma_4k120_flag = enable;
}

unsigned char get_i3ddma_4k120_flag(void)
{
	return i3ddma_4k120_flag;
}


void h3ddma_run_4k120_capture_config(I3DDMA_TIMING_T *timing)
{
	h3ddma_remap_4k120_memory();

	drvif_h3ddma_set_4k120_capture(timing->h_act_len, timing->v_act_len);
}

void h3ddma_capture_dma_freeze(unsigned char dma_num, unsigned char enable)
{
	h3ddma_cap0_cap_ctl0_RBUS	h3ddma_cap0_cap_ctl0_reg;
	h3ddma_cap1_cap_ctl0_RBUS	h3ddma_cap1_cap_ctl0_reg;
	h3ddma_cap2_cap_ctl0_RBUS	h3ddma_cap2_cap_ctl0_reg;
	h3ddma_cap3_cap_ctl0_RBUS	h3ddma_cap3_cap_ctl0_reg;

	switch(dma_num)
	{
		case 0:
			h3ddma_cap0_cap_ctl0_reg.regValue = rtd_inl(H3DDMA_CAP0_Cap_CTL0_reg);
			h3ddma_cap0_cap_ctl0_reg.cap0_freeze_en = enable;
			IoReg_Write32(H3DDMA_CAP0_Cap_CTL0_reg, h3ddma_cap0_cap_ctl0_reg.regValue);
			break;
		case 1:
			h3ddma_cap1_cap_ctl0_reg.regValue = rtd_inl(H3DDMA_CAP1_Cap_CTL0_reg);
			h3ddma_cap1_cap_ctl0_reg.cap1_freeze_en = enable;
			IoReg_Write32(H3DDMA_CAP1_Cap_CTL0_reg, h3ddma_cap1_cap_ctl0_reg.regValue);
			break;
		case 2:
			h3ddma_cap2_cap_ctl0_reg.regValue = rtd_inl(H3DDMA_CAP2_Cap_CTL0_reg);
			h3ddma_cap2_cap_ctl0_reg.cap2_freeze_en = enable;
			IoReg_Write32(H3DDMA_CAP2_Cap_CTL0_reg, h3ddma_cap2_cap_ctl0_reg.regValue);
			break;
		case 3:
			h3ddma_cap3_cap_ctl0_reg.regValue = rtd_inl(H3DDMA_CAP3_Cap_CTL0_reg);
			h3ddma_cap3_cap_ctl0_reg.cap3_freeze_en = enable;
			IoReg_Write32(H3DDMA_CAP3_Cap_CTL0_reg, h3ddma_cap3_cap_ctl0_reg.regValue);
			break;
		default:
			rtd_pr_i3ddma_emerg("h3ddma_capture_dma_freeze dma_num[%d] error\n", dma_num);
			break;
	}


}

/* setting to call kernel api */
bool scaler_SE_TMDS_stretch_Proc(SE_NN_info info) {
#if IS_ENABLED(CONFIG_RTK_KDRV_SE)

        //unsigned int se_sta, se_end;
        bool status=0;
        KGAL_SURFACE_INFO_T ssurf;
        KGAL_SURFACE_INFO_T dsurf;
        KGAL_RECT_T srect;
        KGAL_RECT_T drect;
        KGAL_BLIT_FLAGS_T sflag = KGAL_BLIT_NOFX;
        KGAL_BLIT_SETTINGS_T sblend;
        memset(&ssurf,0, sizeof(KGAL_SURFACE_INFO_T));
        memset(&dsurf,0, sizeof(KGAL_SURFACE_INFO_T));
        memset(&srect,0, sizeof(KGAL_RECT_T));
        memset(&drect,0, sizeof(KGAL_RECT_T));
        memset(&sblend,0, sizeof(KGAL_BLIT_SETTINGS_T));
        sblend.srcBlend = KGAL_BLEND_ONE;
        sblend.dstBlend = KGAL_BLEND_ZERO;

        ssurf.physicalAddress = info.src_phyaddr;
        ssurf.width = info.src_w;
        ssurf.height = info.src_h;
        ssurf.pixelFormat            = KGAL_PIXEL_FORMAT_NV16;
        srect.x = info.src_x;
        srect.y = info.src_y;
        srect.w = info.src_w;
        srect.h = info.src_h;

        dsurf.physicalAddress = info.dst_phyaddr;
        dsurf.width = info.dst_w;
        dsurf.height = info.dst_h;
        dsurf.pixelFormat           = KGAL_PIXEL_FORMAT_YUY2;
        drect.x = info.dst_x;
        drect.y = info.dst_y;
        drect.w = info.dst_w;
        drect.h = info.dst_h;

        if(info.src_fmt == KGAL_PIXEL_FORMAT_NV12 || info.src_fmt == KGAL_PIXEL_FORMAT_NV16)
        {
                ssurf.bpp         = 16;
                ssurf.pitch = info.src_w*2;
        }
        else if(info.src_fmt == KGAL_PIXEL_FORMAT_RGB888)//KGAL_PIXEL_FORMAT_YUV444
        {
                ssurf.bpp         = 24;
                ssurf.pitch = info.src_w*3;
                info.src_phyaddr_uv = info.dst_phyaddr_uv; //just init, no use in SE driver.
        }

        if(info.dst_fmt == KGAL_PIXEL_FORMAT_NV12)
        {
                dsurf.bpp         = 16;
                dsurf.pitch = info.dst_w;
        }
        else if(info.dst_fmt == KGAL_PIXEL_FORMAT_YUY2)
        {
                dsurf.bpp         = 16;
                dsurf.pitch = info.dst_w*2;
        }

        status = KGAL_NV12_StretchBlit(&ssurf, &srect, &dsurf, &drect, &sflag, &sblend, info.src_phyaddr_uv, info.dst_phyaddr_uv);


        return status;
#else
        //rtd_pr_i3ddma_err("[%s] need enable CONFIG_RTK_KDRV_SE",__func__);
        return 0;
#endif
}

void scaler_hdmi_4k120_UV_interleave(void)
{
	SE_NN_info info;
	unsigned int u_addr, v_addr, se_addr;

	unsigned int width;
	unsigned int length;

	memset(&info, 0, sizeof(SE_NN_info));

	width = get_i3ddma_4k120_width(); //1920
	length = get_i3ddma_4k120_length(); //2160
	get_i3ddma_4k120_se_addr(width, &u_addr, &v_addr, &se_addr);
	//rtd_pr_i3ddma_emerg("se_task:width=%d,length=%d,u_addr=%x,v_addr=%x,se_addr=%x\n",width,length,u_addr,v_addr,se_addr);

	info.src_x = 0;
	info.src_y = 0;
	info.src_w = width;
	info.src_h = length/2;
	info.src_phyaddr = u_addr;
	info.src_phyaddr_uv = v_addr;
	info.src_fmt = KGAL_PIXEL_FORMAT_NV16;

	info.dst_x = 0;
	info.dst_y = 0;
	info.dst_w = width;
	info.dst_h = length/2;
	info.dst_phyaddr = se_addr;
	info.dst_phyaddr_uv = se_addr;
	info.dst_fmt = KGAL_PIXEL_FORMAT_YUY2;

	scaler_SE_TMDS_stretch_Proc(info);
}
#endif

// FixMe
#if defined CONFIG_SCALER_BRING_UP || defined BUILD_QUICK_SHOW
typedef struct
{
	unsigned int HTotal;		// h_threshold
	unsigned int VTotal;		// v_threshold
	unsigned int HWidth;		// h_active
	unsigned int VHeight;		// v_active
	unsigned int HStartPos;		// h_active_start
	unsigned int VStartPos;		// v_active_start
	unsigned int isProg;		// progressive or interlace
	unsigned int SampleRate;	// sample rate in 100Hz
	unsigned int FrameRate;	//Frame Rate, //vfreq unit 0.001Hz
    unsigned char hdrSource;
} VO_TIMING ;


typedef enum _DOLBY_HDMI_MODE {
     DOLBY_HDMI_OFF		= 0x0,
     DOLBY_HDMI_IDMA_INIT	= 0x1,
     DOLBY_HDMI_CAPTURE	= 0x2,
     DOLBY_HDMI_VO_RUN		= 0x3
} DOLBY_HDMI_MODE;

//VO_DMEM *VO_Dmem=NULL;
//VO_DATA *vo=NULL;
VODMA_HDMI_AUTO_MODE hdmi_autoMode=0;
static unsigned int vsyncgen_pixel_freq=0;



#define VO_MAIN_DCU_INDEX_Y     0
#define VO_MAIN_DCU_INDEX_C1    1
#define VO_MAIN_DCU_INDEX_C2    2
#define VO_MAIN_SEQ_ADDR        ((unsigned int)(0x08200000))
#define PCR_PLL			27000000
#define APLL			108000000
#define IVS_HDR_START 1
#define IVS_START 4
#define IVS_WIDTH 2
#define INT_POS 8

extern VODMA_MODE Mode_Decision(unsigned int width, unsigned int height, bool isProg);
extern HDR_MODE HDR_DolbyVision_Get_CurDolbyMode(void);
extern DOLBY_HDMI_MODE HDR_DolbyVision_Get_CurHdmiMode(void);
extern void Scaler_I2rnd_set_display(unsigned char display);

void dma_set_v1(VO_VIDEO_PLANE plane, VO_TIMING *tmode,VIDEO_RPC_VOUT_CONFIG_HDMI_3D *HDMI_INFO , VODMA_DMA_MODE dma_mode, VODMA_DDOMAIN_CHROMA Ddomain_Chroma)
{
    vodma_vodma_v1_dcfg_RBUS        DMA_v1_dcfg;        //0xb800_5000
    vodma_vodma_v1_dsize_RBUS       DMA_v1_dsize;       //0xb800_5004
    vodma_vodma_v1_line_blk1_RBUS        V1_y_line_sel ;        //0xb800_5008
    vodma_vodma_v1_line_blk2_RBUS        V1_c_line_sel ;        //0xb800_500C
    vodma_vodma_v1_blk1_RBUS        DMA_v1_hoffset;     //0xb800_5010
    vodma_vodma_v1_blk2_RBUS        DMA_v1_voffset;     //0xb800_5014
    //vodma_vodma_v1_blk3_RBUS      DMA_v1_voffset2;    //0xb800_5018
    //vodma_vodma_v1_blk4_RBUS      DMA_v1_hoffset2;        //0xb800_5010
    //vodma_vodma_v1_blk5_RBUS      DMA_v1_hoffset3;        //0xb800_5010
    vodma_vodma_v1_seq_3d_l1_RBUS DMA_v1_seq_3d_l1; //0xb800_501c
    vodma_vodma_v1_seq_3d_r1_RBUS DMA_v1_seq_3d_r1; //0xb800_5020
    vodma_vodma_v1_seq_3d_l2_RBUS DMA_v1_seq_3d_l2; //0xb800_5024
    vodma_vodma_v1_seq_3d_r2_RBUS DMA_v1_seq_3d_r2; //0xb800_5028
    //vodma_vodma_v1_seq_3d_l3_RBUS DMA_v1_seq_3d_l3;   //0xb800_51c0
    //vodma_vodma_v1_seq_3d_r3_RBUS DMA_v1_seq_3d_r3;   //0xb800_51c4
    //vodma_vodma_v1_seq_3d_l4_RBUS DMA_v1_seq_3d_l4;   //0xb800_51c8
    //vodma_vodma_v1_seq_3d_r4_RBUS DMA_v1_seq_3d_r4;   //0xb800_51cc
    //vodma_vodma_v1_v_flip_RBUS    DMA_v1_seq_v_flip;  //0xb800_502c
    vodma_vodma_v1chroma_fmt_RBUS   V1_C_up_fmt;        //0xb800_5030
    vodma_vodma_v1_cu_RBUS      V1_C_up_mode420;    //0xb800_5034
    vodma_vodma_v1422_to_444_RBUS   V1_C_up_mode422;    //0xb800_5038
    //vodma_vodma_line_select_RBUS  line_sel;           //0xb800_5040
    //vodma_vodma_quincunx_setting_RBUS quincunx_setting;   //0xb800_50e0

    DMA_v1_dcfg.merge_forward = 0;
    DMA_v1_dcfg.merge_backward = 0;
    //DMA_v1_dcfg.pd_switch = 0;
    //DMA_v1_dcfg.pd_seq = 0;
    DMA_v1_dcfg.film_condition = 0;

    DMA_v1_dcfg.field_mode = 0;
    DMA_v1_dcfg.field_fw = 0;
    DMA_v1_dcfg.l_flag_fw = 0;
    DMA_v1_dcfg.force_2d_en = 0;
    DMA_v1_dcfg.force_2d_sel = 0;
    DMA_v1_dcfg.dma_auto_mode = HDMI_INFO->hw_auto_mode;

    DMA_v1_dcfg.seq_data_pack_type = 0; //1:field based, 0:line based
    DMA_v1_dcfg.seq_data_width = (HDMI_INFO->data_bit_width == 8)? 0: 1 ; // 8bits, 10bits;

    DMA_v1_dcfg.double_chroma = 0;
    DMA_v1_dcfg.uv_off =  0;
    DMA_v1_dcfg.pxl_swap_sel = 0;
    DMA_v1_dcfg.keep_go_en = 0;
    DMA_v1_dcfg.dma_state_reset_en = 1;
    DMA_v1_dcfg.v_flip_en = 0;
    DMA_v1_dcfg.chroma_swap_en = 0;
    DMA_v1_dcfg.double_pixel_mode = 0;

    DMA_v1_dcfg.dma_mode = dma_mode;        //0:seq 1:blk
    DMA_v1_dcfg.vodma_go = 1;

    if(HDMI_INFO->enable_comps && ((tmode->HWidth % 32) != 0)){
        DMA_v1_dsize.p_y_len = tmode->HWidth + (32 - (tmode->HWidth % 32));
    }else{
        DMA_v1_dsize.p_y_len = tmode->HWidth ;
    }
    DMA_v1_dsize.p_y_len = tmode->HWidth;   //width
    DMA_v1_dsize.p_y_nline = tmode->VHeight; //height

    V1_y_line_sel.y_auto_line_step = 1; // 0: FW mode, 1: HW mode
    V1_y_line_sel.y_line_step = 0;
    V1_c_line_sel.c_auto_line_step = 1; // 0: FW mode, 1: HW mode
    V1_c_line_sel.c_line_step = 0;
    V1_y_line_sel.y_line_select = 0;
    V1_c_line_sel.c_line_select = 0;

    if (dma_mode == DMA_BLOCK_MODE) {
        //DMA_v1_YC.p_v_ads = VO_MAIN_DCU_INDEX_C2;
        //DMA_v1_YC.p_u_ads = VO_MAIN_DCU_INDEX_C1;
        //DMA_v1_YC.p_y_ads = VO_MAIN_DCU_INDEX_Y;

        //DMA_v1_hoffset3.v_h_offset = 0;
        //DMA_v1_hoffset3.u_h_offset = 0;
        //DMA_v1_hoffset.y_ha_shift = 0;
        //DMA_v1_hoffset2.y_h_offset = 0;

        //DMA_v1_voffset.p_y_offset = 0;
        //DMA_v1_voffset.p_u_offset = 0;

        //DMA_v1_voffset2.p_v_offset = 0;
    }
    else {

#if 0
        vodma->V1_seq_3D_L1.st_adr = (HDMI_INFO->r2_st_adr & 0x7FFFFFF0) >> 4;
        vodma->V1_seq_3D_R1.st_adr = (HDMI_INFO->l1_st_adr & 0x7FFFFFF0) >> 4;
        vodma->V1_seq_3D_L2.st_adr = (HDMI_INFO->r1_st_adr & 0x7FFFFFF0) >> 4;
        vodma->V1_seq_3D_R2.st_adr = (HDMI_INFO->l2_st_adr & 0x7FFFFFF0) >> 4;
#endif
//      DMA_v1_staddr.st_adr = (HDMI_INFO->r2_st_adr & 0x7FFFFFF0) >> 4;

        DMA_v1_seq_3d_l1.st_adr = (HDMI_INFO->l1_st_adr & 0x7FFFFFF0) >> 4;
        DMA_v1_seq_3d_r1.st_adr = (HDMI_INFO->r1_st_adr & 0x7FFFFFF0) >> 4;
        DMA_v1_seq_3d_l2.st_adr = (HDMI_INFO->l2_st_adr & 0x7FFFFFF0) >> 4;
        DMA_v1_seq_3d_r2.st_adr = (HDMI_INFO->r2_st_adr & 0x7FFFFFF0) >> 4;
//      DMA_v1_seq_3d_l3.st_adr = (VO_MAIN_SEQ_ADDR) >> 4;
//      DMA_v1_seq_3d_r3.st_adr = (VO_MAIN_SEQ_ADDR) >> 4;
//      DMA_v1_seq_3d_l4.st_adr = (VO_MAIN_SEQ_ADDR) >> 4;
//      DMA_v1_seq_3d_r4.st_adr = (VO_MAIN_SEQ_ADDR) >> 4;

        //DMA_v1_hoffset3.v_h_offset = 0;
        //DMA_v1_hoffset3.u_h_offset = 0;
        //DMA_v1_hoffset.y_ha_shift = 0;
        //DMA_v1_hoffset2.y_h_offset = 0;

        //DMA_v1_voffset.p_y_offset = 0;
        //DMA_v1_voffset.p_u_offset = 0;

        //DMA_v1_voffset2.p_v_offset = 0;

        //DMA_v1_seq_v_flip.line_step = 0 ;
        //DMA_v1_seq_v_flip.line_step = ( (tmode->VHeight * (Ddomain_Chroma_fmt == DDOMAIN_FMT_422?2:3) * (DMA_v1_dcfg.seq_data_width?10:8)) + 127)/128;
    }

    /*
     * chroma upsampling
     * 0: YUV 420->422 (blk only)
     * 1: YUV 420->444 (blk only)
     * 2: YUV 422->444 (seq or blk)
     * 3: YUV 422 bypass (seq or blk)
     * 4: YUV 444 bypass (seq only)
     * 5: ARGB8888 444 bypass (seq only)
     * 6: RGB888 444 bypass (seq only)
     * 7: RGB565 444 bypass (seq only)
     * 8: YUV 444 bypass (blk only)
     * 9: YUV 422v->444 (blk only)
     */
    V1_C_up_fmt.g_position = 0;
    if(HDMI_INFO->enable_comps)
        V1_C_up_fmt.seq_color_swap = (HDMI_INFO->chroma_fmt == VO_CHROMA_YUV422)? 2: 5 ; // 2: for 422, 5: for 444
    else
        V1_C_up_fmt.seq_color_swap = (HDMI_INFO->chroma_fmt == VO_CHROMA_YUV422)? 2: (HDMI_INFO->chroma_fmt == VO_CHROMA_YUV444)? 3: 5; //YUV422: 2 // YUV444: 3 //ARGB8888,RGB888,RGB565: 5
    V1_C_up_fmt.alpha_position = 0;
    if (dma_mode == DMA_SEQUENTIAL_MODE) {
        if (Ddomain_Chroma == DDOMAIN_FMT_422) // ddomain 422
        {
            if (HDMI_INFO->chroma_fmt == VO_CHROMA_YUV411)
                V1_C_up_fmt.encfmt = 6; //411->422
            else
                V1_C_up_fmt.encfmt = 3; //422 bypass
        }
        else // ddomain 444
        {
            if (HDMI_INFO->chroma_fmt == VO_CHROMA_YUV444 || HDMI_INFO->chroma_fmt == VO_CHROMA_RGB888)
                V1_C_up_fmt.encfmt = 4; //444 bypass
            else if (HDMI_INFO->chroma_fmt == VO_CHROMA_ARGB8888)
                V1_C_up_fmt.encfmt = 5; //ARGB8888 444 bypass
            else if (HDMI_INFO->chroma_fmt == VO_CHROMA_YUV411)
                V1_C_up_fmt.encfmt = 0xa;   //YUV411 444 bypass
            else if (HDMI_INFO->chroma_fmt == VO_CHROMA_RGB565)
                V1_C_up_fmt.encfmt = 7; //RGB565 444 bypass
            else
                V1_C_up_fmt.encfmt = 2; //422->444
        }
    }
    else {
        if (Ddomain_Chroma == DDOMAIN_FMT_422) // ddomain 422
        {
            if (HDMI_INFO->chroma_fmt== VO_CHROMA_YUV422)
                V1_C_up_fmt.encfmt = 3; //422 bypass
            else
                V1_C_up_fmt.encfmt = 0; //420->422
        }
        else // ddomain 444
        {
            V1_C_up_fmt.encfmt = 1; //420->444
        }
    }
    V1_C_up_fmt.encfmt = 4;


    V1_C_up_mode420.mode = 0;


    V1_C_up_mode422.fir422_sel = 0;

#if 0
    //even or odd line selection
    line_sel.y1_line_select = !tmode->isProg;
    line_sel.c1_line_select = !tmode->isProg;

    quincunx_setting.hori_drop_mode = 0 ;
    quincunx_setting.hori_drop_en = 0 ;
    quincunx_setting.quincunx_mode_src_sel = HDMI_INFO->hw_auto_mode;
    quincunx_setting.quincunx_int_mode = HDMI_INFO->quincunx_en ;
    quincunx_setting.quincunx_int_en = 0 ;
    quincunx_setting.quincunx_mode_fw = HDMI_INFO->quincunx_mode_fw ;
#endif
    if (plane == VO_VIDEO_PLANE_V1)
    {
        rtd_outl(VODMA_VODMA_V1_DCFG_reg, DMA_v1_dcfg.regValue);
        rtd_pr_i3ddma_info("[%s %d]0x%x\n", __FUNCTION__, __LINE__, rtd_inl(VODMA_VODMA_V1_DCFG_reg));
        rtd_outl(VODMA_VODMA_V1_DSIZE_reg, DMA_v1_dsize.regValue);
        //rtd_outl(VODMA_VODMA_VD1_ADS_reg, DMA_v1_YC.regValue);
        rtd_outl(VODMA_VODMA_V1_BLK1_reg, DMA_v1_hoffset.regValue);
        rtd_outl(VODMA_VODMA_V1_BLK2_reg, DMA_v1_voffset.regValue);
        rtd_outl(VODMA_VODMA_V1_LINE_BLK1_reg, V1_y_line_sel.regValue);
        rtd_outl(VODMA_VODMA_V1_LINE_BLK1_reg, V1_c_line_sel.regValue);
        //rtd_outl(VODMA_VODMA_V1_BLK3_reg, DMA_v1_voffset2.regValue);
       // rtd_outl(VODMA_VODMA_V1_BLK4_reg, DMA_v1_hoffset2.regValue);
       // rtd_outl(VODMA_VODMA_V1_BLK5_reg, DMA_v1_hoffset3.regValue);
        //rtd_outl(VODMA_VODMA_V1_SEQ_reg, DMA_v1_staddr.regValue);
        rtd_outl(VODMA_VODMA_V1_SEQ_3D_L1_reg, DMA_v1_seq_3d_l1.regValue);
        rtd_outl(VODMA_VODMA_V1_SEQ_3D_R1_reg, DMA_v1_seq_3d_r1.regValue);
        rtd_outl(VODMA_VODMA_V1_SEQ_3D_L2_reg, DMA_v1_seq_3d_l2.regValue);
        rtd_outl(VODMA_VODMA_V1_SEQ_3D_R2_reg, DMA_v1_seq_3d_r2.regValue);
        rtd_outl(VODMA_VODMA_V1_SEQ_3D_L3_reg, DMA_v1_seq_3d_l1.regValue);
        rtd_outl(VODMA_VODMA_V1_SEQ_3D_R3_reg, DMA_v1_seq_3d_r1.regValue);
        rtd_outl(VODMA_VODMA_V1_SEQ_3D_L4_reg, DMA_v1_seq_3d_l2.regValue);
        rtd_outl(VODMA_VODMA_V1_SEQ_3D_R4_reg, DMA_v1_seq_3d_r2.regValue);
        //rtd_outl(VODMA_VODMA_V1_V_FLIP_reg, DMA_v1_seq_v_flip.regValue);
        rtd_outl(VODMA_VODMA_V1CHROMA_FMT_reg, V1_C_up_fmt.regValue);
        rtd_outl(VODMA_VODMA_V1_CU_reg, V1_C_up_mode420.regValue);
        rtd_outl(VODMA_VODMA_V1422_TO_444_reg, V1_C_up_mode422.regValue);
        //rtd_outl(VODMA_VODMA_LINE_SELECT_reg, line_sel.regValue);
        //rtd_outl(VODMA_VODMA_quincunx_setting_reg, quincunx_setting.regValue);
    }

}


void timinggen_set_v1(VO_VIDEO_PLANE plane, VO_TIMING *tmode)
{
    vodma_vodma_v1sgen_RBUS             V1_syncGen ;
    vodma_vodma_v1int_RBUS          V1_syncGen_mode ;
    vodma_vodma_v1vgip_ivs1_RBUS        V1_vgipPos_topIVS ;
    vodma_vodma_v1vgip_ivs2_RBUS        V1_vgipPos_botIVS ;
    vodma_vodma_v1vgip_ihs_RBUS         V1_vgipPos_IHS ;
    vodma_vodma_v1vgip_fd_RBUS      V1_vgipPos_topFld ;
    vodma_vodma_v1vgip_fd2_RBUS         V1_vgipPos_botFld ;
    vodma_vodma_v1vgip_hact_RBUS        V1_vgipPos_HACT ;
    vodma_vodma_v1vgip_vact1_RBUS       V1_vgipPos_topVACT ;
    vodma_vodma_v1vgip_vact2_RBUS       V1_vgipPos_botVACT ;
    vodma_vodma_v1vgip_hbg_RBUS     V1_vgipPos_HBG ;
    vodma_vodma_v1vgip_vbg1_RBUS        V1_vgipPos_topVBG ;
    vodma_vodma_v1vgip_vbg2_RBUS        V1_vgipPos_botVBG ;
    vodma_vodma_v1vgip_bg_clr_RBUS  V1_vgipPos_BGCLR ;
    vodma_vodma_v1vgip_intpos_RBUS  V1_vgipPos_intrpt ;

    unsigned int HTotal,VTotal, HActStart, HActEnd, VActStart, VActEnd;

    HTotal      = tmode->HTotal;
    VTotal      = tmode->VTotal;
    HActStart   = tmode->HStartPos;
    HActEnd     = tmode->HStartPos + tmode->HWidth;
    VActStart   = tmode->VStartPos;
    VActEnd     = tmode->VStartPos + tmode->VHeight;

    V1_syncGen.vthr_rst1 = 0;
    V1_syncGen.v_thr = 0xfff; //VTotal;
    V1_syncGen.h_thr = HTotal;

    V1_syncGen_mode.vgip_en = 1;
    V1_syncGen_mode.interlace = !tmode->isProg;
    V1_syncGen_mode.top_fld_indc = 0;
    V1_syncGen_mode.v_thr = 0xfff;

    /* vsync width top */
	V1_vgipPos_topIVS.v_st = (tmode->hdrSource? IVS_HDR_START: IVS_START);
	V1_vgipPos_topIVS.v_end = V1_vgipPos_topIVS.v_st + IVS_WIDTH;
	/* vsync width bot */
	V1_vgipPos_botIVS.v_st = (tmode->hdrSource? IVS_HDR_START: IVS_START);
	V1_vgipPos_botIVS.v_end = V1_vgipPos_botIVS.v_st + IVS_WIDTH;


    V1_vgipPos_IHS.h_st = 0;

    //should cover active window
    V1_vgipPos_topFld.v_end = 0xfff;
    V1_vgipPos_topFld.v_st = 0xfff;

    //should cover active window
    V1_vgipPos_botFld.v_end = 0xfff;
    V1_vgipPos_botFld.v_st = (V1_vgipPos_topIVS.v_st - 1)? (V1_vgipPos_topIVS.v_st - 1): V1_vgipPos_topIVS.v_st;

    V1_vgipPos_HACT.h_end = HActEnd;
    V1_vgipPos_HACT.h_st = HActStart;

    /* v active move -20 */
    V1_vgipPos_topVACT.v_end = VActEnd;
    V1_vgipPos_topVACT.v_st = VActStart;

    /* v active move -20 */
    /* no matter progressive or interlace, the below reg must be assigned */
    V1_vgipPos_botVACT.v_end = V1_vgipPos_topVACT.v_end;
    V1_vgipPos_botVACT.v_st = V1_vgipPos_topVACT.v_st;

    V1_vgipPos_HBG.h_end = HActEnd;
    V1_vgipPos_HBG.h_st = HActStart;

    V1_vgipPos_topVBG.v_end = VActEnd;
    V1_vgipPos_topVBG.v_st = VActStart;

    V1_vgipPos_botVBG.v_end = VActEnd;
    V1_vgipPos_botVBG.v_st = VActStart;

//  V1_vgipPos_BGCLR.bg_y = 0;
//  V1_vgipPos_BGCLR.bg_u = 0;
//  V1_vgipPos_BGCLR.bg_v = 0;

    V1_vgipPos_intrpt.v2 = 8;
    V1_vgipPos_intrpt.v1 = 8;

    if (plane == VO_VIDEO_PLANE_V1)
    {
        rtd_outl(VODMA_VODMA_V1SGEN_reg, V1_syncGen.regValue);
        rtd_outl(VODMA_VODMA_V1INT_reg, V1_syncGen_mode.regValue);
        rtd_outl(VODMA_VODMA_V1VGIP_IVS1_reg, V1_vgipPos_topIVS.regValue);
        rtd_outl(VODMA_VODMA_V1VGIP_IVS2_reg, V1_vgipPos_botIVS.regValue);
        rtd_outl(VODMA_VODMA_V1VGIP_IHS_reg, V1_vgipPos_IHS.regValue);
        rtd_outl(VODMA_VODMA_V1VGIP_FD_reg, V1_vgipPos_topFld.regValue);
        rtd_outl(VODMA_VODMA_V1VGIP_FD2_reg, V1_vgipPos_botFld.regValue);
        rtd_outl(VODMA_VODMA_V1VGIP_HACT_reg, V1_vgipPos_HACT.regValue);
        rtd_outl(VODMA_VODMA_V1VGIP_VACT1_reg, V1_vgipPos_topVACT.regValue);
        rtd_outl(VODMA_VODMA_V1VGIP_VACT2_reg, V1_vgipPos_botVACT.regValue);
        rtd_outl(VODMA_VODMA_V1VGIP_HBG_reg, V1_vgipPos_HBG.regValue);
        rtd_outl(VODMA_VODMA_V1VGIP_VBG1_reg, V1_vgipPos_topVBG.regValue);
        rtd_outl(VODMA_VODMA_V1VGIP_VBG2_reg, V1_vgipPos_botVBG.regValue);
        rtd_outl(VODMA_VODMA_V1VGIP_BG_CLR_reg, V1_vgipPos_BGCLR.regValue);
        rtd_outl(VODMA_VODMA_V1VGIP_INTPOS_reg, V1_vgipPos_intrpt.regValue);
    }
}


void vsyncgen_set_v1(VO_VIDEO_PLANE plane, VO_TIMING *tmode)
{
    vodma_vodma_clkgen_RBUS clkgen;
    vodma_vodma_pvs_free_RBUS       pvs_free;
    vodma_vodma_pvs_ctrl_RBUS       pvs_ctrl;
    vodma_vodma_pvs0_gen_RBUS       pvs0;
    vodma_vodma_pvs_width_ctrl_RBUS pvs_width_ctrl;

    unsigned int vodmapll, vodmapll0, freerun_clk;
    unsigned int nMCode, nNCode, nOCode, regValue, busclk = 0;
    //int div_n = 0;
    int ratio_n_off = 0;

    IV_SRC timgen_src = IV_SRC_SEL_HDMIDMAVS;//IV_SRC_SEL_PIXELCLOCK; //PLLDDS, MAINIVS, MAINDVS, PIXELCLOCK ///change i3ddma
    VODMA_CLK_SRC vodma_clk_src = VODMA_CLK_SEL_PLLBUS_2;
    PVS0_FREE_CLK_SRC free_clk_src = PVS0_FREE_CLK_SEL_CRYSTALCLK; //PVS0_FREE_CLK_SEL_PCRCLK, PVS0_FREE_CLK_SEL_PIXELCLK
    //set pcr pll
#if 0
    sys_pll_2_bus1_RBUS PLL_2_BUS1_REG;
    PLL_2_BUS1_REG.regValue = rtd_inl(SYSTEM_SYS_PLL_2_BUS1_reg);
    PLL_2_BUS1_REG.pllbus_m_h = 14;
    PLL_2_BUS1_REG.pllbus_n_h = 0;
    rtd_outl(SYSTEM_SYS_PLL_2_BUS1_reg, PLL_2_BUS1_REG.regValue);
    sys_pll_2_bus3_RBUS PLL_2_BUS3_REG;
    PLL_2_BUS3_REG.regValue = rtd_inl(SYSTEM_SYS_PLL_3_BUS1_reg);
    PLL_2_BUS3_REG.pcr_psaud_rstb = 1;
    PLL_2_BUS3_REG.plldds_pwdn = 0;
    PLL_2_BUS3_REG.plldds_rstb = 1;
    PLL_2_BUS3_REG.plldds_oeb = 0;
    PLL_2_BUS3_REG.plldds_ddsen = 1;
    rtd_outl(SYSTEM_SYS_PLL_3_BUS1_reg, PLL_2_BUS3_REG.regValue);
#endif

    /* vodma clk gen */
    clkgen.vodma_enclk = 1;
    clkgen.vodma_clk_sel = vodma_clk_src;
    clkgen.vodma_clk_div2_en = !tmode->isProg;
    clkgen.vodma_clk_div_n = 0;
    clkgen.vodma_clk_ratio_n_off = 0;

    switch(clkgen.vodma_clk_sel) {
    case VODMA_CLK_SEL_ADCCLK:
        vodmapll = APLL;
        break;
    case VODMA_CLK_SEL_HDMICLK:
    case VODMA_CLK_SEL_PLLBUS_2:
    default:
#if 1
        regValue = rtd_inl(0xB8000460);
        nMCode = ((regValue & 0x000ff000)>>12);
        nNCode = ((regValue & 0x00300000)>>20);
        nOCode = ((regValue & 0x00000180)>>7);

        busclk = 27000000*(nMCode+2)/(nNCode+1)/(nOCode+1);

        //ROSPrintfInternal("PLL_VODMA m: %d, n: %d, o: %d, clk: %d\n", nMCode, nNCode, nOCode, busclk);

        vodmapll = busclk ;
#endif
        break;
    }
    vodmapll0 = vodmapll ;

#if 0 // [ML4BU-251] HW issue, vodma_clk_div_n need keep in zero to avoid output clock error
    if((vodmapll/2)>(tmode->SampleRate*10000)){
        div_n = (vodmapll/(tmode->SampleRate*10000))-1;
        vodmapll = vodmapll / (div_n + 1);
        clkgen.vodma_clk_div_n = div_n;
        //ROSPrintfInternal("div_n: %d, clk: %d\n", clkgen.vodma_clk_div_n, vodmapll);
    }
#endif

#if 1 // [ML4BU-251] HW issue, vodma_clk_div_n need keep in zero to avoid output clock error
    if ((vodmapll/64*63)>(tmode->SampleRate*10000)) {
        ratio_n_off = 64 - (tmode->SampleRate*64/(vodmapll/10000)) - 1;
        vodmapll = (vodmapll/64) * (64 - ratio_n_off);
        clkgen.vodma_clk_ratio_n_off = ratio_n_off ;
        //ROSPrintfInternal("ratio_n_off: %d, clk: %d\n", clkgen.vodma_clk_ratio_n_off, vodmapll);
    }
#endif

    if (clkgen.vodma_clk_div2_en)
    {
        vodmapll = vodmapll / 2;
        //ROSPrintfInternal("div2: %d, clk: %d\n", clkgen.vodma_clk_div2_en, vodmapll);
    }

    clkgen.gating_src_sel = 0;
    clkgen.en_fifo_full_gate = 0;
    clkgen.en_fifo_empty_gate = 0 ;


    /* pvs freerun */
    pvs_free.pvs0_free_en = 1;      //0: disable(clk from vodma), 1:enable(clk from PCK_CLK)
    pvs_free.pvs0_free_vs_reset_en = 0;
    pvs_free.pvs0_free_period_update = 1;
    pvs_ctrl.pvs0_free_clk_sel = free_clk_src;
    switch(pvs_ctrl.pvs0_free_clk_sel) {
    case PVS0_FREE_CLK_SEL_PIXELCLK:
        freerun_clk = vodmapll;
        break;
    case PVS0_FREE_CLK_SEL_PCRACLK:
    case PVS0_FREE_CLK_SEL_PCRBCLK:
    case PVS0_FREE_CLK_SEL_CRYSTALCLK:
    default:
        freerun_clk = PCR_PLL ;
        break;
    }
    vsyncgen_pixel_freq=vodmapll;
    pvs_free.pvs0_free_period = (freerun_clk / ((int)tmode->FrameRate) ) * 1000;  //vfreq unit 0.001Hz


    pvs_width_ctrl.pvs0_vs_width = pvs_free.pvs0_free_period / tmode->VTotal ;


    pvs_ctrl.pvs0_free_l_flag_en = 0 ;
    pvs_ctrl.pvs0_free_vs_inv_en = 0 ;
    pvs_ctrl.pvs0_free_vs_sel = 0 ;
    pvs_ctrl.mask_one_vs = 0 ;
    //pvs_ctrl.dummy18005308_29_11 = 0x40000;

    /* pvs0 gen */
    pvs0.en_pvgen = 1;
    pvs0.en_pfgen = 1;
    pvs0.pfgen_inv = 0;
    pvs0.iv_inv_en = 0;
    pvs0.iv_src_sel = timgen_src;
    pvs0.iv2pv_dly = 2;

    if (plane == VO_VIDEO_PLANE_V1)
    {
        rtd_outl(VODMA_VODMA_CLKGEN_reg, clkgen.regValue);
        rtd_outl(VODMA_VODMA_PVS_Free_reg, pvs_free.regValue);
        rtd_outl(VODMA_VODMA_PVS_WIDTH_CTRL_reg, pvs_width_ctrl.regValue);
        rtd_outl(VODMA_VODMA_PVS_CTRL_reg, pvs_ctrl.regValue);
        rtd_outl(VODMA_VODMA_PVS0_Gen_reg, pvs0.regValue);
    }
}

#if 0  //mark unused function
static void VODMA_HDMI_Set_AutoMode(VODMA_HDMI_AUTO_MODE autoMode)
{

	hdmi_autoMode = autoMode;
	return;
}
#endif

VODMA_HDMI_AUTO_MODE VODMA_HDMI_Get_AutoMode(void)
{
	return hdmi_autoMode;
}

void VODMA_EnableDoubleBuf(unsigned char enable)
{
    vodma_vodma_reg_db_ctrl_RBUS vodma_vodma_reg_db_ctrl_reg;
    vodma_vodma_reg_db_ctrl_reg.regValue = rtd_inl(VODMA_VODMA_REG_DB_CTRL_reg);

    if(enable)
    {
        vodma_vodma_reg_db_ctrl_reg.vodma_db_en = 1;
        vodma_vodma_reg_db_ctrl_reg.vodmavsg_db_en = 1;
    }
    else
    {
        vodma_vodma_reg_db_ctrl_reg.vodma_db_en = 0;
        vodma_vodma_reg_db_ctrl_reg.vodmavsg_db_en = 0;
    }

    rtd_outl(VODMA_VODMA_REG_DB_CTRL_reg, vodma_vodma_reg_db_ctrl_reg.regValue);
}

void VODMA_DBApply(unsigned char apply)
{
    vodma_vodma_reg_db_ctrl_RBUS vodma_vodma_reg_db_ctrl_reg;
    vodma_vodma_reg_db_ctrl_reg.regValue = rtd_inl(VODMA_VODMA_REG_DB_CTRL_reg);

    vodma_vodma_reg_db_ctrl_reg.vodma_db_rdy= apply;
    vodma_vodma_reg_db_ctrl_reg.vodmavsg_db_rdy = apply;

    rtd_outl(VODMA_VODMA_REG_DB_CTRL_reg, vodma_vodma_reg_db_ctrl_reg.regValue);

}

extern void Config_VO_Dispinfo(SLR_VOINFO *pVOInfo);
void VODMA_SetVOInfo(unsigned int ch, VO_TIMING *timemode, unsigned char source,VODMA_DDOMAIN_CHROMA Ddomain_Chroma,VO_COLORSPACE colorspace)
{
	//VO_DATA *vo = &vodma_data[0];
		SLR_VOINFO VOInfo;
		//unsigned char isRGB = 0;

		rtd_pr_i3ddma_debug("[vo] vomda_set_voinfo\n");

		memset(&VOInfo, 0, sizeof(SLR_VOINFO));

		/*isRGB = ((vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_ARGB) ||
			 (vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_ABGR) ||
			 (vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_BGR) ||
			 (vo->image[ch].pixel_format == HAL_VO_PIXEL_FORMAT_RGB));*/

		VOInfo.plane = VO_VIDEO_PLANE_V1;
		VOInfo.port = ch;
		VOInfo.mode = Mode_Decision(timemode->HWidth, timemode->VHeight, timemode->isProg);
		VOInfo.h_total = timemode->HTotal;
		VOInfo.v_total = (timemode->isProg) ? timemode->VTotal : timemode->VTotal/2;
		VOInfo.h_start = timemode->HStartPos;
		VOInfo.v_start = timemode->VStartPos - 2 - 1; /* IVS_START = 2 */
		VOInfo.h_width = timemode->HWidth;
		VOInfo.v_length = timemode->VHeight;
		VOInfo.h_freq = vsyncgen_pixel_freq / timemode->HTotal / 100;
		VOInfo.v_freq = timemode->FrameRate/100;
		VOInfo.pixel_clk = vsyncgen_pixel_freq;
		VOInfo.progressive = timemode->isProg;
		VOInfo.chroma_fmt = (VODMA_DDOMAIN_CHROMA_FMT) Ddomain_Chroma;
		VOInfo.i_ratio =  0; /* 0. 16:9, 1. 4:3 //480*100/720=66.67 */
		VOInfo.source =  0; /* 3 for rgb, or just set 0 */
		VOInfo.isJPEG = 0;
		VOInfo.vdec_source = VOInfo.isJPEG;
		VOInfo.colorspace = colorspace;
		VOInfo.mode_3d = 0;
		VOInfo.force2d = 0;
		VOInfo.afd = 8;
		VOInfo.pixelAR_hor = 1;
		VOInfo.pixelAR_ver = 1;
		VOInfo.launcher = 0;
		VOInfo.src_h_wid = timemode->HWidth;
		VOInfo.src_v_len = timemode->VHeight;

		rtd_pr_i3ddma_info("\nSet Voinfo:\n");
		rtd_pr_i3ddma_info("plane=%d\n", VOInfo.plane);
		rtd_pr_i3ddma_info("port=%d\n", VOInfo.port);
		rtd_pr_i3ddma_info("mode=%d\n", VOInfo.mode);
		rtd_pr_i3ddma_info("h_start=%d\n", VOInfo.h_start);
		rtd_pr_i3ddma_info("v_start=%d\n", VOInfo.v_start);
		rtd_pr_i3ddma_info("h_width=%d\n", VOInfo.h_width);
		rtd_pr_i3ddma_info("v_length=%d\n", VOInfo.v_length);
		rtd_pr_i3ddma_info("h_total=%d\n", VOInfo.h_total);
		rtd_pr_i3ddma_info("v_total=%d\n", VOInfo.v_total);
		rtd_pr_i3ddma_info("chroma_fmt=%d\n", VOInfo.chroma_fmt);
		rtd_pr_i3ddma_info("progressive=%d\n", VOInfo.progressive);
		rtd_pr_i3ddma_info("h_freq=%d\n", VOInfo.h_freq);
		rtd_pr_i3ddma_info("v_freq=%d\n", VOInfo.v_freq);
		rtd_pr_i3ddma_info("pixel_clk=%d\n", VOInfo.pixel_clk);
		rtd_pr_i3ddma_info("i_ratio=%d\n", VOInfo.i_ratio);
		rtd_pr_i3ddma_info("source=%d\n", VOInfo.source);
		rtd_pr_i3ddma_info("isJPEG=%d\n", VOInfo.isJPEG);
		rtd_pr_i3ddma_info("mode_3d=%d\n", VOInfo.mode_3d);
		rtd_pr_i3ddma_info("force2d=%d\n", VOInfo.force2d);
		rtd_pr_i3ddma_info("afd=%d\n", VOInfo.afd);
		rtd_pr_i3ddma_info("HPAR=%d\n", VOInfo.pixelAR_hor);
		rtd_pr_i3ddma_info("VPAR=%d\n", VOInfo.pixelAR_ver);
		rtd_pr_i3ddma_info("src_h_wid=%d\n", VOInfo.src_h_wid);
		rtd_pr_i3ddma_info("src_v_len=%d\n", VOInfo.src_v_len);
		rtd_pr_i3ddma_info("rotate=%d\n", VOInfo.screenRotation);
		rtd_pr_i3ddma_info("launcher=%d\n", VOInfo.launcher);

		//set_zoom_smoothtoggle_vo_hwid(VOInfo.src_h_wid);
		//set_zoom_smoothtoggle_vo_vlen(VOInfo.src_v_len);
		//if ((get_vo_nosignal_flag(ch) == 0) && (memcmp(&vo->videoPlane[ch].VOInfo, &VOInfo, sizeof(SLR_VOINFO) - (sizeof(SLR_AFD_TYPE) + (sizeof(unsigned int)*4))) == 0)) { /* don't compare voinfo AFD, PixelAR and launcher*/
		//	rtd_pr_i3ddma_debug("[vo] enter the same timing\n");
		//	return S_FAIL;
		//}

		//memcpy(&vo->videoPlane[ch].VOInfo, &VOInfo, sizeof(SLR_VOINFO));
		memcpy(Scaler_VOInfoPointer(ch), &VOInfo, sizeof(SLR_VOINFO));
		//Scaler_DispSetInputInfoByDisp(ch, SLR_INPUT_VO_SOURCE_TYPE, VOInfo.isJPEG);
		//set_vo_nosignal_flag(ch, 0);
		Config_VO_Dispinfo(&VOInfo);
		return;

}

void dvrif_vodma_pqdecomp_setting(VIDEO_RPC_VOUT_CONFIG_HDMI_3D *HDMI_INFO)
{
    vodma_vodma_pq_decmp_RBUS vodma_vodma_pq_decmp_reg;
    vodma_vodma_pq_decmp_pair_RBUS vodma_vodma_pq_decmp_pair_reg;
    unsigned int TotalSize = 0;
    unsigned int comp_wid = HDMI_INFO->width;
    unsigned int comp_ratio = HDMI_INFO->comps_bits;

    if (HDMI_INFO->enable_comps == TRUE)
    {
            /*
            width need to align 32

            line sum bit = (width * compression bit + 256) / 128

            for new PQC and PQDC @Crixus 20170615
        */
        if((comp_wid % 32) != 0){
            comp_wid = comp_wid + (32 - (comp_wid % 32));
        }

        TotalSize = (comp_wid * comp_ratio + 256) * 10 / 128;
        if((TotalSize % 10) != 0){
            TotalSize = TotalSize / 10 + 1;
        }else{
            TotalSize = TotalSize / 10;
        }

        //vodma de-compression setting
        vodma_vodma_pq_decmp_reg.regValue = rtd_inl(VODMA_VODMA_PQ_DECMP_reg);
        vodma_vodma_pq_decmp_reg.decmp_height = HDMI_INFO->height;
        vodma_vodma_pq_decmp_reg.decmp_width_div32 = ((HDMI_INFO->width + 0x1f) & ~0x1f) >> 5;
        rtd_outl(VODMA_VODMA_PQ_DECMP_reg, vodma_vodma_pq_decmp_reg.regValue);

        //de-compression bits setting
        vodma_vodma_pq_decmp_pair_reg.regValue = rtd_inl(VODMA_VODMA_PQ_DECMP_PAIR_reg);

        if(HDMI_INFO->comps_line_mode)
        {
                vodma_vodma_pq_decmp_pair_reg.decmp_line_mode = HDMI_INFO->comps_line_mode;
                vodma_vodma_pq_decmp_pair_reg.decmp_line_sum_bit = TotalSize;//HDMI_INFO->comps_bits; // Constrain: decmp_line_sum_bit = cmp_line_sum_bit
                //vodma_vodma_pq_decmp_pair_reg.decmp_two_line_prediction_en = 0;
        }
        else
        {
                vodma_vodma_pq_decmp_pair_reg.decmp_line_mode = 0;
                vodma_vodma_pq_decmp_pair_reg.decmp_line_sum_bit = 0;
                //vodma_vodma_pq_decmp_pair_reg.decmp_two_line_prediction_en = 1;
        }
        if(Get_PANEL_VFLIP_ENABLE()){
            vodma_vodma_pq_decmp_pair_reg.decmp_two_line_prediction_en = 0;
        }else if(is_rotate_180_bypass_vi(SLR_MAIN_DISPLAY))
        {
            vodma_vodma_pq_decmp_pair_reg.decmp_two_line_prediction_en = 0;
        }
        else {
            if ((HDMI_INFO->comps_line_mode) && (get_current_driver_pattern(DRIVER_FREERUN_PATTERN) == 1)) {
                vodma_vodma_pq_decmp_pair_reg.decmp_two_line_prediction_en = 0;
            } else {
                vodma_vodma_pq_decmp_pair_reg.decmp_two_line_prediction_en = 1;
            }
        }

        // sync kernel always use 10 bit
        vodma_vodma_pq_decmp_pair_reg.decmp_data_bit_width = 1;

        //decompression YUV444
        if (HDMI_INFO->chroma_fmt == VO_CHROMA_YUV422) {
            vodma_vodma_pq_decmp_pair_reg.decmp_data_format = 1;//422
            vodma_vodma_pq_decmp_pair_reg.decmp_data_color = 1;
			vodma_vodma_pq_decmp_pair_reg.decmp_pair_para = 1;
		} else {
            vodma_vodma_pq_decmp_pair_reg.decmp_data_format = 0;//444
            vodma_vodma_pq_decmp_pair_reg.decmp_data_color = 1; //currently no case rgb channel
            vodma_vodma_pq_decmp_pair_reg.decmp_pair_para = 1;
		}
        rtd_outl(VODMA_VODMA_PQ_DECMP_PAIR_reg, vodma_vodma_pq_decmp_pair_reg.regValue);

        //enable de-compression
        vodma_vodma_pq_decmp_reg.regValue = rtd_inl(VODMA_VODMA_PQ_DECMP_reg);
        vodma_vodma_pq_decmp_reg.decmp_en = 1;
        rtd_outl(VODMA_VODMA_PQ_DECMP_reg, vodma_vodma_pq_decmp_reg.regValue);
    }
    else

    {
        //disable de-compression
        vodma_vodma_pq_decmp_reg.regValue = rtd_inl(VODMA_VODMA_PQ_DECMP_reg);
        vodma_vodma_pq_decmp_reg.decmp_en = 0;
        rtd_outl(VODMA_VODMA_PQ_DECMP_reg, vodma_vodma_pq_decmp_reg.regValue);
    }
}


void VODMA_Compression_Mask_Setting(bool bEnable,unsigned int width,unsigned int length)
{
	vodma_vodma_v1vgip_mask_ctrl_RBUS vodma_vodma_v1vgip_mask_ctrl_reg;
	vodma_vodma_v1vgip_hmask_RBUS vodma_vodma_v1vgip_hmask_reg;
	vodma_vodma_v1vgip_vmask_RBUS vodma_vodma_v1vgip_vmask_reg;
	vodma_vodma_reg_db_ctrl_RBUS vodma_reg_db_ctrl;

    vodma_vodma_v1vgip_mask_ctrl_reg.regValue = rtd_inl(VODMA_VODMA_V1VGIP_MASK_CTRL_reg);
    vodma_vodma_v1vgip_mask_ctrl_reg.mask_den_en = bEnable; // disable in default
    //vodma_vodma_v1vgip_mask_ctrl_reg.mask_data_en = 2; // 0: disable, 1: window mode, 2: mask mode (default)

    //vodma->V1_vgipPos_bg_clr.bg_y = 0;
    //vodma->V1_vgipPos_bg_clr.bg_u = 0x200;
    //vodma->V1_vgipPos_bg_clr.bg_v = 0x200;
    //vo_rtd_outl(VODMA_VODMA_V1VGIP_BG_CLR_reg, vodma->V1_vgipPos_bg_clr.regValue);
	if(bEnable){
	    vodma_vodma_v1vgip_hmask_reg.regValue = rtd_inl(VODMA_VODMA_V1VGIP_HMASK_reg);
	    vodma_vodma_v1vgip_hmask_reg.h_st = 1; // count by den from 1
	    vodma_vodma_v1vgip_hmask_reg.h_end = width;
	    rtd_outl(VODMA_VODMA_V1VGIP_HMASK_reg, vodma_vodma_v1vgip_hmask_reg.regValue);

	    vodma_vodma_v1vgip_vmask_reg.regValue = rtd_inl(VODMA_VODMA_V1VGIP_VMASK_reg);
	    vodma_vodma_v1vgip_vmask_reg.v_st = 1; // count by den from 1
	    vodma_vodma_v1vgip_vmask_reg.v_end = length;
	    rtd_outl(VODMA_VODMA_V1VGIP_VMASK_reg, vodma_vodma_v1vgip_vmask_reg.regValue);
	}

	rtd_outl(VODMA_VODMA_V1VGIP_MASK_CTRL_reg, vodma_vodma_v1vgip_mask_ctrl_reg.regValue);

    vodma_reg_db_ctrl.regValue = rtd_inl(VODMA_VODMA_REG_DB_CTRL_reg);
    vodma_reg_db_ctrl.vodma_db_rdy = 1;
    rtd_outl(VODMA_VODMA_REG_DB_CTRL_reg, vodma_reg_db_ctrl.regValue);

}

void Scaler_HdrSetVTopSize(unsigned short pic_width, unsigned short pic_height)
{
    hdr_all_top_top_pic_size_RBUS top_pic_size_reg;
    hdr_all_top_top_pic_total_RBUS top_pic_total_reg;
    hdr_all_top_top_pic_sta_RBUS top_pic_sta_reg;
    hdr_all_top_top_d_buf_RBUS top_d_buf_reg;
    dm_input_format_RBUS input_format_reg;
    hdr_all_top_top_ctl_RBUS top_ctl_reg;

    top_ctl_reg.regValue = rtd_inl(HDR_ALL_TOP_TOP_CTL_reg);

    top_pic_size_reg.regValue = rtd_inl(HDR_ALL_TOP_TOP_PIC_SIZE_reg);
    top_pic_total_reg.regValue = rtd_inl(HDR_ALL_TOP_TOP_PIC_TOTAL_reg);
    top_pic_sta_reg.regValue = rtd_inl(HDR_ALL_TOP_TOP_PIC_STA_reg);

    top_pic_size_reg.dolby_hsize = pic_width;
    top_pic_size_reg.dolby_vsize = pic_height;

    top_pic_total_reg.dolby_h_total = VODMA_VODMA_V1SGEN_get_h_thr(rtd_inl(VODMA_VODMA_V1SGEN_reg)) + 1;   //0x1131;

    top_pic_sta_reg.dolby_h_den_sta = VODMA_VODMA_V1VGIP_HACT_get_h_st(rtd_inl(VODMA_VODMA_V1VGIP_HACT_reg));// + h_offset; //0x008f;

    top_pic_sta_reg.dolby_v_den_sta = VODMA_VODMA_V1VGIP_VACT1_get_v_st(rtd_inl(VODMA_VODMA_V1VGIP_VACT1_reg)) + 3 - VODMA_VODMA_V1VGIP_IVS1_get_v_end(rtd_inl(VODMA_VODMA_V1VGIP_IVS1_reg));

    rtd_pr_i3ddma_info("[%s %d]h0=%d,%dx%d,%d\n",
        __FUNCTION__, __LINE__, top_pic_sta_reg.dolby_h_den_sta, top_pic_size_reg.dolby_hsize, top_pic_size_reg.dolby_vsize, top_pic_total_reg.dolby_h_total);

    input_format_reg.regValue= rtd_inl(DM_Input_Format_reg);

    rtd_outl(DM_Input_Format_reg, input_format_reg.regValue);
    rtd_outl(HDR_ALL_TOP_TOP_PIC_SIZE_reg, top_pic_size_reg.regValue);
    rtd_outl(HDR_ALL_TOP_TOP_PIC_TOTAL_reg, top_pic_total_reg.regValue);
    rtd_outl(HDR_ALL_TOP_TOP_PIC_STA_reg, top_pic_sta_reg.regValue);

    //HDR10 apply
    top_d_buf_reg.regValue = rtd_inl(HDR_ALL_TOP_TOP_D_BUF_reg);
    top_d_buf_reg.dolby_double_apply = 1;
    rtd_outl(HDR_ALL_TOP_TOP_D_BUF_reg, top_d_buf_reg.regValue);
}

void VODMA_SetFlagFifo(unsigned int progressive, unsigned int frame_rate, unsigned int src_frame_rate)
{
    vodma_vodma_i2rnd_RBUS    vo_i2rnd;
    vodma_vodma_i2rnd_m_flag_RBUS vo_i2rnd_m_flag;
//    vodma_vodma_i2rnd_s_flag_RBUS vo_i2rnd_s_flag;
    vodma_vodma_i2rnd_dma_info_RBUS vodma_i2rnd_dma_info;
    vodma_vodma_i2rnd_fifo_th_RBUS vodma_vodma_i2rnd_fifo_th_reg;
    vodma_vodma_reg_db_ctrl2_RBUS vodma_vodma_reg_db_ctrl2_reg;

    unsigned int interlace = 0;

    vo_i2rnd_m_flag.regValue = rtd_inl(VODMA_vodma_i2rnd_m_flag_reg);
    //vo_i2rnd_m_flag.i2rnd_hw_mask_repeat_frame_m_en = 1;
    vo_i2rnd_m_flag.i2rnd_m_block_free_run = 0;
    vo_i2rnd_m_flag.i2rnd_m_block_sel = 1;
    vo_i2rnd_m_flag.i2rnd_m_field_fw = 1;
    vo_i2rnd_m_flag.i2rnd_m_field_free_run = 0;
    vo_i2rnd_m_flag.i2rnd_m_field_sel = ((Get_PANEL_VFLIP_ENABLE())? 2: 1);
    vo_i2rnd_m_flag.i2rnd_m_lflag_fw = 0;
    vo_i2rnd_m_flag.i2rnd_m_lflag_free_run = 0;
    vo_i2rnd_m_flag.i2rnd_m_lflag_sel = 1;
    rtd_outl(VODMA_vodma_i2rnd_m_flag_reg, vo_i2rnd_m_flag.regValue);

    //clear write point
    vodma_i2rnd_dma_info.regValue = rtd_inl(VODMA_vodma_i2rnd_dma_info_reg);
    vodma_i2rnd_dma_info.dma_info_rptr_fw_dispm = 0;
    vodma_i2rnd_dma_info.dma_info_rptr_fw_set_dispm = 1;
    vodma_i2rnd_dma_info.dma_info_wptr_fw_dispm = 0;
    vodma_i2rnd_dma_info.dma_info_wptr_fw_set_dispm= 1;
    vodma_i2rnd_dma_info.dma_info_rptr_fw_i3ddma = 0;
    vodma_i2rnd_dma_info.dma_info_rptr_fw_set_i3ddma= 1;
    vodma_i2rnd_dma_info.dma_info_wptr_fw_i3ddma = 0;
    vodma_i2rnd_dma_info.dma_info_wptr_fw_set_i3ddma= 1;
    rtd_outl(VODMA_vodma_i2rnd_dma_info_reg, vodma_i2rnd_dma_info.regValue);

    //enable fifo mode
    vo_i2rnd.regValue = rtd_inl(VODMA_vodma_i2rnd_reg);
    vo_i2rnd.i2rnd_flag_fifo_en = 1;
    vo_i2rnd.i2rnd_src_flag_inv = 0;
    rtd_outl(VODMA_vodma_i2rnd_reg, vo_i2rnd.regValue);

    interlace = (progressive ? 0 : 1);
    //repeat mask enable in interlace source, but DTV does not enable @Crixus 20160803
    vo_i2rnd_m_flag.regValue = rtd_inl(VODMA_vodma_i2rnd_m_flag_reg);
    if(interlace)
        vo_i2rnd_m_flag.i2rnd_hw_mask_repeat_frame_m_en = 1;
    else
        vo_i2rnd_m_flag.i2rnd_hw_mask_repeat_frame_m_en = 0;
    rtd_outl(VODMA_vodma_i2rnd_m_flag_reg, vo_i2rnd_m_flag.regValue);

    //merlin4 update input faster mode
    vodma_vodma_i2rnd_fifo_th_reg.regValue = rtd_inl(VODMA_vodma_i2rnd_fifo_th_reg);
    vodma_vodma_i2rnd_fifo_th_reg.flag_fifo_style_i3ddma = ((src_frame_rate > frame_rate)? 1: 0);
    vodma_vodma_i2rnd_fifo_th_reg.flag_fifo_fast_int_i3ddma = (((src_frame_rate > frame_rate) && interlace)? 1: 0);//merlin4 can support interlace faster mode
    vodma_vodma_i2rnd_fifo_th_reg.flag_fifo_style_dispm = 0;
    vodma_vodma_i2rnd_fifo_th_reg.flag_fifo_fast_int_dispm = 0;
    rtd_outl(VODMA_vodma_i2rnd_fifo_th_reg, vodma_vodma_i2rnd_fifo_th_reg.regValue);

    //disable i2r db_en make it apply directly
    vodma_vodma_reg_db_ctrl2_reg.regValue = rtd_inl(VODMA_VODMA_REG_DB_CTRL2_reg);
    vodma_vodma_reg_db_ctrl2_reg.i2rnd_db_en = 0;
    //vodma_vodma_reg_db_ctrl2_reg.i2rnd_db_rdy = 1;
    rtd_outl(VODMA_VODMA_REG_DB_CTRL2_reg, vodma_vodma_reg_db_ctrl2_reg.regValue);
}

void VODMA_Iv2PvDelay(unsigned char enable, VIDEO_RPC_VOUT_CONFIG_HDMI_3D *HDMI_INFO, VO_TIMING *timemode){

    INT32 id2pv_delay_1 = 0, id2pv_delay_2 = 0;
    UINT32 voHtotal, voVactive, voVstart, VoClock;
    UINT32 i3ddmaVtotal, i3ddmaVstart, i3ddmaVactive, i3ddmaVfreq;
    unsigned long long temp1;
    vodma_vodma_read_start_RBUS vodma_vodma_read_start_reg;
    vodma_vodma_pvs0_gen_RBUS vodma_vodma_pvs0_gen_reg;

    if (enable == _ENABLE)
    {
        voHtotal = timemode->HTotal;
        voVactive = timemode->VHeight;
        voVstart = timemode->VStartPos;

        VoClock = vsyncgen_pixel_freq;

        i3ddmaVtotal = HDMI_INFO->vtotal;
        i3ddmaVstart = HDMI_INFO->v_start;
        i3ddmaVactive = HDMI_INFO->height;
        i3ddmaVfreq = HDMI_INFO->framerate /100;


        rtd_pr_i3ddma_info("[iv2dv_dly][%d,%d,%d,%d,%d,%d,%d,%d]\n",voHtotal, voVactive, voVstart, VoClock,i3ddmaVtotal, i3ddmaVstart, i3ddmaVactive, i3ddmaVfreq);

        vodma_vodma_read_start_reg.regValue = rtd_inl(VODMA_VODMA_READ_START_reg);
        vodma_vodma_read_start_reg.line_cnt = VODMA_VSTART_PREREAD_LINE;
        rtd_outl(VODMA_VODMA_READ_START_reg, vodma_vodma_read_start_reg.regValue);

        //rtd_pr_i3ddma_emerg("[crixus]VoClock = %d, voHtotal = %d, voVstart = %d, voVactive = %d\n", VoClock, voHtotal, voVstart, voVactive);
        //rtd_pr_i3ddma_emerg("[crixus]i3ddmaVtotal = %d, i3ddmaVstart = %d, i3ddmaVactive = %d, i3ddmaVfreq = %d\n", i3ddmaVtotal, i3ddmaVstart, i3ddmaVactive, i3ddmaVfreq);
        /*
            T1: i3ddma act start
            T2: vodma act start
            N: DDR writing time
            Note: ECN for vodma_pre_read 0xb8005144[11:0]
            (before ecn : vodma pre_read = 0)

            formula:
                     T1 <= T2
                     => 1/i3ddma_clock * i3ddma_Htotal * (i3ddma_Vstart + N) <= 1/vo_clock * vo_Htotal * (iv2pv_delay+vodma pre_read)

            T1 = 1/(i3ddma_Htotal*i3ddma_Vtotal*i3ddma_freq) * i3ddma_Htotal * (i3ddma_Vstart + N)
                   = 1/(i3ddma_Vtotal*i3ddma_freq) * (i3ddma_Vstart + N)
                   = (i3ddma_Vstart + N) / (i3ddma_Vtotal*i3ddma_freq)

            T1 <= T2
            => T1 <= 1/vo_clock * vo_Htotal * iv2pv_delay
            => T1 * (vo_clock / vo_Htotal) <= iv2pv_delay
            => iv2pv_delay >= T1 * (vo_clock / vo_Htotal);
        */
        temp1 = (unsigned long long)(i3ddmaVstart + I3DDMA_DRAM_MARGIN) * 10000000;
        do_div(temp1 , (i3ddmaVtotal * i3ddmaVfreq));
        //rtd_pr_i3ddma_emerg("[crixus]1.temp1 = %lld\n", temp1);

        temp1 = temp1 * (unsigned long long)(VoClock / voHtotal);
        do_div(temp1, 10000);/*Modify to use VO real clock to calculate @Crixus 20161230*/
        if((int)temp1 % 100 > 0){
            do_div(temp1,100);
            temp1 = temp1 + 1;
        }
        else{
            do_div(temp1,100);
        }
        //rtd_pr_i3ddma_emerg("[crixus]2.temp1 = %lld\n", temp1);

        id2pv_delay_1 = (int)temp1 + VODMA_IV2PV_DELAY_CAL_MARGIN - VODMA_VSTART_PREREAD_LINE;
        //rtd_pr_i3ddma_emerg("[crixus]id2pv_delay_1 line = %d\n", id2pv_delay_1);

        /*
            T3: i3ddma act end
            T4: vodma act end

            formula:
                     T3 <= T4
                     => 1/i3ddma_clock * i3ddma_Htotal * (i3ddma_Vactive + i3ddma_Vstart + N) <= 1/vo_clock * vo_Htotal * (vo_Vsatrt + vo_Vactive + iv2pv_delay)

            T3 = 1/(i3ddma_Htotal*i3ddma_Vtotal*i3ddma_freq) * i3ddma_Htotal * (i3ddma_Vactive + i3ddma_Vstart + N)
                   = 1/(i3ddma_Vtotal*i3ddma_freq) * (i3ddma_Vactive + i3ddma_Vstart + N)
                   = (i3ddma_Vactive + i3ddma_Vstart + N) / (i3ddma_Vtotal*i3ddma_freq)
            iv2pv_delay >= temp1 * (vo_clock / vo_Htotal) - (vo_Vsatrt + vo_Vactive);
        */

        temp1 = (unsigned long long)(i3ddmaVactive + i3ddmaVstart + I3DDMA_DRAM_MARGIN) * 10000000;
        do_div(temp1, (i3ddmaVtotal * i3ddmaVfreq));
        //rtd_pr_i3ddma_emerg("[crixus]1.temp1 = %lld\n", temp1);

        temp1 = temp1 * (unsigned long long)(VoClock / voHtotal);
        do_div(temp1,10000);
        if((int)temp1 % 100 > 0){
            do_div(temp1,100);
            temp1 = temp1 + 1;
        }
        else{
            do_div(temp1,100);
        }
        temp1 = temp1 - (unsigned long long)(voVactive + voVstart);/*Modify to use VO real clock to calculate @Crixus 20161230*/
        //rtd_pr_i3ddma_emerg("[crixus]2.temp1 = %lld\n", temp1);

        id2pv_delay_2 = (int)temp1 + VODMA_IV2PV_DELAY_CAL_MARGIN;
        //rtd_pr_i3ddma_emerg("[crixus]id2pv_delay_2 line = %d\n", id2pv_delay_2);

        if(id2pv_delay_1 > id2pv_delay_2)
            id2pv_delay_1 = id2pv_delay_1 * voHtotal;
        else
            id2pv_delay_1 = id2pv_delay_2 * voHtotal;

        rtd_pr_i3ddma_info("[%s %d]id2pv_delay = 0x%x\n", __FUNCTION__, __LINE__, id2pv_delay_1);
    }
    else{
        id2pv_delay_1 = 2;//default setting
        rtd_pr_i3ddma_info("[%s %d]id2pv_delay = 0x%x\n", __FUNCTION__, __LINE__, id2pv_delay_1);
    }

    vodma_vodma_pvs0_gen_reg.regValue = rtd_inl(VODMA_VODMA_PVS0_Gen_reg);
    vodma_vodma_pvs0_gen_reg.iv2pv_dly = id2pv_delay_1;

    rtd_outl(VODMA_VODMA_PVS0_Gen_reg, vodma_vodma_pvs0_gen_reg.regValue);
}

int VODMA_ConfigHDMI(VIDEO_RPC_VOUT_CONFIG_HDMI_3D *HDMI_INFO)
{
    VO_TIMING timemode;
    VODMA_DDOMAIN_CHROMA_FMT Ddomain_Chroma_fmt;
//    vodma_vodma_reg_db_ctrl2_RBUS       V1_vodma_reg_db_ctrl2;
    unsigned int porch_extend = 0;

    porch_extend =0;

    if ( HDMI_INFO->videoPlane > 2)
    {
        rtd_pr_i3ddma_info("[vo] Invalid plane %d @ %s\n", HDMI_INFO->videoPlane, __FUNCTION__);
        return -1 ;
    }

    if ( HDMI_INFO->width < 86)
    {
        rtd_pr_i3ddma_info("[vo] HDMI width=%d!!\n", HDMI_INFO->width);
        return -1 ;
    }

    //plane->secure = 0;

    if (HDMI_INFO->videoPlane == VO_VIDEO_PLANE_V3)
        Ddomain_Chroma_fmt =FMT_444;
    else
        Ddomain_Chroma_fmt = (HDMI_INFO->chroma_fmt == VO_CHROMA_YUV422 || HDMI_INFO->chroma_fmt == VO_CHROMA_YUV411)? FMT_422: FMT_444; // no 420

    if(((HDMI_INFO->width == 1360) || (HDMI_INFO->width == 1366))
        && ((HDMI_INFO->chroma_fmt == VO_CHROMA_YUV444) || (HDMI_INFO->chroma_fmt == VO_CHROMA_RGB888))
        &&(HDMI_INFO->enable_comps == FALSE))
    {
        HDMI_INFO->width = ((HDMI_INFO->width + 0x1f) & ~0x1f); // 1360 -> 1376
    }

    timemode.isProg     = HDMI_INFO->progressive;// 1 ;
    timemode.HWidth     = (HDMI_INFO->quincunx_en|| HDMI_INFO->sensio3D_en)? HDMI_INFO->width*2: HDMI_INFO->width;
    timemode.VHeight    = HDMI_INFO->height;
    timemode.HStartPos  = (HDMI_INFO->videoPlane == VO_VIDEO_PLANE_V3)? 140: 72;//140;
    timemode.VStartPos  = 20;
    timemode.hdrSource = HDMI_INFO->hdr_source;

    if(HDMI_INFO->htotal > (timemode.HWidth + timemode.HStartPos)){
        timemode.HTotal = HDMI_INFO->htotal;
        if(timemode.HTotal < (timemode.HStartPos * 2 + timemode.HWidth)){
            timemode.HStartPos = (timemode.HTotal - timemode.HWidth) / 2;
        }
    }
    else{
        timemode.HTotal = timemode.HWidth + (timemode.HStartPos * 2);
    }

    timemode.FrameRate  = (HDMI_INFO->framerate == 0)? 60000: HDMI_INFO->framerate ;

    if(HDMI_INFO->vtotal > (timemode.VHeight + timemode.VStartPos)){
        timemode.VTotal = (timemode.isProg? HDMI_INFO->vtotal: HDMI_INFO->vtotal * 2) + ((timemode.isProg) ?0:(timemode.FrameRate <= 100000? 18: 8)) + porch_extend;//add enough porch for vo isr and dolby case.

        if(timemode.isProg){
            if(timemode.VTotal < (timemode.VStartPos * 2 + timemode.VHeight)){
                timemode.VStartPos = (timemode.VTotal - timemode.VHeight) / 2;
            }
        }
        else{
            if(timemode.VTotal < (timemode.VStartPos * 2 + timemode.VHeight * 2)){
                timemode.VStartPos = (timemode.VTotal - timemode.VHeight * 2) / 2;
            }
        }
    }
    else{
        timemode.VTotal = (timemode.isProg? HDMI_INFO->height: HDMI_INFO->height * 2) + (timemode.VStartPos * 2) + ((timemode.isProg) ?0:(timemode.FrameRate <= 100000? 18: 8)) + porch_extend;
    }

    timemode.HStartPos = (timemode.HStartPos + 0x3) & ~0x3; // Hstart must be 4N for 4 pixel

    timemode.SampleRate = (timemode.HTotal * timemode.VTotal * (timemode.FrameRate / 1000) + 9999) / 10000;

    rtd_pr_i3ddma_info("[vo] p%d timemode:Prog %d,[%d/%d/%d/%d/%d/%d],FR/SR[%d(%d)/%d]\n",
        HDMI_INFO->videoPlane,  timemode.isProg,
        timemode.HTotal, timemode.VTotal, timemode.HStartPos, timemode.VStartPos, timemode.HWidth, timemode.VHeight,
        timemode.FrameRate, HDMI_INFO->framerate, timemode.SampleRate);

    drivf_scaler_reset_freerun();
    dvrif_vodma_pqdecomp_setting(HDMI_INFO);
    switch (HDMI_INFO->videoPlane)
    {
        case VO_VIDEO_PLANE_V1:
        {
            VODMA_EnableDoubleBuf(1);

            /* i-domain timing gen */
            timinggen_set_v1(0, &timemode);

            /* syncgen */
            vsyncgen_set_v1(0, &timemode);

             /* dma */
            dma_set_v1(0, &timemode, HDMI_INFO, DMA_SEQUENTIAL_MODE, (VODMA_DDOMAIN_CHROMA) Ddomain_Chroma_fmt);

            Scaler_HdrSetVTopSize(HDMI_INFO->width, HDMI_INFO->height);

            VODMA_SetFlagFifo(timemode.isProg, (timemode.FrameRate / 100), (HDMI_INFO->srcframerate/100));
            VODMA_SetVOInfo(VO_VIDEO_PLANE_V1, &timemode, 5,(VODMA_DDOMAIN_CHROMA) Ddomain_Chroma_fmt, VO_COLORSPACE_YUV);
            drv_I3ddmaVodma_GameMode_iv2pv_delay(TRUE);
            VODMA_DBApply(1);
        }
        break;

    default:
        break;
    }

    return 0 ;
}

#endif
#if 1//def CONFIG_FORCE_RUN_I3DDMA

extern void sync_pc_mode_info(void);
unsigned char drivf_i3ddma_config(unsigned char display, unsigned char src)
{
    I3DDMA_TIMING_T i3ddma_timing;
    unsigned char mode;

    if(src != VGIP_SRC_TMDS && src != VGIP_SRC_DPRX
#ifdef CONFIG_SUPPORT_SRC_ADC
        && src != VGIP_SRC_ADC
#endif//#ifdef CONFIG_SUPPORT_SRC_ADC
        && src != VGIP_SRC_ATV)
    {
        rtd_pr_i3ddma_emerg("drivf_i3ddma_config src error\n");
        return FALSE;
    }

    if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
    {
        vbe_disp_set_freesync_mode_flag(drvif_Hdmi_GetAMDFreeSyncEnable());//save current freesync mode
        Set_Val_VRR_timingMode_flag(drvif_Hdmi_GetVRREnable());//save current vrr mode
    }
#if IS_ENABLED(CONFIG_RTK_DPRX)
    else if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_DP)
    {
        vbe_disp_set_freesync_mode_flag(drvif_Dprx_GetDrrMode() == DP_DRR_MODE_AMD_FREE_SYNC);//save current freesync mode
        Set_Val_VRR_timingMode_flag(drvif_Dprx_GetDrrMode() == DP_DRR_MODE_ADAPTIVE_SYNC);//save current vrr mode
    }
#endif
    else
    {
        vbe_disp_set_freesync_mode_flag(0);//save current freesync mode
        Set_Val_VRR_timingMode_flag(0);//save current vrr mode
    }
#ifndef BUILD_QUICK_SHOW
#ifndef CONFIG_RTK_KDRV_DV_IDK_DUMP
    if(get_i3ddma_4k120_flag() == TRUE)
#else
    if (1)
#endif
    {
        rtd_pr_i3ddma_emerg("enable 4k120 flag\n");
        dvrif_i3ddma_compression_set_enable(false);
        //Re-order control for HDMI Dolby @Crixus 20180322
        set_i3ddma_dolby_reorder_mode(_DISABLE);
    }
    else
#endif
    {
        dvrif_i3ddma_compression_set_enable(true);
        if(Get_Val_vsc_run_pc_mode() && ((src == VGIP_SRC_TMDS) || (src == VGIP_SRC_ADC))){
            rtd_pr_i3ddma_notice("[%s][%d] i3ddma COMPRESSION_20_BITS. \n",__FUNCTION__,__LINE__);
            //K5LG-1492:modify for i3ddma compression to 20bit
            dvrif_i3ddma_set_compression_bits(COMPRESSION_20_BITS);//pc mode go 20bit only YPP and hdmi
        } else{
            rtd_pr_i3ddma_notice("[%s][%d] i3ddma COMPRESSION_12_BITS. \n",__FUNCTION__,__LINE__);
            dvrif_i3ddma_set_compression_bits(COMPRESSION_12_BITS);//default 12 bit
        }
        dvrif_i3ddma_set_compression_mode(I3DDMA_COMP_LINE_MODE);
        //Re-order control for HDMI Dolby @Crixus 20180322
        if((get_HDMI_HDR_mode() == HDR_DOLBY_HDMI)
#ifdef CONFIG_SUPPORT_DOLBY_VSIF
            && (get_HDMI_Dolby_VSIF_mode() != DOLBY_HDMI_VSIF_LL)
#endif
        )
        {//this is standard mode
            set_i3ddma_dolby_reorder_mode(_ENABLE);
            i3ddma_dolby_reorder_update_color_format();//update color format to 422 12bits
        }
        else{
            set_i3ddma_dolby_reorder_mode(_DISABLE);
        }
    }

    if(I3DDMA_DolbyVision_HDMI_Init() == 1)
    {
        rtd_pr_i3ddma_emerg("drivf_i3ddma_config alloc buf error\n");
        return FALSE;
    }

    //drvif_scaler_i3ddma_vodma_fifo_setting(_ENABLE);         // move to video fw
    //drvif_scaler_i3ddma_vodma_fifo_input_fast(SLR_MAIN_DISPLAY, Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ), Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE));    // move to video fw
    //rtd_pr_i3ddma_emerg("[crixus]Non-I2rnd path@@@\n");

    memset(&i3ddma_timing, 0 , sizeof(I3DDMA_TIMING_T));
    //rtd_pr_i3ddma_emerg("%d===%d\n",Scaler_DispGetInputInfoByDisp(display,SLR_DISP_INTERLACE), Scaler_DispGetStatus(display, SLR_DISP_INTERLACE));
    i3ddma_timing.color = (I3DDMA_COLOR_SPACE_T) Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_COLOR_SPACE);
    i3ddma_timing.depth = (I3DDMA_COLOR_DEPTH_T) Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_COLOR_DEPTH);

    i3ddma_timing.colorimetry = (I3DDMA_COLORIMETRY_T) Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_COLOR_IMETRY);

    i3ddma_timing.h_act_len = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPH_ACT_WID_PRE);
    i3ddma_timing.h_act_sta = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPH_ACT_STA_PRE);
    i3ddma_timing.v_act_len = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPV_ACT_LEN_PRE);
    i3ddma_timing.v_act_sta =  Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_IPV_ACT_STA_PRE);

    i3ddma_timing.h_freq = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_H_FREQ);
    i3ddma_timing.h_total = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_H_LEN);
    i3ddma_timing.i3ddma_3dformat = Get_Val_I3ddma3Dformat();
    i3ddma_timing.i3ddma_force2d = VO_FORCE2D_DISABLE;
    i3ddma_timing.polarity = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_POLARITY);
    i3ddma_timing.progressive = Scaler_DispGetStatus(display,SLR_DISP_INTERLACE)?0:1;
    i3ddma_timing.v_active_space1 = 0;
    i3ddma_timing.v_active_space2 = 0;
    i3ddma_timing.v_freq = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_FREQ);
    i3ddma_timing.v_total = Scaler_DispGetInputInfoByDisp(display, SLR_INPUT_V_LEN);
#ifndef BUILD_QUICK_SHOW
    if(get_i3ddma_4k120_flag() == TRUE)
    {
        i3ddma_timing.h_act_len = i3ddma_timing.h_act_len / 2;
    }
#endif
#if 1 // dump source info
    rtd_pr_i3ddma_notice("[IDMA] C/D/CM/= %d/%d/%d\n", i3ddma_timing.color, i3ddma_timing.depth, i3ddma_timing.colorimetry);
    rtd_pr_i3ddma_notice("[IDMA] H/HST/HFre/HT=%d/%d/%d/%d\n",     i3ddma_timing.h_act_len, i3ddma_timing.h_act_sta, i3ddma_timing.h_freq, i3ddma_timing.h_total);
    rtd_pr_i3ddma_notice("[IDMA] 3D/PO/Prog=%d/%d/%d\n", i3ddma_timing.i3ddma_3dformat , i3ddma_timing.polarity, i3ddma_timing.progressive);
    rtd_pr_i3ddma_notice("[IDMA] V/VST/VFRE/VT=%d/%d/%d/%d\n", i3ddma_timing.v_act_len, i3ddma_timing.v_act_sta , i3ddma_timing.v_freq, i3ddma_timing.v_total);
#endif

    drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_HW_I3DDMA_DMA, Get_Val_I3ddma3DMode());
    rtd_pr_i3ddma_notice("################# i3ddma_timing.v_freq : %d\n", i3ddma_timing.v_freq);

    drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_TIMING, (unsigned long) &i3ddma_timing);
    drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_SRC_IDX, Scaler_DispGetInputInfo(SLR_INPUT_INPUT_SRC));
    drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_SRC_TYPE, Scaler_InputSrcGetMainChType());
    drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_SRC_FROM, Scaler_InputSrcGetMainChFrom());
    drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_MODE_CURR, Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR));
    drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_H_COUNT, Scaler_DispGetInputInfo(SLR_INPUT_H_COUNT));

    drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_HW_HSD_ENABLE, _DISABLE);
    drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_INPUT_H_SIZE, i3ddma_timing.h_act_len);
    drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_OUTPUT_H_SIZE, i3ddma_timing.h_act_len);
    drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_HW_DITHER12X10_ENABLE, _DISABLE);

    // [WOSQRTK-10007][BW] Force enable HDMI 444(420) to 422!!
    if((i3ddma_timing.h_act_len >= 3800) && (i3ddma_timing.v_freq > 480)
        && (i3ddma_timing.color == I3DDMA_COLOR_YUV444) && (i3ddma_timing.depth > 0))
    {
        drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_HW_4XXTO4XX_ENABLE, _ENABLE);
        rtd_pr_i3ddma_notice("[IDMA] Force enable HDMI 444(420) to 422!!\n");
    }else{
        drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_HW_4XXTO4XX_ENABLE, _DISABLE);
    }

    drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_DEPTH, I3DDMA_COLOR_DEPTH_10B);

    if(dvrif_i3ddma_compression_get_enable()){
        drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV444);
        rtd_pr_i3ddma_notice("\r\n#### func:%s compress(yuv444) comp_bit(%d)#######\r\n", __FUNCTION__, dvrif_i3ddma_get_compression_bits());
    }
#ifndef BUILD_QUICK_SHOW
    else if(get_i3ddma_4k120_flag() == TRUE)
    {
        drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_DEPTH, I3DDMA_COLOR_DEPTH_8B);
        drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV422);
        rtd_pr_i3ddma_notice("\r\n#### func:%s 4k120 yuv420(yuv422)#######\r\n", __FUNCTION__);
    }
#endif
    else if((i3ddma_timing.i3ddma_3dformat == I3DDMA_VERTICAL_STRIPE)|| (i3ddma_timing.i3ddma_3dformat == I3DDMA_CHECKER_BOARD))
    {
        // In Vertical strip & Checker board 3D, I3DDMA only support YUV444 capture
        drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV444);
    } // [Dolby Vision] IDMA capture in RGB format
    else if((get_HDMI_HDR_mode() == HDR_DOLBY_HDMI) && (i3ddma_timing.color == I3DDMA_COLOR_RGB)){
        drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_RGB);
    }
    else if((get_HDMI_HDR_mode() == HDR_HDR10_HDMI) || (i3ddma_timing.h_act_len > 3840)/*&& (i3ddma_timing.color != I3DDMA_COLOR_RGB)*/){
        //drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV444);
        rtd_pr_i3ddma_notice("\r\n####func:%s hdmi10 or width=4096 go YUV422#######\r\n",__FUNCTION__);
        drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV422);
    }else if((src == VGIP_SRC_ATV) && (get_AVD_Input_Source() == _SRC_TV)){
        rtd_pr_i3ddma_notice("\r\n####func:%s ATV go YUV422#######\r\n",__FUNCTION__);
        drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV422);
    }else if(i3ddma_timing.color == I3DDMA_COLOR_RGB) {//sync i3ddma uzd color issue from S4AP @Crixus 20170620
        rtd_pr_i3ddma_notice("\r\n####func:%s RGB go YUV444#######\r\n",__FUNCTION__);
        drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV444);
    }else {
        // [WOSQRTK-10007][BW] Force enable HDMI 444(420) to 422!!
        if((i3ddma_timing.h_act_len >= 3800) && (i3ddma_timing.v_freq > 480)
            && (i3ddma_timing.color == I3DDMA_COLOR_YUV444) && (i3ddma_timing.depth > 0))
        {
            drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, I3DDMA_COLOR_YUV422);
            rtd_pr_i3ddma_notice("[IDMA] Force color downsample to 422, wid/freq=%d/%d\n", i3ddma_timing.h_act_len, i3ddma_timing.v_freq);
        }else {
            rtd_pr_i3ddma_notice("\r\n####func:%s go original format(%d)#######\r\n",__FUNCTION__, i3ddma_timing.color);
            drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR, i3ddma_timing.color);
        }
    }

    if(Get_PANEL_VFLIP_ENABLE()) {
        if(is_rotate_180_bypass_vi(SLR_MAIN_DISPLAY))
        {
            rtd_pr_i3ddma_emerg("i3ddma vflip progressive rotate 180, send vo vflip=false to vcpu\n");
            rtd_pr_i3ddma_emerg("vflip=%d\n", FALSE);
            scaler_send_rotate_vflip_rpc(FALSE);
        }else {
            rtd_pr_i3ddma_emerg("vflip=%d\n", TRUE);
            scaler_send_rotate_vflip_rpc(TRUE);
        }

    } else {
        if(is_rotate_180_bypass_vi(SLR_MAIN_DISPLAY))
        {
            rtd_pr_i3ddma_emerg("i3ddma vflip progressive rotate 180, send vo vflip=true to vcpu\n");
            rtd_pr_i3ddma_emerg("vflip=%d\n", TRUE);
            scaler_send_rotate_vflip_rpc(TRUE);
        }else {
            rtd_pr_i3ddma_emerg("vflip=%d\n", FALSE);
            scaler_send_rotate_vflip_rpc(FALSE);
        }

    }


#ifdef CONFIG_HDR_SDR_SEAMLESS
    if(Get_DisplayMode_Src(display) == VSC_INPUTSRC_HDMI)
    {
        backup_hdmi_src_info(&i3ddma_timing);
        set_target_i3ddma_color_format(drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR));
    }
#endif
    //drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_FRAMERATE_X2, true);
    if(src == VGIP_SRC_TMDS || src == VGIP_SRC_DPRX)
        mode = VGIP_MODE_DIGITAL;
    else
        mode = VGIP_MODE_ANALOG;
    fw_i3ddma_set_vgip(src, mode);
    drv_I3DDMA_ApplySetting(display);

    return TRUE;
}
#endif
#ifndef BUILD_QUICK_SHOW
void  drvif_I3DDMA_set_capture_Mhl(SIZE *outsize)
{
    unsigned int num;
    unsigned int len;
    unsigned int rem;
    unsigned int lineunits;

     //lr_separate_ctrl1_RBUS lr_separate_ctrl1;
     h3ddma_lr_separate_ctrl1_RBUS lr_separate_ctrl1;
     //lr_separate_ctrl2_RBUS lr_separate_ctrl2;
     h3ddma_lr_separate_ctrl2_RBUS lr_separate_ctrl2;
     //cap_ctl1_RBUS cap_ctl1;
    // h3ddma_cap_ctl1_RBUS cap_ctl1;
     //cap_ctl0_RBUS cap_ctl0;
     h3ddma_cap0_cap_ctl0_RBUS cap_ctl0;
     //tg_v1_end_RBUS tg_v1_end;
     h3ddma_tg_v1_end_RBUS tg_v1_end;
     //i3ddma_enable_RBUS i3ddma_enable;
     h3ddma_i3ddma_enable_RBUS i3ddma_enable;
//   i3ddma_i3ddma_byte_channel_swap_RBUS  i3ddma_byte_channel_swap;
     h3ddma_cap0_wr_dma_pxltobus_RBUS h3ddma_cap0_wr_dma_pxltobus_reg;

    I3DDMA_MHL_CapBufInit();

    // dispi hdmi_3ddma
     lr_separate_ctrl1.regValue = IoReg_Read32(H3DDMA_LR_Separate_CTRL1_reg);
     lr_separate_ctrl1.hdmi_3d_structure = 0x0a;
     lr_separate_ctrl1.progressive = 1;
    // lr_separate_ctrl1.hdmi_3d_sel = 0;
     IoReg_Write32(H3DDMA_LR_Separate_CTRL1_reg,lr_separate_ctrl1.regValue);

    lr_separate_ctrl2.regValue = IoReg_Read32(H3DDMA_LR_Separate_CTRL2_reg);
    lr_separate_ctrl2.hact = outsize->nWidth;
    lr_separate_ctrl2.vact = outsize->nLength;
       IoReg_Write32(H3DDMA_LR_Separate_CTRL2_reg,lr_separate_ctrl2.regValue);

    //Capture
    //check this item
    //rtd_pr_i3ddma_info("[scart out]I3DDMA: 422cap = %d, 8bit=%d\n",Scaler_DispGetStatus(SLR_SUB_DISPLAY,SLR_DISP_422CAP), !Scaler_DispGetStatus(SLR_SUB_DISPLAY,SLR_DISP_10BIT));
    if(Scaler_DispGetStatus(SLR_SUB_DISPLAY,SLR_DISP_422CAP)) //422
        lineunits = (outsize->nWidth * 8 * 2) /128;
    else   //444
        lineunits = (outsize->nWidth * 8 * 3) /128;
    len = 32;
    num = (unsigned int)(lineunits / len);
    rem = (unsigned int)(lineunits % len);
    if(rem == 0)
    {
        num -= 1;
        rem = 32;
    }
#if 0
    cap_ctl1.regValue = IoReg_Read32(H3DDMA_Cap_CTL1_reg);
    cap_ctl1.cap_write_num = num;
    cap_ctl1.cap_water_lv = len;
    IoReg_Write32(H3DDMA_Cap_CTL1_reg,cap_ctl1.regValue);
#endif
    cap_ctl0.regValue = IoReg_Read32(H3DDMA_CAP0_Cap_CTL0_reg);
    h3ddma_cap0_wr_dma_pxltobus_reg.regValue = IoReg_Read32(H3DDMA_CAP0_WR_DMA_pxltobus_reg);
//  cap_ctl0.cap_write_rem = rem;
//  cap_ctl0.cap_write_len = len;
    cap_ctl0.cap0_auto_block_sel_en = 1;
//  cap_ctl0.vs_fall_rst_en = 1;
    cap_ctl0.cap0_quad_buf_en = 0;

    cap_ctl0.cap0_triple_buf_en= 0;

    if(Scaler_DispGetStatus(SLR_SUB_DISPLAY,SLR_DISP_422CAP)) //422
    {
        h3ddma_cap0_wr_dma_pxltobus_reg.cap0_pixel_encoding = 1;
        h3ddma_cap0_wr_dma_pxltobus_reg.cap0_bit_sel= 0;
     }
     else   //444
    {
        h3ddma_cap0_wr_dma_pxltobus_reg.cap0_pixel_encoding = 0;
        h3ddma_cap0_wr_dma_pxltobus_reg.cap0_bit_sel = 0;
    }

    IoReg_Write32(H3DDMA_CAP0_Cap_CTL0_reg,cap_ctl0.regValue);
    IoReg_Write32(H3DDMA_CAP0_WR_DMA_pxltobus_reg,h3ddma_cap0_wr_dma_pxltobus_reg.regValue);
    // why enable tripple buffer?, reset FIFO address at VS fall, HW auto set memory address, pixel encode 422,
    // length of data to fill in DDR LEN= 32, remaining part REM= 26

    // 2:2 pull up disable, first VS falling edge pixel num = 2, first VS falling edge line num = 2
     tg_v1_end.regValue = IoReg_Read32(H3DDMA_TG_v1_end_reg);
     tg_v1_end.tg_vend1= 2;
     tg_v1_end.tg_hend1= 2;
     tg_v1_end.pullup_en = 0;
     IoReg_Write32(H3DDMA_TG_v1_end_reg,tg_v1_end.regValue);
    // because input is a 2D signal, the value here is important here, just don't be equal to 0?
      i3ddma_enable.regValue = IoReg_Read32(H3DDMA_I3DDMA_enable_reg);
    // i3ddma_enable.cap_en = 1;
     i3ddma_enable.lr_separate_en =1;
     i3ddma_enable.tg_en =1;
     i3ddma_enable.frc_en = 1;
    IoReg_Write32(H3DDMA_I3DDMA_enable_reg,i3ddma_enable.regValue);// FRC, timing gen, 3D LR separate, Capture enable

    // 8-byte swap
    //IoReg_Mask32(H3DDMA_I3DDMA_byte_channel_swap_reg, ~H3DDMA_I3DDMA_byte_channel_swap_dma_8byte_swap_mask, H3DDMA_I3DDMA_byte_channel_swap_dma_8byte_swap(0x1));

    //scart out use 3-buffer
    IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg, idma_cap_buf[0].phyaddr);  // block0, DDR buffer L1 start address
    IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_1_reg,  idma_cap_buf[1].phyaddr);  // block1, DDR buffer L2 start address
    //IoReg_Write32(H3DDMA_Cap_L3_Start_reg,  idma_cap_buf[2].phyaddr);  // block2, DDR buffer L3 start address
    //IoReg_Write32(H3DDMA_Cap_L4_Start_reg,  idma_cap_buf[3].phyaddr);  // block3, DDR buffer L4 start address

}

unsigned char I3DDMA_MHL_CapBufInit(void) {

    unsigned int size = 1920*1080*4;

    if(idma_cap_buf[0].phyaddr == 0)
    {
            /*Use pli memory allocation @Crixus 20150801*/
            unsigned long malloccacheaddr  = (unsigned long)dvr_malloc_uncached_specific(size*2, GFP_DCU2_FIRST, NULL);

            #ifdef CONFIG_ARM64
            if (malloccacheaddr == (unsigned long)NULL) {
                rtd_pr_i3ddma_debug("malloc idma buffer fail .......................................\n");
                return 1;
            }
            #else
            if (malloccacheaddr == (unsigned int)NULL) {
                rtd_pr_i3ddma_debug("malloc idma buffer fail .......................................\n");
                return 1;
            }
            #endif
            idma_cap_buf[0].size = size;
            idma_cap_buf[0].cache = (void*)(malloccacheaddr);
            idma_cap_buf[0].phyaddr = (unsigned long)dvr_to_phys((void*)idma_cap_buf[0].cache);

            rtd_pr_i3ddma_debug("[idma-cap-buf] buf[0]=%lx(%d KB), phy(%lx) \n", (unsigned long)idma_cap_buf[0].cache, idma_cap_buf[0].size >> 10, (unsigned long)idma_cap_buf[0].phyaddr);

            idma_cap_buf[1].size = size;
            idma_cap_buf[1].cache = (void*)(malloccacheaddr + size);
            idma_cap_buf[1].phyaddr = (unsigned long)dvr_to_phys((void*)idma_cap_buf[1].cache);
            rtd_pr_i3ddma_debug("[idma-cap-buf] buf[1]=%lx(%d KB), phy(%lx) \n", (unsigned long)idma_cap_buf[1].cache, idma_cap_buf[1].size >> 10, (unsigned long)idma_cap_buf[1].phyaddr);

            rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_low_reg, idma_cap_buf[0].phyaddr );
            rtd_outl(H3DDMA_CAP0_CTI_DMA_WR_Rule_check_up_reg, idma_cap_buf[1].phyaddr + idma_cap_buf[1].size);
    }
    return 0;
}

void  drvif_I3DDMA_set_capture_for2D(SIZE *outsize)
{
    //unsigned int num;
    //unsigned int len;
    //unsigned int rem;
    //unsigned int lineunits;

     //lr_separate_ctrl1_RBUS lr_separate_ctrl1;
     h3ddma_lr_separate_ctrl1_RBUS lr_separate_ctrl1;
     //lr_separate_ctrl2_RBUS lr_separate_ctrl2;
     h3ddma_lr_separate_ctrl2_RBUS lr_separate_ctrl2;
     //cap_ctl1_RBUS cap_ctl1;
    // h3ddma_cap_ctl1_RBUS cap_ctl1;
     //cap_ctl0_RBUS cap_ctl0;
     h3ddma_cap0_cap_ctl0_RBUS cap_ctl0;
     //tg_v1_end_RBUS tg_v1_end;
     h3ddma_tg_v1_end_RBUS tg_v1_end;
     //i3ddma_enable_RBUS i3ddma_enable;
     h3ddma_i3ddma_enable_RBUS i3ddma_enable;
//   i3ddma_i3ddma_byte_channel_swap_RBUS  i3ddma_byte_channel_swap;
     h3ddma_cap0_wr_dma_pxltobus_RBUS h3ddma_cap0_wr_dma_pxltobus_reg;


    I3DDMA_disp2tve_CapBufInit();

    // dispi hdmi_3ddma
     lr_separate_ctrl1.regValue = IoReg_Read32(H3DDMA_LR_Separate_CTRL1_reg);
     lr_separate_ctrl1.hdmi_3d_structure = 0x0a;
     lr_separate_ctrl1.progressive = 1;
    // lr_separate_ctrl1.hdmi_3d_sel = 0;
     IoReg_Write32(H3DDMA_LR_Separate_CTRL1_reg,lr_separate_ctrl1.regValue);

    lr_separate_ctrl2.regValue = IoReg_Read32(H3DDMA_LR_Separate_CTRL2_reg);
    lr_separate_ctrl2.hact = outsize->nWidth;
    lr_separate_ctrl2.vact = outsize->nLength;
       IoReg_Write32(H3DDMA_LR_Separate_CTRL2_reg,lr_separate_ctrl2.regValue);

    //Capture
    //check this item
    rtd_pr_i3ddma_debug("[scart out]I3DDMA: 422cap = %d, 8bit=%d\n",Scaler_DispGetStatus(SLR_SUB_DISPLAY,SLR_DISP_422CAP), !Scaler_DispGetStatus(SLR_SUB_DISPLAY,SLR_DISP_10BIT));
    //if(Scaler_DispGetStatus(SLR_SUB_DISPLAY,SLR_DISP_422CAP)) //422
    //  lineunits = (outsize->nWidth * 8 * 2) /128;
    //else   //444
    //  lineunits = (outsize->nWidth * 8 * 3) /128;
    //len = 32;
    //num = (unsigned int)(lineunits / len);
    //rem = (unsigned int)(lineunits % len);
#if 0
    cap_ctl1.regValue = IoReg_Read32(H3DDMA_Cap_CTL1_reg);
    cap_ctl1.cap_write_num = num;
    cap_ctl1.cap_water_lv = len;
    IoReg_Write32(H3DDMA_Cap_CTL1_reg,cap_ctl1.regValue);
#endif
    cap_ctl0.regValue = IoReg_Read32(H3DDMA_CAP0_Cap_CTL0_reg);
    h3ddma_cap0_wr_dma_pxltobus_reg.regValue=IoReg_Read32(H3DDMA_CAP0_WR_DMA_pxltobus_reg);
//  cap_ctl0.cap_write_rem = rem;
//  cap_ctl0.cap_write_len = len;
    cap_ctl0.cap0_auto_block_sel_en = 1;
//  cap_ctl0.vs_fall_rst_en = 1;
    cap_ctl0.cap0_quad_buf_en = 0;

    #if 0
    if (scaler_dispDtoI3ddma_get_enable() == TRUE) {
        cap_ctl0.triple_buf_en= 1;//tve use 3-buffer //scart out use 3-buffer
    } else {
    if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_THRIP))
        cap_ctl0.quad_buf_en= 1;//interlaced use 4-buffer to jump even and odd field
    else
        cap_ctl0.triple_buf_en= 1;//tve use 3-buffer //scart out use 3-buffer
    }
    #endif
    cap_ctl0.cap0_quad_buf_en= 1;
    if(Scaler_DispGetStatus(SLR_SUB_DISPLAY,SLR_DISP_422CAP)) //422
    {
        h3ddma_cap0_wr_dma_pxltobus_reg.cap0_pixel_encoding = 1;
        h3ddma_cap0_wr_dma_pxltobus_reg.cap0_bit_sel = 0;

     }
     else   //444
    {
         h3ddma_cap0_wr_dma_pxltobus_reg.cap0_bit_sel=0;
         h3ddma_cap0_wr_dma_pxltobus_reg.cap0_pixel_encoding=0;

    }

    IoReg_Write32(H3DDMA_CAP0_Cap_CTL0_reg,cap_ctl0.regValue);
     IoReg_Write32(H3DDMA_CAP0_WR_DMA_pxltobus_reg, h3ddma_cap0_wr_dma_pxltobus_reg.regValue);
    // why enable tripple buffer?, reset FIFO address at VS fall, HW auto set memory address, pixel encode 422,
    // length of data to fill in DDR LEN= 32, remaining part REM= 26

    // 2:2 pull up disable, first VS falling edge pixel num = 2, first VS falling edge line num = 2
     tg_v1_end.regValue = IoReg_Read32(H3DDMA_TG_v1_end_reg);
     tg_v1_end.tg_vend1= 2;
     tg_v1_end.tg_hend1= 2;
     tg_v1_end.pullup_en = 0;
     IoReg_Write32(H3DDMA_TG_v1_end_reg,tg_v1_end.regValue);
    // because input is a 2D signal, the value here is important here, just don't be equal to 0?
      i3ddma_enable.regValue = IoReg_Read32(H3DDMA_I3DDMA_enable_reg);
//   i3ddma_enable.cap_en = 1;
     i3ddma_enable.lr_separate_en =1;
     i3ddma_enable.tg_en =1;
     if(Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_THRIP))
        i3ddma_enable.frc_en = 1;
     else
         i3ddma_enable.frc_en = 0;
    IoReg_Write32(H3DDMA_I3DDMA_enable_reg,i3ddma_enable.regValue);// FRC, timing gen, 3D LR separate, Capture enable

    // 8-byte swap
    IoReg_Mask32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, ~H3DDMA_CAP0_CTI_DMA_WR_Ctrl_cap0_dma_8byte_swap_mask, H3DDMA_CAP0_CTI_DMA_WR_Ctrl_cap0_dma_8byte_swap(0x1));

    //scart out use 3-buffer
    IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg, idma_cap_buf[0].phyaddr);  // block0, DDR buffer L1 start address
    IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_1_reg,  idma_cap_buf[1].phyaddr);  // block1, DDR buffer L2 start address
    IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_2_reg,  idma_cap_buf[2].phyaddr);  // block2, DDR buffer L3 start address
    IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_3_reg,  idma_cap_buf[3].phyaddr);  // block3, DDR buffer L4 start address

}

void  drvif_I3DDMA_Set_ViCapture(SIZE outsize)
{
#define CAP_LEN 32
#define CAP_REM 12

    //unsigned int num, rem;
//fixed me, remove in mag2 @ Crixus 0305 i3ddma_rgb2yuv_dither_4xxto4xx_hdmi_4xxto4xx_ctrl_RBUS hdmi_4xxto4xx_ctrl_reg;
    //lr_separate_ctrl1_RBUS lr_separate_ctrl1_reg;
    h3ddma_lr_separate_ctrl1_RBUS lr_separate_ctrl1_reg;
     //cap_ctl1_RBUS cap_ctl1;
     //h3ddma_cap_ctl1_RBUS cap_ctl1_reg;
     //cap_ctl0_RBUS cap_ctl0;
     h3ddma_cap0_cap_ctl0_RBUS cap_ctl0_reg;

    h3ddma_cap0_wr_dma_num_bl_wrap_addr_0_RBUS cap_l1_start_reg;
    h3ddma_cap0_wr_dma_num_bl_wrap_addr_1_RBUS cap_l2_start_reg;
    h3ddma_cap0_wr_dma_num_bl_wrap_addr_r0_RBUS cap_r1_start_reg;
    h3ddma_cap0_wr_dma_num_bl_wrap_addr_r1_RBUS cap_r2_start_reg;

    h3ddma_tg_v1_end_RBUS tg_v1_end_reg;
    h3ddma_cap0_cti_dma_wr_ctrl_RBUS i3ddma_byte_channel_swap_reg;
    h3ddma_i3ddma_enable_RBUS i3ddma_enable_reg;
    h3ddma_cap0_wr_dma_pxltobus_RBUS h3ddma_cap0_wr_dma_pxltobus_reg;

    I3DDMA_3DDMAInit();

#if 0//fixed me, remove in mag2 @ Crixus 0305
    // 444 to 422 convert in IDMA UZD
    hdmi_4xxto4xx_ctrl_reg.regValue = IoReg_Read32(I3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_4XXTO4XX_CTRL_VADDR);
    hdmi_4xxto4xx_ctrl_reg.yuv_4xxto4xx_sel = 0; //  don't convert
    IoReg_Write32(I3DDMA_RGB2YUV_DITHER_4XXTO4XX_HDMI_4XXTO4XX_CTRL_VADDR, hdmi_4xxto4xx_ctrl_reg.regValue);
#endif
    // IDMA LR separate control
    lr_separate_ctrl1_reg.regValue = IoReg_Read32(H3DDMA_LR_Separate_CTRL1_reg);
    //lr_separate_ctrl1_reg.chess_board_format = 0;
    //lr_separate_ctrl1_reg.hdmi_3d_sel = 0;
    lr_separate_ctrl1_reg.vs_inv = 0;
    lr_separate_ctrl1_reg.hs_inv = 0;
    lr_separate_ctrl1_reg.progressive = 1;
    lr_separate_ctrl1_reg.hdmi_3d_structure = 0xa; // frame 2D
    IoReg_Write32(H3DDMA_LR_Separate_CTRL1_reg, lr_separate_ctrl1_reg.regValue);

    // IDMA capture FIFO setting
    //num = (outsize.nWidth - CAP_REM) / CAP_LEN;
    //rem = ((outsize.nWidth - CAP_REM) % CAP_LEN) + CAP_REM;

    cap_ctl0_reg.regValue = IoReg_Read32(H3DDMA_CAP0_Cap_CTL0_reg);
    cap_ctl0_reg.cap0_triple_buf_en = 0;
    cap_ctl0_reg.cap0_quad_buf_en = 0;
    cap_ctl0_reg.cap0_freeze_en = 0;
//  cap_ctl0_reg.bit_sel = 1;           // 10bit (display output 10bit)
//  cap_ctl0_reg.pixel_encoding = 1;    // 422 format (VI input format support 420 or 422)
//  cap_ctl0_reg.cap_write_len = CAP_LEN;
//  cap_ctl0_reg.cap_write_rem = rem;
    IoReg_Write32(H3DDMA_CAP0_Cap_CTL0_reg, cap_ctl0_reg.regValue);

    h3ddma_cap0_wr_dma_pxltobus_reg.regValue=IoReg_Read32(H3DDMA_CAP0_WR_DMA_pxltobus_reg);
    h3ddma_cap0_wr_dma_pxltobus_reg.cap0_bit_sel=1;
    h3ddma_cap0_wr_dma_pxltobus_reg.cap0_pixel_encoding=1;
    IoReg_Write32(H3DDMA_CAP0_WR_DMA_pxltobus_reg, h3ddma_cap0_wr_dma_pxltobus_reg.regValue);
#if 0
    cap_ctl1_reg.regValue = IoReg_Read32(H3DDMA_Cap_CTL1_reg);
    cap_ctl1_reg.cap_water_lv = CAP_LEN;
    cap_ctl1_reg.cap_write_num = num;
    IoReg_Write32(H3DDMA_Cap_CTL1_reg, cap_ctl1_reg.regValue);
#endif
    i3ddma_byte_channel_swap_reg.regValue = IoReg_Read32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg);
    i3ddma_byte_channel_swap_reg.cap0_dma_1byte_swap = 0;
    i3ddma_byte_channel_swap_reg.cap0_dma_2byte_swap = 1;
    i3ddma_byte_channel_swap_reg.cap0_dma_4byte_swap = 1;
    i3ddma_byte_channel_swap_reg.cap0_dma_8byte_swap = 1;
    IoReg_Write32(H3DDMA_CAP0_CTI_DMA_WR_Ctrl_reg, i3ddma_byte_channel_swap_reg.regValue);

    cap_l1_start_reg.cap0_sta_addr_0 = (UINT32)i3ddmaCtrl.cap_buffer[0].phyaddr >> 3;
    cap_r1_start_reg.cap0_sta_addr_r0 = (UINT32)i3ddmaCtrl.cap_buffer[0].phyaddr >> 3;
    cap_l2_start_reg.cap0_sta_addr_1 = (UINT32)i3ddmaCtrl.cap_buffer[1].phyaddr >> 3;
    cap_r2_start_reg.cap0_sta_addr_r1 = (UINT32)i3ddmaCtrl.cap_buffer[1].phyaddr >> 3;
    IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_0_reg,  cap_l1_start_reg.regValue);  // block0, DDR buffer L1 start address
    IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_r0_reg,  cap_r1_start_reg.regValue);  // block0, DDR buffer R1 start address
    IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_1_reg,  cap_l2_start_reg.regValue);  // block0, DDR buffer L2 start address
    IoReg_Write32(H3DDMA_CAP0_WR_DMA_num_bl_wrap_addr_r1_reg,  cap_r2_start_reg.regValue);  // block0, DDR buffer R1 start address

    tg_v1_end_reg.regValue = IoReg_Read32(H3DDMA_TG_v1_end_reg);
    tg_v1_end_reg.pullup_en = 0;
    tg_v1_end_reg.tg_hend1 = 2;
    tg_v1_end_reg.tg_vend1 = 2;
    IoReg_Write32(H3DDMA_TG_v1_end_reg, tg_v1_end_reg.regValue );

    // IDMA timeing gen enable
    i3ddma_enable_reg.regValue = IoReg_Read32(H3DDMA_I3DDMA_enable_reg);
    i3ddma_enable_reg.frc_en = 0;
    i3ddma_enable_reg.lr_separate_en = 1;
    i3ddma_enable_reg.tg_en = 1;
//  i3ddma_enable_reg.cap_en = 0; // disable IDMA capture before VI capture enable
    IoReg_Write32(H3DDMA_I3DDMA_enable_reg, i3ddma_enable_reg.regValue );

    return;
}

void drvif_I3DDMA_set_3ddma_hwMode(unsigned char enable,unsigned char display)
{
    if(enable){
        I3DDMA_Setup3DDMA(&i3ddmaCtrl, I3DDMA_3D_OPMODE_HW,display);
        I3DDMA_ForceVODMA2D(0, 0);
    }else{
        // Stop HDMI DMA & VO DMA
//      I3DDMA_ResetVODMA4K2K();
        I3DDMA_Setup3DDMA(&i3ddmaCtrl, I3DDMA_3D_OPMODE_DISABLE,display);
        // wait DMA write done
        msleep(100);
    }
    return;
}
#endif
void drv_I3DDMA_ApplySetting(unsigned char display)
{
#ifndef CONFIG_ENABLE_HDMI_NN
    SIZE insize,outsize;
#endif

#if 0
    if(i3ddmaCtrl.hw_hsd_enable == 1){
        I3DDMA_ultrazoom_config_h_scaling_down(i3ddmaCtrl.input_h_size, i3ddmaCtrl.output_h_size, 0);
        i3ddmaCtrl.ptx_timing->h_act_len = i3ddmaCtrl.output_h_size;
    }
#endif

    if(i3ddmaCtrl.ptx_timing == NULL || i3ddmaCtrl.pgen_timing == NULL)
    {
        return;
    }

    I3DDMA_Get3DGenTiming(i3ddmaCtrl.ptx_timing, i3ddmaCtrl.pgen_timing);

    //HDMI 4096x2160 case uzd to 3840x2160 @Crixus 20170620
#ifndef CONFIG_ENABLE_HDMI_NN
    //if((i3ddmaCtrl.ptx_timing->h_act_len > 3840) /*&& (get_HDMI_HDR_mode() == HDR_HDR10_HDMI)*/)
    if(0)
    {
        insize.nLength = i3ddmaCtrl.ptx_timing->v_act_len;
        insize.nWidth = i3ddmaCtrl.ptx_timing->h_act_len;
        outsize.nLength = i3ddmaCtrl.ptx_timing->v_act_len;
        outsize.nWidth = 3840;
        i3ddmaCtrl.pgen_timing->h_act_len = 3840;
        I3DDMA_ultrazoom_config_scaling_down(&insize, &outsize, 1);

    }
#if 1//def CONFIG_FORCE_RUN_I3DDMA
    else if((i3ddmaCtrl.ptx_timing->h_act_len ==1440) && (DMA_VGIP_DMA_VGIP_CTRL_get_dma_in_sel(rtd_inl(DMA_VGIP_DMA_VGIP_CTRL_reg)) == VGIP_SRC_ATV))
    {
        rtd_pr_i3ddma_debug("av source,1440->960\n");
        insize.nLength = i3ddmaCtrl.ptx_timing->v_act_len;
        insize.nWidth = i3ddmaCtrl.ptx_timing->h_act_len;
        outsize.nLength = i3ddmaCtrl.ptx_timing->v_act_len;

#ifdef CONFIG_ENABLE_VD_27MHZ_MODE
        //self-dianosis AV source force to 704, rzhen@2016-09-01
        if((Get_DisplayMode_Src(display) == VSC_INPUTSRC_AVD)
            && Get_Factory_SelfDiagnosis_Mode()){
            outsize.nWidth = 704;
            i3ddmaCtrl.pgen_timing->h_act_len = 704;
        }
        else
#endif
        {
            outsize.nWidth = 960;
            i3ddmaCtrl.pgen_timing->h_act_len = 960;
        }

        I3DDMA_ultrazoom_config_scaling_down(&insize, &outsize, 1);
    }
    else if((i3ddmaCtrl.ptx_timing->h_act_len > 1920) && (i3ddmaCtrl.ptx_timing->v_act_len > 240)/*2880x576i case*/
        && (i3ddmaCtrl.ptx_timing->progressive == 0)/*only interlace*/
        && (DMA_VGIP_DMA_VGIP_CTRL_get_dma_in_sel(rtd_inl(DMA_VGIP_DMA_VGIP_CTRL_reg)) == VGIP_SRC_TMDS))
    {
        // 2880x576i case need to scale down to 1920x576i @Crixus 20160725
        //rtd_pr_i3ddma_emerg("HDMI 2880x576i => 1920x576i\n");
        insize.nLength = i3ddmaCtrl.ptx_timing->v_act_len;
        insize.nWidth = i3ddmaCtrl.ptx_timing->h_act_len;
        outsize.nLength = i3ddmaCtrl.ptx_timing->v_act_len;
        outsize.nWidth = 1920;
        i3ddmaCtrl.pgen_timing->h_act_len = 1920;

        I3DDMA_ultrazoom_config_scaling_down(&insize, &outsize, 1);
    }
#endif
    else
#endif
    {
        //h3ddma_hsd_i3ddma_hsd_ctrl0_RBUS i3ddma_hsd_ctrl0_reg;
        //h3ddma_hsd_i3ddma_vsd_ctrl0_RBUS i3ddma_vsd_ctrl0_reg;
        //i3ddma_hsd_ctrl0_reg.regValue = IoReg_Read32(H3DDMA_HSD_I3DDMA_HSD_Ctrl0_reg);
        //i3ddma_hsd_ctrl0_reg.h_zoom_en = 0;
        //IoReg_Write32(H3DDMA_HSD_I3DDMA_HSD_Ctrl0_reg, i3ddma_hsd_ctrl0_reg.regValue);

        //reset v-scale down buffer mode @Crixus 20160804
        //i3ddma_vsd_ctrl0_reg.regValue = IoReg_Read32(H3DDMA_HSD_I3DDMA_VSD_Ctrl0_reg);
        //i3ddma_vsd_ctrl0_reg.v_zoom_en = 0;
        //i3ddma_vsd_ctrl0_reg.buffer_mode = 0;
        //IoReg_Write32(H3DDMA_HSD_I3DDMA_VSD_Ctrl0_reg, i3ddma_vsd_ctrl0_reg.regValue);
    }

    I3DDMA_BandWidth_Control(i3ddmaCtrl.ptx_timing, i3ddmaCtrl.pgen_timing);
    I3DDMA_Dithering12to10(i3ddmaCtrl.ptx_timing->depth, i3ddmaCtrl.pgen_timing->depth);
    I3DDMA_4xxto4xx_Downsample(i3ddmaCtrl.ptx_timing->color, i3ddmaCtrl.pgen_timing->color);



    switch (i3ddmaCtrl.hw_i3ddma_dma) {

        case I3DDMA_3D_OPMODE_HW_2DONLY_L:
              I3DDMA_Setup3DDMA(&i3ddmaCtrl, I3DDMA_3D_OPMODE_HW_2DONLY_L,display);
              I3DDMA_ForceVODMA2D(1, 1);
        break;
        case I3DDMA_3D_OPMODE_HW_2DONLY_R:
              I3DDMA_Setup3DDMA(&i3ddmaCtrl, I3DDMA_3D_OPMODE_HW_2DONLY_R,display);
              I3DDMA_ForceVODMA2D(1, 0);
        break;
        case I3DDMA_3D_OPMODE_HW:
              I3DDMA_Setup3DDMA(&i3ddmaCtrl, I3DDMA_3D_OPMODE_HW,display);
              I3DDMA_ForceVODMA2D(0, 0);
        break;

        case I3DDMA_3D_OPMODE_DISABLE:
            I3DDMA_Setup3DDMA(&i3ddmaCtrl, I3DDMA_3D_OPMODE_DISABLE,display);
        break;
        case I3DDMA_3D_OPMODE_HW_NODMA:
            I3DDMA_Setup3DDMA(&i3ddmaCtrl, I3DDMA_3D_OPMODE_HW_NODMA,display);
        break;
        case I3DDMA_3D_OPMODE_SW:
            break;
        default:
                rtd_pr_i3ddma_debug("unknow HDMI 3D DMA mode\n");
        break;

    }
    if(dvrif_i3ddma_compression_get_enable()){
        //Re-order control for HDMI Dolby @Crixus 20180322
#ifndef BUILD_QUICK_SHOW
        drvif_I3DDMA_set_reorder_mode(get_i3ddma_dolby_reorder_mode());
#endif
        dvrif_i3ddma_comp_setting(TRUE, i3ddmaCtrl.pgen_timing->h_act_len, i3ddmaCtrl.pgen_timing->v_act_len, dvrif_i3ddma_get_compression_bits());
    }
    else{
        //Re-order control for HDMI Dolby @Crixus 20180322
#ifndef BUILD_QUICK_SHOW
        drvif_I3DDMA_set_reorder_mode(_DISABLE);
#endif
        dvrif_i3ddma_comp_setting(FALSE, i3ddmaCtrl.pgen_timing->h_act_len, i3ddmaCtrl.pgen_timing->v_act_len, 0);
    }
}
#ifndef BUILD_QUICK_SHOW
char drvif_I3DDMA_3D_FeatureEnable(char enable) {
    static char curEnStatus=0;

#ifndef ENABLE_HDMI_DMA_FOR_4K2K
    i3ddmaCtrl.force_all_3d_disable = enable == 0;
#endif
    if (i3ddmaCtrl.force_all_3d_disable) {
        I3DDMA_3DDMADeInit();
    } else {
        // [IDMA] allocate/release IDMA memory dynamic
        if(I3DDMA_Get3DAllocReleaseMemoryDynamic() == FALSE)
        I3DDMA_3DDMAInit();
    }

    if(enable != curEnStatus)
        i3ddmaCtrl.resume = 1;
    curEnStatus = enable;

    rtd_pr_i3ddma_debug("hdmi.force = %d %d\n", i3ddmaCtrl.force_all_3d_disable, enable);
    return TRUE;

}
#endif
unsigned long drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_LIST input_type)
{
    switch(input_type)
    {
        case I3DDMA_INFO_HW_I3DDMA_DMA:
            return (UINT32) i3ddmaCtrl.hw_i3ddma_dma;
        break;

        case I3DDMA_INFO_HW_DITHER12X10_ENABLE:
            return i3ddmaCtrl.hw_dither12x10_enable;
        break;

        case I3DDMA_INFO_HW_DITHER10x8_ENABLE:
            return i3ddmaCtrl.hw_dither10x8_enable;
        break;

        case I3DDMA_INFO_HW_4XXTO4XX_ENABLE:
            return i3ddmaCtrl.hw_4xxto4xx_enable;
        break;

        case I3DDMA_INFO_HW_RGBTOYUV_ENABLE:
            return i3ddmaCtrl.hw_rgbtoyuv_enable;
        break;

        case I3DDMA_INFO_INPUT_TIMING:
            return (unsigned long) i3ddmaCtrl.ptx_timing;
        break;

        case I3DDMA_INFO_GEN_TIMING:
            return (unsigned long) i3ddmaCtrl.pgen_timing;
        break;

        case I3DDMA_INFO_FRAMERATE_X2:
            return i3ddmaCtrl.frame_rate_x2;
        break;

        case I3DDMA_INFO_ENABLE_3DDMA:
            return i3ddmaCtrl.enable_3ddma;
        break;

        case I3DDMA_INFO_HW_HSD_ENABLE:
            return i3ddmaCtrl.hw_hsd_enable;
        break;

        case I3DDMA_INFO_INPUT_SRC_IDX:
            return i3ddmaCtrl.input_src_idx;
        break;

        case I3DDMA_INFO_INPUT_SRC_TYPE:
            return i3ddmaCtrl.input_src_type;
        break;

        case I3DDMA_INFO_INPUT_SRC_FROM:
            return i3ddmaCtrl.input_src_from;
        break;

        case I3DDMA_INFO_INPUT_MODE_CURR:
            return i3ddmaCtrl.input_src_mode;
        break;

        case I3DDMA_INFO_INPUT_H_COUNT:
            return i3ddmaCtrl.input_h_Count;
        break;

        case I3DDMA_INFO_INPUT_H_SIZE:
            return i3ddmaCtrl.input_h_size;
        break;

        case I3DDMA_INFO_OUTPUT_H_SIZE:
            return i3ddmaCtrl.output_h_size;
        break;

        case I3DDMA_INFO_TARGET_DEPTH:
            return i3ddmaCtrl.targetDepth;
        break;

        case I3DDMA_INFO_TARGET_COLOR:
            return i3ddmaCtrl.targetColor;
        break;
        case I3DDMA_INFO_ENABLE_4XXTO4XX:
        case I3DDMA_INFO_PIXEL_BITS:
            break;
        default:
            break;

    };

    return 0;
}


void drvif_I3DDMA_Set_Input_Info_Vaule(I3DDMA_INFO_LIST input_type, unsigned long value)
{
    switch(input_type)
    {
        case I3DDMA_INFO_HW_I3DDMA_DMA:
            i3ddmaCtrl.hw_i3ddma_dma = (I3DDMA_3D_OPMODE_T) value;
        break;

        case I3DDMA_INFO_HW_DITHER12X10_ENABLE:
            i3ddmaCtrl.hw_dither12x10_enable= value;
        break;

        case I3DDMA_INFO_HW_DITHER10x8_ENABLE:
            i3ddmaCtrl.hw_dither10x8_enable= value;
        break;

        case I3DDMA_INFO_HW_4XXTO4XX_ENABLE:
            i3ddmaCtrl.hw_4xxto4xx_enable= value;
        break;

        case I3DDMA_INFO_HW_RGBTOYUV_ENABLE:
            i3ddmaCtrl.hw_rgbtoyuv_enable= value;
        break;

        case I3DDMA_INFO_INPUT_TIMING:
        if((i3ddmaCtrl.ptx_timing != NULL) && (((void*) value) != NULL))
        {
            #ifdef CONFIG_ARM64
                memcpy_toio((void*) i3ddmaCtrl.ptx_timing, (void*) value, sizeof(I3DDMA_TIMING_T));
            #else
                memcpy((void*) i3ddmaCtrl.ptx_timing, (void*) value, sizeof(I3DDMA_TIMING_T));
            #endif
        }
        break;

        case I3DDMA_INFO_GEN_TIMING:
        #ifdef CONFIG_ARM64
            memcpy_toio((void*) i3ddmaCtrl.pgen_timing, (void*) value, sizeof(I3DDMA_TIMING_T));
        #else
            memcpy((void*) i3ddmaCtrl.pgen_timing, (void*) value, sizeof(I3DDMA_TIMING_T));
        #endif
        break;

        case I3DDMA_INFO_FRAMERATE_X2:
            i3ddmaCtrl.frame_rate_x2= value;
        break;

        case I3DDMA_INFO_HW_HSD_ENABLE:
            i3ddmaCtrl.hw_hsd_enable = value;
        break;

        case I3DDMA_INFO_INPUT_SRC_IDX:
            i3ddmaCtrl.input_src_idx= value;
        break;

        case I3DDMA_INFO_INPUT_SRC_TYPE:
            i3ddmaCtrl.input_src_type= value;
        break;

        case I3DDMA_INFO_INPUT_SRC_FROM:
            i3ddmaCtrl.input_src_from= value;
        break;

        case I3DDMA_INFO_INPUT_MODE_CURR:
            i3ddmaCtrl.input_src_mode= value;
        break;

        case I3DDMA_INFO_INPUT_H_COUNT:
            i3ddmaCtrl.input_h_Count= value;
        break;

        case I3DDMA_INFO_INPUT_H_SIZE:
            i3ddmaCtrl.input_h_size= value;
        break;

        case I3DDMA_INFO_OUTPUT_H_SIZE:
            i3ddmaCtrl.output_h_size= value;
        break;

        case I3DDMA_INFO_TARGET_DEPTH:
            i3ddmaCtrl.targetDepth= (I3DDMA_COLOR_DEPTH_T) value;
        break;

        case I3DDMA_INFO_TARGET_COLOR:
            i3ddmaCtrl.targetColor= (I3DDMA_COLOR_SPACE_T) value;
        break;
        case I3DDMA_INFO_ENABLE_4XXTO4XX:
        case I3DDMA_INFO_PIXEL_BITS:
        case I3DDMA_INFO_ENABLE_3DDMA:
            break;
        default:
            break;

    };

}


void drvif_I3DDMA_DeInit(void)
{
    I3DDMA_3DDMADeInit();
    i3ddmaCtrl.ptx_timing = NULL;
    i3ddmaCtrl.pgen_timing = NULL;
}

void drvif_I3DDMA_Init(void)
{

    i3ddmaCtrl.hw_dither12x10_enable = 0;
    i3ddmaCtrl.hw_i3ddma_dma = I3DDMA_3D_OPMODE_HW;

// [HDMI-DMA] HDMI-DMA capture enable for 4k2k video timing
#ifdef ENABLE_HDMI_DMA_FOR_4K2K
    i3ddmaCtrl.force_all_3d_disable = 0;
    i3ddmaCtrl.force_2dto3d_enable = 1;
/* not defined
#elif ENABLE_DRIVER_I3DDMA
    i3ddmaCtrl.force_all_3d_disable = 0;
    i3ddmaCtrl.force_2dto3d_enable = 1;
*/
#else
    i3ddmaCtrl.force_all_3d_disable = 1;
    i3ddmaCtrl.force_2dto3d_enable = 0;
#endif

    i3ddmaCtrl.force_3dto2d_enable = 0;
    i3ddmaCtrl.force_2dto3d_mode = I3DDMA_2D_ONLY;
#ifndef CONFIG_FORCE_RUN_I3DDMA
    i3ddmaCtrl.i3ddma_3d_capability = 0;
#endif
    i3ddmaCtrl.enable_3ddma = 1;

//#ifdef HDMI_HAVE_HDMI_DMA
#if 0//ndef CONFIG_FORCE_RUN_I3DDMA
    i3ddmaCtrl.hw_dither10x8_enable = 0;
    i3ddmaCtrl.hw_4xxto4xx_enable = 0;
    i3ddmaCtrl.hw_rgbtoyuv_enable = 0;
    i3ddmaCtrl.cap_buffer[0].cache = NULL;
    i3ddmaCtrl.cap_buffer[1].cache = NULL;
    i3ddmaCtrl.cap_buffer[2].cache = NULL;
    i3ddmaCtrl.cap_buffer[3].cache = NULL;
#endif

//#ifdef HDMI_HAVE_HDMI_3D_TG

    memset((void*) &inputTiming, 0, sizeof(I3DDMA_TIMING_T));
    memset((void*) &genTiming, 0, sizeof(I3DDMA_TIMING_T));
    i3ddmaCtrl.ptx_timing = &inputTiming;
    i3ddmaCtrl.pgen_timing = &genTiming;

    // [IDMA] allocate/release IDMA memory dynamic
    if(I3DDMA_Get3DAllocReleaseMemoryDynamic() == FALSE)
    I3DDMA_3DDMAInit();

    return;
}

static unsigned char i3ddma_dolby_reorder_flag = _DISABLE;
void set_i3ddma_dolby_reorder_mode(unsigned char enable){
    i3ddma_dolby_reorder_flag = enable;
}

unsigned char get_i3ddma_dolby_reorder_mode(void){
    return i3ddma_dolby_reorder_flag;
}

void i3ddma_dolby_reorder_update_color_format(void)
{
    //reorder output format is 422 12bits
    Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_COLOR_SPACE, COLOR_YUV422);
    Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_COLOR_DEPTH, I3DDMA_COLOR_DEPTH_12B);
    Scaler_DispSetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422, 1);
}

void Set_Val_I3ddma3Dformat(I3DDMA_3D_T i3ddma3DType)
{
    i3ddma3Dformat = i3ddma3DType;
}

I3DDMA_3D_T Get_Val_I3ddma3Dformat(void)
{
    return i3ddma3Dformat;
}

void Set_Val_I3ddma3DMode(I3DDMA_3D_OPMODE_T emode)
{
    i3ddmaOpMode = emode;
}

I3DDMA_3D_OPMODE_T Get_Val_I3ddma3DMode(void)
{
    return i3ddmaOpMode;
}

