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
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/vip/film.h>
#include <tvscalercontrol/vip/scalerColor.h>

#include <rtd_log/rtd_module_log.h>

/*===================================  Types ================================*/

/*================================== Variables ==============================*/
extern unsigned char fw_film_Table_Idx;
extern unsigned char fw_film_Timing;
extern unsigned char fw_film_is_dtv;

/*================================ Definitions ==============================*/



#if 0

 unsigned char  tFILM_INIT[] = {
	0x04, 0x00, 0xff, 0x02,            /* select page 2*/
	/* source size*/
	0x04, 0x00, RSPI_GETADDRESS(_P2_00_ACTIVEWINDOWCTRL_MSB), 0xb0,
	0x04, 0x00, RSPI_GETADDRESS(_P2_01_ACTIVEWINDOWCTRL_H_LSB), 0xd0,
	0x04, 0x00, RSPI_GETADDRESS(_P2_02_ACTIVEWINDOWCTRL_V_LSB), 0xf0,
	0x04, 0x00, RSPI_GETADDRESS(_P2_03_ACTIVEWINDOWCTRL_B_LSB), 0x8a,

	/* film control*/
	0x04, 0x00, RSPI_GETADDRESS(_P2_13_IP_FILM_MODE), 0xbf,
	/* film threshold*/
	0x04, 0x00, RSPI_GETADDRESS(_P2_79_), 0x10,
	0x04, 0x00, RSPI_GETADDRESS(_P2_7A_), 0x10,
	0x04, 0x00, RSPI_GETADDRESS(_P2_7B_), 0x15,
	0x04, 0x00, RSPI_GETADDRESS(_P2_7C_), 0x14,
	0x04, 0x00, RSPI_GETADDRESS(_P2_7D_), 0x78,
	0x04, 0x00, RSPI_GETADDRESS(_P2_7E_), 0x13,
	0x04, 0x00, RSPI_GETADDRESS(_P2_7F_), 0x88,
	0x04, 0x00, RSPI_GETADDRESS(_P2_80_), 0x0f,
	0x04, 0x00, RSPI_GETADDRESS(_P2_81_), 0x14,
	0x04, 0x00, RSPI_GETADDRESS(_P2_82_), 0x32,
	0x04, 0x00, RSPI_GETADDRESS(_P2_83_), 0x07,
	0x04, 0x00, RSPI_GETADDRESS(_P2_84_), 0xd0,
	0
  };



/*=====================Init Table===================*/

UINT32 film32_init_table_TSB[26][2] = {
	/* SD		HD*/
	{	    2,       2  	},	/*Film_sawtooth_choose*/
	{	    0,       0  	},	/*Film LeaveQuick en*/
	{	    1,       1  	},	/*Film_VerScrollingText_en*/
	{	    0,       0         },	/*Film_thumbnail_en*/
	{	    1,       1  	},	/*Film_Pair1_en*/
	{	    0,       0  	},	/*Film_Pair2_en*/
	{	    0,       0  	},	/*Film_Pair_en_auto*/
	{	    1,       1  	},	/*Film_Violate_strict_en*/
	{	   22,     56	},	/*Film_SawtoothThl*/
	{	    18,      42	},	/*Film_FrMotionThl*/
	{	   40,     40	},	/*Film_FieldMotionSumThl*/
	{	   21,     21 	},	/*Film_FrStaticSThl*/
	{	   20,     20	},	/*Film_FiStaticSThl*/
	{	     8,       8	},	/*Film_PairRatio_all1*/
	{ 	     8,       8	},	/*Film_PairRatio_all2*/
	{	 120,     120	},	/*Film_FiMinSThl*/
	{	5000,   5000      },	/*Film_FiMaxSThl*/
	{	   40,       40	},	/*Film_SubFieldMotionSumThl*/
	{	   15,	       15     },	/*Film_SubFrStaticsSTh1*/
	{	   20,	       20     },	/*Film_SubFiStaticsSTh1*/
	{	     8,	        8      },	/*Film_PairRatio_sub1*/
	{	     8,	        8      },	/*Film_PairRatio_sub2*/
	{	   50,	      50      },	/*Film_SubFiMinSTh1*/
	{	2000,    2000    },	/*	Film_SubFiMaxSTh1*/
	{	   50,	       50     },	/*Film_SubFiMinSTh1*/
	{	    0,	        0      },	/*Film32_debounce_FieldNum*/
};

UINT32 film22_init_table_TSB[28][2] = {
	/* SD		HD*/
	{	2,		2	},	/*Film_sawtooth_choose*/
	{	0,		0	},	/*Film LeaveQuick en*/
	{	1,		1	},	/*Film_VerScrollingText_en*/
	{	0,		0	},	/*Film_thumbnail_en*/
	{	1,		1	},	/*Film_Pair1_en*/
	{	0,		0	},	/*Film_Pair2_en*/
	{	0,		0	},	/*Film_Pair_en_auto*/
	{	1,		1	},	/*Film_Violate_strict_en*/
	{	60,		60	},	/*Film22_SawtoothTh1*/
	{	22,		26	},	/*Film_SawtoothThl*/
	{	16,		16	},	/*Film_FrMotionThl*/
	{	40,		40	},	/*Film_FieldMotionSumThl*/
	{	20,		20	},	/*Film22_FiStaticSTh1*/
	{	21,		21	},	/*Film_FrStaticSThl*/
	{	8,		8	},	/*Film_PairRatio_all1*/
	{	8,		8	},	/*Film_PairRatio_all2*/
	{	120,		120	},	/*Film_FiMinSThl*/
	{	5000,	5000},	/*Film_FiMaxSThl*/
	{	40,		40	},	/*Film_SubFieldMotionSumThl*/
	{	20,		20	},	/*Film_SubFiStaticSTh1*/
	{	15,		15	},	/*Film_SubFrStaticsSTh1*/
	{	8,		8	},	/*Film_PairRatio_sub1*/
	{	8,		8	},	/*Film_PairRatio_sub2*/
	{	50,		50	},	/*Film_SubFiMinSTh1*/
	{	2000,	2000},	/*Film_SubFiMaxSTh1*/
	{	4,		4	},	/*Film22_debounce_FieldNum*/
	{	0,		0	},	/*Film22_SubFieldMotionSumThl*/
	{	40,		40	},	/*Film22_FieldMotionSumThl*/
};

UINT32 film32_init_table[27][2] = {
	/* SD		HD*/
	{	    2,       2  	},	/*Film_sawtooth_choose*/
	{	    0,       0  	},	/*Film LeaveQuick en*/
	{	    1,       1  	},	/*Film_VerScrollingText_en*/
	{	    0,       0         },	/*Film_thumbnail_en*/
	{	    1,       1  	},	/*Film_Pair1_en*/
	{	    0,       0  	},	/*Film_Pair2_en*/
	{	    0,       0  	},	/*Film_Pair_en_auto*/
	{	    1,       1  	},	/*Film_Violate_strict_en*/
	{	   22,     56	},	/*Film_SawtoothThl*/
	{	    18,      42	},	/*Film_FrMotionThl*/
	{	   40,     40	},	/*Film_FieldMotionSumThl*/
	{	   21,     21 	},	/*Film_FrStaticSThl*/
	{	   20,     20	},	/*Film_FiStaticSThl*/
	{	     8,       8	},	/*Film_PairRatio_all1*/
	{ 	     8,       8	},	/*Film_PairRatio_all2*/
	{	 120,     120	},	/*Film_FiMinSThl*/
	{	5000,   5000      },	/*Film_FiMaxSThl*/
	{	   40,       40	},	/*Film_SubFieldMotionSumThl*/
	{	   15,	       15     },	/*Film_SubFrStaticsSTh1*/
	{	   20,	       20     },	/*Film_SubFiStaticsSTh1*/
	{	     8,	        8      },	/*Film_PairRatio_sub1*/
	{	     8,	        8      },	/*Film_PairRatio_sub2*/
	{	   50,	      50      },	/*Film_SubFiMinSTh1*/
	{	2000,    2000    },	/*	Film_SubFiMaxSTh1*/
	{	   50,	       50     },	/*Film_SubFiMinSTh1*/
	{	    0,	        0      },	/*Film32_debounce_FieldNum*/
	{	1,		0	},	/*fw film enable*/
};

UINT32 film22_init_table[29][2] = {
	/* SD		HD*/
	{	2,		2	},	/*Film_sawtooth_choose*/
	{	0,		0	},	/*Film LeaveQuick en*/
	{	1,		1	},	/*Film_VerScrollingText_en*/
	{	0,		0	},	/*Film_thumbnail_en*/
	{	1,		1	},	/*Film_Pair1_en*/
	{	0,		0	},	/*Film_Pair2_en*/
	{	0,		0	},	/*Film_Pair_en_auto*/
	{	1,		1	},	/*Film_Violate_strict_en*/
	{	60,		60	},	/*Film22_SawtoothTh1*/
	{	22,		26	},	/*Film_SawtoothThl*/
	{	16,		16	},	/*Film_FrMotionThl*/
	{	40,		40	},	/*Film_FieldMotionSumThl*/
	{	20,		20	},	/*Film22_FiStaticSTh1*/
	{	21,		21	},	/*Film_FrStaticSThl*/
	{	8,		8	},	/*Film_PairRatio_all1*/
	{	8,		8	},	/*Film_PairRatio_all2*/
	{	120,		120	},	/*Film_FiMinSThl*/
	{	5000,	5000},	/*Film_FiMaxSThl*/
	{	40,		40	},	/*Film_SubFieldMotionSumThl*/
	{	20,		20	},	/*Film_SubFiStaticSTh1*/
	{	15,		15	},	/*Film_SubFrStaticsSTh1*/
	{	8,		8	},	/*Film_PairRatio_sub1*/
	{	8,		8	},	/*Film_PairRatio_sub2*/
	{	50,		50	},	/*Film_SubFiMinSTh1*/
	{	2000,	2000},	/*Film_SubFiMaxSTh1*/
	{	4,		4	},	/*Film22_debounce_FieldNum*/
	{	0,		0	},	/*Film22_SubFieldMotionSumThl*/
	{	40,		40	},	/*Film22_FieldMotionSumThl*/
	{	1,		0	},	/*fw film enable*/
};
#endif
/*================================== Function ===============================*/

/*o---------------------------------------------------o*/
/*      FILM_init*/
/*      ==> to initialize the FILM setting*/
/*      @param  {none}*/
/*      @return {none}*/
/*o---------------------------------------------------o*/
void drvif_module_film_init(unsigned char display)  /*nick187    many error should be fixed.....*/
{
/*	unsigned short    Width, Height,Blank;*/
	di_im_di_control_RBUS di_control_reg;
	di_im_di_film_sawtooth_filmframe_th_RBUS film_sawtooth_filmframe_th_reg;
	di_im_di_film_static_sum_th_RBUS film_static_sum_th_reg;
	di_im_di_film_field_judge_th_RBUS film_field_judge_th_reg;
	di_im_di_film_static_sum_sub_th_RBUS film_static_sum_sub_th_reg;
	di_im_di_film_field_judge_sub_th_RBUS film_field_judge_sub_th_reg;
	di_im_di_film_teeth_RBUS film_teeth_reg;

	/* fw film setting*/
	di_im_di_film_new_function_main_RBUS film_new_function_main_reg;

/*	Width  = info->CapWid;*/
/*	Height = info->IPV_ACT_LEN;*/
/*	Blank = info->IHTotal - Width;*/
	/* FILM main/sub select*/
	/*tFILM_INIT[3]  = 0x2 + ((info->display)<<3);       */ /* main/sub*/
    /* source size*/
	/*tFILM_INIT[7]  = 0x80 | (Width>>8)<<4 | (Height>>8)<<2 | (Blank >> 8) ;*/
	/*tFILM_INIT[11] = Width & 0xff;*/
	/*tFILM_INIT[15] = Height & 0xff;*/

	/*tFILM_INIT[19] = 0x8a;*/
	/*tFILM_INIT[19] = Blank & 0xff;*/

	/* set register*/
	/*----------> Rspi_CodeW(tFILM_INIT);*/

	di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
/*-->w	di_control_reg.ip_enable = 1;*/
	/*di_control_reg.ma_3aenable = (Width>=960)?1:0;*/
	di_control_reg.film_enable = 0x1f;
	di_control_reg.write_enable_5 =1;

	IoReg_Write32(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);
	film_sawtooth_filmframe_th_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_SAWTOOTH_FILMFRAME_TH_reg);
	film_sawtooth_filmframe_th_reg.film_sawtooththl = 0xc;  /* 20140226 chris modify need to be check*/
	/*film_sawtooth_filmframe_th_reg.film_frmotionthl = 0x10;*/
	film_sawtooth_filmframe_th_reg.film_leavequick_en = 0; /* 20110419 by flora*/
	IoReg_Write32(DI_IM_DI_FILM_SAWTOOTH_FILMFRAME_TH_reg, film_sawtooth_filmframe_th_reg.regValue);

	film_static_sum_th_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_STATIC_SUM_TH_reg);
	film_static_sum_th_reg.film22_fistaticsthl = 0x14; /* for demo*/
	film_static_sum_th_reg.film_frstaticsthl = 0x15;
	film_static_sum_th_reg.film_fistaticsthl = 0x14;
	IoReg_Write32(DI_IM_DI_FILM_STATIC_SUM_TH_reg, film_static_sum_th_reg.regValue);

	film_field_judge_th_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_FIELD_JUDGE_TH_reg);
	film_field_judge_th_reg.film_fiminsthl = 0x78;
	film_field_judge_th_reg.film_fimaxsthl = 0x1388;
	IoReg_Write32(DI_IM_DI_FILM_FIELD_JUDGE_TH_reg, film_field_judge_th_reg.regValue);

	film_static_sum_sub_th_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_STATIC_SUM_SUB_TH_reg);
	film_static_sum_sub_th_reg.film_subfieldmotionsumthl = 0x28;
	film_static_sum_sub_th_reg.film22_subfistaticsthl = 0x14;
	film_static_sum_sub_th_reg.film_subfrstaticsthl = 0x0f;
	film_static_sum_sub_th_reg.film_subfistaticsthl = 0x14;
	IoReg_Write32(DI_IM_DI_FILM_STATIC_SUM_SUB_TH_reg, film_static_sum_sub_th_reg.regValue);


	film_field_judge_sub_th_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_FIELD_JUDGE_SUB_TH_reg);
	film_field_judge_sub_th_reg.film_subfiminsthl = 0x32;
	film_field_judge_sub_th_reg.film_subfimaxsthl = 0x07d0;
	IoReg_Write32(DI_IM_DI_FILM_FIELD_JUDGE_SUB_TH_reg, film_field_judge_sub_th_reg.regValue);

	/* fw film setting*/
	film_new_function_main_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_NEW_FUNCTION_MAIN_reg);
	film_new_function_main_reg.fw_film_en = 0x1;
	film_new_function_main_reg.film_mixedfilm_margin = 0x2;
	film_new_function_main_reg.fw_film_reg_sel = 0x0;
	film_new_function_main_reg.badeditor_ratio = 0x2;
	film_new_function_main_reg.force_badeditor_en = 0x0;
	IoReg_Write32(DI_IM_DI_FILM_NEW_FUNCTION_MAIN_reg, film_new_function_main_reg.regValue);

	/* film mask for 22 sequence */
	{
	di_im_di_film_mask_detection_RBUS film_mask_detection_reg;
	film_mask_detection_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_MASK_DETECTION_reg);
	film_mask_detection_reg.film_mask_en = 1;
	IoReg_Write32(DI_IM_DI_FILM_MASK_DETECTION_reg, film_mask_detection_reg.regValue);
	}

	// Merlin4 teethth
	// film_teeth_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_TEETH_reg);
	film_teeth_reg.film_teethth_en=1;
	film_teeth_reg.film_teethth_teeth_lb=2;
	film_teeth_reg.film_teethth_moving_lb=4;
	film_teeth_reg.film_teethth_prog_lb=4;
	if( fw_film_is_dtv )
	{
		film_teeth_reg.film_teethth_ratio=0;
	}
	else
	{
		film_teeth_reg.film_teethth_ratio=25;
	}
	IoReg_Write32(DI_IM_DI_FILM_TEETH_reg, film_teeth_reg.regValue);

}
/*o---------------------------------------------------o*/
/*      FILM_enable*/
/*      ==> to enable FILM detection function*/
/*      @param <film_sel> { 0 ~ 3, default 3*/
/*                          0  : Disable*/
/*						    1  : 32 film enable*/
/*							2  : 32 + 32sub film enable*/
/*                          3  : 32 + 32sub + 22 film enable  }*/
/*      @return {none}*/
/*o---------------------------------------------------o*/

void drvif_module_film_mode(DRV_film_mode *ptr)
{
	extern FILM_FW_ShareMem *g_Share_Memory_FILM_FW;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = NULL;
	di_im_di_control_RBUS di_control_reg;
	di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
	di_control_reg.film_enable = ptr->film_status;
	di_control_reg.write_enable_5=1;
	IoReg_Write32(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);
	IoReg_SetBits(DI_db_reg_ctl_reg, DI_db_reg_ctl_db_apply_mask);

	if (g_Share_Memory_FILM_FW != NULL){

		g_Share_Memory_FILM_FW->film_en_ap_status = ptr->film_status;
	}

	VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	if(VIP_RPC_system_info_structure_table == NULL){
		pr_debug("[%s:%d] Warning here!! RPC_system_info=NULL! \n",__FILE__, __LINE__);
	}else{
		VIP_RPC_system_info_structure_table->SCPU_ISRIINFO_TO_VCPU.DI_info.DI_film_status = ptr->film_status;
	}
}

int y_film_result[78];
void drvif_module_film_read_result(void)
{
	di_im_di_si_film_32_result_1_a_RBUS film_32_result_1_a_reg;
	di_im_di_si_film_32_result_1_b_RBUS film_32_result_1_b_reg;
	di_im_di_si_film_32_result_2_a_RBUS film_32_result_2_a_reg;
	di_im_di_si_film_32_result_2_b_RBUS film_32_result_2_b_reg;
	di_im_di_si_film_22_result_a_RBUS film_22_result_a_reg;
	di_im_di_si_film_22_result_b_RBUS film_22_result_b_reg;
	di_im_di_si_film_pattern_a_RBUS film_pattern_a_reg;
	di_im_di_si_film_final_result_RBUS film_final_result_reg;

	film_32_result_1_a_reg.regValue = IoReg_Read32(DI_IM_DI_SI_FILM_32_RESULT_1_A_reg);
	film_32_result_1_b_reg.regValue = IoReg_Read32(DI_IM_DI_SI_FILM_32_RESULT_1_B_reg);
	film_32_result_2_a_reg.regValue = IoReg_Read32(DI_IM_DI_SI_FILM_32_RESULT_2_A_reg);
	film_32_result_2_b_reg.regValue = IoReg_Read32(DI_IM_DI_SI_FILM_32_RESULT_2_B_reg);
	film_22_result_a_reg.regValue = IoReg_Read32(DI_IM_DI_SI_FILM_22_RESULT_A_reg);
	film_22_result_b_reg.regValue = IoReg_Read32(DI_IM_DI_SI_FILM_22_RESULT_B_reg);
	film_pattern_a_reg.regValue = IoReg_Read32(DI_IM_DI_SI_FILM_PATTERN_A_reg);
	film_final_result_reg.regValue = IoReg_Read32(DI_IM_DI_SI_FILM_FINAL_RESULT_reg);

	memset(y_film_result,0,sizeof(y_film_result));
	// frame diffsum
	y_film_result[0]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_reg)&0xfffff;
	y_film_result[1]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_H_T_reg)&0xfffff;
	y_film_result[2]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_H_M_reg)&0xfffff;
	y_film_result[3]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_H_B_reg)&0xfffff;
	y_film_result[4]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_V_L_reg)&0xfffff;
	y_film_result[5]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_V_M_reg)&0xfffff;
	y_film_result[6]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_V_R_reg)&0xfffff;
	y_film_result[7]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_C_reg)&0xfffff;
	// prev diffsum
	y_film_result[8]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_PRE_reg)&0xfffff;
	y_film_result[9]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_PRE_H_T_reg)&0xfffff;
	y_film_result[10]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_PRE_H_M_reg)&0xfffff;
	y_film_result[11]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_PRE_H_B_reg)&0xfffff;
	y_film_result[12]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_PRE_V_L_reg)&0xfffff;
	y_film_result[13]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_PRE_V_M_reg)&0xfffff;
	y_film_result[14]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_PRE_V_R_reg)&0xfffff;
	y_film_result[15]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_PRE_C_reg)&0xfffff;
	// next diffsum
	y_film_result[16]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_NEXT_reg)&0xfffff;
	y_film_result[17]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_NEXT_H_T_reg)&0xfffff;
	y_film_result[18]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_NEXT_H_M_reg)&0xfffff;
	y_film_result[19]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_NEXT_H_B_reg)&0xfffff;
	y_film_result[20]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_NEXT_V_L_reg)&0xfffff;
	y_film_result[21]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_NEXT_V_M_reg)&0xfffff;
	y_film_result[22]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_NEXT_V_R_reg)&0xfffff;
	y_film_result[23]=IoReg_Read32(DI_IM_DI_SI_FILM_MOTION_NEXT_C_reg)&0xfffff;
	// 32_1 detected
	y_film_result[24]=(film_32_result_1_a_reg.film_detected321&0x80)>>7;
	y_film_result[25]=(film_32_result_1_a_reg.film_detected321&0x40)>>6;
	y_film_result[26]=(film_32_result_1_a_reg.film_detected321&0x20)>>5;
	y_film_result[27]=(film_32_result_1_a_reg.film_detected321&0x10)>>4;
	y_film_result[28]=(film_32_result_1_a_reg.film_detected321&0x08)>>3;
	y_film_result[29]=(film_32_result_1_a_reg.film_detected321&0x04)>>2;
	y_film_result[30]=(film_32_result_1_a_reg.film_detected321&0x02)>>1;
	y_film_result[31]=(film_32_result_1_a_reg.film_detected321&0x01)>>0;
	// 32_2 detected
	y_film_result[32]=(film_32_result_2_a_reg.film_detected322&0x80)>>7;
	y_film_result[33]=(film_32_result_2_a_reg.film_detected322&0x40)>>6;
	y_film_result[34]=(film_32_result_2_a_reg.film_detected322&0x20)>>5;
	y_film_result[35]=(film_32_result_2_a_reg.film_detected322&0x10)>>4;
	y_film_result[36]=(film_32_result_2_a_reg.film_detected322&0x08)>>3;
	y_film_result[37]=(film_32_result_2_a_reg.film_detected322&0x04)>>2;
	y_film_result[38]=(film_32_result_2_a_reg.film_detected322&0x02)>>1;
	y_film_result[39]=(film_32_result_2_a_reg.film_detected322&0x01)>>0;
	// 22 detected
	y_film_result[40]=(film_22_result_a_reg.film_detected22&0x80)>>7;
	y_film_result[41]=(film_22_result_a_reg.film_detected22&0x40)>>6;
	y_film_result[42]=(film_22_result_a_reg.film_detected22&0x20)>>5;
	y_film_result[43]=(film_22_result_a_reg.film_detected22&0x10)>>4;
	y_film_result[44]=(film_22_result_a_reg.film_detected22&0x08)>>3;
	y_film_result[45]=(film_22_result_a_reg.film_detected22&0x04)>>2;
	y_film_result[46]=(film_22_result_a_reg.film_detected22&0x02)>>1;
	y_film_result[47]=(film_22_result_a_reg.film_detected22&0x01)>>0;
	// 32_1 sequence
	y_film_result[48]=(film_32_result_1_a_reg.film_sequence321_39_32>>3)&0x1f;
	y_film_result[49]=( (film_32_result_1_a_reg.film_sequence321_39_32&0x7)<<2 )|( (film_32_result_1_b_reg.film_sequence321_31_0>>30)&0x3 );
	y_film_result[50]=(film_32_result_1_b_reg.film_sequence321_31_0>>25)&0x1f;
	y_film_result[51]=(film_32_result_1_b_reg.film_sequence321_31_0>>20)&0x1f;
	y_film_result[52]=(film_32_result_1_b_reg.film_sequence321_31_0>>15)&0x1f;
	y_film_result[53]=(film_32_result_1_b_reg.film_sequence321_31_0>>10)&0x1f;
	y_film_result[54]=(film_32_result_1_b_reg.film_sequence321_31_0>>5)&0x1f;
	y_film_result[55]=(film_32_result_1_b_reg.film_sequence321_31_0>>0)&0x1f;
	// 32_2 sequence
	y_film_result[56]=(film_32_result_2_a_reg.film_sequence322_39_32>>3)&0x1f;
	y_film_result[57]=( (film_32_result_2_a_reg.film_sequence322_39_32&0x7)<<2 )|( (film_32_result_2_b_reg.film_sequence322_31_0>>30)&0x3 );
	y_film_result[58]=(film_32_result_2_b_reg.film_sequence322_31_0>>25)&0x1f;
	y_film_result[59]=(film_32_result_2_b_reg.film_sequence322_31_0>>20)&0x1f;
	y_film_result[60]=(film_32_result_2_b_reg.film_sequence322_31_0>>15)&0x1f;
	y_film_result[61]=(film_32_result_2_b_reg.film_sequence322_31_0>>10)&0x1f;
	y_film_result[62]=(film_32_result_2_b_reg.film_sequence322_31_0>>5)&0x1f;
	y_film_result[63]=(film_32_result_2_b_reg.film_sequence322_31_0>>0)&0x1f;
	// 22 sequence
	y_film_result[64]=(film_22_result_a_reg.film_sequence22_39_32>>3)&0x1f;
	y_film_result[65]=( (film_22_result_a_reg.film_sequence22_39_32&0x7)<<2 )|( (film_22_result_b_reg.film_sequence22_31_0>>30)&0x3 );
	y_film_result[66]=(film_22_result_b_reg.film_sequence22_31_0>>25)&0x1f;
	y_film_result[67]=(film_22_result_b_reg.film_sequence22_31_0>>20)&0x1f;
	y_film_result[68]=(film_22_result_b_reg.film_sequence22_31_0>>15)&0x1f;
	y_film_result[69]=(film_22_result_b_reg.film_sequence22_31_0>>10)&0x1f;
	y_film_result[70]=(film_22_result_b_reg.film_sequence22_31_0>>5)&0x1f;
	y_film_result[71]=(film_22_result_b_reg.film_sequence22_31_0>>0)&0x1f;

	y_film_result[72]=(film_pattern_a_reg.film_framestatic>>7)&1;
	y_film_result[73]=(film_pattern_a_reg.film_prefistatic1>>7)&1;
	y_film_result[74]=(film_pattern_a_reg.film_framepair>>0)&3;

	y_film_result[75]=film_final_result_reg.film_detected;
	y_film_result[76]=0;
	y_film_result[77]=film_final_result_reg.film_sequence;
}

extern unsigned int vpq_project_id;
extern unsigned int vpqex_project_id;
void drvif_module_film_setting(DRV_film_table_t2 *ptr)
{
	di_im_di_control_RBUS di_control_reg;
	di_im_di_film_sawtooth_filmframe_th_RBUS film_sawtooth_filmframe_th_reg;
	di_im_di_film_static_sum_th_RBUS film_static_sum_th_reg;
	di_im_di_film_field_judge_th_RBUS film_field_judge_th_reg;
	di_im_di_film_static_sum_sub_th_RBUS film_static_sum_sub_th_reg;
	di_im_di_film_field_judge_sub_th_RBUS film_field_judge_sub_th_reg;
	di_im_di_film_motion_sum_th_RBUS film_motion_sum_th_reg;
	di_im_di_film_new_function_main_RBUS film_new_function_main_reg;	/* fw film setting*/
	di_im_di_film_edge_type_check_RBUS film_edge_type_check_reg;
	di_im_di_film_mask_detection_RBUS film_mask_detection_reg;
	di_im_di_film_teeth_RBUS film_teeth_reg;
	di_di_film_new_function_RBUS film_new_function_reg;// Merlin5
	di_di_film_teethshake_RBUS film_teethshake_reg;
	di_di_film_hsize_ctrl_RBUS film_hsize_ctrl_reg;
	di_di_film_vsize_ctrl_RBUS film_vsize_ctrl_reg;
	di_di_film_bothdiff_ctrl_RBUS film_bothdiff_ctrl_reg;
	di_di_film_nonfilmregion_extend_RBUS film_nonfilmregion_extend_reg;
	di_di_film_nonfilmregion_debounce_RBUS film_nonfilmregion_debounce_reg;
	di_di_film_nonfilmregion_1_RBUS film_nonfilmregion_1_reg;
	di_di_film_nonfilmregion_2_RBUS film_nonfilmregion_2_reg;

	di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
	film_sawtooth_filmframe_th_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_SAWTOOTH_FILMFRAME_TH_reg);
	film_static_sum_th_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_STATIC_SUM_TH_reg);
	film_field_judge_th_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_FIELD_JUDGE_TH_reg);
	film_static_sum_sub_th_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_STATIC_SUM_SUB_TH_reg);
	film_field_judge_sub_th_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_FIELD_JUDGE_SUB_TH_reg);
	film_motion_sum_th_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_MOTION_SUM_TH_reg);
	film_new_function_main_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_NEW_FUNCTION_MAIN_reg);	/* fw film setting*/
	film_edge_type_check_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_EDGE_TYPE_CHECK_reg);
	film_mask_detection_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_MASK_DETECTION_reg);
	film_teeth_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_TEETH_reg);
	film_new_function_reg.regValue = IoReg_Read32(DI_DI_FILM_NEW_FUNCTION_reg);
	film_teethshake_reg.regValue = IoReg_Read32(DI_DI_FILM_TEETHSHAKE_reg);
	film_hsize_ctrl_reg.regValue = IoReg_Read32(DI_DI_FILM_HSIZE_CTRL_reg);
	film_vsize_ctrl_reg.regValue = IoReg_Read32(DI_DI_FILM_VSIZE_CTRL_reg);
	film_bothdiff_ctrl_reg.regValue = IoReg_Read32(DI_DI_FILM_BOTHDIFF_CTRL_reg);
	film_nonfilmregion_extend_reg.regValue = IoReg_Read32(DI_DI_FILM_NONFILMREGION_EXTEND_reg);
	film_nonfilmregion_debounce_reg.regValue = IoReg_Read32(DI_DI_FILM_NONFILMREGION_DEBOUNCE_reg);
	film_nonfilmregion_1_reg.regValue = IoReg_Read32(DI_DI_FILM_NONFILMREGION_1_reg);
	film_nonfilmregion_2_reg.regValue = IoReg_Read32(DI_DI_FILM_NONFILMREGION_2_reg);

	/*extern DRV_film32_table_t gFilm32InitTable[2];*/
	//if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000){
	film_sawtooth_filmframe_th_reg.film_frmotionthl = 				ptr->film_row_px_frame.film_FrMotionThl;
	//}
	film_bothdiff_ctrl_reg.di_film_bothdiff_range = 				ptr->film_row_px_frame.film_bothdiff_range;
	film_teethshake_reg.di_film_bothdiff_weight = 				ptr->film_row_px_frame.film_bothdiff_weight;
	film_teethshake_reg.di_film_bothdiff_shift = 				ptr->film_row_px_frame.film_bothdiff_shift;

	film_teeth_reg.film_teethth_teeth_lb = ptr->film_row_px_field.film_teethth_teeth_lb;
	film_teeth_reg.film_teethth_moving_lb = ptr->film_row_px_field.film_teethth_moving_lb;
	film_teeth_reg.film_teethth_prog_lb = ptr->film_row_px_field.film_teethth_prog_lb;
	film_new_function_reg.di_film_teethth_prog_ub = ptr->film_row_px_field.film_teethth_prog_ub;
	film_teeth_reg.film_teethth_en = ptr->film_row_px_field.film_teethth_en;
	//if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000){
	film_motion_sum_th_reg.film_sawtooth_choose = ptr->film_row_px_field.film_sawtooth_choose;
	film_sawtooth_filmframe_th_reg.film_sawtooththl = 				ptr->film_row_px_field.film_SawtoothThl;
	film_sawtooth_filmframe_th_reg.film22_sawtooththl = 			ptr->film_row_px_field.film_film22_SawtoothTh1;
	//}
	film_motion_sum_th_reg.film22_filter_andor= ptr->film_row_px_field.film_andor;
	film_edge_type_check_reg.film_edgetype_v_pass = ptr->film_row_px_field.film_V_edge_pass;
	film_edge_type_check_reg.film_edgetype_r_pass = ptr->film_row_px_field.film_R_edge_pass;
	film_edge_type_check_reg.film_edgetype_h_pass = ptr->film_row_px_field.film_H_edge_pass;
	film_edge_type_check_reg.film_edgetype_l_pass = ptr->film_row_px_field.film_L_edge_pass;
	film_edge_type_check_reg.film_edgetype_n_pass = ptr->film_row_px_field.film_N_edge_pass;
	film_edge_type_check_reg.film_edgetype_sobelths= ptr->film_row_px_field.film_SobelTHs;
	film_edge_type_check_reg.film_edgetype_sobelthm= ptr->film_row_px_field.film_SobelTHm;
	//if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000){
	film_mask_detection_reg.film_mask_en = ptr->film_row_px_field.film_mask_en;	/* jzl film mask for 22 sequence */
	//}
	film_mask_detection_reg.film_mask_pass_thd= ptr->film_row_px_field.film_mask_pass_count;
	film_mask_detection_reg.film_mask_fiedif_thd= ptr->film_row_px_field.film_mask_FildDiff_thd;
	film_mask_detection_reg.film_mask_frmdif_thd= ptr->film_row_px_field.film_mask_FrmDiff_thd;
	film_new_function_reg.di_film_teethth_moving_single_side = ptr->film_row_px_field.film_teethth_moving_single_side;
	film_teethshake_reg.di_film_teethth_timing = ptr->film_row_px_field.film_teethth_timing;
	film_teethshake_reg.di_film_teethth_teeth_both_en = ptr->film_row_px_field.film_teethth_teeth_both_en;
	//if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000){
	film_static_sum_th_reg.film_frstaticsthl = 				ptr->film_row_th_all.film_FrStaticSThl;
	film_static_sum_th_reg.film_fistaticsthl = 				ptr->film_row_th_all.film_FiStaticSThl;
	film_static_sum_th_reg.film_fieldmotionsumthl = 		ptr->film_row_th_all.film_FieldMotionSumThl;
	film_field_judge_th_reg.film_fiminsthl = 				ptr->film_row_th_all.film_FiMinSThl;
	film_field_judge_th_reg.film_fimaxsthl = 				ptr->film_row_th_all.film_FiMaxSThl;
	film_field_judge_th_reg.film_pairratio_all1 = 			ptr->film_row_th_all.film_PairRatio_all1;
	film_field_judge_th_reg.film_pairratio_all2 = 			ptr->film_row_th_all.film_PairRatio_all2;
	film_motion_sum_th_reg.film22_fieldmotionsumthl = 		ptr->film_row_th_all.film_film22_FieldMotionSumThl;
	film_static_sum_th_reg.film22_fistaticsthl = 			ptr->film_row_th_all.film_film22_FiStaticSThl;

	film_static_sum_sub_th_reg.film_subfrstaticsthl = 			ptr->film_row_th_sub.film_SubFrStaticsSTh1;
	film_static_sum_sub_th_reg.film_subfistaticsthl = 			ptr->film_row_th_sub.film_SubFiStaticsSTh1;
	film_static_sum_sub_th_reg.film_subfieldmotionsumthl = 		ptr->film_row_th_sub.film_SubFieldMotionSumThl;
	film_field_judge_sub_th_reg.film_subfiminsthl = 				ptr->film_row_th_sub.film_SubFiMinSTh1;
	film_field_judge_sub_th_reg.film_subfimaxsthl = 			ptr->film_row_th_sub.film_SubFiMaxSTh1;
	film_field_judge_sub_th_reg.film_pairratio_sub1 = 			ptr->film_row_th_sub.film_PairRatio_sub1;
	film_field_judge_sub_th_reg.film_pairratio_sub2 = 			ptr->film_row_th_sub.film_PairRatio_sub2;
	film_motion_sum_th_reg.film22_subfieldmotionsumthl = 	ptr->film_row_th_sub.Film22_SubFieldMotionSumThl;
	//}
	film_static_sum_sub_th_reg.film22_subfistaticsthl = 	ptr->film_row_th_sub.Film22_SubFiStaticSThl;

	film_teeth_reg.film_teethth_ratio = ptr->film_row_pair.film_teethth_ratio;
	//if (vpq_project_id != 0x00060000 || vpqex_project_id != 0x00060000){
	film_sawtooth_filmframe_th_reg.film_pair_en_auto = 			ptr->film_row_pair.film_pair_en_auto;
	film_sawtooth_filmframe_th_reg.film_pair1_en = 				ptr->film_row_pair.film_pair1_en;
	film_sawtooth_filmframe_th_reg.film_pair2_en = 				ptr->film_row_pair.film_pair2_en;

	film_sawtooth_filmframe_th_reg.film_leavequick_en = 			ptr->film_row_frame.film_LeaveQuick_en;
	film_sawtooth_filmframe_th_reg.film_verscrollingtext_en = 		ptr->film_row_frame.film_VerScrollingText_en;
	film_sawtooth_filmframe_th_reg.film_thumbnail_en = 			ptr->film_row_frame.film_thumbnail_en;
	film_sawtooth_filmframe_th_reg.film_violate_strict_en = 			ptr->film_row_frame.film_violate_strict_en;
	film_motion_sum_th_reg.film32_debounce_fieldnum = 		ptr->film_row_frame.film_film32_debounce_fieldnum;
	film_motion_sum_th_reg.film22_debounce_fieldnum = 		ptr->film_row_frame.film_film22_debounce_fieldnum;
	film_new_function_reg.di_film_framestaticonly = 		ptr->film_row_frame.film_framestaticonly;

	film_new_function_main_reg.fw_film_en =	ptr->film_row_px_frame.film_fw_en;
	film_new_function_main_reg.fw_film_doublebuffer_en = 0x1; 		/*jzl 20150815 for boundary issue*/
	//}
	
	film_nonfilmregion_extend_reg.di_film_nonfilmregion_force = 		ptr->film_row_nonfilm.di_film_nonfilmregion_force;
	film_nonfilmregion_1_reg.di_film_nonfilmregion_s_big = 		ptr->film_row_nonfilm.di_film_nonfilmregion_s_big;
	film_nonfilmregion_1_reg.di_film_nonfilmregion_s_small = 		ptr->film_row_nonfilm.di_film_nonfilmregion_s_small;
	film_nonfilmregion_2_reg.di_film_nonfilmregion_s_both = 		ptr->film_row_nonfilm.di_film_nonfilmregion_s_both;
	film_nonfilmregion_2_reg.di_film_nonfilmregion_th = 		ptr->film_row_nonfilm.di_film_nonfilmregion_th;
	film_nonfilmregion_debounce_reg.di_film_nonfilmregion_up_debounce = 		ptr->film_row_nonfilm.di_film_nonfilmregion_up_debounce;
	film_nonfilmregion_extend_reg.di_film_nonfilmregion_up_extend = 		ptr->film_row_nonfilm.di_film_nonfilmregion_up_extend;
	film_nonfilmregion_debounce_reg.di_film_nonfilmregion_dn_debounce = 		ptr->film_row_nonfilm.di_film_nonfilmregion_dn_debounce;
	film_nonfilmregion_extend_reg.di_film_nonfilmregion_dn_extend = 		ptr->film_row_nonfilm.di_film_nonfilmregion_dn_extend;
	
	film_hsize_ctrl_reg.di_film_xpos_1by4 = 8;
	film_hsize_ctrl_reg.di_film_xpos_3by4 = 24;
	film_vsize_ctrl_reg.di_film_ypos_1by4 = 8;
	film_vsize_ctrl_reg.di_film_ypos_3by4 = 29;

	IoReg_Write32(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);
	IoReg_Write32(DI_IM_DI_FILM_SAWTOOTH_FILMFRAME_TH_reg, film_sawtooth_filmframe_th_reg.regValue);
	IoReg_Write32(DI_IM_DI_FILM_STATIC_SUM_TH_reg, film_static_sum_th_reg.regValue);
	IoReg_Write32(DI_IM_DI_FILM_FIELD_JUDGE_TH_reg, film_field_judge_th_reg.regValue);
	IoReg_Write32(DI_IM_DI_FILM_STATIC_SUM_SUB_TH_reg, film_static_sum_sub_th_reg.regValue);
	IoReg_Write32(DI_IM_DI_FILM_FIELD_JUDGE_SUB_TH_reg, film_field_judge_sub_th_reg.regValue);
	IoReg_Write32(DI_IM_DI_FILM_MOTION_SUM_TH_reg, film_motion_sum_th_reg.regValue);
	IoReg_Write32(DI_IM_DI_FILM_NEW_FUNCTION_MAIN_reg, film_new_function_main_reg.regValue);
	IoReg_Write32(DI_IM_DI_FILM_EDGE_TYPE_CHECK_reg, film_edge_type_check_reg.regValue);
	IoReg_Write32(DI_IM_DI_FILM_MASK_DETECTION_reg, film_mask_detection_reg.regValue);
	IoReg_Write32(DI_IM_DI_FILM_TEETH_reg, film_teeth_reg.regValue);
	IoReg_Write32(DI_DI_FILM_NEW_FUNCTION_reg, film_new_function_reg.regValue);
	IoReg_SetBits(DI_IM_DI_SI_FILM_FINAL_RESULT_reg, _BIT14); // frame static only
	IoReg_Write32(DI_DI_FILM_TEETHSHAKE_reg, film_teethshake_reg.regValue);
	IoReg_Write32(DI_DI_FILM_HSIZE_CTRL_reg, film_hsize_ctrl_reg.regValue);
	IoReg_Write32(DI_DI_FILM_VSIZE_CTRL_reg, film_vsize_ctrl_reg.regValue);
	IoReg_Write32(DI_DI_FILM_BOTHDIFF_CTRL_reg, film_bothdiff_ctrl_reg.regValue);
	IoReg_Write32(DI_DI_FILM_NONFILMREGION_EXTEND_reg, film_nonfilmregion_extend_reg.regValue);
	IoReg_Write32(DI_DI_FILM_NONFILMREGION_DEBOUNCE_reg, film_nonfilmregion_debounce_reg.regValue);
	IoReg_Write32(DI_DI_FILM_NONFILMREGION_1_reg, film_nonfilmregion_1_reg.regValue);
	IoReg_Write32(DI_DI_FILM_NONFILMREGION_2_reg, film_nonfilmregion_2_reg.regValue);
}
void drvif_module_film_setting_tv006(DRV_film_table_t *ptr)
{
	//di_im_di_control_RBUS di_control_reg;
	di_im_di_film_sawtooth_filmframe_th_RBUS film_sawtooth_filmframe_th_reg;
	di_im_di_film_static_sum_th_RBUS film_static_sum_th_reg;
	di_im_di_film_field_judge_th_RBUS film_field_judge_th_reg;
	di_im_di_film_static_sum_sub_th_RBUS film_static_sum_sub_th_reg;
	di_im_di_film_field_judge_sub_th_RBUS film_field_judge_sub_th_reg;
	di_im_di_film_motion_sum_th_RBUS film_motion_sum_th_reg;
	di_im_di_film_new_function_main_RBUS film_new_function_main_reg;	/* fw film setting*/
	di_im_di_film_mask_detection_RBUS film_mask_detection_reg;

	//di_control_reg.regValue = IoReg_Read32(DI_IM_DI_CONTROL_reg);
	film_sawtooth_filmframe_th_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_SAWTOOTH_FILMFRAME_TH_reg);
	film_static_sum_th_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_STATIC_SUM_TH_reg);
	film_field_judge_th_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_FIELD_JUDGE_TH_reg);
	film_static_sum_sub_th_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_STATIC_SUM_SUB_TH_reg);
	film_field_judge_sub_th_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_FIELD_JUDGE_SUB_TH_reg);
	film_motion_sum_th_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_MOTION_SUM_TH_reg);
	film_new_function_main_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_NEW_FUNCTION_MAIN_reg);	/* fw film setting*/
	film_mask_detection_reg.regValue = IoReg_Read32(DI_IM_DI_FILM_MASK_DETECTION_reg);

	/*extern DRV_film32_table_t gFilm32InitTable[2];*/


	film_motion_sum_th_reg.film_sawtooth_choose= ptr->film_sawtooth_choose;

	film_sawtooth_filmframe_th_reg.film_leavequick_en = 			ptr->film_LeaveQuick_en;
	film_sawtooth_filmframe_th_reg.film_verscrollingtext_en = 		ptr->film_VerScrollingText_en;
	film_sawtooth_filmframe_th_reg.film_thumbnail_en = 			ptr->film_thumbnail_en;
	film_sawtooth_filmframe_th_reg.film_pair1_en = 				ptr->film_pair1_en;
	film_sawtooth_filmframe_th_reg.film_pair2_en = 				ptr->film_pair2_en;
	film_sawtooth_filmframe_th_reg.film_pair_en_auto = 			ptr->film_pair_en_auto;
	film_sawtooth_filmframe_th_reg.film_violate_strict_en = 			ptr->film_violate_strict_en;
	film_sawtooth_filmframe_th_reg.film_sawtooththl = 				ptr->film_SawtoothThl;
	film_sawtooth_filmframe_th_reg.film22_sawtooththl = 			ptr->film_film22_SawtoothTh1;
	film_sawtooth_filmframe_th_reg.film_frmotionthl = 				ptr->film_FrMotionThl;

	film_static_sum_th_reg.film_fieldmotionsumthl = 		ptr->film_FieldMotionSumThl;
	film_static_sum_th_reg.film22_fistaticsthl = 			ptr->film_film22_FiStaticSThl;
	film_static_sum_th_reg.film_frstaticsthl = 				ptr->film_FrStaticSThl;
	film_static_sum_th_reg.film_fistaticsthl = 				ptr->film_FiStaticSThl;

	film_field_judge_th_reg.film_pairratio_all1 = 			ptr->film_PairRatio_all1;
	film_field_judge_th_reg.film_pairratio_all2 = 			ptr->film_PairRatio_all2;
	film_field_judge_th_reg.film_fiminsthl = 				ptr->film_FiMinSThl;
	film_field_judge_th_reg.film_fimaxsthl = 				ptr->film_FiMaxSThl;

	film_static_sum_sub_th_reg.film_subfieldmotionsumthl = 		ptr->film_SubFieldMotionSumThl;
	film_static_sum_sub_th_reg.film_subfrstaticsthl = 			ptr->film_SubFrStaticsSTh1;
	film_static_sum_sub_th_reg.film_subfistaticsthl = 			ptr->film_SubFiStaticsSTh1;

	film_field_judge_sub_th_reg.film_pairratio_sub1 = 			ptr->film_PairRatio_sub1;
	film_field_judge_sub_th_reg.film_pairratio_sub2 = 			ptr->film_PairRatio_sub2;
	film_field_judge_sub_th_reg.film_subfiminsthl = 				ptr->film_SubFiMinSTh1;
	film_field_judge_sub_th_reg.film_subfimaxsthl = 			ptr->film_SubFiMaxSTh1;

	film_motion_sum_th_reg.film32_debounce_fieldnum = 		ptr->film_film32_debounce_fieldnum;
	film_motion_sum_th_reg.film22_debounce_fieldnum = 		ptr->film_film22_debounce_fieldnum;
	film_motion_sum_th_reg.film22_subfieldmotionsumthl = 	ptr->film_film22_SubFieldMotionSumThl;
	film_motion_sum_th_reg.film22_fieldmotionsumthl = 		ptr->film_film22_FieldMotionSumThl;

	film_new_function_main_reg.fw_film_en =	ptr->film_fw_en;
	film_new_function_main_reg.fw_film_doublebuffer_en = 0x1; 		/*jzl 20150815 for boundary issue*/

	film_mask_detection_reg.film_mask_en = 1;	/* jzl film mask for 22 sequence */

	//IoReg_Write32(DI_IM_DI_CONTROL_reg, di_control_reg.regValue);
	IoReg_Write32(DI_IM_DI_FILM_SAWTOOTH_FILMFRAME_TH_reg, film_sawtooth_filmframe_th_reg.regValue);
	IoReg_Write32(DI_IM_DI_FILM_STATIC_SUM_TH_reg, film_static_sum_th_reg.regValue);
	IoReg_Write32(DI_IM_DI_FILM_FIELD_JUDGE_TH_reg, film_field_judge_th_reg.regValue);
	IoReg_Write32(DI_IM_DI_FILM_STATIC_SUM_SUB_TH_reg, film_static_sum_sub_th_reg.regValue);
	IoReg_Write32(DI_IM_DI_FILM_FIELD_JUDGE_SUB_TH_reg, film_field_judge_sub_th_reg.regValue);
	IoReg_Write32(DI_IM_DI_FILM_MOTION_SUM_TH_reg, film_motion_sum_th_reg.regValue);
	IoReg_Write32(DI_IM_DI_FILM_NEW_FUNCTION_MAIN_reg, film_new_function_main_reg.regValue);
	IoReg_Write32(DI_IM_DI_FILM_MASK_DETECTION_reg, film_mask_detection_reg.regValue);

	IoReg_Mask32(DI_db_reg_ctl_reg, ~DI_db_reg_ctl_db_apply_mask, DI_db_reg_ctl_db_apply_mask);
}


