#ifndef _RTK_AI_COMMON_H_
#define _RTK_AI_COMMON_H_

/************************************************************************
 *  Include files
 ************************************************************************/
#include <rbus/nnwp_reg.h>
#include <rbus/sys_reg_reg.h>
#include <scaler/scalerCommon.h>
#include <tvscalercontrol/io/ioregdrv.h>

 /************************************************************************
 *  Definitions
 ************************************************************************/
#define BIT_AI_PQ						_BIT0
#define BIT_AI_AUDIO					_BIT1
#define BIT_AI_CAMERA					_BIT2

#define rtk_ai_enable_status( X )	\
		IoReg_Write32(NNWP_nnwp_dmy1_rw_reg,IoReg_Read32(NNWP_nnwp_dmy1_rw_reg)|X)

#define rtk_ai_disable_status( X )	\
		IoReg_Write32(NNWP_nnwp_dmy1_rw_reg,IoReg_Read32(NNWP_nnwp_dmy1_rw_reg)&(~X))

#endif	/* _RTK_AI_COMMON_H_ */
