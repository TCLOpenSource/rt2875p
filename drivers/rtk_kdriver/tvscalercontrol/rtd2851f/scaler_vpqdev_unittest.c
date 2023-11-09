/*Kernel Header file*/
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
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
#include <linux/version.h>

/*RBUS Header file*/
#include <scaler/scalerCommon.h>

/*TVScaler Header file*/
#include "tvscalercontrol/io/ioregdrv.h"
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>
#include <tvscalercontrol/vip/viptable.h>
#include <scaler_vpqmemcdev.h>
#include <tvscalercontrol/vdc/video.h>
//#include <tvscalercontrol/avd/avdctrl.h>
#include "scaler_vpqdev.h"
#include "scaler_vpqdev_unittest.h"
#include "scaler_vscdev.h"
#include <tvscalercontrol/vip/pq_rpc.h>
//#include <mach/RPCDriver.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/pll27x_reg_reg.h>
#include <rbus/sfg_reg.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/gibi_od.h>
#include <tvscalercontrol/vip/ultrazoom.h>
#include <tvscalercontrol/vip/tc_hdr.h>
#include <tvscalercontrol/vip/dcc.h>
#include <tvscalercontrol/vip/di_ma.h>
#include <tvscalercontrol/vip/intra.h>
#include <tvscalercontrol/vip/localcontrast.h>
#include <tvscalercontrol/vip/xc.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/scalerdrv/scalerip.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <rbus/mis_gpio_reg.h>
#include <rtd_log/rtd_module_log.h>
#include <tvscalercontrol/panel/panelapi.h>
//#include <tvscalercontrol/vo/rtk_vo.h>
#include <asm/io.h>


#define TAG_NAME "VPQUT"

#ifdef CONFIG_RTK_FEATURE_FOR_GKI
#define FS_ACCESS_API_SUPPORTED
#endif

//#define	vip_malloc(x)	kmalloc(x, GFP_KERNEL)
//#define	vip_free(x)	kfree(x)

static dev_t vpqut_dev;/*vpqut device number*/
static struct cdev vpqut_cdev;
static struct task_struct *checkusb_tsk;

static struct semaphore VPQUT_Semaphore;

extern PQ_device_struct *g_Share_Memory_PQ_device_struct;

extern ADCGainOffset m_defaultYPbPrGainOffsetData;
extern ADCGainOffset m_defaultVgaGainOffset;
extern VIP_TABLE_DATA_STRUCT m_defaultColorDataTable;
extern VIP_TABLE_DATA_STRUCT m_defaultColorFacTable;
extern VIP_TABLE_DATA_STRUCT_EX m_defaultPictureModeTable;
extern VIP_TABLE_DATA_STRUCT m_defaultColorTempTable;
extern SLR_VIP_TABLE m_defaultVipTable;
extern SLR_VIP_TABLE_CUSTOM_TV001 m_customVipTable;
//extern SLR_VIP_TABLE_CUSTOM_TV002 m_customVipTable_demo_tv002;

extern DRV_RPC_AutoMA_Flag S_RPC_AutoMA_Flag;
extern DRV_VipNewDcti_auto_adjust tVipNewDcti_auto_adjust[DCTI_TABLE_LEVEL_MAX];
extern DRV_VipNewDDcti_Table gVipDNewDcti_Table[DCTI_TABLE_LEVEL_MAX];
extern DRV_VipNewIDcti_Table gVipINewDcti_Table[DCTI_TABLE_LEVEL_MAX];
extern DRV_D_vcti_t gVipVcti_Table[DCTI_TABLE_LEVEL_MAX];
extern DRV_D_vcti_lpf_t gVipVcti_lpf_Table[DCTI_TABLE_LEVEL_MAX];
extern DRV_VipUV_Delay_TOP_Table gVipUV_Delay_TOP_Table;
extern DRV_VipChromaCompensation_t g_Chroma_Compensation_Curve[8];
//extern DRV_Dnoise_Coef Dnoise_Coef[10];
//extern DRV_Dnoise_Level Dnoise_Level[11];
//extern DRV_SU_table_8tap_t SU_table_8tap[9];
//extern DRV_SU_table_6tap_t SU_table_6tap[9];
extern DRV_Dirsu_Table gVipDirsu_Table[MAX_DIRECTIONAL_WEIGHTING_LEVEL];
extern DRV_rtnr_nm_ctrl_t gRtnrNmCtrl[2];
extern DRV_rtnr_nm_thd_t gRtnrNmThd[NM_LEVEL];
extern DRV_di_smd_table_t gSMDtable[7];
extern DRV_di_ma_hme_table gHMETable[gHMETable_MAX];
extern DRV_di_ma_hmc_table gHMCTable[gHMCTable_MAX];
extern DRV_di_ma_pan_table gPanTable[gPANTable_MAX];
extern DRV_film_table_t gFilmInitTable[5];
extern DRV_Intra_Ver2_Table gVipIntra_Set_Ver2_tab;
extern DRV_NEW_UVC gUVCTable[new_UVC_MAX];
extern DRV_Gamma_BS gBSTable[Blue_Stretch_MAX];
extern DRV_SU_PK_Mask gVipSupk_Mask;
//extern DRV_Skipir_Ring gVipSkipir_Ring[11];
//extern DRV_Skipir_Islet gVipSkipir_Islet[11];
extern DRV_EMFMk2 gVipEmfMk2[11];
extern DRV_2Dpk_Seg gVip2Dpk_Seg;
extern DRV_2Dpk_Tex_Detect gVip2Dpk_Tex_Detect;
extern DRV_OSD_sharp gVipOSD_sharp;
extern DRV_D_DLTI_Table D_DLTI_Table[D_DLTI_table_num];
extern DRV_Un_Shp_Mask gVipUn_Shp_Mask;
extern DRV_Color_Mapping gCMTable[Color_Mapping_MAX];
extern RPC_DCC_Curve_boundary_table Curve_boundary_table[Curve_boundary_table_MAX];
extern RPC_DCC_Advance_control_table Advance_control_table[Advance_control_table_MAX];
extern short gColorGamutMatrix_Apply[3][3];
extern unsigned short tYUV2RGB_COEF[YUV2RGB_TBL_SELECT_ITEM_MAX][tUV2RGB_COEF_Items_Max];
extern unsigned char ConBriMapping_Offset_Table[16][VIP_YUV2RGB_LEVEL_SELECT_MAX][2];
extern unsigned char AVBlackLevelMapping_Table[8][3][4];
extern unsigned char ATVBlackLevelMapping_Table[8][3][4];
extern unsigned short tRGB2YUV_COEF[RGB2YUV_COEF_MATRIX_MODE_Max][RGB2YUV_COEF_MATRIX_ITEMS_Max];
extern DRV_Local_Dimming_Table Local_Dimming_Table[LD_Table_NUM];
extern unsigned int LD_Backlight_Profile_Interpolation_table[LD_Table_NUM][BL_Profile_Table_NUM][BL_Profile_Table_ROW][BL_Profile_Table_COLUMN];
extern DRV_LD_Data_Compensation_NewMode_2DTable LD_Data_Compensation_NewMode_2DTable[LD_Table_NUM];
extern DRV_Local_Dimming_InterFace_Table Local_Dimming_InterFace_Table[LD_Table_NUM];
//extern DRV_Local_Contrast_Table Local_Contrast_Table[LC_Table_NUM]; //move to vip table
//extern unsigned int LC_Backlight_Profile_Interpolation_table[LC_Table_NUM][2][125]; //move to vip table
//extern unsigned int LC_ToneMappingSlopePoint_Table[LC_Table_ToneM_NUM][LC_Curve_ToneM_PointSlope]; //move to vip table
extern DRV_di_ma_init_table DI_MA_Init_Table;
extern DRV_di_ma_adjust_table DI_MA_Adjust_Table[DI_MA_Adjust_Table_MAX];
extern DRV_di_TNR_XC_table di_TNR_XC_table[di_TNR_XC_table_MAX];
extern Scaler_DI_Coef_t Scaler_DI_Coef_table[VIP_QUALITY_SOURCE_NUM];
extern DRV_Adaptive_Gamma_Ctrl Adaptive_Gamma_Ctrl_Table[12];
extern unsigned char od_table_test[OD_table_length];
extern DRV_od_table_t OD_table;
extern DRV_od_table_mode_t OD_setting_table[VIP_QUALITY_SOURCE_NUM];
extern DRV_FIR_Coef FIR_Coef_Ctrl[DRV_FIR_Coef_4Tap_NUM];
extern DRV_TwoStepSU_Table TwoStepSU_Table[TwoStepSU_Table_NUM];
extern DRV_ScalingDown_COEF_TAB ScaleDown_COEF_TAB;
extern unsigned char POD_DATA[6][289];
extern unsigned int PCID_table;
extern unsigned int PCID2_Table;
extern unsigned char PCID2_Pol_Table_R;
extern unsigned char PCID2_Pol_Table_G;
extern unsigned char PCID2_Pol_Table_B;
extern DRV_MA_SNR_IESM_Coef pq_misc_MA_SNR_IESM_TBL[MA_SNR_IESM_TBL_MAX];
extern unsigned int t_inv_GAMMA[512];
extern int t_gamma_curve_22[1025];
extern DRV_De_CT_1D DeCT_1D[DeCT_1D_Level_MAX];
extern unsigned short Power1div22Gamma[1025];
extern unsigned short Power22InvGamma[1025];
extern unsigned short LinearInvGamma[1025];
extern unsigned short LEDOutGamma[257];
extern DRV_Vip_Profile_Table gVip_Profile_Table[PROFILE_TABLE_TOTAL];
extern VIP_PQ_ByPass_Struct PQ_ByPass_Struct[VIP_PQ_ByPass_TBL_Max];
extern bool PQ_check_source_table[PQ_check_MAX][VIP_QUALITY_SOURCE_NUM];
extern PQ_check_struct PQ_check_register_table[200];
extern PQ_check_diff_struct PQ_check_register_diff_table[200];
extern VIP_I_De_XC_TBL I_De_XC_TBL[I_De_XC_TBL_Max];
extern VIP_I_De_Contour_TBL I_De_Contour_TBL[I_De_Contour_TBL_Max];
extern unsigned short BOE_RGBW_TAB_160624[16128];
extern unsigned short BOE_RGBW_TAB_160713[16128];
extern VIP_Output_InvOutput_Gamma Output_InvOutput_Gamma[Output_InvOutput_Gamma_TBL_MAX];
//extern DRV_DeMura_CTRL_TBL DeMura_CTRL_TBL;
extern VIP_DeMura_TBL DeMura_TBL;
extern VIP_SU_PK_Coeff SU_PK_Coeff[SU_PK_COEF_MAX];
extern VIP_MA_ChromaError MA_Chroma_Error_Table[t_MA_Chroma_Error_MAX];
extern unsigned int LUT_3D[LUT3D_TBL_Num][LUT3D_TBL_ITEM];
extern UINT8 hdr_table[HDR_TABLE_NUM][HDR_ITEM_MAX];
extern UINT32 hdr_YUV2RGB[VIP_HDR_DM_CSC1_YUV2RGB_TABLE_Max][12];
extern UINT32 hdr_RGB2OPT[HDR_RGB2OPT_TABLE_NUM][9];
extern unsigned int HDR10_3DLUT_17x17x17[17*17*17][3];
//extern UINT32 EOTF_LUT_HLG_DEFAULT[1025];
//extern UINT16 OETF_LUT_HLG_DEFAULT[1025];
extern UINT32 EOTF_LUT_R[1025];
extern UINT16 OETF_LUT_R[1025];
//extern UINT16 Tone_Mapping_LUT_R[129];
extern UINT16 HDR10_3DLUT_24x24x24[HDR_24x24x24_size][3];
extern unsigned short HLG_3DLUT_24x24x24[HDR_24x24x24_size][3];
extern unsigned short HLG_OETF_LUT_R[OETF_size];
extern unsigned int HLG_EOTF_LUT_R[EOTF_size];
#if 0	/* juwen, move to "tc_hdr", elieli */
extern DRV_TCHDR_Table TCHDR_Table[TC_Table_NUM];
extern DRV_TCHDR_COEF_C_lut TCHDR_COEF_C_LUT_TBL;
extern DRV_TCHDR_COEF_I_lut TCHDR_COEF_I_LUT_TBL;
extern DRV_TCHDR_COEF_S_lut TCHDR_COEF_S_LUT_TBL;
extern DRV_TCHDR_COEF_P_lut TCHDR_COEF_P_LUT_TBL;
extern DRV_TCHDR_COEF_D_lut TCHDR_COEF_D_LUT_TBL;
#endif
unsigned int crc32(const void *buf, unsigned int size);

extern unsigned char (*Setup_IMD)(StructDisplayInfo*);

int vpqut_open(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t  vpqut_read(struct file *filep, char *buffer, size_t count, loff_t *offp)
{
	return count;
}

#if IS_ENABLED(CONFIG_RTK_USBDUMP_ENABLE)
#ifdef FS_ACCESS_API_SUPPORTED
static int checkusb_file(void *arg);
#endif
#endif
ssize_t vpqut_write(struct file *filep, const char *buffer, size_t count, loff_t *offp)
{
#if IS_ENABLED(CONFIG_RTK_USBDUMP_ENABLE)
#ifdef FS_ACCESS_API_SUPPORTED
	if(checkusb_tsk == NULL){
		checkusb_tsk = kthread_create(checkusb_file, NULL, "check_usb");
		if (checkusb_tsk) {
			wake_up_process(checkusb_tsk);
		}else{
			rtd_printk(KERN_ERR, TAG_NAME, "scalevpqut: kthread_create error...\n");
		}
	}
#endif
#endif
	return count;
}

int vpqut_release(struct inode *inode, struct file *filep)
{
	return 0;
}
#if IS_ENABLED(CONFIG_RTK_USBDUMP_ENABLE)
#ifdef FS_ACCESS_API_SUPPORTED
static struct file* file_open(const char* path, int flags, int rights) {
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
}

static void file_close(struct file* file) {
	filp_close(file, NULL);
}
#endif
#endif

#ifdef FS_ACCESS_API_SUPPORTED
static int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
	int ret;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	mm_segment_t oldfs;
	oldfs = get_fs();
	set_fs(KERNEL_DS);
#endif
	ret = kernel_read(file, data, size, &offset);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 11, 0))
	set_fs(oldfs);
#endif
	return ret;
}
#endif

#if IS_ENABLED(CONFIG_RTK_USBDUMP_ENABLE)
#ifdef FS_ACCESS_API_SUPPORTED
/*
static int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size) {
	mm_segment_t oldfs;
	int ret;

	oldfs = get_fs();
	set_fs(KERNEL_DS);

	ret = vfs_write(file, data, size, &offset);

	set_fs(oldfs);
	return ret;
}

static int file_sync(struct file* file) {
	vfs_fsync(file, 0);
	return 0;
}
*/
extern int rtk_get_usb_path(char *buf, int buf_len);
static int checkusb_file(void *arg)
{
	char buf[255] = {0};
	unsigned char buff[1] = {1};
	static struct file *vpq_file = NULL;
	VPQC_FILE_struct *file_info=NULL;

	char File_Title[7]={0};
	int File_item = 0;
	int File_start_offset=0;
	int file_info_count;

	sfg_sfg_force_bg_at_dif_RBUS sfg_force_bg_at_dif_reg;
	sfg_force_bg_at_dif_reg.regValue = IoReg_Read32(SFG_SFG_FORCE_BG_AT_DIF_reg);
	while(1) {
		msleep(1000);
		memset(buf,0,sizeof(buf));
		rtk_get_usb_path(buf, sizeof(buf));

		if(strlen(buf)!=0){
			strcat(buf,"/vpqtable.dat");
//			printk(KERN_EMERG "[VPQ USB]checkusb_file	%s\n",buf);
			vpq_file = (file_open(buf, O_RDWR, 0600));
			if (vpq_file) {

				file_read(vpq_file, 0, (void*)&File_Title, sizeof(char)*7);

				if(strcmp(File_Title, "VPQGEN")==0){		// check title
					sfg_force_bg_at_dif_reg.en_force_bg_dif = 1;
					sfg_force_bg_at_dif_reg.bg_dif_blu = 0;
					sfg_force_bg_at_dif_reg.bg_dif_grn = 1023;
					sfg_force_bg_at_dif_reg.bg_dif_red = 0;
					IoReg_Write32(SFG_SFG_FORCE_BG_AT_DIF_reg, sfg_force_bg_at_dif_reg.regValue);


					file_read(vpq_file, 7,(void*) &File_item, sizeof(int));					//get total item number
					file_read(vpq_file, 11,(void*) &File_start_offset, sizeof(int));

					file_info = (VPQC_FILE_struct *)dvr_malloc(sizeof(VPQC_FILE_struct)*File_item);
					if(file_info == NULL){
						printk(KERN_ERR "[VPQ table]checkusb_file alloc memory error\n");
						break;
					}

					file_read(vpq_file, File_start_offset, (void*)file_info, sizeof(VPQC_FILE_struct)*File_item);

					for(file_info_count=0; file_info_count<File_item; file_info_count++){
//						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_m_defaultYPbPrGainOffsetData,(void*) &m_defaultYPbPrGainOffsetData, sizeof(m_defaultYPbPrGainOffsetData));
//						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_m_defaultVgaGainOffset,(void*) &m_defaultVgaGainOffset, sizeof(m_defaultVgaGainOffset));
//						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_m_defaultColorDataTable,(void*) &m_defaultColorDataTable, sizeof(VIP_TABLE_DATA_STRUCT));
//						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_m_defaultColorFacTable,(void*) &m_defaultColorFacTable, sizeof(VIP_TABLE_DATA_STRUCT));
//						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_m_defaultPictureModeTable, (void*)&m_defaultPictureModeTable, sizeof(VIP_TABLE_DATA_STRUCT_EX));
//						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_m_defaultColorTempTable,(void*) &m_defaultColorTempTable, sizeof(VIP_TABLE_DATA_STRUCT));
//						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_m_defaultVipTable, (void*)&m_defaultVipTable, sizeof(m_defaultVipTable));
//						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_m_customVipTable, (void*)&m_customVipTable, sizeof(m_customVipTable));
//						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_m_customVipTable_demo_tv002, &m_customVipTable_demo_tv002, sizeof(m_customVipTable_demo_tv002));

						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_S_RPC_AutoMA_Flag, (void*)&S_RPC_AutoMA_Flag, sizeof(S_RPC_AutoMA_Flag));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_tVipNewDcti_auto_adjust,(void*) &tVipNewDcti_auto_adjust, sizeof(tVipNewDcti_auto_adjust));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gVipDNewDcti_Table, (void*)&gVipDNewDcti_Table, sizeof(gVipDNewDcti_Table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gVipINewDcti_Table,(void*) &gVipINewDcti_Table, sizeof(gVipINewDcti_Table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gVipVcti_Table, (void*)&gVipVcti_Table, sizeof(gVipVcti_Table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gVipVcti_lpf_Table,(void*) &gVipVcti_lpf_Table, sizeof(gVipVcti_lpf_Table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gVipUV_Delay_TOP_Table,(void*) &gVipUV_Delay_TOP_Table, sizeof(gVipUV_Delay_TOP_Table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_g_Chroma_Compensation_Curve,(void*) &g_Chroma_Compensation_Curve, sizeof(g_Chroma_Compensation_Curve));
//						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_Dnoise_Coef, &Dnoise_Coef, sizeof(Dnoise_Coef));
//						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_Dnoise_Level, &Dnoise_Level, sizeof(Dnoise_Level));
					//	VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_SUFIR256_CEC1x00_Hamm, &SUFIR256_CEC1x00_Hamm, sizeof(SUFIR256_CEC1x00_Hamm));
					//	VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_SUFIR256_CEP0x75, &SUFIR256_CEP0x75, sizeof(SUFIR256_CEP0x75));
					//	VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_SUFIR256_PM41x5_32, &SUFIR256_PM41x5_32, sizeof(SUFIR256_PM41x5_32));
					//	VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_SUFIR256_PM46_32, &SUFIR256_PM46_32, sizeof(SUFIR256_PM46_32));
					//	VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_SUFIR256_PM56_32, &SUFIR256_PM56_32, sizeof(SUFIR256_PM56_32));
					//	VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_SUFIR256_CLS0x80, &SUFIR256_CLS0x80, sizeof(SUFIR256_CLS0x80));
					//	VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_SUFIR256_Sinc3z_Hamm, &SUFIR256_Sinc3z_Hamm, sizeof(SUFIR256_Sinc3z_Hamm));
					//	VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_SUFIRcls256_96s2_45, &SUFIRcls256_96s2_45, sizeof(SUFIRcls256_96s2_45));
//						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_SU_table_8tap, &SU_table_8tap, sizeof(SU_table_8tap));
					//	VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_SUFIR192_Sinc2x6_Hamm, &SUFIR192_Sinc2x6_Hamm, sizeof(SUFIR192_Sinc2x6_Hamm));
					//	VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_SUFIR192_PM52_32, &SUFIR192_PM52_32, sizeof(SUFIR192_PM52_32));
					//	VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_SUFIR192_Sinc2x4_Hamm, &SUFIR192_Sinc2x4_Hamm, sizeof(SUFIR192_Sinc2x4_Hamm));
					//	VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_SUFIR192_PM56_32, &SUFIR192_PM56_32, sizeof(SUFIR192_PM56_32));
					//	VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_SUFIR192_PM52_36, &SUFIR192_PM52_36, sizeof(SUFIR192_PM52_36));
					//	VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_SUFIR192_Sinc2x2_Hamm, &SUFIR192_Sinc2x2_Hamm, sizeof(SUFIR192_Sinc2x2_Hamm));
					//	VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_SUFIR192_Hann0x70, &SUFIR192_Hann0x70, sizeof(SUFIR192_Hann0x70));
					//	VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_SUFIRcls192_84s2_55, &SUFIRcls192_84s2_55, sizeof(SUFIRcls192_84s2_55));
//						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_SU_table_6tap, &SU_table_6tap, sizeof(SU_table_6tap));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gVipDirsu_Table, (void*)&gVipDirsu_Table, sizeof(gVipDirsu_Table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gRtnrNmCtrl, (void*)&gRtnrNmCtrl, sizeof(gRtnrNmCtrl));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gRtnrNmThd, (void*)&gRtnrNmThd, sizeof(gRtnrNmThd));

						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gSMDtable, (void*)&gSMDtable, sizeof(gSMDtable));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gHMETable, (void*)&gHMETable, sizeof(gHMETable));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gHMCTable, (void*)&gHMCTable, sizeof(gHMCTable));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gPanTable, (void*)&gPanTable, sizeof(gPanTable));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gFilmInitTable,(void*) &gFilmInitTable, sizeof(gFilmInitTable));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gVipIntra_Set_Ver2_tab, (void*)&gVipIntra_Set_Ver2_tab, sizeof(gVipIntra_Set_Ver2_tab));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gUVCTable,(void*) &gUVCTable, sizeof(gUVCTable));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gBSTable, (void*)&gBSTable, sizeof(gBSTable));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gVipSupk_Mask,(void*) &gVipSupk_Mask, sizeof(gVipSupk_Mask));
//						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gVipSkipir_Ring, &gVipSkipir_Ring, sizeof(gVipSkipir_Ring));
//						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gVipSkipir_Islet, &gVipSkipir_Islet, sizeof(gVipSkipir_Islet));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gVipEmfMk2, (void*)&gVipEmfMk2, sizeof(gVipEmfMk2));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gVip2Dpk_Seg, (void*)&gVip2Dpk_Seg, sizeof(gVip2Dpk_Seg));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gVip2Dpk_Tex_Detect,(void*) &gVip2Dpk_Tex_Detect, sizeof(gVip2Dpk_Tex_Detect));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gVipOSD_sharp, (void*)&gVipOSD_sharp, sizeof(gVipOSD_sharp));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_D_DLTI_Table, (void*)&D_DLTI_Table, sizeof(D_DLTI_Table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gVipUn_Shp_Mask, (void*)&gVipUn_Shp_Mask, sizeof(gVipUn_Shp_Mask));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gCMTable, (void*)&gCMTable, sizeof(gCMTable));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_Curve_boundary_table,(void*) &Curve_boundary_table, sizeof(Curve_boundary_table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_Advance_control_table, (void*)&Advance_control_table, sizeof(Advance_control_table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gColorGamutMatrix_Apply, (void*)&gColorGamutMatrix_Apply, sizeof(gColorGamutMatrix_Apply));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_tYUV2RGB_COEF,(void*) &tYUV2RGB_COEF, sizeof(tYUV2RGB_COEF));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_ConBriMapping_Offset_Table, (void*)&ConBriMapping_Offset_Table, sizeof(ConBriMapping_Offset_Table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_AVBlackLevelMapping_Table, (void*)&AVBlackLevelMapping_Table, sizeof(AVBlackLevelMapping_Table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_ATVBlackLevelMapping_Table, (void*)&ATVBlackLevelMapping_Table, sizeof(ATVBlackLevelMapping_Table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_tRGB2YUV_COEF, (void*)&tRGB2YUV_COEF, sizeof(tRGB2YUV_COEF));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_Local_Dimming_Table,(void*) &Local_Dimming_Table, sizeof(Local_Dimming_Table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_LD_Backlight_Profile_Interpolation_table,(void*) &LD_Backlight_Profile_Interpolation_table, sizeof(LD_Backlight_Profile_Interpolation_table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_LD_Data_Compensation_NewMode_2DTable,(void*) &LD_Data_Compensation_NewMode_2DTable, sizeof(LD_Data_Compensation_NewMode_2DTable));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_Local_Dimming_InterFace_Table, (void*)&Local_Dimming_InterFace_Table, sizeof(Local_Dimming_InterFace_Table));
						//VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_Local_Contrast_Table, (void*)&Local_Contrast_Table, sizeof(Local_Contrast_Table));
						//VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_LC_Backlight_Profile_Interpolation_table,(void*) &LC_Backlight_Profile_Interpolation_table, sizeof(LC_Backlight_Profile_Interpolation_table));
						//VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_LC_ToneMappingSlopePoint_Table, (void*)&LC_ToneMappingSlopePoint_Table, sizeof(LC_ToneMappingSlopePoint_Table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_DI_MA_Init_Table, (void*)&DI_MA_Init_Table, sizeof(DI_MA_Init_Table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_DI_MA_Adjust_Table,(void*) &DI_MA_Adjust_Table, sizeof(DI_MA_Adjust_Table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_di_TNR_XC_table,(void*) &di_TNR_XC_table, sizeof(di_TNR_XC_table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_Scaler_DI_Coef_table, (void*)&Scaler_DI_Coef_table, sizeof(Scaler_DI_Coef_table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_Adaptive_Gamma_Ctrl_Table, (void*)&Adaptive_Gamma_Ctrl_Table, sizeof(Adaptive_Gamma_Ctrl_Table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_od_table_test, (void*)&od_table_test, sizeof(od_table_test));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_OD_table, (void*)&OD_table, sizeof(OD_table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_OD_setting_table, (void*)&OD_setting_table, sizeof(OD_setting_table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_FIR_Coef_Ctrl, (void*)&FIR_Coef_Ctrl, sizeof(FIR_Coef_Ctrl));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_TwoStepSU_Table, (void*)&TwoStepSU_Table, sizeof(TwoStepSU_Table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_ScaleDown_COEF_TAB,(void*) &ScaleDown_COEF_TAB, sizeof(ScaleDown_COEF_TAB));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_POD_DATA, (void*)&POD_DATA, sizeof(POD_DATA));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_PCID_table, (void*)&PCID_table, sizeof(PCID_table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_PCID2_Table, (void*)&PCID2_Table, sizeof(PCID2_Table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_PCID2_Pol_Table_R,(void*) &PCID2_Pol_Table_R, sizeof(PCID2_Pol_Table_R));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_PCID2_Pol_Table_G, (void*)&PCID2_Pol_Table_G, sizeof(PCID2_Pol_Table_G));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_PCID2_Pol_Table_B, (void*)&PCID2_Pol_Table_B, sizeof(PCID2_Pol_Table_B));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_pq_misc_MA_SNR_IESM_TBL, (void*)&pq_misc_MA_SNR_IESM_TBL, sizeof(pq_misc_MA_SNR_IESM_TBL));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_t_inv_GAMMA, (void*)&t_inv_GAMMA, sizeof(t_inv_GAMMA));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_t_gamma_curve_22, (void*)&t_gamma_curve_22, sizeof(t_gamma_curve_22));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_DeCT_1D,(void*) &DeCT_1D, sizeof(DeCT_1D));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_Power1div22Gamma,(void*) &Power1div22Gamma, sizeof(Power1div22Gamma));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_Power22InvGamma,(void*) &Power22InvGamma, sizeof(Power22InvGamma));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_LinearInvGamma, (void*)&LinearInvGamma, sizeof(LinearInvGamma));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_LEDOutGamma, (void*)&LEDOutGamma, sizeof(LEDOutGamma));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_gVip_Profile_Table, (void*)&gVip_Profile_Table, sizeof(gVip_Profile_Table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_PQ_ByPass_Struct, (void*)&PQ_ByPass_Struct, sizeof(PQ_ByPass_Struct));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_PQ_check_source_table, (void*)&PQ_check_source_table, sizeof(PQ_check_source_table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_PQ_check_register_table,(void*) &PQ_check_register_table, sizeof(PQ_check_register_table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_PQ_check_register_diff_table, (void*)&PQ_check_register_diff_table, sizeof(PQ_check_register_diff_table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_I_De_XC_TBL, (void*)&I_De_XC_TBL, sizeof(I_De_XC_TBL));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_I_De_Contour_TBL,(void*) &I_De_Contour_TBL, sizeof(I_De_Contour_TBL));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_BOE_RGBW_TAB_160624, (void*)&BOE_RGBW_TAB_160624, sizeof(BOE_RGBW_TAB_160624));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_BOE_RGBW_TAB_160713,(void*) &BOE_RGBW_TAB_160713, sizeof(BOE_RGBW_TAB_160713));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_Output_InvOutput_Gamma,(void*) &Output_InvOutput_Gamma, sizeof(Output_InvOutput_Gamma));
						//VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_DeMura_CTRL_TBL,(void*) &DeMura_CTRL_TBL, sizeof(DeMura_CTRL_TBL));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_DeMura_TBL,(void*) &DeMura_TBL, sizeof(DeMura_TBL));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_SU_PK_Coeff,(void*) &SU_PK_Coeff, sizeof(SU_PK_Coeff));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_MA_Chroma_Error_Table,(void*) &MA_Chroma_Error_Table, sizeof(MA_Chroma_Error_Table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_LUT_3D,(void*) &LUT_3D, sizeof(LUT_3D));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_hdr_table, (void*)&hdr_table, sizeof(hdr_table));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_hdr_YUV2RGB,(void*) &hdr_YUV2RGB, sizeof(hdr_YUV2RGB));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_hdr_RGB2OPT, (void*)&hdr_RGB2OPT, sizeof(hdr_RGB2OPT));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_HDR10_3DLUT_17x17x17, (void*)&HDR10_3DLUT_17x17x17, sizeof(HDR10_3DLUT_17x17x17));
//						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_EOTF_LUT_HLG_DEFAULT, &EOTF_LUT_HLG_DEFAULT, sizeof(EOTF_LUT_HLG_DEFAULT));
//						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_OETF_LUT_HLG_DEFAULT, &OETF_LUT_HLG_DEFAULT, sizeof(OETF_LUT_HLG_DEFAULT));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_EOTF_LUT_R, (void*)&EOTF_LUT_R, sizeof(EOTF_LUT_R));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_OETF_LUT_R, (void*)&OETF_LUT_R, sizeof(OETF_LUT_R));
//						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_Tone_Mapping_LUT_R, (void*)&Tone_Mapping_LUT_R, sizeof(Tone_Mapping_LUT_R));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_HDR10_3DLUT_24x24x24, (void*)&HDR10_3DLUT_24x24x24, sizeof(HDR10_3DLUT_24x24x24));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_HLG_3DLUT_24x24x24, (void*)&HLG_3DLUT_24x24x24, sizeof(HLG_3DLUT_24x24x24));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_HLG_OETF_LUT_R, (void*)&HLG_OETF_LUT_R, sizeof(HLG_OETF_LUT_R));
						VPQ_GenFile_setfile_data(vpq_file, &file_info[file_info_count], VPQUT_IOC_PQ_CMD_SET_HLG_EOTF_LUT_R, (void*)&HLG_EOTF_LUT_R, sizeof(HLG_EOTF_LUT_R));
					}

					msleep(2000);
					VipTableInit();
//					xLoadDefaultTable();
					Setup_IMD(Scaler_DispGetInfoPtr());
					dvr_free((void *)file_info);
					sfg_force_bg_at_dif_reg.en_force_bg_dif = 0;
					IoReg_Write32(SFG_SFG_FORCE_BG_AT_DIF_reg, sfg_force_bg_at_dif_reg.regValue);
				}

				file_close(vpq_file);
				vpq_file = NULL;
			}


			memset(buf,0,sizeof(buf));
			rtk_get_usb_path(buf, sizeof(buf));
			strcat(buf,"/PQdemo.txt");

			vpq_file = (file_open(buf, O_RDWR, 0600));
			//printk(KERN_EMERG "[VPQ USB]checkusb_file %p,	%d,	%d\n",buf,vpq_file,NULL);
//			printk("[VPQ USB]checkusb_file %s,	%p,	\n",buf,vpq_file);

			if(vpq_file != NULL){
				printk(KERN_EMERG "[VPQ demo]enable PQ demo mode\n");
				(fwif_color_get_demo_callback(DEMO_CALLBACKID_ON_OFF_SWITCH))((void *)buff);
				file_close(vpq_file);
				vpq_file = NULL;
			}

			break;
		}

		if (freezing(current))
			try_to_freeze();
		if (kthread_should_stop())
			break;
	}
	checkusb_tsk = NULL;
	return 0;
}
#endif
#endif

#if 1
int vpqut_copy_vpqtable(struct file *table_file, VPQC_FILE_struct *array_info, unsigned char* array, unsigned int size){
	int ret = 0;
#ifdef FS_ACCESS_API_SUPPORTED
	if(array_info->size != size){
		printk(KERN_ERR "[VPQ table]case %d size error	%d != %d\n",array_info->id, array_info->size, size);
		ret = -1;
	}else{
		file_read(table_file, array_info->index, array, array_info->size);
		if(array_info->crc != crc32(array, size)){
			printk(KERN_ERR "[VPQ table]case %d  write crc error	%x != %x\n",array_info->id, array_info->crc, crc32(array, size));
			ret = -1;
		}
	}
#endif
	return ret;
}
#endif

int VPQ_GenFile_setfile_data(struct file *table_file, VPQC_FILE_struct *file_info, unsigned int id, unsigned char* array , unsigned int size)
{
	if(file_info->id == id)
		vpqut_copy_vpqtable(table_file, file_info, array, size);
	return 0;
}

unsigned int table_size;
extern StructDisplayInfo* Scaler_DispGetInfoPtr(void);
long vpqut_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
	int ret = 0;
	switch (cmd){
		case VPQUT_IOC_REG:
		{
			VPQUT_REG_struct buf;
			if (copy_from_user(&buf, (int __user *)arg, sizeof(VPQUT_REG_struct))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQUT_IOC_REGISTER fail\n");
				ret = -1;
			} else {
				if(buf.type == 'r'){
					printk(KERN_EMERG "VPQUT_IOC_REG read\n");
					buf.value = IoReg_Read32(buf.reg);
					if(copy_to_user((void __user *)arg, (void *)&buf, sizeof(VPQUT_REG_struct))){

						printk(KERN_EMERG "vpqut ioctl VPQUT_IOC_REG fail\n");
						ret = -1;
					}


				}else if(buf.type == 'w'){
					printk(KERN_EMERG "VPQUT_IOC_REG write\n");
					IoReg_Write32(buf.reg, buf.value);
				}
				ret = 0;
			}
		}
		break;
		case VPQUT_IOC_REG_BIT:
		{
			VPQUT_REG_struct buf;
			if (copy_from_user(&buf, (int __user *)arg, sizeof(VPQUT_REG_struct))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQUT_IOC_REGISTER fail\n");
				ret = -1;
			} else {
				if(buf.type == 'r'){
					printk(KERN_EMERG "VPQUT_IOC_REG_BIT read\n");
					buf.value = IoReg_Read32(buf.reg);
					if (buf.bit[0] < 32 && buf.bit[1] <32) {
						buf.value = (buf.value & (andREG_Table2[buf.bit[0]] & andREG_Table1[buf.bit[1]]))>>buf.bit[1];
						if(copy_to_user((void __user *)arg, (void *)&buf, sizeof(VPQUT_REG_struct))){
							ret = -1;
						}
					} else
						ret = -1;
				}else if(buf.type == 'w'){
					printk(KERN_EMERG "VPQUT_IOC_REG_BIT write\n");
					if (buf.bit[0] < 32 && buf.bit[1] <32) {
						buf.value = (IoReg_Read32(buf.reg) & ((andREG_Table1[buf.bit[0]]<<1) | (andREG_Table2[buf.bit[1]]>>1))) | (buf.value<<buf.bit[1]);
						IoReg_Write32(buf.reg, buf.value);
					}
				}
			}
		}
		break;
		case VPQUT_IOC_GET_MEM_INFO:
		{
			int i;
			StructMemBlock MemTag[MEMIDX_UNKNOW];
			for(i=0;i<MEMIDX_UNKNOW;i++){
				MemTag[i].Allocate_StartAddr = drvif_memory_get_MemTag_addr(i)->Allocate_StartAddr;
				MemTag[i].Allocate_VirAddr = drvif_memory_get_MemTag_addr(i)->Allocate_VirAddr;
				MemTag[i].Size = drvif_memory_get_MemTag_addr(i)->Size;
				MemTag[i].Status = drvif_memory_get_MemTag_addr(i)->Status;
				MemTag[i].StartAddr = drvif_memory_get_MemTag_addr(i)->StartAddr;
			}
			if(copy_to_user((void __user *)arg, (void *)&MemTag, sizeof(StructMemBlock)*MEMIDX_UNKNOW)){

				ret =-1;
			}

		}
		break;
		case VPQUT_IOC_SET_DI_MEM:
		{
			void __iomem *base;
			StructMemBlock DI_mem_info;
			DI_mem_info.Allocate_StartAddr = drvif_memory_get_MemTag_addr(MEMIDX_DI)->Allocate_StartAddr;
			DI_mem_info.Allocate_VirAddr = drvif_memory_get_MemTag_addr(MEMIDX_DI)->Allocate_VirAddr;
			DI_mem_info.Size = drvif_memory_get_MemTag_addr(MEMIDX_DI)->Size;
			DI_mem_info.Status = drvif_memory_get_MemTag_addr(MEMIDX_DI)->Status;
			DI_mem_info.StartAddr = drvif_memory_get_MemTag_addr(MEMIDX_DI)->StartAddr;

			base = ioremap(DI_mem_info.StartAddr, DI_mem_info.Size);
			if (copy_from_user(base, (int __user *)arg, DI_mem_info.Size)) {
				printk(KERN_EMERG "vpqut ioctl VPQUT_IOC_SET_DI_MEM fail\n");
				ret = -1;
			}
			iounmap(base);
		}
		break;
		case VPQUT_IOC_GET_DI_MEM:
		{
			void __iomem *base;
			StructMemBlock DI_mem_info;
			DI_mem_info.Allocate_StartAddr = drvif_memory_get_MemTag_addr(MEMIDX_DI)->Allocate_StartAddr;
			DI_mem_info.Allocate_VirAddr = drvif_memory_get_MemTag_addr(MEMIDX_DI)->Allocate_VirAddr;
			DI_mem_info.Size = drvif_memory_get_MemTag_addr(MEMIDX_DI)->Size;
			DI_mem_info.Status = drvif_memory_get_MemTag_addr(MEMIDX_DI)->Status;
			DI_mem_info.StartAddr = drvif_memory_get_MemTag_addr(MEMIDX_DI)->StartAddr;

			base = ioremap(DI_mem_info.StartAddr, DI_mem_info.Size);
			if(copy_to_user((void __user *)arg, base, DI_mem_info.Size)){
				printk(KERN_EMERG "vpqut ioctl VPQUT_IOC_GET_DI_MEM fail\n");
				ret = -1;
			}
			iounmap(base);
		}
		break;
#if 0
		case VPQUT_IOC_GET_VD_MEM_INFO:
		{
			StructMemBlock VD_mem_info = {0};
			VD_mem_info.StartAddr = AVD_vpq_mem_isAllocated();
			VD_mem_info.Size = _SDRAM_3D_COM_USE;
			if(copy_to_user((void __user *)arg, &VD_mem_info, sizeof(StructMemBlock))){
				printk(KERN_EMERG "vpqut ioctl VPQUT_IOC_GET_VD_MEM fail\n");
				ret = -1;
			}
		}
		break;
		case VPQUT_IOC_SET_VD_MEM:
		{
			unsigned int StartAddr;
			void __iomem *base;
			StartAddr = AVD_vpq_mem_isAllocated();

			base = ioremap(StartAddr, _SDRAM_3D_COM_USE);
			if (copy_from_user(base, (int __user *)arg, _SDRAM_3D_COM_USE)) {
				printk(KERN_EMERG "vpqut ioctl VPQUT_IOC_SET_VD_MEM fail\n");
				ret = -1;
			}
			iounmap(base);
		}
		break;
		case VPQUT_IOC_GET_VD_MEM:
		{
			unsigned int StartAddr;
			void __iomem *base;
			StartAddr = AVD_vpq_mem_isAllocated();

			base = ioremap(StartAddr, _SDRAM_3D_COM_USE);
			if(copy_to_user((void __user *)arg, base, _SDRAM_3D_COM_USE)){
				printk(KERN_EMERG "vpqut ioctl VPQUT_IOC_GET_VD_MEM fail\n");
				ret = -1;
			}
			iounmap(base);
		}
		break;
#endif
		case VPQUT_IOC_SET_TABLE_SIZE:
		{
			if (copy_from_user((void *)&table_size,(const void __user *) arg, (sizeof(unsigned int)))) {
				printk(KERN_EMERG "vpqut ioctl VPQUT_IOC_SET_TABLE_SIZE fail\n");
				ret = 1;
			}
		}
		break;

#if 1
		case VPQUT_IOC_SET_m_defaultVipTable:
		{
			if(table_size!=sizeof(SLR_VIP_TABLE)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_m_defaultVipTable fail	%d != %zu\n",table_size,sizeof(SLR_VIP_TABLE));
				ret = 2;
				break;
			}
			if (copy_from_user(&m_defaultVipTable, (void __user *)arg, (sizeof(SLR_VIP_TABLE)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_m_defaultVipTable fail\n");
				ret = 1;
				break;
			}
			printk(KERN_EMERG "[rock]vpqut ioctl VPQUT_IOC_SET_m_defaultVipTable ok\n");
			VipTableInit();
//			fwif_color_video_quality_handler();
//			Setup_IMD(Scaler_DispGetInfoPtr());
		}
		break;
		case VPQUT_IOC_SET_Run_Setup_IMD:
		{
			Setup_IMD(Scaler_DispGetInfoPtr());
		}
		break;
		case VPQUT_IOC_SET_S_RPC_AutoMA_Flag:
		{
			if(table_size!=sizeof(S_RPC_AutoMA_Flag)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_S_RPC_AutoMA_Flag fail	%d != %zu\n",table_size,sizeof(S_RPC_AutoMA_Flag));
				ret = 2;
				break;
			}
			if (copy_from_user(&S_RPC_AutoMA_Flag, (void __user *)arg, (sizeof(S_RPC_AutoMA_Flag)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_S_RPC_AutoMA_Flag fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_tVipNewDcti_auto_adjust:
		{
			if(table_size!=sizeof(tVipNewDcti_auto_adjust)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_tVipNewDcti_auto_adjust fail	%d != %zu\n",table_size,sizeof(tVipNewDcti_auto_adjust));
				ret = 2;
				break;
			}
			if (copy_from_user(&tVipNewDcti_auto_adjust, (void __user *)arg, (sizeof(tVipNewDcti_auto_adjust)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_tVipNewDcti_auto_adjust fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_gVipDNewDcti_Table:
		{
			if(table_size!=sizeof(gVipDNewDcti_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gVipDNewDcti_Table fail	%d != %zu\n",table_size,sizeof(gVipDNewDcti_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&gVipDNewDcti_Table, (void __user *)arg, (sizeof(gVipDNewDcti_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gVipDNewDcti_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_gVipINewDcti_Table:
		{
			if(table_size!=sizeof(gVipINewDcti_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gVipINewDcti_Table fail	%d != %zu\n",table_size,sizeof(gVipINewDcti_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&gVipINewDcti_Table, (void __user *)arg, (sizeof(gVipINewDcti_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gVipINewDcti_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_gVipVcti_Table:
		{
			if(table_size!=sizeof(gVipVcti_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gVipVcti_Table fail	%d != %zu\n",table_size,sizeof(gVipVcti_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&gVipVcti_Table, (void __user *)arg, (sizeof(gVipVcti_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gVipVcti_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_gVipVcti_lpf_Table:
		{
			if(table_size!=sizeof(gVipVcti_lpf_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gVipVcti_lpf_Table fail	%d != %zu\n",table_size,sizeof(gVipVcti_lpf_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&gVipVcti_lpf_Table, (void __user *)arg, (sizeof(gVipVcti_lpf_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gVipVcti_lpf_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_gVipUV_Delay_TOP_Table:
		{
			if(table_size!=sizeof(gVipUV_Delay_TOP_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gVipUV_Delay_TOP_Table fail	%d != %zu\n",table_size,sizeof(gVipUV_Delay_TOP_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&gVipUV_Delay_TOP_Table, (void __user *)arg, (sizeof(gVipUV_Delay_TOP_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gVipUV_Delay_TOP_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_g_Chroma_Compensation_Curve:
		{
			if(table_size!=sizeof(g_Chroma_Compensation_Curve)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_g_Chroma_Compensation_Curve fail	%d != %zu\n",table_size,sizeof(g_Chroma_Compensation_Curve));
				ret = 2;
				break;
			}
			if (copy_from_user(&g_Chroma_Compensation_Curve, (void __user *)arg, (sizeof(g_Chroma_Compensation_Curve)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_g_Chroma_Compensation_Curve fail\n");
				ret = 1;
				break;
			}
		}
		break;
#if 0
		case VPQUT_IOC_SET_Dnoise_Coef:
		{
			if(table_size!=sizeof(Dnoise_Coef)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_Dnoise_Coef fail	%d != %zu\n",table_size,sizeof(Dnoise_Coef));
				ret = 2;
				break;
			}
			if (copy_from_user(&Dnoise_Coef, (void __user *)arg, (sizeof(Dnoise_Coef)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_Dnoise_Coef fail\n");
				ret = 1;
				break;
			}
		}
		break;
#endif
#if 0
		case VPQUT_IOC_SET_Dnoise_Level:
		{
			if(table_size!=sizeof(Dnoise_Level)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_Dnoise_Level fail	%d != %zu\n",table_size,sizeof(Dnoise_Level));
				ret = 2;
				break;
			}
			if (copy_from_user(&Dnoise_Level, (void __user *)arg, (sizeof(Dnoise_Level)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_Dnoise_Level fail\n");
				ret = 1;
				break;
			}
		}
		break;
#endif
#if 0
		case VPQUT_IOC_SET_SU_table_8tap:
		{
			if(table_size!=sizeof(SU_table_8tap)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_SU_table_8tap fail	%d != %zu\n",table_size,sizeof(SU_table_8tap));
				ret = 2;
				break;
			}
			if (copy_from_user(&SU_table_8tap, (void __user *)arg, (sizeof(SU_table_8tap)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_SU_table_8tap fail\n");
				ret = 1;
				break;
			}
		}
		break;
#endif
#if 0
		case VPQUT_IOC_SET_SU_table_6tap:
		{
			if(table_size!=sizeof(SU_table_6tap)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_SU_table_6tap fail	%d != %zu\n",table_size,sizeof(SU_table_6tap));
				ret = 2;
				break;
			}
			if (copy_from_user(&SU_table_6tap, (void __user *)arg, (sizeof(SU_table_6tap)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_SU_table_6tap fail\n");
				ret = 1;
				break;
			}
		}
		break;
#endif
		case VPQUT_IOC_SET_gVipDirsu_Table:
		{
			if(table_size!=sizeof(gVipDirsu_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gVipDirsu_Table fail	%d != %zu\n",table_size,sizeof(gVipDirsu_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&gVipDirsu_Table, (void __user *)arg, (sizeof(gVipDirsu_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gVipDirsu_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_PQ_CMD_SET_gRtnrNmCtrl:
		{
			if(table_size!=sizeof(gRtnrNmCtrl)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_PQ_CMD_SET_gRtnrNmCtrl fail	%d != %zu\n",table_size,sizeof(gRtnrNmCtrl));
				ret = 2;
				break;
			}
			if (copy_from_user(&gRtnrNmCtrl, (void __user *)arg, (sizeof(gRtnrNmCtrl)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_PQ_CMD_SET_gRtnrNmCtrl fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_PQ_CMD_SET_gRtnrNmThd:
		{
			if(table_size!=sizeof(gRtnrNmThd)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_PQ_CMD_SET_gRtnrNmThd fail	%d != %zu\n",table_size,sizeof(gRtnrNmThd));
				ret = 2;
				break;
			}
			if (copy_from_user(&gRtnrNmThd, (void __user *)arg, (sizeof(gRtnrNmThd)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_PQ_CMD_SET_gRtnrNmThd fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_gSMDtable:
		{
			if(table_size!=sizeof(gSMDtable)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gSMDtable fail	%d != %zu\n",table_size,sizeof(gSMDtable));
				ret = 2;
				break;
			}
			if (copy_from_user(&gSMDtable, (void __user *)arg, (sizeof(gSMDtable)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gSMDtable fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_gHMETable:
		{
			if(table_size!=sizeof(gHMETable)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gHMETable fail	%d != %zu\n",table_size,sizeof(gHMETable));
				ret = 2;
				break;
			}
			if (copy_from_user(&gHMETable, (void __user *)arg, (sizeof(gHMETable)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gHMETable fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_gHMCTable:
		{
			if(table_size!=sizeof(gHMCTable)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gHMCTable fail	%d != %zu\n",table_size,sizeof(gHMCTable));
				ret = 2;
				break;
			}
			if (copy_from_user(&gHMCTable, (void __user *)arg, (sizeof(gHMCTable)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gHMCTable fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_gPanTable:
		{
			if(table_size!=sizeof(gPanTable)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gPanTable fail	%d != %zu\n",table_size,sizeof(gPanTable));
				ret = 2;
				break;
			}
			if (copy_from_user(&gPanTable, (void __user *)arg, (sizeof(gPanTable)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gPanTable fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_gFilmInitTable:
		{
			if(table_size!=sizeof(gFilmInitTable)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gFilmInitTable fail	%d != %zu\n",table_size,sizeof(gFilmInitTable));
				ret = 2;
				break;
			}
			if (copy_from_user(&gFilmInitTable, (void __user *)arg, (sizeof(gFilmInitTable)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gFilmInitTable fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_gVipIntra_Set_Ver2_tab:
		{
			if(table_size!=sizeof(gVipIntra_Set_Ver2_tab)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gVipIntra_Set_Ver2_tab fail	%d != %zu\n",table_size,sizeof(gVipIntra_Set_Ver2_tab));
				ret = 2;
				break;
			}
			if (copy_from_user(&gVipIntra_Set_Ver2_tab, (void __user *)arg, (sizeof(gVipIntra_Set_Ver2_tab)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gVipIntra_Set_Ver2_tab fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_gUVCTable:
		{
			if(table_size!=sizeof(gUVCTable)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gUVCTable fail	%d != %zu\n",table_size,sizeof(gUVCTable));
				ret = 2;
				break;
			}
			if (copy_from_user(&gUVCTable, (void __user *)arg, (sizeof(gUVCTable)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gUVCTable fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_gBSTable:
		{
			if(table_size!=sizeof(gBSTable)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gBSTable fail	%d != %zu\n",table_size,sizeof(gBSTable));
				ret = 2;
				break;
			}
			if (copy_from_user(&gBSTable, (void __user *)arg, (sizeof(gBSTable)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gBSTable fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_gVipSupk_Mask:
		{
			if(table_size!=sizeof(gVipSupk_Mask)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gVipSupk_Mask fail	%d != %zu\n",table_size,sizeof(gVipSupk_Mask));
				ret = 2;
				break;
			}
			if (copy_from_user(&gVipSupk_Mask, (void __user *)arg, (sizeof(gVipSupk_Mask)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gVipSupk_Mask fail\n");
				ret = 1;
				break;
			}
		}
		break;
#if 0
		case VPQUT_IOC_SET_gVipSkipir_Ring:
		{
			if(table_size!=sizeof(gVipSkipir_Ring)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gVipSkipir_Ring fail	%d != %zu\n",table_size,sizeof(gVipSkipir_Ring));
				ret = 2;
				break;
			}
			if (copy_from_user(&gVipSkipir_Ring, (void __user *)arg, (sizeof(gVipSkipir_Ring)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gVipSkipir_Ring fail\n");
				ret = 1;
				break;
			}
		}
		break;
#endif
#if 0
		case VPQUT_IOC_SET_gVipSkipir_Islet:
		{
			if(table_size!=sizeof(gVipSkipir_Islet)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gVipSkipir_Islet fail	%d != %zu\n",table_size,sizeof(gVipSkipir_Islet));
				ret = 2;
				break;
			}
			if (copy_from_user(&gVipSkipir_Islet, (void __user *)arg, (sizeof(gVipSkipir_Islet)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gVipSkipir_Islet fail\n");
				ret = 1;
				break;
			}
		}
		break;
#endif
		case VPQUT_IOC_SET_gVipEmfMk2:
		{
			if(table_size!=sizeof(gVipEmfMk2)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gVipEmfMk2 fail	%d != %zu\n",table_size,sizeof(gVipEmfMk2));
				ret = 2;
				break;
			}
			if (copy_from_user(&gVipEmfMk2, (void __user *)arg, (sizeof(gVipEmfMk2)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gVipEmfMk2 fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_gVip2Dpk_Seg:
		{
			if(table_size!=sizeof(gVip2Dpk_Seg)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gVip2Dpk_Seg fail	%d != %zu\n",table_size,sizeof(gVip2Dpk_Seg));
				ret = 2;
				break;
			}
			if (copy_from_user(&gVip2Dpk_Seg, (void __user *)arg, (sizeof(gVip2Dpk_Seg)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gVip2Dpk_Seg fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_gVip2Dpk_Tex_Detect:
		{
			if(table_size!=sizeof(gVip2Dpk_Tex_Detect)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gVip2Dpk_Tex_Detect fail	%d != %zu\n",table_size,sizeof(gVip2Dpk_Tex_Detect));
				ret = 2;
				break;
			}
			if (copy_from_user(&gVip2Dpk_Tex_Detect, (void __user *)arg, (sizeof(gVip2Dpk_Tex_Detect)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gVip2Dpk_Tex_Detect fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_gVipOSD_sharp:
		{
			if(table_size!=sizeof(gVipOSD_sharp)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gVipOSD_sharp fail	%d != %zu\n",table_size,sizeof(gVipOSD_sharp));
				ret = 2;
				break;
			}
			if (copy_from_user(&gVipOSD_sharp, (void __user *)arg, (sizeof(gVipOSD_sharp)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gVipOSD_sharp fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_D_DLTI_Table:
		{
			if(table_size!=sizeof(D_DLTI_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_D_DLTI_Table fail	%d != %zu\n",table_size,sizeof(D_DLTI_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&D_DLTI_Table, (void __user *)arg, (sizeof(D_DLTI_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_D_DLTI_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_gVipUn_Shp_Mask:
		{
			if(table_size!=sizeof(gVipUn_Shp_Mask)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gVipUn_Shp_Mask fail	%d != %zu\n",table_size,sizeof(gVipUn_Shp_Mask));
				ret = 2;
				break;
			}
			if (copy_from_user(&gVipUn_Shp_Mask, (void __user *)arg, (sizeof(gVipUn_Shp_Mask)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gVipUn_Shp_Mask fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_gCMTable:
		{
			if(table_size!=sizeof(gCMTable)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gCMTable fail	%d != %zu\n",table_size,sizeof(gCMTable));
				ret = 2;
				break;
			}
			if (copy_from_user(&gCMTable, (void __user *)arg, (sizeof(gCMTable)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gCMTable fail\n");
				ret = 1;
				break;
			}
		}
		break;
		case VPQUT_IOC_SET_Curve_boundary_table:
		{
			if(table_size!=sizeof(Curve_boundary_table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_Curve_boundary_table fail	%d != %zu\n",table_size,sizeof(Curve_boundary_table));
				ret = 2;
				break;
			}
			if (copy_from_user(&Curve_boundary_table, (void __user *)arg, (sizeof(Curve_boundary_table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_Curve_boundary_table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_Advance_control_table:
		{
			if(table_size!=sizeof(Advance_control_table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_Advance_control_table fail	%d != %zu\n",table_size,sizeof(Advance_control_table));
				ret = 2;
				break;
			}
			if (copy_from_user(&Advance_control_table, (void __user *)arg, (sizeof(Advance_control_table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_Advance_control_table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_gColorGamutMatrix_Apply:
		{
			if(table_size!=sizeof(gColorGamutMatrix_Apply)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gColorGamutMatrix_Apply fail	%d != %zu\n",table_size,sizeof(gColorGamutMatrix_Apply));
				ret = 2;
				break;
			}
			if (copy_from_user(&gColorGamutMatrix_Apply, (void __user *)arg, (sizeof(gColorGamutMatrix_Apply)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gColorGamutMatrix_Apply fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_tYUV2RGB_COEF:
		{
			if(table_size!=sizeof(tYUV2RGB_COEF)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_tYUV2RGB_COEF fail	%d != %zu\n",table_size,sizeof(tYUV2RGB_COEF));
				ret = 2;
				break;
			}
			if (copy_from_user(&tYUV2RGB_COEF, (void __user *)arg, (sizeof(tYUV2RGB_COEF)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_tYUV2RGB_COEF fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_ConBriMapping_Offset_Table:
		{
			if(table_size!=sizeof(ConBriMapping_Offset_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_ConBriMapping_Offset_Table fail	%d != %zu\n",table_size,sizeof(ConBriMapping_Offset_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&ConBriMapping_Offset_Table, (void __user *)arg, (sizeof(ConBriMapping_Offset_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_ConBriMapping_Offset_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_AVBlackLevelMapping_Table:
		{
			if(table_size!=sizeof(AVBlackLevelMapping_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_AVBlackLevelMapping_Table fail	%d != %zu\n",table_size,sizeof(AVBlackLevelMapping_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&AVBlackLevelMapping_Table, (void __user *)arg, (sizeof(AVBlackLevelMapping_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_AVBlackLevelMapping_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_ATVBlackLevelMapping_Table:
		{
			if(table_size!=sizeof(ATVBlackLevelMapping_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_ATVBlackLevelMapping_Table fail	%d != %zu\n",table_size,sizeof(ATVBlackLevelMapping_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&ATVBlackLevelMapping_Table, (void __user *)arg, (sizeof(ATVBlackLevelMapping_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_ATVBlackLevelMapping_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_tRGB2YUV_COEF:
		{
			if(table_size!=sizeof(tRGB2YUV_COEF)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_tRGB2YUV_COEF fail	%d != %zu\n",table_size,sizeof(tRGB2YUV_COEF));
				ret = 2;
				break;
			}
			if (copy_from_user(&tRGB2YUV_COEF, (void __user *)arg, (sizeof(tRGB2YUV_COEF)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_tRGB2YUV_COEF fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_Local_Dimming_Table:
		{
			if(table_size!=sizeof(Local_Dimming_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_Local_Dimming_Table fail	%d != %zu\n",table_size,sizeof(Local_Dimming_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&Local_Dimming_Table, (void __user *)arg, (sizeof(Local_Dimming_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_Local_Dimming_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_LD_Backlight_Profile_Interpolation_table:
		{
			if(table_size!=sizeof(LD_Backlight_Profile_Interpolation_table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_LD_Backlight_Profile_Interpolation_table fail	%d != %zu\n",table_size,sizeof(LD_Backlight_Profile_Interpolation_table));
				ret = 2;
				break;
			}
			if (copy_from_user(&LD_Backlight_Profile_Interpolation_table, (void __user *)arg, (sizeof(LD_Backlight_Profile_Interpolation_table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_LD_Backlight_Profile_Interpolation_table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_LD_Data_Compensation_NewMode_2DTable:
		{
			if(table_size!=sizeof(LD_Data_Compensation_NewMode_2DTable)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_LD_Data_Compensation_NewMode_2DTable fail	%d != %zu\n",table_size,sizeof(LD_Data_Compensation_NewMode_2DTable));
				ret = 2;
				break;
			}
			if (copy_from_user(&LD_Data_Compensation_NewMode_2DTable, (void __user *)arg, (sizeof(LD_Data_Compensation_NewMode_2DTable)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_LD_Data_Compensation_NewMode_2DTable fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_PQ_CMD_SET_Local_Dimming_InterFace_Table:
		{
			if(table_size!=sizeof(Local_Dimming_InterFace_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_PQ_CMD_SET_Local_Dimming_InterFace_Table fail	%d != %zu\n",table_size,sizeof(Local_Dimming_InterFace_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&Local_Dimming_InterFace_Table, (void __user *)arg, (sizeof(Local_Dimming_InterFace_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_PQ_CMD_SET_Local_Dimming_InterFace_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;
/*
		case VPQUT_IOC_SET_Local_Contrast_Table:
		{
			if(table_size!=sizeof(Local_Contrast_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_Local_Contrast_Table fail	%d != %zu\n",table_size,sizeof(Local_Contrast_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&Local_Contrast_Table, (void __user *)arg, (sizeof(Local_Contrast_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_Local_Contrast_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_LC_Backlight_Profile_Interpolation_table:
		{
			if(table_size!=sizeof(LC_Backlight_Profile_Interpolation_table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_LC_Backlight_Profile_Interpolation_table fail	%d != %zu\n",table_size,sizeof(LC_Backlight_Profile_Interpolation_table));
				ret = 2;
				break;
			}
			if (copy_from_user(&LC_Backlight_Profile_Interpolation_table, (void __user *)arg, (sizeof(LC_Backlight_Profile_Interpolation_table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_LC_Backlight_Profile_Interpolation_table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_LC_ToneMappingSlopePoint_Table:
		{
			if(table_size!=sizeof(LC_ToneMappingSlopePoint_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_LC_ToneMappingSlopePoint_Table fail	%d != %zu\n",table_size,sizeof(LC_ToneMappingSlopePoint_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&LC_ToneMappingSlopePoint_Table, (void __user *)arg, (sizeof(LC_ToneMappingSlopePoint_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_LC_ToneMappingSlopePoint_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;
*/
		case VPQUT_IOC_SET_DI_MA_Init_Table:
		{
			if(table_size!=sizeof(DI_MA_Init_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_DI_MA_Init_Table fail	%d != %zu\n",table_size,sizeof(DI_MA_Init_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&DI_MA_Init_Table, (void __user *)arg, (sizeof(DI_MA_Init_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_DI_MA_Init_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_DI_MA_Adjust_Table:
		{
			if(table_size!=sizeof(DI_MA_Adjust_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_DI_MA_Adjust_Table fail	%d != %zu\n",table_size,sizeof(DI_MA_Adjust_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&DI_MA_Adjust_Table, (void __user *)arg, (sizeof(DI_MA_Adjust_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_DI_MA_Adjust_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_di_TNR_XC_table:
		{
			if(table_size!=sizeof(di_TNR_XC_table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_di_TNR_XC_table fail	%d != %zu\n",table_size,sizeof(di_TNR_XC_table));
				ret = 2;
				break;
			}
			if (copy_from_user(&di_TNR_XC_table, (void __user *)arg, (sizeof(di_TNR_XC_table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_di_TNR_XC_table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_Scaler_DI_Coef_table:
		{
			if(table_size!=sizeof(Scaler_DI_Coef_table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_Scaler_DI_Coef_table fail	%d != %zu\n",table_size,sizeof(Scaler_DI_Coef_table));
				ret = 2;
				break;
			}
			if (copy_from_user(&Scaler_DI_Coef_table, (void __user *)arg, (sizeof(Scaler_DI_Coef_table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_Scaler_DI_Coef_table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_Adaptive_Gamma_Ctrl_Table:
		{
			if(table_size!=sizeof(Adaptive_Gamma_Ctrl_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_Adaptive_Gamma_Ctrl_Table fail	%d != %zu\n",table_size,sizeof(Adaptive_Gamma_Ctrl_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&Adaptive_Gamma_Ctrl_Table, (void __user *)arg, (sizeof(Adaptive_Gamma_Ctrl_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_Adaptive_Gamma_Ctrl_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_od_table_test:
		{
			if(table_size!=sizeof(od_table_test)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_od_table_test fail	%d != %zu\n",table_size,sizeof(od_table_test));
				ret = 2;
				break;
			}
			if (copy_from_user(&od_table_test, (void __user *)arg, (sizeof(od_table_test)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_od_table_test fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_OD_table:
		{
			if(table_size!=sizeof(OD_table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_OD_table fail	%d != %zu\n",table_size,sizeof(OD_table));
				ret = 2;
				break;
			}
			if (copy_from_user(&OD_table, (void __user *)arg, (sizeof(OD_table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_OD_table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_OD_setting_table:
		{
			if(table_size!=sizeof(OD_setting_table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_OD_setting_table fail	%d != %zu\n",table_size,sizeof(OD_setting_table));
				ret = 2;
				break;
			}
			if (copy_from_user(&OD_setting_table, (void __user *)arg, (sizeof(OD_setting_table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_OD_setting_table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_FIR_Coef_Ctrl:
		{
			if(table_size!=sizeof(FIR_Coef_Ctrl)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_FIR_Coef_Ctrl fail	%d != %zu\n",table_size,sizeof(FIR_Coef_Ctrl));
				ret = 2;
				break;
			}
			if (copy_from_user(&FIR_Coef_Ctrl, (void __user *)arg, (sizeof(FIR_Coef_Ctrl)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_FIR_Coef_Ctrl fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_TwoStepSU_Table:
		{
			if(table_size!=sizeof(TwoStepSU_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_TwoStepSU_Table fail	%d != %zu\n",table_size,sizeof(TwoStepSU_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&TwoStepSU_Table, (void __user *)arg, (sizeof(TwoStepSU_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_TwoStepSU_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_ScaleDown_COEF_TAB:
		{
			if(table_size!=sizeof(ScaleDown_COEF_TAB)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_ScaleDown_COEF_TAB fail	%d != %zu\n",table_size,sizeof(ScaleDown_COEF_TAB));
				ret = 2;
				break;
			}
			if (copy_from_user(&ScaleDown_COEF_TAB, (void __user *)arg, (sizeof(ScaleDown_COEF_TAB)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_ScaleDown_COEF_TAB fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_POD_DATA:
		{
			if(table_size!=sizeof(POD_DATA)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_POD_DATA fail	%d != %zu\n",table_size,sizeof(POD_DATA));
				ret = 2;
				break;
			}
			if (copy_from_user(&POD_DATA, (void __user *)arg, (sizeof(POD_DATA)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_POD_DATA fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_PCID_table:
		{
			if(table_size!=sizeof(PCID_table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_PCID_table fail	%d != %zu\n",table_size,sizeof(PCID_table));
				ret = 2;
				break;
			}
			if (copy_from_user(&PCID_table, (void __user *)arg, (sizeof(PCID_table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_PCID_table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_PCID2_Table:
		{
			if(table_size!=sizeof(PCID2_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_PCID2_Table fail	%d != %zu\n",table_size,sizeof(PCID2_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&PCID2_Table, (void __user *)arg, (sizeof(PCID2_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_PCID2_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_PCID2_Pol_Table_R:
		{
			if(table_size!=sizeof(PCID2_Pol_Table_R)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_PCID2_Pol_Table_R fail	%d != %zu\n",table_size,sizeof(PCID2_Pol_Table_R));
				ret = 2;
				break;
			}
			if (copy_from_user(&PCID2_Pol_Table_R, (void __user *)arg, (sizeof(PCID2_Pol_Table_R)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_PCID2_Pol_Table_R fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_PCID2_Pol_Table_G:
		{
			if(table_size!=sizeof(PCID2_Pol_Table_G)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_PCID2_Pol_Table_G fail	%d != %zu\n",table_size,sizeof(PCID2_Pol_Table_G));
				ret = 2;
				break;
			}
			if (copy_from_user(&PCID2_Pol_Table_G, (void __user *)arg, (sizeof(PCID2_Pol_Table_G)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_PCID2_Pol_Table_G fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_PCID2_Pol_Table_B:
		{
			if(table_size!=sizeof(PCID2_Pol_Table_B)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_PCID2_Pol_Table_B fail	%d != %zu\n",table_size,sizeof(PCID2_Pol_Table_B));
				ret = 2;
				break;
			}
			if (copy_from_user(&PCID2_Pol_Table_B, (void __user *)arg, (sizeof(PCID2_Pol_Table_B)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_PCID2_Pol_Table_B fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_pq_misc_MA_SNR_IESM_TBL:
		{
			if(table_size!=sizeof(pq_misc_MA_SNR_IESM_TBL)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_ fail	%d != %zu\n",table_size,sizeof(pq_misc_MA_SNR_IESM_TBL));
				ret = 2;
				break;
			}
			if (copy_from_user(&pq_misc_MA_SNR_IESM_TBL, (void __user *)arg, (sizeof(pq_misc_MA_SNR_IESM_TBL)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_ fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_t_inv_GAMMA:
		{
			if(table_size!=sizeof(t_inv_GAMMA)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_t_inv_GAMMA fail	%d != %zu\n",table_size,sizeof(t_inv_GAMMA));
				ret = 2;
				break;
			}
			if (copy_from_user(&t_inv_GAMMA, (void __user *)arg, (sizeof(t_inv_GAMMA)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_t_inv_GAMMA fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_t_gamma_curve_22:
		{
			if(table_size!=sizeof(t_gamma_curve_22)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_t_gamma_curve_22 fail	%d != %zu\n",table_size,sizeof(t_gamma_curve_22));
				ret = 2;
				break;
			}
			if (copy_from_user(&t_gamma_curve_22, (void __user *)arg, (sizeof(t_gamma_curve_22)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_t_gamma_curve_22 fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_PQ_CMD_SET_DeCT_1D:
		{
			if(table_size!=sizeof(DeCT_1D)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_PQ_CMD_SET_DeCT_1D fail	%d != %zu\n",table_size,sizeof(DeCT_1D));
				ret = 2;
				break;
			}
			if (copy_from_user(&DeCT_1D, (void __user *)arg, (sizeof(DeCT_1D)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_PQ_CMD_SET_DeCT_1D fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_Power1div22Gamma:
		{
			if(table_size!=sizeof(Power1div22Gamma)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_Power1div22Gamma fail	%d != %zu\n",table_size,sizeof(Power1div22Gamma));
				ret = 2;
				break;
			}
			if (copy_from_user(&Power1div22Gamma, (void __user *)arg, (sizeof(Power1div22Gamma)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_Power1div22Gamma fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_Power22InvGamma:
		{
			if(table_size!=sizeof(Power22InvGamma)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_Power22InvGamma fail	%d != %zu\n",table_size,sizeof(Power22InvGamma));
				ret = 2;
				break;
			}
			if (copy_from_user(&Power22InvGamma, (void __user *)arg, (sizeof(Power22InvGamma)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_Power22InvGamma fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_LinearInvGamma:
		{
			if(table_size!=sizeof(LinearInvGamma)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_LinearInvGamma fail	%d != %zu\n",table_size,sizeof(LinearInvGamma));
				ret = 2;
				break;
			}
			if (copy_from_user(&LinearInvGamma, (void __user *)arg, (sizeof(LinearInvGamma)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_LinearInvGamma fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_LEDOutGamma:
		{
			if(table_size!=sizeof(LEDOutGamma)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_LEDOutGamma fail	%d != %zu\n",table_size,sizeof(LEDOutGamma));
				ret = 2;
				break;
			}
			if (copy_from_user(&LEDOutGamma, (void __user *)arg, (sizeof(LEDOutGamma)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_LEDOutGamma fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_gVip_Profile_Table:
		{
			if(table_size!=sizeof(gVip_Profile_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_gVip_Profile_Table fail	%d != %zu\n",table_size,sizeof(gVip_Profile_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&gVip_Profile_Table, (void __user *)arg, (sizeof(gVip_Profile_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_gVip_Profile_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_PQ_ByPass_Struct:
		{
			if(table_size!=sizeof(PQ_ByPass_Struct)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_PQ_ByPass_Struct fail	%d != %zu\n",table_size,sizeof(PQ_ByPass_Struct));
				ret = 2;
				break;
			}
			if (copy_from_user(&PQ_ByPass_Struct, (void __user *)arg, (sizeof(PQ_ByPass_Struct)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_PQ_ByPass_Struct fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_PQ_check_source_table:
		{
			if(table_size!=sizeof(PQ_check_source_table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_PQ_check_source_table fail	%d != %zu\n",table_size,sizeof(PQ_check_source_table));
				ret = 2;
				break;
			}
			if (copy_from_user(&PQ_check_source_table, (void __user *)arg, (sizeof(PQ_check_source_table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_PQ_check_source_table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_PQ_check_register_table:
		{
			if(table_size!=sizeof(PQ_check_register_table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_PQ_check_register_table fail	%d != %zu\n",table_size,sizeof(PQ_check_register_table));
				ret = 2;
				break;
			}
			if (copy_from_user(&PQ_check_register_table, (void __user *)arg, (sizeof(PQ_check_register_table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_PQ_check_register_table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_PQ_check_register_diff_table:
		{
			if(table_size!=sizeof(PQ_check_register_diff_table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_PQ_check_register_diff_table fail	%d != %zu\n",table_size,sizeof(PQ_check_register_diff_table));
				ret = 2;
				break;
			}
			if (copy_from_user(&PQ_check_register_diff_table, (void __user *)arg, (sizeof(PQ_check_register_diff_table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_PQ_check_register_diff_table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_I_De_XC_TBL:
		{
			if(table_size!=sizeof(I_De_XC_TBL)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_I_De_XC_TBL fail	%d != %zu\n",table_size,sizeof(I_De_XC_TBL));
				ret = 2;
				break;
			}
			if (copy_from_user(&I_De_XC_TBL, (void __user *)arg, (sizeof(I_De_XC_TBL)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_I_De_XC_TBL fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_I_De_Contour_TBL:
		{
			if(table_size!=sizeof(I_De_Contour_TBL)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_I_De_Contour_TBL fail	%d != %zu\n",table_size,sizeof(I_De_Contour_TBL));
				ret = 2;
				break;
			}
			if (copy_from_user(&I_De_Contour_TBL, (void __user *)arg, (sizeof(I_De_Contour_TBL)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_I_De_Contour_TBL fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_BOE_RGBW_TAB_160624:
		{
			if(table_size!=sizeof(BOE_RGBW_TAB_160624)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_BOE_RGBW_TAB_160624 fail	%d != %zu\n",table_size,sizeof(BOE_RGBW_TAB_160624));
				ret = 2;
				break;
			}
			if (copy_from_user(&BOE_RGBW_TAB_160624, (void __user *)arg, (sizeof(BOE_RGBW_TAB_160624)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_BOE_RGBW_TAB_160624 fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_BOE_RGBW_TAB_160713:
		{
			if(table_size!=sizeof(BOE_RGBW_TAB_160713)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_BOE_RGBW_TAB_160713 fail	%d != %zu\n",table_size,sizeof(BOE_RGBW_TAB_160713));
				ret = 2;
				break;
			}
			if (copy_from_user(&BOE_RGBW_TAB_160713, (void __user *)arg, (sizeof(BOE_RGBW_TAB_160713)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_BOE_RGBW_TAB_160713 fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_Output_InvOutput_Gamma:
		{
			if(table_size!=sizeof(Output_InvOutput_Gamma)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_Output_InvOutput_Gamma fail	%d != %zu\n",table_size,sizeof(Output_InvOutput_Gamma));
				ret = 2;
				break;
			}
			if (copy_from_user(&Output_InvOutput_Gamma, (void __user *)arg, (sizeof(Output_InvOutput_Gamma)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_Output_InvOutput_Gamma fail\n");
				ret = 1;
				break;
			}
		}
		break;
#if 0
		case VPQUT_IOC_SET_DeMura_CTRL_TBL:
		{
			if(table_size!=sizeof(DeMura_CTRL_TBL)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_DeMura_CTRL_TBL fail	%d != %zu\n",table_size,sizeof(DeMura_CTRL_TBL));
				ret = 2;
				break;
			}
			if (copy_from_user(&DeMura_CTRL_TBL, (void __user *)arg, (sizeof(DeMura_CTRL_TBL)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_DeMura_CTRL_TBL fail\n");
				ret = 1;
				break;
			}
		}
		break;
#endif
		case VPQUT_IOC_SET_DeMura_TBL:
		{
			if(table_size!=sizeof(DeMura_TBL)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_DeMura_TBL fail	%d != %zu\n",table_size,sizeof(DeMura_TBL));
				ret = 2;
				break;
			}
			if (copy_from_user(&DeMura_TBL, (void __user *)arg, (sizeof(DeMura_TBL)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_DeMura_TBL fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_SU_PK_Coeff:
		{
			if(table_size!=sizeof(SU_PK_Coeff)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_SU_PK_Coeff fail	%d != %zu\n",table_size,sizeof(SU_PK_Coeff));
				ret = 2;
				break;
			}
			if (copy_from_user(&SU_PK_Coeff, (void __user *)arg, (sizeof(SU_PK_Coeff)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_SU_PK_Coeff fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_MA_Chroma_Error_Table:
		{
			if(table_size!=sizeof(MA_Chroma_Error_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_MA_Chroma_Error_Table fail	%d != %zu\n",table_size,sizeof(MA_Chroma_Error_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&MA_Chroma_Error_Table, (void __user *)arg, (sizeof(MA_Chroma_Error_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_MA_Chroma_Error_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_LUT_3D:
		{
			if(table_size!=sizeof(LUT_3D)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_LUT_3D fail	%d != %zu\n",table_size,sizeof(LUT_3D));
				ret = 2;
				break;
			}
			if (copy_from_user(&LUT_3D, (void __user *)arg, (sizeof(LUT_3D)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_LUT_3D fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_hdr_table:
		{
			if(table_size!=sizeof(hdr_table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_hdr_table fail	%d != %zu\n",table_size,sizeof(hdr_table));
				ret = 2;
				break;
			}
			if (copy_from_user(&hdr_table, (void __user *)arg, (sizeof(hdr_table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_hdr_table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_hdr_YUV2RGB:
		{
			if(table_size!=sizeof(hdr_YUV2RGB)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_hdr_YUV2RGB fail	%d != %zu\n",table_size,sizeof(hdr_YUV2RGB));
				ret = 2;
				break;
			}
			if (copy_from_user(&hdr_YUV2RGB, (void __user *)arg, (sizeof(hdr_YUV2RGB)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_hdr_YUV2RGB fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_hdr_RGB2OPT:
		{
			if(table_size!=sizeof(hdr_RGB2OPT)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_hdr_RGB2OPT fail	%d != %zu\n",table_size,sizeof(hdr_RGB2OPT));
				ret = 2;
				break;
			}
			if (copy_from_user(&hdr_RGB2OPT, (void __user *)arg, (sizeof(hdr_RGB2OPT)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_hdr_RGB2OPT fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_HDR10_3DLUT_17x17x17:
		{
			if(table_size!=sizeof(HDR10_3DLUT_17x17x17)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_HDR10_3DLUT_17x17x17 fail	%d != %zu\n",table_size,sizeof(HDR10_3DLUT_17x17x17));
				ret = 2;
				break;
			}
			if (copy_from_user(&HDR10_3DLUT_17x17x17, (void __user *)arg, (sizeof(HDR10_3DLUT_17x17x17)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_HDR10_3DLUT_17x17x17 fail\n");
				ret = 1;
				break;
			}
		}
		break;
#if 0
		case VPQUT_IOC_SET_EOTF_LUT_HLG_DEFAULT:
		{
			if(table_size!=sizeof(EOTF_LUT_HLG_DEFAULT)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_EOTF_LUT_HLG_DEFAULT fail	%d != %zu\n",table_size,sizeof(EOTF_LUT_HLG_DEFAULT));
				ret = 2;
				break;
			}
			if (copy_from_user(&EOTF_LUT_HLG_DEFAULT, (void __user *)arg, (sizeof(EOTF_LUT_HLG_DEFAULT)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_EOTF_LUT_HLG_DEFAULT fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_OETF_LUT_HLG_DEFAULT:
		{
			if(table_size!=sizeof(OETF_LUT_HLG_DEFAULT)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_OETF_LUT_HLG_DEFAULT fail	%d != %zu\n",table_size,sizeof(OETF_LUT_HLG_DEFAULT));
				ret = 2;
				break;
			}
			if (copy_from_user(&OETF_LUT_HLG_DEFAULT, (void __user *)arg, (sizeof(OETF_LUT_HLG_DEFAULT)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_OETF_LUT_HLG_DEFAULT fail\n");
				ret = 1;
				break;
			}
		}
		break;
#endif
		case VPQUT_IOC_SET_EOTF_LUT_R:
		{
			if(table_size!=sizeof(EOTF_LUT_R)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_EOTF_LUT_R fail	%d != %zu\n",table_size,sizeof(EOTF_LUT_R));
				ret = 2;
				break;
			}
			if (copy_from_user(&EOTF_LUT_R, (void __user *)arg, (sizeof(EOTF_LUT_R)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_EOTF_LUT_R fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_OETF_LUT_R:
		{
			if(table_size!=sizeof(OETF_LUT_R)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_OETF_LUT_R fail	%d != %zu\n",table_size,sizeof(OETF_LUT_R));
				ret = 2;
				break;
			}
			if (copy_from_user(&OETF_LUT_R, (void __user *)arg, (sizeof(OETF_LUT_R)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_OETF_LUT_R fail\n");
				ret = 1;
				break;
			}
		}
		break;
#if 0
		case VPQUT_IOC_SET_Tone_Mapping_LUT_R:
		{
			if(table_size!=sizeof(Tone_Mapping_LUT_R)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_Tone_Mapping_LUT_R fail	%d != %zu\n",table_size,sizeof(Tone_Mapping_LUT_R));
				ret = 2;
				break;
			}
			if (copy_from_user(&Tone_Mapping_LUT_R, (void __user *)arg, (sizeof(Tone_Mapping_LUT_R)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_Tone_Mapping_LUT_R fail\n");
				ret = 1;
				break;
			}
		}
		break;
#endif

		case VPQUT_IOC_SET_HDR10_3DLUT_24x24x24:
		{
			if(table_size!=sizeof(HDR10_3DLUT_24x24x24)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_HDR10_3DLUT_24x24x24 fail	%d != %zu\n",table_size,sizeof(HDR10_3DLUT_24x24x24));
				ret = 2;
				break;
			}
			if (copy_from_user(&HDR10_3DLUT_24x24x24, (void __user *)arg, (sizeof(HDR10_3DLUT_24x24x24)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_HDR10_3DLUT_24x24x24 fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_HLG_3DLUT_24x24x24:
		{
			if(table_size!=sizeof(HLG_3DLUT_24x24x24)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_HLG_3DLUT_24x24x24 fail	%d != %zu\n",table_size,sizeof(HLG_3DLUT_24x24x24));
				ret = 2;
				break;
			}
			if (copy_from_user(&HLG_3DLUT_24x24x24, (void __user *)arg, (sizeof(HLG_3DLUT_24x24x24)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_HLG_3DLUT_24x24x24 fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_HLG_OETF_LUT_R:
		{
			if(table_size!=sizeof(HLG_OETF_LUT_R)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_HLG_OETF_LUT_R fail	%d != %zu\n",table_size,sizeof(HLG_OETF_LUT_R));
				ret = 2;
				break;
			}
			if (copy_from_user(&HLG_OETF_LUT_R, (void __user *)arg, (sizeof(HLG_OETF_LUT_R)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_HLG_OETF_LUT_R fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_HLG_EOTF_LUT_R:
		{
			if(table_size!=sizeof(HLG_EOTF_LUT_R)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_HLG_EOTF_LUT_R fail	%d != %zu\n",table_size,sizeof(HLG_EOTF_LUT_R));
				ret = 2;
				break;
			}
			if (copy_from_user(&HLG_EOTF_LUT_R, (void __user *)arg, (sizeof(HLG_EOTF_LUT_R)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_HLG_EOTF_LUT_R fail\n");
				ret = 1;
				break;
			}
		}
		break;
#if 0 	/* juwen, move to "tc_hdr", elieli */
		case VPQUT_IOC_SET_TCHDR_Table:
		{
			if(table_size!=sizeof(TCHDR_Table)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_TCHDR_Table fail	%d != %zu\n",table_size,sizeof(TCHDR_Table));
				ret = 2;
				break;
			}
			if (copy_from_user(&TCHDR_Table, (void __user *)arg, (sizeof(TCHDR_Table)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_TCHDR_Table fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_TCHDR_COEF_C_LUT_TBL:
		{
			if(table_size!=sizeof(TCHDR_COEF_C_LUT_TBL)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_TCHDR_COEF_C_LUT_TBL fail	%d != %zu\n",table_size,sizeof(TCHDR_COEF_C_LUT_TBL));
				ret = 2;
				break;
			}
			if (copy_from_user(&TCHDR_COEF_C_LUT_TBL, (void __user *)arg, (sizeof(TCHDR_COEF_C_LUT_TBL)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_TCHDR_COEF_C_LUT_TBL fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_TCHDR_COEF_I_LUT_TBL:
		{
			if(table_size!=sizeof(TCHDR_COEF_I_LUT_TBL)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_TCHDR_COEF_I_LUT_TBL fail	%d != %zu\n",table_size,sizeof(TCHDR_COEF_I_LUT_TBL));
				ret = 2;
				break;
			}
			if (copy_from_user(&TCHDR_COEF_I_LUT_TBL, (void __user *)arg, (sizeof(TCHDR_COEF_I_LUT_TBL)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_TCHDR_COEF_I_LUT_TBL fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_TCHDR_COEF_S_LUT_TBL:
		{
			if(table_size!=sizeof(TCHDR_COEF_S_LUT_TBL)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_TCHDR_COEF_S_LUT_TBL fail	%d != %zu\n",table_size,sizeof(TCHDR_COEF_S_LUT_TBL));
				ret = 2;
				break;
			}
			if (copy_from_user(&TCHDR_COEF_S_LUT_TBL, (void __user *)arg, (sizeof(TCHDR_COEF_S_LUT_TBL)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_TCHDR_COEF_S_LUT_TBL fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_TCHDR_COEF_P_LUT_TBL:
		{
			if(table_size!=sizeof(TCHDR_COEF_P_LUT_TBL)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_TCHDR_COEF_P_LUT_TBL fail	%d != %zu\n",table_size,sizeof(TCHDR_COEF_P_LUT_TBL));
				ret = 2;
				break;
			}
			if (copy_from_user(&TCHDR_COEF_P_LUT_TBL, (void __user *)arg, (sizeof(TCHDR_COEF_P_LUT_TBL)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_TCHDR_COEF_P_LUT_TBL fail\n");
				ret = 1;
				break;
			}
		}
		break;

		case VPQUT_IOC_SET_TCHDR_COEF_D_LUT_TBL:
		{
			if(table_size!=sizeof(TCHDR_COEF_D_LUT_TBL)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_TCHDR_COEF_D_LUT_TBL fail	%d != %zu\n",table_size,sizeof(TCHDR_COEF_D_LUT_TBL));
				ret = 2;
				break;
			}
			if (copy_from_user(&TCHDR_COEF_D_LUT_TBL, (void __user *)arg, (sizeof(TCHDR_COEF_D_LUT_TBL)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_TCHDR_COEF_D_LUT_TBL fail\n");
				ret = 1;
				break;
			}
		}
		break;
#endif
#if 0
		case VPQUT_IOC_SET_:
		{
			if(table_size!=sizeof(tVipNewDcti_auto_adjust)){
				printk(KERN_EMERG "[error][size]vpqut ioctl VPQUT_IOC_SET_ fail	%d != %zu\n",table_size,sizeof(tVipNewDcti_auto_adjust));
				ret = 2;
				break;
			}
			if (copy_from_user(&tVipNewDcti_auto_adjust, (void __user *)arg, (sizeof(tVipNewDcti_auto_adjust)))) {
				printk(KERN_EMERG "[error][copy_from_user]vpqut ioctl VPQUT_IOC_SET_ fail\n");
				ret = 1;
				break;
			}
		}
		break;
#endif
#endif
	}
	return ret;
}


struct file_operations vpqut_fops = {
	.owner = THIS_MODULE,
	.open = vpqut_open,
	.release = vpqut_release,
	.read  = vpqut_read,
	.write = vpqut_write,
	.unlocked_ioctl = vpqut_ioctl,
#ifdef CONFIG_ARM64
#ifdef CONFIG_COMPAT
	.compat_ioctl = vpqut_ioctl,
#endif
#endif
};

static struct class *vpqut_class = NULL;
static struct platform_device *vpqut_platform_devs = NULL;
static struct platform_driver vpqut_platform_driver = {
	.driver = {
		.name = "vpqutheck",
		.bus = &platform_bus_type,
    },
} ;

static char *vpqut_devnode(struct device *dev, umode_t *mode)
{
	return NULL;
}

int vpqut_module_init(void)
{
	int result;
	result = alloc_chrdev_region(&vpqut_dev, 0, 1, "vpqutdev");

	if (result != 0) {
		rtd_printk(KERN_ERR, TAG_NAME, "Cannot allocate VPQ device number\n");
		return result;
	}

	vpqut_class = class_create(THIS_MODULE, "vpqutdev");
	if (IS_ERR(vpqut_class)) {
		rtd_printk(KERN_ERR, TAG_NAME, "scalevpqut: can not create class...\n");
		result = PTR_ERR(vpqut_class);
		goto fail_class_create;
	}

	vpqut_class->devnode = vpqut_devnode;

	vpqut_platform_devs = platform_device_register_simple("vpqutdev", -1, NULL, 0);
	if (platform_driver_register(&vpqut_platform_driver) != 0) {
		rtd_printk(KERN_ERR, TAG_NAME, "scalevpqut: can not register platform driver...\n");
		result = -ENOMEM;
		goto fail_platform_driver_register;
	}

	cdev_init(&vpqut_cdev, &vpqut_fops);
	result = cdev_add(&vpqut_cdev, vpqut_dev, 1);
	if (result < 0) {
		rtd_printk(KERN_ERR, TAG_NAME, "scalevpqut: can not add character device...\n");
		goto fail_cdev_init;
	}

	device_create(vpqut_class, NULL, vpqut_dev, NULL, "vpqutdev");

	sema_init(&VPQUT_Semaphore, 1);

#if IS_ENABLED(CONFIG_RTK_USBDUMP_ENABLE)
#ifdef FS_ACCESS_API_SUPPORTED
	checkusb_tsk = kthread_create(checkusb_file, NULL, "check_usb");
	if (checkusb_tsk) {
		wake_up_process(checkusb_tsk);
	}else{
		rtd_printk(KERN_ERR, TAG_NAME, "scalevpqut: kthread_create error...\n");
	}
#endif
#endif

	return 0;/*Success*/

fail_cdev_init:
	platform_driver_unregister(&vpqut_platform_driver);
fail_platform_driver_register:
	platform_device_unregister(vpqut_platform_devs);
	vpqut_platform_devs = NULL;
	class_destroy(vpqut_class);
fail_class_create:
	vpqut_class = NULL;
	unregister_chrdev_region(vpqut_dev, 1);
	return result;

}



void __exit vpqut_module_exit(void)
{
	if (vpqut_platform_devs == NULL)
		BUG();

	kthread_stop(checkusb_tsk);

	device_destroy(vpqut_class, vpqut_dev);
	cdev_del(&vpqut_cdev);

	platform_driver_unregister(&vpqut_platform_driver);
	platform_device_unregister(vpqut_platform_devs);
	vpqut_platform_devs = NULL;

	class_destroy(vpqut_class);
	vpqut_class = NULL;

	unregister_chrdev_region(vpqut_dev, 1);

}

#ifdef CONFIG_SUPPORT_SCALER_MODULE
// the module init/exit will be moved to scaler_module.c if scaler was built as a kernel module
#else
module_init(vpqut_module_init);
module_exit(vpqut_module_exit);
#endif
