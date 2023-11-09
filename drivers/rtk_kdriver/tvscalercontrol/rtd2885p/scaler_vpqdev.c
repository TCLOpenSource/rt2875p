/*Kernel Header file*/
#ifndef BUILD_QUICK_SHOW
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/fs.h>		/* everything... */
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/suspend.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/pageremap.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/hrtimer.h>
#include <linux/vmalloc.h>
#include <linux/poll.h>
#include <rtk_kdriver/quick_show/quick_show.h>
#else
#include <no_os/slab.h>
#include <include/string.h>
#include <timer.h>
#include <sysdefs.h>
#include <no_os/printk.h>
#include <no_os/semaphore.h>
#include <no_os/spinlock.h>
#include <no_os/spinlock_types.h>
#include <malloc.h>
#include <div64.h>
#include <rtd_log/rtd_module_log.h>
#include <no_os/pageremap.h>
#include <no_os/math64.h>

#include <qs_pq_setting.h>
#include <rtk_kdriver/quick_show/quick_show.h>
#include <scaler/scalerCommon.h>
#endif

#include <rtk_kdriver/io.h>

#ifdef CONFIG_ARM64
#include <asm/io.h>
#endif

/*RBUS Header file*/

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
#else
#include <scalercommon/scalerCommon.h>
#endif

#ifdef CONFIG_COMPAT
#include <linux/compat.h>
#define to_user_ptr(x)          compat_ptr((unsigned int)x)
#else
#define to_user_ptr(x)          ((void* __user)(x)) // convert 32 bit value to user pointer
#endif


/*TVScaler Header file*/
#include "tvscalercontrol/io/ioregdrv.h"
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/scaler/scalercolorlib_tv002.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>
#include <tvscalercontrol/vip/viptable.h>
#include <scaler_vpqmemcdev.h>
#include "scaler_vpqdev.h"
#include "scaler_vscdev.h"
#include "ioctrl/vpq/vpq_cmd_id.h"
#include <ioctrl/vpq/vpq_extern_cmd_id.h>
#include <tvscalercontrol/vip/pq_rpc.h>
//#include <mach/RPCDriver.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/pll27x_reg_reg.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/gibi_od.h>
#include <tvscalercontrol/vip/pcid.h>
#include <tvscalercontrol/vip/valc.h>
#include <tvscalercontrol/vip/fcic.h>
#include <tvscalercontrol/vip/ultrazoom.h>
#include <tvscalercontrol/vip/still_logo_detect.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/scalerdrv/scalerip.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <rtd_log/rtd_module_log.h>
#include <tvscalercontrol/panel/panelapi.h>
#ifdef CONFIG_RTK_LOW_POWER_MODE
#include <rtk_kdriver/rtk-kdrv-common.h>
#endif
#include <rbus/timer_reg.h>
//#include <vo/rtk_vo.h>

#include "vgip_isr/scalerVIP.h"
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
#include <tvscalercontrol/vip/scalerPQMaskCtrl.h>
#include <tvscalercontrol/vip/scalerPQMaskColorLib.h>
#endif

#ifndef CONFIG_MEMC_NOTSUPPORT
#include "memc_reg_def.h"
#endif
#ifndef BUILD_QUICK_SHOW
//#include <gal/rtk_se_export.h>
//#include <rbus/se_reg.h>
//#include <gal/rtk_se_lib.h>
#include <rtk_kdriver/rtk_semaphore.h>
#include "vgip_isr/scalerAI.h"
#include <rtk_ai.h>
#include <scaler_vpqleddev.h>
#include <vgip_isr/scalerAiDebug.h>
#include <vgip_isr/scalerAIOptee.h>
//#include <rbus/h3ddma_reg.h> // lesley

#ifdef UT_flag
#ifdef memcmp
#undef memcmp
#endif
#include <string.h>
#endif //UT_flag

#define LGDB_OFF
#ifndef UT_flag
static dev_t vpq_devno;/*vpq device number*/
static struct cdev vpq_cdev;
#endif //UT_flag
#endif
static struct semaphore VPQ_Semaphore;
static struct semaphore VPQ_ICM_Semaphore;
struct semaphore VPQ_DM_DMA_TBL_Semaphore;
struct semaphore VPQ_HDR_CSC_Semaphore;
struct semaphore VPQ_Demura_Semaphore;
struct semaphore VPQ_DM_Y_histogram_ctrl_reg_Semaphore;
struct semaphore VPQ_IP_Bypass_Semaphore;
struct semaphore VPQ_NNSR_Model_TBL_Semaphore;
//struct semaphore VPQ_DM_3DLUT_Semaphore;
//struct semaphore VPQ_DM_EOTF_Semaphore;
//struct semaphore VPQ_DM_OETF_Semaphore;
extern struct semaphore Gamma_Semaphore;
struct semaphore VPQ_I_CSC_Semaphore;
struct semaphore VPQ_DI_RTNR_CONTROL_Semaphore;

extern struct semaphore VPQ_ld_running_Semaphore;/*For adaptive streaming info Semaphore*/
extern struct semaphore VPQ_LED_Semaphore;
//extern webos_info_t  webos_tooloption;
extern void scalerAI_execute_NN(void); // josh 1018
extern struct semaphore Orbit_Semaphore;
extern unsigned char get_MEMC_bypass_status_refer_platform_model(void);
extern unsigned char aipq_DynamicContrastLevel;
unsigned char g_Writ_Gamma_disr=0;

DECLARE_WAIT_QUEUE_HEAD(g_ai_buf_wait);

#if 1
/* for demura hal function */
#define DeMura_tv006FMT_to_tv001FMT(arg) ((arg>127)?((128-arg)):(arg))

#define VPQ_CMD_CROSSMODLE_MODE0	"testmode=0\n"
#define VPQ_CMD_CROSSMODLE_MODE1	"testmode=1\n"
#define VPQ_CMD_CROSSMODLE_MODE2	"testmode=2\n"

#define deMura_7blk_Block 63
unsigned char deMura_7Blk_Decode_TBL[deMura_7blk_Block] = {
   /*R_L,  R_M,  R_H,  G_L,  G_M,  G_H,  B_L,  B_M,  B_H,  */
     0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,    /* block 0*/
     0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,    /* block 1*/
     0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,    /* block 2*/
     0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,    /* block 3*/
     0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,    /* block 4*/
     0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,    /* block 5*/
     0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,    /* block 6*/
};
#if 0	/* "Demura_LMMH_4ch_TBL" is too large, use dynamic allocate */
unsigned char Demura_LMMH_4ch_TBL[Demura_LMMH_TBL_Size];
#else
unsigned char *Demura_LMMH_4ch_TBL = NULL;
#endif
extern VIP_DeMura_TBL DeMura_TBL;
#endif


#if 1
/*========================= for INNX demura =================================*/
#if 0	/* use dynamic alloc*/
extern char INNX_Demura_DeLut1[INNX_Demura_h_271][INNX_Demura_W16];
extern char INNX_Demura_DeLut2[INNX_Demura_h_271][INNX_Demura_W16];
extern char INNX_Demura_DeLut3[INNX_Demura_h_271][INNX_Demura_W16];
extern char INNX_Demura_DeLut4[INNX_Demura_h_271][INNX_Demura_W16];
extern char INNX_Demura_DeLut5[INNX_Demura_h_271][INNX_Demura_W16];
#else
#endif
extern unsigned int INNX_Demura_gray_setting[7];
unsigned char tv006_decontour_level=10;
/*========================= for INNX demura =================================*/
#endif

#undef VIPprintf
#ifdef CONFIG_SCALER_ENABLE_V4L2
#define V4L2printf_cmd(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_V4L2_interface1_DEBUG,fmt,##args)
#define V4L2printf_Info(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_V4L2_interface2_DEBUG,fmt,##args)
#endif

extern UINT16 HDR10_3DLUT_24x24x24[24*24*24*3];
extern UINT32 EOTF_LUT_R[1025];
extern UINT16 OETF_LUT_R[1025];
extern UINT32 EOTF_LUT_Linear[1025];
extern UINT16 OETF_LUT_Linear[1025];
static unsigned char PQ_Dev_Status = PQ_DEV_NOTHING;
static unsigned char Demo_Flag = false;
//UINT8 g_InvGamma=0;
static GAMMA_RGB_T gamma;
extern COLORELEM_TAB_T icm_tab_elem_of_vip_table;
extern COLORELEM_TAB_T icm_tab_elem_write;
UINT8 g_Color_Mode = COLOR_MODE_ADV;
UINT8 g_flag_cm_adv_init_ok;
UINT8 g_flag_cm_exp_init_ok;
CHIP_CM_REGION_EXT_T cmRegionExt;//for CM ioctl tmps

extern UINT8 MEMC_First_Run_Done;
extern UINT8 MEMC_First_Run_force_SetInOutUseCase_Done;
extern UINT8 MEMC_First_Run_FBG_enable;//using for booting fbg checking
extern void drvif_color_Set_Sharpness_VPK3_init(void);
extern void Scaler_MEMC_Set_VpqLowDelayToMEMC_Flag(unsigned char u1_mode);

unsigned char str_resume_do_picturemode=0;
#ifdef CONFIG_RTK_SRE_AATS_UCT_AT
//AT V6.2 Patch 2022/03/31 - START
unsigned int g_ScalerApplyPQLinear = 0;
char         g_ActiveAATS_UCT_GD_On = 0;
unsigned char PQ_UT_apply_Linear_Flag;
//Required misc/rtd_logger.c and emcu/rtk_kdv_emcu.c
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS) && (IS_ENABLED(CONFIG_REALTEK_LOGBUF))
extern unsigned int str_status;
#else
extern void get_local_time(unsigned int* ,unsigned int* , unsigned int* , unsigned int*);
#endif
//AT V6.2 Patch 2022/03/31 - END
//AT V6.3 Patch 2022/08/04 - START
unsigned int g_AATS_UCT_AT_Switch = 0;
//AT V6.3 Patch 2022/08/04 - END
#endif

CHIP_CM_REGION_T g_cm_rgn_adv = {{
	{{	TRUE,
		CHIP_CM_COLOR_REGION_RED,		//INDEX_0	//INDEX_1	//INDEX_2	//INDEX_3	//INDEX_4	//INDEX_5//INDEX_6//INDEX_7
		{5632, 5760, 5888, 5888, 0, 64, 128, 192 }, //HUE_index //0~6016
		{174, 220, 220, 220, 220, 220, 190, 170 }, //HUE_gain //0~255
		{384, 512, 1024, 1536, 2048, 3072, 3584, 4096 }, //SAT_index //0~4095
		{128, 215, 230, 220, 200, 201, 180, 130 }, //SAT_gain //0~255
		{0, 585, 1170, 1752, 2340, 2925, 3510, 4096 }, //ITN_index //0~4095
		{128, 250, 250, 250, 250, 180, 180, 160 }, //ITN_gain //0~255
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //R offset by hue,	   //0~2048 ==> -1024~1023
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //G offset by hue,	   //0~2048 ==> -1024~1023
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //B offset by hue,	   //0~2048 ==> -1024~1023
	},

	{	TRUE,
		CHIP_CM_COLOR_REGION_GRN,		//INDEX_0	//INDEX_1	//INDEX_2	//INDEX_3	//INDEX_4	//INDEX_5//INDEX_6//INDEX_7
		{1152, 1216, 1408, 1536, 2432, 2560, 2688, 2752 }, //HUE_index //0~6016
		{174, 220, 220, 220, 220, 220, 190, 170 }, //HUE_gain //0~255
		{384, 512, 1024, 1536, 2048, 3072, 3584, 4096 }, //SAT_index //0~4095
		{128, 215, 230, 220, 200, 201, 180, 130 }, //SAT_gain //0~255
		{0, 585, 1170, 1752, 2340, 2925, 3510, 4096 }, //ITN_index //0~4095
		{128, 250, 250, 250, 250, 180, 180, 160 }, //ITN_gain //0~255
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //R offset by hue,	   //0~2048 ==> -1024~1023
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //G offset by hue,	   //0~2048 ==> -1024~1023
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //B offset by hue,	   //0~2048 ==> -1024~1023
	},

	{	TRUE,
		CHIP_CM_COLOR_REGION_BLU,		//INDEX_0	//INDEX_1	//INDEX_2	//INDEX_3	//INDEX_4	//INDEX_5//INDEX_6//INDEX_7
		{2752, 2816, 2944, 3072, 3456, 3584, 3712, 3840 }, //HUE_index //0~6016
		{174, 218, 220, 220, 220, 220, 190, 170 }, //HUE_gain //0~255
		{384, 512, 1024, 1536, 2048, 3072, 3584, 4096 }, //SAT_index //0~4095
		{128, 193, 230, 220, 200, 201, 180, 130 }, //SAT_gain //0~255
		{0, 585, 1170, 1752, 2340, 2925, 3510, 4096 }, //ITN_index //0~4095
		{110, 160, 220, 220, 220, 180, 180, 160 }, //ITN_gain //0~255
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //R offset by hue,	   //0~2048 ==> -1024~1023
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //G offset by hue,	   //0~2048 ==> -1024~1023
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //B offset by hue,	   //0~2048 ==> -1024~1023
	},

	{	TRUE,
		CHIP_CM_COLOR_REGION_CYN,		//INDEX_0	//INDEX_1	//INDEX_2	//INDEX_3	//INDEX_4	//INDEX_5//INDEX_6//INDEX_7
		{3840, 3840, 3968, 4096, 4224, 4352, 4480, 4480 }, //HUE_index //0~6016
		{174, 174, 220, 220, 220, 220, 190, 190 }, //HUE_gain //0~255
		{384, 512, 1024, 1536, 2048, 3072, 3584, 4096 }, //SAT_index //0~4095
		{128, 206, 230, 220, 200, 201, 180, 130 }, //SAT_gain //0~255
		{0, 585, 1170, 1752, 2340, 2925, 3510, 4096 }, //ITN_index //0~4095
		{128, 250, 250, 250, 250, 180, 180, 160 }, //ITN_gain //0~255
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //R offset by hue,	   //0~2048 ==> -1024~1023
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //G offset by hue,	   //0~2048 ==> -1024~1023
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //B offset by hue,	   //0~2048 ==> -1024~1023
	},

	{	TRUE,
		CHIP_CM_COLOR_REGION_MGT,		//INDEX_0	//INDEX_1	//INDEX_2	//INDEX_3	//INDEX_4	//INDEX_5//INDEX_6//INDEX_7
		{4672, 4736, 4864, 5120, 5248, 5504, 5760, 5888 }, //HUE_index //0~6016
		{174, 220, 220, 220, 220, 220, 190, 170 }, //HUE_gain //0~255
		{384, 512, 1024, 1536, 2048, 3072, 3584, 4096 }, //SAT_index //0~4095
		{128, 215, 230, 220, 200, 201, 180, 130 }, //SAT_gain //0~255
		{0, 585, 1170, 1752, 2340, 2925, 3510, 4096 }, //ITN_index //0~4095
		{128, 250, 250, 250, 250, 180, 180, 160 }, //ITN_gain //0~255
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //R offset by hue,	   //0~2048 ==> -1024~1023
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //G offset by hue,	   //0~2048 ==> -1024~1023
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //B offset by hue,	   //0~2048 ==> -1024~1023
	},

	{	TRUE,
		CHIP_CM_COLOR_REGION_YLW,		//INDEX_0	//INDEX_1	//INDEX_2	//INDEX_3	//INDEX_4	//INDEX_5//INDEX_6//INDEX_7
		{640, 704, 704, 768, 832, 832, 896, 1024 }, //HUE_index //0~6016
		{174, 220, 220, 220, 220, 220, 190, 170 }, //HUE_gain //0~255
		{384, 512, 1024, 1536, 2048, 3072, 3584, 4096 }, //SAT_index //0~4095
		{128, 215, 230, 220, 200, 201, 180, 130 }, //SAT_gain //0~255
		{0, 585, 1170, 1752, 2340, 2925, 3510, 4096 }, //ITN_index //0~4095
		{128, 250, 250, 250, 250, 180, 180, 160 }, //ITN_gain //0~255
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //R offset by hue,	   //0~2048 ==> -1024~1023
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //G offset by hue,	   //0~2048 ==> -1024~1023
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //B offset by hue,	   //0~2048 ==> -1024~1023
	},

	{	TRUE,
		CHIP_CM_COLOR_REGION_SKIN,		//INDEX_0	//INDEX_1	//INDEX_2	//INDEX_3	//INDEX_4	//INDEX_5//INDEX_6//INDEX_7
		{256, 320, 320, 384, 384, 384, 512, 640 }, //HUE_index //0~6016
		{174, 220, 220, 220, 220, 220, 190, 170 }, //HUE_gain //0~255
		{384, 512, 1024, 1536, 2048, 3072, 3584, 4096 }, //SAT_index //0~4095
		{128, 215, 230, 220, 200, 201, 180, 130 }, //SAT_gain //0~255
		{0, 585, 1170, 1752, 2340, 2925, 3510, 4096 }, //ITN_index //0~4095
		{128, 250, 250, 250, 250, 180, 180, 160 }, //ITN_gain //0~255
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //R offset by hue,	   //0~2048 ==> -1024~1023
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //G offset by hue,	   //0~2048 ==> -1024~1023
		{1024, 1024, 1024, 1024, 1024, 1024, 1024, 1024, }, //B offset by hue,	   //0~2048 ==> -1024~1023
	},},
	{32, 32, 32, 32, 32, 32, 32, 32, }, //gain by sat for offset by hue,	   //0~64
	{32, 32, 32, 32, 32, 32, 32, 32, }, //gain by int for offset by hue,	   //0~64
	1,
}};


CHIP_CM_REGION_T g_cm_rgn_exp;
CHIP_COLOR_CONTROL_T g_cm_ctrl_adv;
CHIP_COLOR_CONTROL_T g_cm_ctrl_exp;


unsigned char g_cm_need_refresh = 1;
unsigned char g_cm_keep_Y = 0;

extern short gamutMatrix_APPLY[3][3];

extern UINT16 GOut_R[1025];
extern UINT16 GOut_G[1025];
extern UINT16 GOut_B[1025];

extern unsigned int final_GAMMA_R[512];
extern unsigned int final_GAMMA_G[512];
extern unsigned int final_GAMMA_B[512];

extern unsigned short Gamma_sRGB2Linear_14bit[1025];
extern unsigned short Gamma_Linear2sRGB_14bit[1025];

UINT32 PQModeInfo_flag[5] = {0};
unsigned char g_LGE_HDR_CSC_CTRL = 2; //BT2020
unsigned char g_InvGammaPowerMode=0;
unsigned char g_IsInvGammaPowerNewMode=0;
unsigned char g_HDR3DLUTForceWrite = 1;
extern unsigned short Power1div22Gamma[1025];
extern unsigned short Power22InvGamma[1025];
unsigned short g_GammaRemap[4097];

unsigned char uc3DCorrectionEn;
extern unsigned char bForceHist3DCtrl;

unsigned char g_srgbForceUpdate=1;

static unsigned char ucColorFilterMode;
static COLORTEMP_ELEM_T curColorTemp = {512, 512, 512, 512, 512, 512};
DRV_Sharpness_Table tDRV_Sharpness_Table;

//Game mode cmd checking flow @Cixus 20161204
extern unsigned int game_mode_cmd_check;
#ifndef BUILD_QUICK_SHOW
unsigned int vpq_project_id = 0x00060000; /*pre_id:16 main_id:8 sub1_id:8 sub2_id:8, example->lg=tv006*/
#else
unsigned int vpq_project_id = 0x00010000; /*pre_id:16 main_id:8 sub1_id:8 sub2_id:8, example->lg=tv006*/
#endif

UINT8 game_process = 0;

CHIP_SHARPNESS_UI_T tCurCHIP_SHARPNESS_UI_T;
CHIP_SHARPNESS_UI_T t3DCHIP_SHARPNESS_UI_T;
CHIP_EDGE_ENHANCE_UI_T tCurCHIP_EDGE_ENHANCE_UI_T;
CHIP_SR_UI_T tCurCHIP_SR_UI_T;
CHIP_LOCAL_CONTRAST_T glc_param={
	0,0,0,128,
	{0,0,1,1,2,2,2,2},
	{0,128,256,384,512,768,896,1023}, //1st : 0 final:1023 , please don't change
	{
		0,128,192,256,320,384,448,512,576,640,704,768,832,896,960,1024,1088,1152,1216,1280,1344,1408,1472,1536,1600,1664,1728,1792,1856,1920,1984,2048,
		0,128,192,256,320,384,448,512,576,640,704,768,832,896,960,1024,1088,1152,1216,1280,1344,1408,1472,1536,1600,1664,1728,1792,1856,1920,1984,2048,
		0,128,256,384,512,640,768,896,1024,1152,1280,1408,1536,1664,1792,1920,2048,2176,2304,2432,2560,2688,2816,2944,3072,3200,3328,3456,3584,3712,3840,3968,
		0,128,256,384,512,640,768,896,1024,1152,1280,1408,1536,1664,1792,1920,2048,2176,2304,2432,2560,2688,2816,2944,3072,3200,3328,3456,3584,3712,3840,3968,
		0,2048,2112,2176,2240,2304,2368,2432,2496,2560,2624,2688,2752,2816,2880,2944,3008,3072,3136,3200,3264,3328,3392,3456,3520,3584,3648,3712,3776,3840,3904,3968,
		0,2048,2112,2176,2240,2304,2368,2432,2496,2560,2624,2688,2752,2816,2880,2944,3008,3072,3136,3200,3264,3328,3392,3456,3520,3584,3648,3712,3776,3840,3904,3968,
		0,2048,2112,2176,2240,2304,2368,2432,2496,2560,2624,2688,2752,2816,2880,2944,3008,3072,3136,3200,3264,3328,3392,3456,3520,3584,3648,3712,3776,3840,3904,3968,
		0,2048,2112,2176,2240,2304,2368,2432,2496,2560,2624,2688,2752,2816,2880,2944,3008,3072,3136,3200,3264,3328,3392,3456,3520,3584,3648,3712,3776,3840,3904,3968,
	},
	1,0,0,0
};

unsigned short tShp_Val[7];
static unsigned char g_prevent_force_write_dcc = 0;

unsigned char g_bAPLColorGainClr = 1;
unsigned char g_bGamut3DLUT_LGDB_NeedUpdated = 0;
unsigned char g_3DLUT_LastInternalSelect = 0; // 0: RTK DB, 1: LG DB
unsigned char g_3DLUT_LastEnableSatus = 0;
unsigned char g_3DLUT_Resume = 0;
//extern GAMUT_3D_LUT_17x17x17_T g_LGDB_3D_LUT_BUF[1]; //no more used
GAMUT_3D_LUT_17x17x17_T g_buf3DLUT_LGDB;

extern struct semaphore Memc_Realcinema_Semaphore;/*For adaptive streaming info Semaphore*/
extern DEBUG_SCALER_VSC_STRUCT DbgSclrFlgTkr;//extern char memc_realcinema_run_flag;

unsigned char g_bDIGameModeOnlyDebug = FALSE;

extern unsigned char g_bLDinited;

extern unsigned char rtk_vsc_setFilmMode(bool value);
extern unsigned char rtk_hal_vsc_GetInputRegion(VIDEO_WID_T wid, KADP_VIDEO_RECT_T * pinregion);
extern unsigned char rtk_hal_vsc_SetInputRegion(VIDEO_WID_T wid, KADP_VIDEO_RECT_T  inregion);
extern unsigned char rtk_hal_vsc_GetOutputRegion(VIDEO_WID_T wid, KADP_VIDEO_RECT_T * poutregion);
extern unsigned char rtk_hal_vsc_SetOutputRegion(VIDEO_WID_T wid, KADP_VIDEO_RECT_T outregion, unsigned short Wide, unsigned short High);

extern void memc_realcinema_framerate(void);
extern unsigned char get_sub_OutputVencMode(void);
extern void drvif_module_yc_boot_init(void);

extern PQ_device_struct *g_Share_Memory_PQ_device_struct;

#ifdef CONFIG_HDR_SDR_SEAMLESS
extern HDR_SDR_SEAMLESS_PQ_STRUCT HDR_SDR_SEAMLESS_PQ;
#endif

#if defined(CONFIG_RTK_8KCODEC_INTERFACE)
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
extern void VPQEX_rlink_AI_SeneInfo(void);
#endif
#endif

#if IS_ENABLED(CONFIG_RTK_AI_DRV)

// 0622 lsy
// lesley 0905
//UINT8 vpq_stereo_face = 0;//0: ap off, 1: all on, 2: demo mode, 3: scene pq off, 4: face pq off, 5: all pq off
//UINT8 vpq_stereo_face_pre = 0;
// end lesley 0905
// 0604 lsy
//UINT8 vpq_stereo_face_secure = 0;// backup for secure mode
// end 0604 lsy
RTK_AI_PQ_mode aipq_mode;
RTK_AI_PQ_mode aipq_mode_pre;
// end 0622 lsy

// lesley 09120
extern unsigned char signal_cnt;
// end lesley 0920

// for scene_detection
extern DRV_AI_SCENE_Ctrl_table ai_scene_ctrl;
extern int scalerAI_pq_mode_ctrl(RTK_AI_PQ_mode ai_pq_mode, unsigned char dcValue);
#endif
unsigned char g_bDeContourSaved = 0;
RTK_DECONTOUR_T De_contour_level;
DRV_film_mode film_mode;
//DRV_film_table_t2 film_table;
DRV_film_table_t film_table;

#ifndef BUILD_QUICK_SHOW

#ifndef UT_flag

extern unsigned char PQ_LED_Dev_Status;
int vpq_open(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t  vpq_read(struct file *filep, char *buffer, size_t count, loff_t *offp)
{
#if 1
#ifdef CONFIG_RTK_SRE_AATS_UCT_AT
    //AT V6.2 Patch 2022/03/31 - START
    char activeAATS_UCT_GD_On;
    if(buffer == NULL) {
        rtd_pr_vpq_emerg("####[David] %s buffer is empty\n", __func__);
        return 0;
    }
    if(count == 0 ) {
        rtd_pr_vpq_emerg("####[David] %s count  is empty\n", __func__);
        return 0;
    }
    activeAATS_UCT_GD_On = g_ActiveAATS_UCT_GD_On;
    *buffer =activeAATS_UCT_GD_On;
    //AT V6.2 Patch 2022/03/31 - END
#endif
	return count;
#else
	return 0;
#endif

}

#if 1 // for VPQ self check
static unsigned char vpq_Cal_bit_is1_Num(int val)
{
    unsigned char bit1_Num = 0;

    while (val)
    {
        bit1_Num++;
        val = (val - 1) & val;
    }

    return bit1_Num;
}

static unsigned char vpq_Cal_bit_shift(int val)
{
	unsigned char bit_shift;

	if (((val & _BIT0) != 0) || (val == 0)) {
		bit_shift = 0;
	} else {
		bit_shift = vpq_Cal_bit_is1_Num(val-1) - vpq_Cal_bit_is1_Num(val) + 1;
	}

	return bit_shift;
}

char vpq_IP_Status_Check_handler(void)
{
	unsigned int i;
	unsigned int clk_val, val;
	unsigned int reg_addr;
	unsigned char reg_shift;
	unsigned int reg_mask;
	unsigned int reg_addr_clk;
	unsigned char reg_shift_clk;
	unsigned int reg_mask_clk;
	unsigned char bit_size;
	unsigned char *name;
	unsigned char num_max;
	VIP_IP_Status_Check_List *VIP_IP_Status_Check_List_TBL = NULL;

	//num_max = sizeof(VIP_IP_Status_Check_List_TBL) / sizeof(VIP_IP_Status_Check_List);
	VIP_IP_Status_Check_List_TBL = drvif_Get_IP_Status_Check_List_TBL(&num_max);

	if (VIP_IP_Status_Check_List_TBL == NULL) {
		rtd_printk(KERN_EMERG,"VIP_IP_Status","VIP_IP_Status_Check_List_TBL = NULL\n");
		return -1;
	}

	for (i=0;i<num_max;i++) {
		name = &VIP_IP_Status_Check_List_TBL[i].name[0];
		reg_addr = VIP_IP_Status_Check_List_TBL[i].reg_addr;
		reg_mask = VIP_IP_Status_Check_List_TBL[i].reg_mask;
		reg_shift = vpq_Cal_bit_shift(reg_mask);
		bit_size = vpq_Cal_bit_is1_Num(reg_mask);

		reg_addr_clk = VIP_IP_Status_Check_List_TBL[i].reg_addr_clk;
		reg_mask_clk = VIP_IP_Status_Check_List_TBL[i].reg_mask_clk;
		reg_shift_clk = vpq_Cal_bit_shift(reg_mask_clk);

		if (reg_addr_clk != 0) {	// need to check clk
			clk_val = drvif_color_get_reg_Val(reg_addr_clk, reg_shift_clk, reg_mask_clk);
			if (clk_val == 1) { // clk is ok
				val = drvif_color_get_reg_Val(reg_addr, reg_shift, reg_mask);

				rtd_printk(KERN_EMERG,"VIP_IP_Status","%d.%s(0x%x[%d:%d])=%d,clk(0x%x[%d])=%d,\n",
					i ,name ,reg_addr, reg_shift+bit_size-1, reg_shift, val, reg_addr_clk, reg_shift_clk, clk_val);

			} else {
				rtd_printk(KERN_EMERG,"VIP_IP_Status","%d.%s(0x%x[%d:%d]),clk(0x%x[%d])=%d,\n",
					i ,name ,reg_addr, reg_shift+bit_size-1, reg_shift, reg_addr_clk, reg_shift_clk, clk_val);

			}

		} else {	// skip clk checking
			val = drvif_color_get_reg_Val(reg_addr, reg_shift, reg_mask);
			rtd_printk(KERN_EMERG,"VIP_IP_Status","%d.%s(0x%x[%d:%d])=%d,\n",
				i ,name ,reg_addr, reg_shift+bit_size-1, reg_shift, val);

		}
	}
	return 0;
}

char vpq_self_Check_handler(void)
{
	unsigned int *EOTF_table32_R, *EOTF_table32_G, *EOTF_table32_B, *EOTF_table32;
	unsigned short *OETF_table16_R, *OETF_table16_G, *OETF_table16_B, *OETF_table16;
	unsigned int *coef;
	unsigned short panel_width, panel_heigth, h_s, h_e, v_s, v_e;
	unsigned short x, y, color_bar_wid;
	unsigned short R, G, B;
	unsigned short dataaccess_bit_num, dataaccess_half_val;
	char ch_result;
	unsigned char curve_isFail = 0;
	unsigned int i;
	VIP_Self_Check_Reg_Get Self_Check_Reg_Get;
	unsigned int temp;
	unsigned int *pMax_Min_Inv, *pMax, *pMin, *pInv;
	_system_setting_info *vipSys_info = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	char HDR_type_str[30] = {0};

	ch_result = 0;

	if(vipSys_info == NULL){
		rtd_printk(KERN_EMERG,"VIP_self_Check","vipSys_info = NULL, fail\n");
		ch_result--;
	} else {
		if (vipSys_info->DolbyHDR_flag == 1)
			//rtd_printk(KERN_EMERG,"VIP_self_Check","self check with dolby vision content\n");
			sprintf(HDR_type_str, "Dolby Vision HDR");
		else if (vipSys_info ->HDR_flag == HAL_VPQ_HDR_MODE_SDR)
			//rtd_printk(KERN_EMERG,"VIP_self_Check","self check with HAL_VPQ_HDR_MODE_SDR\n");
			sprintf(HDR_type_str, "SDR Content");
		else if (vipSys_info ->HDR_flag == HAL_VPQ_HDR_MODE_HDR10)
			//rtd_printk(KERN_EMERG,"VIP_self_Check","self check with HAL_VPQ_HDR_MODE_HDR10\n");
			sprintf(HDR_type_str, "HDR10 Content");
		else if (vipSys_info ->HDR_flag == HAL_VPQ_HDR_MODE_HLG)
			//rtd_printk(KERN_EMERG,"VIP_self_Check","self check with HAL_VPQ_HDR_MODE_HLG\n");
			sprintf(HDR_type_str, "HLG Content");
		else if (vipSys_info ->HDR_flag == HAL_VPQ_HDR_MODE_SDR_MAX_RGB)
			//rtd_printk(KERN_EMERG,"VIP_self_Check","self check with HAL_VPQ_HDR_MODE_SDR_MAX_RGB\n");
			sprintf(HDR_type_str, "SDR Max RGB Content");
		else
			//rtd_printk(KERN_EMERG,"VIP_self_Check","self check with unknow stream=%d\n", vipSys_info ->HDR_flag);
			sprintf(HDR_type_str, "Unknow Content");
		rtd_printk(KERN_EMERG,"VIP_self_Check","self check with %s\n", HDR_type_str);

		// hdr dm check
		drvif_Self_Check_Reg_Get(SLR_MAIN_DISPLAY, &Self_Check_Reg_Get);
		// check eotf
		rtd_printk(KERN_EMERG,"VIP_self_Check","EOTF check\n");
		if (Self_Check_Reg_Get.eotf_lut_enable == 1) {
			EOTF_table32 = dvr_malloc(sizeof(int)*EOTF_size*3);
			if (EOTF_table32 != NULL) {
				EOTF_table32_R = EOTF_table32 + sizeof(int)*EOTF_size*0;
				EOTF_table32_G = EOTF_table32 + sizeof(int)*EOTF_size*1;
				EOTF_table32_B = EOTF_table32 + sizeof(int)*EOTF_size*2;
				drvif_DM2_EOTF_Get(EOTF_table32_R, EOTF_table32_G, EOTF_table32_B);

				// table check, curve is monotonically increasing or not
				curve_isFail = 0;
				for (i=0;i<(EOTF_size-1);i++) {
					if ((EOTF_table32_R[i] > EOTF_table32_R[i+1]) || (EOTF_table32_G[i] > EOTF_table32_G[i+1])
						|| (EOTF_table32_B[i] > EOTF_table32_B[i+1])) {
						curve_isFail = 1;
						break;
					}
				}

				if (curve_isFail == 0)
					rtd_printk(KERN_EMERG,"VIP_self_Check","EOTF is monotonically increasing\n");
				else {
					rtd_printk(KERN_EMERG,"VIP_self_Check","EOTF is not monotonically increasing, EOTF check fail\n");
					ch_result--;
				}

				// table check, curve is all 0 or not
				curve_isFail = 1;
				for (i=0;i<EOTF_size;i++) {
					if ((EOTF_table32_R[i] != 0) || (EOTF_table32_G[i] != 0) || (EOTF_table32_B[i] != 0)) {
						curve_isFail = 0;
						break;
					}
				}

				if (curve_isFail == 0)
					rtd_printk(KERN_EMERG,"VIP_self_Check","EOTF is not all 0\n");
				else {
					rtd_printk(KERN_EMERG,"VIP_self_Check","EOTF is all 0, EOTF check fail\n");
					ch_result--;
				}

				dvr_free((void *)EOTF_table32);
			} else {
				ch_result--;
				rtd_printk(KERN_EMERG,"VIP_self_Check","EOTF malloc fail\n");
			}
		} else {
			rtd_printk(KERN_EMERG,"VIP_self_Check","EOTF is off, skip check\n");
		}

		// check b01-04
		rtd_printk(KERN_EMERG,"VIP_self_Check","dm 3x3 check\n");
		if (Self_Check_Reg_Get.dm_b0104_enable == 1) {
			coef = dvr_malloc(sizeof(int)*9);
			if (coef != NULL) {
				drvif_HDR_RGB2OPT_Get((unsigned char*)&temp, coef);
				temp = 0;
				for (i=0;i<9;i++)
					temp = temp + coef[i];
				if (temp != 0)
					rtd_printk(KERN_EMERG,"VIP_self_Check","dm 3x3 is not all 0\n");
				else {
					rtd_printk(KERN_EMERG,"VIP_self_Check","dm 3x3 is all 0, dm 3x3  check fail\n");
					ch_result--;
				}

				dvr_free((void *)coef);
			} else {
				ch_result--;
				rtd_printk(KERN_EMERG,"VIP_self_Check","dm 3x3 malloc fail\n");
			}
		} else {
			rtd_printk(KERN_EMERG,"VIP_self_Check","dm 3x3 is off, skip check\n");
		}

		// check oetf
		rtd_printk(KERN_EMERG,"VIP_self_Check","OETF check\n");
		if (Self_Check_Reg_Get.oetf_lut_enable == 1) {
			OETF_table16 = dvr_malloc(sizeof(short)*OETF_size*3);
			if (OETF_table16 != NULL) {
				OETF_table16_R = OETF_table16 + sizeof(int)*OETF_size*0;
				OETF_table16_G = OETF_table16 + sizeof(int)*OETF_size*1;
				OETF_table16_B = OETF_table16 + sizeof(int)*OETF_size*2;
				drvif_DM2_OETF_Get(OETF_table16_R, OETF_table16_G, OETF_table16_B);
#if 0	// gain curve, skip
				// table check, curve is monotonically increasing or not
				curve_isFail = 0;
				for (i=0;i<(OETF_size-1);i++) {
					if ((OETF_table16_R[i] > OETF_table16_R[i+1]) || (OETF_table16_G[i] > OETF_table16_G[i+1]) 
						|| (OETF_table16_B[i] > OETF_table16_B[i+1])) {
						curve_isFail = 1;
						break;
					}
				}

				if (curve_isFail == 0)
					rtd_printk(KERN_EMERG,"VIP_self_Check","OETF is monotonically increasing\n");
				else {
					rtd_printk(KERN_EMERG,"VIP_self_Check","OETF is not monotonically increasing, OETF check fail\n");
					ch_result--;
				}
#endif
				// table check, curve is all 0 or not
				curve_isFail = 1;
				for (i=0;i<OETF_size;i++) {
					if ((OETF_table16_R[i] != 0) || (OETF_table16_G[i] != 0) || (OETF_table16_B[i] != 0)) {
						curve_isFail = 0;
						break;
					}
				}

				if (curve_isFail == 0)
					rtd_printk(KERN_EMERG,"VIP_self_Check","OETF is not all 0\n");
				else {
					rtd_printk(KERN_EMERG,"VIP_self_Check","OETF is all 0, OETF check fail\n");
					ch_result--;
				}

				dvr_free((void *)OETF_table16);
			} else {
				ch_result--;
				rtd_printk(KERN_EMERG,"VIP_self_Check","OETF malloc fail\n");
			}
		} else {
			rtd_printk(KERN_EMERG,"VIP_self_Check","OETF is off, skip check\n");
		}

		// check b05-01
		rtd_printk(KERN_EMERG,"VIP_self_Check","CSC3 check\n");
		if (Self_Check_Reg_Get.dm_b0501_enable == 1) {
			pMax_Min_Inv = dvr_malloc(sizeof(int)*8);
			if (pMax_Min_Inv != NULL) {
				pMax = pMax_Min_Inv;
				pMin = (pMax_Min_Inv+2);
				pInv = (pMax_Min_Inv+5);
				drvif_DM_HDR_3dLUT_max_min_inv_get((unsigned char*)&temp, pMax, pMin, pInv);

				if ((pInv[0] != 0) && (pInv[1] != 0) && (pInv[2] != 0))
					rtd_printk(KERN_EMERG,"VIP_self_Check","CSC3 Inv c1-3 is not 0\n");
				else {
					rtd_printk(KERN_EMERG,"VIP_self_Check","CSC3 Inv c1-3 check fail, CINV=%d,%d,%d,\n", pInv[0], pInv[1], pInv[2]);
					ch_result--;
				}

				dvr_free((void *)pMax_Min_Inv);
			} else {
				ch_result--;
				rtd_printk(KERN_EMERG,"VIP_self_Check","CSC3 malloc fail\n");
			}
		} else {
			rtd_printk(KERN_EMERG,"VIP_self_Check","CSC3 is off, skip check\n");
		}

		drvif_color_Get_main_den_h_v_start_end(&h_s, &h_e, &v_s, &v_e);
		panel_width = h_e - h_s;
		panel_heigth = v_e - v_s;
		dataaccess_bit_num = vpq_Cal_bit_is1_Num(drvif_color_get_pcid_data_access_read_R_mask());
		dataaccess_half_val = 1 << (dataaccess_bit_num-1);

		// data access check by line od
		color_bar_wid = panel_width/7;

		rtd_printk(KERN_EMERG,"VIP_self_Check","Color bar check, panel_width=%d, panel_heigth=%d, h_s=%d, h_e=%d, v_s=%d, v_e=%d, color_bar_wid=%d, dataaccess_bit_num=%d, dataaccess_half_val=%d\n", 
			panel_width, panel_heigth, h_s, h_e, v_s, v_e, color_bar_wid, dataaccess_bit_num, dataaccess_half_val);

		// check white point,
		y = panel_heigth>>3;	// 1/8
		x = color_bar_wid>>1;
		drvif_color_set_pcid_data_access_read(x, y,	1, 1);
		drvif_color_get_pcid_data_access_read(&R, &G, &B);
		if ((R>dataaccess_half_val) && (G>dataaccess_half_val) && (B>dataaccess_half_val)) {
			rtd_printk(KERN_EMERG,"VIP_self_Check","white point is ok, [%d,%d]=R:G:B=%d,%d,%d, ch_result=%d\n", x, y, R, G, B, ch_result);
		} else {
			ch_result--;
			rtd_printk(KERN_EMERG,"VIP_self_Check","white point fail, [%d,%d]=R:G:B=%d,%d,%d, ch_result=%d\n", x, y, R, G, B, ch_result);
		}

		// check yellow point,
		y = panel_heigth>>3;	// 1/8
		x = (color_bar_wid>>1) + (color_bar_wid * 1);
		drvif_color_set_pcid_data_access_read(x, y,	1, 1);
		drvif_color_get_pcid_data_access_read(&R, &G, &B);
		if ((R>dataaccess_half_val) && (G>dataaccess_half_val) && (B<dataaccess_half_val)) {
			rtd_printk(KERN_EMERG,"VIP_self_Check","yellow point is ok, [%d,%d]=R:G:B=%d,%d,%d, ch_result=%d\n", x, y, R, G, B, ch_result);
		} else {
			ch_result--;
			rtd_printk(KERN_EMERG,"VIP_self_Check","yellow point fail, [%d,%d]=R:G:B=%d,%d,%d, ch_result=%d\n", x, y, R, G, B, ch_result);
		}

		// check cyan point,
		y = panel_heigth>>3;	// 1/8
		x = (color_bar_wid>>1) + (color_bar_wid * 2);
		drvif_color_set_pcid_data_access_read(x, y,	1, 1);
		drvif_color_get_pcid_data_access_read(&R, &G, &B);
		if ((R<dataaccess_half_val) && (G>dataaccess_half_val) && (B>dataaccess_half_val)) {
			rtd_printk(KERN_EMERG,"VIP_self_Check","cyan point is ok, [%d,%d]=R:G:B=%d,%d,%d, ch_result=%d\n", x, y, R, G, B, ch_result);
		} else {
			ch_result--;
			rtd_printk(KERN_EMERG,"VIP_self_Check","cyan point fail, [%d,%d]=R:G:B=%d,%d,%d, ch_result=%d\n", x, y, R, G, B, ch_result);
		}

		// check green point,
		y = panel_heigth>>3;	// 1/8
		x = (color_bar_wid>>1) + (color_bar_wid * 3);
		drvif_color_set_pcid_data_access_read(x, y,	1, 1);
		drvif_color_get_pcid_data_access_read(&R, &G, &B);
		if ((R<dataaccess_half_val) && (G>dataaccess_half_val) && (B<dataaccess_half_val)) {
			rtd_printk(KERN_EMERG,"VIP_self_Check","green point is ok, [%d,%d]=R:G:B=%d,%d,%d, ch_result=%d\n", x, y, R, G, B, ch_result);
		} else {
			ch_result--;
			rtd_printk(KERN_EMERG,"VIP_self_Check","green point fail, [%d,%d]=R:G:B=%d,%d,%d, ch_result=%d\n", x, y, R, G, B, ch_result);
		}

		// check Magenta point,
		y = panel_heigth>>3;	// 1/8
		x = (color_bar_wid>>1) + (color_bar_wid * 4);
		drvif_color_set_pcid_data_access_read(x, y,	1, 1);
		drvif_color_get_pcid_data_access_read(&R, &G, &B);
		if ((R>dataaccess_half_val) && (G<dataaccess_half_val) && (B>dataaccess_half_val)) {
			rtd_printk(KERN_EMERG,"VIP_self_Check","Magenta point is ok, [%d,%d]=R:G:B=%d,%d,%d, ch_result=%d\n", x, y, R, G, B, ch_result);
		} else {
			ch_result--;
			rtd_printk(KERN_EMERG,"VIP_self_Check","Magenta point fail, [%d,%d]=R:G:B=%d,%d,%d, ch_result=%d\n", x, y, R, G, B, ch_result);
		}

		// check Red point
		y = panel_heigth>>3;	// 1/8
		x = (color_bar_wid>>1) + (color_bar_wid * 5);
		drvif_color_set_pcid_data_access_read(x, y,	1, 1);
		drvif_color_get_pcid_data_access_read(&R, &G, &B);
		if ((R>dataaccess_half_val) && (G<dataaccess_half_val) && (B<dataaccess_half_val)) {
			rtd_printk(KERN_EMERG,"VIP_self_Check","Red point is ok, [%d,%d]=R:G:B=%d,%d,%d, ch_result=%d\n", x, y, R, G, B, ch_result);
		} else {
			ch_result--;
			rtd_printk(KERN_EMERG,"VIP_self_Check","Red point fail, [%d,%d]=R:G:B=%d,%d,%d, ch_result=%d\n", x, y, R, G, B, ch_result);
		}

		// check Blue point
		y = panel_heigth>>3;	// 1/8
		x = (color_bar_wid>>1) + (color_bar_wid * 6);
		drvif_color_set_pcid_data_access_read(x, y,	1, 1);
		drvif_color_get_pcid_data_access_read(&R, &G, &B);
		if ((R<dataaccess_half_val) && (G<dataaccess_half_val) && (B>dataaccess_half_val)) {
			rtd_printk(KERN_EMERG,"VIP_self_Check","Blue point is ok, [%d,%d]=R:G:B=%d,%d,%d, ch_result=%d\n", x, y, R, G, B, ch_result);
		} else {
			ch_result--;
			rtd_printk(KERN_EMERG,"VIP_self_Check","Blue point fail, [%d,%d]=R:G:B=%d,%d,%d, ch_result=%d\n", x, y, R, G, B, ch_result);
		}
	}

	if (ch_result == 0) {
		rtd_printk(KERN_EMERG,"VIP_self_Check","!!!!%s VIP self check final result is ok!!!!\n", HDR_type_str);
	} else {
		rtd_printk(KERN_EMERG,"VIP_self_Check","!!!!%s VIP self check final result is fail!!!!, fail number=%d\n", HDR_type_str, (ch_result*(-1)));
	}

	return 0;
}
#endif
extern unsigned char VPQ_ALL_Linear_byCMD_Flag;
ssize_t vpq_write(struct file *filep, const char *buffer, size_t count, loff_t *offp)
{

	char cmd_buf[128] = {0};
	long ret = count;
#if 1

	if (count >= 128)
		return -EFAULT;

	if (copy_from_user(cmd_buf, buffer, count)) {
		ret = -EFAULT;
                return ret;
	}


	printk(KERN_EMERG "vpqdev cmd : %s \n", cmd_buf);

        if (count > 0){
              cmd_buf[count] = '\0';
        }

	if(strcmp(cmd_buf, "linear_mode=on\n") == 0) {
		VPQ_ALL_Linear_byCMD_Flag = 1;
		Scaler_apply_PQ_Linear(1);
	} else if (strcmp(cmd_buf, "linear_mode=off\n") == 0) {
		VPQ_ALL_Linear_byCMD_Flag = 0;
		Scaler_apply_PQ_Linear(0);
	}

	if(strcmp(cmd_buf, "VPQ_self_test=data_path_status\n") == 0) {
		vpq_IP_Status_Check_handler();
	} else if (strcmp(cmd_buf, "VPQ_self_test=usb_check\n") == 0) {
		vpq_self_Check_handler();
	} else if(strcmp(cmd_buf, VPQ_CMD_CROSSMODLE_MODE1) == 0){
		Scaler_Set_VIP_Disable_PQ(VPQ_ModuleTestMode_PQByPass, 1);
		Scaler_Set_PQ_ModuleTest(VPQ_ModuleTestMode_PQByPass);
		printk(KERN_EMERG "PQ_bypass_mode1\n");
	}else if (strcmp(cmd_buf, VPQ_CMD_CROSSMODLE_MODE2) == 0){
		Scaler_Set_VIP_Disable_PQ(VPQ_ModuleTestMode_HDRPQByPass, 1);
		Scaler_Set_PQ_ModuleTest(VPQ_ModuleTestMode_HDRPQByPass);
		printk(KERN_EMERG "PQ_bypass_mode2\n");
	}else if (strcmp(cmd_buf, VPQ_CMD_CROSSMODLE_MODE0) == 0){
		vpq_boot_init();
		vpq_ioctl_set_stop_run_by_idx(VPQ_IOC_PQ_CMD_INIT,0);
		vpq_extern_ioctl_set_stop_run_by_idx(VPQ_EXTERN_IOC_PQ_CMD_INIT,0);
		Scaler_Set_PQ_ModuleTest(VPQ_ModuleTestMode_MAX);
		printk(KERN_EMERG "PQ_bypass_mode0\n");
	}
//AT V6.3 Patch 2022/08/04 - START
#ifdef CONFIG_RTK_SRE_AATS_UCT_AT
        rtd_pr_vpq_emerg(KERN_EMERG "####[AT-DEBUG] CONFIG_RTK_SRE_AATS_UCT_AT OK\n");
        if(strcmp(cmd_buf, "aats_uct_at_switch=on\n") == 0)
        {
            //dynamic switch on/off for AATS-UCT/AT Test
            g_AATS_UCT_AT_Switch= 1;
            rtd_pr_vpq_emerg("####[David] AATS UCT/AT ON:%d\n", g_AATS_UCT_AT_Switch);
        } else if(strcmp(cmd_buf, "aats_uct_at_switch=off\n") == 0)
        {
            g_AATS_UCT_AT_Switch= 0;
            rtd_pr_vpq_emerg("####[David] AATS UCT/AT OFF:%d\n", g_AATS_UCT_AT_Switch);
        }

        if (g_AATS_UCT_AT_Switch == 1)
        {
            unsigned int hour, minute, second, millisecond;
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS) && (IS_ENABLED(CONFIG_REALTEK_LOGBUF))
            struct tm time;
            if(str_status == 0){
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0))
                struct timespec64 ts;
                ktime_get_real_ts64(&ts);
#else
                struct timeval ts;
                do_gettimeofday(&ts);
#endif
                second = ts.tv_sec;
                millisecond = (ts.tv_nsec/1000000);
                second -= (sys_tz.tz_minuteswest * 60);
                time64_to_tm(second, 0, &time);
                hour = time.tm_hour;
                minute = time.tm_min;
                second = time.tm_sec;
            }
#else
            get_local_time(&hour,&minute,&second,&millisecond);
#endif
            //AT V6.3 Patch 2022/08/04 - END
            if(strcmp(cmd_buf, "linear_mode=on\n") == 0){
            //AT V6.2 Patch 2022/03/31 - START
            PQ_UT_apply_Linear_Flag=1;
            rtd_pr_vpq_emerg("PQ_linear mode flag on \n");
            rtd_pr_vpq_emerg("####[David] PQ_linear mode -hope on- :%d  \n", PQ_UT_apply_Linear_Flag);
            //AT V6.2 Patch 2022/03/31 - END
            //Scaler_apply_PQ_Linear(1);
            //rtd_pr_vpq_emerg("PQ_linear mode on \n");
            } else if (strcmp(cmd_buf, "linear_mode=off\n") == 0){
                //AT V6.2 Patch 2022/03/31 - START
                PQ_UT_apply_Linear_Flag=0;
                Scaler_apply_PQ_Linear(0);
                rtd_pr_vpq_emerg("PQ linear mode flag off \n");
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS) && (IS_ENABLED(CONFIG_REALTEK_LOGBUF))
                if(str_status == 0){
                    rtd_pr_vpq_emerg("####[David] PQ_linear mode off, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n",jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
                }else{
                    rtd_pr_vpq_emerg("####[David] PQ_linear mode off, jiffies: %lu, rtc[ms]:%lu, str_status: %01d\n",jiffies, jiffies*1000/HZ,str_status);
                }
#else
                rtd_pr_vpq_emerg("####[David] PQ_linear mode off, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n",jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
#endif
                //AT V6.2 Patch 2022/03/31 - END
                //Scaler_apply_PQ_Linear(0);
                //rtd_pr_vpq_emerg("PQ linear mode off \n");
            }
            //AT V6.2 Patch 2022/03/31 - START
            if (strcmp(cmd_buf, "gd=on\n") == 0){
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS) && (IS_ENABLED(CONFIG_REALTEK_LOGBUF))
                if(str_status == 0){
                    rtd_pr_vpq_emerg("####[David] gd on, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n", jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
                }else{
                    rtd_pr_vpq_emerg("####[David] gd on, jiffies: %lu, rtc[ms]:%lu, str_status: %01d\n",jiffies, jiffies*1000/HZ,str_status);
                }
#else
                rtd_pr_vpq_emerg("####[David] gd on, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n", jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
#endif
            } else if (strcmp(cmd_buf, "gd=off\n") == 0){
                g_ActiveAATS_UCT_GD_On = 0;
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS) && (IS_ENABLED(CONFIG_REALTEK_LOGBUF))
                if(str_status == 0){
                    rtd_pr_vpq_emerg("####[David] gd off, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n", jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
                }else{
                    rtd_pr_vpq_emerg("####[David] gd off, jiffies: %lu, rtc[ms]:%lu, str_status: %01d\n",jiffies, jiffies*1000/HZ,str_status);
                }
#else
                rtd_pr_vpq_emerg("####[David] gd off, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n", jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
#endif
            } else if (strcmp(cmd_buf, "vr=on\n") == 0){
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS) && (IS_ENABLED(CONFIG_REALTEK_LOGBUF))
                if(str_status == 0){
                    rtd_pr_vpq_emerg("####[David] vr on, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n", jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
                }else{
                    rtd_pr_vpq_emerg("####[David] vr on, jiffies: %lu, rtc[ms]:%lu, str_status: %01d\n",jiffies, jiffies*1000/HZ,str_status);
                }
#else
                rtd_pr_vpq_emerg("####[David] vr on, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n", jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
#endif
            } else if (strcmp(cmd_buf, "vr=off\n") == 0) {
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS) && (IS_ENABLED(CONFIG_REALTEK_LOGBUF))
                if(str_status == 0){
                    rtd_pr_vpq_emerg("####[David] vr off, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n", jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
                }else{
                    rtd_pr_vpq_emerg("####[David] vr off, jiffies: %lu, rtc[ms]:%lu, str_status: %01d\n",jiffies, jiffies*1000/HZ,str_status);
                }
#else
                rtd_pr_vpq_emerg("####[David] vr off, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n", jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
#endif
            }
        //AT V6.2 Patch 2022/03/31 - END
        } else if (g_AATS_UCT_AT_Switch == 0) {
           rtd_pr_vpq_emerg("####[AT-DEBUG] AATS_UCT_AT_Switch: %d\n", g_AATS_UCT_AT_Switch);
        }
#endif
//AT V6.3 Patch 2022/08/04 - END

	if(NULL == g_Share_Memory_PQ_device_struct){
		return count;
	}else{
	if(strcmp(cmd_buf, "enable_pqautocheck\n") == 0){
		g_Share_Memory_PQ_device_struct->pqautocheck_en=1;
		printk(KERN_EMERG "enable_pqautocheck\n");
	}

	if(strcmp(cmd_buf, "disable_pqautocheck\n") == 0){
		g_Share_Memory_PQ_device_struct->pqautocheck_en=0;
		printk(KERN_EMERG "disable_pqautocheck\n");
	}

	if(strcmp(cmd_buf, "pqautocheck_all\n") == 0){
		unsigned int PQ_check_error[200];
		vpq_PQ_check_timming("all", PQ_check_error);
		printk(KERN_EMERG "pqautocheck_all\n");
	}
	}

	return count;
#else
	return 0;
#endif
}

int vpq_release(struct inode *inode, struct file *filep)
{
	return 0;
}

#if IS_ENABLED(CONFIG_RTK_AI_DRV)
unsigned int vpq_poll(struct file *filp, poll_table *wait)
{
	poll_wait(filp, &g_ai_buf_wait, wait);

	if (ScalerAI_CheckBufferReady(filp) == 0) {
		return POLLIN | POLLRDNORM;
	} else {
		return 0;
	}
}
void vpq_wakeUpPoll(void)
{
	wake_up_interruptible(&g_ai_buf_wait);
}
#endif

#endif //UT_flag

void vpq_set_color_temp_filter(void)
{
	COLORTEMP_ELEM_T ct;
	static COLORTEMP_ELEM_T ctStore = {0};
	static unsigned char ucColorFilterModeStore = 0;
	unsigned char waitSync = 0;
	memcpy(&ct, &curColorTemp, sizeof(COLORTEMP_ELEM_T));

	if (ucColorFilterModeStore != ucColorFilterMode ) {
		waitSync = 1;
		ucColorFilterModeStore = ucColorFilterMode;
	}

	if (memcmp(&ctStore, &curColorTemp, sizeof(COLORTEMP_ELEM_T)) != 0) {
		waitSync = 1;
		memcpy(&ctStore, &curColorTemp, sizeof(COLORTEMP_ELEM_T));
	}

	switch (ucColorFilterMode) {
	case 1: /*SET filter gain value to RED only*/
			ct.Green_Gain = ct.Blue_Gain = 0;
			break;
	case 2: /*SET filter gain value to GREEN only*/
			ct.Red_Gain = ct.Blue_Gain = 0;
			break;
	case 3: /*SET filter gain value to BLUE only*/
			ct.Red_Gain = ct.Green_Gain = 0;
			break;
	case 0: /*SET filter gain value to off mode*/
	default:
			break;
			;
	}

	/*
	fwif_color_setrgbcontrast_By_Table(ct.Red_Gain, ct.Green_Gain, ct.Blue_Gain, 0);
	fwif_color_setrgbbrightness_By_Table(ct.Red_Offset, ct.Green_Offset, ct.Blue_Offset);
	*/
	fwif_color_set_color_temp_tv006(TRUE, ct.Red_Gain, ct.Green_Gain, ct.Blue_Gain, (ct.Red_Offset<<4)-8192, (ct.Green_Offset<<4)-8192, (ct.Blue_Offset<<4)-8192, waitSync);
}

void vpq_init_gamma(void)
{
	int i;

	for (i = 0; i < 1024; i++)
		gamma.Gamme_Red[i] = gamma.Gamme_Green[i] = gamma.Gamme_Blue[i] = i;
}

void vpq_gamma_multiply_remap(GAMMA_RGB_T *pGammaRGBIn)
{
	int i, j, hdiff, qdiff;
	if (NULL == pGammaRGBIn)
		return;

	// R
	for (i = 0, j = 0; i < 4096; i+=4, j++) {
		g_GammaRemap[i] = pGammaRGBIn->Gamme_Red[j] << 2;
	}
	g_GammaRemap[4096] = (pGammaRGBIn->Gamme_Red[1023] << 2) + (pGammaRGBIn->Gamme_Red[1023] >> 8);

	for (i = 0; i < 4096; i+=4) {
		hdiff = (g_GammaRemap[i+4]-g_GammaRemap[i])>>1;
		qdiff = hdiff>>1;
		g_GammaRemap[i+1] = g_GammaRemap[i]+qdiff;
		g_GammaRemap[i+2] = g_GammaRemap[i]+hdiff;
		g_GammaRemap[i+3] = g_GammaRemap[i+2]+qdiff;
	}

	for (i = 0; i < 1025; i++) {
		GOut_R[i] = g_GammaRemap[Power1div22Gamma[i]]<<2;
	}

	// G
	for (i = 0, j = 0; i < 4096; i+=4, j++) {
		g_GammaRemap[i] = pGammaRGBIn->Gamme_Green[j] << 2;
	}
	g_GammaRemap[4096] = (pGammaRGBIn->Gamme_Green[1023] << 2) + (pGammaRGBIn->Gamme_Green[1023] >> 8);

	for (i = 0; i < 4096; i+=4) {
		hdiff = (g_GammaRemap[i+4]-g_GammaRemap[i])>>1;
		qdiff = hdiff>>1;
		g_GammaRemap[i+1] = g_GammaRemap[i]+qdiff;
		g_GammaRemap[i+2] = g_GammaRemap[i]+hdiff;
		g_GammaRemap[i+3] = g_GammaRemap[i+2]+qdiff;
	}

	for (i = 0; i < 1025; i++) {
		GOut_G[i] = g_GammaRemap[Power1div22Gamma[i]]<<2;
	}


	// B
	for (i = 0, j = 0; i < 4096; i+=4, j++) {
		g_GammaRemap[i] = pGammaRGBIn->Gamme_Blue[j] << 2;
	}
	g_GammaRemap[4096] = (pGammaRGBIn->Gamme_Blue[1023] << 2) + (pGammaRGBIn->Gamme_Blue[1023] >> 8);

	for (i = 0; i < 4096; i+=4) {
		hdiff = (g_GammaRemap[i+4]-g_GammaRemap[i])>>1;
		qdiff = hdiff>>1;
		g_GammaRemap[i+1] = g_GammaRemap[i]+qdiff;
		g_GammaRemap[i+2] = g_GammaRemap[i]+hdiff;
		g_GammaRemap[i+3] = g_GammaRemap[i+2]+qdiff;
	}

	for (i = 0; i < 1025; i++) {
		GOut_B[i] = g_GammaRemap[Power1div22Gamma[i]]<<2;
	}

}

void vpq_mapping_gamma_by_gamut_mode(void)
{
	int i;
	unsigned char bGammaDo2p2 = FALSE;

	switch(g_InvGammaPowerMode) {
	case VPQ_GAMMA_MODE_INV_OFF_INNER_BYPASS:
	case VPQ_GAMMA_MODE_INV_ON_INNER_BYPASS:
	default:
		bGammaDo2p2 = FALSE;
		break;
	case VPQ_GAMMA_MODE_INV_OFF_INNER_ON:
	case VPQ_GAMMA_MODE_INV_ON_INNER_ON:
		bGammaDo2p2 = TRUE;
		break;
	}

	if (bGammaDo2p2) {
		vpq_gamma_multiply_remap(&gamma);
	} else {
		for (i = 0; i < 1024; i++) {
			GOut_R[i] = (gamma.Gamme_Red[i] << 2) + ((gamma.Gamme_Red[i] >> 8)<<2);
			GOut_G[i] = (gamma.Gamme_Green[i] << 2) + ((gamma.Gamme_Green[i] >> 8)<<2);
			GOut_B[i] = (gamma.Gamme_Blue[i] << 2) + ((gamma.Gamme_Blue[i] >> 8)<<2);
		}

		GOut_R[1024] = GOut_R[1023];
		GOut_G[1024] = GOut_G[1023];
		GOut_B[1024] = GOut_B[1023];
	}

}

void vpq_set_OSD_gamma(void)
{
	int i;
	down(&VPQ_Semaphore);
	for (i = 0; i < 1024; i+=4) {
		GOut_R[i>>2] = (gamma.Gamme_Red[i] << 2); // 10bit to 12bit
		GOut_G[i>>2] = (gamma.Gamme_Green[i] << 2);
		GOut_B[i>>2] = (gamma.Gamme_Blue[i] << 2);
	}

	GOut_R[256] = GOut_R[255];
	GOut_G[256] = GOut_G[255];
	GOut_B[256] = GOut_B[255];

	fwif_color_osd_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B, GOut_R, GOut_G, GOut_B);

	drvif_color_colorwrite_mixer_gamma2_Enable(1);
	drvif_color_colorwrite_mixer_gamma2(final_GAMMA_R, GAMMA_CHANNEL_R);
	drvif_color_colorwrite_mixer_gamma2(final_GAMMA_G, GAMMA_CHANNEL_G);
	drvif_color_colorwrite_mixer_gamma2(final_GAMMA_B, GAMMA_CHANNEL_B);
	up(&VPQ_Semaphore);
}

void vpq_set_gamma(unsigned char ucDoCtrlBack, unsigned char useRPC)
{
#if 0
	down(&VPQ_Semaphore);
	if (!useRPC) {
		vpq_mapping_gamma_by_gamut_mode();
		fwif_color_gamma_control_front(SLR_MAIN_DISPLAY);
		fwif_color_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B, GOut_R, GOut_G, GOut_B);
		/*fwif_color_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B, gamma.Gamme_Red, gamma.Gamme_Green, gamma.Gamme_Blue);*/
		/*WaitFor_DEN_STOP();*/
		fwif_color_set_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B);
		if (ucDoCtrlBack)
			fwif_color_gamma_control_back(SLR_MAIN_DISPLAY, 1);

	} else {
		static UINT32 temp[1536];
		vpq_mapping_gamma_by_gamut_mode();
		//fwif_color_gamma_control_front(SLR_MAIN_DISPLAY);
		fwif_color_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B, GOut_R, GOut_G, GOut_B);

		memcpy(temp, final_GAMMA_R, sizeof(UINT32)*512);
		memcpy(temp+512, final_GAMMA_G, sizeof(UINT32)*512);
		memcpy(temp+1024, final_GAMMA_B, sizeof(UINT32)*512);

		fwif_color_ChangeUINT32Endian(temp, 1536, 1);		//gamma run to d domain isr need enfian

		if (drvif_color_Gamma_Read_Write_Check())
			WaitFor_DEN_START();

#ifndef UT_flag
#ifdef CONFIG_ARM64
		memcpy_toio((UINT32 *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_GAMMA), temp, sizeof(UINT32)*3*(512));
#else
		memcpy((UINT32 *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_GAMMA), temp, sizeof(UINT32)*3*(512));
#endif
		if ((Scaler_SendRPC(SCALERIOC_SET_GAMMA, 0, 0)) < 0) {
			rtd_pr_vpq_err(" update GAMMA to driver fail !!!\n");
		}
#endif //UT_flag
	}
	up(&VPQ_Semaphore);
#endif
}

void vpq_set_inv_gamma(unsigned char ucDoCtrlBack)
{
#if 0
	down(&VPQ_Semaphore);
	fwif_color_inv_gamma_control_front(SLR_MAIN_DISPLAY);
	fwif_color_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B, Power22InvGamma, Power22InvGamma, Power22InvGamma);
	fwif_color_set_inv_gamma_encode_tv006();
	if (ucDoCtrlBack)
		fwif_color_inv_gamma_control_back(SLR_MAIN_DISPLAY, 1);

	up(&VPQ_Semaphore);
#endif	
}

#if 1
/* for Demura hal function*/
char vpq_DeMura_Encode_128x3bit(unsigned char* in_array, unsigned char* encode_array)
{
	unsigned short i,j;
	/* 1 bloock = (R_low + R_mid + R_high) + (G_low + G_mid + G_high) + (B_low + B_mid + B_high)*/
	/* this function will finish "7 block encode", R0,G0,B0 ~ R6,G6,B6. */
	for (j=0;j<3;j++){     /* 3 channel*/
		for(i=0;i<5;i++) { /* 1 channel, 128 bit*/
			// "deMura_TBL" is 6 bit (0x3F)
			encode_array[j*16+i*3+0] = ((in_array[j*21+i*4+0]&0x3F)<<2) + ((in_array[j*21+i*4+1]&0x3F)>>4);
			encode_array[j*16+i*3+1] = ((in_array[j*21+i*4+1]&0x3F)<<4) + ((in_array[j*21+i*4+2]&0x3F)>>2);
			encode_array[j*16+i*3+2] = ((in_array[j*21+i*4+2]&0x3F)<<6) + ((in_array[j*21+i*4+3]&0x3F)>>0);
		}
		encode_array[j*16+i*3] = (in_array[j*21+i*4+0]&0x3F)<<2; // bit dummy
	}
	return 0;
}

unsigned char vpq_demura_SD_Bit_check(unsigned char *LMMH, unsigned int mode)
{
	unsigned int i;
	unsigned char shiftBit_1 = 0, shiftBit_2;
	char tmp = 0, maxV = 0, minV = 0;

	// check R channel
	if ((mode & Demura_LMMH_R_SD_check_BIT)  != 0) {
		for (i=Demura_LMMH_4ch_R_sta;i<Demura_LMMH_4ch_G_sta;i++) {
			tmp = (char)DeMura_tv006FMT_to_tv001FMT(LMMH[i]);
			if (tmp > maxV)
				maxV = tmp;
			if (tmp < minV)
				minV = tmp;
		}
	}

	// check G channel
	if ((mode & Demura_LMMH_G_SD_check_BIT)  != 0) {
		for (i=Demura_LMMH_4ch_G_sta;i<Demura_LMMH_4ch_B_sta;i++) {
			tmp = (char)DeMura_tv006FMT_to_tv001FMT(LMMH[i]);
			if (tmp > maxV)
				maxV = tmp;
			if (tmp < minV)
				minV = tmp;
		}
	}

	// check B channel
	if ((mode & Demura_LMMH_B_SD_check_BIT)  != 0) {
		for (i=Demura_LMMH_4ch_B_sta;i<Demura_LMMH_4ch_W_sta;i++) {
			tmp = (char)DeMura_tv006FMT_to_tv001FMT(LMMH[i]);
			if (tmp > maxV)
				maxV = tmp;
			if (tmp < minV)
				minV = tmp;
		}
	}

	// check W channel
	if ((mode & Demura_LMMH_W_SD_check_BIT)  != 0) {
		for (i=Demura_LMMH_4ch_W_sta;i<Demura_LMMH_4ch_CRC_sta;i++) {
			tmp = (char)DeMura_tv006FMT_to_tv001FMT(LMMH[i]);
			if (tmp > maxV)
				maxV = tmp;
			if (tmp < minV)
				minV = tmp;
		}
	}

	if (maxV > 62)
		shiftBit_1 = 2;
	else if (maxV > 31)
		shiftBit_1 = 1;
	else
		shiftBit_1 = 0;

	if (minV < (-64))
		shiftBit_2 = 2;
	else if (minV < (-32))
		shiftBit_2 = 1;
	else
		shiftBit_2 = 0;

	rtd_pr_vpq_info("vpq_demura_SD_Bit_check, max=%d, min=%d, shiftBit_1=%d, shiftBit_2=%d, mode=%d \n", maxV, minV, shiftBit_1, shiftBit_2, mode);

	return ((shiftBit_1 > shiftBit_2) ? (shiftBit_1) : (shiftBit_2));

}

char vpq_demura_SD(char Val, unsigned char shiftBit)
{
	char ret_val = 0;

	if (shiftBit == 0) {
		ret_val = Val;
	} else {
		ret_val = (Val + (1<<(shiftBit-1)))>>shiftBit;
	}

	return ret_val;
}

void vpq_LMMH_demura_data_to_dma(unsigned char *LMMH,VIP_DeMura_TBL * Demura_TBL, unsigned char mode)
{
	unsigned int i,j,k, index, index_base;
	unsigned short ver_blk_num, hor_384Bit_blk_num;
	unsigned char *pDMA_tmp;
	char tmpL, tmpM1, tmpM2, tmpH;
	unsigned char scalingDown_bit = 0;
	unsigned char *DMA_8bit;

	if (Demura_TBL == NULL || LMMH == NULL) {
		rtd_pr_vpq_emerg("vpq_demura_data_to_dma table NULL ~~\n");
		return;
	}

	DMA_8bit = &Demura_TBL->TBL[0];
	scalingDown_bit = vpq_demura_SD_Bit_check(LMMH, Demura_LMMH_W_SD_check_BIT);
	/* merlin7 add gain to replace scale */
	//Demura_TBL->DeMura_CTRL_TBL.demura_table_scale = scalingDown_bit;
	if(scalingDown_bit==0)
	{
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_l_r = 16;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m_r= 16;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m2_r= 16;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m3_r= 16;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_h_r= 16;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_l_g= 16;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m_g= 16;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m2_g= 16;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m3_g= 16;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_h_g= 16;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_l_b= 16;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m_b= 16;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m2_b= 16;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m3_b= 16;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_h_b= 16;
	}
	else if(scalingDown_bit==1)
	{
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_l_r = 32;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m_r= 32;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m2_r= 32;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m3_r= 32;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_h_r= 32;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_l_g= 32;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m_g= 32;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m2_g= 32;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m3_g= 32;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_h_g= 32;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_l_b= 32;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m_b= 32;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m2_b= 32;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m3_b= 32;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_h_b= 32;
	}
	else
	{
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_l_r = 64;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m_r= 64;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m2_r= 64;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m3_r= 64;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_h_r= 64;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_l_g= 64;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m_g= 64;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m2_g= 64;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m3_g= 64;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_h_g= 64;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_l_b= 64;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m_b= 64;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m2_b= 64;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_m3_b= 64;
		Demura_TBL->DeMura_CTRL_TBL.demura_gain_h_b= 64;
	}

	if (mode == 1) {
		hor_384Bit_blk_num = 69;	/* 481/7+1*/
		ver_blk_num = 271;
	} else {
		hor_384Bit_blk_num = 35;	/*241/7+1*/
		ver_blk_num = 136;
	}

	for(i =0;i<ver_blk_num;i++) {
		/* one line*/
		index = 0;
		index_base = i*Demura_LMMH_W_Size;
		for (j=0;j<hor_384Bit_blk_num;j++) {
			for(k=0;k<(deMura_7blk_Block/9);k++) {
				tmpL = (char)DeMura_tv006FMT_to_tv001FMT(LMMH[Demura_LMMH_4ch_WL_sta + index + index_base]);
				tmpM1 = (char)DeMura_tv006FMT_to_tv001FMT(LMMH[Demura_LMMH_4ch_WM1_sta + index + index_base]);
				tmpM2 = (char)DeMura_tv006FMT_to_tv001FMT(LMMH[Demura_LMMH_4ch_WM2_sta + index + index_base]);
				tmpH = (char)DeMura_tv006FMT_to_tv001FMT(LMMH[Demura_LMMH_4ch_WH_sta + index + index_base]);

				tmpL = vpq_demura_SD(tmpL, scalingDown_bit);	/* low*/
				tmpM1 = vpq_demura_SD(((tmpM1 + tmpM2)>>1), scalingDown_bit); /*Middle*/
				tmpH = vpq_demura_SD(tmpH, scalingDown_bit);	/* High */

				/* R */
				deMura_7Blk_Decode_TBL[k*9+0] = (unsigned char)tmpL;	/* low*/
				deMura_7Blk_Decode_TBL[k*9+1] = (unsigned char)tmpM1; /*Middle*/
				deMura_7Blk_Decode_TBL[k*9+2] = (unsigned char)tmpH;	/* High */

				/* G */
				deMura_7Blk_Decode_TBL[k*9+3] =(unsigned char) tmpL;	/* low*/
				deMura_7Blk_Decode_TBL[k*9+4] = (unsigned char)tmpM1; /*Middle*/
				deMura_7Blk_Decode_TBL[k*9+5] = (unsigned char)tmpH;	/* High */

				/* B */
				deMura_7Blk_Decode_TBL[k*9+6] = (unsigned char)tmpL;	/* low*/
				deMura_7Blk_Decode_TBL[k*9+7] = (unsigned char)tmpM1; /*Middle*/
				deMura_7Blk_Decode_TBL[k*9+8] = (unsigned char)tmpH;	/* High */

				index = (index+1 >= Demura_LMMH_W_Size)?(Demura_LMMH_W_Size-1):(index+1);
			}

			pDMA_tmp = DMA_8bit+(48*(i*hor_384Bit_blk_num+j));
			vpq_DeMura_Encode_128x3bit(deMura_7Blk_Decode_TBL, pDMA_tmp);
		}
	}

}
#endif
#if 1
/*========================= for INNX demura =================================*/
void vpq_INNX_Demura_data_to_dma(VIP_DeMura_TBL * Demura_TBL, char *pDeLut)
{
	unsigned int i,j,k, index;
	unsigned short ver_blk_num, hor_384Bit_blk_num;
	unsigned char *pDMA_tmp;
	char tmpL, tmpM, tmpH;
	unsigned char *DMA_8bit;
	unsigned short div_tmp;
	unsigned int gray_setting[5];
	unsigned char err;
	char *INNX_Demura_DeLut1;
	char *INNX_Demura_DeLut2;
	char *INNX_Demura_DeLut3;
	//char *INNX_Demura_DeLut4;
	//char *INNX_Demura_DeLut5;

	if ((Demura_TBL == NULL) || (pDeLut == NULL)) {
		rtd_pr_vpq_emerg("vpq_INNX_Demura_data_to_dma table NULL, Demura_TBL=%p, pDeLut=%p ~~\n", Demura_TBL, pDeLut);
		return;
	}

	INNX_Demura_DeLut1 = &pDeLut[INNX_Demura_DecodeLUT_Low_sta_addr];
	INNX_Demura_DeLut2 = &pDeLut[INNX_Demura_DecodeLUT_Mid_sta_addr];
	INNX_Demura_DeLut3 = &pDeLut[INNX_Demura_DecodeLUT_Mid2_sta_addr];
	//INNX_Demura_DeLut4 = &pDeLut[INNX_Demura_DecodeLUT_Mid3_sta_addr];
	//INNX_Demura_DeLut5 = &pDeLut[INNX_Demura_DecodeLUT_High_sta_addr];

	DMA_8bit = &Demura_TBL->TBL[0];
	/* merlin7 add gain to replace scale */
	//Demura_TBL->DeMura_CTRL_TBL.demura_table_scale = 0;
	Demura_TBL->DeMura_CTRL_TBL.demura_gain_l_r = 16;
	Demura_TBL->DeMura_CTRL_TBL.demura_gain_m_r= 16;
	Demura_TBL->DeMura_CTRL_TBL.demura_gain_m2_r= 16;
	Demura_TBL->DeMura_CTRL_TBL.demura_gain_m3_r= 16;
	Demura_TBL->DeMura_CTRL_TBL.demura_gain_h_r= 16;
	Demura_TBL->DeMura_CTRL_TBL.demura_gain_l_g= 16;
	Demura_TBL->DeMura_CTRL_TBL.demura_gain_m_g= 16;
	Demura_TBL->DeMura_CTRL_TBL.demura_gain_m2_g= 16;
	Demura_TBL->DeMura_CTRL_TBL.demura_gain_m3_g= 16;
	Demura_TBL->DeMura_CTRL_TBL.demura_gain_h_g= 16;
	Demura_TBL->DeMura_CTRL_TBL.demura_gain_l_b= 16;
	Demura_TBL->DeMura_CTRL_TBL.demura_gain_m_b= 16;
	Demura_TBL->DeMura_CTRL_TBL.demura_gain_m2_b= 16;
	Demura_TBL->DeMura_CTRL_TBL.demura_gain_m3_b= 16;
	Demura_TBL->DeMura_CTRL_TBL.demura_gain_h_b= 16;

	hor_384Bit_blk_num = 69;	/* 481/7+1*/
	ver_blk_num = 271;

	for(i =0;i<ver_blk_num;i++) {
		/* one line*/
		index = 0;
		for (j=0;j<hor_384Bit_blk_num;j++) {
			for(k=0;k<(deMura_7blk_Block/9);k++) {

				tmpL = INNX_Demura_DeLut1[i * INNX_Demura_W16 + index];	/* low*/
				tmpM = INNX_Demura_DeLut2[i * INNX_Demura_W16 + index]; /*Middle*/
				tmpH = INNX_Demura_DeLut3[i * INNX_Demura_W16 + index];	/* High */

				/* R */
				deMura_7Blk_Decode_TBL[k*9+0] = (unsigned char)tmpL;	/* low*/
				deMura_7Blk_Decode_TBL[k*9+1] = (unsigned char)tmpM; /*Middle*/
				deMura_7Blk_Decode_TBL[k*9+2] = (unsigned char)tmpH;	/* High */

				/* G */
				deMura_7Blk_Decode_TBL[k*9+3] =(unsigned char) tmpL;	/* low*/
				deMura_7Blk_Decode_TBL[k*9+4] = (unsigned char)tmpM; /*Middle*/
				deMura_7Blk_Decode_TBL[k*9+5] = (unsigned char)tmpH;	/* High */

				/* B */
				deMura_7Blk_Decode_TBL[k*9+6] = (unsigned char)tmpL;	/* low*/
				deMura_7Blk_Decode_TBL[k*9+7] = (unsigned char)tmpM; /*Middle*/
				deMura_7Blk_Decode_TBL[k*9+8] = (unsigned char)tmpH;	/* High */
				//if(i < 20 || i > 250)
					//if(index < 10 || (index > 470))
						//rtd_pr_vpq_info("roger, JW[%d][%d] = [%d]\n", i, index, tmpL);

				index = (index+1 >= INNX_Demura_W)?(INNX_Demura_W-1):(index+1);
			}

			pDMA_tmp = DMA_8bit+(48*(i*hor_384Bit_blk_num+j));
			vpq_DeMura_Encode_128x3bit(deMura_7Blk_Decode_TBL, pDMA_tmp);
		}
	}

	// set control threshold table
	gray_setting[0] = INNX_Demura_gray_setting[0];
	gray_setting[1] = INNX_Demura_gray_setting[2];
	gray_setting[2] = INNX_Demura_gray_setting[3];
	gray_setting[3] = INNX_Demura_gray_setting[4];
	gray_setting[4] = INNX_Demura_gray_setting[1];

	//data check
	err = 0;
	for (i=1;i<5;i++) {
		if (gray_setting[i] <=  gray_setting[i-1]) {
			err =1;
		}
		if (err == 1) {
			rtd_pr_vpq_emerg("vpq_INNX_Demura threshold setting error,  lower_cutoff=%d, low_lv=%d, mid_lv=%d, high_lv=%d, high_cut=%d, use default\n",
				gray_setting[0], gray_setting[1], gray_setting[2], gray_setting[3], gray_setting[4]);
			return;
		}
	}
	// r, g, b low_cut threshold
	Demura_TBL->DeMura_CTRL_TBL.demura_r_lower_cutoff = gray_setting[0];
	Demura_TBL->DeMura_CTRL_TBL.demura_g_lower_cutoff = gray_setting[0];
	Demura_TBL->DeMura_CTRL_TBL.demura_b_lower_cutoff = gray_setting[0];
	// r, g, b low level threshold
	Demura_TBL->DeMura_CTRL_TBL.demura_r_l_level = gray_setting[1];
	Demura_TBL->DeMura_CTRL_TBL.demura_g_l_level = gray_setting[1];
	Demura_TBL->DeMura_CTRL_TBL.demura_b_l_level = gray_setting[1];
	// r, g, b mid level threshold
	Demura_TBL->DeMura_CTRL_TBL.demura_r_m_level = gray_setting[2];
	Demura_TBL->DeMura_CTRL_TBL.demura_g_m_level = gray_setting[2];
	Demura_TBL->DeMura_CTRL_TBL.demura_b_m_level = gray_setting[2];
	// r, g, b high level threshold
	Demura_TBL->DeMura_CTRL_TBL.demura_r_h_level = gray_setting[3];
	Demura_TBL->DeMura_CTRL_TBL.demura_g_h_level = gray_setting[3];
	Demura_TBL->DeMura_CTRL_TBL.demura_b_h_level = gray_setting[3];
	// r, g, b high_cut threshold
	Demura_TBL->DeMura_CTRL_TBL.demura_r_upper_cutoff = gray_setting[4];
	Demura_TBL->DeMura_CTRL_TBL.demura_g_upper_cutoff = gray_setting[4];
	Demura_TBL->DeMura_CTRL_TBL.demura_b_upper_cutoff = gray_setting[4];

	// r, g, b dvi_fac_L  threshold
	div_tmp = (65536 / (gray_setting[1] -gray_setting[0]))>>2;
	Demura_TBL->DeMura_CTRL_TBL.r_div_factor_l = div_tmp;
	Demura_TBL->DeMura_CTRL_TBL.g_div_factor_l = div_tmp;
	Demura_TBL->DeMura_CTRL_TBL.b_div_factor_l = div_tmp;
	// r, g, b dvi_fac_L_M  threshold
	div_tmp = (65536 / (gray_setting[2] -gray_setting[1]))>>2;
	Demura_TBL->DeMura_CTRL_TBL.r_div_factor_l_m = div_tmp;
	Demura_TBL->DeMura_CTRL_TBL.g_div_factor_l_m = div_tmp;
	Demura_TBL->DeMura_CTRL_TBL.b_div_factor_l_m = div_tmp;
	// r, g, b dvi_fac_M_H  threshold
	div_tmp = (65536 / (gray_setting[3] -gray_setting[2]))>>2;
	Demura_TBL->DeMura_CTRL_TBL.r_div_factor_m_h = div_tmp;
	Demura_TBL->DeMura_CTRL_TBL.g_div_factor_m_h = div_tmp;
	Demura_TBL->DeMura_CTRL_TBL.b_div_factor_m_h = div_tmp;
	// r, g, b dvi_fac_M_H  threshold
	div_tmp = (65536 / (gray_setting[4] -gray_setting[3]))>>2;
	Demura_TBL->DeMura_CTRL_TBL.r_div_factor_h = div_tmp;
	Demura_TBL->DeMura_CTRL_TBL.g_div_factor_h = div_tmp;
	Demura_TBL->DeMura_CTRL_TBL.b_div_factor_h = div_tmp;

}
/*========================= for INNX demura =================================*/
#endif

unsigned int out_GAMMA_R[512], out_GAMMA_G[512], out_GAMMA_B[512];
void vpq_set_out_gamma(void)
{
#if 0
	extern unsigned short LEDOutGamma[257];
	static unsigned char panelTypeDef = 0;
	unsigned char panel_type=0;
#if 0
	panel_type=((IoReg_Read32(MISC_GPIO_GP2DATI_VADDR)>>18)&0x1);//0xb801bd38[18]=1 OLED
#else
	panel_type =0; //pleae fixed me //((IoReg_Read32(MISC_GPIO_GP2DATI_VADDR)>>18)&0x1);//0xb801bd38[18]=1 OLED
#endif
	if (panel_type)
		panelTypeDef = 1;

	down(&VPQ_Semaphore);
	if (panel_type || panelTypeDef) {
		fwif_color_out_gamma_control_front();
		fwif_color_out_gamma_encode(out_GAMMA_R, out_GAMMA_G, out_GAMMA_B, LEDOutGamma, LEDOutGamma, LEDOutGamma);
		fwif_color_set_out_gamma_encode_tv006();
		fwif_color_out_gamma_control_back();
		fwif_color_out_gamma_control_enable(1);
	} else
		fwif_color_out_gamma_control_enable(0);

	up(&VPQ_Semaphore);
#endif	
}

void vpq_update_icm(unsigned char waiting)
{
	//down(&VPQ_Semaphore);
	fwif_color_set_ICM_table_driver_tv006(&icm_tab_elem_write, waiting);
	//up(&VPQ_Semaphore);
}
unsigned char update_ICM_Pillar_Offset = 0;

void vpq_ICM_handler(unsigned int cmd)
{
	//unsigned int time_0 = 0, time_1 = 0, time_2 = 0, time_3 = 0;
	unsigned char use_ISR = 1;
	//time_0 = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
	down(&VPQ_ICM_Semaphore);
	switch (cmd) {
		case VPQ_IOC_SET_FRESH_COLOR:
		case VPQ_IOC_SET_PREFERRED_COLOR:
		case VPQ_IOC_SET_COLOR_MANAGEMENT:
		{
			if (g_Color_Mode == COLOR_MODE_EXP)	/*expert mode*/
				fwif_color_calc_ICM_gain_table_driver_tv006(&g_cm_rgn_exp.stColorRegionType, &g_cm_ctrl_exp, &icm_tab_elem_of_vip_table, &icm_tab_elem_write);
			else	/*default advance mode*/
				fwif_color_calc_ICM_gain_table_driver_tv006(&g_cm_rgn_adv.stColorRegionType, &g_cm_ctrl_adv, &icm_tab_elem_of_vip_table, &icm_tab_elem_write);

			vpq_update_icm(TRUE);
			//time_1 = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);

			if (use_ISR == 1) {
				update_ICM_Pillar_Offset = 1;
				fwif_color_icm_pillar_sendRPCtoVideo(update_ICM_Pillar_Offset);
			} else {
				update_ICM_Pillar_Offset = 0;
				drvif_Wait_UZUDTG_END(0, 1);
				//time_2 = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);

				fwif_color_icm_set_pillar();
				fwif_color_set_ICM_RGB_offset_tv006();
				//time_3 = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			}
		}
		break;

		case VPQ_IOC_SET_INIT_COLOR_EXP:
		case VPQ_IOC_SET_INIT_COLOR_ADV:
		{
			//time_1 = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			fwif_color_set_ICM_table_driver_init_tv006();
			//time_2 = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			//time_3 = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
		}
		break;

		case VPQ_IOC_SET_ICM_ELEM:
		{
			vpq_update_icm(TRUE);
			//time_1 = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			//time_2 = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
			//time_3 = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
		}
		break;

		default:
			rtd_pr_vpq_emerg("vpq_ICM_handler command %d not found!\n", cmd);
	}
	up(&VPQ_ICM_Semaphore);

	//rtd_pr_vpq_info("vpq_ICM_handler t0~3=%d,%d,%d,%d,cmd=%x\n", time_0, time_1, time_2, time_3, cmd);

}

void vpq_cm_global_init(void)
{
	int rgn;
	memcpy(&g_cm_rgn_exp, &g_cm_rgn_adv, sizeof(CHIP_CM_REGION_T));

	for (rgn = 0; rgn < CHIP_CM_COLOR_REGION_MAX; rgn++) {
		g_cm_ctrl_adv.stColor[rgn].masterGain = 0;
		g_cm_ctrl_adv.stColor[rgn].RGBOffset_masterGain = 0;
		g_cm_ctrl_adv.stColor[rgn].stColorGain.stColorGainHue = 128;
		g_cm_ctrl_adv.stColor[rgn].stColorGain.stColorGainSat = 128;
		g_cm_ctrl_adv.stColor[rgn].stColorGain.stColorGainInt = 128;
	}

	memcpy(&g_cm_ctrl_exp, &g_cm_ctrl_adv, sizeof(CHIP_COLOR_CONTROL_T));
	g_cm_need_refresh = 1;
}

void vpq_set_3d_lut_index(unsigned char idx, unsigned char bForceWrite)
{
	static unsigned char last3dlut_idx = 0xff;

	if (idx > IDX_3DLUT_MAX)
		return;

	if (bForceWrite || (last3dlut_idx > 0 && (last3dlut_idx-1) != fwif_color_3d_lut_access_index(ACCESS_MODE_GET, 0)))
		bForceWrite = 1;

	down(&VPQ_Semaphore);
	if (last3dlut_idx != idx || bForceWrite) {
		fwif_color_set_3dLUT(idx);
		last3dlut_idx = idx;
	}
	if (idx > 0)
		fwif_color_3d_lut_access_index(ACCESS_MODE_SET, idx-1);
	up(&VPQ_Semaphore);
}

unsigned char vpq_set_Gamut_3D_Lut_LGDB(GAMUT_3D_LUT_17x17x17_T *pLut, unsigned int iEnable, unsigned char bOnlySetEnable)
{
	int idx;
	static unsigned int pLUT3D[LUT3D_TBL_ITEM];
	//extern unsigned char lastLUT_tbl_index;// RTK last index
	unsigned short *pArray;

	if (NULL == pLut)
		return FALSE;

	down(&VPQ_Semaphore);
	//lastLUT_tbl_index = 0xff; //clear rtk DB last index
	//fwif_color_3d_lut_access_index(ACCESS_MODE_SET, 0); //clear rtk DB index
	pArray = &pLut->pt[0].r_data;

	if (bOnlySetEnable && iEnable) {
		//fwif_color_WaitFor_DEN_STOP_UZUDTG();
		drvif_color_D_3dLUT_Enable(1);
	} else {
		for (idx = 0; idx < 4913; idx++) {
			pLUT3D[idx<<1] = (pLut->pt[idx].r_data<<16)+pLut->pt[idx].g_data;
			pLUT3D[(idx<<1)+1] = pLut->pt[idx].b_data;
		}

		//fwif_color_WaitFor_DEN_STOP_UZUDTG();

		if (iEnable) {
			if (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)	// DMA
				fwif_color_D_LUT_By_DMA_16(SLR_MAIN_DISPLAY, 1, pArray);
			else		// sram
				drvif_color_D_3dLUT(1, pLUT3D);
		} else {
			drvif_color_D_3dLUT(0, pLUT3D);
		}
	}
	up(&VPQ_Semaphore);

	return TRUE;
}
extern unsigned char bODInited;

int vpq_decide_OD_byPanelTconType(void)
{
	if (drvif_color_get_HW_MEX_STATUS(MEX_OD) == 0)
		return 0;

	switch (Get_DISPLAY_PANEL_TYPE())
	{
	//tconless
	case P_EPI_TYPE:
	case P_MINI_LVDS:
	case P_CEDS:
	case P_CHPI:
	case P_ISP:
		return 1;
	//otherwise
	default:
		return 0;
	}
}

//struct platform_device *vpq_rtpm_devs = 0;
static struct platform_device *vpq_platform_devs = NULL;	// move here for vpq run time pm
static unsigned char vpq_rtpm_tsk_block = 0;
static unsigned char vpq_rtpm_tsk_block_VPQ_done = 0;
static unsigned char vpq_rtpm_tsk_block_SE_done = 0;
static unsigned char vpq_rtpm_tsk_block_Film_done = 0;
static unsigned char vpq_rtpm_tsk_VPQ_freeze_stauts = 0;
static unsigned char vpq_rtpm_tsk_SE_freeze_stauts = 0;
static unsigned char vpq_rtpm_tsk_Film_freeze_stauts = 0;

#define VPQ_PM_REFCNT (atomic_read(&(&vpq_platform_devs->dev)->power.usage_count))
#define VPQ_PM_STATUS (vpq_platform_devs->dev.power.runtime_status)	 // enum rpm_status { RPM_ACTIVE = 0, RPM_RESUMING,	RPM_SUSPENDED, RPM_SUSPENDING,};
#define VPQ_PM_AUTO (vpq_platform_devs->dev.power.runtime_auto)	 // runtime_auto = 1 => auto.     runtime_auto = 0 => on 

#define VPQ_PM_LOG(PM_LV, TYPE, MESSAGE) \
{\
	if(vpq_platform_devs==NULL)\
		rtd_pr_vpq_emerg("LowPower::error::picturequality::video50::na::na-na::%s::fail::vpq dev=NULL\n", TYPE);\
	else if((&vpq_platform_devs->dev)==NULL)\
		rtd_pr_vpq_emerg("LowPower::error::picturequality::video50::na::na-na::%s::fail::vpq dev=NULL\n", TYPE);\
	else\
		rtd_pr_vpq_info("LowPower::%s::picturequality::video50::%d::%s-%s::%s::ok::%s::rpm_status=%d::runtime_auto=%d\n", \
			PM_LV, VPQ_PM_REFCNT, (current->group_leader?current->group_leader->comm:"-"), current->comm, TYPE, MESSAGE, VPQ_PM_STATUS, VPQ_PM_AUTO);\
}

#if IS_ENABLED(CONFIG_RTK_KDRV_THERMAL_SENSOR)
int Scaler_register_PQ_thermal_handler(void);
int Scaler_register_MEMC_thermal_handler(void);
#endif
extern unsigned char bPictureEnabled;
void vpq_boot_init(void)
{
	DRV_Sharpness_Level Sharpness_Level;

	if(is_QS_pq_enable()==1) { //current is QS flow pq_quick_show flow
#ifndef UT_flag
		fwif_color_load_QS_vip_memory_from_carveout();
#endif
		fwif_color_MEX_MODE_init();

		fwif_color_DI_IP_ini();
		fwif_color_DI_IEGSM_ini();
		vpq_cm_global_init();
		vpq_init_gamma();
		//vpq_set_out_gamma();//un-use
		//fwif_color_set_DM2_OE_EO_CSC3_Sram_init_Flag_TV006(1);
		drvif_DM2_B0203_noSram_Enable(1, 0);

		Scaler_VIP_Project_ID_init();
		memset(&gamutMatrix_APPLY, 0, sizeof(short)*3*3);
		
		/* set panel dither*/
		if (Get_DISPLAY_PANEL_MPLUS_RGBW() == TRUE || Get_DISPLAY_PANEL_BOW_RGBW() == TRUE)
			fwif_color_set_Panel_Dither(TRUE,VIP_PANEL_BIT8,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);
		else
			fwif_color_set_Panel_Dither(TRUE,VIP_PANEL_BIT10,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);
	
		/* ST2094 Ctrl ini*/
		fwif_color_ST2094_Ctrl_Structure_ini();

		// hdr vivi init
		fwif_color_HDR_VIVID_CTRL_Structure_ini();	

		memc_logo_to_demura_init();
		fwif_color_set_SLD_init();
	
		Scaler_color_Set_HDR_AutoRun(TRUE); // use HFC to set HDR inv gamma and EOTF for TV006

		// blk-decont init
		fwif_color_I_BLK_decontour_init();

		/* demura ini */
		fwif_color_DeMura_init(1, 0);

		/* register TC DMA*/
		fwif_color_Reg_TC_DMA_addr();

		/* YC init at boot */
		drvif_module_yc_boot_init();

		fwif_color_set_FILM_FW_ShareMemory();
		//fwif_color_set_Picture_Control_tv006(pic_ctrl.wId, &pic_ctrl);
		//fwif_color_GSR_init();
	/* It is for workaround End  */

		// i domain dither hist init 
		drvif_I_Main_DITHER_Y_HISTOGRAM_init();

		fwif_color_PQ_SR_MODE_init();
		drvif_color_disable_FCIC_CLK(); //for merlin7 power consumption

#ifdef CONFIG_SUPPORT_SCALER_MODULE
#if IS_ENABLED(CONFIG_RTK_KDRV_THERMAL_SENSOR)
		Scaler_register_PQ_thermal_handler();
		Scaler_register_MEMC_thermal_handler();
#endif
#endif		
		rtd_pr_vpq_info("boot ini from qs !!!\n");

		// SLD enable
		bPictureEnabled = 1;
		
		// for low power mode, run time pm
		vpq_rtpm_tsk_block = 0; 	// init status is on for socts HEVC test. so vsc need to set tsk block while vsc is suspend for VSC socts.
		vpq_rtpm_tsk_block_VPQ_done = 0;		// default tsk is on, so this flag need to be set by tsk
		vpq_rtpm_tsk_block_SE_done = 0; 		// default tsk is on, so this flag need to be set by tsk
		vpq_rtpm_tsk_block_Film_done = 0;		// default tsk is on, so this flag need to be set by tsk
		vpq_rtpm_tsk_VPQ_freeze_stauts = 0;
		vpq_rtpm_tsk_SE_freeze_stauts = 0;
		vpq_rtpm_tsk_Film_freeze_stauts = 0;
		rtd_pr_vpq_info("VPQ_RTPM, QS boot init\n");
	
	}else{
		fwif_color_MEX_MODE_init();
		fwif_color_DI_IP_ini();
		fwif_color_DI_IEGSM_ini();
		vpq_cm_global_init();
		vpq_init_gamma();
		/*vpq_set_inv_gamma(0);
		vpq_set_out_gamma();
		vpq_set_gamma(0, 0);
		fwif_color_inv_gamma_control_back(SLR_MAIN_DISPLAY, 0);
		fwif_color_gamma_control_back(SLR_MAIN_DISPLAY, 0);*/		
		//fwif_color_inv_gamma_control_back(SLR_MAIN_DISPLAY, 0);
		//fwif_color_gamma_control_back(SLR_MAIN_DISPLAY, 1);
		//fwif_color_gamma_control_back(SLR_SUB_DISPLAY, 1);
#ifdef CONFIG_DUAL_CHANNEL
		fwif_color_inv_gamma_control_back(SLR_SUB_DISPLAY, 0);
		fwif_color_gamma_control_back(SLR_SUB_DISPLAY, 1);
#endif
		fwif_color_set_InvGamma(SLR_MAIN_DISPLAY, 0, &Gamma_sRGB2Linear_14bit[0], &Gamma_sRGB2Linear_14bit[0], &Gamma_sRGB2Linear_14bit[0]);
		fwif_set_gamma_system_enhance(&Gamma_Linear2sRGB_14bit[0], &Gamma_Linear2sRGB_14bit[0], &Gamma_Linear2sRGB_14bit[0]);
		drvif_color_set_Vivid_Color_Enable(TRUE);
		drvif_color_set_sub_Vivid_Color_Enable(TRUE);
		Scaler_VIP_Project_ID_init();
		drvif_color_Set_sr_mdom_sub_en(0);
		//fwif_color_set_ICM_table_driver_init_tv006();
		//fwif_color_set_ICM_table_driver_init_elem_tv006(&icm_tab_elem_write);
		//memcpy(&icm_tab_elem_of_vip_table, &icm_tab_elem_write, sizeof(COLORELEM_TAB_T));
		//vpq_update_icm(FALSE, 1);	/* 20190708 roger, write fail when boot ==> mark */
		//vpq_ICM_handler(VPQ_IOC_SET_FRESH_COLOR);
		fwif_color_set_ICM_table_driver(0, 0, 0);
		fwif_color_SetDNR_tv006(2);	/*rock_rau 20150922 vpq_project_id 0x00060000 NR set use fwif_color_SetDNR_tv006 function*/
		memset(&Sharpness_Level, 0, sizeof(DRV_Sharpness_Level));
		memset(&gamutMatrix_APPLY, 0, sizeof(short)*3*3);
		drvif_color_set_Sharpness_level(&Sharpness_Level);
		//fwif_color_3d_lut_init_tv006();
		fwif_color_access_DeJaggy_Level(De_Jaggy_LEVEL_MAX, 1);
#ifdef CONFIG_HDR_SDR_SEAMLESS
		hdr_resume();
		/* set defautl table and ini clk.*/
		rtd_pr_vpq_info("boot ini!!!\n");
		//fwif_color_set_DM2_EOTF_TV006(EOTF_LUT_R);
		//fwif_color_set_DM2_OETF_TV006(OETF_LUT_R);
		/* set default 3D LUT for seamless*/
		//set default 3D LUT at drvif_TV006_HDR10_init when source change
#endif

		/* always on for snr hw issue, elieli*/
		if (drvif_color_Get_DRV_SNR_Clock(SLR_MAIN_DISPLAY, 0) == 1) {
			drvif_color_Set_DRV_SNR_Clock(SLR_MAIN_DISPLAY, 0);
			drvif_color_DRV_SNR_Mosquito_NR_En(0xF);
			drvif_color_iEdgeSmooth_en(0xF);
			drvif_color_Set_DRV_SNR_Clock(SLR_MAIN_DISPLAY, 1);
		} else {
			drvif_color_DRV_SNR_Mosquito_NR_En(0xF);
			drvif_color_iEdgeSmooth_en(0xF);
		}
		drvif_color_Set_DRV_SNR_Clock(SLR_SUB_DISPLAY, 0);	// disable SDNR2 for default
		/* always on for UZU_Bypass_No_PwrSave hw issue, elieli*/
		//drvif_color_Access_UZU_Bypass_No_PwrSave(1, 1);
		// set MEMC SLD ini
		memc_logo_to_demura_init();
		fwif_color_set_SLD_init();
		
		/* set panel dither*/
		if (Get_DISPLAY_PANEL_MPLUS_RGBW() == TRUE || Get_DISPLAY_PANEL_BOW_RGBW() == TRUE)
			fwif_color_set_Panel_Dither(TRUE,VIP_PANEL_BIT8,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);
		else
			fwif_color_set_Panel_Dither(TRUE,VIP_PANEL_BIT10,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);

#if (!defined(CONFIG_RTK_8KCODEC_INTERFACE)) // 8k no need init tcon
		if (vpq_decide_OD_byPanelTconType()) {
			if (Scaler_Init_OD()) { // init od dma and lut index
				fwif_color_set_od_default_table(); // set od and rec od LUT
				fwif_color_set_od(1);
			}
			else {
				fwif_color_set_od_liner_table(0); // od lut
				fwif_color_set_od_liner_table(1); // rec od lut
			}
		} else {
			bODInited = 0;	
		}
#endif
		fwif_color_set_od_2p();

		Scaler_color_Set_HDR_AutoRun(TRUE); // use HFC to set HDR inv gamma and EOTF for TV006

		// blk-decont init
		fwif_color_I_BLK_decontour_init();

		/* demura ini */
		fwif_color_DeMura_init(1, 0);

		/* register TC DMA*/
		fwif_color_Reg_TC_DMA_addr();

		/* ST2094 Ctrl ini*/
		fwif_color_ST2094_Ctrl_Structure_ini();

		// hdr vivi init
		fwif_color_HDR_VIVID_CTRL_Structure_ini();

#if 0 // def VPQ_COMPILER_ERROR_ENABLE_MERLIN6
		//if( webos_tooloption.eBackLight == 2 ) // OLED
		if (strcmp(webos_strToolOption.eBackLight, "oled") == 0)
		{
			fwif_color_set_LDSetLUT(0);
			memc_logo_to_demura_init();
		}
#endif

	//#ifndef CONFIG_MEMC_BYPASS
		//if(get_MEMC_bypass_status_refer_platform_model() == FALSE || get_platform() == PLATFORM_K8HP){
		if(get_MEMC_bypass_status_refer_platform_model() == FALSE){
			//Scaler_MEMC_Handler();
			if(MEMC_First_Run_Done == 0){
				rtd_pr_vpq_debug("[MEMC][init]%s %d[%d][%d]\n",__func__,__LINE__,MEMC_First_Run_Done,MEMC_First_Run_force_SetInOutUseCase_Done);
				//memc initial RPC
				MEMC_First_Run_Done = 1;
				MEMC_First_Run_FBG_enable = _ENABLE;
				/* Due to video path boot logo, memc force bg enable move to vpqex_ioctl VPQ_EXTERN_IOC_INIT */
		                //Scaler_MEMC_output_force_bg_enable(TRUE,__func__,__LINE__);
				Scaler_MEMC_initial_Case();
				if(MEMC_First_Run_force_SetInOutUseCase_Done == 0){//need to trigger the Scaler_MEMC_SetInOutUseCase again when the snapshop on!!
					MEMC_First_Run_force_SetInOutUseCase_Done=1;
				}
				rtd_pr_vpq_debug("[MEMC][init]MEMC_Initialize in vpq_boot_init Done!![%d][%d]\n",MEMC_First_Run_Done,MEMC_First_Run_force_SetInOutUseCase_Done);
			}
		}
	//#endif

		/* YC init at boot */
		drvif_module_yc_boot_init();


	/* It is for workaround start */
	// Due to  Webos5 still not call V4L2_CID_EXT_VPQ_INIT
	// VPQ init value set here firt ,KTASKWBS-10929

		fwif_color_set_DCC_Init(SLR_MAIN_DISPLAY);
		/*fwif_color_set_DCC_Init_tv006();*/
		//fwif_color_set_dcc_force_write_flag_RPC(1, 0);

		// enable DM & Composer clock
		drvif_Set_DM_HDR_CLK();
		//drvif_TV006_HDR10_init();
		fwif_color_set_FILM_FW_ShareMemory();
		//fwif_color_set_Picture_Control_tv006(pic_ctrl.wId, &pic_ctrl);
		//fwif_color_GSR_init();
	/* It is for workaround End  */

		// i domain dither hist init 
		drvif_I_Main_DITHER_Y_HISTOGRAM_init();

	        fwif_color_PQ_SR_MODE_init();
		drvif_color_disable_FCIC_CLK(); //for merlin7 power consumption

#if IS_ENABLED(CONFIG_RTK_AI_DRV)
		/* NN init*/
		scalerAI_Init();
#endif

#ifdef CONFIG_SUPPORT_SCALER_MODULE
#if IS_ENABLED(CONFIG_RTK_KDRV_THERMAL_SENSOR)
		Scaler_register_PQ_thermal_handler();
		Scaler_register_MEMC_thermal_handler();
#endif
#endif

		// for low power mode, run time pm
		vpq_rtpm_tsk_block = 0;		// init status is on for socts HEVC test. so vsc need to set tsk block while vsc is suspend for VSC socts.
		vpq_rtpm_tsk_block_VPQ_done = 0;		// default tsk is on, so this flag need to be set by tsk
		vpq_rtpm_tsk_block_SE_done = 0;			// default tsk is on, so this flag need to be set by tsk
		vpq_rtpm_tsk_block_Film_done = 0;		// default tsk is on, so this flag need to be set by tsk
		vpq_rtpm_tsk_VPQ_freeze_stauts = 0;
		vpq_rtpm_tsk_SE_freeze_stauts = 0;
		vpq_rtpm_tsk_Film_freeze_stauts = 0;
		rtd_pr_vpq_info("VPQ_RTPM, boot init\n");
	}
}

#endif
void vpq_boot_init_QS(void)
{
	fwif_color_DI_IP_ini();
#ifdef VIP_NEED_TO_APPLY_DITHER_FLOW
    #ifndef UT_flag    
    if((Get_PANEL_DITHER_ENABLE()!=0) || (Get_PANEL_PANEL_DITHER_TEMPORAL_ENABLE()!=0) || (Get_PANEL_PANEL_DITHER_BIT_TABLE_SELECT()!=0) || (Get_PANEL_PANEL_DITHER_OLED_TH_EN()!=0) || (Get_PANEL_PANEL_DITHER_OLED_TH()!=0)){
        fwif_color_set_Panel_Dither_By_ini();
    }else{
    	if (Get_DISPLAY_PANEL_MPLUS_RGBW() == TRUE || Get_DISPLAY_PANEL_BOW_RGBW() == TRUE)
    		fwif_color_set_Panel_Dither(TRUE,VIP_PANEL_BIT8,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);
    	else
    		fwif_color_set_Panel_Dither(TRUE,VIP_PANEL_BIT10,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);
    }
    #endif
#endif
}

#ifndef BUILD_QUICK_SHOW

void *vpq_demo_overscan_func(void *param)
{
	VIDEO_WID_T wid;
	KADP_VIDEO_RECT_T inregion;
	KADP_VIDEO_RECT_T outregion;

	wid = VIDEO_WID_0;
	if(Scaler_InputRegionType_Get(wid) == INPUT_BASE_ON_DISP_WIN){
		StructSrcRect newInput = cal_new_input_base_on_display_win(wid);
		inregion.x = newInput.srcx;
		inregion.y = newInput.srcy;
		inregion.h = newInput.src_height;
		inregion.w = newInput.src_wid;
	}else{
		rtk_hal_vsc_GetInputRegion(wid, &inregion);
	}

	if ( !rtk_hal_vsc_GetOutputRegion(wid, &outregion))
		return NULL;

	if (param != NULL) {
		unsigned short *ocInfo = (unsigned short *)param;
		switch(ocInfo[0]) {
		case 1:/*read*/
			ocInfo[1] = inregion.x;
			ocInfo[2] = inregion.y;
			ocInfo[3] = inregion.w;
			ocInfo[4] = inregion.h;

			ocInfo[5] = outregion.x;
			ocInfo[6] = outregion.y;
			ocInfo[7] = outregion.w;
			ocInfo[8] = outregion.h;
			return NULL;
			break;
		case 2:/*write*/
			if (ocInfo[3] && ocInfo[4]) {
				inregion.x = ocInfo[1];
				inregion.y = ocInfo[2];
				inregion.w = ocInfo[3];
				inregion.h = ocInfo[4];
			}

			if (ocInfo[7] && ocInfo[8]) {
				outregion.x = ocInfo[5];
				outregion.y = ocInfo[6];
				outregion.w = ocInfo[7];
				outregion.h = ocInfo[8];
			}
			break;
		default:
			return NULL;
		}
	}else {/*custom overscan by source*/
		_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

		switch (VIP_RPC_system_info_structure_table->VIP_source) {

			case VIP_QUALITY_HDMI_1080I:
			case VIP_QUALITY_HDMI_1080P:

				inregion.x = 39;
				inregion.y = 23;
				inregion.w = 1847;
				inregion.h = 1037;

				break;
			case VIP_QUALITY_CVBS_PAL:
			case VIP_QUALITY_CVBS_S_PAL:
			case VIP_QUALITY_TV_PAL:
				inregion.x = 18;
				inregion.y = 18;
				inregion.w = 668;
				inregion.h = 539;

				break;


		default:
			return NULL;
		}
	}

	rtk_hal_vsc_SetInputRegion(wid, inregion);
	rtk_hal_vsc_SetOutputRegion(wid, outregion, 0, 0);

	return NULL;
}

void *vpq_demo_pq_func(void *param)
{
	unsigned char enable;
	//printk(KERN_EMERG "[MEMC][%s]\n",__FUNCTION__);
	if (param == NULL)
		return NULL;

	enable = *((unsigned char*)param);

	if (enable==1 && vpq_project_id != 0x00010000) {
		vpq_project_id = 0x00010000;
		fwif_color_set_pq_demo_flag_rpc(1);
		Scaler_VIP_Project_ID_init();
		fwif_color_handler();
		fwif_color_video_quality_handler();
		fwif_color_vpq_pic_init();
		Scaler_SetDNR(4);
		Scaler_SetMPEGNR(4, CALLED_NOT_BY_OSD);
		fwif_color_set_PQA_motion_threshold_TV006();
		fwif_color_SetAutoMAFlag(1);
		vpq_demo_overscan_func(NULL);
		Demo_Flag = 1;
	} else if (enable==0 && vpq_project_id != 0x00060000) {
		vpq_project_id = 0x00060000;
		g_InvGammaPowerMode = 0;
		g_IsInvGammaPowerNewMode = 0;
		fwif_color_set_pq_demo_flag_rpc(0);
		Scaler_VIP_Project_ID_init();
		vpq_boot_init();
		fwif_color_inv_gamma_control_back(SLR_MAIN_DISPLAY, 0);
		drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 0, 0);
		drvif_color_inv_gamma_enable(SLR_SUB_DISPLAY, 0, 0);
		fwif_color_gamma_control_back(SLR_MAIN_DISPLAY, 1);
		fwif_color_SetDNR_tv006(0);		/*rock_rau 20150922 vpq_project_id 0x00060000 NR set use fwif_color_SetDNR_tv006 function*/
		fwif_color_set_DCC_Init(SLR_MAIN_DISPLAY);
		fwif_color_handler();
		fwif_color_video_quality_handler();
		fwif_color_vpq_pic_init();
		fwif_color_SetAutoMAFlag(1);
		Demo_Flag = 0;
	} else if (enable==2 && vpq_project_id != 0x00010000) {			//rock_rau 20150813 PQ by pass
		DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
		vpq_project_id = 0x00010000;
		fwif_color_set_pq_demo_flag_rpc(1);
		Scaler_VIP_Project_ID_init();
		fwif_color_handler();
		fwif_color_video_quality_handler();
		fwif_color_vpq_pic_init();
		fwif_color_SetAutoMAFlag(1);
/*==============================================*/
		Scaler_SetContrast(50);
		Scaler_SetBrightness(50);
		Scaler_SetSaturation(50);
		Scaler_SetHue(50);
		fwif_color_RHAL_SetDNR(0);

		Scaler_SetMPEGNR(0, CALLED_NOT_BY_OSD);
		Scaler_SetDNR(0);
		Scaler_SetSharpness(50);
		Scaler_SetDCC_Mode(0);

		Scaler_set_ICM_table(255, 0);
		Scaler_SetSharpnessTable(255);
		Scaler_SetMBPeaking(255);
		Scaler_SetMBSUPeaking(255);
		memset(&drv_vipCSMatrix_t, 0, sizeof(DRV_VIP_YUV2RGB_CSMatrix));
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_UVOffset_ctrl, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);


//		Color_Fun_Bypass color_fun_bypass[3];
//		color_fun_bypass[0].idIP = BYPASS_RGB_CON;
//		color_fun_bypass[0].bypass_switch = 0;
//		color_fun_bypass[1].idIP = BYPASS_RGB_BRI;
//		color_fun_bypass[1].bypass_switch = 0;
//		color_fun_bypass[2].idIP = BYPASS_GAMMA;
//		color_fun_bypass[2].bypass_switch = 0;
//		color_fun_bypass[3].idIP = BYPASS_D_DITHER;
//		color_fun_bypass[3].bypass_switch = 0;
//		color_fun_bypass[4].idIP = BYPASS_SRGB;
//		color_fun_bypass[4].bypass_switch = 0;
//		drvif_color_bypass(&color_fun_bypass, 3);

		vpq_demo_overscan_func(NULL);
		Demo_Flag = 1;
/*==============================================*/


	}

	return NULL;
}

#ifndef UT_flag

static sys_reg_sys_srst3_RBUS vpq_rtpm_sys_reg_sys_srst3_reg;
static sys_reg_sys_clken3_RBUS vpq_rtpm_sys_reg_sys_clken3_reg;
void vpq_low_power_mode_suspend(void)
{
	sys_reg_sys_srst3_RBUS sys_reg_sys_srst3_reg;
	sys_reg_sys_clken3_RBUS sys_reg_sys_clken3_reg;

	// restore
	vpq_rtpm_sys_reg_sys_srst3_reg.regValue = IoReg_Read32(SYS_REG_SYS_SRST3_reg);
	vpq_rtpm_sys_reg_sys_clken3_reg.regValue = IoReg_Read32(SYS_REG_SYS_CLKEN3_reg);

	sys_reg_sys_srst3_reg.regValue = 0;
	sys_reg_sys_clken3_reg.regValue = 0;

	sys_reg_sys_srst3_reg.rstn_disp_lg_a10 = 1;
	//sys_reg_sys_srst3_reg.rstn_disp_lg_mplus_nouse = 1;
	sys_reg_sys_srst3_reg.rstn_disp_lg_pod = 1;
	//sys_reg_sys_srst3_reg.rstn_disp_lg_hcic = 1;
	sys_reg_sys_srst3_reg.write_data = 0;

	sys_reg_sys_clken3_reg.clken_disp_lg_a10 = 1;
	//sys_reg_sys_clken3_reg.clken_disp_lg_mplus_nouse = 1;
	sys_reg_sys_clken3_reg.clken_disp_lg_pod = 1;
	//sys_reg_sys_clken3_reg.clken_disp_lg_hcic = 1;
	sys_reg_sys_clken3_reg.write_data = 0;

	//IoReg_Write32(SYS_REG_SYS_SRST3_reg, sys_reg_sys_srst3_reg.regValue);		// no rest to keep register setting
	IoReg_Write32(SYS_REG_SYS_CLKEN3_reg, sys_reg_sys_clken3_reg.regValue);
	rtd_pr_vpq_info("VPQ_RTPM, vpq_rumtime_suspend, restore, srst3=%x, clken3= %x,\n", 
		vpq_rtpm_sys_reg_sys_srst3_reg.regValue, vpq_rtpm_sys_reg_sys_clken3_reg.regValue);
}

void vpq_low_power_mode_resume(void)
{
	sys_reg_sys_srst3_RBUS sys_reg_sys_srst3_reg;
	sys_reg_sys_clken3_RBUS sys_reg_sys_clken3_reg;

	sys_reg_sys_srst3_reg.regValue = 0;
	sys_reg_sys_clken3_reg.regValue = 0;

	sys_reg_sys_srst3_reg.rstn_disp_lg_a10 = vpq_rtpm_sys_reg_sys_srst3_reg.rstn_disp_lg_a10;
	//sys_reg_sys_srst3_reg.rstn_disp_lg_mplus_nouse = 1;
	sys_reg_sys_srst3_reg.rstn_disp_lg_pod = vpq_rtpm_sys_reg_sys_srst3_reg.rstn_disp_lg_pod;
	//sys_reg_sys_srst3_reg.rstn_disp_lg_hcic = 1;
	sys_reg_sys_srst3_reg.write_data = 1;

	sys_reg_sys_clken3_reg.clken_disp_lg_a10 = vpq_rtpm_sys_reg_sys_clken3_reg.clken_disp_lg_a10;
	//sys_reg_sys_clken3_reg.clken_disp_lg_mplus_nouse = 1;
	sys_reg_sys_clken3_reg.clken_disp_lg_pod = vpq_rtpm_sys_reg_sys_clken3_reg.clken_disp_lg_pod;
	//sys_reg_sys_clken3_reg.clken_disp_lg_hcic = 1;
	sys_reg_sys_clken3_reg.write_data = 1;
	
	//IoReg_Write32(SYS_REG_SYS_SRST3_reg, sys_reg_sys_srst3_reg.regValue);	// no rest to keep register setting
	IoReg_Write32(SYS_REG_SYS_CLKEN3_reg, sys_reg_sys_clken3_reg.regValue);

	rtd_pr_vpq_info("VPQ_RTPM, vpq_rumtime_resume, restore, srst3=%x, clken3= %x,\n", 
		vpq_rtpm_sys_reg_sys_srst3_reg.regValue, vpq_rtpm_sys_reg_sys_clken3_reg.regValue);

}

static sys_reg_sys_srst3_RBUS vpq_vbe_rtpm_sys_reg_sys_srst3_reg;
static sys_reg_sys_clken3_RBUS vpq_vbe_rtpm_sys_reg_sys_clken3_reg;
void vpq_vbe_low_power_mode_suspend(void)
{
	sys_reg_sys_srst3_RBUS sys_reg_sys_srst3_reg;
	sys_reg_sys_clken3_RBUS sys_reg_sys_clken3_reg;

	// restore
	vpq_vbe_rtpm_sys_reg_sys_srst3_reg.regValue = IoReg_Read32(SYS_REG_SYS_SRST3_reg);
	vpq_vbe_rtpm_sys_reg_sys_clken3_reg.regValue = IoReg_Read32(SYS_REG_SYS_CLKEN3_reg);

	sys_reg_sys_srst3_reg.regValue = 0;
	sys_reg_sys_clken3_reg.regValue = 0;

	//sys_reg_sys_srst3_reg.rstn_disp_lg_a10 = 1;
	//sys_reg_sys_srst3_reg.rstn_disp_lg_mplus_nouse = 1;
	//sys_reg_sys_srst3_reg.rstn_disp_lg_pod = 1;
	sys_reg_sys_srst3_reg.rstn_disp_lg_hcic = 1;
	sys_reg_sys_srst3_reg.write_data = 0;

	//sys_reg_sys_clken3_reg.clken_disp_lg_a10 = 1;
	//sys_reg_sys_clken3_reg.clken_disp_lg_mplus_nouse = 1;
	//sys_reg_sys_clken3_reg.clken_disp_lg_pod = 1;
	sys_reg_sys_clken3_reg.clken_disp_lg_hcic = 1;
	sys_reg_sys_clken3_reg.write_data = 0;

	//IoReg_Write32(SYS_REG_SYS_SRST3_reg, sys_reg_sys_srst3_reg.regValue);		// no rest to keep register setting
	IoReg_Write32(SYS_REG_SYS_CLKEN3_reg, sys_reg_sys_clken3_reg.regValue);
	rtd_pr_vpq_info("VPQ_RTPM, vpq_vbe_rumtime_suspend, restore, srst3=%x, clken3= %x,\n", 
		vpq_vbe_rtpm_sys_reg_sys_srst3_reg.regValue, vpq_vbe_rtpm_sys_reg_sys_clken3_reg.regValue);
}

void vpq_vbe_low_power_mode_resume(void)
{
	sys_reg_sys_srst3_RBUS sys_reg_sys_srst3_reg;
	sys_reg_sys_clken3_RBUS sys_reg_sys_clken3_reg;

	sys_reg_sys_srst3_reg.regValue = 0;
	sys_reg_sys_clken3_reg.regValue = 0;

	//sys_reg_sys_srst3_reg.rstn_disp_lg_a10 = vpq_vbe_rtpm_sys_reg_sys_srst3_reg.rstn_disp_lg_a10;
	//sys_reg_sys_srst3_reg.rstn_disp_lg_mplus_nouse = 1;
	//sys_reg_sys_srst3_reg.rstn_disp_lg_pod = vpq_vbe_rtpm_sys_reg_sys_srst3_reg.rstn_disp_lg_pod;
	sys_reg_sys_srst3_reg.rstn_disp_lg_hcic = vpq_vbe_rtpm_sys_reg_sys_srst3_reg.rstn_disp_lg_hcic;
	sys_reg_sys_srst3_reg.write_data = 1;

	//sys_reg_sys_clken3_reg.clken_disp_lg_a10 = vpq_vbe_rtpm_sys_reg_sys_clken3_reg.clken_disp_lg_a10;
	//sys_reg_sys_clken3_reg.clken_disp_lg_mplus_nouse = 1;
	//sys_reg_sys_clken3_reg.clken_disp_lg_pod = vpq_vbe_rtpm_sys_reg_sys_clken3_reg.clken_disp_lg_pod;
	sys_reg_sys_clken3_reg.clken_disp_lg_hcic = vpq_vbe_rtpm_sys_reg_sys_clken3_reg.clken_disp_lg_hcic;
	sys_reg_sys_clken3_reg.write_data = 1;
	
	//IoReg_Write32(SYS_REG_SYS_SRST3_reg, sys_reg_sys_srst3_reg.regValue);		// no rest to keep register setting
	IoReg_Write32(SYS_REG_SYS_CLKEN3_reg, sys_reg_sys_clken3_reg.regValue);

	rtd_pr_vpq_info("VPQ_RTPM, vpq_vbe_rumtime_resume, restore, srst3=%x, clken3= %x,\n", 
		vpq_vbe_rtpm_sys_reg_sys_srst3_reg.regValue, vpq_vbe_rtpm_sys_reg_sys_clken3_reg.regValue);
	
}

#ifdef CONFIG_PM
void pcid_do_suspend(void)
{
	int i=0, j=0, k=0;

	// reset pcid regional table
	for(i=0; i<16; i++)
	{
		for(j=0; j<3; j++)
		{
			for(k=0; k<PCID_TBL_LEN; k++)
			{
				pcidRgnTbl[i][j][k] = 0;
			}
		}
	}
}

void valc_do_suspend(void)
{
	int i=0, j=0, k=0;

	// reset pcid regional table
	for(i=0; i<3; i++)
	{
		for(j=0; j<2; j++)
		{
			for(k=0; k<VALC_TBL_LEN; k++)
			{
				valcTbl[i][j][k] = 0;
			}
		}
	}
}

extern unsigned char SLD_SW_En;
extern unsigned char SLD_DMA_Ready;
extern unsigned char MEMC_Pixel_LOGO_For_SW_SLD;
void vpq_do_prepare_suspend(void)
{
	//pr_info("%s: SLD_HW disable SLD DMA in prepare_suspend\n", __func__);
	rtd_pr_vpq_info("SLD_HW", "%s finished\n", __func__);
	SLD_SW_En = 0;
	SLD_DMA_Ready = 0;
	//VIP_GSR2_init_done = 0;
	//SLD_DMA_APL_Ready_CNT = 0;	
	//SLD_DMA_presuspend_done = 1;
	drvif_SLD_Block_APL_DMA_OFF();
}

VPQ_SUSPEND_RESUME_T vpq_suspend_resume;
void vpq_do_suspend(void)
{
	*((unsigned char *)Scaler_GetShareMemVirAddr(SCALERIOC_SETAUTOMAFLAG)) = 0;

	vpq_suspend_resume.CRT_PLL_SSC0_S_R = IoReg_Read32(PLL27X_REG_PLL_SSC0_reg);
  	vpq_suspend_resume.CRT_PLL_SSC3_S_R = IoReg_Read32(PLL27X_REG_PLL_SSC3_reg);
	vpq_suspend_resume.CRT_PLL_SSC4_S_R = IoReg_Read32(PLL27X_REG_PLL_SSC4_reg);
	vpq_suspend_resume.CRT_SYS_DCLKSS_S_R = IoReg_Read32(PLL27X_REG_SYS_DCLKSS_reg);
	vpq_suspend_resume.CRT_SYS_PLL_DISP1_S_R = IoReg_Read32(PLL27X_REG_SYS_PLL_DISP1_reg);
	vpq_suspend_resume.CRT_SYS_PLL_DISP2_S_R = IoReg_Read32(PLL27X_REG_SYS_PLL_DISP2_reg);
	vpq_suspend_resume.CRT_SYS_PLL_DISP3_S_R = IoReg_Read32(PLL27X_REG_SYS_PLL_DISP3_reg);
	vpq_suspend_resume.CRT_SYS_DISPCLKSEL_S_R = IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg);

	pcid_do_suspend();
	g_bLDinited = 0;
}

void vpq_do_resume_instanboot(void)
{
	IoReg_Write32(PLL27X_REG_PLL_SSC0_reg, vpq_suspend_resume.CRT_PLL_SSC0_S_R&~_BIT0);
	IoReg_Write32(PLL27X_REG_PLL_SSC3_reg, vpq_suspend_resume.CRT_PLL_SSC3_S_R);
	IoReg_Write32(PLL27X_REG_PLL_SSC4_reg, vpq_suspend_resume.CRT_PLL_SSC4_S_R);
	IoReg_Write32(PLL27X_REG_SYS_DCLKSS_reg, vpq_suspend_resume.CRT_SYS_DCLKSS_S_R);
	IoReg_Write32(PLL27X_REG_SYS_PLL_DISP1_reg, vpq_suspend_resume.CRT_SYS_PLL_DISP1_S_R);
	IoReg_Write32(PLL27X_REG_SYS_PLL_DISP2_reg, vpq_suspend_resume.CRT_SYS_PLL_DISP2_S_R);
	IoReg_Write32(PLL27X_REG_SYS_PLL_DISP3_reg, vpq_suspend_resume.CRT_SYS_PLL_DISP3_S_R);
	IoReg_Write32(SYS_REG_SYS_DISPCLKSEL_reg, vpq_suspend_resume.CRT_SYS_DISPCLKSEL_S_R);
 	IoReg_Write32(PLL27X_REG_PLL_SSC0_reg, (vpq_suspend_resume.CRT_PLL_SSC0_S_R|_BIT0));
	mdelay(1);
	IoReg_Write32(PLL27X_REG_PLL_SSC0_reg, (vpq_suspend_resume.CRT_PLL_SSC0_S_R&~_BIT0));

	vpq_do_resume();
}

void od_do_resume(void)
{
	extern unsigned int vpqex_project_id;
	extern unsigned char bODInited;
	extern unsigned char bODTableLoaded;
	extern char od_table_mode_store;// 0:target mode 1:delta mode

	fwif_color_set_od_2p();

	if (!bODInited)
		return;

	bODInited = 0;

	if (Scaler_Init_OD()) {
		if (bODTableLoaded && od_table_mode_store == 1)
			fwif_color_od_table_restore();
		else
			fwif_color_set_od_default_table();

		if (vpqex_project_id != 0x00060000)
			Scaler_Set_OD_Bit_By_XML();
		rtd_printk(KERN_EMERG,"VPQ_OD", "%s finished\n", __func__);
	}
	else {
		fwif_color_set_od_liner_table(0);
		fwif_color_set_od_liner_table(1);
	}
}

void vpq_do_resume(void)
{
	extern unsigned char lastLUT_tbl_index;
	extern unsigned char g_bNotFirstRun_LD_Data_Compensation_NewMode_2DTable;
	extern unsigned char output_gamma_DataStored;
	DRV_Sharpness_Level Sharpness_Level;
  	//_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	
	fwif_color_MEX_MODE_init();

	VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 0;

	g_cm_need_refresh = 1;
	g_srgbForceUpdate = 1;
	g_InvGammaPowerMode = 0;
	g_IsInvGammaPowerNewMode = 0;
	g_HDR3DLUTForceWrite = 1;
	lastLUT_tbl_index = 0xff;
	g_3DLUT_Resume = 1;
	g_prevent_force_write_dcc = 0;
	g_bNotFirstRun_LD_Data_Compensation_NewMode_2DTable = 0;
	output_gamma_DataStored = 0;
	str_resume_do_picturemode=1;

	fwif_color_DI_IP_ini();
	fwif_color_DI_IEGSM_ini();
	Scaler_color_set_m_nowSource(255);
	//vpq_set_out_gamma();
	vpq_init_gamma();
	//fwif_color_inv_gamma_control_back(SLR_MAIN_DISPLAY, 0);
	//vpq_set_gamma(1, 0);
	//fwif_color_gamma_control_back(SLR_SUB_DISPLAY, 1);
	fwif_color_set_InvGamma(SLR_MAIN_DISPLAY, 0, &Gamma_sRGB2Linear_14bit[0], &Gamma_sRGB2Linear_14bit[0], &Gamma_sRGB2Linear_14bit[0]);
	fwif_set_gamma_system_enhance(&Gamma_Linear2sRGB_14bit[0], &Gamma_Linear2sRGB_14bit[0], &Gamma_Linear2sRGB_14bit[0]);

	if (Scaler_access_Project_TV002_Style(0, 0) == Project_TV002_Style_1)
		Scaler_set_Init_TV002(Project_TV002_Style_1);
	/*else {
		vpq_set_inv_gamma(0);
		drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 0, 0);
		drvif_color_inv_gamma_enable(SLR_SUB_DISPLAY, 0, 0);
	}*/
	drvif_color_set_Vivid_Color_Enable(TRUE);
	drvif_color_set_sub_Vivid_Color_Enable(TRUE);
	drvif_color_Set_sr_mdom_sub_en(0);
	//fwif_color_set_ICM_table_driver_init_tv006();
	//vpq_update_icm(FALSE, 1);
	//vpq_ICM_handler(VPQ_IOC_SET_FRESH_COLOR);
	fwif_color_set_ICM_table_driver(0, 0, 0);
	memset(&Sharpness_Level, 0, sizeof(DRV_Sharpness_Level));
	memset(&gamutMatrix_APPLY, 0, sizeof(short)*3*3);
	drvif_color_set_Sharpness_level(&Sharpness_Level);
	fwif_color_set_DCC_Init(SLR_MAIN_DISPLAY);
	//vpq_set_3d_lut_index(0, 0);
	od_do_resume();
	drvif_Set_DM_HDR_CLK();	// enable DM & Composer clock
	hdr_resume();
	fwif_color_access_DeJaggy_Level(De_Jaggy_LEVEL_MAX, 1);
	/* always on for snr hw issue, elieli*/
	if (drvif_color_Get_DRV_SNR_Clock(SLR_MAIN_DISPLAY, 0) == 1) {
		drvif_color_Set_DRV_SNR_Clock(SLR_MAIN_DISPLAY, 0);
		drvif_color_DRV_SNR_Mosquito_NR_En(0xFF);
		drvif_color_iEdgeSmooth_en(0xFF);
		drvif_color_Set_DRV_SNR_Clock(SLR_MAIN_DISPLAY, 1);
	} else {
		drvif_color_DRV_SNR_Mosquito_NR_En(0xFF);
		drvif_color_iEdgeSmooth_en(0xFF);
	}
	drvif_color_Set_DRV_SNR_Clock(SLR_SUB_DISPLAY, 0);	// disable SDNR2 for default
	/* always on for UZU_Bypass_No_PwrSave hw issue, elieli*/
	//drvif_color_Access_UZU_Bypass_No_PwrSave(1, 1);

	/* set panel dither*/
	if (Get_DISPLAY_PANEL_MPLUS_RGBW() == TRUE || Get_DISPLAY_PANEL_BOW_RGBW() == TRUE)
		fwif_color_set_Panel_Dither(TRUE,VIP_PANEL_BIT8,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);
	else
		fwif_color_set_Panel_Dither(TRUE,VIP_PANEL_BIT10,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);

	Scaler_color_Set_HDR_AutoRun(TRUE); // use HFC to set HDR inv gamma and EOTF for TV006

	memc_logo_to_demura_init();
	fwif_color_set_SLD_init();

	// blk-decont init
	fwif_color_I_BLK_decontour_init();

	/* demura ini */
	rtd_pr_vpq_info("vpq_do_resume fwif_color_DeMura_init start\n");
	fwif_color_DeMura_init(1, 0);

	/* register TC DMA*/
	fwif_color_Reg_TC_DMA_addr();

	// hdr vivi init
	fwif_color_HDR_VIVID_CTRL_Structure_ini();	

	/* ST2094 Ctrl ini*/
	fwif_color_ST2094_Ctrl_Structure_ini();

	//if( webos_tooloption.eBackLight == 2 ) // OLED
#ifdef VPQ_COMPILER_ERROR_ENABLE_MERLIN6
	if (strcmp(webos_strToolOption.eBackLight, "oled") == 0)
		fwif_color_set_LDSetLUT(0);
#endif

#ifdef CONFIG_CUSTOMER_TV002
	Scaler_fwif_color_set_LocalDimming_table(0, 1);
	g_bLDinited = 1;
#endif

#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	//re-set aipq_mode when resume
	scalerAI_pq_mode_ctrl(aipq_mode,aipq_DynamicContrastLevel);
#endif

	fwif_color_PQ_SR_MODE_init();

#ifdef ENABLE_VIP_TABLE_CHECKSUM
/* === checksum ========== */
	extern VIP_table_crc_value vip_table_crc;
	VIP_table_crc_value vip_table_crc_now;
	fwif_color_check_VIPTable_crc(&vip_table_crc_now, fwif_color_GetShare_Memory_VIP_TABLE_Struct());
	rtd_pr_vpq_debug("[%s %d] VIP_table_checksum: %x, resume_checksum %x\n", __FILE__, __LINE__,
	vip_table_crc.checkSum, vip_table_crc_now.checkSum);

	if(vip_table_crc.checkSum != vip_table_crc_now.checkSum)
		rtd_pr_vpq_debug("[%s %d] Error! Checksum changed.\n", __FILE__, __LINE__);
/* ===================== */
#endif
	//return 0;
}

static int vpq_suspend(struct device *p_dev)
{
#ifdef VPQ_RunTime_PM_Enable
	//vpq_rtpm_tsk_block = 1;	// task will got to sleep, not block mode, remove this code
	//vpq_rtpm_tsk_block_VPQ_done = 1;	// tsk will set as 1 and then go to seelp, remove this code
	//vpq_rtpm_tsk_block_SE_done = 1;	// tsk will set as 1 and then go to seelp, remove this code
	//vpq_rtpm_tsk_block_Film_done = 1;	// tsk will set as 1 and then go to seelp, remove this code
	pm_runtime_disable(&vpq_platform_devs->dev);
	//rtd_pr_vpq_info("VPQ_RTPM, vpq_suspend\n");
	VPQ_PM_LOG("info", "stop", "VPQ_RTPM STR suspend");
#endif
	return 0;
}

static int vpq_resume(struct device *p_dev)
{
#ifdef VPQ_RunTime_PM_Enable
	//vpq_rtpm_tsk_block = 1;		// return from freeze, so do nothing, remoe this code
	//vpq_rtpm_tsk_block_VPQ_done = 1;		// return from freeze, so do nothing, remoe this code
	//vpq_rtpm_tsk_block_SE_done = 1;		// return from freeze, so do nothing, remoe this code
	//vpq_rtpm_tsk_block_Film_done = 1;		// return from freeze, so do nothing, remoe this code
	pm_runtime_enable(&vpq_platform_devs->dev);
	//rtd_pr_vpq_info("VPQ_RTPM, vpq_resume\n");
	VPQ_PM_LOG("info", "srtart", "VPQ_RTPM STR resume");
#endif

	//idomain mpeg double buffer enable https://jira.realtek.com/browse/RL6557-2310
	IoReg_SetBits(MPEGNR_ICH1_MPEGNR_DB_CTRL_reg, MPEGNR_ICH1_MPEGNR_DB_CTRL_mpegnr_db_en_mask);
	//idomain RGB2YUV double buffer enable https://jira.realtek.com/browse/RL6557-2311
	IoReg_SetBits(RGB2YUV_ICH1_RGB2YUV_CTRL_reg, RGB2YUV_ICH1_RGB2YUV_CTRL_rgb2yuv_db_en_mask);
	IoReg_SetBits(RGB2YUV_ICH1_422to444_CTRL_DB_reg, RGB2YUV_ICH1_422to444_CTRL_DB_db_en_mask);
	IoReg_SetBits(RGB2YUV_ICH2_422to444_CTRL_reg, RGB2YUV_ICH2_422to444_CTRL_db_en_mask);
	//idomain SPNR double buffer enable https://jira.realtek.com/browse/RL6557-2312
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_en_mask);
	return 0;
}

void vpq_low_power_mode_suspend_DI_RTNR(void)
{
	vgip_data_path_select_RBUS data_path_select_reg;
	data_path_select_reg.regValue = IoReg_Read32(VGIP_Data_Path_Select_reg);
	data_path_select_reg.uzd1_in_sel = 0;
	data_path_select_reg.xcdi_in_sel = 0;
	data_path_select_reg.di_i_clk_en = 0;
	data_path_select_reg.xcdi_clk_en = 0;//Enable clock. Will Add 20151218
	IoReg_Write32(VGIP_Data_Path_Select_reg, data_path_select_reg.regValue);

}

extern void vsc_runtime_pm_get(void); //wait_scaler_patch
extern void vsc_runtime_pm_put(void); //wait_scaler_patch

int vpq_rumtime_suspend(struct device *dev)
{
#ifdef VPQ_RunTime_PM_Enable
	unsigned int cur_time1 = 0;
	unsigned int cost_vpq_time = 0;

	rtd_pr_vpq_info("VPQ_RTPM, vpq_rumtime_suspend sta\n");
	vpq_rtpm_tsk_block = 1;

	cur_time1 = drvif_color_get_cur_counter();
	while(!vpq_get_VPQ_TSK_Stop())
	{//wait vpq freeze
		udelay(50);
		if(drvif_color_wait_timeout_check_by_counter(cur_time1, 1000))
		{
		    rtd_pr_vpq_err("### VPQ_RTPM [err] wait VPQ related task pending fail ###\r\n");
		    return -1;
		}
	}

	cost_vpq_time = drvif_color_report_cost_time_by_counter(cur_time1);//calculate vpq time
	rtd_pr_vpq_info("VPQ_RTPM, TSK_Stop ready , vpq cost time =%d \n",cost_vpq_time);

	SLD_SW_En = 0;
	SLD_DMA_Ready = 0;	
	//SLD_DMA_APL_Ready_CNT = 0;
	vpq_low_power_mode_suspend_DI_RTNR();
	drvif_SLD_Block_APL_DMA_OFF();

	vpq_low_power_mode_suspend();
	scalerAI_enableLowPowerMode(1);
	//rtd_pr_vpq_info("VPQ_RTPM, vpq_rumtime_suspend end\n");
#ifndef UT_flag
	vsc_runtime_pm_put(); //wait_scaler_patch
#endif
	VPQ_PM_LOG("info", "suspend", "VPQ_RTPM runtime PM suspen end");
#endif
	return 0;
}

int vpq_rumtime_resume(struct device *dev)
{
#ifdef VPQ_RunTime_PM_Enable

	vsc_runtime_pm_get();//wait_scaler_patch
	
	// for RPM power 
	vpq_low_power_mode_resume();
	vpq_do_resume();

	vpq_rtpm_tsk_block = 0;
	scalerAI_enableLowPowerMode(0);

	//rtd_pr_vpq_info("VPQ_RTPM, vpq_rumtime_resume end\n");
	VPQ_PM_LOG("info", "resume", "VPQ_RTPM runtime PM resume end");
#endif
	return 0;
}

#ifdef CONFIG_HIBERNATION
typedef struct {
	unsigned int id;
	unsigned int size;
	void *vaddr;
} STD_SHAREMEM_SAVE;

STD_SHAREMEM_SAVE vpq_std_save[] = {
	{SCALERIOC_VIP_system_info_structure, sizeof(_system_setting_info), NULL},
	{SCALERIOC_VIP_RPC_system_info_structure, sizeof(_RPC_system_setting_info), NULL},
	{SCALERIOC_VIP_RPC_SMARTPIC_CLUS, sizeof(_RPC_clues), NULL},
	{SCALERIOC_VIP_TABLE_STRUCT, sizeof(SLR_VIP_TABLE), NULL},
	{SCALERIOC_VIP_RPC_TABLE_STRUCT, sizeof(RPC_SLR_VIP_TABLE), NULL},
	{SCALERIOC_VIP_TABLE_CUSTOM_STRUCT, sizeof(SLR_VIP_TABLE_CUSTOM_TV001), NULL},
	{SCALERIOC_VIP_TABLE_CRC_STRUCT, sizeof(VIP_table_crc_value), NULL},
};

static int vpq_suspend_std(struct device *p_dev)
{
	int i;
#ifdef CONFIG_ARM64 //ARM32 compatible
	unsigned long smAddr = 0;
#else
	unsigned int smAddr = 0;
#endif
	for (i = 0; i < sizeof(vpq_std_save)/sizeof(STD_SHAREMEM_SAVE); i++) {
		vpq_std_save[i].vaddr = vmalloc(vpq_std_save[i].size);
		if (vpq_std_save[i].vaddr) {
			smAddr = Scaler_GetShareMemVirAddr(vpq_std_save[i].id);
			if (smAddr) {
#ifdef CONFIG_ARM64
				memcpy_fromio(vpq_std_save[i].vaddr, (void*)smAddr, vpq_std_save[i].size);
#else
				memcpy(vpq_std_save[i].vaddr, (void*)smAddr, vpq_std_save[i].size);
#endif
			}
		}
	}

	return 0;
}

static int vpq_resume_std(struct device *p_dev)
{
	int i;
#ifdef CONFIG_ARM64 //ARM32 compatible
	unsigned long smAddr = 0;
#else
	unsigned int smAddr = 0;
#endif
	for (i = 0; i < sizeof(vpq_std_save)/sizeof(STD_SHAREMEM_SAVE); i++) {
		if (vpq_std_save[i].vaddr) {
			smAddr = Scaler_GetShareMemVirAddr(vpq_std_save[i].id);
			if (smAddr) {
#ifdef CONFIG_ARM64
				memcpy_toio((void*)smAddr, vpq_std_save[i].vaddr, vpq_std_save[i].size);
#else
				memcpy((void*)smAddr, vpq_std_save[i].vaddr, vpq_std_save[i].size);
#endif
			}
			vfree(vpq_std_save[i].vaddr);
			vpq_std_save[i].vaddr = NULL;
		}
	}
	return 0;
}
#endif

#endif

_SE_PARAM_FilmDetectCtrl pParam; // = NULL;
_SE_OUTPUT_FilmDetectCtrl pOutput; // = NULL;
_SE_ReadBack_Regs pRBRegs; // = NULL;
extern UINT8 se_filmCad_enterCnt[_FRC_CADENCE_NUM_][_FILM_MOT_NUM];
//SE_FMT_ST pInfo;
void SE_FilmDtect_Init(void)
{
#ifdef CONFIG_MEMC_NOTSUPPORT // for code size reduce, when MEMC_BYPASS
		return;
#else

#if 1
	int u8_k, u8_i;
	pr_notice("[jerry_SE_FilmDetect][SE_FilmDtect_Init]\n");

	//pParam
	pParam.u1_filmDet_bypass = 0; // FRC_TOP__PQL_1__pql_film_det_bypass_ADDR[0]
	pParam.u32_filmDet_cadence_en = 0x3fff; // FRC_TOP__PQL_1__pql_film_det_bypass_ADDR[15:1]

	pParam.u4_Mot_rgn_diffWgt = 4; // FRC_TOP__PQL_1__pql_film_mot_rgn_diff_wgt_ADDR[3:0]
	pParam.u4_Mot_all_diffWgt = 4; // FRC_TOP__PQL_1__pql_film_mot_rgn_diff_wgt_ADDR[7:4]
	pParam.u27_Mot_all_min = 0xc8; // FRC_TOP__PQL_1__pql_film_mot_rgn_diff_wgt_ADDR[31:8]

	pParam.u4_enter_22_cntGain = 3; // FRC_TOP__PQL_1__pql_film_enter_22_cnt_gain_ADDR[3:0]
	pParam.u4_enter_32_cntGain = 3; // FRC_TOP__PQL_1__pql_film_enter_22_cnt_gain_ADDR[7:4]
	pParam.u4_enter_else_cntGain = 2; // FRC_TOP__PQL_1__pql_film_enter_22_cnt_gain_ADDR[11:8]

	pParam.u1_quit_motWgt_auto_en = 1; // FRC_TOP__PQL_1__pql_film_enter_22_cnt_gain_ADDR[23:20]
	pParam.u4_quit_bigMot_wgt = 2; // FRC_TOP__PQL_1__pql_film_enter_22_cnt_gain_ADDR[15:12]
	pParam.u4_quit_smlMot_wgt = 5; // FRC_TOP__PQL_1__pql_film_enter_22_cnt_gain_ADDR[19:16]
	pParam.u8_quit_cnt_th = 0x4; //FRC_TOP__PQL_1__pql_film_mix_mode_enter_cnt_th_ADDR[23:16]
	pParam.u8_quit_prd_th = 0x8; //FRC_TOP__PQL_1__pql_film_mix_mode_enter_cnt_th_ADDR[31:24]

	pParam.u1_mixMode_en = 1;
	pParam.u1_mixMode_det_en = 1; // FRC_TOP__PQL_1__pql_film_enter_22_cnt_gain_ADDR[26]

	for (u8_k = 0; u8_k < 18; u8_k ++){
		pParam.u1_mixMode_rgnEn[u8_k] = 1; // FRC_TOP__PQL_1__pql_film_enter_22_cnt_gain_ADDR[31:27]
	}
	pParam.u8_mixMode_enter_cntTh = 0xe; //FRC_TOP__PQL_1__pql_film_mix_mode_enter_cnt_th_ADDR[7:0]
	pParam.u8_mixMode_cnt_max = 0x14; //FRC_TOP__PQL_1__pql_film_mix_mode_enter_cnt_th_ADDR[15:8]
	pParam.u27_mixMode_rgnMove_minMot = 0x220; // FRC_TOP__PQL_1__pql_film_mix_mode_rgn_move_min_mot_ADDR[27:0]
	pParam.u4_mixMode_rgnMove_motWgt = 0xa; // FRC_TOP__PQL_1__pql_film_mix_mode_rgn_move_min_mot_ADDR[31:28]

	pParam.u1_FrcCadSupport_En = 0; // FRC_TOP__PQL_1__pql_film_det_bypass_ADDR[16]
	pParam.u3_DbgPrt_Rgn = 0;	// FRC_TOP__PQL_1__pql_film_det_bypass_ADDR[19:17]
	pParam.u1_BadEditSlowOut_En = 0; // FRC_TOP__PQL_1__pql_film_det_bypass_ADDR[28]
	pParam.u1_QuickSwitch_En = 0; // FRC_TOP__PQL_1__pql_film_det_bypass_ADDR[29]
	pParam.u1_StillDetect_En = 0; // FRC_TOP__PQL_1__pql_film_det_bypass_ADDR[30]
	pParam.u1_DbgPrt_FRChg_En = 0; // FRC_TOP__PQL_1__pql_film_det_bypass_ADDR[31]

	pParam.u5_dbg_param1 = 0;
	pParam.u32_dbg_param2 = 0;
	pParam.u32_dbg_cnt = 0;

	//pOutput
	for (u8_k = 0; u8_k < 18; u8_k ++){
		for (u8_i = 0; u8_i < 15; u8_i ++){
			se_filmCad_enterCnt[u8_i][u8_k] = 0;
		}

		for (u8_i = 0; u8_i < 8; u8_i ++){
			pOutput.u27_ipme_motionPool[u8_k][u8_i] = 0;
		}

		pOutput.u32_Mot_sequence[u8_k]  = 0;

		pOutput.u8_det_cadence_Id[u8_k] = 0; //_CAD_VIDEO;
		pOutput.u8_cur_cadence_Id[u8_k] = 0; //_CAD_VIDEO;
		pOutput.u8_pre_cadence_Id[u8_k] = 0; //_CAD_VIDEO;
		pOutput.u8_phase_Idx[u8_k]      = 0;

		pOutput.u27_quit_bigMot[u8_k] = 0x1FF;
		pOutput.u27_quit_smlMot[u8_k] = 0;
		pOutput.u27_quit_motTh[u8_k]  = 0;
		pOutput.u8_quit_cnt[u8_k]     = 0;
		pOutput.u8_quit_prd[u8_k]     = 0;

		pOutput.u8_phT_phase_Idx[u8_k] = 0;

		pOutput.u27_FrmMotion_S[u8_k] = 0;

		pOutput.u8_cur_cadence_out[u8_k] = 0; //_CAD_VIDEO;
		pOutput.u8_phT_phase_Idx_out[u8_k] = 0;

		pOutput.u27_g_FrmMot_Th[u8_k] = 0;

		pOutput.u1_cad_resync_true[u8_k] = 0;
	}

	for(u8_k = 0; u8_k < 26; u8_k ++){
		pOutput.u27_FrmMotionSort[u8_k] = 0;
		pOutput.u27_FrmMotionSortTick[u8_k] = 0;
	}

	pOutput.u5_dbg_param1_pre     = 0;
	pOutput.u32_dbg_param2_pre    = 0;
	pOutput.u32_dbg_cnt_hold      = 0;
	pOutput.u8_dbg_motion_t = 0; //_FILM_ALL;

	pOutput.u8_flbk_lvl_cad_cnt = 0;

	pOutput.u1_mixMode = 0;
	pOutput.u8_mixMode_cnt = 0;

	pOutput.u1_Mot_selBig = 0;
	pOutput.u1_cad_fastOut_true = 0;
	pOutput.u8_BadEdit_flag = 0;
	pOutput.u8_reset_flag = 0;
	pOutput.u8_flush_flag = 0;
	pOutput.u32_input_frame_rate = 0;
	pOutput.u8_pre_cadence_out = 0;

	for(u8_k = 0; u8_k < KEEP_FRAME_NUM; u8_k ++){
		pOutput.u8_cadence_sequence[u8_k] = 0;
	}

	//pRBRegs
	pRBRegs.u3_ipme_filmMode_rb = 0;
	pRBRegs.u27_ipme_AllMot_rb = 0;
	pRBRegs.u27_ipme_CenterMot_rb = 0;
	for(u8_k = 0; u8_k < 5; u8_k ++){
		pRBRegs.u27_ipme_5Mot_rb[u8_k] = 0;
	}
	for(u8_k = 0; u8_k < 12; u8_k ++){
		pRBRegs.u27_ipme_12Mot_rb[u8_k] = 0;
	}
	pRBRegs.u8_sys_N_rb = 1; // 30 to 60
	pRBRegs.u8_sys_M_rb = 2;
	pRBRegs.u1_SW_sc_true = 0;


//	pInfo.s1_CPUAddr = (uint32_t)dvr_malloc_specific(960*540,GFP_DCU2); //RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.SeqBufAddr;
//	memset((void *)pInfo.s1_CPUAddr,0xaa,960*540);
//	pInfo.s1_Addr = 0x42c00000;//(uint32_t)dvr_to_phys((unsigned int *)pInfo.s1_CPUAddr);

//	pInfo.s2_CPUAddr = (uint32_t)dvr_malloc_specific(960*540,GFP_DCU2); //RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.SeqBufAddr;
//	memset((void *)pInfo.s2_CPUAddr,0x55,960*540);
//	pInfo.s2_Addr = 0x42c7e900;//(uint32_t)dvr_to_phys((unsigned int *)pInfo.s2_CPUAddr);

#endif
#endif
}

//#define FILM_MODE_DETECT_TEST
//#ifdef FILM_MODE_DETECT_TEST
static bool film_fw_tsk_running_flag = FALSE;
static struct task_struct *p_film_fw_tsk = NULL;
extern int ScalerVIP_SE_Proc(void);

//extern void scalerAI_preprocessing(void);
int cnt = 0;
static int film_mode_detect_tsk(void *p)//This task run new_game_mode_tsk
{
#ifdef CONFIG_MEMC_NOTSUPPORT // for code size reduce, when MEMC_BYPASS
		return 0;
#else

	unsigned int time1 = 0, time2 = 0, CadenceDetect_dbg_en = 0;
	rtd_pr_vpq_debug("film_mode_detect_tsk()\n");
//	pr_notice("[jerry_film_mode_detect_tsk_01]\n");
	current->flags &= ~PF_NOFREEZE;

	SE_FilmDtect_Init();
	CadenceDetect_dbg_en = (rtd_inl(SOFTWARE1_SOFTWARE1_63_reg)>>27)&0x1;

	while (1)
	{
        	_RPC_system_setting_info* RPC_system_info_structure_table = NULL;
        	unsigned char SE_rdPtr = 0;
        	unsigned char SE_wrPtr = 0;
        	unsigned char SE_status = 0;

            // to prevent STR/Snapshot fail
		if (freezing(current))
		{
			vpq_rtpm_tsk_Film_freeze_stauts = 1;
			try_to_freeze();
		}
		vpq_rtpm_tsk_Film_freeze_stauts = 0;
		if (kthread_should_stop()){
			rtd_pr_vpq_debug("film_mode_detect_tsk stop!!\n");
			break;
		}

		if (vpq_rtpm_tsk_block == 1) {
			msleep(1);
			vpq_rtpm_tsk_block_Film_done = 1;
			continue;
		} else {
			vpq_rtpm_tsk_block_Film_done = 0;
		}

		cnt++;
		if(cnt>6000){
			cnt = 1;
		}

        	RPC_system_info_structure_table = scaler_GetShare_Memory_RPC_system_setting_info_Struct();
        	if(RPC_system_info_structure_table==NULL)
        	{
        		//ROSPrintf(" Mem4 = %p\n",RPC_system_info_structure_table);
        		return -1;
        	}

	 	preempt_disable();
		_rtd_hwsem_lock(SB2_HD_SEM_NEW_3_reg, SEMA_HW_SEM_3_SCPU_2);
		SE_rdPtr = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.rdPtr;
		SE_rdPtr = (SE_rdPtr>=MAX_FILM_BUFF) ? 0 : SE_rdPtr;
		SE_wrPtr = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.wrPtr;
		SE_status = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.pic[SE_rdPtr].status;
		_rtd_hwsem_unlock(SB2_HD_SEM_NEW_3_reg, SEMA_HW_SEM_3_SCPU_2);
		preempt_enable();

		if(PPOVERLAY_Main_Display_Control_RSV_get_m_force_bg(rtd_inl(PPOVERLAY_Main_Display_Control_RSV_reg)) == 0)
			pr_debug("%d|%d@%d,T=%d,%d\n", SE_rdPtr, SE_wrPtr, SE_status, (time2 - time1)/90, (rtd_inl(TIMER_SCPU_CLK90K_LO_reg) - time2)/90);

		if(SE_status != 1){ // sleep when buffer empty
			if(cnt%1000 == 1 && CadenceDetect_dbg_en){
				pr_notice("[%s][status is wrong !!][SE_rdPtr,%d,][status,%d,]\n", __FUNCTION__, SE_rdPtr, SE_status);
			}
			hw_msleep(4);//need to switch
			pr_debug("SLEP\n");
			continue;
		}

		//Add Film mode detect code below++
		time1 = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);

//		pr_notice("[jerry_film_mode_detect_tsk_02]\n");
		ScalerVIP_SE_Proc();

		//Add Film mode detect code below++
		time2 = rtd_inl(TIMER_SCPU_CLK90K_LO_reg);

		rtd_pr_vpq_debug("film_mode_detect_tsk_mid(%d)\n", rtd_inl(TIMER_SCPU_CLK90K_LO_reg)-time1);
		//Add Film mode detect code above--
	}

    rtd_pr_vpq_debug("\r\n####film_mode_detect_tsk: exit...####\n");
    do_exit(0);
    return 0;
#endif
}



static void create_film_fw_tsk(void)
{
    int err;
	if (film_fw_tsk_running_flag == FALSE) {
		p_film_fw_tsk = kthread_create(film_mode_detect_tsk, NULL, "film_mode_detect_tsk");

	    if (p_film_fw_tsk) {
			wake_up_process(p_film_fw_tsk);
			film_fw_tsk_running_flag = TRUE;
	    } else {
	    	err = PTR_ERR(p_film_fw_tsk);
		rtd_pr_vpq_debug("Unable to start film_mode_detect_tsk (err_id = %d)./n", err);
	    }
	}
}

static void delete_film_fw_tsk(void)
{
	int ret;
	if (film_fw_tsk_running_flag && p_film_fw_tsk) {
 		ret = kthread_stop(p_film_fw_tsk);
 		if (!ret) {
 			p_film_fw_tsk = NULL;
 			film_fw_tsk_running_flag = FALSE;
			rtd_pr_vpq_info("film_fw_tsk thread stopped\n");
 		}
	}
}

//#endif
#define VT_SOURCE_CAP_I3DDMA_BUF	(0x0A)

#if 0 //IS_ENABLED(CONFIG_RTK_AI_DRV)
// TODO : thread get stuck in Android, current trigger in se_tsk
#define VPQ_PQMASK_STARTUP_DELAY 10
static bool pqmask_tsk_running_flag = FALSE;
struct timer_list PQMaskTimer;
struct semaphore Sem_PQMaskCtrl;
static struct task_struct *pPQMaskTask = NULL;

/**
 * @brief 
 * Used to noidfy the PQMask processing thread by semaphore to control speed
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static void PQMaskTimerFunctionEntry(struct timer_list *t) {
#else
void PQMaskTimerFunctionEntry(unsigned long arg)
#endif
	if( Sem_PQMaskCtrl.count < 1 )
		up(&Sem_PQMaskCtrl);
}

char PQMask_CreateTimer(void) {

	rtd_pr_vpq_emerg("[%s] start\n", __FUNCTION__);

	if (timer_pending(&PQMaskTimer)) {
		rtd_pr_vpq_emerg("Error, timer exist!\n");
		dump_stack();
		return -1;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	timer_setup(&PQMaskTimer, PQMaskTimerFunctionEntry, 0);
#else
	init_timer(&PQMaskTimer);
	PQMaskTimer.function = (void *)PQMaskTimerFunctionEntry;
	PQMaskTimer.data = ((unsigned long)0);
#endif
	PQMaskTimer.expires = jiffies + VPQ_PQMASK_STARTUP_DELAY*HZ;
	add_timer(&PQMaskTimer);

	rtd_pr_vpq_emerg("[%s] done\n", __FUNCTION__);

	return 0;
}

char PQMask_DeleteTimer(void) {

	rtd_pr_vpq_emerg("[%s] start\n", __FUNCTION__);

	if (timer_pending(&PQMaskTimer)) {
		del_timer_sync(&PQMaskTimer);
		rtd_pr_vpq_info("[%s] delete timer ok\n", __FUNCTION__);
	} else {
		rtd_pr_vpq_info("[%s] timer not exist\n", __FUNCTION__);
	}

	rtd_pr_vpq_emerg("[%s] done\n", __FUNCTION__);

	return 0;
}

static int PQMaskProcessThread(void *p) {

	unsigned long ProcStart = 0;
	unsigned long ProcEnd = 0;
	unsigned long TimePadding = 0;
	unsigned long TimeProc = 0;
	PQMASK_GLB_CTRL_T *pPQMaskGlbCtrl = (PQMASK_GLB_CTRL_T *)scalerPQMask_GetCtrlStruct(PQMASK_STRUCT_GLB_CTRL);

	rtd_pr_vpq_debug("[%s] start\n", __FUNCTION__);

	set_freezable();

	while(1) {

		if (freezing(current))
		{
			try_to_freeze();
		}

		if (kthread_should_stop()){
			rtd_pr_vpq_debug("[%s] stop\n", __FUNCTION__);
			break;
		}

		if(down_timeout(&Sem_PQMaskCtrl, msecs_to_jiffies(2000))) {	
			continue;
		}

		if( pPQMaskGlbCtrl->LimitFPS != 0 ) {
			ProcStart = drvif_Get_90k_Lo_clk();
			scalerPQMask_postprocessing_entry();
			ProcEnd = drvif_Get_90k_Lo_clk();
		}

		// re-trigger the timer
#if 1
		if( pPQMaskGlbCtrl->LimitFPS > 0 && aipq_mode.pqmask_mode != 0 ) {
			TimePadding = 1000/pPQMaskGlbCtrl->LimitFPS;
			if( ProcEnd > ProcStart )
				TimeProc = (ProcEnd-ProcStart)/90;
			else 
				TimeProc = (0xffffffff - (ProcStart-ProcEnd))/90;

			// minimum next trigger padding time = 1ms
			if( TimePadding > TimeProc )
				TimePadding = msecs_to_jiffies(TimePadding-TimeProc);
			else 
				TimePadding = msecs_to_jiffies(1);

		} else {
			TimePadding = msecs_to_jiffies(1000);
		}
		mod_timer(&PQMaskTimer, jiffies + TimePadding);
#else
		if( pPQMaskGlbCtrl->LimitFPS > 0 ) {
			if( (1000/pPQMaskGlbCtrl->LimitFPS) > ((ProcEnd-ProcStart)/90) )
				mod_timer(&PQMaskTimer, jiffies + msecs_to_jiffies(1000/pPQMaskGlbCtrl->LimitFPS));
			else 
				mod_timer(&PQMaskTimer, jiffies + msecs_to_jiffies(1);
		} else {
			TimePadding = HZ;
			mod_timer(&PQMaskTimer, jiffies + HZ);
		}
#endif
	}

	rtd_pr_vpq_debug("[%s] end\n", __FUNCTION__);
	do_exit(0);
	return 0;
}

char PQMask_CreateThread(void) {

	char cRet = 0;
	int err = 0;

	sema_init(&Sem_PQMaskCtrl, 0);
	PQMask_CreateTimer();

	if( pqmask_tsk_running_flag == FALSE ) {
		pPQMaskTask = kthread_create(PQMaskProcessThread, NULL, "PQMaskTask");

		if( pPQMaskTask ) {
			wake_up_process(pPQMaskTask);
			pqmask_tsk_running_flag = TRUE;
			cRet = 0;
		} else {
			err = PTR_ERR(pPQMaskTask);
			rtd_pr_vpq_emerg("Unable to start PQMaskTask (err_id = %d)./n", err);
			cRet = -1;
		}
	}

	return cRet;
}

char PQMask_DeleteThread(void) {

	char cRet = 0;

	PQMask_DeleteTimer();
	// sema_destroy(&Sem_PQMaskCtrl); // why

	if(pPQMaskTask){
		if (!kthread_stop(pPQMaskTask)) {
			pqmask_tsk_running_flag = FALSE;
			pPQMaskTask = NULL;
			rtd_pr_vpq_info("PQMaskTask thread stopped\n");
		}
	}else{
		rtd_pr_vpq_info("PQMaskTask thread stopped already\n");
	}
	return cRet;
}

#endif

#if IS_ENABLED(CONFIG_RTK_AI_DRV)
extern void scalerAI_SE_draw_Proc(void);
extern void scalerPQMaskColor_Function_En_CheckDummyRegister(void);
extern unsigned int vgip_isr_cnt;
static bool se_tsk_running_flag = FALSE;
static struct task_struct *p_se_tsk = NULL;
int sem_se_ai_flag=0;
struct semaphore sem_se_ai;
extern DRV_AI_Ctrl_table ai_ctrl;
unsigned char PQ_set_done = 0;
extern unsigned char h3ddma_get_cap_enable_mask(void);
extern AI_AUDIO_STATE aiAudioState;
/**
 * @brief 
 * for PQ module to trigger SE do necessary actions
 * @param p 
 * @return int 
 */
static int se_tsk(void *p)//This task run se proc
{
	static unsigned int vgip_isr_cnt_pre = 0;
	//h3ddma_cap1_cap_status_RBUS h3ddma_cap1_cap_status_reg;
	od_od_ctrl_RBUS od_ctrl_reg;
	char try_lock_times = 20;

	rtd_pr_vpq_debug("se_tsk()\n");

	current->flags &= ~PF_NOFREEZE;

	while (1)
	{
		static unsigned char source_pre = 255;
		unsigned char source_cur = 255;
		int nn_en=0;//, memc_en=0;
		// lesley 0920
		int signal_cnt_th = ai_ctrl.ai_global3.signal_cnt_th;//5
		// end lesley 0920

		// to prevent STR/Snapshot fail
		if (freezing(current))
		{
			vpq_rtpm_tsk_SE_freeze_stauts = 1;
			try_to_freeze();
		}
		vpq_rtpm_tsk_SE_freeze_stauts = 0;
		if (kthread_should_stop()){
			rtd_pr_vpq_debug("[%s] stop!!\n", __FUNCTION__);
			break;
		}

		if (vpq_rtpm_tsk_block == 1) {
			msleep(1);
			vpq_rtpm_tsk_block_SE_done = 1;
			continue;
		} else {
			vpq_rtpm_tsk_block_SE_done = 0;
		}

		od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
		if(od_ctrl_reg.dummy1802ca00_31_12>>19 & 1) nn_en = 1; // bit 31

		//if(od_ctrl_reg.dummy1802ca00_31_12>>17 & 1) memc_en = 1; // bit 29

		// scalerPQMask_BWTest();

		try_lock_times = 40;
#ifdef VPQ_RunTime_PM_Enable
		while ((down_timeout(&sem_se_ai, msecs_to_jiffies(1*50) /*50 ms*/)) && (try_lock_times > 0) && (vpq_rtpm_tsk_block == 0))	
			try_lock_times--;

		if ((try_lock_times <= 0) || (vpq_rtpm_tsk_block == 1)) {	
			//rtd_pr_vpq_info("VPQ_RTPM, try_lock_times=%d, vpq_rtpm_tsk_block=%d,,\n", try_lock_times, vpq_rtpm_tsk_block);
			continue;
		}
#else
		if(down_timeout(&sem_se_ai, msecs_to_jiffies(2*1000) /*2sec*/)) {
		//if(down_timeout(&sem_se_ai, (2*1000) /*2sec*/)) {
			continue;
		}
#endif
		// lesley 0920
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE)
		{
			signal_cnt = 0;
		}
		else
		{
			if(signal_cnt < signal_cnt_th)
			{
				signal_cnt++;
			}
		}
		// end lesley 0920
		scalerPQMask_postprocessing_entry();
		scalerPQMaskColor_Function_En_CheckDummyRegister();//====for Demo Use====Remove it after Demo====

		if((vgip_isr_cnt != vgip_isr_cnt_pre))
		{
			_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

			vgip_isr_cnt_pre = vgip_isr_cnt;

			/* check source chaging or not */
			source_cur = VIP_RPC_system_info_structure_table->VIP_source;
			//source_cur = fwif_vip_source_check(3, NOT_BY_DISPLAY);
			if(source_cur != source_pre)
			{
				rtd_printk(KERN_INFO,"VPQ_AI", "timing changed\n");
				fw_scalerip_reset_NN();
			}

#if defined(CONFIG_RTK_8KCODEC_INTERFACE)
			VPQEX_rlink_AI_SeneInfo();
#endif

			#if 0
			// Check NN write done
			h3ddma_cap1_cap_status_reg.regValue = IoReg_Read32(H3DDMA_CAP1_Cap_Status_reg);
			if (h3ddma_cap1_cap_status_reg.cap1_cap_last_wr_flag)
			{
				//pr_emerg("vgip_isr, h3ddma_set_film_buffer_addr\n");
				h3ddma_set_film_buffer_addr();

				IoReg_Write32(H3DDMA_CAP1_Cap_Status_reg, h3ddma_cap1_cap_status_reg.regValue);
			}
			#endif

#if I3DMA_MULTICROP_MODE > 0
			if(aipq_mode.ap_mode>AI_MODE_ON)
				rtd_printk(KERN_INFO,"VPQ_AI", "se_tsk:nn_en=%d,i3ddma_enable=%d,src=%d\n",nn_en,h3ddma_get_cap_enable_mask(),Get_DisplayMode_Src(SLR_MAIN_DISPLAY));

			if(nn_en &&
				(((h3ddma_get_cap_enable_mask()&0x1)==1)||(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG)))
#else
			if(nn_en &&
				((fwif_color_get_force_run_i3ddma_enable(SLR_MAIN_DISPLAY))/*(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)*/ ||(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG)))
#endif
			{
				// 0622 lsy
				//if(vpq_stereo_face != AI_PQ_AP_OFF)
				if(aipq_mode.ap_mode >= AI_MODE_ON)
				// end 0622 lsy
				{
					scalerAI_preprocessing();
				}
			}
			//scalerAI_SE_draw_Proc();

			/* update pre source */
			source_pre = source_cur;
		}

		if((PQ_set_done && get_svp_protect_status_aipq()==0) || (aiAudioState.dbgOsdOn>=1)  )
		{
			scalerAI_SE_draw_Proc();
			PQ_set_done = 0;
		}


	}

    rtd_pr_vpq_debug("\r\n####[%s] exit...####\n", __FUNCTION__);
    do_exit(0);
    return 0;
}

static void create_se_tsk(void)
{
    int err;

	sema_init(&sem_se_ai, 0);
	sem_se_ai_flag = 1;

	if (se_tsk_running_flag == FALSE) {
		p_se_tsk = kthread_create(se_tsk, NULL, "se_tsk");

	    if (p_se_tsk) {
			wake_up_process(p_se_tsk);
			se_tsk_running_flag = TRUE;
	    } else {
	    	err = PTR_ERR(p_se_tsk);
		rtd_pr_vpq_debug("Unable to start se_tsk (err_id = %d)./n", err);
	    }
	}
}

static void delete_se_tsk(void)
{
	int ret;
	if (se_tsk_running_flag && p_se_tsk) {
 		ret = kthread_stop(p_se_tsk);
 		if (!ret) {
 			p_se_tsk = NULL;
 			se_tsk_running_flag = FALSE;
			rtd_pr_vpq_info("se_tsk thread stopped\n");
 		}
	}
}
#endif

static bool VPQ_fw_tsk_running_flag = FALSE;
static struct task_struct *p_VPQ_task = NULL;
#ifdef CONFIG_RTK_8KCODEC_INTERFACE
	extern void VPQMEMC_rlink_MEMC_ID(void);
	extern unsigned char R8K_MEMC_ID_update;
	extern char VPQEX_rlink_host_Noise_Level_Info_Send(void);
#endif
#if defined(CONFIG_H5CX_SUPPORT)
extern char VPQEX_rlink_host_Noise_Level_Info_Send(void);
extern char VPQEX_rlink_set_Low_Delay(unsigned char low_delay_mode);
#endif

extern unsigned char demura_write_en;
extern unsigned char* LD_virAddr;
void Scaler_Get_Logo_Detect_VirAddr(void);
void Scaler_Check_Logo_Detect_VirAddr(void);

static int VPQ_detect_tsk(void *p)
{
	extern VIP_HDR10_EOTF_ByLuminace_TBL HDR10_EOTF_ByLuminace_TBL_cur;
	extern VIP_HDR10_OETF_ByLuminace_TBL HDR10_OETF_ByLuminace_TBL_cur;

#if 0
    struct cpumask vsc_cpumask;
    rtd_pr_vpq_debug("avd_detect_tsk()\n");
    cpumask_clear(&vsc_cpumask);
    cpumask_set_cpu(0, &vsc_cpumask); // run task in core 0
    cpumask_set_cpu(2, &vsc_cpumask); // run task in core 2
    cpumask_set_cpu(3, &vsc_cpumask); // run task in core 3
    sched_setaffinity(0, &vsc_cpumask);
#endif
	current->flags &= ~PF_NOFREEZE;
    while (1)
    {
		msleep(5);

		if (freezing(current))
		{
			vpq_rtpm_tsk_VPQ_freeze_stauts = 1;
			try_to_freeze();
		}
		vpq_rtpm_tsk_VPQ_freeze_stauts = 0;
		if (kthread_should_stop()) {
         		break;
      		}

		if (vpq_rtpm_tsk_block == 1) {
			//msleep(1);
			vpq_rtpm_tsk_block_VPQ_done = 1;
			continue;
		} else {
			vpq_rtpm_tsk_block_VPQ_done = 0;
		}

#if defined(CONFIG_RTK_8KCODEC_INTERFACE)
		if(R8K_MEMC_ID_update == 1){
			VPQMEMC_rlink_MEMC_ID();
			R8K_MEMC_ID_update = 0;
			rtd_pr_vpq_info("[MEMC]VPQMEMC_rlink_MEMC_ID\n");
		}
		VPQEX_rlink_host_Noise_Level_Info_Send();
#endif
#if defined(CONFIG_H5CX_SUPPORT)
		VPQEX_rlink_host_Noise_Level_Info_Send();
#endif

		if (((HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[29] & 0xFFFFFFFF) != 0) && (HDR10_OETF_ByLuminace_TBL_cur.OETF_setting[29] == 1))
			msleep(5);
		else
			Scaler_color_Get_HDR10_EOTF_OETF_ByMaxCLL_ByPanelLuminance_for_task();

		Scaler_color_set_HDR_AutoRun_TBL();
		//rtd_pr_vpq_info("VPQ_detect_tsk\n");

		fwif_color_set_PQ_ByPass_Handler_VPQ_TSK();
		fwif_color_NNSR_model_TBL_update_VPQTSK();

		/* HDR vivid */
		fwif_color_ScalerVIP_HDR_VIVID_frameSync();
		fwif_color_HDR_VIVID_frameSync_DMA_Apply_TASK();

#ifdef CONFIG_SUPPORT_SLD_BY_MEMC_DEMURA
			if( LD_virAddr == NULL && MEMC_First_Run_force_SetInOutUseCase_Done == 1 && SLD_SW_En == 1)
			{
				Scaler_Get_Logo_Detect_VirAddr();
			}
	
			if( LD_virAddr != NULL && SLD_SW_En == 1)
			{
				Scaler_Check_Logo_Detect_VirAddr();
			}
	
			if( demura_write_en && SLD_SW_En == 1)
			{
				fwif_color_DeMura_init(0, 1);
				demura_write_en = false;
			}
#endif


    }

    rtd_pr_vpq_debug("\r\n####VPQ_detect_tsk: exit...####\n");
    do_exit(0);
    return 0;
}

static void delete_VPQ_tsk(void)
{
	int ret;

	if (VPQ_fw_tsk_running_flag && p_VPQ_task) {
 		ret = kthread_stop(p_VPQ_task);
 		if (!ret) {
 			p_VPQ_task = NULL;
 			VPQ_fw_tsk_running_flag = FALSE;
			rtd_pr_vpq_info("VPQ_tsk thread stopped\n");
 		}
	}
}

static void create_VPQ_tsk(void)
{
	int err;

	if (VPQ_fw_tsk_running_flag == FALSE) {
		p_VPQ_task = kthread_create(VPQ_detect_tsk, NULL, "VPQ_detect_tsk");

	    if (p_VPQ_task) {
			wake_up_process(p_VPQ_task);
			VPQ_fw_tsk_running_flag = TRUE;
	    } else {
	    	err = PTR_ERR(p_VPQ_task);
	    	rtd_pr_vpq_emerg("Unable to start VPQ_tsk (err_id = %d)./n", err);
	    }
	}

}

char vpq_pm_runtime_get(unsigned char isSyncFlag)
{
#ifdef VPQ_RunTime_PM_Enable
	if (isSyncFlag == 1) {
		pm_runtime_get_sync(&vpq_platform_devs->dev);
		VPQ_PM_LOG("info", "open", "VPQ_RTPM dev open, sync=1");
		
	} else {
		
		pm_runtime_get(&vpq_platform_devs->dev);
		VPQ_PM_LOG("info", "open", "VPQ_RTPM dev open, sync=0");

	}
	//rtd_pr_vpq_info("VPQ_RTPM, get, sync Flag=%d,\n", isSyncFlag);
#endif	
	return 0;
}

char vpq_pm_runtime_put(unsigned char isSyncFlag)
{
#ifdef VPQ_RunTime_PM_Enable
	if (isSyncFlag == 1) {
		
		pm_runtime_put_sync(&vpq_platform_devs->dev);
		VPQ_PM_LOG("info", "close", "VPQ_RTPM dev close, sync=1");
		
	} else {
		
		pm_runtime_put(&vpq_platform_devs->dev);
		VPQ_PM_LOG("info", "close", "VPQ_RTPM dev close, sync=0");
		
	}
	//rtd_pr_vpq_info("VPQ_RTPM, put, sync Flag=%d,\n", isSyncFlag);
#endif
	return 0;
}

unsigned char vpq_get_VPQ_TSK_Stop(void) 
{
	unsigned char ret;
	unsigned char freeze_VPQ_done, se_done, film_done;

	if (VPQ_fw_tsk_running_flag == 1)
		freeze_VPQ_done = vpq_rtpm_tsk_block_VPQ_done;
	else
		freeze_VPQ_done = 1;
	
	if (se_tsk_running_flag == 1)
		se_done = vpq_rtpm_tsk_block_SE_done;
	else
		se_done = 1;

	if (film_fw_tsk_running_flag == 1)
		film_done = vpq_rtpm_tsk_block_Film_done;
	else
		film_done = 1;

	if ((vpq_rtpm_tsk_VPQ_freeze_stauts == 1) && (vpq_rtpm_tsk_SE_freeze_stauts == 1) && (vpq_rtpm_tsk_Film_freeze_stauts == 1))
		ret = 1;
	else if ((freeze_VPQ_done == 1) && (se_done == 1) && (film_done == 1) && (vpq_rtpm_tsk_block == 1) )
		ret = 1;
	else
		ret = 0;

	return ret;
}

unsigned char vpq_set_VPQ_TSK_Stop(unsigned char stop_en)
{
#ifdef VPQ_RunTime_PM_Enable	
	if (stop_en == 1)
		vpq_rtpm_tsk_block = 1;
	else 
		vpq_rtpm_tsk_block = 0;
	
	rtd_pr_vpq_info("VPQ_RTPM, set tsk stop = %d,\n", stop_en);
#endif
	return 0;
}

bool vpq_check_array(unsigned int *arg, unsigned int *pre_arg, unsigned int len)
{
	unsigned int i;

	for(i=0;i<len;i++) {
		if(arg[i] != pre_arg[i])
			return 0;
	}
	return 1;
}

#endif //UT_flag

unsigned char ioctl_cmd_stop[256] = {0};
unsigned char vpq_ioctl_get_stop_run(unsigned int cmd)
{
	return (ioctl_cmd_stop[_IOC_NR(cmd)&0xff]|ioctl_cmd_stop[0]);
}

unsigned char vpq_ioctl_get_stop_run_by_idx(unsigned char cmd_idx)
{
	return ioctl_cmd_stop[cmd_idx];
}

void vpq_ioctl_set_stop_run_by_idx(unsigned char cmd_idx, unsigned char stop)
{
	ioctl_cmd_stop[cmd_idx] = stop;
}

unsigned char HDR_ioctl_cmd_stop[256] = {0};
unsigned char vpq_HDR_ioctl_get_stop_run(unsigned int cmd)
{
	return (HDR_ioctl_cmd_stop[_IOC_NR(cmd)&0xff]|HDR_ioctl_cmd_stop[0]);
}

unsigned char vpq_HDR_ioctl_get_stop_run_by_idx(unsigned char cmd_idx)
{
	return HDR_ioctl_cmd_stop[cmd_idx];
}

void vpq_HDR_ioctl_set_stop_run_by_idx(unsigned char cmd_idx, unsigned char stop)
{
	HDR_ioctl_cmd_stop[cmd_idx] = stop;
}

unsigned char dolbyHDR_ioctl_cmd_stop[256] = {0};
unsigned char vpq_dolbyHDR_ioctl_get_stop_run(unsigned int cmd)
{
	return (dolbyHDR_ioctl_cmd_stop[_IOC_NR(cmd)&0xff]|dolbyHDR_ioctl_cmd_stop[0]);
}

unsigned char vpq_dolbyHDR_ioctl_get_stop_run_by_idx(unsigned char cmd_idx)
{
	return dolbyHDR_ioctl_cmd_stop[cmd_idx];
}

void vpq_dolbyHDR_ioctl_set_stop_run_by_idx(unsigned char cmd_idx, unsigned char stop)
{
	dolbyHDR_ioctl_cmd_stop[cmd_idx] = stop;
}

unsigned char vpq_get_handler_bypass(void)
{
#ifdef CONFIG_SCALER_BRING_UP
	return 1;
#endif
	return ioctl_cmd_stop[VPQ_IOC_PQ_CMD_OPEN];
}

COLORTEMP_ELEM_T ct;

DRV_Sharpness_Level Sharpness_Level;
RTK_NOISE_REDUCTION_T NR_Level;
RTK_MPEG_NOISE_REDUCTION_T MPEGNR_Level;
CHIP_DCC_T tFreshContrast_coef;
FreshContrastLUT_T tFreshContrastLUT;
VPQ_SetPicCtrl_T pic_ctrl = {0, {100, 50, 50, 0}, {128, 128, 128, 128}};
COLOR_GAMUT_T gamutData;
VPQ_SAT_LUT_T satLutData;
HDR_Set3DLUT_16_T phdr3dlut16;
UINT32 Eotf_table32[EOTF_size] = {0};
UINT16 Oetf_table16[OETF_size] = {0};
UINT16 Oetf_table16_prog[OETF_PROG_SIZE_TV006] = {0};
extern struct semaphore* get_gamemode_check_semaphore(void);

int PictureMode_flg = 0;  // for MEMC wrt by JerryWang 20161125
CHIP_EDGE_ENHANCE_UI_T tCHIP_EDGE_ENHANCE_UI_T;
HDR_SetGamut33Matrix_T gamut33;
HAL_VPQ_CLEAR_WHITE clear_white;
DRV_VIP_YUV2RGB_UV_Offset uvOffset;
DRV_VIP_YUV2RGB_UV_Offset_byUV_CURVE uv_offset_byuv_curve;
HAL_VPQ_SET_BLACK_LEVEL blackLv;
UINT32 HDR_info_bin[131] = {0};
//extern bool get_support_vo_force_v_top(unsigned int func_flag);

long vpq_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
#ifdef RTK_PQ_demo_mode		//for demo
	extern bool PQ_demo_mode;
#endif
	int ret = 0;
	unsigned char isBlack_LvDiff_Flag = 0;
	unsigned char mCon, mBri, mHue, mSat;
	unsigned char  input_info;
	unsigned char  level;
	UINT8 data_protect_cfu = 100;
	HAL_VPQ_DATA_T Eotf_data;
	HAL_VPQ_DATA_T Oetf_data;
	HAL_VPQ_DATA_T PQModeInfo_data;
	UINT8 HDR_InvGamma_en = 0;
	HAL_VPQ_DATA_T OD_Info;
	HAL_VPQ_DATA_T DeMura_Info;
	static int refCnt=0;
	

        UINT8 source=0;

        SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
        gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
        source = fwif_vip_source_check(3, NOT_BY_DISPLAY);/* get vip source by input src and timing*/
        if (source >= VIP_QUALITY_SOURCE_NUM)
                source = 0;

#ifdef VPQ_RunTime_PM_Enable
	if(vpq_get_VPQ_TSK_Stop() ==1){
		rtd_pr_vpq_emerg("vpq hal function block (%s:%d) /n", __FUNCTION__, __LINE__);
		return 0;
	}
#endif

	if (vpq_ioctl_get_stop_run(cmd))
		return 0;

#ifdef RTK_PQ_demo_mode		//for demo
	if(PQ_demo_mode == 1 && cmd == VPQ_IOC_SET_PQModeInfo){
		unsigned char buf[1] = {0};
		(fwif_color_get_demo_callback(DEMO_CALLBACKID_ON_OFF_SWITCH))((void *)buf);
		PQ_demo_mode = 0;
	}
#endif

/* //no more need
	if (vpq_project_id != 0x00060000) {
		if (VPQ_IOC_INIT == cmd) {
			vpq_project_id = 0x00060000;
			vpq_boot_init();
		} else {
			if (Demo_Flag && VPQ_IOC_SET_PIC_CTRL == cmd) {
				vpq_demo_overscan_func(NULL);
				return 0;
			}
			return -1;
		}
	}
*/
	if (VPQ_IOC_INIT == cmd ) {
		
		if(PQ_Dev_Status != PQ_DEV_INIT_DONE){
			//fwif_color_inv_gamma_control_back(SLR_MAIN_DISPLAY, 0);
			//fwif_color_gamma_control_back(SLR_MAIN_DISPLAY, 1);
			//fwif_color_gamma_control_back(SLR_SUB_DISPLAY, 1);
#ifdef CONFIG_DUAL_CHANNEL
			//fwif_color_inv_gamma_control_back(SLR_SUB_DISPLAY, 0);
			//fwif_color_gamma_control_back(SLR_SUB_DISPLAY, 1);
#endif
			fwif_color_set_DCC_Init(SLR_MAIN_DISPLAY);
			/*fwif_color_set_DCC_Init_tv006();*/
			fwif_color_set_dcc_force_write_flag_RPC(1, 0);
	
			// enable DM & Composer clock
			drvif_Set_DM_HDR_CLK();
			//drvif_TV006_HDR10_init();
			fwif_color_set_FILM_FW_ShareMemory();
			fwif_color_set_Picture_Control_tv006(pic_ctrl.wId, &pic_ctrl);
	
			Scaler_color_Set_HDR_AutoRun(TRUE); // use HFC to set HDR inv gamma and EOTF for TV006
		}
		refCnt++;
		
		PQ_Dev_Status = PQ_DEV_INIT_DONE;
		return 0;
	} else if (VPQ_IOC_UNINIT == cmd) {
		refCnt--;
		if(refCnt<=0){
			PQ_Dev_Status = PQ_DEV_UNINIT;
		}
		return 0;
	} else if (VPQ_IOC_OPEN == cmd || VPQ_IOC_CLOSE == cmd) {
		return 0;
	} else if (VPQ_IOC_SET_PROJECT_ID == cmd) {
		//vpq_project_id = arg;
		/*rtd_pr_vpq_debug("vpq_project_id = %x\n", vpq_project_id);*/
		return 0;
	}

	if (PQ_Dev_Status != PQ_DEV_INIT_DONE)
		return -1;

	switch (cmd) {
	/*
	case VPQ_IOC_INIT:
		if (PQ_DEV_UNINIT == PQ_Dev_Status || PQ_DEV_NOTHING == PQ_Dev_Status) {
			fwif_color_inv_gamma_control_back(SLR_MAIN_DISPLAY, 1);
			fwif_color_gamma_control_back(SLR_MAIN_DISPLAY, 1);
			PQ_Dev_Status = PQ_DEV_INIT_DONE;
		} else
			return -1;
		break;

	case VPQ_IOC_UNINIT:
		if (PQ_DEV_CLOSE == PQ_Dev_Status || PQ_DEV_INIT_DONE == PQ_Dev_Status) {
			PQ_Dev_Status = PQ_DEV_UNINIT;
		} else
			return -1;
		break;

	case VPQ_IOC_OPEN:
		if (PQ_DEV_CLOSE == PQ_Dev_Status || PQ_DEV_INIT_DONE == PQ_Dev_Status) {
			PQ_Dev_Status = PQ_DEV_OPEN_DONE;
		} else
			return -1;
		break;

	case VPQ_IOC_CLOSE:
		if (PQ_DEV_OPEN_DONE == PQ_Dev_Status) {
			PQ_Dev_Status = PQ_DEV_CLOSE;
		} else
			 return -1;
		break;
	*/
	case VPQ_IOC_SET_COLOR_TEMP:
		{

			if (copy_from_user(&ct, (int __user *)arg, sizeof(COLORTEMP_ELEM_T))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_SET_COLOR_TEMP fail\n");
				ret = -1;
			} else {
				memcpy(&curColorTemp, &ct, sizeof(COLORTEMP_ELEM_T));
				vpq_set_color_temp_filter();
				/*rtd_pr_vpq_debug("kernel VPQ_IOC_SET_COLOR_TEMP success\n");*/
				ret = 0;
			}
		}
		break;
	case VPQ_IOC_SET_COLOR_FILTER:
		{
			unsigned int args;
			if (copy_from_user(&args, (int __user *)arg, sizeof(unsigned int))) {
				rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_PIC_CTRL fail\n");
				ret = -1;
			} else {
				ucColorFilterMode = (unsigned char)args;
				vpq_set_color_temp_filter();
				ret = 0;
			}
		}
		break;
	case VPQ_IOC_GET_MOTION_LEVEL:
		{
			_clues *smartPic_clue;
			UINT32 PQA_Motion_Level = 0;

			/* Get Info from Share Mem*/
			smartPic_clue = fwif_color_Get_SmartPic_clue();
			if (0 == smartPic_clue)
				return -1;
			PQA_Motion_Level = smartPic_clue->motion_ratio;
			if (copy_to_user((void __user *)arg, (void *)&PQA_Motion_Level, sizeof(UINT32))) {
				ret = -1;
				rtd_pr_vpq_err("kernel VPQ_IOC_GET_MOTION_LEVEL fail\n");
			} else {
				rtd_pr_vpq_err("[henry] get motion level\n");
				ret = 0;
				/*rtd_pr_vpq_debug("kernel VPQ_IOC_GET_MOTION_LEVEL success\n");*/
			}

		}
		break;
	case VPQ_IOC_GET_NOISE_LEVEL:
		{
			_clues *smartPic_clue;
			UINT32 PQA_Noise_Level = 0;

			/* Get Info from Share Mem*/
			smartPic_clue = fwif_color_Get_SmartPic_clue();
			if (0 == smartPic_clue)
				return -1;
			PQA_Noise_Level = smartPic_clue->noise_ratio;
			if (copy_to_user((void __user *)arg, (void *)&PQA_Noise_Level, sizeof(UINT16))) {
				ret = -1;
				rtd_pr_vpq_err("kernel VPQ_IOC_GET_NOISE_LEVEL fail\n");
			} else {
				ret = 0;
				/*rtd_pr_vpq_debug("kernel VPQ_IOC_GET_NOISE_LEVEL success\n");*/
			}

		}
		break;
	case VPQ_IOC_GET_Input_V_Freq:
		{
			unsigned int Input_V_Freq;

			Input_V_Freq = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ);
			if (copy_to_user((void __user *)arg, (void *)&Input_V_Freq, sizeof(unsigned int))) {
				ret = -1;
				rtd_pr_vpq_err("kernel VPQ_IOC_GET_Input_V_Freq fail\n");
			} else {
				ret = 0;
				/*rtd_pr_vpq_debug("kernel VPQ_IOC_GET_Input_V_Freq success\n");*/
			}
		}
		break;
	case VPQ_IOC_SET_GAMMA_LUT:
		{
			if (copy_from_user(&gamma, (int __user *)arg, sizeof(gamma))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_SET_GAMMA_LUT fail\n");
				ret = -1;
			} else {
				vpq_set_OSD_gamma();
				vpq_set_gamma(0, 1);
				fwif_color_gamma_control_back(SLR_MAIN_DISPLAY, 0);
				ret = 0;
			}
		}
		break;
	case VPQ_IOC_SET_SPLIT_DEMO:
		{
			bool bOnOff;

			if (copy_from_user(&bOnOff, (int __user *)arg, sizeof(bool))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_SET_SPLIT_DEMO fail\n");
				ret = -1;
			} else {
				if (bOnOff == TRUE)
					Scaler_SetMagicPicture(SLR_MAGIC_STILLDEMO);
				else
					Scaler_SetMagicPicture(SLR_MAGIC_OFF);
			}
		}
		break;
	case VPQ_IOC_SET_IRE_INNER_PATTERN:
		{
			unsigned int args;
			if (copy_from_user(&args, (int __user *)arg, sizeof(unsigned int))) {
				rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_IRE_INNER_PATTERN fail\n");
				ret = -1;
			} else {
				fwif_color_set_WB_Pattern_IRE(args>>16, (args&0xffff)*2); //IREx2
				ret = 0;
			}
		}
		break;
	case VPQ_IOC_SET_FILM_MODE:
		{
			unsigned char bCinemaMode;
			if (copy_from_user(&film_mode, (int __user *)arg, sizeof(film_mode))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_SET_FILM_MODE fail\n");
				ret = -1;
			} else {
				//down(get_DI_semaphore());
				drvif_module_film_mode((DRV_film_mode *) &film_mode);
				//up(get_DI_semaphore());
				/*rtd_pr_vpq_debug("kernel VPQ_IOC_SET_FILM_MODE success\n");*/

				down(&Memc_Realcinema_Semaphore);
				if (film_mode.film_status ==0)
					bCinemaMode = 0;
				else
					bCinemaMode = 1;
				
					//memc_realcinema_framerate();
				DbgSclrFlgTkr.memc_realcinema_run_flag = TRUE;//Run memc mode
				
				fwif_color_set_cinema_mode_en(bCinemaMode);
				ret = Scaler_MEMC_set_cinema_mode_en(bCinemaMode);
				up(&Memc_Realcinema_Semaphore);

			}
		}
		break;
	case VPQ_IOC_SET_FILM_TABLE:
		{
			//return 0;/*fix table not match from hal, use internal finetune*/
			if (copy_from_user(&film_table, (int __user *)arg, sizeof(DRV_film_table_t))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_SET_FILM_TABLE fail\n");
				ret = -1;
			} else {
				//down(get_DI_semaphore());
				drvif_module_film_setting_tv006((DRV_film_table_t *) &film_table);
				//up(get_DI_semaphore());
				/*rtd_pr_vpq_debug("kernel VPQ_IOC_SET_FILM_TABLE success\n");*/
				ret = 0;
			}
		}
		break;

	case VPQ_IOC_SET_FILM_FPS:
		{

			unsigned int args;
			if (copy_from_user(&args, (int __user *)arg, sizeof(unsigned int))) {
				rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_FILM_FPS fail\n");
				ret = -1;
			} else {
				unsigned char bIs48HzMode = false;
				bIs48HzMode = (args&0xff) ? 1 : 0;
				rtk_vsc_setFilmMode(bIs48HzMode);
				ret = 0;
			}
		}
		break;

	case VPQ_IOC_SET_SHARPNESS_LEVEL:
		{
                #ifndef LGDB_OFF
			return 0; //fixed me after demo please remove
                #endif
			if (copy_from_user(&Sharpness_Level, (int __user *)arg, sizeof(DRV_Sharpness_Level))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_SET_SHARPNESS_LEVEL fail\n");
				ret = -1;
			} else {
				drvif_color_set_Sharpness_level((DRV_Sharpness_Level *) &Sharpness_Level);
				/*rtd_pr_vpq_debug("kernel VPQ_IOC_SET_SHARPNESS_LEVEL success\n");*/
				ret = 0;
			}
		}
		break;

	case VPQ_IOC_SET_SHARPNESS_TABLE:
		{
                #ifndef LGDB_OFF
			static CHIP_SHARPNESS_UI_T tCHIP_SHARPNESS_UI_T;


			if (copy_from_user(&tCHIP_SHARPNESS_UI_T, (void __user *)arg, sizeof(CHIP_SHARPNESS_UI_T))) {
				rtd_pr_vpq_emerg("kernel VPQ_IOC_SET_SHARPNESS_TABLE fail\n");
				ret = -1;
			} else {

				memcpy(&tCurCHIP_SHARPNESS_UI_T, &tCHIP_SHARPNESS_UI_T, sizeof(CHIP_SHARPNESS_UI_T));
				fwif_color_set_sharpness_table_TV006(&tCurCHIP_SHARPNESS_UI_T, &tCurCHIP_SR_UI_T);
				ret = 0;
			}
                #else    //control table by RTK
			static CHIP_SHARPNESS_UI_T tCHIP_SHARPNESS_UI_T;
			/*CHIP_EDGE_ENHANCE_UI_T tCHIP_EDGE_ENHANCE_UI_T;*/
			/*UINT16 sEdgeEnhanceValue = tShp_Val[4];*/ /* 0: off 1:on*/

				UINT8 source=0,idx=0;
				SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
				gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
				if (gVip_Table == NULL) {
					return -1;
				}
                                ret = 0;

				/*get pqa table index need check source first. because ap flow or table index will get incorrect table*/
				source = fwif_vip_source_check(3, NOT_BY_DISPLAY);/* get vip source by input src and timing*/
				if (source >= VIP_QUALITY_SOURCE_NUM)
					source = 0;

				idx = gVip_Table->VIP_QUALITY_Extend3_Coef[source][VIP_QUALITY_FUNCTION_SharpTable];

				Scaler_SetSharpnessTable(idx);
				Scaler_SetSharpness(50); //fixed me

				Scaler_SetDCti(gVip_Table->VIP_QUALITY_Coef[source][VIP_QUALITY_FUNCTION_DCTi]); //fixed me
				Scaler_SetIDCti(gVip_Table->VIP_QUALITY_Coef[source][VIP_QUALITY_FUNCTION_I_DCTi]);//fixed me
				Scaler_SetDLti(gVip_Table->VIP_QUALITY_Coef[source][VIP_QUALITY_FUNCTION_DLTi]);//fixed me
        			Scaler_SetMBPeaking(gVip_Table->VIP_QUALITY_Extend3_Coef[source][VIP_QUALITY_FUNCTION_MB_Peaking]); //fixed me after need to remove
        			Scaler_SetMBSUPeaking(gVip_Table->VIP_QUALITY_Extend3_Coef[source][VIP_QUALITY_FUNCTION_MB_SU_Peaking]); //fixed me after need to remove

				//ret = 0;

				//return 0;

			if (copy_from_user(&tCHIP_SHARPNESS_UI_T, (void __user *)arg, sizeof(CHIP_SHARPNESS_UI_T))) {
				rtd_pr_vpq_emerg("kernel VPQ_IOC_SET_SHARPNESS_TABLE fail\n");
				ret = -1;
			} else {

                        //rtd_pr_vpq_emerg("rord rord sharpness is ok \n");
				//memcpy(&tCurCHIP_SHARPNESS_UI_T, &tCHIP_SHARPNESS_UI_T, sizeof(CHIP_SHARPNESS_UI_T));
				/*memcpy(&tCHIP_EDGE_ENHANCE_UI_T, &tCurCHIP_EDGE_ENHANCE_UI_T, sizeof(CHIP_EDGE_ENHANCE_UI_T));*/
				/*if (uc3DCorrectionEn == false) {*/
					//fwif_color_set_sharpness_table_TV006(&tCurCHIP_SHARPNESS_UI_T, &tCurCHIP_EDGE_ENHANCE_UI_T, &tCurCHIP_SR_UI_T);
				/*
					drvif_color_set_Vertical_Peaking_Enable(true);
				} else {
					fwif_color_set_SharpnessFilterCoef_for_3D((CHIP_SHARPNESS_UI_T *)&tCHIP_SHARPNESS_UI_T, (CHIP_SHARPNESS_UI_T *) &t3DCHIP_SHARPNESS_UI_T);
					fwif_color_set_sharpness_table_TV006(tCHIP_SHARPNESS_UI_T, tCHIP_EDGE_ENHANCE_UI_T, sEdgeEnhanceValue);
					drvif_color_set_Vertical_Peaking_Enable(false);
				}
				*/
				/*rtd_pr_vpq_debug("kernel VPQ_IOC_SET_SHARPNESS_TABLE success\n");*/
				ret = 0;
		}
                #endif
        }

	break;

	case VPQ_IOC_SET_VPQ_Shp_Val:
	{
        #ifndef LGDB_OFF

        	signed char weitht_value=0;

        	ret = 0;

		if (copy_from_user(&tShp_Val, (int __user *)arg, sizeof(UINT16)*7)) {
			rtd_pr_vpq_err("kernel VPQ_IOC_SET_VPQ_Shp_Val fail\n");
			ret = -1;
		} else {

			//rtd_pr_vpq_emerg("rord val[0]=%d,val[1]=%d,val[2]=%d \n",tShp_Val[0],tShp_Val[1],tShp_Val[2]);

                        tShp_Val[1] = (tShp_Val[1]>=31)?31:tShp_Val[1];

                        if(tShp_Val[1] >= 31)
                        {
                                 weitht_value=31;
                        }
                        else if(tShp_Val[1] <= 10)
                        {
                                weitht_value=0;
                        }
                        else
                        {
                                weitht_value = (tShp_Val[1]- 10)*31/( 31- 10);

                        }

                        weitht_value = 31-weitht_value;

                       // rtd_pr_vpq_emerg("rord2 weitht_value=%d \n",weitht_value);
                        drvif_color_scaler_setscaleup_directionalscaler_weight(weitht_value);
                }

        #else //control table by rtk
	UINT8 level_tmp=0;


	ret = 0;

		if (copy_from_user(&tShp_Val, (int __user *)arg, sizeof(UINT16)*7)) {
			rtd_pr_vpq_err("kernel VPQ_IOC_SET_VPQ_Shp_Val fail\n");
			ret = -1;
		} else {

			//rtd_pr_vpq_emerg("rord val[0]=%d,val[1]=%d,val[2]=%d \n",tShp_Val[0],tShp_Val[1],tShp_Val[2]);
			level_tmp =tShp_Val[1]+20;
			if(tShp_Val[1] ==0)
			level_tmp =0;

			fwif_color_set_sharpness_level(Scaler_GetSharpnessTable(),level_tmp);


                        Scaler_SetDLti(gVip_Table->VIP_QUALITY_Coef[source][VIP_QUALITY_FUNCTION_DLTi]);//fixed me

                        fwif_color_set_DLTI_level(tShp_Val[1]);

			/*fwif_color_set_sharpness_level_TV006(&tShp_Val[0]);*/
			rtd_pr_vpq_debug("kernel VPQ_IOC_SET_VPQ_Shp_Val success\n");
			ret = 0;
		}
        #endif
	}
	break;

#if 0
	case VPQ_IOC_SET_EDGE_ENHANCE:
	{
		return 0; //fixed me after demo please remove


		/*CHIP_SHARPNESS_UI_T tCHIP_SHARPNESS_UI_T;*/


		if (copy_from_user((void *)&tCHIP_EDGE_ENHANCE_UI_T,(void __user *)arg , sizeof(CHIP_EDGE_ENHANCE_UI_T))) {
			rtd_pr_vpq_err("kernel VPQ_IOC_SET_EDGE_ENHANCE fail\n");
			ret = -1;
		} else {
			memcpy(&tCurCHIP_EDGE_ENHANCE_UI_T, &tCHIP_EDGE_ENHANCE_UI_T, sizeof(CHIP_EDGE_ENHANCE_UI_T));
			/*memcpy(&tCHIP_SHARPNESS_UI_T, &tCurCHIP_SHARPNESS_UI_T, sizeof(CHIP_SHARPNESS_UI_T));*/
			/*if (uc3DCorrectionEn == false) {*/
				fwif_color_set_sharpness_table_TV006(&tCurCHIP_SHARPNESS_UI_T, &tCurCHIP_EDGE_ENHANCE_UI_T, &tCurCHIP_SR_UI_T);
			/*
				drvif_color_set_Vertical_Peaking_Enable(true);
			} else {
				fwif_color_set_SharpnessFilterCoef_for_3D((CHIP_SHARPNESS_UI_T *)&tCHIP_SHARPNESS_UI_T, (CHIP_SHARPNESS_UI_T *) &t3DCHIP_SHARPNESS_UI_T);
				fwif_color_set_sharpness_table_TV006(tCHIP_SHARPNESS_UI_T, tCHIP_EDGE_ENHANCE_UI_T, uEeVal);
				drvif_color_set_Vertical_Peaking_Enable(false);
			}
			*/
			/*rtd_pr_vpq_debug("kernel VPQ_IOC_SET_EDGE_ENHANCE success\n");*/
			ret = 0;
		}
	}
	break;
#endif
	case VPQ_IOC_SET_SR_TABLE:
	{
		RTK_SR_UI_T tRTK_SR_UI_T;

		if (copy_from_user((void *)&tRTK_SR_UI_T,(void __user *)arg , sizeof(tRTK_SR_UI_T))) {
			rtd_pr_vpq_err("kernel VPQ_IOC_SET_EDGE_ENHANCE fail\n");
			ret = -1;
		} else {
		fwif_color_access_DeJaggy_Level(tRTK_SR_UI_T.SR_LEVEL, 1);
		Scaler_setiESM(gVip_Table->VIP_QUALITY_Extend2_Coef[source][VIP_QUALITY_FUNCTION_iESM_table]);
                        #ifdef LGDB_OFF

        		drvif_color_setSR_levle_Tv006(tRTK_SR_UI_T.SR_LEVEL);
                        #else
        		memcpy(&tCurCHIP_SR_UI_T, &tRTK_SR_UI_T.sr_ui, sizeof(CHIP_SR_UI_T));
			fwif_color_set_sharpness_table_TV006(&tCurCHIP_SHARPNESS_UI_T, &tCurCHIP_SR_UI_T);

                        #endif

			ret = 0;
		}
	}
	break;

	case VPQ_IOC_SET_NR_PQA_TABLE:
		{
			unsigned char PQA_Table_Idx = 0;
			if (copy_from_user(&PQA_Table_Idx, (int __user *)arg, sizeof(unsigned char))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_SET_NR_PQA_TABLE fail\n");
				ret = -1;
			} else {
				fwif_color_set_PQA_table(PQA_Table_Idx);
				/*rtd_pr_vpq_debug("kernel VPQ_IOC_SET_NR_PQA_TABLE success\n");*/
				ret = 0;
			}
		}
	break;

	case VPQ_IOC_SET_NR_PQA_INPUT_TABLE:
		{
			unsigned char PQA_Input_Table_Idx = 0;
			if (copy_from_user(&PQA_Input_Table_Idx, (int __user *)arg, sizeof(unsigned char))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_SET_NR_PQA_INPUT_TABLE fail\n");
				ret = -1;
			} else {
				fwif_color_set_PQA_Input_table(PQA_Input_Table_Idx);
				/*rtd_pr_vpq_debug("kernel VPQ_IOC_SET_NR_PQA_INPUT_TABLE success\n");*/
				ret = 0;
			}
		}
	break;
	#if 0
	case VPQ_IOC_SET_NR_LEVEL:
		{

			if (copy_from_user(&NR_Level, (int __user *)arg, sizeof(RTK_NOISE_REDUCTION_T))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_SET_NR_LEVEL fail\n");
				ret = -1;
			} else {

                        #ifdef LGDB_OFF //control table by rtk
                                NR_Level.NR_LEVEL=4;
			        Scaler_SetDNR(NR_Level.NR_LEVEL);

                        #else
				fwif_color_Set_NR_Table_tv006(&NR_Level);


				fwif_color_SetDNR_tv006(NR_Level.NR_LEVEL);

                        #endif

				ret = 0;
			}
		}

	break;
	#endif
	case VPQ_IOC_SET_NR_INPUT:
		{
			unsigned char args = 0;
			if (copy_from_user(&args, (int __user *)arg, sizeof(unsigned char))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_SET_NR_INPUT fail\n");
				ret = -1;
			} else {
				fwif_color_Set_NR_Input_tv006(args);
				/*rtd_pr_vpq_debug("kernel VPQ_IOC_SET_NR_PQA_TABLE success\n");*/
				ret = 0;
			}

		}

	break;

	case VPQ_IOC_SET_MPEGNR_LEVEL:
		{
			if (copy_from_user(&MPEGNR_Level, (void __user *)arg, sizeof(RTK_MPEG_NOISE_REDUCTION_T))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_SET_MPEGNR_LEVEL fail\n");
				ret = -1;
			} else {

                                #ifdef LGDB_OFF  //control table by rtk
					Scaler_SetMPEGNR(MPEGNR_Level.MPEG_LEVEL, CALLED_NOT_BY_OSD); //fixed me

                                #else
				fwif_color_Set_MPEGNR_Table_tv006(&MPEGNR_Level);
				fwif_color_Set_MPEGNR_tv006(MPEGNR_Level.MPEG_LEVEL);
                                #endif
				ret = 0;
			}
		}

	break;

        case VPQ_IOC_SET_Decontour:
        {
                if (copy_from_user(&De_contour_level, (void __user *)arg, sizeof(RTK_DECONTOUR_T))) {
                        rtd_pr_vpq_err("kernel VPQ_IOC_SET_Decontour fail\n");
                        ret = -1;
                } else {

                      // rtd_pr_vpq_emerg("rord UI_Lv=%d,curve[0]=%d \n",De_contour_level.UI_Lv,De_contour_level.De_Contour_Table.Ddecontour_curve[0]);

                       // fwif_color_Set_I_De_Contour(level);
                       // fwif_color_set_LC_decontour(0,gVip_Table->VIP_QUALITY_Extend_Coef[source][VIP_QUALITY_FUNCTION_LC_Table]
                       // , level.UI_Lv);
                       tv006_decontour_level = De_contour_level.De_Contour_Table.Idecontour_level;

                      fwif_color_Set_De_Contour_tv006(&De_contour_level);
                      g_bDeContourSaved = 1;

                      ret = 0;
                }

        }

        break;


	case VPQ_IOC_GET_HistoData_Countbins:
	{
		UINT32 HistoData_bin[TV006_VPQ_chrm_bin];

		if (fwif_color_get_HistoData_Countbins(TV006_VPQ_chrm_bin, &(HistoData_bin[0])) == FALSE) {
			rtd_pr_vpq_err("kernel VPQ_IOC_GET_HistoData_Countbins fail\n");
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg, (void *)&HistoData_bin[0], sizeof(HistoData_bin))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_GET_HistoData_Countbins fail\n");
				ret = -1;
			} else
				ret = 0;
		}
	}
	break;
	case VPQ_IOC_GET_HistoData_chrm_bin:
	{
		UINT32 chrm_bin[COLOR_AutoSat_HISTOGRAM_LEVEL];

		if (fwif_color_get_HistoData_chrmbins(COLOR_AutoSat_HISTOGRAM_LEVEL, &(chrm_bin[0])) == FALSE) {
			rtd_pr_vpq_err("kernel VPQ_IOC_GET_HistoData_chrm_bin fail\n");
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg, (void *)&chrm_bin[0], sizeof(chrm_bin))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_GET_HistoData_chrm_bin fail\n");
				ret = -1;
			} else
				ret = 0;
		}
	}
	break;
	case VPQ_IOC_GET_HistoData_hue_bin:
	{
		UINT32 hue_bin[COLOR_HUE_HISTOGRAM_LEVEL];

		if (fwif_color_get_HistoData_huebins(COLOR_HUE_HISTOGRAM_LEVEL, &(hue_bin[0])) == FALSE) {
			rtd_pr_vpq_err("kernel fwif_color_get_HistoData_Countbins fail\n");
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg, (void *)&hue_bin[0], sizeof(hue_bin))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_GET_HistoData_hue_bin fail\n");
				ret = -1;
			} else
				ret = 0;
		}
	}
	break;
	case VPQ_IOC_GET_HistoData_APL:
	{
		unsigned int DC_APL = 0;

		if (fwif_color_get_HistoData_APL(&DC_APL) == FALSE) {
			rtd_pr_vpq_err("kernel fwif_color_get_HistoData_APL fail\n");
			ret = -1;
		} else {
			if (drvif_color_get_WB_pattern_on())
				DC_APL = 940;
			if (copy_to_user((void __user *)arg, (void *)&DC_APL, sizeof(unsigned int))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_GET_HistoData_APL fail\n");
				ret = -1;
			} else
				ret = 0;
		}
	}
	break;

	case VPQ_IOC_GET_HistoData_Min:
	{
		signed int DC_min = 0;

		if (fwif_color_get_HistoData_Min(&DC_min) == FALSE) {
			rtd_pr_vpq_err("kernel fwif_color_get_HistoData_Min fail\n");
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg, (void *)&DC_min, sizeof(signed int))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_GET_HistoData_Min fail\n");
				ret = -1;
			} else
				ret = 0;
		}
	}
	break;

	case VPQ_IOC_GET_HistoData_Max:
	{
		signed int DC_max = 0;

		if (fwif_color_get_HistoData_Max(&DC_max) == FALSE) {
			rtd_pr_vpq_err("kernel fwif_color_get_HistoData_Max fail\n");
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg, (void *)&DC_max, sizeof(signed int))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_GET_HistoData_Max fail\n");
				ret = -1;
			} else
				ret = 0;
		}
	}
	break;

	case VPQ_IOC_GET_HistoData_PeakLow:
	{
		signed int DC_peak_low = 0;

		if (fwif_color_get_HistoData_PeakLow(&DC_peak_low) == FALSE) {
			rtd_pr_vpq_err("kernel fwif_color_get_HistoData_PeakLow fail\n");
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg, (void *)&DC_peak_low, sizeof(signed int))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_GET_HistoData_PeakLow fail\n");
				ret = -1;
			} else
				ret = 0;
		}
	}
	break;

	case VPQ_IOC_GET_HistoData_PeakHigh:
	{
		signed int DC_peak_high;

		if (fwif_color_get_HistoData_PeakHigh(&DC_peak_high) == FALSE) {
			rtd_pr_vpq_err("kernel fwif_color_get_HistoData_PeakHigh fail\n");
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg, (void *)&DC_peak_high, sizeof(signed int))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_GET_HistoData_PeakHigh fail\n");
				ret = -1;
			} else
				ret = 0;
		}
	}
	break;

	case VPQ_IOC_GET_HistoData_skin_count:
	{
		unsigned int skin_count;

		if (fwif_color_get_HistoData_skinCount(&skin_count) == FALSE) {
			rtd_pr_vpq_err("kernel fwif_color_get_HistoData_skinCount fail\n");
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg, (void *)&skin_count, sizeof(unsigned int))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_GET_HistoData_skin_count fail\n");
				ret = -1;
			} else
				ret = 0;
		}
	}
	break;

	case VPQ_IOC_GET_HistoData_sat_status:
	{
		unsigned int sat_status;

		if (fwif_color_get_HistoData_SatStatus(&sat_status) == FALSE) {
			rtd_pr_vpq_err("kernel fwif_color_get_HistoData_SatStatus fail\n");
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg, (void *)&sat_status, sizeof(unsigned int))) {
				rtd_pr_vpq_err("kernel fwif_color_get_HistoData_SatStatus fail\n");
				ret = -1;
			} else
				ret = 0;
		}
	}
	break;

	case VPQ_IOC_GET_HistoData_diff_sum:
	{
		unsigned int diff_sum;

		/*rtd_pr_vpq_debug("kernel VPQ_IOC_GET_HistoData_diff_sum\n");*/
		if (fwif_color_get_HistoData_diffSum(&diff_sum) == FALSE) {
			rtd_pr_vpq_err("kernel fwif_color_get_HistoData_diffSum fail\n");
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg, (void *)&diff_sum, sizeof(unsigned int))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_GET_HistoData_diff_sum fail\n");
				ret = -1;
			} else
				ret = 0;
		}
	}
	break;
	case VPQ_IOC_GET_HistoData_motion:
	{
		unsigned int motion;

		/*rtd_pr_vpq_debug("kernel VPQ_IOC_GET_HistoData_motion\n");*/
		if (fwif_color_get_HistoData_motion(&motion) == FALSE) {
			rtd_pr_vpq_err("kernel VPQ_IOC_GET_HistoData_motion fail\n");
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg, (void *)&motion, sizeof(unsigned int))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_GET_HistoData_motion fail\n");
				ret = -1;
			} else
				ret = 0;
		}
	}
	break;

	case VPQ_IOC_GET_HistoData_texture:
	{
		unsigned int texture;

		/*rtd_pr_vpq_debug("kernel VPQ_IOC_GET_HistoData_texture\n");*/
		if (fwif_color_get_HistoData_texture(&texture) == FALSE) {
			rtd_pr_vpq_err("kernel fwif_color_get_HistoData_texture fail\n");
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg, (void *)&texture, sizeof(unsigned int))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_GET_HistoData_texture fail\n");
				ret = -1;
			} else
				ret = 0;
		}
	}
	break;
	case VPQ_IOC_SET_FreshContrast:
	{
		if (fwif_color_vpq_stop_ioctl(STOP_VPQ_IOC_SET_FreshContrast, ACCESS_MODE_GET, 0))
			return 0;

		if (copy_from_user(&tFreshContrast_coef, (void __user *)arg, sizeof(CHIP_DCC_T))) {
			rtd_pr_vpq_err("kernel VPQ_IOC_SET_FreshContrast fail\n");
			ret = -1;
		} else {

			if (fwif_color_set_dcc_FreshContrast_tv006(&tFreshContrast_coef) == FALSE) {
				rtd_pr_vpq_err("kernel fwif_color_set_dcc_FreshContrast_tv006 fail\n");
				ret = -1;
			} else {
				/*rtd_pr_vpq_debug("5.kernel VPQ_IOC_SET_FreshContrast success\n");*/
				ret = 0;
			}
		}
	}
	break;

	case VPQ_IOC_SET_FreshContrastLUT:
	{
		if (fwif_color_vpq_stop_ioctl(STOP_VPQ_IOC_SET_FreshContrastLUT, ACCESS_MODE_GET, 0))
			return 0;

		if (copy_from_user(&tFreshContrastLUT, (int __user *)arg, sizeof(FreshContrastLUT_T))) {
			rtd_pr_vpq_err("kernel VPQ_IOC_SET_FreshContrastLUT fail\n");
			ret = -1;
		} else {
			if (drvif_color_get_WB_pattern_on()) {
				if (!fwif_color_set_bypass_dcc_Curve_Write_tv006()) {
					rtd_pr_vpq_err("kernel fwif_color_set_dcc_Curve_Write_tv006 fail\n");
						ret = -1;
				} else if (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE) {
					if (!g_prevent_force_write_dcc)
						fwif_color_set_dcc_force_write_flag_RPC(0, 1);
					g_prevent_force_write_dcc = 1;
					return 0;
				}
			} else if (fwif_color_set_dcc_Curve_Write_tv006(&tFreshContrastLUT.FreshContrastLUT[0]) == FALSE) {
				rtd_pr_vpq_err("kernel fwif_color_set_dcc_Curve_Write_tv006 fail\n");
				ret = -1;
			} else
				ret = 0;

			g_prevent_force_write_dcc = 0;
		}
	}
	break;

	case VPQ_IOC_GET_FreshContrastBypassLUT:
	{
		FreshContrastLUT_T tFreshContrastLUT;

		if (fwif_color_get_FreshContrastBypassLUT(DCC_Curve_Node_MAX, &(tFreshContrastLUT.FreshContrastLUT[0])) == FALSE) {
			rtd_pr_vpq_err("fwif_color_get_FreshContrastBypassLUT fail\n");
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg, (void *)&(tFreshContrastLUT.FreshContrastLUT[0]), sizeof(FreshContrastLUT_T))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_GET_FreshContrastBypassLUT copy_to_user fail\n");
				ret = -1;
			} else
				ret = 0;
		}
	}
	break;

	case VPQ_IOC_SET_ColorGain:
	{
		RTK_COLOR_GAIN_T colorGain;

		if (fwif_color_vpq_stop_ioctl(STOP_VPQ_IOC_SET_SAT_LUT, ACCESS_MODE_GET, 0))
			return 0;

		if (copy_from_user(&colorGain, (void __user *)arg, sizeof(RTK_COLOR_GAIN_T))) {
					rtd_pr_vpq_err("kernel VPQ_IOC_SET_ColorGain fail\n");
					ret = -1;
		} else {
			fwif_color_access_apl_color_gain_tv006(colorGain.wId, &colorGain.uCgVal, ACCESS_MODE_WRITE);
			if (g_bAPLColorGainClr) {
				fwif_color_update_YUV2RGB_CSC_tv006(colorGain.wId);
				g_bAPLColorGainClr = 0;
			} else
				g_bAPLColorGainClr = 1;
		}
	}
	break;

	case VPQ_IOC_SET_CLEAR_WHITE:
	{
		if (copy_from_user(&clear_white, (void __user *)arg, sizeof(HAL_VPQ_CLEAR_WHITE))) {
			rtd_pr_vpq_err("kernel VPQ_IOC_SET_CLEAR_WHITE_GAIN fail\n");
			ret = -1;
		} else {
			memcpy(&(uvOffset.Uoffset[0]), &(clear_white.Uoffset[0]), sizeof(unsigned char)*VIP_YUV2RGB_Y_Seg_Max);
			memcpy(&(uvOffset.Voffset[0]), &(clear_white.Voffset[0]), sizeof(unsigned char)*VIP_YUV2RGB_Y_Seg_Max);
			memcpy(&(uvOffset.UV_index[0]), &(clear_white.UV_offset_index[0]), sizeof(unsigned char)*(VIP_YUV2RGB_Y_Seg_Max - 1));
			memcpy(&(uv_offset_byuv_curve.gainCurve_index[0]), &(clear_white.gain_by_UV_index[0]), sizeof(unsigned short)*(VIP_YUV2RGB_Y_Seg_Max - 1));
			memcpy(&(uv_offset_byuv_curve.gain[0]), &(clear_white.gain_by_UV[0]), sizeof(unsigned short)*VIP_YUV2RGB_Y_Seg_Max);

			/*debug*/
			/*
			{
			UINT8 i;
			rtd_pr_vpq_info("[CLEAR_WHITE], wId = %d, enable = %d, mode = %d\n", clear_white.wId, clear_white.enable, clear_white.mode);
			rtd_pr_vpq_info("[CLEAR_WHITE], Uoffset =");
			for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++)
			printk(" %d,", clear_white.Uoffset[i]);
			rtd_pr_vpq_info("\n[CLEAR_WHITE], Voffset =");
			for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++)
			printk(" %d,", clear_white.Voffset[i]);
			rtd_pr_vpq_info("\n[CLEAR_WHITE], UV_offset_index =");
			for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max - 1; i++)
			printk(" %d,", clear_white.UV_offset_index[i]);
			rtd_pr_vpq_info("\n[CLEAR_WHITE], gain_by_UV_index =");
			for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max - 1; i++)
			printk(" %d,", clear_white.gain_by_UV_index[i]);
			rtd_pr_vpq_info("\n[CLEAR_WHITE], gain_by_UV =");
			for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++)
			printk(" %d,", clear_white.gain_by_UV[i]);
			}
			*/

			if (!fwif_color_set_YUV2RGB_UV_Offset_tv006(clear_white.wId, clear_white.enable, clear_white.mode, &uvOffset, &uv_offset_byuv_curve))
				return -1;
			/*rtd_pr_vpq_debug("kernel VPQ_IOC_SET_CLEAR_WHITE_GAIN,%d,%d, ret =%d\n", level, display, ret);*/
		}
	}
	break;

	case VPQ_IOC_SET_BLUE_STRETCH:
	{

		#if 0 ////k5l hw remove
		RTK_BLUE_STRETCH_T blue_stretch;

		if (copy_from_user(&blue_stretch, (void __user *)arg, sizeof(RTK_BLUE_STRETCH_T))) {
			rtd_pr_vpq_err("kernel VPQ_IOC_SET_BLUE_STRETCH fail\n");
			ret = -1;
		} else {
			drvif_color_set_Blue_Stretch(&blue_stretch.gamma_bs);
		}
		#endif
	}
	break;
	case VPQ_IOC_SET_LOCALCONTRAST_PARAM:
		{

			if (copy_from_user(&glc_param, (void __user *)arg, sizeof(CHIP_LOCAL_CONTRAST_T))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_SET_LOCALCONTRAST_PARAM fail\n");
				ret = -1;
			} else {
				fwif_color_set_LocalContrast_table_TV006(&glc_param);

				ret = 0;
			}
		}
	break;

	case VPQ_IOC_SET_INIT_COLOR_ADV:
	{
		if (COLOR_MODE_ADV != g_Color_Mode)
			g_cm_need_refresh = 1;

		if (copy_from_user(&cmRegionExt, (void __user *)arg, sizeof(CHIP_CM_REGION_EXT_T))) {
			rtd_pr_vpq_err("kernel VPQ_IOC_SET_INIT_COLOR_ADV fail\n");
			ret = -1;
		} else {
			int i;

			if (cmRegionExt.ctrl[0] != 4) //verion4, add keepY
				return -1;

			// check param
			for (i = 0; i < CHIP_CM_COLOR_REGION_MAX; i++) {
				if (cmRegionExt.cm_rgn.stColorRegionType.stColorRegion[i].regionNum >= CHIP_CM_COLOR_REGION_MAX) {
					rtd_pr_vpq_warn("[VPQ][kernel] %d-regionNum(%d) > CHIP_CM_COLOR_REGION_MAX, return!!\n",
						i, cmRegionExt.cm_rgn.stColorRegionType.stColorRegion[i].regionNum);
					return -1;
				}
			}

			memcpy(&g_cm_rgn_adv, &cmRegionExt.cm_rgn, sizeof(CHIP_CM_REGION_T));
			g_cm_keep_Y = g_cm_rgn_adv.stColorRegionType.keepY;
			vpq_ICM_handler(VPQ_IOC_SET_INIT_COLOR_ADV);
			g_cm_need_refresh = 1;
			g_flag_cm_adv_init_ok = 1;
		}
	}
	break;

	case VPQ_IOC_SET_INIT_COLOR_EXP:
	{
		if (COLOR_MODE_EXP != g_Color_Mode)
			g_cm_need_refresh = 1;

		if (copy_from_user(&cmRegionExt, (void __user *)arg, sizeof(CHIP_CM_REGION_EXT_T))) {
			rtd_pr_vpq_err("kernel VPQ_IOC_SET_INIT_COLOR_EXP fail\n");
			ret = -1;
		} else {
			int i;

			if (cmRegionExt.ctrl[0] != 4) //verion4, add keepY
				return -1;

			// check param
			for (i = 0; i < CHIP_CM_COLOR_REGION_MAX; i++) {
				if (cmRegionExt.cm_rgn.stColorRegionType.stColorRegion[i].regionNum >= CHIP_CM_COLOR_REGION_MAX) {
					rtd_pr_vpq_warn("[VPQ][kernel] %d-regionNum(%d) > CHIP_CM_COLOR_REGION_MAX, return!!\n",
						i, cmRegionExt.cm_rgn.stColorRegionType.stColorRegion[i].regionNum);
					return -1;
				}
			}

			memcpy(&g_cm_rgn_exp, &cmRegionExt.cm_rgn, sizeof(CHIP_CM_REGION_T));
			g_cm_keep_Y = g_cm_rgn_exp.stColorRegionType.keepY;
			vpq_ICM_handler(VPQ_IOC_SET_INIT_COLOR_EXP);
			g_cm_need_refresh = 1;
			g_flag_cm_exp_init_ok = 1;
		}
	}
	break;

	case VPQ_IOC_SET_FRESH_COLOR:
	{
		CHIP_COLOR_CONTROL_FRESH_T cmControl;
		int ret_compare;

		if (copy_from_user(&cmControl, (void __user *)arg, sizeof(CHIP_COLOR_CONTROL_FRESH_T))) {
			rtd_pr_vpq_err("kernel VPQ_IOC_SET_FRESH_COLOR fail\n");
			return -1;
		}

		if (cmRegionExt.ctrl[0] != 4) {//verion4, add keepY
			return -1;
		}

		//if (COLOR_MODE_ADV == g_Color_Mode) {
			if (g_flag_cm_adv_init_ok) {
				ret_compare = memcmp(&g_cm_ctrl_adv, &cmControl.cm_ctrl, sizeof(CHIP_COLOR_CONTROL_T));
				if (ret_compare == 0 && !g_cm_need_refresh) {
					return 0;
				}
				memcpy(&g_cm_ctrl_adv, &cmControl.cm_ctrl, sizeof(CHIP_COLOR_CONTROL_T));
				vpq_ICM_handler(VPQ_IOC_SET_FRESH_COLOR);
				g_cm_need_refresh = 0;
			}
		/*
		} else {
			if (g_flag_cm_exp_init_ok) {
				ret_compare = memcmp(&g_cm_ctrl_exp, &cmControl.cm_ctrl, sizeof(CHIP_COLOR_CONTROL_T));
				if (ret_compare == 0 && !g_cm_need_refresh) {
					return 0;
				}
				memcpy(&g_cm_ctrl_exp, &cmControl.cm_ctrl, sizeof(CHIP_COLOR_CONTROL_T));
				fwif_color_calc_ICM_gain_table_driver_tv006(&g_cm_rgn_exp.stColorRegionType, &g_cm_ctrl_exp, &icm_tab_elem_of_vip_table, &icm_tab_elem_write);
				vpq_update_icm(TRUE);
				g_cm_need_refresh = 0;
			}
		}
		*/
	}
	break;

	case VPQ_IOC_SET_PREFERRED_COLOR:
	{
		CHIP_COLOR_CONTROL_ADV_EXT_T cmControlExt;

		if (copy_from_user(&cmControlExt, (void __user *)arg, sizeof(CHIP_COLOR_CONTROL_ADV_EXT_T))) {
			rtd_pr_vpq_err("kernel VPQ_IOC_SET_PREFERRED_COLOR fail\n");
			ret = -1;
		} else {
			if (cmRegionExt.ctrl[0] != 4) //verion4, add keepY
				return -1;
			if (g_flag_cm_adv_init_ok) {
				int ret_compare;
				ret_compare = memcmp(&g_cm_ctrl_adv, &cmControlExt.cm_ctrl, sizeof(CHIP_COLOR_CONTROL_T));
				if (ret_compare == 0 && !g_cm_need_refresh && g_Color_Mode == COLOR_MODE_ADV) {
					return 0;
				}
				memcpy(&g_cm_ctrl_adv, &cmControlExt.cm_ctrl, sizeof(CHIP_COLOR_CONTROL_T));
				g_Color_Mode = COLOR_MODE_ADV;
				vpq_ICM_handler(VPQ_IOC_SET_PREFERRED_COLOR);
				g_cm_need_refresh = 0;
			} else {
				rtd_pr_vpq_warn("[VPQ][kernel] need call HAL_VPQ_InitColorForAdvanced first!\n");
				ret = -1;
			}
		}
	}
	break;

	case VPQ_IOC_SET_COLOR_MANAGEMENT:
	{
		CHIP_COLOR_CONTROL_EXP_EXT_T cmControlExt;

		if (copy_from_user(&cmControlExt, (void __user *)arg, sizeof(CHIP_COLOR_CONTROL_EXP_EXT_T))) {
			rtd_pr_vpq_err("kernel VPQ_IOC_SET_COLOR_MANAGEMENT fail\n");
			ret = -1;
		} else {
			if (cmRegionExt.ctrl[0] != 4) //verion4, add keepY
				return -1;
			if (g_flag_cm_exp_init_ok) {
				int ret_compare;
				ret_compare = memcmp(&g_cm_ctrl_exp, &cmControlExt.cm_ctrl, sizeof(CHIP_COLOR_CONTROL_T));
				if (ret_compare == 0 && !g_cm_need_refresh && g_Color_Mode == COLOR_MODE_EXP) {
					return 0;
				}
				memcpy(&g_cm_ctrl_exp, &cmControlExt.cm_ctrl, sizeof(CHIP_COLOR_CONTROL_T));
				g_Color_Mode = COLOR_MODE_EXP;
				vpq_ICM_handler(VPQ_IOC_SET_COLOR_MANAGEMENT);
				g_cm_need_refresh = 0;
			} else {
				rtd_pr_vpq_warn("[VPQ][kernel] need call HAL_VPQ_InitColorForExpert first!\n");
				ret = -1;
			}
		}
	}
	break;

	case VPQ_IOC_SET_ICM_ELEM:
	{
		if (copy_from_user(&icm_tab_elem_write, (int __user *)arg, sizeof(COLORELEM_TAB_T))) {
			rtd_pr_vpq_err("kernel VPQ_IOC_SET_ICM_ELEM fail\n");
			ret = -1;
		} else {
			vpq_ICM_handler(VPQ_IOC_SET_ICM_ELEM);
		}
	}
	break;
#if 0
	case VPQ_IOC_SET_3D_COLOR_CORRECTION:
		{
			unsigned char c3DMode;
			unsigned int args;
			/*
			CHIP_SHARPNESS_UI_T tCHIP_SHARPNESS_UI_T;
			CHIP_EDGE_ENHANCE_UI_T tCHIP_EDGE_ENHANCE_UI_T;
			*/

			//static BOOLEAN lowdelaymode;
			if (copy_from_user(&args, (int __user *)arg, sizeof(unsigned int))) {
				rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_PIC_CTRL fail\n");
				ret = -1;
			} else {
				uc3DCorrectionEn = args&0xff;
				c3DMode = (args>>8)&0xff;

				if (HAL_VPQ_3D_MODE_3D_FS == c3DMode)
					bForceHist3DCtrl = true;
				else
					bForceHist3DCtrl = false;

				fwif_color_set_hist_3d_en(bForceHist3DCtrl);
				ret = 0;
			}

			/*
			memcpy(&tCHIP_SHARPNESS_UI_T, &tCurCHIP_SHARPNESS_UI_T, sizeof(CHIP_SHARPNESS_UI_T));
			memcpy(&tCHIP_EDGE_ENHANCE_UI_T, &tCurCHIP_EDGE_ENHANCE_UI_T, sizeof(CHIP_EDGE_ENHANCE_UI_T));
			if (uc3DCorrectionEn == false) {
				fwif_color_set_sharpness_table_TV006(tCHIP_SHARPNESS_UI_T, tCHIP_EDGE_ENHANCE_UI_T, uEeVal);
				drvif_color_set_Vertical_Peaking_Enable(true);
			} else {
				fwif_color_set_SharpnessFilterCoef_for_3D((CHIP_SHARPNESS_UI_T *)&tCHIP_SHARPNESS_UI_T, (CHIP_SHARPNESS_UI_T *) &t3DCHIP_SHARPNESS_UI_T);
				fwif_color_set_sharpness_table_TV006(tCHIP_SHARPNESS_UI_T, tCHIP_EDGE_ENHANCE_UI_T, uEeVal);
				drvif_color_set_Vertical_Peaking_Enable(false);
			}
			*/
			/*ToDo :: add sharpness v peaking and 2d coeff table*/
			ret = 0;
		}
		break;
#endif
	case VPQ_IOC_SET_PIC_CTRL:
	{
		/*unsigned char display = 0;*/
		/*rtd_pr_vpq_debug("~~kernel VPQ_IOC_SET_CLEAR_WHITE_GAIN~~\n");*/

		PictureMode_flg = 1;  // for MEMC wrt by JerryWang 20161125
		if (copy_from_user(&pic_ctrl, (int __user *)arg, sizeof(VPQ_SetPicCtrl_T))) {
			rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_PIC_CTRL fail\n");
			ret = -1;
		} else {
			ret = fwif_color_set_Picture_Control_tv006(pic_ctrl.wId, &pic_ctrl);
		}
		break;
	}
	case VPQ_IOC_SET_BLACK_LEVEL:
	{

		//rtd_pr_vpq_info("\n HDR_MODE, VPQ_IOC_SET_BLACK_LEVEL ");

		if (copy_from_user(&blackLv, (int __user *)arg, sizeof(HAL_VPQ_SET_BLACK_LEVEL))) {
			rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_BLACK_LEVEL fail\n");
			ret = -1;
		} else {

			input_info = blackLv.uBlVal[SET_BLACK_LEVEL_ITEM_Input_Info];



			switch (blackLv.uBlVal[SET_BLACK_LEVEL_ITEM_Level])
			{
			case UI_LEVEL_LOW:
				level = RGB2YUV_BLACK_LEVEL_LOW;
				break;
			case UI_LEVEL_HIGH:
				level = RGB2YUV_BLACK_LEVEL_HIGH;
				break;
			case UI_LEVEL_AUTO:
				level = RGB2YUV_BLACK_LEVEL_AUTO;
				break;
			default:
				level = RGB2YUV_BLACK_LEVEL_AUTO;
			}

			//20161111 roger, when sub record, don't change VD con/bri, WOSQRTK-9730
			if ((input_info == HAL_VPQ_INPUT_ATV || input_info == HAL_VPQ_INPUT_AV) && (get_sub_OutputVencMode() == 0)) {
				if (fwif_color_get_VDBlackLevel_MappingValue(input_info, level-1, &mCon, &mBri, &mHue, &mSat) == 0)
				{
					fwif_color_set_VDBlackLevel(mCon, mBri, mHue, mSat);

				}
			}

			/*20151128 roger, WOSQRTK-3568, LG want input_info == HAL_VPQ_INPUT_MEDIA_MOVIE, and level == RGB2YUV_BLACK_LEVEL_AUTO then level = RGB2YUV_BLACK_LEVEL_LOW directly*/
			if (input_info == HAL_VPQ_INPUT_MEDIA_MOVIE)
			{
				if (level == RGB2YUV_BLACK_LEVEL_AUTO)
					level = RGB2YUV_BLACK_LEVEL_LOW;
			}

			/* compare with previous, change black lv from UI while comparation is differnet*/
			if (level == fwif_color_get_rgb2yuv_black_level_tv006())
				isBlack_LvDiff_Flag = 0;
			else
				isBlack_LvDiff_Flag = 1;

			fwif_color_set_rgb2yuv_black_level_tv006(level);

#ifndef CONFIG_HDR_SDR_SEAMLESS	/* seamless need to check force bg and mode active*/
			Scaler_SetDataFormatHandler();
#else
			if (get_support_vo_force_v_top(support_sdr_max_rgb)) {
				Scaler_Set_HDR_YUV2RGB(PQModeInfo_flag[0], PQModeInfo_flag[1]);
			} else {
				if ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE) && (Check_AP_Set_Enable_ForceBG(SLR_MAIN_DISPLAY) == FALSE))
				{
					Scaler_Set_HDR_YUV2RGB(PQModeInfo_flag[0], PQModeInfo_flag[1]);
					if (isBlack_LvDiff_Flag == 1) {
						Scaler_SetDataFormatHandler();
					} else {
						/* seamless. do seamless process in isr (void Scaler_hdr_setting_SEAMLESS()), avoid transient noise*/
						;
					}
				} else {
					Scaler_Set_HDR_YUV2RGB(PQModeInfo_flag[0], PQModeInfo_flag[1]);
					Scaler_SetDataFormatHandler();
				}
			}
			HDR_SDR_SEAMLESS_PQ.HDR_SDR_SEAMLESS_PQ_SET[HDR_SDR_SEAMLESS_PQ_HAL_PQModeInfo]=1;
#endif
			ret = 0;
		}
		break;
	}

	case VPQ_IOC_SET_HDR_GAMUTMATRIX:
	{
		//rtd_pr_vpq_info("\n HDR_MODE, VPQ_IOC_SET_HDR_GAMUTMATRIX PQModeInfo_flag:%d",PQModeInfo_flag[0]);
		//if (PQModeInfo_flag[0] != HAL_VPQ_HDR_MODE_HDR10) break;	/* seamless will only change PQModeInfo_flag_seamless, this condition is not correct*/
		if (copy_from_user(&gamut33, (int __user *)arg, sizeof(HDR_SetGamut33Matrix_T))) {
			rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_HDR_GAMUTMATRIX fail\n");
			ret = -1;
		} else {
			//drvif_HDR_RGB2OPT(1, gamut33.Gamut33Matrix);
			fwif_HDR_RGB2OPT(1, gamut33.Gamut33Matrix);
		}
		break;
	}

	case VPQ_IOC_SET_HDR_3DLUT_16: //new
	{
		//rtd_pr_vpq_info("\n HDR_MODE, VPQ_IOC_SET_HDR_3DLUT_16");
		//20151112 roger, fail ==> try again, I can't get the batter way to fix this copy fail issue
		data_protect_cfu = 100;

		while(copy_from_user(&phdr3dlut16, (void __user *)arg, sizeof(short) * VIP_DM_HDR_3D_LUT_UI_TBL_SIZE) && (data_protect_cfu > 0))
		{
			msleep(10);
			data_protect_cfu--;
		}

		if (data_protect_cfu == 0)
		{
			rtd_pr_vpq_err("HDR10, kernel VPQ_IOC_SET_GAMUT_3D_LUT_N17_BUFFER fail\n");
			ret = -1;
		}
		else
		{
			//static struct task_struct *p_pq_tsk_HDR_3dLUT_16 = NULL;

			static PQ_HDR_3dLUT_16_PARAM args;
			args.c3dLUT_16_enable = 1;
			args.a3dLUT_16_pArray = (UINT16*)(void __user *)&(phdr3dlut16.hdr_3dlut);

			//UINT32 rc = 0;
			//for (rc = 0; rc < 10; rc++)
			//	printk("3dlut[%d]= %d, 3dlut[%d]= %d, 3dlut[%d]= %d\n", 3*rc, phdr3dlut16.hdr_3dlut[3*rc], 3*rc + 1, phdr3dlut16.hdr_3dlut[3*rc + 1], 3*rc + 2, phdr3dlut16.hdr_3dlut[3*rc + 2]);

			#if 0
			p_pq_tsk_HDR_3dLUT_16 = kthread_create(fwif_color_set_DM_HDR_3dLUT_16, (void *)&args, "fwif_color_set_DM_HDR_3dLUT_16");
			if (p_pq_tsk_HDR_3dLUT_16) {
				wake_up_process(p_pq_tsk_HDR_3dLUT_16);
			}
			#else

			drvif_Set_DM_HDR_CLK();	// enable DM & Composer clock
			fwif_color_set_DM_HDR_3dLUT_17x17x17_16_TV006(&args, 1);
			#endif
		}
		break;

	}

	case VPQ_IOC_SET_HDR_3DLUT_24x24x24_16_1:
	{
		//rtd_pr_vpq_info("\n HDR_MODE, VPQ_IOC_SET_HDR_3DLUT_24x24x24_16_1\n");
		data_protect_cfu = 100;

		while(copy_from_user(&phdr3dlut16, (void __user *)arg, (sizeof(short) * ((HDR_24x24x24_size*3)>>1))) && (data_protect_cfu > 0))
		{
			msleep(10);
			data_protect_cfu--;
		}

		if (data_protect_cfu == 0)
		{
			rtd_pr_vpq_err("HDR10, CSC3 table copy fail\n");
			ret = -1;
		}
		break;

	}

	case VPQ_IOC_SET_HDR_3DLUT_24x24x24_16_2:
	{
		//rtd_pr_vpq_info("\n HDR_MODE, VPQ_IOC_SET_HDR_3DLUT_24x24x24_16_2");
		data_protect_cfu = 100;
		while(copy_from_user(((UINT16*)(void __user *)&(phdr3dlut16.hdr_3dlut)) + ((HDR_24x24x24_size*3)>>1), (void __user *)arg, (sizeof(short) * ((HDR_24x24x24_size*3)>>1))) && (data_protect_cfu > 0))
		{
			msleep(10);
			data_protect_cfu--;
		}

		if (data_protect_cfu == 0)
		{
			rtd_pr_vpq_err("HDR10, CSC3 table copy fail 2\n");
			ret = -1;
		}
		else
		{
			static PQ_HDR_3dLUT_16_PARAM args;
			args.c3dLUT_16_enable = 1;
			args.a3dLUT_16_pArray = (UINT16*)(void __user *)&(phdr3dlut16.hdr_3dlut);
			drvif_Set_DM_HDR_CLK();	// enable DM & Composer clock
			fwif_color_set_DM_HDR_3dLUT_24x24x24_16_TV006(&args, 1);
		}

		break;
	}
	case VPQ_IOC_SET_HDR_Eotf:
	{
		//rtd_pr_vpq_info("\n HDR_MODE, VPQ_IOC_SET_HDR_Eotf");
		if (copy_from_user(&Eotf_data, (int __user *)arg, sizeof(HAL_VPQ_DATA_T))) {
			rtd_pr_vpq_err("HDR10, Eotf struct copy fail\n");
			ret = -1;
			break;
		}

		data_protect_cfu = 100;
		while(copy_from_user(Eotf_table32, (int __user *)Eotf_data.pData, EOTF_size * 4) && data_protect_cfu)
		{
			msleep(10);
			data_protect_cfu--;
		}

		if (data_protect_cfu == 0)
		{
			rtd_pr_vpq_err("HDR10, Eotf table copy fail\n");
			ret = -1;
		}
		else
		{
			Eotf_table32[EOTF_size - 1] = 0xffffffff;
			//UINT16 i;
			//for (i = 0; i < EOTF_size; i++)
				//printk("roger, Eotf[%d] = %d\n", i, Eotf_table32[i]);

			fwif_color_set_DM2_EOTF_TV006(Eotf_table32, 1); //SEAMLESS mode block it
		}
		break;
	}
	case VPQ_IOC_SET_HDR_Oetf:
	{
		//rtd_pr_vpq_info("\n HDR_MODE, VPQ_IOC_SET_HDR_Oetf ");
		if (copy_from_user(&Oetf_data, (int __user *)arg, sizeof(HAL_VPQ_DATA_T))) {
			rtd_pr_vpq_err("HDR10, Oetf struct copy fail\n");
			ret = -1;
			break;
		}

		data_protect_cfu = 100;
		while(copy_from_user(Oetf_table16, (int __user *)Oetf_data.pData, OETF_size * 2) && data_protect_cfu)
		{
			msleep(10);
			data_protect_cfu--;
		}

		if (data_protect_cfu == 0)
		{
			rtd_pr_vpq_err("HDR10, Oetf table copy fail\n");
			ret = -1;
		}
		else
		{
			Oetf_table16[OETF_size - 1] = 0xffff;
			//UINT16 i;
			//for (i = 0; i < OETF_size; i++)
				//printk("roger, Oetf[%d] = %d\n", i, Oetf_table16[i]);

			fwif_color_set_DM2_OETF_TV006(Oetf_table16, 1);
		}
		break;
	}

	case VPQ_IOC_SET_PQModeInfo:
	{
		//rtd_pr_vpq_info("HDR main contorl, SET_PQModeInfo\n");
		if (copy_from_user(&PQModeInfo_data, (int __user *)arg, sizeof(HAL_VPQ_DATA_T))) {
			rtd_pr_vpq_err("HDR main contorl, PQModeInfo struct copy fail\n");
			ret = -1;
			break;
		}

		if(copy_from_user(PQModeInfo_flag, (int __user *)PQModeInfo_data.pData, PQModeInfo_data.length))
		{
			rtd_pr_vpq_err("HDR main contorl, PQModeInfo table copy fail\n");
			ret = -1;
		}
		else
		{

			pr_info("[HDR new flow][VPQ HAL], type = %d\n", PQModeInfo_flag[0]);
			if (get_support_vo_force_v_top(support_sdr_max_rgb) && (PQModeInfo_flag[0] == HAL_VPQ_HDR_MODE_SDR))
				PQModeInfo_flag[0] = HAL_VPQ_HDR_MODE_SDR_MAX_RGB;
#if 0
			if (PQModeInfo_flag[0] == HAL_VPQ_HDR_MODE_HDR10)
				rtd_pr_vpq_info("\n HDR_MODE, PQModeInfo_flag :HAL_VPQ_HDR_MODE_HDR10 \n");
			else if (PQModeInfo_flag[0] == HAL_VPQ_HDR_MODE_HLG)
				rtd_pr_vpq_info("\nHDR_MODE, PQModeInfo_flag :HAL_VPQ_HDR_MODE_HLG \n");
			else
				rtd_pr_vpq_info("\n HDR_MODE, PQModeInfo_flag :HAL_VPQ_HDR_MODE_SDR \n");
#endif
			if (PQModeInfo_flag[1] == 0) PQModeInfo_flag[1] = 2;	//data protect
			g_LGE_HDR_CSC_CTRL = PQModeInfo_flag[1];


#ifndef CONFIG_HDR_SDR_SEAMLESS	/* seamless need to check force bg and mode active*/
			drvif_TV006_SET_HDR_mode(PQModeInfo_flag);
			Scaler_SetDataFormatHandler();
#else
			if (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)
			{
				rtd_pr_vpq_info("\nHDR_MODE, PQModeInfo: seamless flow\n");
				/* seamless. do seamless process in isr (void Scaler_hdr_setting_SEAMLESS()), avoid transient noise*/;
				Scaler_Set_HDR_YUV2RGB(PQModeInfo_flag[0], PQModeInfo_flag[1]);
			} else {
				rtd_pr_vpq_info("\nHDR_MODE, PQModeInfo: scaler flow\n");
				Scaler_Set_HDR_YUV2RGB(PQModeInfo_flag[0], PQModeInfo_flag[1]);
				//drvif_TV006_SET_HDR_mode(PQModeInfo_flag);
				//Scaler_SetDataFormatHandler();
			}
			HDR_SDR_SEAMLESS_PQ.HDR_SDR_SEAMLESS_PQ_SET[HDR_SDR_SEAMLESS_PQ_HAL_PQModeInfo]=1;
#endif
		}
		break;
	}

	case VPQ_IOC_GET_HDR_PicInfo:
	{
		//get histogram protect
		if (fwif_color_get_DM_HDR10_enable_TV006() == 0) {
			//rtd_pr_vpq_notice("kernel fwif_color_get_DM2_HDR_histogram_TV006 should not run in SDR content\n");
			ret = -1;
			break;
		}

		if (fwif_color_get_DM2_HDR_histogram_TV006(HDR_info_bin) == 0) {
			rtd_pr_vpq_err("kernel fwif_color_get_DM2_HDR_histogram_TV006 fail\n");
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg,(void *) HDR_info_bin, 131 * 4)) {
				rtd_pr_vpq_err("kernel VPQ_IOC_GET_HDR_PicInfo fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;
	}

	case VPQ_IOC_SET_HDR_InvGamma:
	{
		//rtd_pr_vpq_info("HDR10, SET_HDR_InvGamma\n");
		if (copy_from_user(&HDR_InvGamma_en, (int __user *)arg, 1)) {
			rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_HDR_InvGamma fail\n");
			ret = -1;
		} else {
#ifndef CONFIG_HDR_SDR_SEAMLESS	/* seamless need to check force bg and mode active*/
			drvif_DM2_GAMMA_Enable(HDR_InvGamma_en);
#else
			if ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE) && (Check_AP_Set_Enable_ForceBG(SLR_MAIN_DISPLAY) == FALSE))
			{
				//g_InvGamma = HDR_InvGamma_en;
				fwif_DM2_GAMMA_Enable(HDR_InvGamma_en);
				rtd_pr_vpq_info("\n HDR_MODE,SEAMLESS_FLOW SET_HDR_InvGamma:%d \n",HDR_InvGamma_en);
				/* seamless. do seamless process in isr (void Scaler_hdr_setting_SEAMLESS()), avoid transient noise*/;
			} else {

				rtd_pr_vpq_info("\n HDR_MODE, SET_HDR_InvGamma :%d \n ",HDR_InvGamma_en);

				//drvif_DM2_GAMMA_Enable(HDR_InvGamma_en);
				fwif_DM2_GAMMA_Enable(HDR_InvGamma_en);
			}
			//HDR_SDR_SEAMLESS_PQ.HDR_SDR_SEAMLESS_PQ_SET[HDR_SDR_SEAMLESS_PQ_HAL_PQModeInfo]=1;
#endif
		}
		break;
	}
/* // remove hal by LG's request
	case VPQ_IOC_HDR_DEMO_Mode_ON:
	{
		Scaler_color_set_HDR_DEMO_Mode_ON();
		break;
	}
	case VPQ_IOC_HDR_DEMO_Mode_OFF:
	{
		Scaler_color_set_HDR_DEMO_Mode_OFF();
		break;
	}
*/
	case VPQ_IOC_SET_COLOR_GAMUT:
		{
			if (copy_from_user(&gamutData, (void __user *)arg, sizeof(COLOR_GAMUT_T))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_SET_COLOR_GAMUT fail\n");
				ret = -1;
			} else {
				unsigned char invgamma_enable;
				short gamutMatrix[3][3];

				invgamma_enable = gamutData.gamutMapping.invgamma_enable & 0xff;

				/*rtd_pr_vpq_debug("gamutData.gamutMapping.invgamma_enable = %d, invgamma_enable = %d\n", gamutData.gamutMapping.invgamma_enable, invgamma_enable);*/

				if (g_InvGammaPowerMode != invgamma_enable) {
					g_InvGammaPowerMode = invgamma_enable;

					switch(g_InvGammaPowerMode)
					{
						case VPQ_GAMMA_MODE_INV_OFF_INNER_BYPASS:
						default:
							drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 0, 1);
							drvif_color_inv_gamma_enable(SLR_SUB_DISPLAY, 0, 1);
							break;
						case VPQ_GAMMA_MODE_INV_OFF_INNER_ON:
							drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 0, 1);
							drvif_color_inv_gamma_enable(SLR_SUB_DISPLAY, 0, 1);
							break;
						case VPQ_GAMMA_MODE_INV_ON_INNER_BYPASS:
							drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 1, 1);
							drvif_color_inv_gamma_enable(SLR_SUB_DISPLAY, 1, 1);
							break;
						case VPQ_GAMMA_MODE_INV_ON_INNER_ON:
							drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 1, 1);
							drvif_color_inv_gamma_enable(SLR_SUB_DISPLAY, 1, 1);
							break;
					}
					vpq_set_gamma(1, 1);
				}

				gamutMatrix[0][0] = gamutData.gamutMapping.gamutmatrix_00;
				gamutMatrix[0][1] = gamutData.gamutMapping.gamutmatrix_01;
				gamutMatrix[0][2] = gamutData.gamutMapping.gamutmatrix_02;
				gamutMatrix[1][0] = gamutData.gamutMapping.gamutmatrix_10;
				gamutMatrix[1][1] = gamutData.gamutMapping.gamutmatrix_11;
				gamutMatrix[1][2] = gamutData.gamutMapping.gamutmatrix_12;
				gamutMatrix[2][0] = gamutData.gamutMapping.gamutmatrix_20;
				gamutMatrix[2][1] = gamutData.gamutMapping.gamutmatrix_21;
				gamutMatrix[2][2] = gamutData.gamutMapping.gamutmatrix_22;

				#if 0 //k5l hw remove
				fwif_sRGB_SetMatrix(SLR_MAIN_DISPLAY, sRGBMatrix, g_srgbForceUpdate);
				#else
				//fwif_colorMap_sRGB_SetMatrix_TV006(SLR_MAIN_DISPLAY, sRGBMatrix, g_srgbForceUpdate);
				fwif_set_Gamut_Matrix_TV006(SLR_MAIN_DISPLAY, gamutMatrix);
				#endif
				//g_srgbForceUpdate = 0;

				/*=== Gamut 3D LUT ===*/
				#if 0 //no more use
				if (gamutData.gamutMapping.b3DLUT_internalselect) {//0:RTK DB, 1:LG DB
					if (g_3DLUT_LastInternalSelect != gamutData.gamutMapping.b3DLUT_internalselect || g_3DLUT_Resume) {
						vpq_set_Gamut_3D_Lut_LGDB(&g_buf3DLUT_LGDB, gamutData.gamutMapping.b3DLUT_enable, 0);
						g_3DLUT_Resume = 0;
					} else {
						/*compare table*/
						if (memcmp(&g_buf3DLUT_LGDB.pt, &g_LGDB_3D_LUT_BUF[0].pt, sizeof(GAMUT_3D_LUT_PT_T)*4913) != 0) {//different
							vpq_set_Gamut_3D_Lut_LGDB(&g_buf3DLUT_LGDB, gamutData.gamutMapping.b3DLUT_enable, 0);
						} else if (g_3DLUT_LastEnableSatus == 0 && gamutData.gamutMapping.b3DLUT_enable == 1) {
							vpq_set_Gamut_3D_Lut_LGDB(&g_buf3DLUT_LGDB, gamutData.gamutMapping.b3DLUT_enable, 1);
						}
					}
					if (gamutData.gamutMapping.b3DLUT_enable) {
						memcpy(&g_LGDB_3D_LUT_BUF[0].pt, &g_buf3DLUT_LGDB.pt, sizeof(GAMUT_3D_LUT_PT_T)*4913);
					}
				} else {
					if (g_3DLUT_LastInternalSelect != gamutData.gamutMapping.b3DLUT_internalselect) {
						vpq_set_3d_lut_index(gamutData.gamutMapping.b3DLUT_enable?(gamutData.gamutMapping.u3DLUT_index+1):0, 1); //force write
					} else {
						vpq_set_3d_lut_index(gamutData.gamutMapping.b3DLUT_enable?(gamutData.gamutMapping.u3DLUT_index+1):0, 0);
					}
				}
				g_3DLUT_LastInternalSelect = gamutData.gamutMapping.b3DLUT_internalselect;
				g_3DLUT_LastEnableSatus = gamutData.gamutMapping.b3DLUT_enable;
				#endif
				/*====================*/
				ret = 0;
			}

		}
		break;

	case VPQ_IOC_SET_SAT_LUT:
		{

			if (fwif_color_vpq_stop_ioctl(STOP_VPQ_IOC_SET_SAT_LUT, ACCESS_MODE_GET, 0))
				return 0;

			if (copy_from_user(&satLutData, (int __user *)arg, sizeof(VPQ_SAT_LUT_T))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_SET_SAT_LUT fail\n");
				ret = -1;
			} else {
				ret = fwif_color_set_DSE_tv006(SLR_MAIN_DISPLAY, &satLutData);
				if (ret == 0)
					g_bAPLColorGainClr = 0;
			}
		}
		break;

	case VPQ_IOC_SET_DITHER_BIT:
		{
			unsigned int args;
			if (copy_from_user(&args, (int __user *)arg, sizeof(unsigned int))) {
				rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_PIC_CTRL fail\n");
				ret = -1;
			} else {
				fwif_color_set_Panel_Dither(TRUE,args,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);
				ret = 0;
			}
		}
		break;
	/*
	case VPQ_IOC_SET_GAMUT_3D_LUT_N9_BUFFER:
		{
			GAMUT_3D_LUT_9x9x9_T *pLut = NULL;
			pLut = (GAMUT_3D_LUT_9x9x9_T *)kmalloc(sizeof(GAMUT_3D_LUT_9x9x9_T), GFP_KERNEL);
			if (pLut == NULL) {
				rtd_pr_vpq_err("kernel VPQ_IOC_SET_GAMUT_3D_LUT_N9_BUFFER malloc fail\n");
				return -1;
			}

			if (copy_from_user(pLut, (void __user *)arg, sizeof(GAMUT_3D_LUT_9x9x9_T))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_SET_GAMUT_3D_LUT_N9_BUFFER fail\n");
				ret = -1;
			} else
				ret = fwif_color_set_Gamut_3D_Lut_N9_to_N17_tv006(pLut)?0:-1;

			kfree(pLut);
		}
			break;
	*/
	case VPQ_IOC_SET_GAMUT_3D_LUT_N17_BUFFER:
		{
			if (copy_from_user(&g_buf3DLUT_LGDB.pt, (void __user *)arg, sizeof(GAMUT_3D_LUT_PT_T)*4913)) {
				rtd_pr_vpq_err("kernel VPQ_IOC_SET_GAMUT_3D_LUT_N17_BUFFER fail\n");
				ret = -1;
			} else {
				g_bGamut3DLUT_LGDB_NeedUpdated = 1;
			}
		}
		break;
	case VPQ_IOC_SET_GAMUT_3D_LUT_INDEX:
		{
			unsigned int value;
			if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned int))) {
				rtd_pr_vpq_err("kernel VPQ_IOC_SET_GAMUT_3D_LUT_INDEX fail\n");
				ret = -1;
			} else {
				fwif_color_set_3dLUT(value);
				ret = 0;
			}
		}
		break;
	case VPQ_IOC_SET_LOWDELAY_MODE:
		{
			game_process = 0;
			if (Get_Factory_SelfDiagnosis_Mode()) {
				ret = 0;
			} else {
				bool lowdelaymode;
				//static bool pre_lowdelaymode;
				if (copy_from_user(&lowdelaymode, (int __user *)arg, sizeof(bool))) {
					rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_LOWDELAY_MODE fail\n");
					ret = -1;
				} else {
					game_process = 1;
#if defined(CONFIG_H5CX_SUPPORT)
					VPQEX_rlink_set_Low_Delay((unsigned char)lowdelaymode);
#endif
					fw_scalerip_set_di_gamemode(lowdelaymode);
					//pre_lowdelaymode = lowdelaymode;
					if(((lowdelaymode == FALSE) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE))||(get_vsc_run_pc_mode() == TRUE&&(Get_DisplayMode_Src(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)) == VSC_INPUTSRC_HDMI))){ //not game mode and not _MODE_STATE_ACTIVE
						//fw_scalerip_set_di_gamemode(lowdelaymode);
						//drv_memory_set_game_mode_dynamic(_DISABLE);
						fw_scalerip_set_di_gamemode_setting(_DISABLE);
						//ret = 0;
						//break;
					}
					if (g_bDIGameModeOnlyDebug) //for di game mode debug, only can enabled by rtice tool
						return ret;
#if 0
					if(((pre_lowdelaymode == lowdelaymode) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE))) {
						//fw_scalerip_set_di_gamemode(lowdelaymode);
						ret = 0;
						break;
					}
#endif
					rtd_printk(KERN_EMERG,"VPQ", "##############[MEMC]VPQ_IOC_SET_LOWDELAY_MODE\n");

					if(lowdelaymode) {
						//printk(KERN_EMERG"[crixus]VPQ_IOC_SET_LOWDELAY_MODE1\n");
						//game mode dynamic, check in vsc task @Crixus 20151228
						drv_memory_set_game_mode_dynamic(_ENABLE);
						pr_notice("[%s][%d][LowDelayMode][ENABLE]", __FUNCTION__, __LINE__);
						//vpq_memc_set_lowDelayMode(_ENABLE);
						//Move to new_game_mode_tsk to sync with scaler game mode process
						//Scaler_MEMC_Set_VpqLowDelayToMEMC_Flag(1);
					} else {
						//printk(KERN_EMERG"[crixus]VPQ_IOC_SET_LOWDELAY_MODE0\n");
						//game mode dynamic, check in vsc task @Crixus 20151228
						drv_memory_set_game_mode_dynamic(_DISABLE);
						scaler_vsc_set_gamemode_go_datafrc_mode(FALSE);
						pr_notice("[%s][%d][LowDelayMode][DISABLE]", __FUNCTION__, __LINE__);
						//vpq_memc_set_lowDelayMode(_DISABLE);
						//Move to new_game_mode_tsk to sync with scaler game mode process
						//Scaler_MEMC_Set_VpqLowDelayToMEMC_Flag(0);
					}

					//Game mode cmd checking flow @Cixus 20161204
					if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE){
						down(get_gamemode_check_semaphore());
						/*overflow protection*/
						if(game_mode_cmd_check == 0xffffffff)
							game_mode_cmd_check = 0;
						else if(lowdelaymode != drv_memory_get_game_mode_flag())	// game_mode_cmd_check + 1 when current  != AP setting in picture mode
							game_mode_cmd_check = game_mode_cmd_check + 1;
						up(get_gamemode_check_semaphore());
						//printk(KERN_EMERG"[crixus]game_mode_cmd_check = %d\n", game_mode_cmd_check);
					}
					ret = 0;
				}
			}
		}
		break;

	case VPQ_IOC_SET_OdExtentions:
		{

			HAL_VPQ_DATA_T OD_Ext_Info;
			HAL_VPQ_OD_Extention_T OD_Ext_Data;
			unsigned int i;
			//char delta_pLUT[PCID_TBL_LEN] = {0};

			if( copy_from_user(&OD_Ext_Info, (int __user *)arg, sizeof(HAL_VPQ_DATA_T)) )
			{
				rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_OdExtentions fail\n");
				ret = -1;
				break;
			}
			else
			{
				if( copy_from_user(&OD_Ext_Data, (int __user *)OD_Ext_Info.pData, sizeof(HAL_VPQ_OD_Extention_T)) )
				{
					rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_OdExtentions_pData fail\n");
					ret = -1;
				}
				else
				{
					if( OD_Ext_Data.extType == 0 ) /*POD*/
					{
						if( OD_Ext_Data.extLength != LGE_POD_TBL_LEN )
						{
							rtd_pr_vpq_err("POD LUT length = %d error!\n", OD_Ext_Data.extLength);
							ret = -1;
						}
						else
						{
							unsigned char* pLUT = NULL;
							pLUT = (unsigned char *)dvr_malloc(LGE_POD_TBL_LEN*sizeof(unsigned char));
							if(pLUT == NULL)
							{
								rtd_pr_vpq_err("[ERROR] VPQ_IOC_SET_OdExtentions POD Table allocate fail\n");
								return -1;
							}

							if( copy_from_user(pLUT, (int __user *)OD_Ext_Data.pExtData, LGE_POD_TBL_LEN*sizeof(UINT8)) )
							{
								rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_OdExtentions_pData(POD) fail\n");
								ret = -1;
							}
							else
							{
								fwif_color_set_LGD_POD(pLUT);
								IoReg_SetBits(PPOVERLAY_Double_Buffer_CTRL_reg, PPOVERLAY_Double_Buffer_CTRL_dreg_dbuf_read_sel_mask);
								drvif_color_set_LGD_POD_enable(1);
								drvif_color_set_pcid_enable(0); // PCID and PCID2 cannot enable at the same time
								drvif_color_set_pcid2_enable(0);
								IoReg_ClearBits(PPOVERLAY_Double_Buffer_CTRL_reg, PPOVERLAY_Double_Buffer_CTRL_dreg_dbuf_read_sel_mask);

								ret = 0;
							}
							dvr_free(pLUT);
						}
					}
					else if( OD_Ext_Data.extType == 1 ) /*PCID*/
					{
						if( OD_Ext_Data.extLength != PCID_TBL_LEN*16 )
						{
							rtd_pr_vpq_err("PCID LUT length = %d error!\n", OD_Ext_Data.extLength);
							ret = -1;
						}
						else
						{
							unsigned int* pLUT = NULL;
							unsigned int timeout_cnt = 0x0325000;
							sys_reg_sys_clken3_RBUS sys_reg_sys_clken3_reg;
							ppoverlay_display_timing_ctrl2_RBUS ppoverlay_display_timing_ctrl2_reg;

							pLUT = (unsigned int *)dvr_malloc(PCID_TBL_LEN*16*sizeof(unsigned int));
							if(pLUT == NULL)
							{
								rtd_pr_vpq_err("[ERROR] VPQ_IOC_SET_OdExtentions PCID Table allocate fail\n");
								return -1;
							}

							if( copy_from_user(pLUT, (int __user *)OD_Ext_Data.pExtData, PCID_TBL_LEN*16*sizeof(UINT32)) )
							{
								rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_OdExtentions_pData(PCID) fail\n");
								ret = -1;
							}
							else
							{
								ppoverlay_display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
								if( ppoverlay_display_timing_ctrl2_reg.hcic_enable != 0 )
								{
									rtd_pr_vpq_err("[ERROR] VPQ_IOC_SET_OdExtentions_pData(PCID) HCIC is enabled\n");
									ret = -1;
								}
								else
								{
									// disable hcic clock
									sys_reg_sys_clken3_reg.regValue = IoReg_Read32(SYS_REG_SYS_CLKEN3_reg);
									sys_reg_sys_clken3_reg.regValue = 0;
									sys_reg_sys_clken3_reg.clken_disp_lg_hcic = 1;
									IoReg_Write32(SYS_REG_SYS_CLKEN3_reg, sys_reg_sys_clken3_reg.regValue);

									// start to write pcid setting
									fwif_color_set_pcid2_data_setting(_PCID_DATA_LGE);
									fwif_color_set_pcid2_pixel_reference_setting(_PCID_PXLREF_LGE);
									fwif_color_set_pcid_RgnTable_Setting(_PCID_RGN_17x17_4x4);

									if (drvif_color_get_pcid2_enable() == 1 )
									{
										drvif_color_set_pcid2_enable(0);
										while ((PPOVERLAY_Double_Buffer_CTRL_get_dreg_dbuf_set(IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg)) == 1) && (timeout_cnt > 10))
										{
											timeout_cnt--;
										}
										if (timeout_cnt <= 10)
										{
											rtd_pr_vpq_emerg("kernel copy VPQ_IOC_SET_OdExtentions_pData(PCID) disable time out\n");
										}
									}

									for(i=0;i<16;i++)
									{
										fwif_color_set_pcid_RgnTableValue(pLUT+(i*PCID_TBL_LEN), i, _PCID_COLOR_ALL);
									}

									IoReg_SetBits(PPOVERLAY_Double_Buffer_CTRL_reg, PPOVERLAY_Double_Buffer_CTRL_dreg_dbuf_read_sel_mask);
									drvif_color_set_LGD_POD_enable(0);
									drvif_color_set_pcid_enable(0);
									drvif_color_set_pcid2_enable(1);
									IoReg_ClearBits(PPOVERLAY_Double_Buffer_CTRL_reg, PPOVERLAY_Double_Buffer_CTRL_dreg_dbuf_read_sel_mask);

									ret = 0;
								}
							}
							dvr_free(pLUT);
						}
					}
					else
					{
						rtd_pr_vpq_err("OD Extend type error = %d error\n", OD_Ext_Data.extType);
						ret = -1;
					}
				}
			}
		}
		break;
		case VPQ_IOC_SET_Od:
		{
			if (copy_from_user(&OD_Info, (int __user *)arg, sizeof(HAL_VPQ_DATA_T))) {
				rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_Od fail\n");
				ret = -1;
				break;
			} else {
				unsigned char *od_table_from_user = NULL;
				od_table_from_user = (unsigned char *)dvr_malloc(OD_table_length*sizeof(UINT8));
				if(od_table_from_user == NULL)
				{
					rtd_pr_vpq_err("VPQ_IOC_SET_Od, dvr_malloc fail n");
					ret = -1;
					break;
				}

				if (copy_from_user(&od_table_from_user[0], (int __user *)OD_Info.pData, (OD_Info.length)*sizeof(unsigned char))) {
					rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_Od fail\n");
					dvr_free((void *)od_table_from_user);
					ret = -1;
					break;
				} else {
					extern unsigned char bODInited;
					extern unsigned char bODTableLoaded;
					extern unsigned char bODPreEnable;
					extern char od_table_mode_store;
					extern unsigned char od_delta_table_g[OD_table_length];
					extern unsigned char od_target_table_g[OD_table_length];

					if(OD_table_length == 289)
					{
						fwif_color_od_table_transform(0, 0, od_table_from_user, od_delta_table_g);
						drvif_color_od_table_17x17_set(od_delta_table_g, 0, 3);// 0:delta mode 3:rgb channels
					}
					else if(OD_table_length == 1089)
					{
						#if 1 // k7 bring up work around
						fwif_color_od_table_interpolation(od_table_from_user, od_target_table_g);
						fwif_color_od_table_transform(0, 0, od_target_table_g, od_delta_table_g);
						drvif_color_od_table_33x33_set(od_delta_table_g, 0, 11);// 0:delta mode, 11:rgb channels for sram 0~5
						#else
						fwif_color_od_table_transform(0, 0, od_table_from_user, od_delta_table_g);
						drvif_color_od_table_33x33_set(od_delta_table_g, 0, 3);// 0:delta mode 1:target mode(output mode)
						#endif
					}
					rtd_printk(KERN_EMERG,"VPQ_OD", "%s, od_delta_table_g updated\n", __func__);

					od_table_mode_store = 1;// 0:target mode 1:delta mode
					bODTableLoaded = TRUE;
					dvr_free((void *)od_table_from_user);
					od_table_from_user = NULL;

					/*Enable OD*/
					bODPreEnable = TRUE;
					if (!bODInited || !bODTableLoaded) {
						rtd_pr_vpq_err("VPQ_IOC_SET_Od, bODInited=0 \n");
						ret = -1;
						break;
					}
					else {
						fwif_color_set_od(TRUE);
						ret = 0;
					}
				}
			}
		}
		break;

		case VPQ_IOC_SET_DeMura:
		{
			unsigned char *INNX_Demura_TBL = NULL;
			unsigned char *pINNX_Demura_DeLut = NULL;
			if (copy_from_user(&DeMura_Info, (int __user *)arg, sizeof(HAL_VPQ_DATA_T))) {
				rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_DeMura fail\n");
				ret = -1;
				break;
			} else {
#if 0	/* "Demura_LMMH_4ch_TBL" is too large, use dynamic allocate */
				if (copy_from_user(&Demura_LMMH_4ch_TBL[0], (int __user *)DeMura_Info.pData, (DeMura_Info.length)*sizeof(unsigned char))) {
					rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_DeMura fail\n");
					ret = -1;

				} else {
					vpq_LMMH_demura_data_to_dma(&Demura_LMMH_4ch_TBL[0], &DeMura_TBL, DeMura_TBL.table_mode);
					fwif_color_DeMura_init(1, 0);
					/*rtd_pr_vpq_info("VIP_Demura_4ch_TBL=%d,%d,%d,%d,%d,%d,%d,%d,\n",
						VIP_Demura_4ch_TBL[0], VIP_Demura_4ch_TBL[1],VIP_Demura_4ch_TBL[5],VIP_Demura_4ch_TBL[6], VIP_Demura_4ch_TBL[DeMura_Info.length-1],
						VIP_Demura_4ch_TBL[DeMura_Info.length-4],VIP_Demura_4ch_TBL[DeMura_Info.length-5],VIP_Demura_4ch_TBL[DeMura_Info.length-6]);*/
					ret = 0;

				}
#else
#if 1
/*========================= for INNX demura =================================*/
				if (DeMura_Info.length == 0x80000 && DeMura_Info.version == 2) {	/* INNX */
					INNX_Demura_TBL = (unsigned char *)dvr_malloc_specific(INNX_Demura_TBL_Size * sizeof(char), GFP_DCU2_FIRST);
					pINNX_Demura_DeLut = (unsigned char *)dvr_malloc_specific(INNX_Demura_DecodeLUT_Size * sizeof(char), GFP_DCU2_FIRST);
					if (INNX_Demura_TBL == NULL) {
						rtd_pr_vpq_emerg("INNX Demura dynamic alloc demura memory fail!!!\n");
						ret = -1;
					}else if(pINNX_Demura_DeLut == NULL) {
						rtd_pr_vpq_emerg("pINNX Demura dynamic alloc demura memory fail!!!\n");
						dvr_free((void *)INNX_Demura_TBL);
						ret = -1;
					} else {
						if (copy_from_user(&INNX_Demura_TBL[0], (int __user *)DeMura_Info.pData, (DeMura_Info.length)*sizeof(unsigned char))) {
							rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_DeMura fail for INNX\n");
							ret = -1;
						} else {
							/* TODO: char to short */
							//fwif_color_INNX_Demura_TBL(INNX_Demura_TBL, DeMura_Info.length, pINNX_Demura_DeLut);
							vpq_INNX_Demura_data_to_dma(&DeMura_TBL, pINNX_Demura_DeLut);
							fwif_color_DeMura_init(1, 0);
							ret = 0;
						}
						dvr_free((void *)INNX_Demura_TBL);
						dvr_free((void *)pINNX_Demura_DeLut);
					}
				} else
/*========================= for INNX demura =================================*/
#endif
				{
					Demura_LMMH_4ch_TBL = (unsigned char *)dvr_malloc_specific(Demura_LMMH_TBL_Size * sizeof(char), GFP_DCU2_FIRST);
					if (Demura_LMMH_4ch_TBL == NULL) {
						rtd_pr_vpq_emerg("dynamic alloc demura memory fail!!!\n");
						ret = -1;
					} else {
						if (copy_from_user(&Demura_LMMH_4ch_TBL[0], (int __user *)DeMura_Info.pData, (DeMura_Info.length)*sizeof(unsigned char))) {
							rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_DeMura fail\n");
							ret = -1;

						} else {
							vpq_LMMH_demura_data_to_dma(&Demura_LMMH_4ch_TBL[0], &DeMura_TBL, DeMura_TBL.table_mode);
							fwif_color_DeMura_init(1, 0);
							rtd_pr_vpq_info("dynamic alloc,  VIP_Demura_4ch_TBL=%d,%d,%d,%d,%d,%d,%d,%d,\n",
								Demura_LMMH_4ch_TBL[0], Demura_LMMH_4ch_TBL[1],Demura_LMMH_4ch_TBL[5],Demura_LMMH_4ch_TBL[6], Demura_LMMH_4ch_TBL[DeMura_Info.length-1],
								Demura_LMMH_4ch_TBL[DeMura_Info.length-4],Demura_LMMH_4ch_TBL[DeMura_Info.length-5],Demura_LMMH_4ch_TBL[DeMura_Info.length-6]);

							ret = 0;

						}
						dvr_free((void *)Demura_LMMH_4ch_TBL);

					}
				}
#endif
			}
		}
		break;
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	case VPQ_IOC_SET_AI_PA:
	{
		/*VIP_NN_CTRL *pNN;
		unsigned char idx;
		unsigned int PA_addr[3] = {0};
		pNN = scalerAI_Access_NN_CTRL_STRUCT();

		idx = pNN->PA_IDX;
		PA_addr[0] = pNN->NN_data_Addr[idx].phy_addr_align;
		PA_addr[1] = pNN->NN_info_Addr[idx].phy_addr_align;
		PA_addr[2] = pNN->NN_Flag_Addr[idx].phy_addr_align;

		if (PA_addr[0] == 0 || PA_addr[1] == 0 || PA_addr[2] == 0) {
			rtd_pr_vpq_emerg("kernel VPQ_IOC_SET_AI_PA fail, PA null\n");
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg,(void *) PA_addr, 3 * 4)) {
				rtd_pr_vpq_emerg("kernel VPQ_IOC_SET_AI_PA fail\n");
				ret = -1;
			} else
				ret = 0;
		}*/

		AI_NN_BUF_INFO info;
		VIP_NN_CTRL *pNN;


		if (copy_from_user((void *)&info,(const void __user *) arg, sizeof(info))) {
			ret = -1;
			break;
		}
		file->private_data = (void*)((unsigned long)info.bufferIndex);

		pNN = scalerAI_Access_NN_CTRL_STRUCT();
		/*rtd_pr_vpq_info("henry NN MEM ini, data phy addr=%x, %x, %x\n",
				pNN->NN_data_Addr[0].phy_addr_align,
				pNN->NN_data_Addr[1].phy_addr_align,
				pNN->NN_data_Addr[2].phy_addr_align);

		rtd_pr_vpq_info("henry NN MEM ini, info phy addr=%x, %x, %x\n",
				pNN->NN_info_Addr[0].phy_addr_align,
				pNN->NN_info_Addr[1].phy_addr_align,
				pNN->NN_info_Addr[2].phy_addr_align);

		rtd_pr_vpq_info("henry NN MEM ini, flag phy addr=%x, %x, %x\n",
				pNN->NN_flag_Addr[0].phy_addr_align,
				pNN->NN_flag_Addr[1].phy_addr_align,
				pNN->NN_flag_Addr[2].phy_addr_align);

		rtd_pr_vpq_info("henry NN MEM ini, indx phy addr=%x\n",
				pNN->NN_indx_Addr.phy_addr_align);

		scalerAI_set_NN_buffer();
		rtd_pr_vpq_info("henry buffer idx=%d\n", pNN->NN_indx_Addr.pVir_addr_align[0]);
		*/

		if (pNN == 0) {
			rtd_pr_vpq_emerg("kernel VPQ_IOC_SET_AI_PA fail, PA null\n");
			ret = -1;
		} else {
			memcpy(&info.nnCtrl, pNN, sizeof(VIP_NN_CTRL));
			if (copy_to_user((void __user *)arg,(void *) &info, sizeof(info))) {
				rtd_pr_vpq_emerg("kernel VPQ_IOC_SET_AI_PA fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;
	}	
	case VPQ_IOC_GET_AI_MODE:
	{
		VIP_NN_CTRL *pNN;
		
		
		pNN = scalerAI_Access_NN_CTRL_STRUCT();

		if (pNN == 0) {
			rtd_pr_vpq_emerg("kernel VPQ_IOC_GET_AI_MODE fail, PA null\n");
			ret = -1;
		} else {
			// 0622 lsy
			//if (copy_to_user((void __user *)arg,(void *) &vpq_stereo_face, sizeof(UINT8))) {
			if (copy_to_user((void __user *)arg,(void *) &aipq_mode.ap_mode, sizeof(UINT8))) {
			// end 0622 lsy
				rtd_pr_vpq_emerg("kernel VPQ_IOC_GET_AI_MODE fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;
	}
	case VPQ_IOC_GET_GENRE_LABELS:
	{
		AI_SCENE_LABEL_T genre_label;
		if (copy_from_user((void *)&genre_label,(const void __user *) arg, sizeof(genre_label))) {
			ret = -1;
			break;
		}
		if (genre_label.in_label_num == 0) {
			genre_label.out_label_num = AI_SCENE_TYPE_NUM;

			if (copy_to_user((void __user *)arg,(void *) &genre_label, sizeof(genre_label))) {
				rtd_pr_vpq_emerg("kernel VPQ_IOC_GET_GENRE_LABELS fail\n");
				ret = -1;
			} else
				ret = 0;
			break;
		}

		if (genre_label.in_label_num < AI_SCENE_TYPE_NUM) {
			rtd_pr_vpq_emerg("kernel VPQ_IOC_GET_GENRE_LABELS fail. size is too small\n");

			ret = -1;
			break;
		}
		memset(genre_label.labels, 0, sizeof(genre_label.labels));
		
		strncpy(genre_label.labels[AI_SCENE_ANIMATION], "animation", 50);
        strncpy(genre_label.labels[AI_SCENE_NIGHT_BUILDING], "nighttime_buildings", 50);
        strncpy(genre_label.labels[AI_SCENE_CONCERT], "concert", 50);
        strncpy(genre_label.labels[AI_SCENE_FIREWORKS], "fireworks", 50);
        strncpy(genre_label.labels[AI_SCENE_GAME], "game", 50);
        strncpy(genre_label.labels[AI_SCENE_LANDSCAPE], "landscape", 50);
        strncpy(genre_label.labels[AI_SCENE_NEWS], "news", 50);
        strncpy(genre_label.labels[AI_SCENE_OTHERs], "others", 50);
        strncpy(genre_label.labels[AI_SCENE_SPORTs], "sports", 50);
		
		genre_label.out_label_num = AI_SCENE_TYPE_NUM;
		if (copy_to_user((void __user *)arg,(void *) &genre_label, sizeof(genre_label))) {
			rtd_pr_vpq_emerg("kernel VPQ_IOC_GET_GENRE_LABELS fail\n");
			ret = -1;
		} else
			ret = 0;
		break;
	}
	case VPQ_IOC_AI_DETECTION_ON:
	{
		unsigned int list;

		if (copy_from_user((void *)&list, (const void __user *)arg, sizeof(list)))
		{
			ret = -1;
			printk("VPQ_IOC_AI_DETECTION_ON fail\n");
			break;
		}
		//cann't turn on AI PQ during playing protected content
		if (get_svp_protect_status_aipq()) {
			rtd_pr_vpq_emerg("cannot turn on AI PQ at secure status\n");
			return -1;
		}


		if (list & AI_SCENE_DETECT)
			scalerAI_pq_scene_mode_ctrl_tv001(1);
		if (list & AI_FACE_DETECT)
			scalerAI_pq_mode_ctrl_tv001(1, aipq_DynamicContrastLevel);
		if (list & AI_SQM_DETECT)
			scalerAI_pq_sqm_mode_ctrl_tv001(1);
		if (list & AI_DEPTH_DETECT)
			scalerAI_pq_depth_mode_ctrl_tv001(1);
		if (list & AI_NOISE_DETECT)
			scalerAI_pq_noise_mode_ctrl_tv001(1);
		ret = 0;
		break;
	}
	case VPQ_IOC_AI_DETECTION_OFF:
	{
		unsigned int list;
		if (copy_from_user((void *)&list, (const void __user *)arg, sizeof(list)))
		{
			ret = -1;
			printk("VPQ_IOC_AI_DETECTION_OFF fail\n");
			break;
		}


		if (list & AI_SCENE_DETECT)
			scalerAI_pq_scene_mode_ctrl_tv001(0);
		if (list & AI_FACE_DETECT)
			scalerAI_pq_mode_ctrl_tv001(0, 0);
		if (list & AI_SQM_DETECT)
			scalerAI_pq_sqm_mode_ctrl_tv001(0);
		if (list & AI_DEPTH_DETECT)
			scalerAI_pq_depth_mode_ctrl_tv001(0);
		if (list & AI_NOISE_DETECT)
			scalerAI_pq_noise_mode_ctrl_tv001(0);
		ret = 0;
		break;
	}
	case VPQ_IOC_GET_90K_CLK:
	{
		unsigned int value = drvif_Get_90k_Lo_clk();
		if (copy_to_user((void __user *)arg, (void *)&value, sizeof(unsigned int))) {
			rtd_pr_vpq_emerg("kernel VPQ_IOC_GET_90K_CLK fail\n");
			ret = -1;
		} else {
			ret = 0;
		}
		break;
	}
	case VPQ_IOC_PQ_GENERAL_CMD:
	{
		int copyRet = 0;
		CMD_GENERAL_T UserCmd;

		copyRet = copy_from_user((void *)&UserCmd, (void __user *)arg, sizeof(CMD_GENERAL_T));
		if( copyRet != 0 ) {
			rtd_pr_vpq_emerg("kernel VPQ_IOC_PQ_GENERAL_CMD copy from user fail %d\n", copyRet);
			ret = -1;
			break;
		}

		switch( UserCmd.id ) {
			case VPQ_GENERAL_CMD_GET_90KCLK:
			{
				unsigned int value = 0;

				if( UserCmd.size != sizeof(unsigned int) ) {
					rtd_pr_vpq_emerg("kernel VPQ_GENERAL_CMD_GET_90KCLK, id %d size %d mismatch\n", UserCmd.id, UserCmd.size);
					ret = -1;
					break;
				}
				
				value = drvif_Get_90k_Lo_clk();

				copyRet = copy_to_user((void __user *)UserCmd.ptr, (void *)&value, sizeof(unsigned int));
				if( copyRet != 0 ) {
					rtd_pr_vpq_emerg("kernel VPQ_GENERAL_CMD_GET_90KCLK, id %d copy to user fail %d\n", UserCmd.id, copyRet);
					ret = -1;
					break;
				}

				ret = 0;
				break;
			}

			case VPQ_GENERAL_CMD_SEMANTIC_DONE:
			{
				if( UserCmd.size != 0 ) {
					rtd_pr_vpq_emerg("kernel VPQ_GENERAL_CMD_SEMANTIC_DONE, id %d size %d mismatch\n", UserCmd.id, UserCmd.size);
					ret = -1;
					break;
				}

				// scalerPQMask_postprocessing_entry();
				ret = 0;
				break;
			}

			default:
			{
				rtd_pr_vpq_emerg("kernel VPQ_IOC_PQ_GENERAL_CMD id %d not found\n", UserCmd.id);
				break;
			}
		}
		break;
	}
	#if IS_ENABLED(CONFIG_RTK_AI_OPTEE)
	case VPQ_IOC_GET_NN_TA_BUF:
	{
		NN_BUF_INFO_T info;
		int copyRet = copy_from_user((void *)&info, (void __user *)arg, sizeof(NN_BUF_INFO_T));
		if( copyRet != 0 ) {
			rtd_pr_vpq_emerg("[AI] VPQ_IOC_GET_NN_TA_BUF copy from user fail %d\n", copyRet);
			return -1;
		}
		if(!ai_optee_get_nn_ta_buf(&info))
			return -1;

		copyRet = copy_to_user((void __user *)arg, (void *)&info, sizeof(NN_BUF_INFO_T));
		if( copyRet != 0 ) {
			rtd_pr_vpq_emerg("[AI]VPQ_IOC_GET_NN_TA_BUF copy to user fail %d\n", copyRet);
			return -1;
		}

		ret = 0;
		break;
	}
	case VPQ_IOC_FREE_NN_TA_BUF:
	{
		NN_BUF_INFO_T info;
		int copyRet = copy_from_user((void *)&info, (void __user *)arg, sizeof(NN_BUF_INFO_T));
		if( copyRet != 0 ) {
			rtd_pr_vpq_emerg("[AI]VPQ_IOC_FREE_NN_TA_BUF copy from user fail %d\n", copyRet);
			return -1;
		}

		if(!ai_optee_release_nn_ta_buf(&info))
			return -1;

		ret = 0;
		break;
	}
	#endif //CONFIG_RTK_AI_OPTEE
#endif

	default:
		rtd_pr_vpq_debug("kernel IO command %d not found!\n", cmd);
		return -1;
	}
	return ret;/*Success*/

}
#ifdef CONFIG_SCALER_ENABLE_V4L2
extern unsigned char vpq_memc_ioctl_get_stop_run_by_idx(unsigned char cmd_idx);
extern unsigned char vpq_led_ioctl_get_stop_run_by_idx(unsigned char cmd_idx);
unsigned char vpq_v4l2_main_get_stop_run(unsigned int cmd)
{
	unsigned int filter_cmd =0xff;
	unsigned char flag0=0, flag1=0;

	//filter_cmd = cmd&0x0fff;

	if (cmd >= V4L2_CID_EXT_LED_BASE && cmd < (V4L2_CID_EXT_LED_BASE + 0x100)) {	// led
		//filter_cmd = filter_cmd&0xff;
		filter_cmd = cmd - V4L2_CID_EXT_LED_BASE;
		flag0 = vpq_led_ioctl_get_stop_run_by_idx(0);
		flag1 = vpq_led_ioctl_get_stop_run_by_idx(filter_cmd&0xff);

	} else if (cmd >= V4L2_CID_EXT_MEMC_BASE && cmd < (V4L2_CID_EXT_MEMC_BASE + 0x100)) {
		//filter_cmd = (filter_cmd&0xff)+10;
		filter_cmd = cmd - V4L2_CID_EXT_MEMC_BASE;
		flag0 = vpq_memc_ioctl_get_stop_run_by_idx(0);
		flag1 = vpq_memc_ioctl_get_stop_run_by_idx(filter_cmd&0xff);

	} else if (cmd >= V4L2_CID_EXT_HDR_BASE && cmd < (V4L2_CID_EXT_HDR_BASE + 0x100)) {
		//filter_cmd = (filter_cmd&0xff)+20;
		filter_cmd = cmd - V4L2_CID_EXT_HDR_BASE;
		flag0 = vpq_HDR_ioctl_get_stop_run_by_idx(0);
		flag1 = vpq_HDR_ioctl_get_stop_run_by_idx(filter_cmd&0xff);

	} else if (cmd >= V4L2_CID_EXT_DOLBY_BASE && cmd < (V4L2_CID_EXT_DOLBY_BASE + 0x100)) {
		//filter_cmd = (filter_cmd&0xff)+30;
		filter_cmd = cmd - V4L2_CID_EXT_DOLBY_BASE;
		flag0 = vpq_dolbyHDR_ioctl_get_stop_run_by_idx(0);
		flag1 = vpq_dolbyHDR_ioctl_get_stop_run_by_idx(filter_cmd&0xff);

	} else if (cmd >= V4L2_CID_EXT_VPQ_BASE && cmd < (V4L2_CID_EXT_VPQ_BASE + 0x100)){
		//filter_cmd = (filter_cmd&0xff)+40;
		filter_cmd = cmd - V4L2_CID_EXT_VPQ_BASE;
		flag0 = vpq_ioctl_get_stop_run_by_idx(0);
		flag1 = vpq_ioctl_get_stop_run_by_idx(filter_cmd&0xff);

	}

	//rtd_pr_vpq_emerg("sta=%x,end=%x, cmd=%x,\n", V4L2_CID_EXT_LED_INIT, V4L2_CID_EXT_VPQ_DECONTOUR, cmd);

	return ((flag0|flag1)&0x1);

}

char vpq_v4l2_DM2_EOTF_Check(unsigned int* pV4L2_EOTF_TBL)
{
	unsigned short i;
	char ret = 0;

	for (i=1;i<EOTF_size;i++) {
		if (pV4L2_EOTF_TBL[i] < pV4L2_EOTF_TBL[i-1]) {
			rtd_pr_vpq_emerg("DM_EOTF_DMA, v4l2 tbl incorrect\n");
			ret = 1;
			break;
		}
	}

	return ret;
}

char vpq_v4l2_DM2_OETF_Check(unsigned short* pV4L2_OETF_TBL)
{
	unsigned short i;
	char ret = 0;
	unsigned short diff;
	unsigned short diff_th = 0x8000;

	for (i=1;i<OETF_size;i++) {
		diff = (pV4L2_OETF_TBL[i] > pV4L2_OETF_TBL[i-1])?(pV4L2_OETF_TBL[i] - pV4L2_OETF_TBL[i-1]):(pV4L2_OETF_TBL[i-1] - pV4L2_OETF_TBL[i]);
		if ((diff > diff_th)) {
			rtd_pr_vpq_emerg("DM_OETF_DMA, v4l2 tbl incorrect\n");
			ret = 1;
			break;
		}
	}

	return ret;
}

unsigned int CONTRAST_LUT_WA_T[4][16] = {
	{64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 936, 1023},	// off
	{64, 120, 187, 245, 315, 388, 460, 526, 590, 655, 716, 773, 829, 886, 926, 1023},	// low
	{64, 108, 157, 215, 305, 392, 476, 541, 610, 672, 730, 778, 826, 876, 916, 1023},	// mid
	{64,	90,   127, 185, 295, 398, 492, 567, 635, 692, 747, 784, 822, 858, 901, 1023},	// high
};
char vpq_v4l2_DYNAMIC_CONTRAST_LUT_WA_T(unsigned int* pLUT)
{
	static unsigned char pre_ui_index = 0xff;
	unsigned char Force16 = 0, i = 0;
	unsigned char ui_index = 0;
	if (pLUT == NULL)
		return -1;

	for (i = 16; i < 32; i++) {
		if (pLUT[i] < pLUT[i-1] || pLUT[i] > 1023/* || pLUT[i] < 0*/) {
			Force16 = TRUE;
			break;
		}
	}

	if (pLUT[3] == 127)
		ui_index = 3;
	else if (pLUT[3] == 140)
		ui_index = 2;
	else if (pLUT[3] == 181)
		ui_index = 0;
	else
		ui_index = 1;

	if (pre_ui_index != ui_index) {
		rtd_pr_vpq_info("DYNAMIC_CONTRAST_LUT_WA_T, ui=%d->%d,\n", pre_ui_index, ui_index);
		pre_ui_index = ui_index;
	}

	// check pAccLUT points
	if (pLUT[15] >= 940 || Force16) { //16 points
#if 0
		// index 0~2, for ire0 and ire 10,
		pLUT[0] =  (pLUT[0] < 64)?(64):(pLUT[0]);
		pLUT[1] =  (pLUT[1] < 128)?(128):(pLUT[1]);
		pLUT[2] =  (pLUT[2] < 192)?(192):(pLUT[2]);

		for (i = 1; i < 16; i++) {
			if (pLUT[i] < pLUT[i-1])
				pLUT[i] = pLUT[i-1] + 8;
		}
#else
		memcpy(pLUT, CONTRAST_LUT_WA_T[ui_index], sizeof(int)*16);
#endif
	} /*else {	// 32 points	// only for 16p

	}*/
	return 0;
}

static unsigned char vpq_V4L2_CID_EXT_VPQ_BLACK_LEVEL_Mapping(struct v4l2_ext_vpq_black_level_info* v4l2_blackLevel)
{
	unsigned char level = RGB2YUV_BLACK_LEVEL_LOW;

	if (v4l2_blackLevel != NULL) {
		if (v4l2_blackLevel->ui_value == Blk_Lv_UI_LEVEL_AUTO && v4l2_blackLevel->curr_input != HAL_VPQ_INPUT_ATV && v4l2_blackLevel->curr_input != HAL_VPQ_INPUT_AV) {
			if (v4l2_blackLevel->black_level_type == V4L2_EXT_VPQ_BLACKLEVEL_Y709_LINEAR_LIMIT_HIGH ||
			v4l2_blackLevel->black_level_type == V4L2_EXT_VPQ_BLACKLEVEL_RGB_Y709_LIMIT_HIGH ||
			v4l2_blackLevel->black_level_type == V4L2_EXT_VPQ_BLACKLEVEL_RGB_BT2020_LIMIT_HIGH ||
			v4l2_blackLevel->black_level_type == V4L2_EXT_VPQ_BLACKLEVEL_RGB_Y601_LIMIT_HIGH)
				level = RGB2YUV_BLACK_LEVEL_HIGH;
			else
				level = RGB2YUV_BLACK_LEVEL_LOW;
		} else if (v4l2_blackLevel->ui_value == Blk_Lv_UI_LEVEL_HIGH) {
			level = RGB2YUV_BLACK_LEVEL_HIGH;
		} else {
			level = RGB2YUV_BLACK_LEVEL_LOW;
		}
	} else {
		rtd_pr_vpq_emerg(" func:%s [error] is null\r\n",__FUNCTION__);
	}

	return level;
}
#endif

#if IS_ENABLED(CONFIG_RTK_AI_DRV)
// 0622 lsy
int vpq_v4l2_ai_sqm_ctrl(unsigned char aipq_sqm_mode)
{
	int ret = -1;
	static int status = 0; // 0: finished. -1: not ready.

	if(status == -1)
	{
		printk("aipq v4l2 setting not ready: sqm %d, \n", aipq_sqm_mode);
		return ret;
	}

	status = -1;

	//if(aipq_sqm_mode==0 || aipq_sqm_mode==1)
	//	ret = scalerAI_pq_sqm_mode_ctrl_tv006(aipq_sqm_mode);

	if(aipq_sqm_mode == 0)
	{
		aipq_mode.sqm_mode = 0;
		ret = scalerAI_pq_mode_ctrl(aipq_mode, 0);
	}
	else if(aipq_sqm_mode == 1)
	{
		aipq_mode.sqm_mode= 1;
		ret = scalerAI_pq_mode_ctrl(aipq_mode, 0);

	}
	else if(aipq_sqm_mode == 2)
	{
		aipq_mode.sqm_mode= 2;
		ret = scalerAI_pq_mode_ctrl(aipq_mode, 0);
	}

	status = 0;

	return ret;
}
// end 0622 lsy

int vpq_v4l2_ai_ctrl(unsigned char stereo_face, unsigned char dcValue)
{

	int ret = -1;
	static int status = 0; // 0: finished. -1: not ready.

	if(status == -1)
	{
		printk("aipq v4l2 setting not ready: stereo %d, dcValue %d\n", stereo_face, dcValue);
		return ret;
	}

	status = -1;

	if(stereo_face == V4L2_VPQ_EXT_STEREO_FACE_OFF)
	{
		aipq_mode.face_mode = 0;
		aipq_mode.sqm_mode=0;
		aipq_mode.depth_mode= 0;
		aipq_mode.pqmask_mode= 0;

		ret = scalerAI_pq_mode_ctrl(aipq_mode, 0);
		// ret = scalerAI_pq_mode_ctrl_tv006(stereo_face, 0);
	}
	else if(stereo_face == V4L2_VPQ_EXT_STEREO_FACE_ON)
	{
		aipq_mode.face_mode = 1;
		aipq_mode.sqm_mode=1;
		aipq_mode.depth_mode= 1;
		aipq_mode.pqmask_mode= 1;
		ret = scalerAI_pq_mode_ctrl(aipq_mode, 0);
		scalerPQMaskColor_ForceOutputCur(10);
		// ret = scalerAI_pq_mode_ctrl_tv006(stereo_face, dcValue);
	}
	else if(stereo_face == V4L2_VPQ_EXT_STEREO_FACE_DEMO)
	{

	
		aipq_mode.face_mode = 2;
		aipq_mode.depth_mode= 2;
		ret = scalerAI_pq_mode_ctrl(aipq_mode, 0);
		//ret = scalerAI_pq_mode_ctrl_tv006(stereo_face, 4);
	}

	status = 0;

	return ret;
}
int vpq_v4l2_ai_depth_ctrl(unsigned char aipq_depth_mode)
{
	int ret = -1;
	static int status = 0; // 0: finished. -1: not ready.

	if(status == -1)
	{
		printk("aipq v4l2 setting not ready: vpq_v4l2_ai_depth_ctrl %d \n", aipq_depth_mode);
		return ret;
	}

	status = -1;

	if(aipq_depth_mode == 0)
	{
		aipq_mode.depth_mode = 0;
		aipq_mode.pqmask_mode = 0;
		ret = scalerAI_pq_mode_ctrl(aipq_mode, 0);
	}
	else if(aipq_depth_mode == 1)
	{
		aipq_mode.depth_mode = 1;
		aipq_mode.pqmask_mode = 1;
		ret = scalerAI_pq_mode_ctrl(aipq_mode, 0);
	}
	else if(aipq_depth_mode == 2)
	{
		aipq_mode.depth_mode = 2;
		aipq_mode.pqmask_mode = 2;
		ret = scalerAI_pq_mode_ctrl(aipq_mode, 0);
	}

	status = 0;

	return ret;
}


#endif

#ifdef CONFIG_SCALER_ENABLE_V4L2
UINT8 V4l2_AIPQ_SR_MODE=0;

void vpq_v4l2_ai_nnsr_set_En(unsigned char bEn)
{
	V4l2_AIPQ_SR_MODE = bEn;

}
char vpq_v4l2_ai_nnsr_get_En(void)
{

	return V4l2_AIPQ_SR_MODE;
}

CHIP_SHARPNESS_T tSHAPNESS_TABLE;
struct v4l2_ext_gamma_lut gamma_lut;
unsigned int v4l2_gamma_table_num;
signed short  v4l2_vpq_MATRIX_PRE[9] = {0,};
v4l2_ext_led_db_lut stReal_db_lut;
unsigned char fV4l2_memc_lowdeay=0;
unsigned char V4l2_memc_motion_pro=0;
struct v4l2_ext_memc_motion_comp_info motion_comp_info;
CHIP_NOISE_REDUCTION_T v4l2_tNOISE_TABLE;
CHIP_MPEG_NOISE_REDUCTION_T v4l2_tMPEG_NOISE_TABLE;
struct v4l2_ext_vpq_black_level_info v4l2_blackLevel;
CHIP_SR_UI_T v4l2_tSR_UI_T;
struct v4l2_ext_vpq_super_resolution_data super_resolution;
unsigned char v4l2_sr_ui_value;

bool v4l2_vpq_lowdelaymode;
UINT8 v4l2_vpq_ire;
//struct v4l2_colortemp_info v4l2_color_temp;
struct v4l2_ext_vpq_color_temp v4l2_color_temp;

UINT8 v4l2_real_cinema=0;
struct v4l2_vpq_gamut_post gamutPost;

struct v4l2_ext_vpq_gamut_lut gamutLut;

CHIP_SHARPNESS_T tSHAPNESS_TABLE;

UINT32 Gamme_Red[1024];
UINT32 Gamme_Green[1024];
UINT32 Gamme_Blue[1024];

extern KADP_LED_DB_LUT_T LED_DB_LUT[7];
DRV_HAL_VPQ_LED_LDCtrlDemoMode LED_LDCtrlDemoMode;
unsigned char LDCtrlSPI[2];

signed int dc_lut[32] = {0,};

#ifdef	CONFIG_RTK_KDRV_DV
#include <dolby_idk_1_6_1_1/dolby_adapter.h>
static void *get_v4l2_vpq_cmn_data(struct v4l2_vpq_cmn_data *pq_data, void __user *ptr)
{
	int length;
	void *data;

	if (pq_data == NULL)
		return NULL;

	if (copy_from_user((void *)pq_data, ptr, sizeof(struct v4l2_vpq_cmn_data))) {
		return NULL;
	}

	length = (pq_data) ? pq_data->length : 0;
	data = kzalloc(length, GFP_KERNEL);

	if (data && !copy_from_user(data, pq_data->p_data, length)) {
		return data;
	} else {
		kfree(data);
		return NULL;
	}
}
#endif


struct v4l2_vpq_dynamnic_contrast_lut_RTK dynamiccontrastlut;
struct v4l2_ext_dynamnic_contrast_ctrl dynamic_ctrl;
struct v4l2_ext_vpq_decontour_data de_contour;
struct v4l2_ext_vpq_block_bypass v4l2_blockbypass;
unsigned char CSC3_HDR_flag_v4l2 = 0xff;
struct v4l2_ext_vpq_sharpness_data sharpness;

struct v4l2_ext_cm_info cm_info;
struct v4l2_ext_vpq_noise_reduction_data noiseReduction;
struct v4l2_ext_vpq_mpeg_noise_reduction_data mpeg_noiseReduction;
struct v4l2_ext_pq_mode_info_RTK pqModeInfo;

struct v4l2_vpq_ext_pattern_info_v2 ext_pattern_info_v2;
int vpq_v4l2_main_ioctl_s_ext_ctrls(struct file *file, void *fh, struct v4l2_ext_controls *ctrls)
{
	unsigned char DM_DMA_mode = 0;
	unsigned int size_byte;

        int ret = 0;
#ifdef isSupport_DM2_TM
	unsigned int i;
#endif
        unsigned int cmd = 0xff;

	_system_setting_info *vip_sysInfo = NULL;
	_RPC_system_setting_info* RPC_SysInfo = NULL;
	unsigned char hdrtype_vsc;
        //unsigned filter_cmd =0xff;
        struct v4l2_ext_control ext_control;
        struct v4l2_vpq_cmn_data pqData;

        SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
        gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
        if (gVip_Table == NULL) {
                return -1;
        }

	vip_sysInfo = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	RPC_SysInfo = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
        if(!ctrls)
        {
            rtd_pr_vpq_emerg(" func:%s [error] ctrls is null\r\n",__FUNCTION__);
            ret = -EFAULT;
            return ret;
        }


	memcpy(&ext_control,ctrls->controls,sizeof(struct v4l2_ext_control));
        cmd = ext_control.id;

	if (vpq_v4l2_main_get_stop_run(cmd))
		return 0;

       V4L2printf_cmd(" VPQ_V4L2 :s_ext_ctrls , cmd ID =0x%x ", cmd);

	switch(cmd)
        {
                case V4L2_CID_EXT_VPQ_INIT:
                {

                        rtd_pr_vpq_emerg("VPQ init \n");

                        if (vpq_project_id != 0x00060000) {
                        	vpq_project_id = 0x00060000;
                        	vpq_boot_init();
                        } else {
                                if (Demo_Flag && VPQ_IOC_SET_PIC_CTRL == cmd)
                        		vpq_demo_overscan_func(NULL);

                        }

                        fwif_color_inv_gamma_control_back(SLR_MAIN_DISPLAY, 0);
                        fwif_color_gamma_control_back(SLR_MAIN_DISPLAY, 1);
                        fwif_color_gamma_control_back(SLR_SUB_DISPLAY, 1);
#ifdef CONFIG_DUAL_CHANNEL
                        fwif_color_inv_gamma_control_back(SLR_SUB_DISPLAY, 0);
                        fwif_color_gamma_control_back(SLR_SUB_DISPLAY, 1);
#endif
                        fwif_color_set_DCC_Init(SLR_MAIN_DISPLAY);
                        /*fwif_color_set_DCC_Init_tv006();*/
                        fwif_color_set_dcc_force_write_flag_RPC(1, 0);

                        // enable DM & Composer clock
                        drvif_Set_DM_HDR_CLK();
                        //drvif_TV006_HDR10_init();
                        fwif_color_set_FILM_FW_ShareMemory();
                        fwif_color_set_Picture_Control_tv006(pic_ctrl.wId, &pic_ctrl);

				Scaler_color_Set_HDR_AutoRun(TRUE); // use HFC to set HDR inv gamma and EOTF for TV006

                        PQ_Dev_Status = PQ_DEV_INIT_DONE;

                        if (PQ_Dev_Status != PQ_DEV_INIT_DONE)
                        	return -1;


                }
                break;
                case V4L2_CID_EXT_VPQ_PICTURE_CTRL:
                {

                        struct v4l2_ext_picture_ctrl_data_RTK pictureCtrl;
                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {
                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }
                                if(copy_from_user((void *)&pictureCtrl, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_picture_ctrl_data_RTK)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }
#if 1
                                V4L2printf_Info("bri=%d,cont=%d,sat=%d,hue=%d, \n",
                                        pictureCtrl.sBrightness,
                                        pictureCtrl.sContrast,
                                        pictureCtrl.sSaturation,
                                        pictureCtrl.sHue);
                                V4L2printf_Info("sPcVal[0]=%d,sPcVal[1]=%d,sPcVal[2]=%d,sPcVal[3]=%d,\n",
                                        pictureCtrl.sPcVal[0],
                                        pictureCtrl.sPcVal[1],
                                        pictureCtrl.sPcVal[2],
                                        pictureCtrl.sPcVal[3]);

                                /* this is for work around. after lg fixed that , please remove */
				/*pictureCtrl.sContrast = (pictureCtrl.sPcVal[0] * 128 + 50) /100;
				pictureCtrl.sBrightness = (pictureCtrl.sPcVal[1] * 255 + 50) /100;
				pictureCtrl.sSaturation = (pictureCtrl.sPcVal[2] * 255 + 50) /100;
				pictureCtrl.sHue = (pictureCtrl.sPcVal[3] * 255 + 50) /100;*/
                                /* this is for work around. after lg fixed that , please remove */

				pic_ctrl.wId = pqData.wid;

				pic_ctrl.Gain_Val[PIC_CTRL_Contrast] =pictureCtrl.sContrast;
				pic_ctrl.Gain_Val[PIC_CTRL_Brightness] =pictureCtrl.sBrightness;
				pic_ctrl.Gain_Val[PIC_CTRL_Color] =pictureCtrl.sSaturation;
				pic_ctrl.Gain_Val[PIC_CTRL_Tint] =pictureCtrl.sHue;

				pic_ctrl.siVal[PIC_CTRL_Contrast] = pictureCtrl.sPcVal[0];
				pic_ctrl.siVal[PIC_CTRL_Brightness] = pictureCtrl.sPcVal[1];
				pic_ctrl.siVal[PIC_CTRL_Color] = pictureCtrl.sPcVal[2];
				pic_ctrl.siVal[PIC_CTRL_Tint] = pictureCtrl.sPcVal[3];
#endif

#if 0 // lesley, for CES
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
				if(ai_scene_ctrl.ai_scene_global.scene_en == 0)
#endif
#endif
                                ret = fwif_color_set_Picture_Control_tv006(0, &pic_ctrl);

				PictureMode_flg = 1;  // for MEMC wrt by JerryWang 20161125
                        }
                }
                break;
                case V4L2_CID_EXT_VPQ_SHARPNESS:
                {

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }
                                if(copy_from_user((void *)&sharpness, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_vpq_sharpness_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;

                                }
                                if(sharpness.pst_chip_data ==NULL) //LG DB NULL, workaround
                                {
                                        UINT8 source=0,idx=0;
                                        SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
                                        gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
                                        if (gVip_Table == NULL) {
                                                return -1;
                                        }
                                        ret = 0;
                                        rtd_pr_vpq_emerg("LG DB NULL, workaround  V4L2_CID_EXT_VPQ_SHARPNESS \n" );

                                        /*get pqa table index need check source first. because ap flow or table index will get incorrect table*/
                                        source = fwif_vip_source_check(3, NOT_BY_DISPLAY);/* get vip source by input src and timing*/
                                        if (source >= VIP_QUALITY_SOURCE_NUM)
                                                source = 0;

                                        idx = gVip_Table->VIP_QUALITY_Extend3_Coef[source][VIP_QUALITY_FUNCTION_SharpTable];

                                        fwif_color_set_sharpness_table(0, idx);//fixed me after demo please remove
                                        Scaler_SetSharpness(sharpness.ui_value[0]); //fixed me after demo pleae remove

                                        if(sharpness.ui_value[0] <15)
                                        {
                                                Scaler_SetDCti(gVip_Table->VIP_QUALITY_Coef[source][VIP_QUALITY_FUNCTION_DCTi]); //fixed me
                                                Scaler_SetIDCti(gVip_Table->VIP_QUALITY_Coef[source][VIP_QUALITY_FUNCTION_I_DCTi]);//fixed me
                                                Scaler_SetDLti(gVip_Table->VIP_QUALITY_Coef[source][VIP_QUALITY_FUNCTION_DLTi]);//fixed me
                                                Scaler_SetMBPeaking(gVip_Table->VIP_QUALITY_Extend3_Coef[source][VIP_QUALITY_FUNCTION_MB_Peaking]); //fixed me after need to remove
                                                Scaler_SetMBSUPeaking(gVip_Table->VIP_QUALITY_Extend3_Coef[source][VIP_QUALITY_FUNCTION_MB_SU_Peaking]); //fixed me after need to remove
                                                Scaler_SetDLti(255);
                                        }
                                        else
                                        {
                                                Scaler_SetDCti(gVip_Table->VIP_QUALITY_Coef[source][VIP_QUALITY_FUNCTION_DCTi]); //fixed me
                                                Scaler_SetIDCti(gVip_Table->VIP_QUALITY_Coef[source][VIP_QUALITY_FUNCTION_I_DCTi]);//fixed me
                                                Scaler_SetDLti(gVip_Table->VIP_QUALITY_Coef[source][VIP_QUALITY_FUNCTION_DLTi]);//fixed me
                                                Scaler_SetMBPeaking(gVip_Table->VIP_QUALITY_Extend3_Coef[source][VIP_QUALITY_FUNCTION_MB_Peaking]); //fixed me after need to remove
                                                Scaler_SetMBSUPeaking(gVip_Table->VIP_QUALITY_Extend3_Coef[source][VIP_QUALITY_FUNCTION_MB_SU_Peaking]); //fixed me after need to remove
                                        }

                                        return ret;

                                } else {
						rtd_pr_vpq_info("get table from V4L2_CID_EXT_VPQ_SHARPNESS\n" );
                                }
                                if(copy_from_user((void *)&tSHAPNESS_TABLE, to_user_ptr(sharpness.pst_chip_data), sizeof(CHIP_SHARPNESS_T)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

					//vpq_V4L2_CID_EXT_VPQ_SHARPNESS_DC_check(&tSHAPNESS_TABLE);

                                memcpy((unsigned char*)&tCurCHIP_SHARPNESS_UI_T.stSharpness,(unsigned char*)&tSHAPNESS_TABLE,sizeof( CHIP_SHARPNESS_T));

                                fwif_color_set_sharpness_table_TV006(&tCurCHIP_SHARPNESS_UI_T, &tCurCHIP_SR_UI_T);
                                fwif_color_Get_sharpness_table_TV006();
                                V4L2printf_Info("sharp_UI=%d,SR_UI =%d \n",sharpness.ui_value[0],sharpness.ui_value[1]);

                        }
                }
                break;
                case V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST:
                {
                        if (fwif_color_vpq_stop_ioctl(STOP_VPQ_IOC_SET_FreshContrast, ACCESS_MODE_GET, 0)){
                                return 0;
                                }

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {
                                //int i=0;

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_from_user((void *)&dynamic_ctrl, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_dynamnic_contrast_ctrl)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }



                                if(dynamic_ctrl.pst_chip_data ==NULL)
                                {
                                       return ret;
                                }

                                if(copy_from_user((void *)&tFreshContrast_coef, to_user_ptr(dynamic_ctrl.pst_chip_data), sizeof( CHIP_DCC_T)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }



                                if (fwif_color_set_dcc_FreshContrast_tv006(&tFreshContrast_coef) == FALSE) {
                                        rtd_pr_vpq_err("kernel fwif_color_set_dcc_FreshContrast_tv006 fail\n");
                                        ret = -1;
                                } else {
                                        /*rtd_pr_vpq_debug("5.kernel VPQ_IOC_SET_FreshContrast success\n");*/
                                        ret = 0;
                                }


                               /* rtd_pr_vpq_emerg("rord   dynamic_ctrl.uDcVal=%d \n", dynamic_ctrl.uDcVal);

                                rtd_pr_vpq_emerg("sizeof chroma_gain_en=%d \n",sizeof(tFreshContrast_coef.stChromaGain.chroma_gain_en));
                                rtd_pr_vpq_emerg("sizeof chroma_gain_mode=%d \n",sizeof(tFreshContrast_coef.stChromaGain.chroma_gain_mode));

                                rtd_pr_vpq_emerg("DCC chroma_gain_en=%d \n",tFreshContrast_coef.stChromaGain.chroma_gain_en);
                                rtd_pr_vpq_emerg("DCC chroma_gain_mode=%d \n",tFreshContrast_coef.stChromaGain.chroma_gain_mode);

                                rtd_pr_vpq_emerg("DCC chroma_gain_lookup_mode=%d \n",tFreshContrast_coef.stChromaGain.chroma_gain_lookup_mode);
                                rtd_pr_vpq_emerg("DCC chroma_gain_base=%d \n",tFreshContrast_coef.stChromaGain.chroma_gain_base);
                                rtd_pr_vpq_emerg("DCC chroma_enhance_above_tab_select_main=%d \n",tFreshContrast_coef.stChromaGain.chroma_enhance_above_tab_select_main);
                                rtd_pr_vpq_emerg("DCC chroma_enhance_below_tab_select_main=%d \n",tFreshContrast_coef.stChromaGain.chroma_enhance_below_tab_select_main);
                                rtd_pr_vpq_emerg("DCC saturation_byy_u_tab_select_main=%d \n",tFreshContrast_coef.stChromaGain.saturation_byy_u_tab_select_main);
                                rtd_pr_vpq_emerg("DCC saturation_byy_v_tab_select_main=%d \n",tFreshContrast_coef.stChromaGain.saturation_byy_v_tab_select_main);
                                rtd_pr_vpq_emerg("DCC chroma_gain_limit_en=%d \n",tFreshContrast_coef.stChromaGain.chroma_gain_limit_en);

                               for(i=0;i<21;i++){

                                        rtd_pr_vpq_emerg("DCC chroma_gain_limit_seg[%d]=%d \n",i,tFreshContrast_coef.stChromaGain.chroma_gain_limit_seg[i]);
                                }

                                for(i=0;i<17;i++){
                                rtd_pr_vpq_emerg("DCC chroma_gain_tab0_seg[%d]=%d \n",i,tFreshContrast_coef.stChromaGain.chroma_gain_tab0_seg[i]);
                                        }

                                for(i=0;i<17;i++){
                                rtd_pr_vpq_emerg("DCC chroma_gain_tab1_seg[%d]=%d \n",i,tFreshContrast_coef.stChromaGain.chroma_gain_tab1_seg[i]);
                                        }

                                rtd_pr_vpq_emerg("DCC y0_center =%d \n",tFreshContrast_coef.stSkinTone.range.y0_center);
                                rtd_pr_vpq_emerg("DCC u0_center =%d \n",tFreshContrast_coef.stSkinTone.range.u0_center);
                                rtd_pr_vpq_emerg("DCC v0_center =%d \n",tFreshContrast_coef.stSkinTone.range.v0_center);
                                rtd_pr_vpq_emerg("DCC y0_range =%d \n",tFreshContrast_coef.stSkinTone.range.y0_range);
                                rtd_pr_vpq_emerg("DCC u0_range =%d \n",tFreshContrast_coef.stSkinTone.range.u0_range);
                                rtd_pr_vpq_emerg("DCC v0_range =%d \n",tFreshContrast_coef.stSkinTone.range.v0_range);

                                rtd_pr_vpq_emerg("DCC y1_center =%d \n",tFreshContrast_coef.stSkinTone.range.y1_center);
                                rtd_pr_vpq_emerg("DCC u1_center =%d \n",tFreshContrast_coef.stSkinTone.range.u1_center);
                                rtd_pr_vpq_emerg("DCC v1_center =%d \n",tFreshContrast_coef.stSkinTone.range.v1_center);
                                rtd_pr_vpq_emerg("DCC y1_range =%d \n",tFreshContrast_coef.stSkinTone.range.y1_range);
                                rtd_pr_vpq_emerg("DCC u1_range =%d \n",tFreshContrast_coef.stSkinTone.range.u1_range);
                                rtd_pr_vpq_emerg("DCC v1_range =%d \n",tFreshContrast_coef.stSkinTone.range.v1_range);


                                rtd_pr_vpq_emerg("DCC y2_center =%d \n",tFreshContrast_coef.stSkinTone.range.y2_center);
                                rtd_pr_vpq_emerg("DCC u2_center =%d \n",tFreshContrast_coef.stSkinTone.range.u2_center);
                                rtd_pr_vpq_emerg("DCC v2_center =%d \n",tFreshContrast_coef.stSkinTone.range.v2_center);
                                rtd_pr_vpq_emerg("DCC y2_range =%d \n",tFreshContrast_coef.stSkinTone.range.y2_range);
                                rtd_pr_vpq_emerg("DCC u2_range =%d \n",tFreshContrast_coef.stSkinTone.range.u2_range);
                                rtd_pr_vpq_emerg("DCC v2_range =%d \n",tFreshContrast_coef.stSkinTone.range.v2_range);

                                rtd_pr_vpq_emerg("DCC y3_center =%d \n",tFreshContrast_coef.stSkinTone.range.y3_center);
                                rtd_pr_vpq_emerg("DCC u3_center =%d \n",tFreshContrast_coef.stSkinTone.range.u3_center);
                                rtd_pr_vpq_emerg("DCC v3_center =%d \n",tFreshContrast_coef.stSkinTone.range.v3_center);
                                rtd_pr_vpq_emerg("DCC y3_range =%d \n",tFreshContrast_coef.stSkinTone.range.y3_range);
                                rtd_pr_vpq_emerg("DCC u3_range =%d \n",tFreshContrast_coef.stSkinTone.range.u3_range);
                                rtd_pr_vpq_emerg("DCC v3_range =%d \n",tFreshContrast_coef.stSkinTone.range.v3_range);

                                rtd_pr_vpq_emerg("DCC D_DCC_SKIN_EN =%d \n",tFreshContrast_coef.stSkinTone.value.D_DCC_SKIN_EN);

                                for(i=0;i<4;i++){
                                        rtd_pr_vpq_emerg("DCC dcc_skin_region_en[%d]=%d \n",i,tFreshContrast_coef.stSkinTone.value.dcc_skin_region_en[i]);
                                }
                                for(i=0;i<4;i++){
                                        rtd_pr_vpq_emerg("DCC dcc_skin_region_enhance[%d]=%d \n",i,tFreshContrast_coef.stSkinTone.value.dcc_skin_region_enhance[i]);
                                }

                                for(i=0;i<8;i++){
                                        rtd_pr_vpq_emerg("DCC Y0_seg[%d]=%d \n",i,tFreshContrast_coef.stSkinTone.value.Y0_seg[i]);
                                }
                                for(i=0;i<8;i++){
                                        rtd_pr_vpq_emerg("DCC Y1_seg[%d]=%d \n",i,tFreshContrast_coef.stSkinTone.value.Y1_seg[i]);
                                }
                                for(i=0;i<8;i++){
                                        rtd_pr_vpq_emerg("DCC Y2_seg[%d]=%d \n",i,tFreshContrast_coef.stSkinTone.value.Y2_seg[i]);
                                }
                                for(i=0;i<8;i++){
                                        rtd_pr_vpq_emerg("DCC Y3_seg[%d]=%d \n",i,tFreshContrast_coef.stSkinTone.value.Y3_seg[i]);
                                }

                                for(i=0;i<8;i++){
                                        rtd_pr_vpq_emerg("DCC c0_seg[%d]=%d \n",i,tFreshContrast_coef.stSkinTone.value.C0_seg[i]);
                                }
                                for(i=0;i<8;i++){
                                        rtd_pr_vpq_emerg("DCC c1_seg[%d]=%d \n",i,tFreshContrast_coef.stSkinTone.value.C1_seg[i]);
                                }
                                for(i=0;i<8;i++){
                                        rtd_pr_vpq_emerg("DCC c2_seg[%d]=%d \n",i,tFreshContrast_coef.stSkinTone.value.C2_seg[i]);
                                }
                                for(i=0;i<8;i++){
                                        rtd_pr_vpq_emerg("DCC c3_seg[%d]=%d \n",i,tFreshContrast_coef.stSkinTone.value.C3_seg[i]);
                                }*/

                        }
                }
                break;
                case V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST_LUT:
                {
                        #define RHAL_VPQ_DCC_NODE_NUM 33


						// rord 0814 remove
                        if (fwif_color_vpq_stop_ioctl(STOP_VPQ_IOC_SET_FreshContrast, ACCESS_MODE_GET, 0))
						// end rord 0814 remove
                                return 0;

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_from_user((void *)&dynamiccontrastlut, to_user_ptr(pqData.p_data), sizeof(struct v4l2_vpq_dynamnic_contrast_lut_RTK)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

#if 0
                                if(copy_from_user((void *)dc_lut, to_user_ptr(pqData.p_data), sizeof(dc_lut)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }
#endif
                                //rtd_pr_vpq_emerg("dc_lut [0]=%d,[1]=%d,[2]=%d,[3]=%d,[4]=%d, \n",
                                //dc_lut[0],dc_lut[1],dc_lut[2],dc_lut[3],dc_lut[4]);

                                if (drvif_color_get_WB_pattern_on()) {
                                        if (!fwif_color_set_bypass_dcc_Curve_Write_tv006()) {
                                              rtd_pr_vpq_err("kernel fwif_color_set_dcc_Curve_Write_tv006 fail\n");
                                                ret = -1;
                                        }else if (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE) {
                                              if (!g_prevent_force_write_dcc)
                                                      fwif_color_set_dcc_force_write_flag_RPC(0, 1);
                                              g_prevent_force_write_dcc = 1;
                                              return 0;
                                        }

                                        return 0;
                                }

                                if(1) //  do Interpolation , form 16 point to 129
                                {
                                        int i, j, k, a, b, Force16 = FALSE;
                                        //FreshContrastLUT_T tFreshContrastLUT;
                                        FreshContrastHalLUT_T tFreshContrastHalLUT;
                                        //SINT32 pAccLUT[32]={0};
						CHIP_DYNAMIC_CONTRAST_LUT DYNAMIC_CONTRAST_LUT_DB;
                                        memcpy(&DYNAMIC_CONTRAST_LUT_DB.sLumaLut[0], dynamiccontrastlut.sLumaLUTxy, sizeof(SINT32)*CHIP_NUM_TRANSCURVE_RTK);

                                        memset(&tFreshContrastHalLUT, 0, sizeof(FreshContrastHalLUT_T));
                                       // memset(&tDCC_LUT, 0, sizeof(FreshContrastLUT_T));

#ifdef LGDB_OFF	// WA patch, wait for DB ready, need to remove in future
                                        vpq_v4l2_DYNAMIC_CONTRAST_LUT_WA_T(&DYNAMIC_CONTRAST_LUT_DB.sLumaLut[0]);
#endif

                                        // check monotonically increasing
                                        for (i = 16; i < 32; i++) {
                                                if (DYNAMIC_CONTRAST_LUT_DB.sLumaLut[i] < DYNAMIC_CONTRAST_LUT_DB.sLumaLut[i-1] || DYNAMIC_CONTRAST_LUT_DB.sLumaLut[i] > 1023 || DYNAMIC_CONTRAST_LUT_DB.sLumaLut[i] < 0) {
                                                        Force16 = TRUE;
                                                        break;
                                                }
                                        }

                                        // check pAccLUT points
                                        if (DYNAMIC_CONTRAST_LUT_DB.sLumaLut[15] >= 940 || Force16) { //16 points
                                                tFreshContrastHalLUT.FreshContrastHalLUT[0] = 0;
                                                for (i = 2; i < RHAL_VPQ_DCC_NODE_NUM; i+=2)
                                                        tFreshContrastHalLUT.FreshContrastHalLUT[i] = DYNAMIC_CONTRAST_LUT_DB.sLumaLut[(i>>1)-1]<<2;//10bits to 12bits
                                                for (i = 1; i < RHAL_VPQ_DCC_NODE_NUM-4; i+=2)
                                                        tFreshContrastHalLUT.FreshContrastHalLUT[i] = (tFreshContrastHalLUT.FreshContrastHalLUT[i+1]+tFreshContrastHalLUT.FreshContrastHalLUT[i-1])>>1;

                                                tFreshContrastHalLUT.FreshContrastHalLUT[29] = (tFreshContrastHalLUT.FreshContrastHalLUT[28]+(tFreshContrastHalLUT.FreshContrastHalLUT[30]<<2))/5;
                                                tFreshContrastHalLUT.FreshContrastHalLUT[31] = (tFreshContrastHalLUT.FreshContrastHalLUT[30]*(1023-992)+tFreshContrastHalLUT.FreshContrastHalLUT[32]*(992-936))/(1023-936);
                                        } else { //32 points
                                                tFreshContrastHalLUT.FreshContrastHalLUT[0] = 0;
                                                for (i = 1; i < RHAL_VPQ_DCC_NODE_NUM; i++)
                                                        tFreshContrastHalLUT.FreshContrastHalLUT[i] = DYNAMIC_CONTRAST_LUT_DB.sLumaLut[i-1]<<2;//10bits to 12bits
                                        }

                                        // calc bin 0~116
                                        for (i = 0; i < RHAL_VPQ_DCC_NODE_NUM-4; i++) {
                                                a = tFreshContrastHalLUT.FreshContrastHalLUT[i];
                                                b = tFreshContrastHalLUT.FreshContrastHalLUT[i+1];
                                                for (j = 0; j < 4; j++) {
                                                        k = i*4+j;
                                                        if (k < DCC_Curve_Node_MAX)
                                                                tFreshContrastLUT.FreshContrastLUT[k] = (b*j+a*(4-j))>>2;
                                                }
                                        }
                                        tFreshContrastLUT.FreshContrastLUT[116] = tFreshContrastHalLUT.FreshContrastHalLUT[29];//Level 928

                                        // calc bin 117~124
                                        a = tFreshContrastLUT.FreshContrastLUT[117] = tFreshContrastHalLUT.FreshContrastHalLUT[30];//level 936
                                        b = tFreshContrastLUT.FreshContrastLUT[124] = tFreshContrastHalLUT.FreshContrastHalLUT[31];//Level 992
                                        for (j = 1; j < 7; j++) {
                                                k = 117+j;
                                                tFreshContrastLUT.FreshContrastLUT[k] = (b*j+a*(7-j))/7;
                                        }
                                        // calc 125~128
                                        a = b;
                                        b = tFreshContrastLUT.FreshContrastLUT[128] = tFreshContrastHalLUT.FreshContrastHalLUT[32];//Level 1023
                                        for (j = 1; j < 4; j++) {
                                                k = 124+j;
                                                tFreshContrastLUT.FreshContrastLUT[k] = (b*j+a*(4-j))>>2;
                                        }
                                }

                                if(fwif_color_set_dcc_Curve_Write_tv006(&tFreshContrastLUT.FreshContrastLUT[0]) == FALSE) {
                                        rtd_pr_vpq_err("kernel fwif_color_set_dcc_Curve_Write_tv006 fail\n");
                                        ret = -1;
                                } else
                                        ret = 0;

                                g_prevent_force_write_dcc = 0;

                        }
                }
                break;
				case V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST_COLOR_GAIN:
				{
					if (fwif_color_vpq_stop_ioctl(STOP_VPQ_IOC_SET_DYNAMIC_CONTRAST_COLOR_GAIN, ACCESS_MODE_GET, 0)) return 0;

					if(!ext_control.ptr)
					{
						rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
						ret = -EFAULT;
						return ret;
					} else {
						if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
						{
							rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
							ret = -EFAULT;
							return ret;
						} else {
							rtd_pr_vpq_emerg(" V4L2_CID_EXT, color gain , [%d][%d][%d]\n", *pqData.p_data, pqData.compat_data, pqData.version);
							#if 0
							fwif_color_access_apl_color_gain_tv006(pqData.wid, (int*)pqData.p_data, ACCESS_MODE_WRITE);
							if (g_bAPLColorGainClr) {
								fwif_color_update_YUV2RGB_CSC_tv006(pqData.wid);
								g_bAPLColorGainClr = 0;
							} else
								g_bAPLColorGainClr = 1;
							#endif
						}
					}
				}
				break;
				
                case V4L2_CID_EXT_VPQ_CM_DB_DATA:
				{

					if(!ext_control.ptr) {
						rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
						ret = -EFAULT;
						return ret;
					} else {

						if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data))) {
							rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
							ret = -EFAULT;
							return ret;
						}

						if(copy_from_user((void *)&cm_info, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_cm_info))) {
							rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user ICM DB fail \r\n",__FUNCTION__,__LINE__);
							ret = -EFAULT;
							return ret;
						}

						/* 0: use dbInfo, 1: use driver internal db */
						if(cm_info.use_internal_cm_db) {
							ret = 0;
							rtd_pr_vpq_emerg("LG DB not ready, workaround  V4L2_CID_EXT_VPQ_CM_DB_DATA \n" );

							fwif_color_set_ICM_tv006_null_case(cm_info.uiInfo);
							vpq_ICM_handler(1, VPQ_IOC_SET_FRESH_COLOR);
							return ret;
						}


						/* get CM data from webOS */
						//rtd_pr_vpq_info("LG DB mode V4L2_CID_EXT_VPQ_CM_DB_DATA \n" );
						if(copy_from_user((void *)&g_cm_v4l2, to_user_ptr(cm_info.dbInfo), sizeof(CHIP_CM_RESULT_T)))
						{
							rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail [g_cm_v4l2]\r\n",__FUNCTION__,__LINE__);
							ret = -EFAULT;
							return ret;
						}
						/* LG ICM mode */
						g_Color_Mode = COLOR_MODE_ADV;	/* default advance mode */
						if (cm_info.uiInfo.cms.enable) g_Color_Mode = COLOR_MODE_EXP;	/* expert mode */

						//rtd_pr_vpq_info("LG DB mode V4L2_CID_EXT_VPQ_CM_DB_DATA copy success\r\n");
						/* set data to RTK */
						fwif_color_CM_v4l2_data_check_tv006(&g_cm_v4l2);
						fwif_color_set_CM_v4l2_data_to_RTK_table_tv006(&g_cm_v4l2);

						vpq_ICM_handler(1, VPQ_IOC_SET_FRESH_COLOR);
#if 0
						v4l2_UV_offset_enable = g_cm_v4l2.stCMClearWhite.enable;
						memcpy(&uvOffset.Uoffset[0], &g_cm_v4l2.stCMClearWhite.Uoffset[0], sizeof(char)*VIP_YUV2RGB_Y_Seg_Max);
						memcpy(&uvOffset.Voffset[0], &g_cm_v4l2.stCMClearWhite.Voffset[0], sizeof(char)*VIP_YUV2RGB_Y_Seg_Max);
						memcpy(&uvOffset.UV_index[0], &g_cm_v4l2.stCMClearWhite.UV_offset_index[0], sizeof(char)*(VIP_YUV2RGB_Y_Seg_Max-1));
						for(idx=0;idx<(VIP_YUV2RGB_Y_Seg_Max-2);idx++)
							uvOffset.UV_index[idx] = g_cm_v4l2.stCMClearWhite.UV_offset_index[idx] - 1;

						memcpy(&uv_offset_byuv_curve.gain[0], &g_cm_v4l2.stCMClearWhite.gain_by_UV[0], sizeof(char)*VIP_YUV2RGB_Y_Seg_Max);
						memcpy(&uv_offset_byuv_curve.gainCurve_index[0], &g_cm_v4l2.stCMClearWhite.gain_by_UV_index[0], sizeof(char)*(VIP_YUV2RGB_Y_Seg_Max-1));
#endif
						if (!fwif_color_set_YUV2RGB_UV_Offset_tv006(0/*display*/, v4l2_UV_offset_enable, 2, &uvOffset, &uv_offset_byuv_curve))
							return -1;
#if 1
						ret = fwif_color_set_DSE_tv006(SLR_MAIN_DISPLAY, &satLUTData_in_CM);
						if (ret == 0) g_bAPLColorGainClr = 0;
#endif
					}
				}
                break;

                case V4L2_CID_EXT_VPQ_NOISE_REDUCTION:
                {


                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }
                                if(copy_from_user((void *)&noiseReduction, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_vpq_noise_reduction_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;


                                }

                                if(noiseReduction.pst_chip_data ==NULL)
                                {
                                        rtd_pr_vpq_emerg("LG DB NULL, workaround  V4L2_CID_EXT_VPQ_NOISE_REDUCTION \n" );

                                        Scaler_SetDNR(noiseReduction.ui_value[0]);//control by rtk driver
                                        return ret;

                                }
                                if(copy_from_user((void *)&v4l2_tNOISE_TABLE, to_user_ptr(noiseReduction.pst_chip_data), sizeof(CHIP_NOISE_REDUCTION_T)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }


                                memcpy((unsigned char*)&NR_Level.NR_TABLE,(unsigned char*)&v4l2_tNOISE_TABLE,sizeof( CHIP_NOISE_REDUCTION_T));
                                NR_Level.NR_LEVEL = noiseReduction.ui_value[0];

                                V4L2printf_Info("NOISE_REDUCTION NR_UI=%d, \n",NR_Level.NR_LEVEL);
                                fwif_color_Set_NR_Table_tv006(&NR_Level);

                                fwif_color_SetDNR_tv006(NR_Level.NR_LEVEL);
                        }
                }
                break;
                case V4L2_CID_EXT_VPQ_MPEG_NOISE_REDUCTION:
                {
                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }
                                if(copy_from_user((void *)&mpeg_noiseReduction, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_vpq_mpeg_noise_reduction_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }


                                if(mpeg_noiseReduction.pst_chip_data ==NULL)
                                {
                                        rtd_pr_vpq_emerg("LG DB NULL, workaround  V4L2_CID_EXT_VPQ_MPEG_NOISE_REDUCTION \n" );
                                        Scaler_SetMPEGNR(mpeg_noiseReduction.ui_value,0);//control by rtk driver
                                        return ret;

                                }

                                if(copy_from_user((void *)&v4l2_tMPEG_NOISE_TABLE, to_user_ptr(mpeg_noiseReduction.pst_chip_data), sizeof(CHIP_MPEG_NOISE_REDUCTION_T)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                memcpy((unsigned char*)&MPEGNR_Level.MNR_TABLE,(unsigned char*)&v4l2_tMPEG_NOISE_TABLE,sizeof( CHIP_MPEG_NOISE_REDUCTION_T));
                                MPEGNR_Level.MPEG_LEVEL = mpeg_noiseReduction.ui_value;

                                fwif_color_Set_MPEGNR_Table_tv006(&MPEGNR_Level);
                                fwif_color_Set_MPEGNR_tv006(MPEGNR_Level.MPEG_LEVEL);


                                V4L2printf_Info( "MPEG_NOISE_REDUCTION NR_UI=%d, \n",MPEGNR_Level.MPEG_LEVEL);

                        }
                }
                break;

		case V4L2_CID_EXT_VPQ_BYPASS_BLOCK:
		{
			if(!ext_control.ptr)
			{
				rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
				ret = -EFAULT;
				return ret;
			}
			else
			{
				if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
				{
					rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
					ret = -EFAULT;
					return ret;
				}
				if(copy_from_user((void *)&v4l2_blockbypass, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_vpq_block_bypass)))
				{
					printk(KERN_EMERG"func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
					ret = -EFAULT;
					return ret;
				}
				else
				{
					if (vip_sysInfo == NULL) {
						rtd_pr_vpq_emerg("func:%s line:%d [error] vip_sysInfo=NULL \r\n",__FUNCTION__,__LINE__);
						ret = -EFAULT;
						return ret;
					}
					rtd_pr_vpq_info("V4L2_CID_EXT_VPQ_BYPASS_BLOCK, MASK=%x, OnOff=%d,\n", v4l2_blockbypass.bypassMask, v4l2_blockbypass.bOnOff);
					// for HDR part
					if (vip_sysInfo->DolbyHDR_flag == 0) {
						if (((v4l2_blockbypass.bypassMask & V4L2_VPQ_BYPASS_MASK_HDR_EXCEPT_PCC_RTK) != 0) && (v4l2_blockbypass.bOnOff == 1)) {
#ifdef isSupport_HDR_3dLUT_24x24x24
							fwif_color_set_DM_HDR_3dLUT_24x24x24_16_TV006(NULL, 1);
#else
							fwif_color_set_DM_HDR_3dLUT_17x17x17_16_TV006(NULL, 1);
#endif
							fwif_color_set_DM2_EOTF_TV006(NULL, 1);
							fwif_color_set_DM2_OETF_TV006(NULL, 1);
#ifdef isSupport_DM2_TM							
							fwif_color_set_DM2_ToneMapping_TV006(NULL);
#endif
						} else {	// resume flow
							if (vip_sysInfo->HDR_flag == HAL_VPQ_HDR_MODE_HDR10)
								hdrtype_vsc = HDR_DM_HDR10;
							else if (vip_sysInfo->HDR_flag == HAL_VPQ_HDR_MODE_HLG)
								hdrtype_vsc = HDR_DM_HLG;
							else
								hdrtype_vsc = HDR_DM_SDR_MAX_RGB;
							Scaler_color_HDRIP_AutoRun(hdrtype_vsc);
						}
					}
				}
			}
		}
		break;

                case V4L2_CID_EXT_VPQ_BLACK_LEVEL:
                {


                        unsigned char  input_info;
                        unsigned char  level;
                        unsigned char mCon, mBri, mHue, mSat;
                        unsigned char isBlack_LvDiff_Flag = 0;

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_from_user((void *)&v4l2_blackLevel, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_vpq_black_level_info)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;

                                }

					if (vip_sysInfo == NULL) {
						rtd_pr_vpq_emerg("func:%s line:%d [error] vip_sysInfo=NULL \r\n",__FUNCTION__,__LINE__);
						ret = -EFAULT;
						return ret;
					}
                                input_info = v4l2_blackLevel.curr_input;
                                //level = v4l2_blackLevel.ui_value;

				rtd_pr_vpq_info("v4l2_Blk_Lv, color space =%d,level_type=%d,curr_input=%d,ui_value=%d,\n",
					v4l2_blackLevel.color_space,v4l2_blackLevel.black_level_type,v4l2_blackLevel.curr_input,v4l2_blackLevel.ui_value);
#if 0
				switch (v4l2_blackLevel.ui_value)
				{
        				case Blk_Lv_UI_LEVEL_LOW:
        					level = RGB2YUV_BLACK_LEVEL_LOW;
        					break;
        				case Blk_Lv_UI_LEVEL_HIGH:
        					level = RGB2YUV_BLACK_LEVEL_HIGH;
        					break;
        				case Blk_Lv_UI_LEVEL_AUTO:
        					level = RGB2YUV_BLACK_LEVEL_AUTO;
        					break;
        				default:
        					level = RGB2YUV_BLACK_LEVEL_AUTO;
				}
#else
				level = vpq_V4L2_CID_EXT_VPQ_BLACK_LEVEL_Mapping(&v4l2_blackLevel);
#endif

                                if ((input_info == HAL_VPQ_INPUT_ATV || input_info == HAL_VPQ_INPUT_AV) && (get_sub_OutputVencMode() == 0)) {
                                        if (fwif_color_get_VDBlackLevel_MappingValue(input_info, level-1, &mCon, &mBri, &mHue, &mSat) == 0)
                                        {
                                                fwif_color_set_VDBlackLevel(mCon, mBri, mHue, mSat);

                                        }
										/* For ATV/AV, black lv control bt ADC, so CSC1 value set bypass */
										level = RGB2YUV_BLACK_LEVEL_LOW;
                                }

                                /*20151128 roger, WOSQRTK-3568, LG want input_info == HAL_VPQ_INPUT_MEDIA_MOVIE, and level == RGB2YUV_BLACK_LEVEL_AUTO then level = RGB2YUV_BLACK_LEVEL_LOW directly*/
                                if (input_info == HAL_VPQ_INPUT_MEDIA_MOVIE)
                                {
                                        if (level == RGB2YUV_BLACK_LEVEL_AUTO)
                                                level = RGB2YUV_BLACK_LEVEL_LOW;
                                }

                                /* compare with previous, change black lv from UI while comparation is differnet*/
                                if (level == fwif_color_get_rgb2yuv_black_level_tv006())
                                        isBlack_LvDiff_Flag = 0;
                                else
                                        isBlack_LvDiff_Flag = 1;

                                fwif_color_set_rgb2yuv_black_level_tv006(level);

#ifndef CONFIG_HDR_SDR_SEAMLESS	/* seamless need to check force bg and mode active*/
                                Scaler_SetDataFormatHandler();
#else
			if (get_support_vo_force_v_top(support_sdr_max_rgb)) {
				Scaler_Set_HDR_YUV2RGB(vip_sysInfo->HDR_flag, HAL_VPQ_COLORIMETRY_MAX);
			} else {
                                if ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE) && (Check_AP_Set_Enable_ForceBG(SLR_MAIN_DISPLAY) == FALSE))
                                {
                                        Scaler_Set_HDR_YUV2RGB(vip_sysInfo->HDR_flag, HAL_VPQ_COLORIMETRY_BT2020);
                                        if (isBlack_LvDiff_Flag == 1) {
                                                Scaler_SetDataFormatHandler();
                                        } else {
                                                /* seamless. do seamless process in isr (void Scaler_hdr_setting_SEAMLESS()), avoid transient noise*/
                                                ;
                                        }
                                } else {
                                        Scaler_Set_HDR_YUV2RGB(vip_sysInfo->HDR_flag, HAL_VPQ_COLORIMETRY_BT2020);
                                        Scaler_SetDataFormatHandler();
                                }
			}
                                HDR_SDR_SEAMLESS_PQ.HDR_SDR_SEAMLESS_PQ_SET[HDR_SDR_SEAMLESS_PQ_HAL_PQModeInfo]=1;
#endif
                                ret = 0;
                        }

                }
                break;

                case V4L2_CID_EXT_VPQ_GAMMA_DATA:
                {
                        UINT16 i=0;

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_from_user((void *)&gamma_lut, to_user_ptr(pqData.p_data), sizeof(gamma_lut)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }
                                if(copy_from_user((void *)Gamme_Blue, to_user_ptr(gamma_lut.table_blue), sizeof(Gamme_Blue)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }
                                if(copy_from_user((void *)Gamme_Green, to_user_ptr(gamma_lut.table_green), sizeof(Gamme_Green)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }
                                if(copy_from_user((void *)Gamme_Red, to_user_ptr(gamma_lut.table_red), sizeof(Gamme_Red)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }
                                v4l2_gamma_table_num = gamma_lut.table_num;
                                for(i = 0; i < 1024; i++)
                                {
                                        gamma.Gamme_Blue[i] = Gamme_Blue[i];
                                        gamma.Gamme_Green[i] = Gamme_Green[i];
                                        gamma.Gamme_Red[i] = Gamme_Red[i];
                                }

                                rtd_pr_vpq_info("V4L2_CID_EXT_VPQ_GAMMA_DATA, %d,\n", g_InvGammaPowerMode);

                                vpq_set_OSD_gamma();
                                vpq_set_gamma(1, 1);
                                //fwif_color_gamma_control_back(SLR_MAIN_DISPLAY, 0);

                                ret = 0;
                        }

                }
                break;
                case V4L2_CID_EXT_VPQ_SUPER_RESOLUTION:
                {


                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }
                                if(copy_from_user((void *)&super_resolution, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_vpq_super_resolution_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;

                                }

                                if(super_resolution.pst_chip_data==NULL)
                                {
                                        rtd_pr_vpq_emerg("LGDB Null point workaround V4L2_CID_EXT_VPQ_SUPER_RESOLUTION \n" );

                                        drvif_color_setSR_levle_Tv006(super_resolution.ui_value);

                                        return ret;

                                }

                                if(copy_from_user((void *)&v4l2_tSR_UI_T, to_user_ptr(super_resolution.pst_chip_data), sizeof(CHIP_SR_UI_T)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                v4l2_sr_ui_value = super_resolution.ui_value;

                                memcpy(&tCurCHIP_SR_UI_T, &v4l2_tSR_UI_T, sizeof(CHIP_SR_UI_T));

                                // driver still need refine /rord.tsao
                                fwif_color_access_DeJaggy_Level(super_resolution.ui_value, 1);
                                Scaler_setiESM(gVip_Table->VIP_QUALITY_Extend2_Coef[RPC_SysInfo->VIP_source][VIP_QUALITY_FUNCTION_iESM_table]);
#if 0 //control table by rtk
                                        drvif_color_setSR_levle_Tv006(tRTK_SR_UI_T.SR_LEVEL);
#else
                                //driver still need refie. rord

                                fwif_color_set_sharpness_table_TV006(&tCurCHIP_SHARPNESS_UI_T, &tCurCHIP_SR_UI_T);
                                fwif_color_Get_sharpness_table_TV006();

                                V4L2printf_Info("SUPER_RESOLUTION  SR_UI =%d \n",super_resolution.ui_value);

#endif

                        }
                }
                break;

                case V4L2_CID_EXT_VPQ_COLORTEMP_DATA:
                {
                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_from_user((void *)&v4l2_color_temp, to_user_ptr(pqData.p_data), sizeof(v4l2_color_temp)))
                                {


                                }

                                V4L2printf_Info("COLORTEMP_DATA R=%d,G=%d,B=%d,r=%d,g=%d,b=%d\n",
                                        v4l2_color_temp.rgb_gain[0],
                                        v4l2_color_temp.rgb_gain[1],
                                        v4l2_color_temp.rgb_gain[2],
                                        v4l2_color_temp.rgb_offset[0],
                                        v4l2_color_temp.rgb_offset[1],
                                        v4l2_color_temp.rgb_offset[2]

                                );


                                curColorTemp.Blue_Gain = v4l2_color_temp.rgb_gain[V4L2_EXT_VPQ_BLUE];
                                curColorTemp.Green_Gain= v4l2_color_temp.rgb_gain[V4L2_EXT_VPQ_GREEN];
                                curColorTemp.Red_Gain= v4l2_color_temp.rgb_gain[V4L2_EXT_VPQ_RED];

                                curColorTemp.Blue_Offset= v4l2_color_temp.rgb_offset[V4L2_EXT_VPQ_BLUE];
                                curColorTemp.Green_Offset= v4l2_color_temp.rgb_offset[V4L2_EXT_VPQ_GREEN];
                                curColorTemp.Red_Offset= v4l2_color_temp.rgb_offset[V4L2_EXT_VPQ_RED];


                                curColorTemp.Red_Gain = v4l2_color_temp.rgb_gain[0]>=192?(320+v4l2_color_temp.rgb_gain[0]):(256+(v4l2_color_temp.rgb_gain[0]<<8)/192);
                                curColorTemp.Green_Gain = v4l2_color_temp.rgb_gain[1]>=192?(320+v4l2_color_temp.rgb_gain[1]):(256+(v4l2_color_temp.rgb_gain[1]<<8)/192);
                                curColorTemp.Blue_Gain = v4l2_color_temp.rgb_gain[2]>=192?(320+v4l2_color_temp.rgb_gain[2]):(256+(v4l2_color_temp.rgb_gain[2]<<8)/192);
                                curColorTemp.Red_Offset = v4l2_color_temp.rgb_offset[0]+448;
                                curColorTemp.Green_Offset = v4l2_color_temp.rgb_offset[1]+448;
                                curColorTemp.Blue_Offset = v4l2_color_temp.rgb_offset[2]+448;

#ifdef GSR_move_GSR_to_LD_Compesation
								rtd_pr_vpq_info("GSR in LDC\n");
#else
                        	if (strcmp(webos_strToolOption.eBackLight, "oled") == 0)//==> It's means led set.
                        	{
                        	        if(g_color_temp_init==0){
                                                vpq_set_color_temp_filter(); //  oled set ,setting once. update color temp by GSR funciton
                                                g_color_temp_init =1;
                                        }
                        	}
                                else
#endif
								{
                                        vpq_set_color_temp_filter(); // non-oled set use this funciton.

                                }

                                V4L2printf_Info( "COLORTEMP_DATA R_gain =%d,G_gain=%d,B_gain=%d\n",
                                        curColorTemp.Red_Gain,
                                        curColorTemp.Green_Gain,
                                        curColorTemp.Blue_Gain
                                );
                                V4L2printf_Info("COLORTEMP_DATA R_offset =%d,G_offset=%d,B_offset=%d\n",
                                        curColorTemp.Red_Offset,
                                        curColorTemp.Green_Offset,
                                        curColorTemp.Blue_Offset
                                );

                        }
                }
                break;
                case V4L2_CID_EXT_VPQ_GAMUT_3DLUT:
                {

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {
                                //int i=0;

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_from_user((void *)&gamutLut, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_vpq_gamut_lut)))
                                {

                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user gamutLut \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_from_user((void *)&g_buf3DLUT_LGDB.pt, to_user_ptr(gamutLut.p_section_data), sizeof(GAMUT_3D_LUT_PT_T)*4913))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                //rtd_pr_vpq_emerg("rord 2 3d lut lut_versio=%d",gamutLut.lut_version );

                               // for(i=0;i<100;i++)
                                      // {
                               // rtd_pr_vpq_emerg("rord 3 lut_R2[%d]=%d",i,g_buf3DLUT_LGDB.pt[i].r_data);
                              //  rtd_pr_vpq_emerg("rord 3 lut_G2[%d]=%d",i,g_buf3DLUT_LGDB.pt[i].g_data);
                               // rtd_pr_vpq_emerg("rord 3 lut_B2[%d]=%d",i,g_buf3DLUT_LGDB.pt[i].b_data);

                                  //      }


				if  (vpq_set_Gamut_3D_Lut_LGDB(&g_buf3DLUT_LGDB, 1, 0) < 0) {
					DD_3DLUT_reWrite_CNT = 1;
					rtd_pr_vpq_emerg("DD_3dLUT_DMA, re-Write active\n");
				} else {
					DD_3DLUT_reWrite_CNT = 0;
				}

                         }

                }
                break;
                case V4L2_CID_EXT_VPQ_OD_TABLE:
                {
					if (strcmp(webos_strToolOption.eBackLight, "oled") == 0)
					{
						rtd_pr_vpq_err("V4L2_CID_EXT_VPQ_OD_TABLE, od not support OLED panel\n");
						return -1;
					}

					if(!ext_control.ptr)
					{
						rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
						ret = -EFAULT;
						return ret;
					}
					else
					{
						unsigned char *od_table_from_user = NULL;

						if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
						{
							rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
							ret = -EFAULT;
							return ret;
						}
						od_table_from_user = (unsigned char *)kmalloc(OD_table_length*sizeof(UINT8), GFP_KERNEL);
						if(od_table_from_user == NULL)
						{
							rtd_pr_vpq_emerg("func:%s line:%d [error] dvr_malloc fail \r\n",__FUNCTION__,__LINE__);
							ret = -EFAULT;
							return ret;
						}

						if(copy_from_user(&od_table_from_user[0], to_user_ptr(pqData.p_data), OD_table_length*sizeof(unsigned char)))
						{
							rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
							kfree((void *)od_table_from_user);
							ret = -EFAULT;
							return ret;
						}
						else
						{
							extern unsigned char bODInited;
							extern unsigned char bODTableLoaded;
							extern unsigned char bODPreEnable;
							extern char od_table_mode_store;
							extern unsigned char od_delta_table_g[OD_table_length];
							extern unsigned char od_target_table_g[OD_table_length];

							if(OD_table_length == 289)
							{
								fwif_color_od_table_transform(0, 0, od_table_from_user, od_delta_table_g);
								drvif_color_od_table_17x17_set(od_delta_table_g, 0, 3); // 0:delta mode, 3:rgb channels
							}
							else if(OD_table_length == 1089)
							{
								#if 0 // k7 bring up work around
								fwif_color_od_table_interpolation(od_table_from_user, od_target_table_g);
								fwif_color_od_table_transform(0, 0, od_target_table_g, od_delta_table_g);
								drvif_color_od_table_33x33_set(od_delta_table_g, 0, 11); // 0:delta mode, 11:rgb channels for sram 0~5
								#else
								fwif_color_od_table_transform(0, 0, od_table_from_user, od_delta_table_g);
								drvif_color_od_table_33x33_set(od_delta_table_g, 0, 3); // 0:delta mode, 3:rgb channels
								#endif
							}
							rtd_printk(KERN_EMERG,"VPQ_OD", "%s, od_delta_table_g updated\n", __func__);

							od_table_mode_store = 1;// 0:target mode 1:delta mode
							bODTableLoaded = TRUE;
							kfree((void *)od_table_from_user);
							od_table_from_user = NULL;

							/*Enable OD*/
							bODPreEnable = TRUE;
							if (!bODInited || !bODTableLoaded) {
								ret = -EFAULT;
								return ret;
							}
							else {
								fwif_color_set_od(TRUE);
								ret = 0;
							}
						}
					}
				}
				break;

		case V4L2_CID_EXT_VPQ_OD_EXTENSION:
		{
			struct v4l2_ext_vpq_od_extension odExtInfo;
			unsigned int i = 0;

			if(!ext_control.ptr)
			{
				rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
				ret = -EFAULT;
				return ret;
			}
			else
			{
				if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
				{
					rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
					ret = -EFAULT;			
					return ret;
				}

				if(copy_from_user((void *)&odExtInfo, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_vpq_od_extension)))
				{
					rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
					ret = -EFAULT;
					return ret;
				}
				else
				{
					rtd_pr_vpq_emerg("[OD_EXTENSION] type=%d, length=%d \n",odExtInfo.ext_type,odExtInfo.ext_length);
					if( odExtInfo.ext_type == 0 ) /*POD*/
					{
						if( odExtInfo.ext_length!= LGE_POD_TBL_LEN )
						{
							rtd_pr_vpq_err("[OD_EXTENSION]POD LUT length = %d error, expect = %d\n", odExtInfo.ext_length, LGE_POD_TBL_LEN);
							ret = -1;
						}
						else
						{
							unsigned char* pLUT = NULL;
							pLUT = (unsigned char *)kmalloc(LGE_POD_TBL_LEN*sizeof(unsigned char), GFP_KERNEL);
							if(pLUT == NULL)
							{
								rtd_pr_vpq_err("[OD_EXTENSION][ERROR] VPQ_IOC_SET_OdExtentions POD Table allocate fail\n");
								ret = -1;
							}
							else
							{
								if( copy_from_user(pLUT, (int __user *)odExtInfo.p_ext_data, LGE_POD_TBL_LEN*sizeof(UINT8)) )
								{
									rtd_pr_vpq_err("[OD_EXTENSION] kernel copy VPQ_IOC_SET_OdExtentions_pData(POD) fail\n");
									ret = -1;
								}
								else
								{
									rtd_pr_vpq_info("[OD_EXTENSION] POD check, panel maker = %d\n", Get_PANEL_MAKER());

									if ((Get_PANEL_MAKER() == KADP_DISP_LCD_PANEL_MAKER_SHARP) || (Get_PANEL_MAKER() == KADP_DISP_LCD_PANEL_MAKER_INNOLUX))
									{
										rtd_pr_vpq_emerg("[OD_EXTENSION] Don't enable POD, panel maker wrong\n");
	                                    kfree((void*)pLUT);
	                                    return -1;
									}

									drvif_color_enable_LGD_POD_CLK();
									drvif_color_enable_LGD_POD_double_buffer();
									fwif_color_set_LGD_POD(pLUT);
									g_LGE_POD_Enable_flag = 1;
									drvif_color_set_LGD_POD_enable(1);
									ret = 0;
								}
								kfree((void*)pLUT);
							}
						}
					}
					else if( odExtInfo.ext_type == 1 ) /*LODC TblSize 17x17 Region 4x4*/
					{
						const unsigned int CMD_SINGLE_TBL_LEN = 17*17;
						const unsigned int CMD_LENGTH = CMD_SINGLE_TBL_LEN*16;
						if( odExtInfo.ext_length != CMD_LENGTH )
						{
							rtd_pr_vpq_err("[OD_EXTENSION] LODC LUT length = %d error, expect = %d\n", odExtInfo.ext_length, CMD_LENGTH);
							ret = -1;
						}
						else
						{
							unsigned char* pLUT = NULL;
							sys_reg_sys_clken3_RBUS sys_reg_sys_clken3_reg;
							pLUT = (unsigned char *)vmalloc(CMD_LENGTH*sizeof(unsigned char));

							if(pLUT == NULL)
							{
								rtd_pr_vpq_err("[OD_EXTENSION][ERROR] VPQ_IOC_SET_OdExtentions LODC Table allocate fail\n");
								ret = -1;
							}
							else
							{
								if( copy_from_user(pLUT, (int __user *)odExtInfo.p_ext_data, CMD_LENGTH*sizeof(unsigned char)) )
								{
									rtd_pr_vpq_err("[OD_EXTENSION] kernel copy VPQ_IOC_SET_OdExtentions_pData(LODC) fail\n");
									ret = -1;
								}
								else
								{
									drvif_color_set_Tcon_pcid1_pcid2_LGPOD_fcic_enable(0,0,0,0);
									sys_reg_sys_clken3_reg.regValue = 0;
									sys_reg_sys_clken3_reg.clken_disp_lg_hcic = 1;
									IoReg_Write32(SYS_REG_SYS_CLKEN3_reg, sys_reg_sys_clken3_reg.regValue);

									// force unused col/row bound setting to zero
									for(i=0; i<17; i++)
									{
										pcidRowBnd[i] = 0;
										pcidColBnd[i] = 0;
									}

									// start to write settings
									fwif_color_set_pcid2_data_setting(_PCID_DATA_LGE);
									fwif_color_set_pcid2_pixel_reference_setting(_PCID_PXLREF_LGE);
									fwif_color_set_pcid_RgnTable_Setting_TV006(_PCID_RGN_17x17_4x4, pcidRowBnd, pcidColBnd);

									fwif_color_refresh_pcid_RgnTable();
									for(i=0;i<16;i++)
									{
										/* pcid_Transpose is different between K6(8bit) and K7(16bit) */
										//fwif_color_pcid_Transpose_TV006(pLUT+(i*CMD_SINGLE_TBL_LEN*sizeof(unsigned char)), 17, TransTbl);
										fwif_color_set_pcid_RgnTableValue(TransTbl, i, _PCID_COLOR_ALL);
									}
									drvif_color_set_Tcon_pcid1_pcid2_LGPOD_fcic_enable(0,1,0,0);
									ret = 0;
								}
								vfree((void*)pLUT);
							}
						}
					}
					else if( odExtInfo.ext_type == 2 ) /*Disable*/
					{
						ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
						display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
						drvif_color_set_Tcon_pcid1_pcid2_LGPOD_fcic_enable(0,0,0,display_timing_ctrl2_reg.hcic_enable);
						ret = 0;
					}
					else if( odExtInfo.ext_type == 3 ) /*LODC TblSize 19x19 Region 1x10*/
					{
						const unsigned int CMD_SINGLE_TBL_LEN = 19 * 19;
						const unsigned int CMD_LENGTH = (CMD_SINGLE_TBL_LEN * 10) + 17 + 17;
						if( odExtInfo.ext_length < CMD_LENGTH )
						{
							rtd_pr_vpq_err("[OD_EXTENSION] LODC LUT length = %d error, expect = %d\n", odExtInfo.ext_length, CMD_LENGTH);
							ret = -1;
						}
						else
						{
							unsigned short* pLUT = NULL;
							sys_reg_sys_clken3_RBUS sys_reg_sys_clken3_reg;
							pLUT = (unsigned short *)vmalloc(CMD_LENGTH*sizeof(unsigned short));
							//rtd_pr_vpq_info("[LINE OD][%d][%d]\n", CMD_LENGTH, sizeof(unsigned short));

							if(pLUT == NULL)
							{
								rtd_pr_vpq_err("[OD_EXTENSION][ERROR] VPQ_IOC_SET_OdExtentions LODC Table allocate fail\n");
								ret = -1;
							}
							else
							{
								if( copy_from_user(pLUT, (int __user *)odExtInfo.p_ext_data, CMD_LENGTH*sizeof(unsigned short)) )
								{
									rtd_pr_vpq_err("[OD_EXTENSION] kernel copy VPQ_IOC_SET_OdExtentions_pData(LODC) fail\n");
									ret = -1;
								}
								else
								{
									drvif_color_set_Tcon_pcid1_pcid2_LGPOD_fcic_enable(0,0,0,0);
									sys_reg_sys_clken3_reg.regValue = 0;
									sys_reg_sys_clken3_reg.clken_disp_lg_hcic = 1;
									IoReg_Write32(SYS_REG_SYS_CLKEN3_reg, sys_reg_sys_clken3_reg.regValue);

									// parse the col/row bound setting from data
									for(i = 0; i < 17; i++)
									{
										pcidRowBnd[i] = (*(pLUT + CMD_SINGLE_TBL_LEN * 10 + i));
										pcidColBnd[i] = (*(pLUT + CMD_SINGLE_TBL_LEN * 10 + 17 + i));
										rtd_pr_vpq_info("[LINE OD][%d] pcidRowBnd = [%d], pcidColBnd = [%d]\n", i, pcidRowBnd[i], pcidColBnd[i]);
									}

									// start to write settings
									fwif_color_set_pcid2_data_setting(_PCID_DATA_LGE);
									fwif_color_set_pcid2_pixel_reference_setting(fwif_color_get_TV006_LineOD_gate_table_index());
									fwif_color_set_pcid_RgnTable_Setting_TV006(_PCID_RGN_19x19_1x10, pcidRowBnd, pcidColBnd);
									fwif_color_refresh_pcid_RgnTable();
									for(i = 0; i < 10; i++)
									{
										fwif_color_pcid_Transpose_TV006(pLUT + (i * CMD_SINGLE_TBL_LEN), 19, TransTbl);
										fwif_color_set_pcid_RgnTableValue(TransTbl, i, _PCID_COLOR_ALL);
									}
									drvif_color_set_Tcon_pcid1_pcid2_LGPOD_fcic_enable(0,1,0,0);
									ret = 0;
								}
								vfree((void*)pLUT);
							}
						}
					}
					else
					{
						rtd_pr_vpq_err("[OD_EXTENSION] OD Extend type error = %d error\n", odExtInfo.ext_type);
						ret = -1;
					}
				}
			}
		}
		break;
                case V4L2_CID_EXT_VPQ_LOCALCONTRAST_DATA:
                {

                       // struct v4l2_ext_vpq_localcontrast_data local_contrast;
                       // CHIP_LOCAL_CONTRAST_T lc_param;

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {
							if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
							{
								rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
								ret = -EFAULT;
								return ret;
							}

							if(copy_from_user((void *)&glc_param, to_user_ptr(pqData.p_data), sizeof(CHIP_LOCAL_CONTRAST_T)))
							{
								rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
								ret = -EFAULT;
								return ret;
							}

							//  rtd_pr_vpq_emerg("LOCALCONTRAST_DATA slope_unit=%d,ui=%d \n",
							//           lc_param.LC_tmap_slope_unit,local_contrast.ui_value);


							/*rtd_pr_vpq_emerg("rord lc Comp_En :%d,LC_Tone_Mapping_En: %d,LC_tmap_slope_unit:%d,LC_tmap_blend_factor:%d \n",
							                                glc_param.M_LC_Comp_En,
							                                glc_param.LC_Tone_Mapping_En,
							                                glc_param.LC_tmap_slope_unit,
							                                glc_param.LC_tmap_blend_factor);
							*/

							rtd_pr_vpq_info("Set LC through V4L2_CID_EXT_VPQ_LOCALCONTRAST_DATA\n" );
							fwif_color_set_LocalContrast_table_TV006(&glc_param);

                        }
                }
                break;
                case V4L2_CID_EXT_VPQ_GAMUT_MATRIX_PRE: // this driver have to discuss with LG
                {

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {

                                    rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                    ret = -EFAULT;
                                    return ret;
                                }

                                if(copy_from_user((void *)v4l2_vpq_MATRIX_PRE, to_user_ptr(pqData.p_data), sizeof(v4l2_vpq_MATRIX_PRE)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }
                                else
                                {

                                        unsigned char i;

                                	for (i=0;i<9;i++) {

                                                gamut33.Gamut33Matrix[i]= (short)v4l2_vpq_MATRIX_PRE[i]*16384/10000;
                                                V4L2printf_Info( "GAMUT_MATRIX_PRE idx[%d] =%d  \n",i,gamut33.Gamut33Matrix[i]);
                                	}
                                        fwif_HDR_RGB2OPT(1, gamut33.Gamut33Matrix);


                                }

                        }

                }
                break;
                case V4L2_CID_EXT_VPQ_GAMUT_MATRIX_POST:
                {

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_err("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_err("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_from_user((void *)&gamutPost, to_user_ptr(pqData.p_data), sizeof(struct v4l2_vpq_gamut_post)))
                                {
                                        rtd_pr_vpq_err("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }
                                else
                                {
                                        unsigned char invgamma_enable = 0;
                                        short gamutMatrix[3][3];
                                        invgamma_enable = ((gamutPost.degamma & 0x1)<<1) + (gamutPost.gamma & 0x1);

                                        rtd_pr_vpq_info("gamutPost.gamma=%d,gamutPost.degamma=%d,\n",
											gamutPost.gamma, gamutPost.degamma);

                                        if (g_InvGammaPowerMode != invgamma_enable) {
                                                g_InvGammaPowerMode = invgamma_enable;

                                                switch(g_InvGammaPowerMode)
                                                {
                                                        case VPQ_GAMMA_MODE_INV_OFF_INNER_BYPASS:
                                                        default:
                                                                drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 0, 1);
                                                                drvif_color_inv_gamma_enable(SLR_SUB_DISPLAY, 0, 1);
                                                                break;
                                                        case VPQ_GAMMA_MODE_INV_OFF_INNER_ON:
                                                                drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 0, 1);
                                                                drvif_color_inv_gamma_enable(SLR_SUB_DISPLAY, 0, 1);
                                                                break;
                                                        case VPQ_GAMMA_MODE_INV_ON_INNER_BYPASS:
                                                                drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 1, 1);
                                                                drvif_color_inv_gamma_enable(SLR_SUB_DISPLAY, 1, 1);
                                                                break;
                                                        case VPQ_GAMMA_MODE_INV_ON_INNER_ON:
                                                                drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 1, 1);
                                                                drvif_color_inv_gamma_enable(SLR_SUB_DISPLAY, 1, 1);
                                                                break;
                                                }
                                                vpq_set_gamma(1, 1);
                                        }

                                        gamutMatrix[0][0] = gamutPost.matrix[0]*2048/10000;
                                        gamutMatrix[0][1] = gamutPost.matrix[1]*2048/10000;
                                        gamutMatrix[0][2] = gamutPost.matrix[2]*2048/10000;
                                        gamutMatrix[1][0] = gamutPost.matrix[3]*2048/10000;
                                        gamutMatrix[1][1] = gamutPost.matrix[4]*2048/10000;
                                        gamutMatrix[1][2] = gamutPost.matrix[5]*2048/10000;
                                        gamutMatrix[2][0] = gamutPost.matrix[6]*2048/10000;
                                        gamutMatrix[2][1] = gamutPost.matrix[7]*2048/10000;
                                        gamutMatrix[2][2] = gamutPost.matrix[8]*2048/10000;

                                        V4L2printf_Info("gamutPost.matrix[0]%d\n",gamutPost.matrix[0]);
                                        V4L2printf_Info("gamutPost.matrix[1]%d\n",gamutPost.matrix[1]);
                                        V4L2printf_Info("gamutPost.matrix[2]%d\n",gamutPost.matrix[2]);
                                        V4L2printf_Info("gamutPost.matrix[3]%d\n",gamutPost.matrix[3]);
                                        V4L2printf_Info("gamutPost.matrix[4]%d\n",gamutPost.matrix[4]);
                                        V4L2printf_Info("gamutPost.matrix[5]%d\n",gamutPost.matrix[5]);
                                        V4L2printf_Info("gamutPost.matrix[6]%d\n",gamutPost.matrix[6]);
                                        V4L2printf_Info("gamutPost.matrix[7]%d\n",gamutPost.matrix[7]);
                                        V4L2printf_Info("gamutPost.matrix[8]%d\n",gamutPost.matrix[8]);

                                        V4L2printf_Info("gamutMatrix[0][0]%d\n",gamutMatrix[0][0]);
                                        V4L2printf_Info("gamutMatrix[0][1]%d\n",gamutMatrix[0][1]);
                                        V4L2printf_Info("gamutMatrix[0][2]%d\n",gamutMatrix[0][2]);
                                        V4L2printf_Info("gamutMatrix[1][0]%d\n",gamutMatrix[1][0]);
                                        V4L2printf_Info("gamutMatrix[1][1]%d\n",gamutMatrix[1][1]);
                                        V4L2printf_Info("gamutMatrix[1][2]%d\n",gamutMatrix[1][2]);
                                        V4L2printf_Info("gamutMatrix[2][0]%d\n",gamutMatrix[2][0]);
                                        V4L2printf_Info("gamutMatrix[2][1]%d\n",gamutMatrix[2][1]);
                                        V4L2printf_Info("gamutMatrix[2][2]%d\n",gamutMatrix[2][2]);

                                        //fwif_colorMap_sRGB_SetMatrix_TV006(SLR_MAIN_DISPLAY, sRGBMatrix, g_srgbForceUpdate);
                                        fwif_set_Gamut_Matrix_TV006(SLR_MAIN_DISPLAY, gamutMatrix);

                                }

                        }

                }
                break;
                case V4L2_CID_EXT_VPQ_PQ_MODE_INFO:
                {

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }


                                if(copy_from_user((void *)&pqModeInfo, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_pq_mode_info_RTK)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }
                                else
                                {
						PQModeInfo_flag[0] = pqModeInfo.hdrStatus;
						PQModeInfo_flag[1] = pqModeInfo.colorimetry;

						pr_info("[HDR new flow][VPQ HAL], type = %d, MODE_STATE=%d\n",
							PQModeInfo_flag[0], Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE));

						if (get_support_vo_force_v_top(support_sdr_max_rgb) && (PQModeInfo_flag[0] == HAL_VPQ_HDR_MODE_SDR))
							PQModeInfo_flag[0] = HAL_VPQ_HDR_MODE_SDR_MAX_RGB;
#if 0
						if (PQModeInfo_flag[0] == HAL_VPQ_HDR_MODE_HDR10)
                                                rtd_pr_vpq_info("\n HDR_MODE, PQModeInfo_flag :HAL_VPQ_HDR_MODE_HDR10 \n");
                                        else if (PQModeInfo_flag[0] == HAL_VPQ_HDR_MODE_HLG)
                                                rtd_pr_vpq_info("\nHDR_MODE, PQModeInfo_flag :HAL_VPQ_HDR_MODE_HLG \n");
                                        else
                                                rtd_pr_vpq_info("\n HDR_MODE, PQModeInfo_flag :HAL_VPQ_HDR_MODE_SDR \n");
#endif
						if (PQModeInfo_flag[1] == 0) PQModeInfo_flag[1] = 2;    //data protect
							g_LGE_HDR_CSC_CTRL = PQModeInfo_flag[1];

                #ifndef CONFIG_HDR_SDR_SEAMLESS /* seamless need to check force bg and mode active*/
						drvif_TV006_SET_HDR_mode(PQModeInfo_flag);
						Scaler_SetDataFormatHandler();
                #else
						if (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)
						{
							//rtd_pr_vpq_info("\nHDR_MODE, PQModeInfo: seamless flow\n");
							/* seamless. do seamless process in isr (void Scaler_hdr_setting_SEAMLESS()), avoid transient noise*/;
							//Scaler_Set_HDR_YUV2RGB(PQModeInfo_flag[0], PQModeInfo_flag[1]);
						} else {
							//rtd_pr_vpq_info("\nHDR_MODE, PQModeInfo: scaler flow\n");
							//Scaler_Set_HDR_YUV2RGB(PQModeInfo_flag[0], PQModeInfo_flag[1]);
							//drvif_TV006_SET_HDR_mode(PQModeInfo_flag);
							//Scaler_SetDataFormatHandler();
						}
						HDR_SDR_SEAMLESS_PQ.HDR_SDR_SEAMLESS_PQ_SET[HDR_SDR_SEAMLESS_PQ_HAL_PQModeInfo]=1;
                #endif

                                }
                        }

                }
                break;
                case V4L2_CID_EXT_VPQ_DECONTOUR:
                {

                        CHIP_DECONTOUR_T De_Contour_Table;

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_from_user((void *)&de_contour, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_vpq_decontour_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;

                                }

                                if(de_contour.pst_chip_data ==NULL)
                                {
                                        rtd_pr_vpq_emerg("LG DB NULL, workaround  V4L2_CID_EXT_VPQ_DECONTOUR \n" );


                                        if( de_contour.ui_value ==0)
                                        {
                                                tv006_decontour_level = 0;

                                        }
                                        else if ( de_contour.ui_value ==1)
                                        {

                                                tv006_decontour_level = 6;
                                        }
                                        else if ( de_contour.ui_value ==2)
                                        {
                                                tv006_decontour_level = 10;

                                        }
                                        else if ( de_contour.ui_value ==3)
                                        {
                                                tv006_decontour_level = 15;

                                        }

                                        return ret;
                                }


                                if(copy_from_user((void *)&De_Contour_Table, to_user_ptr(de_contour.pst_chip_data), sizeof(De_Contour_Table)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                memcpy((unsigned char*)&De_contour_level.De_Contour_Table,(unsigned char*)&De_Contour_Table,sizeof( CHIP_DECONTOUR_T));
                                De_contour_level.UI_Lv = de_contour.ui_value;

                                rtd_pr_vpq_emerg("DECONTOUR Idecontour_level=%d,ui=%d \n",
                                        De_Contour_Table.Idecontour_level,de_contour.ui_value);

                                tv006_decontour_level = De_contour_level.De_Contour_Table.Idecontour_level;
                                fwif_color_Set_De_Contour_tv006(&De_contour_level);
                                g_bDeContourSaved = 1;

                        }
                }
                break;


		case V4L2_CID_EXT_HDR_3DLUT:
		{
			if(!ext_control.ptr)
			{
				rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
				ret = -EFAULT;
				return ret;
			}
			else
			{
				if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
				{
					rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
					ret = -EFAULT;
					return ret;
				}
				if(copy_from_user((void *)&st3dlut, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_hdr_3dlut_RTK)))
				{
					printk(KERN_EMERG"func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
					ret = -EFAULT;
					return ret;
				}

#ifdef isSupport_HDR_3dLUT_24x24x24
				if (st3dlut.data_size != HDR_24x24x24_size*3*sizeof(short)) {	// 24*24*24*3*sizeof(short)
					printk(KERN_EMERG"size is not correct, set for 24x24x24 st3dlut.data_size = %d, \r\n", st3dlut.data_size);
					ret = -EFAULT;
					return ret;
				}
#else
				if (st3dlut.data_size != VIP_DM_HDR_3D_LUT_UI_TBL_SIZE*sizeof(short)) {	// 17*17*17*3*sizeof(short)
					printk(KERN_EMERG"size is not correct, set for 17x17x17 st3dlut.data_size = %d, \r\n", st3dlut.data_size);
					ret = -EFAULT;
					return ret;
				}
#endif
				if(copy_from_user((void *)&phdr3dlut16.hdr_3dlut, to_user_ptr(st3dlut.p3dlut), st3dlut.data_size))
				{
					rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
					ret = -EFAULT;
					return ret;
				}
				else
				{
					if (vip_sysInfo == NULL) {
						rtd_pr_vpq_emerg("func:%s line:%d [error] vip_sysInfo=NULL \r\n",__FUNCTION__,__LINE__);
						ret = -EFAULT;
						return ret;
					}

					if (((v4l2_blockbypass.bypassMask & V4L2_VPQ_BYPASS_MASK_HDR_EXCEPT_PCC_RTK) != 0) && (v4l2_blockbypass.bOnOff == 1)) {
						rtd_pr_vpq_emerg("V4L2_CID_EXT_HDR_3DLUT skip, bypassMask=%x, onOff=%d,\n", v4l2_blockbypass.bypassMask, v4l2_blockbypass.bOnOff);
						ret = -EFAULT;
						return ret;
					}

					if (vip_sysInfo->DolbyHDR_flag == 0) {
						static PQ_HDR_3dLUT_16_PARAM args;
						args.c3dLUT_16_enable = 1;
						args.a3dLUT_16_pArray = (UINT16*)(void __user *)&(phdr3dlut16.hdr_3dlut);
						drvif_Set_DM_HDR_CLK(); // enable DM & Composer clock
#ifdef FREEZE_FOR_SEAMLESS_BY_V4L2
						if (vpq_v4l2_hdrtype_compare_in_active(st3dlut.hdr_mode, vip_sysInfo->HDR_flag) == 1) {
							rtd_pr_vpq_info("DM_freeze, freeze flag CSC3,\n");
							RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Status] = 2; /*  set freeze flag, freez in isr*/
						}
#else
						if (vpq_v4l2_hdrtype_compare(st3dlut.hdr_mode, vip_sysInfo->HDR_flag) == 1) {
							CSC3_LUT_reLoad_Flag = 1;
							DM_DMA_mode = 2;
						} else {
							CSC3_LUT_reLoad_Flag = 0;
							DM_DMA_mode = 1;
						}
#endif
						if (st3dlut.hdr_mode != CSC3_HDR_flag_v4l2) {
							CSC3_HDR_flag_v4l2 = st3dlut.hdr_mode;
							rtd_pr_vpq_info("DM_3dLUT_DMA, mode=%d,\n", CSC3_HDR_flag_v4l2);
						}
#ifdef isSupport_HDR_3dLUT_24x24x24
						if (fwif_color_set_DM_HDR_3dLUT_24x24x24_16_TV006(&args, DM_DMA_mode)<0)
#else
						if (fwif_color_set_DM_HDR_3dLUT_17x17x17_16_TV006(&args, DM_DMA_mode)<0)
#endif							
							CSC3_LUT_reLoad_Flag = 1;
						else
							CSC3_LUT_reLoad_Flag = 0;					
					}
				}
			}
		}
		break;
		case V4L2_CID_EXT_HDR_EOTF:
		{

			if(!ext_control.ptr)
			{
				rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
				ret = -EFAULT;
				return ret;
			}
			else
			{

				if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
				{
					rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
					ret = -EFAULT;
					return ret;
				}

				if(copy_from_user((void *)Eotf_table32, to_user_ptr(pqData.p_data), sizeof(UINT32)*OETF_SIZE_TV006))
				{
					rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
					ret = -EFAULT;
					return ret;
				}

				if (vip_sysInfo == NULL) {
					rtd_pr_vpq_emerg("func:%s line:%d [error] vip_sysInfo=NULL \r\n",__FUNCTION__,__LINE__);
					ret = -EFAULT;
					return ret;
				}

				if (((v4l2_blockbypass.bypassMask & V4L2_VPQ_BYPASS_MASK_HDR_EXCEPT_PCC_RTK) != 0) && (v4l2_blockbypass.bOnOff == 1)) {
						rtd_pr_vpq_emerg("V4L2_CID_EXT_HDR_EOTF skip, bypassMask=%x, onOff=%d,\n", v4l2_blockbypass.bypassMask, v4l2_blockbypass.bOnOff);
						ret = -EFAULT;
						return ret;
				}

				if (vip_sysInfo->DolbyHDR_flag == 0) {
					Eotf_table32[EOTF_size - 1] = 0xffffffff;
#if 0  // control by HFC in v4l2
					if (vpq_v4l2_DM2_EOTF_Check(&Eotf_table32[0]) == 1)
						fwif_color_set_DM2_EOTF_TV006(NULL, 1);
					else
						fwif_color_set_DM2_EOTF_TV006(Eotf_table32, 1);
#endif
				}
			}
		}
		break;

		case V4L2_CID_EXT_HDR_OETF:
		case V4L2_CID_EXT_HDR_HLG_Y_GAIN_TBL:
		{
			if(!ext_control.ptr)
			{
				rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
				ret = -EFAULT;
				return ret;
			}
			else
			{
				if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
				{

					rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
					ret = -EFAULT;
					return ret;
				}

				size_byte = pqData.length;
				if ((size_byte != (sizeof(short)*OETF_PROG_SIZE_TV006)) && (size_byte != (sizeof(short)*OETF_SIZE_TV006))) {
					rtd_pr_vpq_emerg("func:%s line:%d [error] lenth is not correct,size_byte=%d, \r\n",__FUNCTION__,__LINE__, size_byte);
					ret = -EFAULT;
					return ret;
				}

				if(copy_from_user((void *)Oetf_table16, to_user_ptr(pqData.p_data), size_byte))
				{
					rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
					ret = -EFAULT;
					return ret;
				}
				else
				{

					if (vip_sysInfo == NULL) {
						rtd_pr_vpq_emerg("func:%s line:%d [error] vip_sysInfo=NULL \r\n",__FUNCTION__,__LINE__);
						ret = -EFAULT;
						return ret;
					}

					if (((v4l2_blockbypass.bypassMask & V4L2_VPQ_BYPASS_MASK_HDR_EXCEPT_PCC_RTK) != 0) && (v4l2_blockbypass.bOnOff == 1)) {
							rtd_pr_vpq_emerg("Y_Gain skip, bypassMask=%x, onOff=%d,\n", v4l2_blockbypass.bypassMask, v4l2_blockbypass.bOnOff);
							ret = -EFAULT;
							return ret;
					}

					if (vip_sysInfo->DolbyHDR_flag == 0) {
#ifdef FREEZE_FOR_SEAMLESS_BY_V4L2
						if ((vip_sysInfo->HDR_flag != HAL_VPQ_HDR_MODE_HLG) &&
							(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)) {
							rtd_pr_vpq_info("DM_freeze, freeze flag Y gain,\n");
							RPC_SysInfo->HDR_info.Ctrl_Item[TV006_HDR_Seamless_Freeze_Status] = 2; /*  set freeze flag, freez in isr*/
						}
#else
						if (vip_sysInfo->HDR_flag != HAL_VPQ_HDR_MODE_HLG) {
							DM_DMA_mode = 2;
							OETF_LUT_reLoad_Flag = 1;
						} else {
							DM_DMA_mode = 1;
							OETF_LUT_reLoad_Flag = 0;
						}
#endif
						if (size_byte == sizeof(short)*OETF_PROG_SIZE_TV006) {	//   programmable index table
							rtd_pr_vpq_info("DM_OETF_DMA, PID, DM_DMA_mode=%d\n", DM_DMA_mode);

							fwif_color_set_DM2_OETF_Set_Prog_Index(1);
							memcpy(&Oetf_table16_prog[0], &Oetf_table16[0], sizeof(short)*OETF_PROG_SIZE_TV006);

							fwif_color_set_DM2_OETF_Conv_TV006(&Oetf_table16_prog[0], &Oetf_table16[0], 1);
                                                } else {
							rtd_pr_vpq_info("DM_OETF_DMA, 1025, DM_DMA_mode=%d,\n", DM_DMA_mode);
							fwif_color_set_DM2_OETF_Set_Prog_Index(0);

						}
						Oetf_table16[OETF_size - 1] = 0xffff;
						//UINT16 i;
						//for (i = 0; i < EOTF_size; i++)
						//printk("Eotf[%d] = %d\n", i, Eotf_table32[i]);
						//if (vpq_v4l2_DM2_OETF_Check(&Oetf_table16[0]) == 1)
							//fwif_color_set_DM2_OETF_TV006(NULL, 1);
						//else
							fwif_color_set_DM2_OETF_TV006(Oetf_table16, DM_DMA_mode);
					}

				}

			}
		}
		break;
		case V4L2_CID_EXT_HDR_TONEMAP:
		{
#ifdef isSupport_DM2_TM
			if(!ext_control.ptr)
			{
				rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
				ret = -EFAULT;
				return ret;
			}
			else
			{
				if (get_platform() != PLATFORM_K6HP) {	// only k6hp can use tone maping
					ret = -EFAULT;
					return ret;
				}

				if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
				{
					rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
					ret = -EFAULT;
					return ret;
				}

				if(copy_from_user((void *)TM_table32, to_user_ptr(pqData.p_data), sizeof(UINT32)*HDR_TONEMAP_SIZE_TV006))
				{
					rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
					ret = -EFAULT;
					return ret;
				}

				if (vip_sysInfo == NULL) {
					rtd_pr_vpq_emerg("func:%s line:%d [error] vip_sysInfo=NULL \r\n",__FUNCTION__,__LINE__);
					ret = -EFAULT;
					return ret;
				}

				if (((v4l2_blockbypass.bypassMask & V4L2_VPQ_BYPASS_MASK_HDR_EXCEPT_PCC_RTK) != 0) && (v4l2_blockbypass.bOnOff == 1)) {
						rtd_pr_vpq_emerg("ToneMapping skip, bypassMask=%x, onOff=%d,\n", v4l2_blockbypass.bypassMask, v4l2_blockbypass.bOnOff);
						ret = -EFAULT;
						return ret;
				}

				if (vip_sysInfo->DolbyHDR_flag == 0) {
					TM_table32[ToneMapping_size - 1] = 0xffffffff;
					for (i=0;i<ToneMapping_size;i++)
						TM_table12[i] = TM_table32[i]>>20;
					fwif_color_set_DM2_ToneMapping_TV006(TM_table12);
				}
			}
#endif
		}
		break;
                case V4L2_CID_EXT_MEMC_MOTION_COMP:
                {

                        VPQ_MEMC_TYPE_T motion;

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {
                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user pqData fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_from_user((void *)&motion_comp_info, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_memc_motion_comp_info)))
                                {

                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user motion_comp_info fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;

                                }
                                else
                                {
                                        V4L2printf_Info("blur_level=%d,judder_level=%d,memc_type =%d \n",
                                                motion_comp_info.blur_level,
                                                motion_comp_info.judder_level,
                                                motion_comp_info.memc_type);

                                        motion = motion_comp_info.memc_type;
                                        ret = HAL_VPQ_MEMC_SetMotionComp(motion_comp_info.blur_level, motion_comp_info.judder_level, motion);

                                }

                        }
                }
                break;
                case V4L2_CID_EXT_LED_INIT:
                {
                        struct v4l2_ext_led_panel_info stRealInfo;
                        unsigned char src_idx = 0;
                        unsigned char TableIdx = 0;
                        HAL_LED_PANEL_INFO_T PANEL_INFO_T;

                        if (PQ_LED_DEV_UNINIT == PQ_LED_Dev_Status || PQ_LED_DEV_NOTHING == PQ_LED_Dev_Status) {
				PQ_LED_Dev_Status = PQ_LED_DEV_INIT_DONE;
			}
                        if(!ext_control.ptr) {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        } else {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user pqData fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_from_user((void *)&stRealInfo, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_led_panel_info)))
                                {

                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user stRealInfo fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;

                                }
                                else
                                {
                                        rtd_pr_vpq_emerg("rord panel_inch=%d,backlight_type=%d,bar_type =%d,module_maker=%d,local_dim_ic_type=%d,panel_type =%d \n",
                                                stRealInfo.panel_inch,
                                                stRealInfo.backlight_type,
                                                stRealInfo.bar_type,
                                                stRealInfo.module_maker,
                                                stRealInfo.local_dim_ic_type,
                                                stRealInfo.panel_type
                                        );

                                        PANEL_INFO_T.hal_inch = stRealInfo.panel_inch;
                                        PANEL_INFO_T.hal_bl_type = stRealInfo.backlight_type;
                                        PANEL_INFO_T.hal_bar_type = stRealInfo.bar_type;
                                        PANEL_INFO_T.hal_maker = stRealInfo.module_maker;
                                        PANEL_INFO_T.hal_icType = stRealInfo.local_dim_ic_type;
                                        PANEL_INFO_T.hal_panel_type = stRealInfo.panel_type;

                                        fwif_color_set_LED_Initialize(PANEL_INFO_T);
                                        fwif_color_set_LD_CtrlSPI_init(PANEL_INFO_T); /*HW Dora provided this script*/

                                        fwif_color_set_LD_Global_Ctrl(src_idx, TableIdx);
                                        fwif_color_set_LD_Backlight_Decision(src_idx, TableIdx);
                                        fwif_color_set_LD_Spatial_Filter(src_idx, TableIdx);
                                        fwif_color_set_LD_Spatial_Remap(src_idx, TableIdx);
                                        fwif_color_set_LD_Boost(src_idx,TableIdx);
                                        fwif_color_set_LD_Temporal_Filter(src_idx, TableIdx);
                                        fwif_color_set_LD_Backlight_Final_Decision(src_idx, TableIdx);
                                        fwif_color_set_LD_Data_Compensation(src_idx, TableIdx);
                                        fwif_color_set_LD_Data_Compensation_NewMode_2DTable(src_idx,TableIdx);
                                        fwif_color_set_LD_Backlight_Profile_Interpolation(src_idx, TableIdx);
										if (PANEL_INFO_T.hal_bl_type == HAL_BL_DIRECT_L || PANEL_INFO_T.hal_bl_type == HAL_BL_DIRECT_VI || PANEL_INFO_T.hal_bl_type == HAL_BL_DIRECT_SKY)
											fwif_color_set_LD_BL_Profile_Interpolation_Table_HV(src_idx, (unsigned char)fwif_color_Get_BLPF_type()-BLPF_EDGE_MAX); /*Direct Type Light Profile Table*/
										else {
											if (PANEL_INFO_T.hal_bar_type < LD_Table_NUM)
												fwif_color_set_LD_BL_Profile_Interpolation_Table(src_idx, PANEL_INFO_T.hal_bar_type); /*Edge Typp Light Profile Table*/
										}

                                        fwif_color_set_LD_Demo_Window(src_idx, TableIdx);

					     				drvif_color_set_LDSPI_TxDoneIntEn(TRUE);
                                        rtd_pr_vpq_debug("kernel VPQ_LED_IOC_SET_LD_INIT success\n");
                                        ret = 0;

                                }

                        }
                }
                break;
                case V4L2_CID_EXT_LED_DEMOMODE:
                {
                        struct v4l2_ext_led_ldim_demo_info stRealInfo;

			if (PQ_LED_Dev_Status != PQ_LED_DEV_INIT_DONE)
				return -1;

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user pqData fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }


                                if(copy_from_user((void *)&stRealInfo, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_led_ldim_demo_info)))
                                {

                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user stRealInfo fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;

                                }
                                else
                                {


                                        LED_LDCtrlDemoMode.bCtrl = stRealInfo.bOnOff;
                                        LED_LDCtrlDemoMode.ctrlMode = stRealInfo.eType;

                                        vpqled_HAL_VPQ_LED_LDCtrlDemoMode(LED_LDCtrlDemoMode.ctrlMode, LED_LDCtrlDemoMode.bCtrl);
                                        rtd_pr_vpq_notice("kernel VPQ_LED_LDCtrlDemoMode success\n");
                                        rtd_pr_vpq_emerg("rord bCtrl=%d,ctrlMode=%d \n",stRealInfo.bOnOff,stRealInfo.eType);
                                        ret = 0;

                                }

                        }
                }
                break;
                case V4L2_CID_EXT_LED_DB_DATA:
                {

			unsigned int i,j,k;
                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user pqData fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }


                                if(copy_from_user((void *)&stReal_db_lut, to_user_ptr(pqData.p_data), sizeof(stReal_db_lut)))
                                {

                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user stRealInfo fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;

                                }
                                else
                                {
                                		/*Remapping between RTK driver and TV006 DB*/
                                		for (i=0;i<LD_Table_NUM;i++) {
												LED_DB_LUT[i].LD_UI_Index = stReal_db_lut[i].ui_index;
												/* backlight descision */
												LED_DB_LUT[i].hal_backlight_decision.hal_ld_maxgain = stReal_db_lut[i].maxgain;
												LED_DB_LUT[i].hal_backlight_decision.hal_ld_avegain = stReal_db_lut[i].avrgain;
												LED_DB_LUT[i].hal_backlight_decision.hal_ld_histshiftbit = stReal_db_lut[i].histoshiftbit;
												LED_DB_LUT[i].hal_backlight_decision.hal_ld_hist0gain = stReal_db_lut[i].histogain[0];
												LED_DB_LUT[i].hal_backlight_decision.hal_ld_hist1gain = stReal_db_lut[i].histogain[1];
												LED_DB_LUT[i].hal_backlight_decision.hal_ld_hist2gain = stReal_db_lut[i].histogain[2];
												LED_DB_LUT[i].hal_backlight_decision.hal_ld_hist3gain = stReal_db_lut[i].histogain[3];
												LED_DB_LUT[i].hal_backlight_decision.hal_ld_hist4gain = stReal_db_lut[i].histogain[4];
												LED_DB_LUT[i].hal_backlight_decision.hal_ld_hist5gain = stReal_db_lut[i].histogain[5];
												LED_DB_LUT[i].hal_backlight_decision.hal_ld_hist6gain = stReal_db_lut[i].histogain[6];
												LED_DB_LUT[i].hal_backlight_decision.hal_ld_hist7gain = stReal_db_lut[i].histogain[7];


												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef00 = stReal_db_lut[i].spatialCoeff[0];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef01 = stReal_db_lut[i].spatialCoeff[1];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef02 = stReal_db_lut[i].spatialCoeff[2];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef03 = stReal_db_lut[i].spatialCoeff[3];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef04 = stReal_db_lut[i].spatialCoeff[4];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef05 = stReal_db_lut[i].spatialCoeff[5];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef10 = stReal_db_lut[i].spatialCoeff[6];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef11 = stReal_db_lut[i].spatialCoeff[7];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef12 = stReal_db_lut[i].spatialCoeff[8];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef13 = stReal_db_lut[i].spatialCoeff[9];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef14 = stReal_db_lut[i].spatialCoeff[10];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef15 = stReal_db_lut[i].spatialCoeff[11];

												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef20 = stReal_db_lut[i].spatialCoeff[12];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef21 = stReal_db_lut[i].spatialCoeff[13];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef22 = stReal_db_lut[i].spatialCoeff[14];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef23 = stReal_db_lut[i].spatialCoeff[15];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef24 = stReal_db_lut[i].spatialCoeff[16];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef25 = stReal_db_lut[i].spatialCoeff[17];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef30 = stReal_db_lut[i].spatialCoeff[18];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef31 = stReal_db_lut[i].spatialCoeff[19];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef32 = stReal_db_lut[i].spatialCoeff[20];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef33 = stReal_db_lut[i].spatialCoeff[21];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef34 = stReal_db_lut[i].spatialCoeff[22];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef35 = stReal_db_lut[i].spatialCoeff[23];


												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef40 = stReal_db_lut[i].spatialCoeff[24];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef41 = stReal_db_lut[i].spatialCoeff[25];
												LED_DB_LUT[i].hal_spatial_filter.hal_ld_spatialnewcoef50 = stReal_db_lut[i].spatialCoeff[26];



												LED_DB_LUT[i].hal_temporal_filter.hal_ld_tmp_pos0thd = stReal_db_lut[i].temporal_pos_thd_0;
												LED_DB_LUT[i].hal_temporal_filter.hal_ld_tmp_pos1thd = stReal_db_lut[i].temporal_pos_thd_1;
												LED_DB_LUT[i].hal_temporal_filter.hal_ld_tmp_posmingain = stReal_db_lut[i].temporal_pos_gain_min;
												LED_DB_LUT[i].hal_temporal_filter.hal_ld_tmp_posmaxgain = stReal_db_lut[i].temporal_pos_gain_max;
												LED_DB_LUT[i].hal_temporal_filter.hal_ld_tmp_neg0thd = stReal_db_lut[i].temporal_neg_thd_0;
												LED_DB_LUT[i].hal_temporal_filter.hal_ld_tmp_neg1thd = stReal_db_lut[i].temporal_neg_thd_1;
												LED_DB_LUT[i].hal_temporal_filter.hal_ld_tmp_negmingain = stReal_db_lut[i].temporal_neg_gain_min;
												LED_DB_LUT[i].hal_temporal_filter.hal_ld_tmp_negmaxgain = stReal_db_lut[i].temporal_neg_gain_max;
												LED_DB_LUT[i].hal_temporal_filter.hal_ld_tmp_maxdiff = stReal_db_lut[i].temporal_maxdiff;
												LED_DB_LUT[i].hal_temporal_filter.hal_ld_tmp_scenechangegain1 = stReal_db_lut[i].temporal_sencechange_gain;

												LED_DB_LUT[i].hal_remap_curve.hal_ld_spatialremapen = stReal_db_lut[i].spatial_remap_en;
												for (j=0;j<65;j++)
													LED_DB_LUT[i].hal_remap_curve.hal_ld_spatialremaptab[j] = stReal_db_lut[i].spatial_remap_tab[j];

												for (j=0;j<17;j++)
													for (k=0;k<17;k++)
														LED_DB_LUT[i].hal_data_comp_table.hal_ld_comp_2Dtable[j][k] = stReal_db_lut[i].comp_2d_tbl[j][k];

												LED_DB_LUT[i].hal_global_ctrl.hal_ld_blk_hnum = stReal_db_lut[i].blk_Hnum;
												LED_DB_LUT[i].hal_global_ctrl.hal_ld_blk_vnum = stReal_db_lut[i].blk_Vnum;
												LED_DB_LUT[i].hal_global_ctrl.hal_ld_blk_hsize = stReal_db_lut[i].ld_blk_Hsize;
												LED_DB_LUT[i].hal_global_ctrl.hal_ld_blk_vsize = stReal_db_lut[i].ld_blk_Vsize;
#if 1
	rtd_printk(KERN_EMERG,"LDV", "i=%d, ui_index=%d, maxgain=%d, avrgain=%d, histoshiftbit=%d,\n",
		i, stReal_db_lut[i].ui_index, stReal_db_lut[i].maxgain, stReal_db_lut[i].avrgain, stReal_db_lut[i].histoshiftbit);
	rtd_printk(KERN_EMERG,"LDV", "histogain[0]=%d, histogain[1]=%d, histogain[2]=%d, histogain[3]=%d,\n",
		stReal_db_lut[i].histogain[0], stReal_db_lut[i].histogain[1], stReal_db_lut[i].histogain[2], stReal_db_lut[i].histogain[3]);
	rtd_printk(KERN_EMERG,"LDV", "histogain[4]=%d, histogain[5]=%d, histogain[6]=%d, histogain[7]=%d,\n",
		stReal_db_lut[i].histogain[4], stReal_db_lut[i].histogain[5], stReal_db_lut[i].histogain[6], stReal_db_lut[i].histogain[7]);
	rtd_printk(KERN_EMERG,"LDV", "spatialCoeff[0]=%d, spatialCoeff[1]=%d, spatialCoeff[2]=%d, spatialCoeff[3]=%d,\n",
		stReal_db_lut[i].spatialCoeff[0], stReal_db_lut[i].spatialCoeff[1], stReal_db_lut[i].spatialCoeff[2], stReal_db_lut[i].spatialCoeff[3]);
	rtd_printk(KERN_EMERG,"LDV", "spatialCoeff[4]=%d, spatialCoeff[5]=%d, spatialCoeff[6]=%d, spatialCoeff[7]=%d,\n",
		stReal_db_lut[i].spatialCoeff[4], stReal_db_lut[i].spatialCoeff[5], stReal_db_lut[i].spatialCoeff[6], stReal_db_lut[i].spatialCoeff[7]);
	rtd_printk(KERN_EMERG,"LDV", "spatialCoeff[8]=%d, spatialCoeff[9]=%d, spatialCoeff[10]=%d, spatialCoeff[11]=%d,\n",
		stReal_db_lut[i].spatialCoeff[8], stReal_db_lut[i].spatialCoeff[9], stReal_db_lut[i].spatialCoeff[10], stReal_db_lut[i].spatialCoeff[11]);
	rtd_printk(KERN_EMERG,"LDV", "temporal_pos_thd_0=%d, temporal_pos_thd_1=%d, temporal_pos_gain_min=%d, temporal_pos_gain_max=%d,\n",
		stReal_db_lut[i].temporal_pos_thd_0, stReal_db_lut[i].temporal_pos_thd_1, stReal_db_lut[i].temporal_pos_gain_min, stReal_db_lut[i].temporal_pos_gain_max);
	rtd_printk(KERN_EMERG,"LDV", "temporal_neg_thd_0=%d, temporal_neg_thd_1=%d, temporal_neg_gain_min=%d, temporal_neg_gain_max=%d,\n",
		stReal_db_lut[i].temporal_neg_thd_0, stReal_db_lut[i].temporal_neg_thd_1, stReal_db_lut[i].temporal_neg_gain_min, stReal_db_lut[i].temporal_neg_gain_max);
	rtd_printk(KERN_EMERG,"LDV", "temporal_maxdiff=%d, temporal_sencechange_gain=%d, spatial_remap_en=%d,\n",
		stReal_db_lut[i].temporal_maxdiff, stReal_db_lut[i].temporal_sencechange_gain, stReal_db_lut[i].spatial_remap_en);
	rtd_printk(KERN_EMERG,"LDV", "blk_Hnum=%d, blk_Vnum=%d, ld_blk_Hsize=%d, ld_blk_Vsize=%d,\n",
		stReal_db_lut[i].blk_Hnum, stReal_db_lut[i].blk_Vnum, stReal_db_lut[i].ld_blk_Hsize, stReal_db_lut[i].ld_blk_Vsize);
#endif


										}

#if 0
                                        /* for debug ,after please remove it */
                                        rtd_pr_vpq_emerg("kernel VPQ_LED_IOC_LDSetDBLUT success\n");

                                        rtd_pr_vpq_emerg(" ui index =%d ,[0]maxgain=%d , \n",LED_DB_LUT[0].LD_UI_Index,LED_DB_LUT[0].hal_backlight_decision.hal_ld_maxgain);
                                        rtd_pr_vpq_emerg(" ui index =%d ,[1]maxgain=%d , \n",LED_DB_LUT[1].LD_UI_Index,LED_DB_LUT[1].hal_backlight_decision.hal_ld_maxgain);

                                        rtd_pr_vpq_emerg(" ui index =%d ,[5]maxgain=%d , \n",LED_DB_LUT[5].LD_UI_Index,LED_DB_LUT[5].hal_backlight_decision.hal_ld_maxgain);
                                        rtd_pr_vpq_emerg(" ui index =%d ,[6]maxgain=%d , \n",LED_DB_LUT[6].LD_UI_Index,LED_DB_LUT[6].hal_backlight_decision.hal_ld_maxgain);

                                        rtd_pr_vpq_emerg(" [6][0][0] =%d ,[6][0][1]=%d , \n",LED_DB_LUT[6].hal_data_comp_table.hal_ld_comp_2Dtable[0][0],LED_DB_LUT[6].hal_data_comp_table.hal_ld_comp_2Dtable[0][1]);

                                        rtd_pr_vpq_emerg(" sizeof(stReal_db_lut)=%d ,LED_DB_LUT=%d ,sizeof(KADP_LED_DB_LUT_T)=%d\n",sizeof(stReal_db_lut),sizeof(LED_DB_LUT),sizeof(KADP_LED_DB_LUT_T));
#endif
                                        fwif_color_set_LDSetDBLUT_TV006(&LED_DB_LUT[0]);
                                        rtd_pr_vpq_debug("kernel VPQ_LED_IOC_LDSetDBLUT success\n");

                                        ret = 0;

                                }

                        }
                }
                break;
                case V4L2_CID_EXT_LED_CONTROL_SPI:
                {
                        struct v4l2_ext_led_spi_ctrl_info stRealInfo;
			//rtd_pr_vpq_emerg("v4l2_ext_led_spi_ctrl_info\n");


			if (PQ_LED_Dev_Status != PQ_LED_DEV_INIT_DONE)
				return -1;

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user pqData fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }


                                if(copy_from_user((void *)&stRealInfo, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_led_spi_ctrl_info)))
                                {

                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user stRealInfo fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;

                                }
                                else
                                {

                                        LDCtrlSPI[0] = stRealInfo.bitMask;
                                        LDCtrlSPI[1] = stRealInfo.ctrlValue;

                                        vpqled_HAL_VPQ_LED_LDCtrlSPI(&LDCtrlSPI[0]);
                                        //rtd_pr_vpq_emerg("LD bitMask=%d,ctrlValue=%d \n",stRealInfo.bitMask,stRealInfo.ctrlValue);
                                        ret = 0;

                                }

                        }
                }
                break;
				case V4L2_CID_EXT_LED_BPL_DATA:
                {
                        struct v4l2_ext_led_bpl_info stRealInfo;

			if (PQ_LED_Dev_Status != PQ_LED_DEV_INIT_DONE)
				return -1;

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user pqData fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }


                                if(copy_from_user((void *)&stRealInfo, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_led_bpl_info)))
                                {

                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user stRealInfo fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;

                                }
                                else
                                {


                                        BPL_data[0] = stRealInfo.ai_brightness;
                                        BPL_data[1] = stRealInfo.sensor_level;
										drvif_color_set_LED_BPL_DATA(BPL_data[0], BPL_data[1]);
                                        
                                        rtd_pr_vpq_notice("kernel V4L2_CID_EXT_LED_BPL_DATA success\n");
                                        rtd_pr_vpq_info("rord ai_brightness=%d,sensor_level=%d \n",stRealInfo.ai_brightness,stRealInfo.sensor_level);
                                        ret = 0;

                                }

                        }
                }
                break;
#ifdef	CONFIG_RTK_KDRV_DV
		case V4L2_CID_EXT_DOLBY_CFG_PATH:
		{
			size_t size;
			struct v4l2_ext_dolby_config_path *config_path = get_v4l2_vpq_cmn_data(&pqData, ext_control.ptr, &size);

			pr_debug("[DolbyVision] V4L2_CID_EXT_DOLBY_CFG_PATH\n");

			if (config_path != NULL) {
				if (size == sizeof(struct v4l2_ext_dolby_config_path)) {
					enum BUFFER_SIZE {
						MAX_SIZE = 256,
					};
					char buffer[MAX_SIZE];
					char *ptr = &buffer[0];

					memset(buffer, 0, sizeof(buffer));
					strncpy(buffer, config_path->sConfigPath, MAX_SIZE - 1);
					pr_info("[%s:%d][Dolby] buffer %s\n", __func__, __LINE__, buffer);

					dolby_adapter_set_pq_table_cfg_path(&ptr, 1);

				} else {
					pr_info("[DolbyVision] V4L2_CID_EXT_DOLBY_CFG_PATH set wrong size of data (%lu %lu)\n", size, sizeof(struct v4l2_ext_dolby_config_path));
				}

				kfree(config_path);

			}

		}
		break;

		case V4L2_CID_EXT_DOLBY_PICTURE_MODE:
		{
			size_t size;
			struct v4l2_ext_dolby_picture_mode *mode = get_v4l2_vpq_cmn_data(&pqData, ext_control.ptr, &size);

			pr_debug("[DolbyVision] V4L2_CID_EXT_DOLBY_PICTURE_MODE\n");

			if (mode != NULL) {
				if (size == sizeof(struct v4l2_ext_dolby_picture_mode)) {
					if (mode->bOnOff) {
						pr_debug("[%s:%d][Dolby] V4L2_CID_EXT_DOLBY_PICTURE_MODE  picture mode value = %d\n",
								__func__,
								__LINE__,
								mode->uPictureMode);

						dolby_adapter_set_ui_dv_picmode(mode->uPictureMode);
						dolby_adapter_set_hdmi_ui_change_flag(TRUE);
					}
				} else {
					pr_info("[DolbyVision] V4L2_CID_EXT_DOLBY_PICTURE_MODE set wrong size of data (%lu %lu)\n", size, sizeof(struct v4l2_ext_dolby_picture_mode));
				}

				kfree(mode);
			}
		}
		break;
		case V4L2_CID_EXT_DOLBY_PICTURE_MENU:
		{
			size_t size;
			struct v4l2_ext_dolby_picture_data *picture_data= get_v4l2_vpq_cmn_data(&pqData, ext_control.ptr, &size);
			pr_debug("[DolbyVision] V4L2_CID_EXT_DOLBY_PICTURE_MENU\n");

			if (picture_data != NULL) {
				if (size == sizeof(struct v4l2_ext_dolby_picture_data)) {
					/* TODO */
					if (picture_data->on_off) {
						switch (picture_data->picture_menu) {
							case V4L2_EXT_DOLBY_BACKLIGHT:
								dolby_adapter_set_ui_dv_backlight_value(picture_data->setting_value);
								dolby_adapter_set_hdmi_ui_change_flag(TRUE);
								break;
							case V4L2_EXT_DOLBY_BRIGHTNESS:
								dolby_adapter_set_ui_dv_brightness_value(picture_data->setting_value);
								break;
							case V4L2_EXT_DOLBY_COLOR:
								dolby_adapter_set_ui_dv_color_value(picture_data->setting_value);
								break;
							case V4L2_EXT_DOLBY_CONTRAST:
								dolby_adapter_set_ui_dv_contrast_value(picture_data->setting_value);
								break;
							case V4L2_EXT_DOLBY_PICTURE_MENU_MAX:
							default:
								break;
						}
					}
				} else {
					pr_info("[DolbyVision] V4L2_CID_EXT_DOLBY_PICTURE_MENU set wrong size of data (%lu %lu)\n", size, sizeof(struct v4l2_ext_dolby_picture_data));
				}

				kfree(picture_data);
			}
		}
		break;

		case V4L2_CID_EXT_DOLBY_GD_DELAY:
		{
			size_t size;
			struct v4l2_ext_dolby_gd_delay_param *gd_val = get_v4l2_vpq_cmn_data(&pqData, ext_control.ptr, &size);
			pr_debug("[DolbyVision] V4L2_CID_EXT_DOLBY_GD_DELAY\n");

			if (gd_val != NULL) {
				if (size == sizeof(struct v4l2_ext_dolby_gd_delay_param)) {
					dolby_adapter_set_gd_delay_table(gd_val, sizeof(struct v4l2_ext_dolby_gd_delay_param));
				} else {
					pr_info("[DolbyVision] V4L2_CID_EXT_DOLBY_GD_DELAY set wrong size of data (%lu %lu)\n", size, sizeof(struct v4l2_ext_dolby_gd_delay_param));
				}
				
				kfree(gd_val);
			}
		}
		break;

		case V4L2_CID_EXT_DOLBY_AMBIENT_LIGHT:
		{
			size_t size;
			struct v4l2_ext_dolby_ambient_light_param *ambient_light = get_v4l2_vpq_cmn_data(&pqData, ext_control.ptr, &size);;
			pr_debug("[DolbyVision] V4L2_CID_EXT_DOLBY_AMBIENT_LIGHT\n");
			if (ambient_light != NULL) {
				if (size == sizeof( struct v4l2_ext_dolby_ambient_light_param)) {
					if (ambient_light->onoff) {
						uint32_t lux_data[4] = { 0 };
						int n = copy_from_user(lux_data, ambient_light->rawdata, sizeof(lux_data));
						if (n) {
							pr_info("[%s:%d][Dolby] copy ambient light raw data fail\n", __func__, __LINE__);
						} else {
							extern int get_dolby_ambient_weight(void);
							int mode = get_dolby_ambient_weight();
							dolby_adapter_set_ambient_mode(mode);
							dolby_adapter_set_ambient_front(lux_data[1]);
							dolby_adapter_set_hdmi_ui_change_flag(TRUE);
						}
					} else {
						dolby_adapter_set_ambient_mode(0);
					}
				} else {
					pr_info("[DolbyVision] V4L2_CID_EXT_DOLBY_AMBIENT_LIGHT set wrong size of data (%lu %lu)\n", size, sizeof(struct v4l2_ext_dolby_ambient_light_param));
				}
				kfree(ambient_light);
			}
		}
		break;
#endif

		case V4L2_CID_EXT_VPQ_EXTRA_PATTERN:
		{
			size_t size;
#ifdef USE_EXT_PATTERN_INFO_V2
			struct v4l2_vpq_ext_pattern_info_v2 *stRealInfo = get_v4l2_vpq_cmn_data(&pqData, ext_control.ptr, &size);
#else
			struct v4l2_vpq_ext_pattern_info *stRealInfo = get_v4l2_vpq_cmn_data(&pqData, ext_control.ptr, &size);
#endif

			pr_debug("[DolbyVision] V4L2_CID_EXT_VPQ_EXTRA_PATTERN\n");
                        if (stRealInfo != NULL) {
				if (size == sizeof(struct v4l2_vpq_ext_pattern_info_v2)) {
					memcpy(&ext_pattern_info_v2, stRealInfo, sizeof(struct v4l2_vpq_ext_pattern_info_v2));
				} else {
					pr_info("[DolbyVision] V4L2_CID_EXT_VPQ_EXTRA_PATTERN set wrong size of data (%lu %lu)\n", size, sizeof(struct v4l2_vpq_ext_pattern_info_v2));
				}
			}

			if (stRealInfo && stRealInfo->eMode == V4L2_VPQ_EXT_PATTERN_WINBOX) {
				inner_pattern_winbox_win_attr inner_info_array[2] = {{0}, {0}};
#ifdef USE_EXT_PATTERN_INFO_V2
				struct v4l2_vpq_ext_pattern_winbox_info *stWinboxInfo = &stRealInfo->stWinboxInfo;
#else
				struct v4l2_vpq_ext_pattern_winbox_info _stWinboxInfo;
				struct v4l2_vpq_ext_pattern_winbox_info *stWinboxInfo = &_stWinboxInfo;

				int copy_size = copy_from_user(stWinboxInfo, stRealInfo->pstWinboxInfo, sizeof(struct v4l2_vpq_ext_pattern_winbox_info));
				if (copy_size) {
					ret = -EFAULT;
					kfree(stRealInfo);
					break;
				}
#endif

				if (stRealInfo->bOnOff) {
					assemble_winbox_win_attr(&inner_info_array[0], &stWinboxInfo->stWinBoxAttr[0]);

					if(stWinboxInfo->u8NumWin >= 2)
						assemble_winbox_win_attr(&inner_info_array[1], &stWinboxInfo->stWinBoxAttr[1]);

				}

				ctrl_inner_pattern(stRealInfo->bOnOff, inner_info_array);
			}

			kfree(stRealInfo);
			break;
		}


            default:
                ret=-1;
                    break;
        }
        return ret;
}




int vpq_v4l2_main_ioctl_g_ext_ctrls(struct file *file, void *fh, struct v4l2_ext_controls *ctrls)
{
        int ret = 0;
        unsigned int cmd = 0xff;
        //unsigned int filter_cmd=0xff;
        struct v4l2_ext_control ext_control;
        struct v4l2_vpq_cmn_data pqData;
	//struct v4l2_ext_hdr_3dlut_RTK st3dlut_get_ext;	
	unsigned int* pwUINT32Temp;

        if(!ctrls)
        {
                rtd_pr_vpq_emerg("func:%s [error] ctrls is null\r\n",__FUNCTION__);
                return -EFAULT;
        }

        memcpy(&ext_control,ctrls->controls,sizeof(struct v4l2_ext_control));

        cmd = ext_control.id;
        V4L2printf_cmd(" VPQ_V4L2 :g_ext_ctrls , cmd ID =0x%x ", cmd);

	if (vpq_v4l2_main_get_stop_run(cmd))
		return 0;

        switch(cmd)
        {
                case V4L2_CID_EXT_VPQ_PICTURE_CTRL:
                {
                        struct v4l2_ext_picture_ctrl_data_RTK pictureCtrl;
                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {
                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {

                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }


                                if(copy_from_user((void *)&pictureCtrl, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_picture_ctrl_data_RTK)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                pictureCtrl.sContrast=pic_ctrl.Gain_Val[PIC_CTRL_Contrast];
                                pictureCtrl.sBrightness=pic_ctrl.Gain_Val[PIC_CTRL_Brightness];
                                pictureCtrl.sSaturation=pic_ctrl.Gain_Val[PIC_CTRL_Color];
                                pictureCtrl.sHue=pic_ctrl.Gain_Val[PIC_CTRL_Tint];
                                pictureCtrl.sPcVal[0]=pic_ctrl.siVal[0];
                                pictureCtrl.sPcVal[1]=pic_ctrl.siVal[1];
                                pictureCtrl.sPcVal[2]=pic_ctrl.siVal[2];
                                pictureCtrl.sPcVal[3]=pic_ctrl.siVal[3];


                                if(copy_to_user(to_user_ptr((pqData.p_data)), &pictureCtrl, sizeof(struct v4l2_ext_picture_ctrl_data_RTK)))
                                {
                                        rtd_pr_vpq_err("func:%s [error] V4L2_CID_EXT_VPQ_PICTURE_CTRL copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }

                        }
                }
                break;
                case V4L2_CID_EXT_VPQ_SHARPNESS:
                {

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }
				if(copy_to_user(to_user_ptr((pqData.p_data)), &sharpness, sizeof(struct v4l2_ext_vpq_sharpness_data)))
				{
					rtd_pr_vpq_emerg("func:%s [error] V4L2_CID_EXT_VPQ_SHARPNESS copy_to_user fail \r\n",__FUNCTION__);
					return -EFAULT;
				}

                        }

                }
                break;

		case V4L2_CID_EXT_VPQ_CM_DB_DATA:
		{

			if(!ext_control.ptr)
			{
				rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
				ret = -EFAULT;
				return ret;
			}
			else
			{

				if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
				{
					rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
					ret = -EFAULT;
					return ret;
				}
                                if(copy_to_user(to_user_ptr((pqData.p_data)), &cm_info, sizeof(struct v4l2_ext_cm_info)))
                                {
                                        rtd_pr_vpq_emerg("func:%s [error] V4L2_CID_EXT_VPQ_CM_DB_DATA copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }
			}

		}
		break;

                case V4L2_CID_EXT_VPQ_HISTO_DATA:
                {
                        struct v4l2_ext_vpq_dc2p_histodata_info histdata_info;

                        if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                ret = -EFAULT;
                                return ret;
                        }

                        if(copy_from_user((void *)&histdata_info, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_vpq_dc2p_histodata_info)))
                        {

                                rtd_pr_vpq_emerg("rord  3 addr1 =%p,addr2=%p \n",&histdata_info,pqData.p_data);
                                rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                ret = -EFAULT;
                                return ret;


                        }

                        if (fwif_color_get_HistoData_Countbins(TV006_VPQ_chrm_bin, (histdata_info.bin)) == FALSE) {
                            rtd_pr_vpq_err("kernel VPQ_IOC_GET_HistoData_Countbins fail\n");
                             return -EFAULT;
                        }

                        if (fwif_color_get_HistoData_chrmbins(COLOR_AutoSat_HISTOGRAM_LEVEL, (histdata_info.chrm_bin)) == FALSE) {
                                rtd_pr_vpq_err("kernel VPQ_IOC_GET_HistoData_chrm_bin fail\n");
                                 return -EFAULT;
                        }

                        if (fwif_color_get_HistoData_huebins(COLOR_HUE_HISTOGRAM_LEVEL, (histdata_info.hue_bin)) == FALSE) {
                                rtd_pr_vpq_err("kernel fwif_color_get_HistoData_Countbins fail\n");
                                 return -EFAULT;
                        }

                        if (fwif_color_get_HistoData_APL(&histdata_info.apl) == FALSE) {
                                rtd_pr_vpq_err("kernel fwif_color_get_HistoData_APL fail\n");
                                return -EFAULT;
                        }
                        else
                        {
        			if (drvif_color_get_WB_pattern_on())
        				histdata_info.apl  = 940;
                        }

                        if (fwif_color_get_HistoData_Min(&histdata_info.min) == FALSE) {
        			rtd_pr_vpq_err("kernel fwif_color_get_HistoData_Min fail\n");
        			return -EFAULT;
        		}

                        if (fwif_color_get_HistoData_Max(&histdata_info.max) == FALSE) {
                                rtd_pr_vpq_err("kernel fwif_color_get_HistoData_Max fail\n");
        			return -EFAULT;
                        }

                        if (fwif_color_get_HistoData_PeakLow(&histdata_info.peak_low) == FALSE) {
                                rtd_pr_vpq_err("kernel fwif_color_get_HistoData_PeakLow fail\n");
                                return -EFAULT;
                        }
                        if (fwif_color_get_HistoData_skinCount(&histdata_info.skin_count) == FALSE) {
                                rtd_pr_vpq_err("kernel fwif_color_get_HistoData_skinCount fail\n");
                                return -EFAULT;
                        }

                        if (fwif_color_get_HistoData_skinCount(&histdata_info.sat_status) == FALSE) {
                                rtd_pr_vpq_err("kernel fwif_color_get_HistoData_skinCount fail\n");
                                return -EFAULT;
                        }

                        if (fwif_color_get_HistoData_diffSum(&histdata_info.diff_sum) == FALSE) {
                                rtd_pr_vpq_err("kernel fwif_color_get_HistoData_diffSum fail\n");
                                return -EFAULT;
                        }
                        if (fwif_color_get_HistoData_motion(&histdata_info.motion) == FALSE) {
                                rtd_pr_vpq_err("kernel VPQ_IOC_GET_HistoData_motion fail\n");
                                 return -EFAULT;
                        }
                        if (fwif_color_get_HistoData_texture(&histdata_info.texture) == FALSE) {
                                rtd_pr_vpq_err("kernel fwif_color_get_HistoData_texture fail\n");
                                return -EFAULT;
                        }


                        if(copy_to_user(to_user_ptr((pqData.p_data)), &histdata_info, sizeof(struct v4l2_ext_vpq_dc2p_histodata_info)))
                        {
                                rtd_pr_vpq_emerg("func:%s [error] v4l2_ext_vpq_dc2p_histodata_info copy_to_user fail \r\n",__FUNCTION__);
                                return -EFAULT;
                        }

                }
                break;
                case V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST:
                {
                        if (fwif_color_vpq_stop_ioctl(STOP_VPQ_IOC_SET_FreshContrast, ACCESS_MODE_GET, 0))
                                return 0;

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_to_user(to_user_ptr((pqData.p_data)), (void *)&dynamic_ctrl, sizeof(struct v4l2_ext_dynamnic_contrast_ctrl)))
                                {
                                        rtd_pr_vpq_err("func:%s [error] V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }


                        }
                }
                break;
                case V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST_LUT:
                {
                        #define RHAL_VPQ_DCC_NODE_NUM 33


                        if (fwif_color_vpq_stop_ioctl(STOP_VPQ_IOC_SET_FreshContrast, ACCESS_MODE_GET, 0))
                                return 0;

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_to_user(to_user_ptr((pqData.p_data)), &dynamiccontrastlut, sizeof(struct v4l2_ext_dynamnic_contrast_lut)))
                                {
                                        rtd_pr_vpq_emerg("func:%s [error] V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST_LUT copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }

                        }
                }
                break;
                case V4L2_CID_EXT_VPQ_NOISE_REDUCTION:
                {

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_to_user(to_user_ptr((pqData.p_data)), &noiseReduction, sizeof(struct v4l2_ext_vpq_noise_reduction_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s [error] V4L2_CID_EXT_VPQ_NOISE_REDUCTION copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }

                        }
                }
                break;
                case V4L2_CID_EXT_VPQ_MPEG_NOISE_REDUCTION:
                {

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

				if(copy_to_user(to_user_ptr((pqData.p_data)), &mpeg_noiseReduction, sizeof(struct v4l2_ext_vpq_mpeg_noise_reduction_data)))
				{
					rtd_pr_vpq_emerg("func:%s [error] V4L2_CID_EXT_VPQ_MPEG_NOISE_REDUCTION copy_to_user fail \r\n",__FUNCTION__);
					return -EFAULT;
				}


                        }
                }
                break;
                case V4L2_CID_EXT_VPQ_BLACK_LEVEL:
                {

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }


                                if(copy_to_user(to_user_ptr((pqData.p_data)),(void *)&v4l2_blackLevel, sizeof(struct v4l2_ext_vpq_black_level_info)))
                                {
                                        rtd_pr_vpq_emerg("func:%s [error] V4L2_CID_EXT_VPQ_BLACK_LEVEL copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }

                        }

                }
                break;

                case V4L2_CID_EXT_VPQ_BYPASS_BLOCK:
                {

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }


                                if(copy_to_user(to_user_ptr((pqData.p_data)),(void *)&v4l2_blockbypass, sizeof(struct v4l2_ext_vpq_block_bypass)))
                                {
                                        rtd_pr_vpq_emerg("func:%s [error] V4L2_CID_EXT_VPQ_BYPASS_BLOCK copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }

                        }

                }
                break;
                case V4L2_CID_EXT_VPQ_GAMMA_DATA:
                {

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_from_user((void *)&gamma_lut, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_gamma_lut)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                gamma_lut.table_num = v4l2_gamma_table_num;
                                if(copy_to_user(to_user_ptr((pqData.p_data)),(void *)&gamma_lut, sizeof(struct v4l2_ext_gamma_lut)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_to_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_to_user(to_user_ptr((gamma_lut.table_blue)),(void *)&Gamme_Blue, sizeof(Gamme_Blue)))
                                {
                                        rtd_pr_vpq_emerg("func:%s [error] V4L2_CID_EXT_VPQ_GAMMA_DATA copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }


                                if(copy_to_user(to_user_ptr((gamma_lut.table_green)),(void *)&Gamme_Green, sizeof(Gamme_Green)))
                                {
                                        rtd_pr_vpq_emerg("func:%s [error] V4L2_CID_EXT_VPQ_GAMMA_DATA copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }


                                if(copy_to_user(to_user_ptr((gamma_lut.table_red)),(void *)&Gamme_Red, sizeof(Gamme_Red)))
                                {
                                        rtd_pr_vpq_emerg("func:%s [error] V4L2_CID_EXT_VPQ_GAMMA_DATA copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }


                        }

                }
                break;
                case V4L2_CID_EXT_VPQ_SUPER_RESOLUTION:
                {

                       // struct v4l2_ext_vpq_super_resolution_data super_resolution;

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_from_user((void *)&super_resolution, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_vpq_super_resolution_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;

                                }
                                super_resolution.ui_value = v4l2_sr_ui_value;

                                if(copy_to_user(to_user_ptr(pqData.p_data),(void *)&super_resolution, sizeof(struct v4l2_ext_vpq_super_resolution_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s [error] v4l2_ext_vpq_super_resolution_data copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }

                                if(copy_to_user(to_user_ptr((super_resolution.pst_chip_data)), (void *)&v4l2_tSR_UI_T, sizeof(CHIP_SR_UI_T)))
                                {
                                        rtd_pr_vpq_err("func:%s [error] V4L2_CID_EXT_VPQ_SUPER_RESOLUTION copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }



                        }
                }
                break;

                case V4L2_CID_EXT_VPQ_COLORTEMP_DATA:
                {
                       // struct v4l2_colortemp_info color_temp;

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_to_user(to_user_ptr(pqData.p_data),(void *)&v4l2_color_temp, sizeof(v4l2_color_temp)))
                                {
                                        rtd_pr_vpq_emerg("func:%s [error] v4l2_color_temp copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }


                          }
                }
                break;
                case V4L2_CID_EXT_VPQ_NOISE_LEVEL:
                {
                        _clues *smartPic_clue;
                        UINT32 PQA_Noise_Level = 0;

                        unsigned short uLevel;

                        if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                ret = -EFAULT;
                                return ret;
                        }


                        if(copy_from_user((void *)&uLevel, to_user_ptr(pqData.p_data), sizeof(unsigned short)))
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                ret = -EFAULT;
                                return ret;

                        }

                        /* Get Info from Share Mem*/
                        smartPic_clue = fwif_color_Get_SmartPic_clue();
                        if (0 == smartPic_clue)
                                return -1;
                        fwif_color_ChangeUINT32Endian_Copy(&smartPic_clue->noise_ratio, 1, &PQA_Noise_Level, 0);

                        uLevel = PQA_Noise_Level;

                        if(copy_to_user(to_user_ptr((pqData.p_data)), &uLevel, sizeof(unsigned short)))
                        {
                                rtd_pr_vpq_emerg("func:%s [error] v4l2_ext_vpq_dc2p_histodata_info copy_to_user fail \r\n",__FUNCTION__);
                                return -EFAULT;
                        }

                }
                break;
                case V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST_BYPASS_LUT:
                {
                        SINT32 dc_lut[16];
                        SINT32 DCBypassLUT[16] = {64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 936, 1023};

                        if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                ret = -EFAULT;
                                return ret;
                        }

                        if(copy_from_user((void *)dc_lut, to_user_ptr(pqData.p_data), sizeof(dc_lut)))
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                ret = -EFAULT;
                                return ret;
                        }

                        if(copy_to_user(to_user_ptr((pqData.p_data)), DCBypassLUT, sizeof(dc_lut)))
                        {
                                rtd_pr_vpq_emerg("func:%s [error] v4l2_ext_vpq_dc2p_histodata_info copy_to_user fail \r\n",__FUNCTION__);
                                return -EFAULT;
                        }

                }
                break;

                case V4L2_CID_EXT_VPQ_LOCALCONTRAST_DATA: // this driver have to discuss with LG
                {

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_err("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_to_user(to_user_ptr((pqData.p_data)), (void *)&glc_param, sizeof(CHIP_LOCAL_CONTRAST_T)))
                                {
                                        rtd_pr_vpq_err("func:%s line:%d [error] copy_to_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }
                        }

                }

                break;


                case V4L2_CID_EXT_VPQ_GAMUT_MATRIX_PRE: // this driver have to discuss with LG
                {

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_err("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }


                                if(copy_to_user(to_user_ptr((pqData.p_data)), (void *)&v4l2_vpq_MATRIX_PRE, sizeof(v4l2_vpq_MATRIX_PRE)))
                                {
                                        rtd_pr_vpq_err("func:%s line:%d [error] copy_to_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }
                        }

                }
                break;
                case V4L2_CID_EXT_VPQ_GAMUT_MATRIX_POST:
                {
                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_err("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {
                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {

                                        rtd_pr_vpq_err("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_to_user(to_user_ptr((pqData.p_data)), (void *)&gamutPost, sizeof(gamutPost)))
                                {
                                        rtd_pr_vpq_err("func:%s line:%d [error] copy_to_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                        }

                }
                break;

                case V4L2_CID_EXT_HDR_PIC_INFO:
                {


                        if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                ret = -EFAULT;
                                return ret;
                        }


				if (fwif_color_get_DM2_Hist_AutoMode_Enable_Flag() == 1) {
					pwUINT32Temp = fwif_color_DM2_Hist_AutoMode_TBL_Get(DM2_Hist_RGB_MAX_Mode);	// idx need to define in future
					memcpy(&HDR_info_bin[0], pwUINT32Temp, sizeof(int)*HDR_Histogram_size);
				} else {

				}

				if(copy_to_user(to_user_ptr((pqData.p_data)), &HDR_info_bin, sizeof(int)*128))	// only get 128 int for v4l2
				{
					rtd_pr_vpq_emerg("func:%s [error] V4L2_CID_EXT_HDR_PIC_INFO copy_to_user fail \r\n",__FUNCTION__);
					return -EFAULT;
				}
                }
                break;
#if 0
		case V4L2_CID_EXT_HDR_3DLUT:
		{
			if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
			{
				rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
				ret = -EFAULT;
				return ret;
			}
			if(copy_from_user((void *)&st3dlut_get_ext, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_hdr_3dlut_RTK)))
			{
				printk(KERN_EMERG"func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
				ret = -EFAULT;
				return ret;
			}
#ifdef isSupport_HDR_3dLUT_24x24x24
			if (st3dlut_get_ext.data_size != HDR_24x24x24_size*3*sizeof(short)) {	// 24*24*24*3*sizeof(short)
				printk(KERN_EMERG"size is not correct get for 24x24x24,  st3dlut.data_size = %d, \r\n", st3dlut_get_ext.data_size);
				ret = -EFAULT;
				return ret;
			}
#else
			if (st3dlut_get_ext.data_size != VIP_DM_HDR_3D_LUT_UI_TBL_SIZE*sizeof(short)) {	// 17*17*17*3*sizeof(short)
				printk(KERN_EMERG"size is not correct get for 17x17x17,  st3dlut.data_size = %d, \r\n", st3dlut_get_ext.data_size);
				ret = -EFAULT;
				return ret;
			}
#endif	
			if(copy_to_user(to_user_ptr((st3dlut_get_ext.p3dlut)), &phdr3dlut16.hdr_3dlut, st3dlut_get_ext.data_size))
			{
				rtd_pr_vpq_emerg("V4L2_CID_EXT_HDR_3DLUT copy_to_user fail, st3dlut_get_ext.p3dlut=%p, \r\n", st3dlut_get_ext.p3dlut);
				return -EFAULT;
			}			
		}
		break;
#endif		
		case V4L2_CID_EXT_HDR_HLG_Y_GAIN_TBL:
		{
			if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
			{
				rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
				ret = -EFAULT;
				return ret;
			}

			if(copy_to_user(to_user_ptr((pqData.p_data)), &Oetf_table16_prog[0], sizeof(short)*OETF_PROG_SIZE_TV006))
			{
				rtd_pr_vpq_emerg("func:%s [error] V4L2_CID_EXT_HDR_HLG_Y_GAIN_TBL copy_to_user fail \r\n",__FUNCTION__);
				return -EFAULT;
			}
		}
		break;

                case V4L2_CID_EXT_VPQ_PQ_MODE_INFO:
                {

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                if(copy_to_user(to_user_ptr((pqData.p_data)), &pqModeInfo, sizeof(struct v4l2_ext_pq_mode_info_RTK)))
                                {
                                        rtd_pr_vpq_emerg("func:%s [error] V4L2_CID_EXT_VPQ_PQ_MODE_INFO copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }

                        }

                }
                break;

                case V4L2_CID_EXT_VPQ_DECONTOUR:
                {

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {
                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }


                                if(copy_to_user(to_user_ptr((pqData.p_data)), &de_contour, sizeof(struct v4l2_ext_vpq_decontour_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s [error] V4L2_CID_EXT_VPQ_DECONTOUR copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }

                        }

                }
                break;


		case V4L2_CID_EXT_VPQ_EXTRA_PATTERN:
		{
                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }


                                if(copy_to_user(to_user_ptr((pqData.p_data)), &ext_pattern_info_v2, sizeof(struct v4l2_vpq_ext_pattern_info_v2)))
                                {
                                        rtd_pr_vpq_emerg("func:%s [error] V4L2_CID_EXT_VPQ_EXTRA_PATTERN copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }

                        }


                }
                break;

                case V4L2_CID_EXT_MEMC_MOTION_COMP:
                {

                        //struct v4l2_ext_memc_motion_comp_info motion_comp_info;
                        //VPQ_MEMC_TYPE_T motion;

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {
                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user pqData fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }



                                if(copy_to_user(to_user_ptr((pqData.p_data)), &motion_comp_info, sizeof(struct v4l2_ext_memc_motion_comp_info)))
                                {
                                        rtd_pr_vpq_err("func:%s [error] V4L2_CID_EXT_MEMC_MOTION_COMP copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }

                        }
                }
                break;



                case V4L2_CID_EXT_LED_DEMOMODE:
                {
                        struct v4l2_ext_led_ldim_demo_info stRealInfo;

			//if (PQ_LED_Dev_Status != PQ_LED_DEV_INIT_DONE)
			//	return -1;


                        if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                        {
                               rtd_pr_vpq_err("func:%s line:%d [error] copy_from_user pqData fail \r\n",__FUNCTION__,__LINE__);
                                ret = -EFAULT;
                                return ret;
                        }


                        if(copy_from_user((void *)&stRealInfo, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_led_ldim_demo_info)))
                        {

                                rtd_pr_vpq_err("func:%s line:%d [error] copy_from_user stRealInfo fail \r\n",__FUNCTION__,__LINE__);
                                ret = -EFAULT;
                                return ret;

                        }
                        else
                        {

                                stRealInfo.bOnOff = LED_LDCtrlDemoMode.bCtrl;
                                stRealInfo.eType = LED_LDCtrlDemoMode.ctrlMode;

                                if(copy_to_user(to_user_ptr((pqData.p_data)), &stRealInfo, sizeof(stRealInfo)))
                                {
                                        rtd_pr_vpq_err("func:%s [error] V4L2_CID_EXT_LED_DEMOMODE copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }

                        }

                }
                break;
                case V4L2_CID_EXT_LED_DB_DATA:
                {
                        //if (PQ_LED_Dev_Status != PQ_LED_DEV_INIT_DONE)
                        //      return -1;
                        return -1; //it is error;
						#if 0
                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user pqData fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }

                                /*LED_DB_LUT[0].LD_UI_Index=7; this is for test , it can be remove
                                LED_DB_LUT[0].hal_backlight_decision.hal_ld_maxgain=99;
                                LED_DB_LUT[0].hal_data_comp_table.hal_ld_comp_2Dtable[0][0] =1048;
                                LED_DB_LUT[0].hal_data_comp_table.hal_ld_comp_2Dtable[0][1] =1049;
                                LED_DB_LUT[6].LD_UI_Index=6;
                                LED_DB_LUT[6].hal_backlight_decision.hal_ld_maxgain=88;
                                LED_DB_LUT[6].hal_data_comp_table.hal_ld_comp_2Dtable[0][0] =2048;
                                LED_DB_LUT[6].hal_data_comp_table.hal_ld_comp_2Dtable[0][1] =2049;
                                */
                                if(copy_to_user(to_user_ptr((pqData.p_data)), &LED_DB_LUT, sizeof(v4l2_ext_led_db_lut)*7))
                                {
                                        rtd_pr_vpq_err("func:%s [error] V4L2_CID_EXT_LED_DB_DATA copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }


                        }
						#endif
                }
                break;
                case V4L2_CID_EXT_LED_APL_DATA:
                {
                        unsigned short BLValue[2];
                        unsigned char suc_flag;
                        struct v4l2_ext_led_apl_info_RTK stRealInfo;

                        //if (PQ_LED_Dev_Status != PQ_LED_DEV_INIT_DONE)
                        //         return -1;

                        if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                       /*if(copy_from_user((void *)&stRealInfo, to_user_ptr(pqData.p_data), sizeof(stRealInfo)))
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user controls fail \r\n",__FUNCTION__,__LINE__);
                                ret = -EFAULT;
                                return ret;

                        }*/


                        if (fwif_color_vpq_stop_ioctl(STOP_VPQ_LED_LDGetAPL, ACCESS_MODE_GET, 0))
                                return 0;


                        suc_flag = vpqled_get_LD_GetAPL_TV006(&(BLValue[0]));

                        if (suc_flag == FALSE) {
                               // rtd_pr_vpq_err("kernel VPQ_LED_LDGetAPL fail\n");
                               // ret = -1;
                        }

                        stRealInfo.block_apl_max=BLValue[0];
                        stRealInfo.block_apl_min=BLValue[1];

                       // rtd_pr_vpq_emerg(" max=%d,min =%d \n",BLValue[0],BLValue[1]);


                        if(copy_to_user(to_user_ptr((pqData.p_data)), &stRealInfo, sizeof(stRealInfo)))
                        {
                                rtd_pr_vpq_emerg("func:%s [error] V4L2_CID_EXT_LED_APL_DATA copy_to_user fail \r\n",__FUNCTION__);
                                return -EFAULT;
                        }



                }
                break;
                case V4L2_CID_EXT_LED_CONTROL_SPI:
                {
                        struct v4l2_ext_led_spi_ctrl_info stRealInfo;
				//unsigned char LED_i = 0;
			//if (PQ_LED_Dev_Status != PQ_LED_DEV_INIT_DONE)
			//	return -1;

                        if(!ext_control.ptr)
                        {
                                rtd_pr_vpq_emerg("func:%s line:%d [error] ext_control.ptr is null\r\n",__FUNCTION__, __LINE__);
                                ret = -EFAULT;
                                return ret;
                        }
                        else
                        {

                                if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                                {
                                        rtd_pr_vpq_emerg("func:%s line:%d [error] copy_from_user pqData fail \r\n",__FUNCTION__,__LINE__);
                                        ret = -EFAULT;
                                        return ret;
                                }
					memset((void*)&stRealInfo, 0, sizeof(struct v4l2_ext_led_spi_ctrl_info));
                                stRealInfo.bitMask =LDCtrlSPI[0];
                                stRealInfo.ctrlValue =LDCtrlSPI[1];

#if 0
					for (LED_i = 0; LED_i<sizeof(struct v4l2_ext_led_spi_ctrl_info);LED_i++)
						printk("read V4L2_CID_EXT_LED_CONTROL_SPI[%d]=%d,size=%ld\n", LED_i, *(((unsigned char*)&stRealInfo)+LED_i), sizeof(struct v4l2_ext_led_spi_ctrl_info));
#endif
					if(copy_to_user(to_user_ptr((pqData.p_data)), &stRealInfo, sizeof(stRealInfo)))
                                {
                                        rtd_pr_vpq_emerg("func:%s [error] V4L2_CID_EXT_LED_APL_DATA copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }

                        }
                }
                break;
				case V4L2_CID_EXT_LED_BPL_DATA:
                {
                        struct v4l2_ext_led_bpl_info stRealInfo;

                        if(copy_from_user((void *)&pqData, to_user_ptr(ext_control.ptr), sizeof(struct v4l2_vpq_cmn_data)))
                        {
                               rtd_pr_vpq_err("func:%s line:%d [error] copy_from_user pqData fail \r\n",__FUNCTION__,__LINE__);
                                ret = -EFAULT;
                                return ret;
                        }


                        if(copy_from_user((void *)&stRealInfo, to_user_ptr(pqData.p_data), sizeof(struct v4l2_ext_led_bpl_info)))
                        {

                                rtd_pr_vpq_err("func:%s line:%d [error] copy_from_user stRealInfo fail \r\n",__FUNCTION__,__LINE__);
                                ret = -EFAULT;
                                return ret;

                        }
                        else
                        {

                                stRealInfo.ai_brightness = BPL_data[0];
                                stRealInfo.sensor_level = BPL_data[1];

                                if(copy_to_user(to_user_ptr((pqData.p_data)), &stRealInfo, sizeof(stRealInfo)))
                                {
                                        rtd_pr_vpq_err("func:%s [error] V4L2_CID_EXT_LED_BPL_DATA copy_to_user fail \r\n",__FUNCTION__);
                                        return -EFAULT;
                                }

                        }

                }
                break;
#ifdef	CONFIG_RTK_KDRV_DV
		case V4L2_CID_EXT_DOLBY_CFG_PATH:
		{
			size_t size;
			struct v4l2_ext_dolby_config_path *config_path = get_v4l2_vpq_cmn_data(&pqData, ext_control.ptr, &size);
			pr_debug("[DolbyVision] V4L2_CID_EXT_DOLBY_CFG_PATH\n");
			if (config_path != NULL) {
				if (size == sizeof(struct v4l2_ext_dolby_config_path)) {
					int n = 0;

					memset(config_path->sConfigPath, 0, sizeof(config_path->sConfigPath));

					dolby_adapter_get_pq_table_cfg_path(config_path->sConfigPath, sizeof(config_path->sConfigPath));

					n = copy_to_user((void *)pqData.p_data, config_path, sizeof(struct v4l2_ext_dolby_config_path));
					if (n != 0) {
						ret = -EFAULT;
						pr_info("[%s:%d][Dolby] V4L2_CID_EXT_DOLBY_CFG_PATH : copy config_path fail\n", __func__, __LINE__);
					}
				} else {
					pr_info("[DolbyVision] V4L2_CID_EXT_DOLBY_CFG_PATH set wrong size of data (%lu %lu)\n", size, sizeof(struct v4l2_ext_dolby_config_path));
				}

				kfree(config_path);
			}
		}
		break;

		case V4L2_CID_EXT_DOLBY_PICTURE_MODE:
		{
			size_t size;
			struct v4l2_ext_dolby_picture_mode *mode = get_v4l2_vpq_cmn_data(&pqData, ext_control.ptr, &size);
			pr_debug("[DolbyVision] V4L2_CID_EXT_DOLBY_PICTURE_MODE\n");
			if (mode != NULL) {
				if (size == sizeof(struct v4l2_ext_dolby_picture_mode)) {
					int n = 0;

					mode->bOnOff = 1;
					mode->uPictureMode = dolby_adapter_get_ui_dv_picmode();
					pr_debug("[%s:%d][Dolby] picture mode value = %d\n", __func__, __LINE__, mode->uPictureMode);

					n = copy_to_user((void *)pqData.p_data, mode, sizeof(struct v4l2_ext_dolby_picture_mode));
					if (n != 0) {
						ret = -EFAULT;
						pr_info("[%s:%d][Dolby] V4L2_CID_EXT_DOLBY_PICTURE_MODE : copy picture_mode fail\n", __func__, __LINE__);
					}
				} else {
					pr_info("[DolbyVision] V4L2_CID_EXT_DOLBY_PICTURE_MODE set wrong size of data (%lu %lu)\n", size, sizeof(struct v4l2_ext_dolby_picture_mode));
				}

				kfree(mode);
			}
		}
		break;
		case V4L2_CID_EXT_DOLBY_PICTURE_MENU:
		{
			size_t size;
			struct v4l2_ext_dolby_picture_data *picture_data= get_v4l2_vpq_cmn_data(&pqData, ext_control.ptr, &size);
			pr_debug("[DolbyVision] V4L2_CID_EXT_DOLBY_PICTURE_MENU\n");

			if (picture_data != NULL) {
				if (size == sizeof(struct v4l2_ext_dolby_picture_data)) {
					int n = 0;
					int val = 0;

					picture_data->on_off = 1;

					switch (picture_data->picture_menu) {
						case V4L2_EXT_DOLBY_BACKLIGHT:
							val = dolby_adapter_get_ui_dv_backlight_value();
							break;
						case V4L2_EXT_DOLBY_BRIGHTNESS:
							val = dolby_adapter_get_ui_dv_brightness_value();
							break;
						case V4L2_EXT_DOLBY_COLOR:
							val = dolby_adapter_get_ui_dv_color_value();
							break;
						case V4L2_EXT_DOLBY_CONTRAST:
							val = dolby_adapter_get_ui_dv_contrast_value();
							break;
						case V4L2_EXT_DOLBY_PICTURE_MENU_MAX:
						default:
							break;
					}

					picture_data->setting_value = val;

					n = copy_to_user((void *)pqData.p_data, picture_data, sizeof(struct v4l2_ext_dolby_picture_data));
					if (n != 0) {
						ret = -EFAULT;
						pr_info("[%s:%d][Dolby] V4L2_CID_EXT_DOLBY_PICTURE_MENU : copy picture_data fail\n", __func__, __LINE__);
					}
				} else {
					pr_info("[DolbyVision] V4L2_CID_EXT_DOLBY_PICTURE_MENU set wrong size of data (%lu %lu)\n", size, sizeof(struct v4l2_ext_dolby_picture_data));
				}

				kfree(picture_data);
			}
		}
		break;

		case V4L2_CID_EXT_DOLBY_GD_DELAY:
		{
			int n;
			size_t gd_table_size;

			const uint8_t *gd_table = dolby_adapter_get_gd_delay_table(&gd_table_size);

                        if(ext_control.ptr == NULL) {
				ret = -EFAULT;
				break;
			}

			n = copy_from_user((void *)&pqData, ext_control.ptr, sizeof(struct v4l2_vpq_cmn_data));
			if (n != 0) {
				ret = -EFAULT;
				break;
			}

			n = copy_to_user((void *)pqData.p_data, gd_table, gd_table_size);
			if (n != 0)
				ret = -EFAULT;
			break;
		}
		break;

		case V4L2_CID_EXT_DOLBY_AMBIENT_LIGHT:
		{
			size_t size;
			struct v4l2_ext_dolby_ambient_light_param *ambient_light = get_v4l2_vpq_cmn_data(&pqData, ext_control.ptr, &size);;
			pr_debug("[DolbyVision] V4L2_CID_EXT_DOLBY_AMBIENT_LIGHT\n");
			if (ambient_light != NULL) {
				if (size == sizeof(struct v4l2_ext_dolby_ambient_light_param)) {
					extern int get_dolby_ambient_weight(void);
					uint32_t lux_data[4] = { 0 };
					int n = 0;

					ambient_light->onoff = (dolby_adapter_get_ambient_target_mode() > 0) ? 1 : 0;
					lux_data[1] = dolby_adapter_get_ambient_front();

					pr_info("[%s:%d][Dolby] V4L2_CID_EXT_DOLBY_AMBIENT_LIGHT %d %d\n", __func__, __LINE__,
							ambient_light->onoff,
							lux_data[1]);
					if (ambient_light->rawdata) {
						n = copy_to_user(ambient_light->rawdata, lux_data, sizeof(lux_data));
						if (n) {
							pr_info("[%s:%d][Dolby] V4L2_CID_EXT_DOLBY_AMBIENT_LIGHT : copy ambient light raw data fail\n", __func__, __LINE__);
							ret = -EFAULT;
						}
					} else {
						pr_info("[%s:%d][Dolby] V4L2_CID_EXT_DOLBY_AMBIENT_LIGHT : need userspace memory for copy raw data\n", __func__, __LINE__);
					}

					n = copy_to_user((void *)pqData.p_data, ambient_light, sizeof(struct v4l2_ext_dolby_ambient_light_param));
					if (n != 0) {
						ret = -EFAULT;
						pr_info("[%s:%d][Dolby] V4L2_CID_EXT_DOLBY_AMBIENT_LIGHT : copy ambient light fail\n", __func__, __LINE__);
					}
				} else {
					pr_info("[DolbyVision] V4L2_CID_EXT_DOLBY_AMBIENT_LIGHT set wrong size of data (%lu %lu)\n", size, sizeof(struct v4l2_ext_dolby_ambient_light_param));
				}

				kfree(ambient_light);
			}
		}
		break;

		case V4L2_CID_EXT_DOLBY_CONTENTS_TYPE:
		{
			extern void dolby_adapter_get_l11_content_type(uint8_t *l11);
			extern bool drvif_Hdmi_GetDolbyL11Content(unsigned char get_data[4]);

			uint8_t dolby_l11_content_type[4] = { 0 };
			VSC_INPUT_TYPE_T srctype;
			int n;

                        if(ext_control.ptr == NULL) {
				ret = -EFAULT;
				break;
			}

			n = copy_from_user((void *)&pqData, ext_control.ptr, sizeof(struct v4l2_vpq_cmn_data));
			if (n != 0) {
				ret = -EFAULT;
				break;
			}

			srctype = Get_DisplayMode_Src(SLR_MAIN_DISPLAY);
			if ((srctype == VSC_INPUTSRC_HDMI) && (get_HDMI_Dolby_VSIF_mode() == DOLBY_HDMI_VSIF_LL)) {
				int ret = drvif_Hdmi_GetDolbyL11Content(dolby_l11_content_type);
				if (ret == FALSE) {
					memset(dolby_l11_content_type, 0, sizeof(dolby_l11_content_type));
				}
			} else {
				dolby_adapter_get_l11_content_type(dolby_l11_content_type);
			}

			n = copy_to_user((void *)pqData.p_data, dolby_l11_content_type, sizeof(dolby_l11_content_type));
			if (n != 0)
				ret = -EFAULT;
			break;
		}
#endif
                default:
                        ret = -1;
                break;
        }
        return ret;
}


int vpq_v4l2_main_ioctl_s_ctrl(struct file *file, void *fh, struct v4l2_control *ctrl)
{
	_RPC_system_setting_info *rpc_vip_sysInfo = NULL;
        int ret = 0;
        struct v4l2_control controls;
        unsigned int cmd = 0xff;
        //unsigned int filter_cmd=0;;
        static unsigned char tmp = 0;

	rpc_vip_sysInfo = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

        if(!ctrl)
        {
                rtd_pr_vpq_emerg("func:%s [error] ctrls is null\r\n",__FUNCTION__);
                ret = -EFAULT;
                return ret;
        }
        memcpy(&controls, ctrl, sizeof(struct v4l2_control));
        cmd = controls.id;
	if (vpq_v4l2_main_get_stop_run(cmd))
		return 0;

        V4L2printf_cmd(" VPQ_V4L2 :s_ctrl , cmd ID =0x%x ", cmd);

        switch(cmd)
        {

                case V4L2_CID_EXT_VPQ_TESTPATTERN:
                {
                        unsigned char patternIre[23] = {0, 5, 10, 15, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200};   //IRE*2 IRE={0,2.5,5,7.5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100}
                        v4l2_vpq_ire = controls.value;
                        if(v4l2_vpq_ire==V4L2_EXT_VPQ_INNER_PATTERN_DISABLE)
                                fwif_color_set_WB_Pattern_IRE(0, v4l2_vpq_ire);
                        else {
                                if (v4l2_vpq_ire > 22)
                                    fwif_color_set_WB_Pattern_IRE(1, patternIre[22]);
                                else
                                    fwif_color_set_WB_Pattern_IRE(1, patternIre[v4l2_vpq_ire]);
                             }

                }
                break;
				case V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST_COLOR_GAIN:
				{
					static unsigned int pre_gain = 255;
					if (pre_gain != controls.value) {
						rtd_pr_vpq_info("V4L2_CID_EXT_VPQ_DYNAMIC_CONTRAST_COLOR_GAIN, [%d]\n", controls.value);
						pre_gain = controls.value;
					} 

					fwif_color_access_apl_color_gain_tv006(0, (int*)(&(controls.value)), ACCESS_MODE_WRITE);
					if (g_bAPLColorGainClr) {
						fwif_color_update_YUV2RGB_CSC_tv006(0);
						g_bAPLColorGainClr = 0;
					} else
						g_bAPLColorGainClr = 1;

				}
				break;
                case V4L2_CID_EXT_VPQ_REAL_CINEMA:
                {
                        unsigned char bCinemaMode;
                        DRV_film_table_t tFilmTable = {1, 2, 0, 1, 0, 1, 0, 0, 1, 8, 8, 8, 8, 22, 18, 40, 40, 21, 15, 20, 20, 120, 50, 3000, 2000, 2, 4, 60, 20, 40, 20};               // SD 60Hz
                        v4l2_real_cinema = controls.value;

                        if(v4l2_real_cinema == true)
                                film_mode.film_status= 28; // enable film detection / 3:2 / 2:2
                        else
                                film_mode.film_status= 0;

                        drvif_module_film_mode((DRV_film_mode *) &film_mode);
                        down(&Memc_Realcinema_Semaphore);
                        if (film_mode.film_status ==0)
                                bCinemaMode = 0;
                        else
                                bCinemaMode = 1;
                        
                        //memc_realcinema_framerate();
                        DbgSclrFlgTkr.memc_realcinema_run_flag = TRUE;//Run memc mode
                        
                        fwif_color_set_cinema_mode_en(bCinemaMode);
                        ret = Scaler_MEMC_set_cinema_mode_en(bCinemaMode);
                        up(&Memc_Realcinema_Semaphore);


                        memcpy(&film_table,&tFilmTable,sizeof(film_table));
			drvif_module_film_setting_tv006((DRV_film_table_t *) &film_table);

                }
                break;

                case V4L2_CID_EXT_VPQ_LOW_DELAY_MODE:
                {
			game_process = 0;
			if (Get_Factory_SelfDiagnosis_Mode()) {
				/* self diagnosis not support low delay setting*/
				ret = 0;
			} else {

				static bool pre_lowdelaymode;

                                v4l2_vpq_lowdelaymode = controls.value;

				if (rpc_vip_sysInfo == NULL) {
					rtd_pr_vpq_emerg("rpc system info = NULL,.V4L2_CID_EXT_VPQ_LOW_DELAY_MODE\n");
				} else {
					rpc_vip_sysInfo->Low_Delay_Status[PQ_Low_Delay_Mode] = v4l2_vpq_lowdelaymode;
				}

                                game_process = 1;
				fw_scalerip_set_di_gamemode(v4l2_vpq_lowdelaymode);
				pre_lowdelaymode = v4l2_vpq_lowdelaymode;
				if(((v4l2_vpq_lowdelaymode == FALSE) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE))||(get_vsc_run_pc_mode() == TRUE&&(Get_DisplayMode_Src(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)) == VSC_INPUTSRC_HDMI))){ //not game mode and not _MODE_STATE_ACTIVE
					//fw_scalerip_set_di_gamemode(lowdelaymode);
					//drv_memory_set_game_mode_dynamic(_DISABLE);
					fw_scalerip_set_di_gamemode_setting(_DISABLE);
					//ret = 0;
					//break;
				}
				if (g_bDIGameModeOnlyDebug) //for di game mode debug, only can enabled by rtice tool
					return ret;
				rtd_printk(KERN_EMERG,"VPQ", "##############[MEMC]VPQ_IOC_SET_LOWDELAY_MODE = %d \n",v4l2_vpq_lowdelaymode);

				if(v4l2_vpq_lowdelaymode) {
					//printk(KERN_EMERG"[crixus]VPQ_IOC_SET_LOWDELAY_MODE1\n");
					//game mode dynamic, check in vsc task @Crixus 20151228
					drv_memory_set_game_mode_dynamic(_ENABLE);
					//[WOSQRTK-12780] call MEMC low delay after force bg, to avoid video shake issue
					//vpq_memc_set_lowDelayMode(_ENABLE);
					//Move to new_game_mode_tsk to sync with scaler game mode process
					//Scaler_MEMC_Set_VpqLowDelayToMEMC_Flag(1);
				} else {
					//printk(KERN_EMERG"[crixus]VPQ_IOC_SET_LOWDELAY_MODE0\n");
					//game mode dynamic, check in vsc task @Crixus 20151228
					drv_memory_set_game_mode_dynamic(_DISABLE);
					//when exit game mode, clear this flag to avoid scaler re-run with data frc mode.
					scaler_vsc_set_gamemode_go_datafrc_mode(FALSE);
					//[WOSQRTK-12780] call MEMC low delay after force bg, to avoid video shake issue
					//vpq_memc_set_lowDelayMode(_DISABLE);
					//Move to new_game_mode_tsk to sync with scaler game mode process
					//Scaler_MEMC_Set_VpqLowDelayToMEMC_Flag(0);
				}

				//Game mode cmd checking flow @Cixus 20161204
				if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE){
					down(get_gamemode_check_semaphore());
					/*overflow protection*/
					if(game_mode_cmd_check == 0xffffffff)
						game_mode_cmd_check = 0;
					else if(v4l2_vpq_lowdelaymode != drv_memory_get_game_mode_flag())	// game_mode_cmd_check + 1 when current  != AP setting in picture mode
						game_mode_cmd_check = game_mode_cmd_check + 1;
					up(get_gamemode_check_semaphore());
					//printk(KERN_EMERG"[crixus]game_mode_cmd_check = %d\n", game_mode_cmd_check);
				}
				pr_notice("[%s][%d][LowDelayMode][memc,%d,vpq,%d,]", __FUNCTION__, __LINE__, fV4l2_memc_lowdeay, v4l2_vpq_lowdelaymode);
				ret = 0;
			}
		}
                break;
                case V4L2_CID_EXT_HDR_INV_GAMMA:
                {

#ifndef CONFIG_HDR_SDR_SEAMLESS	/* seamless need to check force bg and mode active*/
                        drvif_DM2_GAMMA_Enable(controls.value);
#else
                        if ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE) && (Check_AP_Set_Enable_ForceBG(SLR_MAIN_DISPLAY) == FALSE))
                        {
                                //g_InvGamma = controls.value;
					//fwif_DM2_GAMMA_Enable(controls.value);			// control by HFC in v4l2
                                rtd_pr_vpq_info("\n HDR_MODE,SEAMLESS_FLOW SET_HDR_InvGamma:%d \n",controls.value);
                                /* seamless. do seamless process in isr (void Scaler_hdr_setting_SEAMLESS()), avoid transient noise*/;
                        } else {

                                rtd_pr_vpq_info("\n HDR_MODE, SET_HDR_InvGamma :%d \n ",controls.value);

                                //drvif_DM2_GAMMA_Enable(HDR_InvGamma_en);
                               //fwif_DM2_GAMMA_Enable(controls.value);			// control by HFC in v4l2
                        }
                        //HDR_SDR_SEAMLESS_PQ.HDR_SDR_SEAMLESS_PQ_SET[HDR_SDR_SEAMLESS_PQ_HAL_PQModeInfo]=1;
#endif


                }
                break;
                case V4L2_CID_EXT_MEMC_INIT:
                {
                        rtd_pr_vpq_debug("##############[MEMC]VPQ_V4L2_IOC_MEMC_INITILIZE\n");
                        HAL_VPQ_MEMC_Initialize();

                }
                break;

                case V4L2_CID_EXT_MEMC_LOW_DELAY_MODE:
                {
                        fV4l2_memc_lowdeay = controls.value;
                        ret = HAL_VPQ_MEMC_LowDelayMode(fV4l2_memc_lowdeay);
				pr_notice("[%s][%d][LowDelayMode][memc,%d,vpq,%d,]", __FUNCTION__, __LINE__, fV4l2_memc_lowdeay, v4l2_vpq_lowdelaymode);
                }
                break;

                case V4L2_CID_EXT_MEMC_MOTION_PRO:
                {
			/*BFI function Control*/
			V4l2_memc_motion_pro =controls.value;
			rtd_pr_vpq_info("V4L2 bBFI_En=%d,\n",V4l2_memc_motion_pro);
			//rtd_pr_vpq_info("webos_tooloption.eBackLight=%d\n", webos_tooloption.eBackLight);
			rtd_pr_vpq_info("webos_tooloption.eBackLight=%s\n", webos_strToolOption.eBackLight);
			//if (webos_tooloption.eBackLight == 2) { // OLED from tool option
			if (strcmp(webos_strToolOption.eBackLight, "oled") == 0) {

                     		if(BFI_init!=0) //need waiting for BFI curve enable
			        fwif_color_set_BFI_En_OLED(V4l2_memc_motion_pro);
			}
			else{
				fwif_color_set_BFI_En_by_UI(V4l2_memc_motion_pro);
			}
			tmp++;
                }

                break;
                case V4L2_CID_EXT_LED_DB_IDX:
                {
                        extern unsigned char vpq_led_LD_lutTableIndex;

                        vpq_led_LD_lutTableIndex = controls.value;

                        //if (PQ_LED_Dev_Status != PQ_LED_DEV_INIT_DONE)
                        //        return -1;
			rtd_pr_vpq_info("V4L2 LD_LUT=%d,\n", vpq_led_LD_lutTableIndex);

                        fwif_color_set_LDSetLUT(vpq_led_LD_lutTableIndex);

                }
                break;
                case V4L2_CID_EXT_LED_EN:
                {
                        extern unsigned char vpq_led_LDEnable;
                        extern unsigned char vpq_ld_running_flag;

                        down(&VPQ_ld_running_Semaphore);
                        vpq_ld_running_flag = TRUE;
                        vpq_led_LDEnable = controls.value;
                        up(&VPQ_ld_running_Semaphore);
                }
                break;

                case V4L2_CID_EXT_LED_FIN:
                {
                        if (PQ_LED_DEV_INIT_DONE == PQ_LED_Dev_Status) {
                                PQ_LED_Dev_Status = PQ_LED_DEV_UNINIT;
                        }
                        else
                                return -1;

                        rtd_pr_vpq_emerg("##############V4L2_CID_EXT_LED_FIN UNINITILIZE\n");

                }
                break;
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
                case V4L2_CID_EXT_VPQ_STEREO_FACE_CTRL:
		{
			rtd_pr_vpq_emerg("aipq V4L2_CID_EXT_VPQ_STEREO_FACE_CTRL mode: %d \n", controls.value);
			#ifndef CONFIG_RTK_AI_DBG_DRV
			if (get_platform() == PLATFORM_KXLP)
				return -1;
			#endif		                	

			if(get_svp_protect_status_aipq())
			{
				rtd_pr_vpq_emerg("aipq protect status (V4L2_CID_EXT_VPQ_STEREO_FACE_CTRL mode: %d, eBackLight %s)\n", controls.value, webos_strToolOption.eBackLight);
				return -1;
			}

			// 0622 lsy
			#if 0
			vpq_stereo_face_pre = vpq_stereo_face;
			vpq_stereo_face = controls.value;
			ret = vpq_v4l2_ai_ctrl(controls.value, dynamic_ctrl.uDcVal);

			if(ret == -1)
				vpq_stereo_face = vpq_stereo_face_pre;

			// 0604 lsy
			vpq_stereo_face_secure = vpq_stereo_face;
			// end 0604 lsy
			#endif

			ret = vpq_v4l2_ai_ctrl(controls.value, dynamic_ctrl.uDcVal);
			rtd_pr_vpq_emerg("aipq V4L2_CID_EXT_VPQ_STEREO_FACE_CTRL mode: ret=%d \n", ret);
			if(ret == -1)
				return ret;
			
			aipq_mode_pre.face_mode = controls.value;

			// end 0622 lsy
		}
                break;

                case V4L2_CID_EXT_AIPQ_SQM_MODE:
		{
			
			rtd_pr_vpq_emerg("aipq V4L2_CID_EXT_AIPQ_SQM_MODE mode: %d \n", controls.value);
			#ifndef CONFIG_RTK_AI_DBG_DRV
			if (get_platform() == PLATFORM_KXLP)
				return -1;
			#endif		                	

			if(get_svp_protect_status_aipq())
			{
				rtd_pr_vpq_emerg("aipq protect status (V4L2_CID_EXT_AIPQ_SQM_MODE mode: %d, eBackLight %s)\n", controls.value, webos_strToolOption.eBackLight);
				return -1;
			}


			ret = vpq_v4l2_ai_sqm_ctrl(controls.value);
			rtd_pr_vpq_emerg("aipq V4L2_CID_EXT_AIPQ_SQM_MODE mode: ret=%d \n", ret);

			if(ret == -1)
				return ret;
			
			aipq_mode_pre.sqm_mode = controls.value;
		}
                break;
// end 0622 lsy 

#endif
		case V4L2_CID_EXT_AIPQ_SR_MODE:
		{
			rtd_pr_vpq_emerg("aipq V4L2_CID_EXT_AIPQ_SR_MODE mode: %d \n", controls.value);

			vpq_v4l2_ai_nnsr_set_En(controls.value);

			if(controls.value ==0)
				drvif_srnn_bbbsr_enable_set(0);
			else
				drvif_srnn_bbbsr_enable_set(1);

		}
		break;

		case V4L2_CID_EXT_AIPQ_DEPTH_MODE:
		{

			#ifndef CONFIG_RTK_AI_DBG_DRV
			if (get_platform() == PLATFORM_KXLP)
				return -1;
			#endif		                	

			if(get_svp_protect_status_aipq())
			{
				rtd_pr_vpq_emerg("aipq protect status (V4L2_CID_EXT_AIPQ_DEPTH_MODE mode: %d)\n", controls.value);
				return -1;
			}

			rtd_pr_vpq_emerg("aipq V4L2_CID_EXT_AIPQ_DEPTH_MODE mode: %d\n", controls.value);

			ret = vpq_v4l2_ai_sqm_ctrl(controls.value);

			if(ret == -1)
				return ret;
			
			aipq_mode_pre.sqm_mode = controls.value;

		}
		break;



		

#ifdef	CONFIG_RTK_KDRV_DV
		case V4L2_CID_EXT_DOLBY_PWM_RATIO:
		{
			int pwm_ratio = controls.value;

			dolby_adapter_set_dolby_pwm_ratio(pwm_ratio);
			dolby_adapter_set_hdmi_ui_change_flag(TRUE);
		}
		break;
#endif

                default:
                  ret=-1;
                    break;
        }


    return ret;
}


int vpq_v4l2_main_ioctl_g_ctrl(struct file *file, void *fh, struct v4l2_control *ctrl)
{
        int ret = 0;
        struct v4l2_control controls;
        unsigned int cmd = 0xff;
        //unsigned int filter_cmd=0;;

        if(!ctrl)
        {
            rtd_pr_vpq_emerg("func:%s [error] ctrls is null\r\n",__FUNCTION__);
            ret = -EFAULT;
            return ret;
        }
        memcpy(&controls, ctrl, sizeof(struct v4l2_control));

        cmd = controls.id;
        V4L2printf_cmd(" VPQ_V4L2 :g_ctrl , cmd ID =0x%x ,value=%d", cmd,controls.value);
	if (vpq_v4l2_main_get_stop_run(cmd))
		return 0;

        switch (cmd)
        {
                case V4L2_CID_EXT_LED_EN:
                {
                        extern unsigned char vpq_led_LDEnable;
                        ctrl->value = vpq_led_LDEnable;

                }
                break;
                case V4L2_CID_EXT_LED_DB_IDX:
                {
                        extern unsigned char vpq_led_LD_lutTableIndex;

                        ctrl->value =vpq_led_LD_lutTableIndex;

                }
                break;
                case V4L2_CID_EXT_MEMC_LOW_DELAY_MODE:
                {

                        ctrl->value =fV4l2_memc_lowdeay;

                }
                break;
                case V4L2_CID_EXT_MEMC_MOTION_PRO:
                {
                        ctrl->value = V4l2_memc_motion_pro;
                        //current not used;
                }

                break;

                case V4L2_CID_EXT_VPQ_TESTPATTERN:
                {
                       ctrl->value =v4l2_vpq_ire;
                }
                break;
                case V4L2_CID_EXT_VPQ_REAL_CINEMA:
                {
                        ctrl->value =v4l2_real_cinema;
                }
                break;
                case V4L2_CID_EXT_VPQ_LOW_DELAY_MODE:
                {
                        ctrl->value = v4l2_vpq_lowdelaymode;
		}
                break;
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
                case V4L2_CID_EXT_VPQ_STEREO_FACE_CTRL:
                {
                        ctrl->value = aipq_mode.face_mode;
                }
                break;
                case V4L2_CID_EXT_AIPQ_SQM_MODE:
                {
                        ctrl->value = aipq_mode.sqm_mode;
                }
                break;

// end 0622 lsy
#endif
		case V4L2_CID_EXT_AIPQ_SR_MODE:
		{

			controls.value =vpq_v4l2_ai_nnsr_get_En();


		}
		break;

#ifdef	CONFIG_RTK_KDRV_DV
		case V4L2_CID_EXT_DOLBY_PWM_RATIO:
		{
			ctrl->value = dolby_adapter_get_dolby_pwm_ratio();
		}
		break;
                case V4L2_CID_EXT_DOLBY_SW_VERSION:
                {
			/* FIXME: need fix the api spec
			 *        this api using 32bit value to carry userspace pointer */
			/*
			 * const char *dolby_version = dolby_adapter_get_dolby_vision_software_version();
			 * char __user *user_ptr = (char __user *)ctrl->value;

			 * if (dolby_version) {
			 * 	if(copy_to_user(user_ptr, dolby_version, strlen(dolby_version)))
			 * 		ret = -EINVAL;
			 * }
			 */
		}
                break;
#endif
				case V4L2_CID_EXT_VPQ_VIDEO_LATENCY:
				{
						ctrl->value = rtk_scaler_calc_current_video_path_latency()/10;
				}
				break;


                default:
                        ret=-1;
                break;
        }
        return ret;
}


unsigned char V4L2_VPQ_Calc_Step_Power(unsigned short* lut_in_17, unsigned short* lut_out_16, unsigned char bCheckPow2)
{
	int i, j;
	int diff, temp1, temp2, idx, sum = 0;
	unsigned short step[16] = {0};
	unsigned short default_lut[16] = {0xf, 0x1f, 0x2f, 0x3f, 0x4f, 0x5f, 0x6f, 0x7f, 0x8f, 0x9f, 0xaf, 0xbf, 0xcf, 0xdf, 0xef, 0xff};
	unsigned char pow2[7] = {1, 2, 4, 8, 16, 32, 64};

	if (NULL == lut_in_17 || NULL == lut_out_16) {
		return FALSE;
	}

	for (i = 1; i < 17; i++)
	{
		diff=lut_in_17[i]-lut_in_17[i-1];
		if (i == 1)
			diff = diff+1;

		if (diff < 0 || (diff == 0 && i > 1)) {
			memcpy(lut_out_16, default_lut, sizeof(default_lut));
			return FALSE;
		}

		if (bCheckPow2) {
			for (j = 0; j < 7; j++) {
				if (diff == pow2[j]) {
					step[i-1] = diff;
					sum += diff;
					break;
				}
			}
			if (j == 7) {
				memcpy(lut_out_16, default_lut, sizeof(default_lut));
				return FALSE;
			}
		} else {
			if (diff > 64)
				diff = 64;
			temp1 = 128;
			idx = 0;
			for (j = 0; j < 7; j++)
			{
				temp2 = abs(pow2[j]-diff);
				if (temp2 < temp1) {
					temp1 = temp2;
					idx = j;
				}
			}
			step[i-1]=pow2[idx];
			sum += pow2[idx];
		}
	}

	if (sum > 256) {
		if ((sum - (step[15] >> 1)) > 256) {
			memcpy(lut_out_16, default_lut, sizeof(default_lut));
			return FALSE;
		}

		step[15] = step[15] >> 1;
	}

	lut_out_16[0] = step[0]-1;
	for (i = 1; i < 16; i++)
	{
		lut_out_16[i] = lut_out_16[i-1]+step[i];
	}

	return TRUE;
}

#endif

#if defined(CONFIG_RTK_8KCODEC_INTERFACE)
#include "kernel/rtk_codec_interface.h"
extern struct rtk_codec_interface *rtk_8k;
//rtk_8k->vpq->info();
#endif

#ifndef UT_flag

struct file_operations vpq_fops = {
	.owner = THIS_MODULE,
	.open = vpq_open,
	.release = vpq_release,
	.read  = vpq_read,
	.write = vpq_write,
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	.poll  = vpq_poll,
#endif
	.unlocked_ioctl = vpq_ioctl,
#ifdef CONFIG_ARM64
#ifdef CONFIG_COMPAT
	.compat_ioctl = vpq_ioctl,
#endif
#endif
};

#ifdef CONFIG_PM
static const struct dev_pm_ops vpq_pm_ops =
{
        .suspend    = vpq_suspend,
        .resume     = vpq_resume,
    .runtime_suspend = vpq_rumtime_suspend,
    .runtime_resume = vpq_rumtime_resume,
    //.runtime_idle = vpq_rumtime_idle,                
#ifdef CONFIG_HIBERNATION
	.freeze 	= vpq_suspend_std,
	.thaw		= vpq_resume_std,
	.poweroff	= vpq_suspend_std,
	.restore	= vpq_resume_std,
#endif

};
#endif // CONFIG_PM

static struct class *vpq_class = NULL;
//static struct platform_device *vpq_platform_devs = NULL;
static struct platform_driver vpq_platform_driver = {
	.driver = {
		.name = "vpqdev",
		.bus = &platform_bus_type,
#ifdef CONFIG_PM
		.pm = &vpq_pm_ops,
#endif
    },
} ;

#ifdef CONFIG_ARM64 //ARM32 compatible
static char *vpq_devnode(struct device *dev, umode_t *mode)
#else
static char *vpq_devnode(struct device *dev, mode_t *mode)
#endif
{
	return NULL;
}

typedef struct{
	unsigned long para1;
	unsigned long para2;
} PQ_THREAD_PARAM;

enum {
	RPC_V2K_FW_FILM = 0,
	RPC_V2K_MAX
};

static struct task_struct *p_pq_tsk = NULL;
unsigned char vgip_isr_flag = FALSE;
static int vpq_vgip_isr_tsk(void *p)
{
	PQ_THREAD_PARAM *pParam = (PQ_THREAD_PARAM *)p;

	switch(pParam->para1&0xff) {
	case RPC_V2K_FW_FILM:
	{
		extern unsigned char g_st_DI_STATUS_VPAN;
		g_st_DI_STATUS_VPAN = pParam->para2&0xff;
		scalerVIP_film_FWmode();
		break;
	}
	default:
		;
	}
	vgip_isr_flag = FALSE;
	return 0;
}

unsigned int rpc_vgip_isr_vact_start(unsigned long para1, unsigned long para2)
{
	int err = 0;
	static PQ_THREAD_PARAM arg;
	arg.para1 = para1;
	arg.para2 = para2;

	if (!vgip_isr_flag) {
		p_pq_tsk = kthread_create(vpq_vgip_isr_tsk, (void *)&arg, "vpq_vgip_isr_tsk");

		if (p_pq_tsk) {
			    wake_up_process(p_pq_tsk);
			    vgip_isr_flag = TRUE;
		} else {
		    err = PTR_ERR(p_pq_tsk);
		    rtd_pr_vpq_err("Unable to start rpc_vgip_isr_vact_start (err_id = %d)./n", err);
		}
	}/* else
		rtd_pr_vpq_debug("rpc_vgip_isr_vact_start vgip_isr_flag = TRUE\n");*/

	return 0;
}





int vpq_module_init(void)
{
	int result;

	printk("vpq_module_init %d\n", __LINE__);
	result = alloc_chrdev_region(&vpq_devno, 0, 1, "vpqdevno");

	if (result != 0) {
		rtd_pr_vpq_err("Cannot allocate VPQ device number\n");
		return result;
	}

	vpq_class = class_create(THIS_MODULE, "vpqdev");
	if (IS_ERR(vpq_class)) {
		rtd_pr_vpq_err("scalevpq: can not create class...\n");
		result = PTR_ERR(vpq_class);
		goto fail_class_create;
	}

	vpq_class->devnode = vpq_devnode;

	vpq_platform_devs = platform_device_register_simple("vpqdev", -1, NULL, 0);
	if (platform_driver_register(&vpq_platform_driver) != 0) {
		rtd_pr_vpq_err("scalevpq: can not register platform driver...\n");
		result = -ENOMEM;
		goto fail_platform_driver_register;
	}

	cdev_init(&vpq_cdev, &vpq_fops);
	result = cdev_add(&vpq_cdev, vpq_devno, 1);
	if (result < 0) {
		rtd_pr_vpq_err("scalevpq: can not add character device...\n");
		goto fail_cdev_init;
	}

	device_create(vpq_class, NULL, vpq_devno, NULL, "vpqdev");

	sema_init(&VPQ_Semaphore, 1);
	sema_init(&VPQ_DM_DMA_TBL_Semaphore, 1);
	sema_init(&VPQ_ICM_Semaphore, 1);
	//sema_init(&VPQ_DM_3DLUT_Semaphore,1);
	//sema_init(&VPQ_DM_EOTF_Semaphore,1);
	//sema_init(&VPQ_DM_OETF_Semaphore,1);
	sema_init(&Gamma_Semaphore,1);
	sema_init(&VPQ_I_CSC_Semaphore,1);
	sema_init(&VPQ_DI_RTNR_CONTROL_Semaphore,1);
	sema_init(&Orbit_Semaphore,1);
	sema_init(&VPQ_HDR_CSC_Semaphore,1);
	sema_init(&VPQ_Demura_Semaphore,1);
	sema_init(&VPQ_DM_Y_histogram_ctrl_reg_Semaphore,1);
	sema_init(&VPQ_IP_Bypass_Semaphore,1);
	sema_init(&VPQ_NNSR_Model_TBL_Semaphore, 1);

	vpq_rtpm_tsk_VPQ_freeze_stauts = 0;
	vpq_rtpm_tsk_SE_freeze_stauts = 0;
	vpq_rtpm_tsk_Film_freeze_stauts = 0;
	// for low power mode, run time pm
	vpq_rtpm_tsk_block = 0;		// init status is on for socts HEVC test. so vsc need to set tsk block while vsc is suspend for VSC socts.
	vpq_rtpm_tsk_block_VPQ_done = 0;		// default tsk is on, so this flag need to be set by tsk
	vpq_rtpm_tsk_block_SE_done = 0;		// default tsk is on, so this flag need to be set by tsk
	vpq_rtpm_tsk_block_Film_done = 0;		// default tsk is on, so this flag need to be set by tsk
#ifdef VPQ_RunTime_PM_Enable	
	pm_runtime_forbid(&vpq_platform_devs->dev);
	pm_runtime_set_active(&vpq_platform_devs->dev);
	pm_runtime_enable(&vpq_platform_devs->dev);	
	//rtd_pr_vpq_info("VPQ_RTPM, module ini\n");
#ifdef CONFIG_RTK_LOW_POWER_MODE
	rtk_lpm_add_device(&vpq_platform_devs->dev);
#endif
	VPQ_PM_LOG("info", "start", "VPQ_RTPM module init, enable");
#endif	

	if(get_MEMC_bypass_status_refer_platform_model() != TRUE){
		create_film_fw_tsk();//Create film mode detect task
	}
	create_VPQ_tsk();
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	create_se_tsk();
#if 0
	// TODO : thread get stuck in Android, current trigger in se_tsk
	PQMask_CreateThread();
#endif
#endif
	if (0x00060000 == vpq_project_id) {
		vpq_boot_init();
	} else
		fwif_color_set_pq_demo_flag_rpc(1);
	fwif_color_reg_demo_callback(DEMO_CALLBACKID_ON_OFF_SWITCH, vpq_demo_pq_func);
	fwif_color_reg_demo_callback(DEMO_CALLBACKID_OVERSCAN, vpq_demo_overscan_func);
	//register_kernel_rpc(RPC_VCPU_ID_0x130_VGIP_ISR_VACT_START, (FUNC_PTR)&rpc_vgip_isr_vact_start);

	//idomain mpeg double buffer enable https://jira.realtek.com/browse/RL6557-2310
	IoReg_SetBits(MPEGNR_ICH1_MPEGNR_DB_CTRL_reg, MPEGNR_ICH1_MPEGNR_DB_CTRL_mpegnr_db_en_mask);
	//idomain RGB2YUV double buffer enable https://jira.realtek.com/browse/RL6557-2311
	IoReg_SetBits(RGB2YUV_ICH1_RGB2YUV_CTRL_reg, RGB2YUV_ICH1_RGB2YUV_CTRL_rgb2yuv_db_en_mask);
	IoReg_SetBits(RGB2YUV_ICH1_422to444_CTRL_DB_reg, RGB2YUV_ICH1_422to444_CTRL_DB_db_en_mask);
	IoReg_SetBits(RGB2YUV_ICH2_422to444_CTRL_reg, RGB2YUV_ICH2_422to444_CTRL_db_en_mask);
	//idomain SPNR double buffer enable https://jira.realtek.com/browse/RL6557-2312
	IoReg_SetBits(NR_SNR_DB_CTRL_reg, NR_SNR_DB_CTRL_snr_db_en_mask);
	return 0;/*Success*/

fail_cdev_init:
	platform_driver_unregister(&vpq_platform_driver);
fail_platform_driver_register:
	platform_device_unregister(vpq_platform_devs);
	vpq_platform_devs = NULL;
	class_destroy(vpq_class);
fail_class_create:
	vpq_class = NULL;
	unregister_chrdev_region(vpq_devno, 1);
	return result;

}




void __exit vpq_module_exit(void)
{
	if (vpq_platform_devs == NULL)
		BUG();

	delete_film_fw_tsk();
	delete_VPQ_tsk();
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	delete_se_tsk();
#if 0
	// TODO : thread get stuck in Android, current trigger in se_tsk
	PQMask_DeleteThread();
#endif
	ai_deinit_debug_proc();
#if IS_ENABLED(CONFIG_RTK_AI_OPTEE)
	ai_optee_deinit();
#endif//CONFIG_RTK_AI_OPTEE
#endif
	device_destroy(vpq_class, vpq_devno);
	cdev_del(&vpq_cdev);

	platform_driver_unregister(&vpq_platform_driver);
	platform_device_unregister(vpq_platform_devs);
	vpq_platform_devs = NULL;

	class_destroy(vpq_class);
	vpq_class = NULL;

	unregister_chrdev_region(vpq_devno, 1);
}

#ifdef CONFIG_SUPPORT_SCALER_MODULE
// the module init/exit will be moved to scaler_module.c if scaler was built as a kernel module
#else
module_init(vpq_module_init);
module_exit(vpq_module_exit);
#endif

#if defined(CONFIG_RTK_AI_DRV_MODULE)
EXPORT_SYMBOL(aipq_mode);
#endif
#endif //UT_flag

#endif

unsigned char Get_Var_Demo_Flag(void) {
	return Demo_Flag;
}
void Set_Var_Demo_Flag(unsigned char value) {
	Demo_Flag = value;
}
unsigned char Get_Var_PQ_Dev_Status(void) {
	return PQ_Dev_Status;
}
void Set_Var_PQ_Dev_Status(unsigned char value) {
	PQ_Dev_Status = value;
}
unsigned char Get_Var_g_bDIGameModeOnlyDebug(void) {
	return g_bDIGameModeOnlyDebug;
}
void Set_Var_g_bDIGameModeOnlyDebug(unsigned char value) {
	g_bDIGameModeOnlyDebug = value;
}
unsigned char Get_Var_g_bAPLColorGainClr(void) {
	return g_bAPLColorGainClr;
}
void Set_Var_g_bAPLColorGainClr(unsigned char value) {
	g_bAPLColorGainClr = value;
}
unsigned char Get_Var_ucColorFilterMode(void) {
	return ucColorFilterMode;
}
void Set_Var_ucColorFilterMode(unsigned char value) {
	ucColorFilterMode = value;
}
unsigned char Get_Var_game_process(void) {
	return game_process;
}
void Set_Var_game_process(unsigned char value) {
	game_process = value;
}
unsigned char Get_Var_g_LGE_HDR_CSC_CTRL(void) {
	return g_LGE_HDR_CSC_CTRL;
}
void Set_Var_g_LGE_HDR_CSC_CTRL(unsigned char value) {
	g_LGE_HDR_CSC_CTRL = value;
}
unsigned int *Get_Var_PQModeInfo_flag(void) {
	return PQModeInfo_flag;
}
unsigned char Get_Var_g_srgbForceUpdate(void) {
	return g_srgbForceUpdate;
}
void Set_Var_g_srgbForceUpdate(unsigned char value) {
	g_srgbForceUpdate = value;
}
unsigned char Get_Var_g_3DLUT_LastInternalSelect(void) {
	return g_3DLUT_LastInternalSelect;
}
void Set_Var_g_3DLUT_LastInternalSelect(unsigned char value) {
	g_3DLUT_LastInternalSelect = value;
}
unsigned char Get_Var_g_3DLUT_LastEnableSatus(void) {
	return g_3DLUT_LastEnableSatus;
}
void Set_Var_g_3DLUT_LastEnableSatus(unsigned char value) {
	g_3DLUT_LastEnableSatus = value;
}
unsigned char Get_Var_g_3DLUT_Resume(void) {
	return g_3DLUT_Resume;
}
void Set_Var_g_3DLUT_Resume(unsigned char value) {
	g_3DLUT_Resume = value;
}
unsigned char Get_Var_g_InvGammaPowerMode(void) {
	return g_InvGammaPowerMode;
}
void Set_Var_g_InvGammaPowerMode(unsigned char value) {
	g_InvGammaPowerMode = value;
}
GAMUT_3D_LUT_17x17x17_T *Get_Var_Ptr_g_buf3DLUT_LGDB() {
	return &g_buf3DLUT_LGDB;
}
unsigned char Get_Var_g_Color_Mode(void) {
	return g_Color_Mode;
}
void Set_Var_g_Color_Mode(unsigned char value) {
	g_Color_Mode = value;
}
unsigned char Get_Var_g_flag_cm_adv_init_ok(void) {
	return g_flag_cm_adv_init_ok;
}
void Set_Var_g_flag_cm_adv_init_ok(unsigned char value) {
	g_flag_cm_adv_init_ok = value;
}
unsigned char Get_Var_g_flag_cm_exp_init_ok(void) {
	return g_flag_cm_exp_init_ok;
}
void Set_Var_g_flag_cm_exp_init_ok(unsigned char value) {
	g_flag_cm_exp_init_ok = value;
}
unsigned char Get_Var_g_cm_need_refresh(void) {
	return g_cm_need_refresh;
}
void Set_Var_g_cm_need_refresh(unsigned char value) {
	g_cm_need_refresh = value;
}
CHIP_CM_REGION_T *Get_Var_Ptr_g_cm_rgn_adv(void) {
	return &g_cm_rgn_adv;
}
CHIP_CM_REGION_T *Get_Var_Ptr_g_cm_rgn_exp(void) {
	return &g_cm_rgn_exp;
}
CHIP_COLOR_CONTROL_T *Get_Var_Ptr_g_cm_ctrl_adv(void) {
	return &g_cm_ctrl_adv;
}
CHIP_COLOR_CONTROL_T *Get_Var_Ptr_g_cm_ctrl_exp(void) {
	return &g_cm_ctrl_exp;
}
CHIP_CM_REGION_EXT_T *Get_Var_Ptr_cmRegionExt(void) {
	return &cmRegionExt;
}
unsigned char *Get_Var_Ptr_deMura_7Blk_Decode_TBL(void) {
	return deMura_7Blk_Decode_TBL;
}
void Set_Var_vpq_project_id(unsigned int value) {
	vpq_project_id = value;
}
