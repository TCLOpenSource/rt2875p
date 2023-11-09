/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2009
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


#include "rtk_vip_logger.h"
#include <rtd_log/rtd_module_log.h>
#include <linux/semaphore.h>

#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/vip/pcid.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/vip/vip_reg_def.h>

#include <rbus/pcid_reg.h>
#include <rbus/lg_pod_reg.h>
#include <rbus/sys_reg_reg.h>
#include <tvscalercontrol/vip/color.h>
#include <rbus/siw_pclrc_reg.h>


/*================================ Global Variables ==============================*/
unsigned char Rgn4x4MappingTbl[16][4] =
{
	/* 0*/ { RGN_NA, RGN_NA, RGN_NA,      0 },
	/* 1*/ { RGN_NA, RGN_NA,      0,      1 },
	/* 2*/ { RGN_NA, RGN_NA,      1,      2 },
	/* 3*/ { RGN_NA, RGN_NA,      2, RGN_NA },
	/* 4*/ { RGN_NA,      0, RGN_NA,      3 },
	/* 5*/ {      0,      1,      3,      4 },
	/* 6*/ {      1,      2,      4,      5 },
	/* 7*/ {      2, RGN_NA,      5, RGN_NA },
	/* 8*/ { RGN_NA,      3, RGN_NA,      6 },
	/* 9*/ {      3,      4,      6,      7 },
	/*10*/ {      4,      5,      7,      8 },
	/*11*/ {      5, RGN_NA,      8, RGN_NA },
	/*12*/ { RGN_NA,      6, RGN_NA, RGN_NA },
	/*13*/ {      6,      7, RGN_NA, RGN_NA },
	/*14*/ {      7,      8, RGN_NA, RGN_NA },
	/*15*/ {      8, RGN_NA, RGN_NA, RGN_NA }
};
unsigned char Rgn1x10MappingTbl[10][2] =
{
	/* 0*/ { RGN_NA,      0 },
	/* 1*/ {      0,      1 },
	/* 2*/ {      1,      2 },
	/* 3*/ {      2,      3 },
	/* 4*/ {      3,      4 },
	/* 5*/ {      4,      5 },
	/* 6*/ {      5,      6 },
	/* 7*/ {      6,      7 },
	/* 8*/ {      7,      8 },
	/* 9*/ {      8, RGN_NA },
};
unsigned int pcidRgnTbl[16][3][PCID_TBL_LEN] = {{{0}}};
unsigned int pcidRgnWtTbl[PCID_WTTBL_LEN]={0};

unsigned int pcidRowBnd[17] = {128, 256, 384, 512, 768, 1024, 1280, 1536, 1792, 2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840};
unsigned int pcidColBnd[17] = {128, 256, 384, 512, 768, 1024, 1280, 1536, 1792, 2048, 2304, 2560, 2816, 3072, 3328, 3584, 3840};
unsigned int TransTbl[361] = {0};
#define PANEL_PCLRC_DATA_SIZE 12544

/*================================ Definitions ==============================*/
#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_PCID_DEBUG,fmt,##args)

/*================================== Function ===============================*/
void drvif_color_set_pcid_regional_table_setting(DRV_pcid_RgnTbl_t* pRgnTbl)
{
	pcid_pcid_regionaltable_ctrl_RBUS pcid_pcid_regionaltable_ctrl_reg;
	
	pcid_pcid_regional_19x19_table_index_bound_0_RBUS pcid_pcid_regional_19x19_table_index_bound_0_reg;
	pcid_pcid_regional_19x19_table_index_bound_1_RBUS pcid_pcid_regional_19x19_table_index_bound_1_reg;
	pcid_pcid_regional_19x19_table_index_bound_2_RBUS pcid_pcid_regional_19x19_table_index_bound_2_reg;
	pcid_pcid_regional_19x19_table_index_bound_3_RBUS pcid_pcid_regional_19x19_table_index_bound_3_reg;
	pcid_pcid_regional_19x19_table_index_bound_4_RBUS pcid_pcid_regional_19x19_table_index_bound_4_reg;
	pcid_pcid_regional_19x19_table_index_bound_5_RBUS pcid_pcid_regional_19x19_table_index_bound_5_reg;
	pcid_pcid_regional_19x19_table_index_bound_6_RBUS pcid_pcid_regional_19x19_table_index_bound_6_reg;
	pcid_pcid_regional_19x19_table_index_bound_7_RBUS pcid_pcid_regional_19x19_table_index_bound_7_reg;
	pcid_pcid_regional_19x19_table_index_bound_8_RBUS pcid_pcid_regional_19x19_table_index_bound_8_reg;

	pcid_pcid_regional_19x19_table_index_bound_9_RBUS pcid_pcid_regional_19x19_table_index_bound_9_reg;
	pcid_pcid_regional_19x19_table_index_bound_10_RBUS pcid_pcid_regional_19x19_table_index_bound_10_reg;
	pcid_pcid_regional_19x19_table_index_bound_11_RBUS pcid_pcid_regional_19x19_table_index_bound_11_reg;
	pcid_pcid_regional_19x19_table_index_bound_12_RBUS pcid_pcid_regional_19x19_table_index_bound_12_reg;
	pcid_pcid_regional_19x19_table_index_bound_13_RBUS pcid_pcid_regional_19x19_table_index_bound_13_reg;
	pcid_pcid_regional_19x19_table_index_bound_14_RBUS pcid_pcid_regional_19x19_table_index_bound_14_reg;
	pcid_pcid_regional_19x19_table_index_bound_15_RBUS pcid_pcid_regional_19x19_table_index_bound_15_reg;
	pcid_pcid_regional_19x19_table_index_bound_16_RBUS pcid_pcid_regional_19x19_table_index_bound_16_reg;
	pcid_pcid_regional_19x19_table_index_bound_17_RBUS pcid_pcid_regional_19x19_table_index_bound_17_reg;

	pcid_pcid_regional_index_horizontal_0_RBUS pcid_pcid_regional_index_horizontal_0_reg;
	pcid_pcid_regional_index_horizontal_1_RBUS pcid_pcid_regional_index_horizontal_1_reg;
	pcid_pcid_regional_index_vertical_0_RBUS pcid_pcid_regional_index_vertical_0_reg;
	pcid_pcid_regional_index_vertical_1_RBUS pcid_pcid_regional_index_vertical_1_reg;
	pcid_pcid_regional_index_vertical_2_RBUS pcid_pcid_regional_index_vertical_2_reg;
	pcid_pcid_regional_index_vertical_3_RBUS pcid_pcid_regional_index_vertical_3_reg;
	pcid_pcid_regional_index_vertical_4_RBUS pcid_pcid_regional_index_vertical_4_reg;
	pcid_pcid_regional_blendingstep_horizontal_RBUS pcid_pcid_regional_blendingstep_horizontal_reg;
	pcid_pcid_regional_blendingstep_vertical_0_RBUS pcid_pcid_regional_blendingstep_vertical_0_reg;
	pcid_pcid_regional_blendingstep_vertical_1_RBUS pcid_pcid_regional_blendingstep_vertical_1_reg;

	pcid_pcid_regionaltable_index_h_RBUS pcid_pcid_regionaltable_index_h_reg;
	pcid_pcid_regionaltable_index_v_RBUS pcid_pcid_regionaltable_index_v_reg;
	
	pcid_pcid_regionaltable_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_RegionalTable_Ctrl_reg);
	
	pcid_pcid_regional_19x19_table_index_bound_0_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_0_reg);
	pcid_pcid_regional_19x19_table_index_bound_1_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_1_reg);
	pcid_pcid_regional_19x19_table_index_bound_2_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_2_reg);
	pcid_pcid_regional_19x19_table_index_bound_3_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_3_reg);
	pcid_pcid_regional_19x19_table_index_bound_4_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_4_reg);
	pcid_pcid_regional_19x19_table_index_bound_5_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_5_reg);
	pcid_pcid_regional_19x19_table_index_bound_6_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_6_reg);
	pcid_pcid_regional_19x19_table_index_bound_7_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_7_reg);
	pcid_pcid_regional_19x19_table_index_bound_8_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_8_reg);

	pcid_pcid_regional_19x19_table_index_bound_9_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_9_reg);
	pcid_pcid_regional_19x19_table_index_bound_10_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_10_reg);
	pcid_pcid_regional_19x19_table_index_bound_11_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_11_reg);
	pcid_pcid_regional_19x19_table_index_bound_12_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_12_reg);
	pcid_pcid_regional_19x19_table_index_bound_13_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_13_reg);
	pcid_pcid_regional_19x19_table_index_bound_14_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_14_reg);
	pcid_pcid_regional_19x19_table_index_bound_15_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_15_reg);
	pcid_pcid_regional_19x19_table_index_bound_16_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_16_reg);
	pcid_pcid_regional_19x19_table_index_bound_17_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_17_reg);

       pcid_pcid_regional_index_horizontal_0_reg.regValue = IoReg_Read32(PCID_PCID_Regional_Index_Horizontal_0_reg);
	pcid_pcid_regional_index_horizontal_1_reg.regValue = IoReg_Read32(PCID_PCID_Regional_Index_Horizontal_1_reg);
	pcid_pcid_regional_index_vertical_0_reg.regValue = IoReg_Read32(PCID_PCID_Regional_Index_Vertical_0_reg);
	pcid_pcid_regional_index_vertical_1_reg.regValue = IoReg_Read32(PCID_PCID_Regional_Index_Vertical_1_reg);
	pcid_pcid_regional_index_vertical_2_reg.regValue = IoReg_Read32(PCID_PCID_Regional_Index_Vertical_2_reg);
	pcid_pcid_regional_index_vertical_3_reg.regValue = IoReg_Read32(PCID_PCID_Regional_Index_Vertical_3_reg);
	pcid_pcid_regional_index_vertical_4_reg.regValue = IoReg_Read32(PCID_PCID_Regional_Index_Vertical_4_reg);
	pcid_pcid_regional_blendingstep_horizontal_reg.regValue = IoReg_Read32(PCID_PCID_Regional_BlendingStep_Horizontal_reg);
	pcid_pcid_regional_blendingstep_vertical_0_reg.regValue = IoReg_Read32(PCID_PCID_Regional_BlendingStep_Vertical_0_reg);
	pcid_pcid_regional_blendingstep_vertical_1_reg.regValue = IoReg_Read32(PCID_PCID_Regional_BlendingStep_Vertical_1_reg);

       pcid_pcid_regionaltable_index_h_reg.regValue = IoReg_Read32(PCID_PCID_RegionalTable_Index_H_reg);
       pcid_pcid_regionaltable_index_v_reg.regValue = IoReg_Read32(PCID_PCID_RegionalTable_Index_V_reg);


	pcid_pcid_regionaltable_ctrl_reg.regional_table_en = 1;
	pcid_pcid_regionaltable_ctrl_reg.pcid_regional_10x1_en = pRgnTbl->Rgn1x10Enable;
	pcid_pcid_regionaltable_ctrl_reg.pcid_regional_19x19tab_en = pRgnTbl->Tbl19x19Enable;
	pcid_pcid_regionaltable_ctrl_reg.pcid_regional_8x24_en = pRgnTbl->RgnWt.Enable;

	pcid_pcid_regional_19x19_table_index_bound_0_reg.col_bound1 = pRgnTbl->ColBnd[0];
	pcid_pcid_regional_19x19_table_index_bound_0_reg.col_bound2 = pRgnTbl->ColBnd[1];
	pcid_pcid_regional_19x19_table_index_bound_1_reg.col_bound3 = pRgnTbl->ColBnd[2];
	pcid_pcid_regional_19x19_table_index_bound_1_reg.col_bound4 = pRgnTbl->ColBnd[3];
	pcid_pcid_regional_19x19_table_index_bound_2_reg.col_bound5 = pRgnTbl->ColBnd[4];
	pcid_pcid_regional_19x19_table_index_bound_2_reg.col_bound6 = pRgnTbl->ColBnd[5];
	pcid_pcid_regional_19x19_table_index_bound_3_reg.col_bound7 = pRgnTbl->ColBnd[6];
	pcid_pcid_regional_19x19_table_index_bound_3_reg.col_bound8 = pRgnTbl->ColBnd[7];
	pcid_pcid_regional_19x19_table_index_bound_4_reg.col_bound9 = pRgnTbl->ColBnd[8];
	pcid_pcid_regional_19x19_table_index_bound_4_reg.col_bound10 = pRgnTbl->ColBnd[9];
	pcid_pcid_regional_19x19_table_index_bound_5_reg.col_bound11 = pRgnTbl->ColBnd[10];
	pcid_pcid_regional_19x19_table_index_bound_5_reg.col_bound12 = pRgnTbl->ColBnd[11];
	pcid_pcid_regional_19x19_table_index_bound_6_reg.col_bound13 = pRgnTbl->ColBnd[12];
	pcid_pcid_regional_19x19_table_index_bound_6_reg.col_bound14 = pRgnTbl->ColBnd[13];
	pcid_pcid_regional_19x19_table_index_bound_7_reg.col_bound15 = pRgnTbl->ColBnd[14];
	pcid_pcid_regional_19x19_table_index_bound_7_reg.col_bound16 = pRgnTbl->ColBnd[15];
	pcid_pcid_regional_19x19_table_index_bound_8_reg.col_bound17 = pRgnTbl->ColBnd[16];

	pcid_pcid_regional_19x19_table_index_bound_9_reg.row_bound1 = pRgnTbl->RowBnd[0];
	pcid_pcid_regional_19x19_table_index_bound_9_reg.row_bound2 = pRgnTbl->RowBnd[1];
	pcid_pcid_regional_19x19_table_index_bound_10_reg.row_bound3 = pRgnTbl->RowBnd[2];
	pcid_pcid_regional_19x19_table_index_bound_10_reg.row_bound4 = pRgnTbl->RowBnd[3];
	pcid_pcid_regional_19x19_table_index_bound_11_reg.row_bound5 = pRgnTbl->RowBnd[4];
	pcid_pcid_regional_19x19_table_index_bound_11_reg.row_bound6 = pRgnTbl->RowBnd[5];
	pcid_pcid_regional_19x19_table_index_bound_12_reg.row_bound7 = pRgnTbl->RowBnd[6];
	pcid_pcid_regional_19x19_table_index_bound_12_reg.row_bound8 = pRgnTbl->RowBnd[7];
	pcid_pcid_regional_19x19_table_index_bound_13_reg.row_bound9 = pRgnTbl->RowBnd[8];
	pcid_pcid_regional_19x19_table_index_bound_13_reg.row_bound10 = pRgnTbl->RowBnd[9];
	pcid_pcid_regional_19x19_table_index_bound_14_reg.row_bound11 = pRgnTbl->RowBnd[10];
	pcid_pcid_regional_19x19_table_index_bound_14_reg.row_bound12 = pRgnTbl->RowBnd[11];
	pcid_pcid_regional_19x19_table_index_bound_15_reg.row_bound13 = pRgnTbl->RowBnd[12];
	pcid_pcid_regional_19x19_table_index_bound_15_reg.row_bound14 = pRgnTbl->RowBnd[13];
	pcid_pcid_regional_19x19_table_index_bound_16_reg.row_bound15 = pRgnTbl->RowBnd[14];
	pcid_pcid_regional_19x19_table_index_bound_16_reg.row_bound16 = pRgnTbl->RowBnd[15];
	pcid_pcid_regional_19x19_table_index_bound_17_reg.row_bound17 = pRgnTbl->RowBnd[16];

	pcid_pcid_regional_index_horizontal_0_reg.rt_hor_index0 = pRgnTbl->HorIdx[0];
	pcid_pcid_regional_index_horizontal_0_reg.rt_hor_index1 = pRgnTbl->HorIdx[1];
	pcid_pcid_regional_index_horizontal_1_reg.rt_hor_index2 = pRgnTbl->HorIdx[2];

	pcid_pcid_regional_index_vertical_0_reg.rt_ver_index0 = pRgnTbl->VerIdx[0];
	pcid_pcid_regional_index_vertical_0_reg.rt_ver_index1 = pRgnTbl->VerIdx[1];
	pcid_pcid_regional_index_vertical_1_reg.rt_ver_index2 = pRgnTbl->VerIdx[2];

	pcid_pcid_regional_index_vertical_1_reg.rt_ver_index3 = pRgnTbl->VerIdx[3];
	pcid_pcid_regional_index_vertical_2_reg.rt_ver_index4 = pRgnTbl->VerIdx[4];
	pcid_pcid_regional_index_vertical_2_reg.rt_ver_index5 = pRgnTbl->VerIdx[5];

	pcid_pcid_regional_index_vertical_3_reg.rt_ver_index6 = pRgnTbl->VerIdx[6];
	pcid_pcid_regional_index_vertical_3_reg.rt_ver_index7 = pRgnTbl->VerIdx[7];
	pcid_pcid_regional_index_vertical_4_reg.rt_ver_index8 = pRgnTbl->VerIdx[8];

	pcid_pcid_regional_blendingstep_horizontal_reg.rt_hor_step0 = pRgnTbl->HorBldStep[0];
	pcid_pcid_regional_blendingstep_horizontal_reg.rt_hor_step1 = pRgnTbl->HorBldStep[1];
	pcid_pcid_regional_blendingstep_horizontal_reg.rt_hor_step2 = pRgnTbl->HorBldStep[2];
	pcid_pcid_regional_blendingstep_vertical_0_reg.rt_ver_step0 = pRgnTbl->VerBldStep[0];
	pcid_pcid_regional_blendingstep_vertical_0_reg.rt_ver_step1 = pRgnTbl->VerBldStep[1];
	pcid_pcid_regional_blendingstep_vertical_0_reg.rt_ver_step2 = pRgnTbl->VerBldStep[2];

	pcid_pcid_regional_blendingstep_vertical_0_reg.rt_ver_step3 = pRgnTbl->VerBldStep[3];
	pcid_pcid_regional_blendingstep_vertical_0_reg.rt_ver_step4 = pRgnTbl->VerBldStep[4];
	pcid_pcid_regional_blendingstep_vertical_0_reg.rt_ver_step5 = pRgnTbl->VerBldStep[5];
	pcid_pcid_regional_blendingstep_vertical_0_reg.rt_ver_step6 = pRgnTbl->VerBldStep[6];
	pcid_pcid_regional_blendingstep_vertical_0_reg.rt_ver_step7 = pRgnTbl->VerBldStep[7];
	pcid_pcid_regional_blendingstep_vertical_1_reg.rt_ver_step8 = pRgnTbl->VerBldStep[8];

	pcid_pcid_regionaltable_index_h_reg.pcid_regional_weight_height = pRgnTbl->RgnWt.Height;
	pcid_pcid_regionaltable_index_h_reg.pcid_regional_weight_width  = pRgnTbl->RgnWt.Width;
	pcid_pcid_regionaltable_index_v_reg.pcid_regional_weight_horbldstep = pRgnTbl->RgnWt.HorBldStep;
	pcid_pcid_regionaltable_index_v_reg.pcid_regional_weight_verbldstep = pRgnTbl->RgnWt.VerBldStep;

	IoReg_Write32(PCID_PCID_RegionalTable_Ctrl_reg, pcid_pcid_regionaltable_ctrl_reg.regValue);

	IoReg_Write32(PCID_PCID_Regional_19x19_Table_index_bound_0_reg, pcid_pcid_regional_19x19_table_index_bound_0_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_19x19_Table_index_bound_1_reg, pcid_pcid_regional_19x19_table_index_bound_1_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_19x19_Table_index_bound_2_reg, pcid_pcid_regional_19x19_table_index_bound_2_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_19x19_Table_index_bound_3_reg, pcid_pcid_regional_19x19_table_index_bound_3_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_19x19_Table_index_bound_4_reg, pcid_pcid_regional_19x19_table_index_bound_4_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_19x19_Table_index_bound_5_reg, pcid_pcid_regional_19x19_table_index_bound_5_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_19x19_Table_index_bound_6_reg, pcid_pcid_regional_19x19_table_index_bound_6_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_19x19_Table_index_bound_7_reg, pcid_pcid_regional_19x19_table_index_bound_7_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_19x19_Table_index_bound_8_reg, pcid_pcid_regional_19x19_table_index_bound_8_reg.regValue);

	IoReg_Write32(PCID_PCID_Regional_19x19_Table_index_bound_9_reg, pcid_pcid_regional_19x19_table_index_bound_9_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_19x19_Table_index_bound_10_reg, pcid_pcid_regional_19x19_table_index_bound_10_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_19x19_Table_index_bound_11_reg, pcid_pcid_regional_19x19_table_index_bound_11_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_19x19_Table_index_bound_12_reg, pcid_pcid_regional_19x19_table_index_bound_12_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_19x19_Table_index_bound_13_reg, pcid_pcid_regional_19x19_table_index_bound_13_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_19x19_Table_index_bound_14_reg, pcid_pcid_regional_19x19_table_index_bound_14_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_19x19_Table_index_bound_15_reg, pcid_pcid_regional_19x19_table_index_bound_15_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_19x19_Table_index_bound_16_reg, pcid_pcid_regional_19x19_table_index_bound_16_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_19x19_Table_index_bound_17_reg, pcid_pcid_regional_19x19_table_index_bound_17_reg.regValue);


	IoReg_Write32(PCID_PCID_Regional_Index_Horizontal_0_reg, pcid_pcid_regional_index_horizontal_0_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_Index_Horizontal_1_reg, pcid_pcid_regional_index_horizontal_1_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_Index_Vertical_0_reg, pcid_pcid_regional_index_vertical_0_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_Index_Vertical_1_reg, pcid_pcid_regional_index_vertical_1_reg.regValue);
       IoReg_Write32(PCID_PCID_Regional_Index_Vertical_2_reg, pcid_pcid_regional_index_vertical_2_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_Index_Vertical_3_reg, pcid_pcid_regional_index_vertical_3_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_Index_Vertical_4_reg, pcid_pcid_regional_index_vertical_4_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_BlendingStep_Horizontal_reg, pcid_pcid_regional_blendingstep_horizontal_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_BlendingStep_Vertical_0_reg, pcid_pcid_regional_blendingstep_vertical_0_reg.regValue);
	IoReg_Write32(PCID_PCID_Regional_BlendingStep_Vertical_1_reg, pcid_pcid_regional_blendingstep_vertical_1_reg.regValue);

	IoReg_Write32(PCID_PCID_RegionalTable_Index_H_reg, pcid_pcid_regionaltable_index_h_reg.regValue);
	IoReg_Write32(PCID_PCID_RegionalTable_Index_V_reg, pcid_pcid_regionaltable_index_v_reg.regValue);

}


void drvif_color_set_pcid_regional_table_valuetable(unsigned int *pTblValue, unsigned int RgnSel, DRV_pcid_channel_t Channel)
{
	unsigned int i=0, iter=0;
	unsigned char TableSize = 17;
	pcid_pcid_lut_addr_RBUS pcid_lut_addr_reg;
	pcid_pcid_lut_data_RBUS pcid_pcid_lut_data_reg;
	pcid_pcid_regionaltable_ctrl_RBUS pcid_pcid_regionaltable_ctrl_reg;

	if(!pTblValue)
	{
		VIPprintf("[Error][%s][%s] Regional Table Ptr is null!\n", __FILE__, __func__);
		return;
	}

	// Reset the internal counter
	pcid_pcid_regionaltable_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_RegionalTable_Ctrl_reg);
	pcid_pcid_regionaltable_ctrl_reg.regional_table_en = 0;
	IoReg_Write32(PCID_PCID_RegionalTable_Ctrl_reg, pcid_pcid_regionaltable_ctrl_reg.regValue);

	pcid_lut_addr_reg.regValue = IoReg_Read32(PCID_PCID_LUT_ADDR_reg);
	pcid_lut_addr_reg.pcid_regional_lut_ax_en = 0;
	IoReg_Write32(PCID_PCID_LUT_ADDR_reg, pcid_lut_addr_reg.regValue);

	pcid_pcid_regionaltable_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_RegionalTable_Ctrl_reg);
	pcid_pcid_regionaltable_ctrl_reg.regional_table_en = 1;
	IoReg_Write32(PCID_PCID_RegionalTable_Ctrl_reg, pcid_pcid_regionaltable_ctrl_reg.regValue);

	// Decide the read count
	TableSize = drvif_color_get_pcid_TableSize();
	iter = (drvif_color_get_pcid_RegionType()==_PCID_RGN_4x4)? TableSize*TableSize*2 : TableSize*TableSize;

	// Setup table access setting
	pcid_lut_addr_reg.regValue = IoReg_Read32(PCID_PCID_LUT_ADDR_reg);
	pcid_lut_addr_reg.pcid_lut_data_sel= 0;
	pcid_lut_addr_reg.pcid_lut_sel = Channel;
	pcid_lut_addr_reg.pcid_regional_lut_sel = RgnSel;
	pcid_lut_addr_reg.pcid_regional_lut_ax_en = 1;

	IoReg_Write32(PCID_PCID_LUT_ADDR_reg, pcid_lut_addr_reg.regValue);

	// Write the value into table
	for(i=0; i<iter; i++)
	{
		pcid_pcid_lut_data_reg.pcid_lut_data = pTblValue[i];
		IoReg_Write32(PCID_PCID_LUT_DATA_reg, pcid_pcid_lut_data_reg.regValue);
		VIPprintf("[VPQ][POD]WRgnTable[%d]=0x%08x\n", i, pTblValue[i]);
	}

	// Disable the table access
	pcid_lut_addr_reg.regValue = IoReg_Read32(PCID_PCID_LUT_ADDR_reg);
	pcid_lut_addr_reg.pcid_regional_lut_ax_en = 0;
	IoReg_Write32(PCID_PCID_LUT_ADDR_reg, pcid_lut_addr_reg.regValue);
}

void drvif_color_set_pcid_regional_weight_table_valuetable(unsigned int*pTblValue)
{
	unsigned int i =0;
	unsigned int TableSize= 8*24;
	unsigned int backup_8x24_en =0;
	pcid_pcid_lut_addr_RBUS pcid_lut_addr_reg;
	pcid_pcid_lut_data_RBUS pcid_pcid_lut_data_reg;
	pcid_pcid_regionaltable_ctrl_RBUS pcid_pcid_regionaltable_ctrl_reg;
	if(!pTblValue)
	{
		VIPprintf("[Error][%s][%s] Regional Weight Table Ptr is null!\n", __FILE__, __func__);
		return;
	}
	// Reset the internal counter
	pcid_pcid_regionaltable_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_RegionalTable_Ctrl_reg);
	backup_8x24_en = pcid_pcid_regionaltable_ctrl_reg.pcid_regional_8x24_en;
	pcid_pcid_regionaltable_ctrl_reg.pcid_regional_8x24_en= 0;
	IoReg_Write32(PCID_PCID_RegionalTable_Ctrl_reg, pcid_pcid_regionaltable_ctrl_reg.regValue);

	pcid_lut_addr_reg.regValue = IoReg_Read32(PCID_PCID_LUT_ADDR_reg);
	pcid_lut_addr_reg.pcid_regional_weight_ax_en= 0;
	IoReg_Write32(PCID_PCID_LUT_ADDR_reg, pcid_lut_addr_reg.regValue);

	pcid_pcid_regionaltable_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_RegionalTable_Ctrl_reg);
	pcid_pcid_regionaltable_ctrl_reg.pcid_regional_8x24_en = 1;
	IoReg_Write32(PCID_PCID_RegionalTable_Ctrl_reg, pcid_pcid_regionaltable_ctrl_reg.regValue);

	// Setup table access setting
	pcid_lut_addr_reg.regValue = IoReg_Read32(PCID_PCID_LUT_ADDR_reg);
	pcid_lut_addr_reg.pcid_lut_data_sel= 2;
	pcid_lut_addr_reg.pcid_regional_weight_ax_en = 1;
	IoReg_Write32(PCID_PCID_LUT_ADDR_reg, pcid_lut_addr_reg.regValue);

	// Write the value into table
	for(i=0; i<TableSize; i++)
	{
		pcid_pcid_lut_data_reg.pcid_lut_data = pTblValue[i];
		IoReg_Write32(PCID_PCID_LUT_DATA_reg, pcid_pcid_lut_data_reg.regValue);
		VIPprintf("[VPQ][POD]wRgnWtTable[%d]=0x%08x\n", i, pTblValue[i]);
	}

	// Disable the table access
	pcid_lut_addr_reg.regValue = IoReg_Read32(PCID_PCID_LUT_ADDR_reg);
	pcid_lut_addr_reg.pcid_regional_weight_ax_en = 0;
	IoReg_Write32(PCID_PCID_LUT_ADDR_reg, pcid_lut_addr_reg.regValue);

	pcid_pcid_regionaltable_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_RegionalTable_Ctrl_reg);
	pcid_pcid_regionaltable_ctrl_reg.pcid_regional_8x24_en = backup_8x24_en;
	IoReg_Write32(PCID_PCID_RegionalTable_Ctrl_reg, pcid_pcid_regionaltable_ctrl_reg.regValue);
}

void drvif_color_set_pcid_pixel_reference_setting(DRV_pcid_ref_setting_t* pPxlRef)
{
	pcid_pcid_line_1_RBUS pcid_line1_reg;
	pcid_pcid_line_2_RBUS pcid_line2_reg;
	pcid_pcid_line_3_RBUS pcid_line3_reg;
	pcid_pcid_line_4_RBUS pcid_line4_reg;

	pcid_line1_reg.regValue = IoReg_Read32(PCID_PCID_LINE_1_reg);
	pcid_line2_reg.regValue = IoReg_Read32(PCID_PCID_LINE_2_reg);
	pcid_line3_reg.regValue = IoReg_Read32(PCID_PCID_LINE_3_reg);
	pcid_line4_reg.regValue = IoReg_Read32(PCID_PCID_LINE_4_reg);

	/* LINE 1 EVEN PIXEL */
	pcid_line1_reg.line1_even_r_line_select  = pPxlRef->line1.even_r.line_sel;
	pcid_line1_reg.line1_even_r_point_select = pPxlRef->line1.even_r.point_sel;
	pcid_line1_reg.line1_even_r_color_select = pPxlRef->line1.even_r.color_sel;
	pcid_line1_reg.line1_even_g_line_select  = pPxlRef->line1.even_g.line_sel;
	pcid_line1_reg.line1_even_g_point_select = pPxlRef->line1.even_g.point_sel;
	pcid_line1_reg.line1_even_g_color_select = pPxlRef->line1.even_g.color_sel;
	pcid_line1_reg.line1_even_b_line_select  = pPxlRef->line1.even_b.line_sel;
	pcid_line1_reg.line1_even_b_point_select = pPxlRef->line1.even_b.point_sel;
	pcid_line1_reg.line1_even_b_color_select = pPxlRef->line1.even_b.color_sel;

	/* LINE 1 ODD PIXEL */
	pcid_line1_reg.line1_odd_r_line_select  = pPxlRef->line1.odd_r.line_sel;
	pcid_line1_reg.line1_odd_r_point_select = pPxlRef->line1.odd_r.point_sel;
	pcid_line1_reg.line1_odd_r_color_select = pPxlRef->line1.odd_r.color_sel;
	pcid_line1_reg.line1_odd_g_line_select  = pPxlRef->line1.odd_g.line_sel;
	pcid_line1_reg.line1_odd_g_point_select = pPxlRef->line1.odd_g.point_sel;
	pcid_line1_reg.line1_odd_g_color_select = pPxlRef->line1.odd_g.color_sel;
	pcid_line1_reg.line1_odd_b_line_select  = pPxlRef->line1.odd_b.line_sel;
	pcid_line1_reg.line1_odd_b_point_select = pPxlRef->line1.odd_b.point_sel;
	pcid_line1_reg.line1_odd_b_color_select = pPxlRef->line1.odd_b.color_sel;

	/* LINE 2 EVEN PIXEL */
	pcid_line2_reg.line2_even_r_line_select  = pPxlRef->line2.even_r.line_sel;
	pcid_line2_reg.line2_even_r_point_select = pPxlRef->line2.even_r.point_sel;
	pcid_line2_reg.line2_even_r_color_select = pPxlRef->line2.even_r.color_sel;
	pcid_line2_reg.line2_even_g_line_select  = pPxlRef->line2.even_g.line_sel;
	pcid_line2_reg.line2_even_g_point_select = pPxlRef->line2.even_g.point_sel;
	pcid_line2_reg.line2_even_g_color_select = pPxlRef->line2.even_g.color_sel;
	pcid_line2_reg.line2_even_b_line_select  = pPxlRef->line2.even_b.line_sel;
	pcid_line2_reg.line2_even_b_point_select = pPxlRef->line2.even_b.point_sel;
	pcid_line2_reg.line2_even_b_color_select = pPxlRef->line2.even_b.color_sel;

	/* LINE 2 ODD PIXEL */
	pcid_line2_reg.line2_odd_r_line_select  = pPxlRef->line2.odd_r.line_sel;
	pcid_line2_reg.line2_odd_r_point_select = pPxlRef->line2.odd_r.point_sel;
	pcid_line2_reg.line2_odd_r_color_select = pPxlRef->line2.odd_r.color_sel;
	pcid_line2_reg.line2_odd_g_line_select  = pPxlRef->line2.odd_g.line_sel;
	pcid_line2_reg.line2_odd_g_point_select = pPxlRef->line2.odd_g.point_sel;
	pcid_line2_reg.line2_odd_g_color_select = pPxlRef->line2.odd_g.color_sel;
	pcid_line2_reg.line2_odd_b_line_select  = pPxlRef->line2.odd_b.line_sel;
	pcid_line2_reg.line2_odd_b_point_select = pPxlRef->line2.odd_b.point_sel;
	pcid_line2_reg.line2_odd_b_color_select = pPxlRef->line2.odd_b.color_sel;

	/* LINE 3 EVEN PIXEL */
	pcid_line3_reg.line3_even_r_line_select  = pPxlRef->line3.even_r.line_sel;
	pcid_line3_reg.line3_even_r_point_select = pPxlRef->line3.even_r.point_sel;
	pcid_line3_reg.line3_even_r_color_select = pPxlRef->line3.even_r.color_sel;
	pcid_line3_reg.line3_even_g_line_select  = pPxlRef->line3.even_g.line_sel;
	pcid_line3_reg.line3_even_g_point_select = pPxlRef->line3.even_g.point_sel;
	pcid_line3_reg.line3_even_g_color_select = pPxlRef->line3.even_g.color_sel;
	pcid_line3_reg.line3_even_b_line_select  = pPxlRef->line3.even_b.line_sel;
	pcid_line3_reg.line3_even_b_point_select = pPxlRef->line3.even_b.point_sel;
	pcid_line3_reg.line3_even_b_color_select = pPxlRef->line3.even_b.color_sel;

	/* LINE 3 ODD PIXEL */
	pcid_line3_reg.line3_odd_r_line_select  = pPxlRef->line3.odd_r.line_sel;
	pcid_line3_reg.line3_odd_r_point_select = pPxlRef->line3.odd_r.point_sel;
	pcid_line3_reg.line3_odd_r_color_select = pPxlRef->line3.odd_r.color_sel;
	pcid_line3_reg.line3_odd_g_line_select  = pPxlRef->line3.odd_g.line_sel;
	pcid_line3_reg.line3_odd_g_point_select = pPxlRef->line3.odd_g.point_sel;
	pcid_line3_reg.line3_odd_g_color_select = pPxlRef->line3.odd_g.color_sel;
	pcid_line3_reg.line3_odd_b_line_select  = pPxlRef->line3.odd_b.line_sel;
	pcid_line3_reg.line3_odd_b_point_select = pPxlRef->line3.odd_b.point_sel;
	pcid_line3_reg.line3_odd_b_color_select = pPxlRef->line3.odd_b.color_sel;

	/* LINE 4 EVEN PIXEL */
	pcid_line4_reg.line4_even_r_line_select  = pPxlRef->line4.even_r.line_sel;
	pcid_line4_reg.line4_even_r_point_select = pPxlRef->line4.even_r.point_sel;
	pcid_line4_reg.line4_even_r_color_select = pPxlRef->line4.even_r.color_sel;
	pcid_line4_reg.line4_even_g_line_select  = pPxlRef->line4.even_g.line_sel;
	pcid_line4_reg.line4_even_g_point_select = pPxlRef->line4.even_g.point_sel;
	pcid_line4_reg.line4_even_g_color_select = pPxlRef->line4.even_g.color_sel;
	pcid_line4_reg.line4_even_b_line_select  = pPxlRef->line4.even_b.line_sel;
	pcid_line4_reg.line4_even_b_point_select = pPxlRef->line4.even_b.point_sel;
	pcid_line4_reg.line4_even_b_color_select = pPxlRef->line4.even_b.color_sel;

	/* LINE 4 ODD PIXEL */
	pcid_line4_reg.line4_odd_r_line_select  = pPxlRef->line4.odd_r.line_sel;
	pcid_line4_reg.line4_odd_r_point_select = pPxlRef->line4.odd_r.point_sel;
	pcid_line4_reg.line4_odd_r_color_select = pPxlRef->line4.odd_r.color_sel;
	pcid_line4_reg.line4_odd_g_line_select  = pPxlRef->line4.odd_g.line_sel;
	pcid_line4_reg.line4_odd_g_point_select = pPxlRef->line4.odd_g.point_sel;
	pcid_line4_reg.line4_odd_g_color_select = pPxlRef->line4.odd_g.color_sel;
	pcid_line4_reg.line4_odd_b_line_select  = pPxlRef->line4.odd_b.line_sel;
	pcid_line4_reg.line4_odd_b_point_select = pPxlRef->line4.odd_b.point_sel;
	pcid_line4_reg.line4_odd_b_color_select = pPxlRef->line4.odd_b.color_sel;

	IoReg_Write32(PCID_PCID_LINE_1_reg, pcid_line1_reg.regValue);
	IoReg_Write32(PCID_PCID_LINE_2_reg, pcid_line2_reg.regValue);
	IoReg_Write32(PCID_PCID_LINE_3_reg, pcid_line3_reg.regValue);
	IoReg_Write32(PCID_PCID_LINE_4_reg, pcid_line4_reg.regValue);
}

void drvif_color_set_pcid_data_setting(DRV_pcid_data_t* pPcid2Setting)
{
	pcid_pcid_ctrl_RBUS pcid_ctrl_reg;
	pcid_pcid_ctrl_2_RBUS pcid_ctrl2_reg;

	pcid_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_CTRL_reg);
	pcid_ctrl2_reg.regValue = IoReg_Read32(PCID_PCID_CTRL_2_reg);

	/* Enable control */
	pcid_ctrl_reg.first_line_boundary_mode = pPcid2Setting->FstBndMode;
	pcid_ctrl_reg.first_line_cfg = pPcid2Setting->FstBndCFG;
	pcid_ctrl_reg.general_boundary_mode = pPcid2Setting->GenBndMode;
	pcid_ctrl_reg.general_cfg = pPcid2Setting->GenBndCFG;

	pcid_ctrl2_reg.pcid_thd_en      = pPcid2Setting->thd_en;
	pcid_ctrl2_reg.pcid_thd_mode    = pPcid2Setting->thd_mode;
	pcid_ctrl2_reg.pcid_tab1_r_thd  = pPcid2Setting->tbl1_r_th;
	pcid_ctrl2_reg.pcid_tab1_g_thd  = pPcid2Setting->tbl1_g_th;
	pcid_ctrl2_reg.pcid_tab1_b_thd  = pPcid2Setting->tbl1_b_th;
	pcid_ctrl2_reg.pcid_tab1_xtr_en = pPcid2Setting->xtr_tbl1_en;

	IoReg_Write32(PCID_PCID_CTRL_reg  , pcid_ctrl_reg.regValue);
	IoReg_Write32(PCID_PCID_CTRL_2_reg, pcid_ctrl2_reg.regValue);
}

void drvif_color_get_pcid_regional_table_setting(DRV_pcid_RgnTbl_t* pRgnTbl)
{
	pcid_pcid_regionaltable_ctrl_RBUS pcid_pcid_regionaltable_ctrl_reg;
	pcid_pcid_regional_19x19_table_index_bound_0_RBUS pcid_pcid_regional_19x19_table_index_bound_0_reg;
	pcid_pcid_regional_19x19_table_index_bound_1_RBUS pcid_pcid_regional_19x19_table_index_bound_1_reg;
	pcid_pcid_regional_19x19_table_index_bound_2_RBUS pcid_pcid_regional_19x19_table_index_bound_2_reg;
	pcid_pcid_regional_19x19_table_index_bound_3_RBUS pcid_pcid_regional_19x19_table_index_bound_3_reg;
	pcid_pcid_regional_19x19_table_index_bound_4_RBUS pcid_pcid_regional_19x19_table_index_bound_4_reg;
	pcid_pcid_regional_19x19_table_index_bound_5_RBUS pcid_pcid_regional_19x19_table_index_bound_5_reg;
	pcid_pcid_regional_19x19_table_index_bound_6_RBUS pcid_pcid_regional_19x19_table_index_bound_6_reg;
	pcid_pcid_regional_19x19_table_index_bound_7_RBUS pcid_pcid_regional_19x19_table_index_bound_7_reg;
	pcid_pcid_regional_19x19_table_index_bound_8_RBUS pcid_pcid_regional_19x19_table_index_bound_8_reg;

	pcid_pcid_regional_19x19_table_index_bound_9_RBUS pcid_pcid_regional_19x19_table_index_bound_9_reg;
	pcid_pcid_regional_19x19_table_index_bound_10_RBUS pcid_pcid_regional_19x19_table_index_bound_10_reg;
	pcid_pcid_regional_19x19_table_index_bound_11_RBUS pcid_pcid_regional_19x19_table_index_bound_11_reg;
	pcid_pcid_regional_19x19_table_index_bound_12_RBUS pcid_pcid_regional_19x19_table_index_bound_12_reg;
	pcid_pcid_regional_19x19_table_index_bound_13_RBUS pcid_pcid_regional_19x19_table_index_bound_13_reg;
	pcid_pcid_regional_19x19_table_index_bound_14_RBUS pcid_pcid_regional_19x19_table_index_bound_14_reg;
	pcid_pcid_regional_19x19_table_index_bound_15_RBUS pcid_pcid_regional_19x19_table_index_bound_15_reg;
	pcid_pcid_regional_19x19_table_index_bound_16_RBUS pcid_pcid_regional_19x19_table_index_bound_16_reg;
	pcid_pcid_regional_19x19_table_index_bound_17_RBUS pcid_pcid_regional_19x19_table_index_bound_17_reg;

	pcid_pcid_regional_index_horizontal_0_RBUS pcid_pcid_regional_index_horizontal_0_reg;
	pcid_pcid_regional_index_horizontal_1_RBUS pcid_pcid_regional_index_horizontal_1_reg;
	pcid_pcid_regional_index_vertical_0_RBUS pcid_pcid_regional_index_vertical_0_reg;
	pcid_pcid_regional_index_vertical_1_RBUS pcid_pcid_regional_index_vertical_1_reg;
	pcid_pcid_regional_index_vertical_2_RBUS pcid_pcid_regional_index_vertical_2_reg;
	pcid_pcid_regional_index_vertical_3_RBUS pcid_pcid_regional_index_vertical_3_reg;
	pcid_pcid_regional_index_vertical_4_RBUS pcid_pcid_regional_index_vertical_4_reg;
	pcid_pcid_regional_blendingstep_horizontal_RBUS pcid_pcid_regional_blendingstep_horizontal_reg;
	pcid_pcid_regional_blendingstep_vertical_0_RBUS pcid_pcid_regional_blendingstep_vertical_0_reg;
	pcid_pcid_regional_blendingstep_vertical_1_RBUS pcid_pcid_regional_blendingstep_vertical_1_reg;

	pcid_pcid_regionaltable_index_h_RBUS pcid_pcid_regionaltable_index_h_reg;
	pcid_pcid_regionaltable_index_v_RBUS pcid_pcid_regionaltable_index_v_reg;

	pcid_pcid_regionaltable_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_RegionalTable_Ctrl_reg);

	pcid_pcid_regional_19x19_table_index_bound_0_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_0_reg);
	pcid_pcid_regional_19x19_table_index_bound_1_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_1_reg);
	pcid_pcid_regional_19x19_table_index_bound_2_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_2_reg);
	pcid_pcid_regional_19x19_table_index_bound_3_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_3_reg);
	pcid_pcid_regional_19x19_table_index_bound_4_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_4_reg);
	pcid_pcid_regional_19x19_table_index_bound_5_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_5_reg);
	pcid_pcid_regional_19x19_table_index_bound_6_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_6_reg);
	pcid_pcid_regional_19x19_table_index_bound_7_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_7_reg);
	pcid_pcid_regional_19x19_table_index_bound_8_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_8_reg);

	pcid_pcid_regional_19x19_table_index_bound_9_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_9_reg);
	pcid_pcid_regional_19x19_table_index_bound_10_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_10_reg);
	pcid_pcid_regional_19x19_table_index_bound_11_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_11_reg);
	pcid_pcid_regional_19x19_table_index_bound_12_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_12_reg);
	pcid_pcid_regional_19x19_table_index_bound_13_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_13_reg);
	pcid_pcid_regional_19x19_table_index_bound_14_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_14_reg);
	pcid_pcid_regional_19x19_table_index_bound_15_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_15_reg);
	pcid_pcid_regional_19x19_table_index_bound_16_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_16_reg);
	pcid_pcid_regional_19x19_table_index_bound_17_reg.regValue = IoReg_Read32(PCID_PCID_Regional_19x19_Table_index_bound_17_reg);

	pcid_pcid_regional_index_horizontal_0_reg.regValue = IoReg_Read32(PCID_PCID_Regional_Index_Horizontal_0_reg);
	pcid_pcid_regional_index_horizontal_1_reg.regValue = IoReg_Read32(PCID_PCID_Regional_Index_Horizontal_1_reg);
	pcid_pcid_regional_index_vertical_0_reg.regValue = IoReg_Read32(PCID_PCID_Regional_Index_Vertical_0_reg);
	pcid_pcid_regional_index_vertical_1_reg.regValue = IoReg_Read32(PCID_PCID_Regional_Index_Vertical_1_reg);
	pcid_pcid_regional_index_vertical_2_reg.regValue = IoReg_Read32(PCID_PCID_Regional_Index_Vertical_2_reg);
	pcid_pcid_regional_index_vertical_3_reg.regValue = IoReg_Read32(PCID_PCID_Regional_Index_Vertical_3_reg);
	pcid_pcid_regional_index_vertical_4_reg.regValue = IoReg_Read32(PCID_PCID_Regional_Index_Vertical_4_reg);
	pcid_pcid_regional_blendingstep_horizontal_reg.regValue = IoReg_Read32(PCID_PCID_Regional_BlendingStep_Horizontal_reg);
	pcid_pcid_regional_blendingstep_vertical_0_reg.regValue = IoReg_Read32(PCID_PCID_Regional_BlendingStep_Vertical_0_reg);
	pcid_pcid_regional_blendingstep_vertical_1_reg.regValue = IoReg_Read32(PCID_PCID_Regional_BlendingStep_Vertical_1_reg);

       pcid_pcid_regionaltable_index_h_reg.regValue = IoReg_Read32(PCID_PCID_RegionalTable_Index_H_reg);
       pcid_pcid_regionaltable_index_v_reg.regValue = IoReg_Read32(PCID_PCID_RegionalTable_Index_V_reg);

	//pcid_pcid_regionaltable_ctrl_reg.regional_table_en = 1;
	pRgnTbl->Rgn1x10Enable = pcid_pcid_regionaltable_ctrl_reg.pcid_regional_10x1_en ;
	pRgnTbl->Tbl19x19Enable = pcid_pcid_regionaltable_ctrl_reg.pcid_regional_19x19tab_en ;
	pRgnTbl->RgnWt.Enable = pcid_pcid_regionaltable_ctrl_reg.pcid_regional_8x24_en ;

	pRgnTbl->ColBnd[0]	 = pcid_pcid_regional_19x19_table_index_bound_0_reg.col_bound1	;
	pRgnTbl->ColBnd[1]	 = pcid_pcid_regional_19x19_table_index_bound_0_reg.col_bound2	;
	pRgnTbl->ColBnd[2]	 = pcid_pcid_regional_19x19_table_index_bound_1_reg.col_bound3	;
	pRgnTbl->ColBnd[3]	 = pcid_pcid_regional_19x19_table_index_bound_1_reg.col_bound4	;
	pRgnTbl->ColBnd[4]	 = pcid_pcid_regional_19x19_table_index_bound_2_reg.col_bound5	;
	pRgnTbl->ColBnd[5]	 = pcid_pcid_regional_19x19_table_index_bound_2_reg.col_bound6	;
	pRgnTbl->ColBnd[6]	 = pcid_pcid_regional_19x19_table_index_bound_3_reg.col_bound7	;
	pRgnTbl->ColBnd[7]	 = pcid_pcid_regional_19x19_table_index_bound_3_reg.col_bound8	;
	pRgnTbl->ColBnd[8]	 = pcid_pcid_regional_19x19_table_index_bound_4_reg.col_bound9	;
	pRgnTbl->ColBnd[9]	 = pcid_pcid_regional_19x19_table_index_bound_4_reg.col_bound10 ;
	pRgnTbl->ColBnd[10]  = pcid_pcid_regional_19x19_table_index_bound_5_reg.col_bound11 ;
	pRgnTbl->ColBnd[11]  = pcid_pcid_regional_19x19_table_index_bound_5_reg.col_bound12 ;
	pRgnTbl->ColBnd[12]  = pcid_pcid_regional_19x19_table_index_bound_6_reg.col_bound13 ;
	pRgnTbl->ColBnd[13]  = pcid_pcid_regional_19x19_table_index_bound_6_reg.col_bound14 ;
	pRgnTbl->ColBnd[14]  = pcid_pcid_regional_19x19_table_index_bound_7_reg.col_bound15 ;
	pRgnTbl->ColBnd[15]  = pcid_pcid_regional_19x19_table_index_bound_7_reg.col_bound16 ;
	pRgnTbl->ColBnd[16]  = pcid_pcid_regional_19x19_table_index_bound_8_reg.col_bound17 ;

	pRgnTbl->RowBnd[0]	 = pcid_pcid_regional_19x19_table_index_bound_9_reg.row_bound1	;
	pRgnTbl->RowBnd[1]	 = pcid_pcid_regional_19x19_table_index_bound_9_reg.row_bound2	;
	pRgnTbl->RowBnd[2]	 = pcid_pcid_regional_19x19_table_index_bound_10_reg.row_bound3 ;
	pRgnTbl->RowBnd[3]	 = pcid_pcid_regional_19x19_table_index_bound_10_reg.row_bound4 ;
	pRgnTbl->RowBnd[4]	 = pcid_pcid_regional_19x19_table_index_bound_11_reg.row_bound5 ;
	pRgnTbl->RowBnd[5]	 = pcid_pcid_regional_19x19_table_index_bound_11_reg.row_bound6 ;
	pRgnTbl->RowBnd[6]	 = pcid_pcid_regional_19x19_table_index_bound_12_reg.row_bound7 ;
	pRgnTbl->RowBnd[7]	 = pcid_pcid_regional_19x19_table_index_bound_12_reg.row_bound8 ;
	pRgnTbl->RowBnd[8]	 = pcid_pcid_regional_19x19_table_index_bound_13_reg.row_bound9 ;
	pRgnTbl->RowBnd[9]	 = pcid_pcid_regional_19x19_table_index_bound_13_reg.row_bound10;
	pRgnTbl->RowBnd[10]  = pcid_pcid_regional_19x19_table_index_bound_14_reg.row_bound11;
	pRgnTbl->RowBnd[11]  = pcid_pcid_regional_19x19_table_index_bound_14_reg.row_bound12;
	pRgnTbl->RowBnd[12]  = pcid_pcid_regional_19x19_table_index_bound_15_reg.row_bound13;
	pRgnTbl->RowBnd[13]  = pcid_pcid_regional_19x19_table_index_bound_15_reg.row_bound14;
	pRgnTbl->RowBnd[14]  = pcid_pcid_regional_19x19_table_index_bound_16_reg.row_bound15;
	pRgnTbl->RowBnd[15]  = pcid_pcid_regional_19x19_table_index_bound_16_reg.row_bound16;
	pRgnTbl->RowBnd[16]  = pcid_pcid_regional_19x19_table_index_bound_17_reg.row_bound17;

	pRgnTbl->HorIdx[0]		 = pcid_pcid_regional_index_horizontal_0_reg.rt_hor_index0	  ;
	pRgnTbl->HorIdx[1]		 = pcid_pcid_regional_index_horizontal_0_reg.rt_hor_index1	  ;
	pRgnTbl->HorIdx[2]		 = pcid_pcid_regional_index_horizontal_1_reg.rt_hor_index2	  ;

	pRgnTbl->VerIdx[0]		 = pcid_pcid_regional_index_vertical_0_reg.rt_ver_index0	  ;
	pRgnTbl->VerIdx[1]		 = pcid_pcid_regional_index_vertical_0_reg.rt_ver_index1	  ;
	pRgnTbl->VerIdx[2]		 = pcid_pcid_regional_index_vertical_1_reg.rt_ver_index2	  ;

	pRgnTbl->VerIdx[3]		 = pcid_pcid_regional_index_vertical_1_reg.rt_ver_index3	  ;
	pRgnTbl->VerIdx[4]		 = pcid_pcid_regional_index_vertical_2_reg.rt_ver_index4	  ;
	pRgnTbl->VerIdx[5]		 = pcid_pcid_regional_index_vertical_2_reg.rt_ver_index5	  ;

	pRgnTbl->VerIdx[6]		 = pcid_pcid_regional_index_vertical_3_reg.rt_ver_index6	  ;
	pRgnTbl->VerIdx[7]		 = pcid_pcid_regional_index_vertical_3_reg.rt_ver_index7	  ;
	pRgnTbl->VerIdx[8]		 = pcid_pcid_regional_index_vertical_4_reg.rt_ver_index8	  ;

	pRgnTbl->HorBldStep[0]	 = pcid_pcid_regional_blendingstep_horizontal_reg.rt_hor_step0;
	pRgnTbl->HorBldStep[1]	 = pcid_pcid_regional_blendingstep_horizontal_reg.rt_hor_step1;
	pRgnTbl->HorBldStep[2]	 = pcid_pcid_regional_blendingstep_horizontal_reg.rt_hor_step2;
	pRgnTbl->VerBldStep[0]	 = pcid_pcid_regional_blendingstep_vertical_0_reg.rt_ver_step0;
	pRgnTbl->VerBldStep[1]	 = pcid_pcid_regional_blendingstep_vertical_0_reg.rt_ver_step1;
	pRgnTbl->VerBldStep[2]	 = pcid_pcid_regional_blendingstep_vertical_0_reg.rt_ver_step2;

	pRgnTbl->VerBldStep[3]	 = pcid_pcid_regional_blendingstep_vertical_0_reg.rt_ver_step3;
	pRgnTbl->VerBldStep[4]	 = pcid_pcid_regional_blendingstep_vertical_0_reg.rt_ver_step4;
	pRgnTbl->VerBldStep[5]	 = pcid_pcid_regional_blendingstep_vertical_0_reg.rt_ver_step5;
	pRgnTbl->VerBldStep[6]	 = pcid_pcid_regional_blendingstep_vertical_0_reg.rt_ver_step6;
	pRgnTbl->VerBldStep[7]	 = pcid_pcid_regional_blendingstep_vertical_0_reg.rt_ver_step7;
	pRgnTbl->VerBldStep[8]	 = pcid_pcid_regional_blendingstep_vertical_1_reg.rt_ver_step8;

	pRgnTbl->RgnWt.Height = pcid_pcid_regionaltable_index_h_reg.pcid_regional_weight_height ;
	pRgnTbl->RgnWt.Width  = pcid_pcid_regionaltable_index_h_reg.pcid_regional_weight_width ;
	pRgnTbl->RgnWt.HorBldStep = pcid_pcid_regionaltable_index_v_reg.pcid_regional_weight_horbldstep ;
	pRgnTbl->RgnWt.VerBldStep = pcid_pcid_regionaltable_index_v_reg.pcid_regional_weight_verbldstep ;
}

void drvif_color_get_pcid_pixel_reference_setting(DRV_pcid_ref_setting_t* pPxlRef)
{
	pcid_pcid_line_1_RBUS pcid_line1_reg;
	pcid_pcid_line_2_RBUS pcid_line2_reg;
	pcid_pcid_line_3_RBUS pcid_line3_reg;
	pcid_pcid_line_4_RBUS pcid_line4_reg;

	pcid_line1_reg.regValue = IoReg_Read32(PCID_PCID_LINE_1_reg);
	pcid_line2_reg.regValue = IoReg_Read32(PCID_PCID_LINE_2_reg);
	pcid_line3_reg.regValue = IoReg_Read32(PCID_PCID_LINE_3_reg);
	pcid_line4_reg.regValue = IoReg_Read32(PCID_PCID_LINE_4_reg);

	/* LINE 1 EVEN PIXEL */
	pPxlRef->line1.even_r.line_sel	 = pcid_line1_reg.line1_even_r_line_select ;
	pPxlRef->line1.even_r.point_sel  = pcid_line1_reg.line1_even_r_point_select;
	pPxlRef->line1.even_r.color_sel  = pcid_line1_reg.line1_even_r_color_select;
	pPxlRef->line1.even_g.line_sel	 = pcid_line1_reg.line1_even_g_line_select ;
	pPxlRef->line1.even_g.point_sel  = pcid_line1_reg.line1_even_g_point_select;
	pPxlRef->line1.even_g.color_sel  = pcid_line1_reg.line1_even_g_color_select;
	pPxlRef->line1.even_b.line_sel	 = pcid_line1_reg.line1_even_b_line_select ;
	pPxlRef->line1.even_b.point_sel  = pcid_line1_reg.line1_even_b_point_select;
	pPxlRef->line1.even_b.color_sel  = pcid_line1_reg.line1_even_b_color_select;

	/* LINE 1 ODD PIXEL */
	pPxlRef->line1.odd_r.line_sel	=  pcid_line1_reg.line1_odd_r_line_select  ;
	pPxlRef->line1.odd_r.point_sel	=  pcid_line1_reg.line1_odd_r_point_select ;
	pPxlRef->line1.odd_r.color_sel	=  pcid_line1_reg.line1_odd_r_color_select ;
	pPxlRef->line1.odd_g.line_sel	=  pcid_line1_reg.line1_odd_g_line_select  ;
	pPxlRef->line1.odd_g.point_sel	=  pcid_line1_reg.line1_odd_g_point_select ;
	pPxlRef->line1.odd_g.color_sel	=  pcid_line1_reg.line1_odd_g_color_select ;
	pPxlRef->line1.odd_b.line_sel	=  pcid_line1_reg.line1_odd_b_line_select  ;
	pPxlRef->line1.odd_b.point_sel	=  pcid_line1_reg.line1_odd_b_point_select ;
	pPxlRef->line1.odd_b.color_sel	=  pcid_line1_reg.line1_odd_b_color_select ;

	/* LINE 2 EVEN PIXEL */
	pPxlRef->line2.even_r.line_sel	 = pcid_line2_reg.line2_even_r_line_select ;
	pPxlRef->line2.even_r.point_sel  = pcid_line2_reg.line2_even_r_point_select;
	pPxlRef->line2.even_r.color_sel  = pcid_line2_reg.line2_even_r_color_select;
	pPxlRef->line2.even_g.line_sel	 = pcid_line2_reg.line2_even_g_line_select ;
	pPxlRef->line2.even_g.point_sel  = pcid_line2_reg.line2_even_g_point_select;
	pPxlRef->line2.even_g.color_sel  = pcid_line2_reg.line2_even_g_color_select;
	pPxlRef->line2.even_b.line_sel	 = pcid_line2_reg.line2_even_b_line_select ;
	pPxlRef->line2.even_b.point_sel  = pcid_line2_reg.line2_even_b_point_select;
	pPxlRef->line2.even_b.color_sel  = pcid_line2_reg.line2_even_b_color_select;

	/* LINE 2 ODD PIXEL */
	pPxlRef->line2.odd_r.line_sel	=  pcid_line2_reg.line2_odd_r_line_select  ;
	pPxlRef->line2.odd_r.point_sel	=  pcid_line2_reg.line2_odd_r_point_select ;
	pPxlRef->line2.odd_r.color_sel	=  pcid_line2_reg.line2_odd_r_color_select ;
	pPxlRef->line2.odd_g.line_sel	=  pcid_line2_reg.line2_odd_g_line_select  ;
	pPxlRef->line2.odd_g.point_sel	=  pcid_line2_reg.line2_odd_g_point_select ;
	pPxlRef->line2.odd_g.color_sel	=  pcid_line2_reg.line2_odd_g_color_select ;
	pPxlRef->line2.odd_b.line_sel	=  pcid_line2_reg.line2_odd_b_line_select  ;
	pPxlRef->line2.odd_b.point_sel	=  pcid_line2_reg.line2_odd_b_point_select ;
	pPxlRef->line2.odd_b.color_sel	=  pcid_line2_reg.line2_odd_b_color_select ;

	/* LINE 3 EVEN PIXEL */
	pPxlRef->line3.even_r.line_sel	 = pcid_line3_reg.line3_even_r_line_select ;
	pPxlRef->line3.even_r.point_sel  = pcid_line3_reg.line3_even_r_point_select;
	pPxlRef->line3.even_r.color_sel  = pcid_line3_reg.line3_even_r_color_select;
	pPxlRef->line3.even_g.line_sel	 = pcid_line3_reg.line3_even_g_line_select ;
	pPxlRef->line3.even_g.point_sel  = pcid_line3_reg.line3_even_g_point_select;
	pPxlRef->line3.even_g.color_sel  = pcid_line3_reg.line3_even_g_color_select;
	pPxlRef->line3.even_b.line_sel	 = pcid_line3_reg.line3_even_b_line_select ;
	pPxlRef->line3.even_b.point_sel  = pcid_line3_reg.line3_even_b_point_select;
	pPxlRef->line3.even_b.color_sel  = pcid_line3_reg.line3_even_b_color_select;

	/* LINE 3 ODD PIXEL */
	pPxlRef->line3.odd_r.line_sel	=  pcid_line3_reg.line3_odd_r_line_select  ;
	pPxlRef->line3.odd_r.point_sel	=  pcid_line3_reg.line3_odd_r_point_select ;
	pPxlRef->line3.odd_r.color_sel	=  pcid_line3_reg.line3_odd_r_color_select ;
	pPxlRef->line3.odd_g.line_sel	=  pcid_line3_reg.line3_odd_g_line_select  ;
	pPxlRef->line3.odd_g.point_sel	=  pcid_line3_reg.line3_odd_g_point_select ;
	pPxlRef->line3.odd_g.color_sel	=  pcid_line3_reg.line3_odd_g_color_select ;
	pPxlRef->line3.odd_b.line_sel	=  pcid_line3_reg.line3_odd_b_line_select  ;
	pPxlRef->line3.odd_b.point_sel	=  pcid_line3_reg.line3_odd_b_point_select ;
	pPxlRef->line3.odd_b.color_sel	=  pcid_line3_reg.line3_odd_b_color_select ;

	/* LINE 4 EVEN PIXEL */
	pPxlRef->line4.even_r.line_sel	 = pcid_line4_reg.line4_even_r_line_select ;
	pPxlRef->line4.even_r.point_sel  = pcid_line4_reg.line4_even_r_point_select;
	pPxlRef->line4.even_r.color_sel  = pcid_line4_reg.line4_even_r_color_select;
	pPxlRef->line4.even_g.line_sel	 = pcid_line4_reg.line4_even_g_line_select ;
	pPxlRef->line4.even_g.point_sel  = pcid_line4_reg.line4_even_g_point_select;
	pPxlRef->line4.even_g.color_sel  = pcid_line4_reg.line4_even_g_color_select;
	pPxlRef->line4.even_b.line_sel	 = pcid_line4_reg.line4_even_b_line_select ;
	pPxlRef->line4.even_b.point_sel  = pcid_line4_reg.line4_even_b_point_select;
	pPxlRef->line4.even_b.color_sel  = pcid_line4_reg.line4_even_b_color_select;

	/* LINE 4 ODD PIXEL */
	pPxlRef->line4.odd_r.line_sel	=  pcid_line4_reg.line4_odd_r_line_select  ;
	pPxlRef->line4.odd_r.point_sel	=  pcid_line4_reg.line4_odd_r_point_select ;
	pPxlRef->line4.odd_r.color_sel	=  pcid_line4_reg.line4_odd_r_color_select ;
	pPxlRef->line4.odd_g.line_sel	=  pcid_line4_reg.line4_odd_g_line_select  ;
	pPxlRef->line4.odd_g.point_sel	=  pcid_line4_reg.line4_odd_g_point_select ;
	pPxlRef->line4.odd_g.color_sel	=  pcid_line4_reg.line4_odd_g_color_select ;
	pPxlRef->line4.odd_b.line_sel	=  pcid_line4_reg.line4_odd_b_line_select  ;
	pPxlRef->line4.odd_b.point_sel	=  pcid_line4_reg.line4_odd_b_point_select ;
	pPxlRef->line4.odd_b.color_sel	=  pcid_line4_reg.line4_odd_b_color_select ;
}

void drvif_color_get_pcid_data_setting(DRV_pcid_data_t* pPcid2Setting)
{
	pcid_pcid_ctrl_RBUS pcid_ctrl_reg;
	pcid_pcid_ctrl_2_RBUS pcid_ctrl2_reg;

	pcid_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_CTRL_reg);
	pcid_ctrl2_reg.regValue = IoReg_Read32(PCID_PCID_CTRL_2_reg);

	/* Enable control */
	pPcid2Setting->FstBndMode = pcid_ctrl_reg.first_line_boundary_mode ;
	pPcid2Setting->FstBndCFG = pcid_ctrl_reg.first_line_cfg ;
	pPcid2Setting->GenBndMode = pcid_ctrl_reg.general_boundary_mode ;
	pPcid2Setting->GenBndCFG = pcid_ctrl_reg.general_cfg ;

	pPcid2Setting->thd_en		 = pcid_ctrl2_reg.pcid_thd_en	  ;
	pPcid2Setting->thd_mode 	 = pcid_ctrl2_reg.pcid_thd_mode   ;
	pPcid2Setting->tbl1_r_th	 = pcid_ctrl2_reg.pcid_tab1_r_thd ;
	pPcid2Setting->tbl1_g_th	 = pcid_ctrl2_reg.pcid_tab1_g_thd ;
	pPcid2Setting->tbl1_b_th	 = pcid_ctrl2_reg.pcid_tab1_b_thd ;
	pPcid2Setting->xtr_tbl1_en	 = pcid_ctrl2_reg.pcid_tab1_xtr_en;
}

void drvif_color_enable_LGD_POD_CLK()
{
	if ((IoReg_Read32(SYS_REG_SYS_CLKEN3_reg) & _BIT14) == 0)	// LGD POD clk not enable
	{
		IoReg_Write32(SYS_REG_SYS_CLKEN3_reg, 0x00004001);	 //LGD POD CLK
		IoReg_Write32(SYS_REG_SYS_SRST3_reg, 0x00004001);	 //LGD POD reset
	}
}

void drvif_color_enable_LGD_POD_double_buffer()
{
	unsigned long flags;//for spin_lock_irqsave

	/* POD DB */
	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	IoReg_SetBits(PPOVERLAY_Double_Buffer_CTRL_reg, PPOVERLAY_Double_Buffer_CTRL_dreg_dbuf_en_mask);
	/*  set stage2 smooth toggle protect */
	IoReg_SetBits(PPOVERLAY_dispd_stage2_sm_ctrl_reg, PPOVERLAY_dispd_stage2_sm_ctrl_dispd_stage2_smooth_toggle_protect_mask);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
}


void drvif_color_set_LGD_POD(unsigned char* pTblValue)
{
	unsigned int i = 0;
	unsigned int reg = 0;

	if (drvif_color_get_HW_MEX_STATUS(MEX_POD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_POD != 1, TBL\n");
		return;
	}

	// 0xb802BB04 ~ 0xb802BCFC
	for(i=0; i<(508/4); i++){
		reg = (pTblValue[i*4+0]<<24) + (pTblValue[i*4+1]<<16) + (pTblValue[i*4+2]<<8) + (pTblValue[i*4+3]<<0);
		IoReg_Write32(LG_POD_POD_reg00_reg+(i*4), reg);
	}

	// 0xb8026C00 ~ 0xb8026FFC
	for(i=0; i<((1532-508)/4); i++){
		reg = (pTblValue[i*4+508+0]<<24) + (pTblValue[i*4+508+1]<<16) + (pTblValue[i*4+508+2]<<8) + (pTblValue[i*4+508+3]<<0);
		IoReg_Write32(LG_POD_POD_reg1fc_reg+(i*4), reg);
	}

	// 0xb802F000 ~ 0xb802F138
	for(i=0; i<((LGE_POD_TBL_LEN-1532)/4); i++){
		reg = (pTblValue[i*4+1532+0]<<24) + (pTblValue[i*4+1532+1]<<16) + (pTblValue[i*4+1532+2]<<8) + (pTblValue[i*4+1532+3]<<0);
		IoReg_Write32(LG_POD_POD_reg5fc_reg+(i*4), reg);
	}

}

void drvif_color_set_Tcon_pcid1_pcid2_LGPOD_fcic_enable(unsigned char pcid1_en,unsigned char pcid_2_en,unsigned char LGPOD_en,unsigned char fcic_en)
{
	unsigned long flags;//for spin_lock_irqsave
	int timeoutcnt = 100;
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
//add double buffer enable-------------------------------------------------------
    ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_Reg;

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

	if (drvif_color_get_HW_MEX_STATUS(MEX_POD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_POD != 1, LGPOD_en=0\n");
		LGPOD_en = 0;
	}
	if (drvif_color_get_HW_MEX_STATUS(MEX_HCIC) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_HCIC != 1, fcic_en=0\n");
		fcic_en = 0;
	}
	if (drvif_color_get_HW_MEX_STATUS(MEX_LINE_OD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_LINE_OD != 1, pcid1_en=pcid2_en=0\n");
		pcid1_en = 0;
		pcid_2_en = 0;
	}

	display_timing_ctrl2_reg.lineod_en = pcid1_en;
	display_timing_ctrl2_reg.lineod2_en = pcid_2_en;
	display_timing_ctrl2_reg.pod_enable = LGPOD_en;
	display_timing_ctrl2_reg.hcic_enable = fcic_en;

	if (fcic_en) {
		display_timing_ctrl2_reg.valc_en = 0;
		rtd_pr_vpq_info("drvif_color_set_fcic_enable, need to disable valc_en\n");
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
		rtd_pr_vpq_emerg("drvif_color_set_fcic_enable, timeout!\n");

}


void drvif_color_set_pcid_enable(unsigned char bEnable)
{
	// put the block at the front position
	unsigned long flags;//for spin_lock_irqsave
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;

	if (drvif_color_get_HW_MEX_STATUS(MEX_LINE_OD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_LINE_OD != 1, EN1\n");
		return;
	}

	/*  set stage2 smooth toggle protect */
	IoReg_SetBits(PPOVERLAY_dispd_stage2_sm_ctrl_reg, PPOVERLAY_dispd_stage2_sm_ctrl_dispd_stage2_smooth_toggle_protect_mask);

	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	display_timing_ctrl2_reg.lineod_en = bEnable;
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);

	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	/*  set stage2 smooth toggle protect */
	IoReg_SetBits(PPOVERLAY_dispd_smooth_toggle2_reg, PPOVERLAY_dispd_smooth_toggle2_dispd_stage2_smooth_toggle_apply_mask);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
}

void drvif_color_set_pcid2_enable(unsigned char bEnable)
{
	// put the block at the back position
	unsigned long flags;//for spin_lock_irqsave
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;

	if (drvif_color_get_HW_MEX_STATUS(MEX_LINE_OD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_LINE_OD != 1, EN2\n");
		return;
	}

	/*  set stage2 smooth toggle protect */
	IoReg_SetBits(PPOVERLAY_dispd_stage2_sm_ctrl_reg, PPOVERLAY_dispd_stage2_sm_ctrl_dispd_stage2_smooth_toggle_protect_mask);

	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	display_timing_ctrl2_reg.lineod2_en = bEnable;
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);

	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	/*  set stage2 smooth toggle protect */
	IoReg_SetBits(PPOVERLAY_dispd_smooth_toggle2_reg, PPOVERLAY_dispd_smooth_toggle2_dispd_stage2_smooth_toggle_apply_mask);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
}

void drvif_color_set_LGD_POD_enable(unsigned char blgdpod)
{
	unsigned long flags;//for spin_lock_irqsave
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;

	if (drvif_color_get_HW_MEX_STATUS(MEX_POD) != 1) {
		rtd_pr_vpq_info("HW_MEX,  MEX_POD != 1, EN\n");
		return;
	}

	if(blgdpod)
		IoReg_SetBits(LG_POD_POD_enable_reg,LG_POD_POD_enable_pod_on_mask);
	else
		IoReg_ClearBits(LG_POD_POD_enable_reg,LG_POD_POD_enable_pod_on_mask);

	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	display_timing_ctrl2_reg.pod_enable= blgdpod;
	if (blgdpod) {
		display_timing_ctrl2_reg.lineod_en = 0;
		display_timing_ctrl2_reg.lineod2_en = 0;
	}
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);

	down(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
	spin_lock_irqsave(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//lock double buffer spinlock
	/* for K6, no using original apply, use stage2 smooth toggle apply */
	IoReg_SetBits(PPOVERLAY_dispd_smooth_toggle2_reg, PPOVERLAY_dispd_smooth_toggle2_dispd_stage2_smooth_toggle_apply_mask);
	spin_unlock_irqrestore(get_DBreg_spinlock(SLR_MAIN_DISPLAY), flags);//unlock double buffer spinlock
	up(get_forcebg_semaphore());/*semaphore must added for not effect mute function when setting b8028300*/
}

void drvif_color_get_pcid_regional_table_valuetable(unsigned int *pRetTbl, unsigned int RgnSel, DRV_pcid_channel_t Channel)
{
	unsigned int i=0, iter=0;
	unsigned char TableSize = 17;
	pcid_pcid_lut_addr_RBUS pcid_lut_addr_reg;
	pcid_pcid_lut_data_RBUS pcid_pcid_lut_data_reg;
	pcid_pcid_regionaltable_ctrl_RBUS pcid_pcid_regionaltable_ctrl_reg;

	// Reset the internal counter
	pcid_pcid_regionaltable_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_RegionalTable_Ctrl_reg);
	pcid_pcid_regionaltable_ctrl_reg.regional_table_en = 0;
	IoReg_Write32(PCID_PCID_RegionalTable_Ctrl_reg, pcid_pcid_regionaltable_ctrl_reg.regValue);

	pcid_lut_addr_reg.regValue = IoReg_Read32(PCID_PCID_LUT_ADDR_reg);
	pcid_lut_addr_reg.pcid_regional_lut_ax_en = 0;
	IoReg_Write32(PCID_PCID_LUT_ADDR_reg, pcid_lut_addr_reg.regValue);

	pcid_pcid_regionaltable_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_RegionalTable_Ctrl_reg);
	pcid_pcid_regionaltable_ctrl_reg.regional_table_en = 1;
	IoReg_Write32(PCID_PCID_RegionalTable_Ctrl_reg, pcid_pcid_regionaltable_ctrl_reg.regValue);

	// Decide the read count
	TableSize = drvif_color_get_pcid_TableSize();
	iter = (drvif_color_get_pcid_RegionType()==_PCID_RGN_4x4)? TableSize*TableSize*2 : TableSize*TableSize;

	// Setup table access setting
	pcid_lut_addr_reg.regValue = IoReg_Read32(PCID_PCID_LUT_ADDR_reg);
	pcid_lut_addr_reg.pcid_lut_sel = Channel;
	pcid_lut_addr_reg.pcid_regional_lut_sel = RgnSel;
	pcid_lut_addr_reg.pcid_lut_data_sel= 0;
	pcid_lut_addr_reg.pcid_regional_lut_ax_en = 1;

	IoReg_Write32(PCID_PCID_LUT_ADDR_reg, pcid_lut_addr_reg.regValue);

	// Read the value from
	for(i=0; i<iter; i++)
	{
		pcid_pcid_lut_data_reg.regValue = IoReg_Read32(PCID_PCID_LUT_DATA_reg);
		pRetTbl[i] = pcid_pcid_lut_data_reg.pcid_lut_data;
		VIPprintf("[VPQ][POD]gRgnTable[%d]=0x%08x\n", i, pRetTbl[i]);
	}

	// Disable the table access
	pcid_lut_addr_reg.regValue = IoReg_Read32(PCID_PCID_LUT_ADDR_reg);
	pcid_lut_addr_reg.pcid_regional_lut_ax_en = 0;
	IoReg_Write32(PCID_PCID_LUT_ADDR_reg, pcid_lut_addr_reg.regValue);

}

void drvif_color_get_pcid_regional_weight_table_valuetable(unsigned int *pRetTbl)
{
	unsigned int i=0;
	unsigned int TableSize = 8*24;
	unsigned int backup_8x24_en =0;
	pcid_pcid_lut_addr_RBUS pcid_lut_addr_reg;
	pcid_pcid_lut_data_RBUS pcid_pcid_lut_data_reg;
	pcid_pcid_regionaltable_ctrl_RBUS pcid_pcid_regionaltable_ctrl_reg;

	// Reset the internal counter
	pcid_pcid_regionaltable_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_RegionalTable_Ctrl_reg);
	backup_8x24_en = pcid_pcid_regionaltable_ctrl_reg.pcid_regional_8x24_en;
	pcid_pcid_regionaltable_ctrl_reg.pcid_regional_8x24_en= 0;
	IoReg_Write32(PCID_PCID_RegionalTable_Ctrl_reg, pcid_pcid_regionaltable_ctrl_reg.regValue);

	pcid_lut_addr_reg.regValue = IoReg_Read32(PCID_PCID_LUT_ADDR_reg);
	pcid_lut_addr_reg.pcid_regional_weight_ax_en= 0;
	IoReg_Write32(PCID_PCID_LUT_ADDR_reg, pcid_lut_addr_reg.regValue);

	pcid_pcid_regionaltable_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_RegionalTable_Ctrl_reg);
	pcid_pcid_regionaltable_ctrl_reg.pcid_regional_8x24_en = 1;
	IoReg_Write32(PCID_PCID_RegionalTable_Ctrl_reg, pcid_pcid_regionaltable_ctrl_reg.regValue);

	// Setup table access setting
	pcid_lut_addr_reg.regValue = IoReg_Read32(PCID_PCID_LUT_ADDR_reg);
	pcid_lut_addr_reg.pcid_lut_data_sel= 2;
	pcid_lut_addr_reg.pcid_regional_weight_ax_en = 1;
	IoReg_Write32(PCID_PCID_LUT_ADDR_reg, pcid_lut_addr_reg.regValue);

	// Read the value from
	for(i=0; i<TableSize; i++)
	{
		pcid_pcid_lut_data_reg.regValue = IoReg_Read32(PCID_PCID_LUT_DATA_reg);
		pRetTbl[i] = pcid_pcid_lut_data_reg.pcid_lut_data;
		VIPprintf("[VPQ][POD]gRgnWtTable[%d]=0x%08x\n", i, pRetTbl[i]);
	}

	// Disable the table access
	pcid_lut_addr_reg.regValue = IoReg_Read32(PCID_PCID_LUT_ADDR_reg);
	pcid_lut_addr_reg.pcid_regional_weight_ax_en = 0;
	IoReg_Write32(PCID_PCID_LUT_ADDR_reg, pcid_lut_addr_reg.regValue);

	pcid_pcid_regionaltable_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_RegionalTable_Ctrl_reg);
	pcid_pcid_regionaltable_ctrl_reg.pcid_regional_8x24_en = backup_8x24_en;
	IoReg_Write32(PCID_PCID_RegionalTable_Ctrl_reg, pcid_pcid_regionaltable_ctrl_reg.regValue);
}


char drvif_color_get_pcid_enable(void)
{
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	return display_timing_ctrl2_reg.lineod_en;
}

char drvif_color_get_pcid2_enable(void)
{
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	return display_timing_ctrl2_reg.lineod2_en;
}

unsigned char drvif_color_get_pcid_TableSize(void)
{
	pcid_pcid_regionaltable_ctrl_RBUS pcid_pcid_regionaltable_ctrl_reg;
	pcid_pcid_regionaltable_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_RegionalTable_Ctrl_reg);
	return (pcid_pcid_regionaltable_ctrl_reg.pcid_regional_19x19tab_en==0)? 17 : 19;
}

DRV_pcid_RgnType_t drvif_color_get_pcid_RegionType(void)
{
	pcid_pcid_regionaltable_ctrl_RBUS pcid_pcid_regionaltable_ctrl_reg;
	pcid_pcid_regionaltable_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_RegionalTable_Ctrl_reg);
	return (pcid_pcid_regionaltable_ctrl_reg.pcid_regional_10x1_en==0)? _PCID_RGN_4x4 : _PCID_RGN_1x10;
}

void drvif_color_set_pcid_data_access_read(unsigned short x, unsigned short y, unsigned char loc, unsigned char cross_bar_en)
{

	pcid_pcid_accessdata_ctrl_RBUS pcid_pcid_accessdata_ctrl_reg;
	pcid_pcid_accessdata_posctrl_RBUS pcid_pcid_accessdata_posctrl_reg;

	pcid_pcid_accessdata_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_AccessData_CTRL_reg);
	pcid_pcid_accessdata_posctrl_reg.regValue = IoReg_Read32(PCID_PCID_AccessData_PosCtrl_reg);

	pcid_pcid_accessdata_posctrl_reg.access_startx = x;
	pcid_pcid_accessdata_posctrl_reg.access_starty = y;

	pcid_pcid_accessdata_ctrl_reg.access_cross_bar_en = cross_bar_en;
	pcid_pcid_accessdata_ctrl_reg.access_location_sel = loc;	// after PCID
	pcid_pcid_accessdata_ctrl_reg.access_read_en = 1;
	pcid_pcid_accessdata_ctrl_reg.access_write_en = 1;

	IoReg_Write32(PCID_PCID_AccessData_CTRL_reg, pcid_pcid_accessdata_ctrl_reg.regValue);
	IoReg_Write32(PCID_PCID_AccessData_PosCtrl_reg, pcid_pcid_accessdata_posctrl_reg.regValue);

}

char drvif_color_get_pcid_data_access_read(unsigned short *R, unsigned short *G, unsigned short *B)
{
	pcid_pcid_accessdata_ctrl_RBUS pcid_pcid_accessdata_ctrl_reg;
	pcid_pcid_accessdata_r_RBUS	pcid_pcid_accessdata_r_reg;
	pcid_pcid_accessdata_g_RBUS pcid_pcid_accessdata_g_reg;
	pcid_pcid_accessdata_b_RBUS pcid_pcid_accessdata_b_reg;
	char ret_val = 0;
	int timeout = 20;

	if (R == NULL || G == NULL || B == NULL) {
		rtd_pr_vpq_emerg("drvif_color_get_pcid_data_access_read, NULL\n");
		ret_val = -1;
		return -1;
	}

	pcid_pcid_accessdata_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_AccessData_CTRL_reg);

	while ((timeout>=0) && (pcid_pcid_accessdata_ctrl_reg.access_read_en == 1)) {
		//ROSTimeDly(ROS_TICKS_PER_SEC / 1000 * 5);
		usleep_range(5000, 10000);
		timeout--;
		pcid_pcid_accessdata_ctrl_reg.regValue = IoReg_Read32(PCID_PCID_AccessData_CTRL_reg);
	}

	if (timeout <= 0) {
		rtd_pr_vpq_emerg("drvif_color_get_pcid_data_access_read, wait time out = %d,\n",timeout);
		ret_val = -1;
	} else {
		pcid_pcid_accessdata_r_reg.regValue = IoReg_Read32(PCID_PCID_AccessData_R_reg);
		pcid_pcid_accessdata_g_reg.regValue = IoReg_Read32(PCID_PCID_AccessData_G_reg);
		pcid_pcid_accessdata_b_reg.regValue = IoReg_Read32(PCID_PCID_AccessData_B_reg);

		*R = pcid_pcid_accessdata_r_reg.access_read_r;
		*G = pcid_pcid_accessdata_g_reg.access_read_g;
		*B = pcid_pcid_accessdata_b_reg.access_read_b;
		ret_val = 0;
	}

	return ret_val;

}

unsigned int drvif_color_get_pcid_data_access_read_R_mask(void)
{
	return PCID_PCID_AccessData_R_access_read_r_mask;
}

 //PCLRC(Primary Color Luminance Ratio Compensation
void drvif_color_set_siw_PCLRC_valuetable(unsigned char *pTblValue)
{


       // pclrcData.positionAfterLineOD = 1; // Signal Path 0 : PCLRC -> LOD, 1 : LOD -> PCLRC
       // pclrcData.data = pclrcData;
       // pclrcData.datasize =  PCLRC_DATA_SIZE; // 4352 
       // pclrcData.enable = 1;
        siw_pclrc_pclrc_db_ctrl_RBUS siw_pclrc_pclrc_db_ctrl_reg;
        siw_pclrc_pclrc_ctrl_RBUS siw_pclrc_pclrc_ctrl_reg;
        siw_pclrc_pclrc_lut_ctrl_RBUS siw_pclrc_pclrc_lut_ctrl_reg;
        //siw_pclrc_pclrc_lut_data_RBUS siw_pclrc_pclrc_lut_data_reg;
	unsigned int i=0;//
        unsigned int tmp_32=0;
        /* Access PCLCR table notice */ 
        //STEP 1 pclrc colck : 0xb8000120 bit25 need to enable 
        //STEP 2 :
        //PCLRC share SRAM with POD, A10, VALC. There are 3 situations:
        //1. PCLRC: PCLRC enable, all other functions disable
        //2. POD, A10: POD and A10 could be enable at the same time, PCLRC and VALC disable.
        //3. VALC: VALC enable, all other functions disable
        //STEP3 0xb8028004 , bit 25 or 26 need to enable. 
        //STEP4 pclrc have 3 kind of lut table size , 1st :(4096+4096 8bit+4bit) ,2nd(4096 bit),3rd(256 bit)
        
	if(!pTblValue)
	{
		VIPprintf("[Error][%s][%s] Table Ptr is null!\n", __FILE__, __func__);
		return;
	}

	// Disable PCLRC 
	siw_pclrc_pclrc_ctrl_reg.regValue = IoReg_Read32(SIW_PCLRC_PCLRC_Ctrl_reg);
        siw_pclrc_pclrc_ctrl_reg.pclrc_bypass_en =0  ; //0: output = PCLRC IP output , 1: output = PCLRC input
	IoReg_Write32(SIW_PCLRC_PCLRC_Ctrl_reg, siw_pclrc_pclrc_ctrl_reg.regValue);


	// db apply  
	siw_pclrc_pclrc_db_ctrl_reg.regValue = IoReg_Read32(SIW_PCLRC_PCLRC_db_Ctrl_reg);
        siw_pclrc_pclrc_db_ctrl_reg.db_en =1;
        siw_pclrc_pclrc_db_ctrl_reg.db_apply=1;
        IoReg_Write32(SIW_PCLRC_PCLRC_db_Ctrl_reg, siw_pclrc_pclrc_db_ctrl_reg.regValue);


        //write table ctrl bit
        siw_pclrc_pclrc_lut_ctrl_reg.regValue = IoReg_Read32(SIW_PCLRC_PCLRC_LUT_Ctrl_reg);
        siw_pclrc_pclrc_lut_ctrl_reg.lut_ax_en=1;
        siw_pclrc_pclrc_lut_ctrl_reg.lut_ax_mode =1; //0x0 LUT access mode 0: 1 byte mode 1: 4 byte mode
        siw_pclrc_pclrc_lut_ctrl_reg.lut_ax_wren =1; //0: Read , 1: Write
        IoReg_Write32(SIW_PCLRC_PCLRC_LUT_Ctrl_reg, siw_pclrc_pclrc_lut_ctrl_reg.regValue);

        //write table 

	for(i=0;i<PANEL_PCLRC_DATA_SIZE/4;i++)
	{

	        tmp_32 = (pTblValue[i*4+3]<<24)|(pTblValue[i*4+2]<<16)|(pTblValue[i*4+1]<<8)|pTblValue[i*4];
            
                
		IoReg_Write32(SIW_PCLRC_PCLRC_LUT_Data_reg,tmp_32) ;
                //if(i<20) //for debug
		//rtd_pr_vpq_emerg("pTblValue[%d] =%d,  \n",i,tmp_32);
	}
        
        siw_pclrc_pclrc_lut_ctrl_reg.regValue = IoReg_Read32(SIW_PCLRC_PCLRC_LUT_Ctrl_reg);
        siw_pclrc_pclrc_lut_ctrl_reg.lut_ax_en=0;
        IoReg_Write32(SIW_PCLRC_PCLRC_LUT_Ctrl_reg, siw_pclrc_pclrc_lut_ctrl_reg.regValue);


}
 
void drvif_color_get_siw_PCLRC_valuetable(unsigned char *pTblValue)
{

        siw_pclrc_pclrc_db_ctrl_RBUS siw_pclrc_pclrc_db_ctrl_reg;
        siw_pclrc_pclrc_ctrl_RBUS siw_pclrc_pclrc_ctrl_reg;
        siw_pclrc_pclrc_lut_ctrl_RBUS siw_pclrc_pclrc_lut_ctrl_reg;
        //siw_pclrc_pclrc_lut_data_RBUS siw_pclrc_pclrc_lut_data_reg;
	unsigned int i=0,tmp_32=0;//, iter=0;
        
        /* Access PCLCR table notice */ 
        //STEP 1 pclrc colck : 0xb8000120 bit25 need to enable 
        //STEP 2 :
        //PCLRC share SRAM with POD, A10, VALC. There are 3 situations:
        //1. PCLRC: PCLRC enable, all other functions disable
        //2. POD, A10: POD and A10 could be enable at the same time, PCLRC and VALC disable.
        //3. VALC: VALC enable, all other functions disable
        //STEP3 0xb8028004 , bit 25 or 26 need to enable. 
        //STEP4 pclrc have 3 kind of lut table size , 1st :(4096+4096 8bit+4bit) ,2nd(4096 bit),3rd(256 bit)

	if(!pTblValue)
	{
		VIPprintf("[Error][%s][%s] Table Ptr is null!\n", __FILE__, __func__);
		return;
	}

	// Disable PCLRC 
	siw_pclrc_pclrc_ctrl_reg.regValue = IoReg_Read32(SIW_PCLRC_PCLRC_Ctrl_reg);
        siw_pclrc_pclrc_ctrl_reg.pclrc_bypass_en =0  ; //0: output = PCLRC IP output , 1: output = PCLRC input
	IoReg_Write32(SIW_PCLRC_PCLRC_Ctrl_reg, siw_pclrc_pclrc_ctrl_reg.regValue);


	// db apply  
	siw_pclrc_pclrc_db_ctrl_reg.regValue = IoReg_Read32(SIW_PCLRC_PCLRC_db_Ctrl_reg);
        siw_pclrc_pclrc_db_ctrl_reg.db_en =1;
        siw_pclrc_pclrc_db_ctrl_reg.db_apply=1;
        IoReg_Write32(SIW_PCLRC_PCLRC_db_Ctrl_reg, siw_pclrc_pclrc_db_ctrl_reg.regValue);


        //write table ctrl bit
        siw_pclrc_pclrc_lut_ctrl_reg.regValue = IoReg_Read32(SIW_PCLRC_PCLRC_LUT_Ctrl_reg);
        siw_pclrc_pclrc_lut_ctrl_reg.lut_ax_en=1;
        siw_pclrc_pclrc_lut_ctrl_reg.lut_ax_mode =1; //0x0 LUT access mode 0: 1 byte mode 1: 4 byte mode
        siw_pclrc_pclrc_lut_ctrl_reg.lut_ax_wren =0; //0: Read , 1: Write
        IoReg_Write32(SIW_PCLRC_PCLRC_LUT_Ctrl_reg, siw_pclrc_pclrc_lut_ctrl_reg.regValue);

        //write table 

	for(i=0;i<PANEL_PCLRC_DATA_SIZE/4;i++)
	{
                tmp_32 =IoReg_Read32(SIW_PCLRC_PCLRC_LUT_Data_reg);
                pTblValue[i*4]=tmp_32&0xff;
                pTblValue[i*4+1]=(tmp_32&0xff00)>>8;
                pTblValue[i*4+2]=(tmp_32&0xff0000)>>16;
                pTblValue[i*4+3]=(tmp_32&0xff000000)>>24;

                //if(i<20)//for debug
		//rtd_pr_vpq_emerg("pTblValue[%d] =%d, g \n",i,tmp_32);
	}
        
        siw_pclrc_pclrc_lut_ctrl_reg.regValue = IoReg_Read32(SIW_PCLRC_PCLRC_LUT_Ctrl_reg);
        siw_pclrc_pclrc_lut_ctrl_reg.lut_ax_en=0;
        IoReg_Write32(SIW_PCLRC_PCLRC_LUT_Ctrl_reg, siw_pclrc_pclrc_lut_ctrl_reg.regValue);


}

void drvif_color_enable_PCLRC_CLK(unsigned char bEnable)
{

	sys_reg_sys_srst4_RBUS sys_reg_sys_srst4_reg;
	sys_reg_sys_srst4_reg.regValue = IoReg_Read32(SYS_REG_SYS_SRST4_reg);


        sys_reg_sys_srst4_reg.rstn_siliconworks_pclrc =1;
        sys_reg_sys_srst4_reg.write_data=1;
        IoReg_Write32(SYS_REG_SYS_SRST4_reg,sys_reg_sys_srst4_reg.regValue);
}
void drvif_color_PCLRC_Register_Setting(void)
{
        siw_pclrc_pclrc_db_ctrl_RBUS siw_pclrc_pclrc_db_ctrl_reg;

        siw_pclrc_pclrc_reg00_03_RBUS siw_pclrc_pclrc_reg00_03_reg;
        siw_pclrc_pclrc_reg04_07_RBUS siw_pclrc_pclrc_reg04_07_reg;
        siw_pclrc_pclrc_reg08_0b_RBUS siw_pclrc_pclrc_reg08_0b_reg;
        siw_pclrc_pclrc_reg0c_0f_RBUS siw_pclrc_pclrc_reg0c_0f_reg;
        siw_pclrc_pclrc_reg10_13_RBUS siw_pclrc_pclrc_reg10_13_reg;
        siw_pclrc_pclrc_reg14_15_RBUS siw_pclrc_pclrc_reg14_15_reg;

        siw_pclrc_pclrc_reg00_03_reg.regValue = IoReg_Read32(SIW_PCLRC_PCLRC_reg00_03_reg);
        siw_pclrc_pclrc_reg04_07_reg.regValue = IoReg_Read32(SIW_PCLRC_PCLRC_reg04_07_reg);
        siw_pclrc_pclrc_reg08_0b_reg.regValue = IoReg_Read32(SIW_PCLRC_PCLRC_reg08_0B_reg);
        siw_pclrc_pclrc_reg0c_0f_reg.regValue = IoReg_Read32(SIW_PCLRC_PCLRC_reg0C_0F_reg);
        siw_pclrc_pclrc_reg10_13_reg.regValue = IoReg_Read32(SIW_PCLRC_PCLRC_reg10_13_reg);
        siw_pclrc_pclrc_reg14_15_reg.regValue = IoReg_Read32(SIW_PCLRC_PCLRC_reg14_15_reg);

        siw_pclrc_pclrc_reg00_03_reg.i_reg00 = 0x0;
        siw_pclrc_pclrc_reg00_03_reg.i_reg01 = 0x1;
        siw_pclrc_pclrc_reg00_03_reg.i_reg02 = 0x5;
        siw_pclrc_pclrc_reg00_03_reg.i_reg03 = 0x12;

        siw_pclrc_pclrc_reg04_07_reg.i_reg04 = 0x7;
        siw_pclrc_pclrc_reg04_07_reg.i_reg05 = 0x11;
        siw_pclrc_pclrc_reg04_07_reg.i_reg06 = 0x6;
        siw_pclrc_pclrc_reg04_07_reg.i_reg07 = 0x13;

        siw_pclrc_pclrc_reg08_0b_reg.i_reg08 = 0x9;
        siw_pclrc_pclrc_reg08_0b_reg.i_reg09 = 0x12;
        siw_pclrc_pclrc_reg08_0b_reg.i_reg0a = 0xb;
        siw_pclrc_pclrc_reg08_0b_reg.i_reg0b = 0x11;

        siw_pclrc_pclrc_reg0c_0f_reg.i_reg0c = 0xa;
        siw_pclrc_pclrc_reg0c_0f_reg.i_reg0d = 0x13;
        siw_pclrc_pclrc_reg0c_0f_reg.i_reg0e = 0x14;
        siw_pclrc_pclrc_reg0c_0f_reg.i_reg0f = 0x15;

        siw_pclrc_pclrc_reg10_13_reg.i_reg10 = 0x15;
        siw_pclrc_pclrc_reg10_13_reg.i_reg11 = 0x15;
        siw_pclrc_pclrc_reg10_13_reg.i_reg12 = 0x15;
        siw_pclrc_pclrc_reg10_13_reg.i_reg13 = 0x15;

        siw_pclrc_pclrc_reg14_15_reg.i_reg14 = 0x15;
        siw_pclrc_pclrc_reg14_15_reg.i_reg15 = 0x11;

        IoReg_Write32(SIW_PCLRC_PCLRC_reg00_03_reg,siw_pclrc_pclrc_reg00_03_reg.regValue);
        IoReg_Write32(SIW_PCLRC_PCLRC_reg04_07_reg,siw_pclrc_pclrc_reg04_07_reg.regValue);
        IoReg_Write32(SIW_PCLRC_PCLRC_reg08_0B_reg,siw_pclrc_pclrc_reg08_0b_reg.regValue);
        IoReg_Write32(SIW_PCLRC_PCLRC_reg0C_0F_reg,siw_pclrc_pclrc_reg0c_0f_reg.regValue);
        IoReg_Write32(SIW_PCLRC_PCLRC_reg10_13_reg,siw_pclrc_pclrc_reg10_13_reg.regValue);
        IoReg_Write32(SIW_PCLRC_PCLRC_reg14_15_reg,siw_pclrc_pclrc_reg14_15_reg.regValue);

        // db apply  
        siw_pclrc_pclrc_db_ctrl_reg.regValue = IoReg_Read32(SIW_PCLRC_PCLRC_db_Ctrl_reg);
        siw_pclrc_pclrc_db_ctrl_reg.db_en =1;
        siw_pclrc_pclrc_db_ctrl_reg.db_apply=1;
        IoReg_Write32(SIW_PCLRC_PCLRC_db_Ctrl_reg, siw_pclrc_pclrc_db_ctrl_reg.regValue);
}
void drvif_color_set_enable_PCLRC_Funciton(unsigned char bEnable,unsigned char position)
{
        unsigned long flags;//for spin_lock_irqsave
        int timeoutcnt = 100;
        ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
        //add double buffer enable-------------------------------------------------------
        ppoverlay_double_buffer_ctrl_RBUS ppoverlay_double_buffer_ctrl_Reg;

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
        //PCLRC share SRAM with POD, A10, VALC. There are 3 situations:
        //1. PCLRC: PCLRC enable, all other functions disable
        //2. POD, A10: POD and A10 could be enable at the same time, PCLRC and VALC disable.
        //3. VALC: VALC enable, all other functions disable

        if(bEnable ==TRUE){
                if(position==_PCLRC_PCLRC_LOD){
                        display_timing_ctrl2_reg.pclrc_en=1;
                        display_timing_ctrl2_reg.pclrc2_en=0;
                }
                else{
                        display_timing_ctrl2_reg.pclrc_en=0;
                        display_timing_ctrl2_reg.pclrc2_en=1;
                }
                display_timing_ctrl2_reg.a10_en =0;
                display_timing_ctrl2_reg.valc_en =0;
        }else{

                display_timing_ctrl2_reg.pclrc_en=0;
                display_timing_ctrl2_reg.pclrc2_en=0;

        }

        IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg,display_timing_ctrl2_reg.regValue);
        
       /* if (drvif_color_get_HW_MEX_STATUS(MEX_POD) != 1) {
        rtd_pr_vpq_info("HW_MEX,  MEX_POD != 1, LGPOD_en=0\n");
        LGPOD_en = 0;
        }
        if (drvif_color_get_HW_MEX_STATUS(MEX_HCIC) != 1) {
        rtd_pr_vpq_info("HW_MEX,  MEX_HCIC != 1, fcic_en=0\n");
        fcic_en = 0;
        }
        if (drvif_color_get_HW_MEX_STATUS(MEX_LINE_OD) != 1) {
        rtd_pr_vpq_info("HW_MEX,  MEX_LINE_OD != 1, pcid1_en=pcid2_en=0\n");
        pcid1_en = 0;
        pcid_2_en = 0;
        }

        display_timing_ctrl2_reg.lineod_en = pcid1_en;
        display_timing_ctrl2_reg.lineod2_en = pcid_2_en;
        display_timing_ctrl2_reg.pod_enable = LGPOD_en;
        display_timing_ctrl2_reg.hcic_enable = fcic_en;

        if (fcic_en) {
        display_timing_ctrl2_reg.valc_en = 0;
        rtd_pr_vpq_info("drvif_color_set_fcic_enable, need to disable valc_en\n");
        }
        IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, display_timing_ctrl2_reg.regValue);
        */

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
        rtd_pr_vpq_emerg("drvif_color_set_fcic_enable, timeout!\n");



}
void drvif_color_get_enable_PCLRC_Funciton(unsigned char *bEnable,unsigned char *position)
{
        ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;

        display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
        //PCLRC share SRAM with POD, A10, VALC. There are 3 situations:
        //1. PCLRC: PCLRC enable, all other functions disable
        //2. POD, A10: POD and A10 could be enable at the same time, PCLRC and VALC disable.
        //3. VALC: VALC enable, all other functions disable

        if(display_timing_ctrl2_reg.pclrc_en ==1){

                *bEnable=1;
                *position=_PCLRC_PCLRC_LOD;
        }
        else if (display_timing_ctrl2_reg.pclrc2_en==1){
                *bEnable =1;
                *position =_PCLRC_LOD_PCLRC;
        }
        else{
                *bEnable =0;
                *position =_PCLRC_PCLRC_LOD;

        }
        
        //rtd_pr_vpq_emerg("rord bEnable=%d, position=%d\n",*bEnable,*position);




}

void drvif_color_set_PCLRC_enable(unsigned char bEnable)
{
	siw_pclrc_pclrc_db_ctrl_RBUS siw_pclrc_pclrc_db_ctrl_reg;
	siw_pclrc_pclrc_ctrl_RBUS siw_pclrc_pclrc_ctrl_reg;
	//set PCLRC enable
	siw_pclrc_pclrc_ctrl_reg.regValue = IoReg_Read32(SIW_PCLRC_PCLRC_Ctrl_reg);
	siw_pclrc_pclrc_ctrl_reg.pclrc_bypass_en = bEnable;
	IoReg_Write32(SIW_PCLRC_PCLRC_Ctrl_reg, siw_pclrc_pclrc_ctrl_reg.regValue);


	// db apply  
	siw_pclrc_pclrc_db_ctrl_reg.regValue = IoReg_Read32(SIW_PCLRC_PCLRC_db_Ctrl_reg);
    siw_pclrc_pclrc_db_ctrl_reg.db_en =1;
    siw_pclrc_pclrc_db_ctrl_reg.db_apply=1;
    IoReg_Write32(SIW_PCLRC_PCLRC_db_Ctrl_reg, siw_pclrc_pclrc_db_ctrl_reg.regValue);
}

