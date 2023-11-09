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
/*RBUS Header file*/

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
#else
#include <scalercommon/scalerCommon.h>
#endif


/*TVScaler Header file*/
#include "tvscalercontrol/io/ioregdrv.h"
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/scaler/scalercolorlib_tv001.h>
#include <tvscalercontrol/scaler/scalercolorlib_tv002.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>
#include <tvscalercontrol/scaler/scalercolorlib_IPQ.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>
#include <tvscalercontrol/vip/ST2094.h>
//#include <tvscalercontrol/vip/scalerColor_tv010.h>
#include <tvscalercontrol/vip/viptable.h>
#include <scaler_vpqmemcdev.h>
#include "scaler_vpqdev_extern.h"
#include "scaler_vpqdev.h"
#include "scaler_vscdev.h"
#include "ioctrl/vpq/vpq_extern_cmd_id.h"
#include <tvscalercontrol/vip/pq_rpc.h>
//#include <mach/RPCDriver.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/pll27x_reg_reg.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/gibi_od.h>
#include <tvscalercontrol/vip/ultrazoom.h>

#include <tvscalercontrol/i3ddma/i3ddma_drv.h>

#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/scalerdrv/scalerip.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/scalerdrv/zoom_smoothtoggle.h>

//#include <rbus/rbus_DesignSpec_MISC_GPIOReg.h>
#include <rtd_log/rtd_module_log.h>
#if defined(CONFIG_H5CX_SUPPORT)
#include <rtk_kdriver/i2c_h5_customized.h>
#endif
#ifdef CONFIG_CUSTOMER_TV002
#include <rtk_pq_proc.h>
#endif
#include "vgip_isr/scalerAI.h"

#define TAG_NAME "VPQEX"

extern struct semaphore ColorSpace_Semaphore;

static dev_t vpqex_devno;/*vpq device number*/
static struct cdev vpqex_cdev;
unsigned int vpqex_project_id = 0x00060000;
static unsigned char PQ_Dev_Extern_Status = PQ_DEV_EXTERN_NOTHING;
extern struct semaphore ICM_Semaphore;
extern struct semaphore LC_Semaphore;
struct semaphore TV002_PQLib_GeneralFunc_Semaphore;
//extern struct semaphore I_RGB2YUV_Semaphore;
extern DRV_film_mode film_mode;
unsigned char g_tv002_demo_fun_flag = 0;
unsigned char g_pq_bypass_lv = 255;
extern unsigned char g_picmode_Cinema;
unsigned char ioctl_extern_cmd_stop[256] = {0};
extern unsigned char get_MEMC_bypass_status_refer_platform_model(void);
HAL_VPQ_Panel_Dither_T Panel_Dither_Info = {0,0,0};
extern unsigned char write_3dlut_base_table_time;
unsigned char DLG_flag=0;
extern unsigned int D_3DLUT_999_Reseult[VIP_D_3D_LUT_SIZE][VIP_8VERTEX_RGB_MAX];
unsigned char SmartHDRBlendingFlag = 0;
unsigned int pre_sourceMaxL_a = 0;
unsigned char Need_recheck_hdr_type = 0;
extern unsigned int sourceMaxL_a;
BRIGHTNESS_LUT_OETF BRIGHTNESS_LUT_T;
unsigned char invoutputgamma_isr_flag = 0;
unsigned char pre_hdrtype = HAL_VPQ_HDR_MODE_SDR;
//---------------TV030----------------------------
unsigned char cur_src = 0;
unsigned short tICM_bin[VIP_ICM_TBL_X*VIP_ICM_TBL_Y*VIP_ICM_TBL_Z];
unsigned char ColorSpaceMode=0;
unsigned short IPQ_OETF[256];
unsigned char aipq_DynamicContrastLevel=SLR_DCC_HIGH;//0;
#define Bin_Num_Gamma 1024
unsigned char tconoutputgamma_en=0;
unsigned int aipq_confing=0;
unsigned int is_InPmode = 0;
unsigned char GammaBlendingFlag=0;
RTK_VPQ_3DLUT_INDEX dlut_index=0;
unsigned char dlut_UI_en = 0;
extern VIP_DeMura_TBL DeMura_TBL;
GammaCurve output_Gamma = {
	{1,0,1,0},
	{0},
};

extern unsigned char LocalContrast_EN_UI;
extern unsigned char LocalContrast_table;
VIP_YUV2RGB_LEVEL_SELECT uvoffset_Level_UI = VIP_YUV2RGB_LEVEL_OFF;
//----------------------------------------------------
extern int memc_logo_to_demura_drop_limit;
unsigned char save_PQ_Extend_Data_Index[PQ_EXTEND_DATA_ENUM_MAX_NUM] = {0};
#ifdef CONFIG_CUSTOMER_TV002
unsigned char g_bEnable_PQ_extend_data = 1;
#else
unsigned char g_bEnable_PQ_extend_data = 0;
#endif
unsigned char g_bLDinited = 0;
extern unsigned char scalerVIP_access_tv002_style_demo_flag(unsigned char access_mode, unsigned char *pFlag);
void Check_smooth_toggle_update_flag(unsigned char display);
void zoom_update_scaler_info_from_vo_smooth_toggle(unsigned char display,unsigned char scaler_before);

#if IS_ENABLED(CONFIG_RTK_AI_DRV)
TV001_COLOR_TEMP_DATA_S vpqex_color_temp = {512, 512, 512, 512, 512, 512};
#endif

int vpqex_open(struct inode *inode, struct file *filp)
{
	printk("vpqex_open %d\n", __LINE__);
	return 0;
}

ssize_t  vpqex_read(struct file *filep, char *buffer, size_t count, loff_t *offp)
{
	return 0;
}

ssize_t vpqex_write(struct file *filep, const char *buffer, size_t count, loff_t *offp)
{
	long ret = count;
	char cmd_buf[128] = {0};
	char tmp_buf[128] = {0};
	int i;

	//rtd_printk(KERN_EMERG, TAG_NAME, "%s(): count=%d, buf=%p\n", __func__, count, buffer);

	if (count >= 128)
		return -EFAULT;

	if (copy_from_user(tmp_buf, buffer, count))
		return -EFAULT;

	for (i = 0; i < count; i++)
		cmd_buf[i] = tmp_buf[i];

	cmd_buf[127] = 0;
	if (strstr(cmd_buf, "logctrl") == cmd_buf) {
		extern unsigned char gVipDebugLogCtrls[256];
		const char * const delim = " ";
		char *sepstr = cmd_buf;
		char *substr = NULL;
		int cnt = 0;
		long lv[4] = {0};

		substr = strsep(&sepstr, delim);

		do
		{
			if (cnt > 0) {
				if (kstrtol(substr, 0, &lv[cnt-1]) != 0) {
					rtd_printk(KERN_EMERG, TAG_NAME, "error parmater,please use int!!\n");
					break;
				}
				//rtd_printk(KERN_EMERG, TAG_NAME, "lv[%d] = %ld\n", cnt-1, lv[cnt-1]);
			}
			substr = strsep(&sepstr, delim);
			cnt++;
		} while (substr && cnt < 5);

		if (cnt == 5) {
			int idx = 0;
			char name[3][5] = {"MEMC", "VGIP", "VIP"};
			unsigned char mask;

			if (lv[0] > 3 || lv[0] < 1 ||
				lv[1] > 31 || lv[1] < 1 ||
				lv[2] > 7 || lv[2] < 1 ||
				lv[3] > 3 || lv[3] < 0) {
				rtd_printk(KERN_EMERG, TAG_NAME, "error parmater, out range!!\n");
				return -EFAULT;
			}

			idx = lv[0]*64+lv[1]*2+lv[2]/4;
			mask = ~(0x3<<((lv[2]%4)*2));

			if (idx < 256) {
				gVipDebugLogCtrls[idx] = (gVipDebugLogCtrls[idx]&mask)+(lv[3]<<((lv[2]%4)*2));
				rtd_printk(KERN_EMERG, TAG_NAME, "PQ log %s_IP%ld_IF%ld=%ld\n", name[lv[0]-1], lv[1], lv[2], lv[3]);
			}
		} else {
			rtd_printk(KERN_EMERG, TAG_NAME, "Usage : echo logctrl %%FUNC %%IP %%Interface %%Value > /dev/vpqexdev\n");
			rtd_printk(KERN_EMERG, TAG_NAME, "%%FUNC : 1=MEMC, 2=VGIP, 3=VIP\n");
			rtd_printk(KERN_EMERG, TAG_NAME, "%%IP : 1~31\n");
			rtd_printk(KERN_EMERG, TAG_NAME, "%%Interface : 1~7\n");
			rtd_printk(KERN_EMERG, TAG_NAME, "%%Value : 0=No Print, 1=Print, 3=IgnoreEnable, 2=NoDefine\n");
			rtd_printk(KERN_EMERG, TAG_NAME, "example : echo logctrl 2 31 7 1 > /dev/vpqexdev\n");
		}
	} else if (strstr(cmd_buf, "logtime") == cmd_buf) {
		extern unsigned int gDebugPrintDelayTime;
		const char * const delim = " ";
		char *sepstr = cmd_buf;
		char *substr = NULL;
		int cnt = 0;
		long lv[1] = {0};

		substr = strsep(&sepstr, delim);

		do
		{
			if (cnt > 0) {
				if (kstrtol(substr, 0, &lv[cnt-1]) != 0) {
					rtd_printk(KERN_EMERG, TAG_NAME, "error parmater,please use int!!\n");
					break;
				}
				//rtd_printk(KERN_EMERG, TAG_NAME, "lv[%d] = %ld\n", cnt-1, lv[cnt-1]);
			}
			substr = strsep(&sepstr, delim);
			cnt++;
		} while (substr && cnt < 2);

		if (cnt == 2) {
			gDebugPrintDelayTime = lv[0];
			rtd_printk(KERN_EMERG, TAG_NAME, "PQ log delaytime = %ld\n", lv[0]);
		}
	}

	return ret;
}

int vpqex_release(struct inode *inode, struct file *filep)
{
	printk("vpqex_release %d\n", __LINE__);
	return 0;
}

#ifdef CONFIG_PM
static int vpqex_suspend(struct device *p_dev)
{
	return 0;
}

static int vpqex_resume(struct device *p_dev)
{
	return 0;
}
#endif

void vpqex_set_sopq_ShareMem(void)
{
	#ifdef CONFIG_CUSTOMER_TV002
	extern RTK_TV002_CALLBACK_SetShareMemoryArea  cb_RTK_TV002_PQLib_SetShareMemoryArea;
	if ( cb_RTK_TV002_PQLib_SetShareMemoryArea != NULL)
		 cb_RTK_TV002_PQLib_SetShareMemoryArea(Scaler_Get_SoPQLib_ShareMem(), Scaler_Get_SoPQLib_ShareMem_Size());
	#endif
}

void vpqex_boot_init(void)
{

}

unsigned char vpq_extern_ioctl_get_stop_run(unsigned int cmd)
{
	return (ioctl_extern_cmd_stop[_IOC_NR(cmd)&0xff]|ioctl_extern_cmd_stop[0]);
}

unsigned char vpq_extern_ioctl_get_stop_run_by_idx(unsigned char cmd_idx)
{
	return ioctl_extern_cmd_stop[cmd_idx];
}

void vpq_extern_ioctl_set_stop_run_by_idx(unsigned char cmd_idx, unsigned char stop)
{
	ioctl_extern_cmd_stop[cmd_idx] = stop;
}

bool vpqex_skip_middle_ware_picmode_data(unsigned int cmd)
{
	switch(cmd) {
		case VPQ_EXTERN_IOC_SET_PIC_MODE_DATA:
		case VPQ_EXTERN_IOC_SET_PIC_MODE_DATA_defaultPictureModeSet:
		case VPQ_EXTERN_IOC_SET_PIC_MODE_DATA_DolbyHDRPictureModeSet:
		case VPQ_EXTERN_IOC_SET_StructColorDataType:
		case VPQ_EXTERN_IOC_SET_StructColorDataType_defaultSDColorData:
		case VPQ_EXTERN_IOC_SET_StructColorDataType_defaultHDColorData:
		case VPQ_EXTERN_IOC_SET_COLOR_TEMP_DATA:
		case VPQ_EXTERN_IOC_SET_COLOR_TEMP_DATA_defColorTempSet:
		case VPQ_EXTERN_IOC_SET_StructColorDataFacModeType:
		case VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_defaultColorFacMode:
		case VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_AvColorFacMode:
		case VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_YppColorFacMode:
		case VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_VgaColorFacMode:
		case VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_HdmiColorFacMode:
		case VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_HdmiSDColorFacMode:
			return true;
	}
	return false;
}
VIP_MAGIC_GAMMA_Curve_Driver_Data magic_gamma_data;
extern VIP_MAGIC_GAMMA_Curve_Driver_Data g_MagicGammaDriverDataSave;
RTK_TableSize_Gamma Gamma;
UINT32 HistoData_bin[TV006_VPQ_chrm_bin];
UINT32 SatHistoData_bin[COLOR_AutoSat_HISTOGRAM_LEVEL];
unsigned char RGBmode_en=0;
unsigned char g_WBBL_WBBS_enable=0;
extern V_R_R_OD_TABLE_TYPE v_od_table;
extern V_R_R_ACC_TABLE_TYPE v_acc_table;
extern DLG_ACC_TABLE_TYPE dlg_acc_table;
extern DLG_OD_TABLE_TYPE dlg_od_table;
unsigned int is_pwm_ctrl_by_dolby=1; //1: backlight ctrl by dolby ,0: backlight ctrl by fpp

long vpqex_ioctl_tv030(struct file *file, unsigned int cmd,  unsigned long arg)
{
	int ret = 0;

	switch (cmd) {
	case VPQ_EXTERN_IOC_SET_OSDPIC_FLAG_TV030://flag
	{
		RTK_VIP_Flag pFlag;
		SCALER_DISPLAY_DATA sdp_data;
		if (copy_from_user(&pFlag,  (int __user *)arg,sizeof(RTK_VIP_Flag))) {
			printk("[kernel]set VPQ_EXTERN_IOC_SET_OSDPIC_FLAG_TV030 fail\n");
			ret = -1;
		} else {

			sdp_data.h_sta = pFlag.flag_2;
			sdp_data.h_end = pFlag.flag_3;
			sdp_data.v_sta = pFlag.flag_4;
			sdp_data.v_end = pFlag.flag_5;
			Scaler_Set_Partten4AutoGamma_mute(pFlag.flag_0,pFlag.flag_1,&sdp_data);
			RGBmode_en= pFlag.flag_6;
			printk("a_RGBmode_en=%d\n",RGBmode_en);
			ret = 0;
		}
	}
	break;

	case VPQ_EXTERN_IOC_GET_DYNAMIC_RANGE_TV030:
	{
		unsigned char DolbyMode = 0;
		unsigned char HdrMode = 0;
		unsigned char ret_Val = 0;

		SLR_VOINFO* pVOInfo = NULL;
		_system_setting_info *VIP_system_info_structure_table = NULL;

		VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
		pVOInfo = Scaler_VOInfoPointer(Scaler_Get_CurVoInfo_plane());

		if (pVOInfo == NULL || VIP_system_info_structure_table == NULL) {
			rtd_printk(KERN_EMERG, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_DYNAMIC_RANGE_TV030, point NULL\n");
			ret = -1;
		} else {
			if(cur_src == 0)
			{
				DolbyMode = VIP_system_info_structure_table->DolbyHDR_flag;//fwif_vip_DolbyHDR_check(SLR_MAIN_DISPLAY, _SRC_HDMI);
				HdrMode = VIP_system_info_structure_table ->HDR_flag;//fwif_vip_HDR10_check(SLR_MAIN_DISPLAY, _SRC_HDMI);
			} else if(cur_src == 1){
				DolbyMode = VIP_system_info_structure_table->DolbyHDR_flag;//fwif_vip_DolbyHDR_check(SLR_MAIN_DISPLAY, _SRC_VO);
				HdrMode = VIP_system_info_structure_table ->HDR_flag;;//fwif_vip_HDR10_check(SLR_MAIN_DISPLAY, _SRC_VO);
			}

			if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_STATE) == _MODE_STATE_ACTIVE) {
				if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_VDEC) {
					if((DolbyMode == 1)&& (HdrMode == HAL_VPQ_HDR_MODE_SDR)) {
						ret_Val = VPQ_DYNAMIC_RANGE_DOLBY_VISION;
					} else if ((DolbyMode == 0)&& (HdrMode == HAL_VPQ_HDR_MODE_HDR10)) {
						ret_Val = VPQ_DYNAMIC_RANGE_HDR10;
					} else if ((DolbyMode == 0)&& (HdrMode == HAL_VPQ_HDR_MODE_ST2094)) {
						ret_Val = VPQ_DYNAMIC_RANGE_HDR10_PlUS;
					} else if ((DolbyMode == 0)&& (HdrMode == HAL_VPQ_HDR_MODE_HLG)) {
						if (pVOInfo->transfer_characteristics == 14) {	// will be HLG if Scaler_color_HLG_support_HLG14() = 1
							ret_Val = VPQ_DYNAMIC_RANGE_HLG_14;
						} else {
							ret_Val = VPQ_DYNAMIC_RANGE_HLG_18;
						}
					} else if ((DolbyMode == 0)&& (HdrMode == HAL_VPQ_HDR_MODE_SDR)) {
						if (pVOInfo->transfer_characteristics == 14) {
							ret_Val = VPQ_DYNAMIC_RANGE_HLG_14;
						} else {
							ret_Val = VPQ_DYNAMIC_RANGE_SDR;
						}
					} else {
						ret_Val = VPQ_DYNAMIC_RANGE_SDR;
					}

				} else {
					if((DolbyMode == 1)&& (HdrMode == HAL_VPQ_HDR_MODE_SDR)) {
						ret_Val = VPQ_DYNAMIC_RANGE_DOLBY_VISION;
					} else if ((DolbyMode == 0)&& (HdrMode == HAL_VPQ_HDR_MODE_HDR10)) {
						ret_Val = VPQ_DYNAMIC_RANGE_HDR10;
					} else if ((DolbyMode == 0)&& (HdrMode == HAL_VPQ_HDR_MODE_ST2094)) {
						ret_Val = VPQ_DYNAMIC_RANGE_HDR10_PlUS;
					} else if ((DolbyMode == 0)&& (HdrMode == HAL_VPQ_HDR_MODE_HLG)) {
						ret_Val = VPQ_DYNAMIC_RANGE_HLG;
					} else {
						ret_Val = VPQ_DYNAMIC_RANGE_SDR;
					}
				}

				if (copy_to_user((void __user *)arg, (void *)&ret_Val, sizeof(unsigned char))){
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_DYNAMIC_RANGE_TV030 fail\n");
					ret = -1;
				}else{
					ret = 0;
				}

			} else {
				ret = -1;
			}
		}
	}
	break;

	case VPQ_EXTERN_IOC_SET_DYNAMIC_RANGE_CUR_SRC_TV030:
	{
		if (copy_from_user((void *)&cur_src, (const void __user *)arg, sizeof(unsigned char))){
			rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_DYNAMIC_RANGE_CUR_SRC_TV030 fail\n");
			ret = -1;
		} else {
			ret = 0;
		}
	}
	break;

	case VPQ_EXTERN_IOC_SET_ICMTABLE:
	{
		extern SLR_VIP_TABLE* m_pVipTable;
		if (copy_from_user(&tICM_bin[0], (int __user *)arg, (VIP_ICM_TBL_X*VIP_ICM_TBL_Y*VIP_ICM_TBL_Z)*sizeof(unsigned short))) {
			printk("kernel copy VPQ_EXTERN_IOC_SET_ICMTABLE fail\n");
			printk("%s %d\n", __FUNCTION__, __LINE__);
			ret = -1;
			break;
		} else {
			int x,y,z;
			printk("kernel copy VPQ_EXTERN_IOC_SET_ICMTABLE success\n");
			printk("%s %d\n", __FUNCTION__, __LINE__);
			ret = 0;
			for (x=0; x<VIP_ICM_TBL_X;x++) {
				for (y=0;y<VIP_ICM_TBL_Y;y++) {
					for (z=0; z<VIP_ICM_TBL_Z; z++) {
						 m_pVipTable->tICM_ini[x][y][z] = tICM_bin[x*(VIP_ICM_TBL_Y*VIP_ICM_TBL_Z)+y*VIP_ICM_TBL_Z+z];

					}

				}

			}
		}
	}
	break;

	case VPQ_EXTERN_IOC_SET_GAMMA_LEVEL_TV030:
	{
		unsigned char level;
		printk("[TV030]VPQ_EXTERN_IOC_SET_GAMMA_LEVE\n");

		if (copy_from_user(&level,	(void __user *)arg,sizeof(unsigned char))) {
			rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_GAMMA_LEVEL fail\n");
			ret = -1;
		} else {
			Scaler_Set_Gamma_level(level);
			ret = 0;
		}
	}
	break;

	#if 1
	case VPQ_EXTERN_IOC_SET_GAMMA_CURVE://ACC outputgamma
	{
		extern unsigned int TconOutputGamma_R[Bin_Num_Output_Gamma / 2];
		extern unsigned int TconOutputGamma_G[Bin_Num_Output_Gamma / 2];
		extern unsigned int TconOutputGamma_B[Bin_Num_Output_Gamma / 2];
		extern DLG_ACC_TABLE_TYPE dlg_acc_table;
		//unsigned short i=0;

		if (copy_from_user(&output_Gamma,	(int __user *)arg,sizeof(GammaCurve))) {
			ret = -1;
		} else {
			//RBswap_en=Gamma.tGammaTable[2047];
			memcpy((unsigned char*)&TconOutputGamma_R[0], (unsigned char*)&output_Gamma.tGammaTable[0*512], sizeof(unsigned int)*512);
			memcpy((unsigned char*)&TconOutputGamma_G[0], (unsigned char*)&output_Gamma.tGammaTable[1*512], sizeof(unsigned int)*512);
			memcpy((unsigned char*)&TconOutputGamma_B[0], (unsigned char*)&output_Gamma.tGammaTable[2*512], sizeof(unsigned int)*512);

			memcpy(dlg_acc_table.dlg_acc_normal_table.TBL_R,TconOutputGamma_R,sizeof(TconOutputGamma_R));
			memcpy(dlg_acc_table.dlg_acc_normal_table.TBL_G,TconOutputGamma_G,sizeof(TconOutputGamma_G));
			memcpy(dlg_acc_table.dlg_acc_normal_table.TBL_B,TconOutputGamma_B,sizeof(TconOutputGamma_B));

			rtd_printk(KERN_EMERG, TAG_NAME,"---------[TconGamma] load Curve ---------\n");
			//printk("RBswap_en=%d\n",RBswap_en);
		#if 0
			for(i = 0 ; i < 512 ; i++)
				printk("tcon_gamma,r[%d] =0x%08x\n", i, TconOutputGamma_R[i]);
			printk("----------------------------------------------------\n");
			for(i = 0 ; i < 512 ; i++)
				printk("tcon_gamma,g[%d] =0x%08x\n", i, TconOutputGamma_G[i]);
			printk("----------------------------------------------------\n");
			for(i = 0 ; i < 512 ; i++)
				printk("tcon_gamma,b[%d] =0x%08x\n", i, TconOutputGamma_B[i]);
		#endif
		fwif_color_set_OutputGamma_System();
			//tconoutputgamma_en=1;
			ret = 0;
		}
	}
	break;
	#endif

	case VPQ_EXTERN_IOC_SET_PQ_SETGAMMA_TABLESIZE_TV030://Autogamma
	{
		static RTK_TableSize_Gamma Gamma;
		extern unsigned char GammaEnable;

		//unsigned short i=0;
		if (copy_from_user(&Gamma,	(int __user *)arg,sizeof(RTK_TableSize_Gamma))) {
			ret = -1;
		} else {
			//fwif_color_gamma_encode_TableSize(&Gamma);
			fwif_color_gamma_encode_TableSize_2(&Gamma);
			GammaEnable = 1;//Gamma.nGammaEnable;
			//fwif_color_set_gamma_Magic();
			//fwif_color_set_OutputGamma_System();
			/*
			if(Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE) == _MODE_STATE_ACTIVE){
				invoutputgamma_isr_flag = 1;
			}else{
				invoutputgamma_isr_flag = 0;
			}
			*/
			if(invoutputgamma_isr_flag == 0){
				fwif_color_set_InvOutputGamma_System(2);
			}
			printk("VPQ_EXTERN_IOC_SET_PQ_SETGAMMA_TABLESIZE_TV030 \n");
			ret = 0;
		}
	}
	break;

	case VPQ_EXTERN_IOC_GET_MEMCLD_ENABLE_TV030:
	{
		unsigned char bEnable = 0;
		bEnable =Scaler_GetLocalContrastEnable();
		if (Scaler_GetLocalContrastEnable() == FALSE) {
			rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetLocalContrastEnable fail\n");
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg, (void *)&bEnable, sizeof(unsigned char))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_MEMCLD_ENABLE_TV030 fail\n");
				ret = -1;
			} else
				ret = 0;
		}
	}
	break;

	case VPQ_EXTERN_IOC_SET_MEMCLD_ENABLE_TV030:
	{
		unsigned int args = 0;

		if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

			ret = -1;
		} else {
			printk("VPQ_EXTERN_IOC_SET_MEMCLD_ENABLE_TV030 args=%d \n",args);
			LocalContrast_EN_UI=args;

			if(LocalContrast_EN_UI==0){
				Scaler_SetLocalContrastEnable(0);
				//fwif_color_set_LC_Global_Ctrl(0, 9);
				printk("[LC] vpqextern LC UI OFF !!!\n");
			}
			else
			{ //UI=ON
				fwif_color_set_LC_Global_Region_Num_Size(LocalContrast_table, LC_BLOCK_60x34);
				if(drvif_color_get_LC_size_support()==0){
					Scaler_SetLocalContrastEnable(0);
					printk("[LC] vpqextern LC OFF,LC UI ON,but size not support\n");
				}else{
					fwif_color_set_LC_Global_Ctrl(0, LocalContrast_table);
					Scaler_SetLocalContrastEnable(1);
					printk("[LC] vpqextern LC ON,LC UI ON,and size support\n");
				}
			}
			ret = 0;
		}
	}
	break;

	case VPQ_EXTERN_IOC_SET_DECONTOUR_ENABLE_TV030:
	{
		unsigned char value = 0;
		printk("VPQ_EXTERN_IOC_SET_DECONTOUR_ENABLE_TV030\n");
		if (copy_from_user(&value,  (void __user *)arg,sizeof(unsigned char))) {
			rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_DECONTOUR_ENABLE_TV030 fail\n");
			ret = -1;
		} else {
			ret = 0;
			Scaler_Set_I_De_Contour((unsigned char)value);
		}
	}
	break;

	case VPQ_EXTERN_IOC_GET_DECONTOUR_ENABLE_TV030:
	{
		unsigned char value = 0;
		printk("VPQ_EXTERN_IOC_GET_DECONTOUR_ENABLE_TV030\n");
		value = (unsigned char)Scaler_Get_I_De_Contour();
		if (copy_to_user((void __user *)arg, (void *)&value, sizeof(unsigned char))) {
			rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_DECONTOUR_ENABLE_TV030 fail\n");
			ret = -1;
		} else
			ret = 0;
	}
	break;

	case VPQ_EXTERN_IOC_GET_HistoData_Min_TV030:
	{
		signed int DC_min = 0;

		if (fwif_color_get_HistoData_Min(&DC_min) == FALSE) {
			rtd_printk(KERN_ERR, TAG_NAME, "kernel fwif_color_get_HistoData_Min fail\n");
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg, (void *)&DC_min, sizeof(signed int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_IOC_GET_HistoData_Min fail\n");
				ret = -1;
			} else
				ret = 0;
		}
	}
	break;

	case VPQ_EXTERN_IOC_GET_HistoData_Max_TV030:
	{
		signed int DC_max = 0;

		if (fwif_color_get_HistoData_Max(&DC_max) == FALSE) {
			rtd_printk(KERN_ERR, TAG_NAME, "kernel fwif_color_get_HistoData_Max fail\n");
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg, (void *)&DC_max, sizeof(signed int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_IOC_GET_HistoData_Max fail\n");
				ret = -1;
			} else
				ret = 0;
		}
	}
	break;

	case VPQ_EXTERN_IOC_GET_HistoData_APL_TV030:
	{
		unsigned int DC_APL = 0;

		if (fwif_color_get_HistoData_APL(&DC_APL) == FALSE) {
			rtd_printk(KERN_ERR, TAG_NAME, "kernel fwif_color_get_HistoData_APL fail\n");
			ret = -1;
		} else {
			if (drvif_color_get_WB_pattern_on())
				DC_APL = 940;
			if (copy_to_user((void __user *)arg, (void *)&DC_APL, sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_IOC_GET_HistoData_APL fail\n");
				ret = -1;
			} else
				ret = 0;
		}
	}
	break;

	case VPQ_EXTERN_IOC_GET_HDR_HIS_MAXRGB:
	{
		static UINT32 HDR_info_bin[131] = {0};
		if (drvif_DM_HDR10_enable_Get() == 0) {
			ret = -1;
			break;
		}
		if (fwif_color_get_DM2_HDR_histogram_MaxRGB(HDR_info_bin) == 0) {
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg, HDR_info_bin, 131 * 4)) {
				ret = -1;
			} else
				ret = 0;
		}
		break;
	}
	case VPQ_EXTERN_IOC_GET_HDR_PicInfo_TV030:
	{
		static UINT32 HDR_info_bin[131] = {0};
		if (drvif_DM_HDR10_enable_Get() == 0) {
			ret = -1;
			break;
		}
		if (fwif_color_get_DM2_HDR_histogram_TV030(HDR_info_bin) == 0) {
			ret = -1;
		} else {
			if (copy_to_user((void __user *)arg, HDR_info_bin, 131 * 4)) {
				ret = -1;
			} else
				ret = 0;
		}
		break;
	}

	case VPQ_EXTERN_IOC_SET_BRIGHTNESS_LUT_OETF:
	{
		#if 1//def CONFIG_SUPPORT_IPQ
		extern UINT16 OETF_LUT_R[1025];
		extern UINT16 IPQ_OETF_LUT_R[1025];
		extern unsigned int EOTF_LUT_2084_nits[OETF_size];
		extern unsigned char OETF_STOP;//3002
		extern unsigned char OETFCurve_STOP;//3002
		extern unsigned char BBC_Enable;
		extern UINT32 HFC_EOTF_LUT_R[5][1025];
		extern unsigned char EOTF_index;
		extern unsigned char OETF_set_by_user;
		#endif
		_system_setting_info *VIP_system_info_structure_table = NULL;
		VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

		if (copy_from_user(&BRIGHTNESS_LUT_T,  (int __user *)arg,sizeof(BRIGHTNESS_LUT_OETF))) {
			printk("VPQ_EXTERN_IOC_SET_BRIGHTNESS_LUT_OETF, copy fail\n");
			ret = -1;
			break;
		} else {
			memcpy((unsigned char*)&IPQ_OETF[0], (unsigned char*)&BRIGHTNESS_LUT_T.pf_LUT[0], sizeof(short)*256);
			if(BRIGHTNESS_LUT_T.un16_length==256){
				if(VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_SDR)
					printk(" kernel VPQ_EXTERN_IOC_SET_BRIGHTNESS_LUT_OETF,HDR_flag=%d \n",VIP_system_info_structure_table->HDR_flag);
				if (VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_HLG
					|| VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_HDR10
					||VIP_system_info_structure_table->HDR_flag ==HAL_VPQ_HDR_MODE_SDR
				)
				{
					//rtd_printk(KERN_DEBUG, TAG_NAME, "HDR Curve\n");
					#if 1 //def CONFIG_SUPPORT_IPQ
					if(OETF_STOP){
						rtd_printk(KERN_EMERG, TAG_NAME,"OETF_STOP\n");
					}
					else{
						if(OETFCurve_STOP){
							rtd_printk(KERN_EMERG, TAG_NAME, "fixed oetf curve\n");
							fwif_color_set_DM2_OETF_TV006(IPQ_OETF_LUT_R,0);
							drvif_color_set_BBC_shift_bit(4);
						#ifdef isSupport_HDR_3dLUT_24x24x24
							drvif_color_Set_ST2094_3Dlut_CInv(1024<<(9-4), 1);
						#else
							drvif_color_Set_ST2094_3Dlut_CInv(1024<<(9-4), 0);
						#endif
							drvif_color_set_BBC_Enable(1);
						}
						else{
							OETF_set_by_user = 1;
							Scaler_SetBrightness_LUT_OETF(OETF_LUT_R,&BRIGHTNESS_LUT_T);
							fwif_color_set_DM2_OETF_TV006(OETF_LUT_R, 0);//fwif_color_set_DM2_OETF_TV006(OETF_LUT_R, 0);
							if(VIP_system_info_structure_table->HDR_flag ==HAL_VPQ_HDR_MODE_SDR){
								Need_recheck_hdr_type = 1;
							}
							else{
								if(Need_recheck_hdr_type)
									Need_recheck_hdr_type = 0;
							}

						}
						if (VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_HDR10) {
							if(sourceMaxL_a != pre_sourceMaxL_a){
								pre_sourceMaxL_a = sourceMaxL_a;
								drvif_fwif_color_Set_HDR10_EOTF_depNITS();
								fwif_color_set_DM2_EOTF_TV006(EOTF_LUT_2084_nits, 0);
							}
						}else if(VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_HLG){
							if(EOTF_index == 0)
								EOTF_index = 1;
							fwif_color_set_DM2_EOTF_TV006(HFC_EOTF_LUT_R[EOTF_index], 1);
							pre_sourceMaxL_a = 0;
							//printk("[HLG]VPQ_EXTERN_IOC_SET_BRIGHTNESS_LUT_OETF \n");
						}else{
							fwif_color_set_DM2_EOTF_TV006(HFC_EOTF_LUT_R[1], 1);
							pre_sourceMaxL_a = 0;
							printk("[Other]VPQ_EXTERN_IOC_SET_BRIGHTNESS_LUT_OETF \n");
						}
						pre_hdrtype = VIP_system_info_structure_table->HDR_flag;
					}
					#endif
				}
			}else if(BRIGHTNESS_LUT_T.un16_length==1024){
					//printk("SDR GammaCurve\n");
					Scaler_SetOETF2Gamma(&BRIGHTNESS_LUT_T);
					SmartHDRBlendingFlag = 1;
					fwif_set_ddomainISR_gamma(0,0,0,0);//fwif_color_set_gamma_Magic();
					//drvif_TV006_SET_HDR_mode_off();
                          		//drvif_DM2_OETF_Enable(0, 0);
					//drvif_DM2_EOTF_Enable(0, 0);
					//drvif_color_set_BBC_Enable(0);
					BBC_Enable=0;
				}
			}
			ret = 0;
		break;
	}

	case VPQ_EXTERN_IOC_GET_LC_HISTOGRAM_TV030:
	{
		unsigned int  plc_out[16];
		Scaler_color_get_LC_His_dat(&plc_out[0]);
		if (copy_to_user((void __user *)arg, (void *)&plc_out[0], sizeof(plc_out) )) {
			rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_LC_HISTOGRAM_TV030 fail\n");
			ret = -1;
		} else
			ret = 0;

	}
	break;

	case VPQ_EXTERN_IOC_SET_RGBWTOSRGBMATRIX_TV030:
	{
		unsigned char i,j;
		extern short BT709Target_sRGB_APPLY[3][3];
		extern short BT2020Target_sRGB_APPLY[3][3];
		extern short DCIP3Target_sRGB_APPLY[3][3];
		extern short ADOBERGBTarget_sRGB_APPLY[3][3];
		extern short BT601_525Target_sRGB_APPLY[3][3];
		extern short BT601_625Target_sRGB_APPLY[3][3];

		RTK_VPQ_sRGB_Matrix sRGB_T;
		extern unsigned char g_srgbForceUpdate;

		rtd_printk(KERN_EMERG, TAG_NAME, "[Kernel]VPQ_EXTERN_IOC_SET_RGBWTOSRGBMATRIX\n");

		if (copy_from_user(&sRGB_T,  (int __user *)arg,sizeof(RTK_VPQ_sRGB_Matrix))) {
			ret = -1;
		} else {
		#if 0//for debug
			for(i=0;i<3;i++){
				 for(j=0;j<3;j++)
					printk("BT709Target[%d][%d]=%d\n",i,j,sRGB_T.BT709Target[i][j]);
				}
			for(i=0;i<3;i++){
				 for(j=0;j<3;j++)
					printk("sRGB_T->BT2020Target[%d][%d]=%d\n",i,j,sRGB_T.BT2020Target[i][j]);
				}
			for(i=0;i<3;i++){
				 for(j=0;j<3;j++)
					printk("sRGB_T->DCIP3Target[%d][%d]=%d\n",i,j,sRGB_T.DCIP3Target[i][j]);
				}
			for(i=0;i<3;i++){
				 for(j=0;j<3;j++)
					printk("sRGB_T->ADOBERGBTarget[%d][%d]=%d\n",i,j,sRGB_T.ADOBERGBTarget[i][j]);
				}
		#endif
			//-----------------------------------------------------------------------------
			for(i=0;i<3;i++){
				 for(j=0;j<3;j++)
					BT709Target_sRGB_APPLY[i][j]=sRGB_T.BT709Target[i][j];
				}
			for(i=0;i<3;i++){
				 for(j=0;j<3;j++)
					BT2020Target_sRGB_APPLY[i][j]=sRGB_T.BT2020Target[i][j];
				}
			for(i=0;i<3;i++){
				 for(j=0;j<3;j++)
					DCIP3Target_sRGB_APPLY[i][j]=sRGB_T.DCIP3Target[i][j];
				}
			for(i=0;i<3;i++){
				 for(j=0;j<3;j++)
					ADOBERGBTarget_sRGB_APPLY[i][j]=sRGB_T.ADOBERGBTarget[i][j];
				}
			for(i=0;i<3;i++){
				 for(j=0;j<3;j++)
					BT601_525Target_sRGB_APPLY[i][j]=sRGB_T.BT601_525Target[i][j];
				}
			for(i=0;i<3;i++){
				 for(j=0;j<3;j++)
					BT601_625Target_sRGB_APPLY[i][j]=sRGB_T.BT601_625Target[i][j];
				}
			//fwif_color_set_sRGBMatrix();
		 	g_srgbForceUpdate = 0;

			ret = 0;
		}
	break;
	}

	case VPQ_EXTERN_IOC_SET_PARTTENFORAUTOGAMMA_TV030:
	{
		//rtd_printk(KERN_EMERG, TAG_NAME,"VPQ_EXTERN_IOC_SET_PARTTENFORAUTOGAMMA_TV030\n");
		INTERNAL_PARTTEN pData;

		if (copy_from_user(&pData,	(void __user *)arg,sizeof(INTERNAL_PARTTEN))) {
			rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_PARTTENFORAUTOGAMMA_TV030 fail\n");
			ret = -1;
		} else {
			Scaler_Set_Partten4AutoGamma(pData.enable,pData.r_Val,pData.g_Val,pData.b_Val);
			ret = 0;
		}
	}
	break;

	case VPQ_EXTERN_IOC_SET_PICTUREMODE_TV030:
	{
		RTK_VPQ_MODE_TYPE_TV030 args = 0;
		//printk("VPQ_EXTERN_IOC_SET_PICTUREMODE_TV030\n");
		if (copy_from_user(&args,  (int __user *)arg,sizeof(RTK_VPQ_MODE_TYPE_TV030))) {
			ret = -1;
		} else {
			ret = 0;
			Scaler_Set_PictureMode_PQsetting((RTK_VPQ_MODE_TYPE_TV030)args);
		}
	}
		break;

	case VPQ_EXTERN_IOC_SET_LOCALCONTRAST_LEVEL_TV030:
	{
		unsigned char level = 0;
		if (copy_from_user(&level,  (int __user *)arg,sizeof(unsigned char))) {
			ret = -1;
		} else {
			printk("VPQ_EXTERN_IOC_SET_LOCALCONTRAST_LEVEL_TV030 level=%d \n",level);
			Scaler_SetLocalContrastTable(level);//table 0 for low; table 1 for high
			LocalContrast_table = level;
			ret = 0;
		}
	}
	break;

#if 1//---------add for ColorSpaece ver 2.0--------------
	case VPQ_EXTERN_IOC_SET_3DLUT_DATA:
	{
		static RTK_VPQ_3DLUT_PARAM lut_param;
		extern unsigned int D_3DLUT_999_Data[VIP_D_3DLUT_NUM][VIP_D_3D_LUT_SIZE][VIP_8VERTEX_RGB_MAX];
		extern int D_3DLUT_Reseult[VIP_D_3D_LUT_SIZE][VIP_8VERTEX_RGB_MAX];

		//int i=0;
		if(copy_from_user(&lut_param, (int __user *)arg, sizeof(RTK_VPQ_3DLUT_PARAM))){
			rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_3DLUT_DATA fail\n");
			ret =-1;
			break;
		}else{
				memcpy(D_3DLUT_999_Data[0],lut_param.D_3DLUT_Data[0],VIP_D_3D_LUT_SIZE * VIP_8VERTEX_RGB_MAX *sizeof(UINT32));
				//memcpy(D_3DLUT_999_Data[1],lut_param.D_3DLUT_Data[1],VIP_D_3D_LUT_SIZE * VIP_8VERTEX_RGB_MAX *sizeof(UINT32));
				//memcpy(D_3DLUT_999_Data[2],lut_param.D_3DLUT_Data[2],VIP_D_3D_LUT_SIZE * VIP_8VERTEX_RGB_MAX *sizeof(UINT32));
				//memcpy(D_3DLUT_Reseult, D_3DLUT_999_Data[0], VIP_D_3D_LUT_SIZE * VIP_8VERTEX_RGB_MAX * sizeof(UINT32));
				printk("VPQ_EXTERN_IOC_SET_3DLUT_DATA\n");
				/*for(i=728;i<740;i++){
					printk("default,3dlut[%d]=%d,%d,%d\n",i,lut_param.D_3DLUT_Data[0][i][0],lut_param.D_3DLUT_Data[0][i][1],lut_param.D_3DLUT_Data[0][i][2]);
				}
				for(i=728;i<740;i++){
					printk("custom,3dlut[%d]=%d,%d,%d\n",i,lut_param.D_3DLUT_Data[1][i][0],lut_param.D_3DLUT_Data[1][i][1],lut_param.D_3DLUT_Data[1][i][2]);
				}
				for(i=728;i<740;i++){
					printk("extend,3dlut[%d]=%d,%d,%d\n",i,lut_param.D_3DLUT_Data[2][i][0],lut_param.D_3DLUT_Data[2][i][1],lut_param.D_3DLUT_Data[2][i][2]);
				}*/
				ret = 0;
		}
	}
	break;

	case VPQ_EXTERN_IOC_SET_3DLUT_ENABLE:
	{
		D_3DLut_PARTTEN pData;

		if (copy_from_user(&pData,	(void __user *)arg,sizeof(D_3DLut_PARTTEN))) {
			rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_3DLUT_ENABLE fail\n");
			ret = -1;
		} else {
			dlut_UI_en=pData.enable;
            dlut_index=pData.index; //0:default ; 1:custom ; 2:EXTEND

            if(dlut_index==LUT_MAX)
                fwif_color_set_linear_3dLUT();
            else
                fwif_color_cpy_D_3DLUT_TBL();

			write_3dlut_base_table_time = 0;
			//down(&ColorSpace_Semaphore);
            //fwif_color_WaitFor_DEN_STOP_UZUDTG();
			//fwif_color_D_LUT_By_DMA(0, 255, (unsigned int *)D_3DLUT_999_Reseult);
			//up(&ColorSpace_Semaphore);
			//drvif_color_D_3dLUT_Enable(dlut_UI_en);
            printk("[3dlut]dlut_UI_en=%d,dlut_index=%d\n",dlut_UI_en,dlut_index);
			printk("[3dlut] tbl[0~3]=%d,%d,%d, \n",D_3DLUT_999_Reseult[0][0],D_3DLUT_999_Reseult[0][1],D_3DLUT_999_Reseult[0][2]);
			ret = 0;
		}
	}
	break;

	case VPQ_EXTERN_IOC_SET_BaseColor:
	{
		RTK_VPQ_COLORSPACE_INDEX ColoSpace_T;
		extern RTK_VPQ_COLORSPACE_INDEX ColoSpace_Index;
		if (copy_from_user(&ColoSpace_T,  (int __user *)arg,sizeof(RTK_VPQ_COLORSPACE_INDEX))) {
			ret = -1;
		} else {
			printk("VPQ_EXTERN_IOC_SET_BaseColor \n");
			dlut_index=3; //linear
			fwif_color_cpy_D_3DLUT_TBL();
			//write_3dlut_base_table_time = 0xff;
			//fwif_color_WaitFor_DEN_STOP_UZUDTG();
			//fwif_color_D_LUT_By_DMA(0, 255, (unsigned int *)D_3DLUT_999_Reseult);

			write_3dlut_base_table_time = 0xff;
			Scaler_Set_ColorSpace_D_3dlutTBL(&ColoSpace_T);
			memcpy(&ColoSpace_Index,&ColoSpace_T,sizeof(RTK_VPQ_COLORSPACE_INDEX));
			drvif_color_D_3dLUT_Enable(1);
			ret = 0;
		}
	}
	break;

	case VPQ_EXTERN_IOC_GET_COLOR_MATRIX_TYPE:
	{
		COLOR_MATRIX_TYPE type = fwif_color_get_colorspaceInfo();
		//printk("VPQ_EXTERN_IOC_GET_COLOR_MATRIX_TYPE\n");
		if (copy_to_user((void __user *)arg, (void *)&type, sizeof(COLOR_MATRIX_TYPE))) {
			ret = -1;
		}
		else
		{
			ret = 0;
		}
	}
	break;

	case VPQ_EXTERN_IOC_SET_COLOR_MATRIX_ENABLE:
	{
		ColorMatrix_PARTTEN pData;
		extern unsigned char g_srgbForceUpdate;

		if (copy_from_user(&pData,	(void __user *)arg,sizeof(ColorMatrix_PARTTEN))) {
			rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_COLOR_MATRIX_ENABLE fail\n");
			ret = -1;
		} else {
			printk(" [sRGB]enable=%d,index=%d\n",pData.enable,pData.index);
			g_srgbForceUpdate = 1;
			fwif_color_set_ColorMatrix(pData.enable,pData.index);
			if(pData.enable==1)
				GammaBlendingFlag=1;
			else
				GammaBlendingFlag=0;
			fwif_set_ddomainISR_gamma(0,0,0,0);
			ret = 0;
		}
	}
	break;
	#endif//---------add for ColorSpaece ver 2.0--------------

	case VPQ_EXTERN_IOC_SET_DECONTOURLEVEL:
		{
			#ifndef  CONFIG_RTK_8KCODEC_INTERFACE
			//extern unsigned char Decontoure_level_by_OSD;
			#endif
			extern unsigned char i_decontour_index;
			extern unsigned char i_decontour_level;
			unsigned char value = 0;

			if (copy_from_user(&value,  (int __user *)arg,sizeof(unsigned char))) {
				ret = -1;
			} else {
				printk("VPQ_EXTERN_IOC_SET_DECONTOURLEVEL:UI= %d,i=%d\n",value,i_decontour_index);
				i_decontour_level=value;
					if(value==0){
						Scaler_Set_I_De_Contour(0);
						Scaler_Set_Block_DeContour(0);
					}else if(value==1){
						Scaler_Set_I_De_Contour(i_decontour_index);
						Scaler_Set_Block_DeContour(0);
					}else if(value==3){
						Scaler_Set_I_De_Contour(i_decontour_index);
						Scaler_Set_Block_DeContour(i_decontour_index);
					}
				ret = 0;
				}
		break;
		}
		case VPQ_EXTERN_IOC_SET_PanelMaxLuminance_TV030:
		{
			extern unsigned short panel_max_luminance;
			rtd_printk(KERN_ERR, TAG_NAME,"VPQ_EXTERN_IOC_SET_PanelMaxLuminance_TV030 \n");
			if (copy_from_user(&panel_max_luminance,  (int __user *)arg,sizeof(unsigned short))) {
				ret = -1;
			} else {
				rtd_printk(KERN_ERR, TAG_NAME,"panel_max_luminance : %d \n", panel_max_luminance);
				}
		}
		break;

		case VPQ_EXTERN_IOC_GET_PIXELINFO_TV030:
		{
			//printk("Ioc VPQ_EXTERN_IOC_GET_PIXELINFO_TV030!!!!!!!!\n");
			SCREEN_PIXEL_INFO pixelInfo;
			if (copy_from_user(&pixelInfo, (int __user *)arg, sizeof(SCREEN_PIXEL_INFO))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_GET_PIXELINFO_TV030 fail\n");
				ret = -1;
			} else {
				ret = 0;
				Scaler_Get_ScreenPixel_info(&pixelInfo);
				if (copy_to_user((void __user *)arg, (void *)&pixelInfo, sizeof(SCREEN_PIXEL_INFO))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_PIXELINFO_TV030 scaler_get_screen_pixel_info fail\n");
					ret = -1;
				} else
					ret = 0;
			}
		}
        break;
	case VPQ_EXTERN_IOC_SET_BLUE_STRETCH:
	{
		unsigned char TableIdx = 0;
		if (copy_from_user(&TableIdx,(int __user *)arg,sizeof(unsigned char))) {
			printk("VPQ_EXTERN_IOC_SET_BLUE_STRETCH\n");
			ret = -1;
		} else {
			ret = 0;
			printk("[blue stretch]TableIdx=%d\n",TableIdx);
			uvoffset_Level_UI = (VIP_YUV2RGB_LEVEL_SELECT) TableIdx;
			fwif_color_set_UV_Offset_tv043((VIP_YUV2RGB_LEVEL_SELECT) TableIdx);
		}
	}
	break;

	case VPQ_EXTERN_IOC_GET_CURRENT_COLOR_MATRIX:
	{
		extern short gColorGamutMatrix_Apply[3][3];
		unsigned char i,j;
		if (copy_to_user((void __user *)arg, (void *)&gColorGamutMatrix_Apply, sizeof(gColorGamutMatrix_Apply))) {
			rtd_printk(KERN_ERR, TAG_NAME, "VPQ_EXTERN_IOC_GET_CURRENT_COLOR_MATRIX\n");
			ret = -1;
		} else {
			ret = 0;

			for(i=0;i<3;i++){
				 for(j=0;j<3;j++)
					rtd_printk(KERN_EMERG, TAG_NAME, "curmatrix[%d][%d]=%d \n",i,j,gColorGamutMatrix_Apply[i][j]);
				}
		}
	}
	break;

    case VPQ_EXTERN_IOC_SET_DEMURA_ENABLE:
    {
        unsigned char demura_en;

        if(copy_from_user(&demura_en,(int __user *)arg,sizeof(unsigned char))) {
            rtd_printk(KERN_ERR, TAG_NAME, "VPQ_EXTERN_IOC_SET_DEMURA_ENABLE\n");
            ret = -1;
        } else {
            ret = 0;
            if(demura_en){
                fwif_color_set_DeMura_En(0);
                fwif_color_DeMura_init(1,0);
            }
            fwif_color_set_DeMura_En(demura_en);
            rtd_printk(KERN_EMERG, TAG_NAME, "demura_en=%d \n",demura_en);
        }
    }
    break;

    case VPQ_EXTERN_IOC_SET_DEMURA_CURVE:
    {
        if (copy_from_user((void *)&DeMura_TBL, (int __user *)arg, sizeof(VIP_DeMura_TBL))) {
            rtd_pr_vpq_err("kernel copy VPQ_EXTERN_IOC_SET_DEMURA_CURVE fail\n");
            ret = -1;
        } else {
            rtd_pr_vpq_err("kernel copy DeMura_TBL done\n");
            if(fwif_color_get_DeMura_En() == 0){
                fwif_color_DeMura_init(1,0);
                Scaler_Set_DeMura_En(1);
                rtd_pr_vpq_err("kernel demura init done!\n");
            }
            ret = 0;
        }
    }
    break;

	case VPQ_EXTERN_IOC_SET_DLG_OD_TABLE:
	{
	    if (copy_from_user((void *)&dlg_od_table, (int __user *)arg, sizeof(DLG_OD_TABLE_TYPE))) {
	        rtd_pr_vpq_err("kernel copy VPQ_EXTERN_IOC_SET_DLG_OD_TABLE fail\n");
	        ret = -1;
	    } else {
	        rtd_pr_vpq_err("kernel copy VPQ_EXTERN_IOC_SET_DLG_OD_TABLE done\n");
	        ret = 0;
	    }
	}
	break;

	case VPQ_EXTERN_IOC_SET_DLG_ACC_TABLE:
	{
	    if (copy_from_user((void *)&dlg_acc_table, (int __user *)arg, sizeof(DLG_ACC_TABLE_TYPE))) {
	        rtd_pr_vpq_err("kernel copy VPQ_EXTERN_IOC_SET_DLG_ACC_TABLE fail\n");
	        ret = -1;
	    } else {
	        rtd_pr_vpq_err("kernel copy VPQ_EXTERN_IOC_SET_DLG_ACC_TABLE done\n");
	        ret = 0;
	    }
	}
	break;

	case VPQ_EXTERN_IOC_SET_VRR_OD_TABLE:
	{
	    if (copy_from_user((void *)&v_od_table, (int __user *)arg, sizeof(V_R_R_OD_TABLE_TYPE))) {
	        rtd_pr_vpq_err("kernel copy VPQ_EXTERN_IOC_SET_VRR_OD_TABLE fail\n");
	        ret = -1;
	    } else {
	        rtd_pr_vpq_err("kernel copy VPQ_EXTERN_IOC_SET_VRR_OD_TABLE done\n");

	        ret = 0;
	    }
	}
	break;

	case VPQ_EXTERN_IOC_SET_VRR_ACC_TABLE:
	{
	    if (copy_from_user((void *)&v_acc_table, (int __user *)arg, sizeof(V_R_R_ACC_TABLE_TYPE))) {
	        rtd_pr_vpq_err("kernel copy VPQ_EXTERN_IOC_SET_VRR_ACC_TABLE fail\n");
	        ret = -1;
	    } else {
	        rtd_pr_vpq_err("kernel copy VPQ_EXTERN_IOC_SET_VRR_ACC_TABLE done\n");

	        ret = 0;
	    }
	}
	break;

#ifdef CONFIG_DOLBY_BACKLIGHT_CTRL_ADIM_PDIM
    case VPQ_EXTERN_IOC_SET_DOLBYBACKLIGHT_TV030:
    {
        KADP_VPQ_DOBYBAKLIGHT_INI glob_dolbybacklight_ini;
        rtd_printk(KERN_INFO, TAG_NAME, "VPQ_EXTERN_IOC_SET_DOLBYBACKLIGHT_TV030 \n");
        if(copy_from_user(&glob_dolbybacklight_ini,  (int __user *)arg,sizeof(KADP_VPQ_DOBYBAKLIGHT_INI)))
        {
            rtd_pr_vpq_err("kernel VPQ_EXTERN_IOC_SET_DOLBYBACKLIGHT_TV030 fail\n");
            ret = -1;
        }
        else
        {   rtd_pr_vpq_info("kernel VPQ_EXTERN_IOC_SET_DOLBYBACKLIGHT_TV030\n");
            Scaler_color_vpq_set_dolbylight_init(glob_dolbybacklight_ini);
        ret = 0;
        }
    }
    break;
#endif
    case VPQ_EXTERN_IOC_PWM_TURN_ON_DOLBYBACKLIGHT_TV030:
    {
        unsigned int args = 0;
        rtd_printk(KERN_ERR, TAG_NAME,"VPQ_EXTERN_IOC_PWM_TURN_ON_DOLBYBACKLIGHT_TV030 \n");
        if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {
        rtd_pr_vpq_err("VPQ_EXTERN_IOC_PWM_TURN_ON_DOLBYBACKLIGHT_TV030 fail\n");
            ret = -1;
        } else {
            is_pwm_ctrl_by_dolby = args;
            rtd_pr_vpq_info("VPQ_EXTERN_IOC_PWM_TURN_ON_DOLBYBACKLIGHT_TV030:%d\n", is_pwm_ctrl_by_dolby);
            ret = 0;
        }
    }
    break;
    #if 1
	case VPQ_EXTERN_IOC_SET_AIPQTABLE:
	{
		static SLR_VIP_TABLE_AIPQ ptr_AIPQ_bin;
		//extern DRV_AI_Tune_ICM_table AI_Tune_ICM_TBL[10];
		//extern AI_DCC_Ctrl_table_TV030 AI_DCC_TBL_TV030[PQ_SCENE_TYPE_NUM_TV030];
		extern ICM_RGB_Offset_table tICM_RGB_Offset[VIP_ICM_TBL_X];
		extern ICM_RGB_Offset_table tICM_RGB_Offset_AIPQ[PQ_SCENE_TYPE_NUM_TV030];
		extern iEdgeSmooth_Offset_table iEdgeSmooth_Offset[PQ_SCENE_TYPE_NUM_TV030];
		extern RTNR_Offset_table RTNR_Offset[PQ_SCENE_TYPE_NUM_TV030];
		extern SNR_Offset_table SNR_Offset[PQ_SCENE_TYPE_NUM_TV030];
		extern MNR_Offset_table MNR_Offset[PQ_SCENE_TYPE_NUM_TV030];
		extern I_DCTI_Offset_table I_DCTI_Offset[PQ_SCENE_TYPE_NUM_TV030];
		extern Directional_SU_Offset_table Directional_SU_Offset[PQ_SCENE_TYPE_NUM_TV030];
		extern DLTI_Offset_table DLTI_Offset[PQ_SCENE_TYPE_NUM_TV030];
		extern D_DCTI_Offset_table D_DCTI_Offset[PQ_SCENE_TYPE_NUM_TV030];
		extern VIP_CDS_Table CDS_TBL_forAI[PQ_SCENE_TYPE_NUM_TV030];
		extern UVOffsetbyY_Offset_table UVOffsetbyY_Offset[PQ_SCENE_TYPE_NUM_TV030] ;
		extern LC_Offset_table LC_Offset[PQ_SCENE_TYPE_NUM_TV030];
		extern unsigned int LC_ToneMappingSlopePoint_Table_forAI[PQ_SCENE_TYPE_NUM_TV030][LC_Curve_ToneM_PointSlope];
		//unsigned int i=0;

		printk("VPQ_EXTERN_IOC_SET_AIPQTABLE\n");
		if (copy_from_user(&ptr_AIPQ_bin,  (int __user *)arg,sizeof(SLR_VIP_TABLE_AIPQ))) {
			printk("VPQ_EXTERN_IOC_SET_AIPQTABLE error !!!!\n");
			ret = -1;
		} else {

			//memcpy(AI_Tune_ICM_TBL, ptr_AIPQ_bin.AI_Tune_ICM_TBL, PQ_SCENE_TYPE_NUM_TV030*sizeof(DRV_AI_Tune_ICM_table));
			//memcpy(AI_DCC_TBL_TV030, ptr_AIPQ_bin.AI_DCC_TBL, PQ_SCENE_TYPE_NUM_TV030*sizeof(AI_DCC_Ctrl_table_TV030));
			memcpy(tICM_RGB_Offset_AIPQ, ptr_AIPQ_bin.tICM_RGB_Offset, PQ_SCENE_TYPE_NUM_TV030*sizeof(ICM_RGB_Offset_table));				
			memcpy(tICM_RGB_Offset, ptr_AIPQ_bin.tICM_RGB_Offset, PQ_SCENE_TYPE_NUM_TV030*sizeof(ICM_RGB_Offset_table));
			memcpy(iEdgeSmooth_Offset, ptr_AIPQ_bin.iEdgeSmooth_Offset, PQ_SCENE_TYPE_NUM_TV030*sizeof(iEdgeSmooth_Offset_table));
			memcpy(RTNR_Offset, ptr_AIPQ_bin.RTNR_Offset, PQ_SCENE_TYPE_NUM_TV030*sizeof(RTNR_Offset_table));
			memcpy(SNR_Offset, ptr_AIPQ_bin.SNR_Offset, PQ_SCENE_TYPE_NUM_TV030*sizeof(SNR_Offset_table));
			memcpy(MNR_Offset, ptr_AIPQ_bin.MNR_Offset, PQ_SCENE_TYPE_NUM_TV030*sizeof(MNR_Offset_table));
			memcpy(I_DCTI_Offset, ptr_AIPQ_bin.I_DCTI_Offset, PQ_SCENE_TYPE_NUM_TV030*sizeof(I_DCTI_Offset_table));
			memcpy(Directional_SU_Offset, ptr_AIPQ_bin.Directional_SU_Offset, PQ_SCENE_TYPE_NUM_TV030*sizeof(Directional_SU_Offset_table));
			memcpy(DLTI_Offset, ptr_AIPQ_bin.DLTI_Offset, PQ_SCENE_TYPE_NUM_TV030*sizeof(DLTI_Offset_table));
			memcpy(D_DCTI_Offset, ptr_AIPQ_bin.D_DCTI_Offset, PQ_SCENE_TYPE_NUM_TV030*sizeof(D_DCTI_Offset_table));
			memcpy(CDS_TBL_forAI, ptr_AIPQ_bin.CDS_TBL_forAI, PQ_SCENE_TYPE_NUM_TV030*sizeof(VIP_CDS_Table));
			memcpy(UVOffsetbyY_Offset, ptr_AIPQ_bin.UVOffsetbyY_Offset, PQ_SCENE_TYPE_NUM_TV030*sizeof(UVOffsetbyY_Offset_table));
			memcpy(LC_Offset, ptr_AIPQ_bin.LC_Offset, PQ_SCENE_TYPE_NUM_TV030*sizeof(LC_Offset_table));
			memcpy(LC_ToneMappingSlopePoint_Table_forAI, ptr_AIPQ_bin.LC_ToneMappingSlopePoint_Table_forAI, PQ_SCENE_TYPE_NUM_TV030*LC_Curve_ToneM_PointSlope*sizeof(unsigned int));

			#if 0
			printk("[aipq]%d,%d,%d,%d,%d,%d,%d \n",SNR_Offset[13].ai_SNR_Curve_Mapping_w1_Offset_TBL.w_0,
			SNR_Offset[13].ai_SNR_Curve_Mapping_w1_Offset_TBL.w_1,
			SNR_Offset[13].ai_SNR_Curve_Mapping_w1_Offset_TBL.w_2,
			SNR_Offset[13].ai_SNR_Curve_Mapping_w1_Offset_TBL.w_3,
			SNR_Offset[13].ai_SNR_Curve_Mapping_w1_Offset_TBL.w_4,
			SNR_Offset[13].ai_SNR_Curve_Mapping_w1_Offset_TBL.w_5,
			SNR_Offset[13].ai_SNR_Curve_Mapping_w1_Offset_TBL.w_6
			);
			printk("[aipq] rgboffset en=%d \n",tICM_RGB_Offset_AIPQ[0].RGB_offset_en);
			#endif
			printk("VPQ_EXTERN_IOC_SET_AIPQTABLE cpy done\n");

			ret = 0;
		}
	}
	break;

	case VPQ_EXTERN_IOC_SET_AIPQ_SCENE_TYPE:
	{
		unsigned int ai_scene_type;

	    if (copy_from_user((void *)&ai_scene_type, (int __user *)arg, sizeof(unsigned int))) {
	        printk("kernel copy VPQ_EXTERN_IOC_SET_AIPQ_SCENE_TYPE fail\n");
	        ret = -1;
	    } else {
	        scalerAI_pq_scene_Type_Set((PQ_SCENE_TYPE_TV030)ai_scene_type);
	        ret = 0;
	    }
	}
	break;
	#endif

	default:
		//rtd_printk(KERN_DEBUG, TAG_NAME, "kernel IO command %d not found!\n", cmd);
		//printk("VPQ_EXTERN_IOC cmd=0x%x unknown\n", cmd);
		return -1;

	}
	return ret;
}
extern int PictureMode_flg;
//extern BOOL GetStillFrameFlag(void);
void PictureModeChg_ByHSBC(bool HSBC_Chg)
{
	//if(HSBC_Chg && GetStillFrameFlag()==true) {
	if(HSBC_Chg && Scaler_PQLContext_GetStillFrameFlag()==true) {
		PictureMode_flg = 1;
	}

}

long vpqex_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
	int ret = 0, i, j;
	extern SLR_VIP_TABLE* m_pVipTable;
	ENUM_TVD_INPUT_FORMAT srcfmt = 0;
	unsigned int vipsource = 0;
	unsigned int brightness = 0;
	unsigned int contrast = 0;
	unsigned int saturation = 0;
	unsigned int hue = 0;
	unsigned int nr = 0;
	unsigned int sharpness = 0;
	TV001_LEVEL_E dbLevel = 0;
	TV001_COLORTEMP_E colorTemp = 0;
	TV001_COLOR_TEMP_DATA_S color_temp_data_s;
	unsigned int backlight = 0;
	RTK_PQModule_T PQModule_T;
	RTK_DEMO_MODE_T DEMO_MODE_T;
	unsigned char onoff = 0;
	TV001_HDR_TYPE_E HdrType = 0;
	unsigned char bEnable = 0;
	unsigned int dcc_mode = 0;
	unsigned int icm_table_idx = 0;
	unsigned int film_mode_enable = 0;
	HDMI_CSC_DATA_RANGE_MODE_T range_mode = 0;
	unsigned int hdr_force_mode = 0;
	unsigned int cinema_pic_mode = 0;
	unsigned int pq_bypass_lv = 0;
	HAL_VPQ_ENG_STRUCT* ENGMENU = NULL;
	HAL_VPQ_ENG_SIZE* ENGMENU_size = NULL;
	HAL_VPQ_ENG_ID ENG_ID;
	unsigned int iEn;
	int level;
	RTK_TV002_SR_T sr_data;
	unsigned int live_colour = 0;
	unsigned int mastered_4k = 0;
	unsigned char TV002_PROJECT_STYLE;
	unsigned char TV002_DBC_POWER_SAVING_MODE;
	unsigned char TV002_DBC_UI_BACKLIGHT;
	unsigned char TV002_DBC_BACKLIGHT;
	unsigned char TV002_UI_PICTURE_MODE;
	unsigned int GammaType = 0;
	unsigned int args = 0;
	HAL_VPQ_ENG_TWOLAYER_STRUCT* ENG_TWOLAYER_MENU = NULL;
	HAL_VPQ_ENG_TWOLAYER_ID ENG_TWOLAYER_ID;
	unsigned int dcr_mode = 0;
	unsigned int DCR_backlight = 0;
	HAL_VPQ_DATA_EXTERN_T HAL_VPQ_DATA_EXTERN;
	unsigned char *pBuff_tmp;
	bool HSBC_Chg = false;
	#ifdef ENABLE_DE_CONTOUR_I2R
	extern unsigned char Eng_Skyworth_Decont_Pattern;
	#endif

	if (vpq_extern_ioctl_get_stop_run(cmd))
		return 0;

	if (_IOC_TYPE(cmd) == VPQ_EXTERN_IOC_MAGIC_TV030)
			return vpqex_ioctl_tv030(file, cmd, arg);

//	if (vpqex_project_id != 0x00060000) {
//		if (VPQ_EXTERN_IOC_INIT == cmd) {
//			vpqex_project_id = 0x00060000;
//			vpqex_boot_init();
//		} else {
//			return -1;
//		}
//	}

//	printk("vpqex_ioctl %d\n", __LINE__);

	if (VPQ_EXTERN_IOC_INIT == cmd) {
		vpqex_project_id = 0x00010000;
		Scaler_color_Set_HDR_AutoRun(TRUE);
		//#ifndef CONFIG_MEMC_BYPASS
			if(get_MEMC_bypass_status_refer_platform_model() == FALSE){
                //for video path boot logo
                Scaler_MEMC_output_force_bg_enable(TRUE,__func__,__LINE__);
			}
		//#endif
		#ifdef CONFIG_CUSTOMER_TV002
		if (!g_bLDinited) {
			Scaler_fwif_color_set_LocalDimming_table(0, 1);
			g_bLDinited = 1;
		}
		#endif
		return 0;
	} else if (VPQ_EXTERN_IOC_UNINIT == cmd) {
		PQ_Dev_Extern_Status = PQ_DEV_EXTERN_UNINIT;
		return 0;
	} else if (VPQ_EXTERN_IOC_OPEN == cmd || VPQ_EXTERN_IOC_CLOSE == cmd) {
		return 0;
	}else if (VPQ_EXTERN_IOC_INIT_PRJ_ID == cmd) {
		VIP_Customer_Project_ID_ENUM prj_id;
		if (copy_from_user(&prj_id, (int __user *)arg, sizeof(unsigned int))) {
			rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_INIT_PRJ_ID fail\n");
			return -1;
		} else {
			fwif_VIP_set_Project_ID(prj_id);
			rtd_printk(KERN_ERR, TAG_NAME, "VPQ_EXTERN_IOC_INIT_PRJ_ID = %d\n", prj_id);
			return 0;
		}
	}

//	if (PQ_Dev_Extern_Status != PQ_DEV_EXTERN_INIT_DONE)
//		return -1;

//	printk("vpqex_ioctl %d\n", __LINE__);
#ifdef Merlin3_rock_mark	//Merlin3 rock mark
	if (fwif_color_get_pq_demo_flag_rpc())
		return 0;
#endif
#ifndef SEND_VIP_MIDDLE_WARE_PIC_MODE_DATA
	if (vpqex_skip_middle_ware_picmode_data(cmd))
		return 0;
#endif

	switch (cmd) {
	/*
	case VPQ_EXTERN_IOC_INIT:
		if (PQ_DEV_EXTERN_UNINIT == PQ_Dev_Extern_Status || PQ_DEV_EXTERN_NOTHING == PQ_Dev_Extern_Status) {
			fwif_color_inv_gamma_control_back(SLR_MAIN_DISPLAY, 1);
			fwif_color_gamma_control_back(SLR_MAIN_DISPLAY, 1);
			PQ_Dev_Extern_Status = PQ_DEV_EXTERN_INIT_DONE;
		} else
			return -1;
		break;

	case VPQ_EXTERN_IOC_UNINIT:
		if (PQ_DEV_EXTERN_CLOSE == PQ_Dev_Extern_Status || PQ_DEV_EXTERN_INIT_DONE == PQ_Dev_Extern_Status) {
			PQ_Dev_Extern_Status = PQ_DEV_EXTERN_UNINIT;
		} else
			return -1;
		break;

	case VPQ_EXTERN_IOC_OPEN:
		if (PQ_DEV_EXTERN_CLOSE == PQ_Dev_Extern_Status || PQ_DEV_EXTERN_INIT_DONE == PQ_Dev_Extern_Status) {
			PQ_Dev_Extern_Status = PQ_DEV_EXTERN_OPEN_DONE;
		} else
			return -1;
		break;

	case VPQ_EXTERN_IOC_CLOSE:
		if (PQ_DEV_EXTERN_OPEN_DONE == PQ_Dev_Extern_Status) {
			PQ_Dev_Extern_Status = PQ_DEV_EXTERN_CLOSE;
		} else
			 return -1;
		break;
	*/

/************************************************************************
 *  TV001 External variables
 ************************************************************************/
 		case VPQ_EXTERN_IOC_REGISTER_NEW_VIPTABLE:
		{
			ret = xRegisterTable();
			if (ret) {
				ret = 0;
				printk("viptable xRegisterTable success\n");
			}
			else {
				ret = -1;
				printk("viptable xRegisterTable fail\n");
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_VGA_ADCGainOffset:
		{
			extern ADCGainOffset m_vgaGainOffset;
			if (copy_from_user(&m_vgaGainOffset, (int __user *)arg, sizeof(ADCGainOffset))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_VGA_ADCGainOffset fail\n");
				ret = -1;
				break;
			} else {
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_VGA_ADCGainOffset success\n");
				ret = 0;
				#if 1
				printk("m_vgaGainOffset={\n");
				printk("%x %x %x ", m_vgaGainOffset.Gain_R, m_vgaGainOffset.Gain_G, m_vgaGainOffset.Gain_B);
				printk("} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_YPBPR_ADCGainOffset:
		{
			extern ADCGainOffset m_yPbPrGainOffsetData[SR_MAX_YPBPR_GAINOFFSET_MODE] ;
			if (copy_from_user(&m_yPbPrGainOffsetData, (int __user *)arg, sizeof(m_yPbPrGainOffsetData))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_YPBPR_ADCGainOffset fail\n");
				ret = -1;
				break;
			} else {
#ifdef CONFIG_ARM64 //ARM32 compatible
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_YPBPR_ADCGainOffset success, sizeof(m_yPbPrGainOffsetData)=%zu\n", sizeof(m_yPbPrGainOffsetData));
#else
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_YPBPR_ADCGainOffset success, sizeof(m_yPbPrGainOffsetData)=%d\n", sizeof(m_yPbPrGainOffsetData));
#endif
				ret = 0;
				#if 1
				printk("m_yPbPrGainOffsetData[16]={\n");
				for (i=0; i<SR_MAX_YPBPR_GAINOFFSET_MODE; i++) {
					//for (j=0; j<VIP_QUALITY_FUNCTION_TOTAL_CHECK; j++) {
						printk("%x %x %x ", m_yPbPrGainOffsetData[i].Gain_R, m_yPbPrGainOffsetData[i].Gain_G, m_yPbPrGainOffsetData[i].Gain_B);
					//}
					printk("\n");
				}
				printk("} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_SLR_VIP_TABLE_CUSTOM_TV001:
		{
			extern void* m_pVipTableCustom; /* SLR_VIP_TABLE_CUSTOM_TV001  */

			if (copy_from_user((SLR_VIP_TABLE_CUSTOM_TV001*)m_pVipTableCustom, (int __user *)arg, sizeof(SLR_VIP_TABLE_CUSTOM_TV001))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_SLR_VIP_TABLE_CUSTOM_TV001 fail\n");
				ret = -1;
				break;
			} else {
				SLR_VIP_TABLE_CUSTOM_TV001* tmp_pVipTableCustom = (SLR_VIP_TABLE_CUSTOM_TV001*)m_pVipTableCustom;
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_SLR_VIP_TABLE_CUSTOM_TV001 success\n");
				ret = 0;
				#if 1
				printk("m_pVipTableCustom->InitParam[16]={\n");
				for (i=0; i<16; i++) {
					//for (j=0; j<VIP_QUALITY_FUNCTION_TOTAL_CHECK; j++) {
						printk("%x ", tmp_pVipTableCustom->InitParam[i]);
					//}
					//printk("\n");
				}
				printk("} end\n");

				printk("m_pVipTableCustom->DCR_TABLE[][][]={\n");
				for (i=0; i<DCR_TABLE_ROW_NUM; i++) {
					for (j=0; j<DCR_TABLE_COL_NUM; j++) {
						printk("%x ", tmp_pVipTableCustom->DCR_TABLE[New_DCR_TABLE_NUM-1][i][j]);
					}
					printk("\n");
				}
				printk("} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_SLR_VIP_TABLE:
		{
			printk("%s %d\n", __FUNCTION__, __LINE__);

			#if 0
				printk("original m_defaultVipTable.VIP_QUALITY_Coef[][]={\n");
				for (i=0; i<3; i++) {
					for (j=0; j<VIP_QUALITY_FUNCTION_TOTAL_CHECK; j++) {
						printk("%x ", m_defaultVipTable.VIP_QUALITY_Coef[i][j]);
					}
					printk("\n");
				}
				printk("} end\n");

				printk("m_defaultVipTable.DrvSetting_Skip_Flag[]={\n");
				for (i=0; i<DrvSetting_Skip_Flag_item_Max; i++) {
					//for (j=0; j<VIP_QUALITY_FUNCTION_TOTAL_CHECK; j++) {
						printk("%x ", m_defaultVipTable.DrvSetting_Skip_Flag[i]);
					//}
					//printk("\n");
				}
				printk("} end\n");
			#endif
			if (copy_from_user(m_pVipTable, (int __user *)arg, sizeof(SLR_VIP_TABLE))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy SLR_VIP_TABLE fail\n");
				printk("%s %d\n", __FUNCTION__, __LINE__);

				ret = -1;
				break;
			} else {
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy SLR_VIP_TABLE success\n");
				printk("%s %d\n", __FUNCTION__, __LINE__);
				ret = 0;

				#if 0
				printk("**m_pVipTable->VIP_QUALITY_Coef[][]={\n");
				for (i=0; i<3; i++) {
					for (j=0; j<VIP_QUALITY_Coef_MAX; j++) {
						printk("%x ", m_pVipTable->VIP_QUALITY_Coef[i][j]);
					}
					printk("\n");
				}
				printk("} end\n");

				printk("m_pVipTable->DrvSetting_Skip_Flag[]={\n");
				for (i=0; i<DrvSetting_Skip_Flag_item_Max; i++) {
					//for (j=0; j<VIP_QUALITY_FUNCTION_TOTAL_CHECK; j++) {
						printk("%x ", m_pVipTable->DrvSetting_Skip_Flag[i]);
					//}
					//printk("\n");
				}
				printk("} end\n");
				#endif
			}
		#if 0
			if( (m_pVipTable->Auto_Function_Array3[0]==84)&& //is TV002
				(m_pVipTable->Auto_Function_Array3[1]==86)&&
				(m_pVipTable->Auto_Function_Array3[2]==48)&&
				(m_pVipTable->Auto_Function_Array3[3]==48)&&
				(m_pVipTable->Auto_Function_Array3[4]==50))
			{
				Scaler_set_Init_TV002(Project_TV002_Style_1);

				if( (m_pVipTable->Auto_Function_Array4[0]=='O')&&
					(m_pVipTable->Auto_Function_Array4[1]=='S')&&
					(m_pVipTable->Auto_Function_Array4[2]=='D')) {
					#if IS_ENABLED(CONFIG_RTK_KDRV_GDMA)
					extern int GDMA_SetAlphaOsdDetectionParameter(int data);

					GDMA_SetAlphaOsdDetectionParameter(m_pVipTable->Auto_Function_Array4[3]);
                    #endif
				}
			}
		#endif

		}
		printk("%s %d\n", __FUNCTION__, __LINE__);
		break;

		case VPQ_EXTERN_IOC_SET_PIC_MODE_DATA:
		{
			extern PIC_MODE_DATA*  m_picModeTable;
			if (copy_from_user(m_picModeTable, (int __user *)arg, sizeof(PIC_MODE_DATA))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_PIC_MODE_DATA fail\n");
				ret = -1;
				break;
			} else {
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_PIC_MODE_DATA success\n");
				ret = 0;
				#if 1
				printk("m_picModeTable={\n");
				printk("%x %x %x \n", m_picModeTable->picMode[PICTURE_MODE_USER].Brightness, m_picModeTable->picMode[PICTURE_MODE_USER].Contrast, m_picModeTable->picMode[PICTURE_MODE_USER].Saturation);
				printk("%x %x %x \n", m_picModeTable->picMode[PICTURE_MODE_VIVID].Brightness, m_picModeTable->picMode[PICTURE_MODE_VIVID].Contrast, m_picModeTable->picMode[PICTURE_MODE_VIVID].Saturation);
				printk("%x %x %x \n", m_picModeTable->picMode[PICTURE_MODE_STD].Brightness, m_picModeTable->picMode[PICTURE_MODE_STD].Contrast, m_picModeTable->picMode[PICTURE_MODE_STD].Saturation);
				printk("} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_PIC_MODE_DATA_defaultPictureModeSet:
		{
			extern SLR_PICTURE_MODE_DATA  m_defaultPictureModeSet[];
			if (copy_from_user(&m_defaultPictureModeSet, (int __user *)arg, sizeof(SLR_PICTURE_MODE_DATA)*7)) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_PIC_MODE_DATA_defaultPictureModeSet fail\n");
				ret = -1;
				break;
			} else {
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_PIC_MODE_DATA_defaultPictureModeSet success\n");
				ret = 0;
				#if 1
				printk("m_defaultPictureModeSet={\n");
				printk("%x %x %x \n", m_defaultPictureModeSet[PICTURE_MODE_USER].Brightness, m_defaultPictureModeSet[PICTURE_MODE_USER].Contrast, m_defaultPictureModeSet[PICTURE_MODE_USER].Saturation);
				printk("%x %x %x \n", m_defaultPictureModeSet[PICTURE_MODE_VIVID].Brightness, m_defaultPictureModeSet[PICTURE_MODE_VIVID].Contrast, m_defaultPictureModeSet[PICTURE_MODE_VIVID].Saturation);
				printk("%x %x %x \n", m_defaultPictureModeSet[PICTURE_MODE_GAME].Brightness, m_defaultPictureModeSet[PICTURE_MODE_GAME].Contrast, m_defaultPictureModeSet[PICTURE_MODE_GAME].Saturation);
				printk("} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_PIC_MODE_DATA_DolbyHDRPictureModeSet:
		{
			extern SLR_PICTURE_MODE_DATA  m_DolbyHDRPictureModeSet[];
			if (copy_from_user(&m_DolbyHDRPictureModeSet, (int __user *)arg, sizeof(SLR_PICTURE_MODE_DATA)*7)) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_PIC_MODE_DATA_DolbyHDRPictureModeSet fail\n");
				ret = -1;
				break;
			} else {
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_PIC_MODE_DATA_DolbyHDRPictureModeSet success\n");
				ret = 0;
				#if 1
				printk("m_DolbyHDRPictureModeSet={\n");
				printk("%x %x %x \n", m_DolbyHDRPictureModeSet[PICTURE_MODE_USER].Brightness, m_DolbyHDRPictureModeSet[PICTURE_MODE_USER].Contrast, m_DolbyHDRPictureModeSet[PICTURE_MODE_USER].Saturation);
				printk("%x %x %x \n", m_DolbyHDRPictureModeSet[PICTURE_MODE_VIVID].Brightness, m_DolbyHDRPictureModeSet[PICTURE_MODE_VIVID].Contrast, m_DolbyHDRPictureModeSet[PICTURE_MODE_VIVID].Saturation);
				printk("%x %x %x \n", m_DolbyHDRPictureModeSet[PICTURE_MODE_STD].Brightness, m_DolbyHDRPictureModeSet[PICTURE_MODE_STD].Contrast, m_DolbyHDRPictureModeSet[PICTURE_MODE_STD].Saturation);
				printk("} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_SLD:
		{
			unsigned long useraddr;// = HAL_VPQ_DATA_EXTERN.pData;
			if (copy_from_user(&HAL_VPQ_DATA_EXTERN, (void __user *)arg, sizeof(HAL_VPQ_DATA_EXTERN_T))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_SLD, PQModeInfo struct copy fail\n");
				ret = -1;
				break;
			}

			if (HAL_VPQ_DATA_EXTERN.length != 4) { // 1 unsigned int = 4 bytes
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_SLD, size error\n");
				ret = -1;
				break;
			}
			useraddr = HAL_VPQ_DATA_EXTERN.pData;
			if(copy_from_user(&memc_logo_to_demura_drop_limit, (void __user *)useraddr, HAL_VPQ_DATA_EXTERN.length))
			{
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_SLD, table copy fail\n");
				ret = -1;
			} else {
				rtd_printk(KERN_INFO, TAG_NAME, "VPQ_EXTERN_IOC_SET_SLD, memc_logo_to_demura_drop_limit=%d\n", memc_logo_to_demura_drop_limit);
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_StructColorDataType:
		{
			extern StructColorDataType*  m_colorDataTable;
			if (copy_from_user(m_colorDataTable, (int __user *)arg, sizeof(StructColorDataType))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataType fail\n");
				ret = -1;
				break;
			} else {
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataType success\n");
				ret = 0;
				#if 1
				printk("m_colorDataTable={\n");
				printk("%x %x %x ", m_colorDataTable->MBPeaking, m_colorDataTable->Intensity, m_colorDataTable->Gamma);
				printk("} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_StructColorDataType_defaultSDColorData:
		{
			extern StructColorDataType  m_defaultSDColorData;
			if (copy_from_user(&m_defaultSDColorData, (int __user *)arg, sizeof(StructColorDataType))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataType_defaultSDColorData fail\n");
				ret = -1;
				break;
			} else {
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataType_defaultSDColorData success\n");
				ret = 0;
				#if 1
				printk("m_defaultSDColorData={\n");
				printk("%x %x %x ", m_defaultSDColorData.MBPeaking, m_defaultSDColorData.Intensity, m_defaultSDColorData.Gamma);
				printk("} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_StructColorDataType_defaultHDColorData:
		{
			extern StructColorDataType  m_defaultHDColorData;
			if (copy_from_user(&m_defaultHDColorData, (int __user *)arg, sizeof(StructColorDataType))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataType_defaultHDColorData fail\n");
				ret = -1;
				break;
			} else {
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataType_defaultHDColorData success\n");
				ret = 0;
				#if 1
				printk("m_defaultHDColorData={\n");
				printk("%x %x %x ", m_defaultHDColorData.MBPeaking, m_defaultHDColorData.Intensity, m_defaultHDColorData.Gamma);
				printk("} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_COLOR_TEMP_DATA:
		{
			extern COLOR_TEMP_DATA*  m_colorTempData;
			if (copy_from_user(m_colorTempData, (int __user *)arg, sizeof(COLOR_TEMP_DATA))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_COLOR_TEMP_DATA fail\n");
				ret = -1;
				break;
			} else {
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_COLOR_TEMP_DATA success\n");
				ret = 0;
				#if 1
				printk("m_colorTempData={\n");
				printk("%x %x %x \n", m_colorTempData->colorTempData[SLR_COLORTEMP_USER].R_val, m_colorTempData->colorTempData[SLR_COLORTEMP_USER].G_val, m_colorTempData->colorTempData[SLR_COLORTEMP_USER].B_val);
				printk("%x %x %x \n", m_colorTempData->colorTempData[SLR_COLORTEMP_NORMAL].R_val, m_colorTempData->colorTempData[SLR_COLORTEMP_NORMAL].G_val, m_colorTempData->colorTempData[SLR_COLORTEMP_NORMAL].B_val);
				printk("%x %x %x \n", m_colorTempData->colorTempData[SLR_COLORTEMP_WARMER].R_val, m_colorTempData->colorTempData[SLR_COLORTEMP_WARMER].G_val, m_colorTempData->colorTempData[SLR_COLORTEMP_WARMER].B_val);
				printk("} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_COLOR_TEMP_DATA_defColorTempSet:
		{
			extern SLR_COLORTEMP_DATA  m_defColorTempSet[];
			if (copy_from_user(&m_defColorTempSet, (int __user *)arg, sizeof(SLR_COLORTEMP_DATA)*SLR_COLORTEMP_MAX_NUM)) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_COLOR_TEMP_DATA_defColorTempSet fail\n");
				ret = -1;
				break;
			} else {
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_COLOR_TEMP_DATA_defColorTempSet success\n");
				ret = 0;
				#if 1
				printk("m_defColorTempSet={\n");
				printk("%x %x %x \n", m_defColorTempSet[SLR_COLORTEMP_USER].R_val, m_defColorTempSet[SLR_COLORTEMP_USER].G_val, m_defColorTempSet[SLR_COLORTEMP_USER].B_val);
				printk("%x %x %x \n", m_defColorTempSet[SLR_COLORTEMP_NORMAL].R_val, m_defColorTempSet[SLR_COLORTEMP_NORMAL].G_val, m_defColorTempSet[SLR_COLORTEMP_NORMAL].B_val);
				printk("%x %x %x \n", m_defColorTempSet[SLR_COLORTEMP_WARMER].R_val, m_defColorTempSet[SLR_COLORTEMP_WARMER].G_val, m_defColorTempSet[SLR_COLORTEMP_WARMER].B_val);
				printk("} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_StructColorDataFacModeType:
		{
			extern StructColorDataFacModeType* m_colorFacModeTable;
			if (copy_from_user(m_colorFacModeTable, (int __user *)arg, sizeof(StructColorDataFacModeType))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType fail\n");
				ret = -1;
				break;
			} else {
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType success\n");
				ret = 0;
				#if 1
				printk("m_colorFacModeTable={\n");
				printk("%x %x %x ", m_colorFacModeTable->Brightness_0, m_colorFacModeTable->Brightness_50, m_colorFacModeTable->Brightness_100);
				printk("} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_defaultColorFacMode:
		{
			extern StructColorDataFacModeType m_defaultColorFacMode;
			if (copy_from_user(&m_defaultColorFacMode, (int __user *)arg, sizeof(StructColorDataFacModeType))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_defaultColorFacMode fail\n");
				ret = -1;
				break;
			} else {
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_defaultColorFacMode success\n");
				ret = 0;
				#if 1
				printk("m_defaultColorFacMode={\n");
				printk("%x %x %x ", m_defaultColorFacMode.Brightness_0, m_defaultColorFacMode.Brightness_50, m_defaultColorFacMode.Brightness_100);
				printk("} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_AvColorFacMode:
		{
			extern StructColorDataFacModeType m_AvColorFacMode;
			if (copy_from_user(&m_AvColorFacMode, (int __user *)arg, sizeof(StructColorDataFacModeType))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_AvColorFacMode fail\n");
				ret = -1;
				break;
			} else {
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_AvColorFacMode success\n");
				ret = 0;
				#if 1
				printk("m_AvColorFacMode={\n");
				printk("%x %x %x ", m_AvColorFacMode.Brightness_0, m_AvColorFacMode.Brightness_50, m_AvColorFacMode.Brightness_100);
				printk("} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_YppColorFacMode:
		{
			extern StructColorDataFacModeType m_YppColorFacMode;
			if (copy_from_user(&m_YppColorFacMode, (int __user *)arg, sizeof(StructColorDataFacModeType))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_YppColorFacMode fail\n");
				ret = -1;
				break;
			} else {
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_YppColorFacMode success\n");
				ret = 0;
				#if 1
				printk("m_YppColorFacMode={\n");
				printk("%x %x %x ", m_YppColorFacMode.Brightness_0, m_YppColorFacMode.Brightness_50, m_YppColorFacMode.Brightness_100);
				printk("} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_VgaColorFacMode:
		{
			extern StructColorDataFacModeType m_VgaColorFacMode;
			if (copy_from_user(&m_VgaColorFacMode, (int __user *)arg, sizeof(StructColorDataFacModeType))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_VgaColorFacMode fail\n");
				ret = -1;
				break;
			} else {
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_VgaColorFacMode success\n");
				ret = 0;
				#if 1
				printk("m_VgaColorFacMode={\n");
				printk("%x %x %x ", m_VgaColorFacMode.Brightness_0, m_VgaColorFacMode.Brightness_50, m_VgaColorFacMode.Brightness_100);
				printk("} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_HdmiColorFacMode:
		{
			extern StructColorDataFacModeType m_HdmiColorFacMode;
			if (copy_from_user(&m_HdmiColorFacMode, (int __user *)arg, sizeof(StructColorDataFacModeType))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_HdmiColorFacMode fail\n");
				ret = -1;
				break;
			} else {
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_HdmiColorFacMode success\n");
				ret = 0;
				#if 1
				printk("m_HdmiColorFacMode={\n");
				printk("%x %x %x ", m_HdmiColorFacMode.Brightness_0, m_HdmiColorFacMode.Brightness_50, m_HdmiColorFacMode.Brightness_100);
				printk("} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_HdmiSDColorFacMode:
		{
			extern StructColorDataFacModeType m_HdmiSDColorFacMode;
			if (copy_from_user(&m_HdmiSDColorFacMode, (int __user *)arg, sizeof(StructColorDataFacModeType))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_HdmiSDColorFacMode fail\n");
				ret = -1;
				break;
			} else {
				rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_HdmiSDColorFacMode success\n");
				ret = 0;
				#if 1
				printk("m_HdmiSDColorFacMode={\n");
				printk("%x %x %x ", m_HdmiSDColorFacMode.Brightness_0, m_HdmiSDColorFacMode.Brightness_50, m_HdmiSDColorFacMode.Brightness_100);
				printk("} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_GET_CUR_SRC_FMT:
		{
			printk("VPQ_EXTERN_IOC_GET_CUR_SRC_FMT\n");
			srcfmt = Scaler_InputSrcFormat(Scaler_color_get_pq_src_idx());
			if (copy_to_user((void __user *)arg, (void *)&srcfmt, sizeof(UINT32))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_CUR_SRC_FMT2 fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;

		case VPQ_EXTERN_IOC_GET_VIP_SRC_TIMING:
		{
			printk("VPQ_EXTERN_IOC_GET_VIP_SRC_TIMING\n");
			vipsource = fwif_vip_source_check(3, NOT_BY_DISPLAY);
			if (copy_to_user((void __user *)arg, (void *)&vipsource, sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_VIP_SRC_TIMING fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;

		case VPQ_EXTERN_IOC_SET_StructColorDataFacMode_EXTERN:
		{
			extern unsigned char g_bUseMiddleWareOSDmap; //for Driver Base MiddleWare OSD map
			extern StructColorDataFacModeType g_curColorFacTableEx; //for Driver Base MiddleWare OSD map

			if (copy_from_user(&g_curColorFacTableEx, (void __user *)arg, sizeof(StructColorDataFacModeType))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacMode_EXTERN fail\n");
				ret = -1;
				break;
			} else {
				//rtd_printk(KERN_DEBUG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacMode_EXTERN success\n");
				g_bUseMiddleWareOSDmap = 1;
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_GET_VIP_SRC_RESYNC:
		{
			unsigned int vip_src_resync = 0;
			vip_src_resync = Scaler_Get_VIP_src_resync_flag();
			if (copy_to_user((void __user *)arg, (void *)&vip_src_resync, sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_VIP_SRC_RESYNC fail\n");
				ret = -1;
			} else {
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_VIP_SRC_RESYNC:
		{
			Scaler_Set_PicMode_VIP_src(fwif_vip_source_check(3, NOT_BY_DISPLAY));
		}
		break;

		case VPQ_EXTERN_IOC_SET_VIP_SET_VIP_HANDLER_RESET:
		{
			rtd_printk(KERN_INFO, TAG_NAME, "VPQ_EXTERN_IOC_SET_VIP_SET_VIP_HANDLER_RESET\n");
			ret = Scaler_Set_VIP_HANDLER_RESET();
			if (ret) {
				ret = 0;
				printk("Scaler_Set_VIP_HANDLER_RESET success\n");
			}
			else {
				ret = -1;
				printk("Scaler_Set_VIP_HANDLER_RESET fail\n");
			}
		}
		break;

		case VPQ_EXTERN_IOC_GET_BRIGHTNESS:
		{
			brightness = (unsigned int)Scaler_GetBrightness();
			if (copy_to_user((void __user *)arg, (void *)&brightness, sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_BRIGHTNESS fail\n");
				ret = -1;
			} else
				ret = 0;

		}
		break;
		case VPQ_EXTERN_IOC_SET_BRIGHTNESS:
		{			
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {
				ret = -1;
			} else {
				ret = 0;
				Scaler_SetBrightness((unsigned char)args);
				HSBC_Chg = true;
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_CONTRAST:
		{
			contrast = (unsigned int)Scaler_GetContrast();
			if (copy_to_user((void __user *)arg, (void *)&contrast, sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_CONTRAST fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;
		case VPQ_EXTERN_IOC_SET_CONTRAST:
		{
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetContrast((unsigned char)args);
				HSBC_Chg = true;
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_SATURATION:
		{
			saturation = (unsigned int)Scaler_GetSaturation();
			if (copy_to_user((void __user *)arg, (void *)&saturation, sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_SATURATION fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;
		case VPQ_EXTERN_IOC_SET_SATURATION:
		{
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetSaturation((unsigned char)args);
				HSBC_Chg = true;
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_HUE:
		{
			hue = (unsigned int)Scaler_GetHue();
			if (copy_to_user((void __user *)arg, (void *)&hue, sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_HUE fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;

		case VPQ_EXTERN_IOC_SET_HUE:
		{
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetHue((unsigned char)args);
				HSBC_Chg = true;
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_NR:
		{
			printk("VPQ_EXTERN_IOC_GET_NR\n");
			nr = (unsigned int)Scaler_GetDNR();
			if (copy_to_user((void __user *)arg, (void *)&nr, sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_NR fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;
		case VPQ_EXTERN_IOC_SET_NR:
		{
			printk("VPQ_EXTERN_IOC_SET_NR\n");
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetDNR((unsigned char)args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_SHARPNESS:
		{
			printk("VPQ_EXTERN_IOC_GET_SHARPNESS\n");
			sharpness = (unsigned int)Scaler_GetSharpness();
			if (copy_to_user((void __user *)arg, (void *)&sharpness, sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_SHARPNESS fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;
		case VPQ_EXTERN_IOC_SET_SHARPNESS_EXTERN:
		{
			printk("VPQ_EXTERN_IOC_SET_SHARPNESS_EXTERN\n");
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetSharpness((unsigned char)args);
#if defined(CONFIG_RTK_8KCODEC_INTERFACE) || defined(CONFIG_H5CX_SUPPORT)
				VPQEX_rlink_host_OSD_Sharpness_Info_Send();
#endif
				HSBC_Chg = true;
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_DE_BLOCKING:
		{
			printk("VPQ_EXTERN_IOC_GET_DE_BLOCKING\n");
			dbLevel = (TV001_LEVEL_E)Scaler_GetMPEGNR();
			if (copy_to_user((void __user *)arg, (void *)&dbLevel, sizeof(TV001_LEVEL_E))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_DE_BLOCKING fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;
		case VPQ_EXTERN_IOC_SET_DE_BLOCKING:
		{
			printk("VPQ_EXTERN_IOC_SET_DE_BLOCKING\n");
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetMPEGNR((unsigned char)args,0);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_COLOR_TEMP:
		{
			printk("VPQ_EXTERN_IOC_GET_COLOR_TEMP\n");

			if (Scaler_GetColorTemp_level_type(&colorTemp) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetColorTemp_level_type fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&colorTemp, sizeof(TV001_COLORTEMP_E))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_COLOR_TEMP fail\n");
					ret = -1;
				} else
					ret = 0;
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_COLOR_TEMP_EXTERN:
		{
			rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_COLOR_TEMP_EXTERN\n");
			if (copy_from_user(&color_temp_data_s,  (int __user *)arg,sizeof(TV001_COLOR_TEMP_DATA_S))) {

				ret = -1;
			} else {
				ret = 0;

				if (!g_WBBL_WBBS_enable) {
					//Scaler_SetColorTemp_level_type((TV001_COLORTEMP_E)color_temp_data_s);
					Scaler_SetColorTempData((TV001_COLOR_TEMP_DATA_S*) &color_temp_data_s);

					if(fwif_VIP_get_Project_ID() == VIP_Project_ID_TV030){
						//fwif_color_set_OutputGamma_System();
					}
					else {
						fwif_color_set_InvOutputGamma_System(color_temp_data_s.gamma_curve_index);
						rtd_printk(KERN_EMERG, TAG_NAME, "fwif_color_set_InvOutputGamma_System, gamma_curve_index %d\n", color_temp_data_s.gamma_curve_index);
					}
				}

				#if IS_ENABLED(CONFIG_RTK_AI_DRV)
				memcpy(&vpqex_color_temp, &color_temp_data_s, sizeof(TV001_COLOR_TEMP_DATA_S));
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_Panel_Dither:
		{
			if (copy_from_user(&Panel_Dither_Info, (int __user *)arg, sizeof(HAL_VPQ_Panel_Dither_T))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_Panel_Dither fail\n");
				ret = -1;
				break;
			} else {
				printk("kernel VPQ_EXTERN_IOC_SET_Panel_Dither en=%d,bit=%d,type=%d\n",Panel_Dither_Info.Enable,Panel_Dither_Info.default_vip_panel_bit,Panel_Dither_Info.nTableType);
				fwif_color_set_Panel_Dither(Panel_Dither_Info.Enable, (VIP_PANEL_BIT)Panel_Dither_Info.default_vip_panel_bit, Panel_Dither_Info.nTableType);
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_GET_BACKLIGHT:
		{
			printk("VPQ_EXTERN_IOC_GET_BACKLIGHT\n");
			backlight = (unsigned int)Scaler_GetBacklight();
			if (copy_to_user((void __user *)arg, (void *)&backlight, sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_BACKLIGHT fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;
		case VPQ_EXTERN_IOC_SET_BACKLIGHT:
		{
			printk("VPQ_EXTERN_IOC_SET_BACKLIGHT\n");
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetBackLight((unsigned char)args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_PANEL_BL:
		{
			printk("VPQ_EXTERN_IOC_SET_PANEL_BL\n");
		}
		break;
		case VPQ_EXTERN_IOC_SET_GAMMA:
		{
			printk("VPQ_EXTERN_IOC_SET_GAMMA\n");
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetGamma((unsigned char)args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_PQ_MODULE:
		{
			printk("VPQ_EXTERN_IOC_GET_PQ_MODULE\n");
			if (copy_from_user(&PQModule_T,  (int __user *)arg,sizeof(RTK_PQModule_T))) {

				ret = -1;
			} else {
				ret = 0;
				if (Scaler_GetPQModule((TV001_PQ_MODULE_E) PQModule_T.PQModule,(unsigned char *) &PQModule_T.onOff) == FALSE) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetPQModule fail\n");
					ret = -1;
				} else {
					if (copy_to_user((void __user *)arg, (void *)&PQModule_T, sizeof(RTK_PQModule_T))) {
						rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_PQ_MODULE fail\n");
						ret = -1;
					} else
						ret = 0;
				}
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_PQ_MODULE:
		{
			printk("VPQ_EXTERN_IOC_SET_PQ_MODULE\n");
			//unsigned int args = 0;
			if (copy_from_user(&PQModule_T,  (int __user *)arg,sizeof(RTK_PQModule_T))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetPQModule((TV001_PQ_MODULE_E) PQModule_T.PQModule,(unsigned char)PQModule_T.onOff);
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_DEMO:
		{
			printk("VPQ_EXTERN_IOC_SET_DEMO\n");
			//unsigned int args = 0;
			if (copy_from_user(&DEMO_MODE_T,  (int __user *)arg,sizeof(RTK_DEMO_MODE_T))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetDemoMode((TV001_DEMO_MODE_E) DEMO_MODE_T.DEMO_MODE,(unsigned char)DEMO_MODE_T.onOff);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_DEBUG_MODE:
		{
			printk("VPQ_EXTERN_IOC_GET_DEBUG_MODE\n");
		}
		break;
		case VPQ_EXTERN_IOC_SET_DEBUG_MODE:
		{
			printk("VPQ_EXTERN_IOC_SET_DEBUG_MODE\n");
		}
		break;
		case VPQ_EXTERN_IOC_GET_SAT_HISTOGRAM:
		{
			if (fwif_color_get_HistoData_chrmbins(COLOR_AutoSat_HISTOGRAM_LEVEL, &(SatHistoData_bin[0])) == FALSE) {
				printk("kernel VPQ_EXTERN_IOC_GET_SAT_HISTOGRAM fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&SatHistoData_bin[0], sizeof(SatHistoData_bin))) {
					printk("kernel VPQ_EXTERN_IOC_GET_SAT_HISTOGRAM fail\n");
					ret = -1;
				} else{
					ret = 0;
				}
			}
		}
		break;

		case VPQ_EXTERN_IOC_GET_HISTOGRAM:
		{
			//printk("VPQ_EXTERN_IOC_GET_HISTOGRAM\n");
			if (fwif_color_get_HistoData_Countbins(TV006_VPQ_chrm_bin, &(HistoData_bin[0])) == FALSE) {
				printk("kernel VPQ_IOC_GET_HistoData_Countbins fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&HistoData_bin[0], sizeof(HistoData_bin))) {
					printk("kernel VPQ_IOC_GET_HistoData_Countbins fail\n");
					ret = -1;
				} else{
					ret = 0;
				}
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_HUE_HISTOGRAM:
		{
			if (fwif_color_get_HistoData_huebins(TV006_VPQ_chrm_bin, &(HistoData_bin[0])) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_IOC_GET_HistoData_huebins fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&HistoData_bin[0], sizeof(HistoData_bin))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_IOC_GET_HistoData_huebins fail\n");
					ret = -1;
				} else
					ret = 0;
			}

		}
		break;
		case VPQ_EXTERN_IOC_SET_BLACK_PATTERN_OUTPUT:
		{
			printk("VPQ_EXTERN_IOC_SET_BLACK_PATTERN_OUTPUT\n");
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetBlackPatternOutput((unsigned char)args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_WHITE_PATTERN_OUTPUT:
		{
			printk("VPQ_EXTERN_IOC_SET_WHITE_PATTERN_OUTPUT\n");
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetWhitePatternOutput((unsigned char)args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_SRC_HDR_INFO:
		{
			printk("VPQ_EXTERN_IOC_GET_SRC_HDR_INFO\n");

			if (Scaler_GetSrcHdrInfo(&GammaType) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetSrcHdrInfo fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&GammaType, sizeof(unsigned int))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_SRC_HDR_INFO fail\n");
					ret = -1;
				} else
					ret = 0;
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_SDR2HDR:
		{
			printk("VPQ_EXTERN_IOC_GET_SDR2HDR\n");

			if (Scaler_GetSDR2HDR(&onoff) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetSDR2HDR fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&onoff, sizeof(unsigned char))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_SDR2HDR fail\n");
					ret = -1;
				} else
					ret = 0;
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_SDR2HDR:
		{
			printk("VPQ_EXTERN_IOC_SET_SDR2HDR\n");
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetSDR2HDR((unsigned char)args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_HDR_TYPE:
		{
			//printk("VPQ_EXTERN_IOC_GET_HDR_TYPE\n");

			if (Scaler_GetHdrType(&HdrType) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetHdrType fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&HdrType, sizeof(unsigned int))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_HDR_TYPE fail\n");
					ret = -1;
				} else
					ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_DCI_TYPE:
		{
			printk("VPQ_EXTERN_IOC_SET_DCI_TYPE\n");
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetDCC_Table((unsigned char)args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_DOLBY_HDR_PIC_MODE:
		{
			printk("VPQ_EXTERN_IOC_GET_DOLBY_HDR_PIC_MODE\n");
		}
		break;
		case VPQ_EXTERN_IOC_SET_DOLBY_HDR_PIC_MODE:
		{
			printk("VPQ_EXTERN_IOC_SET_DOLBY_HDR_PIC_MODE\n");
		}
		break;


		case VPQ_EXTERN_IOC_GET_HDR10_ENABLE:
		{
			printk("VPQ_EXTERN_IOC_GET_HDR10_ENABLE\n");

			if (Scaler_GetHdr10Enable(&bEnable) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetHdr10Enable fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&bEnable, sizeof(unsigned char))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_HDR10_ENABLE fail\n");
					ret = -1;
				} else
					ret = 0;
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_HDR10_ENABLE:
		{
			printk("VPQ_EXTERN_IOC_SET_HDR10_ENABLE\n");
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetHdr10Enable((unsigned char)args);
			}
		}
		break;

		case VPQ_EXTERN_IOC_GET_DOLBYHDR_ENABLE:
		{
			printk("VPQ_EXTERN_IOC_GET_HDR10_ENABLE\n");

			if (Scaler_GetDOLBYHDREnable(&bEnable) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetDOLBYHDREnable fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&bEnable, sizeof(unsigned char))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_DOLBYHDR_ENABLE fail\n");
					ret = -1;
				} else
					ret = 0;
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_DOLBYHDR_ENABLE:
		{
			printk("VPQ_EXTERN_IOC_SET_DOLBYHDR_ENABLE\n");
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetDOLBYHDREnable((unsigned char)args);
			}
		}
		break;


		case VPQ_EXTERN_IOC_SET_DREAM_PANEL_LOG_ENABLE:
		{
			printk("VPQ_EXTERN_IOC_SET_DREAM_PANEL_LOG_ENABLE\n");
		}
		break;
		case VPQ_EXTERN_IOC_GET_DREAM_PANEL_BACKLIGHT:
		{
			printk("VPQ_EXTERN_IOC_GET_DREAM_PANEL_BACKLIGHT\n");
		}
		break;
		case VPQ_EXTERN_IOC_SET_DREAM_PANEL_BACKLIGHT:
		{
			printk("VPQ_EXTERN_IOC_SET_DREAM_PANEL_BACKLIGHT\n");
		}
		break;
		case VPQ_EXTERN_IOC_GET_DCC_MODE:
		{
			dcc_mode = (unsigned int)Scaler_GetDCC_Mode();
			if (copy_to_user((void __user *)arg, (void *)&dcc_mode, sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_DCC_MODE fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;
		case VPQ_EXTERN_IOC_SET_DCC_MODE:
		{
			if (copy_from_user(&dcc_mode,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				#ifdef ENABLE_DE_CONTOUR_I2R
				if(Eng_Skyworth_Decont_Pattern)
					return ret;
				#endif
				printk("dynamic contrast level=%d\n",dcc_mode);
				aipq_DynamicContrastLevel=dcc_mode;
				Scaler_SetDCC_Mode(dcc_mode&0xff);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_ICM_TBL_IDX:
		{
			icm_table_idx = Scaler_GetICMTable();
			if (copy_to_user((void __user *)arg, (void *)&icm_table_idx, sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_ICM_TBL_IDX fail\n");
				ret = -1;
			} else
				ret = 0;

		}
		break;
		case VPQ_EXTERN_IOC_SET_ICM_TBL_IDX:
		{
			if (copy_from_user(&icm_table_idx,  (int __user *)arg,sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_ICM_TBL_IDX fail\n");
				ret = -1;
			} else {
				printk("VPQ_EXTERN_IOC_SET_ICM_TBL_IDX,icm_table_idx=%d\n",icm_table_idx);
				Scaler_SetICMTable(icm_table_idx&0xff);
				ret = 0;
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_FILM_MODE:
		{
			film_mode_enable = film_mode.film_status?1:0;
			if (copy_to_user((void __user *)arg, (void *)&film_mode_enable, sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_FILM_MODE fail\n");
				ret = -1;
			} else
				ret = 0;

		}
		break;
		case VPQ_EXTERN_IOC_SET_FILM_MODE:
		{
			if (copy_from_user(&film_mode_enable,  (int __user *)arg,sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_FILM_MODE fail\n");
				ret = -1;
			} else {
				film_mode.film_status = film_mode_enable?28:0;
				down(get_DI_semaphore());
				drvif_module_film_mode((DRV_film_mode *) &film_mode);
				up(get_DI_semaphore());
				ret = 0;
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_HDMI_CSC_DATA_RANGE_MODE:
		{
			range_mode = Scaler_GetHDMI_CSC_DataRange_Mode();
			if (copy_to_user((void __user *)arg, (void *)&range_mode, sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_HDMI_CSC_DATA_RANGE_MODE fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;
		case VPQ_EXTERN_IOC_SET_HDMI_CSC_DATA_RANGE_MODE:
		{
			if (copy_from_user(&range_mode,  (int __user *)arg,sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_FILM_MODE fail\n");
				ret = -1;
			} else {
				extern unsigned char g_HDR_mode;
				extern unsigned char g_HDR_color_format;
				_system_setting_info *VIP_system_info_structure_table = NULL;
				VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
				Scaler_SetHDMI_CSC_DataRange_Mode((HDMI_CSC_DATA_RANGE_MODE_T)range_mode);
				if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI) {
					if (VIP_system_info_structure_table != NULL &&
						(VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_HDR10 ||
						VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_HLG))
						Scaler_Set_HDR_YUV2RGB(g_HDR_mode, g_HDR_color_format);
				}
				Scaler_SetDataFormatHandler();
				ret = 0;
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_HDR_FORCE_MODE:
		{
			if (copy_from_user(&hdr_force_mode,  (int __user *)arg,sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_HDR_FORCE_MODE fail\n");
				ret = -1;
			} else {
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_CINEMA_PIC_MODE:
		{
			if (copy_from_user(&cinema_pic_mode,  (int __user *)arg,sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_CINEMA_PIC_MODE fail\n");
				ret = -1;
			} else {
				//#ifndef CONFIG_CUSTOMER_TV002
				//Scaler_Set_CinemaMode_PQ(cinema_pic_mode);
				//#endif
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_PQ_BYPASS_LV:
		{
			if (copy_from_user(&pq_bypass_lv,  (int __user *)arg,sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_PQ_BYPASS_LV fail\n");
				ret = -1;
			} else {
				g_pq_bypass_lv = pq_bypass_lv;
				if (pq_bypass_lv < VIP_PQ_ByPass_TBL_Max)
					Scaler_color_Set_PQ_ByPass_Lv(pq_bypass_lv);
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_PQ_SOURCE_TYPE:
		{
			unsigned int pq_source_from_ap = 0;
			if (copy_from_user(&pq_source_from_ap,  (int __user *)arg,sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_PQ_SOURCE_TYPE fail\n");
				ret = -1;
			} else {
				fwif_color_set_PQ_SOURCE_TYPE_From_AP((unsigned char)pq_source_from_ap);
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_ICM_byAP:
		{
			if (!g_bEnable_PQ_extend_data) return 0;

			if (copy_from_user(&HAL_VPQ_DATA_EXTERN, (void __user *)arg, sizeof(HAL_VPQ_DATA_EXTERN_T))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_ICM_byAP, PQModeInfo struct copy fail\n");
				ret = -1;
				break;
			}

			if (HAL_VPQ_DATA_EXTERN.length > 0x400000) { //Max 4MB
				ret = -1;
				break;
			}

			pBuff_tmp = (unsigned char *)dvr_malloc_specific(HAL_VPQ_DATA_EXTERN.length * sizeof(char), GFP_DCU2_FIRST);
			if (pBuff_tmp == NULL) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_ICM_byAP dynamic alloc memory fail!!!\n");
				ret = -1;
			} else {
				unsigned long useraddr = HAL_VPQ_DATA_EXTERN.pData;
				if(copy_from_user(pBuff_tmp, (void __user *)useraddr, HAL_VPQ_DATA_EXTERN.length * sizeof(char)))
				{
					rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_ICM_byAP, table copy fail\n");
					ret = -1;
				}
				else {
					rtd_printk(KERN_INFO, TAG_NAME, "VPQ_EXTERN_IOC_SET_ICM_byAP, pBuff_tmp[0]=%d\n", pBuff_tmp[0]);
					if (sizeof(VIP_PQ_EXT_ICM_DATA) == HAL_VPQ_DATA_EXTERN.length) {
						VIP_PQ_EXT_ICM_DATA *pICMData = (VIP_PQ_EXT_ICM_DATA *)pBuff_tmp;
						rtd_printk(KERN_INFO, TAG_NAME, "Start set PQ EXT ICM DATA\n");
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_ICM_Table, pICMData->pICM_Table);

						Scaler_set_ICM_table(Scaler_get_ICM_table(), 0);
						save_PQ_Extend_Data_Index[PQ_EXTEND_DATA_ENUM_ICM] = pICMData->index;
					}
					ret = 0;
				}
				dvr_free(pBuff_tmp);
			}
			break;
		}

		case VPQ_EXTERN_IOC_SET_DCC_byAP:
		{
			if (!g_bEnable_PQ_extend_data) return 0;

			if (copy_from_user(&HAL_VPQ_DATA_EXTERN, (void __user *)arg, sizeof(HAL_VPQ_DATA_EXTERN_T))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_DCC_byAP, PQModeInfo struct copy fail\n");
				ret = -1;
				break;
			}

			if (HAL_VPQ_DATA_EXTERN.length > 0x400000) { //Max 4MB
				ret = -1;
				break;
			}

			pBuff_tmp = (unsigned char *)dvr_malloc_specific(HAL_VPQ_DATA_EXTERN.length * sizeof(char), GFP_DCU2_FIRST);
			if (pBuff_tmp == NULL) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_DCC_byAP dynamic alloc memory fail!!!\n");
				ret = -1;
			} else {
				unsigned long useraddr = HAL_VPQ_DATA_EXTERN.pData;
				if(copy_from_user(pBuff_tmp, (void __user *)useraddr, HAL_VPQ_DATA_EXTERN.length * sizeof(char)))
				{
					rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_DCC_byAP, table copy fail\n");
					ret = -1;
				}
				else {
					rtd_printk(KERN_INFO, TAG_NAME, "VPQ_EXTERN_IOC_SET_DCC_byAP, pBuff_tmp[0]=%d\n", pBuff_tmp[0]);
					if (sizeof(VIP_PQ_EXT_DCC_DATA) == HAL_VPQ_DATA_EXTERN.length) {
						VIP_PQ_EXT_DCC_DATA *pDCCData = (VIP_PQ_EXT_DCC_DATA *)pBuff_tmp;
						rtd_printk(KERN_INFO, TAG_NAME, "Start set PQ EXT DCC DATA\n");
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_DCC_Curve_Control_Coef, pDCCData->pDCC_Curve_Control_Coef);
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_DCC_Boundary_check_Table, pDCCData->pDCC_Boundary_check_Table);
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_DCC_Level_and_Blend_Coef_Table, pDCCData->pDCC_Level_and_Blend_Coef_Table);
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_DCCHist_Factor_Table, pDCCData->pDCCHist_Factor_Table);
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_DCC_AdaptCtrl_Level_Table, pDCCData->pDCC_AdaptCtrl_Level_Table);
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_USER_DEFINE_CURVE_DCC_CRTL_Table, pDCCData->pUSER_DEFINE_CURVE_DCC_CRTL_Table);
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_Database_Curve_CRTL_Table, pDCCData->pDatabase_Curve_CRTL_Table);
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_Color_Independent_Blending_Table, pDCCData->pColor_Independent_Blending_Table);
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_DCC_Chroma_Compensation_Table, pDCCData->pDCC_Chroma_Compensation_Table);
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_Local_Contrast_Table, pDCCData->pLocal_Contrast_Table);

						Scaler_SetDCC_Table(Scaler_GetDCC_Table());
						Scaler_Set_DCC_Color_Independent_Table(Scaler_Get_DCC_Color_Independent_Table());
						Scaler_Set_DCC_chroma_compensation_Table(Scaler_Get_DCC_chroma_compensation_Table());
						Scaler_SetLocalContrastTable(Scaler_GetLocalContrastTable());
						save_PQ_Extend_Data_Index[PQ_EXTEND_DATA_ENUM_DCC] = pDCCData->index;
					}
					ret = 0;
				}
				dvr_free(pBuff_tmp);
			}
			break;
		}

		case VPQ_EXTERN_IOC_SET_Black_Adjust_byAP:
		{
			if (!g_bEnable_PQ_extend_data) return 0;

			if (copy_from_user(&HAL_VPQ_DATA_EXTERN, (void __user *)arg, sizeof(HAL_VPQ_DATA_EXTERN_T))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_Black_Adjust_byAP, PQModeInfo struct copy fail\n");
				ret = -1;
				break;
			}

			if (HAL_VPQ_DATA_EXTERN.length > 0x400000) { //Max 4MB
				ret = -1;
				break;
			}

			pBuff_tmp = (unsigned char *)dvr_malloc_specific(HAL_VPQ_DATA_EXTERN.length * sizeof(char), GFP_DCU2_FIRST);
			if (pBuff_tmp == NULL) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_Black_Adjust_byAP dynamic alloc memory fail!!!\n");
				ret = -1;
			} else {
				unsigned long useraddr = HAL_VPQ_DATA_EXTERN.pData;
				if(copy_from_user(pBuff_tmp, (void __user *)useraddr, HAL_VPQ_DATA_EXTERN.length * sizeof(char)))
				{
					rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_Black_Adjust_byAP, table copy fail\n");
					ret = -1;
				}
				else {
					rtd_printk(KERN_INFO, TAG_NAME, "VPQ_EXTERN_IOC_SET_Black_Adjust_byAP, pBuff_tmp[0]=%d\n", pBuff_tmp[0]);
					if (sizeof(VIP_PQ_EXT_BLACK_ADJUST_DATA) == HAL_VPQ_DATA_EXTERN.length) {
						VIP_PQ_EXT_BLACK_ADJUST_DATA *pBlackAdjustData = (VIP_PQ_EXT_BLACK_ADJUST_DATA *)pBuff_tmp;
						rtd_printk(KERN_INFO, TAG_NAME, "Start set PQ EXT Black Adjust DATA\n");
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_DCC_Black_Adjust_Table, pBlackAdjustData->pDCC_Black_Adjust_Table);

						save_PQ_Extend_Data_Index[PQ_EXTEND_DATA_ENUM_BLACK_ADJUST] = pBlackAdjustData->index;
					}
					ret = 0;
				}
				dvr_free(pBuff_tmp);
			}
			break;
		}

		case VPQ_EXTERN_IOC_SET_Sharpness_byAP:
		{
			if (!g_bEnable_PQ_extend_data) return 0;

			if (copy_from_user(&HAL_VPQ_DATA_EXTERN, (void __user *)arg, sizeof(HAL_VPQ_DATA_EXTERN_T))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_Sharpness_byAP, PQModeInfo struct copy fail\n");
				ret = -1;
				break;
			}

			if (HAL_VPQ_DATA_EXTERN.length > 0x400000) { //Max 4MB
				ret = -1;
				break;
			}

			pBuff_tmp = (unsigned char *)dvr_malloc_specific(HAL_VPQ_DATA_EXTERN.length * sizeof(char), GFP_DCU2_FIRST);
			if (pBuff_tmp == NULL) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_Sharpness_byAP dynamic alloc memory fail!!!\n");
				ret = -1;
			} else {
				unsigned long useraddr = HAL_VPQ_DATA_EXTERN.pData;
				if(copy_from_user(pBuff_tmp, (void __user *)useraddr, HAL_VPQ_DATA_EXTERN.length * sizeof(char)))
				{
					rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_Sharpness_byAP, table copy fail\n");
					ret = -1;
				}
				else {
					rtd_printk(KERN_INFO, TAG_NAME, "VPQ_EXTERN_IOC_SET_Sharpness_byAP, pBuff_tmp[0]=%d\n", pBuff_tmp[0]);
					if (sizeof(VIP_PQ_EXT_SHARP_DATA) == HAL_VPQ_DATA_EXTERN.length) {
						VIP_PQ_EXT_SHARP_DATA *pSharpData = (VIP_PQ_EXT_SHARP_DATA *)pBuff_tmp;
						rtd_printk(KERN_INFO, TAG_NAME, "Start set PQ EXT Sharpness DATA\n");
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_Ddomain_SHPTable, pSharpData->pDdomain_SHPTable);
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_D_DLTI_Table, pSharpData->pD_DLTI_Table);
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_VipNewIDcti_Table, pSharpData->pVipNewIDcti_Table);
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_VipNewDDcti_Table, pSharpData->pVipNewDDcti_Table);
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_CDS_ini, pSharpData->pCDS_ini);
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_Idomain_MBPKTable, pSharpData->pIdomain_MBPKTable);
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_Ddomain_MBSUTable, pSharpData->pDdomain_MBSUTable);

						Scaler_SetSharpnessTable(Scaler_GetSharpnessTable());
						Scaler_SetDLti(Scaler_GetDLti());
						Scaler_SetIDCti(Scaler_GetIDCti());
						Scaler_SetDCti(Scaler_GetDCti());
						Scaler_SetCDSTable(Scaler_GetCDSTable());
						Scaler_SetMBPeaking(Scaler_GetMBPeaking());
						Scaler_SetMBSUPeaking(Scaler_GetMBSUPeaking());
						Scaler_SetSharpness(Scaler_GetSharpness());
						save_PQ_Extend_Data_Index[PQ_EXTEND_DATA_ENUM_SHARP] = pSharpData->index;
					}
					ret = 0;
				}
				dvr_free(pBuff_tmp);
			}
			break;
		}

		case VPQ_EXTERN_IOC_SET_CSC_byAP:
		{
			if (!g_bEnable_PQ_extend_data) return 0;

			if (copy_from_user(&HAL_VPQ_DATA_EXTERN, (void __user *)arg, sizeof(HAL_VPQ_DATA_EXTERN_T))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_CSC_byAP, PQModeInfo struct copy fail\n");
				ret = -1;
				break;
			}

			if (HAL_VPQ_DATA_EXTERN.length > 0x400000) { //Max 4MB
				ret = -1;
				break;
			}

			pBuff_tmp = (unsigned char *)dvr_malloc_specific(HAL_VPQ_DATA_EXTERN.length * sizeof(char), GFP_DCU2_FIRST);
			if (pBuff_tmp == NULL) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_CSC_byAP dynamic alloc memory fail!!!\n");
				ret = -1;
			} else {
				unsigned long useraddr = HAL_VPQ_DATA_EXTERN.pData;
				if(copy_from_user(pBuff_tmp, (void __user *)useraddr, HAL_VPQ_DATA_EXTERN.length * sizeof(char)))
				{
					rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_CSC_byAP, table copy fail\n");
					ret = -1;
				}
				else {
					rtd_printk(KERN_INFO, TAG_NAME, "VPQ_EXTERN_IOC_SET_CSC_byAP, pBuff_tmp[0]=%d\n", pBuff_tmp[0]);
					if (sizeof(VIP_PQ_EXT_CSC_DATA) == HAL_VPQ_DATA_EXTERN.length) {
						extern short ColorMap_Matrix_Apply[3][3];
						VIP_PQ_EXT_CSC_DATA *pCSCData = (VIP_PQ_EXT_CSC_DATA *)pBuff_tmp;
						rtd_printk(KERN_INFO, TAG_NAME, "Start set PQ EXT CSC DATA\n");
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_CSC_Mapping, pCSCData->pCSC_Mapping);

						fwif_color_ColorMap_SetMatrix(SLR_MAIN_DISPLAY, ColorMap_Matrix_Apply, 1, 1, 0);
						save_PQ_Extend_Data_Index[PQ_EXTEND_DATA_ENUM_CSC] = pCSCData ->index;
					}
					ret = 0;
				}
				dvr_free(pBuff_tmp);
			}
			break;
		}

		case VPQ_EXTERN_IOC_SET_NR_byAP:
		{
			if (!g_bEnable_PQ_extend_data) return 0;

			if (copy_from_user(&HAL_VPQ_DATA_EXTERN, (void __user *)arg, sizeof(HAL_VPQ_DATA_EXTERN_T))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_NR_byAP, PQModeInfo struct copy fail\n");
				ret = -1;
				break;
			}

			if (HAL_VPQ_DATA_EXTERN.length > 0x400000) { //Max 4MB
				ret = -1;
				break;
			}

			pBuff_tmp = (unsigned char *)dvr_malloc_specific(HAL_VPQ_DATA_EXTERN.length * sizeof(char), GFP_DCU2_FIRST);
			if (pBuff_tmp == NULL) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_NR_byAP dynamic alloc memory fail!!!\n");
				ret = -1;
			} else {
				unsigned long useraddr = HAL_VPQ_DATA_EXTERN.pData;
				if(copy_from_user(pBuff_tmp, (void __user *)useraddr, HAL_VPQ_DATA_EXTERN.length * sizeof(char)))
				{
					rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_NR_byAP, table copy fail\n");
					ret = -1;
				}
				else {
					rtd_printk(KERN_INFO, TAG_NAME, "VPQ_EXTERN_IOC_SET_NR_byAP, pBuff_tmp[0]=%d\n", pBuff_tmp[0]);
					if (sizeof(VIP_PQ_EXT_NR_DATA) == HAL_VPQ_DATA_EXTERN.length) {
						VIP_PQ_EXT_NR_DATA *pNRData = (VIP_PQ_EXT_NR_DATA *)pBuff_tmp;
						rtd_printk(KERN_INFO, TAG_NAME, "Start set PQ EXT NR DATA\n");
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_Manual_NR_Table, pNRData->pManual_NR_Table);
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_PQA_Table_Write_Mode, pNRData->pPQA_Table_Write_Mode);
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_PQA_Input_Table, pNRData->pPQA_Input_Table);
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_MA_SNR_IESM_Table, pNRData->pMA_SNR_IESM_Table);
						Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_I_De_Contour_Table, pNRData->pI_De_Contour_Table);

						scaler_set_PQA_table(scaler_get_PQA_table());
						scaler_set_PQA_Input_table(scaler_get_PQA_Input_table());
						Scaler_SetDNR(Scaler_GetDNR());
						Scaler_Set_I_De_Contour(Scaler_Get_I_De_Contour());
						save_PQ_Extend_Data_Index[PQ_EXTEND_DATA_ENUM_NR] = pNRData ->index;
					}
					ret = 0;
				}
				dvr_free(pBuff_tmp);
			}
			break;
		}

		case VPQ_EXTERN_IOC_SET_WBBL_byAP:
		{
			if (!g_bEnable_PQ_extend_data) return 0;

			if (copy_from_user(&HAL_VPQ_DATA_EXTERN, (void __user *)arg, sizeof(HAL_VPQ_DATA_EXTERN_T))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_WBBL_byAP, PQModeInfo struct copy fail\n");
				ret = -1;
				break;
			}

			if (HAL_VPQ_DATA_EXTERN.length > 0x400000) { //Max 4MB
				ret = -1;
				break;
			}

			pBuff_tmp = (unsigned char *)dvr_malloc_specific(HAL_VPQ_DATA_EXTERN.length * sizeof(char), GFP_DCU2_FIRST);
			if (pBuff_tmp == NULL) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_WBBL_byAP dynamic alloc memory fail!!!\n");
				ret = -1;
			} else {
				unsigned long useraddr = HAL_VPQ_DATA_EXTERN.pData;
				if(copy_from_user(pBuff_tmp, (void __user *)useraddr, HAL_VPQ_DATA_EXTERN.length * sizeof(char)))
				{
					rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_WBBL_byAP, table copy fail\n");
					ret = -1;
				}
				else {
					rtd_printk(KERN_INFO, TAG_NAME, "VPQ_EXTERN_IOC_SET_WBBL_byAP, pBuff_tmp[0]=%d\n", pBuff_tmp[0]);
					if (sizeof(VIP_PQ_EXT_WBBL_DATA) == HAL_VPQ_DATA_EXTERN.length) {
						VIP_PQ_EXT_WBBL_DATA *pWBBLData = (VIP_PQ_EXT_WBBL_DATA *)pBuff_tmp;
						rtd_printk(KERN_INFO, TAG_NAME, "Start set PQ EXT WBBL DATA\n");
						save_PQ_Extend_Data_Index[PQ_EXTEND_DATA_ENUM_WBBL] = pWBBLData->index;
						if(Scaler_access_Project_TV002_Style(0, 0) == Project_TV002_Style_1)
						{
							Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_WBBL_Table, pWBBLData->pWBBL_Table);
							fwif_WBBL_TV002();
							g_WBBL_WBBS_enable = true;
						}
					}
					ret = 0;
				}
				dvr_free(pBuff_tmp);
			}
			break;
		}

		case VPQ_EXTERN_IOC_SET_WB_BS_byAP:
		{
			if (!g_bEnable_PQ_extend_data) return 0;

			if (copy_from_user(&HAL_VPQ_DATA_EXTERN, (void __user *)arg, sizeof(HAL_VPQ_DATA_EXTERN_T))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_WB_BS_byAP, PQModeInfo struct copy fail\n");
				ret = -1;
				break;
			}

			if (HAL_VPQ_DATA_EXTERN.length > 0x400000) { //Max 4MB
				ret = -1;
				break;
			}

			pBuff_tmp = (unsigned char *)dvr_malloc_specific(HAL_VPQ_DATA_EXTERN.length * sizeof(char), GFP_DCU2_FIRST);
			if (pBuff_tmp == NULL) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_WB_BS_byAP dynamic alloc memory fail!!!\n");
				ret = -1;
			} else {
				unsigned long useraddr = HAL_VPQ_DATA_EXTERN.pData;
				if(copy_from_user(pBuff_tmp, (void __user *)useraddr, HAL_VPQ_DATA_EXTERN.length * sizeof(char)))
				{
					rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_WB_BS_byAP, table copy fail\n");
					ret = -1;
				}
				else {
					rtd_printk(KERN_INFO, TAG_NAME, "VPQ_EXTERN_IOC_SET_WB_BS_byAP, pBuff_tmp[0]=%d\n", pBuff_tmp[0]);
					if (sizeof(VIP_PQ_EXT_WB_BS_DATA) == HAL_VPQ_DATA_EXTERN.length) {
						VIP_PQ_EXT_WB_BS_DATA *pWBBSData = (VIP_PQ_EXT_WB_BS_DATA *)pBuff_tmp;
						rtd_printk(KERN_INFO, TAG_NAME, "Start set PQ EXT WB BS DATA\n");
						save_PQ_Extend_Data_Index[PQ_EXTEND_DATA_ENUM_WB_BS] = pWBBSData->index;
						if(Scaler_access_Project_TV002_Style(0, 0) == Project_TV002_Style_1)
						{
							Scaler_color_set_AP_PQ_extend_data(PQ_EXT_TYPE_WB_BS_Table, pWBBSData->pWB_BS_Table);
							fwif_Blue_Stretch_TV002();
							g_WBBL_WBBS_enable = true;
						}
					}
					ret = 0;
				}
				dvr_free(pBuff_tmp);
			}
			break;
		}

		case VPQ_EXTERN_IOC_TV002_BL_CTRL:
		{
			if (copy_from_user(&HAL_VPQ_DATA_EXTERN, (void __user *)arg, sizeof(HAL_VPQ_DATA_EXTERN_T))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_TV002_BL_CTRL, PQModeInfo struct copy fail\n");
				ret = -1;
				break;
			}

			if (HAL_VPQ_DATA_EXTERN.length > 0x400000) { //Max 4MB
				ret = -1;
				break;
			}

			pBuff_tmp = (unsigned char *)kmalloc(HAL_VPQ_DATA_EXTERN.length * sizeof(char), GFP_KERNEL);
			if (pBuff_tmp == NULL) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_TV002_BL_CTRL dynamic alloc memory fail!!!\n");
				ret = -1;
			} else {
				unsigned long useraddr = HAL_VPQ_DATA_EXTERN.pData;
				if(copy_from_user(pBuff_tmp, (void __user *)useraddr, HAL_VPQ_DATA_EXTERN.length * sizeof(char)))
				{
					rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_TV002_BL_CTRL, table copy fail\n");
					ret = -1;
				}
				else {
					if (Scaler_Update_BL_DBC_Param_TV002(HAL_VPQ_DATA_EXTERN.cmd, HAL_VPQ_DATA_EXTERN.length, pBuff_tmp) == 0)
						Scaler_DBC_Set_To_Device_TV002(1);
					ret = 0;
				}
				kfree(pBuff_tmp);
			}
			break;
		}

		case VPQ_EXTERN_IOC_TV002_SET_SOLIB_GENERAL:
		{
			if (copy_from_user(&HAL_VPQ_DATA_EXTERN, (void __user *)arg, sizeof(HAL_VPQ_DATA_EXTERN_T))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_TV002_SET_SOLIB_GENERAL, PQModeInfo struct copy fail\n");
				ret = -1;
				break;
			}

			if (HAL_VPQ_DATA_EXTERN.length > 0x400000) { //Max 4MB
				ret = -1;
				break;
			}

			pBuff_tmp = (unsigned char *)dvr_malloc_specific(HAL_VPQ_DATA_EXTERN.length * sizeof(char), GFP_DCU2_FIRST);
			if (pBuff_tmp == NULL) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_TV002_SET_SOLIB_GENERAL dynamic alloc memory fail!!!\n");
				ret = -1;
			} else {
				unsigned long useraddr = HAL_VPQ_DATA_EXTERN.pData;
				if(copy_from_user(pBuff_tmp, (void __user *)useraddr, HAL_VPQ_DATA_EXTERN.length * sizeof(char)))
				{
					rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_TV002_SET_SOLIB_GENERAL, data copy fail\n");
					ret = -1;
				}
				else {
			#ifdef CONFIG_CUSTOMER_TV002
					extern RTK_TV002_CALLBACK_SetGeneralFunc cb_RTK_TV002_PQLib_SetGeneralFunc;
					down(&TV002_PQLib_GeneralFunc_Semaphore);
					if (cb_RTK_TV002_PQLib_SetGeneralFunc != NULL && cb_RTK_TV002_PQLib_SetGeneralFunc(HAL_VPQ_DATA_EXTERN.cmd, pBuff_tmp, HAL_VPQ_DATA_EXTERN.length) != 0)
						rtd_printk(KERN_ERR, TAG_NAME, "SonyPQLib_SetGeneralFunc fail\n");
					up(&TV002_PQLib_GeneralFunc_Semaphore);
			#endif
					ret = 0;
				}
				dvr_free(pBuff_tmp);
			}
			break;
		}

		case VPQ_EXTERN_IOC_TV002_GET_SOLIB_GENERAL:
		{
			if (copy_from_user(&HAL_VPQ_DATA_EXTERN, (void __user *)arg, sizeof(HAL_VPQ_DATA_EXTERN_T))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_TV002_GET_SOLIB_GENERAL, PQModeInfo struct copy fail\n");
				ret = -1;
				break;
			}

			if (HAL_VPQ_DATA_EXTERN.length > 0x400000) { //Max 4MB
				ret = -1;
				break;
			}

			pBuff_tmp = (unsigned char *)dvr_malloc_specific(HAL_VPQ_DATA_EXTERN.length * sizeof(char), GFP_DCU2_FIRST);
			if (pBuff_tmp == NULL) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_TV002_GET_SOLIB_GENERAL dynamic alloc memory fail!!!\n");
				ret = -1;
			} else {
		#ifdef CONFIG_CUSTOMER_TV002
				extern RTK_TV002_CALLBACK_GetGeneralFunc cb_RTK_TV002_PQLib_GetGeneralFunc;
				down(&TV002_PQLib_GeneralFunc_Semaphore);
				if (cb_RTK_TV002_PQLib_GetGeneralFunc != NULL && cb_RTK_TV002_PQLib_GetGeneralFunc(HAL_VPQ_DATA_EXTERN.cmd, pBuff_tmp, HAL_VPQ_DATA_EXTERN.length) == 0) {
					unsigned long useraddr = HAL_VPQ_DATA_EXTERN.pData;
					if(copy_to_user((void __user *)useraddr, pBuff_tmp, HAL_VPQ_DATA_EXTERN.length * sizeof(char)))
					{
						rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_TV002_GET_SOLIB_GENERAL, data copy fail\n");
						ret = -1;
					}
					else {
						ret = 0;
					}
				} else
					rtd_printk(KERN_ERR, TAG_NAME, "SonyPQLib_GetGeneralFunc fail\n");
				up(&TV002_PQLib_GeneralFunc_Semaphore);
		#endif
				dvr_free(pBuff_tmp);
			}
			break;
		}

		case VPQ_EXTERN_IOC_TV002_SET_SOLIB_SHARE_MEM:
		{
	#ifdef CONFIG_CUSTOMER_TV002
			if (copy_from_user(&HAL_VPQ_DATA_EXTERN, (void __user *)arg, sizeof(HAL_VPQ_DATA_EXTERN_T))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_TV002_SET_SOLIB_SHARE_MEM, PQModeInfo struct copy fail\n");
				ret = -1;
				break;
			}

			if (HAL_VPQ_DATA_EXTERN.length > 0x400000) { //Max 4MB
				ret = -1;
				break;
			}

			pBuff_tmp = Scaler_Get_SoPQLib_ShareMem();//(unsigned char *)dvr_malloc_specific(HAL_VPQ_DATA_EXTERN.length * sizeof(char), GFP_DCU2_FIRST);
			if (pBuff_tmp == NULL) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_TV002_SET_SOLIB_SHARE_MEM get memory addr fail!!!\n");
				ret = -1;
			} else {
				unsigned long useraddr = HAL_VPQ_DATA_EXTERN.pData;

				if (HAL_VPQ_DATA_EXTERN.length > Scaler_Get_SoPQLib_ShareMem_Size()) {
					rtd_printk(KERN_ERR, TAG_NAME, "VPQ_EXTERN_IOC_TV002_SET_SOLIB_SHARE_MEM, size over\n");
					HAL_VPQ_DATA_EXTERN.length = Scaler_Get_SoPQLib_ShareMem_Size();
				}

				if(copy_from_user(pBuff_tmp, (void __user *)useraddr, HAL_VPQ_DATA_EXTERN.length * sizeof(char)))
				{
					rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_TV002_SET_SOLIB_SHARE_MEM, data copy fail\n");
					ret = -1;
				}
				else {
					/*
			#ifdef CONFIG_CUSTOMER_TV002
					static bool set_sopqlib_sharemem_flag = false;

					if (!set_sopqlib_sharemem_flag && cb_RTK_TV002_PQLib_SetShareMemoryArea != NULL) {
						cb_RTK_TV002_PQLib_SetShareMemoryArea(Scaler_Get_SoPQLib_ShareMem(), Scaler_Get_SoPQLib_ShareMem_Size());
						set_sopqlib_sharemem_flag = true;
					}
			#endif
					*/

					ret = 0;
				}
				//dvr_free(pBuff_tmp);
			}
	#endif
			break;
		}

		case VPQ_EXTERN_IOC_TV002_GET_SOLIB_SHARE_MEM:
		{
	#ifdef CONFIG_CUSTOMER_TV002
			if (copy_from_user(&HAL_VPQ_DATA_EXTERN, (void __user *)arg, sizeof(HAL_VPQ_DATA_EXTERN_T))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_TV002_GET_SOLIB_SHARE_MEM, PQModeInfo struct copy fail\n");
				ret = -1;
				break;
			}

			if (HAL_VPQ_DATA_EXTERN.length > 0x400000) { //Max 4MB
				ret = -1;
				break;
			}

			pBuff_tmp = Scaler_Get_SoPQLib_ShareMem();//(unsigned char *)dvr_malloc_specific(HAL_VPQ_DATA_EXTERN.length * sizeof(char), GFP_DCU2_FIRST);
			if (pBuff_tmp == NULL) {
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_TV002_GET_SOLIB_SHARE_MEM get memory addr fail!!!\n");
				ret = -1;
			} else {
				unsigned long useraddr = HAL_VPQ_DATA_EXTERN.pData;

				if (HAL_VPQ_DATA_EXTERN.length > Scaler_Get_SoPQLib_ShareMem_Size()) {
					rtd_printk(KERN_ERR, TAG_NAME, "VPQ_EXTERN_IOC_TV002_GET_SOLIB_SHARE_MEM, size over\n");
					HAL_VPQ_DATA_EXTERN.length = Scaler_Get_SoPQLib_ShareMem_Size();
				}

				if(copy_to_user((void __user *)useraddr, pBuff_tmp, HAL_VPQ_DATA_EXTERN.length * sizeof(char)))
				{
					rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_TV002_GET_SOLIB_SHARE_MEM, data copy fail\n");
					ret = -1;
				}
				else {
					ret = 0;
				}

				//dvr_free(pBuff_tmp);
			}
	#endif
			break;
		}
		case VPQ_EXTERN_IOC_GET_ACCESS_DATA:
		{
			if (copy_from_user(&HAL_VPQ_DATA_EXTERN, (void __user *)arg, sizeof(HAL_VPQ_DATA_EXTERN_T))) {
				rtd_printk(KERN_ERR, TAG_NAME, "VPQ_EXTERN_IOC_GET_ACCESS_DATA, PQModeInfo struct copy fail\n");
				ret = -1;
				break;
			}

			if (HAL_VPQ_DATA_EXTERN.length > 0x400000) { //Max 4MB
				ret = -1;
				break;
			}

			if (HAL_VPQ_DATA_EXTERN.length != sizeof(ACCESS_DATA_PT_T)) {
				rtd_printk(KERN_ERR, TAG_NAME, "VPQ_EXTERN_IOC_GET_ACCESS_DATA length error!!!\n");
				ret = -1;
				break;
			}
			pBuff_tmp = (unsigned char *)dvr_malloc_specific(HAL_VPQ_DATA_EXTERN.length * sizeof(char), GFP_DCU2_FIRST);
			if (pBuff_tmp == NULL) {
				rtd_printk(KERN_ERR, TAG_NAME, "VPQ_EXTERN_IOC_GET_ACCESS_DATA malloc fail!!!\n");
				ret = -1;
			} else {
				unsigned long useraddr = HAL_VPQ_DATA_EXTERN.pData;
				if (copy_from_user(pBuff_tmp, (void __user *)useraddr, HAL_VPQ_DATA_EXTERN.length * sizeof(char))) {
					rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_GET_DATA_ACCESS, struct copy fail\n");
					ret = -1;
				} else {
					drvif_color_get_access_data_point((ACCESS_DATA_PT_T *)pBuff_tmp);

					if(copy_to_user((void __user *)useraddr, pBuff_tmp, HAL_VPQ_DATA_EXTERN.length * sizeof(char))) {
						rtd_printk(KERN_ERR, TAG_NAME, "VPQ_EXTERN_IOC_GET_DATA_ACCESS, data copy fail\n");
						ret = -1;
					}
					else {
						ret = 0;
					}
					dvr_free(pBuff_tmp);
				}
			}
			break;
		}

		case VPQ_EXTERN_IOC_GET_PQ_ENGMENU:
		{
			printk("VPQ_EXTERN_IOC_GET_PQ_Engineer_menu\n");
			ENGMENU = Scaler_Get_ENGMENU();
			if (copy_to_user((void __user *)arg, (void *)ENGMENU, sizeof(HAL_VPQ_ENG_STRUCT))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_PQ_Engineer_menu fail\n");
				ret = -1;
			} else
				ret = 0;

		}
		break;

		case VPQ_EXTERN_IOC_GET_PQ_ENGMENU_size:
		{
			printk("VPQ_EXTERN_IOC_GET_PQ_Engineer_menu_size\n");
			ENGMENU_size = Scaler_Get_ENGMENU_size();
			if (copy_to_user((void __user *)arg, (void *)ENGMENU_size, sizeof(HAL_VPQ_ENG_SIZE))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_PQ_Engineer_menu_size fail\n");
				ret = -1;
			} else
				ret = 0;

		}
		break;

		case VPQ_EXTERN_IOC_GET_PQ_ENGMENU_item:
		{
			printk("VPQ_EXTERN_IOC_GET_PQ_Engineer_menu\n");
			ENGMENU = Scaler_Get_ENGMENU();
			if (copy_to_user((void __user *)arg, (ENGMENU->item), sizeof(HAL_VPQ_ENG_ITEM_STRUCT)*HAL_VPQ_ENG_ITEM_MAX_NUM)) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_PQ_Engineer_menu fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;

		case VPQ_EXTERN_IOC_GET_PQ_ENGMENU_ID:
		{
			printk("VPQ_EXTERN_IOC_GET_PQ_Engineer_menu\n");
			ENGMENU = Scaler_Get_ENGMENU();
			if (copy_to_user((void __user *)arg, (ENGMENU->ID), sizeof(HAL_VPQ_ENG_ID_STRUCT)*HAL_VPQ_ENG_ID_MAX_NUM)) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_PQ_Engineer_menu fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;

		case VPQ_EXTERN_IOC_GET_PQ_ENG_ID:
		{
			if (copy_from_user(&ENG_ID,  (int __user *)arg,sizeof(HAL_VPQ_ENG_ID))) {
				ret = -1;
			} else {
				ret = 0;
				ENG_ID.ID_value= Scaler_Get_ENGMENU_ID((unsigned int)ENG_ID.ID);
			}
			printk("VPQ_EXTERN_IOC_GET_PQ_ENG_ID\n");
			if (copy_to_user((void __user *)arg, (void *)(&ENG_ID), sizeof(HAL_VPQ_ENG_ID))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_PQ_ENG_ID fail\n");
				ret = -1;
			} else
				ret = 0;

		}
		break;

		case VPQ_EXTERN_IOC_SET_PQ_ENG_ID:
		{
			if (copy_from_user(&ENG_ID,  (int __user *)arg,sizeof(HAL_VPQ_ENG_ID))) {
				ret = -1;
			} else {
				ret = 0;
				Scaler_Set_ENGMENU_ID(ENG_ID.ID, ENG_ID.ID_value);
			}
		}
		break;

		case VPQ_EXTERN_IOC_GET_PQ_ENG_TWOLAYER_MENU_item:
		{
			printk("VPQ_EXTERN_IOC_GET_PQ_Engineer_TWOLAYER_menu\n");
			ENG_TWOLAYER_MENU = Scaler_Get_ENG_TWOLAYER_MENU();
			if (copy_to_user((void __user *)arg, (ENG_TWOLAYER_MENU->item), sizeof(HAL_VPQ_ENG_ITEM_TWOLAYER_STRUCT)*HAL_VPQ_ENG_ITEM_MAX_NUM)) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_PQ_Engineer_menu fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;

		case VPQ_EXTERN_IOC_GET_PQ_ENG_TWOLAYER_MENU_ID:
		{
			printk("VPQ_EXTERN_IOC_GET_PQ_Engineer_TWOLAYER_menu\n");
			ENG_TWOLAYER_MENU = Scaler_Get_ENG_TWOLAYER_MENU();
			if (copy_to_user((void __user *)arg, (ENG_TWOLAYER_MENU->ID), sizeof(HAL_VPQ_ENG_ID_STRUCT)*HAL_VPQ_ENG_ID_MAX_NUM)) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_PQ_Engineer_TWOLAYER_menu fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;

		case VPQ_EXTERN_IOC_GET_PQ_ENG_TWOLAYER_ID:
		{
			if (copy_from_user(&ENG_TWOLAYER_ID,  (int __user *)arg,sizeof(HAL_VPQ_ENG_TWOLAYER_ID))) {
				ret = -1;
			} else {
				ret = 0;
				ENG_TWOLAYER_ID.ID_value= Scaler_Get_ENG_TWOLAYER_MENU_ID((unsigned int)ENG_TWOLAYER_ID.ITEM,(unsigned int)ENG_TWOLAYER_ID.ID);
			}
			printk("VPQ_EXTERN_IOC_GET_PQ_TWOLAYER_ENG_ID\n");
			if (copy_to_user((void __user *)arg, (void *)(&ENG_TWOLAYER_ID), sizeof(HAL_VPQ_ENG_TWOLAYER_ID))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_PQ_ENG_TWOLAYER_ID fail\n");
				ret = -1;
			} else
				ret = 0;

		}
		break;

		case VPQ_EXTERN_IOC_SET_PQ_ENG_TWOLAYER_ID:
		{
			if (copy_from_user(&ENG_TWOLAYER_ID,  (int __user *)arg,sizeof(HAL_VPQ_ENG_TWOLAYER_ID))) {
				ret = -1;
			} else {
				ret = 0;
				Scaler_Set_ENG_TWOLAYER_MENU_ID(ENG_TWOLAYER_ID.ITEM,ENG_TWOLAYER_ID.ID, ENG_TWOLAYER_ID.ID_value);
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_TV002_EXT_FUN_EN:
		{
			if (copy_from_user(&iEn,  (void __user *)arg,sizeof(unsigned int))) {
				ret = -1;
			} else {
				g_tv002_demo_fun_flag = iEn&0xff;
				scalerVIP_access_tv002_style_demo_flag(1, &g_tv002_demo_fun_flag);
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_TV002_BLE:
		{
			if (copy_from_user(&level,  (void __user *)arg,sizeof(int))) {
				ret = -1;
			} else {
				Scaler_set_BLE(level&0xff);
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_TV002_SGAMMA:
		{
			if (copy_from_user(&level,  (void __user *)arg,sizeof(int))) {
				ret = -1;
			} else {
				Scaler_set_sGamma(level&0xff);
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_TV002_MAGIC_GAMMA:
		{

			if (copy_from_user(&magic_gamma_data,  (void __user *)arg,sizeof(VIP_MAGIC_GAMMA_Curve_Driver_Data))) {
				ret = -1;
			} else {
				memcpy(&g_MagicGammaDriverDataSave, &magic_gamma_data, sizeof(VIP_MAGIC_GAMMA_Curve_Driver_Data));
				fwif_color_set_gamma_from_MagicGamma_MiddleWare();
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_GET_TV002_SR:
		{
			sr_data.mode = Scaler_get_Clarity_RealityCreation();
			sr_data.value = Scaler_get_Clarity_Resolution();
			if (copy_to_user((void __user *)arg, (void *)&sr_data, sizeof(RTK_TV002_SR_T))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_TV002_SR fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;

		case VPQ_EXTERN_IOC_SET_TV002_SR:
		{
			if (copy_from_user(&sr_data,  (void __user *)arg,sizeof(RTK_TV002_SR_T))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_TV002_SR fail\n");
				ret = -1;
			} else {
				Scaler_set_Clarity_RealityCreation(sr_data.mode);
				Scaler_set_Clarity_Resolution(sr_data.value);
				//fwif_color_set_clarity_resolution();
				Scaler_Set_Clarity_Resolution_TV002();
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_GET_TV002_LIVE_COLOUR:
		{
			live_colour = Scaler_get_LiveColor();
			if (copy_to_user((void __user *)arg, (void *)&live_colour, sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_TV002_LIVE_COLOUR fail\n");
				ret = -1;
			} else {
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_TV002_LIVE_COLOUR:
		{
			if (copy_from_user(&live_colour,  (void __user *)arg,sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_TV002_LIVE_COLOUR fail\n");
				ret = -1;
			} else {
				Scaler_set_LiveColor(live_colour);
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_GET_TV002_MASTERED_4K:
		{
			mastered_4k = Scaler_get_Clarity_Mastered4K();
			if (copy_to_user((void __user *)arg, (void *)&mastered_4k, sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_TV002_MASTERED_4K fail\n");
				ret = -1;
			} else {
				ret = 0;
			}
		}
		break;
#if 0
		case VPQ_EXTERN_IOC_SET_TV002_Intelligent_Pic_Enable:
		{
			if (copy_from_user(&level,  (void __user *)arg,sizeof(int))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_TV002_Intelligent_Pic_Enable fail\n");
				ret = -1;
			} else {
				rtd_printk(KERN_INFO, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_TV002_Intelligent_Pic_Enable level=%d,\n", level);
				Scaler_set_Intelligent_Picture_Enable((unsigned char)(level&0xff));
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_TV002_Intelligent_Pic_Optimisation:
		{
			if (copy_from_user(&level,  (void __user *)arg,sizeof(int))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_TV002_Intelligent_Pic_Optimisation fail\n");
				ret = -1;
			} else {
				rtd_printk(KERN_INFO, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_TV002_Intelligent_Pic_Optimisation level=%d,\n", level);
				Scaler_set_Intelligent_Picture_Optimisation((unsigned char)(level&0xff));
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_TV002_Intelligent_Pic_Signal_Lv_Indicator_ONOFF:
		{
			if (copy_from_user(&level,  (void __user *)arg,sizeof(int))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_TV002_Intelligent_Pic_Signal_Lv_Indicator_ONOFF fail\n");
				ret = -1;
			} else {
				rtd_printk(KERN_INFO, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_TV002_Intelligent_Pic_Signal_Lv_Indicator_ONOFF level=%d,\n", level);
				Scaler_set_Intelligent_Picture_Signal_Level_Indicator_ONOFF((unsigned char)(level&0xff));
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_GET_TV002_Intelligent_Pic_Signal_Level:
		{
			//printk("VPQ_EXTERN_IOC_PQ_CMD_GET_GDBC_MODE\n");
			level = (int)Scaler_Intelligent_Picture_get_Intelligent_Picture_Signal_Level();
			if (copy_to_user((void __user *)arg, (void *)&level, sizeof(unsigned int))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_TV002_Intelligent_Pic_Signal_Level fail\n");
				ret = -1;
			} else {
				rtd_printk(KERN_INFO, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_TV002_Intelligent_Pic_Signal_Level level=%d,\n", level);
				ret = 0;
			}
		}
		break;
#endif
		case VPQ_EXTERN_IOC_SET_TV002_MASTERED_4K:
		{
			if (copy_from_user(&mastered_4k, (void __user *)arg, sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_TV002_MASTERED_4K fail\n");
				ret = -1;
			} else {
				Scaler_set_Clarity_Mastered4K(mastered_4k);
				fwif_color_set_mastered_4k();
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_TV002_PROJECT_STYLE:
		{
			if (copy_from_user(&TV002_PROJECT_STYLE, (void __user *)arg, sizeof(unsigned char))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_TV002_PROJECT_STYLE fail\n");
				ret = -1;
			} else {
				Scaler_access_Project_TV002_Style(1, TV002_PROJECT_STYLE);
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_TV002_DBC_POWER_SAVING_MODE:
		{
			if (copy_from_user(&TV002_DBC_POWER_SAVING_MODE, (void __user *)arg, sizeof(unsigned char))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_TV002_DBC_POWER_SAVING_MODE fail\n");
				ret = -1;
			} else {
				Scaler_set_DBC_POWER_Saving_Mode_TV002(TV002_DBC_POWER_SAVING_MODE);
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_TV002_DBC_UI_BACKLIGHT:
		{
			if (copy_from_user(&TV002_DBC_UI_BACKLIGHT, (void __user *)arg, sizeof(unsigned char))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_TV002_DBC_UI_BACKLIGHT fail\n");
				ret = -1;
			} else {
				Scaler_set_DBC_UI_blacklight_Value_TV002(TV002_DBC_UI_BACKLIGHT);
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_GET_TV002_DBC_BACKLIGHT:
		{
			TV002_DBC_BACKLIGHT = Scaler_get_DBC_blacklight_Value_TV002();
			if (copy_to_user((void __user *)arg, (void *)&TV002_DBC_BACKLIGHT, sizeof(unsigned char))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_TV002_DBC_BACKLIGHT fail\n");
				ret = -1;
			} else {
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_TV002_UI_PICTURE_MODE:
		{
			if (copy_from_user(&TV002_UI_PICTURE_MODE, (void __user *)arg, sizeof(unsigned char))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_TV002_UI_PICTURE_MODE fail\n");
				ret = -1;
			} else {
				Scaler_set_UI_Picture_Mode_TV002(TV002_UI_PICTURE_MODE);
				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_GET_TV002_HDR_TYPE:
		{
			_system_setting_info *vip_sys_info = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
			if(vip_sys_info != NULL) {
				if (copy_to_user((void __user *)arg, (void *)&vip_sys_info->HDR_flag, sizeof(unsigned char))) {
					rtd_printk(KERN_EMERG, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_TV002_HDR_TYPE fail\n");
					ret = -1;
				} else {
					rtd_printk(KERN_DEBUG, TAG_NAME, "VPQ_EXTERN_IOC_GET_TV002_HDR_TYPE=%d,\n", vip_sys_info->HDR_flag);
					ret = 0;
				}
			} else {
				rtd_printk(KERN_EMERG, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_TV002_HDR_TYPE, system info = NULL\n");
				ret = -1;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_PQ_SETGAMMA_TABLESIZE:
		{

			if (copy_from_user(&Gamma,  (int __user *)arg,sizeof(RTK_TableSize_Gamma))) {
				ret = -1;
			} else {
				fwif_color_gamma_encode_TableSize(&Gamma);
				fwif_set_gamma(1, 0, 0, 0);

				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_WCG_MODE:
		{
			PQ_WCG_MODE_STATUS wcgmode_tmp;
			printk("[WCG] VPQ_EXTERN_IOC_SET_WCG_MODE\n");

			if (copy_from_user(&wcgmode_tmp, (int __user *)arg, sizeof(unsigned int))) {
				printk("kernel copy VPQ_EXTERN_IOC_SET_WCG_MODE fail\n");
				ret = -1;
				break;
			} else {
				if(wcgmode_tmp==WCGMODE_WCG)
                   printk("WCGMODE_WCG\n");
                else if(wcgmode_tmp==WCGMODE_AUTO)
                    printk("WCGMODE_AUTO\n");
                else if(wcgmode_tmp==WCGMODE_STANDARD)
                    printk("WCGMODE_STANDARD\n");
                else
				    printk("VPQ_EXTERN_IOC_SET_WCG_MODE UNKNOW[%d]\n", wcgmode_tmp);

                Scaler_Set_WCG_mode(wcgmode_tmp);


				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_GET_GDBC_MODE:
		{
			printk("VPQ_EXTERN_IOC_PQ_CMD_GET_GDBC_MODE\n");
			dcr_mode = (unsigned int)Scaler_GetDcrMode();
			if (copy_to_user((void __user *)arg, (void *)&dcr_mode, sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_DCC_MODE fail\n");
				ret = -1;
			} else
				ret = 0;

		}
		break;
		case VPQ_EXTERN_IOC_SET_GDBC_MODE:
		{
			//printk(KERN_NOTICE "VPQ_EXTERN_IOC_PQ_CMD_SET_GDBC_MODE\n");
			if (copy_from_user(&dcr_mode,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetDcrMode(dcr_mode&0xff);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_DCR_BACKLIGHT:
		{
			//printk(KERN_NOTICE "VPQ_EXTERN_IOC_GET_DCR_BACKLIGHT\n");
			DCR_backlight = (unsigned int)Scaler_Get_DCR_Backlight();
			if (copy_to_user((void __user *)arg, (void *)&DCR_backlight, sizeof(unsigned int))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_BACKLIGHT fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;
		case VPQ_EXTERN_IOC_SET_SLR_HDR_TABLE:
		{
			unsigned int hdr_table_size = 0;
			SLR_HDR_TABLE* ptr_HDR_bin = (SLR_HDR_TABLE*)dvr_malloc_specific(sizeof(SLR_HDR_TABLE), GFP_DCU2_FIRST);
			if(ptr_HDR_bin != NULL)
			{
				if (copy_from_user(&hdr_table_size, (void __user *)arg, sizeof(unsigned int))) { //size check
					rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_SLR_HDR_TABLE fail\n");
				} else {
					if (hdr_table_size != sizeof(SLR_HDR_TABLE)) {
						rtd_printk(KERN_ERR, TAG_NAME, "VPQ_EXTERN_IOC_SET_SLR_HDR_TABLE struct size miss match, AP=%d, driver=%zu!!\n",
							hdr_table_size, sizeof(SLR_HDR_TABLE));
						ret = -1;
					} else {
						if (copy_from_user(ptr_HDR_bin, (void __user *)arg, sizeof(SLR_HDR_TABLE))) {
							rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_SLR_HDR_TABLE fail\n");
							ret = -1;
						} else {
							rtd_printk(KERN_INFO, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_SLR_HDR_TABLE success\n");
							Scaler_color_copy_HDR_table_from_AP(ptr_HDR_bin);
							ret = 0;
						}
					}
				}
				dvr_free((void *)ptr_HDR_bin);
			}
			else
			{
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_SLR_HDR_TABLE ptr_HDR_bin = NULL\n");
				ret = -1;
			}
		}
		break;
		case VPQ_EXTERN_IOC_Sync_PQ_table_bin_ENABLE:
		{
			unsigned int args = 0;
			rtd_printk(KERN_INFO, TAG_NAME, "VPQ_EXTERN_IOC_Sync_PQ_table_bin_ENABLE\n");
			if 	(copy_from_user((void *)&args, (const void __user *)arg, sizeof(unsigned char)))
			{
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_Sync_PQ_table_bin_ENABLE fail\n");
				ret = -1;
			} else {
				ret = 0;
				Scaler_Set_PQ_table_bin_En((unsigned char)args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_PANORAMA_TYPE:
		{
			unsigned int args = 0;
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {
				ret = -1;
			} else {
				if(args){
					Check_smooth_toggle_update_flag((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY));
					zoom_update_scaler_info_from_vo_smooth_toggle((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),0);
					drvif_color_ultrazoom_config_scaling_up((TV030_RATIO_TYPE_E)args);
					IoReg_SetBits(SCALEUP_DM_UZU_DB_CTRL_reg, SCALEUP_DM_UZU_DB_CTRL_db_apply_mask);
					Scaler_DispSetRatioMode(SLR_RATIO_PANORAMA);
				}
				else
					Scaler_DispSetRatioMode(SLR_RATIO_CUSTOM);

				imd_smooth_main_double_buffer_apply(ZOOM_UZU);
				ret = 0;
			}
			rtd_printk(KERN_ERR, TAG_NAME, "=======VPQ_EXTERN_IOC_SET_PANORAMA_TYPE ret= %d args=%d \n", ret,args);
		}
		break;
		case VPQ_EXTERN_IOC_SET_SPLIT_DEMO_TV030:
		{
			bool bOnOff;
			//printk("yuan,VPQ_EXTERN_IOC_SET_SPLIT_DEMO_TV030=%d\n",bOnOff);

			if (copy_from_user(&bOnOff, (int __user *)arg, sizeof(bool))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_SPLIT_DEMO_TV030 fail\n");
				ret = -1;
			} else {
				if (bOnOff == TRUE)
					Scaler_SetMagicPicture(SLR_MAGIC_STILLDEMO);
				else
					Scaler_SetMagicPicture(SLR_MAGIC_OFF);
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_COLORSPACE://HSI
		{
			//printk("[Kernel]VPQ_EXTERN_IOC_SET_COLORSPACE\n");
			_system_setting_info *VIP_system_info_structure_table = NULL;
			static unsigned char pre_colorspaceisNative=0;
			RTK_VPQ_COLORSPACE_INDEX ColoSpace_T;
			extern RTK_VPQ_COLORSPACE_INDEX ColoSpace_Index;
			extern unsigned char g_srgbForceUpdate;

			VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

			if (copy_from_user(&ColoSpace_T,  (int __user *)arg,sizeof(RTK_VPQ_COLORSPACE_INDEX))) {
				ret = -1;
			} else {

				ColorSpaceMode = ColoSpace_T.mode;
				if(pre_colorspaceisNative==1)
					g_srgbForceUpdate=1;

				//printk("yuan,HDR_flag=%d\n",VIP_system_info_structure_table ->HDR_flag);
				//printk("yuan,pre_colorspaceisNative=%d,g_srgbForceUpdate2=%d\n",pre_colorspaceisNative,g_srgbForceUpdate2);


				if(VIP_system_info_structure_table ->HDR_flag==0 || (VIP_system_info_structure_table ->HDR_flag ==HAL_VPQ_HDR_MODE_SDR_MAX_RGB)){
				if(ColoSpace_T.mode==0){//Color Space Mode=Auto
					drvif_color_D_3dLUT_Enable(0);
					fwif_color_set_sRGBMatrix();
				}
				else if(ColoSpace_T.mode==1){//Color Space Mode=Native
					drvif_color_D_3dLUT_Enable(0);
					fwif_color_set_color_mapping_enable(SLR_MAIN_DISPLAY, 0);//disable sRGB;//Scaler_Set_ColorSpace_InvGamma_sRGB_OFF();//Scaler_Set_ColorSpace_InvGamma_sRGB(0);
				}
				else if(ColoSpace_T.mode==2){//Color Space Mode=Custom
					Scaler_Set_ColorSpace_D_3dlutTBL(&ColoSpace_T);
					memcpy(&ColoSpace_Index,&ColoSpace_T,sizeof(RTK_VPQ_COLORSPACE_INDEX));
					drvif_color_D_3dLUT_Enable(1);
					fwif_color_set_sRGBMatrix();
				}
				}
				else{
					//printk("HDR no need to run ColorSpace flow\n");
					drvif_color_D_3dLUT_Enable(0);
					fwif_color_set_sRGBMatrix();
					ColorSpaceMode=0;
				}
				//fwif_color_set_gamma_Magic();
				if(ColorSpaceMode==1)//Color Space Mode=Native
					pre_colorspaceisNative=1;
				else
					pre_colorspaceisNative=0;

				g_srgbForceUpdate = 0;

				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_RGBWTOSRGBMATRIX:
		{
			//printk("[Kernel]VPQ_EXTERN_IOC_SET_RGBWTOSRGBMATRIX\n");
			unsigned char i,j;
			extern short BT709Target_sRGB_APPLY[3][3];
			extern short BT2020Target_sRGB_APPLY[3][3];
			extern short DCIP3Target_sRGB_APPLY[3][3];
			extern short ADOBERGBTarget_sRGB_APPLY[3][3];

			RTK_VPQ_sRGB_Matrix sRGB_T;
			extern unsigned char g_srgbForceUpdate;
			_system_setting_info *VIP_system_info_structure_table = NULL;
			VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

			if (copy_from_user(&sRGB_T,  (int __user *)arg,sizeof(RTK_VPQ_sRGB_Matrix))) {
				ret = -1;
			} else {
			#if 0//for debug
				for(i=0;i<3;i++){
					for(j=0;j<3;j++)
						printk("BT709Target[%d][%d]=%d\n",i,j,sRGB_T.BT709Target[i][j]);
					}
				for(i=0;i<3;i++){
					for(j=0;j<3;j++)
						printk("sRGB_T->BT2020Target[%d][%d]=%d\n",i,j,sRGB_T.BT2020Target[i][j]);
					}
				for(i=0;i<3;i++){
					for(j=0;j<3;j++)
						printk("sRGB_T->DCIP3Target[%d][%d]=%d\n",i,j,sRGB_T.DCIP3Target[i][j]);
					}
				for(i=0;i<3;i++){
					for(j=0;j<3;j++)
						printk("sRGB_T->ADOBERGBTarget[%d][%d]=%d\n",i,j,sRGB_T.ADOBERGBTarget[i][j]);
					}
			#endif
				//-----------------------------------------------------------------------------
				for(i=0;i<3;i++){
					for(j=0;j<3;j++)
						BT709Target_sRGB_APPLY[i][j]=sRGB_T.BT709Target[i][j];
				}
				for(i=0;i<3;i++){
					for(j=0;j<3;j++)
						BT2020Target_sRGB_APPLY[i][j]=sRGB_T.BT2020Target[i][j];
				}
				for(i=0;i<3;i++){
					for(j=0;j<3;j++)
						DCIP3Target_sRGB_APPLY[i][j]=sRGB_T.DCIP3Target[i][j];
				}
				for(i=0;i<3;i++){
					for(j=0;j<3;j++)
						ADOBERGBTarget_sRGB_APPLY[i][j]=sRGB_T.ADOBERGBTarget[i][j];
				}

				g_srgbForceUpdate = 0;

				drvif_color_set_PQ_Module(PQ_ByPass_Gamma,0);
				drvif_color_set_PQ_Module(PQ_ByPass_InvGamma,0);

				ret = 0;
			}
		}
		break;

		case VPQ_EXTERN_IOC_GET_PANORAMA_TYPE:
		{
			TV030_RATIO_TYPE_E PamoramaType = (TV030_RATIO_TYPE_E)Scaler_DispCheckRatio(SLR_RATIO_PANORAMA);

			if(copy_to_user((void __user *)arg, (void *)&PamoramaType, sizeof(TV030_RATIO_TYPE_E)))
			{
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_PANORAMA_TYPE fail\n");
				ret = -1;
			}else
				ret = 0;
		}
		break;

		case VPQ_EXTERN_IOC_GET_VD_COLOR_byAP:
		{
			VD_COLOR_ST vd_color = {0};
			if (fwif_color_module_vdc_GetConBriSatHue_byAP(&vd_color) != 0)
				return -1;

			if(copy_to_user((void __user *)arg, &vd_color, sizeof(VD_COLOR_ST))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_VD_COLOR_byAP fail\n");
				ret = -1;
			} else
				ret = 0;
		}
		break;

		case VPQ_EXTERN_IOC_SET_VD_COLOR_byAP:
		{
			VD_COLOR_ST vd_color = {0};

			if (copy_from_user(&vd_color,  (void __user *)arg,sizeof(VD_COLOR_ST))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_VD_COLOR_byAP fail\n");
				ret = -1;
			} else {
				ret = fwif_color_module_vdc_SetConBriSatHue_byAP(&vd_color);
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_OD_TABLE:
		{
			extern SLR_VIP_TABLE_OD slr_od_table;
			extern DLG_OD_TABLE_TYPE dlg_od_table;

			rtd_printk(KERN_EMERG, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_OD_TABLE \n");

			if (copy_from_user(&slr_od_table,  (void __user *)arg,sizeof(SLR_VIP_TABLE_OD))) {
				rtd_printk(KERN_EMERG, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_OD_TABLE fail\n");
				ret = -1;
			} else {
				memcpy(&dlg_od_table.dlg_od_normal_table,&slr_od_table,sizeof(SLR_VIP_TABLE_OD));
				if(drvif_color_od_enable_get() == 0)
				{
					rtd_printk(KERN_EMERG, "OD", "[IOC] od is off,init od by bin!!!\n");
					if(Scaler_Init_OD()){
						fwif_color_set_od_table();
						drvif_color_od_enable_set(1);
					}
				}
			}
		}
		break;
        case VPQ_EXTERN_IOC_SET_RADCR_Para:
        {
            extern int scalerVIP_RADCR_set_table_from_user_space(unsigned long user_data_addr);
            if (copy_from_user(&HAL_VPQ_DATA_EXTERN, (int __user *)arg, sizeof(HAL_VPQ_DATA_EXTERN_T))) {
                rtd_printk(KERN_ERR, TAG_NAME, "VPQ_EXTERN_IOC_SET_RADCR_Para hal_VPQ_DATA_EXTERN copy fail\n");
                ret = -1;
            } else {
                if (HAL_VPQ_DATA_EXTERN.length != sizeof(RADCR_TBL_ST)) {
                    rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_SET_RADCR_Para len error len=%d,\n", HAL_VPQ_DATA_EXTERN.length);
                    ret = -1;
                } else {
                    ret = scalerVIP_RADCR_set_table_from_user_space(HAL_VPQ_DATA_EXTERN.pData);
                }
            }
        }
        break;

        case VPQ_EXTERN_IOC_SET_RADCR_En:
        {
            extern void scalerVIP_RADCR_set_enable(short enable);
            unsigned int En;
            if (copy_from_user(&En,  (void __user *)arg, sizeof(unsigned int))) {
                ret = -1;
            } else {
                scalerVIP_RADCR_set_enable(En);
                ret = 0;
            }
        }
        break;

	case VPQ_EXTERN_IOC_SET_RADCR_Backlight_Mapping:
	{
		extern int scalerVIP_RADCR_set_bl_mapping_table_from_user_space(unsigned long user_data_addr);
		if (copy_from_user(&HAL_VPQ_DATA_EXTERN, (int __user *)arg, sizeof(HAL_VPQ_DATA_EXTERN_T))) {
			rtd_pr_vpq_err("VPQ_EXTERN_IOC_SET_RADCR_Para hal_VPQ_DATA_EXTERN copy fail\n");
			ret = -1;
		} else {
			if (HAL_VPQ_DATA_EXTERN.length != sizeof(RADCR_BL_MAP_ST)) {
				rtd_pr_vpq_info("kernel VPQ_EXTERN_IOC_SET_RADCR_Backlight_Mapping len error len=%d,\n", HAL_VPQ_DATA_EXTERN.length);
				ret = -1;
			} else {
				ret = scalerVIP_RADCR_set_bl_mapping_table_from_user_space(HAL_VPQ_DATA_EXTERN.pData);
			}
		}
	}
	break;

	default:
		//rtd_printk(KERN_DEBUG, TAG_NAME, "kernel IO command %d not found!\n", cmd);
		//printk("VPQ_EXTERN_IOC cmd=0x%x unknown\n", cmd);
		return -1;
			;
	}

	PictureModeChg_ByHSBC(HSBC_Chg);

	return ret;/*Success*/

}

#if defined(CONFIG_RTK_8KCODEC_INTERFACE)
#include "kernel/rtk_codec_interface.h"
extern struct rtk_codec_interface *rtk_8k;
#if IS_ENABLED(CONFIG_RTK_AI_DRV)
void VPQEX_rlink_AI_SeneInfo(void)
{
	extern int scene_nn;
	extern int scene_pq;
	unsigned int scene_info[2] = {(unsigned int)scene_nn, (unsigned int)scene_pq};

	if(rtk_8k == NULL)
	{
		printk("%s, null pointer: rtk_8k\n", __FUNCTION__);
		return;
	}

	rtk_8k->vpqex->R8k_VPQEX_AI_SET_SceneInfo(scene_info);

	//printk("%s, scene_nn:%d, scene_pq:%d, set ! \n", __FUNCTION__, scene_nn, scene_pq);
}
#endif
char VPQEX_rlink_host_which_source(void)
{
	static unsigned int vip_srcTiming = 0xFFFFFFFF;
	static unsigned char src_pre = 0xFF;
	unsigned int vFreq;
	_RPC_system_setting_info *rpcSystemInfo = NULL;
	rpcSystemInfo = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if(rpcSystemInfo == NULL)
	{
		printk("%s, rpcSystemInfo null pointer: rtk_8k\n", __FUNCTION__);
		return -1;
	}

	if(rtk_8k == NULL)
	{
		printk("%s, null pointer: rtk_8k\n", __FUNCTION__);
		return -1;
	}

	//if (rpcSystemInfo->VIP_source != src_pre) {

		//vip_srcTiming = rpcSystemInfo->VIP_source;	// there are different source define between m7, mk2 and h5, send source timng to slave
		vFreq = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ);
		if (vFreq <= 350)
			vFreq = 0;
		else
			vFreq = 1;

		vip_srcTiming = (Get_DisplayMode_Src(SLR_MAIN_DISPLAY)<<28) + (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_IPV_ACT_LEN)<<16)
			+ (Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE)<<15) + (vFreq<<14)
			+ (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_IPH_ACT_WID));

		rtk_8k->vpqex->R8k_VPQEX_host_which_source(&vip_srcTiming);

		rtd_printk(KERN_INFO, TAG_NAME, "VPQEX_rlink_host_which_source, vip_src_timing change from %d to %d, vip_srcTiming=%x,\n",
			src_pre, rpcSystemInfo->VIP_source, vip_srcTiming);
		src_pre = rpcSystemInfo->VIP_source;
		//Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI
	//}
	return 0;
	//printk("%s, scene_nn:%d, scene_pq:%d, set ! \n", __FUNCTION__, scene_nn, scene_pq);
}

char VPQEX_rlink_host_OSD_Sharpness_Info_Send(void)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;
	unsigned char OSD_sharpness, OSD_sharpness_gain, OSD_sharpness_gain_100;
	static unsigned int send_val[5] = {0};

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		printk("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	OSD_sharpness = VIP_system_info_structure_table->OSD_Info.OSD_Sharpness;		// shp ui
	OSD_sharpness_gain = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_SHARPNESS, OSD_sharpness);		// shp gain in 0~255
	OSD_sharpness_gain_100 = (OSD_sharpness_gain*100) >> 8;	// shp gain in 0~100

	if(rtk_8k == NULL)
	{
		printk("%s, null pointer: rtk_8k\n", __FUNCTION__);
		return -1;
	}

	send_val[0] = OSD_sharpness;
	send_val[1] = OSD_sharpness_gain;
	send_val[2] = OSD_sharpness_gain_100;

	rtk_8k->vpqex->R8k_VPQEX_host_host_OSD_Sharpness_Info(&send_val[0]);

	rtd_printk(KERN_INFO, TAG_NAME, "rlink_host_OSD_Shp_Info, OSD_shp=%d, OSD_shp_gain=%d\n", OSD_sharpness, OSD_sharpness_gain);

	return 0;
}

char VPQEX_rlink_host_OSD_NNSR_Info_Send(unsigned int *OSD_NNSR_Info)
{
	_system_setting_info *VIP_system_info_structure_table = NULL;

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	if (VIP_system_info_structure_table == NULL) {
		printk("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	if (OSD_NNSR_Info == NULL) {
		printk("~VPQEX_rlink_host_OSD_NNSR_Info_Send NULL return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	if(rtk_8k == NULL)
	{
		printk("%s, null pointer: rtk_8k\n", __FUNCTION__);
		return -1;
	}

	rtk_8k->vpqex->R8k_VPQEX_host_host_OSD_NNSR_Info(&OSD_NNSR_Info[0]);

	rtd_printk(KERN_INFO, TAG_NAME, "rlink_host_OSD_NNSR_Info, OSD_NNSR_Info=%d,%d,%d,%d,%d,\n",
		OSD_NNSR_Info[0], OSD_NNSR_Info[1], OSD_NNSR_Info[2], OSD_NNSR_Info[3], OSD_NNSR_Info[4]);

	return 0;
}

char VPQEX_rlink_host_Noise_Level_Info_Send(void)
{
	unsigned int t_th = 90 * 500; // 500 ms
	_system_setting_info *VIP_system_info_structure_table = NULL;
	_clues* SmartPic_clue=NULL;
	unsigned int MAD_level[4];
	static unsigned int t_90k_pre = 0;
	unsigned int t_90k, t_90k_diff;
	unsigned int noise_MAD_Lv;
	static unsigned int noise_MAD_Lv_pre = 0xffffffff;

	VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	SmartPic_clue = fwif_color_GetShare_Memory_SmartPic_clue();

	if (VIP_system_info_structure_table == NULL || SmartPic_clue == NULL) {
		printk("~get VIP_system_info_structure_table Error return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return -1;
	}

	/*if (Noise_Level_Info == NULL) {
		printk("~VPQEX_rlink_host_OSD_NNSR_Info_Send NULL return, %s->%d, %s~\n", __FILE__, __LINE__, __FUNCTION__);
		return -1;
	}*/

	if(rtk_8k == NULL)
	{
		printk("%s, null pointer: rtk_8k\n", __FUNCTION__);
		return -1;
	}

	t_90k = drvif_Get_90k_Lo_clk();
	t_90k_diff = (t_90k>t_90k_pre)?(t_90k - t_90k_pre):(t_90k);

	MAD_level[0] = SmartPic_clue->RTNR_MAD_count_Y_avg_ratio;
	MAD_level[1] = SmartPic_clue->RTNR_MAD_count_Y2_avg_ratio;
	MAD_level[2] = SmartPic_clue->RTNR_MAD_count_Y3_avg_ratio;
	MAD_level[3] = SmartPic_clue->RTNR_MAD_count_Y4_avg_ratio;

	noise_MAD_Lv = MAD_level[1] / 100; // use [1] as noise
	// update MAD hist, while 1. every 500 ms, 2. mode is active, 3. noise level change
	if ((t_90k_diff > t_th) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_STATE) == _MODE_STATE_ACTIVE) &&
		(noise_MAD_Lv != noise_MAD_Lv_pre)) {

		rtk_8k->vpqex->R8k_VPQEX_host_host_Noise_Level_Info(&MAD_level[0]);

		rtd_printk(KERN_INFO, TAG_NAME, "rlink Noise_Level_Info, Noise_Level_Info=%d,%d,%d,%d,%d,t_90k=%d,\n",
			MAD_level[0], MAD_level[1], MAD_level[2], MAD_level[3], MAD_level[4], t_90k);

		t_90k_pre = t_90k;
		noise_MAD_Lv_pre = noise_MAD_Lv;
	}
	return 0;
}

#elif defined(CONFIG_H5CX_SUPPORT)
//#define H5X_I2C_PORT                3
///< Only available on the RTK platform
//#define I2C_M_NORMAL_SPEED      0x0000 /* 20120716 - Kevin Wang add for Standard Speed Transmission : 100Kbps */
//#define I2C_M_FAST_SPEED        0x0002 /* 20120716 - Kevin Wang add for Fast Speed Transmission : 400Kbps */
//#define I2C_M_HIGH_SPEED        0x0004 /* 20120716 - Kevin Wang add for High Speed Transmission : > 400Kbps to max 3.4 Mbps */
//#define I2C_M_LOW_SPEED         0x0006 /* 20120716 - Kevin Wang add for Low  Speed Transmission : < 100Kbps */
//#define H5X_I2C_SPEED_FLAG      I2C_M_HIGH_SPEED

//#define i2c_rlink_VPQ_ADDR 0x32
#define i2c_rlink_Host_VPQ_Block_size 16
#define i2c_rlink_Cmd_size_less8k 2
//extern int i2c_master_send_ex_flag(unsigned char bus_id, unsigned char addr, unsigned char *write_buff, unsigned int write_len, __u16 flags);

static int VPQEX_i2c_calc_checksum(unsigned char *data, unsigned short len, unsigned char *checksum)
{
    unsigned int sum = 0;
    unsigned int i = 0;

    if ((data == NULL) || (len == 0) || (NULL == checksum))
    return -1;

    for (; i<len; i++)
    sum += data[i];

    *checksum = (unsigned char)((256-(sum%256)) %256);
    return 0;
}

char VPQEX_rlink_host_which_source(void)
{
	unsigned char i2c_cmd_data[i2c_rlink_Cmd_size_less8k+i2c_rlink_Host_VPQ_Block_size+1];
	_RPC_system_setting_info *rpcSystemInfo = NULL;
	unsigned char checksum;
	int ret_val = 0;

	rpcSystemInfo = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if(rpcSystemInfo == NULL)
	{
		rtd_pr_vpq_emerg("%s, rpcSystemInfo null pointer: rtk_8k\n", __FUNCTION__);
		return -1;
	}
	memset(i2c_cmd_data, 0, sizeof(i2c_cmd_data));
	// cmd
	i2c_cmd_data[0] = I2C_ONLY_PQ;	// I2C_ONLY_PQ = 0x32
	i2c_cmd_data[1] = VIP_RLK_I2C_ONLY_HOST_SOURCE_INFO;
	// data
	i2c_cmd_data[2] = Get_DisplayMode_Src(SLR_MAIN_DISPLAY);
	i2c_cmd_data[3] = (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_IPH_ACT_WID)&0xFF00)>>8;
	i2c_cmd_data[4] = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_IPH_ACT_WID)&0x00FF;
	i2c_cmd_data[5] = (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_IPV_ACT_LEN)&0xFF00)>>8;
	i2c_cmd_data[6] = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_IPV_ACT_LEN)&0x00FF;
	i2c_cmd_data[7] = Scaler_DispGetStatus(SLR_MAIN_DISPLAY, SLR_DISP_INTERLACE);
	i2c_cmd_data[8] = Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_V_FREQ)/10;
	// checksum
	VPQEX_i2c_calc_checksum(&i2c_cmd_data[i2c_rlink_Cmd_size_less8k], i2c_rlink_Host_VPQ_Block_size, &checksum);
	i2c_cmd_data[i2c_rlink_Cmd_size_less8k+i2c_rlink_Host_VPQ_Block_size] = checksum;

	ret_val = i2c_master_send_ex_flag(H5X_I2C_PORT, H5X_I2C_ADDR, i2c_cmd_data, (unsigned short)sizeof(i2c_cmd_data), H5X_I2C_SPEED_FLAG);
    if (ret_val < 0)
    {
		// TODO. error handle
		rtd_pr_vpq_emerg("%s send data failed, ret_val=%d, size=%d \n", __func__, ret_val, sizeof(i2c_cmd_data));
    } else {
		rtd_pr_vpq_info("%s, i2c_src[0:9]=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",
						__func__, i2c_cmd_data[0], i2c_cmd_data[1], i2c_cmd_data[2], i2c_cmd_data[3], i2c_cmd_data[4], i2c_cmd_data[5],
						i2c_cmd_data[6], i2c_cmd_data[6], i2c_cmd_data[8], i2c_cmd_data[9], i2c_cmd_data[10], i2c_cmd_data[11],
						i2c_cmd_data[12], i2c_cmd_data[13], i2c_cmd_data[14], i2c_cmd_data[15], i2c_cmd_data[16], i2c_cmd_data[17], i2c_cmd_data[18]);
    }
	return ret_val;

}

char VPQEX_rlink_host_OSD_Sharpness_Info_Send(void)
{
	unsigned char i2c_cmd_data[i2c_rlink_Cmd_size_less8k+i2c_rlink_Host_VPQ_Block_size+1];
	_system_setting_info *SystemInfo = NULL;
	unsigned char checksum;
	unsigned char OSD_sharpness, OSD_sharpness_gain, OSD_sharpness_gain_100;
	int ret_val = 0;

	SystemInfo = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if(SystemInfo == NULL)
	{
		rtd_pr_vpq_emerg("%s, SystemInfo null pointer: rtk_8k\n", __FUNCTION__);
		return -1;
	}
	memset(i2c_cmd_data, 0, sizeof(i2c_cmd_data));

	OSD_sharpness = SystemInfo->OSD_Info.OSD_Sharpness;		// shp ui
	OSD_sharpness_gain = fwif_OsdMapToRegValue(APP_VIDEO_OSD_ITEM_SHARPNESS, OSD_sharpness);		// shp gain in 0~255
	OSD_sharpness_gain_100 = (OSD_sharpness_gain*100) >> 8;	// shp gain in 0~100

	// cmd
	i2c_cmd_data[0] = I2C_ONLY_PQ;	// I2C_ONLY_PQ = 0x32
	i2c_cmd_data[1] = VIP_RLK_I2C_ONLY_OSD_Sharpness_Info;
	// data
	i2c_cmd_data[2] = OSD_sharpness;
	i2c_cmd_data[3] = OSD_sharpness_gain;
	i2c_cmd_data[4] = OSD_sharpness_gain_100;
	// checksum
	VPQEX_i2c_calc_checksum(&i2c_cmd_data[i2c_rlink_Cmd_size_less8k], i2c_rlink_Host_VPQ_Block_size, &checksum);
	i2c_cmd_data[i2c_rlink_Cmd_size_less8k+i2c_rlink_Host_VPQ_Block_size] = checksum;

	ret_val = i2c_master_send_ex_flag(H5X_I2C_PORT, H5X_I2C_ADDR, i2c_cmd_data, (unsigned short)sizeof(i2c_cmd_data), H5X_I2C_SPEED_FLAG);
    if (ret_val < 0)
    {
		// TODO. error handle
		rtd_pr_vpq_emerg("%s send data failed, ret_val=%d, size=%d \n", __func__, ret_val, sizeof(i2c_cmd_data));
    } else {
		rtd_pr_vpq_info("%s, i2c_src[0:9]=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",
						__func__, i2c_cmd_data[0], i2c_cmd_data[1], i2c_cmd_data[2], i2c_cmd_data[3], i2c_cmd_data[4], i2c_cmd_data[5],
						i2c_cmd_data[6], i2c_cmd_data[6], i2c_cmd_data[8], i2c_cmd_data[9], i2c_cmd_data[10], i2c_cmd_data[11],
						i2c_cmd_data[12], i2c_cmd_data[13], i2c_cmd_data[14], i2c_cmd_data[15], i2c_cmd_data[16], i2c_cmd_data[17], i2c_cmd_data[18]);
    }

	return ret_val;

}

char VPQEX_rlink_host_OSD_NNSR_Info_Send(unsigned int *OSD_NNSR_Info)
{
	unsigned char i2c_cmd_data[i2c_rlink_Cmd_size_less8k+i2c_rlink_Host_VPQ_Block_size+1];
	_system_setting_info *SystemInfo = NULL;
	unsigned char checksum;

	int ret_val = 0;

	SystemInfo = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	if(SystemInfo == NULL)
	{
		rtd_pr_vpq_emerg("%s, SystemInfo null pointer: rtk_8k\n", __FUNCTION__);
		return -1;
	}
	memset(i2c_cmd_data, 0, sizeof(i2c_cmd_data));

	// cmd
	i2c_cmd_data[0] = I2C_ONLY_PQ;	// I2C_ONLY_PQ = 0x32
	i2c_cmd_data[1] = VIP_RLK_I2C_ONLY_OSD_NNSR_Info;
	// data, how to get nnsr info?
	i2c_cmd_data[2] = 2;
	i2c_cmd_data[3] = 3;
	i2c_cmd_data[4] = 4;
	// checksum
	VPQEX_i2c_calc_checksum(&i2c_cmd_data[i2c_rlink_Cmd_size_less8k], i2c_rlink_Host_VPQ_Block_size, &checksum);
	i2c_cmd_data[i2c_rlink_Cmd_size_less8k+i2c_rlink_Host_VPQ_Block_size] = checksum;

	ret_val = i2c_master_send_ex_flag(H5X_I2C_PORT, H5X_I2C_ADDR, i2c_cmd_data, (unsigned short)sizeof(i2c_cmd_data), H5X_I2C_SPEED_FLAG);
    if (ret_val < 0)
    {
		// TODO. error handle
		rtd_pr_vpq_emerg("%s send data failed, ret_val=%d, size=%d \n", __func__, ret_val, sizeof(i2c_cmd_data));
    } else {
		rtd_pr_vpq_info("%s, i2c_src[0:9]=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",
						__func__, i2c_cmd_data[0], i2c_cmd_data[1], i2c_cmd_data[2], i2c_cmd_data[3], i2c_cmd_data[4], i2c_cmd_data[5],
						i2c_cmd_data[6], i2c_cmd_data[6], i2c_cmd_data[8], i2c_cmd_data[9], i2c_cmd_data[10], i2c_cmd_data[11],
						i2c_cmd_data[12], i2c_cmd_data[13], i2c_cmd_data[14], i2c_cmd_data[15], i2c_cmd_data[16], i2c_cmd_data[17], i2c_cmd_data[18]);
    }

	return ret_val;

}

extern VIP_RLK_Cal_LV_Info Host_RLK_Cal_LV_Info;
extern unsigned char scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio(void);
extern unsigned char scalerVIP_DI_MiddleWare_MCNR_Get_Pan_Flag(void);
extern unsigned char scalerVIP_DI_MiddleWare_MCNR_Get_V_Pan_Flag(void);
char VPQEX_rlink_host_Noise_Level_Info_Send(void)
{
	unsigned int t_th = 90 * 300; // 300 ms
	_clues* SmartPic_clue=NULL;
	static unsigned int t_90k_pre = 0;
	unsigned int t_90k, t_90k_diff;

	unsigned char i2c_cmd_data[i2c_rlink_Cmd_size_less8k+i2c_rlink_Host_VPQ_Block_size+1];
	_system_setting_info *SystemInfo = NULL;
	unsigned char checksum;
	static unsigned char log_cnt = 0;
	int ret_val = 0;

	SystemInfo = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
	SmartPic_clue = fwif_color_GetShare_Memory_SmartPic_clue();

	if((SystemInfo == NULL) ||  (SmartPic_clue == NULL))
	{
		rtd_pr_vpq_emerg("%s, SystemInfo or SmartPic_clue null pointer: i2c \n", __FUNCTION__);
		return -1;
	}
	memset(i2c_cmd_data, 0, sizeof(i2c_cmd_data));

	t_90k = drvif_Get_90k_Lo_clk();
	t_90k_diff = (t_90k>t_90k_pre)?(t_90k - t_90k_pre):(t_90k);

	// cmd
	i2c_cmd_data[0] = I2C_ONLY_PQ;	// I2C_ONLY_PQ = 0x32
	i2c_cmd_data[1] = VIP_RLK_I2C_ONLY_Noise_Level_Info;
	// data,
	i2c_cmd_data[2] = (SmartPic_clue->RTNR_MAD_count_Y_avg_ratio&0xFF00)>>8;
	i2c_cmd_data[3] = SmartPic_clue->RTNR_MAD_count_Y_avg_ratio&0x00FF;
	i2c_cmd_data[4] = (SmartPic_clue->RTNR_MAD_count_Y2_avg_ratio&0xFF00)>>8;
	i2c_cmd_data[5] = SmartPic_clue->RTNR_MAD_count_Y2_avg_ratio&0x00FF;
	i2c_cmd_data[6] = (SmartPic_clue->RTNR_MAD_count_Y3_avg_ratio&0xFF00)>>8;
	i2c_cmd_data[7] = SmartPic_clue->RTNR_MAD_count_Y3_avg_ratio&0x00FF;
	i2c_cmd_data[8] = (SmartPic_clue->RTNR_MAD_count_Y4_avg_ratio&0xFF00)>>8;
	i2c_cmd_data[9] = SmartPic_clue->RTNR_MAD_count_Y4_avg_ratio&0x00FF;
	i2c_cmd_data[10] = SmartPic_clue->Hist_Y_Mean_Value;
	i2c_cmd_data[11] = scalerVIP_DI_MiddleWare_MCNR_Get_GMV_Ratio();
	i2c_cmd_data[12] = scalerVIP_DI_MiddleWare_MCNR_Get_Pan_Flag();
	i2c_cmd_data[13] = scalerVIP_DI_MiddleWare_MCNR_Get_V_Pan_Flag();
	i2c_cmd_data[14] = Host_RLK_Cal_LV_Info.MAD_Noise_lv;
	// checksum
	VPQEX_i2c_calc_checksum(&i2c_cmd_data[i2c_rlink_Cmd_size_less8k], i2c_rlink_Host_VPQ_Block_size, &checksum);
	i2c_cmd_data[i2c_rlink_Cmd_size_less8k+i2c_rlink_Host_VPQ_Block_size] = checksum;

	// update MAD hist, while 1. every 300 ms, 2. mode is active, 3. noise level change
	if ((t_90k_diff > t_th) && (Scaler_DispGetInputInfoByDisp(SLR_MAIN_DISPLAY,SLR_INPUT_STATE) == _MODE_STATE_ACTIVE)) {

		ret_val = i2c_master_send_ex_flag(H5X_I2C_PORT, H5X_I2C_ADDR, i2c_cmd_data, (unsigned short)sizeof(i2c_cmd_data), H5X_I2C_SPEED_FLAG);
		if (ret_val < 0)
		{
			// TODO. error handle
			rtd_pr_vpq_emerg("%s send data failed, ret_val=%d, size=%d \n", __func__, ret_val, sizeof(i2c_cmd_data));
		} else {
			if (log_cnt >= 5) {
				rtd_pr_vpq_info("%s, i2c_src[0:9]=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",
					__func__, i2c_cmd_data[0], i2c_cmd_data[1], i2c_cmd_data[2], i2c_cmd_data[3], i2c_cmd_data[4], i2c_cmd_data[5],
					i2c_cmd_data[6], i2c_cmd_data[6], i2c_cmd_data[8], i2c_cmd_data[9], i2c_cmd_data[10], i2c_cmd_data[11],
					i2c_cmd_data[12], i2c_cmd_data[13], i2c_cmd_data[14], i2c_cmd_data[15], i2c_cmd_data[16], i2c_cmd_data[17], i2c_cmd_data[18]);
				log_cnt = 0;
			}
			log_cnt++;
		}

		t_90k_pre = t_90k;
	}

	return ret_val;

}

char VPQEX_rlink_set_Low_Delay(unsigned char low_delay_mode)
{
	unsigned char i2c_cmd_data[i2c_rlink_Cmd_size_less8k+i2c_rlink_Host_VPQ_Block_size+1];
	_RPC_system_setting_info *rpcSystemInfo = NULL;
	unsigned char checksum;
	int ret_val = 0;

	rpcSystemInfo = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);

	if(rpcSystemInfo == NULL)
	{
		rtd_pr_vpq_emerg("%s, rpcSystemInfo null pointer: rtk_8k\n", __FUNCTION__);
		return -1;
	}
	memset(i2c_cmd_data, 0, sizeof(i2c_cmd_data));
	// cmd
	i2c_cmd_data[0] = I2C_ONLY_PQ;	// I2C_ONLY_PQ = 0x32
	i2c_cmd_data[1] = VIP_RLK_I2C_ONLY_LOW_Delay;
	// data
	i2c_cmd_data[2] = (low_delay_mode==1)?(1):(0);
	// checksum
	VPQEX_i2c_calc_checksum(&i2c_cmd_data[i2c_rlink_Cmd_size_less8k], i2c_rlink_Host_VPQ_Block_size, &checksum);
	i2c_cmd_data[i2c_rlink_Cmd_size_less8k+i2c_rlink_Host_VPQ_Block_size] = checksum;

	ret_val = i2c_master_send_ex_flag(H5X_I2C_PORT, H5X_I2C_ADDR, i2c_cmd_data, (unsigned short)sizeof(i2c_cmd_data), H5X_I2C_SPEED_FLAG);
    if (ret_val < 0)
    {
		// TODO. error handle
		rtd_pr_vpq_emerg("%s send data failed, ret_val=%d, size=%d \n", __func__, ret_val, sizeof(i2c_cmd_data));
    } else {
		rtd_pr_vpq_info("%s, i2c_src[0:9]=%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\n",
						__func__, i2c_cmd_data[0], i2c_cmd_data[1], i2c_cmd_data[2], i2c_cmd_data[3], i2c_cmd_data[4], i2c_cmd_data[5],
						i2c_cmd_data[6], i2c_cmd_data[6], i2c_cmd_data[8], i2c_cmd_data[9], i2c_cmd_data[10], i2c_cmd_data[11],
						i2c_cmd_data[12], i2c_cmd_data[13], i2c_cmd_data[14], i2c_cmd_data[15], i2c_cmd_data[16], i2c_cmd_data[17], i2c_cmd_data[18]);
    }
	return ret_val;

}

#endif

struct file_operations vpqex_fops = {
	.owner = THIS_MODULE,
	.open = vpqex_open,
	.release = vpqex_release,
	.read  = vpqex_read,
	.write = vpqex_write,
	.unlocked_ioctl = vpqex_ioctl,
#ifdef CONFIG_ARM64
#ifdef CONFIG_COMPAT
	.compat_ioctl = vpqex_ioctl,
#endif
#endif
};

#ifdef CONFIG_PM
static const struct dev_pm_ops vpqex_pm_ops =
{
        .suspend    = vpqex_suspend,
        .resume     = vpqex_resume,
#ifdef CONFIG_HIBERNATION
	.freeze 	= vpqex_suspend,
	.thaw		= vpqex_resume,
	.poweroff	= vpqex_suspend,
	.restore	= vpqex_resume,
#endif

};
#endif // CONFIG_PM

static struct class *vpqex_class = NULL;
static struct platform_device *vpqex_platform_devs = NULL;
static struct platform_driver vpqex_platform_driver = {
	.driver = {
		.name = VPQ_EXTERN_DEVICE_NAME,
		.bus = &platform_bus_type,
#ifdef CONFIG_PM
		.pm = &vpqex_pm_ops,
#endif
    },
} ;

#ifdef CONFIG_ARM64 //ARM32 compatible
static char *vpqex_devnode(struct device *dev, umode_t *mode)
#else
static char *vpqex_devnode(struct device *dev, mode_t *mode)
#endif
{
	return NULL;
}

int vpqex_major   = 0;
int vpqex_minor   = 0 ;
int vpqex_nr_devs = 1;

int vpqex_module_init(void)
{
	int result;
	//dev_t devno = 0;//vbe device number
	printk("vpqex_module_init %d\n", __LINE__);

	result = alloc_chrdev_region(&vpqex_devno, vpqex_minor, vpqex_nr_devs, VPQ_EXTERN_DEVICE_NAME);
	vpqex_major = MAJOR(vpqex_devno);
	if (result != 0) {
		rtd_printk(KERN_ERR, TAG_NAME, "Cannot allocate VPQEX device number\n");
		printk("vpqex_module_init %d\n", __LINE__);
		return result;
	}

	printk("vpqex_module_init %d\n", __LINE__);

	printk("VPQEX_DEVICE init module major number = %d\n", vpqex_major);
	//vpqex_devno = MKDEV(vpqex_major, vpqex_minor);

	vpqex_class = class_create(THIS_MODULE, VPQ_EXTERN_DEVICE_NAME);
	if (IS_ERR(vpqex_class)) {
		rtd_printk(KERN_ERR, TAG_NAME, "scalevpqex: can not create class...\n");
		printk("vpqex_module_init %d\n", __LINE__);
		result = PTR_ERR(vpqex_class);
		goto fail_class_create;
	}

	vpqex_class->devnode = vpqex_devnode;

	vpqex_platform_devs = platform_device_register_simple(VPQ_EXTERN_DEVICE_NAME, -1, NULL, 0);
	if (platform_driver_register(&vpqex_platform_driver) != 0) {
		rtd_printk(KERN_ERR, TAG_NAME, "scalevpqex: can not register platform driver...\n");
		printk("vpqex_module_init %d\n", __LINE__);
		result = -ENOMEM;
		goto fail_platform_driver_register;
	}

	cdev_init(&vpqex_cdev, &vpqex_fops);
	vpqex_cdev.owner = THIS_MODULE;
   	vpqex_cdev.ops = &vpqex_fops;
	result = cdev_add(&vpqex_cdev, vpqex_devno, 1);
	if (result < 0) {
		rtd_printk(KERN_ERR, TAG_NAME, "scalevpqex: can not add character device...\n");
		printk("vpqex_module_init %d\n", __LINE__);
		goto fail_cdev_init;
	}

	device_create(vpqex_class, NULL, vpqex_devno, NULL, VPQ_EXTERN_DEVICE_NAME);

	//sema_init(&VPQ_EXTERN_Semaphore, 1);
	sema_init(&ICM_Semaphore,1);
	sema_init(&LC_Semaphore,1);
	sema_init(&TV002_PQLib_GeneralFunc_Semaphore,1);
	//sema_init(&I_RGB2YUV_Semaphore,1);
	sema_init(&ColorSpace_Semaphore,1);

	vpqex_boot_init();

	printk("vpqex_module_init %d\n", __LINE__);
	return 0;/*Success*/

fail_cdev_init:
	platform_driver_unregister(&vpqex_platform_driver);
fail_platform_driver_register:
	platform_device_unregister(vpqex_platform_devs);
	vpqex_platform_devs = NULL;
	class_destroy(vpqex_class);
fail_class_create:
	vpqex_class = NULL;
	unregister_chrdev_region(vpqex_devno, 1);
	printk("vpqex_module_init %d\n", __LINE__);
	return result;

}



void __exit vpqex_module_exit(void)
{
	printk("vpqex_module_exit\n");

	if (vpqex_platform_devs == NULL)
		BUG();

	device_destroy(vpqex_class, vpqex_devno);
	cdev_del(&vpqex_cdev);

	platform_driver_unregister(&vpqex_platform_driver);
	platform_device_unregister(vpqex_platform_devs);
	vpqex_platform_devs = NULL;

	class_destroy(vpqex_class);
	vpqex_class = NULL;

	unregister_chrdev_region(vpqex_devno, 1);
}


#ifdef CONFIG_SUPPORT_SCALER_MODULE
// the module init/exit will be moved to scaler_module.c if scaler was built as a kernel module
#else
module_init(vpqex_module_init);
module_exit(vpqex_module_exit);
#endif

