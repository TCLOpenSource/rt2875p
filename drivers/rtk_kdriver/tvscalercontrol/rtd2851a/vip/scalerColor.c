#ifndef BUILD_QUICK_SHOW
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/types.h>

#include <linux/module.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/random.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <asm/cacheflush.h>
#include <linux/pageremap.h>
#include <linux/kthread.h>
#include <rtk_kdriver/io.h>
#else
#include <no_os/printk.h>
#include <no_os/semaphore.h>
#include <no_os/slab.h>
#include <include/string.h>
#include <timer.h>
#include <tvscalercontrol/hdmirx/hdmi_vfe.h>
#include <sysdefs.h>
#include <qs_pq_setting.h>
#endif







#ifdef CONFIG_ARM64
#include <asm/io.h>
#endif


/*some include about AP*/
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/scaler/scalercolorlib_tv002.h>
#include <tvscalercontrol/scaler/scalercolorlib_tv010.h>
#include <tvscalercontrol/scaler/scalertimer.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <scaler_vpqmemcdev.h>
#include <tvscalercontrol/i3ddma/i3ddma_drv.h>
#include <tvscalercontrol/i3ddma/i3ddma.h>
/*some include about VIP Driver*/
#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/vip/di_ma.h>
#include <tvscalercontrol/vip/peaking.h>
#include <tvscalercontrol/vip/icm.h>
#include <tvscalercontrol/vip/dcc.h>
#include <tvscalercontrol/vip/ultrazoom.h>
#include <tvscalercontrol/vip/color.h>
#include <tvscalercontrol/vip/xc.h>
#include <tvscalercontrol/vip/intra.h>
#include <tvscalercontrol/vip/nr.h>
#include <tvscalercontrol/vip/film.h>
#include <tvscalercontrol/vip/pq_adaptive.h>
#include <tvscalercontrol/vip/gibi_od.h>
#include <tvscalercontrol/vip/pcid.h>
#ifndef BUILD_QUICK_SHOW
#include <tvscalercontrol/vdc/video.h>
#endif
#include <tvscalercontrol/vip/localdimming.h>
#include <tvscalercontrol/vip/localcontrast.h>
#include <tvscalercontrol/vip/still_logo_detect.h>
#include <tvscalercontrol/vip/st2094.h>
#include <tvscalercontrol/vip/pcid.h>
//====================20161027 START=======================================================
//juwen, 0627, TC
#include <tvscalercontrol/vip/tc_hdr.h>
//====================20161027 END=========================================================

#ifndef BUILD_QUICK_SHOW
#include <rtk_kdriver/quick_show/quick_show.h>
#else
#include <rtk_kdriver/rmm/rmm.h>
#endif


#include <tvscalercontrol/vip/viptable.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/memc_mux.h>
/*some include about scaler*/
#include <scaler/scalerDrvCommon.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/panel/panelapi.h>
/*some include about rbus*/
#include <rbus/vdtop_reg.h>
#include <rbus/ldspi_reg.h>
#include <rbus/pinmux_lvdsphy_reg.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/epi_reg.h>
#ifdef CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
#include <rbus/osdovl_reg.h>
#endif //CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE

/*some include about device driver*/
#include <tvscalercontrol/hdmirx/hdmifun.h>
#include <tvscalercontrol/hdmirx/hdmi_vfe.h>
#include <scaler_vscdev.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>
#include "tvscalercontrol/scalerdrv/scalermemory.h"
#include <tvscalercontrol/vip/ai_pq.h>

#ifndef BUILD_QUICK_SHOW
#include <mach/rtk_platform.h>
#endif
#include "rtk_vip_logger.h"
#include <rtd_log/rtd_module_log.h>


#undef VIPprintf
#ifdef CONFIG_CUSTOMER_TV030
#define VIPprintf(fmt, args...)
#else
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG, VIP_LOGMODULE_VIP_COLOR_scaler_DEBUG, fmt, ##args)
#endif

//#ifndef BUILD_QUICK_SHOW
#define	vip_malloc(x)	kmalloc(x, GFP_KERNEL)
#define	vip_free(x)	kfree(x)
//#endif

struct semaphore ICM_Semaphore;
struct semaphore I_RGB2YUV_Semaphore;
struct semaphore Gamma_Semaphore;

_system_setting_info		 	*system_info_structure_table=NULL;
_RPC_system_setting_info		 	*RPC_system_info_structure_table=NULL;
SLR_VIP_TABLE		             *g_Share_Memory_VIP_TABLE_Struct_isr= NULL;
_clues *SmartPic_clue=NULL;
_RPC_clues *RPC_SmartPic_clue=NULL;
VIP_table_crc_value *vip_table_crc_isr = NULL;

/*#define OSD_Contrast_Compensation 1*/
unsigned char g_bUseMiddleWareOSDmap = 0; //for Driver Base MiddleWare OSD map
StructColorDataFacModeType g_curColorFacTableEx = {0}; //for Driver Base MiddleWare OSD map

extern unsigned int	 *dbDCCHisto_TABLE;
extern unsigned int	 *dbDCCCurve_TABLE;
/*#define complement2(arg) (((~arg)&0x7FF) + 1)*/

extern BOOL Low_ConBri_BrightnessBoost_Disable;

#ifndef UT_flag
static SLR_VIP_TABLE *gVip_Table = NULL;
#endif// end of UT_flag

RPC_DCC_LAYER_Array_Struct *g_DCC_LAYER_Array_Struct = NULL;
_clues *g_SmartPic_clue = NULL;
SLR_VIP_TABLE *g_Share_Memory_VIP_TABLE_Struct = NULL;
RPC_SLR_VIP_TABLE *g_RPC_Share_Memory_VIP_TABLE_Struct = NULL;
/* === checksum ========== */
VIP_table_crc_value vip_table_crc = {0};
/* =========    Gamma  =============*/
#define Vmax_Gamma 4095
#define Vmax_invGamma 16383
#define gamma_bit_depth 12;
#define inv_gamma_bit_depth 14;
unsigned int *tGAMMA_temp_R;
unsigned int *tGAMMA_temp_G;
unsigned int *tGAMMA_temp_B;
unsigned char g_GammaMode = 0;
UINT16 GOut_R[Bin_Num_Gamma + 1], GOut_G[Bin_Num_Gamma + 1], GOut_B[Bin_Num_Gamma + 1];
UINT16 gamma_scale = (Vmax_Gamma + 1) / Bin_Num_Gamma;

unsigned char g_nSD_HD_mode = 0;

unsigned int final_GAMMA_R[Bin_Num_Gamma / 2], final_GAMMA_G[Bin_Num_Gamma / 2], final_GAMMA_B[Bin_Num_Gamma / 2]; // Num of index: Mac2=128, Sirius=512 (jyyang_2013/12/30)
unsigned int final_OutGAMMA_R[Bin_Num_Output_InvOutput_Gamma / 2], final_OutGAMMA_G[Bin_Num_Output_InvOutput_Gamma / 2], final_OutGAMMA_B[Bin_Num_Output_InvOutput_Gamma / 2];
unsigned int isr_write_gamma_SN = 0;
unsigned int vip_8vertex_temp[VIP_D_3D_LUT_UI_TBL_SIZE];

extern VIP_Output_InvOutput_Gamma Output_InvOutput_Gamma[Output_InvOutput_Gamma_TBL_MAX];
/* =========    Gamma  =============*/

/* == == == ==Inv   Gamma == == == == == == =*/
extern unsigned short Power22InvGamma[1025];
extern unsigned short LinearInvGamma[1025];
unsigned int final_invGAMMA_R[Bin_Num_Gamma / 2], final_invGAMMA_G[Bin_Num_Gamma / 2], final_invGAMMA_B[Bin_Num_Gamma / 2]; // Num of index: Mac2=128, Sirius=512 (jyyang_2013/12/30)
/* == == == ==Inv    Gamma == == == == == == =*/

/*========== xvYcc ============*/
extern unsigned int t_inv_GAMMA[512];
extern unsigned int t_gamma_curve_22[1025];
/*============================*/

#if 1
/*========================= for INNX demura =================================*/
#if 0	/* use dynamic alloc*/
char INNX_Demura_DeLut1[INNX_Demura_h_271][INNX_Demura_W16]={0};
char INNX_Demura_DeLut2[INNX_Demura_h_271][INNX_Demura_W16]={0};
char INNX_Demura_DeLut3[INNX_Demura_h_271][INNX_Demura_W16]={0};
char INNX_Demura_DeLut4[INNX_Demura_h_271][INNX_Demura_W16]={0};
char INNX_Demura_DeLut5[INNX_Demura_h_271][INNX_Demura_W16]={0};
static unsigned char INNX_Demura_CodeLutALL_row[31][22]={0};
static unsigned char INNX_Demura_CodeLutAll[31*INNX_Demura_h_271][22]={0};
static unsigned char INNX_Demura_header_infor[INNX_Demura_size_header_infor];
static unsigned char INNX_Demura_DataByRow[22*5*31*271];
#else
unsigned int INNX_Demura_gray_setting[7];

#ifndef UT_flag
static unsigned char *INNX_Demura_CodeLutALL_row;//[INNX_Demura_num_31][INNX_Demura_num_22]={0};
static unsigned char *INNX_Demura_CodeLutAll;//[INNX_Demura_num_31*INNX_Demura_h_271][INNX_Demura_num_22]={0};
static unsigned char *INNX_Demura_header_infor;//[INNX_Demura_size_header_infor];
static unsigned char *INNX_Demura_DataByRow;//[INNX_Demura_num_22*INNX_Demura_num_level*INNX_Demura_num_31*INNX_Demura_h_271];
#endif// end of UT_flag

#endif
/*========================= for INNX demura =================================*/
#endif

#ifdef CONFIG_HDR_SDR_SEAMLESS
HDR_SDR_SEAMLESS_PQ_STRUCT HDR_SDR_SEAMLESS_PQ = {0};
#endif

//extern DRV_Local_Contrast_Table Local_Contrast_Table[LC_Table_NUM]; //move to vip table

//====================20161027 START=======================================================
#if 0	/* juwen, move to "tc_hdr", elieli */
#define TCHDR_COEF_C_LUT_TBL_SIZE 33
#define TCHDR_COEF_I_LUT_TBL_SIZE 33
#define TCHDR_COEF_S_LUT_TBL_SIZE 33
#define TCHDR_COEF_P_LUT_TBL_SIZE 33
#define TCHDR_COEF_D_LUT_RED_TBL_SIZE 33
#define TCHDR_COEF_D_LUT_GREEN_TBL_SIZE 33
#define TCHDR_COEF_D_LUT_BLUE_TBL_SIZE 33
unsigned int TCHDR_COEF_C_LUT_TBL_ReadReg[TCHDR_COEF_C_LUT_TBL_SIZE];
unsigned int TCHDR_COEF_I_LUT_TBL_ReadReg[TCHDR_COEF_I_LUT_TBL_SIZE];
unsigned int TCHDR_COEF_S_LUT_TBL_ReadReg[TCHDR_COEF_S_LUT_TBL_SIZE];
unsigned int TCHDR_COEF_P_LUT_TBL_ReadReg[TCHDR_COEF_P_LUT_TBL_SIZE];
unsigned int TCHDR_COEF_D_LUT_RED_TBL_ReadReg[TCHDR_COEF_D_LUT_RED_TBL_SIZE];
unsigned int TCHDR_COEF_D_LUT_GREEN_TBL_ReadReg[TCHDR_COEF_D_LUT_GREEN_TBL_SIZE];
unsigned int TCHDR_COEF_D_LUT_BLUE_TBL_ReadReg[TCHDR_COEF_D_LUT_BLUE_TBL_SIZE];
#else
extern unsigned int TCHDR_COEF_C_LUT_TBL_ReadReg[TCHDR_COEF_C_LUT_TBL_SIZE];
extern unsigned int TCHDR_COEF_I_LUT_TBL_ReadReg[TCHDR_COEF_I_LUT_TBL_SIZE];
extern unsigned int TCHDR_COEF_S_LUT_TBL_ReadReg[TCHDR_COEF_S_LUT_TBL_SIZE];
extern unsigned int TCHDR_COEF_P_LUT_TBL_ReadReg[TCHDR_COEF_P_LUT_TBL_SIZE];
extern unsigned int TCHDR_COEF_D_LUT_RED_TBL_ReadReg[TCHDR_COEF_D_LUT_RED_TBL_SIZE];
extern unsigned int TCHDR_COEF_D_LUT_GREEN_TBL_ReadReg[TCHDR_COEF_D_LUT_GREEN_TBL_SIZE];
extern unsigned int TCHDR_COEF_D_LUT_BLUE_TBL_ReadReg[TCHDR_COEF_D_LUT_BLUE_TBL_SIZE];
#endif
//====================20161027 END=========================================================


/*========== POD & PCID ============*/
unsigned int PCID_ValueTBL[81] = {0};
unsigned int POD_ValueTBL_1[81] = {0};
unsigned int POD_ValueTBL_2[81] = {0};
/*============================*/

/* =========    Dither =============*/
extern unsigned char tDITHER_SEQUENCE[VIP_DITHER_SEQUENCE_MAX][3][16];
extern unsigned char tDITHER_MAIN_LUT[VIP_DITHER_MAINTABLE_MAX][3][4][4] ;
extern unsigned char tDITHER_PANEL_LUT[VIP_DITHER_PANELTABLE_MAX][3][16][4] ;
extern unsigned char tDITHER_TEMPORAL[VIP_DITHER_TEMPORAL_MAX][16];
extern unsigned char tDDither_Coef[VIP_DYNAMIC_DITHERING_TABLE_MAX][15];
/*=========================*/

/* =========     ICM =============*/
/*current elem tab from vip table*/
COLORELEM_TAB_T icm_tab_elem_of_vip_table;
COLORELEM_TAB_T icm_tab_elem_write;

unsigned char g_ICM_pillar_update_trigger = 0;
unsigned short *g_tICM_ini_s_pillar = 0;
unsigned short *g_tICM_ini_i_pillar = 0;
unsigned short *g_tICM_ini_h_pillar = 0;
/* =========     ICM =============*/

/* ======== histogram ============*/
unsigned char bForceHist3DCtrl;
/* ======== histogram ============*/

/* ========= ColorMatrix ==========*/
#if defined(ENABLE_HDMI_FORCE_LIMIT_RANGE)
#define FORCE_DATA_RANGE 1
#elif defined(ENABLE_HDMI_FORCE_FULL_RANGE)
#define FORCE_DATA_RANGE 2
#else
#define FORCE_DATA_RANGE 0
#endif

/* ================DM HDR 3D LUT===================*/
unsigned int DM_HDR_3D_DRV_LUT[VIP_DM_HDR_3D_LUT_DRV_TBL_SIZE];
/*unsigned int DM_HDR_3D_UI_LUT[VIP_DM_HDR_3D_LUT_UI_TBL_SIZE];*/
/* ================DM HDR 3D LUT===================*/

/* ================DM HDR 3D LUT===================*/
unsigned int D_3D_DRV_LUT[VIP_D_3D_LUT_DRV_TBL_SIZE];
/*unsigned int DM_HDR_3D_UI_LUT[VIP_DM_HDR_3D_LUT_UI_TBL_SIZE];*/
/* ================DM HDR 3D LUT===================*/
extern unsigned short tYUV2RGB_COEF[YUV2RGB_TBL_SELECT_ITEM_MAX][tUV2RGB_COEF_Items_Max];
extern unsigned short tRGB2YUV_COEF[RGB2YUV_COEF_MATRIX_ITEMS_Max][RGB2YUV_COEF_MATRIX_ITEMS_Max];

extern unsigned char RGB2YUV_if_RGB_mode;
extern DRV_Dirsu_Table gVipDirsu_Table[MAX_DIRECTIONAL_WEIGHTING_LEVEL];
extern unsigned char ConBriMapping_Offset_Table[16][VIP_YUV2RGB_LEVEL_SELECT_MAX][2];
extern unsigned char AVBlackLevelMapping_Table[8][3][4];
extern unsigned char ATVBlackLevelMapping_Table[8][3][4];
/* ========= ColorMatrix ==========*/

extern DRV_VipNewDcti_auto_adjust tVipNewDcti_auto_adjust[DCTI_TABLE_LEVEL_MAX];
extern DRV_VipNewDDcti_Table gVipDNewDcti_Table[DCTI_TABLE_LEVEL_MAX];
extern DRV_VipNewIDcti_Table gVipINewDcti_Table[DCTI_TABLE_LEVEL_MAX];
extern DRV_D_vcti_t gVipVcti_Table[DCTI_TABLE_LEVEL_MAX];
extern DRV_D_vcti_lpf_t gVipVcti_lpf_Table[DCTI_TABLE_LEVEL_MAX];
extern DRV_VipUV_Delay_TOP_Table gVipUV_Delay_TOP_Table[]; //JZ add for CVBS NTSC

extern RPC_DCC_Advance_control_table Advance_control_table[Advance_control_table_MAX];
extern RPC_DCC_Curve_boundary_table Curve_boundary_table[Curve_boundary_table_MAX];
extern DRV_RPC_AutoMA_Flag S_RPC_AutoMA_Flag;

/*scaling down coef*/
extern DRV_ScalingDown_COEF_TAB ScaleDown_COEF_TAB;

/*profile coef*/
extern DRV_Vip_Profile_Table gVip_Profile_Table[PROFILE_TABLE_TOTAL];

/* vip pq bypass*/
extern VIP_PQ_ByPass_Struct PQ_ByPass_Struct[VIP_PQ_ByPass_TBL_Max];

/* for De XC */
extern VIP_I_De_XC_TBL I_De_XC_TBL[I_De_XC_TBL_Max];
/* for De Contour */
extern VIP_I_De_Contour_TBL I_De_Contour_TBL[I_De_Contour_TBL_Max];

/* MA I SNR and edge smooth*/
extern DRV_MA_SNR_IESM_Coef pq_misc_MA_SNR_IESM_TBL[MA_SNR_IESM_TBL_MAX];

/* PQ by pass*/
extern VIP_PQ_ByPass_Struct PQ_ByPass_Struct[VIP_PQ_ByPass_TBL_Max];

extern unsigned char vdc_power_status;

FILM_FW_ShareMem *g_Share_Memory_FILM_FW = NULL;
PQ_device_struct *g_Share_Memory_PQ_device_struct = NULL;
#ifndef BUILD_QUICK_SHOW

unsigned char bODInited = FALSE;
#endif

unsigned char bODTableLoaded = FALSE;
unsigned char bODPreEnable = FALSE;
unsigned int od_table_store[OD_table_length];
char od_table_mode_store = 0x7f;
extern int  rtice_Set_D_LUT_9to17(unsigned int *array);

#ifndef UT_flag
#ifndef BUILD_QUICK_SHOW

/*other memory Endian*/
void fwif_color_ChangeUINT32Endian_Copy(unsigned int *pwTemp, int nSize, unsigned int *pwCopyTemp, unsigned char convert_endian_flag)
{

	int i;
	if (pwTemp && pwCopyTemp) {
		for (i = 0; i < (nSize); i++)
			pwCopyTemp[i] = convert_endian_flag?Scaler_ChangeUINT32Endian(pwTemp[i]):pwTemp[i];

	} else {
		VIPprintf("Warring!! %s's pwTemp = %lx, size = %d\n", __FUNCTION__, (unsigned long)pwTemp, nSize);
	}
}
#endif
void fwif_color_ChangeINT32Endian_Copy(int *pwTemp, int nSize, int *pwCopyTemp, unsigned char convert_endian_flag)
{
	int i;
	if (pwTemp && pwCopyTemp) {

		for (i = 0; i < (nSize); i++)
			pwCopyTemp[i] = convert_endian_flag?Scaler_ChangeUINT32Endian(pwTemp[i]):pwTemp[i];

	} else {
		VIPprintf("Warring!! %s's pwTemp = %lx, size = %d\n", __FUNCTION__, (unsigned long)pwTemp, nSize);
	}
}

void fwif_color_ChangeUINT16Endian_Copy(unsigned short *pwTemp, int nSize, unsigned short *pwCopyTemp, unsigned char convert_endian_flag)
{
	int i;
	if (pwTemp && pwCopyTemp) {
		for (i = 0; i < (nSize); i++)
			pwCopyTemp[i] = convert_endian_flag?Scaler_ChangeUINT16Endian(pwTemp[i]):pwTemp[i];

	} else {
		VIPprintf("Warring!! %s's pwTemp = %lx, size = %d\n", __FUNCTION__, (unsigned long)pwTemp, nSize);
	}
}
void fwif_color_ChangeINT16Endian_Copy(short *pwTemp, int nSize, short *pwCopyTemp, unsigned char convert_endian_flag)
{
	int i;
	if (pwTemp && pwCopyTemp) {

		for (i = 0; i < (nSize); i++)
			pwCopyTemp[i] = convert_endian_flag?Scaler_ChangeUINT16Endian(pwTemp[i]):pwTemp[i];

	} else {
		VIPprintf("Warring!! %s's pwTemp = %lx, size = %d\n", __FUNCTION__, (unsigned long)pwTemp, nSize);
	}
}

/* same memory Endian*/
void fwif_color_ChangeUINT32Endian(unsigned int *pwTemp, int nSize, unsigned char convert_endian_flag)
{
	int i;
	if (pwTemp) {

		if(convert_endian_flag) {
			for (i = 0; i < (nSize); i++)
				pwTemp[i] = Scaler_ChangeUINT32Endian(pwTemp[i]);
		}

	} else {
		VIPprintf("Warring!! %s's pwTemp = %lx, size = %d\n", __FUNCTION__, (unsigned long)pwTemp, nSize);
	}
}
void fwif_color_ChangeINT32Endian(int *pwTemp, int nSize, unsigned char convert_endian_flag)
{
	int i;
	if (pwTemp) {

		if(convert_endian_flag) {
			for (i = 0; i < (nSize); i++)
				pwTemp[i] = Scaler_ChangeUINT32Endian(pwTemp[i]);
		}

	} else {
		VIPprintf("Warring!! %s's pwTemp = %lx, size = %d\n", __FUNCTION__, (unsigned long)pwTemp, nSize);
	}
}

void fwif_color_ChangeUINT16Endian(unsigned short *pwTemp, int nSize, unsigned char convert_endian_flag)
{
	int i;
	if (pwTemp) {
		if(convert_endian_flag) {
			for (i = 0; i < (nSize); i++)
				pwTemp[i] = Scaler_ChangeUINT16Endian(pwTemp[i]);
		}

	} else {
		VIPprintf("Warring!! %s's pwTemp = %lx, size = %d\n", __FUNCTION__, (unsigned long)pwTemp, nSize);
	}
}
void fwif_color_ChangeINT16Endian(short  *pwTemp, int nSize, unsigned char convert_endian_flag)
{
	int i;
	if (pwTemp) {
		if(convert_endian_flag) {
			for (i = 0; i < (nSize); i++)
				pwTemp[i] = Scaler_ChangeUINT16Endian(pwTemp[i]);
		}

	} else {
		VIPprintf("Warring!! %s's pwTemp = %lx, size = %d\n", __FUNCTION__, (unsigned long)pwTemp, nSize);
	}
}

unsigned int fwif_color_ChangeOneUINT32Endian(unsigned int pwTemp, unsigned char convert_endian_flag)
{
	if(convert_endian_flag) {
		return Scaler_ChangeUINT32Endian(pwTemp);
	} else {
		return pwTemp;
	}

}

unsigned short fwif_color_ChangeOneUINT16Endian(unsigned short pwTemp, unsigned char convert_endian_flag)
{
	if(convert_endian_flag) {
		return Scaler_ChangeUINT16Endian(pwTemp);
	} else {
		return pwTemp;
	}

}
//#endif	


unsigned char fwif_color_Send_RPC_VIP_Table(SLR_VIP_TABLE *pVIPtable)
{
	extern VIP_MA_ChromaError MA_Chroma_Error_Table[t_MA_Chroma_Error_MAX];
	int ret;
	unsigned long virAddr = 0;
	RPC_SLR_VIP_TABLE *pRPC_SLR_VIP_TABLE = NULL;
	VIP_MA_ChromaError *pMA_Chroma_Error_Table = MA_Chroma_Error_Table;

	virAddr = Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_TABLE_STRUCT);

	VIPprintf("sizeof SLR_VIP_TABLE = %zu !!\n", sizeof(SCALERIOC_VIP_RPC_TABLE_STRUCT));
	if (0 == virAddr) {
		VIPprintf("GetShareMemVirAddr SCALERIOC_VIP_RPC_TABLE_STRUCT fail !!!\n");
		return FALSE;
	}
	pRPC_SLR_VIP_TABLE = (RPC_SLR_VIP_TABLE*)virAddr;

#ifdef CONFIG_ARM64
	memcpy_toio(pRPC_SLR_VIP_TABLE->VIP_QUALITY_Coef, pVIPtable->VIP_QUALITY_Coef, sizeof(unsigned char)*(VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK));
	memcpy_toio(pRPC_SLR_VIP_TABLE->MA_Chroma_Error_Table, pMA_Chroma_Error_Table, (sizeof(RPC_VIP_MA_ChromaError)*t_MA_Chroma_Error_MAX));
#else
	memcpy(pRPC_SLR_VIP_TABLE->VIP_QUALITY_Coef, pVIPtable->VIP_QUALITY_Coef, sizeof(unsigned char)*(VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK));
	memcpy(pRPC_SLR_VIP_TABLE->MA_Chroma_Error_Table, pMA_Chroma_Error_Table, (sizeof(RPC_VIP_MA_ChromaError)*t_MA_Chroma_Error_MAX));
#endif

	g_RPC_Share_Memory_VIP_TABLE_Struct = pRPC_SLR_VIP_TABLE;
#ifndef BUILD_QUICK_SHOW
	ret = Scaler_SendRPC(SCALERIOC_VIP_RPC_TABLE_STRUCT, 0, 0);
	if (0 !=  ret) {
		VIPprintf("ret = %d, Get SCALERIOC_VIP_TABLE_STRUCT fail !!!\n", ret);
		return FALSE;
	}
#endif
	return TRUE;

}


unsigned char fwif_color_Send_VIP_Table(SLR_VIP_TABLE *pVIPtable)
{

	unsigned long virAddr = 0;
	unsigned long cluAddr = Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);
	unsigned long RPCcluAddr = Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_SMARTPIC_CLUS);


	virAddr = Scaler_GetShareMemVirAddr(SCALERIOC_VIP_TABLE_STRUCT);

	VIPprintf("sizeof SLR_VIP_TABLE = %zu !!\n", sizeof(SLR_VIP_TABLE));
	if (0 == virAddr) {
		VIPprintf("GetShareMemVirAddr SCALERIOC_VIP_TABLE_STRUCT fail !!!\n");
		return FALSE;
	}

#ifdef CONFIG_ARM64
	memcpy_toio((SLR_VIP_TABLE *)virAddr, pVIPtable, sizeof(SLR_VIP_TABLE));
#else
	memcpy((SLR_VIP_TABLE *)virAddr, pVIPtable, sizeof(SLR_VIP_TABLE));
#endif
	g_Share_Memory_VIP_TABLE_Struct = (SLR_VIP_TABLE *)virAddr;

	fwif_color_Share_Memory_Access_VIP_TABLE_Struct(virAddr);


	SmartPic_clue = 	((_clues *)cluAddr);
	RPC_SmartPic_clue = ((_RPC_clues *)RPCcluAddr);
	//initial DCC apply curve
	{
	int DCC_Identity[Curve_num_Max]=
	{0,  8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96,104,112,120,128,136,144,152,160,168,176,184,192,
	200,208,216,224,232,240,258,256,264,272,280,288,296,304,312,320,328,336,344,352,360,368,376,384,392,
	400,408,416,424,432,440,458,456,464,472,480,488,496,504,512,520,528,536,544,552,560,568,576,584,592,
	600,608,616,624,632,640,658,656,664,672,680,688,696,704,712,720,728,736,744,752,760,768,776,784,792,
	800,808,816,824,832,840,858,856,864,872,880,888,896,904,912,920,928,936,944,952,960,968,976,984,992,
	1000,1008,1016,1023};
	memcpy(RPC_SmartPic_clue->RPC_Adapt_DCC_Info.Apply_Curve, DCC_Identity, DCC_Curve_Node_MAX * sizeof(int));
	}

	return TRUE;
}

DRV_RPC_AutoMA_Flag *fwif_color_GetAutoMA_Struct(void)
{
	_system_setting_info *system_setting_info = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	return &(system_setting_info->S_RPC_AutoMA_Flag);
}

RPC_DCC_LAYER_Array_Struct *fwif_color_GetDCC_LAYER_Array_Struct(void)
{
	return g_DCC_LAYER_Array_Struct;
}
/*
RPC_DCC_FUNCTION_Enable* fwif_color_GetDCC_FUNCTION_Struct(void)
{
	return g_DCC_FUNCTION_Struct;
}
*/
RPC_ICM_Global_Ctrl *fwif_color_GetICM_Global_Ctrl_Struct(void)
{
	_system_setting_info *system_setting_info = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (system_setting_info != NULL)
		return &(system_setting_info->PQ_Setting_Info.S_RPC_ICM_Global_Ctrl);
	else
		return NULL;
}
SLR_VIP_TABLE *fwif_color_GetShare_Memory_VIP_TABLE_Struct(void)
{
	return g_Share_Memory_VIP_TABLE_Struct;
}

RPC_DCC_Advance_control_table *fwif_color_GetShare_Memory_DCC_Advance_control_Struct(void)
{
	_system_setting_info *system_setting_info = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	return &(system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[0]);
}

_clues *fwif_color_GetShare_Memory_SmartPic_clue(void)
{
	g_SmartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);
	return g_SmartPic_clue;
}

void fwif_color_set_VIP_RPC_system_info_structure(void)
{
	int ret;
	_RPC_system_setting_info *RPC_system_setting_info = (_RPC_system_setting_info *) Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if (RPC_system_setting_info == NULL)
		return;

	//RPC_system_setting_info->Project_ID = VIP_Project_ID_TV001;
	RPC_system_setting_info->SCPU_ISRIINFO_TO_VCPU.VGIP_ISR_IN_VCPU_flag = 1;

#ifdef CONFIG_ARM64
	memcpy_toio((unsigned char *)(&RPC_system_setting_info->ScalingDown_Setting_INFO.ScalingDown_COEF_TAB), (unsigned char *)&ScaleDown_COEF_TAB, sizeof(DRV_ScalingDown_COEF_TAB));
	memcpy_toio((unsigned char *)(&RPC_system_setting_info->I_De_XC_CTRL.De_XC_TBL[0]), (unsigned char*)&I_De_XC_TBL[0], sizeof(VIP_I_De_XC_TBL)*I_De_XC_TBL_Max);
	memcpy_toio((unsigned char *)(&RPC_system_setting_info->PQ_ByPass_Ctrl.PQ_ByPass_Struct[0]), (unsigned char*)&PQ_ByPass_Struct[0], sizeof(VIP_PQ_ByPass_Struct )*VIP_PQ_ByPass_TBL_Max);
#else
	memcpy((unsigned char *)(&RPC_system_setting_info->ScalingDown_Setting_INFO.ScalingDown_COEF_TAB), (unsigned char *)&ScaleDown_COEF_TAB, sizeof(DRV_ScalingDown_COEF_TAB));
	memcpy((unsigned char *)(&RPC_system_setting_info->I_De_XC_CTRL.De_XC_TBL[0]), (unsigned char*)&I_De_XC_TBL[0], sizeof(VIP_I_De_XC_TBL)*I_De_XC_TBL_Max);
	memcpy((unsigned char *)(&RPC_system_setting_info->PQ_ByPass_Ctrl.PQ_ByPass_Struct[0]), (unsigned char*)&PQ_ByPass_Struct[0], sizeof(VIP_PQ_ByPass_Struct )*VIP_PQ_ByPass_TBL_Max);
#endif
#ifndef BUILD_QUICK_SHOW

	ret = Scaler_SendRPC(SCALERIOC_VIP_RPC_system_info_structure, 0, 0);
	if (0 !=  ret) {
		VIPprintf("ret = %d, update SCALERIOC_VIP_RPC_system_info_structure to driver fail !!!\n", ret);
	}
#endif

	fwif_color_Set_RPC_system_info_structure_table((unsigned long)RPC_system_setting_info);

}




/*Init function : system info. from AP to video fw and include some pq_misc_table structure!!*/
void fwif_color_set_VIP_system_info_structure(void)/*init by AP, this function used for video fw init sharemeory address*/
{
	//int ret, i;
	_system_setting_info *system_setting_info = (_system_setting_info *) Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if (system_setting_info == NULL)
		return;

	/*set PQ misc table to  share memory*/
#ifdef CONFIG_ARM64
	memcpy_toio((unsigned char *)(&system_setting_info->PQ_Setting_Info.I_D_Dcti_INFO.auto_adjust_Table[0]), (unsigned char*)&tVipNewDcti_auto_adjust[0], sizeof(DRV_VipNewDcti_auto_adjust)*DCTI_TABLE_LEVEL_MAX);
	memcpy_toio((unsigned char *)(&system_setting_info->PQ_Setting_Info.I_D_Dcti_INFO.iDcti_Table[0]), (unsigned char *)&gVipINewDcti_Table[0], sizeof(DRV_VipNewIDcti_Table)*DCTI_TABLE_LEVEL_MAX);
	memcpy_toio((unsigned char *)(&system_setting_info->PQ_Setting_Info.I_D_Dcti_INFO.dDcti_Table[0]), (unsigned char *)&gVipDNewDcti_Table[0], sizeof(DRV_VipNewDDcti_Table)*DCTI_TABLE_LEVEL_MAX);
	memcpy_toio((unsigned char *)(&system_setting_info->S_RPC_AutoMA_Flag), (unsigned char *)&S_RPC_AutoMA_Flag, sizeof(DRV_RPC_AutoMA_Flag));
	memcpy_toio((unsigned char *)(&system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[0]), (unsigned char *)&Advance_control_table[0], sizeof(RPC_DCC_Advance_control_table) * Advance_control_table_MAX);
	memcpy_toio((unsigned char *)(&system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Curve_boundary_table[0]), (unsigned char *)&Curve_boundary_table[0], sizeof(RPC_DCC_Curve_boundary_table) * Curve_boundary_table_MAX);
	memcpy_toio((unsigned char *)(&system_setting_info->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[0]), (unsigned char*)&gVip_Profile_Table[0], sizeof(DRV_Vip_Profile_Table)*PROFILE_TABLE_TOTAL);
	memcpy_toio((unsigned char *)(&system_setting_info->PQ_Setting_Info.sMA_SNR_IESM_Coef_CTRL.MA_SNR_IESM_TBL[0]), (unsigned char*)&pq_misc_MA_SNR_IESM_TBL[0], sizeof(DRV_MA_SNR_IESM_Coef)*MA_SNR_IESM_TBL_MAX);
	memset_io ((unsigned char*)&system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type_setFlag[0], 0, PQA_INPUT_TYPE_MAX*sizeof(char));
	memset_io ((unsigned char*)&system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item_setFlag[0], 0, PQA_INPUT_ITEM_MAX*sizeof(char));

#else
	memcpy((unsigned char *)(&system_setting_info->PQ_Setting_Info.I_D_Dcti_INFO.auto_adjust_Table[0]), (unsigned char*)&tVipNewDcti_auto_adjust[0], sizeof(DRV_VipNewDcti_auto_adjust)*DCTI_TABLE_LEVEL_MAX);
	memcpy((unsigned char *)(&system_setting_info->PQ_Setting_Info.I_D_Dcti_INFO.iDcti_Table[0]), (unsigned char *)&gVipINewDcti_Table[0], sizeof(DRV_VipNewIDcti_Table)*DCTI_TABLE_LEVEL_MAX);
	memcpy((unsigned char *)(&system_setting_info->PQ_Setting_Info.I_D_Dcti_INFO.dDcti_Table[0]), (unsigned char *)&gVipDNewDcti_Table[0], sizeof(DRV_VipNewDDcti_Table)*DCTI_TABLE_LEVEL_MAX);
	memcpy((unsigned char *)(&system_setting_info->S_RPC_AutoMA_Flag), (unsigned char *)&S_RPC_AutoMA_Flag, sizeof(DRV_RPC_AutoMA_Flag));
	memcpy((unsigned char *)(&system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Advance_control_table[0]), (unsigned char *)&Advance_control_table[0], sizeof(RPC_DCC_Advance_control_table) * Advance_control_table_MAX);
	memcpy((unsigned char *)(&system_setting_info->PQ_Setting_Info.DCC_info_coef.DCC_Curve_boundary_table[0]), (unsigned char *)&Curve_boundary_table[0], sizeof(RPC_DCC_Curve_boundary_table) * Curve_boundary_table_MAX);
	//RPC//memcpy((unsigned char *)(&system_setting_info->PQ_Setting_Info.ScalingDown_Setting_INFO.ScalingDown_COEF_TAB), (unsigned char *)&ScaleDown_COEF_TAB, sizeof(DRV_ScalingDown_COEF_TAB));

	memcpy((unsigned char *)(&system_setting_info->PQ_Setting_Info.Profile_Table_Coef.Vip_Profile_Table[0]), (unsigned char*)&gVip_Profile_Table[0], sizeof(DRV_Vip_Profile_Table)*PROFILE_TABLE_TOTAL);

	memcpy((unsigned char *)(&system_setting_info->PQ_Setting_Info.I_De_Contour_CTRL.De_Contour_TBL[0]), (unsigned char*)&I_De_Contour_TBL[0], sizeof(VIP_I_De_Contour_TBL)*I_De_Contour_TBL_Max);
	//RPC//memcpy((unsigned char *)(&system_setting_info->PQ_Setting_Info.I_De_XC_CTRL.De_XC_TBL[0]), (unsigned char*)&I_De_XC_TBL[0], sizeof(VIP_I_De_XC_TBL)*I_De_XC_TBL_Max);

	memcpy((unsigned char *)(&system_setting_info->PQ_Setting_Info.sMA_SNR_IESM_Coef_CTRL.MA_SNR_IESM_TBL[0]), (unsigned char*)&pq_misc_MA_SNR_IESM_TBL[0], sizeof(DRV_MA_SNR_IESM_Coef)*MA_SNR_IESM_TBL_MAX);
	//RPC//memcpy((unsigned char *)(&system_setting_info->PQ_Setting_Info.PQ_ByPass_Ctrl.PQ_ByPass_Struct[0]), (unsigned char*)&PQ_ByPass_Struct[0], sizeof(VIP_PQ_ByPass_Struct )*VIP_PQ_ByPass_TBL_Max);


	//tool ctrl, flow ctrl ini
	memset ((unsigned char*)&system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Type_setFlag[0], 0, PQA_INPUT_TYPE_MAX*sizeof(char));
	memset ((unsigned char*)&system_setting_info->PQ_Setting_Info.PQA_Setting_INFO.Flow_Ctrl_Input_Item_setFlag[0], 0, PQA_INPUT_ITEM_MAX*sizeof(char));
#endif

	fwif_color_Set_system_info_structure_table((unsigned long)system_setting_info);
	/* ini value, some demo platform have no AP, give OSD value*/
	system_setting_info->OSD_Info.Contrast_Gain = 128;
	system_setting_info->OSD_Info.Brightness_Gain = 128;
	system_setting_info->OSD_Info.Saturation_Gain = 128;
	system_setting_info->OSD_Info.Hue_Gain = 0;
	system_setting_info->OSD_Info.Saturation_Cmps = 128;
	system_setting_info->OSD_Info.Brightness_Cmps = 0;

/* //RPC//
	ret = Scaler_SendRPC(SCALERIOC_VIP_system_info_structure, 0, 0);
	if (0 !=  ret) {
		VIPprintf("ret = %d, update fwif_color_set_VIP_system_info_structure to driver fail !!!\n", ret);
	}
*/

}


/*
_system_setting_info* fwif_color_Get_VIP_system_info_structure(void)
{
	return ((_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure));
}
*/


/*===================================================================================== //DCC CSFC 20140127*/
/*==Set About DCC =====*/
#ifdef CONFIG_HW_SUPPORT_D_HISTOGRAM
void fwif_color_D_hist_init(unsigned char display, unsigned short width, unsigned short height)
{
	/*int ret = 0;*/
	d_histogram_d_ich1_hist_in_cselect_RBUS d_ich1_hist_in_cselect_Reg;
	d_histogram_d_ich1_hist_in_range_hor_RBUS d_ich1_hist_in_range_hor_Reg;
	d_histogram_d_ich1_hist_in_range_ver_RBUS d_ich1_hist_in_range_ver_Reg;
#if 0	/*#ifdef CONFIG_DUAL_CHANNEL*/
	d_histogram_d_ich2_hist_in_cselect_RBUS d_ich2_hist_in_cselect_Reg;
	d_histogram_d_ich2_hist_in_range_hor_RBUS d_ich2_hist_in_range_hor_Reg;
	d_histogram_d_ich2_hist_in_range_ver_RBUS d_ich2_hist_in_range_ver_Reg;
#endif

	/*	unsigned short ret;*/
	unsigned char shift_bits = 0,BinMode = 0;
	unsigned int total_pixel;
	static unsigned int Main_total_pixel;/*, Sub_total_pixel = 0;*/

	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = NULL;
	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if (display == _MAIN_DISPLAY) {
		d_ich1_hist_in_cselect_Reg.regValue = IoReg_Read32(D_HISTOGRAM_D_ICH1_Hist_IN_CSELECT_reg);
		d_ich1_hist_in_cselect_Reg.ch1_his_enable_histog = 0;
		d_ich1_hist_in_cselect_Reg.ch1_his_irq_en = 0;

	/*o----Check & Modify Histogram Bin Mode, avoid setting error. 20160624 from Merlin2 -----o*/
		if( COLOR_D_HISTOGRAM_LEVEL == 32 ) { //video fw bin detector = 32 bin
			d_ich1_hist_in_cselect_Reg.bin_mode = 0;
			BinMode = Mode_Orig_32Bin;
			VIPprintf("D_Hist BinMode -> Mode_Orig_32Bin \n");
		} else { //video fw bin detector = 128 bin
			d_ich1_hist_in_cselect_Reg.bin_mode = 1;
			BinMode = Mode_128Bin;
			VIPprintf("D_Hist BinMode -> Mode_128Bin \n");
		}

		/*~20120816 fix 3d en DCR*/
		IoReg_Write32(D_HISTOGRAM_D_ICH1_Hist_IN_CSELECT_reg, d_ich1_hist_in_cselect_Reg.regValue);

		Main_total_pixel = (width*height) + 1;
		total_pixel = Main_total_pixel;
		VIPprintf("\n$$$$[DCC] width = %d, height = %d, total_pixel = %d\n", width, height, total_pixel);

		if (total_pixel <=  0x10000) { /*(2^16)*/
			shift_bits = 0;
		} else if (total_pixel > 0x10000 && total_pixel <=  0x20000) {/*(2^16) ~ (2^17)*/
			shift_bits = 0;
		} else if (total_pixel > 0x20000 && total_pixel <=  0x40000) {/*(2^17) ~ (2^18)*/
			shift_bits = 1;
		} else if (total_pixel > 0x40000 && total_pixel <=  0x80000) {/*(2^18) ~ (2^19)*/
			shift_bits = 2;
		} else if (total_pixel > 0x80000 && total_pixel <=  0x100000) {/*(2^19) ~ (2^20)*/
			shift_bits = 3;
		} else if (total_pixel > 0x100000 && total_pixel <=  0x200000) {/*(2^20) ~ (2^21)*/
			shift_bits = 4;
		} else if (total_pixel > 0x200000 && total_pixel <=  0x400000) {/*(2^21) ~ (2^22)*/
			shift_bits = 5;
		} else if ((total_pixel > 0x400000 && total_pixel <=  0x800000)) {/*(2^22) ~ (2^23)*/
			shift_bits = 6;
		} else {
			shift_bits = 7;
		}

		/*=== 20140512 CSFC for vip video fw infomation ====*/
		VIP_RPC_system_info_structure_table->DCC_D_hist_shift_bit = shift_bits;
		VIP_RPC_system_info_structure_table->D_Histogram_Bin_Mode = BinMode;
		/*====================================*/

		/*Histogram init*/
		//NOTE: if DTG, Pip_swap_en = 1(b8028004[29]), D_histogram range follow pip_mixer setting and range_hor and range_ver no-used!
		d_ich1_hist_in_range_hor_Reg.regValue = IoReg_Read32(D_HISTOGRAM_D_ICH1_Hist_IN_Range_Hor_reg);
		d_ich1_hist_in_range_hor_Reg.ch1_his_horend = width - 1;
		d_ich1_hist_in_range_hor_Reg.ch1_his_horstart = 0;
		IoReg_Write32(D_HISTOGRAM_D_ICH1_Hist_IN_Range_Hor_reg, d_ich1_hist_in_range_hor_Reg.regValue);
		d_ich1_hist_in_range_ver_Reg.regValue = IoReg_Read32(D_HISTOGRAM_D_ICH1_Hist_IN_Range_Ver_reg);
		d_ich1_hist_in_range_ver_Reg.ch1_his_verend = height - 1;
		d_ich1_hist_in_range_ver_Reg.ch1_his_verstart = 0;
		IoReg_Write32(D_HISTOGRAM_D_ICH1_Hist_IN_Range_Ver_reg, d_ich1_hist_in_range_ver_Reg.regValue);
		d_ich1_hist_in_cselect_Reg.regValue = IoReg_Read32(D_HISTOGRAM_D_ICH1_Hist_IN_CSELECT_reg);
		d_ich1_hist_in_cselect_Reg.ch1_his_shift_sel = shift_bits;
		d_ich1_hist_in_cselect_Reg.ch1_his_mode_sel = 1;
		d_ich1_hist_in_cselect_Reg.ch1_his_num_flds = 0;
		d_ich1_hist_in_cselect_Reg.ch1_his_enable_histog = 1;
		IoReg_Write32(D_HISTOGRAM_D_ICH1_Hist_IN_CSELECT_reg, d_ich1_hist_in_cselect_Reg.regValue);
		/*o-------------------------------------------------------------------------o*/
	}
#if 0	/*#ifdef CONFIG_DUAL_CHANNEL*/
	else {
		d_ich2_hist_in_cselect_Reg.regValue = IoReg_Read32(D_HISTOGRAM_D_ICH2_Hist_IN_CSELECT_reg);
		d_ich2_hist_in_cselect_Reg.ch2_his_enable_histog = 0;
		IoReg_Write32(D_HISTOGRAM_D_ICH2_Hist_IN_CSELECT_reg, d_ich2_hist_in_cselect_Reg.regValue);

		Sub_total_pixel = width*height;
		total_pixel = Sub_total_pixel;

		if (total_pixel <=  0x10000)
			shift_bits = 0;
		else if (total_pixel > 0x10000 && total_pixel <=  0x20000)
			shift_bits = 1;
		else if (total_pixel > 0x20000 && total_pixel <=  0x40000)
			shift_bits = 2;
		else if (total_pixel > 0x40000 && total_pixel <=  0x80000)
			shift_bits = 3;
		else if (total_pixel > 0x80000 && total_pixel <=  0x100000)
			shift_bits = 4;
		else
			shift_bits = 5;

		/*Histogram init*/
		d_ich2_hist_in_range_hor_Reg.regValue = IoReg_Read32(D_HISTOGRAM_D_ICH2_Hist_IN_Range_Hor_reg);
		d_ich2_hist_in_range_hor_Reg.ch2_his_horend = width - 1;
		d_ich2_hist_in_range_hor_Reg.ch2_his_horstart = 0;
		IoReg_Write32(D_HISTOGRAM_D_ICH2_Hist_IN_Range_Hor_reg, d_ich2_hist_in_range_hor_Reg.regValue);
		d_ich2_hist_in_range_ver_Reg.regValue = IoReg_Read32(D_HISTOGRAM_D_ICH2_Hist_IN_Range_Ver_reg);
		d_ich2_hist_in_range_ver_Reg.ch2_his_verend = height - 1;
		d_ich2_hist_in_range_ver_Reg.ch2_his_verstart = 0;
		IoReg_Write32(D_HISTOGRAM_D_ICH2_Hist_IN_Range_Ver_reg, d_ich2_hist_in_range_ver_Reg.regValue);
		d_ich2_hist_in_cselect_Reg.regValue = IoReg_Read32(D_HISTOGRAM_D_ICH2_Hist_IN_CSELECT_reg);
	  	d_ich2_hist_in_cselect_Reg.ch2_his_even_odd_pos_sel = 0;/*after scale down*/
		d_ich2_hist_in_cselect_Reg.ch2_his_shift_sel = shift_bits;
		d_ich2_hist_in_cselect_Reg.ch2_his_mode_sel = 1;
		d_ich2_hist_in_cselect_Reg.ch2_his_num_flds = 0;
		d_ich2_hist_in_cselect_Reg.ch2_his_enable_histog = 1;
		IoReg_Write32(D_HISTOGRAM_D_ICH2_Hist_IN_CSELECT_reg, d_ich2_hist_in_cselect_Reg.regValue);
	}
#endif

}
#endif //CONFIG_HW_SUPPORT_D_HISTOGRAM
void fwif_color_hist_init(unsigned char display, unsigned short width, unsigned short height, Histogram_BIN_MODE BinMode)/*TO DO : Change to Table Base Driver!! CSFC, 20140822*/
{
	/*int ret = 0;*/
	histogram_ich1_hist_in_cselect_RBUS ich1_hist_in_cselect_Reg;
	histogram_ich1_hist_in_range_hor_RBUS ich1_hist_in_range_hor_Reg;
	histogram_ich1_hist_in_range_ver_RBUS ich1_hist_in_range_ver_Reg;
#if 0	/*#ifdef CONFIG_DUAL_CHANNEL*/
	ich2_hist_in_cselect_RBUS ich2_hist_in_cselect_Reg;
	ich2_hist_in_range_hor_RBUS ich2_hist_in_range_hor_Reg;
	ich2_hist_in_range_ver_RBUS ich2_hist_in_range_ver_Reg;
#endif

	/*	unsigned short ret;*/
	unsigned char shift_bits;
	unsigned int total_pixel;
	static unsigned int Main_total_pixel;/*, Sub_total_pixel = 0;*/

	_system_setting_info *VIP_system_info_structure_table = NULL;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if (display == _MAIN_DISPLAY) {
		ich1_hist_in_cselect_Reg.regValue = IoReg_Read32(HISTOGRAM_ICH1_Hist_IN_CSELECT_reg);
		ich1_hist_in_cselect_Reg.ch1_his_enable_histog = 0;

	/*o----Check & Modify Histogram Bin Mode, avoid setting error. 20160624 from Merlin2 -----o*/
		if( COLOR_HISTOGRAM_LEVEL == 32 ) { //video fw bin detector = 32 bin
			ich1_hist_in_cselect_Reg.bin_mode = 0;
			if( BinMode == Mode_128to32Bin ) {
				VIPprintf("Warning!!! Hist BinMode setting missmatch!!\n");
				VIPprintf("Bin_Mode change from Mode_128to32Bin to Mode_Orig_32Bin \n");
				BinMode = Mode_Orig_32Bin;
			} else if( BinMode == Mode_128Bin ) {
				VIPprintf("Warning!!! Hist BinMode setting missmatch!!\n");
				VIPprintf("Bin_Mode change from Mode_128Bin to Mode_Orig_32Bin \n");
				BinMode = Mode_Orig_32Bin;
			} else {
				VIPprintf("Hist BinMode -> Mode_Orig_32Bin \n");
			}
		} else { //video fw bin detector = 128 bin
			ich1_hist_in_cselect_Reg.bin_mode = 1;
			if( BinMode == Mode_Orig_32Bin ) {
				VIPprintf("Warning!!! Hist BinMode setting missmatch!!\n");
				VIPprintf("Bin_Mode change from Mode_Orig_32Bin to Mode_128to32Bin \n");
				BinMode = Mode_128to32Bin;
			} else if (BinMode == Mode_128to32Bin ){
				VIPprintf("Hist BinMode -> Mode_128to32Bin \n");
			} else {
				VIPprintf("Hist BinMode -> Mode_128Bin \n");
			}
		}

		/*20120816 fix 3d en DCR*/
		if (Scaler_Disp3dGetInfo((SCALER_DISP_CHANNEL)display, SLR_DISP_3D_3DTYPE) == SLR_3D_FRAMESEQUENCE) {
			VIPprintf("fwif_color_hist_init, 3d_en\n");
			ich1_hist_in_cselect_Reg.ch1_3d_en = 1;
		} else {
			ich1_hist_in_cselect_Reg.ch1_3d_en = 0;
		}
		if (bForceHist3DCtrl)
			ich1_hist_in_cselect_Reg.ch1_3d_en = 1;

		/*~20120816 fix 3d en DCR*/
		IoReg_Write32(HISTOGRAM_ICH1_Hist_IN_CSELECT_reg, ich1_hist_in_cselect_Reg.regValue);

		Main_total_pixel = (width*height) + 1;
		total_pixel = Main_total_pixel;
		VIPprintf("\n$$$$[DCC] width = %d, height = %d, total_pixel = %d\n", width, height, total_pixel);

		if (total_pixel <=  0x10000) { /*(2^16)*/
			shift_bits = 0;
		} else if (total_pixel > 0x10000 && total_pixel <=  0x20000) {/*(2^16) ~ (2^17)*/
			shift_bits = 0;
		} else if (total_pixel > 0x20000 && total_pixel <=  0x40000) {/*(2^17) ~ (2^18)*/
			shift_bits = 1;
		} else if (total_pixel > 0x40000 && total_pixel <=  0x80000) {/*(2^18) ~ (2^19)*/
			shift_bits = 2;
		} else if (total_pixel > 0x80000 && total_pixel <=  0x100000) {/*(2^19) ~ (2^20)*/
			shift_bits = 3;
		} else if (total_pixel > 0x100000 && total_pixel <=  0x200000) {/*(2^20) ~ (2^21)*/
			shift_bits = 4;
		} else if (total_pixel > 0x200000 && total_pixel <=  0x400000) {/*(2^21) ~ (2^22)*/
			shift_bits = 5;
		} else if ((total_pixel > 0x400000 && total_pixel <=  0x800000)) {/*(2^22) ~ (2^23)*/
			shift_bits = 6;
		} else {
			shift_bits = 7;
		}

		/*=== 20140512 CSFC for vip video fw infomation ====*/
		if (VIP_system_info_structure_table == NULL || VIP_RPC_system_info_structure_table == NULL) {
			VIPprintf("ERROR, VIP_system_info_structure_table == NULL !!\n");
		} else {
			VIP_RPC_system_info_structure_table->DCC_hist_shift_bit = shift_bits;
			VIP_system_info_structure_table->Histogram_Bin_Mode = BinMode;
		}
		/*====================================*/

		/*Histogram init*/
		ich1_hist_in_range_hor_Reg.regValue = IoReg_Read32(HISTOGRAM_ICH1_Hist_IN_Range_Hor_reg);
		ich1_hist_in_range_hor_Reg.ch1_his_horend = width - 1;
		ich1_hist_in_range_hor_Reg.ch1_his_horstart = 0;
		IoReg_Write32(HISTOGRAM_ICH1_Hist_IN_Range_Hor_reg, ich1_hist_in_range_hor_Reg.regValue);
		ich1_hist_in_range_ver_Reg.regValue = IoReg_Read32(HISTOGRAM_ICH1_Hist_IN_Range_Ver_reg);
		ich1_hist_in_range_ver_Reg.ch1_his_verend = height - 1;
		ich1_hist_in_range_ver_Reg.ch1_his_verstart = 0;
		IoReg_Write32(HISTOGRAM_ICH1_Hist_IN_Range_Ver_reg, ich1_hist_in_range_ver_Reg.regValue);
		ich1_hist_in_cselect_Reg.regValue = IoReg_Read32(HISTOGRAM_ICH1_Hist_IN_CSELECT_reg);
	  ich1_hist_in_cselect_Reg.ch1_his_even_odd_pos_sel = 0;/*after scale down*/
		ich1_hist_in_cselect_Reg.ch1_his_shift_sel = shift_bits;
		ich1_hist_in_cselect_Reg.ch1_his_mode_sel = 1;
		ich1_hist_in_cselect_Reg.ch1_his_num_flds = 0;
		ich1_hist_in_cselect_Reg.ch1_his_enable_histog = 1;
		IoReg_Write32(HISTOGRAM_ICH1_Hist_IN_CSELECT_reg, ich1_hist_in_cselect_Reg.regValue);
		/*o-------------------------------------------------------------------------o*/
	}
#if 0	/*#ifdef CONFIG_DUAL_CHANNEL*/
	else {
		ich2_hist_in_cselect_Reg.regValue = IoReg_Read32(HISTOGRAM_ICH2_HIST_IN_CSELECT_VADDR);
		ich2_hist_in_cselect_Reg.ch2_his_enable_histog = 0;
		IoReg_Write32(HISTOGRAM_ICH2_HIST_IN_CSELECT_VADDR, ich2_hist_in_cselect_Reg.regValue);

		Sub_total_pixel = width*height;
		total_pixel = Sub_total_pixel;

		if (total_pixel <=  0x10000)
			shift_bits = 0;
		else if (total_pixel > 0x10000 && total_pixel <=  0x20000)
			shift_bits = 1;
		else if (total_pixel > 0x20000 && total_pixel <=  0x40000)
			shift_bits = 2;
		else if (total_pixel > 0x40000 && total_pixel <=  0x80000)
			shift_bits = 3;
		else if (total_pixel > 0x80000 && total_pixel <=  0x100000)
			shift_bits = 4;
		else
			shift_bits = 5;

		/*Histogram init*/
		ich2_hist_in_range_hor_Reg.regValue = IoReg_Read32(HISTOGRAM_ICH2_HIST_IN_RANGE_HOR_VADDR);
		ich2_hist_in_range_hor_Reg.ch2_his_horend = width - 1;
		ich2_hist_in_range_hor_Reg.ch2_his_horstart = 0;
		IoReg_Write32(HISTOGRAM_ICH2_HIST_IN_RANGE_HOR_VADDR, ich2_hist_in_range_hor_Reg.regValue);
		ich2_hist_in_range_ver_Reg.regValue = IoReg_Read32(HISTOGRAM_ICH2_HIST_IN_RANGE_VER_VADDR);
		ich2_hist_in_range_ver_Reg.ch2_his_verend = height - 1;
		ich2_hist_in_range_ver_Reg.ch2_his_verstart = 0;
		IoReg_Write32(HISTOGRAM_ICH2_HIST_IN_RANGE_VER_VADDR, ich2_hist_in_range_ver_Reg.regValue);
		ich2_hist_in_cselect_Reg.regValue = IoReg_Read32(HISTOGRAM_ICH2_HIST_IN_CSELECT_VADDR);
	  ich2_hist_in_cselect_Reg.ch2_his_even_odd_pos_sel = 0;/*after scale down*/
		ich2_hist_in_cselect_Reg.ch2_his_shift_sel = shift_bits;
		ich2_hist_in_cselect_Reg.ch2_his_mode_sel = 1;
		ich2_hist_in_cselect_Reg.ch2_his_num_flds = 0;
		ich2_hist_in_cselect_Reg.ch2_his_enable_histog = 1;
		IoReg_Write32(HISTOGRAM_ICH2_HIST_IN_CSELECT_VADDR, ich2_hist_in_cselect_Reg.regValue);
	}
#endif
}

void fwif_color_set_hist_3d_en(unsigned char bEnable)
{
	histogram_ich1_hist_in_cselect_RBUS ich1_hist_in_cselect_Reg;
	ich1_hist_in_cselect_Reg.regValue = IoReg_Read32(HISTOGRAM_ICH1_Hist_IN_CSELECT_reg);

	ich1_hist_in_cselect_Reg.ch1_3d_en = bEnable;
	IoReg_Write32(HISTOGRAM_ICH1_Hist_IN_CSELECT_reg, ich1_hist_in_cselect_Reg.regValue);
}

/*o-----DCC init Process-------o*/
void fwif_color_set_DCC_Init(unsigned char display)/*AP DCC block HW Setting init process*/
{
	drvif_color_dcc_Curve_init(display);

	drvif_color_set_dcc_chroma_compensation_Init(display);
	drvif_color_set_dcc_Chroma_enhance();
}

/*o------Picture mode setting-------o*/
void fwif_color_set_dcc_OnOff(unsigned char src_idx, VIP_DCC_Disable_Mode Mode)
{
	/*=== 20140421 CSFC for vip video fw infomation ====*/
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	/*====================================*/

	/*0:DCC_HWOFF_Bypass, 1:DCC_FWOFF_NZ, 2:DCC_ON*/
	switch (Mode) {
	case DCC_HWOFF_Bypass:
			drvif_color_dcc_on(_DISABLE);
			VIP_system_info_structure_table->DCC_FW_ONOFF = _ENABLE;
			break;
	case DCC_FWOFF_NZ:
			drvif_color_dcc_on(_ENABLE);
			VIP_system_info_structure_table->DCC_FW_ONOFF = _DISABLE;
			break;
	case DCC_ON:
			drvif_color_dcc_on(_ENABLE);
			VIP_system_info_structure_table->DCC_FW_ONOFF = _ENABLE;
			break;
	case DCC_ON_APPLY_AP_CURVE:
			drvif_color_dcc_on(_ENABLE);
			VIP_system_info_structure_table->DCC_FW_ONOFF = _ENABLE;
			break;
	default:
			drvif_color_dcc_on(_DISABLE);
			VIP_system_info_structure_table->DCC_FW_ONOFF = _DISABLE;
			break;
	}
}


/*Set DCC Mode : off/low/middle/high/default*/
void fwif_color_set_dcc_mode(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;

	VIPprintf(" VIPDCCTable_Setting fwif_color_set_dcc_mode  = %d \n", value);

	/*=== 20140421 CSFC for vip video fw infomation ====*/
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	VIP_system_info_structure_table->OSD_Info.OSD_DCC_Mode = value;
	/*====================================*/

	switch (value) {
	case SLR_DCC_OFF:/*default -> DCC_FWOFF_NZ*/
			fwif_color_set_dcc_OnOff(src_idx, DCC_FWOFF_NZ);/*0:DCC_HWOFF_Bypass, 1:DCC_FWOFF_NZ, 2:DCC_ON, */
			break;
	case SLR_DCC_LOW:
	case SLR_DCC_MID:
	case SLR_DCC_HIGH:
	case SLR_DCC_Default:
			fwif_color_set_dcc_OnOff(src_idx, DCC_ON);
			break;
	default:
			fwif_color_set_dcc_OnOff(src_idx, DCC_HWOFF_Bypass);
			break;
	}
}

/*o-------for VIP table DCC info sync. to video fw-------o*/
void fwif_color_set_DCC_Boundary_Check_Table(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.Boundary_Check_Table = value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/
}

void fwif_color_set_DCC_Level_and_Blend_Coef_Table(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.Level_and_Blend_Coef_Table = value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/
}


void fwif_color_set_DCC_hist_adjust_table(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.hist_adjust_table = value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/
}

void fwif_color_set_DCC_AdaptCtrl_Level_Table(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.AdaptCtrl_Level_Table = value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/
}

void fwif_color_set_DCC_User_Curve_Table(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.User_Curve_Table = value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/
}

void fwif_color_set_DCC_database_Table(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.database_DCC_Table = value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/
}

void fwif_color_set_DCC_Picture_Mode_Weight(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.Picture_Mode_Weight = value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/
}

void fwif_color_set_DCL_W(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.DCL_W_level = value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*==============================*/
}

void fwif_color_set_DCL_B(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.DCL_B_level = value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/
}


void fwif_color_set_DCC_W_expand(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.Expand_W_Level = value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/
}

void fwif_color_set_DCC_B_expand(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.Expand_B_Level = value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/
}

void fwif_color_set_DCC_Blending_DelayTime(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.Delay_time = value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/
}

void fwif_color_set_DCC_Blending_Step(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.Step = value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/
}

/*o------- for AP eng. menu level sync. -------o*/
void fwif_color_set_dcc_table_select(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.DCC_Table = value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/
}

#if 0
void fwif_color_set_dcc_level(unsigned char src_idx, unsigned char value, unsigned char SceneChange)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.DCC_Level = value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/
}

void fwif_color_set_DCC_Sindex(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.DCC_SCurve_index = value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/
}

void fwif_color_set_DCC_Shigh(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.DCC_SCurve_high = value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/
}

void fwif_color_set_DCC_Slow(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.DCC_SCurve_low = value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/
}
#endif

void fwif_color_set_DCC_SC_for_Coef_Change(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/
}

void fwif_color_set_DCC_histogram_gain(unsigned char src_idx, unsigned char value, unsigned char bin_num)
{

	unsigned char Main_DCC_Hist_adjust_table = 0;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	SLR_VIP_TABLE *prt = NULL;
	VIP_DCC_Hist_Factor_Table *g_DCCHist_Factor_Table = NULL;

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get _system_setting_info Error! return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	Main_DCC_Hist_adjust_table = VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.hist_adjust_table;

	prt = fwif_color_GetShare_Memory_VIP_TABLE_Struct();
	if (prt == NULL) {
		VIPprintf("~get share mem vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	g_DCCHist_Factor_Table = &(prt->DCC_Control_Structure.DCCHist_Factor_Table[Main_DCC_Hist_adjust_table]);

	g_DCCHist_Factor_Table->gain[bin_num] = value;
	/*=== 201408 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/


}

void fwif_color_set_DCC_histogram_offset(unsigned char src_idx, unsigned char value, unsigned char bin_num)
{

	unsigned char Main_DCC_Hist_adjust_table = 0;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	SLR_VIP_TABLE *prt = NULL;
	VIP_DCC_Hist_Factor_Table *g_DCCHist_Factor_Table = NULL;

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get _system_setting_info Error! return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	Main_DCC_Hist_adjust_table = VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.hist_adjust_table;

	prt = fwif_color_GetShare_Memory_VIP_TABLE_Struct();
	if (prt == NULL) {
		VIPprintf("~get share mem vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	g_DCCHist_Factor_Table = &(prt->DCC_Control_Structure.DCCHist_Factor_Table[Main_DCC_Hist_adjust_table]);

	g_DCCHist_Factor_Table->offset[bin_num] = value;
	/*=== 201408 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/

}

void fwif_color_set_DCC_histogram_limit(unsigned char src_idx, unsigned char value, unsigned char bin_num)
{
	unsigned char Main_DCC_Hist_adjust_table = 0;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	SLR_VIP_TABLE *prt = NULL;
	VIP_DCC_Hist_Factor_Table *g_DCCHist_Factor_Table = NULL;

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get _system_setting_info Error! return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	Main_DCC_Hist_adjust_table = VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.hist_adjust_table;

	prt = fwif_color_GetShare_Memory_VIP_TABLE_Struct();
	if (prt == NULL) {
		VIPprintf("~get share mem vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	g_DCCHist_Factor_Table = &(prt->DCC_Control_Structure.DCCHist_Factor_Table[Main_DCC_Hist_adjust_table]);

	g_DCCHist_Factor_Table->limit[bin_num] = value;
	/*=== 201408 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	/*===============================*/
}


UINT8 DEBUG_DCC_CURVE_SETECT = 0;
unsigned char fwif_color_Debug_Get_UserCurveDCC_Curve_Select(void)
{
	return DEBUG_DCC_CURVE_SETECT;
}

unsigned char fwif_color_Debug_Get_HIST_MEAN_VALUE(void)
{
	_clues *smartPic_clue;
	unsigned int pwCopyTemp = 0;
	smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);
	fwif_color_ChangeUINT32Endian_Copy(&smartPic_clue->Hist_Y_Mean_Value, 1, &pwCopyTemp, 0);
	return pwCopyTemp;
}

unsigned char fwif_color_Debug_Get_Pure_Color_Flag(void)
{
	_clues *smartPic_clue;
	smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);
	return smartPic_clue->PureColor;
}

unsigned char fwif_color_Debug_Get_Skin_Tone_Found_Flag(void)
{
	_clues *smartPic_clue;
	smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);
	return smartPic_clue->skintoneflag;
}

unsigned char fwif_color_Debug_Get_UserCurveDCC_Curve_Weighting(unsigned char table_select)
{
	unsigned char return_ret = 0;

	return return_ret;
}

unsigned char fwif_color_Debug_Get_UserCurveDCC_Curve_Weighting_Skin(unsigned char table_select)
{
	unsigned char return_ret = 0;

	return return_ret;
}

void fwif_color_Debug_Set_UserCurveDCC_Curve_Apply(unsigned char table_select, unsigned char value)
{


}

void fwif_color_Debug_Set_UserCurveDCC_Curve_Select(unsigned char value)
{
	DEBUG_DCC_CURVE_SETECT = value;
}

void fwif_color_Debug_Set_UserCurveDCC_Curve_Weighting(unsigned char table_select, unsigned char value)
{

}

void fwif_color_Debug_Set_UserCurveDCC_Curve_Weighting_Skin(unsigned char table_select, unsigned char value)
{

}


/*Set System Fw Layer Driver*/
void fwif_color_set_dcc_chroma_compensation_driver(unsigned char src_idx, unsigned char value)
{
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	VIP_DCC_Chroma_Compensation_TABLE *Chroma_Compensation_TABLE = NULL;
	//VIP_DCC_Chroma_Compensation_TABLE Chroma_Compensation_disabled = {{0}};

	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	if (value >= DCC_Chroma_Compensation_TABLE_MAX)
		value = 0;
		//Chroma_Compensation_TABLE = &(Chroma_Compensation_disabled);

	Chroma_Compensation_TABLE = &(gVip_Table->DCC_Control_Structure.DCC_Chroma_Compensation_Table[value]);

	// setting some information to sharememory
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.DCC_Chroma_Comp_Table= value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	//===============================

	VIPprintf("fwif_color_set_dcc_chroma_compensation Table = %d\n",value);
	drvif_color_set_dcc_chroma_compensation_Curve_Mode(_MAIN_DISPLAY, Chroma_Compensation_TABLE);
}

void fwif_color_set_dcc_Color_Independent_driver(unsigned char src_idx, unsigned char value)
{
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	VIP_DCC_Color_Independent_Blending_Table *Color_Independent_Blending_Table = NULL;
	VIP_DCC_Color_Independent_Blending_Table Color_Independent_disabled = {{0}};

	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	if (value >= Color_Independent_Blending_Table_MAX)
		Color_Independent_Blending_Table = &(Color_Independent_disabled);
	else
	Color_Independent_Blending_Table = &(gVip_Table->DCC_Control_Structure.Color_Independent_Blending_Table[value]);

	// setting some information to sharememory
	VIP_system_info_structure_table->PQ_Setting_Info.DCC_info_coef.DCC_Color_Dep_Table= value;
	VIP_system_info_structure_table->DCC_Status_Change = 1;
	//===============================

	drvif_color_set_dcc_Color_Independent(Color_Independent_Blending_Table);
}


/*======== Set DCC =============*/
/*===============================================================================================	//DCC CSFC 20140127*/



/*===============================================================================================	//PictureMode CSFC 20140128*/
/*==== set picture mode*/
void fwif_color_set_set_picture_mode(unsigned char src_idx, unsigned char value)
{

	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	/*=== 20140722 CSFC for vip video fw infomation ====*/
	/*setting some information to sharememory*/
	VIP_system_info_structure_table->OSD_Info.Picture_mode = value;
}

/*==== set picture mode*/
/*===============================================================================================	//PictureMode CSFC 20140128*/

int fwif_color_setCon_Bri_Color_Tint(unsigned char src_idx, unsigned char display, int Con, int Bri, int Color, int Tint)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	SLR_VIP_TABLE *gVip_Table = NULL;

	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t = {0};
	unsigned char coef_level;
	unsigned char RGB_level;
	unsigned char tbl_select;
	unsigned char i, j, k; /*, Y_Clamp;*/
	int contrast, bri, color, tint, deg_tint;
	/*unsigned short *table_idx[3][3];*/
	unsigned short table_idx[3][3][VIP_YUV2RGB_Y_Seg_Max];
	short satTmp[3][3], hueTmp[3][3], kTmp[3][3], kTmp1[3][3];
	unsigned short satCmps;
	short cmps = 0;
	unsigned char Input_mode;

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (VIP_system_info_structure_table == NULL || gVip_Table == NULL) {
		VIPprintf("~get shareMem | gvipTbl error return~\n");
		return -1;
	}

	memset(satTmp, 0, sizeof(satTmp));
	memset(hueTmp, 0, sizeof(hueTmp));
	memset(kTmp, 0, sizeof(kTmp));
	memset(kTmp1, 0, sizeof(kTmp1));

	coef_level = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.CoefByY_CtrlItem_Level;
	RGB_level = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.RGB_Offset_Level;
	tbl_select = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_TBL_Select;

	if (coef_level >=  VIP_YUV2RGB_LEVEL_SELECT_MAX || RGB_level >=  VIP_YUV2RGB_LEVEL_SELECT_MAX || tbl_select >=  YUV2RGB_TBL_Num) {
		VIPprintf("~level or table select Error return, level = %d, table select = %d, %s->%d, %s~\n", coef_level, tbl_select, __FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	Input_mode = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode;

	/*cal mapping gain.*/
	/*fwif_color_CalContrast_Gain(Con, &contrast, &Y_Clamp);*/
	contrast = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_CONTRAST, Con);
	bri = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_BRIGHTNESS, Bri);
	color = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_SATURTUION, Color);
	tint = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_HUE, Tint);
	tint = tint - 128;
	if (tint >=  0)
		deg_tint = tint;
	else
		deg_tint = 360 - (tint * (-1));
	VIPprintf("~gain, con = %d, bri = %d, color = %d, tint = %d~\n", contrast, bri, color, deg_tint);

	/*contrast compensation*/
	if (OSD_Contrast_Compensation) {
		fwif_color_CalContrast_Compensation(display, contrast, VIP_system_info_structure_table);
		cmps = fwif_color_ChangeOneUINT16Endian(VIP_system_info_structure_table->OSD_Info.Brightness_Cmps, 0);
		satCmps = fwif_color_ChangeOneUINT16Endian(VIP_system_info_structure_table->OSD_Info.Saturation_Cmps, 0);
		VIPprintf("fwif_color_setSaturation:::com_Saturation = %d, Saturation_Cmps = %d\n", color, satCmps);
	} else {
		cmps = 0;
		satCmps = 128;
	}
	/*Euler's formula*/
	hueTmp[0][0] = 1024;
	hueTmp[1][1] = CAdjustCosine(deg_tint);
	hueTmp[1][2] = CAdjustSine(deg_tint) * (-1);
	hueTmp[2][1] = CAdjustSine(deg_tint);
	hueTmp[2][2] = CAdjustCosine(deg_tint);

	/*restrict for sat, refernece to spec for k32.*/
	color = (color * satCmps) >> 7;
	color = (color > 128) ?  75 + (color*53 / 128) : color;
	satTmp[0][0] = 128;	/*128  = 1*/
	satTmp[1][1] = color;
	satTmp[2][2] = color;

	/*get vip table info for k coeff.*/	/* get table from pq misc, vip table is for coef gain*/
	/*table_idx[0][0] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[tbl_select].COEF_By_Y.K11[0]);
	table_idx[0][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[tbl_select].COEF_By_Y.K12[0]);
	table_idx[0][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[tbl_select].COEF_By_Y.K13[0]);
	table_idx[1][0] = table_idx[0][0];
	table_idx[1][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[tbl_select].COEF_By_Y.K22[0]);
	table_idx[1][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[tbl_select].COEF_By_Y.K23[0]);
	table_idx[2][0] = table_idx[0][0];
	table_idx[2][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[tbl_select].COEF_By_Y.K32[0]);
	table_idx[2][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[tbl_select].COEF_By_Y.K33[0]);*/
	for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++) {
		table_idx[0][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
		table_idx[0][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K12];
		table_idx[0][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K13];
		table_idx[1][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
		table_idx[1][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K22];
		table_idx[1][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K23];
		table_idx[2][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
		table_idx[2][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K32];
		table_idx[2][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K33];
	}

	/*matrix computation for contrast, sat and hue*/
	for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++) {
		for (j = 0; j < 3; j++) {
			for (k = 0; k < 3; k++) {
				kTmp[j][k] = table_idx[j][k][i];
				/*bigger than 0x3FF  = >neg*/
				if (j ==0) {	/* Y */
					if (kTmp[j][k] > VIP_YUV2RGB_K_Y_NEG_BND) {
						kTmp[j][k] = VIP_YUV2RGB_K_Y_NEG_CHANGE(kTmp[j][k]);
						kTmp[j][k] = kTmp[j][k] * (-1);
					}
				} else { /* C */
					if (kTmp[j][k] > VIP_YUV2RGB_K_C_NEG_BND) {
						kTmp[j][k] = VIP_YUV2RGB_K_C_NEG_CHANGE(kTmp[j][k]);
						kTmp[j][k] = kTmp[j][k] * (-1);
					}
				}
			}
		}

		/*3x3 matrix multiply*/
		for (j = 0; j < 3; j++)
			for (k = 0; k < 3; k++)
				kTmp1[j][k] = (kTmp[j][0] * hueTmp[0][k] + kTmp[j][1] * hueTmp[1][k] + kTmp[j][2] * hueTmp[2][k]) >> 10;

		for (j = 0; j < 3; j++)
			for (k = 0; k < 3; k++)
				kTmp[j][k] = (kTmp1[j][0] * satTmp[0][k] + kTmp1[j][1] * satTmp[1][k] + kTmp1[j][2] * satTmp[2][k]) >> 7;

		kTmp[0][0] = (kTmp[0][0] * contrast) >> 7;

		drv_vipCSMatrix_t.COEF_By_Y.K11[i] = kTmp[0][0];/*juwen, Merlin3, K11:S(11,8)->S(15,12)*//* juwen, by modifying pq_misc_table value*/
		drv_vipCSMatrix_t.COEF_By_Y.K12[i] = kTmp[0][1];
		drv_vipCSMatrix_t.COEF_By_Y.K13[i] = kTmp[0][2];
		drv_vipCSMatrix_t.COEF_By_Y.K22[i] = kTmp[1][1];
		drv_vipCSMatrix_t.COEF_By_Y.K23[i] = kTmp[1][2];
		drv_vipCSMatrix_t.COEF_By_Y.K32[i] = kTmp[2][1];
		drv_vipCSMatrix_t.COEF_By_Y.K33[i] = kTmp[2][2];
	}

	/*RGB offset setting for brightness*/
	drv_vipCSMatrix_t.RGB_Offset.R_offset = (((bri - 128) << 5) + (cmps << 4)) & 0x1ffff; /*20140220 roger for sync the brightness step with Mac, Magellen	modified by Flora, for Sirius; re - modified by Elsie 20140205*/
	drv_vipCSMatrix_t.RGB_Offset.G_offset = (((bri - 128) << 5) + (cmps << 4)) & 0x1ffff; /*cmps: 4bit fraction*/
	drv_vipCSMatrix_t.RGB_Offset.B_offset = (((bri - 128) << 5) + (cmps << 4)) & 0x1ffff;

	/*Clamp setting*/
	drv_vipCSMatrix_t.CTRL_ITEM.Y_Clamp = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.CTRL_ITEM.Y_Clamp;

	/*share memory setting*/
#ifdef CONFIG_ARM64
	memcpy_toio(&VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y, &drv_vipCSMatrix_t.COEF_By_Y, sizeof(DRV_VIP_YUV2RGB_COEF_By_Y));
	memcpy_toio(&VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset, &drv_vipCSMatrix_t.RGB_Offset, sizeof(DRV_VIP_YUV2RGB_RGB_Offset));
#else
	memcpy(&VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y, &drv_vipCSMatrix_t.COEF_By_Y, sizeof(DRV_VIP_YUV2RGB_COEF_By_Y));
	memcpy(&VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset, &drv_vipCSMatrix_t.RGB_Offset, sizeof(DRV_VIP_YUV2RGB_RGB_Offset));
#endif
	fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K11[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
	fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K12[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
	fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K13[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
	fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K22[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
	fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K23[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
	fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K32[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
	fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K33[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
	fwif_color_ChangeUINT32Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset.R_offset), 1, 0);
	fwif_color_ChangeUINT32Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset.G_offset), 1, 0);
	fwif_color_ChangeUINT32Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset.B_offset), 1, 0);
	VIP_system_info_structure_table->OSD_Info.Contrast_Gain = fwif_color_ChangeOneUINT16Endian((UINT16)contrast, 0);
	VIP_system_info_structure_table->OSD_Info.OSD_Contrast = Con;
	VIP_system_info_structure_table->OSD_Info.OSD_Hue = Tint;
	VIP_system_info_structure_table->OSD_Info.Hue_Gain = fwif_color_ChangeOneUINT16Endian(deg_tint, 0);
	VIP_system_info_structure_table->OSD_Info.OSD_Saturation = Color;
	VIP_system_info_structure_table->OSD_Info.Saturation_Gain = fwif_color_ChangeOneUINT16Endian(color, 0);
	VIP_system_info_structure_table->OSD_Info.OSD_Brightness = Bri;
	VIP_system_info_structure_table->OSD_Info.Brightness_Gain = fwif_color_ChangeOneUINT16Endian(bri, 0);

	if (display == SLR_MAIN_DISPLAY) {
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Y_Clamp, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_Y_Only, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Offset_Only, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	}
#ifdef VIP_CONFIG_DUAL_CHENNEL
	else {
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Y_Clamp, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_Y_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Offset_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
	}
#endif
	return 0;
}

int fwif_color_setCon_Bri_Color_Tint_from_System_Info(unsigned char src_idx, unsigned char display)
{
	int Con, Bri, Color, Tint;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL || gVip_Table == NULL) {
		VIPprintf("~get shareMem | return~\n");
		return  -1;
	}

	Con = VIP_system_info_structure_table->OSD_Info.OSD_Contrast;
	Bri = VIP_system_info_structure_table->OSD_Info.OSD_Brightness;
	Color = VIP_system_info_structure_table->OSD_Info.OSD_Saturation;
	Tint = VIP_system_info_structure_table->OSD_Info.OSD_Hue;
	VIPprintf("set pic From Sys = %d, %d, %d, %d\n", Con, Bri, Color, Tint);
	return fwif_color_setCon_Bri_Color_Tint(src_idx, display, Con, Bri, Color, Tint);

}

int fwif_color_get_ConBriMapping_Offset_Value(unsigned char HAL_Src_Idx, unsigned char UI_Level, unsigned char *Contrast, unsigned char *Brightness)
{
	unsigned char maxTBL_size;

	maxTBL_size = sizeof(ConBriMapping_Offset_Table) / (sizeof(char) * VIP_YUV2RGB_LEVEL_SELECT_MAX * 2);
	if (HAL_Src_Idx >=  maxTBL_size || UI_Level >=  VIP_YUV2RGB_LEVEL_SELECT_MAX) {
		VIPprintf("ConBriMapping_ReDefine Table size Error\n");
		return  -1;
	}

	*Contrast = ConBriMapping_Offset_Table[HAL_Src_Idx][UI_Level][0];
	*Brightness = ConBriMapping_Offset_Table[HAL_Src_Idx][UI_Level][1];
	VIPprintf("get_ConBriMapping_Offset = %d, %d\n", *Contrast, *Brightness);
	return 0;
}

int fwif_color_setConBri_FacMappingValue(unsigned char Contrast, unsigned char Brightness)
{
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	StructColorDataFacModeType *pColorFacTable = NULL;
	pColorFacTable = fwif_color_get_color_fac_mode(VIP_system_info_structure_table->color_fac_src_idx, 0);
	if (pColorFacTable == NULL) {
		VIPprintf("pColorFacTable = NULL\n");
		return  -1;
	}

	pColorFacTable->Brightness_50 = Brightness;
	pColorFacTable->Contrast_100 = Contrast;
	pColorFacTable->Contrast_50 = Contrast >> 1;
	return 0;
}

int fwif_color_get_VDBlackLevel_MappingValue(unsigned char HAL_Src_Idx, unsigned char UI_Level,
	unsigned char *Contrast, unsigned char *Brightness, unsigned char *Hue, unsigned char *Sat)
{
#ifndef BUILD_QUICK_SHOW

	unsigned char VDSatandard = 1; /*PAL*/
	unsigned char input_src_type;
	unsigned char input_src_form;
	unsigned char input_timming_info;
	unsigned char vdc_ColorStandard = VDC_MODE_NO_SIGNAL;

	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if (!vdc_power_status) {
		VIPprintf("VDBlackLevel_MappingValue Error, VD power off\n");
		return -1;
	}

	/*data protect*/
	if (HAL_Src_Idx > 1 || UI_Level > RGB2YUV_BLACK_LEVEL_AUTO) {
		VIPprintf("VDBlackLevel_MappingValue Error\n");
		return  -1;
	}

	if (UI_Level == RGB2YUV_BLACK_LEVEL_UNDEFINE)
		UI_Level = RGB2YUV_BLACK_LEVEL_LOW;	/* CVBS UNDEFINE ==> set default, level low*/

	UI_Level--; /*remapping to array index*/

	input_src_type = VIP_system_info_structure_table->Input_src_Type;
	input_src_form = VIP_system_info_structure_table->Input_src_Form;
	input_timming_info = VIP_system_info_structure_table->Timing;

	if (input_src_form == _SRC_FROM_VDC) {
		vdc_ColorStandard = drvif_module_vdc_ReadMode(VDC_SET);

		if (input_src_type == _SRC_TV) {
			if (vdc_ColorStandard == VDC_MODE_NTSC)
				VDSatandard = 0; /*NTSC*/
			else if (vdc_ColorStandard == VDC_MODE_PALM)
				VDSatandard = 4; /*PAL - M*/
			else if (vdc_ColorStandard == VDC_MODE_NTSC443)
				VDSatandard = 3; /*NTSC - 443*/
			else if (vdc_ColorStandard == VDC_MODE_PALI)
				VDSatandard = 1; /*PAL*/
			else if (vdc_ColorStandard == VDC_MODE_SECAM)
				VDSatandard = 2; /*SECAM*/
			else if (vdc_ColorStandard == VDC_MODE_PALN)
				VDSatandard = 5; /*PAL - N*/
			else if (vdc_ColorStandard == VDC_MODE_PAL60)
				VDSatandard = 6; /*PAL - 60*/
			else if (vdc_ColorStandard == VDC_MODE_NTSC50)
				VDSatandard = 7; /*PAL - Nc*/
			else
				VDSatandard = 1; /*PAL*/
		} else if (input_src_type == _SRC_CVBS) {
			if (vdc_ColorStandard == VDC_MODE_NTSC)
				VDSatandard = 0; /*NTSC*/
			else if (vdc_ColorStandard == VDC_MODE_PALM)
				VDSatandard = 4; /*PAL - M*/
			else if (vdc_ColorStandard == VDC_MODE_NTSC443)
				VDSatandard = 3; /*NTSC - 443*/
			else if (vdc_ColorStandard == VDC_MODE_PALI)
				VDSatandard = 1; /*PAL*/
			else if (vdc_ColorStandard == VDC_MODE_SECAM)
				VDSatandard = 2; /*SECAM*/
			else if (vdc_ColorStandard == VDC_MODE_PALN)
				VDSatandard = 5; /*PAL - N*/
			else if (vdc_ColorStandard == VDC_MODE_PAL60)
				VDSatandard = 6; /*PAL - 60*/
			else if (vdc_ColorStandard == VDC_MODE_NTSC50)
				VDSatandard = 7; /*PAL - Nc*/
			else
				VDSatandard = 1; /*PAL*/
		}  else {
				return -1;
		}
	} else {
		return -1;
	}

	if (0 == HAL_Src_Idx) {/*ATV*/
		*Contrast = ATVBlackLevelMapping_Table[VDSatandard][UI_Level][0];
		*Brightness = ATVBlackLevelMapping_Table[VDSatandard][UI_Level][1];
		*Hue = ATVBlackLevelMapping_Table[VDSatandard][UI_Level][2];
		*Sat = ATVBlackLevelMapping_Table[VDSatandard][UI_Level][3];



	} else {/*AV*/
		*Contrast = AVBlackLevelMapping_Table[VDSatandard][UI_Level][0];
		*Brightness = AVBlackLevelMapping_Table[VDSatandard][UI_Level][1];
		*Hue = AVBlackLevelMapping_Table[VDSatandard][UI_Level][2];
		*Sat = AVBlackLevelMapping_Table[VDSatandard][UI_Level][3];

	}
	/*VIPprintf("Get VD Con = %d, Bri = %d\n", *Contrast, *Brightness);*/
#endif	
	return 0;
}

int fwif_color_set_VDBlackLevel(unsigned char Contrast, unsigned char Brightness, unsigned char Hue, unsigned char Sat)
{
#ifndef BUILD_QUICK_SHOW //quick show no vd case
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if ((VIP_system_info_structure_table->Input_src_Form) !=  _SRC_FROM_VDC) {
		VIPprintf("Warning!! It's not vdc Src !!");
		return  -1;
	}
	fwif_color_WaitFor_DEN_STOP_UZUDTG();
	drvif_module_vdc_SetContrast(Contrast);
	drvif_module_vdc_SetBrightness(Brightness);

	drvif_module_vdc_SetHue(Hue);
	drvif_module_vdc_SetSaturation(Sat);

#endif
	return 0;
}
//============== ICM  =====
int fwif_color_icm_set_ICM_H_7axis_adjust_HSI(unsigned char display, unsigned char which_table, int prvValue, int value, unsigned char Tuning_Axis, unsigned char HSI)
{
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	ICM_H_7axis_table *tICM_H_7axis = NULL;
	int i = 0, prvoffset = 0, offset = 0, sta = 0, end = 0, h_total;
	int adjust_len = 0;
	int result = 0;
	unsigned char src_idx = 0;
	unsigned short valueRange = 0, valueCenter = 0;
	unsigned char HSI_idx = 0;
	StructColorDataType *pTable = NULL;

	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}

	if (which_table >= VIP_ICM_TBL_X)
		return 0;

	tICM_H_7axis = &(gVip_Table->tICM_H_7axis[which_table]);
	Scaler_Get_Display_info(&display,&src_idx);
	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;

	switch(HSI)
	{
		case _HUE_ELEM:
			valueRange = 255;
			valueCenter = 128;
			HSI_idx = 2;
			break;
		case _SAT_ELEM:
			valueRange = 1023;
			valueCenter = 512;
			HSI_idx = 3;
			break;
		case _ITN_ELEM:
			valueRange = 1023;
			valueCenter = 512;
			HSI_idx = 4;
			break;
		default:
			return 0;
	}

	if (value < -valueRange || value > valueRange)
		return 0;
	if (Tuning_Axis == Axis_tuning_off) {
		for (i = 0; i < Axis_Max; i++) {

			sta = *(&(tICM_H_7axis->R[0]) + ((i*5)+0));
			end = *(&(tICM_H_7axis->R[0]) + ((i*5)+1));
			offset = *(&(tICM_H_7axis->R[0]) + ((i*5)+HSI_idx));
			h_total = (tICM_H_7axis->Total_axis);
			adjust_len = ((end-sta) > 0) ? ((end-sta)+1) : ((h_total-sta)+end+1);
			prvoffset = pTable->ICM_7Axis_adjust_value[i][HSI] - valueCenter;
			offset -= valueCenter;

			//printf("[%d] sta =%d, end =%d, offset =%d prvValue = %d\n", i, sta, end, offset, prvValue);
			//. sta&itn valid pillar: 0~8
			result = drvif_color_icm_Adjust_Block(HSI, prvoffset, offset, adjust_len, sta, end, h_total, 0, 8, 9, 0,8, 9);
		}
	} else {
		sta = *(&(tICM_H_7axis->R[0])+((Tuning_Axis*5)+0));
		end = *(&(tICM_H_7axis->R[0])+((Tuning_Axis*5)+1));
		h_total = (tICM_H_7axis->Total_axis);

		adjust_len = ((end-sta) > 0) ? ((end-sta)+1) : ((h_total-sta)+end+2);

		offset = value;
		//printf("[%d] sta =%d, end =%d, offset =%d prvoffset = %d\n", Tuning_Axis, sta, end, offset, prvValue);
		//. sta&itn valid pillar: 0~8
		result = drvif_color_icm_Adjust_Block(HSI, prvValue, offset, adjust_len, sta, end, h_total, 0, 8, 9, 0, 8, 9);
	}
	return result;
}

int fwif_color_icm_get_ICM_H_7axis_adjust_HSI(int index, unsigned char Tuning_Axis)
{
	StructColorDataType *pTable = NULL;
	ICM_H_7axis_table *tICM_H_7axis = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	unsigned char which_table;
	int value = 0;


	Scaler_Get_Display_info(&display,&src_idx);
	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return 0;
	gVip_Table=fwif_colo_get_AP_vip_table_gVIP_Table();
	if(gVip_Table==NULL)
	{
		printf("~get vipTable Error return, %s->%d, %s~\n",__FILE__, __LINE__,__FUNCTION__);
		return 0;
	}

	which_table = pTable->ICM_7Axis_Table;
	if (which_table >= VIP_ICM_TBL_X)
		which_table = 0;
	tICM_H_7axis = &(gVip_Table->tICM_H_7axis[which_table]);

	value = *(&(tICM_H_7axis->R[0])+((Tuning_Axis*5)+index));
	return value;
}


/*===============================================================================================	//ConBri CSFC 20140128*/
/*========Set Con/Bri=============*/
#if defined(CONFIG_RTK_AI_DRV)
extern short bri_10bit;
void fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain_AI(unsigned char src_idx, unsigned char display,  _system_setting_info *system_info_struct/*, int Con, int Bri, int Color*/)
{
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	unsigned short table_idx[3][3][VIP_YUV2RGB_Y_Seg_Max];
	short satTmp[3][3], hueTmp[3][3], kTmp[3][3], kTmp1[3][3];
	unsigned int satCmps;
	short cmps = 0;
	unsigned char Input_mode, YUV2RGB_TBL_Idx;

	UINT32 contrast, bri, color, deg_tint;
	int tbl_offset_R, tbl_offset_G, tbl_offset_B;

	UINT8 i, j, k;

	unsigned char bt2020_Constant_En;
	int bt2020_k13, bt2020_k32;

	Input_mode = system_info_struct->using_YUV2RGB_Matrix_Info.Input_Data_Mode;
	YUV2RGB_TBL_Idx = fwif_color_get_Decide_YUV2RGB_TBL_Index(src_idx, display, Input_mode);

	if (display == SLR_MAIN_DISPLAY) {
		contrast = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Contrast_Gain, 0);
		bri = bri_10bit;//fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Brightness_Gain, 0);
		color = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Saturation_Gain, 0);
		deg_tint = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Hue_Gain, 0);
		/*contrast compensation*/
		if (OSD_Contrast_Compensation) {
			cmps = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Brightness_Cmps, 0);
			satCmps = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Saturation_Cmps, 0);
			VIPprintf("fwif_color_setSaturation:::com_Saturation = %d, Saturation_Cmps = %d\n", color, satCmps);
		} else {
			cmps = 0;
			satCmps = 128;
		}
		/* for K13_2, K32_2*/
		if (system_info_struct->BT2020_CTRL.Enable_Flag == 1 && system_info_struct->BT2020_CTRL.Mode == BT2020_MODE_Constant)
			bt2020_Constant_En = 1;
		else
			bt2020_Constant_En = 0;
	} else {
		contrast = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Contrast_Gain_Sub, 0);
		bri = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Brightness_Gain_Sub, 0);
		color = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Saturation_Gain_Sub, 0);
		deg_tint = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Hue_Gain_Sub, 0);
		/*contrast compensation*/
		if (OSD_Contrast_Compensation) {
			cmps = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Brightness_Cmps_Sub, 0);
			satCmps = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Saturation_Cmps_Sub, 0);
			VIPprintf("fwif_color_setSaturation:::com_Saturation_sub = %d, Saturation_Cmps_Sub = %d\n", color, satCmps);
		} else {
			cmps = 0;
			satCmps = 128;
		}
		/* for K13_2, K32_2*/
		if (system_info_struct->BT2020_CTRL.Enable_Flag_Sub == 1 && system_info_struct->BT2020_CTRL.Mode_Sub == BT2020_MODE_Constant)
			bt2020_Constant_En = 1;
		else
			bt2020_Constant_En = 0;
	}

	VIPprintf("[vpq][%s][%d]contrast = %d, bri = %d, color = %d, deg_tint = %d\n", __FUNCTION__, __LINE__, contrast, bri, color, deg_tint);
	memset(satTmp, 0, sizeof(satTmp));
	memset(hueTmp, 0, sizeof(hueTmp));
	memset(kTmp, 0, sizeof(kTmp));
	memset(kTmp1, 0, sizeof(kTmp1));

	/* no hue function while bt2020_Constant_En*/
	if (bt2020_Constant_En == 1)
		deg_tint = 0;
	/*Euler's formula*/
	hueTmp[0][0] = 1024;
	hueTmp[1][1] = CAdjustCosine(deg_tint);
	hueTmp[1][2] = CAdjustSine(deg_tint) * (-1);
	hueTmp[2][1] = CAdjustSine(deg_tint);
	hueTmp[2][2] = CAdjustCosine(deg_tint);
	/*restrict for sat, refernece to spec for k32.*/
	#if 1 //lesley
	color = (color * satCmps) >> 7;
	color = (color > 128) ?  75 + (color*53 / 128) : color;
	#endif
	satTmp[0][0] = 128;	/*128  = 1*/
	satTmp[1][1] = color;
	satTmp[2][2] = color;

	for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++) {
		table_idx[0][0][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K11];
		table_idx[0][1][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K12];
		table_idx[0][2][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K13];
		table_idx[1][0][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K11];
		table_idx[1][1][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K22];
		table_idx[1][2][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K23];
		table_idx[2][0][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K11];
		table_idx[2][1][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K32];
		table_idx[2][2][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K33];
	}

	/*matrix computation for contrast, sat and hue*/
	for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++) {
		for (j = 0; j < 3; j++) {
			for (k = 0; k < 3; k++) {
				kTmp[j][k] = table_idx[j][k][i];
				/*bigger than 0x3FF  = >neg*/
				if (j == 0) {
					if ((kTmp[j][k] > VIP_YUV2RGB_K_Y_NEG_BND)){
						kTmp[j][k] = VIP_YUV2RGB_K_Y_NEG_CHANGE(kTmp[j][k]);
						kTmp[j][k] = kTmp[j][k] * (-1);
					}
				} else {
					if ((kTmp[j][k] > VIP_YUV2RGB_K_C_NEG_BND)){
						kTmp[j][k] = VIP_YUV2RGB_K_C_NEG_CHANGE(kTmp[j][k]);
						kTmp[j][k] = kTmp[j][k] * (-1);
					}
				}
			}
		}
		/*3x3 matrix multiply*/
		for (j = 0; j < 3; j++)
			for (k = 0; k < 3; k++)
				kTmp1[j][k] = (kTmp[j][0] * hueTmp[0][k] + kTmp[j][1] * hueTmp[1][k] + kTmp[j][2] * hueTmp[2][k]) >> 10;

		for (j = 0; j < 3; j++)
			for (k = 0; k < 3; k++)
				kTmp[j][k] = (kTmp1[j][0] * satTmp[0][k] + kTmp1[j][1] * satTmp[1][k] + kTmp1[j][2] * satTmp[2][k]) >> 7;

		kTmp[0][0] = (kTmp[0][0] * contrast) >> 7;

		drv_vipCSMatrix_t.COEF_By_Y.K11[i] = kTmp[0][0];
		drv_vipCSMatrix_t.COEF_By_Y.K12[i] = kTmp[0][1];
		drv_vipCSMatrix_t.COEF_By_Y.K13[i] = kTmp[0][2];
		drv_vipCSMatrix_t.COEF_By_Y.K22[i] = kTmp[1][1];
		drv_vipCSMatrix_t.COEF_By_Y.K23[i] = kTmp[1][2];
		drv_vipCSMatrix_t.COEF_By_Y.K32[i] = kTmp[2][1];
		drv_vipCSMatrix_t.COEF_By_Y.K33[i] = kTmp[2][2];
	}

	/* bt2020_Constant_En,  for bt 2020 k13, k32*/
	if (bt2020_Constant_En == 1) {
		bt2020_k13 =  tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K13_2];
		bt2020_k32 =  tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K32_2];
		if (bt2020_k13 > VIP_YUV2RGB_K_C_NEG_BND) {
			bt2020_k13 = VIP_YUV2RGB_K_C_NEG_CHANGE(bt2020_k13);
			bt2020_k13 = bt2020_k13 * (-1);
		}
		if (bt2020_k32 > VIP_YUV2RGB_K_C_NEG_BND) {
			bt2020_k32 = VIP_YUV2RGB_K_C_NEG_CHANGE(bt2020_k32);
			bt2020_k32 = bt2020_k32 * (-1);
		}
		/* no hue function while bt 2020, only saturation*/
		bt2020_k13 = (bt2020_k13 * satTmp[1][1])>>7;
		bt2020_k32 = (bt2020_k32 * satTmp[1][1])>>7;
		/* driver for bt 2020*/
		drv_vipCSMatrix_t.COEF_By_Y.bt2020_K13= bt2020_k13;
		drv_vipCSMatrix_t.COEF_By_Y.bt2020_K32= bt2020_k32;
	}

	/*RGB offset setting for brightness*/
	/*Roffset, Goffset, Boffset: S(15,2)*//*bigger than 0x3FFF  = >neg*/
	tbl_offset_R = (tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_R_Offset] > VIP_YUV2RGB_OFFSET_NEG_BND) ? (VIP_YUV2RGB_OFFSET_NEG_CHANGE(tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_R_Offset])) : (tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_R_Offset]);
	tbl_offset_G = (tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_G_Offset] > VIP_YUV2RGB_OFFSET_NEG_BND) ? (VIP_YUV2RGB_OFFSET_NEG_CHANGE(tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_G_Offset])) : (tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_G_Offset]);
	tbl_offset_B = (tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_B_Offset] > VIP_YUV2RGB_OFFSET_NEG_BND) ? (VIP_YUV2RGB_OFFSET_NEG_CHANGE(tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_B_Offset])) : (tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_B_Offset]);
	#if 1 // lesley, bri 10 bits
	drv_vipCSMatrix_t.RGB_Offset.R_offset = (((bri - (128<<2)) << (5-2)) + (cmps << 4) + tbl_offset_R) & 0x1ffff; /*20140220 roger for sync the brightness step with Mac, Magellen	modified by Flora, for Sirius; re - modified by Elsie 20140205*/
	drv_vipCSMatrix_t.RGB_Offset.G_offset = (((bri - (128<<2)) << (5-2)) + (cmps << 4) + tbl_offset_G) & 0x1ffff; /*cmps: 4bit fraction*/
	drv_vipCSMatrix_t.RGB_Offset.B_offset = (((bri - (128<<2)) << (5-2)) + (cmps << 4) + tbl_offset_B) & 0x1ffff;
	#else // bri 8 bits
	drv_vipCSMatrix_t.RGB_Offset.R_offset = (((bri - 128) << 5) + (cmps << 4) + tbl_offset_R) & 0x1ffff; /*20140220 roger for sync the brightness step with Mac, Magellen	modified by Flora, for Sirius; re - modified by Elsie 20140205*/
	drv_vipCSMatrix_t.RGB_Offset.G_offset = (((bri - 128) << 5) + (cmps << 4) + tbl_offset_G) & 0x1ffff; /*cmps: 4bit fraction*/
	drv_vipCSMatrix_t.RGB_Offset.B_offset = (((bri - 128) << 5) + (cmps << 4) + tbl_offset_B) & 0x1ffff;
	#endif

	VIPprintf("[vpq][%s][%d]Input_mode = %d, YUV2RGB_TBL_Idx = %d, cmps = %d\n", __FUNCTION__, __LINE__, Input_mode, YUV2RGB_TBL_Idx, cmps);
	VIPprintf("[vpq][%s][%d]tbl_offset_R = %d, RGB_Offset.R_offset = %d\n", __FUNCTION__, __LINE__, tbl_offset_R, drv_vipCSMatrix_t.RGB_Offset.R_offset);
	VIPprintf("[vpq][%s][%d]tbl_offset_G = %d, RGB_Offset.G_offset = %d\n", __FUNCTION__, __LINE__, tbl_offset_G, drv_vipCSMatrix_t.RGB_Offset.G_offset);
	VIPprintf("[vpq][%s][%d]tbl_offset_B = %d, RGB_Offset.B_offset = %d\n", __FUNCTION__, __LINE__, tbl_offset_B, drv_vipCSMatrix_t.RGB_Offset.B_offset);

#ifdef ENABLE_xvYcc
	/* sync YUV2RGB flow*/
#endif

	/*Clamp setting*/
	/*drv_vipCSMatrix_t.CTRL_ITEM.Y_Clamp = Y_Clamp;*/
	drv_vipCSMatrix_t.CTRL_ITEM.Y_Clamp = system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.CTRL_ITEM.Y_Clamp;

	if (display == SLR_MAIN_DISPLAY) {
		/*share memory setting*/
#ifdef CONFIG_ARM64
		memcpy_toio(&system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y, &drv_vipCSMatrix_t.COEF_By_Y, sizeof(DRV_VIP_YUV2RGB_COEF_By_Y));
		memcpy_toio(&system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset, &drv_vipCSMatrix_t.RGB_Offset, sizeof(DRV_VIP_YUV2RGB_RGB_Offset));
#else
		memcpy(&system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y, &drv_vipCSMatrix_t.COEF_By_Y, sizeof(DRV_VIP_YUV2RGB_COEF_By_Y));
		memcpy(&system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset, &drv_vipCSMatrix_t.RGB_Offset, sizeof(DRV_VIP_YUV2RGB_RGB_Offset));
#endif
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K11[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K12[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K13[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K22[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K23[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K32[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K33[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT32Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset.R_offset), 1, 0);
		fwif_color_ChangeUINT32Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset.G_offset), 1, 0);
		fwif_color_ChangeUINT32Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset.B_offset), 1, 0);
		/*drv setting*/
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Y_Clamp, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_Y_Only, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Offset_Only, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		/* bt2020_Constant_En,  for bt 2020 k13, k32*/
		if (bt2020_Constant_En == 1) {
			/*drv setting*/
			drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_BT2020_Only, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
			/*memory setting*/
			fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.bt2020_K13), 1, 0);
			fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.bt2020_K32), 1, 0);
		}
	}
/*#ifdef VIP_CONFIG_DUAL_CHENNEL*/
	else {
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Y_Clamp, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_Y_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Offset_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		/* bt2020_Constant_En,  for bt 2020 k13, k32*/
		if (bt2020_Constant_En == 1)
			drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_BT2020_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
	}
/*#endif*/
	return;

}
#endif

static struct file* file_open(const char* path, int flags, int rights) {
#ifndef BUILD_QUICK_SHOW
	struct file* filp = NULL;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	mm_segment_t oldfs;
	oldfs = get_fs();
	set_fs(KERNEL_DS);
#endif
	filp = filp_open(path, flags, rights);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	set_fs(oldfs);
#endif
	if(IS_ERR(filp)) {
		return NULL;
	}
	return filp;
#else //BUILD_QUICK_SHOW
	return NULL;
#endif //BUILD_QUICK_SHOW
}

static void file_close(struct file* file) {
#ifndef BUILD_QUICK_SHOW

	filp_close(file, NULL);
#endif
}

static int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
#ifndef BUILD_QUICK_SHOW
	int ret;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	mm_segment_t oldfs;
	oldfs = get_fs();
	set_fs(KERNEL_DS);
#endif
	ret = kernel_write(file, data, size, &offset);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	set_fs(oldfs);
#endif
	return ret;
#else
	return 0;
#endif 
}

static int file_sync(struct file* file) {
#ifndef BUILD_QUICK_SHOW
	
	vfs_fsync(file, 0);
#endif
	return 0;
}





struct file *file = NULL;
char buf[128];
unsigned char xxx[500]={1,2,3,4,5,6,7,8,9,10};
extern unsigned int PCID2_Table[];
extern DRV_QS_Table QS_mode_table;

void fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(unsigned char src_idx, unsigned char display,  _system_setting_info *system_info_struct)
{
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	unsigned short table_idx[3][3][VIP_YUV2RGB_Y_Seg_Max];
	short satTmp[3][3], hueTmp[3][3], kTmp[3][3], kTmp1[3][3];
	unsigned int satCmps;
	short cmps = 0;
	unsigned char Input_mode, YUV2RGB_TBL_Idx;

	UINT32 contrast, bri, color, deg_tint;
	int tbl_offset_R, tbl_offset_G, tbl_offset_B;

	UINT8 i, j, k;

	unsigned char bt2020_Constant_En;
	int bt2020_k13, bt2020_k32;

	//static QS_value_Store qs_gain;


	Input_mode = system_info_struct->using_YUV2RGB_Matrix_Info.Input_Data_Mode;
	YUV2RGB_TBL_Idx = fwif_color_get_Decide_YUV2RGB_TBL_Index(src_idx, display, Input_mode);
#ifndef BUILD_QUICK_SHOW

	if(is_QS_pq_enable()==1) { //current is QS flow	pq_quick_show flow
		YUV2RGB_TBL_Idx =7;
	}
#endif
	if (display == SLR_MAIN_DISPLAY) {
		contrast = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Contrast_Gain, 0);
		bri = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Brightness_Gain, 0);
		color = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Saturation_Gain, 0);
		deg_tint = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Hue_Gain, 0);
		/*contrast compensation*/
		if (OSD_Contrast_Compensation) {
			cmps = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Brightness_Cmps, 0);
			satCmps = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Saturation_Cmps, 0);
			VIPprintf("fwif_color_setSaturation:::com_Saturation = %d, Saturation_Cmps = %d\n", color, satCmps);
		} else {
			cmps = 0;
			satCmps = 128;
		}
		/* for K13_2, K32_2*/
		if (system_info_struct->BT2020_CTRL.Enable_Flag == 1 && system_info_struct->BT2020_CTRL.Mode == BT2020_MODE_Constant)
			bt2020_Constant_En = 1;
		else
			bt2020_Constant_En = 0;
	} else {
		contrast = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Contrast_Gain_Sub, 0);
		bri = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Brightness_Gain_Sub, 0);
		color = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Saturation_Gain_Sub, 0);
		deg_tint = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Hue_Gain_Sub, 0);
		/*contrast compensation*/
		if (OSD_Contrast_Compensation) {
			cmps = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Brightness_Cmps_Sub, 0);
			satCmps = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Saturation_Cmps_Sub, 0);
			VIPprintf("fwif_color_setSaturation:::com_Saturation_sub = %d, Saturation_Cmps_Sub = %d\n", color, satCmps);
		} else {
			cmps = 0;
			satCmps = 128;
		}
		/* for K13_2, K32_2*/
		if (system_info_struct->BT2020_CTRL.Enable_Flag_Sub == 1 && system_info_struct->BT2020_CTRL.Mode_Sub == BT2020_MODE_Constant)
			bt2020_Constant_En = 1;
		else
			bt2020_Constant_En = 0;
	}

	if (Scaler_Get_CinemaMode_PQ()) {
		contrast = 128;
		bri = 128;
		color = 128;
		deg_tint = 0;
		cmps = 0;
		satCmps = 128;
	}


	pr_emerg("[vpq][%s][%d]contrast = %d, bri = %d, color = %d, deg_tint = %d ,satCmps =%d , cmps =%d \n", __FUNCTION__, __LINE__, contrast, bri, color, deg_tint,satCmps,cmps);


	/*
	qs_gain.OSD_Contrast =fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.OSD_Contrast, 0);
	qs_gain.OSD_Brightness =fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.OSD_Brightness, 0);
	qs_gain.OSD_Saturation =fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.OSD_Saturation, 0);
	qs_gain.OSD_Hue =fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.OSD_Hue, 0);
	qs_gain.OSD_Sharpness = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.OSD_Sharpness, 0);
	qs_gain.OSD_colorTemp = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.OSD_colorTemp, 0);

	qs_gain.Contrast_Gain = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Contrast_Gain, 0);
	qs_gain.Brightness_Gain = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Brightness_Gain, 0);
	qs_gain.Saturation_Gain= fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Saturation_Gain, 0);
	qs_gain.Hue_Gain= fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Hue_Gain, 0);
	qs_gain.Brightness_Cmps = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Brightness_Cmps, 0);
	qs_gain.Saturation_Cmps = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Saturation_Cmps, 0);

	pr_emerg("\n [vpq] OSD_Contrast %d, OSD_Brightness :%d ,OSD_Saturation%d , OSD_Hue %d ,OSD_Sharpness %d , OSD_colorTemp %d\n"
		,qs_gain.OSD_Contrast
		,qs_gain.OSD_Brightness
		,qs_gain.OSD_Saturation
		,qs_gain.OSD_Hue
		,qs_gain.OSD_Sharpness
		,qs_gain.OSD_colorTemp);

	pr_emerg("\n [vpq] Contrast_Gain %d, Brightness_Gain :%d ,Saturation_Gain %d , Hue_Gain %d , Brightness_Cmps%d ,Saturation_Cmps %d \n",qs_gain.Contrast_Gain
		,qs_gain.Brightness_Gain
		,qs_gain.Saturation_Gain
		,qs_gain.Hue_Gain
		,qs_gain.Brightness_Cmps
		,qs_gain.Saturation_Cmps);

	
	fwif_write_qs_pq_table(qs_Osd,(unsigned char*)&qs_gain); //pq_quick_show flow 
	*/
	memset(satTmp, 0, sizeof(satTmp));
	memset(hueTmp, 0, sizeof(hueTmp));
	memset(kTmp, 0, sizeof(kTmp));
	memset(kTmp1, 0, sizeof(kTmp1));

	/* no hue function while bt2020_Constant_En*/
	if (bt2020_Constant_En == 1)
		deg_tint = 0;
	/*Euler's formula*/
	hueTmp[0][0] = 1024;
	hueTmp[1][1] = CAdjustCosine(deg_tint);
	hueTmp[1][2] = CAdjustSine(deg_tint) * (-1);
	hueTmp[2][1] = CAdjustSine(deg_tint);
	hueTmp[2][2] = CAdjustCosine(deg_tint);
	/*restrict for sat, refernece to spec for k32.*/
	color = (color * satCmps) >> 7;
	color = (color > 128) ?  75 + (color*53 / 128) : color;
	satTmp[0][0] = 128;	/*128  = 1*/
	satTmp[1][1] = color;
	satTmp[2][2] = color;

	for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++) {
		table_idx[0][0][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K11];
		table_idx[0][1][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K12];
		table_idx[0][2][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K13];
		table_idx[1][0][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K11];
		table_idx[1][1][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K22];
		table_idx[1][2][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K23];
		table_idx[2][0][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K11];
		table_idx[2][1][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K32];
		table_idx[2][2][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K33];
	}

	/*matrix computation for contrast, sat and hue*/
	for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++) {
		for (j = 0; j < 3; j++) {
			for (k = 0; k < 3; k++) {
				kTmp[j][k] = table_idx[j][k][i];
				/*bigger than 0x3FF  = >neg*/
				if (j == 0) {
					if ((kTmp[j][k] > VIP_YUV2RGB_K_Y_NEG_BND)){
						kTmp[j][k] = VIP_YUV2RGB_K_Y_NEG_CHANGE(kTmp[j][k]);
						kTmp[j][k] = kTmp[j][k] * (-1);
					}
				} else {
					if ((kTmp[j][k] > VIP_YUV2RGB_K_C_NEG_BND)){
						kTmp[j][k] = VIP_YUV2RGB_K_C_NEG_CHANGE(kTmp[j][k]);
						kTmp[j][k] = kTmp[j][k] * (-1);
					}
				}
			}
		}
		/*3x3 matrix multiply*/
		for (j = 0; j < 3; j++)
			for (k = 0; k < 3; k++)
				kTmp1[j][k] = (kTmp[j][0] * hueTmp[0][k] + kTmp[j][1] * hueTmp[1][k] + kTmp[j][2] * hueTmp[2][k]) >> 10;

		for (j = 0; j < 3; j++)
			for (k = 0; k < 3; k++)
				kTmp[j][k] = (kTmp1[j][0] * satTmp[0][k] + kTmp1[j][1] * satTmp[1][k] + kTmp1[j][2] * satTmp[2][k]) >> 7;

		kTmp[0][0] = (kTmp[0][0] * contrast) >> 7;

		drv_vipCSMatrix_t.COEF_By_Y.K11[i] = kTmp[0][0];
		drv_vipCSMatrix_t.COEF_By_Y.K12[i] = kTmp[0][1];
		drv_vipCSMatrix_t.COEF_By_Y.K13[i] = kTmp[0][2];
		drv_vipCSMatrix_t.COEF_By_Y.K22[i] = kTmp[1][1];
		drv_vipCSMatrix_t.COEF_By_Y.K23[i] = kTmp[1][2];
		drv_vipCSMatrix_t.COEF_By_Y.K32[i] = kTmp[2][1];
		drv_vipCSMatrix_t.COEF_By_Y.K33[i] = kTmp[2][2];
	}

	/*
	pr_emerg("\n vpq contrast %d, deg_tint %d , color %d  satCmps %d , cmps %d\n ",
		contrast,
		deg_tint,
		color,satCmps,cmps);

	pr_emerg("\n vpq K11 %x, K12,%x , k13 %x  \n ",
		kTmp[0][0],
		kTmp[0][1],
		kTmp[0][2]);


	pr_emerg("\n vpq K22 %x, K23,%x , k32 %x ,k33 %x \n ",
		kTmp[1][1],
		kTmp[1][2],
		kTmp[2][1],
		kTmp[2][2]);

	*/
	

	/* bt2020_Constant_En,  for bt 2020 k13, k32*/
	if (bt2020_Constant_En == 1) {
		bt2020_k13 =  tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K13_2];
		bt2020_k32 =  tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K32_2];
		if (bt2020_k13 > VIP_YUV2RGB_K_C_NEG_BND) {
			bt2020_k13 = VIP_YUV2RGB_K_C_NEG_CHANGE(bt2020_k13);
			bt2020_k13 = bt2020_k13 * (-1);
		}
		if (bt2020_k32 > VIP_YUV2RGB_K_C_NEG_BND) {
			bt2020_k32 = VIP_YUV2RGB_K_C_NEG_CHANGE(bt2020_k32);
			bt2020_k32 = bt2020_k32 * (-1);
		}
		/* no hue function while bt 2020, only saturation*/
		bt2020_k13 = (bt2020_k13 * satTmp[1][1])>>7;
		bt2020_k32 = (bt2020_k32 * satTmp[1][1])>>7;
		/* driver for bt 2020*/
		drv_vipCSMatrix_t.COEF_By_Y.bt2020_K13= bt2020_k13;
		drv_vipCSMatrix_t.COEF_By_Y.bt2020_K32= bt2020_k32;
	}

	/*RGB offset setting for brightness*/
	/*Roffset, Goffset, Boffset: S(15,2)*//*bigger than 0x3FFF  = >neg*/
	tbl_offset_R = (tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_R_Offset] > VIP_YUV2RGB_OFFSET_NEG_BND) ? (VIP_YUV2RGB_OFFSET_NEG_CHANGE(tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_R_Offset])) : (tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_R_Offset]);
	tbl_offset_G = (tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_G_Offset] > VIP_YUV2RGB_OFFSET_NEG_BND) ? (VIP_YUV2RGB_OFFSET_NEG_CHANGE(tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_G_Offset])) : (tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_G_Offset]);
	tbl_offset_B = (tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_B_Offset] > VIP_YUV2RGB_OFFSET_NEG_BND) ? (VIP_YUV2RGB_OFFSET_NEG_CHANGE(tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_B_Offset])) : (tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_B_Offset]);

	drv_vipCSMatrix_t.RGB_Offset.R_offset = (((bri - 128) << 5) + (cmps << 4) + tbl_offset_R) & 0x1ffff; /*20140220 roger for sync the brightness step with Mac, Magellen	modified by Flora, for Sirius; re - modified by Elsie 20140205*/
	drv_vipCSMatrix_t.RGB_Offset.G_offset = (((bri - 128) << 5) + (cmps << 4) + tbl_offset_G) & 0x1ffff; /*cmps: 4bit fraction*/
	drv_vipCSMatrix_t.RGB_Offset.B_offset = (((bri - 128) << 5) + (cmps << 4) + tbl_offset_B) & 0x1ffff;


	//pr_emerg("[vpq][%s][%d]Input_mode = %d, YUV2RGB_TBL_Idx = %d, cmps = %d\n", __FUNCTION__, __LINE__, Input_mode, YUV2RGB_TBL_Idx, cmps);
	//pr_emerg("[vpq][%s][%d]tbl_offset_R = %d, RGB_Offset.R_offset = %d\n", __FUNCTION__, __LINE__, tbl_offset_R, drv_vipCSMatrix_t.RGB_Offset.R_offset);
	//pr_emerg("[vpq][%s][%d]tbl_offset_G = %d, RGB_Offset.G_offset = %d\n", __FUNCTION__, __LINE__, tbl_offset_G, drv_vipCSMatrix_t.RGB_Offset.G_offset);
	//pr_emerg("[vpq][%s][%d]tbl_offset_B = %d, RGB_Offset.B_offset = %d\n", __FUNCTION__, __LINE__, tbl_offset_B, drv_vipCSMatrix_t.RGB_Offset.B_offset);

#ifdef ENABLE_xvYcc
	/* sync YUV2RGB flow*/
#endif

	/*Clamp setting*/
	/*drv_vipCSMatrix_t.CTRL_ITEM.Y_Clamp = Y_Clamp;*/
	drv_vipCSMatrix_t.CTRL_ITEM.Y_Clamp = system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.CTRL_ITEM.Y_Clamp;

	if (display == SLR_MAIN_DISPLAY) {
		/*share memory setting*/
#ifdef CONFIG_ARM64
		memcpy_toio(&system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y, &drv_vipCSMatrix_t.COEF_By_Y, sizeof(DRV_VIP_YUV2RGB_COEF_By_Y));
		memcpy_toio(&system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset, &drv_vipCSMatrix_t.RGB_Offset, sizeof(DRV_VIP_YUV2RGB_RGB_Offset));
#else
		memcpy(&system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y, &drv_vipCSMatrix_t.COEF_By_Y, sizeof(DRV_VIP_YUV2RGB_COEF_By_Y));
		memcpy(&system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset, &drv_vipCSMatrix_t.RGB_Offset, sizeof(DRV_VIP_YUV2RGB_RGB_Offset));
#endif
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K11[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K12[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K13[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K22[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K23[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K32[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K33[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT32Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset.R_offset), 1, 0);
		fwif_color_ChangeUINT32Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset.G_offset), 1, 0);
		fwif_color_ChangeUINT32Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset.B_offset), 1, 0);
		/*drv setting*/
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Y_Clamp, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_Y_Only, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Offset_Only, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		/* bt2020_Constant_En,  for bt 2020 k13, k32*/
		if (bt2020_Constant_En == 1) {
			/*drv setting*/
			drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_BT2020_Only, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
			/*memory setting*/
			fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.bt2020_K13), 1, 0);
			fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.bt2020_K32), 1, 0);
		}
	}
/*#ifdef VIP_CONFIG_DUAL_CHENNEL*/
	else {
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Y_Clamp, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_Y_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Offset_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		/* bt2020_Constant_En,  for bt 2020 k13, k32*/
		if (bt2020_Constant_En == 1)
			drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_BT2020_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
	}
/*#endif*/
	return;

}
/*extern UINT8 OPC_contrastbrightness = 0; */ /*add by lichun_zhao for OPC*/
/**
 * fwif_color_setContrast
 * Setting contrast control
 *
 * @param { display: Main or sub }
 * @return { none }
 *
 */
void fwif_color_setContrast(unsigned char src_idx, unsigned char display, unsigned char value)
{
	unsigned char Y_Clamp, osd_contrast, contrast;
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	static unsigned char first_con=0;
	osd_contrast = value;

	fwif_color_CalContrast_Gain(value, &contrast, &Y_Clamp);
	if (OSD_Contrast_Compensation) {
		fwif_color_CalContrast_Compensation(display, contrast, VIP_system_info_structure_table);
	}

	if (display == SLR_MAIN_DISPLAY) {
		VIP_system_info_structure_table->OSD_Info.Contrast_Gain = fwif_color_ChangeOneUINT16Endian((UINT16)contrast, 0);
		VIP_system_info_structure_table->OSD_Info.OSD_Contrast = value;
	} else {
		VIP_system_info_structure_table->OSD_Info.Contrast_Gain_Sub= fwif_color_ChangeOneUINT16Endian((UINT16)contrast, 0);
		VIP_system_info_structure_table->OSD_Info.OSD_Contrast_Sub = value;
	}
#ifndef BUILD_QUICK_SHOW
	if(is_QS_pq_enable()==1){	
		if(first_con==0){ //pq_quick_show flow
			first_con =1; //first time don't do it 
		}else{	
			fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(src_idx, display, VIP_system_info_structure_table);
		}

	}else{
		fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(src_idx, display, VIP_system_info_structure_table);

	}
#else
		fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(src_idx, display, VIP_system_info_structure_table);

#endif
	
}

 #if 0
void fwif_color_setContrast(unsigned char src_idx, unsigned char display, unsigned char value)
{
	unsigned char Drv_Skip_Flag = 0;
	/*unsigned char centerMode = 0;*/

	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();

	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	/*unsigned short *table_idx;*/
	unsigned short table_idx[VIP_YUV2RGB_Y_Seg_Max];
	unsigned char i, y_idx_max, Y_Clamp;
	unsigned char contrast = 0;
	unsigned char level;
	unsigned char tbl_select;
	unsigned char Input_mode;

	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	level = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.CoefByY_CtrlItem_Level;
	tbl_select = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_TBL_Select;
	Input_mode = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode;

	if (level >= VIP_YUV2RGB_LEVEL_SELECT_MAX || tbl_select >= YUV2RGB_TBL_Num) {
		VIPprintf("~level or table select Error return, level = %d, table select = %d, %s->%d, %s~\n", level, tbl_select, __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	Drv_Skip_Flag = gVip_Table->DrvSetting_Skip_Flag[DrvSetting_Skip_Flag_coefByY];

	/* Contrast gain calculate*/
	fwif_color_CalContrast_Gain(value, &contrast, &Y_Clamp);

	y_idx_max = VIP_YUV2RGB_Y_Seg_Max;
	/*table_idx = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K11[0]);*/	/* get table from pq misc, vip table is for coef gain*/
	for (i = 0; i < y_idx_max; i++)
		table_idx[i] =  tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];

	if (display == SLR_MAIN_DISPLAY) {
#ifdef ENABLE_xvYcc
		unsigned char xvYcc_en_flag = 0;
		/*get xvYcc mode flag*/
		/*_system_setting_info *VIP_system_info_structure_table = NULL;*/
		/*VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);*/
		if ((VIP_system_info_structure_table->xvYcc_flag == 1) || (VIP_system_info_structure_table->xvYcc_auto_mode_flag == 1)) {
			xvYcc_en_flag = 1;
		} else {
			xvYcc_en_flag = 0;
		}

		VIPprintf("fwif_color_setContrast, xvYcc_en_flag =%d\n", xvYcc_en_flag);

		if (xvYcc_en_flag) {
			for (i = 0; i < y_idx_max; i++) {
				if (table_idx[i] > 0x3ff) {
					drv_vipCSMatrix_t.COEF_By_Y.K11[i] = complement2((complement2(table_idx[i]*contrast)>>7)>>1);
				} else {
					drv_vipCSMatrix_t.COEF_By_Y.K11[i] = ((table_idx[i]*contrast)>>7)>>1;
				}
				/*	set reg value to share memory.*/
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K11[i] = drv_vipCSMatrix_t.COEF_By_Y.K11[i];
				fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K11[i]), 1, 0);
			}
		} else {
			for (i = 0; i < y_idx_max; i++) {
				drv_vipCSMatrix_t.COEF_By_Y.K11[i] = (table_idx[i]*contrast)>>7;
				/*	set reg value to share memory.*/
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K11[i] = drv_vipCSMatrix_t.COEF_By_Y.K11[i];
				fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K11[i]), 1, 0);
			}
		}
#else
		/*contrast is a mapping gain.*/
		/*drv_vipCSMatrix_t.K11[VIP_CSMatrix_Index_0]= (2 * contrast) & 0x1ff;	*/ /*20140219 roger, back the coef first*/
		for (i = 0; i < y_idx_max; i++) {
			drv_vipCSMatrix_t.COEF_By_Y.K11[i] = (table_idx[i]*contrast)>>7;
			/*	set reg value to share memory.*/
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K11[i] = drv_vipCSMatrix_t.COEF_By_Y.K11[i];
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K11[i]), 1, 0);
		}
#endif
		/*o------------- contrast ------------o*/

		drv_vipCSMatrix_t.CTRL_ITEM.Y_Clamp = Y_Clamp;

		VIP_system_info_structure_table->OSD_Info.Contrast_Gain = fwif_color_ChangeOneUINT16Endian((UINT16)contrast, 0);
		VIP_system_info_structure_table->OSD_Info.OSD_Contrast = value;

		/* compensation*/
		if (OSD_Contrast_Compensation) {
			/*if (pData == NULL) {
				VIPprintf("~SLR_PICTURE_MODE_DATA = NULL, fwif_color_setContrast, Return\n~");
				return;
			}*/
			fwif_color_CalContrast_Compensation(display, contrast, VIP_system_info_structure_table);
			fwif_color_setBrightness(src_idx, display, VIP_system_info_structure_table->OSD_Info.OSD_Brightness);
			fwif_color_setSaturation(src_idx, display, VIP_system_info_structure_table->OSD_Info.OSD_Saturation);
		}
	} else {
		/*contrast is a mapping gain.*/
		/*drv_vipCSMatrix_t.K11[VIP_CSMatrix_Index_0]= (2 * contrast) & 0x1ff;	*/ /*20140219 roger, back the coef first*/
		for (i = 0; i < y_idx_max; i++) {
			drv_vipCSMatrix_t.COEF_By_Y.K11[i] = (table_idx[i]*contrast)>>7;
		}
		/*o------------- contrast ------------o*/

		drv_vipCSMatrix_t.CTRL_ITEM.Y_Clamp = Y_Clamp;

		VIP_system_info_structure_table->OSD_Info.Contrast_Gain_Sub= fwif_color_ChangeOneUINT16Endian((UINT16)contrast, 0);
		VIP_system_info_structure_table->OSD_Info.OSD_Contrast_Sub= value;

		/* compensation*/
		if (OSD_Contrast_Compensation) {
			/*if (pData == NULL) {
				VIPprintf("~SLR_PICTURE_MODE_DATA = NULL, fwif_color_setContrast, Return\n~");
				return;
			}*/
			fwif_color_CalContrast_Compensation(display, contrast, VIP_system_info_structure_table);
			fwif_color_setBrightness(src_idx, display, VIP_system_info_structure_table->OSD_Info.OSD_Brightness_Sub);
			fwif_color_setSaturation(src_idx, display, VIP_system_info_structure_table->OSD_Info.OSD_Saturation_Sub);
		}
	}

	/* skip driver setting*/
	if (Drv_Skip_Flag == 1)
		return;

	if (display == SLR_MAIN_DISPLAY) {
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Y_Clamp, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_Y_Only, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	}
	else {
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Y_Clamp, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_Y_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
	}
}
#endif

void fwif_color_CalContrast_Gain(unsigned char OSD_Contrast, unsigned char *Contrast_Gain, unsigned char *Y_Clamp)
{
	unsigned char briConIdx = 0;
	unsigned char contrast = 0;
	UINT16 temp;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = NULL;
	UINT8 vip_source;
	/*VIP_QUALITY_Extend2_Coef* pVipCoefArray = NULL;*/
	unsigned char *pVipCoefArray = NULL;
	UINT8 csp_control;

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if (VIP_system_info_structure_table == NULL || VIP_RPC_system_info_structure_table == NULL) {
		/*VIPprintf("~VIP_system_info_structure_table||pVipCoefArray = NULL, fwif_color_CalContrast_Gain, Return\n~");*/
		VIPprintf("~table NULL, [%s:%d], Return\n~", __FILE__, __LINE__);
		return;
	}
	vip_source = VIP_RPC_system_info_structure_table->VIP_source;

	/*pVipCoefArray = fwif_color_get_VIP_Extend2_CoefArray(vip_source);*/
	pVipCoefArray = (unsigned char *)fwif_color_get_VIP_Extend2_CoefArray(vip_source);
	if (pVipCoefArray == NULL) {
		/*VIPprintf("~VIP_system_info_structure_table||pVipCoefArray = NULL, fwif_color_CalContrast_Gain, Return\n~");*/
		VIPprintf("~table NULL, [%s:%d], Return\n~", __FILE__, __LINE__);
		return;
	}
	/*csp_control = pVipCoefArray->ColorSpace_control;*/
	csp_control = pVipCoefArray[VIP_QUALITY_FUNCTION_ColorSpace_Control];
	printk("csp_control = %d, vip_source = %d\n", csp_control, vip_source);

	/*o------------- contrast ------------o*/
	briConIdx = OSD_Contrast;

	temp = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_CONTRAST, briConIdx);
	if (temp > 255)
		contrast = 255;
	else
		contrast = (UINT8)(temp);

#ifdef CONFIG_HDR_DEMO
#else
	/*20140220 roger for new mode START================================*/
	if (csp_control == RGB2YUV_OUT_DATA_16_235) {
		if (contrast < (255 - 21))
			contrast = contrast + 21;
		else
			contrast = 255;
		*Y_Clamp = 1;
	} else {
		*Y_Clamp = 0;
	}
	/*20140220 roger for new mode END================================*/
	VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.CTRL_ITEM.Y_Clamp = *Y_Clamp;
#endif

	*Contrast_Gain = contrast;

	printk("fwif_color_setContrast:::OSD_contrast = %d, contrast = %d\n", briConIdx, contrast);

}

void fwif_color_CalContrast_Compensation(unsigned char display, unsigned char Contrast_Gain, _system_setting_info *system_info_struct)
{
	short sat_cmps_tmp, bri_cmps_tmp;
	unsigned short sat_cmps_gain = 128, bri_cmps_gain = 128;

	if (system_info_struct == NULL) {
		VIPprintf("~table NULL TBL=%lx,[%s:%d], Return\n~", (unsigned long)system_info_struct, __FILE__, __LINE__);
		return;
	}

	/* center is always at 128*/
	/*o-----------Saturation Compensation-------------o*/
 	/*matrix is 16~235/16~240 to 16~235, so sat_cmps_gain = 128.*/ /*y:16-236, C:16-240, (235-16)/(240-16) ~= 0.9777 ~= (125>>7)*/ /*change "sat_cmps_gain" to adjust compensation effect*/
	sat_cmps_tmp = (Contrast_Gain * sat_cmps_gain) >> 7;

	/*o-----------Brightness Compensation-------------o*/
	if ((fwif_color_ChangeOneUINT16Endian(system_info_struct->RGB2YUV_COEF_CSMatrix.Y_gain, 0) != 0x0100) ||
		(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.CTRL_ITEM.Y_Clamp == 1) ||
		(system_info_struct->HDMI_data_range == MODE_RAG_FULL) || (system_info_struct->HDMI_video_format == COLOR_RGB) ||
		(system_info_struct->Input_src_Type == _SRC_VGA))
		bri_cmps_tmp = 0;
	else {
		bri_cmps_tmp = 64 - ((64*Contrast_Gain)>>7); /* calculted in 10 bit*/
		bri_cmps_tmp = (bri_cmps_tmp*bri_cmps_gain)>>7; /*adjust compensation by bri_cmps_gain.*/
	}
	/*pr_emerg("bri_cmps_tmp =%d, Y_gain =%d ,Y_Clamp =%d,HDMI_data_range=%d ,HDMI_video_format =%d , Input_src_Type=%d\n ",
		bri_cmps_tmp,
		system_info_struct->RGB2YUV_COEF_CSMatrix.Y_gain,
		system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.CTRL_ITEM.Y_Clamp,
		system_info_struct->HDMI_data_range,
		system_info_struct->HDMI_video_format,
		system_info_struct->Input_src_Type
		);
	*/

	if (display == SLR_MAIN_DISPLAY) {
		system_info_struct->OSD_Info.Saturation_Cmps = fwif_color_ChangeOneUINT16Endian(sat_cmps_tmp, 0);
		system_info_struct->OSD_Info.Brightness_Cmps = fwif_color_ChangeOneUINT16Endian(bri_cmps_tmp, 0);
	} else {
		system_info_struct->OSD_Info.Saturation_Cmps_Sub = fwif_color_ChangeOneUINT16Endian(sat_cmps_tmp, 0);
		system_info_struct->OSD_Info.Brightness_Cmps_Sub = fwif_color_ChangeOneUINT16Endian(bri_cmps_tmp, 0);
	}

	VIPprintf("~ch=%d,Contrast_Gain=%d, Saturation_Cmps=%d,sat_cmps_tmp=%d, Brightness_Cmps=%d, bri_cmps_tmp=%d,\n~",
		display, Contrast_Gain, system_info_struct->OSD_Info.Saturation_Cmps, sat_cmps_tmp,
		system_info_struct->OSD_Info.Brightness_Cmps, bri_cmps_tmp);
}


void fwif_color_SetDataFormatHandler(SCALER_DISP_CHANNEL display, unsigned short mode, unsigned short channel, unsigned char en_422to444,
	unsigned char InputSrcGetType)
{
	unsigned char nMode = 0;
	_system_setting_info *system_info_struct = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	unsigned char is_hdr_block_used;
	
	is_hdr_block_used = check_hdr_block_use(display);
	if (is_hdr_block_used == 0)		
		drvif_color_set422to444(channel, en_422to444);
	else
		drvif_color_set422to444(channel, 0);

	rtd_pr_vpq_info("DataFormatHandler,display=%d,is_hdr_block_used=%d,en_422to444=%d,\n",
		display, is_hdr_block_used, en_422to444);

	if (InputSrcGetType == _SRC_YPBPR) {
		if ((mode !=  _MODE_480I)
			&& (mode !=  _MODE_480P)
			&& (mode !=  _MODE_576I)
			&& (mode !=  _MODE_576P))
			nMode = 1;
	} else if ( InputSrcGetType == _SRC_VO || InputSrcGetType == _SRC_HDMI || InputSrcGetType == _SRC_DISPLAYPORT ) {
		if ((mode !=  _MODE_480I)
			&& (mode !=  _MODE_480P)
			&& (mode !=  _MODE_576I)
			&& (mode !=  _MODE_576P))
			nMode = 1;
	}

/*	VIPprintf("Enter Set DataFormat Handler, info->IPH_ACT_WID  = %d\n", info->IPV_ACT_LEN);*/
/*#if !TEST_PATTERN_GEN*/
	//down(&I_RGB2YUV_Semaphore);	// could not use semaphore in seamless flow
	fwif_color_colorspacergb2yuvtransfer(channel, InputSrcGetType, nMode, 1);
	//up(&I_RGB2YUV_Semaphore);
/*#endif*/
	/*drvif_color_colorspaceyuv2rgbtransfer(display, InputSrcGetType, 1);*/	 /*Erin 20100415 for compile temporarily*/
	/*initialization in quality handler, mark thiss function. elieli*/
	/*fwif_color_colorspaceyuv2rgbtransfer(display, InputSrcGetType, 1);*/
	/* update YUV2RGB for driver base*/
if(is_QS_pq_enable()==0){//current is QS flow	pq_quick_show flow
	fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(0, display, system_info_struct);
}
	g_nSD_HD_mode = nMode;/* for WCG */
	Scaler_Set_ColorMapping_By_ColorFormat(channel, nMode);

}

void fwif_color_setBrightness(unsigned char src_idx, unsigned char display, unsigned char value)
{
	unsigned char bri, osd_bri;
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	static unsigned char first_bri=0;
	osd_bri = value;

	bri = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_BRIGHTNESS, osd_bri);
	if (bri > 255)
		bri = 255;
	else
		bri = (UINT8)(bri);

	if (display == SLR_MAIN_DISPLAY) {
		VIP_system_info_structure_table->OSD_Info.OSD_Brightness = value;
		VIP_system_info_structure_table->OSD_Info.Brightness_Gain = fwif_color_ChangeOneUINT16Endian((UINT16)bri, 0);
	} else {
		VIP_system_info_structure_table->OSD_Info.OSD_Brightness_Sub= value;
		VIP_system_info_structure_table->OSD_Info.Brightness_Gain_Sub = fwif_color_ChangeOneUINT16Endian((UINT16)bri, 0);
	}
#ifndef BUILD_QUICK_SHOW	
	if(is_QS_pq_enable()==1){	
		if(first_bri==0){ //pq_quick_show 
			first_bri =1; //first time don't do it 
		}else{	
			fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(src_idx, display, VIP_system_info_structure_table);
		}
	}else{
		fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(src_idx, display, VIP_system_info_structure_table);
	}
#else
		fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(src_idx, display, VIP_system_info_structure_table);


#endif 
}

#if 0
void fwif_color_setBrightness(unsigned char src_idx, unsigned char display, unsigned char value)
{
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	unsigned char briConIdx = 0;
	unsigned char brightness = 0;
	UINT16 temp;
	short cmps = 0;
	unsigned char Drv_Skip_Flag = 0;
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();

	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	Drv_Skip_Flag = gVip_Table->DrvSetting_Skip_Flag[DrvSetting_Skip_Flag_coefByY];

	if (display == SLR_MAIN_DISPLAY) {
		VIP_system_info_structure_table->OSD_Info.OSD_Brightness = value;
		/*o------------- brightness ------------o*/
		briConIdx = value;

		temp = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_BRIGHTNESS, briConIdx);
		if (temp > 255)
			brightness = 255;
		else
			brightness = (UINT8)(temp);

		VIPprintf("fwif_color_setBrightness:::OSD_brightness = %d, brightness = %d\n", briConIdx, brightness);

		/*compensation*/
		if (OSD_Contrast_Compensation)
			cmps = fwif_color_ChangeOneUINT16Endian(VIP_system_info_structure_table->OSD_Info.Brightness_Cmps, 0);
		else
			cmps = 0;
		VIPprintf("fwif_color_setBrightness:::com_brightness = %d, Brightness_Cmps = %d\n", brightness, VIP_system_info_structure_table->OSD_Info.Brightness_Cmps);

#ifdef ENABLE_xvYcc
		unsigned char xvYcc_en_flag = 0;
		/*get xvYcc mode flag*/
		/*_system_setting_info *VIP_system_info_structure_table = NULL;*/
		/*VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);*/
		if ((VIP_system_info_structure_table->xvYcc_flag == 1) || (VIP_system_info_structure_table->xvYcc_auto_mode_flag == 1)) {
			xvYcc_en_flag = 1;
		} else {
			xvYcc_en_flag = 0;
		}

		VIPprintf("fwif_color_setBrightness, xvYcc_en_flag =%d\n", xvYcc_en_flag);
		VIPprintf("offset =%d\n", (((brightness-128) << 5) + (cmps << 4)));


		if (xvYcc_en_flag) {
			drv_vipCSMatrix_t.RGB_Offset.R_offset = ((((brightness-128) << 5) + (cmps << 4)) + 6144) & 0x1ffff;
			drv_vipCSMatrix_t.RGB_Offset.G_offset = ((((brightness-128) << 5) + (cmps << 4)) + 6144) & 0x1ffff;
			drv_vipCSMatrix_t.RGB_Offset.B_offset = ((((brightness-128) << 5) + (cmps << 4)) + 6144) & 0x1ffff;
		}

		VIPprintf("fwif_color_setBrightness, xvYcc_en_flag =%d\n", xvYcc_en_flag);
		/*VIPprintf("2.R_offset =%d\n", (drv_vipCSMatrix_t.RGB_Offset.R_offset));*/
		/*VIPprintf("2.G_offset =%d\n", (drv_vipCSMatrix_t.RGB_Offset.G_offset));*/
		/*VIPprintf("2.B_offset =%d\n", (drv_vipCSMatrix_t.RGB_Offset.B_offset));*/
#else
		/*drv_vipCSMatrix_t.RGB_Offset.R_offset = ((brightness-128) << 5) & 0x1ffff; */ /*20140220 roger for sync the brightness step with Mac, Magellen	*/ /*modified by Flora, for Sirius; re-modified by Elsie 20140205*/
		/*drv_vipCSMatrix_t.RGB_Offset.G_offset = ((brightness-128) << 5) & 0x1ffff;*/
		/*drv_vipCSMatrix_t.RGB_Offset.B_offset = ((brightness-128) << 5) & 0x1ffff;*/
		drv_vipCSMatrix_t.RGB_Offset.R_offset = (((brightness-128) << 5) + (cmps << 4)) & 0x1ffff; /*20140220 roger for sync the brightness step with Mac, Magellen	*/ /*modified by Flora, for Sirius; re-modified by Elsie 20140205*/
		drv_vipCSMatrix_t.RGB_Offset.G_offset = (((brightness-128) << 5) + (cmps << 4)) & 0x1ffff;	/* cmps: 4bit fraction*/
		drv_vipCSMatrix_t.RGB_Offset.B_offset = (((brightness - 128) << 5) + (cmps << 4)) & 0x1ffff;

#endif

		/*	set reg value to share memory.*/
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset.R_offset = drv_vipCSMatrix_t.RGB_Offset.R_offset;
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset.G_offset = drv_vipCSMatrix_t.RGB_Offset.G_offset;
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset.B_offset = drv_vipCSMatrix_t.RGB_Offset.B_offset;
		fwif_color_ChangeUINT32Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset.R_offset), 3, 0);
	} else {
		VIP_system_info_structure_table->OSD_Info.OSD_Brightness_Sub= value;
		/*o------------- brightness ------------o*/
		briConIdx = value;

		temp = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_BRIGHTNESS, briConIdx);
		if (temp > 255)
			brightness = 255;
		else
			brightness = (UINT8)(temp);

		VIPprintf("Sub fwif_color_setBrightness:::OSD_brightness = %d, brightness = %d\n", briConIdx, brightness);

		/*compensation*/
		if (OSD_Contrast_Compensation)
			cmps = fwif_color_ChangeOneUINT16Endian(VIP_system_info_structure_table->OSD_Info.Brightness_Cmps_Sub, 0);
		else
			cmps = 0;
		VIPprintf("Sub fwif_color_setBrightness:::com_brightness = %d, Brightness_Cmps = %d\n", brightness, VIP_system_info_structure_table->OSD_Info.Brightness_Cmps_Sub);

		/*drv_vipCSMatrix_t.RGB_Offset.R_offset = ((brightness-128) << 5) & 0x1ffff; */ /*20140220 roger for sync the brightness step with Mac, Magellen	*/ /*modified by Flora, for Sirius; re-modified by Elsie 20140205*/
		/*drv_vipCSMatrix_t.RGB_Offset.G_offset = ((brightness-128) << 5) & 0x1ffff;*/
		/*drv_vipCSMatrix_t.RGB_Offset.B_offset = ((brightness-128) << 5) & 0x1ffff;*/
		drv_vipCSMatrix_t.RGB_Offset.R_offset = (((brightness-128) << 5) + (cmps << 4)) & 0x1ffff; /*20140220 roger for sync the brightness step with Mac, Magellen	*/ /*modified by Flora, for Sirius; re-modified by Elsie 20140205*/
		drv_vipCSMatrix_t.RGB_Offset.G_offset = (((brightness-128) << 5) + (cmps << 4)) & 0x1ffff;	/* cmps: 4bit fraction*/
		drv_vipCSMatrix_t.RGB_Offset.B_offset = (((brightness - 128) << 5) + (cmps << 4)) & 0x1ffff;
	}

	// skip driver setting
	if (Drv_Skip_Flag == 1)
		return;

	if (display == SLR_MAIN_DISPLAY) {
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Offset_Only, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	}
	else {
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Offset_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
	}
}
#endif

/*======== Set Con/Bri =============*/
/*===============================================================================================	//ConBri CSFC 20140128*/


/*===============================================================================================	//ConBri CSFC 20140128*/
/*======== Set hue/sat =============*/

/*
 * fwif_color_icmhuesat
 * Setting hue sat control
 *
 * @param { display: Main or sub }
 * @return { none }
 *
 */

void fwif_color_setHue(unsigned char src_idx, unsigned char display, unsigned char value)
{
	short hue = 0, deg_hue = 0;
	unsigned char osdHue = value;
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	static unsigned char first_hue=0;
	hue = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_HUE, osdHue);
	hue -=  128;
	if (hue >=  0)
		deg_hue = hue;
	else
		deg_hue = 360 - (hue * (-1));

	if (display == SLR_MAIN_DISPLAY) {
		VIP_system_info_structure_table->OSD_Info.Hue_Gain = fwif_color_ChangeOneUINT16Endian(deg_hue, 0);
		VIP_system_info_structure_table->OSD_Info.OSD_Hue = osdHue;
	} else {
		VIP_system_info_structure_table->OSD_Info.Hue_Gain_Sub= fwif_color_ChangeOneUINT16Endian(deg_hue, 0);
		VIP_system_info_structure_table->OSD_Info.OSD_Hue_Sub = osdHue;
	}
#ifndef BUILD_QUICK_SHOW	
	if(is_QS_pq_enable()==1){
		if(first_hue ==0){
			first_hue =1; //pq_quick_show
		}else{
			fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(src_idx, display, VIP_system_info_structure_table);
		}
	}else{
		fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(src_idx, display, VIP_system_info_structure_table);
	}
#else

		fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(src_idx, display, VIP_system_info_structure_table);

#endif 
}

#if 0
void fwif_color_setHue(unsigned char src_idx, unsigned char display, unsigned char value)
{
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	/*short hueBnd_deg = _HUE_MAX_Deg;*/
	unsigned short satCmps, sat;
	short hue = 0, deg_hue = 0;
	unsigned char osdHue = value;
	short satTmp[3][3], hueTmp[3][3], kTmp[3][3], kTmp1[3][3];
	unsigned char i, j, k, y_idx_max;
	/*unsigned short *table_idx[3][3];*/
	unsigned short table_idx[3][3][VIP_YUV2RGB_Y_Seg_Max];
	unsigned char level;
	unsigned char tbl_select;
	unsigned char Input_mode;
	unsigned char Drv_Skip_Flag = 0;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	level = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.CoefByY_CtrlItem_Level;
	tbl_select = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_TBL_Select;
	y_idx_max = VIP_YUV2RGB_Y_Seg_Max;
	Input_mode = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode;
	Drv_Skip_Flag = gVip_Table->DrvSetting_Skip_Flag[DrvSetting_Skip_Flag_coefByY];

	if (level >=  VIP_YUV2RGB_LEVEL_SELECT_MAX || tbl_select >=  YUV2RGB_TBL_Num) {
		VIPprintf("~level or table select Error return, level = %d, table select = %d, %s->%d, %s~\n", level, tbl_select, __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	memset(satTmp, 0, sizeof(satTmp));
	memset(hueTmp, 0, sizeof(hueTmp));
	memset(kTmp, 0, sizeof(kTmp));
	memset(kTmp1, 0, sizeof(kTmp1));

	if (display == SLR_MAIN_DISPLAY) {	/*	hue calculate..*/
		VIP_system_info_structure_table->OSD_Info.OSD_Hue = osdHue;
		hue = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_HUE, osdHue);
		hue -=  128;
		if (hue >=  0)
			/*deg_hue = (unsigned short)((hue * hueBnd_deg) >> 7);*/
			deg_hue = hue;
		else
			/*deg_hue = (unsigned short)(360 - ((hue * (-1) * hueBnd_deg) >> 7));*/
			deg_hue = 360 - (hue * (-1));
		VIP_system_info_structure_table->OSD_Info.Hue_Gain = fwif_color_ChangeOneUINT16Endian(deg_hue, 0);
		/*Euler's formula*/
		hueTmp[0][0] = 1024;
		hueTmp[1][1] = CAdjustCosine(deg_hue);
		hueTmp[1][2] = CAdjustSine(deg_hue)*(-1);
		hueTmp[2][1] = CAdjustSine(deg_hue);
		hueTmp[2][2] = CAdjustCosine(deg_hue);
		VIPprintf("fwif_color_set hue:::::osdHue = %d, Hue = %d, %d\n", osdHue, hue, deg_hue);

		/*get sat info*/
		sat = fwif_color_ChangeOneUINT16Endian(VIP_system_info_structure_table->OSD_Info.Saturation_Gain, 0);
		/*compensation*/
		if (OSD_Contrast_Compensation) {
			satCmps = fwif_color_ChangeOneUINT16Endian(VIP_system_info_structure_table->OSD_Info.Saturation_Cmps, 0);
			sat = (sat * satCmps) >> 7;
			VIPprintf("fwif_color_setSaturation:::com_Saturation = %d, Saturation_Cmps = %d\n", sat, VIP_system_info_structure_table->OSD_Info.Saturation_Cmps);
		}
		/*restrict for sat, refernece to spec for k32.*/
		sat = (sat > 128) ?  75 + (sat * 53 / 128) : sat;
		satTmp[0][0] = 128;	/*128  = 1*/
		satTmp[1][1] = sat;
		satTmp[2][2] = sat;

		/*get vip table info for k coeff.*/	/* get table from pq misc, vip table is for coef gain*/
		/*table_idx[0][0] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K11[0]);
		table_idx[0][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K12[0]);
		table_idx[0][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K13[0]);
		table_idx[1][0] = table_idx[0][0];
		table_idx[1][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K22[0]);
		table_idx[1][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K23[0]);
		table_idx[2][0] = table_idx[0][0];
		table_idx[2][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K32[0]);
		table_idx[2][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K33[0]);*/
		for (i = 0; i < y_idx_max; i++) {
			table_idx[0][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
			table_idx[0][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K12];
			table_idx[0][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K13];
			table_idx[1][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
			table_idx[1][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K22];
			table_idx[1][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K23];
			table_idx[2][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
			table_idx[2][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K32];
			table_idx[2][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K33];
		}

#ifdef ENABLE_xvYcc
		unsigned char xvYcc_en_flag = 0;
		/*get xvYcc mode flag*/
		/*_system_setting_info *VIP_system_info_structure_table = NULL;*/
		/*VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);*/
		if ((VIP_system_info_structure_table->xvYcc_flag == 1) || (VIP_system_info_structure_table->xvYcc_auto_mode_flag == 1)) {
			xvYcc_en_flag = 1;
		} else {
			xvYcc_en_flag = 0;
		}

		VIPprintf("fwif_color_setHue, xvYcc_en_flag =%d\n", xvYcc_en_flag);

		for (i = 0; i < y_idx_max; i++) {
			for (j = 0; j < 3; j++) {
				for (k = 0; k < 3; k++) {
					kTmp[j][k] = table_idx[j][k][i];
					/* bigger than 0x3FF =>neg*/
					if (kTmp[j][k] > 0x3FF) {
						kTmp[j][k] = complement2(kTmp[j][k]);
						kTmp[j][k] = kTmp[j][k] * (-1);
					}
				}
			}

			/* 3x3 matrix multiply*/
			for (j = 0; j < 3; j++)
				for (k = 0; k < 3; k++)
					kTmp1[j][k] = (kTmp[j][0] * hueTmp[0][k] + kTmp[j][1] * hueTmp[1][k] + kTmp[j][2] * hueTmp[2][k])>>10;

			for (j = 0; j < 3; j++) {
				for (k = 0; k < 3; k++) {
					kTmp[j][k] = (kTmp1[j][0] * satTmp[0][k] + kTmp1[j][1] * satTmp[1][k] + kTmp1[j][2] * satTmp[2][k])>>7;
					if (xvYcc_en_flag) {
						kTmp[j][k] = kTmp[j][k]/2;
					}
				}
			}

			drv_vipCSMatrix_t.COEF_By_Y.K12[i] = kTmp[0][1];
			drv_vipCSMatrix_t.COEF_By_Y.K13[i] = kTmp[0][2];
			drv_vipCSMatrix_t.COEF_By_Y.K22[i] = kTmp[1][1];
			drv_vipCSMatrix_t.COEF_By_Y.K23[i] = kTmp[1][2];
			drv_vipCSMatrix_t.COEF_By_Y.K32[i] = kTmp[2][1];
			drv_vipCSMatrix_t.COEF_By_Y.K33[i] = kTmp[2][2];

			/*	set reg value to share memory.*/
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K12[i] = drv_vipCSMatrix_t.COEF_By_Y.K12[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K13[i] = drv_vipCSMatrix_t.COEF_By_Y.K13[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K22[i] = drv_vipCSMatrix_t.COEF_By_Y.K22[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K23[i] = drv_vipCSMatrix_t.COEF_By_Y.K23[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K32[i] = drv_vipCSMatrix_t.COEF_By_Y.K32[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K33[i] = drv_vipCSMatrix_t.COEF_By_Y.K33[i];
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K12[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K13[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K22[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K23[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K32[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K33[i]), 1, 0);
		}
#else
		for (i = 0; i < y_idx_max; i++) {
			for (j = 0; j < 3; j++) {
				for (k = 0; k < 3; k++) {
					kTmp[j][k] = table_idx[j][k][i];
					/*bigger than 0x3FF  = >neg*/
					if (kTmp[j][k] > 0x3FF) {
						kTmp[j][k] = complement2(kTmp[j][k]);
						kTmp[j][k] = kTmp[j][k] * (-1);
					}
				}
			}

			/*3x3 matrix multiply*/
			for (j = 0; j <  3; j++)
				for (k = 0; k < 3; k++)
					kTmp1[j][k] = (kTmp[j][0] * hueTmp[0][k] + kTmp[j][1] * hueTmp[1][k] + kTmp[j][2] * hueTmp[2][k]) >> 10;

			for (j = 0; j < 3; j++)
				for (k = 0; k  < 3; k++)
					kTmp[j][k] = (kTmp1[j][0] * satTmp[0][k] + kTmp1[j][1] * satTmp[1][k] + kTmp1[j][2] * satTmp[2][k]) >> 7;

			drv_vipCSMatrix_t.COEF_By_Y.K12[i] = kTmp[0][1];
			drv_vipCSMatrix_t.COEF_By_Y.K13[i] = kTmp[0][2];
			drv_vipCSMatrix_t.COEF_By_Y.K22[i] = kTmp[1][1];
			drv_vipCSMatrix_t.COEF_By_Y.K23[i] = kTmp[1][2];
			drv_vipCSMatrix_t.COEF_By_Y.K32[i] = kTmp[2][1];
			drv_vipCSMatrix_t.COEF_By_Y.K33[i] = kTmp[2][2];

			/*	set reg value to share memory.*/
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K12[i] = drv_vipCSMatrix_t.COEF_By_Y.K12[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K13[i] = drv_vipCSMatrix_t.COEF_By_Y.K13[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K22[i] = drv_vipCSMatrix_t.COEF_By_Y.K22[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K23[i] = drv_vipCSMatrix_t.COEF_By_Y.K23[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K32[i] = drv_vipCSMatrix_t.COEF_By_Y.K32[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K33[i] = drv_vipCSMatrix_t.COEF_By_Y.K33[i];
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K12[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K13[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K22[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K23[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K32[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K33[i]), 1, 0);
		}
#endif
	} else {	/*	hue calculate..*/
		VIP_system_info_structure_table->OSD_Info.OSD_Hue_Sub = osdHue;
		hue = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_HUE, osdHue);
		hue -=  128;
		if (hue >=  0)
			/*deg_hue = (unsigned short)((hue * hueBnd_deg) >> 7);*/
			deg_hue = hue;
		else
			/*deg_hue = (unsigned short)(360 - ((hue * (-1) * hueBnd_deg) >> 7));*/
			deg_hue = 360 - (hue * (-1));
		VIP_system_info_structure_table->OSD_Info.Hue_Gain_Sub = fwif_color_ChangeOneUINT16Endian(deg_hue, 0);
		/*Euler's formula*/
		hueTmp[0][0] = 1024;
		hueTmp[1][1] = CAdjustCosine(deg_hue);
		hueTmp[1][2] = CAdjustSine(deg_hue)*(-1);
		hueTmp[2][1] = CAdjustSine(deg_hue);
		hueTmp[2][2] = CAdjustCosine(deg_hue);
		VIPprintf("sub fwif_color_set hue:::::osdHue = %d, Hue = %d, %d\n", osdHue, hue, deg_hue);

		/*get sat info*/
		sat = fwif_color_ChangeOneUINT16Endian(VIP_system_info_structure_table->OSD_Info.Saturation_Gain_Sub, 0);
		/*compensation*/
		if (OSD_Contrast_Compensation) {
			satCmps = fwif_color_ChangeOneUINT16Endian(VIP_system_info_structure_table->OSD_Info.Saturation_Cmps_Sub, 0);
			sat = (sat * satCmps) >> 7;
			VIPprintf("sub fwif_color_setSaturation:::com_Saturation = %d, Saturation_Cmps = %d\n", sat, VIP_system_info_structure_table->OSD_Info.Saturation_Cmps);
		}
		/*restrict for sat, refernece to spec for k32.*/
		sat = (sat > 128) ?  75 + (sat * 53 / 128) : sat;
		satTmp[0][0] = 128;	/*128  = 1*/
		satTmp[1][1] = sat;
		satTmp[2][2] = sat;

		/*get vip table info for k coeff.*/	/* get table from pq misc, vip table is for coef gain*/
		/*table_idx[0][0] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K11[0]);
		table_idx[0][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K12[0]);
		table_idx[0][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K13[0]);
		table_idx[1][0] = table_idx[0][0];
		table_idx[1][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K22[0]);
		table_idx[1][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K23[0]);
		table_idx[2][0] = table_idx[0][0];
		table_idx[2][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K32[0]);
		table_idx[2][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K33[0]);*/
		for (i = 0; i < y_idx_max; i++) {
			table_idx[0][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
			table_idx[0][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K12];
			table_idx[0][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K13];
			table_idx[1][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
			table_idx[1][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K22];
			table_idx[1][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K23];
			table_idx[2][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
			table_idx[2][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K32];
			table_idx[2][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K33];
		}

		for (i = 0; i < y_idx_max; i++) {
			for (j = 0; j < 3; j++) {
				for (k = 0; k < 3; k++) {
					kTmp[j][k] = table_idx[j][k][i];
					/*bigger than 0x3FF  = >neg*/
					if (kTmp[j][k] > 0x3FF) {
						kTmp[j][k] = complement2(kTmp[j][k]);
						kTmp[j][k] = kTmp[j][k] * (-1);
					}
				}
			}

			/*3x3 matrix multiply*/
			for (j = 0; j <  3; j++)
				for (k = 0; k < 3; k++)
					kTmp1[j][k] = (kTmp[j][0] * hueTmp[0][k] + kTmp[j][1] * hueTmp[1][k] + kTmp[j][2] * hueTmp[2][k]) >> 10;

			for (j = 0; j < 3; j++)
				for (k = 0; k  < 3; k++)
					kTmp[j][k] = (kTmp1[j][0] * satTmp[0][k] + kTmp1[j][1] * satTmp[1][k] + kTmp1[j][2] * satTmp[2][k]) >> 7;

			drv_vipCSMatrix_t.COEF_By_Y.K12[i] = kTmp[0][1];
			drv_vipCSMatrix_t.COEF_By_Y.K13[i] = kTmp[0][2];
			drv_vipCSMatrix_t.COEF_By_Y.K22[i] = kTmp[1][1];
			drv_vipCSMatrix_t.COEF_By_Y.K23[i] = kTmp[1][2];
			drv_vipCSMatrix_t.COEF_By_Y.K32[i] = kTmp[2][1];
			drv_vipCSMatrix_t.COEF_By_Y.K33[i] = kTmp[2][2];
		}
	}

	// skip driver setting
	if (Drv_Skip_Flag == 1)
		return;

	if (display == SLR_MAIN_DISPLAY) {
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	}
	else {
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
	}
}
#endif

void fwif_color_setSaturation(unsigned char src_idx, unsigned char display, unsigned char value)
{
	unsigned short sat;
	unsigned char osdSat;
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	static unsigned char first_sat =0;
	osdSat = value;
	sat = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_SATURTUION, osdSat);

	if (display == SLR_MAIN_DISPLAY) {
		VIP_system_info_structure_table->OSD_Info.Saturation_Gain = fwif_color_ChangeOneUINT16Endian(sat, 0);
		VIP_system_info_structure_table->OSD_Info.OSD_Saturation = osdSat;
	} else {
		VIP_system_info_structure_table->OSD_Info.Saturation_Gain_Sub= fwif_color_ChangeOneUINT16Endian(sat, 0);
		VIP_system_info_structure_table->OSD_Info.OSD_Saturation_Sub = osdSat;
	}
#ifndef BUILD_QUICK_SHOW	
	if(is_QS_pq_enable()==1){
		if(first_sat ==0){
			first_sat =1; //pq_quick_show
		}else{
			fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(src_idx, display, VIP_system_info_structure_table);
		}
	}else{
		fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(src_idx, display, VIP_system_info_structure_table);
	}
#else
		fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(src_idx, display, VIP_system_info_structure_table);


#endif 
}

#if 0
void fwif_color_setSaturation(unsigned char src_idx, unsigned char display, unsigned char value)
{
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	short deg_hue;
	short satTmp[3][3], hueTmp[3][3], kTmp[3][3], kTmp1[3][3];
	unsigned char i, j, k, y_idx_max;
	unsigned short sat = 0;
	UINT16 osdSat = 0, cmps;
	unsigned short table_idx[3][3][VIP_YUV2RGB_Y_Seg_Max];/*unsigned short *table_idx[3][3];*/
	unsigned char level;
	unsigned char tbl_select;
	unsigned char Input_mode;
	unsigned char Drv_Skip_Flag;

	_system_setting_info *VIP_system_info_structure_table = NULL;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	level = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.CoefByY_CtrlItem_Level;
	tbl_select = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_TBL_Select;
	y_idx_max = VIP_YUV2RGB_Y_Seg_Max;
	Input_mode = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode;
	Drv_Skip_Flag = gVip_Table->DrvSetting_Skip_Flag[DrvSetting_Skip_Flag_coefByY];

	if (level >=  VIP_YUV2RGB_LEVEL_SELECT_MAX || tbl_select >=  YUV2RGB_TBL_Num) {
		VIPprintf("~level or table select Error return, level = %d, table select = %d, %s->%d, %s~\n", level, tbl_select, __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	memset(satTmp, 0, sizeof(satTmp));
	memset(hueTmp, 0, sizeof(hueTmp));
	memset(kTmp, 0, sizeof(kTmp));
	memset(kTmp1, 0, sizeof(kTmp1));

	if (display == SLR_MAIN_DISPLAY) {
		VIP_system_info_structure_table->OSD_Info.OSD_Saturation = value;

		/*sat gain cal.*/
		osdSat = value;
		sat = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_SATURTUION, osdSat);
		VIP_system_info_structure_table->OSD_Info.Saturation_Gain = fwif_color_ChangeOneUINT16Endian(sat, 0);

		/*compensation*/
		if (OSD_Contrast_Compensation) {
			cmps = fwif_color_ChangeOneUINT16Endian(VIP_system_info_structure_table->OSD_Info.Saturation_Cmps, 0);
			sat = (sat * cmps) >> 7;
			VIPprintf("fwif_color_setSaturation:::com_Saturation = %d, Saturation_Cmps = %d\n", sat, VIP_system_info_structure_table->OSD_Info.Saturation_Cmps);
		}
		/*restrict for sat, refernece to spec for k32.*/
		sat = (sat > 128) ?  75 + (sat * 53 / 128) : sat;
		satTmp[0][0] = 128; /*128  = 1*/
		satTmp[1][1] = sat;
		satTmp[2][2] = sat;
		VIPprintf("\nfwif_color_seticmhuesat:::osdSat = %d, sat = %d\n", osdSat, sat);

		/*	get hue info..*/
		deg_hue = fwif_color_ChangeOneUINT16Endian(VIP_system_info_structure_table->OSD_Info.Hue_Gain, 0);
		/*Euler's formula*/
		hueTmp[0][0] = 1024;
		hueTmp[1][1] = CAdjustCosine(deg_hue);
		hueTmp[1][2] = CAdjustSine(deg_hue) * (-1);
		hueTmp[2][1] = CAdjustSine(deg_hue);
		hueTmp[2][2] = CAdjustCosine(deg_hue);
		VIPprintf("fwif_color_set sat:::::degHue = %d\n", deg_hue);

		/*get vip table info for k coeff.*/		/* get table from pq misc, vip table is for coef gain*/
		/*table_idx[0][0] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K11[0]);
		table_idx[0][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K12[0]);
		table_idx[0][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K13[0]);
		table_idx[1][0] = table_idx[0][0];
		table_idx[1][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K22[0]);
		table_idx[1][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K23[0]);
		table_idx[2][0] = table_idx[0][0];
		table_idx[2][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K32[0]);
		table_idx[2][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K33[0]);*/
		for (i = 0; i < y_idx_max; i++) {
			table_idx[0][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
			table_idx[0][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K12];
			table_idx[0][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K13];
			table_idx[1][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
			table_idx[1][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K22];
			table_idx[1][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K23];
			table_idx[2][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
			table_idx[2][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K32];
			table_idx[2][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K33];
		}

#ifdef ENABLE_xvYcc
		unsigned char xvYcc_en_flag = 0;
		/*get xvYcc mode flag*/
		/*_system_setting_info *VIP_system_info_structure_table = NULL;*/
		/*VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);*/
		if ((VIP_system_info_structure_table->xvYcc_flag == 1) || (VIP_system_info_structure_table->xvYcc_auto_mode_flag == 1)) {
			xvYcc_en_flag = 1;
		} else {
			xvYcc_en_flag = 0;
		}

		VIPprintf("fwif_color_setSaturation, xvYcc_en_flag =%d\n", xvYcc_en_flag);

		for (i = 0; i < y_idx_max; i++) {
			for (j = 0; j < 3; j++) {
				for (k = 0; k < 3; k++) {
					kTmp[j][k] = table_idx[j][k][i];
					/* bigger than 0x3FF =>neg*/
					if (kTmp[j][k] > 0x3FF) {
						kTmp[j][k] = complement2(kTmp[j][k]);
						kTmp[j][k] = kTmp[j][k] * (-1);
					}
				}
			}

			/* 3x3 matrix multiply*/
			for (j = 0; j < 3; j++)
				for (k = 0; k < 3; k++)
					kTmp1[j][k] = (kTmp[j][0] * hueTmp[0][k] + kTmp[j][1] * hueTmp[1][k] + kTmp[j][2] * hueTmp[2][k]) >> 10;

			for (j = 0; j < 3; j++) {
				for (k = 0; k < 3; k++) {
					kTmp[j][k] = (kTmp1[j][0] * satTmp[0][k] + kTmp1[j][1] * satTmp[1][k] + kTmp1[j][2] * satTmp[2][k]) >> 7;
					if (xvYcc_en_flag) {
						kTmp[j][k] = kTmp[j][k]/2;
					}
				}
			}

			drv_vipCSMatrix_t.COEF_By_Y.K12[i] = kTmp[0][1];
			drv_vipCSMatrix_t.COEF_By_Y.K13[i] = kTmp[0][2];
			drv_vipCSMatrix_t.COEF_By_Y.K22[i] = kTmp[1][1];
			drv_vipCSMatrix_t.COEF_By_Y.K23[i] = kTmp[1][2];
			drv_vipCSMatrix_t.COEF_By_Y.K32[i] = kTmp[2][1];
			drv_vipCSMatrix_t.COEF_By_Y.K33[i] = kTmp[2][2];

			/*	set reg value to share memory.*/
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K12[i] = drv_vipCSMatrix_t.COEF_By_Y.K12[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K13[i] = drv_vipCSMatrix_t.COEF_By_Y.K13[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K22[i] = drv_vipCSMatrix_t.COEF_By_Y.K22[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K23[i] = drv_vipCSMatrix_t.COEF_By_Y.K23[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K32[i] = drv_vipCSMatrix_t.COEF_By_Y.K32[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K33[i] = drv_vipCSMatrix_t.COEF_By_Y.K33[i];
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K12[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K13[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K22[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K23[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K32[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K33[i]), 1, 0);
		}
#else
		for (i = 0; i < y_idx_max; i++) {
			for (j = 0; j < 3; j++) {
				for (k = 0; k < 3; k++) {
					kTmp[j][k] = table_idx[j][k][i];
					/* bigger than 0x3FF =>neg*/
					if (kTmp[j][k] > 0x3FF) {
						kTmp[j][k] = complement2(kTmp[j][k]);
						kTmp[j][k] = kTmp[j][k] * (-1);
					}
				}
			}

			/* 3x3 matrix multiply*/
			for (j = 0; j < 3; j++)
				for (k = 0; k < 3; k++)
					kTmp1[j][k] = (kTmp[j][0] * hueTmp[0][k] + kTmp[j][1] * hueTmp[1][k] + kTmp[j][2] * hueTmp[2][k])>>10;

			for (j = 0; j < 3; j++)
				for (k = 0; k < 3; k++)
					kTmp[j][k] = (kTmp1[j][0] * satTmp[0][k] + kTmp1[j][1] * satTmp[1][k] + kTmp1[j][2] * satTmp[2][k])>>7;

			drv_vipCSMatrix_t.COEF_By_Y.K12[i] = kTmp[0][1];
			drv_vipCSMatrix_t.COEF_By_Y.K13[i] = kTmp[0][2];
			drv_vipCSMatrix_t.COEF_By_Y.K22[i] = kTmp[1][1];
			drv_vipCSMatrix_t.COEF_By_Y.K23[i] = kTmp[1][2];
			drv_vipCSMatrix_t.COEF_By_Y.K32[i] = kTmp[2][1];
			drv_vipCSMatrix_t.COEF_By_Y.K33[i] = kTmp[2][2];

			/*	set reg value to share memory.*/
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K12[i] = drv_vipCSMatrix_t.COEF_By_Y.K12[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K13[i] = drv_vipCSMatrix_t.COEF_By_Y.K13[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K22[i] = drv_vipCSMatrix_t.COEF_By_Y.K22[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K23[i] = drv_vipCSMatrix_t.COEF_By_Y.K23[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K32[i] = drv_vipCSMatrix_t.COEF_By_Y.K32[i];
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K33[i] = drv_vipCSMatrix_t.COEF_By_Y.K33[i];
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K12[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K13[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K22[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K23[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K32[i]), 1, 0);
			fwif_color_ChangeUINT16Endian(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K33[i]), 1, 0);
		}
#endif
	} else {
		VIP_system_info_structure_table->OSD_Info.OSD_Saturation_Sub= value;

		/*sat gain cal.*/
		osdSat = value;
		sat = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_SATURTUION, osdSat);
		VIP_system_info_structure_table->OSD_Info.Saturation_Gain_Sub= fwif_color_ChangeOneUINT16Endian(sat, 0);

		/*compensation*/
		if (OSD_Contrast_Compensation) {
			cmps = fwif_color_ChangeOneUINT16Endian(VIP_system_info_structure_table->OSD_Info.Saturation_Cmps_Sub, 0);
			sat = (sat * cmps) >> 7;
			VIPprintf("fwif_color_setSaturation:::com_Saturation = %d, Saturation_Cmps = %d\n", sat, VIP_system_info_structure_table->OSD_Info.Saturation_Cmps_Sub);
		}
		/*restrict for sat, refernece to spec for k32.*/
		sat = (sat > 128) ?  75 + (sat * 53 / 128) : sat;
		satTmp[0][0] = 128; /*128  = 1*/
		satTmp[1][1] = sat;
		satTmp[2][2] = sat;
		VIPprintf("sub fwif_color_seticmhuesat:::osdSat = %d, sat = %d\n", osdSat, sat);

		/*	get hue info..*/
		deg_hue = fwif_color_ChangeOneUINT16Endian(VIP_system_info_structure_table->OSD_Info.Hue_Gain_Sub, 0);
		/*Euler's formula*/
		hueTmp[0][0] = 1024;
		hueTmp[1][1] = CAdjustCosine(deg_hue);
		hueTmp[1][2] = CAdjustSine(deg_hue) * (-1);
		hueTmp[2][1] = CAdjustSine(deg_hue);
		hueTmp[2][2] = CAdjustCosine(deg_hue);
		VIPprintf("sub fwif_color_set sat:::::degHue = %d\n", deg_hue);

		/*get vip table info for k coeff.*/		/* get table from pq misc, vip table is for coef gain*/
		/*table_idx[0][0] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K11[0]);
		table_idx[0][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K12[0]);
		table_idx[0][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K13[0]);
		table_idx[1][0] = table_idx[0][0];
		table_idx[1][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K22[0]);
		table_idx[1][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K23[0]);
		table_idx[2][0] = table_idx[0][0];
		table_idx[2][1] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K32[0]);
		table_idx[2][2] = &(gVip_Table->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[level].COEF_By_Y.K33[0]);*/
		for (i = 0; i < y_idx_max; i++) {
			table_idx[0][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
			table_idx[0][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K12];
			table_idx[0][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K13];
			table_idx[1][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
			table_idx[1][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K22];
			table_idx[1][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K23];
			table_idx[2][0][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K11];
			table_idx[2][1][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K32];
			table_idx[2][2][i] = tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K33];
		}

		for (i = 0; i < y_idx_max; i++) {
			for (j = 0; j < 3; j++) {
				for (k = 0; k < 3; k++) {
					kTmp[j][k] = table_idx[j][k][i];
					/* bigger than 0x3FF =>neg*/
					if (kTmp[j][k] > 0x3FF) {
						kTmp[j][k] = complement2(kTmp[j][k]);
						kTmp[j][k] = kTmp[j][k] * (-1);
					}
				}
			}

			/* 3x3 matrix multiply*/
			for (j = 0; j < 3; j++)
				for (k = 0; k < 3; k++)
					kTmp1[j][k] = (kTmp[j][0] * hueTmp[0][k] + kTmp[j][1] * hueTmp[1][k] + kTmp[j][2] * hueTmp[2][k])>>10;

			for (j = 0; j < 3; j++)
				for (k = 0; k < 3; k++)
					kTmp[j][k] = (kTmp1[j][0] * satTmp[0][k] + kTmp1[j][1] * satTmp[1][k] + kTmp1[j][2] * satTmp[2][k])>>7;

			drv_vipCSMatrix_t.COEF_By_Y.K12[i] = kTmp[0][1];
			drv_vipCSMatrix_t.COEF_By_Y.K13[i] = kTmp[0][2];
			drv_vipCSMatrix_t.COEF_By_Y.K22[i] = kTmp[1][1];
			drv_vipCSMatrix_t.COEF_By_Y.K23[i] = kTmp[1][2];
			drv_vipCSMatrix_t.COEF_By_Y.K32[i] = kTmp[2][1];
			drv_vipCSMatrix_t.COEF_By_Y.K33[i] = kTmp[2][2];
		}
	}

	// skip driver setting
	if (Drv_Skip_Flag==1)
		return;

	if (display==SLR_MAIN_DISPLAY) {
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	} else {
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
	}
}
#endif

void fwif_color_set_ColorMap_3x3_Table(unsigned char idx)
{
	SLR_VIP_TABLE *pVipTable = fwif_colo_get_AP_vip_table_gVIP_Table();
	extern short ColorMap_Matrix_Apply[3][3];

	if (pVipTable == NULL)
		return;

	if(idx>=Color_Map_Table_NUM) {
		drvif_color_set_Color_Mapping_Enable(SLR_MAIN_DISPLAY, 0);
	} else {
		memcpy(ColorMap_Matrix_Apply, pVipTable->VIP_Color_Mapping_Matrix[idx], sizeof(ColorMap_Matrix_Apply));
		fwif_color_ColorMap_SetMatrix(SLR_MAIN_DISPLAY, ColorMap_Matrix_Apply, 1, 1, 1);//can't wait porch
	}
}

unsigned char fwif_color_icm_ini2elemtable(unsigned short *tICM_ini, unsigned char which_table, COLORELEM_TAB_T *st_icm_tab)
{
	int h, si;
	int h_sta, h_end, si_sta, si_end;
	int LayerSize;

	unsigned int baseAddr			= which_table * VIP_ICM_TBL_Y * VIP_ICM_TBL_Z;
	unsigned int baseAddr_hue_tab	= baseAddr + (2 + SATSEGMAX * ITNSEGMAX * 0) * VIP_ICM_TBL_Z;		/*hue table*/
	unsigned int baseAddr_sat_tab	= baseAddr + (2 + SATSEGMAX * ITNSEGMAX * 1) * VIP_ICM_TBL_Z;		/*sat table*/
	unsigned int baseAddr_itn_tab	= baseAddr + (2 + SATSEGMAX * ITNSEGMAX * 2) * VIP_ICM_TBL_Z;		/*itn table*/

	VIPprintf("fwif_color_icm_ini2elemtable Initialize  which_table = %d\n", which_table);

	if (which_table >= VIP_ICM_TBL_X) /*overflow*/
		return false;

	/* icm table, hue/sat/itn*/
	h_sta = 0;
	h_end = HUESEGMAX - 1;
	si_sta = 0;
	si_end = SATSEGMAX - 1;
	LayerSize = HUESEGMAX * SATSEGMAX;

	for (si = si_sta; si <= si_end; si++) {
		for (h = h_sta; h <= h_end; h++) {

			st_icm_tab->elem[0][si][h].H = tICM_ini[baseAddr_hue_tab + LayerSize * 0  + si * HUESEGMAX + h];
			st_icm_tab->elem[0][si][h].S = tICM_ini[baseAddr_sat_tab + LayerSize * 0  + si * HUESEGMAX + h];
			st_icm_tab->elem[0][si][h].I = tICM_ini[baseAddr_itn_tab + LayerSize * 0  + si * HUESEGMAX + h];
			st_icm_tab->elem[1][si][h].H = tICM_ini[baseAddr_hue_tab + LayerSize * 1  + si * HUESEGMAX + h];
			st_icm_tab->elem[1][si][h].S = tICM_ini[baseAddr_sat_tab + LayerSize * 1  + si * HUESEGMAX + h];
			st_icm_tab->elem[1][si][h].I = tICM_ini[baseAddr_itn_tab + LayerSize * 1  + si * HUESEGMAX + h];
			st_icm_tab->elem[2][si][h].H = tICM_ini[baseAddr_hue_tab + LayerSize * 2  + si * HUESEGMAX + h];
			st_icm_tab->elem[2][si][h].S = tICM_ini[baseAddr_sat_tab + LayerSize * 2  + si * HUESEGMAX + h];
			st_icm_tab->elem[2][si][h].I = tICM_ini[baseAddr_itn_tab + LayerSize * 2  + si * HUESEGMAX + h];
			st_icm_tab->elem[3][si][h].H = tICM_ini[baseAddr_hue_tab + LayerSize * 3  + si * HUESEGMAX + h];
			st_icm_tab->elem[3][si][h].S = tICM_ini[baseAddr_sat_tab + LayerSize * 3  + si * HUESEGMAX + h];
			st_icm_tab->elem[3][si][h].I = tICM_ini[baseAddr_itn_tab + LayerSize * 3  + si * HUESEGMAX + h];
			st_icm_tab->elem[4][si][h].H = tICM_ini[baseAddr_hue_tab + LayerSize * 4  + si * HUESEGMAX + h];
			st_icm_tab->elem[4][si][h].S = tICM_ini[baseAddr_sat_tab + LayerSize * 4  + si * HUESEGMAX + h];
			st_icm_tab->elem[4][si][h].I = tICM_ini[baseAddr_itn_tab + LayerSize * 4  + si * HUESEGMAX + h];
			st_icm_tab->elem[5][si][h].H = tICM_ini[baseAddr_hue_tab + LayerSize * 5  + si * HUESEGMAX + h];
			st_icm_tab->elem[5][si][h].S = tICM_ini[baseAddr_sat_tab + LayerSize * 5  + si * HUESEGMAX + h];
			st_icm_tab->elem[5][si][h].I = tICM_ini[baseAddr_itn_tab + LayerSize * 5  + si * HUESEGMAX + h];
			st_icm_tab->elem[6][si][h].H = tICM_ini[baseAddr_hue_tab + LayerSize * 6  + si * HUESEGMAX + h];
			st_icm_tab->elem[6][si][h].S = tICM_ini[baseAddr_sat_tab + LayerSize * 6  + si * HUESEGMAX + h];
			st_icm_tab->elem[6][si][h].I = tICM_ini[baseAddr_itn_tab + LayerSize * 6  + si * HUESEGMAX + h];
			st_icm_tab->elem[7][si][h].H = tICM_ini[baseAddr_hue_tab + LayerSize * 7  + si * HUESEGMAX + h];
			st_icm_tab->elem[7][si][h].S = tICM_ini[baseAddr_sat_tab + LayerSize * 7  + si * HUESEGMAX + h];
			st_icm_tab->elem[7][si][h].I = tICM_ini[baseAddr_itn_tab + LayerSize * 7  + si * HUESEGMAX + h];
			st_icm_tab->elem[8][si][h].H = tICM_ini[baseAddr_hue_tab + LayerSize * 8  + si * HUESEGMAX + h];
			st_icm_tab->elem[8][si][h].S = tICM_ini[baseAddr_sat_tab + LayerSize * 8  + si * HUESEGMAX + h];
			st_icm_tab->elem[8][si][h].I = tICM_ini[baseAddr_itn_tab + LayerSize * 8  + si * HUESEGMAX + h];
			st_icm_tab->elem[9][si][h].H = tICM_ini[baseAddr_hue_tab + LayerSize * 9  + si * HUESEGMAX + h];
			st_icm_tab->elem[9][si][h].S = tICM_ini[baseAddr_sat_tab + LayerSize * 9  + si * HUESEGMAX + h];
			st_icm_tab->elem[9][si][h].I = tICM_ini[baseAddr_itn_tab + LayerSize * 9  + si * HUESEGMAX + h];
			st_icm_tab->elem[10][si][h].H = tICM_ini[baseAddr_hue_tab + LayerSize * 10 + si * HUESEGMAX + h];
			st_icm_tab->elem[10][si][h].S = tICM_ini[baseAddr_sat_tab + LayerSize * 10 + si * HUESEGMAX + h];
			st_icm_tab->elem[10][si][h].I = tICM_ini[baseAddr_itn_tab + LayerSize * 10 + si * HUESEGMAX + h];
			st_icm_tab->elem[11][si][h].H = tICM_ini[baseAddr_hue_tab + LayerSize * 11 + si * HUESEGMAX + h];
			st_icm_tab->elem[11][si][h].S = tICM_ini[baseAddr_sat_tab + LayerSize * 11 + si * HUESEGMAX + h];
			st_icm_tab->elem[11][si][h].I = tICM_ini[baseAddr_itn_tab + LayerSize * 11 + si * HUESEGMAX + h];

		}
	}
	return true;
}

unsigned char fwif_color_icm_global_adjust_on_elem_table(COLORELEM_TAB_T *st_icm_tab_in, int g_dhue_cur, int *g_dsatbysat_cur, int *g_ditnbyitn_cur, COLORELEM_TAB_T *st_icm_tab_out)
{
	int h, s, i;
	int h_sta, h_end, s_sta, s_end, i_sta, i_end;;

	if (NULL == st_icm_tab_in || NULL == g_dsatbysat_cur || NULL == g_ditnbyitn_cur || NULL == st_icm_tab_out)
		return false;

	h_sta = 0;
	h_end = HUESEGMAX - 1;
	s_sta = 0;
	s_end = SATSEGMAX - 1;
	i_sta = 0;
	i_end = ITNSEGMAX - 1;

	for (i = i_sta; i <= i_end; i++) {
		for (s = s_sta; s <= s_end; s++) {
			for (h = h_sta; h <= h_end; h++) {

				st_icm_tab_out->elem[i][s][h].H = (st_icm_tab_in->elem[i][s][h].H + g_dhue_cur + HUERANGE) % HUERANGE;
				st_icm_tab_out->elem[i][s][h].S = CLIP(0, SATRANGEEX, st_icm_tab_in->elem[i][s][h].S + g_dsatbysat_cur[s]);
				st_icm_tab_out->elem[i][s][h].I = CLIP(0, ITNRANGEEX, st_icm_tab_in->elem[i][s][h].I + g_ditnbyitn_cur[i]);
			}
		}
	}

	return true;
}

unsigned char fwif_color_icm_global_gain_access(unsigned int access_mode, unsigned char hsi_sel, short* pValue)
{
	static short hue_gain = 128;//128 = 1
	static short sat_gain = 128;//128 = 1
	static short itn_gain = 128;//128 = 1

	if (pValue == NULL)
		return -1;
	if (access_mode == 0) {//read
		switch(hsi_sel) {
		case ICM_Global_hue:
			*pValue = hue_gain;
			break;
		case ICM_Global_Sat:
			*pValue = sat_gain;
			break;
		case ICM_Global_Itn:
			*pValue = itn_gain;
			break;
		default:
			return -1;
		}
	} else if (access_mode == 1) {//write
		switch(hsi_sel) {
		case ICM_Global_hue:
			hue_gain = *pValue;
			break;
		case ICM_Global_Sat:
			sat_gain = *pValue;
			break;
		case ICM_Global_Itn:
			itn_gain = *pValue;
			break;
		default:
			return -1;
		}
	} else if (access_mode == 2) {//reset
		hue_gain = sat_gain = itn_gain = 128;
	}

	return 0;
}

void fwif_color_icm_global_gain_calc(int *g_dhue_cur, int *g_dsatbysat_cur, int *g_ditnbyitn_cur)
{
	short gain;//128 = 1
	int i;

	// hue
	fwif_color_icm_global_gain_access(0/*read*/, ICM_Global_hue, &gain);
	*g_dhue_cur = ((*g_dhue_cur)*gain)>>7;

	// sat
	fwif_color_icm_global_gain_access(0/*read*/, ICM_Global_Sat, &gain);
	for (i = 0; i < SATSEGMAX; i++)
		g_dsatbysat_cur[i] = (g_dsatbysat_cur[i]*gain)>>7;

	// itn
	fwif_color_icm_global_gain_access(0/*read*/, ICM_Global_Itn, &gain);
	for (i = 0; i < ITNSEGMAX; i++)
		g_ditnbyitn_cur[i] = (g_ditnbyitn_cur[i]*gain)>>7;
}

void fwif_color_Icm_table_select(unsigned char display, unsigned short *tICM_ini, unsigned char which_table)
{
	/*COLORELEM_TAB_T icm_tab_elem_write;*/
	int dhue;
	int dsatbysat[SATSEGMAX];
	int ditnbyitn[ITNSEGMAX];

	/*Global Setting*/
	unsigned int baseAddr			 = which_table * VIP_ICM_TBL_Y * VIP_ICM_TBL_Z;
	unsigned int baseAddr_global	 = baseAddr + 0 * VIP_ICM_TBL_Z;		/*global control*/
	unsigned int baseAddr_pillar	 = baseAddr + 1 * VIP_ICM_TBL_Z;		/*pillar*/

	unsigned short *tICM_ini_ctrl = &tICM_ini[baseAddr_global];
	unsigned short *tICM_ini_uv_coring = &tICM_ini[baseAddr_global + 4];
	unsigned short *tICM_ini_global_adj = &tICM_ini[baseAddr_global + 6];
	unsigned short *tICM_ini_pillar = &tICM_ini[baseAddr_pillar];

	if (which_table >=  VIP_ICM_TBL_X) {
		drvif_color_Icm_Enable(0);
		return;
	}

	if (display == SLR_MAIN_DISPLAY) {
		drvif_color_icm_save_global_of_viptable(tICM_ini_global_adj[0], &tICM_ini_global_adj[1], &tICM_ini_global_adj[13]);
		drvif_color_icm_Get_Global_Ctrl(TRUE, &dhue, dsatbysat, ditnbyitn);
		drvif_color_icm_Set_global(dhue, dsatbysat, ditnbyitn);

		fwif_color_icm_ini2elemtable(tICM_ini, which_table, &icm_tab_elem_of_vip_table);
		fwif_color_icm_global_adjust_on_elem_table(&icm_tab_elem_of_vip_table, dhue, dsatbysat, ditnbyitn, &icm_tab_elem_write);

		drvif_color_icm_set_ctrl(tICM_ini_ctrl[0], tICM_ini_ctrl[1], tICM_ini_ctrl[2], tICM_ini_ctrl[3]);
		drvif_color_icm_set_uv_coring(tICM_ini_uv_coring[0], tICM_ini_uv_coring[1]);
		drvif_color_icm_set_pillar(&tICM_ini_pillar[0], &tICM_ini_pillar[48], &tICM_ini_pillar[56]);

		drvif_color_icm_Write3DTable(icm_tab_elem_write.elem);
		drvif_color_Icm_Enable(1);
	}
}
void fwif_color_Icm_table_select_By_DMA(unsigned char display, unsigned short *tICM_ini, unsigned char which_table)
{
#ifndef BUILD_QUICK_SHOW

	unsigned int addr = 0, size;
	unsigned int *pVir_addr = 0;
	char ret;

	/*COLORELEM_TAB_T icm_tab_elem_write;*/
	int dhue;
	int dsatbysat[SATSEGMAX];
	int ditnbyitn[ITNSEGMAX];

	/*Global Setting*/
	unsigned int baseAddr			 = which_table * VIP_ICM_TBL_Y * VIP_ICM_TBL_Z;
	unsigned int baseAddr_global	 = baseAddr + 0 * VIP_ICM_TBL_Z;		/*global control*/
	unsigned int baseAddr_pillar	 = baseAddr + 1 * VIP_ICM_TBL_Z;		/*pillar*/

	unsigned short *tICM_ini_ctrl = &tICM_ini[baseAddr_global];
	unsigned short *tICM_ini_uv_coring = &tICM_ini[baseAddr_global + 4];
	unsigned short *tICM_ini_global_adj = &tICM_ini[baseAddr_global + 6];
	unsigned short *tICM_ini_pillar = &tICM_ini[baseAddr_pillar];

	addr = get_query_VIP_DMAto3DTABLE_start_address(DMAto3DTABLE_ICM);
	addr = drvif_memory_get_data_align(addr, (1 << 12));
	size = drvif_memory_get_data_align(VIP_DMAto3DTable_ICM_SIZE, (1 << 12));
	if (addr == 0) {
		rtd_pr_vpq_emerg("fwif_color_Icm_table_select_By_DMA, addr = NULL\n");
		return;
	}

	if (which_table >=  VIP_ICM_TBL_X) {
		drvif_color_Icm_Enable(0);
		return;
	}

	if (display == SLR_MAIN_DISPLAY) {
		drvif_color_icm_save_global_of_viptable(tICM_ini_global_adj[0], &tICM_ini_global_adj[1], &tICM_ini_global_adj[13]);
		drvif_color_icm_Get_Global_Ctrl(TRUE, &dhue, dsatbysat, ditnbyitn);
		drvif_color_icm_Set_global(dhue, dsatbysat, ditnbyitn);

		fwif_color_icm_ini2elemtable(tICM_ini, which_table, &icm_tab_elem_of_vip_table);
		fwif_color_icm_global_adjust_on_elem_table(&icm_tab_elem_of_vip_table, dhue, dsatbysat, ditnbyitn, &icm_tab_elem_write);

		drvif_color_icm_set_ctrl(tICM_ini_ctrl[0], tICM_ini_ctrl[1], tICM_ini_ctrl[2], tICM_ini_ctrl[3]);
		drvif_color_icm_set_uv_coring(tICM_ini_uv_coring[0], tICM_ini_uv_coring[1]);
		//drvif_color_icm_set_pillar(&tICM_ini_pillar[0], &tICM_ini_pillar[48], &tICM_ini_pillar[56]);
		g_tICM_ini_h_pillar = &tICM_ini_pillar[0];
		g_tICM_ini_s_pillar = &tICM_ini_pillar[48];
		g_tICM_ini_i_pillar = &tICM_ini_pillar[56];

		pVir_addr =(unsigned int*) dvr_remap_uncached_memory(addr, size, __builtin_return_address(0));
		ret = drvif_color_icm_Write3DTable_By_DMA(icm_tab_elem_write.elem, addr, pVir_addr, PILLAR_NUM_INFO_TYPE_FROM_GLOBAL_VAR);
		g_ICM_pillar_update_trigger = 1;
		dvr_unmap_memory((void *)pVir_addr, size);

		drvif_color_Icm_Enable(ret);
	}
#endif
}

void fwif_color_icm_set_pillar_by_ISR(void)
{
	if (g_ICM_pillar_update_trigger) {
		g_ICM_pillar_update_trigger = 0;
		drvif_color_icm_set_pillar(g_tICM_ini_h_pillar, g_tICM_ini_s_pillar, g_tICM_ini_i_pillar);
	}
}

unsigned char fwif_color_icm_Write3DTable_By_DMA(COLORELEM ICM_TAB_ACCESS[ITNSEGMAX][SATSEGMAX][HUESEGMAX])
{
#ifndef BUILD_QUICK_SHOW

	unsigned int addr = 0, size;
	unsigned int *pVir_addr = 0;
	unsigned char ret;

	addr = get_query_VIP_DMAto3DTABLE_start_address(DMAto3DTABLE_ICM);
	addr = drvif_memory_get_data_align(addr, (1 << 12));
	size = drvif_memory_get_data_align(VIP_DMAto3DTable_ICM_SIZE, (1 << 12));
	if (addr == 0) {
		rtd_pr_vpq_emerg("fwif_color_Icm_table_select_By_DMA, addr = NULL\n");
		return false;
	}

	if (ICM_TAB_ACCESS == NULL)
		return false;

	pVir_addr =(unsigned int*) dvr_remap_uncached_memory(addr, size, __builtin_return_address(0));
	ret = drvif_color_icm_Write3DTable_By_DMA(icm_tab_elem_write.elem, addr, pVir_addr, PILLAR_NUM_INFO_TYPE_FROM_GLOBAL_VAR);
	dvr_unmap_memory((void *)pVir_addr, size);

	return ret;
#else 

	return 0;
#endif
}

void fwif_color_set_ICM_table(unsigned char src_idx, unsigned char value)
{
	StructColorDataType *pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;
	pTable->ICM_Table = value;

	down(&ICM_Semaphore);
	fwif_color_set_ICM_table_driver(src_idx, value);
	up(&ICM_Semaphore);
}

void fwif_color_set_ICM_table_driver(unsigned char src_idx, unsigned char value)
{

	int dhue;
	int dsatbysat[SATSEGMAX];
	int ditnbyitn[ITNSEGMAX];
	UINT8 nIndex = 0;
	int *pwINT32Temp = NULL;

	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
#ifndef BUILD_QUICK_SHOW
	fwif_color_Icm_table_select_By_DMA(_MAIN_DISPLAY, &(gVip_Table->tICM_ini)[0][0][0], value);
#else
	//fwif_color_Icm_table_select(_MAIN_DISPLAY, &(gVip_Table->tICM_ini)[0][0][0], value);
	return; //need to fixed this function 
#endif
	drvif_color_icm_Get_Global_Ctrl(FALSE, &dhue, dsatbysat, ditnbyitn);
/*
	drvif_color_icm_table_select(_MAIN_DISPLAY, &(gVip_Table->tICM_ini)[0][0][0], value);

	//global ctrl read from ICM Table
	drvif_color_icm_Get_Global_Ctrl(TRUE, &dhue, dsatbysat, ditnbyitn);

	drvif_color_icm_global_adjust(dhue, dsatbysat, ditnbyitn, FALSE);
*/


	/*====   global ctrl write  to share memory  =====*/
	pwINT32Temp = (int *)&dhue;
	fwif_color_ChangeINT32Endian(pwINT32Temp, 1, 0);
	pwINT32Temp = (int *)&dsatbysat;
	fwif_color_ChangeINT32Endian(pwINT32Temp, SATSEGMAX, 0);
	pwINT32Temp = (int *)&ditnbyitn;
	fwif_color_ChangeINT32Endian(pwINT32Temp, ITNSEGMAX, 0);

	if (NULL !=  fwif_color_GetICM_Global_Ctrl_Struct()) {
		fwif_color_GetICM_Global_Ctrl_Struct()->dHue = dhue;
		for (nIndex = 0; nIndex < SATSEGMAX; nIndex++) {
			(fwif_color_GetICM_Global_Ctrl_Struct()->dSatBySat)[nIndex] = dsatbysat[nIndex];
		}
		for (nIndex = 0; nIndex < ITNSEGMAX; nIndex++) {
			(fwif_color_GetICM_Global_Ctrl_Struct()->dItnByItn)[nIndex] = ditnbyitn[nIndex];
		}
	}
	/*==============*/
}
void fwif_color_set_ICM_Global_Adjust(int dhue_cur, int dsatbysat_cur[SATSEGMAX], int ditnbyitn_cur[ITNSEGMAX], bool bSubtractPrevious)
{
	drvif_color_icm_global_adjust(dhue_cur, dsatbysat_cur, ditnbyitn_cur, bSubtractPrevious);
}
bool fwif_color_icm_SramBlockAccessSpeedup(unsigned int *buf, bool if_write_ic, int h_sta, int h_end, int s_sta, int s_end, int i_sta, int i_end)
{
	return drvif_color_icm_SramBlockAccessSpeedup(buf, if_write_ic,  h_sta,  h_end,  s_sta,  s_end,  i_sta,  i_end);
}
void  fwif_color_icm_enhancement(unsigned char display, unsigned char which_table, unsigned char value)
{
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	/*drvif_color_icm_enhancement(SLR_MAIN_DISPLAY, &(gVip_Table->tICM_ini)[0][0][0], which_table, value);*/
}
void  fwif_color_ChannelColourSet(unsigned char display, unsigned char which_table,  signed char value)
{
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	/*drvif_color_ChannelColourSet(SLR_MAIN_DISPLAY, &(gVip_Table->tICM_ini)[0][0][0], which_table, value);*/
}

void fwif_color_icm_user_function(unsigned char display, unsigned char which_table, unsigned char skin_tone, unsigned char color_wheel, unsigned char detail_enhance, unsigned dynamic_tint)
{
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	drvif_color_icm_user_function(display, &(gVip_Table->tICM_ini)[0][0][0], which_table, skin_tone, color_wheel, detail_enhance, dynamic_tint);
}

void fwif_color_get_icm_table_Adjust_Block(unsigned char display, unsigned short *tICM_ini, unsigned char which_table,
																						int *SR, unsigned int *buf)
{
	int h, s, i;
/*	int h_sta, h_end, si_sta, si_end;*/
/*	bool if_write_ic;*/
/*	int timeout;*/
	int LayerSize;

	int hlen, slen, ilen;

	unsigned char ICM_Num_Limite	= 255;/*30;*/ /*planck changed to 255 for the max of icm tables.20101111*/

	/* Global Setting*/
	unsigned int baseAddr			= which_table * VIP_ICM_TBL_Y * VIP_ICM_TBL_Z;
	/*unsigned int baseAddr_global	= baseAddr + 0 * VIP_ICM_TBL_Z;*/		/*global control*/
	/*unsigned int baseAddr_pillar	= baseAddr + 1 * VIP_ICM_TBL_Z;*/		/*pillar*/

	unsigned int baseAddr_hue_tab	= baseAddr + (2 + SATSEGMAX * ITNSEGMAX * 0) * VIP_ICM_TBL_Z;		/*hue table*/
	unsigned int baseAddr_sat_tab	= baseAddr + (2 + SATSEGMAX * ITNSEGMAX * 1) * VIP_ICM_TBL_Z;		/*sat table*/
	unsigned int baseAddr_itn_tab	= baseAddr + (2 + SATSEGMAX * ITNSEGMAX * 2) * VIP_ICM_TBL_Z;		/*itn table*/


	hlen = SR[h_end] - SR[h_sta] + 1;
	slen = SR[s_end] - SR[s_sta] + 1;
	ilen = SR[i_end] - SR[i_sta] + 1;

	if (which_table >= ICM_Num_Limite) /*overflow*/
		return;
	if (display == SLR_MAIN_DISPLAY) {
		LayerSize = HUESEGMAX * SATSEGMAX;

		for (i = SR[i_sta]; i <= SR[i_end]; i++) {
			for (s = SR[s_sta]; s <= SR[s_end]; s++) {
				for (h = SR[h_sta]; h <= SR[h_end]; h++) {
					buf[(((i - SR[i_sta]) * slen + (s - SR[s_sta])) * hlen + (h - SR[h_sta])) * 2 + 0] = (tICM_ini[baseAddr_hue_tab + LayerSize * i  + s * HUESEGMAX + h] << 16)+(tICM_ini[baseAddr_sat_tab + LayerSize * i  + s * HUESEGMAX + h]);
					buf[(((i - SR[i_sta]) * slen + (s - SR[s_sta])) * hlen + (h - SR[h_sta])) * 2 + 1] = (tICM_ini[baseAddr_itn_tab + LayerSize * i  + s * HUESEGMAX + h]);
				}
			}
		}


	}   /* end main*/
}

void fwif_color_modify_icm_table_Adjust_Block(unsigned int *buf, unsigned char color, ICM_OSD_table *ICM_OSD_Table, int *SR, unsigned char *OSD)
{
	int h = 0, s = 0, i = 0;
	int hlen, slen, ilen;
	int org_hue, org_sat, org_int;

	hlen = SR[h_end] - SR[h_sta] + 1;
	slen = SR[s_end] - SR[s_sta] + 1;
	ilen = SR[i_end] - SR[i_sta] + 1;

	for (h = SR[h_sta]; h <= SR[h_end]; h++) {
		for (s = SR[s_sta]; s <= SR[s_end]; s++) {
			for (i = SR[i_sta]; i <= SR[i_end]; i++) {
				if ((ICM_OSD_Table->ICM_OSD_index[i][s][h]) == color) {
					org_hue = (buf[(((i - SR[i_sta]) * slen + (s - SR[s_sta])) * hlen + (h - SR[h_sta])) * 2 + 0]>>16 & 0xffff) + (unsigned int)(OSD[_HUE_ELEM]*(ICM_OSD_Table->ICM_OSD_weight[i][s][h]));
					org_hue = drvif_color_icm_FormatValue(org_hue, _HUE_ELEM, 0, 1);
					org_sat = (buf[(((i - SR[i_sta]) * slen + (s - SR[s_sta])) * hlen + (h - SR[h_sta])) * 2 + 0] & 0xffff) + (unsigned int)((OSD[_SAT_ELEM]*(ICM_OSD_Table->ICM_OSD_weight[i][s][h])) << 2);
					org_sat = drvif_color_icm_FormatValue(org_sat, _SAT_ELEM, 0, 1);
					org_int = (buf[(((i - SR[i_sta]) * slen + (s - SR[s_sta])) * hlen + (h - SR[h_sta])) * 2 + 1] & 0xffff) + (unsigned int)((OSD[_ITN_ELEM]*(ICM_OSD_Table->ICM_OSD_weight[i][s][h])) << 2);
					org_int = drvif_color_icm_FormatValue((org_int-ITN_SUBTRACT_CONST), _ITN_ELEM, 0, 1);
					buf[(((i - SR[i_sta]) * slen + (s - SR[s_sta])) * hlen + (h - SR[h_sta])) * 2 + 0] = (org_hue << 16)+org_sat;
					buf[(((i - SR[i_sta]) * slen + (s - SR[s_sta])) * hlen + (h - SR[h_sta])) * 2 + 1] = org_int;
				}
			}
		}
	}
}

void fwif_color_icm_OSD_color(unsigned char display, unsigned char which_table, ICM_OSD_table *ICM_OSD_Table, unsigned char *ICM_OSD_value, unsigned char color)
{
	/*
	display 		:	main/sub
	which_table		:	timming use ICM table
	ICM_OSD_Table	:	ICM OSD index/weight/Search range
	color			:	Action color
	*/
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	/*int i = 0, offset = 0, sta = 0, end = 0, h_total;*/
	unsigned int *buf;
	int SR[6];

	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	memcpy(SR, &(ICM_OSD_Table->ICM_SR[color][0]), sizeof(int)*6);
	if (SR[h_end] >= SR[h_sta]) {
		buf = (unsigned int *)vip_malloc(((SR[i_end] - SR[i_sta] + 1) * (SR[s_end] - SR[s_sta] + 1) * (SR[h_end] - SR[h_sta] + 1) * 2) * sizeof(unsigned int));
		if (buf) {
			fwif_color_get_icm_table_Adjust_Block(display, &(gVip_Table->tICM_ini)[0][0][0], which_table, SR, buf);
			fwif_color_modify_icm_table_Adjust_Block(buf, (color+1), ICM_OSD_Table, SR, &ICM_OSD_value[color*3]);
			drvif_color_icm_SramBlockAccessSpeedup(buf, 1,  SR[h_sta],  SR[h_end],  SR[s_sta],  SR[s_end],  SR[i_sta],  SR[i_end]);
			/*free(buf);*/
			vip_free((void *)buf);
		}
	} else {		/*if hue step over 0*/
		memcpy(SR, &(ICM_OSD_Table->ICM_SR[color][0]), sizeof(int)*6);
		SR[h_end] = ICM_OSD_Table->h_Total_axis;		/* h_sta ~ h_Total_axis*/
		buf = (unsigned int *)vip_malloc(((SR[i_end] - SR[i_sta] + 1) * (SR[s_end] - SR[s_sta] + 1) * (SR[h_end] - SR[h_sta] + 1) * 2) * sizeof(unsigned int));
		if (buf) {
			fwif_color_get_icm_table_Adjust_Block(display, &(gVip_Table->tICM_ini)[0][0][0], which_table, SR, buf);
			fwif_color_modify_icm_table_Adjust_Block(buf, (color+1), ICM_OSD_Table, SR, &ICM_OSD_value[color*3]);
			drvif_color_icm_SramBlockAccessSpeedup(buf, 1,  SR[h_sta],  SR[h_end],  SR[s_sta],  SR[s_end],  SR[i_sta],  SR[i_end]);
			/*free(buf);*/
			vip_free((void *)buf);
		}

		memcpy(SR, &(ICM_OSD_Table->ICM_SR[color][0]), sizeof(int)*6);
		SR[h_sta] = 0;		/* 0 ~ h_end*/
		buf = (unsigned int *)vip_malloc(((SR[i_end] - SR[i_sta] + 1) * (SR[s_end] - SR[s_sta] + 1) * (SR[h_end] - SR[h_sta] + 1) * 2) * sizeof(unsigned int));
		if (buf) {
			fwif_color_get_icm_table_Adjust_Block(display, &(gVip_Table->tICM_ini)[0][0][0], which_table, SR, buf);
			fwif_color_modify_icm_table_Adjust_Block(buf, (color+1), ICM_OSD_Table, SR, &ICM_OSD_value[color*3]);
			drvif_color_icm_SramBlockAccessSpeedup(buf, 1,  SR[h_sta],  SR[h_end],  SR[s_sta],  SR[s_end],  SR[i_sta],  SR[i_end]);
			/*free(buf);*/
			vip_free((void *)buf);
		}
	}
}


/*============== ICM  =====*/


/*=============================================================================//VDC ConBri CSFC 20140210*/
/*===  VDC Contrast / Brightness ================*/

void fwif_color_module_vdc_SetConBriHueSat(unsigned char src_idx, unsigned char value)
{
#ifndef BUILD_QUICK_SHOW
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	drvif_module_vdc_SetContrast(gVip_Table->VD_ConBriHueSat[value].VD_contrast);
	drvif_module_vdc_SetBrightness(gVip_Table->VD_ConBriHueSat[value].VD_brightness);
	drvif_module_vdc_SetHue(gVip_Table->VD_ConBriHueSat[value].VD_hue);
	drvif_module_vdc_SetSaturation(gVip_Table->VD_ConBriHueSat[value].VD_saturation);
#endif 	
}

int fwif_color_module_vdc_SetConBriSatHue_byAP(VD_COLOR_ST *pVD_Color)
{
#ifndef BUILD_QUICK_SHOW
	if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_AVD) {
		drvif_module_vdc_SetContrast(pVD_Color->Contrast);
		drvif_module_vdc_SetBrightness(pVD_Color->Brightness);
		drvif_module_vdc_SetSaturation(pVD_Color->Saturation);
		drvif_module_vdc_SetHue(pVD_Color->Hue);
		return 0;
	} else
		return -1;
#else 

	return -1;
#endif 
}

int fwif_color_module_vdc_GetConBriSatHue_byAP(VD_COLOR_ST *pVD_Color)
{
	if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_AVD) {
		pVD_Color->Contrast = VDTOP_LUMA_CONTRAST_ADJ_get_contrast(IoReg_Read32(VDTOP_LUMA_CONTRAST_ADJ_reg));
		pVD_Color->Brightness = VDTOP_LUMA_BRIGHTNESS_ADJ_get_brightness(IoReg_Read32(VDTOP_LUMA_BRIGHTNESS_ADJ_reg));
		pVD_Color->Saturation = VDTOP_CHROMA_SATURATION_ADJ_get_saturation(IoReg_Read32(VDTOP_CHROMA_SATURATION_ADJ_reg));
		pVD_Color->Hue = VDTOP_CHROMA_HUE_PHASE_ADJ_get_hue(IoReg_Read32(VDTOP_CHROMA_HUE_PHASE_ADJ_reg));
		return 0;
	} else
		return -1;
}

/*===  VDC Contrast / Brightness ================*/
/*=============================================================================//VDC ConBri CSFC 20140210*/

/*======== Set color temp=============*/
void fwif_color_setrgbcontrast(unsigned char display, unsigned short Red, unsigned short Green, unsigned short Blue, unsigned char  BlendRatio, unsigned char Overlay)
{
	VIPprintf("fwif_color_setrgbcontrast: display = %d, R = %d, G = %d, B = %d, BlendRatio = %d, Overlay = %d\n", display, Red, Green, Blue, BlendRatio, Overlay);

	/*USER:LewisLee DATE:2010/08/04*/
	/*at power on eye guard mode, we need set contrast later*/
	/*
	if (_TRUE == Scaler_Get_During_Power_On_EyeGuard_Adjust(SLR_EYE_GUARD_CONTRAST)) {
		Scaler_Set_Contrast_During_Power_On_EyeGuard(SLR_EYE_GUARD_CINTRAST_R, Red);
		Scaler_Set_Contrast_During_Power_On_EyeGuard(SLR_EYE_GUARD_CINTRAST_G, Green);
		Scaler_Set_Contrast_During_Power_On_EyeGuard(SLR_EYE_GUARD_CINTRAST_B, Blue);
	}
	elseif (_FALSE == Scaler_Get_During_Power_On_EyeGuard_Adjust(SLR_EYE_GUARD_CONTRAST))
	*/

		drvif_color_setrgbcontrast(
			display,
			Red,
			Green,
			Blue,
			BlendRatio,
			Overlay);

}

void fwif_color_setrgbbrightness(unsigned char display, unsigned short Red, unsigned short Green, unsigned short Blue)
{
	VIPprintf("drvif_color_setrgbbrightness: display = %d, R = %d, G = %d, B = %d\n", display, Red, Green, Blue);

	/*USER:LewisLee DATE:2010/08/04*/
	/*at power on eye guard mode, we need set brightness later*/
	/*
	if (_TRUE == Scaler_Get_During_Power_On_EyeGuard_Adjust(SLR_EYE_GUARD_BRIGHTNESS))
	{
		Scaler_Set_Contrast_During_Power_On_EyeGuard(SLR_EYE_GUARD_CINTRAST_R, Red);
		Scaler_Set_Contrast_During_Power_On_EyeGuard(SLR_EYE_GUARD_CINTRAST_G, Green);
		Scaler_Set_Contrast_During_Power_On_EyeGuard(SLR_EYE_GUARD_CINTRAST_B, Blue);
	}
	elseif (_FALSE == Scaler_Get_During_Power_On_EyeGuard_Adjust(SLR_EYE_GUARD_BRIGHTNESS))
	*/

		drvif_color_setrgbbrightness(
			display,
			Red,
			Green,
			Blue);

}

void fwif_color_Gamma_Curve_Write(unsigned int *Gamma_Encode, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx)
{
	drvif_color_Gamma_Curve_Write(Gamma_Encode, RGB_chanel_idx);
}

void fwif_color_setgamma(unsigned char display, unsigned char Mode)
{
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();

	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	if (Mode > 9) {
		VIPprintf("fwif_color_setgamma error");
		return;
	}

	drvif_color_setgamma(display, Mode, &gVip_Table->tGAMMA[Mode]);

}
void fwif_color_setrgbcontrast_Identity(UINT8 isOverlay/* = 0*/)
{
	fwif_color_setrgbcontrast(
		_MAIN_DISPLAY,
		512,
		512,
		512,
		Scaler_PipGetInfo(SLR_PIP_BLENDING), isOverlay);
}

void fwif_color_setrgbbrightness_Identity()
{

	fwif_color_setrgbbrightness(
		_MAIN_DISPLAY,
		512,
		512,
		512);
}

void fwif_color_setrgbcontrast_By_Table(unsigned short R, unsigned short G, unsigned short B, UINT8 isOverlay/* = 0*/)
{
	VIPprintf("fwif_color_setrgbcontrast_By_Table: R = %d, G = %d, B = %d\n", R, G, B);
	fwif_color_setrgbcontrast(_MAIN_DISPLAY,
		R,
		G,
		B,
		Scaler_PipGetInfo(SLR_PIP_BLENDING), isOverlay);
}

void fwif_color_setrgbbrightness_By_Table(unsigned short R, unsigned short G, unsigned short B)
{
	VIPprintf("fwif_color_setrgbbrightness_By_Table: R = %d, G = %d, B = %d\n", R, G, B);
	fwif_color_setrgbbrightness(_MAIN_DISPLAY,
		R,
		G,
		B);
}

void fwif_color_set_color_temp(unsigned char enable, unsigned short Red_Con, unsigned short Green_Con, unsigned short Blue_Con,
	short Red_Bri, short Green_Bri, short Blue_Bri, unsigned char waitSync)
{
	if (waitSync)
		fwif_color_WaitFor_SYNC_START_UZUDTG();
	drvif_color_set_color_temp(enable, Red_Con, Green_Con, Blue_Con, Red_Bri, Green_Bri, Blue_Bri);
}

void fwif_color_gamma_control_front(unsigned char display)
{
	drvif_color_gamma_control_front(display);
}

void fwif_color_gamma_control_back(unsigned char display, unsigned char ucWriteEnable)
{
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if (VIP_RPC_system_info_structure_table != NULL) {
		if (VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_DEMO_EN] == 1)
			return;
	}

	drvif_color_gamma_control_back(display, ucWriteEnable);
}

UINT32 Identity_Gamma[512] = {
	0x00000404, 0x00080404, 0x00100404, 0x00180404, 0x00200404, 0x00280404, 0x00300404, 0x00380404, /*Identity	1~	8*/
	0x00400404, 0x00480404, 0x00500404, 0x00580404, 0x00600404, 0x00680404, 0x00700404, 0x00780404, /*Identity	9~ 16*/
	0x00800404, 0x00880404, 0x00900404, 0x00980404, 0x00a00404, 0x00a80404, 0x00b00404, 0x00b80404, /*Identity  17~ 24*/
	0x00c00404, 0x00c80404, 0x00d00404, 0x00d80404, 0x00e00404, 0x00e80404, 0x00f00404, 0x00f80404, /*Identity  25~ 32*/
	0x01000404, 0x01080404, 0x01100404, 0x01180404, 0x01200404, 0x01280404, 0x01300404, 0x01380404, /*Identity  33~ 40*/
	0x01400404, 0x01480404, 0x01500404, 0x01580404, 0x01600404, 0x01680404, 0x01700404, 0x01780404, /*Identity  41~ 48*/
	0x01800404, 0x01880404, 0x01900404, 0x01980404, 0x01a00404, 0x01a80404, 0x01b00404, 0x01b80404, /*Identity  49~ 56*/
	0x01c00404, 0x01c80404, 0x01d00404, 0x01d80404, 0x01e00404, 0x01e80404, 0x01f00404, 0x01f80404, /*Identity  57~ 64*/
	0x02000404, 0x02080404, 0x02100404, 0x02180404, 0x02200404, 0x02280404, 0x02300404, 0x02380404, /*Identity  65~ 72*/
	0x02400404, 0x02480404, 0x02500404, 0x02580404, 0x02600404, 0x02680404, 0x02700404, 0x02780404, /*Identity  73~ 80*/
	0x02800404, 0x02880404, 0x02900404, 0x02980404, 0x02a00404, 0x02a80404, 0x02b00404, 0x02b80404, /*Identity  81~ 88*/
	0x02c00404, 0x02c80404, 0x02d00404, 0x02d80404, 0x02e00404, 0x02e80404, 0x02f00404, 0x02f80404, /*Identity  89~ 96*/
	0x03000404, 0x03080404, 0x03100404, 0x03180404, 0x03200404, 0x03280404, 0x03300404, 0x03380404, /*Identity  97~104*/
	0x03400404, 0x03480404, 0x03500404, 0x03580404, 0x03600404, 0x03680404, 0x03700404, 0x03780404, /*Identity 105~112*/
	0x03800404, 0x03880404, 0x03900404, 0x03980404, 0x03a00404, 0x03a80404, 0x03b00404, 0x03b80404, /*Identity 113~120*/
	0x03c00404, 0x03c80404, 0x03d00404, 0x03d80404, 0x03e00404, 0x03e80404, 0x03f00404, 0x03f80404, /*Identity 121~128*/
	0x04000404, 0x04080404, 0x04100404, 0x04180404, 0x04200404, 0x04280404, 0x04300404, 0x04380404, /*Identity 129~136*/
	0x04400404, 0x04480404, 0x04500404, 0x04580404, 0x04600404, 0x04680404, 0x04700404, 0x04780404, /*Identity 137~144*/
	0x04800404, 0x04880404, 0x04900404, 0x04980404, 0x04a00404, 0x04a80404, 0x04b00404, 0x04b80404, /*Identity 145~152*/
	0x04c00404, 0x04c80404, 0x04d00404, 0x04d80404, 0x04e00404, 0x04e80404, 0x04f00404, 0x04f80404, /*Identity 153~160*/
	0x05000404, 0x05080404, 0x05100404, 0x05180404, 0x05200404, 0x05280404, 0x05300404, 0x05380404, /*Identity 161~168*/
	0x05400404, 0x05480404, 0x05500404, 0x05580404, 0x05600404, 0x05680404, 0x05700404, 0x05780404, /*Identity 169~176*/
	0x05800404, 0x05880404, 0x05900404, 0x05980404, 0x05a00404, 0x05a80404, 0x05b00404, 0x05b80404, /*Identity 177~184*/
	0x05c00404, 0x05c80404, 0x05d00404, 0x05d80404, 0x05e00404, 0x05e80404, 0x05f00404, 0x05f80404, /*Identity 185~192*/
	0x06000404, 0x06080404, 0x06100404, 0x06180404, 0x06200404, 0x06280404, 0x06300404, 0x06380404, /*Identity 193~200*/
	0x06400404, 0x06480404, 0x06500404, 0x06580404, 0x06600404, 0x06680404, 0x06700404, 0x06780404, /*Identity 201~208*/
	0x06800404, 0x06880404, 0x06900404, 0x06980404, 0x06a00404, 0x06a80404, 0x06b00404, 0x06b80404, /*Identity 209~216*/
	0x06c00404, 0x06c80404, 0x06d00404, 0x06d80404, 0x06e00404, 0x06e80404, 0x06f00404, 0x06f80404, /*Identity 217~224*/
	0x07000404, 0x07080404, 0x07100404, 0x07180404, 0x07200404, 0x07280404, 0x07300404, 0x07380404, /*Identity 225~232*/
	0x07400404, 0x07480404, 0x07500404, 0x07580404, 0x07600404, 0x07680404, 0x07700404, 0x07780404, /*Identity 233~240*/
	0x07800404, 0x07880404, 0x07900404, 0x07980404, 0x07a00404, 0x07a80404, 0x07b00404, 0x07b80404, /*Identity 241~248*/
	0x07c00404, 0x07c80404, 0x07d00404, 0x07d80404, 0x07e00404, 0x07e80404, 0x07f00404, 0x07f80404, /*Identity 249~256*/
	0x08000404, 0x08080404, 0x08100404, 0x08180404, 0x08200404, 0x08280404, 0x08300404, 0x08380404, /*Identity 257~264*/
	0x08400404, 0x08480404, 0x08500404, 0x08580404, 0x08600404, 0x08680404, 0x08700404, 0x08780404, /*Identity 265~272*/
	0x08800404, 0x08880404, 0x08900404, 0x08980404, 0x08a00404, 0x08a80404, 0x08b00404, 0x08b80404, /*Identity 273~280*/
	0x08c00404, 0x08c80404, 0x08d00404, 0x08d80404, 0x08e00404, 0x08e80404, 0x08f00404, 0x08f80404, /*Identity 281~288*/
	0x09000404, 0x09080404, 0x09100404, 0x09180404, 0x09200404, 0x09280404, 0x09300404, 0x09380404, /*Identity 289~296*/
	0x09400404, 0x09480404, 0x09500404, 0x09580404, 0x09600404, 0x09680404, 0x09700404, 0x09780404, /*Identity 297~304*/
	0x09800404, 0x09880404, 0x09900404, 0x09980404, 0x09a00404, 0x09a80404, 0x09b00404, 0x09b80404, /*Identity 305~312*/
	0x09c00404, 0x09c80404, 0x09d00404, 0x09d80404, 0x09e00404, 0x09e80404, 0x09f00404, 0x09f80404, /*Identity 313~320*/
	0x0a000404, 0x0a080404, 0x0a100404, 0x0a180404, 0x0a200404, 0x0a280404, 0x0a300404, 0x0a380404, /*Identity 321~328*/
	0x0a400404, 0x0a480404, 0x0a500404, 0x0a580404, 0x0a600404, 0x0a680404, 0x0a700404, 0x0a780404, /*Identity 329~336*/
	0x0a800404, 0x0a880404, 0x0a900404, 0x0a980404, 0x0aa00404, 0x0aa80404, 0x0ab00404, 0x0ab80404, /*Identity 337~344*/
	0x0ac00404, 0x0ac80404, 0x0ad00404, 0x0ad80404, 0x0ae00404, 0x0ae80404, 0x0af00404, 0x0af80404, /*Identity 345~352*/
	0x0b000404, 0x0b080404, 0x0b100404, 0x0b180404, 0x0b200404, 0x0b280404, 0x0b300404, 0x0b380404, /*Identity 353~360*/
	0x0b400404, 0x0b480404, 0x0b500404, 0x0b580404, 0x0b600404, 0x0b680404, 0x0b700404, 0x0b780404, /*Identity 361~368*/
	0x0b800404, 0x0b880404, 0x0b900404, 0x0b980404, 0x0ba00404, 0x0ba80404, 0x0bb00404, 0x0bb80404, /*Identity 369~376*/
	0x0bc00404, 0x0bc80404, 0x0bd00404, 0x0bd80404, 0x0be00404, 0x0be80404, 0x0bf00404, 0x0bf80404, /*Identity 377~384*/
	0x0c000404, 0x0c080404, 0x0c100404, 0x0c180404, 0x0c200404, 0x0c280404, 0x0c300404, 0x0c380404, /*Identity 385~392*/
	0x0c400404, 0x0c480404, 0x0c500404, 0x0c580404, 0x0c600404, 0x0c680404, 0x0c700404, 0x0c780404, /*Identity 393~400*/
	0x0c800404, 0x0c880404, 0x0c900404, 0x0c980404, 0x0ca00404, 0x0ca80404, 0x0cb00404, 0x0cb80404, /*Identity 401~408*/
	0x0cc00404, 0x0cc80404, 0x0cd00404, 0x0cd80404, 0x0ce00404, 0x0ce80404, 0x0cf00404, 0x0cf80404, /*Identity 409~416*/
	0x0d000404, 0x0d080404, 0x0d100404, 0x0d180404, 0x0d200404, 0x0d280404, 0x0d300404, 0x0d380404, /*Identity 417~424*/
	0x0d400404, 0x0d480404, 0x0d500404, 0x0d580404, 0x0d600404, 0x0d680404, 0x0d700404, 0x0d780404, /*Identity 425~432*/
	0x0d800404, 0x0d880404, 0x0d900404, 0x0d980404, 0x0da00404, 0x0da80404, 0x0db00404, 0x0db80404, /*Identity 433~440*/
	0x0dc00404, 0x0dc80404, 0x0dd00404, 0x0dd80404, 0x0de00404, 0x0de80404, 0x0df00404, 0x0df80404, /*Identity 441~448*/
	0x0e000404, 0x0e080404, 0x0e100404, 0x0e180404, 0x0e200404, 0x0e280404, 0x0e300404, 0x0e380404, /*Identity 449~456*/
	0x0e400404, 0x0e480404, 0x0e500404, 0x0e580404, 0x0e600404, 0x0e680404, 0x0e700404, 0x0e780404, /*Identity 457~464*/
	0x0e800404, 0x0e880404, 0x0e900404, 0x0e980404, 0x0ea00404, 0x0ea80404, 0x0eb00404, 0x0eb80404, /*Identity 465~472*/
	0x0ec00404, 0x0ec80404, 0x0ed00404, 0x0ed80404, 0x0ee00404, 0x0ee80404, 0x0ef00404, 0x0ef80404, /*Identity 473~480*/
	0x0f000404, 0x0f080404, 0x0f100404, 0x0f180404, 0x0f200404, 0x0f280404, 0x0f300404, 0x0f380404, /*Identity 481~488*/
	0x0f400404, 0x0f480404, 0x0f500404, 0x0f580404, 0x0f600404, 0x0f680404, 0x0f700404, 0x0f780404, /*Identity 489~496*/
	0x0f800404, 0x0f880404, 0x0f900404, 0x0f980404, 0x0fa00404, 0x0fa80404, 0x0fb00404, 0x0fb80404, /*Identity 497~504*/
	0x0fc00404, 0x0fc80404, 0x0fd00404, 0x0fd80404, 0x0fe00404, 0x0fe80404, 0x0ff00404, 0x0ff80404, /*Identity 505~512*/
};

void fwif_color_set_gamma_Identity(void)
{
	VIPprintf("fwif_color_set_gamma_Identity\n");

	drvif_color_gamma_control_front(0);

	fwif_color_Gamma_Curve_Write(Identity_Gamma, GAMMA_CHANNEL_R);	/*R*/
	fwif_color_Gamma_Curve_Write(Identity_Gamma, GAMMA_CHANNEL_G);	/*G*/
	fwif_color_Gamma_Curve_Write(Identity_Gamma, GAMMA_CHANNEL_B);	/*B*/
	drvif_color_gamma_control_back(0, 1);
}


void fwif_color_get_gamma_default(unsigned char Gamma_Mode, unsigned int **In_R, unsigned int **In_G, unsigned int **In_B)
{
	unsigned short OSD_Color_temp, OSD_Color_name;

	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	g_GammaMode = Gamma_Mode;
	OSD_Color_name = scaler_get_color_temp_level_type(GET_UI_SOURCE_FROM_PCB());
	switch (OSD_Color_name) {
	case SLR_COLORTEMP_USER:
	case SLR_COLORTEMP_NORMAL:
			OSD_Color_temp = 2;
			break;
	case SLR_COLORTEMP_WARMER:
	case SLR_COLORTEMP_WARM:
			OSD_Color_temp = 1;
			break;
	case SLR_COLORTEMP_COOL:
	case SLR_COLORTEMP_COOLER:
			OSD_Color_temp = 3;
			break;
	default:
			VIPprintf("ERROR!! Scaler_color_get_gamma_default get the wrong color temp!!");
			OSD_Color_temp = 3;
			break;
		/*return;*/
	}

	VIPprintf("Scaler_color_get_gamma_default, Mode = %d\n", Gamma_Mode);
	if (Gamma_Mode !=  0) {
		switch (Gamma_Mode) {
		case 1:
				Gamma_Mode = OSD_Color_temp;
				break;
		case 2:
				Gamma_Mode = OSD_Color_temp + 3;
				break;
		default:
				VIPprintf("ERROR!! Scaler_color_get_gamma_default get the wrong mode!!");
				Gamma_Mode = OSD_Color_temp;
				break;
		}
	}

	VIPprintf("Scaler_color_get_gamma_default, OSD_Color_temp = %d, OSD_Color_name = %d, Mode = %d\n", OSD_Color_temp, OSD_Color_name, Gamma_Mode);

	switch (Gamma_Mode) {
	case 0:
			*In_R = Identity_Gamma;
			*In_G = Identity_Gamma;
			*In_B = Identity_Gamma;
		break;
	case 1:
	default:
			*In_R = gVip_Table->tGAMMA[0].R;
			*In_G = gVip_Table->tGAMMA[0].G;
			*In_B = gVip_Table->tGAMMA[0].B;
			break;
	case 2:
			*In_R = gVip_Table->tGAMMA[1].R;
			*In_G = gVip_Table->tGAMMA[1].G;
			*In_B = gVip_Table->tGAMMA[1].B;
			break;
	case 3:/*cool 2.2*/
			*In_R = gVip_Table->tGAMMA[4].R;
			*In_G = gVip_Table->tGAMMA[4].G;
			*In_B = gVip_Table->tGAMMA[4].B;
			break;
	case 4:/*warm 2.4*/
			*In_R = gVip_Table->tGAMMA[5].R;
			*In_G = gVip_Table->tGAMMA[5].G;
			*In_B = gVip_Table->tGAMMA[5].B;
			break;
	case 5:/*standard 2.4*/
			*In_R = gVip_Table->tGAMMA[6].R;
			*In_G = gVip_Table->tGAMMA[6].G;
			*In_B = gVip_Table->tGAMMA[6].B;
			break;
	case 6:/*cool 2.4*/
			*In_R = gVip_Table->tGAMMA[7].R;
			*In_G = gVip_Table->tGAMMA[7].G;
			*In_B = gVip_Table->tGAMMA[7].B;
			break;
	}
}

void fwif_color_encode_gamma_debug(UINT8 type)
{
	UINT16 i;

	switch (type) {
	case 1:
			VIPprintf("======choose encode gamma data result======\n");
			for (i = 0; i < Bin_Num_Gamma / 2; i++) {
				if ((i < 10) || (i > ((Bin_Num_Gamma / 2) - 10)))
					/*VIPprintf("tGAMMA_temp_R[%d] = 0x%x\n", i, tGAMMA_temp_R[i]);*/
					VIPprintf("tGAMMA_temp_R[%d] = %x, tGAMMA_temp_G[%d] = %x, tGAMMA_temp_B[%d] = %x\n", i, tGAMMA_temp_R[i], i, tGAMMA_temp_G[i], i, tGAMMA_temp_B[i]);
			}
			break;
	case 2:
			VIPprintf("======after encode======\n");
			for (i = 0; i < Bin_Num_Gamma / 2; i++) {
				if ((i < 10) || (i > ((Bin_Num_Gamma / 2) - 10)))
					/*VIPprintf("tGAMMA_temp_R[%d] = 0x%x\n", i, tGAMMA_temp_R[i]);*/
					VIPprintf("final_GAMMA_R[%d] = %x, final_GAMMA_G[%d] = %x, final_GAMMA_B[%d] = %x\n", i, final_GAMMA_R[i], i, final_GAMMA_G[i], i, final_GAMMA_B[i]);
			}
			break;
	default:
			/*VIPprintf("======   ======\n");*/
			break;
	}
	/*VIPprintf("bin_num = %d\n", Bin_Num_Gamma);*/
}

void fwif_color_gamma_decode(UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B, unsigned int *In_R, unsigned int *In_G, unsigned int *In_B)
{
	UINT16 i;

	Out_R[0] = 0;
	Out_G[0] = 0;
	Out_B[0] = 0;

	for (i = 0; i < Bin_Num_Gamma; i++) {
		if (i % 2 == 0) {
			/*VIPprintf("0x%08x\n", tGAMMA_1_G[i/2]);*/

			Out_R[i] = (In_R[i / 2] >> 16);

			Out_R[i + 1] = (Out_R[i] + ((In_R[i / 2] & 0x7F00) >> 8));

			Out_G[i] = (In_G[i / 2] >> 16);
			Out_G[i + 1] = (Out_G[i] + ((In_G[i / 2] & 0x7F00) >> 8));

			Out_B[i] = (In_B[i / 2] >> 16);
			Out_B[i + 1] = (Out_B[i] + ((In_B[i / 2] & 0x7F00) >> 8));
		}
	}

	/*data protect*/
	Out_R[Bin_Num_Gamma] = Out_R[Bin_Num_Gamma - 1];
	Out_G[Bin_Num_Gamma] = Out_G[Bin_Num_Gamma - 1];
	Out_B[Bin_Num_Gamma] = Out_B[Bin_Num_Gamma - 1];
}

void fwif_color_decode_gamma_debug(UINT8 type)
{
	UINT16 i;
	/*VIPprintf("bin_num = %d\n", Bin_Num_Gamma);*/

	switch (type) {
	case 1:
			VIPprintf("======gamma_decode======\n");
			break;
	case 2:
			VIPprintf("======gamma_blending_S_curve======\n");
			break;
	default:
			/*VIPprintf("======   ======\n");*/
			break;
	}

	for (i = 0; i < Bin_Num_Gamma; i++) {
		if ((i < 10) || (i > (Bin_Num_Gamma - 10)))
			VIPprintf("GOut_R[%d] = %d, GOut_G[%d] = %d, GOut_B[%d] = %d\n", i, GOut_R[i], i, GOut_G[i], i, GOut_B[i]);
	}

}

void fwif_color_gamma_blending_S_curve(UINT16 index, UINT16 C_gain1, UINT16 C_gain0)
{
#if 0
	UINT16 G0[Bin_Num_Gamma + 1], G1[Bin_Num_Gamma + 1], Gz[Bin_Num_Gamma + 1];
	UINT8 Curve = 1;
	UINT16 i;

	VIPprintf("fwif_color_gamma_blending_S_curve, index = %d, C_gain1 = %d, C_gain0 = %d\n", index, C_gain1, C_gain0);
	if (C_gain1 == 0 && C_gain0 == 0)
		return;

	/*	for gamma >1: index = 1023, gain0 = (0.5 - 0.5^gamma)*400; (20140526_jyyang)*/
	FLOAT32 C_gain0f = (float)(C_gain0 / 100.);
	FLOAT32 C_gain1f = (float)(C_gain1 / 100.);

	if (index <=  0)
		index = 1;

	for (i = 0; i <=  Bin_Num_Gamma; i++) {
		G0[i] = i * gamma_scale;
		/*if (i !=  0 && G0[i] < G0[i - 1])*/
			/*VIPprintf("G0[%d] = %d\n", i, G0[i]);*/
	}

	for (i = 0; i <=  Bin_Num_Gamma; i++) {
		if (Curve == 1) {
			if (i <=  index)
				G1[i] = (unsigned short)(((float)i * i / index) * gamma_scale + 0.5);
			else
				G1[i] = (unsigned short)(((float)(i - Bin_Num_Gamma)*(i - Bin_Num_Gamma)/(index - Bin_Num_Gamma) + Bin_Num_Gamma) * gamma_scale + 0.5);
		} else {
			if (i <=  index)
				G1[i] = (unsigned short)(((float)(i - index) * (i - index) / (0 - index) + index) * gamma_scale + 0.5);
			else
				G1[i] = (unsigned short)(((float)(i - index) * (i - index) / (Bin_Num_Gamma - index) + index) * gamma_scale + 0.5);
		}
		/*if (i !=  0 && G1[i] < G1[i - 1])*/
			/*VIPprintf("G1[%d] = %d\n", i, G1[i]);*/
	}

	Gz[0] = 0;
	for (i = 0; i <=  Bin_Num_Gamma; i++) {
		if (i <=  index)
			Gz[i] = (unsigned short)((G1[i] - G0[i]) * C_gain0f + G0[i] + 0.5);
		else
			Gz[i] = (unsigned short)((G1[i] - G0[i]) * C_gain1f + G0[i] + 0.5);
		/*if (i !=  0 && Gz[i] < Gz[i - 1])*/
			/*VIPprintf("Gz[%d] = %d, index = %d, C_gain0f = %f, C_gain1f = %f\n", i, Gz[i], index, C_gain0f, C_gain1f);*/
	}

	for (i = 1; i < Bin_Num_Gamma; i++) {
		/*VIPprintf("=============%d========%d===in\n", i, GOut_R[i]);*/
		/*if (((i <=  index) && (Gz[i + 1] < GOut_R[i])) || ((i > index) && (Gz[i + 1] > GOut_R[i])))*/
			GOut_R[i] = GOut_R[i] * Gz[i + 1] / ((i + 1) * gamma_scale);
		/*if (((i <=  index) && (Gz[i + 1] < GOut_G[i])) || ((i > index) && (Gz[i + 1] > GOut_G[i]))*/)
			GOut_G[i] = GOut_G[i] * Gz[i + 1] / ((i + 1) * gamma_scale);
		/*if (((i <=  index) && (Gz[i + 1] < GOut_B[i])) || ((i > index) && (Gz[i + 1] > GOut_B[i]))*/)
			GOut_B[i] = GOut_B[i] * Gz[i + 1] / ((i + 1) * gamma_scale);

		#if 0
		if (i !=  0 && GOut_R[i] < GOut_R[i - 1]) {
			GOut_R[i] = GOut_R[i - 1];
			/*VIPprintf("GOut_R[%d] = %d\n", i, GOut_R[i]);*/
		}
		if (i !=  0 && GOut_G[i] < GOut_G[i - 1]) {
			GOut_G[i] = GOut_G[i - 1];
			/*VIPprintf("GOut_G[%d] = %d\n", i, GOut_G[i]);*/
		}
		if (i !=  0 && GOut_B[i] < GOut_B[i - 1]) {
			GOut_B[i] = GOut_B[i - 1];
			/*VIPprintf("GOut_B[%d] = %d\n", i, GOut_B[i]);*/
		}
		#endif
		/*VIPprintf("GOut_R[%d] = %d, GOut_G[%d] = %d, GOut_B[%d] = %d\n", i, GOut_R[i], i, GOut_G[i], i, GOut_B[i]);*/
	}

	GOut_R[Bin_Num_Gamma] = GOut_R[Bin_Num_Gamma - 1];
	GOut_G[Bin_Num_Gamma] = GOut_G[Bin_Num_Gamma - 1];
	GOut_B[Bin_Num_Gamma] = GOut_B[Bin_Num_Gamma - 1];

#endif
}


void fwif_color_gamma_curve_data_protect(UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B)
{
	UINT16 i;
	UINT16 err_cnt = 0;

	/*gamma data protect*/
	Out_R[Bin_Num_Gamma] = Out_R[Bin_Num_Gamma - 1];
	Out_G[Bin_Num_Gamma] = Out_G[Bin_Num_Gamma - 1];
	Out_B[Bin_Num_Gamma] = Out_B[Bin_Num_Gamma - 1];

	/*R, make sure gamma curve is always increasing, and smaller than upper bound*/
	for (i = 0; i < Bin_Num_Gamma; i++) {
		if (Out_R[i] > Out_R[i + 1]) {
			if (err_cnt < 30)
				VIPprintf("Gamma curve ERROR!! Out_R[%d] = %d BIGGER than Out_R[%d] = %d\n", i, Out_R[i], i + 1, Out_R[i + 1]);
			/*Out_R[i + 1] = Out_R[i];*/
			err_cnt++;
		}
		if (Out_R[i] > Vmax_Gamma) {
			if (err_cnt < 30)
				VIPprintf("Gamma curve warning!! Out_R[%d] = %d BIGGER than Vmax_Gamma = %d\n", i, Out_R[i], Vmax_Gamma);
			Out_R[i] = Vmax_Gamma;
			err_cnt++;
		}
	}

	/*G, make sure gamma curve is always increasing, and smaller than upper bound*/
	for (i = 0; i < Bin_Num_Gamma; i++) {
		if (Out_G[i] > Out_G[i + 1]) {
			if (err_cnt < 30)
				VIPprintf("Gamma curve ERROR!! Out_G[%d] = %d BIGGER than Out_G[%d] = %d\n", i, Out_G[i], i + 1, Out_G[i + 1]);
			/*Out_G[i + 1] = Out_G[i];*/
			err_cnt++;
		}
		if (Out_G[i] > Vmax_Gamma) {
			if (err_cnt < 30)
				VIPprintf("Gamma curve warning!! Out_G[%d] = %d BIGGER than Vmax_Gamma = %d\n", i, Out_G[i], Vmax_Gamma);
			Out_G[i] = Vmax_Gamma;
			err_cnt++;
		}
	}

	/*B, make sure gamma curve is always increasing, and smaller than upper bound*/
	for (i = 0; i < Bin_Num_Gamma; i++) {
		if (Out_B[i] > Out_B[i + 1]) {
			if (err_cnt < 30)
				VIPprintf("Gamma curve ERROR!! Out_B[%d] = %d BIGGER than Out_B[%d] = %d\n", i, Out_B[i], i + 1, Out_B[i + 1]);
			/*Out_B[i + 1] = Out_B[i];*/
			err_cnt++;
		}

		if (Out_B[i] > Vmax_Gamma) {
			if (err_cnt < 30)
				VIPprintf("Gamma curve warning!! Out_B[%d] = %d BIGGER than Vmax_Gamma = %d\n", i, Out_B[i], Vmax_Gamma);
			Out_B[i] = Vmax_Gamma;
			err_cnt++;
		}
	}

	/*aviod all 0 to gamma curve*/
	if (Out_R[Bin_Num_Gamma] == 0 || Out_G[Bin_Num_Gamma] == 0 || Out_B[Bin_Num_Gamma] == 0 ||
		Out_R[Bin_Num_Gamma - 1] == 0 || Out_G[Bin_Num_Gamma - 1] == 0 || Out_B[Bin_Num_Gamma - 1] == 0) {
		VIPprintf("Gamma curve ERROR!! we have all 0 gamma curve!\n");
	}
}

void fwif_color_gamma_data_reduce(UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B)
{
	UINT16 i;
	for (i = 0; i <= Bin_Num_Gamma / 4; i++)
	{
		Out_R[i] = Out_R[i * 4];
		Out_G[i] = Out_G[i * 4];
		Out_B[i] = Out_B[i * 4];
	}
}

void fwif_color_gamma_encode(unsigned int *final_R, unsigned int *final_G, unsigned int *final_B, UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B)
{
	UINT16 i;
	UINT16 d0, d1;
	UINT8 d_max = 255;

	for (i = 0; i < Bin_Num_Gamma-1; i++) {
		if (i % 2 == 0) {
			d0 = (Out_R[i + 1] >=  Out_R[i]) ? (Out_R[i + 1] - Out_R[i]) : 0;
			d1 = (Out_R[i + 2] >=  Out_R[i + 1]) ? (Out_R[i + 2] - Out_R[i + 1]) : 0;
			d0 = (d0 >=  d_max) ? d_max : d0;
			d1 = (d1 >=  d_max) ? d_max : d1;
			final_R[i / 2] = (Out_R[i] << 16) + (d0 << 8) + d1;

			d0 = (Out_G[i + 1] >=  Out_G[i]) ? (Out_G[i + 1] - Out_G[i]) : 0;
			d1 = (Out_G[i + 2] >=  Out_G[i + 1]) ? (Out_G[i + 2] - Out_G[i + 1]) : 0;
			d0 = (d0 >=  d_max) ? d_max : d0;
			d1 = (d1 >=  d_max) ? d_max : d1;
			final_G[i / 2] = (Out_G[i] << 16) + (d0 << 8) + d1;


			d0 = (Out_B[i + 1] >= Out_B[i]) ? (Out_B[i + 1] - Out_B[i]) : 0;
			d1 = (Out_B[i + 2] >= Out_B[i + 1]) ? (Out_B[i + 2] - Out_B[i + 1]) : 0;
			d0 = (d0 >=  d_max) ? d_max : d0;
			d1 = (d1 >=  d_max) ? d_max : d1;
			final_B[i / 2] = (Out_B[i] << 16) + (d0 << 8) + d1;

		}
	}

	#if 0
	rtd_pr_vpq_debug("fwif_color_gamma_encode:\n");

	for (i = 0; i < 10; i++)
		rtd_pr_vpq_debug("final_R[%d] = 0x%x\n", i, final_R[i]);

	for (i = 250; i < 260; i++)
		rtd_pr_vpq_debug("final_R[%d] = 0x%x\n", i, final_R[i]);

	for (i = 500; i < 512; i++)
		rtd_pr_vpq_debug("final_R[%d] = 0x%x\n", i, final_R[i]);


	for (i = 0; i < 10; i++)
		rtd_pr_vpq_debug("Out_R[%d] = %d\n", i, Out_R[i]);
	for (i = 500; i < 520; i++)
		rtd_pr_vpq_debug("Out_R[%d] = %d\n", i, Out_R[i]);
	for (i = 1000; i < 1024; i++)
		rtd_pr_vpq_debug("Out_R[%d] = %d\n", i, Out_R[i]);
	#endif

}

void fwif_color_out_gamma_encode(unsigned int *final_R, unsigned int *final_G, unsigned int *final_B, UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B)
{
	UINT16 i;
	UINT16 d0, d1;
	UINT8 d_max = 255;

	for (i = 0; i < Bin_Num_Output_InvOutput_Gamma-1; i++) {
		if (i % 2 == 0) {
			d0 = (Out_R[i + 1] >=  Out_R[i]) ? (Out_R[i + 1] - Out_R[i]) : 0;
			d1 = (Out_R[i + 2] >=  Out_R[i + 1]) ? (Out_R[i + 2] - Out_R[i + 1]) : 0;
			d0 = (d0 >=  d_max) ? d_max : d0;
			d1 = (d1 >=  d_max) ? d_max : d1;
			final_R[i / 2] = (Out_R[i] << 16) + (d0 << 8) + d1;

			d0 = (Out_G[i + 1] >=  Out_G[i]) ? (Out_G[i + 1] - Out_G[i]) : 0;
			d1 = (Out_G[i + 2] >=  Out_G[i + 1]) ? (Out_G[i + 2] - Out_G[i + 1]) : 0;
			d0 = (d0 >=  d_max) ? d_max : d0;
			d1 = (d1 >=  d_max) ? d_max : d1;
			final_G[i / 2] = (Out_G[i] << 16) + (d0 << 8) + d1;


			d0 = (Out_B[i + 1] >= Out_B[i]) ? (Out_B[i + 1] - Out_B[i]) : 0;
			d1 = (Out_B[i + 2] >= Out_B[i + 1]) ? (Out_B[i + 2] - Out_B[i + 1]) : 0;
			d0 = (d0 >=  d_max) ? d_max : d0;
			d1 = (d1 >=  d_max) ? d_max : d1;
			final_B[i / 2] = (Out_B[i] << 16) + (d0 << 8) + d1;

		}
	}

}

void fwif_color_output_gamma_encode(unsigned int *final_R, unsigned int *final_G, unsigned int *final_B, UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B) // add for Merlin2 (jyyang 20160616)
{
	// gamma, inv_gamma: 1025bin, 512 grid(g0,d0,d1)
	// output_gamma, in_output_vgamma: 257bin, 128 grid(g0,d0,d1)
	// array size: final_R[128], Out_R[1025](for shared with gamma)
	UINT16 i,j0,j1,j2;
	UINT16 d0, d1;
	UINT8 d_max = 127;

	for (i = 0; i < Bin_Num_Output_InvOutput_Gamma; i++) {
		if (i % 2 == 0) {
			j0 = 4*i;
			j1 = j0+4; // 4*(i+1)
			j2 = j1+4; // 4*(i+2)
			d0 = (Out_R[j1] >=  Out_R[j0]) ? (Out_R[j1] - Out_R[j0]) : 0;
			d1 = (Out_R[j2] >=  Out_R[j1]) ? (Out_R[j2] - Out_R[j1]) : 0;
			d0 = (d0 >=  d_max) ? d_max : d0;
			d1 = (d1 >=  d_max) ? d_max : d1;
			final_R[i / 2] = (Out_R[j0] << 16) + (d0 << 8) + d1;

			d0 = (Out_G[j1] >=  Out_G[j0]) ? (Out_G[j1] - Out_G[j0]) : 0;
			d1 = (Out_G[j2] >=  Out_G[j1]) ? (Out_G[j2] - Out_G[j1]) : 0;
			d0 = (d0 >=  d_max) ? d_max : d0;
			d1 = (d1 >=  d_max) ? d_max : d1;
			final_G[i / 2] = (Out_G[j0] << 16) + (d0 << 8) + d1;


			d0 = (Out_B[j1] >= Out_B[j0]) ? (Out_B[j1] - Out_B[j0]) : 0;
			d1 = (Out_B[j2] >= Out_B[j1]) ? (Out_B[j2] - Out_B[j1]) : 0;
			d0 = (d0 >=  d_max) ? d_max : d0;
			d1 = (d1 >=  d_max) ? d_max : d1;
			final_B[i / 2] = (Out_B[j0] << 16) + (d0 << 8) + d1;

		}
	}

}
void fwif_color_out_gamma_encode_1ch(unsigned int * final, UINT16 * Out)
{
	UINT16 i;
	UINT16 d0, d1;
	UINT8 d_max = 127;

	for (i = 0; i < 256; i++) {
		if (i % 2 == 0) {
			d0 = (Out[i + 1] >=  Out[i]) ? (Out[i + 1] - Out[i]) : 0;
			d1 = (Out[i + 2] >=  Out[i + 1]) ? (Out[i + 2] - Out[i + 1]) : 0;
			d0 = (d0 >=  d_max) ? d_max : d0;
			d1 = (d1 >=  d_max) ? d_max : d1;
			final[i / 2] = (Out[i] << 16) + (d0 << 8) + d1;
		}
	}

}



void fwif_color_gamma_pre_encode_TV006_HDR_demo(UINT16 *final, UINT16 *main, UINT16 *sub)
{
	UINT16 i;

	//main gamma curve
	for (i = 0; i <= Bin_Num_Gamma / 2; i++)
	{
		final[i] = main[i * 2];
	}

	//sub gamma curve
	for (i = (Bin_Num_Gamma / 2) + 1; i <= Bin_Num_Gamma; i++)
	{
		final[i] = sub[(i - (Bin_Num_Gamma / 2)) * 2];
	}

	#if 0
	rtd_pr_vpq_debug("fwif_color_gamma_pre_encode_TV006_HDR_demo:\n");
	for (i = 0; i < 10; i++)
		rtd_pr_vpq_debug("final[%d] = %d\n", i, final[i]);

	for (i = 500; i < 520; i++)
		rtd_pr_vpq_debug("final[%d] = %d\n", i, final[i]);

	for (i = 1000; i < 1025; i++)
		rtd_pr_vpq_debug("final[%d] = %d\n", i, final[i]);
	#endif
}




void fwif_color_set_gamma_encode(unsigned int *final_GAMMA_R, unsigned int *final_GAMMA_G, unsigned int *final_GAMMA_B)
{
	fwif_color_Gamma_Curve_Write(final_GAMMA_R, GAMMA_CHANNEL_R);	/*R*/
	fwif_color_Gamma_Curve_Write(final_GAMMA_G, GAMMA_CHANNEL_G);	/*G*/
	fwif_color_Gamma_Curve_Write(final_GAMMA_B, GAMMA_CHANNEL_B);	/*B*/
}
bool fwif_color_set_ddomainISR_gamma_encode(unsigned int *final_GAMMA_R, unsigned int *final_GAMMA_G, unsigned int *final_GAMMA_B)
{
	ISR_GAMMA_WRITE_ST *p_stISRGamma = (ISR_GAMMA_WRITE_ST *)Scaler_GetShareMemVirAddr(SCALERIOC_SET_GAMMA);
	extern unsigned int isr_write_gamma_SN;

	if (p_stISRGamma != NULL) {
		isr_write_gamma_SN++;
		fwif_color_ChangeUINT32Endian_Copy(&isr_write_gamma_SN, 1, &p_stISRGamma->write_SerialNumber_start, 1);
		fwif_color_ChangeUINT32Endian_Copy(final_GAMMA_R, Bin_Num_Gamma / 2, p_stISRGamma->encoded_GAMMA_R, 1);
		fwif_color_ChangeUINT32Endian_Copy(final_GAMMA_G, Bin_Num_Gamma / 2, p_stISRGamma->encoded_GAMMA_G, 1);
		fwif_color_ChangeUINT32Endian_Copy(final_GAMMA_B, Bin_Num_Gamma / 2, p_stISRGamma->encoded_GAMMA_B, 1);
		p_stISRGamma->write_SerialNumber_end = p_stISRGamma->write_SerialNumber_start;

		if ((Scaler_SendRPC(SCALERIOC_SET_GAMMA, 0, 0)) < 0) {
			rtd_pr_vpq_err(" update GAMMA to driver fail !!!\n");
			return false;
		}
	}
	return true;
}
/*======== Set color temp end=============*/

void fwif_color_Set_Inv_Gamma_Encode(unsigned int *final_Inv_GAMMA_R, unsigned int *final_Inv_GAMMA_G, unsigned int *final_Inv_GAMMA_B)
{
	drvif_color_Inv_Gamma_Curve_Write(final_Inv_GAMMA_R, GAMMA_CHANNEL_R);	/*R*/
	drvif_color_Inv_Gamma_Curve_Write(final_Inv_GAMMA_G, GAMMA_CHANNEL_G);	/*G*/
	drvif_color_Inv_Gamma_Curve_Write(final_Inv_GAMMA_B, GAMMA_CHANNEL_B);	/*B*/
}

/*======================================================*/
/*please follow below step*/
/*0. gamma curve reg control*/
/*1. choose encode gamma data*/
/*2. decode gamma data to gamma curve*/
/*3. process the gamma curve*/
/*4. s-curve for gamma*/
/*5. gamma curve data protect and debug*/
/*6. encode gamma cuve to gamma data*/
/*7. write gamma table*/
/**/
/*BUT, each step can be SKIPPED or REPLACED*/
/*======================================================*/
void fwif_set_gamma(unsigned char Gamma_Mode, unsigned char Gamma_Sindex, unsigned char Gamma_Shigh, unsigned char Gamma_Slow)
{
#ifdef CONFIG_RTK_GAMUT_SUPPORT
	extern unsigned short Power1div22Gamma[1025];
#endif
	unsigned char display = 0;
	unsigned char src_idx = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	/*0. gamma curve reg control*/
	fwif_color_gamma_control_front(display);

	/*1. choose encode gamma data*/
	fwif_color_get_gamma_default(Gamma_Mode, &tGAMMA_temp_R, &tGAMMA_temp_G, &tGAMMA_temp_B);
	/*fwif_color_encode_gamma_debug(1);	debug*/

	/*2. decode gamma data to gamma curve*/
	fwif_color_gamma_decode(GOut_R, GOut_G, GOut_B, tGAMMA_temp_R, tGAMMA_temp_G, tGAMMA_temp_B);
	/*fwif_color_decode_gamma_debug(1);*/	/*debug*/

	/*3. process the gamma curve*/
	/*do nothing for main branch*/


	/*4. s - curve for gamma*/
	/*fwif_color_gamma_blending_S_curve(Gamma_Sindex, Gamma_Shigh, Gamma_Slow);*/
	/*fwif_color_decode_gamma_debug(2);*/	/*debug*/


	/*5. gamma curve data protect and debug*/
	fwif_color_gamma_curve_data_protect(GOut_R, GOut_G, GOut_B);		/*DO NOT mark this, must check*/


	/*6. encode gamma cuve to gamma data*/
#ifdef ENABLE_xvYcc
	fwif_color_gamma_remmping_for_xvYcc(src_idx, GOut_R, GOut_G, GOut_B);
#endif
	/*fwif_color_gamma_encode();	*/ /*=== marked by Elsie ===*/
#ifdef CONFIG_RTK_GAMUT_SUPPORT
	fwif_color_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B, Power1div22Gamma, Power1div22Gamma, Power1div22Gamma);
#else
	fwif_color_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B, GOut_R, GOut_G, GOut_B);
#endif
	/*fwif_color_encode_gamma_debug(2);	*/ /*debug*/

	/*7. write gamma table*/
	fwif_color_set_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B);
	fwif_color_gamma_control_back(display, 1);

}
void fwif_set_ddomainISR_gamma(unsigned char Gamma_Mode, unsigned char Gamma_Sindex, unsigned char Gamma_Shigh, unsigned char Gamma_Slow)
{
	unsigned char display = 0;
	unsigned char src_idx = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	down(&Gamma_Semaphore);

	/*0. gamma curve reg control*/
	//fwif_color_gamma_control_front(display);

	/*1. choose encode gamma data*/
	fwif_color_get_gamma_default(Gamma_Mode, &tGAMMA_temp_R, &tGAMMA_temp_G, &tGAMMA_temp_B);
	/*fwif_color_encode_gamma_debug(1);	debug*/

	/*2. decode gamma data to gamma curve*/
	fwif_color_gamma_decode(GOut_R, GOut_G, GOut_B, tGAMMA_temp_R, tGAMMA_temp_G, tGAMMA_temp_B);
	/*fwif_color_decode_gamma_debug(1);*/	/*debug*/

	/*3. process the gamma curve*/
	/*do nothing for main branch*/


	/*4. s - curve for gamma*/
	fwif_color_gamma_blending_S_curve(Gamma_Sindex, Gamma_Shigh, Gamma_Slow);
	/*fwif_color_decode_gamma_debug(2);*/	/*debug*/


	/*5. gamma curve data protect and debug*/
	fwif_color_gamma_curve_data_protect(GOut_R, GOut_G, GOut_B);		/*DO NOT mark this, must check*/


	/*6. encode gamma cuve to gamma data*/
#ifdef ENABLE_xvYcc
	fwif_color_gamma_remmping_for_xvYcc(src_idx, GOut_R, GOut_G, GOut_B);
#endif
	/*fwif_color_gamma_encode();	*/ /*=== marked by Elsie ===*/
	fwif_color_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B, GOut_R, GOut_G, GOut_B);
	/*fwif_color_encode_gamma_debug(2);	*/ /*debug*/

	/*7. write gamma table*/
	fwif_color_set_ddomainISR_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B);

	up(&Gamma_Semaphore);
}

void fwif_set_software_gamma(unsigned char Gamma_Mode, unsigned char Gamma_Sindex, unsigned char Gamma_Shigh, unsigned char Gamma_Slow)
{
	unsigned char display = 0;
	unsigned char src_idx = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	/*0. gamma curve reg control*/
	fwif_color_gamma_control_front(display);

	/*1. choose encode gamma data*/
	fwif_color_get_gamma_default(Gamma_Mode, &tGAMMA_temp_R, &tGAMMA_temp_G, &tGAMMA_temp_B);
	/*fwif_color_encode_gamma_debug(1);	debug*/

	/*2. decode gamma data to gamma curve*/
	fwif_color_gamma_decode(GOut_R, GOut_G, GOut_B, tGAMMA_temp_R, tGAMMA_temp_G, tGAMMA_temp_B);
	/*fwif_color_decode_gamma_debug(1);*/	/*debug*/

	/*3. process the gamma curve*/
	/*do nothing for main branch*/


	/*4. s - curve for gamma*/
	fwif_color_gamma_blending_S_curve(Gamma_Sindex, Gamma_Shigh, Gamma_Slow);
	/*fwif_color_decode_gamma_debug(2);*/	/*debug*/


	/*5. gamma curve data protect and debug*/
	fwif_color_gamma_curve_data_protect(GOut_R, GOut_G, GOut_B);		/*DO NOT mark this, must check*/


	/*6. encode gamma cuve to gamma data*/
#ifdef ENABLE_xvYcc
	fwif_color_gamma_remmping_for_xvYcc(src_idx, GOut_R, GOut_G, GOut_B);
#endif
	/*fwif_color_gamma_encode();	*/ /*=== marked by Elsie ===*/
	fwif_color_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B, GOut_R, GOut_G, GOut_B);
	/*fwif_color_encode_gamma_debug(2);	*/ /*debug*/

	/*7. write gamma table*/
	fwif_color_set_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B);
	fwif_color_gamma_control_back(display, 1);

}

UINT8 SCPU_Automa_ISR = 0;

void fwif_color_SetAutoMAFlag(unsigned char flag)
{
	SCPU_Automa_ISR = flag;
}

unsigned char fwif_color_GetAutoMAFlag(void)
{
	return SCPU_Automa_ISR;

}

void fwif_color_disable_VIP(unsigned char flag)
{
	if(flag == _ENABLE)
	{
		drvif_color_disable_VIP();
	}
}

/*===============================================================================================	//NR rock 20140127*/
void fwif_color_RTNR_flow(DRV_NR_Item *ptr)
{
	if (!ptr) {
		VIPprintf("drvif_color_NR_flow table to NULL\n");
		return;
	}

	drvif_color_DRV_RTNR_General_ctrl(&(ptr->S_RTNR_TABLE.S_RTNR_General_ctrl));
	drvif_color_DRV_RTNR_Old_Y(&(ptr->S_RTNR_TABLE.S_RTNR_Old_Y));
	drvif_color_DRV_RTNR_Dark_K(&(ptr->S_RTNR_TABLE.S_RTNR_Dark_K));

// chen 170522
//	drvif_color_DRV_RTNR_New_Method(&(ptr->S_RTNR_TABLE.S_RTNR_New_Method));
//	drvif_color_DRV_RTNR_Mark(&(ptr->S_RTNR_TABLE.S_RTNR_Mask));
//	drvif_color_DRV_RTNR_By_Y(&(ptr->S_RTNR_TABLE.S_RTNR_By_Y));
// end chen 170522
	drvif_color_DRV_RTNR_LSB(&(ptr->S_RTNR_TABLE.S_RTNR_LSB));
	drvif_color_DRV_MCNR_table(&(ptr->S_RTNR_TABLE.S_MCNR_Table));
	/*drvif_color_DRV_RTNR_SNR(&(ptr->S_RTNR_TABLE.S_RTNR_SNR));*/
	drvif_color_DRV_RTNR_MASNR(&(ptr->S_RTNR_TABLE.S_RTNR_MASNR));
	drvif_color_DRV_RTNR_C(&(ptr->S_RTNR_TABLE.S_RTNR_C));
}

void fwif_color_SNR_flow(DRV_NR_Item *ptr)
{
	bool bIsVDSrc;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	DRV_SNR_General_ctrl SNR_General_ctrl = {0};
	
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if (!ptr) {
		VIPprintf("drvif_color_SNR_flow table to NULL\n");
		return;
	}

	drvif_IESM_format_convert();
#ifndef BUILD_QUICK_SHOW

	bIsVDSrc = ((VIP_system_info_structure_table->Input_src_Form) == _SRC_FROM_VDC && vdc_power_status);
#else
	bIsVDSrc = 0;
#endif

	if ((Scaler_APDEM_Arg_Access(DEM_ARG_After_Filter_En, 0, 0) != 0xFF) && (Scaler_APDEM_Arg_Access(DEM_ARG_After_Filter_Val, 0, 0) != 0xFF)) {
		memcpy(&SNR_General_ctrl, &(ptr->S_SNR_TABLE.S_SNR_General_ctrl), sizeof(DRV_SNR_General_ctrl));
		if (Scaler_APDEM_Arg_Access(DEM_ARG_After_Filter_En, 0, 0) == 1) {
			SNR_General_ctrl.SNR_prelpf_snr_sel = (SNR_General_ctrl.SNR_prelpf_snr_sel * Scaler_APDEM_Arg_Access(DEM_ARG_After_Filter_Val, 0, 0))>>7;
		}
		drvif_color_DRV_SNR_General_ctrl(&SNR_General_ctrl, bIsVDSrc);
	} else
		drvif_color_DRV_SNR_General_ctrl(&(ptr->S_SNR_TABLE.S_SNR_General_ctrl), bIsVDSrc);

	drvif_color_DRV_SNR_Spatial_NR_Y(&(ptr->S_SNR_TABLE.S_SNR_Spatial_NR_Y));
	drvif_color_DRV_SNR_Spatial_NR_C(&(ptr->S_SNR_TABLE.S_SNR_Spatial_NR_C));
	drvif_color_DRV_SNR_Modified_LPF(&(ptr->S_SNR_TABLE.S_SNR_Modified_LPF));
	drvif_color_DRV_SNR_Impulse_NR(&(ptr->S_SNR_TABLE.S_SNR_Impulse_NR));
	drvif_color_DRV_SNR_Curve_Mapping_Mode(&(ptr->S_SNR_TABLE.S_SNR_Curve_Mapping_Mode));
	drvif_color_DRV_SNR_NR_Mask(&(ptr->S_SNR_TABLE.S_SNR_NR_Mask));
	drvif_color_DRV_SNR_NR_Mask_W1W2(&(ptr->S_SNR_TABLE.S_SNR_NR_Mask_W1W2)); /*Mac3, leonard wu@20140722*/
	drvif_color_DRV_SNR_Veritcal_NR(&(ptr->S_SNR_TABLE.S_SNR_Veritcal_NR));
	drvif_color_DRV_SNR_Weight_Adjust(&(ptr->S_SNR_TABLE.S_SNR_WeightAdjust)) ; /*add by jyyang Merlin*/
	drvif_color_DRV_SNR_UVEdge(&(ptr->S_SNR_TABLE.S_SNR_UVEdge));/*Merlin*/
	/*drvif_color_DRV_SNR_Mosquito_NR(&(ptr->S_SNR_TABLE.S_SNR_Mosquito_NR));*/ /*Move to MPEG NR due to LG requirement*/
	if (bIsVDSrc)
		drvif_color_DRV_SNR_VD_Spatial_NR_Y(&(ptr->S_SNR_TABLE.S_SNR_VD_Spatial_NR_Y)); /*Mac3*/
}

void fwif_color_PQA_Input_Item_Check(_system_setting_info *VIP_system_info_structure_table, unsigned int *PQA_TABLE, unsigned char table_select)
{
	UINT16 PQA_ITEM_count;
	UINT32 PQA_row, PQA_Reg, NR_Table;
	UINT8 PQA_Input_item, i;

	if (!PQA_TABLE || VIP_system_info_structure_table == NULL) {
		VIPprintf("TABLE is NULL!!= [%s->%d], %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	NR_Table = ((table_select * PQA_MODE_MAX * PQA_ITEM_MAX * PQA_LEVEL_MAX) + (PQA_MODE_OFFSET * PQA_ITEM_MAX * PQA_LEVEL_MAX));

	/*reset*/
	for (i = 0; i < PQA_INPUT_ITEM_MAX; i++)
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[i] = 0;

	for (PQA_ITEM_count = 0; PQA_ITEM_count < PQA_ITEM_MAX; PQA_ITEM_count++) {
		PQA_row = NR_Table + (PQA_ITEM_count * PQA_LEVEL_MAX);
		PQA_Reg = *(PQA_TABLE + PQA_row + PQA_reg);
		PQA_Input_item = *(PQA_TABLE + PQA_row + PQA_input_item);

		if (PQA_Reg == 0xffffffff || PQA_ITEM_count >=  PQA_ITEM_MAX)
			break;
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[PQA_Input_item] = 1;
	}

	NR_Table = ((table_select * PQA_MODE_MAX * PQA_ITEM_MAX * PQA_LEVEL_MAX) + (PQA_MODE_WRITE * PQA_ITEM_MAX * PQA_LEVEL_MAX));

	for (PQA_ITEM_count = 0; PQA_ITEM_count < PQA_ITEM_MAX; PQA_ITEM_count++) {
		PQA_row = NR_Table + (PQA_ITEM_count * PQA_LEVEL_MAX);

		PQA_Reg = *(PQA_TABLE + PQA_row + PQA_reg);
		PQA_Input_item = *(PQA_TABLE + PQA_row + PQA_input_item);

		if (PQA_Reg == 0xffffffff || PQA_ITEM_count >=  PQA_ITEM_MAX)
			break;
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_Item_Check_Flag[PQA_Input_item] = 1;
	}
}

void fwif_color_SetDNR(unsigned char display, unsigned char level)
{
	SLR_VIP_TABLE *gVip_Table = NULL;
	unsigned int *PQA_TABLE = NULL;
	unsigned int *PQA_TABLE_OFFSET_TEMP = NULL;
	unsigned char tab_idx = 0;

	/*=== 20140421 CSFC for vip video fw infomation ====*/
	_system_setting_info *VIP_system_info_structure_table = NULL;

	VIPprintf("fwif_color_SetDNR	%d	%d\n", display, level);

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	VIP_system_info_structure_table->OSD_Info.OSD_NR_Mode = level;
	/*====================================*/

	gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	PQA_TABLE = &(gVip_Table->PQA_Table[0][0][0][0]);
	PQA_TABLE_OFFSET_TEMP = &(VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.OFFSET_TEMP[0]);

	tab_idx = Scaler_GetDNR_table();
	if (tab_idx >= PQA_TABLE_MAX)
		tab_idx = 0;

	fwif_color_PQA_Input_Item_Check(VIP_system_info_structure_table, PQA_TABLE, tab_idx);
	if (level < 4) {	/*Manual Mode*/
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR] = 0;
		/*WaitFor_DEN_STOP();*/
		fwif_color_RTNR_flow(&(gVip_Table->Manual_NR_Table[tab_idx][level]));
		fwif_color_SNR_flow(&(gVip_Table->Manual_NR_Table[tab_idx][level]));
	} else {	/*Auto Mode*/
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR] = 0;
		/*WaitFor_DEN_STOP();*/
		fwif_color_RTNR_flow(&(gVip_Table->Manual_NR_Table[tab_idx][DRV_NR_AUTO]));
		fwif_color_SNR_flow(&(gVip_Table->Manual_NR_Table[tab_idx][DRV_NR_AUTO]));

		drvif_color_PQ_Adaptive_offset_temp(PQA_TABLE, tab_idx, I_DNR, PQA_TABLE_OFFSET_TEMP);
		/*fwif_color_regPQATableOffsetTemp(PQA_TABLE_OFFSET_TEMP);*/
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR] = 1;
	}

	if (Scaler_Get_CinemaMode_PQ() && level == DRV_NR_OFF)
		fwif_color_SNR_flow(&(gVip_Table->Manual_NR_Table[tab_idx][DRV_NR_MID]));
}
void fwif_color_RHAL_SetDNR(unsigned char level)
{
	SLR_VIP_TABLE *gVip_Table = NULL;
	unsigned int *PQA_TABLE = NULL;
	unsigned int *PQA_TABLE_OFFSET_TEMP = NULL;

	/*=== 20140421 CSFC for vip video fw infomation ====*/
	_system_setting_info *VIP_system_info_structure_table = NULL;

	VIPprintf("fwif_color_SetDNR		%d\n", level);

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	VIP_system_info_structure_table->OSD_Info.OSD_NR_Mode = level;
	/*====================================*/

	gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	PQA_TABLE = &(gVip_Table->PQA_Table[0][0][0][0]);
	PQA_TABLE_OFFSET_TEMP = &(VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.OFFSET_TEMP[0]);

	fwif_color_PQA_Input_Item_Check(VIP_system_info_structure_table, PQA_TABLE, fwif_color_get_PQA_table());

	if (level < 4) {	/*Manual Mode*/
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR] = 0;
		/*WaitFor_DEN_STOP();*/
		fwif_color_RTNR_flow(&(gVip_Table->Manual_NR_Table[fwif_color_get_PQA_table()][level]));
		fwif_color_SNR_flow(&(gVip_Table->Manual_NR_Table[fwif_color_get_PQA_table()][level]));
	} else {	/*Auto Mode*/
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR] = 0;
		/*WaitFor_DEN_STOP();*/
		fwif_color_RTNR_flow(&(gVip_Table->Manual_NR_Table[fwif_color_get_PQA_table()][DRV_NR_AUTO]));
		fwif_color_SNR_flow(&(gVip_Table->Manual_NR_Table[fwif_color_get_PQA_table()][DRV_NR_AUTO]));

		drvif_color_PQ_Adaptive_offset_temp(PQA_TABLE, fwif_color_get_PQA_table(), I_DNR, PQA_TABLE_OFFSET_TEMP);
		/*fwif_color_regPQATableOffsetTemp(PQA_TABLE_OFFSET_TEMP);*/
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR] = 1;
	}
}

void fwif_color_RHAL_SetMPEGNR(unsigned char level)
{
	/*int ret;*/
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	_system_setting_info *VIP_system_info_structure_table = NULL;
	unsigned int *PQA_TABLE = NULL;
	unsigned int *PQA_TABLE_OFFSET_TEMP = NULL;

	VIPprintf("fwif_color_SetMPEGNR	%d\n", level);

	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	/*=== 20140421 CSFC for vip video fw infomation ====*/
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	PQA_TABLE = &(gVip_Table->PQA_Table[0][0][0][0]);
	PQA_TABLE_OFFSET_TEMP = &(VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.OFFSET_TEMP[0]);

	if (level < 4) {
		DRV_NR_Item *ptr = NULL;
		/*Manual Mode*/
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_MPEGNR] = 0;
		/*WaitFor_DEN_STOP();*/
		ptr = &(gVip_Table->Manual_NR_Table[fwif_color_get_PQA_table()][level]);
		/*drvif_color_MPEGNR_flow(&(gVip_Table.Manual_NR_Table[fwif_color_get_PQA_table()][level]));*/
		//drvif_color_DRV_MPG_General_ctrl(&(ptr->S_MPG_TABLE.S_MPEGNR_General_ctrl));
		drvif_color_DRV_MPG_H(&(ptr->S_MPG_TABLE.S_MPEGNR_H));
		drvif_color_DRV_MPG_V(&(ptr->S_MPG_TABLE.S_MPEGNR_V));
		drvif_color_DRV_SNR_Mosquito_NR(&(ptr->S_SNR_TABLE.S_SNR_Mosquito_NR)); /*Add from set SNR due to LG requirement*/
	} else {
		DRV_NR_Item *ptr = NULL;
		/*Auto Mode*/
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_MPEGNR] = 0;
		/*WaitFor_DEN_STOP();*/
		ptr = &(gVip_Table->Manual_NR_Table[fwif_color_get_PQA_table()][DRV_NR_AUTO]);
		/*drvif_color_MPEGNR_flow(&(gVip_Table.Manual_NR_Table[fwif_color_get_PQA_table()][DRV_NR_AUTO]));*/
		//drvif_color_DRV_MPG_General_ctrl(&(ptr->S_MPG_TABLE.S_MPEGNR_General_ctrl));
		drvif_color_DRV_MPG_H(&(ptr->S_MPG_TABLE.S_MPEGNR_H));
		drvif_color_DRV_MPG_V(&(ptr->S_MPG_TABLE.S_MPEGNR_V));
		drvif_color_DRV_SNR_Mosquito_NR(&(ptr->S_SNR_TABLE.S_SNR_Mosquito_NR)); /*Add from set SNR due to LG requirement*/
		drvif_color_PQ_Adaptive_offset_temp(PQA_TABLE, fwif_color_get_PQA_table(), I_MPEGNR, PQA_TABLE_OFFSET_TEMP);
		/*fwif_color_regPQATableOffsetTemp(PQA_TABLE_OFFSET_TEMP);*/
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_MPEGNR] = 1;
	}

	VIP_system_info_structure_table->OSD_Info.OSD_MpegNR_Mode = level;
}


void fwif_color_SetMPEGNR(unsigned char display, unsigned char level, unsigned char calledByOSD)
{
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	_system_setting_info *VIP_system_info_structure_table = NULL;
	unsigned int *PQA_TABLE = NULL;
	unsigned int *PQA_TABLE_OFFSET_TEMP = NULL;
	unsigned char tab_idx = 0;

	VIPprintf("fwif_color_SetMPEGNR	%d	%d\n", display, level);

	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	/*=== 20140421 CSFC for vip video fw infomation ====*/
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	PQA_TABLE = &(gVip_Table->PQA_Table[0][0][0][0]);
	PQA_TABLE_OFFSET_TEMP = &(VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.OFFSET_TEMP[0]);

	tab_idx = Scaler_GetDNR_table();
	if (tab_idx >= PQA_TABLE_MAX)
		tab_idx = 0;

	if (level < 4) {
		DRV_NR_Item *ptr = NULL;
		/*Manual Mode*/
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_MPEGNR] = 0;
		/*WaitFor_DEN_STOP();*/
		ptr = &(gVip_Table->Manual_NR_Table[tab_idx][level]);
		/*drvif_color_MPEGNR_flow(&(gVip_Table.Manual_NR_Table[Scaler_GetDNR_table()][level]));*/
		//drvif_color_DRV_MPG_General_ctrl(&(ptr->S_MPG_TABLE.S_MPEGNR_General_ctrl));
		drvif_color_DRV_MPG_H(&(ptr->S_MPG_TABLE.S_MPEGNR_H));

		drvif_color_DRV_MPG_V(&(ptr->S_MPG_TABLE.S_MPEGNR_V));
		drvif_color_DRV_SNR_Mosquito_NR(&(ptr->S_SNR_TABLE.S_SNR_Mosquito_NR));
		drvif_color_DRV_SNR_Mosquito_NR_En(ptr->S_SNR_TABLE.S_SNR_General_ctrl.SNR_mosquito_detect_en);

		drvif_color_set_DRV_MPG_H_SW_Ctrl(&(ptr->S_MPG_TABLE.S_MPEGNR_H.S_MPEGNR_SW_ctrl));
		drvif_color_set_DRV_MPG_V_SW_Ctrl(&(ptr->S_MPG_TABLE.S_MPEGNR_V.S_MPEGNR_SW_ctrl));


	} else {
		DRV_NR_Item *ptr = NULL;
		/*Auto Mode*/
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_MPEGNR] = 0;
		/*WaitFor_DEN_STOP();*/
		ptr = &(gVip_Table->Manual_NR_Table[tab_idx][DRV_NR_AUTO]);
		/*drvif_color_MPEGNR_flow(&(gVip_Table.Manual_NR_Table[Scaler_GetDNR_table()][DRV_NR_AUTO]));*/
		//drvif_color_DRV_MPG_General_ctrl(&(ptr->S_MPG_TABLE.S_MPEGNR_General_ctrl));
		drvif_color_DRV_MPG_H(&(ptr->S_MPG_TABLE.S_MPEGNR_H));
		drvif_color_DRV_MPG_V(&(ptr->S_MPG_TABLE.S_MPEGNR_V));
		drvif_color_PQ_Adaptive_offset_temp(PQA_TABLE, tab_idx, I_MPEGNR, PQA_TABLE_OFFSET_TEMP);
		/*fwif_color_regPQATableOffsetTemp(PQA_TABLE_OFFSET_TEMP);*/
		drvif_color_DRV_SNR_Mosquito_NR(&(ptr->S_SNR_TABLE.S_SNR_Mosquito_NR));
		drvif_color_DRV_SNR_Mosquito_NR_En(ptr->S_SNR_TABLE.S_SNR_General_ctrl.SNR_mosquito_detect_en);
		drvif_color_set_DRV_MPG_H_SW_Ctrl(&(ptr->S_MPG_TABLE.S_MPEGNR_H.S_MPEGNR_SW_ctrl));
		drvif_color_set_DRV_MPG_V_SW_Ctrl(&(ptr->S_MPG_TABLE.S_MPEGNR_V.S_MPEGNR_SW_ctrl));

		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_MPEGNR] = 1;
	}

	VIP_system_info_structure_table->OSD_Info.OSD_MpegNR_Mode = level;
}

DRV_RTNR_General_ctrl *fwif_color_GetRTNR_General(unsigned char source, unsigned char level)
{
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}

	if (source >= PQA_TABLE_MAX)
		source = 0;
	if (level >= DRV_NR_Level_MAX)
		level = 0;
	return &(gVip_Table->Manual_NR_Table[source][level].S_RTNR_TABLE.S_RTNR_General_ctrl);
}
void fwif_color_set_iESM(unsigned char display, unsigned char value)
{
	DRV_IEdgeSmooth_Coef *ptr = NULL;
	DRV_IEdgeSmooth_Coef I_EDGE_Smooth_Coef_disable = {{0}};
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	ptr = &(gVip_Table->I_EDGE_Smooth_Coef[value]);
	/*drvif_color_IEDGESMOOTH_flow(&(gVip_Table.I_EDGE_Smooth_Coef[value]));*/
	if (value >= 255) {
		ptr = &I_EDGE_Smooth_Coef_disable;
	}
	drvif_color_DRV_IESM_Basic(&(ptr->S_IESM_Basic));
	//drvif_color_DRV_DLTI_2D(&(ptr->S_DLTI_2D)); //mac6p removed IESM DLTI2D
	drvif_color_DRV_IESM_Additional_setting(&(ptr->S_IESM_Additional_setting));
}

void fwif_color_set_PQA_Input_table(unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_table_select = value;

}

void fwif_color_set_PQA_table(unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select = value;

	/* MA I SNR and EGSM */
	fwif_color_set_MA_SNR_IESM_TBL(value);

}

unsigned char fwif_color_get_PQA_Input_table(void)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	return VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_Input_table_select;
}

unsigned char fwif_color_get_PQA_table(void)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}
	return  VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.PQA_table_select;
}
void fwif_color_set_MA_SNR_IESM_TBL(unsigned char which_TBL)
{
#ifndef BUILD_QUICK_SHOW

	DRV_MA_SNR_IESM_Coef data;
	_system_setting_info *system_setting_info =(_system_setting_info *) Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	system_setting_info->PQ_Setting_Info.sMA_SNR_IESM_Coef_CTRL.TBL_sel = which_TBL;
	memcpy(&data, &(pq_misc_MA_SNR_IESM_TBL[which_TBL]), sizeof(DRV_MA_SNR_IESM_Coef));

	VIP_RPC_system_info_structure_table->MASNR_Ctrl.RTNR_isnr_Get_Motion_En = data.S_MA_ISNR_GetMotion.RTNR_isnr_Get_Motion_En;
	VIP_RPC_system_info_structure_table->MASNR_Ctrl.nr_motion_en = data.S_MA_SNR_Motion_Weight1.nr_motion_en;
	VIP_RPC_system_info_structure_table->MASNR_Ctrl.IESM_Motion_En = data.S_MA_IESM_Motion_Teeth.IESM_Motion_En;
	VIP_RPC_system_info_structure_table->MASNR_Ctrl.AVLPF_Motion_En = data.S_MA_IESM_Motion_Teeth.AVLPF_Motion_En;

	if (drv_memory_Get_MASNR_Mem_Ready_Flag(0) == 0) {
		data.S_MA_ISNR_GetMotion.RTNR_isnr_Get_Motion_En = 0;
		data.S_MA_SNR_Motion_Weight1.nr_motion_en = 0;
		data.S_MA_IESM_Motion_Teeth.IESM_Motion_En = 0;
		data.S_MA_IESM_Motion_Teeth.AVLPF_Motion_En = 0;
	}

	// add for Merlin2 (jyyang 20160620)
	drvif_color_MASNR_DB_Enable();
	drvif_color_DRV_MA_ISNR_GetMotion(&data.S_MA_ISNR_GetMotion);
	drvif_color_DRV_MA_SNR_Motion_Weight1(&data.S_MA_SNR_Motion_Weight1);
	drvif_color_DRV_MA_SNR_Motion_Weight2(&data.S_MA_SNR_Motion_Weight2);
	drvif_color_DRV_MA_IESM_Motion_Ctrl(&data.S_MA_IESM_Motion_Teeth);
	drvif_color_DRV_MA_IESM_Motion_Weight(&data.S_MA_IESM_Motion_Weight);
	drvif_color_DRV_MA_VLPF_Motion_Weight(&data.S_MA_VLPF_Motion_Weight);
	drvif_color_MASNR_DB_Apply();
#endif	
}

void fwif_color_set_TNRXC_Ctrl(unsigned char src_idx, unsigned char TableIdx)
{
	DRV_di_TNR_XC_CTRL data;
	extern DRV_di_TNR_XC_table di_TNR_XC_table[di_TNR_XC_table_MAX];

	if (TableIdx >=  di_TNR_XC_table_MAX)
	TableIdx = 0;

	data.cp_temporal_xc_en = di_TNR_XC_table[TableIdx].di_TNR_XC_CTRL.cp_temporal_xc_en;
	data.cp_temporalenable = di_TNR_XC_table[TableIdx].di_TNR_XC_CTRL.cp_temporalenable;
	data.cp_temporalthly = di_TNR_XC_table[TableIdx].di_TNR_XC_CTRL.cp_temporalthly;

	drvif_color_di_set_TNRXC_Ctrl((DRV_di_TNR_XC_CTRL *) (&data));
}
void fwif_color_set_TNRXC_MK2(unsigned char src_idx, unsigned char TableIdx)
{
	DRV_di_TNR_XC_MKII data;
	extern DRV_di_TNR_XC_table di_TNR_XC_table[di_TNR_XC_table_MAX];

	if (TableIdx >=  di_TNR_XC_table_MAX)
		TableIdx = 0;

	data.tnrxc_mkii_en = di_TNR_XC_table[TableIdx].di_TNR_XC_MKII.tnrxc_mkii_en;
	data.tnrxc_mkii_thc1 = di_TNR_XC_table[TableIdx].di_TNR_XC_MKII.tnrxc_mkii_thc1;
	data.tnrxc_mkii_thc2 = di_TNR_XC_table[TableIdx].di_TNR_XC_MKII.tnrxc_mkii_thc2;
	data.tnrxc_mkii_thy = di_TNR_XC_table[TableIdx].di_TNR_XC_MKII.tnrxc_mkii_thy;

	drvif_color_di_set_TNRXC_Mk2((DRV_di_TNR_XC_MKII *)(&data));
}



void fwif_color_Set_NRtemporal(unsigned char display, unsigned char value)
{
	drvif_color_noisereduction_temporal(display, value);
}

/*===============================================================================================	//NR rock 20140127*/


/*==============================rord.tsao start===================================================*/

void fwif_color_set_shp_driver(unsigned char src_idx, unsigned char value)
{
	drvif_color_TwoD_peaking_AdaptiveCtrl(_MAIN_DISPLAY, value);
	drvif_color_sharpness_adjust(_MAIN_DISPLAY, value);		/*Texture*/
	drvif_color_2dpk_edge_Coef_apply(_MAIN_DISPLAY, value);	/*Edge*/
	drvif_color_2dpk_Vertical_Coef_apply(_MAIN_DISPLAY, value);	/*Vertical*/
	drvif_color_2dpk_MKIII_init(_MAIN_DISPLAY, value);

}
void fwif_color_sharpness_osd_adjust(unsigned char src_idx, unsigned char value)
{
	/*VIPprintf("[Scalercolor]fwif_color_sharpness_osd_adjust = %d\n", value);*/
	drvif_color_TwoD_peaking_AdaptiveCtrl(_MAIN_DISPLAY, value);/*sharpness MKII from Magellan, 20130515*/
	drvif_color_sharpness_adjust(_MAIN_DISPLAY, value);		/*Texture*/
	drvif_color_2dpk_edge_Coef_apply(_MAIN_DISPLAY,  Scaler_GetSharpness());	/*Edge*/
	drvif_color_2dpk_Vertical_Coef_apply(_MAIN_DISPLAY, value);	/*Vertical*/

}

void fwif_color_set_CDSTable_level(unsigned char table, unsigned char value)
{
	//printk("yuan,CDSTable index=%d\n",table);

	unsigned char sharp = 0, level = 0;
	signed short data = 0;
	signed short max_value = 0, min_value = 0;
	//DRV_Sharpness_Level sharp_gain;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();

	color_sharp_dm_cds_peaking_gain_cm0_tex_RBUS		color_sharp_dm_cds_peaking_gain_cm0_tex_reg;
	color_sharp_dm_cds_peaking_bound_cm0_tex_RBUS		color_sharp_dm_cds_peaking_bound_cm0_tex_reg;
	color_sharp_dm_cds_peaking_gain_cm0_edge_RBUS		color_sharp_dm_cds_peaking_gain_cm0_edge_reg;
	color_sharp_dm_cds_peaking_bound_cm0_edge_RBUS		color_sharp_dm_cds_peaking_bound_cm0_edge_reg;

	color_sharp_dm_cds_peaking_gain_cm1_tex_RBUS		color_sharp_dm_cds_peaking_gain_cm1_tex_reg;
	color_sharp_dm_cds_peaking_bound_cm1_tex_RBUS		color_sharp_dm_cds_peaking_bound_cm1_tex_reg;
	color_sharp_dm_cds_peaking_gain_cm1_edge_RBUS		color_sharp_dm_cds_peaking_gain_cm1_edge_reg;
	color_sharp_dm_cds_peaking_bound_cm1_edge_RBUS		color_sharp_dm_cds_peaking_bound_cm1_edge_reg;


	color_sharp_dm_cds_peaking_gain_cm2_tex_RBUS		color_sharp_dm_cds_peaking_gain_cm2_tex_reg;
	color_sharp_dm_cds_peaking_bound_cm2_tex_RBUS		color_sharp_dm_cds_peaking_bound_cm2_tex_reg;
	color_sharp_dm_cds_peaking_gain_cm2_edge_RBUS		color_sharp_dm_cds_peaking_gain_cm2_edge_reg;
	color_sharp_dm_cds_peaking_bound_cm2_edge_RBUS		color_sharp_dm_cds_peaking_bound_cm2_edge_reg;

	color_sharp_dm_cds_peaking_gain_cm3_tex_RBUS		color_sharp_dm_cds_peaking_gain_cm3_tex_reg;
	color_sharp_dm_cds_peaking_bound_cm3_tex_RBUS		color_sharp_dm_cds_peaking_bound_cm3_tex_reg;
	color_sharp_dm_cds_peaking_gain_cm3_edge_RBUS		color_sharp_dm_cds_peaking_gain_cm3_edge_reg;
	color_sharp_dm_cds_peaking_bound_cm3_edge_RBUS		color_sharp_dm_cds_peaking_bound_cm3_edge_reg;

	color_sharp_dm_cds_peaking_gain_cm0_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_TEX_reg);
	color_sharp_dm_cds_peaking_bound_cm0_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_TEX_reg);
	color_sharp_dm_cds_peaking_gain_cm0_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_EDGE_reg);
	color_sharp_dm_cds_peaking_bound_cm0_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_EDGE_reg);


	color_sharp_dm_cds_peaking_gain_cm1_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_TEX_reg);
	color_sharp_dm_cds_peaking_bound_cm1_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_TEX_reg);
	color_sharp_dm_cds_peaking_gain_cm1_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_EDGE_reg);
	color_sharp_dm_cds_peaking_bound_cm1_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_EDGE_reg);


	color_sharp_dm_cds_peaking_gain_cm2_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_TEX_reg);
	color_sharp_dm_cds_peaking_bound_cm2_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_TEX_reg);
	color_sharp_dm_cds_peaking_gain_cm2_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_EDGE_reg);
	color_sharp_dm_cds_peaking_bound_cm2_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_EDGE_reg);


	color_sharp_dm_cds_peaking_gain_cm3_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_TEX_reg);
	color_sharp_dm_cds_peaking_bound_cm3_tex_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_TEX_reg);
	color_sharp_dm_cds_peaking_gain_cm3_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_EDGE_reg);
	color_sharp_dm_cds_peaking_bound_cm3_edge_reg.regValue = IoReg_Read32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_EDGE_reg);

	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	VIP_system_info_structure_table->OSD_Info.OSD_Sharpness = value;

	if (table >= Sharp_table_num)
		table = 0;

	sharp = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_SHARPNESS, value);
	level = (sharp*100) >> 8;
	//printk("yuan,sharplevel=%d\n",level);

	min_value =0;

/* CM0 edge*/
	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM0_Ctrl.edge_gain_neg;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_gain_cm0_edge_reg.gain_neg=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM0_Ctrl.edge_gain_pos;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_gain_cm0_edge_reg.gain_pos=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM0_Ctrl.edge_hv_pos;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	color_sharp_dm_cds_peaking_bound_cm0_edge_reg.hv_pos=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM0_Ctrl.edge_hv_neg;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	color_sharp_dm_cds_peaking_bound_cm0_edge_reg.hv_neg=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM0_Ctrl.edge_lv;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_bound_cm0_edge_reg.lv=data;
	min_value =0;

/* CM0 text*/
	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM0_Ctrl.tex_gain_neg;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_gain_cm0_tex_reg.gain_neg=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM0_Ctrl.tex_gain_pos;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_gain_cm0_tex_reg.gain_pos=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM0_Ctrl.tex_hv_pos;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	color_sharp_dm_cds_peaking_bound_cm0_tex_reg.hv_pos=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM0_Ctrl.tex_hv_neg;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	color_sharp_dm_cds_peaking_bound_cm0_tex_reg.hv_neg =data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM0_Ctrl.tex_lv;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_bound_cm0_tex_reg.lv=data;

//---------------------------------------------------------------------------

	/* CM1 edge*/
	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM1_Ctrl.edge_gain_neg;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_gain_cm1_edge_reg.gain_neg=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM1_Ctrl.edge_gain_pos;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_gain_cm1_edge_reg.gain_pos=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM1_Ctrl.edge_hv_pos;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	color_sharp_dm_cds_peaking_bound_cm1_edge_reg.hv_pos=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM1_Ctrl.edge_hv_neg;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	color_sharp_dm_cds_peaking_bound_cm1_edge_reg.hv_neg=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM1_Ctrl.edge_lv;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_bound_cm1_edge_reg.lv=data;
	min_value =0;

	/* CM1 text*/
	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM1_Ctrl.tex_gain_neg;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_gain_cm1_tex_reg.gain_neg=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM1_Ctrl.tex_gain_pos;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_gain_cm1_tex_reg.gain_pos=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM1_Ctrl.tex_hv_pos;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	color_sharp_dm_cds_peaking_bound_cm1_tex_reg.hv_pos=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM1_Ctrl.tex_hv_neg;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	color_sharp_dm_cds_peaking_bound_cm1_tex_reg.hv_neg =data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM1_Ctrl.tex_lv;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_bound_cm1_tex_reg.lv=data;

	//---------------------------------------------------------------------------
	/* CM2 edge*/
	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM2_Ctrl.edge_gain_neg;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_gain_cm2_edge_reg.gain_neg=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM2_Ctrl.edge_gain_pos;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_gain_cm2_edge_reg.gain_pos=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM2_Ctrl.edge_hv_pos;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	color_sharp_dm_cds_peaking_bound_cm2_edge_reg.hv_pos=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM2_Ctrl.edge_hv_neg;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	color_sharp_dm_cds_peaking_bound_cm2_edge_reg.hv_neg=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM2_Ctrl.edge_lv;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_bound_cm2_edge_reg.lv=data;
	min_value =0;

	/* CM2 text*/
	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM2_Ctrl.tex_gain_neg;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_gain_cm2_tex_reg.gain_neg=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM2_Ctrl.tex_gain_pos;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_gain_cm2_tex_reg.gain_pos=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM2_Ctrl.tex_hv_pos;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	color_sharp_dm_cds_peaking_bound_cm2_tex_reg.hv_pos=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM2_Ctrl.tex_hv_neg;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	color_sharp_dm_cds_peaking_bound_cm2_tex_reg.hv_neg =data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM2_Ctrl.tex_lv;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_bound_cm2_tex_reg.lv=data;

	//---------------------------------------------------------------------------
	/* CM3 edge*/
	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM3_Ctrl.edge_gain_neg;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_gain_cm3_edge_reg.gain_neg=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM3_Ctrl.edge_gain_pos;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_gain_cm3_edge_reg.gain_pos=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM3_Ctrl.edge_hv_pos;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	color_sharp_dm_cds_peaking_bound_cm3_edge_reg.hv_pos=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM3_Ctrl.edge_hv_neg;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	color_sharp_dm_cds_peaking_bound_cm3_edge_reg.hv_neg=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM3_Ctrl.edge_lv;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_bound_cm3_edge_reg.lv=data;
	min_value =0;

	/* CM3 text*/
	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM3_Ctrl.tex_gain_neg;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_gain_cm3_tex_reg.gain_neg=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM3_Ctrl.tex_gain_pos;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_gain_cm3_tex_reg.gain_pos=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM3_Ctrl.tex_hv_pos;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	color_sharp_dm_cds_peaking_bound_cm3_tex_reg.hv_pos=data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM3_Ctrl.tex_hv_neg;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	color_sharp_dm_cds_peaking_bound_cm3_tex_reg.hv_neg =data;

	max_value = 2*gVip_Table->CDS_ini[table].CDS_CM3_Ctrl.tex_lv;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	color_sharp_dm_cds_peaking_bound_cm3_tex_reg.lv=data;

	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_TEX_reg, color_sharp_dm_cds_peaking_gain_cm0_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_TEX_reg, color_sharp_dm_cds_peaking_bound_cm0_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM0_EDGE_reg, color_sharp_dm_cds_peaking_gain_cm0_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM0_EDGE_reg, color_sharp_dm_cds_peaking_bound_cm0_edge_reg.regValue);

	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_TEX_reg, color_sharp_dm_cds_peaking_gain_cm1_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_TEX_reg, color_sharp_dm_cds_peaking_bound_cm1_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM1_EDGE_reg, color_sharp_dm_cds_peaking_gain_cm1_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM1_EDGE_reg, color_sharp_dm_cds_peaking_bound_cm1_edge_reg.regValue);

	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_TEX_reg, color_sharp_dm_cds_peaking_gain_cm2_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_TEX_reg, color_sharp_dm_cds_peaking_bound_cm2_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM2_EDGE_reg, color_sharp_dm_cds_peaking_gain_cm2_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM2_EDGE_reg, color_sharp_dm_cds_peaking_bound_cm2_edge_reg.regValue);

	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_TEX_reg, color_sharp_dm_cds_peaking_gain_cm3_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_TEX_reg, color_sharp_dm_cds_peaking_bound_cm3_tex_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_GAIN_CM3_EDGE_reg, color_sharp_dm_cds_peaking_gain_cm3_edge_reg.regValue);
	IoReg_Write32(COLOR_SHARP_DM_CDS_PEAKING_BOUND_CM3_EDGE_reg, color_sharp_dm_cds_peaking_bound_cm3_edge_reg.regValue);

}

void fwif_color_set_sharpness_level(unsigned char table, unsigned char value)
{
	unsigned char sharp = 0, level = 0;
	signed short data = 0;
	signed short max_value = 0, min_value = 0;
	DRV_Sharpness_Level sharp_gain;
	_system_setting_info *VIP_system_info_structure_table = NULL;

	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	VIP_system_info_structure_table->OSD_Info.OSD_Sharpness = value;

	if (table >= Sharp_table_num)
		table = 0;

	sharp = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_SHARPNESS, value);
	level = (sharp*100) >> 8;

/*cal edge_shp*/
	max_value = gVip_Table->Ddomain_SHPTable[table].Edge_Shp_set.G_Pos_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Edge_Shp_set.G_Pos_Min;
	data = min_value+(max_value - min_value)*level/100;

	if (data > 255)
		data = 255;
	sharp_gain.Edge_Level.G_Pos_Gain  = data;

	max_value = gVip_Table->Ddomain_SHPTable[table].Edge_Shp_set.G_Neg_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Edge_Shp_set.G_Neg_Min;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	sharp_gain.Edge_Level.G_Neg_Gain  = data;

	max_value = gVip_Table->Ddomain_SHPTable[table].Edge_Shp_set.HV_POS_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Edge_Shp_set.HV_POS_Min;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	sharp_gain.Edge_Level.HV_POS_Gain = data;

	max_value = gVip_Table->Ddomain_SHPTable[table].Edge_Shp_set.HV_NEG_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Edge_Shp_set.HV_NEG_Min;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	sharp_gain.Edge_Level.HV_NEG_Gain = data;

	max_value = gVip_Table->Ddomain_SHPTable[table].Edge_Shp_set.LV_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Edge_Shp_set.LV_Min;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	sharp_gain.Edge_Level.LV_Gain = data;

/*cal Texture_shp*/
	max_value = gVip_Table->Ddomain_SHPTable[table].Texture_Shp_set.G_Pos_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Texture_Shp_set.G_Pos_Min;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	sharp_gain.Texture_Level.G_Pos_Gain  = data;

	max_value = gVip_Table->Ddomain_SHPTable[table].Texture_Shp_set.G_Neg_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Texture_Shp_set.G_Neg_Min;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	sharp_gain.Texture_Level.G_Neg_Gain  = data;

	max_value = gVip_Table->Ddomain_SHPTable[table].Texture_Shp_set.HV_POS_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Texture_Shp_set.HV_POS_Min;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	sharp_gain.Texture_Level.HV_POS_Gain = data;

	max_value = gVip_Table->Ddomain_SHPTable[table].Texture_Shp_set.HV_NEG_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Texture_Shp_set.HV_NEG_Min;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 1023)
		data = 1023;
	sharp_gain.Texture_Level.HV_NEG_Gain = data;

	max_value = gVip_Table->Ddomain_SHPTable[table].Texture_Shp_set.LV_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Texture_Shp_set.LV_Min;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	sharp_gain.Texture_Level.LV_Gain = data;

/*cal vertical*/
	max_value = gVip_Table->Ddomain_SHPTable[table].Vertical.G_Pos_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Vertical.G_Pos_Min;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	sharp_gain.Vertical_Level.G_Pos_Gain  = data;

	max_value = gVip_Table->Ddomain_SHPTable[table].Vertical.G_Neg_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Vertical.G_Neg_Min;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	sharp_gain.Vertical_Level.G_Neg_Gain  = data;

	max_value = gVip_Table->Ddomain_SHPTable[table].Vertical.HV_POS_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Vertical.HV_POS_Min;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	sharp_gain.Vertical_Level.HV_POS_Gain = data;

	max_value = gVip_Table->Ddomain_SHPTable[table].Vertical.HV_NEG_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Vertical.HV_NEG_Min;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	sharp_gain.Vertical_Level.HV_NEG_Gain = data;

	max_value = gVip_Table->Ddomain_SHPTable[table].Vertical.LV_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Vertical.LV_Min;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	sharp_gain.Vertical_Level.LV_Gain = data;

	/*cal vertical Edge  */

	max_value = gVip_Table->Ddomain_SHPTable[table].Vertical_edg.G_Pos_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Vertical_edg.G_Pos_Min;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	sharp_gain.Vertical_Edge_Level.G_Pos_Gain  = data;

	max_value = gVip_Table->Ddomain_SHPTable[table].Vertical_edg.G_Neg_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Vertical_edg.G_Neg_Min;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	sharp_gain.Vertical_Edge_Level.G_Neg_Gain  = data;

	max_value = gVip_Table->Ddomain_SHPTable[table].Vertical_edg.HV_POS_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Vertical_edg.HV_POS_Min;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	sharp_gain.Vertical_Edge_Level.HV_POS_Gain = data;

	max_value = gVip_Table->Ddomain_SHPTable[table].Vertical_edg.HV_NEG_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Vertical_edg.HV_NEG_Min;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	sharp_gain.Vertical_Edge_Level.HV_NEG_Gain = data;

	max_value = gVip_Table->Ddomain_SHPTable[table].Vertical_edg.LV_Max;
	min_value = gVip_Table->Ddomain_SHPTable[table].Vertical_edg.LV_Min;
	data = min_value+(max_value - min_value)*level/100;
	if (data > 255)
		data = 255;
	sharp_gain.Vertical_Edge_Level.LV_Gain = data;


	drvif_color_set_Sharpness_level((DRV_Sharpness_Level *)&sharp_gain);}

void fwif_color_set_sharpness_table(unsigned char src_idx, unsigned char value)
{
	static DRV_Sharpness_Table sharpness = {0};
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		return;
	}

	if (value >= Sharp_table_num)
	{
		value = 0;
	}
	memcpy(&sharpness,&(gVip_Table->Ddomain_SHPTable[value]),sizeof(VIP_Sharpness_Table));

	if ((Scaler_APDEM_Arg_Access(DEM_ARG_After_Filter_En, 0, 0) != 0xFF) && (Scaler_APDEM_Arg_Access(DEM_ARG_After_Filter_Val, 0, 0) != 0xFF)) {
		if (Scaler_APDEM_Arg_Access(DEM_ARG_After_Filter_En, 0, 0) == 1) {
			sharpness.Max_Min_Lpf.Weit_0 = (1024 * Scaler_APDEM_Arg_Access(DEM_ARG_After_Filter_Val, 0, 0))>>7;
			sharpness.Max_Min_Lpf.Weit_1 = (1024 * Scaler_APDEM_Arg_Access(DEM_ARG_After_Filter_Val, 0, 0))>>7;
			sharpness.Max_Min_Lpf.Weit_2 = (1024 * Scaler_APDEM_Arg_Access(DEM_ARG_After_Filter_Val, 0, 0))>>7;
			sharpness.Max_Min_Lpf.Weit_3 = (1024 * Scaler_APDEM_Arg_Access(DEM_ARG_After_Filter_Val, 0, 0))>>7;
			sharpness.Max_Min_Lpf.Weit_4 = (1024 * Scaler_APDEM_Arg_Access(DEM_ARG_After_Filter_Val, 0, 0))>>7;
			sharpness.Max_Min_Lpf.Weit_5 = (1024 * Scaler_APDEM_Arg_Access(DEM_ARG_After_Filter_Val, 0, 0))>>7;
			sharpness.Max_Min_Lpf.Weit_6 = (1024 * Scaler_APDEM_Arg_Access(DEM_ARG_After_Filter_Val, 0, 0))>>7;
		}
	}
	
	drvif_color_Set_Sharpness_Table((DRV_Sharpness_Table *)&sharpness);
}


void fwif_color_set_MBPK_table(unsigned char src_idx, unsigned char value)
{
	VIP_MBPK_Table MBPK = {{0}};
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	if (value >= MBPK_table_num) {	/* bypass = 255*/
		MBPK.MBPK_Ctrl.mb_peaking_en = 0;
		value = 0;
	} else {
		MBPK.MBPK_Ctrl.mb_peaking_en = gVip_Table->Idomain_MBPKTable[value].MBPK_Ctrl.mb_peaking_en;
	}

	MBPK.MBPK_Ctrl.MB_Ver_Vfilter_data = gVip_Table->Idomain_MBPKTable[value].MBPK_Ctrl.MB_Ver_Vfilter_data;
	MBPK.MBPK_Ctrl.MB_H_En = gVip_Table->Idomain_MBPKTable[value].MBPK_Ctrl.MB_H_En;
	MBPK.MBPK_Ctrl.MB_V_En = gVip_Table->Idomain_MBPKTable[value].MBPK_Ctrl.MB_V_En;
	MBPK.MBPK_Ctrl.MB_SNR_flag_en = gVip_Table->Idomain_MBPKTable[value].MBPK_Ctrl.MB_SNR_flag_en;
	MBPK.MBPK_Ctrl.MB_MOS_coring = gVip_Table->Idomain_MBPKTable[value].MBPK_Ctrl.MB_MOS_coring;
	MBPK.MBPK_Ctrl.MB_WEI_coring = gVip_Table->Idomain_MBPKTable[value].MBPK_Ctrl.MB_WEI_coring;

	MBPK.MBPK_H_Coef.MB_Hor_Filter_C0 = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Coef.MB_Hor_Filter_C0;
	MBPK.MBPK_H_Coef.MB_Hor_Filter_C1 = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Coef.MB_Hor_Filter_C1;
	MBPK.MBPK_H_Coef.MB_Hor_Filter_C2 = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Coef.MB_Hor_Filter_C2;
	MBPK.MBPK_H_Coef.MB_Hor_Filter_C3 = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Coef.MB_Hor_Filter_C3;
	MBPK.MBPK_H_Coef.MB_Hor_Filter_C4 = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Coef.MB_Hor_Filter_C4;
	MBPK.MBPK_H_Coef.MB_Hor_Filter_C5 = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Coef.MB_Hor_Filter_C5;
	MBPK.MBPK_H_Coef.MB_Hor_Filter_C6 = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Coef.MB_Hor_Filter_C6;

	MBPK.MBPK_H_Table.MB_D2_shift_bit = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Table.MB_D2_shift_bit;
	MBPK.MBPK_H_Table.MB_Gain_Neg = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Table.MB_Gain_Neg;
	MBPK.MBPK_H_Table.MB_Gain_Neg2 = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Table.MB_Gain_Neg2;
	MBPK.MBPK_H_Table.MB_Gain_Pos = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Table.MB_Gain_Pos;
	MBPK.MBPK_H_Table.MB_Gain_Pos2 = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Table.MB_Gain_Pos2;
	MBPK.MBPK_H_Table.MB_HV_Neg = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Table.MB_HV_Neg;
	MBPK.MBPK_H_Table.MB_HV_Pos = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Table.MB_HV_Pos;
	MBPK.MBPK_H_Table.MB_LV = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Table.MB_LV;
	MBPK.MBPK_H_Table.MB_LV2 = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Table.MB_LV2;

	MBPK.MBPK_V_Coef.MB_Ver_Filter_C0 = gVip_Table->Idomain_MBPKTable[value].MBPK_V_Coef.MB_Ver_Filter_C0;
	MBPK.MBPK_V_Coef.MB_Ver_Filter_C1 = gVip_Table->Idomain_MBPKTable[value].MBPK_V_Coef.MB_Ver_Filter_C1;
	MBPK.MBPK_V_Coef.MB_Ver_Filter_C2 = gVip_Table->Idomain_MBPKTable[value].MBPK_V_Coef.MB_Ver_Filter_C2;

	MBPK.MBPK_V_Table.MB_D2_shift_bit_V = gVip_Table->Idomain_MBPKTable[value].MBPK_V_Table.MB_D2_shift_bit_V;
	MBPK.MBPK_V_Table.MB_Gain_Neg2_V = gVip_Table->Idomain_MBPKTable[value].MBPK_V_Table.MB_Gain_Neg2_V;
	MBPK.MBPK_V_Table.MB_Gain_Neg_V = gVip_Table->Idomain_MBPKTable[value].MBPK_V_Table.MB_Gain_Neg_V;
	MBPK.MBPK_V_Table.MB_Gain_Pos2_V = gVip_Table->Idomain_MBPKTable[value].MBPK_V_Table.MB_Gain_Pos2_V;
	MBPK.MBPK_V_Table.MB_Gain_Pos_V = gVip_Table->Idomain_MBPKTable[value].MBPK_V_Table.MB_Gain_Pos_V;
	MBPK.MBPK_V_Table.MB_HV_Neg_V = gVip_Table->Idomain_MBPKTable[value].MBPK_V_Table.MB_HV_Neg_V;
	MBPK.MBPK_V_Table.MB_HV_Pos_V = gVip_Table->Idomain_MBPKTable[value].MBPK_V_Table.MB_HV_Pos_V;
	MBPK.MBPK_V_Table.MB_LV2_V = gVip_Table->Idomain_MBPKTable[value].MBPK_V_Table.MB_LV2_V;
	MBPK.MBPK_V_Table.MB_LV_V = gVip_Table->Idomain_MBPKTable[value].MBPK_V_Table.MB_LV_V;

	/*magellan2 end*/

	MBPK.MBPK_H_Deovershoot0.MB_De_Over_H_en = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Deovershoot0.MB_De_Over_H_en;
	MBPK.MBPK_H_Deovershoot0.MB_De_Over_H_S_gain = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Deovershoot0.MB_De_Over_H_S_gain;
	MBPK.MBPK_H_Deovershoot0.MB_De_Over_H_B_range = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Deovershoot0.MB_De_Over_H_B_range;
	MBPK.MBPK_H_Deovershoot0.MB_De_Over_H_S_range = gVip_Table->Idomain_MBPKTable[value].MBPK_H_Deovershoot0.MB_De_Over_H_S_range;

	MBPK.MBPK_V_Deovershoot0.MB_De_Over_V_en = gVip_Table->Idomain_MBPKTable[value].MBPK_V_Deovershoot0.MB_De_Over_V_en;
	MBPK.MBPK_V_Deovershoot0.MB_De_Over_V_S_gain = gVip_Table->Idomain_MBPKTable[value].MBPK_V_Deovershoot0.MB_De_Over_V_S_gain;
	MBPK.MBPK_V_Deovershoot0.MB_De_Over_V_B_range = gVip_Table->Idomain_MBPKTable[value].MBPK_V_Deovershoot0.MB_De_Over_V_B_range;
	MBPK.MBPK_V_Deovershoot0.MB_De_Over_V_S_range = gVip_Table->Idomain_MBPKTable[value].MBPK_V_Deovershoot0.MB_De_Over_V_S_range;

	/*merlin2 end*/

	drvif_color_Set_MBPK_Table((VIP_MBPK_Table *)&MBPK);
}

void fwif_color_set_MBSU_table(unsigned char src_idx, unsigned char value)
{
	VIP_MBPK_Table MBPK = {{0}};
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	if (value >= MBSU_table_num) {	/* bypass = 255*/
		MBPK.MBPK_Ctrl.mb_peaking_en = 0;
		value = 0;
	} else {
		MBPK.MBPK_Ctrl.mb_peaking_en = gVip_Table->Ddomain_MBSUTable[value].MBPK_Ctrl.mb_peaking_en;
	}

	MBPK.MBPK_Ctrl.MB_Ver_Vfilter_data = gVip_Table->Ddomain_MBSUTable[value].MBPK_Ctrl.MB_Ver_Vfilter_data;
	MBPK.MBPK_Ctrl.MB_H_En = gVip_Table->Ddomain_MBSUTable[value].MBPK_Ctrl.MB_H_En;
	MBPK.MBPK_Ctrl.MB_V_En = gVip_Table->Ddomain_MBSUTable[value].MBPK_Ctrl.MB_V_En;
	MBPK.MBPK_Ctrl.MB_SNR_flag_en = gVip_Table->Ddomain_MBSUTable[value].MBPK_Ctrl.MB_SNR_flag_en;
	MBPK.MBPK_Ctrl.MB_MOS_coring = gVip_Table->Ddomain_MBSUTable[value].MBPK_Ctrl.MB_MOS_coring;
	MBPK.MBPK_Ctrl.MB_WEI_coring = gVip_Table->Ddomain_MBSUTable[value].MBPK_Ctrl.MB_WEI_coring;

	MBPK.MBPK_H_Coef.MB_Hor_Filter_C0 = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Coef.MB_Hor_Filter_C0;
	MBPK.MBPK_H_Coef.MB_Hor_Filter_C1 = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Coef.MB_Hor_Filter_C1;
	MBPK.MBPK_H_Coef.MB_Hor_Filter_C2 = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Coef.MB_Hor_Filter_C2;
	MBPK.MBPK_H_Coef.MB_Hor_Filter_C3 = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Coef.MB_Hor_Filter_C3;
	MBPK.MBPK_H_Coef.MB_Hor_Filter_C4 = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Coef.MB_Hor_Filter_C4;
	MBPK.MBPK_H_Coef.MB_Hor_Filter_C5 = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Coef.MB_Hor_Filter_C5;
	MBPK.MBPK_H_Coef.MB_Hor_Filter_C6 = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Coef.MB_Hor_Filter_C6;

	MBPK.MBPK_H_Table.MB_D2_shift_bit = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Table.MB_D2_shift_bit;
	MBPK.MBPK_H_Table.MB_Gain_Neg = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Table.MB_Gain_Neg;
	MBPK.MBPK_H_Table.MB_Gain_Neg2 = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Table.MB_Gain_Neg2;
	MBPK.MBPK_H_Table.MB_Gain_Pos = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Table.MB_Gain_Pos;
	MBPK.MBPK_H_Table.MB_Gain_Pos2 = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Table.MB_Gain_Pos2;
	MBPK.MBPK_H_Table.MB_HV_Neg = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Table.MB_HV_Neg;
	MBPK.MBPK_H_Table.MB_HV_Pos = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Table.MB_HV_Pos;
	MBPK.MBPK_H_Table.MB_LV = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Table.MB_LV;
	MBPK.MBPK_H_Table.MB_LV2 = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Table.MB_LV2;

	MBPK.MBPK_V_Coef.MB_Ver_Filter_C0 = gVip_Table->Ddomain_MBSUTable[value].MBPK_V_Coef.MB_Ver_Filter_C0;
	MBPK.MBPK_V_Coef.MB_Ver_Filter_C1 = gVip_Table->Ddomain_MBSUTable[value].MBPK_V_Coef.MB_Ver_Filter_C1;
	MBPK.MBPK_V_Coef.MB_Ver_Filter_C2 = gVip_Table->Ddomain_MBSUTable[value].MBPK_V_Coef.MB_Ver_Filter_C2;

	MBPK.MBPK_V_Table.MB_D2_shift_bit_V = gVip_Table->Ddomain_MBSUTable[value].MBPK_V_Table.MB_D2_shift_bit_V;
	MBPK.MBPK_V_Table.MB_Gain_Neg2_V = gVip_Table->Ddomain_MBSUTable[value].MBPK_V_Table.MB_Gain_Neg2_V;
	MBPK.MBPK_V_Table.MB_Gain_Neg_V = gVip_Table->Ddomain_MBSUTable[value].MBPK_V_Table.MB_Gain_Neg_V;
	MBPK.MBPK_V_Table.MB_Gain_Pos2_V = gVip_Table->Ddomain_MBSUTable[value].MBPK_V_Table.MB_Gain_Pos2_V;
	MBPK.MBPK_V_Table.MB_Gain_Pos_V = gVip_Table->Ddomain_MBSUTable[value].MBPK_V_Table.MB_Gain_Pos_V;
	MBPK.MBPK_V_Table.MB_HV_Neg_V = gVip_Table->Ddomain_MBSUTable[value].MBPK_V_Table.MB_HV_Neg_V;
	MBPK.MBPK_V_Table.MB_HV_Pos_V = gVip_Table->Ddomain_MBSUTable[value].MBPK_V_Table.MB_HV_Pos_V;
	MBPK.MBPK_V_Table.MB_LV2_V = gVip_Table->Ddomain_MBSUTable[value].MBPK_V_Table.MB_LV2_V;
	MBPK.MBPK_V_Table.MB_LV_V = gVip_Table->Ddomain_MBSUTable[value].MBPK_V_Table.MB_LV_V;

	/*magellan2 end*/

	MBPK.MBPK_H_Deovershoot0.MB_De_Over_H_en = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Deovershoot0.MB_De_Over_H_en;
	MBPK.MBPK_H_Deovershoot0.MB_De_Over_H_S_gain = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Deovershoot0.MB_De_Over_H_S_gain;
	MBPK.MBPK_H_Deovershoot0.MB_De_Over_H_B_range = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Deovershoot0.MB_De_Over_H_B_range;
	MBPK.MBPK_H_Deovershoot0.MB_De_Over_H_S_range = gVip_Table->Ddomain_MBSUTable[value].MBPK_H_Deovershoot0.MB_De_Over_H_S_range;

	MBPK.MBPK_V_Deovershoot0.MB_De_Over_V_en = gVip_Table->Ddomain_MBSUTable[value].MBPK_V_Deovershoot0.MB_De_Over_V_en;
	MBPK.MBPK_V_Deovershoot0.MB_De_Over_V_S_gain = gVip_Table->Ddomain_MBSUTable[value].MBPK_V_Deovershoot0.MB_De_Over_V_S_gain;
	MBPK.MBPK_V_Deovershoot0.MB_De_Over_V_B_range = gVip_Table->Ddomain_MBSUTable[value].MBPK_V_Deovershoot0.MB_De_Over_V_B_range;
	MBPK.MBPK_V_Deovershoot0.MB_De_Over_V_S_range = gVip_Table->Ddomain_MBSUTable[value].MBPK_V_Deovershoot0.MB_De_Over_V_S_range;

	/*merlin2 end*/

	drvif_color_Set_MBSU_Table((VIP_MBPK_Table *)&MBPK);
}


void fwif_color_VIPShpTable_to_DRVShpTable(VIP_Sharpness_Table *VIPShpTable, DRV_Sharpness_Table *DRVShpTable)
{
	memcpy(DRVShpTable,VIPShpTable,sizeof(VIP_Sharpness_Table));
}


void fwif_color_set_shp_vgain(unsigned char src_idx, unsigned char value)
{
	drvif_color_Sharpness_Vertical_Gain_adjust(_MAIN_DISPLAY,  value);
}



void fwif_color_set_cds_table(unsigned char src_idx, unsigned char TableIdx)
{
	VIP_CDS_Table CDS_Table = {0};

	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	if (TableIdx < Sharp_table_num)	/* bypass = 255*/
	memcpy(&CDS_Table, &(gVip_Table->CDS_ini[TableIdx]), sizeof(VIP_CDS_Table));


	drvif_color_set_cds_table((VIP_CDS_Table *) (&CDS_Table));


}

unsigned char lastLUT_tbl_index = 0xff;
void fwif_color_set_3dLUT(UINT8 TableIdx)
{
	/*static unsigned int LUT3D[LUT3D_TBL_ITEM];*/
	unsigned int *LUT3D = NULL;
	extern unsigned int LG_LUT_3D_EXTEND[LUT3D_EXTEND_TBL_Num][LUT3D_TBL_ITEM];
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	unsigned char onlyEnable = 0;
	extern unsigned int LUT_3D[LUT3D_TBL_Num][LUT3D_TBL_ITEM];
	if (gVip_Table == NULL)
		return;

	LUT3D = LUT_3D[0];

	if (TableIdx > 0 && TableIdx <= IDX_3DLUT_MAX) { /* bypass = 0*/
		if (TableIdx <= LUT3D_TBL_Num && TableIdx <= IDX_3DLUT_BIG_TABLE_MAX)
			LUT3D = LUT_3D[TableIdx - 1];
		else
			LUT3D = LG_LUT_3D_EXTEND[TableIdx - 1 - IDX_3DLUT_BIG_TABLE_MAX];

		if (lastLUT_tbl_index == TableIdx - 1)
			onlyEnable = 1;
		else
			lastLUT_tbl_index = TableIdx - 1;
		/*memcpy(&LUT3D, &(gVip_Table->LUT_3D[TableIdx - 1]), sizeof(unsigned int)*LUT3D_TBL_ITEM);*/
	}

	fwif_color_WaitFor_DEN_STOP_UZUDTG();
	if (TableIdx == 0)	//disable
		drvif_color_D_3dLUT(0, LUT3D);
	else {
		if (onlyEnable)
			drvif_color_D_3dLUT_Enable(1);
		else
			drvif_color_D_3dLUT(1, LUT3D);
	}
}



void fwif_color_set_unsharp_mask_driver(unsigned char src_idx, unsigned char value)
{
	extern DRV_Un_Shp_Mask gVipUn_Shp_Mask[16];
	DRV_Un_Shp_Mask gVipUn_Shp_Mask_disable = {0};

	if (value >= sizeof(gVipUn_Shp_Mask)/sizeof(DRV_Un_Shp_Mask))
		drvif_color_sharpness_unsharp_mask(&gVipUn_Shp_Mask_disable);
	else
	drvif_color_sharpness_unsharp_mask(&gVipUn_Shp_Mask[value]);
}



void fwif_color_set_egsm_postshp_level_driver(unsigned char src_idx, unsigned char value)/*this function control postshp level*/
{
	drvif_color_edgesmooth_postshp_adjust(_MAIN_DISPLAY, value);
}

void fwif_color_set_dismd_driver(unsigned char src_idx, unsigned char value)
{
	extern DRV_di_smd_table_t gSMDtable[7];
	DRV_di_smd_table_t gSMDtable_disable = {{0}};

	if (value >= sizeof(gSMDtable)/sizeof(DRV_di_smd_table_t))
		drvif_color_smd_table(&gSMDtable_disable);
	else
	drvif_color_smd_table(&gSMDtable[value]);
}

void fwif_color_set_emf_mk2_driver(unsigned char src_idx, unsigned char value)
{
	extern DRV_EMFMk2 gVipEmfMk2[11];
	DRV_EMFMk2 gVipEmfMk2_disable = {0};

	if (value >= sizeof(gVipEmfMk2)/sizeof(DRV_EMFMk2))
		drvif_color_emfmk2(&gVipEmfMk2_disable);
	else
	drvif_color_emfmk2(&gVipEmfMk2[value]);
}

void fwif_color_set_shp_2Dpk_Seg(unsigned char src_idx, unsigned char value)
{
	extern DRV_2Dpk_Seg gVip2Dpk_Seg[];

	drvif_color_2dpk_seg(&gVip2Dpk_Seg[value]);

}

void fwif_color_set_shp_2Dpk_Tex_Detect(unsigned char src_idx, unsigned char value)
{
	extern DRV_2Dpk_Tex_Detect gVip2Dpk_Tex_Detect[];

	drvif_color_2dpk_tex_detect(&gVip2Dpk_Tex_Detect[value]);
}

void fwif_color_set_TwoD_peaking_AdaptiveCtrl(unsigned char src_idx, unsigned char value)
{
	drvif_color_TwoD_peaking_AdaptiveCtrl(_MAIN_DISPLAY, value);
}


void fwif_color_set_ma_BTR(unsigned char src_idx, unsigned char value)
{
	drvif_color_ma_BTR(value);
}


void fwif_color_set_ma_BER(unsigned char src_idx, unsigned char value)
{
	drvif_color_ma_BER(value);
}


void fwif_color_ma_init(void) /* when turning on auto MA (OSD)*/
{
	/*Deinterlace only Main*/
	/* Thur modified 20090114
	DI_Control_Register_RBUS reg18061000_Reg;
	Frame_Motion_Detector_Threshold_A_RBUS reg18061080_Reg;
	Frame_Motion_Detector_Threshold_B_RBUS reg18061084_Reg;
	Frame_Motion_Detector_Threshold_C_RBUS reg18061088_Reg;
	Field_Teeth_Detector_Threshold_A_RBUS reg1806108c_Reg;
	Field_Teeth_Detector_Threshold_B_and_Anti_CC_and_Still_Logo_RBUS reg18061090_Reg;
	Sawtooth__Threshold_RBUS reg18061094_Reg;
	Compensation_Threshold_A_RBUS reg18061098_Reg;
	Compensation_Threshold_B_RBUS reg1806109c_Reg;
	*/
	histogram_ich1_hist_in_cselect_RBUS		ich1_hist_in_cselect_reg;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("fwif_color_ma_init error!! VIP_system_info_structure_table NULL!!\n");
	} else {
		VIPprintf("fwif_color_ma_init\n");
	}

	Scaler_SetMADI_INIT();


	/* LearnRPG new algo on LV*/
	drvif_color_ma_HFD(1);
	drvif_color_ma_TeethCheck(0);

	ich1_hist_in_cselect_reg.regValue = IoReg_Read32(HISTOGRAM_ICH1_Hist_IN_CSELECT_reg);
	ich1_hist_in_cselect_reg.ch1_his_updatelockmode_en = 0;/*disable for histogram lock, 20130314, CSFC*/
	IoReg_Write32(HISTOGRAM_ICH1_Hist_IN_CSELECT_reg, ich1_hist_in_cselect_reg.regValue);



	drvif_color_ma_BTR(1);

	drvif_color_ma_Pan_Detection(1);

	Scaler_SetMADI_HME(6);
	Scaler_SetMADI_HMC(6);

	Scaler_SetMA_Chroma_Error(1);

// chen 170522
//	drvif_color_ZOOM_MOTION_En(0);
//	drvif_color_ZOOM_MOTION_Level(1);
// end chen 170522
	/*vif_color_ZOOM_MOTION_En(1);*/
	drvif_color_SLOW_MOTION_Level(2);
	/*vif_color_SLOW_MOTION_En(1);*/

	/*drvif_color_ma_444_and_422_mode(3, 0);*/
}



/*chen 0716*/
void fwif_color_set_rtnr_masnr_driver(unsigned char src_idx, unsigned char value)
{
	/*integrate gVipRtnr_Masnr to manual table*/
	#if 0
	extern DRV_Rtnr_Masnr gVipRtnr_Masnr[];

	drvif_color_DRV_RTNR_MASNR(&gVipRtnr_Masnr[value]);
	#endif
}



/*end chen 0716*/


/*
unsigned int  fwif_color_regRTNR_Noise_measure(DRV_NM_Table *ptr)
{
	if (!ptr)
		return FALSE;
		NM_TABLE = ptr;
	return TRUE;
}


void fwif_color_set_RTNR_Noise_Measure(unsigned char display, unsigned char value)
{
	drvif_color_RTNR_Noise_Measure_flow(NM_TABLE+value);
}
*/


/*Level 0~5*/
void fwif_color_set_RTNR_Noise_Measure_flow(unsigned char display, unsigned char value)
{
	extern DRV_rtnr_nm_thd_t gRtnrNmThd[NM_LEVEL];
	extern DRV_rtnr_nm_ctrl_t gRtnrNmCtrl[2];
	DRV_rtnr_nm_thd_t nm_thd;
	DRV_rtnr_nm_ctrl_t nm_ctrl;
	unsigned char value_tmp;
	value_tmp = (value > 5) ? 5 : value;
	//value_tmp = (value < 0) ? 0 : value;

	/*drvif_color_DRV_NM_Init(value_tmp);*/

	nm_ctrl.main_enable = gRtnrNmCtrl[0].main_enable;
	nm_ctrl.start = gRtnrNmCtrl[0].start;
	nm_ctrl.ref_y_en = gRtnrNmCtrl[0].ref_y_en;
	nm_ctrl.fw_yref = gRtnrNmCtrl[0].fw_yref;
	nm_ctrl.fw_ydiff = gRtnrNmCtrl[0].fw_ydiff;
	nm_ctrl.pixmove_en = gRtnrNmCtrl[0].pixmove_en;
	nm_ctrl.varsel = gRtnrNmCtrl[0].varsel;
	nm_ctrl.debug_conti = gRtnrNmCtrl[0].debug_conti;
	nm_ctrl.samplingbit = gRtnrNmCtrl[0].samplingbit;
	nm_ctrl.topmargin = gRtnrNmCtrl[0].topmargin;
	nm_ctrl.bottommargin = gRtnrNmCtrl[0].bottommargin;
	nm_ctrl.leftmargin = gRtnrNmCtrl[0].leftmargin;
	nm_ctrl.rightmargin = gRtnrNmCtrl[0].rightmargin;

	nm_thd.vardiff_thd = gRtnrNmThd[value_tmp].vardiff_thd;
	nm_thd.varhigh_thd = gRtnrNmThd[value_tmp].varhigh_thd;
	nm_thd.varlow_thd = gRtnrNmThd[value_tmp].varlow_thd;
	nm_thd.meandiff_thd = gRtnrNmThd[value_tmp].meandiff_thd;
	nm_thd.noiselevel_thd0 = gRtnrNmThd[value_tmp].noiselevel_thd0;
	nm_thd.noiselevel_thd1 = gRtnrNmThd[value_tmp].noiselevel_thd1;
	nm_thd.noiselevel_thd2 = gRtnrNmThd[value_tmp].noiselevel_thd2;
	nm_thd.noiselevel_thd3 = gRtnrNmThd[value_tmp].noiselevel_thd3;
	nm_thd.noiselevel_thd4 = gRtnrNmThd[value_tmp].noiselevel_thd4;
	nm_thd.noiselevel_thd5 = gRtnrNmThd[value_tmp].noiselevel_thd5;
	nm_thd.noiselevel_thd6 = gRtnrNmThd[value_tmp].noiselevel_thd6;
	nm_thd.noiselevel_thd7 = gRtnrNmThd[value_tmp].noiselevel_thd7;
	nm_thd.noiselevel_thd8 = gRtnrNmThd[value_tmp].noiselevel_thd8;
	nm_thd.noiselevel_thd9 = gRtnrNmThd[value_tmp].noiselevel_thd9;
	nm_thd.noiselevel_thd10 = gRtnrNmThd[value_tmp].noiselevel_thd10;
	nm_thd.noiselevel_thd11 = gRtnrNmThd[value_tmp].noiselevel_thd11;
	nm_thd.noiselevel_thd12 = gRtnrNmThd[value_tmp].noiselevel_thd12;
	nm_thd.noiselevel_thd13 = gRtnrNmThd[value_tmp].noiselevel_thd13;
	nm_thd.noiselevel_thd14 = gRtnrNmThd[value_tmp].noiselevel_thd14;

//	drvif_color_DRV_NM_Init((DRV_rtnr_nm_ctrl_t *)&nm_ctrl, (DRV_rtnr_nm_thd_t *)&nm_thd);

}

void fwif_set_color_MADI_HMC(unsigned char src_idx, unsigned char level)
{
	extern DRV_di_ma_hmc_table gHMCTable[gHMCTable_MAX];
	DRV_di_ma_hmc_table hmcTableHere;

	if (level >= gHMCTable_MAX)
		level = 0;

	hmcTableHere = gHMCTable[level];
/*
	hmcTableHere.di_ma_hmc_main_control.hmc_en = gHMCTable[level].di_ma_hmc_main_control.hmc_en;
	hmcTableHere.di_ma_hmc_main_control.ma_horicomth = gHMCTable[level].di_ma_hmc_main_control.ma_horicomth;
	hmcTableHere.di_ma_hmc_main_control.hmc_follow_MA_en = gHMCTable[level].di_ma_hmc_main_control.hmc_follow_MA_en;
	hmcTableHere.di_ma_hmc_main_control.hmc_half_mv_blending_en = gHMCTable[level].di_ma_hmc_main_control.hmc_half_mv_blending_en;
	hmcTableHere.di_ma_hmc_main_control.hmc_odd_mv_no_comp_en = gHMCTable[level].di_ma_hmc_main_control.hmc_odd_mv_no_comp_en;
	hmcTableHere.di_ma_hmc_main_control.hmc_even_mv_no_teeth_en = gHMCTable[level].di_ma_hmc_main_control.hmc_even_mv_no_teeth_en;
	hmcTableHere.di_ma_hmc_main_control.hmc_window_en = gHMCTable[level].di_ma_hmc_main_control.hmc_window_en;
	hmcTableHere.di_ma_hmc_main_control.hmc_window_top = gHMCTable[level].di_ma_hmc_main_control.hmc_window_top;
	hmcTableHere.di_ma_hmc_main_control.hmc_window_bot = gHMCTable[level].di_ma_hmc_main_control.hmc_window_bot;
	hmcTableHere.di_ma_hmc_main_control.hmc_window_start_line = gHMCTable[level].di_ma_hmc_main_control.hmc_window_start_line;

	hmcTableHere.di_ma_hmc_mv_refine.mv_refine_en = gHMCTable[level].di_ma_hmc_mv_refine.mv_refine_en;
	hmcTableHere.di_ma_hmc_mv_refine.mv_refine_subPx_en = gHMCTable[level].di_ma_hmc_mv_refine.mv_refine_subPx_en;
	hmcTableHere.di_ma_hmc_mv_refine.mv_refine_Y_thd = gHMCTable[level].di_ma_hmc_mv_refine.mv_refine_Y_thd;
	hmcTableHere.di_ma_hmc_mv_refine.mv_refine_subPx_Y_thd = gHMCTable[level].di_ma_hmc_mv_refine.mv_refine_subPx_Y_thd;
	hmcTableHere.di_ma_hmc_mv_refine.mv_refine_curMotion_en = gHMCTable[level].di_ma_hmc_mv_refine.mv_refine_curMotion_en;
	hmcTableHere.di_ma_hmc_mv_refine.mv_refine_curMotion_thd = gHMCTable[level].di_ma_hmc_mv_refine.mv_refine_curMotion_thd;
	hmcTableHere.di_ma_hmc_mv_refine.mv_refine_sobel_en = gHMCTable[level].di_ma_hmc_mv_refine.mv_refine_sobel_en;
	hmcTableHere.di_ma_hmc_mv_refine.mv_refine_sobel_diff_thd = gHMCTable[level].di_ma_hmc_mv_refine.mv_refine_sobel_diff_thd;
	hmcTableHere.di_ma_hmc_mv_refine.mv_refine_sobel_nonEdge_thd = gHMCTable[level].di_ma_hmc_mv_refine.mv_refine_sobel_nonEdge_thd;

	hmcTableHere.di_ma_hmc_morphology.en = gHMCTable[level].di_ma_hmc_morphology.en;
	hmcTableHere.di_ma_hmc_morphology.hh_h_thd = gHMCTable[level].di_ma_hmc_morphology.hh_h_thd;
	hmcTableHere.di_ma_hmc_morphology.hh_v_thd = gHMCTable[level].di_ma_hmc_morphology.hh_v_thd;
	hmcTableHere.di_ma_hmc_morphology.noise_thd = gHMCTable[level].di_ma_hmc_morphology.noise_thd;
	hmcTableHere.di_ma_hmc_morphology.same_1_thd = gHMCTable[level].di_ma_hmc_morphology.same_1_thd;
	hmcTableHere.di_ma_hmc_morphology.same_2_thd = gHMCTable[level].di_ma_hmc_morphology.same_2_thd;
	hmcTableHere.di_ma_hmc_morphology.same_hh_1_thd = gHMCTable[level].di_ma_hmc_morphology.same_hh_1_thd;
	hmcTableHere.di_ma_hmc_morphology.zero_1_thd = gHMCTable[level].di_ma_hmc_morphology.zero_1_thd;
	hmcTableHere.di_ma_hmc_morphology.zero_2_thd = gHMCTable[level].di_ma_hmc_morphology.zero_2_thd;
	hmcTableHere.di_ma_hmc_morphology.zero_hh_1_thd = gHMCTable[level].di_ma_hmc_morphology.zero_hh_1_thd;

	hmcTableHere.di_ma_hmc_dynamic_control.clip_en = gHMCTable[level].di_ma_hmc_dynamic_control.clip_en;
	hmcTableHere.di_ma_hmc_dynamic_control.clip_max_thd = gHMCTable[level].di_ma_hmc_dynamic_control.clip_max_thd;
	hmcTableHere.di_ma_hmc_dynamic_control.clip_min_thd = gHMCTable[level].di_ma_hmc_dynamic_control.clip_min_thd;
	hmcTableHere.di_ma_hmc_dynamic_control.thd_bias = gHMCTable[level].di_ma_hmc_dynamic_control.thd_bias;
	hmcTableHere.di_ma_hmc_dynamic_control.thd_en = gHMCTable[level].di_ma_hmc_dynamic_control.thd_en;

	hmcTableHere.di_ma_hmc_mv_correct.correct_en	= gHMCTable[level].di_ma_hmc_mv_correct.correct_en;
	hmcTableHere.di_ma_hmc_mv_correct.energy_bias	= gHMCTable[level].di_ma_hmc_mv_correct.energy_bias;
	hmcTableHere.di_ma_hmc_mv_correct.pre_diff_th	= gHMCTable[level].di_ma_hmc_mv_correct.pre_diff_th;
	hmcTableHere.di_ma_hmc_mv_correct.nxt_diff_th 	= gHMCTable[level].di_ma_hmc_mv_correct.nxt_diff_th;
	hmcTableHere.di_ma_hmc_mv_correct.weight1		= gHMCTable[level].di_ma_hmc_mv_correct.weight1;
	hmcTableHere.di_ma_hmc_mv_correct.weight2 		= gHMCTable[level].di_ma_hmc_mv_correct.weight2;
	hmcTableHere.di_ma_hmc_mv_correct.weight3 		= gHMCTable[level].di_ma_hmc_mv_correct.weight3;
	hmcTableHere.di_ma_hmc_mv_correct.weight4 		= gHMCTable[level].di_ma_hmc_mv_correct.weight4;
	hmcTableHere.di_ma_hmc_mv_correct.energy_th1 	= gHMCTable[level].di_ma_hmc_mv_correct.energy_th1;
	hmcTableHere.di_ma_hmc_mv_correct.energy_th2 	= gHMCTable[level].di_ma_hmc_mv_correct.energy_th2;
	hmcTableHere.di_ma_hmc_mv_correct.energy_th3 	= gHMCTable[level].di_ma_hmc_mv_correct.energy_th3;

	hmcTableHere.di_ma_hmc_mv_energy_check.energy_check_en 	= gHMCTable[level].di_ma_hmc_mv_energy_check.energy_check_en;
	hmcTableHere.di_ma_hmc_mv_energy_check.check_pre_th 	= gHMCTable[level].di_ma_hmc_mv_energy_check.check_pre_th;
	hmcTableHere.di_ma_hmc_mv_energy_check.check_nxt_th 	= gHMCTable[level].di_ma_hmc_mv_energy_check.check_nxt_th;
	hmcTableHere.di_ma_hmc_mv_energy_check.check_pre_gain 	= gHMCTable[level].di_ma_hmc_mv_energy_check.check_pre_gain;
	hmcTableHere.di_ma_hmc_mv_energy_check.check_nxt_gain 	= gHMCTable[level].di_ma_hmc_mv_energy_check.check_nxt_gain;

	hmcTableHere.di_ma_hmc_mv_teeth_check.allTeethCheck_en 	= gHMCTable[level].di_ma_hmc_mv_teeth_check.allTeethCheck_en;
	hmcTableHere.di_ma_hmc_mv_teeth_check.all_teeth_th		= gHMCTable[level].di_ma_hmc_mv_teeth_check.all_teeth_th;
	hmcTableHere.di_ma_hmc_mv_teeth_check.notTeethRecover_en= gHMCTable[level].di_ma_hmc_mv_teeth_check.notTeethRecover_en;
	hmcTableHere.di_ma_hmc_mv_teeth_check.notTeeth_Hline_th	= gHMCTable[level].di_ma_hmc_mv_teeth_check.notTeeth_Hline_th;
	hmcTableHere.di_ma_hmc_mv_teeth_check.notTeeth_Dline_th = gHMCTable[level].di_ma_hmc_mv_teeth_check.notTeeth_Dline_th;
*/
	drvif_color_ma_HMC((DRV_di_ma_hmc_table *)(&hmcTableHere));
}

void fwif_set_color_MADI_HME(unsigned char src_idx, unsigned char level)
{
//#ifndef BUILD_QUICK_SHOW
	extern DRV_di_ma_hme_table gHMETable[gHMETable_MAX];
	DRV_di_ma_hme_table hmeTableHere;

	if (level >= gHMETable_MAX)
		level = 0;

	//hmeTableHere.di_ma_hme_prefer_even_mv.en = gHMETable[level].di_ma_hme_prefer_even_mv.en; //mac6p removed
	hmeTableHere.di_ma_hme_prefer_even_mv.ratio = gHMETable[level].di_ma_hme_prefer_even_mv.ratio;

	hmeTableHere.di_ma_hme_avoid_RL.en = gHMETable[level].di_ma_hme_avoid_RL.en;
	hmeTableHere.di_ma_hme_avoid_RL.Nedge = gHMETable[level].di_ma_hme_avoid_RL.Nedge;
	hmeTableHere.di_ma_hme_avoid_RL.RLedge = gHMETable[level].di_ma_hme_avoid_RL.RLedge;

	hmeTableHere.di_ma_hme_ref_y.en = gHMETable[level].di_ma_hme_ref_y.en;
	hmeTableHere.di_ma_hme_ref_y.thd0 = gHMETable[level].di_ma_hme_ref_y.thd0;
	hmeTableHere.di_ma_hme_ref_y.thd1 = gHMETable[level].di_ma_hme_ref_y.thd1;
	hmeTableHere.di_ma_hme_ref_y.thd2 = gHMETable[level].di_ma_hme_ref_y.thd2;
	hmeTableHere.di_ma_hme_ref_y.bias1 = gHMETable[level].di_ma_hme_ref_y.bias1;
	hmeTableHere.di_ma_hme_ref_y.bias2 = gHMETable[level].di_ma_hme_ref_y.bias2;
	hmeTableHere.di_ma_hme_ref_y.bias3 = gHMETable[level].di_ma_hme_ref_y.bias3;

	hmeTableHere.di_ma_hme_invalid_check.chck_1_en = gHMETable[level].di_ma_hme_invalid_check.chck_1_en;
	hmeTableHere.di_ma_hme_invalid_check.chck_2_en = gHMETable[level].di_ma_hme_invalid_check.chck_2_en;
	hmeTableHere.di_ma_hme_invalid_check.zeroMV_en = gHMETable[level].di_ma_hme_invalid_check.zeroMV_en;
	hmeTableHere.di_ma_hme_invalid_check.cnt = gHMETable[level].di_ma_hme_invalid_check.cnt;
	hmeTableHere.di_ma_hme_invalid_check.maxmin_thd = gHMETable[level].di_ma_hme_invalid_check.maxmin_thd;

	hmeTableHere.di_ma_hme_decision.min = gHMETable[level].di_ma_hme_decision.min;
	hmeTableHere.di_ma_hme_decision.x_conti_cnt = gHMETable[level].di_ma_hme_decision.x_conti_cnt;
	hmeTableHere.di_ma_hme_decision.line_ratio = gHMETable[level].di_ma_hme_decision.line_ratio;
	hmeTableHere.di_ma_hme_decision.max_abs = gHMETable[level].di_ma_hme_decision.max_abs;
	hmeTableHere.di_ma_hme_decision.sum_max_ratio = gHMETable[level].di_ma_hme_decision.sum_max_ratio;

	hmeTableHere.DI_HMC_ME_REFINE_CTRL_1 = gHMETable[level].DI_HMC_ME_REFINE_CTRL_1;

	drvif_color_ma_HME((DRV_di_ma_hme_table *)(&hmeTableHere));
//#endif	
}

void fwif_set_color_MADI_PAN(unsigned char src_idx, unsigned char level)
{
	extern DRV_di_ma_pan_table gPanTable[gPANTable_MAX];
	DRV_di_ma_pan_table panTableHere;

	if (level >= gPANTable_MAX)
		level = 0;

	panTableHere.di_ma_pan_control.pan_en = gPanTable[level].di_ma_pan_control.pan_en;
	panTableHere.di_ma_pan_control.force_comp_en = gPanTable[level].di_ma_pan_control.force_comp_en;
	panTableHere.di_ma_pan_control.pan_mc_rls_en = gPanTable[level].di_ma_pan_control.pan_mc_rls_en;
	panTableHere.di_ma_pan_control.sub_px_det_en = gPanTable[level].di_ma_pan_control.sub_px_det_en;
	panTableHere.di_ma_pan_control.pan_mc_adpative_en = gPanTable[level].di_ma_pan_control.pan_mc_adpative_en;
	panTableHere.di_ma_pan_control.pan_mc_teeth_check_en = gPanTable[level].di_ma_pan_control.pan_mc_teeth_check_en;
	panTableHere.di_ma_pan_control.pan_me_hist_sel = gPanTable[level].di_ma_pan_control.pan_me_hist_sel;
	panTableHere.di_ma_pan_control.pan_odd_mv_en = gPanTable[level].di_ma_pan_control.pan_odd_mv_en;
	panTableHere.di_ma_pan_control.sobel_diff_thd = gPanTable[level].di_ma_pan_control.sobel_diff_thd;
	panTableHere.di_ma_pan_control.sobel_diff_thd_rls = gPanTable[level].di_ma_pan_control.sobel_diff_thd_rls;
	panTableHere.di_ma_pan_control.teeth_check_thd = gPanTable[level].di_ma_pan_control.teeth_check_thd;

	panTableHere.di_ma_pan_edge_type.Hedge_en = gPanTable[level].di_ma_pan_edge_type.Hedge_en;
	panTableHere.di_ma_pan_edge_type.Ledge_en = gPanTable[level].di_ma_pan_edge_type.Ledge_en;
	panTableHere.di_ma_pan_edge_type.Nedge_en = gPanTable[level].di_ma_pan_edge_type.Nedge_en;
	panTableHere.di_ma_pan_edge_type.Redge_en = gPanTable[level].di_ma_pan_edge_type.Redge_en;
	panTableHere.di_ma_pan_edge_type.Vedge_en = gPanTable[level].di_ma_pan_edge_type.Vedge_en;

	panTableHere.di_ma_pan_detection.diffMaxMin = gPanTable[level].di_ma_pan_detection.diffMaxMin;
	panTableHere.di_ma_pan_detection.diffMinThd = gPanTable[level].di_ma_pan_detection.diffMinThd;
	panTableHere.di_ma_pan_detection.restIdx = gPanTable[level].di_ma_pan_detection.restIdx;
	panTableHere.di_ma_pan_detection.restIdxThd = gPanTable[level].di_ma_pan_detection.restIdxThd;
	panTableHere.di_ma_pan_detection.panConti_str = gPanTable[level].di_ma_pan_detection.panConti_str;
	panTableHere.di_ma_pan_detection.panConti = gPanTable[level].di_ma_pan_detection.panConti;
	panTableHere.di_ma_pan_detection.panDeb_str = gPanTable[level].di_ma_pan_detection.panDeb_str;
	panTableHere.di_ma_pan_detection.panDeb = gPanTable[level].di_ma_pan_detection.panDeb;
	drvif_color_ma_PAN((DRV_di_ma_pan_table *)(&panTableHere));
}

void fwif_color_ma_set_DI_MA_Adjust_Table(unsigned char src_idx, unsigned char level)
{
	extern DRV_di_ma_adjust_table DI_MA_Adjust_Table[DI_MA_Adjust_Table_MAX];
	DRV_di_ma_adjust_table data;

	/*Table Protection*/
	if (level >=  DI_MA_Adjust_Table_MAX)
		level = 0;

	/*Set DI MA Control Parameters*/
	data.di_ma_control.ma_controlmode = DI_MA_Adjust_Table[level].di_ma_control.ma_controlmode;
	data.di_ma_control.ma_stilllogoena = DI_MA_Adjust_Table[level].di_ma_control.ma_stilllogoena;
	data.di_ma_control.ma_messintra90 = DI_MA_Adjust_Table[level].di_ma_control.ma_messintra90;
	data.di_ma_control.ma_smoothintra90 = DI_MA_Adjust_Table[level].di_ma_control.ma_smoothintra90;

	/*Set Frame Motion*/
	data.di_ma_frame_motion_th.ma_framehtha = DI_MA_Adjust_Table[level].di_ma_frame_motion_th.ma_framehtha;
	data.di_ma_frame_motion_th.ma_framemtha = DI_MA_Adjust_Table[level].di_ma_frame_motion_th.ma_framemtha;
	data.di_ma_frame_motion_th.ma_framerlvtha = DI_MA_Adjust_Table[level].di_ma_frame_motion_th.ma_framerlvtha;
	data.di_ma_frame_motion_th.ma_framestha = DI_MA_Adjust_Table[level].di_ma_frame_motion_th.ma_framestha;

	data.di_ma_frame_motion_th.ma_framehthb = DI_MA_Adjust_Table[level].di_ma_frame_motion_th.ma_framehthb;
	data.di_ma_frame_motion_th.ma_framemthb = DI_MA_Adjust_Table[level].di_ma_frame_motion_th.ma_framemthb;
	data.di_ma_frame_motion_th.ma_framerlvthb = DI_MA_Adjust_Table[level].di_ma_frame_motion_th.ma_framerlvthb;
	data.di_ma_frame_motion_th.ma_framesthb = DI_MA_Adjust_Table[level].di_ma_frame_motion_th.ma_framesthb;

	data.di_ma_frame_motion_th.ma_framehthc = DI_MA_Adjust_Table[level].di_ma_frame_motion_th.ma_framehthc;
	data.di_ma_frame_motion_th.ma_framemthc = DI_MA_Adjust_Table[level].di_ma_frame_motion_th.ma_framemthc;
	data.di_ma_frame_motion_th.ma_framerlvthc = DI_MA_Adjust_Table[level].di_ma_frame_motion_th.ma_framerlvthc;
	data.di_ma_frame_motion_th.ma_framesthc = DI_MA_Adjust_Table[level].di_ma_frame_motion_th.ma_framesthc;

	/*Set Field teeth TH*/
	data.di_ma_field_teeth_th.ma_fieldteethhtha = DI_MA_Adjust_Table[level].di_ma_field_teeth_th.ma_fieldteethhtha;
	data.di_ma_field_teeth_th.ma_fieldteethmtha = DI_MA_Adjust_Table[level].di_ma_field_teeth_th.ma_fieldteethmtha;
	data.di_ma_field_teeth_th.ma_fieldteethrlvtha = DI_MA_Adjust_Table[level].di_ma_field_teeth_th.ma_fieldteethrlvtha;
	data.di_ma_field_teeth_th.ma_fieldteethstha = DI_MA_Adjust_Table[level].di_ma_field_teeth_th.ma_fieldteethstha;

	drvif_color_ma_set_DI_MA_Adjust_Table((DRV_di_ma_adjust_table *)(&data));

}
void fwif_color_ma_set_DI_MA_Init_Table(void)
{
	extern DRV_di_ma_init_table DI_MA_Init_Table;
	DRV_di_ma_init_table data;
	/*Set Parameters*/

	data.di_ma_control_init.ma_onedirectionweaveen = DI_MA_Init_Table.di_ma_control_init.ma_onedirectionweaveen;
	//data.di_ma_control_init.ma_preferintra = DI_MA_Init_Table.di_ma_control_init.ma_preferintra; //mac6p removed

	data.di_ma_others_th.ma_fieldteeththb = DI_MA_Init_Table.di_ma_others_th.ma_fieldteeththb;
	data.di_ma_others_th.ma_stilllogovdth = DI_MA_Init_Table.di_ma_others_th.ma_stilllogovdth;
	data.di_ma_others_th.ma_ydiffth = DI_MA_Init_Table.di_ma_others_th.ma_ydiffth;

	data.di_ma_sawtooth_th.ma_sawtoothhth = DI_MA_Init_Table.di_ma_sawtooth_th.ma_sawtoothhth;
	data.di_ma_sawtooth_th.ma_sawtoothmth = DI_MA_Init_Table.di_ma_sawtooth_th.ma_sawtoothmth;
	data.di_ma_sawtooth_th.ma_sawtoothrlvth = DI_MA_Init_Table.di_ma_sawtooth_th.ma_sawtoothrlvth;
	data.di_ma_sawtooth_th.ma_sawtoothsth = DI_MA_Init_Table.di_ma_sawtooth_th.ma_sawtoothsth;

	drvif_color_ma_set_DI_MA_Init_Table((DRV_di_ma_init_table *)(&data));
}





void fwif_color_set_SCurve_driver(unsigned char item_idx, unsigned char value)
{
	unsigned char src_idx = 0;
	unsigned char display = 0;
	StructColorDataType *pTable = NULL;
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;

	if ((VIP_system_info_structure_table->input_display) == SLR_MAIN_DISPLAY) {
		switch (item_idx) {
		case gamma_mode:
				pTable->Gamma = value;
			break;
		case S_gamma_index:
				pTable->Sindex = value;
			break;
		case S_gamma_high:
				pTable->Shigh = value;
			break;
		case S_gamma_low:
				pTable->Slow = value;
			break;
		}

		fwif_set_gamma(pTable->Gamma, pTable->Sindex, pTable->Shigh, pTable->Slow);
	}
}



void fwif_module_set_film_mode(unsigned char src_idx, unsigned char value)
{
	UINT8 Timing;
	DRV_film_mode data = {0};
	extern DRV_film_mode film_mode;

	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if (film_mode.film_status == 0)
		value = 0;
	Timing = VIP_system_info_structure_table->Timing;
	switch (value) {
	case 0:
			data.film_status = 0; /*disable film detection*/
			break;
	default:
	case 1:
			if (Timing == _MODE_1080I30 || Timing == _MODE_480I)
				data.film_status = 28; /*enable film detection / 3:2 / 2:2*/
			else if (Timing == _MODE_1080I25 || Timing == _MODE_576I)
				data.film_status = 20; /*enable film detection / 2:2*/
			break;
	case 2:
			if (Timing == _MODE_1080I30 || Timing == _MODE_480I)
				data.film_status = 31; /*enable film detection / 3:2 / 2:2 / 3:2 sub / 2:2 sub*/
			else if (Timing == _MODE_1080I25 || Timing == _MODE_576I)
				data.film_status = 21; /*enable film detection / 2:2 / 2:2 sub*/
			break;
	}
	down(get_DI_semaphore());
	drvif_module_film_mode((DRV_film_mode *) &data);
	up(get_DI_semaphore());
	if (g_Share_Memory_FILM_FW != NULL)
		g_Share_Memory_FILM_FW->film_en_ap_status = data.film_status;

}

void fwif_module_set_film_setting(void)
{
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	_RPC_system_setting_info *RPC_VIP_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	extern DRV_film_table_t gFilmInitTable[5];
	DRV_film_table_t data;

	unsigned char Table_Idx = 0;
	unsigned char Timing = 0;
	Timing = VIP_system_info_structure_table->Timing;

	switch (Timing) {
	case _MODE_480I: /*60Hz SD*/
			Table_Idx = 0;
			break;
	case _MODE_1080I30: /*60Hz HD*/
			Table_Idx = 1;
			break;
	case _MODE_576I: /*50Hz SD*/
			Table_Idx = 2;
			break;
	case _MODE_1080I25: /*50Hz HD*/
			Table_Idx = 3;
			break;
	default:
			Table_Idx = 0;
	}

	if (RPC_VIP_system_info_structure_table->VIP_source == VIP_QUALITY_DTV_576I)
		Table_Idx = 4; 	/* for WOSQRTK-3233 */

	data.film_sawtooth_choose = 		gFilmInitTable[Table_Idx].film_sawtooth_choose;
	data.film_LeaveQuick_en = 			gFilmInitTable[Table_Idx].film_LeaveQuick_en;
	data.film_VerScrollingText_en = 	gFilmInitTable[Table_Idx].film_VerScrollingText_en;
	data.film_thumbnail_en = 			gFilmInitTable[Table_Idx].film_thumbnail_en;
	data.film_pair1_en = 				gFilmInitTable[Table_Idx].film_pair1_en;
	data.film_pair2_en = 				gFilmInitTable[Table_Idx].film_pair2_en;
	data.film_pair_en_auto = 			gFilmInitTable[Table_Idx].film_pair_en_auto;
	data.film_violate_strict_en = 		gFilmInitTable[Table_Idx].film_violate_strict_en;
	data.film_SawtoothThl = 			gFilmInitTable[Table_Idx].film_SawtoothThl;
	data.film_film22_SawtoothTh1 = 		gFilmInitTable[Table_Idx].film_film22_SawtoothTh1;
	data.film_FrMotionThl = 			gFilmInitTable[Table_Idx].film_FrMotionThl;

	data.film_FieldMotionSumThl = 		gFilmInitTable[Table_Idx].film_FieldMotionSumThl;
	data.film_film22_FiStaticSThl = 	gFilmInitTable[Table_Idx].film_film22_FiStaticSThl;
	data.film_FrStaticSThl = 			gFilmInitTable[Table_Idx].film_FrStaticSThl;
	data.film_FiStaticSThl = 			gFilmInitTable[Table_Idx].film_FiStaticSThl;

	data.film_PairRatio_all1 = 			gFilmInitTable[Table_Idx].film_PairRatio_all1;
	data.film_PairRatio_all2 = 			gFilmInitTable[Table_Idx].film_PairRatio_all2;
	data.film_FiMinSThl = 				gFilmInitTable[Table_Idx].film_FiMinSThl;
	data.film_FiMaxSThl = 				gFilmInitTable[Table_Idx].film_FiMaxSThl;

	data.film_SubFieldMotionSumThl = 	gFilmInitTable[Table_Idx].film_SubFieldMotionSumThl;
	data.film_SubFrStaticsSTh1 = 		gFilmInitTable[Table_Idx].film_SubFrStaticsSTh1;
	data.film_SubFiStaticsSTh1 = 		gFilmInitTable[Table_Idx].film_SubFiStaticsSTh1;

	data.film_PairRatio_sub1 = 			gFilmInitTable[Table_Idx].film_PairRatio_sub1;
	data.film_PairRatio_sub2 = 			gFilmInitTable[Table_Idx].film_PairRatio_sub2;
	data.film_SubFiMinSTh1 = 			gFilmInitTable[Table_Idx].film_SubFiMinSTh1;
	data.film_SubFiMaxSTh1 = 			gFilmInitTable[Table_Idx].film_SubFiMaxSTh1;

	data.film_film32_debounce_fieldnum = 	gFilmInitTable[Table_Idx].film_film32_debounce_fieldnum;
	data.film_film22_debounce_fieldnum = 	gFilmInitTable[Table_Idx].film_film22_debounce_fieldnum;
	data.film_film22_SubFieldMotionSumThl = gFilmInitTable[Table_Idx].film_film22_SubFieldMotionSumThl;
	data.film_film22_FieldMotionSumThl = 	gFilmInitTable[Table_Idx].film_film22_FieldMotionSumThl;

	data.film_fw_en  = 					gFilmInitTable[Table_Idx].film_fw_en;

	drvif_module_film_setting((DRV_film_table_t *)&data);
}

void fwif_module_set_film_table_t(unsigned char Table_Idx)
{
	extern DRV_film_table_t gFilmInitTable[5];
	down(get_DI_semaphore());
	drvif_module_film_setting((DRV_film_table_t *)&gFilmInitTable[Table_Idx]);
	up(get_DI_semaphore());
}

void fwif_color_set_MA_Chroma_Error(unsigned char src_idx, unsigned char value)
{
	DRV_MA_ChromaError data = {0};
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	extern VIP_MA_ChromaError MA_Chroma_Error_Table[t_MA_Chroma_Error_MAX];
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	if (value >= t_MA_Chroma_Error_MAX)
		drvif_color_ma_ChromaError((DRV_MA_ChromaError *) &data);
	else {
		data.chromaerror_all = MA_Chroma_Error_Table[value].chromaerror_all;
		data.chromaerror_en = MA_Chroma_Error_Table[value].chromaerror_en;
		data.chromaerror_framemotionc_th = MA_Chroma_Error_Table[value].chromaerror_framemotionc_th;
		data.chromaerror_th = MA_Chroma_Error_Table[value].chromaerror_th;

		data.di444to422Lowpass = MA_Chroma_Error_Table[value].di444to422Lowpass;

		drvif_color_ma_ChromaError((DRV_MA_ChromaError *)&data);
	}
}

unsigned char fwif_color_get_MA_Chroma_Error_en_from_vipTable(unsigned char src_idx, unsigned char value)
{
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	extern VIP_MA_ChromaError MA_Chroma_Error_Table[t_MA_Chroma_Error_MAX];
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}

	return MA_Chroma_Error_Table[value].chromaerror_en;
}

void fwif_color_set_dlti_driver(unsigned char src_idx, unsigned char value)
{
	/*extern DRV_Shp_Dlti gVipShp_Dlti[];
	DRV_Shp_Dlti Dlti_disabled = {0};*/
	extern DRV_D_DLTI_Table D_DLTI_Table[D_DLTI_table_num] ;
	DRV_D_DLTI_Table Dlti_disabled = {0};

	if (value >= D_DLTI_table_num) /* 255 = bypass*/
		/*drvif_color_shp_dlti(&Dlti_disabled);*/
		drvif_color_Set_D_DLTI_Table(&Dlti_disabled);
	else
		/*drvif_color_shp_dlti(&gVipShp_Dlti[value]);*/
		drvif_color_Set_D_DLTI_Table(&D_DLTI_Table[value]);
}

void fwif_color_set_dcti(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	DRV_VipNewDDcti_Table DCTI_disabled = {{0}};
	if (value >= DCTI_TABLE_LEVEL_MAX)
		value = DCTI_TABLE_LEVEL_MAX - 1;

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	VIP_system_info_structure_table->PQ_Setting_Info.I_D_Dcti_INFO.dDcti_table_select = value;

	if (value >= DCTI_TABLE_LEVEL_MAX)	/* 255 = bypass*/
		drvif_color_dnewdcti_table(&DCTI_disabled);
	else
		drvif_color_dnewdcti_table((DRV_VipNewDDcti_Table *)&gVipDNewDcti_Table[value]);
}
void fwif_color_set_Intra_Ver2_driver(unsigned char src_idx, unsigned char value)
{
	extern DRV_Intra_Ver2_Table gVipIntra_Set_Ver2_tab[];

	drvif_color_Intra_Ver2_table(&gVipIntra_Set_Ver2_tab[value]);
}

void fwif_color_set_scaleup_hcoef(unsigned char src_idx, unsigned char display, unsigned char TableIdx)
{
	extern DRV_FIR_Coef FIR_Coef_Ctrl[DRV_FIR_Coef_4Tap_NUM];
	DRV_FIR_Coef *pData = NULL;

	if (TableIdx < DRV_FIR_Coef_4Tap_NUM)
		pData = &FIR_Coef_Ctrl[TableIdx];
	else
		pData = &FIR_Coef_Ctrl[0];

#ifdef RUN_ON_TVBOX
	if ((scalerdisplay_get_display_tve_mode() == _MODE_480P) &&
		(scalerdisplay_get_BoxDispInfo(_BOX_DISP_WID) >=  704) &&
		(scalerdisplay_get_BoxDispInfo(_BOX_DISP_WID) < 720))
		pData = &FIR_Coef_Ctrl[20];
#endif

	drvif_color_scaler_set_scaleup_hcoef(display, pData, TableIdx, 0);
}

void fwif_color_set_scaleup_vcoef(unsigned char src_idx, unsigned char display, unsigned char TableIdx)
{
	extern DRV_FIR_Coef FIR_Coef_Ctrl[DRV_FIR_Coef_4Tap_NUM];
	DRV_FIR_Coef *pData = NULL;

	if (TableIdx < DRV_FIR_Coef_4Tap_NUM)
		pData = &FIR_Coef_Ctrl[TableIdx];
	else
		pData = &FIR_Coef_Ctrl[0];

#ifdef RUN_ON_TVBOX
	if ((scalerdisplay_get_display_tve_mode() == _MODE_480P) &&
		(scalerdisplay_get_BoxDispInfo(_BOX_DISP_WID) >=  704) &&
		(scalerdisplay_get_BoxDispInfo(_BOX_DISP_WID) < 720))
		pData = &FIR_Coef_Ctrl[20];
#endif

	drvif_color_scaler_set_scaleup_vcoef(display, pData, TableIdx, 0);
}

void fwif_color_set_scaleup_hv4tap_coef(unsigned char src_idx, unsigned char display, char CoefIdx_HY, char CoefIdx_VY, char CoefIdx_HC, char CoefIdx_VC)
{
	extern DRV_FIR_Coef FIR_Coef_Ctrl[DRV_FIR_Coef_4Tap_NUM];
	//DRV_FIR_Coef data;
	int CoefSel_in[4], CoefSel_out[4], TabSel_out[4];
	int i;

	#ifdef RUN_ON_TVBOX
		if ((scalerdisplay_get_display_tve_mode() == _MODE_480P) &&
			(scalerdisplay_get_BoxDispInfo(_BOX_DISP_WID) >=  704) &&(scalerdisplay_get_BoxDispInfo(_BOX_DISP_WID) < 720))
		{
			CoefIdx_HY = 20;
			CoefIdx_VY = 20;
		}
	#endif

	CoefSel_in[0] = CoefIdx_HY;
	CoefSel_in[1] = CoefIdx_VY;
	CoefSel_in[2] = CoefIdx_HC;
	CoefSel_in[3] = CoefIdx_VC;

	drvif_color_scaler_scaleup_table_arrange(CoefSel_in, CoefSel_out, TabSel_out);

	for (i = 0; i < 4; i++) {
		if (CoefSel_out[i] < 0 || CoefSel_out[i] >= DRV_FIR_Coef_4Tap_NUM)
			CoefSel_out[i] = 0;
	}

	/*H_Y*/
	//memcpy(&data, &(FIR_Coef_Ctrl[CoefSel_out[0]]), sizeof(DRV_FIR_Coef));
	drvif_color_scaler_set_scaleup_hcoef(display, &(FIR_Coef_Ctrl[CoefSel_out[0]]), CoefSel_out[0], TabSel_out[0]);
	/*V_Y*/
	//memcpy(&data, &(FIR_Coef_Ctrl[CoefSel_out[1]]), sizeof(DRV_FIR_Coef));
	drvif_color_scaler_set_scaleup_vcoef(display, &(FIR_Coef_Ctrl[CoefSel_out[1]]), CoefSel_out[1], TabSel_out[1]);
	/*H_C*/
	//memcpy(&data, &(FIR_Coef_Ctrl[CoefSel_out[2]]), sizeof(DRV_FIR_Coef));
	drvif_color_scaler_set_scaleup_coefHC(display, &(FIR_Coef_Ctrl[CoefSel_out[2]]), CoefSel_out[2], TabSel_out[2]);
	/*V_C*/
	//memcpy(&data, &(FIR_Coef_Ctrl[CoefSel_out[3]]), sizeof(DRV_FIR_Coef));
	drvif_color_scaler_set_scaleup_coefVC(display, &(FIR_Coef_Ctrl[CoefSel_out[3]]), CoefSel_out[3], TabSel_out[3]);
}


void fwif_color_set_scaleup_dircoef(unsigned char src_idx, unsigned char display, unsigned char TableIdx)
{
	extern DRV_FIR_Coef FIR_Coef_Ctrl[DRV_FIR_Coef_4Tap_NUM];

	if (TableIdx >= DRV_FIR_Coef_4Tap_NUM)
		TableIdx = 0;

	drvif_color_scaler_set_scaleup_dircoef(display, &FIR_Coef_Ctrl[TableIdx]);
}

void fwif_color_set_scaleuph_12tap_driver(unsigned char src_idx, unsigned char display, unsigned char TableIdx)
{
	extern DRV_SU_table_H12tap_t SU_table_h12tap[DRV_SU_table_H12tap_t_NUM];

	VIPprintf("fwif_color_set_scaleuph_8tap_driver, value = %d\n", TableIdx);

	if(TableIdx >= DRV_SU_table_H12tap_t_NUM) {
		TableIdx = 37;
	}

	drvif_color_scaler_setscaleup_hcoef_12tap(display, &(SU_table_h12tap[TableIdx]));
}


void fwif_color_set_scaleupv_8tap_driver(unsigned char src_idx, unsigned char display, unsigned char TableIdx)
{
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	_RPC_system_setting_info *RPC_VIP_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	extern DRV_SU_table_V8tap_t SU_table_v8tap[16];
	unsigned char tap_num_def[4] = {4, 8, 8, 2}; //mac6
	DRV_SU_table_V8tap_t data;

	VIPprintf("fwif_color_set_scaleupv_8tap_driver, value = %d\n", TableIdx);

	if (TableIdx >= 16) TableIdx = 12;		//20150804 roger protect the index overflow

	data.ver_mode_sel = SU_table_v8tap[TableIdx].ver_mode_sel;

	if (data.ver_mode_sel > 3)
		data.ver_mode_sel = 3;

	if (drvif_color_ultrazoom_scalerup_PR_mode(VIP_system_info_structure_table->Mem_Width) == NOT_PR_MODE) {
		if (VIP_system_info_structure_table->Mem_Width <= 960) {
			// can use 8tap, no correction required
		} else if (VIP_system_info_structure_table->Mem_Width <= 1920) {
			if (tap_num_def[data.ver_mode_sel] > 4)
				data.ver_mode_sel = 0;
		} else {
			data.ver_mode_sel = 3;
		}
	} else {
		if (VIP_system_info_structure_table->Mem_Width <= 960) {
			if (tap_num_def[data.ver_mode_sel] > 4)
				data.ver_mode_sel = 0;
		} else {
			data.ver_mode_sel = 3;
		}
	}

	RPC_VIP_system_info_structure_table->ScalingUp_Ctrl.UZU_Vertical_Mode = data.ver_mode_sel;

	data.coef_table = SU_table_v8tap[TableIdx].coef_table;
	data.tap_num = SU_table_v8tap[TableIdx].tap_num;

	/*drvif_color_scaler_setscaleup_vcoef_8tap(display, &(SU_table_v8tap[TableIdx]));*/
	drvif_color_scaler_setscaleup_vcoef_8tap(display, &data);
}
#ifdef CONFIG_HW_SUPPORT_TWOSTEP_SCALEUP
void fwif_color_set_twostep_scaleup_coef(unsigned char src_idx, unsigned char display, unsigned char TableIdx)
{
	// 2016.7/4 add for Merlin2 by jyyang
	// ratio_sel = 0:2X, 1:1.5X
	// when source is 1920x1080_120Hz, 2step_mode = 1, ratio_sel = 0.
	// when source is 2560x1440_120Hz, 2step_mode = 1, ratio_sel = 1.
	// otherwise, 2step_mode = 0, ratio_sel = 0.

	extern DRV_FIR_Coef FIR_Coef_Ctrl[DRV_FIR_Coef_4Tap_NUM];
	extern DRV_SU_table_H12tap_t SU_table_h12tap[DRV_SU_table_H12tap_t_NUM];
	extern DRV_TwoStepSU_Table TwoStepSU_Table[TwoStepSU_Table_NUM];
	int TableIdxHY, TableIdxVY, TableIdxHC, TableIdxVC, TableIdxHY_12tap;

	if(TableIdx >= TwoStepSU_Table_NUM) TableIdx = 0;

	TableIdxHY = TwoStepSU_Table[TableIdx].TwoStepSU_FIR_Sel.Tab4t_HY;
	TableIdxVY = TwoStepSU_Table[TableIdx].TwoStepSU_FIR_Sel.Tab4t_VY;
	TableIdxHC = TwoStepSU_Table[TableIdx].TwoStepSU_FIR_Sel.Tab4t_HC;
	TableIdxVC = TwoStepSU_Table[TableIdx].TwoStepSU_FIR_Sel.Tab4t_VC;
	TableIdxHY_12tap= TwoStepSU_Table[TableIdx].TwoStepSU_FIR_Sel.Tab12t_HY;

	if(TableIdxHY >= DRV_FIR_Coef_4Tap_NUM) TableIdxHY = 0;
	if(TableIdxVY >= DRV_FIR_Coef_4Tap_NUM) TableIdxVY = 0;
	if(TableIdxHC >= DRV_FIR_Coef_4Tap_NUM) TableIdxHC = 0;
	if(TableIdxVC >= DRV_FIR_Coef_4Tap_NUM) TableIdxVC = 0;
	if(TableIdxHY_12tap >= DRV_SU_table_H12tap_t_NUM) TableIdxHY_12tap = 0;

	drvif_color_twostep_scaleup_coef_4tap(&(FIR_Coef_Ctrl[TableIdxHY]), &(FIR_Coef_Ctrl[TableIdxVY]), &(FIR_Coef_Ctrl[TableIdxHC]), &(FIR_Coef_Ctrl[TableIdxVC]));
	drvif_color_twostep_scaleup_hcoef_12tap(&(SU_table_h12tap[TableIdxHY_12tap]));
	drvif_color_twostep_scaleup_mflti(&(TwoStepSU_Table[TableIdx].TwoStepSU_Mflti));
	drvif_color_twostep_scaleup_dering(&(TwoStepSU_Table[TableIdx].TwoStepSU_Dering));
}
void fwif_color_set_twostep_scaleup_coefH12(unsigned char src_idx, unsigned char display, unsigned char TableIdx)
{
	extern DRV_SU_table_H12tap_t SU_table_h12tap[DRV_SU_table_H12tap_t_NUM];

	if(TableIdx >= DRV_SU_table_H12tap_t_NUM) {
		TableIdx = 12;
	}

	drvif_color_twostep_scaleup_hcoef_12tap(&(SU_table_h12tap[TableIdx]));
}
#endif //CONFIG_HW_SUPPORT_TWOSTEP_SCALEUP
void fwif_color_set_scaleup_dir_weighting_driver(unsigned char src_idx, unsigned char value)
{
	_RPC_system_setting_info *RPC_VIP_system_info_structure_table= NULL;

	if (value >=  MAX_DIRECTIONAL_WEIGHTING_LEVEL)
		value = 0;

	RPC_VIP_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if (RPC_VIP_system_info_structure_table == NULL) {
		rtd_pr_vpq_emerg("fwif_color_set_scaleup_dir_weighting_driver, system info = NULL\n");
	} else {
		RPC_VIP_system_info_structure_table->ScalingUp_Ctrl.UZU_Dir_Phase_En = gVipDirsu_Table[value].Dirsu_Ctrl.DS_phase_En;
		/*rtd_pr_vpq_emerg("fwif_color_set_scaleup_dir_weighting_driver=%d\n", system_setting_info->PQ_Setting_Info.ScalingUp_Ctrl.UZU_Dir_Phase_En);*/
	}

	drvif_color_scaler_setscaleup_directionalscaler(&gVipDirsu_Table[value]);
}

void fwif_color_set_scaledownh_table(unsigned char src_idx, unsigned char value)
{
	unsigned char *index_table = NULL;
	_RPC_system_setting_info *RPC_system_setting_info = NULL;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	index_table = &(gVip_Table->SD_H_table[value][0]);

	RPC_system_setting_info = (_RPC_system_setting_info *) Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if (RPC_system_setting_info == NULL) {
		VIPprintf("~get system_setting_info return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	/*extern DRV_ScaleDown_COEF_TAB ScaleDown_COEF_TAB;*/

	fwif_color_scaler_setscaledown_hcoef_table(_MAIN_DISPLAY, index_table, &ScaleDown_COEF_TAB, RPC_system_setting_info);
}

void fwif_color_set_scaledownv_table(unsigned char src_idx, unsigned char value)
{
	unsigned char *index_table = NULL;
	_RPC_system_setting_info *RPC_system_setting_info = NULL;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	index_table = &(gVip_Table->SD_V_table[value][0]);

	RPC_system_setting_info = (_RPC_system_setting_info *) Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if (RPC_system_setting_info == NULL) {
		VIPprintf("~get system_setting_info return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	/*extern DRV_ScaleDown_COEF_TAB ScaleDown_COEF_TAB;*/

	fwif_color_scaler_setscaledown_vcoef_table(_MAIN_DISPLAY, index_table, &ScaleDown_COEF_TAB, RPC_system_setting_info);

}

unsigned char fwif_color_get_scaledownh(unsigned char src_idx)
{
	return fwif_color_scaler_getscaledown_hcoef(_MAIN_DISPLAY);
}

void fwif_color_set_scaledownh(unsigned char src_idx, unsigned char value)
{
	short *coef_table = &(ScaleDown_COEF_TAB.FIR_Coef_Table[value][0]);
	drvif_color_scaler_setscaledown_hcoef(_MAIN_DISPLAY, coef_table);
}

unsigned char fwif_color_get_scaledownv(unsigned char src_idx)
{
	return fwif_color_scaler_getscaledown_vcoef(_MAIN_DISPLAY);
}

void fwif_color_set_scaledownv(unsigned char src_idx, unsigned char value)
{
	short *coef_table = &(ScaleDown_COEF_TAB.FIR_Coef_Table[value][0]);
	drvif_color_scaler_setscaledown_vcoef(_MAIN_DISPLAY, coef_table);
}

unsigned char fwif_color_get_scaledown444To422(unsigned char src_idx)
{
	return fwif_color_scaler_getscaledown_444_to_422_coef(_MAIN_DISPLAY);
}

void fwif_color_set_scaledown444To422(unsigned char src_idx, unsigned char value)
{
	_RPC_system_setting_info *RPC_system_setting_info = NULL;
	RPC_system_setting_info = (_RPC_system_setting_info *) Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if (RPC_system_setting_info == NULL) {
		VIPprintf("~get system_setting_info return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	RPC_system_setting_info->ScalingDown_Setting_INFO.SDFH444To422Sel = value;

	fwif_color_scaler_setscaledown_444_to_422_coef(_MAIN_DISPLAY, value, &ScaleDown_COEF_TAB);
}

void fwif_color_set_scalepk_driver(unsigned char src_idx, unsigned char value)
{
	extern VIP_SU_PK_Coeff SU_PK_Coeff[SU_PK_COEF_MAX];
	DRV_SU_PK_Coeff data = {0};
	signed int Coring, X1;
	signed int ndG1, ndOffset;

	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

#if 0

	if ((Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY) == SLR_SUB_DISPLAY) && ((Scaler_PipGetInfo(SLR_PIP_TYPE) == SUBTYPE_PIPSMALL) || (Scaler_PipGetInfo(SLR_PIP_TYPE) == SUBTYPE_PIPLARGE) || (Scaler_PipGetInfo(SLR_PIP_TYPE) == SUBTYPE_PIPMEDIUM)))
		drvif_color_scaler_setds_pk(&gVip_Table->SU_PK_Coeff[0]);
	else
		drvif_color_scaler_setds_pk(&gVip_Table->SU_PK_Coeff[value]);

#else
	if (value >= SU_PK_COEF_MAX)
		drvif_color_scaler_setds_pk((DRV_SU_PK_Coeff *)(&data));
	else {
		if (((VIP_system_info_structure_table->input_display) == SLR_SUB_DISPLAY) && ((Scaler_PipGetInfo(SLR_PIP_TYPE) == SUBTYPE_PIPSMALL) || (Scaler_PipGetInfo(SLR_PIP_TYPE) == SUBTYPE_PIPLARGE) || (Scaler_PipGetInfo(SLR_PIP_TYPE) == SUBTYPE_PIPMEDIUM))) {
			data.Pk_En = SU_PK_Coeff[0].Pk_En;
			data.Pk_X1 = SU_PK_Coeff[0].Pk_X1;
			data.Pk_Coring = SU_PK_Coeff[0].Pk_Coring;
			data.Pk_NdG1 = SU_PK_Coeff[0].Pk_NdG1;
			data.Pk_NdG2 = SU_PK_Coeff[0].Pk_NdG2;
			data.Pk_NdLmtN = SU_PK_Coeff[0].Pk_NdLmtN;
			data.Pk_NdLmtP = SU_PK_Coeff[0].Pk_NdLmtP;
		} else {
			data.Pk_En = SU_PK_Coeff[value].Pk_En;
			data.Pk_X1 = SU_PK_Coeff[value].Pk_X1;
			data.Pk_Coring = SU_PK_Coeff[value].Pk_Coring;
			data.Pk_NdG1 = SU_PK_Coeff[value].Pk_NdG1;
			data.Pk_NdG2 = SU_PK_Coeff[value].Pk_NdG2;
			data.Pk_NdLmtN = SU_PK_Coeff[value].Pk_NdLmtN;
			data.Pk_NdLmtP = SU_PK_Coeff[value].Pk_NdLmtP;
		}


		Coring = data.Pk_Coring;
		X1 = data.Pk_X1;
		ndG1 = data.Pk_NdG1;
		ndOffset = (X1 - Coring) * ndG1 / 4;

		data.ndOffset = ndOffset;

		drvif_color_scaler_setds_pk((DRV_SU_PK_Coeff *)(&data));
	}

#endif

}

void fwif_color_set_supk_type_driver(unsigned char src_idx, unsigned char value)
{
#ifndef BUILD_QUICK_SHOW

	extern DRV_SU_PK_Mask gVipSupk_Mask[];
	if (drvif_scaler3d_decide_isPR3dMode() && (drvif_scaler3d_decide_3d_PR_enter_3DDMA() == FALSE)) /*3D pure horizontal scaling PK type for PR 3D line merge mode*/{
		if (value != 6) {
			VIPprintf("Warning!! must set pure horizontal scaling PK for PR 3D mode (for 3D line merge)!!\n");
			VIPprintf("Current setting : %d, change setting to 6!!\n", value);
			value = 6;
		}
	}

	drvif_color_scaler_setds_supk_mask((DRV_SU_PK_Mask *)&gVipSupk_Mask[value]);

#endif
}

void fwif_color_set_intra_all_pacific_new_switch (unsigned char src_idx, unsigned char value)	/*Erin 100414*/
{
	drvif_color_intra_all_pacific_new_switch (value);

}

void fwif_color_set_inewdcti(unsigned char src_idx, unsigned char value)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	if (value >= DCTI_TABLE_LEVEL_MAX)
		value = DCTI_TABLE_LEVEL_MAX - 1;

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	VIP_system_info_structure_table->PQ_Setting_Info.I_D_Dcti_INFO.iDcti_table_select = value;

	drvif_color_inewdcti_table((DRV_VipNewIDcti_Table *)&gVipINewDcti_Table[value]);
}

void fwif_color_set_v_dcti_driver(unsigned char src_idx, unsigned char value)
{

	if (value >= DCTI_TABLE_LEVEL_MAX)
		value = 0;

	drvif_color_scaler_setds_vcti(_MAIN_DISPLAY,&gVipVcti_Table[value]);
}

void fwif_color_set_v_dcti_lpf_driver(unsigned char src_idx, unsigned char value)
{

	if (value >= DCTI_TABLE_LEVEL_MAX)
		value = 0;

	drvif_color_scaler_setds_vcti_lpf(_MAIN_DISPLAY,&gVipVcti_lpf_Table[value]);
}

unsigned char fwif_color_get_UV_Delay(void)
{
	unsigned char value;
	DRV_VipUV_Delay data;
	drvif_color_Get_UV_Delay(&data);

	value = data.u_delay*16 + data.v_delay;
	return value;
}


void fwif_color_set_UV_Delay(unsigned char src_idx, unsigned char value)
{
	DRV_VipUV_Delay data;

	data.u_delay = value/16;
	data.v_delay = value%16;

	drvif_color_Set_UV_Delay((DRV_VipUV_Delay *) &data);
}

void fwif_color_set_UV_Delay_TOP(unsigned char src_idx, unsigned char value)
{

	if(value == 1) {
		_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
		unsigned char bIsVDSrc = ((VIP_system_info_structure_table->Input_src_Form) == _SRC_FROM_VDC && vdc_power_status);
		value = value-1;
		if (bIsVDSrc)
			drvif_color_Set_UV_Delay_TOP((DRV_VipUV_Delay_TOP_Table*)&gVipUV_Delay_TOP_Table[value]);
	}

}


void fwif_color_UV_Delay_Enable(unsigned char src_idx, unsigned char value)
{
	drvif_color_UV_Delay_Enable((VIP_UV_DELAY_ENABLE)value);
}

#if 0
void fwif_color_set_Adaptive_Gamma(unsigned char src_idx, unsigned char value)
{
	drvif_color_Adaptive_Gamma(_MAIN_DISPLAY, value);
}
#else
//removed from merlin3, by caroline, 20170606
/*
void fwif_color_set_Adaptive_Gamma(unsigned char src_idx, unsigned char TableIdx)
{

	extern DRV_Adaptive_Gamma_Ctrl Adaptive_Gamma_Ctrl_Table[12];
	DRV_Adaptive_Gamma_Ctrl data = {0};

	if (TableIdx >= sizeof(Adaptive_Gamma_Ctrl_Table)/sizeof(DRV_Adaptive_Gamma_Ctrl))
		drvif_color_set_Adaptive_Gamma((DRV_Adaptive_Gamma_Ctrl *)(&data));
	else {
		data.gamma_adapt_en = Adaptive_Gamma_Ctrl_Table[TableIdx].gamma_adapt_en;
		data.sat_thl = Adaptive_Gamma_Ctrl_Table[TableIdx].sat_thl;
		data.sat_thl_mul = Adaptive_Gamma_Ctrl_Table[TableIdx].sat_thl_mul;

		drvif_color_set_Adaptive_Gamma((DRV_Adaptive_Gamma_Ctrl *)(&data));
	}
}
*/

#endif

void fwif_color_data_range_convert(unsigned char channel, unsigned char nSrcType, unsigned char *contrast, unsigned char *brightness)
{
	unsigned char cont_gain, bri_gain;

	if (channel !=  _CHANNEL1)
		return;

	if ((nSrcType == _SRC_CVBS) || (nSrcType == _SRC_SV) || (nSrcType == _SRC_COMPONENT) || (nSrcType == _SRC_YPBPR) || (nSrcType == _SRC_TV)
				|| (nSrcType == _SRC_VO)
#if 1/*def CONFIG_SCART_ENABLE*/
				|| (nSrcType == _SRC_SCART) || (nSrcType == _SRC_SCART_RGB)
#endif
#ifdef CONFIG_INPUT_SOURCE_IPG
				|| (nSrcType == _SRC_IPG)
#endif
				) {
		cont_gain = 141;	/*experimental value, amplify by 128*/
		bri_gain = 105;
	} else if (nSrcType == _SRC_HDMI) {
		unsigned char video_format = drvif_Hdmi_GetColorSpace();

		if (drvif_IsHDMI() && (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR) != _MODE_NEW)) /*HDMI Video Mode,  gary for xoceco 20071227*/{
			if (video_format !=  0) {/*YUV444 or YUV422*/
				cont_gain = 141;	/*experimental value, amplify by 128*/
				bri_gain = 105;
			} else {	/*RGB*/
				if (FORCE_DATA_RANGE&0x1) {/*force data as limit range*/
					cont_gain = 141;	/*experimental value, amplify by 128*/
					bri_gain = 105;
				} else if (FORCE_DATA_RANGE&0x2) {/*force data as full range*/
					cont_gain = 128;
					bri_gain = 128;
				} else {/*auto detect, */
					HDMI_RGB_YUV_RANGE_MODE_T yuv_range;
					yuv_range = drvif_IsRGB_YUV_RANGE();
					if (yuv_range == MODE_RAG_LIMIT) {
						cont_gain = 141;	/*experimental value, amplify by 128*/
						bri_gain = 105;
					} else {
						cont_gain = 128;	/*experimental value, amplify by 128*/
						bri_gain = 128;
					}
				}
			}
		} else { /*DVI mode*/
			if ((video_format&(_BIT4|_BIT5))) {  /*YUV444 or YUV422*/
				cont_gain = 141;	/*experimental value, amplify by 128*/
				bri_gain = 105;
			} else {  	/*RGB*/
				if (FORCE_DATA_RANGE&0x1) {/*force data as limit range*/
					cont_gain = 141;	/*experimental value, amplify by 128*/
					bri_gain = 105;
				} else if (FORCE_DATA_RANGE&0x2) {/*force data as full range*/
					cont_gain = 128;
					bri_gain = 128;
				} else {/*auto detect, */
					HDMI_RGB_YUV_RANGE_MODE_T yuv_range;
					yuv_range = drvif_IsRGB_YUV_RANGE();
					if (yuv_range == MODE_RAG_LIMIT) {
						cont_gain = 141;	/*experimental value, amplify by 128*/
						bri_gain = 105;
					} else {
						cont_gain = 128;	/*experimental value, amplify by 128*/
						bri_gain = 128;
					}
				}
			}
		}
	} else {
		cont_gain = 128;
		bri_gain = 128;
	}

	*contrast = ((*contrast) * cont_gain) >> 7;
	*brightness = ((*brightness) * bri_gain) >> 7;

}

/**
 * ColorSpaceTransfer
 * Set Color space rgb2yuv conversion matrix
 * for source CCIR 601 YCbCr: Bypass RGB2YUV
 * for source CCIR 601 RGB: Use Tab -1 as ccir601 RGB2YUV
 * for source CCIR 709 YPbPr: overwrite Tab - 2 as ccir709 YPbPr to 601 YCbCr
 * for source CCIR 709 RGB: overwrite Tab - 2 as ccir709 RGB to 601 YCbCr
 */
unsigned char fwif_color_get_colorspacergb2yuvtransfer_table(unsigned char channel, unsigned char nSrcType, unsigned char nSD_HD, bool bUse_HDMI_Range_Detect)
{
	unsigned char video_format;
	/*unsigned short *table_index = 0;*/
	unsigned char table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
	unsigned char CHOOSE_DATA_RANGE = 1;
	UINT8 data_range = 0; /*morgan_shen  20121112  choose rgb2yuv table by the flag of data range.*/
	RGB2YUV_BLACK_LEVEL blk_lv = fwif_color_get_rgb2yuv_black_level_tv006();
	UINT8 isBt2020_Vo =0, isBt2020_HDMI = 0;
	UINT8 isHDMIFullRangeYCC = 0;
	SLR_VOINFO* pVOInfo = NULL;
	/*pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());*/

	HDMI_AVI_T pAviInfo = {0};
	/*drvif_Hdmi_GetAviInfoFrame((unsigned char*) (&pAviInfo)); //pAviInfo size = 16 bytes*/

	extern unsigned char RGB2YUV_if_RGB_mode;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if (fwif_VIP_get_Project_ID() == VIP_Project_ID_TV030)
		blk_lv = RGB2YUV_BLACK_LEVEL_LOW;

	if (RGB2YUV_BLACK_LEVEL_AUTO == blk_lv || RGB2YUV_BLACK_LEVEL_UNDEFINE == blk_lv)
		bUse_HDMI_Range_Detect = true;
	else
		bUse_HDMI_Range_Detect = false;


	if (fwif_color_get_force_run_i3ddma_enable(channel)) {
		if (nSrcType == _SRC_HDMI) {
			if (drvif_IsHDMI() && drvif_Hdmi_GetColorSpace() != 0 && (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR) != _MODE_NEW)) {

				switch(Scaler_GetHDMI_CSC_DataRange_Mode()) {
				case HDMI_CSC_DATA_RANGE_MODE_AUTO:
					isHDMIFullRangeYCC = drvif_IsRGB_YUV_RANGE()==MODE_RAG_FULL?1:0;
					break;
				case HDMI_CSC_DATA_RANGE_MODE_FULL:
					isHDMIFullRangeYCC = 1;
					break;
				case HDMI_CSC_DATA_RANGE_MODE_LIMITED:
				default:
					isHDMIFullRangeYCC = 0;
					break;
					;
				}
			}
			isBt2020_Vo |= isBt2020_HDMI;
		}
		if (nSrcType == _SRC_VGA || nSrcType == _SRC_HDMI || nSrcType == _SRC_SCART_RGB)
			nSrcType = _SRC_VO;
	}

	//VIPprintf("drvif_color_get_colorspacergb2yuvtransfer_table_tv006, nSrcType = %d, nSD_HD = %d\n", nSrcType, nSD_HD);
#if 0	/* move to HDMI and vo case*/
	/* for bt2020*/
	if (VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag == 1 && VIP_system_info_structure_table->BT2020_CTRL.Mode == BT2020_MODE_Constant) {
		table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_ConstantY;
		return table_num;
	} else if (VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag == 1 && VIP_system_info_structure_table->BT2020_CTRL.Mode == BT2020_MODE_Non_Constant) {
		table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
		return table_num;
	}
#endif
		if (nSrcType == _SRC_VGA) {
			if (RGB2YUV_BLACK_LEVEL_LOW == blk_lv) {
				if (!nSD_HD) {
					table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
				} else {
					table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC709_16_235;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
				}
			} else {
				if (!nSD_HD) {
					table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
				} else {
					table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
				}
			}
			RGB2YUV_if_RGB_mode = TRUE;

		} else if ((nSrcType == _SRC_YPBPR) || (nSrcType == _SRC_COMPONENT))  {
			if (RGB2YUV_BLACK_LEVEL_LOW == blk_lv) {
				if (!nSD_HD) {
					table_num = RGB2YUV_COEF_TV006_YPP_LOW;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
				} else {
					table_num = RGB2YUV_COEF_TV006_YPP_LOW;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
				}
			} else {
				if (!nSD_HD) {
					table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
				} else {
					table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
				}
			}
		} else if (nSrcType == _SRC_SCART_RGB) {
			if (RGB2YUV_BLACK_LEVEL_HIGH == blk_lv) {
				if (!nSD_HD) {
					table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
				} else {
					table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
				}
			} else {
				if (!nSD_HD) {
					table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
				} else {
					table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
				}
			}
		} else if ((nSrcType == _SRC_TV) || (nSrcType == _SRC_CVBS) || (nSrcType == _SRC_SV) || (nSrcType == _SRC_SCART)) {
			table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
		} else if (nSrcType == _SRC_HDMI) {

			video_format = drvif_Hdmi_GetColorSpace();

			if (CHOOSE_DATA_RANGE) {
				data_range = drvif_IsRGB_YUV_RANGE();  /*0 :default 1 : Limited Range 2: Full Range*/

				switch(Scaler_GetHDMI_CSC_DataRange_Mode()) {
				case HDMI_CSC_DATA_RANGE_MODE_AUTO:
					break;
				case HDMI_CSC_DATA_RANGE_MODE_FULL:
					data_range = MODE_RAG_FULL;
					break;
				case HDMI_CSC_DATA_RANGE_MODE_LIMITED:
					data_range = MODE_RAG_LIMIT;
					break;
				default:
					;
				}

			}

			if(pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_SMPTE170) //601
				nSD_HD = 0;
			else if(pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_ITU709) //709
				nSD_HD = 1;
			else if(pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_FUTURE)
			{
				if(pAviInfo.EC == 5 || pAviInfo.EC == 6)//bt2020
					isBt2020_HDMI = 1;
				else if(pAviInfo.EC ==1)//709
					nSD_HD = 1;
				else if(pAviInfo.EC ==0)//601
					nSD_HD = 0;
				else if(VIP_system_info_structure_table ->HDR_flag)
					isBt2020_HDMI = 1;
				else //709
					nSD_HD = 1;
			}
			rtd_pr_vpq_info("HDMI,pAviInfo.C=%d,nSD_HD=%d,pAviInfo.EC=%d,isBt2020_HDMI=%d,HDR_flag=%d,drvif_IsHDMI()=%d,Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR)=%d\n",pAviInfo.C,nSD_HD,pAviInfo.EC,isBt2020_HDMI,VIP_system_info_structure_table ->HDR_flag,drvif_IsHDMI(),Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR));
			if (drvif_IsHDMI() && (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR) != _MODE_NEW)) /*HDMI Video Mode,  gary for xoceco 20071227*/{
				if (video_format !=  0) {/*YUV444 or YUV422*/
					if (isBt2020_HDMI == 1) {
						if ((bUse_HDMI_Range_Detect && data_range == MODE_RAG_FULL) || (RGB2YUV_BLACK_LEVEL_HIGH == blk_lv)) {
							table_num = RGB2YUV_COEF_YCC2020_0_255_TO_YCC709_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						} else {
							table_num = RGB2YUV_COEF_YCC2020_16_235_TO_YCC709_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					} else if (!nSD_HD) {
						if ((bUse_HDMI_Range_Detect && data_range == MODE_RAG_FULL) || (RGB2YUV_BLACK_LEVEL_HIGH == blk_lv)) {
							table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						}
					} else {
						if ((bUse_HDMI_Range_Detect && data_range == MODE_RAG_FULL) || (RGB2YUV_BLACK_LEVEL_HIGH == blk_lv)) {
							table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						} else {
							table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					}
					printk("RGB2YUV_SRC_HDMI_YUV\n");
				} else {/*RGB*/

					RGB2YUV_if_RGB_mode = TRUE;
					if ((bUse_HDMI_Range_Detect && data_range == MODE_RAG_LIMIT) || RGB2YUV_BLACK_LEVEL_LOW == blk_lv) {
						if (isBt2020_HDMI == 1) {
							table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
						} else if (!nSD_HD) {
							table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC709_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					} else {
						if (isBt2020_HDMI == 1) {
							table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
						} else if (!nSD_HD) {
							table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					}
					printk("RGB2YUV_SRC_HDMI_RGB\n");
				}
			} else { /*DVI mode*/
				if ((video_format&(_BIT4|_BIT5))) { /*YUV444 or YUV422*/
					if (!nSD_HD) {
						if ((bUse_HDMI_Range_Detect && data_range == MODE_RAG_FULL) || RGB2YUV_BLACK_LEVEL_HIGH == blk_lv) {
							table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						}
					} else {
						if ((bUse_HDMI_Range_Detect && data_range == MODE_RAG_FULL) || RGB2YUV_BLACK_LEVEL_HIGH == blk_lv) {
							table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						} else {
							table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					}
					printk("RGB2YUV_SRC_DVI_YUV\n");
				} else {/*RGB*/
					RGB2YUV_if_RGB_mode = TRUE;
					if ((bUse_HDMI_Range_Detect && data_range == MODE_RAG_LIMIT) || RGB2YUV_BLACK_LEVEL_LOW == blk_lv) {
						if (!nSD_HD) {
							table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC709_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					} else {
						if (!nSD_HD) {
							table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					}
					printk("RGB2YUV_SRC_DVI_RGB\n");
				}
			}
		} else if (nSrcType == _SRC_VO) {
			pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
			if (pVOInfo == NULL) {
				isBt2020_Vo = 0;
			} else {
				if (pVOInfo->colour_primaries == 9)	/* bt2020*/
					isBt2020_Vo = 1;
				else
					isBt2020_Vo = 0;
			}

		    if((Scaler_VOFromJPEG(Scaler_Get_CurVoInfo_plane()) == 1) &&  // 0) DTV, 1) JPEG
				(pVOInfo->colorspace == 0)) {	// pVOInfo->colorspace == 0 ==> YUV
		    			printk("VO JPEG\n");
					if (!nSD_HD) {
						table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
					} else {
						table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
					}
			}else if ((Scaler_DispGetInputInfo(SLR_INPUT_COLOR_SPACE) == COLOR_RGB) || 
			(Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 2) ||
			(pVOInfo->colorspace == 1)/* aRGB case */) {
					   printk("VO RGB\n");
				if (RGB2YUV_BLACK_LEVEL_LOW == blk_lv) {
					if (isBt2020_Vo == 1) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_0_255;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
					}
					else if (!nSD_HD) {
						table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
					} else {
						table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC709_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
					}
				} else {
					if (isBt2020_Vo == 1) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
					}
					else if (!nSD_HD) {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
					} else {
						table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
					}
				}
			} else if (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 0) { /*DTV source*/
					   printk("VO DTV\n");
				if (RGB2YUV_BLACK_LEVEL_HIGH == blk_lv || isHDMIFullRangeYCC) {
					if (isBt2020_Vo == 1) {
						table_num = RGB2YUV_COEF_YCC2020_0_255_TO_YCC709_16_235;
						if (fwif_VIP_get_Project_ID() == VIP_Project_ID_TV030)
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
						else
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
					} else if (!nSD_HD) {
						table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
					} else {
						table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
					}
				} else {
					if (VIP_system_info_structure_table ->HDR_flag) {
						if (isBt2020_Vo == 1) {
							table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC2020_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						} else if (!nSD_HD) {
							table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					}else{
						if (isBt2020_Vo == 1) {
						table_num = RGB2YUV_COEF_YCC2020_16_235_TO_YCC709_16_235;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						} else if (!nSD_HD) {
						table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
						table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
						VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					}
				}
			}
		} else {
			/*which_tab = 0;*/
			//VIPprintf("unhandler type%d\n", nSrcType);
			table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
			/*return NULL;*/
	  }

	rtd_pr_vpq_info("ch=%d, Src=%d, nSD_HD=%d, HDMI_Range_Detect=%d, blk_lv=%d, force_run_i3ddma_en()=%d, IsHDMI()=%d, Hdmi_ColorSpace()=%d, isHDMIFullRangeYCC = %d, HDR_flag = %d, isBt2020_Vo = %d, prjID = %d\n",
		channel, nSrcType, nSD_HD, bUse_HDMI_Range_Detect, blk_lv, fwif_color_get_force_run_i3ddma_enable(channel), drvif_IsHDMI(), drvif_Hdmi_GetColorSpace(), isHDMIFullRangeYCC, VIP_system_info_structure_table ->HDR_flag, isBt2020_Vo, fwif_VIP_get_Project_ID());
	if(pVOInfo != NULL){
		rtd_pr_vpq_info("Hdmi_ModeInfo()=%d, HDMI_CSC_DataRange()=%d, IsRGB_YUV()=%d, EC=%d, colour_primaries=%d,InputInfo()=%d,VOFrom()=%d, Src()=%d, VO_colorspace = %d\n",
			Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR), Scaler_GetHDMI_CSC_DataRange_Mode(), drvif_IsRGB_YUV_RANGE(), pAviInfo.EC, pVOInfo->colour_primaries,
			Scaler_DispGetInputInfo(SLR_INPUT_COLOR_SPACE), Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()), Get_DisplayMode_Src(SLR_MAIN_DISPLAY), pVOInfo->colorspace);
	}
	//printk("[rgb2yuv]table_num=%d\n",table_num);
	return table_num;

}

extern unsigned char g_LGE_HDR_CSC_CTRL;
void fwif_color_colorspacergb2yuvtransfer(unsigned char channel, unsigned char nSrcType, unsigned char nSD_HD, unsigned char VO_Jpeg_Back_YUV)
{
	/*VIP_QUALITY_Extend2_Coef* pVipCoefArray = fwif_color_get_VIP_Extend2_CoefArray(fwif_vip_source_check(3, 0));*/
	//unsigned char *pVipCoefArray = (unsigned char *)fwif_color_get_VIP_Extend2_CoefArray(fwif_vip_source_check(3, 0));
	//UINT8 colorspace_control = 0;
	unsigned short *table_index = NULL;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	unsigned char table_num = RGB2YUV_COEF_MATRIX_MODE_Max;
#if 0
	SLR_VOINFO* pVOInfo = NULL;
	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());

	if (pVipCoefArray)
		/*colorspace_control = pVipCoefArray->ColorSpace_control;*/
		colorspace_control = pVipCoefArray[VIP_QUALITY_FUNCTION_ColorSpace_Control];
	/*VIPprintf("csp_control = %d, fwif_vip_source_check(3, 0) = %d\n", csp_control, fwif_vip_source_check(3, 0));*/
#endif
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL || VIP_RPC_system_info_structure_table == NULL)
		return;
#if 0
	switch (colorspace_control) {
	case RGB2YUV_OUT_DATA_Old_Mode:	/*old mode*/
			table_index = fwif_color_get_colorspacergb2yuvtransfer_org_table(channel, nSrcType, nSD_HD, VO_Jpeg_Back_YUV, false);
			break;
	case RGB2YUV_OUT_DATA_16_235: 	/*RGB2YUV	16 - 235*/
	case RGB2YUV_OUT_DATA_0_255: 	/*RGB2YUV	0 - 255*/
			table_index = fwif_color_get_colorspacergb2yuvtransfer_new_table(nSrcType, nSD_HD, colorspace_control);
			break;
	case RGB2YUV_OUT_DATA_newTable_newPath_Mode: 	/*new table new path*/
			table_num = fwif_color_get_colorspacergb2yuvtransfer_new_table_new_path(nSrcType, nSD_HD, colorspace_control, VIP_system_info_structure_table);
			table_index = &(tRGB2YUV_COEF[table_num][0]);
			break;
	case RGB2YUV_OUT_DATA_0_255_HDR10_Mode:
			//table_index = &(tRGB2YUV_COEF[RGB2YUV_COEF_RGB_0_255_TO_YCC709_0_255][0]);
			table_index = &(tRGB2YUV_COEF[RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235][0]);
			break;
	default:
			VIPprintf("fwif_color_get_colorspacergb2yuvtransfer_new_table, colorspace_control ERROR, %d\n", colorspace_control);
			return;
			break;
	}
#endif
	//20150915 roger, LGE HDR flow not perfect, so using flag to control
	if (fwif_VIP_get_Project_ID() == VIP_Project_ID_TV006) {
		if ((VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En] == 0) || (channel == SLR_SUB_DISPLAY)) {		//20160107 roger, sub == non-HDR in K2L
			table_num = fwif_color_get_colorspacergb2yuvtransfer_table_tv006(channel, nSrcType, nSD_HD, true);
			table_index = &(tRGB2YUV_COEF[table_num][0]);
		} else {
			if (g_LGE_HDR_CSC_CTRL == 1) {	/*HAL_VPQ_HDR_CSC1_BT709 = 1, HAL_VPQ_HDR_BT2020 = 2*/
					table_index = &(tRGB2YUV_COEF[RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235][0]);
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
			} else {
					table_index = &(tRGB2YUV_COEF[RGB2YUV_COEF_RGB_0_255_TO_YCC2020_16_235][0]);
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
			}
		}
	} else {
			if (VIP_system_info_structure_table ->HDR_flag == HAL_VPQ_HDR_MODE_ST2094) {
				table_index = &(tRGB2YUV_COEF[RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235][0]);
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
			}
			else if(VIP_system_info_structure_table ->HDR_flag == HAL_VPQ_HDR_MODE_HDR10 ||
				VIP_system_info_structure_table ->HDR_flag == HAL_VPQ_HDR_MODE_HLG) {
				table_index = &(tRGB2YUV_COEF[RGB2YUV_COEF_RGB_0_255_TO_YCC2020_16_235][0]);
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
			} else {
				table_num = fwif_color_get_colorspacergb2yuvtransfer_table(channel, nSrcType, nSD_HD, true);
    			table_index = &(tRGB2YUV_COEF[table_num][0]);
			}
		

	}
#if 1
	/* KTASKWBS-5741, V com pattern, use 601->709 matrix*/
	if ((Get_DISPLAY_PANEL_MPLUS_RGBW() == TRUE /*|| Get_DISPLAY_PANEL_BOW_RGBW() == TRUE*/) && Get_Val_vsc_run_pc_mode() == 1
		&& Get_DisplayMode_Src(SLR_MAIN_DISPLAY)  == VSC_INPUTSRC_JPEG && drvif_M_Plus_Status_Check() == 0) {
		table_num = RGB2YUV_COEF_YCC601_0_255_TO_YCC709_0_255;
		table_index = &(tRGB2YUV_COEF[table_num][0]);
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
	}
#endif
rtd_pr_vpq_emerg("rgb2yuv,display=%d,tab_num=%d,Input_Data_Mode=%d,M+=%d,PANEL_BOW_RGBW=%d, pc_mode=%d,Mode_src=%d,HDR_06en=%d, HDR_csc=%d,HDR_flag=%d\n",
		channel, table_num, VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode, Get_DISPLAY_PANEL_MPLUS_RGBW(),
		Get_DISPLAY_PANEL_BOW_RGBW(), Get_Val_vsc_run_pc_mode(), Get_DisplayMode_Src(SLR_MAIN_DISPLAY),
		VIP_RPC_system_info_structure_table->HDR_info.Ctrl_Item[TV006_HDR_En], g_LGE_HDR_CSC_CTRL,VIP_system_info_structure_table ->HDR_flag);

	if (NULL !=  table_index) {
		/*set using matrix to share memory*/
		if (VIP_system_info_structure_table !=  NULL) {
#ifdef CONFIG_ARM64
			memcpy_toio((unsigned short *)(&VIP_system_info_structure_table->RGB2YUV_COEF_CSMatrix), table_index, sizeof(VIP_RGB2YUV_COEF_CSMatrix));
#else
			memcpy((unsigned short *)(&VIP_system_info_structure_table->RGB2YUV_COEF_CSMatrix), table_index, sizeof(VIP_RGB2YUV_COEF_CSMatrix));
#endif
			fwif_color_ChangeUINT16Endian((unsigned short *)&VIP_system_info_structure_table->RGB2YUV_COEF_CSMatrix, sizeof(VIP_RGB2YUV_COEF_CSMatrix)/sizeof(short), 0);
		} else {
			VIPprintf("VIP_system_info_structure_table = NULL, fwif_color_colorspacergb2yuvtransfer\n");
		}

		/*fwif_color_colorspacergb2yuv_set_yuv2rgb_inputMode(channel, table_index);*/
		drvif_color_colorspacergb2yuvtransfer(channel, table_index);

#ifdef CONFIG_FORCE_RUN_I3DDMA
		if (channel == SLR_MAIN_DISPLAY && fwif_color_get_force_run_i3ddma_enable(channel)) {
			drvif_HDMI_HDR_I3DDMA_RGB2YUV_only_Matrix(tRGB2YUV_COEF[(nSD_HD?RGB2YUV_COEF_RGB_16_235_TO_YCC709_16_235:RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235)]);
		}
#endif //CONFIG_FORCE_RUN_I3DDMA
	} else {
		VIPprintf("ERROR, table_index == NULL !!\n");
	}

	if (VIP_system_info_structure_table == NULL || VIP_RPC_system_info_structure_table == NULL) {
		VIPprintf("ERROR, VIP_system_info_structure_table == NULL !!\n");
	} else {
		fwif_color_set_DCC_YUV2RGB_CTRL(channel, nSrcType, nSD_HD, VO_Jpeg_Back_YUV, VIP_system_info_structure_table);
		fwif_color_set_ICM_YUV2RGB_CTRL(channel, nSrcType, nSD_HD, VO_Jpeg_Back_YUV, VIP_system_info_structure_table);
	}
}

#if 0
void fwif_color_colorspacergb2yuvtransfer_Seamless(unsigned char HDR_Mode, unsigned char nSrcType)
{
	unsigned short *table_index = NULL;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	unsigned char table_num = RGB2YUV_COEF_MATRIX_MODE_Max;

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL)
		return;

	if (HDR_Mode == 0) {
		table_num = fwif_color_get_colorspacergb2yuvtransfer_table_tv006(SLR_MAIN_DISPLAY, nSrcType, 1, true);
		//table_num = fwif_color_get_colorspacergb2yuvtransfer_new_table_new_path(nSrcType, 1, RGB2YUV_OUT_DATA_newTable_newPath_Mode, VIP_system_info_structure_table);
		table_index = &(tRGB2YUV_COEF[table_num][0]);
	} else {
		/* use webos flag to determine csc*/
		if (g_LGE_HDR_CSC_CTRL == 1) {	/*HAL_VPQ_HDR_CSC1_BT709 = 1, HAL_VPQ_HDR_BT2020 = 2*/
				table_index = &(tRGB2YUV_COEF[RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235][0]);
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
		} else {
				table_index = &(tRGB2YUV_COEF[RGB2YUV_COEF_RGB_0_255_TO_YCC2020_16_235][0]);
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_2020_NonConstantY;
		}
	}

	/* seamless is in porch*/
	if (NULL !=  table_index) {
		/*set using matrix to share memory*/
		if (VIP_system_info_structure_table !=  NULL) {
#ifdef CONFIG_ARM64
			memcpy_toio((unsigned short *)(&VIP_system_info_structure_table->RGB2YUV_COEF_CSMatrix), table_index, sizeof(VIP_RGB2YUV_COEF_CSMatrix));
#else
			memcpy((unsigned short *)(&VIP_system_info_structure_table->RGB2YUV_COEF_CSMatrix), table_index, sizeof(VIP_RGB2YUV_COEF_CSMatrix));
#endif
			fwif_color_ChangeUINT16Endian((unsigned short *)&VIP_system_info_structure_table->RGB2YUV_COEF_CSMatrix, sizeof(VIP_RGB2YUV_COEF_CSMatrix)/sizeof(short), 0);
		} else {
			VIPprintf("VIP_system_info_structure_table = NULL, fwif_color_colorspacergb2yuvtransfer\n");
		}

		/*fwif_color_colorspacergb2yuv_set_yuv2rgb_inputMode(channel, table_index);*/
		drvif_color_colorspacergb2yuvtransfer(SLR_MAIN_DISPLAY, table_index);

#if 0
#ifdef CONFIG_FORCE_RUN_I3DDMA
		if (channel == SLR_MAIN_DISPLAY && fwif_color_get_force_run_i3ddma_enable(channel)) {
			drvif_HDMI_HDR_I3DDMA_RGB2YUV_only_Matrix(tRGB2YUV_COEF[(nSD_HD?RGB2YUV_COEF_RGB_16_235_TO_YCC709_16_235:RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235)]);
		}
#endif
#endif
	} else {
		VIPprintf("ERROR, table_index == NULL !!\n");
	}
}
#endif

void fwif_color_set_DCC_YUV2RGB_CTRL(unsigned char display, unsigned char nSrcType, unsigned char nSD_HD, unsigned char VO_Jpeg_Back_YUV,  _system_setting_info *VIP_system_info_structure_table)
{
	unsigned char isHD, isFull_range;

	unsigned char data_format_range;

	data_format_range = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode;

	switch (data_format_range) {
		case YUV2RGB_INPUT_601_Limted:
			isHD = 0;
			isFull_range = 0;
			break;

		case YUV2RGB_INPUT_709_Limted:
		case YUV2RGB_INPUT_2020_NonConstantY:
		case YUV2RGB_INPUT_2020_ConstantY:
			isHD = 1;
			isFull_range = 0;
			break;

		case YUV2RGB_INPUT_601_Full:
			isHD = 0;
			isFull_range = 1;
			break;

		case YUV2RGB_INPUT_709_Full:
		case YUV2RGB_INPUT_2020_NonConstantY_Full:
		case YUV2RGB_INPUT_2020_ConstantY_Full:
			isHD = 1;
			isFull_range = 1;
			break;

		default:
			isHD = 1;
			isFull_range = 1;
			break;

	}
	drvif_color_DCC_ctrl_YUV2RGB(isHD, isFull_range);

}

void fwif_color_set_ICM_YUV2RGB_CTRL(unsigned char display, unsigned char nSrcType, unsigned char nSD_HD, unsigned char VO_Jpeg_Back_YUV,  _system_setting_info *VIP_system_info_structure_table)
{
	unsigned char isHD, isFull_range;

	unsigned char data_format_range;

	data_format_range = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode;

	switch (data_format_range) {
		case YUV2RGB_INPUT_601_Limted:
			isHD = 0;
			isFull_range = 0;
			break;

		case YUV2RGB_INPUT_709_Limted:
		case YUV2RGB_INPUT_2020_NonConstantY:
		case YUV2RGB_INPUT_2020_ConstantY:
			isHD = 1;
			isFull_range = 0;
			break;

		case YUV2RGB_INPUT_601_Full:
			isHD = 0;
			isFull_range = 1;
			break;

		case YUV2RGB_INPUT_709_Full:
		case YUV2RGB_INPUT_2020_NonConstantY_Full:
		case YUV2RGB_INPUT_2020_ConstantY_Full:
			isHD = 1;
			isFull_range = 1;
			break;

		default:
			isHD = 1;
			isFull_range = 1;
			break;

	}

	isFull_range = 1;	//force full range for all case
	drvif_color_Icm_ctrl_YUV2RGB(isHD, isFull_range);

}

#if 0
void fwif_color_colorspacergb2yuv_set_yuv2rgb_inputMode(unsigned char display, unsigned short *table_idx)
{

	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	if (table_idx == tRGB2YUV_COEF_YCC601_BYPASS_NORMAL_GAIN)
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
	else if (table_idx == tRGB2YUV_COEF_YCC601_0_255_TO_YCC601_16_235)
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
	/*else if (table_idx == tRGB2YUV_COEF_YCC709_16_235_TO_YCC601_16_235)
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
	else if (table_idx == tRGB2YUV_COEF_YCC709_0_255_TO_YCC601_16_235)
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;*/
	else if (table_idx == tRGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235)
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
	else if (table_idx == tRGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235)
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
	else if (table_idx == tRGB2YUV_COEF_YCC709_BYPASS_NORMAL_GAIN)
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
	else if (table_idx == tRGB2YUV_COEF_YCC709_0_255_TO_YCC709_16_235)
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
	else		/* other matrix is old path, use 601 YUV2RGB matrix*/
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;

}
#endif

void fwif_color_setycontrastbrightness(unsigned char display, unsigned char Contrast, unsigned char Brightness)
{
	/*VIP_QUALITY_Extend2_Coef* pVipCoefArray = fwif_color_get_VIP_Extend2_CoefArray(fwif_vip_source_check(3, 0));*/
	/*UINT8 csp_control = pVipCoefArray->ColorSpace_control;*/
	unsigned char *pVipCoefArray = (unsigned char *)fwif_color_get_VIP_Extend2_CoefArray(fwif_vip_source_check(3, 0));
	UINT8 csp_control = 0;

	if (pVipCoefArray)
		csp_control = pVipCoefArray[VIP_QUALITY_FUNCTION_ColorSpace_Control];
	VIPprintf("csp_control = %d, fwif_vip_source_check(3, 0) = %d\n", csp_control, fwif_vip_source_check(3, 0));

	drvif_color_setycontrastbrightness_new(display, Contrast, Brightness, csp_control);
}

void fwif_colo_load_vip_table(SLR_VIP_TABLE *ptr)
{
	if (ptr == NULL) {
		VIPprintf("load pq table error\n");
		return;
	}
	/*memcpy((SLR_VIP_TABLE *)&gVip_Table, (SLR_VIP_TABLE *)ptr, sizeof(SLR_VIP_TABLE));*/
    gVip_Table = ptr;

	VIPprintf(" ===load_vip_table=== \n");
	fwif_colo_vip_ShareMemory_init(ptr);
}

SLR_VIP_TABLE *fwif_colo_get_AP_vip_table_gVIP_Table(void)
{
	if (gVip_Table == NULL) {
		VIPprintf("gVip_Table is NULL\n");
		return NULL;
	}
	return (SLR_VIP_TABLE *)gVip_Table;

	/*return (SLR_VIP_TABLE *)&gVip_Table;*/
}

void fwif_colo_vip_ShareMemory_init(SLR_VIP_TABLE *pVIPtable)
{
/*send all vip table to share memory*/
	VIPprintf("fwif_colo_vip_ShareMemory_init start\n");
	fwif_color_Send_VIP_Table(pVIPtable);
	fwif_color_Send_RPC_VIP_Table(pVIPtable);
#ifdef ENABLE_VIP_TABLE_CHECKSUM
/* =========	checksum ============*/
	fwif_color_check_VIPTable_crc(&vip_table_crc, pVIPtable);
	fwif_color_Send_VIP_Table_crc(&vip_table_crc);
	fwif_color_Send_RPC_VIP_Table(pVIPtable);
	VIPprintf("[%s %d] vip table checkSum: %x\n", __FILE__, __LINE__, vip_table_crc.checkSum);
/*=============================*/
#endif
	fwif_color_set_VIP_system_info_structure();
	fwif_color_set_VIP_RPC_system_info_structure();
	Scaler_Init_ColorLib_customer_share_memory_struct();/*for custom share memory init by each project*/
	VIPprintf("fwif_colo_vip_ShareMemory_init end\n");
}


unsigned char fwif_OsdMapToRegValue(APP_Video_OsdItemType_t bOsdItem, unsigned char value)
{
	unsigned int temp = 0;
	unsigned char startPoint, endPoint;
	unsigned char reg_point0 = 0;
	unsigned char reg_point25 = 0;
	unsigned char reg_point50 = 0;
	unsigned char reg_point75 = 0;
	unsigned char reg_point100 = 0;

	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	StructColorDataFacModeType *pColorFacTable;


#ifdef CONFIG_OSD_VALUE_NO_NEED_MAP_RANGE
			//printk( "[tim_li] fwif_OsdMapToRegValue value =%d\n",value);
			return value;
#endif

	if(fwif_VIP_get_Project_ID() == VIP_Project_ID_TV010)   // no re-mapping
	{
     	if (Scaler_VOFromJPEG(Scaler_Get_CurVoInfo_plane()) == 1)
			value=128;

	  	return value;
	} else if (fwif_VIP_get_Project_ID() == VIP_Project_ID_TV030) {
		return value;
	}

	if (g_bUseMiddleWareOSDmap)
		pColorFacTable = &g_curColorFacTableEx;
	else
		pColorFacTable = fwif_color_get_color_fac_mode(VIP_system_info_structure_table->color_fac_src_idx, 0);

	pr_emerg("g_bUseMiddleWareOSDmap =%d,g_curColorFacTableEx=0x%p \n",g_bUseMiddleWareOSDmap,&g_curColorFacTableEx);   
	if (pColorFacTable == NULL)
		return 0;

	switch (bOsdItem) {
	case APP_VIDEO_OSD_ITEM_BRIGHTNESS:
		reg_point0 = pColorFacTable->Brightness_0;
		reg_point25 = pColorFacTable->Brightness_25;
		reg_point50 = pColorFacTable->Brightness_50;
		reg_point75 = pColorFacTable->Brightness_75;
		reg_point100 = pColorFacTable->Brightness_100;
		break;
	case APP_VIDEO_OSD_ITEM_CONTRAST:
		reg_point0 = pColorFacTable->Contrast_0;
		reg_point25 = pColorFacTable->Contrast_25;
		reg_point50 = pColorFacTable->Contrast_50;
		reg_point75 = pColorFacTable->Contrast_75;
		reg_point100 = pColorFacTable->Contrast_100;
		break;
	case APP_VIDEO_OSD_ITEM_SATURTUION:
		reg_point0 = pColorFacTable->Saturation_0;
		reg_point25 = pColorFacTable->Saturation_25;
		reg_point50 = pColorFacTable->Saturation_50;
		reg_point75 = pColorFacTable->Saturation_75;
		reg_point100 = pColorFacTable->Saturation_100;
		break;
	case APP_VIDEO_OSD_ITEM_HUE:
		reg_point0 = pColorFacTable->Hue_0;
		reg_point25 = pColorFacTable->Hue_25;
		reg_point50 = pColorFacTable->Hue_50;
		reg_point75 = pColorFacTable->Hue_75;
		reg_point100 = pColorFacTable->Hue_100;
		break;
	case APP_VIDEO_OSD_ITEM_SHARPNESS:
	default:
		reg_point0 = pColorFacTable->Sharpness_0;
		reg_point25 = pColorFacTable->Sharpness_25;
		reg_point50 = pColorFacTable->Sharpness_50;
		reg_point75 = pColorFacTable->Sharpness_75;
		reg_point100 = pColorFacTable->Sharpness_100;
		break;
	}

	VIPprintf(" === bOsdItem = %d, reg_point0 = %d  reg_point25 = %d \n\n", bOsdItem, reg_point0, reg_point25);
	VIPprintf(" === reg_point50 = %d  reg_point75 = %d  reg_point100 = %d \n\n", reg_point50, reg_point75, reg_point100);
	if (value < 25) {
		startPoint = reg_point0;
		endPoint = reg_point25;
		temp = value;
	} else if ((value >=  25) && (value < 50)) {
		startPoint = reg_point25;
		endPoint = reg_point50;
		temp = value - 25;
	} else if ((value >=  50) && (value < 75)) {
		startPoint = reg_point50;
		endPoint = reg_point75;
		temp = value - 50;
	} else /*if (value >=  75) */{
		startPoint = reg_point75;
		endPoint = reg_point100;
		temp = value - 75;
	}

	if (endPoint >= startPoint) {
		temp = (endPoint - startPoint) * temp / 25;
		temp +=  startPoint;
	} else if ((endPoint < startPoint)) {
		temp = (startPoint-endPoint) * temp / 25;
		temp = startPoint - temp;
	}

	if (temp > 255)
		temp = 255;
	pr_emerg(" bOsdItem =%d,OSD_vaule=%d,reg_value=%d, reg_point0=%d,reg_point25=%d,reg_point50=%d,reg_point75=%d,reg_point100=%d \n", 
		bOsdItem,
		value,
		temp,
		reg_point0,
		reg_point25,
		reg_point50,
		reg_point75,
		reg_point100);

	/*printf2(" bOsdItem =%d,OSD_value=%d,reg_value=%d, reg_point0=%d,reg_point25=%d,reg_point50=%d,reg_point75=%d,reg_point100=%d \n", 
		bOsdItem,
		value,
		temp,
		reg_point0,
		reg_point25,
		reg_point50,
		reg_point75,
		reg_point100);*/

	return (unsigned char)temp;
}



/*==============================rord.tsao end=====================================================*/
#if 0
void fwif_color_set_YUV2RGB_COEF_BY_Y_driver(unsigned char src_idx, unsigned char which_table)
{
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	drvif_color_colorspaceyuv2rgbtransfer_coef_by_y(_MAIN_DISPLAY, which_table, gVip_Table->YUV2RGB_COEF_BY_Y[which_table]);
}

void fwif_color_set_YUV2RGB_UV_OFFSET_BY_Y_driver(unsigned char src_idx, unsigned char which_table)
{
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	drvif_color_colorspaceyuv2rgbtransfer_uv_offset_by_y(_MAIN_DISPLAY, which_table, gVip_Table->YUV2RGB_UV_OFFSET_BY_Y[which_table]);
}
#endif

unsigned char fwif_color_get_Decide_YUV2RGB_TBL_Index(unsigned char src_idx, unsigned char display, unsigned char Input_Data_Mode)
{
	unsigned char ret_idx;

	switch (Input_Data_Mode) {
		case  YUV2RGB_INPUT_601_Limted:
			ret_idx = YUV2RGB_TBL_SELECT_601_Limted_235_240_to_255;
			break;
		case  YUV2RGB_INPUT_601_Full:
			ret_idx = YUV2RGB_TBL_SELECT_601_Limted_NO_Gain;
			break;
		case  YUV2RGB_INPUT_709_Limted:
			ret_idx = YUV2RGB_TBL_SELECT_709_Limted_235_240_to_255;
			break;
		case  YUV2RGB_INPUT_709_Full:
			ret_idx = YUV2RGB_TBL_SELECT_709_Limted_NO_Gain;
			break;
		case  YUV2RGB_INPUT_2020_NonConstantY:
			ret_idx = YUV2RGB_TBL_SELECT_2020_NonConstantY;
			break;
		case  YUV2RGB_INPUT_2020_ConstantY:
			ret_idx = YUV2RGB_TBL_SELECT_2020_ConstantY;
			break;
		default:
			ret_idx = YUV2RGB_TBL_SELECT_601_Limted_NO_Gain;
			break;
	}
	return ret_idx;
}

void fwif_color_set_YUV2RGB(unsigned char src_idx, unsigned char display, unsigned char which_table, unsigned char drvSkip_Flag_coefByY, unsigned char drvSkip_Flag_uvOffset)
{
	/*base default setting*/
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	/*unsigned char i;*/
	SLR_VIP_TABLE *gVip_Table = NULL;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	unsigned char YUV2RGB_TBL_Idx;
	unsigned char Input_mode;

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL)
		return;

	Input_mode = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode;
	gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	/*use VIP_YUV2RGB_LEVEL_OFF as deault, set level by picture mode*/
	VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.RGB_Offset_Level = VIP_YUV2RGB_LEVEL_OFF;
	VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.UV_Offset_Level = VIP_YUV2RGB_LEVEL_OFF;
	VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.CoefByY_CtrlItem_Level = VIP_YUV2RGB_LEVEL_OFF;
	VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_TBL_Select = which_table;
	memcpy(&drv_vipCSMatrix_t, &(gVip_Table->YUV2RGB_CSMatrix_Table[which_table].YUV2RGB_CSMatrix[VIP_YUV2RGB_LEVEL_OFF]), sizeof(DRV_VIP_YUV2RGB_CSMatrix));
	YUV2RGB_TBL_Idx = fwif_color_get_Decide_YUV2RGB_TBL_Index(src_idx, display, Input_mode);

	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_YUV2RGB_Base_Ctrl, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_CoefByY_ctrl, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);

	/*TBL2 (Sub channel)*/
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_YUV2RGB_Base_Ctrl, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
	if (drvSkip_Flag_coefByY == 0) {
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_CoefByY_Index, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	}

	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_UVOffset_ctrl, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	if (drvSkip_Flag_uvOffset == 0) {
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_UVOffset_Coef, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_UVOffset_Index, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	}

}
#if 0 /*for LG DB, chane to fwif_color_set_YUV2RGB_UV_Offset_tv006, no more use this*/
int fwif_color_set_YUV2RGB_UV_Offset(unsigned char src_idx, unsigned char display, unsigned char level)
{
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	unsigned char which_table, vip_src;
	/*unsigned char i;*/
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if (gVip_Table == NULL || VIP_system_info_structure_table == NULL) {
		/*VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);*/
		VIPprintf("~get vipTable|shareMem Error return");
		return  -1;
	}

	if (display == SLR_MAIN_DISPLAY) {
		vip_src = VIP_system_info_structure_table->VIP_source;
		which_table = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_TBL_Select;
		drv_vipCSMatrix_t.CTRL_ITEM.UVOffset_En = 1;
		drv_vipCSMatrix_t.CTRL_ITEM.UVOffset_Mode_Ctrl = 0;	/*default setting, */
		VIPprintf("UV offset %d, %d, %d\n", which_table, vip_src, level);
		/*setting from UI*/
		memcpy(&drv_vipCSMatrix_t.UV_Offset, &(gVip_Table->YUV2RGB_CSMatrix_Table[which_table].YUV2RGB_CSMatrix[level].UV_Offset), sizeof(DRV_VIP_YUV2RGB_UV_Offset));
		/*info setting to share mem*/
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.UV_Offset_Level = level;
		memcpy(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.UV_Offset), &(gVip_Table->YUV2RGB_CSMatrix_Table[which_table].YUV2RGB_CSMatrix[level].UV_Offset), sizeof(DRV_VIP_YUV2RGB_UV_Offset));

		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_UVOffset_ctrl, VIP_YUV2RGB_Y_Seg_Max, display, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_UVOffset_Coef, VIP_YUV2RGB_Y_Seg_Max, display, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_UVOffset_Index, VIP_YUV2RGB_Y_Seg_Max, display, VIP_Table_Select_1);
	} else {
		VIPprintf("~No Sub~ return\n");
	}

	return 0;
}
#endif
void fwif_color_set_CS_Matrix_byTBL_Level(unsigned char src_idx, unsigned char display, unsigned char which_table, VIP_YUV2RGB_LEVEL_SELECT which_Lv, unsigned char drvSkip_Flag_coefByY, unsigned char drvSkip_Flag_uvOffset)
{
	/*now no use, set by con, bri, sat, hue*/
}

void fwif_color_set_UV_Offset_byTBL_Level(unsigned char src_idx, unsigned char display, unsigned char which_table, VIP_YUV2RGB_LEVEL_SELECT which_Lv, unsigned char drvSkip_Flag_coefByY, unsigned char drvSkip_Flag_uvOffset)
{
	/* base default setting*/
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t = {0};
	/*unsigned char i;*/

	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	if (VIP_system_info_structure_table == NULL)
		return;

	VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.UV_Offset_Level = which_Lv;
	VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_TBL_Select = which_table;

	memcpy(&(drv_vipCSMatrix_t.UV_Offset), &(gVip_Table->YUV2RGB_CSMatrix_Table[which_table].YUV2RGB_CSMatrix[which_Lv].UV_Offset), sizeof(DRV_VIP_YUV2RGB_UV_Offset));

	/* set using matrix to share memory*/
	if (VIP_system_info_structure_table != NULL) {
#ifdef CONFIG_ARM64
		memcpy_toio(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.UV_Offset), &drv_vipCSMatrix_t.UV_Offset, sizeof(DRV_VIP_YUV2RGB_UV_Offset));
#else
		memcpy(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.UV_Offset), &drv_vipCSMatrix_t.UV_Offset, sizeof(DRV_VIP_YUV2RGB_UV_Offset));
#endif
	} else
		VIPprintf("VIP_system_info_structure_table = NULL, fwif_color_colorspaceyuv2rgbtransfer\n");

	if (drvSkip_Flag_uvOffset == 0) {
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_UVOffset_Coef, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_UVOffset_Index, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	}

}


/*initialization in quality handler, mark thiss function. elieli*/
/*using fwif_color_set_YUV2RGB instead of both fwif_color_set_YUV2RGB_COEF_BY_Y and fwif_color_set_YUV2RGB_UV_OFFSET_BY_Y*/
#if 0
void fwif_color_set_YUV2RGB_COEF_BY_Y(unsigned char src_idx, unsigned char display, unsigned char which_table)
{
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	/*unsigned short *table_idx[YUV2RGB_COEF_BY_Y_Curve_Num];*/
	unsigned char i;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	drv_vipCSMatrix_t.CTRL_ITEM.CoefByY_En = 1;
	VIPprintf("YUV2RGB Coef by Y ON, table is %d\n", which_table);

	/*
	for (i = 0; i < YUV2RGB_COEF_BY_Y_Curve_Num; i++)
		table_idx[i] = (&(gVip_Table->YUV2RGB_COEF_BY_Y[0][i][0]) + (YUV2RGB_COEF_BY_Y_Curve_Num*YUV2RGB_COEF_BY_Y_Seg_Num*which_table));
	*/

	memcpy(&(drv_vipCSMatrix_t.COEF_By_Y), &(gVip_Table->YUV2RGB_CSMatrix[which_table].COEF_By_Y), sizeof(DRV_VIP_YUV2RGB_COEF_By_Y));
	/*
	for (i = 0; i < YUV2RGB_COEF_BY_Y_Seg_Num; i++)
	{
		drv_vipCSMatrix_t.COEF_By_Y.K11[i] = table_idx[0][i];
		drv_vipCSMatrix_t.K12[i] = table_idx[1][i];
		drv_vipCSMatrix_t.K13[i] = table_idx[2][i];
		drv_vipCSMatrix_t.K22[i] = table_idx[3][i];
		drv_vipCSMatrix_t.K23[i] = table_idx[4][i];
		drv_vipCSMatrix_t.K32[i] = table_idx[5][i];
		drv_vipCSMatrix_t.K33[i] = table_idx[6][i];
	}
	*/

	/*o-------------base setting----------o*/
	drv_vipCSMatrix_t.CTRL_ITEM.CoefByY_En = 1;
	/*o-------------base setting----------o*/

	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_CoefByY_ctrl);
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_Y_Only, VIP_YUV2RGB_Y_Seg_Max);
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_Max);

}

void fwif_color_set_YUV2RGB_UV_OFFSET_BY_Y(unsigned char src_idx, unsigned char display, unsigned char which_table)
{
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	/*unsigned int *table_idx[YUV2RGB_UV_OFFSET_BY_Y_Curve_Num];*/
	unsigned char i;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	drv_vipCSMatrix_t.CTRL_ITEM.UVOffset_En = 1;
	drv_vipCSMatrix_t.CTRL_ITEM.UVOffset_Mode_Ctrl = 0;	/*default setting, */
	VIPprintf("UV offset ON, table is %d\n", which_table);
	/*
	for (i = 0; i < YUV2RGB_UV_OFFSET_BY_Y_Curve_Num; i++)
		table_idx[i] = (&(gVip_Table->YUV2RGB_UV_OFFSET_BY_Y[0][i][0]) + (YUV2RGB_UV_OFFSET_BY_Y_Curve_Num*YUV2RGB_UV_OFFSET_BY_Y_Seg_Num*which_table));

	for (i = 0; i < YUV2RGB_UV_OFFSET_BY_Y_Seg_Num; i++)
	{
		drv_vipCSMatrix_t.Uoffset[i] = table_idx[0][i];
		drv_vipCSMatrix_t.Voffset[i] = table_idx[1][i];
		drv_vipCSMatrix_t.Ustep[i] = table_idx[2][i];
		drv_vipCSMatrix_t.Vstep[i] = table_idx[3][i];
	}
	*/

	memcpy(&(drv_vipCSMatrix_t.UV_Offset), &(gVip_Table->YUV2RGB_CSMatrix[which_table].UV_Offset), sizeof(DRV_VIP_YUV2RGB_UV_Offset));

	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_UVOffset_ctrl);
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_UVOffset_Coef);

}


void fwif_color_colorspaceyuv2rgbtransfer(unsigned char display, unsigned char nSrcType, unsigned char VO_Jpeg_Back_YUV)
{
	/*base default setting*/
	unsigned int *table_idx = 0;
	unsigned int *uvOffset_tableIdx = 0;
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	unsigned char i;

	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	/*use first table for default*/
	table_idx = &(gVip_Table->YUV2RGB_COEF_BY_Y[0][0][0]);
	uvOffset_tableIdx = &(gVip_Table->YUV2RGB_UV_OFFSET_BY_Y[0][0][0]);

	for (i = 0; i < VIP_CSMatrix_Index_Max; i++) {
		drv_vipCSMatrix_t.K11[i] = table_idx[i];
		drv_vipCSMatrix_t.K12[i] = table_idx[i+VIP_CSMatrix_Index_Max];
		drv_vipCSMatrix_t.K13[i] = table_idx[i+VIP_CSMatrix_Index_Max*2];
		drv_vipCSMatrix_t.K22[i] = table_idx[i+VIP_CSMatrix_Index_Max*3];
		drv_vipCSMatrix_t.K23[i] = table_idx[i+VIP_CSMatrix_Index_Max*4];
		drv_vipCSMatrix_t.K32[i] = table_idx[i+VIP_CSMatrix_Index_Max*5];
		drv_vipCSMatrix_t.K33[i] = table_idx[i+VIP_CSMatrix_Index_Max*6];
		drv_vipCSMatrix_t.Uoffset[i] = table_idx[i];
		drv_vipCSMatrix_t.Voffset[i] = table_idx[i+VIP_CSMatrix_Index_Max];
		drv_vipCSMatrix_t.Ustep[i] = table_idx[i+VIP_CSMatrix_Index_Max*2];
		drv_vipCSMatrix_t.Vstep[i] = table_idx[i+VIP_CSMatrix_Index_Max*3];
	}

	/*o-------------default base setting----------o*/
	drv_vipCSMatrix_t.Overlay = 0;
	drv_vipCSMatrix_t.YUV2RGB_Enable = 1;
	drv_vipCSMatrix_t.Table_Select = display;	/*main CH use table 1*/
	drv_vipCSMatrix_t.OutShift_En = 0;
	drv_vipCSMatrix_t.CbCr_Clamp = 1;
	drv_vipCSMatrix_t.Y_Clamp = 0;
	drv_vipCSMatrix_t.R_offset  = 0;
	drv_vipCSMatrix_t.G_offset  = 0;
	drv_vipCSMatrix_t.B_offset  = 0;
	drv_vipCSMatrix_t.CoefByY_En = 1;
	drv_vipCSMatrix_t.UVOffset_En = 1;
	drv_vipCSMatrix_t.UVOffset_Mode_Ctrl = 0; /*use and mode to avoid color distortion.*/
	/*o-------------default base setting----------o*/

	for (i = 0; i < VIP_CSMatrix_MAX; i++)
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, (VIP_CSMatrix_WriteType)i, VIP_CSMatrix_Index_Max);

	if (table_idx !=  NULL) {
		/*set using matrix to share memory*/
		if (VIP_system_info_structure_table !=  NULL) {
			memcpy((unsigned short *)(&VIP_system_info_structure_table->YUV2RGB_COEF_CSMatrix), &drv_vipCSMatrix_t, sizeof(DRV_VIP_YUV2RGB_CSMatrix));
		} else {
			VIPprintf("VIP_system_info_structure_table = NULL, fwif_color_colorspaceyuv2rgbtransfer\n");
		}
	} else {
		VIPprintf("ERROR, table_index == NULL, fwif_color_colorspaceyuv2rgbtransfer !!\n");
	}

#if 0
#ifdef RUN_ON_TVBOX
	if ((scalerdisplay_get_display_tve_mode() >=  _MODE_480I) && (scalerdisplay_get_display_tve_mode() <=  _MODE_576P))
		table_index = tYUV2RGB_COEF_TVBOX_SD;
	else
		table_index = tYUV2RGB_COEF_TVBOX_HD;
#else
	table_index = tYUV2RGB_COEF_601_YCbCr_NOClampY;
#endif

	drvif_color_colorspaceyuv2rgbtransfer(display, table_index);

	if (table_index !=  NULL) {
		/*set using matrix to share memory*/
		if (VIP_system_info_structure_table !=  NULL) {
			memcpy((unsigned short *)(&VIP_system_info_structure_table->YUV2RGB_COEF_CSMatrix), table_index, sizeof(VIP_YUV2RGB_COEF_CSMatrix));
		} else {
			VIPprintf("VIP_system_info_structure_table = NULL, fwif_color_colorspaceyuv2rgbtransfer\n");
		}
	} else {
		VIPprintf("ERROR, table_index == NULL, fwif_color_colorspaceyuv2rgbtransfer !!\n");
	}
#endif
}
#endif

unsigned char fwif_color_BackLight_Remap(unsigned char nValue)
{
	unsigned char nBackLight_Value;
	UINT16 temp;
	nBackLight_Value = nValue;

	temp = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_BACKLIGHT,nBackLight_Value);
	if(temp > 255)
		nBackLight_Value = 255;
	else
		nBackLight_Value = (UINT8)(temp);
	return nBackLight_Value;
}

void fwif_color_Set_Local_Dimming_Full(unsigned char nValue)
{
	drvif_color_Set_Local_Dimming_Full(fwif_color_BackLight_Remap(nValue));
}
void fwif_color_Set_Local_Dimming_Off(unsigned char nValue)
{
	drvif_color_Set_Local_Dimming_Off(fwif_color_BackLight_Remap(nValue));
}
void fwif_color_Local_Dimming_Init(unsigned char nValue)
{
	drvif_color_Local_Dimming_Init(fwif_color_BackLight_Remap(nValue));
}
// =============

/*#ifdef LocalDimming*/

void fwif_color_set_LD_Enable(unsigned char src_idx, bool enable)
{
	drvif_color_set_LD_Enable(enable);
}


void fwif_color_set_LD_Global_Ctrl(unsigned char src_idx, unsigned char TableIdx)
{

	DRV_LD_Global_Ctrl data;
	extern DRV_Local_Dimming_Table Local_Dimming_Table[LD_Table_NUM];

	data.ld_comp_en = Local_Dimming_Table[TableIdx].LD_Global_Ctrl.ld_comp_en;
	data.ld_db_en = Local_Dimming_Table[TableIdx].LD_Global_Ctrl.ld_db_en;
	data.ld_db_apply = Local_Dimming_Table[TableIdx].LD_Global_Ctrl.ld_db_apply;
	data.ld_db_read_level = Local_Dimming_Table[TableIdx].LD_Global_Ctrl.ld_db_read_level;
	data.ld_3d_mode = Local_Dimming_Table[TableIdx].LD_Global_Ctrl.ld_3d_mode;
	data.ld_blk_hnum = Local_Dimming_Table[TableIdx].LD_Global_Ctrl.ld_blk_hnum;
	data.ld_blk_vnum = Local_Dimming_Table[TableIdx].LD_Global_Ctrl.ld_blk_vnum;
	data.ld_subblk_mode = Local_Dimming_Table[TableIdx].LD_Global_Ctrl.ld_subblk_mode;
	data.ld_gdim_mode = Local_Dimming_Table[TableIdx].LD_Global_Ctrl.ld_gdim_mode;
	data.ld_blk_type = Local_Dimming_Table[TableIdx].LD_Global_Ctrl.ld_blk_type;
	data.ld_hist_mode = Local_Dimming_Table[TableIdx].LD_Global_Ctrl.ld_hist_mode;
	data.ld_tenable = Local_Dimming_Table[TableIdx].LD_Global_Ctrl.ld_tenable;
	data.ld_valid = Local_Dimming_Table[TableIdx].LD_Global_Ctrl.ld_valid;
	data.ld_blk_hsize = Local_Dimming_Table[TableIdx].LD_Global_Ctrl.ld_blk_hsize;
	data.ld_blk_vsize = Local_Dimming_Table[TableIdx].LD_Global_Ctrl.ld_blk_vsize;

	drvif_color_set_LD_Global_Ctrl((DRV_LD_Global_Ctrl *) &data);
}

void fwif_color_set_LD_Backlight_Decision(unsigned char src_idx, unsigned char TableIdx)
{

	DRV_LD_Backlight_Decision data;
	extern DRV_Local_Dimming_Table Local_Dimming_Table[LD_Table_NUM];

	data.ld_maxgain = Local_Dimming_Table[TableIdx].LD_Backlight_Decision.ld_maxgain;
	data.ld_avegain = Local_Dimming_Table[TableIdx].LD_Backlight_Decision.ld_avegain;
	data.ld_histshiftbit = Local_Dimming_Table[TableIdx].LD_Backlight_Decision.ld_histshiftbit;
	data.ld_hist0gain = Local_Dimming_Table[TableIdx].LD_Backlight_Decision.ld_hist0gain;
	data.ld_hist1gain = Local_Dimming_Table[TableIdx].LD_Backlight_Decision.ld_hist1gain;
	data.ld_hist2gain = Local_Dimming_Table[TableIdx].LD_Backlight_Decision.ld_hist2gain;
	data.ld_hist3gain = Local_Dimming_Table[TableIdx].LD_Backlight_Decision.ld_hist3gain;
	data.ld_hist4gain = Local_Dimming_Table[TableIdx].LD_Backlight_Decision.ld_hist4gain;
	data.ld_hist5gain = Local_Dimming_Table[TableIdx].LD_Backlight_Decision.ld_hist5gain;
	data.ld_hist6gain = Local_Dimming_Table[TableIdx].LD_Backlight_Decision.ld_hist6gain;
	data.ld_hist7gain = Local_Dimming_Table[TableIdx].LD_Backlight_Decision.ld_hist7gain;

	drvif_color_set_LD_Backlight_Decision((DRV_LD_Backlight_Decision *) &data);
}

void fwif_color_set_LD_Spatial_Filter(unsigned char src_idx, unsigned char TableIdx)
{

	DRV_LD_Spatial_Filter data;
	extern DRV_Local_Dimming_Table Local_Dimming_Table[LD_Table_NUM];

	data.ld_spatialcoef0 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialcoef0;
	data.ld_spatialcoef1 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialcoef1;
	data.ld_spatialcoef2 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialcoef2;
	data.ld_spatialcoef3 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialcoef3;
	data.ld_spatialcoef4 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialcoef4;
	data.ld_spatialcoef5 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialcoef5;
	data.ld_spatialcoef6 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialcoef6;
	data.ld_spatialcoef7 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialcoef7;
	data.ld_spatialcoef8 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialcoef8;
	data.ld_spatialcoef9 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialcoef9;
	data.ld_spatialcoef10 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialcoef10;
	data.spatial_new_mode = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.spatial_new_mode;
	data.ld_spatialnewcoef00 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef00;
	data.ld_spatialnewcoef01 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef01;
	data.ld_spatialnewcoef02 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef02;
	data.ld_spatialnewcoef03 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef03;
	data.ld_spatialnewcoef04 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef04;
	data.ld_spatialnewcoef10 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef10;
	data.ld_spatialnewcoef11 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef11;
	data.ld_spatialnewcoef12 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef12;
	data.ld_spatialnewcoef13 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef13;
	data.ld_spatialnewcoef14 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef14;
	data.ld_spatialnewcoef20 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef20;
	data.ld_spatialnewcoef21 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef21;
	data.ld_spatialnewcoef22 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef22;
	data.ld_spatialnewcoef23 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef23;
	data.ld_spatialnewcoef24 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef24;
	data.ld_spatialnewcoef30 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef30;
	data.ld_spatialnewcoef31 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef31;
	data.ld_spatialnewcoef32 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef32;
	data.ld_spatialnewcoef33 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef33;
	data.ld_spatialnewcoef34 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef34;
	data.ld_spatialnewcoef05 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef05;
	data.ld_spatialnewcoef15 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef15;
	data.ld_spatialnewcoef25 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef25;
	data.ld_spatialnewcoef35 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef35;
	data.ld_spatialnewcoef40 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef40;
	data.ld_spatialnewcoef41 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef41;
	data.ld_spatialnewcoef50 = Local_Dimming_Table[TableIdx].LD_Spatial_Filter.ld_spatialnewcoef50;

	drvif_color_set_LD_Spatial_Filter((DRV_LD_Spatial_Filter *) &data);
}

void fwif_color_set_LD_Spatial_Remap(unsigned char src_idx, unsigned char TableIdx)
{

	DRV_LD_Spatial_Remap data;
	extern DRV_Local_Dimming_Table Local_Dimming_Table[LD_Table_NUM];

	data.ld_spatialremapen = Local_Dimming_Table[TableIdx].LD_Spatial_Remap.ld_spatialremapen;
	memcpy(&(data.ld_spatialremaptab), &(Local_Dimming_Table[TableIdx].LD_Spatial_Remap.ld_spatialremaptab),sizeof(unsigned short)*65);

	drvif_color_set_LD_Spatial_Remap((DRV_LD_Spatial_Remap *) &data);
}

void fwif_color_set_LD_Boost(unsigned char src_idx, unsigned char TableIdx)
{
	DRV_LD_Boost data;
	extern DRV_Local_Dimming_Table Local_Dimming_Table[LD_Table_NUM];

	data.ld_backlightboosten= Local_Dimming_Table[TableIdx].LD_Boost.ld_backlightboosten;
	memcpy(&(data.ld_boost_gain_lut), &(Local_Dimming_Table[TableIdx].LD_Boost.ld_boost_gain_lut),sizeof(unsigned short)*65);
	memcpy(&(data.ld_boost_curve_lut), &(Local_Dimming_Table[TableIdx].LD_Boost.ld_boost_curve_lut),sizeof(unsigned short)*65);
	drvif_color_set_LD_Boost((DRV_LD_Boost *) &data);
}


void fwif_color_set_LD_Temporal_Filter(unsigned char src_idx, unsigned char TableIdx)
{

	DRV_LD_Temporal_Filter data;
	extern DRV_Local_Dimming_Table Local_Dimming_Table[LD_Table_NUM];

	data.ld_tmp_pos0thd = Local_Dimming_Table[TableIdx].LD_Temporal_Filter.ld_tmp_pos0thd;
	data.ld_tmp_pos1thd = Local_Dimming_Table[TableIdx].LD_Temporal_Filter.ld_tmp_pos1thd;
	data.ld_tmp_posmingain = Local_Dimming_Table[TableIdx].LD_Temporal_Filter.ld_tmp_posmingain;
	data.ld_tmp_posmaxgain = Local_Dimming_Table[TableIdx].LD_Temporal_Filter.ld_tmp_posmaxgain;
	data.ld_tmp_neg0thd = Local_Dimming_Table[TableIdx].LD_Temporal_Filter.ld_tmp_neg0thd;
	data.ld_tmp_neg1thd = Local_Dimming_Table[TableIdx].LD_Temporal_Filter.ld_tmp_neg1thd;
	data.ld_tmp_negmingain = Local_Dimming_Table[TableIdx].LD_Temporal_Filter.ld_tmp_negmingain;
	data.ld_tmp_negmaxgain = Local_Dimming_Table[TableIdx].LD_Temporal_Filter.ld_tmp_negmaxgain;
	data.ld_tmp_maxdiff = Local_Dimming_Table[TableIdx].LD_Temporal_Filter.ld_tmp_maxdiff;
	data.ld_tmp_scenechangegain1 = Local_Dimming_Table[TableIdx].LD_Temporal_Filter.ld_tmp_scenechangegain1;

	drvif_color_set_LD_Temporal_Filter((DRV_LD_Temporal_Filter *) &data);
}

void fwif_color_set_LD_Backlight_Final_Decision(unsigned char src_idx, unsigned char TableIdx)
{

	DRV_LD_Backlight_Final_Decision data;
	extern DRV_Local_Dimming_Table Local_Dimming_Table[LD_Table_NUM];

	data.ld_blu_wr_num = Local_Dimming_Table[TableIdx].LD_Backlight_Final_Decision.ld_blu_wr_num;
	data.ld_blu_spitotal = Local_Dimming_Table[TableIdx].LD_Backlight_Final_Decision.ld_blu_spitotal;
	data.ld_blu_spien = Local_Dimming_Table[TableIdx].LD_Backlight_Final_Decision.ld_blu_spien;
	data.ld_blu_nodim = Local_Dimming_Table[TableIdx].LD_Backlight_Final_Decision.ld_blu_nodim;
	data.ld_blu_usergain = Local_Dimming_Table[TableIdx].LD_Backlight_Final_Decision.ld_blu_usergain;

	drvif_color_set_LD_Backlight_Final_Decision((DRV_LD_Backlight_Final_Decision *) &data);
}


void fwif_color_set_LD_Data_Compensation(unsigned char src_idx, unsigned char TableIdx)
{
	DRV_LD_Data_Compensation data;
	extern DRV_Local_Dimming_Table Local_Dimming_Table[LD_Table_NUM];

	data.ld_data_comp_mode = Local_Dimming_Table[TableIdx].LD_Data_Compensation.ld_data_comp_mode;
	data.ld_comp_satrange = Local_Dimming_Table[TableIdx].LD_Data_Compensation.ld_comp_satrange;
	data.ld_comp_softmax = Local_Dimming_Table[TableIdx].LD_Data_Compensation.ld_comp_softmax;
	data.ld_comp_minlimit = Local_Dimming_Table[TableIdx].LD_Data_Compensation.ld_comp_minlimit;
	data.ld_softcomp_gain = Local_Dimming_Table[TableIdx].LD_Data_Compensation.ld_softcomp_gain;
	data.ld_comp_maxmode = Local_Dimming_Table[TableIdx].LD_Data_Compensation.ld_comp_maxmode;
	data.ld_comp_inv_shift = Local_Dimming_Table[TableIdx].LD_Data_Compensation.ld_comp_inv_shift;
	data.ld_comp_inv_mode = Local_Dimming_Table[TableIdx].LD_Data_Compensation.ld_comp_inv_mode;
	data.ld_srcgainsel = Local_Dimming_Table[TableIdx].LD_Data_Compensation.ld_srcgainsel;

	drvif_color_set_LD_Data_Compensation((DRV_LD_Data_Compensation *) &data);
}

unsigned char g_bNotFirstRun_LD_Data_Compensation_NewMode_2DTable = 0;
void fwif_color_set_LD_Data_Compensation_NewMode_2DTable(unsigned char src_idx, unsigned char TableIdx)
{
	static DRV_LD_Data_Compensation_NewMode_2DTable data = {0};
	extern DRV_LD_Data_Compensation_NewMode_2DTable LD_Data_Compensation_NewMode_2DTable[LD_Table_NUM];

	if (g_bNotFirstRun_LD_Data_Compensation_NewMode_2DTable &&
		memcmp(&LD_Data_Compensation_NewMode_2DTable[TableIdx], &data, sizeof(DRV_LD_Data_Compensation_NewMode_2DTable)) == 0)
		return;

	memcpy(&(data.ld_comp_2Dtable), &(LD_Data_Compensation_NewMode_2DTable[TableIdx].ld_comp_2Dtable),sizeof(unsigned int)*17*17);
	drvif_color_set_LD_Data_Compensation_NewMode_2DTable((DRV_LD_Data_Compensation_NewMode_2DTable *) &data);

	g_bNotFirstRun_LD_Data_Compensation_NewMode_2DTable = 1;
}




void fwif_color_set_LD_Backlight_Profile_Interpolation(unsigned char src_idx, unsigned char TableIdx)
{

	DRV_LD_Backlight_Profile_Interpolation data;
	extern DRV_Local_Dimming_Table Local_Dimming_Table[LD_Table_NUM];

	data.ld_tab_hsize = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_tab_hsize;
	data.ld_hfactor = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_hfactor;
	data.ld_hinitphase3_right = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_hinitphase3_right;
	data.ld_hboundary = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_hboundary;
	data.ld_hinitphase1 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_hinitphase1;
	data.ld_hinitphase2 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_hinitphase2;
	data.ld_hinitphase3_left = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_hinitphase3_left;
	data.ld_tab_vsize = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_tab_vsize;
	data.ld_vfactor = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vfactor;
	data.ld_vinitphase3_right = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vinitphase3_right;
	data.ld_vboundary = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vboundary;
	data.ld_vinitphase1 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vinitphase1;
	data.ld_vinitphase2 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vinitphase2;
	data.ld_vinitphase3_left = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vinitphase3_left;
	data.ld_blight_update_en = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_blight_update_en;
	data.ld_blight_sw_mode = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_blight_sw_mode;
	data.ld_table_sw_mode = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_table_sw_mode;
	data.ld_htabsel_0 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_0;
	data.ld_htabsel_1 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_1;
	data.ld_htabsel_2 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_2;
	data.ld_htabsel_3 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_3;
	data.ld_htabsel_4 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_4;
	data.ld_htabsel_5 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_5;
	data.ld_htabsel_6 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_6;
	data.ld_htabsel_7 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_7;
	data.ld_htabsel_8 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_8;
	data.ld_htabsel_9 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_9;
	data.ld_htabsel_10 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_10;
	data.ld_htabsel_11 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_11;
	data.ld_htabsel_12 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_12;
	data.ld_htabsel_13 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_13;
	data.ld_htabsel_14 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_14;
	data.ld_htabsel_15 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_15;
	data.ld_htabsel_16 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_16;
	data.ld_htabsel_17 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_17;
	data.ld_htabsel_18 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_18;
	data.ld_htabsel_19 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_19;
	data.ld_htabsel_20 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_20;
	data.ld_htabsel_21 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_21;
	data.ld_htabsel_22 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_22;
	data.ld_htabsel_23 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_23;
	data.ld_htabsel_24 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_24;
	data.ld_htabsel_25 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_25;
	data.ld_htabsel_26 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_26;
	data.ld_htabsel_27 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_27;
	data.ld_htabsel_28 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_28;
	data.ld_htabsel_29 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_29;
	data.ld_htabsel_30 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_30;
	data.ld_htabsel_31 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_htabsel_31;
	data.ld_vtabsel_0 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_0;
	data.ld_vtabsel_1 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_1;
	data.ld_vtabsel_2 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_2;
	data.ld_vtabsel_3 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_3;
	data.ld_vtabsel_4 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_4;
	data.ld_vtabsel_5 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_5;
	data.ld_vtabsel_6 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_6;
	data.ld_vtabsel_7 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_7;
	data.ld_vtabsel_8 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_8;
	data.ld_vtabsel_9 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_9;
	data.ld_vtabsel_10 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_10;
	data.ld_vtabsel_11 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_11;
	data.ld_vtabsel_12 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_12;
	data.ld_vtabsel_13 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_13;
	data.ld_vtabsel_14 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_14;
	data.ld_vtabsel_15 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_15;
	data.ld_vtabsel_16 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_16;
	data.ld_vtabsel_17 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_17;
	data.ld_vtabsel_18 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_18;
	data.ld_vtabsel_19 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_19;
	data.ld_vtabsel_20 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_20;
	data.ld_vtabsel_21 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_21;
	data.ld_vtabsel_22 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_22;
	data.ld_vtabsel_23 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_23;
	data.ld_vtabsel_24 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_24;
	data.ld_vtabsel_25 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_25;
	data.ld_vtabsel_26 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_26;
	data.ld_vtabsel_27 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_27;
	data.ld_vtabsel_28 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_28;
	data.ld_vtabsel_29 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_29;
	data.ld_vtabsel_30 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_30;
	data.ld_vtabsel_31 = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation.ld_vtabsel_31;
	drvif_color_set_LD_Backlight_Profile_Interpolation((DRV_LD_Backlight_Profile_Interpolation *) &data);
}

void fwif_color_set_LD_BL_Profile_Interpolation_Table(unsigned char src_idx, unsigned char TableIdx)
{

	int i, j, k = 0;
	extern unsigned int LD_Backlight_Profile_Interpolation_table[LD_Table_NUM][BL_Profile_Table_NUM][BL_Profile_Table_ROW][BL_Profile_Table_COLUMN];
	static unsigned int data[BL_Profile_Table_NUM][BL_Profile_Table_ROW][BL_Profile_Table_COLUMN];

	for (i = 0; i < BL_Profile_Table_NUM; i++) {
		for (j = 0; j < BL_Profile_Table_ROW; j++) {
			for (k = 0; k < BL_Profile_Table_COLUMN; k++) {
				data[i][j][k] = LD_Backlight_Profile_Interpolation_table[TableIdx][i][j][k];
				VIPprintf(" %d\t", data[i][j][k]);
			}
			VIPprintf("\n");
		}
		VIPprintf("\n");
	}
	drvif_color_set_LD_BL_Profile_Interpolation_Table((unsigned int *)&data);

}



#if 0

void fwif_color_set_LD_Backlight_Profile_Interpolation_Table(unsigned char src_idx, unsigned char TableIdx)
{
	int data[6][4][20];
	int i, j, k = 0;

	for (i = 0; i < 6; i++) {
		for (j = 0; j < 4; j++) {
			for (k = 0; k < 20; k++) {
				data[i][j][k] = Local_Dimming_Table[TableIdx].LD_Backlight_Profile_Interpolation_table[i][j][k];
				VIPprintf(" %d\t", data[i][j][k]);
			}
			VIPprintf("\n");
		}
		VIPprintf("\n");
	}
	drvif_color_set_LD_Backlight_Profile_Interpolation_Table(data);
}
#endif

void fwif_color_set_LD_Demo_Window(unsigned char src_idx, unsigned char TableIdx)
{

	DRV_LD_Demo_Window data;
	extern DRV_Local_Dimming_Table Local_Dimming_Table[LD_Table_NUM];

	data.ld_demo_en = Local_Dimming_Table[TableIdx].LD_Demo_Window.ld_demo_en;
	data.ld_demo_mode = Local_Dimming_Table[TableIdx].LD_Demo_Window.ld_demo_mode;
	data.ld_demo_top = Local_Dimming_Table[TableIdx].LD_Demo_Window.ld_demo_top;
	data.ld_demo_bottom = Local_Dimming_Table[TableIdx].LD_Demo_Window.ld_demo_bottom;
	data.ld_demo_left = Local_Dimming_Table[TableIdx].LD_Demo_Window.ld_demo_left;
	data.ld_demo_right = Local_Dimming_Table[TableIdx].LD_Demo_Window.ld_demo_right;

	drvif_color_set_LD_Demo_Window((DRV_LD_Demo_Window *) &data);
}


void fwif_color_set_LC_Enable(unsigned char src_idx,bool enable)
{
	drvif_color_set_LC_Enable(enable);
	drvif_color_check_LC_bypass_case();
}


void fwif_color_set_LC_Global_Ctrl(unsigned char src_idx,unsigned char TableIdx)
{
	DRV_LC_Global_Ctrl data;

	if (gVip_Table == NULL)
		return;

	if (TableIdx >= LC_Table_NUM)
		TableIdx = LC_Table_NUM-1;

	data = gVip_Table->Local_Contrast_Table[TableIdx].LC_Global_Ctrl;
	drvif_color_set_LC_Global_Ctrl(&data);
}

void fwif_color_set_LC_Backlight_Decision(unsigned char src_idx,unsigned char TableIdx)
{
	DRV_LC_Backlight_Decision *pData = NULL;

	if (gVip_Table == NULL)
		return;

	if (TableIdx >= LC_Table_NUM)
		TableIdx = LC_Table_NUM-1;
	//pData = &Local_Contrast_Table[TableIdx].LC_Backlight_Decision;
	pData = &gVip_Table->Local_Contrast_Table[TableIdx].LC_Backlight_Decision;

	drvif_color_set_LC_Backlight_Decision(pData);
}

void fwif_color_set_LC_Spatial_Filter(unsigned char src_idx,unsigned char TableIdx)
{
	DRV_LC_Spatial_Filter *pData = NULL;

	if (gVip_Table == NULL)
		return;

	if (TableIdx >= LC_Table_NUM)
		TableIdx = LC_Table_NUM-1;
	//pData = &Local_Contrast_Table[TableIdx].LC_Spatial_Filter;
	pData = &gVip_Table->Local_Contrast_Table[TableIdx].LC_Spatial_Filter;

	drvif_color_set_LC_Spatial_Filter(pData);
}

void fwif_color_set_LC_Temporal_Filter(unsigned char src_idx,unsigned char TableIdx)
{
	DRV_LC_Temporal_Filter *pData = NULL;

	if (gVip_Table == NULL)
		return;

	if (TableIdx >= LC_Table_NUM)
		TableIdx = LC_Table_NUM-1;
	//pData = &Local_Contrast_Table[TableIdx].LC_Temporal_Filter;
	pData = &gVip_Table->Local_Contrast_Table[TableIdx].LC_Temporal_Filter;

	drvif_color_set_LC_Temporal_Filter(pData);
}

void fwif_color_set_LC_Backlight_Profile_Interpolation(unsigned char src_idx,unsigned char TableIdx)
{
	//extern unsigned int LC_Backlight_Profile_Interpolation_table[LC_Table_NUM][2][125];
	DRV_LC_Backlight_Profile_Interpolation *pData = NULL;

	if (gVip_Table == NULL)
		return;

	if (TableIdx >= LC_Table_NUM)
		TableIdx = 0;
	//data = Local_Contrast_Table[TableIdx].LC_Backlight_Profile_Interpolation;
	pData = &gVip_Table->Local_Contrast_Table[TableIdx].LC_Backlight_Profile_Interpolation;
	drvif_color_set_LC_Backlight_Profile_Interpolation(pData);

	drvif_color_set_LC_BL_Profile_Interpolation_Table(gVip_Table->LC_Backlight_Profile_Interpolation_table[TableIdx][0],0,(pData->lc_tab_hsize+1)*5);
	drvif_color_set_LC_BL_Profile_Interpolation_Table(gVip_Table->LC_Backlight_Profile_Interpolation_table[TableIdx][1],1,(pData->lc_tab_vsize+1)*5);
}

void fwif_color_set_LC_Backlight_Profile_Interpolation_only_table(unsigned char src_idx,unsigned char TableIdx, unsigned int (*pTable)[125])
{
	DRV_LC_Backlight_Profile_Interpolation *pData = NULL;

	if (gVip_Table == NULL)
		return;

	if (TableIdx >= LC_Table_NUM)
		TableIdx = 0;
	//data = Local_Contrast_Table[TableIdx].LC_Backlight_Profile_Interpolation;
	pData = &gVip_Table->Local_Contrast_Table[TableIdx].LC_Backlight_Profile_Interpolation;
	drvif_color_set_LC_Backlight_Profile_Interpolation(pData);

	drvif_color_set_LC_BL_Profile_Interpolation_Table(pTable[0],0,(pData->lc_tab_hsize+1)*5);
	drvif_color_set_LC_BL_Profile_Interpolation_Table(pTable[1],1,(pData->lc_tab_vsize+1)*5);
}


void fwif_color_set_LC_Demo_Window(unsigned char src_idx,unsigned char TableIdx)
{
	DRV_LC_Demo_Window *pData = NULL;

	if (gVip_Table == NULL)
		return;

	if (TableIdx >= LC_Table_NUM)
		TableIdx = LC_Table_NUM-1;
	//pData = &Local_Contrast_Table[TableIdx].LC_Demo_Window;
	pData = &gVip_Table->Local_Contrast_Table[TableIdx].LC_Demo_Window;

	drvif_color_set_LC_Demo_Window(pData);
}


void fwif_color_set_LC_ToneMapping_SetGrid0(unsigned char src_idx,unsigned char TableIdx)
{
	DRV_LC_ToneMapping_Grid0 *pData = NULL;

	if (gVip_Table == NULL)
		return;

	if (TableIdx >= LC_Table_NUM)
		TableIdx = LC_Table_NUM-1;
	//pData = &Local_Contrast_Table[TableIdx].LC_ToneMapping_Grid0;
	pData = &gVip_Table->Local_Contrast_Table[TableIdx].LC_ToneMapping_Grid0;

	drvif_color_set_LC_ToneMapping_Grid0(pData);
}

void fwif_color_set_LC_ToneMapping_SetGrid2(unsigned char src_idx,unsigned char TableIdx)
{
	DRV_LC_ToneMapping_Grid2 *pData = NULL;

	if (gVip_Table == NULL)
		return;

	if (TableIdx >= LC_Table_NUM)
		TableIdx = LC_Table_NUM-1;
	//pData = &Local_Contrast_Table[TableIdx].LC_ToneMapping_Grid2;
	pData = &gVip_Table->Local_Contrast_Table[TableIdx].LC_ToneMapping_Grid2;

	drvif_color_set_LC_ToneMapping_Grid2(pData);
}

void fwif_color_set_LC_ToneMapping_CurveSelect(unsigned char src_idx,unsigned char TableIdx)
{
	DRV_LC_ToneMapping_CurveSelect *pData = NULL;

	if (gVip_Table == NULL)
		return;

	if (TableIdx >= LC_Table_NUM)
		TableIdx = LC_Table_NUM-1;
	//pData = &Local_Contrast_Table[TableIdx].LC_ToneMapping_CurveSelect;
	pData = &gVip_Table->Local_Contrast_Table[TableIdx].LC_ToneMapping_CurveSelect;

	drvif_color_set_LC_ToneMapping_CurveSelect(pData);
}

void fwif_color_set_LC_ToneMapping_Blend(unsigned char src_idx,unsigned char TableIdx)
{
	DRV_LC_ToneMapping_Blend *pData = NULL;

	if (TableIdx >= LC_Table_NUM)
		TableIdx = LC_Table_NUM-1;
	//pData = &Local_Contrast_Table[TableIdx].LC_ToneMapping_Blend;
	pData = &gVip_Table->Local_Contrast_Table[TableIdx].LC_ToneMapping_Blend;

	drvif_color_set_LC_ToneMapping_Blend(pData);
}

void fwif_color_set_LC_Shpnr_Gain1st(unsigned char src_idx,unsigned char TableIdx)
{
	DRV_LC_Shpnr_Gain1st *pData = NULL;
	DRV_LC_Shpnr_Gain1st LC_Shpnr_Gain1st = {0};
	char div;
	char DEM_lv;
	
	if (gVip_Table == NULL)
		return;

	if (TableIdx >= LC_Table_NUM)
		TableIdx = LC_Table_NUM-1;
	//pData = &Local_Contrast_Table[TableIdx].LC_Shpnr_Gain1st;

	if (Scaler_APDEM_Arg_Access(DEM_ARG_LC_level, 0, 0) != 0xFF) {

		memcpy(&LC_Shpnr_Gain1st, &gVip_Table->Local_Contrast_Table[TableIdx].LC_Shpnr_Gain1st, sizeof(DRV_LC_Shpnr_Gain1st));
		div = (char)LC_Shpnr_Gain1st.lc_gain_by_yin_divisor;
		DEM_lv = (char)Scaler_APDEM_Arg_Access(DEM_ARG_LC_level, 0, 0);
		
		LC_Shpnr_Gain1st.lc_gain_by_yin_divisor = (unsigned char)(((div+2-DEM_lv)<0)?(0):(((div+2-DEM_lv)>7)?(7):(div+2-DEM_lv)));
		
		drvif_color_set_LC_Shpnr_Gain1st(&LC_Shpnr_Gain1st);
	} else {
		pData = &gVip_Table->Local_Contrast_Table[TableIdx].LC_Shpnr_Gain1st;

		drvif_color_set_LC_Shpnr_Gain1st(pData);
	}
}

void fwif_color_set_LC_Shpnr_Gain2nd(unsigned char src_idx,unsigned char TableIdx)
{
	DRV_LC_Shpnr_Gain2nd *pData = NULL;

	if (gVip_Table == NULL)
		return;

	if (TableIdx >= LC_Table_NUM)
		TableIdx = LC_Table_NUM-1;
	//pData = &Local_Contrast_Table[TableIdx].LC_Shpnr_Gain2nd;
	pData = &gVip_Table->Local_Contrast_Table[TableIdx].LC_Shpnr_Gain2nd;

	drvif_color_set_LC_Shpnr_Gain2nd(pData);
}

void fwif_color_set_LC_Diff_Ctrl0(unsigned char src_idx,unsigned char TableIdx)
{
	DRV_LC_Diff_Ctrl0 *pData = NULL;

	if (gVip_Table == NULL)
		return;

	if (TableIdx >= LC_Table_NUM)
		TableIdx = LC_Table_NUM-1;
	//pData = &Local_Contrast_Table[TableIdx].LC_Diff_Ctrl0;
	pData = &gVip_Table->Local_Contrast_Table[TableIdx].LC_Diff_Ctrl0;

	drvif_color_set_LC_Diff_Ctrl0(pData);
}

//juwen, add LC : tone mapping curve
void fwif_color_set_LC_ToneMappingSlopePoint(unsigned char src_idx,unsigned char TableIdx)
{
	//extern unsigned int LC_ToneMappingSlopePoint_Table[LC_Table_ToneM_NUM][LC_Curve_ToneM_PointSlope] ;

	if (gVip_Table == NULL)
		return;

	if(TableIdx>=LC_Table_ToneM_NUM)
		TableIdx=LC_Table_ToneM_NUM-1;

	drvif_color_set_LC_ToneMappingSlopePoint(gVip_Table->LC_ToneMappingSlopePoint_Table[TableIdx]);
}

void fwif_color_set_LC_DebugMode(unsigned char src_idx, unsigned char DebugMode)
{
	unsigned char mode;

	if(DebugMode > 3)
		mode = 0;
	else
		mode  = DebugMode;

	drvif_color_set_LC_DebugMode(mode);
}

char fwif_color_get_LC_Enable(void)
{
	return drvif_color_get_LC_Enable();
}

char fwif_color_get_LC_DebugMode(void)
{
	return drvif_color_get_LC_DebugMode();
}

void fwif_color_set_LC_Global_Region_Num_Size(unsigned char TableIdx, unsigned char TotalBlkNum)
{
	unsigned char sizeValid = 0;
	unsigned char nBlkHNum = 0, nBlkVNum = 0;
	unsigned char tab_hsize = 0, tab_vsize = 0;
	unsigned int  nImageHsize = 0, nImageVsize = 0;
	VIP_DRV_Local_Contrast_Region_Num_Size LC_Region_Num_Size;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = NULL;
	_system_setting_info *VIP_system_info_structure_table = NULL;

	// size information
	ppoverlay_main_active_h_start_end_RBUS ppoverlay_main_active_h_start_end_reg;
	ppoverlay_main_active_v_start_end_RBUS ppoverlay_main_active_v_start_end_reg;

	if (gVip_Table == NULL)
		return;

	ppoverlay_main_active_h_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_Active_H_Start_End_reg);
	ppoverlay_main_active_v_start_end_reg.regValue = rtd_inl(PPOVERLAY_MAIN_Active_V_Start_End_reg);

	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if(VIP_RPC_system_info_structure_table == NULL || VIP_system_info_structure_table == NULL)
	{
		VIPprintf("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
		return;
	}

	if( TableIdx >= LC_Table_NUM)
		TableIdx = LC_Table_NUM-1;

	switch(TotalBlkNum)
	{
		case LC_BLOCK_16x16:
			nBlkHNum = 16;
			nBlkVNum = 16;
			break;
		case LC_BLOCK_32x16:
			nBlkHNum = 32;
			nBlkVNum = 16;
			break;
		case LC_BLOCK_48x27:
			nBlkHNum = 48;
			nBlkVNum = 27;
			break;
		case LC_BLOCK_60x34:
			nBlkHNum = 60;
			nBlkVNum = 34;
			break;
		default:
			rtd_pr_vpq_emerg("[LC]block mode error, set mode=%d\n", TotalBlkNum);
			nBlkHNum = 16;
			nBlkVNum = 16;
			break;
	}

	nBlkHNum = nBlkHNum - 1;
	nBlkVNum = nBlkVNum - 1;

	tab_hsize = gVip_Table->Local_Contrast_Table[TableIdx].LC_Backlight_Profile_Interpolation.lc_tab_hsize;
	tab_vsize = gVip_Table->Local_Contrast_Table[TableIdx].LC_Backlight_Profile_Interpolation.lc_tab_vsize;

	nImageHsize = ppoverlay_main_active_h_start_end_reg.mh_act_end - ppoverlay_main_active_h_start_end_reg.mh_act_sta;
	nImageVsize = ppoverlay_main_active_v_start_end_reg.mv_act_end - ppoverlay_main_active_v_start_end_reg.mv_act_sta;

	rtd_pr_vpq_info("[%s] GetSize %d %d\n", __FUNCTION__, nImageHsize, nImageVsize);
	rtd_pr_vpq_info("[%s] Info2VideoFW Tbl=%d TabH/V=%d/%d NumH/V=%d/%d\n",
		__FUNCTION__, TableIdx, tab_hsize, tab_vsize, nBlkHNum, nBlkVNum);

	/* recording info for videoFW */
	VIP_RPC_system_info_structure_table->Local_Contrast_Ctrl.LC_Table_Idx = TableIdx;
	VIP_RPC_system_info_structure_table->Local_Contrast_Ctrl.BL_Profile_Interpolation_lc_tab_hsize = tab_hsize;
	VIP_RPC_system_info_structure_table->Local_Contrast_Ctrl.BL_Profile_Interpolation_lc_tab_vsize = tab_vsize;
	VIP_RPC_system_info_structure_table->Local_Contrast_Ctrl.Local_Contrast_Region_Num_Size.nBlk_Hnum = nBlkHNum;
	VIP_RPC_system_info_structure_table->Local_Contrast_Ctrl.Local_Contrast_Region_Num_Size.nBlk_Vnum = nBlkVNum;

	sizeValid = fwif_color_Cal_LC_Global_Region_Num_Size(&LC_Region_Num_Size, nImageHsize, nImageVsize, tab_hsize, tab_vsize, nBlkHNum, nBlkVNum);
	drvif_color_set_LC_size_support(sizeValid);

	rtd_pr_vpq_info("[%s][H] Num=%d, Size=%d, Tab=%d, Factor=%d\n",
		__FUNCTION__, LC_Region_Num_Size.nBlk_Hnum, LC_Region_Num_Size.nBlk_Hsize, tab_hsize, LC_Region_Num_Size.lc_hfactor);
	rtd_pr_vpq_info("[%s][V] Num=%d, Size=%d, Tab=%d, Factor=%d\n",
		__FUNCTION__, LC_Region_Num_Size.nBlk_Vnum, LC_Region_Num_Size.nBlk_Vsize, tab_vsize, LC_Region_Num_Size.lc_vfactor);

	drvif_color_set_HV_Num_Fac_Size(&LC_Region_Num_Size);

}


char fwif_color_LC_check_Hblk_size(unsigned short nRegionHSize)
{
	lc_lc_global_ctrl2_RBUS blu_lc_global_ctrl2;//jw, 171106, add size constrain
	char Ret = 1;

	blu_lc_global_ctrl2.regValue = rtd_inl(LC_LC_Global_Ctrl2_reg);//jw, 171106, add size constrain

	// region size should be even
	if((nRegionHSize%2)!= 0)
	{
		Ret = 0;
	}

	// region size check according to hist mode
	switch (blu_lc_global_ctrl2.lc_hist_mode)
	{
		case 0:
			Ret = 1;
			break;
		case 1:
			if(nRegionHSize%2 != 0)
			{
	          	Ret = 0;
			}
			break;
		case 2:
			if(nRegionHSize%4 != 0)
			{
	          	Ret = 0;
			}
			break;
		default:
			if(nRegionHSize <= 128)
			{
	          	Ret = 0;
			}
			break;
	}
	return Ret;
}

unsigned char fwif_color_Cal_LC_Global_Region_Num_Size(
	VIP_DRV_Local_Contrast_Region_Num_Size *LC_Region_Num_Size,
	unsigned short nImageHsize, unsigned short nImageVsize,
	unsigned char tab_hsize, unsigned char tab_vsize,
	unsigned char nBlkHNum, unsigned char nBlkVNum
)
{
	unsigned short nRegionHSize = 0;
	unsigned short nRegionVSize = 0;
	unsigned int h_factor = 0;
	unsigned int v_factor = 0;
	unsigned char ValidHBlk = 0;
	unsigned char ValidVBlk = 0;
	unsigned int i = 0;
	const unsigned int PxlMode = 1;

	// assumption
	// sub blk mode = 0, hist mode = 2, reg_htab = 4, reg_vtab = 4

	// Constrains
	// max block num = 48x27, min block num = 4x3
	// min hor block num = 1px : 24, 2px : 48, 4px : 96
	// H factor < (1048576/2/PxlMode)
	// H/V block size x H/V block num == ImageSize
	// H block size should be even
	// H block size should be ratio of hist mode
	// HTab <= ImageH/(HBlkNum*5*pxlMode)

	if(nImageHsize == 0 || nImageVsize == 0)
	{
		nBlkHNum = 0;
		nRegionHSize = 0;
		h_factor = 0;

		nBlkVNum = 0;
		nRegionVSize = 0;
		v_factor = 0;

		return 0;
	}
	else
	{
		// Horizontal information, try to find max suitable block nums
		for( i=48; i>=4; i-- )
		{
			nBlkHNum = i-1;
			nRegionHSize = (nImageHsize%i==0)? (nImageHsize/i) : (nImageHsize/i)+1;
			h_factor = (1048576*i*(tab_hsize+1))/nImageHsize;

			if( i<=(nBlkHNum+1) && ValidHBlk==0 )
			{
				// constrains
				if( nRegionHSize < (24*PxlMode) )
					continue;
				if( h_factor >= (1048576/(5*PxlMode)) )
					continue;
				if( (i*nRegionHSize)!=nImageHsize )
					continue;
				if( nRegionHSize%2 == 1 )
					continue;
				if( fwif_color_LC_check_Hblk_size(nRegionHSize) == 0 )
					continue;
				if( tab_hsize > ((nImageHsize/(i*5*PxlMode))-1) )
					continue;

				ValidHBlk = 1;
				break;
			}
		}

		// Vertical information, try to find max suitable block nums
		for( i=27; i>=3; i--)
		{
			nBlkVNum = i-1;
			nRegionVSize = (nImageVsize%i)==0? (nImageVsize/i) : (nImageVsize/i)+1;
			v_factor = (1048576 *i*(tab_vsize+1))/nImageVsize;

			if( i<=(nBlkVNum+1) && ValidVBlk==0 )
			{
				// constrains
				if( v_factor >= 1048576 )
					continue;
				if( (i*nRegionVSize)!=nImageVsize )
					continue;
				if( nRegionVSize < 1 )
					continue;

				ValidVBlk = 1;
				break;
			}
		}
	}

	rtd_pr_vpq_info("[%s][%d][H] Num = %d, Tab = %d, Factor = %d, RgnSize = %d\n",
		__FUNCTION__, ValidHBlk, nBlkHNum, tab_hsize, h_factor, nRegionHSize);
	rtd_pr_vpq_info("[%s][%d][V] Num = %d, Tab = %d, Factor = %d, RgnSize = %d\n",
		__FUNCTION__, ValidVBlk, nBlkVNum, tab_vsize, v_factor, nRegionVSize);

	LC_Region_Num_Size->nBlk_Hnum = nBlkHNum;
	LC_Region_Num_Size->nBlk_Hsize = nRegionHSize;
	LC_Region_Num_Size->nBlk_Vnum = nBlkVNum;
	LC_Region_Num_Size->nBlk_Vsize = nRegionVSize;
	LC_Region_Num_Size->lc_hfactor = h_factor;
	LC_Region_Num_Size->lc_vfactor = v_factor;

	return (ValidHBlk==1 && ValidVBlk==1)? 1 : 0;
}

unsigned char fwif_color_get_LC_blk_hnum(void)
{
	return drvif_color_get_LC_blk_hnum();
}

unsigned char fwif_color_get_LC_blk_vnum(void)
{
	return drvif_color_get_LC_blk_vnum();
}

unsigned char fwif_color_get_LD_Enable(void)
{
	return drvif_color_get_LD_Enable();
}

unsigned int  fwif_color_get_get_LC_APL(unsigned int *ptr_out, unsigned int bufferSz)
{
	if(!ptr_out)
		return FALSE;

	 drvif_color_get_LC_APL(ptr_out, bufferSz);
	 return TRUE;
}

unsigned int  fwif_color_get_get_LC_Histogram(unsigned int *plc_out)
{
	if(!plc_out)
		return FALSE;

	 drvif_color_get_LC_Histogram(plc_out);

	 return TRUE;
}

unsigned int fwif_color_di_pow_Cal(unsigned int x, unsigned int p)
{
	int i=0;
	unsigned int r = 1;

	  if(p == 0) return 1;
	  if(x == 0) return 0;


	for(i=0;i<p;i++)
	{
		r*=x;
	}
	return r;
}

void fwif_set_pq_dir_path(char *path, char *patch_path)
{
	path = "system\\project\\TvServer\\Custom\\Vip_Sirius ";
	patch_path = "system\\project\\TvServer\\Custom\\Vip_Sirius ";
	/*VIPprintf("\n fwif_set_pq_dir_path = %s  \n", path);*/
}
void fwif_get_average_luma(unsigned char *val)
{
	*val = fwif_color_Debug_Get_HIST_MEAN_VALUE();
	/*VIPprintf("\n fwif_get_average_luma = %d  \n", *val);*/
}

unsigned char fwif_get_Y_Total_Hist_Cnt_distribution(unsigned char MODE,unsigned char size, unsigned int *Y_Hist_distr_cnt)
{
	UINT8 i = 0;
	unsigned int Y_Hist_Cnt[COLOR_HISTOGRAM_LEVEL];
	unsigned int luma_distribution[COLOR_HISTOGRAM_LEVEL];
#if 0
	unsigned int Pixel_Total_Cnt = 0, Hist_Wid = 0, Hist_Len = 0;
	unsigned short Ratio_check = 0;
	//histogram_ich1_hist_in_range_hor_RBUS ich1_hist_in_range_hor_Reg;
	//histogram_ich1_hist_in_range_ver_RBUS ich1_hist_in_range_ver_Reg;
#endif
	_clues *smartPic_clue = NULL;


	if (Y_Hist_distr_cnt == NULL) {
		return FALSE;
	}

	if (MODE) {
#if 1
		if ( (size > COLOR_HISTOGRAM_LEVEL)||((COLOR_HISTOGRAM_LEVEL%size)!=0)) {
			return FALSE;
		} else {
			fwif_get_luma_level_distribution(&luma_distribution[0], size);

			for (i = 0; i < size; i++) {
				Y_Hist_distr_cnt[i] = luma_distribution[i];
			}

			return TRUE;
		}
#else
		fwif_get_luma_level_distribution(&luma_distribution[0], size);

		ich1_hist_in_range_hor_Reg.regValue = IoReg_Read32(HISTOGRAM_ICH1_Hist_IN_Range_Hor_reg);
		ich1_hist_in_range_ver_Reg.regValue = IoReg_Read32(HISTOGRAM_ICH1_Hist_IN_Range_Ver_reg);
		Hist_Wid = ich1_hist_in_range_hor_Reg.ch1_his_horend - ich1_hist_in_range_hor_Reg.ch1_his_horstart;
		Hist_Len = ich1_hist_in_range_ver_Reg.ch1_his_verend - ich1_hist_in_range_ver_Reg.ch1_his_verstart;
		Pixel_Total_Cnt = Hist_Wid*Hist_Len;

		for (i = 0; i < COLOR_HISTOGRAM_LEVEL; i++) {
			Ratio_check += luma_distribution[i];
			Y_Hist_Cnt[i] = (unsigned int)(luma_distribution[i] * Pixel_Total_Cnt) / 100;
			Y_Hist_distr_cnt[i] = Y_Hist_Cnt[i];
		}

		if (Ratio_check <= 99) {
			return FALSE;
		} else {
			return TRUE;
		}
#endif
	} else {

		smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);

		if (smartPic_clue == NULL) {
			return FALSE;
		}

		fwif_color_ChangeUINT32Endian_Copy(&smartPic_clue->Y_Main_Hist[0], COLOR_HISTOGRAM_LEVEL, &Y_Hist_Cnt[0], 0);

		for (i = 0; i < COLOR_HISTOGRAM_LEVEL; i++) {
			Y_Hist_distr_cnt[i] = Y_Hist_Cnt[i];
		}

		return TRUE;

	}

}


void fwif_get_luma_level_distribution(unsigned int *luma_distribution, unsigned char size)
{
	unsigned char nTotalSize = COLOR_HISTOGRAM_LEVEL;
	unsigned char nSegment = nTotalSize / size;
	unsigned char nIndex = 0, nSegIndx = 0;
	unsigned short nPoint = 1;
	unsigned int nSumPoint = 0;
	_clues *smartPic_clue;
	unsigned int pwCopyTemp = 0;
	smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);

	if (smartPic_clue == NULL) {
		return;
	}

	for (nIndex = 0; nIndex < size; nIndex++) {
		luma_distribution[nIndex] = 0;
	}
	for (nIndex = 0; nIndex < size; nIndex++) {
		nSumPoint = 0;
		for (nSegIndx = 0; nSegIndx < nSegment; nSegIndx++) {
			pwCopyTemp = 0;
			if (nIndex * nSegment + nSegIndx < nTotalSize) {
				fwif_color_ChangeUINT32Endian_Copy(&smartPic_clue->Y_Main_Hist[nIndex * nSegment + nSegIndx], 1, &pwCopyTemp, 0);
				nSumPoint +=  pwCopyTemp;
				/*VIPprintf("\n Y_Main_Hist_Ratio[%d] = %d  ", nIndex*nSegment+nSegIndx, pwCopyTemp);*/
			}
		}
		luma_distribution[nIndex]  = (nSumPoint*nPoint);
	}
	#if 0
	for (nIndex = 0; nIndex < size; nIndex++) {
		VIPprintf("\n luma_distribution[%d] = %d  ", nIndex, luma_distribution[nIndex]);
	}
	#endif
}

int fwif_color_get_rgb_pixel_info(RGB_pixel_info *data)
{
	//return drvif_color_get_rgb_pixel_info(data);//removed from merlin3, by caroline,20170606
	return 0;
}

short CAdjustCosine(short fDegree)
{
	/*Lewis, the curve value table have multiplied by 1024*/
	unsigned short fCosCurveAngle[14] = {   0,    6,   12,  18,  24,  30,  37,  44,  52,  60,  68,  78, 90, 91};
	unsigned short fCosCurveValue[14] = {1024, 1018, 1002, 974, 935, 887, 818, 737, 630, 512, 384, 213,  0,  0};
	unsigned char  i, cQuadrant;
	signed short  fDegreeTemp, fDegreeSpan, fCosValueSpan, fCosValue;

	if (fDegree < 0)
		fDegree +=  360;

	fDegree = fDegree%360;

	/*locate degree on which quadrant*/
	cQuadrant = (unsigned char)(fDegree / 90);
	cQuadrant++;
	if (cQuadrant == 1) {/*first quadrant*/
		//fDegree = fDegree;
	} else if (cQuadrant == 2) /*second quadrant*/
		fDegree = 180 - fDegree;
	else if (cQuadrant == 3) /*third quadrant*/
		fDegree = fDegree - 180;
	else if (cQuadrant == 4) /*forth quadrant*/
		fDegree = 360 - fDegree;
	/*locate degree on which quadrant*/

	if (fDegree <= fCosCurveAngle[0]) {
		fCosValue = fCosCurveValue[0];
	} else {
		for (i = 1; i < 13; i++)
			if (fCosCurveAngle[i] >= fDegree)
				break;

		i--;
		fDegreeTemp = fDegree - fCosCurveAngle[i];
		fDegreeSpan = fCosCurveAngle[i + 1] - fCosCurveAngle[i];
		fCosValueSpan = fCosCurveValue[i + 1] - fCosCurveValue[i];
		fCosValue = fCosCurveValue[i]+fDegreeTemp*fCosValueSpan/fDegreeSpan;
	}

	if (cQuadrant == 2 || cQuadrant == 3)
		fCosValue = fCosValue*(-1);

	return fCosValue;
}

short CAdjustSine(short fDegree)
{
	/*Lewis, this table have multiplied by 1024*/
	unsigned short fSinCurveAngle[14] = {0,   6,  12,  18,  24,  30,  37,  44,  52,  60,  68,  78,    90,   91};
	unsigned short fSinCurveValue[14] = {0, 107, 213, 316, 416, 512, 616, 711, 807, 887, 949, 1002, 1024, 1024};
	unsigned char  i, cQuadrant;
	signed short  fDegreeTemp, fDegreeSpan, fSinValueSpan, fSinValue;

	if (fDegree < 0)
		fDegree +=  360;

	fDegree = fDegree%360;

	/*locate degree on which quadrant*/
	cQuadrant = (unsigned char)(fDegree / 90);
	cQuadrant++;
	if (cQuadrant == 1) {/*first quadrant*/
		//fDegree = fDegree;
	} else if (cQuadrant == 2) /*second quadrant*/
		fDegree = 180 - fDegree;
	else if (cQuadrant == 3) /*third quadrant*/
		fDegree = fDegree - 180;
	else if (cQuadrant == 4) /*forth quadrant*/
		fDegree = 360  - fDegree;
	/*locate degree on which quadrant*/

	if (fDegree <= fSinCurveAngle[0]) {
		fSinValue = fSinCurveValue[0];
	} else {
		for (i = 1; i < 13; i++)
			if (fSinCurveAngle[i] >= fDegree)
				break;
		i--;
		fDegreeTemp = fDegree - fSinCurveAngle[i];
		fDegreeSpan = fSinCurveAngle[i + 1] - fSinCurveAngle[i];
		fSinValueSpan = fSinCurveValue[i + 1] - fSinCurveValue[i];
		fSinValue = fSinCurveValue[i]+fDegreeTemp*fSinValueSpan/fDegreeSpan;
	}

	if (cQuadrant == 3 || cQuadrant == 4)
		fSinValue = fSinValue * (-1);

	return fSinValue;
}

void CAdjustMatrixMultiply(short (*arrayA)[3], short (*arrayB)[3], short (*arrayC)[3])
{
	unsigned char i, j;
	signed int Temp[3][3];

	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
#ifdef CONFIG_sRGB_User_Hue_Mapping

			Temp[i][j] = ((signed int)(arrayA[i][0] * arrayB[0][j]))
						+ ((signed int)(arrayA[i][1] * arrayB[1][j]))
						+ ((signed int)(arrayA[i][2] * arrayB[2][j]));


			arrayC[i][j] = (signed short)(Temp[i][j] >> 10);
#else
			Temp[i][j] = ((signed int)(arrayA[i][0] * arrayB[0][j]))
						+ ((signed int)(arrayA[i][1] * arrayB[1][j]))
						+ ((signed int)(arrayA[i][2] * arrayB[2][j]));

			arrayC[i][j] = (signed short)(Temp[i][j] >> 14);
#endif

		}
	}
}

short abs_value(short original_value)
{
	if (original_value < 0)
		original_value = -original_value;
	return original_value;
}

/*Elsie 20140825: OD middleware*/
void fwif_color_set_od(unsigned char bOD)
{
#ifndef BUILD_QUICK_SHOW

	if (!bODInited)
		return;

	drvif_color_od(bOD);
#endif	
}

/*
tablemode
	-1: inverse
	0: target, can't control gain level (gain level is ignored)
	1: delta
*/
void fwif_color_set_od_table(char tablemode, unsigned char level)
 {
 #ifndef BUILD_QUICK_SHOW

 	extern DRV_od_table_t OD_table;
	extern unsigned char bODInited;

	if (!bODInited) {
		if (!Scaler_Init_OD())
			return;
	}

	if (tablemode == OD_TABLE_MODE_INVERSE) {
		drvif_color_od_table(OD_table.table_inversemode, 0);
		drvif_color_od_gain(level);
	} else if (tablemode == OD_TABLE_MODE_DELTA) {
		drvif_color_od_table(OD_table.table_deltamode, 0);
		drvif_color_od_gain(level);
	} else { //tablemode == OD_TABLE_MODE_TARGET
		drvif_color_od_table(OD_table.table_targetmode, 1);
		memcpy(od_table_store, OD_table.table_targetmode, sizeof(od_table_store));
	}

	od_table_mode_store = (char)tablemode;
	bODTableLoaded = TRUE;
	/*if (!targetmode)
 		drvif_color_od_table(OD_table.table_deltamode, targetmode);
 	else
		drvif_color_od_table(OD_table.table_targetmode, targetmode);*/
#endif		
 }
#ifndef BUILD_QUICK_SHOW

bool fwif_color_od_init(UINT8 color_depth_bits, UINT8 dma_mode, UINT8 freq)
{
	unsigned short dispH;
	unsigned short dispV;
	UINT8 bit_sel = 2;

	dispV = Get_DISP_ACT_END_VPOS()-Get_DISP_ACT_STA_VPOS();
	dispH = Get_DISP_ACT_END_HPOS()-Get_DISP_ACT_STA_HPOS();
	rtd_pr_vpq_debug("OD_Height=%d,OD_Width=%d,freq=%d\n",dispV,dispH,Get_DISPLAY_REFRESH_RATE());

	// y mode
	drvif_color_od_Ymode();

	if(color_depth_bits == 6)
		bit_sel = 0;
	else if(color_depth_bits == 8)
		bit_sel = 1;
	else
		bit_sel = 2;

	if (drvif_color_od_pqc(dispV, dispH, bit_sel, dma_mode, 4, 1, 3))
		bODInited = TRUE;
	else
		return false;

	return true;
}
#endif
void fwif_color_handler(void)
{
	SCALER_DISP_CHANNEL display;
	unsigned short UZD_outH;
	unsigned short UZD_outV;

	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*Scaler_Setswitch_DVIandHDMI(SLR_Switch_Auto_MODE);	*/ /*=== marked by Elsie ===*/

	display = (SCALER_DISP_CHANNEL)(VIP_system_info_structure_table->input_display);
	UZD_outV = VIP_system_info_structure_table->Cap_Height;
	UZD_outH = VIP_system_info_structure_table->Cap_Width;

	drvif_color_digital_filtering_init(display);	/*hsinyi 20090617*/

/*==============peaking init=========================*/
	drvif_color_sharpness_init(display);
	if (fwif_VIP_get_Project_ID() != VIP_Project_ID_TV006)
		drvif_color_2dpk_init(display, 1);/*0 - ->old function, 1 - ->new function*/

	/*FixMe*/
	/*drvif_color_2dpk_MKIII_init(display, 1);*/ /*20130221 March add*/
/*=================================================*/




	drvif_color_noisereduction_impulse(display, 0, Scaler_DispGetStatus(display, SLR_DISP_FSYNC));

	drvif_color_noise_estimation_init(display, TRUE/*enable*/);

	/*default gamma*/
	/*drvif_color_S_Curve(0, 0, 0, 0, 0);*/
}

void fwif_set_od_setting_table(DRV_od_table_mode_t from_bin[])
{
	int i=0;
	extern DRV_od_table_mode_t OD_setting_table[VIP_QUALITY_SOURCE_NUM];
	for(i=0; i!= VIP_QUALITY_SOURCE_NUM; ++i)
	{
		OD_setting_table[i].type = from_bin[i].type;
		OD_setting_table[i].gain = from_bin[i].gain;

		//rtd_pr_vpq_emerg("[MHH][kernel]OD_setting_table[%d].type=%d,\n",i,OD_setting_table[i].type);
		//rtd_pr_vpq_emerg("[MHH][kernel]OD_setting_table[%d].gain=%d,\n",i,OD_setting_table[i].gain);
	}
	fwif_color_od_handler();
}

void fwif_set_od_table(DRV_od_table_t* from_bin)
{
	int i;
	extern DRV_od_table_t OD_table;
	//OD_table = from_bin;
	for(i=0; i!= OD_table_length; ++i)
	{
		OD_table.table_deltamode[i] = from_bin->table_deltamode[i];
		OD_table.table_targetmode[i] = from_bin->table_targetmode[i];
		OD_table.table_inversemode[i] = from_bin->table_inversemode[i];
		//rtd_pr_vpq_emerg("[MHH][kernel]OD_table->table_deltamode[%d]=%d,\n",i,OD_table.table_deltamode[i]);
	}
	fwif_color_od_handler();
}


void fwif_set_od_table_test(unsigned char from_bin[])
{
	int i=0;
	extern unsigned char m_pod_table_test[OD_table_length];

	for(i=0; i!= OD_table_length; ++i)
	{
		m_pod_table_test[i] = from_bin[i];
		//rtd_pr_vpq_emerg("[MHH][kernel]m_pod_table_test[%d]=%d,\n",i,m_pod_table_test[i]);
	}
}

extern VIP_DeMura_TBL DeMura_TBL;
void fwif_set_DeMura(VIP_DeMura_TBL* from_bin)
{
	int i=0;
	extern VIP_DeMura_TBL DeMura_TBL;
	DeMura_TBL.DeMura_CTRL_TBL = from_bin->DeMura_CTRL_TBL;
	DeMura_TBL.table_mode = from_bin->table_mode;
	DeMura_TBL.DeMura_Adaptive_TBL = from_bin->DeMura_Adaptive_TBL;

	for(i=0; i!= VIP_Demura_Encode_TBL_ROW*VIP_Demura_Encode_TBL_COL; ++i)
		DeMura_TBL.TBL[i] = from_bin->TBL[i];

	//rtd_pr_vpq_emerg("[MHH][kernel]DeMura_TBL.DeMura_CTRL_TBL.demura_table_scale=%d,\n",DeMura_TBL.DeMura_CTRL_TBL.demura_table_scale);
	//rtd_pr_vpq_emerg("[MHH][kernel]DeMura_TBL.DeMura_CTRL_TBL.demura_block_size=%d,\n",DeMura_TBL.DeMura_CTRL_TBL.demura_block_size);
	//rtd_pr_vpq_emerg("[MHH][kernel]DeMura_TBL.DeMura_CTRL_TBL.demura_rgb_table_seperate=%d,\n",DeMura_TBL.DeMura_CTRL_TBL.demura_rgb_table_seperate);
	//rtd_pr_vpq_emerg("[MHH][kernel]DeMura_TBL.DeMura_CTRL_TBL.demura_en=%d,\n",DeMura_TBL.DeMura_CTRL_TBL.demura_en);
	//rtd_pr_vpq_emerg("[MHH][kernel]DeMura_TBL.DeMura_CTRL_TBL.demura_r_upper_cutoff=%d,\n",DeMura_TBL.DeMura_CTRL_TBL.demura_r_upper_cutoff);
	//rtd_pr_vpq_emerg("[MHH][kernel]DeMura_TBL.DeMura_CTRL_TBL.demura_r_lower_cutoff=%d,\n",DeMura_TBL.DeMura_CTRL_TBL.demura_r_lower_cutoff);
	//rtd_pr_vpq_emerg("[MHH][kernel]DeMura_TBL.DeMura_CTRL_TBL.demura_r_m_level=%d,\n",DeMura_TBL.DeMura_CTRL_TBL.demura_r_m_level);
	//rtd_pr_vpq_emerg("[MHH][kernel]DeMura_TBL.DeMura_CTRL_TBL.demura_r_l_level=%d,\n",DeMura_TBL.DeMura_CTRL_TBL.demura_r_l_level);
	//rtd_pr_vpq_emerg("[MHH][kernel]DeMura_TBL.DeMura_CTRL_TBL.demura_r_h_level=%d,\n",DeMura_TBL.DeMura_CTRL_TBL.demura_r_h_level);
	//rtd_pr_vpq_emerg("[MHH]m_pDeMura_TBL.DeMura_Adaptive_TBL=%d,\n",DeMura_TBL.DeMura_Adaptive_TBL.adaptiveScale_En);
	//for(i=0; i!= VIP_Demura_Adaptive_Scale_LV_NUM; ++i)
		//rtd_pr_vpq_emerg("[MHH][kernel]m_pDeMura_TBL.adaptiveScale[%d]=%d,\n",i,DeMura_TBL.DeMura_Adaptive_TBL.adaptiveScale[i]);
	fwif_color_DeMura_init();
}


void fwif_set_VALC(unsigned char from_bin[6][17][17])
{
	int i,j,k;
	extern unsigned char VALC_Table[6][17][17];
	unsigned char InputLUT[578] = {0};
	for(i=0; i!= 6; ++i)
	{
		for(j=0; j!= 17; ++j)
		{
			for(k=0; k!= 17; ++k)
			{
				VALC_Table[i][j][k] = from_bin[i][j][k];
				//rtd_pr_vpq_emerg("[MHH][kernel]m_pVALC_Table[i][j][k]=%d,\n",VALC_Table[i][j][k]);
			}
		}
	}

	for(i=0; i!= 17; ++i)
	{
		rtd_pr_vpq_emerg("[MHH][kernel]m_pVALC_Table=%d\n",VALC_Table[0][0][i]);
	}
	//R
	memcpy(&(InputLUT[0]), &(VALC_Table[_PCID2_COLOR_R*2][0]), sizeof(unsigned char)*289);
	memcpy(&(InputLUT[289]), &(VALC_Table[_PCID2_COLOR_R*2+1][0]), sizeof(unsigned char)*289);
	fwif_color_set_pcid2_valuetable_channel(InputLUT,_PCID2_COLOR_R);
	//G
	memcpy(&(InputLUT[0]), &(VALC_Table[_PCID2_COLOR_G*2][0]), sizeof(unsigned char)*289);
	memcpy(&(InputLUT[289]), &(VALC_Table[_PCID2_COLOR_G*2+1][0]), sizeof(unsigned char)*289);
	fwif_color_set_pcid2_valuetable_channel(InputLUT,_PCID2_COLOR_G);
	//B
	memcpy(&(InputLUT[0]), &(VALC_Table[_PCID2_COLOR_B*2][0]), sizeof(unsigned char)*289);
	memcpy(&(InputLUT[289]), &(VALC_Table[_PCID2_COLOR_B*2+1][0]), sizeof(unsigned char)*289);
	fwif_color_set_pcid2_valuetable_channel(InputLUT,_PCID2_COLOR_B);

	//fwif_color_set_POD_DATA_table
	//fwif_color_set_pcid2_valuetable_bycolor((&(*VALC_Table)[0][0]), _PCID2_COLOR_R);
	//fwif_color_set_pcid2_valuetable_bycolor((&(*VALC_Table)[0][0]+289*2), _PCID2_COLOR_G);
	//fwif_color_set_pcid2_valuetable_bycolor((&(*VALC_Table)[0][0]+289*4), _PCID2_COLOR_B);
}

void fwif_set_VALCprotectSetting(DRV_pcid_valc_t from_bin[])
{
	int i;
	extern DRV_pcid_valc_t pVALCprotectSetting[2];
	for(i=0; i!= 2; ++i)
	{
		pVALCprotectSetting[i] = from_bin[i];
	}
	drvif_color_pcid_VALC_protection(pVALCprotectSetting);
}

void fwif_set_LEDOutGamma(unsigned short from_bin[])
{
	int i;
	extern unsigned short LEDOutGamma[257];
	extern unsigned int out_GAMMA_R[128], out_GAMMA_G[128], out_GAMMA_B[128];
	for(i=0; i!= 257; ++i)
	{
		LEDOutGamma[i] = from_bin[i];
		//rtd_pr_vpq_emerg("[MHH][kernel]LEDOutGamma[%d]=%d,\n",i,LEDOutGamma[i]);
	}
	fwif_color_out_gamma_encode(out_GAMMA_R, out_GAMMA_G, out_GAMMA_B, LEDOutGamma, LEDOutGamma, LEDOutGamma);
}

void fwif_set_Output_InvOutput_Gamma(VIP_Output_InvOutput_Gamma from_bin[])
{
	int i;
	extern int OutputGAMMA_tableselect;
	extern VIP_Output_InvOutput_Gamma Output_InvOutput_Gamma[Output_InvOutput_Gamma_TBL_MAX];
	for(i=0; i!= Output_InvOutput_Gamma_TBL_MAX; ++i)
	{
		Output_InvOutput_Gamma[i] = from_bin[i];
		//rtd_pr_vpq_emerg("[MHH]m_pLEDOutGamma[%d]=%d,\n",i,m_pOutput_InvOutput_Gamma[i].Output_InvOutput_Gamma_CTRL_ITEM.output_Gamma_En);
	}
	fwif_color_colorwrite_InvOutput_gamma(OutputGAMMA_tableselect);
	//rtd_pr_vpq_debug("OutputGAMMA_tableselect=%d,\n",OutputGAMMA_tableselect);
}



void fwif_color_od_handler(void)
{
	unsigned char source;
	extern DRV_od_table_mode_t OD_setting_table[VIP_QUALITY_SOURCE_NUM];
	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);/* get vip source by input src and timing*/
	if (source >= VIP_QUALITY_SOURCE_NUM)
		source = 0;

 	if (Scaler_Init_OD()) {
		//fwif_color_set_od_table(OD_setting_table[source].type,OD_setting_table[source].gain);
		fwif_color_set_od_table(OD_TABLE_MODE_DELTA,OD_setting_table[source].gain);
	 	fwif_color_set_od(1);
 	}
}

void fwif_color_set_new_uvc(unsigned char src_idx, unsigned char level)
{
	extern DRV_NEW_UVC gUVCTable[new_UVC_MAX];
	DRV_NEW_UVC UVCTableHere;

	if (level >= new_UVC_MAX)
		level = 0;

	if (level == 255) {
		UVCTableHere.UVC_ctrl.UVC_main_en = 0;
		UVCTableHere.UVC_ctrl.UVC_sub_en = 0;
	} else {
		UVCTableHere.UVC_ctrl.UVC_main_en = gUVCTable[level].UVC_ctrl.UVC_main_en;
		UVCTableHere.UVC_ctrl.UVC_sub_en = gUVCTable[level].UVC_ctrl.UVC_sub_en;
	}

	UVCTableHere.UVC_ctrl.UVC_overlay_sel = gUVCTable[level].UVC_ctrl.UVC_overlay_sel;
	UVCTableHere.UVC_ctrl.UVC_skin_sel = gUVCTable[level].UVC_ctrl.UVC_skin_sel;
	UVCTableHere.UVC_ctrl.UVC_qdrt1_en = gUVCTable[level].UVC_ctrl.UVC_qdrt1_en;
	UVCTableHere.UVC_ctrl.UVC_qdrt2_en = gUVCTable[level].UVC_ctrl.UVC_qdrt2_en;
	UVCTableHere.UVC_ctrl.UVC_qdrt3_en = gUVCTable[level].UVC_ctrl.UVC_qdrt3_en;
	UVCTableHere.UVC_ctrl.UVC_qdrt4_en = gUVCTable[level].UVC_ctrl.UVC_qdrt4_en;
	UVCTableHere.UVC_ctrl.UVC_y0 = gUVCTable[level].UVC_ctrl.UVC_y0;
	UVCTableHere.UVC_ctrl.UVC_y_slope = gUVCTable[level].UVC_ctrl.UVC_y_slope;

	UVCTableHere.UVC_set_global.UVC_U_thd1 = gUVCTable[level].UVC_set_global.UVC_U_thd1;
	UVCTableHere.UVC_set_global.UVC_U_slope1 = gUVCTable[level].UVC_set_global.UVC_U_slope1;
	UVCTableHere.UVC_set_global.UVC_U_thd2 = gUVCTable[level].UVC_set_global.UVC_U_thd2;
	UVCTableHere.UVC_set_global.UVC_U_slope2 = gUVCTable[level].UVC_set_global.UVC_U_slope2;
	UVCTableHere.UVC_set_global.UVC_V_thd1 = gUVCTable[level].UVC_set_global.UVC_V_thd1;
	UVCTableHere.UVC_set_global.UVC_V_slope1 = gUVCTable[level].UVC_set_global.UVC_V_slope1;
	UVCTableHere.UVC_set_global.UVC_V_thd2 = gUVCTable[level].UVC_set_global.UVC_V_thd2;
	UVCTableHere.UVC_set_global.UVC_V_slope2 = gUVCTable[level].UVC_set_global.UVC_V_slope2;

	UVCTableHere.UVC_set_skin.UVC_U_thd1 = gUVCTable[level].UVC_set_skin.UVC_U_thd1;
	UVCTableHere.UVC_set_skin.UVC_U_slope1 = gUVCTable[level].UVC_set_skin.UVC_U_slope1;
	UVCTableHere.UVC_set_skin.UVC_U_thd2 = gUVCTable[level].UVC_set_skin.UVC_U_thd2;
	UVCTableHere.UVC_set_skin.UVC_U_slope2 = gUVCTable[level].UVC_set_skin.UVC_U_slope2;
	UVCTableHere.UVC_set_skin.UVC_V_thd1 = gUVCTable[level].UVC_set_skin.UVC_V_thd1;
	UVCTableHere.UVC_set_skin.UVC_V_slope1 = gUVCTable[level].UVC_set_skin.UVC_V_slope1;
	UVCTableHere.UVC_set_skin.UVC_V_thd2 = gUVCTable[level].UVC_set_skin.UVC_V_thd2;
	UVCTableHere.UVC_set_skin.UVC_V_slope2 = gUVCTable[level].UVC_set_skin.UVC_V_slope2;
	drvif_color_set_new_uvc((DRV_NEW_UVC *)(&UVCTableHere));
}


void fwif_color_set_blue_stretch(unsigned char src_idx, unsigned char table)
{
	extern DRV_Gamma_BS gBSTable[Blue_Stretch_MAX];

	if (table >= Blue_Stretch_MAX)
		table = 0;
	drvif_color_set_Blue_Stretch((DRV_Gamma_BS *)(&(gBSTable[table])));
}

void fwif_color_set_pcid(unsigned char bpcid)
{

}

void fwif_color_set_pcid_valuetable(unsigned char* InputLUT)
{
	// Transfer the value to register value
	bool isSameTbl = true;
	int Row = 0, Col = 0;
	unsigned int TblValue = 0;
	for(Row=0; Row<9; Row++)
	{
		for(Col=0; Col<9; Col++)
		{
			//-------------------
			// Value1 | Value3 |
			//-------------------   will be transform to {Value1, Value2, Value3, Value4} {MSB, LSB}
			// Value2 | Value4 |
			//-------------------
			unsigned int Value1 = 0, Value2 = 0, Value3 = 0, Value4 = 0;
			if( Row<8 && Col<8 )
			{
				Value1 = InputLUT[Row*34   +Col*2  ];
				Value2 = InputLUT[Row*34+17+Col*2  ];
				Value3 = InputLUT[Row*34   +Col*2+1];
				Value4 = InputLUT[Row*34+17+Col*2+1];
			}
			else if( Row < 8 )
			{
				Value1 = InputLUT[Row*34   +Col*2  ];
				Value2 = InputLUT[Row*34+17+Col*2  ];
				Value3 = Value1;
				Value4 = Value2;
			}
			else if( Col < 8 )
			{
				Value1 = InputLUT[Row*34   +Col*2  ];
				Value2 = Value1;
				Value3 = InputLUT[Row*34   +Col*2+1];
				Value4 = Value3;
			}
			else
			{
				Value1 = InputLUT[Row*34   +Col*2  ];
				Value2 = Value1;
				Value3 = Value1;
				Value4 = Value1;
			}
			TblValue = ( (Value1 << 24) + (Value2 << 16) + (Value3 << 8) + Value4 );
			if( PCID_ValueTBL[Row*9+Col] != TblValue )
			{
				PCID_ValueTBL[Row*9+Col] = TblValue;
				isSameTbl = false;
			}
		}
	}

	if( isSameTbl == false )
	{
		fwif_color_WaitFor_DEN_STOP_MEMCDTG();
		drvif_color_pcid_valuetable(PCID_ValueTBL);
	}


}

void fwif_color_set_pcid_pixel_setting(void)
{

}

void fwif_color_set_pcid2(unsigned char bpcid2)
{

}


void fwif_color_set_pcid2_valuetable(unsigned char* InputLUT)
{
	// Transfer the value to register value
	bool isSameTbl_1 = true, isSameTbl_2 = true;
	int Row = 0, Col = 0;
	unsigned int TblValue = 0;
	for(Row=0; Row<9; Row++)
	{
		for(Col=0; Col<9; Col++)
		{
			//-------------------
			// Value1 | Value3 |
			//-------------------   will be transform to {Value1, Value2, Value3, Value4} {MSB, LSB}
			// Value2 | Value4 |
			//-------------------
			unsigned int Value1 = 0, Value2 = 0, Value3 = 0, Value4 = 0;
			if( Row<8 && Col<8 )
			{
				Value1 = InputLUT[Row*34   +Col*2  ];
				Value2 = InputLUT[Row*34+17+Col*2  ];
				Value3 = InputLUT[Row*34   +Col*2+1];
				Value4 = InputLUT[Row*34+17+Col*2+1];
			}
			else if( Row < 8 )
			{
				Value1 = InputLUT[Row*34   +Col*2  ];
				Value2 = InputLUT[Row*34+17+Col*2  ];
				Value3 = Value1;
				Value4 = Value2;
			}
			else if( Col < 8 )
			{
				Value1 = InputLUT[Row*34   +Col*2  ];
				Value2 = Value1;
				Value3 = InputLUT[Row*34   +Col*2+1];
				Value4 = Value3;
			}
			else
			{
				Value1 = InputLUT[Row*34   +Col*2  ];
				Value2 = Value1;
				Value3 = Value1;
				Value4 = Value1;
			}
			TblValue = ( (Value1 << 24) + (Value2 << 16) + (Value3 << 8) + Value4 );
			if( POD_ValueTBL_1[Row*9+Col] != TblValue )
			{
				POD_ValueTBL_1[Row*9+Col] = TblValue;
				isSameTbl_1 = false;
			}
		}
	}

	for(Row=0; Row<9; Row++)
	{
		for(Col=0; Col<9; Col++)
		{
			//-------------------
			// Value1 | Value3 |
			//-------------------   will be transform to {Value1, Value2, Value3, Value4} {MSB, LSB}
			// Value2 | Value4 |
			//-------------------
			unsigned int Value1 = 0, Value2 = 0, Value3 = 0, Value4 = 0;
			if( Row<8 && Col<8 )
			{
				Value1 = InputLUT[289+Row*34   +Col*2  ];
				Value2 = InputLUT[289+Row*34+17+Col*2  ];
				Value3 = InputLUT[289+Row*34   +Col*2+1];
				Value4 = InputLUT[289+Row*34+17+Col*2+1];
			}
			else if( Row < 8 )
			{
				Value1 = InputLUT[289+Row*34   +Col*2  ];
				Value2 = InputLUT[289+Row*34+17+Col*2  ];
				Value3 = Value1;
				Value4 = Value2;
			}
			else if( Col < 8 )
			{
				Value1 = InputLUT[289+Row*34   +Col*2  ];
				Value2 = Value1;
				Value3 = InputLUT[289+Row*34   +Col*2+1];
				Value4 = Value3;
			}
			else
			{
				Value1 = InputLUT[289+Row*34   +Col*2  ];
				Value2 = Value1;
				Value3 = Value1;
				Value4 = Value1;
			}
			TblValue = ( (Value1 << 24) + (Value2 << 16) + (Value3 << 8) + Value4 );
			if( POD_ValueTBL_2[Row*9+Col] != TblValue )
			{
				POD_ValueTBL_2[Row*9+Col] = TblValue;
				isSameTbl_2 = false;
			}
		}
	}
#if 0
	if( isSameTbl_1 == false || isSameTbl_2 == false )
	{

		if( isSameTbl_1 == false ) {
			fwif_color_WaitFor_SYNC_START_MEMCDTG();
			drvif_color_pcid2_valuetable(POD_ValueTBL_1, 0, _PCID2_COLOR_ALL);
		}
		if( isSameTbl_2 == false ) {
			fwif_color_WaitFor_SYNC_START_MEMCDTG();
			drvif_color_pcid2_valuetable(POD_ValueTBL_2, 1, _PCID2_COLOR_ALL);
		}
	}
#else
	fwif_color_WaitFor_SYNC_START_MEMCDTG();
	drvif_color_pcid2_valuetable(POD_ValueTBL_1, 0, _PCID2_COLOR_ALL);
	fwif_color_WaitFor_SYNC_START_MEMCDTG();
	drvif_color_pcid2_valuetable(POD_ValueTBL_2, 1, _PCID2_COLOR_ALL);
#endif
}

void fwif_color_set_pcid2_valuetable_channel(unsigned char* InputLUT, unsigned char channel)
{
	// Transfer the value to register value
	bool isSameTbl_1 = true, isSameTbl_2 = true;
	int Row = 0, Col = 0;
	unsigned int TblValue = 0;
	for(Row=0; Row<9; Row++)
	{
		for(Col=0; Col<9; Col++)
		{
			//-------------------
			// Value1 | Value3 |
			//-------------------   will be transform to {Value1, Value2, Value3, Value4} {MSB, LSB}
			// Value2 | Value4 |
			//-------------------
			unsigned int Value1 = 0, Value2 = 0, Value3 = 0, Value4 = 0;
			if( Row<8 && Col<8 )
			{
				Value1 = InputLUT[Row*34   +Col*2  ];
				Value2 = InputLUT[Row*34+17+Col*2  ];
				Value3 = InputLUT[Row*34   +Col*2+1];
				Value4 = InputLUT[Row*34+17+Col*2+1];
			}
			else if( Row < 8 )
			{
				Value1 = InputLUT[Row*34   +Col*2  ];
				Value2 = InputLUT[Row*34+17+Col*2  ];
				Value3 = Value1;
				Value4 = Value2;
			}
			else if( Col < 8 )
			{
				Value1 = InputLUT[Row*34   +Col*2  ];
				Value2 = Value1;
				Value3 = InputLUT[Row*34   +Col*2+1];
				Value4 = Value3;
			}
			else
			{
				Value1 = InputLUT[Row*34   +Col*2  ];
				Value2 = Value1;
				Value3 = Value1;
				Value4 = Value1;
			}
			TblValue = ( (Value1 << 24) + (Value2 << 16) + (Value3 << 8) + Value4 );
			if( POD_ValueTBL_1[Row*9+Col] != TblValue )
			{
				POD_ValueTBL_1[Row*9+Col] = TblValue;
				isSameTbl_1 = false;
			}
		}
	}

	for(Row=0; Row<9; Row++)
	{
		for(Col=0; Col<9; Col++)
		{
			//-------------------
			// Value1 | Value3 |
			//-------------------   will be transform to {Value1, Value2, Value3, Value4} {MSB, LSB}
			// Value2 | Value4 |
			//-------------------
			unsigned int Value1 = 0, Value2 = 0, Value3 = 0, Value4 = 0;
			if( Row<8 && Col<8 )
			{
				Value1 = InputLUT[289+Row*34   +Col*2  ];
				Value2 = InputLUT[289+Row*34+17+Col*2  ];
				Value3 = InputLUT[289+Row*34   +Col*2+1];
				Value4 = InputLUT[289+Row*34+17+Col*2+1];
			}
			else if( Row < 8 )
			{
				Value1 = InputLUT[289+Row*34   +Col*2  ];
				Value2 = InputLUT[289+Row*34+17+Col*2  ];
				Value3 = Value1;
				Value4 = Value2;
			}
			else if( Col < 8 )
			{
				Value1 = InputLUT[289+Row*34   +Col*2  ];
				Value2 = Value1;
				Value3 = InputLUT[289+Row*34   +Col*2+1];
				Value4 = Value3;
			}
			else
			{
				Value1 = InputLUT[289+Row*34   +Col*2  ];
				Value2 = Value1;
				Value3 = Value1;
				Value4 = Value1;
			}
			TblValue = ( (Value1 << 24) + (Value2 << 16) + (Value3 << 8) + Value4 );
			if( POD_ValueTBL_2[Row*9+Col] != TblValue )
			{
				POD_ValueTBL_2[Row*9+Col] = TblValue;
				isSameTbl_2 = false;
			}
		}
	}

#if 0
	if( isSameTbl_1 == false || isSameTbl_2 == false )
	{

		if( isSameTbl_1 == false ) {
			fwif_color_WaitFor_SYNC_START_MEMCDTG();
			drvif_color_pcid2_valuetable(POD_ValueTBL_1, 0, channel);
		}
		if( isSameTbl_2 == false ) {
			fwif_color_WaitFor_SYNC_START_MEMCDTG();
			drvif_color_pcid2_valuetable(POD_ValueTBL_2, 1, channel);
		}
	}
#else
	fwif_color_WaitFor_SYNC_START_MEMCDTG();
	drvif_color_pcid2_valuetable(POD_ValueTBL_1, 0, channel);
	fwif_color_WaitFor_SYNC_START_MEMCDTG();
	drvif_color_pcid2_valuetable(POD_ValueTBL_2, 1, channel);
#endif
}

void fwif_color_set_pcid2_poltable(void)
{
	unsigned int Pol_R[8] = {0};
	unsigned int Pol_G[8] = {0};
	unsigned int Pol_B[8] = {0};

#if 0
	// Read INV1/2/3/4 setting from EPI packet setting
	unsigned int TmpRegValue = IoReg_Read32(EPI_EPI_PACKET_CTR1_INV_TABLE_1_reg);

	// Change reigster sequence from
	// (MSB) Line1 { INV1, INV2, INV3, INV4 } ... Line 8 { INV1, INV2, INV3, INV4 } to
	// (MSB) Line8 { INV4, INV3, INV2, INV1 } ... Line 1 { INV1, INV2, INV3, INV4 }
	unsigned int INV_RegValue = 0;
	int i = 0, j = 0;
	for( i=0; i<32; i++ )
		INV_RegValue = ( INV_RegValue << 1 ) + ( ( TmpRegValue >> i ) & 0x01 );

	// Fill the INV signal 4x8 table
	int DummyLineNum = 2;
	unsigned char INV_Signal[8][4] = {0};
	for( i=0; i<8; i++ )
	{
		unsigned int BitOffset = (i*4);
		unsigned int Mask = (0xf << BitOffset);
		unsigned int TmpValue = ( INV_RegValue & Mask ) >> BitOffset;
		for( j=0; j<4; j++ )
		{
			INV_Signal[(i+8-DummyLineNum)%8][j] = ( ( TmpValue >> (3-j) ) & ( 0x1 ) );
		}
	}

	// Fill in the INV 4x16 expand table
	unsigned char INV_ExpandTable[4][16] = {0};
	for( i=0; i<8; i++ )
	{
		INV_ExpandTable[i/2][ ((i+1)%2) + 0 ] = (INV_Signal[i][0] == 1)? 1 : 0;
		INV_ExpandTable[i/2][ ((i+1)%2) + 4 ] = (INV_Signal[i][0] == 1)? 0 : 1;

		INV_ExpandTable[i/2][ ((i+1)%2) + 2 ] = (INV_Signal[i][1] == 1)? 1 : 0;
		INV_ExpandTable[i/2][ ((i+1)%2) + 6 ] = (INV_Signal[i][1] == 1)? 0 : 1;

		INV_ExpandTable[i/2][ ((i+1)%2) + 8 ] = (INV_Signal[i][2] == 1)? 1 : 0;
		INV_ExpandTable[i/2][ ((i+1)%2) + 12] = (INV_Signal[i][2] == 1)? 0 : 1;

		INV_ExpandTable[i/2][ ((i+1)%2) + 10] = (INV_Signal[i][3] == 1)? 1 : 0;
		INV_ExpandTable[i/2][ ((i+1)%2) + 14] = (INV_Signal[i][3] == 1)? 0 : 1;
	}

	// Fill in the Polarity 4x24 expand table according to INV expand table[0~4][0~8]
	unsigned char Pol_ExpandTable[4][24] = {0};
	for( i=0; i<4; i++ )
		for( j=0; j<24; j++ )
			Pol_ExpandTable[i][j] = INV_ExpandTable[i][j%8];

	// Generate R/G/B channel polarity
	unsigned int Pol_R[8] = {0};
	unsigned int Pol_G[8] = {0};
	unsigned int Pol_B[8] = {0};
	for(i=0; i<8; i++)
	{
		for(j=0; j<8; j++)
		{
			Pol_R[i] = (Pol_R[i] << 1) + Pol_ExpandTable[i%4][j*3  ];
			Pol_G[i] = (Pol_G[i] << 1) + Pol_ExpandTable[i%4][j*3+1];
			Pol_B[i] = (Pol_B[i] << 1) + Pol_ExpandTable[i%4][j*3+2];
		}
	}
#endif

	drvif_color_pcid2_poltable(Pol_R, _PCID2_COLOR_R);
	drvif_color_pcid2_poltable(Pol_G, _PCID2_COLOR_G);
	drvif_color_pcid2_poltable(Pol_B, _PCID2_COLOR_B);
}

void fwif_color_set_pcid2_pixel_setting(void)
{
	// Default setting as the spec
	DRV_pcid2_data_t sPcid2Setting;

	// Noise Reduction
	sPcid2Setting.pcid_thd_en = 0;
	sPcid2Setting.pcid_thd_mode = 0;
	sPcid2Setting.pcid_tbl1_r_th = 0;
	sPcid2Setting.pcid_tbl1_g_th = 0;
	sPcid2Setting.pcid_tbl1_b_th = 0;
	sPcid2Setting.pcid_tbl2_r_th = 0;
	sPcid2Setting.pcid_tbl2_g_th = 0;
	sPcid2Setting.pcid_tbl2_b_th = 0;

	// XTR
	sPcid2Setting.xtr_tbl1_en = 1;
	sPcid2Setting.xtr_tbl2_en = 1;

	// Other
	sPcid2Setting.boundary_mode = 0;

	// Pixel reference
	/* line 1 pixel setting */
	sPcid2Setting.pixel_ref.line1_even_r.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line1_even_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_even_r.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line1_even_g.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line1_even_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_even_g.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line1_even_b.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line1_even_b.point_sel = _REF_POINT_NEXT;
	sPcid2Setting.pixel_ref.line1_even_b.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line1_odd_r.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line1_odd_r.point_sel = _REF_POINT_PREV;
	sPcid2Setting.pixel_ref.line1_odd_r.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line1_odd_g.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line1_odd_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_odd_g.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line1_odd_b.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line1_odd_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_odd_b.color_sel = _REF_COLOR_G;

	/* line 2 pixel setting */
	sPcid2Setting.pixel_ref.line2_even_r.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line2_even_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line2_even_r.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line2_even_g.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line2_even_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line2_even_g.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line2_even_b.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line2_even_b.point_sel = _REF_POINT_NEXT;
	sPcid2Setting.pixel_ref.line2_even_b.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line2_odd_r.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line2_odd_r.point_sel = _REF_POINT_PREV;
	sPcid2Setting.pixel_ref.line2_odd_r.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line2_odd_g.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line2_odd_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line2_odd_g.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line2_odd_b.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line2_odd_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line2_odd_b.color_sel = _REF_COLOR_G;

	/* line 3 pixel setting */
	sPcid2Setting.pixel_ref.line3_even_r.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line3_even_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_even_r.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line3_even_g.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line3_even_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_even_g.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line3_even_b.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line3_even_b.point_sel = _REF_POINT_NEXT;
	sPcid2Setting.pixel_ref.line3_even_b.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line3_odd_r.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line3_odd_r.point_sel = _REF_POINT_PREV;
	sPcid2Setting.pixel_ref.line3_odd_r.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line3_odd_g.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line3_odd_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_odd_g.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line3_odd_b.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line3_odd_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_odd_b.color_sel = _REF_COLOR_G;

	/* line 4 pixel setting */
	sPcid2Setting.pixel_ref.line4_even_r.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line4_even_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line4_even_r.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line4_even_g.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line4_even_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line4_even_g.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line4_even_b.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line4_even_b.point_sel = _REF_POINT_NEXT;
	sPcid2Setting.pixel_ref.line4_even_b.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line4_odd_r.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line4_odd_r.point_sel = _REF_POINT_PREV;
	sPcid2Setting.pixel_ref.line4_odd_r.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line4_odd_g.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line4_odd_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line4_odd_g.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line4_odd_b.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line4_odd_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line4_odd_b.color_sel = _REF_COLOR_G;

	drvif_color_pcid2_pixel_setting(&sPcid2Setting);
}

void fwif_color_set_PCID_BypassSetting(void)
{
	// Default setting as the spec
	DRV_pcid2_data_t sPcid2Setting;

	// Noise Reduction
	sPcid2Setting.pcid_thd_en = 0;
	sPcid2Setting.pcid_thd_mode = 0;
	sPcid2Setting.pcid_tbl1_r_th = 0;
	sPcid2Setting.pcid_tbl1_g_th = 0;
	sPcid2Setting.pcid_tbl1_b_th = 0;
	sPcid2Setting.pcid_tbl2_r_th = 0;
	sPcid2Setting.pcid_tbl2_g_th = 0;
	sPcid2Setting.pcid_tbl2_b_th = 0;

	// XTR
	sPcid2Setting.xtr_tbl1_en = 1;
	sPcid2Setting.xtr_tbl2_en = 1;

	// Other
	sPcid2Setting.boundary_mode = 0;

	// Pixel reference
	/* line 1 pixel setting */
	sPcid2Setting.pixel_ref.line1_even_r.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line1_even_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_even_r.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line1_even_g.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line1_even_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_even_g.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line1_even_b.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line1_even_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_even_b.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line1_odd_r.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line1_odd_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_odd_r.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line1_odd_g.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line1_odd_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_odd_g.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line1_odd_b.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line1_odd_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_odd_b.color_sel = _REF_COLOR_B;

	/* line 2 pixel setting */
	sPcid2Setting.pixel_ref.line2_even_r.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line2_even_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line2_even_r.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line2_even_g.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line2_even_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line2_even_g.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line2_even_b.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line2_even_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line2_even_b.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line2_odd_r.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line2_odd_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line2_odd_r.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line2_odd_g.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line2_odd_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line2_odd_g.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line2_odd_b.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line2_odd_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line2_odd_b.color_sel = _REF_COLOR_B;

	/* line 3 pixel setting */
	sPcid2Setting.pixel_ref.line3_even_r.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line3_even_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_even_r.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line3_even_g.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line3_even_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_even_g.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line3_even_b.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line3_even_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_even_b.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line3_odd_r.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line3_odd_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_odd_r.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line3_odd_g.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line3_odd_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_odd_g.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line3_odd_b.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line3_odd_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_odd_b.color_sel = _REF_COLOR_B;

	/* line 4 pixel setting */
	sPcid2Setting.pixel_ref.line4_even_r.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line4_even_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line4_even_r.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line4_even_g.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line4_even_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line4_even_g.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line4_even_b.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line4_even_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line4_even_b.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line4_odd_r.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line4_odd_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line4_odd_r.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line4_odd_g.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line4_odd_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line4_odd_g.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line4_odd_b.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line4_odd_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line4_odd_b.color_sel = _REF_COLOR_B;

	drvif_color_pcid2_pixel_setting(&sPcid2Setting);

	//======================
	drvif_color_pcid_poltable();
}

void fwif_color_set_PCID_RgnWeightSetting(DRV_pcid_valc_t *pSetting)
{
	int i=0;
	DRV_pcid_valc_t rgnSetting;
	rgnSetting.RgnWeightEnable = pSetting->RgnWeightEnable;
	rgnSetting.InputSz = pSetting->InputSz;
	rgnSetting.SftSel = pSetting->SftSel;
	rgnSetting.Sta_x = pSetting->Sta_x;
	rgnSetting.Sta_y = pSetting->Sta_y;
	for(i=0; i<25; i++)
	{
		rgnSetting.HorWeight[i] = pSetting->HorWeight[i];
	}
	for(i=0; i<10; i++)
	{
		rgnSetting.VerWeight[i] = pSetting->VerWeight[i];
	}
	drvif_color_set_pcid_regional_weight_setting(&rgnSetting);
}

void fwif_color_get_PCID_RgnWeightSetting(DRV_pcid_valc_t *pSetting)
{
	drvif_color_get_pcid_regional_weight_setting(pSetting);
}


void fwif_color_set_PCID_VALCSetting(void)
{
	// Default setting as the spec
	DRV_pcid2_data_t sPcid2Setting;
	unsigned int pPcid2PolTable[8]={0};

	// Noise Reduction
	sPcid2Setting.pcid_thd_en = 0;
	sPcid2Setting.pcid_thd_mode = 0;
	sPcid2Setting.pcid_tbl1_r_th = 0;
	sPcid2Setting.pcid_tbl1_g_th = 0;
	sPcid2Setting.pcid_tbl1_b_th = 0;
	sPcid2Setting.pcid_tbl2_r_th = 0;
	sPcid2Setting.pcid_tbl2_g_th = 0;
	sPcid2Setting.pcid_tbl2_b_th = 0;

	// XTR
	sPcid2Setting.xtr_tbl1_en = 1;
	sPcid2Setting.xtr_tbl2_en = 1;

	// Other
	sPcid2Setting.boundary_mode = 0;

	// Pixel reference
	/* line 1 pixel setting */
	sPcid2Setting.pixel_ref.line1_even_r.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line1_even_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_even_r.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line1_even_g.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line1_even_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_even_g.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line1_even_b.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line1_even_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_even_b.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line1_odd_r.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line1_odd_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_odd_r.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line1_odd_g.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line1_odd_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_odd_g.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line1_odd_b.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line1_odd_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_odd_b.color_sel = _REF_COLOR_B;

	/* line 2 pixel setting */
	sPcid2Setting.pixel_ref.line2_even_r.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line2_even_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line2_even_r.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line2_even_g.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line2_even_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line2_even_g.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line2_even_b.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line2_even_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line2_even_b.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line2_odd_r.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line2_odd_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line2_odd_r.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line2_odd_g.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line2_odd_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line2_odd_g.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line2_odd_b.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line2_odd_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line2_odd_b.color_sel = _REF_COLOR_B;

	/* line 3 pixel setting */
	sPcid2Setting.pixel_ref.line3_even_r.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line3_even_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_even_r.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line3_even_g.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line3_even_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_even_g.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line3_even_b.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line3_even_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_even_b.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line3_odd_r.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line3_odd_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_odd_r.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line3_odd_g.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line3_odd_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_odd_g.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line3_odd_b.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line3_odd_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_odd_b.color_sel = _REF_COLOR_B;

	/* line 4 pixel setting */
	sPcid2Setting.pixel_ref.line4_even_r.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line4_even_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line4_even_r.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line4_even_g.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line4_even_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line4_even_g.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line4_even_b.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line4_even_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line4_even_b.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line4_odd_r.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line4_odd_r.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line4_odd_r.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line4_odd_g.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line4_odd_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line4_odd_g.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line4_odd_b.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line4_odd_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line4_odd_b.color_sel = _REF_COLOR_B;

	drvif_color_pcid2_pixel_setting(&sPcid2Setting);

	//======================
	pPcid2PolTable[0] = 0xaaaa;
	pPcid2PolTable[1] = 0x0000;
	pPcid2PolTable[2] = 0x5555;
	pPcid2PolTable[3] = 0xffff;
	pPcid2PolTable[4] = 0xaaaa;
	pPcid2PolTable[5] = 0x0000;
	pPcid2PolTable[6] = 0x5555;
	pPcid2PolTable[7] = 0xffff;
	drvif_color_pcid2_poltable(&(pPcid2PolTable[0]), _PCID2_COLOR_R);
	pPcid2PolTable[0] = 0x5555;
	pPcid2PolTable[1] = 0x0000;
	pPcid2PolTable[2] = 0xaaaa;
	pPcid2PolTable[3] = 0xffff;
	pPcid2PolTable[4] = 0x5555;
	pPcid2PolTable[5] = 0x0000;
	pPcid2PolTable[6] = 0xaaaa;
	pPcid2PolTable[7] = 0xffff;
	drvif_color_pcid2_poltable(&(pPcid2PolTable[0]), _PCID2_COLOR_G);
	pPcid2PolTable[0] = 0xaaaa;
	pPcid2PolTable[1] = 0x0000;
	pPcid2PolTable[2] = 0x5555;
	pPcid2PolTable[3] = 0xffff;
	pPcid2PolTable[4] = 0xaaaa;
	pPcid2PolTable[5] = 0x0000;
	pPcid2PolTable[6] = 0x5555;
	pPcid2PolTable[7] = 0xffff;
	drvif_color_pcid2_poltable(&(pPcid2PolTable[0]), _PCID2_COLOR_B);
}

void fwif_color_set_PCID_LineOD_DualGateSetting(void)
{
	// Default setting as the spec
	DRV_pcid2_data_t sPcid2Setting;

	// Noise Reduction
	sPcid2Setting.pcid_thd_en = 0;
	sPcid2Setting.pcid_thd_mode = 0;
	sPcid2Setting.pcid_tbl1_r_th = 0;
	sPcid2Setting.pcid_tbl1_g_th = 0;
	sPcid2Setting.pcid_tbl1_b_th = 0;
	sPcid2Setting.pcid_tbl2_r_th = 0;
	sPcid2Setting.pcid_tbl2_g_th = 0;
	sPcid2Setting.pcid_tbl2_b_th = 0;

	// XTR
	sPcid2Setting.xtr_tbl1_en = 1;
	sPcid2Setting.xtr_tbl2_en = 1;

	// Other
	sPcid2Setting.boundary_mode = 0;

	// Pixel reference
	/* line 1 pixel setting */
	sPcid2Setting.pixel_ref.line1_even_r.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line1_even_r.point_sel = _REF_POINT_PREV;
	sPcid2Setting.pixel_ref.line1_even_r.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line1_even_g.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line1_even_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_even_g.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line1_even_b.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line1_even_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_even_b.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line1_odd_r.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line1_odd_r.point_sel = _REF_POINT_PREV;
	sPcid2Setting.pixel_ref.line1_odd_r.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line1_odd_g.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line1_odd_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_odd_g.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line1_odd_b.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line1_odd_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line1_odd_b.color_sel = _REF_COLOR_G;

	/* line 2 pixel setting */
	sPcid2Setting.pixel_ref.line2_even_r.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line2_even_r.point_sel = _REF_POINT_NEXT;
	sPcid2Setting.pixel_ref.line2_even_r.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line2_even_g.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line2_even_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line2_even_g.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line2_even_b.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line2_even_b.point_sel = _REF_POINT_NEXT;
	sPcid2Setting.pixel_ref.line2_even_b.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line2_odd_r.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line2_odd_r.point_sel = _REF_POINT_PREV;
	sPcid2Setting.pixel_ref.line2_odd_r.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line2_odd_g.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line2_odd_g.point_sel = _REF_POINT_NEXT;
	sPcid2Setting.pixel_ref.line2_odd_g.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line2_odd_b.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line2_odd_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line2_odd_b.color_sel = _REF_COLOR_G;

	/* line 3 pixel setting */
	sPcid2Setting.pixel_ref.line3_even_r.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line3_even_r.point_sel = _REF_POINT_PREV;
	sPcid2Setting.pixel_ref.line3_even_r.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line3_even_g.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line3_even_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_even_g.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line3_even_b.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line3_even_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_even_b.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line3_odd_r.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line3_odd_r.point_sel = _REF_POINT_PREV;
	sPcid2Setting.pixel_ref.line3_odd_r.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line3_odd_g.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line3_odd_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_odd_g.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line3_odd_b.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line3_odd_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line3_odd_b.color_sel = _REF_COLOR_G;

	/* line 4 pixel setting */
	sPcid2Setting.pixel_ref.line4_even_r.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line4_even_r.point_sel = _REF_POINT_NEXT;
	sPcid2Setting.pixel_ref.line4_even_r.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line4_even_g.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line4_even_g.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line4_even_g.color_sel = _REF_COLOR_R;

	sPcid2Setting.pixel_ref.line4_even_b.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line4_even_b.point_sel = _REF_POINT_NEXT;
	sPcid2Setting.pixel_ref.line4_even_b.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line4_odd_r.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line4_odd_r.point_sel = _REF_POINT_PREV;
	sPcid2Setting.pixel_ref.line4_odd_r.color_sel = _REF_COLOR_B;

	sPcid2Setting.pixel_ref.line4_odd_g.line_sel  = _REF_LINE_PREV;
	sPcid2Setting.pixel_ref.line4_odd_g.point_sel = _REF_POINT_NEXT;
	sPcid2Setting.pixel_ref.line4_odd_g.color_sel = _REF_COLOR_G;

	sPcid2Setting.pixel_ref.line4_odd_b.line_sel  = _REF_LINE_CURR;
	sPcid2Setting.pixel_ref.line4_odd_b.point_sel = _REF_POINT_CURR;
	sPcid2Setting.pixel_ref.line4_odd_b.color_sel = _REF_COLOR_G;

	drvif_color_pcid2_pixel_setting(&sPcid2Setting);

	//======================
	drvif_color_pcid_poltable();
}

void fwif_color_set_pcid_RgnWeight(void)
{
	DRV_pcid_valc_t sRgnWeight;
	unsigned int i=0;

	sRgnWeight.RgnWeightEnable = 1;
	sRgnWeight.InputSz = 1; // 0:3840x2160 1:1920x1080
	sRgnWeight.Sta_x = 0;
	sRgnWeight.Sta_y = 0;
	sRgnWeight.SftSel = 2;
	for(i=0; i<25; i++)
		sRgnWeight.HorWeight[i] = 8;
	for(i=0; i<10; i++)
		sRgnWeight.VerWeight[i] = 8;

	drvif_color_set_pcid_regional_weight_setting(&sRgnWeight);
}

void fwif_color_get_pcid_valuetable(char* pRetTbl, unsigned char ucTblIdx, unsigned char ucChannel)
{
	unsigned int EncodedTbl[81] = {0};
	int Row = 0, Col = 0;

	if( ucTblIdx > 1 )
		ucTblIdx = 1;
	if( ucChannel > 2 )
		ucChannel = 2;

	drvif_color_get_pcid_valuetable(EncodedTbl, ucTblIdx, ucChannel);

	for(Row=0; Row<9; Row++)
	{
		for(Col=0; Col<9; Col++)
		{
			if( Row<8 && Col<8 )
			{
				pRetTbl[Row*34   +Col*2  ] = (EncodedTbl[Row*9+Col] >> 24) & 0xff;
				pRetTbl[Row*34+17+Col*2  ] = (EncodedTbl[Row*9+Col] >> 16) & 0xff;
				pRetTbl[Row*34   +Col*2+1] = (EncodedTbl[Row*9+Col] >> 8 ) & 0xff;
				pRetTbl[Row*34+17+Col*2+1] = (EncodedTbl[Row*9+Col] >> 0 ) & 0xff;
			}
			else if( Row < 8 )
			{
				pRetTbl[Row*34   +Col*2  ] = (EncodedTbl[Row*9+Col] >> 24) & 0xff;
				pRetTbl[Row*34+17+Col*2  ] = (EncodedTbl[Row*9+Col] >> 16) & 0xff;
			}
			else if( Col < 8 )
			{
				pRetTbl[Row*34   +Col*2  ] = (EncodedTbl[Row*9+Col] >> 24) & 0xff;
				pRetTbl[Row*34   +Col*2+1] = (EncodedTbl[Row*9+Col] >> 8 ) & 0xff;
			}
			else
			{
				pRetTbl[Row*34   +Col*2  ] = (EncodedTbl[Row*9+Col] >> 24) & 0xff;
			}
		}
	}

}

void fwif_color_set_pcid_single_valuetable(char* pInputLUT, unsigned char ucTblIdx, unsigned char ucChannel)
{
	// Transfer the value to register value
	unsigned int Row = 0, Col = 0;
	unsigned int TblValue = 0;

	if( ucTblIdx > 1 )
		ucTblIdx = 1;
	if( ucChannel > 3 )
		ucChannel = 3;

	// Write the table when table is not the same
	for(Row=0; Row<9; Row++)
	{
		for(Col=0; Col<9; Col++)
		{
			//-------------------
			// Value1 | Value3 |
			//-------------------   will be transform to {Value1, Value2, Value3, Value4} {MSB, LSB}
			// Value2 | Value4 |
			//-------------------
			if( Row<8 && Col<8 )
			{
				TblValue =  ( ( pInputLUT[Row*34   +Col*2  ] & 0xff ) << 24 ) +
							( ( pInputLUT[Row*34+17+Col*2  ] & 0xff ) << 16 ) +
							( ( pInputLUT[Row*34   +Col*2+1] & 0xff ) << 8  ) +
							( ( pInputLUT[Row*34+17+Col*2+1] & 0xff ) << 0 );
			}
			else if( Row < 8 )
			{
				TblValue =  ( ( pInputLUT[Row*34   +Col*2  ] & 0xff ) << 24 ) +
							( ( pInputLUT[Row*34+17+Col*2  ] & 0xff ) << 16 ) +
							( ( pInputLUT[Row*34   +Col*2  ] & 0xff ) << 8  ) +
							( ( pInputLUT[Row*34+17+Col*2  ] & 0xff ) << 0 );
			}
			else if( Col < 8 )
			{
				TblValue =  ( ( pInputLUT[Row*34   +Col*2  ] & 0xff ) << 24 ) +
							( ( pInputLUT[Row*34   +Col*2  ] & 0xff ) << 16 ) +
							( ( pInputLUT[Row*34   +Col*2+1] & 0xff ) << 8  ) +
							( ( pInputLUT[Row*34   +Col*2+1] & 0xff ) << 0 );
			}
			else
			{
				TblValue =  ( ( pInputLUT[Row*34   +Col*2  ] & 0xff ) << 24 ) +
							( ( pInputLUT[Row*34   +Col*2  ] & 0xff ) << 16 ) +
							( ( pInputLUT[Row*34   +Col*2  ] & 0xff ) << 8  ) +
							( ( pInputLUT[Row*34   +Col*2  ] & 0xff ) << 0 );
			}
			PCID_ValueTBL[Row*9+Col] = TblValue;
		}
	}
	drvif_color_pcid2_valuetable(PCID_ValueTBL, ucTblIdx, ucChannel);
}

#endif// end of UT_flag

UINT8 TV006_WB_Pattern_En = 0;
unsigned char Get_tv006_wb_pattern(void)
{
	return TV006_WB_Pattern_En;
}
void Set_tv006_wb_pattern(unsigned char value)
{
	TV006_WB_Pattern_En=value;
}
void fwif_color_set_WB_Pattern_IRE(unsigned char ucType, unsigned char ucIRE)
{
	unsigned char ucY, ucCr, ucCb;
	short vY[5] =	{235,	235,	81,	145,	41};
	short vCb[5] =	{128,	128,	90,	54,	240};
	short vCr[5] =	{128,	128,	240,	34,	110};

	if (ucIRE > 100)
		ucIRE = 100;
	ucY = (ucIRE * (vY[ucType]-16) + 50) / 100 + 16;
	ucCb = (ucIRE * (vCb[ucType]-128) + 50) / 100 + 128;
	ucCr = (ucIRE * (vCr[ucType]-128) + 50) / 100 + 128;

	if (ucType) {
		Set_tv006_wb_pattern(1);//TV006_WB_Pattern_En = 1;
		fwif_color_set_osd_Outer(1);
		drvif_color_set_WB_pattern_RGB(ucCr, ucY, ucCb);
		drvif_color_set_WB_pattern_setup();
#ifdef CONFIG_HW_SUPPORT_MEMC
		//when no signal, enable MEMC out bg
		Scaler_MEMC_Booting_NoSignal_Check_OutputBG(_DISABLE);
#endif

	} else if(Get_tv006_wb_pattern()/*TV006_WB_Pattern_En*/){

		unsigned char b422Format = !(Scaler_DispGetInputInfo(SLR_INPUT_FRAMESYNC)) && Scaler_DispGetStatus(_MAIN_DISPLAY, SLR_DISP_422CAP);
		fwif_color_set_osd_Outer(1);
		if(dvrif_memory_compression_get_enable(SLR_MAIN_DISPLAY) == TRUE){
			drvif_color_set_WB_pattern_off(0, 0);
		}else{
			drvif_color_set_WB_pattern_off(0, b422Format);
		}
#ifdef CONFIG_HW_SUPPORT_MEMC
		//when no signal, disable MEMC out bg
		Scaler_MEMC_Booting_NoSignal_Check_OutputBG(_ENABLE);
#endif
		Set_tv006_wb_pattern(0);//TV006_WB_Pattern_En = 0;
	}


}

#ifndef UT_flag

/*=== ABOUT Histogram for HAL_DC === .....(start)*/
unsigned char fwif_color_get_HistoData_Countbins(UINT8 nBinNum, UINT32 *uCountBins)
{
	if (fwif_get_Y_Total_Hist_Cnt_distribution(1, TV006_VPQ_chrm_bin, uCountBins) == FALSE) {
		return FALSE;
	} else {
		if (nBinNum > TV006_VPQ_chrm_bin) {
			VIPprintf(" HistoBins not match! only %d", COLOR_HISTOGRAM_LEVEL);
		} else if ((nBinNum < TV006_VPQ_chrm_bin)) {
			return FALSE;
		}
		return TRUE;
	}
}

unsigned char fwif_color_get_HistoData_chrmbins(UINT8 nBinNum, UINT32 *uchrmbins)
{
	_clues *smartPic_clue = NULL;
	smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);

	if (smartPic_clue == NULL) {
		return FALSE;
	}

	if (nBinNum !=  COLOR_AutoSat_HISTOGRAM_LEVEL) {
		VIPprintf(" Sat HistoBins not match");
		return FALSE;
	} else {
		fwif_color_ChangeUINT32Endian_Copy(&smartPic_clue->Sat_Main_His[0], COLOR_AutoSat_HISTOGRAM_LEVEL, uchrmbins, 0);
		/*uchrmbins = &(smartPic_clue->Sat_Main_His);*/
	}

	return TRUE;

}

unsigned char fwif_color_get_HistoData_huebins(UINT8 nBinNum, UINT32 *uhuebins)
{
	_clues *smartPic_clue = NULL;
	smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);

	if (smartPic_clue == NULL) {
		return FALSE;
	}

	if (nBinNum !=  COLOR_HUE_HISTOGRAM_LEVEL) {
		VIPprintf(" Sat HueBins not match");
		return FALSE;
	} else {
		fwif_color_ChangeUINT32Endian_Copy(&smartPic_clue->Hue_Main_His[0], COLOR_HUE_HISTOGRAM_LEVEL, uhuebins, 0);
		/*uhuebins = smartPic_clue->Hue_Main_His;*/
	}

	return TRUE;
}

unsigned char fwif_color_get_HistoData_APL(UINT32 *uAPL)
{
	_clues *smartPic_clue = NULL;
	smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);

	if (smartPic_clue == NULL) {
		return FALSE;
	}

	fwif_color_ChangeUINT32Endian_Copy(&(smartPic_clue->Hist_Y_APL_Exact), 1, (unsigned int*)uAPL, 0);

	/**uAPL = smartPic_clue->Hist_Y_Mean_Value;*/

	return TRUE;
}

unsigned char fwif_color_get_HistoData_Min(signed int *uMin)
{
	_clues *smartPic_clue = NULL;
	smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);

	if (smartPic_clue == NULL) {
		return FALSE;
	}

	fwif_color_ChangeINT32Endian_Copy((int*)&(smartPic_clue->Hist_Peak_Dark_Pixel_value), 1, uMin, 0);

	/**uMin = smartPic_clue->Hist_Peak_Dark_IRE_Value;*/

	return TRUE;

}

unsigned char fwif_color_get_HistoData_Max(signed int *uMax)
{
	_clues *smartPic_clue = NULL;
	smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);

	if (smartPic_clue == NULL) {
		return FALSE;
	}

	fwif_color_ChangeINT32Endian_Copy((int*)&(smartPic_clue->Hist_Peak_White_Pixel_value), 1, uMax, 0);

	/**uMax = smartPic_clue->Hist_Peak_White_IRE_Value;*/

	return TRUE;

}

unsigned char fwif_color_get_HistoData_PeakLow(signed int *uPeakLow)
{
	_clues *smartPic_clue = NULL;
	smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);

	if (smartPic_clue == NULL) {
		return FALSE;
	}


	fwif_color_ChangeINT32Endian_Copy((int*)&(smartPic_clue->Hist_Peak_Dark_Pixel_value), 1, uPeakLow, 0);

	/**uPeakLow = smartPic_clue->Hist_Peak_Dark_Pixel_value;*/

	return TRUE;
}

unsigned char fwif_color_get_HistoData_PeakHigh(signed int *uPeakHigh)
{
	_clues *smartPic_clue = NULL;
	smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);

	if (smartPic_clue == NULL) {
		return FALSE;
	}

	fwif_color_ChangeINT32Endian_Copy((int*)&(smartPic_clue->Hist_Peak_White_Pixel_value), 1, uPeakHigh, 0);

	/**uPeakHigh = smartPic_clue->Hist_Peak_White_Pixel_value;*/

	return TRUE;

}

unsigned char fwif_color_get_HistoData_skinCount(UINT32 *uskinCount)
{
	_clues *smartPic_clue = NULL;
	UINT32 Hue2 = 0, Hue3 = 0, Hue4 = 0;

	smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);

	if (smartPic_clue == NULL) {
		return FALSE;
	}

	fwif_color_ChangeUINT32Endian_Copy(&(smartPic_clue->Hue_Main_His[2]), 1, &Hue2, 0);
	fwif_color_ChangeUINT32Endian_Copy(&(smartPic_clue->Hue_Main_His[3]), 1, &Hue3, 0);
	fwif_color_ChangeUINT32Endian_Copy(&(smartPic_clue->Hue_Main_His[4]), 1, &Hue4, 0);

	*uskinCount = (Hue2+Hue3+Hue4);

	return TRUE;

}

unsigned char fwif_color_get_HistoData_SatStatus(UINT32 *uSatStatus)
{
	_clues *smartPic_clue = NULL;
	smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);

	if (smartPic_clue == NULL) {
		return FALSE;
	}

	fwif_color_ChangeUINT32Endian_Copy(&(smartPic_clue->sat_ratio_mean_value), 1, uSatStatus, 0);

	/**uSatStatus = smartPic_clue->sat_ratio_mean_value;*/

	return TRUE;

}

unsigned char fwif_color_get_HistoData_diffSum(UINT32 *udiffSum)
{
	_clues *smartPic_clue = NULL;
	smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);

	if (smartPic_clue == NULL) {
		return FALSE;
	}

	/*fwif_color_ChangeUINT32Endian_Copy(&(smartPic_clue->Hist_diff_sum), 1, udiffSum, 0);	*/ /*=== marked by Elsie ===*/

	/**udiffSum = smartPic_clue->Hist_diff_sum;*/

	return TRUE;

}

unsigned char fwif_color_get_HistoData_motion(UINT32 *umotion)
{
	_clues *smartPic_clue = NULL;
	smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);

	if (smartPic_clue == NULL) {
		return FALSE;
	}

	fwif_color_ChangeUINT32Endian_Copy(&(smartPic_clue->RTNR_MAD_count_Y_avg_ratio), 1, umotion, 0);

	/**umotion = smartPic_clue->RTNR_MAD_count_Y_avg_ratio;*/

	return TRUE;

}


unsigned char fwif_color_get_HistoData_texture(UINT32 *utexture)
{
#if 0 // no implement
	UINT32 TSH_Step_Hist[20];
	_clues *smartPic_clue = NULL;
	smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);

	if (smartPic_clue == NULL) {
		return FALSE;
	}

	fwif_color_ChangeUINT32Endian_Copy(&(smartPic_clue->TSH_Step_Hist[0]), 20, &TSH_Step_Hist[0], 0);


	*utexture = TSH_Step_Hist[0]+TSH_Step_Hist[1]+TSH_Step_Hist[2]+TSH_Step_Hist[3]+TSH_Step_Hist[4];
#endif
	return TRUE;

}
/*=== ABOUT Histogram for HAL_DC === .....(end)*/



/*=== ABOUT DCC for HAL_DC === .....(start)*/
unsigned char  fwif_color_set_dcc_Curve_Write(unsigned char display, UINT8 CurveNum, signed int *sAccLUT)
{
	static int DCC_Curve_BASE[TableSeg_num_Max], DCC_Curve_G0[TableSeg_num_Max], DCC_Curve_G1[TableSeg_num_Max];
	static unsigned int dcc_histogram_to_write[TableSeg_num_Max];

	int i = 0;
	UINT32 	INTERP_DATA = 0;
	/*unsigned int CURVE_LUT[64];*/
	int Curve_data_range = 1 << DCC_Curve_DataRang_bit;
	int Curve_segment_range = 1 << DCC_Curve_Segment_bit;
	unsigned char bit_shift = DCC_Curve_DataRang_bit-2;

	_system_setting_info *VIP_system_info_structure_table=NULL;
	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	for (i = 0; i < (TableSeg_num_Max); i++) {
		/*for 32 DCC segment from MacArthur*/
		if (i == 0) {
			DCC_Curve_BASE[i] = (sAccLUT[0] + Curve_data_range)%Curve_data_range;
		} else {
			DCC_Curve_BASE[i] = sAccLUT[i << 1]%Curve_data_range;
		}
		DCC_Curve_G0[i] = (sAccLUT[(i << 1) + 1] - sAccLUT[i << 1])%Curve_segment_range;
		DCC_Curve_G1[i] = (sAccLUT[(i << 1)+2] - sAccLUT[(i << 1) + 1])%Curve_segment_range;
	}

	for (i = 0; i < (TableSeg_num_Max); i++) {
		INTERP_DATA = ((DCC_Curve_BASE[i] << (bit_shift<<1)) + (DCC_Curve_G0[i] << bit_shift) + DCC_Curve_G1[i]); //Mac6 --> 10bit
		//INTERP_DATA = ((DCC_Curve_BASE[i] << 20) + (DCC_Curve_G0[i] << 10) + DCC_Curve_G1[i]);/*Magellan2 --> 10bit to 12bit*/
		dcc_histogram_to_write[i] = INTERP_DATA;/*move to D - domain ISR to write DCC table!! Reduce Write timing miss match!!(histogram_new_dcc_write_table())*/
		/*VIPprintf("[%d] = >%d, %d, %d\n", i, DCC_Curve_BASE[i], DCC_Curve_G0[i], DCC_Curve_G1[i]);*/
		/*VIPprintf("dcc_histogram_to_write[%d] = %x\n", i, dcc_histogram_to_write[i]);*/
	}

	return drvif_color_dcc_Curve_Write(display, TableSeg_num_Max, dcc_histogram_to_write);
}
void fwif_color_inv_gamma_init()
{
	drvif_color_inv_gamma_init();
}

unsigned char fwif_color_get_FreshContrastBypassLUT(unsigned char Curve_Seg, signed int *pDCBypassLUT)
{
	_clues *smartPic_clue = NULL;

	if (Curve_Seg !=  DCC_Curve_Node_MAX) {
		VIPprintf("Curve Segment not match!!");
		return FALSE;
	}

	smartPic_clue = (_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SMARTPIC_CLUS);

	if (smartPic_clue == NULL) {
		VIPprintf("smartPic_clue NULL!!!");
		return FALSE;
	}

	fwif_color_ChangeINT32Endian_Copy(&((smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.ByPass_Curve)[0]), DCC_Curve_Node_MAX, pDCBypassLUT, 0);

	return TRUE;

}
/*=== ABOUT DCC for HAL_DC === .....(end)*/

void fwif_color_set_LDSetLUT(UINT8 lutTableIndex)
{
	/*	lutTableIndex = 0 -> off*/
	/*	lutTableIndex = 1 -> SDR_Low*/
	/*	lutTableIndex = 2 -> SDR_Mid*/
	/*	lutTableIndex = 3 -> SDR_Hiigh*/
	/*	lutTableIndex = 4 -> HDR_Low*/
	/*	lutTableIndex = 5 -> HDR_Mid*/
	/*	lutTableIndex = 6 -> HDR_Hiigh*/

	rtd_pr_vpq_debug("lutTableIndex = %d\n", lutTableIndex);

	if (lutTableIndex >= LD_Table_NUM)
		lutTableIndex = 1; /* parameter protection*/


	fwif_color_set_LD_Backlight_Decision(0, lutTableIndex);
	fwif_color_set_LD_Spatial_Filter(0, lutTableIndex);
	fwif_color_set_LD_Temporal_Filter(0, lutTableIndex);
	fwif_color_set_LD_Spatial_Remap(0, lutTableIndex);
	//fwif_color_set_LD_Data_Compensation(0, lutTableIndex);
	fwif_color_set_LD_Data_Compensation_NewMode_2DTable(0, lutTableIndex);

}

bool fwif_color_set_LED_Initialize(HAL_LED_PANEL_INFO_T panelInfo)
{
	bool pass = 0;

	unsigned short dispWidth = 3840;
	unsigned short dispHeight = 2160;

	/*rtd_pr_vpq_debug("ic=%d, bl_type=%d, bar_type=%d\n",panelInfo.hal_icType,panelInfo.hal_bl_type,panelInfo.hal_bar_type);*/

	if (panelInfo.hal_icType == 1) {
		if (panelInfo.hal_bl_type == HAL_BL_EDGE_LED) {
			blu_ld_global_ctrl0_RBUS blu_ld_global_ctrl0;
			blu_ld_global_ctrl1_RBUS blu_ld_global_ctrl1;
			blu_ld_global_ctrl2_RBUS blu_ld_global_ctrl2;
			blu_ld_global_ctrl3_RBUS blu_ld_global_ctrl3;
			blu_ld_backlight_ctrl1_RBUS blu_ld_backlight_ctrl1; /* for control spi total*/
			blu_ld_inter_hctrl1_RBUS blu_ld_inter_hctrl1;/* for ld_hfactor*/
			blu_ld_inter_vctrl1_RBUS blu_ld_inter_vctrl1;/* for ld_vfactor*/

			pass = 1;

			blu_ld_global_ctrl0.regValue = IoReg_Read32(BLU_LD_Global_Ctrl0_reg);
			blu_ld_global_ctrl1.regValue = IoReg_Read32(BLU_LD_Global_Ctrl1_reg);
			blu_ld_global_ctrl2.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);
			blu_ld_global_ctrl3.regValue = IoReg_Read32(BLU_LD_Global_Ctrl3_reg);
			blu_ld_backlight_ctrl1.regValue = IoReg_Read32(BLU_LD_Backlight_Ctrl1_reg);
			blu_ld_inter_hctrl1.regValue = IoReg_Read32(BLU_LD_Inter_HCtrl1_reg);
			blu_ld_inter_vctrl1.regValue = IoReg_Read32(BLU_LD_Inter_VCtrl1_reg);

			blu_ld_global_ctrl2.ld_blk_type = 1;

			if (panelInfo.hal_bar_type == HAL_LED_BAR_6)
				blu_ld_global_ctrl2.ld_blk_hnum = 5;/*LD_Global_Ctrl_Table[5];*/
			else if (panelInfo.hal_bar_type == HAL_LED_BAR_12)
				blu_ld_global_ctrl2.ld_blk_hnum = 11;
			else if (panelInfo.hal_bar_type == HAL_LED_BAR_16)
				blu_ld_global_ctrl2.ld_blk_hnum = 15;
			else if (panelInfo.hal_bar_type == HAL_LED_BAR_20)
				blu_ld_global_ctrl2.ld_blk_hnum = 19;
			else if (panelInfo.hal_bar_type == HAL_LED_BAR_240)  /*edge not support*/
				pass = 0;
			else
				pass = 0;

			blu_ld_global_ctrl2.ld_blk_vnum = 0;	/*	one - sided edge*/ /*LD_Global_Ctrl_Table[6];*/
			blu_ld_global_ctrl3.ld_blk_hsize = dispWidth/(blu_ld_global_ctrl2.ld_blk_hnum + 1);/*LD_Global_Ctrl_Table[13];*/
			blu_ld_global_ctrl3.ld_blk_vsize = dispHeight/(blu_ld_global_ctrl2.ld_blk_vnum + 1);/*LD_Global_Ctrl_Table[14];*/
			blu_ld_backlight_ctrl1.ld_blu_spitotal = 2 * (blu_ld_global_ctrl2.ld_blk_hnum + 1) - 1;
			blu_ld_inter_hctrl1.ld_hfactor = 1048576*(blu_ld_inter_hctrl1.ld_tab_hsize+1)*(blu_ld_global_ctrl2.ld_blk_hnum+1)/dispWidth;
			blu_ld_inter_vctrl1.ld_vfactor = 1048576*(blu_ld_inter_vctrl1.ld_tab_vsize)/dispHeight;


			IoReg_Write32(BLU_LD_Global_Ctrl0_reg, blu_ld_global_ctrl0.regValue);
			IoReg_Write32(BLU_LD_Global_Ctrl1_reg, blu_ld_global_ctrl1.regValue);
			IoReg_Write32(BLU_LD_Global_Ctrl2_reg, blu_ld_global_ctrl2.regValue);
			IoReg_Write32(BLU_LD_Global_Ctrl3_reg, blu_ld_global_ctrl3.regValue);
			IoReg_Write32(BLU_LD_Backlight_Ctrl1_reg, blu_ld_backlight_ctrl1.regValue);
			IoReg_Write32(BLU_LD_Inter_HCtrl1_reg, blu_ld_inter_hctrl1.regValue);
			IoReg_Write32(BLU_LD_Inter_VCtrl1_reg, blu_ld_inter_vctrl1.regValue);
		}
	}

	if (!pass)
		drvif_HAL_VPQ_LED_LDEnable(0);
	return pass;
}




unsigned char LD_CMD;
void fwif_color_set_LD_CtrlSPI_init(HAL_LED_PANEL_INFO_T panelInfo)
{
	int i;
	unsigned char ucld_blk_hnum, ucld_blk_vnum = 0;

	blu_ld_global_ctrl2_RBUS blu_ld_global_ctrl2;

	ldspi_ld_ctrl_RBUS ld_ctrl;
	ldspi_out_data_ctrl_RBUS out_data_ctrl;
	//ldspi_timing_ctrl_RBUS timing_ctrl;
	//ldspi_send_delay_RBUS send_delay;
	//ldspi_data_unit_delay_RBUS data_unit_delay;
	//ldspi_v_sync_duty_RBUS v_sync_duty;
	//ldspi_data_h_time_RBUS data_h_time;
	//ldspi_cs_hold_time_RBUS cs_hold_time;

	ldspi_outindex_addr_ctrl_RBUS ldspi_outindex_addr_ctrl;
	ldspi_outindex_rwport_RBUS ldspi_outindex_rwport;
	ldspi_data_addr_ctrl_RBUS ldspi_data_addr_ctrl;
	ldspi_data_rwport_RBUS ldspi_data_rwport;

	blu_ld_global_ctrl2.regValue = IoReg_Read32(BLU_LD_Global_Ctrl2_reg);

	ucld_blk_hnum = blu_ld_global_ctrl2.ld_blk_hnum;
	ucld_blk_vnum = blu_ld_global_ctrl2.ld_blk_vnum;


	ld_ctrl.regValue = IoReg_Read32(LDSPI_LD_CTRL_reg);
	out_data_ctrl.regValue = IoReg_Read32(LDSPI_Out_data_CTRL_reg);


	ld_ctrl.start_enable = 0;/*Ld_spi disable*/
	IoReg_Write32(LDSPI_LD_CTRL_reg, ld_ctrl.regValue);

	ld_ctrl.ld_mode = 1;
	ld_ctrl.ld_spi_en = 1;
	ld_ctrl.ld_pin_port_sel = 1;
	ld_ctrl.separate_two_block = 0;

	out_data_ctrl.data_endian = 1;
	out_data_ctrl.output_units = ucld_blk_hnum+2;/*0x49;*/
	out_data_ctrl.output_data_format = 0;


	IoReg_Write32(LDSPI_LD_CTRL_reg, ld_ctrl.regValue);
	IoReg_Write32(LDSPI_Out_data_CTRL_reg, out_data_ctrl.regValue);

	/* set index sram*/
	ldspi_outindex_addr_ctrl.regValue = IoReg_Read32(LDSPI_Outindex_Addr_CTRL_reg);
	ldspi_outindex_addr_ctrl.outindex_rw_pointer_set = 0x0;
	IoReg_Write32(LDSPI_Outindex_Addr_CTRL_reg, ldspi_outindex_addr_ctrl.regValue);
	ldspi_outindex_rwport.outindex_rw_data_port = 0x7e; /*cmd*/
	IoReg_Write32(LDSPI_Outindex_Rwport_reg, ldspi_outindex_rwport.regValue);
	ldspi_outindex_rwport.outindex_rw_data_port = 0x7f; /*ID*/
	IoReg_Write32(LDSPI_Outindex_Rwport_reg, ldspi_outindex_rwport.regValue);

	if (ucld_blk_vnum == 0) /*1-sided*/
		for (i = 2*ucld_blk_hnum; i >= 0; i = i - 2) {
			ldspi_outindex_rwport.outindex_rw_data_port = i;
			IoReg_Write32(LDSPI_Outindex_Rwport_reg, ldspi_outindex_rwport.regValue);
		}
	else /*2-sided*/
		for (i = 2*ucld_blk_hnum+1; i >= 0; i--) {
			ldspi_outindex_rwport.outindex_rw_data_port = i;
			IoReg_Write32(LDSPI_Outindex_Rwport_reg, ldspi_outindex_rwport.regValue);
		}

	/* set data sram for cmd and id*/
	ldspi_data_addr_ctrl.regValue = IoReg_Read32(LDSPI_Data_Addr_CTRL_reg);
	ldspi_data_addr_ctrl.data_rw_pointer_set = 0x7e;
	IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Modify data in data SRAM for cmd and id*/
	ldspi_data_rwport.data_rw_data_port = 0xaa;
	IoReg_Write32(LDSPI_Data_Rwport_reg, ldspi_data_rwport.regValue); /*ID*/
	ldspi_data_rwport.data_rw_data_port = 0x81;
	IoReg_Write32(LDSPI_Data_Rwport_reg, ldspi_data_rwport.regValue); /*Cmd*/
	LD_CMD = 0x81;

	/* Init data in data SRAM*/
	ldspi_data_addr_ctrl.data_rw_pointer_set = 0x0;
	IoReg_Write32(LDSPI_Data_Addr_CTRL_reg, ldspi_data_addr_ctrl.regValue); /*Modify data in data SRAM for cmd and id*/
	ldspi_data_rwport.data_rw_data_port = 0x99;
	for (i = 0; i < 64; i++) {		/*in Mac3 data sram size is 64 + header size = 128, clear data part.*/

		IoReg_Write32(LDSPI_Data_Rwport_reg, ldspi_data_rwport.regValue);
	}

	ld_ctrl.send_follow_vsync = 1; /*Send_follow_Vsync*/
	ld_ctrl.start_enable = 1;/*Ld_spi enable*/
	IoReg_Write32(LDSPI_LD_CTRL_reg, ld_ctrl.regValue);

}

void fwif_color_vpq_pic_init()
{
#ifndef BUILD_QUICK_SHOW

	if (fwif_VIP_get_Project_ID() == VIP_Project_ID_TV006) {
		fwif_color_SetDNR_tv006(fwif_color_GetDNR_tv006());
		fwif_color_Set_MPEGNR_tv006(fwif_color_MPEGDNR_tv006());
	} else if(fwif_VIP_get_Project_ID() == VIP_Project_ID_TV010){
		if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE)
			Scaler_SetPictureMode_tv010(fwif_color_get_picture_mode(Scaler_GetUserInputSrc(SLR_MAIN_DISPLAY)));
	}else{
		Scaler_RefreshPictureMode();
	}
#endif
}

void fwif_color_inv_gamma_control_front(unsigned char display)
{
	drvif_color_inv_gamma_control_front(display);
}

void fwif_color_inv_gamma_control_back(unsigned char display, unsigned char ucWriteEnable)
{
	drvif_color_inv_gamma_control_back(display, ucWriteEnable);
}

void fwif_color_Inv_Gamma_Curve_Write(unsigned int *Gamma_Encode, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx)
{
	drvif_color_Inv_Gamma_Curve_Write(Gamma_Encode, RGB_chanel_idx);
}

void fwif_color_set_Inv_gamma_encode(void)
{
	fwif_color_Inv_Gamma_Curve_Write(t_inv_GAMMA, GAMMA_CHANNEL_R);	/*R*/
	fwif_color_Inv_Gamma_Curve_Write(t_inv_GAMMA, GAMMA_CHANNEL_G);	/*G*/
	fwif_color_Inv_Gamma_Curve_Write(t_inv_GAMMA, GAMMA_CHANNEL_B);	/*B*/
}

void fwif_color_set_Inv_gamma_DEMO_TV006(void)
{
	static UINT16 demo_inv_gamma[Bin_Num_Gamma + 1] = {0};
	static unsigned short TV006DEMO_InvGamma[1025] = {
		0, 22, 72, 107, 147, 201, 241, 298, 337, 387, 433, 472, 528, 567, 620, 659, 696, 750, 787, 831, 875, 911, 963, 997, 1031, 1082, 1115, 1148, 1198, 1231, 1263, 1311, 1343, 1374, 1421, 1452, 1481, 1529, 1558, 1588, 1634, 1662, 1691, 1728, 1764, 1792, 1819, 1864, 1890, 1918, 1953, 1987, 2013, 2039, 2082, 2107, 2133, 2158, 2200, 2224, 2249, 2275, 2315, 2338, 2362,
		2388, 2426, 2449, 2473, 2496, 2536, 2558, 2580, 2602, 2642, 2664, 2686, 2711, 2746, 2767, 2789, 2809, 2847, 2868, 2889, 2910, 2934, 2968, 2988, 3008, 3028, 3058, 3085, 3105, 3124, 3143, 3176, 3199, 3218, 3237, 3256, 3288, 3310, 3329, 3348, 3366, 3394, 3419, 3437, 3456, 3474, 3495, 3526, 3543, 3561, 3579, 3595, 3628, 3647, 3664, 3681, 3698, 3718, 3749, 3765, 3782, 3798, 3814, 3840,
		3865, 3880, 3897, 3913, 3928, 3955, 3978, 3993, 4009, 4025, 4039, 4064, 4088, 4103, 4118, 4134, 4148, 4168, 4196, 4210, 4225, 4240, 4255, 4270, 4299, 4316, 4330, 4345, 4359, 4373, 4392, 4419, 4433, 4447, 4461, 4475, 4488, 4511, 4534, 4547, 4561, 4575, 4589, 4601, 4624, 4646, 4659, 4672, 4686, 4699, 4712, 4732, 4756, 4768, 4781, 4794, 4807, 4820, 4834, 4861, 4875, 4888, 4901, 4913,
		4926, 4938, 4959, 4980, 4992, 5005, 5017, 5029, 5041, 5053, 5077, 5095, 5106, 5118, 5131, 5143, 5154, 5166, 5190, 5207, 5218, 5230, 5242, 5253, 5265, 5276, 5297, 5317, 5327, 5339, 5350, 5362, 5373, 5383, 5399, 5422, 5434, 5445, 5456, 5467, 5478, 5491, 5515, 5528, 5539, 5550, 5560, 5571, 5582, 5592, 5607, 5630, 5642, 5652, 5662, 5673, 5684, 5694, 5704, 5718, 5740, 5753, 5762, 5773,
		5783, 5793, 5804, 5813, 5824, 5845, 5861, 5870, 5881, 5891, 5901, 5911, 5921, 5930, 5945, 5966, 5977, 5986, 5996, 6006, 6016, 6025, 6035, 6044, 6061, 6080, 6090, 6099, 6109, 6118, 6128, 6137, 6147, 6155, 6172, 6191, 6201, 6210, 6219, 6228, 6238, 6247, 6256, 6264, 6277, 6296, 6310, 6318, 6327, 6336, 6345, 6354, 6363, 6372, 6380, 6396, 6415, 6424, 6433, 6441, 6450, 6459, 6468, 6477,
		6485, 6494, 6510, 6527, 6537, 6545, 6553, 6562, 6570, 6579, 6588, 6596, 6604, 6618, 6636, 6647, 6654, 6663, 6671, 6679, 6688, 6696, 6704, 6712, 6722, 6739, 6754, 6762, 6770, 6778, 6786, 6794, 6802, 6810, 6818, 6826, 6837, 6854, 6867, 6875, 6883, 6891, 6898, 6906, 6914, 6922, 6930, 6937, 6947, 6964, 6978, 6986, 6993, 7001, 7008, 7016, 7024, 7031, 7039, 7046, 7054, 7068, 7085, 7094,
		7101, 7108, 7116, 7123, 7131, 7138, 7146, 7153, 7160, 7169, 7184, 7199, 7207, 7214, 7221, 7229, 7236, 7243, 7251, 7258, 7265, 7272, 7280, 7294, 7309, 7318, 7325, 7332, 7339, 7346, 7353, 7360, 7367, 7374, 7381, 7388, 7399, 7414, 7426, 7433, 7440, 7447, 7453, 7460, 7467, 7474, 7481, 7488, 7494, 7501, 7514, 7528, 7539, 7546, 7552, 7559, 7566, 7572, 7579, 7586, 7593, 7599, 7605, 7612,
		7623, 7637, 7649, 7656, 7662, 7669, 7675, 7682, 7689, 7695, 7702, 7708, 7715, 7720, 7728, 7741, 7755, 7764, 7770, 7776, 7783, 7789, 7796, 7802, 7808, 7815, 7821, 7827, 7833, 7840, 7853, 7867, 7876, 7882, 7888, 7894, 7900, 7907, 7913, 7919, 7925, 7931, 7938, 7943, 7950, 7960, 7974, 7985, 7992, 7997, 8003, 8009, 8015, 8021, 8027, 8033, 8039, 8046, 8051, 8057, 8064, 8076, 8088, 8098,
		8104, 8110, 8116, 8122, 8127, 8133, 8139, 8145, 8150, 8156, 8165, 8178, 8190, 8196, 8200, 8204, 8208, 8212, 8217, 8221, 8226, 8230, 8235, 8239, 8243, 8248, 8252, 8257, 8261, 8265, 8270, 8278, 8288, 8298, 8305, 8310, 8314, 8319, 8323, 8328, 8332, 8337, 8342, 8346, 8351, 8356, 8360, 8365, 8370, 8374, 8378, 8384, 8394, 8404, 8414, 8420, 8425, 8429, 8433, 8438, 8443, 8448, 8453, 8458,
		8463, 8467, 8472, 8477, 8482, 8486, 8491, 8498, 8508, 8519, 8528, 8534, 8538, 8543, 8548, 8553, 8558, 8563, 8568, 8573, 8578, 8583, 8588, 8593, 8598, 8603, 8610, 8620, 8631, 8641, 8646, 8651, 8656, 8661, 8666, 8672, 8677, 8682, 8687, 8693, 8698, 8703, 8708, 8713, 8719, 8729, 8740, 8751, 8757, 8762, 8767, 8773, 8778, 8783, 8789, 8794, 8800, 8805, 8811, 8816, 8821, 8826, 8833, 8845,
		8856, 8866, 8872, 8876, 8882, 8887, 8893, 8899, 8904, 8910, 8916, 8921, 8927, 8932, 8937, 8944, 8955, 8968, 8977, 8984, 8989, 8994, 9000, 9006, 9011, 9017, 9023, 9029, 9035, 9040, 9046, 9051, 9060, 9072, 9085, 9093, 9098, 9104, 9109, 9115, 9121, 9127, 9133, 9139, 9145, 9151, 9157, 9162, 9170, 9183, 9196, 9203, 9207, 9210, 9212, 9215, 9219, 9222, 9226, 9229, 9233, 9236, 9240, 9243,
		9247, 9250, 9254, 9257, 9261, 9264, 9267, 9271, 9274, 9278, 9284, 9291, 9299, 9307, 9313, 9317, 9320, 9323, 9327, 9330, 9334, 9337, 9341, 9345, 9348, 9352, 9356, 9359, 9363, 9367, 9370, 9374, 9378, 9381, 9384, 9388, 9393, 9400, 9408, 9417, 9423, 9428, 9432, 9435, 9438, 9442, 9446, 9450, 9454, 9457, 9461, 9465, 9469, 9473, 9476, 9480, 9484, 9488, 9491, 9495, 9499, 9503, 9510, 9518,
		9527, 9534, 9539, 9543, 9547, 9550, 9554, 9558, 9562, 9566, 9570, 9574, 9578, 9582, 9586, 9590, 9594, 9598, 9602, 9605, 9609, 9613, 9619, 9628, 9636, 9645, 9651, 9655, 9658, 9662, 9666, 9670, 9674, 9678, 9682, 9686, 9691, 9695, 9699, 9703, 9707, 9711, 9715, 9719, 9723, 9728, 9736, 9745, 9754, 9761, 9766, 9769, 9773, 9777, 9781, 9786, 9790, 9794, 9798, 9803, 9807, 9811, 9815, 9820,
		9824, 9828, 9832, 9836, 9842, 9851, 9860, 9869, 9875, 9879, 9883, 9887, 9892, 9896, 9900, 9905, 9909, 9913, 9918, 9922, 9927, 9931, 9935, 9939, 9943, 9948, 9954, 9963, 9973, 9982, 9988, 9992, 9996, 10000, 10005, 10009, 10014, 10018, 10023, 10027, 10032, 10036, 10041, 10045, 10050, 10054, 10058, 10064, 10072, 10082, 10092, 10099, 10103, 10107, 10112, 10116, 10121, 10126, 10130, 10135, 10139, 10144, 10149, 10153,
		10158, 10163, 10167, 10171, 10178, 10187, 10197, 10207, 10212, 10216, 10220, 10228, 10240, 10251, 10262, 10273, 10283, 10301, 10322, 10333, 10343, 10355, 10366, 10377, 10387, 10399, 10421, 10438, 10448, 10459, 10470, 10482, 10493, 10503, 10518, 10542, 10554, 10565, 10576, 10588, 10599, 10610, 10621, 10642, 10661, 10672, 10683, 10694, 10706, 10717, 10728, 10744, 10768, 10780, 10791, 10802, 10814, 10825, 10836, 10849, 10873, 10888, 10899, 10911,
		10923, 10934, 10948, 10972, 10986, 10997, 11009, 11021, 11032, 11044, 11056, 11081, 11096, 11107, 11119, 11131, 11143, 11155, 11167, 11191, 11208, 11219, 11231, 11242, 11261, 11289, 11324, 11347, 11370, 11394, 11433, 11456, 11479, 11502, 11543, 11565, 11589, 11611, 11652, 11675, 11699, 11721, 11762, 11785, 11809, 11832, 11873, 11896, 11920, 11943, 11983, 12007, 12031, 12054, 12095, 12119, 12143, 12166, 12207, 12231, 12255, 12279, 12320, 16384,
	};

	//set inv gamma
	//down(&VPQ_Semaphore);
	fwif_color_inv_gamma_control_front(SLR_MAIN_DISPLAY);
	fwif_color_gamma_pre_encode_TV006_HDR_demo(demo_inv_gamma, Power22InvGamma, TV006DEMO_InvGamma);
	fwif_color_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B, demo_inv_gamma, demo_inv_gamma, demo_inv_gamma);
	fwif_color_set_inv_gamma_encode_tv006();
	drvif_color_inv_gamma_control_back_TV006_demo();
	//up(&VPQ_Semaphore);

	//set gamma
	//main no change
	//sub disable
	drvif_color_gamma_enable(SLR_SUB_DISPLAY, 0);
}

void fwif_color_reset_invGamma_tv006(unsigned char value)
{
	fwif_color_inv_gamma_control_front(SLR_MAIN_DISPLAY);
	fwif_color_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B, Power22InvGamma, Power22InvGamma, Power22InvGamma);
	fwif_color_set_inv_gamma_encode_tv006();
	fwif_color_inv_gamma_control_back(SLR_MAIN_DISPLAY, 1);
	drvif_color_inv_gamma_init();
}

void fwif_color_out_gamma_control_front(void)
{
	drvif_color_out_gamma_control_front();
}

void fwif_color_out_gamma_control_back(void)
{
	drvif_color_out_gamma_control_back();
}

void fwif_color_out_gamma_control_enable(unsigned char enable)
{
	drvif_color_out_gamma_control_enable(enable);
}

void fwif_color_Out_Gamma_Curve_Write(unsigned int *Gamma_Encode, VIP_GAMMA_CHANNEL_RGB RGB_chanel_idx)
{
	drvif_color_Out_Gamma_Curve_Write(Gamma_Encode, RGB_chanel_idx);
}

void fwif_color_set_xvYCC_flag()
{
	SLR_VOINFO *pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
	HDMI_COLORIMETRY_T HDMI_COLORIMETRY_MODE = drvif_Hdmi_GetColorimetry();

	/* set xvYcc Status in system infomation*/
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if ((pVOInfo->xvYCC == 1) || (HDMI_COLORIMETRY_MODE == HDMI_COLORIMETRY_XYYCC601) || (HDMI_COLORIMETRY_MODE == HDMI_COLORIMETRY_XYYCC709)) {
		VIP_system_info_structure_table->xvYcc_auto_mode_flag = _ENABLE;
	} else {
		VIP_system_info_structure_table->xvYcc_auto_mode_flag = _DISABLE;
	}


}

void fwif_color_xvYcc_colorspaceyuv2rgbtransfer(unsigned char display, unsigned char nSrcTyp, unsigned char xvYcc_en_flag)
{
	drvif_color_xvYcc_colorspaceyuv2rgbtransfer(display, xvYcc_en_flag);
}

#if 1
void fwif_color_gamma_remmping_for_xvYcc(unsigned char src_idx, UINT16 *GOut_R, UINT16 *GOut_G, UINT16 *GOut_B)
{
	/* Get xvYcc auto mode Status in system infomation	and set xvYcc_flag*/
	SLR_PICTURE_MODE_DATA *pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	UINT16 i;
	int temp_gamma = 0;
	UINT32 out_low_index = 0, out_high_index = 0;
	UINT32 w = 0, /*z = 0,*/ out_cen = 0, temp_low = 0, temp_high = 0, temp_out = 0;
/*	int max = pow(2, bit);*/

	if (pData == NULL)
		return;

	/*VIPprintf("10 bit value -> %d, %d\n", (pow(2, 10)), (1 << 10));*/

	for (i = 0; i <= Bin_Num_Gamma; i++) {
		temp_gamma = t_gamma_curve_22[i];
		out_cen = (UINT32)temp_gamma;

		if (temp_gamma == 0)
			out_low_index  = ((temp_gamma << 10))>>12;
		else
			out_low_index  = ((temp_gamma << 10)-2048)>>12;

		out_high_index = ((temp_gamma << 10)+2048)>>12;

		/*VIPprintf("[%d], %lf, %d, %d\n", i, out_cen, out_low_index, out_high_index);*/

		w = (((out_cen*1024)/4096)-(UINT32)out_low_index);
		/*w = 1.0/2.0;*/
		if (i%50 == 0) {
			VIPprintf("w = %d, %d  %d \n", w, ((out_cen*1023)/4096), out_low_index);
		}
		temp_low = (UINT32)GOut_R[out_low_index];
		temp_high = (UINT32)GOut_R[out_high_index];
		temp_out  = ((1-w)*temp_low + w*temp_high);
		GOut_R[i] = (UINT16)temp_out;

		if (i%50 == 0) {
			VIPprintf("R:[%d] %d, %d\n", i, temp_out, GOut_R[i]);
		}

		temp_low = (UINT32)GOut_G[out_low_index];
		temp_high = (UINT32)GOut_G[out_high_index];
		temp_out  = ((1-w)*temp_low + w*temp_high);
		GOut_G[i] = (UINT16)temp_out;

		if (i%50 == 0) {
			VIPprintf("G:[%d]%d, %d\n", i, temp_out, GOut_G[i]);
		}

		temp_low = (UINT32)GOut_B[out_low_index];
		temp_high = (UINT32)GOut_B[out_high_index];
		temp_out  = ((1-w)*temp_low + w*temp_high);
		GOut_B[i] = (UINT16)temp_out;

		if (i%50 == 0) {
			VIPprintf("B:[%d]%d, %d\n", i, temp_out, GOut_B[i]);
		}
		/*w =  ((out_cen << 18))>>12-(((out_cen << 10)-2048)>>12)<< 8;*/
		/*VIPprintf("[%d] %d, %d, %d\n", i, GOut_R[i], GOut_G[i], GOut_B[i]);*/
	}
}
#else
void fwif_color_gamma_remmping_for_xvYcc(unsigned char src_idx, UINT16 *GOut_R, UINT16 *GOut_G, UINT16 *GOut_B)
{
	/* Get xvYcc auto mode Status in system infomation	and set xvYcc_flag*/
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	SLR_PICTURE_MODE_DATA *pData = (SLR_PICTURE_MODE_DATA *)fwif_color_get_cur_picture_mode_data(src_idx);
	if (pData == NULL)
		return;

	if (((pData->xvYcc_Mode) == SLR_xvYcc_OFF) || (((pData->xvYcc_Mode) == SLR_xvYcc_Auto) && (VIP_system_info_structure_table->xvYcc_auto_mode_flag == 0))) {
		VIPprintf("xvYcc off!!");
		return;
	}

	UINT16 i;
	int temp_gamma = 0;
	UINT32 out_low_index = 0, out_high_index = 0;
	FLOAT32 w = 0, z = 0, out_cen = 0, temp_low = 0, temp_high = 0, temp_out = 0;
/*	int max = pow(2, bit);*/

	/*VIPprintf("10 bit value -> %d, %d\n", (pow(2, 10)), (1 << 10));*/

	for (i = 0; i <= Bin_Num_Gamma; i++) {
		temp_gamma = t_gamma_curve_22[i];
		out_cen = (FLOAT32)temp_gamma;

		if (temp_gamma == 0)
			out_low_index  = ((temp_gamma << 10))>>12;
		else
			out_low_index  = ((temp_gamma << 10)-2048)>>12;

		out_high_index = ((temp_gamma << 10)+2048)>>12;

		/*VIPprintf("[%d], %lf, %d, %d\n", i, out_cen, out_low_index, out_high_index);*/

		w = (((out_cen*1024)/4096)-(FLOAT32)out_low_index);
		/*w = 1.0/2.0;*/
		if (i%50 == 0) {
			VIPprintf("w = %lf, %lf  %d \n", w, ((out_cen*1023)/4096), out_low_index);
		}
		temp_low = (FLOAT32)GOut_R[out_low_index];
		temp_high = (FLOAT32)GOut_R[out_high_index];
		temp_out  = ((1-w)*temp_low + w*temp_high)+0.5;
		GOut_R[i] = (UINT16)temp_out;

		if (i%50 == 0) {
			VIPprintf("R:[%d] %lf, %d\n", i, temp_out, GOut_R[i]);
		}

		temp_low = (FLOAT32)GOut_G[out_low_index];
		temp_high = (FLOAT32)GOut_G[out_high_index];
		temp_out  = ((1-w)*temp_low + w*temp_high)+0.5;
		GOut_G[i] = (UINT16)temp_out;

		if (i%50 == 0) {
			VIPprintf("G:[%d]%lf, %d\n", i, temp_out, GOut_G[i]);
		}

		temp_low = (FLOAT32)GOut_B[out_low_index];
		temp_high = (FLOAT32)GOut_B[out_high_index];
		temp_out  = ((1-w)*temp_low + w*temp_high)+0.5;
		GOut_B[i] = (UINT16)temp_out;

		if (i%50 == 0) {
			VIPprintf("B:[%d]%lf, %d\n", i, temp_out, GOut_B[i]);
		}
		/*w =  ((out_cen << 18))>>12-(((out_cen << 10)-2048)>>12)<< 8;*/
		/*VIPprintf("[%d] %d, %d, %d\n", i, GOut_R[i], GOut_G[i], GOut_B[i]);*/
	}

}
#endif

void fwif_color_setrgbcolortemp_contrast(unsigned short Red, unsigned short Green, unsigned short Blue)
{
	drvif_color_setrgbcolortemp_contrast(Red, Green, Blue);
}


void fwif_color_setrgbcolortemp_bright(unsigned short Red, unsigned short Green, unsigned short Blue)
{
	drvif_color_setrgbcolortemp_bright(Red, Green, Blue);
}

void fwif_color_setrgbcolortemp_en(unsigned char enable)
{
	drvif_color_setrgbcolortemp_en(enable);
}

#if 0 // 20170524 jimmy, D-decontour removed in Merlin3
/*Elsie 20150310 de-countour middleware*/
void fwif_color_set_de_contour(unsigned char src_idx, unsigned char level)
{
	extern DRV_De_CT_1D DeCT_1D[DeCT_1D_Level_MAX];
	DRV_De_CT_1D data;

	/* Table Protection*/
	if (level >= DeCT_1D_Level_MAX)
		level = 0;

	/*EN*/
	data.DeContour_ctrl.DeCT_en = DeCT_1D[level].DeContour_ctrl.DeCT_en;

	/*LPF tap Set*/
	data.DeContour_tap.K0 = DeCT_1D[level].DeContour_tap.K0;
	data.DeContour_tap.K1 = DeCT_1D[level].DeContour_tap.K1;
	data.DeContour_tap.K2 = DeCT_1D[level].DeContour_tap.K2;
	data.DeContour_tap.K3 = DeCT_1D[level].DeContour_tap.K3;
	data.DeContour_tap.K4 = DeCT_1D[level].DeContour_tap.K4;
	data.DeContour_tap.K5 = DeCT_1D[level].DeContour_tap.K5;
	data.DeContour_tap.LP_SHIFT = DeCT_1D[level].DeContour_tap.LP_SHIFT;

	/*De-contour Criteria & Weight*/
	data.DeContour_set.W1 = DeCT_1D[level].DeContour_set.W1;
	data.DeContour_set.W2 = DeCT_1D[level].DeContour_set.W2;
	data.DeContour_set.W1W2 = DeCT_1D[level].DeContour_set.W1W2;
	data.DeContour_set.MIN_GAP_W3 = DeCT_1D[level].DeContour_set.MIN_GAP_W3;
	data.DeContour_set.GAP_W3 = DeCT_1D[level].DeContour_set.GAP_W3;
	data.DeContour_set.SMOOTH_GAP_W = DeCT_1D[level].DeContour_set.SMOOTH_GAP_W;
	data.DeContour_set.ALL_CHANNEL_W = DeCT_1D[level].DeContour_set.ALL_CHANNEL_W;

	drvif_color_set_de_contour((DRV_De_CT_1D *)(&data));
}
#endif

void fwif_color_set_color_mapping(unsigned char src_idx, unsigned char table)
{
	extern DRV_Color_Mapping gCMTable[Color_Mapping_MAX];
	unsigned display;
	display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);

	if (table >= Color_Mapping_MAX)
		table = 0;
	drvif_color_set_Color_Mapping(display,(DRV_Color_Mapping *)(&(gCMTable[table])));
}

void fwif_color_set_color_mapping_enable(unsigned char display, unsigned char enable)
{
	drvif_color_set_Color_Mapping_Enable(display, enable);
}

void fwif_color_ColorMap_SetMatrix(unsigned char display, short (*ColorMapMatrix)[3], unsigned char scale, unsigned char forceUpdate, unsigned char bNoWaitPorch)
{
	extern short gColorGamutMatrix_Apply[3][3];
	static DRV_Color_Mapping CMTable;
	short ColorMapMatrix_2s_complement[3][3];
	signed short ii, jj;

	if (memcmp(gColorGamutMatrix_Apply, ColorMapMatrix, sizeof(gColorGamutMatrix_Apply)) == 0 && !forceUpdate)
		return;
	memcpy(gColorGamutMatrix_Apply, ColorMapMatrix, sizeof(gColorGamutMatrix_Apply));

	/*here have to do software clip*/

	CMTable.Color_Mapping_En=1;
	CMTable.CM_InOffset.XOffset=0;
	CMTable.CM_InOffset.YOffset=0;
	CMTable.CM_InOffset.ZOffset=0;

	CMTable.CM_OutOffset.ROffset=0;
	CMTable.CM_OutOffset.GOffset=0;
	CMTable.CM_OutOffset.BOffset=0;


	//-----------------------------------------------------------------------------------------------
	/*juwen, k5lp, for tv006 : LG K5LP only*/
	for(ii = 0;ii < 3; ii++){
		for(jj = 0;  jj < 3; jj++){
			ColorMapMatrix_2s_complement[ii][jj] = gColorGamutMatrix_Apply[ii][jj] * scale;
			if(ColorMapMatrix_2s_complement[ii][jj]  > 8191){
				ColorMapMatrix_2s_complement[ii][jj] = 8191 ;
			}
			else if(ColorMapMatrix_2s_complement[ii][jj]  < -8192){
				ColorMapMatrix_2s_complement[ii][jj] = -8192 ;
			}
		}
	}

	/*juwen, k5lp, for tv006 : LG K5LP only, 2'S complement*/
	CMTable.CM_Data.K11=ColorMapMatrix_2s_complement[0][0];
	CMTable.CM_Data.K12=ColorMapMatrix_2s_complement[0][1];
	CMTable.CM_Data.K13=ColorMapMatrix_2s_complement[0][2];
	CMTable.CM_Data.K21=ColorMapMatrix_2s_complement[1][0];
	CMTable.CM_Data.K22=ColorMapMatrix_2s_complement[1][1];
	CMTable.CM_Data.K23=ColorMapMatrix_2s_complement[1][2];
	CMTable.CM_Data.K31=ColorMapMatrix_2s_complement[2][0];
	CMTable.CM_Data.K32=ColorMapMatrix_2s_complement[2][1];
	CMTable.CM_Data.K33=ColorMapMatrix_2s_complement[2][2];
	//-----------------------------------------------------------------------------------------------

	if (!bNoWaitPorch) { //from FormatHandler calling, can't wait due to ISR and spin_lock/semaphore section
		if (!drvif_scalerdisplay_get_force_bg_status() && Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE) {
			fwif_color_WaitFor_SYNC_START_UZUDTG();
		}
	}
	drvif_color_set_Color_Mapping(display,(DRV_Color_Mapping *)&CMTable);
}


unsigned int crc32(const void *buf, unsigned int size)
{
	static unsigned int crc32_tab[] = {
		0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
		0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
		0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
		0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
		0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
		0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
		0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
		0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
		0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
		0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
		0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
		0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
		0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
		0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
		0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
		0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
		0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
		0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
		0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
		0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
		0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
		0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
		0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
		0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
		0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
		0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
		0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
		0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
		0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
		0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
		0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
		0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
		0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
		0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
		0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
		0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
		0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
		0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
		0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
		0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
		0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
		0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
		0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
	};
	const unsigned char *p;
	unsigned int crc = 0xFFFFFFFF;

	p = (const unsigned char *)buf;
	crc = crc ^ ~0U;

	while (size--)
		crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);

	return crc ^ ~0U;
}
/* === checksum ========== */
unsigned int checksum(const void *buf, unsigned int size)
{
	const unsigned char *p;
	unsigned int sum = 0;
	unsigned char tmp;
	p = (const unsigned char *)buf;


	while (size--)
	{
		tmp = *p++;
		if( tmp== 0)  tmp = 1;
		sum += tmp;
	}
	return sum;
}

void fwif_color_check_VIPTable_crc(VIP_table_crc_value* crc_value, SLR_VIP_TABLE * vip_table)
{
	SLR_VIP_TABLE *g_Share_Memory_VIP_TABLE_Struct;
	g_Share_Memory_VIP_TABLE_Struct = vip_table;

	crc_value->VIP_QUALITY_Coef = checksum(&g_Share_Memory_VIP_TABLE_Struct->VIP_QUALITY_Coef,
		sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK);
	crc_value->VIP_QUALITY_Extend_Coef = checksum(&g_Share_Memory_VIP_TABLE_Struct->VIP_QUALITY_Extend_Coef,
		sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK);
	crc_value->VIP_QUALITY_Extend2_Coef = checksum(&g_Share_Memory_VIP_TABLE_Struct->VIP_QUALITY_Extend2_Coef,
		sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK);
	crc_value->VIP_QUALITY_Extend3_Coef = checksum(&g_Share_Memory_VIP_TABLE_Struct->VIP_QUALITY_Extend3_Coef,
		sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK);
	crc_value->VIP_QUALITY_3Dmode_Coef = checksum(&g_Share_Memory_VIP_TABLE_Struct->VIP_QUALITY_3Dmode_Coef,
		sizeof(unsigned char)*VIP_QUALITY_3DSOURCE_NUM*VIP_QUALITY_3DFUNCTION_TOTAL_CHECK);
	crc_value->VIP_QUALITY_Extend4_Coef = checksum(&g_Share_Memory_VIP_TABLE_Struct->VIP_QUALITY_Extend4_Coef,
		sizeof(unsigned char)*VIP_QUALITY_SOURCE_NUM*VIP_QUALITY_FUNCTION_TOTAL_CHECK);
	crc_value->SD_H_table = checksum(&g_Share_Memory_VIP_TABLE_Struct->SD_H_table,
		sizeof(unsigned char)*UZD_Idx_TBL_Max*VIP_ScalingDown_FIR_Level_Max);
	crc_value->SD_V_table = checksum(&g_Share_Memory_VIP_TABLE_Struct->SD_V_table,
		sizeof(unsigned char)*UZD_Idx_TBL_Max*VIP_ScalingDown_FIR_Level_Max);
	crc_value->DCR_TABLE = checksum(&g_Share_Memory_VIP_TABLE_Struct->DCR_TABLE,
		sizeof(unsigned char)*t_DCR_TABLE_ROW_MAX*t_DCR_TABLE_COL_MAX);
	crc_value->ICM_by_timing_picmode = checksum(&g_Share_Memory_VIP_TABLE_Struct->ICM_by_timing_picmode,
		sizeof(unsigned char)*VIP_TABLE_PICTURE_MODE_MAX*VIP_TABLE_TIMIMG_MAX);
	crc_value->Auto_Function_Array1 = checksum(&g_Share_Memory_VIP_TABLE_Struct->Auto_Function_Array1,
		sizeof(unsigned char)*Auto_Function_Array_MAX);
	crc_value->Auto_Function_Array2 = checksum(&g_Share_Memory_VIP_TABLE_Struct->Auto_Function_Array2,
		sizeof(unsigned char)*Auto_Function_Array_MAX);
	crc_value->Auto_Function_Array3 = checksum(&g_Share_Memory_VIP_TABLE_Struct->Auto_Function_Array3,
		sizeof(unsigned char)*Auto_Function_Array_MAX);
	crc_value->Auto_Function_Array4 = checksum(&g_Share_Memory_VIP_TABLE_Struct->Auto_Function_Array4,
		sizeof(unsigned char)*Auto_Function_Array_MAX);
	crc_value->Auto_Function_Array5 = checksum(&g_Share_Memory_VIP_TABLE_Struct->Auto_Function_Array5,
		sizeof(unsigned char)*Auto_Function_Array_MAX);
	crc_value->DrvSetting_Skip_Flag = checksum(&g_Share_Memory_VIP_TABLE_Struct->DrvSetting_Skip_Flag,
		sizeof(unsigned char)*DrvSetting_Skip_Flag_item_Max);
	crc_value->VD_ConBriHueSat = checksum(&g_Share_Memory_VIP_TABLE_Struct->VD_ConBriHueSat,
		sizeof(VIP_VD_ConBriHueSat)*t_VD_ConBriHueSat_MAX);
	crc_value->Idomain_MBPKTable = checksum(&g_Share_Memory_VIP_TABLE_Struct->Idomain_MBPKTable, sizeof(VIP_MBPK_Table)*MBPK_table_num);
	crc_value->Tone_Mapping_LUT_R = checksum(&g_Share_Memory_VIP_TABLE_Struct->Tone_Mapping_LUT_R, sizeof(unsigned short)*Tone_Mapping_Table_MAX*Tone_Mapping_size);
	crc_value->ODtable_Coeff = checksum(&g_Share_Memory_VIP_TABLE_Struct->ODtable_Coeff,
		sizeof(unsigned int)*t_ODtable_Coeff_ROW_MAX*t_ODtable_Coeff_COL_MAX);
	crc_value->CDS_ini = checksum(&g_Share_Memory_VIP_TABLE_Struct->CDS_ini, sizeof(VIP_CDS_Table)*Sharp_table_num);
	crc_value->I_EDGE_Smooth_Coef = checksum(&g_Share_Memory_VIP_TABLE_Struct->I_EDGE_Smooth_Coef,
		sizeof(DRV_IEdgeSmooth_Coef)*t_I_EDGE_Smooth_Coef_MAX);
	crc_value->YUV2RGB_CSMatrix_Table = checksum(&g_Share_Memory_VIP_TABLE_Struct->YUV2RGB_CSMatrix_Table,
		sizeof(VIP_YUV2RGB_CSMatrix_Table)*YUV2RGB_TBL_Num);
	crc_value->tICM_ini = checksum(&g_Share_Memory_VIP_TABLE_Struct->tICM_ini,
		sizeof(unsigned short)*VIP_ICM_TBL_X*VIP_ICM_TBL_Y*VIP_ICM_TBL_Z);
	crc_value->tICM_H_7axis = checksum(&g_Share_Memory_VIP_TABLE_Struct->tICM_H_7axis, sizeof(ICM_H_7axis_table)*VIP_ICM_TBL_X);
	crc_value->tGAMMA = checksum(&g_Share_Memory_VIP_TABLE_Struct->tGAMMA, sizeof(VIP_Gamma)*t_GAMMA_MAX);
	crc_value->DCC_Control_Structure = checksum(&g_Share_Memory_VIP_TABLE_Struct->DCC_Control_Structure, sizeof(VIP_DCC_Control_Structure));
	crc_value->Ddomain_SHPTable = checksum(&g_Share_Memory_VIP_TABLE_Struct->Ddomain_SHPTable,
		sizeof(VIP_Sharpness_Table)*Sharp_table_num);
	crc_value->Manual_NR_Table = checksum(&g_Share_Memory_VIP_TABLE_Struct->Manual_NR_Table,
		sizeof(DRV_NR_Item)*PQA_TABLE_MAX*DRV_NR_Level_MAX);
	crc_value->PQA_Table = checksum(&g_Share_Memory_VIP_TABLE_Struct->PQA_Table,
		sizeof(unsigned int)*PQA_TABLE_MAX*PQA_MODE_MAX*PQA_ITEM_MAX*PQA_LEVEL_MAX);
	crc_value->PQA_Input_Table = checksum(&g_Share_Memory_VIP_TABLE_Struct->PQA_Input_Table,
		sizeof(unsigned int)*PQA_I_TABLE_MAX*PQA_I_ITEM_MAX*PQA_I_LEVEL_MAX);
	crc_value->checkSum = checksum(crc_value, sizeof(VIP_table_crc_value)-sizeof(unsigned int));
}
/*==========================*/

void fwif_VIP_set_Project_ID(VIP_Customer_Project_ID_ENUM Project_ID)
{
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table=NULL;
	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if (VIP_RPC_system_info_structure_table != NULL)
		VIP_RPC_system_info_structure_table->Project_ID = Project_ID;
}

unsigned char fwif_VIP_get_Project_ID(void)
{
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table=NULL;
	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if (VIP_RPC_system_info_structure_table != NULL)
		return VIP_RPC_system_info_structure_table->Project_ID;
	else
		return VIP_Project_ID_TV001;
}

#if 0	//===marked by Elsie===
void fwif_color_set_DCC_ICM_DataPath_Swap(unsigned char mode)
{
	drvif_color_set_DCC_ICM_DataPath_Swap(mode);
}

void fwif_color_set_FW_On_Off_HMCNR(unsigned char En)
{
	drvif_color_set_FW_On_Off_HMCNR(En);
}
#endif

unsigned char fwif_color_set_pq_demo_flag_rpc(unsigned char flag)
{
	_RPC_system_setting_info *RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if (NULL == RPC_system_info_structure_table)
		return 0;

	RPC_system_info_structure_table->PQ_demo_flag = flag;

	return 1;
}

static DEMO_CALLBACK_FUNC m_cbFunc_demo_func = NULL;
static DEMO_CALLBACK_FUNC m_cbFunc_overscan_func = NULL;
void fwif_color_reg_demo_callback(DEMO_CALLBACK_ID id, DEMO_CALLBACK_FUNC cbFunc)
{
	switch (id)
	{
	case DEMO_CALLBACKID_ON_OFF_SWITCH:
		m_cbFunc_demo_func = cbFunc;
	break;
	case DEMO_CALLBACKID_OVERSCAN:
		m_cbFunc_overscan_func = cbFunc;
	break;
	default:
		;
	}
}

DEMO_CALLBACK_FUNC fwif_color_get_demo_callback(DEMO_CALLBACK_ID id)
{
	switch (id)
	{
	case DEMO_CALLBACKID_ON_OFF_SWITCH:
		return m_cbFunc_demo_func;
	break;
	case DEMO_CALLBACKID_OVERSCAN:
		return m_cbFunc_overscan_func;
	break;
	default:
		return NULL;
		;
	}
}

char fwif_color_set_bt2020_Handler(unsigned char display, unsigned char Enable_Flag, unsigned char bt2020_Mode)
{
	unsigned char InvGamma_TBL_Sel, CM_TBL_Select;
	unsigned short *invGammaTBL;
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;

	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

#ifdef CONFIG_ARM64
	memcpy_fromio(&drv_vipCSMatrix_t, &(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix), sizeof(DRV_VIP_YUV2RGB_CSMatrix));
#else
	memcpy(&drv_vipCSMatrix_t, &(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix), sizeof(DRV_VIP_YUV2RGB_CSMatrix));
#endif
	fwif_color_ChangeUINT32Endian((unsigned int *) &(drv_vipCSMatrix_t.RGB_Offset), 3, 0);
	fwif_color_ChangeUINT16Endian(&(drv_vipCSMatrix_t.COEF_By_Y.K11[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
	fwif_color_ChangeUINT16Endian(&(drv_vipCSMatrix_t.COEF_By_Y.K12[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
	fwif_color_ChangeUINT16Endian(&(drv_vipCSMatrix_t.COEF_By_Y.K13[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
	fwif_color_ChangeUINT16Endian(&(drv_vipCSMatrix_t.COEF_By_Y.K22[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
	fwif_color_ChangeUINT16Endian(&(drv_vipCSMatrix_t.COEF_By_Y.K23[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
	fwif_color_ChangeUINT16Endian(&(drv_vipCSMatrix_t.COEF_By_Y.K32[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
	fwif_color_ChangeUINT16Endian(&(drv_vipCSMatrix_t.COEF_By_Y.K33[0]), VIP_YUV2RGB_Y_Seg_Max, 0);

	if (display == SLR_MAIN_DISPLAY) {
		if (Enable_Flag == 1) {
			VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag = Enable_Flag;
			VIP_system_info_structure_table->BT2020_CTRL.Mode = bt2020_Mode;
		} else {
			VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag = 0;
			VIP_system_info_structure_table->BT2020_CTRL.Mode = BT2020_MODE_Max;
		}
	} else {
		VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag_Sub = 0;
		VIP_system_info_structure_table->BT2020_CTRL.Mode_Sub = BT2020_MODE_Max;
		VIPprintf("No Sub for bt 2020, return\n");
		return -1;
#if 0
		if (Enable_Flag == 1) {
			VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag_Sub = Enable_Flag;
			VIP_system_info_structure_table->BT2020_CTRL.Mode_Sub = bt2020_Mode;
			if (bt2020_Mode == BT2020_MODE_Non_Constant)
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode_Sub = YUV2RGB_INPUT_2020_NonConstantY;
			else
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode_Sub = YUV2RGB_INPUT_2020_ConstantY;
		} else {
			VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag_Sub = 0;
			VIP_system_info_structure_table->BT2020_CTRL.Mode_Sub = BT2020_MODE_Max;
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode_Sub = YUV2RGB_INPUT_MODE_UNKNOW;/*determine by "Scaler_SetDataFormatHandler()"*/
		}
#endif
	}

	if (bt2020_Mode == BT2020_MODE_Constant && Enable_Flag == 1) {
		InvGamma_TBL_Sel = 1;
		invGammaTBL = &Power22InvGamma[0];
		CM_TBL_Select = 1;
		drv_vipCSMatrix_t.CTRL_ITEM.Bt2020_En_TBL1 = 1;
		drv_vipCSMatrix_t.CTRL_ITEM.CoefByY_En = 0;	/* refference to spec*/
		drv_vipCSMatrix_t.CTRL_ITEM.UVOffset_En = 0;	/* refference to spec*/
	} else {
		InvGamma_TBL_Sel = 0;
		invGammaTBL = &LinearInvGamma[0];
		CM_TBL_Select = 0;
		drv_vipCSMatrix_t.CTRL_ITEM.Bt2020_En_TBL1 = 0;
	}

	/*record to sh mem*/
	/*memcpy(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix), &drv_vipCSMatrix_t, sizeof(DRV_VIP_YUV2RGB_CSMatrix));*/

	/* set RGB2YUV*/
	Scaler_SetDataFormatHandler();
	/* set Inv Gamma*/
	fwif_color_set_InvGamma(display, InvGamma_TBL_Sel, invGammaTBL, invGammaTBL, invGammaTBL);
	/* set Color Mapping Matrix*/
	//fwif_color_set_color_mapping(0, CM_TBL_Select);/*juwen, Merlin3, remove color map*/
	/* set YUV2RGB*/
	fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(0, display, VIP_system_info_structure_table);
	/* set Bt2020 Constant mode enable */
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_YUV2RGB_Base_Ctrl, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_CoefByY_ctrl, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_UVOffset_ctrl, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	return 0;
}

char fwif_color_set_InvGamma(unsigned char display, unsigned char tblSelect, unsigned short *invGamma_R, unsigned short *invGamma_G,  unsigned short *invGamma_B)
{
	/*unsigned int final_invGAMMA_R[Bin_Num_Gamma / 2], final_invGAMMA_G[Bin_Num_Gamma / 2], final_invGAMMA_B[Bin_Num_Gamma / 2];*/
	if (display != SLR_MAIN_DISPLAY) {
		VIPprintf("No Sub for Inv Gamma, return\n");
		return -1;
	}

	fwif_color_inv_gamma_control_front(display);
	fwif_color_gamma_encode(final_invGAMMA_R, final_invGAMMA_G, final_invGAMMA_B, invGamma_R, invGamma_G, invGamma_B);
	fwif_color_Set_Inv_Gamma_Encode(final_invGAMMA_R, final_invGAMMA_G, final_invGAMMA_B);
	fwif_color_inv_gamma_control_back(display, 1);
	return 0;
}

void fwif_OneKey_HDR_HLG_Decide_HDR(unsigned char mode)
{
#if 0
	unsigned char dolby_mode, hdmi_in_mux, dolby_ratio, en_422to444_1, VGIP_SWAP, en_tc_422to444_1;
       //unsigned char display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);

	en_tc_422to444_1 = 0;

	if ((mode == HAL_VPQ_HDR_MODE_HDR10 || mode == HAL_VPQ_HDR_MODE_HLG)) {
		if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC) {
                #if 0 // remove dolby mode(3) compress moe
			if(drvif_HDR_get_VO_Decompress_Status(Get_DisplayMode_Port(display)))
			{//Playback HDR10 mode and compress mode
				dolby_mode = 3;
				hdmi_in_mux = 1;
				dolby_ratio = 0;
			}
			else
                #endif
			{//Playback HDR10 mode and no compress mode
				dolby_mode = 1;
				hdmi_in_mux = 1;
				dolby_ratio = 0;
			}
			en_422to444_1 = 0;
			VGIP_SWAP = 1;
		} else {
			dolby_mode = 0;
			hdmi_in_mux = 0;
			dolby_ratio = 0;
			en_422to444_1 = 1;	/* en_422to444_1 should be 1 in VDEC SDR, but need  to check in HDMI*/
			VGIP_SWAP = 0;
		}
	} else {
			dolby_mode = 0;
			hdmi_in_mux = 0;
			dolby_ratio = 0;
			en_422to444_1 = 1;	/* en_422to444_1 should be 1 in VDEC SDR, but need  to check in HDMI*/
			VGIP_SWAP = 0;
	}

	drvif_DM_HDR_Vtop_set(dolby_mode, hdmi_in_mux, dolby_ratio, en_422to444_1, en_tc_422to444_1);
	set_HDR10_DM_setting();
	fwif_color_set_HDR10_runmode();
	drvif_color_set_HDR_RGB_swap(VGIP_SWAP);
#endif
}

void fwif_color_set_DM_HDR_3dLUT(unsigned char enable, unsigned int *pArray)
{
	fwif_color_DM_HDR_3dLUT_Encode(pArray, DM_HDR_3D_DRV_LUT);
	drvif_DM_HDR_3dLUT(enable, DM_HDR_3D_DRV_LUT);
}

static UINT16 pre_hdr_3dlut[HDR_24x24x24_size*3] = {0};
UINT8 fwif_HDR_compare_3DLUT(void *p, VIP_HDR10_CSC3_TABLE_Mode mode)
{
	PQ_HDR_3dLUT_16_PARAM *pParam = (PQ_HDR_3dLUT_16_PARAM *)p;
	extern unsigned char g_HDR3DLUTForceWrite;
	UINT16 i;

	unsigned int HDR3DLUT_ZIZE;

	if (mode == OLD_17x17x17)
		HDR3DLUT_ZIZE = VIP_DM_HDR_3D_LUT_UI_TBL_SIZE;
	else if (mode == NEW_24x24x24)
		HDR3DLUT_ZIZE = HDR_24x24x24_size*3;
	if (g_HDR3DLUTForceWrite)
	{
		for(i = 0; i < HDR3DLUT_ZIZE; i++)
		{
			pre_hdr_3dlut[i] = pParam->a3dLUT_16_pArray[i];
		}
		return 1;
	}
	else
	{
		UINT8 diff = 0;
		i = 0;
		while (i < HDR3DLUT_ZIZE)
		{
			if (pre_hdr_3dlut[i] != pParam->a3dLUT_16_pArray[i])
			{
				diff = 1;
				break;
			}
			i++;
		}

		if (diff)
		{
			for(i = 0; i < HDR3DLUT_ZIZE; i++)
			{
				pre_hdr_3dlut[i] = pParam->a3dLUT_16_pArray[i];
			}
			return 1;
		}
		else {
			return 0;
		}
	}


}

void fwif_HDR_RGB2OPT(unsigned char enable, unsigned int *pArray)
{
	extern dm_dm_submodule_enable_RBUS dm_submodule_temp;
	_RPC_system_setting_info *RPC_system_setting_info = NULL;
	RPC_system_setting_info = (_RPC_system_setting_info *) Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if (RPC_system_setting_info->HDR_info.Ctrl_Item[TV006_HDR_En] == 0 && fwif_VIP_get_Project_ID() == VIP_Project_ID_TV006)
		return;

	drvif_HDR_RGB2OPT(enable, pArray);

	if(enable==0)
		dm_submodule_temp.b01_04_enable = 0;
	else
		dm_submodule_temp.b01_04_enable = 1;
}

void fwif_DM2_GAMMA_Enable(unsigned char En)
{
	drvif_DM2_GAMMA_Enable(En);
}

char fwif_color_set_DM_HDR_3dLUT_16(void *p)
{
#ifndef BUILD_QUICK_SHOW

	PQ_HDR_3dLUT_16_PARAM *pParam = (PQ_HDR_3dLUT_16_PARAM *)p;
	unsigned char enable = pParam->c3dLUT_16_enable;
	unsigned short *pArray = pParam->a3dLUT_16_pArray;

	extern UINT8 game_process;
	if (game_process)
	{
		game_process = 0;
		msleep_interruptible(50);	//for game bug, delay 1 frame
	}

	drvif_DM_HDR_3dLUT_16(enable, pArray);
#endif
	return 1;
}

void fwif_color_get_Read_DM_HDR_3dLUT(unsigned char *enable, unsigned int *pArray, unsigned char getFrom_reg)
{
	unsigned int i;
	if (getFrom_reg == 1) {
		drvif_Read_DM_HDR_3dLUT(enable, DM_HDR_3D_DRV_LUT);
		fwif_color_DM_HDR_3dLUT_Decode(DM_HDR_3D_DRV_LUT, pArray);
	} else {
		for (i=0;i<VIP_DM_HDR_3D_LUT_UI_TBL_SIZE; i++)
			pArray[i] = pre_hdr_3dlut[i];
	}
}

char fwif_color_DM_HDR_3dLUT_Encode(unsigned int* p_inArray, unsigned int *p_outArray)
{
	unsigned short i, ii, jj;

	ii = 0;
	jj = 0;

	for (i = 0; i < VIP_DM_HDR_3D_LUT_SIZE; i++) {
		p_outArray[ii] = (p_inArray[jj] << 16) + (p_inArray[jj+1]);
		ii++;
		jj += 2;
		p_outArray[ii] = p_inArray[jj];
		ii++;
		jj++;
	}
	return 0;

}

char fwif_color_DM_HDR_3dLUT_Decode(unsigned int* p_inArray, unsigned int *p_outArray)
{
	unsigned short i, ii, jj;

	ii = 0;
	jj = 0;

	for (i = 0; i < VIP_DM_HDR_3D_LUT_SIZE; i++) {
		p_outArray[ii] = p_inArray[jj] >> 16;
		ii++;
		p_outArray[ii] = p_inArray[jj] & 0xFFFF;
		ii++;
		jj++;
		p_outArray[ii] = p_inArray[jj] & 0xFFFF;;
		ii ++;
		jj ++;
	}
	return 0;
}

void fwif_color_cpy_D_3DLUT_TBL(void)
{
	extern int D_3DLUT_999_Reseult[VIP_D_3DLUT_999_SIZE][VIP_8VERTEX_RGB_MAX];

	memcpy(vip_8vertex_temp, D_3DLUT_999_Reseult, VIP_D_3DLUT_999_SIZE * VIP_8VERTEX_RGB_MAX * sizeof(UINT32));
	rtice_Set_D_LUT_9to17(vip_8vertex_temp);
	fwif_color_WaitFor_DEN_STOP_UZUDTG();
	fwif_color_set_D_3dLUT(255, vip_8vertex_temp);

}
void fwif_color_set_D_3dLUT(unsigned char LUT_Ctrl, unsigned int *pArray)
{
	fwif_color_D_3dLUT_Encode(pArray, D_3D_DRV_LUT);
	drvif_color_D_3dLUT(LUT_Ctrl, D_3D_DRV_LUT);
}

void fwif_color_get_Read_D_3dLUT(unsigned char *LUT_Ctrl, unsigned int *pArray)
{
	drvif_color_Read_D_3dLUT(LUT_Ctrl, D_3D_DRV_LUT);
	fwif_color_D_3dLUT_Decode(D_3D_DRV_LUT, pArray);
}

char fwif_color_D_3dLUT_Encode(unsigned int* p_inArray, unsigned int *p_outArray)
{
	unsigned short i, ii, jj;

	ii = 0;
	jj = 0;

	for (i = 0; i < VIP_D_3D_LUT_SIZE; i++) {
		p_outArray[ii] = (p_inArray[jj] << 16) + (p_inArray[jj+1]);
		ii++;
		jj += 2;
		p_outArray[ii] = p_inArray[jj];
		ii++;
		jj++;
	}
	return 0;

}

char fwif_color_D_3dLUT_Decode(unsigned int* p_inArray, unsigned int *p_outArray)
{
	unsigned short i, ii, jj;

	ii = 0;
	jj = 0;

	for (i = 0; i < VIP_D_3D_LUT_SIZE; i++) {
		p_outArray[ii] = p_inArray[jj] >> 16;
		ii++;
		p_outArray[ii] = p_inArray[jj] & 0xFFFF;
		ii++;
		jj++;
		p_outArray[ii] = p_inArray[jj] & 0xFFFF;;
		ii ++;
		jj ++;
	}
	return 0;
}

void drvif_color_calculate_D_3DLUT_by_8vertex(UINT8 axis, UINT16 *vip_8vertex)
{
	static UINT16 pre_8vertex[VIP_8VERTEX_MAX][VIP_8VERTEX_RGB_MAX] = {0};
	UINT8 diff_check = 0;
	extern UINT32 D_3DLUT_999[VIP_D_3DLUT_999_SIZE][VIP_8VERTEX_RGB_MAX];
	UINT8 i, j, k, ip, jp, kp;

	for (i = 0; i < VIP_8VERTEX_MAX; i++)
	{
		if (pre_8vertex[i][axis] != *(vip_8vertex + (i * VIP_8VERTEX_RGB_MAX) + axis))
		{
			diff_check = 1;
			pre_8vertex[i][axis] = *(vip_8vertex + (i * VIP_8VERTEX_RGB_MAX) + axis);
		}
	}

	if (diff_check)
	{
		for (k = 0; k < 9; k++)
		{
			kp = 8 - k;
			for (j = 0; j < 9; j++)
			{
				jp = 8 - j;
				for (i = 0; i < 9; i++)
				{
					ip = 8 - i;
					D_3DLUT_999[i + (j * 9) + (k * 9 * 9)][axis] = (
					(i	*	j	*	k	* (*(vip_8vertex + (VIP_8VERTEX_WHITE * VIP_8VERTEX_RGB_MAX) + axis))) +
					(ip	*	jp	*	kp	* (*(vip_8vertex + (VIP_8VERTEX_BLACK * VIP_8VERTEX_RGB_MAX) + axis))) +
					(i	*	jp	*	kp	* (*(vip_8vertex + (VIP_8VERTEX_RED * VIP_8VERTEX_RGB_MAX) + axis))) +
					(ip	*	j	*	kp	* (*(vip_8vertex + (VIP_8VERTEX_GREEN * VIP_8VERTEX_RGB_MAX) + axis))) +
					(ip	*	jp	*	k	* (*(vip_8vertex + (VIP_8VERTEX_BLUE * VIP_8VERTEX_RGB_MAX) + axis))) +
					(i	*	j	*	kp	* (*(vip_8vertex + (VIP_8VERTEX_YELLOW * VIP_8VERTEX_RGB_MAX) + axis))) +
					(ip	*	j	*	k	* (*(vip_8vertex + (VIP_8VERTEX_CYAN * VIP_8VERTEX_RGB_MAX) + axis))) +
					(i	*	jp	*	k	* (*(vip_8vertex + (VIP_8VERTEX_MAGENTA * VIP_8VERTEX_RGB_MAX) + axis)))
					) >> 9;/* "/(8*8*8)" ==> ">> 9" */
				}
			}
		}
	}
}

void drvif_color_D_3DLUT_by_8vertex_pre_process_TV030(UINT8 axis, UINT16 vip_8vertex_tmp[][VIP_8VERTEX_RGB_MAX], UINT16 ori[])
{
	/*pre process, RGB 3vertex using other algo*/
	ori[VIP_8VERTEX_R] = vip_8vertex_tmp[VIP_8VERTEX_RED][axis];
	ori[VIP_8VERTEX_G] = vip_8vertex_tmp[VIP_8VERTEX_GREEN][axis];
	ori[VIP_8VERTEX_B] = vip_8vertex_tmp[VIP_8VERTEX_BLUE][axis];

	vip_8vertex_tmp[VIP_8VERTEX_RED][VIP_8VERTEX_R] = 65535;
	vip_8vertex_tmp[VIP_8VERTEX_RED][VIP_8VERTEX_G] = 0;
	vip_8vertex_tmp[VIP_8VERTEX_RED][VIP_8VERTEX_B] = 0;

	vip_8vertex_tmp[VIP_8VERTEX_GREEN][VIP_8VERTEX_R] = 0;
	vip_8vertex_tmp[VIP_8VERTEX_GREEN][VIP_8VERTEX_G] = 65535;
	vip_8vertex_tmp[VIP_8VERTEX_GREEN][VIP_8VERTEX_B] = 0;

	vip_8vertex_tmp[VIP_8VERTEX_BLUE][VIP_8VERTEX_R] = 0;
	vip_8vertex_tmp[VIP_8VERTEX_BLUE][VIP_8VERTEX_G] = 0;
	vip_8vertex_tmp[VIP_8VERTEX_BLUE][VIP_8VERTEX_B] = 65535;

}


void drvif_color_calculate_D_3DLUT_by_8vertex_RGB_TV030(UINT8 axis, UINT16 vip_8vertex_tmp[][VIP_8VERTEX_RGB_MAX], UINT16 ori[])
{
	INT16 diff[VIP_8VERTEX_RGB_MAX];
	INT16 i, j, k;
	extern UINT32 D_3DLUT_999[VIP_D_3DLUT_999_SIZE][VIP_8VERTEX_RGB_MAX];

	/* calculate offset*/
	diff[VIP_8VERTEX_R] = ori[VIP_8VERTEX_R] - vip_8vertex_tmp[VIP_8VERTEX_RED][axis];
	diff[VIP_8VERTEX_G] = ori[VIP_8VERTEX_G] - vip_8vertex_tmp[VIP_8VERTEX_GREEN][axis];
	diff[VIP_8VERTEX_B] = ori[VIP_8VERTEX_B] - vip_8vertex_tmp[VIP_8VERTEX_BLUE][axis];

	/*nothing change case*/
	if (diff[VIP_8VERTEX_R] == 0 &&
		diff[VIP_8VERTEX_G] == 0 &&
		diff[VIP_8VERTEX_B] == 0)
		return;

	/* adjust region RGB */
	for (k = 0; k < 9; k++)
	{
		for (j = 0; j < 9; j++)
		{
			for (i = 0; i < 9; i++)
			{
				/*region R*/
				if ((i - 2 * (j + k)) >= 0)
					D_3DLUT_999[i + (j * 9) + (k * 9 * 9)][axis] = D_3DLUT_999[i + (j * 9) + (k * 9 * 9)][axis] + ((diff[VIP_8VERTEX_R] * (i - 2 * (j + k))) / 9);

				/*region G*/
				if ((j - 2 * (i + k)) >= 0)
					D_3DLUT_999[i + (j * 9) + (k * 9 * 9)][axis] = D_3DLUT_999[i + (j * 9) + (k * 9 * 9)][axis] + ((diff[VIP_8VERTEX_G] * (j - 2 * (i + k))) / 9);

				/*region B*/
				if ((k - 2 * (i + j)) >= 0)
					D_3DLUT_999[i + (j * 9) + (k * 9 * 9)][axis] = D_3DLUT_999[i + (j * 9) + (k * 9 * 9)][axis] + ((diff[VIP_8VERTEX_G] * (k - 2 * (i + j))) / 9);
			}
		}
	}
}


void drvif_color_calculate_D_3DLUT_by_8vertex_TV030(UINT8 axis, UINT16 *vip_8vertex)
{
	static UINT16 pre_8vertex[VIP_8VERTEX_MAX][VIP_8VERTEX_RGB_MAX] = {0};
	UINT16 vip_8vertex_tmp[VIP_8VERTEX_MAX][VIP_8VERTEX_RGB_MAX] = {0};
	UINT8 diff_check = 0;
	extern UINT32 D_3DLUT_999[VIP_D_3DLUT_999_SIZE][VIP_8VERTEX_RGB_MAX];
	UINT8 i, j, k, ip, jp, kp;
	UINT16 ori[3];

	for (i = 0; i < VIP_8VERTEX_MAX; i++)
	{
		vip_8vertex_tmp[i][axis] = *(vip_8vertex + (i * VIP_8VERTEX_RGB_MAX) + axis);
		if (pre_8vertex[i][axis] != *(vip_8vertex + (i * VIP_8VERTEX_RGB_MAX) + axis))
		{
			diff_check = 1;
			pre_8vertex[i][axis] = *(vip_8vertex + (i * VIP_8VERTEX_RGB_MAX) + axis);
		}
	}

	if (diff_check)
	{
		/*TV030 pre process, RGB 3vertex using other algo*/
		drvif_color_D_3DLUT_by_8vertex_pre_process_TV030(axis, vip_8vertex_tmp, ori);

		/* calculate YCM first*/
		for (k = 0; k < 9; k++)
		{
			kp = 8 - k;
			for (j = 0; j < 9; j++)
			{
				jp = 8 - j;
				for (i = 0; i < 9; i++)
				{
					ip = 8 - i;
					D_3DLUT_999[i + (j * 9) + (k * 9 * 9)][axis] = (
					(i	*	j	*	k	* vip_8vertex_tmp[VIP_8VERTEX_WHITE][axis]) +
					(ip	*	jp	*	kp	* vip_8vertex_tmp[VIP_8VERTEX_BLACK][axis]) +
					(i	*	jp	*	kp	* vip_8vertex_tmp[VIP_8VERTEX_RED][axis]) +
					(ip	*	j	*	kp	* vip_8vertex_tmp[VIP_8VERTEX_GREEN][axis]) +
					(ip	*	jp	*	k	* vip_8vertex_tmp[VIP_8VERTEX_BLUE][axis]) +
					(i	*	j	*	kp	* vip_8vertex_tmp[VIP_8VERTEX_YELLOW][axis]) +
					(ip	*	j	*	k	* vip_8vertex_tmp[VIP_8VERTEX_CYAN][axis]) +
					(i	*	jp	*	k	* vip_8vertex_tmp[VIP_8VERTEX_MAGENTA][axis])
					) >> 9;/* "/(8*8*8)" ==> ">> 9" */
				}
			}
		}


		/* calculate RGB*/
		drvif_color_calculate_D_3DLUT_by_8vertex_RGB_TV030(axis, vip_8vertex_tmp, ori);

	}
}


void drvif_color_get_D_3DLUT_by_8vertex(UINT16 *vip_8vertex)
{
	extern UINT32 D_3DLUT_999[VIP_D_3DLUT_999_SIZE][VIP_8VERTEX_RGB_MAX];

	//UINT32 *vip_8vertex_temp;

	/*unsigned char i=0;
		for(i=0;i<24;i++)
			printk("%d\n",*(vip_8vertex+i));*/

	//check R axis and calculate R data
	drvif_color_calculate_D_3DLUT_by_8vertex(VIP_8VERTEX_R, vip_8vertex);
	//check G axis and calculate G data
	drvif_color_calculate_D_3DLUT_by_8vertex(VIP_8VERTEX_G, vip_8vertex);
	//check B axis and calculate B data
	drvif_color_calculate_D_3DLUT_by_8vertex(VIP_8VERTEX_B, vip_8vertex);

	//write d-domain 3DLUT flow
	//vip_8vertex_temp = (unsigned int *)vip_malloc(VIP_D_3D_LUT_UI_TBL_SIZE * sizeof(UINT32));

	//if (vip_8vertex_temp == NULL) {
	//	rtd_pr_vpq_info("\n");
	//	return;
	//}

	memcpy(vip_8vertex_temp, D_3DLUT_999, VIP_D_3DLUT_999_SIZE * VIP_8VERTEX_RGB_MAX * sizeof(UINT32));

	rtice_Set_D_LUT_9to17(vip_8vertex_temp);
	fwif_color_WaitFor_DEN_STOP_UZUDTG();
	fwif_color_set_D_3dLUT(255, vip_8vertex_temp);

	//vip_free(vip_8vertex_temp);
	//vip_8vertex_temp = NULL;

}

#endif// end of UT_flag
static char Osd_Outer_En=0;
void fwif_color_set_osd_Outer(unsigned char enable)
{
	Osd_Outer_En = enable;
}

char fwif_color_get_osd_Outer(void)
{
	return Osd_Outer_En;

}
#ifndef UT_flag

void fwif_color_VIP_get_VO_Info(unsigned char input_src_type)
{
	SLR_VOINFO* pVOInfo = NULL;
	_system_setting_info *VIP_system_info_structure_table=NULL;
	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if( input_src_type == _SRC_VO || (input_src_type == _SRC_HDMI))
	{
		//  usb
		//    transfer_characteristics    gamma&OETF  curve
		if(pVOInfo->transfer_characteristics == 16) {
			VIP_system_info_structure_table->HDR_CTRL.EOTF_Mode = ITU_SMPTE2084;
		} else if(pVOInfo->transfer_characteristics == 18) {
			VIP_system_info_structure_table->HDR_CTRL.EOTF_Mode = ITU_ARIB_STD_B67;
		} else {

			VIP_system_info_structure_table->HDR_CTRL.EOTF_Mode = ITU_BT1886;
		}
        //    colour_primaries    source's RGBW xy value   depend on RGBW panel  how to use 3x3 or 3D LUT
		if( pVOInfo->colour_primaries == 9 ) {
			VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag = 1;
			VIP_system_info_structure_table->BT2020_CTRL.Mode = BT2020_MODE_Non_Constant;  // default
		} else {
			VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag = 0;
			VIP_system_info_structure_table->BT2020_CTRL.Mode = BT2020_MODE_Max;
		}
		//    matrix_coefficiets    RGB2YUV
		if(pVOInfo->matrix_coefficiets == 9) {
			VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag = 1;
			VIP_system_info_structure_table->BT2020_CTRL.Mode = BT2020_MODE_Non_Constant;
		}
		else if (pVOInfo->matrix_coefficiets == 10) {
			VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag = 1;
			VIP_system_info_structure_table->BT2020_CTRL.Mode = BT2020_MODE_Constant;
		}
	} else {
		VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag = 0;
		VIP_system_info_structure_table->BT2020_CTRL.Mode = BT2020_MODE_Max;
		VIP_system_info_structure_table->HDR_CTRL.EOTF_Mode = HDR_MODE_Max;
	}
	if (fwif_VIP_get_Project_ID() != VIP_Project_ID_TV030) {
	rtd_pr_vpq_info("get VO info, input_src_type=%d,video_full_range_flag=%d,transfer_characteristics=%d,colour_primaries=%d,matrix_coef=%d,EOTF_Mode=%d,bt2020_Enable=%d,BT2020.Mode=%d\n",
		input_src_type,pVOInfo->video_full_range_flag,pVOInfo->transfer_characteristics,pVOInfo->colour_primaries,pVOInfo->matrix_coefficiets,
		VIP_system_info_structure_table->HDR_CTRL.EOTF_Mode, VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag, VIP_system_info_structure_table->BT2020_CTRL.Mode);
	}
}

void fwif_color_VIP_get_DRM_Info(unsigned char input_src_type)
{
	_system_setting_info *VIP_system_info_structure_table=NULL;
	vfe_hdmi_drm_t drm_info;
	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/* get drm info*/
	vfe_hdmi_drv_get_drm_info(&drm_info);
	/* get AVI info frame*/
	/*int vfe_hdmi_drv_get_port_avi_info(vfe_hdmi_avi_t *info_frame)
	unsigned char drvif_Hdmi_GetAviInfoFrame(HDMI_AVI_T *pAviInfo)*/ //pAviInfo size = 16 bytes

	if(input_src_type == _SRC_HDMI)
	{
		if (drm_info.len !=0 && ((drm_info.eEOTFtype == 1 /*Traditional Gamma - HDR Lumi range*/) || (drm_info.eEOTFtype == 2 /*ST2048*/))) {
			VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag = 1;
			VIP_system_info_structure_table->BT2020_CTRL.Mode = BT2020_MODE_Non_Constant;
			VIP_system_info_structure_table->HDR_CTRL.EOTF_Mode = ITU_SMPTE2084;
		} else if(drm_info.len !=0 && drm_info.eEOTFtype == 3 /*Future EOTF HLG??*/) {
			VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag = 1;
			VIP_system_info_structure_table->BT2020_CTRL.Mode = BT2020_MODE_Non_Constant;
			VIP_system_info_structure_table->HDR_CTRL.EOTF_Mode = ITU_ARIB_STD_B67;
		} else {
			VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag = 0;
			VIP_system_info_structure_table->BT2020_CTRL.Mode = BT2020_MODE_Max;
			VIP_system_info_structure_table->HDR_CTRL.EOTF_Mode = HDR_MODE_Max;
		}
	} else {
		VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag = 0;
		VIP_system_info_structure_table->BT2020_CTRL.Mode = BT2020_MODE_Max;
		VIP_system_info_structure_table->HDR_CTRL.EOTF_Mode = HDR_MODE_Max;
	}
	rtd_pr_vpq_info("get DRM info, input_src_type=%d,drm.len=%d,drm.eEOTFtype=%d,EOTF_Mode=%d,bt2020_Enable=%d,BT2020.Mode=%d\n",
		input_src_type,drm_info.len,drm_info.eEOTFtype,
		VIP_system_info_structure_table->HDR_CTRL.EOTF_Mode, VIP_system_info_structure_table->BT2020_CTRL.Enable_Flag, VIP_system_info_structure_table->BT2020_CTRL.Mode);
}

char fwif_color_PQ_ByPass_Handler(unsigned char table_idx, unsigned char isSet_Flag, _RPC_system_setting_info *VIP_RPC_system_info_structure_table, unsigned char Force_byPass)
{
	unsigned char tbl_idx, i;
	//VIP_PQ_ByPass_Struct *pPQ_ByPass_Struct;
	VIP_PQ_ByPass_Struct pPQ_ByPass_Struct;
	unsigned char isIMode_Flag = 0;

	if (isSet_Flag == 1){

		if (table_idx >= VIP_PQ_ByPass_TBL_Max)
			table_idx = 0;

		//VIP_system_info_structure_table->PQ_Setting_Info.PQ_ByPass_Ctrl.table_idx = table_idx;
		VIP_RPC_system_info_structure_table->PQ_ByPass_Ctrl.table_idx = table_idx;
		tbl_idx = table_idx;

		//pPQ_ByPass_Struct = &(PQ_ByPass_Struct[tbl_idx]);
		memcpy((unsigned char*)&pPQ_ByPass_Struct, (unsigned char*)&(PQ_ByPass_Struct[tbl_idx]), sizeof(VIP_PQ_ByPass_Struct));

		isIMode_Flag = Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE);
		if (isIMode_Flag == 1) {
			pPQ_ByPass_Struct.VIP_PQ_ByPass_TBL[PQ_ByPass_I_DI_IP_Enable] = 0;
			pPQ_ByPass_Struct.VIP_PQ_ByPass_TBL[PQ_ByPass_I_DI] = 0;
		}

		for (i=0; i<VIP_PQ_ByPass_ITEMS_Max; i++) {
			if (pPQ_ByPass_Struct.VIP_PQ_ByPass_TBL[i] == 1  || Force_byPass == 1)
				drvif_color_set_PQ_ByPass(i);
		}
	}

	if (fwif_VIP_get_Project_ID() != VIP_Project_ID_TV030) {
	rtd_pr_vpq_info("PQ_ByPass_Handler, table_idx=%d, sys_table_idx=%d, isSet_Flag=%d, Force_byPass=%d, isIMode_Flag=%d\n",
		table_idx, VIP_RPC_system_info_structure_table->PQ_ByPass_Ctrl.table_idx, isSet_Flag, Force_byPass, isIMode_Flag);
	}

	return VIP_RPC_system_info_structure_table->PQ_ByPass_Ctrl.table_idx;
}

unsigned char fwif_color_set_FILM_FW_ShareMemory(void)
{
	int ret;

	g_Share_Memory_FILM_FW = ((FILM_FW_ShareMem *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_FW_FILM));
	if (NULL == g_Share_Memory_FILM_FW)
		return 0;

#ifdef CONFIG_ARM64
	memset_io(g_Share_Memory_FILM_FW, 0, sizeof(FILM_FW_ShareMem));
#else
	memset(g_Share_Memory_FILM_FW, 0, sizeof(FILM_FW_ShareMem));
#endif
	g_Share_Memory_FILM_FW->version = 1;
	g_Share_Memory_FILM_FW->smooth_toggle_film = 0xff;		//default disable smoothtoggle film setting
	ret = Scaler_SendRPC(SCALERIOC_VIP_FW_FILM, 0, 0);
	if (0 !=  ret) {
		VIPprintf("ret = %d, update fwif_color_set_FILM_FW_ShareMemory to driver fail !!!\n", ret);
	}

	return !ret;
}

void fwif_color_WaitFor_DEN_STOP_UZUDTG(void)
{
	UINT32 timeoutcnt = 0x032500;
	UINT32 denEnd;
	ppoverlay_new_meas2_linecnt_real_RBUS new_meas2_linecnt_real_reg;
	ppoverlay_dv_den_start_end_RBUS dv_den_start_end_reg;
	dv_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg);
	denEnd = dv_den_start_end_reg.dv_den_end;
	do {
	                new_meas2_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
	                if(new_meas2_linecnt_real_reg.uzudtg_dly_line_cnt_rt > denEnd) {
	                        return;
	                }
	} while(timeoutcnt-- != 0);

	if(timeoutcnt == 0){
	        VIPprintf("\n%s ERROR not wait the correct line \n", __FUNCTION__);
}
}

void fwif_color_WaitFor_SYNC_START_UZUDTG(void)
{
	UINT32 timeoutcnt = 0x032500;
	UINT32 SyncStartDelay, denStart;
	ppoverlay_new_meas2_linecnt_real_RBUS new_meas2_linecnt_real_reg;
	ppoverlay_dv_den_start_end_RBUS dv_den_start_end_reg;
	dv_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg);
	if (dv_den_start_end_reg.dv_den_sta < 30) {
		fwif_color_WaitFor_DEN_STOP_UZUDTG();
		return;
	}

	SyncStartDelay = dv_den_start_end_reg.dv_den_sta-30;
	denStart = dv_den_start_end_reg.dv_den_sta-10;
	do {
		new_meas2_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
		if(new_meas2_linecnt_real_reg.uzudtg_dly_line_cnt_rt < denStart && new_meas2_linecnt_real_reg.uzudtg_dly_line_cnt_rt > SyncStartDelay)
			return;
	} while(timeoutcnt-- != 0);

	if(timeoutcnt == 0){
	        VIPprintf("\n%s ERROR not wait the correct line \n", __FUNCTION__);
	}
}

void fwif_color_WaitFor_DEN_STOP_MEMCDTG(void)
{
	UINT32 timeoutcnt = 0x032500;
	UINT32 denEnd;
	ppoverlay_new_meas1_linecnt_real_RBUS new_meas1_linecnt_real_reg;
	ppoverlay_memc_mux_ctrl_RBUS ppoverlay_memc_mux_ctrl_reg;
	ppoverlay_memcdtg_dv_den_start_end_RBUS memcdtg_dv_den_start_end;

	ppoverlay_memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
	if (ppoverlay_memc_mux_ctrl_reg.memc_outmux_sel == 0) {
		fwif_color_WaitFor_DEN_STOP_UZUDTG();
		return;
	}

	memcdtg_dv_den_start_end.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg);
	denEnd = memcdtg_dv_den_start_end.memcdtg_dv_den_end;
	do {
                new_meas1_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg);
                if(new_meas1_linecnt_real_reg.memcdtg_line_cnt_rt > denEnd) {
                        return;
                }
	} while(timeoutcnt-- != 0);

	if(timeoutcnt == 0){
	        VIPprintf("\n%s ERROR not wait the correct line \n", __FUNCTION__);
	}
}

void fwif_color_WaitFor_SYNC_START_MEMCDTG(void)
{
	UINT32 timeoutcnt = 0x032500;
	UINT32 SyncStartDelay, denStart;
	ppoverlay_new_meas1_linecnt_real_RBUS new_meas1_linecnt_real_reg;
	ppoverlay_memc_mux_ctrl_RBUS ppoverlay_memc_mux_ctrl_reg;
	ppoverlay_memcdtg_dv_den_start_end_RBUS memcdtg_dv_den_start_end;

	ppoverlay_memc_mux_ctrl_reg.regValue = IoReg_Read32(PPOVERLAY_MEMC_MUX_CTRL_reg);
	if (ppoverlay_memc_mux_ctrl_reg.memc_outmux_sel == 0) {
		fwif_color_WaitFor_DEN_STOP_UZUDTG();
		return;
	}

	memcdtg_dv_den_start_end.regValue = IoReg_Read32(PPOVERLAY_memcdtg_DV_DEN_Start_End_reg);
	if (memcdtg_dv_den_start_end.memcdtg_dv_den_sta < 30) {
		fwif_color_WaitFor_DEN_STOP_MEMCDTG();
		return;
	}

	SyncStartDelay = memcdtg_dv_den_start_end.memcdtg_dv_den_sta-30;
	denStart = memcdtg_dv_den_start_end.memcdtg_dv_den_sta-10;
	do {
		new_meas1_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas1_linecnt_real_reg);
	        if(new_meas1_linecnt_real_reg.memcdtg_line_cnt_rt < denStart && new_meas1_linecnt_real_reg.memcdtg_line_cnt_rt > SyncStartDelay)
				return;
	} while(timeoutcnt-- != 0);

	if(timeoutcnt == 0){
	        VIPprintf("\n%s ERROR not wait the correct line \n", __FUNCTION__);
	}
}


void fwif_color_pattern_mute(unsigned char  bflag)
{
	if(((IoReg_Read32(CON_BRI_DM_COLOR_RGB_CTRL_reg)&_BIT1)>>1) != bflag) {
		if(bflag == FALSE){
			msleep(50);
			fwif_color_WaitFor_DEN_STOP_UZUDTG();
			IoReg_Write32(CON_BRI_DM_CONTRAST_A_reg, 0x20080200);
			IoReg_ClearBits(CON_BRI_DM_COLOR_RGB_CTRL_reg, _BIT1);
		}else {
			if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE){
				fwif_color_WaitFor_DEN_STOP_UZUDTG();
				IoReg_Write32(CON_BRI_DM_CONTRAST_A_reg, 0x00000000);
				IoReg_SetBits(CON_BRI_DM_COLOR_RGB_CTRL_reg, _BIT1);
			}
		}
	}
}

#ifdef CONFIG_HW_SUPPORT_I_DE_XC //mac6 removed de_xcxl
unsigned char  fwif_color_Set_I_De_XC(unsigned char value)
{
	VIP_I_De_XC_TBL byPass = {0};
	VIP_I_De_XC_TBL *ptr;
	unsigned char mem_ready;
	_RPC_system_setting_info *RPC_VIP_system_info_structure_table = NULL;
	RPC_VIP_system_info_structure_table = (_RPC_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	RPC_VIP_system_info_structure_table->I_De_XC_CTRL.table_select = value;

	mem_ready = drv_memory_Get_I_De_XC_Mem_Ready_Flag(0);	/* need to check mem index, if i2run is on*/

	if (value >= I_De_XC_TBL_Max || mem_ready == 0)
		ptr = &byPass;
	else
		ptr = &I_De_XC_TBL[value];

	return drvif_color_Set_I_De_XC(ptr);

}
#endif //CONFIG_HW_SUPPORT_I_DE_XC
unsigned char  fwif_color_Set_I_De_Contour(unsigned char value)
{
	VIP_I_De_Contour_TBL byPass = {0};
	VIP_I_De_Contour_TBL *ptr;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	VIP_system_info_structure_table->PQ_Setting_Info.I_De_Contour_CTRL.table_select = value;

	if (value >= I_De_Contour_TBL_Max)
		ptr = &byPass;
	else
		ptr = &I_De_Contour_TBL[value];

	return drvif_color_Set_I_De_Contour(ptr);

}

void fwif_color_I_De_Contour_line_buffer_check(void)
{
	drvif_color_I_De_Contour_line_buffer_check();
}

extern DRV_AI_Ctrl_table AI_Ctrl_TBL[10];
extern AI_ICM_Ctrl_table AI_ICM_TBL[10];
extern AI_DCC_Ctrl_table AI_DCC_TBL[10];
extern DRV_AI_SCENE_Ctrl_table AI_Scene_Ctrl_TBL[10];

unsigned char  fwif_color_Set_AI_Ctrl(unsigned char value_icm, unsigned char value_dcc, unsigned char value_ctrl)
{
#if defined(CONFIG_RTK_AI_DRV)
	if (value_icm > 10)
		value_icm =10;
	if (value_dcc > 10)
		value_dcc =10;
	if (value_ctrl > 10)
		value_ctrl =10;

        drvif_color_AI_ICM_table_set(&AI_ICM_TBL[value_icm]); //set AI icm

        drvif_color_AI_DCC_table_set(&AI_DCC_TBL[value_dcc]); //set AI dcc

        scaler_AI_obj_Ctrl_Set(&AI_Ctrl_TBL[value_ctrl]); //set dynamic vale;
        scaler_AI_Scene_Ctrl_Set(&AI_Scene_Ctrl_TBL[0]);//tmp
       return 0;
#else
	VIPprintf("[%s][%s] CONFIG_RTK_AI_DRV disabled\n", __FILE__, __FUNCTION__);
	return 1;
#endif
}

//====================20161027 START=======================================================
//juwen, 0627, TC
#if 0 /* juwen, move to "tc_hdr", elieli */
void fwif_color_set_TC_Enable(unsigned char src_idx, bool enable)
{
	drvif_color_set_TC_Enable(enable);
}

void fwif_color_set_TC_certification_Enable(unsigned char src_idx, bool certification_enable, bool certification_bitout)
{
	drvif_color_set_TC_certification_Enable(certification_enable, certification_bitout);
}

//----------compile-0628-------------
void fwif_color_set_TC_Range_Adaptation(unsigned char src_idx, unsigned char TableIdx)
{

	DRV_TCHDR_RangeAdaption data;
	extern DRV_TCHDR_Table TCHDR_Table[5];

	data.tchdr_footroom_int= TCHDR_Table[TableIdx].TCHDR_RangeAdaption.tchdr_footroom_int;
	data.tchdr_l_headroom_int= TCHDR_Table[TableIdx].TCHDR_RangeAdaption.tchdr_l_headroom_int;
	data.tchdr_c_headroom_int= TCHDR_Table[TableIdx].TCHDR_RangeAdaption.tchdr_c_headroom_int;
	data.tchdr_inv_y_limited_range_ratio_int= TCHDR_Table[TableIdx].TCHDR_RangeAdaption.tchdr_inv_y_limited_range_ratio_int;
	data.tchdr_inv_chroma_limited_range_ratio_int= TCHDR_Table[TableIdx].TCHDR_RangeAdaption.tchdr_inv_chroma_limited_range_ratio_int;

	drvif_color_set_TC_Range_Adaptation( (DRV_TCHDR_RangeAdaption*) &data);
}

void fwif_color_set_TC_L_Compute(unsigned char src_idx, unsigned char TableIdx)
{

	DRV_TCHDR_l_compute data;
	extern DRV_TCHDR_Table TCHDR_Table[5];

	data.tchdr_alpha_a= TCHDR_Table[TableIdx].TCHDR_l_compute.tchdr_alpha_a;
	data.tchdr_alpha_b= TCHDR_Table[TableIdx].TCHDR_l_compute.tchdr_alpha_b;

	drvif_color_set_TC_L_Compute( (DRV_TCHDR_l_compute*) &data);
}


void fwif_color_set_TC_T_Compute(unsigned char src_idx, unsigned char TableIdx)
{

	DRV_TCHDR_T_compute data;
	extern DRV_TCHDR_Table TCHDR_Table[5];


	data.tchdr_oct0= TCHDR_Table[TableIdx].TCHDR_T_compute.tchdr_oct0;
	data.tchdr_oct1= TCHDR_Table[TableIdx].TCHDR_T_compute.tchdr_oct1;
	data.tchdr_oct2= TCHDR_Table[TableIdx].TCHDR_T_compute.tchdr_oct2;

	drvif_color_set_TC_T_Compute( (DRV_TCHDR_T_compute*) &data);
}

void fwif_color_set_TC_M_Compute(unsigned char src_idx, unsigned char TableIdx)
{

	DRV_TCHDR_M_compute data;
	extern DRV_TCHDR_Table TCHDR_Table[5];


	data.tchdr_oct3= TCHDR_Table[TableIdx].TCHDR_M_compute.tchdr_oct3;
	data.tchdr_oct4= TCHDR_Table[TableIdx].TCHDR_M_compute.tchdr_oct4;
	data.tchdr_oct5= TCHDR_Table[TableIdx].TCHDR_M_compute.tchdr_oct5;
	data.tchdr_oct6= TCHDR_Table[TableIdx].TCHDR_M_compute.tchdr_oct6;

	drvif_color_set_TC_M_Compute( (DRV_TCHDR_M_compute*) &data);
}

void fwif_color_set_TC_Write_C_LUT(unsigned char src_idx,unsigned char C_lut_idx)
{
	extern DRV_TCHDR_COEF_C_lut TCHDR_COEF_C_LUT_TBL;

	unsigned short *c_lut_coef_ptr = &(TCHDR_COEF_C_LUT_TBL.TCHDR_COEF_C_Lut[C_lut_idx][0]);
	drvif_color_set_TC_Write_C_LUT(c_lut_coef_ptr);
}


void fwif_color_set_TC_Read_C_LUT(unsigned short *c_lut_coef_read_ptr)
{
	//if (IoReg_Read32(0xb802e4f4) & 0x1)
		drvif_color_set_TC_Read_C_LUT(TCHDR_COEF_C_LUT_TBL_ReadReg);
	//if (IoReg_Read32(0xb802e4f4) & 0x2)
		decode_TC_Read_LUTs_regValue(TCHDR_COEF_C_LUT_TBL_ReadReg, c_lut_coef_read_ptr);//(input, output)

}

void fwif_color_set_TC_Write_I_LUT(unsigned char src_idx,unsigned char I_lut_idx)
{

	extern DRV_TCHDR_COEF_I_lut TCHDR_COEF_I_LUT_TBL;

	unsigned short *i_lut_coef_ptr = &(TCHDR_COEF_I_LUT_TBL.TCHDR_COEF_I_Lut[I_lut_idx][0]);
	drvif_color_set_TC_Write_I_LUT(i_lut_coef_ptr);
}

void fwif_color_set_TC_Read_I_LUT(unsigned short *i_lut_coef_read_ptr)
{

	drvif_color_set_TC_Read_I_LUT(TCHDR_COEF_I_LUT_TBL_ReadReg);
	decode_TC_Read_LUTs_regValue(TCHDR_COEF_I_LUT_TBL_ReadReg, i_lut_coef_read_ptr);//(input, output)

}

void fwif_color_set_TC_Write_S_LUT(unsigned char src_idx,unsigned char S_lut_idx)
{
	extern DRV_TCHDR_COEF_S_lut TCHDR_COEF_S_LUT_TBL;

	unsigned short *s_lut_coef_ptr = &(TCHDR_COEF_S_LUT_TBL.TCHDR_COEF_S_Lut[S_lut_idx][0]);
	drvif_color_set_TC_Write_S_LUT(s_lut_coef_ptr);
}

void fwif_color_set_TC_Read_S_LUT(unsigned short *s_lut_coef_read_ptr)
{

	drvif_color_set_TC_Read_S_LUT(TCHDR_COEF_S_LUT_TBL_ReadReg);
	decode_TC_Read_LUTs_regValue(TCHDR_COEF_S_LUT_TBL_ReadReg, s_lut_coef_read_ptr);//(input, output)

}



void fwif_color_set_TC_Write_P_LUT(unsigned char src_idx,unsigned char P_lut_idx)
{
	extern DRV_TCHDR_COEF_P_lut TCHDR_COEF_P_LUT_TBL;

	unsigned short *p_lut_coef_ptr = &(TCHDR_COEF_P_LUT_TBL.TCHDR_COEF_P_Lut[P_lut_idx][0]);
	drvif_color_set_TC_Write_P_LUT(p_lut_coef_ptr);
}

void fwif_color_set_TC_Read_P_LUT(unsigned short *p_lut_coef_read_ptr)
{

	drvif_color_set_TC_Read_P_LUT(TCHDR_COEF_P_LUT_TBL_ReadReg);
	decode_TC_Read_LUTs_regValue(TCHDR_COEF_P_LUT_TBL_ReadReg, p_lut_coef_read_ptr);//(input, output)

}


void fwif_color_set_TC_D_LUT_Step_Threshold(unsigned char src_idx, unsigned char TableIdx)
{

	DRV_TCHDR_D_lut_step_thl data;
	extern DRV_TCHDR_Table TCHDR_Table[5];


	data.tchdr_dlut_step0= TCHDR_Table[TableIdx].TCHDR_D_lut_step_thl.tchdr_dlut_step0;
	data.tchdr_dlut_step1= TCHDR_Table[TableIdx].TCHDR_D_lut_step_thl.tchdr_dlut_step1;
	data.tchdr_dlut_step2= TCHDR_Table[TableIdx].TCHDR_D_lut_step_thl.tchdr_dlut_step2;
	data.tchdr_dlut_step3= TCHDR_Table[TableIdx].TCHDR_D_lut_step_thl.tchdr_dlut_step3;

	data.tchdr_dlut_threshold0= TCHDR_Table[TableIdx].TCHDR_D_lut_step_thl.tchdr_dlut_threshold0;
	data.tchdr_dlut_threshold1= TCHDR_Table[TableIdx].TCHDR_D_lut_step_thl.tchdr_dlut_threshold1;
	data.tchdr_dlut_threshold2= TCHDR_Table[TableIdx].TCHDR_D_lut_step_thl.tchdr_dlut_threshold2;

	drvif_color_set_TC_D_LUT_Step_Threshold( (DRV_TCHDR_D_lut_step_thl*) &data);
}


/*
NOTE : juwen, 0628
In C model : R/G/B use the same D_LUT table <=> set D_lut_Red_idx = D_lut_Green_idx =D_lut_Blue_idx

However, in the following API we can let R/G/B use different D_LUT table.
*/
void fwif_color_set_TC_Write_D_LUT(unsigned char src_idx,unsigned char D_lut_Red_idx,unsigned char D_lut_Geen_idx,unsigned char D_lut_Blue_idx)
{
	extern DRV_TCHDR_COEF_D_lut TCHDR_COEF_D_LUT_TBL;

	unsigned short *d_lut_Red_coef_ptr = &(TCHDR_COEF_D_LUT_TBL.TCHDR_COEF_D_Lut[D_lut_Red_idx][0]);
	unsigned short *d_lut_Green_coef_ptr = &(TCHDR_COEF_D_LUT_TBL.TCHDR_COEF_D_Lut[D_lut_Geen_idx][0]);
	unsigned short *d_lut_Blue_coef_ptr = &(TCHDR_COEF_D_LUT_TBL.TCHDR_COEF_D_Lut[D_lut_Blue_idx][0]);
	drvif_color_set_TC_Write_D_LUT(d_lut_Red_coef_ptr, d_lut_Green_coef_ptr, d_lut_Blue_coef_ptr);
}

/*
NOTE : juwen, 20160629
In C model : R/G/B use the same D_LUT table <=> set D_lut_Red_idx = D_lut_Green_idx =D_lut_Blue_idx

However, in the following API we read R/G/B D_LUT table separate.
*/
void fwif_color_set_TC_Read_D_LUT(unsigned short *d_lut_red_coef_read_ptr,unsigned short *d_lut_green_coef_read_ptr,unsigned short *d_lut_blue_coef_read_ptr)
{
	drvif_color_set_TC_Read_D_LUT(TCHDR_COEF_D_LUT_RED_TBL_ReadReg, TCHDR_COEF_D_LUT_GREEN_TBL_ReadReg, TCHDR_COEF_D_LUT_BLUE_TBL_ReadReg);
	decode_TC_Read_LUTs_regValue(TCHDR_COEF_D_LUT_RED_TBL_ReadReg, d_lut_red_coef_read_ptr);//(input, output)
	decode_TC_Read_LUTs_regValue(TCHDR_COEF_D_LUT_GREEN_TBL_ReadReg, d_lut_green_coef_read_ptr);//(input, output)
	decode_TC_Read_LUTs_regValue(TCHDR_COEF_D_LUT_BLUE_TBL_ReadReg, d_lut_blue_coef_read_ptr);//(input, output)
}
#else
#ifndef BUILD_QUICK_SHOW
char fwif_color_rtice_set_TC_Write_P_LUT(unsigned char src_idx, unsigned short *p_lut_coef_ptr)
{
	if (drvif_Wait_VGIP_start(1, 50, 10000) < 0)
		return -1;

	if (drvif_color_set_TC_Write_P_LUT(p_lut_coef_ptr) < 0)
		return -2;

	return 0;
}

char fwif_color_rtice_set_TC_Read_P_LUT(unsigned short *p_lut_coef_read_ptr)
{
	if (drvif_Wait_VGIP_start(1, 50, 10000) < 0)
		return -1;

	if (drvif_color_set_TC_Read_P_LUT(TCHDR_COEF_P_LUT_TBL_ReadReg) < 0)
		return -2;

	decode_TC_Read_LUTs_regValue(TCHDR_COEF_P_LUT_TBL_ReadReg, p_lut_coef_read_ptr);//(input, output)

	return 0;

}

static unsigned short C_LUT[65];
static unsigned short P_LUT[65];
static unsigned short D_LUT[65];

void fwif_color_ScalerVIP_set_TC_v130_metadata2reg_frameSync(int display_OETF, int display_Brightness, int proc_mode, int width, int height, int yuv_range, _RPC_system_setting_info *RPC_sys_info)
{
	unsigned short muA, muB;
	SL_HDR1_metadata sl_hdr1_metadata;
	//unsigned short INV_C_LUT[65];
	//unsigned short C_LUT[65];
	//unsigned short P_LUT[65];
	//unsigned short D_LUT[65];
	unsigned short D_LUT_THRESHOLD[3];
	unsigned short D_LUT_STEP[4];

	//int display_EOTF;

	// UV2 SRGB matrix
	signed short *OCT;
	signed short OCT_709_SDR2MDR[7]  = { 32, 70, 175, 403, -48, -120, 475 };
	signed short OCT_2020_SDR2MDR[7] = { 33, 58, 203, 377, -42, -146, 482 };
	signed short OCT_709_HDR2MDR[7]  = {  0,  0,   0, 202, -24, -60, 238 };
	signed short OCT_2020_HDR2MDR[7] = {  0,  0,   0, 189, -21, -73, 241 };

	// RGB2YUV matrix (fixed)
	int RGBtoYUV_709[9] = { 186, 627, 63, -103, -346, 448, 448, -407, -41 };
	int RGBtoYUV_2020[9] = { 230, 595, 52, -125, -323, 448, 448, -412, -36 };
	int *RGBtoYUV;
	int i;

	if (1/*sys_info->Debug_Buff_8[0] == 1*/) {
#if 1
		/* change endain in SCPU, SCPU is faster than VCPU */
		sl_hdr1_metadata.specVersion 								=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.specVersion, 1) 					;
		sl_hdr1_metadata.payloadMode 								=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.payloadMode, 1) 					;
		sl_hdr1_metadata.hdrPicColourSpace							=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.hdrPicColourSpace, 1)			;
		sl_hdr1_metadata.hdrMasterDisplayColourSpace				=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.hdrMasterDisplayColourSpace, 1)	;
		sl_hdr1_metadata.hdrMasterDisplayMaxLuminance 				=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.hdrMasterDisplayMaxLuminance, 1) ;
		sl_hdr1_metadata.hdrMasterDisplayMinLuminance				=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.hdrMasterDisplayMinLuminance, 1)	;
		sl_hdr1_metadata.sdrPicColourSpace 							=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.sdrPicColourSpace, 1) 			;
		sl_hdr1_metadata.sdrMasterDisplayColourSpace 				=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.sdrMasterDisplayColourSpace, 1) 	;
		sl_hdr1_metadata.u.variables.tmInputSignalBlackLevelOffset	=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.tmInputSignalBlackLevelOffset, 1);
		sl_hdr1_metadata.u.variables.tmInputSignalWhiteLevelOffset	=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.tmInputSignalWhiteLevelOffset, 1);
		sl_hdr1_metadata.u.variables.shadowGain						=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.shadowGain, 1)					;
		sl_hdr1_metadata.u.variables.highlightGain 					=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.highlightGain, 1) 				;
		sl_hdr1_metadata.u.variables.midToneWidthAdjFactor 			=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.midToneWidthAdjFactor, 1) 		;
		sl_hdr1_metadata.u.variables.tmOutputFineTuningNumVal 		=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.tmOutputFineTuningNumVal, 1) 	;
		sl_hdr1_metadata.u.variables.saturationGainNumVal			=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainNumVal, 1)			;

		if (sl_hdr1_metadata.payloadMode == 0) {
			sl_hdr1_metadata.u.variables.saturationGainX[0]				=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainX[0], 1)			;
			sl_hdr1_metadata.u.variables.saturationGainY[0]				=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainY[0], 1)			;
			sl_hdr1_metadata.u.variables.saturationGainX[1]				=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainX[1], 1)			;
			sl_hdr1_metadata.u.variables.saturationGainY[1]				=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainY[1], 1)			;
			sl_hdr1_metadata.u.variables.saturationGainX[2]				=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainX[2], 1)			;
			sl_hdr1_metadata.u.variables.saturationGainY[2]				=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainY[2], 1)			;
			sl_hdr1_metadata.u.variables.saturationGainX[3]				=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainX[3], 1)			;
			sl_hdr1_metadata.u.variables.saturationGainY[3]				=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainY[3], 1)			;
			sl_hdr1_metadata.u.variables.saturationGainX[4]				=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainX[4], 1)			;
			sl_hdr1_metadata.u.variables.saturationGainY[4]				=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainY[4], 1)			;
			sl_hdr1_metadata.u.variables.saturationGainX[5]				=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainX[5], 1)			;
			sl_hdr1_metadata.u.variables.saturationGainY[5]				=		fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainY[5], 1)		;
		} else /*if (sl_hdr1_metadata.payloadMode == 1)*/ {
			for (i=0;i<33;i++) {
				sl_hdr1_metadata.u.tables.luminanceMappingX[i] = fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.tables.luminanceMappingX[i], 1);
				sl_hdr1_metadata.u.tables.luminanceMappingY[i] = fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.tables.luminanceMappingY[i], 1);
			}
		}
#endif
	;
	}else{
		sl_hdr1_metadata.specVersion = 0;
		sl_hdr1_metadata.payloadMode = 0;
		sl_hdr1_metadata.hdrPicColourSpace = 0;
		sl_hdr1_metadata.hdrMasterDisplayColourSpace = 0;
		sl_hdr1_metadata.hdrMasterDisplayMaxLuminance = 1000;
		sl_hdr1_metadata.hdrMasterDisplayMinLuminance = 0;
		sl_hdr1_metadata.sdrPicColourSpace = 0;
		sl_hdr1_metadata.sdrMasterDisplayColourSpace = 0;
		sl_hdr1_metadata.u.variables.tmInputSignalBlackLevelOffset = 101;
		sl_hdr1_metadata.u.variables.tmInputSignalWhiteLevelOffset = 3;
		sl_hdr1_metadata.u.variables.shadowGain = 88;
		sl_hdr1_metadata.u.variables.highlightGain = 157;
		sl_hdr1_metadata.u.variables.midToneWidthAdjFactor = 0;
		sl_hdr1_metadata.u.variables.tmOutputFineTuningNumVal = 0;
		sl_hdr1_metadata.u.variables.saturationGainNumVal = 6;
		sl_hdr1_metadata.u.variables.saturationGainX[0] = 0;
		sl_hdr1_metadata.u.variables.saturationGainY[0] = 64;
		sl_hdr1_metadata.u.variables.saturationGainX[1] = 5;
		sl_hdr1_metadata.u.variables.saturationGainY[1] = 64;
		sl_hdr1_metadata.u.variables.saturationGainX[2] = 245;
		sl_hdr1_metadata.u.variables.saturationGainY[2] = 64;
		sl_hdr1_metadata.u.variables.saturationGainX[3] = 249;
		sl_hdr1_metadata.u.variables.saturationGainY[3] = 64;
		sl_hdr1_metadata.u.variables.saturationGainX[4] = 253;
		sl_hdr1_metadata.u.variables.saturationGainY[4] = 64;
		sl_hdr1_metadata.u.variables.saturationGainX[5] = 255;
		sl_hdr1_metadata.u.variables.saturationGainY[5] = 64;
	}


	// Compute muA and muB
	drvif_fwif_color_set_TC_v130_Compute_muA_muB(&sl_hdr1_metadata, &muA, &muB);

	// Compute C_LUT
	drvif_fwif_color_set_TC_v130_Compute_C_LUT( proc_mode, &sl_hdr1_metadata, display_Brightness, C_LUT);

	  // Compute P_LUT
	drvif_fwif_color_set_TC_v130_Compute_P_LUT( proc_mode, &sl_hdr1_metadata, display_Brightness, P_LUT);

	// Fill D_LUT
	drvif_fwif_color_set_TC_v130_Fill_D_LUT(proc_mode, display_OETF, display_Brightness, D_LUT, D_LUT_THRESHOLD, D_LUT_STEP);

	// OCT matrix pointer
    if (sl_hdr1_metadata.hdrPicColourSpace == 0) // rec709
    {
        OCT = (proc_mode == SDR2MDR) ? OCT_709_SDR2MDR : OCT_709_HDR2MDR;
        RGBtoYUV = RGBtoYUV_709;//juwen, note, use after D LUT. transe DLUT output RGB -> YUV
    }
    else // rec2020
    {
        OCT = (proc_mode == SDR2MDR) ? OCT_2020_SDR2MDR : OCT_2020_HDR2MDR;
        RGBtoYUV = RGBtoYUV_2020;//juwen, note, use after D LUT. transe DLUT output RGB -> YUV
    }

	drvif_fwif_color_set_TC_v130_HW_setting_call_Each_Frame(&muA, &muB, OCT,C_LUT, P_LUT, D_LUT, D_LUT_THRESHOLD, D_LUT_STEP, yuv_range,RGBtoYUV);

#if 0	/* for debug*/
{
	DRV_TC_HDR_CTRL* TC_HDR_CTRL = drvif_fwif_color_Get_TC_CTRL();
	static unsigned int PRINT_CNT = 0;
	unsigned char iii;
	unsigned int printf_Flag = TC_HDR_CTRL->TC_Debug_Log;
	unsigned int print_delay = TC_HDR_CTRL->TC_Debug_Delay;
	//juwen, 0315
	if((printf_Flag&_BIT0)!=0) {
		if(PRINT_CNT%print_delay==0) {
			rtd_pr_vpq_info("juwen,fwif_color_ScalerVIP_set_TC_v130_metadata2reg_frameSync\n\n");
			printk("\n");
			printk("%d           specVersion\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.specVersion ,1));
			printk("%d           payloadMode\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.payloadMode ,1));
			printk("%d           hdrPicColourSpace\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.hdrPicColourSpace ,1));
			printk("%d           hdrMasterDisplayColourSpace\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.hdrMasterDisplayColourSpace ,1));
			printk("%d           hdrMasterDisplayMaxLuminance\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.hdrMasterDisplayMaxLuminance ,1));
			printk("%d           hdrMasterDisplayMinLuminance\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.hdrMasterDisplayMinLuminance ,1));
			printk("%d           sdrPicColourSpace\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.sdrPicColourSpace ,1));
			printk("%d           sdrMasterDisplayColourSpace\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.sdrMasterDisplayColourSpace ,1));
			printk("%d           tmInputSignalBlackLevelOffset\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.tmInputSignalBlackLevelOffset ,1));
			printk("%d           tmInputSignalWhiteLevelOffset\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.tmInputSignalWhiteLevelOffset ,1));
			printk("%d           shadowGain\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.shadowGain ,1));
			printk("%d           highlightGain\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.highlightGain ,1));
			printk("%d           midToneWidthAdjFactor\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.midToneWidthAdjFactor ,1));
			printk("%d           tmOutputFineTuningNumVal\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.tmOutputFineTuningNumVal ,1));
			printk("%d           saturationGainNumVal\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainNumVal, 1));
			printk("%d           saturationGainX[0]\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainX[0] ,1));
			printk("%d           saturationGainY[0]\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainY[0] ,1));
			printk("%d           saturationGainX[1]\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainX[1] ,1));
			printk("%d           saturationGainY[1]\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainY[1] ,1));
			printk("%d           saturationGainX[2]\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainX[2] ,1));
			printk("%d           saturationGainY[2]\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainY[2] ,1));
			printk("%d           saturationGainX[3]\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainX[3] ,1));
			printk("%d           saturationGainY[3]\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainY[3] ,1));
			printk("%d           saturationGainX[4]\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainX[4] ,1));
			printk("%d           saturationGainY[4]\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainY[4] ,1));
			printk("%d           saturationGainX[5]\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainX[5] ,1));
			printk("%d           saturationGainY[5]\n", fwif_color_ChangeOneUINT32Endian(RPC_sys_info->HDR_info.tc_hdr_metadata.variables.saturationGainY[5] ,1));
			printk("\n");

		}
	}
	PRINT_CNT++;
}
#endif


}


void fwif_color_set_TC_v130_metadata2reg_init(int display_OETF, int display_Brightness, int proc_mode, int width, int height, int yuv_range)
{
	unsigned short muA, muB;
	SL_HDR1_metadata sl_hdr1_metadata;
	//unsigned short INV_C_LUT[65];
	//unsigned short C_LUT[65];
	//unsigned short P_LUT[65];
	//unsigned short D_LUT[65];
	unsigned short D_LUT_THRESHOLD[3];
	unsigned short D_LUT_STEP[4];

	// UV2 SRGB matrix
	signed short *OCT;
	signed short OCT_709_SDR2MDR[7]  = { 32, 70, 175, 403, -48, -120, 475 };
	signed short OCT_2020_SDR2MDR[7] = { 33, 58, 203, 377, -42, -146, 482 };
	signed short OCT_709_HDR2MDR[7]  = {  0,  0,   0, 202, -24, -60, 238 };
	signed short OCT_2020_HDR2MDR[7] = {  0,  0,   0, 189, -21, -73, 241 };

	// RGB2YUV matrix (fixed)
	int RGBtoYUV_709[9] = { 186, 627, 63, -103, -346, 448, 448, -407, -41 };
	int RGBtoYUV_2020[9] = { 230, 595, 52, -125, -323, 448, 448, -412, -36 };
	int *RGBtoYUV;

	//int display_EOTF;
	sl_hdr1_metadata.specVersion = 0;
	sl_hdr1_metadata.payloadMode = 0;
	sl_hdr1_metadata.hdrPicColourSpace = 0;
	sl_hdr1_metadata.hdrMasterDisplayColourSpace = 0;
	sl_hdr1_metadata.hdrMasterDisplayMaxLuminance = 1000;
	sl_hdr1_metadata.hdrMasterDisplayMinLuminance = 0;
	sl_hdr1_metadata.sdrPicColourSpace = 0;
	sl_hdr1_metadata.sdrMasterDisplayColourSpace = 0;
	sl_hdr1_metadata.u.variables.tmInputSignalBlackLevelOffset = 101;
	sl_hdr1_metadata.u.variables.tmInputSignalWhiteLevelOffset = 3;
	sl_hdr1_metadata.u.variables.shadowGain = 88;
	sl_hdr1_metadata.u.variables.highlightGain = 157;
	sl_hdr1_metadata.u.variables.midToneWidthAdjFactor = 0;
	sl_hdr1_metadata.u.variables.tmOutputFineTuningNumVal = 0;
	sl_hdr1_metadata.u.variables.saturationGainNumVal = 6;
	sl_hdr1_metadata.u.variables.saturationGainX[0] = 0;
	sl_hdr1_metadata.u.variables.saturationGainY[0] = 64;
	sl_hdr1_metadata.u.variables.saturationGainX[1] = 5;
	sl_hdr1_metadata.u.variables.saturationGainY[1] = 64;
	sl_hdr1_metadata.u.variables.saturationGainX[2] = 245;
	sl_hdr1_metadata.u.variables.saturationGainY[2] = 64;
	sl_hdr1_metadata.u.variables.saturationGainX[3] = 249;
	sl_hdr1_metadata.u.variables.saturationGainY[3] = 64;
	sl_hdr1_metadata.u.variables.saturationGainX[4] = 253;
	sl_hdr1_metadata.u.variables.saturationGainY[4] = 64;
	sl_hdr1_metadata.u.variables.saturationGainX[5] = 255;
	sl_hdr1_metadata.u.variables.saturationGainY[5] = 64;

	//
	// Preparation of "static" LUTs
	//

	// Compute muA and muB
	drvif_fwif_color_set_TC_v130_Compute_muA_muB(&sl_hdr1_metadata, &muA, &muB);

	// Compute C_LUT
	drvif_fwif_color_set_TC_v130_Compute_C_LUT( proc_mode, &sl_hdr1_metadata, display_Brightness, C_LUT);

	// Compute P_LUT
	drvif_fwif_color_set_TC_v130_Compute_P_LUT( proc_mode, &sl_hdr1_metadata, display_Brightness, P_LUT);

	// Fill D_LUT
	drvif_fwif_color_set_TC_v130_Fill_D_LUT(proc_mode, display_OETF, display_Brightness, D_LUT, D_LUT_THRESHOLD, D_LUT_STEP);

	// OCT matrix pointer
    if (sl_hdr1_metadata.hdrPicColourSpace == 0) // rec709
    {
        OCT = (proc_mode == SDR2MDR) ? OCT_709_SDR2MDR : OCT_709_HDR2MDR;
        RGBtoYUV = RGBtoYUV_709;//juwen, note, use after D LUT. transe DLUT output RGB -> YUV
    }
    else // rec2020
    {
        OCT = (proc_mode == SDR2MDR) ? OCT_2020_SDR2MDR : OCT_2020_HDR2MDR;
        RGBtoYUV = RGBtoYUV_2020;//juwen, note, use after D LUT. transe DLUT output RGB -> YUV
    }

	drvif_fwif_color_set_TC_v130_HW_setting_init(&muA, &muB, OCT,C_LUT, P_LUT, D_LUT, D_LUT_THRESHOLD, D_LUT_STEP,yuv_range,RGBtoYUV);

}

void* fwif_color_Get_TC_CTRL(void)
{
	DRV_TC_HDR_CTRL *p;
	p = drvif_fwif_color_Get_TC_CTRL();
	if (p == NULL)
		rtd_pr_vpq_emerg("TC CTRL struc = NULL\n");

	return (void*)p;
}

char fwif_color_ScalerVIP_TC_metadata_frameSync(void)
{
	int display_OETF, display_Brightness, proc_mode, yuv_range;
	_RPC_system_setting_info *RPC_sys_info = NULL;
	DRV_TC_HDR_CTRL *pTC_HDR_CTRL = NULL;

	RPC_sys_info = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	pTC_HDR_CTRL = (DRV_TC_HDR_CTRL*)fwif_color_Get_TC_CTRL();
	if (RPC_sys_info == NULL || pTC_HDR_CTRL == NULL) {
		rtd_pr_vpq_emerg("TC frame sync, point = NULL, syste, info = %lx, TC_HDR_CTRL = %lx\n", (unsigned long)RPC_sys_info, (unsigned long)pTC_HDR_CTRL);

		return -1;
	}

	if (pTC_HDR_CTRL->Enable == 1) {
		display_OETF = pTC_HDR_CTRL->TC_CTRL[TC_HDR_display_OETF];
		display_Brightness = pTC_HDR_CTRL->TC_CTRL[TC_HDR_display_Brightness];
		proc_mode = pTC_HDR_CTRL->TC_CTRL[TC_HDR_display_proc_mode];
		yuv_range = pTC_HDR_CTRL->TC_CTRL[TC_HDR_display_yuv_range];
		fwif_color_ScalerVIP_set_TC_v130_metadata2reg_frameSync(display_OETF, display_Brightness, proc_mode, 0, 0, yuv_range,  RPC_sys_info);
	}

	return 0;
}

void fwif_color_TC_Flow(unsigned char TC_En)
{
	extern dm_dm_submodule_enable_RBUS dm_submodule_temp;
	unsigned int I3DDMA_Color_format, DisplayMode_Src;
	unsigned char I3DDMA_En = 0;

	I3DDMA_Color_format = (unsigned int)drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR);
	DisplayMode_Src = (unsigned int)Get_DisplayMode_Src(SLR_MAIN_DISPLAY);
	I3DDMA_En = fwif_color_get_force_run_i3ddma_enable(SLR_MAIN_DISPLAY);

	rtd_pr_vpq_info("fwif_color_TC_Flow, TC_En=%d, I3DDMA_Color_format=%d, DisplayMode_Src=%d,I3DDMA_En=%d,\n", TC_En, I3DDMA_Color_format, DisplayMode_Src, I3DDMA_En);

	if (TC_En == 1) {
		dm_submodule_temp.regValue = 0;
		drvif_color_set_TC_Run_Mode(1);
		drvif_color_set_TC_Enable(1);

	} else {
#if 0	/* move to fwif_color_set_HDR10_runmode() */
		if ((I3DDMA_Color_format == I3DDMA_COLOR_YUV444)  && (I3DDMA_En == 1))
			drvif_color_set_TC_Run_Mode(1);
		else
			drvif_color_set_TC_Run_Mode(0);

	#endif
		drvif_color_set_TC_Enable(0);
	}
}

char fwif_color_set_TC_FrameSync_Enable(unsigned char Enable_flag)
{
	extern VIP_TCHDR_Mode_TBL TCHDR_Mode_TBL;

	_RPC_system_setting_info *RPC_sys_info = NULL;
	SLR_VOINFO* pVOInfo = NULL;
	DRV_TC_HDR_CTRL *pTC_HDR_CTRL = NULL;

	pTC_HDR_CTRL = (DRV_TC_HDR_CTRL*)fwif_color_Get_TC_CTRL();

	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());

	RPC_sys_info = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if (RPC_sys_info == NULL || pVOInfo == NULL || pTC_HDR_CTRL == NULL) {
		rtd_pr_vpq_emerg("TC frame sync, point = NULL, syste, info = %p, pVOInfo = %p, pTC_HDR_CTRL= %p\n", RPC_sys_info, pVOInfo, pTC_HDR_CTRL);
		return -1;
	}

	if (Enable_flag == 1) {
		if (pVOInfo->video_full_range_flag == 1)
			pTC_HDR_CTRL->TC_CTRL[TC_HDR_display_yuv_range] = TC_YUV_Range_Full;
		else
			pTC_HDR_CTRL->TC_CTRL[TC_HDR_display_yuv_range] = TC_YUV_Range_Limit;

		if (((pVOInfo->colour_primaries == 9) || (pVOInfo->colour_primaries == 2)) && ((pVOInfo->transfer_characteristics == 16) || (pVOInfo->transfer_characteristics == 18)))
			pTC_HDR_CTRL->TC_CTRL[TC_HDR_display_proc_mode] = TC_Proc_Mode_HDR;
		else
			pTC_HDR_CTRL->TC_CTRL[TC_HDR_display_proc_mode] = TC_Proc_Mode_SDR;

		pTC_HDR_CTRL->TC_CTRL[TC_HDR_display_Brightness] = TCHDR_Mode_TBL.display_Brightness;
		pTC_HDR_CTRL->TC_CTRL[TC_HDR_display_OETF] = TCHDR_Mode_TBL.display_OETF;

		pTC_HDR_CTRL->Enable = 1;
	} else {
		pTC_HDR_CTRL->Enable = 0;
	}

	return 0;

}
#endif
#endif
//====================20161027 END=========================================================

/*========================== ST2094==========================================*/
#if 1
char fwif_color_set_ST2094_FrameSync_Enable(unsigned char Enable)
{
	VIP_ST2094_CTRL* ST2094_CTRL = NULL;
	extern VIP_ST2094_TBL ST2094_TBL;
	ST2094_CTRL = drvif_fwif_color_Get_ST2094_Ctrl();

	if (ST2094_CTRL == NULL ) {
		rtd_pr_vpq_emerg("get ST2094 CTRL error, ST2094_CTRL=%p\n",
			ST2094_CTRL);
		return -1;
	}

	ST2094_CTRL->Enable = Enable;

	if (Enable == 1) {
		drvif_fwif_color_Set_ST2094_demo_flag(ST2094_Demo_OFF, ST2094_flow_layer);
		memcpy((unsigned char*)drvif_fwif_color_Get_ST2094_TBL(),  (unsigned char*)&ST2094_TBL, sizeof(VIP_ST2094_TBL));
	}
	return 0;
}

int fwif_color_ScalerVIP_ST2094_demo_flow(void)
{
	//extern unsigned int EOTF_LUT_R[1025];
	//extern unsigned short OETF_LUT_ByMaxCLL_ByPanelLuminance[1025];
	//extern unsigned int hdr_RGB2OPT[HDR_RGB2OPT_TABLE_NUM][9];
	//extern unsigned short tRGB2YUV_COEF[RGB2YUV_COEF_MATRIX_MODE_Max][RGB2YUV_COEF_MATRIX_ITEMS_Max];
	extern unsigned short OETF_ST2094_demo_hdr10[OETF_size];
	extern unsigned int EOTF_ST2094_demo_hdr10[EOTF_size];

	SLR_VIP_TABLE *gVIP_Table;
	unsigned char LC_En, LC_table_sel, vip_src;
	_RPC_system_setting_info *RPC_sys_info;
	_system_setting_info * VIP_system_info_structure_table;
	VIP_ST2094_CTRL* ST2094_CTRL = NULL;
	static unsigned char CNT = 0;
	int ST2094_demo_mode[VIP_ST2094_layer_MAX] = {ST2094_Demo_OFF};
	unsigned char Debug_Log, Debug_log_delay;
	//SLR_VOINFO* pVOInfo = NULL;

	ST2094_CTRL = drvif_fwif_color_Get_ST2094_Ctrl();
	RPC_sys_info = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	gVIP_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	ST2094_demo_mode[ST2094_flow_layer] = drvif_fwif_color_Get_ST2094_demo_flag(ST2094_flow_layer);
	ST2094_demo_mode[ST2094_engMenu_layer] = drvif_fwif_color_Get_ST2094_demo_flag(ST2094_engMenu_layer);

	CNT++;

	if (ST2094_CTRL == NULL || RPC_sys_info == NULL || gVIP_Table == NULL || VIP_system_info_structure_table == NULL) {
		if ((CNT % 100) == 0)
			rtd_pr_vpq_emerg("ST2094 demo flow, ST2094_CTRL=%p, RPC_sys_info=%p, gVIP_Table=%p,VIP_system_info_structure_table=%p \n",
				ST2094_CTRL, RPC_sys_info, gVIP_Table, VIP_system_info_structure_table);
		return -1;
	}

	Debug_Log = ST2094_CTRL->Debug_Log;
	Debug_log_delay = ST2094_CTRL->Debug_Delay;

	if (ST2094_demo_mode[ST2094_engMenu_layer] != ST2094_demo_mode[ST2094_flow_layer]) {
		if (ST2094_demo_mode[ST2094_engMenu_layer] == ST2094_Demo_OFF) {

			drvif_color_set_BBC_Enable(2);
			drvif_DM2_GAMMA_Enable(0);

			vip_src = RPC_sys_info->VIP_source;
			LC_En = gVIP_Table->VIP_QUALITY_Extend_Coef[vip_src][VIP_QUALITY_FUNCTION_LC_Enable];
			LC_table_sel = gVIP_Table->VIP_QUALITY_Extend_Coef[vip_src][VIP_QUALITY_FUNCTION_LC_Table];
			Scaler_SetLocalContrastEnable(LC_En);

			/* set color matrs flow*/
			Scaler_Set_HDR_YUV2RGB(HAL_VPQ_HDR_MODE_ST2094, HAL_VPQ_COLORIMETRY_BT2020);
			//memcpy((unsigned short *)(&VIP_system_info_structure_table->RGB2YUV_COEF_CSMatrix), &(tRGB2YUV_COEF[RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235][0]), sizeof(VIP_RGB2YUV_COEF_CSMatrix));
			//scalerVIP_Write_RGB2YUV(1);
			drvif_color_colorspacergb2yuvtransfer(_CHANNEL1, &(tRGB2YUV_COEF[RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235][0]));

			drvif_fwif_color_Set_ST2094_demo_flag(ST2094_Demo_OFF, ST2094_flow_layer);

		} else if (ST2094_demo_mode[ST2094_engMenu_layer] == ST2094_Demo_ON_HDR10) {
			//pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
			drvif_DM2_EOTF_Set_By_DMA(EOTF_ST2094_demo_hdr10, EOTF_ST2094_demo_hdr10, EOTF_ST2094_demo_hdr10,
				ST2094_CTRL->EOTF_DMA.phy_addr_align, ST2094_CTRL->EOTF_DMA.pVir_addr_align);
			drvif_DM2_OETF_Set_By_DMA(OETF_ST2094_demo_hdr10, OETF_ST2094_demo_hdr10, OETF_ST2094_demo_hdr10,
				ST2094_CTRL->OETF_DMA.phy_addr_align, ST2094_CTRL->OETF_DMA.pVir_addr_align);
			drvif_color_Set_ST2094_3Dlut_CInv(1024, 0);
			drvif_color_set_BBC_Enable(0);
			drvif_DM2_GAMMA_Enable(0);
			drvif_color_Set_HDR_OETF_Decrease(0);

			Scaler_SetLocalContrastEnable(0);

			/* set color matrs flow*/
			Scaler_Set_HDR_YUV2RGB(HAL_VPQ_HDR_MODE_HDR10, HAL_VPQ_COLORIMETRY_BT2020);
			//drvif_color_colorspacergb2yuvtransfer(SLR_MAIN_DISPLAY, &(tRGB2YUV_COEF[RGB2YUV_COEF_RGB_16_235_TO_YCC709_16_235][0]));
			//memcpy((unsigned short *)(&VIP_system_info_structure_table->RGB2YUV_COEF_CSMatrix), &(tRGB2YUV_COEF[RGB2YUV_COEF_RGB_16_235_TO_YCC709_16_235][0]), sizeof(VIP_RGB2YUV_COEF_CSMatrix));
			//scalerVIP_Write_RGB2YUV(1);
			drvif_color_colorspacergb2yuvtransfer(_CHANNEL1, &(tRGB2YUV_COEF[RGB2YUV_COEF_RGB_16_235_TO_YCC709_16_235][0]));

			drvif_fwif_color_Set_ST2094_demo_flag(ST2094_Demo_ON_HDR10, ST2094_flow_layer);

		}
	}


	if((Debug_Log&_BIT2)!=0) {
		if (CNT >= Debug_log_delay) {
			CNT = 0;
			rtd_pr_vpq_info("ST2094 demo mode, flow =%d, eng=%d \n", ST2094_demo_mode[ST2094_flow_layer], ST2094_demo_mode[ST2094_engMenu_layer]);
		}
	}

	return ST2094_demo_mode[ST2094_engMenu_layer];

}

char fwif_color_ScalerVIP_ST2094_frameSync(void)
{
	extern VIP_ST2094_TBL ST2094_TBL;
	VIP_ST2094_CTRL* ST2094_CTRL = NULL;
	static unsigned char CNT = 0;
	_RPC_system_setting_info *RPC_sys_info;
	unsigned char Debug_Log, Debug_log_delay;
	unsigned short v_sta, v_len;
	unsigned short line_start, line_end;
	unsigned int clk_start, clk_end;

	CNT++;
	ST2094_CTRL = drvif_fwif_color_Get_ST2094_Ctrl();
	RPC_sys_info = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if (ST2094_CTRL == NULL || RPC_sys_info == NULL) {
		if ((CNT % 100) == 0)
			rtd_pr_vpq_emerg("ST2094_frameSync error, ST2094_CTRL=%p, RPC_sys_info=%p \n", ST2094_CTRL, RPC_sys_info);
		return -1;
	}

	if (ST2094_CTRL->Enable != 1)	/* is not ST2094 case */
		return 1;

	Debug_Log = ST2094_CTRL->Debug_Log;
	Debug_log_delay = ST2094_CTRL->Debug_Delay;

	if (fwif_color_ScalerVIP_ST2094_demo_flow() == ST2094_Demo_OFF){	/* default setting, demo mode is enabled by engineer menu or tool*/

		if((Debug_Log&_BIT2)!=0) {
			clk_start = drvif_Get_90k_Lo_clk();
			drvif_Get_VGIP_Active_Info(&v_sta, &v_len, &line_start);
		}

		drvif_fwif_color_ScalerVIP_set_ST2094_metadata2reg_frameSync(0, RPC_sys_info, ST2094_CTRL);

		if((Debug_Log&_BIT2)!=0) {
			clk_end = drvif_Get_90k_Lo_clk();
			drvif_Get_VGIP_Active_Info(&v_sta, &v_len, &line_end);

			ST2094_CTRL->Debug_VGIP_Line_sta = line_start;
			ST2094_CTRL->Debug_VGIP_Line_end = line_end;
			ST2094_CTRL->Debug_VGIP_Line_diff = line_end-line_start;
			ST2094_CTRL->Debug_90clk_sta = clk_start;
			ST2094_CTRL->Debug_90clk_end = clk_end;
			ST2094_CTRL->Debug_90clk_diff = clk_end-clk_start;

			if (CNT >= Debug_log_delay) {
				CNT = 0;
				rtd_pr_vpq_info("ST2094 frame sync, 90k clock diff = %d, VGIP lint cnt diff = %d,v_sta=%d, v_len=%d, \n",
					clk_end-clk_start, line_end-line_start, v_sta, v_len);
			}
		}

	}

	return 0;
}

char fwif_color_ST2094_Ctrl_Structure_ini(void)
{
#ifndef BUILD_QUICK_SHOW

	VIP_ST2094_CTRL* ST2094_CTRL = NULL;
	unsigned int eotf_phy_addr = 0, oetf_phy_addr = 0, HDR_3D_LUT_phy_addr = 0;
	unsigned int  *pVir_addr = NULL;
	unsigned int size;
	unsigned char OETF_remap = 0, EOTF_remap = 0/*, HDR_3DLUT_remap = 0*/;
	_RPC_system_setting_info *RPC_sys_info;
	RPC_sys_info = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	ST2094_CTRL = drvif_fwif_color_Get_ST2094_Ctrl();
	eotf_phy_addr = get_query_VIP_DMAto3DTABLE_start_address(DMAto3DTABLE_HDR_EOTF);
	oetf_phy_addr = get_query_VIP_DMAto3DTABLE_start_address(DMAto3DTABLE_HDR_OETF);
	HDR_3D_LUT_phy_addr = get_query_VIP_DMAto3DTABLE_start_address(DMAto3DTABLE_HDR_3D_LUT);

	if (ST2094_CTRL == NULL || eotf_phy_addr == 0 || oetf_phy_addr == 0 || HDR_3D_LUT_phy_addr == 0 || RPC_sys_info == NULL) {
		rtd_pr_vpq_emerg("get ST2094 DMA error, ST2094_CTRL=%p, eotf_phy_addr=%x, oetf_phy_addr=%x, HDR_3D_LUT_phy_addr=%x,RPC_sys_info=%p\n",
			ST2094_CTRL, eotf_phy_addr, oetf_phy_addr, HDR_3D_LUT_phy_addr,RPC_sys_info);
		return -1;
	}

	/* ctrl ini*/
	ST2094_CTRL->Enable = 0;
	ST2094_CTRL->Debug_Log = 0;
	ST2094_CTRL->Debug_Delay = 50;

	/* EOTF DMA */
	if (ST2094_CTRL->EOTF_DMA.pVir_addr_align == NULL) {
		eotf_phy_addr = drvif_memory_get_data_align(eotf_phy_addr, (1 << 12));
		size = drvif_memory_get_data_align(VIP_DMAto3DTable_HDR_EOTF_SIZE, (1 << 12));
		ST2094_CTRL->EOTF_DMA.phy_addr_align = eotf_phy_addr;
		ST2094_CTRL->EOTF_DMA.size = size;
		pVir_addr = dvr_remap_uncached_memory(eotf_phy_addr, size, __builtin_return_address(0));
		ST2094_CTRL->EOTF_DMA.pVir_addr_align = pVir_addr;
		EOTF_remap = 1;
	}

	/* OETF DMA */
	if (ST2094_CTRL->OETF_DMA.pVir_addr_align == NULL) {
		oetf_phy_addr = drvif_memory_get_data_align(oetf_phy_addr, (1 << 12));
		size = drvif_memory_get_data_align(VIP_DMAto3DTable_HDR_OETF_SIZE, (1 << 12));
		ST2094_CTRL->OETF_DMA.phy_addr_align = oetf_phy_addr;
		ST2094_CTRL->OETF_DMA.size = size;
		pVir_addr = dvr_remap_uncached_memory(oetf_phy_addr, size, __builtin_return_address(0));
		ST2094_CTRL->OETF_DMA.pVir_addr_align = pVir_addr;
		OETF_remap = 1;
	}

	rtd_pr_vpq_info("ST2094_ini, EOTF:addr=%x, pVir_addr=%p, size_aliged=%d, remap=%d, OETF:addr=%x, pVir_addr=%p, size_aliged=%d, remap=%d,\n",
		ST2094_CTRL->EOTF_DMA.phy_addr_align, ST2094_CTRL->EOTF_DMA.pVir_addr_align, ST2094_CTRL->EOTF_DMA.size, EOTF_remap,
		ST2094_CTRL->OETF_DMA.phy_addr_align, ST2094_CTRL->OETF_DMA.pVir_addr_align, ST2094_CTRL->OETF_DMA.size, OETF_remap);

#ifdef CONFIG_BW_96B_ALIGNED
	/* EOTF DMA */
	if (EOTF_remap == 1) {
		ST2094_CTRL->EOTF_DMA.size = dvr_size_alignment(ST2094_CTRL->EOTF_DMA.size);
		ST2094_CTRL->EOTF_DMA.phy_addr_align = (unsigned int)dvr_memory_alignment((unsigned long)ST2094_CTRL->EOTF_DMA.phy_addr_align, ST2094_CTRL->EOTF_DMA.size);
		ST2094_CTRL->EOTF_DMA.pVir_addr_align = (unsigned int*)dvr_memory_alignment((unsigned long)ST2094_CTRL->EOTF_DMA.pVir_addr_align, ST2094_CTRL->EOTF_DMA.size);
		if ((ST2094_CTRL->EOTF_DMA.phy_addr_align == 0) || (ST2094_CTRL->EOTF_DMA.pVir_addr_align == NULL)) {
			rtd_pr_vpq_emerg("ST2094, EOTF 96 aligned fail, addr=%x, pVir_addr=%p,\n",
				ST2094_CTRL->EOTF_DMA.phy_addr_align, ST2094_CTRL->EOTF_DMA.pVir_addr_align);
			BUG();
		}
	}

	/* OETF DMA */
	if (OETF_remap == 1) {
		ST2094_CTRL->OETF_DMA.size = dvr_size_alignment(ST2094_CTRL->OETF_DMA.size);
		ST2094_CTRL->OETF_DMA.phy_addr_align = (unsigned int)dvr_memory_alignment((unsigned long)ST2094_CTRL->OETF_DMA.phy_addr_align, ST2094_CTRL->OETF_DMA.size);
		ST2094_CTRL->OETF_DMA.pVir_addr_align = (unsigned int*)dvr_memory_alignment((unsigned long)ST2094_CTRL->OETF_DMA.pVir_addr_align, ST2094_CTRL->OETF_DMA.size);
		if ((ST2094_CTRL->OETF_DMA.phy_addr_align == 0) || (ST2094_CTRL->OETF_DMA.pVir_addr_align == NULL)) {
			rtd_pr_vpq_emerg("ST2094, OETF 96 aligned fail, addr=%x, pVir_addr=%p,\n",
				ST2094_CTRL->OETF_DMA.phy_addr_align, ST2094_CTRL->OETF_DMA.pVir_addr_align);
			BUG();
		}
	}

	rtd_pr_vpq_info("ST2094_ini, 96B aligned, EOTF:addr=%x, pVir_addr=%p, size_aliged=%d, remap=%d, OETF:addr=%x, pVir_addr=%p, size_aliged=%d, remap=%d,\n",
		ST2094_CTRL->EOTF_DMA.phy_addr_align, ST2094_CTRL->EOTF_DMA.pVir_addr_align, ST2094_CTRL->EOTF_DMA.size, EOTF_remap,
		ST2094_CTRL->OETF_DMA.phy_addr_align, ST2094_CTRL->OETF_DMA.pVir_addr_align, ST2094_CTRL->OETF_DMA.size, OETF_remap);
#endif

#if 0
	/* 3D LUT DMA */
	if (ST2094_CTRL->HDR_3D_LUT_DMA.pVir_addr_align == NULL) {
		HDR_3D_LUT_phy_addr = drvif_memory_get_data_align(HDR_3D_LUT_phy_addr, (1 << 12));
		size = drvif_memory_get_data_align(VIP_DMAto3DTable_HDR_3D_LUT_SIZE, (1 << 12));
		ST2094_CTRL->HDR_3D_LUT_DMA.phy_addr_align = HDR_3D_LUT_phy_addr;
		ST2094_CTRL->HDR_3D_LUT_DMA.size = size;
		pVir_addr = dvr_remap_uncached_memory(HDR_3D_LUT_phy_addr, size, __builtin_return_address(0));
		ST2094_CTRL->HDR_3D_LUT_DMA.pVir_addr_align = pVir_addr;
	}
#endif
#endif //BUILD_QUICK_SHOW end
	return 0;
}

#endif
/*========================== HDR10 +==========================================*/

#ifdef CONFIG_HW_SUPPORT_SLD/*SLD, hack, elieli*/
//juwen, 0603, SLD
void fwif_color_set_SLD_Enable(unsigned char src_idx,bool enable)
{
	drvif_color_set_SLD_Enable(enable);
}

void fwif_color_set_SLD_Ctrl(unsigned char src_idx, DRV_Still_Logo_Detection_Table* ptr)
{
	DRV_SLD_Ctrl data;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if (VIP_system_info_structure_table->PQ_Setting_Info.SLD_INFO.SLD_Size.sld_width > 2000) /* 4K */
		ptr->SLD_Ctrl.sld_scale_mode = 1;
	else
		ptr->SLD_Ctrl.sld_scale_mode = 0;

	data.sld_en= ptr->SLD_Ctrl.sld_en;//juwen, enable not put in table?delete ?
	data.sld_blend_en= ptr->SLD_Ctrl.sld_blend_en;
	data.sld_global_drop= ptr->SLD_Ctrl.sld_global_drop;
	data.sld_hpf_type= ptr->SLD_Ctrl.sld_hpf_type;
	data.sld_hpf_thl = ptr->SLD_Ctrl.sld_hpf_thl;
	data.sld_scale_mode = ptr->SLD_Ctrl.sld_scale_mode;

	drvif_color_set_SLD_control((DRV_SLD_Ctrl*) &data);
}

void fwif_color_set_SLD_input_size(unsigned short sld_width, unsigned short sld_height)
{
	DRV_SLD_Size data;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	data.sld_height = sld_height;
	data.sld_width = sld_width;

	VIP_system_info_structure_table->PQ_Setting_Info.SLD_INFO.SLD_Size.sld_height = fwif_color_ChangeOneUINT16Endian(sld_height, 0);
	VIP_system_info_structure_table->PQ_Setting_Info.SLD_INFO.SLD_Size.sld_width= fwif_color_ChangeOneUINT16Endian(sld_width, 0);

	drvif_color_set_SLD_input_size( (DRV_SLD_Size*) &data);
}


void fwif_color_set_SLD_curve_map(unsigned char src_idx, DRV_Still_Logo_Detection_Table* ptr)
{
	DRV_SLD_CurveMap data;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	data.sld_curve_seg_0= ptr->SLD_CurveMap.sld_curve_seg_0;
	data.sld_curve_seg_1= ptr->SLD_CurveMap.sld_curve_seg_1;

	data.sld_curve_gain_0= ptr->SLD_CurveMap.sld_curve_gain_0;
	data.sld_curve_gain_1= ptr->SLD_CurveMap.sld_curve_gain_1;
	data.sld_curve_gain_2= ptr->SLD_CurveMap.sld_curve_gain_2;

	data.sld_curve_offset_0= ptr->SLD_CurveMap.sld_curve_offset_0;
	data.sld_curve_offset_1= ptr->SLD_CurveMap.sld_curve_offset_1;
	data.sld_curve_offset_2= ptr->SLD_CurveMap.sld_curve_offset_2;


	drvif_color_set_SLD_curve_map((DRV_SLD_CurveMap*) &data);
}

void fwif_color_set_SLD_drop(unsigned char src_idx, DRV_Still_Logo_Detection_Table* ptr)
{
	DRV_SLD_DROP data;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	data.sld_cnt_thl= ptr->SLD_DROP.sld_cnt_thl;
	data.sld_hpf_diff_thl= ptr->SLD_DROP.sld_hpf_diff_thl;
	data.sld_pixel_diff_thl= ptr->SLD_DROP.sld_pixel_diff_thl;

	drvif_color_set_SLD_drop((DRV_SLD_DROP*) &data);
}
#endif //CONFIG_HW_SUPPORT_SLD

void fwif_color_set_BOE_RGBW(int w_rate)
{
	return;
	// 20170531 henry merlin3 remove
	// 20160627 jyyang
//	extern unsigned short BOE_RGBW_TAB_160713[16128];
//	drvif_color_set_BOE_RGBW(BOE_RGBW_TAB_160713, w_rate);
}

void fwif_color_set_RGBW_histogram(int *hist)
{
	rgbw_d_rgbw_adaptwhite_1_RBUS rgbw_d_rgbw_adaptwhite_1_REG;
	rgbw_d_rgbw_adaptwhite_2_RBUS rgbw_d_rgbw_adaptwhite_2_REG;
	rgbw_d_rgbw_adaptwhite_3_RBUS rgbw_d_rgbw_adaptwhite_3_REG;
	rgbw_d_rgbw_adaptwhite_4_RBUS rgbw_d_rgbw_adaptwhite_4_REG;

	rgbw_d_rgbw_adaptwhite_hist_1_RBUS rgbw_d_rgbw_adaptwhite_hist_1_REG;
	rgbw_d_rgbw_adaptwhite_hist_2_RBUS rgbw_d_rgbw_adaptwhite_hist_2_REG;
	rgbw_d_rgbw_adaptwhite_hist_3_RBUS rgbw_d_rgbw_adaptwhite_hist_3_REG;
	rgbw_d_rgbw_adaptwhite_hist_4_RBUS rgbw_d_rgbw_adaptwhite_hist_4_REG;
	int shiftb;

	rgbw_d_rgbw_adaptwhite_1_REG.regValue = IoReg_Read32(RGBW_D_RGBW_ADAPTWHITE_1_reg);
	rgbw_d_rgbw_adaptwhite_2_REG.regValue = IoReg_Read32(RGBW_D_RGBW_ADAPTWHITE_2_reg);
	rgbw_d_rgbw_adaptwhite_3_REG.regValue = IoReg_Read32(RGBW_D_RGBW_ADAPTWHITE_3_reg);
	rgbw_d_rgbw_adaptwhite_4_REG.regValue = IoReg_Read32(RGBW_D_RGBW_ADAPTWHITE_4_reg);
	rgbw_d_rgbw_adaptwhite_hist_1_REG.regValue = IoReg_Read32(RGBW_D_RGBW_ADAPTWHITE_HIST_1_reg);
	rgbw_d_rgbw_adaptwhite_hist_2_REG.regValue = IoReg_Read32(RGBW_D_RGBW_ADAPTWHITE_HIST_2_reg);
	rgbw_d_rgbw_adaptwhite_hist_3_REG.regValue = IoReg_Read32(RGBW_D_RGBW_ADAPTWHITE_HIST_3_reg);
	rgbw_d_rgbw_adaptwhite_hist_4_REG.regValue = IoReg_Read32(RGBW_D_RGBW_ADAPTWHITE_HIST_4_reg);

	shiftb = 14 - rgbw_d_rgbw_adaptwhite_2_REG.hist_shift_sel;

	hist[0] = rgbw_d_rgbw_adaptwhite_hist_1_REG.hist_bin0>>shiftb;
	hist[1] = rgbw_d_rgbw_adaptwhite_hist_1_REG.hist_bin1>>shiftb;
	hist[2] = rgbw_d_rgbw_adaptwhite_hist_2_REG.hist_bin2>>shiftb;
	hist[3] = rgbw_d_rgbw_adaptwhite_hist_2_REG.hist_bin3>>shiftb;
	hist[4] = rgbw_d_rgbw_adaptwhite_hist_3_REG.hist_bin4>>shiftb;
	hist[5] = rgbw_d_rgbw_adaptwhite_hist_3_REG.hist_bin5>>shiftb;
	hist[6] = rgbw_d_rgbw_adaptwhite_hist_4_REG.hist_bin6>>shiftb;
	hist[7] = rgbw_d_rgbw_adaptwhite_hist_4_REG.hist_bin7>>shiftb;
}

void fwif_color_set_RGBW_adaptive_white(int hist[9][8])
{
	rgbw_d_rgbw_adaptwhite_1_RBUS rgbw_d_rgbw_adaptwhite_1_REG;
	rgbw_d_rgbw_adaptwhite_2_RBUS rgbw_d_rgbw_adaptwhite_2_REG;
	rgbw_d_rgbw_adaptwhite_3_RBUS rgbw_d_rgbw_adaptwhite_3_REG;
	rgbw_d_rgbw_adaptwhite_4_RBUS rgbw_d_rgbw_adaptwhite_4_REG;
	int i,j;
	int wt_sum = 0;
	int hist_wt[9][8] = {
			{1, 3, 7, 7, 9, 11, 13, 13,},   // max-min
			{0, 4, 8, 12, 16, 20, 23, 27,}, // red
			{0, 5, 10, 17, 19, 22, 24, 27,},// green
			{0, 4, 8, 12, 16, 20, 23, 27,}, // blue
			{0, 9, 18, 27, 27, 27, 27, 27,},// cyan
			{0, 0, 0, 0, 7, 14, 21, 27,},   // meganta
			{0, 4, 10, 17, 21, 25, 29, 34,},// yellow
			{0, 0, 0, 0, 0, 0, 0, 0,},      // white
			{0, 0, 0, 5, 10, 12, 15, 17,},  // min(Sat, min)
		};
	int wc_grids[33] = {0,128,256,380,450,490,515,540,560,579,597,613,628,642,655,667,678,688,697,705,713,721,729,737,745,733,761,769,777,785,793,801,809};
	int wc_idx;
	int wc_ph;
	int wc_out;

	rgbw_d_rgbw_adaptwhite_1_REG.regValue = IoReg_Read32(RGBW_D_RGBW_ADAPTWHITE_1_reg);
	rgbw_d_rgbw_adaptwhite_2_REG.regValue = IoReg_Read32(RGBW_D_RGBW_ADAPTWHITE_2_reg);
	rgbw_d_rgbw_adaptwhite_3_REG.regValue = IoReg_Read32(RGBW_D_RGBW_ADAPTWHITE_3_reg);
	rgbw_d_rgbw_adaptwhite_4_REG.regValue = IoReg_Read32(RGBW_D_RGBW_ADAPTWHITE_4_reg);

	for(i=0;i<9;i++) {
	for(j=0;j<8;j++) {
			wt_sum+=hist[i][j]*hist_wt[i][j];
		}
	}
	// w_cut mapping // 33 grid = X(0~4096) Y(0~1024)
	wc_idx = wt_sum/128;
	wc_ph = wt_sum - wc_idx*128;
	wc_out = wc_grids[wc_idx] + (wc_grids[wc_idx+1]-wc_grids[wc_idx])*wc_ph/128;
	wc_out = (wc_out<0)?0:((wc_out>1023)?1023:wc_out);
	rgbw_d_rgbw_adaptwhite_1_REG.adaptwhite_cutweight = wc_out;
	IoReg_Write32(RGBW_D_RGBW_ADAPTWHITE_1_reg, rgbw_d_rgbw_adaptwhite_1_REG.regValue);

}



void fwif_color_colorwrite_Output_gamma(unsigned char value)
{
	unsigned char fmt;
	unsigned char display = 0;
	unsigned char src_idx = 0;
	VIP_Output_InvOutput_Gamma_CTRL_ITEM *prt_CTRL;

	Scaler_Get_Display_info(&display, &src_idx);

	if (value >= Output_InvOutput_Gamma_TBL_MAX) {
		drvif_color_set_output_gamma_format(0, 0, 0, 0);
		return;
	}

	prt_CTRL = &Output_InvOutput_Gamma[value].Output_InvOutput_Gamma_CTRL_ITEM;

	if (prt_CTRL->output_Gamma_En != 1) {
		drvif_color_set_output_gamma_format(0, 0, 0, 0);
		return;
	}

	fmt = drvif_color_Get_RGBW_Mode();

	if (prt_CTRL->output_Location == 1 && (fmt == 1 || fmt == 2))
		fmt = 1;
	else
		fmt = 0;

	drvif_color_output_gamma_control_front(display);
	drvif_color_colorwrite_output_gamma(&Output_InvOutput_Gamma[value].Output_Gamma_TBL.TBL_R[0], GAMMA_CHANNEL_R);
	drvif_color_colorwrite_output_gamma(&Output_InvOutput_Gamma[value].Output_Gamma_TBL.TBL_G[0], GAMMA_CHANNEL_G);
	drvif_color_colorwrite_output_gamma(&Output_InvOutput_Gamma[value].Output_Gamma_TBL.TBL_B[0], GAMMA_CHANNEL_B);
	drvif_color_colorwrite_output_gamma(&Output_InvOutput_Gamma[value].Output_Gamma_TBL.TBL_W[0], GAMMA_CHANNEL_W);
	drvif_color_output_gamma_control_back(display);

	drvif_color_set_output_gamma_format(prt_CTRL->output_Gamma_En, fmt, prt_CTRL->output_Location, 0);

}

unsigned char outputgamma_format = 0;
void fwif_color_read_output_gamma(unsigned char value)
{
	outputgamma_out_gamma_ctrl_1_RBUS outputgamma_out_gamma_ctrl_1_reg;
	outputgamma_out_gamma_ctrl_2_RBUS outputgamma_out_gamma_ctrl_2_reg;
	outputgamma_out_gamma_ctrl_1_reg.regValue = IoReg_Read32(OUTPUTGAMMA_OUT_GAMMA_CTRL_1_reg);
	outputgamma_out_gamma_ctrl_2_reg.regValue = IoReg_Read32(OUTPUTGAMMA_OUT_GAMMA_CTRL_2_reg);

	Output_InvOutput_Gamma[value].Output_InvOutput_Gamma_CTRL_ITEM.output_Gamma_En = outputgamma_out_gamma_ctrl_1_reg.out_gamma_en;
	Output_InvOutput_Gamma[value].Output_InvOutput_Gamma_CTRL_ITEM.output_Location= outputgamma_out_gamma_ctrl_1_reg.out_gamma_location;
	outputgamma_out_gamma_ctrl_1_reg.out_gamma_en = 0;//
	IoReg_Write32(OUTPUTGAMMA_OUT_GAMMA_CTRL_1_reg, outputgamma_out_gamma_ctrl_1_reg.regValue);//
	outputgamma_format = outputgamma_out_gamma_ctrl_2_reg.out_gamma_format;

	drvif_color_colorRead_outGamma(&Output_InvOutput_Gamma[value].Output_Gamma_TBL.TBL_R[0], GAMMA_CHANNEL_R);
	drvif_color_colorRead_outGamma(&Output_InvOutput_Gamma[value].Output_Gamma_TBL.TBL_G[0], GAMMA_CHANNEL_G);
	drvif_color_colorRead_outGamma(&Output_InvOutput_Gamma[value].Output_Gamma_TBL.TBL_B[0], GAMMA_CHANNEL_B);
	drvif_color_colorRead_outGamma(&Output_InvOutput_Gamma[value].Output_Gamma_TBL.TBL_W[0], GAMMA_CHANNEL_W);
	outputgamma_out_gamma_ctrl_1_reg.out_gamma_en = Output_InvOutput_Gamma[value].Output_InvOutput_Gamma_CTRL_ITEM.output_Gamma_En;//
	IoReg_Write32(OUTPUTGAMMA_OUT_GAMMA_CTRL_1_reg, outputgamma_out_gamma_ctrl_1_reg.regValue);//
#if 0
	drvif_color_colorRead_outGamma(Output_InvOutput_Gamma[Output_InvOutput_Gamma_TBL_TV030].Output_Gamma_TBL.TBL_R, GAMMA_CHANNEL_R);
	drvif_color_colorRead_outGamma(Output_InvOutput_Gamma[Output_InvOutput_Gamma_TBL_TV030].Output_Gamma_TBL.TBL_G, GAMMA_CHANNEL_G);
	drvif_color_colorRead_outGamma(Output_InvOutput_Gamma[Output_InvOutput_Gamma_TBL_TV030].Output_Gamma_TBL.TBL_B, GAMMA_CHANNEL_B);
	drvif_color_colorRead_outGamma(Output_InvOutput_Gamma[Output_InvOutput_Gamma_TBL_TV030].Output_Gamma_TBL.TBL_W, GAMMA_CHANNEL_W);
#endif
}


void fwif_color_colorwrite_InvOutput_gamma(unsigned char value)
{
	unsigned char display = 0;
	unsigned char src_idx = 0;
	VIP_Output_InvOutput_Gamma_CTRL_ITEM *prt_CTRL;

	Scaler_Get_Display_info(&display, &src_idx);

	if (value >= Output_InvOutput_Gamma_TBL_MAX) {
		drvif_color_colorwrite_inv_output_gamma_Enable(0);
		return;
	}

	prt_CTRL = &Output_InvOutput_Gamma[value].Output_InvOutput_Gamma_CTRL_ITEM;

	if (prt_CTRL->InvOutput_Gamma_En != 1) {
		drvif_color_colorwrite_inv_output_gamma_Enable(0);
		return;
	}

	drvif_color_inv_output_gamma_control_front(display);
	drvif_color_colorwrite_inv_output_gamma(&Output_InvOutput_Gamma[value].InvOutput_Gamma_TBL.TBL_R[0], GAMMA_CHANNEL_R);
	drvif_color_colorwrite_inv_output_gamma(&Output_InvOutput_Gamma[value].InvOutput_Gamma_TBL.TBL_G[0], GAMMA_CHANNEL_G);
	drvif_color_colorwrite_inv_output_gamma(&Output_InvOutput_Gamma[value].InvOutput_Gamma_TBL.TBL_B[0], GAMMA_CHANNEL_B);
	drvif_color_output_gamma_control_back(display);

	drvif_color_colorwrite_inv_output_gamma_Enable(prt_CTRL->InvOutput_Gamma_En);

}

void fwif_color_rtice_DM2_EOTF_Set(unsigned int *EOTF_table32_R, unsigned int *EOTF_table32_G, unsigned int *EOTF_table32_B)
{
	drvif_DM2_B0203_noSram_Enable(1);
	drvif_DM2_EOTF_Enable(1, 1);
#if 0	/* use  DMA driver writing*/
	drvif_DM2_EOTF_Set(EOTF_table32_R, EOTF_table32_G, EOTF_table32_B);
#else
	fwif_color_DM2_EOTF_Set_By_DMA(EOTF_table32_R, EOTF_table32_G, EOTF_table32_B);
#endif
}

void fwif_color_rtice_DM2_EOTF_Get(unsigned int *EOTF_table32_R, unsigned int *EOTF_table32_G, unsigned int *EOTF_table32_B)
{
	drvif_DM2_B0203_noSram_Enable(1);
	drvif_DM2_EOTF_Enable(1, 1);
	drvif_DM2_EOTF_Get(EOTF_table32_R, EOTF_table32_G, EOTF_table32_B);
	//printk("\n[eli]re=%d,%d,%d\n", EOTF_table32_R[1], EOTF_table32_G[2], EOTF_table32_B[3]);
}

void fwif_color_rtice_DM2_OETF_Set(unsigned short *OETF_table16_R, unsigned short *OETF_table16_G, unsigned short *OETF_table16_B)
{
	drvif_DM2_B0203_noSram_Enable(1);
	drvif_DM2_OETF_Enable(1, 1);
#if 0	/* use  DMA driver writing*/
	drvif_DM2_OETF_Set(OETF_table16_R, OETF_table16_G, OETF_table16_B);
#else
	fwif_color_DM2_OETF_Set_By_DMA(OETF_table16_R, OETF_table16_G, OETF_table16_B);
#endif
}

void fwif_color_rtice_DM2_OETF_Get(unsigned short *OETF_table16_R, unsigned short *OETF_table16_G, unsigned short *OETF_table16_B)
{
	drvif_DM2_B0203_noSram_Enable(1);
	drvif_DM2_OETF_Enable(1, 1);
	drvif_DM2_OETF_Get(OETF_table16_R, OETF_table16_G, OETF_table16_B);

}

#ifdef CONFIG_HW_SUPPORT_HDR_TONE //mac6 removed HDR tone mapping
char fwif_color_rtice_DM2_ToneMapping_Set(short *TM_table12_R, short *TM_table12_G, short *TM_table12_B)
{
	if (drvif_Wait_VGIP_start(1, 50, 10000) < 0)
		return -1;
	if (drvif_DM2_ToneMapping_Enable(1, 1) < 0)
		return -1;
	if (drvif_DM2_ToneMapping_Set(TM_table12_R, TM_table12_G, TM_table12_B) < 0)
		return -1;
	return 0;
}

char fwif_color_rtice_DM2_ToneMapping_Get(short *TM_table12_R, short *TM_table12_G, short *TM_table12_B)
{
	if (drvif_Wait_VGIP_start(1, 50, 10000) < 0)
		return -1;
	if (drvif_DM2_ToneMapping_Enable(1, 1) < 0)
		return -2;
	if (drvif_DM2_ToneMapping_Get(TM_table12_R, TM_table12_G, TM_table12_B) < 0)
		return -3;
	return 0;
}
#endif // CONFIG_HW_SUPPORT_HDR_TONE

#ifdef CONFIG_HW_SUPPORT_HDR_3DLUT_24
char fwif_color_rtice_DM2_3D_24x24x24_LUT_Set(unsigned char enable, unsigned short *pArray)
{
	extern UINT16 HDR_3DLUT_Compact[HDR_24x24x24_size*3];
	/*_system_setting_info *system_setting_info = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);*/
	char ret;
	static PQ_HDR_3dLUT_16_PARAM args;
	args.c3dLUT_16_enable = enable;
	args.a3dLUT_16_pArray = pArray;

	/*ret = drvif_Wait_VGIP_start(1, 50, 10000);
	if (ret < 0)
		return -1;*/
	if (fwif_HDR_compare_3DLUT((void*) &args, NEW_24x24x24) != 1) {
		rtd_pr_vpq_emerg("Rtice HDR 3D LUT same\n");
		//return 0;
	}

	fwif_color_DM2_3D_LUT_BIT_Change(pArray, &HDR_3DLUT_Compact[0], NEW_24x24x24);
#if 0	/*use DMA drvier*/
	ret = drvif_Set_DM_HDR_3dLUT_24x24x24_16(enable, &HDR10_3DLUT_Compact[0]);
#else
	ret = fwif_color_DM2_3D_24x24x24_LUT_Set_By_DMA(enable, &HDR_3DLUT_Compact[0]);
#endif
	if (ret < 0)
		return -1;

	return ret;
}

char fwif_color_rtice_DM2_3D_24x24x24_LUT_Get(unsigned short *pArray, unsigned char getFromReg)
{
	char ret = 0;
	unsigned int i;
	/*ret = drvif_Wait_VGIP_start(1, 50, 10000);
	if (ret < 0)
		return -1;*/

	if (getFromReg == 1) {
		ret = drvif_Get_DM_HDR_3dLUT_24x24x24_16(pArray);
		if (ret < 0)
			return -1;

		/* return from compact form*/
		for (i=0;i<(HDR_24x24x24_size*3);i++)
			pArray[i] = pArray[i]<<4;
	} else {
		memcpy((unsigned char*)pArray, (unsigned char*)pre_hdr_3dlut, sizeof(short)*(HDR_24x24x24_size*3));
	}

	return ret;
}
#endif // CONFIG_HW_SUPPORT_HDR_3DLUT_24

char fwif_color_DM2_3D_LUT_BIT_Change(unsigned short *pArray, unsigned short *outArray, VIP_HDR10_CSC3_TABLE_Mode mode)
{
	unsigned int HDR3DLUT_ZIZE, i;
	if (mode == OLD_17x17x17)
		HDR3DLUT_ZIZE = VIP_DM_HDR_3D_LUT_UI_TBL_SIZE;
	else if (mode == NEW_24x24x24)
		HDR3DLUT_ZIZE = HDR_24x24x24_size*3;

	if (pArray == NULL || outArray == NULL) {
		rtd_pr_vpq_emerg("NULL point in DM2_3D_LUT_BIT_Change\n");
		return -1;
	}

	for (i=0;i<HDR3DLUT_ZIZE;i++)
		outArray[i] = pArray[i]>>4;

	return 0;
}

char fwif_color_DM2_EOTF_Set_By_DMA(unsigned int *EOTF_table32_R, unsigned int *EOTF_table32_G, unsigned int *EOTF_table32_B)
{
#ifndef BUILD_QUICK_SHOW
	char ret;
	unsigned int addr = 0, size;
	unsigned int *pVir_addr = 0;
	unsigned short v_sta, v_len, line_cnt, v_end;
	unsigned short delay1, delay2;
	delay1 = 30;

	addr = get_query_VIP_DMAto3DTABLE_start_address(DMAto3DTABLE_HDR_EOTF);
	addr = drvif_memory_get_data_align(addr, (1 << 12));
	size = drvif_memory_get_data_align(VIP_DMAto3DTable_HDR_EOTF_SIZE, (1 << 12));
	if (addr == 0) {
		rtd_pr_vpq_emerg("fwif_color_DM2_EOTF_Set_By_DMA, addr = NULL\n");
		return -1;
	}

	ret = drvif_Get_VO_Active_Info(&v_sta, &v_end, &line_cnt);
	if (ret < 0) {
		rtd_pr_vpq_emerg("fwif_color_DM2_EOTF_Set_By_DMA is not ready, ret = %d\n", ret);
		//return -1;
	} else {
		/* wait for active area to write DMA*/
		v_len = ((v_end - v_sta)<0)?(0):((v_end - v_sta));
		delay2 = delay1 + (v_len >> 1);
		ret = drvif_Wait_VO_start(delay1, v_len, 0x032500);
		if (ret < 0) {
			rtd_pr_vpq_emerg("fwif_color_DM2_EOTF_Set_By_DMA fail to wait for active, ret = %d, v_end=%d, v_sta=%d, delay1=%d, delay2=%d,\n",
				ret, v_end, v_sta, delay1, delay2);
			//return -1;
		}
	}
	pVir_addr = (unsigned int*)dvr_remap_uncached_memory(addr, size, __builtin_return_address(0));
	ret = drvif_DM2_EOTF_Set_By_DMA(EOTF_table32_R, EOTF_table32_G, EOTF_table32_B, addr, pVir_addr);
	dvr_unmap_memory((void *)pVir_addr, size);

	return ret;
#else
	return 0;
#endif
}

char fwif_color_DM2_OETF_Set_By_DMA(unsigned short *OETF_table16_R, unsigned short *OETF_table16_G, unsigned short *OETF_table16_B)
{
#ifndef BUILD_QUICK_SHOW

	char ret;
	unsigned int addr = 0, size;
	unsigned int *pVir_addr = 0;
	unsigned short v_sta, v_len, line_cnt, v_end;
	unsigned short delay1, delay2;
	delay1 = 30;

	addr = get_query_VIP_DMAto3DTABLE_start_address(DMAto3DTABLE_HDR_OETF);
	addr = drvif_memory_get_data_align(addr, (1 << 12));
	size = drvif_memory_get_data_align(VIP_DMAto3DTable_HDR_OETF_SIZE, (1 << 12));
	if (addr == 0) {
		rtd_pr_vpq_emerg("fwif_color_DM2_OETF_Set_By_DMA, addr = NULL\n");
		return -1;
	}

	ret = drvif_Get_VO_Active_Info(&v_sta, &v_end, &line_cnt);
	if (ret < 0) {
		rtd_pr_vpq_emerg("fwif_color_DM2_OETF_Set_By_DMA is not ready, ret = %d\n", ret);
		//return -1;
	} else {
		/* wait for active area to write DMA*/
		v_len = ((v_end - v_sta)<0)?(0):((v_end - v_sta));
		delay2 = delay1 + (v_len >> 1);
		ret = drvif_Wait_VO_start(delay1, v_len, 0x032500);
		if (ret < 0) {
			rtd_pr_vpq_emerg("fwif_color_DM2_OETF_Set_By_DMA fail to wait for active, ret = %d, v_end=%d, v_sta=%d, delay1=%d, delay2=%d,\n",
				ret, v_end, v_sta, delay1, delay2);
			return -1;
		}
	}
	pVir_addr = (unsigned int*)dvr_remap_uncached_memory(addr, size, __builtin_return_address(0));
	 ret = drvif_DM2_OETF_Set_By_DMA(OETF_table16_R, OETF_table16_G, OETF_table16_B, addr, pVir_addr);
	dvr_unmap_memory((void *)pVir_addr, size);

	return ret;
#else

	return 0;
#endif
	
}

#ifdef CONFIG_HW_SUPPORT_HDR_3DLUT_24
char fwif_color_DM2_3D_24x24x24_LUT_Set_By_DMA(unsigned char enable, unsigned short *pArray)
{
	char ret;
	unsigned int addr = 0, size;
	unsigned int *pVir_addr = 0;
	unsigned short v_sta, v_len, line_cnt, v_end;
	unsigned short delay1, delay2;
	delay1 = 30;

	addr = get_query_VIP_DMAto3DTABLE_start_address(DMAto3DTABLE_HDR_3D_LUT);
	addr = drvif_memory_get_data_align(addr, (1 << 12));
	size = drvif_memory_get_data_align(VIP_DMAto3DTable_HDR_3D_LUT_SIZE, (1 << 12));
	if (addr == 0) {
		rtd_pr_vpq_emerg("fwif_color_DM2_3D_24x24x24_LUT_Set_By_DMA, addr = NULL\n");
		return -1;
	}

	ret = drvif_Get_VO_Active_Info(&v_sta, &v_end, &line_cnt);
	if (ret < 0) {
		rtd_pr_vpq_emerg("fwif_color_DM2_3D_24x24x24_LUT_Set_By_DMA is not ready, ret = %d\n", ret);
		//return -1;
	} else {
		/* wait for active area to write DMA*/
		v_len = ((v_end - v_sta)<0)?(0):((v_end - v_sta));
		delay2 = delay1 + (v_len >> 1);
		ret = drvif_Wait_VO_start(delay1, v_len, 0x032500);
		if (ret < 0) {
			rtd_pr_vpq_emerg("fwif_color_DM2_3D_24x24x24_LUT_Set_By_DMA fail to wait for active, ret = %d, v_end=%d, v_sta=%d, delay1=%d, delay2=%d,\n",
				ret, v_end, v_sta, delay1, delay2);
			//return -1;
		}
	}
	pVir_addr = (unsigned int*)dvr_remap_uncached_memory(addr, size, __builtin_return_address(0));
	ret = drvif_Set_DM_HDR_3dLUT_24x24x24_16_By_DMA(enable, pArray, addr, pVir_addr);
	dvr_unmap_memory((void *)pVir_addr, size);

	return ret;

}
#endif // CONFIG_HW_SUPPORT_HDR_3DLUT_24

char fwif_color_DM_3D_17x17x17_LUT_Set_By_DMA(unsigned char enable, unsigned short *pArray)
{
#ifndef BUILD_QUICK_SHOW

	char ret;
	unsigned int addr = 0, size;
	unsigned int *pVir_addr = 0;
	unsigned short v_sta, v_len, line_cnt, v_end;
	unsigned short delay1, delay2;
	delay1 = 30;

	addr = get_query_VIP_DMAto3DTABLE_start_address(DMAto3DTABLE_HDR_3D_LUT);
	addr = drvif_memory_get_data_align(addr, (1 << 12));
	size = drvif_memory_get_data_align(VIP_DMAto3DTable_HDR_3D_LUT_SIZE, (1 << 12));
	if (addr == 0) {
		rtd_pr_vpq_emerg("fwif_color_DM_3D_17x17x17_LUT_Set_By_DMA, addr = NULL\n");
		return -1;
	}

	ret = drvif_Get_VO_Active_Info(&v_sta, &v_end, &line_cnt);
	if (ret < 0) {
		rtd_pr_vpq_emerg("fwif_color_DM_3D_17x17x17_LUT_Set_By_DMA is not ready, ret = %d\n", ret);
		//return -1;
	} else {
		/* wait for active area to write DMA*/
		v_len = ((v_end - v_sta)<0)?(0):((v_end - v_sta));
		delay2 = delay1 + (v_len >> 1);
		ret = drvif_Wait_VO_start(delay1, v_len, 0x032500);
		if (ret < 0) {
			rtd_pr_vpq_emerg("fwif_color_DM_3D_17x17x17_LUT_Set_By_DMA fail to wait for active, ret = %d, v_end=%d, v_sta=%d, delay1=%d, delay2=%d,\n",
				ret, v_end, v_sta, delay1, delay2);
			//return -1;
		}
	}
	pVir_addr = (unsigned int*)dvr_remap_uncached_memory(addr, size, __builtin_return_address(0));
	ret = drvif_Set_DM_HDR_3dLUT_17x17x17_16_By_DMA(enable, pArray, addr, pVir_addr);
	dvr_unmap_memory((void *)pVir_addr, size);

	return ret;
#else
	return 0;

#endif
}

void fwif_color_Set_system_info_structure_table(unsigned long arg)
{
	printk("fwif_color_SetDCC_system_info_structure_table\n");
	system_info_structure_table = ((_system_setting_info*)arg);
}

void fwif_color_Set_RPC_system_info_structure_table(unsigned long arg)
{
	printk("fwif_color_Set_RPC_system_info_structure_table\n");
	RPC_system_info_structure_table = ((_RPC_system_setting_info*)arg);
}

void fwif_color_Share_Memory_Access_VIP_TABLE_Struct(unsigned long arg)
{
	printk("fwif_color_Share_Memory_Access_VIP_TABLE_Struct\n");
	g_Share_Memory_VIP_TABLE_Struct_isr= ((SLR_VIP_TABLE *)arg);

	#ifdef VCPU_INTERRUPT
	ROSPM_AddDevice(&vip_device);
	#endif
}

void fwif_color_Share_Memory_Access_VIP_TABLE_CRC_Struct(unsigned long arg)
{
	vip_table_crc_isr = ((VIP_table_crc_value *)arg);
}


unsigned char fwif_color_get_force_run_i3ddma_enable(unsigned char display)
{
#if 1//def CONFIG_FORCE_RUN_I3DDMA
	return get_force_i3ddma_enable(display);
#else
	return 0;
#endif
}

void fwif_color_safe_od_enable(unsigned char bEnable)
{
#ifndef BUILD_QUICK_SHOW

	extern unsigned char bODInited;

	if (!bODInited)
		return;
	pr_notice("fwif_color_safe_od_enable(%d)\n", bEnable);
	drvif_color_safe_od_enable(bEnable, bODPreEnable);
#endif
}

void fwif_color_gamma_encode_transition_remap(unsigned short *InOut_R, unsigned short *InOut_G, unsigned short *InOut_B)
{
#ifdef CONFIG_RTK_GAMUT_SUPPORT
	return;
#else
	extern unsigned short g_GammaRemap[4097];
	extern unsigned short Power1div22Gamma[1025];
	int i, j, hdiff, qdiff;

	if (Scaler_Get_CinemaMode_PQ() == 0)
		return;

	if (NULL == InOut_R || NULL == InOut_G || NULL == InOut_B)
		return;

	// R
	for (i = 0, j = 0; i <= 4096; i+=4, j++) {
		g_GammaRemap[i] = InOut_R[j];
	}

	for (i = 0; i < 4096; i+=4) {
		hdiff = (g_GammaRemap[i+4]-g_GammaRemap[i])>>1;
		qdiff = hdiff>>1;
		g_GammaRemap[i+1] = g_GammaRemap[i]+qdiff;
		g_GammaRemap[i+2] = g_GammaRemap[i]+hdiff;
		g_GammaRemap[i+3] = g_GammaRemap[i+2]+qdiff;
	}

	for (i = 0; i < 1025; i++) {
		InOut_R[i] = g_GammaRemap[Power1div22Gamma[i]];
	}

	// G
	for (i = 0, j = 0; i <= 4096; i+=4, j++) {
		g_GammaRemap[i] = InOut_G[j];
	}

	for (i = 0; i < 4096; i+=4) {
		hdiff = (g_GammaRemap[i+4]-g_GammaRemap[i])>>1;
		qdiff = hdiff>>1;
		g_GammaRemap[i+1] = g_GammaRemap[i]+qdiff;
		g_GammaRemap[i+2] = g_GammaRemap[i]+hdiff;
		g_GammaRemap[i+3] = g_GammaRemap[i+2]+qdiff;
	}

	for (i = 0; i < 1025; i++) {
		InOut_G[i] = g_GammaRemap[Power1div22Gamma[i]];
	}

	// B
	for (i = 0, j = 0; i <= 4096; i+=4, j++) {
		g_GammaRemap[i] = InOut_B[j];
	}

	for (i = 0; i < 4096; i+=4) {
		hdiff = (g_GammaRemap[i+4]-g_GammaRemap[i])>>1;
		qdiff = hdiff>>1;
		g_GammaRemap[i+1] = g_GammaRemap[i]+qdiff;
		g_GammaRemap[i+2] = g_GammaRemap[i]+hdiff;
		g_GammaRemap[i+3] = g_GammaRemap[i+2]+qdiff;
	}

	for (i = 0; i < 1025; i++) {
		InOut_B[i] = g_GammaRemap[Power1div22Gamma[i]];
	}
#endif
}

void fwif_color_set_gamma_from_MagicGamma_MiddleWare(void)
{
#ifndef BUILD_QUICK_SHOW

	extern VIP_MAGIC_GAMMA_Curve_Driver_Data g_MagicGammaDriverDataSave;
#ifdef CONFIG_RTK_GAMUT_SUPPORT
	extern unsigned short Power1div22Gamma[1025];
#endif
	bool bNoUseRPC = false;
	unsigned char display = 0;
	unsigned char src_idx = 0;
	Scaler_Get_Display_info(&display,&src_idx);

	down(&Gamma_Semaphore);

	//0. gamma curve reg control
	//fwif_color_gamma_control_front(display);

	//1. choose encode gamma data
	fwif_color_get_gamma_default(g_GammaMode, &tGAMMA_temp_R, &tGAMMA_temp_G, &tGAMMA_temp_B);

	//2. decode gamma data to gamma curve
	fwif_color_gamma_decode(GOut_R, GOut_G, GOut_B, tGAMMA_temp_R, tGAMMA_temp_G, tGAMMA_temp_B);

	//3. calc gamma curve multiply from vpq ext HAL setting
	fwif_color_Cal_gammaCurve_from_MagicGamma_MiddleWare(&g_MagicGammaDriverDataSave);

	fwif_color_gamma_encode_transition_remap(GOut_R, GOut_G, GOut_B);

	// following is same as fwif_set_gamma
	//4. gamma curve data protect and debug
	fwif_color_gamma_curve_data_protect(GOut_R, GOut_G, GOut_B);		//DO NOT mark this, must check

	//5. encode gamma cuve to gamma data
#ifdef ENABLE_xvYcc
	fwif_color_gamma_remmping_for_xvYcc(src_idx);
#endif
#ifdef CONFIG_RTK_GAMUT_SUPPORT
	fwif_color_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B, Power1div22Gamma, Power1div22Gamma, Power1div22Gamma);
#else
	fwif_color_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B, GOut_R, GOut_G, GOut_B);
#endif
	//fwif_color_encode_gamma_debug(2); //debug

/*
	//6. write gamma table
	fwif_color_set_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B);

	//7. gamma curve reg control
	fwif_color_gamma_control_back(display, 1);
*/

#ifdef CONFIG_RTK_GAMUT_SUPPORT
	if (display != SLR_MAIN_DISPLAY ||
		!drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 1, 1) ||
		!fwif_color_set_ddomainISR_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B))
			bNoUseRPC = true;
#else
	if (display != SLR_MAIN_DISPLAY ||
		!drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, Scaler_Get_CinemaMode_PQ(), 1) ||
		!fwif_color_set_ddomainISR_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B))
			bNoUseRPC = true;
#endif

	if (bNoUseRPC) {
		#ifdef CONFIG_RTK_GAMUT_SUPPORT
		drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 1, 0);
		drvif_color_inv_gamma_enable(SLR_SUB_DISPLAY, 1, 0);
		#else
		drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, Scaler_Get_CinemaMode_PQ(), 0);
		drvif_color_inv_gamma_enable(SLR_SUB_DISPLAY, Scaler_Get_CinemaMode_PQ(), 0);
		#endif

		fwif_color_gamma_control_front(display);
		fwif_color_set_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B);
		fwif_color_gamma_control_back(display, 1);
	}

	up(&Gamma_Semaphore);
#endif	
}

char fwif_color_Cal_gammaCurve_from_MagicGamma_MiddleWare(VIP_MAGIC_GAMMA_Curve_Driver_Data *pData)
{
	unsigned short i, j, shift_bit, step;
	unsigned short MagicGammaR_tmp[MAGIC_GAMMA_POINT_NUM],	MagicGammaG_tmp[MAGIC_GAMMA_POINT_NUM],  MagicGammaB_tmp[MAGIC_GAMMA_POINT_NUM];
	unsigned short spec_index[MAGIC_GAMMA_POINT_NUM]={0, 107, 207, 311, 411, 515, 619, 719, 823, 923, 1023};
	unsigned short r0,r1, g0,g1, b0,b1, index0, index1, tmp, idx = 0;
	unsigned int ratioR = 0, ratioG = 0, ratioB = 0;
	unsigned char ratio_shiftBit =10;
	int hdiff, qdiff;
	extern unsigned short g_GammaRemap[4097];

	// error check;
	if(pData == NULL)
	{
		rtd_pr_vpq_err("%s pData = NULL\n", __FUNCTION__);
		return -1;
	}

	shift_bit = pData->shift_bit;

	//change gamma power by pic mode
	for(i=0;i<MAGIC_GAMMA_POINT_NUM;i++)
	{
		MagicGammaR_tmp[i] = pData->GammaR[i];
		MagicGammaG_tmp[i] = pData->GammaG[i];
		MagicGammaB_tmp[i] = pData->GammaB[i];

		MagicGammaR_tmp[i] = (MagicGammaR_tmp[i] * pData->Gamma_Coef[i])>>shift_bit;
		MagicGammaG_tmp[i] = (MagicGammaG_tmp[i] * pData->Gamma_Coef[i])>>shift_bit;
		MagicGammaB_tmp[i] = (MagicGammaB_tmp[i] * pData->Gamma_Coef[i])>>shift_bit;
	}

	//data check;
	for(i=0;i<MAGIC_GAMMA_POINT_NUM-1;i++)
	{
		if(spec_index[i]>=spec_index[i+1])
		{
			rtd_pr_vpq_err("spec_index Error, return\n");
			return -1;

		}
		if(MagicGammaR_tmp[i]>=MagicGammaR_tmp[i+1])
		{
			rtd_pr_vpq_err("MagicGammaR_tmp Error, return\n");
			return -1;

		}
		if(MagicGammaG_tmp[i]>=MagicGammaG_tmp[i+1])
		{
			rtd_pr_vpq_err("MagicGammaR_tmp Error, return\n");
			return -1;

		}
		if(MagicGammaB_tmp[i]>=MagicGammaB_tmp[i+1])
		{
			rtd_pr_vpq_err("MagicGammaR_tmp Error, return\n");
			return -1;

		}

	}
	if((spec_index[0]!=0) || (spec_index[MAGIC_GAMMA_POINT_NUM-1]!=(Bin_Num_Gamma-1)))
	{
		rtd_pr_vpq_err("(spec_index[0]!=0) || (spec_index[MAGIC_GAMMA_POINT_NUM]!=(Bin_Num_Gamma-1)), return\n");
		return -1;

	}
	if((MagicGammaR_tmp[0]!=0) || (MagicGammaG_tmp[0]!=0) || (MagicGammaB_tmp[0]!=0))
	{
		rtd_pr_vpq_err("(MagicGammaR_tmp[0]!=0) || (MagicGammaG_tmp[0]!=0) || (MagicGammaB_tmp[0]!=0), return\n");
		return -1;

	}

	// Red
	for (i = 0, j = 0; i < 4096; i+=4, j++) {
		g_GammaRemap[i] = GOut_R[j];
	}
	g_GammaRemap[4096] = GOut_R[1023];

	for (i = 0; i < 4096; i+=4) {
		hdiff = (g_GammaRemap[i+4]-g_GammaRemap[i])>>1;
		qdiff = hdiff>>1;
		g_GammaRemap[i+1] = g_GammaRemap[i]+qdiff;
		g_GammaRemap[i+2] = g_GammaRemap[i]+hdiff;
		g_GammaRemap[i+3] = g_GammaRemap[i+2]+qdiff;
	}

	step = 0; index0 = 0; index1 = 0;
	r0 = 0; r1 = 0;
	for(i = 0; i < Bin_Num_Gamma; i++)
	{
		if(i == spec_index[step])
		{
			GOut_R[i] = g_GammaRemap[MagicGammaR_tmp[step]];
			if((step+1)<MAGIC_GAMMA_POINT_NUM)
			{
				index0 = spec_index[step];
				r0 = MagicGammaR_tmp[step];
				index1 = spec_index[step+1];
				r1 = MagicGammaR_tmp[step+1];
				tmp = index1 - index0;
				ratioR = ((r1 - r0)<<ratio_shiftBit)/tmp;
				idx = 1;
				step++;
			}
		}
		else if (step > 0)
		{
			GOut_R[i] = MagicGammaR_tmp[step-1] + ((idx * ratioR)>>ratio_shiftBit);
			if (GOut_R[i] > 4096)
				GOut_R[i] = 4096;
			GOut_R[i] = g_GammaRemap[GOut_R[i]];
			idx++;
		}
	}

	// Green
	for (i = 0, j = 0; i < 4096; i+=4, j++) {
		g_GammaRemap[i] = GOut_G[j];;
	}
	g_GammaRemap[4096] = GOut_G[1023];

	for (i = 0; i < 4096; i+=4) {
		hdiff = (g_GammaRemap[i+4]-g_GammaRemap[i])>>1;
		qdiff = hdiff>>1;
		g_GammaRemap[i+1] = g_GammaRemap[i]+qdiff;
		g_GammaRemap[i+2] = g_GammaRemap[i]+hdiff;
		g_GammaRemap[i+3] = g_GammaRemap[i+2]+qdiff;
	}

	step = 0; index0 = 0; index1 = 0;
	g0 = 0; g1 = 0;
	for(i = 0; i < Bin_Num_Gamma; i++)
	{
		if(i == spec_index[step])
		{
			GOut_G[i] = g_GammaRemap[MagicGammaG_tmp[step]];
			if((step+1)<MAGIC_GAMMA_POINT_NUM)
			{
				index0 = spec_index[step];
				g0 = MagicGammaG_tmp[step];
				index1 = spec_index[step+1];
				g1 = MagicGammaG_tmp[step+1];
				tmp = index1 - index0;
				ratioG = ((g1 - g0)<<ratio_shiftBit)/tmp;

				idx = 1;
				step++;
			}
		}
		else if (step > 0)
		{
			GOut_G[i] = MagicGammaG_tmp[step-1] + ((idx * ratioG)>>ratio_shiftBit);
			if (GOut_G[i] > 4096)
				GOut_G[i] = 4096;
			GOut_G[i] = g_GammaRemap[GOut_G[i]];
			idx++;
		}
	}

	// Blue
	for (i = 0, j = 0; i < 4096; i+=4, j++) {
		g_GammaRemap[i] = GOut_B[j];;
	}
	g_GammaRemap[4096] = GOut_B[1023];

	for (i = 0; i < 4096; i+=4) {
		hdiff = (g_GammaRemap[i+4]-g_GammaRemap[i])>>1;
		qdiff = hdiff>>1;
		g_GammaRemap[i+1] = g_GammaRemap[i]+qdiff;
		g_GammaRemap[i+2] = g_GammaRemap[i]+hdiff;
		g_GammaRemap[i+3] = g_GammaRemap[i+2]+qdiff;
	}

	step = 0; index0 = 0; index1 = 0;
	b0 = 0; b1 = 0;
	for(i = 0; i < Bin_Num_Gamma; i++)
	{
		if(i == spec_index[step])
		{
			GOut_B[i] = g_GammaRemap[MagicGammaB_tmp[step]];
			if((step+1)<MAGIC_GAMMA_POINT_NUM)
			{
				index0 = spec_index[step];
				b0 = MagicGammaB_tmp[step];
				index1 = spec_index[step+1];
				b1 = MagicGammaB_tmp[step+1];
				tmp = index1 - index0;
				ratioB = ((b1 - b0)<<ratio_shiftBit)/tmp;

				idx = 1;
				step++;
			}
		}
		else if (step > 0)
		{
			GOut_B[i] = MagicGammaB_tmp[step-1] + ((idx * ratioB)>>ratio_shiftBit);
			if (GOut_B[i] > 4096)
				GOut_B[i] = 4096;
			GOut_B[i] = g_GammaRemap[GOut_B[i]];
			idx++;
		}
	}


	//data protect
	GOut_R[0]=0;
	GOut_G[0]=0;
	GOut_B[0]=0;
	GOut_R[Bin_Num_Gamma] = GOut_R[Bin_Num_Gamma - 1];
	GOut_G[Bin_Num_Gamma] = GOut_G[Bin_Num_Gamma - 1];
	GOut_B[Bin_Num_Gamma] = GOut_B[Bin_Num_Gamma - 1];

#if 0
	printf("GOut_R=\n");
	for(i = 0; i <= Bin_Num_Gamma; i++)
		printf("%d\n",GOut_R[i]);
	printf("\n");

	printf("MagicGammaR_tmp=\n");
	for(i = 0; i < MAGIC_GAMMA_POINT_NUM; i++)
		printf("%d\n",MagicGammaR_tmp[i]);
	printf("\n");
#endif
	return 0;
}

void fwif_color_set_rtk_colortemp_output_gamma(unsigned char enable, unsigned short Red_Con, unsigned short Green_Con, unsigned short Blue_Con,
	short Red_Bri, short Green_Bri, short Blue_Bri, unsigned char waitSync)
{
	static short OutGamma[3][257];
	extern unsigned int out_GAMMA_R[128];
	extern unsigned int out_GAMMA_G[128];
	extern unsigned int out_GAMMA_B[128];
	int i, idx;

	down(&Gamma_Semaphore);
	for (i = 0; i < 257; i++) {
		idx = i << 2;
		OutGamma[0][i] = ((GOut_R[idx]*(int)Red_Con)>>11)+Red_Bri;
		OutGamma[1][i] = ((GOut_G[idx]*(int)Green_Con)>>11)+Green_Bri;
		OutGamma[2][i] = ((GOut_B[idx]*(int)Blue_Con)>>11)+Blue_Bri;

		OutGamma[0][i] = OutGamma[0][i] > 4095 ? 4095 : (OutGamma[0][i] < 0 ? 0 : OutGamma[0][i]);
		OutGamma[1][i] = OutGamma[1][i] > 4095 ? 4095 : (OutGamma[1][i] < 0 ? 0 : OutGamma[1][i]);
		OutGamma[2][i] = OutGamma[2][i] > 4095 ? 4095 : (OutGamma[2][i] < 0 ? 0 : OutGamma[2][i]);
	}
	fwif_color_out_gamma_control_front();
	fwif_color_out_gamma_encode(out_GAMMA_R, out_GAMMA_G, out_GAMMA_B, OutGamma[0], OutGamma[1], OutGamma[2]);
	if (waitSync)
		fwif_color_WaitFor_SYNC_START_UZUDTG();
	drvif_color_Out_Gamma_Curve_Write(out_GAMMA_R, GAMMA_CHANNEL_R);	/*R*/
	drvif_color_Out_Gamma_Curve_Write(out_GAMMA_G, GAMMA_CHANNEL_G);	/*G*/
	drvif_color_Out_Gamma_Curve_Write(out_GAMMA_B, GAMMA_CHANNEL_B);	/*B*/
	fwif_color_out_gamma_control_back();
	fwif_color_out_gamma_control_enable(enable);
	up(&Gamma_Semaphore);
}

void fwif_color_set_OutputGamma_System(unsigned int gamma_curve_index)
{
	unsigned char display;
	unsigned char src_idx;
	unsigned short i=0;
	unsigned char output_Gamma_En;
	unsigned char output_Location;
	unsigned char fmt;

	Scaler_Get_Display_info(&display, &src_idx);
	
	down(&Gamma_Semaphore);
	{
	/*0. gamma curve reg control*/
	/*fwif_color_out_gamma_control_front();*/

	/*1. choose encode gamma data*/
	fwif_color_set_gamma_curve_index(gamma_curve_index, &tGAMMA_temp_R, &tGAMMA_temp_G, &tGAMMA_temp_B);

	/*2. decode gamma data to gamma curve*/
	fwif_color_gamma_decode(GOut_R, GOut_G, GOut_B, tGAMMA_temp_R, tGAMMA_temp_G, tGAMMA_temp_B);
	//fwif_color_decode_gamma_debug(1); /*debug*/

	for(i=1020;i<1025;i++) printk("GOut_R[%d]=%d,%d,%d\n",i,GOut_R[i], GOut_G[i], GOut_B[i]);
	//fwif_color_set_GainOffsetGamma(GOut_R, GOut_G, GOut_B);
	
	/*5. gamma curve data protect and debug*/
	fwif_color_gamma_curve_data_protect(GOut_R, GOut_G, GOut_B);		/*DO NOT mark this, must check*/

	for(i=1020;i<1025;i++) printk("aGOut_R[%d]=%d,%d,%d\n",i,GOut_R[i], GOut_G[i], GOut_B[i]);

	fwif_color_gamma_data_reduce(GOut_R, GOut_G, GOut_B);

	/*6. encode gamma cuve to gamma data*/
	fwif_color_out_gamma_encode(final_OutGAMMA_R, final_OutGAMMA_G, final_OutGAMMA_B, GOut_R, GOut_G, GOut_B);
	//fwif_color_encode_gamma_debug(2);  /*debug*/

	/*7. write gamma table*/
	fwif_color_out_gamma_control_front();

	fwif_color_Out_Gamma_Curve_Write(final_OutGAMMA_R, GAMMA_CHANNEL_R);
	fwif_color_Out_Gamma_Curve_Write(final_OutGAMMA_G, GAMMA_CHANNEL_G);
	fwif_color_Out_Gamma_Curve_Write(final_OutGAMMA_B, GAMMA_CHANNEL_B);
	//fwif_color_Out_Gamma_Curve_Write(LinearOutputGamma, GAMMA_CHANNEL_W);

	fwif_color_out_gamma_control_back();

	output_Gamma_En = 1;
	fmt = 0;
	output_Location = 1;/*After RGBW*/
	drvif_color_set_output_gamma_format(output_Gamma_En, fmt, output_Location, 0);

	}

	up(&Gamma_Semaphore);
}

void fwif_color_set_clarity_resolution(void)
{
#ifndef BUILD_QUICK_SHOW

	unsigned short value = Scaler_GetSharpnessTable();
	unsigned char gain = 0;

	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	switch(Scaler_get_Clarity_RealityCreation()) {
	case 0: //off
		gain = 0;
		break;
	case 1: //auto
	default:
		if (value < Sharp_table_num)
			gain = gVip_Table->Ddomain_SHPTable[value].SR_Control.Ring_Gen_Gain;
		break;
	case 2: //manual
		gain = Scaler_get_Clarity_Resolution()<<1;
		break;
	}
	drvif_color_setSR_Ring_Gen_Gain(gain);
#endif
}

#if 1	// reduce log times version
#define DRTM_Log_Show_Num 300
#define DRTM_ABS(x) ((x >= 0)?(x):((x)*(-1)))
#define VIP_DRTM_Monitor_items 6
#define VIP_DRTM_buf_num 2
unsigned int VIP_RTNR_measured[VIP_DRTM_buf_num][VIP_DRTM_Monitor_items];
unsigned int VIP_DI_measured[VIP_DRTM_buf_num][VIP_DRTM_Monitor_items];
extern unsigned int set_di_sta_90k;

unsigned int VIP_RTNR_measured_print[VIP_DRTM_buf_num][VIP_DRTM_Monitor_items];
unsigned int VIP_RTNR_measured_times_print[VIP_DRTM_Monitor_items]={0};
unsigned int VIP_RTNR_measured_t_record[VIP_DRTM_Monitor_items]={0};

unsigned int VIP_DI_measured_print[VIP_DRTM_buf_num][VIP_DRTM_Monitor_items];
unsigned int VIP_DI_measured_times_print[VIP_DRTM_Monitor_items]={0};
unsigned int VIP_DI_measured_t_record[VIP_DRTM_Monitor_items]={0};

void fwif_color_DI_RTNR_Timing_Measure_Check_ISR(void)
{
	unsigned char i, j;
	int diff_rtnr[VIP_DRTM_Monitor_items], diff_di[VIP_DRTM_Monitor_items];
	int tmp;
	static unsigned short cnt = 0;
	unsigned int t_now  = 0;
	unsigned int t_diff = 0;
	unsigned int t_setDI = 0, t_swDetect = 0, t_SMT = 0, t_DMA_check = 0;
	unsigned int t_diff_min = 0xffffffff;
	unsigned int t_th = 3 * 42 * 90; // 3 frames
	_RPC_system_setting_info *RPC_sysInfo = NULL;
	unsigned char final_print_update = 0;
	static unsigned char need_to_print_flag = 0;
	static unsigned int t_print_pre = 0;
	
	RPC_sysInfo = (_RPC_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);	
	if (RPC_sysInfo == NULL) {
		rtd_printk(KERN_INFO,"DRTM", "rpc sys info = NULL,,fwif_color_DI_RTNR_Timing_Measure_Check_ISR\n");
		return;
	}

	// newest index is  0
	drvif_color_Get_DI_RTNR_Timing_Measurement(&VIP_RTNR_measured[0][0], &VIP_DI_measured[0][0]);
	t_now = drvif_Get_90k_Lo_clk();

	// set di 90k
	t_setDI = set_di_sta_90k;
	t_diff = (t_now < t_setDI)?(t_now):(t_now - t_setDI);
	if (t_diff < t_diff_min)
		t_diff_min = t_diff;

	// sw reset 90k
	t_swDetect = Scaler_ChangeUINT32Endian(RPC_sysInfo->PQ_IP_Enable_Monitor_2.DI_RTNR_SW_Detection_90k);
	t_diff = (t_now < t_swDetect)?(t_now):(t_now - t_swDetect);
	if (t_diff < t_diff_min)
		t_diff_min = t_diff;

	// smt 90k
	t_SMT = Scaler_ChangeUINT32Endian(RPC_sysInfo->PQ_IP_Enable_Monitor_2.DI_RTNR_SMT_90k);
	t_diff = (t_now < t_SMT)?(t_now):(t_now - t_SMT);
	if (t_diff < t_diff_min)
		t_diff_min = t_diff;

	// DMA check 90k
	t_DMA_check = Scaler_ChangeUINT32Endian(RPC_sysInfo->PQ_IP_Enable_Monitor_2.DI_RTNR_DMA_Check_90k);
	t_diff = (t_now < t_DMA_check)?(t_now):(t_now - t_DMA_check);
	if (t_diff < t_diff_min)
		t_diff_min = t_diff;	
	
	// 0. vact rinsing->vact falling, 1. vact falling -> vs rising, 2. vs rising -> vact rinsing
	// 3. hact rising->hact falling, 4. hact falling->hact rising, 5. den rising->den rising
	for (i=0;i<VIP_DRTM_Monitor_items;i++) {
		tmp = (int)VIP_RTNR_measured[0][i] - (int)VIP_RTNR_measured[1][i];
		diff_rtnr[i] = DRTM_ABS(tmp);
		tmp = (int)VIP_DI_measured[0][i] - (int)VIP_DI_measured[1][i];
		diff_di[i] = DRTM_ABS(tmp);
	}

	for (i=0;i<VIP_DRTM_Monitor_items;i++) {
		if (diff_rtnr[i] != 0) {
			VIP_RTNR_measured_times_print[i]++;
			VIP_RTNR_measured_t_record[i] = t_now;
			final_print_update = 1;
		}
		if (diff_di[i] != 0) {
			VIP_DI_measured_times_print[i]++;
			VIP_DI_measured_t_record[i] = t_now;
			final_print_update = 1;
		}
	}

	if (final_print_update == 1) {
		memcpy(&VIP_RTNR_measured_print[0][0], &VIP_RTNR_measured[0][0], sizeof(int)*VIP_DRTM_buf_num*VIP_DRTM_Monitor_items);
		memcpy(&VIP_DI_measured_print[0][0], &VIP_DI_measured[0][0], sizeof(int)*VIP_DRTM_buf_num*VIP_DRTM_Monitor_items);
		need_to_print_flag = 1;
	} 

	if (t_diff_min < t_th)
		cnt = DRTM_Log_Show_Num;
	else
		cnt++;

	if (cnt >= DRTM_Log_Show_Num) {
		if (need_to_print_flag == 1) {
			rtd_printk(KERN_INFO,"DRTM","NR,vact:%d-%d@%d[%d],1:%d-%d@%d[%d],2:%d-%d@%d[%d],hact:%d-%d@%d[%d],4:%d-%d@%d[%d],5:%d-%d@%d[%d]\n",
				VIP_RTNR_measured_print[1][0], VIP_RTNR_measured_print[0][0], VIP_RTNR_measured_t_record[0], VIP_RTNR_measured_times_print[0],
				VIP_RTNR_measured_print[1][1], VIP_RTNR_measured_print[0][1], VIP_RTNR_measured_t_record[1], VIP_RTNR_measured_times_print[1], 
				VIP_RTNR_measured_print[1][2], VIP_RTNR_measured_print[0][2], VIP_RTNR_measured_t_record[2], VIP_RTNR_measured_times_print[2], 
				VIP_RTNR_measured_print[1][3], VIP_RTNR_measured_print[0][3], VIP_RTNR_measured_t_record[3], VIP_RTNR_measured_times_print[3], 
				VIP_RTNR_measured_print[1][4], VIP_RTNR_measured_print[0][4], VIP_RTNR_measured_t_record[4], VIP_RTNR_measured_times_print[4], 
				VIP_RTNR_measured_print[1][5], VIP_RTNR_measured_print[0][5], VIP_RTNR_measured_t_record[5], VIP_RTNR_measured_times_print[5]);
			rtd_printk(KERN_INFO,"DRTM","DI,vact:%d-%d@%d[%d],1:%d-%d@%d[%d],2:%d-%d@%d[%d],hact:%d-%d@%d[%d],4:%d-%d@%d[%d],5:%d-%d@%d[%d]\n",
				VIP_DI_measured_print[1][0], VIP_DI_measured_print[0][0], VIP_DI_measured_t_record[0], VIP_DI_measured_times_print[0], 
				VIP_DI_measured_print[1][1], VIP_DI_measured_print[0][1], VIP_DI_measured_t_record[1], VIP_DI_measured_times_print[1], 
				VIP_DI_measured_print[1][2], VIP_DI_measured_print[0][2], VIP_DI_measured_t_record[2], VIP_DI_measured_times_print[2], 
				VIP_DI_measured_print[1][3], VIP_DI_measured_print[0][3], VIP_DI_measured_t_record[3], VIP_DI_measured_times_print[3], 
				VIP_DI_measured_print[1][4], VIP_DI_measured_print[0][4], VIP_DI_measured_t_record[4], VIP_DI_measured_times_print[4], 
				VIP_DI_measured_print[1][5], VIP_DI_measured_print[0][5], VIP_DI_measured_t_record[5], VIP_DI_measured_times_print[5]);
			rtd_printk(KERN_INFO,"DRTM","TR,t:%d-%d,t_setDI=%d,t_swDetect=%d,t_SMT=%d,t_DMA_check=%d\n",
				t_print_pre, t_now, t_setDI, t_swDetect, t_SMT, t_DMA_check);			
			for (i=0;i<VIP_DRTM_Monitor_items;i++) {
				VIP_RTNR_measured_times_print[i] = 0;
				VIP_DI_measured_times_print[i] = 0;
				VIP_RTNR_measured_t_record[i] = 0;
				VIP_DI_measured_t_record[i] = 0;
			}

			need_to_print_flag = 0;
			
		}
		t_print_pre = t_now;
		cnt = 0;
	}


	
	for (j=0;j<(VIP_DRTM_buf_num-1);j++) {
		memcpy(&VIP_RTNR_measured[VIP_DRTM_buf_num-1-j][0], &VIP_RTNR_measured[VIP_DRTM_buf_num-2-j][0], sizeof(int)*VIP_DRTM_Monitor_items);
		memcpy(&VIP_DI_measured[VIP_DRTM_buf_num-1-j][0], &VIP_DI_measured[VIP_DRTM_buf_num-2-j][0], sizeof(int)*VIP_DRTM_Monitor_items);
		//VIP_DI_RTNR_measured_90k[VIP_DRTM_buf_num-1-j] = VIP_DI_RTNR_measured_90k[VIP_DRTM_buf_num-2-j];
	}

}

#else
#define DRTM_ABS(x) ((x >= 0)?(x):((x)*(-1)))
#define VIP_DRTM_Monitor_items 6
#define VIP_DRTM_buf_num 2
#define VIP_DRTM_target_num 3
//#define VIP_DI_RTNR_measured_log_reduce_max 6
/*#define VIP_DI_RTNR_measured_log_reduce_th 300
typedef enum _VIP_DRTM_SP_ITEMs {
	DRTM_SP_H_Total = 0,	// rtnr h total is small, di h total is not stable.
	DRTM_SP_V_Total,

	DRTM_SP_MAX,
} VIP_DRTM_SP_ITEMs;*/

unsigned int VIP_RTNR_measured[VIP_DRTM_buf_num][VIP_DRTM_Monitor_items];
unsigned int VIP_DI_measured[VIP_DRTM_buf_num][VIP_DRTM_Monitor_items];
unsigned int VIP_DI_RTNR_measured_90k[VIP_DRTM_buf_num];
unsigned int VIP_RTNR_measured_target[VIP_DRTM_target_num][VIP_DRTM_Monitor_items];
unsigned int VIP_DI_measured_target[VIP_DRTM_target_num][VIP_DRTM_Monitor_items];
unsigned char VIP_RTNR_target_idx[VIP_DRTM_Monitor_items] = {0};
unsigned char VIP_DI_target_idx[VIP_DRTM_Monitor_items] = {0};
//unsigned int VIP_RTNR_measured_pre[VIP_DI_RTNR_measured_items];
//unsigned int VIP_DI_measured_pre[VIP_DI_RTNR_measured_items];
//unsigned int VIP_DI_RTNR_measured_90k_pre;
//unsigned int VIP_DI_RTNR_measured_CNT[DRTM_SP_MAX];
//unsigned int VIP_DI_RTNR_measured_CNT_pre[DRTM_SP_MAX];
void fwif_color_DI_RTNR_Timing_Measure_Check_ISR(void)
{
	unsigned char i, j, target_idx_tmp;
	int diff_rtnr[VIP_DRTM_Monitor_items], diff_di[VIP_DRTM_Monitor_items];
	int diff_target_rtnr, diff_target_di, tmp;
	//int taget_diff[VIP_DRTM_target_num];
	unsigned char porch_print_flag = 0;
	unsigned char target_diff_cnt;
	// newest index is  0
	drvif_color_Get_DI_RTNR_Timing_Measurement(&VIP_RTNR_measured[0][0], &VIP_DI_measured[0][0]);
	VIP_DI_RTNR_measured_90k[0] = drvif_Get_90k_Lo_clk();

	// 0. vact rinsing->vact falling, 1. vact falling -> vs rising, 2. vs rising -> vact rinsing
	// 3. hact rising->hact falling, 4. hact falling->hact rising, 5. den rising->den rising
	for (i=0;i<VIP_DRTM_Monitor_items;i++) {
		tmp = (int)VIP_RTNR_measured[0][i] - (int)VIP_RTNR_measured[1][i];
		diff_rtnr[i] = DRTM_ABS(tmp);
		tmp = (int)VIP_DI_measured[0][i] - (int)VIP_DI_measured[1][i];
		diff_di[i] = DRTM_ABS(tmp);
	}

	//diff_target_rtnr = (VIP_RTNR_measured[0][0] * 10)>>10; // 1%
	//diff_target_di = (VIP_DI_measured[0][0] * 10)>>10; // 1%

	// RTNR
	for (i=0;i<VIP_DRTM_Monitor_items;i++) {
		/*tmp = DRTM_ABS(((int)VIP_RTNR_measured[0][i]) - ((int)VIP_RTNR_measured_target[0][i]));
		tmp1 = DRTM_ABS(((int)VIP_RTNR_measured[0][i]) - ((int)VIP_RTNR_measured_target[1][i]));
		tmp2 = DRTM_ABS(((int)VIP_RTNR_measured[0][i]) - ((int)VIP_RTNR_measured_target[2][i]));*/
		target_diff_cnt = 0;
		diff_target_rtnr = (VIP_RTNR_measured[0][i] * 100)>>10; // 10%
		for (j=0;j<VIP_DRTM_target_num;j++) {
			tmp = DRTM_ABS(((int)VIP_RTNR_measured[0][i]) - ((int)VIP_RTNR_measured_target[j][i]));
			if (tmp>diff_target_rtnr)
				target_diff_cnt++;
		}
		if (target_diff_cnt >= VIP_DRTM_target_num) {
			target_idx_tmp = VIP_RTNR_target_idx[i];
			VIP_RTNR_measured_target[target_idx_tmp][i] = VIP_RTNR_measured[0][i];
			porch_print_flag = 1;
			VIP_RTNR_target_idx[i]++;
			if (VIP_RTNR_target_idx[i] >= VIP_DRTM_target_num)
				VIP_RTNR_target_idx[i] = 0;
		}
	}

	// DI
	for (i=0;i<VIP_DRTM_Monitor_items;i++) {
		/*tmp = DRTM_ABS(((int)VIP_DI_measured[0][i]) - ((int)VIP_DI_measured_target[0][i]));
		tmp1 = DRTM_ABS(((int)VIP_DI_measured[0][i]) - ((int)VIP_DI_measured_target[1][i]));
		tmp2 = DRTM_ABS(((int)VIP_DI_measured[0][i]) - ((int)VIP_DI_measured_target[2][i]));*/
		target_diff_cnt = 0;
		diff_target_di = (VIP_DI_measured[0][i] * 100)>>10; // 10%
		for (j=0;j<VIP_DRTM_target_num;j++) {
			tmp = DRTM_ABS(((int)VIP_DI_measured[0][i]) - ((int)VIP_DI_measured_target[j][i]));
			if (tmp>diff_target_di)
				target_diff_cnt++;
		}
		if (target_diff_cnt >= VIP_DRTM_target_num) {
			target_idx_tmp = VIP_DI_target_idx[i];
			VIP_DI_measured_target[target_idx_tmp][i] = VIP_DI_measured[0][i];
			porch_print_flag = 1;
			VIP_DI_target_idx[i]++;
			if (VIP_DI_target_idx[i] >= VIP_DRTM_target_num)
				VIP_DI_target_idx[i] = 0;
		}
	}

	if ((diff_rtnr[0] != 0) || (diff_rtnr[3] != 0) || (diff_di[0] != 0) || (diff_di[3] != 0) || (porch_print_flag == 1)) {

		rtd_printk(KERN_INFO,"DRTM", "t=%d->%d,RTNR:Vact:%d->%d,Hact:%d->%d,DI:Vact:%d->%d,Hact:%d->%d,\n",
			VIP_DI_RTNR_measured_90k[1], VIP_DI_RTNR_measured_90k[0],
			VIP_RTNR_measured[1][0], VIP_RTNR_measured[0][0],
			VIP_RTNR_measured[1][3], VIP_RTNR_measured[0][3],
			VIP_DI_measured[1][0], VIP_DI_measured[0][0],
			VIP_DI_measured[1][3], VIP_DI_measured[0][3]);

		rtd_printk(KERN_INFO,"DRTM", "RTNR,1:%d,%d,%d@%d,2:%d,%d,%d@%d,4:%d,%d,%d@%d,5:%d,%d,%d@%d,\n",
			VIP_RTNR_measured_target[0][1], VIP_RTNR_measured_target[1][1], VIP_RTNR_measured_target[2][1], VIP_RTNR_measured[0][1],
			VIP_RTNR_measured_target[0][2], VIP_RTNR_measured_target[1][2], VIP_RTNR_measured_target[2][2], VIP_RTNR_measured[0][2],
			VIP_RTNR_measured_target[0][4], VIP_RTNR_measured_target[1][4], VIP_RTNR_measured_target[2][4], VIP_RTNR_measured[0][4],
			VIP_RTNR_measured_target[0][5], VIP_RTNR_measured_target[1][5], VIP_RTNR_measured_target[2][5], VIP_RTNR_measured[0][5]);

		rtd_printk(KERN_INFO,"DRTM", "DI,1:%d,%d,%d@%d,2:%d,%d,%d@%d,4:%d,%d,%d@%d,5:%d,%d,%d@%d,\n",
			VIP_DI_measured_target[0][1], VIP_DI_measured_target[1][1], VIP_DI_measured_target[2][1], VIP_DI_measured[0][1],
			VIP_DI_measured_target[0][2], VIP_DI_measured_target[1][2], VIP_DI_measured_target[2][2], VIP_DI_measured[0][2],
			VIP_DI_measured_target[0][4], VIP_DI_measured_target[1][4], VIP_DI_measured_target[2][4], VIP_DI_measured[0][4],
			VIP_DI_measured_target[0][5], VIP_DI_measured_target[1][5], VIP_DI_measured_target[2][5], VIP_DI_measured[0][5]);

		/*rtd_printk(KERN_INFO,"DRTM", "RTNR,1:%d,%d,2:%d,%d,4:%d,%d,5:%d,%d, DI,1:%d,%d,2:%d,%d,4:%d,%d,5:%d,%d,\n",
			VIP_RTNR_measured[0][1], VIP_RTNR_measured[1][1],
			VIP_RTNR_measured[0][2], VIP_RTNR_measured[1][2],
			VIP_RTNR_measured[0][4], VIP_RTNR_measured[1][4],
			VIP_RTNR_measured[0][5], VIP_RTNR_measured[1][5],
			VIP_DI_measured[0][1], VIP_DI_measured[1][1],
			VIP_DI_measured[0][2], VIP_DI_measured[1][2],
			VIP_DI_measured[0][4], VIP_DI_measured[1][4],
			VIP_DI_measured[0][5], VIP_DI_measured[1][5]);*/

	}

	for (j=0;j<(VIP_DRTM_buf_num-1);j++) {
		memcpy(&VIP_RTNR_measured[VIP_DRTM_buf_num-1-j][0], &VIP_RTNR_measured[VIP_DRTM_buf_num-2-j][0], sizeof(int)*VIP_DRTM_Monitor_items);
		memcpy(&VIP_DI_measured[VIP_DRTM_buf_num-1-j][0], &VIP_DI_measured[VIP_DRTM_buf_num-2-j][0], sizeof(int)*VIP_DRTM_Monitor_items);
		VIP_DI_RTNR_measured_90k[VIP_DRTM_buf_num-1-j] = VIP_DI_RTNR_measured_90k[VIP_DRTM_buf_num-2-j];
	}


/*else if (((diff_rtnr[0][1]>diff_target_rtnr) && (diff_rtnr[1][1]>diff_target_rtnr) && (diff_rtnr[2][1]>diff_target_rtnr))
				|| ((diff_rtnr[0][2]>diff_target_rtnr) && (diff_rtnr[1][2]>diff_target_rtnr) && (diff_rtnr[2][2]>diff_target_rtnr))
				|| ((diff_rtnr[0][4]>diff_target_rtnr) && (diff_rtnr[1][4]>diff_target_rtnr) && (diff_rtnr[2][4]>diff_target_rtnr))
				|| ((diff_rtnr[0][5]>diff_target_rtnr) && (diff_rtnr[1][5]>diff_target_rtnr) && (diff_rtnr[2][5]>diff_target_rtnr))
				|| ((diff_di[0][1]>diff_target_di) && (diff_di[1][1]>diff_target_di) && (diff_di[2][1]>diff_target_di))
				|| ((diff_di[0][2]>diff_target_di) && (diff_di[1][2]>diff_target_di) && (diff_di[2][2]>diff_target_di))
				|| ((diff_di[0][4]>diff_target_di) && (diff_di[1][4]>diff_target_di) && (diff_di[2][4]>diff_target_di))
				|| ((diff_di[0][5]>diff_target_di) && (diff_di[1][5]>diff_target_di) && (diff_di[2][5]>diff_target_di)))*/
}
#endif


void fwif_color_set_live_colour(unsigned char level)
{
	short gain_preset[4] = {0, 64, 96, 128};
	int dhue;
	int dsatbysat[SATSEGMAX];
	int ditnbyitn[ITNSEGMAX];

	if (level > 3)
		return;

	down(&ICM_Semaphore);
	fwif_color_icm_global_gain_access(1, ICM_Global_Sat, &gain_preset[level]);
	drvif_color_icm_Get_Global_Ctrl(FALSE, &dhue, dsatbysat, ditnbyitn);
	fwif_color_icm_global_gain_calc(&dhue, dsatbysat, ditnbyitn);
	fwif_color_icm_global_adjust_on_elem_table(&icm_tab_elem_of_vip_table, dhue, dsatbysat, ditnbyitn, &icm_tab_elem_write);
	//drvif_color_icm_Write3DTable(icm_tab_elem_write.elem);
	fwif_color_icm_Write3DTable_By_DMA(icm_tab_elem_write.elem);
	up(&ICM_Semaphore);
}

void fwif_color_set_mastered_4k(void) /*fake function*/
{
	unsigned short value = Scaler_GetSharpnessTable();
	unsigned char mode = Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
	unsigned char gain = 0;

	if ((mode == _MODE_1080P60) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) < 245) && (Scaler_DispGetInputInfo(SLR_INPUT_V_FREQ) > 235)) {
		SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
		if (gVip_Table == NULL) {
			VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
			return;
		}

		if (Scaler_get_Clarity_Mastered4K()) {
			gain = 128;
		} else {
			if (value < Sharp_table_num)
				gain = gVip_Table->Ddomain_SHPTable[value].SR_Control.Ring_Gen_Gain;
		}
		drvif_color_setSR_Ring_Gen_Gain(gain);
	}
}

unsigned char DI_SW_Reset_INIT_Flag = 1;
extern unsigned int DI_SW_Detection_in_VPQ_TSK_Stable_CNT;
extern unsigned int DI_SW_Detection_90k;
void fwif_color_DI_IP_ini(void)
{
	unsigned long addr_sta, addr_end, size;

	DI_SW_Reset_INIT_Flag = 0;
	DI_SW_Detection_90k = 0;
	DI_SW_Detection_in_VPQ_TSK_Stable_CNT = 0;
#if 0
	addr_sta = get_query_start_address(QUERY_IDX_DI);
	if (get_platform_model() == PLATFORM_MODEL_2K)
		addr_end = addr_sta + VIP_DI_1_BUFFER_SIZE_2K_PLATFORM;
	else
		addr_end = addr_sta + VIP_DI_1_BUFFER_SIZE;
#else
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
	size = carvedout_buf_query_secure(CARVEDOUT_SCALER_DI_NR, (void **)&addr_sta) ;
#else
	size = carvedout_buf_query(CARVEDOUT_SCALER_DI_NR, (void **)&addr_sta) ;
#endif
	addr_end = addr_sta + size;
#endif
	rtd_pr_vpq_info("DI_Bnd_setting, sta=%lx, end=%lx, size=%lx,\n", addr_sta, addr_end, size);
	drvif_color_DI_Addr_Boundary_ini((unsigned int)addr_sta, (unsigned int)addr_end, (unsigned int)addr_sta, (unsigned int)addr_end);
	drvif_color_set_DI_detection(1, 0 ,0);
	drv_memory_di_Set_PQC_init();
	drvif_color_set_DI_RTNR_Timing_Measurement();
	
}

void fwif_color_DI_IEGSM_ini(void)
{
	unsigned long addr_sta, addr_end, size;
#if 0
	addr_sta = get_query_start_address(QUERY_IDX_DI);
	if (get_platform_model() == PLATFORM_MODEL_2K)
		addr_end = addr_sta + VIP_DI_1_BUFFER_SIZE_2K_PLATFORM;
	else
		addr_end = addr_sta + VIP_DI_1_BUFFER_SIZE;
#else
#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
	size = carvedout_buf_query_secure(CARVEDOUT_SCALER_DI_NR, (void **)&addr_sta) ;
#else
	size = carvedout_buf_query(CARVEDOUT_SCALER_DI_NR, (void **)&addr_sta) ;
#endif
	addr_end = addr_sta + size;
#endif
	drvif_color_iEdgeSmooth_Addr_Boundary_ini((unsigned int)addr_sta, (unsigned int)addr_end);
}

unsigned int get_query_VIP_DMAto3DTABLE_start_address(unsigned char idx)
{
#ifndef BUILD_QUICK_SHOW

	unsigned int base_addr =0, addr = 0;

 	base_addr = get_query_start_address(QUERY_IDX_VIP_DMAto3DTABLE);
	//base_addr = drvif_memory_get_data_align(base_addr, (1 << 12));

	if (base_addr == 0) {
			rtd_pr_vpq_emerg("\n  get DMA table index error, base address = 0 \n");
	} else {
		if (idx == DMAto3DTABLE_HDR_3D_LUT)
			addr = base_addr;
		else if (idx == DMAto3DTABLE_HDR_EOTF)
			addr = base_addr + VIP_DMAto3DTable_HDR_3D_LUT_SIZE;
		else if (idx == DMAto3DTABLE_HDR_OETF)
			addr = base_addr + VIP_DMAto3DTable_HDR_3D_LUT_SIZE + VIP_DMAto3DTable_HDR_EOTF_SIZE;
		else if (idx == DMAto3DTABLE_HDR_D_3D_LUT)
			addr = base_addr + VIP_DMAto3DTable_HDR_3D_LUT_SIZE + VIP_DMAto3DTable_HDR_EOTF_SIZE + VIP_DMAto3DTable_HDR_OETF_SIZE;
		else if (idx == DMAto3DTABLE_ICM)
			addr = base_addr + VIP_DMAto3DTable_HDR_3D_LUT_SIZE + VIP_DMAto3DTable_HDR_EOTF_SIZE + VIP_DMAto3DTable_HDR_OETF_SIZE + VIP_DMAto3DTable_D_3D_LUT_SIZE;
		else if (idx == DMAto3DTABLE_TC)
			addr = base_addr + VIP_DMAto3DTable_HDR_3D_LUT_SIZE + VIP_DMAto3DTable_HDR_EOTF_SIZE + VIP_DMAto3DTable_HDR_OETF_SIZE + VIP_DMAto3DTable_D_3D_LUT_SIZE +
				VIP_DMAto3DTable_ICM_SIZE;
		else
			rtd_pr_vpq_emerg("\n  get DMA table index error, idx = %d \n", idx);
	}
	return addr;

#else
	return 0;
#endif

}

//extern DRV_DeMura_CTRL_TBL DeMura_CTRL_TBL;
//extern VIP_DeMura_TBL DeMura_TBL;
unsigned char fwif_color_set_DeMura_TBL_Mode(unsigned char mode)
{
	if (mode >= VIP_DeMura_TBL_Mode_MAX)
		mode = DeMura_TBL_241x136;
	DeMura_TBL.table_mode = mode;
	return 0;
}

unsigned char fwif_color_get_DeMura_TBL_Mode(void)
{
	return DeMura_TBL.table_mode;
}

char fwif_color_DeMura_init(void)
{
#ifndef BUILD_QUICK_SHOW

	extern DRV_DeMura_DMA_TBL DMA_TBL_Buff;
	DRV_DeMura_DMA_TBL DMA_TBL;

	//unsigned int phy_memory, size;
	DRV_DeMura_CTRL_TBL buff = {0};
	unsigned int tbl_line_size;
	unsigned short panel_width, panel_heigth, h_s, h_e, v_s, v_e;
	unsigned int base_addr;
	unsigned int *vir_addr;
	char ret = 0;
	unsigned int base_addr_aligned = 0, sizeDemuraTbl_aligned;
	unsigned int  *vir_addr_aligned = NULL;
#ifdef CONFIG_ARM64 //ARM32 compatible
	unsigned long va_temp;
#else
	unsigned int va_temp;
#endif
	unsigned char aligned_val = 0;
	//unsigned int sizeDemuraTbl = sizeof(unsigned char)*VIP_Demura_Encode_TBL_ROW*VIP_Demura_Encode_TBL_COL;
	unsigned int size_offset = 10 * 1024; // avoid trash other memory while size do 4k and 96B aligned
	unsigned int sizeDemuraTbl = VIP_DeMura_BUFFER_SIZE - size_offset;

	drvif_color_Get_main_den_h_v_start_end(&h_s, &h_e, &v_s, &v_e);

	panel_width = h_e - h_s;
	panel_heigth = v_e - v_s;

	memcpy(&buff, &DeMura_TBL.DeMura_CTRL_TBL, sizeof(DRV_DeMura_CTRL_TBL));

	base_addr = get_query_start_address(QUERY_IDX_VIP_DeMura);
	base_addr = drvif_memory_get_data_align(base_addr, (1 << 12));
	sizeDemuraTbl = drvif_memory_get_data_align(sizeDemuraTbl, (1 << 12));
	if (base_addr == 0) {
		rtd_pr_vpq_emerg("[%s] addr = NULL\n", __func__);
		return -1;
	}
	vir_addr = dvr_remap_uncached_memory(base_addr, sizeDemuraTbl, __builtin_return_address(0));

	if (DeMura_TBL.table_mode == DeMura_TBL_481x271) {
		DMA_TBL.burst_len = 32;
		if (DeMura_TBL.DeMura_CTRL_TBL.demura_rgb_table_seperate == DeMura_Ch_Y_Mode) {	/* Y mode */
			/*real line step in unit of 128 bit*/
			DMA_TBL.line_128_num = 69;		/* 68*128+5*18 = 8794 bit = 68.703125 128_bit */
			DMA_TBL.line_step = (VIP_DemuraTBL_LineSize_Y_481x271 >> 4);	/* 481/7 = 68.714 ~= 69, 69*128 = 8832 bit = 1104 byte */
			tbl_line_size = VIP_DemuraTBL_LineSize_Y_481x271;
		} else {
			/*real line step in unit of 128 bit*/
			DMA_TBL.line_128_num = 207;		/* 68*128*3+128*2+18 = 26386 bit = 206.140625 128_bit */
			DMA_TBL.line_step = (VIP_DemuraTBL_LineSize_RGB_481x271 >> 4);	/* 481/7 = 68.714 ~= 69, 69*128*3 = 26496 bit = 3312 byte */
			tbl_line_size = VIP_DemuraTBL_LineSize_RGB_481x271;
		}
		DMA_TBL.table_height = 271;		/* table mode = DeMura_TBL_481x271, table size = 481x271 for 2k(blk size=4x4) and 4k(blk size=8x8) panel*/
		DMA_TBL.table_width = 481;

		if (panel_heigth == 2160 && panel_width == 3840)	/* 4k panel*/
			buff.demura_block_size = 0;		/* block size = 8x8 */
		else if (panel_heigth == 1080 && panel_width == 1920)/* 2k panel*/
			buff.demura_block_size = 2;		/* block size = 4x4 */
		else
			buff.demura_en = 0;

	} else {
		DMA_TBL.burst_len = 32;
		if (DeMura_TBL.DeMura_CTRL_TBL.demura_rgb_table_seperate == DeMura_Ch_Y_Mode) {	/* Y mode */
			/*real line step in unit of 128 bit*/
			DMA_TBL.line_128_num = 35;		/* 34*128+3*18 = 4406 bit = 34.421875 128_bit */
			DMA_TBL.line_step = (VIP_DemuraTBL_LineSize_Y_241x136 >> 4);	/* 241/7 = 34.428 ~= 35, 35*128 = 4480 bit = 560 byte */
			tbl_line_size = VIP_DemuraTBL_LineSize_Y_241x136;
		} else {
			/*real line step in unit of 128 bit*/
			DMA_TBL.line_128_num = 104;		/* 34*128*3+128+36 = 13220 bit = 103.28125 128_bit */
			DMA_TBL.line_step = (VIP_DemuraTBL_LineSize_RGB_241x136 >> 4);	/* 241/7 = 34.428 ~= 35, 35*128*3 = 13440 bit = 1680 byte */
			tbl_line_size = VIP_DemuraTBL_LineSize_RGB_241x136;
		}
		DMA_TBL.table_height = 136;		/* table mode = DeMura_TBL_241x136, table size = 136x241 for 2k(blk size=8x8) and 4k(blk size=16x16) panel*/
		DMA_TBL.table_width = 241;

		if (panel_heigth == 2160 && panel_width == 3840)	/* 4k panel*/
			buff.demura_block_size = 1;		/* block size = 16x16 */
		else if (panel_heigth == 1080 && panel_width == 1920)/* 2k panel*/
			buff.demura_block_size = 0;		/* block size = 8x8 */
		else
			buff.demura_en = 0;
	}

	sizeDemuraTbl_aligned = sizeDemuraTbl;
	base_addr_aligned = base_addr;
#ifdef CONFIG_ARM64 //ARM32 compatible
	va_temp = (unsigned long)vir_addr;
#else
	va_temp = (unsigned int)vir_addr;
#endif
	vir_addr_aligned = (unsigned int*)va_temp;
	aligned_val = 0;

#ifdef CONFIG_BW_96B_ALIGNED
	sizeDemuraTbl_aligned = dvr_size_alignment(sizeDemuraTbl);
	base_addr_aligned = (unsigned int)dvr_memory_alignment((unsigned long)base_addr, sizeDemuraTbl_aligned);
#ifdef CONFIG_ARM64 //ARM32 compatible
	va_temp = (unsigned long)vir_addr;
#else //CONFIG_ARM64
	va_temp = (unsigned int)vir_addr;
#endif //CONFIG_ARM64
	vir_addr_aligned = (unsigned int*)dvr_memory_alignment((unsigned long)va_temp, sizeDemuraTbl_aligned);
	aligned_val = 6;
#endif

	if (aligned_val != 0) {
		DMA_TBL.burst_len = ((DMA_TBL.burst_len%aligned_val)==0)?(DMA_TBL.burst_len):(DMA_TBL.burst_len - (DMA_TBL.burst_len % aligned_val));
		//DMA_TBL.line_step = ((DMA_TBL.line_step%aligned_val)==0)?(DMA_TBL.line_step):(DMA_TBL.line_step + (aligned_val - (DMA_TBL.line_step % aligned_val)));//merlin5 will add. h5x, delete
	}

	/*printf("[DeMura] = hs=%d,he=%d,vs=%d,ve=%d,ret=%d,\n", h_s, h_e, v_s, v_e, DMA_ret);*/

	memcpy((unsigned char*)&DMA_TBL_Buff, (unsigned char*)&DMA_TBL, sizeof(DRV_DeMura_DMA_TBL));
	ret = drvif_color_set_DeMura(&DMA_TBL, &buff, &DeMura_TBL.TBL[0], base_addr_aligned, vir_addr_aligned, tbl_line_size);

	dvr_unmap_memory((void *)vir_addr, sizeDemuraTbl);

	return ret;
#else

	return 0;
#endif
}

char fwif_color_Get_DeMura_CTRL(DRV_DeMura_CTRL_TBL *ptr)
{
	char ret;
	ret = drvif_color_Get_DeMura_CTRL(ptr);
	return ret;
}
#ifndef BUILD_QUICK_SHOW

char fwif_color_set_DeMura_En(unsigned char enable)
{
	drvif_color_set_DeMura_En(enable);
	return 0;
}
#endif

unsigned char fwif_color_get_DeMura_En(void)
{
	return drvif_color_get_DeMura_En();
}

#if 1
/*========================= for INNX demura =================================*/
static int fwif_color_INNX_Demura_round_mod4_inx(int number)
{
	int o_value=0;
//number > 0
	if(  (number%4) == 0 ||   (number%4) == 1)
	{
		o_value = number ;
	}
	else if( (number%4) == 2)
	{
		o_value = number +2;
	}
	else //( (number%4) == 3)
	{
		o_value = number +1;
	}

	return o_value;
}

static int fwif_color_INNX_Demura_round_mod8192_inx(int number)
{
	int o_value=0;
//number > 0
	if(  (number%8192) < (8192/2))
	{
		o_value = number ;
	}
	else if( (number%4) == 2)
	{
		o_value = number +8192;
	}
	else //( (number%4) == 3)
	{
		o_value = number +8192;
	}

	return o_value;
}


static unsigned char fwif_color_INNX_Demura_pow2_inx( unsigned char read_bit)
{
    unsigned char pow_value=0;

	pow_value = (1 << read_bit);

	return pow_value;
}


static unsigned int fwif_color_INNX_Demura_crcArray_sumAll(unsigned char* data, unsigned int crcStart, unsigned int crcLength)
{
			unsigned int crc_out;

			int index = 0;

			crc_out = 0;

			for (index = crcStart; index < crcStart + crcLength; index += 1)
			{
				crc_out = crc_out + (unsigned int)data[index];
			}

			if((crc_out&0xffff) !=  (unsigned int)(data[ INNX_Demura_addr_LUTCheckSum ]<<8) +  (unsigned int)(data[ INNX_Demura_addr_LUTCheckSum+1 ])   )
			{
				rtd_pr_vpq_emerg("INNX_Demura---lut checksum error---\n");
				rtd_pr_vpq_emerg("INNX_Demura---lut checksum golden %x---\n", (unsigned int)(data[ INNX_Demura_addr_LUTCheckSum ]<<8) +  (unsigned int)(data[ INNX_Demura_addr_LUTCheckSum+1 ])   );
				rtd_pr_vpq_emerg("INNX_Demura---lut checksum compute %x---\n", crc_out   );
				return 1;
			}


			return 0;
}


static char fwif_color_INNX_Demura_Check_header_Information(unsigned char* arr, unsigned int* Block_Type,unsigned int* Block_DataChecksum, unsigned int* Block_StartAddress, unsigned int* Block_DataLength,unsigned int* gray_setting)
{
	unsigned int i,block_idx, compute_header_Sum = 0;
	unsigned char flag_find_lut1=0, Identification=0, DemuraFunctionDefinition =0;
	unsigned char ASIC[5];


// 1.check whether header is valid

	Identification = arr[INNX_Demura_addr_Identification];
	if(!(Identification == 0x99)) return 1;

	for(i=0 ; i<5 ; i++)
	{
		ASIC[i] = arr[INNX_Demura_addr_ASIC+i];
	}
	if(ASIC[0] == 0x09 && ASIC[1] == 0x09 && ASIC[2] == 0x09 && ASIC[3] == 0x09 && ASIC[4] == 0x09 )
	{
		rtd_pr_vpq_emerg("INNX_Demura----------ASIC error ----------\n");
		return 2;
	}


	DemuraFunctionDefinition =  arr[INNX_Demura_addr_DemuraFunctionDefinition]&0x1;
	if(!(DemuraFunctionDefinition == 0x1)) return 3;
//end : 1.check whether header is valid

	block_idx=0;
	do
	{

		if(((arr[INNX_Demura_LUT_address_find+(block_idx*12)])& 0xf )== 1)
		{
			flag_find_lut1 = 1;
			break;
		}

		block_idx = block_idx+1;

	}while(flag_find_lut1 ==0);

	*Block_Type				= (arr[INNX_Demura_LUT_address_find+(block_idx*12)-1]<<8) + (arr[INNX_Demura_LUT_address_find+(block_idx*12)]);
	*Block_DataChecksum      = (arr[INNX_Demura_LUT_address_find+(block_idx*12)+1]<<8) + (arr[INNX_Demura_LUT_address_find+(block_idx*12)+2]);//little/big endian ???? check !!!!!!!
	*Block_StartAddress      = (arr[INNX_Demura_LUT_address_find+(block_idx*12)+3]<<24) + (arr[INNX_Demura_LUT_address_find+(block_idx*12)+4]<<16)+(arr[INNX_Demura_LUT_address_find+(block_idx*12)+5]<<8)+(arr[INNX_Demura_LUT_address_find+(block_idx*12)+6]);
	*Block_DataLength        = (arr[INNX_Demura_LUT_address_find+(block_idx*12)+7]<<24) + (arr[INNX_Demura_LUT_address_find+(block_idx*12)+8]<<16)+(arr[INNX_Demura_LUT_address_find+(block_idx*12)+9]<<8)+(arr[INNX_Demura_LUT_address_find+(block_idx*12)+10]);

//compute header checksum
	for(i = 0 ; i<0x7e ; i++)
	{
		 compute_header_Sum = compute_header_Sum + arr[i];
	}

	if(compute_header_Sum !=  (unsigned int)(arr[ INNX_Demura_addr_HeaderCheckSum ]<<8) +  (unsigned int)(arr[ INNX_Demura_addr_HeaderCheckSum+1 ])   )
	{

		rtd_pr_vpq_emerg("INNX_Demura---header checksum error---\n");
		rtd_pr_vpq_emerg("INNX_Demura---header checksum golden %x---\n", (unsigned int)(arr[ INNX_Demura_addr_HeaderCheckSum ]<<8) +  (unsigned int)(arr[ INNX_Demura_addr_HeaderCheckSum+1 ])  );
		rtd_pr_vpq_emerg("INNX_Demura---header checksum compute %x---\n", compute_header_Sum );
		return 4;
	}

//read setting gray bar
	gray_setting[0]     = (unsigned int)arr[  INNX_Demura_addr_BotLimit ]<<2;// gray_setting[0] = low cutoff
	gray_setting[1]     = (unsigned int)arr[   INNX_Demura_addr_TopLimit]<<2;// gray_setting[1] = high cutoff
	gray_setting[2]     = (unsigned int)arr[   INNX_Demura_addr_Gray1	]<<2;// gray_setting[2] = low level
	gray_setting[3]     = (unsigned int)arr[   INNX_Demura_addr_Gray2	]<<2;// gray_setting[3] = mid level
	gray_setting[4]     = (unsigned int)arr[   INNX_Demura_addr_Gray3	]<<2;// gray_setting[4] = high level
	gray_setting[5]     = (unsigned int)arr[   INNX_Demura_addr_Gray4	]<<2;//no use
	gray_setting[6]     = (unsigned int)arr[   INNX_Demura_addr_Gray5	]<<2;//no use

	return 0;

}

//static void fwif_color_INNX_Demura_Decode_AQC_inx(unsigned char Code[31*271][22], char DeLut[271][496])
static void fwif_color_INNX_Demura_Decode_AQC_inx(unsigned char *Code, char *DeLut)
{
	int H = 271;
	//int W = 481;
	int w16 = 496;

	//int Code, DeLut;
	int i=0;
	int j=0;
	int k=0;
	int m=0;
	int Mode=0;
	int Data[4]={0};

    int Min ;
    int Range;
    int M, N, Q;
    int subRange1 ;
    int subRange2 ;
    int subRange3 ;


    for ( i = 0; i < H; i++)
    {
        for ( j = 0; j < (w16 / 16); j++)
        {
            Min 	    = (int)Code[((i * 31 + j) * INNX_Demura_num_22) + 21];
            Range      = (int)Code[((i * 31 + j) * INNX_Demura_num_22) +  20];

            subRange1 = fwif_color_INNX_Demura_round_mod4_inx((Range * 1))/ 4 + Min;
            subRange2 = fwif_color_INNX_Demura_round_mod4_inx((Range * 2))/ 4 + Min;
            subRange3 = fwif_color_INNX_Demura_round_mod4_inx((Range * 3))/ 4 + Min;


            for ( k = 0; k < 4; k++)
            {
                 Mode = Code[((i * 31 + j) * INNX_Demura_num_22) +  (16 + k)];

                for ( m = 0; m < 4; m++)
                {
                    Data[m] = Code[((i * 31 + j) * INNX_Demura_num_22) +   (k * 4 + m)];
                }

                switch (Mode)
                {
                    case 0:
                        M = Min+Range;
                        N = Min;
                        break;

                    case 1:
                        M = subRange3;
                        N = Min;
                        break;

                    case 2:
                        M = subRange2;
                        N = Min;
                        break;

                    case 3:
                        M = subRange1;
                        N = Min;
                        break;

                    case 4:
                        M = Min + Range;
                        N = subRange1;
                        break;

                    case 5:
                        M = Min + Range;
                        N = subRange2;
                        break;

                    case 6:
                        M = Min + Range;
                        N = subRange3;
                        break;

                    case 7:
                        M = subRange3;
                        N = subRange1;
                        break;

                    default:
                        M = Min + Range;
                        N = Min;
                        break;
                }
                Q = M - N;

                for ( m = 0; m < 4; m++)
                {
                    if (Q != 0)
                        //DeLut[i][j*16 + k*4 + m] = (int) round((((double)Data[m] * 546 / 8192) * Q + N));
                        DeLut[(i * INNX_Demura_W16) + (j*16 + k*4 + m)] = (char) ((fwif_color_INNX_Demura_round_mod8192_inx((int)Data[m] * 546 * Q)/ 8192)  + N);
                    else
                        DeLut[(i * INNX_Demura_W16) + (j*16 + k*4 + m)] = (char)N;
                }
            }
        }
    }
}

char fwif_color_INNX_Demura_TBL(unsigned char * pINNX_DemuraTBL, unsigned short TBL_Size, char *pDeLut)
{
#ifndef BUILD_QUICK_SHOW
	//long binSize;
	unsigned int i,k,ii,m,n,h,startAddress;
	unsigned char  c_left_BIT,read_BIT_value, c_read_BIT;
	unsigned char c_read_BIT_arr[4] = {4,3,7,7};
	unsigned int processing_BIT;
	unsigned int Block_Type=0	;
	unsigned int Block_DataChecksum=0  ;
	unsigned int Block_StartAddress=0   ;
	unsigned int Block_DataLength=0     ;
	unsigned int lut_crc_out = 0;
	char *INNX_Demura_DeLut1;
	char *INNX_Demura_DeLut2;
	char *INNX_Demura_DeLut3;
	char *INNX_Demura_DeLut4;
	char *INNX_Demura_DeLut5;
	//unsigned char crc_out = 0;
	//unsigned char BotLimit =0;
	//unsigned char TopLimit =0;
	/*unsigned char Gray1    =0;
	unsigned char Gray2    =0;
	unsigned char Gray3    =0;
	unsigned char Gray4    =0;
	unsigned char Gray5    =0;*/
	//unsigned int gray_setting[7];
	char ret = 0;

	INNX_Demura_CodeLutALL_row = (unsigned char *)dvr_malloc_specific((31 * 22) * sizeof(char), GFP_DCU2_FIRST);
	INNX_Demura_CodeLutAll = (unsigned char *)dvr_malloc_specific((31 * INNX_Demura_h_271 * 22) * sizeof(char), GFP_DCU2_FIRST);
	INNX_Demura_header_infor = (unsigned char *)dvr_malloc_specific(INNX_Demura_size_header_infor * sizeof(char), GFP_DCU2_FIRST);
	INNX_Demura_DataByRow = (unsigned char *)dvr_malloc_specific((22 * 5 * 31 * 271) * sizeof(char), GFP_DCU2_FIRST);

	if ((INNX_Demura_CodeLutALL_row == NULL) || (INNX_Demura_CodeLutAll == NULL) || (INNX_Demura_header_infor == NULL)
		|| (INNX_Demura_DataByRow == NULL) || (pDeLut == NULL)) {
		rtd_pr_vpq_emerg("INNX_Demura, dynamic alloc fail, CodeLutALL_row=%p, CodeLutAll=%p, header_infor=%p, DataByRow=%p, pDeLut=%p\n",
			INNX_Demura_CodeLutALL_row, INNX_Demura_CodeLutAll, INNX_Demura_header_infor, INNX_Demura_DataByRow, pDeLut);
		return -1;
	}

	if (pINNX_DemuraTBL == NULL) {
		rtd_pr_vpq_emerg("INNX_Demura----------pINNX_DemuraTBL = NULL ----------\n");
		return -1;
	}

	rtd_pr_vpq_info("INNX_Demura, set INNX Demura, INNX demura size =%d,\n", TBL_Size);

	INNX_Demura_DeLut1 = &pDeLut[INNX_Demura_DecodeLUT_Low_sta_addr];
	INNX_Demura_DeLut2 = &pDeLut[INNX_Demura_DecodeLUT_Mid_sta_addr];
	INNX_Demura_DeLut3 = &pDeLut[INNX_Demura_DecodeLUT_High_sta_addr];
	INNX_Demura_DeLut4 = &pDeLut[INNX_Demura_DecodeLUT_4_sta_addr];
	INNX_Demura_DeLut5 = &pDeLut[INNX_Demura_DecodeLUT_5_sta_addr];

	ret = fwif_color_INNX_Demura_Check_header_Information(pINNX_DemuraTBL, &Block_Type, &Block_DataChecksum,  &Block_StartAddress,  &Block_DataLength,INNX_Demura_gray_setting);//gray_setting :u(10,0)

	if (ret != 0) {
		rtd_pr_vpq_emerg("INNX_Demura-------header check error, error code = %d ----------\n", ret);
		return ret;
	}

	//lut_crc_out = crcArray(buffer, lut_crc_out,Block_StartAddress, Block_DataLength);
	lut_crc_out = fwif_color_INNX_Demura_crcArray_sumAll(pINNX_DemuraTBL, Block_StartAddress, Block_DataLength);

	if (lut_crc_out != 0) {
		rtd_pr_vpq_emerg("INNX_Demura-------CRC check error, error code = %d ----------\n", lut_crc_out);
		return lut_crc_out;
	}


	//lut 1
	c_left_BIT= 8;

	startAddress = Block_StartAddress;

	ii= startAddress;

	processing_BIT=pINNX_DemuraTBL[ii];

	k=0;//c_read_BIT idx
	c_read_BIT = c_read_BIT_arr[k%22];


	h=0;

	while(ii<Block_DataLength+startAddress)
	{

	do {
		read_BIT_value = (unsigned char)(fwif_color_INNX_Demura_pow2_inx(c_read_BIT)-1);

		INNX_Demura_DataByRow[k]  = processing_BIT&read_BIT_value;
		k=k+1;


		c_left_BIT = c_left_BIT - c_read_BIT;
		processing_BIT = processing_BIT>>c_read_BIT;

		if(k%22 < 16)
		{
			c_read_BIT = c_read_BIT_arr[0];
		}else if(k%22 < 20)
		{
			c_read_BIT = c_read_BIT_arr[1];
		}else if(k%22 == 20)
		{
			c_read_BIT = c_read_BIT_arr[2];
		}else if(k%22 == 21)
		{
			c_read_BIT = c_read_BIT_arr[3];
		}
	} while(c_read_BIT < c_left_BIT || c_read_BIT == c_left_BIT);



	if(c_read_BIT > c_left_BIT){
		if(((k % (5*22*31))==0)&&(k!=0))//start next line
		{
			ii=ii+17;
			c_left_BIT =8	;
			processing_BIT =(pINNX_DemuraTBL[ii]);

			k=0;

			for ( m = 0; m < (496 / 16); m++)
			{
			    for( n = 0; n< INNX_Demura_num_22; n++)
			    {
			        INNX_Demura_CodeLutALL_row[ (m*INNX_Demura_num_22)+n]  = INNX_Demura_DataByRow[0*22+m*110+n] ;
			    }
			}

			for ( m = 0; m < (INNX_Demura_W16 / 16); m++)
			{
			    for( n = 0; n< INNX_Demura_num_22; n++)
			    {
					INNX_Demura_CodeLutAll[((h*(INNX_Demura_W16 / 16)+m)*INNX_Demura_num_22)+n] =  INNX_Demura_CodeLutALL_row[ (m*INNX_Demura_num_22)+n] ;
			    }
			}

				h=h+1;
		}
		else
		{
			ii=ii+1;
			processing_BIT = (processing_BIT)+(pINNX_DemuraTBL[ii]<<c_left_BIT);
			c_left_BIT = c_left_BIT+8;
		}
	}
	};


	    fwif_color_INNX_Demura_Decode_AQC_inx(INNX_Demura_CodeLutAll,  INNX_Demura_DeLut1);
#if 0
	    for (i = 0; i < INNX_Demura_h_271; i++)
	    {
	        for (j = 0; j < INNX_Demura_W; j++)
	        {
	            INNX_Demura_DeLut1[i][ j] = INNX_Demura_DeLut1[i][ j]-64;
	        }
	    }
#endif
	//lut 2
	c_left_BIT= 8;

	startAddress = Block_StartAddress;

	ii= startAddress;

	processing_BIT=pINNX_DemuraTBL[ii];

	k=0;//c_read_BIT idx
	c_read_BIT = c_read_BIT_arr[k%22];


	h=0;

	while(ii<Block_DataLength+startAddress)
	{

	do {
		read_BIT_value = (unsigned char)(fwif_color_INNX_Demura_pow2_inx(c_read_BIT)-1);

		INNX_Demura_DataByRow[k]  = processing_BIT&read_BIT_value;
		k=k+1;


		c_left_BIT = c_left_BIT - c_read_BIT;
		processing_BIT = processing_BIT>>c_read_BIT;

		if(k%22 < 16)
		{
			c_read_BIT = c_read_BIT_arr[0];
		}else if(k%22 < 20)
		{
			c_read_BIT = c_read_BIT_arr[1];
		}else if(k%22 == 20)
		{
			c_read_BIT = c_read_BIT_arr[2];
		}else if(k%22 == 21)
		{
			c_read_BIT = c_read_BIT_arr[3];
		}
	} while(c_read_BIT < c_left_BIT || c_read_BIT == c_left_BIT);



	if(c_read_BIT > c_left_BIT){
		if(((k % (5*22*31))==0)&&(k!=0))//start next line
		{
			ii=ii+17;
			c_left_BIT =8	;
			processing_BIT =(pINNX_DemuraTBL[ii]);

			k=0;

			for ( m = 0; m < (496 / 16); m++)
			{
			    for( n = 0; n< INNX_Demura_num_22; n++)
			    {
			        INNX_Demura_CodeLutALL_row[ (m*INNX_Demura_num_22)+n]  = INNX_Demura_DataByRow[1*22+m*110+n] ;
			    }
			}

			for ( m = 0; m < (INNX_Demura_W16 / 16); m++)
			{
			    for( n = 0; n< INNX_Demura_num_22; n++)
			    {
					INNX_Demura_CodeLutAll[((h*(INNX_Demura_W16 / 16)+m)*INNX_Demura_num_22)+n] =  INNX_Demura_CodeLutALL_row[ (m*INNX_Demura_num_22)+n] ;
			    }
			}

				h=h+1;
		}
		else
		{
			ii=ii+1;
			processing_BIT = (processing_BIT)+(pINNX_DemuraTBL[ii]<<c_left_BIT);
			c_left_BIT = c_left_BIT+8;
		}
	}
	};

	    fwif_color_INNX_Demura_Decode_AQC_inx(INNX_Demura_CodeLutAll,  INNX_Demura_DeLut2);
#if 0
	    for (i = 0; i < INNX_Demura_h_271; i++)
	    {
	        for (j = 0; j < INNX_Demura_W; j++)
	        {
	            INNX_Demura_DeLut2[i][ j] = INNX_Demura_DeLut2[i][ j]-64;
	        }
	    }
#endif


	//lut 3
	c_left_BIT= 8;

	startAddress = Block_StartAddress;

	ii= startAddress;

	processing_BIT=pINNX_DemuraTBL[ii];

	k=0;//c_read_BIT idx
	c_read_BIT = c_read_BIT_arr[k%22];


	h=0;

	while(ii<Block_DataLength+startAddress)
	{

	do {
		read_BIT_value = (unsigned char)(fwif_color_INNX_Demura_pow2_inx(c_read_BIT)-1);

		INNX_Demura_DataByRow[k]  = processing_BIT&read_BIT_value;
		k=k+1;


		c_left_BIT = c_left_BIT - c_read_BIT;
		processing_BIT = processing_BIT>>c_read_BIT;

		if(k%22 < 16)
		{
			c_read_BIT = c_read_BIT_arr[0];
		}else if(k%22 < 20)
		{
			c_read_BIT = c_read_BIT_arr[1];
		}else if(k%22 == 20)
		{
			c_read_BIT = c_read_BIT_arr[2];
		}else if(k%22 == 21)
		{
			c_read_BIT = c_read_BIT_arr[3];
		}
	} while(c_read_BIT < c_left_BIT || c_read_BIT == c_left_BIT);



	if(c_read_BIT > c_left_BIT){
		if(((k % (5*22*31))==0)&&(k!=0))//start next line
		{
			ii=ii+17;
			c_left_BIT =8	;
			processing_BIT =(pINNX_DemuraTBL[ii]);

			k=0;

			for ( m = 0; m < (496 / 16); m++)
			{
			    for( n = 0; n< INNX_Demura_num_22; n++)
			    {
			        INNX_Demura_CodeLutALL_row[ (m*INNX_Demura_num_22)+n]  = INNX_Demura_DataByRow[2*22+m*110+n] ;
			    }
			}

			for ( m = 0; m < (INNX_Demura_W16 / 16); m++)
			{
			    for( n = 0; n< INNX_Demura_num_22; n++)
			    {
					INNX_Demura_CodeLutAll[((h*(INNX_Demura_W16 / 16)+m)*INNX_Demura_num_22)+n] =  INNX_Demura_CodeLutALL_row[ (m*INNX_Demura_num_22)+n] ;
			    }
			}

				h=h+1;
		}
		else
		{
			ii=ii+1;
			processing_BIT = (processing_BIT)+(pINNX_DemuraTBL[ii]<<c_left_BIT);
			c_left_BIT = c_left_BIT+8;
		}
	}
	};


	    fwif_color_INNX_Demura_Decode_AQC_inx(INNX_Demura_CodeLutAll,  INNX_Demura_DeLut3);
#if 0
	    for (i = 0; i < INNX_Demura_h_271; i++)
	    {
	        for (j = 0; j < INNX_Demura_W; j++)
	        {
	            INNX_Demura_DeLut3[i][ j] = INNX_Demura_DeLut3[i][ j]-64;
	        }
	    }
#endif
	//lut 4
	c_left_BIT= 8;

	startAddress = Block_StartAddress;

	ii= startAddress;

	processing_BIT=pINNX_DemuraTBL[ii];

	k=0;//c_read_BIT idx
	c_read_BIT = c_read_BIT_arr[k%22];


	h=0;

	while(ii<Block_DataLength+startAddress)
	{

	do {
		read_BIT_value = (unsigned char)(fwif_color_INNX_Demura_pow2_inx(c_read_BIT)-1);

		INNX_Demura_DataByRow[k]  = processing_BIT&read_BIT_value;
		k=k+1;


		c_left_BIT = c_left_BIT - c_read_BIT;
		processing_BIT = processing_BIT>>c_read_BIT;

		if(k%22 < 16)
		{
			c_read_BIT = c_read_BIT_arr[0];
		}else if(k%22 < 20)
		{
			c_read_BIT = c_read_BIT_arr[1];
		}else if(k%22 == 20)
		{
			c_read_BIT = c_read_BIT_arr[2];
		}else if(k%22 == 21)
		{
			c_read_BIT = c_read_BIT_arr[3];
		}
	} while(c_read_BIT < c_left_BIT || c_read_BIT == c_left_BIT);



	if(c_read_BIT > c_left_BIT){
		if(((k % (5*22*31))==0)&&(k!=0))//start next line
		{
			ii=ii+17;
			c_left_BIT =8	;
			processing_BIT =(pINNX_DemuraTBL[ii]);

			k=0;

			for ( m = 0; m < (496 / 16); m++)
			{
			    for( n = 0; n< INNX_Demura_num_22; n++)
			    {
			        INNX_Demura_CodeLutALL_row[ (m*INNX_Demura_num_22)+n]  = INNX_Demura_DataByRow[3*22+m*110+n] ;
			    }
			}

			for ( m = 0; m < (INNX_Demura_W16 / 16); m++)
			{
			    for( n = 0; n< INNX_Demura_num_22; n++)
			    {
					INNX_Demura_CodeLutAll[((h*(INNX_Demura_W16 / 16)+m)*INNX_Demura_num_22)+n] =  INNX_Demura_CodeLutALL_row[ (m*INNX_Demura_num_22)+n] ;
			    }
			}

				h=h+1;
		}
		else
		{
			ii=ii+1;
			processing_BIT = (processing_BIT)+(pINNX_DemuraTBL[ii]<<c_left_BIT);
			c_left_BIT = c_left_BIT+8;
		}
	}
	};


	    fwif_color_INNX_Demura_Decode_AQC_inx(INNX_Demura_CodeLutAll,  INNX_Demura_DeLut4);
#if 0
	    for (i = 0; i < INNX_Demura_h_271; i++)
	    {
	        for (j = 0; j < INNX_Demura_W; j++)
	        {
	            INNX_Demura_DeLut4[i][ j] = INNX_Demura_DeLut4[i][ j]-64;
	        }
	    }
#endif



	//lut 5
	c_left_BIT= 8;

	startAddress = Block_StartAddress;

	ii= startAddress;

	processing_BIT=pINNX_DemuraTBL[ii];

	k=0;//c_read_BIT idx
	c_read_BIT = c_read_BIT_arr[k%22];


	h=0;

	while(ii<Block_DataLength+startAddress)
	{

	do {
		read_BIT_value = (unsigned char)(fwif_color_INNX_Demura_pow2_inx(c_read_BIT)-1);

		INNX_Demura_DataByRow[k]  = processing_BIT&read_BIT_value;
		k=k+1;


		c_left_BIT = c_left_BIT - c_read_BIT;
		processing_BIT = processing_BIT>>c_read_BIT;

		if(k%22 < 16)
		{
			c_read_BIT = c_read_BIT_arr[0];
		}else if(k%22 < 20)
		{
			c_read_BIT = c_read_BIT_arr[1];
		}else if(k%22 == 20)
		{
			c_read_BIT = c_read_BIT_arr[2];
		}else if(k%22 == 21)
		{
			c_read_BIT = c_read_BIT_arr[3];
		}
	} while(c_read_BIT < c_left_BIT || c_read_BIT == c_left_BIT);



	if(c_read_BIT > c_left_BIT){
		if(((k % (5*22*31))==0)&&(k!=0))//start next line
		{
			ii=ii+17;
			c_left_BIT =8	;
			processing_BIT =(pINNX_DemuraTBL[ii]);

			k=0;

			for ( m = 0; m < (496 / 16); m++)
			{
			    for( n = 0; n< INNX_Demura_num_22; n++)
			    {
			        INNX_Demura_CodeLutALL_row[ (m*INNX_Demura_num_22)+n]  = INNX_Demura_DataByRow[4*22+m*110+n] ;
			    }
			}

			for ( m = 0; m < (INNX_Demura_W16 / 16); m++)
			{
			    for( n = 0; n< INNX_Demura_num_22; n++)
			    {
					INNX_Demura_CodeLutAll[((h*(INNX_Demura_W16 / 16)+m)*INNX_Demura_num_22)+n] =  INNX_Demura_CodeLutALL_row[ (m*INNX_Demura_num_22)+n] ;
			    }
			}

				h=h+1;
		}
		else
		{
			ii=ii+1;
			processing_BIT = (processing_BIT)+(pINNX_DemuraTBL[ii]<<c_left_BIT);
			c_left_BIT = c_left_BIT+8;
		}
	}
	};


	    fwif_color_INNX_Demura_Decode_AQC_inx(INNX_Demura_CodeLutAll,  INNX_Demura_DeLut5);
#if 0
	    for (i = 0; i < INNX_Demura_h_271; i++)
	    {
	        for (j = 0; j < INNX_Demura_W; j++)
	        {
	            INNX_Demura_DeLut5[i][ j] = INNX_Demura_DeLut5[i][ j]-64;
	        }
	    }
#endif

	for (i = 0; i < INNX_Demura_DecodeLUT_Size; i++)
		pDeLut[i] = pDeLut[i]-64;

	dvr_free((void *)INNX_Demura_CodeLutALL_row);
	dvr_free((void *)INNX_Demura_CodeLutAll);
	dvr_free((void *)INNX_Demura_header_infor);
	dvr_free((void *)INNX_Demura_DataByRow);
#endif
	return 0;
}


/*========================= for INNX demura =================================*/
#endif

#ifdef CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE
static unsigned int mixer_3x3[2][15] = {
	{0, 0 ,0, 0x0d49, 0x3b4c, 0x3f6b, 0x3f01, 0x0910,  0x3fef, 0x3fdb, 0x3f32, 0x08f3, 0, 0, 0,},		//2020->709
	{0, 0 ,0, 0x0505, 0x02A2, 0x0059, 0x008E, 0x075B,  0x0017, 0x0022, 0x00B4, 0x072A, 0, 0, 0,},		//709->2020
};

void fwif_color_set_OSD_enhance_init(void)
{
	static unsigned int mixer_gamma1[] = {
		0x00001010, 0x00201010, 0x00401010, 0x00601010, 0x00801010, 0x00a01010, 0x00c01010, 0x00e01010, 0x01001010, 0x01201010, 0x01401010, 0x01601010, 0x01801010, 0x01a01010, 0x01c01010, 0x01e01010,
		0x02001010, 0x02201010, 0x02401010, 0x02601010, 0x02801010, 0x02a01010, 0x02c01010, 0x02e01010, 0x03001010, 0x03201010, 0x03401010, 0x03601010, 0x03801010, 0x03a01010, 0x03c01010, 0x03e01010,
		0x04001010, 0x04201010, 0x04401010, 0x04601010, 0x04801010, 0x04a01010, 0x04c01010, 0x04e01010, 0x05001010, 0x05201010, 0x05401010, 0x05601010, 0x05801010, 0x05a01010, 0x05c01010, 0x05e01010,
		0x06001010, 0x06201010, 0x06401010, 0x06601010, 0x06801010, 0x06a01010, 0x06c01010, 0x06e01010, 0x07001010, 0x07201010, 0x07401010, 0x07601010, 0x07801010, 0x07a01010, 0x07c01010, 0x07e01010,
		0x08001010, 0x08201010, 0x08401010, 0x08601010, 0x08801010, 0x08a01010, 0x08c01010, 0x08e01010, 0x09001010, 0x09201010, 0x09401010, 0x09601010, 0x09801010, 0x09a01010, 0x09c01010, 0x09e01010,
		0x0a001010, 0x0a201010, 0x0a401010, 0x0a601010, 0x0a801010, 0x0aa01010, 0x0ac01010, 0x0ae01010, 0x0b001010, 0x0b201010, 0x0b401010, 0x0b601010, 0x0b801010, 0x0ba01010, 0x0bc01010, 0x0be01010,
		0x0c001010, 0x0c201010, 0x0c401010, 0x0c601010, 0x0c801010, 0x0ca01010, 0x0cc01010, 0x0ce01010, 0x0d001010, 0x0d201010, 0x0d401010, 0x0d601010, 0x0d801010, 0x0da01010, 0x0dc01010, 0x0de01010,
		0x0e001010, 0x0e201010, 0x0e401010, 0x0e601010, 0x0e801010, 0x0ea01010, 0x0ec01010, 0x0ee01010, 0x0f001010, 0x0f201010, 0x0f401010, 0x0f601010, 0x0f801010, 0x0fa01010, 0x0fc01010, 0x0fe01010,
	};
	static unsigned int mixer_gamma2[] = {
#if 0
		0x00001010, 0x00201010, 0x00401010, 0x00601010, 0x00801010, 0x00a01010, 0x00c01010, 0x00e01010, 0x01001010, 0x01201010, 0x01401010, 0x01601010, 0x01801010, 0x01a01010, 0x01c01010, 0x01e01010,
		0x02001010, 0x02201010, 0x02401010, 0x02601010, 0x02801010, 0x02a01010, 0x02c01010, 0x02e01010, 0x03001010, 0x03201010, 0x03401010, 0x03601010, 0x03801010, 0x03a01010, 0x03c01010, 0x03e01010,
		0x04001010, 0x04201010, 0x04401010, 0x04601010, 0x04801010, 0x04a01010, 0x04c01010, 0x04e01010, 0x05001010, 0x05201010, 0x05401010, 0x05601010, 0x05801010, 0x05a01010, 0x05c01010, 0x05e01010,
		0x06001010, 0x06201010, 0x06401010, 0x06601010, 0x06801010, 0x06a01010, 0x06c01010, 0x06e01010, 0x07001010, 0x07201010, 0x07401010, 0x07601010, 0x07801010, 0x07a01010, 0x07c01010, 0x07e01010,
		0x08001010, 0x08201010, 0x08401010, 0x08601010, 0x08801010, 0x08a01010, 0x08c01010, 0x08e01010, 0x09001010, 0x09201010, 0x09401010, 0x09601010, 0x09801010, 0x09a01010, 0x09c01010, 0x09e01010,
		0x0a001010, 0x0a201010, 0x0a401010, 0x0a601010, 0x0a801010, 0x0aa01010, 0x0ac01010, 0x0ae01010, 0x0b001010, 0x0b201010, 0x0b401010, 0x0b601010, 0x0b801010, 0x0ba01010, 0x0bc01010, 0x0be01010,
		0x0c001010, 0x0c201010, 0x0c401010, 0x0c601010, 0x0c801010, 0x0ca01010, 0x0cc01010, 0x0ce01010, 0x0d001010, 0x0d201010, 0x0d401010, 0x0d601010, 0x0d801010, 0x0da01010, 0x0dc01010, 0x0de01010,
		0x0e001010, 0x0e201010, 0x0e401010, 0x0e601010, 0x0e801010, 0x0ea01010, 0x0ec01010, 0x0ee01010, 0x0f001010, 0x0f201010, 0x0f401010, 0x0f601010, 0x0f801010, 0x0fa01010, 0x0fc01010, 0x0fe01010,
#else
		0x00000505, 0x000a0506, 0x00150506, 0x00200606, 0x002c0607, 0x00390607, 0x00460708, 0x00550707, 0x00630808, 0x00730808, 0x00830809, 0x00940909, 0x00a60909, 0x00b8090a, 0x00cb0a0a, 0x00df0a0a,
		0x00f30b0b, 0x01090a0b, 0x011e0b0c, 0x01350b0c, 0x014c0c0c, 0x01640c0d, 0x017d0c0d, 0x01960d0d, 0x01b00d0e, 0x01cb0d0e, 0x01e60e0f, 0x02030e0e, 0x021f0f0f, 0x023d0f0f, 0x025b0f10, 0x027a1010,
		0x029a1010, 0x02ba1011, 0x02db1111, 0x02fd1111, 0x031f1212, 0x03431112, 0x03661213, 0x038b1213, 0x03b01313, 0x03d61314, 0x03fd1314, 0x04241414, 0x044c1415, 0x04751415, 0x049e1516, 0x04c91515,
		0x04f31616, 0x051f1616, 0x054b1617, 0x05781717, 0x05a61717, 0x05d41718, 0x06031818, 0x06331818, 0x06631919, 0x06951819, 0x06c6191a, 0x06f9191a, 0x072c1a1a, 0x07601a1b, 0x07951a1b, 0x07ca1b1b,
		0x08001b1b, 0x08361b1a, 0x086b1b1a, 0x08a01a1a, 0x08d41a1a, 0x09081919, 0x093a1919, 0x096c1819, 0x099d1819, 0x09ce1718, 0x09fd1817, 0x0a2c1717, 0x0a5a1717, 0x0a881716, 0x0ab51617, 0x0ae21516,
		0x0b0d1516, 0x0b381515, 0x0b621515, 0x0b8c1414, 0x0bb41414, 0x0bdc1413, 0x0c031413, 0x0c2a1313, 0x0c501313, 0x0c761212, 0x0c9a1212, 0x0cbe1112, 0x0ce11112, 0x0d041011, 0x0d251110, 0x0d461010,
		0x0d661010, 0x0d86100f, 0x0da50f10, 0x0dc40e0f, 0x0de10e0f, 0x0dfe0e0e, 0x0e1a0e0e, 0x0e360d0d, 0x0e500d0d, 0x0e6a0d0c, 0x0e830d0c, 0x0e9c0c0c, 0x0eb40c0c, 0x0ecc0b0b, 0x0ee20b0b, 0x0ef80a0b,
		0x0f0d0a0b, 0x0f22090a, 0x0f350a09, 0x0f480909, 0x0f5a0909, 0x0f6c0908, 0x0f7d0809, 0x0f8e0708, 0x0f9d0708, 0x0fac0707, 0x0fba0707, 0x0fc80606, 0x0fd40606, 0x0fe00605, 0x0feb0605, 0x0ff60504,
#endif
	};

	drvif_color_colorwrite_mixer_gamma1_Enable(0);
	drvif_color_colorwrite_mixer_gamma1_Enable(0);
	drvif_color_set_mixer_osd3x3_Enable(0);

	//	drvif_color_set_mixer_osd3x3(mixer_3x3);
	IoReg_Mask32(OSDOVL_Mixer_OSD_GAMMA_CTRL_reg,~OSDOVL_Mixer_OSD_GAMMA_CTRL_osd_gamma_position_mask,OSDOVL_Mixer_OSD_GAMMA_CTRL_osd_gamma_position(5));

	drvif_color_colorwrite_mixer_gamma1(mixer_gamma1, GAMMA_CHANNEL_R);
	drvif_color_colorwrite_mixer_gamma1(mixer_gamma1, GAMMA_CHANNEL_G);
	drvif_color_colorwrite_mixer_gamma1(mixer_gamma1, GAMMA_CHANNEL_B);

	drvif_color_colorwrite_mixer_gamma2(mixer_gamma2, GAMMA_CHANNEL_R);
	drvif_color_colorwrite_mixer_gamma2(mixer_gamma2, GAMMA_CHANNEL_G);
	drvif_color_colorwrite_mixer_gamma2(mixer_gamma2, GAMMA_CHANNEL_B);
}

void fwif_color_set_OSD_enhance_enable(unsigned char Enable)
{
//	drvif_color_set_mixer_osd3x3(mixer_3x3[(Enable-1)]);
	if(Enable>0 && Enable<3){
		drvif_color_set_mixer_osd3x3(mixer_3x3[(Enable-1)]);
		Enable = 1;
	}
	drvif_color_set_mixer_osd3x3_Enable(Enable);

	drvif_color_colorwrite_mixer_gamma1_Enable(Enable);
	drvif_color_colorwrite_mixer_gamma2_Enable(Enable);
}
#endif //CONFIG_HW_SUPPORT_MIXER_OSD_COLOR_ENHANCE

static struct task_struct *p_thread_set_od_bits = NULL;
static bool run_thread_set_od_bits_flag = false;
static unsigned char g_set_od_bits = 4;

static int thread_set_od_bits(void *p)
{
#ifndef BUILD_QUICK_SHOW

	unsigned char bits;

	do {
		bits = g_set_od_bits;
		drvif_color_set_od_bits(bits, 2); //10bit color depth
	} while (!kthread_should_stop() && run_thread_set_od_bits_flag && bits != g_set_od_bits);

	run_thread_set_od_bits_flag = false;
#endif
	return 0;
}

unsigned char fwif_color_set_od_bits(unsigned char bits)
{
#ifndef BUILD_QUICK_SHOW

	unsigned short dispH;
	unsigned short dispV;
	unsigned int od_mem_size_pre_allocated = 0;
	unsigned long od_addr = 0;
	unsigned char od_mem_max_bit = 0;

	if (!bODInited)
		return FALSE;

	dispV = Get_DISP_ACT_END_VPOS()-Get_DISP_ACT_STA_VPOS();//VIP_system_info_structure_table->D_Height;
	dispH = Get_DISP_ACT_END_HPOS()-Get_DISP_ACT_STA_HPOS();//VIP_system_info_structure_table->D_Width;

#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
	od_mem_size_pre_allocated = carvedout_buf_query_secure(CARVEDOUT_SCALER_OD, (void *)&od_addr);
#else
	od_mem_size_pre_allocated = carvedout_buf_query(CARVEDOUT_SCALER_OD, (void *)&od_addr);
#endif

	od_mem_max_bit = drvif_color_od_calc_max_bit(dispH, dispV, drvif_color_get_od_dma_mode(), od_mem_size_pre_allocated);

	if (bits > od_mem_max_bit) {
		bits = od_mem_max_bit;
		//return FALSE;
	}

	/* Use thread to prevent too much delay */
	g_set_od_bits = bits;
	if (!run_thread_set_od_bits_flag) {
		p_thread_set_od_bits = kthread_create(thread_set_od_bits, NULL, "thread_set_od_bits");

		if (IS_ERR(p_thread_set_od_bits)) {
			rtd_pr_vpq_err("Unable to creat thread_set_od_bits (err_id = %ld).\n", PTR_ERR(p_thread_set_od_bits));
			drvif_color_set_od_bits(bits, 2); //10bit color depth
		} else {
			run_thread_set_od_bits_flag = true;
			wake_up_process(p_thread_set_od_bits);
		}
	}
	//drvif_color_set_od_bits(bits, 2); //10bit color depth

	return TRUE;
#else
	return 0;
#endif
}

unsigned char fwif_color_set_od_dma_enable(unsigned char enable)
{
	return drvif_color_set_od_dma_enable(enable);
}

unsigned char fwif_color_get_od_bits(void)
{
	return drvif_color_get_od_bits();
}

unsigned char fwif_color_get_od_en(void)
{
	return drvif_color_get_od_en();
}

unsigned char fwif_color_od_test(unsigned char en)
{
#ifndef BUILD_QUICK_SHOW

	extern unsigned char bODInited;
	extern unsigned char bODTableLoaded;
	extern unsigned char bODPreEnable;
	extern char od_table_mode_store;
	extern unsigned int od_table_store[OD_table_length];
	extern unsigned int od_delta_table[OD_table_length];
	extern unsigned char od_table_tv006_store[OD_table_length];
	extern unsigned char od_table_test[OD_table_length];

	if (en) {
		if (!bODInited) {
			if (!Scaler_Init_OD())
				return FALSE;
		}

		memcpy(od_table_tv006_store, od_table_test, sizeof(od_table_tv006_store));
		fwif_color_od_table_tv006_convert(od_table_tv006_store, od_table_store);
		fwif_color_od_table_tv006_transform(0, od_table_store, od_delta_table);
		drvif_color_od_table(od_delta_table, 0);// 0:delta mode 1:target mode(output mode)
		od_table_mode_store = 1;// 0:target mode 1:delta mode -1:inverse mode
		bODTableLoaded = TRUE;
		bODPreEnable = TRUE;
		fwif_color_set_od(TRUE);
	} else {
		fwif_color_set_od(FALSE);
	}

	return TRUE;
#else

return 0;
#endif
}

unsigned char fwif_color_set_liner_od_table(void)
{
	extern unsigned char bODTableLoaded;
	extern char od_table_mode_store;
	extern unsigned int od_table_store[OD_table_length];
	extern unsigned int od_delta_table[OD_table_length];
	extern unsigned char od_table_tv006_store[OD_table_length];
	unsigned char od_table_liner[17][17];
	int i, j;

	for (i = 0; i < 17; i++)
		for (j = 0; j < 17; j++)
			od_table_liner[i][j] = j << 4;

	for (i = 0; i < 17; i++)
		od_table_liner[i][16] -= 1;

	memcpy(od_table_tv006_store, od_table_liner, sizeof(od_table_tv006_store));
	fwif_color_od_table_tv006_convert(od_table_tv006_store, od_table_store);
	fwif_color_od_table_tv006_transform(0, od_table_store, od_delta_table);
	drvif_color_od_table(od_delta_table, 0);// 0:delta mode 1:target mode(output mode)
	od_table_mode_store = 1;// 0:target mode 1:delta mode -1:inverse mode
	bODTableLoaded = TRUE;

	return TRUE;
}


/*
char check_at_uzudtg_disp_area(void)
{
	UINT32 	den_start=0;
	UINT32 	den_end=0;
	UINT32	vtotal=0;
	INT32	line_cnt=0;
	INT32 	dtgm2uzu_sft=0;
	INT32	line_cnt_sft=0;

	ppoverlay_dv_den_start_end_RBUS dv_den_start_end_reg;
	dv_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg);
	den_start = dv_den_start_end_reg.dv_den_sta;
	den_end = dv_den_start_end_reg.dv_den_end;

	ppoverlay_new_meas2_linecnt_real_RBUS new_meas2_linecnt_real_reg;
	new_meas2_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
	line_cnt = new_meas2_linecnt_real_reg.uzudtg_dly_line_cnt_rt;

	ppoverlay_uzudtg_control1_RBUS ppoverlay_uzudtg_control1_reg;
	ppoverlay_uzudtg_control1_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_control1_reg);
	dtgm2uzu_sft = ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_line;

	ppoverlay_dv_total_RBUS dv_total_reg;
	dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
	vtotal = dv_total_reg.dv_total;

	if((line_cnt - dtgm2uzu_sft) < 0) {
		line_cnt_sft = line_cnt - dtgm2uzu_sft + vtotal;
	} else {
		line_cnt_sft = line_cnt - dtgm2uzu_sft;
	}

	if((line_cnt_sft >= (den_start + 5)) && (line_cnt_sft < (den_end - 5))) {
		return 1;	// at disp area
	} else {
		return 0;	// fail, not at disp area
	}
}
*/

char check_at_uzudtg_front_porch(unsigned char waitFixedLine, unsigned int FixedLine)
{
	UINT32 	den_end=0;
	UINT32	vtotal=0;
	INT32	line_cnt=0;
	INT32 	dtgm2uzu_sft=0;
	INT32	line_cnt_sft=0;

	ppoverlay_dv_den_start_end_RBUS dv_den_start_end_reg;
	ppoverlay_new_meas2_linecnt_real_RBUS new_meas2_linecnt_real_reg;
	ppoverlay_uzudtg_control1_RBUS ppoverlay_uzudtg_control1_reg;
	ppoverlay_dv_total_RBUS dv_total_reg;
	dv_den_start_end_reg.regValue = IoReg_Read32(PPOVERLAY_DV_DEN_Start_End_reg);
	den_end = dv_den_start_end_reg.dv_den_end;

	new_meas2_linecnt_real_reg.regValue = IoReg_Read32(PPOVERLAY_new_meas2_linecnt_real_reg);
	line_cnt = new_meas2_linecnt_real_reg.uzudtg_dly_line_cnt_rt;

	ppoverlay_uzudtg_control1_reg.regValue = IoReg_Read32(PPOVERLAY_uzudtg_control1_reg);
	dtgm2uzu_sft = ppoverlay_uzudtg_control1_reg.dtgm2uzuvs_line;

	dv_total_reg.regValue = IoReg_Read32(PPOVERLAY_DV_total_reg);
	vtotal = dv_total_reg.dv_total;

	if((line_cnt - dtgm2uzu_sft) < 0) {
		line_cnt_sft = line_cnt - dtgm2uzu_sft + vtotal;
	} else {
		line_cnt_sft = line_cnt - dtgm2uzu_sft;
	}

	if (!waitFixedLine) {
		if((line_cnt_sft >= (den_end + 5)) && (line_cnt_sft < (vtotal - 2))) {
			return 1;	// at front porch
		} else {
			return 0;	// fail, not at front porch
		}
	} else {
		if (line_cnt_sft == FixedLine)
			return 1;
		else
			return 0;
	}
}

unsigned int rand32(void)
{
#ifndef BUILD_QUICK_SHOW

	unsigned int rand;
	get_random_bytes(&rand, sizeof(unsigned int));
	return rand;
#else

	return 0;
#endif
	
}

unsigned char fwif_Set_VIP_Disable_PQ(VPQ_ModuleTest_Item ModuleTest_Item, unsigned char Enable)
{
	unsigned char i;
	unsigned char isIMode_Flag = 0;
	isIMode_Flag = Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE);

	switch (ModuleTest_Item) {
		case Disable_init:
			fwif_color_SetAutoMAFlag(0);
		break;
		case Disable_HDR:
			drvif_color_Disable_HDR(Enable);
		break;
		case Disable_VPQ_ByPass:
			for (i=0; i<VIP_PQ_ByPass_ITEMS_Max; i++) {
				if( (i!=PQ_ByPass_I_DI_IP_Enable) ){
					if( (isIMode_Flag) && (i!=PQ_ByPass_I_DI) ){
						drvif_color_set_PQ_ByPass(i);
					}else{
						if( i!=PQ_ByPass_I_DI ){
							drvif_color_set_PQ_ByPass(i);
						}
					}
				}
			}
			drvif_color_VPQ_ByPass(Enable);
		break;
		case Disable_MEMC:
#ifdef CONFIG_HW_SUPPORT_MEMC
			drvif_color_Disable_MEMC(Enable);
#endif
		break;

		default:
		break;

	}

	return TRUE;
}

 /* ====  Dither ======*/
 void fwif_color_dither_mode(KADP_DISP_TCON_DITHER_T mode)
{
	switch (mode) {
	case KADP_DISP_TCON_DITHER_TRUNC:
			drvif_color_Panel_Dither_mode(0);
			break;
	case KADP_DISP_TCON_DITHER_ROUND:
	case KADP_DISP_TCON_DITHER_RANDOM:
			drvif_color_Panel_Dither_mode(1);
			break;
	}
}
 void fwif_color_set_Main_Dither(unsigned char enable,unsigned char nTableType)
{
	drvif_color_set_Dither_Sequence(VIP_DITHER_IP_MAIN,tDITHER_SEQUENCE[VIP_DITHER_SEQUENCE_MAIN]);
	drvif_color_set_MainType_Dither_Table(VIP_DITHER_IP_MAIN,tDITHER_MAIN_LUT[nTableType]);
	drvif_color_set_Dither_Temporal(VIP_DITHER_IP_MAIN,tDITHER_TEMPORAL[VIP_DITHER_TEMPORAL_MAIN_GENERAL]);
	drvif_color_set_Main_Dither_Enable(enable);
}
 void fwif_color_set_HDMI_Dither(unsigned char enable,unsigned char nTableType)
{
	drvif_color_set_Dither_Sequence(VIP_DITHER_IP_HDMI,tDITHER_SEQUENCE[VIP_DITHER_SEQUENCE_MAIN]);
	drvif_color_set_MainType_Dither_Table(VIP_DITHER_IP_HDMI,tDITHER_MAIN_LUT[nTableType]);
	drvif_color_set_Dither_Temporal(VIP_DITHER_IP_HDMI,tDITHER_TEMPORAL[VIP_DITHER_TEMPORAL_MAIN_GENERAL]);
	drvif_color_set_HDMI_Dither_Enable(enable);
}
void fwif_color_set_HSD_Dither(unsigned char enable)
{
	drvif_color_set_Dither_Temporal(VIP_DITHER_IP_HSD,tDITHER_TEMPORAL[VIP_DITHER_TEMPORAL_MAIN_GENERAL]);
	drvif_color_set_HSD_Dither_Enable(enable);
}
#ifdef CONFIG_HW_SUPPORT_MEMC
void fwif_color_set_MEMC_dither(unsigned char enable,VIP_PANEL_BIT default_vip_panel_bit,unsigned char nTableType)
{
		drvif_color_Setditherbit(default_vip_panel_bit);
		drvif_color_set_Dither_Sequence(VIP_DITHER_IP_MEMC_MUX,tDITHER_SEQUENCE[VIP_DITHER_SEQUENCE_PANEL]);
	        drvif_color_set_PanelType_Dither_Table(VIP_DITHER_IP_MEMC_MUX,tDITHER_PANEL_LUT[nTableType]);
		drvif_color_set_Dither_Temporal(VIP_DITHER_IP_MEMC_MUX,tDITHER_TEMPORAL[VIP_DITHER_TEMPORAL_MEMC]);
		drvif_memc_mux_Dither_ctrl(enable, enable);
}
#endif //CONFIG_HW_SUPPORT_MEMC
void fwif_color_set_Panel_Dither(unsigned char enable,VIP_PANEL_BIT default_vip_panel_bit,unsigned char nTableType)
{
	drvif_color_Setditherbit(default_vip_panel_bit);
	drvif_color_set_Dither_Sequence(VIP_DITHER_IP_PANEL,tDITHER_SEQUENCE[VIP_DITHER_SEQUENCE_PANEL]);
	drvif_color_set_PanelType_Dither_Table(VIP_DITHER_IP_PANEL,tDITHER_PANEL_LUT[nTableType]);
	drvif_color_set_Dither_Temporal(VIP_DITHER_IP_PANEL,tDITHER_TEMPORAL[VIP_DITHER_TEMPORAL_MAIN_GENERAL]);
	drvif_color_set_Panel_Dither_Enable(enable);
}
void fwif_color_set_Dynamic_Dither(unsigned char enable, unsigned char nTableType)
{
	drvif_color_set_Dynamic_Dither(enable,tDDither_Coef[nTableType]);
}
 void fwif_color_set_PCID_Dither(unsigned char enable,unsigned char nTableType)
{
	drvif_color_set_Dither_Sequence(VIP_DITHER_IP_PCID,tDITHER_SEQUENCE[VIP_DITHER_SEQUENCE_MAIN]);
	drvif_color_set_MainType_Dither_Table(VIP_DITHER_IP_PCID,tDITHER_MAIN_LUT[nTableType]);
	drvif_color_set_Dither_Temporal(VIP_DITHER_IP_PCID,tDITHER_TEMPORAL[VIP_DITHER_TEMPORAL_MAIN_GENERAL]);
	drvif_color_set_PCID_Dither_Enable(enable);
}
 void fwif_color_set_LGD_Dither(unsigned char enable,unsigned char nTableType)
{
	drvif_color_set_Dither_Sequence(VIP_DITHER_IP_LGD,tDITHER_SEQUENCE[VIP_DITHER_SEQUENCE_MAIN]);
	drvif_color_set_MainType_Dither_Table(VIP_DITHER_IP_LGD,tDITHER_MAIN_LUT[nTableType]);
	drvif_color_set_Dither_Temporal(VIP_DITHER_IP_LGD,tDITHER_TEMPORAL[VIP_DITHER_TEMPORAL_MAIN_GENERAL]);
	drvif_color_set_LGD_Dither_Enable(enable);
}
 /* ==================*/

/*sync TV030*/
void fwif_color_set_pcid2_valuetable_bycolor(unsigned char* InputLUT, DRV_pcid2_channel_t Channel)
{
    // Transfer the value to register value
    bool isSameTbl_1 = true, isSameTbl_2 = true;
    int Row = 0, Col = 0;
    unsigned int TblValue = 0;
    unsigned int POD_ValueTBL_1[81] = {0};
    unsigned int POD_ValueTBL_2[81] = {0};
    for(Row=0; Row<9; Row++)
    {
        for(Col=0; Col<9; Col++)
        {
            //-------------------
            // Value1 | Value3 |
            //-------------------     will be transform to {Value1, Value2, Value3, Value4} {MSB, LSB}
            // Value2 | Value4 |
            //-------------------
            unsigned int Value1 = 0, Value2 = 0, Value3 = 0, Value4 = 0;
            if( Row<8 && Col<8 )
            {
                Value1 = InputLUT[Row*34   +Col*2  ];
                Value2 = InputLUT[Row*34+17+Col*2  ];
                Value3 = InputLUT[Row*34   +Col*2+1];
                Value4 = InputLUT[Row*34+17+Col*2+1];
            }
            else if( Row < 8 )
            {
                Value1 = InputLUT[Row*34   +Col*2  ];
                Value2 = InputLUT[Row*34+17+Col*2  ];
                Value3 = Value1;
                Value4 = Value2;
            }
            else if( Col < 8 )
            {
                Value1 = InputLUT[Row*34   +Col*2  ];
                Value2 = Value1;
                Value3 = InputLUT[Row*34   +Col*2+1];
                Value4 = Value3;
            }
            else
            {
                Value1 = InputLUT[Row*34   +Col*2  ];
                Value2 = Value1;
                Value3 = Value1;
                Value4 = Value1;
            }
            TblValue = ( (Value1 << 24) + (Value2 << 16) + (Value3 << 8) + Value4 );
			if( POD_ValueTBL_1[Row*9+Col] != TblValue )
			{
				POD_ValueTBL_1[Row*9+Col] = TblValue;
				isSameTbl_1 = false;
			}
        }
    }

    for(Row=0; Row<9; Row++)
    {
        for(Col=0; Col<9; Col++)
        {
            //-------------------
            // Value1 | Value3 |
            //-------------------     will be transform to {Value1, Value2, Value3, Value4} {MSB, LSB}
            // Value2 | Value4 |
            //-------------------
            unsigned int Value1 = 0, Value2 = 0, Value3 = 0, Value4 = 0;
            if( Row<8 && Col<8 )
            {
                Value1 = InputLUT[289+Row*34   +Col*2  ];
                Value2 = InputLUT[289+Row*34+17+Col*2  ];
                Value3 = InputLUT[289+Row*34   +Col*2+1];
                Value4 = InputLUT[289+Row*34+17+Col*2+1];
            }
            else if( Row < 8 )
            {
                Value1 = InputLUT[289+Row*34   +Col*2  ];
                Value2 = InputLUT[289+Row*34+17+Col*2  ];
                Value3 = Value1;
                Value4 = Value2;
            }
            else if( Col < 8 )
            {
                Value1 = InputLUT[289+Row*34   +Col*2  ];
                Value2 = Value1;
                Value3 = InputLUT[289+Row*34   +Col*2+1];
                Value4 = Value3;
            }
            else
            {
                Value1 = InputLUT[289+Row*34   +Col*2  ];
                Value2 = Value1;
                Value3 = Value1;
                Value4 = Value1;
            }
            TblValue = ( (Value1 << 24) + (Value2 << 16) + (Value3 << 8) + Value4 );
			if( POD_ValueTBL_2[Row*9+Col] != TblValue )
			{
				POD_ValueTBL_2[Row*9+Col] = TblValue;
				isSameTbl_2 = false;
			}
        }
    }

#if 0
	if( isSameTbl_1 == false || isSameTbl_2 == false )
	{
		fwif_color_WaitFor_DEN_STOP_MEMCDTG();//20170927

		if( isSameTbl_1 == false ){
			//fwif_color_WaitFor_SYNC_START_MEMCDTG();//20170927
			drvif_color_pcid2_valuetable(POD_ValueTBL_1, 0, Channel);
			}
		if( isSameTbl_2 == false ){
			//fwif_color_WaitFor_SYNC_START_MEMCDTG();//20170927
			drvif_color_pcid2_valuetable(POD_ValueTBL_2, 1, Channel);
			}
	}
#else
	fwif_color_WaitFor_DEN_STOP_MEMCDTG();//20170927
	drvif_color_pcid2_valuetable(POD_ValueTBL_1, 0, Channel);
	drvif_color_pcid2_valuetable(POD_ValueTBL_2, 1, Channel);
#endif

}

/*sync TV030 end*/

void fwif_color_set_POD_DATA_table(DRV_POD_DATA_table_t* POD_DATA_table)
{
	unsigned char InputLUT[578] = {0};
	drvif_color_pcid2_pixel_setting(&(POD_DATA_table->pcid2_data));
	drvif_color_pcid2_poltable(&(POD_DATA_table->pPcid2PolTable[_PCID2_COLOR_R][0]),_PCID2_COLOR_R);
	drvif_color_pcid2_poltable(&(POD_DATA_table->pPcid2PolTable[_PCID2_COLOR_G][0]),_PCID2_COLOR_G);
	drvif_color_pcid2_poltable(&(POD_DATA_table->pPcid2PolTable[_PCID2_COLOR_B][0]),_PCID2_COLOR_B);
	drvif_color_pcid_VALC_protection(&(POD_DATA_table->pcid_valc));
	//R
	memcpy(&(InputLUT[0]), &(POD_DATA_table->POD_DATA[_PCID2_COLOR_R*2][0]), sizeof(unsigned char)*289);
	memcpy(&(InputLUT[289]), &(POD_DATA_table->POD_DATA[_PCID2_COLOR_R*2+1][0]), sizeof(unsigned char)*289);
	fwif_color_set_pcid2_valuetable_channel(InputLUT,_PCID2_COLOR_R);
	//G
	memcpy(&(InputLUT[0]), &(POD_DATA_table->POD_DATA[_PCID2_COLOR_G*2][0]), sizeof(unsigned char)*289);
	memcpy(&(InputLUT[289]), &(POD_DATA_table->POD_DATA[_PCID2_COLOR_G*2+1][0]), sizeof(unsigned char)*289);
	fwif_color_set_pcid2_valuetable_channel(InputLUT,_PCID2_COLOR_G);
	//B
	memcpy(&(InputLUT[0]), &(POD_DATA_table->POD_DATA[_PCID2_COLOR_B*2][0]), sizeof(unsigned char)*289);
	memcpy(&(InputLUT[289]), &(POD_DATA_table->POD_DATA[_PCID2_COLOR_B*2+1][0]), sizeof(unsigned char)*289);
	fwif_color_set_pcid2_valuetable_channel(InputLUT,_PCID2_COLOR_B);
}

char fwif_color_get_Peaking_Data_Access(unsigned int *index, unsigned int *YUV)
{
	return drvif_color_get_Peaking_Data_Access(index, YUV);
}

char fwif_color_get_UZD_Data_Access(unsigned int *index, unsigned int *YUV)
{
	return drvif_color_get_UZD_Data_Access(index, YUV);
}

char fwif_color_get_UZU_Data_Access(unsigned int *index, unsigned int *YUV)
{
	return drvif_color_get_UZU_Data_Access(index, YUV);
}

char fwif_color_get_ICM_Data_Access(unsigned int *index, unsigned int *YUV)
{
	return drvif_color_get_ICM_Data_Access(index, YUV);
}

char fwif_color_get_YUV2RGB_Data_Access(unsigned int *index, unsigned int *YUV)
{
	return drvif_color_get_YUV2RGB_Data_Access(index, YUV);
}

char fwif_color_get_Panel_Compensation_Data_Access(unsigned int *index, unsigned int *YUV)
{
	return drvif_color_get_Panel_Compensation_Data_Access(index, YUV);
}

char fwif_color_set_PQ_SOURCE_TYPE_From_AP(unsigned char pq_src)
{
	_RPC_system_setting_info *sys_RPC_info =(_RPC_system_setting_info *) Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if (sys_RPC_info == NULL) {
		rtd_pr_vpq_emerg("fwif_color_set_PQ_SOURCE_TYPE_From_AP, table NULL.\n");
		return -1;
	}

	if (pq_src >= PQ_SRC_From_AP_ITEM_MAX)
		pq_src = PQ_SRC_From_AP_UNKNOW;

	sys_RPC_info->PQ_source_type_from_AP = pq_src;
	rtd_pr_vpq_info("fwif_color_set_PQ_SOURCE_TYPE_From_AP, pq_src = %d,\n", pq_src);
	return 0;
}

unsigned char fwif_color_Get_PQ_SOURCE_TYPE_From_AP(void)
{
	_RPC_system_setting_info *sys_RPC_info =(_RPC_system_setting_info *) Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if (sys_RPC_info == NULL) {
		rtd_pr_vpq_emerg("fwif_color_Get_PQ_SOURCE_TYPE_From_AP, table NULL.\n");
		return PQ_SRC_From_AP_UNKNOW;
	}

	return sys_RPC_info->PQ_source_type_from_AP;
}

void fwif_color_IESM_format_convert(void)
{
	drvif_IESM_format_convert();
}

void fwif_color_get_LC_Blight_dat(unsigned int *ptr_out, unsigned int bufferSz)
{
	drvif_color_get_LC_Blight_dat(ptr_out, bufferSz);
}

void fwif_color_get_LC_Hist_dat(unsigned int *plc_out)
{
	 drvif_color_get_LC_Histogram(plc_out);
}


//TV030
unsigned char flag_of_setting_gammacurve_index_without_GammaMode = 0;
unsigned char gammalevel = 0;
int MagicGammaR_20Pts[20]={0};
int MagicGammaG_20Pts[20]={0};
int MagicGammaB_20Pts[20]={0};
UINT16 rtice_GOut_R[Bin_Num_Gamma], rtice_GOut_G[Bin_Num_Gamma], rtice_GOut_B[Bin_Num_Gamma];

void fwif_color_set_gamma_curve_index(unsigned char gamma_level, unsigned int **In_R, unsigned int **In_G, unsigned int **In_B)
{
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	unsigned char l_gamma_level;
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	l_gamma_level = gamma_level;
	rtd_pr_vpq_debug("Scaler_color_get_gamma_default, l_gamma_level = = %d\n",l_gamma_level);
	switch (l_gamma_level) {
	case GAMMA_CURVE_0:
			*In_R = gVip_Table->tGAMMA[0].R;
			*In_G = gVip_Table->tGAMMA[0].G;
			*In_B = gVip_Table->tGAMMA[0].B;
			break;
	case GAMMA_CURVE_1:
			*In_R = gVip_Table->tGAMMA[1].R;
			*In_G = gVip_Table->tGAMMA[1].G;
			*In_B = gVip_Table->tGAMMA[1].B;
			break;
	case GAMMA_CURVE_2:
			*In_R = gVip_Table->tGAMMA[2].R;
			*In_G = gVip_Table->tGAMMA[2].G;
			*In_B = gVip_Table->tGAMMA[2].B;
			break;
	case GAMMA_CURVE_3:
			*In_R = gVip_Table->tGAMMA[3].R;
			*In_G = gVip_Table->tGAMMA[3].G;
			*In_B = gVip_Table->tGAMMA[3].B;
			break;
	case GAMMA_CURVE_4:
			*In_R = gVip_Table->tGAMMA[4].R;
			*In_G = gVip_Table->tGAMMA[4].G;
			*In_B = gVip_Table->tGAMMA[4].B;
			break;
	case GAMMA_CURVE_5:
			*In_R = gVip_Table->tGAMMA[5].R;
			*In_G = gVip_Table->tGAMMA[5].G;
			*In_B = gVip_Table->tGAMMA[5].B;
			break;
	case GAMMA_CURVE_6:
			*In_R = gVip_Table->tGAMMA[6].R;
			*In_G = gVip_Table->tGAMMA[6].G;
			*In_B = gVip_Table->tGAMMA[6].B;
			break;
	case GAMMA_CURVE_7:
			*In_R = gVip_Table->tGAMMA[7].R;
			*In_G = gVip_Table->tGAMMA[7].G;
			*In_B = gVip_Table->tGAMMA[7].B;
			break;
	case GAMMA_CURVE_8:
			*In_R = gVip_Table->tGAMMA[8].R;
			*In_G = gVip_Table->tGAMMA[8].G;
			*In_B = gVip_Table->tGAMMA[8].B;
			break;
	case GAMMA_CURVE_9:
			*In_R = gVip_Table->tGAMMA[9].R;
			*In_G = gVip_Table->tGAMMA[9].G;
			*In_B = gVip_Table->tGAMMA[9].B;
			break;
	case GAMMA_CURVE_identity:
	default:
			*In_R = Identity_Gamma;
			*In_G = Identity_Gamma;
			*In_B = Identity_Gamma;
			break;
	}
}

void fwif_color_set_20pointsData(RTK_TV030_MagicGamma *pData)
{
	unsigned char points_num=0;
	unsigned char k;
	points_num =20;
	if(pData->enable==0){
		for(k=0;k<(points_num);k++){
			MagicGammaR_20Pts[k] = 0;
			MagicGammaG_20Pts[k] = 0;
			MagicGammaB_20Pts[k] = 0;
		}
	}
	else{
		for(k=0;k<3;k++)
		{
			MagicGammaR_20Pts[k] = (pData->MagicGammaR[k]-128)*2;
			MagicGammaG_20Pts[k] = (pData->MagicGammaG[k]-128)*2;
			MagicGammaB_20Pts[k] = (pData->MagicGammaB[k]-128)*2;
		}
		for(k=3;k<points_num;k++)
		{
			MagicGammaR_20Pts[k] = (pData->MagicGammaR[k]-128)*4;
			MagicGammaG_20Pts[k] = (pData->MagicGammaG[k]-128)*4;
			MagicGammaB_20Pts[k] = (pData->MagicGammaB[k]-128)*4;
		}

	}

#if 0 //add for debug
	printk("yuan,Magic enable=%d\n",pData->enable);
	for(k=0;k<(points_num);k++){
		printk("MagicGammaR_12Pts[%d]=%d\n",k,MagicGammaR_12Pts[k]);
		printk("MagicGammaG_12Pts[%d]=%d\n",k,MagicGammaG_12Pts[k]);
		printk("MagicGammaB_12Pts[%d]=%d\n",k,MagicGammaB_12Pts[k]);
	}
#endif

}

void fwif_color_set_20pointsGamma(UINT16 *GOut_R, UINT16 *GOut_G, UINT16 *GOut_B)
{
	unsigned char points_num=20;//0 point +12
	unsigned short spec_index[20] = {51 ,102 ,153 ,205 ,256 ,307 ,358 ,409 ,460 ,512 ,563 ,614 ,665 ,716 ,767 ,818 ,870 ,921 ,972 ,1023};
	unsigned int i=0;
	unsigned short inter_index[21] = {0,51 ,102 ,153 ,205 ,256 ,307 ,358 ,409 ,460 ,512 ,563 ,614 ,665 ,716 ,767 ,818 ,870 ,921 ,972 ,1023};

	for(i=0;i<(points_num);i++){
		GOut_R[spec_index[i]] = GOut_R[spec_index[i]]+ MagicGammaR_20Pts[i];
		GOut_G[spec_index[i]] = GOut_G[spec_index[i]]+ MagicGammaG_20Pts[i];
		GOut_B[spec_index[i]] = GOut_B[spec_index[i]]+ MagicGammaB_20Pts[i];
	}

	#if 0 //add for debug
		for(i=0;i<(points_num);i++)
			printk("GOut_R[%d]=%d\n",spec_index[i],GOut_R[spec_index[i]]);
	#endif



	for(i=0;i<(points_num);i++){
		fwif_Slope_Interpolation(GOut_R, inter_index[i], inter_index[i+1], GOut_R[inter_index[i]], GOut_R[inter_index[i+1]]);
		fwif_Slope_Interpolation(GOut_G, inter_index[i], inter_index[i+1], GOut_G[inter_index[i]], GOut_G[inter_index[i+1]]);
		fwif_Slope_Interpolation(GOut_B, inter_index[i], inter_index[i+1], GOut_B[inter_index[i]], GOut_B[inter_index[i+1]]);
	}

	#if 0 //add for debug
		for(i=0;i<(points_num);i++)
			printk("I_GOut_R[%d]=%d\n",inter_index[i],GOut_R[inter_index[i]]);
	#endif

	for(i=0;i<(Bin_Num_Gamma);i++){
	if (GOut_R[i] > 4096)
				GOut_R[i] = 4096;
	if (GOut_G[i] > 4096)
				GOut_G[i] = 4096;
	if (GOut_B[i] > 4096)
				GOut_B[i] = 4096;
}
	memcpy(rtice_GOut_R,GOut_R,(Bin_Num_Gamma)*sizeof(UINT16));
	memcpy(rtice_GOut_G,GOut_G,(Bin_Num_Gamma)*sizeof(UINT16));
	memcpy(rtice_GOut_B,GOut_B,(Bin_Num_Gamma)*sizeof(UINT16));

	//data protect
	GOut_R[0]=0;
	GOut_G[0]=0;
	GOut_B[0]=0;
	GOut_R[Bin_Num_Gamma] = GOut_R[Bin_Num_Gamma - 1];
	GOut_G[Bin_Num_Gamma] = GOut_G[Bin_Num_Gamma - 1];
	GOut_B[Bin_Num_Gamma] = GOut_B[Bin_Num_Gamma - 1];


	#if 0 //for debug
	printk("============================================================\n");
	printk("Magic_GOut_R[48] = %d, rtice_GOut_R[0] = %d, GOut_B[0] = %d\n",GOut_R[48],rtice_GOut_R[48],GOut_B[0]);
	printk("Magic_GOut_R[49] = %d, GOut_G[512] = %d, GOut_B[512] = %d\n",GOut_R[49],rtice_GOut_R[49],GOut_B[512]);
	printk("Magic_GOut_R[50] = %d, GOut_G[1020] = %d, GOut_B[1020] = %d\n",GOut_R[50],rtice_GOut_R[50],GOut_B[1020]);
	printk("Magic_GOut_R[51] = %d, GOut_G[1023] = %d, GOut_B[1023] = %d\n",GOut_R[51],rtice_GOut_R[51],GOut_B[1023]);
	printk("Magic_GOut_R[52] = %d, GOut_G[1024] = %d, GOut_B[1024] = %d\n",GOut_R[52],rtice_GOut_R[52],GOut_B[1024]);
	printk("Magic_GOut_R[53] = %d, GOut_G[1024] = %d, GOut_B[1024] = %d\n",GOut_R[53],rtice_GOut_R[53],GOut_B[1024]);
	#endif


}

void fwif_color_set_GainOffsetGamma(UINT16 *GOut_R, UINT16 *GOut_G, UINT16 *GOut_B)
{
	UINT16 Red_Gain,Green_Gain,Blue_Gain;
	UINT16 Red_Offset,Green_Offset,Blue_Offset;
	unsigned int i=0;
	SINT16 *Gamma_R,*Gamma_B,*Gamma_G;

	con_bri_dm_contrast_a_RBUS		dm_contrast_a_REG;
	con_bri_dm_brightness_1_RBUS		dm_brightness_1_REG;
	dm_brightness_1_REG.regValue	= IoReg_Read32(CON_BRI_DM_Brightness_1_reg);
	dm_contrast_a_REG.regValue	= IoReg_Read32(CON_BRI_DM_CONTRAST_A_reg);

	Red_Gain		=dm_contrast_a_REG.m_con_a_r;
	Green_Gain		=dm_contrast_a_REG.m_con_a_g;
	Blue_Gain		=dm_contrast_a_REG.m_con_a_b	;

	Red_Offset		=dm_brightness_1_REG.m_bri1_r;
	Green_Offset		=dm_brightness_1_REG.m_bri1_g;
	Blue_Offset		=dm_brightness_1_REG.m_bri1_b;

	Gamma_R = (SINT16*)GOut_R;
	Gamma_G = (SINT16*)GOut_G;
	Gamma_B = (SINT16*)GOut_B;

			#if 0 //for debug
			printk("Red_Gain=%d,Green_Gain=%d,Blue_Gain=%d\n",Red_Gain,Green_Gain,Blue_Gain);
			printk("Red_Offset=%d,Green_Offset=%d,Blue_Offset=%d\n",Red_Offset,Green_Offset,Blue_Offset);
			printk("============================================================\n");
			printk("Ori_GOut_R[0] = %d, GOut_G[0] = %d, GOut_B[0] = %d\n",GOut_R[0],GOut_G[0],GOut_B[0]);
			printk("Ori_GOut_R[512] = %d, GOut_G[512] = %d, GOut_B[512] = %d\n",GOut_R[512],GOut_G[512],GOut_B[512]);
			printk("Ori_GOut_R[1020] = %d, GOut_G[1020] = %d, GOut_B[1020] = %d\n",GOut_R[1020],GOut_G[1020],GOut_B[1020]);
			printk("Ori_GOut_R[1023] = %d, GOut_G[1023] = %d, GOut_B[1023] = %d\n",GOut_R[1023],GOut_G[1023],GOut_B[1023]);
			printk("Ori_GOut_R[1024] = %d, GOut_G[1024] = %d, GOut_B[1024] = %d\n",GOut_R[1024],GOut_G[1024],GOut_B[1024]);
			#endif

	for(i=0;i<(Bin_Num_Gamma);i++){

		Gamma_R[i] = (Gamma_R[i]*Red_Gain)>>9;
		Gamma_G[i] = (Gamma_G[i]*Green_Gain)>>9;
		Gamma_B[i] = (Gamma_B[i]*Blue_Gain)>>9;
	}
			#if 0
			printk("============================================================\n");
			printk("GOut_R[0] = %d, Gamma_G[0] = %d, Gamma_B[0] = %d\n",Gamma_R[0],Gamma_G[0],Gamma_B[0]);
			printk("GOut_R[512] = %d, Gamma_G[512] = %d, Gamma_B[512] = %d\n",Gamma_R[512],Gamma_G[512],Gamma_B[512]);
			printk("GOut_R[1020] = %d, Gamma_G[1020] = %d, Gamma_B[1020] = %d\n",Gamma_R[1020],Gamma_G[1020],Gamma_B[1020]);
			printk("GOut_R[1023] = %d, Gamma_G[1023] = %d, Gamma_B[1023] = %d\n",Gamma_R[1023],Gamma_G[1023],Gamma_B[1023]);
			printk("GOut_R[1024] = %d, Gamma_G[1024] = %d, Gamma_B[1024] = %d\n",Gamma_R[1024],Gamma_G[1024],Gamma_B[1024]);
			#endif

	for(i=0;i<(Bin_Num_Gamma);i++){
		Gamma_R[i] = Gamma_R[i]+Red_Offset-512;
		Gamma_G[i] = Gamma_G[i]+Green_Offset-512;
		Gamma_B[i] = Gamma_B[i]+Blue_Offset-512;
	}

for(i=0;i<(Bin_Num_Gamma);i++){
	if (Gamma_R[i] < 0)
				Gamma_R[i] = 0;
	if (Gamma_G[i] < 0)
				Gamma_G[i] = 0;
	if (Gamma_B[i] < 0)
				Gamma_B[i] = 0;
}

for(i=0;i<(Bin_Num_Gamma);i++){
	if (Gamma_R[i] > 4096)
				Gamma_R[i] = 4096;
	if (Gamma_G[i] > 4096)
				Gamma_G[i] = 4096;
	if (Gamma_B[i] > 4096)
				Gamma_B[i] = 4096;
}

	//data protect
	Gamma_R[0]=0;
	Gamma_G[0]=0;
	Gamma_B[0]=0;
	Gamma_R[Bin_Num_Gamma] = Gamma_R[Bin_Num_Gamma - 1];
	Gamma_G[Bin_Num_Gamma] = Gamma_G[Bin_Num_Gamma - 1];
	Gamma_B[Bin_Num_Gamma] = Gamma_B[Bin_Num_Gamma - 1];

			#if 0
			printk("============================================================\n");
			printk("O_GOut_R[0] = %d, Gamma_G[0] = %d, Gamma_B[0] = %d\n",Gamma_R[0],Gamma_G[0],Gamma_B[0]);
			printk("O_GOut_R[512] = %d, Gamma_G[512] = %d, Gamma_B[512] = %d\n",Gamma_R[512],Gamma_G[512],Gamma_B[512]);
			printk("O_GOut_R[1020] = %d, Gamma_G[1020] = %d, Gamma_B[1020] = %d\n",Gamma_R[1020],Gamma_G[1020],Gamma_B[1020]);
			printk("O_GOut_R[1023] = %d, Gamma_G[1023] = %d, Gamma_B[1023] = %d\n",Gamma_R[1023],Gamma_G[1023],Gamma_B[1023]);
			printk("O_GOut_R[1024] = %d, Gamma_G[1024] = %d, Gamma_B[1024] = %d\n",Gamma_R[1024],Gamma_G[1024],Gamma_B[1024]);
			#endif
}

void fwif_color_set_gamma_Magic(void)
{
	static UINT16 HDR_GOut_R[Bin_Num_Gamma + 1], HDR_GOut_G[Bin_Num_Gamma + 1], HDR_GOut_B[Bin_Num_Gamma + 1];
	static UINT16 SDR_GOut_R[Bin_Num_Gamma + 1], SDR_GOut_G[Bin_Num_Gamma + 1], SDR_GOut_B[Bin_Num_Gamma + 1];

	extern unsigned char ColorSpaceMode;
	unsigned char GammaBlendingFlag;

	unsigned char display = 0;
	unsigned char src_idx = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	if((ColorSpaceMode==0)||(ColorSpaceMode==2))
		GammaBlendingFlag =1;
	else
		GammaBlendingFlag =0;

	if(GammaBlendingFlag==1){
		fwif_color_WaitFor_DEN_STOP_UZUDTG();
		drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 1, 0);
	}
	else{
		fwif_color_WaitFor_DEN_STOP_UZUDTG();
		drvif_color_inv_gamma_enable(SLR_MAIN_DISPLAY, 0, 0);
	}

	//printk("yuan,set Gamma Main function\n");
	//printk("yuan,GammaBlendingFlag=%d\n",GammaBlendingFlag);
	//printk("yuan,GammaBlendOETF=%d\n",GammaBlendOETF);

	down(&Gamma_Semaphore);
	/*0. gamma curve reg control*/
	fwif_color_gamma_control_front(display);

	/*1. choose encode gamma data*/
	fwif_color_set_gamma_curve_index(1, &tGAMMA_temp_R, &tGAMMA_temp_G, &tGAMMA_temp_B);

	/*2. decode gamma data to gamma curve*/
	fwif_color_gamma_decode(GOut_R, GOut_G, GOut_B, tGAMMA_temp_R, tGAMMA_temp_G, tGAMMA_temp_B);
	/*fwif_color_decode_gamma_debug(1);*/	/*debug*/

	/*3. 20points*/
	fwif_color_set_20pointsGamma(GOut_R, GOut_G, GOut_B);

	if(GammaBlendingFlag==1){
		//if(GammaBlendOETF==1){
			fwif_set_gamma_multiply_remapOETF(SDR_GOut_R, SDR_GOut_G, SDR_GOut_B, GOut_R, GOut_G, GOut_B);
			fwif_set_gamma_multiply_remap(HDR_GOut_R, HDR_GOut_G, HDR_GOut_B, SDR_GOut_R, SDR_GOut_G, SDR_GOut_B);
		//}
		//else
		//fwif_set_gamma_multiply_remap(HDR_GOut_R, HDR_GOut_G, HDR_GOut_B, GOut_R, GOut_G, GOut_B);

		/*4. Gain Offset*/
		fwif_color_set_GainOffsetGamma(HDR_GOut_R, HDR_GOut_G, HDR_GOut_B);
		/*5. gamma curve data protect and debug*/
		fwif_color_gamma_curve_data_protect(HDR_GOut_R, HDR_GOut_G, HDR_GOut_B);	/*DO NOT mark this, must check*/
		fwif_color_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B, HDR_GOut_R, HDR_GOut_G, HDR_GOut_B);
	}else{
		fwif_set_gamma_multiply_remapOETF(SDR_GOut_R, SDR_GOut_G, SDR_GOut_B, GOut_R, GOut_G, GOut_B);
		/*4. Gain Offset*/
		//fwif_color_set_GainOffsetGamma(SDR_GOut_R, SDR_GOut_G, SDR_GOut_B);
		/*5. gamma curve data protect and debug*/
		fwif_color_gamma_curve_data_protect(SDR_GOut_R, SDR_GOut_G, SDR_GOut_B);		/*DO NOT mark this, must check*/
		fwif_color_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B, SDR_GOut_R, SDR_GOut_G, SDR_GOut_B);
	}

	/*7. write gamma table*/
	fwif_color_set_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B);

	fwif_color_gamma_control_back(display, 1);
	GammaBlendingFlag =0;
	//GammaBlendOETF =0;

	up(&Gamma_Semaphore);
}

void fwif_color_set_gamma_from_MagicGamma_10points(RTK_TV030_MagicGamma *pData)
{
	unsigned char display = 0;
	unsigned char src_idx = 0;
	unsigned char Gamma_Mode =0;
	unsigned char k=0;
	unsigned int i=0;
	unsigned char points_num=12;
	int MagicGammaR_tmp[12]={0,0,0,0,0,0,0,0,0,0,0,0};
	int MagicGammaG_tmp[12]={0,0,0,0,0,0,0,0,0,0,0,0};
	int MagicGammaB_tmp[12]={0,0,0,0,0,0,0,0,0,0,0,0};
	unsigned short spec_index[12] = {51,102, 153, 204, 306, 408, 510,612, 714, 816, 918, 1024};

	Gamma_Mode =1;
	Scaler_Get_Display_info(&display, &src_idx);

	/*0. gamma curve reg control*/
	fwif_color_gamma_control_front(display);

	/*1. choose encode gamma data*/
	if(flag_of_setting_gammacurve_index_without_GammaMode == 1)
	{
		if(gammalevel > t_GAMMA_MAX)
		{
			rtd_pr_vpq_emerg("fwif_set_gamma, gammalevel > t_GAMMA_MAX\n");
			return;
		}
		fwif_color_set_gamma_curve_index(gammalevel, &tGAMMA_temp_R, &tGAMMA_temp_G, &tGAMMA_temp_B);
	}
	else
	{
		fwif_color_get_gamma_default(Gamma_Mode, &tGAMMA_temp_R, &tGAMMA_temp_G, &tGAMMA_temp_B);
		//fwif_color_encode_gamma_debug(1); //debug
	}

	/*2. decode gamma data to gamma curve*/
	fwif_color_gamma_decode(GOut_R, GOut_G, GOut_B, tGAMMA_temp_R, tGAMMA_temp_G, tGAMMA_temp_B);
	/*fwif_color_decode_gamma_debug(1);*/	/*debug*/

#if 0 //for debug
for (i = 1020; i < 1025; i++) {
		  //printk("Ori_GOut_R[%d] = %d, GOut_G[%d] = %d, GOut_B[%d] = %d\n", i, GOut_R[i], i, GOut_G[i], i, GOut_B[i]);
  }
#endif


	#if 1//tcl magic 10 points
	for(k=0;k<(points_num);k++)
	{
		MagicGammaR_tmp[k] = (pData->MagicGammaR[k]-128)*4;
		MagicGammaG_tmp[k] = (pData->MagicGammaG[k]-128)*4;
		MagicGammaB_tmp[k] = (pData->MagicGammaB[k]-128)*4;
	}

	for(i=0;i<(points_num);i++){

		GOut_R[spec_index[i]] = GOut_R[spec_index[i]]+ MagicGammaR_tmp[i];
		GOut_G[spec_index[i]] = GOut_G[spec_index[i]]+ MagicGammaG_tmp[i];
		GOut_B[spec_index[i]] = GOut_B[spec_index[i]]+ MagicGammaB_tmp[i];
	}
	for(i=0;i<(points_num-1);i++){

		fwif_Slope_Interpolation(GOut_R, spec_index[i], spec_index[i+1], GOut_R[spec_index[i]], GOut_R[spec_index[i+1]]);
		fwif_Slope_Interpolation(GOut_G, spec_index[i], spec_index[i+1], GOut_G[spec_index[i]], GOut_G[spec_index[i+1]]);
		fwif_Slope_Interpolation(GOut_B, spec_index[i], spec_index[i+1], GOut_B[spec_index[i]], GOut_B[spec_index[i+1]]);
	}
	//data protect
	GOut_R[0]=0;
	GOut_G[0]=0;
	GOut_B[0]=0;
	//GOut_R[Bin_Num_Gamma] = GOut_R[Bin_Num_Gamma - 1];
	//GOut_G[Bin_Num_Gamma] = GOut_G[Bin_Num_Gamma - 1];
	//GOut_B[Bin_Num_Gamma] = GOut_B[Bin_Num_Gamma - 1];

#if 0 //for debug
for (i = 900; i < 1025; i++) {
		  printk("GOut_R[%d] = %d, GOut_G[%d] = %d, GOut_B[%d] = %d\n", i, GOut_R[i], i, GOut_G[i], i, GOut_B[i]);
  }
#endif
	#endif


	/*5. gamma curve data protect and debug*/
	fwif_color_gamma_curve_data_protect(GOut_R, GOut_G, GOut_B);		/*DO NOT mark this, must check*/


	/*6. encode gamma cuve to gamma data*/
#ifdef ENABLE_xvYcc
	fwif_color_gamma_remmping_for_xvYcc(src_idx, GOut_R, GOut_G, GOut_B);
#endif
	/*fwif_color_gamma_encode();	*/ /*=== marked by Elsie ===*/
	fwif_color_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B, GOut_R, GOut_G, GOut_B);
	/*fwif_color_encode_gamma_debug(2);	*/ /*debug*/

	/*7. write gamma table*/
	fwif_color_set_gamma_encode(final_GAMMA_R, final_GAMMA_G, final_GAMMA_B);
	fwif_color_gamma_control_back(display, 1);

}

void fwif_set_gamma_multiply_remap(UINT16 *final_R, UINT16 *final_G, UINT16 *final_B, UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B)
{
	int i, j, hdiff, qdiff;
	extern unsigned short g_GammaRemap[4097];
	extern unsigned short Power1div22Gamma[1025];

	if ((NULL == Out_R)||(NULL == Out_G)||(NULL == Out_B))
		return;

	// R
	for (i = 0, j = 0; i < 4096; i+=4, j++) {
		g_GammaRemap[i] = Out_R[j];// << 2;
	}
	g_GammaRemap[4096] = (Out_R[1023] ) + (Out_R[1023] >> 8);

	for (i = 0; i < 4096; i+=4) {
		hdiff = (g_GammaRemap[i+4]-g_GammaRemap[i])>>1;
		qdiff = hdiff>>1;
		g_GammaRemap[i+1] = g_GammaRemap[i]+qdiff;
		g_GammaRemap[i+2] = g_GammaRemap[i]+hdiff;
		g_GammaRemap[i+3] = g_GammaRemap[i+2]+qdiff;
	}

	for (i = 0; i < 1025; i++) {
		final_R[i] = g_GammaRemap[Power1div22Gamma[i]];
	}

				#if 0//for debug
				for(i=0;i<20;i++){
					printk("yuan,1.Out_R[%d]=%d\n",i,Out_R[i]);
				for(i=0;i<10;i++)
					printk("yuan,GammaRemap[%d]=%d\n",i,g_GammaRemap[i]);
				for(i=0;i<10;i++)
					printk("yuan,hdiff[%d]=%d\n",i,(g_GammaRemap[i+4]-g_GammaRemap[i])>>1);
				for(i=0;i<10;i++)
					printk("yuan,qdiff[%d]=%d\n",i,((g_GammaRemap[i+4]-g_GammaRemap[i])>>1)>>1);
				for(i=0;i<10;i++)
					printk("yuan,1.final_R[%d]=%d\n",Power1div22Gamma[i],final_R[i]);
					}


				for(i=1010;i<1024;i++){
					printk("yuan,1.Out_R[%d]=%d\n",i,Out_R[i]);
				for(i=0;i<10;i++)
					printk("yuan,GammaRemap[%d]=%d\n",i,g_GammaRemap[i]);
				for(i=0;i<10;i++)
					printk("yuan,hdiff[%d]=%d\n",i,(g_GammaRemap[i+4]-g_GammaRemap[i])>>1);
				for(i=0;i<10;i++)
					printk("yuan,qdiff[%d]=%d\n",i,((g_GammaRemap[i+4]-g_GammaRemap[i])>>1)>>1);
				for(i=0;i<10;i++)
					printk("yuan,1.final_R[%d]=%d\n",Power1div22Gamma[i],final_R[i]);
					}
				#endif

	// G
	for (i = 0, j = 0; i < 4096; i+=4, j++) {
		g_GammaRemap[i] = Out_G[j];// << 2;
	}
	g_GammaRemap[4096] = (Out_G[1023]) + (Out_G[1023] >> 8);

	for (i = 0; i < 4096; i+=4) {
		hdiff = (g_GammaRemap[i+4]-g_GammaRemap[i])>>1;
		qdiff = hdiff>>1;
		g_GammaRemap[i+1] = g_GammaRemap[i]+qdiff;
		g_GammaRemap[i+2] = g_GammaRemap[i]+hdiff;
		g_GammaRemap[i+3] = g_GammaRemap[i+2]+qdiff;
	}

	for (i = 0; i < 1025; i++) {
		final_G[i] = g_GammaRemap[Power1div22Gamma[i]];
	}



	// B
	for (i = 0, j = 0; i < 4096; i+=4, j++) {
		g_GammaRemap[i] = Out_B[j];// << 2;
	}
	g_GammaRemap[4096] = (Out_B[1023]) + (Out_B[1023] >> 8);

	for (i = 0; i < 4096; i+=4) {
		hdiff = (g_GammaRemap[i+4]-g_GammaRemap[i])>>1;
		qdiff = hdiff>>1;
		g_GammaRemap[i+1] = g_GammaRemap[i]+qdiff;
		g_GammaRemap[i+2] = g_GammaRemap[i]+hdiff;
		g_GammaRemap[i+3] = g_GammaRemap[i+2]+qdiff;
	}


	for (i = 0; i < 1025; i++) {
		final_B[i] = g_GammaRemap[Power1div22Gamma[i]];
	}
				#if 0//for debug
				for(i=0;i<20;i++){
					printk("yuan,Out_B[%d]=%d\n",i,Out_B[i]);
				for(i=0;i<10;i++)
					printk("yuan,GammaRemap[%d]=%d\n",i,g_GammaRemap[i]);
				for(i=0;i<10;i++)
					printk("yuan,hdiff[%d]=%d\n",i,(g_GammaRemap[i+4]-g_GammaRemap[i])>>1);
				for(i=0;i<10;i++)
					printk("yuan,qdiff[%d]=%d\n",i,((g_GammaRemap[i+4]-g_GammaRemap[i])>>1)>>1);
				for(i=0;i<10;i++)
					printk("yuan,final_B[%d]=%d\n",Power1div22Gamma[i],final_B[i]);
					}
				for(i=1010;i<1024;i++){
					printk("yuan,Out_B[%d]=%d\n",i,Out_B[i]);
				for(i=0;i<10;i++)
					printk("yuan,GammaRemap[%d]=%d\n",i,g_GammaRemap[i]);
				for(i=0;i<10;i++)
					printk("yuan,hdiff[%d]=%d\n",i,(g_GammaRemap[i+4]-g_GammaRemap[i])>>1);
				for(i=0;i<10;i++)
					printk("yuan,qdiff[%d]=%d\n",i,((g_GammaRemap[i+4]-g_GammaRemap[i])>>1)>>1);
				for(i=0;i<10;i++)
					printk("yuan,final_B[%d]=%d\n",Power1div22Gamma[i],final_B[i]);
					}
				#endif
}

void fwif_set_gamma_multiply_remapOETF(UINT16 *final_R, UINT16 *final_G, UINT16 *final_B, UINT16 *Out_R, UINT16 *Out_G, UINT16 *Out_B)
{
	int i, j, hdiff, qdiff;
	extern unsigned short g_GammaRemap[4097];
	extern unsigned short OEFT2Gamma[1025];

	if ((NULL == Out_R)||(NULL == Out_G)||(NULL == Out_B))
		return;

	// R
	for (i = 0, j = 0; i < 4096; i+=4, j++) {
		g_GammaRemap[i] = Out_R[j];// << 2;
	}
	g_GammaRemap[4096] = (Out_R[1023] ) + (Out_R[1023] >> 8);

	for (i = 0; i < 4096; i+=4) {
		hdiff = (g_GammaRemap[i+4]-g_GammaRemap[i])>>1;
		qdiff = hdiff>>1;
		g_GammaRemap[i+1] = g_GammaRemap[i]+qdiff;
		g_GammaRemap[i+2] = g_GammaRemap[i]+hdiff;
		g_GammaRemap[i+3] = g_GammaRemap[i+2]+qdiff;
	}

	for (i = 0; i < 1025; i++) {
		final_R[i] = g_GammaRemap[OEFT2Gamma[i]];
	}

				#if 0//for debug
				for(i=0;i<20;i++){
					printk("yuan,1.Out_R[%d]=%d\n",i,Out_R[i]);
				for(i=0;i<10;i++)
					printk("yuan,GammaRemap[%d]=%d\n",i,g_GammaRemap[i]);
				for(i=0;i<10;i++)
					printk("yuan,hdiff[%d]=%d\n",i,(g_GammaRemap[i+4]-g_GammaRemap[i])>>1);
				for(i=0;i<10;i++)
					printk("yuan,qdiff[%d]=%d\n",i,((g_GammaRemap[i+4]-g_GammaRemap[i])>>1)>>1);
				for(i=0;i<10;i++)
					printk("yuan,1.final_R[%d]=%d\n",OEFT2Gamma[i],final_R[i]);
					}


				for(i=1010;i<1024;i++){
					printk("yuan,1.Out_R[%d]=%d\n",i,Out_R[i]);
				for(i=0;i<10;i++)
					printk("yuan,GammaRemap[%d]=%d\n",i,g_GammaRemap[i]);
				for(i=0;i<10;i++)
					printk("yuan,hdiff[%d]=%d\n",i,(g_GammaRemap[i+4]-g_GammaRemap[i])>>1);
				for(i=0;i<10;i++)
					printk("yuan,qdiff[%d]=%d\n",i,((g_GammaRemap[i+4]-g_GammaRemap[i])>>1)>>1);
				for(i=0;i<10;i++)
					printk("yuan,1.final_R[%d]=%d\n",OEFT2Gamma[i],final_R[i]);
					}
				#endif

	// G
	for (i = 0, j = 0; i < 4096; i+=4, j++) {
		g_GammaRemap[i] = Out_G[j];// << 2;
	}
	g_GammaRemap[4096] = (Out_G[1023]) + (Out_G[1023] >> 8);

	for (i = 0; i < 4096; i+=4) {
		hdiff = (g_GammaRemap[i+4]-g_GammaRemap[i])>>1;
		qdiff = hdiff>>1;
		g_GammaRemap[i+1] = g_GammaRemap[i]+qdiff;
		g_GammaRemap[i+2] = g_GammaRemap[i]+hdiff;
		g_GammaRemap[i+3] = g_GammaRemap[i+2]+qdiff;
	}

	for (i = 0; i < 1025; i++) {
		final_G[i] = g_GammaRemap[OEFT2Gamma[i]];
	}



	// B
	for (i = 0, j = 0; i < 4096; i+=4, j++) {
		g_GammaRemap[i] = Out_B[j];// << 2;
	}
	g_GammaRemap[4096] = (Out_B[1023]) + (Out_B[1023] >> 8);

	for (i = 0; i < 4096; i+=4) {
		hdiff = (g_GammaRemap[i+4]-g_GammaRemap[i])>>1;
		qdiff = hdiff>>1;
		g_GammaRemap[i+1] = g_GammaRemap[i]+qdiff;
		g_GammaRemap[i+2] = g_GammaRemap[i]+hdiff;
		g_GammaRemap[i+3] = g_GammaRemap[i+2]+qdiff;
	}


	for (i = 0; i < 1025; i++) {
		final_B[i] = g_GammaRemap[OEFT2Gamma[i]];
	}
				#if 0//for debug
				for(i=0;i<20;i++){
					printk("yuan,Out_B[%d]=%d\n",i,Out_B[i]);
				for(i=0;i<10;i++)
					printk("yuan,GammaRemap[%d]=%d\n",i,g_GammaRemap[i]);
				for(i=0;i<10;i++)
					printk("yuan,hdiff[%d]=%d\n",i,(g_GammaRemap[i+4]-g_GammaRemap[i])>>1);
				for(i=0;i<10;i++)
					printk("yuan,qdiff[%d]=%d\n",i,((g_GammaRemap[i+4]-g_GammaRemap[i])>>1)>>1);
				for(i=0;i<10;i++)
					printk("yuan,final_B[%d]=%d\n",OEFT2Gamma[i],final_B[i]);
					}
				for(i=1010;i<1024;i++){
					printk("yuan,Out_B[%d]=%d\n",i,Out_B[i]);
				for(i=0;i<10;i++)
					printk("yuan,GammaRemap[%d]=%d\n",i,g_GammaRemap[i]);
				for(i=0;i<10;i++)
					printk("yuan,hdiff[%d]=%d\n",i,(g_GammaRemap[i+4]-g_GammaRemap[i])>>1);
				for(i=0;i<10;i++)
					printk("yuan,qdiff[%d]=%d\n",i,((g_GammaRemap[i+4]-g_GammaRemap[i])>>1)>>1);
				for(i=0;i<10;i++)
					printk("yuan,final_B[%d]=%d\n",OEFT2Gamma[i],final_B[i]);
					}
				#endif
}

unsigned char fwif_Slope_Interpolation(UINT16* Array, unsigned short inLow, unsigned short inHigh, UINT16 outLow, UINT16 outHigh)
{
	UINT16 i;

	if (inLow >= inHigh){
		printf("drvif_color_Slope_Interpolation ERROR: inLow == %d >= %d == inHigh\n", inLow, inHigh);
		return _FALSE;
	}

	if (outLow > outHigh){
		printf("drvif_color_Slope_Interpolation ERROR: outLow == %d >= %d == outHigh\n", outLow, outHigh);
		return _FALSE;
	}

  //unsigned short Slope=(outHigh-outLow)/(inHigh-inLow);
	//if((outHigh-outLow)%(inHigh-inLow)!=0)
	//Slope+=1;

	for(i=inLow;i<=inHigh;i++)
	{
		//printk("Ori==========%d========%d===\n",i,Array[i]);
		Array[i] = outLow + (((outHigh-outLow) * (i - inLow)) / (inHigh - inLow));
		//printk("mid=========%d========%d===\n",i,Array[i]);
		//temp+= (((outHigh-outLow) * (i - inLow))/(inHigh-inLow));
		Array[i]=(Array[i] >= outHigh) ? outHigh : Array[i];
		//printk("Result========%d========%d===\n",i,Array[i]);
	}

	return _TRUE;
}

unsigned char fwif_get_colorspaceInfo(void)
{
	SLR_VOINFO* pVOInfo = NULL;
	HDMI_AVI_T pAviInfo;
	//unsigned char input_src_form=0;
	COLORSPACE_ITEM isColorSpaceIndex=is709;
	_system_setting_info *VIP_system_info_structure_table=NULL;

	//SrcType=Scaler_InputSrcGetMainChType();
	drvif_Hdmi_GetAviInfoFrame(&pAviInfo); //pAviInfo size = 16 bytes
	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	/*
	if (fwif_color_get_force_run_i3ddma_enable(SLR_MAIN_DISPLAY) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) != VSC_INPUTSRC_VDEC)) {
		fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(SLR_MAIN_DISPLAY), &SrcType, &input_src_form);
		//printk("tim_li,i3ddma_enable,SrcType==%d \n",SrcType);
	}
	*/

	if(VIP_system_info_structure_table->Input_src_Type==_SRC_HDMI /*SrcType==_SRC_HDMI*/)
	{
		if(pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_SMPTE170)	//601
			isColorSpaceIndex=is601;
		else if(pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_ITU709) //709
			isColorSpaceIndex=is709;
		else if(pAviInfo.C == VFE_HDMI_AVI_COLORIMETRY_FUTURE)
		{
			if(pAviInfo.EC == 5 || pAviInfo.EC == 6)//bt2020
				isColorSpaceIndex=isBT2020;
			else if(pAviInfo.EC ==1)//709
				isColorSpaceIndex=is709;
			else if(pAviInfo.EC ==0)//601
				isColorSpaceIndex=is601;
			else if(VIP_system_info_structure_table ->HDR_flag)
				isColorSpaceIndex=isBT2020;
			else //709
				isColorSpaceIndex=is709;
		}
		else if(VIP_system_info_structure_table ->HDR_flag)
			isColorSpaceIndex=isBT2020;
		else //709
			isColorSpaceIndex=is709;
	}
	else if(VIP_system_info_structure_table->Input_src_Type==_SRC_VO /*SrcType==_SRC_VO*/)
	{
		if (pVOInfo->colour_primaries == 9 || VIP_system_info_structure_table ->HDR_flag)	/* bt2020*/
			isColorSpaceIndex=isBT2020;
		else
			isColorSpaceIndex=is709;
	}
	else//709
		isColorSpaceIndex=is709;

	//printk("tim_li,srctype=%d,pAviInfo.C=%d,pAviInfo.EC=%d \n",VIP_system_info_structure_table->Input_src_Type,pAviInfo.C,pAviInfo.EC);
	//printk("isColorSpaceIndex=%d,isBT2020=%d,is709=%d,is601=%d\n",isColorSpaceIndex,isBT2020,is709,is601);

	return (COLORSPACE_ITEM) isColorSpaceIndex;
}

void fwif_color_set_sRGBMatrix(void)
{
	extern short BT2020Target_sRGB_APPLY[3][3];
	extern short BT709Target_sRGB_APPLY[3][3];
	extern short DCIP3Target_sRGB_APPLY[3][3];
	//extern short ADOBERGBTarget_sRGB_APPLY[3][3]; //mac7p pq compile fix
	extern unsigned char g_srgbForceUpdate;
	COLORSPACE_ITEM isColorSpaceIndex=is709;
	unsigned char isDCIP3=0;
	SLR_VOINFO* pVOInfo = NULL;

	_system_setting_info *VIP_system_info_structure_table=NULL;
	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());

	isColorSpaceIndex=fwif_get_colorspaceInfo();

	if(VIP_system_info_structure_table ->HDR_flag==1){
		HDMI_DRM_T DRM_Info;
		drvif_Hdmi_GetDrmInfoFrame(&DRM_Info);

		#if 0//for debug
		printk("x0=%d,y0=%d\n",DRM_Info.display_primaries_x0,DRM_Info.display_primaries_y0);
		printk("x1=%d,y1=%d\n",DRM_Info.display_primaries_x1,DRM_Info.display_primaries_y1);
		printk("x2=%d,y2=%d\n",DRM_Info.display_primaries_x2,DRM_Info.display_primaries_y2);
		#endif

		if( (DRM_Info.display_primaries_x0 == 13250)&&(DRM_Info.display_primaries_y0 == 34500)
			&&(DRM_Info.display_primaries_x1 ==7500)&&(DRM_Info.display_primaries_y1 == 3000)
			&&(DRM_Info.display_primaries_x2== 34000)&&(DRM_Info.display_primaries_y2 == 16000))	/* DCIP3*/
			isDCIP3 = 1;
		else
			isDCIP3 = 0;
	}
	else
		isDCIP3 = 0;

	//printk("fwif_color_set_sRGBMatrix adobe=%d\n",pVOInfo->isAdobeRGB); //mac7p pq compile fix
	if(isColorSpaceIndex==isBT2020)
	{	printk("isBT2020\n");
		fwif_color_ColorMap_SetMatrix(SLR_MAIN_DISPLAY, BT2020Target_sRGB_APPLY, 2, g_srgbForceUpdate, 0);
	/*}else if(pVOInfo->isAdobeRGB) //mac7p pq compile fix
	{	printk("isAdobeRGB\n");
		fwif_color_ColorMap_SetMatrix(SLR_MAIN_DISPLAY, ADOBERGBTarget_sRGB_APPLY, g_srgbForceUpdate); */
	}else if(isDCIP3 ==1)
	{	printk("isDCIP3\n");
		fwif_color_ColorMap_SetMatrix(SLR_MAIN_DISPLAY, DCIP3Target_sRGB_APPLY, 2, g_srgbForceUpdate, 0);
	}
	else if(isColorSpaceIndex == is709)
	{	printk("is709\n");
		fwif_color_ColorMap_SetMatrix(SLR_MAIN_DISPLAY, BT709Target_sRGB_APPLY, 2, g_srgbForceUpdate, 0);
	}
	else
	{	printk("the others 709\n");
		fwif_color_ColorMap_SetMatrix(SLR_MAIN_DISPLAY, BT709Target_sRGB_APPLY, 2, g_srgbForceUpdate, 0);
	}
}
#if 0
char fwif_colo_get_QS_Table(DRV_QS_Table *ptr)
{
	char ret;

	ptr = QS_mode_table;
	return ret;
}

DRV_QS_Table *fwif_colo_get_QS_Table(void)
{
	if (gVip_Table == NULL) {
		VIPprintf("gVip_Table is NULL\n");
		return NULL;
	}
	return (SLR_VIP_TABLE *)gVip_Table;

	/*return (SLR_VIP_TABLE *)&gVip_Table;*/
}
#endif

unsigned char fwif_write_qs_pq_table(qs_idx idx , unsigned char *ptr)
{
#ifndef BUILD_QUICK_SHOW
	struct file* filp = NULL;
	char filename[500];

	//static DRV_Sharpness_Table sharp = {0};
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return -1;
	}


	if(ptr ==NULL){
		pr_emerg("~ptr =null, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	if(idx ==qs_Osd){
		QS_value_Store *pQs_value;
		pQs_value = (QS_value_Store*)ptr;
		
		memcpy(&QS_mode_table.qs_Store, pQs_value, sizeof(QS_value_Store));


		pr_emerg("\n vpq vpq fwif_write_qs_pq_table  OSD_Contrast %d, OSD_Brightness :%d ,OSD_Saturation%d , OSD_Hue %d ,OSD_Sharpness %d , OSD_colorTemptemp %d \n"
			,QS_mode_table.qs_Store.OSD_Contrast
			,QS_mode_table.qs_Store.OSD_Brightness
			,QS_mode_table.qs_Store.OSD_Saturation
			,QS_mode_table.qs_Store.OSD_Hue
			,QS_mode_table.qs_Store.OSD_Sharpness
			,QS_mode_table.qs_Store.OSD_colorTemp);

		pr_emerg("\n vpq fwif_write_qs_pq_table  Contrast_Gain %d, Brightness_Gain :%d ,Saturation_Gain %d , Hue_Gain %d , Brightness_Cmps%d ,Saturation_Cmps%d\n"
			,QS_mode_table.qs_Store.Contrast_Gain
			,QS_mode_table.qs_Store.Brightness_Gain
			,QS_mode_table.qs_Store.Saturation_Gain
			,QS_mode_table.qs_Store.Hue_Gain
			,QS_mode_table.qs_Store.Brightness_Cmps
			,QS_mode_table.qs_Store.Saturation_Cmps);



	}else if (idx ==qs_sharp_idx){

		memcpy(&QS_mode_table.qs_sharp,&(gVip_Table->Ddomain_SHPTable[1]),sizeof(VIP_Sharpness_Table));

	}else if (idx ==qs_MBPK_idx){

		memcpy(&QS_mode_table.qs_MBPK,&(gVip_Table->Idomain_MBPKTable[1]),sizeof(VIP_MBPK_Table));

	}else if (idx ==qs_DLTI_idx){

		//memcpy(&QS_mode_table.qs_sharp,&(gVip_Table->Ddomain_SHPTable[1]),sizeof(VIP_Sharpness_Table));

	}else if (idx ==qs_iDcti_idx){

		memcpy(&QS_mode_table.qs_iDcti,&(gVip_Table->VIP_INewDcti_Table[1]),sizeof(DRV_VipNewIDcti_Table));

	}else if (idx ==qs_dDcti_idx){

		memcpy(&QS_mode_table.qs_dDcti,&(gVip_Table->VIP_DNewDcti_Table[1]),sizeof(DRV_VipNewDDcti_Table));

	}else if (idx ==qs_Gamma_idx){

		//memcpy(&QS_mode_table.qs_sharp,&(gVip_Table->Ddomain_SHPTable[1]),sizeof(VIP_Sharpness_Table));

	}else{



	}

	/* write to  qs_pq_table  */

	sprintf(filename, "/mnt/vendor/factory/qs_pq_table.bin");

	filp = file_open(filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
	if (filp == NULL) {
		rtd_printk(KERN_INFO,"fwif_update_qs_pq_table", "(%d)open fail\n", __LINE__);
		return -1;
	}

	file_write(filp, 0, (unsigned char*)&QS_mode_table, sizeof(DRV_QS_Table));
	file_sync(filp);
	file_close(filp);
#endif
	return 0;

}


/**********************************  for quick show funciton ***************************************/

#ifdef BUILD_QUICK_SHOW /*for quick_show mode */

VIP_YUV2RGB_INPUT_DATA_MODE Input_Data_Mode_quick_show = YUV2RGB_INPUT_601_Limted;
extern SLR_VIP_TABLE m_defaultVipTable; 
void *pq_table_addr = NULL;
vfe_hdmi_timing_info_t vpq_hdmi_timinginfo;

/***********************globa ***************************/

	unsigned int satCmps_qs;
	short cmps_qs = 0;
	UINT32 contrast_qs, bri_qs, color_qs, deg_tint_qs;
	int tbl_offset_R_qs, tbl_offset_G_qs, tbl_offset_B_qs;
	unsigned char quick_show_src_idx=255;

/********************************************************/
extern int get_pq_table(void **addr, unsigned int *len);

void fwif_color_pq_init_quick_show(void)
{
	unsigned int pq_table_len = 0;
	unsigned char ret;
	unsigned char i;
	extern void *pq_table_addr;

	if(0 != vfe_hdmi_drv_get_display_timing_info(&vpq_hdmi_timinginfo, 0))
	{
		printf("\n%s:%d **** Get timing error  ****\n", __FILE__, __LINE__);
		return;
	}


	printf("+VIP color handler+\n");



	ret = get_pq_table(&pq_table_addr, &pq_table_len);
	if (ret) {
		printf("\n%s:%d **** cannot read qs_pq_table.bin ****\n", __FILE__, __LINE__);
	}else{
		printf("\n%s:%d **** pq addr:0x%08x, len:0x%08x  ****\n", __FILE__, __LINE__,
		(unsigned long)pq_table_addr, pq_table_len);

		memcpy((unsigned char *)&QS_mode_table, (unsigned char *)pq_table_addr, pq_table_len);
	}

	printf("\n  OSD_Contrast %d, OSD_Brightness :%d ,OSD_Saturation%d , OSD_Hue %d ,OSD_Sharpness %d , OSD_colorTemp %d\n"
		,QS_mode_table.qs_Store.OSD_Contrast
		, QS_mode_table.qs_Store.OSD_Brightness
		,QS_mode_table.qs_Store.OSD_Saturation
		,QS_mode_table.qs_Store.OSD_Hue
		,QS_mode_table.qs_Store.OSD_Sharpness
		,QS_mode_table.qs_Store.OSD_colorTemp);

	printf("\n  Contrast_Gain %d, Brightness_Gain :%d ,Saturation_Gain %d , Hue_Gain %d , Brightness_Cmps%d ,Saturation_Cmps%d\n"
		,QS_mode_table.qs_Store.Contrast_Gain
		,QS_mode_table.qs_Store.Brightness_Gain
		,QS_mode_table.qs_Store.Saturation_Gain
		,QS_mode_table.qs_Store.Hue_Gain
		,QS_mode_table.qs_Store.Brightness_Cmps
		,QS_mode_table.qs_Store.Saturation_Cmps);




}
	
void fwif_color_colorspacergb2yuvtransfer_quick_show(void)
{
	unsigned char table_num = RGB2YUV_COEF_MATRIX_MODE_Max;
	unsigned char data_range=MODE_RAG_LIMIT;
	unsigned short *table_index = NULL;
	unsigned char color_space;  //COLOR_RGB
	unsigned char color_imetry;
	unsigned char hd_input =0;
	SCALER_DISP_CHANNEL display = (SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	
	printf("[vpq]  %s  funciton start ,line %d \n",__func__,__LINE__);

	//table_num = 2; //fwif_color_get_colorspacergb2yuvtransfer_table(channel, nSrcType, nSD_HD, true);
	//table_index = &(tRGB2YUV_COEF[table_num][0]);

	//vpq_hdmi_timinginfo.color_space
	//vpq_hdmi_timinginfo.color_imetry


	data_range = drvif_IsRGB_YUV_RANGE(); //MODE_RAG_LIMIT;
		
	color_space =drvif_Hdmi_GetColorSpace();
	//data_range = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)display, SLR_INPUT_COLOR_IMETRY)()

	if(color_space ==COLOR_YUV422){
		drvif_color_set422to444(_CHANNEL1, 1);
	}else{
		drvif_color_set422to444(_CHANNEL1, 0);
	}

	//color_imetry = drvif_Hdmi_GetColorimetry();//HDMI_COLORIMETRY_601

	color_imetry = HDMI_COLORIMETRY_709;//dias always use 709 limit 
		
	/*if(vpq_hdmi_timinginfo.active.w >720)
		hd_input =1;
	else
		hd_input =0;
	*/
		hd_input =1 ;//dias always use 709 limit 
	switch (color_space)
	{

		case COLOR_RGB:

			if(data_range ==  MODE_RAG_LIMIT ){ // input is limit range 
				if (color_imetry == HDMI_COLORIMETRY_601 ){
					table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
					Input_Data_Mode_quick_show = YUV2RGB_INPUT_601_Limted;

				}else if (color_imetry == HDMI_COLORIMETRY_709 ){
					table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC709_16_235;
					Input_Data_Mode_quick_show = YUV2RGB_INPUT_709_Limted;

				}

			}else if(data_range == MODE_RAG_FULL){ //input is full range 
				if (color_imetry == HDMI_COLORIMETRY_601 ){
					table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235;
					Input_Data_Mode_quick_show = YUV2RGB_INPUT_601_Limted;

				}else if (color_imetry == HDMI_COLORIMETRY_709 ){
					table_num = RGB2YUV_COEF_RGB_0_255_TO_YCC709_16_235;
					Input_Data_Mode_quick_show = YUV2RGB_INPUT_709_Limted;

				}
			}else{ // data_range = MODE_RAG_DEFAULT or MODE_RAG_UNKNOW

				if (color_imetry == HDMI_COLORIMETRY_601 ){
					table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
					Input_Data_Mode_quick_show = YUV2RGB_INPUT_601_Limted;

				}else if (color_imetry == HDMI_COLORIMETRY_709 ){
					table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC709_16_235;
					Input_Data_Mode_quick_show = YUV2RGB_INPUT_709_Limted;

				}else{
					table_num = RGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235;
					Input_Data_Mode_quick_show = YUV2RGB_INPUT_601_Limted;

				}
			}
			
			break;


		case COLOR_YUV444: 
		case COLOR_YUV422: 
			if ( data_range == MODE_RAG_FULL) {
				table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
			} else {
				table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
			}

			if(hd_input ==1){
				Input_Data_Mode_quick_show = YUV2RGB_INPUT_709_Limted;
			}else{
				Input_Data_Mode_quick_show = YUV2RGB_INPUT_601_Limted;
			}

			break;

		default:
		case COLOR_UNKNOW: 
			if ( data_range == MODE_RAG_FULL) {
				table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
			} else {
				table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
			}

			if(hd_input ==1){
				Input_Data_Mode_quick_show = YUV2RGB_INPUT_709_Limted;
			}else{
				Input_Data_Mode_quick_show = YUV2RGB_INPUT_601_Limted;
			}
			break;

	}
	table_index = &(tRGB2YUV_COEF[table_num][0]);


	printf("[vpq] color_space =%d ,data_range =%d ,color_imetry =%d ,table_num =%d ,hd_input =%d ,Input_Data_Mode =%d\n",
		color_space,
		data_range,
		color_imetry,
		table_num,
		hd_input,
		Input_Data_Mode_quick_show);


	drvif_color_colorspacergb2yuvtransfer(0, table_index);

	printf("[vpq]  %s  funciton end ,line %d \n",__func__,__LINE__);

}
void fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain_quick_show(void)
{
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	unsigned short table_idx[3][3][VIP_YUV2RGB_Y_Seg_Max];
	short satTmp[3][3], hueTmp[3][3], kTmp[3][3], kTmp1[3][3];
	unsigned char Input_mode, YUV2RGB_TBL_Idx;

#if 0  // move to globa 
	unsigned int satCmps;
	short cmps = 0;
	UINT32 contrast, bri, color, deg_tint;
	int tbl_offset_R, tbl_offset_G, tbl_offset_B;
#endif 
	UINT8 i, j, k;

	unsigned char bt2020_Constant_En;
	int bt2020_k13, bt2020_k32;

	//Input_mode = system_info_struct->using_YUV2RGB_Matrix_Info.Input_Data_Mode;
	//YUV2RGB_TBL_Idx = fwif_color_get_Decide_YUV2RGB_TBL_Index(src_idx, display, Input_mode);

	bt2020_Constant_En =0;
/**************************************************************************************/

	Input_mode =Input_Data_Mode_quick_show;

	switch (Input_Data_Mode_quick_show) {
		case  YUV2RGB_INPUT_601_Limted:
			YUV2RGB_TBL_Idx = YUV2RGB_TBL_SELECT_601_Limted_235_240_to_255;
			break;
		case  YUV2RGB_INPUT_601_Full:
			YUV2RGB_TBL_Idx = YUV2RGB_TBL_SELECT_601_Limted_NO_Gain;
			break;
		case  YUV2RGB_INPUT_709_Limted:
			YUV2RGB_TBL_Idx = YUV2RGB_TBL_SELECT_709_Limted_235_240_to_255;
			break;
		case  YUV2RGB_INPUT_709_Full:
			YUV2RGB_TBL_Idx = YUV2RGB_TBL_SELECT_709_Limted_NO_Gain;
			break;
		case  YUV2RGB_INPUT_2020_NonConstantY:
			YUV2RGB_TBL_Idx = YUV2RGB_TBL_SELECT_2020_NonConstantY;
			break;
		case  YUV2RGB_INPUT_2020_ConstantY:
			YUV2RGB_TBL_Idx = YUV2RGB_TBL_SELECT_2020_ConstantY;
			break;
		default:
			YUV2RGB_TBL_Idx = YUV2RGB_TBL_SELECT_601_Limted_NO_Gain;
			break;
	}



/**************************************************************************************/
	//if (Scaler_Get_CinemaMode_PQ()) 
	{
		contrast_qs = 128;
		bri_qs = 128;
		color_qs = 128;
		deg_tint_qs = 0;
		cmps_qs = 0;
		satCmps_qs = 102;

		contrast_qs = QS_mode_table.qs_Store.Contrast_Gain;
		bri_qs = QS_mode_table.qs_Store.Brightness_Gain;
		color_qs = QS_mode_table.qs_Store.Saturation_Gain;

		deg_tint_qs = QS_mode_table.qs_Store.Hue_Gain;
		cmps_qs = QS_mode_table.qs_Store.Brightness_Cmps;
		satCmps_qs = QS_mode_table.qs_Store.Saturation_Cmps;
		


		
	}

	printf("[vpq][%s][%d]contrast_qs = %d, bri_qs = %d, color_qs = %d, deg_tint_qs = %d\n", __FUNCTION__, __LINE__, contrast_qs, bri_qs, color_qs, deg_tint_qs);
	printf("[vpq][%s][%d]satCmps_qs = %d, cmps_qs = %d\n", __FUNCTION__, __LINE__, satCmps_qs, cmps_qs);

	memset(satTmp, 0, sizeof(satTmp));
	memset(hueTmp, 0, sizeof(hueTmp));
	memset(kTmp, 0, sizeof(kTmp));
	memset(kTmp1, 0, sizeof(kTmp1));

	/* no hue function while bt2020_Constant_En*/
	if (bt2020_Constant_En == 1)
		deg_tint_qs = 0;
	/*Euler's formula*/
	hueTmp[0][0] = 1024;
	hueTmp[1][1] = CAdjustCosine(deg_tint_qs);
	hueTmp[1][2] = CAdjustSine(deg_tint_qs) * (-1);
	hueTmp[2][1] = CAdjustSine(deg_tint_qs);
	hueTmp[2][2] = CAdjustCosine(deg_tint_qs);
	/*restrict for sat, refernece to spec for k32.*/
	color_qs = (color_qs * satCmps_qs) >> 7;
	color_qs = (color_qs > 128) ?  75 + (color_qs*53 / 128) : color_qs;
	satTmp[0][0] = 128;	/*128  = 1*/
	satTmp[1][1] = color_qs;
	satTmp[2][2] = color_qs;

	for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++) {
		table_idx[0][0][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K11];
		table_idx[0][1][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K12];
		table_idx[0][2][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K13];
		table_idx[1][0][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K11];
		table_idx[1][1][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K22];
		table_idx[1][2][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K23];
		table_idx[2][0][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K11];
		table_idx[2][1][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K32];
		table_idx[2][2][i] = tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_K33];
	}

	/*matrix computation for contrast, sat and hue*/
	for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++) {
		for (j = 0; j < 3; j++) {
			for (k = 0; k < 3; k++) {
				kTmp[j][k] = table_idx[j][k][i];
				/*bigger than 0x3FF  = >neg*/
				if (j == 0) {
					if ((kTmp[j][k] > VIP_YUV2RGB_K_Y_NEG_BND)){
						kTmp[j][k] = VIP_YUV2RGB_K_Y_NEG_CHANGE(kTmp[j][k]);
						kTmp[j][k] = kTmp[j][k] * (-1);
					}
				} else {
					if ((kTmp[j][k] > VIP_YUV2RGB_K_C_NEG_BND)){
						kTmp[j][k] = VIP_YUV2RGB_K_C_NEG_CHANGE(kTmp[j][k]);
						kTmp[j][k] = kTmp[j][k] * (-1);
					}
				}
			}
		}
		/*3x3 matrix multiply*/
		for (j = 0; j < 3; j++)
			for (k = 0; k < 3; k++)
				kTmp1[j][k] = (kTmp[j][0] * hueTmp[0][k] + kTmp[j][1] * hueTmp[1][k] + kTmp[j][2] * hueTmp[2][k]) >> 10;

		for (j = 0; j < 3; j++)
			for (k = 0; k < 3; k++)
				kTmp[j][k] = (kTmp1[j][0] * satTmp[0][k] + kTmp1[j][1] * satTmp[1][k] + kTmp1[j][2] * satTmp[2][k]) >> 7;

		kTmp[0][0] = (kTmp[0][0] * contrast_qs) >> 7;

		drv_vipCSMatrix_t.COEF_By_Y.K11[i] = kTmp[0][0];
		drv_vipCSMatrix_t.COEF_By_Y.K12[i] = kTmp[0][1];
		drv_vipCSMatrix_t.COEF_By_Y.K13[i] = kTmp[0][2];
		drv_vipCSMatrix_t.COEF_By_Y.K22[i] = kTmp[1][1];
		drv_vipCSMatrix_t.COEF_By_Y.K23[i] = kTmp[1][2];
		drv_vipCSMatrix_t.COEF_By_Y.K32[i] = kTmp[2][1];
		drv_vipCSMatrix_t.COEF_By_Y.K33[i] = kTmp[2][2];
	}

	/*
	printf("\n vpq contrast %d, deg_tint %d , color %d  satCmps %d , cmps %d\n ",
		contrast,
		deg_tint,
		color,
		satCmps,
		cmps);
	
	printf("\n vpq K11 %x, K12,%x , k13 %x  \n ",
		kTmp[0][0],
		kTmp[0][1],
		kTmp[0][2]);


	printf("\n vpq K22 %x, K23,%x , k32 %x ,k33 %x \n ",
		kTmp[1][1],
		kTmp[1][2],
		kTmp[2][1],
		kTmp[2][2]);
	*/


	

	/* bt2020_Constant_En,  for bt 2020 k13, k32*/
	if (bt2020_Constant_En == 1) {
		bt2020_k13 =  tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K13_2];
		bt2020_k32 =  tYUV2RGB_COEF[Input_mode][tUV2RGB_COEF_K32_2];
		if (bt2020_k13 > VIP_YUV2RGB_K_C_NEG_BND) {
			bt2020_k13 = VIP_YUV2RGB_K_C_NEG_CHANGE(bt2020_k13);
			bt2020_k13 = bt2020_k13 * (-1);
		}
		if (bt2020_k32 > VIP_YUV2RGB_K_C_NEG_BND) {
			bt2020_k32 = VIP_YUV2RGB_K_C_NEG_CHANGE(bt2020_k32);
			bt2020_k32 = bt2020_k32 * (-1);
		}
		/* no hue function while bt 2020, only saturation*/
		bt2020_k13 = (bt2020_k13 * satTmp[1][1])>>7;
		bt2020_k32 = (bt2020_k32 * satTmp[1][1])>>7;
		/* driver for bt 2020*/
		drv_vipCSMatrix_t.COEF_By_Y.bt2020_K13= bt2020_k13;
		drv_vipCSMatrix_t.COEF_By_Y.bt2020_K32= bt2020_k32;
	}

	/*RGB offset setting for brightness*/
	/*Roffset, Goffset, Boffset: S(15,2)*//*bigger than 0x3FFF  = >neg*/
	tbl_offset_R_qs = (tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_R_Offset] > VIP_YUV2RGB_OFFSET_NEG_BND) ? (VIP_YUV2RGB_OFFSET_NEG_CHANGE(tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_R_Offset])) : (tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_R_Offset]);
	tbl_offset_G_qs = (tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_G_Offset] > VIP_YUV2RGB_OFFSET_NEG_BND) ? (VIP_YUV2RGB_OFFSET_NEG_CHANGE(tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_G_Offset])) : (tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_G_Offset]);
	tbl_offset_B_qs = (tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_B_Offset] > VIP_YUV2RGB_OFFSET_NEG_BND) ? (VIP_YUV2RGB_OFFSET_NEG_CHANGE(tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_B_Offset])) : (tYUV2RGB_COEF[YUV2RGB_TBL_Idx][tUV2RGB_COEF_B_Offset]);
	drv_vipCSMatrix_t.RGB_Offset.R_offset = (((bri_qs - 128) << 5) + (cmps_qs << 4) + tbl_offset_R_qs) & 0x1ffff; /*20140220 roger for sync the brightness step with Mac, Magellen	modified by Flora, for Sirius; re - modified by Elsie 20140205*/
	drv_vipCSMatrix_t.RGB_Offset.G_offset = (((bri_qs - 128) << 5) + (cmps_qs << 4) + tbl_offset_G_qs) & 0x1ffff; /*cmps: 4bit fraction*/
	drv_vipCSMatrix_t.RGB_Offset.B_offset = (((bri_qs - 128) << 5) + (cmps_qs << 4) + tbl_offset_B_qs) & 0x1ffff;

	printf("[vpq][%s][%d]Input_mode = %d, YUV2RGB_TBL_Idx = %d, cmps = %d\n", __FUNCTION__, __LINE__, Input_mode, YUV2RGB_TBL_Idx, cmps_qs);

	//printf("[vpq][%s][%d]tbl_offset_R = %d, RGB_Offset.R_offset = %d\n", __FUNCTION__, __LINE__, tbl_offset_R, drv_vipCSMatrix_t.RGB_Offset.R_offset);
	//printf("[vpq][%s][%d]tbl_offset_G = %d, RGB_Offset.G_offset = %d\n", __FUNCTION__, __LINE__, tbl_offset_G, drv_vipCSMatrix_t.RGB_Offset.G_offset);
	//printf("[vpq][%s][%d]tbl_offset_B = %d, RGB_Offset.B_offset = %d\n", __FUNCTION__, __LINE__, tbl_offset_B, drv_vipCSMatrix_t.RGB_Offset.B_offset);


	/*Clamp setting*/
	/*drv_vipCSMatrix_t.CTRL_ITEM.Y_Clamp = Y_Clamp;*/
	//drv_vipCSMatrix_t.CTRL_ITEM.Y_Clamp = system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.CTRL_ITEM.Y_Clamp;
	drv_vipCSMatrix_t.CTRL_ITEM.Y_Clamp = 0;
		/*share memory setting*/
#ifdef CONFIG_ARM64
		memcpy_toio(&system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y, &drv_vipCSMatrix_t.COEF_By_Y, sizeof(DRV_VIP_YUV2RGB_COEF_By_Y));
		memcpy_toio(&system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset, &drv_vipCSMatrix_t.RGB_Offset, sizeof(DRV_VIP_YUV2RGB_RGB_Offset));
#else
		//memcpy(&system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y, &drv_vipCSMatrix_t.COEF_By_Y, sizeof(DRV_VIP_YUV2RGB_COEF_By_Y));
		//memcpy(&system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset, &drv_vipCSMatrix_t.RGB_Offset, sizeof(DRV_VIP_YUV2RGB_RGB_Offset));
#endif
		/*fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K11[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K12[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K13[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K22[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K23[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K32[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.K33[0]), VIP_YUV2RGB_Y_Seg_Max, 0);
		fwif_color_ChangeUINT32Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset.R_offset), 1, 0);
		fwif_color_ChangeUINT32Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset.G_offset), 1, 0);
		fwif_color_ChangeUINT32Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.RGB_Offset.B_offset), 1, 0);
		*/
		
		/*drv setting*/





		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Y_Clamp, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_Y_Only, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Offset_Only, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		/* bt2020_Constant_En,  for bt 2020 k13, k32*/


		/*if (bt2020_Constant_En == 1) {
			//drv setting
			drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_BT2020_Only, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
			//memory setting
			fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.bt2020_K13), 1, 0);
			fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.bt2020_K32), 1, 0);
		}*/

	return;

}
void fwif_color_set_YUV2RGB_quick_show(void)
{
	/*base default setting*/
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	/*unsigned char i;*/
	unsigned char YUV2RGB_TBL_Idx;
	unsigned char Input_mode;

	Input_mode =1;// VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode;

	//memcpy(&drv_vipCSMatrix_t, &(gVip_Table->YUV2RGB_CSMatrix_Table[0].YUV2RGB_CSMatrix[VIP_YUV2RGB_LEVEL_OFF]), sizeof(DRV_VIP_YUV2RGB_CSMatrix));
	
	memcpy(&drv_vipCSMatrix_t, &(m_defaultVipTable.YUV2RGB_CSMatrix_Table[0].YUV2RGB_CSMatrix[VIP_YUV2RGB_LEVEL_OFF]), sizeof(DRV_VIP_YUV2RGB_CSMatrix));

	drvif_color_setYUV2RGB_CSMatrix_quick_show(&drv_vipCSMatrix_t, VIP_CSMatrix_YUV2RGB_Base_Ctrl, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	drvif_color_setYUV2RGB_CSMatrix_quick_show(&drv_vipCSMatrix_t, VIP_CSMatrix_CoefByY_ctrl, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);



	//drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_CoefByY_Index, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	//drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_UVOffset_ctrl, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	//drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_UVOffset_Coef, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	//drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_UVOffset_Index, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);

}

void fwif_color_SetDataFormatHandler_quick_show(unsigned char en_422to444)
{
#if 1 //new flow 
	drvif_color_set_Vivid_Color_Enable(TRUE);


#else

	_RPC_system_setting_info* RPC_system_info_structure_table;
	_system_setting_info *VIP_system_info_structure_table=NULL;

	VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	drvif_color_set_Vivid_Color_Enable(TRUE);
	//fwif_color_colorspacergb2yuvtransfer_quick_show();
	//fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain_quick_show();
	//fwif_color_set_YUV2RGB_quick_show();

	if(RPC_system_info_structure_table == NULL){
		printf2( "fwif_color_SetDataFormatHandler_quick_show, NULL table \n");
		return;
	}

	RPC_system_info_structure_table->PQ_DI_RTNR_PQC_CTRL.cmp_alpha_en = 0;


	//set PQC initial value
	//printf2("1 RPC_system_info_structure_table->PQ_DI_RTNR_PQC_CTRL.cmp_alpha_en =%d\n",RPC_system_info_structure_table->PQ_DI_RTNR_PQC_CTRL.cmp_alpha_en);
	//printf2("2 cmp_alpha_en=%d\n",RPC_system_info_structure_table->PQ_DI_RTNR_PQC_CTRL.cmp_alpha_en);

	printf("[vpq][%s][%d] \n", __FUNCTION__, __LINE__);


	quick_show_src_idx = fwif_vip_source_check(0,0);

	if(VIP_system_info_structure_table == NULL){
		printf2( "VIP_system_info_structure_table, NULL table \n");
		return;
	}


	VIP_system_info_structure_table->input_display = Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	VIP_system_info_structure_table->HDMI_color_space = Scaler_DispGetInputInfo(SLR_INPUT_COLOR_SPACE);
	VIP_system_info_structure_table->IV_Start = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_STA);
	VIP_system_info_structure_table->IH_Start = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_STA);
	VIP_system_info_structure_table->I_Height = Scaler_DispGetInputInfo(SLR_INPUT_IPV_ACT_LEN);
	VIP_system_info_structure_table->I_Width = Scaler_DispGetInputInfo(SLR_INPUT_IPH_ACT_WID);
	VIP_system_info_structure_table->Mem_Height = Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_LEN);
	VIP_system_info_structure_table->Mem_Width = Scaler_DispGetInputInfo(SLR_INPUT_MEM_ACT_WID);
	VIP_system_info_structure_table->Cap_Height = Scaler_DispGetInputInfo(SLR_INPUT_CAP_LEN);
	VIP_system_info_structure_table->Cap_Width = Scaler_DispGetInputInfo(SLR_INPUT_CAP_WID);
	VIP_system_info_structure_table->DI_Width = Scaler_DispGetInputInfo(SLR_INPUT_DI_WID);
	VIP_system_info_structure_table->D_Height = Scaler_DispGetInputInfo(SLR_INPUT_DISP_LEN);
	VIP_system_info_structure_table->D_Width = Scaler_DispGetInputInfo(SLR_INPUT_DISP_WID);
	VIP_system_info_structure_table->color_fac_src_idx = Scaler_DispGetInputInfoByDisp((SCALER_DISP_CHANNEL)SLR_MAIN_DISPLAY, SLR_INPUT_DATA_ARRAY_IDX);

#if 0
	printf2(" 3 input_display = %d\n",VIP_system_info_structure_table->input_display);
	printf2(" 4 HDMI_color_space = %d\n",VIP_system_info_structure_table->HDMI_color_space);
	printf2(" 5 IV_Start = %d\n",VIP_system_info_structure_table->IV_Start);
	printf2(" 6 IH_Start = %d\n",VIP_system_info_structure_table->IH_Start);
	printf2(" 7 I_Height = %d\n",VIP_system_info_structure_table->I_Height);
	printf2(" 8 I_Width = %d\n",VIP_system_info_structure_table->I_Width);
	printf2(" 9 Mem_Height = %d\n",VIP_system_info_structure_table->Mem_Height);
	printf2("10 Mem_Width = %d\n",VIP_system_info_structure_table->Mem_Width);
	printf2("11 Cap_Height = %d\n",VIP_system_info_structure_table->Cap_Height);
	printf2("12 Cap_Width = %d\n",VIP_system_info_structure_table->Cap_Width);
	printf2("13 DI_Width = %d\n",VIP_system_info_structure_table->DI_Width);
	printf2("14 D_Height = %d\n",VIP_system_info_structure_table->D_Height);
#endif



#if 0 
	drvif_color_Set_D_DLTI_Table(&QS_mode_table.qs_DLTI);
	drvif_color_dnewdcti_table((DRV_VipNewDDcti_Table *)&QS_mode_table.qs_dDcti);
	drvif_color_inewdcti_table((DRV_VipNewIDcti_Table *)&QS_mode_table.qs_iDcti);


	fwif_color_set_scaleup_hv4tap_coef(0,0
	,(char)m_defaultVipTable.VIP_QUALITY_Extend2_Coef[quick_show_src_idx][VIP_QUALITY_FUNCTION_SU_H]
	,(char)m_defaultVipTable.VIP_QUALITY_Extend2_Coef[quick_show_src_idx][VIP_QUALITY_FUNCTION_SU_V]
	,(char)m_defaultVipTable.VIP_QUALITY_Extend2_Coef[quick_show_src_idx][VIP_QUALITY_FUNCTION_SU_C_H]
	,(char)m_defaultVipTable.VIP_QUALITY_Extend2_Coef[quick_show_src_idx][VIP_QUALITY_FUNCTION_SU_C_V]);


	fwif_color_set_scaleuph_12tap_driver(0,0,m_defaultVipTable.VIP_QUALITY_Extend2_Coef[quick_show_src_idx][VIP_QUALITY_FUNCTION_SU_H_12TAP]);

	fwif_color_set_scaleupv_8tap_driver(0,0,m_defaultVipTable.VIP_QUALITY_Extend2_Coef[quick_show_src_idx][VIP_QUALITY_FUNCTION_SU_V_8TAP]);

	fwif_color_set_scaleup_dircoef(0,0, m_defaultVipTable.VIP_QUALITY_Extend2_Coef[quick_show_src_idx][VIP_QUALITY_FUNCTION_SU_DIR]);

	fwif_color_set_scaleup_dir_weighting_driver(0, m_defaultVipTable.VIP_QUALITY_Extend2_Coef[quick_show_src_idx][VIP_QUALITY_FUNCTION_SU_DIR_Weighting]);

	fwif_color_set_scaledownh_table(0,m_defaultVipTable.VIP_QUALITY_Extend2_Coef[quick_show_src_idx][VIP_QUALITY_FUNCTION_SD_H_Table]);

	fwif_color_set_scaledownv_table(0,m_defaultVipTable.VIP_QUALITY_Extend2_Coef[quick_show_src_idx][VIP_QUALITY_FUNCTION_SD_V_Table]);
	
	fwif_color_set_scaledown444To422(0,m_defaultVipTable.VIP_QUALITY_Extend2_Coef[quick_show_src_idx][VIP_QUALITY_FUNCTION_SD_444To422]);



	

	drvif_color_2dpk_init(1,1);
	drvif_color_Set_Sharpness_Table((DRV_Sharpness_Table *)&QS_mode_table.qs_sharp);


//quick_show_src_idx//
	printf("\n [vpq][%s][%d] , sharpness table=%d \n", __FUNCTION__, __LINE__,m_defaultVipTable.VIP_QUALITY_Extend3_Coef[quick_show_src_idx][VIP_QUALITY_FUNCTION_SharpTable]);

	
	fwif_color_set_sharpness_level(m_defaultVipTable.VIP_QUALITY_Extend3_Coef[quick_show_src_idx][VIP_QUALITY_FUNCTION_SharpTable], QS_mode_table.qs_Store.OSD_Sharpness);


	//fwif_color_set_sharpness(50);

//	fwif_color_colorspacergb2yuvtransfer();
//	fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain();
//	fwif_color_set_YUV2RGB();

	//fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(0, display, system_info_struct);
	//Scaler_Set_ColorMapping_By_ColorFormat(channel, InputSrcGetType, nMode);

#endif
#endif
}
void fwif_color_SET_StructColorDataFacModeType_quick_show(void)
{
	int ret = 0;
	unsigned int pq_setting_len = 0;
	struct PQ_QsSettings *pq_qs_table = NULL;
	extern int get_pq_setting(void **addr, unsigned int *len);
	extern StructColorDataFacModeType* m_colorFacModeTable;


	ret = get_pq_setting((void **)&pq_qs_table, &pq_setting_len);
	if (pq_qs_table !=NULL){
		memcpy(&g_curColorFacTableEx, &pq_qs_table->defFacData, sizeof(HAL_StructColorDataFacModeType));
		g_bUseMiddleWareOSDmap=1;
	}

}
#endif /* end of quick_show */
#endif// end of UT_flag

