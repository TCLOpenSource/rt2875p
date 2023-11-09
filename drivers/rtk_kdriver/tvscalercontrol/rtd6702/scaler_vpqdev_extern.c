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

#ifdef CONFIG_CUSTOMER_TV002
#include <rtk_pq_proc.h>
#endif

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

//---------------TV030----------------------------
unsigned char cur_src = 0;
unsigned int tICM_bin[VIP_ICM_TBL_X*VIP_ICM_TBL_Y*VIP_ICM_TBL_Z];
unsigned char ColorSpaceMode=0;
unsigned short IPQ_OETF[256];
unsigned char aipq_DynamicContrastLevel=0;
//----------------------------------------------------

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

	//rtd_printk(KERN_EMERG, TAG_NAME, "%s(): count=%d, buf=%p\n", __func__, count, buffer);

	if (count >= 128)
		return -EFAULT;

	if (copy_from_user(cmd_buf, buffer, count))
		return -EFAULT;

	if (cmd_buf[count] != '\0')
		cmd_buf[count] = '\0';

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
unsigned char RGBmode_en=0;
unsigned char g_WBBL_WBBS_enable=0;

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
		if (copy_from_user(&tICM_bin[0], (int __user *)arg, (VIP_ICM_TBL_X*VIP_ICM_TBL_Y*VIP_ICM_TBL_Z)*sizeof(unsigned int))) {
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
						 m_pVipTable->tICM_ini[x][y][z] = (unsigned short)tICM_bin[x*(VIP_ICM_TBL_Y*VIP_ICM_TBL_Z)+y*VIP_ICM_TBL_Z+z];

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

	case VPQ_EXTERN_IOC_SET_PQ_SETGAMMA_TABLESIZE_TV030:
	{
		static RTK_TableSize_Gamma Gamma;
		extern unsigned char GammaEnable;
		unsigned short i=0;
		if (copy_from_user(&Gamma,	(int __user *)arg,sizeof(RTK_TableSize_Gamma))) {
			ret = -1;
		} else {
			for(i = 250 ; i < 256 ; i++)
				printk("gamma,b[%d] = %d\n", i, Gamma.pu16Gamma_b[i]);
			
			fwif_color_gamma_encode_TableSize(&Gamma);
			fwif_color_gamma_encode_TableSize_2(&Gamma);
			GammaEnable = 1;//Gamma.nGammaEnable;
			fwif_color_set_gamma_Magic();
			fwif_color_set_OutputGamma_System();
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
			Scaler_SetLocalContrastEnable((unsigned char)args);
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
		BRIGHTNESS_LUT_OETF BRIGHTNESS_LUT_T;
		#ifdef CONFIG_SUPPORT_IPQ
		extern UINT16 OETF_LUT_R[1025];
		extern UINT16 IPQ_OETF_LUT_R[1025];
		extern unsigned int EOTF_LUT_2084_nits[OETF_size];
		extern unsigned char OETF_STOP;//3002
		extern unsigned char OETFCurve_STOP;//3002
		#endif
		_system_setting_info *VIP_system_info_structure_table = NULL;
		VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
		
		if (copy_from_user(&BRIGHTNESS_LUT_T,  (int __user *)arg,sizeof(BRIGHTNESS_LUT_OETF))) {
			rtd_printk(KERN_EMERG, TAG_NAME,"VPQ_EXTERN_IOC_SET_BRIGHTNESS_LUT_OETF, copy fail\n");
			ret = -1;
			break;
		} else {
			memcpy((unsigned char*)&IPQ_OETF[0], (unsigned char*)&BRIGHTNESS_LUT_T.pf_LUT[0], sizeof(short)*256);
			if(BRIGHTNESS_LUT_T.un16_length==256){
				if (VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_HLG || VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_HDR10) {
					rtd_printk(KERN_DEBUG, TAG_NAME, "HDR Curve\n");
					#ifdef CONFIG_SUPPORT_IPQ
					if(OETF_STOP){
						printk("OETF_STOP\n");
					}
					else{
						if(OETFCurve_STOP){
							printk("fixed oetf curve\n");
							fwif_color_set_DM2_OETF_TV006(IPQ_OETF_LUT_R,0);
							drvif_color_set_BBC_shift_bit(4);
							drvif_color_Set_ST2094_3Dlut_CInv(1024<<(9-4), 1);								
						}
						else{
							Scaler_SetBrightness_LUT_OETF(OETF_LUT_R,&BRIGHTNESS_LUT_T);
							fwif_color_set_DM2_OETF_TV006(OETF_LUT_R, 0);//fwif_color_set_DM2_OETF_TV006(OETF_LUT_R, 0);
						}
						if (VIP_system_info_structure_table->HDR_flag == HAL_VPQ_HDR_MODE_HDR10) {
							drvif_fwif_color_Set_HDR10_EOTF_depNITS();
							fwif_color_set_DM2_EOTF_TV006(EOTF_LUT_2084_nits, 0);
						}
					}
					#endif
				}else
					printk("OETF 256 but flag!=HLG/HDR\n");
			}else if(BRIGHTNESS_LUT_T.un16_length==1024){
					printk("SDR GammaCurve\n");
					Scaler_SetOETF2Gamma(&BRIGHTNESS_LUT_T);
					fwif_color_set_gamma_Magic();
					//Scaler_SetBrightness_LUT_OETF(OETF_LUT_R,&BRIGHTNESS_LUT_T);
					//fwif_color_rtice_DM2_OETF_Set(OETF_LUT_R,OETF_LUT_R,OETF_LUT_R);
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
		
		RTK_VPQ_sRGB_Matrix sRGB_T;
		extern unsigned char g_srgbForceUpdate;
		//_system_setting_info *VIP_system_info_structure_table = NULL;
		//VIP_system_info_structure_table = (_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);
		printk("[Kernel]VPQ_EXTERN_IOC_SET_RGBWTOSRGBMATRIX\n");

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
			fwif_color_set_sRGBMatrix();
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
			Scaler_SetLocalContrastTable(level);//table 0 for low; table 1 for high
			ret = 0;
		}
	}
	break;
	
	default:
		rtd_printk(KERN_DEBUG, TAG_NAME, "kernel IO command %d not found!\n", cmd);
		printk("VPQ_EXTERN_IOC cmd=0x%x unknown\n", cmd);
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
    unsigned int wcg_mode = 0;
	unsigned int DCR_backlight = 0;
	HAL_VPQ_DATA_EXTERN_T HAL_VPQ_DATA_EXTERN;
	unsigned char *pBuff_tmp;
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
		#ifndef CONFIG_MEMC_BYPASS
                //for video path boot logo
                Scaler_MEMC_output_force_bg_enable(TRUE,__func__,__LINE__);
		#endif
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
			rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_INIT_PRJ_ID = %d\n", prj_id);
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


				#if 1
				printk("m_pVipTable->VIP_QUALITY_Coef[][]={\n");
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
					extern int GDMA_SetAlphaOsdDetectionParameter(int data);

					GDMA_SetAlphaOsdDetectionParameter(m_pVipTable->Auto_Function_Array4[3]);
				}
			}
			
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
			printk("VPQ_EXTERN_IOC_GET_BRIGHTNESS\n");
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
			printk("VPQ_EXTERN_IOC_SET_BRIGHTNESS\n");
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {
				ret = -1;
			} else {
				ret = 0;
				Scaler_SetBrightness((unsigned char)args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_CONTRAST:
		{
			printk("VPQ_EXTERN_IOC_GET_CONTRAST\n");
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
			printk("VPQ_EXTERN_IOC_SET_CONTRAST\n");
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetContrast((unsigned char)args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_SATURATION:
		{
			printk("VPQ_EXTERN_IOC_GET_SATURATION\n");
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
			printk("VPQ_EXTERN_IOC_SET_SATURATION\n");
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetSaturation((unsigned char)args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_HUE:
		{
			printk("VPQ_EXTERN_IOC_GET_HUE\n");
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
			printk("VPQ_EXTERN_IOC_SET_HUE\n");
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
				Scaler_SetHue((unsigned char)args);
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
			printk("VPQ_EXTERN_IOC_SET_COLOR_TEMP_EXTERN\n");
			if (copy_from_user(&color_temp_data_s,  (int __user *)arg,sizeof(TV001_COLOR_TEMP_DATA_S))) {

				ret = -1;
			} else {
				ret = 0;

				if (!g_WBBL_WBBS_enable) {
					//Scaler_SetColorTemp_level_type((TV001_COLORTEMP_E)color_temp_data_s);
					Scaler_SetColorTempData((TV001_COLOR_TEMP_DATA_S*) &color_temp_data_s);

					if(fwif_VIP_get_Project_ID() == VIP_Project_ID_TV030)
						fwif_color_set_OutputGamma_System();
				}

				#if IS_ENABLED(CONFIG_RTK_AI_DRV)
				memcpy(&vpqex_color_temp, &color_temp_data_s, sizeof(TV001_COLOR_TEMP_DATA_S));
				#endif
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
		case VPQ_EXTERN_IOC_GET_HISTOGRAM:
		{
			//printk("VPQ_EXTERN_IOC_GET_HISTOGRAM\n");
			if (fwif_color_get_HistoData_Countbins(TV006_VPQ_chrm_bin, &(HistoData_bin[0])) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_IOC_GET_HistoData_Countbins fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&HistoData_bin[0], sizeof(HistoData_bin))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_IOC_GET_HistoData_Countbins fail\n");
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
						Scaler_Set_HDR_YUV2RGB(SLR_MAIN_DISPLAY, g_HDR_mode, g_HDR_color_format);
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
				#ifndef CONFIG_CUSTOMER_TV002
				Scaler_Set_CinemaMode_PQ(cinema_pic_mode);
				#endif
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

						fwif_color_ColorMap_SetMatrix(SLR_MAIN_DISPLAY, ColorMap_Matrix_Apply, 1, 1);
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
				rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_TV002_MAGIC_GAMMA[%s:%d]\n", __FUNCTION__, __LINE__);

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
                rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_PQ_SETGAMMA_TABLESIZE[%s:%d]\n", __FUNCTION__, __LINE__);
				fwif_set_gamma(1, 0, 0, 0);

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
				Check_smooth_toggle_update_flag((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY));
				zoom_update_scaler_info_from_vo_smooth_toggle((SCALER_DISP_CHANNEL)Scaler_DispGetInputInfo(SLR_INPUT_DISPLAY),0);
				drvif_color_ultrazoom_config_scaling_up((TV030_RATIO_TYPE_E)args);
				IoReg_SetBits(SCALEUP_DM_UZU_DB_CTRL_reg, SCALEUP_DM_UZU_DB_CTRL_db_apply_mask);
				if(args)
					Scaler_DispSetRatioMode(SLR_RATIO_PANORAMA);
				else
					Scaler_DispSetRatioMode(SLR_RATIO_CUSTOM);

				imd_smooth_main_double_buffer_apply(ZOOM_UZU);
				ret = 0;
			}
			//rtd_printk(KERN_ERR, TAG_NAME, "=======VPQ_EXTERN_IOC_SET_PANORAMA_TYPE ret= %d args=%d \n", ret,args);
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
				fwif_color_set_gamma_Magic();
				if(ColorSpaceMode==1)//Color Space Mode=Native
					pre_colorspaceisNative=1;
				else
					pre_colorspaceisNative=0;

				g_srgbForceUpdate = 0;

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

		case VPQ_EXTERN_IOC_SET_WCG_MODE:
		{
			rtd_printk(KERN_EMERG, TAG_NAME, "VPQ_EXTERN_IOC_SET_WCG_MODE\n");
            /*
                2:Standard
                1:Auto
                0:WCG
            */
			if (copy_from_user(&wcg_mode,  (int __user *)arg,sizeof(unsigned int))) {

				ret = -1;
			} else {
				ret = 0;
                rtd_printk(KERN_EMERG, TAG_NAME, "wcg_mode=%d\n",wcg_mode);
                #ifdef Add_Panel_Coordinates
                if(wcg_mode==1)/*auto mode*/
                    fwif_color_set_sRGBMatrix();
                else
                #endif
				fwif_color_set_ColorMap_3x3_Table(wcg_mode);
			}
		}
		break;
        
	default:
		rtd_printk(KERN_DEBUG, TAG_NAME, "kernel IO command %d not found!\n", cmd);
		printk("VPQ_EXTERN_IOC cmd=0x%x unknown\n", cmd);
		return -1;
			;
	}
	return ret;/*Success*/

}
#ifdef CONFIG_RTK_8KCODEC_INTERFACE
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