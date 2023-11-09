/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2017
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for color related functions.
 *
 * @author 	$Author$
 * @date 	$Date$
 * @version 	$Revision$
 */

/**
 * @addtogroup color
 * @{
 */

/*============================ Module dependency  ===========================*/
#include <rtd_log/rtd_module_log.h>
#include <linux/semaphore.h>


#include <linux/string.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <linux/kernel.h>
#include "tvscalercontrol/vip/fcic.h"
#include <tvscalercontrol/scaler/vipinclude.h> // for typedef that used in VIP_reg_def.h
#include <tvscalercontrol/vip/vip_reg_def.h>
//#include <rbus/fcic_top_reg.h> /*merlin8 remove fcic*/
//#include <rbus/lg_m_plus_reg.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include "rtk_vip_logger.h"
#include <tvscalercontrol/vip/color.h>
#include <rbus/sys_reg_reg.h>

#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_DEBUG,fmt,##args)

#define TAG_NAME "VPQ_FCIC"

/*================================  definitions ===============================*/


/*===================================  Types ================================*/

/*================================== Variables ==============================*/

void drvif_color_set_fcic_table(unsigned int *pFCICtbl, unsigned int uFCICtbl_length)
{
#if 0 /*merlin8 remove fcic*/
	fcic_top_fcic_flash_if_1_RBUS fcic_top_fcic_flash_if_1;
	fcic_top_fcic_ctrl_1_RBUS fcic_top_fcic_ctrl_1;
	fcic_top_fcic_io_reg_RBUS fcic_top_fcic_io_reg;
	fcic_top_fcic_flash_if_2_RBUS fcic_top_fcic_flash_if_2;
	fcic_top_fcic_crtl_2_RBUS fcic_top_fcic_crtl_2;
	//lg_m_plus_m_plus_ctl_RBUS lg_m_plus_m_plus_ctl; //mpluse remove from k8 

	unsigned int i;

	fcic_top_fcic_flash_if_1.regValue = IoReg_Read32(FCIC_TOP_FCIC_FLASH_IF_1_reg);
	fcic_top_fcic_ctrl_1.regValue = IoReg_Read32(FCIC_TOP_FCIC_CTRL_1_reg);
	fcic_top_fcic_io_reg.regValue = IoReg_Read32(FCIC_TOP_FCIC_IO_reg_reg);
	fcic_top_fcic_crtl_2.regValue = IoReg_Read32(FCIC_TOP_FCIC_CRTL_2_reg);
	//lg_m_plus_m_plus_ctl.regValue = IoReg_Read32(LG_M_PLUS_M_plus_ctl_reg); //xz 20180705

	if (drvif_color_get_HW_MEX_STATUS(MEX_HCIC) != 1) {
		rtd_printk(KERN_INFO, TAG_NAME, "HW_MEX,  MEX_HCIC != 1,TBL\n");
		return;
	}

	if (pFCICtbl == NULL)
		return;

	/* settings before writing table*/
	fcic_top_fcic_flash_if_1.flash_done = 0;
	fcic_top_fcic_ctrl_1.fcic_rbus_if_enable = 1;
	/*if(lg_m_plus_m_plus_ctl.wpr_on)	//mpluse remove from k8 
		fcic_top_fcic_io_reg.mplus_en= 1;
	else*/
		fcic_top_fcic_io_reg.mplus_en= 0;
	fcic_top_fcic_io_reg.fcic_io_reg_en = 0;	//xz 20180705
	//fcic_top_fcic_io_reg.fcic_io_reg_en = 1;
	//fcic_top_fcic_io_reg.color_4 = 3;
	//fcic_top_fcic_io_reg.color_3 = 2;
	//fcic_top_fcic_io_reg.color_2 = 1;
	//fcic_top_fcic_io_reg.color_1 = 0;

	IoReg_Write32(FCIC_TOP_FCIC_FLASH_IF_1_reg, fcic_top_fcic_flash_if_1.regValue);
	IoReg_Write32(FCIC_TOP_FCIC_CTRL_1_reg, fcic_top_fcic_ctrl_1.regValue);
	IoReg_Write32(FCIC_TOP_FCIC_IO_reg_reg, fcic_top_fcic_io_reg.regValue);


	/*write table*/
	if(uFCICtbl_length>0)
	{
		int time_out;
		for(i = 0; i < uFCICtbl_length; i++) {
			fcic_top_fcic_flash_if_2.flash_data_0 = pFCICtbl[i];
			fcic_top_fcic_crtl_2.multi_access_en = 0;
			fcic_top_fcic_crtl_2.flash_write_enable = 1;

			IoReg_Write32(FCIC_TOP_FCIC_FLASH_IF_2_reg, fcic_top_fcic_flash_if_2.regValue);
			IoReg_Write32(FCIC_TOP_FCIC_CRTL_2_reg,fcic_top_fcic_crtl_2.regValue); //Multi_access_en = 0; flash_write_enable=1;

			time_out = 1000;
			do {
				fcic_top_fcic_crtl_2.regValue = IoReg_Read32(FCIC_TOP_FCIC_CRTL_2_reg);
				time_out --;
			} while((time_out>0) && (fcic_top_fcic_crtl_2.flash_write_enable == 1));

			if (time_out <= 0)
				rtd_printk(KERN_ERR, TAG_NAME, "fcic_table write timeout!\n");
		}

		/*HW bug, SW work around, reset Addr to 0 for next time */
		//has fixed in merlin4, can ignore
		/*
		for(i = 0; i < 0x20000 - uFCICtbl_length; i++) {
			fcic_top_fcic_crtl_2.multi_access_en = 0;
			fcic_top_fcic_crtl_2.flash_write_enable = 1;

			IoReg_Write32(FCIC_TOP_FCIC_CRTL_2_reg,fcic_top_fcic_crtl_2.regValue); //Multi_access_en = 0; flash_write_enable=1;
		}
		*/
	}


	/*settings after writing table*/
	fcic_top_fcic_flash_if_1.regValue = IoReg_Read32(FCIC_TOP_FCIC_FLASH_IF_1_reg);
	fcic_top_fcic_flash_if_1.flash_done = 1;
	fcic_top_fcic_ctrl_1.fcic_rbus_if_enable = 0;

	IoReg_Write32(FCIC_TOP_FCIC_CTRL_1_reg, fcic_top_fcic_ctrl_1.regValue);
	IoReg_Write32(FCIC_TOP_FCIC_FLASH_IF_1_reg, fcic_top_fcic_flash_if_1.regValue);
#endif
}

void drvif_color_set_fcic_ctrl(unsigned char *u8pControlTbl, unsigned int u32Ctrlsize)
{
#if 0 /*merlin8 remove fcic*/
//	fcic_top_fcic_reg000_RBUS fcic_top_fcic_reg000;
	unsigned int reg_value;
	unsigned int total_length = 0;
	unsigned int total_length_max = 166; /*reg000~reg661*///662/4=165.5~166
	unsigned int i = 0;

	if (drvif_color_get_HW_MEX_STATUS(MEX_HCIC) != 1) {
		rtd_printk(KERN_INFO, TAG_NAME, "HW_MEX,  MEX_HCIC != 1, CTRL\n");
		return;
	}

	if (u8pControlTbl == NULL)
		return;

	if (u32Ctrlsize%4 ==0)
		total_length = u32Ctrlsize/4;
	else
		total_length = u32Ctrlsize/4+1;

	if (total_length>total_length_max)  //1
		return;

	for (i=0;i<total_length;i++) {
		reg_value = ((unsigned int) (*(u8pControlTbl+4*i)) << 24) + ((unsigned int) (*(u8pControlTbl+4*i+1)) << 16) + ((unsigned int) (*(u8pControlTbl+4*i+2)) << 8) + ((unsigned int) *(u8pControlTbl+4*i+3));
		//printk("[%d] %d\n",i, reg_value);
		IoReg_Write32((FCIC_TOP_FCIC_reg000_reg + 4*i), reg_value);
	}
#endif
}
void drvif_color_enable_FCIC_CLK(void)
{
#if 0 /*merlin8 remove fcic*/
	if ((IoReg_Read32(SYS_REG_SYS_CLKEN3_reg) & _BIT22) == 0)	// LGD FCIC clk not enable
	{
		IoReg_Write32(SYS_REG_SYS_CLKEN3_reg, 0x00400001);	 //LGD FCIC CLK
		IoReg_Write32(SYS_REG_SYS_SRST3_reg, 0x00400001);	 //LGD FCIC reset
	}
#endif
}

void drvif_color_disable_FCIC_CLK(void)
{
#if 0 /*merlin8 remove fcic*/
	IoReg_Write32(SYS_REG_SYS_CLKEN3_reg, SYS_REG_SYS_CLKEN3_clken_disp_lg_hcic_mask);	 //LGD FCIC CLK
	IoReg_Write32(SYS_REG_SYS_SRST3_reg, SYS_REG_SYS_SRST3_rstn_disp_lg_hcic_mask);	 //LGD FCIC reset
#endif
}

void drvif_color_set_fcic_enable(unsigned char bCtrl)
{
#if 0 /*merlin8 remove fcic*/
	unsigned long flags;//for spin_lock_irqsave
	int timeoutcnt = 100;
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
//add double buffer enable-------------------------------------------------------
    ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_Reg;

	if (drvif_color_get_HW_MEX_STATUS(MEX_HCIC) != 1) {
		rtd_printk(KERN_INFO, TAG_NAME, "HW_MEX,  MEX_HCIC != 1, EN\n");
		return;
	}
        drvif_color_enable_FCIC_CLK();

	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
    ppoverlay_double_buffer_ctrl_Reg.regValue = IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg);
    ppoverlay_double_buffer_ctrl_Reg.dreg_dbuf_en = 1;
    IoReg_Write32(PPOVERLAY_Double_Buffer_CTRL_reg, ppoverlay_double_buffer_ctrl_Reg.regValue);
//---------------------------------------------------------------------------
	/*  set stage2 smooth toggle protect */
	IoReg_SetBits(PPOVERLAY_dispd_stage2_sm_ctrl_reg, PPOVERLAY_dispd_stage2_sm_ctrl_dispd_stage2_smooth_toggle_protect_mask);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/

	// wait for original db apply finish to let stage2 apply work
	while ((IoReg_Read32(PPOVERLAY_Double_Buffer_CTRL_reg) & PPOVERLAY_Double_Buffer_CTRL_dreg_dbuf_set_mask) && timeoutcnt > 10) {
		timeoutcnt--;
		msleep_interruptible(10);
	}

	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	display_timing_ctrl2_reg.hcic_enable= bCtrl;
	if (bCtrl) {
		display_timing_ctrl2_reg.valc_en = 0;
		rtd_printk(KERN_INFO, TAG_NAME, "drvif_color_set_fcic_enable, need to disable valc_en\n");
	}
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);

	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	/* for K6, no using original apply, use stage2 smooth toggle apply */
	IoReg_SetBits(PPOVERLAY_dispd_smooth_toggle2_reg, PPOVERLAY_dispd_smooth_toggle2_dispd_stage2_smooth_toggle_apply_mask);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/

	// wait for db apply finish
	timeoutcnt = 100;
	while ((IoReg_Read32(PPOVERLAY_dispd_smooth_toggle2_reg) & PPOVERLAY_dispd_smooth_toggle2_dispd_stage2_smooth_toggle_apply_mask) && timeoutcnt > 10) {
		timeoutcnt--;
		msleep_interruptible(10);
	}
	if (timeoutcnt <= 10)
		rtd_printk(KERN_EMERG, TAG_NAME, "drvif_color_set_fcic_enable, timeout!\n");
#endif
}

int drvif_color_check_fcic_table(unsigned int *pFCICtbl, unsigned int uFCICtbl_length)
{
#if 0 /*merlin8 remove fcic*/
	unsigned int idx = (16728*2+4375*6)-1;
	fcic_top_fcic_flash_dbg_1_RBUS fcic_flash_dbg_1;
	fcic_top_fcic_flash_dbg_2_RBUS fcic_flash_dbg_2;

	fcic_flash_dbg_1.regValue = IoReg_Read32(FCIC_TOP_FCIC_FLASH_dbg_1_reg);
	fcic_flash_dbg_2.regValue = IoReg_Read32(FCIC_TOP_FCIC_FLASH_dbg_2_reg);

	fcic_flash_dbg_1.flash_dbg_mux = 7;
	if (uFCICtbl_length > idx && pFCICtbl != NULL) {
		int time_out;
		fcic_flash_dbg_1.rd_addr = 4374;

		fcic_flash_dbg_1.flash_dbg_rd_en = 1;
		IoReg_Write32(FCIC_TOP_FCIC_FLASH_dbg_1_reg, fcic_flash_dbg_1.regValue);

		time_out = 1000;

		do {
			fcic_flash_dbg_1.regValue = IoReg_Read32(FCIC_TOP_FCIC_FLASH_dbg_1_reg);
			time_out --;
		} while((time_out>0) && (fcic_flash_dbg_1.flash_dbg_rd_en == 1));

		if (time_out>0) {
			fcic_flash_dbg_2.regValue = IoReg_Read32(FCIC_TOP_FCIC_FLASH_dbg_2_reg);
			if (pFCICtbl[idx] != fcic_flash_dbg_2.flash_rdata_0) {
				rtd_printk(KERN_INFO, TAG_NAME, "fcic check = 0\n");
				return -1;
			}
		}
	}
#endif
	return 0;
}

