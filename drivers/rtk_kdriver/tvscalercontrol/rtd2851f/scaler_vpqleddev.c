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
#include <asm-generic/ioctl.h>

/*RBUS Header file*/
#include <scaler/scalerCommon.h>

/*TVScaler Header file*/
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/vip/viptable.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>
#include <tvscalercontrol/vip/scalerColor_tv043.h>
#include "scaler_vscdev.h"

#include "scaler_vpqleddev.h"
#include "ioctrl/vpq/vpq_led_cmd_id.h"

#include <rtd_log/rtd_module_log.h>


#ifndef UT_flag
static dev_t vpq_led_devno;/*vpq device number*/
static struct cdev vpq_led_cdev;
#endif

static unsigned char PQ_LED_Dev_Status = PQ_LED_DEV_NOTHING;

struct semaphore VPQ_ld_running_Semaphore;/*For adaptive streaming info Semaphore*/
static struct semaphore VPQ_LED_Semaphore;

unsigned char vpq_ld_running_flag = FALSE;//Run memc mode
unsigned char vpq_led_LDEnable = 0;//Run memc mode
unsigned char vpq_led_LD_lutTableIndex = 0;
unsigned char vpq_led_LocalDimmingDemoCtrlMode = FALSE;
VPQLED_LD_LDSPI_DATASRAM_TYPE tVPQLED_LD_LDSPI_DataSRAM; 

struct semaphore *Get_Val_VPQ_ld_running_Semaphore(void)
{//VPQ_ld_running_Semaphore
	return &VPQ_ld_running_Semaphore;
}

unsigned char Get_Val_vpq_ld_running_flag(void)
{//vpq_ld_running_flag
	return vpq_ld_running_flag;
}

void Set_Val_vpq_ld_running_flag(unsigned char flag)
{//vpq_ld_running_flag
	vpq_ld_running_flag = flag;
}

unsigned char Get_Val_vpq_led_LDEnable(void)
{//vpq_led_LDEnable
	return vpq_led_LDEnable;
}

void Set_Val_vpq_vpq_led_LDEnable(unsigned char value)
{//vpq_led_LDEnable
	vpq_led_LDEnable = value;
}

unsigned char Get_Var_PQ_LED_Dev_Status(void) 
{
	return PQ_LED_Dev_Status;
}

void Set_Var_PQ_LED_Dev_Status(unsigned char value) 
{
	PQ_LED_Dev_Status = value;
}


KADP_LED_DB_LUT_T LED_DB_LUT[7];
#ifndef UT_flag

int vpq_led_open(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t  vpq_led_read(struct file *filep, char *buffer, size_t count, loff_t *offp)
{
	return 0;

}

ssize_t vpq_led_write(struct file *filep, const char *buffer, size_t count, loff_t *offp)
{
	return 0;
}

int vpq_led_release(struct inode *inode, struct file *filep)
{
	return 0;
}
#endif

unsigned char led_ioctl_cmd_stop[16] = {0};
unsigned char vpq_led_ioctl_get_stop_run(unsigned int cmd)
{
	return (led_ioctl_cmd_stop[_IOC_NR(cmd)&0xf]|led_ioctl_cmd_stop[0]);
}

unsigned char vpq_led_ioctl_get_stop_run_by_idx(unsigned char cmd_idx)
{
	return led_ioctl_cmd_stop[cmd_idx];
}

void vpq_led_ioctl_set_stop_run_by_idx(unsigned char cmd_idx, unsigned char stop)
{
	led_ioctl_cmd_stop[cmd_idx] = stop;
}



long vpq_led_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
	int ret = 0;
	static HAL_LED_PANEL_INFO_T PANEL_INFO_T = { 0 };
	unsigned char src_idx = 0;
	unsigned char TableIdx = 0;
	if (vpq_led_ioctl_get_stop_run(cmd))
		return 0;

	switch (cmd) {
	case VPQ_LED_IOC_INIT:
		if (PQ_LED_DEV_UNINIT == PQ_LED_Dev_Status || PQ_LED_DEV_NOTHING == PQ_LED_Dev_Status) {
			PQ_LED_Dev_Status = PQ_LED_DEV_INIT_DONE;
		} else
			return -1;
		ret = 0;
	break;

	case VPQ_LED_IOC_UNINIT:
		if (PQ_LED_DEV_INIT_DONE == PQ_LED_Dev_Status) {
			PQ_LED_Dev_Status = PQ_LED_DEV_UNINIT;
		} else
			return -1;
		break;

	case VPQ_LED_IOC_SET_LD_INIT:
		{


			if (PQ_LED_DEV_UNINIT == PQ_LED_Dev_Status || PQ_LED_DEV_NOTHING == PQ_LED_Dev_Status) {
				PQ_LED_Dev_Status = PQ_LED_DEV_INIT_DONE;
			}


			if (copy_from_user(&PANEL_INFO_T, (int __user *)arg, sizeof(HAL_LED_PANEL_INFO_T))) {
				rtd_pr_vpq_err("kernel VPQ_LED_IOC_SET_LD_INIT fail\n");
				ret = -1;
			} else {
				if(PANEL_INFO_T.hal_inch == 255 && PANEL_INFO_T.hal_bl_type == 255 && PANEL_INFO_T.hal_bar_type == 255 && PANEL_INFO_T.hal_maker == 255){
					rtd_pr_vpq_debug("kernel VPQ_LED_IOC_SET_LD_INIT success\n");
					ret = 0;
				}else{
					/*LD table initialization*/
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
					if (PANEL_INFO_T.hal_bl_type == HAL_BL_DIRECT_L || PANEL_INFO_T.hal_bl_type == HAL_BL_DIRECT_M)
						fwif_color_set_LD_BL_Profile_Interpolation_Table_HV(src_idx, TableIdx); /*Direct Type Light Profile Table*/
					else
						fwif_color_set_LD_BL_Profile_Interpolation_Table(src_idx, TableIdx); /*Edge Typp Light Profile Table*/
					fwif_color_set_LD_Demo_Window(src_idx, TableIdx);
					fwif_color_set_LED_Initialize(PANEL_INFO_T);
				
					/*LDSPI initialization*/
					fwif_color_set_LD_CtrlSPI_init(PANEL_INFO_T); 

					/*LD & LDSPI ISR*/
					fwif_color_set_LD_hw_blight_end_ie2_en(false); /*enable after real chip test*/
					fwif_color_set_LDSPI_TXDone_ISR_En(false); /*enable after real chip test*/
					//fwif_color_set_LD_To_LDSPI_Method_tv043(MOVE_LD_TO_LDSPI_BYSW_120HZ);
					fwif_color_Set_LD_Init_Done(true);
					rtd_pr_vpq_debug("kernel VPQ_LED_IOC_SET_LD_INIT success\n");
					ret = 0;
				}
			}
		}
		break;

	case VPQ_LED_IOC_SET_LDEnable:
		{
			unsigned char bCtrl;

			if (PQ_LED_Dev_Status != PQ_LED_DEV_INIT_DONE)
				return -1;

			if (copy_from_user(&bCtrl, (int __user *)arg, sizeof(unsigned char))) {
				rtd_pr_vpq_err("kernel VPQ_LED_LDEnable fail\n");
				ret = -1;
			} else {

				drvif_HAL_VPQ_LED_LDEnable(bCtrl);
/*				down(&VPQ_ld_running_Semaphore);
				vpq_ld_running_flag = TRUE;
				vpq_led_LDEnable = bCtrl;
				up(&VPQ_ld_running_Semaphore);*/

				rtd_pr_vpq_debug("kernel VPQ_LED_LDEnable success\n");
				ret = 0;
			}
		}
		break;

	case VPQ_LED_IOC_SET_LDSetLUT:
		{
			if (PQ_LED_Dev_Status != PQ_LED_DEV_INIT_DONE)
				return -1;

			if (copy_from_user(&vpq_led_LD_lutTableIndex, (int __user *)arg, sizeof(UINT8))) {
				rtd_pr_vpq_err("kernel VPQ_LED_LDSetLUT fail\n");
				ret = -1;
			} else {
				if (vpq_led_LocalDimmingDemoCtrlMode == FALSE) /* demo mode, record index and do not apply*/
					fwif_color_set_LDSetLUT(vpq_led_LD_lutTableIndex);
				rtd_pr_vpq_debug("kernel VPQ_LED_LDSetLUT success\n");
				ret = 0;
			}

		}
		break;

	case VPQ_LED_IOC_LDSetDBLUT:
		{



			if (PQ_LED_Dev_Status != PQ_LED_DEV_INIT_DONE)
				return -1;

			printk( "ADD = %p,arg=%lu, size=%zu", &LED_DB_LUT[0], arg, sizeof(KADP_LED_DB_LUT_T));


			if (copy_from_user(&LED_DB_LUT[0], (int __user *)arg, sizeof(LED_DB_LUT))) {
				rtd_pr_vpq_err("kernel VPQ_LED_IOC_LDSetDBLUT fail\n");
				ret = -1;
			} else {
				fwif_color_set_LDSetDBLUT_TV006(&LED_DB_LUT[0]);
				rtd_pr_vpq_debug("kernel VPQ_LED_IOC_LDSetDBLUT success\n");
				ret = 0;
			}


		}
		break;

	case VPQ_LED_IOC_SET_LDEnablePixelCompensation:
		{
			unsigned char bCtrl;

			if (PQ_LED_Dev_Status != PQ_LED_DEV_INIT_DONE)
				return -1;

			if (copy_from_user(&bCtrl, (int __user *)arg, sizeof(unsigned char))) {
				rtd_pr_vpq_err("kernel VPQ_LED_LDEnablePixelCompensation fail\n");
				ret = -1;
			} else {
				drvif_color_set_LD_Data_Compensation_Enable(bCtrl);
				rtd_pr_vpq_debug("kernel VPQ_LED_LDEnablePixelCompensation success\n");
				ret = 0;
			}

		}
		break;

	case VPQ_LED_IOC_SET_LDCtrlDemoMode:
		{
			DRV_HAL_VPQ_LED_LDCtrlDemoMode LED_LDCtrlDemoMode;

			if (PQ_LED_Dev_Status != PQ_LED_DEV_INIT_DONE)
				return -1;

			if (copy_from_user(&LED_LDCtrlDemoMode, (int __user *)arg, sizeof(DRV_HAL_VPQ_LED_LDCtrlDemoMode))) {
				rtd_pr_vpq_err("kernel VPQ_LED_LDCtrlDemoMode fail\n");
				ret = -1;
			} else {

				vpqled_HAL_VPQ_LED_LDCtrlDemoMode(LED_LDCtrlDemoMode.ctrlMode, LED_LDCtrlDemoMode.bCtrl);
				rtd_pr_vpq_notice("kernel VPQ_LED_LDCtrlDemoMode success\n");
				ret = 0;
			}

		}
		break;

	case VPQ_LED_IOC_SET_LDCtrlSPI:
		{
			unsigned char LDCtrlSPI[2];

			if (PQ_LED_Dev_Status != PQ_LED_DEV_INIT_DONE)
				return -1;

			if (copy_from_user(&LDCtrlSPI[0], (int __user *)arg, sizeof(unsigned char)*2)) {
				rtd_pr_vpq_err("kernel VPQ_LED_IOC_SET_LDCtrlSPI fail\n");
				ret = -1;
			} else {

				vpqled_HAL_VPQ_LED_LDCtrlSPI(&LDCtrlSPI[0]);

				rtd_pr_vpq_debug("kernel VPQ_LED_IOC_SET_LDCtrlSPI success\n");
				ret = 0;
			}

		}
		break;
	case VPQ_LED_IOC_LDGetAPL:
		{
			unsigned short BLValue[2];
			unsigned char suc_flag;

			if (fwif_color_vpq_stop_ioctl(STOP_VPQ_LED_LDGetAPL, ACCESS_MODE_GET, 0))
				return 0;

			if (PQ_LED_Dev_Status != PQ_LED_DEV_INIT_DONE)
				return -1;

			suc_flag = vpqled_get_LD_GetAPL_TV006(&(BLValue[0]));

			if (suc_flag == FALSE) {
				rtd_pr_vpq_err("kernel VPQ_LED_LDGetAPL fail\n");
				ret = -1;
			} else {
				if (copy_to_user((void __user *)arg, (void *)&BLValue[0], sizeof(UINT16)*2)) {
					rtd_pr_vpq_err("kernel VPQ_LED_LDGetAPL fail\n");
					ret = -1;
				} else
					ret = 0;
			}


		}
		break;

	case VPQ_LED_IOC_SET_SLR_VIP_TABLE_LD:
		{
			
			extern DRV_Local_Dimming_Table Local_Dimming_Table[LD_Table_NUM];			
			rtd_pr_vpq_info("VPQ_LED_IOC_SET_SLR_VIP_TABLE_LD\n");
			if (copy_from_user(&Local_Dimming_Table[0],  (int __user *)arg,sizeof(Local_Dimming_Table))) {
				ret = -1;
			} else {

				rtd_pr_vpq_info("Table0-MaxGain=%d\n",Local_Dimming_Table[0].LD_Backlight_Decision.ld_maxgain);
				/*LD table initialization*/
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
				if (PANEL_INFO_T.hal_bl_type == HAL_BL_DIRECT_L || PANEL_INFO_T.hal_bl_type == HAL_BL_DIRECT_M)
					fwif_color_set_LD_BL_Profile_Interpolation_Table_HV(src_idx, TableIdx); /*Direct Type Light Profile Table*/
				else
					fwif_color_set_LD_BL_Profile_Interpolation_Table(src_idx, TableIdx); /*Edge Typp Light Profile Table*/
				fwif_color_set_LD_Demo_Window(src_idx, TableIdx);
				fwif_color_set_LED_Initialize(PANEL_INFO_T);

				rtd_pr_vpq_debug("kernel VPQ_LED_IOC_SET_LD_INIT success\n");
				ret = 0;				

			}
		}
		break;

       case VPQ_LED_IOC_SET_SLR_VIP_TABLE_LDSPI:
		{

			extern DRV_LDINTERFACE_Table LDSPI_Table[LDInterface_Table_Num];
			rtd_pr_vpq_info("VPQ_LED_IOC_SET_SLR_VIP_TABLE_LDSPI\n");
			if (copy_from_user(&LDSPI_Table[0],  (int __user *)arg,sizeof(LDSPI_Table))) {
					ret = -1;
			} else {

					rtd_pr_vpq_info("LDSPI_Table[0][0][0]=%d\n",LDSPI_Table[0].LDINTERFACE_Basic_Ctrl.ld_mode);
					fwif_set_LdInterface_Table(src_idx, TableIdx);
					/*LDSPI initialization*/
					fwif_color_set_LDInterface_CtrlSPI_init(src_idx, TableIdx);

					/*LD & LDSPI ISR*/
					fwif_color_set_LD_hw_blight_end_ie2_en(false); /*enable after real chip test*/
					fwif_color_set_LDSPI_TXDone_ISR_En(false); /*enable after real chip test*/
					fwif_color_Set_LD_Init_Done(true);
					rtd_pr_vpq_debug("kernel VPQ_LED_IOC_SET_LD_INIT success\n");
					ret = 0;	
			}
		}
		break;


	case VPQ_LED_IOC_SET_SLR_VIP_TABLE_LDCOMP:
		{
			
			extern DRV_LD_Data_Compensation_NewMode_2DTable LD_Data_Compensation_NewMode_2DTable[LD_Table_NUM];			
			rtd_pr_vpq_info("VPQ_LED_IOC_SET_SLR_VIP_TABLE_LDCOMP\n");
			if (copy_from_user(&LD_Data_Compensation_NewMode_2DTable[0],  (int __user *)arg,sizeof(LD_Data_Compensation_NewMode_2DTable))) {
				ret = -1;
			} else {

				rtd_pr_vpq_info("2DTable[0][0][5]=%d\n",LD_Data_Compensation_NewMode_2DTable[0].ld_comp_2Dtable[0][5]);
				fwif_color_set_LD_Data_Compensation_NewMode_2DTable(src_idx,TableIdx);					
			}
		}
		break;

	case VPQ_LED_IOC_SET_SLR_VIP_TABLE_BLPF:
		{						
			extern SLR_VIP_TABLE_BLPF m_defaultVipTableBLPF;
			
			rtd_pr_vpq_info("VPQ_LED_IOC_SET_SLR_VIP_TABLE_BLPF\n");
			if (copy_from_user(&m_defaultVipTableBLPF,  (int __user *)arg,sizeof(m_defaultVipTableBLPF))) {
				ret = -1;
			} else {

				rtd_pr_vpq_info("BLPF H[0][5]=%d\n",m_defaultVipTableBLPF.LD_Backlight_Profile_Interpolation_table_H[0][5]);
				rtd_pr_vpq_info("BLPF V[0][5]=%d\n",m_defaultVipTableBLPF.LD_Backlight_Profile_Interpolation_table_V[0][5]);
				fwif_color_set_LD_BL_Profile_Interpolation_Table_HV(src_idx, TableIdx);				
			}
			
		}
		break;

		/*TV043*/
	case VPQ_LED_IOC_LDSetDutyLimit:
		{
			unsigned short DutyLimit[2];

			if (PQ_LED_Dev_Status != PQ_LED_DEV_INIT_DONE)
				return -1;

			if (copy_from_user(&DutyLimit[0], (int __user *)arg, sizeof(unsigned short)*2)) {
				rtd_pr_vpq_err("kernel VPQ_LED_IOC_LDSetDutyLimit fail\n");
				ret = -1;
			} else {

				fwif_color_set_LD_Boost_TV043(&DutyLimit[0]);
				rtd_pr_vpq_debug("kernel VPQ_LED_IOC_LDSetDutyLimit success\n");
				ret = 0;
			}

		}
		break;
	
	case VPQ_LED_IOC_LDSetDuty:
		{
			unsigned short Duty;
	
			if (PQ_LED_Dev_Status != PQ_LED_DEV_INIT_DONE)
				return -1;
	
			if (copy_from_user(&Duty, (int __user *)arg, sizeof(unsigned short))) {
				rtd_pr_vpq_err("kernel VPQ_LED_IOC_LDSetDuty fail\n");
				ret = -1;
			} else {
				fwif_color_set_LD_SPIDataSRAM_Duty(Duty);
				rtd_pr_vpq_debug("kernel VPQ_LED_IOC_LDSetDuty success\n");
				ret = 0;
			}
	
		}
		break;
	
	case VPQ_LED_IOC_SET_LDSPI_SRAM_Data:
		{						
			
			DRV_LD_LDSPI_DATASRAM_TYPE tLD_LDSPI_DataSRAM;

			if (copy_from_user(&tVPQLED_LD_LDSPI_DataSRAM,  (int __user *)arg,sizeof(tVPQLED_LD_LDSPI_DataSRAM))) {
				rtd_pr_vpq_emerg("kernel VPQ_LED_IOC_SET_LDSPI_SRAM_Data fail\n");
				ret = -1;
			} else {

				tLD_LDSPI_DataSRAM.SRAM_Length = tVPQLED_LD_LDSPI_DataSRAM.SRAM_Length;
				tLD_LDSPI_DataSRAM.SRAM_Position= tVPQLED_LD_LDSPI_DataSRAM.SRAM_Position;
				tLD_LDSPI_DataSRAM.SRAM_Value = &tVPQLED_LD_LDSPI_DataSRAM.SRAM_Value[0];
				
				rtd_pr_vpq_info("SRAM_Length=%d pSRAM_Value=%p arg=0x%x\n", tLD_LDSPI_DataSRAM.SRAM_Length, tLD_LDSPI_DataSRAM.SRAM_Value, arg);
				rtd_pr_vpq_info("SRAM_Position=0x%x, SRAM_Value[0]=%d\n",tLD_LDSPI_DataSRAM.SRAM_Position,tLD_LDSPI_DataSRAM.SRAM_Value[0]);
				drvif_color_set_LDSPI_DataSRAM_Data_Continuous(&tLD_LDSPI_DataSRAM, 1);								
			}
			
		}
		break;

	default:
		rtd_pr_vpq_err("kernel IO command %d not found!\n", cmd);
		return -1;
			;
	}
	return ret;/*Success*/

}

void vpqled_do_resume(void) {
	vpq_led_LD_lutTableIndex = 0;
	vpq_led_LocalDimmingDemoCtrlMode = FALSE;
}

void vpqled_HAL_VPQ_LED_LDCtrlSPI(unsigned char *LDCtrlSPI)
{
	down(&VPQ_LED_Semaphore);
	drvif_HAL_VPQ_LED_LDCtrlSPI(LDCtrlSPI);
	up(&VPQ_LED_Semaphore);
}

void vpqled_HAL_VPQ_LED_LDCtrlDemoMode(LOCALDIMMNG_DEMO_TYPE_T bType, BOOLEAN bCtrl)
{
	static DRV_LD_Global_Ctrl tLD_Global_Ctrl_ori;
	static DRV_LD_Spatial_Filter tLD_Spatial_Filter_ori;
	DRV_LD_Global_Ctrl tLD_Global_Ctrl;
	DRV_LD_Spatial_Filter tLD_Spatial_Filter;

	down(&VPQ_LED_Semaphore);
	vpq_led_LocalDimmingDemoCtrlMode = bCtrl;
	rtk_hal_vsc_set_localDimmingCtrlDemoMode(bType, bCtrl);
	fwif_color_set_output_gamma_PTG_TV006(bCtrl, 4095, 4095, 4095, 4095); /*generate white pattern*/
	if (bCtrl == true) {
			drvif_color_get_LD_Global_Ctrl(&tLD_Global_Ctrl_ori);
			drvif_color_get_LD_Spatial_Filter(&tLD_Spatial_Filter_ori);
			memcpy(&tLD_Global_Ctrl, &tLD_Global_Ctrl_ori, sizeof(DRV_LD_Global_Ctrl));
			memcpy(&tLD_Spatial_Filter, &tLD_Spatial_Filter_ori, sizeof(DRV_LD_Spatial_Filter));
			tLD_Global_Ctrl.ld_tenable = 0; /*disable temporal filter*/
			tLD_Spatial_Filter.ld_spatialnewcoef01 = 0;
			tLD_Spatial_Filter.ld_spatialnewcoef02 = 0;
			tLD_Spatial_Filter.ld_spatialnewcoef03 = 0;
			tLD_Spatial_Filter.ld_spatialnewcoef04 = 0;
			tLD_Spatial_Filter.ld_spatialnewcoef05 = 0;
			tLD_Spatial_Filter.ld_spatialnewcoef11 = 0;
			tLD_Spatial_Filter.ld_spatialnewcoef12 = 0;
			tLD_Spatial_Filter.ld_spatialnewcoef13 = 0;
			tLD_Spatial_Filter.ld_spatialnewcoef14 = 0;
			tLD_Spatial_Filter.ld_spatialnewcoef15 = 0;
			drvif_color_set_LD_Global_Ctrl(&tLD_Global_Ctrl);
			drvif_color_set_LD_Spatial_Filter(&tLD_Spatial_Filter);
			
		} else {
			drvif_color_set_LD_Global_Ctrl(&tLD_Global_Ctrl_ori);
			drvif_color_set_LD_Spatial_Filter(&tLD_Spatial_Filter_ori);
			fwif_color_set_LDSetLUT(vpq_led_LD_lutTableIndex);	/*demo mode off, recover LD_index(table) */
		}
		
	up(&VPQ_LED_Semaphore);
}


unsigned char vpqled_get_LD_GetAPL_TV006(UINT16* BLValue)
{
	unsigned char suc_flag;
	down(&VPQ_LED_Semaphore);
	suc_flag = fwif_color_get_LD_GetAPL_TV006(BLValue);
	up(&VPQ_LED_Semaphore);
	return suc_flag;

}

#ifndef UT_flag
struct file_operations vpq_led_fops = {
	.owner =    THIS_MODULE,
	.open  =    vpq_led_open,
	.release =  vpq_led_release,
	.read  =    vpq_led_read,
	.write = 	vpq_led_write,
	.unlocked_ioctl =    vpq_led_ioctl,
#ifdef CONFIG_ARM64
#ifdef CONFIG_COMPAT
	.compat_ioctl = vpq_led_ioctl,
#endif
#endif
};




static struct class *vpq_led_class;
static struct platform_device *vpq_led_platform_devs;
static struct platform_driver vpq_led_platform_driver = {
	.driver = {
		.name       = "vpqleddev",
		.bus        = &platform_bus_type,
    },
} ;


static char *vpq_led_devnode(struct device *dev, umode_t *mode)
{
	return NULL;
}

int vpq_led_module_init(void)
{
	int result;
	result = alloc_chrdev_region(&vpq_led_devno, 0, 1, "vpqleddevno");

	if (result != 0) {
		rtd_pr_vpq_err("Cannot allocate VPQ LED device number\n");
		return result;
	}

	vpq_led_class = class_create(THIS_MODULE, "vpqleddev");
	if (IS_ERR(vpq_led_class)) {
		rtd_pr_vpq_err("scalevpqled: can not create class...\n");
		result = PTR_ERR(vpq_led_class);
		goto fail_class_create;
	}

	vpq_led_class->devnode = vpq_led_devnode;

	vpq_led_platform_devs = platform_device_register_simple("vpqleddev", -1, NULL, 0);
	if (platform_driver_register(&vpq_led_platform_driver) != 0) {
		rtd_pr_vpq_err("scalevpq: can not register platform driver...\n");
		result = -ENOMEM;
		goto fail_platform_driver_register;
	}

	cdev_init(&vpq_led_cdev, &vpq_led_fops);
	result = cdev_add(&vpq_led_cdev, vpq_led_devno, 1);
	if (result < 0) {
		rtd_pr_vpq_err("scalevpqled: can not add character device...\n");
		goto fail_cdev_init;
	}

	device_create(vpq_led_class, NULL, vpq_led_devno, NULL, "vpqleddev");
	sema_init(&VPQ_LED_Semaphore, 1);
	sema_init(&VPQ_ld_running_Semaphore, 1);

	return 0;/*Success*/

fail_cdev_init:
	platform_driver_unregister(&vpq_led_platform_driver);
fail_platform_driver_register:
	platform_device_unregister(vpq_led_platform_devs);
	vpq_led_platform_devs = NULL;
	class_destroy(vpq_led_class);
fail_class_create:
	vpq_led_class = NULL;
	unregister_chrdev_region(vpq_led_devno, 1);
	return result;

}



void __exit vpq_led_module_exit(void)
{
	if (vpq_led_platform_devs == NULL)
		BUG();

	device_destroy(vpq_led_class, vpq_led_devno);
	cdev_del(&vpq_led_cdev);

	platform_driver_unregister(&vpq_led_platform_driver);
	platform_device_unregister(vpq_led_platform_devs);
	vpq_led_platform_devs = NULL;

	class_destroy(vpq_led_class);
	vpq_led_class = NULL;

	unregister_chrdev_region(vpq_led_devno, 1);
}

#ifdef CONFIG_SUPPORT_SCALER_MODULE
// the module init/exit will be moved to scaler_module.c if scaler was built as a kernel module
#else
module_init(vpq_led_module_init);
module_exit(vpq_led_module_exit);
#endif
#endif

