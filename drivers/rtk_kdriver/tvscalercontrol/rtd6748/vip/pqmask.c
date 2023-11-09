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

#if 0
/*DOUBLE BUFFER CONTROL*/
#endif
void drvif_module_set_pqmask_idomain_db_enable(unsigned char bEnable) 
{
	if( bEnable != 0 )
		IoReg_SetBits(PQMASK_I_PQMASK_I_Double_BUF_reg, PQMASK_I_PQMASK_I_Double_BUF_db_en_mask);
	else
		IoReg_ClearBits(PQMASK_I_PQMASK_I_Double_BUF_reg, PQMASK_I_PQMASK_I_Double_BUF_db_en_mask);
}

void drvif_module_set_pqmask_idomain_db_apply(void)
{
	IoReg_SetBits(PQMASK_I_PQMASK_I_Double_BUF_reg, PQMASK_I_PQMASK_I_Double_BUF_db_apply_mask);
}

int drvif_module_wait_pqmask_idomain_db_apply_done(unsigned int WaitCnt)
{
	int Counter = WaitCnt;
	int Ret = 0;
	pqmask_i_pqmask_i_double_buf_RBUS pqmask_i_pqmask_i_double_buf_reg;
	do {
		usleep_range(500, 1000);
		Counter--;
		pqmask_i_pqmask_i_double_buf_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Double_BUF_reg);
	}
	while( (Counter>0) && (pqmask_i_pqmask_i_double_buf_reg.db_apply!=0) );

	if( Counter <= 0 )
		Ret = -1;

	return Ret;
}

void drvif_module_set_pqmask_ddomain_db_enable(unsigned char bEnable) 
{
	if( bEnable != 0 )
		IoReg_SetBits(PQMASK_D_PQMASK_Double_BUF_reg, PQMASK_D_PQMASK_Double_BUF_db_en_mask);
	else
		IoReg_ClearBits(PQMASK_D_PQMASK_Double_BUF_reg, PQMASK_D_PQMASK_Double_BUF_db_en_mask);
}

void drvif_module_set_pqmask_ddomain_db_apply(void)
{
	IoReg_SetBits(PQMASK_D_PQMASK_Double_BUF_reg, PQMASK_D_PQMASK_Double_BUF_db_apply_mask);
}

int drvif_module_wait_pqmask_ddomain_db_apply_done(unsigned int WaitCnt)
{
	int Counter = WaitCnt;
	int Ret = 0;
	pqmask_d_pqmask_double_buf_RBUS pqmask_d_pqmask_double_buf_reg;
	do {
		usleep_range(500, 1000);
		Counter--;
		pqmask_d_pqmask_double_buf_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_Double_BUF_reg);
	}
	while( (Counter>0) && (pqmask_d_pqmask_double_buf_reg.db_apply!=0) );

	if( Counter <= 0 )
		Ret = -1;

	return Ret;
}

#if 0
/*I DOMAIN DMA SET FUNCTIONS*/
#endif

void drvif_color_set_pqmask_idomain_DMA_enable(unsigned char bEnable)
{
	pqmask_i_pqmask_i_dma_rd_ctrl_RBUS pqmask_i_pqmask_i_dma_rd_ctrl_reg;

	pqmask_i_pqmask_i_dma_rd_ctrl_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_RD_Ctrl_reg);
	pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_enable = (bEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_I_PQMASK_I_DMA_RD_Ctrl_reg, pqmask_i_pqmask_i_dma_rd_ctrl_reg.regValue);
}

void drvif_color_set_pqmask_idomain_DMA_config(PQMASK_DMA_CONFIG_T *pDmaConfig, PQMASK_MODULE_DMA_CONFIG_T *pModuleConfig)
{
	pqmask_i_pqmask_i_dma_rd_rule_check_up_RBUS pqmask_i_pqmask_i_dma_rd_rule_check_up_reg;
	pqmask_i_pqmask_i_dma_rd_rule_check_low_RBUS pqmask_i_pqmask_i_dma_rd_rule_check_low_reg;
	pqmask_i_pqmask_i_dma_burst_RBUS pqmask_i_pqmask_i_dma_burst_reg;
	pqmask_i_pqmask_i_dma_line_step_RBUS pqmask_i_pqmask_i_dma_line_step_reg;
	pqmask_i_pqmask_i_dma_rd_ctrl_RBUS pqmask_i_pqmask_i_dma_rd_ctrl_reg;

	pqmask_i_pqmask_i_dma_rd_ctrl_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_RD_Ctrl_reg);
	switch(pDmaConfig->SwapType)
	{
		case PQMASK_DMA_NONE_SWP:
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_1byte_swap = 0;
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_2byte_swap = 0;
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_4byte_swap = 0;
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_8byte_swap = 0;
			break;
		case PQMASK_DMA_1BYTE_SWP:
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_1byte_swap = 1;
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_2byte_swap = 0;
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_4byte_swap = 0;
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_8byte_swap = 0;
			break;
		case PQMASK_DMA_2BYTE_SWP:
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_1byte_swap = 0;
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_2byte_swap = 1;
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_4byte_swap = 0;
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_8byte_swap = 0;
			break;
		case PQMASK_DMA_4BYTE_SWP:
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_1byte_swap = 0;
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_2byte_swap = 0;
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_4byte_swap = 1;
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_8byte_swap = 0;
			break;
		case PQMASK_DMA_8BYTE_SWP:
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_1byte_swap = 0;
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_2byte_swap = 0;
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_4byte_swap = 0;
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_8byte_swap = 1;
			break;
		default:
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_1byte_swap = 0;
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_2byte_swap = 0;
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_4byte_swap = 0;
			pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_8byte_swap = 0;
			break;
	}
	IoReg_Write32(PQMASK_I_PQMASK_I_DMA_RD_Ctrl_reg, pqmask_i_pqmask_i_dma_rd_ctrl_reg.regValue);

	pqmask_i_pqmask_i_dma_rd_rule_check_up_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_RD_Rule_check_up_reg);
	pqmask_i_pqmask_i_dma_rd_rule_check_up_reg.dma_up_limit = pDmaConfig->AddrUpLimit;
	IoReg_Write32(PQMASK_I_PQMASK_I_DMA_RD_Rule_check_up_reg, pqmask_i_pqmask_i_dma_rd_rule_check_up_reg.regValue);

	pqmask_i_pqmask_i_dma_rd_rule_check_low_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_RD_Rule_check_low_reg);
	pqmask_i_pqmask_i_dma_rd_rule_check_low_reg.dma_low_limit = pDmaConfig->AddrLowLimit;
	IoReg_Write32(PQMASK_I_PQMASK_I_DMA_RD_Rule_check_low_reg, pqmask_i_pqmask_i_dma_rd_rule_check_low_reg.regValue);

	pqmask_i_pqmask_i_dma_burst_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_Burst_reg);
	pqmask_i_pqmask_i_dma_burst_reg.data_burst_length = pModuleConfig->DataBurstLen;
	IoReg_Write32(PQMASK_I_PQMASK_I_DMA_Burst_reg, pqmask_i_pqmask_i_dma_burst_reg.regValue);

	pqmask_i_pqmask_i_dma_line_step_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_Line_Step_reg);
	pqmask_i_pqmask_i_dma_line_step_reg.dma_line_step = pModuleConfig->LineStep;
	IoReg_Write32(PQMASK_I_PQMASK_I_DMA_Line_Step_reg, pqmask_i_pqmask_i_dma_line_step_reg.regValue);
}

void drvif_color_set_pqmask_idomain_DMA_SNR_Addr(unsigned int Addr)
{
	pqmask_i_pqmask_i_dma_start_addr_snr_RBUS pqmask_i_pqmask_i_dma_start_addr_snr_reg;

	pqmask_i_pqmask_i_dma_start_addr_snr_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_Start_Addr_SNR_reg);
	pqmask_i_pqmask_i_dma_start_addr_snr_reg.dma_start_addr_snr = Addr;
	IoReg_Write32(PQMASK_I_PQMASK_I_DMA_Start_Addr_SNR_reg, pqmask_i_pqmask_i_dma_start_addr_snr_reg.regValue);
}

void drvif_color_set_pqmask_idomain_DMA_Decontour_Addr(unsigned int Addr)
{
	pqmask_i_pqmask_i_dma_start_addr_dec_RBUS pqmask_i_pqmask_i_dma_start_addr_dec_reg;

	pqmask_i_pqmask_i_dma_start_addr_dec_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_Start_Addr_DEC_reg);
	pqmask_i_pqmask_i_dma_start_addr_dec_reg.dma_start_addr_dec = Addr;
	IoReg_Write32(PQMASK_I_PQMASK_I_DMA_Start_Addr_DEC_reg, pqmask_i_pqmask_i_dma_start_addr_dec_reg.regValue);
}

void drvif_color_set_pqmask_idomain_SNR_enable(unsigned char bEnable)
{
	pqmask_i_pqmask_i_dma_en_RBUS pqmask_i_pqmask_i_dma_en_reg;

	pqmask_i_pqmask_i_dma_en_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_en_reg);
	pqmask_i_pqmask_i_dma_en_reg.pqmask_snr_en = (bEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_I_PQMASK_I_DMA_en_reg, pqmask_i_pqmask_i_dma_en_reg.regValue);
}

void drvif_color_set_pqmask_idomain_Decontour_enable(unsigned char bEnable)
{
	pqmask_i_pqmask_i_dma_en_RBUS pqmask_i_pqmask_i_dma_en_reg;

	pqmask_i_pqmask_i_dma_en_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_en_reg);
	pqmask_i_pqmask_i_dma_en_reg.pqmask_dec_en = (bEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_I_PQMASK_I_DMA_en_reg, pqmask_i_pqmask_i_dma_en_reg.regValue);
}

#if 0
/*I DOMAIN DMA GET FUNCTIONS*/
#endif

void drvif_color_get_pqmask_idomain_DMA_enable(unsigned char *bEnable)
{
	pqmask_i_pqmask_i_dma_rd_ctrl_RBUS pqmask_i_pqmask_i_dma_rd_ctrl_reg;

	pqmask_i_pqmask_i_dma_rd_ctrl_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_RD_Ctrl_reg);
	*bEnable = ( pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_enable!= 0)? 1 : 0;
}

void drvif_color_get_pqmask_idomain_DMA_config(PQMASK_DMA_CONFIG_T *pDmaConfig, PQMASK_MODULE_DMA_CONFIG_T *pModuleConfig)
{
	pqmask_i_pqmask_i_dma_rd_rule_check_up_RBUS pqmask_i_pqmask_i_dma_rd_rule_check_up_reg;
	pqmask_i_pqmask_i_dma_rd_rule_check_low_RBUS pqmask_i_pqmask_i_dma_rd_rule_check_low_reg;
	pqmask_i_pqmask_i_dma_burst_RBUS pqmask_i_pqmask_i_dma_burst_reg;
	pqmask_i_pqmask_i_dma_line_step_RBUS pqmask_i_pqmask_i_dma_line_step_reg;
	pqmask_i_pqmask_i_dma_rd_ctrl_RBUS pqmask_i_pqmask_i_dma_rd_ctrl_reg;

	pqmask_i_pqmask_i_dma_rd_ctrl_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_RD_Ctrl_reg);
	switch( (pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_8byte_swap << 3) +
			(pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_4byte_swap << 2) +
			(pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_2byte_swap << 1) +
			(pqmask_i_pqmask_i_dma_rd_ctrl_reg.dma_1byte_swap << 0)
		)
	{
		case 0x0:
			pDmaConfig->SwapType = PQMASK_DMA_NONE_SWP;
			break;
		case 0x1:
			pDmaConfig->SwapType = PQMASK_DMA_1BYTE_SWP;
			break;
		case 0x2:
			pDmaConfig->SwapType = PQMASK_DMA_2BYTE_SWP;
			break;
		case 0x4:
			pDmaConfig->SwapType = PQMASK_DMA_4BYTE_SWP;
			break;
		case 0x8:
			pDmaConfig->SwapType = PQMASK_DMA_8BYTE_SWP;
			break;
		default:
			pDmaConfig->SwapType = PQMASK_DMA_NONE_SWP;
			break;
	}

	pqmask_i_pqmask_i_dma_rd_rule_check_up_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_RD_Rule_check_up_reg);
	pDmaConfig->AddrUpLimit = pqmask_i_pqmask_i_dma_rd_rule_check_up_reg.dma_up_limit;

	pqmask_i_pqmask_i_dma_rd_rule_check_low_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_RD_Rule_check_low_reg);
	pDmaConfig->AddrLowLimit = pqmask_i_pqmask_i_dma_rd_rule_check_low_reg.dma_low_limit;

	pqmask_i_pqmask_i_dma_burst_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_Burst_reg);
	pModuleConfig->DataBurstLen = pqmask_i_pqmask_i_dma_burst_reg.data_burst_length;

	pqmask_i_pqmask_i_dma_line_step_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_Line_Step_reg);
	pModuleConfig->LineStep = pqmask_i_pqmask_i_dma_line_step_reg.dma_line_step;
}

void drvif_color_get_pqmask_idomain_DMA_SNR_Addr(unsigned int *Addr)
{
	pqmask_i_pqmask_i_dma_start_addr_snr_RBUS pqmask_i_pqmask_i_dma_start_addr_snr_reg;

	pqmask_i_pqmask_i_dma_start_addr_snr_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_Start_Addr_SNR_reg);
	*Addr = pqmask_i_pqmask_i_dma_start_addr_snr_reg.dma_start_addr_snr;
}

void drvif_color_get_pqmask_idomain_DMA_Decontour_Addr(unsigned int *Addr)
{
	pqmask_i_pqmask_i_dma_start_addr_dec_RBUS pqmask_i_pqmask_i_dma_start_addr_dec_reg;

	pqmask_i_pqmask_i_dma_start_addr_dec_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_Start_Addr_DEC_reg);
	*Addr = pqmask_i_pqmask_i_dma_start_addr_dec_reg.dma_start_addr_dec;
}

void drvif_color_get_pqmask_idomain_SNR_enable(unsigned char *bEnable)
{
	pqmask_i_pqmask_i_dma_en_RBUS pqmask_i_pqmask_i_dma_en_reg;

	pqmask_i_pqmask_i_dma_en_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_en_reg);
	*bEnable = (pqmask_i_pqmask_i_dma_en_reg.pqmask_snr_en != 0)? 1 : 0;
}

void drvif_color_get_pqmask_idomain_Decontour_enable(unsigned char *bEnable)
{
	pqmask_i_pqmask_i_dma_en_RBUS pqmask_i_pqmask_i_dma_en_reg;

	pqmask_i_pqmask_i_dma_en_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_DMA_en_reg);
	*bEnable = (pqmask_i_pqmask_i_dma_en_reg.pqmask_snr_en != 0)? 1 : 0;
}

#if 0
/*I DOMAIN SET FUNCTIONS*/
#endif

void drvif_color_set_pqmask_idomain_scaling_enable(unsigned char bHorEnable, unsigned char bVerEnable)
{
	pqmask_i_pqmask_i_scaleup_hor_RBUS pqmask_i_pqmask_i_scaleup_hor_reg;
	pqmask_i_pqmask_i_scaleup_ver_RBUS pqmask_i_pqmask_i_scaleup_ver_reg;

	pqmask_i_pqmask_i_scaleup_hor_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Scaleup_Hor_reg);
	pqmask_i_pqmask_i_scaleup_hor_reg.h_zoom_en = (bHorEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_I_PQMASK_I_Scaleup_Hor_reg, pqmask_i_pqmask_i_scaleup_hor_reg.regValue);

	pqmask_i_pqmask_i_scaleup_ver_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Scaleup_Ver_reg);
	pqmask_i_pqmask_i_scaleup_ver_reg.v_zoom_en = (bVerEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_I_PQMASK_I_Scaleup_Ver_reg, pqmask_i_pqmask_i_scaleup_ver_reg.regValue);
}

void drvif_color_set_pqmask_idomain_scaling(PQMASK_SACLING_T *ptr)
{
	pqmask_i_pqmask_i_scaleup_hor_RBUS pqmask_i_pqmask_i_scaleup_hor_reg;
	pqmask_i_pqmask_i_scaleup_ver_RBUS pqmask_i_pqmask_i_scaleup_ver_reg;
	pqmask_i_pqmask_i_scaleup_phase_RBUS pqmask_i_pqmask_i_scaleup_phase_reg;

	pqmask_i_pqmask_i_scaleup_hor_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Scaleup_Hor_reg);
	pqmask_i_pqmask_i_scaleup_hor_reg.hor_factor = ptr->Hor.Factor;
	IoReg_Write32(PQMASK_I_PQMASK_I_Scaleup_Hor_reg, pqmask_i_pqmask_i_scaleup_hor_reg.regValue);

	pqmask_i_pqmask_i_scaleup_ver_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Scaleup_Ver_reg);
	pqmask_i_pqmask_i_scaleup_ver_reg.ver_factor = ptr->Ver.Factor;
	IoReg_Write32(PQMASK_I_PQMASK_I_Scaleup_Ver_reg, pqmask_i_pqmask_i_scaleup_ver_reg.regValue);

	pqmask_i_pqmask_i_scaleup_phase_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Scaleup_Phase_reg);
	pqmask_i_pqmask_i_scaleup_phase_reg.hor_ini = ptr->Hor.InitPhase;
	pqmask_i_pqmask_i_scaleup_phase_reg.ver_ini = ptr->Ver.InitPhase;
	IoReg_Write32(PQMASK_I_PQMASK_I_Scaleup_Phase_reg, pqmask_i_pqmask_i_scaleup_phase_reg.regValue);
}

void drvif_color_set_pqmask_idomain_size(PQMASK_IO_SIZE_T *ptr)
{
	pqmask_i_pqmask_i_intputsize_ctrl_RBUS pqmask_i_pqmask_i_intputsize_ctrl_reg;
	pqmask_i_pqmask_i_intputsize_ctrl2_RBUS pqmask_i_pqmask_i_intputsize_ctrl2_reg;
	pqmask_i_pqmask_i_outputsize_ctrl_RBUS pqmask_i_pqmask_i_outputsize_ctrl_reg;
	pqmask_i_pqmask_i_outputsize_ctrl2_RBUS pqmask_i_pqmask_i_outputsize_ctrl2_reg;

	pqmask_i_pqmask_i_intputsize_ctrl_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_IntputSize_Ctrl_reg);
	pqmask_i_pqmask_i_intputsize_ctrl_reg.in_h_sta = ptr->Input.HorSta;
	pqmask_i_pqmask_i_intputsize_ctrl_reg.in_width = ptr->Input.Width;
	IoReg_Write32(PQMASK_I_PQMASK_I_IntputSize_Ctrl_reg, pqmask_i_pqmask_i_intputsize_ctrl_reg.regValue);

	pqmask_i_pqmask_i_intputsize_ctrl2_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_IntputSize_Ctrl2_reg);
	pqmask_i_pqmask_i_intputsize_ctrl2_reg.in_v_sta = ptr->Input.VerSta;
	pqmask_i_pqmask_i_intputsize_ctrl2_reg.in_height = ptr->Input.Height;
	IoReg_Write32(PQMASK_I_PQMASK_I_IntputSize_Ctrl2_reg, pqmask_i_pqmask_i_intputsize_ctrl2_reg.regValue);

	pqmask_i_pqmask_i_outputsize_ctrl_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_OutputSize_Ctrl_reg);
	pqmask_i_pqmask_i_outputsize_ctrl_reg.out_h_sta = ptr->Output.HorSta;
	pqmask_i_pqmask_i_outputsize_ctrl_reg.out_width = ptr->Output.Width;
	IoReg_Write32(PQMASK_I_PQMASK_I_OutputSize_Ctrl_reg, pqmask_i_pqmask_i_outputsize_ctrl_reg.regValue);

	pqmask_i_pqmask_i_outputsize_ctrl2_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_OutputSize_Ctrl2_reg);
	pqmask_i_pqmask_i_outputsize_ctrl2_reg.out_v_sta = ptr->Output.VerSta;
	pqmask_i_pqmask_i_outputsize_ctrl2_reg.out_height = ptr->Output.Height;
	IoReg_Write32(PQMASK_I_PQMASK_I_OutputSize_Ctrl2_reg, pqmask_i_pqmask_i_outputsize_ctrl2_reg.regValue);
}

void drvif_color_set_pqmask_idomain_dm_window_enable(unsigned char bEnable)
{
	pqmask_i_pqmask_i_window_sta_RBUS pqmask_i_pqmask_i_window_sta_reg;
	pqmask_i_pqmask_i_window_sta_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Window_sta_reg);
	pqmask_i_pqmask_i_window_sta_reg.window_en = (bEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_I_PQMASK_I_Window_sta_reg, pqmask_i_pqmask_i_window_sta_reg.regValue);
}

void drvif_color_set_pqmask_idomain_dm_window(PQMASK_WINDOW_T *ptr)
{
	pqmask_i_pqmask_i_window_sta_RBUS pqmask_i_pqmask_i_window_sta_reg;
	pqmask_i_pqmask_i_window_end_RBUS pqmask_i_pqmask_i_window_end_reg;
	pqmask_i_pqmask_i_winow_weight_RBUS pqmask_i_pqmask_i_window_weight_reg;

	pqmask_i_pqmask_i_window_sta_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Window_sta_reg);
	pqmask_i_pqmask_i_window_sta_reg.win_x_sta = ptr->HorSta;
	pqmask_i_pqmask_i_window_sta_reg.win_y_sta = ptr->VerSta;
	IoReg_Write32(PQMASK_I_PQMASK_I_Window_sta_reg, pqmask_i_pqmask_i_window_sta_reg.regValue);

	pqmask_i_pqmask_i_window_end_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Window_end_reg);
	pqmask_i_pqmask_i_window_end_reg.win_x_end = ptr->HorEnd;
	pqmask_i_pqmask_i_window_end_reg.win_y_end = ptr->VerSta;
	IoReg_Write32(PQMASK_I_PQMASK_I_Window_end_reg, pqmask_i_pqmask_i_window_end_reg.regValue);

	pqmask_i_pqmask_i_window_weight_reg.regValue = IoReg_Read32(PQMASK_I_PQMASK_I_Winow_weight_reg);
	pqmask_i_pqmask_i_window_weight_reg.weight_dec = ptr->weight.DecontourWeight;
	pqmask_i_pqmask_i_window_weight_reg.weight_snr = ptr->weight.SNRWeight;
	IoReg_Write32(PQMASK_I_PQMASK_I_Winow_weight_reg, pqmask_i_pqmask_i_window_weight_reg.regValue);
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

void drvif_color_set_pqmask_ddomain_DMA_enable(unsigned char bEnable)
{
	pqmask_d_pqmask_d_dma_rd_ctrl_RBUS pqmask_d_pqmask_d_dma_rd_ctrl_reg;

	pqmask_d_pqmask_d_dma_rd_ctrl_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_D_DMA_RD_Ctrl_reg);
	pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_enable = (bEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_D_PQMASK_D_DMA_RD_Ctrl_reg, pqmask_d_pqmask_d_dma_rd_ctrl_reg.regValue);
}

void drvif_color_set_pqmask_ddomain_DMA_config(PQMASK_DMA_CONFIG_T *pDmaConfig)
{
	pqmask_d_pqmask_d_dma_rd_ctrl_RBUS pqmask_d_pqmask_d_dma_rd_ctrl_reg;
	pqmask_d_pqmask_d_dma_rd_rule_check_low_RBUS pqmask_d_pqmask_d_dma_rd_rule_check_low_reg;
	pqmask_d_pqmask_d_dma_rd_rule_check_up_RBUS pqmask_d_pqmask_d_dma_rd_rule_check_up_reg;

	pqmask_d_pqmask_d_dma_rd_ctrl_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_D_DMA_RD_Ctrl_reg);
	switch(pDmaConfig->SwapType)
	{
		case PQMASK_DMA_NONE_SWP:
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_1byte_swap = 0;
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_2byte_swap = 0;
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_4byte_swap = 0;
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_8byte_swap = 0;
			break;
		case PQMASK_DMA_1BYTE_SWP:
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_1byte_swap = 1;
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_2byte_swap = 0;
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_4byte_swap = 0;
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_8byte_swap = 0;
			break;
		case PQMASK_DMA_2BYTE_SWP:
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_1byte_swap = 0;
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_2byte_swap = 1;
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_4byte_swap = 0;
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_8byte_swap = 0;
			break;
		case PQMASK_DMA_4BYTE_SWP:
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_1byte_swap = 0;
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_2byte_swap = 0;
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_4byte_swap = 1;
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_8byte_swap = 0;
			break;
		case PQMASK_DMA_8BYTE_SWP:
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_1byte_swap = 0;
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_2byte_swap = 0;
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_4byte_swap = 0;
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_8byte_swap = 1;
			break;
		default:
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_1byte_swap = 0;
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_2byte_swap = 0;
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_4byte_swap = 0;
			pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_8byte_swap = 0;
			break;
	}
	IoReg_Write32(PQMASK_D_PQMASK_D_DMA_RD_Ctrl_reg, pqmask_d_pqmask_d_dma_rd_ctrl_reg.regValue);

	pqmask_d_pqmask_d_dma_rd_rule_check_low_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_D_DMA_RD_Rule_check_low_reg);
	pqmask_d_pqmask_d_dma_rd_rule_check_low_reg.dma_low_limit = pDmaConfig->AddrLowLimit;
	IoReg_Write32(PQMASK_D_PQMASK_D_DMA_RD_Rule_check_low_reg, pqmask_d_pqmask_d_dma_rd_rule_check_low_reg.regValue);

	pqmask_d_pqmask_d_dma_rd_rule_check_up_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_D_DMA_RD_Rule_check_up_reg);
	pqmask_d_pqmask_d_dma_rd_rule_check_up_reg.dma_up_limit = pDmaConfig->AddrUpLimit;
	IoReg_Write32(PQMASK_D_PQMASK_D_DMA_RD_Rule_check_up_reg, pqmask_d_pqmask_d_dma_rd_rule_check_up_reg.regValue);
}

#if 0
/*D DOMAIN DMA GET FUNCTIONS*/
#endif

void drvif_color_get_pqmask_ddomain_DMA_enable(unsigned char *bEnable)
{
	pqmask_d_pqmask_d_dma_rd_ctrl_RBUS pqmask_d_pqmask_d_dma_rd_ctrl_reg;

	pqmask_d_pqmask_d_dma_rd_ctrl_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_D_DMA_RD_Ctrl_reg);
	*bEnable = (pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_enable != 0)? 1 : 0;
}

void drvif_color_get_pqmask_ddomain_DMA_config(PQMASK_DMA_CONFIG_T *pDmaConfig)
{
	pqmask_d_pqmask_d_dma_rd_ctrl_RBUS pqmask_d_pqmask_d_dma_rd_ctrl_reg;
	pqmask_d_pqmask_d_dma_rd_rule_check_low_RBUS pqmask_d_pqmask_d_dma_rd_rule_check_low_reg;
	pqmask_d_pqmask_d_dma_rd_rule_check_up_RBUS pqmask_d_pqmask_d_dma_rd_rule_check_up_reg;

	pqmask_d_pqmask_d_dma_rd_ctrl_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_D_DMA_RD_Ctrl_reg);
	switch( (pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_8byte_swap << 3) +
			(pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_4byte_swap << 2) +
			(pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_2byte_swap << 1) +
			(pqmask_d_pqmask_d_dma_rd_ctrl_reg.dma_1byte_swap << 0)
		)
	{
		case 0x0:
			pDmaConfig->SwapType = PQMASK_DMA_NONE_SWP;
			break;
		case 0x1:
			pDmaConfig->SwapType = PQMASK_DMA_1BYTE_SWP;
			break;
		case 0x2:
			pDmaConfig->SwapType = PQMASK_DMA_2BYTE_SWP;
			break;
		case 0x4:
			pDmaConfig->SwapType = PQMASK_DMA_4BYTE_SWP;
			break;
		case 0x8:
			pDmaConfig->SwapType = PQMASK_DMA_8BYTE_SWP;
			break;
		default:
			pDmaConfig->SwapType = PQMASK_DMA_NONE_SWP;
			break;
	}

	pqmask_d_pqmask_d_dma_rd_rule_check_low_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_D_DMA_RD_Rule_check_low_reg);
	pDmaConfig->AddrLowLimit = pqmask_d_pqmask_d_dma_rd_rule_check_low_reg.dma_low_limit;

	pqmask_d_pqmask_d_dma_rd_rule_check_up_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_D_DMA_RD_Rule_check_up_reg);
	pDmaConfig->AddrUpLimit = pqmask_d_pqmask_d_dma_rd_rule_check_up_reg.dma_up_limit;
}

#if 0
/*D DOMAIN SHARPNESS DMA SET FUNCTIONS*/
#endif
void drvif_color_set_pqmask_shp_DMA_config(PQMASK_MODULE_DMA_CONFIG_T *pModuleConfig)
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

void drvif_color_set_pqmask_shp_Text_Addr(unsigned int Addr) {

	pqmask_d_pqmask_shp_dma_start_addr_tex_RBUS pqmask_d_pqmask_shp_dma_start_addr_tex_reg;

	pqmask_d_pqmask_shp_dma_start_addr_tex_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_DMA_Start_Addr_TEX_reg);
	pqmask_d_pqmask_shp_dma_start_addr_tex_reg.dma_start_addr_shp_tex = Addr;
	IoReg_Write32(PQMASK_D_PQMASK_SHP_DMA_Start_Addr_TEX_reg, pqmask_d_pqmask_shp_dma_start_addr_tex_reg.regValue);

}

void drvif_color_set_pqmask_shp_Edge_Addr(unsigned int Addr) {
	
	pqmask_d_pqmask_shp_dma_start_addr_edge_RBUS pqmask_d_pqmask_shp_dma_start_addr_edge_reg;

	pqmask_d_pqmask_shp_dma_start_addr_edge_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_DMA_Start_Addr_EDGE_reg);
	pqmask_d_pqmask_shp_dma_start_addr_edge_reg.dma_start_addr_shp_edge = Addr;
	IoReg_Write32(PQMASK_D_PQMASK_SHP_DMA_Start_Addr_EDGE_reg, pqmask_d_pqmask_shp_dma_start_addr_edge_reg.regValue);
}

void drvif_color_set_pqmask_shp_enable(unsigned char bTextureEnable, unsigned char bEdgeEnable)
{
	pqmask_d_pqmask_dma_en_shp_RBUS pqmask_d_pqmask_dma_en_shp_reg;

	pqmask_d_pqmask_dma_en_shp_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_shp_reg);
	pqmask_d_pqmask_dma_en_shp_reg.shp_tex_pqmask_en = (bTextureEnable != 0)? 1 : 0;
	pqmask_d_pqmask_dma_en_shp_reg.shp_edge_pqmask_en = (bEdgeEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_D_PQMASK_DMA_en_shp_reg, pqmask_d_pqmask_dma_en_shp_reg.regValue);
}

#if 0
/*D DOMAIN SHARPNESS DMA GET FUNCTIONS*/
#endif
void drvif_color_get_pqmask_shp_DMA_config(PQMASK_MODULE_DMA_CONFIG_T *pModuleConfig)
{
	pqmask_d_pqmask_dma_en_shp_RBUS pqmask_d_pqmask_dma_en_shp_reg;
	pqmask_d_pqmask_shp_dma_line_step_RBUS pqmask_d_pqmask_shp_dma_line_step_reg;

	pqmask_d_pqmask_dma_en_shp_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_shp_reg);
	pModuleConfig->DataBurstLen = pqmask_d_pqmask_dma_en_shp_reg.data_burst_length_shp;

	pqmask_d_pqmask_shp_dma_line_step_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_DMA_Line_Step_reg);
	pModuleConfig->LineStep = pqmask_d_pqmask_shp_dma_line_step_reg.dma_shp_line_step;
}

void drvif_color_get_pqmask_shp_Text_Addr(unsigned int *Addr) {

	pqmask_d_pqmask_shp_dma_start_addr_tex_RBUS pqmask_d_pqmask_shp_dma_start_addr_tex_reg;

	pqmask_d_pqmask_shp_dma_start_addr_tex_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_DMA_Start_Addr_TEX_reg);
	*Addr = pqmask_d_pqmask_shp_dma_start_addr_tex_reg.dma_start_addr_shp_tex;

}

void drvif_color_get_pqmask_shp_Edge_Addr(unsigned int *Addr) {

	pqmask_d_pqmask_shp_dma_start_addr_edge_RBUS pqmask_d_pqmask_shp_dma_start_addr_edge_reg;

	pqmask_d_pqmask_shp_dma_start_addr_edge_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_DMA_Start_Addr_EDGE_reg);
	*Addr = pqmask_d_pqmask_shp_dma_start_addr_edge_reg.dma_start_addr_shp_edge;	
}

void drvif_color_get_pqmask_shp_enable(unsigned char *bTextureEnable, unsigned char *bEdgeEnable)
{
	pqmask_d_pqmask_dma_en_shp_RBUS pqmask_d_pqmask_dma_en_shp_reg;

	pqmask_d_pqmask_dma_en_shp_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_shp_reg);
	*bTextureEnable = (pqmask_d_pqmask_dma_en_shp_reg.shp_tex_pqmask_en != 0)? 1 : 0;
	*bEdgeEnable = ( pqmask_d_pqmask_dma_en_shp_reg.shp_edge_pqmask_en != 0)? 1 : 0;
}

#if 0
/*D DOMAIN SHARPNESS SET FUNCTIONS*/
#endif

void drvif_color_set_pqmask_shp_scaling_enable(unsigned char bHorEnable, unsigned char bVerEnable)
{
	pqmask_d_pqmask_shp_scaleup_hor_RBUS pqmask_d_pqmask_shp_scaleup_hor_reg;
	pqmask_d_pqmask_shp_scaleup_ver_RBUS pqmask_d_pqmask_shp_scaleup_ver_reg;

	pqmask_d_pqmask_shp_scaleup_hor_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_Scaleup_Hor_reg);
	pqmask_d_pqmask_shp_scaleup_hor_reg.shp_h_zoom_en = (bHorEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_D_PQMASK_SHP_Scaleup_Hor_reg, pqmask_d_pqmask_shp_scaleup_hor_reg.regValue);

	pqmask_d_pqmask_shp_scaleup_ver_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_Scaleup_Ver_reg);
	pqmask_d_pqmask_shp_scaleup_ver_reg.shp_v_zoom_en = (bVerEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_D_PQMASK_SHP_Scaleup_Ver_reg, pqmask_d_pqmask_shp_scaleup_ver_reg.regValue);
}

void drvif_color_set_pqmask_shp_scaling(PQMASK_SACLING_T *ptr)
{
	pqmask_d_pqmask_shp_scaleup_hor_RBUS pqmask_d_pqmask_shp_scaleup_hor_reg;
	pqmask_d_pqmask_shp_scaleup_ver_RBUS pqmask_d_pqmask_shp_scaleup_ver_reg;
	pqmask_d_pqmask_shp_scaleup_phase_RBUS pqmask_d_pqmask_shp_scaleup_phase_reg;

	pqmask_d_pqmask_shp_scaleup_hor_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_Scaleup_Hor_reg);
	pqmask_d_pqmask_shp_scaleup_hor_reg.shp_hor_factor = ptr->Hor.Factor;
	IoReg_Write32(PQMASK_D_PQMASK_SHP_Scaleup_Hor_reg, pqmask_d_pqmask_shp_scaleup_hor_reg.regValue);

	pqmask_d_pqmask_shp_scaleup_ver_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_Scaleup_Ver_reg);
	pqmask_d_pqmask_shp_scaleup_ver_reg.shp_ver_factor = ptr->Ver.Factor;
	IoReg_Write32(PQMASK_D_PQMASK_SHP_Scaleup_Ver_reg, pqmask_d_pqmask_shp_scaleup_ver_reg.regValue);

	pqmask_d_pqmask_shp_scaleup_phase_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_Scaleup_Phase_reg);
	pqmask_d_pqmask_shp_scaleup_phase_reg.shp_hor_ini = ptr->Hor.InitPhase;
	pqmask_d_pqmask_shp_scaleup_phase_reg.shp_ver_ini = ptr->Ver.InitPhase;
	IoReg_Write32(PQMASK_D_PQMASK_SHP_Scaleup_Phase_reg, pqmask_d_pqmask_shp_scaleup_phase_reg.regValue);
}

void drvif_color_set_pqmask_shp_size(PQMASK_IO_SIZE_T *ptr)
{
	pqmask_d_pqmask_shp_intputsize_ctrl_RBUS pqmask_d_pqmask_shp_intputsize_ctrl_reg;
	pqmask_d_pqmask_shp_intputsize_ctrl2_RBUS pqmask_d_pqmask_shp_intputsize_ctrl2_reg;
	pqmask_d_pqmask_shp_outputsize_ctrl_RBUS pqmask_d_pqmask_shp_outputsize_ctrl_reg;
	pqmask_d_pqmask_shp_outputsize_ctrl2_RBUS pqmask_d_pqmask_shp_outputsize_ctrl2_reg;

	pqmask_d_pqmask_shp_intputsize_ctrl_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_IntputSize_Ctrl_reg);
	pqmask_d_pqmask_shp_intputsize_ctrl_reg.shp_in_h_sta = ptr->Input.HorSta;
	pqmask_d_pqmask_shp_intputsize_ctrl_reg.shp_in_width = ptr->Input.Width;
	IoReg_Write32(PQMASK_D_PQMASK_SHP_IntputSize_Ctrl_reg, pqmask_d_pqmask_shp_intputsize_ctrl_reg.regValue);

	pqmask_d_pqmask_shp_intputsize_ctrl2_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_IntputSize_Ctrl2_reg);
	pqmask_d_pqmask_shp_intputsize_ctrl2_reg.shp_in_v_sta = ptr->Input.VerSta;
	pqmask_d_pqmask_shp_intputsize_ctrl2_reg.shp_in_height = ptr->Input.Height;
	IoReg_Write32(PQMASK_D_PQMASK_SHP_IntputSize_Ctrl2_reg, pqmask_d_pqmask_shp_intputsize_ctrl2_reg.regValue);

	pqmask_d_pqmask_shp_outputsize_ctrl_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_OutputSize_Ctrl_reg);
	pqmask_d_pqmask_shp_outputsize_ctrl_reg.shp_out_h_sta = ptr->Output.HorSta;
	pqmask_d_pqmask_shp_outputsize_ctrl_reg.shp_out_width = ptr->Output.Width;
	IoReg_Write32(PQMASK_D_PQMASK_SHP_OutputSize_Ctrl_reg, pqmask_d_pqmask_shp_outputsize_ctrl_reg.regValue);

	pqmask_d_pqmask_shp_outputsize_ctrl2_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_OutputSize_Ctrl2_reg);
	pqmask_d_pqmask_shp_outputsize_ctrl2_reg.shp_out_v_sta = ptr->Output.VerSta;
	pqmask_d_pqmask_shp_outputsize_ctrl2_reg.shp_out_height = ptr->Output.Height;
	IoReg_Write32(PQMASK_D_PQMASK_SHP_OutputSize_Ctrl2_reg, pqmask_d_pqmask_shp_outputsize_ctrl2_reg.regValue);
}

void drvif_color_set_pqmask_shp_dm_window_enable(unsigned char bEnable)
{
	pqmask_d_pqmask_shp_window_sta_RBUS pqmask_d_pqmask_shp_window_sta_reg;
	
	pqmask_d_pqmask_shp_window_sta_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_Window_STA_reg);
	pqmask_d_pqmask_shp_window_sta_reg.shp_window_en = (bEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_D_PQMASK_SHP_Window_STA_reg, pqmask_d_pqmask_shp_window_sta_reg.regValue);
}

void drvif_color_set_pqmask_shp_dm_window(PQMASK_WINDOW_T *ptr)
{
	pqmask_d_pqmask_shp_window_sta_RBUS pqmask_d_pqmask_shp_window_sta_reg;
	pqmask_d_pqmask_shp_window_end_RBUS pqmask_d_pqmask_shp_window_end_reg;
	pqmask_d_pqmask_shp_winow_weight_RBUS pqmask_d_pqmask_shp_winow_weight_reg;

	pqmask_d_pqmask_shp_window_sta_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_Window_STA_reg);
	pqmask_d_pqmask_shp_window_sta_reg.shp_window_x_sta = ptr->HorSta;
	pqmask_d_pqmask_shp_window_sta_reg.shp_window_y_sta = ptr->VerSta;
	IoReg_Write32(PQMASK_D_PQMASK_SHP_Window_STA_reg, pqmask_d_pqmask_shp_window_sta_reg.regValue);

	pqmask_d_pqmask_shp_window_end_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_Window_END_reg);
	pqmask_d_pqmask_shp_window_end_reg.shp_window_x_end = ptr->HorEnd;
	pqmask_d_pqmask_shp_window_end_reg.shp_window_y_end = ptr->VerEnd;
	IoReg_Write32(PQMASK_D_PQMASK_SHP_Window_END_reg, pqmask_d_pqmask_shp_window_end_reg.regValue);

	pqmask_d_pqmask_shp_winow_weight_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_SHP_Winow_weight_reg);
	pqmask_d_pqmask_shp_winow_weight_reg.edge_weight = ptr->weight.EdgeWeight;
	pqmask_d_pqmask_shp_winow_weight_reg.texture_weight = ptr->weight.TextureWeight;
	IoReg_Write32(PQMASK_D_PQMASK_SHP_Winow_weight_reg, pqmask_d_pqmask_shp_winow_weight_reg.regValue);
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
/*D DOMAIN ICM DMA SET FUNCTIONS*/
#endif

void drvif_color_set_pqmask_icm_DMA_config(PQMASK_MODULE_DMA_CONFIG_T *pModuleConfig)
{
	pqmask_d_pqmask_icm_dma_line_step_RBUS pqmask_d_pqmask_icm_dma_line_step_reg;
	pqmask_d_pqmask_dma_en_icm_RBUS pqmask_d_pqmask_dma_en_icm_reg;

	pqmask_d_pqmask_icm_dma_line_step_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_DMA_Line_Step_reg);
	pqmask_d_pqmask_icm_dma_line_step_reg.dma_icm_line_step = pModuleConfig->LineStep;
	IoReg_Write32(PQMASK_D_PQMASK_ICM_DMA_Line_Step_reg, pqmask_d_pqmask_icm_dma_line_step_reg.regValue);

	pqmask_d_pqmask_dma_en_icm_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_icm_reg);
	pqmask_d_pqmask_dma_en_icm_reg.data_burst_length_icm = pModuleConfig->DataBurstLen;
	IoReg_Write32(PQMASK_D_PQMASK_DMA_en_icm_reg, pqmask_d_pqmask_dma_en_icm_reg.regValue);
}

void drvif_color_set_pqmask_icm_HUE_Addr(unsigned int Addr) {

	pqmask_d_pqmask_icm_dma_start_addr_h_RBUS pqmask_d_pqmask_icm_dma_start_addr_h_reg;

	pqmask_d_pqmask_icm_dma_start_addr_h_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_DMA_Start_Addr_H_reg);
	pqmask_d_pqmask_icm_dma_start_addr_h_reg.dma_start_addr_icm_h = Addr;
	IoReg_Write32(PQMASK_D_PQMASK_ICM_DMA_Start_Addr_H_reg, pqmask_d_pqmask_icm_dma_start_addr_h_reg.regValue);
}

void drvif_color_set_pqmask_icm_SAT_Addr(unsigned int Addr) {

	pqmask_d_pqmask_icm_dma_start_addr_s_RBUS pqmask_d_pqmask_icm_dma_start_addr_s_reg;

	pqmask_d_pqmask_icm_dma_start_addr_s_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_DMA_Start_Addr_S_reg);
	pqmask_d_pqmask_icm_dma_start_addr_s_reg.dma_start_addr_icm_s = Addr;
	IoReg_Write32(PQMASK_D_PQMASK_ICM_DMA_Start_Addr_S_reg, pqmask_d_pqmask_icm_dma_start_addr_s_reg.regValue);
}

void drvif_color_set_pqmask_icm_INT_Addr(unsigned int Addr) {

	pqmask_d_pqmask_icm_dma_start_addr_i_RBUS pqmask_d_pqmask_icm_dma_start_addr_i_reg;

	pqmask_d_pqmask_icm_dma_start_addr_i_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_DMA_Start_Addr_I_reg);
	pqmask_d_pqmask_icm_dma_start_addr_i_reg.dma_start_addr_icm_i = Addr;
	IoReg_Write32(PQMASK_D_PQMASK_ICM_DMA_Start_Addr_I_reg, pqmask_d_pqmask_icm_dma_start_addr_i_reg.regValue);
}

void drvif_color_set_pqmask_icm_enable(unsigned char bHueEnable, unsigned char bSaturationEnable, unsigned char bIntensityEnable)
{
	pqmask_d_pqmask_dma_en_icm_RBUS pqmask_d_pqmask_dma_en_icm_reg;

	pqmask_d_pqmask_dma_en_icm_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_icm_reg);
	pqmask_d_pqmask_dma_en_icm_reg.icm_h_pqmask_en = (bHueEnable != 0)? 1 : 0;
	pqmask_d_pqmask_dma_en_icm_reg.icm_s_pqmask_en = (bSaturationEnable != 0)? 1 : 0;
	pqmask_d_pqmask_dma_en_icm_reg.icm_i_pqmask_en = (bIntensityEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_D_PQMASK_DMA_en_icm_reg, pqmask_d_pqmask_dma_en_icm_reg.regValue);
}

#if 0
/*D DOMAIN ICM DMA GET FUNCTIONS*/
#endif

void drvif_color_get_pqmask_icm_DMA_config(PQMASK_MODULE_DMA_CONFIG_T *pModuleConfig)
{
	pqmask_d_pqmask_icm_dma_line_step_RBUS pqmask_d_pqmask_icm_dma_line_step_reg;
	pqmask_d_pqmask_dma_en_icm_RBUS pqmask_d_pqmask_dma_en_icm_reg;

	pqmask_d_pqmask_icm_dma_line_step_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_DMA_Line_Step_reg);
	pModuleConfig->LineStep = pqmask_d_pqmask_icm_dma_line_step_reg.dma_icm_line_step;

	pqmask_d_pqmask_dma_en_icm_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_icm_reg);
	pModuleConfig->DataBurstLen = pqmask_d_pqmask_dma_en_icm_reg.data_burst_length_icm;
}

void drvif_color_get_pqmask_icm_HUE_Addr(unsigned int *Addr) {

	pqmask_d_pqmask_icm_dma_start_addr_h_RBUS pqmask_d_pqmask_icm_dma_start_addr_h_reg;

	pqmask_d_pqmask_icm_dma_start_addr_h_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_DMA_Start_Addr_H_reg);
	*Addr = pqmask_d_pqmask_icm_dma_start_addr_h_reg.dma_start_addr_icm_h;
}

void drvif_color_get_pqmask_icm_SAT_Addr(unsigned int *Addr) {
	
	pqmask_d_pqmask_icm_dma_start_addr_s_RBUS pqmask_d_pqmask_icm_dma_start_addr_s_reg;

	pqmask_d_pqmask_icm_dma_start_addr_s_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_DMA_Start_Addr_S_reg);
	*Addr = pqmask_d_pqmask_icm_dma_start_addr_s_reg.dma_start_addr_icm_s;
}

void drvif_color_get_pqmask_icm_INT_Addr(unsigned int *Addr) {

	pqmask_d_pqmask_icm_dma_start_addr_i_RBUS pqmask_d_pqmask_icm_dma_start_addr_i_reg;

	pqmask_d_pqmask_icm_dma_start_addr_i_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_DMA_Start_Addr_I_reg);
	*Addr = pqmask_d_pqmask_icm_dma_start_addr_i_reg.dma_start_addr_icm_i;
}

void drvif_color_get_pqmask_icm_enable(unsigned char *bHueEnable, unsigned char *bSaturationEnable, unsigned char *bIntensityEnable)
{
	pqmask_d_pqmask_dma_en_icm_RBUS pqmask_d_pqmask_dma_en_icm_reg;

	pqmask_d_pqmask_dma_en_icm_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_icm_reg);
	*bHueEnable = (pqmask_d_pqmask_dma_en_icm_reg.icm_h_pqmask_en != 0)? 1 : 0;
	*bIntensityEnable = (pqmask_d_pqmask_dma_en_icm_reg.icm_i_pqmask_en != 0)? 1 : 0;
	*bSaturationEnable = (pqmask_d_pqmask_dma_en_icm_reg.icm_s_pqmask_en != 0)? 1 : 0;
	IoReg_Write32(PQMASK_D_PQMASK_DMA_en_icm_reg, pqmask_d_pqmask_dma_en_icm_reg.regValue);
}

#if 0
/*D DOMAIN ICM SET FUNCTIONS*/
#endif

void drvif_color_set_pqmask_icm_scaling_enable(unsigned char bHorEnable, unsigned char bVerEnable)
{
	pqmask_d_pqmask_icm_scaleup_hor_RBUS pqmask_d_pqmask_icm_scaleup_hor_reg;
	pqmask_d_pqmask_icm_scaleup_ver_RBUS pqmask_d_pqmask_icm_scaleup_ver_reg;

	pqmask_d_pqmask_icm_scaleup_hor_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_Scaleup_Hor_reg);
	pqmask_d_pqmask_icm_scaleup_hor_reg.icm_h_zoom_en = (bHorEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_D_PQMASK_ICM_Scaleup_Hor_reg, pqmask_d_pqmask_icm_scaleup_hor_reg.regValue);

	pqmask_d_pqmask_icm_scaleup_ver_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_Scaleup_Ver_reg);
	pqmask_d_pqmask_icm_scaleup_ver_reg.icm_v_zoom_en = (bVerEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_D_PQMASK_ICM_Scaleup_Ver_reg, pqmask_d_pqmask_icm_scaleup_ver_reg.regValue);
}

void drvif_color_set_pqmask_icm_scaling(PQMASK_SACLING_T *ptr)
{
	pqmask_d_pqmask_icm_scaleup_hor_RBUS pqmask_d_pqmask_icm_scaleup_hor_reg;
	pqmask_d_pqmask_icm_scaleup_ver_RBUS pqmask_d_pqmask_icm_scaleup_ver_reg;
	pqmask_d_pqmask_icm_scaleup_phase_RBUS pqmask_d_pqmask_icm_scaleup_phase_reg;

	pqmask_d_pqmask_icm_scaleup_hor_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_Scaleup_Hor_reg);
	pqmask_d_pqmask_icm_scaleup_hor_reg.icm_hor_factor = ptr->Hor.Factor;
	IoReg_Write32(PQMASK_D_PQMASK_ICM_Scaleup_Hor_reg, pqmask_d_pqmask_icm_scaleup_hor_reg.regValue);

	pqmask_d_pqmask_icm_scaleup_ver_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_Scaleup_Ver_reg);
	pqmask_d_pqmask_icm_scaleup_ver_reg.icm_ver_factor = ptr->Ver.Factor;
	IoReg_Write32(PQMASK_D_PQMASK_ICM_Scaleup_Ver_reg, pqmask_d_pqmask_icm_scaleup_ver_reg.regValue);

	pqmask_d_pqmask_icm_scaleup_phase_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_Scaleup_Phase_reg);
	pqmask_d_pqmask_icm_scaleup_phase_reg.icm_hor_ini = ptr->Hor.InitPhase;
	pqmask_d_pqmask_icm_scaleup_phase_reg.icm_ver_ini = ptr->Ver.InitPhase;
	IoReg_Write32(PQMASK_D_PQMASK_ICM_Scaleup_Phase_reg, pqmask_d_pqmask_icm_scaleup_phase_reg.regValue);
}

void drvif_color_set_pqmask_icm_size(PQMASK_IO_SIZE_T *ptr)
{
	pqmask_d_pqmask_icm_intputsize_ctrl_RBUS pqmask_d_pqmask_icm_intputsize_ctrl_reg;
	pqmask_d_pqmask_icm_intputsize_ctrl2_RBUS pqmask_d_pqmask_icm_intputsize_ctrl2_reg;
	pqmask_d_pqmask_icm_outputsize_ctrl_RBUS pqmask_d_pqmask_icm_outputsize_ctrl_reg;
	pqmask_d_pqmask_icm_outputsize_ctrl2_RBUS pqmask_d_pqmask_icm_outputsize_ctrl2_reg;

	pqmask_d_pqmask_icm_intputsize_ctrl_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_IntputSize_Ctrl_reg);
	pqmask_d_pqmask_icm_intputsize_ctrl_reg.icm_in_h_sta = ptr->Input.HorSta;
	pqmask_d_pqmask_icm_intputsize_ctrl_reg.icm_in_width = ptr->Input.Width;
	IoReg_Write32(PQMASK_D_PQMASK_ICM_IntputSize_Ctrl_reg, pqmask_d_pqmask_icm_intputsize_ctrl_reg.regValue);

	pqmask_d_pqmask_icm_intputsize_ctrl2_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_IntputSize_Ctrl2_reg);
	pqmask_d_pqmask_icm_intputsize_ctrl2_reg.icm_in_v_sta = ptr->Input.VerSta;
	pqmask_d_pqmask_icm_intputsize_ctrl2_reg.icm_in_height = ptr->Input.Height;
	IoReg_Write32(PQMASK_D_PQMASK_ICM_IntputSize_Ctrl2_reg, pqmask_d_pqmask_icm_intputsize_ctrl2_reg.regValue);

	pqmask_d_pqmask_icm_outputsize_ctrl_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_OutputSize_Ctrl_reg);
	pqmask_d_pqmask_icm_outputsize_ctrl_reg.icm_out_h_sta = ptr->Output.HorSta;
	pqmask_d_pqmask_icm_outputsize_ctrl_reg.icm_out_width = ptr->Output.Width;
	IoReg_Write32(PQMASK_D_PQMASK_ICM_OutputSize_Ctrl_reg, pqmask_d_pqmask_icm_outputsize_ctrl_reg.regValue);

	pqmask_d_pqmask_icm_outputsize_ctrl2_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_OutputSize_Ctrl2_reg);
	pqmask_d_pqmask_icm_outputsize_ctrl2_reg.icm_out_v_sta = ptr->Output.VerSta;
	pqmask_d_pqmask_icm_outputsize_ctrl2_reg.icm_out_height = ptr->Output.Height;
	IoReg_Write32(PQMASK_D_PQMASK_ICM_OutputSize_Ctrl2_reg, pqmask_d_pqmask_icm_outputsize_ctrl2_reg.regValue);
}

void drvif_color_set_pqmask_icm_dm_window_enable(unsigned char bEnable)
{
	pqmask_d_pqmask_icm_window_sta_RBUS pqmask_d_pqmask_icm_window_sta_reg;
	
	pqmask_d_pqmask_icm_window_sta_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_Window_STA_reg);
	pqmask_d_pqmask_icm_window_sta_reg.icm_window_en = (bEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_D_PQMASK_ICM_Window_STA_reg, pqmask_d_pqmask_icm_window_sta_reg.regValue);
}

void drvif_color_set_pqmask_icm_dm_window(PQMASK_WINDOW_T *ptr)
{
	pqmask_d_pqmask_icm_window_sta_RBUS pqmask_d_pqmask_icm_window_sta_reg;
	pqmask_d_pqmask_icm_window_end_RBUS pqmask_d_pqmask_icm_window_end_reg;
	pqmask_d_pqmask_icm_winow_weight_RBUS pqmask_d_pqmask_icm_winow_weight_reg;

	pqmask_d_pqmask_icm_window_sta_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_Window_STA_reg);
	pqmask_d_pqmask_icm_window_sta_reg.icm_window_x_sta = ptr->HorSta;
	pqmask_d_pqmask_icm_window_sta_reg.icm_window_y_sta = ptr->VerSta;
	IoReg_Write32(PQMASK_D_PQMASK_ICM_Window_STA_reg, pqmask_d_pqmask_icm_window_sta_reg.regValue);

	pqmask_d_pqmask_icm_window_end_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_Window_END_reg);
	pqmask_d_pqmask_icm_window_end_reg.icm_window_x_end = ptr->HorEnd;
	pqmask_d_pqmask_icm_window_end_reg.icm_window_y_end = ptr->VerEnd;
	IoReg_Write32(PQMASK_D_PQMASK_ICM_Window_END_reg, pqmask_d_pqmask_icm_window_end_reg.regValue);

	pqmask_d_pqmask_icm_winow_weight_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_ICM_Winow_weight_reg);
	pqmask_d_pqmask_icm_winow_weight_reg.icm_hoffet_weight = ptr->weight.HueWeight;
	pqmask_d_pqmask_icm_winow_weight_reg.icm_soffet_weight = ptr->weight.SaturationWeight;
	pqmask_d_pqmask_icm_winow_weight_reg.icm_ioffet_weight = ptr->weight.IntensityWeight;
	IoReg_Write32(PQMASK_D_PQMASK_ICM_Winow_weight_reg, pqmask_d_pqmask_icm_winow_weight_reg.regValue);
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
	ptr->weight.HueWeight = pqmask_d_pqmask_icm_winow_weight_reg.icm_hoffet_weight;
	ptr->weight.SaturationWeight = pqmask_d_pqmask_icm_winow_weight_reg.icm_soffet_weight;
	ptr->weight.IntensityWeight = pqmask_d_pqmask_icm_winow_weight_reg.icm_ioffet_weight;
}

#if 0
/*D DOMAIN LOCALCONTRAST DMA SET FUNCTIONS*/
#endif

void drvif_color_set_pqmask_local_contrast_DMA_config(PQMASK_MODULE_DMA_CONFIG_T *pModuleConfig)
{
	pqmask_d_pqmask_lc_dma_line_step_RBUS pqmask_d_pqmask_lc_dma_line_step_reg;
	pqmask_d_pqmask_dma_en_lc_RBUS pqmask_d_pqmask_dma_en_lc_reg;

	pqmask_d_pqmask_lc_dma_line_step_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_DMA_Line_Step_reg);
	pqmask_d_pqmask_lc_dma_line_step_reg.dma_lc_line_step = pModuleConfig->LineStep;
	IoReg_Write32(PQMASK_D_PQMASK_LC_DMA_Line_Step_reg, pqmask_d_pqmask_lc_dma_line_step_reg.regValue);

	pqmask_d_pqmask_dma_en_lc_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_lc_reg);
	pqmask_d_pqmask_dma_en_lc_reg.data_burst_length_lc = pModuleConfig->DataBurstLen;
	IoReg_Write32(PQMASK_D_PQMASK_DMA_en_lc_reg, pqmask_d_pqmask_dma_en_lc_reg.regValue);
}

void drvif_color_set_pqmask_local_contrast_Addr(unsigned int LocalContrastAddr)
{
	pqmask_d_pqmask_lc_dma_start_addr_RBUS pqmask_d_pqmask_lc_dma_start_addr_reg;

	pqmask_d_pqmask_lc_dma_start_addr_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_DMA_Start_Addr_reg);
	pqmask_d_pqmask_lc_dma_start_addr_reg.dma_start_addr_lc = LocalContrastAddr;
	IoReg_Write32(PQMASK_D_PQMASK_LC_DMA_Start_Addr_reg, pqmask_d_pqmask_lc_dma_start_addr_reg.regValue);
}

void drvif_color_set_pqmask_local_contrast_enable(unsigned char bLocalContrastEnable)
{
	pqmask_d_pqmask_dma_en_lc_RBUS pqmask_d_pqmask_dma_en_lc_reg;

	pqmask_d_pqmask_dma_en_lc_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_lc_reg);
	pqmask_d_pqmask_dma_en_lc_reg.lc_pqmask_en = bLocalContrastEnable;
	IoReg_Write32(PQMASK_D_PQMASK_DMA_en_lc_reg, pqmask_d_pqmask_dma_en_lc_reg.regValue);
}

#if 0
/*D DOMAIN LOCALCONTRAST DMA GET FUNCTIONS*/
#endif

void drvif_color_get_pqmask_local_contrast_DMA_config(PQMASK_MODULE_DMA_CONFIG_T *pModuleConfig)
{
	pqmask_d_pqmask_lc_dma_line_step_RBUS pqmask_d_pqmask_lc_dma_line_step_reg;
	pqmask_d_pqmask_dma_en_lc_RBUS pqmask_d_pqmask_dma_en_lc_reg;

	pqmask_d_pqmask_lc_dma_line_step_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_DMA_Line_Step_reg);
	pModuleConfig->LineStep = pqmask_d_pqmask_lc_dma_line_step_reg.dma_lc_line_step;

	pqmask_d_pqmask_dma_en_lc_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_lc_reg);
	pModuleConfig->DataBurstLen = pqmask_d_pqmask_dma_en_lc_reg.data_burst_length_lc;
}

void drvif_color_get_pqmask_local_contrast_Addr(unsigned int *LocalContrastAddr)
{
	pqmask_d_pqmask_lc_dma_start_addr_RBUS pqmask_d_pqmask_lc_dma_start_addr_reg;

	pqmask_d_pqmask_lc_dma_start_addr_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_DMA_Start_Addr_reg);
	*LocalContrastAddr = pqmask_d_pqmask_lc_dma_start_addr_reg.dma_start_addr_lc;
}

void drvif_color_get_pqmask_local_contrast_enable(unsigned char *bLocalContrastEnable)
{
	pqmask_d_pqmask_dma_en_lc_RBUS pqmask_d_pqmask_dma_en_lc_reg;

	pqmask_d_pqmask_dma_en_lc_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_DMA_en_lc_reg);
	*bLocalContrastEnable = pqmask_d_pqmask_dma_en_lc_reg.lc_pqmask_en;
}

#if 0
/*D DOMAIN LOCALCONTRAST SET FUNCTIONS*/
#endif

void drvif_color_set_pqmask_local_contrast_scaling_enable(unsigned char bHorEnable, unsigned char bVerEnable)
{
	pqmask_d_pqmask_lc_scaleup_hor_RBUS pqmask_d_pqmask_lc_scaleup_hor_reg;
	pqmask_d_pqmask_lc_scaleup_ver_RBUS pqmask_d_pqmask_lc_scaleup_ver_reg;

	pqmask_d_pqmask_lc_scaleup_hor_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_Scaleup_Hor_reg);
	pqmask_d_pqmask_lc_scaleup_hor_reg.lc_h_zoom_en = (bHorEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_D_PQMASK_LC_Scaleup_Hor_reg, pqmask_d_pqmask_lc_scaleup_hor_reg.regValue);

	pqmask_d_pqmask_lc_scaleup_ver_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_Scaleup_Ver_reg);
	pqmask_d_pqmask_lc_scaleup_ver_reg.lc_v_zoom_en = (bVerEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_D_PQMASK_LC_Scaleup_Ver_reg, pqmask_d_pqmask_lc_scaleup_ver_reg.regValue);
}

void drvif_color_set_pqmask_local_contrast_scaling(PQMASK_SACLING_T *ptr)
{
	pqmask_d_pqmask_lc_scaleup_hor_RBUS pqmask_d_pqmask_lc_scaleup_hor_reg;
	pqmask_d_pqmask_lc_scaleup_ver_RBUS pqmask_d_pqmask_lc_scaleup_ver_reg;
	pqmask_d_pqmask_lc_scaleup_phase_RBUS pqmask_d_pqmask_lc_scaleup_phase_reg;

	pqmask_d_pqmask_lc_scaleup_hor_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_Scaleup_Hor_reg);
	pqmask_d_pqmask_lc_scaleup_hor_reg.lc_hor_factor = ptr->Hor.Factor;
	IoReg_Write32(PQMASK_D_PQMASK_LC_Scaleup_Hor_reg, pqmask_d_pqmask_lc_scaleup_hor_reg.regValue);

	pqmask_d_pqmask_lc_scaleup_ver_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_Scaleup_Ver_reg);
	pqmask_d_pqmask_lc_scaleup_ver_reg.lc_ver_factor = ptr->Ver.Factor;
	IoReg_Write32(PQMASK_D_PQMASK_LC_Scaleup_Ver_reg, pqmask_d_pqmask_lc_scaleup_ver_reg.regValue);

	pqmask_d_pqmask_lc_scaleup_phase_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_Scaleup_Phase_reg);
	pqmask_d_pqmask_lc_scaleup_phase_reg.lc_hor_ini = ptr->Hor.InitPhase;
	pqmask_d_pqmask_lc_scaleup_phase_reg.lc_ver_ini = ptr->Ver.InitPhase;
	IoReg_Write32(PQMASK_D_PQMASK_LC_Scaleup_Phase_reg, pqmask_d_pqmask_lc_scaleup_phase_reg.regValue);
}

void drvif_color_set_pqmask_local_contrast_size(PQMASK_IO_SIZE_T *ptr)
{
	pqmask_d_pqmask_lc_intputsize_ctrl_RBUS pqmask_d_pqmask_lc_intputsize_ctrl_reg;
	pqmask_d_pqmask_lc_intputsize_ctrl2_RBUS pqmask_d_pqmask_lc_intputsize_ctrl2_reg;
	pqmask_d_pqmask_lc_outputsize_ctrl_RBUS pqmask_d_pqmask_lc_outputsize_ctrl_reg;
	pqmask_d_pqmask_lc_outputsize_ctrl2_RBUS pqmask_d_pqmask_lc_outputsize_ctrl2_reg;

	pqmask_d_pqmask_lc_intputsize_ctrl_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_IntputSize_Ctrl_reg);
	pqmask_d_pqmask_lc_intputsize_ctrl_reg.lc_in_h_sta = ptr->Input.HorSta;
	pqmask_d_pqmask_lc_intputsize_ctrl_reg.lc_in_width = ptr->Input.Width;
	IoReg_Write32(PQMASK_D_PQMASK_LC_IntputSize_Ctrl_reg, pqmask_d_pqmask_lc_intputsize_ctrl_reg.regValue);

	pqmask_d_pqmask_lc_intputsize_ctrl2_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_IntputSize_Ctrl2_reg);
	pqmask_d_pqmask_lc_intputsize_ctrl2_reg.lc_in_v_sta = ptr->Input.VerSta;
	pqmask_d_pqmask_lc_intputsize_ctrl2_reg.lc_in_height = ptr->Input.Height;
	IoReg_Write32(PQMASK_D_PQMASK_LC_IntputSize_Ctrl2_reg, pqmask_d_pqmask_lc_intputsize_ctrl2_reg.regValue);

	pqmask_d_pqmask_lc_outputsize_ctrl_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_OutputSize_Ctrl_reg);
	pqmask_d_pqmask_lc_outputsize_ctrl_reg.lc_out_h_sta = ptr->Output.HorSta;
	pqmask_d_pqmask_lc_outputsize_ctrl_reg.lc_out_width = ptr->Output.Width;
	IoReg_Write32(PQMASK_D_PQMASK_LC_OutputSize_Ctrl_reg, pqmask_d_pqmask_lc_outputsize_ctrl_reg.regValue);

	pqmask_d_pqmask_lc_outputsize_ctrl2_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_OutputSize_Ctrl2_reg);
	pqmask_d_pqmask_lc_outputsize_ctrl2_reg.lc_out_v_sta = ptr->Output.VerSta;
	pqmask_d_pqmask_lc_outputsize_ctrl2_reg.lc_out_height = ptr->Output.Height;
	IoReg_Write32(PQMASK_D_PQMASK_LC_OutputSize_Ctrl2_reg, pqmask_d_pqmask_lc_outputsize_ctrl2_reg.regValue);
}

void drvif_color_set_pqmask_local_contrast_dm_window_enable(unsigned char bEnable)
{
	pqmask_d_pqmask_lc_window_sta_RBUS pqmask_d_pqmask_lc_window_sta_reg;
	
	pqmask_d_pqmask_lc_window_sta_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_Window_STA_reg);
	pqmask_d_pqmask_lc_window_sta_reg.lc_window_en = (bEnable != 0)? 1 : 0;
	IoReg_Write32(PQMASK_D_PQMASK_LC_Window_STA_reg, pqmask_d_pqmask_lc_window_sta_reg.regValue);
}

void drvif_color_set_pqmask_local_contrast_dm_window(PQMASK_WINDOW_T *ptr)
{
	pqmask_d_pqmask_lc_window_sta_RBUS pqmask_d_pqmask_lc_window_sta_reg;
	pqmask_d_pqmask_lc_window_end_RBUS pqmask_d_pqmask_lc_window_end_reg;
	pqmask_d_pqmask_lc_winow_weight_RBUS pqmask_d_pqmask_lc_winow_weight_reg;

	pqmask_d_pqmask_lc_window_sta_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_Window_STA_reg);
	pqmask_d_pqmask_lc_window_sta_reg.lc_window_x_sta = ptr->HorSta;
	pqmask_d_pqmask_lc_window_sta_reg.lc_window_y_sta = ptr->VerSta;
	IoReg_Write32(PQMASK_D_PQMASK_LC_Window_STA_reg, pqmask_d_pqmask_lc_window_sta_reg.regValue);

	pqmask_d_pqmask_lc_window_end_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_Window_END_reg);
	pqmask_d_pqmask_lc_window_end_reg.lc_window_x_end = ptr->HorEnd;
	pqmask_d_pqmask_lc_window_end_reg.lc_window_y_end = ptr->VerEnd;
	IoReg_Write32(PQMASK_D_PQMASK_LC_Window_END_reg, pqmask_d_pqmask_lc_window_end_reg.regValue);

	pqmask_d_pqmask_lc_winow_weight_reg.regValue = IoReg_Read32(PQMASK_D_PQMASK_LC_Winow_weight_reg);
	pqmask_d_pqmask_lc_winow_weight_reg.lc_weight = ptr->weight.LocalContrastWeight;
	IoReg_Write32(PQMASK_D_PQMASK_LC_Winow_weight_reg, pqmask_d_pqmask_lc_winow_weight_reg.regValue);
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


