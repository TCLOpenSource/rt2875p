#ifndef BUILD_QUICK_SHOW

/*Kernel Header file*/
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/fs.h>		/* everything... */
#include <linux/cdev.h>
#include <linux/platform_device.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/pageremap.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/hrtimer.h>
#include <linux/vmalloc.h>
#include <linux/pm_runtime.h>
#include <linux/suspend.h>
#include <asm-generic/ioctl.h>

#ifdef CONFIG_ARM64
#include <asm/io.h>
#endif
#include <rtk_kdriver/quick_show/quick_show.h>
#else
#include <no_os/semaphore.h>
#include <no_os/slab.h>
#include <asm-generic/ioctl.h>
#include <include/string.h>
#include <sysdefs.h>

#endif

/*RBUS Header file*/
#include <scaler/scalerCommon.h>

/*TVScaler Header file*/
#include "tvscalercontrol/io/ioregdrv.h"
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>
#include <tvscalercontrol/vip/viptable.h>
#include <scaler_vpqmemcdev.h>
#include "scaler_vpqdev.h"
#include "scaler_vscdev.h"
#include "ioctrl/vpq/vpq_cmd_id.h"
#include <tvscalercontrol/vip/pq_rpc.h>
//#include <mach/RPCDriver.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/pll27x_reg_reg.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/gibi_od.h>
#include <tvscalercontrol/vip/pcid.h>
#include <tvscalercontrol/vip/ultrazoom.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/scalerdrv/scalerip.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <rtd_log/rtd_module_log.h>
#include <tvscalercontrol/panel/panelapi.h>
#ifdef CONFIG_RTK_LOW_POWER_MODE
#include <rtk_kdriver/rtk-kdrv-common.h>
#endif
//#include <tvscalercontrol/vo/rtk_vo.h>
#ifndef BUILD_QUICK_SHOW

#include "vgip_isr/scalerAI.h"
#include "vgip_isr/scalerVIP.h"
//#include <rtk_ai.h>
#include <rtk_kdriver/rtk_semaphore.h>
#endif



#ifdef UT_flag
#ifdef memcmp
#undef memcmp
#endif
#include <string.h>
#endif

static struct semaphore VPQ_Semaphore;
struct semaphore VPQ_DM_DMA_TBL_Semaphore;
//struct semaphore VPQ_DM_3DLUT_Semaphore;
extern struct semaphore Gamma_Semaphore;

static struct platform_device *vpq_platform_devs = NULL;	// move here for vpq run time pm
static unsigned char vpq_rtpm_tsk_block = 0;
static unsigned char vpq_rtpm_tsk_block_VPQ_done = 0;
static unsigned char vpq_rtpm_tsk_VPQ_freeze_stauts = 0;
static bool VPQ_fw_tsk_running_flag = FALSE;
static struct task_struct *p_VPQ_task = NULL;


#define VPQ_PM_REFCNT (atomic_read(&(&vpq_platform_devs->dev)->power.usage_count))
#define VPQ_PM_STATUS (vpq_platform_devs->dev.power.runtime_status)	 // enum rpm_status { RPM_ACTIVE = 0, RPM_RESUMING,	RPM_SUSPENDED, RPM_SUSPENDING,};
#define VPQ_PM_AUTO (vpq_platform_devs->dev.power.runtime_auto)	 // runtime_auto = 1 => auto.     runtime_auto = 0 => on

#if 0 
#define VPQ_PM_LOG(PM_LV, TYPE, MESSAGE) \

		rtd_pr_vpq_info("LowPower::%s::picturequality::video50::%d::%s-%s::%s::ok::%s::rpm_status=%d::runtime_auto=%d\n", \
			PM_LV, VPQ_PM_REFCNT, (current->group_leader?current->group_leader->comm:"-"), current->comm, TYPE, MESSAGE, VPQ_PM_STATUS, VPQ_PM_AUTO);\

#endif 

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
#ifdef UT_flag
#undef VPQ_PM_LOG
#define VPQ_PM_LOG(PM_LV, TYPE, MESSAGE)
#endif 

#if 1
/* for demura hal function */
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
char *pINNX_Demura_DeLut;
#endif
unsigned char *INNX_Demura_TBL = NULL;
extern unsigned int INNX_Demura_gray_setting[7];
/*========================= for INNX demura =================================*/
#endif

extern unsigned int PCID_ValueTBL[81];
extern unsigned int POD_ValueTBL_1[81];
extern unsigned int POD_ValueTBL_2[81];

//extern UINT32 EOTF_LUT_HLG_DEFAULT[1025];
//extern UINT16 OETF_LUT_HLG_DEFAULT_DCC_OFF[1025];
extern UINT16 HDR10_3DLUT_24x24x24[24*24*24*3];
extern UINT32 EOTF_LUT_R[1025];
extern UINT16 OETF_LUT_R[1025];
unsigned char od_table_store_r[OD_table_length];
unsigned char od_table_store_g[OD_table_length];
unsigned char od_table_store_b[OD_table_length];
unsigned char od_table_store_w[OD_table_length];
unsigned int od_delta_table_resume[OD_table_length];

static unsigned char PQ_Dev_Status = PQ_DEV_NOTHING;
static unsigned char Demo_Flag = false;
UINT8 g_InvGamma=0;
static GAMMA_RGB_T gamma;
extern COLORELEM_TAB_T icm_tab_elem_of_vip_table;
extern COLORELEM_TAB_T icm_tab_elem_write;
UINT8 g_Color_Mode = COLOR_MODE_ADV;
UINT8 g_flag_cm_adv_init_ok;
UINT8 g_flag_cm_exp_init_ok;
CHIP_CM_REGION_T g_cm_rgn_adv = {{
	{{	TRUE,
		CHIP_CM_COLOR_REGION_RED,		//INDEX_0	//INDEX_1	//INDEX_2	//INDEX_3	//INDEX_4	//INDEX_5//INDEX_6//INDEX_7
		{5632, 5760, 5888, 5888, 0, 64, 128, 192 }, //HUE_index //0~6016
		{174, 220, 220, 220, 220, 220, 190, 170 }, //HUE_gain //0~255
		{384, 512, 1024, 1536, 2048, 3072, 3584, 4096 }, //SAT_index //0~4095
		{128, 215, 230, 220, 200, 201, 180, 130 }, //SAT_gain //0~255
		{512, 1024, 1536, 2048, 2560, 3072, 3584, 4096 }, //ITN_index //0~4095
		{128, 250, 250, 250, 250, 180, 180, 160 }, //ITN_gain //0~255
	},

	{	TRUE,
		CHIP_CM_COLOR_REGION_GRN,		//INDEX_0	//INDEX_1	//INDEX_2	//INDEX_3	//INDEX_4	//INDEX_5//INDEX_6//INDEX_7
		{1152, 1216, 1408, 1536, 2432, 2560, 2688, 2752 }, //HUE_index //0~6016
		{174, 220, 220, 220, 220, 220, 190, 170 }, //HUE_gain //0~255
		{384, 512, 1024, 1536, 2048, 3072, 3584, 4096 }, //SAT_index //0~4095
		{128, 215, 230, 220, 200, 201, 180, 130 }, //SAT_gain //0~255
		{512, 1024, 1536, 2048, 2560, 3072, 3584, 4096 }, //ITN_index //0~4095
		{128, 250, 250, 250, 250, 180, 180, 160 }, //ITN_gain //0~255
	},

	{	TRUE,
		CHIP_CM_COLOR_REGION_BLU,		//INDEX_0	//INDEX_1	//INDEX_2	//INDEX_3	//INDEX_4	//INDEX_5//INDEX_6//INDEX_7
		{2752, 2816, 2944, 3072, 3456, 3584, 3712, 3840 }, //HUE_index //0~6016
		{174, 218, 220, 220, 220, 220, 190, 170 }, //HUE_gain //0~255
		{384, 512, 1024, 1536, 2048, 3072, 3584, 4096 }, //SAT_index //0~4095
		{128, 193, 230, 220, 200, 201, 180, 130 }, //SAT_gain //0~255
		{512, 1024, 1536, 2048, 2560, 3072, 3584, 4096 }, //ITN_index //0~4095
		{110, 160, 220, 220, 220, 180, 180, 160 }, //ITN_gain //0~255
	},

	{	TRUE,
		CHIP_CM_COLOR_REGION_CYN,		//INDEX_0	//INDEX_1	//INDEX_2	//INDEX_3	//INDEX_4	//INDEX_5//INDEX_6//INDEX_7
		{3840, 3840, 3968, 4096, 4224, 4352, 4480, 4480 }, //HUE_index //0~6016
		{174, 174, 220, 220, 220, 220, 190, 190 }, //HUE_gain //0~255
		{384, 512, 1024, 1536, 2048, 3072, 3584, 4096 }, //SAT_index //0~4095
		{128, 206, 230, 220, 200, 201, 180, 130 }, //SAT_gain //0~255
		{0, 512, 1536, 2048, 2560, 3072, 3584, 4096 }, //ITN_index //0~4095
		{128, 250, 250, 250, 250, 180, 180, 160 }, //ITN_gain //0~255
	},

	{	TRUE,
		CHIP_CM_COLOR_REGION_MGT,		//INDEX_0	//INDEX_1	//INDEX_2	//INDEX_3	//INDEX_4	//INDEX_5//INDEX_6//INDEX_7
		{4672, 4736, 4864, 5120, 5248, 5504, 5760, 5888 }, //HUE_index //0~6016
		{174, 220, 220, 220, 220, 220, 190, 170 }, //HUE_gain //0~255
		{384, 512, 1024, 1536, 2048, 3072, 3584, 4096 }, //SAT_index //0~4095
		{128, 215, 230, 220, 200, 201, 180, 130 }, //SAT_gain //0~255
		{0, 512, 1536, 2048, 2560, 3072, 3584, 4096 }, //ITN_index //0~4095
		{128, 250, 250, 250, 250, 180, 180, 160 }, //ITN_gain //0~255
	},

	{	TRUE,
		CHIP_CM_COLOR_REGION_YLW,		//INDEX_0	//INDEX_1	//INDEX_2	//INDEX_3	//INDEX_4	//INDEX_5//INDEX_6//INDEX_7
		{640, 704, 704, 768, 832, 832, 896, 1024 }, //HUE_index //0~6016
		{174, 220, 220, 220, 220, 220, 190, 170 }, //HUE_gain //0~255
		{384, 512, 1024, 1536, 2048, 3072, 3584, 4096 }, //SAT_index //0~4095
		{128, 215, 230, 220, 200, 201, 180, 130 }, //SAT_gain //0~255
		{0, 512, 1536, 2048, 2560, 3072, 3584, 4096 }, //ITN_index //0~4095
		{128, 250, 250, 250, 250, 180, 180, 160 }, //ITN_gain //0~255
	},

	{	TRUE,
		CHIP_CM_COLOR_REGION_SKIN,		//INDEX_0	//INDEX_1	//INDEX_2	//INDEX_3	//INDEX_4	//INDEX_5//INDEX_6//INDEX_7
		{256, 320, 320, 384, 384, 384, 512, 640 }, //HUE_index //0~6016
		{174, 220, 220, 220, 220, 220, 190, 170 }, //HUE_gain //0~255
		{384, 512, 1024, 1536, 2048, 3072, 3584, 4096 }, //SAT_index //0~4095
		{128, 215, 230, 220, 200, 201, 180, 130 }, //SAT_gain //0~255
		{0, 512, 1536, 2048, 2560, 3072, 3584, 4096 }, //ITN_index //0~4095
		{128, 250, 250, 250, 250, 180, 180, 160 }, //ITN_gain //0~255
	},},
	0,
}};
CHIP_CM_REGION_T g_cm_rgn_exp;
CHIP_COLOR_CONTROL_T g_cm_ctrl_adv;
CHIP_COLOR_CONTROL_T g_cm_ctrl_exp;
unsigned char g_cm_need_refresh = 1;
unsigned char g_cm_keep_Y = 0;

extern UINT16 GOut_R[1025];
extern UINT16 GOut_G[1025];
extern UINT16 GOut_B[1025];

extern unsigned int final_GAMMA_R[512];
extern unsigned int final_GAMMA_G[512];
extern unsigned int final_GAMMA_B[512];

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

CHIP_SHARPNESS_UI_T tCurCHIP_SHARPNESS_UI_T;
CHIP_SHARPNESS_UI_T t3DCHIP_SHARPNESS_UI_T;
CHIP_EDGE_ENHANCE_UI_T tCurCHIP_EDGE_ENHANCE_UI_T;
CHIP_SR_UI_T tCurCHIP_SR_UI_T;
CHIP_LOCAL_CONTRAST_T glc_param={
	0,0,0,128,
	{0,0,1,1,2,2,2,2},
	{32,64,128,192,704,83,896,960},
	{
		0,64,128,192,256,320,384,448,512,576,640,704,768,832,896,960,1024,1088,1152,1216,1280,1344,1408,1472,1536,1600,1664,1728,1792,1856,1920,1984,
		0,64,128,192,256,320,384,448,512,576,640,704,768,832,896,960,1024,1088,1152,1216,1280,1344,1408,1472,1536,1600,1664,1728,1792,1856,1920,1984,
		0,128,256,384,512,640,768,896,1024,1152,1280,1408,1536,1664,1792,1920,2048,2176,2304,2432,2560,2688,2816,2944,3072,3200,3328,3456,3584,3712,3840,3968,
		0,128,256,384,512,640,768,896,1024,1152,1280,1408,1536,1664,1792,1920,2048,2176,2304,2432,2560,2688,2816,2944,3072,3200,3328,3456,3584,3712,3840,3968,
		0,2048,2112,2176,2240,2304,2368,2432,2496,2560,2624,2688,2752,2816,2880,2944,3008,3072,3136,3200,3264,3328,3392,3456,3520,3584,3648,3712,3776,3840,3904,4032,
		0,2048,2112,2176,2240,2304,2368,2432,2496,2560,2624,2688,2752,2816,2880,2944,3008,3072,3136,3200,3264,3328,3392,3456,3520,3584,3648,3712,3776,3840,3904,4032,
		0,2048,2112,2176,2240,2304,2368,2432,2496,2560,2624,2688,2752,2816,2880,2944,3008,3072,3136,3200,3264,3328,3392,3456,3520,3584,3648,3712,3776,3840,3904,4032,
		0,2048,2112,2176,2240,2304,2368,2432,2496,2560,2624,2688,2752,2816,2880,2944,3008,3072,3136,3200,3264,3328,3392,3456,3520,3584,3648,3712,3776,3840,3904,4032,
	},
};

/*sycn TV030*/
extern unsigned char VALC_Table[6][17][17];
extern DRV_pcid_valc_t  pVALCprotectSetting[2];
/*sycn TV030 end*/

//TV030
unsigned char str_resume_do_picturemode=0;

#ifdef CONFIG_RTK_SRE_AATS_UCT_AT
//AT V6.2 Patch 2022/03/31 - START
unsigned int g_ScalerApplyPQLinear = 0;
char         g_ActiveAATS_UCT_GD_On = 0;
unsigned char PQ_UT_apply_Linear_Flag;
//Required misc/rtd2851a/rtd_logger.c and emcu/rtk_kdv_emcu.c
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

//unsigned short tShp_Val[7]; //no used

unsigned char g_bAPLColorGainClr = 1;
unsigned char g_bGamut3DLUT_LGDB_NeedUpdated = 0;
unsigned char g_3DLUT_LastInternalSelect = 0; // 0: RTK DB, 1: LG DB
unsigned char g_3DLUT_LastEnableSatus = 0;
unsigned char g_3DLUT_Resume = 0;
extern GAMUT_3D_LUT_17x17x17_T g_LGDB_3D_LUT_BUF[1];
GAMUT_3D_LUT_17x17x17_T g_buf3DLUT_LGDB;

extern struct semaphore Memc_Realcinema_Semaphore;/*For adaptive streaming info Semaphore*/
extern DEBUG_SCALER_VSC_STRUCT DbgSclrFlgTkr;//extern char memc_realcinema_run_flag;

unsigned char g_bDIGameModeOnlyDebug = FALSE;

UINT8 g_pcid_en = 0;
UINT8 g_pcid2_en = 0;

extern void memc_realcinema_framerate(void);
extern unsigned char get_sub_OutputVencMode(void);

extern PQ_device_struct *g_Share_Memory_PQ_device_struct;

#ifdef CONFIG_HDR_SDR_SEAMLESS
extern HDR_SDR_SEAMLESS_PQ_STRUCT HDR_SDR_SEAMLESS_PQ;
#endif

#if defined(CONFIG_RTK_AI_DRV)
UINT8 vpq_stereo_face = 0;//0: off, 1: on, 2: demo
extern unsigned char signal_cnt;
#endif

extern void vsc_runtime_pm_get(void); //wait_scaler_patch
extern void vsc_runtime_pm_put(void); //wait_scaler_patch

#ifndef BUILD_QUICK_SHOW

#ifndef UT_flag
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
        printk(KERN_EMERG "####[David] %s buffer is empty\n", __func__);
        return 0;
    }

    if(count == 0 ) {
        printk(KERN_EMERG "####[David] %s count  is empty\n", __func__);
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

ssize_t vpq_write(struct file *filep, const char *buffer, size_t count, loff_t *offp)
{

	long ret = count;
	char cmd_buf[100] = {0};

	#ifndef CONFIG_ARM64
	pr_notice("%s(): count=%d, buf=0x%08lx\n", __func__, count, (long)buffer);
	#endif

	if (count >= 100)
		return -EFAULT;

	if (copy_from_user(cmd_buf, buffer, count)) {
		ret = -EFAULT;
	}

	if(count > 0) {
	   cmd_buf[count] = '\0';
	}

	if(strcmp(cmd_buf, "VPQ_self_test=data_path_status\n") == 0) {
		vpq_IP_Status_Check_handler();
	} else if (strcmp(cmd_buf, "VPQ_self_test=usb_check\n") == 0) {
		vpq_self_Check_handler();
	}

//AT V6.3 Patch 2022/08/04 - START
#ifdef CONFIG_RTK_SRE_AATS_UCT_AT
    if(strcmp(cmd_buf, "aats_uct_at_switch=on\n") == 0)
    {
        //dynamic switch on/off for AATS-UCT/AT Test
        g_AATS_UCT_AT_Switch= 1;
        printk(KERN_EMERG "####[David] AATS UCT/AT ON:%d\n", g_AATS_UCT_AT_Switch);
    } else if(strcmp(cmd_buf, "aats_uct_at_switch=off\n") == 0)
    {
        g_AATS_UCT_AT_Switch= 0;
        printk(KERN_EMERG "####[David] AATS UCT/AT OFF:%d\n", g_AATS_UCT_AT_Switch);
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
        printk(KERN_EMERG "PQ_linear mode flag on \n");
        printk(KERN_EMERG "####[David] PQ_linear mode -hope on- :%d  \n", PQ_UT_apply_Linear_Flag);
        //AT V6.2 Patch 2022/03/31 - END
        //Scaler_apply_PQ_Linear(1);
        //printk(KERN_EMERG "PQ_linear mode on \n");
        } else if (strcmp(cmd_buf, "linear_mode=off\n") == 0){
            //AT V6.2 Patch 2022/03/31 - START
            PQ_UT_apply_Linear_Flag=0;
            Scaler_apply_PQ_Linear(0);
            printk(KERN_EMERG "PQ linear mode flag off \n");
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS) && (IS_ENABLED(CONFIG_REALTEK_LOGBUF))
            if(str_status == 0){
                printk(KERN_EMERG "####[David] PQ_linear mode off, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n",jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
            }else{
                printk(KERN_EMERG "####[David] PQ_linear mode off, jiffies: %lu, rtc[ms]:%lu, str_status: %01d\n",jiffies, jiffies*1000/HZ,str_status);
            }
#else
            printk(KERN_EMERG "####[David] PQ_linear mode off, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n",jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
#endif
            //AT V6.2 Patch 2022/03/31 - END
            //Scaler_apply_PQ_Linear(0);
            //printk(KERN_EMERG "PQ linear mode off \n");
        }
        //AT V6.2 Patch 2022/03/31 - START
        if (strcmp(cmd_buf, "gd=on\n") == 0){
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS) && (IS_ENABLED(CONFIG_REALTEK_LOGBUF))
            if(str_status == 0){
                printk(KERN_EMERG "####[David] gd on, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n", jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
            }else{
                printk(KERN_EMERG "####[David] gd on, jiffies: %lu, rtc[ms]:%lu, str_status: %01d\n",jiffies, jiffies*1000/HZ,str_status);
            }
#else
            printk(KERN_EMERG "####[David] gd on, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n", jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
#endif
        } else if (strcmp(cmd_buf, "gd=off\n") == 0){
            g_ActiveAATS_UCT_GD_On = 0;
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS) && (IS_ENABLED(CONFIG_REALTEK_LOGBUF))
            if(str_status == 0){
                printk(KERN_EMERG "####[David] gd off, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n", jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
            }else{
                printk(KERN_EMERG "####[David] gd off, jiffies: %lu, rtc[ms]:%lu, str_status: %01d\n",jiffies, jiffies*1000/HZ,str_status);
            }
#else
            printk(KERN_EMERG "####[David] gd off, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n", jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
#endif
        } else if (strcmp(cmd_buf, "vr=on\n") == 0){
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS) && (IS_ENABLED(CONFIG_REALTEK_LOGBUF))
            if(str_status == 0){
                printk(KERN_EMERG "####[David] vr on, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n", jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
            }else{
                printk(KERN_EMERG "####[David] vr on, jiffies: %lu, rtc[ms]:%lu, str_status: %01d\n",jiffies, jiffies*1000/HZ,str_status);
            }
#else
            printk(KERN_EMERG "####[David] vr on, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n", jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
#endif
        } else if (strcmp(cmd_buf, "vr=off\n") == 0) {
#if defined(CONFIG_TRACEPOINTS) && defined(CONFIG_ANDROID_VENDOR_HOOKS) && (IS_ENABLED(CONFIG_REALTEK_LOGBUF))
            if(str_status == 0){
                printk(KERN_EMERG "####[David] vr off, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n", jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
            }else{
                printk(KERN_EMERG "####[David] vr off, jiffies: %lu, rtc[ms]:%lu, str_status: %01d\n",jiffies, jiffies*1000/HZ,str_status);
            }
#else
            printk(KERN_EMERG "####[David] vr off, jiffies: %lu, rtc[ms]:%lu, sys-time: %02d:%02d:%02d.%03d\n", jiffies, jiffies*1000/HZ,hour,minute,second,millisecond);
#endif
        }
        //AT V6.2 Patch 2022/03/31 - END
    }
#endif
//AT V6.3 Patch 2022/08/04 - END
#if 1
	//printk(KERN_EMERG "vpqdev cmd : %s \n", buffer);

	if(strcmp(cmd_buf, "testmode=1\n") == 0){
		Scaler_Set_VIP_Disable_PQ(VPQ_ModuleTestMode_PQByPass, 1);
		Scaler_Set_PQ_ModuleTest(VPQ_ModuleTestMode_PQByPass);
		printk(KERN_EMERG "PQ_bypass_mode1\n");
	}else if (strcmp(cmd_buf, "testmode=2\n") == 0){
		Scaler_Set_VIP_Disable_PQ(VPQ_ModuleTestMode_HDRPQByPass, 1);
		Scaler_Set_PQ_ModuleTest(VPQ_ModuleTestMode_HDRPQByPass);
		printk(KERN_EMERG "PQ_bypass_mode2\n");
	}else if (strcmp(cmd_buf, "testmode=0\n") == 0){
		Scaler_Set_PQ_ModuleTest(VPQ_ModuleTestMode_MAX);
		printk(KERN_EMERG "PQ_bypass_mode0\n");
	}


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
	return ret;
#endif
}

int vpq_release(struct inode *inode, struct file *filep)
{
	return 0;
}

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
	fwif_color_set_color_temp(TRUE, ct.Red_Gain<<2, ct.Green_Gain<<2, ct.Blue_Gain<<2, (ct.Red_Offset<<2)-2048, (ct.Green_Offset<<2)-2048, (ct.Blue_Offset<<2)-2048, waitSync);
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
		GOut_R[i] = g_GammaRemap[Power1div22Gamma[i]];
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
		GOut_G[i] = g_GammaRemap[Power1div22Gamma[i]];
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
		GOut_B[i] = g_GammaRemap[Power1div22Gamma[i]];
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
			GOut_R[i] = (gamma.Gamme_Red[i] << 2) + (gamma.Gamme_Red[i] >> 8);
			GOut_G[i] = (gamma.Gamme_Green[i] << 2) + (gamma.Gamme_Green[i] >> 8);
			GOut_B[i] = (gamma.Gamme_Blue[i] << 2) + (gamma.Gamme_Blue[i] >> 8);
		}

		GOut_R[1024] = GOut_R[1023];
		GOut_G[1024] = GOut_G[1023];
		GOut_B[1024] = GOut_B[1023];
	}

}

void vpq_set_gamma(unsigned char ucDoCtrlBack, unsigned char useRPC)
{
	extern struct semaphore Gamma_Semaphore;

	down(&Gamma_Semaphore);
	vpq_mapping_gamma_by_gamut_mode();
	fwif_color_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B, GOut_R, GOut_G, GOut_B);
	if (!useRPC || !fwif_color_set_ddomainISR_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B)) {
		fwif_color_gamma_control_front(SLR_MAIN_DISPLAY);
		fwif_color_set_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B);
		if (ucDoCtrlBack)
			fwif_color_gamma_control_back(SLR_MAIN_DISPLAY, 1);
	}/* else {
		ISR_GAMMA_WRITE_ST *p_stISRGamma = (ISR_GAMMA_WRITE_ST *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_GAMMA);
		extern unsigned int isr_write_gamma_SN;

		if (p_stISRGamma != NULL) {
			isr_write_gamma_SN++;
			fwif_color_ChangeUINT32Endian_Copy(&isr_write_gamma_SN, sizeof(unsigned int), &p_stISRGamma->write_SerialNumber_start, 1);
			fwif_color_ChangeUINT32Endian_Copy(final_GAMMA_R, sizeof(final_GAMMA_R)/sizeof(unsigned int), p_stISRGamma->encoded_GAMMA_R, 1);
			fwif_color_ChangeUINT32Endian_Copy(final_GAMMA_G, sizeof(final_GAMMA_G)/sizeof(unsigned int), p_stISRGamma->encoded_GAMMA_G, 1);
			fwif_color_ChangeUINT32Endian_Copy(final_GAMMA_B, sizeof(final_GAMMA_B)/sizeof(unsigned int), p_stISRGamma->encoded_GAMMA_B, 1);
			p_stISRGamma->write_SerialNumber_end = p_stISRGamma->write_SerialNumber_start;

			if ((Scaler_SendRPC(SCALERIOC_SET_GAMMA, 0, 0)) < 0) {
				rtd_pr_vpq_err(" update GAMMA to driver fail !!!\n");
			}
		}

	}*/
	up(&Gamma_Semaphore);

}

void vpq_set_inv_gamma(unsigned char ucDoCtrlBack)
{
	down(&VPQ_Semaphore);
	fwif_color_inv_gamma_control_front(SLR_MAIN_DISPLAY);
	fwif_color_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B, Power22InvGamma, Power22InvGamma, Power22InvGamma);
	fwif_color_set_inv_gamma_encode_tv006();
	if (ucDoCtrlBack)
		fwif_color_inv_gamma_control_back(SLR_MAIN_DISPLAY, 1);

	up(&VPQ_Semaphore);
}

unsigned int out_GAMMA_R[128], out_GAMMA_G[128], out_GAMMA_B[128];
void vpq_set_out_gamma(void)
{
#ifdef VPQ_COMPILER_ERROR_ENABLE

	extern unsigned short LEDOutGamma[257];
	static unsigned char panelTypeDef = 0;
	unsigned char panel_type=((IoReg_Read32(MISC_GPIO_GP2DATI_VADDR)>>18)&0x1);//0xb801bd38[18]=1 OLED
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
#endif

void vpq_update_icm(unsigned char waiting)
{
	down(&VPQ_Semaphore);
	fwif_color_set_ICM_table_driver_tv006(&icm_tab_elem_write, waiting);
	up(&VPQ_Semaphore);
}

void vpq_cm_global_init(void)
{
	int rgn;
	memcpy(&g_cm_rgn_exp, &g_cm_rgn_adv, sizeof(CHIP_CM_REGION_T));

	for (rgn = 0; rgn < CHIP_CM_COLOR_REGION_MAX; rgn++) {
		g_cm_ctrl_adv.stColor[rgn].masterGain = 0;
		g_cm_ctrl_adv.stColor[rgn].stColorGain.stColorGainHue = 128;
		g_cm_ctrl_adv.stColor[rgn].stColorGain.stColorGainSat = 128;
		g_cm_ctrl_adv.stColor[rgn].stColorGain.stColorGainInt = 128;
	}

	memcpy(&g_cm_ctrl_exp, &g_cm_ctrl_adv, sizeof(CHIP_COLOR_CONTROL_T));
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
	extern unsigned char lastLUT_tbl_index;// RTK last index

	if (NULL == pLut)
		return FALSE;

	down(&VPQ_Semaphore);
	lastLUT_tbl_index = 0xff; //clear rtk DB last index
	fwif_color_3d_lut_access_index(ACCESS_MODE_SET, 0); //clear rtk DB index

	if (bOnlySetEnable && iEnable) {
		fwif_color_WaitFor_DEN_STOP_UZUDTG();
		drvif_color_D_3dLUT_Enable(1);
	} else {
		for (idx = 0; idx < 4913; idx++) {
			pLUT3D[idx<<1] = (pLut->pt[idx].r_data<<16)+pLut->pt[idx].g_data;
			pLUT3D[(idx<<1)+1] = pLut->pt[idx].b_data;
		}

		fwif_color_WaitFor_DEN_STOP_UZUDTG();

		if (iEnable)
			drvif_color_D_3dLUT(1, pLUT3D);
		else
			drvif_color_D_3dLUT(0, pLUT3D);
	}
	up(&VPQ_Semaphore);
	return TRUE;
}

extern DRV_POD_DATA_table_t POD_DATA[3];
//unsigned char InputLUT[578] = {0};
void vpq_boot_init(void)
{
#ifndef BUILD_QUICK_SHOW

#ifdef CONFIG_CUSTOMER_TV030
	//unsigned char VALC_DeltaTbl[PCID_TABLE_LEN] = {0};
#endif
	DRV_Sharpness_Level Sharpness_Level;

if(is_QS_pq_enable()==1) { //current is QS flow	pq_quick_show flow
	fwif_color_DI_IP_ini();
	fwif_color_DI_IEGSM_ini();
	vpq_cm_global_init();
	vpq_init_gamma();
	vpq_set_inv_gamma(0);
	vpq_set_out_gamma();


	fwif_color_set_DM2_OE_EO_CSC3_Sram_init_Flag_TV006(1);
	drvif_DM2_B0203_noSram_Enable(1);

	/* set panel dither*/
	if (Get_DISPLAY_PANEL_MPLUS_RGBW() == TRUE || Get_DISPLAY_PANEL_BOW_RGBW() == TRUE)
		fwif_color_set_Panel_Dither(TRUE,VIP_PANEL_BIT8,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);
	else
		fwif_color_set_Panel_Dither(TRUE,VIP_PANEL_BIT10,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);

	/* ST2094 Ctrl ini*/
	fwif_color_ST2094_Ctrl_Structure_ini();

}else{
	fwif_color_DI_IP_ini();
	fwif_color_DI_IEGSM_ini();
	vpq_cm_global_init();
	vpq_init_gamma();
	vpq_set_inv_gamma(0);
	vpq_set_out_gamma();
	vpq_set_gamma(0, 0);
	fwif_color_inv_gamma_control_back(SLR_MAIN_DISPLAY, 0);
	fwif_color_gamma_control_back(SLR_MAIN_DISPLAY, 0);
	//20210114 roger, mark vivid color for boot logo video path issue(color wrong)
	//drvif_color_set_Vivid_Color_Enable(TRUE);
	//20190118 roger, mark ICM init for boot logo video path issue
	//fwif_color_set_ICM_table_driver_init_tv006();
	//fwif_color_set_ICM_table_driver_init_elem_tv006(&icm_tab_elem_write);
	//memcpy(&icm_tab_elem_of_vip_table, &icm_tab_elem_write, sizeof(COLORELEM_TAB_T));
	//vpq_update_icm(FALSE);
	fwif_color_SetDNR_tv006(2);
	memset(&Sharpness_Level, 0, sizeof(DRV_Sharpness_Level));
	drvif_color_set_Sharpness_level(&Sharpness_Level);
	fwif_color_3d_lut_init_tv006();
	fwif_color_set_DCC_Init(SLR_MAIN_DISPLAY);

#if 0//def CONFIG_HDR_SDR_SEAMLESS //mark due to seamless no need work 2017/11
	{
		static PQ_HDR_3dLUT_16_PARAM args;

		/* set defautl table and ini clk.*/
		rtd_pr_vpq_info("boot ini!!!\n");
		fwif_color_set_DM2_EOTF_TV006(EOTF_LUT_R);
		fwif_color_set_DM2_OETF_TV006(OETF_LUT_R);
		/* set default 3D LUT for seamless*/
		/*printk("[elieli]VIP_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En]=%d\n", VIP_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En]);*/
		//extern struct semaphore VPQ_DM_3DLUT_Semaphore;
		args.c3dLUT_16_enable = 1;
		args.a3dLUT_16_pArray = (UINT16*)&HDR10_3DLUT_24x24x24[0];
		fwif_color_set_DM_HDR_3dLUT_24x24x24_16_TV006(&args);
	}
#endif
	fwif_color_set_DM2_OE_EO_CSC3_Sram_init_Flag_TV006(1);
	drvif_DM2_B0203_noSram_Enable(1);
#ifdef CONFIG_SNR_CLOCK_HW_ISSUE
	/* always on for snr hw issue, elieli*/
	if (drvif_color_Get_DRV_SNR_Clock(0) == 1) {
		drvif_color_Set_DRV_SNR_Clock(0);
		drvif_color_DRV_SNR_Mosquito_NR_En(0xF);
		drvif_color_iEdgeSmooth_en(0xF);
		drvif_color_Set_DRV_SNR_Clock(1);
	} else {
		drvif_color_DRV_SNR_Mosquito_NR_En(0xF);
		drvif_color_iEdgeSmooth_en(0xF);
	}
#endif
	/* always on for UZU_Bypass_No_PwrSave hw issue, elieli*/
	//drvif_color_Access_UZU_Bypass_No_PwrSave(1, 1);

	/* set panel dither*/
	if (Get_DISPLAY_PANEL_MPLUS_RGBW() == TRUE || Get_DISPLAY_PANEL_BOW_RGBW() == TRUE)
		fwif_color_set_Panel_Dither(TRUE,VIP_PANEL_BIT8,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);
	else
		fwif_color_set_Panel_Dither(TRUE,VIP_PANEL_BIT10,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);
#if 1//OD load from bootcode //mac7p booycode OD not ready, enable for bringup test first
	if (Scaler_Init_OD()) {
		fwif_color_set_liner_od_table();
		Scaler_Set_OD_Bit_By_XML();
	}
#endif
	/* demura ini */
	fwif_color_DeMura_init();

	/* ST2094 Ctrl ini*/
	fwif_color_ST2094_Ctrl_Structure_ini();
}
#if defined(CONFIG_RTK_AI_DRV)
	/* NN init*/
	scalerAI_Init();
#endif

	// for low power mode, run time pm
	Set_Var_VPQ_rtpm_tsk_block(0);		// init status is on for socts HEVC test. so vsc need to set tsk block while vsc is suspend for VSC socts.
	Set_Var_VPQ_rtpm_tsk_block_VPQ_done(0);		// default tsk is on, so this flag need to be set by tsk
	Set_Var_VPQ_rtpm_tsk_VPQ_freeze_stauts(0);


#ifdef CONFIG_CUSTOMER_TV030

	if(Get_DISPLAY_PANEL_TYPE()==  P_MINI_LVDS){

		fwif_color_DeMura_init();
		fwif_color_set_DeMura_En(1);

		//Scaler_color_colorwrite_Output_InvOutput_gamma(2); //bootcode load bin
#if 0
		drvif_color_pcid2_pixel_setting(&(POD_DATA[1].pcid2_data));
		drvif_color_pcid2_poltable(&(POD_DATA[1].pPcid2PolTable[_PCID2_COLOR_R][0]),_PCID2_COLOR_R);
		drvif_color_pcid2_poltable(&(POD_DATA[1].pPcid2PolTable[_PCID2_COLOR_G][0]),_PCID2_COLOR_G);
		drvif_color_pcid2_poltable(&(POD_DATA[1].pPcid2PolTable[_PCID2_COLOR_B][0]),_PCID2_COLOR_B);
		drvif_color_pcid_VALC_protection(&(POD_DATA[1].pcid_valc));

		// R1
		memcpy(VALC_DeltaTbl, &(POD_DATA[1].POD_DATA[_PCID2_COLOR_R*2][0]), sizeof(char)*PCID_TABLE_LEN);
		//drvif_color_pcid_TblTrans_Target2Delta(VALC_DeltaTbl);
		fwif_color_set_pcid_single_valuetable(VALC_DeltaTbl, 0, _PCID2_COLOR_R);
		// R2
		memcpy(VALC_DeltaTbl, &(POD_DATA[1].POD_DATA[_PCID2_COLOR_R*2+1][0]), sizeof(char)*PCID_TABLE_LEN);
		//drvif_color_pcid_TblTrans_Target2Delta(VALC_DeltaTbl);
		fwif_color_set_pcid_single_valuetable(VALC_DeltaTbl, 1, _PCID2_COLOR_R);
		// G1
		memcpy(VALC_DeltaTbl, &(POD_DATA[1].POD_DATA[_PCID2_COLOR_G*2][0]), sizeof(char)*PCID_TABLE_LEN);
		//drvif_color_pcid_TblTrans_Target2Delta(VALC_DeltaTbl);
		fwif_color_set_pcid_single_valuetable(VALC_DeltaTbl, 0, _PCID2_COLOR_G);
		// G2
		memcpy(VALC_DeltaTbl, &(POD_DATA[1].POD_DATA[_PCID2_COLOR_G*2+1][0]), sizeof(char)*PCID_TABLE_LEN);
		//drvif_color_pcid_TblTrans_Target2Delta(VALC_DeltaTbl);
		fwif_color_set_pcid_single_valuetable(VALC_DeltaTbl, 1, _PCID2_COLOR_G);
		// B1
		memcpy(VALC_DeltaTbl, &(POD_DATA[1].POD_DATA[_PCID2_COLOR_B*2][0]), sizeof(char)*PCID_TABLE_LEN);
		//drvif_color_pcid_TblTrans_Target2Delta(VALC_DeltaTbl);
		fwif_color_set_pcid_single_valuetable(VALC_DeltaTbl, 0, _PCID2_COLOR_B);
		// B2
		memcpy(VALC_DeltaTbl, &(POD_DATA[1].POD_DATA[_PCID2_COLOR_B*2+1][0]), sizeof(char)*PCID_TABLE_LEN);
		//drvif_color_pcid_TblTrans_Target2Delta(VALC_DeltaTbl);
		fwif_color_set_pcid_single_valuetable(VALC_DeltaTbl, 1, _PCID2_COLOR_B);
#endif

	}

#endif
#endif //quick show
}

void *vpq_demo_overscan_func(void *param)
{
	KADP_VIDEO_WID_T wid;
	KADP_VIDEO_RECT_T inregion;
	KADP_VIDEO_RECT_T outregion;

	wid = KADP_VIDEO_WID_0;
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

	rtk_hal_vsc_SetInputRegion((KADP_VIDEO_WID_T)wid, inregion);
	rtk_hal_vsc_SetOutputRegion((VIDEO_WID_T)wid, outregion,0,0);

	return NULL;
}

void *vpq_demo_pq_func(void *param)
{
	unsigned char enable;

	if (param == NULL)
		return NULL;

	enable = *((unsigned char*)param);

	if (enable==1 && fwif_VIP_get_Project_ID() != VIP_Project_ID_TV001) {
		fwif_color_set_pq_demo_flag_rpc(1);
		fwif_VIP_set_Project_ID(VIP_Project_ID_TV001);
		fwif_color_handler();
		fwif_color_video_quality_handler();
		fwif_color_vpq_pic_init();
		Scaler_SetDNR(4);
		Scaler_SetMPEGNR(4, CALLED_NOT_BY_OSD);
		fwif_color_set_PQA_motion_threshold_TV006();
		fwif_color_SetAutoMAFlag(1);
		vpq_demo_overscan_func(NULL);
		Demo_Flag = 1;
	} else if (enable==0 && fwif_VIP_get_Project_ID() != VIP_Project_ID_TV006) {
		g_InvGammaPowerMode = 0;
		g_IsInvGammaPowerNewMode = 0;
		fwif_color_set_pq_demo_flag_rpc(0);
		fwif_VIP_set_Project_ID(VIP_Project_ID_TV006);
		vpq_boot_init();
		fwif_color_inv_gamma_control_back(SLR_MAIN_DISPLAY, 0);
		drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 0, 0);
		drvif_color_inv_gamma_enable(SLR_SUB_DISPLAY, 0, 0);
		fwif_color_gamma_control_back(SLR_MAIN_DISPLAY, 1);
		fwif_color_SetDNR_tv006(0);
		fwif_color_set_DCC_Init(SLR_MAIN_DISPLAY);
		fwif_color_handler();
		fwif_color_video_quality_handler();
		fwif_color_vpq_pic_init();
		fwif_color_SetAutoMAFlag(1);
		Demo_Flag = 0;
	} else if (enable==2 && fwif_VIP_get_Project_ID() != VIP_Project_ID_TV001) {			//rock_rau 20150813 PQ by pass
		DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
		fwif_color_set_pq_demo_flag_rpc(1);
		fwif_VIP_set_Project_ID(VIP_Project_ID_TV001);
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

#ifndef BUILD_QUICK_SHOW


#ifndef UT_flag

#ifdef CONFIG_PM

OD_SUSPEND_RESUME_T od_suspend_resume;
void od_do_suspend(void)
{
	//extern unsigned char bODInited;
	//int i = 0;
	od_od_ctrl_RBUS od_ctrl_reg;
	//if (!bODInited)
		//return;
	od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
	od_suspend_resume.od_enable = od_ctrl_reg.od_en;
	drvif_color_od_table_seperate_read(&(od_table_store_r[0]), 0);//R
	drvif_color_od_table_seperate_read(&(od_table_store_g[0]), 1);//G
	drvif_color_od_table_seperate_read(&(od_table_store_b[0]), 2);//B
	//drvif_color_od_table_seperate_read(&(od_table_store_w[0]), 4);//W
}
DRV_pcid_valc_t Setting;
unsigned char pcid_en = 0;
unsigned char pcid2_en = 0;
DRV_pcid2_data_t Pcid2Setting;

void pcid_do_suspend(void)
{
	char InputLUT[289] = {0};
	int i=0;
	for(i=0; i<81; i++)
	{
		PCID_ValueTBL[i] = 0;
		POD_ValueTBL_1[i] = 0;
		POD_ValueTBL_2[i] = 0;
	}
	drvif_color_get_pcid_enable(&pcid_en);
	drvif_color_get_pcid2_enable(&pcid2_en);
	drvif_color_get_pcid2_pixel_setting(&Pcid2Setting);
	//PCID value table
	//R
	fwif_color_get_pcid_valuetable(InputLUT, 0,_PCID2_COLOR_R);
	memcpy(&((POD_DATA[1]).POD_DATA[_PCID2_COLOR_R*2][0]), &(InputLUT[0]), sizeof(char)*289);
	fwif_color_get_pcid_valuetable(InputLUT, 1,_PCID2_COLOR_R);
	memcpy(&((POD_DATA[1]).POD_DATA[_PCID2_COLOR_R*2+1][0]), &(InputLUT[0]), sizeof(char)*289);
	//G
	fwif_color_get_pcid_valuetable(InputLUT, 0,_PCID2_COLOR_G);
	memcpy(&((POD_DATA[1]).POD_DATA[_PCID2_COLOR_G*2][0]), &(InputLUT[0]), sizeof(char)*289);
	fwif_color_get_pcid_valuetable(InputLUT, 1,_PCID2_COLOR_G);
	memcpy(&((POD_DATA[1]).POD_DATA[_PCID2_COLOR_G*2+1][0]), &(InputLUT[0]),  sizeof(char)*289);
	//B
	fwif_color_get_pcid_valuetable(InputLUT, 0,_PCID2_COLOR_B);
	memcpy(&((POD_DATA[1]).POD_DATA[_PCID2_COLOR_B*2][0]), &(InputLUT[0]), sizeof(char)*289);
	fwif_color_get_pcid_valuetable(InputLUT, 1,_PCID2_COLOR_B);
	memcpy(&((POD_DATA[1]).POD_DATA[_PCID2_COLOR_B*2+1][0]), &(InputLUT[0]), sizeof(char)*289);

	drvif_color_get_pcid_VALC_protection((DRV_pcid_valc_t*)&(POD_DATA[1].pcid_valc.valc_en));
	fwif_color_get_PCID_RgnWeightSetting(&Setting);
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
	fwif_color_read_output_gamma(Output_InvOutput_Gamma_TBL_Reserved04);
	od_do_suspend();
	pcid_do_suspend();
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
	extern unsigned char bODInited;
	//extern unsigned char bODTableLoaded;
	//extern char od_table_mode_store;// 0:target mode 1:delta mode -1:inverse mode

	bODInited = 0;
	if (Scaler_Init_OD()) {
#if 0
		if (bODTableLoaded && od_table_mode_store == 1)
			fwif_color_od_table_restore();
		else
			fwif_color_set_liner_od_table();
#endif
		fwif_color_od_table_17x17_transform(0, od_table_store_r, od_delta_table_resume);
		drvif_color_od_table_seperate_write(&(od_delta_table_resume[0]), 0, 0);// 0:delta mode 1:target mode(output mode) R
		fwif_color_od_table_17x17_transform(0, od_table_store_g, od_delta_table_resume);
		drvif_color_od_table_seperate_write(&(od_delta_table_resume[0]), 0, 1);// 0:delta mode 1:target mode(output mode) G
		fwif_color_od_table_17x17_transform(0, od_table_store_b, od_delta_table_resume);
		drvif_color_od_table_seperate_write(&(od_delta_table_resume[0]), 0, 2);// 0:delta mode 1:target mode(output mode) B

		if (fwif_VIP_get_Project_ID() != VIP_Project_ID_TV006) {
			if (od_suspend_resume.od_enable)
				fwif_color_set_od(TRUE);
			else
				fwif_color_set_od(FALSE);
			//Scaler_Set_OD_Bit_By_XML();
		} else
			fwif_color_set_od(1);
	}
	//drvif_color_od_table_seperate_write(&(od_table_store_w[0]), 1, 4);// 0:delta mode 1:target mode(output mode) W
	//drvif_color_od(od_suspend_resume.od_enable);
	//od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
	//od_ctrl_reg.od_en = od_suspend_resume.od_enable;
	//IoReg_Write32(OD_OD_CTRL_reg, (od_ctrl_reg.regValue));
}

void pcid_do_resume()
{
	fwif_color_set_POD_DATA_table(&(POD_DATA[1]));
	//
	drvif_color_pcid_enable(pcid_en); // PCID and PCID2 cannot enable at the same time
	drvif_color_pcid2_enable(pcid2_en);
	//fwif_color_set_pcid2_valuetable(pLUT);
	//fwif_color_set_pcid2_pixel_setting();
	fwif_color_set_PCID_RgnWeightSetting(&Setting);
}

void vpq_do_resume(void)
{
	extern unsigned char lastLUT_tbl_index;
	extern unsigned char g_bNotFirstRun_LD_Data_Compensation_NewMode_2DTable;
	extern unsigned char output_gamma_DataStored;
	DRV_Sharpness_Level Sharpness_Level;
  	//_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 0;

	g_cm_need_refresh = 1;
	g_srgbForceUpdate = 1;
	g_InvGammaPowerMode = 0;
	g_IsInvGammaPowerNewMode = 0;
	g_HDR3DLUTForceWrite = 1;
	lastLUT_tbl_index = 0xff;
	g_3DLUT_Resume = 1;
	g_bNotFirstRun_LD_Data_Compensation_NewMode_2DTable = 0;
	g_pcid_en = 0;
	g_pcid2_en = 0;
	output_gamma_DataStored = 0;
	str_resume_do_picturemode=1; //TV030

	pcid_do_resume();
	fwif_color_DI_IP_ini();
	fwif_color_DI_IEGSM_ini();
	Scaler_color_set_m_nowSource(255);
	vpq_set_out_gamma();
	vpq_init_gamma();
	vpq_set_inv_gamma(0);
	fwif_color_inv_gamma_control_back(SLR_MAIN_DISPLAY, 0);
	vpq_set_gamma(1, 0);
	fwif_color_gamma_control_back(SLR_SUB_DISPLAY, 1);
	fwif_color_colorwrite_Output_gamma(Output_InvOutput_Gamma_TBL_Reserved04);
	drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 0, 0);
	drvif_color_inv_gamma_enable(SLR_SUB_DISPLAY, 0, 0);
	drvif_color_set_Vivid_Color_Enable(TRUE);
	fwif_color_set_ICM_table_driver_init_tv006();
	vpq_update_icm(FALSE);
	memset(&Sharpness_Level, 0, sizeof(DRV_Sharpness_Level));
	drvif_color_set_Sharpness_level(&Sharpness_Level);
	fwif_color_set_DCC_Init(SLR_MAIN_DISPLAY);
	vpq_set_3d_lut_index(0, 0);
	od_do_resume();
	drvif_Set_DM_HDR_CLK();	// enable DM & Composer clock
	hdr_resume();

#ifdef CONFIG_SNR_CLOCK_HW_ISSUE
	/* always on for snr hw issue, elieli*/
	if (drvif_color_Get_DRV_SNR_Clock(0) == 1) {
		drvif_color_Set_DRV_SNR_Clock(0);
		drvif_color_DRV_SNR_Mosquito_NR_En(0xFF);
		drvif_color_iEdgeSmooth_en(0xFF);
		drvif_color_Set_DRV_SNR_Clock(1);
	} else {
		drvif_color_DRV_SNR_Mosquito_NR_En(0xFF);
		drvif_color_iEdgeSmooth_en(0xFF);
	}
#endif //CONFIG_SNR_CLOCK_HW_ISSUE

	/* always on for UZU_Bypass_No_PwrSave hw issue, elieli*/
	//drvif_color_Access_UZU_Bypass_No_PwrSave(1, 1);

	/* set panel dither*/
	if (Get_DISPLAY_PANEL_MPLUS_RGBW() == TRUE || Get_DISPLAY_PANEL_BOW_RGBW() == TRUE)
		fwif_color_set_Panel_Dither(TRUE,VIP_PANEL_BIT8,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);
	else
		fwif_color_set_Panel_Dither(TRUE,VIP_PANEL_BIT10,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);

	/* demura ini */
	fwif_color_DeMura_init();

	/* ST2094 Ctrl ini*/
	fwif_color_ST2094_Ctrl_Structure_ini();

#if defined(CONFIG_RTK_AI_DRV)
	/* NN init*/
	scalerAI_Init();
#endif

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
#endif //ENABLE_VIP_TABLE_CHECKSUM
	//return 0;
}

static int vpq_suspend(struct device *p_dev)
{
	pm_runtime_disable(&vpq_platform_devs->dev);
	rtd_pr_vpq_info("VPQ_RTPM, vpq_suspend\n");
	VPQ_PM_LOG("info", "stop", "VPQ_RTPM STR suspend");
	return 0;
}

static int vpq_resume(struct device *p_dev)
{
	pm_runtime_enable(&vpq_platform_devs->dev);
	rtd_pr_vpq_info("VPQ_RTPM, vpq_resume\n");

	VPQ_PM_LOG("info", "srtart", "VPQ_RTPM STR resume");
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
	{SCALERIOC_VIP_SMARTPIC_CLUS, sizeof(_clues), NULL},
	{SCALERIOC_VIP_RPC_SMARTPIC_CLUS, sizeof(_RPC_clues), NULL},
	{SCALERIOC_VIP_TABLE_STRUCT, sizeof(SLR_VIP_TABLE), NULL},
	{SCALERIOC_VIP_RPC_TABLE_STRUCT, sizeof(RPC_SLR_VIP_TABLE), NULL},
	{SCALERIOC_VIP_TABLE_CUSTOM_STRUCT, sizeof(SLR_VIP_TABLE_CUSTOM_TV001), NULL},
	{SCALERIOC_SET_GAMMA, sizeof(ISR_GAMMA_WRITE_ST), NULL},
};

static int vpq_suspend_std(struct device *p_dev)
{
	int i;
	unsigned long smAddr = 0;
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
	unsigned long smAddr = 0;
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
#endif //CONFIG_HIBERNATION

#endif //CONFIG_PM

#if defined(CONFIG_RTK_AI_DRV)
extern void h3ddma_set_film_buffer_addr(void);//
extern void scalerAI_SE_draw_Proc(void);
extern void drvif_i3ddma_buf_SE_capture(void);
extern unsigned char get_vt_src_cap_status(void);
//void debug_i3ddma_cap_enable(unsigned char value);// mac7p vt module no use
extern unsigned int vgip_isr_cnt;
static bool se_tsk_running_flag = FALSE;
static struct task_struct *p_se_tsk = NULL;
int sem_se_ai_flag=0;
struct semaphore sem_se_ai;
extern DRV_AI_Ctrl_table ai_ctrl;
unsigned char PQ_set_done = 0;
static int se_tsk(void *p)//This task run se proc
{
	static unsigned int vgip_isr_cnt_pre = 0;
	//h3ddma_cap1_cap_status_RBUS h3ddma_cap1_cap_status_reg;
	od_od_ctrl_RBUS od_ctrl_reg;


	rtd_pr_vpq_debug("se_tsk()\n");

	current->flags &= ~PF_NOFREEZE;

	while (1)
	{

		static unsigned char source_pre = 255;
		unsigned char source_cur = 255;
		int nn_en=0, vt_en=0, memc_en=0;
		int signal_cnt_th = ai_ctrl.ai_global3.signal_cnt_th;//5


 #if 1 // vdec
   	_RPC_system_setting_info* RPC_system_info_structure_table = NULL;
    	unsigned char SE_rdPtr = 0;
    	unsigned char SE_wrPtr = 0;
    	unsigned char SE_status = 0;
    	RPC_system_info_structure_table = scaler_GetShare_Memory_RPC_system_setting_info_Struct();
    	if((RPC_system_info_structure_table==NULL))
    	{
    		//ROSPrintf(" Mem4 = %p\n",RPC_system_info_structure_table);
    		return -1;
    	}
#endif
		od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
		if(od_ctrl_reg.dummy1802ca00_31_7>>24 & 1) nn_en = 1; // bit 31
		if(od_ctrl_reg.dummy1802ca00_31_7>>23 & 1) vt_en = 1; // bit 30
		if(od_ctrl_reg.dummy1802ca00_31_7>>22 & 1) memc_en = 1; // bit 29
		//debug_i3ddma_cap_enable(vt_en);// mac7p vt module no use
		// to prevent STR/Snapshot fail
		if (freezing(current))
		{
			try_to_freeze();
		}

		if (kthread_should_stop()){
			rtd_pr_vpq_debug("se_tsk stop!!\n");
			break;
		}
#if 1 // vdec
	 	preempt_disable();
		_rtd_hwsem_lock(SB2_HD_SEM_NEW_3_reg, SEMA_HW_SEM_3_SCPU_2);
		SE_rdPtr = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.rdPtr;
		SE_wrPtr = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.wrPtr;
		SE_status = RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.pic[SE_rdPtr].status;
		_rtd_hwsem_unlock(SB2_HD_SEM_NEW_3_reg, SEMA_HW_SEM_3_SCPU_2);
		preempt_enable();

		if(down_timeout(&sem_se_ai, msecs_to_jiffies(2*1000) /*2sec*/) && SE_status != 1){ // sleep when vdec buffer empty
			//rtd_pr_vpq_emerg("lsy check timeout %d, status %d\n",down_timeout(&sem_se_ai, msecs_to_jiffies(2*1000)), SE_status);
			//hw_msleep(4);//need to switch
			//pr_debug("SLEP\n");
			continue;
		}
		else if(SE_status == 1)
		{
			//rtd_pr_vpq_emerg("lsy check start\n");
			ScalerVIP_SE_Proc();
		}

#else
		if(down_timeout(&sem_se_ai, msecs_to_jiffies(2*1000) /*2sec*/)) {
		//if(down_timeout(&sem_se_ai, (2*1000) /*2sec*/)) {
			continue;
		}
#endif


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



		if((vgip_isr_cnt != vgip_isr_cnt_pre))
		{
			_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

			vgip_isr_cnt_pre = vgip_isr_cnt;

			/* check source chaging or not */
			source_cur = VIP_RPC_system_info_structure_table->VIP_source;
			//source_cur = fwif_vip_source_check(3, NOT_BY_DISPLAY);
			if(source_cur != source_pre)
				fw_scalerip_reset_NN();

#ifdef CONFIG_RTK_8KCODEC_INTERFACE
#if defined(CONFIG_RTK_AI_DRV)
			VPQEX_rlink_AI_SeneInfo();
#endif
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

			if(nn_en &&
				((fwif_color_get_force_run_i3ddma_enable(SLR_MAIN_DISPLAY))||(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG)))
			{

				if(vpq_stereo_face != AI_PQ_AP_OFF)
				{
					scalerAI_preprocessing();
				}
			}
			if(vt_en)
			{
				#if 0 // mac7p vt module no use
				if(get_vt_src_cap_status() == VT_SOURCE_CAP_I3DDMA_BUF) /* vt: i3ddma buf se capture */
				{
					drvif_i3ddma_buf_SE_capture();
				}
				#endif
			}

			/* update pre source */
			source_pre = source_cur;
		}

		if(PQ_set_done)
		{
			scalerAI_SE_draw_Proc();
			PQ_set_done = 0;
		}


	}

    rtd_pr_vpq_debug("\r\n####se_tsk: exit...####\n");
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
	if (se_tsk_running_flag) {
 		ret = kthread_stop(p_se_tsk);
 		if (!ret) {
 			p_se_tsk = NULL;
 			se_tsk_running_flag = FALSE;
			rtd_pr_vpq_info("se_tsk thread stopped\n");
 		}
	}
}
#endif

static int VPQ_detect_tsk(void *p)
{
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
			Set_Var_VPQ_rtpm_tsk_VPQ_freeze_stauts(1);
			try_to_freeze();
		}
		Set_Var_VPQ_rtpm_tsk_VPQ_freeze_stauts(0);
		if (kthread_should_stop()) {
			break;
		}

		if (Get_Var_VPQ_rtpm_tsk_block() == 1) {
			//msleep(1);

			Set_Var_VPQ_rtpm_tsk_block_VPQ_done(1);

			//rtd_pr_vpq_info("VPQ_detect_tsk block ,vpq_rtpm_tsk_block_VPQ_done\n");

			continue;
		} else {
			Set_Var_VPQ_rtpm_tsk_block_VPQ_done(0);
		}

		
		//fwif_color_DI_RTNR_FRZ_ReCheck_TSK();
#ifdef HDR_calman_check
		Scaler_color_Get_HDR10_EOTF_OETF_ByMaxCLL_ByPanelLuminance_for_task();
#endif
		Scaler_color_set_HDR_AutoRun_TBL();
		//rtd_pr_vpq_info("VPQ_detect_tsk\n");

    }

    rtd_pr_vpq_debug("\r\n####VPQ_detect_tsk: exit...####\n");
    do_exit(0);
    return 0;
}
static void delete_VPQ_tsk(void)
{
	int ret;

	if (Get_Var_VPQ_fw_tsk_running_flag()) {
 		ret = kthread_stop(p_VPQ_task);
 		if (!ret) {
 			p_VPQ_task = NULL;
			Set_Var_VPQ_fw_tsk_running_flag(FALSE);
			rtd_pr_vpq_info("VPQ_tsk thread stopped\n");
 		}
	}
}

static void create_VPQ_tsk(void)
{
	int err;

	if (Get_Var_VPQ_fw_tsk_running_flag() == FALSE) {
		p_VPQ_task = kthread_create(VPQ_detect_tsk, NULL, "VPQ_detect_tsk");

	    if (p_VPQ_task) {
			wake_up_process(p_VPQ_task);
			Set_Var_VPQ_fw_tsk_running_flag(TRUE);
	    } else {
	    	err = PTR_ERR(p_VPQ_task);
	    	rtd_pr_vpq_emerg("Unable to start VPQ_tsk (err_id = %d)./n", err);
	    }
	}

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


int vpq_rumtime_suspend(struct device *p_dev)
{
	unsigned int cur_time1 = 0;
	unsigned int cost_vpq_time = 0;

	rtd_pr_vpq_info("VPQ_RTPM, vpq_rumtime_suspend sta\n");
	Set_Var_VPQ_rtpm_tsk_block(1);

	cur_time1 = drvif_color_get_cur_counter();
	while(!vpq_get_VPQ_TSK_Stop())
	{//wait vpq freeze
		udelay(50);
		if(drvif_color_wait_timeout_check_by_counter(cur_time1, 1000))
		{
		    printk(KERN_ERR "### VPQ_RTPM [err] wait VPQ related task pending fail ###\r\n");
		    return -1;
		}
	}

	cost_vpq_time = drvif_color_report_cost_time_by_counter(cur_time1);//calculate vpq time
	rtd_pr_vpq_info("VPQ_RTPM, TSK_Stop ready , vpq cost time =%d \n",cost_vpq_time);

	vpq_low_power_mode_suspend();
#ifndef UT_flag
	vsc_runtime_pm_put(); //wait_scaler_patch
#endif
	VPQ_PM_LOG("info", "suspend", "VPQ_RTPM runtime PM suspen end");

	return 0;
}

int vpq_rumtime_resume(struct device *p_dev)
{

	extern unsigned char lastLUT_tbl_index;
	extern unsigned char g_bNotFirstRun_LD_Data_Compensation_NewMode_2DTable;
	extern unsigned char output_gamma_DataStored;
	DRV_Sharpness_Level Sharpness_Level;
	//_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);


#ifndef UT_flag
	VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] = 0;


	vsc_runtime_pm_get();//wait_scaler_patch

	// for RPM power
	vpq_low_power_mode_resume();


	g_cm_need_refresh = 1;
	g_srgbForceUpdate = 1;
	g_InvGammaPowerMode = 0;
	g_IsInvGammaPowerNewMode = 0;
	g_HDR3DLUTForceWrite = 1;
	lastLUT_tbl_index = 0xff;
	g_3DLUT_Resume = 1;
	g_bNotFirstRun_LD_Data_Compensation_NewMode_2DTable = 0;
	g_pcid_en = 0;
	g_pcid2_en = 0;
	output_gamma_DataStored = 0;
	str_resume_do_picturemode=1; //TV030
	pcid_do_resume();
	fwif_color_DI_IP_ini();
	fwif_color_DI_IEGSM_ini();
	Scaler_color_set_m_nowSource(255);
	vpq_set_out_gamma();
	vpq_init_gamma();
	vpq_set_inv_gamma(0);
	fwif_color_inv_gamma_control_back(SLR_MAIN_DISPLAY, 0);
	vpq_set_gamma(1, 0);
	fwif_color_gamma_control_back(SLR_SUB_DISPLAY, 1);
	fwif_color_colorwrite_Output_gamma(Output_InvOutput_Gamma_TBL_Reserved04);
	drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 0, 0);
	drvif_color_inv_gamma_enable(SLR_SUB_DISPLAY, 0, 0);
	drvif_color_set_Vivid_Color_Enable(TRUE);
	fwif_color_set_ICM_table_driver_init_tv006();
	vpq_update_icm(FALSE);
	memset(&Sharpness_Level, 0, sizeof(DRV_Sharpness_Level));
	drvif_color_set_Sharpness_level(&Sharpness_Level);
	fwif_color_set_DCC_Init(SLR_MAIN_DISPLAY);
	vpq_set_3d_lut_index(0, 0);
	od_do_resume();
	drvif_Set_DM_HDR_CLK();	// enable DM & Composer clock
	hdr_resume();

#endif

#ifdef CONFIG_SNR_CLOCK_HW_ISSUE
	/* always on for snr hw issue, elieli*/
	if (drvif_color_Get_DRV_SNR_Clock(0) == 1) {
		drvif_color_Set_DRV_SNR_Clock(0);
		drvif_color_DRV_SNR_Mosquito_NR_En(0xFF);
		drvif_color_iEdgeSmooth_en(0xFF);
		drvif_color_Set_DRV_SNR_Clock(1);
	} else {
		drvif_color_DRV_SNR_Mosquito_NR_En(0xFF);
		drvif_color_iEdgeSmooth_en(0xFF);
	}
#endif //CONFIG_SNR_CLOCK_HW_ISSUE

	/* always on for UZU_Bypass_No_PwrSave hw issue, elieli*/
	//drvif_color_Access_UZU_Bypass_No_PwrSave(1, 1);

	/* set panel dither*/
	if (Get_DISPLAY_PANEL_MPLUS_RGBW() == TRUE || Get_DISPLAY_PANEL_BOW_RGBW() == TRUE)
		fwif_color_set_Panel_Dither(TRUE,VIP_PANEL_BIT8,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);
	else
		fwif_color_set_Panel_Dither(TRUE,VIP_PANEL_BIT10,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);

	/* demura ini */
	fwif_color_DeMura_init();

	/* ST2094 Ctrl ini*/
	fwif_color_ST2094_Ctrl_Structure_ini();

#if defined(CONFIG_RTK_AI_DRV)
	/* NN init*/
	scalerAI_Init();
#endif

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
#endif //ENABLE_VIP_TABLE_CHECKSUM

	Set_Var_VPQ_rtpm_tsk_block(0);

	rtd_pr_vpq_info("VPQ_RTPM, vpq_rumtime_resume end\n");
	VPQ_PM_LOG("info", "resume", "VPQ_RTPM runtime PM resume end");

	return 0;
}
char vpq_pm_runtime_get(unsigned char isSyncFlag)
{

	if (isSyncFlag == 1) {
		pm_runtime_get_sync(&vpq_platform_devs->dev);
		VPQ_PM_LOG("info", "open", "VPQ_RTPM dev open, sync=1");
	} else {
		pm_runtime_get(&vpq_platform_devs->dev);
		VPQ_PM_LOG("info", "open", "VPQ_RTPM dev open, sync=0");
	}
	//rtd_pr_vpq_info("VPQ_RTPM, get, sync Flag=%d,\n", isSyncFlag);
	
	return 0;
}

char vpq_pm_runtime_put(unsigned char isSyncFlag)
{
	if (isSyncFlag == 1) {
		pm_runtime_put_sync(&vpq_platform_devs->dev);
		VPQ_PM_LOG("info", "close", "VPQ_RTPM dev close, sync=1");
	} else {
		pm_runtime_put(&vpq_platform_devs->dev);
		VPQ_PM_LOG("info", "close", "VPQ_RTPM dev close, sync=0");
	}
	//rtd_pr_vpq_info("VPQ_RTPM, put, sync Flag=%d,\n", isSyncFlag);
	return 0;
}
#endif


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

	sys_reg_sys_srst3_reg.rstn_disp_lg_mplus = 1;
	sys_reg_sys_srst3_reg.rstn_disp_lg_pod = 1;
	sys_reg_sys_srst3_reg.rstn_disp_lg_hcic_nouse = 1;
	sys_reg_sys_srst3_reg.write_data = 0;

	sys_reg_sys_clken3_reg.clken_disp_lg_mplus = 1;
	sys_reg_sys_clken3_reg.clken_disp_lg_pod = 1;
	sys_reg_sys_clken3_reg.clken_disp_lg_hcic_nouse = 1;
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

	sys_reg_sys_srst3_reg.rstn_disp_lg_mplus = vpq_rtpm_sys_reg_sys_srst3_reg.rstn_disp_lg_mplus;
	sys_reg_sys_srst3_reg.rstn_disp_lg_pod = vpq_rtpm_sys_reg_sys_srst3_reg.rstn_disp_lg_pod;
	sys_reg_sys_srst3_reg.rstn_disp_lg_hcic_nouse = vpq_rtpm_sys_reg_sys_srst3_reg.rstn_disp_lg_hcic_nouse;
	sys_reg_sys_srst3_reg.write_data = 1;

	sys_reg_sys_clken3_reg.clken_disp_lg_mplus = vpq_rtpm_sys_reg_sys_clken3_reg.clken_disp_lg_mplus;
	sys_reg_sys_clken3_reg.clken_disp_lg_pod = vpq_rtpm_sys_reg_sys_clken3_reg.clken_disp_lg_pod;
	sys_reg_sys_clken3_reg.clken_disp_lg_hcic_nouse = vpq_rtpm_sys_reg_sys_clken3_reg.clken_disp_lg_hcic_nouse;
	sys_reg_sys_clken3_reg.write_data = 1;

	//IoReg_Write32(SYS_REG_SYS_SRST3_reg, sys_reg_sys_srst3_reg.regValue);	// no rest to keep register setting
	IoReg_Write32(SYS_REG_SYS_CLKEN3_reg, sys_reg_sys_clken3_reg.regValue);

	rtd_pr_vpq_info("VPQ_RTPM, vpq_rumtime_resume, restore, srst3=%x, clken3= %x,\n", 
		vpq_rtpm_sys_reg_sys_srst3_reg.regValue, vpq_rtpm_sys_reg_sys_clken3_reg.regValue);

}


unsigned char vpq_get_VPQ_TSK_Stop(void)
{
	unsigned char ret;
	unsigned char freeze_VPQ_done=0;

	if (Get_Var_VPQ_fw_tsk_running_flag() == 1)
		freeze_VPQ_done = Get_Var_VPQ_rtpm_tsk_block_VPQ_done();
	else
		freeze_VPQ_done = 1;

	if (Get_Var_VPQ_rtpm_tsk_VPQ_freeze_stauts() == 1){
		ret = 1;
	}
	else if ((freeze_VPQ_done == 1) && (Get_Var_VPQ_rtpm_tsk_block() == 1) ){
		ret = 1;
	}
	else{
		ret = 0;
	}
	return ret;
}

unsigned char vpq_set_VPQ_TSK_Stop(unsigned char stop_en)
{
	if (stop_en == 1)
		Set_Var_VPQ_rtpm_tsk_block(1);
	else 
		Set_Var_VPQ_rtpm_tsk_block(0);

	rtd_pr_vpq_info("VPQ_RTPM, set tsk stop = %d,\n", stop_en);
	return 0;
}



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

unsigned char vpq_get_handler_bypass(void)
{
#if 0//def CONFIG_SCALER_BRING_UP
	return 1;
#endif
	return ioctl_cmd_stop[VPQ_IOC_PQ_CMD_OPEN];
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
	Demura_TBL->DeMura_CTRL_TBL.demura_table_scale = scalingDown_bit;

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
	char *INNX_Demura_DeLut4;
	char *INNX_Demura_DeLut5;

	if ((Demura_TBL == NULL) || (pDeLut == NULL)) {
		rtd_pr_vpq_emerg("vpq_INNX_Demura_data_to_dma table NULL, Demura_TBL=%p, pDeLut=%p ~~\n", Demura_TBL, pDeLut);
		return;
	}

	INNX_Demura_DeLut1 = &pDeLut[INNX_Demura_DecodeLUT_Low_sta_addr];
	INNX_Demura_DeLut2 = &pDeLut[INNX_Demura_DecodeLUT_Mid_sta_addr];
	INNX_Demura_DeLut3 = &pDeLut[INNX_Demura_DecodeLUT_High_sta_addr];
	INNX_Demura_DeLut4 = &pDeLut[INNX_Demura_DecodeLUT_4_sta_addr];
	INNX_Demura_DeLut5 = &pDeLut[INNX_Demura_DecodeLUT_5_sta_addr];

	DMA_8bit = &Demura_TBL->TBL[0];
	Demura_TBL->DeMura_CTRL_TBL.demura_table_scale = 0;

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



COLORTEMP_ELEM_T ct;
DRV_film_mode film_mode;
DRV_Sharpness_Level Sharpness_Level;
CHIP_NOISE_REDUCTION_T NR_Level;
CHIP_MPEG_NOISE_REDUCTION_T MPEGNR_Level;
CHIP_DCC_T tFreshContrast_coef;
FreshContrastLUT_T tFreshContrastLUT;
VPQ_SetPicCtrl_T pic_ctrl = {0, {100, 50, 50, 0}, {128, 128, 128, 128}};
COLOR_GAMUT_T gamutData;
VPQ_SAT_LUT_T satLutData;
HDR_Set3DLUT_16_T phdr3dlut16;
UINT32 Eotf_table32[EOTF_size] = {0};
UINT16 Oetf_table16[OETF_size] = {0};
UINT32 PQModeInfo_flag[5] = {0};
unsigned char g_LGE_HDR_CSC_CTRL = 2; //BT2020
CHIP_CM_REGION_EXT_T cmRegionExt;//for CM ioctl tmp
UINT8 game_process = 0;

extern struct semaphore* get_gamemode_check_semaphore(void);

int PictureMode_flg = 0;  // for MEMC wrt by JerryWang 20161125

#ifndef BUILD_QUICK_SHOW


static long VPQ_IOC_INIT_impl(unsigned long arg)
{



if(is_QS_pq_enable()==1) { //current is QS flow	

	fwif_color_set_FILM_FW_ShareMemory();
	pr_emerg("[vpq][%s][%d]is_QS_pq_enable =1 \n", __FUNCTION__, __LINE__);

	PQ_Dev_Status = PQ_DEV_INIT_DONE;

}else{
	fwif_color_inv_gamma_control_back(SLR_MAIN_DISPLAY, 0);
	fwif_color_gamma_control_back(SLR_MAIN_DISPLAY, 1);
	fwif_color_gamma_control_back(SLR_SUB_DISPLAY, 1);
#ifdef CONFIG_DUAL_CHANNEL
	fwif_color_inv_gamma_control_back(SLR_SUB_DISPLAY, 0);
	fwif_color_gamma_control_back(SLR_SUB_DISPLAY, 1);
#endif
	//fwif_color_set_DCC_Init(SLR_MAIN_DISPLAY);
	//fwif_color_set_DCC_Init_tv006();

	// enable DM & Composer clock
	drvif_Set_DM_HDR_CLK();
	drvif_TV006_HDR10_init();

	fwif_color_set_FILM_FW_ShareMemory();

	fwif_color_set_Picture_Control_tv006(pic_ctrl.wId, &pic_ctrl);
	PQ_Dev_Status = PQ_DEV_INIT_DONE;
}	
	return 0;
}

static long VPQ_IOC_UNINIT_impl(unsigned long arg)
{
	PQ_Dev_Status = PQ_DEV_UNINIT;
	return 0;
}

static long VPQ_IOC_OPEN_impl(unsigned long arg)
{
	return 0;
}

static long VPQ_IOC_CLOSE_impl(unsigned long arg)
{
	return 0;
}

static long VPQ_IOC_SET_PROJECT_ID_impl(unsigned long arg)
{
	int ret = 0;
	VIP_Customer_Project_ID_ENUM prj_id;
	if (copy_from_user(&prj_id, (int __user *)arg, sizeof(unsigned int))) {
		rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_PROJECT_ID fail\n");
		ret = -1;
	} else {
		fwif_VIP_set_Project_ID(prj_id);
		ret = 0;
	}
	return ret;
}

static long VPQ_IOC_SET_COLOR_TEMP_impl(unsigned long arg)
{
	int ret = 0;
	if (copy_from_user(&ct, (int __user *)arg, sizeof(COLORTEMP_ELEM_T))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_COLOR_TEMP fail\n");
		ret = -1;
	} else {
		memcpy(&curColorTemp, &ct, sizeof(COLORTEMP_ELEM_T));
		vpq_set_color_temp_filter();
		/*rtd_pr_vpq_debug("kernel VPQ_IOC_SET_COLOR_TEMP success\n");*/
		ret = 0;
	}
	return ret;
}

static long VPQ_IOC_SET_COLOR_FILTER_impl(unsigned long arg)
{
	int ret = 0;
	unsigned int args;
	if (copy_from_user(&args, (int __user *)arg, sizeof(unsigned int))) {
		rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_COLOR_FILTER fail\n");
		ret = -1;
	} else {
		ucColorFilterMode = (unsigned char)args;
		vpq_set_color_temp_filter();
		ret = 0;
	}
	return ret;
}

static long VPQ_IOC_GET_MOTION_LEVEL_impl(unsigned long arg)
{
	int ret = 0;
	_clues *smartPic_clue;
	UINT32 PQA_Motion_Level = 0;

	/* Get Info from Share Mem*/
	smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);
	if (0 == smartPic_clue)
		return -1;
	PQA_Motion_Level = smartPic_clue->motion_ratio;
	if (copy_to_user((void __user *)arg, (void *)&PQA_Motion_Level, sizeof(UINT32))) {
		ret = -1;
		rtd_pr_vpq_err("kernel VPQ_IOC_GET_MOTION_LEVEL fail\n");
	} else {
		ret = 0;
		/*rtd_pr_vpq_debug("kernel VPQ_IOC_GET_MOTION_LEVEL success\n");*/
	}
	return ret;
}


static long VPQ_IOC_GET_NOISE_LEVEL_impl(unsigned long arg)
{
	int ret = 0;
	_clues *smartPic_clue;
	UINT32 PQA_Noise_Level = 0;

	/* Get Info from Share Mem*/
	smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);
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
	return ret;
}

static long VPQ_IOC_GET_Input_V_Freq_impl(unsigned long arg)
{
	int ret = 0;
	unsigned int Input_V_Freq;

	Input_V_Freq = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ);
	if (copy_to_user((void __user *)arg, (void *)&Input_V_Freq, sizeof(unsigned int))) {
		ret = -1;
		rtd_pr_vpq_err("kernel VPQ_IOC_GET_Input_V_Freq fail\n");
	} else {
		ret = 0;
		/*rtd_pr_vpq_debug("kernel VPQ_IOC_GET_Input_V_Freq success\n");*/
	}
	return ret;
}

static long VPQ_IOC_SET_GAMMA_LUT_impl(unsigned long arg)
{
	int ret = 0;
	if (copy_from_user(&gamma, (int __user *)arg, sizeof(gamma))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_GAMMA_LUT fail\n");
		ret = -1;
	} else {
		vpq_set_gamma(0, 1);
		fwif_color_gamma_control_back(SLR_MAIN_DISPLAY, 0);
		ret = 0;
	}
	return ret;
}

static long VPQ_IOC_SET_SPLIT_DEMO_impl(unsigned long arg)
{
	int ret = 0;
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
	return ret;
}

static long VPQ_IOC_SET_IRE_INNER_PATTERN_impl(unsigned long arg)
{
	int ret = 0;
	unsigned int args;
	if (copy_from_user(&args, (int __user *)arg, sizeof(unsigned int))) {
		rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_IRE_INNER_PATTERN fail\n");
		ret = -1;
	} else {
		fwif_color_set_WB_Pattern_IRE(args>>16, args&0xffff);
		ret = 0;
	}
	return ret;
}

static long VPQ_IOC_SET_FILM_MODE_impl(unsigned long arg)
{
	int ret = 0;
	unsigned char bCinemaMode;
	if (copy_from_user(&film_mode, (int __user *)arg, sizeof(film_mode))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_FILM_MODE fail\n");
		ret = -1;
	} else {
		down(get_DI_semaphore());
		drvif_module_film_mode((DRV_film_mode *) &film_mode);
		up(get_DI_semaphore());
		/*rtd_pr_vpq_debug("kernel VPQ_IOC_SET_FILM_MODE success\n");*/

		down(&Memc_Realcinema_Semaphore);
		if (film_mode.film_status ==0)
			bCinemaMode = 0;
		else
			bCinemaMode = 1;

		//memc_realcinema_framerate();
		DbgSclrFlgTkr.memc_realcinema_run_flag = TRUE;//Run memc mode

		fwif_color_set_cinema_mode_en(bCinemaMode);
		up(&Memc_Realcinema_Semaphore);
		ret = Scaler_MEMC_set_cinema_mode_en(bCinemaMode);
	}
	return ret;
}

static long VPQ_IOC_SET_FILM_FPS_impl(unsigned long arg)
{
	int ret = 0;
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
	return ret;
}
/* no used
static long VPQ_IOC_SET_SHARPNESS_LEVEL_impl(unsigned long arg)
{
	int ret = 0;
	if (copy_from_user(&Sharpness_Level, (int __user *)arg, sizeof(DRV_Sharpness_Level))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_SHARPNESS_LEVEL fail\n");
		ret = -1;
	} else {
		drvif_color_set_Sharpness_level((DRV_Sharpness_Level *) &Sharpness_Level);
		//rtd_pr_vpq_debug("kernel VPQ_IOC_SET_SHARPNESS_LEVEL success\n");
		ret = 0;
	}
	return ret;
}
*/

static long VPQ_IOC_SET_SHARPNESS_TABLE_impl(unsigned long arg)
{
	int ret = 0;
	static CHIP_SHARPNESS_UI_T tCHIP_SHARPNESS_UI_T;

	if (copy_from_user(&tCHIP_SHARPNESS_UI_T, (void __user *)arg, sizeof(CHIP_SHARPNESS_UI_T))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_SHARPNESS_TABLE fail\n");
		ret = -1;
	} else {
		memcpy(&tCurCHIP_SHARPNESS_UI_T, &tCHIP_SHARPNESS_UI_T, sizeof(CHIP_SHARPNESS_UI_T));
		fwif_color_set_sharpness_table_TV006(&tCurCHIP_SHARPNESS_UI_T, &tCurCHIP_EDGE_ENHANCE_UI_T, &tCurCHIP_SR_UI_T);
		ret = 0;
	}
	return ret;
}

/* no used
static long VPQ_IOC_SET_VPQ_Shp_Val_impl(unsigned long arg)
{
	int ret = 0;
	if (copy_from_user(&tShp_Val, (int __user *)arg, sizeof(UINT16)*7)) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_VPQ_Shp_Val fail\n");
		ret = -1;
	} else {

		//fwif_color_set_sharpness_level_TV006(&tShp_Val[0]);
		rtd_pr_vpq_debug("kernel VPQ_IOC_SET_VPQ_Shp_Val success\n");
		ret = 0;
	}
	return ret;
}
*/

static long VPQ_IOC_SET_EDGE_ENHANCE_impl(unsigned long arg)
{
	int ret = 0;
	CHIP_EDGE_ENHANCE_UI_T tCHIP_EDGE_ENHANCE_UI_T;

	if (copy_from_user((void *)&tCHIP_EDGE_ENHANCE_UI_T,(void __user *)arg , sizeof(CHIP_EDGE_ENHANCE_UI_T))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_EDGE_ENHANCE fail\n");
		ret = -1;
	} else {
		memcpy(&tCurCHIP_EDGE_ENHANCE_UI_T, &tCHIP_EDGE_ENHANCE_UI_T, sizeof(CHIP_EDGE_ENHANCE_UI_T));
		fwif_color_set_sharpness_table_TV006(&tCurCHIP_SHARPNESS_UI_T, &tCurCHIP_EDGE_ENHANCE_UI_T, &tCurCHIP_SR_UI_T);
		ret = 0;
	}
	return ret;
}

static long VPQ_IOC_SET_SR_TABLE_impl(unsigned long arg)
{
	int ret = 0;
	RTK_SR_UI_T tRTK_SR_UI_T;

	if (copy_from_user((void *)&tRTK_SR_UI_T,(void __user *)arg , sizeof(tRTK_SR_UI_T))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_EDGE_ENHANCE fail\n");
		ret = -1;
	} else {
		memcpy(&tCurCHIP_SR_UI_T, &tRTK_SR_UI_T.sr_ui, sizeof(CHIP_SR_UI_T));
		fwif_color_set_sharpness_table_TV006(&tCurCHIP_SHARPNESS_UI_T, &tCurCHIP_EDGE_ENHANCE_UI_T, &tCurCHIP_SR_UI_T);
		ret = 0;
	}
	return ret;
}

/* no used
static long VPQ_IOC_SET_NR_PQA_TABLE_impl(unsigned long arg)
{
	int ret = 0;
	unsigned char PQA_Table_Idx = 0;
	if (copy_from_user(&PQA_Table_Idx, (int __user *)arg, sizeof(unsigned char))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_NR_PQA_TABLE fail\n");
		ret = -1;
	} else {
		fwif_color_set_PQA_table(PQA_Table_Idx);
		//rtd_pr_vpq_debug("kernel VPQ_IOC_SET_NR_PQA_TABLE success\n");
		ret = 0;
	}
	return ret;
}

static long VPQ_IOC_SET_NR_PQA_INPUT_TABLE_impl(unsigned long arg)
{
	int ret = 0;
	unsigned char PQA_Input_Table_Idx = 0;
	if (copy_from_user(&PQA_Input_Table_Idx, (int __user *)arg, sizeof(unsigned char))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_NR_PQA_INPUT_TABLE fail\n");
		ret = -1;
	} else {
		fwif_color_set_PQA_Input_table(PQA_Input_Table_Idx);
		//rtd_pr_vpq_debug("kernel VPQ_IOC_SET_NR_PQA_INPUT_TABLE success\n");
		ret = 0;
	}
	return ret;
}
*/

static long VPQ_IOC_SET_NR_LEVEL_impl(unsigned long arg)
{
	int ret = 0;
	if (copy_from_user(&NR_Level, (int __user *)arg, sizeof(CHIP_NOISE_REDUCTION_T))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_NR_LEVEL fail\n");
		ret = -1;
	} else {
		fwif_color_Set_NR_Table_tv006(&NR_Level);
		fwif_color_SetDNR_tv006(NR_Level.NR_LEVEL);
		rtd_pr_vpq_info("kernel VPQ_IOC_SET_NR_LEVEL success, Lv = %d\n", NR_Level.NR_LEVEL);
		ret = 0;
	}
	return ret;
}

/* no used
static long VPQ_IOC_SET_NR_INPUT_impl(unsigned long arg)
{
	int ret = 0;
	unsigned char args = 0;
	if (copy_from_user(&args, (int __user *)arg, sizeof(unsigned char))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_NR_INPUT fail\n");
		ret = -1;
	} else {
		fwif_color_Set_NR_Input_tv006(args);
		//rtd_pr_vpq_debug("kernel VPQ_IOC_SET_NR_PQA_TABLE success\n");
		ret = 0;
	}
	return ret;
}
*/

static long VPQ_IOC_SET_MPEGNR_LEVEL_impl(unsigned long arg)
{
	int ret = 0;
	if (copy_from_user(&MPEGNR_Level, (void __user *)arg, sizeof(CHIP_MPEG_NOISE_REDUCTION_T))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_MPEGNR_LEVEL fail\n");
		ret = -1;
	} else {
		fwif_color_Set_MPEGNR_Table_tv006(&MPEGNR_Level);
		fwif_color_Set_MPEGNR_tv006(MPEGNR_Level.MPEG_LEVEL);

		//rtd_pr_vpq_info("kernel VPQ_IOC_SET_MPEGNR_LEVEL success, Lv = %d\n", MPEGNR_Level.MPEG_LEVEL);
		ret = 0;
	}
	return ret;
}

static long VPQ_IOC_GET_HistoData_Countbins_impl(unsigned long arg)
{
	int ret = 0;
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
	return ret;
}

static long VPQ_IOC_GET_HistoData_chrm_bin_impl(unsigned long arg)
{
	int ret = 0;
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
	return ret;
}

static long VPQ_IOC_GET_HistoData_hue_bin_impl(unsigned long arg)
{
	int ret = 0;
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
	return ret;
}

static long VPQ_IOC_GET_HistoData_APL_impl(unsigned long arg)
{
	int ret = 0;
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
	return ret;
}

static long VPQ_IOC_GET_HistoData_Min_impl(unsigned long arg)
{
	int ret = 0;
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
	return ret;
}

static long VPQ_IOC_GET_HistoData_Max_impl(unsigned long arg)
{
	int ret = 0;
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
	return ret;
}

static long VPQ_IOC_GET_HistoData_PeakLow_impl(unsigned long arg)
{
	int ret = 0;
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
	return ret;
}

static long VPQ_IOC_GET_HistoData_PeakHigh_impl(unsigned long arg)
{
	int ret = 0;
	signed int DC_peak_high = 0;

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
	return ret;
}

static long VPQ_IOC_GET_HistoData_skin_count_impl(unsigned long arg)
{
	int ret = 0;
	unsigned int skin_count = 0;

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
	return ret;
}

static long VPQ_IOC_GET_HistoData_sat_status_impl(unsigned long arg)
{
	int ret = 0;
	unsigned int sat_status = 0;

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
	return ret;
}

static long VPQ_IOC_GET_HistoData_diff_sum_impl(unsigned long arg)
{
	int ret = 0;
	unsigned int diff_sum = 0;

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
	return ret;
}

static long VPQ_IOC_GET_HistoData_motion_impl(unsigned long arg)
{
	int ret = 0;
	unsigned int motion = 0;

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
	return ret;
}

static long VPQ_IOC_GET_HistoData_texture_impl(unsigned long arg)
{
	int ret = 0;
	unsigned int texture = 0;

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
	return ret;
}

static long VPQ_IOC_SET_FreshContrast_impl(unsigned long arg)
{
	int ret = 0;
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
	return ret;
}

static long VPQ_IOC_SET_FreshContrastLUT_impl(unsigned long arg)
{
	int ret = 0;
	if (fwif_color_vpq_stop_ioctl(STOP_VPQ_IOC_SET_FreshContrastLUT, ACCESS_MODE_GET, 0))
		return 0;

	if (copy_from_user(&tFreshContrastLUT, (int __user *)arg, sizeof(FreshContrastLUT_T))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_FreshContrastLUT fail\n");
		ret = -1;
	} else {
		if (drvif_color_get_WB_pattern_on()) {
			if (!fwif_color_set_bypass_dcc_Curve_Write_tv006()) {
				rtd_pr_vpq_err("kernel fwif_color_set_bypass_dcc_Curve_Write_tv006 fail\n");
					ret = -1;
			}
		} else if (fwif_color_set_dcc_Curve_Write_tv006(&tFreshContrastLUT.FreshContrastLUT[0], 1) == FALSE) {
			rtd_pr_vpq_err("kernel fwif_color_set_dcc_Curve_Write_tv006 fail\n");
			ret = -1;
		} else
			ret = 0;
	}
	return ret;
}

static long VPQ_IOC_GET_FreshContrastBypassLUT_impl(unsigned long arg)
{
	int ret = 0;
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
	return ret;
}

static long VPQ_IOC_SET_ColorGain_impl(unsigned long arg)
{
	int ret = 0;
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
	return ret;
}

static long VPQ_IOC_SET_CLEAR_WHITE_impl(unsigned long arg)
{
	int ret = 0;
	HAL_VPQ_CLEAR_WHITE clear_white;
	DRV_VIP_YUV2RGB_UV_Offset uvOffset;

	if (copy_from_user(&clear_white, (void __user *)arg, sizeof(HAL_VPQ_CLEAR_WHITE))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_CLEAR_WHITE_GAIN fail\n");
		ret = -1;
	} else {
		memcpy(&uvOffset.Uoffset[0], &clear_white.Uoffset[0], sizeof(unsigned char)*VIP_YUV2RGB_Y_Seg_Max);
		memcpy(&uvOffset.Voffset[0], &clear_white.Voffset[0], sizeof(unsigned char)*VIP_YUV2RGB_Y_Seg_Max);
		memcpy(&uvOffset.Ustep[0], &clear_white.Ustep[0], sizeof(unsigned char)*VIP_YUV2RGB_Y_Seg_Max);
		memcpy(&uvOffset.Vstep[0], &clear_white.Vstep[0], sizeof(unsigned char)*VIP_YUV2RGB_Y_Seg_Max);
		memcpy(&uvOffset.UV_index[0], &clear_white.UV_index[0], sizeof(unsigned char)*(VIP_YUV2RGB_Y_Seg_Max-1));
		if (!fwif_color_set_YUV2RGB_UV_Offset_tv006(clear_white.wId, clear_white.enable, clear_white.mode, &uvOffset))
			return -1;
		/*rtd_pr_vpq_debug("kernel VPQ_IOC_SET_CLEAR_WHITE_GAIN,%d,%d, ret =%d\n", level, display, ret);*/
	}
	return ret;
}

static long VPQ_IOC_SET_BLUE_STRETCH_impl(unsigned long arg)
{
	int ret = 0;
	RTK_BLUE_STRETCH_T blue_stretch;

	if (copy_from_user(&blue_stretch, (void __user *)arg, sizeof(RTK_BLUE_STRETCH_T))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_BLUE_STRETCH fail\n");
		ret = -1;
	} else {
		drvif_color_set_Blue_Stretch(&blue_stretch.gamma_bs);
	}
	return ret;
}

static long VPQ_IOC_SET_LOCALCONTRAST_PARAM_impl(unsigned long arg)
{
	int ret = 0;
	if (copy_from_user(&glc_param, (void __user *)arg, sizeof(CHIP_LOCAL_CONTRAST_T))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_LOCALCONTRAST_PARAM fail\n");
		ret = -1;
	} else {
		fwif_color_set_LocalContrast_table_TV006(&glc_param);
		ret = 0;
	}
	return ret;
}

static long VPQ_IOC_SET_INIT_COLOR_ADV_impl(unsigned long arg)
{
	int ret = 0;
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
		fwif_color_set_ICM_table_driver_init_tv006();
		g_flag_cm_adv_init_ok = 1;
	}
	return ret;
}

static long VPQ_IOC_SET_INIT_COLOR_EXP_impl(unsigned long arg)
{
	int ret = 0;
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
		fwif_color_set_ICM_table_driver_init_tv006();
		g_flag_cm_exp_init_ok = 1;
	}
	return ret;
}

static long VPQ_IOC_SET_FRESH_COLOR_impl(unsigned long arg)
{
	int ret = 0;
	CHIP_COLOR_CONTROL_FRESH_T cmControl;
	int ret_compare;

	if (copy_from_user(&cmControl, (void __user *)arg, sizeof(CHIP_COLOR_CONTROL_FRESH_T))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_FRESH_COLOR fail\n");
		return -1;
	}

	if (cmRegionExt.ctrl[0] != 4) //verion4, add keepY
		return -1;

	if (COLOR_MODE_ADV == g_Color_Mode) {
		if (g_flag_cm_adv_init_ok) {
			ret_compare = memcmp(&g_cm_ctrl_adv, &cmControl.cm_ctrl, sizeof(CHIP_COLOR_CONTROL_T));
			if (ret_compare == 0 && !g_cm_need_refresh) {
				return 0;
			}
			memcpy(&g_cm_ctrl_adv, &cmControl.cm_ctrl, sizeof(CHIP_COLOR_CONTROL_T));
			fwif_color_calc_ICM_gain_table_driver_tv006(&g_cm_rgn_adv.stColorRegionType, &g_cm_ctrl_adv, &icm_tab_elem_of_vip_table, &icm_tab_elem_write);
			vpq_update_icm(TRUE);
			g_cm_need_refresh = 0;
		}
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
	return ret;
}

static long VPQ_IOC_SET_PREFERRED_COLOR_impl(unsigned long arg)
{
	int ret = 0;
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
			fwif_color_calc_ICM_gain_table_driver_tv006(&g_cm_rgn_adv.stColorRegionType, &g_cm_ctrl_adv, &icm_tab_elem_of_vip_table, &icm_tab_elem_write);
			vpq_update_icm(TRUE);
			g_Color_Mode = COLOR_MODE_ADV;
			g_cm_need_refresh = 0;
		} else {
			rtd_pr_vpq_warn("[VPQ][kernel] need call HAL_VPQ_InitColorForAdvanced first!\n");
			ret = -1;
		}
	}
	return ret;
}

static long VPQ_IOC_SET_COLOR_MANAGEMENT_impl(unsigned long arg)
{
	int ret = 0;
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
			fwif_color_calc_ICM_gain_table_driver_tv006(&g_cm_rgn_exp.stColorRegionType, &g_cm_ctrl_exp, &icm_tab_elem_of_vip_table, &icm_tab_elem_write);
			vpq_update_icm(TRUE);
			g_Color_Mode = COLOR_MODE_EXP;
			g_cm_need_refresh = 0;
		} else {
			rtd_pr_vpq_warn("[VPQ][kernel] need call HAL_VPQ_InitColorForExpert first!\n");
			ret = -1;
		}
	}
	return ret;
}

/* no used
static long VPQ_IOC_SET_ICM_ELEM_impl(unsigned long arg)
{
	int ret = 0;
	if (copy_from_user(&icm_tab_elem_write, (int __user *)arg, sizeof(COLORELEM_TAB_T))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_ICM_ELEM fail\n");
		ret = -1;
	} else {
		vpq_update_icm(TRUE);
	}
	return ret;
}
*/

static long VPQ_IOC_SET_PIC_CTRL_impl(unsigned long arg)
{
	int ret = 0;

	if (Demo_Flag) {
		vpq_demo_overscan_func(NULL);
		return 0;
	} else if (PQ_Dev_Status != PQ_DEV_INIT_DONE)
		return -1;

	PictureMode_flg = 1;  // for MEMC wrt by JerryWang 20161125
	if (copy_from_user(&pic_ctrl, (int __user *)arg, sizeof(VPQ_SetPicCtrl_T))) {
		rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_PIC_CTRL fail\n");
		ret = -1;
	} else {
		ret = fwif_color_set_Picture_Control_tv006(pic_ctrl.wId, &pic_ctrl);
	}
	return ret;
}

static long VPQ_IOC_SET_BLACK_LEVEL_impl(unsigned long arg)
{
	int ret = 0;
	HAL_VPQ_SET_BLACK_LEVEL blackLv;
	unsigned char mCon, mBri, mHue, mSat;
	unsigned char  input_info;
	unsigned char  level;
	unsigned char isBlack_LvDiff_Flag = 0;
	rtd_pr_vpq_info("\n HDR_MODE, VPQ_IOC_SET_BLACK_LEVEL ");

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

		/*20151128 roger, WOSQRTK-3568, lg want input_info == HAL_VPQ_INPUT_MEDIA_MOVIE, and level == RGB2YUV_BLACK_LEVEL_AUTO then level = RGB2YUV_BLACK_LEVEL_LOW directly*/
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
		fwif_color_SetDataFormatHandler_tv006();
#else
		if ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE) && (Check_AP_Set_Enable_ForceBG(SLR_MAIN_DISPLAY) == FALSE))
		{
			if (isBlack_LvDiff_Flag == 1) {
				fwif_color_SetDataFormatHandler_tv006();
			} else {
				/* seamless. do seamless process in isr (void Scaler_hdr_setting_SEAMLESS()), avoid transient noise*/
				;
			}
		} else {
			fwif_color_SetDataFormatHandler_tv006();
		}
		HDR_SDR_SEAMLESS_PQ.HDR_SDR_SEAMLESS_PQ_SET[HDR_SDR_SEAMLESS_PQ_HAL_PQModeInfo]=1;
#endif
		ret = 0;
	}
	return ret;
}

static long VPQ_IOC_SET_HDR_GAMUTMATRIX_impl(unsigned long arg)
{
	int ret = 0;
	HDR_SetGamut33Matrix_T gamut33;
	rtd_pr_vpq_info("\n HDR_MODE, VPQ_IOC_SET_HDR_GAMUTMATRIX PQModeInfo_flag:%d",PQModeInfo_flag[0]);
	//if (PQModeInfo_flag[0] != HAL_VPQ_HDR_MODE_HDR10) return ret;	/* seamless will only change PQModeInfo_flag_seamless, this condition is not correct*/
	if (copy_from_user(&gamut33, (int __user *)arg, sizeof(HDR_SetGamut33Matrix_T))) {
		rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_HDR_GAMUTMATRIX fail\n");
		ret = -1;
	} else {
		//drvif_HDR_RGB2OPT(1, gamut33.Gamut33Matrix);
		fwif_HDR_RGB2OPT(1, gamut33.Gamut33Matrix);
	}
	return ret;
}

static long VPQ_IOC_SET_HDR_3DLUT_16_impl(unsigned long arg)
{
	int ret = 0;
	UINT8 data_protect_cfu = 100;
	rtd_pr_vpq_info("\n HDR_MODE, VPQ_IOC_SET_HDR_3DLUT_16");
	//20151112 roger, fail ==> try again, I can't get the batter way to fix this copy fail issue
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
		fwif_color_set_DM_HDR_3dLUT_17x17x17_16_TV006(&args);
	#endif
	}
	return ret;
}

static long VPQ_IOC_SET_HDR_3DLUT_24x24x24_16_1_impl(unsigned long arg)
{
	int ret = 0;
	UINT8 data_protect_cfu = 100;
	rtd_pr_vpq_info("\n HDR_MODE, VPQ_IOC_SET_HDR_3DLUT_24x24x24_16_1\n");
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
	return ret;
}

static long VPQ_IOC_SET_HDR_3DLUT_24x24x24_16_2_impl(unsigned long arg)
{
	int ret = 0;
	UINT8 data_protect_cfu = 100;
	rtd_pr_vpq_info("\n HDR_MODE, VPQ_IOC_SET_HDR_3DLUT_24x24x24_16_2");
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
		drvif_Set_DM_HDR_CLK(); // enable DM & Composer clock
		fwif_color_set_DM_HDR_3dLUT_24x24x24_16_TV006(&args);
	}
	return ret;
}

static long VPQ_IOC_SET_HDR_Eotf_impl(unsigned long arg)
{
	int ret = 0;
	HAL_VPQ_DATA_T Eotf_data;
	UINT8 data_protect_cfu = 100;
	rtd_pr_vpq_info("\n HDR_MODE, VPQ_IOC_SET_HDR_Eotf");
	if (copy_from_user(&Eotf_data, (int __user *)arg, sizeof(HAL_VPQ_DATA_T))) {
		rtd_pr_vpq_err("HDR10, Eotf struct copy fail\n");
		ret = -1;
		return ret;
	}

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

		fwif_color_set_DM2_EOTF_TV006(Eotf_table32); //SEAMLESS mode block it
	}
	return ret;
}

static long VPQ_IOC_SET_HDR_Oetf_impl(unsigned long arg)
{
	int ret = 0;
	HAL_VPQ_DATA_T Oetf_data;
	UINT8 data_protect_cfu = 100;
	rtd_pr_vpq_info("\n HDR_MODE, VPQ_IOC_SET_HDR_Oetf ");
	if (copy_from_user(&Oetf_data, (int __user *)arg, sizeof(HAL_VPQ_DATA_T))) {
		rtd_pr_vpq_err("HDR10, Oetf struct copy fail\n");
		ret = -1;
		return ret;
	}

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

		fwif_color_set_DM2_OETF_TV006(Oetf_table16);
	}
	return ret;
}

static long VPQ_IOC_SET_PQModeInfo_impl(unsigned long arg)
{
	int ret = 0;
	HAL_VPQ_DATA_T PQModeInfo_data;
	rtd_pr_vpq_info("HDR main contorl, SET_PQModeInfo\n");
	if (copy_from_user(&PQModeInfo_data, (int __user *)arg, sizeof(HAL_VPQ_DATA_T))) {
		rtd_pr_vpq_err("HDR main contorl, PQModeInfo struct copy fail\n");
		ret = -1;
		return ret;
	}

	if (PQModeInfo_data.length > sizeof(PQModeInfo_flag))
		PQModeInfo_data.length = sizeof(PQModeInfo_flag);

	if(copy_from_user(PQModeInfo_flag, (int __user *)PQModeInfo_data.pData, PQModeInfo_data.length))
	{
		rtd_pr_vpq_err("HDR main contorl, PQModeInfo table copy fail\n");
		ret = -1;
	}
	else
	{

		pr_info("[HDR new flow][VPQ HAL], type = %d\n", PQModeInfo_flag[0]);
		if (PQModeInfo_flag[0] == HAL_VPQ_HDR_MODE_HDR10) {
			rtd_pr_vpq_info("\n HDR_MODE, PQModeInfo_flag :HAL_VPQ_HDR_MODE_HDR10 \n");
		} else if (PQModeInfo_flag[0] == HAL_VPQ_HDR_MODE_HLG) {
			rtd_pr_vpq_info("\nHDR_MODE, PQModeInfo_flag :HAL_VPQ_HDR_MODE_HLG \n");
		} else {
			rtd_pr_vpq_info("\n HDR_MODE, PQModeInfo_flag :HAL_VPQ_HDR_MODE_SDR \n");
		}

		if (PQModeInfo_flag[1] == 0) PQModeInfo_flag[1] = 2;	//data protect
		g_LGE_HDR_CSC_CTRL = PQModeInfo_flag[1];


#ifndef CONFIG_HDR_SDR_SEAMLESS	/* seamless need to check force bg and mode active*/
		drvif_TV006_SET_HDR_mode(PQModeInfo_flag);
		fwif_color_SetDataFormatHandler_tv006();
#else
		if (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)
		{
			rtd_pr_vpq_info("\nHDR_MODE, PQModeInfo: seamless flow\n");
			/* seamless. do seamless process in isr (void Scaler_hdr_setting_SEAMLESS()), avoid transient noise*/;
		} else {
			rtd_pr_vpq_info("\nHDR_MODE, PQModeInfo: scaler flow\n");
			drvif_TV006_SET_HDR_mode(PQModeInfo_flag);
			fwif_color_SetDataFormatHandler_tv006();
		}
		HDR_SDR_SEAMLESS_PQ.HDR_SDR_SEAMLESS_PQ_SET[HDR_SDR_SEAMLESS_PQ_HAL_PQModeInfo]=1;
#endif
	}
	return ret;
}

static long VPQ_IOC_GET_HDR_PicInfo_impl(unsigned long arg)
{
	int ret = 0;
	UINT32 HDR_info_bin[131] = {0};
	//get histogram protect
	if (fwif_color_get_DM_HDR10_enable_TV006() == 0) {
		//rtd_pr_vpq_notice("kernel fwif_color_get_DM2_HDR_histogram_TV006 should not run in SDR content\n");
		ret = -1;
		return ret;
	}

	if (fwif_color_get_DM2_HDR_histogram_TV006(HDR_info_bin) == 0) {
		rtd_pr_vpq_err("kernel fwif_color_get_DM2_HDR_histogram_TV006 fail\n");
		ret = -1;
	} else {
		if (copy_to_user((void __user *)arg, HDR_info_bin, 131 * 4)) {
			rtd_pr_vpq_err("kernel VPQ_IOC_GET_HDR_PicInfo fail\n");
			ret = -1;
		} else
			ret = 0;
	}
	return ret;
}

static long VPQ_IOC_SET_HDR_InvGamma_impl(unsigned long arg)
{
	int ret = 0;
	UINT8 HDR_InvGamma_en = 0;
	rtd_pr_vpq_info("HDR10, SET_HDR_InvGamma\n");
	if (copy_from_user(&HDR_InvGamma_en, (void __user *)arg, 1)) {
		rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_HDR_InvGamma fail\n");
		ret = -1;
	} else {
#ifndef CONFIG_HDR_SDR_SEAMLESS	/* seamless need to check force bg and mode active*/
		drvif_DM2_GAMMA_Enable(HDR_InvGamma_en);
#else
		if ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE) && (Check_AP_Set_Enable_ForceBG(SLR_MAIN_DISPLAY) == FALSE))
		{
			g_InvGamma = HDR_InvGamma_en;

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
	return ret;
}


static long VPQ_IOC_SET_COLOR_GAMUT_impl(unsigned long arg)
{
	int ret = 0;
	if (copy_from_user(&gamutData, (void __user *)arg, sizeof(COLOR_GAMUT_T))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_COLOR_GAMUT fail\n");
		ret = -1;
	} else {
		unsigned char invgamma_enable;
		short sRGBMatrix[3][3];

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

		sRGBMatrix[0][0] = gamutData.gamutMapping.gamutmatrix_00;
		sRGBMatrix[0][1] = gamutData.gamutMapping.gamutmatrix_01;
		sRGBMatrix[0][2] = gamutData.gamutMapping.gamutmatrix_02;
		sRGBMatrix[1][0] = gamutData.gamutMapping.gamutmatrix_10;
		sRGBMatrix[1][1] = gamutData.gamutMapping.gamutmatrix_11;
		sRGBMatrix[1][2] = gamutData.gamutMapping.gamutmatrix_12;
		sRGBMatrix[2][0] = gamutData.gamutMapping.gamutmatrix_20;
		sRGBMatrix[2][1] = gamutData.gamutMapping.gamutmatrix_21;
		sRGBMatrix[2][2] = gamutData.gamutMapping.gamutmatrix_22;

		fwif_color_ColorMap_SetMatrix(SLR_MAIN_DISPLAY, sRGBMatrix, 2, g_srgbForceUpdate, 0);
		g_srgbForceUpdate = 0;

		/*=== Gamut 3D LUT ===*/
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
		/*====================*/
		ret = 0;
	}
	return ret;
}

static long VPQ_IOC_SET_SAT_LUT_impl(unsigned long arg)
{
	int ret = 0;
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
	return ret;
}

static long VPQ_IOC_SET_DITHER_BIT_impl(unsigned long arg)
{
	int ret = 0;
	unsigned int args;
	if (copy_from_user(&args, (int __user *)arg, sizeof(unsigned int))) {
		rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_DITHER_BIT fail\n");
		ret = -1;
	} else {
		fwif_color_set_Panel_Dither(TRUE,args,VIP_DITHER_PANELTABLE_BLANCE_ROLL_15);
		ret = 0;
	}
	return ret;
}

/* no used
static long VPQ_IOC_SET_GAMUT_3D_LUT_N9_BUFFER_impl(unsigned long arg)
{
	int ret = 0;
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
	return ret;
}
*/

static long VPQ_IOC_SET_GAMUT_3D_LUT_N17_BUFFER_impl(unsigned long arg)
{
	int ret = 0;
	if (copy_from_user(&g_buf3DLUT_LGDB.pt, (void __user *)arg, sizeof(GAMUT_3D_LUT_PT_T)*4913)) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_GAMUT_3D_LUT_N17_BUFFER fail\n");
		ret = -1;
	} else {
		g_bGamut3DLUT_LGDB_NeedUpdated = 1;
	}
	return ret;
}

static long VPQ_IOC_SET_GAMUT_3D_LUT_INDEX_impl(unsigned long arg)
{
	int ret = 0;
	unsigned int value;
	if (copy_from_user(&value, (void __user *)arg, sizeof(unsigned int))) {
		rtd_pr_vpq_err("kernel VPQ_IOC_SET_GAMUT_3D_LUT_INDEX fail\n");
		ret = -1;
	} else {
		fwif_color_set_3dLUT(value);
		ret = 0;
	}
	return ret;
}

static long VPQ_IOC_SET_LOWDELAY_MODE_impl(unsigned long arg)
{
	int ret = 0;
	game_process = 0;
	if (Get_Factory_SelfDiagnosis_Mode()) {
		ret = 0;
	} else {
		bool lowdelaymode;
		static bool pre_lowdelaymode;
		if (copy_from_user(&lowdelaymode, (int __user *)arg, sizeof(bool))) {
			rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_LOWDELAY_MODE fail\n");
			ret = -1;
		} else {
			game_process = 1;
			fw_scalerip_set_di_gamemode(lowdelaymode);
			pre_lowdelaymode = lowdelaymode;
			if(((lowdelaymode == FALSE) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE))||(Get_Val_vsc_run_pc_mode() == TRUE&&(Get_DisplayMode_Src(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY)) == VSC_INPUTSRC_HDMI))){ //not game mode and not _MODE_STATE_ACTIVE
				//fw_scalerip_set_di_gamemode(lowdelaymode);
				//drv_memory_set_game_mode_dynamic(_DISABLE);
				fw_scalerip_set_di_gamemode_setting(_DISABLE);
				//ret = 0;
				//return ret;
			}
			if (g_bDIGameModeOnlyDebug) //for di game mode debug, only can enabled by rtice tool
				return ret;
#if 0
			if(((pre_lowdelaymode == lowdelaymode) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE))) {
				//fw_scalerip_set_di_gamemode(lowdelaymode);
				ret = 0;
				return ret;
			}
#endif
			if(lowdelaymode) {
				//printk(KERN_EMERG"[crixus]VPQ_IOC_SET_LOWDELAY_MODE1\n");
				//game mode dynamic, check in vsc task @Crixus 20151228
				drv_memory_set_game_mode_dynamic(_ENABLE);
			} else {
				//printk(KERN_EMERG"[crixus]VPQ_IOC_SET_LOWDELAY_MODE0\n");
				//game mode dynamic, check in vsc task @Crixus 20151228
				drv_memory_set_game_mode_dynamic(_DISABLE);
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
	return ret;
}

static void vpq_set_pod_table(unsigned char* pLUT)
{
	drvif_color_pcid_enable(0); // PCID and PCID2 cannot enable at the same time
	fwif_color_set_pcid2_valuetable(pLUT);
	fwif_color_set_pcid2_poltable();
	fwif_color_set_pcid2_pixel_setting();
	fwif_color_set_pcid_RgnWeight();
	drvif_color_pcid2_enable(1);
	g_pcid2_en = 1;
	g_pcid_en = 0;
}

static void vpq_set_pcid_table(unsigned char* pLUT)
{
	drvif_color_pcid2_enable(0); // PCID and PCID2 cannot enable at the same time
	fwif_color_set_pcid_valuetable(pLUT);
	drvif_color_pcid_poltable();
	fwif_color_set_pcid_pixel_setting();
	fwif_color_set_pcid_RgnWeight();
	drvif_color_pcid_enable(1);
	g_pcid_en = 1;
	g_pcid2_en = 0;
}

static void vpq_set_valc_table(unsigned char* pLUT)
{
	drvif_color_pcid2_enable(0); // PCID and PCID2 cannot enable at the same time
	fwif_color_set_pcid_valuetable(pLUT);
	drvif_color_pcid_poltable();
	fwif_color_set_pcid_pixel_setting();
	fwif_color_set_pcid_RgnWeight();
	drvif_color_pcid_enable(1);
	drvif_color_pcid_VALC_enable(1);
	g_pcid_en = 1;
	g_pcid2_en = 0;
}

static long Process_OD_Ext_Data(HAL_VPQ_OD_Extention_T OD_Ext_Data)
{
	int ret = 0;
	unsigned int tab_len[OD_TYPE_ENUM_MAX] = {POD_TABLE_LEN, PCID_TABLE_LEN, PCID_TABLE_LEN};
	unsigned char *name[OD_TYPE_ENUM_MAX] = {"POD", "PCID", "VALC"};
	unsigned int tab_length;

	if (OD_Ext_Data.extType >= OD_TYPE_ENUM_MAX) {
		rtd_pr_vpq_err("OD Extend type error = %d error\n", OD_Ext_Data.extType);
		return -1;
	}

	tab_length = tab_len[OD_Ext_Data.extType];
	if( OD_Ext_Data.extLength != tab_length )
	{
		rtd_pr_vpq_err("POD LUT length = %d error!\n", OD_Ext_Data.extLength);
		ret = -1;
	}
	else
	{
		unsigned char* pLUT = NULL;
		pLUT = (unsigned char *)dvr_malloc(tab_length*sizeof(unsigned char));
		if(pLUT == NULL)
		{
			rtd_pr_vpq_err("[ERROR] VPQ_IOC_SET_PCID %s Table allocate fail\n", name[OD_Ext_Data.extType]);
			return -1;
		}

		if( copy_from_user(pLUT, (int __user *)OD_Ext_Data.pExtData, tab_length*sizeof(UINT8)) )
		{
			rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_PCID_pData(%s) fail\n", name[OD_Ext_Data.extType]);
			ret = -1;
		}
		else
		{
			switch(OD_Ext_Data.extType) {
			default:
			case OD_TYPE_POD:
				vpq_set_pod_table(pLUT);
			break;
			case OD_TYPE_PCID:
				vpq_set_pcid_table(pLUT);
			break;
			case OD_TYPE_VALC:
				vpq_set_valc_table(pLUT);
			break;
			}
			ret = 0;
		}
		dvr_free(pLUT);
	}
	return ret;
}

static long VPQ_IOC_SET_PCID_impl(unsigned long arg)
{
	int ret = 0;
	HAL_VPQ_DATA_T OD_Ext_Info;
	HAL_VPQ_OD_Extention_T OD_Ext_Data;

	if( copy_from_user(&OD_Ext_Info, (int __user *)arg, sizeof(HAL_VPQ_DATA_T)) )
	{
		rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_PCID fail\n");
		ret = -1;
		return ret;
	}
	else
	{
		if( copy_from_user(&OD_Ext_Data, (int __user *)OD_Ext_Info.pData, sizeof(HAL_VPQ_OD_Extention_T)) )
		{
			rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_PCID_pData fail\n");
			ret = -1;
		}
		else
		{
			ret = Process_OD_Ext_Data(OD_Ext_Data);
		}
	}
	return ret;
}

static long VPQ_IOC_SET_Od_impl(unsigned long arg)
{
#ifndef BUILD_QUICK_SHOW

	int ret = 0;
	extern unsigned char od_table_tv006_store[OD_table_length];
	HAL_VPQ_DATA_T OD_Info;

	enum PLAFTORM_TYPE platform = get_platform();
	if (PLATFORM_KXLP != platform)
		return -1;

	if (copy_from_user(&OD_Info, (int __user *)arg, sizeof(HAL_VPQ_DATA_T))) {
		rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_PCID fail\n");
		ret = -1;
		return ret;
	} else {
		if (OD_Info.length > OD_table_length)
			return -1;
		if (copy_from_user(&od_table_tv006_store[0], (int __user *)OD_Info.pData, (OD_Info.length)*sizeof(unsigned char))) {
			rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_OdExtentions_pData fail\n");
			ret = -1;
		} else {
			extern unsigned char bODInited;
			extern unsigned char bODTableLoaded;
			extern unsigned char bODPreEnable;
			extern char od_table_mode_store;
			extern unsigned int od_table_store[OD_table_length];
			extern unsigned int od_delta_table[OD_table_length];
			fwif_color_od_table_tv006_convert(od_table_tv006_store, od_table_store);

			fwif_color_od_table_tv006_transform(0, od_table_store, od_delta_table);

			drvif_color_od_table(od_delta_table, 0);// 0:delta mode 1:target mode(output mode)
			/*
			if (g_pcid2_en)
				drvif_color_pcid2_enable(1);
			else if (g_pcid_en)
				drvif_color_pcid_enable(1);
			*/
			od_table_mode_store = 1;// 0:target mode 1:delta mode -1:inverse mode
			bODTableLoaded = TRUE;

			/*Enable OD*/
			bODPreEnable = TRUE;
			if (!bODInited)
				return -1;
			fwif_color_set_od(TRUE);
			ret = 0;
		}
	}
	return ret;
#else
	return 0;

#endif
}

static long VPQ_IOC_SET_LMMH_DeMura_impl(unsigned long arg)
{
	int ret = 0;
	HAL_VPQ_DATA_T DeMura_Info;
	if (copy_from_user(&DeMura_Info, (int __user *)arg, sizeof(HAL_VPQ_DATA_T))) {
		rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_DeMura fail\n");
		ret = -1;
		return ret;
	} else {
#if 0	/* "Demura_LMMH_4ch_TBL" is too large, use dynamic allocate */
		if (copy_from_user(&Demura_LMMH_4ch_TBL[0], (int __user *)DeMura_Info.pData, (DeMura_Info.length)*sizeof(unsigned char))) {
			rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_DeMura fail\n");
			ret = -1;
		} else {
			vpq_LMMH_demura_data_to_dma(&Demura_LMMH_4ch_TBL[0], &DeMura_TBL, DeMura_TBL.table_mode);
			fwif_color_DeMura_init();
			/*rtd_pr_vpq_info("VIP_Demura_4ch_TBL=%d,%d,%d,%d,%d,%d,%d,%d,\n",
				VIP_Demura_4ch_TBL[0], VIP_Demura_4ch_TBL[1],VIP_Demura_4ch_TBL[5],VIP_Demura_4ch_TBL[6], VIP_Demura_4ch_TBL[DeMura_Info.length-1],
				VIP_Demura_4ch_TBL[DeMura_Info.length-4],VIP_Demura_4ch_TBL[DeMura_Info.length-5],VIP_Demura_4ch_TBL[DeMura_Info.length-6]);*/
			ret = 0;

		}
#else
#if 1
/*========================= for INNX demura =================================*/
		if (DeMura_Info.length == INNX_Demura_TBL_Size && DeMura_Info.version == 2) {	/* INNX */
			INNX_Demura_TBL = (unsigned char *)dvr_malloc_specific(INNX_Demura_TBL_Size * sizeof(char), GFP_DCU2_FIRST);
			pINNX_Demura_DeLut = (unsigned char *)dvr_malloc_specific(INNX_Demura_DecodeLUT_Size * sizeof(char), GFP_DCU2_FIRST);
			if (INNX_Demura_TBL == NULL) {
				rtd_pr_vpq_emerg("INNX Demura dynamic alloc demura memory fail!!!\n");
				ret = -1;
			} else {
				if (copy_from_user(&INNX_Demura_TBL[0], (int __user *)DeMura_Info.pData, (DeMura_Info.length)*sizeof(unsigned char))) {
					rtd_pr_vpq_err("kernel copy VPQ_IOC_SET_DeMura fail for INNX\n");
					ret = -1;
				} else {
					fwif_color_INNX_Demura_TBL(INNX_Demura_TBL, DeMura_Info.length, pINNX_Demura_DeLut);
					vpq_INNX_Demura_data_to_dma(&DeMura_TBL, pINNX_Demura_DeLut);
					fwif_color_DeMura_init();
					ret = 0;
				}
				dvr_free((void *)INNX_Demura_TBL);
				dvr_free((void *)pINNX_Demura_DeLut);
			}
		} else if (DeMura_Info.length == Demura_LMMH_TBL_Size)
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
					fwif_color_DeMura_init();
					rtd_pr_vpq_info("dynamic alloc,  VIP_Demura_4ch_TBL=%d,%d,%d,%d,%d,%d,%d,%d,\n",
						Demura_LMMH_4ch_TBL[0], Demura_LMMH_4ch_TBL[1],Demura_LMMH_4ch_TBL[5],Demura_LMMH_4ch_TBL[6], Demura_LMMH_4ch_TBL[DeMura_Info.length-1],
						Demura_LMMH_4ch_TBL[DeMura_Info.length-4],Demura_LMMH_4ch_TBL[DeMura_Info.length-5],Demura_LMMH_4ch_TBL[DeMura_Info.length-6]);
					ret = 0;
				}
				dvr_free((void *)Demura_LMMH_4ch_TBL);
			}
		} else {
			ret = -1;
		}
#endif
	}
	return ret;
}

#if defined(CONFIG_RTK_AI_DRV)
static long VPQ_IOC_SET_AI_PA_impl(unsigned long arg)
{
	int ret = 0;
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

	VIP_NN_CTRL *pNN;
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
		if (copy_to_user((void __user *)arg,(void *) pNN, sizeof(VIP_NN_CTRL))) {
			rtd_pr_vpq_emerg("kernel VPQ_IOC_SET_AI_PA fail\n");
			ret = -1;
		} else
			ret = 0;
	}
	return ret;
}
#endif

long vpq_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
	int ret = 0;

	if(vpq_get_VPQ_TSK_Stop() ==1){
		rtd_pr_vpq_emerg("vpq hal function block (%s:%d) /n", __FUNCTION__, __LINE__);
		return 0;
	}

	if (vpq_ioctl_get_stop_run(cmd))
		return 0;
	pr_emerg("vpq_ioctl =%d \n",cmd);

	switch(cmd) {
	case VPQ_IOC_INIT:
		return VPQ_IOC_INIT_impl(arg);
	case VPQ_IOC_UNINIT:
		return VPQ_IOC_UNINIT_impl(arg);
	case VPQ_IOC_OPEN:
		return VPQ_IOC_OPEN_impl(arg);
	case VPQ_IOC_CLOSE:
		return VPQ_IOC_CLOSE_impl(arg);
	case VPQ_IOC_SET_PROJECT_ID:
		return VPQ_IOC_SET_PROJECT_ID_impl(arg);
	case VPQ_IOC_SET_PIC_CTRL:
		return VPQ_IOC_SET_PIC_CTRL_impl(arg);
		break;
	default:
		;
	}

	if (PQ_Dev_Status != PQ_DEV_INIT_DONE)
		return -1;

	switch (cmd) {
	case VPQ_IOC_SET_COLOR_TEMP:
		ret = VPQ_IOC_SET_COLOR_TEMP_impl(arg);
	break;
	case VPQ_IOC_SET_COLOR_FILTER:
		ret = VPQ_IOC_SET_COLOR_FILTER_impl(arg);
	break;
	case VPQ_IOC_GET_MOTION_LEVEL:
		ret = VPQ_IOC_GET_MOTION_LEVEL_impl(arg);
	break;
	case VPQ_IOC_GET_NOISE_LEVEL:
		ret = VPQ_IOC_GET_NOISE_LEVEL_impl(arg);
	break;
	case VPQ_IOC_GET_Input_V_Freq:
		ret = VPQ_IOC_GET_Input_V_Freq_impl(arg);
	break;
	case VPQ_IOC_SET_GAMMA_LUT:
		ret = VPQ_IOC_SET_GAMMA_LUT_impl(arg);
	break;
	case VPQ_IOC_SET_SPLIT_DEMO:
		ret = VPQ_IOC_SET_SPLIT_DEMO_impl(arg);
	break;
	case VPQ_IOC_SET_IRE_INNER_PATTERN:
		ret = VPQ_IOC_SET_IRE_INNER_PATTERN_impl(arg);
	break;
	case VPQ_IOC_SET_FILM_MODE:
		ret = VPQ_IOC_SET_FILM_MODE_impl(arg);
	break;
	case VPQ_IOC_SET_FILM_FPS:
		ret = VPQ_IOC_SET_FILM_FPS_impl(arg);
	break;
	/*case VPQ_IOC_SET_SHARPNESS_LEVEL: //no used case
		ret = VPQ_IOC_SET_SHARPNESS_LEVEL_impl(arg);
	break;*/
	case VPQ_IOC_SET_SHARPNESS_TABLE:
		ret = VPQ_IOC_SET_SHARPNESS_TABLE_impl(arg);
	break;
	/*case VPQ_IOC_SET_VPQ_Shp_Val: //no used case
		ret = VPQ_IOC_SET_VPQ_Shp_Val_impl(arg);
	break;*/
	case VPQ_IOC_SET_EDGE_ENHANCE:
		ret = VPQ_IOC_SET_EDGE_ENHANCE_impl(arg);
	break;
	case VPQ_IOC_SET_SR_TABLE:
		ret = VPQ_IOC_SET_SR_TABLE_impl(arg);
	break;
	/*case VPQ_IOC_SET_NR_PQA_TABLE: //no used case
		ret = VPQ_IOC_SET_NR_PQA_TABLE_impl(arg);
	break;
	case VPQ_IOC_SET_NR_PQA_INPUT_TABLE: //no used case
		ret = VPQ_IOC_SET_NR_PQA_INPUT_TABLE_impl(arg);
	break;*/
	case VPQ_IOC_SET_NR_LEVEL:
		ret = VPQ_IOC_SET_NR_LEVEL_impl(arg);
	break;
	/*case VPQ_IOC_SET_NR_INPUT: //no used case
		ret = VPQ_IOC_SET_NR_INPUT_impl(arg);
	break; */
	case VPQ_IOC_SET_MPEGNR_LEVEL:
		ret = VPQ_IOC_SET_MPEGNR_LEVEL_impl(arg);
	break;
	case VPQ_IOC_GET_HistoData_Countbins:
		ret = VPQ_IOC_GET_HistoData_Countbins_impl(arg);
	break;
	case VPQ_IOC_GET_HistoData_chrm_bin:
		ret = VPQ_IOC_GET_HistoData_chrm_bin_impl(arg);
	break;
	case VPQ_IOC_GET_HistoData_hue_bin:
		ret = VPQ_IOC_GET_HistoData_hue_bin_impl(arg);
	break;
	case VPQ_IOC_GET_HistoData_APL:
		ret = VPQ_IOC_GET_HistoData_APL_impl(arg);
	break;
	case VPQ_IOC_GET_HistoData_Min:
		ret = VPQ_IOC_GET_HistoData_Min_impl(arg);
	break;
	case VPQ_IOC_GET_HistoData_Max:
		ret = VPQ_IOC_GET_HistoData_Max_impl(arg);
	break;
	case VPQ_IOC_GET_HistoData_PeakLow:
		ret = VPQ_IOC_GET_HistoData_PeakLow_impl(arg);
	break;
	case VPQ_IOC_GET_HistoData_PeakHigh:
		ret = VPQ_IOC_GET_HistoData_PeakHigh_impl(arg);
	break;
	case VPQ_IOC_GET_HistoData_skin_count:
		ret = VPQ_IOC_GET_HistoData_skin_count_impl(arg);
	break;
	case VPQ_IOC_GET_HistoData_sat_status:
		ret = VPQ_IOC_GET_HistoData_sat_status_impl(arg);
	break;
	case VPQ_IOC_GET_HistoData_diff_sum:
		ret = VPQ_IOC_GET_HistoData_diff_sum_impl(arg);
	break;
	case VPQ_IOC_GET_HistoData_motion:
		ret = VPQ_IOC_GET_HistoData_motion_impl(arg);
	break;
	case VPQ_IOC_GET_HistoData_texture:
		ret = VPQ_IOC_GET_HistoData_texture_impl(arg);
	break;
	case VPQ_IOC_SET_FreshContrast:
		ret = VPQ_IOC_SET_FreshContrast_impl(arg);
	break;
	case VPQ_IOC_SET_FreshContrastLUT:
		ret = VPQ_IOC_SET_FreshContrastLUT_impl(arg);
	break;
	case VPQ_IOC_GET_FreshContrastBypassLUT:
		ret = VPQ_IOC_GET_FreshContrastBypassLUT_impl(arg);
	break;
	case VPQ_IOC_SET_ColorGain:
		ret = VPQ_IOC_SET_ColorGain_impl(arg);
	break;
	case VPQ_IOC_SET_CLEAR_WHITE:
		ret = VPQ_IOC_SET_CLEAR_WHITE_impl(arg);
	break;
	case VPQ_IOC_SET_BLUE_STRETCH:
		ret = VPQ_IOC_SET_BLUE_STRETCH_impl(arg);
	break;
	case VPQ_IOC_SET_LOCALCONTRAST_PARAM:
		ret = VPQ_IOC_SET_LOCALCONTRAST_PARAM_impl(arg);
	break;
	case VPQ_IOC_SET_INIT_COLOR_ADV:
		ret = VPQ_IOC_SET_INIT_COLOR_ADV_impl(arg);
	break;
	case VPQ_IOC_SET_INIT_COLOR_EXP:
		ret = VPQ_IOC_SET_INIT_COLOR_EXP_impl(arg);
	break;
	case VPQ_IOC_SET_FRESH_COLOR:
		ret = VPQ_IOC_SET_FRESH_COLOR_impl(arg);
	break;
	case VPQ_IOC_SET_PREFERRED_COLOR:
		ret = VPQ_IOC_SET_PREFERRED_COLOR_impl(arg);
	break;
	case VPQ_IOC_SET_COLOR_MANAGEMENT:
		ret = VPQ_IOC_SET_COLOR_MANAGEMENT_impl(arg);
	break;
	/*case VPQ_IOC_SET_ICM_ELEM: //no used case
		ret = VPQ_IOC_SET_ICM_ELEM_impl(arg);
	break;*/
	case VPQ_IOC_SET_BLACK_LEVEL:
		ret = VPQ_IOC_SET_BLACK_LEVEL_impl(arg);
	break;
	case VPQ_IOC_SET_HDR_GAMUTMATRIX:
		ret = VPQ_IOC_SET_HDR_GAMUTMATRIX_impl(arg);
	break;
	case VPQ_IOC_SET_HDR_3DLUT_16: //new
		ret = VPQ_IOC_SET_HDR_3DLUT_16_impl(arg);
	break;
	case VPQ_IOC_SET_HDR_3DLUT_24x24x24_16_1:
		ret = VPQ_IOC_SET_HDR_3DLUT_24x24x24_16_1_impl(arg);
	break;
	case VPQ_IOC_SET_HDR_3DLUT_24x24x24_16_2:
		ret = VPQ_IOC_SET_HDR_3DLUT_24x24x24_16_2_impl(arg);
	break;
	case VPQ_IOC_SET_HDR_Eotf:
		ret = VPQ_IOC_SET_HDR_Eotf_impl(arg);
	break;
	case VPQ_IOC_SET_HDR_Oetf:
		ret = VPQ_IOC_SET_HDR_Oetf_impl(arg);
	break;
	case VPQ_IOC_SET_PQModeInfo:
		ret = VPQ_IOC_SET_PQModeInfo_impl(arg);
	break;
	case VPQ_IOC_GET_HDR_PicInfo:
		ret = VPQ_IOC_GET_HDR_PicInfo_impl(arg);
	break;
	case VPQ_IOC_SET_HDR_InvGamma:
		ret = VPQ_IOC_SET_HDR_InvGamma_impl(arg);
	break;
	case VPQ_IOC_HDR_DEMO_Mode_ON:
		Scaler_color_set_HDR_DEMO_Mode_ON();
	break;
	case VPQ_IOC_HDR_DEMO_Mode_OFF:
		Scaler_color_set_HDR_DEMO_Mode_OFF();
	break;
	case VPQ_IOC_SET_COLOR_GAMUT:
		ret = VPQ_IOC_SET_COLOR_GAMUT_impl(arg);
	break;
	case VPQ_IOC_SET_SAT_LUT:
		ret = VPQ_IOC_SET_SAT_LUT_impl(arg);
	break;
	case VPQ_IOC_SET_DITHER_BIT:
		ret = VPQ_IOC_SET_DITHER_BIT_impl(arg);
	break;
	/*case VPQ_IOC_SET_GAMUT_3D_LUT_N9_BUFFER: //no used
		ret = VPQ_IOC_SET_GAMUT_3D_LUT_N9_BUFFER_impl(arg);
	break;*/
	case VPQ_IOC_SET_GAMUT_3D_LUT_N17_BUFFER:
		ret = VPQ_IOC_SET_GAMUT_3D_LUT_N17_BUFFER_impl(arg);
	break;
	case VPQ_IOC_SET_GAMUT_3D_LUT_INDEX:
		ret = VPQ_IOC_SET_GAMUT_3D_LUT_INDEX_impl(arg);
	break;
	case VPQ_IOC_SET_LOWDELAY_MODE:
		ret = VPQ_IOC_SET_LOWDELAY_MODE_impl(arg);
	break;
	case VPQ_IOC_SET_PCID:
		ret = VPQ_IOC_SET_PCID_impl(arg);
	break;
	case VPQ_IOC_SET_Od:
		ret = VPQ_IOC_SET_Od_impl(arg);
	break;
	case VPQ_IOC_SET_LMMH_DeMura:
		ret = VPQ_IOC_SET_LMMH_DeMura_impl(arg);
	break;
#if defined(CONFIG_RTK_AI_DRV)
	case VPQ_IOC_SET_AI_PA:
		ret = VPQ_IOC_SET_AI_PA_impl(arg);
	break;
#endif
	default:
		rtd_pr_vpq_debug("kernel IO command %d not found!\n", cmd);
		return -1;
			;
	}
	return ret;/*Success*/

}

#ifndef UT_flag

struct file_operations vpq_fops = {
	.owner = THIS_MODULE,
	.open = vpq_open,
	.release = vpq_release,
	.read  = vpq_read,
	.write = vpq_write,
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
#ifdef CONFIG_HIBERNATION
	.freeze 	= vpq_suspend_std,
	.thaw		= vpq_resume_std,
	.poweroff	= vpq_suspend_std,
	.restore	= vpq_resume_std,
#endif //CONFIG_HIBERNATION

};
#endif //CONFIG_PM

static struct class *vpq_class = NULL;
static struct platform_driver vpq_platform_driver = {
	.driver = {
		.name = "vpqdev",
		.bus = &platform_bus_type,
#ifdef CONFIG_PM
		.pm = &vpq_pm_ops,
#endif //CONFIG_PM
    },
} ;

static dev_t vpq_devno;/*vpq device number*/
static struct cdev vpq_cdev;

static char *vpq_devnode(struct device *dev, umode_t *mode)
{
	return NULL;
}

int vpq_module_init(void)
{
	int result;
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
	sema_init(&VPQ_DM_DMA_TBL_Semaphore,1);
	//sema_init(&VPQ_DM_3DLUT_Semaphore,1);
	sema_init(&Gamma_Semaphore,1);

	// for low power mode, run time pm
	Set_Var_VPQ_rtpm_tsk_block(0);		// init status is on for socts HEVC test. so vsc need to set tsk block while vsc is suspend for VSC socts.
	Set_Var_VPQ_rtpm_tsk_block_VPQ_done(0);		// default tsk is on, so this flag need to be set by tsk
	Set_Var_VPQ_rtpm_tsk_VPQ_freeze_stauts(0);

	pm_runtime_forbid(&vpq_platform_devs->dev);
	pm_runtime_set_active(&vpq_platform_devs->dev);
	pm_runtime_enable(&vpq_platform_devs->dev);
#ifdef CONFIG_RTK_LOW_POWER_MODE
	rtk_lpm_add_device(&vpq_platform_devs->dev);
#endif
	VPQ_PM_LOG("info", "start", "VPQ_RTPM module init, enable");






	create_VPQ_tsk();
#if defined(CONFIG_RTK_AI_DRV)
	create_se_tsk();
#endif
	vpq_boot_init();
	fwif_color_reg_demo_callback(DEMO_CALLBACKID_ON_OFF_SWITCH, vpq_demo_pq_func);
	fwif_color_reg_demo_callback(DEMO_CALLBACKID_OVERSCAN, vpq_demo_overscan_func);

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

	delete_VPQ_tsk();
#if defined(CONFIG_RTK_AI_DRV)
	delete_se_tsk();
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

module_init(vpq_module_init);
module_exit(vpq_module_exit);
#endif
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


unsigned char Get_Var_VPQ_rtpm_tsk_block(void)
{
	return vpq_rtpm_tsk_block;
}
void Set_Var_VPQ_rtpm_tsk_block(unsigned char rtpm_tsk_block)
{
	vpq_rtpm_tsk_block = rtpm_tsk_block;
}

unsigned char Get_Var_VPQ_rtpm_tsk_block_VPQ_done(void)
{
	return vpq_rtpm_tsk_block_VPQ_done;
}

void Set_Var_VPQ_rtpm_tsk_block_VPQ_done(unsigned char block_VPQ_done)
{
	vpq_rtpm_tsk_block_VPQ_done = block_VPQ_done;
}

unsigned char Get_Var_VPQ_rtpm_tsk_VPQ_freeze_stauts(void)
{
	return vpq_rtpm_tsk_VPQ_freeze_stauts;
}

void Set_Var_VPQ_rtpm_tsk_VPQ_freeze_stauts(unsigned char VPQ_freeze_stauts)
{
	vpq_rtpm_tsk_VPQ_freeze_stauts = VPQ_freeze_stauts;
}

unsigned char Get_Var_VPQ_fw_tsk_running_flag(void)
{
	return VPQ_fw_tsk_running_flag;
}

void Set_Var_VPQ_fw_tsk_running_flag(unsigned char fw_tsk_running_flag)
{
	VPQ_fw_tsk_running_flag = fw_tsk_running_flag;
}
