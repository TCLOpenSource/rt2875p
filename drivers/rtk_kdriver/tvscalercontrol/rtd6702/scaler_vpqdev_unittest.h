#ifndef __SCALER_VPQDEV_UNITTEST_H
#define  __SCALER_VPQDEV_UNITTEST_H


/**
 * Video Picture Quality Information.
 *
 * @see DDI Implementation Guide
*/
#include <scaler/vipCommon.h>
#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerDrvCommon.h>
#else
	#include <scalercommon/scalerDrvCommon.h>
#endif

#define VPQUT_IOC_MAGIC 'P'

enum VPQUT_IOC_pq_cmd{
	VPQUT_IOC_PQ_CMD_INIT = 0,
	VPQUT_IOC_PQ_CMD_REG = 1,
	VPQUT_IOC_PQ_CMD_REG_BIT = 2,
	VPQUT_IOC_PQ_CMD_GET_MEM_INFO = 3,
	VPQUT_IOC_PQ_CMD_SET_DI_MEM = 4,
	VPQUT_IOC_PQ_CMD_GET_DI_MEM = 5,
	VPQUT_IOC_PQ_CMD_GET_VD_MEM_INFO = 6,
	VPQUT_IOC_PQ_CMD_SET_VD_MEM = 7,
	VPQUT_IOC_PQ_CMD_GET_VD_MEM = 8,
	VPQUT_IOC_PQ_CMD_SET_TABLE_SIZE = 9,
	VPQUT_IOC_PQ_CMD_SET_Run_Setup_IMD = 10,

	VPQUT_IOC_PQ_CMD_SET_m_defaultYPbPrGainOffsetData = 50,
	VPQUT_IOC_PQ_CMD_SET_m_defaultVgaGainOffset = 51,
	VPQUT_IOC_PQ_CMD_SET_m_defaultColorDataTable = 52,
	VPQUT_IOC_PQ_CMD_SET_m_defaultColorFacTable = 53,
	VPQUT_IOC_PQ_CMD_SET_m_defaultPictureModeTable = 54,
	VPQUT_IOC_PQ_CMD_SET_m_defaultVipTable = 55,
	VPQUT_IOC_PQ_CMD_SET_m_defaultColorTempTable = 56,
	VPQUT_IOC_PQ_CMD_SET_m_customVipTable = 57,
	VPQUT_IOC_PQ_CMD_SET_m_customVipTable_demo_tv002 = 58,

	VPQUT_IOC_PQ_CMD_SET_S_RPC_AutoMA_Flag = 100,
	VPQUT_IOC_PQ_CMD_SET_tVipNewDcti_auto_adjust = 101,
	VPQUT_IOC_PQ_CMD_SET_gVipDNewDcti_Table = 102,
	VPQUT_IOC_PQ_CMD_SET_gVipINewDcti_Table = 103,
	VPQUT_IOC_PQ_CMD_SET_gVipVcti_Table = 104,
	VPQUT_IOC_PQ_CMD_SET_gVipVcti_lpf_Table = 105,
	VPQUT_IOC_PQ_CMD_SET_gVipUV_Delay_TOP_Table = 106,
	VPQUT_IOC_PQ_CMD_SET_g_Chroma_Compensation_Curve = 107,
	VPQUT_IOC_PQ_CMD_SET_Dnoise_Coef = 108,
	VPQUT_IOC_PQ_CMD_SET_Dnoise_Level = 109,
	VPQUT_IOC_PQ_CMD_SET_SUFIR256_CEC1x00_Hamm = 110,
	VPQUT_IOC_PQ_CMD_SET_SUFIR256_CEP0x75 = 111,
	VPQUT_IOC_PQ_CMD_SET_SUFIR256_PM41x5_32 = 112,
	VPQUT_IOC_PQ_CMD_SET_SUFIR256_PM46_32 = 113,
	VPQUT_IOC_PQ_CMD_SET_SUFIR256_PM56_32 = 114,
	VPQUT_IOC_PQ_CMD_SET_SUFIR256_CLS0x80 = 115,
	VPQUT_IOC_PQ_CMD_SET_SUFIR256_Sinc3z_Hamm = 116,
	VPQUT_IOC_PQ_CMD_SET_SUFIRcls256_96s2_45 = 117,
	VPQUT_IOC_PQ_CMD_SET_SU_table_8tap = 118,
	VPQUT_IOC_PQ_CMD_SET_SUFIR192_Sinc2x6_Hamm = 119,
	VPQUT_IOC_PQ_CMD_SET_SUFIR192_PM52_32 = 120,
	VPQUT_IOC_PQ_CMD_SET_SUFIR192_Sinc2x4_Hamm = 121,
	VPQUT_IOC_PQ_CMD_SET_SUFIR192_PM56_32 = 122,
	VPQUT_IOC_PQ_CMD_SET_SUFIR192_PM52_36 = 123,
	VPQUT_IOC_PQ_CMD_SET_SUFIR192_Sinc2x2_Hamm = 124,
	VPQUT_IOC_PQ_CMD_SET_SUFIR192_Hann0x70 = 125,
	VPQUT_IOC_PQ_CMD_SET_SUFIRcls192_84s2_55 = 126,
	VPQUT_IOC_PQ_CMD_SET_SU_table_6tap = 127,
	VPQUT_IOC_PQ_CMD_SET_gVipDirsu_Table = 128,
	VPQUT_IOC_PQ_CMD_SET_gSMDtable = 129,
	VPQUT_IOC_PQ_CMD_SET_gHMETable = 130,
	VPQUT_IOC_PQ_CMD_SET_gHMCTable = 131,
	VPQUT_IOC_PQ_CMD_SET_gPanTable = 132,
	VPQUT_IOC_PQ_CMD_SET_gFilmInitTable = 133,
	VPQUT_IOC_PQ_CMD_SET_gVipIntra_Set_Ver2_tab = 134,
	VPQUT_IOC_PQ_CMD_SET_gUVCTable = 135,
	VPQUT_IOC_PQ_CMD_SET_gBSTable = 136,
	VPQUT_IOC_PQ_CMD_SET_gVipSupk_Mask = 137,
	VPQUT_IOC_PQ_CMD_SET_gVipSkipir_Ring = 138,
	VPQUT_IOC_PQ_CMD_SET_gVipSkipir_Islet = 139,
	VPQUT_IOC_PQ_CMD_SET_gVipEmfMk2 = 140,
	VPQUT_IOC_PQ_CMD_SET_gVip2Dpk_Seg = 141,
	VPQUT_IOC_PQ_CMD_SET_gVip2Dpk_Tex_Detect = 142,
	VPQUT_IOC_PQ_CMD_SET_gVipOSD_sharp = 143,
	VPQUT_IOC_PQ_CMD_SET_D_DLTI_Table = 144,
	VPQUT_IOC_PQ_CMD_SET_gVipUn_Shp_Mask = 145,
	VPQUT_IOC_PQ_CMD_SET_gCMTable = 146,
	VPQUT_IOC_PQ_CMD_SET_Curve_boundary_table = 147,
	VPQUT_IOC_PQ_CMD_SET_Advance_control_table = 148,
	VPQUT_IOC_PQ_CMD_SET_sRGB_Hue_Mapping = 149,
	VPQUT_IOC_PQ_CMD_SET_sRGB_RGB_LUT = 150,
	VPQUT_IOC_PQ_CMD_SET_sRGB_YUV2RGB = 151,
	VPQUT_IOC_PQ_CMD_SET_sRGB_RGB2YUV = 152,
	VPQUT_IOC_PQ_CMD_SET_sRGB_APPLY = 153,
	VPQUT_IOC_PQ_CMD_SET_YUV2RGB_Matrix_COEF_601_YCbCr = 154,
	VPQUT_IOC_PQ_CMD_SET_tYUV2RGB_COEF_TVBOX_SD = 155,
	VPQUT_IOC_PQ_CMD_SET_tYUV2RGB_COEF_TVBOX_HD = 156,
	VPQUT_IOC_PQ_CMD_SET_tYUV2RGB_COEF_601_YCbCr_NOClampY = 157,
	VPQUT_IOC_PQ_CMD_SET_tYUV2RGB_COEF = 158,
	VPQUT_IOC_PQ_CMD_SET_ConBriMapping_Offset_Table = 159,
	VPQUT_IOC_PQ_CMD_SET_AVBlackLevelMapping_Table = 160,
	VPQUT_IOC_PQ_CMD_SET_ATVBlackLevelMapping_Table = 161,
	VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_709_YUV_0_255 = 162,
	VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_709_YUV_16_235 = 163,
	VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_709_RGB_0_255 = 164,
	VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_709_RGB_16_235 = 165,
	VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_601_RGB_0_255 = 166,
	VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_601_RGB_16_235 = 167,
	VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_601_YCbCr_0_255 = 168,
	VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_601_YCbCr_16_235 = 169,
	VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_601_YCbCr_16_255 = 170,
	VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_TVBOX_SD = 171,
	VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_TVBOX_720 = 172,
	VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_TVBOX_1080 = 173,
	VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF = 174,
	VPQUT_IOC_PQ_CMD_SET_Local_Dimming_Table = 175,
	VPQUT_IOC_PQ_CMD_SET_LD_Backlight_Profile_Interpolation_table = 176,
	VPQUT_IOC_PQ_CMD_SET_LD_Data_Compensation_NewMode_2DTable = 177,
	VPQUT_IOC_PQ_CMD_SET_Local_Contrast_Table = 178,
	VPQUT_IOC_PQ_CMD_SET_LC_Backlight_Profile_Interpolation_table = 179,
	VPQUT_IOC_PQ_CMD_SET_LC_ToneMappingSlopePoint_Table = 180,
	VPQUT_IOC_PQ_CMD_SET_DI_MA_Init_Table = 181,
	VPQUT_IOC_PQ_CMD_SET_DI_MA_Adjust_Table = 182,
	VPQUT_IOC_PQ_CMD_SET_di_TNR_XC_table = 183,
	VPQUT_IOC_PQ_CMD_SET_Scaler_DI_Coef_table = 184,
	VPQUT_IOC_PQ_CMD_SET_Adaptive_Gamma_Ctrl_Table = 185,
	VPQUT_IOC_PQ_CMD_SET_OD_table = 186,
	VPQUT_IOC_PQ_CMD_SET_OD_setting_table = 187,
	VPQUT_IOC_PQ_CMD_SET_FIR_Coef_Ctrl = 188,
	VPQUT_IOC_PQ_CMD_SET_TwoStepSU_Table = 189,
	VPQUT_IOC_PQ_CMD_SET_ScaleDown_COEF_TAB = 190,
	VPQUT_IOC_PQ_CMD_SET_PCID_table = 191,
	VPQUT_IOC_PQ_CMD_SET_PCID2_Table = 192,
	VPQUT_IOC_PQ_CMD_SET_PCID2_Pol_Table_R = 193,
	VPQUT_IOC_PQ_CMD_SET_PCID2_Pol_Table_G = 194,
	VPQUT_IOC_PQ_CMD_SET_PCID2_Pol_Table_B = 195,
	VPQUT_IOC_PQ_CMD_SET_pq_misc_MA_SNR_IESM_TBL = 196,
	VPQUT_IOC_PQ_CMD_SET_t_inv_GAMMA = 197,
	VPQUT_IOC_PQ_CMD_SET_t_gamma_curve_22 = 198,
	VPQUT_IOC_PQ_CMD_SET_Power1div22Gamma = 199,
	VPQUT_IOC_PQ_CMD_SET_Power22InvGamma = 200,
	VPQUT_IOC_PQ_CMD_SET_LinearInvGamma = 201,
	VPQUT_IOC_PQ_CMD_SET_LEDOutGamma = 202,
	VPQUT_IOC_PQ_CMD_SET_gVip_Profile_Table = 203,
	VPQUT_IOC_PQ_CMD_SET_PQ_ByPass_Struct = 204,
	VPQUT_IOC_PQ_CMD_SET_PQ_check_source_table = 205,
	VPQUT_IOC_PQ_CMD_SET_PQ_check_register_table = 206,
	VPQUT_IOC_PQ_CMD_SET_PQ_check_register_diff_table = 207,
	VPQUT_IOC_PQ_CMD_SET_I_De_XC_TBL = 208,
	VPQUT_IOC_PQ_CMD_SET_I_De_Contour_TBL = 209,
	VPQUT_IOC_PQ_CMD_SET_BOE_RGBW_TAB_160624 = 210,
	VPQUT_IOC_PQ_CMD_SET_BOE_RGBW_TAB_160713 = 211,
	VPQUT_IOC_PQ_CMD_SET_Output_InvOutput_Gamma = 212,
	VPQUT_IOC_PQ_CMD_SET_hdr_table = 213,
	VPQUT_IOC_PQ_CMD_SET_hdr_YUV2RGB = 214,
	VPQUT_IOC_PQ_CMD_SET_hdr_RGB2OPT = 215,
	VPQUT_IOC_PQ_CMD_SET_HDR10_3DLUT_17x17x17 = 216,
	VPQUT_IOC_PQ_CMD_SET_EOTF_LUT_HLG_DEFAULT = 217,
	VPQUT_IOC_PQ_CMD_SET_OETF_LUT_HLG_DEFAULT = 218,
	VPQUT_IOC_PQ_CMD_SET_EOTF_LUT_R = 219,
	VPQUT_IOC_PQ_CMD_SET_OETF_LUT_R = 220,
	VPQUT_IOC_PQ_CMD_SET_Tone_Mapping_LUT_R = 221,
	VPQUT_IOC_PQ_CMD_SET_HDR10_3DLUT_24x24x24 = 222,
	VPQUT_IOC_PQ_CMD_SET_HLG_3DLUT_24x24x24 = 223,
	VPQUT_IOC_PQ_CMD_SET_HLG_OETF_LUT_R = 224,
	VPQUT_IOC_PQ_CMD_SET_HLG_EOTF_LUT_R = 225,
	VPQUT_IOC_PQ_CMD_SET_TCHDR_Table = 226,
	VPQUT_IOC_PQ_CMD_SET_TCHDR_COEF_C_LUT_TBL = 227,
	VPQUT_IOC_PQ_CMD_SET_TCHDR_COEF_I_LUT_TBL = 228,
	VPQUT_IOC_PQ_CMD_SET_TCHDR_COEF_S_LUT_TBL = 229,
	VPQUT_IOC_PQ_CMD_SET_TCHDR_COEF_P_LUT_TBL = 230,
	VPQUT_IOC_PQ_CMD_SET_TCHDR_COEF_D_LUT_TBL = 231,
};

unsigned int andREG_Table1[32]={
    0xffffffff, 0xfffffffe, 0xfffffffc, 0xfffffff8,
    0xfffffff0, 0xffffffe0, 0xffffffc0, 0xffffff80,
    0xffffff00, 0xfffffe00, 0xfffffc00, 0xfffff800,
    0xfffff000, 0xffffe000, 0xffffc000, 0xffff8000,
    0xffff0000, 0xfffe0000, 0xfffc0000, 0xfff80000,
    0xfff00000, 0xffe00000, 0xffc00000, 0xff800000,
    0xff000000, 0xfe000000, 0xfc000000, 0xf8000000,
    0xf0000000, 0xe0000000, 0xc0000000, 0x80000000
    };

unsigned int andREG_Table2[32]={
    0x00000001, 0x00000003, 0x00000007, 0x0000000f,
    0x0000001f, 0x0000003f, 0x0000007f, 0x000000ff,
    0x000001ff, 0x000003ff, 0x000007ff, 0x00000fff,
    0x00001fff, 0x00003fff, 0x00007fff, 0x0000ffff,
    0x0001ffff, 0x0003ffff, 0x0007ffff, 0x000fffff,
    0x001fffff, 0x003fffff, 0x007fffff, 0x00ffffff,
    0x01ffffff, 0x03ffffff, 0x07ffffff, 0x0fffffff,
    0x1fffffff, 0x3fffffff, 0x7fffffff, 0xffffffff
    };

typedef struct {
	DRV_RPC_AutoMA_Flag S_RPC_AutoMA_Flag;
	DRV_VipNewDcti_auto_adjust tVipNewDcti_auto_adjust[DCTI_TABLE_LEVEL_MAX];
} SLR_VIP_MISC_TABLE;

typedef struct _VPQUT_REG_struct {
	char type;
	unsigned int reg;
	unsigned int bit[2];
	unsigned int value;
}VPQUT_REG_struct;

typedef struct _VPQC_FILE_struct {
	unsigned int id;
	unsigned int index;
	unsigned int size;
	unsigned int crc;
	unsigned int array_index;
}VPQC_FILE_struct;

#define VPQUT_IOC_INIT						_IO(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_INIT)
#define VPQUT_IOC_REG						_IOWR(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_REG, int)
#define VPQUT_IOC_REG_BIT					_IOWR(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_REG_BIT, int)
#define VPQUT_IOC_GET_MEM_INFO				_IOR(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_GET_MEM_INFO, int)
#define VPQUT_IOC_SET_DI_MEM				_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_DI_MEM, int)
#define VPQUT_IOC_GET_DI_MEM				_IOR(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_GET_DI_MEM, int)
#define VPQUT_IOC_GET_VD_MEM_INFO			_IOR(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_GET_VD_MEM_INFO, int)
#define VPQUT_IOC_SET_VD_MEM			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_VD_MEM, int)
#define VPQUT_IOC_GET_VD_MEM			_IOR(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_GET_VD_MEM, int)
#define VPQUT_IOC_SET_TABLE_SIZE			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_TABLE_SIZE, int)
#define VPQUT_IOC_SET_m_defaultVipTable			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_m_defaultVipTable, int)
#define VPQUT_IOC_SET_Run_Setup_IMD		_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_Run_Setup_IMD, int)

#define VPQUT_IOC_SET_S_RPC_AutoMA_Flag			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_S_RPC_AutoMA_Flag, int)
#define VPQUT_IOC_SET_tVipNewDcti_auto_adjust			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_tVipNewDcti_auto_adjust, int)
#define VPQUT_IOC_SET_gVipDNewDcti_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gVipDNewDcti_Table, int)
#define VPQUT_IOC_SET_gVipINewDcti_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gVipINewDcti_Table, int)
#define VPQUT_IOC_SET_gVipVcti_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gVipVcti_Table, int)
#define VPQUT_IOC_SET_gVipVcti_lpf_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gVipVcti_lpf_Table, int)
#define VPQUT_IOC_SET_gVipUV_Delay_TOP_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gVipUV_Delay_TOP_Table, int)
#define VPQUT_IOC_SET_g_Chroma_Compensation_Curve			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_g_Chroma_Compensation_Curve, int)
#define VPQUT_IOC_SET_Dnoise_Coef			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_Dnoise_Coef, int)
#define VPQUT_IOC_SET_Dnoise_Level			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_Dnoise_Level, int)
#define VPQUT_IOC_SET_SU_table_8tap			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_SU_table_8tap, int)
#define VPQUT_IOC_SET_SU_table_6tap			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_SU_table_6tap, int)
#define VPQUT_IOC_SET_gVipDirsu_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gVipDirsu_Table, int)
#define VPQUT_IOC_SET_gSMDtable			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gSMDtable, int)
#define VPQUT_IOC_SET_gHMETable			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gHMETable, int)
#define VPQUT_IOC_SET_gHMCTable			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gHMCTable, int)
#define VPQUT_IOC_SET_gPanTable			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gPanTable, int)
#define VPQUT_IOC_SET_gFilmInitTable			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gFilmInitTable, int)
#define VPQUT_IOC_SET_gVipIntra_Set_Ver2_tab			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gVipIntra_Set_Ver2_tab, int)
#define VPQUT_IOC_SET_gUVCTable			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gUVCTable, int)
#define VPQUT_IOC_SET_gBSTable			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gBSTable, int)
#define VPQUT_IOC_SET_gVipSupk_Mask			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gVipSupk_Mask, int)
#define VPQUT_IOC_SET_gVipSkipir_Ring			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gVipSkipir_Ring, int)
#define VPQUT_IOC_SET_gVipSkipir_Islet			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gVipSkipir_Islet, int)
#define VPQUT_IOC_SET_gVipEmfMk2			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gVipEmfMk2, int)
#define VPQUT_IOC_SET_gVip2Dpk_Seg			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gVip2Dpk_Seg, int)
#define VPQUT_IOC_SET_gVip2Dpk_Tex_Detect			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gVip2Dpk_Tex_Detect, int)
#define VPQUT_IOC_SET_gVipOSD_sharp			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gVipOSD_sharp, int)
#define VPQUT_IOC_SET_D_DLTI_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_D_DLTI_Table, int)
#define VPQUT_IOC_SET_gVipUn_Shp_Mask			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gVipUn_Shp_Mask, int)
#define VPQUT_IOC_SET_gCMTable			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gCMTable, int)
#define VPQUT_IOC_SET_Curve_boundary_table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_Curve_boundary_table, int)
#define VPQUT_IOC_SET_Advance_control_table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_Advance_control_table, int)
#define VPQUT_IOC_SET_sRGB_Hue_Mapping			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_sRGB_Hue_Mapping, int)
#define VPQUT_IOC_SET_sRGB_RGB_LUT			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_sRGB_RGB_LUT, int)
#define VPQUT_IOC_SET_sRGB_YUV2RGB			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_sRGB_YUV2RGB, int)
#define VPQUT_IOC_SET_sRGB_RGB2YUV			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_sRGB_RGB2YUV, int)
#define VPQUT_IOC_SET_sRGB_APPLY			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_sRGB_APPLY, int)
#define VPQUT_IOC_SET_YUV2RGB_Matrix_COEF_601_YCbCr			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_YUV2RGB_Matrix_COEF_601_YCbCr, int)
#define VPQUT_IOC_SET_tYUV2RGB_COEF_TVBOX_SD			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_tYUV2RGB_COEF_TVBOX_SD, int)
#define VPQUT_IOC_SET_tYUV2RGB_COEF_TVBOX_HD			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_tYUV2RGB_COEF_TVBOX_HD, int)
#define VPQUT_IOC_SET_tYUV2RGB_COEF_601_YCbCr_NOClampY			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_tYUV2RGB_COEF_601_YCbCr_NOClampY, int)
#define VPQUT_IOC_SET_tYUV2RGB_COEF			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_tYUV2RGB_COEF, int)
#define VPQUT_IOC_SET_ConBriMapping_Offset_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_ConBriMapping_Offset_Table, int)
#define VPQUT_IOC_SET_AVBlackLevelMapping_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_AVBlackLevelMapping_Table, int)
#define VPQUT_IOC_SET_ATVBlackLevelMapping_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_ATVBlackLevelMapping_Table, int)
#define VPQUT_IOC_SET_tRGB2YUV_COEF_709_YUV_0_255			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_709_YUV_0_255, int)
#define VPQUT_IOC_SET_tRGB2YUV_COEF_709_YUV_16_235			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_709_YUV_16_235, int)
#define VPQUT_IOC_SET_tRGB2YUV_COEF_709_RGB_0_255			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_709_RGB_0_255, int)
#define VPQUT_IOC_SET_tRGB2YUV_COEF_709_RGB_16_235			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_709_RGB_16_235, int)
#define VPQUT_IOC_SET_tRGB2YUV_COEF_601_RGB_0_255			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_601_RGB_0_255, int)
#define VPQUT_IOC_SET_tRGB2YUV_COEF_601_RGB_16_235			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_601_RGB_16_235, int)
#define VPQUT_IOC_SET_tRGB2YUV_COEF_601_YCbCr_0_255			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_601_YCbCr_0_255, int)
#define VPQUT_IOC_SET_tRGB2YUV_COEF_601_YCbCr_16_235			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_601_YCbCr_16_235, int)
#define VPQUT_IOC_SET_tRGB2YUV_COEF_601_YCbCr_16_255			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_601_YCbCr_16_255, int)
#define VPQUT_IOC_SET_tRGB2YUV_COEF_TVBOX_SD			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_TVBOX_SD, int)
#define VPQUT_IOC_SET_tRGB2YUV_COEF_TVBOX_720			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_TVBOX_720, int)
#define VPQUT_IOC_SET_tRGB2YUV_COEF_TVBOX_1080			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF_TVBOX_1080, int)
#define VPQUT_IOC_SET_tRGB2YUV_COEF			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF, int)
#define VPQUT_IOC_SET_Local_Dimming_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_Local_Dimming_Table, int)
#define VPQUT_IOC_SET_LD_Backlight_Profile_Interpolation_table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_LD_Backlight_Profile_Interpolation_table, int)
#define VPQUT_IOC_SET_LD_Data_Compensation_NewMode_2DTable			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_LD_Data_Compensation_NewMode_2DTable, int)
#define VPQUT_IOC_SET_Local_Contrast_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_Local_Contrast_Table, int)
#define VPQUT_IOC_SET_LC_Backlight_Profile_Interpolation_table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_LC_Backlight_Profile_Interpolation_table, int)
#define VPQUT_IOC_SET_LC_ToneMappingSlopePoint_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_LC_ToneMappingSlopePoint_Table, int)
#define VPQUT_IOC_SET_DI_MA_Init_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_DI_MA_Init_Table, int)
#define VPQUT_IOC_SET_DI_MA_Adjust_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_DI_MA_Adjust_Table, int)
#define VPQUT_IOC_SET_di_TNR_XC_table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_di_TNR_XC_table, int)
#define VPQUT_IOC_SET_Scaler_DI_Coef_table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_Scaler_DI_Coef_table, int)
#define VPQUT_IOC_SET_Adaptive_Gamma_Ctrl_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_Adaptive_Gamma_Ctrl_Table, int)
#define VPQUT_IOC_SET_OD_table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_OD_table, int)
#define VPQUT_IOC_SET_OD_setting_table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_OD_setting_table, int)
#define VPQUT_IOC_SET_FIR_Coef_Ctrl			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_FIR_Coef_Ctrl, int)
#define VPQUT_IOC_SET_TwoStepSU_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_TwoStepSU_Table, int)
#define VPQUT_IOC_SET_ScaleDown_COEF_TAB			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_ScaleDown_COEF_TAB, int)
#define VPQUT_IOC_SET_PCID_table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_PCID_table, int)
#define VPQUT_IOC_SET_PCID2_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_PCID2_Table, int)
#define VPQUT_IOC_SET_PCID2_Pol_Table_R			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_PCID2_Pol_Table_R, int)
#define VPQUT_IOC_SET_PCID2_Pol_Table_G			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_PCID2_Pol_Table_G, int)
#define VPQUT_IOC_SET_PCID2_Pol_Table_B			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_PCID2_Pol_Table_B, int)
#define VPQUT_IOC_SET_pq_misc_MA_SNR_IESM_TBL			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_pq_misc_MA_SNR_IESM_TBL, int)
#define VPQUT_IOC_SET_t_inv_GAMMA			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_t_inv_GAMMA, int)
#define VPQUT_IOC_SET_t_gamma_curve_22			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_t_gamma_curve_22, int)
#define VPQUT_IOC_SET_Power1div22Gamma			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_Power1div22Gamma, int)
#define VPQUT_IOC_SET_Power22InvGamma			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_Power22InvGamma, int)
#define VPQUT_IOC_SET_LinearInvGamma			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_LinearInvGamma, int)
#define VPQUT_IOC_SET_LEDOutGamma			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_LEDOutGamma, int)
#define VPQUT_IOC_SET_gVip_Profile_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_gVip_Profile_Table, int)
#define VPQUT_IOC_SET_PQ_ByPass_Struct			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_PQ_ByPass_Struct, int)
#define VPQUT_IOC_SET_PQ_check_source_table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_PQ_check_source_table, int)
#define VPQUT_IOC_SET_PQ_check_register_table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_PQ_check_register_table, int)
#define VPQUT_IOC_SET_PQ_check_register_diff_table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_PQ_check_register_diff_table, int)
#define VPQUT_IOC_SET_I_De_XC_TBL			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_I_De_XC_TBL, int)
#define VPQUT_IOC_SET_I_De_Contour_TBL			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_I_De_Contour_TBL, int)
#define VPQUT_IOC_SET_BOE_RGBW_TAB_160624			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_BOE_RGBW_TAB_160624, int)
#define VPQUT_IOC_SET_BOE_RGBW_TAB_160713			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_BOE_RGBW_TAB_160713, int)
#define VPQUT_IOC_SET_Output_InvOutput_Gamma			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_Output_InvOutput_Gamma, int)
#define VPQUT_IOC_SET_hdr_table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_hdr_table, int)
#define VPQUT_IOC_SET_hdr_YUV2RGB			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_hdr_YUV2RGB, int)
#define VPQUT_IOC_SET_hdr_RGB2OPT			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_hdr_RGB2OPT, int)
#define VPQUT_IOC_SET_HDR10_3DLUT_17x17x17			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_HDR10_3DLUT_17x17x17, int)
#define VPQUT_IOC_SET_EOTF_LUT_HLG_DEFAULT			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_EOTF_LUT_HLG_DEFAULT, int)
#define VPQUT_IOC_SET_OETF_LUT_HLG_DEFAULT			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_OETF_LUT_HLG_DEFAULT, int)
#define VPQUT_IOC_SET_EOTF_LUT_R			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_EOTF_LUT_R, int)
#define VPQUT_IOC_SET_OETF_LUT_R			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_OETF_LUT_R, int)
#define VPQUT_IOC_SET_Tone_Mapping_LUT_R			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_Tone_Mapping_LUT_R, int)
#define VPQUT_IOC_SET_HDR10_3DLUT_24x24x24			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_HDR10_3DLUT_24x24x24, int)
#define VPQUT_IOC_SET_HLG_3DLUT_24x24x24			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_HLG_3DLUT_24x24x24, int)
#define VPQUT_IOC_SET_HLG_OETF_LUT_R			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_HLG_OETF_LUT_R, int)
#define VPQUT_IOC_SET_HLG_EOTF_LUT_R			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_HLG_EOTF_LUT_R, int)
#define VPQUT_IOC_SET_TCHDR_Table			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_TCHDR_Table, int)
#define VPQUT_IOC_SET_TCHDR_COEF_C_LUT_TBL			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_TCHDR_COEF_C_LUT_TBL, int)
#define VPQUT_IOC_SET_TCHDR_COEF_I_LUT_TBL			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_TCHDR_COEF_I_LUT_TBL, int)
#define VPQUT_IOC_SET_TCHDR_COEF_S_LUT_TBL			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_TCHDR_COEF_S_LUT_TBL, int)
#define VPQUT_IOC_SET_TCHDR_COEF_P_LUT_TBL			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_TCHDR_COEF_P_LUT_TBL, int)
#define VPQUT_IOC_SET_TCHDR_COEF_D_LUT_TBL			_IOW(VPQUT_IOC_MAGIC, VPQUT_IOC_PQ_CMD_SET_TCHDR_COEF_D_LUT_TBL, int)

void vpqc_do_suspend(void);
void vpqc_do_resume(void);
void vpqc_do_resume_instanboot(void);
unsigned char vpqc_get_handler_bypass(void);

int vpqut_load_vpqtable(struct file *table_file, VPQC_FILE_struct *array_info);
int VPQ_GenFile_setfile_data(struct file *table_file, VPQC_FILE_struct *file_info, unsigned int id, unsigned char* array , unsigned int size);
#endif