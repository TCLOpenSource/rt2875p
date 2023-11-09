/**
 * @file hal_post.c
 * @brief This file is for post register setting
 * @date Nov.14.2014
 * @version V0.0.1
 * @author Martin Yan
 * @par Copyright (C) 2014 Real-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/

#include "memc_isr/Common/kw_debug.h"
#include "memc_isr/Common/kw_common.h"
#include "memc_isr/Driver/regio.h"
#include "memc_isr/Common/memc_type.h"
#include "memc_reg_def.h"
#include "linux/spinlock.h"

static DEFINE_SPINLOCK(memc_task_Spinlock);/*Spin lock no context switch. This is for copy paramter*/
/**
 * @brief This function set post pattern size
 * @param [in] TIMING_PARA_ST stOutputTiming
 * @retval VOID
*/
VOID HAL_POST_SetPatternSize(TIMING_PARA_ST stOutputTiming)
{
	//reg_post_patt_htotal
	WriteRegister(KPOST_TOP_KPOST_TOP_10_reg,0,15,stOutputTiming.u32HTotal/4);
	//reg_post_patt_vtotal
	WriteRegister(KPOST_TOP_KPOST_TOP_10_reg,16,31,stOutputTiming.u32VTotal);
	//reg_post_patt_hact
	WriteRegister(KPOST_TOP_KPOST_TOP_14_reg,0,15,stOutputTiming.u32HActive);
	//reg_post_patt_vact
	WriteRegister(KPOST_TOP_KPOST_TOP_14_reg,16,31,stOutputTiming.u32VActive);
}

/**
 * @brief This function set post pr mode enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetPREnable(BOOL bEnable)
{
	//reg_pr_mode_en
	WriteRegister(KPOST_TOP_KPOST_TOP_28_reg,0,0,(unsigned int)bEnable);
}

/**
 * @brief This function set post hde center value
 * @param [in] unsigned int u32CenterValue
 * @retval VOID
*/
VOID HAL_POST_SetHdeCenterValue(unsigned int u32CenterValue)
{
	//reg_hde_center_value
	WriteRegister(KPOST_TOP_KPOST_TOP_28_reg,1,12,u32CenterValue);
}

/**
 * @brief This function set post 4 port hde center value
 * @param [in] unsigned int u32CenterValue
 * @retval VOID
*/
VOID HAL_POST_Set4PortHdeCenterValue(unsigned int u32CenterValue)
{
	//reg_4port_hde_center_value
//	WriteRegister(FRC_TOP__KPOST_TOP__4port_hde_center_value_ADDR no mat,FRC_TOP__KPOST_TOP__4port_hde_center_value_BITSTART no mat,FRC_TOP__KPOST_TOP__4port_hde_center_value_BITEND no mat,u32CenterValue);
}

/**
 * @brief This function set v scaler bypass
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetVscalerBypass(BOOL bEnable)
{
	//reg_post_vsc_bypass
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KPOST_TOP__post_vsc_bypass_ADDR no mat,FRC_TOP__KPOST_TOP__post_vsc_bypass_BITSTART no mat,FRC_TOP__KPOST_TOP__post_vsc_bypass_BITEND no mat,(unsigned int)bEnable);
}

/**
 * @brief This function set v scaler mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_POST_SetVscalerMode(unsigned int u32Mode)
{
	//reg_post_vsc_mode
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KPOST_TOP__post_vsc_mode_ADDR no mat,FRC_TOP__KPOST_TOP__post_vsc_mode_BITSTART no mat,FRC_TOP__KPOST_TOP__post_vsc_mode_BITEND no mat,u32Mode);
}

/**
 * @brief This function set v active
 * @param [in] unsigned int u32Vactive
 * @retval VOID
*/
VOID HAL_POST_SetVActive(unsigned int u32Vactive)
{
	//reg_post_v_act
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KPOST_TOP__post_v_act_ADDR no mat,FRC_TOP__KPOST_TOP__post_v_act_BITSTART no mat,FRC_TOP__KPOST_TOP__post_v_act_BITEND no mat,u32Vactive);
}

/**
 * @brief This function set h scaler bypass
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetHscalerBypass(BOOL bEnable)
{
	//reg_post_hsc_bypass
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KPOST_TOP__post_hsc_bypass_ADDR no mat,FRC_TOP__KPOST_TOP__post_hsc_bypass_BITSTART no mat,FRC_TOP__KPOST_TOP__post_hsc_bypass_BITEND no mat,(unsigned int)bEnable);
}

/**
 * @brief This function set h scaler mode
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_POST_SetHscalerMode(unsigned int u32Mode)
{
	//reg_post_hsc_mode_y
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KPOST_TOP__post_hsc_mode_y_ADDR no mat,FRC_TOP__KPOST_TOP__post_hsc_mode_y_BITSTART no mat,FRC_TOP__KPOST_TOP__post_hsc_mode_y_BITEND no mat,u32Mode);
	//reg_post_hsc_mode_c
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KPOST_TOP__post_hsc_mode_c_ADDR no mat,FRC_TOP__KPOST_TOP__post_hsc_mode_c_BITSTART no mat,FRC_TOP__KPOST_TOP__post_hsc_mode_c_BITEND no mat,u32Mode);
}

/**
 * @brief This function set h scaler mode of C
 * @param [in] unsigned int u32Mode
 * @retval VOID
*/
VOID HAL_POST_SetHscalerMode_C(unsigned int u32Mode)
{
	//reg_post_hsc_mode_c
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KPOST_TOP__post_hsc_mode_c_ADDR no mat,FRC_TOP__KPOST_TOP__post_hsc_mode_c_BITSTART no mat,FRC_TOP__KPOST_TOP__post_hsc_mode_c_BITEND no mat,u32Mode);
}

/**
 * @brief This function set h scaler offset
 * @param [in] unsigned int u32Offset
 * @retval VOID
*/
VOID HAL_POST_SetHscalerOffset(unsigned int u32Offset)
{
	//reg_post_hsc_ofst_y
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KPOST_TOP__post_hsc_ofst_y_ADDR no mat,FRC_TOP__KPOST_TOP__post_hsc_ofst_y_BITSTART no mat,FRC_TOP__KPOST_TOP__post_hsc_ofst_y_BITEND no mat,u32Offset);
	//reg_post_hsc_ofst_c
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KPOST_TOP__post_hsc_ofst_c_ADDR no mat,FRC_TOP__KPOST_TOP__post_hsc_ofst_c_BITSTART no mat,FRC_TOP__KPOST_TOP__post_hsc_ofst_c_BITEND no mat,u32Offset);
}

/**
 * @brief This function set h scaler offset of C
 * @param [in] unsigned int u32Offset
 * @retval VOID
*/
VOID HAL_POST_SetHscalerOffset_C(unsigned int u32Offset)
{
	//reg_post_hsc_ofst_c
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KPOST_TOP__post_hsc_ofst_c_ADDR no mat,FRC_TOP__KPOST_TOP__post_hsc_ofst_c_BITSTART no mat,FRC_TOP__KPOST_TOP__post_hsc_ofst_c_BITEND no mat,u32Offset);
}

/**
 * @brief This function set h scaler coef
 * @param [in] unsigned int u32Coef
 * @retval VOID
*/
VOID HAL_POST_SetHscalerCoef(unsigned int u32Coef)
{
	//reg_post_hsc_coef_y
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KPOST_TOP__post_hsc_coef_y_ADDR no mat,FRC_TOP__KPOST_TOP__post_hsc_coef_y_BITSTART no mat,FRC_TOP__KPOST_TOP__post_hsc_coef_y_BITEND no mat,u32Coef);
	//reg_post_hsc_coef_c
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KPOST_TOP__post_hsc_coef_c_ADDR no mat,FRC_TOP__KPOST_TOP__post_hsc_coef_c_BITSTART no mat,FRC_TOP__KPOST_TOP__post_hsc_coef_c_BITEND no mat,u32Coef);
}

/**
 * @brief This function set h scaler coef of C
 * @param [in] unsigned int u32Coef
 * @retval VOID
*/
VOID HAL_POST_SetHscalerCoef_C(unsigned int u32Coef)
{
	//reg_post_hsc_coef_c
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KPOST_TOP__post_hsc_coef_c_ADDR no mat,FRC_TOP__KPOST_TOP__post_hsc_coef_c_BITSTART no mat,FRC_TOP__KPOST_TOP__post_hsc_coef_c_BITEND no mat,u32Coef);
}

/**
 * @brief This function set h scaler coef1
 * @param [in] unsigned int u32Coef
 * @retval VOID
*/
VOID HAL_POST_SetHscalerCoef1(unsigned int u32Coef)
{
	//reg_post_hsc_coef_y1
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KPOST_TOP__post_hsc_coef_y1_ADDR no mat,FRC_TOP__KPOST_TOP__post_hsc_coef_y1_BITSTART no mat,FRC_TOP__KPOST_TOP__post_hsc_coef_y1_BITEND no mat,u32Coef);
	//reg_post_hsc_coef_c1
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KPOST_TOP__post_hsc_coef_c1_ADDR no mat,FRC_TOP__KPOST_TOP__post_hsc_coef_c1_BITSTART no mat,FRC_TOP__KPOST_TOP__post_hsc_coef_c1_BITEND no mat,u32Coef);
}

/**
 * @brief This function set h scaler coef1 of C
 * @param [in] unsigned int u32Coef
 * @retval VOID
*/
VOID HAL_POST_SetHscalerCoef1_C(unsigned int u32Coef)
{
	//reg_post_hsc_coef_c1
// MEMC_K6L_BRING_UP //	WriteRegister(FRC_TOP__KPOST_TOP__post_hsc_coef_c1_ADDR no mat,FRC_TOP__KPOST_TOP__post_hsc_coef_c1_BITSTART no mat,FRC_TOP__KPOST_TOP__post_hsc_coef_c1_BITEND no mat,u32Coef);
}


/**
 * @brief This function set csc enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetCSCEnable(BOOL bEnable)
{
	//reg_post_convert_on
	WriteRegister(KPOST_TOP_KPOST_TOP_94_reg,24,24,(unsigned int)bEnable);
}

/**
 * @brief This function set post csc mode
 * @param [in] CSC_MODE enCSC_Mode
 * @retval VOID
*/
VOID HAL_POST_SetCSCMode(CSC_MODE enCSC_Mode)
{
	//reg_post_convert_mode
	WriteRegister(KPOST_TOP_KPOST_TOP_94_reg,25,28,(unsigned int)enCSC_Mode);
}

/**
 * @brief This function set post mute screen enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetMuteScreenEnable(BOOL bEnable)
{
	//reg_bluescreen
	WriteRegister(KPOST_TOP_KPOST_TOP_60_reg,30,30,(unsigned int)bEnable);
}

/**
 * @brief This function set post mute screen RGB
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetMuteScreenRGBEnable(unsigned int u32RGB)
{
	//reg_bluescreen_rgb
	WriteRegister(KPOST_TOP_KPOST_TOP_60_reg,0,29,u32RGB);
}

/**
 * @brief This function set post data mapping
 * @param [in] unsigned int u32Data_Mapping
 * @retval VOID
*/
VOID HAL_POST_SetDataMapping(unsigned int u32Data_Mapping)
{
	//reg_post_convert_map
	WriteRegister(KPOST_TOP_KPOST_TOP_94_reg,29,31,(unsigned int)u32Data_Mapping);
}

/**
 * @brief This function get post data mapping
 * @param [in] VOID
 * @retval data mapping
*/
unsigned int HAL_POST_GetDataMapping(VOID)
{
	unsigned int u32Data_Mapping = 0;
	//reg_post_convert_map
	ReadRegister(KPOST_TOP_KPOST_TOP_94_reg,29,31,&u32Data_Mapping);
	return u32Data_Mapping;
}

/**
 * @brief This function set black insert line mode enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetBlackInsertLineModeEnable(BOOL bEnable)
{
	//reg_blackinsert_line_en
	WriteRegister(KPOST_TOP_KPOST_TOP_08_reg,10,10,(unsigned int)bEnable);
}

/**
 * @brief This function set black insert line mode pattern number
 * @param [in] unsigned int u32PatternNum
 * @retval VOID
*/
VOID HAL_POST_SetBlackInsertLineModePatternNum(unsigned int u32PatternNum)
{
	//reg_blackinsert_line_type
	WriteRegister(KPOST_TOP_KPOST_TOP_08_reg,11,14,u32PatternNum);
}

/**
 * @brief This function set black insert line mode pattern
 * @param [in] unsigned int u32Pattern
 * @retval VOID
*/
VOID HAL_POST_SetBlackInsertLineModeLPattern(unsigned int u32Pattern)
{
	//reg_blackinsert_line_patt_l
	WriteRegister(KPOST_TOP_KPOST_TOP_08_reg,15,30,u32Pattern);
}

/**
 * @brief This function set black insert line mode pattern
 * @param [in] unsigned int u32Pattern
 * @retval VOID
*/
VOID HAL_POST_SetBlackInsertLineModeRPattern(unsigned int u32Pattern)
{
	//reg_blackinsert_line_patt_r
	WriteRegister(KPOST_TOP_KPOST_TOP_0C_reg,15,30,u32Pattern);
}

/**
 * @brief This function set black insert frame mode enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetBlackInsertFrameModeEnable(BOOL bEnable)
{
	//reg_blackinsert_frame_en
	WriteRegister(KPOST_TOP_KPOST_TOP_08_reg,31,31,(unsigned int)bEnable);
}

/**
 * @brief This function set kpost out interrupt select
 * @param [in] 7 for FRC, 6 for OSD, 5 for Out
 * @retval VOID
*/
VOID HAL_POST_SetOutINTSel(unsigned int u32Sel)
{
	//reg_post_out_int_sel
	WriteRegister(KPOST_TOP_KPOST_TOP_04_reg,13,15,u32Sel);
}

/**
 * @brief This function set kpost out interrupt source
 * @param [in] 1
 * @retval VOID
*/
VOID HAL_POST_SetOutINTSource(unsigned int u32Sor)
{
	//reg_post_int_source_sel
	WriteRegister(KPOST_TOP_KPOST_TOP_08_reg,0,0,u32Sor);
}

/**
 * @brief This function set kpost out interrupt horizantal number
 * @param [in] H number
 * @retval VOID
*/
VOID HAL_POST_SetOutINTHNum(unsigned int u32Hnum)
{
	//reg_post_out_vtotal_int
	WriteRegister(KPOST_TOP_KPOST_TOP_04_reg,20,31,u32Hnum);
}

/**
 * @brief This function set kpost out interrupt enable
 * @param [in] u32IntType: 0 for H sync, 1 for V sync
 * @param [in] bEnable
 * @retval VOID
*/
VOID HAL_POST_SetOutINTEnable(unsigned int u32IntType, BOOL bEnable)
{
	unsigned int u32InINT_en = 0;
	ReadRegister(KPOST_TOP_KPOST_TOP_08_reg,5,8, &u32InINT_en);
	
	if(bEnable)
	{
		u32InINT_en = u32InINT_en | (1 << u32IntType);
	}
	else
	{
		u32InINT_en = u32InINT_en & (~(1 << u32IntType));
	}
	//rtd_pr_memc_notice("%s %d\n",__func__,__LINE__);
	//rtd_pr_memc_notice("[%d,%d,%d]\r\n",u32InINT_en, u32IntType, bEnable);
	//reg_post_int_en
	WriteRegister(KPOST_TOP_KPOST_TOP_08_reg,5,8,u32InINT_en);
}

BOOL HAL_POST_GetOutINTEnable(unsigned int u32IntType)
{
	unsigned int v = 0;

       ReadRegister(KPOST_TOP_KPOST_TOP_08_reg,5,8,&v);
       return (v >> u32IntType) & 0x1;
}

/**
 * @brief This function set kpost top out interrupt clear
 * @param [in] eIntType: 0 for H sync, 1 for V sync
 * @param [in] bWclr
 * @retval VOID
*/
VOID HAL_KPOST_TOP_SetOutINTWclr(INT_TYPE eIntType, BOOL bWclr)
{
	unsigned int u32InINT_Clear = 0;
	unsigned long flags;//for spin_lock_irqsave
	spin_lock_irqsave(&memc_task_Spinlock, flags);
	ReadRegister(KPOST_TOP_KPOST_TOP_08_reg,1,4, &u32InINT_Clear);

	if(bWclr)
		u32InINT_Clear = u32InINT_Clear | ( 1 << (unsigned int)eIntType);
	else
		u32InINT_Clear = u32InINT_Clear & (~( 1 << (unsigned int)eIntType));
	
	//reg_kmc_int_en
	WriteRegister(KPOST_TOP_KPOST_TOP_08_reg,1,4, u32InINT_Clear);
	spin_unlock_irqrestore(&memc_task_Spinlock, flags);
}

/**
 * @brief This function get kost top out interrupt status
 * @retval unsigned int
*/
unsigned int HAL_KPOST_TOP_GetOutINTStatus(VOID)
{
	unsigned int u32OutINT_Sts = 0;
	//ReadRegister(KPOST_TOP_KPOST_TOP_F0_reg,24,27, &u32OutINT_Sts);
	ReadRegister(KPOST_TOP_KPOST_TOP_F0_reg,27,30, &u32OutINT_Sts);
	return u32OutINT_Sts;
}

/**
 * @brief This function set post cursor enable
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetCursorEnable(BOOL bEnable)
{
	//reg_post_cursor_en
	WriteRegister(KPOST_TOP_KPOST_TOP_00_reg,15,15,(unsigned int)bEnable);
}

/**
 * @brief This function set post cursor show
 * @param [in] BOOL bEnable
 * @retval VOID
*/
VOID HAL_POST_SetCursorShow(BOOL bEnable)
{
	//reg_post_cursor_show
	WriteRegister(KPOST_TOP_KPOST_TOP_00_reg,13,13,(unsigned int)bEnable);
}

/**
 * @brief This function set post cursor position
 * @param [in] reg_post_cursor_h
 * @param [in] reg_post_cursor_v
 * @retval VOID
*/
VOID HAL_POST_SetCursorPosition(unsigned int u32Hposition, unsigned int u32Vposition)
{
	//reg_post_cursor_h
	WriteRegister(KPOST_TOP_KPOST_TOP_00_reg,0,12,(unsigned int)u32Hposition);
	//reg_post_cursor_v
	WriteRegister(KPOST_TOP_KPOST_TOP_00_reg,16,28,(unsigned int)u32Vposition);
}

/**
 * @brief This function set post cursor width
 * @param [in] unsigned int u32Width(0~15)
 * @retval VOID
*/
VOID HAL_POST_SetCursorWidth(unsigned int u32Width)
{
	//reg_post_cursor_h_width
	WriteRegister(KPOST_TOP_KPOST_TOP_20_reg,28,31,(unsigned int)u32Width);
}

/**
 * @brief This function set post cursor color
 * @param [in] unsigned int u32Width(0~7)
 * @retval VOID
*/
VOID HAL_POST_SetCursorColor(unsigned int u32Color)
{
	//reg_post_cursor_h_width
	WriteRegister(KPOST_TOP_KPOST_TOP_00_reg,29,31,(unsigned int)u32Color);
}

