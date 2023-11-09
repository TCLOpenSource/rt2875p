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
#include <linux/delay.h>

#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/vip/pqmask.h>
#include <tvscalercontrol/vip/vip_reg_def.h>

/*=========================================================*/
/*================= DOUBLE BUFFER CONTROL =================*/
/*=========================================================*/

#if 0
/*DOUBLE BUFFER CONTROL*/
#endif
void drvif_module_set_pqmask_domain_db_enable(unsigned char bEnable, unsigned char DomainItem)
{
	switch(DomainItem){
		case PQMASK_DOMAIN_I:
			if( bEnable != 0 )
				IoReg_SetBits(PQMASK_I_PQMASK_I_Double_BUF_reg, PQMASK_I_PQMASK_I_Double_BUF_db_en_mask);
			else
				IoReg_ClearBits(PQMASK_I_PQMASK_I_Double_BUF_reg, PQMASK_I_PQMASK_I_Double_BUF_db_en_mask);
			break;
		case PQMASK_DOMAIN_D:
			if( bEnable != 0 )
				IoReg_SetBits(PQMASK_D_PQMASK_Double_BUF_reg, PQMASK_D_PQMASK_Double_BUF_db_en_mask);
			else
				IoReg_ClearBits(PQMASK_D_PQMASK_Double_BUF_reg, PQMASK_D_PQMASK_Double_BUF_db_en_mask);
			break;
		case PQMASK_DOMAIN_SRNN:
			if( bEnable != 0 )
				IoReg_SetBits(PQMASK_SRNN_PQMASK_SRNN_Double_BUF_reg, PQMASK_SRNN_PQMASK_SRNN_Double_BUF_db_en_mask);
			else
				IoReg_ClearBits(PQMASK_SRNN_PQMASK_SRNN_Double_BUF_reg, PQMASK_SRNN_PQMASK_SRNN_Double_BUF_db_en_mask);
			break;
		default:
			rtd_pr_vpq_emerg("[%s] DomainItem %d overflow\n", __FUNCTION__, DomainItem);
			break;
	}
}

void drvif_module_set_pqmask_domain_db_apply(unsigned char DomainItem)
{
	switch(DomainItem){
		case PQMASK_DOMAIN_I:
			IoReg_SetBits(PQMASK_I_PQMASK_I_Double_BUF_reg, PQMASK_I_PQMASK_I_Double_BUF_db_apply_mask);
			break;
		case PQMASK_DOMAIN_D:
			IoReg_SetBits(PQMASK_D_PQMASK_Double_BUF_reg, PQMASK_D_PQMASK_Double_BUF_db_apply_mask);
			break;
		case PQMASK_DOMAIN_SRNN:
			IoReg_SetBits(PQMASK_SRNN_PQMASK_SRNN_Double_BUF_reg, PQMASK_SRNN_PQMASK_SRNN_Double_BUF_db_apply_mask);
			break;
		default:
			rtd_pr_vpq_emerg("[%s] DomainItem %d overflow\n", __FUNCTION__, DomainItem);
			break;
	}

}

int drvif_module_wait_pqmask_domain_db_apply_done(unsigned int WaitCnt, unsigned char DomainItem)
{
	unsigned int PQMASK_Domain_Double_BUF_reg;
	unsigned int PQMASK_Domain_Double_BUF_db_apply_mask;
	unsigned char PQmask_db_apply_flag;
	int Counter = WaitCnt;
	int Ret = 0;

	// init the related reg
	switch(DomainItem){
		case PQMASK_DOMAIN_I:
			PQMASK_Domain_Double_BUF_reg = PQMASK_I_PQMASK_I_Double_BUF_reg;
			PQMASK_Domain_Double_BUF_db_apply_mask = PQMASK_I_PQMASK_I_Double_BUF_db_apply_mask;
			break;
		case PQMASK_DOMAIN_D:
			PQMASK_Domain_Double_BUF_reg = PQMASK_D_PQMASK_Double_BUF_reg;
			PQMASK_Domain_Double_BUF_db_apply_mask = PQMASK_D_PQMASK_Double_BUF_db_apply_mask;
			break;
		case PQMASK_DOMAIN_SRNN:
			PQMASK_Domain_Double_BUF_reg = PQMASK_SRNN_PQMASK_SRNN_Double_BUF_reg;
			PQMASK_Domain_Double_BUF_db_apply_mask = PQMASK_SRNN_PQMASK_SRNN_Double_BUF_db_apply_mask;
			break;
		default:
			rtd_pr_vpq_emerg("[%s] DomainItem %d overflow\n", __FUNCTION__, DomainItem);
			// break;
			return -1;
	}

	do {
		usleep_range(500, 1000);
		Counter--;
		PQmask_db_apply_flag = (IoReg_Read32(PQMASK_Domain_Double_BUF_reg) & PQMASK_Domain_Double_BUF_db_apply_mask) >> 1;
	}
	while( (Counter>0) && (PQmask_db_apply_flag!=0) );

	if( Counter <= 0 )
		Ret = -1;

	return Ret;
}

/*=========================================================*/
/*===================== SET FUNCTIONS =====================*/
/*=========================================================*/

#if 0
/* DOMAIN SET FUNCTIONS*/
#endif

void drvif_color_set_pqmask_domain_DMA_enable(unsigned char bEnable,  unsigned char DomainItem)
{
	switch(DomainItem){
		case PQMASK_DOMAIN_I:
			if( bEnable != 0 )
				IoReg_SetBits(PQMASK_I_PQMASK_I_DMA_RD_Ctrl_reg, PQMASK_I_PQMASK_I_DMA_RD_Ctrl_dma_enable_mask);
			else
				IoReg_ClearBits(PQMASK_I_PQMASK_I_DMA_RD_Ctrl_reg, PQMASK_I_PQMASK_I_DMA_RD_Ctrl_dma_enable_mask);
			break;
		case PQMASK_DOMAIN_D:
			if( bEnable != 0 )
				IoReg_SetBits(PQMASK_D_PQMASK_D_DMA_RD_Ctrl_reg, PQMASK_D_PQMASK_D_DMA_RD_Ctrl_dma_enable_mask);
			else
				IoReg_ClearBits(PQMASK_D_PQMASK_D_DMA_RD_Ctrl_reg, PQMASK_D_PQMASK_D_DMA_RD_Ctrl_dma_enable_mask);
			break;
		case PQMASK_DOMAIN_SRNN:
			if( bEnable != 0 )
				IoReg_SetBits(PQMASK_SRNN_PQMASK_SRNN_DMA_RD_Ctrl_reg, PQMASK_SRNN_PQMASK_SRNN_DMA_RD_Ctrl_dma_enable_mask);
			else
				IoReg_ClearBits(PQMASK_SRNN_PQMASK_SRNN_DMA_RD_Ctrl_reg, PQMASK_SRNN_PQMASK_SRNN_DMA_RD_Ctrl_dma_enable_mask);
			break;
		default:
			rtd_pr_vpq_emerg("[%s] DomainItem %d overflow\n", __FUNCTION__, DomainItem);
			break;
	}
}


void drvif_color_set_pqmask_domain_DMA_config(PQMASK_DOMAIN_DMA_CONFIG_T *pDmaConfig, unsigned char DomainItem)
{
	// since in I/D/Srnn domainm the following structure are same, we use I domain structure as representation
	pqmask_i_pqmask_i_dma_rd_rule_check_up_RBUS pqmask_dma_rd_rule_check_up_reg;
	pqmask_i_pqmask_i_dma_rd_rule_check_low_RBUS pqmask_dma_rd_rule_check_low_reg;
	pqmask_i_pqmask_i_dma_rd_ctrl_RBUS pqmask_dma_rd_ctrl_reg;
	unsigned int PQMASK_DMA_RD_Ctrl_reg, PQMASK_DMA_RD_Rule_check_up_reg, PQMASK_DMA_RD_Rule_check_low_reg;

	// init each domain reg address
	switch(DomainItem){
		case PQMASK_DOMAIN_I:
			PQMASK_DMA_RD_Ctrl_reg = PQMASK_I_PQMASK_I_DMA_RD_Ctrl_reg;
			PQMASK_DMA_RD_Rule_check_up_reg = PQMASK_I_PQMASK_I_DMA_RD_Rule_check_up_reg;
			PQMASK_DMA_RD_Rule_check_low_reg = PQMASK_I_PQMASK_I_DMA_RD_Rule_check_low_reg;
			break;
		case PQMASK_DOMAIN_D:
			PQMASK_DMA_RD_Ctrl_reg = PQMASK_D_PQMASK_D_DMA_RD_Ctrl_reg;
			PQMASK_DMA_RD_Rule_check_up_reg = PQMASK_D_PQMASK_D_DMA_RD_Rule_check_up_reg;
			PQMASK_DMA_RD_Rule_check_low_reg = PQMASK_D_PQMASK_D_DMA_RD_Rule_check_low_reg;
			break;
		case PQMASK_DOMAIN_SRNN:
			PQMASK_DMA_RD_Ctrl_reg = PQMASK_SRNN_PQMASK_SRNN_DMA_RD_Ctrl_reg;
			PQMASK_DMA_RD_Rule_check_up_reg = PQMASK_SRNN_PQMASK_SRNN_DMA_RD_Rule_check_up_reg;
			PQMASK_DMA_RD_Rule_check_low_reg = PQMASK_SRNN_PQMASK_SRNN_DMA_RD_Rule_check_low_reg;
			break;
		default:
			rtd_pr_vpq_emerg("[%s] DomainItem %d overflow\n", __FUNCTION__, DomainItem);
			//break;
			return;
	}
	// main code
	pqmask_dma_rd_ctrl_reg.regValue = IoReg_Read32(PQMASK_DMA_RD_Ctrl_reg);
	pqmask_dma_rd_ctrl_reg.dma_1byte_swap =  (pDmaConfig->SwapType)&0x1;
	pqmask_dma_rd_ctrl_reg.dma_2byte_swap = ((pDmaConfig->SwapType)&0x2) >> 1;
	pqmask_dma_rd_ctrl_reg.dma_4byte_swap = ((pDmaConfig->SwapType)&0x4) >> 2;
	pqmask_dma_rd_ctrl_reg.dma_8byte_swap = ((pDmaConfig->SwapType)&0x8) >> 3;
	IoReg_Write32(PQMASK_DMA_RD_Ctrl_reg, pqmask_dma_rd_ctrl_reg.regValue);

	pqmask_dma_rd_rule_check_up_reg.regValue = IoReg_Read32(PQMASK_DMA_RD_Rule_check_up_reg);
	pqmask_dma_rd_rule_check_up_reg.dma_up_limit = pDmaConfig->AddrUpLimit;
	IoReg_Write32(PQMASK_DMA_RD_Rule_check_up_reg, pqmask_dma_rd_rule_check_up_reg.regValue);

	pqmask_dma_rd_rule_check_low_reg.regValue = IoReg_Read32(PQMASK_DMA_RD_Rule_check_low_reg);
	pqmask_dma_rd_rule_check_low_reg.dma_low_limit = pDmaConfig->AddrLowLimit;
	IoReg_Write32(PQMASK_DMA_RD_Rule_check_low_reg, pqmask_dma_rd_rule_check_low_reg.regValue);
}
#if 0
/* BLOCK SET FUNCTIONS*/
#endif
void drvif_color_set_pqmask_block_DMA_config(PQMASK_BLOCK_DMA_CONFIG_T *pDmaConfig, unsigned char BlockItem)
{
	// since in PSNR/Shp/ICM/LC/Srnn_In/Srnn_Out domain the following structure are same, we use I domain structure as representation
	pqmask_i_pqmask_i_dma_line_step_RBUS pqmask_i_pqmask_i_dma_line_step_reg;
	// however burst lenght in Idomain/Ddomain/Srnndomain is different with each other
	// for PSNR block we use PSNR structure as representation
	pqmask_i_pqmask_i_dma_burst_RBUS ppqmask_i_pqmask_dma_burst_reg;
	// for Shp/ICM/LC block we use Shp structure as representation
	pqmask_d_pqmask_dma_en_shp_RBUS  pqmask_d_pqmask_dma_burst_reg;
	// for Srnn_In/Srnn_Out block we use srnn_i structure as representation
	pqmask_srnn_pqmask_dma_en_srnn_i_RBUS ppqmask_srnn_pqmask_dma_burst_reg;
	unsigned int PQMASK_BlockModule_DMA_Line_Step_reg, PQMASK_BlockModule_DMA_Burst_reg;
	// Init the reg
	switch(BlockItem){
		case PQMASK_BLKMOD_SPNR:
			PQMASK_BlockModule_DMA_Line_Step_reg = PQMASK_I_PQMASK_I_DMA_Line_Step_reg;
			PQMASK_BlockModule_DMA_Burst_reg     = PQMASK_I_PQMASK_I_DMA_Burst_reg;
			break;
		case PQMASK_BLKMOD_SHARPNESS:
			PQMASK_BlockModule_DMA_Line_Step_reg = PQMASK_D_PQMASK_SHP_DMA_Line_Step_reg;
			PQMASK_BlockModule_DMA_Burst_reg     = PQMASK_D_PQMASK_DMA_en_shp_reg;
			break;
		case PQMASK_BLKMOD_ICM:
			PQMASK_BlockModule_DMA_Line_Step_reg = PQMASK_D_PQMASK_ICM_DMA_Line_Step_reg;
			PQMASK_BlockModule_DMA_Burst_reg     = PQMASK_D_PQMASK_DMA_en_icm_reg;
			break;
		case PQMASK_BLKMOD_LOCALCONTRAST:
			PQMASK_BlockModule_DMA_Line_Step_reg = PQMASK_D_PQMASK_LC_DMA_Line_Step_reg;
			PQMASK_BlockModule_DMA_Burst_reg     = PQMASK_D_PQMASK_DMA_en_lc_reg;
			break;
		case PQMASK_BLKMOD_SRNN_IN:
			PQMASK_BlockModule_DMA_Line_Step_reg = PQMASK_SRNN_PQMASK_SRNN_I_DMA_Line_Step_reg;
			PQMASK_BlockModule_DMA_Burst_reg     = PQMASK_SRNN_PQMASK_DMA_en_srnn_I_reg;
			break;
		case PQMASK_BLKMOD_SRNN_OUT:
			PQMASK_BlockModule_DMA_Line_Step_reg = PQMASK_SRNN_PQMASK_SRNN_O_DMA_Line_Step_reg;
			PQMASK_BlockModule_DMA_Burst_reg     = PQMASK_SRNN_PQMASK_DMA_en_srnn_o_reg;
			break;
		default:
			rtd_pr_vpq_emerg("[%s] BlockItem %d overflow\n", __FUNCTION__, BlockItem);
			//break;
			return;
	}
	// main code
	pqmask_i_pqmask_i_dma_line_step_reg.regValue = IoReg_Read32(PQMASK_BlockModule_DMA_Line_Step_reg);
	pqmask_i_pqmask_i_dma_line_step_reg.dma_line_step = pDmaConfig->LineStep;
	IoReg_Write32(PQMASK_BlockModule_DMA_Line_Step_reg, pqmask_i_pqmask_i_dma_line_step_reg.regValue);

	if (BlockItem == PQMASK_BLKMOD_SPNR){
		ppqmask_i_pqmask_dma_burst_reg.regValue = IoReg_Read32(PQMASK_BlockModule_DMA_Burst_reg);
		ppqmask_i_pqmask_dma_burst_reg.data_burst_length = pDmaConfig->DataBurstLen;
		IoReg_Write32(PQMASK_BlockModule_DMA_Burst_reg, ppqmask_i_pqmask_dma_burst_reg.regValue);
	}
	else if(BlockItem == PQMASK_BLKMOD_SHARPNESS || BlockItem == PQMASK_BLKMOD_ICM || BlockItem == PQMASK_BLKMOD_LOCALCONTRAST){
		pqmask_d_pqmask_dma_burst_reg.regValue = IoReg_Read32(PQMASK_BlockModule_DMA_Burst_reg);
		pqmask_d_pqmask_dma_burst_reg.data_burst_length_shp = pDmaConfig->DataBurstLen;
		IoReg_Write32(PQMASK_BlockModule_DMA_Burst_reg, pqmask_d_pqmask_dma_burst_reg.regValue);
	}
	else if (BlockItem == PQMASK_BLKMOD_SRNN_IN || BlockItem == PQMASK_BLKMOD_SRNN_OUT){
		ppqmask_srnn_pqmask_dma_burst_reg.regValue = IoReg_Read32(PQMASK_BlockModule_DMA_Burst_reg);
		ppqmask_srnn_pqmask_dma_burst_reg.data_burst_length_srnn_i = pDmaConfig->DataBurstLen;
		IoReg_Write32(PQMASK_BlockModule_DMA_Burst_reg, ppqmask_srnn_pqmask_dma_burst_reg.regValue);
	}
}

void drvif_color_set_pqmask_block_scaling_enable(unsigned char bHorEnable, unsigned char bVerEnable, unsigned char BlockItem)
{
	// since in PSNR/Shp/ICM/LC/Srnn_In/Srnn_Out domain the following structure are same, we use I domain structure as representation
	pqmask_i_pqmask_i_scaleup_hor_RBUS pqmask_i_pqmask_i_scaleup_hor_reg;
	pqmask_i_pqmask_i_scaleup_ver_RBUS pqmask_i_pqmask_i_scaleup_ver_reg;
	unsigned int PQMASK_BlockModule_Scaleup_Hor_reg, PQMASK_BlockModule_Scaleup_Ver_reg;
	// Init the reg
	switch(BlockItem){
		case PQMASK_BLKMOD_SPNR:
			PQMASK_BlockModule_Scaleup_Hor_reg = PQMASK_I_PQMASK_I_Scaleup_Hor_reg;
			PQMASK_BlockModule_Scaleup_Ver_reg = PQMASK_I_PQMASK_I_Scaleup_Ver_reg;
			break;
		case PQMASK_BLKMOD_SHARPNESS:
			PQMASK_BlockModule_Scaleup_Hor_reg = PQMASK_D_PQMASK_SHP_Scaleup_Hor_reg;
			PQMASK_BlockModule_Scaleup_Ver_reg = PQMASK_D_PQMASK_SHP_Scaleup_Ver_reg;
			break;
		case PQMASK_BLKMOD_ICM:
			PQMASK_BlockModule_Scaleup_Hor_reg = PQMASK_D_PQMASK_ICM_Scaleup_Hor_reg;
			PQMASK_BlockModule_Scaleup_Ver_reg = PQMASK_D_PQMASK_ICM_Scaleup_Ver_reg;
			break;
		case PQMASK_BLKMOD_LOCALCONTRAST:
			PQMASK_BlockModule_Scaleup_Hor_reg = PQMASK_D_PQMASK_LC_Scaleup_Hor_reg;
			PQMASK_BlockModule_Scaleup_Ver_reg = PQMASK_D_PQMASK_LC_Scaleup_Ver_reg;
			break;
		case PQMASK_BLKMOD_SRNN_IN:
			PQMASK_BlockModule_Scaleup_Hor_reg = PQMASK_SRNN_PQMASK_SRNN_I_Scaleup_Hor_reg;
			PQMASK_BlockModule_Scaleup_Ver_reg = PQMASK_SRNN_PQMASK_SRNN_I_Scaleup_Ver_reg;
			break;
		case PQMASK_BLKMOD_SRNN_OUT:
			PQMASK_BlockModule_Scaleup_Hor_reg = PQMASK_SRNN_PQMASK_SRNN_O_Scaleup_Hor_reg;
			PQMASK_BlockModule_Scaleup_Ver_reg = PQMASK_SRNN_PQMASK_SRNN_O_Scaleup_Ver_reg;
			break;
		default:
			rtd_pr_vpq_emerg("[%s] BlockItem %d overflow\n", __FUNCTION__, BlockItem);
			//break;
			return;
	}
	// main code
	pqmask_i_pqmask_i_scaleup_hor_reg.regValue = IoReg_Read32(PQMASK_BlockModule_Scaleup_Hor_reg);
	pqmask_i_pqmask_i_scaleup_hor_reg.h_zoom_en = (bHorEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_BlockModule_Scaleup_Hor_reg, pqmask_i_pqmask_i_scaleup_hor_reg.regValue);

	pqmask_i_pqmask_i_scaleup_ver_reg.regValue = IoReg_Read32(PQMASK_BlockModule_Scaleup_Ver_reg);
	pqmask_i_pqmask_i_scaleup_ver_reg.v_zoom_en = (bVerEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_BlockModule_Scaleup_Ver_reg, pqmask_i_pqmask_i_scaleup_ver_reg.regValue);
}

void drvif_color_set_pqmask_block_scaling(PQMASK_SACLING_T *ptr, unsigned char BlockItem)
{
	// since in PSNR/Shp/ICM/LC/Srnn_In/Srnn_Out domain the following structure are same, we use I domain structure as representation
	pqmask_i_pqmask_i_scaleup_hor_RBUS pqmask_i_pqmask_i_scaleup_hor_reg;
	pqmask_i_pqmask_i_scaleup_ver_RBUS pqmask_i_pqmask_i_scaleup_ver_reg;
	pqmask_i_pqmask_i_scaleup_phase_RBUS pqmask_i_pqmask_i_scaleup_phase_reg;
	unsigned int PQMASK_BlockModule_Scaleup_Hor_reg, PQMASK_BlockModule_Scaleup_Ver_reg, PQMASK_BlockModule_Scaleup_Phase_reg;
	// Init the reg
	switch(BlockItem){
		case PQMASK_BLKMOD_SPNR:
			PQMASK_BlockModule_Scaleup_Hor_reg = PQMASK_I_PQMASK_I_Scaleup_Hor_reg;
			PQMASK_BlockModule_Scaleup_Ver_reg = PQMASK_I_PQMASK_I_Scaleup_Ver_reg;
			PQMASK_BlockModule_Scaleup_Phase_reg = PQMASK_I_PQMASK_I_Scaleup_Phase_reg;
			break;
		case PQMASK_BLKMOD_SHARPNESS:
			PQMASK_BlockModule_Scaleup_Hor_reg = PQMASK_D_PQMASK_SHP_Scaleup_Hor_reg;
			PQMASK_BlockModule_Scaleup_Ver_reg = PQMASK_D_PQMASK_SHP_Scaleup_Ver_reg;
			PQMASK_BlockModule_Scaleup_Phase_reg = PQMASK_D_PQMASK_SHP_Scaleup_Phase_reg;
			break;
		case PQMASK_BLKMOD_ICM:
			PQMASK_BlockModule_Scaleup_Hor_reg = PQMASK_D_PQMASK_ICM_Scaleup_Hor_reg;
			PQMASK_BlockModule_Scaleup_Ver_reg = PQMASK_D_PQMASK_ICM_Scaleup_Ver_reg;
			PQMASK_BlockModule_Scaleup_Phase_reg = PQMASK_D_PQMASK_ICM_Scaleup_Phase_reg;
			break;
		case PQMASK_BLKMOD_LOCALCONTRAST:
			PQMASK_BlockModule_Scaleup_Hor_reg = PQMASK_D_PQMASK_LC_Scaleup_Hor_reg;
			PQMASK_BlockModule_Scaleup_Ver_reg = PQMASK_D_PQMASK_LC_Scaleup_Ver_reg;
			PQMASK_BlockModule_Scaleup_Phase_reg = PQMASK_D_PQMASK_LC_Scaleup_Phase_reg;
			break;
		case PQMASK_BLKMOD_SRNN_IN:
			PQMASK_BlockModule_Scaleup_Hor_reg = PQMASK_SRNN_PQMASK_SRNN_I_Scaleup_Hor_reg;
			PQMASK_BlockModule_Scaleup_Ver_reg = PQMASK_SRNN_PQMASK_SRNN_I_Scaleup_Ver_reg;
			PQMASK_BlockModule_Scaleup_Phase_reg = PQMASK_SRNN_PQMASK_SRNN_I_Scaleup_Phase_reg;
			break;
		case PQMASK_BLKMOD_SRNN_OUT:
			PQMASK_BlockModule_Scaleup_Hor_reg = PQMASK_SRNN_PQMASK_SRNN_O_Scaleup_Hor_reg;
			PQMASK_BlockModule_Scaleup_Ver_reg = PQMASK_SRNN_PQMASK_SRNN_O_Scaleup_Ver_reg;
			PQMASK_BlockModule_Scaleup_Phase_reg = PQMASK_SRNN_PQMASK_SRNN_O_Scaleup_Phase_reg;
			break;
		default:
			rtd_pr_vpq_emerg("[%s] BlockItem %d overflow\n", __FUNCTION__, BlockItem);
			//break;
			return;
	}
	// main code
	pqmask_i_pqmask_i_scaleup_hor_reg.regValue = IoReg_Read32(PQMASK_BlockModule_Scaleup_Hor_reg);
	pqmask_i_pqmask_i_scaleup_hor_reg.hor_factor = ptr->Hor.Factor;
	IoReg_Write32(PQMASK_BlockModule_Scaleup_Hor_reg, pqmask_i_pqmask_i_scaleup_hor_reg.regValue);

	pqmask_i_pqmask_i_scaleup_ver_reg.regValue = IoReg_Read32(PQMASK_BlockModule_Scaleup_Ver_reg);
	pqmask_i_pqmask_i_scaleup_ver_reg.ver_factor = ptr->Ver.Factor;
	IoReg_Write32(PQMASK_BlockModule_Scaleup_Ver_reg, pqmask_i_pqmask_i_scaleup_ver_reg.regValue);

	pqmask_i_pqmask_i_scaleup_phase_reg.regValue = IoReg_Read32(PQMASK_BlockModule_Scaleup_Phase_reg);
	pqmask_i_pqmask_i_scaleup_phase_reg.hor_ini = ptr->Hor.InitPhase;
	pqmask_i_pqmask_i_scaleup_phase_reg.ver_ini = ptr->Ver.InitPhase;
	IoReg_Write32(PQMASK_BlockModule_Scaleup_Phase_reg, pqmask_i_pqmask_i_scaleup_phase_reg.regValue);
}


void drvif_color_set_pqmask_block_size(PQMASK_IO_SIZE_T *ptr, unsigned char BlockItem)
{
	// since in PSNR/Shp/ICM/LC/Srnn_In/Srnn_Out domain the following structure are same, we use I domain structure as representation
	pqmask_i_pqmask_i_intputsize_ctrl_RBUS pqmask_i_pqmask_i_intputsize_ctrl_reg;
	pqmask_i_pqmask_i_intputsize_ctrl2_RBUS pqmask_i_pqmask_i_intputsize_ctrl2_reg;
	pqmask_i_pqmask_i_outputsize_ctrl_RBUS pqmask_i_pqmask_i_outputsize_ctrl_reg;
	pqmask_i_pqmask_i_outputsize_ctrl2_RBUS pqmask_i_pqmask_i_outputsize_ctrl2_reg;
	unsigned int PQMASK_BlockModule_IntputSize_Ctrl_reg, PQMASK_BlockModule_IntputSize_Ctrl2_reg;
	unsigned int PQMASK_BlockModule_OutputSize_Ctrl_reg, PQMASK_BlockModule_OutputSize_Ctrl2_reg;

	// Init the reg
	switch(BlockItem){
		case PQMASK_BLKMOD_SPNR:
			PQMASK_BlockModule_IntputSize_Ctrl_reg = PQMASK_I_PQMASK_I_IntputSize_Ctrl_reg;
			PQMASK_BlockModule_IntputSize_Ctrl2_reg = PQMASK_I_PQMASK_I_IntputSize_Ctrl2_reg;
			PQMASK_BlockModule_OutputSize_Ctrl_reg = PQMASK_I_PQMASK_I_OutputSize_Ctrl_reg;
			PQMASK_BlockModule_OutputSize_Ctrl2_reg = PQMASK_I_PQMASK_I_OutputSize_Ctrl2_reg;
			break;
		case PQMASK_BLKMOD_SHARPNESS:
			PQMASK_BlockModule_IntputSize_Ctrl_reg = PQMASK_D_PQMASK_SHP_IntputSize_Ctrl_reg;
			PQMASK_BlockModule_IntputSize_Ctrl2_reg = PQMASK_D_PQMASK_SHP_IntputSize_Ctrl2_reg;
			PQMASK_BlockModule_OutputSize_Ctrl_reg = PQMASK_D_PQMASK_SHP_OutputSize_Ctrl_reg;
			PQMASK_BlockModule_OutputSize_Ctrl2_reg = PQMASK_D_PQMASK_SHP_OutputSize_Ctrl2_reg;
			break;
		case PQMASK_BLKMOD_ICM:
			PQMASK_BlockModule_IntputSize_Ctrl_reg = PQMASK_D_PQMASK_ICM_IntputSize_Ctrl_reg;
			PQMASK_BlockModule_IntputSize_Ctrl2_reg = PQMASK_D_PQMASK_ICM_IntputSize_Ctrl2_reg;
			PQMASK_BlockModule_OutputSize_Ctrl_reg = PQMASK_D_PQMASK_ICM_OutputSize_Ctrl_reg;
			PQMASK_BlockModule_OutputSize_Ctrl2_reg = PQMASK_D_PQMASK_ICM_OutputSize_Ctrl2_reg;
			break;
		case PQMASK_BLKMOD_LOCALCONTRAST:
			PQMASK_BlockModule_IntputSize_Ctrl_reg = PQMASK_D_PQMASK_LC_IntputSize_Ctrl_reg;
			PQMASK_BlockModule_IntputSize_Ctrl2_reg = PQMASK_D_PQMASK_LC_IntputSize_Ctrl2_reg;
			PQMASK_BlockModule_OutputSize_Ctrl_reg = PQMASK_D_PQMASK_LC_OutputSize_Ctrl_reg;
			PQMASK_BlockModule_OutputSize_Ctrl2_reg = PQMASK_D_PQMASK_LC_OutputSize_Ctrl2_reg;
			break;
		case PQMASK_BLKMOD_SRNN_IN:
			PQMASK_BlockModule_IntputSize_Ctrl_reg = PQMASK_SRNN_PQMASK_SRNN_I_IntputSize_Ctrl_reg;
			PQMASK_BlockModule_IntputSize_Ctrl2_reg = PQMASK_SRNN_PQMASK_SRNN_I_IntputSize_Ctrl2_reg;
			PQMASK_BlockModule_OutputSize_Ctrl_reg = PQMASK_SRNN_PQMASK_SRNN_I_OutputSize_Ctrl_reg;
			PQMASK_BlockModule_OutputSize_Ctrl2_reg = PQMASK_SRNN_PQMASK_SRNN_I_OutputSize_Ctrl2_reg;
			break;
		case PQMASK_BLKMOD_SRNN_OUT:
			PQMASK_BlockModule_IntputSize_Ctrl_reg = PQMASK_SRNN_PQMASK_SRNN_O_IntputSize_Ctrl_reg;
			PQMASK_BlockModule_IntputSize_Ctrl2_reg = PQMASK_SRNN_PQMASK_SRNN_O_IntputSize_Ctrl2_reg;
			PQMASK_BlockModule_OutputSize_Ctrl_reg = PQMASK_SRNN_PQMASK_SRNN_O_OutputSize_Ctrl_reg;
			PQMASK_BlockModule_OutputSize_Ctrl2_reg = PQMASK_SRNN_PQMASK_SRNN_O_OutputSize_Ctrl2_reg;
			break;
		default:
			rtd_pr_vpq_emerg("[%s] BlockItem %d overflow\n", __FUNCTION__, BlockItem);
			//break;
			return;
	}
	// main code
	pqmask_i_pqmask_i_intputsize_ctrl_reg.regValue = IoReg_Read32(PQMASK_BlockModule_IntputSize_Ctrl_reg);
	pqmask_i_pqmask_i_intputsize_ctrl_reg.in_h_sta = ptr->Input.HorSta;
	pqmask_i_pqmask_i_intputsize_ctrl_reg.in_width = ptr->Input.Width;
	IoReg_Write32(PQMASK_BlockModule_IntputSize_Ctrl_reg, pqmask_i_pqmask_i_intputsize_ctrl_reg.regValue);

	pqmask_i_pqmask_i_intputsize_ctrl2_reg.regValue = IoReg_Read32(PQMASK_BlockModule_IntputSize_Ctrl2_reg);
	pqmask_i_pqmask_i_intputsize_ctrl2_reg.in_v_sta = ptr->Input.VerSta;
	pqmask_i_pqmask_i_intputsize_ctrl2_reg.in_height = ptr->Input.Height;
	IoReg_Write32(PQMASK_BlockModule_IntputSize_Ctrl2_reg, pqmask_i_pqmask_i_intputsize_ctrl2_reg.regValue);

	pqmask_i_pqmask_i_outputsize_ctrl_reg.regValue = IoReg_Read32(PQMASK_BlockModule_OutputSize_Ctrl_reg);
	pqmask_i_pqmask_i_outputsize_ctrl_reg.out_h_sta = ptr->Output.HorSta;
	pqmask_i_pqmask_i_outputsize_ctrl_reg.out_width = ptr->Output.Width;
	IoReg_Write32(PQMASK_BlockModule_OutputSize_Ctrl_reg, pqmask_i_pqmask_i_outputsize_ctrl_reg.regValue);

	pqmask_i_pqmask_i_outputsize_ctrl2_reg.regValue = IoReg_Read32(PQMASK_BlockModule_OutputSize_Ctrl2_reg);
	pqmask_i_pqmask_i_outputsize_ctrl2_reg.out_v_sta = ptr->Output.VerSta;
	pqmask_i_pqmask_i_outputsize_ctrl2_reg.out_height = ptr->Output.Height;
	IoReg_Write32(PQMASK_BlockModule_OutputSize_Ctrl2_reg, pqmask_i_pqmask_i_outputsize_ctrl2_reg.regValue);
}

void drvif_color_set_pqmask_block_dm_window_enable(unsigned char bEnable, unsigned char BlockItem)
{
	switch(BlockItem){
		case PQMASK_BLKMOD_SPNR:
			if( bEnable != 0 )
				IoReg_SetBits(PQMASK_I_PQMASK_I_Window_sta_reg, PQMASK_I_PQMASK_I_Window_sta_window_en_mask);
			else
				IoReg_ClearBits(PQMASK_I_PQMASK_I_Window_sta_reg, PQMASK_I_PQMASK_I_Window_sta_window_en_mask);
			break;
		case PQMASK_BLKMOD_SHARPNESS:
			if( bEnable != 0 )
				IoReg_SetBits(PQMASK_D_PQMASK_SHP_Window_STA_reg, PQMASK_D_PQMASK_SHP_Window_STA_shp_window_en_mask);
			else
				IoReg_ClearBits(PQMASK_D_PQMASK_SHP_Window_STA_reg, PQMASK_D_PQMASK_SHP_Window_STA_shp_window_en_mask);
			break;
		case PQMASK_BLKMOD_ICM:
			if( bEnable != 0 )
				IoReg_SetBits(PQMASK_D_PQMASK_ICM_Window_STA_reg, PQMASK_D_PQMASK_ICM_Window_STA_icm_window_en_mask);
			else
				IoReg_ClearBits(PQMASK_D_PQMASK_ICM_Window_STA_reg, PQMASK_D_PQMASK_ICM_Window_STA_icm_window_en_mask);
			break;
		case PQMASK_BLKMOD_LOCALCONTRAST:
			if( bEnable != 0 )
				IoReg_SetBits(PQMASK_D_PQMASK_LC_Window_STA_reg, PQMASK_D_PQMASK_LC_Window_STA_lc_window_en_mask);
			else
				IoReg_ClearBits(PQMASK_D_PQMASK_LC_Window_STA_reg, PQMASK_D_PQMASK_LC_Window_STA_lc_window_en_mask);
			break;
		case PQMASK_BLKMOD_SRNN_IN:
			if( bEnable != 0 )
				IoReg_SetBits(PQMASK_SRNN_PQMASK_SRNN_I_Window_STA_reg, PQMASK_SRNN_PQMASK_SRNN_I_Window_STA_srnn_i_window_en_mask);
			else
				IoReg_ClearBits(PQMASK_SRNN_PQMASK_SRNN_I_Window_STA_reg, PQMASK_SRNN_PQMASK_SRNN_I_Window_STA_srnn_i_window_en_mask);
			break;
		case PQMASK_BLKMOD_SRNN_OUT:
			if( bEnable != 0 )
				IoReg_SetBits(PQMASK_SRNN_PQMASK_SRNN_O_Window_STA_reg, PQMASK_SRNN_PQMASK_SRNN_O_Window_STA_srnn_o_window_en_mask);
			else
				IoReg_ClearBits(PQMASK_SRNN_PQMASK_SRNN_O_Window_STA_reg, PQMASK_SRNN_PQMASK_SRNN_O_Window_STA_srnn_o_window_en_mask);
			break;
		default:
			rtd_pr_vpq_emerg("[%s] BlockItem %d overflow\n", __FUNCTION__, BlockItem);
			break;
	}
}

void drvif_color_set_pqmask_block_dm_window(PQMASK_WINDOW_T *ptr, unsigned char BlockItem)
{
	// since in PSNR/Shp/ICM/LC/Srnn_In/Srnn_Out domain the following structure are same, we use I domain structure as representation
	pqmask_i_pqmask_i_window_sta_RBUS pqmask_i_pqmask_i_window_sta_reg;
	pqmask_i_pqmask_i_window_end_RBUS pqmask_i_pqmask_i_window_end_reg;
	unsigned int PQMASK_BlockModule_Window_Sta_reg, PQMASK_BlockModule_Window_End_reg;
	// the demo weight structure are not same
	pqmask_i_pqmask_i_winow_weight_RBUS   pqmask_i_pqmask_i_window_weight_reg;
	pqmask_d_pqmask_shp_winow_weight_RBUS pqmask_d_pqmask_shp_window_weight_reg;
	pqmask_d_pqmask_icm_winow_weight_RBUS pqmask_d_pqmask_icm_window_weight_reg;
	pqmask_d_pqmask_lc_winow_weight_RBUS  pqmask_d_pqmask_lc_window_weight_reg;
	pqmask_srnn_pqmask_srnn_i_winow_weight_RBUS pqmask_srnn_pqmask_srnn_i_window_weight_reg;
	pqmask_srnn_pqmask_srnn_o_winow_weight_RBUS pqmask_srnn_pqmask_srnn_o_window_weight_reg;

	// Init the reg
	switch(BlockItem){
		case PQMASK_BLKMOD_SPNR:
			PQMASK_BlockModule_Window_Sta_reg = PQMASK_I_PQMASK_I_Window_sta_reg;
			PQMASK_BlockModule_Window_End_reg = PQMASK_I_PQMASK_I_Window_end_reg;
			pqmask_i_pqmask_i_window_weight_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Winow_weight_reg);
			pqmask_i_pqmask_i_window_weight_reg.weight_dec = ptr->weight.DecontourWeight;
			pqmask_i_pqmask_i_window_weight_reg.weight_snr = ptr->weight.SNRWeight;
			IoReg_Write32(PQMASK_I_PQMASK_I_Winow_weight_reg, pqmask_i_pqmask_i_window_weight_reg.regValue);
			break;
		case PQMASK_BLKMOD_SHARPNESS:
			PQMASK_BlockModule_Window_Sta_reg = PQMASK_D_PQMASK_SHP_Window_STA_reg;
			PQMASK_BlockModule_Window_End_reg = PQMASK_D_PQMASK_SHP_Window_END_reg;
			pqmask_d_pqmask_shp_window_weight_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_Winow_weight_reg);
			pqmask_d_pqmask_shp_window_weight_reg.edge_weight = ptr->weight.EdgeWeight;
			pqmask_d_pqmask_shp_window_weight_reg.texture_weight = ptr->weight.TextureWeight;
			IoReg_Write32(PQMASK_D_PQMASK_SHP_Winow_weight_reg, pqmask_d_pqmask_shp_window_weight_reg.regValue);
			break;
		case PQMASK_BLKMOD_ICM:
			PQMASK_BlockModule_Window_Sta_reg = PQMASK_D_PQMASK_ICM_Window_STA_reg;
			PQMASK_BlockModule_Window_End_reg = PQMASK_D_PQMASK_ICM_Window_END_reg;
			pqmask_d_pqmask_icm_window_weight_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_Winow_weight_reg);
			pqmask_d_pqmask_icm_window_weight_reg.icm_hoffet_weight = ptr->weight.ICM_Weight0;
			pqmask_d_pqmask_icm_window_weight_reg.icm_soffet_weight = ptr->weight.ICM_Weight1;
			pqmask_d_pqmask_icm_window_weight_reg.icm_ioffet_weight = ptr->weight.ICM_Weight2;
			IoReg_Write32(PQMASK_D_PQMASK_ICM_Winow_weight_reg, pqmask_d_pqmask_icm_window_weight_reg.regValue);
			break;
		case PQMASK_BLKMOD_LOCALCONTRAST:
			PQMASK_BlockModule_Window_Sta_reg = PQMASK_D_PQMASK_LC_Window_STA_reg;
			PQMASK_BlockModule_Window_End_reg = PQMASK_D_PQMASK_LC_Window_END_reg;
			pqmask_d_pqmask_lc_window_weight_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_Winow_weight_reg);
			pqmask_d_pqmask_lc_window_weight_reg.lc_weight = ptr->weight.LocalContrastWeight;
			IoReg_Write32(PQMASK_D_PQMASK_LC_Winow_weight_reg, pqmask_d_pqmask_lc_window_weight_reg.regValue);
			break;
		case PQMASK_BLKMOD_SRNN_IN:
			PQMASK_BlockModule_Window_Sta_reg = PQMASK_SRNN_PQMASK_SRNN_I_Window_STA_reg;
			PQMASK_BlockModule_Window_End_reg = PQMASK_SRNN_PQMASK_SRNN_I_Window_STA_reg;
			pqmask_srnn_pqmask_srnn_i_window_weight_reg.regValue = IoReg_Read32(PQMASK_SRNN_PQMASK_SRNN_I_Winow_weight_reg);
			pqmask_srnn_pqmask_srnn_i_window_weight_reg.srnn_i_0_weight = ptr->weight.SRNNIn_Weight0;
			pqmask_srnn_pqmask_srnn_i_window_weight_reg.srnn_i_1_weight = ptr->weight.SRNNIn_Weight1;
			pqmask_srnn_pqmask_srnn_i_window_weight_reg.srnn_i_2_weight = ptr->weight.SRNNIn_Weight2;
			IoReg_Write32(PQMASK_SRNN_PQMASK_SRNN_I_Winow_weight_reg, pqmask_srnn_pqmask_srnn_i_window_weight_reg.regValue);
			break;
		case PQMASK_BLKMOD_SRNN_OUT:
			PQMASK_BlockModule_Window_Sta_reg = PQMASK_SRNN_PQMASK_SRNN_O_Window_STA_reg;
			PQMASK_BlockModule_Window_End_reg = PQMASK_SRNN_PQMASK_SRNN_O_Window_STA_reg;
			pqmask_srnn_pqmask_srnn_o_window_weight_reg.regValue = IoReg_Read32(PQMASK_SRNN_PQMASK_SRNN_O_Winow_weight_reg);
			pqmask_srnn_pqmask_srnn_o_window_weight_reg.o0_weight = ptr->weight.SRNNOut_Weight0;
			IoReg_Write32(PQMASK_SRNN_PQMASK_SRNN_O_Winow_weight_reg, pqmask_srnn_pqmask_srnn_o_window_weight_reg.regValue);
			break;
		default:
			rtd_pr_vpq_emerg("[%s] BlockItem %d overflow\n", __FUNCTION__, BlockItem);
			//break;
			return;
	}
	// main code
	pqmask_i_pqmask_i_window_sta_reg.regValue = IoReg_Read32(PQMASK_BlockModule_Window_Sta_reg);
	pqmask_i_pqmask_i_window_sta_reg.win_x_sta = ptr->HorSta;
	pqmask_i_pqmask_i_window_sta_reg.win_y_sta = ptr->VerSta;
	IoReg_Write32(PQMASK_BlockModule_Window_Sta_reg, pqmask_i_pqmask_i_window_sta_reg.regValue);

	pqmask_i_pqmask_i_window_end_reg.regValue = IoReg_Read32(PQMASK_BlockModule_Window_End_reg);
	pqmask_i_pqmask_i_window_end_reg.win_x_end = ptr->HorEnd;
	pqmask_i_pqmask_i_window_end_reg.win_y_end = ptr->VerSta;
	IoReg_Write32(PQMASK_BlockModule_Window_End_reg, pqmask_i_pqmask_i_window_end_reg.regValue);
}


#if 0
/* MODULE SET FUNCTIONS*/
#endif
void drvif_color_set_pqmask_module_DMA_Addr(unsigned int Addr, unsigned char ModuleItem){
	// since in all modules the following structure are same, we use I domain structure as representation
	pqmask_i_pqmask_i_dma_start_addr_snr_RBUS pqmask_module_dma_start_addr_reg;
	switch(ModuleItem){
		case PQMASK_I_SNR:
			pqmask_module_dma_start_addr_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_Start_Addr_SNR_reg);
			pqmask_module_dma_start_addr_reg.dma_start_addr_snr = Addr;
			IoReg_Write32(PQMASK_I_PQMASK_I_DMA_Start_Addr_SNR_reg, pqmask_module_dma_start_addr_reg.regValue);
			break;
		case PQMASK_I_DECONT:
			pqmask_module_dma_start_addr_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_Start_Addr_DEC_reg);
			pqmask_module_dma_start_addr_reg.dma_start_addr_snr = Addr;
			IoReg_Write32(PQMASK_I_PQMASK_I_DMA_Start_Addr_DEC_reg, pqmask_module_dma_start_addr_reg.regValue);
			break;
		case PQMASK_D_SHP_EDGE:
			pqmask_module_dma_start_addr_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_DMA_Start_Addr_EDGE_reg);
			pqmask_module_dma_start_addr_reg.dma_start_addr_snr = Addr;
			IoReg_Write32(PQMASK_D_PQMASK_SHP_DMA_Start_Addr_EDGE_reg, pqmask_module_dma_start_addr_reg.regValue);
			break;
		case PQMASK_D_SHP_TEXT:
			pqmask_module_dma_start_addr_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_DMA_Start_Addr_TEX_reg);
			pqmask_module_dma_start_addr_reg.dma_start_addr_snr = Addr;
			IoReg_Write32(PQMASK_D_PQMASK_SHP_DMA_Start_Addr_TEX_reg, pqmask_module_dma_start_addr_reg.regValue);
			break;
		case PQMASK_D_ICM_0:
			pqmask_module_dma_start_addr_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_DMA_Start_Addr_H_reg);
			pqmask_module_dma_start_addr_reg.dma_start_addr_snr = Addr;
			IoReg_Write32(PQMASK_D_PQMASK_ICM_DMA_Start_Addr_H_reg, pqmask_module_dma_start_addr_reg.regValue);
			break;
		case PQMASK_D_ICM_1:
			pqmask_module_dma_start_addr_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_DMA_Start_Addr_S_reg);
			pqmask_module_dma_start_addr_reg.dma_start_addr_snr = Addr;
			IoReg_Write32(PQMASK_D_PQMASK_ICM_DMA_Start_Addr_S_reg, pqmask_module_dma_start_addr_reg.regValue);
			break;
		case PQMASK_D_ICM_2:
			pqmask_module_dma_start_addr_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_DMA_Start_Addr_I_reg);
			pqmask_module_dma_start_addr_reg.dma_start_addr_snr = Addr;
			IoReg_Write32(PQMASK_D_PQMASK_ICM_DMA_Start_Addr_I_reg, pqmask_module_dma_start_addr_reg.regValue);
			break;
		case PQMASK_D_LC:
			pqmask_module_dma_start_addr_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_DMA_Start_Addr_reg);
			pqmask_module_dma_start_addr_reg.dma_start_addr_snr = Addr;
			IoReg_Write32(PQMASK_D_PQMASK_LC_DMA_Start_Addr_reg, pqmask_module_dma_start_addr_reg.regValue);
			break;
		case PQMASK_D_DCC: // same with LC
			pqmask_module_dma_start_addr_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_DMA_Start_Addr_reg);
			pqmask_module_dma_start_addr_reg.dma_start_addr_snr = Addr;
			IoReg_Write32(PQMASK_D_PQMASK_LC_DMA_Start_Addr_reg, pqmask_module_dma_start_addr_reg.regValue);
			break;
		case PQMASK_SRNN_IN_0:
			pqmask_module_dma_start_addr_reg.regValue = IoReg_Read32(PQMASK_SRNN_PQMASK_SRNN_I_DMA_Start_Addr_0_reg);
			pqmask_module_dma_start_addr_reg.dma_start_addr_snr = Addr;
			IoReg_Write32(PQMASK_SRNN_PQMASK_SRNN_I_DMA_Start_Addr_0_reg, pqmask_module_dma_start_addr_reg.regValue);
			break;
		case PQMASK_SRNN_IN_1:
			pqmask_module_dma_start_addr_reg.regValue = IoReg_Read32(PQMASK_SRNN_PQMASK_SRNN_I_DMA_Start_Addr_1_reg);
			pqmask_module_dma_start_addr_reg.dma_start_addr_snr = Addr;
			IoReg_Write32(PQMASK_SRNN_PQMASK_SRNN_I_DMA_Start_Addr_1_reg, pqmask_module_dma_start_addr_reg.regValue);
			break;
		case PQMASK_SRNN_IN_2:
			pqmask_module_dma_start_addr_reg.regValue = IoReg_Read32(PQMASK_SRNN_PQMASK_SRNN_I_DMA_Start_Addr_2_reg);
			pqmask_module_dma_start_addr_reg.dma_start_addr_snr = Addr;
			IoReg_Write32(PQMASK_SRNN_PQMASK_SRNN_I_DMA_Start_Addr_2_reg, pqmask_module_dma_start_addr_reg.regValue);
			break;
		case PQMASK_SRNN_OUT_0:
			pqmask_module_dma_start_addr_reg.regValue = IoReg_Read32(PQMASK_SRNN_PQMASK_SRNN_O_DMA_Start_Addr_0_reg);
			pqmask_module_dma_start_addr_reg.dma_start_addr_snr = Addr;
			IoReg_Write32(PQMASK_SRNN_PQMASK_SRNN_O_DMA_Start_Addr_0_reg, pqmask_module_dma_start_addr_reg.regValue);
			break;
		default:
			rtd_pr_vpq_emerg("[%s] Module item: %d overflow\n", __FUNCTION__, ModuleItem);
			break;
	}
}

void drvif_color_set_pqmask_module_enable(unsigned char bEnablem, unsigned char ModuleItem)
{
	switch(ModuleItem){
		case PQMASK_I_SNR:
			if( bEnablem != 0 )
				IoReg_SetBits(PQMASK_I_PQMASK_I_DMA_en_reg, PQMASK_I_PQMASK_I_DMA_en_pqmask_snr_en_mask);
			else
				IoReg_ClearBits(PQMASK_I_PQMASK_I_DMA_en_reg, PQMASK_I_PQMASK_I_DMA_en_pqmask_snr_en_mask);
			break;
		case PQMASK_I_DECONT:
			if( bEnablem != 0 )
				IoReg_SetBits(PQMASK_I_PQMASK_I_DMA_en_reg, PQMASK_I_PQMASK_I_DMA_en_pqmask_dec_en_mask);
			else
				IoReg_ClearBits(PQMASK_I_PQMASK_I_DMA_en_reg, PQMASK_I_PQMASK_I_DMA_en_pqmask_dec_en_mask);
			break;
		case PQMASK_D_SHP_EDGE:
			if( bEnablem != 0 )
				IoReg_SetBits(PQMASK_D_PQMASK_DMA_en_shp_reg, PQMASK_D_PQMASK_DMA_en_shp_shp_edge_pqmask_en_mask);
			else
				IoReg_ClearBits(PQMASK_D_PQMASK_DMA_en_shp_reg, PQMASK_D_PQMASK_DMA_en_shp_shp_edge_pqmask_en_mask);
			break;
		case PQMASK_D_SHP_TEXT:
			if( bEnablem != 0 )
				IoReg_SetBits(PQMASK_D_PQMASK_DMA_en_shp_reg, PQMASK_D_PQMASK_DMA_en_shp_shp_tex_pqmask_en_mask);
			else
				IoReg_ClearBits(PQMASK_D_PQMASK_DMA_en_shp_reg, PQMASK_D_PQMASK_DMA_en_shp_shp_tex_pqmask_en_mask);
			break;
		case PQMASK_D_ICM_0:
			if( bEnablem != 0 )
				IoReg_SetBits(PQMASK_D_PQMASK_DMA_en_icm_reg, PQMASK_D_PQMASK_DMA_en_icm_icm_h_pqmask_en_mask);
			else
				IoReg_ClearBits(PQMASK_D_PQMASK_DMA_en_icm_reg, PQMASK_D_PQMASK_DMA_en_icm_icm_h_pqmask_en_mask);
			break;
		case PQMASK_D_ICM_1:
			if( bEnablem != 0 )
				IoReg_SetBits(PQMASK_D_PQMASK_DMA_en_icm_reg, PQMASK_D_PQMASK_DMA_en_icm_icm_s_pqmask_en_mask);
			else
				IoReg_ClearBits(PQMASK_D_PQMASK_DMA_en_icm_reg, PQMASK_D_PQMASK_DMA_en_icm_icm_s_pqmask_en_mask);
			break;
		case PQMASK_D_ICM_2:
			if( bEnablem != 0 )
				IoReg_SetBits(PQMASK_D_PQMASK_DMA_en_icm_reg, PQMASK_D_PQMASK_DMA_en_icm_icm_i_pqmask_en_mask);
			else
				IoReg_ClearBits(PQMASK_D_PQMASK_DMA_en_icm_reg, PQMASK_D_PQMASK_DMA_en_icm_icm_i_pqmask_en_mask);
			break;
		case PQMASK_D_LC:
			if( bEnablem != 0 )
				IoReg_SetBits(PQMASK_D_PQMASK_DMA_en_lc_reg, PQMASK_D_PQMASK_DMA_en_lc_lc_pqmask_en_mask);
			else
				IoReg_ClearBits(PQMASK_D_PQMASK_DMA_en_lc_reg, PQMASK_D_PQMASK_DMA_en_lc_lc_pqmask_en_mask);
			break;
		case PQMASK_D_DCC:
			if( bEnablem != 0 )
				IoReg_SetBits(PQMASK_D_PQMASK_DMA_en_lc_reg, PQMASK_D_PQMASK_DMA_en_lc_dcc_pqmask_en_mask);
			else
				IoReg_ClearBits(PQMASK_D_PQMASK_DMA_en_lc_reg, PQMASK_D_PQMASK_DMA_en_lc_dcc_pqmask_en_mask);
			break;
		case PQMASK_SRNN_IN_0:
			if( bEnablem != 0 )
				IoReg_SetBits(PQMASK_SRNN_PQMASK_DMA_en_srnn_I_reg, PQMASK_SRNN_PQMASK_DMA_en_srnn_I_srnn_i_0_pqmask_en_mask);
			else
				IoReg_ClearBits(PQMASK_SRNN_PQMASK_DMA_en_srnn_I_reg, PQMASK_SRNN_PQMASK_DMA_en_srnn_I_srnn_i_0_pqmask_en_mask);
			break;
		case PQMASK_SRNN_IN_1:
			if( bEnablem != 0 )
				IoReg_SetBits(PQMASK_SRNN_PQMASK_DMA_en_srnn_I_reg, PQMASK_SRNN_PQMASK_DMA_en_srnn_I_srnn_i_1_pqmask_en_mask);
			else
				IoReg_ClearBits(PQMASK_SRNN_PQMASK_DMA_en_srnn_I_reg, PQMASK_SRNN_PQMASK_DMA_en_srnn_I_srnn_i_1_pqmask_en_mask);
			break;
		case PQMASK_SRNN_IN_2:
			if( bEnablem != 0 )
				IoReg_SetBits(PQMASK_SRNN_PQMASK_DMA_en_srnn_I_reg, PQMASK_SRNN_PQMASK_DMA_en_srnn_I_srnn_i_2_pqmask_en_mask);
			else
				IoReg_ClearBits(PQMASK_SRNN_PQMASK_DMA_en_srnn_I_reg, PQMASK_SRNN_PQMASK_DMA_en_srnn_I_srnn_i_2_pqmask_en_mask);
			break;
		case PQMASK_SRNN_OUT_0:
			if( bEnablem != 0 )
				IoReg_SetBits(PQMASK_SRNN_PQMASK_DMA_en_srnn_o_reg, PQMASK_SRNN_PQMASK_DMA_en_srnn_o_srnn_o_0_pqmask_en_mask);
			else
				IoReg_ClearBits(PQMASK_SRNN_PQMASK_DMA_en_srnn_o_reg, PQMASK_SRNN_PQMASK_DMA_en_srnn_o_srnn_o_0_pqmask_en_mask);
			break;
		default:
			rtd_pr_vpq_emerg("[%s] Module item: %d overflow\n", __FUNCTION__, ModuleItem);
			break;
	}
}





/*=========================================================*/
/*===================== GET FUNCTIONS =====================*/
/*=========================================================*/

#if 0
/*DOMAIN DMA GET FUNCTIONS*/
#endif

unsigned char drvif_color_get_pqmask_domain_DMA_enable(unsigned char DomainItem)
{
	unsigned char bEnable;
	unsigned int u32_RegVal;
	// init each domain reg address
	switch(DomainItem){
		case PQMASK_DOMAIN_I:
			u32_RegVal = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_RD_Ctrl_reg);
			bEnable = PQMASK_I_PQMASK_I_DMA_RD_Ctrl_get_dma_enable(u32_RegVal);
			break;
		case PQMASK_DOMAIN_D:
			u32_RegVal = IoReg_Read32(PQMASK_D_PQMASK_D_DMA_RD_Ctrl_reg);
			bEnable = PQMASK_D_PQMASK_D_DMA_RD_Ctrl_get_dma_enable(u32_RegVal);
			break;
		case PQMASK_DOMAIN_SRNN:
			u32_RegVal = IoReg_Read32(PQMASK_SRNN_PQMASK_SRNN_DMA_RD_Ctrl_reg);
			bEnable = PQMASK_SRNN_PQMASK_SRNN_DMA_RD_Ctrl_get_dma_enable(u32_RegVal);
			break;
		default:
			rtd_pr_vpq_emerg("[%s] DomainItem %d overflow\n", __FUNCTION__, DomainItem);
			bEnable = 0;
			break;
	}
	return bEnable;
}

#if 0
/*BLOCK DMA GET FUNCTIONS*/
#endif



#if 0
/*MODULE DMA GET FUNCTIONS*/
#endif

unsigned char drvif_color_get_pqmask_module_enable(unsigned char ModuleItem)
{
	unsigned char bEnable;
	unsigned int u32_RegVal;
	switch(ModuleItem){
		case PQMASK_I_SNR:
			u32_RegVal = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_en_reg);
			bEnable = PQMASK_I_PQMASK_I_DMA_en_get_pqmask_snr_en(u32_RegVal);
			break;
		case PQMASK_I_DECONT:
			u32_RegVal = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_en_reg);
			bEnable = PQMASK_I_PQMASK_I_DMA_en_get_pqmask_dec_en(u32_RegVal);
			break;
		case PQMASK_D_SHP_EDGE:
			u32_RegVal = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_shp_reg);
			bEnable = PQMASK_D_PQMASK_DMA_en_shp_get_shp_edge_pqmask_en(u32_RegVal);
			break;
		case PQMASK_D_SHP_TEXT:
			u32_RegVal = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_shp_reg);
			bEnable = PQMASK_D_PQMASK_DMA_en_shp_get_shp_tex_pqmask_en(u32_RegVal);
			break;
		case PQMASK_D_ICM_0:
			u32_RegVal = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_icm_reg);
			bEnable = PQMASK_D_PQMASK_DMA_en_icm_get_icm_h_pqmask_en(u32_RegVal);
			break;
		case PQMASK_D_ICM_1:
			u32_RegVal = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_icm_reg);
			bEnable = PQMASK_D_PQMASK_DMA_en_icm_get_icm_s_pqmask_en(u32_RegVal);
			break;
		case PQMASK_D_ICM_2:
			u32_RegVal = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_icm_reg);
			bEnable = PQMASK_D_PQMASK_DMA_en_icm_get_icm_i_pqmask_en(u32_RegVal);
			break;
		case PQMASK_D_LC:
			u32_RegVal = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_lc_reg);
			bEnable = PQMASK_D_PQMASK_DMA_en_lc_get_lc_pqmask_en(u32_RegVal);
			break;
		case PQMASK_D_DCC:
			u32_RegVal = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_lc_reg);
			bEnable = PQMASK_D_PQMASK_DMA_en_lc_get_dcc_pqmask_en(u32_RegVal);
			break;
		case PQMASK_SRNN_IN_0:
			u32_RegVal = IoReg_Read32(PQMASK_SRNN_PQMASK_DMA_en_srnn_I_reg);
			bEnable = PQMASK_SRNN_PQMASK_DMA_en_srnn_I_get_srnn_i_0_pqmask_en(u32_RegVal);
			break;
		case PQMASK_SRNN_IN_1:
			u32_RegVal = IoReg_Read32(PQMASK_SRNN_PQMASK_DMA_en_srnn_I_reg);
			bEnable = PQMASK_SRNN_PQMASK_DMA_en_srnn_I_get_srnn_i_1_pqmask_en(u32_RegVal);
			break;
		case PQMASK_SRNN_IN_2:
			u32_RegVal = IoReg_Read32(PQMASK_SRNN_PQMASK_DMA_en_srnn_I_reg);
			bEnable = PQMASK_SRNN_PQMASK_DMA_en_srnn_I_get_srnn_i_2_pqmask_en(u32_RegVal);
			break;
		case PQMASK_SRNN_OUT_0:
			u32_RegVal = IoReg_Read32(PQMASK_SRNN_PQMASK_DMA_en_srnn_o_reg);
			bEnable = PQMASK_SRNN_PQMASK_DMA_en_srnn_o_get_srnn_o_0_pqmask_en(u32_RegVal);
			break;
		default:
			rtd_pr_vpq_emerg("[%s] Module item: %d overflow\n", __FUNCTION__, ModuleItem);
			bEnable = 0;
			break;
	}
	return bEnable;
}

unsigned int drvif_color_get_pqmask_module_DMA_Addr(unsigned char ModuleItem){
	unsigned char Address = 0;
	unsigned int u32_RegVal = 0;
	switch(ModuleItem){
		case PQMASK_I_SNR:
			u32_RegVal = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_Start_Addr_SNR_reg);
			Address = PQMASK_I_PQMASK_I_DMA_Start_Addr_SNR_get_dma_start_addr_snr(u32_RegVal);
			break;
		case PQMASK_I_DECONT:
			u32_RegVal = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_Start_Addr_DEC_reg);
			Address = PQMASK_I_PQMASK_I_DMA_Start_Addr_DEC_get_dma_start_addr_dec(u32_RegVal);
			break;
		case PQMASK_D_SHP_EDGE:
			u32_RegVal = IoReg_Read32(PQMASK_D_PQMASK_SHP_DMA_Start_Addr_EDGE_reg);
			Address = PQMASK_D_PQMASK_SHP_DMA_Start_Addr_EDGE_get_dma_start_addr_shp_edge(u32_RegVal);
			break;
		case PQMASK_D_SHP_TEXT:
			u32_RegVal = IoReg_Read32(PQMASK_D_PQMASK_SHP_DMA_Start_Addr_TEX_reg);
			Address = PQMASK_D_PQMASK_SHP_DMA_Start_Addr_TEX_get_dma_start_addr_shp_tex(u32_RegVal);
			break;
		case PQMASK_D_ICM_0:
			u32_RegVal = IoReg_Read32(PQMASK_D_PQMASK_ICM_DMA_Start_Addr_H_reg);
			Address = PQMASK_D_PQMASK_ICM_DMA_Start_Addr_H_get_dma_start_addr_icm_h(u32_RegVal);
			break;
		case PQMASK_D_ICM_1:
			u32_RegVal = IoReg_Read32(PQMASK_D_PQMASK_ICM_DMA_Start_Addr_S_reg);
			Address = PQMASK_D_PQMASK_ICM_DMA_Start_Addr_S_get_dma_start_addr_icm_s(u32_RegVal);
			break;
		case PQMASK_D_ICM_2:
			u32_RegVal = IoReg_Read32(PQMASK_D_PQMASK_ICM_DMA_Start_Addr_I_reg);
			Address = PQMASK_D_PQMASK_ICM_DMA_Start_Addr_I_get_dma_start_addr_icm_i(u32_RegVal);
			break;
		case PQMASK_D_LC:
			u32_RegVal = IoReg_Read32(PQMASK_D_PQMASK_LC_DMA_Start_Addr_reg);
			Address = PQMASK_D_PQMASK_LC_DMA_Start_Addr_get_dma_start_addr_lc(u32_RegVal);
			break;
		case PQMASK_D_DCC: // same with LC
			u32_RegVal = IoReg_Read32(PQMASK_D_PQMASK_LC_DMA_Start_Addr_reg);
			Address = PQMASK_D_PQMASK_LC_DMA_Start_Addr_get_dma_start_addr_lc(u32_RegVal);
			break;
		case PQMASK_SRNN_IN_0:
			u32_RegVal = IoReg_Read32(PQMASK_SRNN_PQMASK_SRNN_I_DMA_Start_Addr_0_reg);
			Address = PQMASK_SRNN_PQMASK_SRNN_I_DMA_Start_Addr_0_get_dma_start_addr_srnn_i_0(u32_RegVal);
			break;
		case PQMASK_SRNN_IN_1:
			u32_RegVal = IoReg_Read32(PQMASK_SRNN_PQMASK_SRNN_I_DMA_Start_Addr_1_reg);
			Address = PQMASK_SRNN_PQMASK_SRNN_I_DMA_Start_Addr_1_get_dma_start_addr_srnn_i_1(u32_RegVal);
			break;
		case PQMASK_SRNN_IN_2:
			u32_RegVal = IoReg_Read32(PQMASK_SRNN_PQMASK_SRNN_I_DMA_Start_Addr_2_reg);
			Address = PQMASK_SRNN_PQMASK_SRNN_I_DMA_Start_Addr_2_get_dma_start_addr_srnn_i_2(u32_RegVal);
			break;
		case PQMASK_SRNN_OUT_0:
			u32_RegVal = IoReg_Read32(PQMASK_SRNN_PQMASK_SRNN_O_DMA_Start_Addr_0_reg);
			Address = PQMASK_SRNN_PQMASK_SRNN_O_DMA_Start_Addr_0_get_dma_start_addr_srnn_o_0(u32_RegVal);
			break;
		default:
			rtd_pr_vpq_emerg("[%s] Module item: %d overflow\n", __FUNCTION__, ModuleItem);
			break;
	}
	return Address;
}



/*Merlin7*/

void drvif_color_get_pqmask_idomain_DMA_config(PQMASK_DOMAIN_DMA_CONFIG_T *pDmaConfig, PQMASK_BLOCK_DMA_CONFIG_T *pModuleConfig)
{
	pqmask_i_pqmask_i_dma_rd_rule_check_up_RBUS pqmask_i_pqmask_i_dma_rd_rule_check_up_reg;
	pqmask_i_pqmask_i_dma_rd_rule_check_low_RBUS pqmask_i_pqmask_i_dma_rd_rule_check_low_reg;
	pqmask_i_pqmask_i_dma_burst_RBUS pqmask_i_pqmask_i_dma_burst_reg;
	pqmask_i_pqmask_i_dma_line_step_RBUS pqmask_i_pqmask_i_dma_line_step_reg;
	pqmask_i_pqmask_i_dma_rd_ctrl_RBUS pqmask_i_pqmask_i_dma_rd_ctrl_reg;

	pqmask_i_pqmask_i_dma_rd_ctrl_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_RD_Ctrl_reg);
	pDmaConfig->SwapType =(pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_8byte_swap << 3) +
						  (pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_4byte_swap << 2) +
						  (pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_2byte_swap << 1) +
						  (pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_1byte_swap << 0);

	pqmask_i_pqmask_i_dma_rd_rule_check_up_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_RD_Rule_check_up_reg);
	pDmaConfig->AddrUpLimit = pqmask_i_pqmask_i_dma_rd_rule_check_up_reg.dma_up_limit;

	pqmask_i_pqmask_i_dma_rd_rule_check_low_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_RD_Rule_check_low_reg);
	pDmaConfig->AddrLowLimit = pqmask_i_pqmask_i_dma_rd_rule_check_low_reg.dma_low_limit;

	pqmask_i_pqmask_i_dma_burst_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_Burst_reg);
	pModuleConfig->DataBurstLen = pqmask_i_pqmask_i_dma_burst_reg.data_burst_length;

	pqmask_i_pqmask_i_dma_line_step_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_Line_Step_reg);
	pModuleConfig->LineStep = pqmask_i_pqmask_i_dma_line_step_reg.dma_line_step;
}


#if 0
/*I DOMAIN GET FUNCTIONS*/
#endif

void drvif_color_get_pqmask_idomain_scaling_enable(unsigned char *bHorEnable, unsigned char *bVerEnable)
{
	pqmask_i_pqmask_i_scaleup_hor_RBUS pqmask_i_pqmask_i_scaleup_hor_reg;
	pqmask_i_pqmask_i_scaleup_ver_RBUS pqmask_i_pqmask_i_scaleup_ver_reg;

	pqmask_i_pqmask_i_scaleup_hor_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Scaleup_Hor_reg);
	*bHorEnable = (pqmask_i_pqmask_i_scaleup_hor_reg.h_zoom_en != 0)? 1 : 0;

	pqmask_i_pqmask_i_scaleup_ver_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Scaleup_Ver_reg);
	*bVerEnable = (pqmask_i_pqmask_i_scaleup_ver_reg.v_zoom_en != 0)? 1 : 0;
}

void drvif_color_get_pqmask_idomain_scaling(PQMASK_SACLING_T *ptr)
{
	pqmask_i_pqmask_i_scaleup_hor_RBUS pqmask_i_pqmask_i_scaleup_hor_reg;
	pqmask_i_pqmask_i_scaleup_ver_RBUS pqmask_i_pqmask_i_scaleup_ver_reg;
	pqmask_i_pqmask_i_scaleup_phase_RBUS pqmask_i_pqmask_i_scaleup_phase_reg;

	pqmask_i_pqmask_i_scaleup_hor_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Scaleup_Hor_reg);
	ptr->Hor.Factor = pqmask_i_pqmask_i_scaleup_hor_reg.hor_factor;

	pqmask_i_pqmask_i_scaleup_ver_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Scaleup_Ver_reg);
	ptr->Ver.Factor = pqmask_i_pqmask_i_scaleup_ver_reg.ver_factor;

	pqmask_i_pqmask_i_scaleup_phase_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Scaleup_Phase_reg);
	ptr->Hor.InitPhase = pqmask_i_pqmask_i_scaleup_phase_reg.hor_ini;
	ptr->Ver.InitPhase = pqmask_i_pqmask_i_scaleup_phase_reg.ver_ini;
}

void drvif_color_get_pqmask_idomain_size(PQMASK_IO_SIZE_T *ptr)
{
	pqmask_i_pqmask_i_intputsize_ctrl_RBUS pqmask_i_pqmask_i_intputsize_ctrl_reg;
	pqmask_i_pqmask_i_intputsize_ctrl2_RBUS pqmask_i_pqmask_i_intputsize_ctrl2_reg;
	pqmask_i_pqmask_i_outputsize_ctrl_RBUS pqmask_i_pqmask_i_outputsize_ctrl_reg;
	pqmask_i_pqmask_i_outputsize_ctrl2_RBUS pqmask_i_pqmask_i_outputsize_ctrl2_reg;

	pqmask_i_pqmask_i_intputsize_ctrl_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_IntputSize_Ctrl_reg);
	ptr->Input.HorSta = pqmask_i_pqmask_i_intputsize_ctrl_reg.in_h_sta;
	ptr->Input.Width = pqmask_i_pqmask_i_intputsize_ctrl_reg.in_width;

	pqmask_i_pqmask_i_intputsize_ctrl2_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_IntputSize_Ctrl2_reg);
	ptr->Input.VerSta = pqmask_i_pqmask_i_intputsize_ctrl2_reg.in_v_sta;
	ptr->Input.Height = pqmask_i_pqmask_i_intputsize_ctrl2_reg.in_height;

	pqmask_i_pqmask_i_outputsize_ctrl_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_OutputSize_Ctrl_reg);
	ptr->Output.HorSta = pqmask_i_pqmask_i_outputsize_ctrl_reg.out_h_sta;
	ptr->Output.Width = pqmask_i_pqmask_i_outputsize_ctrl_reg.out_width;

	pqmask_i_pqmask_i_outputsize_ctrl2_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_OutputSize_Ctrl2_reg);
	ptr->Output.VerSta = pqmask_i_pqmask_i_outputsize_ctrl2_reg.out_v_sta;
	ptr->Output.Height = pqmask_i_pqmask_i_outputsize_ctrl2_reg.out_height;
}

void drvif_color_get_pqmask_idomain_size_rd_status(PQMASK_IMG_SIZE_T *ptr)
{
	pqmask_i_pqmask_i_outputsize_read_RBUS pqmask_i_pqmask_i_outputsize_read_reg;

	pqmask_i_pqmask_i_outputsize_read_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_OutputSize_Read_reg);

	ptr->Width = pqmask_i_pqmask_i_outputsize_read_reg.width_r;
	ptr->Height = pqmask_i_pqmask_i_outputsize_read_reg.height_r;
}

void drvif_color_get_pqmask_idomain_dm_window_enable(unsigned char *bEnable)
{
	pqmask_i_pqmask_i_window_sta_RBUS pqmask_i_pqmask_i_window_sta_reg;
	pqmask_i_pqmask_i_window_sta_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Window_sta_reg);
	*bEnable = (pqmask_i_pqmask_i_window_sta_reg.window_en != 0)? 1 : 0;
}

void drvif_color_get_pqmask_idomain_dm_window(PQMASK_WINDOW_T *ptr)
{
	pqmask_i_pqmask_i_window_sta_RBUS pqmask_i_pqmask_i_window_sta_reg;
	pqmask_i_pqmask_i_window_end_RBUS pqmask_i_pqmask_i_window_end_reg;
	pqmask_i_pqmask_i_winow_weight_RBUS pqmask_i_pqmask_i_window_weight_reg;

	pqmask_i_pqmask_i_window_sta_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Window_sta_reg);
	ptr->HorSta = pqmask_i_pqmask_i_window_sta_reg.win_x_sta;
	ptr->VerSta = pqmask_i_pqmask_i_window_sta_reg.win_y_sta;

	pqmask_i_pqmask_i_window_end_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Window_end_reg);
	ptr->HorEnd = pqmask_i_pqmask_i_window_end_reg.win_x_end;
	ptr->VerSta = pqmask_i_pqmask_i_window_end_reg.win_y_end;

	pqmask_i_pqmask_i_window_weight_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Winow_weight_reg);
	ptr->weight.DecontourWeight = pqmask_i_pqmask_i_window_weight_reg.weight_dec;
	ptr->weight.SNRWeight = pqmask_i_pqmask_i_window_weight_reg.weight_snr;
}

#if 0
/*D DOMAIN DMA SET FUNCTIONS*/
#endif


#if 0
/*D DOMAIN DMA GET FUNCTIONS*/
#endif

void drvif_color_get_pqmask_ddomain_DMA_config(PQMASK_DOMAIN_DMA_CONFIG_T *pDmaConfig)
{
	pqmask_d_pqmask_d_dma_rd_ctrl_RBUS pqmask_d_pqmask_d_dma_rd_ctrl_reg;
	pqmask_d_pqmask_d_dma_rd_rule_check_low_RBUS pqmask_d_pqmask_d_dma_rd_rule_check_low_reg;
	pqmask_d_pqmask_d_dma_rd_rule_check_up_RBUS pqmask_d_pqmask_d_dma_rd_rule_check_up_reg;

	pqmask_d_pqmask_d_dma_rd_ctrl_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_D_DMA_RD_Ctrl_reg);

	pDmaConfig->SwapType =(pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_8byte_swap << 3) +
						  (pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_4byte_swap << 2) +
						  (pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_2byte_swap << 1) +
						  (pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_1byte_swap << 0);

	pqmask_d_pqmask_d_dma_rd_rule_check_low_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_D_DMA_RD_Rule_check_low_reg);
	pDmaConfig->AddrLowLimit = pqmask_d_pqmask_d_dma_rd_rule_check_low_reg.dma_low_limit;

	pqmask_d_pqmask_d_dma_rd_rule_check_up_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_D_DMA_RD_Rule_check_up_reg);
	pDmaConfig->AddrUpLimit = pqmask_d_pqmask_d_dma_rd_rule_check_up_reg.dma_up_limit;
}

#if 0
/*D DOMAIN SHARPNESS DMA SET FUNCTIONS*/
#endif

void drvif_color_set_pqmask_shp_DMA_config(PQMASK_BLOCK_DMA_CONFIG_T *pModuleConfig)
{
	pqmask_d_pqmask_dma_en_shp_RBUS pqmask_d_pqmask_dma_en_shp_reg;
	pqmask_d_pqmask_shp_dma_line_step_RBUS pqmask_d_pqmask_shp_dma_line_step_reg;

	pqmask_d_pqmask_dma_en_shp_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_shp_reg);
	pqmask_d_pqmask_dma_en_shp_reg.data_burst_length_shp = pModuleConfig->DataBurstLen;
	IoReg_Write32(PQMASK_D_PQMASK_DMA_en_shp_reg, pqmask_d_pqmask_dma_en_shp_reg.regValue);

	pqmask_d_pqmask_shp_dma_line_step_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_DMA_Line_Step_reg);
	pqmask_d_pqmask_shp_dma_line_step_reg.dma_shp_line_step = pModuleConfig->LineStep;
	IoReg_Write32(PQMASK_D_PQMASK_SHP_DMA_Line_Step_reg, pqmask_d_pqmask_shp_dma_line_step_reg.regValue);
}

#if 0
/*D DOMAIN SHARPNESS DMA GET FUNCTIONS*/
#endif
void drvif_color_get_pqmask_shp_DMA_config(PQMASK_BLOCK_DMA_CONFIG_T *pModuleConfig)
{
	pqmask_d_pqmask_dma_en_shp_RBUS pqmask_d_pqmask_dma_en_shp_reg;
	pqmask_d_pqmask_shp_dma_line_step_RBUS pqmask_d_pqmask_shp_dma_line_step_reg;

	pqmask_d_pqmask_dma_en_shp_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_shp_reg);
	pModuleConfig->DataBurstLen = pqmask_d_pqmask_dma_en_shp_reg.data_burst_length_shp;

	pqmask_d_pqmask_shp_dma_line_step_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_DMA_Line_Step_reg);
	pModuleConfig->LineStep = pqmask_d_pqmask_shp_dma_line_step_reg.dma_shp_line_step;
}

#if 0
/*D DOMAIN SHARPNESS GET FUNCTIONS*/
#endif

void drvif_color_get_pqmask_shp_scaling_enable(unsigned char *bHorEnable, unsigned char *bVerEnable)
{
	pqmask_d_pqmask_shp_scaleup_hor_RBUS pqmask_d_pqmask_shp_scaleup_hor_reg;
	pqmask_d_pqmask_shp_scaleup_ver_RBUS pqmask_d_pqmask_shp_scaleup_ver_reg;

	pqmask_d_pqmask_shp_scaleup_hor_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_Scaleup_Hor_reg);
	*bHorEnable = (pqmask_d_pqmask_shp_scaleup_hor_reg.shp_h_zoom_en != 0)? 1 : 0;

	pqmask_d_pqmask_shp_scaleup_ver_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_Scaleup_Ver_reg);
	*bVerEnable = (pqmask_d_pqmask_shp_scaleup_ver_reg.shp_v_zoom_en != 0)? 1 : 0;
}

void drvif_color_get_pqmask_shp_scaling(PQMASK_SACLING_T *ptr)
{
	pqmask_d_pqmask_shp_scaleup_hor_RBUS pqmask_d_pqmask_shp_scaleup_hor_reg;
	pqmask_d_pqmask_shp_scaleup_ver_RBUS pqmask_d_pqmask_shp_scaleup_ver_reg;
	pqmask_d_pqmask_shp_scaleup_phase_RBUS pqmask_d_pqmask_shp_scaleup_phase_reg;

	pqmask_d_pqmask_shp_scaleup_hor_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_Scaleup_Hor_reg);
	ptr->Hor.Factor = pqmask_d_pqmask_shp_scaleup_hor_reg.shp_hor_factor;

	pqmask_d_pqmask_shp_scaleup_ver_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_Scaleup_Ver_reg);
	ptr->Ver.Factor = pqmask_d_pqmask_shp_scaleup_ver_reg.shp_ver_factor;

	pqmask_d_pqmask_shp_scaleup_phase_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_Scaleup_Phase_reg);
	ptr->Hor.InitPhase = pqmask_d_pqmask_shp_scaleup_phase_reg.shp_hor_ini;
	ptr->Ver.InitPhase = pqmask_d_pqmask_shp_scaleup_phase_reg.shp_ver_ini;
}

void drvif_color_get_pqmask_shp_size(PQMASK_IO_SIZE_T *ptr)
{
	pqmask_d_pqmask_shp_intputsize_ctrl_RBUS pqmask_d_pqmask_shp_intputsize_ctrl_reg;
	pqmask_d_pqmask_shp_intputsize_ctrl2_RBUS pqmask_d_pqmask_shp_intputsize_ctrl2_reg;
	pqmask_d_pqmask_shp_outputsize_ctrl_RBUS pqmask_d_pqmask_shp_outputsize_ctrl_reg;
	pqmask_d_pqmask_shp_outputsize_ctrl2_RBUS pqmask_d_pqmask_shp_outputsize_ctrl2_reg;

	pqmask_d_pqmask_shp_intputsize_ctrl_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_IntputSize_Ctrl_reg);
	ptr->Input.HorSta = pqmask_d_pqmask_shp_intputsize_ctrl_reg.shp_in_h_sta;
	ptr->Input.Width = pqmask_d_pqmask_shp_intputsize_ctrl_reg.shp_in_width;

	pqmask_d_pqmask_shp_intputsize_ctrl2_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_IntputSize_Ctrl2_reg);
	ptr->Input.VerSta = pqmask_d_pqmask_shp_intputsize_ctrl2_reg.shp_in_v_sta;
	ptr->Input.Height = pqmask_d_pqmask_shp_intputsize_ctrl2_reg.shp_in_height;

	pqmask_d_pqmask_shp_outputsize_ctrl_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_OutputSize_Ctrl_reg);
	ptr->Output.HorSta = pqmask_d_pqmask_shp_outputsize_ctrl_reg.shp_out_h_sta;
	ptr->Output.Width = pqmask_d_pqmask_shp_outputsize_ctrl_reg.shp_out_width;

	pqmask_d_pqmask_shp_outputsize_ctrl2_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_OutputSize_Ctrl2_reg);
	ptr->Output.VerSta = pqmask_d_pqmask_shp_outputsize_ctrl2_reg.shp_out_v_sta;
	ptr->Output.Height = pqmask_d_pqmask_shp_outputsize_ctrl2_reg.shp_out_height;
}

void drvif_color_get_pqmask_shp_size_rd_status(PQMASK_IMG_SIZE_T *ptr)
{
	pqmask_d_pqmask_shp_outputsize_read_RBUS pqmask_d_pqmask_shp_outputsize_read_reg;

	pqmask_d_pqmask_shp_outputsize_read_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_OutputSize_Read_reg);

	ptr->Width = pqmask_d_pqmask_shp_outputsize_read_reg.shp_width_r;
	ptr->Height = pqmask_d_pqmask_shp_outputsize_read_reg.shp_height_r;
}

void drvif_color_get_pqmask_shp_dm_window_enable(unsigned char *bEnable)
{
	pqmask_d_pqmask_shp_window_sta_RBUS pqmask_d_pqmask_shp_window_sta_reg;
	
	pqmask_d_pqmask_shp_window_sta_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_Window_STA_reg);
	*bEnable = (pqmask_d_pqmask_shp_window_sta_reg.shp_window_en != 0)? 1 : 0;
}

void drvif_color_get_pqmask_shp_dm_window(PQMASK_WINDOW_T *ptr)
{
	pqmask_d_pqmask_shp_window_sta_RBUS pqmask_d_pqmask_shp_window_sta_reg;
	pqmask_d_pqmask_shp_window_end_RBUS pqmask_d_pqmask_shp_window_end_reg;
	pqmask_d_pqmask_shp_winow_weight_RBUS pqmask_d_pqmask_shp_winow_weight_reg;

	pqmask_d_pqmask_shp_window_sta_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_Window_STA_reg);
	ptr->HorSta = pqmask_d_pqmask_shp_window_sta_reg.shp_window_x_sta;
	ptr->VerSta = pqmask_d_pqmask_shp_window_sta_reg.shp_window_y_sta;
	IoReg_Write32(PQMASK_D_PQMASK_SHP_Window_STA_reg, pqmask_d_pqmask_shp_window_sta_reg.regValue);

	pqmask_d_pqmask_shp_window_end_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_Window_END_reg);
	ptr->HorEnd = pqmask_d_pqmask_shp_window_end_reg.shp_window_x_end;
	ptr->VerEnd = pqmask_d_pqmask_shp_window_end_reg.shp_window_y_end;
	IoReg_Write32(PQMASK_D_PQMASK_SHP_Window_END_reg, pqmask_d_pqmask_shp_window_end_reg.regValue);

	pqmask_d_pqmask_shp_winow_weight_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_Winow_weight_reg);
	ptr->weight.EdgeWeight = pqmask_d_pqmask_shp_winow_weight_reg.edge_weight;
	ptr->weight.TextureWeight = pqmask_d_pqmask_shp_winow_weight_reg.texture_weight;
}


#if 0
/*D DOMAIN ICM GET FUNCTIONS*/
#endif

void drvif_color_get_pqmask_icm_scaling_enable(unsigned char *bHorEnable, unsigned char *bVerEnable)
{
	pqmask_d_pqmask_icm_scaleup_hor_RBUS pqmask_d_pqmask_icm_scaleup_hor_reg;
	pqmask_d_pqmask_icm_scaleup_ver_RBUS pqmask_d_pqmask_icm_scaleup_ver_reg;

	pqmask_d_pqmask_icm_scaleup_hor_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_Scaleup_Hor_reg);
	*bHorEnable = (pqmask_d_pqmask_icm_scaleup_hor_reg.icm_h_zoom_en != 0)? 1 : 0;

	pqmask_d_pqmask_icm_scaleup_ver_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_Scaleup_Ver_reg);
	*bVerEnable = (pqmask_d_pqmask_icm_scaleup_ver_reg.icm_v_zoom_en != 0)? 1 : 0;
}

void drvif_color_get_pqmask_icm_scaling(PQMASK_SACLING_T *ptr)
{
	pqmask_d_pqmask_icm_scaleup_hor_RBUS pqmask_d_pqmask_icm_scaleup_hor_reg;
	pqmask_d_pqmask_icm_scaleup_ver_RBUS pqmask_d_pqmask_icm_scaleup_ver_reg;
	pqmask_d_pqmask_icm_scaleup_phase_RBUS pqmask_d_pqmask_icm_scaleup_phase_reg;

	pqmask_d_pqmask_icm_scaleup_hor_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_Scaleup_Hor_reg);
	ptr->Hor.Factor = pqmask_d_pqmask_icm_scaleup_hor_reg.icm_hor_factor;

	pqmask_d_pqmask_icm_scaleup_ver_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_Scaleup_Ver_reg);
	ptr->Ver.Factor = pqmask_d_pqmask_icm_scaleup_ver_reg.icm_ver_factor;

	pqmask_d_pqmask_icm_scaleup_phase_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_Scaleup_Phase_reg);
	ptr->Hor.InitPhase = pqmask_d_pqmask_icm_scaleup_phase_reg.icm_hor_ini;
	ptr->Ver.InitPhase = pqmask_d_pqmask_icm_scaleup_phase_reg.icm_ver_ini;
}

void drvif_color_get_pqmask_icm_size(PQMASK_IO_SIZE_T *ptr)
{
	pqmask_d_pqmask_icm_intputsize_ctrl_RBUS pqmask_d_pqmask_icm_intputsize_ctrl_reg;
	pqmask_d_pqmask_icm_intputsize_ctrl2_RBUS pqmask_d_pqmask_icm_intputsize_ctrl2_reg;
	pqmask_d_pqmask_icm_outputsize_ctrl_RBUS pqmask_d_pqmask_icm_outputsize_ctrl_reg;
	pqmask_d_pqmask_icm_outputsize_ctrl2_RBUS pqmask_d_pqmask_icm_outputsize_ctrl2_reg;

	pqmask_d_pqmask_icm_intputsize_ctrl_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_IntputSize_Ctrl_reg);
	ptr->Input.HorSta = pqmask_d_pqmask_icm_intputsize_ctrl_reg.icm_in_h_sta;
	ptr->Input.Width = pqmask_d_pqmask_icm_intputsize_ctrl_reg.icm_in_width;

	pqmask_d_pqmask_icm_intputsize_ctrl2_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_IntputSize_Ctrl2_reg);
	ptr->Input.VerSta = pqmask_d_pqmask_icm_intputsize_ctrl2_reg.icm_in_v_sta;
	ptr->Input.Height = pqmask_d_pqmask_icm_intputsize_ctrl2_reg.icm_in_height;

	pqmask_d_pqmask_icm_outputsize_ctrl_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_OutputSize_Ctrl_reg);
	ptr->Output.HorSta = pqmask_d_pqmask_icm_outputsize_ctrl_reg.icm_out_h_sta;
	ptr->Output.Width = pqmask_d_pqmask_icm_outputsize_ctrl_reg.icm_out_width;

	pqmask_d_pqmask_icm_outputsize_ctrl2_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_OutputSize_Ctrl2_reg);
	ptr->Output.VerSta = pqmask_d_pqmask_icm_outputsize_ctrl2_reg.icm_out_v_sta;
	ptr->Output.Height = pqmask_d_pqmask_icm_outputsize_ctrl2_reg.icm_out_height;
}

void drvif_color_get_pqmask_icm_size_rd_status(PQMASK_IMG_SIZE_T *ptr)
{
	pqmask_d_pqmask_icm_outputsize_read_RBUS pqmask_d_pqmask_icm_outputsize_read_reg;

	pqmask_d_pqmask_icm_outputsize_read_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_OutputSize_Read_reg);

	ptr->Width = pqmask_d_pqmask_icm_outputsize_read_reg.icm_width_r;
	ptr->Height = pqmask_d_pqmask_icm_outputsize_read_reg.icm_height_r;
}

void drvif_color_get_pqmask_icm_dm_window_enable(unsigned char *bEnable)
{
	pqmask_d_pqmask_icm_window_sta_RBUS pqmask_d_pqmask_icm_window_sta_reg;
	
	pqmask_d_pqmask_icm_window_sta_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_Window_STA_reg);
	*bEnable = (pqmask_d_pqmask_icm_window_sta_reg.icm_window_en != 0)? 1 : 0;
}

void drvif_color_get_pqmask_icm_dm_window(PQMASK_WINDOW_T *ptr)
{
	pqmask_d_pqmask_icm_window_sta_RBUS pqmask_d_pqmask_icm_window_sta_reg;
	pqmask_d_pqmask_icm_window_end_RBUS pqmask_d_pqmask_icm_window_end_reg;
	pqmask_d_pqmask_icm_winow_weight_RBUS pqmask_d_pqmask_icm_winow_weight_reg;

	pqmask_d_pqmask_icm_window_sta_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_Window_STA_reg);
	ptr->HorSta = pqmask_d_pqmask_icm_window_sta_reg.icm_window_x_sta;
	ptr->VerSta = pqmask_d_pqmask_icm_window_sta_reg.icm_window_y_sta;

	pqmask_d_pqmask_icm_window_end_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_Window_END_reg);
	ptr->HorEnd = pqmask_d_pqmask_icm_window_end_reg.icm_window_x_end;
	ptr->VerEnd = pqmask_d_pqmask_icm_window_end_reg.icm_window_y_end;

	pqmask_d_pqmask_icm_winow_weight_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_Winow_weight_reg);
	ptr->weight.ICM_Weight0 = pqmask_d_pqmask_icm_winow_weight_reg.icm_hoffet_weight;
	ptr->weight.ICM_Weight1 = pqmask_d_pqmask_icm_winow_weight_reg.icm_soffet_weight;
	ptr->weight.ICM_Weight2 = pqmask_d_pqmask_icm_winow_weight_reg.icm_ioffet_weight;
}


#if 0
/*D DOMAIN LOCALCONTRAST DMA GET FUNCTIONS*/
#endif

void drvif_color_get_pqmask_local_contrast_DMA_config(PQMASK_BLOCK_DMA_CONFIG_T *pModuleConfig)
{
	pqmask_d_pqmask_lc_dma_line_step_RBUS pqmask_d_pqmask_lc_dma_line_step_reg;
	pqmask_d_pqmask_dma_en_lc_RBUS pqmask_d_pqmask_dma_en_lc_reg;

	pqmask_d_pqmask_lc_dma_line_step_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_DMA_Line_Step_reg);
	pModuleConfig->LineStep = pqmask_d_pqmask_lc_dma_line_step_reg.dma_lc_line_step;

	pqmask_d_pqmask_dma_en_lc_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_lc_reg);
	pModuleConfig->DataBurstLen = pqmask_d_pqmask_dma_en_lc_reg.data_burst_length_lc;
}

#if 0
/*D DOMAIN LOCALCONTRAST GET FUNCTIONS*/
#endif

void drvif_color_get_pqmask_local_contrast_scaling_enable(unsigned char *bHorEnable, unsigned char *bVerEnable)
{
	pqmask_d_pqmask_lc_scaleup_hor_RBUS pqmask_d_pqmask_lc_scaleup_hor_reg;
	pqmask_d_pqmask_lc_scaleup_ver_RBUS pqmask_d_pqmask_lc_scaleup_ver_reg;

	pqmask_d_pqmask_lc_scaleup_hor_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_Scaleup_Hor_reg);
	*bHorEnable = (pqmask_d_pqmask_lc_scaleup_hor_reg.lc_h_zoom_en != 0)? 1 : 0;

	pqmask_d_pqmask_lc_scaleup_ver_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_Scaleup_Ver_reg);
	*bVerEnable = (pqmask_d_pqmask_lc_scaleup_ver_reg.lc_v_zoom_en != 0)? 1 : 0;
}

void drvif_color_get_pqmask_local_contrast_scaling(PQMASK_SACLING_T *ptr)
{
	pqmask_d_pqmask_lc_scaleup_hor_RBUS pqmask_d_pqmask_lc_scaleup_hor_reg;
	pqmask_d_pqmask_lc_scaleup_ver_RBUS pqmask_d_pqmask_lc_scaleup_ver_reg;
	pqmask_d_pqmask_lc_scaleup_phase_RBUS pqmask_d_pqmask_lc_scaleup_phase_reg;

	pqmask_d_pqmask_lc_scaleup_hor_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_Scaleup_Hor_reg);
	ptr->Hor.Factor = pqmask_d_pqmask_lc_scaleup_hor_reg.lc_hor_factor;

	pqmask_d_pqmask_lc_scaleup_ver_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_Scaleup_Ver_reg);
	ptr->Ver.Factor = pqmask_d_pqmask_lc_scaleup_ver_reg.lc_ver_factor;

	pqmask_d_pqmask_lc_scaleup_phase_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_Scaleup_Phase_reg);
	ptr->Hor.InitPhase = pqmask_d_pqmask_lc_scaleup_phase_reg.lc_hor_ini;
	ptr->Ver.InitPhase = pqmask_d_pqmask_lc_scaleup_phase_reg.lc_ver_ini;
}

void drvif_color_get_pqmask_local_contrast_size(PQMASK_IO_SIZE_T *ptr)
{
	pqmask_d_pqmask_lc_intputsize_ctrl_RBUS pqmask_d_pqmask_lc_intputsize_ctrl_reg;
	pqmask_d_pqmask_lc_intputsize_ctrl2_RBUS pqmask_d_pqmask_lc_intputsize_ctrl2_reg;
	pqmask_d_pqmask_lc_outputsize_ctrl_RBUS pqmask_d_pqmask_lc_outputsize_ctrl_reg;
	pqmask_d_pqmask_lc_outputsize_ctrl2_RBUS pqmask_d_pqmask_lc_outputsize_ctrl2_reg;

	pqmask_d_pqmask_lc_intputsize_ctrl_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_IntputSize_Ctrl_reg);
	ptr->Input.HorSta = pqmask_d_pqmask_lc_intputsize_ctrl_reg.lc_in_h_sta;
	ptr->Input.Width = pqmask_d_pqmask_lc_intputsize_ctrl_reg.lc_in_width;

	pqmask_d_pqmask_lc_intputsize_ctrl2_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_IntputSize_Ctrl2_reg);
	ptr->Input.VerSta = pqmask_d_pqmask_lc_intputsize_ctrl2_reg.lc_in_v_sta;
	ptr->Input.Height = pqmask_d_pqmask_lc_intputsize_ctrl2_reg.lc_in_height;

	pqmask_d_pqmask_lc_outputsize_ctrl_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_OutputSize_Ctrl_reg);
	ptr->Output.HorSta = pqmask_d_pqmask_lc_outputsize_ctrl_reg.lc_out_h_sta;
	ptr->Output.Width = pqmask_d_pqmask_lc_outputsize_ctrl_reg.lc_out_width;

	pqmask_d_pqmask_lc_outputsize_ctrl2_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_OutputSize_Ctrl2_reg);
	ptr->Output.VerSta = pqmask_d_pqmask_lc_outputsize_ctrl2_reg.lc_out_v_sta;
	ptr->Output.Height = pqmask_d_pqmask_lc_outputsize_ctrl2_reg.lc_out_height;
}

void drvif_color_get_pqmask_local_contrast_size_rd_status(PQMASK_IMG_SIZE_T *ptr)
{
	pqmask_d_pqmask_lc_outputsize_read_RBUS pqmask_d_pqmask_lc_outputsize_read_reg;

	pqmask_d_pqmask_lc_outputsize_read_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_OutputSize_Read_reg);

	ptr->Width = pqmask_d_pqmask_lc_outputsize_read_reg.lc_width_r;
	ptr->Height = pqmask_d_pqmask_lc_outputsize_read_reg.lc_height_r;
}

void drvif_color_get_pqmask_local_contrast_dm_window_enable(unsigned char *bEnable)
{
	pqmask_d_pqmask_lc_window_sta_RBUS pqmask_d_pqmask_lc_window_sta_reg;
	
	pqmask_d_pqmask_lc_window_sta_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_Window_STA_reg);
	*bEnable = (pqmask_d_pqmask_lc_window_sta_reg.lc_window_en != 0)? 1 : 0;
}

void drvif_color_get_pqmask_local_contrast_dm_window(PQMASK_WINDOW_T *ptr)
{
	pqmask_d_pqmask_lc_window_sta_RBUS pqmask_d_pqmask_lc_window_sta_reg;
	pqmask_d_pqmask_lc_window_end_RBUS pqmask_d_pqmask_lc_window_end_reg;
	pqmask_d_pqmask_lc_winow_weight_RBUS pqmask_d_pqmask_lc_winow_weight_reg;

	pqmask_d_pqmask_lc_window_sta_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_Window_STA_reg);
	ptr->HorSta = pqmask_d_pqmask_lc_window_sta_reg.lc_window_x_sta;
	ptr->VerSta = pqmask_d_pqmask_lc_window_sta_reg.lc_window_y_sta;

	pqmask_d_pqmask_lc_window_end_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_Window_END_reg);
	ptr->HorEnd = pqmask_d_pqmask_lc_window_end_reg.lc_window_x_end;
	ptr->VerEnd = pqmask_d_pqmask_lc_window_end_reg.lc_window_y_end;

	pqmask_d_pqmask_lc_winow_weight_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_Winow_weight_reg);
	ptr->weight.LocalContrastWeight = pqmask_d_pqmask_lc_winow_weight_reg.lc_weight;
}


