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
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
//#include <mach/RPCDriver.h>
#include <linux/pageremap.h>
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
#endif

#ifdef CONFIG_ARM64
#include <asm/io.h>
#endif

/*some include about AP*/
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/scaler/scalertimer.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/vip/GSR2.h> 

/*some include about VIP Driver*/
/*#include <tvscalercontrol/vip/scalerColor.h>*/
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>
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
#include <tvscalercontrol/vdc/video.h>
#include <tvscalercontrol/vip/localdimming.h>
#include <tvscalercontrol/vip/localcontrast.h>
#include <tvscalercontrol/vip/fcic.h>
#include <tvscalercontrol/vip/ST2094.h>
#include <rbus/timer_reg.h>
#include <tvscalercontrol/panel/panelapi.h>
#include <tvscalercontrol/i3ddma/i3ddma_drv.h>
/*some include about scaler*/
#include <mach/rtk_log.h>

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerDrvCommon.h>
#else
#include <scalercommon/scalerDrvCommon.h>
#endif

#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
/*some include about rbus*/
/*some include about device driver*/
#include <tvscalercontrol/hdmirx/hdmifun.h>
//#include <tvscalercontrol/hdmirx/hdmiInternal.h>
#include <rtd_log/rtd_module_log.h>
//#include <mach/io.h>
#include <rbus/hdr_all_top_reg.h>
#include <tvscalercontrol/vip/ai_pq.h>
#include <rbus/osdovl_reg.h>
#include <rbus/blu_reg.h>



// for register dump
#include <tvscalercontrol/io/ioregdrv.h>
#undef rtd_outl
#define rtd_outl(x, y)     								IoReg_Write32(x,y)
#undef rtd_inl
#define rtd_inl(x)     									IoReg_Read32(x)
#undef rtd_maskl
#define rtd_maskl(x, y, z)     							IoReg_Mask32(x,y,z)
#undef rtd_setbits
#define rtd_setbits(offset, Mask) rtd_outl(offset, (rtd_inl(offset) | Mask))
#undef rtd_clearbits
#define rtd_clearbits(offset, Mask) rtd_outl(offset, ((rtd_inl(offset) & ~(Mask))))
#define TAG_NAME "VPQ"
//#define VIPprintf(fmt, args...)	printk(KERN_DEBUG fmt, ##args)	//20150822 roger mark for image release
#define VIPprintf(fmt, args...)
#define	vip_malloc(x)	kmalloc(x, GFP_KERNEL)
#define	vip_free(x)	kfree(x)
extern unsigned int vpq_project_id; /*pre_id:16 main_id:8 sub1_id:8 sub2_id:8, example->lg=tv006*/
extern struct semaphore VPQ_DM_DMA_TBL_Semaphore;

#define Monotonic_Check 0

static RGB2YUV_BLACK_LEVEL rgb2yuv_bl_tv006 = RGB2YUV_BLACK_LEVEL_AUTO;
RTK_NOISE_REDUCTION_T *pNR_Table_tv006;
RTK_MPEG_NOISE_REDUCTION_T *pMPEGNR_Table_tv006;
extern unsigned short tYUV2RGB_COEF[YUV2RGB_TBL_SELECT_ITEM_MAX][tUV2RGB_COEF_Items_Max];

extern UINT16 HDR10_3DLUT_24x24x24[HDR_24x24x24_size][3];
extern UINT32 EOTF_LUT_R[1025];
extern UINT16 OETF_LUT_R[1025];
extern unsigned short OETF_LUT_ByMaxCLL_ByPanelLuminance[1025];
extern unsigned short HLG_3DLUT_24x24x24[HDR_24x24x24_size][3];
extern unsigned int HLG_EOTF_LUT_R[EOTF_size];
extern unsigned short HLG_OETF_LUT_R[OETF_size];

extern unsigned short Power22InvGamma[1025];
extern unsigned short LinearInvGamma[1025];
unsigned char bCinemaModeEn;
unsigned char bNr_Input=0;
extern unsigned char get_vsc_run_adaptive_stream(unsigned char display);
extern RTK_NOISE_REDUCTION_T NR_Level;

unsigned short OutGamma_4CHDGA_R[1025];
unsigned short OutGamma_4CHDGA_G[1025];
unsigned short OutGamma_4CHDGA_B[1025];
unsigned short OutGamma_4CHDGA_W[1025];

unsigned int OutGamma_PTG_R[512];
unsigned int OutGamma_PTG_G[512];
unsigned int OutGamma_PTG_B[512];
unsigned int OutGamma_PTG_W[512];

unsigned int OutGamma_Ori_R[512];
unsigned int OutGamma_Ori_G[512];
unsigned int OutGamma_Ori_B[512];
unsigned int OutGamma_Ori_W[512];

UINT8 TNR_Y_Store_Mad[15][10];
UINT8 TNR_C_Store_Mad[7][10];

unsigned char bBFI_InputFrameRateEn = 0;
unsigned char bBFI_UIEn = 0;
unsigned char bBFI_En = 0;

extern unsigned int final_OutGAMMA_R_d0d1[Bin_Num_Gamma / 2], final_OutGAMMA_G_d0d1[Bin_Num_Gamma / 2], final_OutGAMMA_B_d0d1[Bin_Num_Gamma / 2],final_OutGAMMA_W_d0d1[Bin_Num_Gamma / 2];
extern unsigned int final_OutGAMMA_R_index[Bin_Num_Gamma / 2], final_OutGAMMA_G_index[Bin_Num_Gamma / 2], final_OutGAMMA_B_index[Bin_Num_Gamma / 2],final_OutGAMMA_W_index[Bin_Num_Gamma / 2];

#if 0
unsigned int LD_size = 0;
unsigned char * LD_virAddr = NULL;
char linedata[961] = {0};	/* 120 byte + '\n'*/
#endif

// Merlin4
#define MplusRegSize 928
#define SiwRegSize 256

//extern UINT32 EOTF_LUT_HLG_DEFAULT[1025];
//extern UINT16 OETF_LUT_HLG_DEFAULT_DCC_OFF[1025];
extern unsigned int PQModeInfo_flag[5];
//unsigned int Ori_PQA_Table[PQA_TABLE_MAX][PQA_MODE_MAX][PQA_ITEM_MAX][PQA_LEVEL_MAX];

#ifdef VPQ_COMPILER_ERROR_ENABLE_MERLIN6
//extern webos_info_t  webos_tooloption;
extern webos_strInfo_t webos_strToolOption;
#endif

#if IS_ENABLED(CONFIG_RTK_AI_DRV)
// for scene_detection
extern DRV_AI_SCENE_Ctrl_table ai_scene_ctrl;
#endif

void fwif_color_set_rgb2yuv_black_level_tv006(RGB2YUV_BLACK_LEVEL lv)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;

	rgb2yuv_bl_tv006 = lv;

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	VIP_system_info_structure_table->OSD_Info.OSD_Film_Mode = lv; // borrow film mode flag, transfer black level to video f/w


}

RGB2YUV_BLACK_LEVEL fwif_color_get_rgb2yuv_black_level_tv006(void)
{
	return rgb2yuv_bl_tv006;
}

unsigned char fwif_color_get_colorspacergb2yuvtransfer_table_tv006(unsigned char channel, unsigned char nSrcType, unsigned char nSD_HD, bool bUse_HDMI_Range_Detect)
{
	unsigned char video_format;
	/*unsigned short *table_index = 0;*/
	unsigned char table_num;
	unsigned char CHOOSE_DATA_RANGE = 1;
	UINT8 data_range = 0; /*morgan_shen  20121112  choose rgb2yuv table by the flag of data range.*/
	RGB2YUV_BLACK_LEVEL blk_lv = fwif_color_get_rgb2yuv_black_level_tv006();
	UINT8 isBt2020_Vo =0, isBt2020_HDMI = 0;
	UINT8 isHDMIFullRangeYCC = 0;
	SLR_VOINFO* pVOInfo = NULL;
	/*pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());*/

	HDMI_AVI_T pAviInfo={0};
	/*drvif_Hdmi_GetAviInfoFrame((unsigned char*) (&pAviInfo)); //pAviInfo size = 16 bytes*/

	extern unsigned char RGB2YUV_if_RGB_mode;
	/*extern unsigned short tRGB2YUV_COEF_YCC601_BYPASS_NORMAL_GAIN[];*/
	/*extern unsigned short tRGB2YUV_COEF_YCC709_BYPASS_NORMAL_GAIN[];*/
	/*extern unsigned short tRGB2YUV_COEF_YCC601_0_255_TO_YCC601_16_235[];*/
	/*extern unsigned short tRGB2YUV_COEF_YCC709_0_255_TO_YCC709_16_235[];*/
	/*extern unsigned short tRGB2YUV_COEF_YCC709_16_235_TO_YCC601_16_235[];*/
	/*extern unsigned short tRGB2YUV_COEF_YCC709_0_255_TO_YCC601_16_235[];*/
	/*extern unsigned short tRGB2YUV_COEF_BYPASS_NORMAL_GAIN[];*/
	/*extern unsigned short tRGB2YUV_COEF_BYPASS_0_255_TO_16_235[];*/
	/*extern unsigned short tRGB2YUV_COEF_RGB_0_255_TO_YCC601_16_235[];*/
	/*extern unsigned short tRGB2YUV_COEF_RGB_16_235_TO_YCC601_16_235[];*/
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if (RGB2YUV_BLACK_LEVEL_AUTO == blk_lv)
		bUse_HDMI_Range_Detect = true;
	else
		bUse_HDMI_Range_Detect = false;

	if (fwif_color_get_force_run_i3ddma_enable(channel)) {
		if (nSrcType == _SRC_HDMI) {
			if (drvif_IsHDMI() && drvif_Hdmi_GetColorSpace() != 0 && (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR) != _MODE_NEW))
				isHDMIFullRangeYCC = drvif_IsRGB_YUV_RANGE()==MODE_RAG_FULL?1:0;
		}
		if (nSrcType == _SRC_VGA || nSrcType == _SRC_HDMI || nSrcType == _SRC_SCART_RGB)
			nSrcType = _SRC_VO;
	}

	VIPprintf("drvif_color_get_colorspacergb2yuvtransfer_table_tv006, nSrcType = %d, nSD_HD = %d\n", nSrcType, nSD_HD);

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
			if (RGB2YUV_BLACK_LEVEL_HIGH == blk_lv) {
				if (!nSD_HD) {
					table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
				} else {
					table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
				}
			} else {
				if (!nSD_HD) {
					table_num = RGB2YUV_COEF_TV006_YPP_LOW;
					VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
				} else {
					table_num = RGB2YUV_COEF_TV006_YPP_LOW;
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
			VIPprintf(" == =  RGB2YUV video_format = %d == = 	\n", video_format);
			if (CHOOSE_DATA_RANGE) {
				data_range = drvif_IsRGB_YUV_RANGE();  /*0 :default 1 : Limited Range 2: Full Range*/
				VIPprintf(" == =  RGB2YUV  DATA_RANGE = %d  (0 :default 1 : Limited Range 2: Full Range) == =   \n", data_range);
			}
			/* get csc*/
			drvif_Hdmi_GetAviInfoFrame((HDMI_AVI_T*) (&pAviInfo)); //pAviInfo size = 16 bytes
			if (pAviInfo.EC == 5 || pAviInfo.EC == 6)
				isBt2020_HDMI = 1;
			else
				isBt2020_HDMI = 0;

			if (drvif_IsHDMI() && (Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR) != _MODE_NEW)) /*HDMI Video Mode,  gary for xoceco 20071227*/{
				if (video_format !=  0) {/*YUV444 or YUV422*/
					if (!nSD_HD) {
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
						VIPprintf("RGB2YUV_SRC_HDMI_YUV\n");
				} else {/*RGB*/

					RGB2YUV_if_RGB_mode = TRUE;
					if (RGB2YUV_BLACK_LEVEL_LOW == blk_lv) {
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

					VIPprintf("RGB2YUV_SRC_HDMI_RGB\n");
				}
			} else { /*DVI mode*/
				if ((video_format&(_BIT4|_BIT5))) { /*YUV444 or YUV422*/
					if (!nSD_HD) {
						if (RGB2YUV_BLACK_LEVEL_HIGH == blk_lv) {
							table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						} else {
							table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
						}
					} else {
						if (RGB2YUV_BLACK_LEVEL_HIGH == blk_lv) {
							table_num = RGB2YUV_COEF_BYPASS_0_255_TO_16_235;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						} else {
							table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
							VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_709_Limted;
						}
					}
					VIPprintf("RGB2YUV_SRC_DVI_YUV\n");
				} else {/*RGB*/
					RGB2YUV_if_RGB_mode = TRUE;
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
					VIPprintf("RGB2YUV_SRC_DVI_RGB\n");
				}
			}
		} else if (nSrcType == _SRC_VO) {
			/*GAME source must enter RGB to YUV*/
			/*if (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 2)*/
			pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());
			if (pVOInfo == NULL) {
				isBt2020_Vo = 0;
			} else {
				if (pVOInfo->colour_primaries == 9)	/* bt2020*/
					isBt2020_Vo = 1;
				else
					isBt2020_Vo = 0;
			}

		    if ((Scaler_DispGetInputInfo(SLR_INPUT_COLOR_SPACE) == COLOR_RGB) || (Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()) == 2)) {
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
				if (RGB2YUV_BLACK_LEVEL_HIGH == blk_lv || isHDMIFullRangeYCC) {
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
			} else {  /*Jpeg source*/
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
			}
			if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_JPEG)
				VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
		} else {
			/*which_tab = 0;*/
			VIPprintf("unhandler type%d\n", nSrcType);
			table_num = RGB2YUV_COEF_BYPASS_NORMAL_GAIN;
			VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode = YUV2RGB_INPUT_601_Limted;
			/*return NULL;*/
	  }
	rtd_pr_vpq_info("ch=%d, Src=%d, nSD_HD=%d, HDMI_Range_Detect=%d, blk_lv=%d, force_run_i3ddma_en()=%d, IsHDMI()=%d, Hdmi_ColorSpace()=%d\n",
		channel, nSrcType, nSD_HD, bUse_HDMI_Range_Detect, blk_lv, fwif_color_get_force_run_i3ddma_enable(channel), drvif_IsHDMI(), drvif_Hdmi_GetColorSpace());

	if(pVOInfo != NULL){
		rtd_pr_vpq_info("Hdmi_ModeInfo()=%d, HDMI_CSC_DataRange()=%d, IsRGB_YUV()=%d, EC=%d, colour_primaries=%d,InputInfo()=%d,VOFrom()=%d, Src()=%d\n",
			Scaler_ModeGet_Hdmi_ModeInfo(SLR_MODE_MODECURR), Scaler_GetHDMI_CSC_DataRange_Mode(), drvif_IsRGB_YUV_RANGE(), pAviInfo.EC, pVOInfo->colour_primaries,
			Scaler_DispGetInputInfo(SLR_INPUT_COLOR_SPACE), Scaler_VOFrom(Scaler_Get_CurVoInfo_plane()), Get_DisplayMode_Src(SLR_MAIN_DISPLAY));
	}


	  return table_num;

}

void fwif_color_Set_NR_Table_tv006(RTK_NOISE_REDUCTION_T *ptr)
{
	if (ptr == NULL) {
		VIPprintf("fwif_color_Set_NR_Table_tv006 error\n");
		return;
	}
	pNR_Table_tv006 = ptr;
	//VIPprintf(" == = Set_NR_Table_tv006 == = \n");

}
RTK_NOISE_REDUCTION_T *fwif_color_Get_NR_Table_tv006(void)
{
	if (pNR_Table_tv006 == NULL) {
		VIPprintf("pNR_Table_tv006 is NULL\n");
		return NULL;
	}
	return (RTK_NOISE_REDUCTION_T *)pNR_Table_tv006;

}
void fwif_color_set_DB_NR_tv006(short *ptr ) //for rtice use
{
        UINT8 i=0,j=0;

        static RTK_NOISE_REDUCTION_T NR_Table;
	static RTK_MPEG_NOISE_REDUCTION_T MPEGNR_Table;

        if(ptr == NULL)
                return;

        /* CHIP_LPFY_T */
         NR_Table.NR_TABLE.LPF_Y.Spatial_Enable_Y       = ptr[0];
         NR_Table.NR_TABLE.LPF_Y.Gain                   = ptr[1];
         NR_Table.NR_TABLE.LPF_Y.Thd_UpBnd              = ptr[2];
         NR_Table.NR_TABLE.LPF_Y.Thd_LowBnd             = ptr[3];
         NR_Table.NR_TABLE.LPF_Y.Thd_Step               = ptr[4];
         NR_Table.NR_TABLE.LPF_Y.SpatialThlY            = ptr[5];
         NR_Table.NR_TABLE.LPF_Y.SpatialThl2Y           = ptr[6];
         NR_Table.NR_TABLE.LPF_Y.SpatialThl4Y           = ptr[7];
         NR_Table.NR_TABLE.LPF_Y.WeightYSel             = ptr[8];

         /* CHIP_LPFC_T*/
         NR_Table.NR_TABLE.LPF_C.Spatial_Enable_C       = ptr[9];
         NR_Table.NR_TABLE.LPF_C.LPResultWeightC        = ptr[10];
         NR_Table.NR_TABLE.LPF_C.SpatialThlC            = ptr[11];
         NR_Table.NR_TABLE.LPF_C.SpatialThl2C           = ptr[12];
         NR_Table.NR_TABLE.LPF_C.SpatialThl4C           = ptr[13];
         NR_Table.NR_TABLE.LPF_C.WeightCSel             = ptr[14];

         /* CHIP_NR_CURVEMAPPING_STEP_T */
         NR_Table.NR_TABLE.CurveMappingStep.Curve_step1 = ptr[15];
         NR_Table.NR_TABLE.CurveMappingStep.Curve_step2 = ptr[16];
         NR_Table.NR_TABLE.CurveMappingStep.Curve_step3 = ptr[17];
         NR_Table.NR_TABLE.CurveMappingStep.Curve_step4 = ptr[18];
         NR_Table.NR_TABLE.CurveMappingStep.Curve_step5 = ptr[19];
         NR_Table.NR_TABLE.CurveMappingStep.Curve_step6 = ptr[20];
         NR_Table.NR_TABLE.CurveMappingStep.Curve_step7 = ptr[21];
         NR_Table.NR_TABLE.CurveMappingStep.Curve_step8 = ptr[22];


         /* CHIP_NR_CURVEMAPPING_WEIGHT_UI_T */
         NR_Table.NR_TABLE.CurveMappingWeight.Weight1_0 = ptr[23];
         NR_Table.NR_TABLE.CurveMappingWeight.Weight1_1 = ptr[24];
         NR_Table.NR_TABLE.CurveMappingWeight.Weight1_2 = ptr[25];
         NR_Table.NR_TABLE.CurveMappingWeight.Weight1_3 = ptr[26];
         NR_Table.NR_TABLE.CurveMappingWeight.Weight1_4 = ptr[27];
         NR_Table.NR_TABLE.CurveMappingWeight.Weight1_5 = ptr[28];
         NR_Table.NR_TABLE.CurveMappingWeight.Weight1_6 = ptr[29];
         NR_Table.NR_TABLE.CurveMappingWeight.Weight1_7 = ptr[30];
         NR_Table.NR_TABLE.CurveMappingWeight.Weight1_8 = ptr[31];

         NR_Table.NR_TABLE.CurveMappingWeight.Weight2_0 = ptr[32];
         NR_Table.NR_TABLE.CurveMappingWeight.Weight2_1 = ptr[33];
         NR_Table.NR_TABLE.CurveMappingWeight.Weight2_2 = ptr[34];
         NR_Table.NR_TABLE.CurveMappingWeight.Weight2_3 = ptr[35];
         NR_Table.NR_TABLE.CurveMappingWeight.Weight2_4 = ptr[36];
         NR_Table.NR_TABLE.CurveMappingWeight.Weight2_5 = ptr[37];
         NR_Table.NR_TABLE.CurveMappingWeight.Weight2_6 = ptr[38];
         NR_Table.NR_TABLE.CurveMappingWeight.Weight2_7 = ptr[39];
         NR_Table.NR_TABLE.CurveMappingWeight.Weight2_8 = ptr[40];

        /* CHIP_BLENDING_T 2 +42 */
         NR_Table.NR_TABLE.BLENDING.Weight1Y = ptr[41];
         NR_Table.NR_TABLE.BLENDING.Weight2Y = ptr[42];

         /* CHIP_TNRY_T */
         NR_Table.NR_TABLE.TNR_Y.Y_enable = ptr[43];

         for(i=0;i<15;i++)
         {
                 for(j=0;j<10;j++)
                         NR_Table.NR_TABLE.TNR_Y.Mad[i][j] = ptr[44+(i*10+j)];
         }

         NR_Table.NR_TABLE.TNR_C.C_enable= ptr[194];
         for(i=0;i<7;i++)
         {
                 for(j=0;j<10;j++){
                         NR_Table.NR_TABLE.TNR_C.Mad[i][j] =  ptr[195+(i*10+j)];
                        }
         }

         MPEGNR_Table.MNR_TABLE.MNR_T.Edge_LPF_enable                 = ptr[265];
         MPEGNR_Table.MNR_TABLE.MNR_T.Edge_Theshold                   = ptr[266];
         MPEGNR_Table.MNR_TABLE.MNR_T.Edge_Theshold_Step              = ptr[267];
         MPEGNR_Table.MNR_TABLE.MNR_T.Far_Var_farEdge_refine_Enable   = ptr[268];
         MPEGNR_Table.MNR_TABLE.MNR_T.Range                           = ptr[269];
         MPEGNR_Table.MNR_TABLE.MNR_T.Far_Var_Gain                    = ptr[270];
         MPEGNR_Table.MNR_TABLE.MNR_T.Far_Var_offset                  = ptr[271];
         MPEGNR_Table.MNR_TABLE.MNR_T.Hdiff_Frac_Shift                = ptr[272];
         MPEGNR_Table.MNR_TABLE.MNR_T.varDiff_refine_gain             = ptr[273];
         MPEGNR_Table.MNR_TABLE.MNR_T.Vdiff_Frac_Shift                = ptr[274];
         MPEGNR_Table.MNR_TABLE.MNR_T.Decay_enable                    = ptr[275];
         MPEGNR_Table.MNR_TABLE.MNR_T.Hdecay_Start                    = ptr[276];
         MPEGNR_Table.MNR_TABLE.MNR_T.Hdecay_Step                     = ptr[277];
         MPEGNR_Table.MNR_TABLE.MNR_T.Vdecay_Start                    = ptr[278];
         MPEGNR_Table.MNR_TABLE.MNR_T.Vdecay_Step                     = ptr[279];
         MPEGNR_Table.MNR_TABLE.MNR_T.MNR_Weight                      = ptr[280];
         MPEGNR_Table.MNR_TABLE.MNR_T.Blending_Mode                   = ptr[281];
         MPEGNR_Table.MNR_TABLE.MNR_T.BlendByEdge_enable              = ptr[282];
         MPEGNR_Table.MNR_TABLE.MNR_T.Decay_all_enable                = ptr[283];

         MPEGNR_Table.MNR_TABLE.MNR_UI_T.MosquitoNR_Enable            = ptr[284];
         MPEGNR_Table.MNR_TABLE.MNR_UI_T.Positive_nLevel_Offset       = ptr[285];
         MPEGNR_Table.MNR_TABLE.MNR_UI_T.Positive_nLevel_Shift        = ptr[286];
         MPEGNR_Table.MNR_TABLE.MNR_UI_T.Negative_nLevel_Offset       = ptr[287];
         MPEGNR_Table.MNR_TABLE.MNR_UI_T.Negative_nLevel_Shift        = ptr[288];
         MPEGNR_Table.MNR_TABLE.MNR_UI_T.Edge_Strength_Gain           = ptr[289];
         MPEGNR_Table.MNR_TABLE.MNR_UI_T.Threshold                    = ptr[290];
         MPEGNR_Table.MNR_TABLE.MNR_UI_T.Threshold_Step               = ptr[291];
         MPEGNR_Table.MNR_TABLE.MNR_UI_T.Shift                        = ptr[292];

         MPEGNR_Table.MNR_TABLE.MPEGNR_UI_T.MpegResultWeight_H        = ptr[293];
         MPEGNR_Table.MNR_TABLE.MPEGNR_UI_T.MpegUpperbound_client     = ptr[294];


        fwif_color_Set_NR_Table_tv006(&NR_Table);
        fwif_color_Set_MPEGNR_Table_tv006(&MPEGNR_Table);

        NR_Table.NR_LEVEL =NR_Level.NR_LEVEL;

        fwif_color_SetDNR_tv006(NR_Table.NR_LEVEL);

        fwif_color_Set_MPEGNR_tv006(MPEGNR_Table.MPEG_LEVEL);

#if 0

        dch1_cp_ctrl_reg.regValue = IoReg_Read32(NR_DCH1_CP_Ctrl_reg);
	prelpf_ctrl_reg.regValue = IoReg_Read32(IEDGE_SMOOTH_prelpf_ctrl_reg);

	new_lpf_nearby_ctrl_reg.regValue = IoReg_Read32(NR_NEW_LPF_NEARBY_CTRL_reg);
	nr_mosquito_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_CTRL_reg);
	nr_edge_filter_reg.regValue = IoReg_Read32(NR_EDGE_FILTER_reg);


	dch1_spatial_thl_reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Thl_reg);
	dch1_spatial_thl2_reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Thl2_reg);
	dch1_Spatial_Weight_Reg.regValue = IoReg_Read32(NR_DCH1_Spatial_Weight_reg);


        curve_mapping_ctrl_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_CTRL_reg);
        curve_mapping_step_0_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_STEP_0_reg);
        curve_mapping_step_1_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_STEP_1_reg);
        curve_mapping_step_2_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_STEP_2_reg);
        curve_mapping_step_3_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_STEP_3_reg);
        curve_mapping_w1_1_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W1_1_reg);
        curve_mapping_w1_2_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W1_2_reg);
        curve_mapping_w1_3_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W1_3_reg);
        curve_mapping_w2_1_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W2_1_reg);
        curve_mapping_w2_2_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W2_2_reg);
        curve_mapping_w2_3_reg.regValue = IoReg_Read32(NR_CURVE_MAPPING_W2_3_reg);
        im_di_rtnr_control_reg.regValue = IoReg_Read32(DI_IM_DI_RTNR_CONTROL_reg);

        nr_mosquito_nr_level_ctrl_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_LEVEL_CTRL_reg);
        nr_mosquito_nr_level_ctrl_1_RBUS_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_LEVEL_CTRL_1_reg);
        nr_mosquito_nr_keep_edge_ctrl_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_KEEP_EDGE_CTRL_reg);
        nr_mosquito_nr_decay_ctrl_1_reg.regValue = IoReg_Read32(NR_MOSQUITO_NR_DECAY_CTRL_1_reg);
	mpegnr_ich1_mpegnr2_reg.regValue = IoReg_Read32(MPEGNR_ICH1_MPEGNR2_reg);

        /*#1 CHIP_LPFY_T 9 +8 */
        ptr[0] = dch1_cp_ctrl_reg.cp_spatialenabley;
        ptr[1] = new_lpf_nearby_ctrl_reg.modified_lpf_edge_gain;
        ptr[2] = new_lpf_nearby_ctrl_reg.modified_lpf_thd_upbnd;
        ptr[3] = new_lpf_nearby_ctrl_reg.modified_lpf_thd_lowbnd;
        ptr[4] = new_lpf_nearby_ctrl_reg.modified_lpf_thd_step;
        ptr[5] = dch1_spatial_thl_reg.cp_spatialthly;
        ptr[6] = dch1_spatial_thl_reg.cp_spatialthlyx2;
        ptr[7] = dch1_spatial_thl2_reg.cp_spatialthlyx4;
        ptr[8] = dch1_cp_ctrl_reg.cp_sresultweight;


        /* CHIP_LPFC_T 6 +14 */
        ptr[9] = dch1_cp_ctrl_reg.cp_spatialenablec;
        ptr[10] = dch1_Spatial_Weight_Reg.cp_lpresultweightc;
        ptr[11] = dch1_spatial_thl_reg.cp_spatialthlc;
        ptr[12] = dch1_spatial_thl_reg.cp_spatialthlcx2;
        ptr[13] = dch1_spatial_thl2_reg.cp_spatialthlcx4;
        ptr[14] = dch1_Spatial_Weight_Reg.cp_lpresultweightc;

        /* CHIP_NR_CURVEMAPPING_STEP_T 8 +22 */
        ptr[15] = curve_mapping_step_0_reg.curvemapping_step1;
        ptr[16] = curve_mapping_step_0_reg.curvemapping_step2;
        ptr[17] = curve_mapping_step_1_reg.curvemapping_step3;
        ptr[18] = curve_mapping_step_1_reg.curvemapping_step4;
        ptr[19] = curve_mapping_step_2_reg.curvemapping_step5;
        ptr[20] = curve_mapping_step_2_reg.curvemapping_step6;
        ptr[21] = curve_mapping_step_3_reg.curvemapping_step7;
        ptr[22] = curve_mapping_step_3_reg.curvemapping_step8;


        /* CHIP_NR_CURVEMAPPING_WEIGHT_UI_T  18 +40 */
        ptr[23] = curve_mapping_w1_1_reg.curvemapping_w1_0;
        ptr[24] = curve_mapping_w1_1_reg.curvemapping_w1_1;
        ptr[25] = curve_mapping_w1_1_reg.curvemapping_w1_2;
        ptr[26] = curve_mapping_w1_1_reg.curvemapping_w1_3;
        ptr[27] = curve_mapping_w1_2_reg.curvemapping_w1_4;
        ptr[28] = curve_mapping_w1_2_reg.curvemapping_w1_5;
        ptr[29] = curve_mapping_w1_2_reg.curvemapping_w1_6;
        ptr[30] = curve_mapping_w1_2_reg.curvemapping_w1_7;
        ptr[31] = curve_mapping_w1_3_reg.curvemapping_w1_8;

        ptr[32] = curve_mapping_w2_1_reg.curvemapping_w2_0;
        ptr[33] = curve_mapping_w2_1_reg.curvemapping_w2_1;
        ptr[34] = curve_mapping_w2_1_reg.curvemapping_w2_2;
        ptr[35] = curve_mapping_w2_1_reg.curvemapping_w2_3;
        ptr[36] = curve_mapping_w2_2_reg.curvemapping_w2_4;
        ptr[37] = curve_mapping_w2_2_reg.curvemapping_w2_5;
        ptr[38] = curve_mapping_w2_2_reg.curvemapping_w2_6;
        ptr[39] = curve_mapping_w2_2_reg.curvemapping_w2_7;
        ptr[40] = curve_mapping_w2_3_reg.curvemapping_w2_8;

        /* CHIP_BLENDING_T 2 +42 */
        ptr[41] = dch1_Spatial_Weight_Reg.cp_weight1y;
        ptr[42] = dch1_Spatial_Weight_Reg.cp_weight2y;



        /* CHIP_TNRY_T 151 +193*/
        ptr[43] = im_di_rtnr_control_reg.cp_rtnr_y_enable;

        for(i=0;i<15;i++)
        {
                for(j=0;j<10;j++)
                        ptr[44+(i*10+j)]= TNR_Y_Store_Mad[i][j];
        }

        /* CHIP_TNRY_T 151 +344*/

        ptr[194]= im_di_rtnr_control_reg.cp_rtnr_c_enable;
        for(i=0;i<15;i++)
        {
                for(j=0;j<10;j++)
                        ptr[195+(i*10+j)]= TNR_C_Store_Mad[i][j];
        }

        /* CHIP_MNR_T 19  +363*/

        ptr[345] = nr_mosquito_nr_level_ctrl_1_RBUS_reg.edge_lpf_en;
        ptr[346] = nr_mosquito_ctrl_RBUS_reg.mosquito_edgethd;
        ptr[347] = nr_mosquito_ctrl_RBUS_reg.mosquito_edgethd_step;
        ptr[348] = nr_mosquito_nr_level_ctrl_1_RBUS_reg.farvar_faredge_refine_en;
        ptr[349] = nr_mosquito_nr_level_ctrl_1_RBUS_reg.hdiff_frac_half_range;
        ptr[350] = nr_mosquito_nr_level_ctrl_1_RBUS_reg.far_var_gain;
        ptr[351] = nr_mosquito_nr_level_ctrl_1_RBUS_reg.far_var_offset;
        ptr[352] = nr_mosquito_nr_level_ctrl_1_RBUS_reg.hdiff_frac_shift;
        ptr[353] = nr_mosquito_nr_level_ctrl_RBUS_reg.mnr_vardiff_refine_gain;
        ptr[354] = nr_mosquito_nr_level_ctrl_1_RBUS_reg.vdiff_frac_shift;
        ptr[355] = nr_mosquito_nr_decay_ctrl_1_reg.mnr_decay_en;
        ptr[356] = nr_mosquito_nr_decay_ctrl_1_reg.mnr_hdecay_start;
        ptr[357] = nr_mosquito_nr_decay_ctrl_1_reg.mnr_hdecay_step;
        ptr[358] = nr_mosquito_nr_decay_ctrl_1_reg.mnr_vdecay_start;
        ptr[359] = nr_mosquito_nr_decay_ctrl_1_reg.mnr_vdecay_step;
        ptr[360] = nr_mosquito_ctrl_RBUS_reg.mosquitonr_weight;
        ptr[361] = nr_mosquito_ctrl_RBUS_reg.mosquitonr_blending_mode;
        ptr[362] = nr_mosquito_nr_level_ctrl_1_RBUS_reg.blendsnr_byedge_en;
        ptr[363] = nr_mosquito_nr_decay_ctrl_1_reg.mnr_decay_allcomp_en;

        /* CHIP_MNR_UI_T 9  +372*/
        ptr[364] = nr_mosquito_ctrl_RBUS_reg.mosquito_detect_en;
        ptr[365] = nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_positive_offset;
        ptr[366] = nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_positive_shift;
        ptr[367] = nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_negative_offset;
        ptr[368] = nr_mosquito_nr_level_ctrl_RBUS_reg.mosquitonr_nlevel_negative_shift;
        ptr[369] = nr_mosquito_ctrl_RBUS_reg.mosquitonr_edge_strength_gain;
        ptr[370] = nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedgethd;
        ptr[371] = nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedgethd_step;
        ptr[372] = nr_mosquito_nr_keep_edge_ctrl_reg.mnr_keepedge_shift;




        /* CHIP_MPEGNR_UI_T 2 +374*/
        ptr[373] = mpegnr_ich1_mpegnr2_reg.cp_mpegresultweight_x ;
        ptr[374] = 0 ;

#endif


}

unsigned char fwif_color_GetDNR_tv006()
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}

	return VIP_system_info_structure_table->OSD_Info.OSD_NR_Mode;
}
void fwif_color_SetDNR_tv006(unsigned char level)
{
	SLR_VIP_TABLE *gVip_Table = NULL;
	RTK_NOISE_REDUCTION_T *pNR_Table =NULL;
	unsigned int *PQA_TABLE = NULL;
	unsigned int *PQA_TABLE_OFFSET_TEMP = NULL;

	/*=== 20140421 CSFC for vip video fw infomation ====*/
	_system_setting_info *VIP_system_info_structure_table = NULL;
	UINT8 source=255,pqa_table_idx=0,pqa_input_idx=0;


	//VIPprintf("fwif_color_SetDNR		%d\n", level);


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

	/*get pqa table index need check source first. because ap flow or table index will get incorrect table*/
	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);/* get vip source by input src and timing*/
	if (source >= VIP_QUALITY_SOURCE_NUM)
		source = 0;

	if(fwif_color_get_NR_Input_tv006()!= 12) //12 is webcam. it is from hal for web cam special case.
	{
	pqa_table_idx = gVip_Table->VIP_QUALITY_Extend3_Coef[source][VIP_QUALITY_FUNCTION_PQA_Table];
		pqa_input_idx = gVip_Table->VIP_QUALITY_Extend3_Coef[source][VIP_QUALITY_FUNCTION_PQA_Input_Table];
	}
	else
	{
		pqa_table_idx = 9;
		pqa_input_idx = 9;
	}


	scaler_set_PQA_table(pqa_table_idx);
	scaler_set_PQA_Input_table(pqa_input_idx);

	PQA_TABLE = &(gVip_Table->PQA_Table[0][0][0][0]);
	PQA_TABLE_OFFSET_TEMP = &(VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.OFFSET_TEMP[0]);

	fwif_color_PQA_Input_Item_Check(VIP_system_info_structure_table, PQA_TABLE, pqa_table_idx);

	pNR_Table = fwif_color_Get_NR_Table_tv006();
	if (pNR_Table == NULL) {
		VIPprintf("~get NR_Table_tv006 Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}


	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_General_ctrl.SNR_spatialenabley = pNR_Table->NR_TABLE.LPF_Y.Spatial_Enable_Y;
	/*gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Modified_LPF.SNR_modified_lpf_enable = pNR_Table->NR_TABLE.LPF_Y.Modified_LPF_enable;*/ /*remove from K2L*/
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Modified_LPF.SNR_modified_lpf_edge_gain = pNR_Table->NR_TABLE.LPF_Y.Gain;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Modified_LPF.SNR_modified_lpf_thd_lowbnd = pNR_Table->NR_TABLE.LPF_Y.Thd_LowBnd;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Modified_LPF.SNR_modified_lpf_thd_upbnd = pNR_Table->NR_TABLE.LPF_Y.Thd_UpBnd;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Modified_LPF.SNR_modified_lpf_thd_step = pNR_Table->NR_TABLE.LPF_Y.Thd_Step;
#if 0//lesley: struct change
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Spatial_NR_Y.SNR_zoranfiltersizey = pNR_Table->NR_TABLE.LPF_Y.RZ_filter_size;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Spatial_NR_Y.SNR_zoranweight2y = pNR_Table->NR_TABLE.LPF_Y.Weight2Y;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Spatial_NR_Y.SNR_zoranweight3y = pNR_Table->NR_TABLE.LPF_Y.Weight3Y;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Spatial_NR_Y.SNR_zoranweight4y = pNR_Table->NR_TABLE.LPF_Y.Weight4Y;
#endif
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Spatial_NR_Y.SNR_spatialthly = pNR_Table->NR_TABLE.LPF_Y.SpatialThlY;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Spatial_NR_Y.SNR_spatialthlyx2 = pNR_Table->NR_TABLE.LPF_Y.SpatialThl2Y;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Spatial_NR_Y.SNR_spatialthlyx4 = pNR_Table->NR_TABLE.LPF_Y.SpatialThl4Y;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Spatial_NR_Y.SNR_zoranweighty_sel = pNR_Table->NR_TABLE.LPF_Y.WeightYSel;



	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_General_ctrl.SNR_spatialenablec = pNR_Table->NR_TABLE.LPF_C.Spatial_Enable_C;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Spatial_NR_C.SNR_lpresultweightc = pNR_Table->NR_TABLE.LPF_C.LPResultWeightC;
#if 0//lesley: struct change
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Spatial_NR_C.SNR_zoranfiltersizec = pNR_Table->NR_TABLE.LPF_C.RZ_filter_Size;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Spatial_NR_C.SNR_zoranweight2c = pNR_Table->NR_TABLE.LPF_C.Weight2C;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Spatial_NR_C.SNR_zoranweight3c = pNR_Table->NR_TABLE.LPF_C.Weight3C;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Spatial_NR_C.SNR_zoranweight4c = pNR_Table->NR_TABLE.LPF_C.Weight4C;
#endif
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Spatial_NR_C.SNR_spatialthlc = pNR_Table->NR_TABLE.LPF_C.SpatialThlC;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Spatial_NR_C.SNR_spatialthlcx2 = pNR_Table->NR_TABLE.LPF_C.SpatialThl2C;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Spatial_NR_C.SNR_spatialthlcx4 = pNR_Table->NR_TABLE.LPF_C.SpatialThl4C;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Spatial_NR_C.SNR_zoranweightc_sel = pNR_Table->NR_TABLE.LPF_C.WeightCSel;

	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Spatial_NR_Y.SNR_weight1y = pNR_Table->NR_TABLE.BLENDING.Weight1Y;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Spatial_NR_Y.SNR_weight2y = pNR_Table->NR_TABLE.BLENDING.Weight2Y;

	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_step1 = pNR_Table->NR_TABLE.CurveMappingStep.Curve_step1;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_step2 = pNR_Table->NR_TABLE.CurveMappingStep.Curve_step2;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_step3 = pNR_Table->NR_TABLE.CurveMappingStep.Curve_step3;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_step4 = pNR_Table->NR_TABLE.CurveMappingStep.Curve_step4;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_step5 = pNR_Table->NR_TABLE.CurveMappingStep.Curve_step5;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_step6 = pNR_Table->NR_TABLE.CurveMappingStep.Curve_step6;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_step7 = pNR_Table->NR_TABLE.CurveMappingStep.Curve_step7;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_step8 = pNR_Table->NR_TABLE.CurveMappingStep.Curve_step8;

	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w1_0 = pNR_Table->NR_TABLE.CurveMappingWeight.Weight1_0;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w1_1 = pNR_Table->NR_TABLE.CurveMappingWeight.Weight1_1;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w1_2 = pNR_Table->NR_TABLE.CurveMappingWeight.Weight1_2;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w1_3 = pNR_Table->NR_TABLE.CurveMappingWeight.Weight1_3;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w1_4 = pNR_Table->NR_TABLE.CurveMappingWeight.Weight1_4;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w1_5 = pNR_Table->NR_TABLE.CurveMappingWeight.Weight1_5;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w1_6 = pNR_Table->NR_TABLE.CurveMappingWeight.Weight1_6;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w1_7 = pNR_Table->NR_TABLE.CurveMappingWeight.Weight1_7;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w1_8 = pNR_Table->NR_TABLE.CurveMappingWeight.Weight1_8;

	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w2_0 = pNR_Table->NR_TABLE.CurveMappingWeight.Weight2_0;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w2_1 = pNR_Table->NR_TABLE.CurveMappingWeight.Weight2_1;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w2_2 = pNR_Table->NR_TABLE.CurveMappingWeight.Weight2_2;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w2_3 = pNR_Table->NR_TABLE.CurveMappingWeight.Weight2_3;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w2_4 = pNR_Table->NR_TABLE.CurveMappingWeight.Weight2_4;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w2_5 = pNR_Table->NR_TABLE.CurveMappingWeight.Weight2_5;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w2_6 = pNR_Table->NR_TABLE.CurveMappingWeight.Weight2_6;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w2_7 = pNR_Table->NR_TABLE.CurveMappingWeight.Weight2_7;
	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_SNR_TABLE.S_SNR_Curve_Mapping_Mode.SNR_curvemapping_w2_8 = pNR_Table->NR_TABLE.CurveMappingWeight.Weight2_8;

	/* k3l */

	if(drv_memory_get_ip_DMA420_mode() == 2 )	//400
		gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_RTNR_TABLE.S_RTNR_General_ctrl.RTNR_c_enable = 0;
	else
		gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_RTNR_TABLE.S_RTNR_General_ctrl.RTNR_c_enable = pNR_Table->NR_TABLE.TNR_C.C_enable;

	gVip_Table->Manual_NR_Table[pqa_table_idx][level].S_RTNR_TABLE.S_RTNR_General_ctrl.RTNR_y_enable = pNR_Table->NR_TABLE.TNR_Y.Y_enable;


#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	if ( vpq_project_id == 0x00060000 && (ai_scene_ctrl.ai_scene_global.scene_en == 0))
		fwif_color_set_PQA_motion_threshold_TV006();
#else
	if ( vpq_project_id == 0x00060000)
		fwif_color_set_PQA_motion_threshold_TV006();
#endif

#if 0
	if (level < 4) {	/*Manual Mode*/

		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR] = 0;
		/*WaitFor_DEN_STOP();*/
		fwif_color_RTNR_flow(&(gVip_Table->Manual_NR_Table[pqa_table_idx][level]));
		fwif_color_SNR_flow(&(gVip_Table->Manual_NR_Table[pqa_table_idx][level]));
	} else {	/*Auto Mode*/
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR] = 0;
		/*WaitFor_DEN_STOP();*/
		fwif_color_RTNR_flow(&(gVip_Table->Manual_NR_Table[pqa_table_idx][DRV_NR_AUTO]));
		fwif_color_SNR_flow(&(gVip_Table->Manual_NR_Table[pqa_table_idx][DRV_NR_AUTO]));

		drvif_color_PQ_Adaptive_offset_temp(PQA_TABLE, pqa_table_idx, I_DNR, PQA_TABLE_OFFSET_TEMP);
		/*fwif_color_regPQATableOffsetTemp(PQA_TABLE_OFFSET_TEMP);*/
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR] = 1;
	}
#else

	VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR] = 0;
	fwif_color_RTNR_flow(&(gVip_Table->Manual_NR_Table[pqa_table_idx][level]));
	fwif_color_SNR_flow(&(gVip_Table->Manual_NR_Table[pqa_table_idx][level]));
	drvif_color_PQ_Adaptive_offset_temp(PQA_TABLE, pqa_table_idx, I_DNR, PQA_TABLE_OFFSET_TEMP);
	VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_DNR] = 1;



#endif
}

void fwif_color_Set_NR_Input_tv006(unsigned char input)
{
	bNr_Input = input;
}


unsigned char fwif_color_get_NR_Input_tv006(void)
{
	return bNr_Input;
}


void fwif_color_Set_MPEGNR_Table_tv006(RTK_MPEG_NOISE_REDUCTION_T *ptr)
{
	if (ptr == NULL) {
		VIPprintf("fwif_color_Set_MPEGNR_Table_tv006 error\n");
		return;
	}
	pMPEGNR_Table_tv006 = ptr;
	VIPprintf(" == = Set_MPEGNR_Table_tv006 == = \n");

}
RTK_MPEG_NOISE_REDUCTION_T *fwif_color_Get_MPEGNR_Table_tv006(void)
{
	if (pMPEGNR_Table_tv006 == NULL) {
		VIPprintf("pMPEGNR_Table_tv006 is NULL\n");
		return NULL;
	}
	return (RTK_MPEG_NOISE_REDUCTION_T *)pMPEGNR_Table_tv006;

}

unsigned char fwif_color_MPEGDNR_tv006()
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return 0;
	}

	return VIP_system_info_structure_table->OSD_Info.OSD_MpegNR_Mode;
}

void fwif_color_Set_MPEGNR_tv006(unsigned char level)
{

	/*int ret;*/
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();

	RTK_MPEG_NOISE_REDUCTION_T *pMPEGNR_Table = fwif_color_Get_MPEGNR_Table_tv006();
	_system_setting_info *VIP_system_info_structure_table = NULL;
	unsigned int *PQA_TABLE = NULL;
	unsigned int *PQA_TABLE_OFFSET_TEMP = NULL;
	UINT8 source=255,pqa_table_idx=0,pqa_input_idx=0;

	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	if (pMPEGNR_Table == NULL) {
		VIPprintf("~get pMPEGNR_Table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}



	/*=== 20140421 CSFC for vip video fw infomation ====*/
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		VIPprintf("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	/*get pqa table index need check source first. because ap flow or table index will get incorrect table*/
	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);/* get vip source by input src and timing*/
	if (source >= VIP_QUALITY_SOURCE_NUM)
		source = 0;


	if(fwif_color_get_NR_Input_tv006()!= 12) //12 is webcam. it is from hal for webcam special case.
	{
	pqa_table_idx = gVip_Table->VIP_QUALITY_Extend3_Coef[source][VIP_QUALITY_FUNCTION_PQA_Table];
		pqa_input_idx = gVip_Table->VIP_QUALITY_Extend3_Coef[source][VIP_QUALITY_FUNCTION_PQA_Input_Table];
	}
	else
	{
		pqa_table_idx = 9;
		pqa_input_idx = 9;
	}

	scaler_set_PQA_table(pqa_table_idx);
	scaler_set_PQA_Input_table(pqa_input_idx);


	PQA_TABLE = &(gVip_Table->PQA_Table[0][0][0][0]);
	PQA_TABLE_OFFSET_TEMP = &(VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.OFFSET_TEMP[0]);

	if (pMPEGNR_Table->MPEG_LEVEL < 4) {
		DRV_NR_Item *pNR_ITEM = NULL;
		/*Manual Mode*/
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_MPEGNR] = 0;
		/*WaitFor_DEN_STOP();*/
		pNR_ITEM = &(gVip_Table->Manual_NR_Table[pqa_table_idx][pMPEGNR_Table->MPEG_LEVEL]);
		/*drvif_color_MPEGNR_flow(&(gVip_Table.Manual_NR_Table[pqa_table][level]));*/


		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_edge_lpf_en = pMPEGNR_Table->MNR_TABLE.MNR_T.Edge_LPF_enable;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mosquito_edgethd = pMPEGNR_Table->MNR_TABLE.MNR_T.Edge_Theshold;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mosquito_edgethd_step = pMPEGNR_Table->MNR_TABLE.MNR_T.Edge_Theshold_Step;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_farvar_faredge_refine_en = pMPEGNR_Table->MNR_TABLE.MNR_T.Far_Var_farEdge_refine_Enable;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_far_var_gain = pMPEGNR_Table->MNR_TABLE.MNR_T.Far_Var_Gain;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_far_var_offset = pMPEGNR_Table->MNR_TABLE.MNR_T.Far_Var_offset;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_hdiff_frac_shift = pMPEGNR_Table->MNR_TABLE.MNR_T.Hdiff_Frac_Shift;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_vardiff_refine_gain = pMPEGNR_Table->MNR_TABLE.MNR_T.varDiff_refine_gain;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_vdiff_frac_shift = pMPEGNR_Table->MNR_TABLE.MNR_T.Vdiff_Frac_Shift;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mnr_decay_en = pMPEGNR_Table->MNR_TABLE.MNR_T.Decay_enable;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_hdecay_start = pMPEGNR_Table->MNR_TABLE.MNR_T.Hdecay_Start;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_hdecay_step = pMPEGNR_Table->MNR_TABLE.MNR_T.Hdecay_Step;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_vdecay_start = pMPEGNR_Table->MNR_TABLE.MNR_T.Vdecay_Start;

		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_vdecay_step = pMPEGNR_Table->MNR_TABLE.MNR_T.Vdecay_Step;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mosquitonr_weight = pMPEGNR_Table->MNR_TABLE.MNR_T.MNR_Weight;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mosquitonr_blending_mode = pMPEGNR_Table->MNR_TABLE.MNR_T.Blending_Mode;

		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_blendsnr_byedge_en = pMPEGNR_Table->MNR_TABLE.MNR_T.BlendByEdge_enable;
                pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mnr_decay_allcomp_en = pMPEGNR_Table->MNR_TABLE.MNR_T.Decay_all_enable;



		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.MNR_hdiff_frac_half_range = pMPEGNR_Table->MNR_TABLE.MNR_T.Range;

		pNR_ITEM->S_SNR_TABLE.S_SNR_General_ctrl.SNR_mosquito_detect_en = pMPEGNR_Table->MNR_TABLE.MNR_UI_T.MosquitoNR_Enable;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mosquitonr_nlevel_positive_offset = pMPEGNR_Table->MNR_TABLE.MNR_UI_T.Positive_nLevel_Offset;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mosquitonr_nlevel_positive_shift = pMPEGNR_Table->MNR_TABLE.MNR_UI_T.Positive_nLevel_Shift;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mosquitonr_nlevel_negative_offset = pMPEGNR_Table->MNR_TABLE.MNR_UI_T.Negative_nLevel_Offset;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mosquitonr_nlevel_negative_shift = pMPEGNR_Table->MNR_TABLE.MNR_UI_T.Negative_nLevel_Shift;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mosquitonr_edge_strength_gain = pMPEGNR_Table->MNR_TABLE.MNR_UI_T.Edge_Strength_Gain;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_keep_edge_threshold = pMPEGNR_Table->MNR_TABLE.MNR_UI_T.Threshold;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_keep_edge_threshold_step = pMPEGNR_Table->MNR_TABLE.MNR_UI_T.Threshold_Step;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_keep_edge_shift = pMPEGNR_Table->MNR_TABLE.MNR_UI_T.Shift;


		pNR_ITEM->S_MPG_TABLE.S_MPEGNR_H.S_MPEGNR_16_LPF.result_weight = pMPEGNR_Table->MNR_TABLE.MPEGNR_UI_T.MpegResultWeight_H;

		// duplicate drvif_color_DRV_MPG_General_ctrl(&(pNR_ITEM->S_MPG_TABLE.S_MPEGNR_General_ctrl));
		drvif_color_DRV_MPG_H_tv006(&(pNR_ITEM->S_MPG_TABLE.S_MPEGNR_H), 1);
		drvif_color_DRV_MPG_V(&(pNR_ITEM->S_MPG_TABLE.S_MPEGNR_V));
		drvif_color_DRV_SNR_Mosquito_NR(&(pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR)); /*Add from set SNR due to LG requirement*/
		drvif_color_DRV_SNR_Mosquito_NR_Oneside_Edge(&(pNR_ITEM->S_SNR_TABLE.S_SNR_MNR_Oneside_Edge)); //merlin5 add
		drvif_color_DRV_SNR_Mosquito_NR_En(pNR_ITEM->S_SNR_TABLE.S_SNR_General_ctrl.SNR_mosquito_detect_en);
		drvif_color_set_DRV_MPG_H_SW_Ctrl(&(pNR_ITEM->S_MPG_TABLE.S_MPEGNR_H.S_MPEGNR_SW_ctrl));
		drvif_color_set_DRV_MPG_V_SW_Ctrl(&(pNR_ITEM->S_MPG_TABLE.S_MPEGNR_V.S_MPEGNR_SW_ctrl));

        } else {
		DRV_NR_Item *pNR_ITEM = NULL;
		/*Auto Mode*/
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_MPEGNR] = 0;
		/*WaitFor_DEN_STOP();*/
		pNR_ITEM = &(gVip_Table->Manual_NR_Table[pqa_table_idx][DRV_NR_AUTO]);
		/*drvif_color_MPEGNR_flow(&(gVip_Table.Manual_NR_Table[pqa_table][DRV_NR_AUTO]));*/

		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_edge_lpf_en = pMPEGNR_Table->MNR_TABLE.MNR_T.Edge_LPF_enable;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mosquito_edgethd = pMPEGNR_Table->MNR_TABLE.MNR_T.Edge_Theshold;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mosquito_edgethd_step = pMPEGNR_Table->MNR_TABLE.MNR_T.Edge_Theshold_Step;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_farvar_faredge_refine_en = pMPEGNR_Table->MNR_TABLE.MNR_T.Far_Var_farEdge_refine_Enable;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_far_var_gain = pMPEGNR_Table->MNR_TABLE.MNR_T.Far_Var_Gain;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_far_var_offset = pMPEGNR_Table->MNR_TABLE.MNR_T.Far_Var_offset;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_hdiff_frac_shift = pMPEGNR_Table->MNR_TABLE.MNR_T.Hdiff_Frac_Shift;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_vardiff_refine_gain = pMPEGNR_Table->MNR_TABLE.MNR_T.varDiff_refine_gain;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_vdiff_frac_shift = pMPEGNR_Table->MNR_TABLE.MNR_T.Vdiff_Frac_Shift;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mnr_decay_en = pMPEGNR_Table->MNR_TABLE.MNR_T.Decay_enable;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_hdecay_start = pMPEGNR_Table->MNR_TABLE.MNR_T.Hdecay_Start;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_hdecay_step = pMPEGNR_Table->MNR_TABLE.MNR_T.Hdecay_Step;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_vdecay_start = pMPEGNR_Table->MNR_TABLE.MNR_T.Vdecay_Start;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_vdecay_step = pMPEGNR_Table->MNR_TABLE.MNR_T.Vdecay_Step;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mosquitonr_weight = pMPEGNR_Table->MNR_TABLE.MNR_T.MNR_Weight;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mosquitonr_blending_mode = pMPEGNR_Table->MNR_TABLE.MNR_T.Blending_Mode;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.MNR_hdiff_frac_half_range = pMPEGNR_Table->MNR_TABLE.MNR_T.Range;

		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_blendsnr_byedge_en = pMPEGNR_Table->MNR_TABLE.MNR_T.BlendByEdge_enable;
                pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mnr_decay_allcomp_en = pMPEGNR_Table->MNR_TABLE.MNR_T.Decay_all_enable;

		pNR_ITEM->S_SNR_TABLE.S_SNR_General_ctrl.SNR_mosquito_detect_en = pMPEGNR_Table->MNR_TABLE.MNR_UI_T.MosquitoNR_Enable;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mosquitonr_nlevel_positive_offset = pMPEGNR_Table->MNR_TABLE.MNR_UI_T.Positive_nLevel_Offset;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mosquitonr_nlevel_positive_shift = pMPEGNR_Table->MNR_TABLE.MNR_UI_T.Positive_nLevel_Shift;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mosquitonr_nlevel_negative_offset = pMPEGNR_Table->MNR_TABLE.MNR_UI_T.Negative_nLevel_Offset;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mosquitonr_nlevel_negative_shift = pMPEGNR_Table->MNR_TABLE.MNR_UI_T.Negative_nLevel_Shift;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_mosquitonr_edge_strength_gain = pMPEGNR_Table->MNR_TABLE.MNR_UI_T.Edge_Strength_Gain;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_keep_edge_threshold = pMPEGNR_Table->MNR_TABLE.MNR_UI_T.Threshold;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_keep_edge_threshold_step = pMPEGNR_Table->MNR_TABLE.MNR_UI_T.Threshold_Step;
		pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR.SNR_keep_edge_shift = pMPEGNR_Table->MNR_TABLE.MNR_UI_T.Shift;


		pNR_ITEM->S_MPG_TABLE.S_MPEGNR_H.S_MPEGNR_16_LPF.result_weight = pMPEGNR_Table->MNR_TABLE.MPEGNR_UI_T.MpegResultWeight_H;

		// duplicate drvif_color_DRV_MPG_General_ctrl(&(pNR_ITEM->S_MPG_TABLE.S_MPEGNR_General_ctrl));
		drvif_color_DRV_MPG_H_tv006(&(pNR_ITEM->S_MPG_TABLE.S_MPEGNR_H), 1);
		drvif_color_DRV_MPG_V(&(pNR_ITEM->S_MPG_TABLE.S_MPEGNR_V));
		drvif_color_DRV_SNR_Mosquito_NR(&(pNR_ITEM->S_SNR_TABLE.S_SNR_Mosquito_NR)); /*Add from set SNR due to LG requirement*/
		drvif_color_DRV_SNR_Mosquito_NR_Oneside_Edge(&(pNR_ITEM->S_SNR_TABLE.S_SNR_MNR_Oneside_Edge)); //merlin5 add
		drvif_color_DRV_SNR_Mosquito_NR_En(pNR_ITEM->S_SNR_TABLE.S_SNR_General_ctrl.SNR_mosquito_detect_en);
		drvif_color_set_DRV_MPG_H_SW_Ctrl(&(pNR_ITEM->S_MPG_TABLE.S_MPEGNR_H.S_MPEGNR_SW_ctrl));
		drvif_color_set_DRV_MPG_V_SW_Ctrl(&(pNR_ITEM->S_MPG_TABLE.S_MPEGNR_V.S_MPEGNR_SW_ctrl));


		drvif_color_PQ_Adaptive_offset_temp(PQA_TABLE, pqa_table_idx, I_MPEGNR, PQA_TABLE_OFFSET_TEMP);
		/*fwif_color_regPQATableOffsetTemp(PQA_TABLE_OFFSET_TEMP);*/
		VIP_system_info_structure_table->PQ_Setting_Info.PQA_Setting_INFO.Run_Flag[I_MPEGNR] = 1;
}

VIP_system_info_structure_table->OSD_Info.OSD_MpegNR_Mode = pMPEGNR_Table->MPEG_LEVEL;
/*====================================*/


Scaler_set_picmode_init_value(PICMODE_INIT_MPEGNR, pMPEGNR_Table->MPEG_LEVEL);



}

void fwif_color_Set_MNR_Enable_Check_tv006(void)
{
	unsigned char MNR_En;
	RTK_MPEG_NOISE_REDUCTION_T *pMPEGNR_Table = fwif_color_Get_MPEGNR_Table_tv006();

	if (pMPEGNR_Table == NULL)
		return;

	MNR_En = pMPEGNR_Table->MNR_TABLE.MNR_UI_T.MosquitoNR_Enable;

	drvif_color_DRV_SNR_Mosquito_NR_En(MNR_En);
}

static unsigned short chip_hue_idx[CHIP_CM_HUE_SEG] = {
	0, 64, 128, 192, 256, 320, 384, 512, 640, 704, 768, 832, 896, 1024, 1152, 1216, 1280,
	1408, 1536, 1664, 1792, 1920, 2048, 2112, 2176, 2304, 2432, 2560, 2624, 2688, 2752,
	2816, 2880, 2944, 3072, 3200, 3328, 3456, 3584, 3712, 3776, 3840, 3968, 4096, 4224, 4352,
	4480, 4608, 4672, 4736, 4864, 4992, 5120, 5248, 5376, 5504, 5632, 5760, 5888, 6016
};

static unsigned short chip_sat_idx[CHIP_CM_SAT_SEG] = {
	0, 128, 256, 384, 512, 1024, 1536, 2048, 2560, 3072, 3584, 4096
};

static unsigned short chip_itn_idx[CHIP_CM_ITN_SEG] = {
	0, 585, 1170, 1752, 2340, 2925, 3510, 4096
};

static CM_HUE_SECTION hue_section;
static CM_SAT_SECTION sat_section;
static CM_ITN_SECTION itn_section;

void fwif_color_set_ICM_table_driver_init_tv006(void)
{
	int i;
	extern CHIP_COLOR_CONTROL_T g_cm_ctrl_adv;

	// save pillar postion except first and last
	unsigned short h_pillar[HUESEGMAX - 1] = {0};
	unsigned short s_pillar[SATSEGMAX - 2] = {0};
	unsigned short i_pillar[ITNSEGMAX - 2] = {0};

	//unsigned char h_pillar_num, s_pillar_num, i_pillar_num;
	int s_global[SATSEGMAX], i_global[ITNSEGMAX];
	RPC_ICM_Global_Ctrl *rpc_icm_global_ctrl = NULL;
	extern unsigned char g_cm_keep_Y;

	for(i = 0; i < CHIP_CM_HUE_SEG - 1; i++)
	{
		h_pillar[i] = chip_hue_idx[i + 1];
	}
	for(i = 0; i < CHIP_CM_SAT_SEG - 2; i++)
	{
		s_pillar[i] = chip_sat_idx[i + 1];
	}
	for(i = 0; i < CHIP_CM_ITN_SEG - 2; i++)
	{
		i_pillar[i] = chip_itn_idx[i + 1];
	}

	//h_pillar_num = CHIP_CM_HUE_SEG + 1;
	//s_pillar_num = CHIP_CM_SAT_SEG;
	//i_pillar_num = CHIP_CM_ITN_SEG;


	memset(s_global, 0, sizeof(s_global));
	memset(i_global, 0, sizeof(i_global));

	drvif_color_icm_Set_global(0, s_global, i_global);
	drvif_color_icm_set_ctrl(0, 0, 0, g_cm_keep_Y);
	drvif_color_icm_set_uv_coring(0, 0);
	// save pillar value only, do not set driver to avoid flashing garbage
	//drvif_color_icm_set_pillar(h_pillar, s_pillar, i_pillar, h_pillar_num, s_pillar_num, i_pillar_num);

	rpc_icm_global_ctrl = fwif_color_GetICM_Global_Ctrl_Struct();
	if (NULL !=  rpc_icm_global_ctrl) {
		rpc_icm_global_ctrl->dHue = 0;
		for (i = 0; i < SATSEGMAX; i++) {
			rpc_icm_global_ctrl->dSatBySat[i] = 0;
		}
		for (i = 0; i < ITNSEGMAX; i++) {
			rpc_icm_global_ctrl->dItnByItn[i] = 0;
		}
	}



	for (i = 0; i < CHIP_CM_COLOR_REGION_MAX; i++) {
		g_cm_ctrl_adv.stColor[i].masterGain= (50 * 135) / 128;
		g_cm_ctrl_adv.stColor[i].RGBOffset_masterGain = (0 * 135) / 128;
		g_cm_ctrl_adv.stColor[i].stColorGain.stColorGainHue= 135;
		g_cm_ctrl_adv.stColor[i].stColorGain.stColorGainSat = 135;
		g_cm_ctrl_adv.stColor[i].stColorGain.stColorGainInt = 128;
	}
}

void fwif_color_set_ICM_table_driver_init_elem_tv006(COLORELEM_TAB_T *pIcm_tab_elem)
{
	int h, s, i;

	if (NULL == pIcm_tab_elem)
		return;

	memset(pIcm_tab_elem, 0, sizeof(COLORELEM_TAB_T));

	for (i = 0; i < CHIP_CM_ITN_SEG; i++) {
		for (s = 0; s < CHIP_CM_SAT_SEG; s++) {
			for (h = 0; h < CHIP_CM_HUE_SEG; h++) {
				pIcm_tab_elem->elem[i][s][h].H = chip_hue_idx[h];
				pIcm_tab_elem->elem[i][s][h].S = chip_sat_idx[s];
				pIcm_tab_elem->elem[i][s][h].I = chip_itn_idx[i]+ITN_SUBTRACT_CONST;
			}
		}
	}
}

void fwif_init_pillar_by_TV006_DB(CHIP_COLOR_REGION_TYPE_T *crRgn_in, CM_HUE_SECTION *hue_section, CM_SAT_SECTION *sat_section, CM_ITN_SECTION *itn_section, COLORELEM_TAB_T *icmTab_in)
{
	unsigned char i, j, k, r, idx;
	unsigned char hpn = 0;	/*hue pillar number*/
	unsigned char same_pillar_flag = 0;
	unsigned short pillar_tmp;
	unsigned short hue_gain_tmp;

	//set 0 to all
	memset(chip_hue_idx, 0, CHIP_CM_HUE_SEG * sizeof(unsigned short));
	memset(chip_sat_idx, 0, CHIP_CM_SAT_SEG * sizeof(unsigned short));
	memset(chip_itn_idx, 0, CHIP_CM_ITN_SEG * sizeof(unsigned short));
	memset(hue_section, 0, sizeof(CM_HUE_SECTION));
	memset(sat_section, 0, sizeof(CM_SAT_SECTION));
	memset(itn_section, 0, sizeof(CM_ITN_SECTION));
	memset(icmTab_in, 0, sizeof(COLORELEM_TAB_T));

	/*hue pillar 0 protect*/
	hue_section->rgn_gain[0] = 128;

	/*sat pillar 0 protect*/
	for(r = 0; r < CHIP_CM_COLOR_REGION_MAX; r++) {
		sat_section->rgn_gain[r][0] = 128;
	}

	/*init hue*/
	for(r = 0; r < CHIP_CM_COLOR_REGION_MAX; r++) {
		for (i = 0; i < CHIP_CM_TBLPOINT; i++) {
			//printk("[ICM DB], r = %d, i = %d, hue_x = %d\n", r, i, crRgn_in->stColorRegion[r].hue_x[i]);

			/*region disable, skip*/
			if (crRgn_in->stColorRegion[r].enable == 0) continue;

			/*pillar is 0, this pillar is no used or first pillar*/
			if (crRgn_in->stColorRegion[r].hue_x[i] == 0)
			{
				if (r == CHIP_CM_COLOR_REGION_RED) /*Red region ==> first pillar*/
				{
					idx = 0;	/*idx is 0, spicial case*/
					hue_section->rgn_gain[idx] = crRgn_in->stColorRegion[r].hue_g[i];
					hue_section->rgn_pillar_rgn[idx] = r;
					hue_section->RGB_offset_curve[0][idx] = ((INT16)(crRgn_in->stColorRegion[r].r_offset_by_hue[i]) - 1024);
					hue_section->RGB_offset_curve[1][idx] = ((INT16)(crRgn_in->stColorRegion[r].g_offset_by_hue[i]) - 1024);
					hue_section->RGB_offset_curve[2][idx] = ((INT16)(crRgn_in->stColorRegion[r].b_offset_by_hue[i]) - 1024);
				}
				/*other region, skip*/
				continue;
			}

			/*get new pillar*/
			pillar_tmp = crRgn_in->stColorRegion[r].hue_x[i];
			hue_gain_tmp = crRgn_in->stColorRegion[r].hue_g[i];

			/*skip, when pillar exist*/
			same_pillar_flag = 0;
			for (idx = 1; idx <= hpn; idx++) {
				if (pillar_tmp == chip_hue_idx[idx])
				{
					same_pillar_flag = 1;
					hue_section->rgn_gain[idx] = (hue_section->rgn_gain[idx] + hue_gain_tmp) / 2;	/*easy average*/
					break;
				}
			}

			/*this pillar data is exist, skip*/
			if (same_pillar_flag) continue;

			/*this pillar data is in need*/ /*hue pillar number, first pillar is 0, start by 1*/
			hpn++;
			if (hpn >= CHIP_CM_HUE_SEG) break;

			/*sort(insert) this pillar*/
			idx = hpn;
			while (idx > 0) {
				if (pillar_tmp < chip_hue_idx[idx - 1]) {
					/*swap, then compare with next pillar*/
					chip_hue_idx[idx] = chip_hue_idx[idx - 1];
					hue_section->rgn_gain[idx] = hue_section->rgn_gain[idx - 1];
					hue_section->rgn_pillar_rgn[idx] = hue_section->rgn_pillar_rgn[idx - 1];
					hue_section->RGB_offset_curve[0][idx] = hue_section->RGB_offset_curve[0][idx - 1];	/*R offset*/
					hue_section->RGB_offset_curve[1][idx] = hue_section->RGB_offset_curve[1][idx - 1];	/*G offset*/
					hue_section->RGB_offset_curve[2][idx] = hue_section->RGB_offset_curve[2][idx - 1];	/*B offset*/
					idx--;
				} else if (pillar_tmp > chip_hue_idx[idx - 1]) {
					/*find the right place, stop and insert value*/
					chip_hue_idx[idx] = pillar_tmp;
					hue_section->rgn_gain[idx] = hue_gain_tmp;
					hue_section->rgn_pillar_rgn[idx] = r;
					hue_section->RGB_offset_curve[0][idx] = ((INT16)(crRgn_in->stColorRegion[r].r_offset_by_hue[i]) - 1024);
					hue_section->RGB_offset_curve[1][idx] = ((INT16)(crRgn_in->stColorRegion[r].g_offset_by_hue[i]) - 1024);
					hue_section->RGB_offset_curve[2][idx] = ((INT16)(crRgn_in->stColorRegion[r].b_offset_by_hue[i]) - 1024);
					break;
				} else {
					break;
				}
			}

			#if 0	/*debug*/
			for (idx = 0; idx < CHIP_CM_HUE_SEG; idx++)
			{
				if (idx % 8 == 0) printk("\n[ICM DB], tmp %d, pillar_hue[%d] = ", hpn, idx);
				printk("%d, ", chip_hue_idx[idx]);
			}
			printk("\n\n");
			#endif
		}
	}


	/*set TV006 DB hue pillar number*/
	hue_section->pillar_num = hpn + 1;
	memcpy(hue_section->rgn_idx, chip_hue_idx, hue_section->pillar_num * sizeof(unsigned short));
	for (i = 0; i < hue_section->pillar_num; i++) hue_section->rgn_pillar[i] = i;

	/*init sat*/
	/*TV006 will set pillar and gain from pillar 1, the first pillar, set all 0*/
	memcpy(&(chip_sat_idx[1]), &(crRgn_in->stColorRegion[0].sat_x), 8 * sizeof(unsigned short));
	memcpy(&(sat_section->rgn_idx[1]), &(crRgn_in->stColorRegion[0].sat_x), 8 * sizeof(unsigned short));
	for (i = 0; i < CHIP_CM_COLOR_REGION_MAX; i++) memcpy(&(sat_section->rgn_gain[i][1]), &(crRgn_in->stColorRegion[i].sat_g), 8 * sizeof(unsigned short));
	sat_section->pillar_num = 9;
	for (i = 0; i < sat_section->pillar_num; i++) sat_section->rgn_pillar[i] = i;

	/*init itn*/ /*set first pillar, 0 <== data protect*/
	memcpy(chip_itn_idx, &(crRgn_in->stColorRegion[0].Int_x), 8 * sizeof(unsigned short));
	crRgn_in->stColorRegion[0].Int_x[0] = 0;	/*first pillar data protect*/
	memcpy(itn_section->rgn_idx, &(crRgn_in->stColorRegion[0].Int_x), 8 * sizeof(unsigned short));
	for (i = 0; i < CHIP_CM_COLOR_REGION_MAX; i++) memcpy(&(itn_section->rgn_gain[i][0]), &(crRgn_in->stColorRegion[i].Int_g), 8 * sizeof(unsigned short));
	itn_section->pillar_num = 8;
	for (i = 0; i < itn_section->pillar_num; i++) itn_section->rgn_pillar[i] = i;

	/*icmTab_in init*/
	for (i = 0; i < itn_section->pillar_num; i++) {
		for (j = 0; j < sat_section->pillar_num; j++) {
			for (k = 0; k < hue_section->pillar_num; k++)
			{
				icmTab_in->elem[i][j][k].I = itn_section->rgn_idx[i] + ITN_SUBTRACT_CONST;
				icmTab_in->elem[i][j][k].S = sat_section->rgn_idx[j];
				icmTab_in->elem[i][j][k].H = hue_section->rgn_idx[k];
			}
		}
	}

	/*debug*/
	#if 1
	VIPprintf("\n\n[ICM DB], init_pillar_by_TV006 debug\n");
	for (i = 0; i < CHIP_CM_HUE_SEG; i++)
	{
		if (i % 8 == 0) VIPprintf("\n[ICM DB], pillar_hue[%d] = ", i);
		VIPprintf("%d, ", chip_hue_idx[i]);
	}
	for (i = 0; i < CHIP_CM_SAT_SEG; i++)
	{
		if (i % 8 == 0) VIPprintf("\n[ICM DB], pillar_sat[%d] = ", i);
		VIPprintf("%d, ", chip_sat_idx[i]);
	}
	for (i = 0; i < CHIP_CM_ITN_SEG; i++)
	{
		if (i % 8 == 0) VIPprintf("\n[ICM DB], pillar_itn[%d] = ", i);
		VIPprintf("%d, ", chip_itn_idx[i]);
	}
	VIPprintf("\n\n");
	#endif
}

unsigned char get_pillar_by_hue(unsigned short hue_idx)
{
	unsigned char hue_pillar = 0;
	int i;

	for(i = 0; i < CHIP_CM_HUE_SEG; i++) {
		if (chip_hue_idx[i] >= hue_idx){
			hue_pillar = i;
			if (chip_hue_idx[i] != hue_idx)
				VIPprintf("[warning]chip_hue_idx loss index %d\n",hue_idx);
			break;
		}
	}

	return hue_pillar;
}

unsigned short get_hue_by_pillar(unsigned char hue_pillar)
{
	if (hue_pillar > CHIP_CM_HUE_SEG - 1)
		return 0;

	return chip_hue_idx[hue_pillar];
}

unsigned char get_pillar_by_sat(unsigned short sat_idx)
{
	unsigned char sat_pillar= 0;
	int i;

	for(i = 0; i < CHIP_CM_SAT_SEG; i++) {
		if (chip_sat_idx[i] >= sat_idx){
			sat_pillar = i;
			if (chip_sat_idx[i] != sat_idx)
				VIPprintf("[warning]chip_sat_idx loss index %d\n",sat_idx);
			break;
		}
	}


	return sat_pillar;
}

unsigned short get_sat_by_pillar(unsigned char sat_pillar)
{
	if (sat_pillar > CHIP_CM_SAT_SEG - 1)
		return 0;

	return chip_sat_idx[sat_pillar];
}


unsigned char get_pillar_by_itn(unsigned short itn_idx)
{
	unsigned char itn_pillar= 0;
	int i;

	for(i = 0; i < CHIP_CM_ITN_SEG; i++) {
		if (chip_itn_idx[i] >= itn_idx){
			itn_pillar = i;
			if (chip_itn_idx[i] != itn_idx)
				VIPprintf("[warning]chip_itn_idx loss index %d\n",itn_idx);
			break;
		}
	}

	return itn_pillar;
}

unsigned short get_itn_by_pillar(unsigned char itn_pillar)
{
	if (itn_pillar > CHIP_CM_ITN_SEG - 1)
		return 0;

	return chip_itn_idx[itn_pillar];
}

unsigned char calc_real_hue_region(UINT16 *hue_idx_in, UINT16 *hue_gain_in, CM_HUE_SECTION *pHue_section)
{
	int i, j;
	UINT8 hue_pillar[CHIP_CM_TBLPOINT] = {0};
	UINT16 hue_idx[CHIP_CM_TBLPOINT] = {0};
	UINT8 bound_num = 0;
	UINT8 r_flag = 0;

	if (NULL == hue_idx_in || NULL == hue_gain_in || NULL == pHue_section)
		return 0;

	memset(pHue_section, 0, sizeof(CM_HUE_SECTION));
	memcpy(hue_idx, hue_idx_in, sizeof(hue_idx));

	for (i = 0; i < CHIP_CM_TBLPOINT; i++) {
		if (i > 0) {
			/*if (hue_idx_in[i] == hue_idx_in[i-1])
				break;
			else */if (hue_idx_in[i] < hue_idx_in[i-1] && !r_flag)
				r_flag = i;
		}
		bound_num = i;
		hue_pillar[i] = get_pillar_by_hue(hue_idx_in[i]);
	}

	if (r_flag) {
		int cnt = 0;

		for (i = 1; i < bound_num+1; i++) {
			/*VIPprintf("hue_pillar=%d\n", hue_pillar[i]);*/
			if (hue_pillar[i] < hue_pillar[0])
			{
				hue_pillar[i] += CHIP_CM_HUE_SEG;
				hue_idx[i] += HUERANGE;
			}
		}

		/*pHue_section->pillar_num = hue_pillar[bound_num]+1+62-hue_pillar[0];*/
		pHue_section->pillar_num = hue_pillar[bound_num]-hue_pillar[0]+1;
		/*VIPprintf("pHue_section->pillar_num = %d, bound_num = %d\n", pHue_section->pillar_num, bound_num);*/
		for (i = 0; i < pHue_section->pillar_num; i++) {
			pHue_section->rgn_pillar[i] = hue_pillar[0]+i;
			pHue_section->rgn_idx[i] = get_hue_by_pillar(pHue_section->rgn_pillar[i]%CHIP_CM_HUE_SEG);
			if (pHue_section->rgn_idx[i] < pHue_section->rgn_idx[0])
				pHue_section->rgn_idx[i] += HUERANGE;
			/*VIPprintf("rgn_pillar = %d, rgn_idx = %d\n", pHue_section->rgn_pillar[i], pHue_section->rgn_idx[i]);*/
		}

		pHue_section->rgn_gain[cnt++] = hue_gain_in[0];
		for (i = 1; i < bound_num+1; i++) {
			for (j = 1; j < hue_pillar[i]-hue_pillar[i-1]+1; j++) {

				pHue_section->rgn_gain[cnt] = ((hue_idx[i]-pHue_section->rgn_idx[cnt])*hue_gain_in[i-1]+
					(pHue_section->rgn_idx[cnt]-hue_idx[i-1])*hue_gain_in[i])/(hue_idx[i]-hue_idx[i-1]);
				/*VIPprintf("i = %d, j = %d, pHue_section->rgn_gain[%d] = %d\n", i, j, cnt, pHue_section->rgn_gain[cnt]);*/
				cnt++;
			}
		}

		for (i = 0; i < pHue_section->pillar_num; i++) {
			pHue_section->rgn_pillar[i] = pHue_section->rgn_pillar[i]%CHIP_CM_HUE_SEG;
			pHue_section->rgn_idx[i] = pHue_section->rgn_idx[i]%HUERANGE;
		}

	} else {
		int cnt = 0;
		pHue_section->pillar_num = hue_pillar[bound_num]-hue_pillar[0]+1;
		/*VIPprintf("pHue_section->pillar_num = %d, bound_num = %d\n", pHue_section->pillar_num, bound_num);*/
		for (i = 0; i < pHue_section->pillar_num; i++)
		{
			pHue_section->rgn_pillar[i] = hue_pillar[0]+i;
			pHue_section->rgn_idx[i] = get_hue_by_pillar(pHue_section->rgn_pillar[i]%CHIP_CM_HUE_SEG);
			/*VIPprintf("rgn_pillar = %d, rgn_idx = %d\n", pHue_section->rgn_pillar[i], pHue_section->rgn_idx[i]);*/
		}

		pHue_section->rgn_gain[cnt++] = hue_gain_in[0];
		for (i = 1; i < bound_num+1; i++) {
			for (j = 1; j < hue_pillar[i]-hue_pillar[i-1]+1; j++) {

				pHue_section->rgn_gain[cnt] = ((hue_idx_in[i]-pHue_section->rgn_idx[cnt])*hue_gain_in[i-1]+
					(pHue_section->rgn_idx[cnt]-hue_idx_in[i-1])*hue_gain_in[i])/(hue_idx_in[i]-hue_idx_in[i-1]);
				/*VIPprintf("i = %d, j = %d, pHue_section->rgn_gain[%d] = %d\n", i, j, cnt, pHue_section->rgn_gain[cnt]);*/
				cnt++;
			}
		}
	}

	return 1;
}

unsigned char fwif_color_copy_ICM_RGB_offset_table_from_tv006_DB(CHIP_COLOR_REGION_TYPE_T *crRgn_in, CM_HUE_SECTION *hue_section, CHIP_COLOR_CONTROL_T *crCtrl_in)
{
	extern ICM_RGB_Offset_table tICM_RGB_Offset[VIP_ICM_TBL_X];
	UINT8 i;

	/*reset all gain to 0*/
	memset(&(tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006]), 0, sizeof(ICM_RGB_Offset_table));

	/*enable*/
	tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_en = 1;	/*always on for LG driver*/

	/*RGB offset table*/
	for (i = 0; i < HUESEGMAX; i++)
	{
		tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_curve[0][i] = (hue_section->RGB_offset_curve[0][i] * crCtrl_in->stColor[hue_section->rgn_pillar_rgn[i]].RGBOffset_masterGain) >> 7;	/*RGB master gain: 0~255/128 ==> gain 0~2*/
		tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_curve[1][i] = (hue_section->RGB_offset_curve[1][i] * crCtrl_in->stColor[hue_section->rgn_pillar_rgn[i]].RGBOffset_masterGain) >> 7;	/*RGB master gain: 0~255/128 ==> gain 0~2*/
		tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_curve[2][i] = (hue_section->RGB_offset_curve[2][i] * crCtrl_in->stColor[hue_section->rgn_pillar_rgn[i]].RGBOffset_masterGain) >> 7;	/*RGB master gain: 0~255/128 ==> gain 0~2*/
	}

	/*gain by S, I curve*/
	for(i = 1; i < 9; i++)
		tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_S_gain[i] = crRgn_in->gain_by_sat_for_rgb_offset[i - 1];
	for(i = 0; i < ITNSEGMAX; i++)
		tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_I_gain[i] = crRgn_in->gain_by_int_for_rgb_offset[i];


	/*data protect*/
	for (i = 0; i < HUESEGMAX; i++)
	{
		if (tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_curve[0][i] > 1023) tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_curve[0][i] = 1023;
		if (tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_curve[0][i] < -1024) tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_curve[0][i] = -1024;
		if (tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_curve[1][i] > 1023) tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_curve[1][i] = 1023;
		if (tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_curve[1][i] < -1024) tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_curve[1][i] = -1024;
		if (tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_curve[2][i] > 1023) tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_curve[2][i] = 1023;
		if (tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_curve[2][i] < -1024) tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_curve[2][i] = -1024;
	}

	for(i = 1; i < SATSEGMAX; i++)
		if (tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_S_gain[i] > 63) tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_S_gain[i] = 63;

	for(i = 0; i < ITNSEGMAX; i++)
		if (tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_I_gain[i] > 63) tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_I_gain[i] = 63;

	return 1;
}
#ifdef CONFIG_SCALER_ENABLE_V4L2
void fwif_color_set_ICM_tv006_null_case(struct v4l2_ext_cm_ui_status cmuiInfo)
{
	extern UINT8 g_Color_Mode;
	UINT8 i;

	if (cmuiInfo.dynamic.enable) {
		extern CHIP_COLOR_CONTROL_T g_cm_ctrl_adv;
		UINT8 gain_OSD_lv[4] = {128, 135, 145, 155};	/*base on 128*/
		unsigned char OSD_lv = cmuiInfo.dynamic.value;
		if (OSD_lv > 3) OSD_lv = 3;
		g_Color_Mode = COLOR_MODE_ADV;

		for (i = 0; i < CHIP_CM_COLOR_REGION_MAX; i++) {
			g_cm_ctrl_adv.stColor[i].masterGain= (50 * gain_OSD_lv[OSD_lv]) / 128;
			g_cm_ctrl_adv.stColor[i].RGBOffset_masterGain = (0 * gain_OSD_lv[OSD_lv]) / 128;
			g_cm_ctrl_adv.stColor[i].stColorGain.stColorGainHue= (128 * gain_OSD_lv[OSD_lv]) / 128;
			g_cm_ctrl_adv.stColor[i].stColorGain.stColorGainSat = (128 * gain_OSD_lv[OSD_lv]) / 128;
			g_cm_ctrl_adv.stColor[i].stColorGain.stColorGainInt = 128;
		}
	} else if(cmuiInfo.cms.enable)
	{
		extern CHIP_COLOR_CONTROL_T g_cm_ctrl_exp;
		g_Color_Mode = COLOR_MODE_EXP;

		for (i = 0; i < V4L2_EXT_CM_CMS_MAX; i++) {
			g_cm_ctrl_exp.stColor[i].masterGain= 70;	//(50 * gain_OSD_lv[0]) / 128;
			g_cm_ctrl_exp.stColor[i].RGBOffset_masterGain = 0;
			g_cm_ctrl_exp.stColor[i].stColorGain.stColorGainHue= 128 + (cmuiInfo.cms.gain_hue[i] * 3);
			g_cm_ctrl_exp.stColor[i].stColorGain.stColorGainSat = 128 + (cmuiInfo.cms.gain_saturation[i] * 3);
			g_cm_ctrl_exp.stColor[i].stColorGain.stColorGainInt = 128 + (cmuiInfo.cms.gain_luminance[i] * 3);
			//printk("roger,[%d] h = %d, s = %d, i = %d\n", i, cmuiInfo.cms.gain_hue[i], cmuiInfo.cms.gain_saturation[i], cmuiInfo.cms.gain_luminance[i]);
		}
	}

}
#endif

unsigned char fwif_color_calc_ICM_gain_table_driver_tv006(CHIP_COLOR_REGION_TYPE_T *crRgn_in, CHIP_COLOR_CONTROL_T *crCtrl_in, COLORELEM_TAB_T *icmTab_in, COLORELEM_TAB_T *icmTab_out)
{
	int rgn, i, h, s, iOffset, hOffset, sOffset, iP, hP, sP;
	int sum_rgn_gain, hue_rgn_gain, sat_rgn_gain, itn_rgn_gain;
	long long tmp64;
	unsigned short h_min, h_max, s_min, s_max, i_min, i_max;
	int h_min_ext, h_max_ext, h_mid;
	static CHIP_COLOR_REGION_TYPE_T crRgn_in_tmp;

	if (NULL == crRgn_in || NULL == crCtrl_in || NULL == icmTab_out)
		return 0;

	//check hue boundary
	memcpy(&crRgn_in_tmp, crRgn_in, sizeof(CHIP_COLOR_REGION_TYPE_T));
	crRgn_in = &crRgn_in_tmp;
	for (rgn = 0; rgn < CHIP_CM_COLOR_REGION_MAX; rgn++) {
		for (h = 0; h < CHIP_CM_TBLPOINT; h++) {
			if (crRgn_in->stColorRegion[rgn].hue_x[h] == HUERANGE)
				crRgn_in->stColorRegion[rgn].hue_x[h] = 0;
		}
	}

	/*init pillar index and data*/
	fwif_init_pillar_by_TV006_DB(crRgn_in, &hue_section, &sat_section, &itn_section, icmTab_in);
	memcpy(icmTab_out, icmTab_in, sizeof(COLORELEM_TAB_T));

	/*reset pillar by LG DB*/
	//move to unsigned char fwif_color_icm_set_pillar(void);
	//drvif_color_icm_set_pillar(&(chip_hue_idx[1]), &(chip_sat_idx[1]), &(chip_itn_idx[1]), hue_section.pillar_num + 1, sat_section.pillar_num, itn_section.pillar_num);

	/*set RGB offset table by LG DB*/
	fwif_color_copy_ICM_RGB_offset_table_from_tv006_DB(crRgn_in, &hue_section, crCtrl_in);


	/*Calc ColorGain*/
	for (i = 0; i < itn_section.pillar_num; i++) {
		iP = itn_section.rgn_pillar[i];
		for (s = 0; s < sat_section.pillar_num; s++) {
			sP = sat_section.rgn_pillar[s];
			for (h = 0; h < hue_section.pillar_num; h++) {
				hP = hue_section.rgn_pillar[h];

				hOffset = crCtrl_in->stColor[hue_section.rgn_pillar_rgn[h]].stColorGain.stColorGainHue-128;
				sOffset = crCtrl_in->stColor[hue_section.rgn_pillar_rgn[h]].stColorGain.stColorGainSat-128;
				iOffset = crCtrl_in->stColor[hue_section.rgn_pillar_rgn[h]].stColorGain.stColorGainInt-128;
				hOffset = (hOffset<<8)*HUERANGE/360;//total need shift 4
				sOffset = (sOffset<<8)*SATRANGE/100;//total need shift 4
				iOffset = (iOffset<<8)*ITNRANGE/100;//total need shift 4

				hue_rgn_gain = hue_section.rgn_gain[h]-128;
				sat_rgn_gain = sat_section.rgn_gain[hue_section.rgn_pillar_rgn[h]][s]-128;
				itn_rgn_gain = itn_section.rgn_gain[hue_section.rgn_pillar_rgn[h]][i]-128;
				sum_rgn_gain = abs(hue_rgn_gain*sat_rgn_gain*itn_rgn_gain);//total need shift 21

				tmp64 = crCtrl_in->stColor[hue_section.rgn_pillar_rgn[h]].masterGain;	/*master gain for K5: 0-255/128 ==> gain 0~2*/
				tmp64 = tmp64*sum_rgn_gain;/*total need shift 7+21 = 28*/
				/*apply offset*/
				icmTab_out->elem[iP][sP][hP].H += ((hue_rgn_gain<0)?(0-((tmp64*hOffset)>>37)):((tmp64*hOffset)>>36)); /*shift 28+8 = 36*/
				icmTab_out->elem[iP][sP][hP].S += ((sat_rgn_gain<0)?(0-((tmp64*sOffset)>>37)):((tmp64*sOffset)>>36)); /*shift 28+8 = 36*/
				icmTab_out->elem[iP][sP][hP].I += ((itn_rgn_gain<0)?(0-((tmp64*iOffset)>>37)):((tmp64*iOffset)>>36)); /*shift 28+8 = 36*/
			}
		}
	}


	/*do clamp*/
	/*Set min, max*/
	h_min = 0;//hue_section.rgn_idx[0];
	h_max = (HUERANGE * 4) / 3;//(hue_section.pillar_num > 0)?hue_section.rgn_idx[hue_section.pillar_num-1]:h_min;
	s_min = 0;//sat_section.rgn_idx[0];
	s_max = (SATRANGE * 4) / 3;//(sat_section.pillar_num > 0)?sat_section.rgn_idx[sat_section.pillar_num-1]:s_min;
	i_min = 0;//itn_section.rgn_idx[0]+ITN_SUBTRACT_CONST;
	i_max = 5846;//((itn_section.pillar_num > 0)?itn_section.rgn_idx[itn_section.pillar_num-1]:i_min)+ITN_SUBTRACT_CONST;

	for (i = 0; i < itn_section.pillar_num; i++) {
		iP = itn_section.rgn_pillar[i];
		for (s = 0; s < sat_section.pillar_num; s++) {
			sP = sat_section.rgn_pillar[s];
			for (h = 0; h < hue_section.pillar_num; h++) {
				hP = hue_section.rgn_pillar[h];

				/*do clamp*/
				if (icmTab_out->elem[iP][sP][hP].S > s_max)
					icmTab_out->elem[iP][sP][hP].S = s_max;
				else if (icmTab_out->elem[iP][sP][hP].S < s_min)
					icmTab_out->elem[iP][sP][hP].S = s_min;

				if (icmTab_out->elem[iP][sP][hP].I > i_max)
					icmTab_out->elem[iP][sP][hP].I = i_max;
				else if (icmTab_out->elem[iP][sP][hP].I < i_min)
					icmTab_out->elem[iP][sP][hP].I = i_min;

				/*calc hue boundary*/
				if (h_min <= h_max) {
					if (icmTab_out->elem[iP][sP][hP].H > h_max)
						icmTab_out->elem[iP][sP][hP].H = h_max;
					else if (icmTab_out->elem[iP][sP][hP].H < h_min)
						icmTab_out->elem[iP][sP][hP].H = h_min;
				} else {
					h_min_ext = 0-(HUERANGE-h_min);
					h_max_ext = HUERANGE+h_max;
					h_mid = (h_min+h_max)>>1;

					if (icmTab_out->elem[iP][sP][hP].H > h_max_ext)
						icmTab_out->elem[iP][sP][hP].H = h_max_ext;
					else if (icmTab_out->elem[iP][sP][hP].H < h_min_ext)
						icmTab_out->elem[iP][sP][hP].H = h_min_ext;
					else if (icmTab_out->elem[iP][sP][hP].H <= h_mid && icmTab_out->elem[iP][sP][hP].H > h_max)
						icmTab_out->elem[iP][sP][hP].H = h_max;
					else if (icmTab_out->elem[iP][sP][hP].H > h_mid && icmTab_out->elem[iP][sP][hP].H < h_min)
						icmTab_out->elem[iP][sP][hP].H = h_min;
				}
			}
		}
	}


	//correct hue value
	for (i = 0; i < ITNSEGMAX; i++) {
		for (s = 0; s < SATSEGMAX; s++) {
			for (h = 0; h < HUESEGMAX; h++) {
				if (icmTab_out->elem[i][s][h].H < 0)
					icmTab_out->elem[i][s][h].H += HUERANGE;
				icmTab_out->elem[i][s][h].H %= HUERANGE;
			}
		}
	}

	return 1;
}

extern RPC_SLR_VIP_TABLE *g_RPC_Share_Memory_VIP_TABLE_Struct;
void fwif_color_icm_pillar_sendRPCtoVideo(unsigned char update_ICM_Pillar_Offset){
	memcpy(&g_RPC_Share_Memory_VIP_TABLE_Struct->ICM_WRITE_TABLE.chip_hue_idx[0], chip_hue_idx, sizeof(unsigned short)*CHIP_CM_HUE_SEG);
	memcpy(&g_RPC_Share_Memory_VIP_TABLE_Struct->ICM_WRITE_TABLE.chip_sat_idx[0], chip_sat_idx, sizeof(unsigned short)*CHIP_CM_SAT_SEG);
	memcpy(&g_RPC_Share_Memory_VIP_TABLE_Struct->ICM_WRITE_TABLE.chip_itn_idx[0], chip_itn_idx, sizeof(unsigned short)*CHIP_CM_ITN_SEG);
	memcpy(&g_RPC_Share_Memory_VIP_TABLE_Struct->ICM_WRITE_TABLE.hue_section_pillar_num, &hue_section.pillar_num, sizeof(unsigned char));
	memcpy(&g_RPC_Share_Memory_VIP_TABLE_Struct->ICM_WRITE_TABLE.sat_section_pillar_num, &sat_section.pillar_num, sizeof(unsigned char));
	memcpy(&g_RPC_Share_Memory_VIP_TABLE_Struct->ICM_WRITE_TABLE.itn_section_pillar_num, &itn_section.pillar_num, sizeof(unsigned char));
	memcpy(&g_RPC_Share_Memory_VIP_TABLE_Struct->ICM_WRITE_TABLE.update_ICM_Pillar_Offset, &update_ICM_Pillar_Offset, sizeof(unsigned char));

}

unsigned char fwif_color_set_ICM_RGB_offset_tv006()
{
	extern ICM_RGB_Offset_table tICM_RGB_Offset[VIP_ICM_TBL_X];
	drvif_color_icm_Set_RGB_Offset_Enable(tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_en);
	if (false == drvif_color_icm_Set_RGB_Offset_Table(tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_curve, tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_S_gain, tICM_RGB_Offset[RGB_OFFSET_TABLE_TV006].RGB_offset_I_gain))
	{
		rtd_pr_vpq_emerg("ERROR, RGB_Offset_Table write fail\n");
		return false;
	}
	return 1;
}

unsigned char fwif_color_icm_set_pillar(void)
{
	drvif_color_icm_set_pillar(&(chip_hue_idx[1]), &(chip_sat_idx[1]), &(chip_itn_idx[1]), hue_section.pillar_num + 1, sat_section.pillar_num, itn_section.pillar_num);
	return 0;
}

unsigned char fwif_color_icm_set_pillar_RGB_Offset_byD_ISR(void)
{
	//unsigned int time_0 = 0, time_1 = 0;
	unsigned short v_sta, v_end,  line_cnt_0, line_cnt_1;

	//time_0 = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);
	drvif_Get_UZUDTG_Info(&v_sta, &v_end, &line_cnt_0);
	//fwif_color_icm_set_pillar();	// remove to videofw D ISR use fwif_color_icm_pillar_sendRPCtoVideo function
	fwif_color_set_ICM_RGB_offset_tv006();
	drvif_Get_UZUDTG_Info(&v_sta, &v_end, &line_cnt_1);
	//time_1 = IoReg_Read32(TIMER_SCPU_CLK90K_LO_reg);

	//rtd_pr_vpq_info("vpq_ICM_handler t0~1=%d,%d,\n", time_0, time_1);

	if (((line_cnt_0>v_sta) && (line_cnt_0<v_end)) || ((line_cnt_1>v_sta) && (line_cnt_1<v_end)))
		rtd_pr_vpq_emerg("vpq_ICM_handler, line cnt error v_sta=%d, v_end=%d, line_cnt_0=%d, line_cnt_1=%d\n",
			v_sta, v_end,  line_cnt_0, line_cnt_1);

	return 0;
}


void fwif_color_set_ICM_Sat_Zero_Patch(COLORELEM_TAB_T *pIcm_tab_elem)
{
	int i, h;

	for (i = 0 ; i < ITNSEGMAX; i++)
		for (h = 0; h < HUESEGMAX; h++)
			pIcm_tab_elem->elem[i][0][h].S = 0;
}

void fwif_color_set_ICM_table_driver_tv006(COLORELEM_TAB_T *pIcm_tab_elem, unsigned char waiting)
{
	unsigned int addr = 0, size;
	unsigned int  *pVir_addr = NULL;
	char ret;
	unsigned int addr_aligned = 0, size_aligned;
	unsigned int  *pVir_addr_aligned = NULL;
#ifdef CONFIG_ARM64 //ARM32 compatible
	unsigned long va_temp;
#else
	unsigned int va_temp;
#endif

	addr = get_query_VIP_DMAto3DTABLE_start_address(DMAto3DTABLE_ICM);

	addr = drvif_memory_get_data_align(addr, (1 << 12));
	size = drvif_memory_get_data_align(VIP_DMAto3DTable_ICM_SIZE, (1 << 12));


	if (addr == 0) {
		rtd_pr_vpq_emerg("ICM :fwif_color_Icm_table_select_By_DMA, addr = NULL\n");
		return;
	}

	if (NULL == pIcm_tab_elem)
		return;

	fwif_color_set_ICM_Sat_Zero_Patch(pIcm_tab_elem);
	if(waiting == TRUE /*&& !drvif_scalerdisplay_get_force_bg_status() && Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE*/) {
		//fwif_color_WaitFor_DEN_STOP_UZUDTG();
		//drvif_Wait_UZUDTG_start(10, -200); //for DMA always need wait, so move to driver
	}
	//drvif_color_icm_Write3DTable(pIcm_tab_elem->elem);


	pVir_addr = dvr_remap_uncached_memory(addr, size, __builtin_return_address(0));

	size_aligned = size;
	addr_aligned = addr;
#ifdef CONFIG_ARM64 //ARM32 compatible
	va_temp = (unsigned long)pVir_addr;
#else
	va_temp = (unsigned int)pVir_addr;
#endif
	pVir_addr_aligned = (unsigned int*)va_temp;

#ifdef CONFIG_BW_96B_ALIGNED
	size_aligned = dvr_size_alignment(size);
	addr_aligned = (unsigned int)dvr_memory_alignment((unsigned long)addr, size_aligned);
#ifdef CONFIG_ARM64 //ARM32 compatible
	va_temp = (unsigned long)pVir_addr;
#else //CONFIG_ARM64
	va_temp = (unsigned int)pVir_addr;
#endif //CONFIG_ARM64
	pVir_addr_aligned = (unsigned int*)dvr_memory_alignment((unsigned long)va_temp, size_aligned);
#endif


	ret = drvif_color_icm_Write3DTable_By_DMA(pIcm_tab_elem->elem, addr_aligned, (unsigned int*)pVir_addr_aligned, hue_section.pillar_num + 1, sat_section.pillar_num, itn_section.pillar_num);
	dvr_unmap_memory((void *)pVir_addr, size);

	if(ret ==false)
	{
		//drvif_color_Icm_Enable(0);
		rtd_pr_vpq_emerg("ICM : load tv006 icm fail\n");

	}


	drvif_color_Icm_Enable(1);
}

unsigned char fwif_color_set_YUV2RGB_UV_Offset_tv006(unsigned char display, unsigned char enable, unsigned char mode, DRV_VIP_YUV2RGB_UV_Offset *pUVOffset, DRV_VIP_YUV2RGB_UV_Offset_byUV_CURVE *uv_offset_byuv_curve)
{
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if (NULL == pUVOffset || NULL == uv_offset_byuv_curve)
		return 0;

	if (mode > 2)	/*0: AND 1:OR 2:Independent*/
		mode = 2;

	if (display == SLR_MAIN_DISPLAY) {
		#if 0 /*========JUST FOR DDTS TEST ONLY===================*/
		{
		unsigned short index_tmp[16] = {0};
		unsigned short gain_tmp[17] = {4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,};
		UINT8 Uofset_tmp[17] = {0x00, 0x00, 0x07, 0x14, 0x2c, 0x39, 0x40, 0x46, 0x49, 0x4f, 0x50, 0x50, 0x50, 0x50, 0x46, 0x3c, 0x20, }; //Uoffset
		UINT8 step_tmp[17] = {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, }; //Ustep

		UINT8 i;

		mode = 2;
		for (i = 0; i < 15; i++) index_tmp[i] = (i + 1) * 512;
		index_tmp[15] = 0x1f00;	/*max, need data protect*/
		for (i = 0; i < 17; i++) gain_tmp[i] = gain_tmp[i] >> (2 * (i + 1));
		#ifdef CONFIG_ARM64
		memcpy_toio(&(drv_vipCSMatrix_t.UV_Offset_byUV_CURVE.gain), gain_tmp, sizeof(short) * 17);
		memcpy_toio(&(drv_vipCSMatrix_t.UV_Offset_byUV_CURVE.gainCurve_index), index_tmp, sizeof(short) * 16);
		memcpy_toio(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.UV_Offset_byUV_CURVE), &(drv_vipCSMatrix_t.UV_Offset_byUV_CURVE), sizeof(DRV_VIP_YUV2RGB_UV_Offset_byUV_CURVE));
		memcpy_toio(&(pUVOffset->Ustep), step_tmp, sizeof(char) * 17);
		memcpy_toio(&(pUVOffset->Vstep), step_tmp, sizeof(char) * 17);
		memcpy_toio(&(pUVOffset->Uoffset), Uofset_tmp, sizeof(char) * 17);
		#else
		memcpy(&(drv_vipCSMatrix_t.UV_Offset_byUV_CURVE.gain), gain_tmp, sizeof(short) * 17);
		memcpy(&(drv_vipCSMatrix_t.UV_Offset_byUV_CURVE.gainCurve_index), index_tmp, sizeof(short) * 16);
		memcpy(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.UV_Offset_byUV_CURVE), &(drv_vipCSMatrix_t.UV_Offset_byUV_CURVE), sizeof(DRV_VIP_YUV2RGB_UV_Offset_byUV_CURVE));
		memcpy(&(pUVOffset->Ustep), step_tmp, sizeof(char) * 17);
		memcpy(&(pUVOffset->Vstep), step_tmp, sizeof(char) * 17);
		memcpy(&(pUVOffset->Uoffset), Uofset_tmp, sizeof(char) * 17);
		#endif
		}
		#endif

		drv_vipCSMatrix_t.CTRL_ITEM.UVOffset_En = enable;
		drv_vipCSMatrix_t.CTRL_ITEM.UVOffset_Mode_Ctrl = mode;	/*0: AND 1:OR 2:Independent*/
		drv_vipCSMatrix_t.CTRL_ITEM.UVOffset_Mode_byUV_byY = 1;	/*force enable UV offset by Y by UV gain*/
		/* record to share mem*/
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.CTRL_ITEM.UVOffset_En = enable;
		VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.CTRL_ITEM.UVOffset_Mode_Ctrl = mode;
#ifdef CONFIG_ARM64
		memcpy_toio(&drv_vipCSMatrix_t.UV_Offset, pUVOffset, sizeof(DRV_VIP_YUV2RGB_UV_Offset));
		memcpy_toio(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.UV_Offset), pUVOffset, sizeof(DRV_VIP_YUV2RGB_UV_Offset));
		memcpy_toio(&(drv_vipCSMatrix_t.UV_Offset_byUV_CURVE), uv_offset_byuv_curve, sizeof(DRV_VIP_YUV2RGB_UV_Offset_byUV_CURVE));
		memcpy_toio(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.UV_Offset_byUV_CURVE), &(drv_vipCSMatrix_t.UV_Offset_byUV_CURVE), sizeof(DRV_VIP_YUV2RGB_UV_Offset_byUV_CURVE));
#else
		memcpy(&drv_vipCSMatrix_t.UV_Offset, pUVOffset, sizeof(DRV_VIP_YUV2RGB_UV_Offset));
		memcpy(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.UV_Offset), pUVOffset, sizeof(DRV_VIP_YUV2RGB_UV_Offset));
		memcpy(&(drv_vipCSMatrix_t.UV_Offset_byUV_CURVE), uv_offset_byuv_curve, sizeof(DRV_VIP_YUV2RGB_UV_Offset_byUV_CURVE));
		memcpy(&(VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.UV_Offset_byUV_CURVE), &(drv_vipCSMatrix_t.UV_Offset_byUV_CURVE), sizeof(DRV_VIP_YUV2RGB_UV_Offset_byUV_CURVE));
#endif

		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_UVOffset_ctrl, VIP_YUV2RGB_Y_Seg_Max, display, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_UVOffset_Coef, VIP_YUV2RGB_Y_Seg_Max, display, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_UVOffset_Index, VIP_YUV2RGB_Y_Seg_Max, display, VIP_Table_Select_1);
	} else {
		VIPprintf("~No Sub~ return\n");
	}

	return 1;
}

#ifdef CONFIG_SCALER_ENABLE_V4L2
void fwif_color_CM_v4l2_data_check_tv006(CHIP_CM_RESULT_T *v4l2_data)
{
	if (cm_data_debug) {
		UINT16 i, j;

		/* CM data */
		for (i = 0; i < CHIP_CM_COLOR_REGION_MAX; i++) {
			printk("CM data, region[%d]\n", v4l2_data->stCMColorRegion.stColorRegion[i].regionNum);
			printk("CM data, Enable[%d]\n", v4l2_data->stCMColorRegion.stColorRegion[i].enable);

			printk("CM data, hue_pillar = [");
			for (j = 0; j < CHIP_HUE_TBLPOINT; j++)
				printk("%d, ", v4l2_data->stCMColorRegion.stColorRegion[i].hue_x[j]);
			printk("]\n");
			printk("CM data, hue_gain = [");
			for (j = 0; j < CHIP_HUE_TBLPOINT; j++)
				printk("%d, ", v4l2_data->stCMColorRegion.stColorRegion[i].hue_g[j]);
			printk("]\n");

			printk("CM data, sat_pillar = [0, ");
			for (j = 0; j < CHIP_SAT_TBLPOINT; j++)
				printk("%d, ", v4l2_data->stCMColorRegion.stColorRegion[i].sat_x[j]);
			printk("]\n");
			printk("CM data, sat_gain = [128, ");
			for (j = 0; j < CHIP_SAT_TBLPOINT; j++)
				printk("%d, ", v4l2_data->stCMColorRegion.stColorRegion[i].sat_g[j]);
			printk("]\n");

			printk("CM data, int_pillar = [0, ");
			for (j = 0; j < CHIP_INT_TBLPOINT; j++)
				printk("%d, ", v4l2_data->stCMColorRegion.stColorRegion[i].Int_x[j]);
			printk("]\n");
			printk("CM data, int_gain = [128, ");
			for (j = 0; j < CHIP_INT_TBLPOINT; j++)
				printk("%d, ", v4l2_data->stCMColorRegion.stColorRegion[i].Int_g[j]);
			printk("]\n");

			printk("CM data, r_offset_by_hue = [");
			for (j = 0; j < CHIP_HUE_TBLPOINT; j++)
				printk("%d, ", v4l2_data->stCMColorRegion.stColorRegion[i].r_offset_by_hue[j]);
			printk("]\n");

			printk("CM data, g_offset_by_hue = [");
			for (j = 0; j < CHIP_HUE_TBLPOINT; j++)
				printk("%d, ", v4l2_data->stCMColorRegion.stColorRegion[i].g_offset_by_hue[j]);
			printk("]\n");

			printk("CM data, b_offset_by_hue = [");
			for (j = 0; j < CHIP_HUE_TBLPOINT; j++)
				printk("%d, ", v4l2_data->stCMColorRegion.stColorRegion[i].b_offset_by_hue[j]);
			printk("]\n");
		}

		printk("CM data, satbysat_global = [128, ");
		for (i = 0; i < CHIP_SAT_TBLPOINT; i++)
			printk("%d, ", v4l2_data->stCMColorRegion.satbysat_global[i]);
		printk("]\n");
		printk("CM data, satbyInt_global = [128, ");
		for (i = 0; i < CHIP_INT_TBLPOINT; i++)
			printk("%d, ", v4l2_data->stCMColorRegion.satbyInt_global[i]);
		printk("]\n");

		printk("CM data, gain_by_sat_for_rgb_offset = [32, ");
		for (i = 0; i < CHIP_SAT_TBLPOINT; i++)
			printk("%d, ", v4l2_data->stCMColorRegion.gain_by_sat_for_rgb_offset[i]);
		printk("]\n");
		printk("CM data, gain_by_int_for_rgb_offset = [32, ");
		for (i = 0; i < CHIP_INT_TBLPOINT; i++)
			printk("%d, ", v4l2_data->stCMColorRegion.gain_by_int_for_rgb_offset[i]);
		printk("]\n");

		printk("CM data, keepY = [%d]\n", v4l2_data->stCMColorRegion.keepY);


		/* CM gain and AI */
		for (i = 0; i < CHIP_CM_COLOR_REGION_MAX; i++) {
			printk("CM data, Gain, gr[%d] = [%d, %d, [%d, %d, %d]]\n", i,
			v4l2_data->stCMColorGain.stFinalColorGain[i].masterGain,
			v4l2_data->stCMColorGain.stFinalColorGain[i].RGBOffset_masterGain,
			v4l2_data->stCMColorGain.stFinalColorGain[i].stColorGain.stColorGainHue,
			v4l2_data->stCMColorGain.stFinalColorGain[i].stColorGain.stColorGainSat,
			v4l2_data->stCMColorGain.stFinalColorGain[i].stColorGain.stColorGainInt);
		}

		printk("\nCM data, Gain, sat_point = [");
		for (i = 0; i < CHIP_SATURATION_NUM; i++)
			printk("%d, ", v4l2_data->stCMColorGain.stSaturationPoint[i]);
		printk("]\n");
		printk("\nCM data, Gain, sat_gain = [");
		for (i = 0; i < CHIP_SATURATION_NUM; i++)
			printk("%d, ", v4l2_data->stCMColorGain.stSaturationGain[i]);
		printk("]\n");

		printk("CM data, Gain, AiOffset[%d] = [%d, %d, %d]\n", 0,
			v4l2_data->stCMColorGain.stAISkinGain.stAiOffset.stAIHueOffset,
			v4l2_data->stCMColorGain.stAISkinGain.stAiOffset.stAISaturationOffset,
			v4l2_data->stCMColorGain.stAISkinGain.stAiOffset.stAIIntensityOffset);

		printk("\nCM data, Gain, AiIcmCurve = [");
		for (i = 0; i < CHIP_AI_UV_DISTANCE_NUM; i++)
			printk("%d, ", v4l2_data->stCMColorGain.stAISkinGain.stAiIcmCurve[i]);
		printk("]\n");

		/* clear white */
		printk("CM data, CW, enable = [%d]\n", v4l2_data->stCMClearWhite.enable);
		printk("CM data, CW, UV_offset_index = [");
		for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max - 1; i++)
			printk("%d, ", v4l2_data->stCMClearWhite.UV_offset_index[i]);
		printk("]\n");

		printk("CM data, CW, Uoffset = [");
		for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++)
			printk("%d, ", v4l2_data->stCMClearWhite.Uoffset[i]);
		printk("]\n");
		printk("CM data, CW, Voffset = [");
		for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++)
			printk("%d, ", v4l2_data->stCMClearWhite.Voffset[i]);
		printk("]\n");

		printk("CM data, CW, gain_by_UV_index = [");
		for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max - 1; i++)
			printk("%d, ", v4l2_data->stCMClearWhite.gain_by_UV_index[i]);
		printk("]\n");

		printk("CM data, CW, gain_by_UV = [");
		for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++)
			printk("%d, ", v4l2_data->stCMClearWhite.gain_by_UV[i]);
		printk("]\n");
	}

}

void fwif_color_set_CM_v4l2_data_to_RTK_table_tv006(CHIP_CM_RESULT_T *v4l2_data)
{
	UINT16 i, idx;
	extern UINT8 g_Color_Mode;
	extern CHIP_CM_REGION_T g_cm_rgn_adv;
	extern CHIP_CM_REGION_T g_cm_rgn_exp;
	extern CHIP_COLOR_CONTROL_T g_cm_ctrl_adv;
	extern CHIP_COLOR_CONTROL_T g_cm_ctrl_exp;
	CHIP_CM_REGION_T *cm_rgn_tmp;
	CHIP_COLOR_CONTROL_T *cm_ctrl_tmp;
	extern DRV_VIP_YUV2RGB_UV_Offset uvOffset;
	extern DRV_VIP_YUV2RGB_UV_Offset_byUV_CURVE uv_offset_byuv_curve;
	extern UINT8 v4l2_UV_offset_enable;
	extern VPQ_SAT_LUT_T satLUTData_in_CM;

	if (g_Color_Mode == COLOR_MODE_EXP) {
		cm_rgn_tmp = &g_cm_rgn_exp;
		cm_ctrl_tmp = &g_cm_ctrl_exp;
	} else {
		cm_rgn_tmp = &g_cm_rgn_adv;
		cm_ctrl_tmp = &g_cm_ctrl_adv;
	}

	/*======= CM data =======*/ /*======= CM data =======*/ /*======= CM data =======*/
	for (i = 0; i < CHIP_CM_COLOR_REGION_MAX; i++) {
		cm_rgn_tmp->stColorRegionType.stColorRegion[i].enable = v4l2_data->stCMColorRegion.stColorRegion[i].enable;
		cm_rgn_tmp->stColorRegionType.stColorRegion[i].regionNum = v4l2_data->stCMColorRegion.stColorRegion[i].regionNum;

		/* hue */
		memcpy(cm_rgn_tmp->stColorRegionType.stColorRegion[i].hue_x, v4l2_data->stCMColorRegion.stColorRegion[i].hue_x, CHIP_HUE_TBLPOINT * sizeof(UINT16));
		memcpy(cm_rgn_tmp->stColorRegionType.stColorRegion[i].hue_g, v4l2_data->stCMColorRegion.stColorRegion[i].hue_g, CHIP_HUE_TBLPOINT * sizeof(UINT16));

		/* sat */
		cm_rgn_tmp->stColorRegionType.stColorRegion[i].sat_x[0] = 0;	/* sat 0 protect,and LG DB no set */
		memcpy(&(cm_rgn_tmp->stColorRegionType.stColorRegion[i].sat_x[1]), v4l2_data->stCMColorRegion.stColorRegion[i].sat_x, CHIP_SAT_TBLPOINT * sizeof(UINT16));
		cm_rgn_tmp->stColorRegionType.stColorRegion[i].sat_g[0] = 128;	/* sat 0 protect,and LG DB no set */
		memcpy(&(cm_rgn_tmp->stColorRegionType.stColorRegion[i].sat_g[1]), v4l2_data->stCMColorRegion.stColorRegion[i].sat_g, CHIP_SAT_TBLPOINT * sizeof(UINT16));

		/* int */
		cm_rgn_tmp->stColorRegionType.stColorRegion[i].Int_x[0] = 0;	/* int 0 protect,and LG DB no set */
		memcpy(&(cm_rgn_tmp->stColorRegionType.stColorRegion[i].Int_x[1]), v4l2_data->stCMColorRegion.stColorRegion[i].Int_x, CHIP_INT_TBLPOINT * sizeof(UINT16));
		cm_rgn_tmp->stColorRegionType.stColorRegion[i].Int_g[0] = 128;	/* int 0 protect,and LG DB no set */
		memcpy(&(cm_rgn_tmp->stColorRegionType.stColorRegion[i].Int_g[1]), v4l2_data->stCMColorRegion.stColorRegion[i].Int_g, CHIP_INT_TBLPOINT * sizeof(UINT16));

		/* rgb offset */
		memcpy(cm_rgn_tmp->stColorRegionType.stColorRegion[i].r_offset_by_hue, v4l2_data->stCMColorRegion.stColorRegion[i].r_offset_by_hue, CHIP_HUE_TBLPOINT * sizeof(UINT16));
		memcpy(cm_rgn_tmp->stColorRegionType.stColorRegion[i].g_offset_by_hue, v4l2_data->stCMColorRegion.stColorRegion[i].g_offset_by_hue, CHIP_HUE_TBLPOINT * sizeof(UINT16));
		memcpy(cm_rgn_tmp->stColorRegionType.stColorRegion[i].b_offset_by_hue, v4l2_data->stCMColorRegion.stColorRegion[i].b_offset_by_hue, CHIP_HUE_TBLPOINT * sizeof(UINT16));
	}

	/* satbysat */
	cm_rgn_tmp->stColorRegionType.satbysat_global[0] = 128;	/* sat 0 protect,and LG DB no set */
	memcpy(&(cm_rgn_tmp->stColorRegionType.satbysat_global[1]), v4l2_data->stCMColorRegion.satbysat_global, CHIP_SAT_TBLPOINT * sizeof(UINT16));

	/* saybyint */
	cm_rgn_tmp->stColorRegionType.satbyInt_global[0] = 128;	/* sat 0 protect,and LG DB no set */
	memcpy(&(cm_rgn_tmp->stColorRegionType.satbyInt_global[1]), v4l2_data->stCMColorRegion.satbyInt_global, CHIP_INT_TBLPOINT * sizeof(UINT16));

	/* rgb offset, gain by sat */
	cm_rgn_tmp->stColorRegionType.gain_by_sat_for_rgb_offset[0] = 32;	/* sat 0 protect,and LG DB no set */
	memcpy(&(cm_rgn_tmp->stColorRegionType.gain_by_sat_for_rgb_offset[1]), v4l2_data->stCMColorRegion.gain_by_sat_for_rgb_offset, CHIP_SAT_TBLPOINT * sizeof(UINT8));

	/* rgb offset, gain by int */
	cm_rgn_tmp->stColorRegionType.gain_by_int_for_rgb_offset[0] = 32;	/* sat 0 protect,and LG DB no set */
	memcpy(&(cm_rgn_tmp->stColorRegionType.gain_by_int_for_rgb_offset[1]), v4l2_data->stCMColorRegion.gain_by_int_for_rgb_offset, CHIP_INT_TBLPOINT * sizeof(UINT8));

	/* keep Y */
	cm_rgn_tmp->stColorRegionType.keepY = v4l2_data->stCMColorRegion.keepY;
	/*------- CM data -------*/ /*------- CM data -------*/ /*------- CM data -------*/


	/*======= CM gain and AI =======*/ /*======= CM gain and AI =======*/ /*======= CM gain and AI =======*/
	memcpy(cm_ctrl_tmp, v4l2_data->stCMColorGain.stFinalColorGain, sizeof(CHIP_COLOR_CONTROL_T));
	/*------- CM gain and AI -------*/ /*------- CM gain and AI -------*/ /*------- CM gain and AI -------*/

	//DSE
	for(idx=0;idx<CHIP_SATURATION_NUM;idx++) {
		satLUTData_in_CM.uSaturationY[idx] = v4l2_data->stCMColorGain.stSaturationGain[idx];
		//satLUTData_in_CM.uSaturationX[idx] = g_cm_v4l2.stCMColorGain.stSaturationPoint[idx];	// DB data still need to check
	}

	satLUTData_in_CM.uSaturationY[16] = satLUTData_in_CM.uSaturationY[15];
	satLUTData_in_CM.uSaturationY[15] = (satLUTData_in_CM.uSaturationY[14] +satLUTData_in_CM.uSaturationY[16]) >> 1;

	/*======= clear white =======*/ /*======= clear white =======*/ /*======= clear white =======*/
	v4l2_UV_offset_enable = v4l2_data->stCMClearWhite.enable;
	//memcpy(&(uvOffset.UV_index[0]), v4l2_data->stCMClearWhite.UV_offset_index, sizeof(unsigned char)*(VIP_YUV2RGB_Y_Seg_Max - 1));
	uvOffset.UV_index[VIP_YUV2RGB_Y_Seg_Max-2] = v4l2_data->stCMClearWhite.UV_offset_index[VIP_YUV2RGB_Y_Seg_Max-2];
	for(idx=0;idx<(VIP_YUV2RGB_Y_Seg_Max-2);idx++)
		uvOffset.UV_index[idx] = v4l2_data->stCMClearWhite.UV_offset_index[idx] - 1;
	memcpy(&(uvOffset.Uoffset[0]), v4l2_data->stCMClearWhite.Uoffset, sizeof(unsigned char)*VIP_YUV2RGB_Y_Seg_Max);
	memcpy(&(uvOffset.Voffset[0]), v4l2_data->stCMClearWhite.Voffset, sizeof(unsigned char)*VIP_YUV2RGB_Y_Seg_Max);
	memcpy(&(uv_offset_byuv_curve.gainCurve_index[0]), v4l2_data->stCMClearWhite.gain_by_UV_index, sizeof(unsigned short)*(VIP_YUV2RGB_Y_Seg_Max - 1));
	memcpy(&(uv_offset_byuv_curve.gain[0]), v4l2_data->stCMClearWhite.gain_by_UV, sizeof(unsigned short)*VIP_YUV2RGB_Y_Seg_Max);
	/*------- clear white -------*/ /*------- clear white -------*/ /*------- clear white -------*/

	// lesley 1014
	#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	{
	extern void drvif_color_get_DB_AI_ICM(CHIP_CM_RESULT_T *v4l2_data);
	drvif_color_get_DB_AI_ICM(v4l2_data);
	}
	#endif
	// end lesley 1014

}
#endif

unsigned char fwif_color_RTK_PQ_demo_check_tv006(VPQ_SetPicCtrl_T *p_setPicCtrl)
{
	unsigned char buf[1] = {1};

	/*
	printk("\nfwif_color_RTK_PQ_demo_check_tv006:\n");
	printk("p_setPicCtrl->siVal[PIC_CTRL_Contrast] = %d\n", p_setPicCtrl->siVal[PIC_CTRL_Contrast]);
	printk("p_setPicCtrl->siVal[PIC_CTRL_Brightness] = %d\n", p_setPicCtrl->siVal[PIC_CTRL_Brightness]);
	printk("p_setPicCtrl->siVal[PIC_CTRL_Color] = %d\n", p_setPicCtrl->siVal[PIC_CTRL_Color]);
	printk("p_setPicCtrl->siVal[PIC_CTRL_Tint] = %d\n", p_setPicCtrl->siVal[PIC_CTRL_Tint]);
	printk("p_setPicCtrl->Gain_Val[PIC_CTRL_Contrast] = %d\n", p_setPicCtrl->Gain_Val[PIC_CTRL_Contrast]);
	printk("p_setPicCtrl->Gain_Val[PIC_CTRL_Brightness] = %d\n", p_setPicCtrl->Gain_Val[PIC_CTRL_Brightness]);
	printk("p_setPicCtrl->Gain_Val[PIC_CTRL_Color] = %d\n", p_setPicCtrl->Gain_Val[PIC_CTRL_Color]);
	printk("p_setPicCtrl->Gain_Val[PIC_CTRL_Tint] = %d\n", p_setPicCtrl->Gain_Val[PIC_CTRL_Tint]);
	*/

	if (p_setPicCtrl->siVal[PIC_CTRL_Contrast] == 99
		&& p_setPicCtrl->siVal[PIC_CTRL_Brightness] == 49
		&& p_setPicCtrl->siVal[PIC_CTRL_Tint] == 9)
	{
		rtd_pr_vpq_debug("\nfwif_color_RTK_PQ_demo_check_tv006  DEMO~~~~~\n");

		(fwif_color_get_demo_callback(DEMO_CALLBACKID_ON_OFF_SWITCH))((void *)buf);
		return TRUE;
	}
	return FALSE;

}

unsigned char fwif_color_get_Decide_YUV2RGB_TBL_Index_tv006(unsigned char src_idx, unsigned char display, unsigned char Input_Data_Mode)
{
	unsigned char ret_idx;

	switch (Input_Data_Mode) {
		case  YUV2RGB_INPUT_601_Limted:
		case  YUV2RGB_INPUT_601_Full:
			ret_idx = YUV2RGB_TBL_SELECT_601_Limted_235_240_to_255;
			break;
		case  YUV2RGB_INPUT_709_Limted:
		case  YUV2RGB_INPUT_709_Full:
			ret_idx = YUV2RGB_TBL_SELECT_709_Limted_235_240_to_255;
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

int fwif_color_set_Picture_Control_tv006(unsigned char display, VPQ_SetPicCtrl_T *p_setPicCtrl)
{
	int ret =0;
	_system_setting_info *system_info_struct = NULL;

	system_info_struct = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (system_info_struct == NULL) {
		VIPprintf("~get shareMem | gvipTbl error return~\n");
		return -1;
	}

	/*remove special demo mode 20160129*/
	//if (fwif_color_RTK_PQ_demo_check_tv006(p_setPicCtrl))
	//	return -1;

	/* get osd and gain value into share mem for con bri color tint*/

	//fwif_color_CalCon_Bri_Color_Tint_Gain_tv006(display, p_setPicCtrl, system_info_struct);
	//ret = fwif_color_setCon_Bri_Color_Tint_tv006(p_setPicCtrl->wId, system_info_struct);

	if (display == SLR_MAIN_DISPLAY) {
		fwif_color_CalCon_Bri_Color_Tint_Gain_tv006(SLR_MAIN_DISPLAY, p_setPicCtrl, system_info_struct);
		ret = fwif_color_setCon_Bri_Color_Tint_tv006(SLR_MAIN_DISPLAY, system_info_struct);
		fwif_color_CalCon_Bri_Color_Tint_Gain_tv006(SLR_SUB_DISPLAY, p_setPicCtrl, system_info_struct);
		fwif_color_setCon_Bri_Color_Tint_tv006(SLR_SUB_DISPLAY, system_info_struct);
	} else if (display == SLR_SUB_DISPLAY) {
		fwif_color_CalCon_Bri_Color_Tint_Gain_tv006(SLR_SUB_DISPLAY, p_setPicCtrl, system_info_struct);
		fwif_color_setCon_Bri_Color_Tint_tv006(SLR_SUB_DISPLAY, system_info_struct);
	}

	return ret;
}
DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
unsigned int table_idx[3][3][VIP_YUV2RGB_Y_Seg_Max];
VPQ_SAT_LUT_T sat_LUT, sat_gain, total_sat_gain;

int fwif_color_setCon_Bri_Color_Tint_tv006(unsigned char display, _system_setting_info *system_info_struct)
{
	SLR_VIP_TABLE *gVip_Table = NULL;

	unsigned char coef_level;
	unsigned char RGB_level;
	unsigned char tbl_select;
	unsigned char i, j, k; /*, Y_Clamp;*/
	/*unsigned short *table_idx[3][3];*/
	short satTmp[3][3], hueTmp[3][3], kTmp[3][3], kTmp1[3][3];
	unsigned int satCmps;
	short cmps = 0;
	short contrast, bri, color;
	short deg_tint;
	int aplColorGain;
	unsigned int con_gain_max, color_gain_max;
	/*VPQ_SAT_LUT_T *sat_LUT = NULL;*/ /* DSE function */
	unsigned char Input_mode, YUV2RGB_TBL_Idx;
	int tbl_offset_R, tbl_offset_G, tbl_offset_B;

	unsigned char bt2020_Constant_En;
	short bt2020_k13, bt2020_k32;

	gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (system_info_struct == NULL || gVip_Table == NULL) {
		VIPprintf("~get shareMem | gvipTbl error return~\n");
		return -1;
	}

	memset(satTmp, 0, sizeof(satTmp));
	memset(hueTmp, 0, sizeof(hueTmp));
	memset(kTmp, 0, sizeof(kTmp));
	memset(kTmp1, 0, sizeof(kTmp1));

	coef_level = system_info_struct->using_YUV2RGB_Matrix_Info.CoefByY_CtrlItem_Level;
	RGB_level = system_info_struct->using_YUV2RGB_Matrix_Info.RGB_Offset_Level;
	tbl_select = system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_TBL_Select;

	if (coef_level >=  VIP_YUV2RGB_LEVEL_SELECT_MAX || RGB_level >=  VIP_YUV2RGB_LEVEL_SELECT_MAX || tbl_select >=  YUV2RGB_TBL_Num) {
		VIPprintf("~level or table select Error return, level = %d, table select = %d, %s->%d, %s~\n", coef_level, tbl_select, __FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	Input_mode = system_info_struct->using_YUV2RGB_Matrix_Info.Input_Data_Mode;
	YUV2RGB_TBL_Idx = fwif_color_get_Decide_YUV2RGB_TBL_Index_tv006(0, display, Input_mode);

	if (display == SLR_MAIN_DISPLAY) {
		/* contrast, brightness, tint, color gain get from share memory */
		contrast = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Contrast_Gain, 0);
		bri = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Brightness_Gain, 0);
		color = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Saturation_Gain, 0);
		deg_tint = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Hue_Gain, 0);

		if (OSD_Contrast_Compensation) {
			fwif_color_CalContrast_Compensation_tv006(display, contrast, system_info_struct);
			cmps = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Brightness_Cmps, 0);
			satCmps = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Saturation_Cmps, 0);
		} else {
			cmps = 0;
			satCmps = 128;
		}

		/* get DSE saturation Gain LUT */
		/*fwif_color_get_SaturationLUT_tv006(SLR_MAIN_DISPLAY, &sat_LUT);*/
		fwif_color_access_SaturationLUT_tv006(SLR_MAIN_DISPLAY, &sat_LUT, ACCESS_MODE_GET);

		/* for K13_2, K32_2*/
		if (system_info_struct->BT2020_CTRL.Enable_Flag == 1 && system_info_struct->BT2020_CTRL.Mode == BT2020_MODE_Constant)
			bt2020_Constant_En = 1;
		else
			bt2020_Constant_En = 0;

		VIPprintf("~tv006~,cmps=%d,satCmps=%d\n", cmps, satCmps);
		VIPprintf("~tv006_gain, con = %d, bri = %d, color = %d, tint = %d~\n", contrast, bri, color, deg_tint);
		VIPprintf("~sat_LUT[0]=%d, sat_LUT[5]=%d, sat_LUT[12]=%d, sat_LUT[16]=%d~\n", sat_LUT.uSaturationY[0], sat_LUT.uSaturationY[5], sat_LUT.uSaturationY[12], sat_LUT.uSaturationY[16]);
	} else {
		/* contrast, brightness, tint, color gain get from share memory */
		contrast = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Contrast_Gain_Sub, 0);
		bri = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Brightness_Gain_Sub, 0);
		color = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Saturation_Gain_Sub, 0);
		deg_tint = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Hue_Gain_Sub, 0);

		if (OSD_Contrast_Compensation) {
			fwif_color_CalContrast_Compensation_tv006(display, contrast, system_info_struct);
			cmps = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Brightness_Cmps_Sub, 0);
			satCmps = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Saturation_Cmps_Sub, 0);
		} else {
			cmps = 0;
			satCmps = 128;
		}

		/* get DSE saturation Gain LUT */
		/*fwif_color_get_SaturationLUT_tv006(SLR_MAIN_DISPLAY, &sat_LUT);*/ /* No DSE for sub*/
		for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++) {
			sat_LUT.uSaturationY[i] = 128; /*0x80 = gain 1*/
		}

		/* for K13_2, K32_2*/
		if (system_info_struct->BT2020_CTRL.Enable_Flag_Sub == 1 && system_info_struct->BT2020_CTRL.Mode_Sub == BT2020_MODE_Constant)
			bt2020_Constant_En = 1;
		else
			bt2020_Constant_En = 0;

		VIPprintf("~sub,tv006~,cmps=%d,satCmps=%d\n", cmps, satCmps);
		VIPprintf("~sub,tv006_gain, con = %d, bri = %d, color = %d, tint = %d~\n", contrast, bri, color, deg_tint);
		/*VIPprintf("~sub,sat_LUT[0]=%d, sat_LUT[5]=%d, sat_LUT[12]=%d, sat_LUT[16]=%d~\n", sat_LUT.uSaturationY[0], sat_LUT.uSaturationY[5], sat_LUT.uSaturationY[12], sat_LUT.uSaturationY[16]);*/
	}

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
	for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++) {
		sat_gain.uSaturationY[i] = color;
	}
	fwif_color_access_Sat_Gain_tv006(display, &sat_gain, ACCESS_MODE_WRITE);

	/*color = (color > 128) ?  75 + (color*53 / 128) : color;*/
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

	/*APL color gain*/
	if (0 == fwif_color_access_apl_color_gain_tv006(display, &aplColorGain, ACCESS_MODE_READ))
		color = color * aplColorGain / 100;

	/*matrix computation for contrast, sat and hue*/
	for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++) {
		con_gain_max = 0;
		color_gain_max = 0;

		/*DSE function, gain by saturation LUT*/
		satTmp[1][1] = (color * sat_LUT.uSaturationY[i]) >> 7;
		satTmp[2][2] = satTmp[1][1];
		VIPprintf("~DSE gain,%d,%d\n", satTmp[1][1], satTmp[2][2]);

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

		/* get max coef for max color gain calculation*/
		for (j = 0; j < 3; j++) {
			for (k = 1; k < 3; k++) {
				if (kTmp[j][k] >= 0) {
					if (kTmp[j][k] > color_gain_max)
						color_gain_max = kTmp[j][k];
				} else {
					if ((kTmp[j][k] * (-1)) > color_gain_max)
						color_gain_max = (kTmp[j][k] * (-1));
				}
			}
		}
		con_gain_max = kTmp[0][0];
		VIPprintf("~max coef,%d,%d\n", con_gain_max, color_gain_max);

		/* max contrast gain and color gain calculation*/
		if (con_gain_max == 0)
			con_gain_max = 1;
		con_gain_max = (VIP_YUV2RGB_K_Y_NEG_BND << 7) / con_gain_max;		/*(0x3FF/con_gain_max) << 7;*/	/* Max Gain : Kxx: S(11, 8) */

		if (color_gain_max == 0)
			color_gain_max = 1;
		color_gain_max = (VIP_YUV2RGB_K_C_NEG_BND << 7) / color_gain_max;	/*(0x3FF/color_gain_max) << 7;*/	/* Max Gain : Kxx: S(11, 8) */

		/* check gain by max gain*/
		if (contrast > con_gain_max)
			contrast = con_gain_max;
		if (satTmp[1][1] > color_gain_max) {
			satTmp[1][1] = color_gain_max;
			satTmp[2][2] = color_gain_max;
		}
		/* record final sat gain curve for tool, elieli*/
		total_sat_gain.uSaturationY[i] = satTmp[1][1];
		VIPprintf("~f_G,%d,%d,%d\n", contrast, satTmp[1][1], satTmp[2][2]);

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
	/*drv_vipCSMatrix_t.RGB_Offset.R_offset = (((bri - 128) << 5) + (cmps << 4) + tbl_offset_R) & 0x7fff;
	drv_vipCSMatrix_t.RGB_Offset.G_offset = (((bri - 128) << 5) + (cmps << 4) + tbl_offset_G) & 0x7fff;
	drv_vipCSMatrix_t.RGB_Offset.B_offset = (((bri - 128) << 5) + (cmps << 4) + tbl_offset_B) & 0x7fff;*/
	/* remapping bri effect to more, elieli*/
	bri = bri - 128;
	bri = (bri * 150) >> 7;
	/* more bri effect for negative*/
	if (bri < 0)
		bri = (bri * 213) >> 7;

	drv_vipCSMatrix_t.RGB_Offset.R_offset = ((bri << 5) + (cmps << 4) + tbl_offset_R) & 0x7fff;
	drv_vipCSMatrix_t.RGB_Offset.G_offset = ((bri << 5) + (cmps << 4) + tbl_offset_G) & 0x7fff;
	drv_vipCSMatrix_t.RGB_Offset.B_offset = ((bri << 5) + (cmps << 4) + tbl_offset_B) & 0x7fff;

	/*Clamp setting*/
	/*drv_vipCSMatrix_t.CTRL_ITEM.Y_Clamp = Y_Clamp;*/
	drv_vipCSMatrix_t.CTRL_ITEM.Y_Clamp = 0;

	/*Y_index Setting*/ /*add from K3L*/
	for (j = 0; j < VIP_YUV2RGB_Y_Seg_Max-1; j++)
		drv_vipCSMatrix_t.COEF_By_Y.Y_index[j] = sat_LUT.uSaturationX[j];

	if (display == SLR_MAIN_DISPLAY) {
		/* record final sat gain curve for tool*/
		fwif_color_access_Total_Sat_Gain_tv006(display, &total_sat_gain, ACCESS_MODE_WRITE);
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
		/* drv setting*/
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Y_Clamp, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_Y_Only, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Offset_Only, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_CoefByY_Index, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		/* bt2020_Constant_En,  for bt 2020 k13, k32*/
		if (bt2020_Constant_En == 1) {
			/*drv setting*/
			drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_BT2020_Only, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
			/*memory setting*/
			fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.bt2020_K13), 1, 0);
			fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.bt2020_K32), 1, 0);
		}
	}
	else {
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Y_Clamp, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_Y_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Offset_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		if (bt2020_Constant_En == 1)
			drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_BT2020_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
	}
	return 0;
}

#if 0
unsigned char fwif_OsdMapToRegValue_tv006(APP_Video_OsdItemType_t bOsdItem, VPQ_SetPicCtrl_T *p_setPicCtrl)
{
	unsigned int temp = 0;
	unsigned char startPoint, endPoint;
	unsigned char reg_point0 = 0;
	unsigned char reg_point50 = 0;
	unsigned char reg_point100 = 0;
	INT16 value;

	switch (bOsdItem) {
	case APP_VIDEO_OSD_ITEM_BRIGHTNESS:
		value = p_setPicCtrl->siVal[PIC_CTRL_Brightness];
		reg_point0 = p_setPicCtrl->setPicCtrl_Mapping.Index_0[PIC_CTRL_Brightness];
		reg_point50 = p_setPicCtrl->setPicCtrl_Mapping.Index_50[PIC_CTRL_Brightness];
		reg_point100 = p_setPicCtrl->setPicCtrl_Mapping.Index_100[PIC_CTRL_Brightness];
		break;
	case APP_VIDEO_OSD_ITEM_CONTRAST:
		value = p_setPicCtrl->siVal[PIC_CTRL_Contrast];
		reg_point0 = p_setPicCtrl->setPicCtrl_Mapping.Index_0[PIC_CTRL_Contrast];
		reg_point50 = p_setPicCtrl->setPicCtrl_Mapping.Index_50[PIC_CTRL_Contrast];
		reg_point100 = p_setPicCtrl->setPicCtrl_Mapping.Index_100[PIC_CTRL_Contrast];
		break;
	case APP_VIDEO_OSD_ITEM_SATURTUION:
		value = p_setPicCtrl->siVal[PIC_CTRL_Color];
		reg_point0 = p_setPicCtrl->setPicCtrl_Mapping.Index_0[PIC_CTRL_Color];
		reg_point50 = p_setPicCtrl->setPicCtrl_Mapping.Index_50[PIC_CTRL_Color];
		reg_point100 = p_setPicCtrl->setPicCtrl_Mapping.Index_100[PIC_CTRL_Color];
		break;
	case APP_VIDEO_OSD_ITEM_HUE:
		value = p_setPicCtrl->siVal[PIC_CTRL_Tint];
		reg_point0 = p_setPicCtrl->setPicCtrl_Mapping.Index_0[PIC_CTRL_Tint];
		reg_point50 = p_setPicCtrl->setPicCtrl_Mapping.Index_50[PIC_CTRL_Tint];
		reg_point100 = p_setPicCtrl->setPicCtrl_Mapping.Index_100[PIC_CTRL_Tint];
		break;
	/*case APP_VIDEO_OSD_ITEM_SHARPNESS:*/
	default:
		value = p_setPicCtrl->siVal[PIC_CTRL_Contrast];
		reg_point0 = p_setPicCtrl->setPicCtrl_Mapping.Index_0[PIC_CTRL_Contrast];
		reg_point50 = p_setPicCtrl->setPicCtrl_Mapping.Index_50[PIC_CTRL_Contrast];
		reg_point100 = p_setPicCtrl->setPicCtrl_Mapping.Index_100[PIC_CTRL_Contrast];
		break;
	}

	if (value < 50) {
		startPoint = reg_point0;
		endPoint = reg_point50;
	} else {
		startPoint = reg_point50;
		endPoint = reg_point100;
		value -=  50;
	}

	if (endPoint >= startPoint) {
		temp = (unsigned char)(startPoint + (endPoint - startPoint)*value/50);
	} else if (endPoint < startPoint) {
		temp = (unsigned char)(startPoint - (startPoint-endPoint)*value/50);
	}

	if (temp > 255)
		temp = 255;

	VIPprintf("mapping = %d, %d, %d, %d, %d, %d \n", bOsdItem, value, temp, reg_point0, reg_point50, reg_point100);

	return (unsigned char)temp;

}
#endif

void fwif_color_CalContrast_Compensation_tv006(unsigned char display, unsigned char Contrast_Gain, _system_setting_info *system_info_struct)
{
	short sat_cmps_tmp, bri_cmps_tmp;
	unsigned short sat_cmps_gain = 128, bri_cmps_gain = 128;

	if (system_info_struct == NULL) {
		VIPprintf("~table NULL TBL=%x,[%s:%d], Return\n~", (unsigned int)system_info_struct, __FILE__, __LINE__);
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


int fwif_color_CalCon_Bri_Color_Tint_Gain_tv006(unsigned char display, VPQ_SetPicCtrl_T *p_setPicCtrl, _system_setting_info *system_info_struct)
{
	short contrast, bri, color;
	short deg_tint, tint;

	if (system_info_struct == NULL) {
		VIPprintf("~get shareMem | gvipTbl error return~\n");
		return -1;
	}

	if (display == SLR_MAIN_DISPLAY) {
		/* Record OSD value for contrast, brightness, hue, color*/
		system_info_struct->OSD_Info.OSD_Contrast = p_setPicCtrl->siVal[PIC_CTRL_Contrast];
		system_info_struct->OSD_Info.OSD_Brightness = p_setPicCtrl->siVal[PIC_CTRL_Brightness];
		system_info_struct->OSD_Info.OSD_Saturation = p_setPicCtrl->siVal[PIC_CTRL_Color];
		system_info_struct->OSD_Info.OSD_Hue = p_setPicCtrl->siVal[PIC_CTRL_Tint];

		/*cal mapping gain.*/
		contrast = p_setPicCtrl->Gain_Val[PIC_CTRL_Contrast];
		bri = p_setPicCtrl->Gain_Val[PIC_CTRL_Brightness];
		color = p_setPicCtrl->Gain_Val[PIC_CTRL_Color];
		tint = p_setPicCtrl->Gain_Val[PIC_CTRL_Tint];
		tint = tint - 128;
		if (tint >=  0)
			deg_tint = tint;
		else
			deg_tint = 360 - (tint * (-1));
		VIPprintf("~gain, con = %d, bri = %d, color = %d, tint = %d~\n", contrast, bri, color, deg_tint);

		/* Record gain value for contrast, brightness, hue, color*/
		system_info_struct->OSD_Info.Contrast_Gain = fwif_color_ChangeOneUINT16Endian(contrast, 0);
		system_info_struct->OSD_Info.Brightness_Gain = fwif_color_ChangeOneUINT16Endian(bri, 0);
		system_info_struct->OSD_Info.Saturation_Gain = fwif_color_ChangeOneUINT16Endian(color, 0);
		system_info_struct->OSD_Info.Hue_Gain = fwif_color_ChangeOneUINT16Endian(deg_tint, 0);

	} else {
		/* Record OSD value for contrast, brightness, hue, color*/
		system_info_struct->OSD_Info.OSD_Contrast_Sub= p_setPicCtrl->siVal[PIC_CTRL_Contrast];
		system_info_struct->OSD_Info.OSD_Brightness_Sub = p_setPicCtrl->siVal[PIC_CTRL_Brightness];
		system_info_struct->OSD_Info.OSD_Saturation_Sub = p_setPicCtrl->siVal[PIC_CTRL_Color];
		system_info_struct->OSD_Info.OSD_Hue_Sub = p_setPicCtrl->siVal[PIC_CTRL_Tint];

		/*cal mapping gain.*/
		contrast = p_setPicCtrl->Gain_Val[PIC_CTRL_Contrast];
		bri = p_setPicCtrl->Gain_Val[PIC_CTRL_Brightness];
		color = p_setPicCtrl->Gain_Val[PIC_CTRL_Color];
		tint = p_setPicCtrl->Gain_Val[PIC_CTRL_Tint];
		tint = tint - 128;
		if (tint >=  0)
			deg_tint = tint;
		else
			deg_tint = 360 - (tint * (-1));
		VIPprintf("~sub_gain, con = %d, bri = %d, color = %d, tint = %d~\n", contrast, bri, color, deg_tint);

		/* Record gain value for contrast, brightness, hue, color*/
		system_info_struct->OSD_Info.Contrast_Gain_Sub = fwif_color_ChangeOneUINT16Endian(contrast, 0);
		system_info_struct->OSD_Info.Brightness_Gain_Sub = fwif_color_ChangeOneUINT16Endian(bri, 0);
		system_info_struct->OSD_Info.Saturation_Gain_Sub = fwif_color_ChangeOneUINT16Endian(color, 0);
		system_info_struct->OSD_Info.Hue_Gain_Sub = fwif_color_ChangeOneUINT16Endian(deg_tint, 0);
	}
	return 0;
}
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
extern short bri_10bit;
int fwif_color_setCon_Bri_Color_Tint_tv006_AI(unsigned char display, _system_setting_info *system_info_struct)
{
	SLR_VIP_TABLE *gVip_Table = NULL;

	unsigned char coef_level;
	unsigned char RGB_level;
	unsigned char tbl_select;
	unsigned char i, j, k; /*, Y_Clamp;*/
	/*unsigned short *table_idx[3][3];*/
	short satTmp[3][3], hueTmp[3][3], kTmp[3][3], kTmp1[3][3];
	unsigned int satCmps;
	short cmps = 0;
	short contrast, bri, color;
	short deg_tint;
	int aplColorGain;
	unsigned int con_gain_max, color_gain_max;
	/*VPQ_SAT_LUT_T *sat_LUT = NULL;*/ /* DSE function */
	unsigned char Input_mode, YUV2RGB_TBL_Idx;
	int tbl_offset_R, tbl_offset_G, tbl_offset_B;

	unsigned char bt2020_Constant_En;
	short bt2020_k13, bt2020_k32;

	gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (system_info_struct == NULL || gVip_Table == NULL) {
		VIPprintf("~get shareMem | gvipTbl error return~\n");
		return -1;
	}

	memset(satTmp, 0, sizeof(satTmp));
	memset(hueTmp, 0, sizeof(hueTmp));
	memset(kTmp, 0, sizeof(kTmp));
	memset(kTmp1, 0, sizeof(kTmp1));

	coef_level = system_info_struct->using_YUV2RGB_Matrix_Info.CoefByY_CtrlItem_Level;
	RGB_level = system_info_struct->using_YUV2RGB_Matrix_Info.RGB_Offset_Level;
	tbl_select = system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_TBL_Select;

	if (coef_level >=  VIP_YUV2RGB_LEVEL_SELECT_MAX || RGB_level >=  VIP_YUV2RGB_LEVEL_SELECT_MAX || tbl_select >=  YUV2RGB_TBL_Num) {
		VIPprintf("~level or table select Error return, level = %d, table select = %d, %s->%d, %s~\n", coef_level, tbl_select, __FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	Input_mode = system_info_struct->using_YUV2RGB_Matrix_Info.Input_Data_Mode;
	YUV2RGB_TBL_Idx = fwif_color_get_Decide_YUV2RGB_TBL_Index_tv006(0, display, Input_mode);

	if (display == SLR_MAIN_DISPLAY) {
		/* contrast, brightness, tint, color gain get from share memory */
		contrast = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Contrast_Gain, 0);
		bri = bri_10bit;//fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Brightness_Gain, 1/*0*/);//lesley
		color = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Saturation_Gain, 0);
		deg_tint = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Hue_Gain, 0);

		if (OSD_Contrast_Compensation) {
			fwif_color_CalContrast_Compensation_tv006(display, contrast, system_info_struct);
			cmps = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Brightness_Cmps, 0);
			satCmps = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Saturation_Cmps, 0);
		} else {
			cmps = 0;
			satCmps = 128;
		}

		/* get DSE saturation Gain LUT */
		/*fwif_color_get_SaturationLUT_tv006(SLR_MAIN_DISPLAY, &sat_LUT);*/
		fwif_color_access_SaturationLUT_tv006(SLR_MAIN_DISPLAY, &sat_LUT, ACCESS_MODE_GET);

		/* for K13_2, K32_2*/
		if (system_info_struct->BT2020_CTRL.Enable_Flag == 1 && system_info_struct->BT2020_CTRL.Mode == BT2020_MODE_Constant)
			bt2020_Constant_En = 1;
		else
			bt2020_Constant_En = 0;

		VIPprintf("~tv006~,cmps=%d,satCmps=%d\n", cmps, satCmps);
		VIPprintf("~tv006_gain, con = %d, bri = %d, color = %d, tint = %d~\n", contrast, bri, color, deg_tint);
		VIPprintf("~sat_LUT[0]=%d, sat_LUT[5]=%d, sat_LUT[12]=%d, sat_LUT[16]=%d~\n", sat_LUT.uSaturationY[0], sat_LUT.uSaturationY[5], sat_LUT.uSaturationY[12], sat_LUT.uSaturationY[16]);
	} else {
		/* contrast, brightness, tint, color gain get from share memory */
		contrast = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Contrast_Gain_Sub, 0);
		bri = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Brightness_Gain_Sub, 0);
		color = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Saturation_Gain_Sub, 0);
		deg_tint = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Hue_Gain_Sub, 0);

		if (OSD_Contrast_Compensation) {
			fwif_color_CalContrast_Compensation_tv006(display, contrast, system_info_struct);
			cmps = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Brightness_Cmps_Sub, 0);
			satCmps = fwif_color_ChangeOneUINT16Endian(system_info_struct->OSD_Info.Saturation_Cmps_Sub, 0);
		} else {
			cmps = 0;
			satCmps = 128;
		}

		/* get DSE saturation Gain LUT */
		/*fwif_color_get_SaturationLUT_tv006(SLR_MAIN_DISPLAY, &sat_LUT);*/ /* No DSE for sub*/
		for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++) {
			sat_LUT.uSaturationY[i] = 128; /*0x80 = gain 1*/
		}

		/* for K13_2, K32_2*/
		if (system_info_struct->BT2020_CTRL.Enable_Flag_Sub == 1 && system_info_struct->BT2020_CTRL.Mode_Sub == BT2020_MODE_Constant)
			bt2020_Constant_En = 1;
		else
			bt2020_Constant_En = 0;

		VIPprintf("~sub,tv006~,cmps=%d,satCmps=%d\n", cmps, satCmps);
		VIPprintf("~sub,tv006_gain, con = %d, bri = %d, color = %d, tint = %d~\n", contrast, bri, color, deg_tint);
		/*VIPprintf("~sub,sat_LUT[0]=%d, sat_LUT[5]=%d, sat_LUT[12]=%d, sat_LUT[16]=%d~\n", sat_LUT.uSaturationY[0], sat_LUT.uSaturationY[5], sat_LUT.uSaturationY[12], sat_LUT.uSaturationY[16]);*/
	}

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
	for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++) {
		sat_gain.uSaturationY[i] = color;
	}
	fwif_color_access_Sat_Gain_tv006(display, &sat_gain, ACCESS_MODE_WRITE);

	/*color = (color > 128) ?  75 + (color*53 / 128) : color;*/
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

	/*APL color gain*/
	if (0 == fwif_color_access_apl_color_gain_tv006(display, &aplColorGain, ACCESS_MODE_READ))
		color = color * aplColorGain / 100;

	/*matrix computation for contrast, sat and hue*/
	for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++) {
		con_gain_max = 0;
		color_gain_max = 0;

		/*DSE function, gain by saturation LUT*/
		satTmp[1][1] = (color * sat_LUT.uSaturationY[i]) >> 7;
		satTmp[2][2] = satTmp[1][1];
		VIPprintf("~DSE gain,%d,%d\n", satTmp[1][1], satTmp[2][2]);

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

		/* get max coef for max color gain calculation*/
		for (j = 0; j < 3; j++) {
			for (k = 1; k < 3; k++) {
				if (kTmp[j][k] >= 0) {
					if (kTmp[j][k] > color_gain_max)
						color_gain_max = kTmp[j][k];
				} else {
					if ((kTmp[j][k] * (-1)) > color_gain_max)
						color_gain_max = (kTmp[j][k] * (-1));
				}
			}
		}
		con_gain_max = kTmp[0][0];
		VIPprintf("~max coef,%d,%d\n", con_gain_max, color_gain_max);

		/* max contrast gain and color gain calculation*/
		if (con_gain_max == 0)
			con_gain_max = 1;
		con_gain_max = (VIP_YUV2RGB_K_Y_NEG_BND << 7) / con_gain_max;		/*(0x3FF/con_gain_max) << 7;*/	/* Max Gain : Kxx: S(11, 8) */

		if (color_gain_max == 0)
			color_gain_max = 1;
		color_gain_max = (VIP_YUV2RGB_K_C_NEG_BND << 7) / color_gain_max;	/*(0x3FF/color_gain_max) << 7;*/	/* Max Gain : Kxx: S(11, 8) */

		/* check gain by max gain*/
		if (contrast > con_gain_max)
			contrast = con_gain_max;
		if (satTmp[1][1] > color_gain_max) {
			satTmp[1][1] = color_gain_max;
			satTmp[2][2] = color_gain_max;
		}
		/* record final sat gain curve for tool, elieli*/
		total_sat_gain.uSaturationY[i] = satTmp[1][1];
		VIPprintf("~f_G,%d,%d,%d\n", contrast, satTmp[1][1], satTmp[2][2]);

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
	/*drv_vipCSMatrix_t.RGB_Offset.R_offset = (((bri - 128) << 5) + (cmps << 4) + tbl_offset_R) & 0x7fff;
	drv_vipCSMatrix_t.RGB_Offset.G_offset = (((bri - 128) << 5) + (cmps << 4) + tbl_offset_G) & 0x7fff;
	drv_vipCSMatrix_t.RGB_Offset.B_offset = (((bri - 128) << 5) + (cmps << 4) + tbl_offset_B) & 0x7fff;*/
	/* remapping bri effect to more, elieli*/
	#if 1 // lesley, bri 10 bits
	bri = bri - (128<<2);
	//bri = (bri * (150<<2)) >> (7+2);
	/* more bri effect for negative*/
	//if (bri < 0)
	//	bri = (bri * (213<<2)) >> (7+2);

	drv_vipCSMatrix_t.RGB_Offset.R_offset = ((bri << (5-2)) + (cmps << 4) + tbl_offset_R) & 0x7fff;
	drv_vipCSMatrix_t.RGB_Offset.G_offset = ((bri << (5-2)) + (cmps << 4) + tbl_offset_G) & 0x7fff;
	drv_vipCSMatrix_t.RGB_Offset.B_offset = ((bri << (5-2)) + (cmps << 4) + tbl_offset_B) & 0x7fff;
	#else // bri 8 bits
	bri = bri - 128;
	bri = (bri * 150) >> 7;
	/* more bri effect for negative*/
	if (bri < 0)
		bri = (bri * 213) >> 7;

	drv_vipCSMatrix_t.RGB_Offset.R_offset = ((bri << 5) + (cmps << 4) + tbl_offset_R) & 0x7fff;
	drv_vipCSMatrix_t.RGB_Offset.G_offset = ((bri << 5) + (cmps << 4) + tbl_offset_G) & 0x7fff;
	drv_vipCSMatrix_t.RGB_Offset.B_offset = ((bri << 5) + (cmps << 4) + tbl_offset_B) & 0x7fff;
	#endif
	/*Clamp setting*/
	/*drv_vipCSMatrix_t.CTRL_ITEM.Y_Clamp = Y_Clamp;*/
	drv_vipCSMatrix_t.CTRL_ITEM.Y_Clamp = 0;

	/*Y_index Setting*/ /*add from K3L*/
	for (j = 0; j < VIP_YUV2RGB_Y_Seg_Max-1; j++)
		drv_vipCSMatrix_t.COEF_By_Y.Y_index[j] = sat_LUT.uSaturationX[j];

	if (display == SLR_MAIN_DISPLAY) {
		/* record final sat gain curve for tool*/
		fwif_color_access_Total_Sat_Gain_tv006(display, &total_sat_gain, ACCESS_MODE_WRITE);
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
		/* drv setting*/
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Y_Clamp, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_Y_Only, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Offset_Only, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_CoefByY_Index, VIP_YUV2RGB_Y_Seg_Max, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
		/* bt2020_Constant_En,  for bt 2020 k13, k32*/
		if (bt2020_Constant_En == 1) {
			/*drv setting*/
			drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_BT2020_Only, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
			/*memory setting*/
			fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.bt2020_K13), 1, 0);
			fwif_color_ChangeUINT16Endian(&(system_info_struct->using_YUV2RGB_Matrix_Info.YUV2RGB_CSMatrix.COEF_By_Y.bt2020_K32), 1, 0);
		}
	}
	else {
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Y_Clamp, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_Y_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_C_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Offset_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
		if (bt2020_Constant_En == 1)
			drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_Coef_BT2020_Only, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);
	}
	return 0;
}
#endif
char fwif_color_set_DSE_tv006(unsigned char display, VPQ_SAT_LUT_T *satLUT)
{
	int ret;
	_system_setting_info *system_info_struct = NULL;

	system_info_struct = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (system_info_struct == NULL) {
		VIPprintf("~get shareMem | gvipTbl error return~\n");
		return -1;
	}

	if (display != SLR_MAIN_DISPLAY) {
		VIPprintf("~only main channel for DSE return~\n");
		return -1;
	}
	/* set sat gain into share mem*/
	/*ret = fwif_color_set_SaturationLUT_tv006(display, satLUT);*/
	ret = fwif_color_access_SaturationLUT_tv006(display, satLUT, ACCESS_MODE_SET);

	if (ret < 0)
		return ret;

	ret = fwif_color_setCon_Bri_Color_Tint_tv006(display, system_info_struct); /* only main channel*/

	return ret;
}

void fwif_color_set_YUV2RGB_tv006(unsigned char src_idx, unsigned char display, unsigned char which_table, unsigned char drvSkip_Flag_coefByY, unsigned char drvSkip_Flag_uvOffset)
{
	/*base default setting*/
	DRV_VIP_YUV2RGB_CSMatrix drv_vipCSMatrix_t;
	/*unsigned char i;*/
	SLR_VIP_TABLE *gVip_Table = NULL;
	//UINT8 i;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	//unsigned char YUV2RGB_TBL_Idx;
	//unsigned char Input_mode;

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL)
		return;

	//Input_mode = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.Input_Data_Mode;
	gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	/*Coef and offset is set by OSD con, bri, hue,sat*/
	VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.RGB_Offset_Level = VIP_YUV2RGB_LEVEL_OFF;
	VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.UV_Offset_Level = VIP_YUV2RGB_LEVEL_OFF;
	VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.CoefByY_CtrlItem_Level = VIP_YUV2RGB_LEVEL_OFF;
	VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_TBL_Select = which_table;

	memcpy(&drv_vipCSMatrix_t, &(gVip_Table->YUV2RGB_CSMatrix_Table[which_table].YUV2RGB_CSMatrix[VIP_YUV2RGB_LEVEL_OFF]), sizeof(DRV_VIP_YUV2RGB_CSMatrix));

	//YUV2RGB_TBL_Idx = fwif_color_get_Decide_YUV2RGB_TBL_Index_tv006(src_idx, display, Input_mode);

	drv_vipCSMatrix_t.CTRL_ITEM.CoefByY_En = 2; /*force Coef by Sat*/
	/* force main use tbl 1, sub use tbl 2 for TV006*/
	drv_vipCSMatrix_t.CTRL_ITEM.Table_Select_Main = 0;
	drv_vipCSMatrix_t.CTRL_ITEM.Table_Select_Sub = 1;

	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_YUV2RGB_Base_Ctrl, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_CoefByY_ctrl, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);

	/*TBL2 (Sub channel)*/
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_YUV2RGB_Base_Ctrl, VIP_YUV2RGB_Y_Seg_0, SLR_SUB_DISPLAY, VIP_Table_Select_2);

}

int fwif_color_update_YUV2RGB_CSC_tv006(unsigned char display)
{
	_system_setting_info *system_info_struct = NULL;
	system_info_struct = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if (NULL == system_info_struct)
		return -1;

	return fwif_color_setCon_Bri_Color_Tint_tv006(display, system_info_struct);
}

#if 0 /* no more use because may confuse when code sync YUV2RGB matrix*/
char fwif_color_set_SaturationLUT_tv006(unsigned char display, VPQ_SAT_LUT_T *satLUT)
{
	unsigned char coef_level, tbl_select, i;
	DRV_VIP_YUV2RGB_COEF_By_Y *coef_TBL;
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	SLR_VIP_TABLE *gVip_Table_shareMem = NULL;
	/*gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();*/
	gVip_Table_shareMem = fwif_color_GetShare_Memory_VIP_TABLE_Struct();

	if (display != SLR_MAIN_DISPLAY) {
		VIPprintf("~DSE No Sub~ return\n");
		return -1;
	}

	if (VIP_system_info_structure_table == NULL || gVip_Table_shareMem == NULL || satLUT == NULL) {
		VIPprintf("~get TBL NULL return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return -2;
	}

	coef_level = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.CoefByY_CtrlItem_Level;
	tbl_select = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_TBL_Select;

	coef_TBL = &(gVip_Table_shareMem->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[coef_level].COEF_By_Y);

	for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++) {
		coef_TBL->K12[i] = fwif_color_ChangeOneUINT16Endian(satLUT->uSaturationY[i], 0);
		coef_TBL->K13[i] = fwif_color_ChangeOneUINT16Endian(satLUT->uSaturationY[i], 0);
		coef_TBL->K22[i] = fwif_color_ChangeOneUINT16Endian(satLUT->uSaturationY[i], 0);
		coef_TBL->K23[i] = fwif_color_ChangeOneUINT16Endian(satLUT->uSaturationY[i], 0);
		coef_TBL->K32[i] = fwif_color_ChangeOneUINT16Endian(satLUT->uSaturationY[i], 0);
		coef_TBL->K33[i] = fwif_color_ChangeOneUINT16Endian(satLUT->uSaturationY[i], 0);
	}
	return 0;
}

char fwif_color_get_SaturationLUT_tv006(unsigned char display, VPQ_SAT_LUT_T *satLUT)
{
	unsigned char coef_level, tbl_select, i;
	DRV_VIP_YUV2RGB_COEF_By_Y *coef_TBL;
	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	SLR_VIP_TABLE *gVip_Table_shareMem = NULL;
	/*gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();*/
	gVip_Table_shareMem = fwif_color_GetShare_Memory_VIP_TABLE_Struct();

	if (display != SLR_MAIN_DISPLAY) {
		VIPprintf("~DSE No Sub~ return\n");
		return -1;
	}

	if (VIP_system_info_structure_table == NULL || gVip_Table_shareMem == NULL || satLUT == NULL) {
		VIPprintf("~get TBL NULL return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return -2;
	}
	coef_level = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.CoefByY_CtrlItem_Level;
	tbl_select = VIP_system_info_structure_table->using_YUV2RGB_Matrix_Info.YUV2RGB_TBL_Select;

	coef_TBL = &(gVip_Table_shareMem->YUV2RGB_CSMatrix_Table[tbl_select].YUV2RGB_CSMatrix[coef_level].COEF_By_Y);

	for (i = 0; i < VIP_YUV2RGB_Y_Seg_Max; i++) {
		satLUT->uSaturationY[i] = fwif_color_ChangeOneUINT16Endian(coef_TBL->K12[i], 0); /* all chroma coef(kxx) should be same for DSE, using k12*/
	}

	VIPprintf("~DSE_Get,%d, =%d~\n", coef_level, tbl_select);

	return 0;

}
#endif /* no more use because may confuse when code sync YUV2RGB matrix*/
char fwif_color_access_SaturationLUT_tv006(unsigned char display, VPQ_SAT_LUT_T *satLUT, ACCESS_MODE mode)
{
	static VPQ_SAT_LUT_T cur_sat_lut = {{0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},
										{0xf, 0x1f, 0x2f, 0x3f, 0x4f, 0x5f, 0x6f, 0x7f, 0x8f, 0x9f, 0xaf, 0xbf, 0xcf, 0xdf, 0xef, 0xff}};

	if (display != SLR_MAIN_DISPLAY) {
		VIPprintf("~DSE No Sub~ return\n");
		return -1;
	}

	if (mode == ACCESS_MODE_SET) {
		memcpy(&cur_sat_lut, satLUT, sizeof(VPQ_SAT_LUT_T));
	} else if (mode == ACCESS_MODE_GET) {
		memcpy(satLUT, &cur_sat_lut, sizeof(VPQ_SAT_LUT_T));
	}

	return 0;
}


char fwif_color_access_Sat_Gain_tv006(unsigned char display, VPQ_SAT_LUT_T *satLUT, ACCESS_MODE mode)
{
	static VPQ_SAT_LUT_T sat_gain = {{0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},
										{0xf, 0x1f, 0x2f, 0x3f, 0x4f, 0x5f, 0x6f, 0x7f, 0x8f, 0x9f, 0xaf, 0xbf, 0xcf, 0xdf, 0xef, 0xff}};

	if (display != SLR_MAIN_DISPLAY) {
		VIPprintf("~No Sub~ return\n");
		return -1;
	}

	if (mode == ACCESS_MODE_SET) {
		memcpy(&sat_gain, satLUT, sizeof(VPQ_SAT_LUT_T));
	} else if (mode == ACCESS_MODE_GET) {
		memcpy(satLUT, &sat_gain, sizeof(VPQ_SAT_LUT_T));
	}

	return 0;
}


char fwif_color_access_Total_Sat_Gain_tv006(unsigned char display, VPQ_SAT_LUT_T *satLUT, ACCESS_MODE mode)
{
	static VPQ_SAT_LUT_T total_sat_gain = {{0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},
											{0xf, 0x1f, 0x2f, 0x3f, 0x4f, 0x5f, 0x6f, 0x7f, 0x8f, 0x9f, 0xaf, 0xbf, 0xcf, 0xdf, 0xef, 0xff}};

	if (display != SLR_MAIN_DISPLAY) {
		VIPprintf("~No Sub~ return\n");
		return -1;
	}

	if (mode == ACCESS_MODE_SET) {
		memcpy(&total_sat_gain, satLUT, sizeof(VPQ_SAT_LUT_T));
	} else if (mode == ACCESS_MODE_GET) {
		memcpy(satLUT, &total_sat_gain, sizeof(VPQ_SAT_LUT_T));
	}

	return 0;
}

char fwif_color_access_apl_color_gain_tv006(unsigned char display, int* pAplColorGain, ACCESS_MODE mode)
{
	static int stAPLColorGain = 100;

	if (display != SLR_MAIN_DISPLAY || pAplColorGain == NULL) {
		return -1;
	}

	if (mode == ACCESS_MODE_SET) {
		stAPLColorGain = *pAplColorGain;
	} else if (mode == ACCESS_MODE_GET) {
		*pAplColorGain = stAPLColorGain;
	}

	return 0;
}

void fwif_color_set_sharpness_table_TV006(CHIP_SHARPNESS_UI_T *ptCHIP_SHARPNESS_UI_T, CHIP_SR_UI_T *ptCHIP_SR_UI_T)
{

	unsigned short value = Scaler_GetSharpnessTable(); /* Get from VIP Table */
	unsigned short vip_src = 0;
	//DRV_Sharpness_LV2 sharpLV2;
        static DRV_Sharpness_Table sharp;
        static DRV_Sharpness_Level sharp_gain;
        static DRV_Sharpness_Other_Ctrl sharp_other_ctrl;
        static DRV_VipNewIDcti_Table i_dcti;
        static DRV_VipNewDDcti_Table d_dcti;
        static VIP_D_vcti_t v_cti;
        static VIP_D_DLTI_Table drv_shp_Dlti;
        static VIP_MBPK_Table MBPK;

	_RPC_system_setting_info *VIP_RPC_system_info_structure_table=(_RPC_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	_system_setting_info *VIP_system_info_structure_table=(_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

        drvif_color_Get_Sharpness_Table(&sharp);
        drvif_color_get_Sharpness_level(&sharp_gain);
        drvif_color_Get_Sharpness_Other_Table(&sharp_other_ctrl);
        drvif_color_get_inewdcti_table(&i_dcti);
        drvif_color_get_dnewdcti_table(&d_dcti);
        drvif_color_scaler_getds_vcti(0, &v_cti);
        drvif_color_Get_D_DLTI_Table(&drv_shp_Dlti);
        drvif_color_Get_MBPK_Table(&MBPK);


	if (NULL == ptCHIP_SHARPNESS_UI_T || NULL == ptCHIP_SR_UI_T)
		return;

	if (VIP_RPC_system_info_structure_table != NULL)
		vip_src = VIP_RPC_system_info_structure_table->VIP_source;


	if (value >= Sharp_table_num)
		value = 0;
	//if((Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP) == FALSE) && (get_scaler_connect_source(SLR_MAIN_DISPLAY) == _SRC_HDMI))//fixed me	RGB444
	//	sharp.TwoD_Peak_AdaptCtrl.Peaking_Enable = 0;	// fixed me
	//else

        sharp.TwoD_Peak_AdaptCtrl.Peaking_Enable = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.Peaking_Enable;
	sharp.TwoD_Peak_AdaptCtrl.Actsftbit_Reg = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.Actsftbit_Reg;

	sharp.TwoD_Peak_AdaptCtrl.Hpf_Vpf_Maxmin_En = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.Hpf_Vpf_Maxmin_En;/*magellan2*/
	sharp.TwoD_Peak_AdaptCtrl.Src_Sel = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.Src_Sel;/*magellan2*/

	sharp.TwoD_Peak_AdaptCtrl.Sobel_h= gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.sobelrange_h;//merlin
	sharp.TwoD_Peak_AdaptCtrl.Sobel_v= gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.sobelrange_v;//merlin
	sharp.TwoD_Peak_AdaptCtrl.Acts_offset= gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.acts_offset;//merlin
	sharp.TwoD_Peak_AdaptCtrl.Small_thl= gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.small_th1;//merlin
	sharp.TwoD_Peak_AdaptCtrl.Edgextrange_v= gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.edgextrange_v;//merlin

	sharp.TwoD_Peak_AdaptCtrl._5x5_h= gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl._5x5_h;//merlin3
	sharp.TwoD_Peak_AdaptCtrl._5x5_v= gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl._5x5_v;//merlin3
	sharp.TwoD_Peak_AdaptCtrl.Tex_Exclude_Smooth= gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.Tex_Exclude_Smooth;//merlin3
	sharp.TwoD_Peak_AdaptCtrl.Sobel_hspace= gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.Sobel_hspace;//merlin3
	sharp.TwoD_Peak_AdaptCtrl.Sobel_vspace= gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.Sobel_vspace;//merlin3


	sharp.TwoD_edg.en_edg = gVip_Table->Ddomain_SHPTable[value].Edge_Shp_coef1.en_edg;
	sharp.TwoD_tex.en_tex = gVip_Table->Ddomain_SHPTable[value].Texture_Shp_coef1.en_tex;

	sharp.Edge_Shp.Edg_D2_Shift_Bit = gVip_Table->Ddomain_SHPTable[value].Edge_Shp_set.Edg_D2_Shift_Bit;
	sharp.Edge_Shp.LV2 = gVip_Table->Ddomain_SHPTable[value].Edge_Shp_set.LV2;
 	sharp.Vertical.en_tex = gVip_Table->Ddomain_SHPTable[value].Vertical.en_tex;
	sharp.Vertical.en_edg = gVip_Table->Ddomain_SHPTable[value].Vertical.en_edg;
 	sharp.Vertical.G_Pos2 = gVip_Table->Ddomain_SHPTable[value].Vertical.G_Pos2;
	sharp.Vertical.LV2 = gVip_Table->Ddomain_SHPTable[value].Vertical.LV2;

	sharp.Vertical.enable_edge= gVip_Table->Ddomain_SHPTable[value].Vertical_edg.enable;//merlin
	sharp.Vertical.LV2_edge= gVip_Table->Ddomain_SHPTable[value].Vertical_edg.LV2;//merlin
 	sharp.Edge_EMF_Table.Seg_1_Offset = gVip_Table->Ddomain_SHPTable[value].Edge_EMF_Table.Seg_1_Offset;
	sharp.Edge_EMF_Table.Seg_2_Offset = gVip_Table->Ddomain_SHPTable[value].Edge_EMF_Table.Seg_2_Offset;
 	sharp.Text_EMF_Table.Seg_1_Offset = gVip_Table->Ddomain_SHPTable[value].Texture_EMF_Table.Seg_1_Offset;
	sharp.Text_EMF_Table.Seg_2_Offset = gVip_Table->Ddomain_SHPTable[value].Texture_EMF_Table.Seg_2_Offset;

 	sharp.MK3_Table.Edge_Filter_Sel = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.Edge_Filter_Sel;
	sharp.TwoD_Peak_AdaptCtrl.PkDlti_comsign = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.PkDlti_comsign;
	sharp.TwoD_Peak_AdaptCtrl.edg_dlti = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.edg_dlti;
	sharp.TwoD_Peak_AdaptCtrl.edg_dlti_tex = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.edg_dlti_tex;
	sharp.TwoD_Peak_AdaptCtrl.zdiffl_en = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.zdiffl_en;
	sharp.TwoD_Peak_AdaptCtrl.zdiffl_lowbnd = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.zdiffl_lowbnd;
	sharp.TwoD_Peak_AdaptCtrl.zdiffl_stepbit = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.zdiffl_stepbit;
	sharp.TwoD_Peak_AdaptCtrl.edge_mono_en = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.edge_mono_en;
	sharp.TwoD_Peak_AdaptCtrl.edge_mono_stepbit = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.edge_mono_stepbit;
	sharp.TwoD_Peak_AdaptCtrl.edge_mono_stepbiu = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.edge_mono_stepbiu;
	sharp.TwoD_Peak_AdaptCtrl.edge_mono_upbnd = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.edge_mono_upbnd;
	sharp.TwoD_Peak_AdaptCtrl.edge_mono_lowbnd = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.edge_mono_lowbnd;

 	sharp.MK3_Table.lowbound= gVip_Table->Ddomain_SHPTable[value].Slope_gain_mask.lowbound;//merlin3
	sharp.MK3_Table.lowbound_v= gVip_Table->Ddomain_SHPTable[value].Slope_gain_mask.lowbound_v;//merlin3
	sharp.MK3_Table.use_ramp= gVip_Table->Ddomain_SHPTable[value].Slope_gain_mask.use_ramp;//merlin3
	sharp.MK3_Table.extend2= gVip_Table->Ddomain_SHPTable[value].Slope_gain_mask.extend2;//merlin3//juwen, delete

 	sharp.After_Filter.C0 = gVip_Table->Ddomain_SHPTable[value].After_Filter.C0;
	sharp.After_Filter.C1 = gVip_Table->Ddomain_SHPTable[value].After_Filter.C1;
	sharp.After_Filter.C2 = gVip_Table->Ddomain_SHPTable[value].After_Filter.C2;
	sharp.After_Filter.C3 = gVip_Table->Ddomain_SHPTable[value].After_Filter.C3;
	sharp.After_Filter.C4 = gVip_Table->Ddomain_SHPTable[value].After_Filter.C4;
	sharp.After_Filter.HPF_Gain = gVip_Table->Ddomain_SHPTable[value].After_Filter.HPF_Gain;
	sharp.After_Filter.Mode = gVip_Table->Ddomain_SHPTable[value].After_Filter.Mode;
	sharp.After_Filter.Enable = gVip_Table->Ddomain_SHPTable[value].After_Filter.Enable;
	sharp.After_Filter.Rgblimit_pos= gVip_Table->Ddomain_SHPTable[value].After_Filter.Rgblimit_pos;
	sharp.After_Filter.Rgblimit_neg= gVip_Table->Ddomain_SHPTable[value].After_Filter.Rgblimit_neg;



 	sharp.SR_Control.Ring_Gen = gVip_Table->Ddomain_SHPTable[value].SR_Control.Ring_Gen;
 	sharp.SR_Control.sr_detail_tex_en = gVip_Table->Ddomain_SHPTable[value].SR_Control.sr_detail_tex_en;
	sharp.SR_Control.sr_detail_edg_en = gVip_Table->Ddomain_SHPTable[value].SR_Control.sr_detail_edg_en;
	sharp.SR_Control.sr_detail_v_tex_en = gVip_Table->Ddomain_SHPTable[value].SR_Control.sr_detail_v_tex_en;
	sharp.SR_Control.sr_detail_v_edg_en = gVip_Table->Ddomain_SHPTable[value].SR_Control.sr_detail_v_edg_en;

	sharp.SR_H9.en_tex = gVip_Table->Ddomain_SHPTable[value].SR_H9.en_tex;
	sharp.SR_H9.en_edg = gVip_Table->Ddomain_SHPTable[value].SR_H9.en_edg;

	sharp.SR_H13.en_tex = gVip_Table->Ddomain_SHPTable[value].SR_H13.en_tex;
	sharp.SR_H13.en_edg = gVip_Table->Ddomain_SHPTable[value].SR_H13.en_edg;

	sharp.SR_H21.en_tex = gVip_Table->Ddomain_SHPTable[value].SR_H21.en_tex;
	sharp.SR_H21.en_edg = gVip_Table->Ddomain_SHPTable[value].SR_H21.en_edg;

	sharp.SR_V9_1.en_tex = gVip_Table->Ddomain_SHPTable[value].SR_V9_1.en_tex;
	sharp.SR_V9_1.en_edg = gVip_Table->Ddomain_SHPTable[value].SR_V9_1.en_edg;

	sharp.SR_V9_2.en_tex = gVip_Table->Ddomain_SHPTable[value].SR_V9_2.en_tex;
	sharp.SR_V9_2.en_edg = gVip_Table->Ddomain_SHPTable[value].SR_V9_2.en_edg;

	sharp.SR_V9_3.en_tex = gVip_Table->Ddomain_SHPTable[value].SR_V9_3.en_tex;
	sharp.SR_V9_3.en_edg = gVip_Table->Ddomain_SHPTable[value].SR_V9_3.en_edg;

	sharp.SR_Ring_H11.en_tex = gVip_Table->Ddomain_SHPTable[value].SR_Ring_H11.en_tex;
	sharp.SR_Ring_H11.en_edg = gVip_Table->Ddomain_SHPTable[value].SR_Ring_H11.en_edg;

	sharp.SR_Ring_V9.en_tex = gVip_Table->Ddomain_SHPTable[value].SR_Ring_V9.en_tex;
	sharp.SR_Ring_V9.en_edg = gVip_Table->Ddomain_SHPTable[value].SR_Ring_V9.en_edg;


	sharp.Sub_Gain_Modify.sub_index_en = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.sub_index_en;
	sharp.Sub_Gain_Modify.EDG.Gain_Pos = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.EDG_Gain_Pos;
	sharp.Sub_Gain_Modify.EDG.Gain_Neg = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.EDG_Gain_Neg;
	sharp.Sub_Gain_Modify.EDG.HV_Pos = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.EDG_HV_Pos;
	sharp.Sub_Gain_Modify.EDG.HV_Neg = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.EDG_HV_Neg;
	sharp.Sub_Gain_Modify.EDG.LV = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.EDG_LV;
	sharp.Sub_Gain_Modify.TEX.Gain_Pos = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.TEX_Gain_Pos;
	sharp.Sub_Gain_Modify.TEX.Gain_Neg = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.TEX_Gain_Neg;
	sharp.Sub_Gain_Modify.TEX.HV_Pos = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.TEX_HV_Pos;
	sharp.Sub_Gain_Modify.TEX.HV_Neg = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.TEX_HV_Neg;
	sharp.Sub_Gain_Modify.TEX.LV = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.TEX_LV;
	sharp.Sub_Gain_Modify.V.Gain_Pos = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.V_Gain_Pos;
	sharp.Sub_Gain_Modify.V.Gain_Neg = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.V_Gain_Neg;
	sharp.Sub_Gain_Modify.V.HV_Pos = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.V_HV_Pos;
	sharp.Sub_Gain_Modify.V.HV_Neg = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.V_HV_Neg;
	sharp.Sub_Gain_Modify.V.LV = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.V_LV;
	sharp.Sub_Gain_Modify.V_EDG.Gain_Pos = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.V_EDG_Gain_Pos;
	sharp.Sub_Gain_Modify.V_EDG.Gain_Neg = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.V_EDG_Gain_Neg;
	sharp.Sub_Gain_Modify.V_EDG.HV_Pos = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.V_EDG_HV_Pos;
	sharp.Sub_Gain_Modify.V_EDG.HV_Neg = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.V_EDG_HV_Neg;
	sharp.Sub_Gain_Modify.V_EDG.LV = gVip_Table->Ddomain_SHPTable[value].Sub_Gain_Modify.V_EDG_LV;

	sharp.Edge_SM.dirsm_rangh = gVip_Table->Ddomain_SHPTable[value].Edge_SM.dirsm_rangh;//merlin
	sharp.Edge_SM.dirsm_rangv= gVip_Table->Ddomain_SHPTable[value].Edge_SM.dirsm_rangv;//merlin
	sharp.Edge_SM.meandiff_rangeh= gVip_Table->Ddomain_SHPTable[value].Edge_SM.meandiff_rangeh;//merlin
	sharp.Edge_SM.meandiff_rangev= gVip_Table->Ddomain_SHPTable[value].Edge_SM.meandiff_rangev;//merlin
	sharp.Edge_SM.meandiff_shiftbit= gVip_Table->Ddomain_SHPTable[value].Edge_SM.meandiff_shiftbit;//merlin
	sharp.Edge_SM.meandiff_step= gVip_Table->Ddomain_SHPTable[value].Edge_SM.meandiff_step;//merlin
	sharp.Edge_SM.meandiff_lowbd= gVip_Table->Ddomain_SHPTable[value].Edge_SM.meandiff_lowbd;//merlin
	sharp.Edge_SM.edge_method_sel= gVip_Table->Ddomain_SHPTable[value].Edge_SM.edge_method_sel;//merlin
	sharp.Edge_SM.edgeindex_step= gVip_Table->Ddomain_SHPTable[value].Edge_SM.edgeindex_step;//merlin
	sharp.Edge_SM.edgeindex_lowbd= gVip_Table->Ddomain_SHPTable[value].Edge_SM.edgeindex_lowbd;//merlin


 	/* Get from LG DB Part */
	/* #1 CHIP_SEGMENTATION_T 13*/
	sharp.TwoD_Peak_AdaptCtrl.Edge_Mode     =       ptCHIP_SHARPNESS_UI_T->stSharpness.stSegmentation.edge_mode;
	sharp.TwoD_Peak_AdaptCtrl.Sobel_Mode    =       ptCHIP_SHARPNESS_UI_T->stSharpness.stSegmentation.sobel_mode;
	sharp.TwoD_Peak_AdaptCtrl.Edgextrange   =       ptCHIP_SHARPNESS_UI_T->stSharpness.stSegmentation.edgextrange;
	sharp.TwoD_Peak_AdaptCtrl.Sobel_Upbnd   =       ptCHIP_SHARPNESS_UI_T->stSharpness.stSegmentation.sobel_upbnd;
	sharp.TwoD_Peak_AdaptCtrl.Sobel_Stepbit =       ptCHIP_SHARPNESS_UI_T->stSharpness.stSegmentation.sobel_stepbit;
	sharp.TwoD_Peak_AdaptCtrl.Zdiff_Upbnd   =       ptCHIP_SHARPNESS_UI_T->stSharpness.stSegmentation.zdiff_upbnd;
	sharp.TwoD_Peak_AdaptCtrl.Zdiff_Stepbit =       ptCHIP_SHARPNESS_UI_T->stSharpness.stSegmentation.zdiff_stepbit;
	sharp.TwoD_Peak_AdaptCtrl.Zigsftbit_Reg =       ptCHIP_SHARPNESS_UI_T->stSharpness.stSegmentation.zigsftbit_reg;
	sharp.TwoD_Peak_AdaptCtrl.Tex_En        =       ptCHIP_SHARPNESS_UI_T->stSharpness.stSegmentation.texture_enable;
	sharp.TwoD_Peak_AdaptCtrl.Tex_Upbnd=	        ptCHIP_SHARPNESS_UI_T->stSharpness.stSegmentation.tex_upbnd;
	sharp.TwoD_Peak_AdaptCtrl.Tex_Stepbit=	        ptCHIP_SHARPNESS_UI_T->stSharpness.stSegmentation.tex_stepbit;
	sharp.TwoD_Peak_AdaptCtrl.Lpf_Mode=	        ptCHIP_SHARPNESS_UI_T->stSharpness.stSegmentation.lpf_mode;
	sharp.TwoD_Peak_AdaptCtrl.Lpf_Filter=	        ptCHIP_SHARPNESS_UI_T->stSharpness.stSegmentation.lpf_filter;

        /* #2 CHIP_EDGE_CORING_MODE_T 11*/
        sharp.MK3_Table.EdgeCoring_En =                 ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCoringMode.edgeCoringEn;
        sharp.MK3_Table.EdgeCoring_Gain_En =            ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCoringMode.edgeGainEn;
        sharp.MK3_Table.EdgeCoring_Bigedge_Ratio =      ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCoringMode.bigEdgeRatio;
        sharp.MK3_Table.EdgeCoring_Smalledge_Ratio =    ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCoringMode.smallEdgeRatio;
        sharp.MK3_Table.EdgeCoring_Deltaedge_Ratio =    ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCoringMode.deltaEdgeRatio;
        sharp.MK3_Table.EdgeCoring_Smalledge_Ulevel =   ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCoringMode.smallEdgeUlevel;
        sharp.MK3_Table.EdgeCoring_DeltaEdge_Ulevel =   ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCoringMode.deltaEdgeUlevel;
        sharp.MK3_Table.EdgeCoring_Clip =               ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCoringMode.edgeCoringClip;
        sharp.MK3_Table.EdgeCoring_Gain_Bound =         ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCoringMode.edgeGainBound;
        sharp.MK3_Table.EdgeCoring_Ser_Range =          ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCoringMode.edgeSerRange;
        sharp.MK3_Table.Edge_Delta_Ext_Range =          ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCoringMode.Edge_delta_ext_R;


        /* #3 CHIP_Y_REMAPPING_T 4*/
        sharp.MK3_Table.LinearY_UM = ptCHIP_SHARPNESS_UI_T->stSharpness.stYRemapping.linearY_Umode;
        sharp.MK3_Table.LinearY_LM = ptCHIP_SHARPNESS_UI_T->stSharpness.stYRemapping.linearY_Lmode;
        sharp.MK3_Table.LinearY_UB = ptCHIP_SHARPNESS_UI_T->stSharpness.stYRemapping.linearY_UB;
        sharp.MK3_Table.LinearY_LB = ptCHIP_SHARPNESS_UI_T->stSharpness.stYRemapping.linearY_LB;


	/* #4 CHIP_EDGE_HPF_T 18 */
	//sharp.TwoD_edg.en_edg                   = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeHPF.edgpk_en;

	sharp_other_ctrl.edgpk_en               = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeHPF.edgpk_en;
        sharp.MK3_Table.Edge_Filter_Sel         = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeHPF.edge_select_filter;//default value is 0
        sharp_other_ctrl.edg_lpf_shift          = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeHPF.edge_lpf_shift;//default value is 8


        sharp.TwoD_edg.C0	= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeHPF.coef_c0_0;
	sharp.TwoD_edg.C1	= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeHPF.coef_c1_0;
	sharp.TwoD_edg.C2	= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeHPF.coef_c2_0;
	sharp.TwoD_edg.C3	= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeHPF.coef_c3_0;
	sharp.TwoD_edg.C4	= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeHPF.coef_c4_0;
	sharp.TwoD_edg.C5	= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeHPF.coef_c5_0;
	sharp.TwoD_edg.C6	= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeHPF.coef_c6_0;
	sharp.TwoD_edg.C7	= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeHPF.coef_c7_0;
	sharp.TwoD_edg.C8	= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeHPF.coef_c8_0;
	sharp.TwoD_edg.C9	= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeHPF.coef_c9_0;
	sharp.TwoD_edg.C10	= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeHPF.coef_c10_0;
	sharp.TwoD_edg.C0H1	= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeHPF.coef_c0_1;
	sharp.TwoD_edg.C0H2	= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeHPF.coef_c0_2;
	sharp.TwoD_edg.C0H3	= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeHPF.coef_c0_3;
	sharp.TwoD_edg.C0H4	= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeHPF.coef_c0_4;


        /* #5 CHIP_EDGE_CURVEMAPPING_STEP_T 28*/
        sharp.Edge_Pos_Curve.S1  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.pos_s1;
        sharp.Edge_Pos_Curve.S2  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.pos_s2;
        sharp.Edge_Pos_Curve.S3  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.pos_s3;
        sharp.Edge_Pos_Curve.S4  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.pos_s4;
        sharp.Edge_Pos_Curve.S5  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.pos_s5;
        sharp.Edge_Pos_Curve.S6  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.pos_s6;
        sharp.Edge_Pos_Curve.S7  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.pos_s7;
        sharp.Edge_Pos_Curve.S8  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.pos_s8;
        sharp.Edge_Pos_Curve.S9  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.pos_s9;
        sharp.Edge_Pos_Curve.S10 = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.pos_s10;
        sharp.Edge_Pos_Curve.S11 = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.pos_s11;
        sharp.Edge_Pos_Curve.S12 = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.pos_s12;
        sharp.Edge_Pos_Curve.S13 = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.pos_s13;
        sharp.Edge_Pos_Curve.S14 = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.pos_s14;
        sharp.Edge_Neg_Curve.S1  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.neg_s1;
        sharp.Edge_Neg_Curve.S2  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.neg_s2;
        sharp.Edge_Neg_Curve.S3  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.neg_s3;
        sharp.Edge_Neg_Curve.S4  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.neg_s4;
        sharp.Edge_Neg_Curve.S5  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.neg_s5;
        sharp.Edge_Neg_Curve.S6  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.neg_s6;
        sharp.Edge_Neg_Curve.S7  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.neg_s7;
        sharp.Edge_Neg_Curve.S8  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.neg_s8;
        sharp.Edge_Neg_Curve.S9  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.neg_s9;
        sharp.Edge_Neg_Curve.S10 = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.neg_s10;
        sharp.Edge_Neg_Curve.S11 = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.neg_s11;
        sharp.Edge_Neg_Curve.S12 = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.neg_s12;
        sharp.Edge_Neg_Curve.S13 = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.neg_s13;
        sharp.Edge_Neg_Curve.S14 = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingStep.neg_s14;

        /* #6 CHIP_EDGE_CURVEMAPPING_UI_T 39 */
	sharp_gain.Edge_Level.G_Pos_Gain        = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.Gain_Pos;
	sharp_gain.Edge_Level.G_Neg_Gain	= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.Gain_Neg;
	sharp.Edge_Shp.G_Pos2                   = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.Gain_Pos2;
	sharp.Edge_Shp.G_Neg2	                = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.Gain_Neg2;
	sharp_gain.Edge_Level.HV_POS_Gain       = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.Range_Pos;
	sharp_gain.Edge_Level.HV_NEG_Gain       = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.Range_Neg;
	sharp_gain.Edge_Level.LV_Gain	        = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.Coring;
	sharp.Edge_Shp.Edg_D2_Shift_Bit         = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.Edge_Shift;
	sharp.Edge_Neg_Curve.enable             = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.Edge_curve_en;
	sharp.Edge_Pos_Curve.G0 		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.pos_g0;
	sharp.Edge_Pos_Curve.G1 		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.pos_g1;
	sharp.Edge_Pos_Curve.G2 		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.pos_g2;
	sharp.Edge_Pos_Curve.G3 		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.pos_g3;
	sharp.Edge_Pos_Curve.G4 		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.pos_g4;
	sharp.Edge_Pos_Curve.G5 		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.pos_g5;
	sharp.Edge_Pos_Curve.G6 		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.pos_g6;
	sharp.Edge_Pos_Curve.G7 		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.pos_g7;
	sharp.Edge_Pos_Curve.G8 		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.pos_g8;
	sharp.Edge_Pos_Curve.G9 		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.pos_g9;
	sharp.Edge_Pos_Curve.G10		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.pos_g10;
	sharp.Edge_Pos_Curve.G11		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.pos_g11;
	sharp.Edge_Pos_Curve.G12		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.pos_g12;
	sharp.Edge_Pos_Curve.G13		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.pos_g13;
	sharp.Edge_Pos_Curve.G14		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.pos_g14;
	sharp.Edge_Neg_Curve.G0		        = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.neg_g0;
	sharp.Edge_Neg_Curve.G1		        = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.neg_g1;
	sharp.Edge_Neg_Curve.G2		        = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.neg_g2;
	sharp.Edge_Neg_Curve.G3		        = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.neg_g3;
	sharp.Edge_Neg_Curve.G4		        = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.neg_g4;
	sharp.Edge_Neg_Curve.G5		        = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.neg_g5;
	sharp.Edge_Neg_Curve.G6		        = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.neg_g6;
	sharp.Edge_Neg_Curve.G7		        = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.neg_g7;
	sharp.Edge_Neg_Curve.G8		        = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.neg_g8;
	sharp.Edge_Neg_Curve.G9		        = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.neg_g9;
	sharp.Edge_Neg_Curve.G10		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.neg_g10;
	sharp.Edge_Neg_Curve.G11		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.neg_g11;
	sharp.Edge_Neg_Curve.G12		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.neg_g12;
	sharp.Edge_Neg_Curve.G13		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.neg_g13;
	sharp.Edge_Neg_Curve.G14		= ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeCurveMappingUI.neg_g14;

        /* #7 CHIP_TEXTURE_HPF_T 18 */
      //  sharp.TwoD_tex.en_tex            = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureHPF.texpk_en;

        sharp_other_ctrl.texpk_en            = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureHPF.texpk_en;

        sharp.MK3_Table.Tex_Filter_Sel   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureHPF.text_select_filter;
        sharp_other_ctrl.tex_lpf_shift   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureHPF.tex_lpf_shift;
        sharp.TwoD_tex.C0                = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureHPF.coef_c0_0;
        sharp.TwoD_tex.C1                = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureHPF.coef_c1_0;
        sharp.TwoD_tex.C2                = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureHPF.coef_c2_0;
        sharp.TwoD_tex.C3                = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureHPF.coef_c3_0;
        sharp.TwoD_tex.C4                = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureHPF.coef_c4_0;
        sharp.TwoD_tex.C5                = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureHPF.coef_c5_0;
        sharp.TwoD_tex.C6                = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureHPF.coef_c6_0;
        sharp.TwoD_tex.C7                = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureHPF.coef_c7_0;
        sharp.TwoD_tex.C8                = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureHPF.coef_c8_0;
        sharp.TwoD_tex.C9                = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureHPF.coef_c9_0;
        sharp.TwoD_tex.C10               = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureHPF.coef_c10_0;
        sharp.TwoD_tex.C0H1              = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureHPF.coef_c0_1;
        sharp.TwoD_tex.C0H2              = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureHPF.coef_c0_2;
        sharp.TwoD_tex.C0H3              = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureHPF.coef_c0_3;
        sharp.TwoD_tex.C0H4              = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureHPF.coef_c0_4;


        /* #8 CHIP_TEXTURE_CURVEMAPPING_STEP_T 28*/
        sharp.Tex_Pos_Curve.S1   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.pos_s1;
        sharp.Tex_Pos_Curve.S2   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.pos_s2;
        sharp.Tex_Pos_Curve.S3   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.pos_s3;
        sharp.Tex_Pos_Curve.S4   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.pos_s4;
        sharp.Tex_Pos_Curve.S5   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.pos_s5;
        sharp.Tex_Pos_Curve.S6   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.pos_s6;
        sharp.Tex_Pos_Curve.S7   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.pos_s7;
        sharp.Tex_Pos_Curve.S8   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.pos_s8;
        sharp.Tex_Pos_Curve.S9   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.pos_s9;
        sharp.Tex_Pos_Curve.S10  = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.pos_s10;
        sharp.Tex_Pos_Curve.S11  = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.pos_s11;
        sharp.Tex_Pos_Curve.S12  = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.pos_s12;
        sharp.Tex_Pos_Curve.S13  = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.pos_s13;
        sharp.Tex_Pos_Curve.S14  = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.pos_s14;
        sharp.Tex_Neg_Curve.S1   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.neg_s1;
        sharp.Tex_Neg_Curve.S2   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.neg_s2;
        sharp.Tex_Neg_Curve.S3   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.neg_s3;
        sharp.Tex_Neg_Curve.S4   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.neg_s4;
        sharp.Tex_Neg_Curve.S5   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.neg_s5;
        sharp.Tex_Neg_Curve.S6   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.neg_s6;
        sharp.Tex_Neg_Curve.S7   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.neg_s7;
        sharp.Tex_Neg_Curve.S8   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.neg_s8;
        sharp.Tex_Neg_Curve.S9   = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.neg_s9;
        sharp.Tex_Neg_Curve.S10  = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.neg_s10;
        sharp.Tex_Neg_Curve.S11  = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.neg_s11;
        sharp.Tex_Neg_Curve.S12  = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.neg_s12;
        sharp.Tex_Neg_Curve.S13  = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.neg_s13;
        sharp.Tex_Neg_Curve.S14  = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingStep.neg_s14;


	/* #9 CHIP_TEXTURE_CURVEMAPPING_UI_T 39 */
	sharp_gain.Texture_Level.G_Pos_Gain      = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.Gain_Pos;
	sharp_gain.Texture_Level.G_Neg_Gain      = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.Gain_Neg;
	sharp.Texture_Shp.G_Pos2                 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.Gain_Pos2;
	sharp.Texture_Shp.G_Neg2	         = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.Gain_Neg2;
	sharp_gain.Texture_Level.HV_POS_Gain     = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.Range_Pos;
	sharp_gain.Texture_Level.HV_NEG_Gain     = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.Range_Neg;
	sharp_gain.Texture_Level.LV_Gain	 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.Coring;
	sharp.Texture_Shp.Edg_D2_Shift_Bit       = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.Tex_Shift;
	sharp.Tex_Neg_Curve.enable               = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.enable;
	sharp.Tex_Pos_Curve.G0 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.pos_g0;
	sharp.Tex_Pos_Curve.G1 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.pos_g1;
	sharp.Tex_Pos_Curve.G2 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.pos_g2;
	sharp.Tex_Pos_Curve.G3 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.pos_g3;
	sharp.Tex_Pos_Curve.G4 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.pos_g4;
	sharp.Tex_Pos_Curve.G5 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.pos_g5;
	sharp.Tex_Pos_Curve.G6 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.pos_g6;
	sharp.Tex_Pos_Curve.G7 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.pos_g7;
	sharp.Tex_Pos_Curve.G8 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.pos_g8;
	sharp.Tex_Pos_Curve.G9 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.pos_g9;
	sharp.Tex_Pos_Curve.G10			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.pos_g10;
	sharp.Tex_Pos_Curve.G11			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.pos_g11;
	sharp.Tex_Pos_Curve.G12			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.pos_g12;
	sharp.Tex_Pos_Curve.G13			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.pos_g13;
	sharp.Tex_Pos_Curve.G14			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.pos_g14;
	sharp.Tex_Neg_Curve.G0 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.neg_g0;
	sharp.Tex_Neg_Curve.G1 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.neg_g1;
	sharp.Tex_Neg_Curve.G2 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.neg_g2;
	sharp.Tex_Neg_Curve.G3 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.neg_g3;
	sharp.Tex_Neg_Curve.G4 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.neg_g4;
	sharp.Tex_Neg_Curve.G5 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.neg_g5;
	sharp.Tex_Neg_Curve.G6 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.neg_g6;
	sharp.Tex_Neg_Curve.G7 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.neg_g7;
	sharp.Tex_Neg_Curve.G8 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.neg_g8;
	sharp.Tex_Neg_Curve.G9 			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.neg_g9;
	sharp.Tex_Neg_Curve.G10			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.neg_g10;
	sharp.Tex_Neg_Curve.G11			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.neg_g11;
	sharp.Tex_Neg_Curve.G12			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.neg_g12;
	sharp.Tex_Neg_Curve.G13			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.neg_g13;
	sharp.Tex_Neg_Curve.G14			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureCurveMappingUI.neg_g14;



         /* #10 CHIP_COMMON_HPF_T 38 */
        sharp.SR_H9.C0    = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h9_c0;
        sharp.SR_H9.C1    = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h9_c1;
        sharp.SR_H9.C2    = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h9_c2;
        sharp.SR_H9.C3    = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h9_c3;
        sharp.SR_H9.C4    = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h9_c4;
        sharp.SR_H13.C0   = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h13_c0;
        sharp.SR_H13.C1   = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h13_c1;
        sharp.SR_H13.C2   = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h13_c2;
        sharp.SR_H13.C3   = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h13_c3;
        sharp.SR_H13.C4   = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h13_c4;
        sharp.SR_H13.C5   = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h13_c5;
        sharp.SR_H13.C6   = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h13_c6;
        sharp.SR_H21.C0   = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h21_c0;
        sharp.SR_H21.C1   = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h21_c1;
        sharp.SR_H21.C2   = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h21_c2;
        sharp.SR_H21.C3   = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h21_c3;
        sharp.SR_H21.C4   = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h21_c4;
        sharp.SR_H21.C5   = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h21_c5;
        sharp.SR_H21.C6   = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h21_c6;
        sharp.SR_H21.C7   = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h21_c7;
        sharp.SR_H21.C8   = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h21_c8;
        sharp.SR_H21.C9   = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h21_c9;
        sharp.SR_H21.C10  = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.h21_c10;
        sharp.SR_V9_1.C0  = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.v91_c0;
        sharp.SR_V9_1.C1  = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.v91_c1;
        sharp.SR_V9_1.C2  = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.v91_c2;
        sharp.SR_V9_1.C3  = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.v91_c3;
        sharp.SR_V9_1.C4  = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.v91_c4;
        sharp.SR_V9_2.C0  = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.v92_c0;
        sharp.SR_V9_2.C1  = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.v92_c1;
        sharp.SR_V9_2.C2  = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.v92_c2;
        sharp.SR_V9_2.C3  = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.v92_c3;
        sharp.SR_V9_2.C4  = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.v92_c4;
        sharp.SR_V9_3.C0  = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.v93_c0;
        sharp.SR_V9_3.C1  = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.v93_c1;
        sharp.SR_V9_3.C2  = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.v93_c2;
        sharp.SR_V9_3.C3  = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.v93_c3;
        sharp.SR_V9_3.C4  = ptCHIP_SHARPNESS_UI_T->stSharpness.stCommonHPF.v93_c4;


        /* #11 CHIP_SLOPE_GAIN_MASK_T 11 */

        sharp.MK3_Table.Slope_gain_en            = ptCHIP_SHARPNESS_UI_T->stSharpness.stSlopeGainMask.Slope_Gain_Enable;
        sharp.MK3_Table.dir            			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stSlopeGainMask.DIR;
        sharp.MK3_Table.flat_method              = ptCHIP_SHARPNESS_UI_T->stSharpness.stSlopeGainMask.FH;
        sharp.MK3_Table.Gain_boundPos            = ptCHIP_SHARPNESS_UI_T->stSharpness.stSlopeGainMask.Gain_boundPos;
        sharp.MK3_Table.Gain_boundNeg            = ptCHIP_SHARPNESS_UI_T->stSharpness.stSlopeGainMask.Gain_boundNeg;
        sharp.MK3_Table.Gain_extendPos           = ptCHIP_SHARPNESS_UI_T->stSharpness.stSlopeGainMask.Gain_extendPos;
        sharp.MK3_Table.Gain_extendNeg           = ptCHIP_SHARPNESS_UI_T->stSharpness.stSlopeGainMask.Gain_extendNeg;
        sharp.MK3_Table.flat_enable              = ptCHIP_SHARPNESS_UI_T->stSharpness.stSlopeGainMask.flat_enable;
        sharp.MK3_Table.flat_offset              = ptCHIP_SHARPNESS_UI_T->stSharpness.stSlopeGainMask.flat_offset;
        sharp.MK3_Table.flat_gain                = ptCHIP_SHARPNESS_UI_T->stSharpness.stSlopeGainMask.flat_gain;
        sharp.MK3_Table.flat_range_h             = ptCHIP_SHARPNESS_UI_T->stSharpness.stSlopeGainMask.flat_range;


        /* #12 CHIP_VERTICAL_HPF_T 6 */
        sharp_other_ctrl.vpk_en = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalHPF.vpk_en;
        sharp.Vertical.C0       = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalHPF.vpk_c0;
        sharp.Vertical.C1       = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalHPF.vpk_c1;
        sharp.Vertical.C2       = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalHPF.vpk_c2;
        sharp.Vertical.C3       = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalHPF.vpk_c3;
        sharp.Vertical.C4       = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalHPF.vpk_c4;


        /* #13 CHIP_VERTICAL_CURVEMAPPING_STEP_T 28 */

        sharp.V_Pos_Curve.S1     = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.pos_s1;
        sharp.V_Pos_Curve.S2     = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.pos_s2;
        sharp.V_Pos_Curve.S3     = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.pos_s3;
        sharp.V_Pos_Curve.S4     = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.pos_s4;
        sharp.V_Pos_Curve.S5     = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.pos_s5;
        sharp.V_Pos_Curve.S6     = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.pos_s6;
        sharp.V_Pos_Curve.S7     = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.pos_s7;
        sharp.V_Pos_Curve.S8     = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.pos_s8;
        sharp.V_Pos_Curve.S9     = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.pos_s9;
        sharp.V_Pos_Curve.S10    = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.pos_s10;
        sharp.V_Pos_Curve.S11    = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.pos_s11;
        sharp.V_Pos_Curve.S12    = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.pos_s12;
        sharp.V_Pos_Curve.S13    = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.pos_s13;
        sharp.V_Pos_Curve.S14    = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.pos_s14;
        sharp.V_Neg_Curve.S1     = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.neg_s1;
        sharp.V_Neg_Curve.S2     = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.neg_s2;
        sharp.V_Neg_Curve.S3     = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.neg_s3;
        sharp.V_Neg_Curve.S4     = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.neg_s4;
        sharp.V_Neg_Curve.S5     = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.neg_s5;
        sharp.V_Neg_Curve.S6     = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.neg_s6;
        sharp.V_Neg_Curve.S7     = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.neg_s7;
        sharp.V_Neg_Curve.S8     = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.neg_s8;
        sharp.V_Neg_Curve.S9     = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.neg_s9;
        sharp.V_Neg_Curve.S10    = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.neg_s10;
        sharp.V_Neg_Curve.S11    = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.neg_s11;
        sharp.V_Neg_Curve.S12    = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.neg_s12;
        sharp.V_Neg_Curve.S13    = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.neg_s13;
        sharp.V_Neg_Curve.S14    = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingStep.neg_s14;

        /* #14 CHIP_VERTICAL_CURVEMAPPING_UI_T 48 */
        sharp_gain.Vertical_Level.G_Pos_Gain             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.Gain_Pos;
        sharp_gain.Vertical_Level.G_Neg_Gain             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.Gain_Neg;
        sharp_gain.Vertical_Level.LV_Gain                = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.Coring;
        sharp.Vertical.G_Pos2                            = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.Gain_Pos2;
        sharp.Vertical.G_Neg2                            = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.Gain_Neg2;
        sharp_gain.Vertical_Level.HV_POS_Gain            = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.Range_Pos;
        sharp_gain.Vertical_Level.HV_NEG_Gain            = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.Range_Neg;
        //1;// K5Lp fixed 1                                = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.V_Parameters_En;
	sharp_gain.Vertical_Edge_Level.G_Pos_Gain        = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.Edge_Gain_Pos;
	sharp_gain.Vertical_Edge_Level.G_Neg_Gain        = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.Edge_Gain_Neg;
	sharp_gain.Vertical_Edge_Level.LV_Gain           = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.Edge_Coring;
	sharp.Vertical.G_Pos2_edge                       = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.Edge_Gain_Pos2;
	sharp.Vertical.G_Neg2_edge                       = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.Edge_Gain_Neg2;
	sharp_gain.Vertical_Edge_Level.HV_POS_Gain       = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.Edge_Range_Pos;
	sharp_gain.Vertical_Edge_Level.HV_NEG_Gain       = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.Edge_Range_Neg;
	sharp.Vertical.V_Tex_en                          = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.v_tex_en;
	sharp.Vertical.V_Dering_En                       = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.v_dering_en;
	sharp.V_Neg_Curve.enable                         = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.enable;
        sharp.V_Pos_Curve.G0                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.pos_g0;
        sharp.V_Pos_Curve.G1                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.pos_g1;
        sharp.V_Pos_Curve.G2                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.pos_g2;
        sharp.V_Pos_Curve.G3                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.pos_g3;
        sharp.V_Pos_Curve.G4                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.pos_g4;
        sharp.V_Pos_Curve.G5                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.pos_g5;
        sharp.V_Pos_Curve.G6                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.pos_g6;
        sharp.V_Pos_Curve.G7                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.pos_g7;
        sharp.V_Pos_Curve.G8                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.pos_g8;
        sharp.V_Pos_Curve.G9                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.pos_g9;
        sharp.V_Pos_Curve.G10                            = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.pos_g10;
        sharp.V_Pos_Curve.G11                            = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.pos_g11;
        sharp.V_Pos_Curve.G12                            = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.pos_g12;
        sharp.V_Pos_Curve.G13                            = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.pos_g13;
        sharp.V_Pos_Curve.G14                            = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.pos_g14;
        sharp.V_Neg_Curve.G0                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.neg_g0;
        sharp.V_Neg_Curve.G1                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.neg_g1;
        sharp.V_Neg_Curve.G2                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.neg_g2;
        sharp.V_Neg_Curve.G3                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.neg_g3;
        sharp.V_Neg_Curve.G4                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.neg_g4;
        sharp.V_Neg_Curve.G5                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.neg_g5;
        sharp.V_Neg_Curve.G6                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.neg_g6;
        sharp.V_Neg_Curve.G7                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.neg_g7;
        sharp.V_Neg_Curve.G8                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.neg_g8;
        sharp.V_Neg_Curve.G9                             = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.neg_g9;
        sharp.V_Neg_Curve.G10                            = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.neg_g10;
        sharp.V_Neg_Curve.G11                            = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.neg_g11;
        sharp.V_Neg_Curve.G12                            = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.neg_g12;
        sharp.V_Neg_Curve.G13                            = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.neg_g13;
        sharp.V_Neg_Curve.G14                            = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalCurveMappingUI.neg_g14;


        /* #15 CHIP_VERTICAL_SLOPE_GAIN_MASK_T 10 */
	sharp.MK3_Table.Slope_gain_v_en		 = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalSlopeGainMask.Slope_Gain_V_Enable;
	sharp.MK3_Table.dir_v			 = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalSlopeGainMask.DIR;
	sharp.MK3_Table.Gain_boundPos_v		 = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalSlopeGainMask.Gain_boundPos;
	sharp.MK3_Table.Gain_boundNeg_v		 = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalSlopeGainMask.Gain_boundNeg;
	sharp.MK3_Table.Gain_extendPos_v	 = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalSlopeGainMask.Gain_extendPos;
	sharp.MK3_Table.Gain_extendNeg_v	 = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalSlopeGainMask.Gain_extendNeg;
	sharp.MK3_Table.flat_v_enable		 = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalSlopeGainMask.flat_v_enable;
	sharp.MK3_Table.flat_offset_v		 = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalSlopeGainMask.flat_offset_v;
	sharp.MK3_Table.flat_gain_v              = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalSlopeGainMask.flat_gain_v;
	sharp.MK3_Table.flat_range_v		 = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalSlopeGainMask.flat_range_v;

        /* #16 CHIP_MKIII_T 61 */
        sharp.MK3_Table.Gain_By_Y_En            = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.gain_by_y_en;
        sharp.MK3_Table.gain_by_y_v_en          = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.gain_by_y_v_en;
        sharp.MK3_Table.gain_by_y_tex_en        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.gain_by_y_tex_en;
        sharp.MK3_Table.gain_by_y_edg_en        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.gain_by_y_edg_en;
        sharp.MK3_Table.Lpf_Weigh_En            = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.lpf_weight_en;
        sharp.MK3_Table.lpf_weigh_edg_en        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.lpf_weight_edg_en;
        sharp.MK3_Table.lpf_weigh_tex_en        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.lpf_weight_tex_en;
        sharp.MK3_Table.MaxMin_Gain_En          = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.max_min_gain_en;
        sharp.MK3_Table.max_min_gain_v_en       = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.max_min_v_en;
        sharp.MK3_Table.max_min_gain_tex_en     = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.max_min_tex_en;
        sharp.MK3_Table.max_min_gain_edg_en     = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.max_min_edg_en;
        sharp.MK3_Table.lv_by_y_enable          = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.lv_by_Y_enable;
        sharp.MK3_Table.lv_by_y_edg_enable      = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.lv_by_y_edg_enable;
        sharp.MK3_Table.lv_by_y_v_edg_enable    = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.lv_by_y_v_edg_enable;
        sharp.MK3_Table.lv_by_y_tex_enable      = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.lv_by_y_tex_enable;
        sharp.MK3_Table.lv_by_y_v_enable        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.lv_by_y_v_enable;
        sharp.MK3_Table.Lpf2d_C0                = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.coef_c0;
        sharp.MK3_Table.Lpf2d_C1                = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.coef_c1;
        sharp.MK3_Table.Lpf2d_C2                = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.coef_c2;
        sharp.MK3_Table.Lpf2d_C3                = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.coef_c3;
        sharp.MK3_Table.Lpf2d_C4                = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.coef_c4;
        sharp.MK3_Table.Lpf2d_C5                = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.coef_c5;
        sharp.MK3_Table.MaxMin_Step_1           = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.maxmin_s1;
        sharp.MK3_Table.MaxMin_Step_2           = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.maxmin_s2;
        sharp.MK3_Table.MaxMin_Step_3           = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.maxmin_s3;
        sharp.MK3_Table.MaxMin_Step_4           = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.maxmin_s4;
        sharp.MK3_Table.MaxMin_Step_5           = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.maxmin_s5;
        sharp.MK3_Table.MaxMin_Step_6           = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.maxmin_s6;
        sharp.MK3_Table.MaxMin_Gain_0           = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.maxmin_g0;
        sharp.MK3_Table.MaxMin_Gain_1           = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.maxmin_g1;
        sharp.MK3_Table.MaxMin_Gain_2           = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.maxmin_g2;
        sharp.MK3_Table.MaxMin_Gain_3           = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.maxmin_g3;
        sharp.MK3_Table.MaxMin_Gain_4           = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.maxmin_g4;
        sharp.MK3_Table.MaxMin_Gain_5           = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.maxmin_g5;
        sharp.MK3_Table.MaxMin_Gain_6           = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.maxmin_g6;
        sharp.MK3_Table.Lpf_Step_1              = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.blending_s1;
        sharp.MK3_Table.Lpf_Step_2              = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.blending_s2;
        sharp.MK3_Table.Lpf_Step_3              = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.blending_s3;
        sharp.MK3_Table.Lpf_Step_4              = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.blending_s4;
        sharp.MK3_Table.Lpf_Step_5              = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.blending_s5;
        sharp.MK3_Table.Lpf_Step_6              = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.blending_s6;
        sharp.MK3_Table.Lpf_Weit_0              = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.blending_g0;
        sharp.MK3_Table.Lpf_Weit_1              = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.blending_g1;
        sharp.MK3_Table.Lpf_Weit_2              = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.blending_g2;
        sharp.MK3_Table.Lpf_Weit_3              = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.blending_g3;
        sharp.MK3_Table.Lpf_Weit_4              = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.blending_g4;
        sharp.MK3_Table.Lpf_Weit_5              = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.blending_g5;
        sharp.MK3_Table.Lpf_Weit_6              = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.blending_g6;
        sharp.MK3_Table.Gain_By_Y_Step_1        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.y_s1;
        sharp.MK3_Table.Gain_By_Y_Step_2        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.y_s2;
        sharp.MK3_Table.Gain_By_Y_Step_3        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.y_s3;
        sharp.MK3_Table.Gain_By_Y_Step_4        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.y_s4;
        sharp.MK3_Table.Gain_By_Y_Step_5        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.y_s5;
        sharp.MK3_Table.Gain_By_Y_Step_6        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.y_s6;
        sharp.MK3_Table.Gain_By_Y_Gain_0        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.y_g0;
        sharp.MK3_Table.Gain_By_Y_Gain_1        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.y_g1;
        sharp.MK3_Table.Gain_By_Y_Gain_2        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.y_g2;
        sharp.MK3_Table.Gain_By_Y_Gain_3        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.y_g3;
        sharp.MK3_Table.Gain_By_Y_Gain_4        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.y_g4;
        sharp.MK3_Table.Gain_By_Y_Gain_5        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.y_g5;
        sharp.MK3_Table.Gain_By_Y_Gain_6        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMKIII.y_g6;

	// lesley 1014
	#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	{
	// extern void drvif_color_get_DB_AI_SHP(CHIP_SHARPNESS_UI_T *ptCHIP_SHARPNESS_UI_T); // V4L2_ERR
	//drvif_color_get_DB_AI_SHP(ptCHIP_SHARPNESS_UI_T); // V4L2_ERR
	}
	#endif
	// end lesley 1014
	
	/* #17 CHIP_CTI_T 18 */
	i_dcti.VipNewIDcti_ctl1.dcti_en   	= ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI.i_dcti_enable;
	i_dcti.VipNewIDcti_ctl2.uvsync_en 	= ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI.i_dcti_uv_sync_enable;
	//i_dcti.VipNewIDcti_ctl1.uvalign_en	= ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI.i_dcti_uv_align_enable;
	i_dcti.VipNewIDcti_ctl1.dcti_mode 	= ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI.i_dcti_mode;
	i_dcti.VipNewIDcti_ctl1.uvgain    	= ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI.i_dcti_gain;
	i_dcti.VipNewIDcti_ctl1.maxlen    	= ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI.i_dcti_Max_search_range;
	i_dcti.VipNewIDcti_ctl1.psmth     	= ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI.i_dcti_Threshold_of_smooth;
	d_dcti.VipNewDDcti_ctl1.dcti_en   	= ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI.d_dcti_enable;
	d_dcti.VipNewDDcti_ctl2.uvsync_en 	= ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI.d_dcti_uv_sync_enable;
	d_dcti.VipNewDDcti_ctl1.uvalign_en	= ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI.d_dcti_uv_align_enable;
	d_dcti.VipNewDDcti_ctl1.dcti_mode 	= ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI.d_dcti_mode;
	d_dcti.VipNewDDcti_ctl1.uvgain    	= ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI.d_dcti_gain;
	d_dcti.VipNewDDcti_ctl1.maxlen    	= ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI.d_dcti_Max_search_range;
	d_dcti.VipNewDDcti_ctl1.psmth     	= ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI.d_dcti_Threshold_of_smooth;
	v_cti.vcti_en                     	= ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI.vcti_enable;
	v_cti.vcti_select                 	= ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI.vcti_type;
	v_cti.vcti_gain                   	= ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI.vcti_gain;
	v_cti.vcti_type3_thd              	= ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI.vcti_type3_thd;


        // v gain not in db , same as  u gain
	i_dcti.VipNewIDcti_ctl1_v.uvgain_v  = ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI.i_dcti_gain;


	/*#18 CHIP_HLTI_T 36*/
	drv_shp_Dlti.DLTI_H_1.dlti_h_en			= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.d_LTI_enable;
	drv_shp_Dlti.DLTI_H_1.Can_shift			= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.d_LTI_can_shift;
	drv_shp_Dlti.DLTI_H_1.maxlen			= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.d_LTI_maxlen;
	drv_shp_Dlti.DLTI_H_1.pnth			= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.d_LTI_pnth;
	drv_shp_Dlti.DLTI_H_2.ffd2_en                   = ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.ffd2_Enable;
	drv_shp_Dlti.DLTI_H_1.Data_sel			= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Data_sel;
	// drv_shp_Dlti.DLTI_H_1.ovc_extend                = ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.ovc_extend;
	drv_shp_Dlti.DLTI_H_1.ffd0_start                = ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.ffd0_start;
	drv_shp_Dlti.DLTI_H_2.D1_CP_shift		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.D1_CP_Shift;
	drv_shp_Dlti.DLTI_H_2.DCP_th			= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.DCP_th;
	drv_shp_Dlti.DLTI_H_2.flat_s_range		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Flat_s_range;
	drv_shp_Dlti.DLTI_H_2.flat_score_thl		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Flat_score_th1;
	// drv_shp_Dlti.DLTI_H_1.Ffd1_th			= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Ffd1_th;
	drv_shp_Dlti.DLTI_H_1.Ffd1_ratio		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Ffd1_ratio;
	// drv_shp_Dlti.DLTI_H_2.Ffd2_range2R		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Ffd2_range2R;
	// drv_shp_Dlti.DLTI_H_2.Ffd2_range2L		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Ffd2_range2L;
	// drv_shp_Dlti.DLTI_H_2.Ffd2_ratio		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Ffd2_ratio;
	drv_shp_Dlti.DLTI_H_OverEnhance.gain_pos 	= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.gain_pos;
	drv_shp_Dlti.DLTI_H_OverEnhance.offset_pos	= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.offset_pos;
	drv_shp_Dlti.DLTI_H_OverEnhance.gain_neg 	= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.gain_neg;
	drv_shp_Dlti.DLTI_H_OverEnhance.offset_neg	= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.offset_neg;
	drv_shp_Dlti.DLTI_H_Tnoff.Tnoff0		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Tnoff0;
	// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff1		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Tnoff1;
	// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff2		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Tnoff2;
	// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff3		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Tnoff3;
	// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff4		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Tnoff4;
	// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff5		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Tnoff5;
	// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff6		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Tnoff6;
	// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff7		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Tnoff7;
	// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff8		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Tnoff8;
	// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff9		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Tnoff9;
	// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff10		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Tnoff10;
	// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff11		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Tnoff11;
	// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff12		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Tnoff12;
	// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff13		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Tnoff13;
	// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff14		= ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI.Tnoff14;

        /*==========set from DB V LTI=============*/
        /*#19 CHIP_VLTI_T 19 */
        drv_shp_Dlti.DLTI_V_1.dlti_v_en                 = ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI.d_LTI_enable;
        drv_shp_Dlti.DLTI_V_1.can_shift_v               = ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI.Can_shift_v;
        drv_shp_Dlti.DLTI_V_1.maxlen_v                  = ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI.maxlen_v;
        drv_shp_Dlti.DLTI_V_1.pnth_v                    = ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI.pnth_v;
        // drv_shp_Dlti.DLTI_V_1.ovc_v_extend              = ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI.ovc_extend;
        drv_shp_Dlti.DLTI_V_1.v_ffd0_start              = ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI.ffd0_start;
        // drv_shp_Dlti.DLTI_V_1.ffd1_th                   = ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI.ffd1_th_v;
        drv_shp_Dlti.DLTI_V_1.ffd1_ratio                = ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI.ffd1_ratio_v;
        drv_shp_Dlti.DLTI_V_OverEnhance.gain_pos        = ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI.gain_pos_v;
        drv_shp_Dlti.DLTI_V_OverEnhance.offset_pos      = ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI.offset_pos_v;
        drv_shp_Dlti.DLTI_V_OverEnhance.gain_neg        = ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI.gain_neg_v;
        drv_shp_Dlti.DLTI_V_OverEnhance.offset_neg      = ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI.offset_neg_v;
        drv_shp_Dlti.DLTI_V_Tnoff.Tnoff0                = ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI.Tnoff0_v;
        // drv_shp_Dlti.DLTI_V_Tnoff.Tnoff1                = ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI.Tnoff1_v;
        // drv_shp_Dlti.DLTI_V_Tnoff.Tnoff2                = ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI.Tnoff2_v;
        // drv_shp_Dlti.DLTI_V_Tnoff.Tnoff3                = ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI.Tnoff3_v;
        // drv_shp_Dlti.DLTI_V_Tnoff.Tnoff4                = ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI.Tnoff4_v;
        // drv_shp_Dlti.DLTI_V_Tnoff.Tnoff5                = ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI.Tnoff5_v;
        // drv_shp_Dlti.DLTI_V_Tnoff.Tnoff6                = ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI.Tnoff6_v;

        /* #20 CHIP_MULTIBAND_PEAKING_HPF_T 8 */
        MBPK.MBPK_Ctrl.mb_peaking_en            = ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingHPF.mb_peaking_en;
        MBPK.MBPK_H_Coef.MB_Hor_Filter_C0       = ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingHPF.h_c0;
        MBPK.MBPK_H_Coef.MB_Hor_Filter_C1       = ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingHPF.h_c1;
        MBPK.MBPK_H_Coef.MB_Hor_Filter_C2       = ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingHPF.h_c2;
        MBPK.MBPK_H_Coef.MB_Hor_Filter_C3       = ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingHPF.h_c3;
        MBPK.MBPK_V_Coef.MB_Ver_Filter_C0       = ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingHPF.v_c0;
        MBPK.MBPK_V_Coef.MB_Ver_Filter_C1       = ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingHPF.v_c1;
        MBPK.MBPK_V_Coef.MB_Ver_Filter_C2       = ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingHPF.v_c2;


	/*21 CHIP_MULTIBAND_PEAKING_UI_T 18*/

        MBPK.MBPK_Ctrl.MB_H_En			= ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI.mb_h_en;
        MBPK.MBPK_H_Table.MB_Gain_Pos		= ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI.H_Gain_Pos;
        MBPK.MBPK_H_Table.MB_Gain_Neg		= ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI.H_Gain_Neg;
        MBPK.MBPK_H_Table.MB_LV		        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI.H_Coring;
        MBPK.MBPK_H_Table.MB_Gain_Pos2 	        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI.H_Gain_Pos2;
        MBPK.MBPK_H_Table.MB_Gain_Neg2 	        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI.H_Gain_Neg2;
        MBPK.MBPK_H_Table.MB_HV_Neg		= ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI.H_Range_Pos;
        MBPK.MBPK_H_Table.MB_HV_Pos		= ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI.H_Range_Neg;
        MBPK.MBPK_H_Table.MB_D2_shift_bit 	= ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI.H_Edge_Shift;
        MBPK.MBPK_Ctrl.MB_V_En 		        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI.mb_v_en;
        MBPK.MBPK_V_Table.MB_Gain_Pos_V	        = ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI.V_Gain_Pos;
        MBPK.MBPK_V_Table.MB_Gain_Neg_V 	= ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI.V_Gain_Neg;
        MBPK.MBPK_V_Table.MB_LV_V 		= ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI.V_Coring ;
        MBPK.MBPK_V_Table.MB_Gain_Pos2_V	= ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI.V_Gain_Pos2;
        MBPK.MBPK_V_Table.MB_Gain_Neg2_V	= ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI.V_Gain_Neg2;
        MBPK.MBPK_V_Table.MB_HV_Pos_V		= ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI.V_Range_Pos;
        MBPK.MBPK_V_Table.MB_HV_Neg_V		= ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI.V_Range_Neg;
        MBPK.MBPK_V_Table.MB_D2_shift_bit_V	= ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI.V_Edge_Shift;

	/*# 22 CHIP_ISE_T*/
 	sharp.SEGPK_ISE.ise_en         = ptCHIP_SHARPNESS_UI_T->stSharpness.stIse.ise_en;
 	sharp.SEGPK_ISE.ise_step       = ptCHIP_SHARPNESS_UI_T->stSharpness.stIse.ise_step;
 	sharp.SEGPK_ISE.ise_scale      = ptCHIP_SHARPNESS_UI_T->stSharpness.stIse.ise_scale;
 	sharp.SEGPK_ISE.ise_ub         = ptCHIP_SHARPNESS_UI_T->stSharpness.stIse.ise_ub;
 	sharp.SEGPK_ISE.ise_h_st       = ptCHIP_SHARPNESS_UI_T->stSharpness.stIse.ise_h_st;
 	sharp.SEGPK_ISE.ise_h_end      = ptCHIP_SHARPNESS_UI_T->stSharpness.stIse.ise_h_end;
 	sharp.SEGPK_ISE.ise_v_st       = ptCHIP_SHARPNESS_UI_T->stSharpness.stIse.ise_v_st;
 	sharp.SEGPK_ISE.ise_v_end      = ptCHIP_SHARPNESS_UI_T->stSharpness.stIse.ise_v_end;

	/*# 23 CHIP_FLAT_FILTER_T*/
	sharp.Texture_Flat_Shp.en_tex	= ptCHIP_SHARPNESS_UI_T->stSharpness.stFlatfilter.tex_flat_filter_en;
	sharp.Texture_Flat_Shp.C0H1 	= ptCHIP_SHARPNESS_UI_T->stSharpness.stFlatfilter.coef_c0_2;
	sharp.Texture_Flat_Shp.C0H2     = ptCHIP_SHARPNESS_UI_T->stSharpness.stFlatfilter.coef_c0_1;
	sharp.Texture_Flat_Shp.C0	= ptCHIP_SHARPNESS_UI_T->stSharpness.stFlatfilter.coef_c0_0;
	sharp.Texture_Flat_Shp.C1	= ptCHIP_SHARPNESS_UI_T->stSharpness.stFlatfilter.coef_c1_0;
	sharp.Texture_Flat_Shp.C2	= ptCHIP_SHARPNESS_UI_T->stSharpness.stFlatfilter.coef_c2_0;
	sharp.Texture_Flat_Shp.C3	= ptCHIP_SHARPNESS_UI_T->stSharpness.stFlatfilter.coef_c3_0;




       /*#24 CHIP_EDGE_EMF_UI_T 14*/
       sharp.Edge_EMF_Table.EMF_En             = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeEMF.emf_enable;
       sharp.Edge_EMF_Table.EMF_Range          = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeEMF.emf_range;
       sharp.Edge_EMF_Table.EMF_Shift          = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeEMF.emf_shiftbit;
       sharp.Edge_EMF_Table.Blend_wt           = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeEMF.uBlend_wt;
       sharp.Edge_EMF_Table.emf_fixextent_neg  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeEMF.fix_extend_neg;
       sharp.Edge_EMF_Table.emf_fixextent_pos  = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeEMF.fix_extend_pos;
       sharp.Edge_EMF_Table.Seg_0_x            = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeEMF.seg0_x;
       sharp.Edge_EMF_Table.Seg_1_x            = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeEMF.seg1_x;
       sharp.Edge_EMF_Table.Seg_0_Gain         = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeEMF.gain0;
       sharp.Edge_EMF_Table.Seg_1_Gain         = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeEMF.gain1;
       sharp.Edge_EMF_Table.Seg_2_Gain         = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeEMF.gain2;
       sharp.Edge_EMF_Table.Seg_0_Offset       = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeEMF.Offset0;
       sharp.Edge_EMF_Table.v5                 = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeEMF.v5;
       sharp.Edge_EMF_Table.h11                = ptCHIP_SHARPNESS_UI_T->stSharpness.stEdgeEMF.h11;


       /*#24 CHIP_TEXTURE_EMF_UI_T 15*/
       sharp.Text_EMF_Table.EMF_En             = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureEMF.emf_enable;
       sharp.Text_EMF_Table.EMF_Range          = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureEMF.emf_range;
       sharp.Text_EMF_Table.EMF_Shift          = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureEMF.emf_shiftbit;
       sharp.Text_EMF_Table.Blend_wt           = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureEMF.blend_wt;
       sharp.Text_EMF_Table.emf_fixextent_neg  = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureEMF.fix_extend_neg;
       sharp.Text_EMF_Table.emf_fixextent_pos  = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureEMF.fix_extend_pos;
       sharp.Text_EMF_Table.Seg_0_x            = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureEMF.seg0_x;
       sharp.Text_EMF_Table.Seg_1_x            = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureEMF.seg1_x;
       sharp.Text_EMF_Table.Seg_0_Gain         = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureEMF.gain0;
       sharp.Text_EMF_Table.Seg_1_Gain         = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureEMF.gain1;
       sharp.Text_EMF_Table.Seg_2_Gain         = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureEMF.gain2;
       sharp.Text_EMF_Table.Seg_0_Offset       = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureEMF.Offset0;
       sharp.Text_EMF_Table.EMF_texgain        = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureEMF.emf_texgain;
       sharp.Text_EMF_Table.v5                 = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureEMF.v5;
       sharp.Text_EMF_Table.h11                = ptCHIP_SHARPNESS_UI_T->stSharpness.stTextureEMF.h11;

       /*#25 CHIP_VERTICAL_EMF_UI_T 2*/
       sharp.Vertical.Vemf_En                  = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalEMF.vemf_en;
       sharp.Vertical.Vext_Reg                 = ptCHIP_SHARPNESS_UI_T->stSharpness.stVerticalEMF.vext_reg;



        // patch for small review window
	if (VIP_system_info_structure_table != NULL) {
		if (VIP_system_info_structure_table->Input_src_Type == _SRC_TV) {
			scaleup_dm_uzu_ctrl_RBUS dm_uzu_Ctrl_reg;
			scaleup_dm_uzu_scale_hor_factor_RBUS dm_uzu_Scale_Hor_Factor_REG;
			dm_uzu_Ctrl_reg.regValue = rtd_inl(SCALEUP_DM_UZU_Ctrl_reg);
			dm_uzu_Scale_Hor_Factor_REG.regValue = rtd_inl(SCALEUP_DM_UZU_Scale_Hor_Factor_reg);

			if ((dm_uzu_Ctrl_reg.h_zoom_en == 0 && dm_uzu_Ctrl_reg.v_zoom_en == 0) ||
				(dm_uzu_Ctrl_reg.h_zoom_en == 1 && dm_uzu_Scale_Hor_Factor_REG.hor_factor >= 862017)) {
				sharp.TwoD_Peak_AdaptCtrl.Peaking_Enable = 0;
			}
		}
		//if((Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP) == FALSE) && (get_scaler_connect_source(SLR_MAIN_DISPLAY) == _SRC_HDMI)){		// HDMI RGB 444 input
		//	sharp.TwoD_Peak_AdaptCtrl.Peaking_Enable = 0;
		//}
	}




        /*SR*/


        sharp.SR_Ring_H11.C0 = ptCHIP_SR_UI_T->ring_h_c0;
        sharp.SR_Ring_H11.C1 = ptCHIP_SR_UI_T->ring_h_c1;
        sharp.SR_Ring_H11.C2 = ptCHIP_SR_UI_T->ring_h_c2;
        sharp.SR_Ring_H11.C3 = ptCHIP_SR_UI_T->ring_h_c3;
        sharp.SR_Ring_H11.C4 = ptCHIP_SR_UI_T->ring_h_c4;
        sharp.SR_Ring_H11.C5 = ptCHIP_SR_UI_T->ring_h_c5;

        sharp.SR_Ring_V9.C0 = ptCHIP_SR_UI_T->ring_v_c0;
        sharp.SR_Ring_V9.C1 = ptCHIP_SR_UI_T->ring_v_c1;
        sharp.SR_Ring_V9.C2 = ptCHIP_SR_UI_T->ring_v_c2;
        sharp.SR_Ring_V9.C3 = ptCHIP_SR_UI_T->ring_v_c3;
        sharp.SR_Ring_V9.C4 = ptCHIP_SR_UI_T->ring_v_c4;

        sharp.SR_Control.Ring_Gen_Gain = ptCHIP_SR_UI_T->Ring_Gen_Gain;
        sharp.SR_Control.Period = ptCHIP_SR_UI_T->Cur_Period;
        sharp.SR_Control.Gain = ptCHIP_SR_UI_T->Cur_Gain;
        sharp.SR_Control.Offset = ptCHIP_SR_UI_T->Cur_Offset;

	sharp_other_ctrl.enhance_mode =2;
        sharp_other_ctrl.segpk_mode=2;
        sharp_other_ctrl.texpk_en=1; //workaround ,this bit lg should be change to 1
        //sharp_other_ctrl.edgpk_en=1;
        //sharp_other_ctrl.vpk_en=1;

	drvif_color_Set_Sharpness_Table((DRV_Sharpness_Table *)&sharp);
	drvif_color_set_Sharpness_level((DRV_Sharpness_Level *)&sharp_gain);
        drvif_color_Set_Sharpness_Other_Table((DRV_Sharpness_Other_Ctrl *)&sharp_other_ctrl);

	//fwif_color_set_iDcti_dDcti_tv006(0, 0, &ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI);
	//fwif_color_set_Vdcti_tv006(gVip_Table->VIP_QUALITY_Coef[vip_src][VIP_QUALITY_FUNCTION_V_DCTi], &ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI);
	//fwif_color_set_dLti_tv006(gVip_Table->VIP_QUALITY_Coef[vip_src][VIP_QUALITY_FUNCTION_DLTi], &ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI,&ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI);
	//fwif_color_set_MultiBand_Peaking_tv006(gVip_Table->VIP_QUALITY_Extend3_Coef[vip_src][VIP_QUALITY_FUNCTION_MB_Peaking], &ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingHPF, &ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI);

	fwif_color_set_iDcti_dDcti_tv006(0, 0, &ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI);
	fwif_color_set_Vdcti_tv006(gVip_Table->VIP_QUALITY_Coef[vip_src][VIP_QUALITY_FUNCTION_V_DCTi], &ptCHIP_SHARPNESS_UI_T->stSharpness.stCTI);
	fwif_color_set_dLti_tv006(gVip_Table->VIP_QUALITY_Coef[vip_src][VIP_QUALITY_FUNCTION_DLTi], &ptCHIP_SHARPNESS_UI_T->stSharpness.stHLTI,&ptCHIP_SHARPNESS_UI_T->stSharpness.stVLTI);
	fwif_color_set_MultiBand_Peaking_tv006(gVip_Table->VIP_QUALITY_Extend3_Coef[vip_src][VIP_QUALITY_FUNCTION_MB_Peaking], &ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingHPF, &ptCHIP_SHARPNESS_UI_T->stSharpness.stMultibandPeakingUI);

        //drvif_color_Get_Sharpness_Table(&sharp);
        //drvif_color_get_Sharpness_level(&sharp_gain);
        //drvif_color_Get_Sharpness_Other_Table(&sharp_other_ctrl);
        //drvif_color_get_inewdcti_table(&i_dcti);
        //drvif_color_get_dnewdcti_table(&d_dcti);
        //drvif_color_scaler_getds_vcti(0, &v_cti);
        //drvif_color_Get_D_DLTI_Table(&drv_shp_Dlti);
        //drvif_color_Get_MBPK_Table(&MBPK);



}

void fwif_color_Get_sharpness_table_TV006(short *xTmp)
{
	//static CHIP_SR_UI_T *ptCHIP_SR_UI_T;
	DRV_Sharpness_Table *pSharp;
	UINT16 i=0;

	pSharp = dvr_malloc(sizeof(DRV_Sharpness_Table));
	
	if (pSharp == NULL)
		return;

	drvif_color_Get_Sharpness_Table(pSharp);

	{
		DRV_Sharpness_Level sharp_gain;
		DRV_Sharpness_Other_Ctrl sharp_other_ctrl;
		
		drvif_color_get_Sharpness_level(&sharp_gain);
		drvif_color_Get_Sharpness_Other_Table(&sharp_other_ctrl);
		
		/*#1 CHIP_SEGMENTATION_T 13 +12*/
		xTmp[0] = pSharp->TwoD_Peak_AdaptCtrl.Edge_Mode;
		xTmp[1] = pSharp->TwoD_Peak_AdaptCtrl.Sobel_Mode;
		xTmp[2] = pSharp->TwoD_Peak_AdaptCtrl.Edgextrange;
		xTmp[3] = pSharp->TwoD_Peak_AdaptCtrl.Sobel_Upbnd ;
		xTmp[4] = pSharp->TwoD_Peak_AdaptCtrl.Sobel_Stepbit;
		xTmp[5] = pSharp->TwoD_Peak_AdaptCtrl.Zdiff_Upbnd  ;
		xTmp[6] = pSharp->TwoD_Peak_AdaptCtrl.Zdiff_Stepbit;
		xTmp[7] = pSharp->TwoD_Peak_AdaptCtrl.Zigsftbit_Reg;
		xTmp[8] = pSharp->TwoD_Peak_AdaptCtrl.Tex_En;
		xTmp[9] = pSharp->TwoD_Peak_AdaptCtrl.Tex_Upbnd ;
		xTmp[10] = pSharp->TwoD_Peak_AdaptCtrl.Tex_Stepbit;
		xTmp[11] = pSharp->TwoD_Peak_AdaptCtrl.Lpf_Mode;
		xTmp[12] = pSharp->TwoD_Peak_AdaptCtrl.Lpf_Filter;



		for(i=0;i<13;i++)
		{
			rtd_pr_vpq_emerg(" CHIP_SEGMENTATION_T[%d]=%d \n",i,xTmp[i]);
		}

		/*#2 CHIP_EDGE_CORING_MODE_T 11 +23*/
		xTmp[13] = pSharp->MK3_Table.EdgeCoring_En;
		xTmp[14] = pSharp->MK3_Table.EdgeCoring_Gain_En;
		xTmp[15] = pSharp->MK3_Table.EdgeCoring_Bigedge_Ratio;
		xTmp[16] = pSharp->MK3_Table.EdgeCoring_Smalledge_Ratio;
		xTmp[17] = pSharp->MK3_Table.EdgeCoring_Deltaedge_Ratio;
		xTmp[18] = pSharp->MK3_Table.EdgeCoring_Smalledge_Ulevel;
		xTmp[19] = pSharp->MK3_Table.EdgeCoring_DeltaEdge_Ulevel;
		xTmp[20] = pSharp->MK3_Table.EdgeCoring_Clip;
		xTmp[21] = pSharp->MK3_Table.EdgeCoring_Gain_Bound;
		xTmp[22] = pSharp->MK3_Table.EdgeCoring_Ser_Range;
		xTmp[23] = pSharp->MK3_Table.Edge_Delta_Ext_Range;


		for(i=13;i<24;i++)
		{
			rtd_pr_vpq_emerg(" CHIP_EDGE_CORING_MODE_T[%d]=%d \n",i,xTmp[i]);
		}

		/*#3 CHIP_Y_REMAPPING_T 4 +27*/
		xTmp[24] = pSharp->MK3_Table.LinearY_UM;
		xTmp[25] = pSharp->MK3_Table.LinearY_LM;
		xTmp[26] = pSharp->MK3_Table.LinearY_UB;
		xTmp[27] = pSharp->MK3_Table.LinearY_LB;


		for(i=24;i<28;i++)
		{
			rtd_pr_vpq_emerg(" CHIP_Y_REMAPPING_T[%d]=%d \n",i,xTmp[i]);
		}

		/*#4 CHIP_EDGE_HPF_T 18 +45*/
		xTmp[28] = sharp_other_ctrl.edgpk_en;
		xTmp[29] = pSharp->MK3_Table.Edge_Filter_Sel; //must be 0
		xTmp[30] = sharp_other_ctrl.edg_lpf_shift; //must be 8
		xTmp[31] = pSharp->TwoD_edg.C0;
		xTmp[32] = pSharp->TwoD_edg.C1;
		xTmp[33] = pSharp->TwoD_edg.C2;
		xTmp[34] = pSharp->TwoD_edg.C3;
		xTmp[35] = pSharp->TwoD_edg.C4;
		xTmp[36] = pSharp->TwoD_edg.C5;
		xTmp[37] = pSharp->TwoD_edg.C6;
		xTmp[38] = pSharp->TwoD_edg.C7;
		xTmp[39] = pSharp->TwoD_edg.C8;
		xTmp[40] = pSharp->TwoD_edg.C9;
		xTmp[41] = pSharp->TwoD_edg.C10;
		xTmp[42] = pSharp->TwoD_edg.C0H1;
		xTmp[43] = pSharp->TwoD_edg.C0H2;
		xTmp[44] = pSharp->TwoD_edg.C0H3;
		xTmp[45] = pSharp->TwoD_edg.C0H4;


		for(i=28;i<46;i++)
		{
			rtd_pr_vpq_emerg(" CHIP_EDGE_HPF_T[%d]=%d \n",i,xTmp[i]);
		}


		/*#5 CHIP_EDGE_CURVEMAPPING_STEP_T 28 +73*/
		xTmp[46]= pSharp->Edge_Pos_Curve.S1;
		xTmp[47]= pSharp->Edge_Pos_Curve.S2;
		xTmp[48]= pSharp->Edge_Pos_Curve.S3;
		xTmp[49]= pSharp->Edge_Pos_Curve.S4;
		xTmp[50]= pSharp->Edge_Pos_Curve.S5;
		xTmp[51]= pSharp->Edge_Pos_Curve.S6;
		xTmp[52]= pSharp->Edge_Pos_Curve.S7;
		xTmp[53]= pSharp->Edge_Pos_Curve.S8;
		xTmp[54]= pSharp->Edge_Pos_Curve.S9;
		xTmp[55]= pSharp->Edge_Pos_Curve.S10;
		xTmp[56]= pSharp->Edge_Pos_Curve.S11;
		xTmp[57]= pSharp->Edge_Pos_Curve.S12;
		xTmp[58]= pSharp->Edge_Pos_Curve.S13;
		xTmp[59]= pSharp->Edge_Pos_Curve.S14;
		xTmp[60]= pSharp->Edge_Neg_Curve.S1;
		xTmp[61]= pSharp->Edge_Neg_Curve.S2;
		xTmp[62]= pSharp->Edge_Neg_Curve.S3;
		xTmp[63]= pSharp->Edge_Neg_Curve.S4;
		xTmp[64]= pSharp->Edge_Neg_Curve.S5;
		xTmp[65]= pSharp->Edge_Neg_Curve.S6;
		xTmp[66]= pSharp->Edge_Neg_Curve.S7;
		xTmp[67]= pSharp->Edge_Neg_Curve.S8;
		xTmp[68]= pSharp->Edge_Neg_Curve.S9;
		xTmp[69]= pSharp->Edge_Neg_Curve.S10;
		xTmp[70]= pSharp->Edge_Neg_Curve.S11;
		xTmp[71]= pSharp->Edge_Neg_Curve.S12;
		xTmp[72]= pSharp->Edge_Neg_Curve.S13;
		xTmp[73]= pSharp->Edge_Neg_Curve.S14;


		for(i=46;i<74;i++)
		{
		        rtd_pr_vpq_emerg(" CHIP_EDGE_CURVEMAPPING_STEP_T[%d]=%d \n",i,xTmp[i]);
		}

		/*#6 CHIP_EDGE_CURVEMAPPING_UI_T 39 +112*/
		xTmp[74] = sharp_gain.Edge_Level.G_Pos_Gain;
		xTmp[75] = sharp_gain.Edge_Level.G_Neg_Gain;
		xTmp[76] = pSharp->Edge_Shp.G_Pos2;
		xTmp[77] = pSharp->Edge_Shp.G_Neg2;
		xTmp[78] = sharp_gain.Edge_Level.HV_POS_Gain;
		xTmp[79] = sharp_gain.Edge_Level.HV_NEG_Gain;
		xTmp[80] = sharp_gain.Edge_Level.LV_Gain;
		xTmp[81] = pSharp->Edge_Shp.Edg_D2_Shift_Bit;
		xTmp[82] = pSharp->Edge_Neg_Curve.enable;
		xTmp[83] = pSharp->Edge_Pos_Curve.G0;
		xTmp[84] = pSharp->Edge_Pos_Curve.G1;
		xTmp[85] = pSharp->Edge_Pos_Curve.G2;
		xTmp[86] = pSharp->Edge_Pos_Curve.G3;
		xTmp[87] = pSharp->Edge_Pos_Curve.G4;
		xTmp[88] = pSharp->Edge_Pos_Curve.G5;
		xTmp[89] = pSharp->Edge_Pos_Curve.G6;
		xTmp[90] = pSharp->Edge_Pos_Curve.G7;
		xTmp[91] = pSharp->Edge_Pos_Curve.G8;
		xTmp[92] = pSharp->Edge_Pos_Curve.G9;
		xTmp[93] = pSharp->Edge_Pos_Curve.G10;
		xTmp[94] = pSharp->Edge_Pos_Curve.G11;
		xTmp[95] = pSharp->Edge_Pos_Curve.G12;
		xTmp[96] = pSharp->Edge_Pos_Curve.G13;
		xTmp[97] = pSharp->Edge_Pos_Curve.G14;
		xTmp[98] = pSharp->Edge_Neg_Curve.G0;
		xTmp[99] = pSharp->Edge_Neg_Curve.G1;
		xTmp[100] = pSharp->Edge_Neg_Curve.G2;
		xTmp[101] = pSharp->Edge_Neg_Curve.G3;
		xTmp[102] = pSharp->Edge_Neg_Curve.G4;
		xTmp[103] = pSharp->Edge_Neg_Curve.G5;
		xTmp[104] = pSharp->Edge_Neg_Curve.G6;
		xTmp[105] = pSharp->Edge_Neg_Curve.G7;
		xTmp[106] = pSharp->Edge_Neg_Curve.G8;
		xTmp[107] = pSharp->Edge_Neg_Curve.G9;
		xTmp[108] = pSharp->Edge_Neg_Curve.G10;
		xTmp[109] = pSharp->Edge_Neg_Curve.G11;
		xTmp[110] = pSharp->Edge_Neg_Curve.G12;
		xTmp[111] = pSharp->Edge_Neg_Curve.G13;
		xTmp[112] = pSharp->Edge_Neg_Curve.G14;


		for(i=74;i<113;i++)
		{
			rtd_pr_vpq_emerg(" CHIP_EDGE_CURVEMAPPING_UI_T[%d]=%d \n",i,xTmp[i]);
		}

		/*#7 CHIP_TEXTURE_HPF_T 18 +130*/
		xTmp[113] = sharp_other_ctrl.texpk_en;
		xTmp[114] = pSharp->MK3_Table.Tex_Filter_Sel;
		xTmp[115] = sharp_other_ctrl.tex_lpf_shift;
		xTmp[116] = pSharp->TwoD_tex.C0;
		xTmp[117] = pSharp->TwoD_tex.C1;
		xTmp[118] = pSharp->TwoD_tex.C2;
		xTmp[119] = pSharp->TwoD_tex.C3;
		xTmp[120] = pSharp->TwoD_tex.C4;
		xTmp[121] = pSharp->TwoD_tex.C5;
		xTmp[122] = pSharp->TwoD_tex.C6;
		xTmp[123] = pSharp->TwoD_tex.C7;
		xTmp[124] = pSharp->TwoD_tex.C8;
		xTmp[125] = pSharp->TwoD_tex.C9;
		xTmp[126] = pSharp->TwoD_tex.C10;
		xTmp[127] = pSharp->TwoD_tex.C0H1;
		xTmp[128] = pSharp->TwoD_tex.C0H2;
		xTmp[129] = pSharp->TwoD_tex.C0H3;
		xTmp[130] = pSharp->TwoD_tex.C0H4;


		for(i=113;i<131;i++)
		{
			rtd_pr_vpq_emerg(" CHIP_TEXTURE_HPF_T[%d]=%d \n",i,xTmp[i]);
		}

		/*#8 CHIP_TEXTURE_CURVEMAPPING_STEP_T 28 +158*/
		xTmp[131] = pSharp->Tex_Pos_Curve.S1;
		xTmp[132] = pSharp->Tex_Pos_Curve.S2;
		xTmp[133] = pSharp->Tex_Pos_Curve.S3;
		xTmp[134] = pSharp->Tex_Pos_Curve.S4;
		xTmp[135] = pSharp->Tex_Pos_Curve.S5;
		xTmp[136] = pSharp->Tex_Pos_Curve.S6;
		xTmp[137] = pSharp->Tex_Pos_Curve.S7;
		xTmp[138] = pSharp->Tex_Pos_Curve.S8;
		xTmp[139] = pSharp->Tex_Pos_Curve.S9;
		xTmp[140] = pSharp->Tex_Pos_Curve.S10;
		xTmp[141] = pSharp->Tex_Pos_Curve.S11;
		xTmp[142] = pSharp->Tex_Pos_Curve.S12;
		xTmp[143] = pSharp->Tex_Pos_Curve.S13;
		xTmp[144] = pSharp->Tex_Pos_Curve.S14;
		xTmp[145] = pSharp->Tex_Neg_Curve.S1;
		xTmp[146] = pSharp->Tex_Neg_Curve.S2;
		xTmp[147] = pSharp->Tex_Neg_Curve.S3;
		xTmp[148] = pSharp->Tex_Neg_Curve.S4;
		xTmp[149] = pSharp->Tex_Neg_Curve.S5;
		xTmp[150] = pSharp->Tex_Neg_Curve.S6;
		xTmp[151] = pSharp->Tex_Neg_Curve.S7;
		xTmp[152] = pSharp->Tex_Neg_Curve.S8;
		xTmp[153] = pSharp->Tex_Neg_Curve.S9;
		xTmp[154] = pSharp->Tex_Neg_Curve.S10;
		xTmp[155] = pSharp->Tex_Neg_Curve.S11;
		xTmp[156] = pSharp->Tex_Neg_Curve.S12;
		xTmp[157] = pSharp->Tex_Neg_Curve.S13;
		xTmp[158] = pSharp->Tex_Neg_Curve.S14;


		for(i=131;i<159;i++)
		{
			rtd_pr_vpq_emerg(" CHIP_TEXTURE_CURVEMAPPING_STEP_T[%d]=%d \n",i,xTmp[i]);
		}


		/*#9 CHIP_TEXTURE_CURVEMAPPING_UI_T 39 +197*/
		xTmp[159] = sharp_gain.Texture_Level.G_Pos_Gain;
		xTmp[160] = sharp_gain.Texture_Level.G_Neg_Gain;
		xTmp[161] = pSharp->Texture_Shp.G_Pos2;
		xTmp[162] = pSharp->Texture_Shp.G_Neg2;
		xTmp[163] = sharp_gain.Texture_Level.HV_POS_Gain;
		xTmp[164] = sharp_gain.Texture_Level.HV_NEG_Gain;
		xTmp[165] = sharp_gain.Texture_Level.LV_Gain;
		xTmp[166] = pSharp->Texture_Shp.Edg_D2_Shift_Bit;
		xTmp[167] = pSharp->Tex_Neg_Curve.enable;
		xTmp[168] = pSharp->Tex_Pos_Curve.G0;
		xTmp[169] = pSharp->Tex_Pos_Curve.G1;
		xTmp[170] = pSharp->Tex_Pos_Curve.G2;
		xTmp[171] = pSharp->Tex_Pos_Curve.G3;
		xTmp[172] = pSharp->Tex_Pos_Curve.G4;
		xTmp[173] = pSharp->Tex_Pos_Curve.G5;
		xTmp[174] = pSharp->Tex_Pos_Curve.G6;
		xTmp[175] = pSharp->Tex_Pos_Curve.G7;
		xTmp[176] = pSharp->Tex_Pos_Curve.G8;
		xTmp[177] = pSharp->Tex_Pos_Curve.G9;
		xTmp[178] = pSharp->Tex_Pos_Curve.G10;
		xTmp[179] = pSharp->Tex_Pos_Curve.G11;
		xTmp[180] = pSharp->Tex_Pos_Curve.G12;
		xTmp[181] = pSharp->Tex_Pos_Curve.G13;
		xTmp[182] = pSharp->Tex_Pos_Curve.G14;
		xTmp[183] = pSharp->Tex_Neg_Curve.G0;
		xTmp[184] = pSharp->Tex_Neg_Curve.G1;
		xTmp[185] = pSharp->Tex_Neg_Curve.G2;
		xTmp[186] = pSharp->Tex_Neg_Curve.G3;
		xTmp[187] = pSharp->Tex_Neg_Curve.G4;
		xTmp[188] = pSharp->Tex_Neg_Curve.G5;
		xTmp[189] = pSharp->Tex_Neg_Curve.G6;
		xTmp[190] = pSharp->Tex_Neg_Curve.G7;
		xTmp[191] = pSharp->Tex_Neg_Curve.G8;
		xTmp[192] = pSharp->Tex_Neg_Curve.G9;
		xTmp[193] = pSharp->Tex_Neg_Curve.G10;
		xTmp[194] = pSharp->Tex_Neg_Curve.G11;
		xTmp[195] = pSharp->Tex_Neg_Curve.G12;
		xTmp[196] = pSharp->Tex_Neg_Curve.G13;
		xTmp[197] = pSharp->Tex_Neg_Curve.G14;


		for(i=159;i<198;i++)
		{
			rtd_pr_vpq_emerg(" CHIP_TEXTURE_CURVEMAPPING_UI_T[%d]=%d \n",i,xTmp[i]);
		}


		/*#10 CHIP_COMMON_HPF_T 38 +235*/
		xTmp[198] = pSharp->SR_H9.C0;
		xTmp[199] = pSharp->SR_H9.C1;
		xTmp[200] = pSharp->SR_H9.C2;
		xTmp[201] = pSharp->SR_H9.C3;
		xTmp[202] = pSharp->SR_H9.C4;
		xTmp[203] = pSharp->SR_H13.C0;
		xTmp[204] = pSharp->SR_H13.C1;
		xTmp[205] = pSharp->SR_H13.C2;
		xTmp[206] = pSharp->SR_H13.C3;
		xTmp[207] = pSharp->SR_H13.C4;
		xTmp[208] = pSharp->SR_H13.C5;
		xTmp[209] = pSharp->SR_H13.C6;
		xTmp[210] = pSharp->SR_H21.C0;
		xTmp[211] = pSharp->SR_H21.C1;
		xTmp[212] = pSharp->SR_H21.C2;
		xTmp[213] = pSharp->SR_H21.C3;
		xTmp[214] = pSharp->SR_H21.C4;
		xTmp[215] = pSharp->SR_H21.C5;
		xTmp[216] = pSharp->SR_H21.C6;
		xTmp[217] = pSharp->SR_H21.C7;
		xTmp[218] = pSharp->SR_H21.C8;
		xTmp[219] = pSharp->SR_H21.C9;
		xTmp[220] = pSharp->SR_H21.C10;
		xTmp[221] = pSharp->SR_V9_1.C0;
		xTmp[222] = pSharp->SR_V9_1.C1;
		xTmp[223] = pSharp->SR_V9_1.C2;
		xTmp[224] = pSharp->SR_V9_1.C3;
		xTmp[225] = pSharp->SR_V9_1.C4;
		xTmp[226] = pSharp->SR_V9_2.C0;
		xTmp[227] = pSharp->SR_V9_2.C1;
		xTmp[228] = pSharp->SR_V9_2.C2;
		xTmp[229] = pSharp->SR_V9_2.C3;
		xTmp[230] = pSharp->SR_V9_2.C4;
		xTmp[231] = pSharp->SR_V9_3.C0;
		xTmp[232] = pSharp->SR_V9_3.C1;
		xTmp[233] = pSharp->SR_V9_3.C2;
		xTmp[234] = pSharp->SR_V9_3.C3;
		xTmp[235] = pSharp->SR_V9_3.C4;

		for(i=198;i<236;i++)
		{
			rtd_pr_vpq_emerg(" CHIP_COMMON_HPF_T[%d]=%d \n",i,xTmp[i]);
		}


		/*#11 CHIP_SLOPE_GAIN_MASK_T 11 +246*/;

		xTmp[236] = pSharp->MK3_Table.Slope_gain_en  ;
		xTmp[237] = pSharp->MK3_Table.Slope_gain_en;
		xTmp[238] = pSharp->MK3_Table.flat_method ;
		xTmp[239] = pSharp->MK3_Table.Gain_boundPos;
		xTmp[240] = pSharp->MK3_Table.Gain_boundNeg;
		xTmp[241] = pSharp->MK3_Table.Gain_extendPos ;
		xTmp[242] = pSharp->MK3_Table.Gain_extendNeg ;
		xTmp[243] = pSharp->MK3_Table.flat_enable;
		xTmp[244] = pSharp->MK3_Table.flat_offset;
		xTmp[245] = pSharp->MK3_Table.flat_gain ;
		xTmp[246] = pSharp->MK3_Table.flat_range_h;

		for(i=236;i<247;i++)
		{
			rtd_pr_vpq_emerg(" CHIP_SLOPE_GAIN_MASK_T[%d]=%d \n",i,xTmp[i]);
		}


		/*#12 CHIP_VERTICAL_HPF_T 6 +252*/
		xTmp[247] = sharp_other_ctrl.vpk_en;
		xTmp[248] = pSharp->Vertical.C0;
		xTmp[249] = pSharp->Vertical.C1;
		xTmp[250] = pSharp->Vertical.C2;
		xTmp[251] = pSharp->Vertical.C3;
		xTmp[252] = pSharp->Vertical.C4;


		for(i=247;i<253;i++)
		{
			rtd_pr_vpq_emerg(" CHIP_VERTICAL_HPF_T[%d]=%d \n",i,xTmp[i]);
		}


		/*#13 CHIP_VERTICAL_CURVEMAPPING_STEP_T 28 +280*/

		xTmp[253] = pSharp->V_Pos_Curve.S1;
		xTmp[254] = pSharp->V_Pos_Curve.S2;
		xTmp[255] = pSharp->V_Pos_Curve.S3;
		xTmp[256] = pSharp->V_Pos_Curve.S4;
		xTmp[257] = pSharp->V_Pos_Curve.S5;
		xTmp[258] = pSharp->V_Pos_Curve.S6;
		xTmp[259] = pSharp->V_Pos_Curve.S7;
		xTmp[260] = pSharp->V_Pos_Curve.S8;
		xTmp[261] = pSharp->V_Pos_Curve.S9;
		xTmp[262] = pSharp->V_Pos_Curve.S10;
		xTmp[263] = pSharp->V_Pos_Curve.S11;
		xTmp[264] = pSharp->V_Pos_Curve.S12;
		xTmp[265] = pSharp->V_Pos_Curve.S13;
		xTmp[266] = pSharp->V_Pos_Curve.S14;
		xTmp[267] = pSharp->V_Neg_Curve.S1;
		xTmp[268] = pSharp->V_Neg_Curve.S2;
		xTmp[269] = pSharp->V_Neg_Curve.S3;
		xTmp[270] = pSharp->V_Neg_Curve.S4;
		xTmp[271] = pSharp->V_Neg_Curve.S5;
		xTmp[272] = pSharp->V_Neg_Curve.S6;
		xTmp[273] = pSharp->V_Neg_Curve.S7;
		xTmp[274] = pSharp->V_Neg_Curve.S8;
		xTmp[275] = pSharp->V_Neg_Curve.S9;
		xTmp[276] = pSharp->V_Neg_Curve.S10;
		xTmp[277] = pSharp->V_Neg_Curve.S11;
		xTmp[278] = pSharp->V_Neg_Curve.S12;
		xTmp[279] = pSharp->V_Neg_Curve.S13;
		xTmp[280] = pSharp->V_Neg_Curve.S14;



		for(i=253;i<281;i++)
		{
			rtd_pr_vpq_emerg(" CHIP_VERTICAL_CURVEMAPPING_STEP_T[%d]=%d \n",i,xTmp[i]);
		}


		/*#14 CHIP_VERTICAL_CURVEMAPPING_UI_T 48 +328*/


		xTmp[281] = sharp_gain.Vertical_Level.G_Pos_Gain;
		xTmp[282] = sharp_gain.Vertical_Level.G_Neg_Gain;
		xTmp[283] = sharp_gain.Vertical_Level.LV_Gain;
		xTmp[284] = pSharp->Vertical.G_Pos2;
		xTmp[285] = pSharp->Vertical.G_Neg2;
		xTmp[286] = sharp_gain.Vertical_Level.HV_POS_Gain;
		xTmp[287] = sharp_gain.Vertical_Level.HV_NEG_Gain;
		xTmp[288] = 1;// K5Lp fixed 1
		xTmp[289] = sharp_gain.Vertical_Edge_Level.G_Pos_Gain;
		xTmp[290] = sharp_gain.Vertical_Edge_Level.G_Neg_Gain;
		xTmp[291]= sharp_gain.Vertical_Edge_Level.LV_Gain;
		xTmp[292]= pSharp->Vertical.G_Pos2_edge;
		xTmp[293]= pSharp->Vertical.G_Neg2_edge;
		xTmp[294]= sharp_gain.Vertical_Edge_Level.HV_POS_Gain;
		xTmp[295]= sharp_gain.Vertical_Edge_Level.HV_NEG_Gain;
		xTmp[296]= pSharp->Vertical.V_Tex_en;
		xTmp[297]= pSharp->Vertical.V_Dering_En;
		xTmp[298]= pSharp->V_Neg_Curve.enable;
		xTmp[299]= pSharp->V_Pos_Curve.G0;
		xTmp[300]= pSharp->V_Pos_Curve.G1;
		xTmp[301]= pSharp->V_Pos_Curve.G2;
		xTmp[302]= pSharp->V_Pos_Curve.G3;
		xTmp[303]= pSharp->V_Pos_Curve.G4;
		xTmp[304]= pSharp->V_Pos_Curve.G5;
		xTmp[305]= pSharp->V_Pos_Curve.G6;
		xTmp[306]= pSharp->V_Pos_Curve.G7;
		xTmp[307]= pSharp->V_Pos_Curve.G8;
		xTmp[308]= pSharp->V_Pos_Curve.G9;
		xTmp[309]= pSharp->V_Pos_Curve.G10;
		xTmp[310]= pSharp->V_Pos_Curve.G11;
		xTmp[311]= pSharp->V_Pos_Curve.G12;
		xTmp[312]= pSharp->V_Pos_Curve.G13;
		xTmp[313]= pSharp->V_Pos_Curve.G14;
		xTmp[314]= pSharp->V_Neg_Curve.G0;
		xTmp[315]= pSharp->V_Neg_Curve.G1;
		xTmp[316]= pSharp->V_Neg_Curve.G2;
		xTmp[317]= pSharp->V_Neg_Curve.G3;
		xTmp[318]= pSharp->V_Neg_Curve.G4;
		xTmp[319]= pSharp->V_Neg_Curve.G5;
		xTmp[320]= pSharp->V_Neg_Curve.G6;
		xTmp[321]= pSharp->V_Neg_Curve.G7;
		xTmp[322]= pSharp->V_Neg_Curve.G8;
		xTmp[323]= pSharp->V_Neg_Curve.G9;
		xTmp[324]= pSharp->V_Neg_Curve.G10;
		xTmp[325]= pSharp->V_Neg_Curve.G11;
		xTmp[326]= pSharp->V_Neg_Curve.G12;
		xTmp[327]= pSharp->V_Neg_Curve.G13;
		xTmp[328]= pSharp->V_Neg_Curve.G14;


		for(i=281;i<329;i++)
		{
			rtd_pr_vpq_emerg(" CHIP_VERTICAL_CURVEMAPPING_UI_T[%d]=%d \n",i,xTmp[i]);
		}


		/*#15 CHIP_VERTICAL_SLOPE_GAIN_MASK_T 10 +338*/
		xTmp[329]= pSharp->MK3_Table.Slope_gain_v_en;
		xTmp[330]= pSharp->MK3_Table.dir_v ;
		xTmp[331]= pSharp->MK3_Table.Gain_boundPos_v;
		xTmp[332]= pSharp->MK3_Table.Gain_boundNeg_v;
		xTmp[333]= pSharp->MK3_Table.Gain_extendPos_v;
		xTmp[334]= pSharp->MK3_Table.Gain_extendNeg_v;
		xTmp[335]= pSharp->MK3_Table.flat_v_enable;
		xTmp[336]= pSharp->MK3_Table.flat_offset_v;
		xTmp[337]= pSharp->MK3_Table.flat_gain_v;
		xTmp[338]= pSharp->MK3_Table.flat_range_v;


		for(i=329;i<339;i++)
		{
			rtd_pr_vpq_emerg(" CHIP_VERTICAL_SLOPE_GAIN_MASK_T[%d]=%d \n",i,xTmp[i]);
		}

		/*#16 CHIP_MKIII_T 61 +399*/
		xTmp[339]= pSharp->MK3_Table.Gain_By_Y_En;
		xTmp[340]= pSharp->MK3_Table.gain_by_y_v_en;
		xTmp[341]= pSharp->MK3_Table.gain_by_y_tex_en;
		xTmp[342]= pSharp->MK3_Table.gain_by_y_edg_en;
		xTmp[343]= pSharp->MK3_Table.Lpf_Weigh_En;
		xTmp[344]= pSharp->MK3_Table.lpf_weigh_edg_en;
		xTmp[345]= pSharp->MK3_Table.lpf_weigh_tex_en;
		xTmp[346]= pSharp->MK3_Table.MaxMin_Gain_En;
		xTmp[347]= pSharp->MK3_Table.max_min_gain_v_en ;
		xTmp[348]= pSharp->MK3_Table.max_min_gain_tex_en;
		xTmp[349]= pSharp->MK3_Table.max_min_gain_edg_en;
		xTmp[350]= pSharp->MK3_Table.lv_by_y_enable;
		xTmp[351]= pSharp->MK3_Table.lv_by_y_edg_enable;
		xTmp[352]= pSharp->MK3_Table.lv_by_y_v_edg_enable;
		xTmp[353]= pSharp->MK3_Table.lv_by_y_tex_enable;
		xTmp[354]= pSharp->MK3_Table.lv_by_y_v_enable;
		xTmp[355]= pSharp->MK3_Table.Lpf2d_C0 ;
		xTmp[356]= pSharp->MK3_Table.Lpf2d_C1;
		xTmp[357]= pSharp->MK3_Table.Lpf2d_C2;
		xTmp[358]= pSharp->MK3_Table.Lpf2d_C3;
		xTmp[359]= pSharp->MK3_Table.Lpf2d_C4;
		xTmp[360]= pSharp->MK3_Table.Lpf2d_C5;
		xTmp[361]= pSharp->MK3_Table.MaxMin_Step_1;
		xTmp[362]= pSharp->MK3_Table.MaxMin_Step_2;
		xTmp[363]= pSharp->MK3_Table.MaxMin_Step_3;
		xTmp[364]= pSharp->MK3_Table.MaxMin_Step_4;
		xTmp[365]= pSharp->MK3_Table.MaxMin_Step_5;
		xTmp[366]= pSharp->MK3_Table.MaxMin_Step_6;
		xTmp[367]= pSharp->MK3_Table.MaxMin_Gain_0;
		xTmp[368]= pSharp->MK3_Table.MaxMin_Gain_1;
		xTmp[369]= pSharp->MK3_Table.MaxMin_Gain_2;
		xTmp[370]= pSharp->MK3_Table.MaxMin_Gain_3;
		xTmp[371]= pSharp->MK3_Table.MaxMin_Gain_4;
		xTmp[372]= pSharp->MK3_Table.MaxMin_Gain_5;
		xTmp[373]= pSharp->MK3_Table.MaxMin_Gain_6;
		xTmp[374]= pSharp->MK3_Table.Lpf_Step_1;
		xTmp[375]= pSharp->MK3_Table.Lpf_Step_2;
		xTmp[376]= pSharp->MK3_Table.Lpf_Step_3;
		xTmp[377]= pSharp->MK3_Table.Lpf_Step_4;
		xTmp[378]= pSharp->MK3_Table.Lpf_Step_5;
		xTmp[379]= pSharp->MK3_Table.Lpf_Step_6;
		xTmp[380]= pSharp->MK3_Table.Lpf_Weit_0;
		xTmp[381]= pSharp->MK3_Table.Lpf_Weit_1;
		xTmp[382]= pSharp->MK3_Table.Lpf_Weit_2;
		xTmp[383]= pSharp->MK3_Table.Lpf_Weit_3;
		xTmp[384]= pSharp->MK3_Table.Lpf_Weit_4;
		xTmp[385]= pSharp->MK3_Table.Lpf_Weit_5;
		xTmp[386]= pSharp->MK3_Table.Lpf_Weit_6;
		xTmp[387]= pSharp->MK3_Table.Gain_By_Y_Step_1;
		xTmp[388]= pSharp->MK3_Table.Gain_By_Y_Step_2;
		xTmp[389]= pSharp->MK3_Table.Gain_By_Y_Step_3;
		xTmp[390]= pSharp->MK3_Table.Gain_By_Y_Step_4;
		xTmp[391]= pSharp->MK3_Table.Gain_By_Y_Step_5;
		xTmp[392]= pSharp->MK3_Table.Gain_By_Y_Step_6;
		xTmp[393]= pSharp->MK3_Table.Gain_By_Y_Gain_0;
		xTmp[394]= pSharp->MK3_Table.Gain_By_Y_Gain_1;
		xTmp[395]= pSharp->MK3_Table.Gain_By_Y_Gain_2;
		xTmp[396]= pSharp->MK3_Table.Gain_By_Y_Gain_3;
		xTmp[397]= pSharp->MK3_Table.Gain_By_Y_Gain_4;
		xTmp[398]= pSharp->MK3_Table.Gain_By_Y_Gain_5;
		xTmp[399]= pSharp->MK3_Table.Gain_By_Y_Gain_6;

		for(i=339;i<400;i++)
		{
			rtd_pr_vpq_emerg(" CHIP_MKIII_T[%d]=%d \n",i,xTmp[i]);
		}
	}

	{
		DRV_VipNewIDcti_Table i_dcti;
		DRV_VipNewDDcti_Table d_dcti;
		VIP_D_vcti_t v_cti;

		drvif_color_get_inewdcti_table(&i_dcti);
		drvif_color_get_dnewdcti_table(&d_dcti);	
		drvif_color_scaler_getds_vcti(0, &v_cti);
		

		/*#17 CHIP_CTI_T 18 +417 */
		xTmp[400] = i_dcti.VipNewIDcti_ctl1.dcti_en;
		xTmp[401] = i_dcti.VipNewIDcti_ctl2.uvsync_en;
		xTmp[402] = 0;// k5lp rmove it i_dcti.VipNewIDcti_ctl1.uvalign_en;
		xTmp[403] = i_dcti.VipNewIDcti_ctl1.dcti_mode;
		xTmp[404] = i_dcti.VipNewIDcti_ctl1.uvgain;
		xTmp[405] = i_dcti.VipNewIDcti_ctl1.maxlen;
		xTmp[406] = i_dcti.VipNewIDcti_ctl1.psmth;
		xTmp[407] = d_dcti.VipNewDDcti_ctl1.dcti_en;
		xTmp[408] = d_dcti.VipNewDDcti_ctl2.uvsync_en;
		xTmp[409] = d_dcti.VipNewDDcti_ctl1.uvalign_en;
		xTmp[410] = d_dcti.VipNewDDcti_ctl1.dcti_mode;
		xTmp[411] = d_dcti.VipNewDDcti_ctl1.uvgain ;
		xTmp[412] = d_dcti.VipNewDDcti_ctl1.maxlen ;
		xTmp[413] = d_dcti.VipNewDDcti_ctl1.psmth;
		xTmp[414] = v_cti.vcti_en;
		xTmp[415] = v_cti.vcti_select;
		xTmp[416] = v_cti.vcti_gain;
		xTmp[417] = v_cti.vcti_type3_thd;

		for(i=400;i<418;i++)
		{
			rtd_pr_vpq_emerg(" CHIP_CTI_T[%d]=%d \n",i,xTmp[i]);
		}
	}
	
	{
		VIP_D_DLTI_Table drv_shp_Dlti;
		drvif_color_Get_D_DLTI_Table(&drv_shp_Dlti);

		/*#18 CHIP_HLTI_T 36 +453*/
		xTmp[418] = drv_shp_Dlti.DLTI_H_1.dlti_h_en;
		xTmp[419] = drv_shp_Dlti.DLTI_H_1.Can_shift;
		xTmp[420] = drv_shp_Dlti.DLTI_H_1.maxlen;
		xTmp[421] = drv_shp_Dlti.DLTI_H_1.pnth;
		xTmp[422] = drv_shp_Dlti.DLTI_H_2.ffd2_en;
		xTmp[423] = drv_shp_Dlti.DLTI_H_1.Data_sel;
		xTmp[424] = 0;// drv_shp_Dlti.DLTI_H_1.ovc_extend;
		xTmp[425] = drv_shp_Dlti.DLTI_H_1.ffd0_start;
		xTmp[426] = drv_shp_Dlti.DLTI_H_2.D1_CP_shift;
		xTmp[427] = drv_shp_Dlti.DLTI_H_2.DCP_th;
		xTmp[428] = drv_shp_Dlti.DLTI_H_2.flat_s_range;
		xTmp[429] = drv_shp_Dlti.DLTI_H_2.flat_score_thl;
		xTmp[430] = 0;// drv_shp_Dlti.DLTI_H_1.Ffd1_th;
		xTmp[431] = drv_shp_Dlti.DLTI_H_1.Ffd1_ratio;
		xTmp[432] = 0;// drv_shp_Dlti.DLTI_H_2.Ffd2_range2R;
		xTmp[433] = 0;// drv_shp_Dlti.DLTI_H_2.Ffd2_range2L;
		xTmp[434] = 0;// drv_shp_Dlti.DLTI_H_2.Ffd2_ratio ;
		xTmp[435] = drv_shp_Dlti.DLTI_H_OverEnhance.gain_pos;
		xTmp[436] = drv_shp_Dlti.DLTI_H_OverEnhance.offset_pos ;
		xTmp[437] = drv_shp_Dlti.DLTI_H_OverEnhance.gain_neg;
		xTmp[438] = drv_shp_Dlti.DLTI_H_OverEnhance.offset_neg;
		xTmp[439] = drv_shp_Dlti.DLTI_H_Tnoff.Tnoff0;
		xTmp[440] = 0;// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff1;
		xTmp[441] = 0;// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff2;
		xTmp[442] = 0;// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff3;
		xTmp[443] = 0;// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff4;
		xTmp[444] = 0;// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff5;
		xTmp[445] = 0;// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff6;
		xTmp[446] = 0;// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff7;
		xTmp[447] = 0;// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff8;
		xTmp[448] = 0;// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff9;
		xTmp[449] = 0;// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff10;
		xTmp[450] = 0;// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff11;
		xTmp[451] = 0;// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff12;
		xTmp[452] = 0;// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff13;
		xTmp[453] = 0;// drv_shp_Dlti.DLTI_H_Tnoff.Tnoff14;


		for(i=418;i<454;i++)
		{
			rtd_pr_vpq_emerg(" CHIP_HLTI_T[%d]=%d \n",i,xTmp[i]);
		}

		/*==========set from DB V LTI=============*/
		/*#19 CHIP_VLTI_T 19 +472*/
		xTmp[454] = drv_shp_Dlti.DLTI_V_1.dlti_v_en;
		xTmp[455] = drv_shp_Dlti.DLTI_V_1.can_shift_v;
		xTmp[456] = drv_shp_Dlti.DLTI_V_1.maxlen_v;
		xTmp[457] = drv_shp_Dlti.DLTI_V_1.pnth_v;
		xTmp[458] = 0;// drv_shp_Dlti.DLTI_V_1.ovc_v_extend;
		xTmp[459] = drv_shp_Dlti.DLTI_V_1.v_ffd0_start;
		xTmp[460] = 0;// drv_shp_Dlti.DLTI_V_1.ffd1_th ;
		xTmp[461] = drv_shp_Dlti.DLTI_V_1.ffd1_ratio;
		xTmp[462] = drv_shp_Dlti.DLTI_V_OverEnhance.gain_pos ;
		xTmp[463] = drv_shp_Dlti.DLTI_V_OverEnhance.offset_pos;
		xTmp[464] = drv_shp_Dlti.DLTI_V_OverEnhance.gain_neg;
		xTmp[465] = drv_shp_Dlti.DLTI_V_OverEnhance.offset_neg;
		xTmp[466] = drv_shp_Dlti.DLTI_V_Tnoff.Tnoff0;
		xTmp[467] = 0;// drv_shp_Dlti.DLTI_V_Tnoff.Tnoff1;
		xTmp[468] = 0;// drv_shp_Dlti.DLTI_V_Tnoff.Tnoff2;
		xTmp[469] = 0;// drv_shp_Dlti.DLTI_V_Tnoff.Tnoff3;
		xTmp[470] = 0;// drv_shp_Dlti.DLTI_V_Tnoff.Tnoff4;
		xTmp[471] = 0;// drv_shp_Dlti.DLTI_V_Tnoff.Tnoff5;
		xTmp[472] = 0;// drv_shp_Dlti.DLTI_V_Tnoff.Tnoff6;
	}

	{
		VIP_MBPK_Table MBPK;
		
		drvif_color_Get_MBPK_Table(&MBPK);
		
		for(i=454;i<473;i++)
		{
		        rtd_pr_vpq_emerg(" CHIP_VLTI_T[%d]=%d \n",i,xTmp[i]);
		}

		/*#20 CHIP_MULTIBAND_PEAKING_HPF_T 8 +480*/
		xTmp[473] = MBPK.MBPK_Ctrl.mb_peaking_en;
		xTmp[474] = MBPK.MBPK_H_Coef.MB_Hor_Filter_C0;
		xTmp[475] = MBPK.MBPK_H_Coef.MB_Hor_Filter_C1;
		xTmp[476] = MBPK.MBPK_H_Coef.MB_Hor_Filter_C2;
		xTmp[477] = MBPK.MBPK_H_Coef.MB_Hor_Filter_C3;
		xTmp[478] = MBPK.MBPK_V_Coef.MB_Ver_Filter_C0;
		xTmp[479] = MBPK.MBPK_V_Coef.MB_Ver_Filter_C1;
		xTmp[480] = MBPK.MBPK_V_Coef.MB_Ver_Filter_C2;


		for(i=473;i<481;i++)
		{
			rtd_pr_vpq_emerg(" CHIP_MULTIBAND_PEAKING_HPF_T[%d]=%d \n",i,xTmp[i]);
		}



		/*#21 CHIP_MULTIBAND_PEAKING_UI_T 18 +498*/

		xTmp[481]= MBPK.MBPK_Ctrl.MB_H_En;
		xTmp[482]= MBPK.MBPK_H_Table.MB_Gain_Pos;
		xTmp[483]= MBPK.MBPK_H_Table.MB_Gain_Neg;
		xTmp[484]= MBPK.MBPK_H_Table.MB_LV;
		xTmp[485]= MBPK.MBPK_H_Table.MB_Gain_Pos2;
		xTmp[486]= MBPK.MBPK_H_Table.MB_Gain_Neg2;
		xTmp[487]= MBPK.MBPK_H_Table.MB_HV_Neg;
		xTmp[488]= MBPK.MBPK_H_Table.MB_HV_Pos;
		xTmp[489]= MBPK.MBPK_H_Table.MB_D2_shift_bit;
		xTmp[490]= MBPK.MBPK_Ctrl.MB_V_En;
		xTmp[491]= MBPK.MBPK_V_Table.MB_Gain_Pos_V;
		xTmp[492]= MBPK.MBPK_V_Table.MB_Gain_Neg_V;
		xTmp[493]= MBPK.MBPK_V_Table.MB_LV_V;
		xTmp[494]= MBPK.MBPK_V_Table.MB_Gain_Pos2_V;
		xTmp[495]= MBPK.MBPK_V_Table.MB_Gain_Neg2_V;
		xTmp[496]= MBPK.MBPK_V_Table.MB_HV_Pos_V;
		xTmp[497]= MBPK.MBPK_V_Table.MB_HV_Neg_V;
		xTmp[498]= MBPK.MBPK_V_Table.MB_D2_shift_bit_V;



		for(i=481;i<499;i++)
		{
			rtd_pr_vpq_emerg(" CHIP_MULTIBAND_PEAKING_UI_T[%d]=%d \n",i,xTmp[i]);
		}
	}


	/*#22 CHIP_ISE_T 8 +506*/
	xTmp[499] = pSharp->SEGPK_ISE.ise_en ;
	xTmp[500] = pSharp->SEGPK_ISE.ise_step;
	xTmp[501] = pSharp->SEGPK_ISE.ise_scale;
	xTmp[502] = pSharp->SEGPK_ISE.ise_ub;
	xTmp[503] = pSharp->SEGPK_ISE.ise_h_st;
	xTmp[504] = pSharp->SEGPK_ISE.ise_h_end;
	xTmp[505] = pSharp->SEGPK_ISE.ise_v_st;
	xTmp[506] = pSharp->SEGPK_ISE.ise_v_end;


	for(i=499;i<507;i++)
	{
		rtd_pr_vpq_emerg(" CHIP_ISE_T[%d]=%d \n",i,xTmp[i]);
	}

	/*#23 CHIP_FLAT_FILTER_T 7 +513*/
	xTmp[507] = pSharp->Texture_Flat_Shp.en_tex;
	xTmp[508] = pSharp->Texture_Flat_Shp.C0H1;
	xTmp[509] = pSharp->Texture_Flat_Shp.C0H2;
	xTmp[510] = pSharp->Texture_Flat_Shp.C0;
	xTmp[511] = pSharp->Texture_Flat_Shp.C1;
	xTmp[512] = pSharp->Texture_Flat_Shp.C2;
	xTmp[513] = pSharp->Texture_Flat_Shp.C3;

	for(i=507;i<514;i++)
	{
		rtd_pr_vpq_emerg(" CHIP_FLAT_FILTER_T[%d]=%d \n",i,xTmp[i]);
	}

	/*#24 CHIP_EDGE_EMF_UI_T 14 +527*/
	xTmp[514] = pSharp->Edge_EMF_Table.EMF_En;
	xTmp[515] = pSharp->Edge_EMF_Table.EMF_Range;
	xTmp[516] = pSharp->Edge_EMF_Table.EMF_Shift;
	xTmp[517] = pSharp->Edge_EMF_Table.Blend_wt;
	xTmp[518] = pSharp->Edge_EMF_Table.emf_fixextent_neg;
	xTmp[519] = pSharp->Edge_EMF_Table.emf_fixextent_pos;
	xTmp[520] = pSharp->Edge_EMF_Table.Seg_0_x;
	xTmp[521] = pSharp->Edge_EMF_Table.Seg_1_x;
	xTmp[522] = pSharp->Edge_EMF_Table.Seg_0_Gain;
	xTmp[523] = pSharp->Edge_EMF_Table.Seg_1_Gain;
	xTmp[524] = pSharp->Edge_EMF_Table.Seg_2_Gain;
	xTmp[525] = pSharp->Edge_EMF_Table.Seg_0_Offset;
	xTmp[526] = pSharp->Edge_EMF_Table.v5;
	xTmp[527] = pSharp->Edge_EMF_Table.h11;

	for(i=514;i<528;i++)
	{
		rtd_pr_vpq_emerg(" CHIP_EDGE_EMF_UI_T[%d]=%d \n",i,xTmp[i]);
	}



	/*#24 CHIP_TEXTURE_EMF_UI_T 15 +542 */
	xTmp[528] = pSharp->Text_EMF_Table.EMF_En;
	xTmp[529] = pSharp->Text_EMF_Table.EMF_Range;
	xTmp[530] = pSharp->Text_EMF_Table.EMF_Shift;
	xTmp[531] = pSharp->Text_EMF_Table.Blend_wt;
	xTmp[532] = pSharp->Text_EMF_Table.emf_fixextent_neg;
	xTmp[533] = pSharp->Text_EMF_Table.emf_fixextent_pos;
	xTmp[534] = pSharp->Text_EMF_Table.Seg_0_x;
	xTmp[535] = pSharp->Text_EMF_Table.Seg_1_x;
	xTmp[536] = pSharp->Text_EMF_Table.Seg_0_Gain;
	xTmp[537] = pSharp->Text_EMF_Table.Seg_1_Gain;
	xTmp[538] = pSharp->Text_EMF_Table.Seg_2_Gain;
	xTmp[539] = pSharp->Text_EMF_Table.Seg_0_Offset;
	xTmp[540] = pSharp->Text_EMF_Table.EMF_texgain;
	xTmp[541] = pSharp->Text_EMF_Table.v5;
	xTmp[542] = pSharp->Text_EMF_Table.h11;

	for(i=529;i<543;i++)
	{
		rtd_pr_vpq_emerg(" CHIP_TEXTURE_EMF_UI_T[%d]=%d \n",i,xTmp[i]);
	}


	/*#25 CHIP_VERTICAL_EMF_UI_T 2 +544*/
	xTmp[543] = pSharp->Vertical.Vemf_En;
	xTmp[544] = pSharp->Vertical.Vext_Reg;

	for(i=0;i<2;i++)
	{
		rtd_pr_vpq_emerg(" CHIP_VERTICAL_EMF_UI_T[%d]=%d \n",i,xTmp[i]);
	}


	/* CHIP_SR_UI_T 16 +560 */

	xTmp[545]  = pSharp->SR_Ring_H11.C0;
	xTmp[546]  = pSharp->SR_Ring_H11.C1;
	xTmp[547]  = pSharp->SR_Ring_H11.C2;
	xTmp[548]  = pSharp->SR_Ring_H11.C3;
	xTmp[549]  = pSharp->SR_Ring_H11.C4;
	xTmp[550]  = pSharp->SR_Ring_H11.C5;
	xTmp[551]  = pSharp->SR_Ring_V9.C0;
	xTmp[552]  = pSharp->SR_Ring_V9.C1;
	xTmp[553]  = pSharp->SR_Ring_V9.C2;
	xTmp[554]  = pSharp->SR_Ring_V9.C3;
	xTmp[555] = pSharp->SR_Ring_V9.C4;
	xTmp[556] = pSharp->SR_Control.Ring_Gen_Gain;
	xTmp[557] = pSharp->SR_Control.Period;
	xTmp[558] = pSharp->SR_Control.Gain;
	xTmp[559] = pSharp->SR_Control.Offset;
	xTmp[560] = 0;


	for(i=545;i<561;i++)
	{
		rtd_pr_vpq_emerg(" CHIP_SR_UI_T[%d]=%d \n",i,xTmp[i]);
	}

	/* ptCHIP_SR_UI_T->ring_h_c0
	ptCHIP_SR_UI_T->ring_h_c1
	ptCHIP_SR_UI_T->ring_h_c2
	ptCHIP_SR_UI_T->ring_h_c3
	ptCHIP_SR_UI_T->ring_h_c4
	ptCHIP_SR_UI_T->ring_h_c5

	ptCHIP_SR_UI_T->ring_v_c0
	ptCHIP_SR_UI_T->ring_v_c1
	ptCHIP_SR_UI_T->ring_v_c2
	ptCHIP_SR_UI_T->ring_v_c3
	ptCHIP_SR_UI_T->ring_v_c4

	ptCHIP_SR_UI_T->Ring_Gen_Gain
	ptCHIP_SR_UI_T->Cur_Period
	ptCHIP_SR_UI_T->Cur_Gain
	ptCHIP_SR_UI_T->Cur_Offset
	*/

	/* Edge Enhance Part */
	dvr_free(pSharp);
}

void fwif_color_set_sharpness_table_init_TV006(unsigned char src_idx, unsigned char value)
{
	static DRV_Sharpness_Table sharp;
	_system_setting_info *VIP_system_info_structure_table=(_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	if (value >= Sharp_table_num)
		value = 0;

	/* get para from register */
	drvif_color_Get_Sharpness_Table(&sharp);

	/* Get from VIP Table */
#ifndef BUILD_QUICK_SHOW	
	if (drvif_scaelr3d_decide_is_3D_display_mode() == 1)
		sharp.TwoD_Peak_AdaptCtrl.Peaking_Enable = 0;
	else
#endif		
		sharp.TwoD_Peak_AdaptCtrl.Peaking_Enable = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.Peaking_Enable;

	//sharp.TwoD_Peak_AdaptCtrl.Dering_En = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.Dering_En;/*merlin2 removed*/

	sharp.TwoD_Peak_AdaptCtrl.PkDlti_comsign = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.PkDlti_comsign;
	sharp.TwoD_Peak_AdaptCtrl.edg_dlti = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.edg_dlti;
	sharp.TwoD_Peak_AdaptCtrl.edg_dlti_tex = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.edg_dlti_tex;
	sharp.TwoD_Peak_AdaptCtrl.zdiffl_en = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.zdiffl_en;
	sharp.TwoD_Peak_AdaptCtrl.zdiffl_lowbnd = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.zdiffl_lowbnd;
	sharp.TwoD_Peak_AdaptCtrl.zdiffl_stepbit = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.zdiffl_stepbit;
	sharp.TwoD_Peak_AdaptCtrl.edge_mono_en = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.edge_mono_en;
	sharp.TwoD_Peak_AdaptCtrl.edge_mono_stepbit = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.edge_mono_stepbit;
	sharp.TwoD_Peak_AdaptCtrl.edge_mono_stepbiu = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.edge_mono_stepbiu;
	sharp.TwoD_Peak_AdaptCtrl.edge_mono_upbnd = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.edge_mono_upbnd;
	sharp.TwoD_Peak_AdaptCtrl.edge_mono_lowbnd = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.edge_mono_lowbnd;
	sharp.TwoD_Peak_AdaptCtrl.Hpf_Vpf_Maxmin_En = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.Hpf_Vpf_Maxmin_En;/*magellan2*/
	sharp.TwoD_Peak_AdaptCtrl.Src_Sel = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.Src_Sel;/*magellan2*/

	sharp.Edge_Shp.LV2 = gVip_Table->Ddomain_SHPTable[value].Edge_Shp_set.LV2;
	sharp.Texture_Shp.LV2 = gVip_Table->Ddomain_SHPTable[value].Texture_Shp_set.LV2;
	sharp.Vertical.LV2 = gVip_Table->Ddomain_SHPTable[value].Vertical.LV2;//merlin

	sharp.MK3_Table.Edge_Text_blend_en = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.Edg_Tex_Blending_Lpf_En;
	sharp.MK3_Table.Edge_Text_blend_filter = gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl2.Edg_Tex_Blending_Lpf_Filter;

	/*magellan2 start*/
	sharp.After_Filter.C0 = gVip_Table->Ddomain_SHPTable[value].After_Filter.C0;
	sharp.After_Filter.C1 = gVip_Table->Ddomain_SHPTable[value].After_Filter.C1;
	sharp.After_Filter.C2 = gVip_Table->Ddomain_SHPTable[value].After_Filter.C2;
	sharp.After_Filter.C3 = gVip_Table->Ddomain_SHPTable[value].After_Filter.C3;
	sharp.After_Filter.C4 = gVip_Table->Ddomain_SHPTable[value].After_Filter.C4;
	sharp.After_Filter.HPF_Gain = gVip_Table->Ddomain_SHPTable[value].After_Filter.HPF_Gain;
	sharp.After_Filter.Mode = gVip_Table->Ddomain_SHPTable[value].After_Filter.Mode;
	sharp.After_Filter.Enable = gVip_Table->Ddomain_SHPTable[value].After_Filter.Enable;
	sharp.After_Filter.Rgblimit_pos= gVip_Table->Ddomain_SHPTable[value].After_Filter.Rgblimit_pos;
	sharp.After_Filter.Rgblimit_neg= gVip_Table->Ddomain_SHPTable[value].After_Filter.Rgblimit_neg;
	/*magellan2 end*/

	sharp.TwoD_Peak_AdaptCtrl.Sobel_h= gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.sobelrange_h;//merlin
	sharp.TwoD_Peak_AdaptCtrl.Sobel_v= gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.sobelrange_v;//merlin
	sharp.TwoD_Peak_AdaptCtrl.Acts_offset= gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.acts_offset;//merlin
	sharp.TwoD_Peak_AdaptCtrl.Small_thl= gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.small_th1;//merlin
	sharp.TwoD_Peak_AdaptCtrl.Edgextrange_v= gVip_Table->Ddomain_SHPTable[value].TwoD_Peak_AdaptCtrl.edgextrange_v;//merlin

	// patch for small review window
	if (VIP_system_info_structure_table != NULL) {
		if(VIP_system_info_structure_table->Input_src_Type == _SRC_TV) {
			scaleup_dm_uzu_ctrl_RBUS dm_uzu_Ctrl_reg;
			scaleup_dm_uzu_scale_hor_factor_RBUS dm_uzu_Scale_Hor_Factor_REG;
			dm_uzu_Ctrl_reg.regValue = rtd_inl(SCALEUP_DM_UZU_Ctrl_reg);
			dm_uzu_Scale_Hor_Factor_REG.regValue = rtd_inl(SCALEUP_DM_UZU_Scale_Hor_Factor_reg);

			if ((dm_uzu_Ctrl_reg.h_zoom_en == 0 && dm_uzu_Ctrl_reg.v_zoom_en == 0) ||
				(dm_uzu_Ctrl_reg.h_zoom_en == 1 && dm_uzu_Scale_Hor_Factor_REG.hor_factor >= 862017)) {
				sharp.TwoD_Peak_AdaptCtrl.Peaking_Enable = 0;
			}
		}
		//if((Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_422CAP) == FALSE) && (get_scaler_connect_source(SLR_MAIN_DISPLAY) == _SRC_HDMI)){		// HDMI RGB 444 input
		//	sharp.TwoD_Peak_AdaptCtrl.Peaking_Enable = 0;
		//}
	}

	drvif_color_Set_Sharpness_Table((DRV_Sharpness_Table *)&sharp);
}


unsigned char fwif_TwoParts_Linear_Interpolation(Sharpness_OSD_Remap_Curve *ptr, unsigned char value)
{


	unsigned char temp = 0;
	unsigned char startPoint, endPoint;
	/*
	unsigned char reg_point_bottom = 0;
	unsigned char reg_point_mid = 0;
	unsigned char reg_point_top = 0;
	*/

	if (value < ptr->Sharpness_OSD_mid) {
		startPoint = ptr->Sharpness_ChipValue_bottom;
		endPoint = ptr->Sharpness_ChipValue_mid;
		temp = (startPoint + (endPoint-startPoint)*value/(ptr->Sharpness_OSD_mid - ptr->Sharpness_OSD_bottom));

	} else {
		startPoint = ptr->Sharpness_ChipValue_mid;
		endPoint = ptr->Sharpness_ChipValue_top;
		value -= ptr->Sharpness_OSD_mid;
		temp = (startPoint + (endPoint-startPoint)*value/(ptr->Sharpness_OSD_top - ptr->Sharpness_OSD_mid));
	}

	if (temp > 255)
		temp = 255;

	return (unsigned char)temp;

}




void fwif_color_set_SharpnessFilterCoef_for_3D(CHIP_SHARPNESS_UI_T *CHIPShpTable, CHIP_SHARPNESS_UI_T *CHIPShpTable3DCorrection)
{
// K4Lp no
}

unsigned char  fwif_color_set_dcc_Curve_Write_tv006(signed int *sAccLUT)
{
	//_clues *smartPic_clue = fwif_color_Get_SmartPic_clue();
	_RPC_clues *RPC_smartPic_clue = (_RPC_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_SMARTPIC_CLUS);

	if (RPC_smartPic_clue == NULL) {
		VIPprintf("smartPic_clue NULL!!!");
		return FALSE;
	}

	if (sAccLUT == NULL) {
		VIPprintf("sAccLUT NULL!!!");
		return FALSE;
	}


	/*Monotonic Check*/
	if (Monotonic_Check) {
		int i=0;
		for (i=0; i<DCC_Curve_Node_MAX-1; i++) {
			if ((*(sAccLUT+i)) > (*(sAccLUT+i+1))) {
				VIPprintf("[Warnning] [%s-%s-%d] [DCC Curve] Not monotonic!,index=%d,%d\n", __FILE__, __func__, __LINE__,i,i+1);
				break;
			}
		}
	}

	fwif_color_set_dcc_OnOff(0, DCC_ON_APPLY_AP_CURVE);
	//fwif_color_ChangeINT32Endian_Copy(sAccLUT, DCC_Curve_Node_MAX, smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.Apply_Curve, 0);
	fwif_color_ChangeINT32Endian_Copy(sAccLUT, DCC_Curve_Node_MAX, RPC_smartPic_clue->RPC_Adapt_DCC_Info.Apply_Curve, 0);
	return TRUE;
}

unsigned char  fwif_color_set_bypass_dcc_Curve_Write_tv006(void)
{
	//_clues *smartPic_clue = fwif_color_Get_SmartPic_clue();
	_RPC_clues *RPC_smartPic_clue = (_RPC_clues *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_SMARTPIC_CLUS);
	signed int sAccLUT[DCC_Curve_Node_MAX];
	int i;

	if (RPC_smartPic_clue == NULL) {
		VIPprintf("smartPic_clue NULL!!!");
		return FALSE;
	}

	for (i = 0; i < DCC_Curve_Node_MAX-1; i++) {
		sAccLUT[i] = i<<5;
	}
	sAccLUT[DCC_Curve_Node_MAX-1] = 4095;

	fwif_color_set_dcc_OnOff(0, DCC_ON_APPLY_AP_CURVE);
	//fwif_color_ChangeINT32Endian_Copy(sAccLUT, DCC_Curve_Node_MAX, smartPic_clue->Adapt_DCC_Info.S_RPC_DCC_LAYER_Array.Apply_Curve, 0);
	fwif_color_ChangeINT32Endian_Copy(sAccLUT, DCC_Curve_Node_MAX, RPC_smartPic_clue->RPC_Adapt_DCC_Info.Apply_Curve, 0);
	return TRUE;
}

// 0622 lsy
//extern UINT8 vpq_stereo_face;//0: off, 1: on, 2: demo
extern RTK_AI_PQ_mode aipq_mode;
// end 0622 lsy
unsigned char fwif_color_set_dcc_FreshContrast_tv006(CHIP_DCC_T *ptr)
{
	if (!ptr) {
		VIPprintf("~fwif_color_set_dcc_FreshContrast_tv006 Table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return FALSE;
	}

	{
	UINT8 i = 0;
	int tmp, tmpValue, tmpDiff;
	static VIP_DCC_Chroma_Compensation_TABLE Chroma_Compensation_TABLE;

	Chroma_Compensation_TABLE.Chroma_Compensation_Ctrl.gain_en = ptr->stChromaGain.chroma_gain_en;
	Chroma_Compensation_TABLE.Chroma_Compensation_Ctrl.gain_mode = ptr->stChromaGain.chroma_gain_mode;
	Chroma_Compensation_TABLE.Chroma_Compensation_Ctrl.gain_lookup_mode = ptr->stChromaGain.chroma_gain_lookup_mode;
	Chroma_Compensation_TABLE.Chroma_Compensation_Ctrl.gain_base = ptr->stChromaGain.chroma_gain_base;
	Chroma_Compensation_TABLE.Chroma_Compensation_Ctrl.enh_mode_above_tab_sel = ptr->stChromaGain.chroma_enhance_above_tab_select_main;
	Chroma_Compensation_TABLE.Chroma_Compensation_Ctrl.enh_mode_below_tab_sel = ptr->stChromaGain.chroma_enhance_below_tab_select_main;
	Chroma_Compensation_TABLE.Chroma_Compensation_Ctrl.byY_mode_u_tab_sel = ptr->stChromaGain.saturation_byy_u_tab_select_main;
	Chroma_Compensation_TABLE.Chroma_Compensation_Ctrl.byY_mode_v_tab_sel = ptr->stChromaGain.saturation_byy_v_tab_select_main;

	for(i = 0; i < Chroma_Gain_Limit_Seg_MAX; i++) {
		if(i>7)
			Chroma_Compensation_TABLE.chroma_gain_limit[i]=64; //fixed me , db need changed.
		else
			Chroma_Compensation_TABLE.chroma_gain_limit[i] = ptr->stChromaGain.chroma_gain_limit_seg[i];
	}

	for(i = 0; i < (Chroma_Gain_Seg_MAX>>2); i++) {
		tmp = i<<2;
		tmpValue = ptr->stChromaGain.chroma_gain_tab0_seg[i];
		tmpDiff = ptr->stChromaGain.chroma_gain_tab0_seg[i+1]-tmpValue;
		Chroma_Compensation_TABLE.chroma_gain_t0[tmp] = tmpValue;
		Chroma_Compensation_TABLE.chroma_gain_t0[tmp+1] = tmpValue+(tmpDiff>>2);
		Chroma_Compensation_TABLE.chroma_gain_t0[tmp+2] = tmpValue+(tmpDiff>>1);
		Chroma_Compensation_TABLE.chroma_gain_t0[tmp+3] = tmpValue+tmpDiff-(tmpDiff>>2);
	}
	Chroma_Compensation_TABLE.chroma_gain_t0[Chroma_Gain_Seg_MAX-1] = ptr->stChromaGain.chroma_gain_tab0_seg[16];

	for(i = 0; i < (Chroma_Gain_Seg_MAX>>2); i++){
		tmp = i<<2;
		tmpValue = ptr->stChromaGain.chroma_gain_tab1_seg[i];
		tmpDiff = ptr->stChromaGain.chroma_gain_tab1_seg[i+1]-tmpValue;
		Chroma_Compensation_TABLE.chroma_gain_t1[tmp] = tmpValue;
		Chroma_Compensation_TABLE.chroma_gain_t1[tmp+1] = tmpValue+(tmpDiff>>2);
		Chroma_Compensation_TABLE.chroma_gain_t1[tmp+2] = tmpValue+(tmpDiff>>1);
		Chroma_Compensation_TABLE.chroma_gain_t1[tmp+3] = tmpValue+tmpDiff-(tmpDiff>>2);
	}
	Chroma_Compensation_TABLE.chroma_gain_t1[Chroma_Gain_Seg_MAX-1] = ptr->stChromaGain.chroma_gain_tab1_seg[16];

	drvif_color_set_dcc_chroma_compensation_Curve_Mode(_MAIN_DISPLAY, &Chroma_Compensation_TABLE);
	}

	{
	VIP_DCC_Color_Independent_Blending_Table Color_independent = {{0}};

	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.Y_Center0 = ptr->stSkinTone.range.y0_center;
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.U_Center0 = ptr->stSkinTone.range.u0_center;
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.V_Center0 = ptr->stSkinTone.range.v0_center;
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.Y_Range0 = ptr->stSkinTone.range.y0_range;
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.U_Range0 = ptr->stSkinTone.range.u0_range;
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.V_Range0 = ptr->stSkinTone.range.v0_range;

	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.Y_Center1 = ptr->stSkinTone.range.y1_center;
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.U_Center1 = ptr->stSkinTone.range.u1_center;
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.V_Center1 = ptr->stSkinTone.range.v1_center;
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.Y_Range1 = ptr->stSkinTone.range.y1_range;
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.U_Range1 = ptr->stSkinTone.range.u1_range;
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.V_Range1 = ptr->stSkinTone.range.v1_range;

	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.Y_Center2 = ptr->stSkinTone.range.y2_center;
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.U_Center2 = ptr->stSkinTone.range.u2_center;
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.V_Center2 = ptr->stSkinTone.range.v2_center;
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.Y_Range2 = ptr->stSkinTone.range.y2_range;
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.U_Range2 = ptr->stSkinTone.range.u2_range;
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.V_Range2 = ptr->stSkinTone.range.v2_range;

	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.Y_Center3 = ptr->stSkinTone.range.y3_center;
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.U_Center3 = ptr->stSkinTone.range.u3_center;
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.V_Center3 = ptr->stSkinTone.range.v3_center;
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.Y_Range3 = ptr->stSkinTone.range.y3_range;
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.U_Range3 = ptr->stSkinTone.range.u3_range;
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.V_Range3 = ptr->stSkinTone.range.v3_range;

	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.region0_en = ptr->stSkinTone.value.D_DCC_SKIN_EN;
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_0 = ptr->stSkinTone.value.Y0_seg[0];
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_1 = ptr->stSkinTone.value.Y0_seg[1];
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_2 = ptr->stSkinTone.value.Y0_seg[2];
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_3 = ptr->stSkinTone.value.Y0_seg[3];
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_4 = ptr->stSkinTone.value.Y0_seg[4];
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_5 = ptr->stSkinTone.value.Y0_seg[5];
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_6 = ptr->stSkinTone.value.Y0_seg[6];
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_7 = ptr->stSkinTone.value.Y0_seg[7];
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_0_c = ptr->stSkinTone.value.C0_seg[0];
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_1_c = ptr->stSkinTone.value.C0_seg[1];
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_2_c = ptr->stSkinTone.value.C0_seg[2];
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_3_c = ptr->stSkinTone.value.C0_seg[3];
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_4_c = ptr->stSkinTone.value.C0_seg[4];
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_5_c = ptr->stSkinTone.value.C0_seg[5];
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_6_c = ptr->stSkinTone.value.C0_seg[6];
	Color_independent.S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_7_c = ptr->stSkinTone.value.C0_seg[7];

	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.region1_en = ptr->stSkinTone.value.D_DCC_SKIN_EN;
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_0 = ptr->stSkinTone.value.Y1_seg[0];
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_1 = ptr->stSkinTone.value.Y1_seg[1];
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_2 = ptr->stSkinTone.value.Y1_seg[2];
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_3 = ptr->stSkinTone.value.Y1_seg[3];
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_4 = ptr->stSkinTone.value.Y1_seg[4];
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_5 = ptr->stSkinTone.value.Y1_seg[5];
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_6 = ptr->stSkinTone.value.Y1_seg[6];
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_7 = ptr->stSkinTone.value.Y1_seg[7];
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_0_c = ptr->stSkinTone.value.C1_seg[0];
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_1_c = ptr->stSkinTone.value.C1_seg[1];
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_2_c = ptr->stSkinTone.value.C1_seg[2];
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_3_c = ptr->stSkinTone.value.C1_seg[3];
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_4_c = ptr->stSkinTone.value.C1_seg[4];
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_5_c = ptr->stSkinTone.value.C1_seg[5];
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_6_c = ptr->stSkinTone.value.C1_seg[6];
	Color_independent.S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_7_c = ptr->stSkinTone.value.C1_seg[7];

	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.region2_en = ptr->stSkinTone.value.D_DCC_SKIN_EN;
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_0 = ptr->stSkinTone.value.Y2_seg[0];
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_1 = ptr->stSkinTone.value.Y2_seg[1];
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_2 = ptr->stSkinTone.value.Y2_seg[2];
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_3 = ptr->stSkinTone.value.Y2_seg[3];
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_4 = ptr->stSkinTone.value.Y2_seg[4];
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_5 = ptr->stSkinTone.value.Y2_seg[5];
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_6 = ptr->stSkinTone.value.Y2_seg[6];
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_7 = ptr->stSkinTone.value.Y2_seg[7];
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_0_c = ptr->stSkinTone.value.C2_seg[0];
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_1_c = ptr->stSkinTone.value.C2_seg[1];
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_2_c = ptr->stSkinTone.value.C2_seg[2];
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_3_c = ptr->stSkinTone.value.C2_seg[3];
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_4_c = ptr->stSkinTone.value.C2_seg[4];
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_5_c = ptr->stSkinTone.value.C2_seg[5];
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_6_c = ptr->stSkinTone.value.C2_seg[6];
	Color_independent.S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_7_c = ptr->stSkinTone.value.C2_seg[7];

	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.region3_en = ptr->stSkinTone.value.D_DCC_SKIN_EN;
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_0 = ptr->stSkinTone.value.Y3_seg[0];
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_1 = ptr->stSkinTone.value.Y3_seg[1];
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_2 = ptr->stSkinTone.value.Y3_seg[2];
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_3 = ptr->stSkinTone.value.Y3_seg[3];
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_4 = ptr->stSkinTone.value.Y3_seg[4];
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_5 = ptr->stSkinTone.value.Y3_seg[5];
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_6 = ptr->stSkinTone.value.Y3_seg[6];
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_7 = ptr->stSkinTone.value.Y3_seg[7];
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_0_c = ptr->stSkinTone.value.C3_seg[0];
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_1_c = ptr->stSkinTone.value.C3_seg[1];
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_2_c = ptr->stSkinTone.value.C3_seg[2];
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_3_c = ptr->stSkinTone.value.C3_seg[3];
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_4_c = ptr->stSkinTone.value.C3_seg[4];
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_5_c = ptr->stSkinTone.value.C3_seg[5];
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_6_c = ptr->stSkinTone.value.C3_seg[6];
	Color_independent.S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_7_c = ptr->stSkinTone.value.C3_seg[7];
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
// 0622 lsy
	//if(vpq_stereo_face != V4L2_VPQ_EXT_STEREO_FACE_OFF)
	if(aipq_mode.face_mode != V4L2_VPQ_EXT_STEREO_FACE_OFF)
// end 0622 lsy
		drvif_color_set_dcc_Color_Independent_AI(&Color_independent);
	else
#endif
		drvif_color_set_dcc_Color_Independent(&Color_independent);
	}
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
	// lesley 1014
	{
	//extern void drvif_color_get_DB_AI_DCC(CHIP_DCC_T *ptr); // V4L2_ERR
	//drvif_color_get_DB_AI_DCC(ptr); // V4L2_ERR
	}
	// end lesley 1014
#endif
	return TRUE;
}

void fwif_color_set_iDcti_dDcti_tv006(unsigned char i_dcti_val, unsigned char d_dcti_val, CHIP_CTI_T *set_cti_db)
{
	//extern DRV_VipNewIDcti_Table gVipINewDcti_Table[DCTI_TABLE_LEVEL_MAX];
	//extern DRV_VipNewDDcti_Table gVipDNewDcti_Table[DCTI_TABLE_LEVEL_MAX];
	static CHIP_CTI_T cti_db = {0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	static unsigned char i_dcti_val_real = 0;
	static unsigned char d_dcti_val_real = 0;
	_system_setting_info *VIP_system_info_structure_table = NULL;
	DRV_VipNewIDcti_Table i_dcti;
	DRV_VipNewDDcti_Table d_dcti;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	if (NULL != set_cti_db) {
		memcpy(&cti_db, set_cti_db, sizeof(CHIP_CTI_T));
	} else {
		StructColorDataType *pTable = NULL;
		unsigned char src_idx = 0;
		unsigned char display = 0;
		Scaler_Get_Display_info(&display, &src_idx);

		pTable = fwif_color_get_color_data(src_idx, 0);
		if (pTable == NULL)
			return;
		pTable->I_NEWDCTi = i_dcti_val;
		pTable->DCti = d_dcti_val;

		if (i_dcti_val >= DCTI_TABLE_LEVEL_MAX)
			i_dcti_val = 0;

		if (d_dcti_val >= DCTI_TABLE_LEVEL_MAX)
			d_dcti_val = 0;

		i_dcti_val_real = i_dcti_val;
		d_dcti_val_real = d_dcti_val;
	}

	//memcpy(&i_dcti, &gVipINewDcti_Table[i_dcti_val_real], sizeof(DRV_VipNewIDcti_Table));
	memcpy(&i_dcti, &gVip_Table->VIP_INewDcti_Table[i_dcti_val_real], sizeof(DRV_VipNewIDcti_Table));
	//memcpy(&d_dcti, &gVipDNewDcti_Table[d_dcti_val_real], sizeof(DRV_VipNewDDcti_Table));
	memcpy(&d_dcti, &gVip_Table->VIP_DNewDcti_Table[d_dcti_val_real], sizeof(DRV_VipNewDDcti_Table));
	if (cti_db.i_dcti_enable != 0xff)
	{
		i_dcti.VipNewIDcti_ctl1.dcti_en = cti_db.i_dcti_enable;
		i_dcti.VipNewIDcti_ctl1.uvgain = cti_db.i_dcti_gain;
		i_dcti.VipNewIDcti_ctl1.dcti_mode = cti_db.i_dcti_mode;
		//i_dcti.VipNewIDcti_ctl1.uvalign_en = cti_db.i_dcti_uv_align_enable;//k5lp remove it
		i_dcti.VipNewIDcti_ctl2.uvsync_en = cti_db.i_dcti_uv_sync_enable;
		i_dcti.VipNewIDcti_ctl1.maxlen = cti_db.i_dcti_Max_search_range;
		i_dcti.VipNewIDcti_ctl1.psmth = cti_db.i_dcti_Threshold_of_smooth;

                // v gain not in db , same as  u gain
                i_dcti.VipNewIDcti_ctl1_v.uvgain_v  = cti_db.i_dcti_gain;;

                d_dcti.VipNewDDcti_ctl1.dcti_en = cti_db.d_dcti_enable;
		d_dcti.VipNewDDcti_ctl1.uvgain = cti_db.d_dcti_gain;
		d_dcti.VipNewDDcti_ctl1.dcti_mode = cti_db.d_dcti_mode;
		d_dcti.VipNewDDcti_ctl1.uvalign_en = cti_db.d_dcti_uv_align_enable;
		d_dcti.VipNewDDcti_ctl2.uvsync_en = cti_db.d_dcti_uv_sync_enable;
		d_dcti.VipNewDDcti_ctl1.maxlen = cti_db.d_dcti_Max_search_range;
		d_dcti.VipNewDDcti_ctl1.psmth = cti_db.d_dcti_Threshold_of_smooth;

		if (d_dcti.VipNewVDDcti_ctl1.uvgain_ver>cti_db.vcti_gain*2)
			d_dcti.VipNewVDDcti_ctl1.uvgain_ver = cti_db.vcti_gain*2;
	}

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	VIP_system_info_structure_table->PQ_Setting_Info.I_D_Dcti_INFO.iDcti_table_select = i_dcti_val_real;
	VIP_system_info_structure_table->PQ_Setting_Info.I_D_Dcti_INFO.dDcti_table_select = d_dcti_val_real;

	drvif_color_inewdcti_table(&i_dcti);
	drvif_color_dnewdcti_table(&d_dcti);
}

void fwif_color_set_Vdcti_tv006(unsigned char v_dcti_val, CHIP_CTI_T *set_cti_db)
{
	//extern DRV_D_vcti_t gVipVcti_Table[DCTI_TABLE_LEVEL_MAX];
	VIP_D_vcti_t drv_D_vcti_t;
	unsigned char old_db_read_level;
	scaleup_dm_uzu_db_ctrl_RBUS scaleup_dm_uzu_db_ctrl_reg;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	//memcpy(&drv_D_vcti_t, &gVipVcti_Table[v_dcti_val], sizeof(DRV_D_vcti_t));
	memcpy(&drv_D_vcti_t, &gVip_Table->VIP_Vcti_Table[v_dcti_val], sizeof(VIP_D_vcti_t));

	if (set_cti_db == NULL)
		return;	/* quality handle, DB can't be overwrited */

	/*printk("vcti=%d,%d,%d,\n", set_cti_db->vcti_gain,  set_cti_db->vcti_type, set_cti_db->vcti_type3_thd);*/

	/*==========set from DB=============*/
	/* data check*/
	if ((set_cti_db->vcti_type <= 3) && (set_cti_db->vcti_gain <= 127) && (set_cti_db->vcti_type3_thd <= 63)) {
		drv_D_vcti_t.vcti_en = set_cti_db->vcti_enable;
		drv_D_vcti_t.vcti_select = set_cti_db->vcti_type;
		drv_D_vcti_t.vcti_gain = set_cti_db->vcti_gain;
		drv_D_vcti_t.vcti_type3_thd = set_cti_db->vcti_type3_thd;
	}
	/*==========set from DB END=============*/
	scaleup_dm_uzu_db_ctrl_reg.regValue = rtd_inl(SCALEUP_DM_UZU_DB_CTRL_reg);
	old_db_read_level = scaleup_dm_uzu_db_ctrl_reg.db_read_level;
	scaleup_dm_uzu_db_ctrl_reg.db_read_level = 0;
	rtd_outl(SCALEUP_DM_UZU_DB_CTRL_reg, scaleup_dm_uzu_db_ctrl_reg.regValue);
#ifndef BUILD_QUICK_SHOW
	if (SCALEUP_DM_UZU_Ctrl_get_v_zoom_en(rtd_inl(SCALEUP_DM_UZU_Ctrl_reg)) == 0 && !drvif_scalerdisplay_get_force_bg_status()/* && Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE*/) {
		scaleup_dm_uzu_db_ctrl_reg.db_read_level = old_db_read_level;
		rtd_outl(SCALEUP_DM_UZU_DB_CTRL_reg, scaleup_dm_uzu_db_ctrl_reg.regValue);
		return;
	}
#endif	
	scaleup_dm_uzu_db_ctrl_reg.db_read_level = old_db_read_level;
	rtd_outl(SCALEUP_DM_UZU_DB_CTRL_reg, scaleup_dm_uzu_db_ctrl_reg.regValue);

	drvif_color_scaler_setds_vcti(_MAIN_DISPLAY,&drv_D_vcti_t);

}

void fwif_color_set_dLti_tv006(unsigned char d_lti_val, CHIP_HLTI_T *H_LTI_db,CHIP_VLTI_T *V_LTI_db)
{
	//extern VIP_D_DLTI_Table D_DLTI_Table[D_DLTI_table_num];
	VIP_D_DLTI_Table drv_shp_Dlti = {{0}};
	static CHIP_HLTI_T store_HLTI_db = {0};
	static CHIP_VLTI_T store_VLTI_db = {0};
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	//UINT8 lti_gain;

	/*printk("fwif_color_set_dLti_tv006=%d\n",d_lti_val);*/

	if (d_lti_val >= VIP_D_DLTI_table_num/*sizeof(D_DLTI_Table)/sizeof(DRV_D_DLTI_Table)*/) {/* 255 = bypass*/
		drvif_color_Set_D_DLTI_Table(&drv_shp_Dlti);
		return;
	}

	//memcpy(&drv_shp_Dlti, &D_DLTI_Table[d_lti_val], sizeof(DRV_D_DLTI_Table));
	memcpy(&drv_shp_Dlti, &gVip_Table->vip_D_DLTI_Table[d_lti_val], sizeof(VIP_D_DLTI_Table));
	if (H_LTI_db == NULL)
		//return;	/* quality handle, DB can't be overwrited */
		H_LTI_db = &store_HLTI_db;
	else
		memcpy(&store_HLTI_db, H_LTI_db, sizeof(CHIP_HLTI_T));

	if (V_LTI_db == NULL)
		//return;	/* quality handle, DB can't be overwrited */
		V_LTI_db = &store_VLTI_db;
	else
		memcpy(&store_VLTI_db, V_LTI_db, sizeof(CHIP_VLTI_T));


	drv_shp_Dlti.DLTI_Control.Dlti_en = 1; //this bit always Enable.

	/*==========set from DB H LTI=============*/


	drv_shp_Dlti.DLTI_H_1.dlti_h_en = H_LTI_db->d_LTI_enable;
	drv_shp_Dlti.DLTI_H_1.Can_shift = H_LTI_db->d_LTI_can_shift;
	drv_shp_Dlti.DLTI_H_1.maxlen =	H_LTI_db->d_LTI_maxlen;
	drv_shp_Dlti.DLTI_H_1.pnth = H_LTI_db->d_LTI_pnth;
	drv_shp_Dlti.DLTI_H_2.ffd2_en = H_LTI_db->ffd2_Enable;
	drv_shp_Dlti.DLTI_H_1.Data_sel = H_LTI_db->Data_sel;
	// drv_shp_Dlti.DLTI_H_1.ovc_extend = H_LTI_db->ovc_extend;
	drv_shp_Dlti.DLTI_H_1.ffd0_start = H_LTI_db->ffd0_start;
	drv_shp_Dlti.DLTI_H_2.D1_CP_shift = H_LTI_db->D1_CP_Shift;
	drv_shp_Dlti.DLTI_H_2.DCP_th = H_LTI_db->DCP_th;
	drv_shp_Dlti.DLTI_H_2.flat_s_range = H_LTI_db->Flat_s_range;
	drv_shp_Dlti.DLTI_H_2.flat_score_thl = H_LTI_db->Flat_score_th1;
	// drv_shp_Dlti.DLTI_H_1.Ffd1_th = H_LTI_db->Ffd1_th;
	drv_shp_Dlti.DLTI_H_1.Ffd1_ratio = H_LTI_db->Ffd1_ratio;

	drv_shp_Dlti.DLTI_H_2.Ffd2_range2R = H_LTI_db->Ffd2_range2R;
	drv_shp_Dlti.DLTI_H_2.Ffd2_range2L = H_LTI_db->Ffd2_range2L;
	drv_shp_Dlti.DLTI_H_2.Ffd2_ratio = H_LTI_db->Ffd2_ratio;
	drv_shp_Dlti.DLTI_H_OverEnhance.gain_pos = H_LTI_db->gain_pos;
	drv_shp_Dlti.DLTI_H_OverEnhance.offset_pos = H_LTI_db->offset_pos;
	drv_shp_Dlti.DLTI_H_OverEnhance.gain_neg = H_LTI_db->gain_neg;
	drv_shp_Dlti.DLTI_H_OverEnhance.offset_neg = H_LTI_db->offset_neg;
	drv_shp_Dlti.DLTI_H_Tnoff.Tnoff0 = H_LTI_db->Tnoff0;
	drv_shp_Dlti.DLTI_H_Tnoff.Tnoff1 = H_LTI_db->Tnoff1;
	drv_shp_Dlti.DLTI_H_Tnoff.Tnoff2 = H_LTI_db->Tnoff2;
	drv_shp_Dlti.DLTI_H_Tnoff.Tnoff3 = H_LTI_db->Tnoff3;
	drv_shp_Dlti.DLTI_H_Tnoff.Tnoff4 = H_LTI_db->Tnoff4;
	drv_shp_Dlti.DLTI_H_Tnoff.Tnoff5 = H_LTI_db->Tnoff5;
	drv_shp_Dlti.DLTI_H_Tnoff.Tnoff6 = H_LTI_db->Tnoff6;
	drv_shp_Dlti.DLTI_H_Tnoff.Tnoff7 = H_LTI_db->Tnoff7;
	drv_shp_Dlti.DLTI_H_Tnoff.Tnoff8 = H_LTI_db->Tnoff8;
	drv_shp_Dlti.DLTI_H_Tnoff.Tnoff9 = H_LTI_db->Tnoff9;
	drv_shp_Dlti.DLTI_H_Tnoff.Tnoff10 = H_LTI_db->Tnoff10;
	drv_shp_Dlti.DLTI_H_Tnoff.Tnoff11 = H_LTI_db->Tnoff11;
	drv_shp_Dlti.DLTI_H_Tnoff.Tnoff12 = H_LTI_db->Tnoff12;
	drv_shp_Dlti.DLTI_H_Tnoff.Tnoff13 = H_LTI_db->Tnoff13;
	drv_shp_Dlti.DLTI_H_Tnoff.Tnoff14 = H_LTI_db->Tnoff14;


	/*==========set from DB V LTI=============*/

	drv_shp_Dlti.DLTI_V_1.dlti_v_en = V_LTI_db->d_LTI_enable;
	drv_shp_Dlti.DLTI_V_1.can_shift_v = V_LTI_db->Can_shift_v;
	drv_shp_Dlti.DLTI_V_1.maxlen_v =	V_LTI_db->maxlen_v;
	drv_shp_Dlti.DLTI_V_1.pnth_v= V_LTI_db->pnth_v;
    // drv_shp_Dlti.DLTI_V_1.ovc_v_extend = V_LTI_db->ovc_extend;
        drv_shp_Dlti.DLTI_V_1.v_ffd0_start = V_LTI_db->ffd0_start;
	// drv_shp_Dlti.DLTI_V_1.ffd1_th = V_LTI_db->ffd1_th_v;
	drv_shp_Dlti.DLTI_V_1.ffd1_ratio = V_LTI_db->ffd1_ratio_v;
	drv_shp_Dlti.DLTI_V_OverEnhance.gain_pos = V_LTI_db->gain_pos_v;
	drv_shp_Dlti.DLTI_V_OverEnhance.offset_pos = V_LTI_db->offset_pos_v;
	drv_shp_Dlti.DLTI_V_OverEnhance.gain_neg = V_LTI_db->gain_neg_v;
	drv_shp_Dlti.DLTI_V_OverEnhance.offset_neg = V_LTI_db->offset_neg_v;
	drv_shp_Dlti.DLTI_V_Tnoff.Tnoff0 = V_LTI_db->Tnoff0_v;
	drv_shp_Dlti.DLTI_V_Tnoff.Tnoff1 = V_LTI_db->Tnoff1_v;
	drv_shp_Dlti.DLTI_V_Tnoff.Tnoff2 = V_LTI_db->Tnoff2_v;
	drv_shp_Dlti.DLTI_V_Tnoff.Tnoff3 = V_LTI_db->Tnoff3_v;
	drv_shp_Dlti.DLTI_V_Tnoff.Tnoff4 = V_LTI_db->Tnoff4_v;
	drv_shp_Dlti.DLTI_V_Tnoff.Tnoff5 = V_LTI_db->Tnoff5_v;
	drv_shp_Dlti.DLTI_V_Tnoff.Tnoff6 = V_LTI_db->Tnoff6_v;


#if 0

	/*printk("lti0=%d,%d,%d,%d,%d,gain=%d\n", D_DLTI_Table[1].DLTI_H_Tnoff.Tnoff10,D_DLTI_Table[1].DLTI_H_Tnoff.Tnoff11,  D_DLTI_Table[1].DLTI_H_Tnoff.Tnoff12, D_DLTI_Table[1].DLTI_H_Tnoff.Tnoff13,D_DLTI_Table[1].DLTI_H_Tnoff.Tnoff14,lti_gain);*/

	/*printk("lti2=%d,%d,%d,%d,%d\n", LTI_db->d_LTI_can_shift,LTI_db->d_LTI_pnth, LTI_db->d_LTI_maxlen,LTI_db->d_LTI_gain ,drv_shp_Dlti.curve5);*/

	/*==========set from DB=============*/
	/* data check for  H LTI */
	if ((H_LTI_db->h_lti.d_LTI_can_shift <= 3) && (H_LTI_db->h_lti.d_LTI_pnth <= 1023) && (H_LTI_db->h_lti.d_LTI_maxlen <= 7) && (H_LTI_db->d_LTI_gain <= 100))
		{
		drv_shp_Dlti.DLTI_Control.Dlti_en = H_LTI_db->d_LTI_enable;
		drv_shp_Dlti.DLTI_H_1.Can_shift = H_LTI_db->d_LTI_can_shift;
		drv_shp_Dlti.DLTI_H_1.maxlen = 	H_LTI_db->d_LTI_maxlen;
		drv_shp_Dlti.DLTI_H_1.pnth = H_LTI_db->d_LTI_pnth;
		//lti_gain = LTI_db->h_lti.d_LTI_gain;

		//printk("lti1=%d,%d,%d,%d,%d,gain=%d\n", drv_shp_Dlti.DLTI_H_Tnoff.Tnoff10,drv_shp_Dlti.DLTI_H_Tnoff.Tnoff11,  drv_shp_Dlti.DLTI_H_Tnoff.Tnoff12, drv_shp_Dlti.DLTI_H_Tnoff.Tnoff13,drv_shp_Dlti.DLTI_H_Tnoff.Tnoff14,lti_gain);
		drv_shp_Dlti.DLTI_H_Tnoff.Tnoff0 = drv_shp_Dlti.DLTI_H_Tnoff.Tnoff0 * lti_gain / 100;
		drv_shp_Dlti.DLTI_H_Tnoff.Tnoff1 = drv_shp_Dlti.DLTI_H_Tnoff.Tnoff1 * lti_gain / 100;
		drv_shp_Dlti.DLTI_H_Tnoff.Tnoff2 = drv_shp_Dlti.DLTI_H_Tnoff.Tnoff2 * lti_gain / 100;
		drv_shp_Dlti.DLTI_H_Tnoff.Tnoff3 = drv_shp_Dlti.DLTI_H_Tnoff.Tnoff3 * lti_gain / 100;
		drv_shp_Dlti.DLTI_H_Tnoff.Tnoff4 = drv_shp_Dlti.DLTI_H_Tnoff.Tnoff4 * lti_gain / 100;
		drv_shp_Dlti.DLTI_H_Tnoff.Tnoff5 = drv_shp_Dlti.DLTI_H_Tnoff.Tnoff5 * lti_gain / 100;
		drv_shp_Dlti.DLTI_H_Tnoff.Tnoff6 = drv_shp_Dlti.DLTI_H_Tnoff.Tnoff6 * lti_gain / 100;
		drv_shp_Dlti.DLTI_H_Tnoff.Tnoff7 = drv_shp_Dlti.DLTI_H_Tnoff.Tnoff7 * lti_gain / 100;
		drv_shp_Dlti.DLTI_H_Tnoff.Tnoff8 = drv_shp_Dlti.DLTI_H_Tnoff.Tnoff8 * lti_gain / 100;
		drv_shp_Dlti.DLTI_H_Tnoff.Tnoff9 = drv_shp_Dlti.DLTI_H_Tnoff.Tnoff9 * lti_gain / 100;
		drv_shp_Dlti.DLTI_H_Tnoff.Tnoff10 = drv_shp_Dlti.DLTI_H_Tnoff.Tnoff10 * lti_gain / 100;
		drv_shp_Dlti.DLTI_H_Tnoff.Tnoff11 = drv_shp_Dlti.DLTI_H_Tnoff.Tnoff11 * lti_gain / 100;
		drv_shp_Dlti.DLTI_H_Tnoff.Tnoff12 = drv_shp_Dlti.DLTI_H_Tnoff.Tnoff12 * lti_gain / 100;
		drv_shp_Dlti.DLTI_H_Tnoff.Tnoff12 = drv_shp_Dlti.DLTI_H_Tnoff.Tnoff13 * lti_gain / 100;
		drv_shp_Dlti.DLTI_H_Tnoff.Tnoff14 = drv_shp_Dlti.DLTI_H_Tnoff.Tnoff14 * lti_gain / 100;

	}
	/*==========set from DB END=============*/

#endif

	//printk("lti2=%d,%d,%d,%d,%d,gain=%d\n", drv_shp_Dlti.DLTI_H_Tnoff.Tnoff10,drv_shp_Dlti.DLTI_H_Tnoff.Tnoff11,  drv_shp_Dlti.DLTI_H_Tnoff.Tnoff12, drv_shp_Dlti.DLTI_H_Tnoff.Tnoff13,drv_shp_Dlti.DLTI_H_Tnoff.Tnoff14,lti_gain);
	drvif_color_Set_D_DLTI_Table(&drv_shp_Dlti);

}

void fwif_color_set_MultiBand_Peaking_tv006(unsigned char multiBand_val, CHIP_MULTIBAND_PEAKING_HPF_T *mb_hpf, CHIP_MULTIBAND_PEAKING_UI_T* mb_UI)
{
	VIP_MBPK_Table MBPK;

	/* ==================get data from vip table======================*/
	UINT8 value = multiBand_val;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	//short h_dc_check, v_dc_check;
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	if (value >= MBPK_table_num) {	/* bypass = 255*/
		MBPK.MBPK_Ctrl.mb_peaking_en = 0;
		value = 0;
	} else {
		memcpy(&MBPK,gVip_Table->Idomain_MBPKTable+value,sizeof(VIP_MBPK_Table));
	}

	/* ==================get data from vip table END======================*/

	if (mb_hpf == NULL || mb_UI == NULL)
		return;	/* quality handle, DB can't be overwrited */

	/*==========set from DB=============*/
	/* filter coeg dc check*/
	//h_dc_check = mb_hpf->h_c0 + 2*mb_hpf->h_c1 + 2*mb_hpf->h_c2 + 2*mb_hpf->h_c3;
	//v_dc_check = mb_hpf->v_c0 + 2*mb_hpf->v_c1 + 2*mb_hpf->v_c2;
	/*printk("MB=%d,%d,%d,%d,%d,%d,%d,,%d,%d,%d,%d\n", mb_hpf->h_c0,mb_hpf->h_c1,mb_hpf->h_c2,mb_hpf->h_c3,mb_hpf->h_c4,mb_hpf->h_c5,mb_hpf->h_c6
		,mb_hpf->v_c0,mb_hpf->v_c1,mb_hpf->v_c2);*/


       // if ((h_dc_check == 0) && (v_dc_check == 0) && (mb_hpf->h_c0 != 0) && (mb_hpf->v_c0 != 0))
       {
		MBPK.MBPK_Ctrl.mb_peaking_en = mb_hpf->mb_peaking_en;
		MBPK.MBPK_H_Coef.MB_Hor_Filter_C0 = mb_hpf->h_c0;
		MBPK.MBPK_H_Coef.MB_Hor_Filter_C1 = mb_hpf->h_c1;
		MBPK.MBPK_H_Coef.MB_Hor_Filter_C2 = mb_hpf->h_c2;
		MBPK.MBPK_H_Coef.MB_Hor_Filter_C3 = mb_hpf->h_c3;
		//MBPK.MBPK_H_Coef.MB_Hor_Filter_C4 = mb_hpf->h_c4;
		//MBPK.MBPK_H_Coef.MB_Hor_Filter_C5 = mb_hpf->h_c5;
		//MBPK.MBPK_H_Coef.MB_Hor_Filter_C6 = mb_hpf->h_c6;
		MBPK.MBPK_V_Coef.MB_Ver_Filter_C0 = mb_hpf->v_c0;
		MBPK.MBPK_V_Coef.MB_Ver_Filter_C1 = mb_hpf->v_c1;
		MBPK.MBPK_V_Coef.MB_Ver_Filter_C2 = mb_hpf->v_c2;

		MBPK.MBPK_Ctrl.MB_H_En = mb_UI->mb_h_en;
		MBPK.MBPK_H_Table.MB_Gain_Pos = mb_UI->H_Gain_Pos;
		MBPK.MBPK_H_Table.MB_Gain_Neg = mb_UI->H_Gain_Neg;
		MBPK.MBPK_H_Table.MB_LV = mb_UI->H_Coring;
		MBPK.MBPK_H_Table.MB_Gain_Pos2 = mb_UI->H_Gain_Pos2;
		MBPK.MBPK_H_Table.MB_Gain_Neg2 = mb_UI->H_Gain_Neg2;
		MBPK.MBPK_H_Table.MB_HV_Neg = mb_UI->H_Range_Pos;
		MBPK.MBPK_H_Table.MB_HV_Pos = mb_UI->H_Range_Neg;
		MBPK.MBPK_H_Table.MB_D2_shift_bit = mb_UI->H_Edge_Shift;
		MBPK.MBPK_Ctrl.MB_V_En = mb_UI->mb_v_en;
		MBPK.MBPK_V_Table.MB_Gain_Pos_V = mb_UI->V_Gain_Pos;
		MBPK.MBPK_V_Table.MB_Gain_Neg_V = mb_UI->V_Gain_Neg;
		MBPK.MBPK_V_Table.MB_LV_V = mb_UI->V_Coring;
		MBPK.MBPK_V_Table.MB_Gain_Pos2_V = mb_UI->V_Gain_Pos2;
		MBPK.MBPK_V_Table.MB_Gain_Neg2_V = mb_UI->V_Gain_Neg2;
		MBPK.MBPK_V_Table.MB_HV_Pos_V = mb_UI->V_Range_Pos;
		MBPK.MBPK_V_Table.MB_HV_Neg_V = mb_UI->V_Range_Neg;
		MBPK.MBPK_V_Table.MB_D2_shift_bit_V = mb_UI->V_Edge_Shift;


	}
	/*==========set from DB END=============*/

	drvif_color_Set_MBPK_Table((VIP_MBPK_Table *)&MBPK);
}

void fwif_color_set_MultiBand_SU_Peaking_tv006(unsigned char multiBand_val, CHIP_MULTIBAND_PEAKING_HPF_T *mb_hpf, CHIP_MULTIBAND_PEAKING_UI_T* mb_UI)
{
	VIP_MBPK_Table MBPK;

	/* ==================get data from vip table======================*/
	UINT8 value = multiBand_val;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	short h_dc_check, v_dc_check;
	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	if (value >= MBPK_table_num) {	/* bypass = 255*/
		MBPK.MBPK_Ctrl.mb_peaking_en = 0;
		value = 0;
	} else {
		memcpy(&MBPK,gVip_Table->Ddomain_MBSUTable+value,sizeof(VIP_MBPK_Table));
	}

	/* ==================get data from vip table END======================*/

	if (mb_hpf == NULL || mb_UI == NULL)
		return;	/* quality handle, DB can't be overwrited */

	/*==========set from DB=============*/
	/* filter coeg dc check*/
	h_dc_check = mb_hpf->h_c0 + 2*mb_hpf->h_c1 + 2*mb_hpf->h_c2 + 2*mb_hpf->h_c3;
	v_dc_check = mb_hpf->v_c0 + 2*mb_hpf->v_c1 + 2*mb_hpf->v_c2;
	/*printk("MB=%d,%d,%d,%d,%d,%d,%d,,%d,%d,%d,%d\n", mb_hpf->h_c0,mb_hpf->h_c1,mb_hpf->h_c2,mb_hpf->h_c3,mb_hpf->h_c4,mb_hpf->h_c5,mb_hpf->h_c6
		,mb_hpf->v_c0,mb_hpf->v_c1,mb_hpf->v_c2);*/
	if ((h_dc_check == 0) && (v_dc_check == 0) && (mb_hpf->h_c0 != 0) && (mb_hpf->v_c0 != 0)) {
		MBPK.MBPK_Ctrl.mb_peaking_en = mb_hpf->mb_peaking_en;
		MBPK.MBPK_H_Coef.MB_Hor_Filter_C0 = mb_hpf->h_c0;
		MBPK.MBPK_H_Coef.MB_Hor_Filter_C1 = mb_hpf->h_c1;
		MBPK.MBPK_H_Coef.MB_Hor_Filter_C2 = mb_hpf->h_c2;
		MBPK.MBPK_H_Coef.MB_Hor_Filter_C3 = mb_hpf->h_c3;
		MBPK.MBPK_V_Coef.MB_Ver_Filter_C0 = mb_hpf->v_c0;
		MBPK.MBPK_V_Coef.MB_Ver_Filter_C1 = mb_hpf->v_c1;
		MBPK.MBPK_V_Coef.MB_Ver_Filter_C2 = mb_hpf->v_c2;

		MBPK.MBPK_Ctrl.MB_H_En = mb_UI->mb_h_en;
		MBPK.MBPK_H_Table.MB_Gain_Pos = mb_UI->H_Gain_Pos;
		MBPK.MBPK_H_Table.MB_Gain_Neg = mb_UI->H_Gain_Neg;
		MBPK.MBPK_H_Table.MB_LV = mb_UI->H_Coring;
		MBPK.MBPK_H_Table.MB_Gain_Pos2 = mb_UI->H_Gain_Pos2;
		MBPK.MBPK_H_Table.MB_Gain_Neg2 = mb_UI->H_Gain_Neg2;
		MBPK.MBPK_H_Table.MB_HV_Neg = mb_UI->H_Range_Pos;
		MBPK.MBPK_H_Table.MB_HV_Pos = mb_UI->H_Range_Neg;
		MBPK.MBPK_H_Table.MB_D2_shift_bit = mb_UI->H_Edge_Shift;
		MBPK.MBPK_Ctrl.MB_V_En = mb_UI->mb_v_en;
		MBPK.MBPK_V_Table.MB_Gain_Pos_V = mb_UI->V_Gain_Pos;
		MBPK.MBPK_V_Table.MB_Gain_Neg_V = mb_UI->V_Gain_Neg;
		MBPK.MBPK_V_Table.MB_LV_V = mb_UI->V_Coring;
		MBPK.MBPK_V_Table.MB_Gain_Pos2_V = mb_UI->V_Gain_Pos2;
		MBPK.MBPK_V_Table.MB_Gain_Neg2_V = mb_UI->V_Gain_Neg2;
		MBPK.MBPK_V_Table.MB_HV_Pos_V = mb_UI->V_Range_Pos;
		MBPK.MBPK_V_Table.MB_HV_Neg_V = mb_UI->V_Range_Neg;
		MBPK.MBPK_V_Table.MB_D2_shift_bit_V = mb_UI->V_Edge_Shift;
	}
	/*==========set from DB END=============*/

	drvif_color_Set_MBSU_Table((VIP_MBPK_Table *)&MBPK);
}

void fwif_color_set_inv_gamma_encode_tv006(void)
{
	extern unsigned int final_GAMMA_R[512];
	extern unsigned int final_GAMMA_G[512];
	extern unsigned int final_GAMMA_B[512];

	drvif_color_Inv_Gamma_Curve_Write(final_GAMMA_R, GAMMA_CHANNEL_R);	/*R*/
	drvif_color_Inv_Gamma_Curve_Write(final_GAMMA_G, GAMMA_CHANNEL_G);	/*G*/
	drvif_color_Inv_Gamma_Curve_Write(final_GAMMA_B, GAMMA_CHANNEL_B);	/*B*/
}

void fwif_color_set_out_gamma_encode_tv006(void)
{
	extern unsigned int out_GAMMA_R[512];
	extern unsigned int out_GAMMA_G[512];
	extern unsigned int out_GAMMA_B[512];

	drvif_color_Out_Gamma_Curve_Write(out_GAMMA_R, GAMMA_CHANNEL_R);	/*R*/
	drvif_color_Out_Gamma_Curve_Write(out_GAMMA_G, GAMMA_CHANNEL_G);	/*G*/
	drvif_color_Out_Gamma_Curve_Write(out_GAMMA_B, GAMMA_CHANNEL_B);	/*B*/
}

void fwif_color_set_cds_table_tv006(unsigned char table_idx)
{
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	VIP_CDS_Table CDS_Table = {{0}};
	StructColorDataType *pTable = NULL;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	Scaler_Get_Display_info(&display, &src_idx);

	pTable = fwif_color_get_color_data(src_idx, 0);
	if (pTable == NULL)
		return;

	pTable->CDS_Table = table_idx;

	if (gVip_Table == NULL) {
		VIPprintf("~get vipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}

	if (table_idx < Sharp_table_num)	/* bypass = 255*/
	memcpy(&CDS_Table, &(gVip_Table->CDS_ini[table_idx]), sizeof(VIP_CDS_Table));

	if ((get_vsc_run_pc_mode()) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI))
		CDS_Table.CDS_Global_Ctrl.cds_enable = 0;

	drvif_color_set_cds_table((VIP_CDS_Table *) (&CDS_Table));
}

void fwif_color_set_DCC_Init_tv006(void)
{
	int i;
	signed int FreshContrastLUT[DCC_Curve_Node_MAX];

	for (i = 0; i < DCC_Curve_Node_MAX; i++)
		FreshContrastLUT[i] = i << 5;

	fwif_color_set_dcc_Curve_Write_tv006(FreshContrastLUT);
}
#if 0
unsigned char fwif_color_set_Gamut_3D_Lut_N9_to_N17_tv006(GAMUT_3D_LUT_9x9x9_T *pLut)
{
	GAMUT_3D_LUT_17x17x17_T *pLut_17_tmp = NULL;
	unsigned int *pLUT3D = NULL;
	int x, y, z, xp, yp, zp;
	int idx = -1;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();

	if (NULL == pLut)
		return FALSE;

	if (pLut->index >= LUT3D_TBL_Num)
		return FALSE;

	pLUT3D = gVip_Table->LUT_3D[pLut->index];

	pLut_17_tmp = (GAMUT_3D_LUT_17x17x17_T *)vip_malloc(sizeof(GAMUT_3D_LUT_17x17x17_T));
	if (pLut_17_tmp == NULL) {
		VIPprintf("%s malloc fail!!\n", __FUNCTION__);
		return FALSE;
	}

	/*interpolation r*/
	for (z = 0; z < 9; z++) {
		for (y = 0; y < 9; y++) {
			for (x = 0; x < 9; x++) {
				xp = x<<1; yp = y<<1; zp = z<<1;
				pLut_17_tmp->pt[xp][yp][zp].r_data = pLut->pt[x][y][z].r_data;
				pLut_17_tmp->pt[xp][yp][zp].g_data = pLut->pt[x][y][z].g_data;
				pLut_17_tmp->pt[xp][yp][zp].b_data = pLut->pt[x][y][z].b_data;
				if (x < 8) {
					pLut_17_tmp->pt[xp+1][yp][zp].r_data = (pLut->pt[x][y][z].r_data+pLut->pt[x+1][y][z].r_data)>>1;
					pLut_17_tmp->pt[xp+1][yp][zp].g_data = (pLut->pt[x][y][z].g_data+pLut->pt[x+1][y][z].g_data)>>1;
					pLut_17_tmp->pt[xp+1][yp][zp].b_data = (pLut->pt[x][y][z].b_data+pLut->pt[x+1][y][z].b_data)>>1;
				}
			}
		}
	}

	/*interpolation g*/
	for (z = 0; z < 9; z++) {
		for (xp = 0; xp < 17; xp++) {
			for (y = 0; y < 8; y++) {
				yp = y<<1; zp = z<<1;
				pLut_17_tmp->pt[xp][yp+1][zp].r_data = (pLut_17_tmp->pt[xp][yp][zp].r_data+pLut_17_tmp->pt[xp][yp+2][zp].r_data)>>1;
				pLut_17_tmp->pt[xp][yp+1][zp].g_data = (pLut_17_tmp->pt[xp][yp][zp].g_data+pLut_17_tmp->pt[xp][yp+2][zp].g_data)>>1;
				pLut_17_tmp->pt[xp][yp+1][zp].b_data = (pLut_17_tmp->pt[xp][yp][zp].b_data+pLut_17_tmp->pt[xp][yp+2][zp].b_data)>>1;
			}
		}
	}

	/*interpolation b*/
	for (xp = 0; xp < 17; xp++) {
		for (yp = 0; yp < 17; yp++) {
			for (z = 0; z < 8; z++) {
				zp = z<<1;
				pLut_17_tmp->pt[xp][yp][zp+1].r_data = (pLut_17_tmp->pt[xp][yp][zp].r_data+pLut_17_tmp->pt[xp][yp][zp+2].r_data)>>1;
				pLut_17_tmp->pt[xp][yp][zp+1].g_data = (pLut_17_tmp->pt[xp][yp][zp].g_data+pLut_17_tmp->pt[xp][yp][zp+2].g_data)>>1;
				pLut_17_tmp->pt[xp][yp][zp+1].b_data = (pLut_17_tmp->pt[xp][yp][zp].b_data+pLut_17_tmp->pt[xp][yp][zp+2].b_data)>>1;
			}
		}
	}

	for (zp = 0; zp < 17; zp++) {
		for (yp = 0; yp < 17; yp++) {
			for (xp = 0; xp < 17; xp++) {
				idx++;
				pLUT3D[idx<<1] = (pLut_17_tmp->pt[xp][yp][zp].r_data<<16)+pLut_17_tmp->pt[xp][yp][zp].g_data;
				pLUT3D[(idx<<1)+1] = pLut_17_tmp->pt[xp][yp][zp].b_data;
			}
		}
	}

	vip_free(pLut_17_tmp);
	return TRUE;
}
#endif

unsigned char fwif_color_set_Gamut_3D_Lut_buffer_tv006(GAMUT_3D_LUT_17x17x17_T *pLut)
{
	unsigned int *pLUT3D = NULL;
	int idx;
	SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	//extern unsigned int LUT_3D[LUT3D_TBL_Num][LUT3D_TBL_ITEM];

	if (NULL == pLut || NULL == gVip_Table)
		return FALSE;

	if (pLut->index >= LUT3D_TBL_Num)
		return FALSE;

	pLUT3D = gVip_Table->LUT_3D[pLut->index];

	for (idx = 0; idx < 4913; idx++) {
		pLUT3D[idx<<1] = (pLut->pt[idx].r_data<<16)+pLut->pt[idx].g_data;
		pLUT3D[(idx<<1)+1] = pLut->pt[idx].b_data;
	}

	return TRUE;
}

unsigned char fwif_color_set_Gamut_3D_Lut_ext_buffer_tv006(GAMUT_3D_LUT_17x17x17_T *pLut)
{
/* //no more used
	unsigned int *pLUT3D = NULL;
	extern unsigned int LG_LUT_3D_EXTEND[LUT3D_EXTEND_TBL_Num][LUT3D_TBL_ITEM];
	int idx;

	if (NULL == pLut)
		return FALSE;

	pLUT3D = LG_LUT_3D_EXTEND[pLut->index-IDX_3DLUT_EXT_TABLE_START];

	for (idx = 0; idx < 4913; idx++) {
		pLUT3D[idx<<1] = (pLut->pt[idx].r_data<<16)+pLut->pt[idx].g_data;
		pLUT3D[(idx<<1)+1] = pLut->pt[idx].b_data;
	}
*/
	return TRUE;
}

void fwif_color_set_color_temp_tv006(unsigned char enable, unsigned short Red_Con, unsigned short Green_Con, unsigned short Blue_Con,
	short Red_Bri, short Green_Bri, short Blue_Bri, unsigned char waitSync)
{
	if (waitSync)
		fwif_color_WaitFor_SYNC_START_UZUDTG();
	drvif_color_set_color_temp(enable, Red_Con, Green_Con, Blue_Con, Red_Bri, Green_Bri, Blue_Bri);
}

char fwif_color_set_bt2020_Handler_tv006(unsigned char display, unsigned char Enable_Flag, unsigned char bt2020_Mode)
{
	unsigned char InvGamma_TBL_Sel;//, CM_TBL_Select;
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
		//CM_TBL_Select = 1;
		drv_vipCSMatrix_t.CTRL_ITEM.Bt2020_En_TBL1 = 1;
		drv_vipCSMatrix_t.CTRL_ITEM.CoefByY_En = 0;	/* refference to spec*/
		drv_vipCSMatrix_t.CTRL_ITEM.UVOffset_En = 0;	/* refference to spec*/
	} else {
		InvGamma_TBL_Sel = 0;
		invGammaTBL = &LinearInvGamma[0];
		//CM_TBL_Select = 0;
		drv_vipCSMatrix_t.CTRL_ITEM.Bt2020_En_TBL1 = 0;
		drv_vipCSMatrix_t.CTRL_ITEM.CoefByY_En = 2; //force Coef by Sat for tv006
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
	/*fwif_color_setCon_Bri_Color_Tint_inYUV2RGB_from_OSD_Gain(0, display, VIP_system_info_structure_table);*/
	fwif_color_setCon_Bri_Color_Tint_tv006(display, VIP_system_info_structure_table);

	/* set Bt2020 Constant mode enable */
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_YUV2RGB_Base_Ctrl, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_CoefByY_ctrl, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	drvif_color_setYUV2RGB_CSMatrix(&drv_vipCSMatrix_t, VIP_CSMatrix_UVOffset_ctrl, VIP_YUV2RGB_Y_Seg_0, SLR_MAIN_DISPLAY, VIP_Table_Select_1);
	return 0;
}


unsigned char fwif_color_vpq_stop_ioctl(VPQ_IOCTL_STOP_RUN id, ACCESS_MODE mode, unsigned char isStop)
{
	static unsigned char ioctl_stop[STOP_VPQ_IOC_MAX] = {0};

	if (mode == ACCESS_MODE_SET) {
		ioctl_stop[id] = isStop;
	}

	return ioctl_stop[id];
}

unsigned char fwif_color_get_LD_GetAPL_TV006(UINT16* BLValue)
{
	unsigned char ret = 1;
	if(BLValue == NULL) {
		VIPprintf(" [Warning] fw GetAPL is not correct, BLValue is NULL\n");
		ret = 0;
		return ret;
	}

	ret = drvif_color_get_LD_GetAPL_TV006(BLValue);
	return ret;
}

void fwif_color_SetDataFormatHandler_tv006(void)
{
	unsigned char en_422to444, InputSrcGetType, InputSrcGetFrom;
	unsigned short mode, channel;
	SCALER_DISP_CHANNEL display;
	unsigned char nMode = 0;
	_system_setting_info *system_info_struct = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	mode = Scaler_DispGetInputInfo(SLR_INPUT_MODE_CURR);
	display = (SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY);
	channel = Scaler_DispGetInputInfo(SLR_INPUT_CHANNEL);
	en_422to444 = (unsigned char)Scaler_DispGetStatus(display, SLR_DISP_422);
	InputSrcGetType = Scaler_InputSrcGetType(display);

	if (fwif_color_get_force_run_i3ddma_enable(display)) {
		fwif_color_vsc_to_scaler_src_mapping(Get_DisplayMode_Src(display), &InputSrcGetType, &InputSrcGetFrom);
		mode = fwif_color_get_cur_input_timing_mode(display);
	}

	if (!(get_vsc_run_adaptive_stream(SLR_MAIN_DISPLAY)))
		drvif_color_set422to444(channel, en_422to444);

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

	fwif_color_colorspacergb2yuvtransfer(channel, InputSrcGetType, nMode, 1, 0);
	fwif_color_setCon_Bri_Color_Tint_tv006(0, system_info_struct);
}

#if 0
void fwif_color_SetDataFormatHandler_tv006_Seamless(unsigned char HDR_Mode)
{
	unsigned char InputSrcGetType = Scaler_InputSrcGetType(SLR_MAIN_DISPLAY);
	_system_setting_info *system_info_struct = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	fwif_color_colorspacergb2yuvtransfer_Seamless(HDR_Mode, InputSrcGetType, 0);
	fwif_color_setCon_Bri_Color_Tint_tv006(0, system_info_struct);
}
#endif

void fwif_color_3d_lut_init_tv006(void)
{
/* //no more used
	int i;
	extern GAMUT_3D_LUT_17x17x17_T gVip_3DLut_TV006[IDX_3DLUT_MAX];

	for (i = 0; i < IDX_3DLUT_BIG_TABLE_MAX; i++) {
		fwif_color_set_Gamut_3D_Lut_buffer_tv006(&gVip_3DLut_TV006[i]);
	}

	for (i = IDX_3DLUT_EXT_TABLE_START; i < IDX_3DLUT_MAX; i++) {
		fwif_color_set_Gamut_3D_Lut_ext_buffer_tv006(&gVip_3DLut_TV006[i]);
	}
*/
}

unsigned char fwif_color_3d_lut_access_index(ACCESS_MODE access, unsigned char setIdx)
{
	static unsigned char curIdx = 0;

	if (access == ACCESS_MODE_GET)
		return curIdx;
	else if (access == ACCESS_MODE_SET)
		curIdx = setIdx;

	return 0;
}

void fwif_color_set_cinema_mode_en(unsigned char bEn)
{
	bCinemaModeEn = bEn;

	return;
}

unsigned char fwif_color_get_cinema_mode_en(void)
{
	return bCinemaModeEn;
}

int fwif_color_set_dcc_force_write_flag_RPC(unsigned sendRPC, unsigned int force_write)
{
	int ret;

	fwif_color_ChangeUINT32Endian(&force_write, 1, 0);

#ifdef CONFIG_ARM64
	memcpy_toio((unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SET_DCC_FORCE_UPDATE), &force_write, sizeof(unsigned int));
#else
	memcpy((unsigned int*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_SET_DCC_FORCE_UPDATE), &force_write, sizeof(unsigned int));
#endif

	if (!sendRPC)
		return 0;

	ret = Scaler_SendRPC(SCALERIOC_VIP_SET_DCC_FORCE_UPDATE, 0, 0);
	if (0 !=  ret) {
		VIPprintf("ret = %d, set SCALERIOC_VIP_SET_DCC_FORCE_UPDATE to driver fail !!!\n", ret);
	}

	return ret;
}

void fwif_color_set_SiW_ModeChange_TV006(UINT8 *pFwData, UINT32 size)
{
	drvif_color_set_SiW_ModeChange(*pFwData);
}
#ifdef CONFIG_SCALER_ENABLE_V4L2
void fwif_color_set_Mplus_ModeChange_TV006(enum v4l2_ext_vbe_mplus_mode mode_sel)
{


	/* -------------------------------------------
    V4L2_EXT_VBE_MPLUS_MPLUS_MODE0 = 0,
    V4L2_EXT_VBE_MPLUS_MPLUS_MODE1,
    V4L2_EXT_VBE_MPLUS_MPLUS_MODE2,
    V4L2_EXT_VBE_MPLUS_MPLUS_MODE3,
    V4L2_EXT_VBE_MPLUS_MPLUS_MODE4,
    V4L2_EXT_VBE_MPLUS_MPLUS_MODE5,
    V4L2_EXT_VBE_MPLUS_MPLUS_MODE6,
    V4L2_EXT_VBE_MPLUS_MPLUS_MODE7,
    V4L2_EXT_VBE_MPLUS_MPLUS_MODE8,
    V4L2_EXT_VBE_MPLUS_MPLUS_MODE9,
    V4L2_EXT_VBE_MPLUS_MPLUS_MODE10,
    V4L2_EXT_VBE_MPLUS_MPLUS_MODEMAX
	------------------------------------------- */
	DRV_Mplus_ModeChange data={0};

	switch (mode_sel) {

		case V4L2_EXT_VBE_MPLUS_MPLUS_MODE0 :
		case V4L2_EXT_VBE_MPLUS_MPLUS_MODEMAX:
		default:
			data.wpr_on = 1;
			data.opc_on = 0;
			data.mce_on = 1;
			data.mode_sel = 1;	/* 001 */
			break;
		case V4L2_EXT_VBE_MPLUS_MPLUS_MODE1 :
			data.wpr_on = 1;
			data.opc_on = 0;
			data.mce_on = 0;
			data.mode_sel = 2;	/* 002 */
			break;
		case V4L2_EXT_VBE_MPLUS_MPLUS_MODE2 :
			data.wpr_on = 1;
			data.opc_on = 0;
			data.mce_on = 0;
			data.mode_sel = 0;	/* 000 */
			break;
		case V4L2_EXT_VBE_MPLUS_MPLUS_MODE3 :
			data.wpr_on = 0;
			data.opc_on = 0;
			data.mce_on = 0;
			data.mode_sel = 0;	/* 000 */
			break;
		case V4L2_EXT_VBE_MPLUS_MPLUS_MODE4 :
			data.wpr_on = 1;
			data.opc_on = 0;
			data.mce_on = 0;
			data.mode_sel = 1;	/* 001 */
			break;
		case  V4L2_EXT_VBE_MPLUS_MPLUS_MODE5:
			data.wpr_on = 1;
			data.opc_on = 0;
			data.mce_on = 0;
			data.mode_sel = 3;	/* 011 */
			break;
		case V4L2_EXT_VBE_MPLUS_MPLUS_MODE6:
			data.wpr_on = 1;
			data.opc_on = 0;
			data.mce_on = 0;
			data.mode_sel = 3;	/* 011 */
			break;
		case V4L2_EXT_VBE_MPLUS_MPLUS_MODE7:
			data.wpr_on = 1;
			data.opc_on = 0;
			data.mce_on = 0;
			data.mode_sel = 4;	/* 100 */
			break;
		case V4L2_EXT_VBE_MPLUS_MPLUS_MODE8:
			data.wpr_on = 1;
			data.opc_on = 0;
			data.mce_on = 0;
			data.mode_sel = 5;	/* 101 */
			break;
		case V4L2_EXT_VBE_MPLUS_MPLUS_MODE9:
			data.wpr_on = 1;
			data.opc_on = 0;
			data.mce_on = 0;
			data.mode_sel = 6;	/* 110 */
			break;
		case V4L2_EXT_VBE_MPLUS_MPLUS_MODE10:
			data.wpr_on = 1;
			data.opc_on = 0;
			data.mce_on = 0;
			data.mode_sel = 7;	/* 111 */
			break;
	}
	drvif_color_set_Mplus_ModeChange((DRV_Mplus_ModeChange *) &data);
}
#endif
//UINT32 pre_Eotf_table32[EOTF_size] = {0};
//UINT16 pre_Oetf_table16[OETF_size] = {0};
//UINT8 Eotf_NOT_set = 0;
//UINT8 Oetf_NOT_set = 0;
UINT16 HDR_3DLUT_Compact[HDR_24x24x24_size*3] =  {0};
static char HDR_3DLUT_flow_status[HDR_DMA_Status_MAX] = {0};
extern unsigned char g_HDR3DLUTForceWrite;
static unsigned char gDB_done_check_in_DM2_Hist_AutoMode = 0;
char fwif_color_Wait_3dLUT_Dimension_DB_done(void)
{
#ifdef isSupport_HDR_3dLUT_24x24x24
	unsigned char db_en, db_lv, db_apply;
	unsigned int count = 0x600;

	do {
		drvif_Get_DM2_DB_Status(&db_en, &db_lv, &db_apply);
		count--;
		usleep_range(50, 100);
	} while((db_apply == 1) && (gDB_done_check_in_DM2_Hist_AutoMode == 1) && (count>=10));

	if (db_apply == 0)	// register is 0,clear flag;
		gDB_done_check_in_DM2_Hist_AutoMode = 0;

	if (count <= 10) {
		rtd_pr_vpq_emerg("can not WaitDM_DB_Apply_Clean, db_apply=%d, g3dLUT_Dimension_DB_done_flag=%d,\n",
			db_apply, gDB_done_check_in_DM2_Hist_AutoMode);
		return -1;
	}
#else
		gDB_done_check_in_DM2_Hist_AutoMode = 0;
#endif
	return 0;

}

char fwif_color_set_DM_HDR_3dLUT_24x24x24_16_TV006(void *p, unsigned char check_err)
{
#if 0	/* use DMA driver*/
	extern struct semaphore VPQ_DM_3DLUT_Semaphore;
	extern unsigned char g_HDR3DLUTForceWrite;
	extern dm_dm_submodule_enable_RBUS dm_submodule_temp;
	PQ_HDR_3dLUT_16_PARAM *pParam = (PQ_HDR_3dLUT_16_PARAM *)p;
	unsigned char enable = pParam->c3dLUT_16_enable;
	unsigned short *pArray = pParam->a3dLUT_16_pArray;
	char ret = 0;
	if (fwif_HDR_compare_3DLUT(p, NEW_24x24x24) != 1) {
		return 0;
	}
	down(&VPQ_DM_3DLUT_Semaphore);
	if (enable == 1) {
		//K3LG-1046 noise screen
		if(((get_OTT_HDR_mode() == HDR_DOLBY_COMPOSER) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC)) ||
			((get_HDMI_HDR_mode() == HDR_DOLBY_HDMI) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)))
		{
			g_HDR3DLUTForceWrite = 1;
			dm_submodule_temp.b0501_enable = 0; /* 24x24x24 CSC3 in VSC*/

		} else {
			fwif_color_DM2_3D_LUT_BIT_Change(pArray, &HDR_3DLUT_Compact[0], NEW_24x24x24);
			ret = drvif_Set_DM_HDR_3dLUT_24x24x24_16(enable, &HDR_3DLUT_Compact[0]);
			if (ret < 0) {
				g_HDR3DLUTForceWrite = 1;
				dm_submodule_temp.b0501_enable = 0; /* enable 24x24x24 CSC3 in VSC*/
			} else {
				g_HDR3DLUTForceWrite = 0;
				dm_submodule_temp.b0501_enable = 1; /* 24x24x24 CSC3 in VSC*/
			}
		}
	} else {
				g_HDR3DLUTForceWrite = 0;
				dm_submodule_temp.b0501_enable = 0; /* 24x24x24 CSC3 in VSC*/
	}
	up(&VPQ_DM_3DLUT_Semaphore);
	return 1;
#else
	//extern UINT16 HDR_3DLUT_Compact[HDR_24x24x24_size*3];
	//extern struct semaphore VPQ_DM_3DLUT_Semaphore;

	PQ_HDR_3dLUT_16_PARAM *pParam = (PQ_HDR_3dLUT_16_PARAM *)p;
	unsigned char enable = pParam->c3dLUT_16_enable;
	unsigned short *pArray = pParam->a3dLUT_16_pArray;
	unsigned char db_en, db_lv, db_apply;
	unsigned char b05_enable, dimension;
	char ret = 0, ret_apply = 0;
	unsigned char check_times = 0, check_times_max = 30;
	static unsigned char printk_cnt = 0;

	down(&VPQ_DM_DMA_TBL_Semaphore);
	g_HDR3DLUTForceWrite = 1; // always force write.
	if (enable == 0 || pArray == NULL) {
		drvif_Set_DM_HDR_3dLUT_24x24x24_16_By_DMA(0, NULL, 0, NULL, 0);
		ret = 0;
	} else if ((drvif_DM_HDR10_enable_Get() == 0) || ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE))) {
		if (fwif_HDR_compare_3DLUT(p, NEW_24x24x24) != 1) {
			rtd_pr_vpq_info("DM_3dLUT_DMA, same\n");
			HDR_3DLUT_flow_status[printk_cnt] = 1;
			ret = 1;
		} else {
			fwif_color_DM2_3D_LUT_BIT_Change(pArray, &HDR_3DLUT_Compact[0], NEW_24x24x24);

			//drvif_Get_DM2_DB_Status(&db_en, &db_lv, &db_apply);
			//drvif_Get_DM2_B05_Enable_Status(&b05_enable, &dimension);
			dimension = drvif_Get_DM2_B05_Dimension_clk_flip_flop();
			if (dimension == 0) {
				drvif_Set_DM_HDR_3dLUT_Dimension(1);
				ret_apply = fwif_color_Wait_3dLUT_Dimension_DB_done();
			} else {
				drvif_Set_DM_HDR_3dLUT_Dimension(1);
				ret_apply = 1;
			}

			if (ret_apply < 0) {
				drvif_Set_DM_HDR_3dLUT_24x24x24_16_By_DMA(0, NULL, 0, NULL, 0);
				rtd_pr_vpq_emerg("DM_3dLUT_DMA, dolby apply fail in sram.\n");
				HDR_3DLUT_flow_status[printk_cnt] = -21;
				ret = -1;
			} else {
				ret = drvif_Set_DM_HDR_3dLUT_24x24x24_16(enable, &HDR_3DLUT_Compact[0]);
			}
			if (ret < 0) {
				drvif_Set_DM_HDR_3dLUT_24x24x24_16_By_DMA(0, NULL, 0, NULL, 0);
				rtd_pr_vpq_emerg("DM_3dLUT_DMA, Sram can't write W\n");
				HDR_3DLUT_flow_status[printk_cnt] = -1;
				ret = -1;
			} else {
				HDR_3DLUT_flow_status[printk_cnt] = 0;
				ret = 0;
			}
		}
	} else {
		if (check_err == 1) {	/* check err status while DMA write */
			do {
				drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Reset();
				check_times++;
			} while((drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Check(0) != 0) && (check_times<check_times_max));

			if (check_times >= check_times_max) {
				rtd_pr_vpq_emerg("DM_3dLUT_DMA, can't reset DM2 DMA Err status, check times = %d,\n", check_times);
				drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Check(1);
				HDR_3DLUT_flow_status[printk_cnt] = -10;
				ret = -1;
			} else {	/* do not check error status, other handle flow will check err status */
				drvif_Get_DM2_DB_Status(&db_en, &db_lv, &db_apply);
				drvif_Get_DM2_B05_Enable_Status(&b05_enable, &dimension);
				if (db_apply == 1 || dimension == 0) {
					drvif_Set_DM_HDR_3dLUT_Dimension(1);
					gDB_done_check_in_DM2_Hist_AutoMode = 1;
					ret_apply = fwif_color_Wait_3dLUT_Dimension_DB_done();

				} else {
					ret_apply = 1;
				}

				if (ret_apply < 0) {
					drvif_Set_DM_HDR_3dLUT_24x24x24_16_By_DMA(0, NULL, 0, NULL, 0);
					rtd_pr_vpq_emerg("DM_3dLUT_DMA, dolby apply fail, db_apply=%d, dim=%d.\n", db_apply, dimension);
					HDR_3DLUT_flow_status[printk_cnt] = -11;
					ret = -1;
				} else {
					if (fwif_HDR_compare_3DLUT(p, NEW_24x24x24) != 1) {
						rtd_pr_vpq_info("DM_3dLUT_DMA, same1\n");
						HDR_3DLUT_flow_status[printk_cnt] = 10;
						ret = 1;
					} else {
						fwif_color_DM2_3D_LUT_BIT_Change(pArray, &HDR_3DLUT_Compact[0], NEW_24x24x24);
						ret = fwif_color_DM2_3D_24x24x24_LUT_Set_By_DMA(enable, &HDR_3DLUT_Compact[0]);
						if (drvif_DM2_Wait_for_DMA_Apply_Done() < 0) {
							rtd_pr_vpq_emerg("DM_3dLUT_DMA, DMA wait not finish.\n");
							HDR_3DLUT_flow_status[printk_cnt] = -12;
							drvif_Set_DM_HDR_3dLUT_24x24x24_16_By_DMA(0, NULL, 0, NULL, 0);
							ret = -1;
						} else {
							if (drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Check(1) != 0) {
								drvif_Set_DM_HDR_3dLUT_24x24x24_16_By_DMA(0, NULL, 0, NULL, 0);
								rtd_pr_vpq_emerg("DM_3dLUT_DMA, error status ~= 0..\n");
								HDR_3DLUT_flow_status[printk_cnt] = -13;
								ret = -1;
							} else {
								//rtd_pr_vpq_info("DM_3dLUT_DMA, done DMA\n");
								HDR_3DLUT_flow_status[printk_cnt] = 11;
							}
						}
					}
				}
			}

		} else {	/* do not check error status, other handle flow will check err status */
			drvif_Get_DM2_DB_Status(&db_en, &db_lv, &db_apply);
			drvif_Get_DM2_B05_Enable_Status(&b05_enable, &dimension);
			if (db_apply == 1 || dimension == 0) {
				drvif_Set_DM_HDR_3dLUT_Dimension(1);
				gDB_done_check_in_DM2_Hist_AutoMode = 1;
				ret_apply = fwif_color_Wait_3dLUT_Dimension_DB_done();
			} else {
				ret_apply = 1;
			}

			if (ret_apply < 0) {
				drvif_Set_DM_HDR_3dLUT_24x24x24_16_By_DMA(0, NULL, 0, NULL, 0);
				rtd_pr_vpq_emerg("DM_3dLUT_DMA, dolby apply fail.\n");
				HDR_3DLUT_flow_status[printk_cnt] = -20;
				ret = -1;
			} else {
				if (fwif_HDR_compare_3DLUT(p, NEW_24x24x24) != 1) {
					//rtd_pr_vpq_info("DM_3dLUT_DMA, HDR 3D LUT same\n");
					HDR_3DLUT_flow_status[printk_cnt] = 20;
					ret = 1;
				} else {
					fwif_color_DM2_3D_LUT_BIT_Change(pArray, &HDR_3DLUT_Compact[0], NEW_24x24x24);
					ret = fwif_color_DM2_3D_24x24x24_LUT_Set_By_DMA(enable, &HDR_3DLUT_Compact[0]);
					rtd_pr_vpq_info("DM_3dLUT_DMA, done DMA 1\n");
					HDR_3DLUT_flow_status[printk_cnt] = 21;
				}
			}
		}
	}
	printk_cnt++;
	if (printk_cnt >= HDR_DMA_Status_MAX) {
		printk_cnt = 0;
		/*
		rtd_pr_vpq_info("DM_3dLUT_DMA, status={%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",
		HDR_3DLUT_flow_status[0],HDR_3DLUT_flow_status[1],HDR_3DLUT_flow_status[2],HDR_3DLUT_flow_status[3],HDR_3DLUT_flow_status[4],
		HDR_3DLUT_flow_status[5],HDR_3DLUT_flow_status[6],HDR_3DLUT_flow_status[7],HDR_3DLUT_flow_status[8],HDR_3DLUT_flow_status[9]);
		rtd_pr_vpq_info("DM_3dLUT_DMA, status={%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",
		HDR_3DLUT_flow_status[10],HDR_3DLUT_flow_status[11],HDR_3DLUT_flow_status[12],HDR_3DLUT_flow_status[13],HDR_3DLUT_flow_status[14],
		HDR_3DLUT_flow_status[15],HDR_3DLUT_flow_status[16],HDR_3DLUT_flow_status[17],HDR_3DLUT_flow_status[18],HDR_3DLUT_flow_status[19]);
		*/
	}

	up(&VPQ_DM_DMA_TBL_Semaphore);
	return ret;
#endif
}

char fwif_color_set_DM_HDR_3dLUT_17x17x17_16_TV006(void *p, unsigned char check_err)
{
#if 0	/* use DMA driver*/
	extern struct semaphore VPQ_DM_3DLUT_Semaphore;
	extern unsigned char g_HDR3DLUTForceWrite;
	extern dm_dm_submodule_enable_RBUS dm_submodule_temp;
	PQ_HDR_3dLUT_16_PARAM *pParam = (PQ_HDR_3dLUT_16_PARAM *)p;
	unsigned char enable = pParam->c3dLUT_16_enable;
	unsigned short *pArray = pParam->a3dLUT_16_pArray;
	char ret = 0;
	if (fwif_HDR_compare_3DLUT(p, NEW_24x24x24) != 1) {
		return 0;
	}
	down(&VPQ_DM_3DLUT_Semaphore);
	if (enable == 1) {
		//K3LG-1046 noise screen
		if(((get_OTT_HDR_mode() == HDR_DOLBY_COMPOSER) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC)) ||
			((get_HDMI_HDR_mode() == HDR_DOLBY_HDMI) && (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)))
		{
			g_HDR3DLUTForceWrite = 1;
			dm_submodule_temp.b0501_enable = 0; /* 24x24x24 CSC3 in VSC*/

		} else {
			fwif_color_DM2_3D_LUT_BIT_Change(pArray, &HDR_3DLUT_Compact[0], NEW_24x24x24);
			ret = drvif_Set_DM_HDR_3dLUT_24x24x24_16(enable, &HDR_3DLUT_Compact[0]);
			if (ret < 0) {
				g_HDR3DLUTForceWrite = 1;
				dm_submodule_temp.b0501_enable = 0; /* enable 24x24x24 CSC3 in VSC*/
			} else {
				g_HDR3DLUTForceWrite = 0;
				dm_submodule_temp.b0501_enable = 1; /* 24x24x24 CSC3 in VSC*/
			}
		}
	} else {
				g_HDR3DLUTForceWrite = 0;
				dm_submodule_temp.b0501_enable = 0; /* 24x24x24 CSC3 in VSC*/
	}
	up(&VPQ_DM_3DLUT_Semaphore);
	return 1;
#else
	//extern UINT16 HDR_3DLUT_Compact[HDR_24x24x24_size*3];
	//extern struct semaphore VPQ_DM_3DLUT_Semaphore;

	PQ_HDR_3dLUT_16_PARAM *pParam;// = (PQ_HDR_3dLUT_16_PARAM *)p;
	unsigned char enable;// = pParam->c3dLUT_16_enable;
	unsigned short *pArray;// = pParam->a3dLUT_16_pArray;
	unsigned char db_en, db_lv, db_apply;
	unsigned char b05_enable, dimension;
	char ret = 0, ret_apply = 0;
	unsigned char check_times = 0, check_times_max = 30;
	static unsigned char printk_cnt = 0;

	if (p == NULL) {
		enable = 0;
		pArray = NULL;
	} else {
		pParam = (PQ_HDR_3dLUT_16_PARAM *)p;
		enable = pParam->c3dLUT_16_enable;
		pArray = pParam->a3dLUT_16_pArray;

	}

	down(&VPQ_DM_DMA_TBL_Semaphore);
	g_HDR3DLUTForceWrite = 1; // always force write.
	if (enable == 0 || pArray == NULL) {
		drvif_Set_DM_HDR_3dLUT_17x17x17_16_By_DMA(0, NULL, 0, NULL, 0);
		rtd_pr_vpq_info("DM_3dLUT_DMA, off\n");
		ret = 0;
	} else if (check_err == 2) {
		fwif_HDR_compare_3DLUT(p, OLD_17x17x17);
		rtd_pr_vpq_info("DM_3dLUT_DMA, copy\n");
		ret = -2;
	} else if ((drvif_DM_HDR10_enable_Get() == 0) || ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE))) {
		if (fwif_HDR_compare_3DLUT(p, OLD_17x17x17) != 1) {
			rtd_pr_vpq_info("DM_3dLUT_DMA, same\n");
			HDR_3DLUT_flow_status[printk_cnt] = 1;
			ret = 1;
		} else {
			fwif_color_DM2_3D_LUT_BIT_Change(pArray, &HDR_3DLUT_Compact[0], OLD_17x17x17);

			//drvif_Get_DM2_DB_Status(&db_en, &db_lv, &db_apply);
			//drvif_Get_DM2_B05_Enable_Status(&b05_enable, &dimension);
			dimension = drvif_Get_DM2_B05_Dimension_clk_flip_flop();
			if (dimension == 0) {
				drvif_Set_DM_HDR_3dLUT_Dimension(1);
				ret_apply = fwif_color_Wait_3dLUT_Dimension_DB_done();
			} else {
				drvif_Set_DM_HDR_3dLUT_Dimension(1);
				ret_apply = 1;
			}

			if (ret_apply < 0) {
				drvif_Set_DM_HDR_3dLUT_17x17x17_16_By_DMA(0, NULL, 0, NULL, 0);
				rtd_pr_vpq_emerg("DM_3dLUT_DMA, dolby apply fail in sram.\n");
				HDR_3DLUT_flow_status[printk_cnt] = -21;
				ret = -1;
			} else {
				ret = drvif_DM_HDR_3dLUT_16(enable, &HDR_3DLUT_Compact[0]);
			}
			if (ret < 0) {
				drvif_Set_DM_HDR_3dLUT_17x17x17_16_By_DMA(0, NULL, 0, NULL, 0);
				rtd_pr_vpq_emerg("DM_3dLUT_DMA, Sram can't write W\n");
				HDR_3DLUT_flow_status[printk_cnt] = -1;
				ret = -1;
			} else {
				HDR_3DLUT_flow_status[printk_cnt] = 0;
				ret = 0;
			}
		}
	} else {
		if (check_err == 1) {	/* check err status while DMA write */
			do {
				drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Reset();
				check_times++;
			} while((drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Check(0) != 0) && (check_times<check_times_max));

			if (check_times >= check_times_max) {
				rtd_pr_vpq_emerg("DM_3dLUT_DMA, can't reset DM2 DMA Err status, check times = %d,\n", check_times);
				drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Check(1);
				HDR_3DLUT_flow_status[printk_cnt] = -10;
				ret = -1;
			} else {	/* do not check error status, other handle flow will check err status */
				drvif_Get_DM2_DB_Status(&db_en, &db_lv, &db_apply);
				drvif_Get_DM2_B05_Enable_Status(&b05_enable, &dimension);
				if (db_apply == 1 || dimension == 0) {
					drvif_Set_DM_HDR_3dLUT_Dimension(1);
					gDB_done_check_in_DM2_Hist_AutoMode = 1;
					ret_apply = fwif_color_Wait_3dLUT_Dimension_DB_done();

				} else {
					ret_apply = 1;
				}

				if (ret_apply < 0) {
					drvif_Set_DM_HDR_3dLUT_17x17x17_16_By_DMA(0, NULL, 0, NULL, 0);
					rtd_pr_vpq_emerg("DM_3dLUT_DMA, dolby apply fail, db_apply=%d, dim=%d.\n", db_apply, dimension);
					HDR_3DLUT_flow_status[printk_cnt] = -11;
					ret = -1;
				} else {
					if (fwif_HDR_compare_3DLUT(p, OLD_17x17x17) != 1) {
						rtd_pr_vpq_info("DM_3dLUT_DMA, same1\n");
						HDR_3DLUT_flow_status[printk_cnt] = 10;
						ret = 1;
					} else {
						fwif_color_DM2_3D_LUT_BIT_Change(pArray, &HDR_3DLUT_Compact[0], OLD_17x17x17);
						ret = fwif_color_DM2_3D_17x17x17_LUT_Set_By_DMA(enable, &HDR_3DLUT_Compact[0]);
						if (drvif_DM2_Wait_for_DMA_Apply_Done() < 0) {
							rtd_pr_vpq_emerg("DM_3dLUT_DMA, DMA wait not finish.\n");
							HDR_3DLUT_flow_status[printk_cnt] = -12;
							drvif_Set_DM_HDR_3dLUT_17x17x17_16_By_DMA(0, NULL, 0, NULL, 0);
							ret = -1;
						} else {
							if (drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Check(1) != 0) {
								drvif_Set_DM_HDR_3dLUT_17x17x17_16_By_DMA(0, NULL, 0, NULL, 0);
								rtd_pr_vpq_emerg("DM_3dLUT_DMA, error status ~= 0..\n");
								HDR_3DLUT_flow_status[printk_cnt] = -13;
								ret = -1;
							} else {
								//rtd_pr_vpq_info("DM_3dLUT_DMA, done DMA\n");
								HDR_3DLUT_flow_status[printk_cnt] = 11;
							}
						}
					}
				}
			}

		} else {	/* do not check error status, other handle flow will check err status */
			drvif_Get_DM2_DB_Status(&db_en, &db_lv, &db_apply);
			drvif_Get_DM2_B05_Enable_Status(&b05_enable, &dimension);
			if (db_apply == 1 || dimension == 0) {
				drvif_Set_DM_HDR_3dLUT_Dimension(1);
				gDB_done_check_in_DM2_Hist_AutoMode = 1;
				ret_apply = fwif_color_Wait_3dLUT_Dimension_DB_done();
			} else {
				ret_apply = 1;
			}

			if (ret_apply < 0) {
				drvif_Set_DM_HDR_3dLUT_17x17x17_16_By_DMA(0, NULL, 0, NULL, 0);
				rtd_pr_vpq_emerg("DM_3dLUT_DMA, dolby apply fail.\n");
				HDR_3DLUT_flow_status[printk_cnt] = -20;
				ret = -1;
			} else {
				if (fwif_HDR_compare_3DLUT(p, OLD_17x17x17) != 1) {
					//rtd_pr_vpq_info("DM_3dLUT_DMA, HDR 3D LUT same\n");
					HDR_3DLUT_flow_status[printk_cnt] = 20;
					ret = 1;
				} else {
					fwif_color_DM2_3D_LUT_BIT_Change(pArray, &HDR_3DLUT_Compact[0], OLD_17x17x17);
					ret = fwif_color_DM2_3D_17x17x17_LUT_Set_By_DMA(enable, &HDR_3DLUT_Compact[0]);
					rtd_pr_vpq_info("DM_3dLUT_DMA, done DMA 1\n");
					HDR_3DLUT_flow_status[printk_cnt] = 21;
				}
			}
		}
	}
	printk_cnt++;
	if (printk_cnt >= HDR_DMA_Status_MAX) {
		printk_cnt = 0;
		/*
		rtd_pr_vpq_info("DM_3dLUT_DMA, status={%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",
		HDR_3DLUT_flow_status[0],HDR_3DLUT_flow_status[1],HDR_3DLUT_flow_status[2],HDR_3DLUT_flow_status[3],HDR_3DLUT_flow_status[4],
		HDR_3DLUT_flow_status[5],HDR_3DLUT_flow_status[6],HDR_3DLUT_flow_status[7],HDR_3DLUT_flow_status[8],HDR_3DLUT_flow_status[9]);
		rtd_pr_vpq_info("DM_3dLUT_DMA, status={%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",
		HDR_3DLUT_flow_status[10],HDR_3DLUT_flow_status[11],HDR_3DLUT_flow_status[12],HDR_3DLUT_flow_status[13],HDR_3DLUT_flow_status[14],
		HDR_3DLUT_flow_status[15],HDR_3DLUT_flow_status[16],HDR_3DLUT_flow_status[17],HDR_3DLUT_flow_status[18],HDR_3DLUT_flow_status[19]);
		*/
	}

	up(&VPQ_DM_DMA_TBL_Semaphore);
	return ret;
#endif
}

void fwif_color_set_HDR_Dolby_mode_Enable(unsigned char HDR_type)
{//Decide HDR setting
	//unsigned char directvo_hdr10_flag;

	//directvo_hdr10_flag =FALSE;

	if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
	{//no need to check dm connect status
		//HDMI HDR10
		drvif_DM_HDR_Vtop_set(1, 0, 0);
	}
	else if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC)
	{//no need to check dm connect status
#ifndef BUILD_QUICK_SHOW		
		if (HDR_type == HDR_DM_TECHNICOLOR) {
			if(drvif_HDR_get_VO_Decompress_Status(Get_DisplayMode_Port(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY))))
			{//Playback TC mode and compress mode
				drvif_DM_HDR_Vtop_set(9, 0, 1);
			}
			else
			{//Playback TC mode and no compress mode
				drvif_DM_HDR_Vtop_set(8, 0, 1);
			}
		} 
		else
#endif			
		{
    #if 0 // Merlin4 only dolby mode 3 (HDMI with compress mode) is be removed
			if(drvif_HDR_get_VO_Decompress_Status(Get_DisplayMode_Port(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY))))
			{//Playback HDR10 mode and compress mode
				drvif_DM_HDR_Vtop_set(1, 1, 0, 0);
			}
			else
    #endif
			{//Playback HDR10 mode and no compress mode
				drvif_DM_HDR_Vtop_set(1, 0, 0);
			}
		}
	}
}

//#define force_bypass_DM2_Hist_AutoMode
static unsigned char DM2_Hist_AutoMode_Flag = 0;
static unsigned char DM2_Hist_AutoMode_Counter = 0;
void fwif_color_set_DM2_Hist_AutoMode_Enable(unsigned char En)
{
#ifdef force_bypass_DM2_Hist_AutoMode
	En = 0;
#endif
	if (En == 1) {
		DM2_Hist_AutoMode_Flag = 1;
		DM2_Hist_AutoMode_Counter = 5;
		drvif_DM2_HDR_histogram_Auto_Mode_Init(1, 1);
	} else {
		DM2_Hist_AutoMode_Flag = 0;
		DM2_Hist_AutoMode_Counter = 0;
		drvif_DM2_HDR_histogram_Auto_Mode_Init(0, 1);
	}
}
unsigned char fwif_color_get_DM2_Hist_AutoMode_Enable_Flag(void)
{
	return DM2_Hist_AutoMode_Flag;
}

unsigned char fwif_color_get_DM2_Hist_AutoMode_Counter(void)
{
	return DM2_Hist_AutoMode_Counter;
}

void fwif_color_set_DM2_Hist_AutoMode_Enable_Flag(unsigned char En)
{
	if (En == 1)
		DM2_Hist_AutoMode_Flag = 1;
	else
		DM2_Hist_AutoMode_Flag = 0;
}

void fwif_color_set_DM2_Hist_AutoMode_Counter(unsigned char Val)
{
	DM2_Hist_AutoMode_Counter = Val;
}
char dm_hdr_path_sel_reg_diff_cnt = 0;
void fwif_color_DM2_HDR_histogram_Mode_set_ISR(unsigned char Mode, unsigned int dm_hdr_path_sel_reg_init)
{
#ifdef VIP_Support_DM_Hist_2Mode_toggle	
	typedef union
	{
	    RBus_UInt32 regValue;
	    struct{
	        RBus_UInt32  b0103_or_eotf:1;
	        RBus_UInt32  b0105_or_oetf:1;
	        RBus_UInt32  bbc_hdr_enable:1;
	        RBus_UInt32  b02b03_no_sram_mode:1;
	        RBus_UInt32  b05_position:2;
	        RBus_UInt32  hdr_hist_position:2;
	        RBus_UInt32  dm_b05_22bit_path:1;
	        RBus_UInt32  res1:23;
	    };
	} dm_hdr_path_sel_RBUS_temp;
	unsigned char db_en, db_lv, db_apply;
	_system_setting_info *system_info_struct = NULL;
	dm_hdr_path_sel_RBUS_temp  dm_hdr_path_sel_reg;
	dm_hdr_path_sel_RBUS_temp  dm_hdr_path_sel_reg_ini;

	system_info_struct = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (system_info_struct == NULL) {
		rtd_pr_vpq_emerg("system_info_struct NULL\n");
		return;
	}

	// set default setting for every case, while DM hist auto mode is active in data start isr, to avoid race condition with register "DM_HDR_PATH_SEL_reg"
	// will be active only in max_RGB, HDR10, HLG.
	if (system_info_struct->DolbyHDR_flag == 1) {
		dm_hdr_path_sel_reg.b0103_or_eotf = 0;
		dm_hdr_path_sel_reg.b0105_or_oetf = 0;
		dm_hdr_path_sel_reg.b02b03_no_sram_mode = 0;
		dm_hdr_path_sel_reg.b05_position = 0;
		dm_hdr_path_sel_reg.bbc_hdr_enable = 0;
		dm_hdr_path_sel_reg.hdr_hist_position = 0;
		dm_hdr_path_sel_reg.dm_b05_22bit_path = 0;
	} else if (system_info_struct->HDR_flag == HAL_VPQ_HDR_MODE_SDR_MAX_RGB) {
		dm_hdr_path_sel_reg.b0103_or_eotf = 1;
		dm_hdr_path_sel_reg.b0105_or_oetf = 1;
		dm_hdr_path_sel_reg.b02b03_no_sram_mode = 1;
		dm_hdr_path_sel_reg.bbc_hdr_enable = 0;
		dm_hdr_path_sel_reg.hdr_hist_position = Mode;
#ifdef ENABLE_HDR_DM_22b_PATH
		dm_hdr_path_sel_reg.b05_position = 3;
		dm_hdr_path_sel_reg.dm_b05_22bit_path = 1;
#else
		dm_hdr_path_sel_reg.b05_position = 1;
		dm_hdr_path_sel_reg.dm_b05_22bit_path = 0;
#endif
	} else if (system_info_struct->HDR_flag == HAL_VPQ_HDR_MODE_HDR10) {
		dm_hdr_path_sel_reg.b0103_or_eotf = 1;
		dm_hdr_path_sel_reg.b0105_or_oetf = 1;
		dm_hdr_path_sel_reg.b02b03_no_sram_mode = 1;
		dm_hdr_path_sel_reg.bbc_hdr_enable = 0;
		dm_hdr_path_sel_reg.hdr_hist_position = Mode;
#ifdef ENABLE_HDR_DM_22b_PATH
		dm_hdr_path_sel_reg.b05_position = 3;
		dm_hdr_path_sel_reg.dm_b05_22bit_path = 1;
#else
		dm_hdr_path_sel_reg.b05_position = 1;
		dm_hdr_path_sel_reg.dm_b05_22bit_path = 0;
#endif
	} else if (system_info_struct->HDR_flag == HAL_VPQ_HDR_MODE_HLG) {
		dm_hdr_path_sel_reg.b0103_or_eotf = 1;
		dm_hdr_path_sel_reg.b0105_or_oetf = 1;
		dm_hdr_path_sel_reg.b02b03_no_sram_mode = 1;
		dm_hdr_path_sel_reg.bbc_hdr_enable = 1;
		dm_hdr_path_sel_reg.hdr_hist_position = Mode;
#ifdef ENABLE_HDR_DM_22b_PATH
		dm_hdr_path_sel_reg.b05_position = 3;
		dm_hdr_path_sel_reg.dm_b05_22bit_path = 1;
#else
		dm_hdr_path_sel_reg.b05_position = 1;
		dm_hdr_path_sel_reg.dm_b05_22bit_path = 0;
#endif
	} else {
		dm_hdr_path_sel_reg.b0103_or_eotf = 1;
		dm_hdr_path_sel_reg.b0105_or_oetf = 1;
		dm_hdr_path_sel_reg.b02b03_no_sram_mode = 1;
		dm_hdr_path_sel_reg.b05_position = 0;
		dm_hdr_path_sel_reg.bbc_hdr_enable = 0;
		dm_hdr_path_sel_reg.hdr_hist_position = Mode;
		dm_hdr_path_sel_reg.dm_b05_22bit_path = 0;
	}

	memcpy(&dm_hdr_path_sel_reg_ini.regValue, &dm_hdr_path_sel_reg_init, sizeof(int));
	dm_hdr_path_sel_reg_ini.hdr_hist_position = Mode;

	if ((dm_hdr_path_sel_reg.regValue != dm_hdr_path_sel_reg_ini.regValue) && (dm_hdr_path_sel_reg_diff_cnt > 0))
		rtd_pr_vpq_info("fwif_color_DM2_HDR_histogram_Mode_set_ISR, init != default\n");

	if (gDB_done_check_in_DM2_Hist_AutoMode == 1) {
		drvif_Get_DM2_DB_Status(&db_en, &db_lv, &db_apply);
		if (db_apply == 0)
			gDB_done_check_in_DM2_Hist_AutoMode = 0;
	}
	drvif_Set_dm_hdr_path_sel_reg_ISR(dm_hdr_path_sel_reg_ini.regValue);
#endif
}

static unsigned int HDR_Hist_Auto_Mode[DM2_Hist_MODE_MAX][HDR_Histogram_size] = {{0}};
static unsigned int HDR_Hist_Auto_Mode_temp[HDR_Histogram_size] = {0};
unsigned char fwif_color_DM2_Hist_AutoMode_ch1_sta_ISR(void)
{
	static unsigned int dm_hdr_path_sel_reg_ini = 0;
	unsigned char hist_position = 0;
	unsigned char IRQ_Status = 0;
	if ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE) && (DM2_Hist_AutoMode_Flag == 1)) {
		IRQ_Status = drvif_DM2_HDR_histogram_IRQ_Status_Get_ISR();
		if (DM2_Hist_AutoMode_Counter == 0) {
			if (IRQ_Status == 1) {
				hist_position = drvif_DM2_HDR_histogram_Mode_Get_ISR();
#ifdef VIP_Support_DM_Hist_2Mode_toggle	
				if (hist_position == DM2_Hist_Y_Mode) {
					if (drvif_DM2_HDR_histogram_Get_ISR(&HDR_Hist_Auto_Mode_temp[0]) == 0)
						memcpy(HDR_Hist_Auto_Mode[DM2_Hist_RGB_MAX_Mode], &HDR_Hist_Auto_Mode_temp[0], sizeof(int)*HDR_Histogram_size);
					fwif_color_DM2_HDR_histogram_Mode_set_ISR(DM2_Hist_RGB_MAX_Mode, dm_hdr_path_sel_reg_ini);
				} else {
					if (drvif_DM2_HDR_histogram_Get_ISR(&HDR_Hist_Auto_Mode_temp[0]) == 0)
						memcpy(HDR_Hist_Auto_Mode[DM2_Hist_Y_Mode], &HDR_Hist_Auto_Mode_temp[0], sizeof(int)*HDR_Histogram_size);
					fwif_color_DM2_HDR_histogram_Mode_set_ISR(DM2_Hist_Y_Mode, dm_hdr_path_sel_reg_ini);
				}
				dm_hdr_path_sel_reg_diff_cnt = ((dm_hdr_path_sel_reg_diff_cnt-1)<0)?(0):(dm_hdr_path_sel_reg_diff_cnt-1);
				//drvif_DM2_HDR_histogram_IRQ_Status_Set_ISR(1);		// clear "hist_irq(hist_irq_status)"
#else
				if (drvif_DM2_HDR_histogram_Get_ISR(&HDR_Hist_Auto_Mode_temp[0]) == 0) {
					memcpy(HDR_Hist_Auto_Mode[DM2_Hist_Y_Mode], &HDR_Hist_Auto_Mode_temp[0], sizeof(int)*HDR_Histogram_size);					
					memcpy(HDR_Hist_Auto_Mode[DM2_Hist_RGB_MAX_Mode], &HDR_Hist_Auto_Mode_temp[0], sizeof(int)*HDR_Histogram_size);
				}
#endif
			}
		} else {
			dm_hdr_path_sel_reg_ini = drvif_Get_dm_hdr_path_sel_reg_ISR();
			if (IRQ_Status == 1) {
				//drvif_DM2_HDR_histogram_IRQ_Status_Set_ISR(1);		// clear "hist_irq(hist_irq_status)"
				DM2_Hist_AutoMode_Counter--;
				dm_hdr_path_sel_reg_diff_cnt = 5;
			}
		}
	}

	return 0;
}

unsigned char fwif_color_DM2_Hist_AutoMode_ch1_end_ISR(void)
{
#if 0
	if ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE) && (DM2_Hist_AutoMode_Flag == 1))
		if (drvif_DM2_HDR_histogram_IRQ_Status_Get_ISR())
			drvif_DM2_HDR_histogram_IRQ_Status_Set_ISR(0);
#endif
	return 0;

}

unsigned int* fwif_color_DM2_Hist_AutoMode_TBL_Get(unsigned char idx)
{
	if (idx > DM2_Hist_MODE_MAX)
		idx = 0;
	return HDR_Hist_Auto_Mode[idx];
}

static unsigned int VIP_DM2_Sat_Hist[HDR_Sat_Histogram_size];
static unsigned int VIP_DM2_Hue_Hist[HDR_Hue_Histogram_size];
unsigned char VIP_DM2_Sat__Hue_Hist_Skip = 0;	// for tool
unsigned char fwif_color_DM2_Sat_Hue_Hist_ch1_sta_ISR(void)
{
	char ret;
	if (VIP_DM2_Sat__Hue_Hist_Skip == 0) {
		ret = drvif_DM2_HDR_sat_histo_get_ISR(&VIP_DM2_Sat_Hist[0]);
		ret = drvif_DM2_HDR_Hue_histo_get_ISR(&VIP_DM2_Hue_Hist[0]);
	}else 
		ret=0;
	
	return ret;
}

char fwif_color_DM2_Sat_Hist_TBL_Get(unsigned int* pSatTbl)
{
	if (pSatTbl == NULL) {
		rtd_pr_vpq_emerg("fwif_color_DM2_Sat_Hist_TBL_Get, NULL !!!\n");
		return -1;
	}

	memcpy(pSatTbl, &VIP_DM2_Sat_Hist[0], sizeof(int)*HDR_Sat_Histogram_size);

	return 0;
}

char fwif_color_DM2_Hue_Hist_TBL_Get(unsigned int* pHueTbl)
{
	if (pHueTbl == NULL) {
		rtd_pr_vpq_emerg("fwif_color_DM2_Hue_Hist_TBL_Get, NULL !!!\n");
		return -1;
	}
	
	memcpy(pHueTbl, &VIP_DM2_Hue_Hist[0], sizeof(int)*HDR_Hue_Histogram_size);

	return 0;
}

#if 0	// use "scaler_vtop_dolby_mode_setting()"
void fwif_color_set_HDR_Dolby_mode_by_HFC(UINT32 V_top_index, UINT8 apply)
{
	extern UINT32 V_top_reg[5][HFC_VTOP_REG_MAX];
	hdr_all_top_top_ctl_RBUS top_ctl_reg_temp;
	top_ctl_reg_temp.regValue = V_top_reg[V_top_index][HFC_VTOP_REG_NORMAL];	//no compress mode default

	if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI)
	{//no need to check dm connect status
		//HDMI HDR10
		drvif_DM_HDR_Vtop_set(top_ctl_reg_temp.dolby_mode, top_ctl_reg_temp.en_422to444_1, top_ctl_reg_temp.en_422to444_tc);
	}
	else if(Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC)
	{//no need to check dm connect status
		if(drvif_HDR_get_VO_Decompress_Status(Get_DisplayMode_Port(Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY))))
		{//Playback compress mode
			top_ctl_reg_temp.regValue = V_top_reg[V_top_index][HFC_VTOP_REG_COMPRESS];
		}
		drvif_DM_HDR_Vtop_set(top_ctl_reg_temp.dolby_mode, top_ctl_reg_temp.en_422to444_1, top_ctl_reg_temp.en_422to444_tc);
	}

	if (apply) WaitDOLBY_DB_Apply_Clean();

}
#endif

char fwif_color_set_DM2_EOTF_TV006(UINT32* EOTF_table32, unsigned char check_err)
{
#if 0	/* use DMA driver*/
	UINT16 i = 0;
	while (i < 1025)
	{
		if (pre_Eotf_table32[i] != EOTF_table32[i]) break;
		i++;
	}

	if (i == 1025) {
		return;
	}
	else
	{
		for (i = 0; i < 1025; i++)
			pre_Eotf_table32[i] = EOTF_table32[i];
	}

	vgip_vgip_chn1_ctrl_RBUS vgip_vgip_chn1_ctrl_Reg;
	vgip_vgip_chn1_ctrl_Reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);

	UINT32 timeoutCnt = 325000;
	if (vgip_vgip_chn1_ctrl_Reg.ch1_in_clk_en == 0)//VGIP clk disable
	{
		rtd_pr_vpq_info("HDR10, booting/source change 1, fwif_color_set_DM2_EOTF_TV006 no wait, VGIP CLK disable\n");
	}
	else if (Check_AP_Set_Enable_ForceBG(SLR_MAIN_DISPLAY))	//in ForceBG
	{
		rtd_pr_vpq_info("HDR10, booting/source change 2, fwif_color_set_DM2_EOTF_TV006 no wait, ForceBG\n");
	}
	else
	{
		while((drvif_DM_HDR10_porch_check() == 0) && timeoutCnt > 10)
		{
			timeoutCnt--;
		}
		if (timeoutCnt <= 10)
			rtd_pr_vpq_err("HDR10, fwif_color_set_DM2_EOTF_TV006 don't get porch!\n");
	}

	if (drvif_DM_HDR10_enable_Get()) {

//#ifndef CONFIG_HDR_SDR_SEAMLESS
		drvif_DM2_EOTF_Enable(1, 1);
//#endif
		drvif_DM2_EOTF_Set(EOTF_table32, EOTF_table32, EOTF_table32);
		Eotf_NOT_set = 0;
	}
	else {
		Eotf_NOT_set = 1;
	}
#else
	//extern struct semaphore VPQ_DM_EOTF_Semaphore;
	unsigned char db_en, db_lv, db_apply;
	unsigned char eotf_lut_enable, b0103_or_eotf, b02b03_no_sram_mode;
	char ret = 0, ret_apply = 0;
	unsigned char check_times = 0, check_times_max = 30;

	down(&VPQ_DM_DMA_TBL_Semaphore);
	if (EOTF_table32 == NULL) {
		drvif_DM2_EOTF_Enable(0, 0);
	} else if ((drvif_DM_HDR10_enable_Get() == 0) || ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE))) {
		//drvif_DM2_EOTF_Enable(0, 1); // use sRam
		rtd_pr_vpq_debug("DM_EOTF_DMA, use Sram\n");

		//drvif_Get_DM2_DB_Status(&db_en, &db_lv, &db_apply);
		//drvif_Get_DM2_OETF_Enable_Status(&eotf_lut_enable, &b0103_or_eotf, &b02b03_no_sram_mode);
		b02b03_no_sram_mode = drvif_Get_DM2_B0203_noSram_clk_flip_flop();
		if (b02b03_no_sram_mode == 0) {
			drvif_DM2_B0203_noSram_Enable(1, 0);
			ret_apply = WaitDM_DB_Apply_Clean();
		} else {
			drvif_DM2_B0203_noSram_Enable(1, 0);
			ret_apply = 1;
		}

		if (ret_apply < 0) {
			drvif_DM2_EOTF_Enable(0, 1);
			rtd_pr_vpq_emerg("DM_EOTF_DMA, Sram, wait DB apply fail.\n");
			ret = -1;
		} else {
			ret = drvif_DM2_EOTF_Set(EOTF_table32, EOTF_table32, EOTF_table32);
			if (ret < 0) {
				drvif_DM2_EOTF_Enable(0, 1);
				rtd_pr_vpq_emerg("DM_EOTF_DMA, Sram, write fail.\n");
			} else {
				drvif_DM2_EOTF_Enable(1, 1);
				rtd_pr_vpq_debug("DM_EOTF_DMA, write done 0\n");
			}
		}
	} else {
		if (check_err == 1) {	/* check err status while DMA write */
			do {
				drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Reset();
				check_times++;
			} while((drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Check(0) != 0) && (check_times<check_times_max));

			if (check_times >= check_times_max) {
				rtd_pr_vpq_emerg("DM_EOTF_DMA, can't reset DM2 DMA Err status, check times = %d,\n", check_times);
				drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Check(1);
				ret = -1;
			} else {
				drvif_Get_DM2_DB_Status(&db_en, &db_lv, &db_apply);
				drvif_Get_DM2_EOTF_Enable_Status(&eotf_lut_enable, &b0103_or_eotf, &b02b03_no_sram_mode);
#if 0	// only enable no Sram to avoid gabage at first time HDR case
				if (db_apply == 1 || eotf_lut_enable == 0 || b0103_or_eotf == 0 || b02b03_no_sram_mode == 0) {
					drvif_DM2_EOTF_Enable(1, 1);
#else
				if (db_apply == 1 || b02b03_no_sram_mode == 0) {
					drvif_DM2_B0203_noSram_Enable(1, 0);
#endif
					ret_apply = WaitDM_DB_Apply_Clean();
				} else {
					ret_apply = 1;
				}

				if (ret_apply < 0) {
					drvif_DM2_EOTF_Enable(0, 1);
					rtd_pr_vpq_emerg("DM_EOTF_DMA, wait DB apply fail, db_apply=%d, no_sram=%d,\n", db_apply, b02b03_no_sram_mode);
					ret = -1;
				} else {
					ret = fwif_color_DM2_EOTF_Set_By_DMA(EOTF_table32, EOTF_table32, EOTF_table32);
					if (drvif_DM2_Wait_for_DMA_Apply_Done() < 0) {
						rtd_pr_vpq_emerg("DM_EOTF_DMA, DMA wait not finish.\n");
						drvif_DM2_EOTF_Enable(0, 1);
						ret = -1;
					} else {
						if (drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Check(1) != 0) {
							drvif_DM2_EOTF_Enable(0, 1);
							rtd_pr_vpq_emerg("DM_EOTF_DMA, error status ~= 0..\n");
							ret = -1;
						} else {
							drvif_DM2_EOTF_Enable(1, 1);
							rtd_pr_vpq_debug("DM_EOTF_DMA, write done 0\n");
							ret = 0;
						}
					}
				}

			}
		} else {
			drvif_Get_DM2_DB_Status(&db_en, &db_lv, &db_apply);
			drvif_Get_DM2_EOTF_Enable_Status(&eotf_lut_enable, &b0103_or_eotf, &b02b03_no_sram_mode);

			if (db_apply == 1 || eotf_lut_enable == 0 || b0103_or_eotf == 0 || b02b03_no_sram_mode == 0) {
				drvif_DM2_EOTF_Enable(1, 1);
				drvif_DM2_B0203_noSram_Enable(1, 0);
				ret_apply = WaitDM_DB_Apply_Clean();
			} else {
				ret_apply = 1;
			}

			if (ret_apply < 0) {
				drvif_DM2_EOTF_Enable(0, 1);
				rtd_pr_vpq_emerg("DM_EOTF_DMA, wait DB apply fail.\n");
				ret = -1;
			} else {
				ret = fwif_color_DM2_EOTF_Set_By_DMA(EOTF_table32, EOTF_table32, EOTF_table32);
				rtd_pr_vpq_debug("DM_EOTF_DMA, write done 1\n");
			}
		}
	}
	up(&VPQ_DM_DMA_TBL_Semaphore);
	return ret;
#endif
}

char fwif_color_set_DM2_OETF_TV006(UINT16* OETF_table16, unsigned char check_err)
{
#if 0	/* use DMA driver*/
	UINT16 i = 0;
	while (i < 1025)
	{
		if (pre_Oetf_table16[i] != OETF_table16[i]) break;
		i++;
	}

	if (i == 1025) {
		return;
	}
	else
	{
		for (i = 0; i < 1025; i++)
			pre_Oetf_table16[i] = OETF_table16[i];
	}

	vgip_vgip_chn1_ctrl_RBUS vgip_vgip_chn1_ctrl_Reg;
	vgip_vgip_chn1_ctrl_Reg.regValue = IoReg_Read32(VGIP_VGIP_CHN1_CTRL_reg);

	UINT32 timeoutCnt = 325000;
	if (vgip_vgip_chn1_ctrl_Reg.ch1_in_clk_en == 0)//VGIP clk disable
	{
		rtd_pr_vpq_info("HDR10, booting/source change 1, fwif_color_set_DM2_OETF_TV006 no wait, VGIP CLK disable\n");
	}
	else if (Check_AP_Set_Enable_ForceBG(SLR_MAIN_DISPLAY))	//in ForceBG
	{
		rtd_pr_vpq_info("HDR10, booting/source change 2, fwif_color_set_DM2_OETF_TV006 no wait, ForceBG\n");
	}
	else
	{
		while((drvif_DM_HDR10_porch_check() == 0) && timeoutCnt > 10)
		{
			timeoutCnt--;
		}
		if (timeoutCnt <= 10)
			rtd_pr_vpq_err("HDR10, fwif_color_set_DM2_OETF_TV006 don't get porch!\n");
	}

	if (drvif_DM_HDR10_enable_Get()) {

#ifndef CONFIG_HDR_SDR_SEAMLESS
		drvif_DM2_OETF_Enable(1, 1);
#endif
		drvif_DM2_OETF_Set(OETF_table16, OETF_table16, OETF_table16);
		Oetf_NOT_set = 0;
	}
	else {
		Oetf_NOT_set = 1;
	}
#else
	//extern struct semaphore VPQ_DM_OETF_Semaphore;
	unsigned char db_en, db_lv, db_apply;
	unsigned char oetf_lut_enable, b0105_or_oetf, b02b03_no_sram_mode;
	char ret = 0, ret_apply = 0;
	unsigned char check_times = 0, check_times_max = 30;

	down(&VPQ_DM_DMA_TBL_Semaphore);
	if (OETF_table16 == NULL){
		drvif_DM2_OETF_Enable(0, 0);
	} else if ((drvif_DM_HDR10_enable_Get() == 0) || ((Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) != _MODE_STATE_ACTIVE))) {
		//drvif_DM2_OETF_Enable(0, 1);  // use srame
		rtd_pr_vpq_info("DM_OETF_DMA, use sRam\n");

		//drvif_Get_DM2_DB_Status(&db_en, &db_lv, &db_apply);
		//drvif_Get_DM2_OETF_Enable_Status(&oetf_lut_enable, &b0105_or_oetf, &b02b03_no_sram_mode);
		b02b03_no_sram_mode = drvif_Get_DM2_B0203_noSram_clk_flip_flop();
		if (b02b03_no_sram_mode == 0) {
			drvif_DM2_B0203_noSram_Enable(1, 0);
			ret_apply = WaitDM_DB_Apply_Clean();
		} else {
			drvif_DM2_B0203_noSram_Enable(1, 0);
			ret_apply = 1;
		}

		if (ret_apply < 0) {
			drvif_DM2_OETF_Enable(0, 1);
			rtd_pr_vpq_emerg("DM_OETF_DMA, Sram, wait DB apply fail.\n");
			ret = -1;
		} else {
			ret = drvif_DM2_OETF_Set(OETF_table16, OETF_table16, OETF_table16);
			if (ret < 0) {
				drvif_DM2_OETF_Enable(0, 1);
				rtd_pr_vpq_emerg("DM_OETF_DMA, Sram, write fail.\n");
			} else {
				drvif_DM2_OETF_Enable(1, 1);
				rtd_pr_vpq_info("DM_OETF_DMA, write done 0\n");
			}
		}
	} else {
		if (check_err == 1) {	/* check err status while DMA write */
			do {
				drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Reset();
				check_times++;
			} while((drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Check(0) != 0) && (check_times<check_times_max));

			if (check_times >= check_times_max) {
				rtd_pr_vpq_emerg("DM_OETF_DMA, can't reset DM2 DMA Err status, check times = %d,\n", check_times);
				drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Check(1);
				ret = -1;
			} else {
				drvif_Get_DM2_DB_Status(&db_en, &db_lv, &db_apply);
				drvif_Get_DM2_OETF_Enable_Status(&oetf_lut_enable, &b0105_or_oetf, &b02b03_no_sram_mode);
#if 0	// only enable no Sram to avoid gabage at first time HDR case
				if (db_apply == 1 || oetf_lut_enable == 0 || b0105_or_oetf == 0 || b02b03_no_sram_mode == 0) {
					drvif_DM2_OETF_Enable(1, 1);
#else
				if (db_apply == 1 ||  b02b03_no_sram_mode == 0) {
					drvif_DM2_B0203_noSram_Enable(1, 0);
#endif
					ret_apply = WaitDM_DB_Apply_Clean();
				} else {
					ret_apply = 1;
				}

				if (ret_apply < 0) {
					drvif_DM2_OETF_Enable(0, 1);
					rtd_pr_vpq_emerg("DM_OETF_DMA, wait DB apply fail, db_apply=%d, no_sram=%d,\n", db_apply, b02b03_no_sram_mode);
					ret = -1;
				} else {
					ret = fwif_color_DM2_OETF_Set_By_DMA(OETF_table16, OETF_table16, OETF_table16);
					if (drvif_DM2_Wait_for_DMA_Apply_Done() < 0) {
						rtd_pr_vpq_emerg("DM_OETF_DMA, DMA wait not finish.\n");
						drvif_DM2_OETF_Enable(0, 1);
						ret = -1;
					} else {
						if (drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Check(1) != 0) {
							drvif_DM2_OETF_Enable(0, 1);
							rtd_pr_vpq_emerg("DM_OETF_DMA, error status ~= 0..\n");
							ret = -1;
						} else {
							drvif_DM2_OETF_Enable(1, 1);
							ret = 0;
							rtd_pr_vpq_info("DM_OETF_DMA, write done 0\n");
						}
					}
				}

			}
		} else {
			drvif_Get_DM2_DB_Status(&db_en, &db_lv, &db_apply);
			drvif_Get_DM2_OETF_Enable_Status(&oetf_lut_enable, &b0105_or_oetf, &b02b03_no_sram_mode);

			if (db_apply == 1 || oetf_lut_enable == 0 || b0105_or_oetf == 0 || b02b03_no_sram_mode == 0) {
				drvif_DM2_OETF_Enable(1, 1);
				drvif_DM2_B0203_noSram_Enable(1, 0);
				ret_apply = WaitDM_DB_Apply_Clean();
			} else {
				ret_apply = 1;
			}

			if (ret_apply < 0) {
				drvif_DM2_OETF_Enable(0, 1);
				rtd_pr_vpq_emerg("DM_OETF_DMA, wait DB apply fail.\n");
				ret = -1;
			} else {
				ret = fwif_color_DM2_OETF_Set_By_DMA(OETF_table16, OETF_table16, OETF_table16);
				rtd_pr_vpq_info("DM_OETF_DMA, write done 1\n");
			}
		}
	}
	up(&VPQ_DM_DMA_TBL_Semaphore);
	return ret;
#endif
}
	
/* EOTF setting function for HDR10_vivid (CUVA) */
void fwif_color_set_DM2_EOTF_HDR10_vivid_TV006(UINT32* EOTF_table32_R, UINT32* EOTF_table32_G, UINT32* EOTF_table32_B, unsigned char check_err)
{
	//extern struct semaphore VPQ_DM_EOTF_Semaphore;

	down(&VPQ_DM_DMA_TBL_Semaphore);
	if (EOTF_table32_R == NULL || EOTF_table32_G == NULL ||EOTF_table32_B == NULL) {
		drvif_DM2_EOTF_Enable(0, 0);
		rtd_pr_vpq_emerg("DM_EOTF_DMA, off\n");
	} else {
		//drvif_DM2_EOTF_Enable(1, 1);
		//drvif_DM2_B0203_noSram_Enable(1);
		fwif_color_DM2_EOTF_Set_By_DMA(EOTF_table32_R, EOTF_table32_G, EOTF_table32_B);
	}
	up(&VPQ_DM_DMA_TBL_Semaphore);
}

/* OETF setting function for HDR10_vivid (CUVA) */
void fwif_color_set_DM2_OETF_HDR10_vivid_TV006(UINT16* OETF_table16_R, UINT16* OETF_table16_G, UINT16* OETF_table16_B, unsigned char check_err)
{
	//extern struct semaphore VPQ_DM_OETF_Semaphore;

	down(&VPQ_DM_DMA_TBL_Semaphore);
	if (OETF_table16_R == NULL || OETF_table16_G == NULL || OETF_table16_B == NULL ){
		drvif_DM2_OETF_Enable(0, 0);
		rtd_pr_vpq_emerg("DM_OETF_DMA, off\n");
	} else {
		//drvif_DM2_EOTF_Enable(1, 1);
		//drvif_DM2_B0203_noSram_Enable(1);
		fwif_color_DM2_OETF_Set_By_DMA(OETF_table16_R, OETF_table16_G, OETF_table16_B);
	}
	up(&VPQ_DM_DMA_TBL_Semaphore);
	
}

unsigned short OOTF_X_006[(OETF_PROG_SIZE_TV006>>1)], OOTF_Y_006[(OETF_PROG_SIZE_TV006>>1)];
char fwif_color_set_DM2_OETF_Conv_TV006(UINT16* in_OETF_table16, UINT16* out_OETF_table16, unsigned char tbl_sel)
{
	extern unsigned int OETF_Prog_Idx_th_TBL[7];
	extern VIP_HDR_OETF_Prog_Idx_TBL VIP_HDR_OETF_Prog_Idx_tbl[OETF_prog_idx_tbl_num];
	VIP_HDR_OETF_Prog_Idx_TBL Prog_Idx_TBL = {{0}, {0}};
	unsigned short i, i_step, i_grid_num;
	unsigned char idx, idx_2;
	unsigned int x_in_32;
	unsigned int x_in_16;
	int ootf_x_0, ootf_x_1, ootf_y_0, ootf_y_1;

	if (tbl_sel >= OETF_prog_idx_tbl_num) {
		rtd_pr_vpq_emerg("fwif_color_set_DM2_OETF_Conv_TV006 error, tbl_sel=%d,\n", tbl_sel);
		return -1;
	}
	if (in_OETF_table16 == NULL || out_OETF_table16 == NULL) {
		rtd_pr_vpq_emerg("fwif_color_set_DM2_OETF_Conv_TV006 error, in_OETF_table16=%p, out_OETF_table16=%p,\n", in_OETF_table16, out_OETF_table16);
		return -1;
	}

	memcpy(&Prog_Idx_TBL, &VIP_HDR_OETF_Prog_Idx_tbl[tbl_sel], sizeof(VIP_HDR_OETF_Prog_Idx_TBL));
	memcpy(&OOTF_X_006[0], &in_OETF_table16[0], sizeof(short)*(OETF_PROG_SIZE_TV006>>1));
	memcpy(&OOTF_Y_006[0], &in_OETF_table16[(OETF_PROG_SIZE_TV006>>1)], sizeof(short)*(OETF_PROG_SIZE_TV006>>1));

	if (OOTF_X_006[0] != 0 || OOTF_Y_006[0] != 0 || OOTF_X_006[(OETF_PROG_SIZE_TV006>>1)-1] != 0xFFFF || OOTF_X_006[(OETF_PROG_SIZE_TV006>>1)-1] != 0xFFFF) {
		OOTF_X_006[0] =  0;
		OOTF_Y_006[0] =  0;
		OOTF_X_006[(OETF_PROG_SIZE_TV006>>1)-1] = 0xFFFF;
		OOTF_X_006[(OETF_PROG_SIZE_TV006>>1)-1] = 0xFFFF;
		rtd_pr_vpq_emerg("OETF_Conv_TV006 error, OOTF_X_006=%d,%d, OOTF_Y_006=%d,%d,\n",
			OOTF_X_006[0], OOTF_X_006[(OETF_PROG_SIZE_TV006>>1)-1], OOTF_Y_006[0], OOTF_Y_006[(OETF_PROG_SIZE_TV006>>1)-1]);
	}

	idx = 0;
	idx_2 = 0;
	for (i=0;i<(OETF_size-1);i++) {
	//for (i=0;i<20;i++) {
		// mapping to value of x-axis on 32 bit
		if (idx < 7)
			if (i >= Prog_Idx_TBL.oetf_grid_num_th[idx])
				idx++;

		if (idx > 0) {
			x_in_32 = OETF_Prog_Idx_th_TBL[idx-1];
			i_grid_num = Prog_Idx_TBL.oetf_grid_num_th[idx-1];
		} else {
			x_in_32 = 0;
			i_grid_num = 0;
		}
		i_step = Prog_Idx_TBL.oetf_step[idx];
		x_in_32 = x_in_32 + (1<<i_step) * (i - i_grid_num);

		// mapping to 16 bit
		x_in_16 = x_in_32>>16;

		//rtd_pr_vpq_emerg("1.OETF_Conv_TV006=%d,%d,%d,%d,%d,%d,%d,\n", i, idx, i_grid_num, i_step, x_in_32, x_in_16, OETF_Prog_Idx_th_TBL[idx-1]);

		// interpolation with tv006 table
		if (x_in_16 <= 0) {	// boundary low
			out_OETF_table16[i] = 0;
		} else if (x_in_16 >= 0xFFFF) {
			out_OETF_table16[i] = 0xFFFF;	// boundary high
		} else {
			if (idx_2 < ((OETF_PROG_SIZE_TV006>>1) -1))	// max of "idx_2" is (OETF_prog_size_tv006>>1) -1
				if (x_in_16 >= OOTF_X_006[idx_2])
					idx_2++;
			if (idx_2 == 0) {
				ootf_x_0 = OOTF_X_006[idx_2];
				ootf_x_1 = OOTF_X_006[idx_2];
				ootf_y_0 = OOTF_Y_006[idx_2];
				ootf_y_1 = OOTF_Y_006[idx_2];
			} else {
				ootf_x_0 = OOTF_X_006[idx_2-1];
				ootf_x_1 = OOTF_X_006[idx_2];
				ootf_y_0 = OOTF_Y_006[idx_2-1];
				ootf_y_1 = OOTF_Y_006[idx_2];
			}
			if (ootf_x_0 >= ootf_x_1 || ootf_y_0 >= ootf_y_1) {
				out_OETF_table16[i] = ootf_y_0;
			} else if (x_in_16 == ootf_x_0) {
				out_OETF_table16[i] = ootf_y_0;
			} else if (x_in_16 == ootf_x_1) {
				out_OETF_table16[i] = ootf_y_1;
			} else {
				out_OETF_table16[i] = (unsigned short) ((x_in_16-ootf_x_0) * (ootf_y_1 - ootf_y_0) / (ootf_x_1-ootf_x_0) + ootf_y_0);
			}
		}
		//rtd_pr_vpq_emerg("2.OETF_Conv_TV006=%d,%d,%d,%d,%d,%d,\n", idx_2, ootf_x_0, ootf_x_1, ootf_y_0, ootf_y_1, out_OETF_table16[i]);

	}
	out_OETF_table16[OETF_size-1] = 0xFFFF;
	return 0;
}

void fwif_color_set_DM2_ToneMapping_TV006(UINT16* R, UINT16* G, UINT16* B)
{
	drvif_DM2_ToneMapping_Set(R, G, B);
}

void hdr_resume()
{
	//UINT16 i = 0;

	PQ_HDR_3dLUT_16_PARAM args;
	extern unsigned int lastCoef9;
	extern UINT32 hdr_YUV2RGB[VIP_HDR_DM_CSC1_YUV2RGB_TABLE_Max][12];
	extern dm_dm_submodule_enable_RBUS dm_submodule_temp;

	drvif_Set_DM_HDR_CLK();	// enable DM & Composer clock
	// enable v top for DMA write
	drvif_DM_HDR_Vtop_set(1, 0, 0);

	drvif_HDR_YUV2RGB(1, hdr_YUV2RGB[HDR_DM_CSC1_YUV2RGB_BT2020_Limted_235_to_255]);	// 20180319 roger, give default value for HAL do not correct case. BT2020 limit range para

	// give default avoid hdr garbage while first play
	args.c3dLUT_16_enable = 1;
	args.a3dLUT_16_pArray = (UINT16*)&HDR10_3DLUT_24x24x24[0];

	//fwif_color_set_DM_HDR_3dLUT_24x24x24_16_TV006(&args, 0);	// for resume time mesearment

	drvif_DM_HDR_Vtop_set(0, 0, 0);
	drvif_TV006_SET_HDR_mode_off();

	lastCoef9 = 0xffffffff;

	dm_submodule_temp.regValue = 0;	/* DM submodule flag init, HDR off setting*/
}

void fwif_color_set_HDR_TV006()
{
#if 0	/* DMA table should  be ok, don't set again*/
	extern unsigned char g_HDR3DLUTForceWrite;
	extern dm_dm_submodule_enable_RBUS dm_submodule_temp;
	char ret = 0;
	if (g_HDR3DLUTForceWrite) {
		extern UINT16 pre_hdr_3dlut[HDR_24x24x24_size*3];
		extern struct semaphore VPQ_DM_3DLUT_Semaphore;
		down(&VPQ_DM_3DLUT_Semaphore);
		ret = drvif_Set_DM_HDR_3dLUT_24x24x24_16(1, pre_hdr_3dlut);
		up(&VPQ_DM_3DLUT_Semaphore);
		if (ret == 0)
			dm_submodule_temp.b0501_enable = 1; /* 24x24x24 CSC3 in VSC*/
		else
			dm_submodule_temp.b0501_enable = 0; /* 24x24x24 CSC3 in VSC*/
		g_HDR3DLUTForceWrite = 0;
	}

#ifndef CONFIG_HDR_SDR_SEAMLESS

	if (Eotf_NOT_set) {

		drvif_DM2_EOTF_Enable(1, 1);
		drvif_DM2_EOTF_Set(pre_Eotf_table32, pre_Eotf_table32, pre_Eotf_table32);

		Eotf_NOT_set = 0;
	}
	if (Oetf_NOT_set) {
		drvif_DM2_OETF_Enable(1, 1);
		drvif_DM2_OETF_Set(pre_Oetf_table16, pre_Oetf_table16, pre_Oetf_table16);
		Oetf_NOT_set = 0;
	}

#else

	if(PQModeInfo_flag[0] == HAL_VPQ_HDR_MODE_HLG)
	{
		drvif_DM2_EOTF_Enable(1, 1);
		drvif_DM2_OETF_Enable(1, 1);
	}
	else if( PQModeInfo_flag[0] == HAL_VPQ_HDR_MODE_HDR10)
	{
		drvif_DM2_EOTF_Enable(1, 0);
		drvif_DM2_OETF_Enable(1, 0);
	}
#endif
#endif
}

void fwif_color_set_HDR10_runmode()
{
	unsigned int I3DDMA_Color_format, DisplayMode_Src;
	unsigned char twopixel_mode = 0, hdmi_input = 0, color_format;
	unsigned char in_sel;
	unsigned char inbits_sel;

	I3DDMA_Color_format = (unsigned int)drvif_I3DDMA_Get_Input_Info_Vaule(I3DDMA_INFO_TARGET_COLOR);
	DisplayMode_Src = (unsigned int)Get_DisplayMode_Src(SLR_MAIN_DISPLAY);

	get_vtop_input_info(&twopixel_mode, &hdmi_input, &color_format);
	inbits_sel = 2; /*K4, HDR10/HLG always 2, 12bit, 14 is for OTT*/

	rtd_pr_vpq_info("HDR10_runmode,I3D_Color_fmt=%d,vsc_Src=%d,2pixel=%d,hdmi_in=%d,colorFMT=%d,\n",
		I3DDMA_Color_format, DisplayMode_Src, twopixel_mode, hdmi_input, color_format);

	if (DisplayMode_Src == VSC_INPUTSRC_HDMI) {
		if (twopixel_mode == 1 && hdmi_input == 1) {
			if (color_format == COLOR_YUV422)
				in_sel = 1;
			else
				in_sel = 2;
		} else if (twopixel_mode == 0 && hdmi_input == 1) {
			if (color_format == COLOR_YUV422)
				in_sel = 1;
			else
				in_sel = 2;
		} else {
			if (color_format == COLOR_YUV422)
				in_sel = 1;
			else
				in_sel = 3;	//444, TC, TC need to disable
		}
	} else /*if ((DisplayMode_Src == VSC_INPUTSRC_AVD) || (DisplayMode_Src == VSC_INPUTSRC_ADC) || (DisplayMode_Src == VSC_INPUTSRC_JPEG))*/ {
		if (color_format == COLOR_YUV422)
			in_sel = 1;
		else
			in_sel = 3;	//444, TC, TC need to disable
	} /*else {
		in_sel = 1;	//422
	}*/
	drvif_color_set_dm_in_sel(in_sel, inbits_sel);
#if 0
	if ((I3DDMA_Color_format == I3DDMA_COLOR_YUV444) && (DisplayMode_Src == VSC_INPUTSRC_HDMI)) // 444
		drvif_color_set_dm_in_sel(3);	//444, TC, TC need to disable
	else
		drvif_color_set_dm_in_sel(1);	//422


	input_format_reg.regValue = IoReg_Read32(DM_Input_Format_reg);
	input_format_reg.dm_inbits_sel = 2; /*K4, HDR10/HLG always 2, 12bit, 14 is for OTT*/
	IoReg_Write32(DM_Input_Format_reg, input_format_reg.regValue);
#endif
}

UINT8 fwif_color_get_DM2_HDR_histogram_TV006(UINT32* HDR_info)
{
	drvif_DM2_HDR_RGB_max_Get(HDR_info);
	//return drvif_DM2_HDR_histogram_Get(HDR_info + 3);
	memcpy((HDR_info + 3), fwif_color_DM2_Hist_AutoMode_TBL_Get(DM2_Hist_RGB_MAX_Mode), sizeof(int)*HDR_Histogram_size);
	return 1;
}

UINT8 fwif_color_get_DM_HDR10_enable_TV006()
{
	return drvif_DM_HDR10_enable_Get();
}


void fwif_color_set_LD_Backlight_Decision_Replace_TV006(KADP_LED_BACKLIGHT_DECISION *LED_DB_LUT_BACKLIGHT_DECISION, DRV_LD_Backlight_Decision *LD_Backlight_Decision)
{
	if (NULL == LED_DB_LUT_BACKLIGHT_DECISION || NULL == LD_Backlight_Decision)
		return;

	LD_Backlight_Decision->ld_maxgain = LED_DB_LUT_BACKLIGHT_DECISION->hal_ld_maxgain;
	LD_Backlight_Decision->ld_avegain = LED_DB_LUT_BACKLIGHT_DECISION->hal_ld_avegain;
	LD_Backlight_Decision->ld_histshiftbit = LED_DB_LUT_BACKLIGHT_DECISION->hal_ld_histshiftbit;
	LD_Backlight_Decision->ld_hist0gain = LED_DB_LUT_BACKLIGHT_DECISION->hal_ld_hist0gain;
	LD_Backlight_Decision->ld_hist1gain = LED_DB_LUT_BACKLIGHT_DECISION->hal_ld_hist1gain;
	LD_Backlight_Decision->ld_hist2gain = LED_DB_LUT_BACKLIGHT_DECISION->hal_ld_hist2gain;
	LD_Backlight_Decision->ld_hist3gain = LED_DB_LUT_BACKLIGHT_DECISION->hal_ld_hist3gain;
	LD_Backlight_Decision->ld_hist4gain = LED_DB_LUT_BACKLIGHT_DECISION->hal_ld_hist4gain;
	LD_Backlight_Decision->ld_hist5gain = LED_DB_LUT_BACKLIGHT_DECISION->hal_ld_hist5gain;
	LD_Backlight_Decision->ld_hist6gain = LED_DB_LUT_BACKLIGHT_DECISION->hal_ld_hist6gain;
	LD_Backlight_Decision->ld_hist7gain = LED_DB_LUT_BACKLIGHT_DECISION->hal_ld_hist7gain;

}

void fwif_color_set_LD_Spatial_Filter_Replace_TV006(KADP_LED_SPATIAL_FILTER *LED_DB_LUT_SPATIAL_FILTER, DRV_LD_Spatial_Filter *LD_Spatial_Filter)
{

	if (NULL == LED_DB_LUT_SPATIAL_FILTER || NULL == LD_Spatial_Filter)
		return;


	LD_Spatial_Filter->ld_spatialnewcoef00 = LED_DB_LUT_SPATIAL_FILTER->hal_ld_spatialnewcoef00;
	LD_Spatial_Filter->ld_spatialnewcoef01 = LED_DB_LUT_SPATIAL_FILTER->hal_ld_spatialnewcoef01;
	LD_Spatial_Filter->ld_spatialnewcoef02 = LED_DB_LUT_SPATIAL_FILTER->hal_ld_spatialnewcoef02;
	LD_Spatial_Filter->ld_spatialnewcoef03 = LED_DB_LUT_SPATIAL_FILTER->hal_ld_spatialnewcoef03;
	LD_Spatial_Filter->ld_spatialnewcoef04 = LED_DB_LUT_SPATIAL_FILTER->hal_ld_spatialnewcoef04;
	LD_Spatial_Filter->ld_spatialnewcoef10 = LED_DB_LUT_SPATIAL_FILTER->hal_ld_spatialnewcoef10;
	LD_Spatial_Filter->ld_spatialnewcoef11 = LED_DB_LUT_SPATIAL_FILTER->hal_ld_spatialnewcoef11;
	LD_Spatial_Filter->ld_spatialnewcoef12 = LED_DB_LUT_SPATIAL_FILTER->hal_ld_spatialnewcoef12;
	LD_Spatial_Filter->ld_spatialnewcoef13 = LED_DB_LUT_SPATIAL_FILTER->hal_ld_spatialnewcoef13;
	LD_Spatial_Filter->ld_spatialnewcoef14 = LED_DB_LUT_SPATIAL_FILTER->hal_ld_spatialnewcoef14;
	LD_Spatial_Filter->ld_spatialnewcoef05 = LED_DB_LUT_SPATIAL_FILTER->hal_ld_spatialnewcoef05;
	LD_Spatial_Filter->ld_spatialnewcoef15 = LED_DB_LUT_SPATIAL_FILTER->hal_ld_spatialnewcoef15;

}

void fwif_color_set_LD_Temporal_Filter_Replace_TV006(KADP_LED_TEMPORAL_FILTER *LED_DB_LUT_TEMPORAL_FILTER, DRV_LD_Temporal_Filter *LD_Temporal_Filter)
{
	if (NULL == LED_DB_LUT_TEMPORAL_FILTER || NULL == LD_Temporal_Filter)
		return;
	LD_Temporal_Filter->ld_tmp_pos0thd = LED_DB_LUT_TEMPORAL_FILTER->hal_ld_tmp_pos0thd;
	LD_Temporal_Filter->ld_tmp_pos1thd = LED_DB_LUT_TEMPORAL_FILTER->hal_ld_tmp_pos1thd;
	LD_Temporal_Filter->ld_tmp_posmingain = LED_DB_LUT_TEMPORAL_FILTER->hal_ld_tmp_posmingain;
	LD_Temporal_Filter->ld_tmp_posmaxgain = LED_DB_LUT_TEMPORAL_FILTER->hal_ld_tmp_posmaxgain;
	LD_Temporal_Filter->ld_tmp_neg0thd = LED_DB_LUT_TEMPORAL_FILTER->hal_ld_tmp_neg0thd;
	LD_Temporal_Filter->ld_tmp_neg1thd = LED_DB_LUT_TEMPORAL_FILTER->hal_ld_tmp_neg1thd;
	LD_Temporal_Filter->ld_tmp_negmingain = LED_DB_LUT_TEMPORAL_FILTER->hal_ld_tmp_negmingain;
	LD_Temporal_Filter->ld_tmp_negmaxgain = LED_DB_LUT_TEMPORAL_FILTER->hal_ld_tmp_negmaxgain;
	LD_Temporal_Filter->ld_tmp_maxdiff = LED_DB_LUT_TEMPORAL_FILTER->hal_ld_tmp_maxdiff;
	LD_Temporal_Filter->ld_tmp_scenechangegain1 = LED_DB_LUT_TEMPORAL_FILTER->hal_ld_tmp_scenechangegain1;

}
void fwif_color_set_LD_Spatial_Remap_Replace_TV006(KADP_LED_REMAP_CURVE *LED_DB_LUT_REMAP_CURVE, DRV_LD_Spatial_Remap *LD_Spatial_Remap)
{
	if (NULL == LED_DB_LUT_REMAP_CURVE || NULL == LD_Spatial_Remap)
		return;


	LD_Spatial_Remap->ld_spatialremapen = LED_DB_LUT_REMAP_CURVE->hal_ld_spatialremapen;
	memcpy(&(LD_Spatial_Remap->ld_spatialremaptab), &(LED_DB_LUT_REMAP_CURVE->hal_ld_spatialremaptab),sizeof(unsigned short)*65);

}

void fwif_color_set_LD_Data_Compensation_NewMode_2DTable_Replace_TV006(KADP_LED_DATA_COMP *LED_DB_LUT_DATA_COMP, DRV_LD_Data_Compensation_NewMode_2DTable *LD_2DTable)
{
	if (NULL == LED_DB_LUT_DATA_COMP || NULL == LD_2DTable)
		return;

	memcpy(LD_2DTable->ld_comp_2Dtable, LED_DB_LUT_DATA_COMP->hal_ld_comp_2Dtable, sizeof(unsigned int)*17*17);

}



void fwif_color_set_LDSetDBLUT_TV006(KADP_LED_DB_LUT_T *LED_DB_LUT)
{

	int i = 0;
	unsigned char tbl_idx = 0;

	extern DRV_Local_Dimming_Table Local_Dimming_Table[LD_Table_NUM];
	extern DRV_LD_Data_Compensation_NewMode_2DTable LD_Data_Compensation_NewMode_2DTable[LD_Table_NUM];
	if (NULL == LED_DB_LUT)
		return;

	for (i=0;i<LD_Table_NUM;i++) {
		tbl_idx = LED_DB_LUT[i].LD_UI_Index;
		if (tbl_idx >= LD_Table_NUM) {
			break;
		} else {
			fwif_color_set_LD_Backlight_Decision_Replace_TV006(&LED_DB_LUT[tbl_idx].hal_backlight_decision, &Local_Dimming_Table[tbl_idx].LD_Backlight_Decision);
			fwif_color_set_LD_Spatial_Filter_Replace_TV006(&LED_DB_LUT[tbl_idx].hal_spatial_filter, &Local_Dimming_Table[tbl_idx].LD_Spatial_Filter);
			fwif_color_set_LD_Temporal_Filter_Replace_TV006(&LED_DB_LUT[tbl_idx].hal_temporal_filter, &Local_Dimming_Table[tbl_idx].LD_Temporal_Filter);
			fwif_color_set_LD_Spatial_Remap_Replace_TV006(&LED_DB_LUT[tbl_idx].hal_remap_curve, &Local_Dimming_Table[tbl_idx].LD_Spatial_Remap);
			fwif_color_set_LD_Data_Compensation_NewMode_2DTable_Replace_TV006(&LED_DB_LUT[tbl_idx].hal_data_comp_table, &LD_Data_Compensation_NewMode_2DTable[tbl_idx]);
		}
	}


}
void fwif_color_set_PQA_motion_threshold_TV006(void)
{
	unsigned int table_row = 0;
	unsigned char i = 0,j=0,y_ini=0,c_ini=0;
	//StructColorDataType *pTable = NULL;
	SLR_VIP_TABLE *shareMem_VIP_Table = NULL;
        SLR_VIP_TABLE *gVip_Table = fwif_colo_get_AP_vip_table_gVIP_Table();
	UINT8 source=255,pqa_table_idx=0;

	unsigned int PQA_reg_value=0,PQA_bitup_value=0,PQA_bitlow_value=0;
	RTK_NOISE_REDUCTION_T *pNR_Table =NULL;
	static unsigned char cp_ini=0;
	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);/* get vip source by input src and timing*/
	if (source >= VIP_QUALITY_SOURCE_NUM)
		source = 0;


	pNR_Table = fwif_color_Get_NR_Table_tv006();

	shareMem_VIP_Table = fwif_color_GetShare_Memory_VIP_TABLE_Struct();

	if (pNR_Table == NULL || shareMem_VIP_Table == NULL||gVip_Table==NULL)
		return;


	pqa_table_idx = gVip_Table->VIP_QUALITY_Extend3_Coef[source][VIP_QUALITY_FUNCTION_PQA_Table];

	y_ini=0;
	c_ini=0;

	if(cp_ini==0)
	{
#ifdef CONFIG_ARM64
	//	memcpy_fromio(&Ori_PQA_Table,&shareMem_VIP_Table->PQA_Table,sizeof(Ori_PQA_Table));
#else
	//	memcpy(&Ori_PQA_Table,&shareMem_VIP_Table->PQA_Table,sizeof(Ori_PQA_Table));
#endif
		cp_ini=1;
	}
	for (table_row = 0; table_row < PQA_ITEM_MAX; table_row++)
	{


		PQA_reg_value = shareMem_VIP_Table->PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row][PQA_reg];
		PQA_bitup_value = shareMem_VIP_Table->PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row][PQA_bitup];
		PQA_bitlow_value = shareMem_VIP_Table->PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row][PQA_bitlow];

		if((PQA_reg_value == DI_IM_DI_RTNR_Y_16_TH12_TH14_reg)&&(PQA_bitup_value == 23)&&(PQA_bitlow_value == 16)&&(y_ini==0)){
			y_ini=1;
			for(i=0;i<15;i++){
				for(j=0;j<10;j++){

					if ( vpq_project_id == 0x00060000)
					        shareMem_VIP_Table->PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row+i][PQA_L00+j] =pNR_Table->NR_TABLE.TNR_Y.Mad[i][j];
                                        //else
					//	shareMem_VIP_Table->PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row+i][PQA_L00+j] =Ori_PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row+i][PQA_L00+j];

				}
			}
		}

		if((PQA_reg_value == DI_IM_DI_RTNR_C_TH4_TH6_reg)&&(PQA_bitup_value == 23)&&(PQA_bitlow_value == 16)&&(c_ini==0)){
			c_ini=1;

			for(i=0;i<7;i++){
				for(j=0;j<10;j++){
					if ((table_row+i) < 255) {
					if ( vpq_project_id == 0x00060000)
					        shareMem_VIP_Table->PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row+i][PQA_L00+j] =pNR_Table->NR_TABLE.TNR_C.Mad[i][j];
					//else
					//	shareMem_VIP_Table->PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row+i][PQA_L00+j] =Ori_PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row+i][PQA_L00+j];


				}
			}
		}
	}
}
}
void fwif_color_get_PQA_motion_threshold_TV006(void)
{
	unsigned int table_row = 0;
	unsigned char i = 0,j=0,y_ini=0,c_ini=0;
	//StructColorDataType *pTable = NULL;
	SLR_VIP_TABLE *shareMem_VIP_Table = NULL;
	UINT8 source=255,pqa_table_idx=0;

	unsigned int PQA_reg_value=0,PQA_bitup_value=0,PQA_bitlow_value=0;
	static unsigned char cp_ini=0;
	source = fwif_vip_source_check(3, NOT_BY_DISPLAY);/* get vip source by input src and timing*/
	if (source >= VIP_QUALITY_SOURCE_NUM)
		source = 0;



	shareMem_VIP_Table = fwif_color_GetShare_Memory_VIP_TABLE_Struct();

	if (shareMem_VIP_Table == NULL)
		return;


	pqa_table_idx = shareMem_VIP_Table->VIP_QUALITY_Extend3_Coef[source][VIP_QUALITY_FUNCTION_PQA_Table];

	y_ini=0;
	c_ini=0;

	if(cp_ini==0)
	{
/*#ifdef CONFIG_ARM64
		memcpy_fromio(&Ori_PQA_Table,&shareMem_VIP_Table->PQA_Table,sizeof(Ori_PQA_Table));
#else
		memcpy(&Ori_PQA_Table,&shareMem_VIP_Table->PQA_Table,sizeof(Ori_PQA_Table));
#endif*/
		cp_ini=1;
	}
	for (table_row = 0; table_row < PQA_ITEM_MAX; table_row++)
	{


		PQA_reg_value = shareMem_VIP_Table->PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row][PQA_reg];
		PQA_bitup_value = shareMem_VIP_Table->PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row][PQA_bitup];
		PQA_bitlow_value = shareMem_VIP_Table->PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row][PQA_bitlow];

		if((PQA_reg_value == DI_IM_DI_RTNR_Y_16_TH12_TH14_reg)&&(PQA_bitup_value == 23)&&(PQA_bitlow_value == 16)&&(y_ini==0)){
			y_ini=1;
			for(i=0;i<15;i++){
				for(j=0;j<10;j++){
                                       TNR_Y_Store_Mad[i][j]=shareMem_VIP_Table->PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row+i][PQA_L00+j];

				}
			}
		}

		if((PQA_reg_value == DI_IM_DI_RTNR_C_TH4_TH6_reg)&&(PQA_bitup_value == 23)&&(PQA_bitlow_value == 16)&&(c_ini==0)){
			c_ini=1;

			for(i=0;i<7;i++){
				for(j=0;j<10;j++){
					if ((table_row+i) < 255) {
                                                TNR_C_Store_Mad[i][j]=shareMem_VIP_Table->PQA_Table[pqa_table_idx][PQA_MODE_WRITE][table_row+i][PQA_L00+j];


				        }
			        }
		        }
	        }
        }
}


#define DGA_InRangeMax 1024
#define DGA_OutRangeMax 4096*4

void fwif_color_set_output_gamma_4CHDGA_TV006 (UINT32 *pRedGammaTable, UINT32 * pGreenGammaTable, UINT32 * pBlueGammaTable, UINT32 * pWhiteGammaTable, UINT16 nTableSize)
{
	int i=0;
	int offset = 0;
	unsigned char output_Gamma_En;
	unsigned char output_Location;
	unsigned char fmt;
        unsigned char mapping_gain = DGA_OutRangeMax / DGA_InRangeMax;

	if (nTableSize == 256) {

		for (i=0;i<256;i++) {
			OutGamma_4CHDGA_R[i] =(unsigned short) ((*(pRedGammaTable+i) * mapping_gain));
			OutGamma_4CHDGA_G[i] =(unsigned short) ((*(pGreenGammaTable+i) * mapping_gain));
			OutGamma_4CHDGA_B[i] =(unsigned short) ((*(pBlueGammaTable+i) * mapping_gain));
			OutGamma_4CHDGA_W[i] =(unsigned short) ((*(pWhiteGammaTable+i) * mapping_gain));
		}
		OutGamma_4CHDGA_R[256] = 2*OutGamma_4CHDGA_R[255] - OutGamma_4CHDGA_R[254];
		OutGamma_4CHDGA_G[256] = 2*OutGamma_4CHDGA_G[255] - OutGamma_4CHDGA_G[254];
		OutGamma_4CHDGA_B[256] = 2*OutGamma_4CHDGA_B[255] - OutGamma_4CHDGA_B[254];
		OutGamma_4CHDGA_W[256] = 2*OutGamma_4CHDGA_W[255] - OutGamma_4CHDGA_W[254];

	} else if (nTableSize == 1024){

#if 0
		for (i=0;i<512;i++) {
			offset = 2*i;
			OutGamma_4CHDGA_R[i] =(unsigned short) (*(pRedGammaTable+offset) * mapping_gain);
			OutGamma_4CHDGA_G[i] =(unsigned short) (*(pGreenGammaTable+offset) * mapping_gain);
			OutGamma_4CHDGA_B[i] =(unsigned short) (*(pBlueGammaTable+offset) * mapping_gain);
			OutGamma_4CHDGA_W[i] =(unsigned short) (*(pWhiteGammaTable+offset) * mapping_gain);
			//printk("OutGamma_4CHDGA_R[%d]=%d\n",i, OutGamma_4CHDGA_R[i]);
		}

		OutGamma_4CHDGA_R[512] = (*(pRedGammaTable+1023) * mapping_gain)*4/3 - (*(pRedGammaTable+mapping_gain*255) * mapping_gain)/3;
		OutGamma_4CHDGA_G[512] = (*(pGreenGammaTable+1023) * mapping_gain)*4/3 - (*(pGreenGammaTable+mapping_gain*255) * mapping_gain)/3;
		OutGamma_4CHDGA_B[512] = (*(pBlueGammaTable+1023) * mapping_gain)*4/3 - (*(pBlueGammaTable+mapping_gain*255) * mapping_gain)/3;
		OutGamma_4CHDGA_W[512] = (*(pWhiteGammaTable+1023) * mapping_gain)*4/3 - (*(pWhiteGammaTable+mapping_gain*255) * mapping_gain)/3;
#else
		for (i=0;i<1024;i++) {
			offset = 1*i;
			OutGamma_4CHDGA_R[i] =(unsigned short) (*(pRedGammaTable+offset) * mapping_gain);
			OutGamma_4CHDGA_G[i] =(unsigned short) (*(pGreenGammaTable+offset) * mapping_gain);
			OutGamma_4CHDGA_B[i] =(unsigned short) (*(pBlueGammaTable+offset) * mapping_gain);
			OutGamma_4CHDGA_W[i] =(unsigned short) (*(pWhiteGammaTable+offset) * mapping_gain);
			//printk("OutGamma_4CHDGA_R[%d]=%d\n",i, OutGamma_4CHDGA_R[i]);

		}
                OutGamma_4CHDGA_R[1024] = (*(pRedGammaTable+1023) * mapping_gain)*4/3 - (*(pRedGammaTable+mapping_gain*255) * mapping_gain)/3;

                OutGamma_4CHDGA_G[1024] = (*(pGreenGammaTable+1023) * mapping_gain)*4/3 - (*(pGreenGammaTable+mapping_gain*255) * mapping_gain)/3;

                OutGamma_4CHDGA_B[1024] = (*(pBlueGammaTable+1023) * mapping_gain)*4/3 - (*(pBlueGammaTable+mapping_gain*255) * mapping_gain)/3;

                OutGamma_4CHDGA_W[1024] = (*(pWhiteGammaTable+1023) * mapping_gain)*4/3 - (*(pWhiteGammaTable+mapping_gain*255) * mapping_gain)/3;

#endif


	} else {
		return ;
	}


	output_Gamma_En = 1;
	if (Get_DISPLAY_PANEL_MPLUS_RGBW() == TRUE || Get_DISPLAY_PANEL_BOW_RGBW() == TRUE)
		fmt = 1; /*RGBW*/
	else
		fmt = 0;
	output_Location = 1;/*After RGBW*/

	//this function is for 12bit
	//fwif_color_out_gamma_encode_1ch(pRedGammaTable, OutGamma_4CHDGA_R);
	//fwif_color_out_gamma_encode_1ch(pGreenGammaTable, OutGamma_4CHDGA_G);
	//fwif_color_out_gamma_encode_1ch(pBlueGammaTable, OutGamma_4CHDGA_B);
	//fwif_color_out_gamma_encode_1ch(pWhiteGammaTable, OutGamma_4CHDGA_W);

	//K24 out_gamma is 14 bit
	fwif_color_out_gamma_encode_1ch_enhance(final_OutGAMMA_R_d0d1,final_OutGAMMA_R_index, OutGamma_4CHDGA_R);
	fwif_color_out_gamma_encode_1ch_enhance(final_OutGAMMA_G_d0d1,final_OutGAMMA_G_index, OutGamma_4CHDGA_G);
	fwif_color_out_gamma_encode_1ch_enhance(final_OutGAMMA_B_d0d1,final_OutGAMMA_B_index, OutGamma_4CHDGA_B);
	fwif_color_out_gamma_encode_1ch_enhance(final_OutGAMMA_W_d0d1,final_OutGAMMA_W_index, OutGamma_4CHDGA_W);

	fwif_color_out_gamma_control_front();

	//this function is for 12bit
	//drvif_color_Out_Gamma_Curve_Write(pRedGammaTable, GAMMA_CHANNEL_R);     /*R*/
	//drvif_color_Out_Gamma_Curve_Write(pGreenGammaTable, GAMMA_CHANNEL_G);   /*G*/
	//drvif_color_Out_Gamma_Curve_Write(pBlueGammaTable, GAMMA_CHANNEL_B);    /*B*/
	//if (fmt) drvif_color_Out_Gamma_Curve_Write(pWhiteGammaTable, GAMMA_CHANNEL_W);  /*W*/

	//K24 out_gamma is 14 bit
	drvif_color_colorwrite_outGamma_enhance(final_OutGAMMA_R_d0d1,final_OutGAMMA_R_index, GAMMA_CHANNEL_R);	/*R*/
	drvif_color_colorwrite_outGamma_enhance(final_OutGAMMA_G_d0d1,final_OutGAMMA_G_index, GAMMA_CHANNEL_G);	/*G*/
	drvif_color_colorwrite_outGamma_enhance(final_OutGAMMA_B_d0d1, final_OutGAMMA_B_index,GAMMA_CHANNEL_B);	/*B*/
	if (fmt) drvif_color_colorwrite_outGamma_enhance(final_OutGAMMA_W_d0d1,final_OutGAMMA_W_index, GAMMA_CHANNEL_W);	/*W*/

	fwif_color_out_gamma_control_back();

	drvif_color_set_output_gamma_format(output_Gamma_En, fmt, output_Location, 0);


}



/* --------- function description :  fwif_color_set_output_gamma_PTG_TV006 ----- ------*/
/* --------- This function is used to set one gray level by output gamma -------- ------*/
/* ----------bCtrl = on, set outgamma_en = on and table = value -----------------------*/
/* ---------- bCtrl = off, set ooutgamma_en = original, table = original -------------------*/
unsigned char output_gamma_DataStored = 0;
void fwif_color_set_output_gamma_PTG_TV006 (unsigned char bCtrl, unsigned short R_value,unsigned short G_value,unsigned short B_value,unsigned short W_value)
{
#ifdef _VIP_Mer7_Compile_Error__
	int i=0;
	//int offset = 0;
	static unsigned char output_Gamma_En_ori;
	static unsigned char fmt_ori = 0;
	static unsigned char output_Location_ori = 0;
	outputgamma_out_gamma_ctrl_1_RBUS outputgamma_out_gamma_ctrl_1_reg;
	outputgamma_out_gamma_ctrl_2_RBUS outputgamma_out_gamma_ctrl_2_reg;

	/* output gamma PTG value*/
	for (i=0;i<(Bin_Num_Output_Gamma>>1);i++) {
		OutGamma_PTG_R[i] = R_value << 16;
		OutGamma_PTG_G[i] = G_value << 16;
		OutGamma_PTG_B[i] = B_value << 16;
		OutGamma_PTG_W[i] = W_value << 16;
	}

	if (bCtrl == 1) {

		/*Read current table, enable, location, fmt*/
		if (output_gamma_DataStored == 0) {
			outputgamma_out_gamma_ctrl_1_reg.regValue = IoReg_Read32(OUTPUTGAMMA_OUT_GAMMA_CTRL_1_reg);
			outputgamma_out_gamma_ctrl_2_reg.regValue = IoReg_Read32(OUTPUTGAMMA_OUT_GAMMA_CTRL_2_reg);

			output_Gamma_En_ori = outputgamma_out_gamma_ctrl_1_reg.out_gamma_en;
			output_Location_ori = outputgamma_out_gamma_ctrl_1_reg.out_gamma_location;
			fmt_ori = outputgamma_out_gamma_ctrl_2_reg.out_gamma_format;

			drvif_color_colorRead_outGamma(OutGamma_Ori_R, GAMMA_CHANNEL_R);
			drvif_color_colorRead_outGamma(OutGamma_Ori_G, GAMMA_CHANNEL_G);
			drvif_color_colorRead_outGamma(OutGamma_Ori_B, GAMMA_CHANNEL_B);
			drvif_color_colorRead_outGamma(OutGamma_Ori_W, GAMMA_CHANNEL_W);
			output_gamma_DataStored = 1;
		}

		fwif_color_out_gamma_control_front();

		drvif_color_Out_Gamma_Curve_Write(OutGamma_PTG_R, GAMMA_CHANNEL_R);	/*R*/
		drvif_color_Out_Gamma_Curve_Write(OutGamma_PTG_G, GAMMA_CHANNEL_G);	/*G*/
		drvif_color_Out_Gamma_Curve_Write(OutGamma_PTG_B, GAMMA_CHANNEL_B);	/*B*/
		drvif_color_Out_Gamma_Curve_Write(OutGamma_PTG_W, GAMMA_CHANNEL_W);	/*W*/

		fwif_color_out_gamma_control_back();

		drvif_color_set_output_gamma_format(1, fmt_ori, output_Location_ori, 0);

		drvif_color_out_gamma_Control_Enhance(1, output_Location_ori);

	} else {

		output_gamma_DataStored = 0;

		fwif_color_out_gamma_control_front();

		drvif_color_Out_Gamma_Curve_Write(OutGamma_Ori_R, GAMMA_CHANNEL_R);	/*R*/
		drvif_color_Out_Gamma_Curve_Write(OutGamma_Ori_G, GAMMA_CHANNEL_G);	/*G*/
		drvif_color_Out_Gamma_Curve_Write(OutGamma_Ori_B, GAMMA_CHANNEL_B);	/*B*/
		drvif_color_Out_Gamma_Curve_Write(OutGamma_Ori_W, GAMMA_CHANNEL_W);	/*W*/

		fwif_color_out_gamma_control_back();

		drvif_color_set_output_gamma_format(output_Gamma_En_ori, fmt_ori, output_Location_ori, 0);
		
		drvif_color_out_gamma_Control_Enhance(output_Gamma_En_ori, output_Location_ori);
	}
#endif

}



void fwif_color_set_SetBOERGBWBypass_TV006(unsigned char BOE_EN)
{
	VIP_RGBW_MODE RGBW_MODE;
	if(BOE_EN == true)
		RGBW_MODE = VIP_RGBW_MODE_DISABLE;
	else
		RGBW_MODE = VIP_RGBW_MODE_SIW;

	fwif_color_safe_od_enable(0);/*protection*/
	fwif_color_WaitFor_DEN_STOP_MEMCDTG();
	drvif_color_set_RGBW_Curcuit(RGBW_MODE);
	fwif_color_safe_od_enable(1);/*protection*/


}

char fwif_color_DM2_EOTF_OETF_3D_24x24x24_LUT_Handler(HAL_VPQ_HDR_MODE_T HDR_MODE)
{
	char ret_val = -1;
	unsigned char check_times = 0, check_times_max = 20;
	unsigned char DMA_set_times = 0, DMA_set_times_max = 3;
	static PQ_HDR_3dLUT_16_PARAM args;
	extern unsigned short OETF_Lmix[16];
	extern VIP_HDR_OETF_Prog_Idx_TBL OETF_prog_idx_tbl[OETF_prog_idx_tbl_num];
	extern unsigned int OETF_prog_th_tbl[OETF_prog_idx_tbl_num][7];
	args.c3dLUT_16_enable = 1;

	if ((HDR_MODE != HAL_VPQ_HDR_MODE_HDR10) && (HDR_MODE != HAL_VPQ_HDR_MODE_HLG) && (HDR_MODE != HAL_VPQ_HDR_MODE_ST2094)) {
		ret_val = 1;
		rtd_pr_vpq_info("DMA_LUT_Handler, SDR\n");
	} else {
		/* DMA will write fail if dolby mode is off */
		if (drvif_DM_HDR10_enable_Get() == 0) {
			if (HDR_MODE == HAL_VPQ_HDR_MODE_ST2094)
				fwif_color_set_HDR_Dolby_mode_Enable(HDR_DM_ST2094);
			else if (HDR_MODE == HAL_VPQ_HDR_MODE_HLG)
				fwif_color_set_HDR_Dolby_mode_Enable(HDR_DM_HLG);
			else
				fwif_color_set_HDR_Dolby_mode_Enable(HDR_DM_HDR10);

			drvif_Set_DM_HDR_3dLUT_Dimension(1);
			drvif_DM2_EOTF_Enable(1, 1);
			drvif_DM2_OETF_Enable(1, 1);
			drvif_DM2_B0203_noSram_Enable(1, 0);
			ret_val = WaitDOLBY_DB_Apply_Clean();
			ret_val = WaitDM_DB_Apply_Clean();
		}

		for (DMA_set_times = 0;DMA_set_times<DMA_set_times_max;DMA_set_times++) {
			ret_val = -1;
			if (HDR_MODE == HAL_VPQ_HDR_MODE_HDR10) {
					args.a3dLUT_16_pArray = &HDR10_3DLUT_24x24x24[0][0];
					fwif_color_set_DM_HDR_3dLUT_24x24x24_16_TV006(&args, 0);
					drvif_color_set_HDR_OETF_Lmix(1, OETF_Lmix);
					Scaler_color_Get_HDR10_EOTF_ByMaxCLL_ByPanelLuminance(0);
					drvif_DM2_OETF_Set_Prog_Index(&OETF_prog_idx_tbl[1]);
					drvif_DM2_OETF_Set_Prog_Index_th(OETF_prog_th_tbl[1]);
					Scaler_color_Get_HDR10_OETF_ByMaxCLL_ByPanelLuminance(0);
					drvif_color_Set_HDR_OETF_Decrease(0);
					drvif_DM2_Wait_for_DMA_Apply_Done();
			} else if (HDR_MODE == HAL_VPQ_HDR_MODE_HLG) {
					args.a3dLUT_16_pArray = &HLG_3DLUT_24x24x24[0][0];
					fwif_color_set_DM_HDR_3dLUT_24x24x24_16_TV006(&args, 0);
					fwif_color_set_DM2_EOTF_TV006(HLG_EOTF_LUT_R, 0);
					drvif_DM2_OETF_Set_Prog_Index(&OETF_prog_idx_tbl[0]);
					drvif_DM2_OETF_Set_Prog_Index_th(OETF_prog_th_tbl[0]);
					fwif_color_set_DM2_OETF_TV006(HLG_OETF_LUT_R, 0);
					drvif_color_Set_HDR_OETF_Decrease(0);
					drvif_DM2_Wait_for_DMA_Apply_Done();
			} else if (HDR_MODE == HAL_VPQ_HDR_MODE_ST2094) {
					args.a3dLUT_16_pArray = &HDR10_3DLUT_24x24x24[0][0];
					fwif_color_set_DM_HDR_3dLUT_24x24x24_16_TV006(&args, 0);
					fwif_color_set_DM2_EOTF_TV006(EOTF_LUT_R, 0);
					drvif_DM2_OETF_Set_Prog_Index(&OETF_prog_idx_tbl[0]);
					drvif_DM2_OETF_Set_Prog_Index_th(OETF_prog_th_tbl[0]);
					fwif_color_set_DM2_OETF_TV006(OETF_LUT_R, 0);
					drvif_color_Set_HDR_OETF_Decrease(0);
					drvif_DM2_Wait_for_DMA_Apply_Done();
			}

			if (drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Check(0) == 0) {
				rtd_pr_vpq_info("DMA_LUT_Handler, DM2 DMA write successful, DMA set times = %d\n", DMA_set_times);
				ret_val = 0; /* err status check okm DMA write successful, finish handle*/
				break;
			}

			do {
				drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Reset();
				check_times++;
			} while((drvif_DM2_EOTF_OETF_3DLUT_DMA_Err_Check(0) != 0) && (check_times<check_times_max));

			if (check_times >= check_times_max) {
				rtd_pr_vpq_emerg("DMA_LUT_Handler, can't reset DM2 DMA Err status, check times = %d, DMA_set_times=%d\n", check_times, DMA_set_times);
				ret_val = -2;	/* can't reset err status*/
				break;
			}
			check_times = 0;
		}
	}

	if (ret_val < 0)
		rtd_pr_vpq_emerg("DMA_LUT_Handler, Write DM2 DMA fail, ret = %d,check times = %d, DMA_set_times=%d,\n", ret_val, check_times, DMA_set_times);

	return ret_val;
}
#if 0
void fwif_color_set_DM2_EOTF_by_HFC(UINT8 tbl_index)
{
	extern UINT32 HFC_EOTF_LUT_R[5][1025];

	if (tbl_index == HFC_EOTF_OFF) {
		drvif_DM2_EOTF_Enable(0, 1);
	} else {
		fwif_color_set_DM2_EOTF_TV006(HFC_EOTF_LUT_R[tbl_index], 0);
	}
}

void fwif_color_set_DM2_OETF_by_HFC(UINT8 tbl_index)
{
	extern unsigned short HFC_OETF_LUT_R[5][OETF_size];

	if (tbl_index == HFC_OETF_OFF) {
		drvif_DM2_OETF_Enable(0, 1);
	} else if (tbl_index == HFC_OETF_CTRL_BY_MAXCLL) {	/*HFC OETF special table index, OETF table control by MAXCLL*/
		Scaler_color_Get_HDR10_OETF_ByMaxCLL_ByPanelLuminance(OETF_LUT_ByMaxCLL_ByPanelLuminance, 0);
		fwif_color_set_DM2_OETF_TV006(OETF_LUT_ByMaxCLL_ByPanelLuminance, 0);
	} else {
		fwif_color_set_DM2_OETF_TV006(HFC_OETF_LUT_R[tbl_index], 0);
	}
}
#endif
/*CSC3 table too big ==> DTV scan crash*/
/*can't ues CSC3[][][] ==> too big*/
unsigned short *fwif_color_get_HFC_3DLUT_24x24x24(UINT8 index)
{
	unsigned short *result;
	switch (index) {
	case HFC_CSC3_OFF:
		result = NULL;
		break;
	case 0:
	default:
		result = &HDR10_3DLUT_24x24x24[0][0];
		break;
	case 1:
		result = &HLG_3DLUT_24x24x24[0][0];
		break;
	};

	return result;
}
extern unsigned short HDR10_3DLUT_17x17x17[17*17*17][3];
unsigned short *fwif_color_get_HFC_3DLUT_17x17x17(UINT8 index)
{
	unsigned short *result;
	switch (index) {
	case HFC_CSC3_OFF:
		result = NULL;
		break;
#ifdef CONFIG_SCALER_ENABLE_V4L2
	case HFC_CSC3_ReLoad:
		result = &pre_hdr_3dlut[0];
		break;
	case 0:
#endif
	case HFC_CSC3_CTRL_BY_MAXCLL:
		result = &HDR10_3DLUT_17x17x17[0][0];
		break;
	default:
		result = &HDR10_3DLUT_17x17x17[0][0];
		break;
	case 1:
		result = &HDR10_3DLUT_17x17x17[0][0];	// need HLG 3d 17x17x17 table?
		break;
	};

	return result;
}

UINT8 fwif_color_DM2_EOTF_OETF_3D_LUT_by_HFC(UINT8 HFC_bit, HDR_flow_control_timing HDR_timing)
{
	extern 	unsigned short OETF_Lmix[16];
	extern UINT32 HFC_EOTF_LUT_R[5][1025];
	extern unsigned short HFC_OETF_LUT_R[5][OETF_size];
	extern HDR_flow_control HDR_control[HFC_SDR_TIMING_NUM];
	extern VIP_HDR_OETF_Prog_Idx_TBL OETF_prog_idx_tbl[OETF_prog_idx_tbl_num];
	extern unsigned int OETF_prog_th_tbl[OETF_prog_idx_tbl_num][7];
#ifdef isSupport_DM2_TM
	extern UINT16 TM_table12[ToneMapping_size];
#endif
#ifdef CONFIG_SCALER_ENABLE_V4L2
	extern unsigned char CSC3_LUT_reLoad_Flag;
	extern unsigned char OETF_LUT_reLoad_Flag;
	extern unsigned char CSC3_HDR_flag_v4l2;
#endif
	unsigned char HFC_bit_csc3 = 0, HFC_bit_oetf = 0;

	char ret_val = -1;
	//unsigned char check_times = 0, check_times_max = 20;
	//unsigned char DMA_set_times = 0, DMA_set_times_max = 3;
	static PQ_HDR_3dLUT_16_PARAM args;
	unsigned char tbl_index = 0;
	args.c3dLUT_16_enable = 1;

	ret_val =0;
	// 3D LUT
#ifdef CONFIG_SCALER_ENABLE_V4L2
	HFC_bit_csc3 = (CSC3_LUT_reLoad_Flag == 1)?(HFC_ctrl_ReLoad):(0);
#endif
	HFC_bit_csc3 = HFC_bit_csc3 | 0x1;
	ret_val = 127;
	if ((HDR_control[HDR_timing].CSC3_ctrl.IPCtrlBits) & HFC_bit_csc3) {
#ifdef isSupport_HDR_3dLUT_24x24x24
		args.a3dLUT_16_pArray = fwif_color_get_HFC_3DLUT_24x24x24(HDR_control[HDR_timing].CSC3_table);
		ret_val =fwif_color_set_DM_HDR_3dLUT_24x24x24_16_TV006(&args, 1);
#else
		args.a3dLUT_16_pArray = fwif_color_get_HFC_3DLUT_17x17x17(HDR_control[HDR_timing].CSC3_table);
		ret_val =fwif_color_set_DM_HDR_3dLUT_17x17x17_16_TV006(&args, 1);
#endif
#ifdef CONFIG_SUPPORT_IPQ
		drvif_color_Set_ST2094_3Dlut_CInv(1024<<5, 1, 10);	// 5x default table for OETF
#endif
	}
	//rtd_pr_vpq_info("DM_3dLUT_DMA, set by HFC, HFC_bit_csc3=%d,ret_val=%d,v4l2_flag=%d\n", HFC_bit_csc3, ret_val, CSC3_HDR_flag_v4l2);

	// EOTF
	if ((HDR_control[HDR_timing].EOTF_ctrl.IPCtrlBits) & HFC_bit) {
		tbl_index = HDR_control[HDR_timing].EOTF_SPtable;
		if (tbl_index == HFC_EOTF_OFF) {
			drvif_DM2_EOTF_Enable(0, 1);
		} else if (tbl_index == HFC_EOTF_CTRL_BY_MAXCLL) {	/*HFC OETF special table index, OETF table control by MAXCLL*/
			Scaler_color_Get_HDR10_EOTF_ByMaxCLL_ByPanelLuminance(0);
		} else {
			ret_val =fwif_color_set_DM2_EOTF_TV006(HFC_EOTF_LUT_R[tbl_index], 1);
		}
	}

	// OETF
#ifdef CONFIG_SCALER_ENABLE_V4L2
	HFC_bit_oetf = (OETF_LUT_reLoad_Flag == 1)?(HFC_ctrl_ReLoad):(0);
#endif
	HFC_bit_oetf = HFC_bit_oetf | 0x1;
	if ((HDR_control[HDR_timing].OETF_ctrl.IPCtrlBits) & HFC_bit) {
		tbl_index = HDR_control[HDR_timing].OETF_SPtable;
		if (tbl_index == HFC_OETF_OFF) {
			drvif_DM2_OETF_Enable(0, 1);
		} else if (tbl_index == HFC_OETF_CTRL_BY_MAXCLL) {	/*HFC OETF special table index, OETF table control by MAXCLL*/
			drvif_DM2_OETF_Set_Prog_Index(&OETF_prog_idx_tbl[1]);
			drvif_DM2_OETF_Set_Prog_Index_th(OETF_prog_th_tbl[1]);
			drvif_color_set_HDR_OETF_Lmix(1, OETF_Lmix);
			Scaler_color_Get_HDR10_OETF_ByMaxCLL_ByPanelLuminance(0);
			//ret_val =fwif_color_set_DM2_OETF_TV006(OETF_LUT_ByMaxCLL_ByPanelLuminance, 1);
		} else {
			drvif_DM2_OETF_Set_Prog_Index(&OETF_prog_idx_tbl[0]);
			drvif_DM2_OETF_Set_Prog_Index_th(OETF_prog_th_tbl[0]);
			ret_val =fwif_color_set_DM2_OETF_TV006(HFC_OETF_LUT_R[tbl_index], 1);
		}
	}
	rtd_pr_vpq_info("DM_OETF_DMA, set by HFC, HFC_bit_oetf=%d,ret_val=%d,ctrl_bit=%d,tbl_idx=%d,\n",
		HFC_bit_oetf, ret_val, HDR_control[HDR_timing].OETF_ctrl.IPCtrlBits, tbl_index);

#ifdef isSupport_DM2_TM
	// Tone Mapping
	ret_val = 127;
	if ((HDR_control[HDR_timing].tone_mapping_ctrl.IPCtrlBits) & HFC_bit) {
		tbl_index = HDR_control[HDR_timing].tone_mapping_table;
		if (tbl_index == HFC_TONE_MAPPING_OFF) {
			fwif_color_set_DM2_ToneMapping_TV006(NULL);
		} else {
			fwif_color_set_DM2_ToneMapping_TV006(TM_table12);
		}
	}
#endif

	return ret_val;
}

void fwif_color_MplusSet(unsigned short *pRegisterSet, unsigned char panelMaker)
{
	unsigned short regSize = 0;
	unsigned char regStep = 0;

	if(panelMaker == 0)
	{
		regSize = MplusRegSize/2; // 8 bits to 16 bits
		regStep = 2;

    fwif_color_ChangeUINT16Endian(pRegisterSet, regSize, 1);
	}
	else if(panelMaker == 1)
	{
		regSize = SiwRegSize;
		regStep = 2;
	}
	else
		return;

	drvif_color_MplusSet(pRegisterSet, regSize, regStep);
}

void fwif_color_MplusGet(unsigned short *pRegisterSet, unsigned char panelMaker)
{
	unsigned short regSize = 0;
	unsigned char regStep = 0;

	if(panelMaker==0)
	{
		regSize = MplusRegSize;
		regStep = 4;
	}
	else if(panelMaker==1)
	{
		regSize = SiwRegSize;
		regStep = 2;
	}
	else
		return;

	drvif_color_MplusGet(pRegisterSet, regSize, regStep);
}

void fwif_color_SetFrameGainLimit_TV006(unsigned short nFrameGainLimit)
{
	drvif_color_SetFrameGainLimit(nFrameGainLimit);

}
void fwif_color_GetFrameGainLimit_TV006 (unsigned short *nFrameGainLimit)
{
	drvif_color_GetFrameGainLimit(nFrameGainLimit);

}
void fwif_color_SetPixelGainLimit_TV006 (unsigned short nPixelGainLimit)
{
	drvif_color_SetPixelGainLimit(nPixelGainLimit);

}
void fwif_color_GetPixelGainLimit_TV006 (unsigned short *nPixelGainLimit)
{
	drvif_color_GetPixelGainLimit(nPixelGainLimit);

}

void fwif_color_set_LocalContrast_table_TV006(CHIP_LOCAL_CONTRAST_T *param)
{
	//unsigned int i = 0;
	unsigned char TableIdx=0;
	unsigned char src_idx = 0;
	unsigned char display = 0;
	SLR_VIP_TABLE *pVipTable = fwif_colo_get_AP_vip_table_gVIP_Table();
	
	Scaler_Get_Display_info(&display, &src_idx);

	if(param==NULL || pVipTable == NULL){
		VIPprintf("~get CHIP_LOCAL_CONTRAST_T table or VipTable Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return;
	}
	TableIdx = Scaler_GetLocalContrastTable();

	//LGE control M_LC_Comp_En
	Scaler_SetLocalContrastEnable(param->M_LC_Comp_En);

	//LGE control
       // Local_Contrast_Table[TableIdx].LC_Global_Ctrl.lc_local_sharp_en = 1;
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_Global_Ctrl.lc_tone_mapping_en = param->LC_Tone_Mapping_En;
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_Blend.lc_tmap_slope_unit = param->LC_tmap_slope_unit;
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_Blend.lc_tmap_blend_factor = param->LC_tmap_blend_factor;

	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_CurveSelect.lc_tmap_curve0_grid = param->LC_tmap_curve_Grid[0];
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_CurveSelect.lc_tmap_curve0_tone = param->LC_tmap_curve_tone[0];
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_CurveSelect.lc_tmap_curve1_grid = param->LC_tmap_curve_Grid[1];
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_CurveSelect.lc_tmap_curve1_tone = param->LC_tmap_curve_tone[1];
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_CurveSelect.lc_tmap_curve2_grid = param->LC_tmap_curve_Grid[2];
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_CurveSelect.lc_tmap_curve2_tone = param->LC_tmap_curve_tone[2];
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_CurveSelect.lc_tmap_curve3_grid = param->LC_tmap_curve_Grid[3];
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_CurveSelect.lc_tmap_curve3_tone = param->LC_tmap_curve_tone[3];
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_CurveSelect.lc_tmap_curve4_grid = param->LC_tmap_curve_Grid[4];
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_CurveSelect.lc_tmap_curve4_tone = param->LC_tmap_curve_tone[4];
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_CurveSelect.lc_tmap_curve5_grid = param->LC_tmap_curve_Grid[5];
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_CurveSelect.lc_tmap_curve5_tone = param->LC_tmap_curve_tone[5];
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_CurveSelect.lc_tmap_curve6_grid = param->LC_tmap_curve_Grid[6];
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_CurveSelect.lc_tmap_curve6_tone = param->LC_tmap_curve_tone[6];
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_CurveSelect.lc_tmap_curve7_grid = param->LC_tmap_curve_Grid[7];
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_CurveSelect.lc_tmap_curve7_tone = param->LC_tmap_curve_tone[7];

	/*
	for (i=0;i<LC_Curve_ToneM_PointSlope;i++)
	{
		pVipTable->VIP_Local_Contrast_Table[TableIdx].LC_ToneMappingSlopePoint_Table[i] = param->LC_ToneMappingSlopePont_Table[i];
	}
	*/

	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_saturation_Table.LC_saturation_en = param->LC_saturation_En;
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_saturation_Table.LC_saturation_Gain = param->LC_saturation_gain;
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_saturation_Table.LC_saturation_blend_factor = param->LC_saturation_blend_factor;
	pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_saturation_Table.LC_no_saturation_enhance_thl = param->LC_no_saturation_enhance_thl;

	fwif_color_set_LocalContrast_ToneMappingXY_SlopePoint_TV006(param); //LG DB , data structure is different

	Scaler_fwif_color_set_LocalContrast_table(src_idx, TableIdx);

	return;

}

void fwif_color_check_LC_tmap_curve_tone(UINT16* TCT_item)
{
	#define TCT_BIT 10
	UINT16 diff[TCT_BIT] = {512, 256, 128, 64, 32, 16, 8, 4, 2, 0};	//max_diff = 512, min_diff = 2, 0 is for data protect
	UINT8 i, j;

	i = 0;
	j = 0;
	//check i = 0
	while(j < TCT_BIT)
	{
		if (diff[j] <= TCT_item[i])
		{
			TCT_item[i] = diff[j];
			break;
		}
		j++;
	}


	//check i > 0
	for (i = 0; i < 7; i++)
	{
		j = 0;
		while(j < TCT_BIT)
		{
			if ((diff[j] <= TCT_item[i + 1] - TCT_item[i])	//diff is 2^x
				&& (TCT_item[i] + diff[j] < 1023))			//and max is 1023
			{
				TCT_item[i + 1] = TCT_item[i] + diff[j];
				break;
			}
			j++;
		}
	}

}

void fwif_color_set_LocalContrast_ToneMappingXY_SlopePoint_TV006(CHIP_LOCAL_CONTRAST_T *param)
{
	UINT8  grid_mode=0,i,j;
	UINT32 TableIdx =1; //lg fixed table1
	unsigned int delta_Y;
	unsigned short delta_grid_X[3][32];
	DRV_LC_ToneMapping_CurveSelect CurveSelect;
	DRV_LC_ToneMapping_Grid0 *grid0;
	DRV_LC_ToneMapping_Grid2 *grid2;
	lc_lc_tonemapping_blending_RBUS blending;
	unsigned int *slope;
	SLR_VIP_TABLE *pVipTable = fwif_colo_get_AP_vip_table_gVIP_Table();

	if (pVipTable == NULL)
		return;

	slope = pVipTable->VIP_Local_Contrast_Table[TableIdx].LC_ToneMappingSlopePoint_Table;
	grid0 = &pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_Grid0;
	grid2 = &pVipTable->VIP_Local_Contrast_Table[TableIdx].Local_Contrast_Table.LC_ToneMapping_Grid2;	

	blending.regValue = IoReg_Read32(LC_LC_ToneMapping_blending_reg);

	/* Control by LG DB */
	CurveSelect.lc_tmap_curve0_grid = param->LC_tmap_curve_Grid[0];
	CurveSelect.lc_tmap_curve1_grid = param->LC_tmap_curve_Grid[1];
	CurveSelect.lc_tmap_curve2_grid = param->LC_tmap_curve_Grid[2];
	CurveSelect.lc_tmap_curve3_grid = param->LC_tmap_curve_Grid[3];
	CurveSelect.lc_tmap_curve4_grid = param->LC_tmap_curve_Grid[4];
	CurveSelect.lc_tmap_curve5_grid = param->LC_tmap_curve_Grid[5];
	CurveSelect.lc_tmap_curve6_grid = param->LC_tmap_curve_Grid[6];
	CurveSelect.lc_tmap_curve7_grid = param->LC_tmap_curve_Grid[7];

	/*
	for (i = 0; i < 8; i++)
		pr_debug("CT%d = %d\n", i, param->LC_tmap_curve_tone[i]);

	pr_debug("diff0 = %d\n", param->LC_tmap_curve_tone[0]);
	for (i = 0; i < 7; i++)
		pr_debug("diff%d = %d\n", i + 1, param->LC_tmap_curve_tone[i + 1] - param->LC_tmap_curve_tone[i]);
	*/
	fwif_color_check_LC_tmap_curve_tone(param->LC_tmap_curve_tone);

	/*
	for (i = 0; i < 8; i++)
		pr_debug("2_CT%d = %d\n", i, param->LC_tmap_curve_tone[i]);

	pr_debug("2_diff0 = %d\n", param->LC_tmap_curve_tone[0]);
	for (i = 0; i < 7; i++)
		pr_debug("2_diff%d = %d\n", i + 1, param->LC_tmap_curve_tone[i + 1] - param->LC_tmap_curve_tone[i]);
	*/
	CurveSelect.lc_tmap_curve0_tone = param->LC_tmap_curve_tone[0];
	CurveSelect.lc_tmap_curve1_tone = param->LC_tmap_curve_tone[1];
	CurveSelect.lc_tmap_curve2_tone = param->LC_tmap_curve_tone[2];
	CurveSelect.lc_tmap_curve3_tone = param->LC_tmap_curve_tone[3];
	CurveSelect.lc_tmap_curve4_tone = param->LC_tmap_curve_tone[4];
	CurveSelect.lc_tmap_curve5_tone = param->LC_tmap_curve_tone[5];
	CurveSelect.lc_tmap_curve6_tone = param->LC_tmap_curve_tone[6];
	CurveSelect.lc_tmap_curve7_tone = param->LC_tmap_curve_tone[7];
	drvif_color_set_LC_ToneMapping_CurveSelect((DRV_LC_ToneMapping_CurveSelect*) &CurveSelect);

	memcpy(delta_grid_X[0], grid0, sizeof(DRV_LC_ToneMapping_Grid0));
	delta_grid_X[0][31]= 4095 - grid0->lc_tmap_g0_th31;
	drvif_color_set_LC_ToneMapping_Grid0(grid0);

	memcpy(delta_grid_X[2], grid2, sizeof(DRV_LC_ToneMapping_Grid2));
	delta_grid_X[2][31]= 4095 - grid2->lc_tmap_g2_th31;
	drvif_color_set_LC_ToneMapping_Grid2(grid2);

	for(i=0;i<32;i++)
	{
		if(i==31)
		{
			delta_grid_X[1][i] = 127;
		}
		else
		{
			delta_grid_X[1][i] = 128;
		}
	}

	for(j=0;j<8;j++)
	{
		int idx;
		
		if(j==0)
			grid_mode = CurveSelect.lc_tmap_curve0_grid;
		else if (j==1)
			grid_mode = CurveSelect.lc_tmap_curve1_grid;
		else if (j==2)
			grid_mode = CurveSelect.lc_tmap_curve2_grid;
		else if (j==3)
			grid_mode = CurveSelect.lc_tmap_curve3_grid;
		else if (j==4)
			grid_mode = CurveSelect.lc_tmap_curve4_grid;
		else if (j==5)
			grid_mode = CurveSelect.lc_tmap_curve5_grid;
		else if (j==6)
			grid_mode = CurveSelect.lc_tmap_curve6_grid;
		else if (j==7)
			grid_mode = CurveSelect.lc_tmap_curve7_grid;

		for(i=0;i<32;i++)
		{
			idx = i+(j*32);

			if(i==31)
				delta_Y= 4095-param->LC_ToneMappingSlopePont_Table[idx];
			else
				delta_Y= param->LC_ToneMappingSlopePont_Table[idx+1]-param->LC_ToneMappingSlopePont_Table[idx];

			if(blending.lc_tmap_slope_unit==0){

				slope[idx] = ((16*delta_Y/delta_grid_X[grid_mode][i])<<16)|param->LC_ToneMappingSlopePont_Table[idx];

			}else{

				slope[idx] = ((32*delta_Y/delta_grid_X[grid_mode][i])<<16)|param->LC_ToneMappingSlopePont_Table[idx];

			}

			//printk("\n slope[%d] =%x \n",i,slope[idx]);
			//printk("\n ori_slope[%d] =%x \n",idx,param->LC_ToneMappingSlopePont_Table[idx]);
			//printk("\n delta_Y=%d ",delta_Y);

		}
	}


	/*for(i=0;i<32;i++)
	{
		printk("\n delta_x0[%d]=%d ",i,delta_grid_X[0][i]);
		printk("\n delta_x1[%d]=%d ",i,delta_grid_X[1][i]);
		printk("\n delta_x2[%d]=%d ",i,delta_grid_X[2][i]);

	}*/

	drvif_color_set_LC_ToneMappingSlopePoint(slope);

}
#if 0/* juwen, move to "tc_hdr", elieli */
unsigned short temp_r[66] = {0};
unsigned short temp_g[66] = {0};
unsigned short temp_b[66] = {0};
void fwif_color_TC_Enable(UINT8 En)
{
	fwif_color_set_TC_Enable(0, 1);
	fwif_color_set_TC_Range_Adaptation(0, 0);
	fwif_color_set_TC_L_Compute(0, 0);
	fwif_color_set_TC_T_Compute(0, 0);
	fwif_color_set_TC_M_Compute(0, 0);
	fwif_color_set_TC_Write_D_LUT(0, 1, 2, 3);
	fwif_color_set_TC_Read_D_LUT(temp_r, temp_g, temp_b);

	fwif_color_set_TC_Write_P_LUT(0, 0);
	fwif_color_set_TC_Read_P_LUT(temp_r);
}

#endif
unsigned char  fwif_color_get_black_detection_info_tv006(VIP_BlackDetectionInfo* BlackDetectionInfo)
{

	static unsigned int cnt;
	_clues *smartPic_clue = fwif_color_Get_SmartPic_clue();
	VIP_BlackDetectionInfo tmp;

	//unsigned short channel;
	if (smartPic_clue == NULL) {
		VIPprintf("smartPic_clue NULL!!!");
		return FALSE;
	}
	if (BlackDetectionInfo == NULL) {
		VIPprintf("BlackDetectionInfo NULL!!!");
		return FALSE;
	}


#ifdef CONFIG_ARM64
	memcpy_fromio(&(tmp), &(smartPic_clue->BlackDetectionInfo), sizeof(VIP_BlackDetectionInfo));
#else
	memcpy(&(tmp), &(smartPic_clue->BlackDetectionInfo), sizeof(VIP_BlackDetectionInfo));
#endif



	fwif_color_ChangeUINT16Endian_Copy((unsigned short*)&(tmp.OriRegion.x), 1, (unsigned short*)&(BlackDetectionInfo->OriRegion.x),0);
	fwif_color_ChangeUINT16Endian_Copy((unsigned short*)&(tmp.OriRegion.y), 1, (unsigned short*)&(BlackDetectionInfo->OriRegion.y),0);
	fwif_color_ChangeUINT16Endian_Copy((unsigned short*)&(tmp.OriRegion.w), 1, (unsigned short*)&(BlackDetectionInfo->OriRegion.w),0);
	fwif_color_ChangeUINT16Endian_Copy((unsigned short*)&(tmp.OriRegion.h), 1, (unsigned short*)&(BlackDetectionInfo->OriRegion.h),0);


	fwif_color_ChangeUINT16Endian_Copy((unsigned short*)&(tmp.ActRegion.x), 1, (unsigned short*)&(BlackDetectionInfo->ActRegion.x),0);
	fwif_color_ChangeUINT16Endian_Copy((unsigned short*)&(tmp.ActRegion.y), 1, (unsigned short*)&(BlackDetectionInfo->ActRegion.y),0);
	fwif_color_ChangeUINT16Endian_Copy((unsigned short*)&(tmp.ActRegion.w), 1, (unsigned short*)&(BlackDetectionInfo->ActRegion.w),0);
	fwif_color_ChangeUINT16Endian_Copy((unsigned short*)&(tmp.ActRegion.h), 1, (unsigned short*)&(BlackDetectionInfo->ActRegion.h),0);




	cnt ++;

	if (tmp.DetectFlag == FALSE)
		return FALSE;

	return TRUE;
}

void fwif_color_set_fcic_TV006(unsigned int *u32pTSCICTbl, unsigned int u32Tscicsize, unsigned char *u8pControlTbl, unsigned int u32Ctrlsize, unsigned char bCtrl)
{
	drvif_color_set_fcic_enable(bCtrl);		/*enable first, then write table ===> OK, write table before enable ==> NG*/
	drvif_color_set_fcic_table(u32pTSCICTbl, u32Tscicsize);
	drvif_color_set_fcic_ctrl(u8pControlTbl, u32Ctrlsize);
	drvif_color_set_fcic_enable(bCtrl);
}

void fwif_colorMap_sRGB_SetMatrix_TV006(unsigned char display, short (*sRGB_Matrix)[3], unsigned char forceUpdate)
{
	extern short sRGB_APPLY[3][3];
	static DRV_Color_Mapping CMTable;
	short sRGB_APPLY_2s_complement[3][3];
	signed short ii, jj;

	if (memcmp(sRGB_APPLY, sRGB_Matrix, sizeof(sRGB_APPLY)) == 0 && !forceUpdate)
		return;
	memcpy(sRGB_APPLY, sRGB_Matrix, sizeof(sRGB_APPLY));

	/*here have to do software clip*/

	CMTable.Color_Mapping_En =0; //disable color mapping first , after check ok , please set enable.
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
			sRGB_APPLY_2s_complement[ii][jj] = sRGB_APPLY[ii][jj] *2 ;
			if(sRGB_APPLY_2s_complement[ii][jj]  > 8191){
				sRGB_APPLY_2s_complement[ii][jj] = 8191 ;
			}
			else if(sRGB_APPLY_2s_complement[ii][jj]  < -8192){
				sRGB_APPLY_2s_complement[ii][jj] = -8192 ;
			}
#if 0
			//2's complement
			if(sRGB_APPLY_2s_complement[ii][jj]  < 0){
				sRGB_APPLY_2s_complement[ii][jj] = 16384 - sRGB_APPLY_2s_complement[ii][jj] ;
			}
			else{
				sRGB_APPLY_2s_complement[ii][jj] = sRGB_APPLY_2s_complement[ii][jj] ;
			}
#endif
		}
	}

	/*juwen, k5lp, for tv006 : LG K5LP only, 2'S complement*/

	CMTable.CM_Data.K11=sRGB_APPLY_2s_complement[0][0];
	CMTable.CM_Data.K12=sRGB_APPLY_2s_complement[0][1];
	CMTable.CM_Data.K13=sRGB_APPLY_2s_complement[0][2];
	CMTable.CM_Data.K21=sRGB_APPLY_2s_complement[1][0];
	CMTable.CM_Data.K22=sRGB_APPLY_2s_complement[1][1];
	CMTable.CM_Data.K23=sRGB_APPLY_2s_complement[1][2];
	CMTable.CM_Data.K31=sRGB_APPLY_2s_complement[2][0];
	CMTable.CM_Data.K32=sRGB_APPLY_2s_complement[2][1];
	CMTable.CM_Data.K33=sRGB_APPLY_2s_complement[2][2];


	//-----------------------------------------------------------------------------------------------

	drvif_color_set_Color_Mapping(display,(DRV_Color_Mapping *)&CMTable);

}

short gamutMatrix_APPLY[3][3] = {{0}};
void fwif_set_Gamut_Matrix_TV006(unsigned char display, short (*gamut_Matrix)[3])
{
	static DRV_Color_Mapping CMTable;
	short APPLY_2s_complement[3][3];
	signed short ii, jj;

	if (memcmp(gamutMatrix_APPLY, gamut_Matrix, sizeof(gamutMatrix_APPLY)) == 0)
		return;
	memcpy(gamutMatrix_APPLY, gamut_Matrix, sizeof(gamutMatrix_APPLY));

	/*here have to do software clip*/

	CMTable.Color_Mapping_En =1;
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
			APPLY_2s_complement[ii][jj] = gamutMatrix_APPLY[ii][jj];
			if(APPLY_2s_complement[ii][jj]  > 8191){
				APPLY_2s_complement[ii][jj] = 8191 ;
			}
			else if(APPLY_2s_complement[ii][jj]  < -8192){
				APPLY_2s_complement[ii][jj] = -8192 ;
			}
#if 0
			//2's complement
			if(sRGB_APPLY_2s_complement[ii][jj]  < 0){
				sRGB_APPLY_2s_complement[ii][jj] = 16384 - sRGB_APPLY_2s_complement[ii][jj] ;
			}
			else{
				sRGB_APPLY_2s_complement[ii][jj] = sRGB_APPLY_2s_complement[ii][jj] ;
			}
#endif
		}
	}

	/*juwen, k5lp, for tv006 : LG K5LP only, 2'S complement*/

	CMTable.CM_Data.K11=APPLY_2s_complement[0][0];
	CMTable.CM_Data.K12=APPLY_2s_complement[0][1];
	CMTable.CM_Data.K13=APPLY_2s_complement[0][2];
	CMTable.CM_Data.K21=APPLY_2s_complement[1][0];
	CMTable.CM_Data.K22=APPLY_2s_complement[1][1];
	CMTable.CM_Data.K23=APPLY_2s_complement[1][2];
	CMTable.CM_Data.K31=APPLY_2s_complement[2][0];
	CMTable.CM_Data.K32=APPLY_2s_complement[2][1];
	CMTable.CM_Data.K33=APPLY_2s_complement[2][2];

	//-----------------------------------------------------------------------------------------------
	fwif_color_WaitFor_DEN_STOP_UZUDTG();
	drvif_color_set_Color_Mapping(display,(DRV_Color_Mapping *)&CMTable);

}
void fwif_color_Set_De_Contour_tv006(RTK_DECONTOUR_T *level)
{

        DRV_LC_decontour ptr_LC_de;
        VIP_I_De_Contour_TBL ptr_i_de;

        drvif_color_get_LC_decontour(&ptr_LC_de);
        drvif_color_Get_I_De_Contour(&ptr_i_de);

        ptr_LC_de.LC_decont_alpha_0 =level->De_Contour_Table.Ddecontour_curve[0];
        ptr_LC_de.LC_decont_alpha_1 =level->De_Contour_Table.Ddecontour_curve[1];
        ptr_LC_de.LC_decont_alpha_2 =level->De_Contour_Table.Ddecontour_curve[2];
        ptr_LC_de.LC_decont_alpha_3 =level->De_Contour_Table.Ddecontour_curve[3];
        ptr_LC_de.LC_decont_alpha_4 =level->De_Contour_Table.Ddecontour_curve[4];
        ptr_LC_de.LC_decont_alpha_5 =level->De_Contour_Table.Ddecontour_curve[5];
        ptr_LC_de.LC_decont_alpha_6 =level->De_Contour_Table.Ddecontour_curve[6];
        ptr_LC_de.LC_decont_alpha_7 =level->De_Contour_Table.Ddecontour_curve[7];
        ptr_LC_de.LC_decont_alpha_8 =level->De_Contour_Table.Ddecontour_curve[8];
        ptr_LC_de.LC_decont_alpha_9 =level->De_Contour_Table.Ddecontour_curve[9];
        ptr_LC_de.LC_decont_alpha_10 =level->De_Contour_Table.Ddecontour_curve[10];
        ptr_LC_de.LC_decont_alpha_11 =level->De_Contour_Table.Ddecontour_curve[11];
        ptr_LC_de.LC_decont_alpha_12 =level->De_Contour_Table.Ddecontour_curve[12];
        ptr_LC_de.LC_decont_alpha_13 =level->De_Contour_Table.Ddecontour_curve[13];
        ptr_LC_de.LC_decont_alpha_14 =level->De_Contour_Table.Ddecontour_curve[14];
        ptr_LC_de.LC_decont_alpha_15 =level->De_Contour_Table.Ddecontour_curve[15];

        if(level->UI_Lv==0)
        {
                ptr_LC_de.LC_decont_en =0;
                ptr_i_de.decont_en = 0;
        }
        else
        {
                ptr_LC_de.LC_decont_en =1;
                ptr_i_de.decont_en = (Scaler_Get_DI_SNR_Borrow_Sram_Mode() == SNR_DI_Sram_VE_USE)?(0):(1);
        }

        ptr_LC_de.LC_decont_shift =2;
        drvif_color_set_LC_decontour(&ptr_LC_de);
        drvif_color_Set_I_De_Contour(&ptr_i_de);

        //scalerVIP_Set_Decontour(level.Idecontour_level);

}

void fwif_color_set_BFI_En_by_InputFrameRate(unsigned char bEn)
{
	bBFI_InputFrameRateEn = bEn;

	return;
}

unsigned char fwif_color_get_BFI_En_by_InputFrameRate(void)
{
	return bBFI_InputFrameRateEn;
}

void fwif_color_set_BFI_En_by_UI(unsigned char bEn)
{
	bBFI_UIEn = bEn;

	return;
}

unsigned char fwif_color_get_BFI_En_by_UI(void)
{
	return bBFI_UIEn;
}

void fwif_color_set_BFI_En(unsigned char bEn)
{
	unsigned char ret;
	unsigned char BFI_SPI_Cmd[2];
	bBFI_En = bEn;
	rtd_pr_vpq_info("bBFI_En=%d,\n", bBFI_En);
	if (0 != (ret = Scaler_SendRPC(SCALERIOC_VIP_RPC_BFI_ENABLE, bEn, 1))){	/*BFI enable information to video FW*/
 		pr_err("ret=%d, SCALERIOC_VIP_RPC_BFI_ENABLE RPC fail !!!\n", ret);
	}
	BFI_SPI_Cmd[0] = 0x10; /*bit 5*/
	if (bBFI_En==0)
		 BFI_SPI_Cmd[1] = 0; /*clear bit*/
	else
		BFI_SPI_Cmd[1] = 1; /*set bit*/
	drvif_HAL_VPQ_LED_LDCtrlSPI(BFI_SPI_Cmd);
	return;
}

unsigned char fwif_color_get_BFI_En(void)
{
	return bBFI_En;
}

unsigned char VIP_GSR2_init_done = 0;
static unsigned int GSR2_ToneMapping_Curve_V4L2[GSR2_IDX] = {
	0,256,512,768,1024,1280,1536,1792,2048,2304,2560,2816,3072,3328,3584,3840,4095
};
static unsigned int GSR2_SatGain_Curve_V4L2[GSR2_IDX] = { 
	4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095,4095
};
static unsigned int GSR2_ToneMapping_Curve_Apply[GSR2_IDX];
static unsigned int GSR2_SatGain_Curve_Apply[GSR2_IDX];
extern DRV_GSR2_Table GSR2_TBL[GSR2_Table_NUM];
char fwif_color_Set_GSR2_MEM_TV006(unsigned int *pTbl_006)
{
	unsigned char i;
	//use table 0
	if (pTbl_006 == NULL) {
		rtd_printk(KERN_EMERG, TAG_NAME, "ERROR, fwif_color_Set_GSR2_MEM_TV006 TBL NULL\n");
		return -1;
	}
#ifdef VPQ_GSR2_in_DDISR
	memcpy(GSR2_ToneMapping_Curve_V4L2, pTbl_006, sizeof(int)*GSR2_IDX);
	memcpy(GSR2_SatGain_Curve_V4L2, (pTbl_006+GSR2_IDX), sizeof(int)*GSR2_IDX);
	i = 0;
#else
	for (i=0;i<GSR2_IDX;i++) {
		GSR2_TBL[0].GSR2_tonemap_curve.gsr2_tonemap_curve[i] = pTbl_006[i];
		GSR2_TBL[0].GSR2_gain_by_sat.gsr2_gain_by_sat_curve[i] = pTbl_006[i + GSR2_IDX];
	}		
	GSR2_TBL[0].GSR2_global_gain = pTbl_006[GSR2_IDX*2];
	
	fwif_color_Set_GSR2(0);
#endif
	return 0;
}

char fwif_color_Set_GSR2_TV006_ISR(unsigned short GSR_Gain)
{
	// "GSR_Gain" comes from osr.ko, and sent to LD, is 12 bit
	unsigned short linear_scale = 256; //	table is 12 bit?
	unsigned char step = 2;	// in 12 bit

	unsigned char i;
	int y, y1, y2, temp;
	unsigned char driver_update_flag = 0;

	if (VIP_GSR2_init_done == 0)
		return 1;
	// re-cal tone mapping curve
	for (i=0;i<GSR2_IDX;i++) {
		y1 = i * linear_scale;
		y2 = GSR2_ToneMapping_Curve_V4L2[i];
		y = y2 + (((y1 - y2) * GSR_Gain)>>12);
		y = (y>4095)?(4095):(y);
		
		// smooth
		temp = GSR2_ToneMapping_Curve_Apply[i];
		if (y > (temp + step)) {
			GSR2_ToneMapping_Curve_Apply[i] = temp + step;
			driver_update_flag = 1;
		} 
		else if (y < (temp - step)) {
			GSR2_ToneMapping_Curve_Apply[i] = temp - step;
			driver_update_flag = 1;
		}
		else if (GSR2_ToneMapping_Curve_Apply[i] != y) {
			GSR2_ToneMapping_Curve_Apply[i] = y;
			driver_update_flag = 1;
		}

		if (driver_update_flag == 1)
			GSR2_TBL[0].GSR2_tonemap_curve.gsr2_tonemap_curve[i] = GSR2_ToneMapping_Curve_Apply[i];
	}

	// re-cal sat curve in 12bit
	for (i=0;i<GSR2_IDX;i++) {
		y1 = (GSR2_IDX - 1) * linear_scale;
		y2 = GSR2_SatGain_Curve_V4L2[i];
		y = y2 + (((y1 - y2) * GSR_Gain)>>12);
		y = (y>4095)?(4095):(y);

		// smooth
		temp = GSR2_SatGain_Curve_Apply[i];
		if (y > (temp + step)) {
			GSR2_SatGain_Curve_Apply[i] = temp + step;
			driver_update_flag = 1;
		} 
		else if (y < (temp - step)) {
			GSR2_SatGain_Curve_Apply[i] = temp - step;
			driver_update_flag = 1;
		}
		else if (GSR2_SatGain_Curve_Apply[i] != y) {
			GSR2_SatGain_Curve_Apply[i] = y;
			driver_update_flag = 1;
		}

		if (driver_update_flag == 1)
			GSR2_TBL[0].GSR2_gain_by_sat.gsr2_gain_by_sat_curve[i] = GSR2_SatGain_Curve_Apply[i]; // 12 bit
		
	}	

	GSR2_TBL[0].GSR2_global_gain = 255;

	if (driver_update_flag == 1) {
		fwif_color_Set_GSR2(0);
#if 0		
		rtd_printk(KERN_INFO, TAG_NAME, "GSR2, TMC=%d,%d,%d,SGC=%d,%d,%d,GSR_gain=%d,TMC[16]=%d,SGC[16]=%d,\n", 
			GSR2_ToneMapping_Curve_V4L2[0], GSR2_ToneMapping_Curve_V4L2[8], GSR2_ToneMapping_Curve_V4L2[16], 
			GSR2_SatGain_Curve_V4L2[0], GSR2_SatGain_Curve_V4L2[8], GSR2_SatGain_Curve_V4L2[16], GSR_Gain, 
			GSR2_TBL[0].GSR2_tonemap_curve.gsr2_tonemap_curve[GSR2_IDX-1],
			GSR2_TBL[0].GSR2_gain_by_sat.gsr2_gain_by_sat_curve[GSR2_IDX-1]);		
#endif
	}

	return 0;
}

char fwif_color_GSR2_init(void)
{
#ifdef VPQ_GSR2_in_DDISR
	unsigned char idx = 0;

	unsigned char i;
	fwif_color_Set_GSR2(idx);
	for (i=0;i<GSR2_IDX;i++) {
		GSR2_ToneMapping_Curve_Apply[i] = GSR2_TBL[idx].GSR2_tonemap_curve.gsr2_tonemap_curve[i];
		GSR2_SatGain_Curve_Apply[i] = GSR2_TBL[idx].GSR2_gain_by_sat.gsr2_gain_by_sat_curve[i];	// 12 bit
	}		
	VIP_GSR2_init_done = 1;
#endif
	return 0;
}



