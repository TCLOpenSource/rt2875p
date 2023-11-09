/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2020
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for color related functions.
 *
 * @author 		Willy Hsu
 * @date 		2020/03/04
 * @version 	1
 */

/*============================ Module dependency  ===========================*/
#ifdef VIP_SUPPORT_Freq_Detection

#include "rtk_vip_logger.h"
#include <rtd_log/rtd_module_log.h>

#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/vip/frequency_detection.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/color.h>
#include <rbus/frequencydetection_reg.h>

/*================================ Global Variables ==============================*/
/*================================ Definitions ==============================*/
#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_PCID_DEBUG,fmt,##args)

/*================================== Function ===============================*/
void drvif_color_set_frequency_detection_enable(unsigned char bEnable)
{
	frequencydetection_fd_h_filter21_0_2_RBUS frequencydetection_fd_h_filter21_0_2_reg;
	frequencydetection_fd_h_filter21_0_2_reg.regValue = IoReg_Read32(FREQUENCYDETECTION_FD_H_FILTER21_0_2_reg);
	frequencydetection_fd_h_filter21_0_2_reg.fd_en = ((bEnable!=0)? 1 : 0);
	IoReg_Write32(FREQUENCYDETECTION_FD_H_FILTER21_0_2_reg, frequencydetection_fd_h_filter21_0_2_reg.regValue);
}

void drvif_color_set_set_frequency_detection_region(DRV_FreqDet_Region_t* pRgn)
{
	frequencydetection_fd_h_filter_start_RBUS frequencydetection_fd_h_filter_start_reg;
	frequencydetection_fd_h_filter_size_RBUS frequencydetection_fd_h_filter_size_reg;

	frequencydetection_fd_h_filter_start_reg.regValue = IoReg_Read32(FREQUENCYDETECTION_FD_H_FILTER_START_reg);
	frequencydetection_fd_h_filter_size_reg.regValue = IoReg_Read32(FREQUENCYDETECTION_FD_H_FILTER_SIZE_reg);

	frequencydetection_fd_h_filter_start_reg.fd_h_filter_h_start = pRgn->HSta;
	frequencydetection_fd_h_filter_start_reg.fd_h_filter_v_start = pRgn->VSta;

	frequencydetection_fd_h_filter_size_reg.fd_h_filter_h_size = pRgn->Width;
	frequencydetection_fd_h_filter_size_reg.fd_h_filter_v_size = pRgn->Height;

	IoReg_Write32(FREQUENCYDETECTION_FD_H_FILTER_START_reg, frequencydetection_fd_h_filter_start_reg.regValue);
	IoReg_Write32(FREQUENCYDETECTION_FD_H_FILTER_SIZE_reg, frequencydetection_fd_h_filter_size_reg.regValue);
}

void drvif_color_set_set_frequency_detection_filter(DRV_FreqDet_Filter_t* pFlt, DRV_FreqDet_FilterType_t FilterSel)
{
	frequencydetection_fd_h_filter_div_RBUS frequencydetection_fd_h_filter_div_reg;
	frequencydetection_fd_h_filter_switch_RBUS frequencydetection_fd_h_filter_switch_reg;
	frequencydetection_fd_bitshift_RBUS frequencydetection_fd_bitshift_reg;

	frequencydetection_fd_h_filter21_0_2_RBUS frequencydetection_fd_h_filter21_0_2_reg;
	frequencydetection_fd_h_filter21_3_5_RBUS frequencydetection_fd_h_filter21_3_5_reg;
	frequencydetection_fd_h_filter21_6_8_RBUS frequencydetection_fd_h_filter21_6_8_reg;
	frequencydetection_fd_h_filter21_9_10_RBUS frequencydetection_fd_h_filter21_9_10_reg;

	frequencydetection_fd_h_filter15_1_0_3_RBUS frequencydetection_fd_h_filter15_1_0_3_reg;
	frequencydetection_fd_h_filter15_1_4_7_RBUS frequencydetection_fd_h_filter15_1_4_7_reg;
	frequencydetection_fd_h_filter15_2_0_3_RBUS frequencydetection_fd_h_filter15_2_0_3_reg;
	frequencydetection_fd_h_filter15_2_4_7_RBUS frequencydetection_fd_h_filter15_2_4_7_reg;
	frequencydetection_fd_h_filter15_3_0_3_RBUS frequencydetection_fd_h_filter15_3_0_3_reg;
	frequencydetection_fd_h_filter15_3_4_7_RBUS frequencydetection_fd_h_filter15_3_4_7_reg;

	frequencydetection_fd_h_filter_div_reg.regValue = IoReg_Read32(FREQUENCYDETECTION_FD_H_FILTER_DIV_reg);
	frequencydetection_fd_h_filter_switch_reg.regValue = IoReg_Read32(FREQUENCYDETECTION_FD_H_FILTER_SWITCH_reg);
	frequencydetection_fd_bitshift_reg.regValue = IoReg_Read32(FREQUENCYDETECTION_FD_BITSHIFT_reg);

	frequencydetection_fd_h_filter21_0_2_reg.regValue = IoReg_Read32(FREQUENCYDETECTION_FD_H_FILTER21_0_2_reg);
	frequencydetection_fd_h_filter21_3_5_reg.regValue = IoReg_Read32(FREQUENCYDETECTION_FD_H_FILTER21_3_5_reg);
	frequencydetection_fd_h_filter21_6_8_reg.regValue = IoReg_Read32(FREQUENCYDETECTION_FD_H_FILTER21_6_8_reg);
	frequencydetection_fd_h_filter21_9_10_reg.regValue = IoReg_Read32(FREQUENCYDETECTION_FD_H_FILTER21_9_10_reg);

	frequencydetection_fd_h_filter15_1_0_3_reg.regValue = IoReg_Read32(FREQUENCYDETECTION_FD_H_FILTER15_1_0_3_reg);
	frequencydetection_fd_h_filter15_1_4_7_reg.regValue = IoReg_Read32(FREQUENCYDETECTION_FD_H_FILTER15_1_4_7_reg);
	frequencydetection_fd_h_filter15_2_0_3_reg.regValue = IoReg_Read32(FREQUENCYDETECTION_FD_H_FILTER15_2_0_3_reg);
	frequencydetection_fd_h_filter15_2_4_7_reg.regValue = IoReg_Read32(FREQUENCYDETECTION_FD_H_FILTER15_2_4_7_reg);
	frequencydetection_fd_h_filter15_3_0_3_reg.regValue = IoReg_Read32(FREQUENCYDETECTION_FD_H_FILTER15_3_0_3_reg);
	frequencydetection_fd_h_filter15_3_4_7_reg.regValue = IoReg_Read32(FREQUENCYDETECTION_FD_H_FILTER15_3_4_7_reg);

	switch(FilterSel)
	{
		case _FD_FLT_TAP21_:
			frequencydetection_fd_h_filter_div_reg.fd_h_filter_band0_div = pFlt->Div;
			frequencydetection_fd_h_filter_switch_reg.fd_h_filter_switch_0 = pFlt->Switch;
			frequencydetection_fd_bitshift_reg.fd_bitshift_a0 = pFlt->BitSft_A;
			frequencydetection_fd_bitshift_reg.fd_bitshift_b0 = pFlt->BitSft_B;
			frequencydetection_fd_h_filter21_0_2_reg.fd_h_filter_band0_coef0   = pFlt->Coef[0];
			frequencydetection_fd_h_filter21_0_2_reg.fd_h_filter_band0_coef1   = pFlt->Coef[1];
			frequencydetection_fd_h_filter21_0_2_reg.fd_h_filter_band0_coef2   = pFlt->Coef[2];
			frequencydetection_fd_h_filter21_3_5_reg.fd_h_filter_band0_coef3   = pFlt->Coef[3];
			frequencydetection_fd_h_filter21_3_5_reg.fd_h_filter_band0_coef4   = pFlt->Coef[4];
			frequencydetection_fd_h_filter21_3_5_reg.fd_h_filter_band0_coef5   = pFlt->Coef[5];
			frequencydetection_fd_h_filter21_6_8_reg.fd_h_filter_band0_coef6   = pFlt->Coef[6];
			frequencydetection_fd_h_filter21_6_8_reg.fd_h_filter_band0_coef7   = pFlt->Coef[7];
			frequencydetection_fd_h_filter21_6_8_reg.fd_h_filter_band0_coef8   = pFlt->Coef[8];
			frequencydetection_fd_h_filter21_9_10_reg.fd_h_filter_band0_coef9  = pFlt->Coef[9];
			frequencydetection_fd_h_filter21_9_10_reg.fd_h_filter_band0_coef10 = pFlt->Coef[10];
			break;
		case _FD_FLT_TAP15_1:
			frequencydetection_fd_h_filter_div_reg.fd_h_filter_band1_div = pFlt->Div;
			frequencydetection_fd_h_filter_switch_reg.fd_h_filter_switch_1 = pFlt->Switch;
			frequencydetection_fd_bitshift_reg.fd_bitshift_a1 = pFlt->BitSft_A;
			frequencydetection_fd_bitshift_reg.fd_bitshift_b1 = pFlt->BitSft_B;
			frequencydetection_fd_h_filter15_1_0_3_reg.fd_h_filter_band1_coef0 = pFlt->Coef[0];
			frequencydetection_fd_h_filter15_1_0_3_reg.fd_h_filter_band1_coef1 = pFlt->Coef[1];
			frequencydetection_fd_h_filter15_1_0_3_reg.fd_h_filter_band1_coef2 = pFlt->Coef[2];
			frequencydetection_fd_h_filter15_1_0_3_reg.fd_h_filter_band1_coef3 = pFlt->Coef[3];
			frequencydetection_fd_h_filter15_1_4_7_reg.fd_h_filter_band1_coef4 = pFlt->Coef[4];
			frequencydetection_fd_h_filter15_1_4_7_reg.fd_h_filter_band1_coef5 = pFlt->Coef[5];
			frequencydetection_fd_h_filter15_1_4_7_reg.fd_h_filter_band1_coef6 = pFlt->Coef[6];
			frequencydetection_fd_h_filter15_1_4_7_reg.fd_h_filter_band1_coef7 = pFlt->Coef[7];
			break;
		case _FD_FLT_TAP15_2:
			frequencydetection_fd_h_filter_div_reg.fd_h_filter_band2_div = pFlt->Div;
			frequencydetection_fd_h_filter_switch_reg.fd_h_filter_switch_2 = pFlt->Switch;
			frequencydetection_fd_bitshift_reg.fd_bitshift_a2 = pFlt->BitSft_A;
			frequencydetection_fd_bitshift_reg.fd_bitshift_b2 = pFlt->BitSft_B;
			frequencydetection_fd_h_filter15_2_0_3_reg.fd_h_filter_band2_coef0 = pFlt->Coef[0];
			frequencydetection_fd_h_filter15_2_0_3_reg.fd_h_filter_band2_coef1 = pFlt->Coef[1];
			frequencydetection_fd_h_filter15_2_0_3_reg.fd_h_filter_band2_coef2 = pFlt->Coef[2];
			frequencydetection_fd_h_filter15_2_0_3_reg.fd_h_filter_band2_coef3 = pFlt->Coef[3];
			frequencydetection_fd_h_filter15_2_4_7_reg.fd_h_filter_band2_coef4 = pFlt->Coef[4];
			frequencydetection_fd_h_filter15_2_4_7_reg.fd_h_filter_band2_coef5 = pFlt->Coef[5];
			frequencydetection_fd_h_filter15_2_4_7_reg.fd_h_filter_band2_coef6 = pFlt->Coef[6];
			frequencydetection_fd_h_filter15_2_4_7_reg.fd_h_filter_band2_coef7 = pFlt->Coef[7];
			break;
		case _FD_FLT_TAP15_3:
			frequencydetection_fd_h_filter_div_reg.fd_h_filter_band3_div = pFlt->Div;
			frequencydetection_fd_h_filter_switch_reg.fd_h_filter_switch_3 = pFlt->Switch;
			frequencydetection_fd_bitshift_reg.fd_bitshift_a3 = pFlt->BitSft_A;
			frequencydetection_fd_bitshift_reg.fd_bitshift_b3 = pFlt->BitSft_B;
			frequencydetection_fd_h_filter15_3_0_3_reg.fd_h_filter_band3_coef0 = pFlt->Coef[0];
			frequencydetection_fd_h_filter15_3_0_3_reg.fd_h_filter_band3_coef1 = pFlt->Coef[1];
			frequencydetection_fd_h_filter15_3_0_3_reg.fd_h_filter_band3_coef2 = pFlt->Coef[2];
			frequencydetection_fd_h_filter15_3_0_3_reg.fd_h_filter_band3_coef3 = pFlt->Coef[3];
			frequencydetection_fd_h_filter15_3_4_7_reg.fd_h_filter_band3_coef4 = pFlt->Coef[4];
			frequencydetection_fd_h_filter15_3_4_7_reg.fd_h_filter_band3_coef5 = pFlt->Coef[5];
			frequencydetection_fd_h_filter15_3_4_7_reg.fd_h_filter_band3_coef6 = pFlt->Coef[6];
			frequencydetection_fd_h_filter15_3_4_7_reg.fd_h_filter_band3_coef7 = pFlt->Coef[7];
			break;
		default:
			break;
	}

	IoReg_Write32(FREQUENCYDETECTION_FD_H_FILTER_DIV_reg, frequencydetection_fd_h_filter_div_reg.regValue);
	IoReg_Write32(FREQUENCYDETECTION_FD_H_FILTER_SWITCH_reg, frequencydetection_fd_h_filter_switch_reg.regValue);
	IoReg_Write32(FREQUENCYDETECTION_FD_BITSHIFT_reg, frequencydetection_fd_bitshift_reg.regValue);

	IoReg_Write32(FREQUENCYDETECTION_FD_H_FILTER21_0_2_reg, frequencydetection_fd_h_filter21_0_2_reg.regValue);
	IoReg_Write32(FREQUENCYDETECTION_FD_H_FILTER21_3_5_reg, frequencydetection_fd_h_filter21_3_5_reg.regValue);
	IoReg_Write32(FREQUENCYDETECTION_FD_H_FILTER21_6_8_reg, frequencydetection_fd_h_filter21_6_8_reg.regValue);
	IoReg_Write32(FREQUENCYDETECTION_FD_H_FILTER21_9_10_reg, frequencydetection_fd_h_filter21_9_10_reg.regValue);

	IoReg_Write32(FREQUENCYDETECTION_FD_H_FILTER15_1_0_3_reg, frequencydetection_fd_h_filter15_1_0_3_reg.regValue);
	IoReg_Write32(FREQUENCYDETECTION_FD_H_FILTER15_1_4_7_reg, frequencydetection_fd_h_filter15_1_4_7_reg.regValue);
	IoReg_Write32(FREQUENCYDETECTION_FD_H_FILTER15_2_0_3_reg, frequencydetection_fd_h_filter15_2_0_3_reg.regValue);
	IoReg_Write32(FREQUENCYDETECTION_FD_H_FILTER15_2_4_7_reg, frequencydetection_fd_h_filter15_2_4_7_reg.regValue);
	IoReg_Write32(FREQUENCYDETECTION_FD_H_FILTER15_3_0_3_reg, frequencydetection_fd_h_filter15_3_0_3_reg.regValue);
	IoReg_Write32(FREQUENCYDETECTION_FD_H_FILTER15_3_4_7_reg, frequencydetection_fd_h_filter15_3_4_7_reg.regValue);
}

void drvif_color_get_ferquency_detection_histogram(unsigned int *pHist, DRV_FreqDet_FilterType_t FilterSel, DRV_FreqDet_HistType_t TypeSel)
{
	unsigned int i=0;
	frequencydetection_fd_h_filter21_0_2_RBUS frequencydetection_fd_h_filter21_0_2_reg;

	// lock the histogram result
	frequencydetection_fd_h_filter21_0_2_reg.regValue = IoReg_Read32(FREQUENCYDETECTION_FD_H_FILTER21_0_2_reg);
	frequencydetection_fd_h_filter21_0_2_reg.lock = 1;
	IoReg_Write32(FREQUENCYDETECTION_FD_H_FILTER21_0_2_reg, frequencydetection_fd_h_filter21_0_2_reg.regValue);

	if(FilterSel == _FD_FLT_TAP21_)
	{
		if(TypeSel == _FD_HIST_COMP )
		{
			for(i=0; i<16; i++)
			{
				pHist[i] = (IoReg_Read32(FREQUENCYDETECTION_FD_A0_HIST0_0_reg+i*4) & 0xffffff);
			}
		}
		else
		{
			for(i=0; i<16; i++)
			{
				pHist[i] = (IoReg_Read32(FREQUENCYDETECTION_FD_B0_HIST0_0_reg+i*4) & 0xffffff);
			}
		}
	}
	else if(FilterSel == _FD_FLT_TAP15_1)
	{
		if(TypeSel == _FD_HIST_COMP )
		{
			for(i=0; i<16; i++)
			{
				pHist[i] = (IoReg_Read32(FREQUENCYDETECTION_FD_A1_HIST1_0_reg+i*4) & 0xffffff);
			}
		}
		else
		{
			for(i=0; i<16; i++)
			{
				pHist[i] = (IoReg_Read32(FREQUENCYDETECTION_FD_B1_HIST1_0_reg+i*4) & 0xffffff);
			}
		}
	}
	else if(FilterSel == _FD_FLT_TAP15_2)
	{
		if(TypeSel == _FD_HIST_COMP )
		{
			for(i=0; i<16; i++)
			{
				pHist[i] = (IoReg_Read32(FREQUENCYDETECTION_FD_A2_HIST2_0_reg+i*4) & 0xffffff);
			}
		}
		else
		{
			for(i=0; i<16; i++)
			{
				pHist[i] = (IoReg_Read32(FREQUENCYDETECTION_FD_B2_HIST2_0_reg+i*4) & 0xffffff);
			}
		}
	}
	else
	{
		if(TypeSel == _FD_HIST_COMP )
		{
			for(i=0; i<16; i++)
			{
				pHist[i] = (IoReg_Read32(FREQUENCYDETECTION_FD_A3_HIST3_0_reg+i*4) & 0xffffff);
			}
		}
		else
		{
			for(i=0; i<16; i++)
			{
				pHist[i] = (IoReg_Read32(FREQUENCYDETECTION_FD_B3_HIST3_0_reg+i*4) & 0xffffff);
			}
		}
	}


	// release the histogram result
	frequencydetection_fd_h_filter21_0_2_reg.regValue = IoReg_Read32(FREQUENCYDETECTION_FD_H_FILTER21_0_2_reg);
	frequencydetection_fd_h_filter21_0_2_reg.lock = 0;
	IoReg_Write32(FREQUENCYDETECTION_FD_H_FILTER21_0_2_reg, frequencydetection_fd_h_filter21_0_2_reg.regValue);
}
#endif
