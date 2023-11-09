/*This is for ADC PTG setting*/
//Kernel Header file
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>


/* RBUS Header file*/

/* Tvscalercontrol Header file */
#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
	#include <scaler/scalerDrvCommon.h>	
#else
	#include <scalercommon/scalerCommon.h>
	#include <scalercommon/scalerDrvCommon.h>	
#endif
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/scalerdrv/mode.h>
#include <tvscalercontrol/scaler_drv_verify/scaler_verify_common.h>
#include <scaler_vscdev.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/panel/panelapi.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <rtd_log/rtd_module_log.h>

unsigned int run_capture_verify_mode(void)
{//TRUE: success      FALSE: fail
	unsigned int verify_result = 0; 
	unsigned int size, curindex;
	ModeTableType *verifytable = NULL;
	unsigned int in_width = 0, in_height = 0;//source width and hight
	VIDEO_RECT_T inregion = {0, 0 , 1920, 1080};
	VIDEO_RECT_T originalInput = {0, 0 , 1920, 1080};
	KADP_VIDEO_RECT_T outregion = {0, 0, _DISP_WID, _DISP_LEN};
	extern unsigned char Check_DisplayFrame_isRightColorbar(void);
	extern void drvif_mode_print_mode_ID(unsigned char mode);
	extern void set_auto_phase_flow_bypass_flag(unsigned char flag);
	extern unsigned char rtk_hal_vsc_SetInputRegionEx(VIDEO_WID_T wid, VIDEO_RECT_T  inregion, VIDEO_RECT_T originalInput);
	extern unsigned char rtk_hal_vsc_SetOutputRegion(VIDEO_WID_T wid, KADP_VIDEO_RECT_T outregion, unsigned short Wide, unsigned short High);
	extern unsigned char rtk_hal_vsc_SetWinBlank(VIDEO_WID_T wid, bool bonoff, KADP_VIDEO_DDI_WIN_COLOR_T color);
	extern struct semaphore* get_adc_detectsemaphore(void);
	extern void adc_ptg_verify_set_status(void);
	size = get_adc_ptg_table_size();//get verify table how many timing
	down(get_adc_detectsemaphore());
	adc_ptg_verify_set_status();
	up(get_adc_detectsemaphore());

	for(curindex = 0; curindex < size ; curindex++)
	{
		verifytable = get_adc_ptg_table_from_index(curindex);
		if(verifytable)
		{
			rtk_hal_vsc_SetWinBlank(0, 1 , 0);//mute off
			drvif_mode_disableonlinemeasure(SLR_MAIN_DISPLAY);
			drvif_mode_onlinemeasure_setting(SLR_MAIN_DISPLAY, 0, 0, 0);
			Scaler_DispSetInputInfoByDisp(SLR_MAIN_DISPLAY, SLR_INPUT_STATE, _MODE_STATE_SEARCH);
			if(iptg_driver_setting(verifytable->ModeID))
			{
				drvif_mode_print_mode_ID(verifytable->ModeID);
				in_width = verifytable->IHWidth;
				in_height = verifytable->IVHeight;
				set_auto_phase_flow_bypass_flag(TRUE);//no need to do auto phase
				//set input no crop
				inregion.x = 0;
				inregion.y = 0;
				inregion.w = in_width;
				inregion.h = in_height;
				originalInput.x = 0;
				originalInput.y = 0;
				originalInput.w = in_width;
				originalInput.h = in_height;

				//set output full panel
				outregion.x = 0;
				outregion.y = 0;
				outregion.w = _DISP_WID;
				outregion.h = _DISP_LEN;
				msleep(30);
				rtk_hal_vsc_SetInputRegionEx(0, inregion, originalInput);//set input
				msleep(30);
				rtk_hal_vsc_SetOutputRegion(0, outregion, _DISP_WID, _DISP_LEN);//set output
				msleep(30);
				rtk_hal_vsc_SetWinBlank(0, 0, 0);//mute off
				msleep(5500);
				if(Check_DisplayFrame_isRightColorbar())
					verify_result += (1<<curindex);
				else
				{
					rtd_pr_vsc_err("\r\n########func:%s verify error(%d)######\r\n",__FUNCTION__, curindex);
					set_auto_phase_flow_bypass_flag(FALSE);
					break;
				}
				msleep(2000);
				set_auto_phase_flow_bypass_flag(FALSE);
			}
			else
			{
				rtd_pr_vsc_err("\r\n########func:%s set ADC PTG error######\r\n",__FUNCTION__);
				return FALSE;
			}

		}
	}

	return verify_result;
}

