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
#include <tvscalercontrol/scaler/scalercolorlib_tv010.h>
#include <tvscalercontrol/vip/scalerColor.h>
//#include <tvscalercontrol/vip/scalerColor_tv010.h>
#include <tvscalercontrol/vip/viptable.h>
#include <scaler_vpqmemcdev.h>
#include "scaler_vpqdev_extern_tv010.h"
#include "scaler_vscdev.h"
#include "ioctrl/vpq/vpq_extern_cmd_id.h"
#include "ioctrl/vpq/vpq_externsky_cmd_id.h"
#include <tvscalercontrol/vip/pq_rpc.h>
//#include <mach/RPCDriver.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/pll27x_reg_reg.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/gibi_od.h>

#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/scalerdrv/scalerip.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
//TODO, Merlin3 tv010 sync, need modify
//#include <rbus/rbus_DesignSpec_MISC_GPIOReg.h>
#include <rtd_log/rtd_module_log.h>


#define TAG_NAME "VPQEX"

static dev_t vpqex_devno;/*vpq device number*/
static struct cdev vpqex_cdev;
unsigned int vpqex_project_id = 0; 
static unsigned char PQ_Dev_Extern_Status = PQ_DEV_EXTERN_NOTHING;

int vpqex_open(struct inode *inode, struct file *filp)
{
	rtd_printk(KERN_INFO, TAG_NAME,"vpqex_open %d\n", __LINE__);
	return 0;
}

ssize_t  vpqex_read(struct file *filep, char *buffer, size_t count, loff_t *offp)
{
	return 0;
}

ssize_t vpqex_write(struct file *filep, const char *buffer, size_t count, loff_t *offp)
{
	return 0;
}

int vpqex_release(struct inode *inode, struct file *filep)
{
	rtd_printk(KERN_ERR, TAG_NAME,"vpqex_release %d\n", __LINE__);
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

void vpqex_boot_init(void)
{
	Scaler_color_Set_HDR_AutoRun(TRUE);
	Scaler_color_Set_DCCProcess(TRUE);

	HAL_VPQ_MEMC_Initialize();
	Scaler_SetMemcLevel(TV001_LEVEL_AUTO);
}

long vpqex_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
	int ret = 0, i, j;

//	if (vpqex_ioctl_get_stop_run(cmd))
//		return 0;

//	if (vpqex_project_id != 0x00060000) {
//		if (VPQ_EXTERN_IOC_INIT == cmd) {
//			vpqex_project_id = 0x00060000;
//			vpqex_boot_init();
//		} else {
//			return -1;
//		}
//	}

//rtd_printk(KERN_ERR, TAG_NAME,"vpqex_ioctl %d\n", __LINE__);

	if (VPQ_EXTERN_IOC_INIT == cmd) {
		return 0;
	} else if (VPQ_EXTERN_IOC_UNINIT == cmd) {
		PQ_Dev_Extern_Status = PQ_DEV_EXTERN_UNINIT;
		return 0;
	} else if (VPQ_EXTERN_IOC_OPEN == cmd || VPQ_EXTERN_IOC_CLOSE == cmd) {
		return 0;
	}

//	if (PQ_Dev_Extern_Status != PQ_DEV_EXTERN_INIT_DONE)
//		return -1;

//rtd_printk(KERN_ERR, TAG_NAME,"vpqex_ioctl %d\n", __LINE__);
	
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
 *  TV010 External variables
 ************************************************************************/		
 		case VPQ_EXTERN_IOC_REGISTER_NEW_VIPTABLE:
		{
			ret = xRegisterTable();
			if (ret) {
				ret = 0;
				rtd_printk(KERN_INFO, TAG_NAME,"viptable xRegisterTable success\n");
			}
			else {
				ret = -1;
				rtd_printk(KERN_ERR, TAG_NAME,"viptable xRegisterTable fail\n");
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
				rtd_printk(KERN_INFO, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_VGA_ADCGainOffset success\n");
				ret = 0;
				#if 0
				rtd_printk(KERN_INFO, TAG_NAME,"m_vgaGainOffset={\n");
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x ", m_vgaGainOffset.Gain_R, m_vgaGainOffset.Gain_G, m_vgaGainOffset.Gain_B);
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");
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
				rtd_printk(KERN_INFO, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_YPBPR_ADCGainOffset success, sizeof(m_yPbPrGainOffsetData)=%d\n", sizeof(m_yPbPrGainOffsetData));
				ret = 0;
				#if 0
				rtd_printk(KERN_INFO, TAG_NAME,"m_yPbPrGainOffsetData[16]={\n");
				for (i=0; i<SR_MAX_YPBPR_GAINOFFSET_MODE; i++) {
					//for (j=0; j<VIP_QUALITY_FUNCTION_TOTAL_CHECK; j++) {
					rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x ", m_yPbPrGainOffsetData[i].Gain_R, m_yPbPrGainOffsetData[i].Gain_G, m_yPbPrGainOffsetData[i].Gain_B);
					//}
				rtd_printk(KERN_INFO, TAG_NAME,"\n");
				}
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");
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
				rtd_printk(KERN_INFO, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_SLR_VIP_TABLE_CUSTOM_TV001 success\n");
				ret = 0;
				#if 0
				rtd_printk(KERN_INFO, TAG_NAME,"m_pVipTableCustom->InitParam[16]={\n");
				for (i=0; i<16; i++) {
					//for (j=0; j<VIP_QUALITY_FUNCTION_TOTAL_CHECK; j++) {
					rtd_printk(KERN_INFO, TAG_NAME,"%x ", tmp_pVipTableCustom->InitParam[i]);
					//}
					//printk("\n");
				}
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");

				rtd_printk(KERN_INFO, TAG_NAME,"m_pVipTableCustom->DCR_TABLE[][][]={\n");
				for (i=0; i<DCR_TABLE_ROW_NUM; i++) {
					for (j=0; j<DCR_TABLE_COL_NUM; j++) {
					rtd_printk(KERN_INFO, TAG_NAME,"%x ", tmp_pVipTableCustom->DCR_TABLE[New_DCR_TABLE_NUM-1][i][j]);
					}
					rtd_printk(KERN_INFO, TAG_NAME,"\n");
				}
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_SET_SLR_VIP_TABLE:
		{

			// rtd_printk(KERN_INFO, TAG_NAME,"%s %d\n", __FUNCTION__, __LINE__);

			extern SLR_VIP_TABLE* m_pVipTable;
			#if 0
			rtd_printk(KERN_ERR, TAG_NAME,"original m_defaultVipTable.VIP_QUALITY_Coef[][]={\n");
				for (i=0; i<3; i++) {
					for (j=0; j<VIP_QUALITY_FUNCTION_TOTAL_CHECK; j++) {
					rtd_printk(KERN_ERR, TAG_NAME,"%x ", m_defaultVipTable.VIP_QUALITY_Coef[i][j]);
					}
				rtd_printk(KERN_ERR, TAG_NAME,"\n");
				}
			rtd_printk(KERN_ERR, TAG_NAME,"} end\n");

			rtd_printk(KERN_ERR, TAG_NAME,"m_defaultVipTable.DrvSetting_Skip_Flag[]={\n");
				for (i=0; i<DrvSetting_Skip_Flag_item_Max; i++) {
					//for (j=0; j<VIP_QUALITY_FUNCTION_TOTAL_CHECK; j++) {
					rtd_printk(KERN_ERR, TAG_NAME,"%x ", m_defaultVipTable.DrvSetting_Skip_Flag[i]);
					//}
					//printk("\n");
				}
			rtd_printk(KERN_ERR, TAG_NAME,"} end\n");
			#endif
			if (copy_from_user(m_pVipTable, (int __user *)arg, sizeof(SLR_VIP_TABLE))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy SLR_VIP_TABLE fail\n");
				// rtd_printk(KERN_ERR, TAG_NAME,"%s %d\n", __FUNCTION__, __LINE__);

				ret = -1;
				break;
			} else {
				// rtd_printk(KERN_INFO, TAG_NAME, "kernel copy SLR_VIP_TABLE success\n");
				// rtd_printk(KERN_INFO, TAG_NAME,"%s %d\n", __FUNCTION__, __LINE__);
				ret = 0;


				#if 0
				rtd_printk(KERN_INFO, TAG_NAME,"m_pVipTable->VIP_QUALITY_Coef[][]={\n");
				for (i=0; i<3; i++) {
					for (j=0; j<VIP_QUALITY_FUNCTION_TOTAL_CHECK; j++) {
					rtd_printk(KERN_INFO, TAG_NAME,"%x ", m_pVipTable->VIP_QUALITY_Coef[i][j]);
					}
					rtd_printk(KERN_INFO, TAG_NAME,"\n");
				}
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");

				rtd_printk(KERN_INFO, TAG_NAME,"m_pVipTable->DrvSetting_Skip_Flag[]={\n");
				for (i=0; i<DrvSetting_Skip_Flag_item_Max; i++) {
					//for (j=0; j<VIP_QUALITY_FUNCTION_TOTAL_CHECK; j++) {
					rtd_printk(KERN_INFO, TAG_NAME,"%x ", m_pVipTable->DrvSetting_Skip_Flag[i]);
					//}
					//printk("\n");
				}
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");
				#endif
			}
		}
		// rtd_printk(KERN_INFO, TAG_NAME,"%s %d\n", __FUNCTION__, __LINE__);
		break;

		case VPQ_EXTERN_IOC_SET_PIC_MODE_DATA:
		{
			extern PIC_MODE_DATA*  m_picModeTable;
			if (copy_from_user(m_picModeTable, (int __user *)arg, sizeof(PIC_MODE_DATA))) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_PIC_MODE_DATA fail\n");
				ret = -1;
				break;
			} else {
				// rtd_printk(KERN_INFO, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_PIC_MODE_DATA success\n");
				ret = 0;
				#if 0
			rtd_printk(KERN_INFO, TAG_NAME,"m_picModeTable={\n");
			rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x \n", m_picModeTable->picMode[PICTURE_MODE_USER].Brightness, m_picModeTable->picMode[PICTURE_MODE_USER].Contrast, m_picModeTable->picMode[PICTURE_MODE_USER].Saturation);
			rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x \n", m_picModeTable->picMode[PICTURE_MODE_VIVID].Brightness, m_picModeTable->picMode[PICTURE_MODE_VIVID].Contrast, m_picModeTable->picMode[PICTURE_MODE_VIVID].Saturation);
			rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x \n", m_picModeTable->picMode[PICTURE_MODE_STD].Brightness, m_picModeTable->picMode[PICTURE_MODE_STD].Contrast, m_picModeTable->picMode[PICTURE_MODE_STD].Saturation);
			rtd_printk(KERN_INFO, TAG_NAME,"} end\n");
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
				// rtd_printk(KERN_INFO, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_PIC_MODE_DATA_defaultPictureModeSet success\n");
				ret = 0;
				#if 0
				rtd_printk(KERN_INFO, TAG_NAME,"m_defaultPictureModeSet={\n");
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x \n", m_defaultPictureModeSet[PICTURE_MODE_USER].Brightness, m_defaultPictureModeSet[PICTURE_MODE_USER].Contrast, m_defaultPictureModeSet[PICTURE_MODE_USER].Saturation);
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x \n", m_defaultPictureModeSet[PICTURE_MODE_VIVID].Brightness, m_defaultPictureModeSet[PICTURE_MODE_VIVID].Contrast, m_defaultPictureModeSet[PICTURE_MODE_VIVID].Saturation);
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x \n", m_defaultPictureModeSet[PICTURE_MODE_GAME].Brightness, m_defaultPictureModeSet[PICTURE_MODE_GAME].Contrast, m_defaultPictureModeSet[PICTURE_MODE_GAME].Saturation);
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");
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
				// rtd_printk(KERN_INFO, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_PIC_MODE_DATA_DolbyHDRPictureModeSet success\n");
				ret = 0;
				#if 0
				rtd_printk(KERN_INFO, TAG_NAME,"m_DolbyHDRPictureModeSet={\n");
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x \n", m_DolbyHDRPictureModeSet[PICTURE_MODE_USER].Brightness, m_DolbyHDRPictureModeSet[PICTURE_MODE_USER].Contrast, m_DolbyHDRPictureModeSet[PICTURE_MODE_USER].Saturation);
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x \n", m_DolbyHDRPictureModeSet[PICTURE_MODE_VIVID].Brightness, m_DolbyHDRPictureModeSet[PICTURE_MODE_VIVID].Contrast, m_DolbyHDRPictureModeSet[PICTURE_MODE_VIVID].Saturation);
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x \n", m_DolbyHDRPictureModeSet[PICTURE_MODE_STD].Brightness, m_DolbyHDRPictureModeSet[PICTURE_MODE_STD].Contrast, m_DolbyHDRPictureModeSet[PICTURE_MODE_STD].Saturation);
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");
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
				// rtd_printk(KERN_INFO, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataType success\n");
				ret = 0;
				#if 0
				rtd_printk(KERN_INFO, TAG_NAME,"m_colorDataTable={\n");
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x ", m_colorDataTable->MBPeaking, m_colorDataTable->Intensity, m_colorDataTable->Gamma);
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");
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
				// rtd_printk(KERN_INFO, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataType_defaultSDColorData success\n");
				ret = 0;
				#if 0
				rtd_printk(KERN_INFO, TAG_NAME,"m_defaultSDColorData={\n");
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x ", m_defaultSDColorData.MBPeaking, m_defaultSDColorData.Intensity, m_defaultSDColorData.Gamma);
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");
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
				// rtd_printk(KERN_INFO, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataType_defaultHDColorData success\n");
				ret = 0;
				#if 0
				rtd_printk(KERN_INFO, TAG_NAME,"m_defaultHDColorData={\n");
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x ", m_defaultHDColorData.MBPeaking, m_defaultHDColorData.Intensity, m_defaultHDColorData.Gamma);
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");
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
				// rtd_printk(KERN_INFO, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_COLOR_TEMP_DATA success\n");
				ret = 0;
				#if 0
				rtd_printk(KERN_INFO, TAG_NAME,"m_colorTempData={\n");
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x \n", m_colorTempData->colorTempData[SLR_COLORTEMP_USER].R_val, m_colorTempData->colorTempData[SLR_COLORTEMP_USER].G_val, m_colorTempData->colorTempData[SLR_COLORTEMP_USER].B_val);
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x \n", m_colorTempData->colorTempData[SLR_COLORTEMP_NORMAL].R_val, m_colorTempData->colorTempData[SLR_COLORTEMP_NORMAL].G_val, m_colorTempData->colorTempData[SLR_COLORTEMP_NORMAL].B_val);
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x \n", m_colorTempData->colorTempData[SLR_COLORTEMP_WARMER].R_val, m_colorTempData->colorTempData[SLR_COLORTEMP_WARMER].G_val, m_colorTempData->colorTempData[SLR_COLORTEMP_WARMER].B_val);
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");
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
				// rtd_printk(KERN_INFO, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_COLOR_TEMP_DATA_defColorTempSet success\n");
				ret = 0;
				#if 0
				rtd_printk(KERN_INFO, TAG_NAME,"m_defColorTempSet={\n");
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x \n", m_defColorTempSet[SLR_COLORTEMP_USER].R_val, m_defColorTempSet[SLR_COLORTEMP_USER].G_val, m_defColorTempSet[SLR_COLORTEMP_USER].B_val);
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x \n", m_defColorTempSet[SLR_COLORTEMP_NORMAL].R_val, m_defColorTempSet[SLR_COLORTEMP_NORMAL].G_val, m_defColorTempSet[SLR_COLORTEMP_NORMAL].B_val);
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x \n", m_defColorTempSet[SLR_COLORTEMP_WARMER].R_val, m_defColorTempSet[SLR_COLORTEMP_WARMER].G_val, m_defColorTempSet[SLR_COLORTEMP_WARMER].B_val);
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");
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
				// rtd_printk(KERN_INFO, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType success\n");
				ret = 0;
				#if 0
				rtd_printk(KERN_INFO, TAG_NAME,"m_colorFacModeTable={\n");
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x ", m_colorFacModeTable->Brightness_0, m_colorFacModeTable->Brightness_50, m_colorFacModeTable->Brightness_100);
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");
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
				// rtd_printk(KERN_INFO, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_defaultColorFacMode success\n");
				ret = 0;
				#if 0
				rtd_printk(KERN_INFO, TAG_NAME,"m_defaultColorFacMode={\n");
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x ", m_defaultColorFacMode.Brightness_0, m_defaultColorFacMode.Brightness_50, m_defaultColorFacMode.Brightness_100);
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");
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
				// rtd_printk(KERN_INFO, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_AvColorFacMode success\n");
				ret = 0;
				#if 0
				rtd_printk(KERN_INFO, TAG_NAME,"m_AvColorFacMode={\n");
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x ", m_AvColorFacMode.Brightness_0, m_AvColorFacMode.Brightness_50, m_AvColorFacMode.Brightness_100);
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");
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
				// rtd_printk(KERN_INFO, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_YppColorFacMode success\n");
				ret = 0;
				#if 0
				rtd_printk(KERN_INFO, TAG_NAME,"m_YppColorFacMode={\n");
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x ", m_YppColorFacMode.Brightness_0, m_YppColorFacMode.Brightness_50, m_YppColorFacMode.Brightness_100);
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");
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
				// rtd_printk(KERN_INFO, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_VgaColorFacMode success\n");
				ret = 0;
				#if 0
				rtd_printk(KERN_INFO, TAG_NAME,"m_VgaColorFacMode={\n");
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x ", m_VgaColorFacMode.Brightness_0, m_VgaColorFacMode.Brightness_50, m_VgaColorFacMode.Brightness_100);
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");
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
				// rtd_printk(KERN_INFO, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_HdmiColorFacMode success\n");
				ret = 0;
				#if 0
				rtd_printk(KERN_INFO, TAG_NAME,"m_HdmiColorFacMode={\n");
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x ", m_HdmiColorFacMode.Brightness_0, m_HdmiColorFacMode.Brightness_50, m_HdmiColorFacMode.Brightness_100);
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");
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
				// rtd_printk(KERN_INFO, TAG_NAME, "kernel copy VPQ_EXTERN_IOC_SET_StructColorDataFacModeType_HdmiSDColorFacMode success\n");
				ret = 0;
				#if 0
				rtd_printk(KERN_INFO, TAG_NAME,"m_HdmiSDColorFacMode={\n");
				rtd_printk(KERN_INFO, TAG_NAME,"%x %x %x ", m_HdmiSDColorFacMode.Brightness_0, m_HdmiSDColorFacMode.Brightness_50, m_HdmiSDColorFacMode.Brightness_100);
				rtd_printk(KERN_INFO, TAG_NAME,"} end\n");
				#endif
			}
		}
		break;

		case VPQ_EXTERN_IOC_GET_BRIGHTNESS:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_BRIGHTNESS\n");
			unsigned int brightness = 0;
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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_BRIGHTNESS\n");
			unsigned int args = 0;
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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_CONTRAST\n");
			unsigned int contrast = 0;
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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_CONTRAST\n");
			unsigned int args = 0;
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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_SATURATION\n");
			unsigned int saturation = 0;
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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_SATURATION\n");
			unsigned int args = 0;
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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_HUE\n");
			unsigned int hue = 0;
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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_HUE\n");
			unsigned int args = 0;
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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_NR\n");
			unsigned int nr = 0;
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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_NR\n");
			unsigned int args = 0;
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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_SHARPNESS\n");
			unsigned int sharpness = 0;
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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_SHARPNESS_EXTERN\n");
			unsigned int args = 0;
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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_DE_BLOCKING\n");
			TV001_LEVEL_E dbLevel = 0;
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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_DE_BLOCKING\n");
			unsigned int args = 0;
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {
				
				ret = -1;
			} else {
				ret = 0;
				//Scaler_SetMPEGNR((unsigned char)args,0);
				Scaler_SetMPEGNR(TV001_LEVEL_AUTO,0);  // default auto
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_DE_RINGING:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_DE_RINGING\n");
			TV001_LEVEL_E drLevel = 0;

			if (Scaler_GetDeRinging_level(&drLevel) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetDeRinging_level fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&drLevel, sizeof(TV001_LEVEL_E))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_DE_RINGING fail\n");
					ret = -1;
				} else
					ret = 0;
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_DE_RINGING:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_DE_RINGING\n");
			unsigned int args = 0;
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {
				
				ret = -1;
			} else {
				ret = 0;
				Scaler_SetDeRinging_level((TV001_LEVEL_E)args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_COLOR_TEMP:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_COLOR_TEMP\n");
			TV001_COLORTEMP_E colorTemp = 0;

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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_COLOR_TEMP_EXTERN\n");
			TV001_COLOR_TEMP_DATA_S args;
			if (copy_from_user(&args,  (int __user *)arg,sizeof(TV001_COLOR_TEMP_DATA_S))) {
				
				ret = -1;
			} else {
				ret = 0;
				//Scaler_SetColorTemp_level_type((TV001_COLORTEMP_E)args);
				Scaler_SetColorTempData((TV001_COLOR_TEMP_DATA_S*) &args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_COLOR_GAIN:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_COLOR_GAIN\n");
			TV001_LEVEL_E colorGain = 0;

			if (Scaler_GetColorGainLevel(&colorGain) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetColorGainLevel fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&colorGain, sizeof(TV001_LEVEL_E))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_COLOR_GAIN fail\n");
					ret = -1;
				} else
					ret = 0;
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_COLOR_GAIN_EXTERN:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_COLOR_GAIN_EXTERN\n");
			unsigned int args = 0;
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {
				
				ret = -1;
			} else {
				ret = 0;
				Scaler_SetColorGainLevel(args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_FLESH_TONE:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_FLESH_TONE\n");
			TV001_LEVEL_E fleshtone = 0;

			if (Scaler_GetFleshToneLevel(&fleshtone) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetFleshToneLevel fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&fleshtone, sizeof(TV001_LEVEL_E))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_FLESH_TONE fail\n");
					ret = -1;
				} else
					ret = 0;
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_FLESH_TONE:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_FLESH_TONE\n");
			unsigned int args = 0;
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {
				
				ret = -1;
			} else {
				ret = 0;
				Scaler_SetFleshToneLevel((TV001_LEVEL_E)args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_BACKLIGHT:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_BACKLIGHT\n");
			unsigned int backlight = 0;
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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_BACKLIGHT\n");
			unsigned int args = 0;
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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_PANEL_BL\n");
		}
		break;
		case VPQ_EXTERN_IOC_GET_GAMMA_NUM:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_GAMMA_NUM\n");
			unsigned int number = 0;

			if (Scaler_GetGammaNum(&number) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetGammaNum fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&number, sizeof(unsigned int))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_GAMMA_NUM fail\n");
					ret = -1;
				} else
					ret = 0;
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_GAMMA:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_GAMMA\n");
			unsigned int args = 0;
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {
				
				ret = -1;
			} else {
				ret = 0;
				Scaler_SetGAMMAMapping_tv010((unsigned char)args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_PQ_MODULE:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_PQ_MODULE\n");
			RTK_PQModule_T PQModule_T;
			if (copy_from_user(&PQModule_T,  (int __user *)arg,sizeof(RTK_PQModule_T))) {
				
				ret = -1;
			} else {
				ret = 0;
				if (Scaler_GetPQModule((TV010_PQ_MODULE_E) PQModule_T.PQModule,(unsigned char *) &PQModule_T.onOff) == FALSE) {
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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_PQ_MODULE\n");
			RTK_PQModule_T PQModule_T;
			//unsigned int args = 0;
			if (copy_from_user(&PQModule_T,  (int __user *)arg,sizeof(RTK_PQModule_T))) {
				
				ret = -1;
			} else {
				ret = 0;
				Scaler_SetPQModule((TV010_PQ_MODULE_E) PQModule_T.PQModule,(unsigned char)PQModule_T.onOff);
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_DEMO:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_DEMO\n");
			RTK_DEMO_MODE_T DEMO_MODE_T;
			//unsigned int args = 0;
			if (copy_from_user(&DEMO_MODE_T,  (int __user *)arg,sizeof(RTK_DEMO_MODE_T))) {
				
				ret = -1;
			} else {
				ret = 0;
				Scaler_SetDemoMode((TV010_DEMO_MODE_E) DEMO_MODE_T.DEMO_MODE,(unsigned char)DEMO_MODE_T.onOff);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_DEBUG_MODE:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_DEBUG_MODE\n");
		}
		break;
		case VPQ_EXTERN_IOC_SET_DEBUG_MODE:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_DEBUG_MODE\n");
		}
		break;
		case VPQ_EXTERN_IOC_GET_HISTOGRAM:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_HISTOGRAM\n");
			
			UINT32 HistoData_bin[TV006_VPQ_chrm_bin];

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
		case VPQ_EXTERN_IOC_GET_SR_LEVEL:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_SR_LEVEL\n");
			TV001_LEVEL_E srLevel = 0;

			if (Scaler_GetSRLevel(&srLevel) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetSRLevel fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&srLevel, sizeof(TV001_LEVEL_E))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_SR_LEVEL fail\n");
					ret = -1;
				} else
					ret = 0;
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_SR_LEVEL:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_SR_LEVEL\n");
			unsigned int args = 0;
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {
				
				ret = -1;
			} else {
				ret = 0;
				Scaler_SetSRLevel((TV010_DEMOLEVEL_E)args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_BLACK_PATTERN_OUTPUT:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_BLACK_PATTERN_OUTPUT\n");
			unsigned int args = 0;
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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_WHITE_PATTERN_OUTPUT\n");
			unsigned int args = 0;
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {
				
				ret = -1;
			} else {
				ret = 0;
				Scaler_SetWhitePatternOutput((unsigned char)args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_OD_LEVEL:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_OD_LEVEL\n");
			unsigned int level = 0;

			if (Scaler_GetODLevel(&level) == FALSE) {
				rtd_printk(KERN_ERR, TAG_NAME, "kernel Scaler_GetODLevel fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&level, sizeof(unsigned int))) {
					rtd_printk(KERN_ERR, TAG_NAME, "kernel VPQ_EXTERN_IOC_GET_OD_LEVEL fail\n");
					ret = -1;
				} else
					ret = 0;
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_OD_READ_ENABLE:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_OD_READ_ENABLE\n");
			unsigned int args = 0;
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {
				
				ret = -1;
			} else {
				ret = 0;
				Scaler_SetODReadEnable((unsigned char)args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_SRC_HDR_INFO:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_SRC_HDR_INFO\n");
			unsigned int GammaType = 0;

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
			rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_SDR2HDR\n");
			unsigned char onoff = 0;

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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_SDR2HDR\n");
			unsigned int args = 0;
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
			TV001_HDR_TYPE_E HdrType = 0;

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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_DCI_TYPE\n");
			unsigned int args = 0;
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {
				
				ret = -1;
			} else {
				ret = 0;
				//Scaler_SetDCC_Table((unsigned char)args);
				// rtd_printk(KERN_INFO,TAG_NAME,"=DCCDebug=file:%s=line:%d=args:%d==\n",__FILE__,__LINE__,(unsigned char)args);
				//Scaler_SetDCCMapping_tv010((unsigned char)args);
				Scaler_SetDCC_Table_tv010((unsigned char)args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_GET_DOLBY_HDR_PIC_MODE:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_DOLBY_HDR_PIC_MODE\n");
		}
		break;
		case VPQ_EXTERN_IOC_SET_DOLBY_HDR_PIC_MODE:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_DOLBY_HDR_PIC_MODE\n");
		}
		break;
		case VPQ_EXTERN_IOC_GET_HDR10_ENABLE:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_HDR10_ENABLE\n");
			unsigned char bEnable = 0;

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
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_HDR10_ENABLE\n");
			unsigned int args = 0;
			if (copy_from_user(&args,  (int __user *)arg,sizeof(unsigned int))) {
				
				ret = -1;
			} else {
				ret = 0;
				Scaler_SetHdr10Enable((unsigned char)args);
			}
		}
		break;
		case VPQ_EXTERN_IOC_SET_DREAM_PANEL_LOG_ENABLE:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_DREAM_PANEL_LOG_ENABLE\n");
		}
		break;
		case VPQ_EXTERN_IOC_GET_DREAM_PANEL_BACKLIGHT:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_GET_DREAM_PANEL_BACKLIGHT\n");
		}
		break;
		case VPQ_EXTERN_IOC_SET_DREAM_PANEL_BACKLIGHT:
		{
			// rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC_SET_DREAM_PANEL_BACKLIGHT\n");
		}
		break;

		case VPQ_EXTERN_IOC_SET_Scaler_SetDCC_Mode:
		{
			unsigned char mode = 0;
			if(copy_from_user(&mode,(int __user *)arg,sizeof(unsigned char)))
			{
				rtd_printk(KERN_ERR, TAG_NAME,"error !!!!! Scaler_SetDCC_Mode= %d \n",mode);
				ret = -1;
			}else{
				// rtd_printk(KERN_INFO, TAG_NAME, "Scaler_SetDCC_Mode= %d \n",mode);
				ret = 0;
				Scaler_SetDCC_Mode(mode);
			}
		}
		break;

		case VPQ_EXTERN_IOC_GET_NOTIFY_PLATFORM_COLOT_TEMP:
		{
		       unsigned char mode;
			 if(copy_from_user(&mode,(void __user *)arg,sizeof(unsigned char)))
			 {
			  	rtd_printk(KERN_EMERG, TAG_NAME,"error: VPQ_EXTERN_IOC_GET_NOTIFY_PLATFORM_COLOT_TEMP \n");
                       	ret = -1;
                      }else{
                      	rtd_printk(KERN_INFO, TAG_NAME," VPQ_EXTERN_IOC_GET_NOTIFY_PLATFORM_COLOT_TEMP mode=%d \n",mode);
				Scaler_SetCOLORTEMPMapping_tv010(mode);
                            ret = 0;
                    }
              }
               break;
		
	default:
		rtd_printk(KERN_INFO, TAG_NAME, "kernel IO command %d not found!\n", cmd);
		rtd_printk(KERN_INFO, TAG_NAME,"VPQ_EXTERN_IOC cmd=0x%x unknown\n", cmd);
		return -1;
			;
	}
	return ret;/*Success*/

}


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
	rtd_printk(KERN_INFO, TAG_NAME,"vpqex_module_init %d\n", __LINE__);

	//init project
	vpqex_project_id = 0x00100000;
	
	int result;
	dev_t devno = 0;//vbe device number
	result = alloc_chrdev_region(&vpqex_devno, vpqex_minor, vpqex_nr_devs, VPQ_EXTERN_DEVICE_NAME);
	vpqex_major = MAJOR(vpqex_devno);
	if (result != 0) {
		rtd_printk(KERN_ERR, TAG_NAME, "Cannot allocate VPQEX device number\n");
		rtd_printk(KERN_ERR, TAG_NAME,"vpqex_module_init %d\n", __LINE__);
		return result;
	}

	rtd_printk(KERN_INFO, TAG_NAME,"vpqex_module_init %d\n", __LINE__);

	rtd_printk(KERN_INFO, TAG_NAME,"VPQEX_DEVICE init module major number = %d\n", vpqex_major);
	//vpqex_devno = MKDEV(vpqex_major, vpqex_minor);
	
	vpqex_class = class_create(THIS_MODULE, VPQ_EXTERN_DEVICE_NAME);
	if (IS_ERR(vpqex_class)) {
		rtd_printk(KERN_ERR, TAG_NAME, "scalevpqex: can not create class...\n");
		rtd_printk(KERN_ERR, TAG_NAME,"vpqex_module_init %d\n", __LINE__);
		result = PTR_ERR(vpqex_class);
		goto fail_class_create;
	}

	vpqex_class->devnode = vpqex_devnode;

	vpqex_platform_devs = platform_device_register_simple(VPQ_EXTERN_DEVICE_NAME, -1, NULL, 0);
	if (platform_driver_register(&vpqex_platform_driver) != 0) {
		rtd_printk(KERN_ERR, TAG_NAME, "scalevpqex: can not register platform driver...\n");
		rtd_printk(KERN_ERR, TAG_NAME,"vpqex_module_init %d\n", __LINE__);
		result = -ENOMEM;
		goto fail_platform_driver_register;
	}

	cdev_init(&vpqex_cdev, &vpqex_fops);
	vpqex_cdev.owner = THIS_MODULE;
   	vpqex_cdev.ops = &vpqex_fops;
	result = cdev_add(&vpqex_cdev, vpqex_devno, 1);
	if (result < 0) {
		rtd_printk(KERN_ERR, TAG_NAME, "scalevpqex: can not add character device...\n");
		rtd_printk(KERN_ERR, TAG_NAME,"vpqex_module_init %d\n", __LINE__);
		goto fail_cdev_init;
	}

	device_create(vpqex_class, NULL, vpqex_devno, NULL, VPQ_EXTERN_DEVICE_NAME);

//	sema_init(&VPQ_EXTERN_Semaphore, 1);

	if (0x00100000 == vpqex_project_id) {
		vpqex_boot_init();
	}

	rtd_printk(KERN_INFO, TAG_NAME,"vpqex_module_init %d\n", __LINE__);
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
	rtd_printk(KERN_ERR, TAG_NAME,"vpqex_module_init %d\n", __LINE__);
	return result;

}



void __exit vpqex_module_exit(void)
{
	rtd_printk(KERN_INFO, TAG_NAME,"vpqex_module_exit\n");
	
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