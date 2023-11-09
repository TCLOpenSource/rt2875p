/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2006
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for FILM related functions.
 *
 * @author 	$Author$
 * @date 	$Date$
 * @version 	$Revision$
 */

/**
 * @addtogroup film
 * @{
 */

/*============================ Module dependency  ===========================*/
/*#include <unistd.h>			*/ /* 'close()'*/
/*#include <fcntl.h>			*/ /* 'open()'*/
/*#include <stdio.h>*/
/*#include <sys/ioctl.h>*/
#include <linux/string.h>
//#include <mach/io.h>

#include <rbus/sb2_reg.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/color.h>
#include <tvscalercontrol/vip/dcc.h>
#include <tvscalercontrol/vip/peaking.h>

#ifdef CONFIG_KDRIVER_USE_NEW_COMMON
	#include <scaler/scalerCommon.h>
	#include <scaler/scalerDrvCommon.h>
#else
#include <scalercommon/scalerCommon.h>
	#include <scalercommon/scalerDrvCommon.h>
#endif


/* oliver+*/
#include <tvscalercontrol/scaler/scalerlib.h>
//#include <rbus/scaler/rbusPpOverlayReg.h>
#include <rbus/ppoverlay_reg.h>
#include <rtd_log/rtd_module_log.h>
#include "rtk_vip_logger.h"


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

#undef VIPprintf
#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_DCC_DEBUG,fmt,##args)

/*===================================  Types ================================*/

/*================================== Variables ==============================*/


/*================================ Definitions ==============================*/
/*o------ User defined table ------o */ /* For Factory Mode range: 5~9 in VIP_video_performance.h*/

/*==================== registers setting for New DCC ====================*/

/*********************************************************************************/
/*o------ MicroCode for New Algorithm -------o*/
extern DRV_VipChromaCompensation_t g_Chroma_Compensation_Curve[8];

/*==========HDR============*/
extern unsigned char HDR_demo_flag;
extern unsigned int dcc_histogram_to_write[TableSeg_num_Max];
extern unsigned short dcc_histogram_to_write_base[TableSeg_num_Max];
/*=========================*/



/*================================== Function ===============================*/

void drvif_color_dcc_on(unsigned char enable)
{
	color_d_vc_global_ctrl_RBUS d_vc_global_ctrl_Reg;
	d_vc_global_ctrl_Reg.regValue = rtd_inl(COLOR_D_VC_Global_CTRL_reg);

	if (enable == 0) {
		d_vc_global_ctrl_Reg.dcc_en = 0;/*dcc curve off*/
		d_vc_global_ctrl_Reg.write_enable_1 =1;
	} else {
		d_vc_global_ctrl_Reg.dcc_en = 1;/*dcc curve on*/
		d_vc_global_ctrl_Reg.write_enable_1 =1;
	}
	rtd_outl(COLOR_D_VC_Global_CTRL_reg, d_vc_global_ctrl_Reg.regValue);
}

unsigned char drvif_color_dcc_db_switch()
{
	short timecnt = 0;
	color_dcc_d_dcc_db_ctrl_RBUS color_dcc_d_dcc_db_ctrl_reg;
	color_dcc_d_dcc_ctrl_RBUS	 	color_dcc_d_dcc_ctrl_reg;
	color_dcc_dcc_y_sram_tab_ctl_RBUS	color_dcc_sram_tab_ctl_0_reg;

	color_dcc_d_dcc_db_ctrl_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_DB_CTRL_reg);
	color_dcc_d_dcc_ctrl_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_CTRL_reg);
	color_dcc_sram_tab_ctl_0_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);

	/*step1 : set db apply*/
	color_dcc_d_dcc_db_ctrl_reg.db_apply = 1 ;
	rtd_outl(COLOR_DCC_D_DCC_DB_CTRL_reg, color_dcc_d_dcc_db_ctrl_reg.regValue);

	/*step2 : set DCC Table select*/
	color_dcc_d_dcc_ctrl_reg.main_table_sel = color_dcc_sram_tab_ctl_0_reg.dcc_y_sram_tab_acc_sel;
	rtd_outl(COLOR_DCC_D_DCC_CTRL_reg, color_dcc_d_dcc_ctrl_reg.regValue);

	/*step3 : check db apply finish*/
	do {
		color_dcc_d_dcc_ctrl_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_CTRL_reg);
		timecnt++;
		} while (color_dcc_d_dcc_ctrl_reg.main_table_sel != color_dcc_sram_tab_ctl_0_reg.dcc_y_sram_tab_acc_sel);

	/*if (color_dcc_d_dcc_ctrl_reg.main_table_sel == color_dcc_sram_tab_ctl_0_reg.sram_tab_acc_sel)*/
		/*VIPprintf("Success!! %d\n", timecnt);*/

	return TRUE;

}

unsigned char drvif_color_dcc_Curve_Write(unsigned char display, unsigned char CurveSegNum, unsigned int *sAccLUT)
{
	unsigned char i = 0;
	short index;
	int timeout = 100000;

	//unsigned short G0, G1;
	//int distance;

	color_dcc_d_dcc_db_ctrl_RBUS color_dcc_d_dcc_db_ctrl_reg;
	color_d_vc_global_ctrl_RBUS		d_vc_global_ctrl_REG;
	/*o---- Identity Curve init coef---o*/
	//G1 = G0 = 8;
	//distance = G0 + G1;

	color_dcc_d_dcc_db_ctrl_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_DB_CTRL_reg);
	d_vc_global_ctrl_REG.regValue 		= rtd_inl(COLOR_D_VC_Global_CTRL_reg);

	if (display == SLR_MAIN_DISPLAY) {
		/* DCC Table Write*/
		/*Step 1: DCC Curve Table Write init*/
		color_dcc_dcc_y_sram_tab_ctl_RBUS	color_dcc_sram_tab_ctl_0_reg;
		color_dcc_sram_tab_ctl_0_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
		color_dcc_sram_tab_ctl_0_reg.dcc_y_sram_tab_acc_index = 0;
		color_dcc_sram_tab_ctl_0_reg.dcc_y_sram_tab_rw_sel = 1;
		if (color_dcc_d_dcc_db_ctrl_reg.db_en == 1) {
			color_dcc_sram_tab_ctl_0_reg.dcc_y_sram_tab_acc_sel = (color_dcc_sram_tab_ctl_0_reg.dcc_y_sram_tab_acc_sel == 0) ? 1 : 0;
		} else {
			color_dcc_sram_tab_ctl_0_reg.dcc_y_sram_tab_acc_sel = 0;
		}
		rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg, color_dcc_sram_tab_ctl_0_reg.regValue);

		/*VIPprintf("D_VSC =%d\n", IoReg_BusyCheckRegBit(PPOVERLAY_DTG_PENDING_STATUS_VADDR, _BIT0, 0x1FFFF));*/

		/*Step 2 : Star DCC Table Write*/
		index = -1;
		for (i = 0; i < CurveSegNum; i++) {
			/*Step 2.1 : check sram table ready to write*/
			do {
				timeout--;
				color_dcc_sram_tab_ctl_0_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
				/*VIPprintf(" timeout!!%d \n", timeout);*/
				if (timeout <= 0) {
					VIPprintf(" timeout!! DCC Init. Write DCC acc_ready Fault!!\n");
					/*break;*/
				}
				} while ((color_dcc_sram_tab_ctl_0_reg.dcc_y_sram_tab_acc_ready != 1));

			/*VIPprintf(" timeout!!%d \n", timeout);*/
			/*Step 2.2 : check sram table ready status*/

			color_dcc_sram_tab_ctl_0_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
			if (color_dcc_sram_tab_ctl_0_reg.dcc_y_sram_tab_acc_ready != 1) {
				color_dcc_sram_tab_ctl_0_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
				color_dcc_sram_tab_ctl_0_reg.dcc_y_sram_tab_rw_sel = 0;
				rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg, color_dcc_sram_tab_ctl_0_reg.regValue);
				VIPprintf("DCC Init. Write DCC Init Table Fault!!\n");
				return FALSE;
			}
			//Base = i * distance - 1;

			/*Step 2.3 : check write index for current index = now index*/
			index++;
			color_dcc_sram_tab_ctl_0_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);

			if (index != color_dcc_sram_tab_ctl_0_reg.dcc_y_sram_tab_acc_index_cur) {    /*cur = index*/
				color_dcc_sram_tab_ctl_0_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
				color_dcc_sram_tab_ctl_0_reg.dcc_y_sram_tab_rw_sel = 0;
				rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg, color_dcc_sram_tab_ctl_0_reg.regValue);
				return FALSE;
			}
			/*VIPprintf("sAccLUT[%d]=%x, %x\n", i, sAccLUT[i],((Base << 16) + (G0 << 8) + G1));*/
			VIPprintf("sAccLUT[%d]=%x\n", i, sAccLUT[i]);
			rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_PORT_reg, sAccLUT[i]);
			/*rtd_outl((COLOR_DCC_DCC_Y_SRAM_TAB_PORT_reg), ((Base << 16) + (G0 << 8) + G1));*/
			timeout = 100000;
		}

		/*Step 3 : check write finish, and avoid other API access reg. at the same time.*/
		do {
			timeout--;
			color_dcc_sram_tab_ctl_0_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
			/*VIPprintf("i =%d, timeout =%d, sram_tab_acc_ready =%d \n", i, timeout, color_dcc_sram_tab_ctl_0_reg.sram_tab_acc_ready);*/
			if (timeout <= 0) {
				VIPprintf(" timeout!! DCC Init. Write DCC Init Table Fault!!\n");
				break;
			}
			} while ((color_dcc_sram_tab_ctl_0_reg.dcc_y_sram_tab_acc_ready != 1));

		/*Step 4 : diable DCC sram table RW_Sel*/
		color_dcc_sram_tab_ctl_0_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
		color_dcc_sram_tab_ctl_0_reg.dcc_y_sram_tab_rw_sel = 0;
		rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg, color_dcc_sram_tab_ctl_0_reg.regValue);

		if (color_dcc_d_dcc_db_ctrl_reg.db_en == 1) {
			if (drvif_color_dcc_db_switch() != TRUE) {
				return FALSE;
			}
		}
	}
	return TRUE;

}


void drvif_color_dcc_Curve_init(unsigned char display)
{
	unsigned char i = 0;
	unsigned short Base = 0, G0, G1;
	int index, timeout = 10;
	int DCCNum = 64;/*this is controlled by DCC segment*/
	int distance;

	_system_setting_info *VIP_system_info_structure_table = (_system_setting_info*)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_system_info_structure);

	color_dcc_d_dcc_ctrl_RBUS	 	color_dcc_d_dcc_ctrl_reg;
	color_d_vc_global_ctrl_RBUS 	color_d_vc_global_ctrl_reg;
	color_dcc_d_dcc_db_ctrl_RBUS color_dcc_d_dcc_db_ctrl_reg;

	color_dcc_d_dcc_ctrl_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_CTRL_reg);
	color_d_vc_global_ctrl_reg.regValue = rtd_inl(COLOR_D_VC_Global_CTRL_reg);
	color_dcc_d_dcc_db_ctrl_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_DB_CTRL_reg);


	/*o---- Identity Curve init coef---o*/
	G1 = G0 = 4096 / DCCNum / 2;
	distance = G0 + G1;

	/* initialize table of DCC Curve*/
	if (display == SLR_MAIN_DISPLAY) {
		color_dcc_dcc_y_sram_tab_ctl_RBUS color_dcc_dcc_y_sram_tab_ctl_reg;

		/*o----Select Dcc Table and Segment num---o*/
		color_dcc_d_dcc_ctrl_reg.dcc_main_mode_sel = 1;
		color_dcc_d_dcc_ctrl_reg.main_table_sel = 1;//hw use tab1 temporary for sram write tab0 (pin-pon toggle)
		color_dcc_d_dcc_ctrl_reg.main_table_segment = 0;
		color_dcc_d_dcc_ctrl_reg.y_mapping_mode =1;
		color_dcc_d_dcc_ctrl_reg.y_mapping_blending_wt=16;

		rtd_outl(COLOR_DCC_D_DCC_CTRL_reg, color_dcc_d_dcc_ctrl_reg.regValue);

		/*o---DCC Curve Table init for Main channel---o*/
		color_dcc_dcc_y_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
		color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_acc_index = 0;
		color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_rw_sel = 1;
		color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_acc_sel = 0;
		rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg, color_dcc_dcc_y_sram_tab_ctl_reg.regValue);


		/*o---- Double Buffer Setting -----o*/
		color_dcc_d_dcc_db_ctrl_reg.db_en = 0;
		color_dcc_d_dcc_db_ctrl_reg.db_read_level = 0;
		rtd_outl(COLOR_DCC_D_DCC_DB_CTRL_reg, color_dcc_d_dcc_db_ctrl_reg.regValue);
		index = -1;
		for (i = 0; i < DCCNum; i++) {
			Base = i * distance;
			timeout = 10;
			do {
				color_dcc_dcc_y_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
				} while ((color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_acc_ready != 1) && ((timeout--) > 0));

			if (timeout <= 0) {
				color_dcc_dcc_y_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
				color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_rw_sel = 0;
				rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg, color_dcc_dcc_y_sram_tab_ctl_reg.regValue);
				rtd_pr_vpq_err("DCC Init. Write DCC Init Table Fault, dcc_y_sram_tab_acc_ready!!");
				return;
			}
			index++;
			if (index != color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_acc_index_cur) {
				color_dcc_dcc_y_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
				color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_rw_sel = 0;
				rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg, color_dcc_dcc_y_sram_tab_ctl_reg.regValue);
				rtd_pr_vpq_err("DCC Init. Write DCC Init Table Fault, dcc_y_sram_tab_acc_index_cur!!");
				return;
			}
			color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_acc_index = i;
			color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_base = Base;
			rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg, color_dcc_dcc_y_sram_tab_ctl_reg.regValue);
			rtd_outl((COLOR_DCC_DCC_Y_SRAM_TAB_PORT_reg), ((G0 << COLOR_DCC_DCC_Y_SRAM_TAB_PORT_dcc_y_sram_tab_g0_shift) + G1));
		}
		color_dcc_dcc_y_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
		color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_rw_sel = 0;
		rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg, color_dcc_dcc_y_sram_tab_ctl_reg.regValue);

		color_dcc_d_dcc_ctrl_reg.main_table_sel = 0; //hw use tab0 (pin-pon toggle)
		rtd_outl(COLOR_DCC_D_DCC_CTRL_reg, color_dcc_d_dcc_ctrl_reg.regValue);
	}
#if 1/*def VIP_CONFIG_DUAL_CHENNEL  */ /* There are two channels*/
	else {
		color_dcc_dcc_y_sram_tab_ctl_RBUS color_dcc_dcc_y_sram_tab_ctl_reg;

		/*o----Select Dcc Table and Segment num---o*/
		color_dcc_d_dcc_ctrl_reg.dcc_sub_mode_sel = 2; /* use sub_table_sel*/
		color_dcc_d_dcc_ctrl_reg.sub_table_sel = 0;
		color_dcc_d_dcc_ctrl_reg.sub_table_segment = 0;
		rtd_outl(COLOR_DCC_D_DCC_CTRL_reg, color_dcc_d_dcc_ctrl_reg.regValue);

		/*o---DCC Curve Table init for Sub channel---o*/
		color_dcc_dcc_y_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
		color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_acc_index = 0;
		color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_rw_sel = 1;
		color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_acc_sel = 2; /* SUB table1*/
		rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg, color_dcc_dcc_y_sram_tab_ctl_reg.regValue);


		/*o---- Double Buffer Setting -----o*/
		color_dcc_d_dcc_db_ctrl_reg.db_en = 0;
		color_dcc_d_dcc_db_ctrl_reg.db_read_level = 0;
		rtd_outl(COLOR_DCC_D_DCC_DB_CTRL_reg, color_dcc_d_dcc_db_ctrl_reg.regValue);
		index = -1;
		for (i = 0; i < DCCNum; i++) {
			Base = i * distance - 1;
			timeout = 10;
			do {
				color_dcc_dcc_y_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
				} while ((color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_acc_ready != 1) && ((timeout--) > 0));

			if (timeout <= 0) {
				color_dcc_dcc_y_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
				color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_rw_sel = 0;
				rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg, color_dcc_dcc_y_sram_tab_ctl_reg.regValue);
				VIPprintf("DCC Init. Write DCC Init Table Fault!!");
				return;
			}
			index++;
			if (index != color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_acc_index_cur) {     /*cur = index*/
				color_dcc_dcc_y_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
				color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_rw_sel = 0;
				rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg, color_dcc_dcc_y_sram_tab_ctl_reg.regValue);
				return;
			}
			if (VIP_system_info_structure_table->HDR_flag== 1) {
				color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_acc_index = i;
				color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_base = dcc_histogram_to_write_base[i];
				rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg, color_dcc_dcc_y_sram_tab_ctl_reg.regValue);
				rtd_outl((COLOR_DCC_DCC_Y_SRAM_TAB_PORT_reg), dcc_histogram_to_write[i]);
			} else {
				color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_acc_index = i;
				color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_base = Base;
				rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg, color_dcc_dcc_y_sram_tab_ctl_reg.regValue);
				rtd_outl((COLOR_DCC_DCC_Y_SRAM_TAB_PORT_reg), ((G0 << COLOR_DCC_DCC_Y_SRAM_TAB_PORT_dcc_y_sram_tab_g0_shift) + G1));
			}
		}
		color_dcc_dcc_y_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
		color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_rw_sel = 0;
		rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg, color_dcc_dcc_y_sram_tab_ctl_reg.regValue);
	}
#endif

	//color_d_vc_global_ctrl_reg.dcc_en = 1; //KRKAMTKA-59 : no need dcc on when init
	//color_d_vc_global_ctrl_reg.write_enable_1 =1; //KRKAMTKA-59 : no need dcc on when init

	#ifdef TV010_PQ_STYLE
		color_d_vc_global_ctrl_reg.icm_dcc_swap = 1 ;
	#endif

	scaler_rtd_outl(COLOR_D_VC_Global_CTRL_reg, color_d_vc_global_ctrl_reg.regValue);
}

void drvif_color_dcc_hist_on(unsigned char display, int status)
{
	if (display == SLR_MAIN_DISPLAY) {
	histogram_ich1_hist_in_cselect_RBUS hist_in_cselect_Reg;
	histogram_ich1_c0_hist_bound_RBUS ich1_hist_bound_Reg;

		hist_in_cselect_Reg.regValue = rtd_inl(HISTOGRAM_ICH1_Hist_IN_CSELECT_reg);
		hist_in_cselect_Reg.ch1_his_enable_histog = status;
		rtd_outl(HISTOGRAM_ICH1_Hist_IN_CSELECT_reg, hist_in_cselect_Reg.regValue);

		ich1_hist_bound_Reg.regValue = rtd_inl(HISTOGRAM_ICH1_C0_Hist_Bound_reg);
		ich1_hist_bound_Reg.ch1_c0_lower_bound = 0;
		ich1_hist_bound_Reg.ch1_c0_upper_bound = 1023; //20150708 chance_qian
		rtd_outl(HISTOGRAM_ICH1_C0_Hist_Bound_reg, ich1_hist_bound_Reg.regValue);

	}
#ifdef CONFIG_DUAL_CHANNEL
	else {
		histogram_ich2_hist_in_cselect_RBUS hist_in_cselect_Reg;
		histogram_ich2_c0_hist_bound_RBUS ich2_hist_bound_Reg;

		hist_in_cselect_Reg.regValue = rtd_inl(HISTOGRAM_ICH2_Hist_IN_CSELECT_reg);
		hist_in_cselect_Reg.ch2_his_enable_histog = status;
		rtd_outl(HISTOGRAM_ICH2_Hist_IN_CSELECT_reg, hist_in_cselect_Reg.regValue);

		ich2_hist_bound_Reg.regValue = rtd_inl(HISTOGRAM_ICH2_C0_Hist_Bound_reg);
		ich2_hist_bound_Reg.ch2_c0_lower_bound = 0;
		ich2_hist_bound_Reg.ch2_c0_upper_bound = 1023; //20150708 chance_qian
		rtd_outl(HISTOGRAM_ICH2_C0_Hist_Bound_reg, ich2_hist_bound_Reg.regValue);
	}
#endif
}




/*============================================================================*/
/*o---------------------------------------------------o*/
/*      DCC_setLevel*/
/*      ==> to set DCC level*/
/*      @param <DCC_level> { 0 ~ 15, default 2*/
/*                           0  : Disable DCC*/
/*                           1~4  : internal user define curve*/
/*                           5~9 :  user define curve by Tool  }*/
/*      @return {none}*/
/*o---------------------------------------------------o*/



void drvif_color_set_dcc_overlap_table(void)
{
#if 1/*def VIP_CONFIG_DUAL_CHENNEL*/

	unsigned char overlap;
	color_dcc_d_dcc_ctrl_RBUS d_dcc_ctrl_Reg;

	overlap = (IoReg_Read32(CON_BRI_DM_COLOR_RGB_CTRL_reg) & 0xf8000000) >> 27;

	/* select overlap table*/
	/*	d_dcc_ctrl_Reg.regValue = IoReg_Read32(DCC_D_DCC_CTRL_reg);  */ /* Erin tmp why not !?*/

	if (overlap == 0) {
		d_dcc_ctrl_Reg.dcc_overlay = 1;
		/*d_dcc_ctrl_Reg.dcc_sub_tab_sel = 2;  */ /* by pass temporarily because sub table fails*/
	} else {
		d_dcc_ctrl_Reg.dcc_overlay = 0;

	}
	IoReg_Write32(COLOR_DCC_D_DCC_CTRL_reg, d_dcc_ctrl_Reg.regValue);
#endif
}

void drvif_color_set_dcc_Color_Independent_AI(VIP_DCC_Color_Independent_Blending_Table *ptr)
{
//#ifdef VPQ_COMPILER_ERROR_ENABLE_MERLIN6_
	UINT32 /*Segment0_0 = 0, Segment0_1 = 0, */Segment1_0 = 0, Segment1_1 = 0, Segment2_0 = 0, Segment2_1 = 0, Segment3_0 = 0, Segment3_1 = 0;
	UINT32 Segment0_0_c = 0, Segment0_1_c = 0, Segment1_0_c = 0, Segment1_1_c = 0, Segment2_0_c = 0, Segment2_1_c = 0, Segment3_0_c = 0, Segment3_1_c = 0; //20150708 chance_qian

	color_dcc_d_dcc_skin_tone_en_RBUS d_dcc_skin_tone_en_reg;

	color_dcc_d_dcc_main_offset_RBUS color_dcc_d_dcc_main_offset_reg;    //20170524 robin_zhang
	color_dcc_d_dcc_sub_offset_RBUS   color_dcc_d_dcc_sub_offset_reg;    //20170524 robin_zhang

	color_dcc_d_dcc_skin_tone_yuv_center_0_RBUS d_dcc_skin_tone_yuv_center_0_reg;
	color_dcc_d_dcc_skin_tone_yuv_center_00_RBUS d_dcc_skin_tone_yuv_center_00_reg;
	color_dcc_d_dcc_skin_tone_yuv_range_0_RBUS d_dcc_skin_tone_yuv_range_0_reg;
    color_dcc_d_dcc_skin_tone_hsi_0_center_width_0_RBUS	d_dcc_skin_tone_hsi_0_center_width_0_reg;
    color_dcc_d_dcc_skin_tone_hsi_0_center_width_1_RBUS	d_dcc_skin_tone_hsi_0_center_width_1_reg;
	color_dcc_d_dcc_skin_tone_segment0_0_RBUS d_dcc_skin_tone_segment0_0_reg;
	color_dcc_d_dcc_skin_tone_segment0_1_RBUS d_dcc_skin_tone_segment0_1_reg;
	color_dcc_d_dcc_skin_tone_segment_c0_0_RBUS d_dcc_skin_tone_segment_c0_0_reg; //20150708 chance_qian
	color_dcc_d_dcc_skin_tone_segment_c0_1_RBUS d_dcc_skin_tone_segment_c0_1_reg; //20150708 chance_qian

	color_dcc_d_dcc_skin_tone_yuv_center_1_RBUS d_dcc_skin_tone_yuv_center_1_reg;
	color_dcc_d_dcc_skin_tone_yuv_center_10_RBUS d_dcc_skin_tone_yuv_center_10_reg;
	color_dcc_d_dcc_skin_tone_yuv_range_1_RBUS d_dcc_skin_tone_yuv_range_1_reg;
    color_dcc_d_dcc_skin_tone_hsi_1_center_width_0_RBUS	d_dcc_skin_tone_hsi_1_center_width_0_reg;
    color_dcc_d_dcc_skin_tone_hsi_1_center_width_1_RBUS	d_dcc_skin_tone_hsi_1_center_width_1_reg;
	color_dcc_d_dcc_skin_tone_segment1_0_RBUS d_dcc_skin_tone_segment1_0_reg;
	color_dcc_d_dcc_skin_tone_segment1_1_RBUS d_dcc_skin_tone_segment1_1_reg;
	color_dcc_d_dcc_skin_tone_segment_c1_0_RBUS d_dcc_skin_tone_segment_c1_0_reg; //20150708 chance_qian
	color_dcc_d_dcc_skin_tone_segment_c1_1_RBUS d_dcc_skin_tone_segment_c1_1_reg; //20150708 chance_qian

	color_dcc_d_dcc_skin_tone_yuv_center_2_RBUS d_dcc_skin_tone_yuv_center_2_reg;
	color_dcc_d_dcc_skin_tone_yuv_center_20_RBUS d_dcc_skin_tone_yuv_center_20_reg;
	color_dcc_d_dcc_skin_tone_yuv_range_2_RBUS d_dcc_skin_tone_yuv_range_2_reg;
    color_dcc_d_dcc_skin_tone_hsi_2_center_width_0_RBUS	d_dcc_skin_tone_hsi_2_center_width_0_reg;
    color_dcc_d_dcc_skin_tone_hsi_2_center_width_1_RBUS	d_dcc_skin_tone_hsi_2_center_width_1_reg;
	color_dcc_d_dcc_skin_tone_segment2_0_RBUS d_dcc_skin_tone_segment2_0_reg;
	color_dcc_d_dcc_skin_tone_segment2_1_RBUS d_dcc_skin_tone_segment2_1_reg;
	color_dcc_d_dcc_skin_tone_segment_c2_0_RBUS d_dcc_skin_tone_segment_c2_0_reg; //20150708 chance_qian
	color_dcc_d_dcc_skin_tone_segment_c2_1_RBUS d_dcc_skin_tone_segment_c2_1_reg; //20150708 chance_qian

	color_dcc_d_dcc_skin_tone_yuv_center_3_RBUS d_dcc_skin_tone_yuv_center_3_reg;
	color_dcc_d_dcc_skin_tone_yuv_center_30_RBUS d_dcc_skin_tone_yuv_center_30_reg;
	color_dcc_d_dcc_skin_tone_yuv_range_3_RBUS d_dcc_skin_tone_yuv_range_3_reg;
    color_dcc_d_dcc_skin_tone_hsi_3_center_width_0_RBUS	d_dcc_skin_tone_hsi_3_center_width_0_reg;
    color_dcc_d_dcc_skin_tone_hsi_3_center_width_1_RBUS	d_dcc_skin_tone_hsi_3_center_width_1_reg;
	color_dcc_d_dcc_skin_tone_segment3_0_RBUS d_dcc_skin_tone_segment3_0_reg;
	color_dcc_d_dcc_skin_tone_segment3_1_RBUS d_dcc_skin_tone_segment3_1_reg;
	color_dcc_d_dcc_skin_tone_segment_c3_0_RBUS d_dcc_skin_tone_segment_c3_0_reg; //20150708 chance_qian
	color_dcc_d_dcc_skin_tone_segment_c3_1_RBUS d_dcc_skin_tone_segment_c3_1_reg; //20150708 chance_qian

	color_dcc_d_dcc_cds_skin_0_RBUS   color_dcc_d_dcc_cds_skin_0_reg;
        color_dcc_d_dcc_cds_skin_0_reg.regValue    = rtd_inl(COLOR_DCC_D_DCC_CDS_SKIN_0_reg);  //20190611 tulia_wang

	d_dcc_skin_tone_en_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_EN_reg);

	color_dcc_d_dcc_main_offset_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_MAIN_OFFSET_reg);    //20170524 robin_zhang
	color_dcc_d_dcc_sub_offset_reg.regValue   = rtd_inl(COLOR_DCC_D_DCC_SUB_OFFSET_reg);     //20170524 robin_zhang

	d_dcc_skin_tone_yuv_center_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_0_reg);
	d_dcc_skin_tone_yuv_center_00_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_00_reg);
	d_dcc_skin_tone_yuv_range_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_RANGE_0_reg);
    d_dcc_skin_tone_hsi_0_center_width_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_0_CENTER_WIDTH_0_reg);
    d_dcc_skin_tone_hsi_0_center_width_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_0_CENTER_WIDTH_1_reg);
	d_dcc_skin_tone_segment0_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT0_0_reg);
	d_dcc_skin_tone_segment0_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT0_1_reg);
	d_dcc_skin_tone_segment_c0_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C0_0_reg); //20150708 chance_qian
	d_dcc_skin_tone_segment_c0_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C0_1_reg); //20150708 chance_qian

	d_dcc_skin_tone_yuv_center_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_1_reg);
	d_dcc_skin_tone_yuv_center_10_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_10_reg);
	d_dcc_skin_tone_yuv_range_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_RANGE_1_reg);
    d_dcc_skin_tone_hsi_1_center_width_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_1_CENTER_WIDTH_0_reg);
    d_dcc_skin_tone_hsi_1_center_width_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_1_CENTER_WIDTH_1_reg);
	d_dcc_skin_tone_segment1_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT1_0_reg);
	d_dcc_skin_tone_segment1_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT1_1_reg);
	d_dcc_skin_tone_segment_c1_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C1_0_reg); //20150708 chance_qian
	d_dcc_skin_tone_segment_c1_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C1_1_reg); //20150708 chance_qian

	d_dcc_skin_tone_yuv_center_2_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_2_reg);
	d_dcc_skin_tone_yuv_center_20_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_20_reg);
	d_dcc_skin_tone_yuv_range_2_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_RANGE_2_reg);
    d_dcc_skin_tone_hsi_2_center_width_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_2_CENTER_WIDTH_0_reg);
    d_dcc_skin_tone_hsi_2_center_width_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_2_CENTER_WIDTH_1_reg);
	d_dcc_skin_tone_segment2_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT2_0_reg);
	d_dcc_skin_tone_segment2_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT2_1_reg);
	d_dcc_skin_tone_segment_c2_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C2_0_reg); //20150708 chance_qian
	d_dcc_skin_tone_segment_c2_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C2_1_reg); //20150708 chance_qian

	d_dcc_skin_tone_yuv_center_3_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_3_reg);
	d_dcc_skin_tone_yuv_center_30_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_30_reg);
	d_dcc_skin_tone_yuv_range_3_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_RANGE_3_reg);
    d_dcc_skin_tone_hsi_3_center_width_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_3_CENTER_WIDTH_0_reg);
    d_dcc_skin_tone_hsi_3_center_width_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_3_CENTER_WIDTH_1_reg);
	d_dcc_skin_tone_segment3_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT3_0_reg);
	d_dcc_skin_tone_segment3_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT3_1_reg);
	d_dcc_skin_tone_segment_c3_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C3_0_reg); //20150708 chance_qian
	d_dcc_skin_tone_segment_c3_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C3_1_reg); //20150708 chance_qian

	#if 0// 191107, mark for AI
	d_dcc_skin_tone_en_reg.y_blending_en = ptr->S_DCC_Skin_Tone_Settings.y_blending_en;// 1;
	#endif// end 191107, mark for AI
	d_dcc_skin_tone_en_reg.y_blending_mode = ptr->S_DCC_Skin_Tone_Settings.y_blending_mode;// 1;
 	#if 0// 191107, mark for AI
    d_dcc_skin_tone_en_reg.colorspace_sel = ptr->S_DCC_Skin_Tone_Settings.colorspace_sel;
        d_dcc_skin_tone_en_reg.region_ratio_blending_en =  ptr->S_DCC_Skin_Tone_Settings.region_ratio_blending_en ;
	color_dcc_d_dcc_cds_skin_0_reg.cds_skin_en = ptr->S_DCC_Skin_Tone_Settings.cds_skin_en;
	/*VIPprintf("en =%d, mode =%d\n",*(DCC_skin_tone_TABLE+1*8+0),*(DCC_skin_tone_TABLE+1*8+1));*/
	/*VIPprintf("y_center =%d, u_center =%d, v_center =%d\n",*(DCC_skin_tone_TABLE+1*8+2),*(DCC_skin_tone_TABLE+1*8+3),*(DCC_skin_tone_TABLE+1*8+4));*/
	/*VIPprintf("y_range =%d, u_range =%d, v_range =%d\n",*(DCC_skin_tone_TABLE+1*8+5),*(DCC_skin_tone_TABLE+1*8+6),*(DCC_skin_tone_TABLE+1*8+7));*/

	d_dcc_skin_tone_en_reg.y_blending_reg0_en = 							ptr->S_DCC_Skin_Tone_Y_Region0_Coef.region0_en;
	d_dcc_skin_tone_en_reg.y_blending_0_enhance_en = 						ptr->S_DCC_Skin_Tone_Y_Region0_Coef.region0_enhance_en; //20150708 chance_qian
	#endif// end 191107, mark for AI

	color_dcc_d_dcc_main_offset_reg.main_offset_en     = ptr->S_DCC_Skin_Tone_Settings.main_offset_en;         //20170524 robin_zhang
	color_dcc_d_dcc_main_offset_reg.main_offset_range = ptr->S_DCC_Skin_Tone_Settings.main_offset_range;    //20170524 robin_zhang
	color_dcc_d_dcc_main_offset_reg.main_offset           = ptr->S_DCC_Skin_Tone_Settings.main_offset;             //20170524 robin_zhang
	color_dcc_d_dcc_sub_offset_reg.sub_offset_en          = ptr->S_DCC_Skin_Tone_Settings.sub_offset_en;         //20170524 robin_zhang
	color_dcc_d_dcc_sub_offset_reg.sub_offset_range     = ptr->S_DCC_Skin_Tone_Settings.sub_offset_range;    //20170524 robin_zhang
	color_dcc_d_dcc_sub_offset_reg.sub_offset               = ptr->S_DCC_Skin_Tone_Settings.sub_offset;              //20170524 robin_zhang

	#if 0// 191107, mark for AI
	d_dcc_skin_tone_yuv_range_0_reg.y_blending_0_y_center =  				ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Y_Center0;
	d_dcc_skin_tone_yuv_center_0_reg.y_blending_0_u_center =  				ptr->S_DCC_Skin_Tone_Y_Region0_Coef.U_Center0;
	d_dcc_skin_tone_yuv_center_0_reg.y_blending_0_v_center =  				ptr->S_DCC_Skin_Tone_Y_Region0_Coef.V_Center0;
	d_dcc_skin_tone_yuv_range_0_reg.y_blending_0_y_range = 					ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Y_Range0;
	d_dcc_skin_tone_yuv_range_0_reg.y_blending_0_u_range = 					ptr->S_DCC_Skin_Tone_Y_Region0_Coef.U_Range0;
	d_dcc_skin_tone_yuv_range_0_reg.y_blending_0_v_range =					ptr->S_DCC_Skin_Tone_Y_Region0_Coef.V_Range0;
	#endif// end 191107, mark for AI
    d_dcc_skin_tone_hsi_0_center_width_0_reg.y_blending_0_h_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region0_Coef.H_CenterWid0;
    d_dcc_skin_tone_hsi_0_center_width_1_reg.y_blending_0_s_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region0_Coef.S_CenterWid0;
    d_dcc_skin_tone_hsi_0_center_width_1_reg.y_blending_0_i_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region0_Coef.I_CenterWid0;

	#if 0// 191107, mark for AI
	Segment0_0 = ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_0) << 24)+
				((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_1) << 16) +
				((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_2) << 8) +
				((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_3));
	Segment0_1 = ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_4) << 24)+
				((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_5) << 16) +
				((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_6) << 8) +
				((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_7));

	d_dcc_skin_tone_segment0_0_reg.regValue = Segment0_0;
	d_dcc_skin_tone_segment0_1_reg.regValue = Segment0_1;
	#endif// end 191107, mark for AI

	Segment0_0_c = ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_0_c) << 24)+	//20150708 chance_qian
				   ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_1_c) << 16) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_2_c) << 8) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_3_c));
	Segment0_1_c = ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_4_c) << 24)+
				   ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_5_c) << 16) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_6_c) << 8) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_7_c));

	d_dcc_skin_tone_segment_c0_0_reg.regValue = Segment0_0_c; //20150708 chance_qian
	d_dcc_skin_tone_segment_c0_1_reg.regValue = Segment0_1_c; //20150708 chance_qian

	d_dcc_skin_tone_en_reg.y_blending_reg1_en = 							ptr->S_DCC_Skin_Tone_Y_Region1_Coef.region1_en;
	d_dcc_skin_tone_en_reg.y_blending_1_enhance_en = 						ptr->S_DCC_Skin_Tone_Y_Region1_Coef.region1_enhance_en; //20150708 chance_qian
	d_dcc_skin_tone_yuv_center_10_reg.y_blending_1_y_center = 				ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Y_Center1;
	d_dcc_skin_tone_yuv_center_1_reg.y_blending_1_u_center = 				ptr->S_DCC_Skin_Tone_Y_Region1_Coef.U_Center1;
	d_dcc_skin_tone_yuv_center_1_reg.y_blending_1_v_center = 				ptr->S_DCC_Skin_Tone_Y_Region1_Coef.V_Center1;
	d_dcc_skin_tone_yuv_range_1_reg.y_blending_1_y_range = 					ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Y_Range1;
	d_dcc_skin_tone_yuv_range_1_reg.y_blending_1_u_range = 					ptr->S_DCC_Skin_Tone_Y_Region1_Coef.U_Range1;
	d_dcc_skin_tone_yuv_range_1_reg.y_blending_1_v_range = 					ptr->S_DCC_Skin_Tone_Y_Region1_Coef.V_Range1;
    d_dcc_skin_tone_hsi_1_center_width_0_reg.y_blending_1_h_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region1_Coef.H_CenterWid1;
    d_dcc_skin_tone_hsi_1_center_width_1_reg.y_blending_1_s_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region1_Coef.S_CenterWid1;
    d_dcc_skin_tone_hsi_1_center_width_1_reg.y_blending_1_i_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region1_Coef.I_CenterWid1;

	Segment1_0 = ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_0) << 24)+
				((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_1) << 16) +
				((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_2) << 8) +
				((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_3));
	Segment1_1 = ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_4) << 24)+
				((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_5) << 16) +
				((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_6) << 8) +
				((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_7));

	d_dcc_skin_tone_segment1_0_reg.regValue = Segment1_0;
	d_dcc_skin_tone_segment1_1_reg.regValue = Segment1_1;

	Segment1_0_c = ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_0_c) << 24)+	//20150708 chance_qian
				   ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_1_c) << 16) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_2_c) << 8) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_3_c));
	Segment1_1_c = ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_4_c) << 24)+
				   ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_5_c) << 16) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_6_c) << 8) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_7_c));

	d_dcc_skin_tone_segment_c1_0_reg.regValue = Segment1_0_c; //20150708 chance_qian
	d_dcc_skin_tone_segment_c1_1_reg.regValue = Segment1_1_c; //20150708 chance_qian

	d_dcc_skin_tone_en_reg.y_blending_reg2_en = 							ptr->S_DCC_Skin_Tone_Y_Region2_Coef.region2_en;
	d_dcc_skin_tone_en_reg.y_blending_2_enhance_en = 						ptr->S_DCC_Skin_Tone_Y_Region2_Coef.region2_enhance_en; //20150708 chance_qian
	d_dcc_skin_tone_yuv_center_20_reg.y_blending_2_y_center = 				ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Y_Center2;
	d_dcc_skin_tone_yuv_center_2_reg.y_blending_2_u_center = 				ptr->S_DCC_Skin_Tone_Y_Region2_Coef.U_Center2;
	d_dcc_skin_tone_yuv_center_2_reg.y_blending_2_v_center = 				ptr->S_DCC_Skin_Tone_Y_Region2_Coef.V_Center2;
	d_dcc_skin_tone_yuv_range_2_reg.y_blending_2_y_range = 					ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Y_Range2;
	d_dcc_skin_tone_yuv_range_2_reg.y_blending_2_u_range = 					ptr->S_DCC_Skin_Tone_Y_Region2_Coef.U_Range2;
	d_dcc_skin_tone_yuv_range_2_reg.y_blending_2_v_range = 					ptr->S_DCC_Skin_Tone_Y_Region2_Coef.V_Range2;
    d_dcc_skin_tone_hsi_2_center_width_0_reg.y_blending_2_h_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region2_Coef.H_CenterWid2;
    d_dcc_skin_tone_hsi_2_center_width_1_reg.y_blending_2_s_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region2_Coef.S_CenterWid2;
    d_dcc_skin_tone_hsi_2_center_width_1_reg.y_blending_2_i_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region2_Coef.I_CenterWid2;

	Segment2_0 = ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_0) << 24)+
				((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_1) << 16) +
				((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_2) << 8) +
				((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_3));
	Segment2_1 = ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_4) << 24)+
				((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_5) << 16) +
				((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_6) << 8) +
				((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_7));

	d_dcc_skin_tone_segment2_0_reg.regValue = Segment2_0;
	d_dcc_skin_tone_segment2_1_reg.regValue = Segment2_1;

	Segment2_0_c = ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_0_c) << 24)+	//20150708 chance_qian
				   ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_1_c) << 16) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_2_c) << 8) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_3_c));
	Segment2_1_c = ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_4_c) << 24)+
				   ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_5_c) << 16) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_6_c) << 8) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_7_c));

	d_dcc_skin_tone_segment_c2_0_reg.regValue = Segment2_0_c; //20150708 chance_qian
	d_dcc_skin_tone_segment_c2_1_reg.regValue = Segment2_1_c; //20150708 chance_qian

	d_dcc_skin_tone_en_reg.y_blending_reg3_en = 							ptr->S_DCC_Skin_Tone_Y_Region3_Coef.region3_en;
	d_dcc_skin_tone_en_reg.y_blending_3_enhance_en = 						ptr->S_DCC_Skin_Tone_Y_Region3_Coef.region3_enhance_en; //20150708 chance_qian
	d_dcc_skin_tone_yuv_center_30_reg.y_blending_3_y_center =					ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Y_Center3;
	d_dcc_skin_tone_yuv_center_3_reg.y_blending_3_u_center = 				ptr->S_DCC_Skin_Tone_Y_Region3_Coef.U_Center3;
	d_dcc_skin_tone_yuv_center_3_reg.y_blending_3_v_center = 				ptr->S_DCC_Skin_Tone_Y_Region3_Coef.V_Center3;
	d_dcc_skin_tone_yuv_range_3_reg.y_blending_3_y_range = 					ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Y_Range3;
	d_dcc_skin_tone_yuv_range_3_reg.y_blending_3_u_range = 					ptr->S_DCC_Skin_Tone_Y_Region3_Coef.U_Range3;
	d_dcc_skin_tone_yuv_range_3_reg.y_blending_3_v_range = 					ptr->S_DCC_Skin_Tone_Y_Region3_Coef.V_Range3;
    d_dcc_skin_tone_hsi_3_center_width_0_reg.y_blending_3_h_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region3_Coef.H_CenterWid3;
    d_dcc_skin_tone_hsi_3_center_width_1_reg.y_blending_3_s_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region3_Coef.S_CenterWid3;
    d_dcc_skin_tone_hsi_3_center_width_1_reg.y_blending_3_i_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region3_Coef.I_CenterWid3;

	Segment3_0 = ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_0) << 24)+
				((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_1) << 16) +
				((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_2) << 8) +
				((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_3));
	Segment3_1 = ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_4) << 24)+
				((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_5) << 16) +
				((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_6) << 8) +
				((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_7));

	d_dcc_skin_tone_segment3_0_reg.regValue = Segment3_0;
	d_dcc_skin_tone_segment3_1_reg.regValue = Segment3_1;

	Segment3_0_c = ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_0_c) << 24)+	//20150708 chance_qian
				   ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_1_c) << 16) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_2_c) << 8) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_3_c));
	Segment3_1_c = ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_4_c) << 24)+
				   ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_5_c) << 16) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_6_c) << 8) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_7_c));

	d_dcc_skin_tone_segment_c3_0_reg.regValue = Segment3_0_c; //20150708 chance_qian
	d_dcc_skin_tone_segment_c3_1_reg.regValue = Segment3_1_c; //20150708 chance_qian

	/*VIPprintf("2.en =%d, mode =%d\n", d_dcc_skin_tone_en_reg.y_blending_en, d_dcc_skin_tone_en_reg.y_blending_mode);*/
	/*VIPprintf("2.y_center =%d, u_center =%d, v_center =%d\n", d_dcc_skin_tone_yuv_center_reg.y_blending_y_center , d_dcc_skin_tone_yuv_center_reg.y_blending_u_center, d_dcc_skin_tone_yuv_center_reg.y_blending_v_center);*/
	/*VIPprintf("2.y_range =%d, u_range =%d, v_range =%d\n", d_dcc_skin_tone_yuv_range_reg.y_blending_y_range, d_dcc_skin_tone_yuv_range_reg.y_blending_u_range, d_dcc_skin_tone_yuv_range_reg.y_blending_v_range);*/


	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_EN_reg, d_dcc_skin_tone_en_reg.regValue);

	rtd_outl(COLOR_DCC_D_DCC_MAIN_OFFSET_reg, color_dcc_d_dcc_main_offset_reg.regValue);    //20170524 robin_zhang
	rtd_outl(COLOR_DCC_D_DCC_SUB_OFFSET_reg, color_dcc_d_dcc_sub_offset_reg.regValue);       //20170524 robin_zhang

	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_0_reg, d_dcc_skin_tone_yuv_center_0_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_00_reg, d_dcc_skin_tone_yuv_center_00_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_RANGE_0_reg, d_dcc_skin_tone_yuv_range_0_reg.regValue);
    rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_0_CENTER_WIDTH_0_reg, d_dcc_skin_tone_hsi_0_center_width_0_reg.regValue);
    rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_0_CENTER_WIDTH_1_reg, d_dcc_skin_tone_hsi_0_center_width_1_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT0_0_reg, d_dcc_skin_tone_segment0_0_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT0_1_reg, d_dcc_skin_tone_segment0_1_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C0_0_reg, d_dcc_skin_tone_segment_c0_0_reg.regValue); //20150708 chance_qian
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C0_1_reg, d_dcc_skin_tone_segment_c0_1_reg.regValue); //20150708 chance_qian

	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_1_reg, d_dcc_skin_tone_yuv_center_1_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_10_reg, d_dcc_skin_tone_yuv_center_10_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_RANGE_1_reg, d_dcc_skin_tone_yuv_range_1_reg.regValue);
    rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_1_CENTER_WIDTH_0_reg, d_dcc_skin_tone_hsi_1_center_width_0_reg.regValue);
    rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_1_CENTER_WIDTH_1_reg, d_dcc_skin_tone_hsi_1_center_width_1_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT1_0_reg, d_dcc_skin_tone_segment1_0_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT1_1_reg, d_dcc_skin_tone_segment1_1_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C1_0_reg, d_dcc_skin_tone_segment_c1_0_reg.regValue); //20150708 chance_qian
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C1_1_reg, d_dcc_skin_tone_segment_c1_1_reg.regValue); //20150708 chance_qian

	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_2_reg, d_dcc_skin_tone_yuv_center_2_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_20_reg, d_dcc_skin_tone_yuv_center_20_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_RANGE_2_reg, d_dcc_skin_tone_yuv_range_2_reg.regValue);
    rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_2_CENTER_WIDTH_0_reg, d_dcc_skin_tone_hsi_2_center_width_0_reg.regValue);
    rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_2_CENTER_WIDTH_1_reg, d_dcc_skin_tone_hsi_2_center_width_1_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT2_0_reg, d_dcc_skin_tone_segment2_0_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT2_1_reg, d_dcc_skin_tone_segment2_1_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C2_0_reg, d_dcc_skin_tone_segment_c2_0_reg.regValue); //20150708 chance_qian
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C2_1_reg, d_dcc_skin_tone_segment_c2_1_reg.regValue); //20150708 chance_qian

	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_3_reg, d_dcc_skin_tone_yuv_center_3_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_30_reg, d_dcc_skin_tone_yuv_center_30_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_RANGE_3_reg, d_dcc_skin_tone_yuv_range_3_reg.regValue);
    rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_3_CENTER_WIDTH_0_reg, d_dcc_skin_tone_hsi_3_center_width_0_reg.regValue);
    rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_3_CENTER_WIDTH_1_reg, d_dcc_skin_tone_hsi_3_center_width_1_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT3_0_reg, d_dcc_skin_tone_segment3_0_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT3_1_reg, d_dcc_skin_tone_segment3_1_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C3_0_reg, d_dcc_skin_tone_segment_c3_0_reg.regValue); //20150708 chance_qian
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C3_1_reg, d_dcc_skin_tone_segment_c3_1_reg.regValue); //20150708 chance_qian

	rtd_outl(COLOR_DCC_D_DCC_CDS_SKIN_0_reg, color_dcc_d_dcc_cds_skin_0_reg.regValue); //tulia_wang
//#endif
}


void drvif_color_set_dcc_Color_Independent(VIP_DCC_Color_Independent_Blending_Table *ptr)
{
//#ifdef VPQ_COMPILER_ERROR_ENABLE_MERLIN6_
	UINT32 Segment0_0 = 0, Segment0_1 = 0, Segment1_0 = 0, Segment1_1 = 0, Segment2_0 = 0, Segment2_1 = 0, Segment3_0 = 0, Segment3_1 = 0;
	UINT32 Segment0_0_c = 0, Segment0_1_c = 0, Segment1_0_c = 0, Segment1_1_c = 0, Segment2_0_c = 0, Segment2_1_c = 0, Segment3_0_c = 0, Segment3_1_c = 0; //20150708 chance_qian

	color_dcc_d_dcc_skin_tone_en_RBUS d_dcc_skin_tone_en_reg;

	color_dcc_d_dcc_main_offset_RBUS color_dcc_d_dcc_main_offset_reg;    //20170524 robin_zhang
	color_dcc_d_dcc_sub_offset_RBUS   color_dcc_d_dcc_sub_offset_reg;    //20170524 robin_zhang

	color_dcc_d_dcc_skin_tone_yuv_center_0_RBUS d_dcc_skin_tone_yuv_center_0_reg;
	color_dcc_d_dcc_skin_tone_yuv_center_00_RBUS d_dcc_skin_tone_yuv_center_00_reg;
	color_dcc_d_dcc_skin_tone_yuv_range_0_RBUS d_dcc_skin_tone_yuv_range_0_reg;
    color_dcc_d_dcc_skin_tone_hsi_0_center_width_0_RBUS	d_dcc_skin_tone_hsi_0_center_width_0_reg;
    color_dcc_d_dcc_skin_tone_hsi_0_center_width_1_RBUS	d_dcc_skin_tone_hsi_0_center_width_1_reg;
	color_dcc_d_dcc_skin_tone_segment0_0_RBUS d_dcc_skin_tone_segment0_0_reg;
	color_dcc_d_dcc_skin_tone_segment0_1_RBUS d_dcc_skin_tone_segment0_1_reg;
	color_dcc_d_dcc_skin_tone_segment_c0_0_RBUS d_dcc_skin_tone_segment_c0_0_reg; //20150708 chance_qian
	color_dcc_d_dcc_skin_tone_segment_c0_1_RBUS d_dcc_skin_tone_segment_c0_1_reg; //20150708 chance_qian

	color_dcc_d_dcc_skin_tone_yuv_center_1_RBUS d_dcc_skin_tone_yuv_center_1_reg;
	color_dcc_d_dcc_skin_tone_yuv_center_10_RBUS d_dcc_skin_tone_yuv_center_10_reg;
	color_dcc_d_dcc_skin_tone_yuv_range_1_RBUS d_dcc_skin_tone_yuv_range_1_reg;
    color_dcc_d_dcc_skin_tone_hsi_1_center_width_0_RBUS	d_dcc_skin_tone_hsi_1_center_width_0_reg;
    color_dcc_d_dcc_skin_tone_hsi_1_center_width_1_RBUS	d_dcc_skin_tone_hsi_1_center_width_1_reg;
	color_dcc_d_dcc_skin_tone_segment1_0_RBUS d_dcc_skin_tone_segment1_0_reg;
	color_dcc_d_dcc_skin_tone_segment1_1_RBUS d_dcc_skin_tone_segment1_1_reg;
	color_dcc_d_dcc_skin_tone_segment_c1_0_RBUS d_dcc_skin_tone_segment_c1_0_reg; //20150708 chance_qian
	color_dcc_d_dcc_skin_tone_segment_c1_1_RBUS d_dcc_skin_tone_segment_c1_1_reg; //20150708 chance_qian

	color_dcc_d_dcc_skin_tone_yuv_center_2_RBUS d_dcc_skin_tone_yuv_center_2_reg;
	color_dcc_d_dcc_skin_tone_yuv_center_20_RBUS d_dcc_skin_tone_yuv_center_20_reg;
	color_dcc_d_dcc_skin_tone_yuv_range_2_RBUS d_dcc_skin_tone_yuv_range_2_reg;
    color_dcc_d_dcc_skin_tone_hsi_2_center_width_0_RBUS	d_dcc_skin_tone_hsi_2_center_width_0_reg;
    color_dcc_d_dcc_skin_tone_hsi_2_center_width_1_RBUS	d_dcc_skin_tone_hsi_2_center_width_1_reg;
	color_dcc_d_dcc_skin_tone_segment2_0_RBUS d_dcc_skin_tone_segment2_0_reg;
	color_dcc_d_dcc_skin_tone_segment2_1_RBUS d_dcc_skin_tone_segment2_1_reg;
	color_dcc_d_dcc_skin_tone_segment_c2_0_RBUS d_dcc_skin_tone_segment_c2_0_reg; //20150708 chance_qian
	color_dcc_d_dcc_skin_tone_segment_c2_1_RBUS d_dcc_skin_tone_segment_c2_1_reg; //20150708 chance_qian

	color_dcc_d_dcc_skin_tone_yuv_center_3_RBUS d_dcc_skin_tone_yuv_center_3_reg;
	color_dcc_d_dcc_skin_tone_yuv_center_30_RBUS d_dcc_skin_tone_yuv_center_30_reg;
	color_dcc_d_dcc_skin_tone_yuv_range_3_RBUS d_dcc_skin_tone_yuv_range_3_reg;
    color_dcc_d_dcc_skin_tone_hsi_3_center_width_0_RBUS	d_dcc_skin_tone_hsi_3_center_width_0_reg;
    color_dcc_d_dcc_skin_tone_hsi_3_center_width_1_RBUS	d_dcc_skin_tone_hsi_3_center_width_1_reg;
	color_dcc_d_dcc_skin_tone_segment3_0_RBUS d_dcc_skin_tone_segment3_0_reg;
	color_dcc_d_dcc_skin_tone_segment3_1_RBUS d_dcc_skin_tone_segment3_1_reg;
	color_dcc_d_dcc_skin_tone_segment_c3_0_RBUS d_dcc_skin_tone_segment_c3_0_reg; //20150708 chance_qian
	color_dcc_d_dcc_skin_tone_segment_c3_1_RBUS d_dcc_skin_tone_segment_c3_1_reg; //20150708 chance_qian

	color_dcc_d_dcc_cds_skin_0_RBUS   color_dcc_d_dcc_cds_skin_0_reg;
    color_dcc_d_dcc_cds_skin_0_reg.regValue    = rtd_inl(COLOR_DCC_D_DCC_CDS_SKIN_0_reg);  //20190611 tulia_wang
	d_dcc_skin_tone_en_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_EN_reg);
	color_dcc_d_dcc_main_offset_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_MAIN_OFFSET_reg);    //20170524 robin_zhang
	color_dcc_d_dcc_sub_offset_reg.regValue   = rtd_inl(COLOR_DCC_D_DCC_SUB_OFFSET_reg);     //20170524 robin_zhang
	d_dcc_skin_tone_yuv_center_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_0_reg);
	d_dcc_skin_tone_yuv_center_00_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_00_reg);
	d_dcc_skin_tone_yuv_range_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_RANGE_0_reg);
    d_dcc_skin_tone_hsi_0_center_width_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_0_CENTER_WIDTH_0_reg);
    d_dcc_skin_tone_hsi_0_center_width_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_0_CENTER_WIDTH_1_reg);
	d_dcc_skin_tone_segment0_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT0_0_reg);
	d_dcc_skin_tone_segment0_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT0_1_reg);
	d_dcc_skin_tone_segment_c0_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C0_0_reg); //20150708 chance_qian
	d_dcc_skin_tone_segment_c0_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C0_1_reg); //20150708 chance_qian

	d_dcc_skin_tone_yuv_center_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_1_reg);
	d_dcc_skin_tone_yuv_center_10_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_10_reg);
	d_dcc_skin_tone_yuv_range_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_RANGE_1_reg);
    d_dcc_skin_tone_hsi_1_center_width_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_1_CENTER_WIDTH_0_reg);
    d_dcc_skin_tone_hsi_1_center_width_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_1_CENTER_WIDTH_1_reg);
	d_dcc_skin_tone_segment1_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT1_0_reg);
	d_dcc_skin_tone_segment1_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT1_1_reg);
	d_dcc_skin_tone_segment_c1_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C1_0_reg); //20150708 chance_qian
	d_dcc_skin_tone_segment_c1_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C1_1_reg); //20150708 chance_qian

	d_dcc_skin_tone_yuv_center_2_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_2_reg);
	d_dcc_skin_tone_yuv_center_20_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_20_reg);
	d_dcc_skin_tone_yuv_range_2_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_RANGE_2_reg);
    d_dcc_skin_tone_hsi_2_center_width_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_2_CENTER_WIDTH_0_reg);
    d_dcc_skin_tone_hsi_2_center_width_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_2_CENTER_WIDTH_1_reg);
	d_dcc_skin_tone_segment2_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT2_0_reg);
	d_dcc_skin_tone_segment2_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT2_1_reg);
	d_dcc_skin_tone_segment_c2_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C2_0_reg); //20150708 chance_qian
	d_dcc_skin_tone_segment_c2_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C2_1_reg); //20150708 chance_qian

	d_dcc_skin_tone_yuv_center_3_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_3_reg);
	d_dcc_skin_tone_yuv_center_30_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_30_reg);
	d_dcc_skin_tone_yuv_range_3_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_RANGE_3_reg);
    d_dcc_skin_tone_hsi_3_center_width_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_3_CENTER_WIDTH_0_reg);
    d_dcc_skin_tone_hsi_3_center_width_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_3_CENTER_WIDTH_1_reg);
	d_dcc_skin_tone_segment3_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT3_0_reg);
	d_dcc_skin_tone_segment3_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT3_1_reg);
	d_dcc_skin_tone_segment_c3_0_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C3_0_reg); //20150708 chance_qian
	d_dcc_skin_tone_segment_c3_1_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C3_1_reg); //20150708 chance_qian

	d_dcc_skin_tone_en_reg.y_blending_en = ptr->S_DCC_Skin_Tone_Settings.y_blending_en;// 1;
	d_dcc_skin_tone_en_reg.y_blending_mode = ptr->S_DCC_Skin_Tone_Settings.y_blending_mode;// 1;
    d_dcc_skin_tone_en_reg.colorspace_sel = ptr->S_DCC_Skin_Tone_Settings.colorspace_sel;
	#if 1 // lesley TBD
        d_dcc_skin_tone_en_reg.region_ratio_blending_en =  1;//ptr->S_DCC_Skin_Tone_Settings.region_ratio_blending_en ;
	color_dcc_d_dcc_cds_skin_0_reg.cds_skin_en = 0;//ptr->S_DCC_Skin_Tone_Settings.cds_skin_en;
	#endif 
	/*VIPprintf("en =%d, mode =%d\n",*(DCC_skin_tone_TABLE+1*8+0),*(DCC_skin_tone_TABLE+1*8+1));*/
	/*VIPprintf("y_center =%d, u_center =%d, v_center =%d\n",*(DCC_skin_tone_TABLE+1*8+2),*(DCC_skin_tone_TABLE+1*8+3),*(DCC_skin_tone_TABLE+1*8+4));*/
	/*VIPprintf("y_range =%d, u_range =%d, v_range =%d\n",*(DCC_skin_tone_TABLE+1*8+5),*(DCC_skin_tone_TABLE+1*8+6),*(DCC_skin_tone_TABLE+1*8+7));*/

	d_dcc_skin_tone_en_reg.y_blending_reg0_en = 							ptr->S_DCC_Skin_Tone_Y_Region0_Coef.region0_en;
	d_dcc_skin_tone_en_reg.y_blending_0_enhance_en = 						ptr->S_DCC_Skin_Tone_Y_Region0_Coef.region0_enhance_en; //20150708 chance_qian

	color_dcc_d_dcc_main_offset_reg.main_offset_en     = ptr->S_DCC_Skin_Tone_Settings.main_offset_en;         //20170524 robin_zhang
	color_dcc_d_dcc_main_offset_reg.main_offset_range = ptr->S_DCC_Skin_Tone_Settings.main_offset_range;    //20170524 robin_zhang
	color_dcc_d_dcc_main_offset_reg.main_offset           = ptr->S_DCC_Skin_Tone_Settings.main_offset;             //20170524 robin_zhang
	color_dcc_d_dcc_sub_offset_reg.sub_offset_en          = ptr->S_DCC_Skin_Tone_Settings.sub_offset_en;         //20170524 robin_zhang
	color_dcc_d_dcc_sub_offset_reg.sub_offset_range     = ptr->S_DCC_Skin_Tone_Settings.sub_offset_range;    //20170524 robin_zhang
	color_dcc_d_dcc_sub_offset_reg.sub_offset               = ptr->S_DCC_Skin_Tone_Settings.sub_offset;              //20170524 robin_zhang

	d_dcc_skin_tone_yuv_center_00_reg.y_blending_0_y_center =  				ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Y_Center0;
	d_dcc_skin_tone_yuv_center_0_reg.y_blending_0_u_center =  				ptr->S_DCC_Skin_Tone_Y_Region0_Coef.U_Center0;
	d_dcc_skin_tone_yuv_center_0_reg.y_blending_0_v_center =  				ptr->S_DCC_Skin_Tone_Y_Region0_Coef.V_Center0;
	d_dcc_skin_tone_yuv_range_0_reg.y_blending_0_y_range = 					ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Y_Range0;
	d_dcc_skin_tone_yuv_range_0_reg.y_blending_0_u_range = 					ptr->S_DCC_Skin_Tone_Y_Region0_Coef.U_Range0;
	d_dcc_skin_tone_yuv_range_0_reg.y_blending_0_v_range =					ptr->S_DCC_Skin_Tone_Y_Region0_Coef.V_Range0;
    d_dcc_skin_tone_hsi_0_center_width_0_reg.y_blending_0_h_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region0_Coef.H_CenterWid0;
    d_dcc_skin_tone_hsi_0_center_width_1_reg.y_blending_0_s_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region0_Coef.S_CenterWid0;
    d_dcc_skin_tone_hsi_0_center_width_1_reg.y_blending_0_i_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region0_Coef.I_CenterWid0;

	Segment0_0 = ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_0) << 24)+
				((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_1) << 16) +
				((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_2) << 8) +
				((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_3));
	Segment0_1 = ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_4) << 24)+
				((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_5) << 16) +
				((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_6) << 8) +
				((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_7));

	d_dcc_skin_tone_segment0_0_reg.regValue = Segment0_0;
	d_dcc_skin_tone_segment0_1_reg.regValue = Segment0_1;

	Segment0_0_c = ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_0_c) << 24)+	//20150708 chance_qian
				   ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_1_c) << 16) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_2_c) << 8) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_3_c));
	Segment0_1_c = ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_4_c) << 24)+
				   ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_5_c) << 16) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_6_c) << 8) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region0_Coef.Segment0_7_c));

	d_dcc_skin_tone_segment_c0_0_reg.regValue = Segment0_0_c; //20150708 chance_qian
	d_dcc_skin_tone_segment_c0_1_reg.regValue = Segment0_1_c; //20150708 chance_qian

	d_dcc_skin_tone_en_reg.y_blending_reg1_en = 							ptr->S_DCC_Skin_Tone_Y_Region1_Coef.region1_en;
	d_dcc_skin_tone_en_reg.y_blending_1_enhance_en = 						ptr->S_DCC_Skin_Tone_Y_Region1_Coef.region1_enhance_en; //20150708 chance_qian
	d_dcc_skin_tone_yuv_center_10_reg.y_blending_1_y_center = 				ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Y_Center1;
	d_dcc_skin_tone_yuv_center_1_reg.y_blending_1_u_center = 				ptr->S_DCC_Skin_Tone_Y_Region1_Coef.U_Center1;
	d_dcc_skin_tone_yuv_center_1_reg.y_blending_1_v_center = 				ptr->S_DCC_Skin_Tone_Y_Region1_Coef.V_Center1;
	d_dcc_skin_tone_yuv_range_1_reg.y_blending_1_y_range = 					ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Y_Range1;
	d_dcc_skin_tone_yuv_range_1_reg.y_blending_1_u_range = 					ptr->S_DCC_Skin_Tone_Y_Region1_Coef.U_Range1;
	d_dcc_skin_tone_yuv_range_1_reg.y_blending_1_v_range = 					ptr->S_DCC_Skin_Tone_Y_Region1_Coef.V_Range1;
    d_dcc_skin_tone_hsi_1_center_width_0_reg.y_blending_1_h_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region1_Coef.H_CenterWid1;
    d_dcc_skin_tone_hsi_1_center_width_1_reg.y_blending_1_s_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region1_Coef.S_CenterWid1;
    d_dcc_skin_tone_hsi_1_center_width_1_reg.y_blending_1_i_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region1_Coef.I_CenterWid1;

	Segment1_0 = ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_0) << 24)+
				((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_1) << 16) +
				((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_2) << 8) +
				((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_3));
	Segment1_1 = ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_4) << 24)+
				((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_5) << 16) +
				((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_6) << 8) +
				((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_7));

	d_dcc_skin_tone_segment1_0_reg.regValue = Segment1_0;
	d_dcc_skin_tone_segment1_1_reg.regValue = Segment1_1;

	Segment1_0_c = ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_0_c) << 24)+	//20150708 chance_qian
				   ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_1_c) << 16) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_2_c) << 8) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_3_c));
	Segment1_1_c = ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_4_c) << 24)+
				   ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_5_c) << 16) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_6_c) << 8) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region1_Coef.Segment1_7_c));

	d_dcc_skin_tone_segment_c1_0_reg.regValue = Segment1_0_c; //20150708 chance_qian
	d_dcc_skin_tone_segment_c1_1_reg.regValue = Segment1_1_c; //20150708 chance_qian

	d_dcc_skin_tone_en_reg.y_blending_reg2_en = 							ptr->S_DCC_Skin_Tone_Y_Region2_Coef.region2_en;
	d_dcc_skin_tone_en_reg.y_blending_2_enhance_en = 						ptr->S_DCC_Skin_Tone_Y_Region2_Coef.region2_enhance_en; //20150708 chance_qian
	d_dcc_skin_tone_yuv_center_20_reg.y_blending_2_y_center = 				ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Y_Center2;
	d_dcc_skin_tone_yuv_center_2_reg.y_blending_2_u_center = 				ptr->S_DCC_Skin_Tone_Y_Region2_Coef.U_Center2;
	d_dcc_skin_tone_yuv_center_2_reg.y_blending_2_v_center = 				ptr->S_DCC_Skin_Tone_Y_Region2_Coef.V_Center2;
	d_dcc_skin_tone_yuv_range_2_reg.y_blending_2_y_range = 					ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Y_Range2;
	d_dcc_skin_tone_yuv_range_2_reg.y_blending_2_u_range = 					ptr->S_DCC_Skin_Tone_Y_Region2_Coef.U_Range2;
	d_dcc_skin_tone_yuv_range_2_reg.y_blending_2_v_range = 					ptr->S_DCC_Skin_Tone_Y_Region2_Coef.V_Range2;
    d_dcc_skin_tone_hsi_2_center_width_0_reg.y_blending_2_h_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region2_Coef.H_CenterWid2;
    d_dcc_skin_tone_hsi_2_center_width_1_reg.y_blending_2_s_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region2_Coef.S_CenterWid2;
    d_dcc_skin_tone_hsi_2_center_width_1_reg.y_blending_2_i_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region2_Coef.I_CenterWid2;

	Segment2_0 = ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_0) << 24)+
				((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_1) << 16) +
				((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_2) << 8) +
				((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_3));
	Segment2_1 = ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_4) << 24)+
				((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_5) << 16) +
				((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_6) << 8) +
				((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_7));

	d_dcc_skin_tone_segment2_0_reg.regValue = Segment2_0;
	d_dcc_skin_tone_segment2_1_reg.regValue = Segment2_1;

	Segment2_0_c = ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_0_c) << 24)+	//20150708 chance_qian
				   ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_1_c) << 16) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_2_c) << 8) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_3_c));
	Segment2_1_c = ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_4_c) << 24)+
				   ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_5_c) << 16) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_6_c) << 8) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region2_Coef.Segment2_7_c));

	d_dcc_skin_tone_segment_c2_0_reg.regValue = Segment2_0_c; //20150708 chance_qian
	d_dcc_skin_tone_segment_c2_1_reg.regValue = Segment2_1_c; //20150708 chance_qian

	d_dcc_skin_tone_en_reg.y_blending_reg3_en = 							ptr->S_DCC_Skin_Tone_Y_Region3_Coef.region3_en;
	d_dcc_skin_tone_en_reg.y_blending_3_enhance_en = 						ptr->S_DCC_Skin_Tone_Y_Region3_Coef.region3_enhance_en; //20150708 chance_qian
	d_dcc_skin_tone_yuv_center_30_reg.y_blending_3_y_center =					ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Y_Center3;
	d_dcc_skin_tone_yuv_center_3_reg.y_blending_3_u_center = 				ptr->S_DCC_Skin_Tone_Y_Region3_Coef.U_Center3;
	d_dcc_skin_tone_yuv_center_3_reg.y_blending_3_v_center = 				ptr->S_DCC_Skin_Tone_Y_Region3_Coef.V_Center3;
	d_dcc_skin_tone_yuv_range_3_reg.y_blending_3_y_range = 					ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Y_Range3;
	d_dcc_skin_tone_yuv_range_3_reg.y_blending_3_u_range = 					ptr->S_DCC_Skin_Tone_Y_Region3_Coef.U_Range3;
	d_dcc_skin_tone_yuv_range_3_reg.y_blending_3_v_range = 					ptr->S_DCC_Skin_Tone_Y_Region3_Coef.V_Range3;
    d_dcc_skin_tone_hsi_3_center_width_0_reg.y_blending_3_h_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region3_Coef.H_CenterWid3;
    d_dcc_skin_tone_hsi_3_center_width_1_reg.y_blending_3_s_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region3_Coef.S_CenterWid3;
    d_dcc_skin_tone_hsi_3_center_width_1_reg.y_blending_3_i_center_width = 	ptr->S_DCC_Skin_Tone_Y_Region3_Coef.I_CenterWid3;

	Segment3_0 = ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_0) << 24)+
				((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_1) << 16) +
				((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_2) << 8) +
				((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_3));
	Segment3_1 = ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_4) << 24)+
				((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_5) << 16) +
				((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_6) << 8) +
				((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_7));

	d_dcc_skin_tone_segment3_0_reg.regValue = Segment3_0;
	d_dcc_skin_tone_segment3_1_reg.regValue = Segment3_1;

	Segment3_0_c = ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_0_c) << 24)+	//20150708 chance_qian
				   ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_1_c) << 16) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_2_c) << 8) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_3_c));
	Segment3_1_c = ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_4_c) << 24)+
				   ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_5_c) << 16) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_6_c) << 8) +
				   ((ptr->S_DCC_Skin_Tone_Y_Region3_Coef.Segment3_7_c));

	d_dcc_skin_tone_segment_c3_0_reg.regValue = Segment3_0_c; //20150708 chance_qian
	d_dcc_skin_tone_segment_c3_1_reg.regValue = Segment3_1_c; //20150708 chance_qian

	/*VIPprintf("2.en =%d, mode =%d\n", d_dcc_skin_tone_en_reg.y_blending_en, d_dcc_skin_tone_en_reg.y_blending_mode);*/
	/*VIPprintf("2.y_center =%d, u_center =%d, v_center =%d\n", d_dcc_skin_tone_yuv_center_reg.y_blending_y_center , d_dcc_skin_tone_yuv_center_reg.y_blending_u_center, d_dcc_skin_tone_yuv_center_reg.y_blending_v_center);*/
	/*VIPprintf("2.y_range =%d, u_range =%d, v_range =%d\n", d_dcc_skin_tone_yuv_range_reg.y_blending_y_range, d_dcc_skin_tone_yuv_range_reg.y_blending_u_range, d_dcc_skin_tone_yuv_range_reg.y_blending_v_range);*/


	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_EN_reg, d_dcc_skin_tone_en_reg.regValue);

	rtd_outl(COLOR_DCC_D_DCC_MAIN_OFFSET_reg, color_dcc_d_dcc_main_offset_reg.regValue);    //20170524 robin_zhang
	rtd_outl(COLOR_DCC_D_DCC_SUB_OFFSET_reg, color_dcc_d_dcc_sub_offset_reg.regValue);       //20170524 robin_zhang

	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_0_reg, d_dcc_skin_tone_yuv_center_0_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_00_reg, d_dcc_skin_tone_yuv_center_00_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_RANGE_0_reg, d_dcc_skin_tone_yuv_range_0_reg.regValue);
    rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_0_CENTER_WIDTH_0_reg, d_dcc_skin_tone_hsi_0_center_width_0_reg.regValue);
    rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_0_CENTER_WIDTH_1_reg, d_dcc_skin_tone_hsi_0_center_width_1_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT0_0_reg, d_dcc_skin_tone_segment0_0_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT0_1_reg, d_dcc_skin_tone_segment0_1_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C0_0_reg, d_dcc_skin_tone_segment_c0_0_reg.regValue); //20150708 chance_qian
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C0_1_reg, d_dcc_skin_tone_segment_c0_1_reg.regValue); //20150708 chance_qian

	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_1_reg, d_dcc_skin_tone_yuv_center_1_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_10_reg, d_dcc_skin_tone_yuv_center_10_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_RANGE_1_reg, d_dcc_skin_tone_yuv_range_1_reg.regValue);
    rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_1_CENTER_WIDTH_0_reg, d_dcc_skin_tone_hsi_1_center_width_0_reg.regValue);
    rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_1_CENTER_WIDTH_1_reg, d_dcc_skin_tone_hsi_1_center_width_1_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT1_0_reg, d_dcc_skin_tone_segment1_0_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT1_1_reg, d_dcc_skin_tone_segment1_1_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C1_0_reg, d_dcc_skin_tone_segment_c1_0_reg.regValue); //20150708 chance_qian
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C1_1_reg, d_dcc_skin_tone_segment_c1_1_reg.regValue); //20150708 chance_qian

	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_2_reg, d_dcc_skin_tone_yuv_center_2_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_20_reg, d_dcc_skin_tone_yuv_center_20_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_RANGE_2_reg, d_dcc_skin_tone_yuv_range_2_reg.regValue);
    rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_2_CENTER_WIDTH_0_reg, d_dcc_skin_tone_hsi_2_center_width_0_reg.regValue);
    rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_2_CENTER_WIDTH_1_reg, d_dcc_skin_tone_hsi_2_center_width_1_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT2_0_reg, d_dcc_skin_tone_segment2_0_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT2_1_reg, d_dcc_skin_tone_segment2_1_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C2_0_reg, d_dcc_skin_tone_segment_c2_0_reg.regValue); //20150708 chance_qian
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C2_1_reg, d_dcc_skin_tone_segment_c2_1_reg.regValue); //20150708 chance_qian

	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_3_reg, d_dcc_skin_tone_yuv_center_3_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_CENTER_30_reg, d_dcc_skin_tone_yuv_center_30_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_YUV_RANGE_3_reg, d_dcc_skin_tone_yuv_range_3_reg.regValue);
    rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_3_CENTER_WIDTH_0_reg, d_dcc_skin_tone_hsi_3_center_width_0_reg.regValue);
    rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_HSI_3_CENTER_WIDTH_1_reg, d_dcc_skin_tone_hsi_3_center_width_1_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT3_0_reg, d_dcc_skin_tone_segment3_0_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT3_1_reg, d_dcc_skin_tone_segment3_1_reg.regValue);
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C3_0_reg, d_dcc_skin_tone_segment_c3_0_reg.regValue); //20150708 chance_qian
	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_SEGMENT_C3_1_reg, d_dcc_skin_tone_segment_c3_1_reg.regValue); //20150708 chance_qian

	rtd_outl(COLOR_DCC_D_DCC_CDS_SKIN_0_reg, color_dcc_d_dcc_cds_skin_0_reg.regValue); //tulia_wang
//#endif
}

void drvif_color_set_dcc_Chroma_enhance(void)
{
	int index, timeout = 10000;
	int chromaGainParaNum = Chroma_Gain_Seg_MAX / 2;
	int base, g0, g1;
	int i, j;

	color_dcc_dcc_c_sram_tab_ctl_RBUS     dcc_c_sram_tab_ctl_reg;
	color_dcc_dcc_c_sram_tab_port_RBUS    dcc_c_sram_tab_port_reg;

	base = g0 = g1 = 0;

	for (j = 0; j < 4; j++) {
		dcc_c_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_C_SRAM_TAB_CTL_reg);
		dcc_c_sram_tab_ctl_reg.dcc_c_sram_tab_acc_index = 0;
		dcc_c_sram_tab_ctl_reg.dcc_c_sram_tab_rw_sel = 1;
		dcc_c_sram_tab_ctl_reg.dcc_c_sram_tab_acc_sel = j;
		rtd_outl(COLOR_DCC_DCC_C_SRAM_TAB_CTL_reg, dcc_c_sram_tab_ctl_reg.regValue);
		index = -1;
		for (i = 0; i < chromaGainParaNum; i++) {
			do {
				dcc_c_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_C_SRAM_TAB_CTL_reg);
				} while ((dcc_c_sram_tab_ctl_reg.dcc_c_sram_tab_acc_ready != 1) && ((timeout--) > 0));

			if (timeout < 0) {
				dcc_c_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_C_SRAM_TAB_CTL_reg);
				dcc_c_sram_tab_ctl_reg.dcc_c_sram_tab_rw_sel = 0;
				rtd_outl(COLOR_DCC_DCC_C_SRAM_TAB_CTL_reg, dcc_c_sram_tab_ctl_reg.regValue);
				VIPprintf("DCC Chroma Table Init. Write DCC Chroma Init Table Fault!!");
				return;
			}
			index++;
			if (index != dcc_c_sram_tab_ctl_reg.dcc_c_sram_tab_acc_index_cur) {     /*cur = index*/
				dcc_c_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_C_SRAM_TAB_CTL_reg);
				dcc_c_sram_tab_ctl_reg.dcc_c_sram_tab_rw_sel = 0;
				rtd_outl(COLOR_DCC_DCC_C_SRAM_TAB_CTL_reg, dcc_c_sram_tab_ctl_reg.regValue);
				VIPprintf("DCC Chroma Table Init. Write DCC Chroma Init Table Fault!!");
				return;
			}
			dcc_c_sram_tab_port_reg.regValue = rtd_inl(COLOR_DCC_DCC_C_SRAM_TAB_PORT_reg);
			dcc_c_sram_tab_port_reg.dcc_c_sram_tab_base = base;
			dcc_c_sram_tab_port_reg.dcc_c_sram_tab_g0 = g0;
			dcc_c_sram_tab_port_reg.dcc_c_sram_tab_g1 = g1;
			rtd_outl(COLOR_DCC_DCC_C_SRAM_TAB_PORT_reg, dcc_c_sram_tab_port_reg.regValue);
		}
		dcc_c_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_C_SRAM_TAB_CTL_reg);
		dcc_c_sram_tab_ctl_reg.dcc_c_sram_tab_rw_sel = 0;
		rtd_outl(COLOR_DCC_DCC_C_SRAM_TAB_CTL_reg, dcc_c_sram_tab_ctl_reg.regValue);
	}
}


void drvif_color_set_dcc_chroma_compensation_Init(unsigned char display)
{
	color_dcc_dcc_ctrl_chroma_gain_RBUS color_dcc_dcc_ctrl_chroma_gain_reg;
	color_dcc_dcc_ctrl_chroma_gain_reg.regValue = rtd_inl(COLOR_DCC_DCC_CTRL_CHROMA_GAIN_reg);

	if (display == SLR_MAIN_DISPLAY) {
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_gain_en_main = 1;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_enhance_above_tab_select_main = 0;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_enhance_below_tab_select_main = 1;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_gain_base_main = 3;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_gain_lookup_mode_main = 1;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_gain_mode_main = 0;
	}
#ifdef VIP_CONFIG_DUAL_CHANNEL
	else {
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_gain_en_sub = 1;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_enhance_above_tab_select_sub = 0;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_enhance_below_tab_select_sub = 1;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_gain_base_sub = 3;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_gain_lookup_mode_sub = 1;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_gain_mode_sub = 0;
	}
#endif
	rtd_outl(COLOR_DCC_DCC_CTRL_CHROMA_GAIN_reg, color_dcc_dcc_ctrl_chroma_gain_reg.regValue);
}


void drvif_color_set_dcc_chroma_compensation_Curve_Mode(unsigned char display, VIP_DCC_Chroma_Compensation_TABLE *table)
{
	if (!table) {
		VIPprintf(":::ERROR: DCC table is NULL!:::\n");
		return;
	}

	if (display == SLR_MAIN_DISPLAY) {
		/*chroma limit*/

		//8 seg -> 21 seg
		//20170514 robin_zhang
		color_dcc_dcc_ctrl_luma_RBUS                                color_dcc_dcc_ctrl_luma_reg;
		color_dcc_dcc_chroma_gain_limit_main_0_RBUS	color_dcc_dcc_chroma_gain_limit_main_0_reg;
		color_dcc_dcc_chroma_gain_limit_main_1_RBUS	color_dcc_dcc_chroma_gain_limit_main_1_reg;
		color_dcc_dcc_chroma_gain_limit_main_2_RBUS	color_dcc_dcc_chroma_gain_limit_main_2_reg;
		color_dcc_dcc_chroma_gain_limit_main_3_RBUS	color_dcc_dcc_chroma_gain_limit_main_3_reg;
		color_dcc_dcc_chroma_gain_limit_main_4_RBUS	color_dcc_dcc_chroma_gain_limit_main_4_reg;


		color_dcc_dcc_ctrl_luma_reg.regValue                          = rtd_inl(COLOR_DCC_DCC_CTRL_LUMA_reg);
		color_dcc_dcc_chroma_gain_limit_main_0_reg.regValue = rtd_inl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_MAIN_0_reg);
		color_dcc_dcc_chroma_gain_limit_main_1_reg.regValue = rtd_inl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_MAIN_1_reg);
		color_dcc_dcc_chroma_gain_limit_main_2_reg.regValue = rtd_inl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_MAIN_2_reg);
		color_dcc_dcc_chroma_gain_limit_main_3_reg.regValue = rtd_inl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_MAIN_3_reg);
		color_dcc_dcc_chroma_gain_limit_main_4_reg.regValue = rtd_inl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_MAIN_4_reg);


		color_dcc_dcc_ctrl_luma_reg.chroma_gain_limit_en_main                               = table->Chroma_Compensation_Ctrl.gain_limit_en;
		color_dcc_dcc_chroma_gain_limit_main_0_reg.chroma_gain_limit_seg0_main  = table->chroma_gain_limit[0] - 1;
		color_dcc_dcc_chroma_gain_limit_main_0_reg.chroma_gain_limit_seg1_main  = table->chroma_gain_limit[1] - 1;
		color_dcc_dcc_chroma_gain_limit_main_0_reg.chroma_gain_limit_seg2_main  = table->chroma_gain_limit[2] - 1;
		color_dcc_dcc_chroma_gain_limit_main_0_reg.chroma_gain_limit_seg3_main  = table->chroma_gain_limit[3] - 1;
		color_dcc_dcc_chroma_gain_limit_main_0_reg.chroma_gain_limit_seg4_main  = table->chroma_gain_limit[4] - 1;
		color_dcc_dcc_chroma_gain_limit_main_1_reg.chroma_gain_limit_seg5_main  = table->chroma_gain_limit[5] - 1;
		color_dcc_dcc_chroma_gain_limit_main_1_reg.chroma_gain_limit_seg6_main  = table->chroma_gain_limit[6] - 1;
		color_dcc_dcc_chroma_gain_limit_main_1_reg.chroma_gain_limit_seg7_main  = table->chroma_gain_limit[7] - 1;
		color_dcc_dcc_chroma_gain_limit_main_1_reg.chroma_gain_limit_seg8_main  = table->chroma_gain_limit[8] - 1;
		color_dcc_dcc_chroma_gain_limit_main_1_reg.chroma_gain_limit_seg9_main  = table->chroma_gain_limit[9] - 1;
		color_dcc_dcc_chroma_gain_limit_main_2_reg.chroma_gain_limit_seg10_main = table->chroma_gain_limit[10] - 1;
		color_dcc_dcc_chroma_gain_limit_main_2_reg.chroma_gain_limit_seg11_main = table->chroma_gain_limit[11] - 1;
		color_dcc_dcc_chroma_gain_limit_main_2_reg.chroma_gain_limit_seg12_main = table->chroma_gain_limit[12] - 1;
		color_dcc_dcc_chroma_gain_limit_main_2_reg.chroma_gain_limit_seg13_main = table->chroma_gain_limit[13] - 1;
		color_dcc_dcc_chroma_gain_limit_main_2_reg.chroma_gain_limit_seg14_main = table->chroma_gain_limit[14] - 1;
		color_dcc_dcc_chroma_gain_limit_main_3_reg.chroma_gain_limit_seg15_main = table->chroma_gain_limit[15] - 1;
		color_dcc_dcc_chroma_gain_limit_main_3_reg.chroma_gain_limit_seg16_main = table->chroma_gain_limit[16] - 1;
		color_dcc_dcc_chroma_gain_limit_main_3_reg.chroma_gain_limit_seg17_main = table->chroma_gain_limit[17] - 1;
		color_dcc_dcc_chroma_gain_limit_main_3_reg.chroma_gain_limit_seg18_main = table->chroma_gain_limit[18] - 1;
		color_dcc_dcc_chroma_gain_limit_main_3_reg.chroma_gain_limit_seg19_main = table->chroma_gain_limit[19] - 1;
		color_dcc_dcc_chroma_gain_limit_main_4_reg.chroma_gain_limit_seg20_main = table->chroma_gain_limit[20] - 1;


		scaler_rtd_outl(COLOR_DCC_DCC_CTRL_LUMA_reg, color_dcc_dcc_ctrl_luma_reg.regValue);
		scaler_rtd_outl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_MAIN_0_reg, color_dcc_dcc_chroma_gain_limit_main_0_reg.regValue);
		scaler_rtd_outl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_MAIN_1_reg, color_dcc_dcc_chroma_gain_limit_main_1_reg.regValue);
		scaler_rtd_outl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_MAIN_2_reg, color_dcc_dcc_chroma_gain_limit_main_2_reg.regValue);
		scaler_rtd_outl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_MAIN_3_reg, color_dcc_dcc_chroma_gain_limit_main_3_reg.regValue);
		scaler_rtd_outl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_MAIN_4_reg, color_dcc_dcc_chroma_gain_limit_main_4_reg.regValue);

		/*chroma table*/
		{
		color_dcc_dcc_ctrl_chroma_gain_RBUS 		color_dcc_dcc_ctrl_chroma_gain_reg;
		color_dcc_dcc_ctrl_chroma_gain_reg.regValue = rtd_inl(COLOR_DCC_DCC_CTRL_CHROMA_GAIN_reg);
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_gain_en_main = table->Chroma_Compensation_Ctrl.gain_en;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_gain_mode_main = table->Chroma_Compensation_Ctrl.gain_mode;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_gain_lookup_mode_main = table->Chroma_Compensation_Ctrl.gain_lookup_mode;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_gain_base_main = table->Chroma_Compensation_Ctrl.gain_base;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_enhance_above_tab_select_main = table->Chroma_Compensation_Ctrl.enh_mode_above_tab_sel;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_enhance_below_tab_select_main = table->Chroma_Compensation_Ctrl.enh_mode_below_tab_sel;
		color_dcc_dcc_ctrl_chroma_gain_reg.saturation_byy_u_tab_select_main = table->Chroma_Compensation_Ctrl.byY_mode_u_tab_sel;
		color_dcc_dcc_ctrl_chroma_gain_reg.saturation_byy_v_tab_select_main = table->Chroma_Compensation_Ctrl.byY_mode_v_tab_sel;
		scaler_rtd_outl(COLOR_DCC_DCC_CTRL_CHROMA_GAIN_reg, color_dcc_dcc_ctrl_chroma_gain_reg.regValue);
		}
	}
#ifdef VIP_CONFIG_DUAL_CHANNEL
	else {
		/*chroma limit*/
		color_dcc_dcc_ctrl_luma_RBUS                         color_dcc_dcc_ctrl_luma_reg;
		color_dcc_dcc_chroma_gain_limit_sub_0_RBUS	color_dcc_dcc_chroma_gain_limit_sub_0_reg;
		color_dcc_dcc_chroma_gain_limit_sub_1_RBUS	color_dcc_dcc_chroma_gain_limit_sub_1_reg;
		color_dcc_dcc_chroma_gain_limit_sub_2_RBUS	color_dcc_dcc_chroma_gain_limit_sub_2_reg;
		color_dcc_dcc_chroma_gain_limit_sub_3_RBUS	color_dcc_dcc_chroma_gain_limit_sub_3_reg;
		color_dcc_dcc_chroma_gain_limit_sub_4_RBUS	color_dcc_dcc_chroma_gain_limit_sub_4_reg;

		color_dcc_dcc_ctrl_luma_reg.regValue                        = rtd_inl(COLOR_DCC_DCC_CTRL_LUMA_reg);
		color_dcc_dcc_chroma_gain_limit_sub_0_reg.regValue = rtd_inl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_SUB_0_reg);
		color_dcc_dcc_chroma_gain_limit_sub_1_reg.regValue = rtd_inl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_SUB_1_reg);
		color_dcc_dcc_chroma_gain_limit_sub_2_reg.regValue = rtd_inl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_SUB_2_reg);
		color_dcc_dcc_chroma_gain_limit_sub_3_reg.regValue = rtd_inl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_SUB_3_reg);
		color_dcc_dcc_chroma_gain_limit_sub_4_reg.regValue = rtd_inl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_SUB_4_reg);

		color_dcc_dcc_ctrl_luma_reg.chroma_gain_limit_en_sub                            = table->Chroma_Compensation_Ctrl.gain_limit_en;
		color_dcc_dcc_chroma_gain_limit_sub_0_reg.chroma_gain_limit_seg0_sub  = table->chroma_gain_limit[0] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_0_reg.chroma_gain_limit_seg1_sub  = table->chroma_gain_limit[1] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_0_reg.chroma_gain_limit_seg2_sub  = table->chroma_gain_limit[2] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_0_reg.chroma_gain_limit_seg3_sub  = table->chroma_gain_limit[3] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_0_reg.chroma_gain_limit_seg4_sub  = table->chroma_gain_limit[4] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_1_reg.chroma_gain_limit_seg5_sub  = table->chroma_gain_limit[5] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_1_reg.chroma_gain_limit_seg6_sub  = table->chroma_gain_limit[6] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_1_reg.chroma_gain_limit_seg7_sub  = table->chroma_gain_limit[7] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_1_reg.chroma_gain_limit_seg8_sub  = table->chroma_gain_limit[8] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_1_reg.chroma_gain_limit_seg9_sub  = table->chroma_gain_limit[9] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_2_reg.chroma_gain_limit_seg10_sub = table->chroma_gain_limit[10] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_2_reg.chroma_gain_limit_seg11_sub = table->chroma_gain_limit[11] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_2_reg.chroma_gain_limit_seg12_sub = table->chroma_gain_limit[12] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_2_reg.chroma_gain_limit_seg13_sub = table->chroma_gain_limit[13] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_2_reg.chroma_gain_limit_seg14_sub = table->chroma_gain_limit[14] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_3_reg.chroma_gain_limit_seg15_sub = table->chroma_gain_limit[15] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_3_reg.chroma_gain_limit_seg16_sub = table->chroma_gain_limit[16] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_3_reg.chroma_gain_limit_seg17_sub = table->chroma_gain_limit[17] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_3_reg.chroma_gain_limit_seg18_sub = table->chroma_gain_limit[18] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_3_reg.chroma_gain_limit_seg19_sub = table->chroma_gain_limit[19] - 1;
		color_dcc_dcc_chroma_gain_limit_sub_4_reg.chroma_gain_limit_seg20_sub = table->chroma_gain_limit[20] - 1;


		scaler_rtd_outl(COLOR_DCC_DCC_CTRL_LUMA_reg, color_dcc_dcc_ctrl_luma_reg.regValue);
		scaler_rtd_outl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_SUB_0_reg, color_dcc_dcc_chroma_gain_limit_sub_0_reg.regValue);
		scaler_rtd_outl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_SUB_1_reg, color_dcc_dcc_chroma_gain_limit_sub_1_reg.regValue);
		scaler_rtd_outl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_SUB_2_reg, color_dcc_dcc_chroma_gain_limit_sub_2_reg.regValue);
		scaler_rtd_outl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_SUB_3_reg, color_dcc_dcc_chroma_gain_limit_sub_3_reg.regValue);
		scaler_rtd_outl(COLOR_DCC_DCC_CHROMA_GAIN_LIMIT_SUB_4_reg, color_dcc_dcc_chroma_gain_limit_sub_4_reg.regValue);


		/*chroma table*/
		{
		color_dcc_dcc_ctrl_chroma_gain_RBUS 		color_dcc_dcc_ctrl_chroma_gain_reg;
		color_dcc_dcc_ctrl_chroma_gain_reg.regValue = rtd_inl(COLOR_DCC_DCC_CTRL_CHROMA_GAIN_reg);
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_gain_en_sub = table->Chroma_Compensation_Ctrl.gain_en;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_gain_mode_sub = table->Chroma_Compensation_Ctrl.gain_mode;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_gain_lookup_mode_sub = table->Chroma_Compensation_Ctrl.gain_lookup_mode;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_gain_base_sub = table->Chroma_Compensation_Ctrl.gain_base;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_enhance_above_tab_select_sub = table->Chroma_Compensation_Ctrl.enh_mode_above_tab_sel;
		color_dcc_dcc_ctrl_chroma_gain_reg.chroma_enhance_below_tab_select_sub = table->Chroma_Compensation_Ctrl.enh_mode_below_tab_sel;
		color_dcc_dcc_ctrl_chroma_gain_reg.saturation_byy_u_tab_select_sub = table->Chroma_Compensation_Ctrl.byY_mode_u_tab_sel;
		color_dcc_dcc_ctrl_chroma_gain_reg.saturation_byy_v_tab_select_sub = table->Chroma_Compensation_Ctrl.byY_mode_v_tab_sel;
		scaler_rtd_outl(COLOR_DCC_DCC_CTRL_CHROMA_GAIN_reg, color_dcc_dcc_ctrl_chroma_gain_reg.regValue);
		}
	}
#endif

	/*set chroma gain table value*/
	{
	int index, timeout = 10000;
	int chromaGainParaNum = 32;
	int base[32], g0[32], g1[32];
	int i, j;

	color_dcc_dcc_c_sram_tab_ctl_RBUS     dcc_c_sram_tab_ctl_reg;
	color_dcc_dcc_c_sram_tab_port_RBUS    dcc_c_sram_tab_port_reg;

	for (j = 0; j < 4; j++) {
		/*calc base, g0, g1*/
		for (i = 0; i < chromaGainParaNum; i++) {
			if (j == 3) {
				base[i] = table->chroma_gain_t3[2 * i];
				g0[i] = table->chroma_gain_t3[2 * i + 1] - table->chroma_gain_t3[2 * i];
				g1[i] = table->chroma_gain_t3[2 * i + 2] - table->chroma_gain_t3[2 * i + 1];
			}else if(j == 2) {
				base[i] = table->chroma_gain_t2[2 * i];
				g0[i] = table->chroma_gain_t2[2 * i + 1] - table->chroma_gain_t2[2 * i];
				g1[i] = table->chroma_gain_t2[2 * i + 2] - table->chroma_gain_t2[2 * i + 1];
			}else if(j == 1) {
				base[i] = table->chroma_gain_t1[2 * i];
				g0[i] = table->chroma_gain_t1[2 * i + 1] - table->chroma_gain_t1[2 * i];
				g1[i] = table->chroma_gain_t1[2 * i + 2] - table->chroma_gain_t1[2 * i + 1];
			} else {
				base[i] = table->chroma_gain_t0[2 * i];
				g0[i] = table->chroma_gain_t0[2 * i + 1] - table->chroma_gain_t0[2 * i];
				g1[i] = table->chroma_gain_t0[2 * i + 2] - table->chroma_gain_t0[2 * i + 1];
			}
			/*VIPprintf("[DCC] chroma[%d], base =%d, g0 =%d, g1 =%d\n", i, base[i], g0[i], g1[i] );*/
		}

		dcc_c_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_C_SRAM_TAB_CTL_reg);
		dcc_c_sram_tab_ctl_reg.dcc_c_sram_tab_acc_index = 0;
		dcc_c_sram_tab_ctl_reg.dcc_c_sram_tab_rw_sel = 1;
		dcc_c_sram_tab_ctl_reg.dcc_c_sram_tab_acc_sel = j;
		rtd_outl(COLOR_DCC_DCC_C_SRAM_TAB_CTL_reg, dcc_c_sram_tab_ctl_reg.regValue);
		index = -1;
		for (i = 0; i < chromaGainParaNum; i++) {
			do {
				dcc_c_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_C_SRAM_TAB_CTL_reg);
				} while ((dcc_c_sram_tab_ctl_reg.dcc_c_sram_tab_acc_ready != 1) && ((timeout--) > 0));
			if (timeout < 0) {
				dcc_c_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_C_SRAM_TAB_CTL_reg);
				dcc_c_sram_tab_ctl_reg.dcc_c_sram_tab_rw_sel = 0;
				rtd_outl(COLOR_DCC_DCC_C_SRAM_TAB_CTL_reg, dcc_c_sram_tab_ctl_reg.regValue);
				VIPprintf("DCC Chroma Table Init. Write DCC Chroma Init Table Fault at %d: Time Out!!\n", i);
				return;
			}
			index++;
			if (index != dcc_c_sram_tab_ctl_reg.dcc_c_sram_tab_acc_index_cur) {     /*cur = index*/
				dcc_c_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_C_SRAM_TAB_CTL_reg);
				dcc_c_sram_tab_ctl_reg.dcc_c_sram_tab_rw_sel = 0;
				rtd_outl(COLOR_DCC_DCC_C_SRAM_TAB_CTL_reg, dcc_c_sram_tab_ctl_reg.regValue);
				VIPprintf("DCC Chroma Table Init. Write DCC Chroma Init Table Fault at %d: Index Error!!\n", i);
				return;
			}

			dcc_c_sram_tab_port_reg.regValue = rtd_inl(COLOR_DCC_DCC_C_SRAM_TAB_PORT_reg);
			dcc_c_sram_tab_port_reg.dcc_c_sram_tab_base = base[i];
			dcc_c_sram_tab_port_reg.dcc_c_sram_tab_g0 = g0[i];
			dcc_c_sram_tab_port_reg.dcc_c_sram_tab_g1 = g1[i];
			rtd_outl(COLOR_DCC_DCC_C_SRAM_TAB_PORT_reg, dcc_c_sram_tab_port_reg.regValue);
		}

		dcc_c_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_C_SRAM_TAB_CTL_reg);
		dcc_c_sram_tab_ctl_reg.dcc_c_sram_tab_rw_sel = 0;
		rtd_outl(COLOR_DCC_DCC_C_SRAM_TAB_CTL_reg, dcc_c_sram_tab_ctl_reg.regValue);
	}
	}

}


void drvif_color_set_dcc_Luma_offset(unsigned char display, UINT8 offset_level)
{
	color_dcc_dcc_ctrl_luma_RBUS color_dcc_dcc_ctrl_luma_Reg;
	color_dcc_dcc_ctrl_luma_Reg.regValue = rtd_inl(COLOR_DCC_DCC_CTRL_LUMA_reg);

	if (display == SLR_MAIN_DISPLAY) {
		color_dcc_dcc_ctrl_luma_Reg.luma_offset_main = offset_level;
	}
#ifdef VIP_CONFIG_DUAL_CHANNEL
	else {
		color_dcc_dcc_ctrl_luma_Reg.luma_offset_sub = offset_level;
	}
#endif

	scaler_rtd_outl(COLOR_DCC_DCC_CTRL_LUMA_reg, color_dcc_dcc_ctrl_luma_Reg.regValue);

}

void drvif_color_get_dcc_current_curve(UINT32 *pCurveNode, UINT32 nodenum, unsigned char display)
{
	UINT32 dcc_decode[129] = {0};

	int i, index;
	int timeout = 0x625000;
	/*int DCCNum = 64;*/ /*this is controled by DCC segment*/
	int DCCNum = nodenum >> 1;

	/*color_dcc_dcc_y_sram_tab_ctl_RBUS	color_dcc_sram_tab_ctl_0_reg;*/
	color_dcc_dcc_y_sram_tab_ctl_RBUS 		color_dcc_dcc_y_sram_tab_ctl_reg;
	/*color_dcc_sram_tab_port_RBUS 	color_dcc_sram_tab_port_reg;*/
	color_dcc_dcc_y_sram_tab_port_RBUS 	color_dcc_dcc_y_sram_tab_port_reg;

	memset(pCurveNode, 0, sizeof(UINT32)*nodenum);

	if (display == SLR_MAIN_DISPLAY) {

		color_dcc_dcc_y_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
		color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_acc_index = 0;
		color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_rw_sel = 2;
		color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_acc_sel = 0; /* main table 1*/
		rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg, color_dcc_dcc_y_sram_tab_ctl_reg.regValue);

		index = 0;
		for (i = 0; i < DCCNum; i++) {
			do {
				color_dcc_dcc_y_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
				} while ((color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_acc_ready != 1) && ((timeout--) > 0));

			if (timeout < 0) {
				color_dcc_dcc_y_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
				color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_rw_sel = 0;
				rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg, color_dcc_dcc_y_sram_tab_ctl_reg.regValue);
				VIPprintf("[DCC]drvif_color_get_dcc_current_curve Failed!!");
				return;
			}
			index++;
			if ((index != color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_acc_index_cur) &&
				(index < DCCNum))
			{
				color_dcc_dcc_y_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
				color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_rw_sel = 0;
				rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg, color_dcc_dcc_y_sram_tab_ctl_reg.regValue);
				printk("[DCC], [%d][%d]\n", index, color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_acc_index_cur);
				return;
			}
			if (i == 0) {
				color_dcc_dcc_y_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
				dcc_decode[(i << 1)] = color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_base;
			}
			color_dcc_dcc_y_sram_tab_port_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_PORT_reg);
			dcc_decode[(i << 1) + 1] = color_dcc_dcc_y_sram_tab_port_reg.dcc_y_sram_tab_g0 + dcc_decode[(i << 1)];
			dcc_decode[(i << 1) + 2] = color_dcc_dcc_y_sram_tab_port_reg.dcc_y_sram_tab_g1 + dcc_decode[(i << 1) + 1];

		}

		color_dcc_dcc_y_sram_tab_ctl_reg.regValue = rtd_inl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg);
		color_dcc_dcc_y_sram_tab_ctl_reg.dcc_y_sram_tab_rw_sel = 0;
		rtd_outl(COLOR_DCC_DCC_Y_SRAM_TAB_CTL_reg, color_dcc_dcc_y_sram_tab_ctl_reg.regValue);

	}

	memcpy(pCurveNode, dcc_decode, sizeof(UINT32)*(nodenum > 129 ? 129 : nodenum));
//#endif
}

void drvif_color_DCC_ctrl_YUV2RGB(unsigned char data_format_sel, unsigned char data_range_sel)
{
	color_dcc_d_dcc_skin_tone_en_RBUS d_dcc_skin_tone_en_reg;

	d_dcc_skin_tone_en_reg.regValue = rtd_inl(COLOR_DCC_D_DCC_SKIN_TONE_EN_reg);

	d_dcc_skin_tone_en_reg.data_format_sel = data_format_sel;
	d_dcc_skin_tone_en_reg.data_range_sel = data_range_sel;

	rtd_outl(COLOR_DCC_D_DCC_SKIN_TONE_EN_reg, d_dcc_skin_tone_en_reg.regValue);
}
void  drvif_color_get_dcc_adjust_value(int val_info , int *value_delta, unsigned int *sAccLUT) //get from   drvif_color_get_dcc_current_curve
{
    int Curve_data_range = 1 << DCC_Curve_DataRang_bit;
	int segdist = Curve_data_range/(Curve_num_Max-1) ;  //>>7
	int segmentIndex = val_info/segdist ;    //>>5
	int distFact = val_info%segdist ;
	int g0 =  sAccLUT[segmentIndex+1]-sAccLUT[segmentIndex];
	int mapout = sAccLUT[segmentIndex]+distFact*g0/segdist;
    *value_delta = mapout - val_info ;

}


