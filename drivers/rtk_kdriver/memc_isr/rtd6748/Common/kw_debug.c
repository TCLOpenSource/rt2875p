#if 1
/**
 * @file kw_debug.c
 * @brief This header file defines Kiwi-Image debug trace to control all model debug info.
 * @date August.24.2014
 * @version V0.0.1
 * @author Lixiangjun
 * @par KW Debug trace V0.0.1 - Copyright (C) 2014 Kiwi-Image Co. LTD
 * @par History:
 *			Version: 0.0.1
*/
#ifdef WIN32
#define DLL_IMPLEMENT
#endif

#include "memc_isr/include/kw_type.h"
#include "memc_isr/Common/kw_debug.h"

#ifdef CONFIG_ARM64 //ARM32 compatible
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
#endif

unsigned int u32DebugLevel = (unsigned int)DBG_MSG;

VOID COM_SetDebugLevel(unsigned int u32Level)
{
	u32DebugLevel = u32Level;
}

unsigned int COM_GetDebugLevel(VOID)
{
	return u32DebugLevel;
}
int64_t MEMC_GetPTS(void)
{
	unsigned int lo = rtd_inl(TIMER_SCPU_CLK90K_LO_reg) ;
	unsigned int hi = rtd_inl(TIMER_SCPU_CLK90K_HI_reg) ;
	int64_t ret = lo | (((int64_t)hi) << 32) ;

	return ret ;
}

#endif
