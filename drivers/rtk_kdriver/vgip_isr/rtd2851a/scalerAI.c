/*==========================================================================
    * Copyright (c)      Realtek Semiconductor Corporation, 2006
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for AI related functions.
 *
 * @author 	$Author$
 * @date 	$Date$
 * @version $Revision$
 */

/*============================ Module dependency  ===========================*/
#include <mach/rtk_log.h>
#include <tvscalercontrol/vip/ai_pq.h>
#include "vgip_isr/scalerAI.h"
#include "vgip_isr/scalerVIP.h"
#include <scaler/vipRPCCommon.h>
#include "gal/rtk_kadp_se.h"
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <rbus/color_sharp_reg.h>
#include <rbus/color_dcc_reg.h>
#include <rbus/color_icm_reg.h>
#include <rbus/od_reg.h>
#include <rbus/scaledown_reg.h>
#include <rbus/timer_reg.h>
#include <tvscalercontrol/i3ddma/i3ddma.h>
#include "vgip_isr/scalerDCC.h"
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include <rtk_ai.h>
#include "memc_isr/scalerMEMC.h"
#include "memc_isr/memc_reg_def.h"
#include "memc_isr/Driver/regio.h"
#include <tvscalercontrol/vip/nr.h>
#include <tvscalercontrol/scaler/scalercolorlib_tv001.h>

#include <rbus/lc_reg.h>

#define ENABLE_DUMP_BUFFER		0
#define SUPPORT_FACE_NUMBER		6
#define AI_TARGET_FPS			8
#define top_num 				9
#include <linux/jiffies.h>

#if defined(CONFIG_RTK_AI_DRV)
/*================================== Variables ==============================*/

//chen 0417
#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))
//#define abs(x) ( (x>0) ? (x) : (-(x)) )

#define	vip_malloc(x)	kmalloc(x, GFP_KERNEL)
#define	vip_free(x)	kfree(x)

unsigned char ic_version = 1; // 0: merlin5, 1: mac7p
unsigned char data_shift = 2; // 0: merlin5, 2: mac7p

unsigned short icm_table[290][60]; //get icm table
unsigned int dcc_table[129]; //get icm table

//AIInfo face_info[6];
AIInfo face_info_pre[6];
AI_IIR_Info face_iir_pre[6];
AI_IIR_Info face_iir_pre2[6];
AIInfo face_info_pre2[6];

// chen 0503
AI_IIR_Info face_iir_pre3[6];
AI_IIR_Info face_iir_pre4[6];
// chen 0703
AI_IIR_Info face_iir_pre5[6];

// db for FW
int icm_h_delta[6]={0};
int icm_s_delta[6]={0};
int icm_val_delta[6]={0};

// static //
static bool scene_change_flag=0;
static int scene_change_count=0;
static int frame_drop_count=0;
static int change_speed_t[6]={0};
static int AI_detect_value_blend[6]={0};
static int value_diff_pre[6]={0};
static int h_diff_pre[6]={0};
static int w_diff_pre[6]={0};
static int IOU_pre[6]={0};

// chen 0429
static int IOU_pre_max2[6]={0};
static int change_speed_t_dcc[6]={0};
static int AI_detect_value_blend_dcc[6]={0};
// end chen 0429

// chen 0527
static int change_speed_t_sharp[6]={0};
static int AI_detect_value_blend_sharp[6]={0};
//end chen 0527

// chen 0805
static int AI_DCC_global_blend=0;
//end chen 0805

// lesley 0821
static int AI_ICM_global_blend = 0;
static int AI_ICM_global_center_u = 0;
static int AI_ICM_global_center_v = 0;
static int AI_ICM_global_h_offset = 0;
static int AI_ICM_global_s_offset = 0;
static int AI_ICM_global_v_offset = 0;
//end lesley 0821

// lesley 0815
static int AI_DCC_global_center_y = 0;
static int AI_DCC_global_center_u = 0;
static int AI_DCC_global_center_v = 0;
// end lesley 0815

// lesley 0813
static int h_adj_pre[6] = {0};
static int s_adj_pre[6] = {0};
// end lesley 0813

// lesley 0808
static int v_adj_pre[6] = {0};

int change_speed_ai_sc = 0;
static bool ai_scene_change_flag=0;
static int ai_scene_change_count=0;
static int ai_scene_change_done=0;
static int change_sc_offset_sta=0;
// end lesley 0808

// chen 0815
static int AI_face_sharp_count=0;
//end chen 0815

// setting //
int scene_change=0;

#define apply_buf_num 16
unsigned char buf_idx_w = 0;
unsigned char buf_idx_r = 0;

AI_ICM_apply face_icm_apply[apply_buf_num][6];
AI_DCC_apply face_dcc_apply[apply_buf_num][6];

// chen 0527
AI_sharp_apply face_sharp_apply[apply_buf_num][6];
//end chen 0527

// chen 0808
static int face_dist_x[6][20]={0};
static int face_dist_y[6][20]={0};
//end chen 0808

// lesley 0712
AI_demo_draw face_demo_draw[apply_buf_num][6] = {0};
//end lesley 0712

// lesley 0829
int still_ratio[5] = {0};
AI_ICM_apply face_icm_apply_pre[6] = {0};
AI_DCC_apply face_dcc_apply_pre[6] = {0};
AI_sharp_apply face_sharp_apply_pre[6] = {0};
// end lesley 0829

// lesley 0906_2
int y_diff_pre[16] = {0};
int hue_ratio_pre[16] = {0};
int show_ai_sc = 0;
// end lesley 0906_2

// lesley 0903
AI_OLD_DCC_apply old_dcc_apply[apply_buf_num] = {0};
// end lesley 0903

// lesley 0910
DB_AI_RTK ai_db_set = {0};
// end lesley 0910

// chen 0815_2
bool AI_face_sharp_dynamic_single = 0;
bool AI_face_sharp_dynamic_global = 0;
// end chen 0815_2

// chen 0502 ........... setting
//DRV_AI_Ctrl_table ai_ctrl;
extern DRV_AI_Ctrl_table ai_ctrl;

// end chen 0502

// chen 0426
extern COLORELEM_TAB_T icm_tab_elem_write;
// end chen 0426

static unsigned int vo_photo_buf_pre = 0;
static unsigned char nn_buf_idx = 0;
static VIP_NN_CTRL VIP_NN_ctrl = {0};
#if 0 // lesley debug print
unsigned int vdecPAddrY=0;
unsigned int vdecPAddrC=0;
unsigned int hdmiPAddrY=0;
unsigned int hdmiPAddrC=0;
unsigned int sePAddrY=0;
unsigned int sePAddrC=0;
unsigned int voPhotoPAddr=0;
#endif
unsigned char bAIInited = false;
int tic_start = 0;
const unsigned char SE_cnt = 8;
unsigned char SE_pre = 0;

// lesley 0919
extern UINT8 v4l2_vpq_stereo_face;
// end lesley 0919

// lesley 0920
unsigned char signal_cnt = 0;
// end lesley 0920


// lesley 1002_1
TOOL_AI_INFO tool_ai_info;
// end lesley 1002_1

// lesley 1007
extern DRV_AI_Tune_ICM_table AI_Tune_ICM_TBL[10];
extern DRV_AI_Tune_DCC_table AI_Tune_DCC_TBL[10];
// end lesley 1007

// lesley 1008
int dcc_user_curve32[32] = {0};
int dcc_user_curve129[129] = {0};
unsigned char dcc_user_curve_write_flag = 0;
//extern void fwif_color_dcc_Curve_interp_tv006(signed int *curve32, signed int *curve129);// lesley 1014 TBD
// end lesley 1008

// lesley 1016
UINT8 reset_face_apply = 0;
// end lesley 1016

// lesley 1022
bool bg_flag = 0;
// end lesley 1022

// for scene_detection
int scene_nn = 0;
int scene_pq = 0;
unsigned int debug_draw_cnt = 0;
int nn_scene = 0;
int ai_scene_rtk_mode = 0; // difference between basic other modes+int scene_update = 0;
int scene_update = 0;
int osd_con=128;
int osd_bri=128;
int osd_sat=128;
int ai_face_rtk_mode = 0; // ai face pq effect on/off

unsigned int CDS_gain_range[20] = { // green rectangle for scene
// max 0:v_gain_p, 1:v_gain_n, 2:v_hv_p, 3:v_hv_n, 4:v_lv,   
// max 5:vedge_gain_p, 6:vedge_gain_n, 7:vedge_hv_p, 8:vedgee_hv_n, 9:vedge_lv,
// min 10:v_gain_p, 11:v_gain_n, 12:v_hv_p, 13:v_hv_n, 14:v_lv,
// min 15:vedge_gain_p, 16:vedge_gain_n, 17:vedge_hv_p, 18:vedgee_hv_n, 19:vedge_lv,
130,130,80,80,2, 60,60,25,25,0, 30,30,50,50,0, 10,10,5,5,0
};
unsigned int CDS_gain_cur[10] = {0};

extern DRV_AI_SCENE_Ctrl_table ai_scene_ctrl;
extern int change_speed_ai_sc;
extern bool scene_change_flag;
extern TV001_COLOR_TEMP_DATA_S vpqex_color_temp;

/*================================== Function ===============================*/
extern void h3ddma_get_NN_output_size(unsigned int *outputWidth, unsigned int *outputLength);
extern int h3ddma_get_NN_read_buffer(unsigned int *a_pulYAddr, unsigned int *a_pulCAddr);
extern unsigned int drvif_memory_get_data_align(unsigned int Value, unsigned int unit);
extern int GDMA_AI_Get_UI_size(int *ui_size_w,int *ui_size_h);
extern int GDMA_AI_SE_draw_block(int s_w,int s_h,int num,unsigned int *color,KGAL_RECT_T *ai_block, unsigned char draw_scene) ;
extern void GDMA_GetGlobalAlpha(unsigned int *alpha, int count);
//extern void Scaler_Set_DCC_Color_Independent_Table(unsigned char value);
#if 0 // mac7p AI TBD memc
extern unsigned char * Scaler_MEMC_GetCadence(void);
#endif
//extern webos_info_t  webos_tooloption;

typedef struct {
	unsigned int    x;  /* x coordinate of its top-left point */
	unsigned int    y;  /* y coordinate of its top-left point */
	unsigned int    w;  /* width of it */
	unsigned int    h;  /* height of it */
} HAL_VO_RECT_T;
typedef enum {
    HAL_VO_PIXEL_FORMAT_NONE = 0,   /* none of these */
    HAL_VO_PIXEL_FORMAT_PALETTE,    /* palette color type */
    HAL_VO_PIXEL_FORMAT_GRAYSCALE,  /* 8bit gray scale */
    HAL_VO_PIXEL_FORMAT_RGB,    /* 24bit RGB */
    HAL_VO_PIXEL_FORMAT_BGR,    /* 24bit RGB */
    HAL_VO_PIXEL_FORMAT_ARGB,   /* 32bit ARGB */
    HAL_VO_PIXEL_FORMAT_ABGR,   /* 32bit ABGR */
    HAL_VO_PIXEL_FORMAT_YUV444P,  /* planar format with each U/V values plane (YYYY UUUU VVVV) */
    HAL_VO_PIXEL_FORMAT_YUV444I,  /* planar format with interleaved U/V values (YYYY UVUVUVUV) */
    HAL_VO_PIXEL_FORMAT_YUV422P,  /* semi-planar format with each U/V values plane (2x1 subsampling ; YYYY UU VV) */
    HAL_VO_PIXEL_FORMAT_YUV422I,  /* semi-planar format with interleaved U/V values (2x1 subsampling ; YYYY UVUV) */
    HAL_VO_PIXEL_FORMAT_YUV422YUYV, /* interleaved YUV values (Y0U0Y0V0Y1U1Y1V1) for MStar Chip Vender */
    HAL_VO_PIXEL_FORMAT_YUV420P,  /* semi-planar format with each U/V values plane (2x2 subsampling ; YYYYYYYY UU VV) */
    HAL_VO_PIXEL_FORMAT_YUV420I,  /* semi-planar format with interleaved U/V values (2x2 subsampling ; YYYYYYYY UVUV) */
    HAL_VO_PIXEL_FORMAT_YUV400,   /* 8bit Y plane without U/V values */
    HAL_VO_PIXEL_FORMAT_YUV224P,  /* semi-planar format with each U/V values plane, 1 Ysamples has 2 U/V samples to horizontal (Y0Y1 U0U0U1U1V0V0V1V1) */
    HAL_VO_PIXEL_FORMAT_YUV224I,  /* semi-planar format with interleaved U/V values (Y0Y1 U0V0U0V0U1V1U1V1) */
    HAL_VO_PIXEL_FORMAT_YUV444SP,  /* sequential packed with non-planar (YUVYUVYUV...) */
    HAL_VO_PIXEL_FORMAT_ALPHAGRAYSCALE,    /* gray scale with alpha */
    HAL_VO_PIXEL_FORMAT_MAX,    /* maximum number of HAL_VO_PIXEL_FORMAT */
} HAL_VO_PIXEL_FORMAT;

typedef struct {
	unsigned char       *buf;           /* buffer pointer of decoded raw data */
	unsigned char       *ofs_y;         /* offset of Y component */
	unsigned char       *ofs_uv;        /* offset of UV components */
	unsigned int        len_buf;        /* buffer length of decoded raw data */
	unsigned int        stride;         /* stride size of decoded raw data */
	HAL_VO_RECT_T       rect;           /* image data rectangular */
	HAL_VO_PIXEL_FORMAT pixel_format;   /* pixel format of image */
} HAL_VO_IMAGE_T;
extern HAL_VO_IMAGE_T *VO_GetPictureInfo(void);

#if ENABLE_DUMP_BUFFER // lesley debug dump
#include <linux/fs.h>
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

int dump_data_to_file(char* tmpname, unsigned int * vir_y, unsigned int * vir_c, unsigned int size)
{
	struct file* filp = NULL;
	unsigned long outfileOffset = 0;

	char filename[500];

	//sprintf(filename, "/tmp/usb/sda/sda/SE/%s.raw", tmpname);
	//sprintf(filename, "/mnt/media_rw/FEE6-D472/SE/%s.raw", tmpname);
	sprintf(filename, "/tmp/%s.raw", tmpname);

	if (vir_y != 0) {
		filp = file_open(filename, O_RDWR | O_CREAT | O_APPEND, 0666);
		if (filp == NULL) {
			rtd_pr_vpq_ai_info( "(%d)open fail\n", __LINE__);
			return FALSE;
		}

		file_write(filp, outfileOffset, (unsigned char*)vir_y, size);
		file_sync(filp);
		file_close(filp);

		filp = file_open(filename, O_RDWR | O_CREAT | O_APPEND, 0666);
		if (filp == NULL) {
			rtd_pr_vpq_ai_info( "(%d)open fail\n", __LINE__);
			return FALSE;
		}

		file_write(filp, outfileOffset, (unsigned char*)vir_c, size/2);
		file_sync(filp);
		file_close(filp);

		return TRUE;
	} else {
		rtd_pr_vpq_ai_info( "dump fail\n");
		return FALSE;
	}
}

void dumpCheck(void){
	static unsigned int cnt=0;
	char name[100];

	unsigned int *pVir_addr;
	unsigned int *pVir_addr_uv;

	VIP_NN_CTRL *pNN;

	pNN = scalerAI_Access_NN_CTRL_STRUCT();
	cnt++;	

	if(rtd_inl(0xb8025128)&(0x1)){

		rtd_outl(0xB8025128, 0);
		pVir_addr = pNN->NN_data_Addr[VIP_NN_BUFFER_416FIT].pVir_addr_align;
		pVir_addr_uv = pNN->NN_data_Addr[VIP_NN_BUFFER_416FIT].pVir_addr_align+(416*416/4);

		sprintf(name, "i3se_416_%d", cnt);
		dump_data_to_file(name, pVir_addr, pVir_addr_uv, 416*416);
	}
	if(rtd_inl(0xb8025128)&(0x2)){

		rtd_pr_vpq_ai_emerg("[dump] aipq i3ddma se\n");
		
		rtd_outl(0xB8025128, 0);
		pVir_addr = pNN->NN_data_Addr[VIP_NN_BUFFER_224FULL].pVir_addr_align;
		pVir_addr_uv = pNN->NN_data_Addr[VIP_NN_BUFFER_224FULL].pVir_addr_align+(224*224/4);

		sprintf(name, "i3se_224_%d", cnt);
		dump_data_to_file(name, pVir_addr, pVir_addr_uv, 224*224);

		pVir_addr = pNN->NN_data_Addr[VIP_NN_BUFFER_NOT_USED].pVir_addr_align;
		pVir_addr_uv = pNN->NN_data_Addr[VIP_NN_BUFFER_NOT_USED].pVir_addr_align+(480*270/4);

		sprintf(name, "i3se_480_%d", cnt);
		dump_data_to_file(name, pVir_addr, pVir_addr_uv, 480*270);
	}		

}
#endif

int scalerAI_pq_mode_ctrl(unsigned char stereo_face, unsigned char dcValue)
{
extern UINT8 vpq_stereo_face;
	static int status = 0; // 0: finished. -1: not ready.
	od_od_ctrl_RBUS od_ctrl_reg;
	
	od_ctrl_reg.regValue = rtd_inl(OD_OD_CTRL_reg);	
					
	if(status == -1)
	{
		rtd_pr_vpq_ai_info("AI_Face mode setting not ready: stereo %d, dcValue %d\n", stereo_face, dcValue);
		return status;
	}

	status = -1;

	//if(stereo_face != AI_FACE_OFF)
	//	scalerAI_execute_NN(); // josh 1018
	
	switch(stereo_face)
	{
		case AI_PQ_AP_OFF:
		{
			rtd_pr_vpq_ai_emerg("aipq path off\n");
			od_ctrl_reg.regValue = od_ctrl_reg.regValue & (~_BIT28); // bit 28 for disable scene PQ
			od_ctrl_reg.regValue = od_ctrl_reg.regValue & (~_BIT27); // bit 27 for ai_scene_rtk_mode
			od_ctrl_reg.regValue = od_ctrl_reg.regValue & (~_BIT26); // bit 26 for ai_race_rtk_mode
			scalerAI_ai_pq_off();
			status = 0;
			vpq_stereo_face = AI_PQ_AP_OFF;
			break;
		}
		case AI_PQ_ALL_ON:
		{
			rtd_pr_vpq_ai_emerg("aipq path on\n");
			od_ctrl_reg.regValue = od_ctrl_reg.regValue | (_BIT28); // bit 28 for disable scene PQ
			od_ctrl_reg.regValue = od_ctrl_reg.regValue | (_BIT27); // bit 27 for ai_scene_rtk_mode
			od_ctrl_reg.regValue = od_ctrl_reg.regValue | (_BIT26); // bit 26 for ai_race_rtk_mode
			scalerAI_ai_pq_on(1, dcValue);
			status = 0;
			vpq_stereo_face = AI_PQ_ALL_ON;
			break;
		}
		case AI_PQ_DEMO:
		{
			rtd_pr_vpq_ai_emerg("aipq path demo\n");
			od_ctrl_reg.regValue = od_ctrl_reg.regValue | (_BIT28); // bit 28 for disable scene PQ
			od_ctrl_reg.regValue = od_ctrl_reg.regValue | (_BIT27); // bit 27 for ai_scene_rtk_mode
			od_ctrl_reg.regValue = od_ctrl_reg.regValue | (_BIT26); // bit 26 for ai_race_rtk_mode
			scalerAI_ai_pq_on(2, 4);
			status = 0;
			vpq_stereo_face = AI_PQ_DEMO;
			break;
		}
		case AI_PQ_SCENE_OFF:
		{
			rtd_pr_vpq_ai_emerg("aipq mode scene pq off\n");
			od_ctrl_reg.regValue = od_ctrl_reg.regValue | (_BIT28); // bit 28 for disable scene PQ
			od_ctrl_reg.regValue = od_ctrl_reg.regValue & (~_BIT27); // bit 27 for ai_scene_rtk_mode
			od_ctrl_reg.regValue = od_ctrl_reg.regValue | (_BIT26);  // bit 26 for ai_race_rtk_mode
			scalerAI_ai_pq_on(2, dcValue);
			status = 0;
			vpq_stereo_face = AI_PQ_SCENE_OFF;
			break;
		}
		case AI_PQ_FACE_OFF:
		{
			rtd_pr_vpq_ai_emerg("aipq mode facepq off\n");
			od_ctrl_reg.regValue = od_ctrl_reg.regValue | (_BIT28); // bit 28 for disable scene PQ
			od_ctrl_reg.regValue = od_ctrl_reg.regValue | (_BIT27); // bit 27 for ai_scene_rtk_mode
			od_ctrl_reg.regValue = od_ctrl_reg.regValue & (~_BIT26);  // bit 26 for ai_race_rtk_mode
			scalerAI_ai_pq_on(2, dcValue);
			status = 0;
			vpq_stereo_face = AI_PQ_FACE_OFF;
			break;
		}
		case AI_PQ_ALL_OFF:
		{
			rtd_pr_vpq_ai_emerg("aipq mode all pq off\n");
			od_ctrl_reg.regValue = od_ctrl_reg.regValue | (_BIT28); // bit 28 for disable scene PQ
			od_ctrl_reg.regValue = od_ctrl_reg.regValue & (~_BIT27); // bit 27 for ai_scene_rtk_mode
			od_ctrl_reg.regValue = od_ctrl_reg.regValue & (~_BIT26); // bit 26 for ai_race_rtk_mode
			scalerAI_ai_pq_on(2, dcValue);
			status = 0;
			vpq_stereo_face = AI_PQ_ALL_OFF;
			break;
		}
		default:
		{
			rtd_pr_vpq_ai_emerg("aipq path not support mode\n");
			od_ctrl_reg.regValue = od_ctrl_reg.regValue & (~_BIT27); // bit 27 for ai_scene_rtk_mode
			od_ctrl_reg.regValue = od_ctrl_reg.regValue & (~_BIT26); // bit 26 for ai_race_rtk_mode
			status = -1;
			break;		
		}
	}

	//rtd_pr_vpq_ai_info("[ai_hack] od dummy: 0x%x\n", od_ctrl_reg.regValue);
	rtd_outl(OD_OD_CTRL_reg, od_ctrl_reg.regValue);

	return status;
}


/*
	In the curved out memory,can only seperate for 3 buffers.
		VIP_NN_BUFFER_NOT_USED=0,
		VIP_NN_BUFFER_224FULL=1,
		VIP_NN_BUFFER_416FIT=2,
	For the new feature, use dvr_malloc to request the needed buffers
		VIP_NN_BUFFER_320X240FULL=3,
		VIP_NN_BUFFER_960X540=4,	(need special size)
		.....
*/
/* NN memory phy to vir */
unsigned char NN_Meomory_Addr_Init(void)
{
	VIP_NN_CTRL *pNN;
	unsigned int phy_addr, phy_addr_base, size;
	unsigned int *pVir_addr;
	unsigned char i;

	pNN = scalerAI_Access_NN_CTRL_STRUCT();
	phy_addr_base = get_query_start_address(QUERY_IDX_VIP_NN);

	if (phy_addr_base == 0 ) {
		rtd_pr_vpq_ai_emerg("NN memory NULL\n");
		return 0;
	}

	for (i=0;i<VIP_NN_BUFFER_NUM;i++) {
		if (pNN->NN_data_Addr[i].pVir_addr_align == NULL) {
			if(i==VIP_NN_BUFFER_320X240FULL){
				size = drvif_memory_get_data_align(VIP_NN_DATA_BUFFER_SIZE, (1 << 12));
				pVir_addr =(unsigned int *)dvr_malloc(size);
				phy_addr =(unsigned int)dvr_to_phys((void *)pVir_addr);
			}else if(i==VIP_NN_BUFFER_960X540){
				size = drvif_memory_get_data_align(VIP_NN_960X540_DATA_BUFFER_SIZE, (1 << 12));
				pVir_addr =(unsigned int *)dvr_malloc(size);
				phy_addr =(unsigned int)dvr_to_phys((void *)pVir_addr);
			}else{
				size = drvif_memory_get_data_align(VIP_NN_DATA_BUFFER_SIZE, (1 << 12));
				phy_addr = phy_addr_base + (i * VIP_NN_DATA_BUFFER_SIZE);
				phy_addr = drvif_memory_get_data_align(phy_addr, (1 << 12));
				pVir_addr = dvr_remap_uncached_memory(phy_addr, size, __builtin_return_address(0));
			}
			pNN->NN_data_Addr[i].phy_addr_align = phy_addr;
			pNN->NN_data_Addr[i].size = size;
			pNN->NN_data_Addr[i].pVir_addr_align = pVir_addr;

		}
	}

	for (i=0;i<VIP_NN_BUFFER_NUM;i++) {
		if (pNN->NN_info_Addr[i].pVir_addr_align == NULL) {
			size = drvif_memory_get_data_align(VIP_NN_INFO_BUFFER_SIZE, (1 << 12));
			if(i==VIP_NN_BUFFER_320X240FULL||i==VIP_NN_BUFFER_960X540){
				pVir_addr =(unsigned int *)dvr_malloc(size);
				phy_addr =(unsigned int)dvr_to_phys((void *)pVir_addr);
			}else{
				phy_addr = phy_addr_base + (3 * VIP_NN_DATA_BUFFER_SIZE + i * VIP_NN_INFO_BUFFER_SIZE);
				phy_addr = drvif_memory_get_data_align(phy_addr, (1 << 12));
				pVir_addr = dvr_remap_uncached_memory(phy_addr, size, __builtin_return_address(0));
			}
			pNN->NN_info_Addr[i].phy_addr_align = phy_addr;
			pNN->NN_info_Addr[i].size = size;
			pNN->NN_info_Addr[i].pVir_addr_align = pVir_addr;

		}
	}

	for (i=0;i<VIP_NN_BUFFER_NUM;i++) {
		if (pNN->NN_flag_Addr[i].pVir_addr_align == NULL) {
			size = drvif_memory_get_data_align(VIP_NN_FLAG_BUFFER_SIZE, (1 << 12));
			if(i==VIP_NN_BUFFER_320X240FULL||i==VIP_NN_BUFFER_960X540){
				pVir_addr =(unsigned int *)dvr_malloc(size);
				phy_addr =(unsigned int)dvr_to_phys((void *)pVir_addr);
			}else{
				phy_addr = phy_addr_base + (3 * VIP_NN_DATA_BUFFER_SIZE + 3 * VIP_NN_INFO_BUFFER_SIZE + i * VIP_NN_FLAG_BUFFER_SIZE);
				phy_addr = drvif_memory_get_data_align(phy_addr, (1 << 12));
				pVir_addr = dvr_remap_uncached_memory(phy_addr, size, __builtin_return_address(0));
			}
			pNN->NN_flag_Addr[i].phy_addr_align = phy_addr;
			pNN->NN_flag_Addr[i].size = size;
			pNN->NN_flag_Addr[i].pVir_addr_align = pVir_addr;

		}
	}

	if (pNN->NN_indx_Addr.pVir_addr_align == NULL) {
		phy_addr = phy_addr_base + (3 * VIP_NN_DATA_BUFFER_SIZE + 3 * VIP_NN_INFO_BUFFER_SIZE + 3 * VIP_NN_FLAG_BUFFER_SIZE);
		phy_addr = drvif_memory_get_data_align(phy_addr, (1 << 12));
		size = drvif_memory_get_data_align(VIP_NN_INDX_BUFFER_SIZE, (1 << 12));
		pNN->NN_indx_Addr.phy_addr_align = phy_addr;
		pNN->NN_indx_Addr.size = size;
		pVir_addr = dvr_remap_uncached_memory(phy_addr, size, __builtin_return_address(0));
		pNN->NN_indx_Addr.pVir_addr_align = pVir_addr;
	}

	for(i=0;i<VIP_NN_BUFFER_NUM;i++){
		rtd_pr_vpq_ai_info("[buffer %d], start data phy addr=%x, vir addr=%p\n",i,pNN->NN_data_Addr[i].phy_addr_align,pNN->NN_data_Addr[i].pVir_addr_align);
	}

	fw_scalerip_reset_NN();

	return 1;
}

static struct task_struct *aiInitTsk;
static int ai_init_data;
static int AI_init_thread(void *arg){
	rtd_pr_vpq_ai_emerg("AI_init_thread start\n");
	//msleep(10*1000);
	//rtd_pr_vpq_ai_emerg("AI_init_thread call\n");
	//scalerAI_execute_NN();	
	rtd_pr_vpq_ai_emerg("AI_init_thread end\n");
	return 0;
}

/* call from bootinit and resume */
void scalerAI_Init(void)
{
	// global variable controls the flow in vgip and d-domain ISR
	bAIInited = NN_Meomory_Addr_Init();

	if(ic_version==0)
		data_shift=0;
	else if(ic_version==1)
		data_shift=2;

	// init PQ
	if(bAIInited)
	{
		// lesley 0812, remove init, set by table
		//drvif_color_AI_obj_dcc_init();
		//drvif_color_AI_obj_icm_init();
		// end lesley 0812
		//drvif_color_AI_obj_srp_init(0);
		// old dcc color dependent
		//Scaler_Set_DCC_Color_Independent_Table(0);

		// lesley 1016
		drivef_ai_tune_icm_set(&AI_Tune_ICM_TBL[0]);
		drivef_ai_tune_dcc_set(&AI_Tune_DCC_TBL[0], 0);
		// end lesley 1016
		{ 			
			od_od_ctrl_RBUS od_ctrl_reg;			
			od_ctrl_reg.regValue = rtd_inl(OD_OD_CTRL_reg);				
			od_ctrl_reg.dummy1802ca00_31_7 = (1 << 24); // bit 31 for enabling i3ddma data
			IoReg_Write32(OD_OD_CTRL_reg, od_ctrl_reg.regValue);
		}
		scalerAI_pq_mode_ctrl(AI_PQ_DEMO,0);

#if 1		
    aiInitTsk = kthread_create(AI_init_thread, &ai_init_data, "AI init thread");
    if (IS_ERR(aiInitTsk)) {
    		int ret;
        ret = PTR_ERR(aiInitTsk);
        aiInitTsk = NULL;
        rtd_pr_vpq_ai_emerg("create AI init thread fail\n");
        return;
    }
    wake_up_process(aiInitTsk);		
#endif    

	}
}

/* called from I-domain, reset NN buffer as timing change */
char fw_scalerip_reset_NN(void)
{
	VIP_NN_CTRL *pNN;
	unsigned int *pVir_addr_align;
	unsigned short I_Width, I_Height;
	unsigned char i;


	I_Width = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);
	I_Height = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);

	pNN = scalerAI_Access_NN_CTRL_STRUCT();
	pVir_addr_align = pNN->NN_data_Addr[0].pVir_addr_align;

	if (pVir_addr_align == NULL) {
		rtd_pr_vpq_ai_emerg("[%s] vir_addr is null\n", __func__);
		return -1;
	}

	for (i=0;i<VIP_NN_BUFFER_NUM;i++)
		memset(pNN->NN_data_Addr[i].pVir_addr_align, 0, pNN->NN_data_Addr[i].size);

	for (i=0;i<VIP_NN_BUFFER_NUM;i++)
		memset(pNN->NN_info_Addr[i].pVir_addr_align, 0, pNN->NN_info_Addr[i].size);

	for (i=0;i<VIP_NN_BUFFER_NUM;i++)
		memset(pNN->NN_flag_Addr[i].pVir_addr_align, 0, pNN->NN_flag_Addr[i].size);

	rtd_pr_vpq_ai_info("[%s] reset done\n",__func__);

	scalerAI_set_nn_buf_idx(0);
	vo_photo_buf_pre = 0;
	memset(pNN->NN_indx_Addr.pVir_addr_align, 0, pNN->NN_indx_Addr.size);

	return 0;
}

/* get NN buffer write index */
unsigned char scalerAI_get_nn_buf_idx(void)
{
	return nn_buf_idx;
}

/* set NN buffer write index */
void scalerAI_set_nn_buf_idx(unsigned char idx)
{
	nn_buf_idx = idx;
}

VIP_NN_CTRL* scalerAI_Access_NN_CTRL_STRUCT(void)
{
	return &VIP_NN_ctrl;
}

#if 0 // for debug flag
void clean_flag(void)
{
	VIP_NN_CTRL *pNN;
	unsigned char cur_idx = 0;

	pNN = scalerAI_Access_NN_CTRL_STRUCT();
	cur_idx = pNN->NN_indx_Addr.pVir_addr_align[0];
	pNN->NN_flag_Addr[cur_idx].pVir_addr_align[0] = 0;
}

#endif

#if 0 // lesley debug print
void dumpbuf(unsigned char mode)
{
		unsigned int VirAddr=0;
		unsigned int in_addr_y, in_addr_c;
		unsigned int bufsize, bufsizeAlign;
		unsigned char *pTemp;
		int j;
		int w;

		if(mode==0)//vdec
		{
			in_addr_y = vdecPAddrY;
			in_addr_c = vdecPAddrC;
			bufsize = 960*540; //tbd
			w = 960;
		}
		else if(mode==1)//hdmi
		{
			in_addr_y = hdmiPAddrY;
			in_addr_c = hdmiPAddrC;
			bufsize = 416*234; //tbd
			w = 416;
		}
		else if(mode==2)//se
		{
			in_addr_y = sePAddrY;
			in_addr_c = sePAddrC;
			bufsize = 416*416;
			w = 416;
		}
		else if(mode==3)//vo photo
		{
			in_addr_y = voPhotoPAddr;
			bufsize = 3840*2160*3;
			w = 1200;//3840*3;//just for check
		}

		rtd_pr_vpq_ai_emerg("[%d] mode%d, yAddr:0x%x, cAddr:0x%x\n", __LINE__, mode, in_addr_y, in_addr_c);


		in_addr_y = drvif_memory_get_data_align(in_addr_y, (1 << 12));
		bufsizeAlign = drvif_memory_get_data_align(bufsize, (1 << 12));
		VirAddr = (unsigned int)dvr_remap_uncached_memory(in_addr_y, bufsizeAlign, __builtin_return_address(0));
		pTemp = (unsigned char *)VirAddr;

		for(j=0;j<w/4;j++)
		{
			rtd_pr_vpq_ai_emerg("mode%d, y %d, %d\n", mode, j, pTemp[j]);
		}

		dvr_unmap_memory((void *)VirAddr, bufsizeAlign);

		if(mode<3)
		{
			in_addr_c = drvif_memory_get_data_align(in_addr_c, (1 << 12));
			bufsizeAlign = drvif_memory_get_data_align(bufsize/2, (1 << 12));
			VirAddr = (unsigned int)dvr_remap_uncached_memory(in_addr_c, bufsizeAlign, __builtin_return_address(0));
			pTemp = (unsigned char *)VirAddr;

			for(j=0;j<w/4;j++)
			{
				rtd_pr_vpq_ai_emerg("mode%d, c %d, %d\n", mode, j, pTemp[j]);
			}

			dvr_unmap_memory((void *)VirAddr, bufsizeAlign);
		}
}
#endif

void scalerAI_SE_draw_scene(unsigned char *index, unsigned int *ai_color,KGAL_RECT_T *ai_block, unsigned int osd_blend, unsigned int total)
{
	int color = 0;
	unsigned char idx = *index;
	int gdma_w = 0, gdma_h = 0;
	unsigned int f_size = 0;
	
	GDMA_AI_Get_UI_size(&gdma_w, &gdma_h);

	if(gdma_w==1920 && gdma_h==1080)
	{
		f_size = 0;
	}
	else if(gdma_w==960 && gdma_h==540)
	{
		f_size = 1;
	}
	else if(gdma_w==480 && gdma_h==270)
	{
		f_size = 2;
	}
	else
	{
		rtd_pr_vpq_ai_emerg("[Err] GDMA_AI_Get_UI_size return wrong size, w:%d, h:%d\n", gdma_w, gdma_h);
		return;
	}

	ai_block[idx].x=100>>f_size;
	ai_block[idx].y=100>>f_size;
	ai_block[idx].w=100>>f_size;
	ai_block[idx].h=100>>f_size;
	
	if(scene_pq==PQ_SCENE_BASIC) 
	{
		color = 0x000f;	//r basic
		ai_block[idx].y += 0;
	}
	else if(scene_pq==PQ_SCENE_DARK)
	{ 
		color = 0xf000;//g darkscene
		ai_block[idx].y += (100>>f_size)*1;
	}
	else if(scene_pq==PQ_SCENE_SPORTs)
	{ 
		color = 0x0f00;//b sports
		ai_block[idx].y += (100>>f_size)*2;
	}
	else if(scene_pq==PQ_SCENE_BEAUTYSCENARY)
	{ 
		color = 0xff00;//cyan landscape
		ai_block[idx].y += (100>>f_size)*3;
	}
	else if(scene_pq==PQ_SCENE_ANIMATION)
	{ 
		color = 0x0f0f;//magenta animation
		ai_block[idx].y += (100>>f_size)*4;
	}
	else if(scene_pq==PQ_SCENE_MOVIE)
	{ 
		color = 0xf00f;//yello movie
		ai_block[idx].y += (100>>f_size)*5;
	}
	else
	{
		color = 0;
		ai_block[idx].x=0;
		ai_block[idx].y=0;
		ai_block[idx].w=0;
		ai_block[idx].h=0;
		rtd_pr_vpq_ai_emerg("[Err] scalerAI_SE_draw_scene get wrong scene class\n");
	}
	
	//ai_color[idx] = (unsigned int)((color & 0xff0f) | (((color & 0x00f0)>>4)*osd_blend/total)<<4);
	ai_color[idx] = (unsigned int)(color | 0x00f0);

	//rtd_pr_vpq_ai_emerg("rec[%d]: x:%d, y:%d, w:%d, h:%d, color:0x%04x\n", idx, ai_block[idx].x, ai_block[idx].y, ai_block[idx].w, ai_block[idx].h, ai_color[idx]);
	
	(*index)++;
}

void scalerAI_SE_draw_Proc(void)
{
#if defined(CONFIG_RTK_KDRV_SE)
	int status = 0;
	unsigned char i=0, draw_idx=0;
	//KGAL_RECT_T ai_block[6] = {0};
	//unsigned int ai_color[6] = {0};
	KGAL_RECT_T *ai_block = NULL;
	unsigned int *ai_color = NULL;
	static unsigned char clear_flag = 0;
	_RPC_system_setting_info* RPC_system_info_structure_table = NULL;
	//unsigned char en_nn;
	bool nn_flag = 0;
	unsigned int alpha = 0;
	unsigned int total = 100;
	unsigned int alpha_th = 10;
	unsigned int osd_blend = 0;
	int gdma_w = 0, gdma_h = 0;
	int signal_cnt_th;
	od_od_ctrl_RBUS od_ctrl_reg;
	unsigned char draw_scene = 0;

	signal_cnt_th = ai_ctrl.ai_global3.signal_cnt_th;
	
	ai_block = vip_malloc((SUPPORT_FACE_NUMBER+1)*sizeof(KGAL_RECT_T));
	ai_color = vip_malloc((SUPPORT_FACE_NUMBER+1)*sizeof(unsigned int));

	RPC_system_info_structure_table = scaler_GetShare_Memory_RPC_system_setting_info_Struct();

	//en_nn = (RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.en_nn)/*&&(webos_tooloption.eBackLight == 2)*/;// vdec en_nn set 1 for k6hp;

	od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
	if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC)
		nn_flag = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.en_nn;//en_nn;
	else
		nn_flag = (od_ctrl_reg.dummy1802ca00_31_7>>24 & 1); // bit 31

	/* Get gdma global alpha */
	GDMA_GetGlobalAlpha(&alpha, 1);
	if(alpha < alpha_th)
		osd_blend = total;
	else if(alpha > alpha_th + total)
		osd_blend = 0;
	else
		osd_blend = total - alpha + alpha_th;

	if((od_ctrl_reg.dummy1802ca00_31_7>>19 & 1) == 1) // bit 26
	{
		for(i=0; i<6; i++)
		{			
			if(face_demo_draw[buf_idx_r][i].w && face_demo_draw[buf_idx_r][i].h)
			{
				ai_block[draw_idx].x=face_demo_draw[buf_idx_r][i].x;
				ai_block[draw_idx].y=face_demo_draw[buf_idx_r][i].y;
				ai_block[draw_idx].w=face_demo_draw[buf_idx_r][i].w;
				ai_block[draw_idx].h=face_demo_draw[buf_idx_r][i].h;

				ai_color[draw_idx] = (unsigned int)((face_demo_draw[buf_idx_r][i].color & 0xff0f) | ((((face_demo_draw[buf_idx_r][i].color & 0x00f0)>>4)*osd_blend/total)<<4));

				//rtd_pr_vpq_ai_emerg("rec[%d]: x:%d, y:%d, w:%d, h:%d, color:0x%04x\n", draw_idx, ai_block[draw_idx].x, ai_block[draw_idx].y, ai_block[draw_idx].w, ai_block[draw_idx].h, ai_color[draw_idx]);

				draw_idx++;
			}
		}
	}

	debug_draw_cnt = draw_idx;
	
	/* Hack for scene demo */
	if((od_ctrl_reg.dummy1802ca00_31_7>>20 & 1) == 1) // bit 27
	{
		draw_scene = 1;
		scalerAI_SE_draw_scene(&draw_idx, ai_color, ai_block, osd_blend, total);
		//rtd_pr_vpq_ai_emerg("=====\n");
	}

	/* get gdma size by calling API */
	GDMA_AI_Get_UI_size(&gdma_w, &gdma_h);
	//rtd_pr_vpq_ai_emerg("draw gdma_w:%d, gdma_h:%d\n", gdma_w, gdma_h);

	if((bg_flag == 0) && (signal_cnt == signal_cnt_th) && (ai_ctrl.ai_global.demo_draw_en == 1) && nn_flag)
	{
		clear_flag = 1;
		status = GDMA_AI_SE_draw_block(gdma_w,gdma_h,draw_idx,ai_color,ai_block, draw_scene);
	}
	else if(clear_flag == 1)
	{
		clear_flag = 0;
		status = GDMA_AI_SE_draw_block(gdma_w,gdma_h,0,ai_color,ai_block, draw_scene);
	}

	vip_free(ai_block);
	vip_free(ai_color);
	ai_block = NULL;
	ai_color = NULL;
#endif
}

/* setting to call kernel api */
bool scalerAI_SE_stretch_Proc(SE_NN_info info) {
#if defined(CONFIG_RTK_KDRV_SE)

	unsigned int se_sta, se_end;
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
	ssurf.pixelFormat 		= info.src_fmt;
	srect.x = info.src_x;
	srect.y = info.src_y;
	srect.w = info.src_w;
	srect.h = info.src_h;

	dsurf.physicalAddress = info.dst_phyaddr;
	dsurf.width = info.dst_w;
	dsurf.height = info.dst_h;
	dsurf.pixelFormat 		= info.dst_fmt;
	drect.x = info.dst_x;
	drect.y = info.dst_y;
	drect.w = info.dst_w;
	drect.h = info.dst_h;

	if(info.src_fmt == KGAL_PIXEL_FORMAT_NV12)
	{
		ssurf.bpp 	= 16;
		ssurf.pitch = info.src_pitch_y;//info.src_w;
	}
	else if(info.src_fmt == KGAL_PIXEL_FORMAT_RGB888)//KGAL_PIXEL_FORMAT_YUV444
	{
		ssurf.bpp 	= 24;
		ssurf.pitch = info.src_pitch_y*3;//info.src_w*3;
		info.src_phyaddr_uv = info.dst_phyaddr_uv; //just init, no use in SE driver.
	}

	if(info.dst_fmt == KGAL_PIXEL_FORMAT_NV12)
	{
		dsurf.bpp 	= 16;
		dsurf.pitch = info.dst_w;
	}
	else if(info.dst_fmt == KGAL_PIXEL_FORMAT_RGB888)
	{
		dsurf.bpp 	= 24;
		dsurf.pitch = info.dst_w*3;
	}

	se_sta = rtd_inl(0xB801B6B8);
	status = KGAL_NV12_StretchBlit(&ssurf, &srect, &dsurf, &drect, &sflag, &sblend, info.src_phyaddr_uv, info.dst_phyaddr_uv);
	se_end = rtd_inl(0xB801B6B8);

	#if 0
	//rtd_pr_vpq_ai_info( "se proc time: %d\n", se_end-se_sta);// exe_time = diff/90 (ms)
	rtd_pr_vpq_ai_info( "[%s] SE info: %d %d 0x%x 0x%x %d pitch %d, %d %d 0x%x 0x%x %d. status: %d\n", __func__,
		info.src_w, info.src_h, info.src_phyaddr, info.src_phyaddr_uv, info.src_fmt, info.src_pitch_y,
		info.dst_w, info.dst_h, info.dst_phyaddr, info.dst_phyaddr_uv, info.dst_fmt, status);
	#endif
	return status;
#else
	//rtd_pr_vpq_ai_err("[%s] need enable CONFIG_RTK_KDRV_SE",__func__);
	return 0;
#endif
}
void scalerAI_cropBuffer(SE_NN_info *se_info)
{
	se_info->src_crop_range.top = ai_scene_ctrl.ai_scene_global.crop_top;
	se_info->src_crop_range.bot = ai_scene_ctrl.ai_scene_global.crop_bot;
	se_info->src_crop_range.left = ai_scene_ctrl.ai_scene_global.crop_left;
	se_info->src_crop_range.right = ai_scene_ctrl.ai_scene_global.crop_right;

	se_info->src_x = se_info->src_x + se_info->src_crop_range.right;
	se_info->src_y = se_info->src_y + se_info->src_crop_range.top;
	se_info->src_w = se_info->src_w - (se_info->src_crop_range.right + se_info->src_crop_range.left);
	se_info->src_h = se_info->src_h - (se_info->src_crop_range.top + se_info->src_crop_range.bot);
}
void scalerAI_updateBuffer(SE_NN_info info)
{
	VIP_NN_BUFFER_INDEX index;
	bool status = 0;
	SE_NN_info info_scene;

	// 1.update VIP_NN_BUFFER_416FIT buffer
	index=VIP_NN_BUFFER_416FIT;
	info.dst_w=416;
	info.dst_h=234;	
	scalerAI_get_NN_buffer(&(info.dst_phyaddr), &(info.dst_phyaddr_uv), index);
	if(info.dst_phyaddr && info.dst_phyaddr_uv)
	status = scalerAI_SE_stretch_Proc(info);

	if(status){
		scalerAI_set_NN_buffer(index);
	}else{
		rtd_pr_vpq_ai_emerg("update nn buffer %d fail\n", index);
	}
	
	// 2.update VIP_NN_BUFFER_224FULL buffer
	memcpy(&info_scene, &info, sizeof(SE_NN_info));
	index=VIP_NN_BUFFER_224FULL;
	info_scene.dst_w=224;
	info_scene.dst_h=224;
	info_scene.dst_phyaddr=0;
	info_scene.dst_phyaddr_uv=0;
	scalerAI_get_NN_buffer(&(info_scene.dst_phyaddr), &(info_scene.dst_phyaddr_uv), index);
	scalerAI_cropBuffer(&info_scene); // to avoid player UI affecting detection result. e.g. landscape becomes animation
	if(info_scene.dst_phyaddr && info_scene.dst_phyaddr_uv)
		status = scalerAI_SE_stretch_Proc(info_scene);

	if(status){
		scalerAI_set_NN_buffer(index);
	}else{
		rtd_pr_vpq_ai_emerg("update nn buffer %d fail\n", index);
	}

	// 3.update VIP_NN_BUFFER_320X240FULL
	index=VIP_NN_BUFFER_320X240FULL;
	info.dst_w=320;
	info.dst_h=240;
	info.dst_phyaddr=0;
	info.dst_phyaddr_uv=0;
	scalerAI_get_NN_buffer(&(info.dst_phyaddr), &(info.dst_phyaddr_uv), index);
	if(info.dst_phyaddr && info.dst_phyaddr_uv)
	status = scalerAI_SE_stretch_Proc(info);

	if(status){
		scalerAI_set_NN_buffer(index);
	}else{
		rtd_pr_vpq_ai_emerg("update nn buffer %d fail\n", index);
	}

	// 4.update VIP_NN_BUFFER_960X540
	index=VIP_NN_BUFFER_960X540;
	//block (0,0)~(240,135)
	info.src_w=240;
	info.src_h=240;
	info.dst_w=240;
	info.dst_h=240;
	info.dst_phyaddr=0;
	info.dst_phyaddr_uv=0;
	scalerAI_get_NN_buffer(&(info.dst_phyaddr), &(info.dst_phyaddr_uv), index);
	if(info.dst_phyaddr && info.dst_phyaddr_uv)
	status = scalerAI_SE_stretch_Proc(info);

	//block (240,135)~(480,270)
	info.src_x=240;
	info.src_y=135;
	info.dst_phyaddr=info.dst_phyaddr+240*135;
	status = scalerAI_SE_stretch_Proc(info);
	
	//block (480,270)~(720,405)
	info.src_x=480;
	info.src_y=270;
	info.dst_phyaddr=info.dst_phyaddr+240*135;
	status = scalerAI_SE_stretch_Proc(info);
	
	//block (720,405)~(960,540)
	info.src_x=720;
	info.src_y=405;
	info.dst_phyaddr=info.dst_phyaddr+240*135;
	status = scalerAI_SE_stretch_Proc(info);

	if(status){
		scalerAI_set_NN_buffer(index);
	}else{
		rtd_pr_vpq_ai_emerg("update nn buffer %d fail\n", index);
	}

}

//int tmp_scene_change_en = 0;
//extern CHIP_DCC_T tFreshContrast_coef;
void scalerAI_ai_pq_off(void)
{
	rtd_pr_vpq_ai_emerg("scalerAI_ai_pq_off \n");

	// reset ap nn_info in share memory
	fw_scalerip_reset_NN();

	fwif_color_Set_AI_Ctrl(0, 0, 0);

	// 0920 henry, force scene change for dynamic control to clean the face info
	//tmp_scene_change_en = ai_ctrl.ai_global3.scene_change_en;
	//ai_ctrl.ai_global3.scene_change_en = 0;
	scene_change_flag=1;
	
	drvif_color_AI_obj_icm_init(0);
	
	drvif_color_AI_cds_init(0);
	drvif_color_AI_obj_srp_init(0);
	drvif_color_AI_scene_srp_init(0,0);

	drvif_color_AI_obj_dcc_init(0);
	// dcc user curve off
	drivef_ai_tune_dcc_set(&AI_Tune_DCC_TBL[0], 0);
	
	//fwif_color_set_dcc_FreshContrast_tv006(&tFreshContrast_coef);
	Scaler_VIP_DCC_Color_Independent_reset();

	// reset
	reset_face_apply = 1;

	// dcc old skin
	//Scaler_Set_DCC_Color_Independent_Table(5);// table 5 is reg0_en=0.
	//ai_ctrl.ai_global2.dcc_old_skin_en = 0;
}
void scalerAI_ai_pq_on(unsigned char mode, unsigned char dcValue)
{
	rtd_pr_vpq_ai_emerg("scalerAI_ai_pq_on\n");

	// 0920 henry, restore scene change setting from VIP 1260 table
	//ai_ctrl.ai_global3.scene_change_en = tmp_scene_change_en;

	if(mode == AI_PQ_DEMO)
	{
		fwif_color_Set_AI_Ctrl(1, 1, 2);

		// icm tbl demo
		drivef_ai_tune_icm_set(&AI_Tune_ICM_TBL[1]);
		
		// dcc user curve on, tbl demo
		drivef_ai_tune_dcc_set(&AI_Tune_DCC_TBL[4], 1);
	}
	else
	{
		fwif_color_Set_AI_Ctrl(1, 1, 1);

		// icm tbl tv006
		drivef_ai_tune_icm_set(&AI_Tune_ICM_TBL[0]);
		
		// dcc user curve on, tbl tv006
		drivef_ai_tune_dcc_set(&AI_Tune_DCC_TBL[dcValue], 1);
	}
	 
	drvif_color_AI_obj_dcc_init(1);
	drvif_color_AI_obj_icm_init(1);

	drvif_color_AI_cds_init(1);
	drvif_color_AI_scene_srp_init(0,0);// dynamic control cm1&2

	if(mode == AI_PQ_FACE_OFF || mode == AI_PQ_ALL_OFF)
		drvif_color_AI_obj_srp_init(0);
	else
		drvif_color_AI_obj_srp_init(1);

}

// called from
// 1. Vdec with film task: ScalerVIP_SE_Proc(), scalerVIP.c
// 2: HDMI: vgip_isr(), rtk_vgip_isr.c -> se_tsk(), scaler_vpqdev.c
void scalerAI_preprocessing(void)
{
	_RPC_system_setting_info* RPC_SysInfo = NULL;
	unsigned char vdec_rdPtr = 0;

	unsigned int in_addr_y_tmp = 0, in_addr_uv_tmp = 0;
	unsigned int in_w_tmp = 0, in_h_tmp = 0, in_pitch_y_tmp = 0/*, in_pitch_c_tmp = 0*/;
    int freq = 0;
	SE_NN_info info;
	HAL_VO_IMAGE_T *voPhoto;
    static unsigned char frame_cnt = 3;
	static unsigned char photo_cnt = 0;

	//rtd_pr_vpq_ai_info("pre processing start \n");

	if(frame_cnt%SE_cnt == SE_cnt-1)
		SE_pre = 1;
	else
		SE_pre = 0;

    freq = Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ);
    //rtd_pr_vpq_ai_emerg("henry freq=%d\n", freq);
    /*if(0) {
		VIP_NN_CTRL *pNN;
		pNN = scalerAI_Access_NN_CTRL_STRUCT();
		rtd_pr_vpq_ai_info("start buffer_2_flag = %d\n", pNN->NN_flag_Addr[2].pVir_addr_align[0]);
	}*/
    //rtd_pr_vpq_ai_emerg(" ISR frame_cnt=%d\n", frame_cnt); // print 0
    frame_cnt = (frame_cnt+1)%SE_cnt;

	RPC_SysInfo = scaler_GetShare_Memory_RPC_system_setting_info_Struct();


	if(RPC_SysInfo == NULL)
	{
		rtd_pr_vpq_ai_emerg("RPC sys info NULL\n");
		return;
	}

	if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC)//(RPC_SysInfo->VIP_source == VIP_QUALITY_HDR_DTV_4k2kP_60 || RPC_SysInfo->VIP_source == VIP_QUALITY_DTV_4k2kP_60)
	{
		vdec_rdPtr = RPC_SysInfo->SCPU_ISRIINFO_TO_VCPU.rdPtr;
		in_addr_y_tmp = RPC_SysInfo->SCPU_ISRIINFO_TO_VCPU.pic[vdec_rdPtr].SeqBufAddr_Curr;
		in_addr_uv_tmp = RPC_SysInfo->SCPU_ISRIINFO_TO_VCPU.pic[vdec_rdPtr].SeqBufAddr_Curr_UV;

		in_w_tmp = RPC_SysInfo->SCPU_ISRIINFO_TO_VCPU.pic[vdec_rdPtr].Width;
		in_h_tmp = RPC_SysInfo->SCPU_ISRIINFO_TO_VCPU.pic[vdec_rdPtr].Height;
		in_pitch_y_tmp = RPC_SysInfo->SCPU_ISRIINFO_TO_VCPU.pic[vdec_rdPtr].pitch_y;
		//in_pitch_c_tmp = RPC_SysInfo->SCPU_ISRIINFO_TO_VCPU.pic[vdec_rdPtr].pitch_c;


		info.src_x = 0;
		info.src_y = 0;
		info.src_w = Scaler_ChangeUINT32Endian(in_w_tmp);
		info.src_h = Scaler_ChangeUINT32Endian(in_h_tmp);
		info.src_pitch_y = Scaler_ChangeUINT32Endian(in_pitch_y_tmp);
		//info.src_pitch_c = Scaler_ChangeUINT32Endian(in_pitch_c_tmp);
		info.src_phyaddr = Scaler_ChangeUINT32Endian(in_addr_y_tmp);
		info.src_phyaddr_uv = Scaler_ChangeUINT32Endian(in_addr_uv_tmp);
		info.src_fmt = KGAL_PIXEL_FORMAT_NV12;

		if(info.src_phyaddr==0 || info.src_phyaddr_uv==0)
		{
			rtd_pr_vpq_ai_emerg("aipq vdec buffer error\n");
		}

		info.dst_x = 0;
		info.dst_y = 0;
		info.dst_w = 416;
		info.dst_h = 234;

		info.dst_phyaddr = 0;
		info.dst_phyaddr_uv = 0;
		info.dst_fmt = KGAL_PIXEL_FORMAT_NV12;

		//rtd_pr_vpq_ai_info( "[%s] src: %d vdec:%d %d addr: 0x%x 0x%x\n", __func__, RPC_SysInfo->VIP_source, vedc_status, vdec_rdPtr, info.src_phyaddr, info.src_phyaddr_uv);
		
		scalerAI_updateBuffer(info);
			
		#if ENABLE_DUMP_BUFFER // lesley debug dump
		dumpCheck();
		#endif		
	}
	else if(fwif_color_get_force_run_i3ddma_enable(SLR_MAIN_DISPLAY))//(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
	{
		//if(0) {
		//	int vgip_hdmi = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
		//	rtd_pr_vpq_ai_info("90k_counter before SE get hdmi buffer = %d\n", vgip_hdmi);
		//}
		#ifdef CONFIG_ENABLE_HDMI_NN
		info.src_x = 0;
		info.src_y = 0;
		//info.src_w = 960;
		//info.src_h = 540;
		h3ddma_get_NN_output_size(&(info.src_w), &(info.src_h));
		
		//rtd_pr_vpq_ai_emerg("[%s] i3ddma: w:%d, h:%d\n", __FUNCTION__, info.src_w, info.src_h);
		info.src_fmt = KGAL_PIXEL_FORMAT_NV12;
		info.src_pitch_y = info.src_w;
		//info.src_pitch_c = info.src_w;

		if(info.src_w == 0 || info.src_h == 0)
		{
			rtd_pr_vpq_ai_emerg("[Error] aipq i3ddma size 0\n");
			return;
		}

		if(h3ddma_get_NN_read_buffer(&(info.src_phyaddr), &(info.src_phyaddr_uv)) < 0)
		{
			rtd_pr_vpq_ai_emerg("[Error] aipq i3ddma for NN drop!!!\n");

			return;
		}
		#endif
		
#if 0 // lesley debug print
		hdmiPAddrY = (info.src_phyaddr);
		hdmiPAddrC = (info.src_phyaddr_uv);
#endif

		info.dst_x = 0;
		info.dst_y = 0;
		info.dst_w = 416;
		info.dst_h = 234;

		info.dst_phyaddr = 0;
		info.dst_phyaddr_uv = 0;
		info.dst_fmt = KGAL_PIXEL_FORMAT_NV12;
		
		scalerAI_updateBuffer(info);
		
		#if ENABLE_DUMP_BUFFER // lesley debug dump
		dumpCheck();
		#endif
	}
	else if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG)
	{
		#if ENABLE_DUMP_BUFFER // lesley debug print
		unsigned int voPhotoPAddr;
		#endif

		info.src_x = 0;
		info.src_y = 0;
		info.src_w = 3840;
		info.src_h = 2160;
		info.src_pitch_y = info.src_w;
		//info.src_pitch_c = info.src_w;
		voPhoto = VO_GetPictureInfo();
		info.src_phyaddr = (unsigned int)voPhoto->buf;

		if((vo_photo_buf_pre == info.src_phyaddr) && photo_cnt >= 10)
		{			
			return;
		}
		else if(vo_photo_buf_pre == info.src_phyaddr)
		{			
			photo_cnt++;
		}
		else
		{
			vo_photo_buf_pre = info.src_phyaddr;
			photo_cnt = 0;
		}

	
		/*
		vo photo buffer is yuvyuvyuv...
		but KGAL_PIXEL_FORMAT_YUV444 is 3 plane, yyy...uuu...vvv...
		so use KGAL_PIXEL_FORMAT_RGB888 to achieve 1 plane order.
		*/
		info.src_fmt = KGAL_PIXEL_FORMAT_RGB888;//KGAL_PIXEL_FORMAT_YUV444;
		#if ENABLE_DUMP_BUFFER // lesley debug print
		voPhotoPAddr = (info.src_phyaddr);
		rtd_pr_vpq_ai_emerg("vo addr=0x%x\n", voPhotoPAddr);
		#endif

		info.dst_x = 0;
		info.dst_y = 0;
		info.dst_w = 480;
		info.dst_h = 270;
		info.dst_phyaddr = 0;
		info.dst_phyaddr_uv = 0;
		info.dst_fmt = KGAL_PIXEL_FORMAT_NV12;
		
		scalerAI_get_NN_buffer(&(info.dst_phyaddr), &(info.dst_phyaddr_uv), VIP_NN_BUFFER_NOT_USED);

		if(info.dst_phyaddr && info.dst_phyaddr_uv)
		{
			scalerAI_SE_stretch_Proc(info);
		}

		info.src_x = 0;
		info.src_y = 0;
		info.src_w = 480;
		info.src_h = 270;
		info.src_pitch_y = info.src_w;
		info.src_phyaddr = info.dst_phyaddr;
		info.src_phyaddr_uv = info.dst_phyaddr_uv;
		info.src_fmt = KGAL_PIXEL_FORMAT_NV12;
		#if 0 // lesley debug print
		voPhotoPAddr = (info.src_phyaddr);
		#endif

		info.dst_x = 0;
		info.dst_y = 0;
		info.dst_w = 416;
		info.dst_h = (info.src_w)?(416*info.src_h/info.src_w):(0);
		info.dst_phyaddr = 0;
		info.dst_phyaddr_uv = 0;
		info.dst_fmt = KGAL_PIXEL_FORMAT_NV12;

		scalerAI_updateBuffer(info);
		
		#if ENABLE_DUMP_BUFFER // lesley debug dump
		dumpCheck();
		#endif		
	}
	else
	{
		//rtd_pr_vpq_ai_info( "[%s] (%d)\n", __func__, __LINE__);
	}
}

void scalerAI_get_NN_buffer(unsigned int *phyaddr_y, unsigned int *phyaddr_c, VIP_NN_BUFFER_INDEX index)
{
	//unsigned char i;
	unsigned char bufIdx=0;
	VIP_NN_CTRL *pNN;
	unsigned char flag;
	unsigned int out_y_size;

	switch(index){
		case VIP_NN_BUFFER_416FIT:
			out_y_size=0x2A400;//416*416=0x2A400
			break;
		case VIP_NN_BUFFER_224FULL:
			out_y_size=0xc400;//224*224=0xc400
			break;	
		case VIP_NN_BUFFER_320X240FULL:
			out_y_size=0x12c00;//320*240=0x12c00
			break;	
		case VIP_NN_BUFFER_960X540:
			out_y_size=0x7E900;//960*540=0x7E900
			break;	
		default:
			out_y_size=0x2A400;//416*416=0x2A400
			break;	
	}

	bufIdx = scalerAI_get_nn_buf_idx();
	pNN = scalerAI_Access_NN_CTRL_STRUCT();
	
	//rtd_pr_vpq_ai_info( "[%s] check flag %d = %d\n", __func__, bufIdx, pNN->NN_flag_Addr[bufIdx].pVir_addr_align[0]);
#if 1
	if(index<VIP_NN_BUFFER_MAX){
		bufIdx = index;
		*phyaddr_y = pNN->NN_data_Addr[bufIdx].phy_addr_align;
		*phyaddr_c = *phyaddr_y + out_y_size;
		flag = pNN->NN_flag_Addr[bufIdx].pVir_addr_align[0];
		
		// 0:driver is going to update buffer data
		// 1:buffer data is ready for ap copy 
		pNN->NN_flag_Addr[bufIdx].pVir_addr_align[0]=pNN->NN_flag_Addr[bufIdx].pVir_addr_align[0]&(~_BIT0);
		
		//rtd_pr_vpq_ai_emerg("get_NN_buffer index=%d\n",bufIdx);		
	}

	return;
#else
	if(isTmp)
	{
		// Y: 480*270=0x1FA40 < 0x2A400,
		// C: 480*270/2=0xFD20, 0x2A400+0xFD20=0x3A120=232k < 280k VIP_NN_DATA_BUFFER_SIZE

		bufIdx = 0;
		*phyaddr_y = pNN->NN_data_Addr[bufIdx].phy_addr_align;
		*phyaddr_c = *phyaddr_y + out_y_size;
		return 0;
	}
	else
	//for(i=0; i<3; i++)
	{
		bufIdx = 2;
		flag = pNN->NN_flag_Addr[bufIdx].pVir_addr_align[0];

#if ENABLE_AP_POSTPROCESS_THREAD
		if((flag&_BIT0) == 0)
#else
		if(flag == 0)
#endif
		{
			*phyaddr_y = pNN->NN_data_Addr[bufIdx].phy_addr_align;
			*phyaddr_c = *phyaddr_y + out_y_size;

			return bufIdx;//break;
		}
		//bufIdx=(bufIdx+1)%3;
	}
	return scalerAI_get_nn_buf_idx();
#endif	
}

void scalerAI_set_NN_buffer(VIP_NN_BUFFER_INDEX bufIdx)
{
	//unsigned char bufIdx = 0;
	VIP_NN_CTRL *pNN;
	
#if 1
	pNN = scalerAI_Access_NN_CTRL_STRUCT();
	pNN->NN_flag_Addr[bufIdx].pVir_addr_align[0] = pNN->NN_flag_Addr[bufIdx].pVir_addr_align[0]|_BIT0;
	tic_start = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
	//rtd_pr_vpq_ai_emerg("set_NN_buffer %d\n", pNN->NN_flag_Addr[bufIdx].pVir_addr_align[0]);	

#else
	bufIdx = scalerAI_get_nn_buf_idx();
	bufIdx = 2;

	pNN = scalerAI_Access_NN_CTRL_STRUCT();

#if ENABLE_AP_POSTPROCESS_THREAD
	if((pNN->NN_flag_Addr[bufIdx].pVir_addr_align[0]&_BIT0) == 0)
		pNN->NN_flag_Addr[bufIdx].pVir_addr_align[0] = pNN->NN_flag_Addr[bufIdx].pVir_addr_align[0]|_BIT0;
#else
	if(pNN->NN_flag_Addr[bufIdx].pVir_addr_align[0] == 0)
		pNN->NN_flag_Addr[bufIdx].pVir_addr_align[0] = 1;
#endif
	pNN->NN_indx_Addr.pVir_addr_align[0] = bufIdx;
	//rtd_pr_vpq_ai_info( "henry buffer idx=%d, flag=%d\n", pNN->NN_indx_Addr.pVir_addr_align[0], pNN->NN_flag_Addr[bufIdx].pVir_addr_align[0]);

	//rtd_pr_vpq_ai_info("set_nn_buffer buffer_2_flag = %d\n", pNN->NN_flag_Addr[2].pVir_addr_align[0]);
	tic_start = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
	//rtd_pr_vpq_ai_info("1. SE buffer = %d\n", pNN->NN_data_Addr[2].pVir_addr_align[416*208/4] & 0xff);


	//bufIdx = (bufIdx+1)%3;
	//scalerAI_set_nn_buf_idx(bufIdx);

#endif
}

void scalerAI_scene_info_get(int *info, unsigned char *NN_flag)
{

	if(ai_ctrl.ai_info_manul_0.info_manual_en==0)
	{
		int idx = 0;
		VIP_NN_CTRL *pNN;
		int tic_end = 0, tic_total = 0;

		pNN = scalerAI_Access_NN_CTRL_STRUCT();
		idx = VIP_NN_BUFFER_224FULL;

		if(!(pNN->NN_flag_Addr[idx].pVir_addr_align[0]&_BIT1))
		{
			return;
		}

		*NN_flag = 1;

		tic_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
		//rtd_pr_vpq_ai_info("buffer_2_flag = %d, time stamp = %d (1/90 ms)\n", pNN->NN_flag_Addr[2].pVir_addr_align[0], tic_end);
		tic_total = (tic_end - tic_start)/90;
		//rtd_pr_vpq_ai_info("buffer_2_flag, total time = %d ms\n", tic_total);

		memcpy((void *)info,(void *)pNN->NN_info_Addr[idx].pVir_addr_align,sizeof(int)*AI_SCENE_TYPE_NUM);

		//set flag after info[i] have updated
		//pNN->NN_flag_Addr[2].pVir_addr_align[0] bit1
		// 0: scalerAI_postprocessing had updated face info and wait for next info
		// 1: scalerAI_postprocessing is updating face info		
		pNN->NN_flag_Addr[idx].pVir_addr_align[0] = pNN->NN_flag_Addr[idx].pVir_addr_align[0]&(~_BIT1);



	}
	else if(ai_ctrl.ai_info_manul_0.info_manual_en==1)
	{
		//todo
	}
}

void scalerAI_obj_info_get(AIInfo *info, unsigned char *NN_flag)
{

	// scaler_AI_Ctrl_Get(&ai_ctrl_dyn);

	if(ai_ctrl.ai_info_manul_0.info_manual_en==0)
	{
		int i, idx = 0, offset = 14;
		VIP_NN_CTRL *pNN;
		int tic_end = 0, tic_total = 0;

		pNN = scalerAI_Access_NN_CTRL_STRUCT();
		idx = VIP_NN_BUFFER_416FIT;

		/*if(0) {
			int tic = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			rtd_pr_vpq_ai_info("90k_counter before compare flag = %d\n", tic);
		}*/

		//rtd_pr_vpq_ai_info("SE counter - 1 = %d\n", SE_pre);
		if(!(pNN->NN_flag_Addr[idx].pVir_addr_align[0]&_BIT1))
		{
			//memcpy(info,face_info,6*sizeof(AIInfo));
			return;
		}

		*NN_flag = 1;

		tic_end = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
		//rtd_pr_vpq_ai_info("buffer_2_flag = %d, time stamp = %d (1/90 ms)\n", pNN->NN_flag_Addr[2].pVir_addr_align[0], tic_end);
		tic_total = (tic_end - tic_start)/90;
		//rtd_pr_vpq_ai_info("buffer_2_flag, total time = %d ms\n", tic_total);

		// for debug
		/*{
		//int UZDaccess = 0;
		scaledown_accessdata_ctrl_uzd_RBUS scaledown_accessdata_ctrl_uzd_reg;

		//rtd_pr_vpq_ai_info("2. nn info = %d\n", pNN->NN_data_Addr[0].pVir_addr_align[0]);

		scaledown_accessdata_ctrl_uzd_reg.regValue = IoReg_Read32(SCALEDOWN_AccessData_CTRL_UZD_reg);
		scaledown_accessdata_ctrl_uzd_reg.read_en = 1;
		IoReg_Write32(SCALEDOWN_AccessData_CTRL_UZD_reg, scaledown_accessdata_ctrl_uzd_reg.regValue);
		IoReg_Write32(SCALEDOWN_AccessData_PosCtrl_UZD_reg, 0x00640064);
		UZDaccess = IoReg_Read32(SCALEDOWN_ReadData_DATA_Y1_UZD_reg) & 0x3ff;
		//rtd_pr_vpq_ai_info("3. uzd access = %d\n", UZDaccess>>2);
		}*/

		for(i = 0; i < 6; i++)
		{
			info[i].pv8 = pNN->NN_info_Addr[idx].pVir_addr_align[offset*i+0];
			info[i].cx12 = pNN->NN_info_Addr[idx].pVir_addr_align[offset*i+1];
			info[i].cy12 = pNN->NN_info_Addr[idx].pVir_addr_align[offset*i+2];
			info[i].w12 = pNN->NN_info_Addr[idx].pVir_addr_align[offset*i+3];
			info[i].h12 = pNN->NN_info_Addr[idx].pVir_addr_align[offset*i+4];
			info[i].range12 = pNN->NN_info_Addr[idx].pVir_addr_align[offset*i+5];
			info[i].cb_med12 = pNN->NN_info_Addr[idx].pVir_addr_align[offset*i+6];
			info[i].cr_med12 = pNN->NN_info_Addr[idx].pVir_addr_align[offset*i+7];
			info[i].hue_med12 = pNN->NN_info_Addr[idx].pVir_addr_align[offset*i+10];
			info[i].sat_med12 = pNN->NN_info_Addr[idx].pVir_addr_align[offset*i+11];
			info[i].val_med12 = pNN->NN_info_Addr[idx].pVir_addr_align[offset*i+12];
			info[i].y_med12 = pNN->NN_info_Addr[idx].pVir_addr_align[offset*i+13];

			/*rtd_pr_vpq_ai_info("[getinfo]  face[%d]\n", i);
			rtd_pr_vpq_ai_info("[getinfo]	pv8 = %d\n", info[i].pv8);
			rtd_pr_vpq_ai_info("[getinfo]	cx12, cy12 = %d, %d\n", info[i].cx12, info[i].cy12);
			rtd_pr_vpq_ai_info("[getinfo]	w12, h12 = %d, %d\n", info[i].w12, info[i].h12);
			rtd_pr_vpq_ai_info("[getinfo]	range12 = %d\n", info[i].range12);
			rtd_pr_vpq_ai_info("[getinfo]	y, u, v = %d, %d, %d\n", info[i].y_med12, info[i].cb_med12, info[i].cr_med12);*/
		}

		//set flag after info[i] have updated
		//pNN->NN_flag_Addr[2].pVir_addr_align[0] bit1
		// 0: scalerAI_postprocessing had updated face info and wait for next info
		// 1: scalerAI_postprocessing is updating face info		
		pNN->NN_flag_Addr[idx].pVir_addr_align[0] = pNN->NN_flag_Addr[idx].pVir_addr_align[0]&(~_BIT1);



	}
	// chen 0502
	else if(ai_ctrl.ai_info_manul_0.info_manual_en==1)
	{
		scene_change=ai_ctrl.ai_info_manul_0.scene_change;

		// face0
		info[0].pv8=ai_ctrl.ai_info_manul_0.pv8;
		info[0].cx12=ai_ctrl.ai_info_manul_0.cx12;
		info[0].cy12=ai_ctrl.ai_info_manul_0.cy12;
		info[0].w12=ai_ctrl.ai_info_manul_0.w12;
		info[0].h12=ai_ctrl.ai_info_manul_0.h12;
		info[0].range12=ai_ctrl.ai_info_manul_0.range12;
		info[0].cb_med12=ai_ctrl.ai_info_manul_0.cb_med12;
		info[0].cr_med12=ai_ctrl.ai_info_manul_0.cr_med12;
		info[0].hue_med12=ai_ctrl.ai_info_manul_0.hue_med12;
		info[0].sat_med12=ai_ctrl.ai_info_manul_0.sat_med12;
		info[0].val_med12=ai_ctrl.ai_info_manul_0.val_med12;
		// face1
		info[1].pv8=ai_ctrl.ai_info_manul_1.pv8;
		info[1].cx12=ai_ctrl.ai_info_manul_1.cx12;
		info[1].cy12=ai_ctrl.ai_info_manul_1.cy12;
		info[1].w12=ai_ctrl.ai_info_manul_1.w12;
		info[1].h12=ai_ctrl.ai_info_manul_1.h12;
		info[1].range12=ai_ctrl.ai_info_manul_1.range12;
		info[1].cb_med12=ai_ctrl.ai_info_manul_1.cb_med12;
		info[1].cr_med12=ai_ctrl.ai_info_manul_1.cr_med12;
		info[1].hue_med12=ai_ctrl.ai_info_manul_1.hue_med12;
		info[1].sat_med12=ai_ctrl.ai_info_manul_1.sat_med12;
		info[1].val_med12=ai_ctrl.ai_info_manul_1.val_med12;
		// face2
		info[2].pv8=ai_ctrl.ai_info_manul_2.pv8;
		info[2].cx12=ai_ctrl.ai_info_manul_2.cx12;
		info[2].cy12=ai_ctrl.ai_info_manul_2.cy12;
		info[2].w12=ai_ctrl.ai_info_manul_2.w12;
		info[2].h12=ai_ctrl.ai_info_manul_2.h12;
		info[2].range12=ai_ctrl.ai_info_manul_2.range12;
		info[2].cb_med12=ai_ctrl.ai_info_manul_2.cb_med12;
		info[2].cr_med12=ai_ctrl.ai_info_manul_2.cr_med12;
		info[2].hue_med12=ai_ctrl.ai_info_manul_2.hue_med12;
		info[2].sat_med12=ai_ctrl.ai_info_manul_2.sat_med12;
		info[2].val_med12=ai_ctrl.ai_info_manul_2.val_med12;
		// face3
		info[3].pv8=ai_ctrl.ai_info_manul_3.pv8;
		info[3].cx12=ai_ctrl.ai_info_manul_3.cx12;
		info[3].cy12=ai_ctrl.ai_info_manul_3.cy12;
		info[3].w12=ai_ctrl.ai_info_manul_3.w12;
		info[3].h12=ai_ctrl.ai_info_manul_3.h12;
		info[3].range12=ai_ctrl.ai_info_manul_3.range12;
		info[3].cb_med12=ai_ctrl.ai_info_manul_3.cb_med12;
		info[3].cr_med12=ai_ctrl.ai_info_manul_3.cr_med12;
		info[3].hue_med12=ai_ctrl.ai_info_manul_3.hue_med12;
		info[3].sat_med12=ai_ctrl.ai_info_manul_3.sat_med12;
		info[3].val_med12=ai_ctrl.ai_info_manul_3.val_med12;
		// face4
		info[4].pv8=ai_ctrl.ai_info_manul_4.pv8;
		info[4].cx12=ai_ctrl.ai_info_manul_4.cx12;
		info[4].cy12=ai_ctrl.ai_info_manul_4.cy12;
		info[4].w12=ai_ctrl.ai_info_manul_4.w12;
		info[4].h12=ai_ctrl.ai_info_manul_4.h12;
		info[4].range12=ai_ctrl.ai_info_manul_4.range12;
		info[4].cb_med12=ai_ctrl.ai_info_manul_4.cb_med12;
		info[4].cr_med12=ai_ctrl.ai_info_manul_4.cr_med12;
		info[4].hue_med12=ai_ctrl.ai_info_manul_4.hue_med12;
		info[4].sat_med12=ai_ctrl.ai_info_manul_4.sat_med12;
		info[4].val_med12=ai_ctrl.ai_info_manul_4.val_med12;
		// face5
		info[5].pv8=ai_ctrl.ai_info_manul_5.pv8;
		info[5].cx12=ai_ctrl.ai_info_manul_5.cx12;
		info[5].cy12=ai_ctrl.ai_info_manul_5.cy12;
		info[5].w12=ai_ctrl.ai_info_manul_5.w12;
		info[5].h12=ai_ctrl.ai_info_manul_5.h12;
		info[5].range12=ai_ctrl.ai_info_manul_5.range12;
		info[5].cb_med12=ai_ctrl.ai_info_manul_5.cb_med12;
		info[5].cr_med12=ai_ctrl.ai_info_manul_5.cr_med12;
		info[5].hue_med12=ai_ctrl.ai_info_manul_5.hue_med12;
		info[5].sat_med12=ai_ctrl.ai_info_manul_5.sat_med12;
		info[5].val_med12=ai_ctrl.ai_info_manul_5.val_med12;

	}// end chen 0502
}

// called from vgip_isr after AI_scene_dynamic_control(int *info)
void scalerAI_scene_PQ_reset(void)
{
	AI_scene_sharp_reset();
	AI_scene_color_reset();
	AI_scene_TNR_reset();
	AI_scene_LC_reset();
	AI_scene_NR_reset();

}

// called from vgip_isr after AI_scene_dynamic_control(int *info)
void scalerAI_scene_PQ_set(void)
{
	AI_scene_sharp_set();
	AI_scene_color_set();
	AI_scene_TNR_set();
	AI_scene_LC_set();
	AI_scene_NR_set();
	AI_scene_DCC_set();
	AI_scene_ICM_set();
}
// called from D-domain ISR b80280d0[0]
void scalerAI_face_PQ_set(void)
{

	//AIInfo info[6] = {0};
	//scalerAI_obj_info_get(info);

	// for debug
	/*{
	int ICMaccess = 0;
	int ODr=0,ODg=0, ODb=0, ODy=0;
	int c11=66, c12=129, c13=25,c1=128, d1=16;

	color_icm_dm_icm_accessdata_ctrl_RBUS color_icm_dm_icm_accessdata_ctrl_reg;
	od_accessdata_ctrl_pc_RBUS od_accessdata_ctrl_pc_reg;

	color_icm_dm_icm_accessdata_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_AccessData_CTRL_reg);
	color_icm_dm_icm_accessdata_ctrl_reg.accessdata_read_en = 1;
	IoReg_Write32(COLOR_ICM_DM_ICM_AccessData_CTRL_reg, color_icm_dm_icm_accessdata_ctrl_reg.regValue);
	IoReg_Write32(COLOR_ICM_DM_ICM_AccessData_PosCtrl_reg, 0x04380780); // x=1920, y=1080
	ICMaccess = IoReg_Read32(COLOR_ICM_DM_ICM_ReadData_DATA_I1_reg) & 0xffff;
	//rtd_pr_vpq_ai_info("4. D domain isr apply PQ\n");
	//rtd_pr_vpq_ai_info("   ICM access = %d\n", ICMaccess >> 4);
	od_accessdata_ctrl_pc_reg.regValue = IoReg_Read32(OD_AccessData_CTRL_PC_reg);
	od_accessdata_ctrl_pc_reg.access_read_en= 1;
	IoReg_Write32(OD_AccessData_CTRL_PC_reg, od_accessdata_ctrl_pc_reg.regValue);
	IoReg_Write32(OD_AccessData_PosCtrl_PC_reg, 0x04380780); // x=1920, y=1080
	ODr = (IoReg_Read32(OD_ReadData_DATA_Channel1_1_PC_reg) & 0xfff)>>4;
	ODg = (IoReg_Read32(OD_ReadData_DATA_Channel2_1_PC_reg) & 0xfff)>>4;
	ODb = (IoReg_Read32(OD_ReadData_DATA_Channel3_1_PC_reg) & 0xfff)>>4;
	ODy = ((c11*ODr+c12*ODg+c13*ODb+c1)>>8)+d1;
	//rtd_pr_vpq_ai_info("   OD access = %d\n", ODy);
	}*/

	// 1025
	int delay = 0, tmp = 0;

	if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC)
		delay = ai_ctrl.ai_global3.apply_delay;

	tmp = (apply_buf_num + (buf_idx_w-1) - delay);
	
	if(tmp<0)
		tmp = 0;
	
	buf_idx_r = tmp%(apply_buf_num);

	if(reset_face_apply)
	{
		memset(&face_icm_apply, 0, sizeof(face_icm_apply));
		memset(&face_dcc_apply, 0, sizeof(face_dcc_apply));
		memset(&face_sharp_apply, 0, sizeof(face_sharp_apply));
		memset(&old_dcc_apply, 0, sizeof(old_dcc_apply));

		reset_face_apply = 0;
	}

	// chen 0429
	if(ai_ctrl.ai_global.AI_icm_en==1)
		drvif_color_AI_obj_icm_set(face_icm_apply[buf_idx_r]);
	if(ai_ctrl.ai_global.AI_dcc_en==1) // lesley 1017, disable AI_dcc_en since the applys are for AI dcc.
		drvif_color_AI_obj_dcc_set(face_dcc_apply[buf_idx_r]);
	if(ai_ctrl.ai_global.AI_sharp_en==1)
	// chen 0527
		drvif_color_AI_obj_sharp_set(face_sharp_apply[buf_idx_r]);
	//end chen 0527

	// lesley 1016
	if(ai_ctrl.ai_global2.dcc_old_skin_en==1)
		drvif_color_old_skin_dcc_set(old_dcc_apply[buf_idx_r]);
	// end lesley 1016
	
	// lesley 0808
	drvif_color_AI_Ctrl_shp();
	// end lesley 0808
}

static void scalerAI_scenepostprocessing(void)
{
	//int i;
	static int info[AI_SCENE_TYPE_NUM];

	_RPC_clues* RPC_SmartPic_clue=NULL;
	//_system_setting_info *system_info_struct = NULL;
	unsigned char NN_flag = 0;

	static unsigned char scene_PQ_reset_done = 1;
	
	RPC_SmartPic_clue = scaler_GetShare_Memory_RPC_SmartPic_Clue_Struct();
	//system_info_struct = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	
	if(RPC_SmartPic_clue == NULL){
		rtd_pr_vpq_ai_info("[scalerAI_postprocessing]	RPC_SmartPic_clue null\n");
		return;
	}
	
	scalerAI_scene_info_get(info, &NN_flag);
	
	#if 0 //debug
	rtd_pr_vpq_ai_emerg("scene type info\n");
	for(i=0;i<AI_SCENE_TYPE_NUM;i++){
		rtd_pr_vpq_ai_emerg("type %d = %d\n",i,info[i]);
	}
	#endif
	
	if(ai_scene_ctrl.ai_scene_global.scene_en)
	{
		AI_scene_dynamic_control(info, NN_flag);
		scalerAI_scene_PQ_set();
		scene_PQ_reset_done = 0;
	}
	else if(!scene_PQ_reset_done)
	{
		scalerAI_scene_PQ_reset();
		scene_PQ_reset_done = 1;	
	}
	
}	

static void scalerAI_facepostprocessing(void)
{
	static AIInfo info[6] = {0};
	_RPC_clues* RPC_SmartPic_clue=NULL;
	unsigned char NN_flag = 0;
	
	//rtd_pr_vpq_ai_info("post processing start \n");
	//if(0)
	//{
	//	int tic = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
	//	rtd_pr_vpq_ai_info("90k_counter in scalerAI_postprocessing = %d\n", tic);
	//}
	
	RPC_SmartPic_clue = scaler_GetShare_Memory_RPC_SmartPic_Clue_Struct();
	
	if(RPC_SmartPic_clue == NULL){
		rtd_pr_vpq_ai_info("[%s] RPC_SmartPic_clue null\n", __FUNCTION__);
		return;
	}
	
	scalerAI_obj_info_get(info, &NN_flag);
	
	/* processing here */
	// chen 0417
	//if(RPC_SmartPic_clue->SceneChange) rtd_pr_vpq_ai_info("[henryscene] sceneChange\n");
//	AI_face_dynamic_control(info, RPC_SmartPic_clue->SceneChange);
	// chen 0703
//	AI_face_dynamic_control(info, RPC_SmartPic_clue->SceneChange, NN_flag_en);
	
	//rtd_pr_vpq_ai_info("NN_flag=%d\n", NN_flag);

	AI_face_dynamic_control(info, RPC_SmartPic_clue->SceneChange, NN_flag);
	
	// end chen 0417
}

// called from VGIP ISR b8022210[25:24]
void demo_setting(void)
{

#if 0 // lesley TBD
	mpegnr_ich1_mpegnr2_RBUS	mpegnr_ich1_mpegnr2_reg	;
	mpegnr_ich1_mpegnr2_reg.regValue=IoReg_Read32(	MPEGNR_ICH1_MPEGNR2_reg	);
	mpegnr_ich1_mpegnr2_reg.cp_mpegresultweight_x = 0;
	IoReg_Write32(	MPEGNR_ICH1_MPEGNR2_reg	,	mpegnr_ich1_mpegnr2_reg	.regValue);
#endif	
	demo_20CES_tv030();
}

// call from vgip_isr
void scalerAI_postprocessing(void)
{
	AI_MC_VPQ_INDEX index;
	
	/* for demo toggle */
	{ 			
		od_od_ctrl_RBUS od_ctrl_reg;			
		od_ctrl_reg.regValue = rtd_inl(OD_OD_CTRL_reg);	
		
		ai_scene_ctrl.ai_scene_global.scene_en = (od_ctrl_reg.dummy1802ca00_31_7>>21 & 1);//bit 28
		ai_scene_rtk_mode = (od_ctrl_reg.dummy1802ca00_31_7>>20 & 1);//bit 27
		ai_face_rtk_mode = (od_ctrl_reg.dummy1802ca00_31_7>>19 & 1);//bit 26

		// move to ddomain_isr
		/*if((od_ctrl_reg.dummy1802ca00_31_7>>19 & 1) == 1) //bit 26
		{
			drvif_color_AI_ICM_mode(1);
			drvif_color_AI_obj_cds_set(1);
		}
		else
		{
			drvif_color_AI_ICM_mode(0);
			drvif_color_AI_obj_cds_set(0);
		}*/		
	}

	demo_setting();

#if 0 // control by ap
	if(rtk_ai_mc_getCurModVpqCtrl(&index)==FALSE){
		//rtd_pr_vpq_ai_emerg("getCurModVpqCtrl fail\n");
		return;
	}
	
	switch(index){
	case AI_MC_VPQ_FACE:
		scalerAI_facepostprocessing();
		break;
	case AI_MC_VPQ_SCENE:
		scalerAI_scenepostprocessing();
		break;
	default:
		break;
	}
#else // do each isr

	scene_update = 0;
	if(rtk_ai_mc_getCurModVpqCtrl(&index)==TRUE){
		scene_update = 1;
	}
	
	scalerAI_facepostprocessing();
	scalerAI_scenepostprocessing();
#endif

	return;
}
void scalerAI_execute_NN(void)
{
#if 0 // josh 1018
	int ret = -1;
	char path[] = "/bin/sh";
    char script[] = "/home/runsystemctl.sh";
	char *argv[] = {path, script," &", NULL};
	char *envp[] = {
        "HOME=/",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };
	static int isDone=0;


	if(isDone){
		rtd_pr_vpq_ai_emerg("exe_AI already run\n");
		return;
	}

	rtd_pr_vpq_ai_emerg("exe_AI start\n");
	ret = call_usermodehelper(path, argv, envp, UMH_WAIT_EXEC );
	if(ret != 0)
		rtd_pr_vpq_ai_emerg("exe_AI fail ret=%d\n", ret);
	else{
		rtd_pr_vpq_ai_emerg("exe_AI success ret=%d\n", ret);
		isDone=1;
	}

#else
	int ret = -1;
	char path[] = "system/bin/sh";
    char script[] = "/data/mac7p_NNIP5_1231/face_scene.sh";//// mac7p AI TBD
	char *argv[] = {path, script, " &", NULL};
	char *envp[] = {
        "HOME=/",
        "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };

	rtd_pr_vpq_ai_info("[henry] exe_AI start\n");
	ret = call_usermodehelper(path, argv, envp, UMH_WAIT_EXEC );
	if(ret != 0)
		rtd_pr_vpq_ai_info("[henry] exe_AI fail ret=%d\n", ret);
	else
		rtd_pr_vpq_ai_info("[henry] exe_AI success ret=%d\n", ret);
#endif
}

extern unsigned short hue_hist_ratio[COLOR_HUE_HISTOGRAM_LEVEL];
unsigned short hue_hist_ratio_pre[COLOR_HUE_HISTOGRAM_LEVEL] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned short get_hue_hist_ratio(void)
{
	int i;
	unsigned short hue_ratio;

	hue_ratio = scalerVIP_ratio_inner_product(&hue_hist_ratio_pre[0], &hue_hist_ratio[0], COLOR_HUE_HISTOGRAM_LEVEL);

	for(i=0; i<COLOR_HUE_HISTOGRAM_LEVEL; i++)
	{
		hue_hist_ratio_pre[i] = hue_hist_ratio[i];
	}


	return hue_ratio;
}
// For scene_detection
// target value
// [mode][][]: normal, dark, sports, beautyscenary, animation, movie
// [mode][IP0: sharpnss][]: [][][feature: edge gainx5, vedge gainx5];
// [mode][IP1: TNR][]: 		[][][feature: mth1,mth2,mth1,mth1,mth1,mth1,mth1,mth1,mth1,mth1,mth1,mth1,mth1,mth1,mth1];
// [mode][IP2: color][]: 	[][][feature: contrast,brightness,saturation,colorTmp_r,colorTmp_g,colorTmp_b;];
// [mode][IP3: LC][]: 		[][][feature: blend_factor]; 
// [mode][IP4: NR][]: 		[][][feature: ]; 
int target_table[PQ_SCENE_TYPE_NUM][PQ_IP_TYPE_NUM][PQ_TUNING_NUM]={
{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, { 33, 31, 28, 24, 20, 18, 15, 13, 12, 11, 9, 6, 5, 5, 3},{ 125, 128<<2, 128, 2048,2048,2048,0,0,0,0,0,0,0,0,0},{  0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0},{ 255, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0}},
{{0,0,0,0,0,5,10,8,8,0,0,0,0,0,0}, { 33, 31, 28, 24, 20, 18, 15, 13, 12, 11, 9, 6, 5, 5, 3},{ 145, 118<<2, 120, 2048,2048,2048,0,0,0,0,0,0,0,0,0},{  0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0},{ 255, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0}},
{{0,0,0,0,0,5,10,8,8,0,0,0,0,0,0}, { 21, 19, 18, 16, 14, 12, 11, 10,  8,  7, 6, 4, 3, 2, 1},{ 132, 120<<2, 150, 2048,2048,2048,0,0,0,0,0,0,0,0,0},{  0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0},{ 255, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0}},
{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, { 33, 31, 28, 24, 20, 18, 15, 13, 12, 11, 9, 6, 5, 5, 3},{ 125, 128<<2, 255, 2048,2048,2048,0,0,0,0,0,0,0,0,0},{  0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0},{ 255, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0}}, 
{{255,255,100,1023,0,0,100,0,0,0,0,0,0,0,0}, { 33, 31, 28, 24, 20, 18, 15, 13, 12, 11, 9, 6, 5, 5, 3},{ 125, 128<<2, 140, 2048,2048,2048,0,0,0,0,0,0,0,0,0},{  0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0},{15, 2, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0}}, 
{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, { 16, 15, 14, 13, 11, 10,  9,  8,  7,  6, 5, 3, 2, 1, 0},{ 118, 128<<2, 120, 2048,1548,1048,0,0,0,0,0,0,0,0,0},{  0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0},{ 255, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0}}
};
// initial == normal 
//float cur_temp_val[2][15]={{ 80, 80,4,4,0,0,0,0,0,0,0,0,0,0,0}, {3,6,9,12,15,18,21,24,27,30,33,36,39,42,45} };  //*128
int cur_temp_val[PQ_IP_TYPE_NUM][PQ_TUNING_NUM]={
{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, { 33, 31, 28, 24, 20, 18, 15, 13, 12, 11, 9, 6, 5, 5, 3},{ 125, 128<<2, 147, 2048,2048,2048,0,0,0,0,0,0,0,0,0},{  64,  0,0,0,0,0,0,0,0,0,0,0,0,0,0},{ 255, 0, 0, 0, 0, 0, 0, 0,0,0,0,0,0,0,0} };  //*128
//{100,100,0,0,0,0,0,0,0,0,0,0,0,0,0}, { 33, 31, 28, 24, 20, 18, 15, 13, 12, 11, 9, 6, 5, 5, 3},{ 128, 118<<2, 197, 2048,2048,2048,0,0,0,0,0,0,0,0,0},{  64,  0,0,0,0,0,0,0,0,0,0,0,0,0,0},{ 8,4,2,3,10, 15,28,39,50,61,72,83,94,105,0} };  //*128

//int change_speed[PQ_IP_TYPE_NUM][PQ_TUNING_NUM]=
//{{ 60, 60,6,6,0,0,0,0,0,0,0,0,0,0,0}, {26,26,26,26,26,26,26,26,26,26,26,26,26,26,26}, { 60, 60, 60, 0,0,0,0,0,0,0,0,0,0,0,0}}; //  /128
int change_speed_acc[PQ_IP_TYPE_NUM][PQ_TUNING_NUM]=
{{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} };

int pretopClasses[top_num][2]={{AI_SCENE_OTHERs,0},{AI_SCENE_OTHERs,0},{AI_SCENE_OTHERs,0},{AI_SCENE_OTHERs,0},{AI_SCENE_OTHERs,0},{AI_SCENE_OTHERs,0},{AI_SCENE_OTHERs,0},{AI_SCENE_OTHERs,0},{AI_SCENE_OTHERs,0}};
void AI_scene_dynamic_control(int *info, unsigned char NN_flag)
{
	int scene_flag_final;
	int mod_num, mm;
	int max_scene_cnt;
	int max_scene_index;
	int jj, kk;
	int temp_tar;
	//float temp_cur;
	int temp_cur;
	int temp_step;


	// initial ....
	int scene_flag=0;		//0:normal, 1: sports, 2:animation
	static int scene_cnt[PQ_SCENE_TYPE_NUM]={0};	//0:normal, 1: dark, 2:sports, 3:beautyscenary, 4:animation, 5:movie 


	// para. setting
	int mod_cnt_maxth=30;
	int scene_cnt_th=10;
	int scene_cnt_method=1;
	//int ai_scene_mode;
	//static int ai_scene_mode_pre=0;
	int osd_pq_scene_cnt_th;
	//	


	int i = 0;
	//int topk = 2; // get top 2 classes and possibilities
	int **topClasses; //topk x 2 matrix, 2 is for saving classes and their possibilites
	
	//int nn_scene = 0;

	static int osd_flag_cnt = 0;
	static int scene_flag_final_pre = 0;
	
	scene_cnt_method = ai_scene_ctrl.ai_scene_global.scene_cnt_method;
	//ai_scene_mode = ai_scene_ctrl.ai_scene_global.ai_scene_mode;
	osd_pq_scene_cnt_th = ai_scene_ctrl.ai_scene_global.osd_pq_scene_cnt_th;

	topClasses = (int **)vip_malloc(top_num*sizeof(int*));
	for(i=0; i<top_num; i++)
		topClasses[i] = (int *)vip_malloc(2*sizeof(int));

	if (!AI_scene_get_top(info, topClasses, AI_SCENE_TYPE_NUM, top_num))
	{
		rtd_pr_vpq_ai_emerg("[%s][Err] top class num > Total class num\n",__func__);
		return;
	}
#if 0	
	/* more strict rules to adjust cur top 1 class */
	if(NN_flag && ai_scene_ctrl.ai_scene_global.adj_top_en==1)
	{
		if(AI_scene_adjust_top(topClasses, AI_SCENE_TYPE_NUM, top_num, 3))
		{
			rtd_pr_vpq_ai_info("top 1 class is adjusted\n");
			//rtd_pr_vpq_ai_info("=======================\n");
			//for(i=0; i<top_num; i++)
			//{
			//	rtd_pr_vpq_ai_emerg("top[%d] class:%d prob:%d, pre[%d] class:%d prob:%d\n", 
			//		i, topClasses[i][0], topClasses[i][1], i, pretopClasses[i][0], pretopClasses[i][1]);
			//}
			//rtd_pr_vpq_ai_info("=======================\n");
		}
	}
#endif
	/* set to global nn idx */
	scene_nn = topClasses[0][0]; 

	/* update pre top class */
	if(NN_flag) 
	{
		for(i=0; i<top_num; i++)
		{
			pretopClasses[i][0]=topClasses[i][0];
			pretopClasses[i][1]=topClasses[i][1];
		}
	}

	/* mapping class from AI ap to PQ */
	AI_scene_mapping_class(topClasses, top_num);

#if 0 // mac7p AI TBD memc
	/* MEMC cadance and black boundary detection */
	AI_scene_detect_movie_using_memc(topClasses);
#endif

	AI_scene_detect_face(topClasses);

#if 1 // adjust MEMC debug show class by counting ap result

	scene_flag = topClasses[0][0];

	if(scene_flag_final_pre == scene_flag)
	{
		osd_flag_cnt++;

		if(osd_flag_cnt >= 60*osd_pq_scene_cnt_th)
		{
			osd_flag_cnt = 60*osd_pq_scene_cnt_th;
			scene_pq = scene_flag;
		}
	}
	else
	{
		osd_flag_cnt = 0;
	}
	
	scene_flag_final_pre = scene_flag;
	
#else // by vip1271

	/*if(ai_scene_mode==0)
		scene_flag = 0;
	else*/ if(nn_scene == 0)
		scene_flag = PQ_SCENE_BASIC;
	else if(nn_scene == 1)
		scene_flag = PQ_SCENE_SPORTs;
	else if(nn_scene == 2)
		scene_flag = PQ_SCENE_BEAUTYSCENARY;
	else if(nn_scene == 4)
		scene_flag = PQ_SCENE_MOVIE;
	else
		scene_flag = PQ_SCENE_BASIC;
#endif

	if(ai_scene_rtk_mode != 0)
	{
		for (mod_num=0; mod_num<PQ_SCENE_TYPE_NUM; mod_num++)
		{
			if(scene_flag==mod_num)
			{	
				//if(scene_update) //lesley TBD
					scene_cnt[mod_num]++;

				if(scene_cnt[mod_num]>mod_cnt_maxth)
					scene_cnt[mod_num]=mod_cnt_maxth;
				
				for(mm=0; mm<PQ_SCENE_TYPE_NUM; mm++)
				{
					if(mm==mod_num)
						continue;

					//if(scene_update) //lesley TBD
						scene_cnt[mm]=scene_cnt[mm]-2;
					if(scene_cnt[mm]<0)
						scene_cnt[mm]=0;
				}
			}
		}

			
		max_scene_cnt=0;
		max_scene_index=0;
		for (mod_num=0; mod_num<PQ_SCENE_TYPE_NUM; mod_num++)
		{
			if(scene_cnt[mod_num]>max_scene_cnt)
			{
				max_scene_cnt=scene_cnt[mod_num];
				max_scene_index=mod_num;
			}
		}

		// not immediate change
		if(scene_cnt_method==1)
		{
			if(max_scene_cnt>=scene_cnt_th)
				scene_flag_final=max_scene_index;
			else
				scene_flag_final=PQ_SCENE_BASIC;
		}
		else //immediate change
			scene_flag_final=scene_flag;

		

		for (jj=0; jj<PQ_IP_TYPE_NUM; jj++) //IP
		{
			// for CDS reset gain as osd value
			if(jj==0) 
			{
		        color_sharp_dm_segpk_edgpk3_RBUS color_sharp_dm_segpk_edgpk3;
		        color_sharp_dm_segpk_edgpk4_RBUS color_sharp_dm_segpk_edgpk4;
		        color_sharp_dm_segpk_vpk5_RBUS color_sharp_dm_segpk_vpk5;
		        color_sharp_dm_segpk_vpk6_RBUS color_sharp_dm_segpk_vpk6;
				color_sharp_dm_segpk_edgpk3.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK3_reg);
				color_sharp_dm_segpk_edgpk4.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_EDGPK4_reg);
		        color_sharp_dm_segpk_vpk5.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK5_reg);
		        color_sharp_dm_segpk_vpk6.regValue = IoReg_Read32(COLOR_SHARP_DM_SEGPK_VPK6_reg);
				
				if(scene_flag_final!=PQ_SCENE_SPORTs && scene_flag_final!=PQ_SCENE_ANIMATION)
				{
					target_table[scene_flag_final][jj][0] = color_sharp_dm_segpk_edgpk3.gain_pos;
					target_table[scene_flag_final][jj][1] = color_sharp_dm_segpk_edgpk3.gain_neg;
					target_table[scene_flag_final][jj][2] = color_sharp_dm_segpk_edgpk4.hv_pos;
					target_table[scene_flag_final][jj][3] = color_sharp_dm_segpk_edgpk4.hv_neg;
					target_table[scene_flag_final][jj][4] = color_sharp_dm_segpk_edgpk4.lv;
					target_table[scene_flag_final][jj][5] = color_sharp_dm_segpk_vpk5.vpk_edg_gain_pos;
					target_table[scene_flag_final][jj][6] = color_sharp_dm_segpk_vpk5.vpk_edg_gain_neg;
					target_table[scene_flag_final][jj][7] = color_sharp_dm_segpk_vpk6.vpk_edg_hv_pos;
					target_table[scene_flag_final][jj][8] = color_sharp_dm_segpk_vpk6.vpk_edg_hv_neg;
					target_table[scene_flag_final][jj][9] = color_sharp_dm_segpk_vpk5.vpk_edg_lv;
				}
				/*else if(scene_flag_final==PQ_SCENE_ANIMATION) // only using edge pk, reset vedge pk
				{
					cds_cm_en[2] = 1;
					target_table[scene_flag_final][jj][5] = color_sharp_dm_segpk_vpk5.vpk_edg_gain_pos;
					target_table[scene_flag_final][jj][6] = color_sharp_dm_segpk_vpk5.vpk_edg_gain_neg;
					target_table[scene_flag_final][jj][7] = color_sharp_dm_segpk_vpk6.vpk_edg_hv_pos;
					target_table[scene_flag_final][jj][8] = color_sharp_dm_segpk_vpk6.vpk_edg_hv_neg;
					target_table[scene_flag_final][jj][9] = color_sharp_dm_segpk_vpk5.vpk_edg_lv;
				}
				else if(scene_flag_final==PQ_SCENE_SPORTs) // only using vedge pk, reset edge pk
				{
					cds_cm_en[1] = 1;
					target_table[scene_flag_final][jj][0] = color_sharp_dm_segpk_edgpk3.gain_pos;
					target_table[scene_flag_final][jj][1] = color_sharp_dm_segpk_edgpk3.gain_neg;
					target_table[scene_flag_final][jj][2] = color_sharp_dm_segpk_edgpk4.hv_pos;
					target_table[scene_flag_final][jj][3] = color_sharp_dm_segpk_edgpk4.hv_neg;
					target_table[scene_flag_final][jj][4] = color_sharp_dm_segpk_edgpk4.lv;
				}*/
			}
			for(kk=0; kk<PQ_TUNING_NUM; kk++)	//feature
			{
				//if(jj==0 && kk>4)
				//	continue;

				temp_tar=target_table[scene_flag_final][jj][kk];
				temp_cur=cur_temp_val[jj][kk];

				/*if(jj==0 && kk<10) {
				rtd_pr_vpq_ai_emerg("[%d] scene_flag:%d, scene_flag_final:%d\n", kk, scene_flag, scene_flag_final);
				rtd_pr_vpq_ai_emerg("[%d] change_speed:%d, change_speed_acc:%d\n", kk, ai_scene_ctrl.change_speed[jj][kk], change_speed_acc[jj][kk]);
				rtd_pr_vpq_ai_emerg("[%d] tar:%d, cur:%d\n", kk, temp_tar, temp_cur);
				}*/
				if(temp_cur<temp_tar)
				{
					temp_step = (ai_scene_ctrl.change_speed[jj][kk]+change_speed_acc[jj][kk]);

					if(scene_flag==scene_flag_final){ /////
						//temp_cur=temp_cur+change_speed[jj][kk]/128.0;
						temp_cur=temp_cur+temp_step/128;

						change_speed_acc[jj][kk] = temp_step%128;

					}

					
					if(temp_cur>temp_tar)
					{
						temp_cur=temp_tar;
						change_speed_acc[jj][kk] = 0;
					}
				}
				else if(temp_cur>temp_tar)
				{
					temp_step = (ai_scene_ctrl.change_speed[jj][kk]-change_speed_acc[jj][kk]);

					if(scene_flag==scene_flag_final){ /////
						//temp_cur=temp_cur-change_speed[jj][kk]/128.0;
						temp_cur=temp_cur-temp_step/128;
						
						change_speed_acc[jj][kk] = -(temp_step%128);
					}
					
					if(temp_cur<temp_tar)
					{
						temp_cur=temp_tar;
						change_speed_acc[jj][kk] = 0;
					}
				}
				else
				{
					change_speed_acc[jj][kk] = 0;
				}			

				//if(scene_cnt_method==0)
				//{
				//	temp_cur=temp_tar;
				//	change_speed_acc[jj][kk] = 0;
				//}
					
				/*if(jj==0) {
				rtd_pr_vpq_ai_emerg("[%d] change_speed:%d, change_speed_acc:%d\n", kk, ai_scene_ctrl.change_speed[jj][kk], change_speed_acc[jj][kk]);
				rtd_pr_vpq_ai_emerg("[%d] tar:%d, cur:%d\n", kk, temp_tar, temp_cur);
				}*/
				cur_temp_val[jj][kk]=temp_cur;
			}
			//if(jj==0) rtd_pr_vpq_ai_emerg("===========\n");
		}

		//scene_cnt_method = 1;

		//AI_scene_dynamic_control_cdsharp(scene_flag, scene_flag_final);
		if(scene_flag_final==PQ_SCENE_SPORTs)
			demo_20CES_tv030_NR(0);
	}
	else if(ai_scene_rtk_mode == 0)
	{
		demo_20CES_tv030_NR(1);
	}

	//scalerAI_scene_PQ_set();//move out
	
	for(i=0; i<top_num; i++)
		vip_free(topClasses[i]);
	vip_free(topClasses);
}
int AI_scene_adjust_top(int **topClasses, int outputCount, int topNum, int searchRange)
{
#define noAdj 0
#define Adjed 1
#define score_max 100
#define score_min 0

	int debugPrint = 0;
    int i = 0;
	int var_th = 0;
	static int score = 0;
	
	if(topClasses[0][0] == pretopClasses[0][0]) // cur top 1 is the same as pre top 1
	{
		if((score += 10) > score_max) score = score_max;
		if(debugPrint) rtd_pr_vpq_ai_emerg("top1: %d is the same, score:%d\n", topClasses[0][0], score);
		return noAdj;
	}
	else
	{
		/* find avg and var of probability of top classes */
		int sum = 0, avg = 0, var = 0;
		
	    for(i=0; i<topNum; i++)
		{
			sum += topClasses[i][1];
			if(debugPrint)rtd_pr_vpq_ai_emerg("top[%d] class:%d prob:%d, pre[%d] class:%d prob:%d\n", 
				i, topClasses[i][0], topClasses[i][1], i, pretopClasses[i][0], pretopClasses[i][1]);
	    }
		avg = sum/topNum;
		//if(debugPrint) rtd_pr_vpq_ai_emerg("sum:%d, avg:%d\n", sum, avg);
		
		sum = 0;
		for(i=0; i<topNum; i++)
		{
			sum += (topClasses[i][1] - avg) * (topClasses[i][1] - avg);
		}
		var = sum/topNum;
		if(debugPrint) rtd_pr_vpq_ai_emerg("sum:%d, var:%d, score:%d\n", sum, var, score);

		/* decide the var threshold by temporal score */		
		if(score >= 90)
			var_th = 4500;
		else if(score >= 85)
			var_th = 4000;
		else if(score >= 80)
			var_th = 3500;
		else if(score >= 75)
			var_th = 2400;
		else
			var_th = 1000;
		
		/* search pre top 1 is in the cur top searchRange or not */
	    for(i=1; i<searchRange; i++) 
		{
			if( (pretopClasses[0][0] == topClasses[i][0]) // pre top in the search range
				&& ( (var < var_th) || (pretopClasses[0][1] > 180) )	
				)	
			{
				int tmp_cls = topClasses[0][0];
				int tmp_prb = topClasses[0][1];

				topClasses[0][0] = topClasses[i][0];
				topClasses[0][1] = (topClasses[i][1]+pretopClasses[0][1])/2;
				
				topClasses[i][0] = tmp_cls;
				topClasses[i][1] = tmp_prb;
				
				if(debugPrint) rtd_pr_vpq_ai_emerg("var:%d, th:%d, score:%d\n", var, var_th, score);
				return Adjed;
			}
			if((score -= 10) < score_min) score = score_min;
	    }
	}

	return noAdj;
	
}
void AI_scene_sharp_set(void)
{
	int setting[15]={0};
	
	if(ai_scene_rtk_mode == 0)
	{
		memcpy(&setting[0], &target_table[PQ_SCENE_BASIC][PQ_IP_SHP][0], sizeof(int)*PQ_TUNING_NUM);
		drvif_color_AI_scene_srp_init(0,0);
	}
	else if(ai_scene_rtk_mode == 1)
	{
		memcpy(&setting[0], &cur_temp_val[PQ_IP_SHP][0], sizeof(int)*PQ_TUNING_NUM);
		
		if(scene_pq == PQ_SCENE_SPORTs || scene_pq == PQ_SCENE_BEAUTYSCENARY)
		{
			drvif_color_AI_scene_srp_init(1,0);
			drvif_color_AI_scene_cdsharp_set_cm1(0, setting);
		}
		else if(scene_pq == PQ_SCENE_ANIMATION)
		{
			drvif_color_AI_scene_srp_init(0,1);
			drvif_color_AI_scene_cdsharp_set_cm2(1, setting);
		}
		else
		{
			drvif_color_AI_scene_srp_init(0,0);
		}
	}
}

short bri_10bit=0;
#if 0 //lesley TBD
extern VPQ_SetPicCtrl_T pic_ctrl;
extern unsigned char PQ_init;
#endif
void AI_scene_color_set(void)
{

	int setting[15]={0};

	_system_setting_info *system_info_struct = NULL;

	system_info_struct = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (system_info_struct == NULL) {
		rtd_pr_vpq_ai_emerg("[%s][Err] ~get shareMem | gvipTbl error return~\n",__func__);
		return;
	}
	
	if(ai_scene_rtk_mode == 0)
		memcpy(&setting[0], &target_table[PQ_SCENE_BASIC][PQ_IP_COLOR][0], sizeof(int)*PQ_TUNING_NUM);
	else
		memcpy(&setting[0], &cur_temp_val[PQ_IP_COLOR][0], sizeof(int)*PQ_TUNING_NUM);

	bri_10bit =  setting[1];
	system_info_struct->OSD_Info.Contrast_Gain = fwif_color_ChangeOneUINT16Endian(setting[0], 0);
	system_info_struct->OSD_Info.Brightness_Gain = fwif_color_ChangeOneUINT16Endian(setting[1]>>2, 0);
	system_info_struct->OSD_Info.Saturation_Gain = fwif_color_ChangeOneUINT16Endian(setting[2], 0);
	fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain_AI(0, SLR_MAIN_DISPLAY, system_info_struct);

	drvif_color_set_color_temp(1,  setting[3],  setting[4],  setting[5], 0, 0, 0);

}

void AI_scene_TNR_set(void)
{
#if 0 //lesley TBD
	int setting[15]={0};

	unsigned int table_row = 0;
	unsigned char i = 0,j=0,y_ini=0,c_ini=0;
	SLR_VIP_TABLE *shareMem_VIP_Table = NULL;
    SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	unsigned char source=255,pqa_table_idx=0;

	unsigned int PQA_reg_value=0,PQA_bitup_value=0,PQA_bitlow_value=0;

	//
	if(ai_scene_rtk_mode == 0)
		memcpy(&setting[0], &target_table[PQ_SCENE_BASIC][PQ_IP_TNR][0], sizeof(int)*PQ_TUNING_NUM);
	else
		memcpy(&setting[0], &cur_temp_val[PQ_IP_TNR][0], sizeof(int)*PQ_TUNING_NUM);
	
	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);/* get vip source by input src and timing*/
	if (source >= VIP_QUALITY_SOURCE_NUM)
		source = 0;

	shareMem_VIP_Table = fwif_color_GetShare_Memory_VIP_TABLE_Struct();

	pqa_table_idx = gVip_Table->VIP_QUALITY_Extend3_Coef[source][VIP_QUALITY_FUNCTION_PQA_Table];

	y_ini=0;
	c_ini=0;

	for (table_row = 0; table_row < PQA_ITEM_MAX; table_row++)
	{


		PQA_reg_value = shareMem_VIP_Table->PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row][PQA_reg];
		PQA_bitup_value = shareMem_VIP_Table->PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row][PQA_bitup];
		PQA_bitlow_value = shareMem_VIP_Table->PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row][PQA_bitlow];

		if((PQA_reg_value == DI_IM_DI_RTNR_Y_16_TH12_TH14_reg)&&(PQA_bitup_value == 23)&&(PQA_bitlow_value == 16)&&(y_ini==0))
		{
			y_ini=1;
			for(i=0;i<15;i++){
				for(j=0;j<10;j++){
					
					shareMem_VIP_Table->PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row+i][PQA_L00+j] = setting[i];
					//shareMem_VIP_Table->PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row+i][PQA_L00+j] = cur_temp_val[PQ_IP_TNR][i];
				}
			}
		}

#if 0 // C
		if((PQA_reg_value == DI_IM_DI_RTNR_C_TH4_TH6_reg)&&(PQA_bitup_value == 23)&&(PQA_bitlow_value == 16)&&(c_ini==0))
		{
			c_ini=1;

			for(i=0;i<7;i++){
				for(j=0;j<10;j++){
					if ((table_row+i) < 255) 
					{

						shareMem_VIP_Table->PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row+i][PQA_L00+j] = cur_temp_val[PQ_IP_TNR][i];
					}
				}
			}
		}
#endif
	}
#endif
}

void AI_scene_LC_set(void)
{

	int setting[15]={0};
	lc_lc_tonemapping_blending_RBUS blending;
	lc_lc_global_ctrl1_RBUS lc_lc_global_ctrl1_reg;

	if(ai_scene_rtk_mode == 0)
		memcpy(&setting[0], &target_table[PQ_SCENE_BASIC][PQ_IP_LC][0], sizeof(int)*PQ_TUNING_NUM);
	else
		memcpy(&setting[0], &cur_temp_val[PQ_IP_LC][0], sizeof(int)*PQ_TUNING_NUM);
	

	blending.regValue = IoReg_Read32(LC_LC_ToneMapping_blending_reg);

	blending.lc_tmap_blend_factor = setting[0];
	IoReg_Write32(LC_LC_ToneMapping_blending_reg, blending.regValue);


	lc_lc_global_ctrl1_reg.regValue = IoReg_Read32(LC_LC_Global_Ctrl1_reg);
	lc_lc_global_ctrl1_reg.lc_db_apply = 1;
	IoReg_Write32(LC_LC_Global_Ctrl1_reg, lc_lc_global_ctrl1_reg.regValue);

}

void AI_scene_NR_set(void)
{
	int setting[15]={0};
	
	nr_mosquito_ctrl_RBUS nr_mosquito_ctrl_RBUS_reg;

	if(ai_scene_rtk_mode == 0)
		memcpy(&setting[0], &target_table[PQ_SCENE_BASIC][PQ_IP_NR][0], sizeof(int)*PQ_TUNING_NUM);
	else
		memcpy(&setting[0], &cur_temp_val[PQ_IP_NR][0], sizeof(int)*PQ_TUNING_NUM);

	nr_mosquito_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_CTRL_reg);

    /*edge_th*/
	nr_mosquito_ctrl_RBUS_reg.mosquito_edgethd		= setting[0];
	nr_mosquito_ctrl_RBUS_reg.mosquito_edgethd_step	= setting[1];

	IoReg_Write32(NR_MOSQUITO_CTRL_reg, nr_mosquito_ctrl_RBUS_reg.regValue);
	//rtd_pr_vpq_ai_emerg("lsy, nr %d %d %d %d %d %d %d %d\n",setting[0],setting[1],setting[2],setting[3],setting[4],setting[5],setting[6],setting[7],setting[8]);

	drvif_color_DRV_SNR_Mosquito_NR_DEMO();
	drvif_color_DRV_SNR_Modified_LPF_DEMO();
}

void AI_scene_NR_reset(void)
{
	//drvif_color_DRV_SNR_Mosquito_NR_reset_DEMO();

	SLR_VIP_TABLE *gVip_Table = NULL;
	gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();

	fwif_color_SNR_flow(&(gVip_Table->Manual_NR_Table[Scaler_GetDNR_table()][Scaler_GetDNR()]));
}


void AI_scene_DCC_set(void)
{
}

void AI_scene_ICM_set(void)
{
}

void AI_scene_sharp_reset(void)
{
	Scaler_VIP_scene_sharpness_reset();
}

void AI_scene_color_reset(void)
{
	/*_system_setting_info *system_info_struct = NULL;

	system_info_struct = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (system_info_struct == NULL) {
		rtd_pr_vpq_ai_emerg("[%s][Err] ~get shareMem | gvipTbl error return~\n",__func__);
		return;
	}

	system_info_struct->OSD_Info.Contrast_Gain = osd_con;
	system_info_struct->OSD_Info.Brightness_Gain = osd_bri;
	system_info_struct->OSD_Info.Saturation_Gain = osd_sat;
	
	fwif_color_vpq_pic_init();
	drvif_color_set_color_temp(0,  2048,  2048,  2048, 0, 0, 0);*/


	//unsigned short sourceIndex = (Scaler_DispGetInputInfoByDisp(0, SLR_INPUT_DATA_ARRAY_IDX));//GET_UI_SOURCE_FROM_DISPLAY(0);
	//Scaler_SetColorTemp((SLR_COLORTEMP_LEVEL)scaler_get_color_temp_level_type(sourceIndex));
	/*unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);
	Scaler_SetColorTemp((SLR_COLORTEMP_LEVEL)scaler_get_color_temp_level_type(src_idx));*/ /*need to fix CSFC*/
	Scaler_SetColorTempData(&vpqex_color_temp);

	Scaler_SetContrast(Scaler_GetContrast());
	Scaler_SetBrightness(Scaler_GetBrightness());
	Scaler_SetSaturation(Scaler_GetSaturation());
	//Scaler_SetHue(Scaler_GetHue());

}

void AI_scene_TNR_reset(void)
{
	//fwif_color_set_PQA_motion_threshold_TV006();// lesley TBD
}

void AI_scene_LC_reset(void)
{
	Scaler_VIP_LC_reset();
}

int AI_scene_get_top(int *pfProb, int **topClasses, int outputCount, int topNum)
{
    int i, j, k;
	#if 1 //1112 lesley, new
	int m, p1=0, p2=0;
	#endif

    if (topNum > AI_SCENE_TYPE_NUM) return FALSE;

    for(i=0; i<topNum; i++)
	{
		topClasses[i][0] = -1; // class
		#if 1  //1112 lesley, new
		topClasses[i][1] = -1; // possibility 0 ~ 255
		#else
		topClasses[i][1] = 0; // possibility 0 ~ 255
		#endif
	}

    for(j = 0; j < topNum; j++)
    {
        for(i=0; i<outputCount; i++)
        {
            for(k=0; k < topNum; k ++)
            {
                if(i == topClasses[k][0])
                    break;
            }

            if(k != topNum)
                continue;

            if(pfProb[i] > topClasses[j][1])
            {
                topClasses[j][0] = i;
                topClasses[j][1] = pfProb[i];
            }
		#if 1 //1112 lesley, new
			/* sports and game classes sometimes have the same probability */
			else if(pfProb[i] == topClasses[j][1])
			{
				for(m=0; m<topNum; m++)
				{
                     if(i==pretopClasses[m][0])
                     {
                         p1=m;
						 break;
                     }
        }
                
  				for(m=0; m<topNum; m++)
				{
                     if(topClasses[j][0]==pretopClasses[m][0])
                     {
                         p2=m;
						 break;
                     }
                }

	            if(p1<p2)
				{
	                topClasses[j][0] = i;
	                topClasses[j][1] = pfProb[i];
				}    
			}
		#endif 
        }
		#if 1 //1112 lesley, new
		#else
		if(topClasses[j][0]==-1) topClasses[j][0] = AI_SCENE_OTHERs;
		#endif
    }

	return TRUE;
}
void AI_scene_mapping_class(int **topClasses, int topk)
{
	int i = 0;

	for(i=0;i<topk;i++)
	{
		switch(topClasses[i][0])
		{
			case AI_SCENE_SPORTs:
				topClasses[i][0] = PQ_SCENE_SPORTs;
				break;
			case AI_SCENE_LANDSCAPE:
				topClasses[i][0] = PQ_SCENE_BEAUTYSCENARY;
				break;
			case AI_SCENE_ANIMATION:
			case AI_SCENE_GAME:
				topClasses[i][0] = PQ_SCENE_ANIMATION;
				break;
			case AI_SCENE_NIGHT_BUILDING:
			case AI_SCENE_FIREWORKS:
				topClasses[i][0] = PQ_SCENE_DARK;
				break;
			case AI_SCENE_CONCERT:
			case AI_SCENE_NEWS:
			case AI_SCENE_OTHERs:
			default:
				topClasses[i][0] = PQ_SCENE_BASIC;
				break;
		}
	}
}
#if 0 // mac7p AI TBD memc
void AI_scene_detect_movie_using_memc(int **topClasses)
{
	unsigned char *FilmInfo = Scaler_MEMC_GetCadence();
	unsigned int bdtop = 0, bdbot = 0;
	static char isFilm = 0;
	int bdtop_th, bdbot_th;
	//unsigned int fps;
	

	bdtop_th = ai_scene_ctrl.ai_scene_global.bdtop_th;
	bdbot_th = ai_scene_ctrl.ai_scene_global.bdbot_th;

	
	if(FilmInfo == NULL)
	{
		rtd_pr_vpq_ai_emerg("[%s][Err] Film info ptr from MEMC error\n",__func__);
		return;
	}

	// Debounce
	if( FilmInfo[_FILM_MEMC_OUT_ALL] == 0 && isFilm > 0 )
		isFilm--;
	else if( FilmInfo[_FILM_MEMC_OUT_ALL] != 0 )
		isFilm = 40;
	
	ReadRegister(KIWI_REG(FRC_TOP__MC__mc_top_rim1), &bdtop);
	ReadRegister(KIWI_REG(FRC_TOP__MC__mc_bot_rim1), &bdbot);

	//RTKReadRegister(0xB80282E4, &fps);
	//fps = ((270000000/(fps+1))+5)/10;

	//if(rtd_inl(0xb8025128)&0x1)
	//	rtd_pr_vpq_ai_info("lsy, cad %d, fps %d\n", FilmInfo[_FILM_MEMC_OUT_ALL], fps);


	if(isFilm && bdtop > bdtop_th && bdbot < bdbot_th )
	//if(isFilm && ((FilmInfo[_FILM_MEMC_OUT_ALL]==0 && fps<30)||(FilmInfo[_FILM_MEMC_OUT_ALL]==1 && fps>45 && fps<55)||(FilmInfo[_FILM_MEMC_OUT_ALL]==2 && fps>55 && fps<65)))
		topClasses[0][0] = PQ_SCENE_MOVIE;
}
#endif

void AI_scene_detect_face(int **topClasses)
{	
	// for 2020 CES show 
	// force basic scene when the face detected
	   
	if(debug_draw_cnt>0)
		topClasses[0][0] = PQ_SCENE_BASIC;
}

//void AI_dynamic_control(AIInfo face_in[6], int scene_change)
// chen 0703
void AI_face_dynamic_control(AIInfo face_in[6], int scene_change, unsigned char NN_flag)
{
	AIInfo face={0};
	int face_remap_index[6]={0};
	int face_cur_IOU_ratio[6]={0};
	int AI_face_flag[6]={0};

	// chen 0409 for db
	int face_out_idx[6]={0};

	int tt, tt2, tt3, tt4; // for for-loop
	int faceIdx_mod=0;
	int d_max_index=0;
	int d_ratio_max=0;
	int x0_pre, x1_pre, y0_pre, y1_pre;
	int x0_cur, x1_cur, y0_cur, y1_cur;
	int w_cur, h_cur, w_pre, h_pre;
	int w_diff, h_diff;
	int d_ratio;
	int area_u;
	int temp_index;

	// chen 0429
	int d_ratio_max2=0;
	int d_max2_index=0;
	int face_cur_IOU_ratio_max2[6]={0};

	int range_gain;
	int sc_count_th;
	int ratio_max_th;

	int frame_drop_num;
	int frame_delay;

	// chen 0805
	int max_face_size=0;
	//end chen 0805

	// lesley 0808
	int ai_sc_y_diff_th;
	int ai_sc_count_th = 0;
	int ai_scene_change = 0;
	static unsigned int preY = 0;
	unsigned int curY = 0;
	int y_diff=0;
	_clues* SmartPic_clue=NULL;

	// end lesley 0808

	// chen 0812
	int disappear_between_faces_new_en=0;
	//end chen 0812


	// chen 0815_2
	int sc_y_diff_th=0;
	int AI_face_sharp_dynamic_en=0;
	color_sharp_shp_cds_region_enable_RBUS reg_color_sharp_shp_cds_region_enable_reg;
	// end chen 0815_2

	// lesley 0815
	int AI_face_sharp_mode = 0;
	static int shp_mode_pre = -1;
	int shp_mode_cur = 0;
	//end lesley 0815

	// lesley 0818
	int debug_face_info_mode = 0;
	// end lesley 0818

	// lesley 0820_2
	int IOU_decay_en = 0;
	int IOU_decay = 0;
	// end lesley 0820_2


	// lesley 0829
	//int keep_still_en;
	int ratio = 0;
	int still_ratio_th;
	int still_ratio_th1;
	int still_ratio_th2;
	int hue_ratio;
	// end lesley 0829

	// lesley 0829_2
	int x, y, w, h, c;
	int draw_tx, draw_ty, draw_tw, draw_th;
	int icm_global_en;
	// end lesley 0829_2

	// lesley 0904
	int scene_change_en;
	// end lesley 0904

	// lesley 0906_2
	int i;
	static unsigned char buf_idx = 0;
	static int ai_sc_y_diff_th_dy = 0;
	int y_diff_pre_avg = 0;
	int ai_sc_y_diff_th1;

	static int ai_sc_hue_ratio_th_dy = 0;
	int hue_ratio_pre_avg = 0;
	int ai_sc_hue_ratio_th;
	int ai_sc_hue_ratio_th1;
	// end lesley 0906_2

	int draw_blend_en;
	int gdma_w = 0, gdma_h = 0;

	// setting //
	//scaler_AI_Ctrl_Get(&ai_ctrl_dyn);
#if 0 // V4L2_ERR
	// lesley 1014
	drvif_color_set_DB_AI_DCC();
	drvif_color_set_DB_AI_ICM();
	drvif_color_set_DB_AI_SHP();
	// end lesley 1014
#endif
	sc_count_th=ai_ctrl.ai_global.sc_count_th;
	ratio_max_th=ai_ctrl.ai_global.ratio_max_th;
	range_gain=ai_ctrl.ai_global.range_gain;
	frame_drop_num=ai_ctrl.ai_global.frame_drop_num;
	frame_delay=ai_ctrl.ai_global.frame_delay;
	// end chen 0429

	// chen 0812
	disappear_between_faces_new_en=ai_ctrl.ai_global3.disappear_between_faces_new_en;
	//end chen 0812


	// chen 0815_2
	sc_y_diff_th = ai_ctrl.ai_global3.sc_y_diff_th;
	AI_face_sharp_dynamic_en = ai_ctrl.ai_shp_tune.AI_face_sharp_dynamic_en;
	reg_color_sharp_shp_cds_region_enable_reg.regValue=IoReg_Read32(COLOR_SHARP_SHP_CDS_REGION_ENABLE_reg);
	// end chen 0815_2

	// lesley 0815
	AI_face_sharp_mode = ai_ctrl.ai_shp_tune.AI_face_sharp_mode;
	//end lesley 0815

	// lesley 0818
	debug_face_info_mode = ai_ctrl.ai_global3.debug_face_info_mode;
	// end lesley 0818

	// lesley 0820_2
	IOU_decay_en = ai_ctrl.ai_global3.IOU_decay_en;
	IOU_decay = ai_ctrl.ai_global3.IOU_decay;
	// end lesley 0820_2

	// lesley 0829
	//keep_still_en = ai_ctrl.ai_global3.keep_still_en;
	still_ratio_th = ai_ctrl.ai_global3.still_ratio_th;
	still_ratio_th1 = ai_ctrl.ai_global3.still_ratio_th1;
	still_ratio_th2 = ai_ctrl.ai_global3.still_ratio_th2;
	// end lesley 0829

	// lesley 0829_2
	icm_global_en = ai_ctrl.ai_icm_tune2.icm_global_en;
	// end lesley 0829_2

	// lesley 0904
	scene_change_en = ai_ctrl.ai_global3.scene_change_en;
	// end lesley 0904

	draw_blend_en = ai_ctrl.ai_global3.draw_blend_en;

// chen 0815_2 move
/*
	if(scene_change==1)
	{
		scene_change_flag=1;
		scene_change_count=0;
	}

	if(scene_change_flag==1)
		scene_change_count++;

	if(scene_change_count>=sc_count_th)
	{
		scene_change_flag=0;
		scene_change_count=0;
	}
*/
// end chen 0815_2 move

	// lesley 0808
	SmartPic_clue = scaler_GetShare_Memory_SmartPic_Clue_Struct();
	ai_sc_y_diff_th = ai_ctrl.ai_global3.ai_sc_y_diff_th;
	ai_sc_count_th = ai_ctrl.ai_global3.ai_sc_count_th;

	curY = SmartPic_clue->Hist_Y_Mean_Value;
	y_diff = abs(curY - preY);
	preY = curY;

	// lesley 0906_2
	// y -------
	ai_sc_y_diff_th1 = ai_ctrl.ai_global3.ai_sc_y_diff_th1;

	y_diff_pre[buf_idx] = y_diff;

	for(i=0; i<16; i++)
		y_diff_pre_avg += y_diff_pre[i];

	ai_sc_y_diff_th_dy = y_diff_pre_avg>>4;

	if(ai_sc_y_diff_th_dy > ai_sc_y_diff_th)
		ai_sc_y_diff_th_dy = ai_sc_y_diff_th;
	if(ai_sc_y_diff_th_dy < ai_sc_y_diff_th1)
		ai_sc_y_diff_th_dy = ai_sc_y_diff_th1;

	// hue -------
	ai_sc_hue_ratio_th = ai_ctrl.ai_global3.ai_sc_hue_ratio_th;
	ai_sc_hue_ratio_th1 = ai_ctrl.ai_global3.ai_sc_hue_ratio_th1;
	hue_ratio = (int)get_hue_hist_ratio();
	hue_ratio_pre[buf_idx] = hue_ratio;

	for(i=0; i<16; i++)
		hue_ratio_pre_avg += hue_ratio_pre[i];

	ai_sc_hue_ratio_th_dy = (hue_ratio_pre_avg>>4) - ai_sc_hue_ratio_th1;

	if(ai_sc_hue_ratio_th_dy < ai_sc_hue_ratio_th)
		ai_sc_hue_ratio_th_dy = ai_sc_hue_ratio_th;
	// ------

	buf_idx = (buf_idx+1)%16;

	// end lesley 0906_2

	/*if((rtd_inl(0xb8025128)&0x1)==1)
	{
		for(i=0; i<6; i++)
			rtd_pr_vpq_ai_emerg("lsy [%d] %d, %d %d %d %d\n", i, face_in[i].pv8, face_in[i].cx12, face_in[i].cy12, face_in[i].w12, face_in[i].h12);
	}*/


 	//if(y_diff > ai_sc_y_diff_th)// lesley 0906_2
 	if(y_diff > ai_sc_y_diff_th_dy || hue_ratio < ai_sc_hue_ratio_th_dy)
 	{
		//change_sc_offset_sta = (ai_sc_y_diff_th - y_diff)*32;// lesley 0906_2
		change_sc_offset_sta = MIN((ai_sc_y_diff_th_dy - y_diff)*32, (hue_ratio - ai_sc_hue_ratio_th_dy));

		ai_scene_change = 1;
 	}

	if(change_sc_offset_sta < -255)
		change_sc_offset_sta = -255;

	if(ai_scene_change == 1)
	{
		ai_scene_change_flag = 1;
		ai_scene_change_count = 0;
	}

	if(ai_scene_change_flag == 1)
	{
		ai_scene_change_count++;
		if((ai_sc_count_th - 1) > 0)
			change_speed_ai_sc = change_sc_offset_sta * (ai_sc_count_th - ai_scene_change_count) / (ai_sc_count_th - 1);
		else
			change_speed_ai_sc = 0;
	}

	if(ai_scene_change_count >= ai_sc_count_th)
	{
		ai_scene_change_flag = 0;
		ai_scene_change_done = 1;
	}

	show_ai_sc = ai_scene_change_flag;
	
	// chen 0815_2 
	
	if(scene_change_en)
	{
		if(scene_change==1 || y_diff>sc_y_diff_th)
		{
			scene_change_flag=1;
			scene_change_count=0;
		}

		if(scene_change_flag==1)
			scene_change_count++;

		if(scene_change_count>=sc_count_th)
		{
			scene_change_flag=0;
		}
	}
	// end chen 0815_2

	// end lesley 0808


	// lesley 0829
	{
		extern unsigned char scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(void);
		ratio = SmartPic_clue->RTNR_MAD_count_Y_avg_ratio + (still_ratio_th - SmartPic_clue->RTNR_MAD_count_Y2_avg_ratio) - (SmartPic_clue->RTNR_MAD_count_Y3_avg_ratio);
		ratio = (ratio/10 + scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio())/2;

		// lesley 0906_1
		if(ratio > still_ratio_th1)
			still_ratio[0] = 32;
		else
			still_ratio[0] = 32 + ratio - still_ratio_th1;

		ratio = scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio();

		if(ratio > still_ratio_th2)
			still_ratio[1] = 32;
		else
			still_ratio[1] = 32 + ratio - still_ratio_th2;
		// end lesley 0906_1

	}
	// end lesley 0829



	// lesley 0815, for init shp mode
	shp_mode_cur = (AI_face_sharp_dynamic_en<<1)|(AI_face_sharp_mode);

	if(shp_mode_cur != shp_mode_pre)
	{
		shp_mode_pre = shp_mode_cur;

		if(AI_face_sharp_dynamic_en)
		{
			AI_face_sharp_dynamic_single = 0;
			AI_face_sharp_dynamic_global = 1;
			reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable = 0;
		}
		else if(AI_face_sharp_mode == 0)
		{
			AI_face_sharp_dynamic_single = 0;
			AI_face_sharp_dynamic_global = 0;
			reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable = 1;
		}
		else if(AI_face_sharp_mode == 1)
		{
			AI_face_sharp_dynamic_single = 1;
			AI_face_sharp_dynamic_global = 0;
			reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable = 1;
		}
		else if(AI_face_sharp_mode == 2)
		{
			AI_face_sharp_dynamic_single = 0;
			AI_face_sharp_dynamic_global = 1;
			reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable = 0;
		}

		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION_ENABLE_reg,reg_color_sharp_shp_cds_region_enable_reg.regValue);
	}
	// end lesley 0815

	// scene change value reset
	if(scene_change_flag==1)
	{
		for (tt=0; tt<6; tt++)
		{
			// for IIR
			value_diff_pre[tt]=0; // Y: luminance
			h_diff_pre[tt]=0;
			w_diff_pre[tt]=0;
			change_speed_t[tt]=0;
			AI_detect_value_blend[tt]=0;
			face_info_pre[tt].cx12=0;
			face_info_pre[tt].cy12=0;
			face_info_pre[tt].h12=0;
			face_info_pre[tt].w12=0;

			IOU_pre[tt]=0;

			// chen 0429
			IOU_pre_max2[tt]=0;
			change_speed_t_dcc[tt]=0;
			AI_detect_value_blend_dcc[tt]=0;
			// end chen 0429

			// chen 0527
			change_speed_t_sharp[tt]=0;
			AI_detect_value_blend_sharp[tt]=0;
			//end chen 0527

			// lesley 0813
			h_adj_pre[tt]=0;
			s_adj_pre[tt]=0;
			// end lesley 0813

			// lesley 0808
			v_adj_pre[tt]=0;
			// end lesley 0808
		}
	}

	for(tt=0; tt<6; tt++)
	{
		face_remap_index[tt]=-1;
		AI_face_flag[tt]=0;
	}

	// face tracking /////////////////////////////////////////
	// IOU check
	// tt: input face index: current
	// tt2: pre faces index

	// chen 0703
	if(NN_flag==1)
		frame_drop_count=0;


	if(frame_drop_count==0)
	{
		for (tt=0; tt<6; tt++) // 6 faces input
		{
			face=face_in[tt];

			d_ratio_max=0;

			// chen 0429
			d_ratio_max2=0;
			// end chen 0429

			if(face.pv8!=0)
			{			
				for (tt2=0; tt2<6; tt2++) // check with pre 6 faces info, face tracking
				{
					x0_pre=face_info_pre[tt2].cx12-face_info_pre[tt2].w12/2;
					x1_pre=face_info_pre[tt2].cx12+face_info_pre[tt2].w12/2;
					y0_pre=face_info_pre[tt2].cy12-face_info_pre[tt2].h12/2;
					y1_pre=face_info_pre[tt2].cy12+face_info_pre[tt2].h12/2;

					x0_cur=face.cx12-face.w12/2;
					x1_cur=face.cx12+face.w12/2;
					y0_cur=face.cy12-face.h12/2;
					y1_cur=face.cy12+face.h12/2;

					w_cur=face.w12;
					h_cur=face.h12;

					w_pre=face_info_pre[tt2].w12;
					h_pre=face_info_pre[tt2].h12;

					w_diff=(w_cur+w_pre)-(MAX(x1_cur,x1_pre)-MIN(x0_cur,x0_pre));
					h_diff=(h_cur+h_pre)-(MAX(y1_cur,y1_pre)-MIN(y0_cur,y0_pre));

					// IOU calculate
					if(w_diff<0 || h_diff<0)
						d_ratio=0;
					else
					{
						area_u=(w_cur*h_cur+w_pre*h_pre-w_diff*h_diff);
						if(area_u<0)
							d_ratio=0;
						else
							d_ratio=w_diff*h_diff*100/area_u;
					}

					if(AI_detect_value_blend[tt2]==0) //means no pre_info
						d_ratio=0;

					if(d_ratio>d_ratio_max)
					{
						d_max_index=tt2;
						d_ratio_max=d_ratio;
					}
				}

				faceIdx_mod=d_max_index;
				face_cur_IOU_ratio[tt]=d_ratio_max;

				if(face_remap_index[faceIdx_mod]==-1)
				{
					if(d_ratio_max<ratio_max_th) //means new face, not occur in previous frame, give new face index
					{
						for (tt3=0; tt3<6;tt3++)
						{
							if(face_remap_index[tt3]==-1 && AI_detect_value_blend[tt3]==0)
							{
								face_remap_index[tt3]=tt;
								faceIdx_mod=tt3;
								AI_face_flag[faceIdx_mod]=1; // 1123check
								break;
							}
						}
					}
					else
					{
						face_remap_index[faceIdx_mod]=tt;
						AI_face_flag[faceIdx_mod]=1; // 1123check
					}
				}
				else //remap to the same index
				{
					if(face_cur_IOU_ratio[face_remap_index[faceIdx_mod]]<d_ratio_max)
					{
						temp_index=face_remap_index[faceIdx_mod];
						face_remap_index[faceIdx_mod]=tt;

						for(tt3=0; tt3<6; tt3++) // give new face index
						{
							if(face_remap_index[tt3]==-1 && AI_detect_value_blend[tt3]==0)
							{
								face_remap_index[tt3]=temp_index;
								faceIdx_mod=tt3;
								AI_face_flag[faceIdx_mod]=1; // 1123check
								break;
							}
						}
					}
					else
					{
						for(tt3=0; tt3<6; tt3++) // give new face index
						{
							if(face_remap_index[tt3]==-1 && AI_detect_value_blend[tt3]==0)
							{
								face_remap_index[tt3]=tt;
								faceIdx_mod=tt3;
								AI_face_flag[faceIdx_mod]=1; // 1123check
								break;
							}
						}
					}
				}
				//AI_face_flag[faceIdx_mod]=1; // 1123check

				// chen 0409 for db
				face_out_idx[tt]=faceIdx_mod;

				// chen 0429
				// .... IOU between current faces
				for (tt2=0; tt2<6; tt2++) // check with pre 6 faces info, face tracking
				{
					if(tt2!=tt && face_in[tt2].pv8!=0)
					{

						// lesley 0928
						if(ai_ctrl.ai_global3.IOU2_mode == 1)
						{

							int dx, dy, dist, r1, r2, maxr, minr, tmp;
							int IOU2_mode1_offset = ai_ctrl.ai_global3.IOU2_mode1_offset;
							int IOU2_mode1_range_gain = ai_ctrl.ai_global3.IOU2_mode1_range_gain;
							int range_ratio = ai_ctrl.ai_global3.IOU2_mode1_range_ratio;

							dx = abs(face.cx12 - face_in[tt2].cx12);
							dy = abs(face.cy12 - face_in[tt2].cy12);
							dist = (dx>dy)?(dx+dy/2-IOU2_mode1_offset*dy/200):(dy+dx/2-IOU2_mode1_offset*dx/200);
							r1 = (face.w12>face.h12)?((face.w12+range_ratio*face.h12/16)/2):((face.h12+range_ratio*face.w12/16)/2);
							r2 = (face_in[tt2].w12>face_in[tt2].h12)?((face_in[tt2].w12+range_ratio*face_in[tt2].h12/16)/2):((face_in[tt2].h12+range_ratio*face_in[tt2].w12/16)/2);
							minr = MIN(r1, r2)*IOU2_mode1_range_gain/8;
							maxr = MAX(r1, r2)*IOU2_mode1_range_gain/8;

							tmp = (maxr)?(100-100*(dist-minr)/maxr):(0);

							if(tmp<0)
								d_ratio = 0;
							else if(tmp>100)
								d_ratio = 100;
							else
								d_ratio = tmp;
						}
						else
						// end lesley 0928
						{
							x0_pre=face_in[tt2].cx12-face_in[tt2].w12*range_gain/4;
							x1_pre=face_in[tt2].cx12+face_in[tt2].w12*range_gain/4;
							y0_pre=face_in[tt2].cy12-face_in[tt2].h12*range_gain/4;
							y1_pre=face_in[tt2].cy12+face_in[tt2].h12*range_gain/4;

							x0_cur=face.cx12-face.w12*range_gain/4;
							x1_cur=face.cx12+face.w12*range_gain/4;
							y0_cur=face.cy12-face.h12*range_gain/4;
							y1_cur=face.cy12+face.h12*range_gain/4;

							w_cur=face.w12*range_gain/2;
							h_cur=face.h12*range_gain/2;

							w_pre=face_in[tt2].w12*range_gain/2;
							h_pre=face_in[tt2].h12*range_gain/2;

							w_diff=(w_cur+w_pre)-(MAX(x1_cur,x1_pre)-MIN(x0_cur,x0_pre));
							h_diff=(h_cur+h_pre)-(MAX(y1_cur,y1_pre)-MIN(y0_cur,y0_pre));

							// IOU calculate
							if(w_diff<0 || h_diff<0)
								d_ratio=0;
							else
							{
								area_u=(w_cur*h_cur+w_pre*h_pre-w_diff*h_diff);
								if(area_u<0)
									d_ratio=0;
								else
									d_ratio=w_diff*h_diff*100/area_u;
							}
						}

						if(d_ratio>d_ratio_max2)
						{
							d_max2_index=tt2;
							d_ratio_max2=d_ratio;
						}
					}
				}
				face_cur_IOU_ratio_max2[tt]=d_ratio_max2;
				// end chen 0429

			}// end pv8!=0
		}// end 6 face tacking
	}
	////////////// end face tracking

// chen 0805
	max_face_size=0;
//end chen 0805

	// after remap, after face tracking
	for (tt4=0; tt4<6; tt4++)
	{
		if(scene_change_flag==1) // no remapping
		{
			face=face_in[tt4];

			// chen 0805
			if(face.pv8>0)
			{
				if(max_face_size<face.w12)
				{
					max_face_size=face.w12;
				}

			}
			//end chen 0805

			AI_face_win_pos_predict(tt4, face);
			AI_face_ICM_blending_value(tt4, face);

			// chen 0429
			AI_face_DCC_blending_value(tt4, face);
			// end chen 0429

			// chen 0527
			AI_face_Sharp_blending_value(tt4, face);
			//end chen 0527

			AI_face_ICM_tuning(tt4, face);

			if(face.pv8>0)
				face_remap_index[tt4]=tt4;
		}
		else
		{
			if(AI_face_flag[tt4]==1 && frame_drop_count==0)
			{

				face=face_in[face_remap_index[tt4]];

				AI_face_win_pos_predict(tt4, face);
				AI_face_ICM_blending_value(tt4, face);

				// chen 0429
				AI_face_DCC_blending_value(tt4, face);
				// end chen 0429

				// chen 0527
				AI_face_Sharp_blending_value(tt4, face);
				//end chen 0527

				AI_face_ICM_tuning(tt4, face);

				// for db
				face_info_pre2[tt4]=face_info_pre[tt4];

				face_info_pre[tt4]=face;
				IOU_pre[tt4]=face_cur_IOU_ratio[face_remap_index[tt4]];

				// chen 0429
				IOU_pre_max2[tt4]=face_cur_IOU_ratio_max2[face_remap_index[tt4]];
				// end chen 0429

			}
			else
			{

				if(AI_detect_value_blend[tt4]>0)
				{
					face=face_info_pre[tt4];

					face_remap_index[tt4]=-2; //pre
					//IOU_pre[tt4]=IOU_pre[tt4];

					// lesley 0820_2
					if(IOU_decay_en)
					 IOU_pre[tt4]=IOU_pre[tt4]*IOU_decay/100;
					// end lesley 0820_2

					// chen 0812
					d_ratio_max2=0;
					for (tt=0; tt<6; tt++) // IOU between two faces, check with cur 6 faces info
					{
						if(face_in[tt].pv8!=0)
						{
							// lesley 0928
							if(ai_ctrl.ai_global3.IOU2_mode == 1)
							{

								int dx, dy, dist, r1, r2, maxr, minr, tmp;
								int IOU2_mode1_offset = ai_ctrl.ai_global3.IOU2_mode1_offset;
								int IOU2_mode1_range_gain = ai_ctrl.ai_global3.IOU2_mode1_range_gain;
								int range_ratio = ai_ctrl.ai_global3.IOU2_mode1_range_ratio;

								dx = abs(face.cx12 - face_in[tt].cx12);
								dy = abs(face.cy12 - face_in[tt].cy12);
								dist = (dx>dy)?(dx+dy/2-IOU2_mode1_offset*dy/200):(dy+dx/2-IOU2_mode1_offset*dx/200);
								r1 = (face.w12>face.h12)?((face.w12+range_ratio*face.h12/16)/2):((face.h12+range_ratio*face.w12/16)/2);
								r2 = (face_in[tt].w12>face_in[tt].h12)?((face_in[tt].w12+range_ratio*face_in[tt].h12/16)/2):((face_in[tt].h12+range_ratio*face_in[tt].w12/16)/2);
								minr = MIN(r1, r2)*IOU2_mode1_range_gain/8;
								maxr = MAX(r1, r2)*IOU2_mode1_range_gain/8;

								tmp = (maxr)?(100-100*(dist-minr)/maxr):(0);

								if(tmp<0)
									d_ratio = 0;
								else if(tmp>100)
									d_ratio = 100;
								else
									d_ratio = tmp;
							}
							else
							// end lesley 0928
							{
								x0_pre=face_in[tt].cx12-face_in[tt].w12*range_gain/4;
								x1_pre=face_in[tt].cx12+face_in[tt].w12*range_gain/4;
								y0_pre=face_in[tt].cy12-face_in[tt].h12*range_gain/4;
								y1_pre=face_in[tt].cy12+face_in[tt].h12*range_gain/4;

								x0_cur=face.cx12-face.w12*range_gain/4;
								x1_cur=face.cx12+face.w12*range_gain/4;
								y0_cur=face.cy12-face.h12*range_gain/4;
								y1_cur=face.cy12+face.h12*range_gain/4;

								w_cur=face.w12*range_gain/2;
								h_cur=face.h12*range_gain/2;

								w_pre=face_in[tt].w12*range_gain/2;
								h_pre=face_in[tt].h12*range_gain/2;

								w_diff=(w_cur+w_pre)-(MAX(x1_cur,x1_pre)-MIN(x0_cur,x0_pre));
								h_diff=(h_cur+h_pre)-(MAX(y1_cur,y1_pre)-MIN(y0_cur,y0_pre));

								// IOU calculate
								if(w_diff<0 || h_diff<0)
									d_ratio=0;
								else
								{
									area_u=(w_cur*h_cur+w_pre*h_pre-w_diff*h_diff);
									if(area_u<0)
										d_ratio=0;
									else
										d_ratio=w_diff*h_diff*100/area_u;
								}
							}

							if(d_ratio>d_ratio_max2)
							{
								d_ratio_max2=d_ratio;
							}
						}
					}
					if(disappear_between_faces_new_en==1)
						IOU_pre_max2[tt4]=d_ratio_max2;
					//end chen 0812

				}

				face.pv8=0;

				//// chen 0409 for db
				face_info_pre2[tt4]=face_info_pre[tt4];

				AI_face_win_pos_predict(tt4,face);
				AI_face_ICM_blending_value(tt4, face);

				// chen 0429
				AI_face_DCC_blending_value(tt4, face);
				// end chen 0429

				// chen 0527
				AI_face_Sharp_blending_value(tt4, face);
				//end chen 0527


				AI_face_ICM_tuning(tt4, face);
			}

			// chen 0805
			if(face.pv8>0)
			{
				if(max_face_size<face.w12)
				{
					max_face_size=face.w12;
				}

			}
			//end chen 0805
		}
	}

	// chen 0815_2
	if(AI_face_sharp_dynamic_en && scene_change_count==sc_count_th)
	{
		if(AI_face_sharp_count<=1)
		{
			AI_face_sharp_dynamic_single=1;
			AI_face_sharp_dynamic_global=0;
			reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable=1;
		}
		else
		{
			AI_face_sharp_dynamic_single=0;
			AI_face_sharp_dynamic_global=1;
			reg_color_sharp_shp_cds_region_enable_reg.cds_region_0_enable=0;
		}

		IoReg_Write32(COLOR_SHARP_SHP_CDS_REGION_ENABLE_reg,reg_color_sharp_shp_cds_region_enable_reg.regValue);
	}
	//end chen 0815_2


	// chen 0805
	AI_face_DCC_blending_value_global(max_face_size);
	//end chen 0805

	// lesley 0815
	AI_face_Sharp_blending_value_global();
	// end lesley 0815

	// lesley 0821
	AI_face_ICM_blending_value_global(max_face_size);
	// end lesley 0821

	// lesley 0829_2, draw face rectangle from gdma
	for(tt=0; tt<6; tt++)
	{
		int factor_w = 0, factor_h = 0; // used to fit the gdma scale
		
		if(debug_face_info_mode == 0)
		{
			x = face_sharp_apply[buf_idx_w][tt].cx12;
			y = face_sharp_apply[buf_idx_w][tt].cy12;
			w = face_sharp_apply[buf_idx_w][tt].w12;
			h = face_sharp_apply[buf_idx_w][tt].h12;

			// color: 0x0000xxxx -> gbar
			if(tt==0)    c = 0xf000;//g
			else if(tt==1) c = 0x0f00;//b
			else if(tt==2) c = 0x000f;//r
			else if(tt==3) c = 0xff00;//cy
			else if(tt==4) c = 0xf00f;//ye
			else if(tt==5) c = 0x0f0f;//ma

			if(icm_global_en)
				c = c | ((AI_ICM_global_blend>>4)<<4);
			else
				c = c | ((AI_detect_value_blend[tt]>>4)<<4);

		}
		// lesley 0906_1
		else if(debug_face_info_mode == 1)
		{
			x = face_sharp_apply[buf_idx_w][tt].cx12;
			y = face_sharp_apply[buf_idx_w][tt].cy12;
			w = face_sharp_apply[buf_idx_w][tt].w12;
			h = face_sharp_apply[buf_idx_w][tt].h12;
			c = 0x0000ffff;

			if(draw_blend_en)
			{
				c = 0x0000ff0f;

				if(icm_global_en)
					c = c | ((AI_ICM_global_blend>>4)<<4);
				else
					c = c | ((AI_detect_value_blend[tt]>>4)<<4);
			}
		}
		// end lesley 0906_1
		else if(debug_face_info_mode == 2)
		{
			x = face_in[tt].cx12;
			y = face_in[tt].cy12;
			w = face_in[tt].w12;
			h = face_in[tt].h12;
			c = 0x0000ffff;
		}

		draw_tx = x - w/2;
		draw_tw = w;

		if(draw_tx < 0)
		{
			draw_tw = w + draw_tx;
			draw_tx = 0;
		}

		if(draw_tx + draw_tw > 3839)
		{
			draw_tw = 3839 - draw_tx;
		}

		draw_ty = y - h/2;
		draw_th = h;

		if(draw_ty < 0)
		{
			draw_th = h + draw_ty;
			draw_ty = 0;
		}

		if(draw_ty + draw_th > 2159)
		{
			draw_th = 2159 - draw_ty;
		}
		
		/* get gdma size by calling API */
		GDMA_AI_Get_UI_size(&gdma_w, &gdma_h);
		//rtd_pr_vpq_ai_emerg("dynamic gdma_w:%d, gdma_h:%d\n", gdma_w, gdma_h);
		
		if(gdma_w == 1920) factor_w = 2;
		else if(gdma_w == 960) factor_w = 4;
		else if(gdma_w == 480) factor_w = 8;
		if(gdma_h == 1080) factor_h = 2;
		else if(gdma_h == 540) factor_h = 4;
		else if(gdma_h == 270) factor_h = 8;
			
		face_demo_draw[buf_idx_w][tt].x   = (unsigned short)(draw_tx/factor_w);
		face_demo_draw[buf_idx_w][tt].y   = (unsigned short)(draw_ty/factor_h);
		face_demo_draw[buf_idx_w][tt].w   = (unsigned short)(draw_tw/factor_w);
		face_demo_draw[buf_idx_w][tt].h   = (unsigned short)(draw_th/factor_h);
		face_demo_draw[buf_idx_w][tt].color = c;

	}
	// end lesley 0829_2

	// lesley 0808
	if(ai_scene_change_count == ai_sc_count_th)
	{
		ai_scene_change_count = 0;
		ai_scene_change_done = 0;
	}
	// end lesley 0808

	// chen 0815_2
	if(scene_change_count==sc_count_th)
	{
		scene_change_count=0;
	}
	//end chen 0815_2


	frame_drop_count++;

	if(frame_drop_count>=frame_drop_num+0)
				frame_drop_count=0;
	
	buf_idx_w = (buf_idx_w+1)%(apply_buf_num);

}

void AI_face_octa_calculate(int width, int height, int range, int *dir, int *mode)
{
	// decide octa direction
	if(width > height)
		*dir = 1;
	else
		*dir = 0;


	// decide octa mode
	if(93*height<100*width)
		*mode = 0;
	else if((93*height>=100*width) && (4*height<5*width))
		*mode = 4;
	else if((27*height<40*width) && (4*height>=5*width))
		*mode = 3;
	else if((11*height<20*width) && (27*height>=40*width))
		*mode = 2;
	else if(11*height>=20*width)
		*mode = 1;

	// for demo
	//*dir = 0;
	//*mode = 2;
}
void AI_face_uvcenter_calculate(int cb_max, int cr_max, int cb_med, int cr_med, int *centerU, int *centerV)
{
	int  UDiff = abs(cb_max - cb_med);
	int  VDiff = abs(cr_max - cr_med);

	if(UDiff<50&&VDiff<100)
	{
		*centerU = cb_max;
		*centerV = cr_max;
	}
	else if(((UDiff+VDiff)>400)||(UDiff>300)||(VDiff>300)||((UDiff>100)&&(VDiff>100)))
	{
		*centerU = cb_med;
		*centerV = cr_med;
	}
	else
	{
		*centerU = (cb_med + cb_max)>>1;
		*centerV = (cr_med + cr_max)>>1;
	}
}

void AI_face_win_pos_predict(int faceIdx, AIInfo face)
{

	int	pos_x_tmp, pos_y_tmp;
	int w_avg, h_avg;
	int center_u;
	int center_v;
	int range;
	int dist_x_cur;
	int dist_y_cur;
	int win_center_x_cur=0;
	int win_center_y_cur=0;
	int win_face_h_cur=0;
	int win_face_w_cur=0;
	int	face_center_u_cur=0;
	int	face_center_v_cur=0;
	int	face_range_cur=0;
	int dist_x_pre;
	int dist_y_pre;
	int pos_x_tmp_mod;
	int pos_y_tmp_mod;
	int w_avg_mod;
	int h_avg_mod;
	int center_u_mod;
	int center_v_mod;
	int range_mod;
	int face_center_u=0;
	int face_center_v=0;
	// henry 0612
	int octa_dir = 0;
	int octa_mode = 0;
	// end henry 0612

// chen 0429
	int iir_weight;
	int iir_weight2;

	int frame_drop_num;
	int frame_delay;

	// chen 0808
	int ii;
	//end chen 0808

	// lesley 0829
	int keep_still_mode;
	int ratio;
	// lesley 0829

	// setting //

	iir_weight=ai_ctrl.ai_global.iir_weight;
	iir_weight2=ai_ctrl.ai_global.iir_weight2;

	frame_drop_num=ai_ctrl.ai_global.frame_drop_num;
	frame_delay=ai_ctrl.ai_global.frame_delay;
	// end chen 0429

	// lesley 0829
	keep_still_mode = ai_ctrl.ai_global3.keep_still_mode;
	// lesley 0829


	face_center_u= face.cb_med12 ;
	face_center_v= face.cr_med12 ;

	////IIR
	if(scene_change_flag==1)
	{
		iir_weight=0;
		iir_weight2=0;
	}


/////////////// calculate face coordinate
	if(face.pv8!=0)
	{
		win_center_x_cur=face.cx12;
		win_center_y_cur=face.cy12;
		win_face_w_cur=face.w12;
		win_face_h_cur=face.h12;
		face_center_u_cur=face_center_u;
		face_center_v_cur=face_center_v;
		face_range_cur=face.range12;

		// lesley 0928
		if(ai_ctrl.ai_global3.IOU2_mode == 1)
		{
			int range_ratio = ai_ctrl.ai_global3.IOU2_mode1_range_ratio;
			face_range_cur=(face.w12>face.h12)?((face.w12+range_ratio*face.h12/16)/2):((face.h12+range_ratio*face.w12/16)/2);
		}
		// end lesley 0928
	}
	else
	{
		win_center_x_cur=0;
		win_center_y_cur=0;
		win_face_h_cur=0;
		win_face_w_cur=0;
		face_center_u_cur=0;
		face_center_v_cur=0;
		face_range_cur=0;
	}

	// lesley 0829
	if(keep_still_mode)
	{
		// lesley 0906_1
		#if 0 // mac7p AI TBD memc
		extern unsigned char MEMC_Lib_GetInfo(unsigned char infoSel, unsigned char x1, unsigned char x2, unsigned char y1, unsigned char y2);
		#endif
		extern unsigned char scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(void);
		_clues* SmartPic_clue=NULL;
		int x1, x2, y1, y2;
		int ratio_step = 32;
		#if 0 // mac7p AI TBD memc
		int still_ratio_th3 = ai_ctrl.ai_global3.still_ratio_th3;
		#endif
		int still_ratio_clamp = ai_ctrl.ai_global3.still_ratio_clamp;

		x1 = (win_center_x_cur-win_face_w_cur/2)/480;
		x2 = (win_center_x_cur+win_face_w_cur/2)/480;
		y1 = (win_center_y_cur-win_face_h_cur/2)/540;
		y2 = (win_center_y_cur+win_face_h_cur/2)/540;
		if(x1 < 0) x1 = 0;
		if(x2 > 7) x2 = 7;
		if(y1 < 0) y1 = 0;
		if(y2 > 3) y2 = 3;
		
		#if 0 // mac7p AI TBD memc
		ratio = MAX(MEMC_Lib_GetInfo(2,x1,x2,y1,y2), MEMC_Lib_GetInfo(3,x1,x2,y1,y2));

		if(ratio < still_ratio_th3)
			still_ratio[2] = 32;
		else
			still_ratio[2] = 32 + still_ratio_th3 - ratio;
		#endif

		ratio = still_ratio[keep_still_mode-1]-still_ratio_clamp;

		if(ratio<0) ratio = 0;
		else if(ratio>32) ratio = 32;

		SmartPic_clue = scaler_GetShare_Memory_SmartPic_Clue_Struct();

		if(scene_change_flag==1 || (AI_detect_value_blend[faceIdx]==0) || (face.pv8==0))
			ratio = 0;
		// end lesley 0906_1

		win_center_x_cur = ((ratio_step - ratio) * win_center_x_cur + ratio * face_iir_pre[faceIdx].cx)/ratio_step;
		win_center_y_cur = ((ratio_step - ratio) * win_center_y_cur + ratio * face_iir_pre[faceIdx].cy)/ratio_step;
		win_face_h_cur = ((ratio_step - ratio) * win_face_h_cur + ratio * face_iir_pre[faceIdx].h)/ratio_step;
		win_face_w_cur = ((ratio_step - ratio) * win_face_w_cur + ratio * face_iir_pre[faceIdx].w)/ratio_step;
		// lesley 0928
		face_range_cur = ((ratio_step - ratio) * face_range_cur + ratio * face_iir_pre[faceIdx].range)/ratio_step;
		// end lesley 0928
		//face_center_u_cur = ((ratio_step - ratio) * face_center_u_cur + ratio * face_iir_pre[faceIdx].center_u)/ratio_step;
		//face_center_v_cur = ((ratio_step - ratio) * face_center_v_cur + ratio * face_iir_pre[faceIdx].center_v)/ratio_step;
		//face_range_cur = ((ratio_step - ratio) * face_range_cur + ratio  *face_iir_pre[faceIdx].range)/ratio_step;


	}
	// end lesley 0829


	// dist_cur ..................
	if(AI_detect_value_blend[faceIdx]==0)
	{
		// center_pos ..............
		pos_x_tmp=win_center_x_cur;
		pos_y_tmp=win_center_y_cur;

		// window size..................
		w_avg=win_face_w_cur;
		h_avg=win_face_h_cur;

		h_diff_pre[faceIdx]=0;
		w_diff_pre[faceIdx]=0;

		// center_uv, range ......
		center_u=face_center_u_cur;
		center_v=face_center_v_cur;
		range=face_range_cur;


		//////////// IIR ////////////////
		// dist .....................
		face_iir_pre[faceIdx].dist_x=1000;
		face_iir_pre[faceIdx].dist_y=1000;

		// center_pos ..............
		face_iir_pre[faceIdx].cx=pos_x_tmp;
		face_iir_pre[faceIdx].cy=pos_y_tmp;

		face_iir_pre2[faceIdx].cx=pos_x_tmp;
		face_iir_pre2[faceIdx].cy=pos_y_tmp;

		// chen 0503
		face_iir_pre3[faceIdx].cx=pos_x_tmp;
		face_iir_pre3[faceIdx].cy=pos_y_tmp;
		face_iir_pre4[faceIdx].cx=pos_x_tmp;
		face_iir_pre4[faceIdx].cy=pos_y_tmp;

		// chen 0703
		face_iir_pre5[faceIdx].cx=pos_x_tmp;
		face_iir_pre5[faceIdx].cy=pos_y_tmp;

		// window size..................
		face_iir_pre[faceIdx].h=h_avg;
		face_iir_pre[faceIdx].w=w_avg;

		// center_uv, range ......
		face_iir_pre[faceIdx].center_u=center_u;
		face_iir_pre[faceIdx].center_v=center_v;
		face_iir_pre[faceIdx].range=range;

		pos_x_tmp_mod=pos_x_tmp;
		pos_y_tmp_mod=pos_y_tmp;
	}
	else
	{
		// dist ...............
		if(face.pv8==0)
		{
			if(face_iir_pre[faceIdx].dist_x==1000)
			{
				dist_x_cur=0;
				dist_y_cur=0;
			}
			else
			{
				dist_x_cur=face_iir_pre[faceIdx].dist_x;
				dist_y_cur=face_iir_pre[faceIdx].dist_y;
			}
		}
		else
		{
			if(frame_drop_num==2)
			{
				dist_x_cur=win_center_x_cur-face_iir_pre2[faceIdx].cx;
				dist_y_cur=win_center_y_cur-face_iir_pre2[faceIdx].cy;
			}
			// chen 0503
			else if(frame_drop_num==3)
			{
				dist_x_cur=win_center_x_cur-face_iir_pre3[faceIdx].cx;
				dist_y_cur=win_center_y_cur-face_iir_pre3[faceIdx].cy;
			}
			else if(frame_drop_num==4)
			{
				dist_x_cur=win_center_x_cur-face_iir_pre4[faceIdx].cx;
				dist_y_cur=win_center_y_cur-face_iir_pre4[faceIdx].cy;
			}
			// chen 0703
			else if(frame_drop_num==5)
			{
				dist_x_cur=win_center_x_cur-face_iir_pre5[faceIdx].cx;
				dist_y_cur=win_center_y_cur-face_iir_pre5[faceIdx].cy;
			}
			else
			{
				dist_x_cur=win_center_x_cur-face_iir_pre[faceIdx].cx;
				dist_y_cur=win_center_y_cur-face_iir_pre[faceIdx].cy;
			}
		}

		dist_x_pre=face_iir_pre[faceIdx].dist_x;
		dist_y_pre=face_iir_pre[faceIdx].dist_y;


		if(face_iir_pre[faceIdx].dist_x==1000)// || frame_drop_count==1)
		{
			face_iir_pre[faceIdx].dist_x=dist_x_cur;
			face_iir_pre[faceIdx].dist_y=dist_y_cur;
		}
		else
		{
			face_iir_pre[faceIdx].dist_x=((16-iir_weight)*dist_x_cur+iir_weight*face_iir_pre[faceIdx].dist_x)/16;
			face_iir_pre[faceIdx].dist_y=((16-iir_weight)*dist_y_cur+iir_weight*face_iir_pre[faceIdx].dist_y)/16;
		}


		// center_pos .................
		pos_x_tmp=face_iir_pre[faceIdx].cx+face_iir_pre[faceIdx].dist_x/frame_drop_num;
		pos_y_tmp=face_iir_pre[faceIdx].cy+face_iir_pre[faceIdx].dist_y/frame_drop_num;


		if(frame_drop_count==0 && frame_drop_num==2)
		{
			pos_x_tmp=face_iir_pre2[faceIdx].cx+face_iir_pre[faceIdx].dist_x;
			pos_y_tmp=face_iir_pre2[faceIdx].cy+face_iir_pre[faceIdx].dist_y;
		}
		// chen 0503
		else if(frame_drop_count==0 && frame_drop_num==3)
		{
			pos_x_tmp=face_iir_pre3[faceIdx].cx+face_iir_pre[faceIdx].dist_x;
			pos_y_tmp=face_iir_pre3[faceIdx].cy+face_iir_pre[faceIdx].dist_y;
		}
		else if(frame_drop_count==0 && frame_drop_num==4)
		{
			pos_x_tmp=face_iir_pre4[faceIdx].cx+face_iir_pre[faceIdx].dist_x;
			pos_y_tmp=face_iir_pre4[faceIdx].cy+face_iir_pre[faceIdx].dist_y;
		}
		else if(frame_drop_count==0 && frame_drop_num==5)
		{
			pos_x_tmp=face_iir_pre5[faceIdx].cx+face_iir_pre[faceIdx].dist_x;
			pos_y_tmp=face_iir_pre5[faceIdx].cy+face_iir_pre[faceIdx].dist_y;
		}

		// chen 0703
		face_iir_pre5[faceIdx].cx=face_iir_pre4[faceIdx].cx;
		face_iir_pre5[faceIdx].cy=face_iir_pre4[faceIdx].cy;


		face_iir_pre4[faceIdx].cx=face_iir_pre3[faceIdx].cx;
		face_iir_pre4[faceIdx].cy=face_iir_pre3[faceIdx].cy;
		face_iir_pre3[faceIdx].cx=face_iir_pre2[faceIdx].cx;
		face_iir_pre3[faceIdx].cy=face_iir_pre2[faceIdx].cy;


		face_iir_pre2[faceIdx].cx=face_iir_pre[faceIdx].cx;
		face_iir_pre2[faceIdx].cy=face_iir_pre[faceIdx].cy;


		face_iir_pre[faceIdx].cx=pos_x_tmp;
		face_iir_pre[faceIdx].cy=pos_y_tmp;


		//// // chen 0409
		if(frame_drop_count==0 && face.pv8==0)
		{
			face_info_pre[faceIdx].cx12=pos_x_tmp;
			face_info_pre[faceIdx].cy12=pos_y_tmp;
		}


		// window_size ...............
		if(face.pv8==0)
		{
			w_avg=face_iir_pre[faceIdx].w;
			h_avg=face_iir_pre[faceIdx].h;

		//	w_avg=face_iir_pre[faceIdx].w+1*w_diff_pre[faceIdx]/frame_drop_num;
		//	h_avg=face_iir_pre[faceIdx].h+1*h_diff_pre[faceIdx]/frame_drop_num;
		}
		else
		{
			w_avg=(iir_weight2*face_iir_pre[faceIdx].w+(16-iir_weight2)*win_face_w_cur+8)/16;
			h_avg=(iir_weight2*face_iir_pre[faceIdx].h+(16-iir_weight2)*win_face_h_cur+8)/16;

			h_diff_pre[faceIdx]=h_avg-face_iir_pre[faceIdx].h;
			w_diff_pre[faceIdx]=w_avg-face_iir_pre[faceIdx].w;
		}

		face_iir_pre[faceIdx].h=h_avg;
		face_iir_pre[faceIdx].w=w_avg;


		// center_uv, range ......
		if(face.pv8==0)
		{
			center_u=face_iir_pre[faceIdx].center_u;
			center_v=face_iir_pre[faceIdx].center_v;
			range=face_iir_pre[faceIdx].range;
		}
		else
		{
		//	center_u=face_center_u_cur;
		//	center_v=face_center_v_cur;
		//	range=face_range_cur;

			center_u=(iir_weight2*face_iir_pre[faceIdx].center_u+(16-iir_weight2)*face_center_u_cur+8)/16;
			center_v=(iir_weight2*face_iir_pre[faceIdx].center_v+(16-iir_weight2)*face_center_v_cur+8)/16;
			range=(iir_weight2*face_iir_pre[faceIdx].range+(16-iir_weight2)*face_range_cur+8)/16;

		}
		face_iir_pre[faceIdx].center_u=center_u;
		face_iir_pre[faceIdx].center_v=center_v;
		face_iir_pre[faceIdx].range=range;
	}

	// .... range_calculate
/*
	if(w_avg>=h_avg)
		range=(w_avg+h_avg/2)/2;
	else
		range=(h_avg+w_avg/2)/2;
*/

	if(face_iir_pre[faceIdx].dist_x==1000)
	{
		pos_x_tmp_mod=pos_x_tmp;
		pos_y_tmp_mod=pos_y_tmp;
	}
	else
	{
		pos_x_tmp_mod=pos_x_tmp+frame_delay*face_iir_pre[faceIdx].dist_x/frame_drop_num;
		pos_y_tmp_mod=pos_y_tmp+frame_delay*face_iir_pre[faceIdx].dist_y/frame_drop_num;
	}

	w_avg_mod=w_avg;//+0*frame_delay*w_diff_pre[faceIdx]/frame_drop_num;
	h_avg_mod=h_avg;//+0*frame_delay*h_diff_pre[faceIdx]/frame_drop_num;
	center_u_mod=center_u;
	center_v_mod=center_v;
	range_mod=range;

	// chen 0808
	for (ii=19;ii>0;ii--)
	{
		face_dist_x[faceIdx][ii]=face_dist_x[faceIdx][ii-1];
		face_dist_y[faceIdx][ii]=face_dist_y[faceIdx][ii-1];
	}
	face_dist_x[faceIdx][0]=face_iir_pre[faceIdx].dist_x;
	face_dist_y[faceIdx][0]=face_iir_pre[faceIdx].dist_y;
	// end chen 0808

	// henry 0612
	AI_face_octa_calculate(w_avg_mod, h_avg_mod, range_mod, &octa_dir, &octa_mode);
	//if(rtd_inl(0xb802e4f0)==3) rtd_pr_vpq_ai_emerg("[Henry] octa_dir=%d, octa_mode=%d\n",octa_dir, octa_mode);
	// end henry 0612

	face_icm_apply[buf_idx_w][faceIdx].pos_x_s=pos_x_tmp_mod;
	face_icm_apply[buf_idx_w][faceIdx].pos_y_s=pos_y_tmp_mod;
	face_icm_apply[buf_idx_w][faceIdx].center_u_s=center_u_mod>>data_shift;
	face_icm_apply[buf_idx_w][faceIdx].center_v_s=center_v_mod>>data_shift;
	face_icm_apply[buf_idx_w][faceIdx].range_s=range_mod;
	// mac7p lesley 1211
	face_icm_apply[buf_idx_w][faceIdx].w12=w_avg_mod;
	face_icm_apply[buf_idx_w][faceIdx].h12=h_avg_mod;
	// end mac7p lesley 1211

	// chen 0429
	face_dcc_apply[buf_idx_w][faceIdx].pos_x_s=pos_x_tmp_mod;
	face_dcc_apply[buf_idx_w][faceIdx].pos_y_s=pos_y_tmp_mod;
	face_dcc_apply[buf_idx_w][faceIdx].center_u_s=center_u_mod>>data_shift;
	face_dcc_apply[buf_idx_w][faceIdx].center_v_s=center_v_mod>>data_shift;
	face_dcc_apply[buf_idx_w][faceIdx].range_s=range_mod;
	// mac7p lesley 1211
	face_dcc_apply[buf_idx_w][faceIdx].w12=w_avg_mod;
	face_dcc_apply[buf_idx_w][faceIdx].h12=h_avg_mod;
	// end mac7p lesley 1211

	// end chen 0429

	// chen 0527 ... sharpness face info
	face_sharp_apply[buf_idx_w][faceIdx].cx12    =pos_x_tmp_mod;
	face_sharp_apply[buf_idx_w][faceIdx].cy12    =pos_y_tmp_mod;
	face_sharp_apply[buf_idx_w][faceIdx].w12     =w_avg_mod;
	face_sharp_apply[buf_idx_w][faceIdx].h12     =h_avg_mod;
	face_sharp_apply[buf_idx_w][faceIdx].range12 =range_mod;
	face_sharp_apply[buf_idx_w][faceIdx].cb_med12=center_u_mod;
	face_sharp_apply[buf_idx_w][faceIdx].cr_med12=center_v_mod;
	face_sharp_apply[buf_idx_w][faceIdx].cb_var12=48;
	face_sharp_apply[buf_idx_w][faceIdx].cr_var12=48;// currently no info


	//rtd_pr_vpq_ai_info("[%d] win_pos original: x:%d, y:%d\n", faceIdx, win_center_x_cur, win_center_y_cur);
	//rtd_pr_vpq_ai_info("[%d] win_pos predict: x:%d, y:%d\n", faceIdx, pos_x_tmp_mod, pos_y_tmp_mod);
	//end chen 0527 ... sharpness face info
}


void AI_face_ICM_blending_value(int faceIdx, AIInfo face)
{
	int value_diff;
	int change_speed;
	int change_speed2;
	int change_speed3;
	int icm_ai_ori[5];

// chen 0524
	int icm_uv_ori[8];
//end chen 0524

	int IOU_value;
	int size_value;
	int change_speed0,change_speed1;


	// chen 0429
	int change_speed_temp;
	int change_speed22;
	int IOU_value2;


	int d_change_speed_default;
	int change_speed_default;

	// disappear
	int val_diff_loth;
	int d_change_speed_val_loth;
	int d_change_speed_val_hith;
	int d_change_speed_val_slope;
	int IOU_diff_loth;
	int d_change_speed_IOU_loth;
	int d_change_speed_IOU_hith;
	int d_change_speed_IOU_slope;
	int IOU_diff_loth2;
	int d_change_speed_IOU_loth2;
	int d_change_speed_IOU_hith2;
	int d_change_speed_IOU_slope2;
	int size_diff_loth;
	int d_change_speed_size_loth;
	int d_change_speed_size_hith;
	int d_change_speed_size_slope;

	// appear //////
	int val_diff_loth_a;
	int d_change_speed_val_loth_a;
	int d_change_speed_val_hith_a;
	int d_change_speed_val_slope_a;
	int IOU_diff_loth_a;
	int d_change_speed_IOU_loth_a;
	int d_change_speed_IOU_hith_a;
	int d_change_speed_IOU_slope_a;
	int IOU_diff_loth_a2;
	int d_change_speed_IOU_loth_a2;
	int d_change_speed_IOU_hith_a2;
	int d_change_speed_IOU_slope_a2;
	int size_diff_loth_a;
	int d_change_speed_size_loth_a;
	int d_change_speed_size_hith_a;
	int d_change_speed_size_slope_a;

	// chen 0808
	int sum_face_dist_x[6]={0};
	int sum_face_dist_y[6]={0};
	int ii=0;
	int dist_ratio_x=0;
	int dist_ratio_y=0;
	int dist_ratio_inv=0;

	int IOU_select=0;
	int sum_count_num=10; // max=19
	//end chen 0808

	// lesley 0820_2
	int IOU_decay_en;
	// end lesley 0820_2

	// lesley 0823
	int blend_size_en = 0;
	int blend_size_hith = 0;
	int blend_size_loth = 0;
	int value_blend_size = 0;
	// end lesley 0823

	// setting //
	d_change_speed_default=ai_ctrl.ai_icm_blend.d_change_speed_default;
	change_speed_default=ai_ctrl.ai_icm_blend.change_speed_default;

	// disappear //////
	val_diff_loth=ai_ctrl.ai_icm_blend.val_diff_loth;
	d_change_speed_val_loth=ai_ctrl.ai_icm_blend.d_change_speed_val_loth;
	d_change_speed_val_hith=ai_ctrl.ai_icm_blend.d_change_speed_val_hith;
	d_change_speed_val_slope=ai_ctrl.ai_icm_blend.d_change_speed_val_slope;

	IOU_diff_loth=ai_ctrl.ai_icm_blend.IOU_diff_loth;//25;
	d_change_speed_IOU_loth=ai_ctrl.ai_icm_blend.d_change_speed_IOU_loth;
	d_change_speed_IOU_hith=ai_ctrl.ai_icm_blend.d_change_speed_IOU_hith;//-50
	d_change_speed_IOU_slope=ai_ctrl.ai_icm_blend.d_change_speed_IOU_slope;

	IOU_diff_loth2=ai_ctrl.ai_icm_blend.IOU_diff_loth2;//70;//25;
	d_change_speed_IOU_loth2=ai_ctrl.ai_icm_blend.d_change_speed_IOU_loth2;//-50;//0;
	d_change_speed_IOU_hith2=ai_ctrl.ai_icm_blend.d_change_speed_IOU_hith2;//-50; //-50
	d_change_speed_IOU_slope2=ai_ctrl.ai_icm_blend.d_change_speed_IOU_slope2;//-2;

	size_diff_loth=ai_ctrl.ai_icm_blend.size_diff_loth;
	d_change_speed_size_loth=ai_ctrl.ai_icm_blend.d_change_speed_size_loth;
	d_change_speed_size_hith=ai_ctrl.ai_icm_blend.d_change_speed_size_hith;
	d_change_speed_size_slope=ai_ctrl.ai_icm_blend.d_change_speed_size_slope;

	// appear //////
	val_diff_loth_a=ai_ctrl.ai_icm_blend.val_diff_loth_a;
	d_change_speed_val_loth_a=ai_ctrl.ai_icm_blend.d_change_speed_val_loth_a;
	d_change_speed_val_hith_a=ai_ctrl.ai_icm_blend.d_change_speed_val_hith_a;
	d_change_speed_val_slope_a=ai_ctrl.ai_icm_blend.d_change_speed_val_slope_a;

	IOU_diff_loth_a=ai_ctrl.ai_icm_blend.IOU_diff_loth_a;//25;
	d_change_speed_IOU_loth_a=ai_ctrl.ai_icm_blend.d_change_speed_IOU_loth_a;
	d_change_speed_IOU_hith_a=ai_ctrl.ai_icm_blend.d_change_speed_IOU_hith_a; //-50
	d_change_speed_IOU_slope_a=ai_ctrl.ai_icm_blend.d_change_speed_IOU_slope_a;

	IOU_diff_loth_a2=ai_ctrl.ai_icm_blend.IOU_diff_loth_a2;//50;//25;
	d_change_speed_IOU_loth_a2=ai_ctrl.ai_icm_blend.d_change_speed_IOU_loth_a2;//-50;
	d_change_speed_IOU_hith_a2=ai_ctrl.ai_icm_blend.d_change_speed_IOU_hith_a2;//-50
	d_change_speed_IOU_slope_a2=ai_ctrl.ai_icm_blend.d_change_speed_IOU_slope_a2;

	size_diff_loth_a=ai_ctrl.ai_icm_blend.size_diff_loth_a;
	d_change_speed_size_loth_a=ai_ctrl.ai_icm_blend.d_change_speed_size_loth_a;
	d_change_speed_size_hith_a=ai_ctrl.ai_icm_blend.d_change_speed_size_hith_a;
	d_change_speed_size_slope_a=ai_ctrl.ai_icm_blend.d_change_speed_size_slope_a;
	// end setting //
	// end chen 0429


	icm_ai_ori[0]=ai_ctrl.ai_global.icm_ai_blend_inside_ratio;
	icm_ai_ori[1]=ai_ctrl.ai_global.icm_ai_blend_ratio0;
	icm_ai_ori[2]=ai_ctrl.ai_global.icm_ai_blend_ratio1;
	icm_ai_ori[3]=ai_ctrl.ai_global.icm_ai_blend_ratio2;
	icm_ai_ori[4]=ai_ctrl.ai_global.icm_ai_blend_ratio3;

	// chen 0524
	icm_uv_ori[0]=ai_ctrl.ai_global.icm_uv_blend_ratio0;
	icm_uv_ori[1]=ai_ctrl.ai_global.icm_uv_blend_ratio1;
	icm_uv_ori[2]=ai_ctrl.ai_global.icm_uv_blend_ratio2;
	icm_uv_ori[3]=ai_ctrl.ai_global.icm_uv_blend_ratio3;
	icm_uv_ori[4]=ai_ctrl.ai_global.icm_uv_blend_ratio4;
	icm_uv_ori[5]=ai_ctrl.ai_global.icm_uv_blend_ratio5;
	icm_uv_ori[6]=ai_ctrl.ai_global.icm_uv_blend_ratio6;
	icm_uv_ori[7]=ai_ctrl.ai_global.icm_uv_blend_ratio7;
	//end chen 0524

	// lesley 0820_2
	IOU_decay_en = ai_ctrl.ai_global3.IOU_decay_en;
	// end lesley 0820_2

	// lesley 0823
	blend_size_en = ai_ctrl.ai_global3.blend_size_en;
	blend_size_hith = ai_ctrl.ai_global3.blend_size_hith;
	blend_size_loth = ai_ctrl.ai_global3.blend_size_loth;
	// end lesley 0823

	// chen 0808
	IOU_select = ai_ctrl.ai_global3.IOU_select;
	sum_count_num = ai_ctrl.ai_global3.sum_count_num;
	sum_face_dist_x[faceIdx]=0;
	sum_face_dist_y[faceIdx]=0;

	for (ii=0; ii<sum_count_num; ii++)
	{
		sum_face_dist_x[faceIdx]=sum_face_dist_x[faceIdx]+face_dist_x[faceIdx][ii];
		sum_face_dist_y[faceIdx]=sum_face_dist_y[faceIdx]+face_dist_y[faceIdx][ii];
	}

	if(face_iir_pre[faceIdx].range>0)
		dist_ratio_x=abs(sum_face_dist_x[faceIdx])*100/face_iir_pre[faceIdx].range;
	else
		dist_ratio_x=100;

	if(face_iir_pre[faceIdx].range>0)
		dist_ratio_y=abs(sum_face_dist_y[faceIdx])*100/face_iir_pre[faceIdx].range;
	else
		dist_ratio_y=100;

	dist_ratio_inv=100-MAX(dist_ratio_x,dist_ratio_y);

	if(dist_ratio_inv<0)
		dist_ratio_inv=0;
	if(dist_ratio_inv>100)
		dist_ratio_inv=100;

	//end chen 0808

	if(face.pv8==0)
	{
		value_diff=value_diff_pre[faceIdx];

		change_speed0=d_change_speed_default+change_speed_ai_sc;

		if(value_diff<=val_diff_loth)
			change_speed1=d_change_speed_val_loth;
		else
		{
			change_speed1=d_change_speed_val_loth+d_change_speed_val_slope*(value_diff-val_diff_loth);

			if(change_speed1<d_change_speed_val_hith)
				change_speed1=d_change_speed_val_hith;
		}

		IOU_value=IOU_pre[faceIdx];

		// chen 0808
		if(IOU_select==1 && IOU_decay_en==0)
			IOU_value=dist_ratio_inv;
		//end chen 0808

		if(IOU_value>=IOU_diff_loth)
			change_speed2=d_change_speed_IOU_loth;
		else
		{
			change_speed2=d_change_speed_IOU_loth+d_change_speed_IOU_slope*(IOU_diff_loth-IOU_value);

			if(change_speed2<d_change_speed_IOU_hith)
				change_speed2=d_change_speed_IOU_hith;
		}

		// chen 0429
		IOU_value2=IOU_pre_max2[faceIdx];
		if(IOU_value2<=IOU_diff_loth2)
			change_speed22=d_change_speed_IOU_loth2;
		else
		{
			change_speed22=d_change_speed_IOU_loth2+d_change_speed_IOU_slope2*(IOU_value2-IOU_diff_loth2)/8;

			if(change_speed22<d_change_speed_IOU_hith2)
				change_speed22=d_change_speed_IOU_hith2;
		}
		change_speed2=change_speed2+change_speed22;
		// end chen 0429


		size_value=face.w12;
		if(size_value>=size_diff_loth)
			change_speed3=d_change_speed_size_loth;
		else
		{
			change_speed3=d_change_speed_size_loth+d_change_speed_size_slope*(size_diff_loth-size_value)/32;

			if(change_speed3<d_change_speed_size_hith)
				change_speed3=d_change_speed_size_hith;
		}
	}
	else
	{
		if(AI_detect_value_blend[faceIdx]>0)
		{
			value_diff=abs(face.val_med12-face_info_pre[faceIdx].val_med12)/16;

		}else
			value_diff=0;

		value_diff_pre[faceIdx]=value_diff;

		change_speed0=change_speed_default+change_speed_ai_sc;

		if(value_diff<=val_diff_loth_a)
			change_speed1=d_change_speed_val_loth_a;
		else
		{
			change_speed1=d_change_speed_val_loth_a+d_change_speed_val_slope_a*(value_diff-val_diff_loth_a);

			if(change_speed1<d_change_speed_val_hith_a)
				change_speed1=d_change_speed_val_hith_a;
		}

		IOU_value=IOU_pre[faceIdx];

		// chen 0808
		if(IOU_select==1)
			IOU_value=dist_ratio_inv;
		//end chen 0808


		if(IOU_value>=IOU_diff_loth_a)
			change_speed2=d_change_speed_IOU_loth_a;
		else
		{
			change_speed2=d_change_speed_IOU_loth_a+d_change_speed_IOU_slope_a*(IOU_diff_loth_a-IOU_value);

			if(change_speed2<d_change_speed_IOU_hith_a)
				change_speed2=d_change_speed_IOU_hith_a;
		}

		// chen 0429
		IOU_value2=IOU_pre_max2[faceIdx];
		if(IOU_value2<=IOU_diff_loth_a2)
			change_speed22=d_change_speed_IOU_loth_a2;
		else
		{
			change_speed22=d_change_speed_IOU_loth_a2+d_change_speed_IOU_slope_a2*(IOU_value2-IOU_diff_loth_a2)/8;

			if(change_speed22<d_change_speed_IOU_hith_a2)
				change_speed22=d_change_speed_IOU_hith_a2;
		}
		change_speed2=change_speed2+change_speed22;
		// end chen 0429


		size_value=face.w12;
		if(size_value>=size_diff_loth_a)
			change_speed3=d_change_speed_size_loth_a;
		else
		{
			change_speed3=d_change_speed_size_loth_a+d_change_speed_size_slope_a*(size_diff_loth_a-size_value)/32;

			if(change_speed3<d_change_speed_size_hith_a)
				change_speed3=d_change_speed_size_hith_a;
		}
	}
	change_speed=change_speed0+change_speed1+change_speed3;
//change_speed=MAX((change_speed+change_speed2),MIN(change_speed,1));

	// chen 0429
	change_speed_temp=change_speed+change_speed2;
  //end chen 0429

	if(frame_drop_count==0)
	{
		//change_speed_t[faceIdx]=change_speed;
		// chen 0429
		change_speed_t[faceIdx]=MAX(change_speed_temp,
			MIN(change_speed,(1-AI_detect_value_blend[faceIdx])));
		//end chen 0429

	}
	if(AI_detect_value_blend[faceIdx]<=1)
	{
		if(change_speed_t[faceIdx]>0)
			change_speed_t[faceIdx]=1;
	}

	AI_detect_value_blend[faceIdx]=AI_detect_value_blend[faceIdx]+change_speed_t[faceIdx];

	if(AI_detect_value_blend[faceIdx]<0)
		AI_detect_value_blend[faceIdx]=0;

	if(AI_detect_value_blend[faceIdx]>255)
		AI_detect_value_blend[faceIdx]=255;


	if(scene_change_flag==1)
	{
		AI_detect_value_blend[faceIdx]=0;
	}


	if(blend_size_en)
	{
		value_blend_size = AI_detect_value_blend[faceIdx] * (face.w12 - blend_size_loth) / (blend_size_hith - blend_size_loth);

		if(value_blend_size > AI_detect_value_blend[faceIdx])
			value_blend_size = AI_detect_value_blend[faceIdx];

		if(value_blend_size < 0)
			value_blend_size = 0;


		AI_detect_value_blend[faceIdx] = value_blend_size;
	}

	face_icm_apply[buf_idx_w][faceIdx].ai_blending_inside_s	=(icm_ai_ori[0]*AI_detect_value_blend[faceIdx]+64*(255-AI_detect_value_blend[faceIdx]))/255;
	face_icm_apply[buf_idx_w][faceIdx].ai_blending_0_s	=(icm_ai_ori[1]*AI_detect_value_blend[faceIdx]+64*(255-AI_detect_value_blend[faceIdx]))/255;
	face_icm_apply[buf_idx_w][faceIdx].ai_blending_1_s	=(icm_ai_ori[2]*AI_detect_value_blend[faceIdx]+64*(255-AI_detect_value_blend[faceIdx]))/255;
	face_icm_apply[buf_idx_w][faceIdx].ai_blending_2_s	=(icm_ai_ori[3]*AI_detect_value_blend[faceIdx]+64*(255-AI_detect_value_blend[faceIdx]))/255;
	face_icm_apply[buf_idx_w][faceIdx].ai_blending_3_s	=(icm_ai_ori[4]*AI_detect_value_blend[faceIdx]+64*(255-AI_detect_value_blend[faceIdx]))/255;

// chen 0524
	face_icm_apply[buf_idx_w][faceIdx].uv_blending_0	=(icm_uv_ori[0]*AI_detect_value_blend[faceIdx]+64*(255-AI_detect_value_blend[faceIdx]))/255;
	face_icm_apply[buf_idx_w][faceIdx].uv_blending_1	=(icm_uv_ori[1]*AI_detect_value_blend[faceIdx]+64*(255-AI_detect_value_blend[faceIdx]))/255;
	face_icm_apply[buf_idx_w][faceIdx].uv_blending_2	=(icm_uv_ori[2]*AI_detect_value_blend[faceIdx]+64*(255-AI_detect_value_blend[faceIdx]))/255;
	face_icm_apply[buf_idx_w][faceIdx].uv_blending_3	=(icm_uv_ori[3]*AI_detect_value_blend[faceIdx]+64*(255-AI_detect_value_blend[faceIdx]))/255;
	face_icm_apply[buf_idx_w][faceIdx].uv_blending_4	=(icm_uv_ori[4]*AI_detect_value_blend[faceIdx]+64*(255-AI_detect_value_blend[faceIdx]))/255;
	face_icm_apply[buf_idx_w][faceIdx].uv_blending_5	=(icm_uv_ori[5]*AI_detect_value_blend[faceIdx]+64*(255-AI_detect_value_blend[faceIdx]))/255;
	face_icm_apply[buf_idx_w][faceIdx].uv_blending_6	=(icm_uv_ori[6]*AI_detect_value_blend[faceIdx]+64*(255-AI_detect_value_blend[faceIdx]))/255;
	face_icm_apply[buf_idx_w][faceIdx].uv_blending_7	=(icm_uv_ori[7]*AI_detect_value_blend[faceIdx]+64*(255-AI_detect_value_blend[faceIdx]))/255;

	if(face_icm_apply[buf_idx_w][faceIdx].uv_blending_0>63){
		face_icm_apply[buf_idx_w][faceIdx].uv_blending_0=63;
	}
	if(face_icm_apply[buf_idx_w][faceIdx].uv_blending_1>63){
		face_icm_apply[buf_idx_w][faceIdx].uv_blending_1=63;
        }
	if(face_icm_apply[buf_idx_w][faceIdx].uv_blending_2>63){
		face_icm_apply[buf_idx_w][faceIdx].uv_blending_2=63;
	}
        if(face_icm_apply[buf_idx_w][faceIdx].uv_blending_3>63){
		face_icm_apply[buf_idx_w][faceIdx].uv_blending_3=63;
	}
        if(face_icm_apply[buf_idx_w][faceIdx].uv_blending_4>63){
		face_icm_apply[buf_idx_w][faceIdx].uv_blending_4=63;
	}
        if(face_icm_apply[buf_idx_w][faceIdx].uv_blending_5>63){
		face_icm_apply[buf_idx_w][faceIdx].uv_blending_5=63;
	}
        if(face_icm_apply[buf_idx_w][faceIdx].uv_blending_6>63){
		face_icm_apply[buf_idx_w][faceIdx].uv_blending_6=63;
	}
        if(face_icm_apply[buf_idx_w][faceIdx].uv_blending_7>63){
		face_icm_apply[buf_idx_w][faceIdx].uv_blending_7=63;
        }
//end chen 0524


}

// lesley 0821
void AI_face_ICM_blending_value_global(int max_face_size)
{
	int i;
	int total_w = 0, center_u_tar = 0, center_v_tar = 0, h_offset = 0, s_offset = 0, v_offset = 0;

	int change_speed;
	int change_speed3;

	int icm_uv_ori[8];

	int size_value;
	int change_speed0;

	int d_change_speed_default;
	int change_speed_default;

	// disappear
	int size_diff_loth;
	int d_change_speed_size_loth;
	int d_change_speed_size_hith;
	int d_change_speed_size_slope;

	// appear //////
	int size_diff_loth_a;
	int d_change_speed_size_loth_a;
	int d_change_speed_size_hith_a;
	int d_change_speed_size_slope_a;

	int uv_blend[8]={0};

	int icm_global_en = ai_ctrl.ai_icm_tune2.icm_global_en;

	// lesley 0822
	int center_uv_step = 0;
	int center_u_cur = 0;
	int center_v_cur = 0;
	// end lesley 0822

	// lesley 0826
	color_icm_d_icm_cds_skin_1_RBUS   icm_d_icm_cds_skin_1_reg;
	int au, av, range, uvdist, delta, delta_u = 0, delta_v = 0;
	int keep_gray_mode = 1;
	int uv_range0_lo = 0;
	int uv_range0_up = 0;
	int range_m = 0;
	// end lesley 0826

	// lesley 0823
	int blend_size_en = 0;
	int blend_size_hith = 0;
	int blend_size_loth = 0;
	int value_blend_size = 0;
	// end lesley 0823

	// lesley 0902
    int center_u_init;
    int center_v_init;
	int center_u_lo;
	int center_u_up;
	int center_v_lo;
	int center_v_up;
	// end lesley 0902

	// setting //
	d_change_speed_default=ai_ctrl.ai_icm_blend.d_change_speed_default;
	change_speed_default=ai_ctrl.ai_icm_blend.change_speed_default;

	// disappear //////
	size_diff_loth=ai_ctrl.ai_icm_blend.size_diff_loth;
	d_change_speed_size_loth=ai_ctrl.ai_icm_blend.d_change_speed_size_loth;
	d_change_speed_size_hith=ai_ctrl.ai_icm_blend.d_change_speed_size_hith;
	d_change_speed_size_slope=ai_ctrl.ai_icm_blend.d_change_speed_size_slope;

	// appear //////
	size_diff_loth_a=ai_ctrl.ai_icm_blend.size_diff_loth_a;
	d_change_speed_size_loth_a=ai_ctrl.ai_icm_blend.d_change_speed_size_loth_a;
	d_change_speed_size_hith_a=ai_ctrl.ai_icm_blend.d_change_speed_size_hith_a;
	d_change_speed_size_slope_a=ai_ctrl.ai_icm_blend.d_change_speed_size_slope_a;

	// lesley 0822
	center_uv_step = ai_ctrl.ai_icm_tune2.center_uv_step;
	// end lesley 0822

	// lesley 0826
	icm_d_icm_cds_skin_1_reg.regValue = IoReg_Read32(COLOR_ICM_D_ICM_CDS_SKIN_1_reg);
	keep_gray_mode = ai_ctrl.ai_icm_tune2.keep_gray_mode;
	uv_range0_lo = ai_ctrl.ai_icm_tune2.uv_range0_lo;
	uv_range0_up = ai_ctrl.ai_icm_tune2.uv_range0_up;
	// end lesley 0826

	// lesley 0823
	blend_size_en = ai_ctrl.ai_global3.blend_size_en;
	blend_size_hith = ai_ctrl.ai_global3.blend_size_hith;
	blend_size_loth = ai_ctrl.ai_global3.blend_size_loth;
	// end lesley 0823

	// lesley 0902
    center_u_init = ai_ctrl.ai_icm_tune2.center_u_init;
    center_v_init = ai_ctrl.ai_icm_tune2.center_v_init;
	center_u_lo = ai_ctrl.ai_icm_tune2.center_u_lo;
	center_u_up = ai_ctrl.ai_icm_tune2.center_u_up;
	center_v_lo = ai_ctrl.ai_icm_tune2.center_v_lo;
	center_v_up = ai_ctrl.ai_icm_tune2.center_v_up;
	// end lesley 0902

	// end setting //

	icm_uv_ori[0]=ai_ctrl.ai_global.icm_uv_blend_ratio0;
	icm_uv_ori[1]=ai_ctrl.ai_global.icm_uv_blend_ratio1;
	icm_uv_ori[2]=ai_ctrl.ai_global.icm_uv_blend_ratio2;
	icm_uv_ori[3]=ai_ctrl.ai_global.icm_uv_blend_ratio3;
	icm_uv_ori[4]=ai_ctrl.ai_global.icm_uv_blend_ratio4;
	icm_uv_ori[5]=ai_ctrl.ai_global.icm_uv_blend_ratio5;
	icm_uv_ori[6]=ai_ctrl.ai_global.icm_uv_blend_ratio6;
	icm_uv_ori[7]=ai_ctrl.ai_global.icm_uv_blend_ratio7;

	if(max_face_size==0)
	{

		change_speed0=d_change_speed_default+change_speed_ai_sc;

		size_value=max_face_size;
		if(size_value>=size_diff_loth)
			change_speed3=d_change_speed_size_loth;
		else
		{
			change_speed3=d_change_speed_size_loth+d_change_speed_size_slope*(size_diff_loth-size_value)/32;

			if(change_speed3<d_change_speed_size_hith)
				change_speed3=d_change_speed_size_hith;
		}
	}
	else
	{
		change_speed0=change_speed_default+change_speed_ai_sc;

		size_value=max_face_size;
		if(size_value>=size_diff_loth_a)
			change_speed3=d_change_speed_size_loth_a;
		else
		{
			change_speed3=d_change_speed_size_loth_a+d_change_speed_size_slope_a*(size_diff_loth_a-size_value)/32;

			if(change_speed3<d_change_speed_size_hith_a)
				change_speed3=d_change_speed_size_hith_a;
		}
	}
	change_speed=change_speed0+change_speed3;

	AI_ICM_global_blend=AI_ICM_global_blend+change_speed;

	if(blend_size_en)
	{
		value_blend_size = AI_ICM_global_blend * (max_face_size - blend_size_loth) / (blend_size_hith - blend_size_loth);

		if(value_blend_size > AI_ICM_global_blend)
			value_blend_size = AI_ICM_global_blend;

		if(value_blend_size < 0)
			value_blend_size = 0;


		AI_ICM_global_blend = value_blend_size;
	}

	if(AI_ICM_global_blend>255)
		AI_ICM_global_blend=255;

	if(AI_ICM_global_blend<0)
		AI_ICM_global_blend=0;

	if(scene_change_flag==1)
	{
		AI_ICM_global_blend=0;
	}

	uv_blend[0]=(icm_uv_ori[0]*AI_ICM_global_blend+64*(255-AI_ICM_global_blend))/255;
	uv_blend[1]=(icm_uv_ori[1]*AI_ICM_global_blend+64*(255-AI_ICM_global_blend))/255;
	uv_blend[2]=(icm_uv_ori[2]*AI_ICM_global_blend+64*(255-AI_ICM_global_blend))/255;
	uv_blend[3]=(icm_uv_ori[3]*AI_ICM_global_blend+64*(255-AI_ICM_global_blend))/255;
	uv_blend[4]=(icm_uv_ori[4]*AI_ICM_global_blend+64*(255-AI_ICM_global_blend))/255;
	uv_blend[5]=(icm_uv_ori[5]*AI_ICM_global_blend+64*(255-AI_ICM_global_blend))/255;
	uv_blend[6]=(icm_uv_ori[6]*AI_ICM_global_blend+64*(255-AI_ICM_global_blend))/255;
	uv_blend[7]=(icm_uv_ori[7]*AI_ICM_global_blend+64*(255-AI_ICM_global_blend))/255;

	if(uv_blend[0]>63)
		uv_blend[0]=63;
	if(uv_blend[1]>63)
		uv_blend[1]=63;
	if(uv_blend[2]>63)
		uv_blend[2]=63;
	if(uv_blend[3]>63)
		uv_blend[3]=63;
	if(uv_blend[4]>63)
		uv_blend[4]=63;
	if(uv_blend[5]>63)
		uv_blend[5]=63;
	if(uv_blend[6]>63)
		uv_blend[6]=63;
	if(uv_blend[7]>63)
		uv_blend[7]=63;

	for(i=0; i<6; i++)
	{
		total_w += AI_detect_value_blend[i];

		center_u_tar += (face_icm_apply[buf_idx_w][i].center_u_s * AI_detect_value_blend[i]);
		center_v_tar += (face_icm_apply[buf_idx_w][i].center_v_s * AI_detect_value_blend[i]);

		h_offset += (face_icm_apply[buf_idx_w][i].hue_adj_s * AI_detect_value_blend[i]);
		s_offset += (face_icm_apply[buf_idx_w][i].sat_adj_s * AI_detect_value_blend[i]);
		v_offset += (face_icm_apply[buf_idx_w][i].int_adj_s * AI_detect_value_blend[i]);
	}

	center_u_tar = (total_w>0)?(center_u_tar/total_w):(0);
	center_v_tar = (total_w>0)?(center_v_tar/total_w):(0);

	// lesley 0826
	if(keep_gray_mode == 0)
	{
		icm_d_icm_cds_skin_1_reg.cds_uv_range_0 = uv_range0_up;
		IoReg_Write32(COLOR_ICM_D_ICM_CDS_SKIN_1_reg,icm_d_icm_cds_skin_1_reg.regValue);
	}
	else if(keep_gray_mode == 1)
	{
		icm_d_icm_cds_skin_1_reg.cds_uv_range_0 = uv_range0_up;
		IoReg_Write32(COLOR_ICM_D_ICM_CDS_SKIN_1_reg,icm_d_icm_cds_skin_1_reg.regValue);

		range = uv_range0_up;

		au = abs(center_u_tar - (2048>>data_shift));
		av = abs(center_v_tar - (2048>>data_shift));
		uvdist = (av>au)?(au/2+av):(av/2+au);

		if(uvdist < range)
		{
			delta = range - uvdist;

			delta_u = (delta * au + (au + av + 1)/2) / (au + av + 1);
			delta_v = (delta * av + (au + av + 1)/2) / (au + av + 1);

			if(center_u_tar>(2048>>data_shift))
				center_u_tar += delta_u;
			else
				center_u_tar -= delta_u;

			if(center_v_tar>(2048>>data_shift))
				center_v_tar += delta_v;
			else
				center_v_tar -= delta_v;

		}
	}
	else if(keep_gray_mode == 3)
	{
		icm_d_icm_cds_skin_1_reg.cds_uv_range_0 = uv_range0_up;
		IoReg_Write32(COLOR_ICM_D_ICM_CDS_SKIN_1_reg,icm_d_icm_cds_skin_1_reg.regValue);

		range = uv_range0_up;

		au = abs(center_u_tar - (2048>>data_shift));
		av = abs(center_v_tar - (2048>>data_shift));
		uvdist = (av>au)?(au/2+av):(av/2+au);

		if(uvdist < range)
		{
			delta = (range - uvdist)/2;

			delta_u = (delta * au + (au + av + 1)/2) / (au + av + 1);
			delta_v = (delta * av + (au + av + 1)/2) / (au + av + 1);

			if(center_u_tar>(2048>>data_shift))
				center_u_tar += delta_u;
			else
				center_u_tar -= delta_u;

			if(center_v_tar>(2048>>data_shift))
				center_v_tar += delta_v;
			else
				center_v_tar -= delta_v;

		}
	}
	// end lesley 0826

	// lesley 0902
	if(scene_change_flag)
	{
		center_u_tar = center_u_init;
		center_v_tar = center_v_init;

		AI_ICM_global_center_u = center_u_init;
		AI_ICM_global_center_v = center_v_init;
	}
	// end lesley 0902

	AI_ICM_global_h_offset = (total_w>0)?(h_offset/total_w):(0);
	AI_ICM_global_s_offset = (total_w>0)?(s_offset/total_w):(0);
	AI_ICM_global_v_offset = (total_w>0)?(v_offset/total_w):(0);

	if(AI_ICM_global_h_offset<0)
	{
		AI_ICM_global_h_offset=(16384>>data_shift)+AI_ICM_global_h_offset;//h_offset sign 14bit(merlin5), 12bit(mac7p)
	}
	if(AI_ICM_global_s_offset<0)
	{
		AI_ICM_global_s_offset=131072+AI_ICM_global_s_offset;//s_offset sign 17bit
	}

	if(AI_ICM_global_v_offset < 0)
	{
		AI_ICM_global_v_offset = 32768 + AI_ICM_global_v_offset;//i_offset sign 15bit
	}

	// lesley 0822
	if(AI_ICM_global_center_u < center_u_tar)
	{
		center_u_cur = AI_ICM_global_center_u + center_uv_step;

		if(center_u_cur > center_u_tar)
			center_u_cur = center_u_tar;
	}
	else if(AI_ICM_global_center_u > center_u_tar)
	{
		center_u_cur = AI_ICM_global_center_u - center_uv_step;

		if(center_u_cur < center_u_tar)
			center_u_cur = center_u_tar;
	}
	else
		center_u_cur = AI_ICM_global_center_u;


	if(AI_ICM_global_center_v < center_v_tar)
	{
		center_v_cur = AI_ICM_global_center_v + center_uv_step;

		if(center_v_cur > center_v_tar)
			center_v_cur = center_v_tar;
	}
	else if(AI_ICM_global_center_v > center_v_tar)
	{
		center_v_cur = AI_ICM_global_center_v - center_uv_step;

		if(center_v_cur < center_v_tar)
			center_v_cur = center_v_tar;
	}
	else
		center_v_cur = AI_ICM_global_center_v;

	// lesley 0826
	if(keep_gray_mode == 2 || keep_gray_mode == 3)
	{
		range = uv_range0_up;

		au = abs(center_u_tar - (2048>>data_shift));
		av = abs(center_v_tar - (2048>>data_shift));
		uvdist = (av>au)?(au/2+av):(av/2+au);

		if(uvdist < range)
		{
			range_m = uvdist;

			if(range_m < uv_range0_lo)
				range_m = uv_range0_lo;

			icm_d_icm_cds_skin_1_reg.cds_uv_range_0 = range_m;
			IoReg_Write32(COLOR_ICM_D_ICM_CDS_SKIN_1_reg,icm_d_icm_cds_skin_1_reg.regValue);
		}
	}
	// end lesley 0826

	AI_ICM_global_center_u = center_u_cur;
	AI_ICM_global_center_v = center_v_cur;

	// lesley 0902
	if(AI_ICM_global_center_u < center_u_lo)
		AI_ICM_global_center_u = center_u_lo;
	else if(AI_ICM_global_center_u > center_u_up)
		AI_ICM_global_center_u = center_u_up;

	if(AI_ICM_global_center_v < center_v_lo)
		AI_ICM_global_center_v = center_v_lo;
	else if(AI_ICM_global_center_v > center_v_up)
		AI_ICM_global_center_v = center_v_up;
	// end lesley 0902

	// end lesley 0822

	if(icm_global_en)
	{
		face_icm_apply[buf_idx_w][0].pos_x_s = 1920;
		face_icm_apply[buf_idx_w][0].pos_y_s = 1080;
		face_dcc_apply[buf_idx_w][0].pos_x_s = 1920;
		face_dcc_apply[buf_idx_w][0].pos_y_s = 1080;

		face_icm_apply[buf_idx_w][0].range_s = 4095;
		face_dcc_apply[buf_idx_w][0].range_s = 4095;

		face_icm_apply[buf_idx_w][0].center_u_s = AI_ICM_global_center_u;
		face_icm_apply[buf_idx_w][0].center_v_s = AI_ICM_global_center_v;
		face_dcc_apply[buf_idx_w][0].center_u_s = AI_ICM_global_center_u;
		face_dcc_apply[buf_idx_w][0].center_v_s = AI_ICM_global_center_v;

		face_icm_apply[buf_idx_w][0].hue_adj_s = AI_ICM_global_h_offset;
		face_icm_apply[buf_idx_w][0].sat_adj_s = AI_ICM_global_s_offset;
		face_icm_apply[buf_idx_w][0].int_adj_s = AI_ICM_global_v_offset;

		face_icm_apply[buf_idx_w][0].uv_blending_0	= uv_blend[0];
		face_icm_apply[buf_idx_w][0].uv_blending_1	= uv_blend[1];
		face_icm_apply[buf_idx_w][0].uv_blending_2	= uv_blend[2];
		face_icm_apply[buf_idx_w][0].uv_blending_3	= uv_blend[3];
		face_icm_apply[buf_idx_w][0].uv_blending_4	= uv_blend[4];
		face_icm_apply[buf_idx_w][0].uv_blending_5	= uv_blend[5];
		face_icm_apply[buf_idx_w][0].uv_blending_6	= uv_blend[6];
		face_icm_apply[buf_idx_w][0].uv_blending_7	= uv_blend[7];

		face_icm_apply[buf_idx_w][1].uv_blending_0	= 63;
		face_icm_apply[buf_idx_w][1].uv_blending_1	= 63;
		face_icm_apply[buf_idx_w][1].uv_blending_2	= 63;
		face_icm_apply[buf_idx_w][1].uv_blending_3	= 63;
		face_icm_apply[buf_idx_w][1].uv_blending_4	= 63;
		face_icm_apply[buf_idx_w][1].uv_blending_5	= 63;
		face_icm_apply[buf_idx_w][1].uv_blending_6	= 63;
		face_icm_apply[buf_idx_w][1].uv_blending_7	= 63;

		face_icm_apply[buf_idx_w][2].uv_blending_0	= 63;
		face_icm_apply[buf_idx_w][2].uv_blending_1	= 63;
		face_icm_apply[buf_idx_w][2].uv_blending_2	= 63;
		face_icm_apply[buf_idx_w][2].uv_blending_3	= 63;
		face_icm_apply[buf_idx_w][2].uv_blending_4	= 63;
		face_icm_apply[buf_idx_w][2].uv_blending_5	= 63;
		face_icm_apply[buf_idx_w][2].uv_blending_6	= 63;
		face_icm_apply[buf_idx_w][2].uv_blending_7	= 63;

		face_icm_apply[buf_idx_w][3].uv_blending_0	= 63;
		face_icm_apply[buf_idx_w][3].uv_blending_1	= 63;
		face_icm_apply[buf_idx_w][3].uv_blending_2	= 63;
		face_icm_apply[buf_idx_w][3].uv_blending_3	= 63;
		face_icm_apply[buf_idx_w][3].uv_blending_4	= 63;
		face_icm_apply[buf_idx_w][3].uv_blending_5	= 63;
		face_icm_apply[buf_idx_w][3].uv_blending_6	= 63;
		face_icm_apply[buf_idx_w][3].uv_blending_7	= 63;

		face_icm_apply[buf_idx_w][4].uv_blending_0	= 63;
		face_icm_apply[buf_idx_w][4].uv_blending_1	= 63;
		face_icm_apply[buf_idx_w][4].uv_blending_2	= 63;
		face_icm_apply[buf_idx_w][4].uv_blending_3	= 63;
		face_icm_apply[buf_idx_w][4].uv_blending_4	= 63;
		face_icm_apply[buf_idx_w][4].uv_blending_5	= 63;
		face_icm_apply[buf_idx_w][4].uv_blending_6	= 63;
		face_icm_apply[buf_idx_w][4].uv_blending_7	= 63;

 		face_icm_apply[buf_idx_w][5].uv_blending_0	= 63;
 		face_icm_apply[buf_idx_w][5].uv_blending_1	= 63;
 		face_icm_apply[buf_idx_w][5].uv_blending_2	= 63;
 		face_icm_apply[buf_idx_w][5].uv_blending_3	= 63;
 		face_icm_apply[buf_idx_w][5].uv_blending_4	= 63;
 		face_icm_apply[buf_idx_w][5].uv_blending_5	= 63;
 		face_icm_apply[buf_idx_w][5].uv_blending_6	= 63;
 		face_icm_apply[buf_idx_w][5].uv_blending_7	= 63;


		face_icm_apply[buf_idx_w][1].hue_adj_s = 0;
		face_icm_apply[buf_idx_w][1].sat_adj_s = 0;
		face_icm_apply[buf_idx_w][1].int_adj_s = 0;

		face_icm_apply[buf_idx_w][2].hue_adj_s = 0;
		face_icm_apply[buf_idx_w][2].sat_adj_s = 0;
		face_icm_apply[buf_idx_w][2].int_adj_s = 0;

		face_icm_apply[buf_idx_w][3].hue_adj_s = 0;
		face_icm_apply[buf_idx_w][3].sat_adj_s = 0;
		face_icm_apply[buf_idx_w][3].int_adj_s = 0;

		face_icm_apply[buf_idx_w][4].hue_adj_s = 0;
		face_icm_apply[buf_idx_w][4].sat_adj_s = 0;
		face_icm_apply[buf_idx_w][4].int_adj_s = 0;

		face_icm_apply[buf_idx_w][5].hue_adj_s = 0;
		face_icm_apply[buf_idx_w][5].sat_adj_s = 0;
		face_icm_apply[buf_idx_w][5].int_adj_s = 0;
	}
}
// end lesley 0821


// chen 0429
void AI_face_DCC_blending_value(int faceIdx, AIInfo face)
{
	int value_diff;
	int change_speed;
	int change_speed2;
	int change_speed3;
	int dcc_ai_ori[5];

	// chen 0524
	int dcc_uv_ori[8];
	//end chen 0524

	int IOU_value;
	int size_value;
	int change_speed0,change_speed1;


	// chen 0429
	int change_speed_temp;
	int change_speed22;
	int IOU_value2;


	int d_change_speed_default;
	int change_speed_default;

	// disappear
	int val_diff_loth;
	int d_change_speed_val_loth;
	int d_change_speed_val_hith;
	int d_change_speed_val_slope;
	int IOU_diff_loth;
	int d_change_speed_IOU_loth;
	int d_change_speed_IOU_hith;
	int d_change_speed_IOU_slope;
	int IOU_diff_loth2;
	int d_change_speed_IOU_loth2;
	int d_change_speed_IOU_hith2;
	int d_change_speed_IOU_slope2;
	int size_diff_loth;
	int d_change_speed_size_loth;
	int d_change_speed_size_hith;
	int d_change_speed_size_slope;

	// appear //////
	int val_diff_loth_a;
	int d_change_speed_val_loth_a;
	int d_change_speed_val_hith_a;
	int d_change_speed_val_slope_a;
	int IOU_diff_loth_a;
	int d_change_speed_IOU_loth_a;
	int d_change_speed_IOU_hith_a;
	int d_change_speed_IOU_slope_a;
	int IOU_diff_loth_a2;
	int d_change_speed_IOU_loth_a2;
	int d_change_speed_IOU_hith_a2;
	int d_change_speed_IOU_slope_a2;
	int size_diff_loth_a;
	int d_change_speed_size_loth_a;
	int d_change_speed_size_hith_a;
	int d_change_speed_size_slope_a;

	// lesley 0823
	int blend_size_en = 0;
	int blend_size_hith = 0;
	int blend_size_loth = 0;
	int value_blend_size = 0;
	// end lesley 0823

	// setting //
	d_change_speed_default=ai_ctrl.ai_dcc_blend.d_change_speed_default;
	change_speed_default=ai_ctrl.ai_dcc_blend.change_speed_default;

	// disappear //////
	val_diff_loth=ai_ctrl.ai_dcc_blend.val_diff_loth;
	d_change_speed_val_loth=ai_ctrl.ai_dcc_blend.d_change_speed_val_loth;
	d_change_speed_val_hith=ai_ctrl.ai_dcc_blend.d_change_speed_val_hith;
	d_change_speed_val_slope=ai_ctrl.ai_dcc_blend.d_change_speed_val_slope;

	IOU_diff_loth=ai_ctrl.ai_dcc_blend.IOU_diff_loth;//25;
	d_change_speed_IOU_loth=ai_ctrl.ai_dcc_blend.d_change_speed_IOU_loth;
	d_change_speed_IOU_hith=ai_ctrl.ai_dcc_blend.d_change_speed_IOU_hith;//-50
	d_change_speed_IOU_slope=ai_ctrl.ai_dcc_blend.d_change_speed_IOU_slope;

	IOU_diff_loth2=ai_ctrl.ai_dcc_blend.IOU_diff_loth2;//70;//25;
	d_change_speed_IOU_loth2=ai_ctrl.ai_dcc_blend.d_change_speed_IOU_loth2;//-50;//0;
	d_change_speed_IOU_hith2=ai_ctrl.ai_dcc_blend.d_change_speed_IOU_hith2;//-50; //-50
	d_change_speed_IOU_slope2=ai_ctrl.ai_dcc_blend.d_change_speed_IOU_slope2;//-2;

	size_diff_loth=ai_ctrl.ai_dcc_blend.size_diff_loth;
	d_change_speed_size_loth=ai_ctrl.ai_dcc_blend.d_change_speed_size_loth;
	d_change_speed_size_hith=ai_ctrl.ai_dcc_blend.d_change_speed_size_hith;
	d_change_speed_size_slope=ai_ctrl.ai_dcc_blend.d_change_speed_size_slope;

	// appear //////
	val_diff_loth_a=ai_ctrl.ai_dcc_blend.val_diff_loth_a;
	d_change_speed_val_loth_a=ai_ctrl.ai_dcc_blend.d_change_speed_val_loth_a;
	d_change_speed_val_hith_a=ai_ctrl.ai_dcc_blend.d_change_speed_val_hith_a;
	d_change_speed_val_slope_a=ai_ctrl.ai_dcc_blend.d_change_speed_val_slope_a;

	IOU_diff_loth_a=ai_ctrl.ai_dcc_blend.IOU_diff_loth_a;//25;
	d_change_speed_IOU_loth_a=ai_ctrl.ai_dcc_blend.d_change_speed_IOU_loth_a;
	d_change_speed_IOU_hith_a=ai_ctrl.ai_dcc_blend.d_change_speed_IOU_hith_a; //-50
	d_change_speed_IOU_slope_a=ai_ctrl.ai_dcc_blend.d_change_speed_IOU_slope_a;

	IOU_diff_loth_a2=ai_ctrl.ai_dcc_blend.IOU_diff_loth_a2;//50;//25;
	d_change_speed_IOU_loth_a2=ai_ctrl.ai_dcc_blend.d_change_speed_IOU_loth_a2;//-50;
	d_change_speed_IOU_hith_a2=ai_ctrl.ai_dcc_blend.d_change_speed_IOU_hith_a2;//-50
	d_change_speed_IOU_slope_a2=ai_ctrl.ai_dcc_blend.d_change_speed_IOU_slope_a2;

	size_diff_loth_a=ai_ctrl.ai_dcc_blend.size_diff_loth_a;
	d_change_speed_size_loth_a=ai_ctrl.ai_dcc_blend.d_change_speed_size_loth_a;
	d_change_speed_size_hith_a=ai_ctrl.ai_dcc_blend.d_change_speed_size_hith_a;
	d_change_speed_size_slope_a=ai_ctrl.ai_dcc_blend.d_change_speed_size_slope_a;

	// lesley 0823
	blend_size_en = ai_ctrl.ai_global3.blend_size_en;
	blend_size_hith = ai_ctrl.ai_global3.blend_size_hith;
	blend_size_loth = ai_ctrl.ai_global3.blend_size_loth;
	// end lesley 0823


	// end setting //
	// end chen 0429


	dcc_ai_ori[0]=ai_ctrl.ai_global.dcc_ai_blend_inside_ratio;
	dcc_ai_ori[1]=ai_ctrl.ai_global.dcc_ai_blend_ratio0;
	dcc_ai_ori[2]=ai_ctrl.ai_global.dcc_ai_blend_ratio1;
	dcc_ai_ori[3]=ai_ctrl.ai_global.dcc_ai_blend_ratio2;
	dcc_ai_ori[4]=ai_ctrl.ai_global.dcc_ai_blend_ratio3;

	// chen 0524
	dcc_uv_ori[0]=ai_ctrl.ai_global2.dcc_uv_blend_ratio0;
	dcc_uv_ori[1]=ai_ctrl.ai_global2.dcc_uv_blend_ratio1;
	dcc_uv_ori[2]=ai_ctrl.ai_global2.dcc_uv_blend_ratio2;
	dcc_uv_ori[3]=ai_ctrl.ai_global2.dcc_uv_blend_ratio3;
	dcc_uv_ori[4]=ai_ctrl.ai_global2.dcc_uv_blend_ratio4;
	dcc_uv_ori[5]=ai_ctrl.ai_global2.dcc_uv_blend_ratio5;
	dcc_uv_ori[6]=ai_ctrl.ai_global2.dcc_uv_blend_ratio6;
	dcc_uv_ori[7]=ai_ctrl.ai_global2.dcc_uv_blend_ratio7;

	//end chen 0524


	if(face.pv8==0)
	{
		value_diff=value_diff_pre[faceIdx];

		change_speed0=d_change_speed_default+change_speed_ai_sc;

		if(value_diff<=val_diff_loth)
			change_speed1=d_change_speed_val_loth;
		else
		{
			change_speed1=d_change_speed_val_loth+d_change_speed_val_slope*(value_diff-val_diff_loth);

			if(change_speed1<d_change_speed_val_hith)
				change_speed1=d_change_speed_val_hith;
		}

		IOU_value=IOU_pre[faceIdx];
		if(IOU_value>=IOU_diff_loth)
			change_speed2=d_change_speed_IOU_loth;
		else
		{
			change_speed2=d_change_speed_IOU_loth+d_change_speed_IOU_slope*(IOU_diff_loth-IOU_value);

			if(change_speed2<d_change_speed_IOU_hith)
				change_speed2=d_change_speed_IOU_hith;
		}

		// chen 0429
		IOU_value2=IOU_pre_max2[faceIdx];
		if(IOU_value2<=IOU_diff_loth2)
			change_speed22=d_change_speed_IOU_loth2;
		else
		{
			change_speed22=d_change_speed_IOU_loth2+d_change_speed_IOU_slope2*(IOU_value2-IOU_diff_loth2)/8;

			if(change_speed22<d_change_speed_IOU_hith2)
				change_speed22=d_change_speed_IOU_hith2;
		}
		change_speed2=change_speed2+change_speed22;
		// end chen 0429


		size_value=face.w12;
		if(size_value>=size_diff_loth)
			change_speed3=d_change_speed_size_loth;
		else
		{
			change_speed3=d_change_speed_size_loth+d_change_speed_size_slope*(size_diff_loth-size_value)/32;

			if(change_speed3<d_change_speed_size_hith)
				change_speed3=d_change_speed_size_hith;
		}
	}
	else
	{
		if(AI_detect_value_blend[faceIdx]>0)
		{
			value_diff=abs(face.y_med12-face_info_pre[faceIdx].y_med12)/16;

		}else
			value_diff=0;

		value_diff_pre[faceIdx]=value_diff;

		change_speed0=change_speed_default+change_speed_ai_sc;

		if(value_diff<=val_diff_loth_a)
			change_speed1=d_change_speed_val_loth_a;
		else
		{
			change_speed1=d_change_speed_val_loth_a+d_change_speed_val_slope_a*(value_diff-val_diff_loth_a);

			if(change_speed1<d_change_speed_val_hith_a)
				change_speed1=d_change_speed_val_hith_a;
		}

		IOU_value=IOU_pre[faceIdx];
		if(IOU_value>=IOU_diff_loth_a)
			change_speed2=d_change_speed_IOU_loth_a;
		else
		{
			change_speed2=d_change_speed_IOU_loth_a+d_change_speed_IOU_slope_a*(IOU_diff_loth_a-IOU_value);

			if(change_speed2<d_change_speed_IOU_hith_a)
				change_speed2=d_change_speed_IOU_hith_a;
		}

		// chen 0429
		IOU_value2=IOU_pre_max2[faceIdx];
		if(IOU_value2<=IOU_diff_loth_a2)
			change_speed22=d_change_speed_IOU_loth_a2;
		else
		{
			change_speed22=d_change_speed_IOU_loth_a2+d_change_speed_IOU_slope_a2*(IOU_value2-IOU_diff_loth_a2)/8;

			if(change_speed22<d_change_speed_IOU_hith_a2)
				change_speed22=d_change_speed_IOU_hith_a2;
		}
		change_speed2=change_speed2+change_speed22;
		// end chen 0429


		size_value=face.w12;
		if(size_value>=size_diff_loth_a)
			change_speed3=d_change_speed_size_loth_a;
		else
		{
			change_speed3=d_change_speed_size_loth_a+d_change_speed_size_slope_a*(size_diff_loth_a-size_value)/32;

			if(change_speed3<d_change_speed_size_hith_a)
				change_speed3=d_change_speed_size_hith_a;
		}
	}
	change_speed=change_speed0+change_speed1+change_speed3;
//change_speed=MAX((change_speed+change_speed2),MIN(change_speed,1));

	// chen 0429
	change_speed_temp=change_speed+change_speed2;
  //end chen 0429

	if(frame_drop_count==0)
	{
		change_speed_t_dcc[faceIdx]=change_speed_temp;
		// chen 0429
	//	change_speed_t[faceIdx]=MAX(change_speed_temp,
	//		MIN(change_speed,(1-AI_detect_value_blend[faceIdx])));
		//end chen 0429

	}
	if(AI_detect_value_blend_dcc[faceIdx]<=1)
	{
		if(change_speed_t_dcc[faceIdx]>0)
			change_speed_t_dcc[faceIdx]=1;
	}

	AI_detect_value_blend_dcc[faceIdx]=AI_detect_value_blend_dcc[faceIdx]+change_speed_t_dcc[faceIdx];

	AI_detect_value_blend_dcc[faceIdx]=MIN(AI_detect_value_blend_dcc[faceIdx],AI_detect_value_blend[faceIdx]);
	if(AI_detect_value_blend_dcc[faceIdx]<0)
		AI_detect_value_blend_dcc[faceIdx]=0;

	if(AI_detect_value_blend_dcc[faceIdx]>255)
		AI_detect_value_blend_dcc[faceIdx]=255;


	if(scene_change_flag==1)
	{
		AI_detect_value_blend_dcc[faceIdx]=0;
	}

	if(blend_size_en)
	{
		value_blend_size = AI_detect_value_blend_dcc[faceIdx] * (face.w12 - blend_size_loth) / (blend_size_hith - blend_size_loth);

		if(value_blend_size > AI_detect_value_blend_dcc[faceIdx])
			value_blend_size = AI_detect_value_blend_dcc[faceIdx];

		if(value_blend_size < 0)
			value_blend_size = 0;


		AI_detect_value_blend_dcc[faceIdx] = value_blend_size;
	}

	face_dcc_apply[buf_idx_w][faceIdx].ai_blending_inside_s	=(dcc_ai_ori[0]*AI_detect_value_blend_dcc[faceIdx]+64*(255-AI_detect_value_blend_dcc[faceIdx]))/255;
	face_dcc_apply[buf_idx_w][faceIdx].ai_blending_0_s	=(dcc_ai_ori[1]*AI_detect_value_blend_dcc[faceIdx]+64*(255-AI_detect_value_blend_dcc[faceIdx]))/255;
	face_dcc_apply[buf_idx_w][faceIdx].ai_blending_1_s	=(dcc_ai_ori[2]*AI_detect_value_blend_dcc[faceIdx]+64*(255-AI_detect_value_blend_dcc[faceIdx]))/255;
	face_dcc_apply[buf_idx_w][faceIdx].ai_blending_2_s	=(dcc_ai_ori[3]*AI_detect_value_blend_dcc[faceIdx]+64*(255-AI_detect_value_blend_dcc[faceIdx]))/255;
	face_dcc_apply[buf_idx_w][faceIdx].ai_blending_3_s	=(dcc_ai_ori[4]*AI_detect_value_blend_dcc[faceIdx]+64*(255-AI_detect_value_blend_dcc[faceIdx]))/255;

// chen 0524
	face_dcc_apply[buf_idx_w][faceIdx].uv_blending_0	=(dcc_uv_ori[0]*AI_detect_value_blend_dcc[faceIdx]+64*(255-AI_detect_value_blend_dcc[faceIdx]))/255;
	face_dcc_apply[buf_idx_w][faceIdx].uv_blending_1	=(dcc_uv_ori[1]*AI_detect_value_blend_dcc[faceIdx]+64*(255-AI_detect_value_blend_dcc[faceIdx]))/255;
	face_dcc_apply[buf_idx_w][faceIdx].uv_blending_2	=(dcc_uv_ori[2]*AI_detect_value_blend_dcc[faceIdx]+64*(255-AI_detect_value_blend_dcc[faceIdx]))/255;
	face_dcc_apply[buf_idx_w][faceIdx].uv_blending_3	=(dcc_uv_ori[3]*AI_detect_value_blend_dcc[faceIdx]+64*(255-AI_detect_value_blend_dcc[faceIdx]))/255;
	face_dcc_apply[buf_idx_w][faceIdx].uv_blending_4	=(dcc_uv_ori[4]*AI_detect_value_blend_dcc[faceIdx]+64*(255-AI_detect_value_blend_dcc[faceIdx]))/255;
	face_dcc_apply[buf_idx_w][faceIdx].uv_blending_5	=(dcc_uv_ori[5]*AI_detect_value_blend_dcc[faceIdx]+64*(255-AI_detect_value_blend_dcc[faceIdx]))/255;
	face_dcc_apply[buf_idx_w][faceIdx].uv_blending_6	=(dcc_uv_ori[6]*AI_detect_value_blend_dcc[faceIdx]+64*(255-AI_detect_value_blend_dcc[faceIdx]))/255;
	face_dcc_apply[buf_idx_w][faceIdx].uv_blending_7	=(dcc_uv_ori[7]*AI_detect_value_blend_dcc[faceIdx]+64*(255-AI_detect_value_blend_dcc[faceIdx]))/255;

	if(face_dcc_apply[buf_idx_w][faceIdx].uv_blending_0>63)
		face_dcc_apply[buf_idx_w][faceIdx].uv_blending_0	=63;
	if(face_dcc_apply[buf_idx_w][faceIdx].uv_blending_1>63)
		face_dcc_apply[buf_idx_w][faceIdx].uv_blending_1	=63;
	if(face_dcc_apply[buf_idx_w][faceIdx].uv_blending_2>63)
		face_dcc_apply[buf_idx_w][faceIdx].uv_blending_2	=63;
	if(face_dcc_apply[buf_idx_w][faceIdx].uv_blending_3>63)
		face_dcc_apply[buf_idx_w][faceIdx].uv_blending_3	=63;
	if(face_dcc_apply[buf_idx_w][faceIdx].uv_blending_4>63)
		face_dcc_apply[buf_idx_w][faceIdx].uv_blending_4	=63;
	if(face_dcc_apply[buf_idx_w][faceIdx].uv_blending_5>63)
		face_dcc_apply[buf_idx_w][faceIdx].uv_blending_5	=63;
	if(face_dcc_apply[buf_idx_w][faceIdx].uv_blending_6>63)
		face_dcc_apply[buf_idx_w][faceIdx].uv_blending_6	=63;
	if(face_dcc_apply[buf_idx_w][faceIdx].uv_blending_7>63)
		face_dcc_apply[buf_idx_w][faceIdx].uv_blending_7	=63;
//end chen 0524

	// lesley 0910
	face_dcc_apply[buf_idx_w][faceIdx].enhance_en = ai_ctrl.ai_global2.dcc_enhance_en;
	// end lesley 0910
}
// end chen 0429

// chen 0805
void AI_face_DCC_blending_value_global(int max_face_size)
{
	// lesley 0815
	int i;
	int total_w = 0, center_y_tar = 0, center_u_tar = 0, center_v_tar = 0;
	// end lesley 0815

	int change_speed;
	int change_speed3;

	int dcc_uv_ori[8];

	int size_value;
	int change_speed0;

	int d_change_speed_default;
	int change_speed_default;

	// disappear
	int size_diff_loth;
	int d_change_speed_size_loth;
	int d_change_speed_size_hith;
	int d_change_speed_size_slope;

	// appear //////
	int size_diff_loth_a;
	int d_change_speed_size_loth_a;
	int d_change_speed_size_hith_a;
	int d_change_speed_size_slope_a;

	// chen 0805
	int uv_blend[8]={0};
	//end chen 0805

	// lesley 0822
	int center_uv_step = 0;
	int center_u_cur = 0;
	int center_v_cur = 0;

	int center_y_step = 0;
	int center_y_cur = 0;
	// end lesley 0822

	// lesley 0823
	int blend_size_en = 0;
	int blend_size_hith = 0;
	int blend_size_loth = 0;
	int value_blend_size = 0;
	// end lesley 0823

	// lesley 0808
	int dcc_global_en = ai_ctrl.ai_global2.dcc_global_en;
	int dcc_old_skin_en = ai_ctrl.ai_global2.dcc_old_skin_en;
	// end lesley 0808

	// lesley 0902
	int center_y_init = 2000>>data_shift; // lesley 0904 TBD
    int center_u_init;
    int center_v_init;
	int center_u_lo;
	int center_u_up;
	int center_v_lo;
	int center_v_up;
	// end lesley 0902

	// lesley 0904
	int dcc_keep_gray_mode;
	int dcc_old_skin_y_range;
	int dcc_old_skin_u_range;
	int dcc_old_skin_v_range;

	int y_range, u_range, v_range, u_range_m, v_range_m;
	color_dcc_d_dcc_skin_tone_yuv_range_0_RBUS d_dcc_skin_tone_yuv_range_0_reg;
	int au, av;
	// end lesley 0904

	// lesley 0822
	center_uv_step = ai_ctrl.ai_global2.center_uv_step;
	// end lesley 0822
	center_y_step = ai_ctrl.ai_global2.center_y_step;

	// lesley 0823
	blend_size_en = ai_ctrl.ai_global3.blend_size_en;
	blend_size_hith = ai_ctrl.ai_global3.blend_size_hith;
	blend_size_loth = ai_ctrl.ai_global3.blend_size_loth;
	// end lesley 0823

	// lesley 0902
    center_u_init = ai_ctrl.ai_icm_tune2.center_u_init;
    center_v_init = ai_ctrl.ai_icm_tune2.center_v_init;
	center_u_lo = ai_ctrl.ai_icm_tune2.center_u_lo;
	center_u_up = ai_ctrl.ai_icm_tune2.center_u_up;
	center_v_lo = ai_ctrl.ai_icm_tune2.center_v_lo;
	center_v_up = ai_ctrl.ai_icm_tune2.center_v_up;
	// end lesley 0902

	// lesley 0904
	dcc_keep_gray_mode = ai_ctrl.ai_global2.dcc_keep_gray_mode;
	dcc_old_skin_y_range = ai_ctrl.ai_global2.dcc_old_skin_y_range;
	dcc_old_skin_u_range = ai_ctrl.ai_global2.dcc_old_skin_u_range;
	dcc_old_skin_v_range = ai_ctrl.ai_global2.dcc_old_skin_v_range;
	d_dcc_skin_tone_yuv_range_0_reg.regValue = IoReg_Read32(COLOR_DCC_D_DCC_SKIN_TONE_YUV_RANGE_0_reg);
	// end lesley 0904

	// setting //
	d_change_speed_default=ai_ctrl.ai_dcc_blend.d_change_speed_default;
	change_speed_default=ai_ctrl.ai_dcc_blend.change_speed_default;

	// disappear //////
	size_diff_loth=ai_ctrl.ai_dcc_blend.size_diff_loth;
	d_change_speed_size_loth=ai_ctrl.ai_dcc_blend.d_change_speed_size_loth;
	d_change_speed_size_hith=ai_ctrl.ai_dcc_blend.d_change_speed_size_hith;
	d_change_speed_size_slope=ai_ctrl.ai_dcc_blend.d_change_speed_size_slope;

	// appear //////
	size_diff_loth_a=ai_ctrl.ai_dcc_blend.size_diff_loth_a;
	d_change_speed_size_loth_a=ai_ctrl.ai_dcc_blend.d_change_speed_size_loth_a;
	d_change_speed_size_hith_a=ai_ctrl.ai_dcc_blend.d_change_speed_size_hith_a;
	d_change_speed_size_slope_a=ai_ctrl.ai_dcc_blend.d_change_speed_size_slope_a;
	// end setting //

	dcc_uv_ori[0]=ai_ctrl.ai_global2.dcc_uv_blend_ratio0;
	dcc_uv_ori[1]=ai_ctrl.ai_global2.dcc_uv_blend_ratio1;
	dcc_uv_ori[2]=ai_ctrl.ai_global2.dcc_uv_blend_ratio2;
	dcc_uv_ori[3]=ai_ctrl.ai_global2.dcc_uv_blend_ratio3;
	dcc_uv_ori[4]=ai_ctrl.ai_global2.dcc_uv_blend_ratio4;
	dcc_uv_ori[5]=ai_ctrl.ai_global2.dcc_uv_blend_ratio5;
	dcc_uv_ori[6]=ai_ctrl.ai_global2.dcc_uv_blend_ratio6;
	dcc_uv_ori[7]=ai_ctrl.ai_global2.dcc_uv_blend_ratio7;



	if(max_face_size==0)
	{

		change_speed0=d_change_speed_default+change_speed_ai_sc;

		size_value=max_face_size;
		if(size_value>=size_diff_loth)
			change_speed3=d_change_speed_size_loth;
		else
		{
			change_speed3=d_change_speed_size_loth+d_change_speed_size_slope*(size_diff_loth-size_value)/32;

			if(change_speed3<d_change_speed_size_hith)
				change_speed3=d_change_speed_size_hith;
		}
	}
	else
	{
		change_speed0=change_speed_default+change_speed_ai_sc;

		size_value=max_face_size;
		if(size_value>=size_diff_loth_a)
			change_speed3=d_change_speed_size_loth_a;
		else
		{
			change_speed3=d_change_speed_size_loth_a+d_change_speed_size_slope_a*(size_diff_loth_a-size_value)/32;

			if(change_speed3<d_change_speed_size_hith_a)
				change_speed3=d_change_speed_size_hith_a;
		}
	}
	change_speed=change_speed0+change_speed3;

	// global blend for global dcc/shp
	AI_DCC_global_blend=AI_DCC_global_blend+change_speed;


	if(blend_size_en)
	{
		value_blend_size = AI_DCC_global_blend * (max_face_size - blend_size_loth) / (blend_size_hith - blend_size_loth);

		if(value_blend_size > AI_DCC_global_blend)
			value_blend_size = AI_DCC_global_blend;

		if(value_blend_size < 0)
			value_blend_size = 0;


		AI_DCC_global_blend = value_blend_size;
	}

	if(AI_DCC_global_blend>255)
		AI_DCC_global_blend=255;

	if(AI_DCC_global_blend<0)
		AI_DCC_global_blend=0;

	if(scene_change_flag==1)
	{
		AI_DCC_global_blend=0;
	}

	uv_blend[0]=(dcc_uv_ori[0]*AI_DCC_global_blend+64*(255-AI_DCC_global_blend))/255;
	uv_blend[1]=(dcc_uv_ori[1]*AI_DCC_global_blend+64*(255-AI_DCC_global_blend))/255;
	uv_blend[2]=(dcc_uv_ori[2]*AI_DCC_global_blend+64*(255-AI_DCC_global_blend))/255;
	uv_blend[3]=(dcc_uv_ori[3]*AI_DCC_global_blend+64*(255-AI_DCC_global_blend))/255;
	uv_blend[4]=(dcc_uv_ori[4]*AI_DCC_global_blend+64*(255-AI_DCC_global_blend))/255;
	uv_blend[5]=(dcc_uv_ori[5]*AI_DCC_global_blend+64*(255-AI_DCC_global_blend))/255;
	uv_blend[6]=(dcc_uv_ori[6]*AI_DCC_global_blend+64*(255-AI_DCC_global_blend))/255;
	uv_blend[7]=(dcc_uv_ori[7]*AI_DCC_global_blend+64*(255-AI_DCC_global_blend))/255;

	if(uv_blend[0]>63)
		uv_blend[0]=63;
	if(uv_blend[1]>63)
		uv_blend[1]=63;
	if(uv_blend[2]>63)
		uv_blend[2]=63;
	if(uv_blend[3]>63)
		uv_blend[3]=63;
	if(uv_blend[4]>63)
		uv_blend[4]=63;
	if(uv_blend[5]>63)
		uv_blend[5]=63;
	if(uv_blend[6]>63)
		uv_blend[6]=63;
	if(uv_blend[7]>63)
		uv_blend[7]=63;

	// lesley 0808
	if(dcc_global_en)
	// end lesley 0808
	{
		face_dcc_apply[buf_idx_w][0].uv_blending_0	= uv_blend[0];
		face_dcc_apply[buf_idx_w][0].uv_blending_1	= uv_blend[1];
		face_dcc_apply[buf_idx_w][0].uv_blending_2	= uv_blend[2];
		face_dcc_apply[buf_idx_w][0].uv_blending_3	= uv_blend[3];
		face_dcc_apply[buf_idx_w][0].uv_blending_4	= uv_blend[4];
		face_dcc_apply[buf_idx_w][0].uv_blending_5	= uv_blend[5];
		face_dcc_apply[buf_idx_w][0].uv_blending_6	= uv_blend[6];
		face_dcc_apply[buf_idx_w][0].uv_blending_7	= uv_blend[7];

		face_dcc_apply[buf_idx_w][1].uv_blending_0	= uv_blend[0];
		face_dcc_apply[buf_idx_w][1].uv_blending_1	= uv_blend[1];
		face_dcc_apply[buf_idx_w][1].uv_blending_2	= uv_blend[2];
		face_dcc_apply[buf_idx_w][1].uv_blending_3	= uv_blend[3];
		face_dcc_apply[buf_idx_w][1].uv_blending_4	= uv_blend[4];
		face_dcc_apply[buf_idx_w][1].uv_blending_5	= uv_blend[5];
		face_dcc_apply[buf_idx_w][1].uv_blending_6	= uv_blend[6];
		face_dcc_apply[buf_idx_w][1].uv_blending_7	= uv_blend[7];

		face_dcc_apply[buf_idx_w][2].uv_blending_0	= uv_blend[0];
		face_dcc_apply[buf_idx_w][2].uv_blending_1	= uv_blend[1];
		face_dcc_apply[buf_idx_w][2].uv_blending_2	= uv_blend[2];
		face_dcc_apply[buf_idx_w][2].uv_blending_3	= uv_blend[3];
		face_dcc_apply[buf_idx_w][2].uv_blending_4	= uv_blend[4];
		face_dcc_apply[buf_idx_w][2].uv_blending_5	= uv_blend[5];
		face_dcc_apply[buf_idx_w][2].uv_blending_6	= uv_blend[6];
		face_dcc_apply[buf_idx_w][2].uv_blending_7	= uv_blend[7];

		face_dcc_apply[buf_idx_w][3].uv_blending_0	= uv_blend[0];
		face_dcc_apply[buf_idx_w][3].uv_blending_1	= uv_blend[1];
		face_dcc_apply[buf_idx_w][3].uv_blending_2	= uv_blend[2];
		face_dcc_apply[buf_idx_w][3].uv_blending_3	= uv_blend[3];
		face_dcc_apply[buf_idx_w][3].uv_blending_4	= uv_blend[4];
		face_dcc_apply[buf_idx_w][3].uv_blending_5	= uv_blend[5];
		face_dcc_apply[buf_idx_w][3].uv_blending_6	= uv_blend[6];
		face_dcc_apply[buf_idx_w][3].uv_blending_7	= uv_blend[7];

		face_dcc_apply[buf_idx_w][4].uv_blending_0	= uv_blend[0];
		face_dcc_apply[buf_idx_w][4].uv_blending_1	= uv_blend[1];
		face_dcc_apply[buf_idx_w][4].uv_blending_2	= uv_blend[2];
		face_dcc_apply[buf_idx_w][4].uv_blending_3	= uv_blend[3];
		face_dcc_apply[buf_idx_w][4].uv_blending_4	= uv_blend[4];
		face_dcc_apply[buf_idx_w][4].uv_blending_5	= uv_blend[5];
		face_dcc_apply[buf_idx_w][4].uv_blending_6	= uv_blend[6];
		face_dcc_apply[buf_idx_w][4].uv_blending_7	= uv_blend[7];

 		face_dcc_apply[buf_idx_w][5].uv_blending_0	= uv_blend[0];
 		face_dcc_apply[buf_idx_w][5].uv_blending_1	= uv_blend[1];
 		face_dcc_apply[buf_idx_w][5].uv_blending_2	= uv_blend[2];
 		face_dcc_apply[buf_idx_w][5].uv_blending_3	= uv_blend[3];
 		face_dcc_apply[buf_idx_w][5].uv_blending_4	= uv_blend[4];
 		face_dcc_apply[buf_idx_w][5].uv_blending_5	= uv_blend[5];
 		face_dcc_apply[buf_idx_w][5].uv_blending_6	= uv_blend[6];
 		face_dcc_apply[buf_idx_w][5].uv_blending_7	= uv_blend[7];
	}

	// lesley 0815
	for(i=0; i<6; i++)
	{
		center_y_tar += ((face_info_pre[i].y_med12>>data_shift) * AI_detect_value_blend_dcc[i]);
		center_u_tar += (face_dcc_apply[buf_idx_w][i].center_u_s * AI_detect_value_blend_dcc[i]);
		center_v_tar += (face_dcc_apply[buf_idx_w][i].center_v_s * AI_detect_value_blend_dcc[i]);
		total_w += AI_detect_value_blend_dcc[i];
	}

	// global uv for global dcc/shp
	// lesley 0904
	center_y_tar = (total_w>0)?(center_y_tar/total_w):(0);
	center_u_tar = (total_w>0)?(center_u_tar/total_w):(0);
	center_v_tar = (total_w>0)?(center_v_tar/total_w):(0);

	if(scene_change_flag)
	{
		center_y_tar = center_y_init;
		center_u_tar = center_u_init;
		center_v_tar = center_v_init;

		AI_DCC_global_center_y = center_y_init;
		AI_DCC_global_center_u = center_u_init;
		AI_DCC_global_center_v = center_v_init;
	}

	if(AI_DCC_global_center_y < center_y_tar)
	{
		center_y_cur = AI_DCC_global_center_y + center_y_step;

		if(center_y_cur > center_y_tar)
			center_y_cur = center_y_tar;
	}
	else if(AI_DCC_global_center_y > center_y_tar)
	{
		center_y_cur = AI_DCC_global_center_y - center_y_step;

		if(center_y_cur < center_y_tar)
			center_y_cur = center_y_tar;
	}
	else
		center_y_cur = AI_DCC_global_center_y;

	if(AI_DCC_global_center_u < center_u_tar)
	{
		center_u_cur = AI_DCC_global_center_u + center_uv_step;

		if(center_u_cur > center_u_tar)
			center_u_cur = center_u_tar;
	}
	else if(AI_DCC_global_center_u > center_u_tar)
	{
		center_u_cur = AI_DCC_global_center_u - center_uv_step;

		if(center_u_cur < center_u_tar)
			center_u_cur = center_u_tar;
	}
	else
		center_u_cur = AI_DCC_global_center_u;


	if(AI_DCC_global_center_v < center_v_tar)
	{
		center_v_cur = AI_DCC_global_center_v + center_uv_step;

		if(center_v_cur > center_v_tar)
			center_v_cur = center_v_tar;
	}
	else if(AI_DCC_global_center_v > center_v_tar)
	{
		center_v_cur = AI_DCC_global_center_v - center_uv_step;

		if(center_v_cur < center_v_tar)
			center_v_cur = center_v_tar;
	}
	else
		center_v_cur = AI_DCC_global_center_v;

	AI_DCC_global_center_y = center_y_cur;
	AI_DCC_global_center_u = center_u_cur;
	AI_DCC_global_center_v = center_v_cur;

	if(AI_DCC_global_center_u < center_u_lo)
		AI_DCC_global_center_u = center_u_lo;
	else if(AI_DCC_global_center_u > center_u_up)
		AI_DCC_global_center_u = center_u_up;

	if(AI_DCC_global_center_v < center_v_lo)
		AI_DCC_global_center_v = center_v_lo;
	else if(AI_DCC_global_center_v > center_v_up)
		AI_DCC_global_center_v = center_v_up;

	if(dcc_keep_gray_mode)
	{
		y_range = dcc_old_skin_y_range;
		u_range = d_dcc_skin_tone_yuv_range_0_reg.y_blending_0_u_range;
		v_range = d_dcc_skin_tone_yuv_range_0_reg.y_blending_0_v_range;

		au = abs(AI_DCC_global_center_u - (2048>>data_shift));//*32>>dcc_old_skin_u_range;
		av = abs(AI_DCC_global_center_v - (2048>>data_shift));//*32>>dcc_old_skin_v_range;

		u_range_m = u_range;
		v_range_m = v_range;

		if(au > (1<<(u_range+2)))
			u_range_m = u_range + 1;

		if(au < (1<<(u_range+1)))
			u_range_m = u_range - 1;

		if(av > (1<<(v_range+2)))
			v_range_m = v_range + 1;

		if(av < (1<<(v_range+1)))
			v_range_m = v_range - 1;

		if(u_range_m<0)
			u_range_m = 0;
		else if(u_range_m>11)
			u_range_m = 11;

		if(v_range_m<0)
			v_range_m = 0;
		else if(v_range_m>11)
			v_range_m = 11;
	}
	else
	{
		y_range = dcc_old_skin_y_range;
		u_range_m = dcc_old_skin_u_range;
		v_range_m = dcc_old_skin_v_range;
	}

	// end lesley 0904
	// end lesley 0815


	// lesley 0808
	if(dcc_old_skin_en)
	{
		old_dcc_apply[buf_idx_w].uv_blend_0 = uv_blend[0];
		old_dcc_apply[buf_idx_w].uv_blend_1 = uv_blend[1];
		old_dcc_apply[buf_idx_w].uv_blend_2 = uv_blend[2];
		old_dcc_apply[buf_idx_w].uv_blend_3 = uv_blend[3];
		old_dcc_apply[buf_idx_w].uv_blend_4 = uv_blend[4];
		old_dcc_apply[buf_idx_w].uv_blend_5 = uv_blend[5];
		old_dcc_apply[buf_idx_w].uv_blend_6 = uv_blend[6];
		old_dcc_apply[buf_idx_w].uv_blend_7 = uv_blend[7];

		old_dcc_apply[buf_idx_w].y_center = AI_DCC_global_center_y;
		old_dcc_apply[buf_idx_w].u_center = AI_DCC_global_center_u;
		old_dcc_apply[buf_idx_w].v_center = AI_DCC_global_center_v;

		old_dcc_apply[buf_idx_w].y_range = y_range;
		old_dcc_apply[buf_idx_w].u_range = u_range_m;
		old_dcc_apply[buf_idx_w].v_range = v_range_m;

		// lesley 0910
		old_dcc_apply[buf_idx_w].enhance_en = ai_ctrl.ai_global2.dcc_enhance_en;
		// end lesley 0910
		
		//drvif_color_old_skin_dcc_set(old_dcc_apply[buf_idx_w]); // lesley 1016, remove to PQ set
	}
	// end lesley 0808


}
//end chen 0805



// chen 0527
void AI_face_Sharp_blending_value(int faceIdx, AIInfo face)
{
	int value_diff;
	int change_speed;
	int change_speed2;
	int change_speed3;
	int IOU_value;
	int size_value;
	int change_speed0,change_speed1;
	int change_speed_temp;
	int change_speed22;
	int IOU_value2;
	int d_change_speed_default;
	int change_speed_default;

	// disappear
	int val_diff_loth;
	int d_change_speed_val_loth;
	int d_change_speed_val_hith;
	int d_change_speed_val_slope;
	int IOU_diff_loth;
	int d_change_speed_IOU_loth;
	int d_change_speed_IOU_hith;
	int d_change_speed_IOU_slope;
	int IOU_diff_loth2;
	int d_change_speed_IOU_loth2;
	int d_change_speed_IOU_hith2;
	int d_change_speed_IOU_slope2;
	int size_diff_loth;
	int d_change_speed_size_loth;
	int d_change_speed_size_hith;
	int d_change_speed_size_slope;

	// appear //////
	int val_diff_loth_a;
	int d_change_speed_val_loth_a;
	int d_change_speed_val_hith_a;
	int d_change_speed_val_slope_a;
	int IOU_diff_loth_a;
	int d_change_speed_IOU_loth_a;
	int d_change_speed_IOU_hith_a;
	int d_change_speed_IOU_slope_a;
	int IOU_diff_loth_a2;
	int d_change_speed_IOU_loth_a2;
	int d_change_speed_IOU_hith_a2;
	int d_change_speed_IOU_slope_a2;
	int size_diff_loth_a;
	int d_change_speed_size_loth_a;
	int d_change_speed_size_hith_a;
	int d_change_speed_size_slope_a;

	// lesley 0823
	int blend_size_en = 0;
	int blend_size_hith = 0;
	int blend_size_loth = 0;
	int value_blend_size = 0;
	// end lesley 0823

	// setting //
	d_change_speed_default=ai_ctrl.ai_sharp_blend.d_change_speed_default;
	change_speed_default=ai_ctrl.ai_sharp_blend.change_speed_default;

	// disappear //////
	val_diff_loth=ai_ctrl.ai_sharp_blend.val_diff_loth;
	d_change_speed_val_loth=ai_ctrl.ai_sharp_blend.d_change_speed_val_loth;
	d_change_speed_val_hith=ai_ctrl.ai_sharp_blend.d_change_speed_val_hith;
	d_change_speed_val_slope=ai_ctrl.ai_sharp_blend.d_change_speed_val_slope;

	IOU_diff_loth=ai_ctrl.ai_sharp_blend.IOU_diff_loth;//25;
	d_change_speed_IOU_loth=ai_ctrl.ai_sharp_blend.d_change_speed_IOU_loth;
	d_change_speed_IOU_hith=ai_ctrl.ai_sharp_blend.d_change_speed_IOU_hith;//-50
	d_change_speed_IOU_slope=ai_ctrl.ai_sharp_blend.d_change_speed_IOU_slope;

	IOU_diff_loth2=ai_ctrl.ai_sharp_blend.IOU_diff_loth2;//70;//25;
	d_change_speed_IOU_loth2=ai_ctrl.ai_sharp_blend.d_change_speed_IOU_loth2;//-50;//0;
	d_change_speed_IOU_hith2=ai_ctrl.ai_sharp_blend.d_change_speed_IOU_hith2;//-50; //-50
	d_change_speed_IOU_slope2=ai_ctrl.ai_sharp_blend.d_change_speed_IOU_slope2;//-2;

	size_diff_loth=ai_ctrl.ai_sharp_blend.size_diff_loth;
	d_change_speed_size_loth=ai_ctrl.ai_sharp_blend.d_change_speed_size_loth;
	d_change_speed_size_hith=ai_ctrl.ai_sharp_blend.d_change_speed_size_hith;
	d_change_speed_size_slope=ai_ctrl.ai_sharp_blend.d_change_speed_size_slope;

	// appear //////
	val_diff_loth_a=ai_ctrl.ai_sharp_blend.val_diff_loth_a;
	d_change_speed_val_loth_a=ai_ctrl.ai_sharp_blend.d_change_speed_val_loth_a;
	d_change_speed_val_hith_a=ai_ctrl.ai_sharp_blend.d_change_speed_val_hith_a;
	d_change_speed_val_slope_a=ai_ctrl.ai_sharp_blend.d_change_speed_val_slope_a;

	IOU_diff_loth_a=ai_ctrl.ai_sharp_blend.IOU_diff_loth_a;//25;
	d_change_speed_IOU_loth_a=ai_ctrl.ai_sharp_blend.d_change_speed_IOU_loth_a;
	d_change_speed_IOU_hith_a=ai_ctrl.ai_sharp_blend.d_change_speed_IOU_hith_a; //-50
	d_change_speed_IOU_slope_a=ai_ctrl.ai_sharp_blend.d_change_speed_IOU_slope_a;

	IOU_diff_loth_a2=ai_ctrl.ai_sharp_blend.IOU_diff_loth_a2;//50;//25;
	d_change_speed_IOU_loth_a2=ai_ctrl.ai_sharp_blend.d_change_speed_IOU_loth_a2;//-50;
	d_change_speed_IOU_hith_a2=ai_ctrl.ai_sharp_blend.d_change_speed_IOU_hith_a2;//-50
	d_change_speed_IOU_slope_a2=ai_ctrl.ai_sharp_blend.d_change_speed_IOU_slope_a2;

	size_diff_loth_a=ai_ctrl.ai_sharp_blend.size_diff_loth_a;
	d_change_speed_size_loth_a=ai_ctrl.ai_sharp_blend.d_change_speed_size_loth_a;
	d_change_speed_size_hith_a=ai_ctrl.ai_sharp_blend.d_change_speed_size_hith_a;
	d_change_speed_size_slope_a=ai_ctrl.ai_sharp_blend.d_change_speed_size_slope_a;

	// lesley 0823
	blend_size_en = ai_ctrl.ai_global3.blend_size_en;
	blend_size_hith = ai_ctrl.ai_global3.blend_size_hith;
	blend_size_loth = ai_ctrl.ai_global3.blend_size_loth;
	// end lesley 0823

	// end setting //

// chen 0815
	if(faceIdx==0)
		AI_face_sharp_count=0;
//end chen 0815

	if(face.pv8==0)
	{
		value_diff=value_diff_pre[faceIdx];

		change_speed0=d_change_speed_default+change_speed_ai_sc;

		if(value_diff<=val_diff_loth)
			change_speed1=d_change_speed_val_loth;
		else
		{
			change_speed1=d_change_speed_val_loth+d_change_speed_val_slope*(value_diff-val_diff_loth);

			if(change_speed1<d_change_speed_val_hith)
				change_speed1=d_change_speed_val_hith;
		}

		IOU_value=IOU_pre[faceIdx];
		if(IOU_value>=IOU_diff_loth)
			change_speed2=d_change_speed_IOU_loth;
		else
		{
			change_speed2=d_change_speed_IOU_loth+d_change_speed_IOU_slope*(IOU_diff_loth-IOU_value);

			if(change_speed2<d_change_speed_IOU_hith)
				change_speed2=d_change_speed_IOU_hith;
		}

		// chen 0429
		IOU_value2=IOU_pre_max2[faceIdx];
		if(IOU_value2<=IOU_diff_loth2)
			change_speed22=d_change_speed_IOU_loth2;
		else
		{
			change_speed22=d_change_speed_IOU_loth2+d_change_speed_IOU_slope2*(IOU_value2-IOU_diff_loth2)/8;

			if(change_speed22<d_change_speed_IOU_hith2)
				change_speed22=d_change_speed_IOU_hith2;
		}
		change_speed2=change_speed2+change_speed22;
		// end chen 0429


		size_value=face.w12;
		if(size_value>=size_diff_loth)
			change_speed3=d_change_speed_size_loth;
		else
		{
			change_speed3=d_change_speed_size_loth+d_change_speed_size_slope*(size_diff_loth-size_value)/32;

			if(change_speed3<d_change_speed_size_hith)
				change_speed3=d_change_speed_size_hith;
		}
	}
	else
	{
		if(AI_detect_value_blend[faceIdx]>0)
		{
			value_diff=abs(face.val_med12-face_info_pre[faceIdx].val_med12)/16;

		}else
			value_diff=0;

		value_diff_pre[faceIdx]=value_diff;

		change_speed0=change_speed_default+change_speed_ai_sc;

		if(value_diff<=val_diff_loth_a)
			change_speed1=d_change_speed_val_loth_a;
		else
		{
			change_speed1=d_change_speed_val_loth_a+d_change_speed_val_slope_a*(value_diff-val_diff_loth_a);

			if(change_speed1<d_change_speed_val_hith_a)
				change_speed1=d_change_speed_val_hith_a;
		}

		IOU_value=IOU_pre[faceIdx];
		if(IOU_value>=IOU_diff_loth_a)
			change_speed2=d_change_speed_IOU_loth_a;
		else
		{
			change_speed2=d_change_speed_IOU_loth_a+d_change_speed_IOU_slope_a*(IOU_diff_loth_a-IOU_value);

			if(change_speed2<d_change_speed_IOU_hith_a)
				change_speed2=d_change_speed_IOU_hith_a;
		}

		IOU_value2=IOU_pre_max2[faceIdx];
		if(IOU_value2<=IOU_diff_loth_a2)
			change_speed22=d_change_speed_IOU_loth_a2;
		else
		{
			change_speed22=d_change_speed_IOU_loth_a2+d_change_speed_IOU_slope_a2*(IOU_value2-IOU_diff_loth_a2)/8;

			if(change_speed22<d_change_speed_IOU_hith_a2)
				change_speed22=d_change_speed_IOU_hith_a2;
		}
		change_speed2=change_speed2+change_speed22;


		size_value=face.w12;
		if(size_value>=size_diff_loth_a)
			change_speed3=d_change_speed_size_loth_a;
		else
		{
			change_speed3=d_change_speed_size_loth_a+d_change_speed_size_slope_a*(size_diff_loth_a-size_value)/32;

			if(change_speed3<d_change_speed_size_hith_a)
				change_speed3=d_change_speed_size_hith_a;
		}
	}
	change_speed=change_speed0+change_speed1+change_speed3;

	change_speed_temp=change_speed+change_speed2;

	if(frame_drop_count==0)
	{
		change_speed_t_sharp[faceIdx]=change_speed_temp;
	}
	if(AI_detect_value_blend_sharp[faceIdx]<=1)
	{
		if(change_speed_t_sharp[faceIdx]>0)
			change_speed_t_sharp[faceIdx]=1;
	}

	AI_detect_value_blend_sharp[faceIdx]=AI_detect_value_blend_sharp[faceIdx]+change_speed_t_sharp[faceIdx];
  AI_detect_value_blend_sharp[faceIdx]=MIN(AI_detect_value_blend_sharp[faceIdx],AI_detect_value_blend[faceIdx]);
	if(AI_detect_value_blend_sharp[faceIdx]<0)
		AI_detect_value_blend_sharp[faceIdx]=0;

	if(AI_detect_value_blend_sharp[faceIdx]>255)
		AI_detect_value_blend_sharp[faceIdx]=255;


	if(scene_change_flag==1)
	{
		AI_detect_value_blend_sharp[faceIdx]=0;
	}

	if(blend_size_en)
	{
		value_blend_size = AI_detect_value_blend_sharp[faceIdx] * (face.w12 - blend_size_loth) / (blend_size_hith - blend_size_loth);

		if(value_blend_size > AI_detect_value_blend_sharp[faceIdx])
			value_blend_size = AI_detect_value_blend_sharp[faceIdx];

		if(value_blend_size < 0)
			value_blend_size = 0;


		AI_detect_value_blend_sharp[faceIdx] = value_blend_size;
	}

// chen 0527 ///sharpness blending ratio curve
	face_sharp_apply[buf_idx_w][faceIdx].pv8=AI_detect_value_blend_sharp[faceIdx];

//end chen 0527 ///calculate sharpness blending curve

// chen 0815
	if(AI_face_sharp_dynamic_single)
	{
		if(AI_face_sharp_count==0 && AI_detect_value_blend_sharp[faceIdx]>0)
		{
			face_sharp_apply[buf_idx_w][faceIdx].pv8=AI_detect_value_blend_sharp[faceIdx];
			// chen 0815_2 remove
//			AI_face_sharp_count++;
		}
		else
			face_sharp_apply[buf_idx_w][faceIdx].pv8=0;
	}

	// chen 0815_2
	if(AI_detect_value_blend_sharp[faceIdx]>0)
		AI_face_sharp_count++;
	//end chen 0815_2


//end chen 0815

}

//end chen 0527

// lesley 0815
void AI_face_Sharp_blending_value_global(void)
{
	int i;

	if(AI_face_sharp_dynamic_global)
	{
		for(i=0; i<6; i++)
		{
			face_sharp_apply[buf_idx_w][i].cb_med12 = AI_DCC_global_center_u<<data_shift;
			face_sharp_apply[buf_idx_w][i].cr_med12 = AI_DCC_global_center_v<<data_shift;
			face_sharp_apply[buf_idx_w][i].pv8 = AI_DCC_global_blend;

		}
	}
}
// end lesley 0815

void AI_face_ICM_tuning(int faceIdx, AIInfo face)
{
	int h_adj;
	int s_adj;

	// lesley 0808
	int v_adj = 0;
	// end lesley 0808

	// lesley 0821
	int icm_global_en = ai_ctrl.ai_icm_tune2.icm_global_en;
	// end lesley 0821

	/////////////// calcualte hue, sat, bright offset
	if(AI_detect_value_blend[faceIdx]>0)
		AI_face_dynamic_ICM_offset(faceIdx,face, &h_adj, &s_adj, &v_adj);
	else
	{
		h_adj=0;
		s_adj=0;
		v_adj=0;

		// lesley 1002_1
		drivef_tool_ai_info_set(faceIdx, 0, 0, 0);
		// end lesley 1002_1
	}

	// lesley 0821
	if(icm_global_en == 0)
	// end lesley 0821
	{
		if(h_adj<0)
		{
			h_adj=(16384>>data_shift)+h_adj;//h_offset sign 14bit(merlin5), 12bit(mac7p)
		}
		if(s_adj<0)
		{
			s_adj=131072+s_adj;//s_offset sign 17bit
		}

		// lesley 0808
		if(v_adj < 0)
		{
			v_adj = 32768 + v_adj;//i_offset sign 15bit
		}
		// end lesley 0808
	}


	face_icm_apply[buf_idx_w][faceIdx].hue_adj_s=h_adj;
	face_icm_apply[buf_idx_w][faceIdx].sat_adj_s=s_adj;

	// lesley 0808
	face_icm_apply[buf_idx_w][faceIdx].int_adj_s = v_adj;
	// end lesley 0808

}
#if 1 //LG
void AI_face_dynamic_ICM_offset(int faceIdx, AIInfo face, int *h_adj_o, int *s_adj_o, int *v_adj_o)
{
	/// YUV, RGB, H,S,I 12b (x16)
	int hue_ori_in;
	int sat_ori_in;
	int int_ori_in;
	int hue_info;
	int sat_info;
	int val_info;
	int h_adj;
	int s_adj;
	int sat_gain_diff;
	int sat_icm_mod;
	int hue_icm_mod;
	int val_mod;
	int sat_icm_mod_norm;
	int sat_off_target;
	int hue_icm_mod_norm;
	int hue_off_target;
	int hue_delta;
	int sat_delta;
	int val_delta;
	int hue_target_hi1, hue_target_hi2, hue_target_hi3;
	int hue_target_lo1, hue_target_lo2, hue_target_lo3;
	int sat_target_hi1, sat_target_hi2, sat_target_hi3;
	int sat_target_lo1, sat_target_lo2, sat_target_lo3;

	int s_adj_th_p_norm;
	int s_adj_th_n_norm;
	int h_adj_th_p_norm;
	int h_adj_th_n_norm;

	int val_delta_dcc;

	int sat3x3_gain; // from 3x3 matrix
	int bri_3x3_delta;			// from 3x3 matrix
	int DCC_delta=0;				// from DCC

// lesley 0813
	int h_adj_cur[6] = {0};
	int s_adj_cur[6] = {0};
	int h_adj_step;
	int s_adj_step;
// end lesley 0813

// lesley 0808
	int val_icm_mod_norm;
	int v_adj = 0;
	int val_target_hi1, val_target_hi2_ratio;
	int val_target_lo1, val_target_lo2_ratio;
	int v_adj_th_p_norm, v_adj_th_n_norm;
	int v_adj_cur[6] = {0};
	int v_adj_step;
// end lesley 0808


	// lesley 1001
	//int maxp, maxn;
	int mid_lo, mid_hi, b1, b2, int_off_target;
	// end lesley 1001
	
	// lesley 1007
	int hue_target_hi2_ratio, hue_target_lo2_ratio, sat_target_hi2_ratio, sat_target_lo2_ratio;
	// end lesley 1007

	// chen 0429
// setting //
	hue_target_hi1=ai_ctrl.ai_icm_tune.hue_target_hi1;
	hue_target_hi2=ai_ctrl.ai_icm_tune.hue_target_hi2;
	hue_target_hi3=ai_ctrl.ai_icm_tune.hue_target_hi3;
	hue_target_lo1=ai_ctrl.ai_icm_tune.hue_target_lo1;
	hue_target_lo2=ai_ctrl.ai_icm_tune.hue_target_lo2;
	hue_target_lo3=ai_ctrl.ai_icm_tune.hue_target_lo3;

	sat_target_hi1=ai_ctrl.ai_icm_tune.sat_target_hi1;
	sat_target_hi2=ai_ctrl.ai_icm_tune.sat_target_hi2;
	sat_target_hi3=ai_ctrl.ai_icm_tune.sat_target_hi3;
	sat_target_lo1=ai_ctrl.ai_icm_tune.sat_target_lo1;
	sat_target_lo2=ai_ctrl.ai_icm_tune.sat_target_lo2;
	sat_target_lo3=ai_ctrl.ai_icm_tune.sat_target_lo3;

	s_adj_th_p_norm=ai_ctrl.ai_icm_tune2.s_adj_th_p_norm;
	s_adj_th_n_norm=ai_ctrl.ai_icm_tune2.s_adj_th_n_norm;
	h_adj_th_p_norm=ai_ctrl.ai_icm_tune2.h_adj_th_p_norm;
	h_adj_th_n_norm=ai_ctrl.ai_icm_tune2.h_adj_th_n_norm;
	sat3x3_gain=ai_ctrl.ai_icm_tune.sat3x3_gain;// from 3x3 matrix
	bri_3x3_delta=ai_ctrl.ai_icm_tune.bri_3x3_delta;		// from 3x3 matrix
// end chen 0429

// lesley 0813
	h_adj_step = ai_ctrl.ai_icm_tune2.h_adj_step;
	s_adj_step = ai_ctrl.ai_icm_tune2.s_adj_step;
// end lesley 0813

// lesley 0808
	val_target_hi1 = ai_ctrl.ai_icm_tune2.val_target_hi1;
	val_target_hi2_ratio = ai_ctrl.ai_icm_tune2.val_target_hi2_ratio;
	val_target_lo1 = ai_ctrl.ai_icm_tune2.val_target_lo1;
	val_target_lo2_ratio = ai_ctrl.ai_icm_tune2.val_target_lo2_ratio;
	v_adj_th_p_norm = ai_ctrl.ai_icm_tune2.v_adj_th_p_norm;
	v_adj_th_n_norm = ai_ctrl.ai_icm_tune2.v_adj_th_n_norm;
	v_adj_step = ai_ctrl.ai_icm_tune2.v_adj_step;
// end lesley 0808

	// lesley 1007
	hue_target_lo2_ratio=ai_ctrl.ai_icm_tune2.hue_target_lo2_ratio;
	hue_target_hi2_ratio=ai_ctrl.ai_icm_tune2.hue_target_hi2_ratio;
	sat_target_lo2_ratio=ai_ctrl.ai_icm_tune2.sat_target_lo2_ratio;
	sat_target_hi2_ratio=ai_ctrl.ai_icm_tune2.sat_target_hi2_ratio;
	// end lesley 1007

// end setting //

	hue_info=face.hue_med12;
	sat_info=face.sat_med12;
	val_info=face.val_med12;

	hue_ori_in=hue_info;
	sat_ori_in=sat_info;
	int_ori_in=val_info;

// ICM:
//	AI_face_ICM_adjust_valuen(hue_info, sat_info, val_info, &hue_delta, &sat_delta, &val_delta, icm_table);
// chen 0426
	AI_face_ICM_adjust_valuen(hue_info, sat_info, val_info, &hue_delta, &sat_delta, &val_delta, icm_tab_elem_write.elem);


// DCC:
	drvif_color_get_dcc_adjust_value(val_info , &val_delta_dcc, dcc_table); //get from drvif_color_get_dcc_current_curve

	// chen 0528
	if(ai_ctrl.ai_icm_tune.icm_table_nouse==1)
	{
		hue_delta=0;
		sat_delta=0;
		val_delta=0;
	}
	//end chen 0528

	//
	icm_h_delta[faceIdx]=hue_delta;
	icm_s_delta[faceIdx]=sat_delta;
	icm_val_delta[faceIdx]=val_delta;

	// hue ----------------------------------------------
	//maxp = h_adj_th_p*360/6144;
	//maxn = h_adj_th_n*360/6144;

	hue_icm_mod=hue_ori_in+hue_delta;
	hue_icm_mod_norm=hue_icm_mod*720/6143;//hue_icm_mod*360/6144;// lesley, nn formula: ((hue*6143/360/8192)*256)<<4, hue: 0~360.

	if(hue_icm_mod_norm>300)
		hue_icm_mod_norm=hue_icm_mod_norm-360;

	//b1 = MIN((hue_target_lo1+hue_target_lo2)/2, hue_target_lo2+maxp);
	//b2 = MAX((hue_target_hi1+hue_target_hi2)/2, hue_target_hi2-maxn);
	b1 = hue_target_lo2 + MIN((hue_target_lo1-hue_target_lo2)*hue_target_lo2_ratio/100, h_adj_th_p_norm);
	b2 = hue_target_hi2 - MIN((hue_target_hi2-hue_target_hi1)*hue_target_hi2_ratio/100, h_adj_th_n_norm);

	if(hue_icm_mod_norm>=hue_target_lo1 && hue_icm_mod_norm<=hue_target_hi1)
		hue_off_target=hue_icm_mod_norm;
	else if(hue_icm_mod_norm>=hue_target_hi1 && hue_icm_mod_norm<=hue_target_hi2)
		hue_off_target=(hue_target_hi2-hue_target_hi1)?((hue_icm_mod_norm-hue_target_hi1)*(b2-hue_target_hi1)/(hue_target_hi2-hue_target_hi1)+hue_target_hi1):(hue_icm_mod_norm);
	else if(hue_icm_mod_norm>=hue_target_hi2 && hue_icm_mod_norm<=hue_target_hi3)
		hue_off_target=(hue_target_hi3-hue_target_hi2)?((hue_icm_mod_norm-hue_target_hi2)*(hue_target_hi3-b2)/(hue_target_hi3-hue_target_hi2)+b2):(hue_icm_mod_norm);
	else if(hue_icm_mod_norm<=hue_target_lo1 && hue_icm_mod_norm>=hue_target_lo2)
		hue_off_target=(hue_target_lo1-hue_target_lo2)?((hue_icm_mod_norm-hue_target_lo2)*(hue_target_lo1-b1)/(hue_target_lo1-hue_target_lo2)+b1):(hue_icm_mod_norm);
	else if(hue_icm_mod_norm<=hue_target_lo2 && hue_icm_mod_norm>=hue_target_lo3)
		hue_off_target=(hue_target_lo2-hue_target_lo3)?((hue_icm_mod_norm-hue_target_lo3)*(b1-hue_target_lo3)/(hue_target_lo2-hue_target_lo3)+hue_target_lo3):(hue_icm_mod_norm);
	else
		hue_off_target=hue_icm_mod_norm;

	if(ic_version==0)
		h_adj=(hue_off_target-hue_icm_mod_norm)*6143/360; // lesley, merlin5 hue 0~6143
	else if(ic_version==1)
		h_adj=(hue_off_target-hue_icm_mod_norm)*1535/360; // lesley, mac7p hue 0~1535

	if(h_adj_pre[faceIdx]<h_adj)
	{
		h_adj_cur[faceIdx] = h_adj_pre[faceIdx] + h_adj_step;

		if(h_adj_cur[faceIdx] > h_adj)
			h_adj_cur[faceIdx] = h_adj;
	}
	else if(h_adj_pre[faceIdx] > h_adj)
	{
		h_adj_cur[faceIdx] = h_adj_pre[faceIdx] - h_adj_step;

		if(h_adj_cur[faceIdx] < h_adj)
			h_adj_cur[faceIdx] = h_adj;
	}
	else
		h_adj_cur[faceIdx] = h_adj_pre[faceIdx];

	h_adj_pre[faceIdx] = h_adj_cur[faceIdx];

	h_adj = h_adj_cur[faceIdx];
	*h_adj_o=h_adj;

	// saturation ----------------------------------------------

	//sat_gain_diff=(sat_ori_in - (sat_ori_in*128 / sat3x3_gain))*0.8; // henry - kernel doesn't support floating mul
	sat_gain_diff=((sat_ori_in - (sat_ori_in*128 / sat3x3_gain))*204)>>8;
	sat_icm_mod=sat_ori_in+sat_delta+sat_gain_diff;
	val_mod=int_ori_in+((bri_3x3_delta)+DCC_delta)*16;
	if(val_mod<16*16)
		val_mod=16*16;
	if(val_mod>235*16)
		val_mod=235*16;

	sat_icm_mod_norm=sat_icm_mod*100/val_mod;

	if(sat_icm_mod_norm>100)
	{
		sat_icm_mod_norm=100;
		sat_icm_mod=sat_icm_mod_norm*val_mod/100;
	}
	if(int_ori_in==0)
		int_ori_in=1;

	// chen 0528
	if(ai_ctrl.ai_icm_tune.icm_sat_hith_nomax==0)
	{
		sat_target_hi1=MAX(sat_target_hi1,sat_ori_in*100/int_ori_in);
		sat_target_hi2=MAX(sat_target_hi2,sat_ori_in*100/int_ori_in);
		sat_target_hi3=MAX(sat_target_hi3,sat_ori_in*100/int_ori_in);
	}
	//end chen 0528
	

	//maxp = s_adj_th_p*100/val_mod;
	//maxn = s_adj_th_n*100/val_mod;

	//b1 = MIN((sat_target_lo1+sat_target_lo2)/2, sat_target_lo2+maxp);
	//b2 = MAX((sat_target_hi1+sat_target_hi2)/2, sat_target_hi2-maxn);
	b1 = sat_target_lo2 + MIN((sat_target_lo1-sat_target_lo2)*sat_target_lo2_ratio/100, s_adj_th_p_norm);
	b2 = sat_target_hi2 - MIN((sat_target_hi2-sat_target_hi1)*sat_target_hi2_ratio/100, s_adj_th_n_norm);

	if(sat_icm_mod_norm>=sat_target_lo1 && sat_icm_mod_norm<=sat_target_hi1)
		sat_off_target=sat_icm_mod_norm;
	else if(sat_icm_mod_norm>=sat_target_hi1 && sat_icm_mod_norm<=sat_target_hi2)
		sat_off_target=(sat_target_hi2-sat_target_hi1)?((sat_icm_mod_norm-sat_target_hi1)*(b2-sat_target_hi1)/(sat_target_hi2-sat_target_hi1)+sat_target_hi1):(sat_icm_mod_norm);
	else if(sat_icm_mod_norm>=sat_target_hi2 && sat_icm_mod_norm<=sat_target_hi3)
		sat_off_target=(sat_target_hi3-sat_target_hi2)?((sat_icm_mod_norm-sat_target_hi2)*(sat_target_hi3-b2)/(sat_target_hi3-sat_target_hi2)+b2):(sat_icm_mod_norm);
	else if(sat_icm_mod_norm<=sat_target_lo1 && sat_icm_mod_norm>=sat_target_lo2)
		sat_off_target=(sat_target_lo1-sat_target_lo2)?((sat_icm_mod_norm-sat_target_lo2)*(sat_target_lo1-b1)/(sat_target_lo1-sat_target_lo2)+b1):(sat_icm_mod_norm);
	else if(sat_icm_mod_norm<=sat_target_lo2 && sat_icm_mod_norm>=sat_target_lo3)
		sat_off_target=(sat_target_lo2-sat_target_lo3)?((sat_icm_mod_norm-sat_target_lo3)*(b1-sat_target_lo3)/(sat_target_lo2-sat_target_lo3)+sat_target_lo3):(sat_icm_mod_norm);
	else
		sat_off_target=sat_icm_mod_norm;

	s_adj=sat_off_target*val_mod/100-sat_icm_mod;
	
	if(s_adj_pre[faceIdx]<s_adj)
	{
		s_adj_cur[faceIdx] = s_adj_pre[faceIdx] + s_adj_step;

		if(s_adj_cur[faceIdx] > s_adj)
			s_adj_cur[faceIdx] = s_adj;
	}
	else if(s_adj_pre[faceIdx] > s_adj)
	{
		s_adj_cur[faceIdx] = s_adj_pre[faceIdx] - s_adj_step;

		if(s_adj_cur[faceIdx] < s_adj)
			s_adj_cur[faceIdx] = s_adj;
	}
	else
		s_adj_cur[faceIdx] = s_adj_pre[faceIdx];

	s_adj_pre[faceIdx] = s_adj_cur[faceIdx];

	s_adj = s_adj_cur[faceIdx];
	*s_adj_o=s_adj;

	// intensity ----------------------------------------------
	
	val_icm_mod_norm=(int_ori_in>>4);

	// lesley 1001
	mid_lo = val_target_lo1/2;
	mid_hi = (255+val_target_hi1)/2;
	b1 = mid_lo + MIN(mid_lo*val_target_lo2_ratio/100, v_adj_th_p_norm);
	b2 = mid_hi - MIN((mid_hi-val_target_hi1)*val_target_hi2_ratio/100, v_adj_th_n_norm);

	if(val_icm_mod_norm < mid_lo)
	{
		int_off_target = val_icm_mod_norm*b1/mid_lo;
	}
	else if(val_icm_mod_norm < val_target_lo1)
	{
		int_off_target = (val_target_lo1 - mid_lo)?((val_icm_mod_norm - mid_lo)*(val_target_lo1-b1)/(val_target_lo1 - mid_lo)+b1):(val_icm_mod_norm);			
	}
	else if(val_icm_mod_norm <= val_target_hi1)
	{
		int_off_target = val_icm_mod_norm;
	}
	else if(val_icm_mod_norm < mid_hi)
	{
		int_off_target = (mid_hi - val_target_hi1)?((val_icm_mod_norm - val_target_hi1)*(b2-val_target_hi1)/(mid_hi - val_target_hi1)+val_target_hi1):(val_icm_mod_norm);			
	}
	else
	{
		int_off_target = (255-mid_hi)?((val_icm_mod_norm - mid_hi)*(255-b2)/(255-mid_hi)+b2):(val_icm_mod_norm);
	}

	v_adj = (int_off_target - val_icm_mod_norm)<<4;
	// end lesley 1001

	// lesley 0808
	if(v_adj_pre[faceIdx]<v_adj)
	{
		v_adj_cur[faceIdx] = v_adj_pre[faceIdx] + v_adj_step;

		if(v_adj_cur[faceIdx] > v_adj)
			v_adj_cur[faceIdx] = v_adj;
	}
	else if(v_adj_pre[faceIdx] > v_adj)
	{
		v_adj_cur[faceIdx] = v_adj_pre[faceIdx] - v_adj_step;

		if(v_adj_cur[faceIdx] < v_adj)
			v_adj_cur[faceIdx] = v_adj;
	}
	else
		v_adj_cur[faceIdx] = v_adj_pre[faceIdx];

	v_adj_pre[faceIdx] = v_adj_cur[faceIdx];

	*v_adj_o = v_adj_cur[faceIdx];
		
	// end lesley 0808


	// lesley 1002_1
	drivef_tool_ai_info_set(faceIdx, hue_icm_mod_norm, sat_icm_mod_norm, val_icm_mod_norm);
	// end lesley 1002_1

}


#else
void AI_dynamic_ICM_offset(int faceIdx, AIInfo face, int *h_adj_o, int *s_adj_o, int *v_adj_o)
{
	/// YUV, RGB, H,S,I 12b (x16)
	int hue_ori_in;
	int sat_ori_in;
	int int_ori_in;
	int hue_info;
	int sat_info;
	int val_info;
	int h_adj;
	int s_adj;
	int sat_gain_diff;
	int sat_icm_mod;
	int hue_icm_mod;
	int val_mod;
	int sat_icm_mod_norm;
	int sat_off_target;
	int hue_icm_mod_norm;
	int hue_off_target;
	int hue_delta;
	int sat_delta;
	int val_delta;
	int hue_target_hi1, hue_target_hi2, hue_target_hi3;
	int hue_target_lo1, hue_target_lo2, hue_target_lo3;
	int sat_target_hi1, sat_target_hi2, sat_target_hi3;
	int sat_target_lo1, sat_target_lo2, sat_target_lo3;

	int s_adj_th_p;
	int s_adj_th_n;
	int h_adj_th_p;
	int h_adj_th_n;

	int val_delta_dcc;

	int sat3x3_gain; // from 3x3 matrix
	int bri_3x3_delta;			// from 3x3 matrix
	int DCC_delta=0;				// from DCC

// lesley 0813
	int h_adj_cur[6] = {0};
	int s_adj_cur[6] = {0};
	int h_adj_step;
	int s_adj_step;
// end lesley 0813

// lesley 0808
	int val_icm_mod_norm;
	int v_adj = 0;
	int val_target_hi1, val_target_hi2;
	int val_target_lo1, val_target_lo2;
	int v_adj_th_max_p, v_adj_th_max_n;
	int v_adj_cur[6] = {0};
	int v_adj_step;
// end lesley 0808

	// lesley 0819
	int beauty_mode;
	int beauty_sat_level, beauty_sat_target;
	int beauty_val_level, beauty_val_target;
	// end lesley 0819

	// chen 0429
// setting //
	hue_target_hi1=ai_ctrl.ai_icm_tune.hue_target_hi1;
	hue_target_hi2=ai_ctrl.ai_icm_tune.hue_target_hi2;
	hue_target_hi3=ai_ctrl.ai_icm_tune.hue_target_hi3;
	hue_target_lo1=ai_ctrl.ai_icm_tune.hue_target_lo1;
	hue_target_lo2=ai_ctrl.ai_icm_tune.hue_target_lo2;
	hue_target_lo3=ai_ctrl.ai_icm_tune.hue_target_lo3;

	sat_target_hi1=ai_ctrl.ai_icm_tune.sat_target_hi1;
	sat_target_hi2=ai_ctrl.ai_icm_tune.sat_target_hi2;
	sat_target_hi3=ai_ctrl.ai_icm_tune.sat_target_hi3;
	sat_target_lo1=ai_ctrl.ai_icm_tune.sat_target_lo1;
	sat_target_lo2=ai_ctrl.ai_icm_tune.sat_target_lo2;
	sat_target_lo3=ai_ctrl.ai_icm_tune.sat_target_lo3;

	s_adj_th_p=ai_ctrl.ai_icm_tune.s_adj_th_p;
	s_adj_th_n=ai_ctrl.ai_icm_tune.s_adj_th_n;
	h_adj_th_p=ai_ctrl.ai_icm_tune.h_adj_th_p;
	h_adj_th_n=ai_ctrl.ai_icm_tune.h_adj_th_n;
	sat3x3_gain=ai_ctrl.ai_icm_tune.sat3x3_gain;// from 3x3 matrix
	bri_3x3_delta=ai_ctrl.ai_icm_tune.bri_3x3_delta;		// from 3x3 matrix
// end chen 0429

// lesley 0813
	h_adj_step = ai_ctrl.ai_icm_tune2.h_adj_step;
	s_adj_step = ai_ctrl.ai_icm_tune2.s_adj_step;
// end lesley 0813

// lesley 0808
	val_target_hi1 = ai_ctrl.ai_icm_tune2.val_target_hi1;
	val_target_hi2 = ai_ctrl.ai_icm_tune2.val_target_hi2;
	val_target_lo1 = ai_ctrl.ai_icm_tune2.val_target_lo1;
	val_target_lo2 = ai_ctrl.ai_icm_tune2.val_target_lo2;
	v_adj_th_max_p = ai_ctrl.ai_icm_tune2.v_adj_th_max_p;
	v_adj_th_max_n = ai_ctrl.ai_icm_tune2.v_adj_th_max_n;
	v_adj_step = ai_ctrl.ai_icm_tune2.v_adj_step;
// end lesley 0808

	// lesley 0819
	beauty_mode = ai_ctrl.ai_icm_tune2.beauty_mode;
	beauty_sat_level = ai_ctrl.ai_icm_tune2.beauty_sat_level;
	beauty_sat_target = ai_ctrl.ai_icm_tune2.beauty_sat_target;
	beauty_val_level = ai_ctrl.ai_icm_tune2.beauty_val_level;
	beauty_val_target = ai_ctrl.ai_icm_tune2.beauty_val_target;
	// end lesley 0819

// end setting //

	hue_info=face.hue_med12;
	sat_info=face.sat_med12;
	val_info=face.val_med12;

	hue_ori_in=hue_info;
	sat_ori_in=sat_info;
	int_ori_in=val_info;

// ICM:
//	AI_face_ICM_adjust_valuen(hue_info, sat_info, val_info, &hue_delta, &sat_delta, &val_delta, icm_table);
// chen 0426
	AI_face_ICM_adjust_valuen(hue_info, sat_info, val_info, &hue_delta, &sat_delta, &val_delta, icm_tab_elem_write.elem);


// DCC:
	drvif_color_get_dcc_adjust_value(val_info , &val_delta_dcc, dcc_table); //get from drvif_color_get_dcc_current_curve

	// chen 0528
	if(ai_ctrl.ai_icm_tune.icm_table_nouse==1)
	{
		hue_delta=0;
		sat_delta=0;
		val_delta=0;
	}
	//end chen 0528

	//
	icm_h_delta[faceIdx]=hue_delta;
	icm_s_delta[faceIdx]=sat_delta;
	icm_val_delta[faceIdx]=val_delta;


	//sat_gain_diff=(sat_ori_in - (sat_ori_in*128 / sat3x3_gain))*0.8; // henry - kernel doesn't support floating mul
	sat_gain_diff=((sat_ori_in - (sat_ori_in*128 / sat3x3_gain))*204)>>8;
	sat_icm_mod=sat_ori_in+sat_delta+sat_gain_diff;
	hue_icm_mod=hue_ori_in+hue_delta;

	val_mod=int_ori_in+((bri_3x3_delta)+DCC_delta)*16;
	if(val_mod<16*16)
		val_mod=16*16;
	if(val_mod>235*16)
		val_mod=235*16;

	sat_icm_mod_norm=sat_icm_mod*100/val_mod;
	hue_icm_mod_norm=hue_icm_mod*360/6144;

	// lesley 0808
	val_icm_mod_norm=(int_ori_in>>4);
	// end lesley 0808

	if(sat_icm_mod_norm>100)
	{
		sat_icm_mod_norm=100;
		sat_icm_mod=sat_icm_mod_norm*val_mod/100;
	}
	if(int_ori_in==0)
		int_ori_in=1;


	if(hue_icm_mod_norm>300)
		hue_icm_mod_norm=hue_icm_mod_norm-360;

	if(hue_icm_mod_norm>=hue_target_lo1 && hue_icm_mod_norm<=hue_target_hi1)
		hue_off_target=hue_icm_mod_norm;
	else if(hue_icm_mod_norm>=hue_target_hi1 && hue_icm_mod_norm<=hue_target_hi2)
		hue_off_target=hue_target_hi1;
	else if(hue_icm_mod_norm>=hue_target_hi2 && hue_icm_mod_norm<=hue_target_hi3)
		hue_off_target=hue_target_hi1+(hue_icm_mod_norm-hue_target_hi2)*(hue_target_hi3-hue_target_hi1)/(hue_target_hi3-hue_target_hi2);
	else if(hue_icm_mod_norm<=hue_target_lo1 && hue_icm_mod_norm>=hue_target_lo2)
		hue_off_target=hue_target_lo1;
	else if(hue_icm_mod_norm<=hue_target_lo2 && hue_icm_mod_norm>=hue_target_lo3)
		hue_off_target=hue_target_lo1+(hue_icm_mod_norm-hue_target_lo2)*(hue_target_lo3-hue_target_lo1)/(hue_target_lo3-hue_target_lo2);
	else
		hue_off_target=hue_icm_mod_norm;

	h_adj=(hue_off_target-hue_icm_mod_norm)*6144/360;

	if(h_adj>h_adj_th_p)
		h_adj=h_adj_th_p;
	if(h_adj<-h_adj_th_n)
		h_adj=-h_adj_th_n;

// chen 0528
	if(ai_ctrl.ai_icm_tune.icm_sat_hith_nomax==0)
	{
		sat_target_hi1=MAX(sat_target_hi1,sat_ori_in*100/int_ori_in);
		sat_target_hi2=MAX(sat_target_hi2,sat_ori_in*100/int_ori_in);
		sat_target_hi3=MAX(sat_target_hi3,sat_ori_in*100/int_ori_in);
	}
	//end chen 0528

	// lesley 0819
	if(beauty_mode)
	{
		s_adj = sat_icm_mod_norm*beauty_sat_level/100;

		if(sat_icm_mod_norm < beauty_sat_target)
			s_adj = 0;
		else if(sat_icm_mod_norm - s_adj < beauty_sat_target)
			s_adj = sat_icm_mod_norm - beauty_sat_target;

		s_adj = -s_adj*val_mod/100;
	}
	else
	// end lesley 0819
	{
		if(sat_icm_mod_norm>=sat_target_lo1 && sat_icm_mod_norm<=sat_target_hi1)
			sat_off_target=sat_icm_mod_norm;
		else if(sat_icm_mod_norm>=sat_target_hi1 && sat_icm_mod_norm<=sat_target_hi2)
			sat_off_target=sat_target_hi1;
		else if(sat_icm_mod_norm>=sat_target_hi2 && sat_icm_mod_norm<=sat_target_hi3)
			sat_off_target=sat_target_hi1+(sat_icm_mod_norm-sat_target_hi2)*(sat_target_hi3-sat_target_hi1)/(sat_target_hi3-sat_target_hi2);
		else if(sat_icm_mod_norm<=sat_target_lo1 && sat_icm_mod_norm>=sat_target_lo2)
			sat_off_target=sat_target_lo1;
		else if(sat_icm_mod_norm<=sat_target_lo2 && sat_icm_mod_norm>=sat_target_lo3)
			sat_off_target=sat_target_lo1+(sat_icm_mod_norm-sat_target_lo2)*(sat_target_lo3-sat_target_lo1)/(sat_target_lo3-sat_target_lo2);
		else
			sat_off_target=sat_icm_mod_norm;

		s_adj=sat_off_target*val_mod/100-sat_icm_mod;

		if(s_adj>s_adj_th_p)
			s_adj=s_adj_th_p;
		if(s_adj<-s_adj_th_n)
			s_adj=-s_adj_th_n;

	}


// lesley 0813
	if(h_adj_pre[faceIdx]<h_adj)
	{
		h_adj_cur[faceIdx] = h_adj_pre[faceIdx] + h_adj_step;

		if(h_adj_cur[faceIdx] > h_adj)
			h_adj_cur[faceIdx] = h_adj;
	}
	else if(h_adj_pre[faceIdx] > h_adj)
	{
		h_adj_cur[faceIdx] = h_adj_pre[faceIdx] - h_adj_step;

		if(h_adj_cur[faceIdx] < h_adj)
			h_adj_cur[faceIdx] = h_adj;
	}
	else
		h_adj_cur[faceIdx] = h_adj_pre[faceIdx];

	h_adj_pre[faceIdx] = h_adj_cur[faceIdx];

	h_adj = h_adj_cur[faceIdx];

	if(s_adj_pre[faceIdx]<s_adj)
	{
		s_adj_cur[faceIdx] = s_adj_pre[faceIdx] + s_adj_step;

		if(s_adj_cur[faceIdx] > s_adj)
			s_adj_cur[faceIdx] = s_adj;
	}
	else if(s_adj_pre[faceIdx] > s_adj)
	{
		s_adj_cur[faceIdx] = s_adj_pre[faceIdx] - s_adj_step;

		if(s_adj_cur[faceIdx] < s_adj)
			s_adj_cur[faceIdx] = s_adj;
	}
	else
		s_adj_cur[faceIdx] = s_adj_pre[faceIdx];

	s_adj_pre[faceIdx] = s_adj_cur[faceIdx];

	s_adj = s_adj_cur[faceIdx];
// end lesley 0813

	*h_adj_o=h_adj;
	*s_adj_o=s_adj;


	// lesley 0819
	if(beauty_mode)
	{
		v_adj = val_icm_mod_norm * beauty_val_level / 100;

		if(val_icm_mod_norm > beauty_val_target)
			v_adj = 0;
		else if(val_icm_mod_norm + v_adj > beauty_val_target)
			v_adj = beauty_val_target - val_icm_mod_norm;

		v_adj = v_adj<<4;
	}
	else
	// end lesley 0819
	{
	// lesley 0808
		#if 0 // lesley 0909
		if((val_target_hi2 - val_target_hi1) == 0 || (val_target_lo2 - val_target_lo1) == 0)
			v_adj = 0;
		else if(val_icm_mod_norm >= val_target_lo1 && val_icm_mod_norm <= val_target_hi1)
			v_adj = 0;
		else if(val_icm_mod_norm >= val_target_hi1 && val_icm_mod_norm <= val_target_hi2)
			v_adj = -((val_icm_mod_norm - val_target_hi1) * v_adj_th_max_n / (val_target_hi2 - val_target_hi1));
		else if(val_icm_mod_norm >= val_target_hi2)
			v_adj = -v_adj_th_max_n;
		else if(val_icm_mod_norm <= val_target_lo1 && val_icm_mod_norm >= val_target_lo2)
			v_adj = ((val_icm_mod_norm - val_target_lo1) * v_adj_th_max_p / (val_target_lo2 - val_target_lo1));
		else if(val_icm_mod_norm <= val_target_lo2)
			v_adj = v_adj_th_max_p;

		#endif

		// lesley 0909
		if(val_icm_mod_norm >= val_target_hi1)
		{
			v_adj = - val_icm_mod_norm * val_target_hi2 / 100;

			if(val_icm_mod_norm + v_adj < val_target_hi1)
				v_adj = val_target_hi1 - val_icm_mod_norm;

			v_adj = v_adj<<4;

			if(v_adj < -v_adj_th_max_n)
				v_adj = -v_adj_th_max_n;
			
		}
		else if(val_icm_mod_norm <= val_target_lo1)
		{
			v_adj =  val_icm_mod_norm * val_target_lo2 / 100;

			if(val_icm_mod_norm + v_adj > val_target_lo1)
				v_adj = val_target_lo1 - val_icm_mod_norm;

			v_adj = v_adj<<4;

			if(v_adj > v_adj_th_max_p)
				v_adj = v_adj_th_max_p;
		}
		// end lesley 0909

	if(rtd_inl(0xb8025128)&(0x1) && face.pv8)
		rtd_pr_vpq_ai_info("lsy, icm %d) h %d %d, s %d %d, v %d %d\n",faceIdx, hue_icm_mod_norm, h_adj, sat_icm_mod_norm, s_adj, val_icm_mod_norm, v_adj);
		
	}

	if(v_adj_pre[faceIdx]<v_adj)
	{
		v_adj_cur[faceIdx] = v_adj_pre[faceIdx] + v_adj_step;

		if(v_adj_cur[faceIdx] > v_adj)
			v_adj_cur[faceIdx] = v_adj;
	}
	else if(v_adj_pre[faceIdx] > v_adj)
	{
		v_adj_cur[faceIdx] = v_adj_pre[faceIdx] - v_adj_step;

		if(v_adj_cur[faceIdx] < v_adj)
			v_adj_cur[faceIdx] = v_adj;
	}
	else
		v_adj_cur[faceIdx] = v_adj_pre[faceIdx];

	v_adj_pre[faceIdx] = v_adj_cur[faceIdx];

	*v_adj_o = v_adj_cur[faceIdx];

	// end lesley 0808

}
#endif

//void AI_face_ICM_adjust_valuen(int hue_info, int sat_info, int val_info, int *hue_delta, int *sat_delta, int *val_delta, unsigned short ICMTAB[290][60])
// chen 0426
void AI_face_ICM_adjust_valuen(int hue_info, int sat_info, int val_info, int *hue_delta, int *sat_delta, int *val_delta, COLORELEM ICM_TAB_ACCESS[ITNSEGMAX][SATSEGMAX][HUESEGMAX])
{
#if 0 // mac7p AI TBD
	int hue_ori_in, sat_ori_in, int_ori_in;
	int hue_index_th[21];
	int sat_index_th[12];
	int int_index_th[8];
	int hue_low_index=0;
	int sat_low_index=0;
	int int_low_index=0;
	int i, ii, ih,is, ii2, ih2,is2;
	int ih2o;
	// chen 0426 remove
//	int gitn, gsat,ghue;
	int sat_weight, hue_weight, int_weight;
	int delta_s_inter;
	int delta_h_inter;
	// chen 0426 remove
//	int SATSEGMAX2=	12;
//	int ITNSEGMAX2=	8;
	int reg_vc_icm_ctrl_his_grid_sel;
	int reg_vc_icm_h_pillar_num, reg_vc_icm_s_pillar_num, reg_vc_icm_i_pillar_num;

	color_icm_dm_icm_pillar_num_ctrl_RBUS	color_icm_dm_icm_pillar_num_ctrl_reg;
	color_icm_dm_icm_ctrl_RBUS              dm_icm_ctrl_reg;
	// chen 0426
	color_icm_dm_icm_hue_segment_0_RBUS color_icm_dm_icm_hue_segment_0;
	color_icm_dm_icm_hue_segment_1_RBUS color_icm_dm_icm_hue_segment_1;
	color_icm_dm_icm_hue_segment_2_RBUS color_icm_dm_icm_hue_segment_2;
	color_icm_dm_icm_hue_segment_3_RBUS color_icm_dm_icm_hue_segment_3;
	color_icm_dm_icm_hue_segment_4_RBUS color_icm_dm_icm_hue_segment_4;

	color_icm_dm_icm_hue_segment_23_RBUS color_icm_dm_icm_hue_segment_23;
	color_icm_dm_icm_hue_segment_22_RBUS color_icm_dm_icm_hue_segment_22;
	color_icm_dm_icm_hue_segment_21_RBUS color_icm_dm_icm_hue_segment_21;
	color_icm_dm_icm_hue_segment_20_RBUS color_icm_dm_icm_hue_segment_20;
	color_icm_dm_icm_hue_segment_19_RBUS color_icm_dm_icm_hue_segment_19;

	color_icm_dm_icm_sat_segment_0_RBUS	color_icm_dm_icm_sat_segment_0;
	color_icm_dm_icm_sat_segment_1_RBUS	color_icm_dm_icm_sat_segment_1;
	color_icm_dm_icm_sat_segment_2_RBUS	color_icm_dm_icm_sat_segment_2;
	color_icm_dm_icm_sat_segment_3_RBUS	color_icm_dm_icm_sat_segment_3;
	color_icm_dm_icm_sat_segment_4_RBUS	color_icm_dm_icm_sat_segment_4;

	color_icm_dm_icm_int_segment_0_RBUS	color_icm_dm_icm_int_segment_0;
	color_icm_dm_icm_int_segment_1_RBUS	color_icm_dm_icm_int_segment_1;
	color_icm_dm_icm_int_segment_2_RBUS	color_icm_dm_icm_int_segment_2;
// end chen 0426

	COLORELEM HSI_8point[2][2][2];
	COLORELEM d_HSI_8point[2][2][2];

	color_icm_dm_icm_pillar_num_ctrl_reg.regValue 	= IoReg_Read32(COLOR_ICM_DM_ICM_PILLAR_NUM_CTRL_reg);
// chen 0426
	color_icm_dm_icm_hue_segment_0.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_0_reg);
	color_icm_dm_icm_hue_segment_1.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_1_reg);
	color_icm_dm_icm_hue_segment_2.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_2_reg);
	color_icm_dm_icm_hue_segment_3.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_3_reg);
	color_icm_dm_icm_hue_segment_4.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_4_reg);

	color_icm_dm_icm_hue_segment_23.regValue	= IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_23_reg);
	color_icm_dm_icm_hue_segment_22.regValue	= IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_22_reg);
	color_icm_dm_icm_hue_segment_21.regValue	= IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_21_reg);
	color_icm_dm_icm_hue_segment_20.regValue	= IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_20_reg);
	color_icm_dm_icm_hue_segment_19.regValue	= IoReg_Read32(COLOR_ICM_DM_ICM_HUE_SEGMENT_19_reg);

	color_icm_dm_icm_sat_segment_0.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_SAT_SEGMENT_0_reg);
	color_icm_dm_icm_sat_segment_1.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_SAT_SEGMENT_1_reg);
	color_icm_dm_icm_sat_segment_2.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_SAT_SEGMENT_2_reg);
	color_icm_dm_icm_sat_segment_3.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_SAT_SEGMENT_3_reg);
	color_icm_dm_icm_sat_segment_4.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_SAT_SEGMENT_4_reg);

	color_icm_dm_icm_int_segment_0.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_INT_SEGMENT_0_reg);
	color_icm_dm_icm_int_segment_1.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_INT_SEGMENT_1_reg);
	color_icm_dm_icm_int_segment_2.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_INT_SEGMENT_2_reg);
// end chen 0426

	reg_vc_icm_h_pillar_num = color_icm_dm_icm_pillar_num_ctrl_reg.h_pillar_num;
	reg_vc_icm_s_pillar_num = color_icm_dm_icm_pillar_num_ctrl_reg.s_pillar_num;
	reg_vc_icm_i_pillar_num = color_icm_dm_icm_pillar_num_ctrl_reg.i_pillar_num;

	dm_icm_ctrl_reg.regValue = IoReg_Read32(COLOR_ICM_DM_ICM_CTRL_reg);
	reg_vc_icm_ctrl_his_grid_sel=dm_icm_ctrl_reg.hsi_grid_sel ;

	hue_ori_in=hue_info;
	sat_ori_in=sat_info;
	int_ori_in=val_info;

	if(reg_vc_icm_i_pillar_num>8)
		reg_vc_icm_i_pillar_num=8;

	if(reg_vc_icm_s_pillar_num>12)
		reg_vc_icm_s_pillar_num=12;

// chen 0426
	// if reg_vc_icm_h_pillar_num=49
	hue_index_th[10]=0;
	hue_index_th[11]=color_icm_dm_icm_hue_segment_0.h_pillar_1;
	hue_index_th[12]=color_icm_dm_icm_hue_segment_0.h_pillar_2;
	hue_index_th[13]=color_icm_dm_icm_hue_segment_1.h_pillar_3;
	hue_index_th[14]=color_icm_dm_icm_hue_segment_1.h_pillar_4;
	hue_index_th[15]=color_icm_dm_icm_hue_segment_2.h_pillar_5;
	hue_index_th[16]=color_icm_dm_icm_hue_segment_2.h_pillar_6;
	hue_index_th[17]=color_icm_dm_icm_hue_segment_3.h_pillar_7;
	hue_index_th[18]=color_icm_dm_icm_hue_segment_3.h_pillar_8;
	hue_index_th[19]=color_icm_dm_icm_hue_segment_4.h_pillar_9;
	hue_index_th[20]=color_icm_dm_icm_hue_segment_4.h_pillar_10;
	hue_index_th[9]=6144;
	hue_index_th[8]=color_icm_dm_icm_hue_segment_23.h_pillar_47;
	hue_index_th[7]=color_icm_dm_icm_hue_segment_22.h_pillar_46;
	hue_index_th[6]=color_icm_dm_icm_hue_segment_22.h_pillar_45;
	hue_index_th[5]=color_icm_dm_icm_hue_segment_21.h_pillar_44;
	hue_index_th[4]=color_icm_dm_icm_hue_segment_21.h_pillar_43;
	hue_index_th[3]=color_icm_dm_icm_hue_segment_20.h_pillar_42;
	hue_index_th[2]=color_icm_dm_icm_hue_segment_20.h_pillar_41;
	hue_index_th[1]=color_icm_dm_icm_hue_segment_19.h_pillar_40;
	hue_index_th[0]=color_icm_dm_icm_hue_segment_19.h_pillar_39;

	// if reg_vc_icm_s_pillar_num=12
	sat_index_th[0]=0;
	sat_index_th[1]=color_icm_dm_icm_sat_segment_0.s_pillar_1;
	sat_index_th[2]=color_icm_dm_icm_sat_segment_0.s_pillar_2;
	sat_index_th[3]=color_icm_dm_icm_sat_segment_1.s_pillar_3;
	sat_index_th[4]=color_icm_dm_icm_sat_segment_1.s_pillar_4;
	sat_index_th[5]=color_icm_dm_icm_sat_segment_2.s_pillar_5;
	sat_index_th[6]=color_icm_dm_icm_sat_segment_2.s_pillar_6;
	sat_index_th[7]=color_icm_dm_icm_sat_segment_3.s_pillar_7;
	sat_index_th[8]=color_icm_dm_icm_sat_segment_3.s_pillar_8;
	sat_index_th[9]=color_icm_dm_icm_sat_segment_4.s_pillar_9;
	sat_index_th[10]=color_icm_dm_icm_sat_segment_4.s_pillar_10;
	if(reg_vc_icm_ctrl_his_grid_sel==0)
		sat_index_th[11]=4096;
	else
		sat_index_th[11]=8192;

	// if reg_vc_icm_i_pillar_num=8
	int_index_th[0]=0;
	int_index_th[1]=color_icm_dm_icm_int_segment_0.i_pillar_1;
	int_index_th[2]=color_icm_dm_icm_int_segment_0.i_pillar_2;
	int_index_th[3]=color_icm_dm_icm_int_segment_1.i_pillar_3;
	int_index_th[4]=color_icm_dm_icm_int_segment_1.i_pillar_4;
	int_index_th[5]=color_icm_dm_icm_int_segment_2.i_pillar_5;
	int_index_th[6]=color_icm_dm_icm_int_segment_2.i_pillar_6;
	if(reg_vc_icm_ctrl_his_grid_sel==0)
		int_index_th[7]=4096;
	else
		int_index_th[7]=8192;
// end chen 0426


	// find nearby 8 points
	// H
	if(hue_ori_in<3000 && hue_ori_in>=0)
	{
		for (i=10; i<=20; i++)
		{
			if(hue_ori_in<hue_index_th[i])
			{
				hue_low_index=i-1;
				break;
			}
			else
				hue_low_index=19;
		}
	}
	else
	{
		for (i=0; i<=9; i++)
		{
			if(hue_ori_in<hue_index_th[i])
			{
				hue_low_index=i-1;
				if (hue_low_index < 0) hue_low_index = 0;
				break;
			}
			else
				hue_low_index=8;
		}
	}

	//S
	for (i=0; i<=reg_vc_icm_s_pillar_num-1; i++)
	{
		if(sat_ori_in<sat_index_th[i])
		{
			sat_low_index=i-1;
			break;
		}
		else
			sat_low_index=reg_vc_icm_s_pillar_num-1-1;
	}
	if(sat_low_index<0)
		sat_low_index=0;


	//I
	for (i=0; i<=reg_vc_icm_i_pillar_num-1; i++)
	{
		if(int_ori_in<int_index_th[i])
		{
			int_low_index=i-1;
			break;
		}
		else
			int_low_index=reg_vc_icm_i_pillar_num-1-1;
	}
	if(int_low_index<0)
		int_low_index=0;


	// 8 points
	for(ii = 0; ii < 2; ii++)
	{
		ii2=ii+int_low_index;

		for(is = 0; is < 2; is++)
		{
			is2=is+sat_low_index;

			for(ih = 0; ih < 2; ih++)
			{
				int hue_low_index_t;

				if(hue_low_index>=10)
					hue_low_index_t=hue_low_index-10;
				else
					hue_low_index_t=hue_low_index+reg_vc_icm_h_pillar_num-1-9;

				if(hue_low_index_t>58)
					hue_low_index_t=58;

				ih2=ih+hue_low_index_t;
				ih2o=ih+hue_low_index;

// chen 0426
				HSI_8point[ii][is][ih].H = ICM_TAB_ACCESS[ii2][is2][ih2].H;
				HSI_8point[ii][is][ih].S = ICM_TAB_ACCESS[ii2][is2][ih2].S;
				HSI_8point[ii][is][ih].I = ICM_TAB_ACCESS[ii2][is2][ih2].I;
// end chen 0426

				// delta_H, delta_S, delta_I
				// chen 0426
				d_HSI_8point[ii][is][ih].H = HSI_8point[ii][is][ih].H-hue_index_th[ih2o];
				d_HSI_8point[ii][is][ih].S = HSI_8point[ii][is][ih].S-sat_index_th[is2];
				d_HSI_8point[ii][is][ih].I = HSI_8point[ii][is][ih].I-int_index_th[ii2];
				// end chen 0426

			}
		}
	}

	// delta_interpolation from ICM
	sat_weight=(sat_ori_in-sat_index_th[sat_low_index])*100/(sat_index_th[sat_low_index+1]-sat_index_th[sat_low_index]);
	hue_weight=(hue_ori_in-hue_index_th[hue_low_index])*100/(hue_index_th[hue_low_index+1]-hue_index_th[hue_low_index]);
	int_weight=(int_ori_in-int_index_th[int_low_index])*100/(int_index_th[int_low_index+1]-int_index_th[int_low_index]);

	delta_s_inter=
	(
	(
	(d_HSI_8point[0][0][0].S*(100-sat_weight)+d_HSI_8point[0][1][0].S*sat_weight)/100*(100-hue_weight)+
	(d_HSI_8point[0][0][1].S*(100-sat_weight)+d_HSI_8point[0][1][1].S*sat_weight)/100*hue_weight
	)/100*(100-int_weight)+
	(
	(d_HSI_8point[1][0][0].S*(100-sat_weight)+d_HSI_8point[1][1][0].S*sat_weight)/100*(100-hue_weight)+
	(d_HSI_8point[1][0][1].S*(100-sat_weight)+d_HSI_8point[1][1][1].S*sat_weight)/100*hue_weight
	)/100*(int_weight)
	)/100;

	delta_h_inter=
	(
	(
	(d_HSI_8point[0][0][0].H*(100-hue_weight)+d_HSI_8point[0][0][1].H*hue_weight)/100*(100-sat_weight)+
	(d_HSI_8point[0][1][0].H*(100-hue_weight)+d_HSI_8point[0][1][1].H*hue_weight)/100*sat_weight
	)/100*(100-int_weight)+
	(
	(d_HSI_8point[1][0][0].H*(100-hue_weight)+d_HSI_8point[1][0][1].H*hue_weight)/100*(100-sat_weight)+
	(d_HSI_8point[1][1][0].H*(100-hue_weight)+d_HSI_8point[1][1][1].H*hue_weight)/100*sat_weight
	)/100*(int_weight)
	)/100;


	*hue_delta=delta_h_inter;
	*sat_delta=delta_s_inter;

	// chen 0528
/*	if(rtd_inl(0xb802e4f0)==3)
	{
		rtd_pr_vpq_ai_emerg("hue_info=%d, sat_info=%d, int_info=%d\n",hue_info,sat_info,val_info);
		rtd_pr_vpq_ai_emerg("hue_pillar_num=%d, sat_pillar_num=%d, int_pillar_num=%d\n",reg_vc_icm_h_pillar_num,reg_vc_icm_s_pillar_num,reg_vc_icm_i_pillar_num);
		rtd_pr_vpq_ai_emerg("hue_low_index=%d, sat_low_index=%d, int_low_index=%d\n",hue_low_index,sat_low_index,int_low_index);
		rtd_pr_vpq_ai_emerg("Hpoint0=%d, Spoint0=%d, Ipoint0=%d\n",HSI_8point[0][0][0].H,HSI_8point[0][0][0].S,HSI_8point[0][0][0].I);
		rtd_pr_vpq_ai_emerg("delta_h_inter=%d, delta_s_inter=%d\n",delta_h_inter,delta_s_inter);
	}
*/


//end chen 0528
#endif
}
// end 0417

#if 0 // V4L2_ERR
// lesley 0910
void drvif_color_get_DB_AI_DCC(CHIP_DCC_T *ptr)
{
	int i;
	
	ai_db_set.dcc_enhance = ptr->stAIDccGain[0].ai_dcc_enhance_en_face;

	for(i=0; i<8; i++)
		ai_db_set.dcc_curve[i] = ptr->stAIDccGain[0].AiDccCurve[i];
}

void drvif_color_get_DB_AI_ICM(CHIP_CM_RESULT_T *v4l2_data)
{
	int i;
	
	//ai_db_set.icm_offset_h = v4l2_data->stCMColorGain.stAISkinGain.stAiOffset[0].stAIHueOffset;
	//ai_db_set.icm_offset_s = v4l2_data->stCMColorGain.stAISkinGain.stAiOffset[0].stAISaturationOffset;
	//ai_db_set.icm_offset_i = v4l2_data->stCMColorGain.stAISkinGain.stAiOffset[0].stAIIntensityOffset;

	for(i=0; i<8; i++)
		ai_db_set.icm_curve[i] = v4l2_data->stCMColorGain.stAISkinGain.stAiIcmCurve[i];
}

void drvif_color_get_DB_AI_SHP(CHIP_SHARPNESS_UI_T *ptCHIP_SHARPNESS_UI_T)
{
	ai_db_set.shp_edge_gain_pos = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.ai_shp_gain_pos;
	ai_db_set.shp_edge_gain_neg = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.ai_shp_gain_neg;
	ai_db_set.shp_texture_gain_pos = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.ai_shp_gain_pos;
	ai_db_set.shp_texture_gain_neg = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.ai_shp_gain_neg;
	ai_db_set.shp_ver_edge_gain_pos = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.ai_shp_vertical_edge_gain_pos;
	ai_db_set.shp_ver_edge_gain_neg = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.ai_shp_vertical_edge_gain_neg;
	ai_db_set.shp_ver_texture_gain_pos = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.ai_shp_vertical_gain_pos;
	ai_db_set.shp_ver_texture_gain_neg = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.ai_shp_vertical_gain_neg;
}

void drvif_color_set_DB_AI_DCC(void)
{
	ai_ctrl.ai_global2.dcc_enhance_en = ai_db_set.dcc_enhance;

	ai_ctrl.ai_global2.dcc_uv_blend_ratio0 = ai_db_set.dcc_curve[0];
	ai_ctrl.ai_global2.dcc_uv_blend_ratio1 = ai_db_set.dcc_curve[1];
	ai_ctrl.ai_global2.dcc_uv_blend_ratio2 = ai_db_set.dcc_curve[2];
	ai_ctrl.ai_global2.dcc_uv_blend_ratio3 = ai_db_set.dcc_curve[3];
	ai_ctrl.ai_global2.dcc_uv_blend_ratio4 = ai_db_set.dcc_curve[4];
	ai_ctrl.ai_global2.dcc_uv_blend_ratio5 = ai_db_set.dcc_curve[5];
	ai_ctrl.ai_global2.dcc_uv_blend_ratio6 = ai_db_set.dcc_curve[6];
	ai_ctrl.ai_global2.dcc_uv_blend_ratio7 = ai_db_set.dcc_curve[7];
}

void drvif_color_set_DB_AI_ICM(void)
{		
 	ai_ctrl.ai_global.icm_uv_blend_ratio0 = ai_db_set.icm_curve[0];
 	ai_ctrl.ai_global.icm_uv_blend_ratio1 = ai_db_set.icm_curve[1];
 	ai_ctrl.ai_global.icm_uv_blend_ratio2 = ai_db_set.icm_curve[2];
 	ai_ctrl.ai_global.icm_uv_blend_ratio3 = ai_db_set.icm_curve[3];
 	ai_ctrl.ai_global.icm_uv_blend_ratio4 = ai_db_set.icm_curve[4];
 	ai_ctrl.ai_global.icm_uv_blend_ratio5 = ai_db_set.icm_curve[5];
 	ai_ctrl.ai_global.icm_uv_blend_ratio6 = ai_db_set.icm_curve[6];
 	ai_ctrl.ai_global.icm_uv_blend_ratio7 = ai_db_set.icm_curve[7];
}

void drvif_color_set_DB_AI_SHP(void)
{
	ai_ctrl.ai_shp_tune.edg_gain_level = ai_db_set.shp_edge_gain_pos;
	ai_ctrl.ai_shp_tune.edg_gain_neg_level = ai_db_set.shp_edge_gain_neg;
	ai_ctrl.ai_shp_tune.tex_gain_level = ai_db_set.shp_texture_gain_pos;
	ai_ctrl.ai_shp_tune.tex_gain_neg_level = ai_db_set.shp_texture_gain_neg;
	ai_ctrl.ai_shp_tune.vpk_edg_gain_level = ai_db_set.shp_ver_edge_gain_pos;
	ai_ctrl.ai_shp_tune.vpk_edg_gain_neg_level = ai_db_set.shp_ver_edge_gain_neg;
	ai_ctrl.ai_shp_tune.vpk_gain_level = ai_db_set.shp_ver_texture_gain_pos;
	ai_ctrl.ai_shp_tune.vpk_gain_neg_level = ai_db_set.shp_ver_texture_gain_neg;

}

// end lesley 0910
#endif
// lesley 1002_1
void drivef_tool_ai_info_set(int idx, int h_norm, int s_norm, int i_norm)
{	
	tool_ai_info.icm[idx].h_norm = h_norm;
	tool_ai_info.icm[idx].s_norm = s_norm;
	tool_ai_info.icm[idx].i_norm = i_norm;
	
	tool_ai_info.icm[idx].x =  face_icm_apply[buf_idx_w][idx].pos_x_s;
	tool_ai_info.icm[idx].y = face_icm_apply[buf_idx_w][idx].pos_y_s;
	tool_ai_info.icm[idx].wt = AI_detect_value_blend[idx];
}

void drivef_tool_ai_info_get(TOOL_AI_INFO *ptr) // read from sharing memory
{
	int i=0;
	
    if (ptr==NULL)
		return;

	memset(ptr, 0, sizeof(TOOL_AI_INFO));

	for(i=0; i<6; i++)
	{
    	ptr->icm[i].x = tool_ai_info.icm[i].x;
    	ptr->icm[i].y = tool_ai_info.icm[i].y;
    	ptr->icm[i].h_norm = tool_ai_info.icm[i].h_norm;
    	ptr->icm[i].s_norm = tool_ai_info.icm[i].s_norm;
    	ptr->icm[i].i_norm = tool_ai_info.icm[i].i_norm;
		ptr->icm[i].wt = tool_ai_info.icm[i].wt;
	}
}
// end lesley 1002_1

// lesley 1016
void drivef_ai_tune_icm_set(DRV_AI_Tune_ICM_table *ptr)
{
		// (0) hue
        ai_ctrl.ai_icm_tune.hue_target_lo3 = ptr->hue_tune.hue_target_lo3;
        ai_ctrl.ai_icm_tune.hue_target_lo2 = ptr->hue_tune.hue_target_lo2;
        ai_ctrl.ai_icm_tune.hue_target_lo1 = ptr->hue_tune.hue_target_lo1;
        ai_ctrl.ai_icm_tune.hue_target_hi1 = ptr->hue_tune.hue_target_hi1;
        ai_ctrl.ai_icm_tune.hue_target_hi2 = ptr->hue_tune.hue_target_hi2;
        ai_ctrl.ai_icm_tune.hue_target_hi3 = ptr->hue_tune.hue_target_hi3;
        ai_ctrl.ai_icm_tune2.hue_target_lo2_ratio = ptr->hue_tune.hue_target_lo2_ratio;
        ai_ctrl.ai_icm_tune2.hue_target_hi2_ratio = ptr->hue_tune.hue_target_hi2_ratio;
        ai_ctrl.ai_icm_tune2.h_adj_th_p_norm = ptr->hue_tune.h_adj_th_p_norm;
        ai_ctrl.ai_icm_tune2.h_adj_th_n_norm = ptr->hue_tune.h_adj_th_n_norm;

		// (1) sat
        ai_ctrl.ai_icm_tune.sat_target_lo3 = ptr->sat_tune.sat_target_lo3;
        ai_ctrl.ai_icm_tune.sat_target_lo2 = ptr->sat_tune.sat_target_lo2;
        ai_ctrl.ai_icm_tune.sat_target_lo1 = ptr->sat_tune.sat_target_lo1;
        ai_ctrl.ai_icm_tune.sat_target_hi1 = ptr->sat_tune.sat_target_hi1;
        ai_ctrl.ai_icm_tune.sat_target_hi2 = ptr->sat_tune.sat_target_hi2;
        ai_ctrl.ai_icm_tune.sat_target_hi3 = ptr->sat_tune.sat_target_hi3;
        ai_ctrl.ai_icm_tune2.sat_target_lo2_ratio = ptr->sat_tune.sat_target_lo2_ratio;
        ai_ctrl.ai_icm_tune2.sat_target_hi2_ratio = ptr->sat_tune.sat_target_hi2_ratio;
        ai_ctrl.ai_icm_tune2.s_adj_th_p_norm = ptr->sat_tune.s_adj_th_p_norm;
        ai_ctrl.ai_icm_tune2.s_adj_th_n_norm = ptr->sat_tune.s_adj_th_n_norm;

		// (2) int
        ai_ctrl.ai_icm_tune2.val_target_lo1 = ptr->val_tune.val_target_lo1;
        ai_ctrl.ai_icm_tune2.val_target_hi1 = ptr->val_tune.val_target_hi1;
        ai_ctrl.ai_icm_tune2.val_target_lo2_ratio = ptr->val_tune.val_target_lo2_ratio;
        ai_ctrl.ai_icm_tune2.val_target_hi2_ratio = ptr->val_tune.val_target_hi2_ratio;
        ai_ctrl.ai_icm_tune2.v_adj_th_p_norm = ptr->val_tune.v_adj_th_p_norm;
        ai_ctrl.ai_icm_tune2.v_adj_th_n_norm = ptr->val_tune.v_adj_th_n_rorm;

}

void drivef_ai_tune_icm_get(DRV_AI_Tune_ICM_table *ptr) // read from sharing memory
{
        if (ptr==NULL) return;

		memset(ptr, 0, sizeof(DRV_AI_Tune_ICM_table));

		// (0) hue
        ptr->hue_tune.hue_target_lo3 = ai_ctrl.ai_icm_tune.hue_target_lo3;
        ptr->hue_tune.hue_target_lo2 = ai_ctrl.ai_icm_tune.hue_target_lo2;
        ptr->hue_tune.hue_target_lo1 = ai_ctrl.ai_icm_tune.hue_target_lo1;
        ptr->hue_tune.hue_target_hi1 = ai_ctrl.ai_icm_tune.hue_target_hi1;
        ptr->hue_tune.hue_target_hi2 = ai_ctrl.ai_icm_tune.hue_target_hi2;
        ptr->hue_tune.hue_target_hi3 = ai_ctrl.ai_icm_tune.hue_target_hi3;
        ptr->hue_tune.hue_target_lo2_ratio = ai_ctrl.ai_icm_tune2.hue_target_lo2_ratio;
        ptr->hue_tune.hue_target_hi2_ratio = ai_ctrl.ai_icm_tune2.hue_target_hi2_ratio;
        ptr->hue_tune.h_adj_th_p_norm = ai_ctrl.ai_icm_tune2.h_adj_th_p_norm;
        ptr->hue_tune.h_adj_th_n_norm = ai_ctrl.ai_icm_tune2.h_adj_th_n_norm;

		// (1) sat
        ptr->sat_tune.sat_target_lo3 = ai_ctrl.ai_icm_tune.sat_target_lo3;
        ptr->sat_tune.sat_target_lo2 = ai_ctrl.ai_icm_tune.sat_target_lo2;
        ptr->sat_tune.sat_target_lo1 = ai_ctrl.ai_icm_tune.sat_target_lo1;
        ptr->sat_tune.sat_target_hi1 = ai_ctrl.ai_icm_tune.sat_target_hi1;
        ptr->sat_tune.sat_target_hi2 = ai_ctrl.ai_icm_tune.sat_target_hi2;
        ptr->sat_tune.sat_target_hi3 = ai_ctrl.ai_icm_tune.sat_target_hi3;
        ptr->sat_tune.sat_target_lo2_ratio = ai_ctrl.ai_icm_tune2.sat_target_lo2_ratio;
        ptr->sat_tune.sat_target_hi2_ratio = ai_ctrl.ai_icm_tune2.sat_target_hi2_ratio;
        ptr->sat_tune.s_adj_th_p_norm = ai_ctrl.ai_icm_tune2.s_adj_th_p_norm;
        ptr->sat_tune.s_adj_th_n_norm = ai_ctrl.ai_icm_tune2.s_adj_th_n_norm;

		// (2) int
        ptr->val_tune.val_target_lo1 = ai_ctrl.ai_icm_tune2.val_target_lo1;
        ptr->val_tune.val_target_hi1 = ai_ctrl.ai_icm_tune2.val_target_hi1;
        ptr->val_tune.val_target_lo2_ratio = ai_ctrl.ai_icm_tune2.val_target_lo2_ratio;
        ptr->val_tune.val_target_hi2_ratio = ai_ctrl.ai_icm_tune2.val_target_hi2_ratio;
        ptr->val_tune.v_adj_th_p_norm = ai_ctrl.ai_icm_tune2.v_adj_th_p_norm;
        ptr->val_tune.v_adj_th_n_rorm = ai_ctrl.ai_icm_tune2.v_adj_th_n_norm;

}

void drivef_ai_tune_dcc_set(DRV_AI_Tune_DCC_table *ptr, unsigned char enable)
{
		color_dcc_d_dcc_ctrl_RBUS color_dcc_d_dcc_ctrl_reg;
		color_dcc_d_dcc_ctrl_reg.regValue=IoReg_Read32(COLOR_DCC_D_DCC_CTRL_reg);
		color_dcc_d_dcc_ctrl_reg.dcc_user_curve_en = enable;
		IoReg_Write32(COLOR_DCC_D_DCC_CTRL_reg ,  color_dcc_d_dcc_ctrl_reg.regValue );

		// dcc user curve
		memcpy(&dcc_user_curve32[0], ptr->dcc_user, sizeof(int)*32);

		// lesley 1017
		fwif_color_dcc_Curve_interp_tv006(dcc_user_curve32, dcc_user_curve129);
		
		dcc_user_curve_write_flag = 1;
		// end lesley 1017
}

void drivef_ai_tune_dcc_get(DRV_AI_Tune_DCC_table *ptr)
{
        if (ptr==NULL)
			return;

		memcpy(ptr->dcc_user, dcc_user_curve32, sizeof(int)*32);
}

void drivef_ai_dcc_user_curve_get(int *ptr)
{
        if (ptr==NULL)
			return;

		memcpy(ptr, dcc_user_curve129, sizeof(int)*129);
}
// end lesley 1016
#endif
